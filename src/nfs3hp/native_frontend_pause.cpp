#include "native_controls_pipeline.h"
#include "native_controls_race.h"
#include "native_frontend_pause.h"
#include "native_load_saves.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_controls_devices.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_glue.h"
#include "native_frontend_loop.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menus.h"
#include "native_load_cardb.h"
#include "port_buttons.h"
#include "port_hud.h"
#include "port_menus.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <iterator>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
x86::reg32 callAt(fe::Guest& guest, x86::reg32 target, const fe::Arguments& arguments = fe::Arguments())
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    x86::reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const x86::reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<x86::reg32> result = guest::pointerCall(guest.app(), cpu, target, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static x86::Float integer(std::int32_t value) { return x86::Float(x86::sreg32(value)); }
    x86::Float load(x86::reg32 address) const { return x86::Float(m_guest.floatAt(address)); }
    x86::Float add(const x86::Float& a, const x86::Float& b) { return fp::add(a, b); }
    x86::Float sub(const x86::Float& a, const x86::Float& b) { return fp::sub(a, b); }
    x86::Float mul(const x86::Float& a, const x86::Float& b) { return fp::mul(a, b); }
    bool above(const x86::Float& a, const x86::Float& b) { return a > b; }
    bool below(const x86::Float& a, const x86::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const x86::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.cpu(), double(value));
        return fp::toInteger<x86::sreg32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

/* The screens of a race's pause, decompiled:
 *
 *   pause      PAUSE.MNU: save the series' game, restart, quit, the options;
 *   psoption,  the options from there: the backdrop dimmed, the sound's button
 *   psoptn2    greyed out when the game has no sound;
 *   pscntl1    the controls, the old ones kept for Cancel;
 *   psgraph    the graphics, likewise;
 *   pscamera   the cameras: no two keys on one;
 *   pshud,     the display of a race, for one player and for each of two: which
 *   pshud1p,   of the racer's or the cop's instruments show, and how large the
 *   pshud2p    map is;
 *   quitto     where to quit to, asked again each time;
 *   aituner    nothing of its own.
 *
 * The saves pause offers -- a series' game, a tournament's -- are here too, with
 * what looks for a file before it is written over; the ghost's and the replay's
 * own writers are native_load_saves.cpp's.  So are the rectangles of the race's
 * views that pause reads (sub_4beb20) and their small helpers.
 *
 * Plan step 5: each function a typed core in frontend::pause (native_frontend_pause.h), in a frame
 * of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * this file calls another as C++ (guest::direct); the rest of the game still by address.  Where
 * several of the game's functions share a body, the body stays a helper and each has a core. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The game's state.
constexpr x86::reg32 kNetworkHost = 0x7a22da;      // int16
constexpr x86::reg32 kNetworkJoin = 0x7a22e4;
constexpr x86::reg32 kInRace = 0x559234;           // the menus are a race's pause screens
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kTrackLightingFlags = 0x06;
constexpr x86::reg32 kDisplayBuffering = 0x7a3a68;
constexpr x86::reg32 kDeveloperFlags = 0x55eb2e;   // byte
constexpr std::uint8_t kDeveloperQuit = 0x20;

// The settings, and their copy that Cancel puts back.
constexpr x86::reg32 kSettingsKept = 0x662ca0;
constexpr x86::reg32 kSettingsSize = 0x176c;
constexpr x86::reg32 kScreenSizeKept = 0x662da0;

// Fields of items the header does not name.
constexpr x86::reg32 kItemOnKey = 0x30;            // a button's key handler
constexpr x86::reg32 kSliderValue = 0x54;          // a slider's value, a float
constexpr std::uint32_t kButton = 5;               // an item's type (item::kType)

// The game's functions.
constexpr std::uint32_t kDimBy = 0x3f19999a;       // 0.6, the shade pushed for 0x451530
constexpr x86::reg32 kButtonKey = 0x461270;
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kCarFlags = 0x200;            // byte of the car
constexpr std::uint8_t kCarIsCop = 0x20;

// This group's own handed to the game: callbacks, handlers.
constexpr x86::reg32 kSaveNameAnswered = 0x4478c0;
constexpr x86::reg32 kOverwriteAnswered = 0x447800;
constexpr x86::reg32 kAskLeaveAnswered = 0x44c610;
constexpr x86::reg32 kQuitChoiceAnswered = 0x44c730;
constexpr x86::reg32 kOnQuitClicked = 0x44c7b0;
constexpr x86::reg32 kOnRestartClicked = 0x44ca10;
constexpr x86::reg32 kRestartAnswered = 0x44c670;
constexpr x86::reg32 kOnSaveClicked = 0x44ca80;
constexpr x86::reg32 kCancelKey = 0x44d170;
constexpr x86::reg32 kOnWidescreenClicked = 0x44d270;
constexpr x86::reg32 kIntensityMoved = 0x44cf70;

/* The settings copied as they stand: into the keeping, back out of it. */
void copySettings(Guest& guest, x86::reg32 to, x86::reg32 from)
{
    for (x86::reg32 at = 0; at < kSettingsSize; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
}

/* The race behind a pause screen, darkened: the shade pushed where the game's code pushes it, under
 * the frame; eax handed on as it is. */
void dim(Guest& guest, CPU& cpu)
{
    const x86::reg32 slot = guest.frame() - 4;
    guest.setDword(slot, kDimBy);
    fe::direct<0x451530, port::text::dimmed>(guest.app(), cpu, slot, 0);  // eax: its result is not read
}

/* The item of a screen named `codelink` (sub_442a40), 0 for none: the look-up's result, which the screens' functions
 * go on with as "what the calls left". */
x86::reg32 foundAt(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    return fe::direct<0x442a40, frontend::menus::findItem>(guest.app(), guest.cpu(), menu, codelink.address);
}

x86::reg32 hideItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    const x86::reg32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setHidden(true);
    return found;
}

x86::reg32 showItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    const x86::reg32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setHidden(false);
    return found;
}

x86::reg32 disableItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    const x86::reg32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setDisabled(true);
    return found;
}

/* The dialog and the question as fe::openDialog and fe::openQuestion open them (sub_444c20, sub_4446d0), with what they
 * answer. */
x86::reg32 openDialogAnswer(Guest& guest, const fe::Dialog& dialog)
{
    return fe::direct<0x444c20, frontend::dialogs::openEdit>(
        guest.app(), guest.cpu(), dialog.lineCount, dialog.lines, dialog.buttonCount, dialog.buttons, dialog.focus,
        dialog.edit, dialog.editLength, dialog.editMode, dialog.callback);
}

x86::reg32 openQuestionAnswer(Guest& guest, const fe::Question& question)
{
    return fe::direct<0x4446d0, frontend::dialogs::openQuestion>(
        guest.app(), guest.cpu(), question.lineCount, question.lines, question.buttonCount, question.buttons,
        question.buttonHelp, question.focus, question.column, question.callback);
}

/* Greyed out and gone, or the opposite. */
void retire(Item& item)
{
    item.setDisabled(true);
    item.setHidden(true);
}

void restore(Item& item)
{
    item.setDisabled(false);
    item.setHidden(false);
}

/* Whether what closed the screen is its button by this codelink: the item
 * selected, a button, that one. */
bool closedBy(Guest& guest, CPU& cpu, x86::reg32 menu, const GuestString& codelink)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    const x86::reg32 item = fe::item::kTable + fe::item::kSize * x86::reg32(first + selected);
    return guest.dword(item + fe::item::kType) == kButton
        && *fe::direct<0x4429c0, port::menus::codelinkMatched>(guest.app(), cpu, cpu.esp - 4,
                                                               guest.dword(item + fe::item::kCodelink),
                                                               codelink.address, 0) != 0;
}

/* The car a player looks from is a cop's: 1, or 0. */
x86::reg32 playerIsCop(Guest& guest, CPU& cpu, x86::reg32 player)
{
    const x86::reg32 car = fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), cpu, player);
    return (guest.byte(car + kCarFlags) & kCarIsCop) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * The saves: whether a file stands, and a game written
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kSavesFolder = 0x7a2f34;
constexpr std::uint32_t kSaveImageSize = 0x20f0;
constexpr std::uint32_t kReserveFlags = 0x10;

constexpr GuestString kSeriesPath{ 0x538bdc, "%s%s.kot" };
constexpr GuestString kNoName{ 0x538be8, "" };
constexpr GuestString kTournamentPath{ 0x538c04, "%s%s.trn" };
constexpr GuestString kKeptPath{ 0x538bf0, "%s%s.%s" };
constexpr GuestString kGhostExtension{ 0x538bec, "gst" };
constexpr GuestString kTournamentExtension{ 0x538c10, "trn" };
constexpr GuestString kReplayExtension{ 0x538c14, "rp3" };

}  // namespace

/* sub_446510: a series' game by this name stands in the saves' folder. */
x86::reg32 frontend::pause::seriesSaveExists(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 + 0x100);
    const x86::reg32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), path, kSeriesPath.address, guest::Words{kSavesFolder, name});
    return *fe::direct<0x4e0e60, ea::fileThere>(app, cpu, path) ? 1 : 0;
}

namespace
{

/* sub_446740, sub_446950, sub_446a90: a save of this kind by this name stands. */
x86::reg32 keptExists(Guest& guest, CPU& cpu, x86::reg32 name, const GuestString& extension)
{
    const x86::reg32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), path, kKeptPath.address, guest::Words{kSavesFolder, name, extension.address});
    return *fe::direct<0x4e0e60, ea::fileThere>(guest.app(), cpu, path) ? 1 : 0;
}

/* sub_446550, sub_4468c0: the game written under this name: a copy of it, its
 * cars' names filled in, to the file.  1 when it went out. */
x86::reg32 writeSave(App* app, CPU& cpu, Guest& guest, x86::reg32 name, const GuestString& pathFormat)
{
    const x86::reg32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), path, pathFormat.address, guest::Words{kSavesFolder, name});
    const x86::reg32 copy =
        *fe::direct<0x4e1620, ea::reserved>(app, cpu, kNoName.address, kSaveImageSize, kReserveFlags);
    if (copy == 0)
        return 0;
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, copy, SETTINGS_ADDRESS(fileSize), kSaveImageSize);
    x86::reg32 written = 0;
    if (guest::direct<0x478010, frontend::pause::checkSaveable>(app, cpu, copy) != 0)
    {
        fe::direct<0x4eeae0, load::blockSaved>(app, cpu, path, copy, kSaveImageSize);
        written = 1;
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, copy);
    return written;
}

// A save's cars, by number, and the room for their names.
constexpr std::uint32_t kCarNameDwords = 4;
constexpr x86::reg32 kSavedFirstCar = 0x177c;
constexpr x86::reg32 kSavedFirstCarName = 0x1780;
constexpr x86::reg32 kSavedSecondCar = 0x17e8;
constexpr x86::reg32 kSavedSecondCarName = 0x17ec;
constexpr x86::reg32 kSavedThirdCar = 0x1994;      // counted from 4; below that none
constexpr x86::reg32 kSavedThirdCarName = 0x1998;

void copyCarName(Guest& guest, x86::reg32 to, x86::reg32 from)
{
    for (std::uint32_t i = 0; i < kCarNameDwords; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

}  // namespace

/* sub_478010: a save about to be written is one, and each of its cars has a
 * name: filled in.  1 when all of it held. */
x86::reg32 frontend::pause::checkSaveable(App* app, CPU& cpu, reg32 save)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    if (guest.dword(save) != kSaveImageSize)
        return 0;
    const x86::reg32 first = *fe::direct<0x439520, load::idOf>(app, cpu, guest.dword(save + kSavedFirstCar));
    if (first == 0)
        return 0;
    copyCarName(guest, save + kSavedFirstCarName, first);
    const x86::reg32 second = *fe::direct<0x439520, load::idOf>(app, cpu, guest.dword(save + kSavedSecondCar));
    if (second == 0)
        return 0;
    copyCarName(guest, save + kSavedSecondCarName, second);
    const std::int32_t third = std::int32_t(guest.dword(save + kSavedThirdCar));
    if (third >= 4)
    {
        const x86::reg32 name = *fe::direct<0x439520, load::idOf>(app, cpu, x86::reg32(third - 4));
        if (name == 0)
            return 0;
        copyCarName(guest, save + kSavedThirdCarName, name);
    }
    return 1;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The saves: the name asked for, and whether to write over what stands
 * ------------------------------------------------------------------------- */

// The kind being saved, what writes it and what looks for it.
constexpr x86::reg32 kSaveKind = 0x5591e8;
constexpr x86::reg32 kWriteSave = 0x5591cc;
constexpr x86::reg32 kSaveExists = 0x5591d0;
constexpr x86::reg32 kSaveName = 0x5591d4;
constexpr std::uint32_t kSaveNameLength = 0xa;
// The name dialog's texts.
constexpr x86::reg32 kNameButtons = 0x6611d4;      // char*[2]
constexpr x86::reg32 kNameQuestion = 0x6611dc;     // char*[1]
// How it went, and the question whether to write over a file.
constexpr x86::reg32 kOverwriteResult = 0x6611b8;  // char*[2]: the name, a line
constexpr x86::reg32 kWrittenResult = 0x6611c0;    // char*[2]: a line, the name
constexpr x86::reg32 kOverwriteQuestion = 0x6611c8;  // char*[1]
constexpr x86::reg32 kOverwriteButtons = 0x6611cc;   // char*[2]

constexpr std::uint32_t kTextSeriesName = 0x3d;
constexpr std::uint32_t kTextTournamentName = 0x3e;
constexpr std::uint32_t kTextGhostName = 0x3f;
constexpr std::uint32_t kTextReplayName = 0x40;
constexpr std::uint32_t kTextOverwrite = 0x3c;
constexpr std::uint32_t kTextSaved = 0x42;
constexpr std::uint32_t kTextNotSaved = 0x43;
constexpr std::uint32_t kTextOk = 0xf9;
constexpr std::uint32_t kTextYes = 0xc4;
constexpr std::uint32_t kTextNo = 0xc5;
constexpr std::uint32_t kTextCancel = 0xc6;
constexpr std::uint32_t kResultTime = 0x100;

constexpr x86::reg32 kWriteSeries = 0x446550, kSeriesExists = 0x446510;
constexpr x86::reg32 kWriteTournament = 0x4468c0, kTournamentExists = 0x446950;
constexpr x86::reg32 kWriteGhost = 0x446780, kGhostExists = 0x446740;
constexpr x86::reg32 kWriteReplay = 0x446a10, kReplayExists = 0x446a90;

struct SaveKind
{
    std::uint32_t text;
    x86::reg32 write;
    x86::reg32 exists;
};
constexpr SaveKind kSaveKinds[] = {
    { kTextSeriesName, kWriteSeries, kSeriesExists },
    { kTextTournamentName, kWriteTournament, kTournamentExists },
    { kTextReplayName, kWriteReplay, kReplayExists },
    { kTextGhostName, kWriteGhost, kGhostExists },
};

}  // namespace

/* sub_447a30: the name to save under asked for, the player's own at first; a
 * kind past the four leaves what the last one chose. */
x86::reg32 frontend::pause::askSaveName(App* app, CPU& cpu, reg32 kind)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    guest.setDword(kSaveKind, kind);
    if (kind < std::size(kSaveKinds))
    {
        guest.setDword(kNameQuestion, fe::text(guest, kSaveKinds[kind].text));
        guest.setDword(kWriteSave, kSaveKinds[kind].write);
        guest.setDword(kSaveExists, kSaveKinds[kind].exists);
    }
    guest.setDword(kNameButtons, fe::text(guest, kTextOk));
    guest.setDword(kNameButtons + 4, fe::text(guest, kTextCancel));
    guest.copyString(kSaveName, SETTINGS_ADDRESS(players[0].name));
    return openDialogAnswer(guest, { 1, kNameQuestion, 2, kNameButtons, 0, kSaveName, kSaveNameLength, 0, kSaveNameAnswered });
}

/* sub_447800, the question whether to write over the file answered: Yes writes
 * and tells how it went, No and Escape ask for another name. */
x86::reg32 frontend::pause::overwriteAnswered(App* app, CPU& cpu, reg32 answer, reg32 keepOpen)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(keepOpen, 0);
    if (answer != fe::kDialogEscape && answer != fe::kDialogEnter)
        return 0;
    if (answer == fe::kDialogEscape || focus == 1)
    {
        guest::direct<0x447a30, askSaveName>(app, cpu, guest.dword(kSaveKind));
        guest.setByte(keepOpen, 0);
        return 0;
    }
    if (focus != 0)
        return 0;
    const bool written = callAt(guest, guest.dword(kWriteSave), fe::args({ kSaveName })) != 0;
    guest.setDword(kOverwriteResult, kSaveName);
    guest.setDword(kOverwriteResult + 4, fe::text(guest, written ? kTextSaved : kTextNotSaved));
    fe::direct<0x444a80, frontend::dialogs::openMessage>(app, cpu, x86::reg32(2), kOverwriteResult, kResultTime,
                                                         answer, x86::reg32(0));
    guest.setByte(keepOpen, 0);
    return 0;
}

/* sub_4478c0, the name dialog answered: a name that stands asks whether to
 * write over it, a new one is written at once. */
x86::reg32 frontend::pause::saveNameAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 4);
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(close, 0);
    const x86::reg32 typed = fe::dialogEditText(guest);
    if (answer == fe::kDialogEscape || (answer == fe::kDialogEnter && focus == 1))
    {
        guest.setByte(close, 1);
        return 0;
    }
    if (answer != fe::kDialogEnter || focus != 0 || typed == 0 || guest.byte(typed) == 0)
        return 0;
    if (callAt(guest, guest.dword(kSaveExists), fe::args({ typed })) == 0)
    {
        guest.copyString(kSaveName, typed);
        const bool written = callAt(guest, guest.dword(kWriteSave), fe::args({ typed })) != 0;
        const x86::reg32 line = fe::text(guest, written ? kTextSaved : kTextNotSaved);
        guest.setDword(kWrittenResult, line);
        guest.setDword(kWrittenResult + 4, kSaveName);
        fe::direct<0x444a80, frontend::dialogs::openMessage>(app, cpu, x86::reg32(2), kWrittenResult, kResultTime,
                                                             answer, x86::reg32(0));
    }
    else
    {
        guest.setDword(kOverwriteQuestion, fe::text(guest, kTextOverwrite));
        guest.setDword(kOverwriteButtons, fe::text(guest, kTextYes));
        guest.setDword(kOverwriteButtons + 4, fe::text(guest, kTextNo));
        guest.copyString(kSaveName, typed);
        fe::openQuestion(guest, { 1, kOverwriteQuestion, 2, kOverwriteButtons, 0, 0, 0, kOverwriteAnswered });
    }
    guest.setByte(close, 0);
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * pause: what it and the display's screens hide
 * ------------------------------------------------------------------------- */

constexpr GuestString kHideMe[] = {
    { 0x5390ac, "HideMe" },  { 0x5390b4, "HideMeToo" }, { 0x5390c0, "HideMe3" },
    { 0x5390c8, "HideMe4" }, { 0x5390d0, "HideMe5" },   { 0x5390d8, "HideMe6" },
};
// A racer's instruments, and a cop's: part of them the two have in common by
// name but for "Cop", part only his.
constexpr GuestString kRacerInstruments[] = {
    { 0x5390e0, "mphkphButton" },       { 0x5390f0, "mphkphText" },
    { 0x5390fc, "tachomButton" },       { 0x53910c, "tachomText" },
    { 0x539118, "mapButton" },          { 0x539124, "mapText" },
    { 0x53912c, "splittimeButton" },    { 0x53913c, "splittimeText" },
    { 0x53914c, "opponentNameButton" }, { 0x539160, "opponentNameText" },
    { 0x539174, "opponentListButton" }, { 0x539188, "opponentListText" },
    { 0x53919c, "mirrorButton" },       { 0x5391ac, "mirrorText" },
};
constexpr GuestString kCopGreyed[] = {
    { 0x5391b8, "splittimeButtonCop" },    { 0x5391cc, "splittimeTextCop" },
    { 0x5391e0, "opponentNameButtonCop" }, { 0x5391f8, "opponentNameTextCop" },
};
constexpr GuestString kCopInstrumentsFirst[] = {
    { 0x53920c, "mphkphButtonCop" }, { 0x53921c, "mphkphTextCop" },
    { 0x53922c, "tachomButtonCop" }, { 0x53923c, "tachomTextCop" },
    { 0x53924c, "mapButtonCop" },    { 0x53925c, "mapTextCop" },
};
constexpr GuestString kCopInstrumentsLast[] = {
    { 0x539268, "opponentListButtonCop" }, { 0x539280, "opponentListTextCop" },
    { 0x539294, "mirrorButtonCop" },       { 0x5392a4, "mirrorTextCop" },
};

}  // namespace

/* sub_44c2d0: the six spare items hidden, as far as the screen has them.  The
 * last one found, or 0. */
x86::reg32 frontend::pause::hideSpare(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    x86::reg32 last = 0;
    for (const GuestString& codelink : kHideMe)
    {
        Item item = fe::findItem(guest, menu, codelink);
        last = item.address();
        if (!item)
            break;
        item.setHidden(true);
    }
    return last;
}

/* sub_44c360: a cop's display: a racer's instruments hidden, what a cop cannot
 * have greyed out. */
x86::reg32 frontend::pause::hideForCop(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    x86::reg32 last = 0;
    for (const GuestString& codelink : kRacerInstruments)
        last = hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopGreyed)
        last = disableItem(guest, menu, codelink);
    return last;
}

/* sub_44c4e0: a racer's display: a cop's items hidden. */
x86::reg32 frontend::pause::hideForRacer(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    x86::reg32 last = 0;
    for (const GuestString& codelink : kCopInstrumentsFirst)
        last = hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopGreyed)
        last = hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopInstrumentsLast)
        last = hideItem(guest, menu, codelink);
    return last;
}

namespace
{

/* ---------------------------------------------------------------------------
 * pause: its questions
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kLeaveTo = 0x559268;          // what the Yes hands the menu loop
constexpr std::uint32_t kAnswerRestart = 6;

/* sub_44c610, sub_44c670, sub_4b67b0, a yes or a no: Yes hands the menu loop
 * what `yes` reads, No and Escape close the box. */
template <typename Yes>
x86::reg32 yesOrNoAnswered(Guest& guest, x86::reg32 answer, x86::reg32 close, Yes yes)
{
    const auto closeBox = [&](std::uint8_t value) {
        if (close != 0)
            guest.setByte(close, value);
    };
    if (answer == fe::kDialogEscape)
    {
        closeBox(1);
        return fe::kDialogHandled;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    const std::uint32_t focus = fe::dialogFocus(guest);
    if (focus == 0)
    {
        closeBox(1);
        return yes();
    }
    closeBox(focus == 1 ? 1 : 0);
    return fe::kDialogHandled;
}

constexpr x86::reg32 kLeaveQuestion = 0x664410;    // char*[1]
constexpr x86::reg32 kLeaveButtons = 0x664414;     // char*[2]

}  // namespace

/* sub_44c6d0: "are you sure?" asked for a choice of the quit box, No first. */
x86::reg32 frontend::pause::askLeave(App* app, CPU& cpu, reg32 question, reg32 leaveTo)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest.setDword(kLeaveTo, leaveTo);
    guest.setDword(kLeaveQuestion, fe::text(guest, question));
    guest.setDword(kLeaveButtons, fe::text(guest, kTextYes));
    guest.setDword(kLeaveButtons + 4, fe::text(guest, kTextNo));
    return openQuestionAnswer(guest, { 1, kLeaveQuestion, 2, kLeaveButtons, 0, 1, 0, kAskLeaveAnswered });
}

namespace
{

// The quit box: each button's Yes and its question.
constexpr x86::reg32 kQuitButtonAnswer = 0x662c88; // int[4]; kQuitJustClose closes
constexpr x86::reg32 kQuitButtonQuestion = 0x662c70;
constexpr std::uint32_t kQuitJustClose = 2;
constexpr x86::reg32 kQuitQuestion = 0x66441c;     // char*[1]
constexpr x86::reg32 kQuitButtons = 0x664420;      // char*[up to 4]
constexpr x86::reg32 kNetworkReplayOff = 0x7d36b4;

constexpr std::uint32_t kTextQuit = 0x1db;
constexpr std::uint32_t kTextQuitToPostGame = 0x1dc;
constexpr std::uint32_t kTextQuitToMain = 0x1dd;
constexpr std::uint32_t kTextQuitSplitScreen = 0x1df;
constexpr std::uint32_t kTextQuitToSystem = 0x1ef;
constexpr std::uint32_t kAskQuitToSystem = 0xfc;
constexpr std::uint32_t kAskQuitNetwork = 0x100;
constexpr std::uint32_t kAskQuitSplitScreen = 0x101;
// Are you sure about the race, of its three kinds?
constexpr std::uint32_t kAskQuitRace = 0xfd;
constexpr std::uint32_t kAskQuitSeries = 0xfe;
constexpr std::uint32_t kAskQuitTournament = 0xff;
// What a Yes hands the menu loop.
constexpr std::uint32_t kLeaveToPostGame = 0xc;
constexpr std::uint32_t kLeaveToMain = 0xb;
constexpr std::uint32_t kLeaveToSystem = 0xa;

/* The question to ask before leaving this race, by its kind. */
std::uint32_t askQuitRace(Guest& guest)
{
    const std::uint32_t type = gameSettings(guest).raceType;
    return type == 1 ? kAskQuitSeries : type == 2 ? kAskQuitTournament : kAskQuitRace;
}

}  // namespace

/* sub_44c730, the quit box answered: its button's question, or Cancel. */
x86::reg32 frontend::pause::quitChoiceAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const auto closeBox = [&](std::uint8_t value) {
        if (close != 0)
            guest.setByte(close, value);
    };
    x86::reg32 result = 0;
    if (answer == fe::kDialogEscape)
    {
        closeBox(1);
        result = fe::kDialogHandled;
    }
    else if (answer == fe::kDialogEnter)
    {
        result = fe::kDialogHandled;
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (std::int32_t(focus) < 0)
        {
            closeBox(0);
        }
        else if (guest.dword(kQuitButtonAnswer + 4 * focus) == kQuitJustClose)
        {
            closeBox(1);
        }
        else
        {
            guest::direct<0x44c6d0, askLeave>(app, cpu, guest.dword(kQuitButtonQuestion + 4 * focus),
                                              guest.dword(kQuitButtonAnswer + 4 * focus));
            closeBox(0);
        }
    }
    return result;
}

/* sub_44c7b0, Quit clicked on pause: where to, by what kind of a race it is. */
x86::reg32 frontend::pause::onQuitClicked(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const bool splitScreen = std::int32_t(gameSettings(guest).playMode) > 1 && guest.word(kNetworkHost) == 0;
    const bool network = std::int32_t(gameSettings(guest).replayMode) >= 2;
    guest.setDword(kQuitQuestion, fe::text(guest, kTextQuit));
    if (network || splitScreen)
    {
        const std::uint32_t first = !network ? kTextQuitSplitScreen
            : guest.dword(kNetworkReplayOff) == 0 ? kTextQuitToPostGame : kTextQuitToMain;
        guest.setDword(kQuitButtons, fe::text(guest, first));
        guest.setDword(kQuitButtons + 4, fe::text(guest, kTextQuitToSystem));
        guest.setDword(kQuitButtons + 8, fe::text(guest, kTextCancel));
        guest.setDword(kQuitButtonAnswer, network ? kLeaveToPostGame : kLeaveToMain);
        guest.setDword(kQuitButtonAnswer + 4, kLeaveToSystem);
        guest.setDword(kQuitButtonAnswer + 8, kQuitJustClose);
        guest.setDword(kQuitButtonQuestion, network ? kAskQuitNetwork : kAskQuitSplitScreen);
        guest.setDword(kQuitButtonQuestion + 4, kAskQuitToSystem);
        fe::openQuestion(guest, { 1, kQuitQuestion, 3, kQuitButtons, 0, 2, 1, kQuitChoiceAnswered });
    }
    else
    {
        const std::uint32_t ask = askQuitRace(guest);
        guest.setDword(kQuitButtons, fe::text(guest, kTextQuitToMain));
        guest.setDword(kQuitButtons + 4, fe::text(guest, kTextQuitToPostGame));
        guest.setDword(kQuitButtons + 8, fe::text(guest, kTextQuitToSystem));
        guest.setDword(kQuitButtons + 12, fe::text(guest, kTextCancel));
        guest.setDword(kQuitButtonAnswer, kLeaveToMain);
        guest.setDword(kQuitButtonAnswer + 4, kLeaveToPostGame);
        guest.setDword(kQuitButtonAnswer + 8, kLeaveToSystem);
        guest.setDword(kQuitButtonAnswer + 12, kQuitJustClose);
        guest.setDword(kQuitButtonQuestion, ask);
        guest.setDword(kQuitButtonQuestion + 4, ask);
        guest.setDword(kQuitButtonQuestion + 8, kAskQuitToSystem);
        fe::openQuestion(guest, { 1, kQuitQuestion, 4, kQuitButtons, 0, 3, 1, kQuitChoiceAnswered });
    }
    return fe::kDialogHandled;
}

namespace
{

constexpr x86::reg32 kRestartQuestion = 0x664434;  // char*[1]
constexpr x86::reg32 kRestartButtons = 0x664438;   // char*[2]
constexpr std::uint32_t kTextRestart = 0x1ea;

}  // namespace

/* sub_44ca10, Restart clicked. */
x86::reg32 frontend::pause::onRestartClicked(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest.setDword(kRestartQuestion, fe::text(guest, kTextRestart));
    guest.setDword(kRestartButtons, fe::text(guest, kTextYes));
    guest.setDword(kRestartButtons + 4, fe::text(guest, kTextNo));
    fe::openQuestion(guest, { 1, kRestartQuestion, 2, kRestartButtons, 0, 0, 0, kRestartAnswered });
    return fe::kDialogHandled;
}

/* sub_44ca80, Save clicked: the name asked for, of the kind of save the race's
 * type keeps. */
x86::reg32 frontend::pause::onSaveClicked(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    const std::uint32_t type = gameSettings(guest).raceType;
    if (type == 1)
        guest::direct<0x447a30, askSaveName>(app, cpu, x86::reg32(1));
    else if (type == 2)
        guest::direct<0x447a30, askSaveName>(app, cpu, x86::reg32(0));
    return fe::kDialogHandled;
}

namespace
{

/* ---------------------------------------------------------------------------
 * pause: as it opens
 * ------------------------------------------------------------------------- */

constexpr GuestString kSaveGame{ 0x5392b4, "SAVE_GAME" };
constexpr GuestString kPauseTitle{ 0x5392c0, "pstitle" };
constexpr GuestString kTitleDash{ 0x5392c8, " - " };
constexpr GuestString kQuitButton{ 0x5392cc, "PSE_QUITBUTTON" };
constexpr GuestString kAdvertMenu{ 0x5392dc, "ad" };
constexpr GuestString kLaunchCarEdit{ 0x5392e0, "PSE_LAUNCHCAREDIT" };
constexpr GuestString kAiTuner{ 0x5392f4, "PSE_AITUNER" };
constexpr GuestString kHud{ 0x539300, "Hud" };
constexpr GuestString kHudFirst{ 0x539304, "Hud_p1" };
constexpr GuestString kHudSecond{ 0x53930c, "Hud_p2" };
constexpr GuestString kControlsButton{ 0x539314, "PS_CONTROLS" };
constexpr GuestString kRestartButton{ 0x539320, "PSE_RESTART" };

constexpr x86::reg32 kPauseTitleText = 0x55926c;
constexpr x86::reg32 kFirstViewRect = 0x7cdac0;
constexpr std::int32_t kWidthForControls = 0x280;
constexpr std::uint16_t kSaveButtonSplitY = 0x104;
constexpr std::uint32_t kNetworkJoined = 2;

constexpr std::uint32_t kTextSaveSeries = 0x490;
constexpr std::uint32_t kTextSaveTournament = 0x491;
constexpr std::uint32_t kTextPaused = 0x1ba;
constexpr std::uint32_t kTextPausedOtherRace = 0x104;

/* The screen's title: the kind of race and "paused", or a word for all others. */
void fillPauseTitle(Guest& guest)
{
    static constexpr std::uint32_t kKinds[] = { 0xd5, 0xd6, 0xd7, 0xdc };
    const std::uint32_t type = gameSettings(guest).raceType;
    if (type >= std::size(kKinds))
    {
        guest.copyString(kPauseTitleText, fe::text(guest, kTextPausedOtherRace));
        return;
    }
    guest.copyString(kPauseTitleText, fe::text(guest, kKinds[type]));
    guest.copyString(kPauseTitleText + guest.length(kPauseTitleText), kTitleDash.address);
    const x86::reg32 paused = fe::text(guest, kTextPaused);
    guest.copyString(kPauseTitleText + guest.length(kPauseTitleText), paused);
}

}  // namespace

/* sub_44cab0, pause as it opens. */
x86::reg32 frontend::pause::onPauseEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0xc);
    // A view's rectangle comes back into these two; its width is what counts.
    const x86::reg32 rectOther = guest.frame();
    const x86::reg32 rectWidth = guest.frame() + 4;
    guest.setDword(rectOther, 0);
    guest.setDword(rectWidth, 0);
    dim(guest, cpu);

    if (Item save = fe::findItem(guest, menu, kSaveGame))
    {
        if (gameSettings(guest).playMode != 1)
            guest.setWord(save.address() + fe::item::kY, kSaveButtonSplitY);
        const std::uint32_t type = gameSettings(guest).raceType;
        if (std::int32_t(gameSettings(guest).playMode) <= 1 && gameSettings(guest).replayMode == 0 && (type == 1 || type == 2))
        {
            const x86::reg32 line = fe::text(guest, type == 1 ? kTextSaveSeries : kTextSaveTournament);
            save.setOnClick(kOnSaveClicked);
            save.setText(line);
        }
        else
        {
            retire(save);
        }
    }

    if (Item title = fe::findItem(guest, menu, kPauseTitle))
    {
        fillPauseTitle(guest);
        title.setText(kPauseTitleText);
    }

    // The developer's Quit goes to the advert's menu with no questions asked.
    // That branch does not look whether the screen has the item.
    const bool developer = (guest.byte(kDeveloperFlags) & kDeveloperQuit) != 0;
    Item quit = fe::findItem(guest, menu, kQuitButton);
    if (developer)
        guest.setDword(quit.address() + fe::item::kNextMenu, kAdvertMenu.address);
    else if (quit)
        quit.setOnClick(kOnQuitClicked);

    hideItem(guest, menu, kLaunchCarEdit);
    hideItem(guest, menu, kAiTuner);

    if (gameSettings(guest).playMode == 1)
    {
        hideItem(guest, menu, kHud);
        showItem(guest, menu, kHudFirst);
        showItem(guest, menu, kHudSecond);
    }
    else
    {
        showItem(guest, menu, kHud);
        hideItem(guest, menu, kHudFirst);
        hideItem(guest, menu, kHudSecond);
    }

    if (Item controls = fe::findItem(guest, menu, kControlsButton))
    {
        guest::direct<0x4beb20, readRect>(app, cpu, kFirstViewRect, rectOther, rectOther, rectWidth, rectOther);
        controls.setDisabled(gameSettings(guest).playMode == 1 && std::int32_t(guest.dword(rectWidth)) < kWidthForControls);
    }

    if (Item restart = fe::findItem(guest, menu, kRestartButton))
    {
        const std::int32_t players = std::int32_t(gameSettings(guest).playMode);
        restart.setOnClick(kOnRestartClicked);
        if (players >= 1 && (guest.word(kNetworkHost) == 0 || guest.dword(kNetworkJoin) == kNetworkJoined))
            retire(restart);
        if (std::int32_t(gameSettings(guest).replayMode) >= 2)
            restore(restart);
    }

    guest::direct<0x44c2d0, hideSpare>(app, cpu, menu);
    return 0;
}

/* ---------------------------------------------------------------------------
 * The sliders of the graphics and the display
 * ------------------------------------------------------------------------- */

/* sub_44cf70, the effects' intensity moved. */
x86::reg32 frontend::pause::intensityMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    // The 0xf the game hands the shown test in edx is not read by it.
    const x86::reg32 shown = fe::direct<0x44a660, frontend::loop::shownNow>(app, cpu, item);
    if (shown == 0)
        return shown;
    gameSettings(guest).alphaIntensity = float(fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, cpu, item, x86::reg32(0)));
    // eax as the slider's value leaves it: the slider, which its code keeps there.
    return item;
}

namespace
{

/* sub_44cfa0 and the five after it, the map's size moved: the slider from 0
 * to 1, the size from 1 to 4. */
x86::reg32 mapSizeMoved(Guest& guest, CPU& cpu, x86::reg32 item, x86::reg32 scale, float& size)
{
    // As intensityMoved: the 0xf in edx not read, eax as the slider's value leaves it (the slider).
    const x86::reg32 shown = fe::direct<0x44a660, frontend::loop::shownNow>(guest.app(), cpu, item);
    if (shown == 0)
        return shown;
    const x86::Float value = fe::direct<0x46c9b0, frontend::glue::sliderValue>(guest.app(), cpu, item, x86::reg32(0));
    FloatMath x87(guest);
    size = float(x87.add(x87.mul(value, x87.load(scale)), x86::Float(1.0)));
    return item;
}

/* ---------------------------------------------------------------------------
 * psoption, pscntl1
 * ------------------------------------------------------------------------- */

constexpr GuestString kAudioButton{ 0x539344, "AudioB" };
constexpr GuestString kForceFeedback{ 0x53934c, "CTRL1_FORCEFEED" };
constexpr GuestString kDeadZone{ 0x53935c, "CTRL1_DEADZONE" };
constexpr GuestString kCancelButton{ 0x53936c, "cancel" };

// The players' controls, as pscntl1 found them.
constexpr x86::reg32 kControlsKept = 0x5f3cd0;
constexpr std::uint32_t kControlsBothSize = 0x68;
constexpr std::uint32_t kControlsOneSize = 0x34;
constexpr x86::reg32 kControlsDone = 0x66440c;     // not Cancel
constexpr std::uint8_t kControllerForce = 0x08;

}  // namespace

/* sub_44d120, psoption and psoptn2 as they open. */
x86::reg32 frontend::pause::onOptionsEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    dim(guest, cpu);
    if (Item audio = fe::findItem(guest, menu, kAudioButton))
    {
        if (gameSettings(guest).installIsRemote != 0)
            audio.setDisabled(true);
    }
    return 0;
}

/* sub_44d170, Cancel's key on pscntl1: Enter on it is what cancels. */
x86::reg32 frontend::pause::cancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (key == 0xd)
        guest.setDword(kControlsDone, 0);
        return x86::reg32(*fe::direct<kButtonKey, port::buttons::buttonKeyed>(
        app, cpu, cpu.esp - 4, item, x86::reg32(std::int32_t(std::int16_t(key)))));
}

/* sub_44d190, pscntl1 as it opens: the controls kept for Cancel. */
x86::reg32 frontend::pause::onControlsEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    dim(guest, cpu);
    guest.setDword(kControlsDone, 1);
    const x86::reg32 controls = fe::direct<0x4904d0, controls::controlSets>(app, cpu, gameSettings(guest).playMode);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, kControlsKept, controls, kControlsBothSize);
    Item force = fe::findItem(guest, menu, kForceFeedback);
    if (!(std::uint8_t(gameSettings(guest).devices[0].kind) & kControllerForce) && force)
        force.setDisabled(true);
    Item deadZone = fe::findItem(guest, menu, kDeadZone);
    if (gameSettings(guest).devices[0].kind == 0 && deadZone)
        deadZone.setDisabled(true);
    if (Item cancel = fe::findItem(guest, menu, kCancelButton))
        guest.setDword(cancel.address() + kItemOnKey, kCancelKey);
    return 0;
}

/* sub_44d220, pscntl1 as it closes: the old controls back on Cancel, the new
 * ones taken up otherwise. */
x86::reg32 frontend::pause::onControlsExit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const x86::reg32 controls = fe::direct<0x4904d0, controls::controlSets>(app, cpu, gameSettings(guest).playMode);
    if (guest.dword(kControlsDone) == 0)
    {
        const std::uint32_t size = gameSettings(guest).playMode == 1 ? kControlsBothSize : kControlsOneSize;
        fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, controls, kControlsKept, size);
    }
    else
    {
        fe::direct<0x4710f0, controls::forceCarFound>(app, cpu);
    }
    fe::direct<0x48f1f0, controls::playersBound>(app, cpu);
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * psgraph
 * ------------------------------------------------------------------------- */

constexpr GuestString kPerspective{ 0x539374, "GRAPH_PERSP" };
constexpr GuestString kTrackLightingText{ 0x539380, "GRAPH_TRACKLIGHTING_TEXT" };
constexpr GuestString kTrackLighting{ 0x53939c, "GRAPH_TRACKLIGHTING" };
constexpr GuestString kWidescreenButton{ 0x5393b0, "WIDESCREEN_BUTTON" };
constexpr GuestString kCarShade{ 0x5393c4, "CARSHADE" };
constexpr GuestString kIntensitySlider{ 0x5393d0, "SFX_INTENSITY" };
constexpr GuestString kZBuffer{ 0x5393e0, "Z_BUFFER" };
constexpr GuestString kProjectedButton{ 0x5393ec, "PROJ_TEX" };
constexpr GuestString kGraphicsCancel{ 0x5393f8, "PSGRAPH_CANCEL" };

}  // namespace

/* sub_44d270, the screen's shape clicked: the views laid out again, for the
 * first player's kind of car. */
x86::reg32 frontend::pause::onWidescreenClicked(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    fe::direct<0x4be5e0, frontend::loopb::viewsLayout>(app, cpu, playerIsCop(guest, cpu, 0));
    return fe::kDialogHandled;
}

/* sub_44d2a0, psgraph as it opens: the settings kept for Cancel, what the card
 * cannot do greyed out.  Port: the original greys the intensity out on a card
 * without alpha (a probe of 0x7a3a58); here every card has it. */
x86::reg32 frontend::pause::onGraphicsEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    dim(guest, cpu);
    if (!(guest.byte(kDisplayFlags) & kTrackLightingFlags))
    {
        disableItem(guest, menu, kPerspective);
        if (Item text = fe::findItem(guest, menu, kTrackLightingText))
            retire(text);
        disableItem(guest, menu, kTrackLighting);
    }
    if (Item wide = fe::findItem(guest, menu, kWidescreenButton))
    {
        if (gameSettings(guest).playMode == 1)
            wide.setDisabled(true);
        else
            wide.setOnClick(kOnWidescreenClicked);
    }
    if (Item shade = fe::findItem(guest, menu, kCarShade))
    {
        if (guest.byte(kDisplayFlags) & kTrackLightingFlags)
            shade.setDisabled(true);
    }
    if (Item intensity = fe::findItem(guest, menu, kIntensitySlider))
    {
        FloatMath x87(guest);
        guest.setFloat(intensity.address() + kSliderValue, float(x86::Float(gameSettings(guest).alphaIntensity)));
        fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, cpu, intensity.address(), kIntensityMoved);
    }
    if (Item zBuffer = fe::findItem(guest, menu, kZBuffer))
        zBuffer.setDisabled(guest.dword(kDisplayBuffering) == 0);
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0);
    return 0;
}

/* sub_44d3e0: the projected textures' button by what is on now. */
x86::reg32 frontend::pause::fitProjected(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0 || gameSettings(guest).projectedText == 0);
    return 0;
}

/* sub_44d420, psgraph as it closes: Cancel puts the settings back, and the
 * views laid out again if that changes the screen's shape. */
x86::reg32 frontend::pause::onGraphicsExit(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (!closedBy(guest, cpu, menu, kGraphicsCancel))
        return 0;
    const bool shapeChanged = guest.dword(kScreenSizeKept) != gameSettings(guest).wideScreen;
    copySettings(guest, SETTINGS_ADDRESS(language), kSettingsKept);
    if (shapeChanged)
        fe::direct<0x4be5e0, frontend::loopb::viewsLayout>(app, cpu, playerIsCop(guest, cpu, 0));
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * pshud, pshud1p, pshud2p
 * ------------------------------------------------------------------------- */

constexpr GuestString kMapScale{ 0x539408, "MAP_SCALE" };
constexpr GuestString kPosition{ 0x539414, "POSITION" };
constexpr GuestString kHudCancel{ 0x53943c, "PSHUD_CANCEL" };
// Where POSITION leads: one player's next screen, a cop or a racer, then two's,
// by the first's kind and the second's.
constexpr GuestString kNextAloneCop{ 0x539420, "ps1pcop" };
constexpr GuestString kNextAloneRacer{ 0x539428, "ps1prace" };
constexpr GuestString kNextCopCop{ 0x53944c, "ps2p2c" };
constexpr GuestString kNextCopRacer{ 0x539454, "ps2p1c2r" };
constexpr GuestString kNextRacerCop{ 0x539460, "ps2p1r2c" };
constexpr GuestString kNextRacerRacer{ 0x53946c, "ps2p2r" };

constexpr x86::reg32 kHudShown = 0x791b1c;
constexpr x86::reg32 kFirstIsRacer = 0x677df0;
constexpr x86::reg32 kSecondIsRacer = 0x677df4;

/* Each of the three screens keeps, for each kind of car, four choices in the
 * settings -- 0 the last on its list, otherwise one past its place -- and, while
 * it is up, each one's place on its list.  Those of the first screen's racer are
 * here; the others' stand further on by a stride. */
constexpr x86::reg32 kHudPlacesStride = 8;
constexpr x86::reg32 kHudList1Places = 0x678828;
constexpr std::uint32_t kHudList1Last = 5;
constexpr x86::reg32 kHudList2Places = 0x678840;
constexpr std::uint32_t kHudList2Last = 4;
constexpr x86::reg32 kHudList3Places = 0x678810;
constexpr std::uint32_t kHudList3Last = 3;
constexpr x86::reg32 kHudList4Places = 0x6787f8;
constexpr std::uint32_t kHudList4Last = 2;

/* A screen's map slider: what it moves for each kind of car, what is told of
 * its moves, and the constants its value comes from. */
struct HudScreen
{
    x86::reg32 mapMoved[2];
    x86::reg32 mapBias;        // float, -1
    x86::reg32 mapScale;       // float, a third
};
constexpr HudScreen kHudScreens[] = {
    { { 0x44cfe0, 0x44cfa0 }, 0x539434, 0x539438 },
    { { 0x44d060, 0x44d020 }, 0x539474, 0x539478 },
    { { 0x44d0e0, 0x44d0a0 }, 0x53947c, 0x539480 },
};

x86::reg32 hudPlaces(x86::reg32 screen, x86::reg32 kind)
{
    return kHudPlacesStride * screen + 4 * kind;
}

/* A choice's place on its list, and a place's choice. */
std::uint32_t placeOf(std::uint32_t choice, std::uint32_t last)
{
    return choice != 0 ? choice - 1 : last;
}

std::uint32_t choiceOf(std::uint32_t place, std::uint32_t last)
{
    return place != last ? place + 1 : 0;
}

/* The three's opening past the kinds: the backdrop, the map's slider, the next
 * screen, and what does not apply to this kind of car hidden. */
void openHud(App* app, CPU& cpu, Guest& guest, x86::reg32 menu, x86::reg32 screen, x86::reg32 kind,
             x86::reg32 next)
{
    const HudScreen& hud = kHudScreens[screen];
    dim(guest, cpu);
    if (Item map = fe::findItem(guest, menu, kMapScale))
    {
        FloatMath x87(guest);
        const x86::Float biased = x87.add(x86::Float(gameSettings(guest).hudOf(screen, kind).mapScale), x87.load(hud.mapBias));
        guest.setFloat(map.address() + kSliderValue, float(x87.mul(biased, x87.load(hud.mapScale))));
        fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, cpu, map.address(), hud.mapMoved[kind]);
    }
    if (Item position = fe::findItem(guest, menu, kPosition))
        guest.setDword(position.address() + fe::item::kNextMenu, next);
    guest::direct<0x44c2d0, frontend::pause::hideSpare>(app, cpu, menu);
    if (kind != 0)
        guest::direct<0x44c360, frontend::pause::hideForCop>(app, cpu, menu);
    else
        guest::direct<0x44c4e0, frontend::pause::hideForRacer>(app, cpu, menu);
}

/* The three's lists set from the choices: all but the first list, which the
 * screens have their own ways with; the fourth choice was read as they opened. */
void placeHudLists(Guest& guest, x86::reg32 screen, x86::reg32 kind, std::uint32_t fourth)
{
    const HudRecord& choices = gameSettings(guest).hudOf(screen, kind);
    const x86::reg32 places = hudPlaces(screen, kind);
    guest.setDword(kHudList2Places + places, placeOf(choices.speedometerForm, kHudList2Last));
    guest.setDword(kHudList3Places + places, placeOf(choices.tachometerForm, kHudList3Last));
    guest.setDword(kHudList4Places + places, placeOf(fourth, kHudList4Last));
}

}  // namespace

/* sub_44d4c0, pshud as it opens. */
x86::reg32 frontend::pause::onHudEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const x86::reg32 kind = playerIsCop(guest, cpu, 0);
    const HudRecord& choices = gameSettings(guest).hudOf(0, kind);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, kind == 0 ? 1 : 0);
    openHud(app, cpu, guest, menu, 0, kind, kind != 0 ? kNextAloneCop.address : kNextAloneRacer.address);
    placeHudLists(guest, 0, kind, fourth);
    guest.setDword(kHudList1Places + hudPlaces(0, kind), placeOf(choices.splitTimeForm, kHudList1Last));
    return 0;
}

namespace
{

/* Where POSITION leads from the two players' screens. */
x86::reg32 nextOfTwo(x86::reg32 firstIsCop, x86::reg32 secondIsCop)
{
    if (firstIsCop != 0)
        return secondIsCop != 0 ? kNextCopCop.address : kNextCopRacer.address;
    return secondIsCop != 0 ? kNextRacerCop.address : kNextRacerRacer.address;
}

}  // namespace

/* sub_44d7a0, pshud1p as it opens: the first player's display. */
x86::reg32 frontend::pause::onHud1Enter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const x86::reg32 first = playerIsCop(guest, cpu, 0);
    const x86::reg32 second = playerIsCop(guest, cpu, 1);
    const HudRecord& choices = gameSettings(guest).hudOf(1, first);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(app, cpu, guest, menu, 1, first, nextOfTwo(first, second));
    placeHudLists(guest, 1, first, fourth);
    guest.setDword(kHudList1Places + hudPlaces(1, first), placeOf(choices.splitTimeForm, kHudList1Last));
    return 0;
}

/* sub_44dad0, pshud2p as it opens: the second player's.  Its first list's place
 * goes by its own choice's being set, but counts from pshud1p's: the original
 * has it so. */
x86::reg32 frontend::pause::onHud2Enter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const x86::reg32 first = playerIsCop(guest, cpu, 0);
    const x86::reg32 second = playerIsCop(guest, cpu, 1);
    const HudRecord& choices = gameSettings(guest).hudOf(2, second);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(app, cpu, guest, menu, 2, second, nextOfTwo(first, second));
    placeHudLists(guest, 2, second, fourth);
    const std::uint32_t place = choices.splitTimeForm != 0
        ? gameSettings(guest).hudOf(1, second).splitTimeForm - 1
        : kHudList1Last;
    guest.setDword(kHudList1Places + hudPlaces(2, second), place);
    return 0;
}

namespace
{

/* sub_44d670, sub_44d9a0, sub_44dcd0, a display's screen as it closes: its
 * lists' places back into choices for `player`'s kind of car, and Cancel puts
 * the settings back over them. */
x86::reg32 onHudExit(Guest& guest, CPU& cpu, x86::reg32 menu, x86::reg32 screen, x86::reg32 player)
{
    const x86::reg32 kind = playerIsCop(guest, cpu, player);
    HudRecord& choices = gameSettings(guest).hudOf(screen, kind);
    const x86::reg32 places = hudPlaces(screen, kind);
    choices.speedometerForm = choiceOf(guest.dword(kHudList2Places + places), kHudList2Last);
    choices.tachometerForm = choiceOf(guest.dword(kHudList3Places + places), kHudList3Last);
    choices.mapStyle = choiceOf(guest.dword(kHudList4Places + places), kHudList4Last);
    choices.splitTimeForm = choiceOf(guest.dword(kHudList1Places + places), kHudList1Last);
    fe::direct<0x4844c0, port::hud::hudPaneSetOf>(guest.app(), cpu, cpu.esp - 4,
                                                  port::Args{ player, 0, 0, 0 });
    if (closedBy(guest, cpu, menu, kHudCancel))
        copySettings(guest, SETTINGS_ADDRESS(language), kSettingsKept);
    return 0;
}

/* ---------------------------------------------------------------------------
 * pscamera
 * ------------------------------------------------------------------------- */

constexpr GuestString kCameraEscape{ 0x53a7ec, "escape" };
constexpr GuestString kCameraAssists{ 0x53a7f4, "assists" };
constexpr GuestString kCameraCancel{ 0x53a7fc, "CANCEL_CAMERA_OPTIONS" };

// The cameras the four keys hold, and those pscamera found.
constexpr x86::reg32 kCamera1Kept = 0x6788fc, kCamera2Kept = 0x6788f4, kCamera3Kept = 0x6788f8, kCamera4Kept = 0x6788f0;
constexpr x86::reg32 kCameraQuestion = 0x678930;   // char*[1]
constexpr x86::reg32 kCameraButtons = 0x678928;    // char*[2]
constexpr x86::reg32 kCameraMode = 0x5e1054;

constexpr x86::reg32 kCameraAnswered = 0x45d770;
constexpr x86::reg32 kCameraKey = 0x45d840;
constexpr x86::reg32 kBuildCameraCycle = 0x423a60;
constexpr x86::reg32 kRememberMode = 0x423ae0;     // eax a player, edx a mode
constexpr x86::reg32 kSetCameraMode = 0x4238b0;    // likewise

constexpr std::uint32_t kTextSameCamera = 0x1da;
constexpr std::uint32_t kTextAssists = 0x196;
constexpr std::uint32_t kMenuBack = 3;

}  // namespace

/* sub_45d770, the question about two keys on one camera answered: Ok takes each
 * camera off all keys but its first, and leaves. */
x86::reg32 frontend::pause::cameraAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    guest.setByte(close, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(close, 1);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    guest.setByte(close, 1);
    if (fe::dialogFocus(guest) != 0)
        return 0;
    x86::reg32* const views = gameSettings(guest).cameraViewsChosen;
    const auto clearSame = [](x86::reg32& later, const x86::reg32& earlier) {
        if (later == earlier)
            later = 0;
    };
    clearSame(views[1], views[0]);
    clearSame(views[2], views[0]);
    clearSame(views[3], views[0]);
    clearSame(views[2], views[1]);
    clearSame(views[3], views[1]);
    clearSame(views[3], views[2]);
    return kMenuBack;
}

/* sub_45d840, pscamera's way out: it asks if two keys hold one camera. */
x86::reg32 frontend::pause::cameraKey(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const x86::reg32* const views = gameSettings(guest).cameraViewsChosen;
    const auto same = [](x86::reg32 a, x86::reg32 b) {
        return a != 0 && a == b;
    };
    if (!same(views[0], views[1]) && !same(views[0], views[2]) && !same(views[0], views[3])
        && !same(views[1], views[2]) && !same(views[1], views[3]) && !same(views[2], views[3]))
        return kMenuBack;
    guest.setDword(kCameraQuestion, fe::text(guest, kTextSameCamera));
    guest.setDword(kCameraButtons, fe::text(guest, kTextOk));
    guest.setDword(kCameraButtons + 4, fe::text(guest, kTextCancel));
    fe::openQuestion(guest, { 1, kCameraQuestion, 2, kCameraButtons, 0, 1, 0, kCameraAnswered });
    return fe::kDialogHandled;
}

/* sub_45d920, pscamera as it opens: the cameras kept for Cancel. */
x86::reg32 frontend::pause::onCameraEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    dim(guest, cpu);
    guest.setDword(kCamera1Kept, gameSettings(guest).cameraViewsChosen[0]);
    guest.setDword(kCamera2Kept, gameSettings(guest).cameraViewsChosen[1]);
    guest.setDword(kCamera3Kept, gameSettings(guest).cameraViewsChosen[2]);
    guest.setDword(kCamera4Kept, gameSettings(guest).cameraViewsChosen[3]);
    if (Item escape = fe::findItem(guest, menu, kCameraEscape))
        escape.setOnClick(kCameraKey);
    if (Item assists = fe::findItem(guest, menu, kCameraAssists))
        assists.setText(fe::text(guest, kTextAssists));
    return 0;
}

/* sub_45d9a0: at least one camera to look from. */
x86::reg32 frontend::pause::ensureSomeCamera(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (gameSettings(guest).cameraViewsChosen[0] == 0 && gameSettings(guest).cameraViewsChosen[1] == 0 && gameSettings(guest).cameraViewsChosen[2] == 0
        && gameSettings(guest).cameraViewsChosen[3] == 0)
        gameSettings(guest).cameraViewsChosen[0] = 1;
    return 0;
}

/* sub_45d9e0, pscamera as it closes: Cancel puts them back; otherwise the race
 * takes them up at once. */
x86::reg32 frontend::pause::onCameraExit(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (closedBy(guest, cpu, menu, kCameraCancel))
    {
        gameSettings(guest).cameraViewsChosen[0] = guest.dword(kCamera1Kept);
        gameSettings(guest).cameraViewsChosen[1] = guest.dword(kCamera2Kept);
        gameSettings(guest).cameraViewsChosen[2] = guest.dword(kCamera3Kept);
        gameSettings(guest).cameraViewsChosen[3] = guest.dword(kCamera4Kept);
        return 0;
    }
    fe::direct<0x472300, load::cameraViewsSet>(app, cpu);
    fe::direct<kBuildCameraCycle, carstate::cycleBuilt>(guest.app(), guest.cpu());
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.cpu(), 0, guest.dword(kCameraMode));
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.cpu(), 1, guest.dword(kCameraMode));
    if (guest.dword(kInRace) != 0)
    {
        fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.cpu(), 0, guest.dword(kCameraMode));
        if (gameSettings(guest).playMode == 1)
            fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.cpu(), 1, guest.dword(kCameraMode));
    }
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * quitto
 * ------------------------------------------------------------------------- */

constexpr GuestString kQuitReplay{ 0x53ff1c, "PSE_QUITREPLAY" };
constexpr GuestString kQuitToPost{ 0x53ff2c, "PSE_QUITTOPOST" };
constexpr GuestString kQuitToMain{ 0x53ff3c, "PSE_QUITTOMAIN" };
constexpr GuestString kQuitToSystem{ 0x53ff4c, "PSE_QUITTOSYSTEM" };

constexpr x86::reg32 kQuitToLeaveTo = 0x55fdc8;
constexpr x86::reg32 kQuitToQuestion = 0x7a3be0;   // char*[1]
constexpr x86::reg32 kQuitToButtons = 0x7a3be4;    // char*[2]
constexpr x86::reg32 kQuitToAnswered = 0x4b67b0;
constexpr x86::reg32 kQuitReplayClicked = 0x4b6870;
constexpr x86::reg32 kQuitPostClicked = 0x4b6890;
constexpr x86::reg32 kQuitMainClicked = 0x4b68d0;
constexpr x86::reg32 kQuitSystemClicked = 0x4b6910;

}  // namespace

/* sub_4b6810: "are you sure?" for a button of quitto. */
x86::reg32 frontend::pause::quitToAsk(App* app, CPU& cpu, reg32 question, reg32 leaveTo)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest.setDword(kQuitToLeaveTo, leaveTo);
    guest.setDword(kQuitToQuestion, fe::text(guest, question));
    guest.setDword(kQuitToButtons, fe::text(guest, kTextYes));
    guest.setDword(kQuitToButtons + 4, fe::text(guest, kTextNo));
    return openQuestionAnswer(guest, { 1, kQuitToQuestion, 2, kQuitToButtons, 0, 0, 0, kQuitToAnswered });
}

/* sub_4b6930, quitto as it opens: a network race has the one way back, any
 * other the two. */
x86::reg32 frontend::pause::onQuitToEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const auto offer = [&](const GuestString& codelink, x86::reg32 onClick) {
        if (Item item = fe::findItem(guest, menu, codelink))
        {
            item.setOnClick(onClick);
            restore(item);
        }
    };
    const auto withhold = [&](const GuestString& codelink) {
        if (Item item = fe::findItem(guest, menu, codelink))
            retire(item);
    };
    if (gameSettings(guest).replayMode != 0)
    {
        offer(kQuitReplay, kQuitReplayClicked);
        withhold(kQuitToPost);
        withhold(kQuitToMain);
    }
    else
    {
        withhold(kQuitReplay);
        offer(kQuitToPost, kQuitPostClicked);
        offer(kQuitToMain, kQuitMainClicked);
    }
    if (Item system = fe::findItem(guest, menu, kQuitToSystem))
        system.setOnClick(kQuitSystemClicked);
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The race's views: their rectangles and the corners of what is drawn over them
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kFullRect = 0x7cda60;         // four ints
constexpr x86::reg32 kOnePlayerRect = 0x7cda90;
constexpr x86::reg32 kTwoPlayersRect = 0x7cdb98;

}  // namespace

/* sub_4be3d0: the full rectangle and the view's -- for one player the first's
 * across, for two its own -- into the corners of the quads drawn over the
 * screen, as floats. */
x86::reg32 frontend::pause::fillViewQuads(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const std::int32_t left = std::int32_t(guest.dword(kFullRect));
    const std::int32_t top = std::int32_t(guest.dword(kFullRect + 4));
    const std::int32_t right = std::int32_t(guest.dword(kFullRect + 8));
    const std::int32_t bottom = std::int32_t(guest.dword(kFullRect + 0xc));
    std::int32_t viewLeft, viewRight, viewTop, viewBottom;
    if (gameSettings(guest).playMode == 1)
    {
        viewLeft = std::int32_t(guest.dword(kOnePlayerRect));
        viewRight = std::int32_t(guest.dword(kOnePlayerRect + 8) + x86::reg32(viewLeft));
        viewTop = top;
        viewBottom = bottom;
    }
    else
    {
        viewLeft = std::int32_t(guest.dword(kTwoPlayersRect));
        viewRight = std::int32_t(guest.dword(kTwoPlayersRect + 8) + x86::reg32(viewLeft));
        viewTop = std::int32_t(guest.dword(kTwoPlayersRect + 4));
        viewBottom = std::int32_t(guest.dword(kTwoPlayersRect + 0xc) + x86::reg32(viewTop));
    }
    const auto setCorners = [&](std::int32_t value, std::initializer_list<x86::reg32> corners) {
        for (const x86::reg32 at : corners)
            guest.setFloat(at, float(FloatMath::integer(value)));
    };
    setCorners(left, { 0x55fed4, 0x55ffd4 });
    setCorners(top, { 0x55fed8, 0x55fef8 });
    setCorners(right, { 0x55ff34, 0x560034 });
    setCorners(bottom, { 0x55ffd8, 0x55fff8, 0x560018, 0x560038 });
    setCorners(viewLeft, { 0x55fef4, 0x55ff54, 0x55ff94, 0x55fff4 });
    setCorners(viewRight, { 0x55ff14, 0x55ff74, 0x55ffb4, 0x560014 });
    setCorners(viewTop, { 0x55ff58, 0x55ff78 });
    setCorners(viewBottom, { 0x55ff98, 0x55ffb8 });
    const x86::reg32 corner = guest.dword(0x55fef8);
    guest.setDword(0x55ff18, corner);
    guest.setDword(0x55ff38, corner);
    return corner;
}

namespace
{

/* sub_4beb00, sub_4beb40: a rectangle's first or second half set. */
void setRectHalf(Guest& guest, x86::reg32 half, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d)
{
    guest.setDword(half, a);
    guest.setDword(half + 4, b);
    guest.setDword(half + 8, c);
    guest.setDword(half + 0xc, d);
}

}  // namespace

/* sub_4beb20: a rectangle read, each of its four through a pointer. */
x86::reg32 frontend::pause::readRect(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d)
{
    Guest guest(app, cpu);
    guest.setDword(a, guest.dword(rect));
    guest.setDword(b, guest.dword(rect + 4));
    guest.setDword(c, guest.dword(rect + 8));
    const x86::reg32 last = guest.dword(rect + 0xc);
    guest.setDword(d, last);
    return last;
}

namespace
{

/* ---------------------------------------------------------------------------
 * Running them
 * ------------------------------------------------------------------------- */

constexpr std::uint8_t kNothing = 0;
constexpr std::uint8_t kEbxEcx = fe::kRestoreEbx | fe::kRestoreEcx;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEsiEdi = kEbxEcxEsi | fe::kRestoreEdi;

/* A core run under the game's name, its arguments read as the game's code hands them. */

/* A function of nothing. */
template <typename Core>
x86::reg32 wholeForNone(App* app, CPU& cpu, x86::Args& args, fe::Site& site, Core core)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return core(app, cpu); });
}

/* A function of eax: a screen, an item, a name. */
template <typename Core>
x86::reg32 wholeForOne(App* app, CPU& cpu, x86::Args& args, fe::Site& site, Core core, x86::reg32 eax)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return core(app, cpu, eax); });
}

/* A function of eax and edx: a dialog's callback, say. */
template <typename Core>
x86::reg32 wholeForTwo(App* app, CPU& cpu, x86::Args& args, fe::Site& site, Core core, x86::reg32 eax, x86::reg32 edx)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return core(app, cpu, eax, edx); });
}

/* A rectangle's: the rectangle in eax, three in edx, ebx, ecx, the fourth on the stack. */
template <typename Core>
x86::reg32 wholeForRect(App* app, CPU& cpu, x86::Args& args, fe::Site& site, Core core, x86::reg32 rect, x86::reg32 a, x86::reg32 b, x86::reg32 c)
{
    const x86::reg32 d = Guest(app, cpu).dword(cpu.esp + 4);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return core(app, cpu, rect, a, b, c, d); });
}

/* A screen's empty handler. */
x86::reg32 wholeNothing(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, fe::Site& site)
{
    return fe::runThunk(app, cpu, args, site, [](Guest&) { return x86::reg32(0); });
}

const GuestString kSeriesExistsStrings[] = { kSeriesPath };
const GuestString kSeriesWriteStrings[] = { kSeriesPath, kNoName };
const GuestString kTournamentWriteStrings[] = { kTournamentPath, kNoName };
const GuestString kGhostExistsStrings[] = { kKeptPath, kGhostExtension };
const GuestString kTournamentExistsStrings[] = { kKeptPath, kTournamentExtension };
const GuestString kReplayExistsStrings[] = { kKeptPath, kReplayExtension };
const GuestString kPauseStrings[] = {
    kSaveGame, kPauseTitle, kTitleDash, kQuitButton, kAdvertMenu, kLaunchCarEdit, kAiTuner, kHud,
    kHudFirst, kHudSecond, kControlsButton, kRestartButton,
};
const GuestString kOptionsStrings[] = { kAudioButton };
const GuestString kControlsStrings[] = { kForceFeedback, kDeadZone, kCancelButton };
const GuestString kGraphicsStrings[] = {
    kPerspective, kTrackLightingText, kTrackLighting, kWidescreenButton, kCarShade, kIntensitySlider,
    kZBuffer, kProjectedButton,
};
const GuestString kProjectedStrings[] = { kProjectedButton };
const GuestString kGraphicsExitStrings[] = { kGraphicsCancel };
const GuestString kHudStrings[] = { kMapScale, kPosition, kNextAloneCop, kNextAloneRacer };
const GuestString kHud2Strings[] = { kMapScale, kPosition, kNextCopCop, kNextCopRacer, kNextRacerCop, kNextRacerRacer };
const GuestString kHudExitStrings[] = { kHudCancel };
const GuestString kCameraStrings[] = { kCameraEscape, kCameraAssists };
const GuestString kCameraExitStrings[] = { kCameraCancel };
const GuestString kQuitToStrings[] = { kQuitReplay, kQuitToPost, kQuitToMain, kQuitToSystem };

}

/* The functions whose body another shares: each its core, the body with what is its own. */

/* The saves: whether one of a kind stands (keptExists), one written (writeSave). */
x86::reg32 frontend::pause::writeSeriesSave(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x100);
    return writeSave(app, cpu, guest, name, kSeriesPath);
}

x86::reg32 frontend::pause::ghostExists(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 + 0x100);
    return keptExists(guest, cpu, name, kGhostExtension);
}

x86::reg32 frontend::pause::writeTournamentSave(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x100);
    return writeSave(app, cpu, guest, name, kTournamentPath);
}

x86::reg32 frontend::pause::tournamentSaveExists(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 + 0x100);
    return keptExists(guest, cpu, name, kTournamentExtension);
}

x86::reg32 frontend::pause::replayExists(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 + 0x100);
    return keptExists(guest, cpu, name, kReplayExtension);
}

/* A yes or a no answered (yesOrNoAnswered), with what the Yes hands the menu loop. */
x86::reg32 frontend::pause::leaveAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kLeaveTo); });
}

x86::reg32 frontend::pause::restartAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return yesOrNoAnswered(guest, answer, close, [] { return kAnswerRestart; });
}

x86::reg32 frontend::pause::quitToAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kQuitToLeaveTo); });
}

/* The map's size moved (mapSizeMoved): the constant its value is scaled by, the size it sets. */
x86::reg32 frontend::pause::mapCopMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x53932c, gameSettings(guest).hud[1].mapScale);
}

x86::reg32 frontend::pause::mapRacerMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x539330, gameSettings(guest).hud[0].mapScale);
}

x86::reg32 frontend::pause::mapFirstCopMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x539334, gameSettings(guest).hud[3].mapScale);
}

x86::reg32 frontend::pause::mapFirstRacerMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x539338, gameSettings(guest).hud[2].mapScale);
}

x86::reg32 frontend::pause::mapSecondCopMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x53933c, gameSettings(guest).hud[5].mapScale);
}

x86::reg32 frontend::pause::mapSecondRacerMoved(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return mapSizeMoved(guest, cpu, item, 0x539340, gameSettings(guest).hud[4].mapScale);
}

/* A display's screen as it closes (onHudExit): which screen, whose kind of car. */
x86::reg32 frontend::pause::hudExit(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return onHudExit(guest, cpu, menu, 0, 0);
}

x86::reg32 frontend::pause::hud1Exit(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return onHudExit(guest, cpu, menu, 1, 0);
}

x86::reg32 frontend::pause::hud2Exit(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    return onHudExit(guest, cpu, menu, 2, 1);
}

/* quitto's buttons: "are you sure?" about where each goes. */
x86::reg32 frontend::pause::quitToReplay(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x4b6810, quitToAsk>(app, cpu, kAskQuitNetwork, kLeaveToPostGame);
    return fe::kDialogHandled;
}

x86::reg32 frontend::pause::quitToPost(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x4b6810, quitToAsk>(app, cpu, askQuitRace(guest), kLeaveToPostGame);
    return fe::kDialogHandled;
}

x86::reg32 frontend::pause::quitToMain(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x4b6810, quitToAsk>(app, cpu, askQuitRace(guest), kLeaveToMain);
    return fe::kDialogHandled;
}

x86::reg32 frontend::pause::quitToSystem(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x4b6810, quitToAsk>(app, cpu, kAskQuitToSystem, kLeaveToSystem);
    return fe::kDialogHandled;
}

/* A rectangle's first or second half set (setRectHalf). */
x86::reg32 frontend::pause::setRectFirst(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d)
{
    Guest guest(app, cpu);
    setRectHalf(guest, rect, a, b, c, d);
    return rect;
}

x86::reg32 frontend::pause::setRectSecond(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d)
{
    Guest guest(app, cpu);
    setRectHalf(guest, rect + 0x10, a, b, c, d);
    return rect;
}

/* The saves. */
x86::reg32 frontend::pauseSeriesSaveExists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_446510", 0, 0, kSeriesExistsStrings, std::size(kSeriesExistsStrings), kNothing };
    return wholeForOne(app, cpu, args, site, pause::seriesSaveExists, args.eax);
}

x86::reg32 frontend::pauseWriteSeriesSave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_446550", 0, 0, kSeriesWriteStrings, std::size(kSeriesWriteStrings) };
    return wholeForOne(app, cpu, args, site, pause::writeSeriesSave, args.eax);
}

x86::reg32 frontend::pauseGhostExists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_446740", 0, 0, kGhostExistsStrings, std::size(kGhostExistsStrings), kNothing };
    return wholeForOne(app, cpu, args, site, pause::ghostExists, args.eax);
}

x86::reg32 frontend::pauseWriteTournamentSave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4468c0", 0, 0, kTournamentWriteStrings, std::size(kTournamentWriteStrings) };
    return wholeForOne(app, cpu, args, site, pause::writeTournamentSave, args.eax);
}

x86::reg32 frontend::pauseTournamentSaveExists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_446950", 0, 0, kTournamentExistsStrings, std::size(kTournamentExistsStrings),
                          kNothing };
    return wholeForOne(app, cpu, args, site, pause::tournamentSaveExists, args.eax);
}

x86::reg32 frontend::pauseReplayExists(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_446a90", 0, 0, kReplayExistsStrings, std::size(kReplayExistsStrings), kNothing };
    return wholeForOne(app, cpu, args, site, pause::replayExists, args.eax);
}

x86::reg32 frontend::pauseCheckSaveable(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478010", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return wholeForOne(app, cpu, args, site, pause::checkSaveable, args.eax);
}

x86::reg32 frontend::pauseOverwriteAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_447800", 0, 0, nullptr, 0, kEbxEcxEsi };
    return wholeForTwo(app, cpu, args, site, pause::overwriteAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseSaveNameAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4478c0", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    return wholeForTwo(app, cpu, args, site, pause::saveNameAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseAskSaveName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_447a30", 0, 0, nullptr, 0 };
    return wholeForOne(app, cpu, args, site, pause::askSaveName, args.eax);
}

/* pause. */
x86::reg32 frontend::pauseHideSpare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c2d0", 0, 0, kHideMe, std::size(kHideMe), kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::hideSpare, args.eax);
}

x86::reg32 frontend::pauseHideForCop(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c360", 0, 0, kRacerInstruments, std::size(kRacerInstruments), kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::hideForCop, args.eax);
}

x86::reg32 frontend::pauseHideForRacer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c4e0", 0, 0, kCopInstrumentsFirst, std::size(kCopInstrumentsFirst), kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::hideForRacer, args.eax);
}

x86::reg32 frontend::pauseLeaveAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c610", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, cpu, args, site, pause::leaveAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseRestartAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c670", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, cpu, args, site, pause::restartAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseAskLeave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c6d0", 0, 0, nullptr, 0, kEbxEcx };
    return wholeForTwo(app, cpu, args, site, pause::askLeave, args.eax, args.edx);
}

x86::reg32 frontend::pauseQuitChoiceAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c730", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, cpu, args, site, pause::quitChoiceAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseQuitClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44c7b0", 0, 0, nullptr, 0 };
    return wholeForNone(app, cpu, args, site, pause::onQuitClicked);
}

x86::reg32 frontend::pauseRestartClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44ca10", 0, 0, nullptr, 0, kEbxEcxEdx };
    return wholeForNone(app, cpu, args, site, pause::onRestartClicked);
}

x86::reg32 frontend::pauseSaveClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44ca80", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, cpu, args, site, pause::onSaveClicked);
}

x86::reg32 frontend::pauseOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44cab0", 0, 0, kPauseStrings, std::size(kPauseStrings) };
    return wholeForOne(app, cpu, args, site, pause::onPauseEnter, args.eax);
}

x86::reg32 frontend::pauseOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44cf60", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, cpu, args, site);
}

/* The sliders. */
x86::reg32 frontend::pauseIntensityMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44cf70", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::intensityMoved, args.eax);
}

x86::reg32 frontend::pauseMapCopMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44cfa0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapCopMoved, args.eax);
}

x86::reg32 frontend::pauseMapRacerMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44cfe0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapRacerMoved, args.eax);
}

x86::reg32 frontend::pauseMapFirstCopMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d020", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapFirstCopMoved, args.eax);
}

x86::reg32 frontend::pauseMapFirstRacerMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d060", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapFirstRacerMoved, args.eax);
}

x86::reg32 frontend::pauseMapSecondCopMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d0a0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapSecondCopMoved, args.eax);
}

x86::reg32 frontend::pauseMapSecondRacerMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d0e0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::mapSecondRacerMoved, args.eax);
}

/* psoption, psoptn2, pscntl1. */
x86::reg32 frontend::pauseOptionsOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d120", 0, 0, kOptionsStrings, std::size(kOptionsStrings), kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::onOptionsEnter, args.eax);
}

x86::reg32 frontend::pauseOptionsOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d160", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, cpu, args, site);
}

x86::reg32 frontend::pauseControlsCancelKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d170", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 item = args.eax, menu = args.edx;
    const std::uint16_t key = std::uint16_t(args.ebx);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return pause::cancelKey(app, cpu, item, menu, key); });
}

x86::reg32 frontend::pauseControlsOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d190", 0, 0, kControlsStrings, std::size(kControlsStrings), kEbxEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::onControlsEnter, args.eax);
}

x86::reg32 frontend::pauseControlsOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d220", 0, 0, nullptr, 0, kEbxEcxEdx };
    return wholeForNone(app, cpu, args, site, pause::onControlsExit);
}

/* psgraph. */
x86::reg32 frontend::pauseWidescreenClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d270", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, cpu, args, site, pause::onWidescreenClicked);
}

x86::reg32 frontend::pauseGraphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d2a0", 0, 0, kGraphicsStrings, std::size(kGraphicsStrings) };
    return wholeForOne(app, cpu, args, site, pause::onGraphicsEnter, args.eax);
}

x86::reg32 frontend::pauseGraphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d3e0", 0, 0, kProjectedStrings, std::size(kProjectedStrings), fe::kRestoreEdx };
    return wholeForOne(app, cpu, args, site, pause::fitProjected, args.eax);
}

x86::reg32 frontend::pauseGraphicsOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d420", 0, 0, kGraphicsExitStrings, std::size(kGraphicsExitStrings) };
    return wholeForOne(app, cpu, args, site, pause::onGraphicsExit, args.eax);
}

/* pshud, pshud1p, pshud2p. */
x86::reg32 frontend::pauseHudOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d4c0", 0, 0, kHudStrings, std::size(kHudStrings) };
    return wholeForOne(app, cpu, args, site, pause::onHudEnter, args.eax);
}

x86::reg32 frontend::pauseHudOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d670", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, cpu, args, site, pause::hudExit, args.eax);
}

x86::reg32 frontend::pauseHud1OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d7a0", 0, 0, kHud2Strings, std::size(kHud2Strings) };
    return wholeForOne(app, cpu, args, site, pause::onHud1Enter, args.eax);
}

x86::reg32 frontend::pauseHud1OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44d9a0", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, cpu, args, site, pause::hud1Exit, args.eax);
}

x86::reg32 frontend::pauseHud2OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44dad0", 0, 0, kHud2Strings, std::size(kHud2Strings) };
    return wholeForOne(app, cpu, args, site, pause::onHud2Enter, args.eax);
}

x86::reg32 frontend::pauseHud2OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44dcd0", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, cpu, args, site, pause::hud2Exit, args.eax);
}

/* aituner. */
x86::reg32 frontend::pauseAiTunerOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44e130", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, cpu, args, site);
}

x86::reg32 frontend::pauseAiTunerOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_44e140", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, cpu, args, site);
}

/* pscamera. */
x86::reg32 frontend::pauseCameraAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45d770", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    return wholeForTwo(app, cpu, args, site, pause::cameraAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseCameraKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45d840", 0, 0, nullptr, 0 };
    return wholeForNone(app, cpu, args, site, pause::cameraKey);
}

x86::reg32 frontend::pauseCameraOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45d920", 0, 0, kCameraStrings, std::size(kCameraStrings), kEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::onCameraEnter, args.eax);
}

x86::reg32 frontend::pauseCameraEnsureOne(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45d9a0", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, cpu, args, site, pause::ensureSomeCamera);
}

x86::reg32 frontend::pauseCameraOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_45d9e0", 0, 0, kCameraExitStrings, std::size(kCameraExitStrings),
                          fe::kRestoreEbx | fe::kRestoreEdx };
    return wholeForOne(app, cpu, args, site, pause::onCameraExit, args.eax);
}

/* quitto. */
x86::reg32 frontend::pauseQuitToAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b67b0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, cpu, args, site, pause::quitToAnswered, args.eax, args.edx);
}

x86::reg32 frontend::pauseQuitToAsk(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b6810", 0, 0, nullptr, 0, kEbxEcx };
    return wholeForTwo(app, cpu, args, site, pause::quitToAsk, args.eax, args.edx);
}

x86::reg32 frontend::pauseQuitToReplay(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b6870", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, cpu, args, site, pause::quitToReplay);
}

x86::reg32 frontend::pauseQuitToPost(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b6890", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, cpu, args, site, pause::quitToPost);
}

x86::reg32 frontend::pauseQuitToMain(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b68d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, cpu, args, site, pause::quitToMain);
}

x86::reg32 frontend::pauseQuitToSystem(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b6910", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, cpu, args, site, pause::quitToSystem);
}

x86::reg32 frontend::pauseQuitToOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b6930", 0, 0, kQuitToStrings, std::size(kQuitToStrings), kEbxEcxEdx };
    return wholeForOne(app, cpu, args, site, pause::onQuitToEnter, args.eax);
}

/* The race's views. */
x86::reg32 frontend::pauseFillViewQuads(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be3d0", 0, 0, nullptr, 0 };
    return wholeForNone(app, cpu, args, site, pause::fillViewQuads);
}

x86::reg32 frontend::pauseSetRectFirst(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4beb00", 4, 0, nullptr, 0, kNothing };
    return wholeForRect(app, cpu, args, site, pause::setRectFirst, args.eax, args.edx, args.ebx, args.ecx);
}

x86::reg32 frontend::pauseReadRect(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4beb20", 4, 0, nullptr, 0, fe::kRestoreEsi };
    return wholeForRect(app, cpu, args, site, pause::readRect, args.eax, args.edx, args.ebx, args.ecx);
}

x86::reg32 frontend::pauseSetRectSecond(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4beb40", 4, 0, nullptr, 0, kNothing };
    return wholeForRect(app, cpu, args, site, pause::setRectSecond, args.eax, args.edx, args.ebx, args.ecx);
}

}
