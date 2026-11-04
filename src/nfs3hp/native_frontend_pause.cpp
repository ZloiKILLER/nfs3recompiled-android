#include "native_frontend.h"
#include <nfs3hp.h>
#include <iterator>

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
 * views that pause reads (sub_4beb20) and their small helpers. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

// The game's state.
constexpr x86::reg32 kPlayers = 0x6fd3b0;          // 1 alone, 2 on a split screen
constexpr x86::reg32 kRaceType = 0x6fd3b8;         // 0 a single race, 1 and 2 the series, 3 another
constexpr x86::reg32 kNetPlayers = 0x6fd3a0;       // a network race's; 2 or more is one
constexpr x86::reg32 kNetworkHost = 0x7a22da;      // int16
constexpr x86::reg32 kNetworkJoin = 0x7a22e4;
constexpr x86::reg32 kInRace = 0x559234;           // the menus are a race's pause screens
constexpr x86::reg32 kNoSound = 0x6fbb54;
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kTrackLightingFlags = 0x06;
constexpr x86::reg32 kDisplayBuffering = 0x7a3a68;
constexpr x86::reg32 kProjectedText = 0x6fbc34;
constexpr x86::reg32 kDeveloperFlags = 0x55eb2e;   // byte
constexpr std::uint8_t kDeveloperQuit = 0x20;

// The settings, and their copy that Cancel puts back.
constexpr x86::reg32 kSettings = 0x6fbb4c;
constexpr x86::reg32 kSettingsKept = 0x662ca0;
constexpr x86::reg32 kSettingsSize = 0x176c;
constexpr x86::reg32 kScreenSize = 0x6fbc4c;       // kSettings' Screen Size
constexpr x86::reg32 kScreenSizeKept = 0x662da0;
constexpr x86::reg32 kIntensity = 0x6fbc38;        // the effects' intensity, a float

// Fields of items the header does not name.
constexpr x86::reg32 kItemOnKey = 0x30;            // a button's key handler
constexpr x86::reg32 kSliderValue = 0x54;          // a slider's value, a float
constexpr std::uint32_t kButton = 5;               // an item's type (item::kType)

// The game's functions.
constexpr x86::reg32 kDimBackground = 0x451530;    // on the stack, the float's bits
constexpr std::uint32_t kDimBy = 0x3f19999a;       // 0.6
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;
constexpr x86::reg32 kOpenMessage = 0x444a80;
constexpr x86::reg32 kShownNow = 0x44a660;
constexpr std::uint32_t kShownAlways = 0xf;
constexpr x86::reg32 kSliderValueOf = 0x46c9b0;    // a float on the x87
constexpr x86::reg32 kSliderSetOnMove = 0x46cac0;
constexpr x86::reg32 kButtonKey = 0x461270;
constexpr x86::reg32 kMemmove = 0x4ea4f0;          // eax from, edx to, ebx bytes
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kCameraTarget = 0x422bd0;     // eax a player: the car
constexpr x86::reg32 kCarFlags = 0x200;            // byte of the car
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr x86::reg32 kControlsFor = 0x4904d0;      // eax the players
constexpr x86::reg32 kForcePlayerCar = 0x4710f0;
constexpr x86::reg32 kBindPlayersControls = 0x48f1f0;
constexpr x86::reg32 kLayoutViews = 0x4be5e0;      // eax 1: for a cop's car
constexpr x86::reg32 kHudPaneSet = 0x4844c0;

// This group's own, called by address so that each stays a call.
constexpr x86::reg32 kCheckSaveable = 0x478010;
constexpr x86::reg32 kAskSaveName = 0x447a30;
constexpr x86::reg32 kSaveNameAnswered = 0x4478c0;
constexpr x86::reg32 kOverwriteAnswered = 0x447800;
constexpr x86::reg32 kHideSpare = 0x44c2d0;
constexpr x86::reg32 kHideForCop = 0x44c360;
constexpr x86::reg32 kHideForRacer = 0x44c4e0;
constexpr x86::reg32 kAskLeaveAnswered = 0x44c610;
constexpr x86::reg32 kAskLeave = 0x44c6d0;
constexpr x86::reg32 kQuitChoiceAnswered = 0x44c730;
constexpr x86::reg32 kOnQuitClicked = 0x44c7b0;
constexpr x86::reg32 kOnRestartClicked = 0x44ca10;
constexpr x86::reg32 kRestartAnswered = 0x44c670;
constexpr x86::reg32 kOnSaveClicked = 0x44ca80;
constexpr x86::reg32 kCancelKey = 0x44d170;
constexpr x86::reg32 kOnWidescreenClicked = 0x44d270;
constexpr x86::reg32 kIntensityMoved = 0x44cf70;
constexpr x86::reg32 kReadRect = 0x4beb20;

/* The settings copied as they stand: into the keeping, back out of it. */
void copySettings(Guest& guest, x86::reg32 to, x86::reg32 from)
{
    for (x86::reg32 at = 0; at < kSettingsSize; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
}

/* The race behind a pause screen, darkened. */
void dim(Guest& guest)
{
    guest.call(kDimBackground, fe::args({}, { kDimBy }));
}

void hideItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    if (Item item = fe::findItem(guest, menu, codelink))
        item.setHidden(true);
}

void showItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    if (Item item = fe::findItem(guest, menu, codelink))
        item.setHidden(false);
}

void disableItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    if (Item item = fe::findItem(guest, menu, codelink))
        item.setDisabled(true);
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
bool closedBy(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    const x86::reg32 item = fe::item::kTable + fe::item::kSize * x86::reg32(first + selected);
    return guest.dword(item + fe::item::kType) == kButton
        && guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), codelink.address })) != 0;
}

/* The car a player looks from is a cop's: 1, or 0. */
x86::reg32 playerIsCop(Guest& guest, x86::reg32 player)
{
    const x86::reg32 car = guest.call(kCameraTarget, fe::args({ player }));
    return (guest.byte(car + kCarFlags) & kCarIsCop) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * The saves: whether a file stands, and a game written
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kFileExists = 0x4e0e60;
constexpr x86::reg32 kReserve = 0x4e1620;          // eax a name, edx bytes, ebx flags
constexpr x86::reg32 kSaveFile = 0x4eeae0;         // eax a path, edx what, ebx bytes
constexpr x86::reg32 kPurge = 0x4e1890;
constexpr x86::reg32 kSavesFolder = 0x7a2f34;
constexpr x86::reg32 kSaveImage = 0x6fbb40;        // the whole of a save: its size first
constexpr std::uint32_t kSaveImageSize = 0x20f0;
constexpr std::uint32_t kReserveFlags = 0x10;

constexpr GuestString kSeriesPath{ 0x538bdc, "%s%s.kot" };
constexpr GuestString kNoName{ 0x538be8, "" };
constexpr GuestString kTournamentPath{ 0x538c04, "%s%s.trn" };
constexpr GuestString kKeptPath{ 0x538bf0, "%s%s.%s" };
constexpr GuestString kGhostExtension{ 0x538bec, "gst" };
constexpr GuestString kTournamentExtension{ 0x538c10, "trn" };
constexpr GuestString kReplayExtension{ 0x538c14, "rp3" };

/* sub_446510: a series' game by this name stands in the saves' folder. */
x86::reg32 seriesSaveExists(Guest& guest, x86::reg32 name)
{
    const x86::reg32 path = guest.frame();   // char[0x100]
    guest.call(kSprintf, fe::args({}, { path, kSeriesPath.address, kSavesFolder, name }));
    return guest.call(kFileExists, fe::args({ path })) != 0 ? 1 : 0;
}

/* sub_446740, sub_446950, sub_446a90: a save of this kind by this name stands. */
x86::reg32 keptExists(Guest& guest, x86::reg32 name, const GuestString& extension)
{
    const x86::reg32 path = guest.frame();   // char[0x100]
    guest.call(kSprintf, fe::args({}, { path, kKeptPath.address, kSavesFolder, name, extension.address }));
    return guest.call(kFileExists, fe::args({ path })) != 0 ? 1 : 0;
}

/* sub_446550, sub_4468c0: the game written under this name: a copy of it, its
 * cars' names filled in, to the file.  1 when it went out. */
x86::reg32 writeSave(Guest& guest, x86::reg32 name, const GuestString& pathFormat)
{
    const x86::reg32 path = guest.frame();   // char[0x100]
    guest.call(kSprintf, fe::args({}, { path, pathFormat.address, kSavesFolder, name }));
    const x86::reg32 copy = guest.call(kReserve, fe::args({ kNoName.address, kSaveImageSize, kReserveFlags }));
    if (copy == 0)
        return 0;
    guest.call(kMemmove, fe::args({ kSaveImage, copy, kSaveImageSize }));
    x86::reg32 written = 0;
    if (guest.call(kCheckSaveable, fe::args({ copy })) != 0)
    {
        guest.call(kSaveFile, fe::args({ path, copy, kSaveImageSize }));
        written = 1;
    }
    guest.call(kPurge, fe::args({ copy }));
    return written;
}

// A save's cars, by number, and the room for their names.
constexpr x86::reg32 kCarName = 0x439520;          // eax a car: its name
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

/* sub_478010: a save about to be written is one, and each of its cars has a
 * name: filled in.  1 when all of it held. */
x86::reg32 checkSaveable(Guest& guest, x86::reg32 save)
{
    if (guest.dword(save) != kSaveImageSize)
        return 0;
    const x86::reg32 first = guest.call(kCarName, fe::args({ guest.dword(save + kSavedFirstCar) }));
    if (first == 0)
        return 0;
    copyCarName(guest, save + kSavedFirstCarName, first);
    const x86::reg32 second = guest.call(kCarName, fe::args({ guest.dword(save + kSavedSecondCar) }));
    if (second == 0)
        return 0;
    copyCarName(guest, save + kSavedSecondCarName, second);
    const std::int32_t third = std::int32_t(guest.dword(save + kSavedThirdCar));
    if (third >= 4)
    {
        const x86::reg32 name = guest.call(kCarName, fe::args({ x86::reg32(third - 4) }));
        if (name == 0)
            return 0;
        copyCarName(guest, save + kSavedThirdCarName, name);
    }
    return 1;
}

/* ---------------------------------------------------------------------------
 * The saves: the name asked for, and whether to write over what stands
 * ------------------------------------------------------------------------- */

// The kind being saved, what writes it and what looks for it.
constexpr x86::reg32 kSaveKind = 0x5591e8;
constexpr x86::reg32 kWriteSave = 0x5591cc;
constexpr x86::reg32 kSaveExists = 0x5591d0;
constexpr x86::reg32 kSaveName = 0x5591d4;
constexpr x86::reg32 kPlayerName = 0x6fd2fc;
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

/* sub_447a30: the name to save under asked for, the player's own at first; a
 * kind past the four leaves what the last one chose. */
x86::reg32 askSaveName(Guest& guest, x86::reg32 kind)
{
    guest.setDword(kSaveKind, kind);
    if (kind < std::size(kSaveKinds))
    {
        guest.setDword(kNameQuestion, fe::text(guest, kSaveKinds[kind].text));
        guest.setDword(kWriteSave, kSaveKinds[kind].write);
        guest.setDword(kSaveExists, kSaveKinds[kind].exists);
    }
    guest.setDword(kNameButtons, fe::text(guest, kTextOk));
    guest.setDword(kNameButtons + 4, fe::text(guest, kTextCancel));
    guest.copyString(kSaveName, kPlayerName);
    fe::openDialog(guest, { 1, kNameQuestion, 2, kNameButtons, 0, kSaveName, kSaveNameLength, 0, kSaveNameAnswered });
    return guest.eax();
}

/* sub_447800, the question whether to write over the file answered: Yes writes
 * and tells how it went, No and Escape ask for another name. */
x86::reg32 overwriteAnswered(Guest& guest, x86::reg32 answer, x86::reg32 keepOpen)
{
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(keepOpen, 0);
    if (answer != fe::kDialogEscape && answer != fe::kDialogEnter)
        return 0;
    if (answer == fe::kDialogEscape || focus == 1)
    {
        guest.call(kAskSaveName, fe::args({ guest.dword(kSaveKind) }));
        guest.setByte(keepOpen, 0);
        return 0;
    }
    if (focus != 0)
        return 0;
    const bool written = guest.call(guest.dword(kWriteSave), fe::args({ kSaveName })) != 0;
    guest.setDword(kOverwriteResult, kSaveName);
    guest.setDword(kOverwriteResult + 4, fe::text(guest, written ? kTextSaved : kTextNotSaved));
    guest.call(kOpenMessage, fe::args({ 2, kOverwriteResult, kResultTime, answer }, { 0 }));
    guest.setByte(keepOpen, 0);
    return 0;
}

/* sub_4478c0, the name dialog answered: a name that stands asks whether to
 * write over it, a new one is written at once. */
x86::reg32 saveNameAnswered(Guest& guest, x86::reg32 answer, x86::reg32 close)
{
    const std::uint32_t focus = fe::dialogFocus(guest);
    guest.setByte(close, 0);
    const x86::reg32 typed = fe::dialogEditText(guest);
    guest.leaveEdx(typed);
    if (answer == fe::kDialogEscape || (answer == fe::kDialogEnter && focus == 1))
    {
        guest.setByte(close, 1);
        return 0;
    }
    if (answer != fe::kDialogEnter || focus != 0 || typed == 0 || guest.byte(typed) == 0)
        return 0;
    if (guest.call(guest.dword(kSaveExists), fe::args({ typed })) == 0)
    {
        guest.copyString(kSaveName, typed);
        const bool written = guest.call(guest.dword(kWriteSave), fe::args({ typed })) != 0;
        const x86::reg32 line = fe::text(guest, written ? kTextSaved : kTextNotSaved);
        guest.setDword(kWrittenResult, line);
        guest.setDword(kWrittenResult + 4, kSaveName);
        guest.call(kOpenMessage, fe::args({ 2, kWrittenResult, kResultTime, answer }, { 0 }));
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

/* sub_44c2d0: the six spare items hidden, as far as the screen has them.  The
 * last one found, or 0. */
x86::reg32 hideSpare(Guest& guest, x86::reg32 menu)
{
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
x86::reg32 hideForCop(Guest& guest, x86::reg32 menu)
{
    for (const GuestString& codelink : kRacerInstruments)
        hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopGreyed)
        disableItem(guest, menu, codelink);
    return guest.eax();
}

/* sub_44c4e0: a racer's display: a cop's items hidden. */
x86::reg32 hideForRacer(Guest& guest, x86::reg32 menu)
{
    for (const GuestString& codelink : kCopInstrumentsFirst)
        hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopGreyed)
        hideItem(guest, menu, codelink);
    for (const GuestString& codelink : kCopInstrumentsLast)
        hideItem(guest, menu, codelink);
    return guest.eax();
}

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

/* sub_44c6d0: "are you sure?" asked for a choice of the quit box, No first. */
x86::reg32 askLeave(Guest& guest, std::uint32_t question, std::uint32_t leaveTo)
{
    guest.setDword(kLeaveTo, leaveTo);
    guest.setDword(kLeaveQuestion, fe::text(guest, question));
    guest.setDword(kLeaveButtons, fe::text(guest, kTextYes));
    guest.setDword(kLeaveButtons + 4, fe::text(guest, kTextNo));
    fe::openQuestion(guest, { 1, kLeaveQuestion, 2, kLeaveButtons, 0, 1, 0, kAskLeaveAnswered });
    return guest.eax();
}

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
    const std::uint32_t type = guest.dword(kRaceType);
    return type == 1 ? kAskQuitSeries : type == 2 ? kAskQuitTournament : kAskQuitRace;
}

/* sub_44c730, the quit box answered: its button's question, or Cancel. */
x86::reg32 quitChoiceAnswered(Guest& guest, x86::reg32 answer, x86::reg32 close)
{
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
            guest.call(kAskLeave, fe::args({ guest.dword(kQuitButtonQuestion + 4 * focus),
                                             guest.dword(kQuitButtonAnswer + 4 * focus) }));
            closeBox(0);
        }
    }
    guest.leaveEdx(result);
    return result;
}

/* sub_44c7b0, Quit clicked on pause: where to, by what kind of a race it is. */
x86::reg32 onQuitClicked(Guest& guest)
{
    const bool splitScreen = std::int32_t(guest.dword(kPlayers)) > 1 && guest.word(kNetworkHost) == 0;
    const bool network = std::int32_t(guest.dword(kNetPlayers)) >= 2;
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

constexpr x86::reg32 kRestartQuestion = 0x664434;  // char*[1]
constexpr x86::reg32 kRestartButtons = 0x664438;   // char*[2]
constexpr std::uint32_t kTextRestart = 0x1ea;

/* sub_44ca10, Restart clicked. */
x86::reg32 onRestartClicked(Guest& guest)
{
    guest.setDword(kRestartQuestion, fe::text(guest, kTextRestart));
    guest.setDword(kRestartButtons, fe::text(guest, kTextYes));
    guest.setDword(kRestartButtons + 4, fe::text(guest, kTextNo));
    fe::openQuestion(guest, { 1, kRestartQuestion, 2, kRestartButtons, 0, 0, 0, kRestartAnswered });
    return fe::kDialogHandled;
}

/* sub_44ca80, Save clicked: the name asked for, of the kind of save the race's
 * type keeps. */
x86::reg32 onSaveClicked(Guest& guest)
{
    const std::uint32_t type = guest.dword(kRaceType);
    if (type == 1)
        guest.call(kAskSaveName, fe::args({ 1 }));
    else if (type == 2)
        guest.call(kAskSaveName, fe::args({ 0 }));
    return fe::kDialogHandled;
}

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
    const std::uint32_t type = guest.dword(kRaceType);
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

/* sub_44cab0, pause as it opens. */
x86::reg32 onPauseEnter(Guest& guest, x86::reg32 menu)
{
    // A view's rectangle comes back into these two; its width is what counts.
    const x86::reg32 rectOther = guest.frame();
    const x86::reg32 rectWidth = guest.frame() + 4;
    guest.setDword(rectOther, 0);
    guest.setDword(rectWidth, 0);
    dim(guest);

    if (Item save = fe::findItem(guest, menu, kSaveGame))
    {
        if (guest.dword(kPlayers) != 1)
            guest.setWord(save.address() + fe::item::kY, kSaveButtonSplitY);
        const std::uint32_t type = guest.dword(kRaceType);
        if (std::int32_t(guest.dword(kPlayers)) <= 1 && guest.dword(kNetPlayers) == 0 && (type == 1 || type == 2))
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

    if (guest.dword(kPlayers) == 1)
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
        guest.call(kReadRect, fe::args({ kFirstViewRect, rectOther, rectOther, rectWidth }, { rectOther }));
        controls.setDisabled(guest.dword(kPlayers) == 1 && std::int32_t(guest.dword(rectWidth)) < kWidthForControls);
    }

    if (Item restart = fe::findItem(guest, menu, kRestartButton))
    {
        const std::int32_t players = std::int32_t(guest.dword(kPlayers));
        restart.setOnClick(kOnRestartClicked);
        if (players >= 1 && (guest.word(kNetworkHost) == 0 || guest.dword(kNetworkJoin) == kNetworkJoined))
            retire(restart);
        if (std::int32_t(guest.dword(kNetPlayers)) >= 2)
            restore(restart);
    }

    guest.call(kHideSpare, fe::args({ menu }));
    return 0;
}

/* ---------------------------------------------------------------------------
 * The sliders of the graphics and the display
 * ------------------------------------------------------------------------- */

/* sub_44cf70, the effects' intensity moved. */
x86::reg32 intensityMoved(Guest& guest, x86::reg32 item)
{
    if (guest.call(kShownNow, fe::args({ item, kShownAlways })) != 0)
    {
        guest.call(kSliderValueOf, fe::args({ item, 0 }));
        guest.setFloat(kIntensity, guest.popFloat());
    }
    return guest.eax();
}

/* sub_44cfa0 and the five after it, the map's size moved: the slider from 0
 * to 1, the size from 1 to 4. */
x86::reg32 mapSizeMoved(Guest& guest, x86::reg32 item, x86::reg32 scale, x86::reg32 size)
{
    if (guest.call(kShownNow, fe::args({ item, kShownAlways })) != 0)
    {
        guest.call(kSliderValueOf, fe::args({ item, 0 }));
        fe::X87 x87(guest);
        const x86::Float value = guest.fpu().pop();
        guest.setFloat(size, float(x87.add(x87.mul(value, x87.load(scale)), x86::Float(1.0))));
    }
    return guest.eax();
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
constexpr x86::reg32 kFirstController = 0x6fc970;
constexpr std::uint8_t kControllerForce = 0x08;

/* sub_44d120, psoption and psoptn2 as they open. */
x86::reg32 onOptionsEnter(Guest& guest, x86::reg32 menu)
{
    dim(guest);
    if (Item audio = fe::findItem(guest, menu, kAudioButton))
    {
        if (guest.dword(kNoSound) != 0)
            audio.setDisabled(true);
    }
    return 0;
}

/* sub_44d170, Cancel's key on pscntl1: Enter on it is what cancels. */
x86::reg32 cancelKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key)
{
    if (key == 0xd)
        guest.setDword(kControlsDone, 0);
    return guest.call(kButtonKey, fe::args({ item, menu, x86::reg32(std::int32_t(std::int16_t(key))) }));
}

/* sub_44d190, pscntl1 as it opens: the controls kept for Cancel. */
x86::reg32 onControlsEnter(Guest& guest, x86::reg32 menu)
{
    dim(guest);
    guest.setDword(kControlsDone, 1);
    const x86::reg32 controls = guest.call(kControlsFor, fe::args({ guest.dword(kPlayers) }));
    guest.call(kMemmove, fe::args({ controls, kControlsKept, kControlsBothSize }));
    Item force = fe::findItem(guest, menu, kForceFeedback);
    if (!(guest.byte(kFirstController) & kControllerForce) && force)
        force.setDisabled(true);
    Item deadZone = fe::findItem(guest, menu, kDeadZone);
    if (guest.dword(kFirstController) == 0 && deadZone)
        deadZone.setDisabled(true);
    if (Item cancel = fe::findItem(guest, menu, kCancelButton))
        guest.setDword(cancel.address() + kItemOnKey, kCancelKey);
    return 0;
}

/* sub_44d220, pscntl1 as it closes: the old controls back on Cancel, the new
 * ones taken up otherwise. */
x86::reg32 onControlsExit(Guest& guest)
{
    const x86::reg32 controls = guest.call(kControlsFor, fe::args({ guest.dword(kPlayers) }));
    if (guest.dword(kControlsDone) == 0)
    {
        const std::uint32_t size = guest.dword(kPlayers) == 1 ? kControlsBothSize : kControlsOneSize;
        guest.call(kMemmove, fe::args({ kControlsKept, controls, size }));
    }
    else
    {
        guest.call(kForcePlayerCar);
    }
    guest.call(kBindPlayersControls);
    return 0;
}

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

/* sub_44d270, the screen's shape clicked: the views laid out again, for the
 * first player's kind of car. */
x86::reg32 onWidescreenClicked(Guest& guest)
{
    guest.call(kLayoutViews, fe::args({ playerIsCop(guest, 0) }));
    return fe::kDialogHandled;
}

/* sub_44d2a0, psgraph as it opens: the settings kept for Cancel, what the card
 * cannot do greyed out.  Port: the original greys the intensity out on a card
 * without alpha (a probe of 0x7a3a58); here every card has it. */
x86::reg32 onGraphicsEnter(Guest& guest, x86::reg32 menu)
{
    copySettings(guest, kSettingsKept, kSettings);
    dim(guest);
    if (!(guest.byte(kDisplayFlags) & kTrackLightingFlags))
    {
        disableItem(guest, menu, kPerspective);
        if (Item text = fe::findItem(guest, menu, kTrackLightingText))
            retire(text);
        disableItem(guest, menu, kTrackLighting);
    }
    if (Item wide = fe::findItem(guest, menu, kWidescreenButton))
    {
        if (guest.dword(kPlayers) == 1)
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
        fe::X87 x87(guest);
        guest.setFloat(intensity.address() + kSliderValue, float(x87.load(kIntensity)));
        guest.call(kSliderSetOnMove, fe::args({ intensity.address(), kIntensityMoved }));
    }
    if (Item zBuffer = fe::findItem(guest, menu, kZBuffer))
        zBuffer.setDisabled(guest.dword(kDisplayBuffering) == 0);
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0);
    return 0;
}

/* sub_44d3e0: the projected textures' button by what is on now. */
x86::reg32 fitProjected(Guest& guest, x86::reg32 menu)
{
    if (Item projected = fe::findItem(guest, menu, kProjectedButton))
        projected.setDisabled(guest.dword(kDisplayBuffering) == 0 || guest.dword(kProjectedText) == 0);
    return 0;
}

/* sub_44d420, psgraph as it closes: Cancel puts the settings back, and the
 * views laid out again if that changes the screen's shape. */
x86::reg32 onGraphicsExit(Guest& guest, x86::reg32 menu)
{
    if (!closedBy(guest, menu, kGraphicsCancel))
        return 0;
    const bool shapeChanged = guest.dword(kScreenSizeKept) != guest.dword(kScreenSize);
    copySettings(guest, kSettings, kSettingsKept);
    if (shapeChanged)
        guest.call(kLayoutViews, fe::args({ playerIsCop(guest, 0) }));
    return 0;
}

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
constexpr x86::reg32 kHudChoices = 0x6fbc64;
constexpr x86::reg32 kHudScreenStride = 0x348;
constexpr x86::reg32 kHudKindStride = 0x1a4;
constexpr x86::reg32 kHudPlacesStride = 8;
constexpr x86::reg32 kHudList1 = 0x0, kHudList1Places = 0x678828;
constexpr std::uint32_t kHudList1Last = 5;
constexpr x86::reg32 kHudList2 = 0x4, kHudList2Places = 0x678840;
constexpr std::uint32_t kHudList2Last = 4;
constexpr x86::reg32 kHudList3 = 0x8, kHudList3Places = 0x678810;
constexpr std::uint32_t kHudList3Last = 3;
constexpr x86::reg32 kHudList4 = 0xc, kHudList4Places = 0x6787f8;
constexpr std::uint32_t kHudList4Last = 2;

/* A screen's map slider: what it moves for each kind of car, what is told of
 * its moves, and the constants its value comes from. */
struct HudScreen
{
    x86::reg32 mapSize[2];     // float, of a racer and of a cop
    x86::reg32 mapMoved[2];
    x86::reg32 mapBias;        // float, -1
    x86::reg32 mapScale;       // float, a third
};
constexpr HudScreen kHudScreens[] = {
    { { 0x6fbc60, 0x6fbe04 }, { 0x44cfe0, 0x44cfa0 }, 0x539434, 0x539438 },
    { { 0x6fbfa8, 0x6fc14c }, { 0x44d060, 0x44d020 }, 0x539474, 0x539478 },
    { { 0x6fc2f0, 0x6fc494 }, { 0x44d0e0, 0x44d0a0 }, 0x53947c, 0x539480 },
};

x86::reg32 hudChoices(x86::reg32 screen, x86::reg32 kind)
{
    return kHudChoices + kHudScreenStride * screen + kHudKindStride * kind;
}

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
void openHud(Guest& guest, x86::reg32 menu, x86::reg32 screen, x86::reg32 kind, x86::reg32 next)
{
    const HudScreen& hud = kHudScreens[screen];
    dim(guest);
    if (Item map = fe::findItem(guest, menu, kMapScale))
    {
        fe::X87 x87(guest);
        const x86::Float biased = x87.add(x87.load(hud.mapSize[kind]), x87.load(hud.mapBias));
        guest.setFloat(map.address() + kSliderValue, float(x87.mul(biased, x87.load(hud.mapScale))));
        guest.call(kSliderSetOnMove, fe::args({ map.address(), hud.mapMoved[kind] }));
    }
    if (Item position = fe::findItem(guest, menu, kPosition))
        guest.setDword(position.address() + fe::item::kNextMenu, next);
    guest.call(kHideSpare, fe::args({ menu }));
    guest.call(kind != 0 ? kHideForCop : kHideForRacer, fe::args({ menu }));
}

/* The three's lists set from the choices: all but the first list, which the
 * screens have their own ways with; the fourth choice was read as they opened. */
void placeHudLists(Guest& guest, x86::reg32 screen, x86::reg32 kind, std::uint32_t fourth)
{
    const x86::reg32 choices = hudChoices(screen, kind);
    const x86::reg32 places = hudPlaces(screen, kind);
    guest.setDword(kHudList2Places + places, placeOf(guest.dword(choices + kHudList2), kHudList2Last));
    guest.setDword(kHudList3Places + places, placeOf(guest.dword(choices + kHudList3), kHudList3Last));
    guest.setDword(kHudList4Places + places, placeOf(fourth, kHudList4Last));
}

/* sub_44d4c0, pshud as it opens. */
x86::reg32 onHudEnter(Guest& guest, x86::reg32 menu)
{
    copySettings(guest, kSettingsKept, kSettings);
    guest.setDword(kHudShown, 1);
    const x86::reg32 kind = playerIsCop(guest, 0);
    const x86::reg32 choices = hudChoices(0, kind);
    const std::uint32_t fourth = guest.dword(choices + kHudList4);
    guest.setDword(kFirstIsRacer, kind == 0 ? 1 : 0);
    openHud(guest, menu, 0, kind, kind != 0 ? kNextAloneCop.address : kNextAloneRacer.address);
    placeHudLists(guest, 0, kind, fourth);
    guest.setDword(kHudList1Places + hudPlaces(0, kind), placeOf(guest.dword(choices + kHudList1), kHudList1Last));
    return 0;
}

/* Where POSITION leads from the two players' screens. */
x86::reg32 nextOfTwo(x86::reg32 firstIsCop, x86::reg32 secondIsCop)
{
    if (firstIsCop != 0)
        return secondIsCop != 0 ? kNextCopCop.address : kNextCopRacer.address;
    return secondIsCop != 0 ? kNextRacerCop.address : kNextRacerRacer.address;
}

/* sub_44d7a0, pshud1p as it opens: the first player's display. */
x86::reg32 onHud1Enter(Guest& guest, x86::reg32 menu)
{
    copySettings(guest, kSettingsKept, kSettings);
    guest.setDword(kHudShown, 1);
    const x86::reg32 first = playerIsCop(guest, 0);
    const x86::reg32 second = playerIsCop(guest, 1);
    const x86::reg32 choices = hudChoices(1, first);
    const std::uint32_t fourth = guest.dword(choices + kHudList4);
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(guest, menu, 1, first, nextOfTwo(first, second));
    placeHudLists(guest, 1, first, fourth);
    guest.setDword(kHudList1Places + hudPlaces(1, first), placeOf(guest.dword(choices + kHudList1), kHudList1Last));
    return 0;
}

/* sub_44dad0, pshud2p as it opens: the second player's.  Its first list's place
 * goes by its own choice's being set, but counts from pshud1p's: the original
 * has it so. */
x86::reg32 onHud2Enter(Guest& guest, x86::reg32 menu)
{
    copySettings(guest, kSettingsKept, kSettings);
    guest.setDword(kHudShown, 1);
    const x86::reg32 first = playerIsCop(guest, 0);
    const x86::reg32 second = playerIsCop(guest, 1);
    const x86::reg32 choices = hudChoices(2, second);
    const std::uint32_t fourth = guest.dword(choices + kHudList4);
    guest.setDword(kFirstIsRacer, first == 0 ? 1 : 0);
    guest.setDword(kSecondIsRacer, second == 0 ? 1 : 0);
    openHud(guest, menu, 2, second, nextOfTwo(first, second));
    placeHudLists(guest, 2, second, fourth);
    const std::uint32_t place = guest.dword(choices + kHudList1) != 0
        ? guest.dword(hudChoices(1, second) + kHudList1) - 1
        : kHudList1Last;
    guest.setDword(kHudList1Places + hudPlaces(2, second), place);
    return 0;
}

/* sub_44d670, sub_44d9a0, sub_44dcd0, a display's screen as it closes: its
 * lists' places back into choices for `player`'s kind of car, and Cancel puts
 * the settings back over them. */
x86::reg32 onHudExit(Guest& guest, x86::reg32 menu, x86::reg32 screen, x86::reg32 player)
{
    const x86::reg32 kind = playerIsCop(guest, player);
    const x86::reg32 choices = hudChoices(screen, kind);
    const x86::reg32 places = hudPlaces(screen, kind);
    guest.setDword(choices + kHudList2, choiceOf(guest.dword(kHudList2Places + places), kHudList2Last));
    guest.setDword(choices + kHudList3, choiceOf(guest.dword(kHudList3Places + places), kHudList3Last));
    guest.setDword(choices + kHudList4, choiceOf(guest.dword(kHudList4Places + places), kHudList4Last));
    guest.setDword(choices + kHudList1, choiceOf(guest.dword(kHudList1Places + places), kHudList1Last));
    guest.call(kHudPaneSet, fe::args({ player }));
    if (closedBy(guest, menu, kHudCancel))
        copySettings(guest, kSettings, kSettingsKept);
    return 0;
}

/* ---------------------------------------------------------------------------
 * pscamera
 * ------------------------------------------------------------------------- */

constexpr GuestString kCameraEscape{ 0x53a7ec, "escape" };
constexpr GuestString kCameraAssists{ 0x53a7f4, "assists" };
constexpr GuestString kCameraCancel{ 0x53a7fc, "CANCEL_CAMERA_OPTIONS" };

// The cameras the four keys hold, and those pscamera found.
constexpr x86::reg32 kCamera1 = 0x6fbba8, kCamera2 = 0x6fbbac, kCamera3 = 0x6fbbb0, kCamera4 = 0x6fbbb4;
constexpr x86::reg32 kCamera1Kept = 0x6788fc, kCamera2Kept = 0x6788f4, kCamera3Kept = 0x6788f8, kCamera4Kept = 0x6788f0;
constexpr x86::reg32 kCameraQuestion = 0x678930;   // char*[1]
constexpr x86::reg32 kCameraButtons = 0x678928;    // char*[2]
constexpr x86::reg32 kCameraMode = 0x5e1054;

constexpr x86::reg32 kCameraAnswered = 0x45d770;
constexpr x86::reg32 kCameraKey = 0x45d840;
constexpr x86::reg32 kSetCameraViews = 0x472300;
constexpr x86::reg32 kBuildCameraCycle = 0x423a60;
constexpr x86::reg32 kRememberMode = 0x423ae0;     // eax a player, edx a mode
constexpr x86::reg32 kSetCameraMode = 0x4238b0;    // likewise

constexpr std::uint32_t kTextSameCamera = 0x1da;
constexpr std::uint32_t kTextAssists = 0x196;
constexpr std::uint32_t kMenuBack = 3;

/* sub_45d770, the question about two keys on one camera answered: Ok takes each
 * camera off all keys but its first, and leaves. */
x86::reg32 cameraAnswered(Guest& guest, x86::reg32 answer, x86::reg32 close)
{
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
    const auto clearSame = [&](x86::reg32 later, x86::reg32 earlier) {
        if (guest.dword(later) == guest.dword(earlier))
            guest.setDword(later, 0);
    };
    clearSame(kCamera2, kCamera1);
    clearSame(kCamera3, kCamera1);
    clearSame(kCamera4, kCamera1);
    guest.leaveEdx(guest.dword(kCamera3));
    clearSame(kCamera3, kCamera2);
    clearSame(kCamera4, kCamera2);
    clearSame(kCamera4, kCamera3);
    return kMenuBack;
}

/* sub_45d840, pscamera's way out: it asks if two keys hold one camera. */
x86::reg32 cameraKey(Guest& guest)
{
    const auto same = [&](x86::reg32 a, x86::reg32 b) {
        return guest.dword(a) != 0 && guest.dword(a) == guest.dword(b);
    };
    if (!same(kCamera1, kCamera2) && !same(kCamera1, kCamera3) && !same(kCamera1, kCamera4)
        && !same(kCamera2, kCamera3) && !same(kCamera2, kCamera4) && !same(kCamera3, kCamera4))
        return kMenuBack;
    guest.setDword(kCameraQuestion, fe::text(guest, kTextSameCamera));
    guest.setDword(kCameraButtons, fe::text(guest, kTextOk));
    guest.setDword(kCameraButtons + 4, fe::text(guest, kTextCancel));
    fe::openQuestion(guest, { 1, kCameraQuestion, 2, kCameraButtons, 0, 1, 0, kCameraAnswered });
    return fe::kDialogHandled;
}

/* sub_45d920, pscamera as it opens: the cameras kept for Cancel. */
x86::reg32 onCameraEnter(Guest& guest, x86::reg32 menu)
{
    dim(guest);
    guest.setDword(kCamera1Kept, guest.dword(kCamera1));
    guest.setDword(kCamera2Kept, guest.dword(kCamera2));
    guest.setDword(kCamera3Kept, guest.dword(kCamera3));
    guest.setDword(kCamera4Kept, guest.dword(kCamera4));
    if (Item escape = fe::findItem(guest, menu, kCameraEscape))
        escape.setOnClick(kCameraKey);
    if (Item assists = fe::findItem(guest, menu, kCameraAssists))
        assists.setText(fe::text(guest, kTextAssists));
    return 0;
}

/* sub_45d9a0: at least one camera to look from. */
x86::reg32 ensureSomeCamera(Guest& guest)
{
    if (guest.dword(kCamera1) == 0 && guest.dword(kCamera2) == 0 && guest.dword(kCamera3) == 0
        && guest.dword(kCamera4) == 0)
        guest.setDword(kCamera1, 1);
    return 0;
}

/* sub_45d9e0, pscamera as it closes: Cancel puts them back; otherwise the race
 * takes them up at once. */
x86::reg32 onCameraExit(Guest& guest, x86::reg32 menu)
{
    if (closedBy(guest, menu, kCameraCancel))
    {
        guest.setDword(kCamera1, guest.dword(kCamera1Kept));
        guest.setDword(kCamera2, guest.dword(kCamera2Kept));
        guest.setDword(kCamera3, guest.dword(kCamera3Kept));
        guest.setDword(kCamera4, guest.dword(kCamera4Kept));
        return 0;
    }
    guest.call(kSetCameraViews);
    guest.call(kBuildCameraCycle);
    guest.call(kRememberMode, fe::args({ 0, guest.dword(kCameraMode) }));
    guest.call(kRememberMode, fe::args({ 1, guest.dword(kCameraMode) }));
    if (guest.dword(kInRace) != 0)
    {
        guest.call(kSetCameraMode, fe::args({ 0, guest.dword(kCameraMode) }));
        if (guest.dword(kPlayers) == 1)
            guest.call(kSetCameraMode, fe::args({ 1, guest.dword(kCameraMode) }));
    }
    return 0;
}

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
constexpr x86::reg32 kQuitToAsk = 0x4b6810;
constexpr x86::reg32 kQuitReplayClicked = 0x4b6870;
constexpr x86::reg32 kQuitPostClicked = 0x4b6890;
constexpr x86::reg32 kQuitMainClicked = 0x4b68d0;
constexpr x86::reg32 kQuitSystemClicked = 0x4b6910;

/* sub_4b6810: "are you sure?" for a button of quitto. */
x86::reg32 quitToAsk(Guest& guest, std::uint32_t question, std::uint32_t leaveTo)
{
    guest.setDword(kQuitToLeaveTo, leaveTo);
    guest.setDword(kQuitToQuestion, fe::text(guest, question));
    guest.setDword(kQuitToButtons, fe::text(guest, kTextYes));
    guest.setDword(kQuitToButtons + 4, fe::text(guest, kTextNo));
    fe::openQuestion(guest, { 1, kQuitToQuestion, 2, kQuitToButtons, 0, 0, 0, kQuitToAnswered });
    return guest.eax();
}

/* sub_4b6930, quitto as it opens: a network race has the one way back, any
 * other the two. */
x86::reg32 onQuitToEnter(Guest& guest, x86::reg32 menu)
{
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
    if (guest.dword(kNetPlayers) != 0)
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

/* ---------------------------------------------------------------------------
 * The race's views: their rectangles and the corners of what is drawn over them
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kFullRect = 0x7cda60;         // four ints
constexpr x86::reg32 kOnePlayerRect = 0x7cda90;
constexpr x86::reg32 kTwoPlayersRect = 0x7cdb98;

/* sub_4be3d0: the full rectangle and the view's -- for one player the first's
 * across, for two its own -- into the corners of the quads drawn over the
 * screen, as floats. */
x86::reg32 fillViewQuads(Guest& guest)
{
    const std::int32_t left = std::int32_t(guest.dword(kFullRect));
    const std::int32_t top = std::int32_t(guest.dword(kFullRect + 4));
    const std::int32_t right = std::int32_t(guest.dword(kFullRect + 8));
    const std::int32_t bottom = std::int32_t(guest.dword(kFullRect + 0xc));
    std::int32_t viewLeft, viewRight, viewTop, viewBottom;
    if (guest.dword(kPlayers) == 1)
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
            guest.setFloat(at, float(fe::X87::integer(value)));
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

/* sub_4beb00, sub_4beb40: a rectangle's first or second half set. */
void setRectHalf(Guest& guest, x86::reg32 half, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d)
{
    guest.setDword(half, a);
    guest.setDword(half + 4, b);
    guest.setDword(half + 8, c);
    guest.setDword(half + 0xc, d);
    guest.leaveEdx(d);
}

/* sub_4beb20: a rectangle read, each of its four through a pointer. */
x86::reg32 readRect(Guest& guest, x86::reg32 rect, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d)
{
    guest.setDword(a, guest.dword(rect));
    guest.setDword(b, guest.dword(rect + 4));
    guest.setDword(c, guest.dword(rect + 8));
    const x86::reg32 last = guest.dword(rect + 0xc);
    guest.setDword(d, last);
    guest.leaveEdx(d);
    return last;
}

/* ---------------------------------------------------------------------------
 * Running them
 * ------------------------------------------------------------------------- */

constexpr std::uint8_t kNothing = 0;
constexpr std::uint8_t kEbxEcx = fe::kRestoreEbx | fe::kRestoreEcx;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEsiEdi = kEbxEcxEsi | fe::kRestoreEdi;

/* A function of eax: a screen, an item, a name. */
template <typename Body>
void wholeForOne(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 eax = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, eax); });
}

/* A function of eax and edx: a dialog's callback, say. */
template <typename Body>
void wholeForTwo(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 eax = cpu.eax;
    const x86::reg32 edx = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, eax, edx); });
}

/* A screen's empty handler. */
void wholeNothing(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site)
{
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
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

/* The saves. */
void frontend::pauseSeriesSaveExists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446510", 0, 4 + 0x100, kSeriesExistsStrings, std::size(kSeriesExistsStrings), kNothing };
    wholeForOne(app, cpu, site, seriesSaveExists);
}

void frontend::pauseWriteSeriesSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446550", 0, 24 + 0x100, kSeriesWriteStrings, std::size(kSeriesWriteStrings) };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 name) { return writeSave(guest, name, kSeriesPath); });
}

void frontend::pauseGhostExists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446740", 0, 4 + 0x100, kGhostExistsStrings, std::size(kGhostExistsStrings), kNothing };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 name) { return keptExists(guest, name, kGhostExtension); });
}

void frontend::pauseWriteTournamentSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4468c0", 0, 24 + 0x100, kTournamentWriteStrings, std::size(kTournamentWriteStrings) };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 name) { return writeSave(guest, name, kTournamentPath); });
}

void frontend::pauseTournamentSaveExists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446950", 0, 4 + 0x100, kTournamentExistsStrings, std::size(kTournamentExistsStrings),
                          kNothing };
    wholeForOne(app, cpu, site,
                [](Guest& guest, x86::reg32 name) { return keptExists(guest, name, kTournamentExtension); });
}

void frontend::pauseReplayExists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446a90", 0, 4 + 0x100, kReplayExistsStrings, std::size(kReplayExistsStrings), kNothing };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 name) { return keptExists(guest, name, kReplayExtension); });
}

void frontend::pauseCheckSaveable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_478010", 0, 20, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    wholeForOne(app, cpu, site, checkSaveable);
}

void frontend::pauseOverwriteAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447800", 0, 16, nullptr, 0, kEbxEcxEsi };
    wholeForTwo(app, cpu, site, overwriteAnswered);
}

void frontend::pauseSaveNameAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4478c0", 0, 20 + 4, nullptr, 0, kEbxEcxEsiEdi };
    wholeForTwo(app, cpu, site, saveNameAnswered);
}

void frontend::pauseAskSaveName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447a30", 0, 24, nullptr, 0 };
    wholeForOne(app, cpu, site, askSaveName);
}

/* pause. */
void frontend::pauseHideSpare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c2d0", 0, 12, kHideMe, std::size(kHideMe), kEcxEdx };
    wholeForOne(app, cpu, site, hideSpare);
}

void frontend::pauseHideForCop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c360", 0, 12, kRacerInstruments, std::size(kRacerInstruments), kEcxEdx };
    wholeForOne(app, cpu, site, hideForCop);
}

void frontend::pauseHideForRacer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c4e0", 0, 12, kCopInstrumentsFirst, std::size(kCopInstrumentsFirst), kEcxEdx };
    wholeForOne(app, cpu, site, hideForRacer);
}

void frontend::pauseLeaveAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c610", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForTwo(app, cpu, site, [](Guest& guest, x86::reg32 answer, x86::reg32 close) {
        return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kLeaveTo); });
    });
}

void frontend::pauseRestartAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c670", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForTwo(app, cpu, site, [](Guest& guest, x86::reg32 answer, x86::reg32 close) {
        return yesOrNoAnswered(guest, answer, close, [] { return kAnswerRestart; });
    });
}

void frontend::pauseAskLeave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c6d0", 0, 12, nullptr, 0, kEbxEcx };
    wholeForTwo(app, cpu, site, askLeave);
}

void frontend::pauseQuitChoiceAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c730", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForTwo(app, cpu, site, quitChoiceAnswered);
}

void frontend::pauseQuitClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44c7b0", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, onQuitClicked);
}

void frontend::pauseRestartClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ca10", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, onRestartClicked);
}

void frontend::pauseSaveClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ca80", 0, 4, nullptr, 0, kNothing };
    fe::runWhole(app, cpu, site, onSaveClicked);
}

void frontend::pauseOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44cab0", 0, 24 + 0xc, kPauseStrings, std::size(kPauseStrings) };
    wholeForOne(app, cpu, site, onPauseEnter);
}

void frontend::pauseOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44cf60", 0, 4, nullptr, 0, kNothing };
    wholeNothing(app, cpu, site);
}

/* The sliders. */
void frontend::pauseIntensityMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44cf70", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, intensityMoved);
}

void frontend::pauseMapCopMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44cfa0", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x53932c, 0x6fbe04); });
}

void frontend::pauseMapRacerMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44cfe0", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x539330, 0x6fbc60); });
}

void frontend::pauseMapFirstCopMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d020", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x539334, 0x6fc14c); });
}

void frontend::pauseMapFirstRacerMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d060", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x539338, 0x6fbfa8); });
}

void frontend::pauseMapSecondCopMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d0a0", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x53933c, 0x6fc494); });
}

void frontend::pauseMapSecondRacerMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d0e0", 0, 12, nullptr, 0, kEcxEdx };
    wholeForOne(app, cpu, site, [](Guest& g, x86::reg32 item) { return mapSizeMoved(g, item, 0x539340, 0x6fc2f0); });
}

/* psoption, psoptn2, pscntl1. */
void frontend::pauseOptionsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d120", 0, 12, kOptionsStrings, std::size(kOptionsStrings), kEcxEdx };
    wholeForOne(app, cpu, site, onOptionsEnter);
}

void frontend::pauseOptionsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d160", 0, 4, nullptr, 0, kNothing };
    wholeNothing(app, cpu, site);
}

void frontend::pauseControlsCancelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d170", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 item = cpu.eax, menu = cpu.edx;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cancelKey(guest, item, menu, key); });
}

void frontend::pauseControlsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d190", 0, 16, kControlsStrings, std::size(kControlsStrings), kEbxEcxEdx };
    wholeForOne(app, cpu, site, onControlsEnter);
}

void frontend::pauseControlsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d220", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, onControlsExit);
}

/* psgraph. */
void frontend::pauseWidescreenClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d270", 0, 4, nullptr, 0, kNothing };
    fe::runWhole(app, cpu, site, onWidescreenClicked);
}

void frontend::pauseGraphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d2a0", 0, 24, kGraphicsStrings, std::size(kGraphicsStrings) };
    wholeForOne(app, cpu, site, onGraphicsEnter);
}

void frontend::pauseGraphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d3e0", 0, 8, kProjectedStrings, std::size(kProjectedStrings), fe::kRestoreEdx };
    wholeForOne(app, cpu, site, fitProjected);
}

void frontend::pauseGraphicsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d420", 0, 24, kGraphicsExitStrings, std::size(kGraphicsExitStrings) };
    wholeForOne(app, cpu, site, onGraphicsExit);
}

/* pshud, pshud1p, pshud2p. */
void frontend::pauseHudOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d4c0", 0, 24, kHudStrings, std::size(kHudStrings) };
    wholeForOne(app, cpu, site, onHudEnter);
}

void frontend::pauseHudOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d670", 0, 24, kHudExitStrings, std::size(kHudExitStrings) };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 menu) { return onHudExit(guest, menu, 0, 0); });
}

void frontend::pauseHud1OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d7a0", 0, 24, kHud2Strings, std::size(kHud2Strings) };
    wholeForOne(app, cpu, site, onHud1Enter);
}

void frontend::pauseHud1OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44d9a0", 0, 24, kHudExitStrings, std::size(kHudExitStrings) };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 menu) { return onHudExit(guest, menu, 1, 0); });
}

void frontend::pauseHud2OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44dad0", 0, 24, kHud2Strings, std::size(kHud2Strings) };
    wholeForOne(app, cpu, site, onHud2Enter);
}

void frontend::pauseHud2OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44dcd0", 0, 24, kHudExitStrings, std::size(kHudExitStrings) };
    wholeForOne(app, cpu, site, [](Guest& guest, x86::reg32 menu) { return onHudExit(guest, menu, 2, 1); });
}

/* aituner. */
void frontend::pauseAiTunerOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e130", 0, 4, nullptr, 0, kNothing };
    wholeNothing(app, cpu, site);
}

void frontend::pauseAiTunerOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e140", 0, 4, nullptr, 0, kNothing };
    wholeNothing(app, cpu, site);
}

/* pscamera. */
void frontend::pauseCameraAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d770", 0, 20, nullptr, 0, kEbxEcxEsiEdi };
    wholeForTwo(app, cpu, site, cameraAnswered);
}

void frontend::pauseCameraKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d840", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, cameraKey);
}

void frontend::pauseCameraOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d920", 0, 12, kCameraStrings, std::size(kCameraStrings), kEcxEdx };
    wholeForOne(app, cpu, site, onCameraEnter);
}

void frontend::pauseCameraEnsureOne(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d9a0", 0, 4, nullptr, 0, kNothing };
    fe::runWhole(app, cpu, site, ensureSomeCamera);
}

void frontend::pauseCameraOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d9e0", 0, 12, kCameraExitStrings, std::size(kCameraExitStrings),
                          fe::kRestoreEbx | fe::kRestoreEdx };
    wholeForOne(app, cpu, site, onCameraExit);
}

/* quitto. */
void frontend::pauseQuitToAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b67b0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForTwo(app, cpu, site, [](Guest& guest, x86::reg32 answer, x86::reg32 close) {
        return yesOrNoAnswered(guest, answer, close, [&] { return guest.dword(kQuitToLeaveTo); });
    });
}

void frontend::pauseQuitToAsk(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6810", 0, 12, nullptr, 0, kEbxEcx };
    wholeForTwo(app, cpu, site, quitToAsk);
}

void frontend::pauseQuitToReplay(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6870", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kQuitToAsk, fe::args({ kAskQuitNetwork, kLeaveToPostGame }));
        return x86::reg32(fe::kDialogHandled);
    });
}

void frontend::pauseQuitToPost(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6890", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kQuitToAsk, fe::args({ askQuitRace(guest), kLeaveToPostGame }));
        return x86::reg32(fe::kDialogHandled);
    });
}

void frontend::pauseQuitToMain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b68d0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kQuitToAsk, fe::args({ askQuitRace(guest), kLeaveToMain }));
        return x86::reg32(fe::kDialogHandled);
    });
}

void frontend::pauseQuitToSystem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6910", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kQuitToAsk, fe::args({ kAskQuitToSystem, kLeaveToSystem }));
        return x86::reg32(fe::kDialogHandled);
    });
}

void frontend::pauseQuitToOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6930", 0, 16, kQuitToStrings, std::size(kQuitToStrings), kEbxEcxEdx };
    wholeForOne(app, cpu, site, onQuitToEnter);
}

/* The race's views. */
void frontend::pauseFillViewQuads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4be3d0", 0, 24 + 0x20, nullptr, 0 };
    fe::runWhole(app, cpu, site, fillViewQuads);
}

void frontend::pauseSetRectFirst(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4beb00", 4, 4, nullptr, 0, kNothing };
    const x86::reg32 rect = cpu.eax, a = cpu.edx, b = cpu.ebx, c = cpu.ecx;
    const x86::reg32 d = Guest(app, cpu).dword(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        setRectHalf(guest, rect, a, b, c, d);
        return rect;
    });
}

void frontend::pauseReadRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4beb20", 4, 8, nullptr, 0, fe::kRestoreEsi };
    const x86::reg32 rect = cpu.eax, a = cpu.edx, b = cpu.ebx, c = cpu.ecx;
    const x86::reg32 d = Guest(app, cpu).dword(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return readRect(guest, rect, a, b, c, d); });
}

void frontend::pauseSetRectSecond(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4beb40", 4, 4, nullptr, 0, kNothing };
    const x86::reg32 rect = cpu.eax, a = cpu.edx, b = cpu.ebx, c = cpu.ecx;
    const x86::reg32 d = Guest(app, cpu).dword(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        setRectHalf(guest, rect + 0x10, a, b, c, d);
        return rect;
    });
}

}
