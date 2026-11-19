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
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
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

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
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
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The game's state.
constexpr nfs::u32 kNetworkHost = 0x7a22da;      // int16
constexpr nfs::u32 kNetworkJoin = 0x7a22e4;
constexpr nfs::u32 kInRace = 0x559234;           // the menus are a race's pause screens
constexpr nfs::u32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kTrackLightingFlags = 0x06;
constexpr nfs::u32 kDisplayBuffering = 0x7a3a68;
constexpr nfs::u32 kDeveloperFlags = 0x55eb2e;   // byte
constexpr std::uint8_t kDeveloperQuit = 0x20;

// The settings, and their copy that Cancel puts back.
constexpr nfs::u32 kSettingsKept = 0x662ca0;
constexpr nfs::u32 kSettingsSize = 0x176c;
constexpr nfs::u32 kScreenSizeKept = 0x662da0;

// Fields of items the header does not name.
constexpr nfs::u32 kItemOnKey = 0x30;            // a button's key handler
constexpr nfs::u32 kSliderValue = 0x54;          // a slider's value, a float
constexpr std::uint32_t kButton = 5;               // an item's type (item::kType)

// The game's functions.
constexpr std::uint32_t kDimBy = 0x3f19999a;       // 0.6, the shade pushed for 0x451530
constexpr nfs::u32 kButtonKey = 0x461270;
constexpr nfs::u32 kSprintf = 0x4df690;
constexpr nfs::u32 kCarFlags = 0x200;            // byte of the car
constexpr std::uint8_t kCarIsCop = 0x20;

// This group's own handed to the game: callbacks, handlers.
constexpr nfs::u32 kSaveNameAnswered = 0x4478c0;
constexpr nfs::u32 kOverwriteAnswered = 0x447800;
constexpr nfs::u32 kAskLeaveAnswered = 0x44c610;
constexpr nfs::u32 kQuitChoiceAnswered = 0x44c730;
constexpr nfs::u32 kOnQuitClicked = 0x44c7b0;
constexpr nfs::u32 kOnRestartClicked = 0x44ca10;
constexpr nfs::u32 kRestartAnswered = 0x44c670;
constexpr nfs::u32 kOnSaveClicked = 0x44ca80;
constexpr nfs::u32 kCancelKey = 0x44d170;
constexpr nfs::u32 kOnWidescreenClicked = 0x44d270;
constexpr nfs::u32 kIntensityMoved = 0x44cf70;

/* The settings copied as they stand: into the keeping, back out of it. */
void copySettings(Guest& guest, nfs::u32 to, nfs::u32 from)
{
    for (nfs::u32 at = 0; at < kSettingsSize; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
}

/* The race behind a pause screen, darkened: the shade pushed where the game's code pushes it, under
 * the frame; eax handed on as it is. */
void dim(Guest& guest, ThreadState& ts)
{
    const nfs::u32 slot = guest.frame() - 4;
    guest.setDword(slot, kDimBy);
    fe::direct<0x451530, port::text::dimmed>(guest.app(), ts, slot, 0);  // eax: its result is not read
}

/* The item of a screen named `codelink` (sub_442a40), 0 for none: the look-up's result, which the screens' functions
 * go on with as "what the calls left". */
nfs::u32 foundAt(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    return fe::direct<0x442a40, frontend::menus::findItem>(guest.app(), guest.ts(), menu, codelink.address);
}

nfs::u32 hideItem(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    const nfs::u32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setHidden(true);
    return found;
}

nfs::u32 showItem(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    const nfs::u32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setHidden(false);
    return found;
}

nfs::u32 disableItem(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    const nfs::u32 found = foundAt(guest, menu, codelink);
    if (Item item = Item(guest, found))
        item.setDisabled(true);
    return found;
}

/* The dialog and the question as fe::openDialog and fe::openQuestion open them (sub_444c20, sub_4446d0), with what they
 * answer. */
nfs::u32 openDialogAnswer(Guest& guest, const fe::Dialog& dialog)
{
    return fe::direct<0x444c20, frontend::dialogs::openEdit>(
        guest.app(), guest.ts(), dialog.lineCount, dialog.lines, dialog.buttonCount, dialog.buttons, dialog.focus,
        dialog.edit, dialog.editLength, dialog.editMode, dialog.callback);
}

nfs::u32 openQuestionAnswer(Guest& guest, const fe::Question& question)
{
    return fe::direct<0x4446d0, frontend::dialogs::openQuestion>(
        guest.app(), guest.ts(), question.lineCount, question.lines, question.buttonCount, question.buttons,
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
bool closedBy(Guest& guest, ThreadState& ts, nfs::u32 menu, const GuestString& codelink)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    const nfs::u32 item = fe::item::kTable + fe::item::kSize * nfs::u32(first + selected);
    return guest.dword(item + fe::item::kType) == kButton
        && *fe::direct<0x4429c0, port::menus::codelinkMatched>(guest.app(), ts, ts.sp - 4,
                                                               guest.dword(item + fe::item::kCodelink),
                                                               codelink.address, 0) != 0;
}

/* The car a player looks from is a cop's: 1, or 0. */
nfs::u32 playerIsCop(Guest& guest, ThreadState& ts, nfs::u32 player)
{
    const nfs::u32 car = fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), ts, player);
    return (guest.byte(car + kCarFlags) & kCarIsCop) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * The saves: whether a file stands, and a game written
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kSavesFolder = 0x7a2f34;
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
nfs::u32 frontend::pause::seriesSaveExists(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 + 0x100);
    const nfs::u32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kSeriesPath.address, guest::Words{kSavesFolder, name});
    return *fe::direct<0x4e0e60, ea::fileThere>(app, ts, path) ? 1 : 0;
}

namespace
{

/* sub_446740, sub_446950, sub_446a90: a save of this kind by this name stands. */
nfs::u32 keptExists(Guest& guest, ThreadState& ts, nfs::u32 name, const GuestString& extension)
{
    const nfs::u32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kKeptPath.address, guest::Words{kSavesFolder, name, extension.address});
    return *fe::direct<0x4e0e60, ea::fileThere>(guest.app(), ts, path) ? 1 : 0;
}

/* sub_446550, sub_4468c0: the game written under this name: a copy of it, its
 * cars' names filled in, to the file.  1 when it went out. */
nfs::u32 writeSave(App* app, ThreadState& ts, Guest& guest, nfs::u32 name, const GuestString& pathFormat)
{
    const nfs::u32 path = guest.frame();   // char[0x100]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, pathFormat.address, guest::Words{kSavesFolder, name});
    const nfs::u32 copy =
        *fe::direct<0x4e1620, ea::reserved>(app, ts, kNoName.address, kSaveImageSize, kReserveFlags);
    if (copy == 0)
        return 0;
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, copy, SETTINGS_ADDRESS(fileSize), kSaveImageSize);
    nfs::u32 written = 0;
    if (guest::direct<0x478010, frontend::pause::checkSaveable>(app, ts, copy) != 0)
    {
        fe::direct<0x4eeae0, load::blockSaved>(app, ts, path, copy, kSaveImageSize);
        written = 1;
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, copy);
    return written;
}

// A save's cars, by number, and the room for their names.
constexpr std::uint32_t kCarNameDwords = 4;
constexpr nfs::u32 kSavedFirstCar = 0x177c;
constexpr nfs::u32 kSavedFirstCarName = 0x1780;
constexpr nfs::u32 kSavedSecondCar = 0x17e8;
constexpr nfs::u32 kSavedSecondCarName = 0x17ec;
constexpr nfs::u32 kSavedThirdCar = 0x1994;      // counted from 4; below that none
constexpr nfs::u32 kSavedThirdCarName = 0x1998;

void copyCarName(Guest& guest, nfs::u32 to, nfs::u32 from)
{
    for (std::uint32_t i = 0; i < kCarNameDwords; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

}  // namespace

/* sub_478010: a save about to be written is one, and each of its cars has a
 * name: filled in.  1 when all of it held. */
nfs::u32 frontend::pause::checkSaveable(App* app, ThreadState& ts, u32 save)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (guest.dword(save) != kSaveImageSize)
        return 0;
    const nfs::u32 first = *fe::direct<0x439520, load::idOf>(app, ts, guest.dword(save + kSavedFirstCar));
    if (first == 0)
        return 0;
    copyCarName(guest, save + kSavedFirstCarName, first);
    const nfs::u32 second = *fe::direct<0x439520, load::idOf>(app, ts, guest.dword(save + kSavedSecondCar));
    if (second == 0)
        return 0;
    copyCarName(guest, save + kSavedSecondCarName, second);
    const std::int32_t third = std::int32_t(guest.dword(save + kSavedThirdCar));
    if (third >= 4)
    {
        const nfs::u32 name = *fe::direct<0x439520, load::idOf>(app, ts, nfs::u32(third - 4));
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
constexpr nfs::u32 kSaveKind = 0x5591e8;
constexpr nfs::u32 kWriteSave = 0x5591cc;
constexpr nfs::u32 kSaveExists = 0x5591d0;
constexpr nfs::u32 kSaveName = 0x5591d4;
constexpr std::uint32_t kSaveNameLength = 0xa;
// The name dialog's texts.
constexpr nfs::u32 kNameButtons = 0x6611d4;      // char*[2]
constexpr nfs::u32 kNameQuestion = 0x6611dc;     // char*[1]
// How it went, and the question whether to write over a file.
constexpr nfs::u32 kOverwriteResult = 0x6611b8;  // char*[2]: the name, a line
constexpr nfs::u32 kWrittenResult = 0x6611c0;    // char*[2]: a line, the name
constexpr nfs::u32 kOverwriteQuestion = 0x6611c8;  // char*[1]
constexpr nfs::u32 kOverwriteButtons = 0x6611cc;   // char*[2]

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

constexpr nfs::u32 kWriteSeries = 0x446550, kSeriesExists = 0x446510;
constexpr nfs::u32 kWriteTournament = 0x4468c0, kTournamentExists = 0x446950;
constexpr nfs::u32 kWriteGhost = 0x446780, kGhostExists = 0x446740;
constexpr nfs::u32 kWriteReplay = 0x446a10, kReplayExists = 0x446a90;

struct SaveKind
{
    std::uint32_t text;
    nfs::u32 write;
    nfs::u32 exists;
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
nfs::u32 frontend::pause::askSaveName(App* app, ThreadState& ts, u32 kind)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
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
nfs::u32 frontend::pause::overwriteAnswered(App* app, ThreadState& ts, u32 answer, u32 keepOpen)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(keepOpen, 0);
    if (answer != fe::kDialogEscape && answer != fe::kDialogEnter)
        return 0;
    if (answer == fe::kDialogEscape || focus == 1)
    {
        guest::direct<0x447a30, askSaveName>(app, ts, guest.dword(kSaveKind));
        guest.setByte(keepOpen, 0);
        return 0;
    }
    if (focus != 0)
        return 0;
    const bool written = callAt(guest, guest.dword(kWriteSave), fe::args({ kSaveName })) != 0;
    guest.setDword(kOverwriteResult, kSaveName);
    guest.setDword(kOverwriteResult + 4, fe::text(guest, written ? kTextSaved : kTextNotSaved));
    fe::direct<0x444a80, frontend::dialogs::openMessage>(app, ts, nfs::u32(2), kOverwriteResult, kResultTime,
                                                         answer, nfs::u32(0));
    guest.setByte(keepOpen, 0);
    return 0;
}

/* sub_4478c0, the name dialog answered: a name that stands asks whether to
 * write over it, a new one is written at once. */
nfs::u32 frontend::pause::saveNameAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 4);
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(close, 0);
    const nfs::u32 typed = fe::dialogEditText(guest);
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
        const nfs::u32 line = fe::text(guest, written ? kTextSaved : kTextNotSaved);
        guest.setDword(kWrittenResult, line);
        guest.setDword(kWrittenResult + 4, kSaveName);
        fe::direct<0x444a80, frontend::dialogs::openMessage>(app, ts, nfs::u32(2), kWrittenResult, kResultTime,
                                                             answer, nfs::u32(0));
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
nfs::u32 frontend::pause::hideSpare(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    nfs::u32 last = 0;
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
nfs::u32 frontend::pause::hideForCop(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    nfs::u32 last = 0;
    for (const GuestString& codelink : kRacerInstruments)
        last = hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopGreyed)
        last = disableItem(guest, menu, codelink);
    return last;
}

/* sub_44c4e0: a racer's display: a cop's items hidden. */
nfs::u32 frontend::pause::hideForRacer(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    nfs::u32 last = 0;
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

constexpr nfs::u32 kLeaveTo = 0x559268;          // what the Yes hands the menu loop
constexpr std::uint32_t kAnswerRestart = 6;

/* sub_44c610, sub_44c670, sub_4b67b0, a yes or a no: Yes hands the menu loop
 * what `yes` reads, No and Escape close the box. */
template <typename Yes>
nfs::u32 yesOrNoAnswered(Guest& guest, nfs::u32 answer, nfs::u32 close, Yes yes)
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

constexpr nfs::u32 kLeaveQuestion = 0x664410;    // char*[1]
constexpr nfs::u32 kLeaveButtons = 0x664414;     // char*[2]

}  // namespace

/* sub_44c6d0: "are you sure?" asked for a choice of the quit box, No first. */
nfs::u32 frontend::pause::askLeave(App* app, ThreadState& ts, u32 question, u32 leaveTo)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest.setDword(kLeaveTo, leaveTo);
    guest.setDword(kLeaveQuestion, fe::text(guest, question));
    guest.setDword(kLeaveButtons, fe::text(guest, kTextYes));
    guest.setDword(kLeaveButtons + 4, fe::text(guest, kTextNo));
    return openQuestionAnswer(guest, { 1, kLeaveQuestion, 2, kLeaveButtons, 0, 1, 0, kAskLeaveAnswered });
}

namespace
{

// The quit box: each button's Yes and its question.
constexpr nfs::u32 kQuitButtonAnswer = 0x662c88; // int[4]; kQuitJustClose closes
constexpr nfs::u32 kQuitButtonQuestion = 0x662c70;
constexpr std::uint32_t kQuitJustClose = 2;
constexpr nfs::u32 kQuitQuestion = 0x66441c;     // char*[1]
constexpr nfs::u32 kQuitButtons = 0x664420;      // char*[up to 4]
constexpr nfs::u32 kNetworkReplayOff = 0x7d36b4;

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
nfs::u32 frontend::pause::quitChoiceAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const auto closeBox = [&](std::uint8_t value) {
        if (close != 0)
            guest.setByte(close, value);
    };
    nfs::u32 result = 0;
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
            guest::direct<0x44c6d0, askLeave>(app, ts, guest.dword(kQuitButtonQuestion + 4 * focus),
                                              guest.dword(kQuitButtonAnswer + 4 * focus));
            closeBox(0);
        }
    }
    return result;
}

/* sub_44c7b0, Quit clicked on pause: where to, by what kind of a race it is. */
nfs::u32 frontend::pause::onQuitClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
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

constexpr nfs::u32 kRestartQuestion = 0x664434;  // char*[1]
constexpr nfs::u32 kRestartButtons = 0x664438;   // char*[2]
constexpr std::uint32_t kTextRestart = 0x1ea;

}  // namespace

/* sub_44ca10, Restart clicked. */
nfs::u32 frontend::pause::onRestartClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setDword(kRestartQuestion, fe::text(guest, kTextRestart));
    guest.setDword(kRestartButtons, fe::text(guest, kTextYes));
    guest.setDword(kRestartButtons + 4, fe::text(guest, kTextNo));
    fe::openQuestion(guest, { 1, kRestartQuestion, 2, kRestartButtons, 0, 0, 0, kRestartAnswered });
    return fe::kDialogHandled;
}

/* sub_44ca80, Save clicked: the name asked for, of the kind of save the race's
 * type keeps. */
nfs::u32 frontend::pause::onSaveClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const std::uint32_t type = gameSettings(guest).raceType;
    if (type == 1)
        guest::direct<0x447a30, askSaveName>(app, ts, nfs::u32(1));
    else if (type == 2)
        guest::direct<0x447a30, askSaveName>(app, ts, nfs::u32(0));
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

constexpr nfs::u32 kPauseTitleText = 0x55926c;
constexpr nfs::u32 kFirstViewRect = 0x7cdac0;
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
    const nfs::u32 paused = fe::text(guest, kTextPaused);
    guest.copyString(kPauseTitleText + guest.length(kPauseTitleText), paused);
}

}  // namespace

/* sub_44cab0, pause as it opens. */
nfs::u32 frontend::pause::onPauseEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc);
    // A view's rectangle comes back into these two; its width is what counts.
    const nfs::u32 rectOther = guest.frame();
    const nfs::u32 rectWidth = guest.frame() + 4;
    guest.setDword(rectOther, 0);
    guest.setDword(rectWidth, 0);
    dim(guest, ts);

    if (Item save = fe::findItem(guest, menu, kSaveGame))
    {
        if (gameSettings(guest).playMode != 1)
            guest.setWord(save.address() + fe::item::kY, kSaveButtonSplitY);
        const std::uint32_t type = gameSettings(guest).raceType;
        if (std::int32_t(gameSettings(guest).playMode) <= 1 && gameSettings(guest).replayMode == 0 && (type == 1 || type == 2))
        {
            const nfs::u32 line = fe::text(guest, type == 1 ? kTextSaveSeries : kTextSaveTournament);
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
        guest::direct<0x4beb20, readRect>(app, ts, kFirstViewRect, rectOther, rectOther, rectWidth, rectOther);
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

    guest::direct<0x44c2d0, hideSpare>(app, ts, menu);
    return 0;
}

/* ---------------------------------------------------------------------------
 * The sliders of the graphics and the display
 * ------------------------------------------------------------------------- */

/* sub_44cf70, the effects' intensity moved. */
nfs::u32 frontend::pause::intensityMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    // The 0xf the game hands the shown test in edx is not read by it.
    const nfs::u32 shown = fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item);
    if (shown == 0)
        return shown;
    gameSettings(guest).alphaIntensity = float(fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, ts, item, nfs::u32(0)));
    // eax as the slider's value leaves it: the slider, which its code keeps there.
    return item;
}

namespace
{

/* sub_44cfa0 and the five after it, the map's size moved: the slider from 0
 * to 1, the size from 1 to 4. */
nfs::u32 mapSizeMoved(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32 scale, float& size)
{
    // As intensityMoved: the 0xf in edx not read, eax as the slider's value leaves it (the slider).
    const nfs::u32 shown = fe::direct<0x44a660, frontend::loop::shownNow>(guest.app(), ts, item);
    if (shown == 0)
        return shown;
    const nfs::Float value = fe::direct<0x46c9b0, frontend::glue::sliderValue>(guest.app(), ts, item, nfs::u32(0));
    FloatMath x87(guest);
    size = float(x87.add(x87.mul(value, x87.load(scale)), nfs::Float(1.0)));
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
constexpr nfs::u32 kControlsKept = 0x5f3cd0;
constexpr std::uint32_t kControlsBothSize = 0x68;
constexpr std::uint32_t kControlsOneSize = 0x34;
constexpr nfs::u32 kControlsDone = 0x66440c;     // not Cancel
constexpr std::uint8_t kControllerForce = 0x08;

}  // namespace

/* sub_44d120, psoption and psoptn2 as they open. */
nfs::u32 frontend::pause::onOptionsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    dim(guest, ts);
    if (Item audio = fe::findItem(guest, menu, kAudioButton))
    {
        if (gameSettings(guest).installIsRemote != 0)
            audio.setDisabled(true);
    }
    return 0;
}

/* sub_44d170, Cancel's key on pscntl1: Enter on it is what cancels. */
nfs::u32 frontend::pause::cancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (key == 0xd)
        guest.setDword(kControlsDone, 0);
        return nfs::u32(*fe::direct<kButtonKey, port::buttons::buttonKeyed>(
        app, ts, ts.sp - 4, item, nfs::u32(std::int32_t(std::int16_t(key)))));
}

/* sub_44d190, pscntl1 as it opens: the controls kept for Cancel. */
nfs::u32 frontend::pause::onControlsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    dim(guest, ts);
    guest.setDword(kControlsDone, 1);
    const nfs::u32 controls = fe::direct<0x4904d0, controls::controlSets>(app, ts, gameSettings(guest).playMode);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kControlsKept, controls, kControlsBothSize);
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
nfs::u32 frontend::pause::onControlsExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const nfs::u32 controls = fe::direct<0x4904d0, controls::controlSets>(app, ts, gameSettings(guest).playMode);
    if (guest.dword(kControlsDone) == 0)
    {
        const std::uint32_t size = gameSettings(guest).playMode == 1 ? kControlsBothSize : kControlsOneSize;
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, controls, kControlsKept, size);
    }
    else
    {
        fe::direct<0x4710f0, controls::forceCarFound>(app, ts);
    }
    fe::direct<0x48f1f0, controls::playersBound>(app, ts);
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
nfs::u32 frontend::pause::onWidescreenClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x4be5e0, frontend::loopb::viewsLayout>(app, ts, playerIsCop(guest, ts, 0));
    return fe::kDialogHandled;
}

/* sub_44d2a0, psgraph as it opens: the settings kept for Cancel, what the card
 * cannot do greyed out.  Port: the original greys the intensity out on a card
 * without alpha (a probe of 0x7a3a58); here every card has it. */
nfs::u32 frontend::pause::onGraphicsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    dim(guest, ts);
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
        guest.setFloat(intensity.address() + kSliderValue, float(nfs::Float(gameSettings(guest).alphaIntensity)));
        fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, ts, intensity.address(), kIntensityMoved);
    }
    if (Item zBuffer = fe::findItem(guest, menu, kZBuffer))
        zBuffer.setDisabled(guest.dword(kDisplayBuffering) == 0);
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0);
    return 0;
}

/* sub_44d3e0: the projected textures' button by what is on now. */
nfs::u32 frontend::pause::fitProjected(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0 || gameSettings(guest).projectedText == 0);
    return 0;
}

/* sub_44d420, psgraph as it closes: Cancel puts the settings back, and the
 * views laid out again if that changes the screen's shape. */
nfs::u32 frontend::pause::onGraphicsExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (!closedBy(guest, ts, menu, kGraphicsCancel))
        return 0;
    const bool shapeChanged = guest.dword(kScreenSizeKept) != gameSettings(guest).wideScreen;
    copySettings(guest, SETTINGS_ADDRESS(language), kSettingsKept);
    if (shapeChanged)
        fe::direct<0x4be5e0, frontend::loopb::viewsLayout>(app, ts, playerIsCop(guest, ts, 0));
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

constexpr nfs::u32 kHudShown = 0x791b1c;
constexpr nfs::u32 kFirstIsRacer = 0x677df0;
constexpr nfs::u32 kSecondIsRacer = 0x677df4;

/* Each of the three screens keeps, for each kind of car, four choices in the
 * settings -- 0 the last on its list, otherwise one past its place -- and, while
 * it is up, each one's place on its list.  Those of the first screen's racer are
 * here; the others' stand further on by a stride. */
constexpr nfs::u32 kHudPlacesStride = 8;
constexpr nfs::u32 kHudList1Places = 0x678828;
constexpr std::uint32_t kHudList1Last = 5;
constexpr nfs::u32 kHudList2Places = 0x678840;
constexpr std::uint32_t kHudList2Last = 4;
constexpr nfs::u32 kHudList3Places = 0x678810;
constexpr std::uint32_t kHudList3Last = 3;
constexpr nfs::u32 kHudList4Places = 0x6787f8;
constexpr std::uint32_t kHudList4Last = 2;

/* A screen's map slider: what it moves for each kind of car, what is told of
 * its moves, and the constants its value comes from. */
struct HudScreen
{
    nfs::u32 mapMoved[2];
    nfs::u32 mapBias;        // float, -1
    nfs::u32 mapScale;       // float, a third
};
constexpr HudScreen kHudScreens[] = {
    { { 0x44cfe0, 0x44cfa0 }, 0x539434, 0x539438 },
    { { 0x44d060, 0x44d020 }, 0x539474, 0x539478 },
    { { 0x44d0e0, 0x44d0a0 }, 0x53947c, 0x539480 },
};

nfs::u32 hudPlaces(nfs::u32 screen, nfs::u32 kind)
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
void openHud(App* app, ThreadState& ts, Guest& guest, nfs::u32 menu, nfs::u32 screen, nfs::u32 kind,
             nfs::u32 next)
{
    const HudScreen& hud = kHudScreens[screen];
    dim(guest, ts);
    if (Item map = fe::findItem(guest, menu, kMapScale))
    {
        FloatMath x87(guest);
        const nfs::Float biased = x87.add(nfs::Float(gameSettings(guest).hudOf(screen, kind).mapScale), x87.load(hud.mapBias));
        guest.setFloat(map.address() + kSliderValue, float(x87.mul(biased, x87.load(hud.mapScale))));
        fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, ts, map.address(), hud.mapMoved[kind]);
    }
    if (Item position = fe::findItem(guest, menu, kPosition))
        guest.setDword(position.address() + fe::item::kNextMenu, next);
    guest::direct<0x44c2d0, frontend::pause::hideSpare>(app, ts, menu);
    if (kind != 0)
        guest::direct<0x44c360, frontend::pause::hideForCop>(app, ts, menu);
    else
        guest::direct<0x44c4e0, frontend::pause::hideForRacer>(app, ts, menu);
}

/* The three's lists set from the choices: all but the first list, which the
 * screens have their own ways with; the fourth choice was read as they opened. */
void placeHudLists(Guest& guest, nfs::u32 screen, nfs::u32 kind, std::uint32_t fourth)
{
    const HudRecord& choices = gameSettings(guest).hudOf(screen, kind);
    const nfs::u32 places = hudPlaces(screen, kind);
    guest.setDword(kHudList2Places + places, placeOf(choices.speedometerForm, kHudList2Last));
    guest.setDword(kHudList3Places + places, placeOf(choices.tachometerForm, kHudList3Last));
    guest.setDword(kHudList4Places + places, placeOf(fourth, kHudList4Last));
}

}  // namespace

/* sub_44d4c0, pshud as it opens. */
nfs::u32 frontend::pause::onHudEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const nfs::u32 kind = playerIsCop(guest, ts, 0);
    const HudRecord& choices = gameSettings(guest).hudOf(0, kind);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, kind == 0 ? 1 : 0);
    openHud(app, ts, guest, menu, 0, kind, kind != 0 ? kNextAloneCop.address : kNextAloneRacer.address);
    placeHudLists(guest, 0, kind, fourth);
    guest.setDword(kHudList1Places + hudPlaces(0, kind), placeOf(choices.splitTimeForm, kHudList1Last));
    return 0;
}

namespace
{

/* Where POSITION leads from the two players' screens. */
nfs::u32 nextOfTwo(nfs::u32 firstIsCop, nfs::u32 secondIsCop)
{
    if (firstIsCop != 0)
        return secondIsCop != 0 ? kNextCopCop.address : kNextCopRacer.address;
    return secondIsCop != 0 ? kNextRacerCop.address : kNextRacerRacer.address;
}

}  // namespace

/* sub_44d7a0, pshud1p as it opens: the first player's display. */
nfs::u32 frontend::pause::onHud1Enter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const nfs::u32 first = playerIsCop(guest, ts, 0);
    const nfs::u32 second = playerIsCop(guest, ts, 1);
    const HudRecord& choices = gameSettings(guest).hudOf(1, first);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(app, ts, guest, menu, 1, first, nextOfTwo(first, second));
    placeHudLists(guest, 1, first, fourth);
    guest.setDword(kHudList1Places + hudPlaces(1, first), placeOf(choices.splitTimeForm, kHudList1Last));
    return 0;
}

/* sub_44dad0, pshud2p as it opens: the second player's.  Its first list's place
 * goes by its own choice's being set, but counts from pshud1p's: the original
 * has it so. */
nfs::u32 frontend::pause::onHud2Enter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    copySettings(guest, kSettingsKept, SETTINGS_ADDRESS(language));
    guest.setDword(kHudShown, 1);
    const nfs::u32 first = playerIsCop(guest, ts, 0);
    const nfs::u32 second = playerIsCop(guest, ts, 1);
    const HudRecord& choices = gameSettings(guest).hudOf(2, second);
    const std::uint32_t fourth = choices.mapStyle;
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(app, ts, guest, menu, 2, second, nextOfTwo(first, second));
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
nfs::u32 onHudExit(Guest& guest, ThreadState& ts, nfs::u32 menu, nfs::u32 screen, nfs::u32 player)
{
    const nfs::u32 kind = playerIsCop(guest, ts, player);
    HudRecord& choices = gameSettings(guest).hudOf(screen, kind);
    const nfs::u32 places = hudPlaces(screen, kind);
    choices.speedometerForm = choiceOf(guest.dword(kHudList2Places + places), kHudList2Last);
    choices.tachometerForm = choiceOf(guest.dword(kHudList3Places + places), kHudList3Last);
    choices.mapStyle = choiceOf(guest.dword(kHudList4Places + places), kHudList4Last);
    choices.splitTimeForm = choiceOf(guest.dword(kHudList1Places + places), kHudList1Last);
    fe::direct<0x4844c0, port::hud::hudPaneSetOf>(guest.app(), ts, ts.sp - 4,
                                                  port::Args{ player, 0, 0, 0 });
    if (closedBy(guest, ts, menu, kHudCancel))
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
constexpr nfs::u32 kCamera1Kept = 0x6788fc, kCamera2Kept = 0x6788f4, kCamera3Kept = 0x6788f8, kCamera4Kept = 0x6788f0;
constexpr nfs::u32 kCameraQuestion = 0x678930;   // char*[1]
constexpr nfs::u32 kCameraButtons = 0x678928;    // char*[2]
constexpr nfs::u32 kCameraMode = 0x5e1054;

constexpr nfs::u32 kCameraAnswered = 0x45d770;
constexpr nfs::u32 kCameraKey = 0x45d840;
constexpr nfs::u32 kBuildCameraCycle = 0x423a60;
constexpr nfs::u32 kRememberMode = 0x423ae0;     // eax a player, edx a mode
constexpr nfs::u32 kSetCameraMode = 0x4238b0;    // likewise

constexpr std::uint32_t kTextSameCamera = 0x1da;
constexpr std::uint32_t kTextAssists = 0x196;
constexpr std::uint32_t kMenuBack = 3;

}  // namespace

/* sub_45d770, the question about two keys on one camera answered: Ok takes each
 * camera off all keys but its first, and leaves. */
nfs::u32 frontend::pause::cameraAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
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
    nfs::u32* const views = gameSettings(guest).cameraViewsChosen;
    const auto clearSame = [](nfs::u32& later, const nfs::u32& earlier) {
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
nfs::u32 frontend::pause::cameraKey(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const nfs::u32* const views = gameSettings(guest).cameraViewsChosen;
    const auto same = [](nfs::u32 a, nfs::u32 b) {
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
nfs::u32 frontend::pause::onCameraEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    dim(guest, ts);
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
nfs::u32 frontend::pause::ensureSomeCamera(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (gameSettings(guest).cameraViewsChosen[0] == 0 && gameSettings(guest).cameraViewsChosen[1] == 0 && gameSettings(guest).cameraViewsChosen[2] == 0
        && gameSettings(guest).cameraViewsChosen[3] == 0)
        gameSettings(guest).cameraViewsChosen[0] = 1;
    return 0;
}

/* sub_45d9e0, pscamera as it closes: Cancel puts them back; otherwise the race
 * takes them up at once. */
nfs::u32 frontend::pause::onCameraExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (closedBy(guest, ts, menu, kCameraCancel))
    {
        gameSettings(guest).cameraViewsChosen[0] = guest.dword(kCamera1Kept);
        gameSettings(guest).cameraViewsChosen[1] = guest.dword(kCamera2Kept);
        gameSettings(guest).cameraViewsChosen[2] = guest.dword(kCamera3Kept);
        gameSettings(guest).cameraViewsChosen[3] = guest.dword(kCamera4Kept);
        return 0;
    }
    fe::direct<0x472300, load::cameraViewsSet>(app, ts);
    fe::direct<kBuildCameraCycle, carstate::cycleBuilt>(guest.app(), guest.ts());
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.ts(), 0, guest.dword(kCameraMode));
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.ts(), 1, guest.dword(kCameraMode));
    if (guest.dword(kInRace) != 0)
    {
        fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), 0, guest.dword(kCameraMode));
        if (gameSettings(guest).playMode == 1)
            fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), 1, guest.dword(kCameraMode));
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

constexpr nfs::u32 kQuitToLeaveTo = 0x55fdc8;
constexpr nfs::u32 kQuitToQuestion = 0x7a3be0;   // char*[1]
constexpr nfs::u32 kQuitToButtons = 0x7a3be4;    // char*[2]
constexpr nfs::u32 kQuitToAnswered = 0x4b67b0;
constexpr nfs::u32 kQuitReplayClicked = 0x4b6870;
constexpr nfs::u32 kQuitPostClicked = 0x4b6890;
constexpr nfs::u32 kQuitMainClicked = 0x4b68d0;
constexpr nfs::u32 kQuitSystemClicked = 0x4b6910;

}  // namespace

/* sub_4b6810: "are you sure?" for a button of quitto. */
nfs::u32 frontend::pause::quitToAsk(App* app, ThreadState& ts, u32 question, u32 leaveTo)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest.setDword(kQuitToLeaveTo, leaveTo);
    guest.setDword(kQuitToQuestion, fe::text(guest, question));
    guest.setDword(kQuitToButtons, fe::text(guest, kTextYes));
    guest.setDword(kQuitToButtons + 4, fe::text(guest, kTextNo));
    return openQuestionAnswer(guest, { 1, kQuitToQuestion, 2, kQuitToButtons, 0, 0, 0, kQuitToAnswered });
}

/* sub_4b6930, quitto as it opens: a network race has the one way back, any
 * other the two. */
nfs::u32 frontend::pause::onQuitToEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const auto offer = [&](const GuestString& codelink, nfs::u32 onClick) {
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

constexpr nfs::u32 kFullRect = 0x7cda60;         // four ints
constexpr nfs::u32 kOnePlayerRect = 0x7cda90;
constexpr nfs::u32 kTwoPlayersRect = 0x7cdb98;

}  // namespace

/* sub_4be3d0: the full rectangle and the view's -- for one player the first's
 * across, for two its own -- into the corners of the quads drawn over the
 * screen, as floats. */
nfs::u32 frontend::pause::fillViewQuads(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const std::int32_t left = std::int32_t(guest.dword(kFullRect));
    const std::int32_t top = std::int32_t(guest.dword(kFullRect + 4));
    const std::int32_t right = std::int32_t(guest.dword(kFullRect + 8));
    const std::int32_t bottom = std::int32_t(guest.dword(kFullRect + 0xc));
    std::int32_t viewLeft, viewRight, viewTop, viewBottom;
    if (gameSettings(guest).playMode == 1)
    {
        viewLeft = std::int32_t(guest.dword(kOnePlayerRect));
        viewRight = std::int32_t(guest.dword(kOnePlayerRect + 8) + nfs::u32(viewLeft));
        viewTop = top;
        viewBottom = bottom;
    }
    else
    {
        viewLeft = std::int32_t(guest.dword(kTwoPlayersRect));
        viewRight = std::int32_t(guest.dword(kTwoPlayersRect + 8) + nfs::u32(viewLeft));
        viewTop = std::int32_t(guest.dword(kTwoPlayersRect + 4));
        viewBottom = std::int32_t(guest.dword(kTwoPlayersRect + 0xc) + nfs::u32(viewTop));
    }
    const auto setCorners = [&](std::int32_t value, std::initializer_list<nfs::u32> corners) {
        for (const nfs::u32 at : corners)
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
    const nfs::u32 corner = guest.dword(0x55fef8);
    guest.setDword(0x55ff18, corner);
    guest.setDword(0x55ff38, corner);
    return corner;
}

namespace
{

/* sub_4beb00, sub_4beb40: a rectangle's first or second half set. */
void setRectHalf(Guest& guest, nfs::u32 half, nfs::u32 a, nfs::u32 b, nfs::u32 c, nfs::u32 d)
{
    guest.setDword(half, a);
    guest.setDword(half + 4, b);
    guest.setDword(half + 8, c);
    guest.setDword(half + 0xc, d);
}

}  // namespace

/* sub_4beb20: a rectangle read, each of its four through a pointer. */
nfs::u32 frontend::pause::readRect(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d)
{
    Guest guest(app, ts);
    guest.setDword(a, guest.dword(rect));
    guest.setDword(b, guest.dword(rect + 4));
    guest.setDword(c, guest.dword(rect + 8));
    const nfs::u32 last = guest.dword(rect + 0xc);
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
nfs::u32 wholeForNone(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts); });
}

/* A function of eax: a screen, an item, a name. */
template <typename Core>
nfs::u32 wholeForOne(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 eax)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, eax); });
}

/* A function of eax and edx: a dialog's callback, say. */
template <typename Core>
nfs::u32 wholeForTwo(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 eax, nfs::u32 edx)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, eax, edx); });
}

/* A rectangle's: the rectangle in eax, three in edx, ebx, ecx, the fourth on the stack. */
template <typename Core>
nfs::u32 wholeForRect(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 rect, nfs::u32 a, nfs::u32 b, nfs::u32 c)
{
    const nfs::u32 d = Guest(app, ts).dword(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, rect, a, b, c, d); });
}

/* A screen's empty handler. */
nfs::u32 wholeNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, fe::Site& site)
{
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
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
nfs::u32 frontend::pause::writeSeriesSave(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x100);
    return writeSave(app, ts, guest, name, kSeriesPath);
}

nfs::u32 frontend::pause::ghostExists(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 + 0x100);
    return keptExists(guest, ts, name, kGhostExtension);
}

nfs::u32 frontend::pause::writeTournamentSave(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x100);
    return writeSave(app, ts, guest, name, kTournamentPath);
}

nfs::u32 frontend::pause::tournamentSaveExists(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 + 0x100);
    return keptExists(guest, ts, name, kTournamentExtension);
}

nfs::u32 frontend::pause::replayExists(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 + 0x100);
    return keptExists(guest, ts, name, kReplayExtension);
}

/* A yes or a no answered (yesOrNoAnswered), with what the Yes hands the menu loop. */
nfs::u32 frontend::pause::leaveAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kLeaveTo); });
}

nfs::u32 frontend::pause::restartAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return yesOrNoAnswered(guest, answer, close, [] { return kAnswerRestart; });
}

nfs::u32 frontend::pause::quitToAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kQuitToLeaveTo); });
}

/* The map's size moved (mapSizeMoved): the constant its value is scaled by, the size it sets. */
nfs::u32 frontend::pause::mapCopMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x53932c, gameSettings(guest).hud[1].mapScale);
}

nfs::u32 frontend::pause::mapRacerMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x539330, gameSettings(guest).hud[0].mapScale);
}

nfs::u32 frontend::pause::mapFirstCopMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x539334, gameSettings(guest).hud[3].mapScale);
}

nfs::u32 frontend::pause::mapFirstRacerMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x539338, gameSettings(guest).hud[2].mapScale);
}

nfs::u32 frontend::pause::mapSecondCopMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x53933c, gameSettings(guest).hud[5].mapScale);
}

nfs::u32 frontend::pause::mapSecondRacerMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return mapSizeMoved(guest, ts, item, 0x539340, gameSettings(guest).hud[4].mapScale);
}

/* A display's screen as it closes (onHudExit): which screen, whose kind of car. */
nfs::u32 frontend::pause::hudExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return onHudExit(guest, ts, menu, 0, 0);
}

nfs::u32 frontend::pause::hud1Exit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return onHudExit(guest, ts, menu, 1, 0);
}

nfs::u32 frontend::pause::hud2Exit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    return onHudExit(guest, ts, menu, 2, 1);
}

/* quitto's buttons: "are you sure?" about where each goes. */
nfs::u32 frontend::pause::quitToReplay(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x4b6810, quitToAsk>(app, ts, kAskQuitNetwork, kLeaveToPostGame);
    return fe::kDialogHandled;
}

nfs::u32 frontend::pause::quitToPost(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x4b6810, quitToAsk>(app, ts, askQuitRace(guest), kLeaveToPostGame);
    return fe::kDialogHandled;
}

nfs::u32 frontend::pause::quitToMain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x4b6810, quitToAsk>(app, ts, askQuitRace(guest), kLeaveToMain);
    return fe::kDialogHandled;
}

nfs::u32 frontend::pause::quitToSystem(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x4b6810, quitToAsk>(app, ts, kAskQuitToSystem, kLeaveToSystem);
    return fe::kDialogHandled;
}

/* A rectangle's first or second half set (setRectHalf). */
nfs::u32 frontend::pause::setRectFirst(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d)
{
    Guest guest(app, ts);
    setRectHalf(guest, rect, a, b, c, d);
    return rect;
}

nfs::u32 frontend::pause::setRectSecond(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d)
{
    Guest guest(app, ts);
    setRectHalf(guest, rect + 0x10, a, b, c, d);
    return rect;
}

/* The saves. */
nfs::u32 frontend::pauseSeriesSaveExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446510", 0, 0, kSeriesExistsStrings, std::size(kSeriesExistsStrings), kNothing };
    return wholeForOne(app, ts, args, site, pause::seriesSaveExists, args.eax);
}

nfs::u32 frontend::pauseWriteSeriesSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446550", 0, 0, kSeriesWriteStrings, std::size(kSeriesWriteStrings) };
    return wholeForOne(app, ts, args, site, pause::writeSeriesSave, args.eax);
}

nfs::u32 frontend::pauseGhostExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446740", 0, 0, kGhostExistsStrings, std::size(kGhostExistsStrings), kNothing };
    return wholeForOne(app, ts, args, site, pause::ghostExists, args.eax);
}

nfs::u32 frontend::pauseWriteTournamentSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4468c0", 0, 0, kTournamentWriteStrings, std::size(kTournamentWriteStrings) };
    return wholeForOne(app, ts, args, site, pause::writeTournamentSave, args.eax);
}

nfs::u32 frontend::pauseTournamentSaveExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446950", 0, 0, kTournamentExistsStrings, std::size(kTournamentExistsStrings),
                          kNothing };
    return wholeForOne(app, ts, args, site, pause::tournamentSaveExists, args.eax);
}

nfs::u32 frontend::pauseReplayExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446a90", 0, 0, kReplayExistsStrings, std::size(kReplayExistsStrings), kNothing };
    return wholeForOne(app, ts, args, site, pause::replayExists, args.eax);
}

nfs::u32 frontend::pauseCheckSaveable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_478010", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return wholeForOne(app, ts, args, site, pause::checkSaveable, args.eax);
}

nfs::u32 frontend::pauseOverwriteAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447800", 0, 0, nullptr, 0, kEbxEcxEsi };
    return wholeForTwo(app, ts, args, site, pause::overwriteAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseSaveNameAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4478c0", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    return wholeForTwo(app, ts, args, site, pause::saveNameAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseAskSaveName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447a30", 0, 0, nullptr, 0 };
    return wholeForOne(app, ts, args, site, pause::askSaveName, args.eax);
}

/* pause. */
nfs::u32 frontend::pauseHideSpare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c2d0", 0, 0, kHideMe, std::size(kHideMe), kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::hideSpare, args.eax);
}

nfs::u32 frontend::pauseHideForCop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c360", 0, 0, kRacerInstruments, std::size(kRacerInstruments), kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::hideForCop, args.eax);
}

nfs::u32 frontend::pauseHideForRacer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c4e0", 0, 0, kCopInstrumentsFirst, std::size(kCopInstrumentsFirst), kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::hideForRacer, args.eax);
}

nfs::u32 frontend::pauseLeaveAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c610", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, ts, args, site, pause::leaveAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseRestartAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c670", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, ts, args, site, pause::restartAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseAskLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c6d0", 0, 0, nullptr, 0, kEbxEcx };
    return wholeForTwo(app, ts, args, site, pause::askLeave, args.eax, args.edx);
}

nfs::u32 frontend::pauseQuitChoiceAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c730", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, ts, args, site, pause::quitChoiceAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseQuitClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44c7b0", 0, 0, nullptr, 0 };
    return wholeForNone(app, ts, args, site, pause::onQuitClicked);
}

nfs::u32 frontend::pauseRestartClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ca10", 0, 0, nullptr, 0, kEbxEcxEdx };
    return wholeForNone(app, ts, args, site, pause::onRestartClicked);
}

nfs::u32 frontend::pauseSaveClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ca80", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, ts, args, site, pause::onSaveClicked);
}

nfs::u32 frontend::pauseOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44cab0", 0, 0, kPauseStrings, std::size(kPauseStrings) };
    return wholeForOne(app, ts, args, site, pause::onPauseEnter, args.eax);
}

nfs::u32 frontend::pauseOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44cf60", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, ts, args, site);
}

/* The sliders. */
nfs::u32 frontend::pauseIntensityMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44cf70", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::intensityMoved, args.eax);
}

nfs::u32 frontend::pauseMapCopMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44cfa0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapCopMoved, args.eax);
}

nfs::u32 frontend::pauseMapRacerMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44cfe0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapRacerMoved, args.eax);
}

nfs::u32 frontend::pauseMapFirstCopMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d020", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapFirstCopMoved, args.eax);
}

nfs::u32 frontend::pauseMapFirstRacerMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d060", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapFirstRacerMoved, args.eax);
}

nfs::u32 frontend::pauseMapSecondCopMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d0a0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapSecondCopMoved, args.eax);
}

nfs::u32 frontend::pauseMapSecondRacerMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d0e0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::mapSecondRacerMoved, args.eax);
}

/* psoption, psoptn2, pscntl1. */
nfs::u32 frontend::pauseOptionsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d120", 0, 0, kOptionsStrings, std::size(kOptionsStrings), kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::onOptionsEnter, args.eax);
}

nfs::u32 frontend::pauseOptionsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d160", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, ts, args, site);
}

nfs::u32 frontend::pauseControlsCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d170", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 item = args.eax, menu = args.edx;
    const std::uint16_t key = std::uint16_t(args.ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return pause::cancelKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::pauseControlsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d190", 0, 0, kControlsStrings, std::size(kControlsStrings), kEbxEcxEdx };
    return wholeForOne(app, ts, args, site, pause::onControlsEnter, args.eax);
}

nfs::u32 frontend::pauseControlsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d220", 0, 0, nullptr, 0, kEbxEcxEdx };
    return wholeForNone(app, ts, args, site, pause::onControlsExit);
}

/* psgraph. */
nfs::u32 frontend::pauseWidescreenClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d270", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, ts, args, site, pause::onWidescreenClicked);
}

nfs::u32 frontend::pauseGraphicsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d2a0", 0, 0, kGraphicsStrings, std::size(kGraphicsStrings) };
    return wholeForOne(app, ts, args, site, pause::onGraphicsEnter, args.eax);
}

nfs::u32 frontend::pauseGraphicsOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d3e0", 0, 0, kProjectedStrings, std::size(kProjectedStrings), fe::kRestoreEdx };
    return wholeForOne(app, ts, args, site, pause::fitProjected, args.eax);
}

nfs::u32 frontend::pauseGraphicsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d420", 0, 0, kGraphicsExitStrings, std::size(kGraphicsExitStrings) };
    return wholeForOne(app, ts, args, site, pause::onGraphicsExit, args.eax);
}

/* pshud, pshud1p, pshud2p. */
nfs::u32 frontend::pauseHudOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d4c0", 0, 0, kHudStrings, std::size(kHudStrings) };
    return wholeForOne(app, ts, args, site, pause::onHudEnter, args.eax);
}

nfs::u32 frontend::pauseHudOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d670", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, ts, args, site, pause::hudExit, args.eax);
}

nfs::u32 frontend::pauseHud1OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d7a0", 0, 0, kHud2Strings, std::size(kHud2Strings) };
    return wholeForOne(app, ts, args, site, pause::onHud1Enter, args.eax);
}

nfs::u32 frontend::pauseHud1OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44d9a0", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, ts, args, site, pause::hud1Exit, args.eax);
}

nfs::u32 frontend::pauseHud2OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44dad0", 0, 0, kHud2Strings, std::size(kHud2Strings) };
    return wholeForOne(app, ts, args, site, pause::onHud2Enter, args.eax);
}

nfs::u32 frontend::pauseHud2OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44dcd0", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    return wholeForOne(app, ts, args, site, pause::hud2Exit, args.eax);
}

/* aituner. */
nfs::u32 frontend::pauseAiTunerOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e130", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, ts, args, site);
}

nfs::u32 frontend::pauseAiTunerOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e140", 0, 4, nullptr, 0, kNothing };
    return wholeNothing(app, ts, args, site);
}

/* pscamera. */
nfs::u32 frontend::pauseCameraAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d770", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    return wholeForTwo(app, ts, args, site, pause::cameraAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseCameraKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d840", 0, 0, nullptr, 0 };
    return wholeForNone(app, ts, args, site, pause::cameraKey);
}

nfs::u32 frontend::pauseCameraOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d920", 0, 0, kCameraStrings, std::size(kCameraStrings), kEcxEdx };
    return wholeForOne(app, ts, args, site, pause::onCameraEnter, args.eax);
}

nfs::u32 frontend::pauseCameraEnsureOne(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d9a0", 0, 0, nullptr, 0, kNothing };
    return wholeForNone(app, ts, args, site, pause::ensureSomeCamera);
}

nfs::u32 frontend::pauseCameraOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d9e0", 0, 0, kCameraExitStrings, std::size(kCameraExitStrings),
                          fe::kRestoreEbx | fe::kRestoreEdx };
    return wholeForOne(app, ts, args, site, pause::onCameraExit, args.eax);
}

/* quitto. */
nfs::u32 frontend::pauseQuitToAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b67b0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForTwo(app, ts, args, site, pause::quitToAnswered, args.eax, args.edx);
}

nfs::u32 frontend::pauseQuitToAsk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6810", 0, 0, nullptr, 0, kEbxEcx };
    return wholeForTwo(app, ts, args, site, pause::quitToAsk, args.eax, args.edx);
}

nfs::u32 frontend::pauseQuitToReplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6870", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, pause::quitToReplay);
}

nfs::u32 frontend::pauseQuitToPost(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6890", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, pause::quitToPost);
}

nfs::u32 frontend::pauseQuitToMain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b68d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, pause::quitToMain);
}

nfs::u32 frontend::pauseQuitToSystem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6910", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, pause::quitToSystem);
}

nfs::u32 frontend::pauseQuitToOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6930", 0, 0, kQuitToStrings, std::size(kQuitToStrings), kEbxEcxEdx };
    return wholeForOne(app, ts, args, site, pause::onQuitToEnter, args.eax);
}

/* The race's views. */
nfs::u32 frontend::pauseFillViewQuads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4be3d0", 0, 0, nullptr, 0 };
    return wholeForNone(app, ts, args, site, pause::fillViewQuads);
}

nfs::u32 frontend::pauseSetRectFirst(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4beb00", 4, 0, nullptr, 0, kNothing };
    return wholeForRect(app, ts, args, site, pause::setRectFirst, args.eax, args.edx, args.ebx, args.ecx);
}

nfs::u32 frontend::pauseReadRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4beb20", 4, 0, nullptr, 0, fe::kRestoreEsi };
    return wholeForRect(app, ts, args, site, pause::readRect, args.eax, args.edx, args.ebx, args.ecx);
}

nfs::u32 frontend::pauseSetRectSecond(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4beb40", 4, 0, nullptr, 0, kNothing };
    return wholeForRect(app, ts, args, site, pause::setRectSecond, args.eax, args.edx, args.ebx, args.ecx);
}

}
