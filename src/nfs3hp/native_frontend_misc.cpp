#include "native_frontend.h"
#include <nfs3hp.h>
#include <array>
#include <iterator>

/* The screens that are neither the race's pause, nor the cars', nor the network's,
 * decompiled:
 *
 *   racetype   the kind of race, of six;
 *   level      LEVEL.MNU: the cops' assists, a duel's own items;
 *   camera     CAMERA.MNU: the views the camera cycles through;
 *   setup      SETUP.MNU: the way in, one player or two, the web site's dialogs;
 *   loadsave   LOADSAVE.MNU: the list of kept replays, tournaments, knockouts
 *              and ghosts, with a description of the one selected;
 *   records    RECORDS.MNU: a track's records, and deleting them;
 *   postgame   POSTGAME.MNU: the sheets after a race, what can be saved of it,
 *              the awards it won, and the way back;
 *   credits    the credits' pages, sliding across; credits1 the showcase's.
 *
 * With them the race's results, which only these screens sort and look up: a
 * table of 16 participants for each of two groups (group 0 the race, group 1
 * the fastest laps) and the order they are sorted in at 0x5595d8.
 *
 * Checking a kept file's cars and loading a ghost's stay with loading (sub_478090,
 * sub_478100, sub_477b50), the network's way out with the network's code
 * (sub_4a6a10), and the dial-up check with Windows' libraries (sub_49f790,
 * sub_49f880): called by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

constexpr x86::reg32 kLanguage = 0x6fbb4c;
constexpr x86::reg32 kPlayers = 0x6fd3b0;          // 0, 1 side by side, 2 a network
constexpr x86::reg32 kRaceType = 0x6fd3b8;         // 0 a single race, 1 a tournament, 2 a knockout, 3 a duel
constexpr x86::reg32 kTrack = 0x6fd4ac;
constexpr x86::reg32 kInRace = 0x559234;
constexpr x86::reg32 kNetworkGame = 0x6fd50c;
constexpr x86::reg32 kNetworkConnected = 0x7a22e0;
constexpr x86::reg32 kNetworkHost = 0x7a22da;      // int16
constexpr x86::reg32 kNetworkLeaving = 0x7a22dc;   // int16: the game is over for this machine
constexpr x86::reg32 kNetworkLost = 0x7a22de;      // int16: to be told as the next screen opens
constexpr x86::reg32 kDeveloperFlags = 0x55eb2e;   // byte
constexpr std::uint8_t kPracticeOnly = 0x20;
constexpr std::uint8_t kNoFinishers = 0x40;
constexpr x86::reg32 kRacesClosed = 0x6fbb54;      // nonzero: races and loading closed

constexpr x86::reg32 kOnDraw = 0x2c;               // an item's drawing, set by code
constexpr x86::reg32 kOnDrawBox = 0x54;            // a box's drawing
constexpr x86::reg32 kListOnMove = 0x74;           // a list's: its selection moved
constexpr x86::reg32 kListCount = 0x48;
constexpr x86::reg32 kListSelected = 0x4c;
constexpr x86::reg32 kListEntries = 0x70;          // char*, 8 bytes each

// The game's.
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kCopy = 0x4ea4f0;             // eax to edx, ebx bytes
constexpr x86::reg32 kFillZero = 0x4e070c;         // eax, edx bytes
constexpr x86::reg32 kMemset = 0x4e0640;           // eax, the byte edx, ebx bytes
constexpr x86::reg32 kFree = 0x4e1890;
constexpr x86::reg32 kCurrentMenu = 0x44aeb0;
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;
constexpr x86::reg32 kOpenMessage = 0x444a80;      // eax lines, edx them, ebx its time, ecx 1; 0 on the stack
constexpr x86::reg32 kCloseNetwork = 0x4a65e0;
constexpr x86::reg32 kStrcmp = 0x4ee310;
constexpr x86::reg32 kStricmp = 0x4e9ec0;
constexpr x86::reg32 kFopen = 0x4edfe8;
constexpr x86::reg32 kFclose = 0x4ee100;
constexpr x86::reg32 kLoadFile = 0x4e0ec0;
constexpr x86::reg32 kFileExists = 0x4e0e60;
constexpr x86::reg32 kLoadFileQuietly = 0x4e0fe0;

constexpr std::uint32_t kShortMessage = 0x200;     // a message's time, in ticks
constexpr std::uint32_t kLongMessage = 0x400;

x86::reg32 openMessage(Guest& guest, x86::reg32 lineCount, x86::reg32 lines, x86::reg32 time)
{
    return guest.call(kOpenMessage, fe::args({ lineCount, lines, time, 1 }, { 0 }));
}

/* Greyed out and hidden, the state and flags bytes as one word. */
void greyAndHide(Guest& guest, x86::reg32 item)
{
    guest.setWord(item + fe::item::kState, std::uint16_t(guest.word(item + fe::item::kState) | 0x1001));
}

void showAgain(Guest& guest, x86::reg32 item)
{
    guest.setWord(item + fe::item::kState, std::uint16_t(guest.word(item + fe::item::kState) & 0xeffe));
}

std::int32_t signedWord(const Guest& guest, x86::reg32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t signedDword(const Guest& guest, x86::reg32 address)
{
    return std::int32_t(guest.dword(address));
}

/* ---------------------------------------------------------------------------
 * level
 * ------------------------------------------------------------------------- */

constexpr GuestString kCopAssists[] = { { 0x53a75c, "copassist1" }, { 0x53a768, "copassist2" } };
constexpr GuestString kDuelDisabled[] = {
    { 0x53a774, "disable1" }, { 0x53a780, "disable2" }, { 0x53a78c, "disable3" },  { 0x53a798, "disable4" },
    { 0x53a7a4, "disable5" }, { 0x53a7b0, "disable6" }, { 0x53a7bc, "disable7" },  { 0x53a7c8, "disable8" },
    { 0x53a7d4, "disable9" }, { 0x53a7e0, "disable10" },
};

/* sub_45d630, level as it opens: in a duel of one player the cops' assists are
 * open and the ten others closed; else the assists are closed. */
x86::reg32 levelOnEnterBody(Guest& guest, x86::reg32 menu)
{
    const bool soloDuel = guest.dword(kRaceType) == 3 && guest.dword(kPlayers) == 0;
    for (const GuestString& codelink : kCopAssists)
        if (Item item = fe::findItem(guest, menu, codelink))
            item.setDisabled(!soloDuel);
    if (soloDuel)
        for (const GuestString& codelink : kDuelDisabled)
            if (Item item = fe::findItem(guest, menu, codelink))
                item.setDisabled(true);
    return 0;
}

/* ---------------------------------------------------------------------------
 * camera
 * ------------------------------------------------------------------------- */

// The views the cycle holds, on or off, and the copy Cancel puts back.
constexpr x86::reg32 kCameraViews = 0x6fbba8;
constexpr x86::reg32 kCameraViewsKept[] = { 0x6788fc, 0x6788f4, 0x6788f8, 0x6788f0 };
constexpr x86::reg32 kCameraMode = 0x5e1054;
constexpr x86::reg32 kDimBackground = 0x451530;    // a float on the stack: a race's pause screen behind
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
constexpr x86::reg32 kCameraEscape = 0x45d840;
constexpr x86::reg32 kSetCameraViews = 0x472300;
constexpr x86::reg32 kBuildCameraCycle = 0x423a60;
constexpr x86::reg32 kRememberMode = 0x423ae0;     // eax the player, edx the mode
constexpr x86::reg32 kSetCameraMode = 0x4238b0;    // likewise
constexpr GuestString kCameraEscapeItem{ 0x53a7ec, "escape" };
constexpr GuestString kCameraAssists{ 0x53a7f4, "assists" };
constexpr GuestString kCameraCancel{ 0x53a7fc, "CANCEL_CAMERA_OPTIONS" };

/* sub_45d920, camera as it opens: the views kept for Cancel. */
x86::reg32 cameraOnEnterBody(Guest& guest, x86::reg32 menu)
{
    guest.call(kDimBackground, fe::args({}, { kFloat0_6 }));
    for (x86::reg32 i = 0; i < std::size(kCameraViewsKept); ++i)
        guest.setDword(kCameraViewsKept[i], guest.dword(kCameraViews + 4 * i));
    if (Item escape = fe::findItem(guest, menu, kCameraEscapeItem))
        escape.setOnClick(kCameraEscape);
    if (Item assists = fe::findItem(guest, menu, kCameraAssists))
        assists.setText(fe::text(guest, 0x196));
    return 0;
}

/* sub_45d9e0, camera as it closes: by its Cancel the views as they were, else
 * the new ones taken up, by the race too if the menus are its pause screens. */
x86::reg32 cameraOnExitBody(Guest& guest, x86::reg32 menu)
{
    const std::int32_t first = signedWord(guest, menu + fe::menu::kFirstItem);
    const std::int32_t selected = signedWord(guest, menu + fe::menu::kSelected);
    const x86::reg32 item = fe::item::kTable + fe::item::kSize * x86::reg32(first + selected);
    if (guest.dword(item) == 5
        && guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kCameraCancel.address })) != 0)
    {
        for (x86::reg32 i = 0; i < std::size(kCameraViewsKept); ++i)
            guest.setDword(kCameraViews + 4 * i, guest.dword(kCameraViewsKept[i]));
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

/* sub_45d9a0, camera on every pass: one view at least. */
x86::reg32 cameraOnFrameBody(Guest& guest)
{
    bool any = false;
    for (x86::reg32 i = 0; i < 4 && !any; ++i)
        any = guest.dword(kCameraViews + 4 * i) != 0;
    if (!any)
        guest.setDword(kCameraViews, 1);
    return 0;
}

/* ---------------------------------------------------------------------------
 * setup
 * ------------------------------------------------------------------------- */

constexpr GuestString kSetupOnePlayer{ 0x539614, "SETUP_ONEPLAYERBUTTON" };
constexpr GuestString kSetupConnect{ 0x53962c, "SETUP_CONNECTPLAYERS" };
constexpr GuestString kSetupTwoPlayers{ 0x539644, "SETUP_TWOPLAYERBUTTON" };
constexpr GuestString kSetupWebSite{ 0x53965c, "SETUP_NfsWebSite" };
constexpr GuestString kSetupLoad{ 0x539670, "SETUP_LOADBUTTON" };
constexpr GuestString kSetupName{ 0x53960c, "Setup" };
constexpr x86::reg32 kResetAttractTimer = 0x4961e0;
constexpr x86::reg32 kAttractMode = 0x496200;
constexpr x86::reg32 kOpenRaceDialog = 0x452bd0;
constexpr x86::reg32 kSetupIsCurrent = 0x452db0;
constexpr x86::reg32 kOnePlayerClicked = 0x452f20;
constexpr x86::reg32 kTwoPlayersClicked = 0x452d40;
constexpr x86::reg32 kWebSiteClicked = 0x452e60;
constexpr x86::reg32 kConnectionLost = 0x664674;    // char*: its line
constexpr x86::reg32 kDialUpRunning = 0x49f790;     // a dial-up connection is up
constexpr x86::reg32 kDialUpEntries = 0x49f880;     // the connections it knows
constexpr x86::reg32 kWebSiteAnswer = 0x55e894;
constexpr x86::reg32 kRaceDialogCanCancel = 0x559540;

/* sub_452f80, setup as it opens: the network shut if the race left one; a
 * developer's start, and the buttons' clicks. */
x86::reg32 setupOnEnterBody(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kPlayers, 0);
    if (guest.dword(kNetworkConnected) != 0)
        guest.call(kCloseNetwork);
    if (guest.word(kNetworkLost) != 0)
    {
        guest.setDword(kConnectionLost, fe::text(guest, 0x114));
        openMessage(guest, 1, kConnectionLost, kShortMessage);
        guest.setWord(kNetworkLost, 0);
    }
    if (Item onePlayer = fe::findItem(guest, menu, kSetupOnePlayer))
    {
        const std::uint8_t flags = guest.byte(kDeveloperFlags);
        if (flags & kPracticeOnly)
        {
            guest.setDword(kRaceType, 3);
        }
        else if (flags & 0x10)
        {
            guest.setDword(kRaceType, 3);
            if (guest.byte(kDeveloperFlags + 1) & 4)
                if (Item connect = fe::findItem(guest, menu, kSetupConnect))
                    connect.setDisabled(true);
        }
        else
        {
            onePlayer.setOnClick(kOnePlayerClicked);
        }
    }
    if (Item twoPlayers = fe::findItem(guest, menu, kSetupTwoPlayers))
        twoPlayers.setOnClick(kTwoPlayersClicked);
    if (Item webSite = fe::findItem(guest, menu, kSetupWebSite))
        webSite.setOnClick(kWebSiteClicked);
    if (guest.dword(kRacesClosed) != 0)
    {
        for (const GuestString& codelink : { kSetupOnePlayer, kSetupTwoPlayers, kSetupLoad })
            if (Item item = fe::findItem(guest, menu, codelink))
                item.setDisabled(true);
    }
    guest.call(kResetAttractTimer);
    return 0;
}

/* sub_452f60, setup on every pass: the attract-mode movie, when it is time. */
x86::reg32 setupOnFrameBody(Guest& guest)
{
    const x86::reg32 played = guest.call(kAttractMode);
    return played != 0 ? fe::kMenuMoviePlayed : played;
}

/* sub_452ac0: the race dialog answered.  The kinds of race its buttons stand
 * for lie one lower in a network game, which has no duel; returns the menu
 * loop's answer, 5 on to the next screen. */
x86::reg32 setupRaceAnsweredBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    constexpr x86::reg32 kOnward = 5, kStay = 2;
    x86::reg32 next = 0;
    bool close = false;
    if (answer == fe::kDialogEnter)
    {
        const bool network = guest.dword(kNetworkGame) != 0;
        close = true;
        next = kOnward;
        switch (fe::dialogFocus(guest))
        {
        case 0:
            guest.setDword(kRaceType, 0);
            break;
        case 1:
            guest.setDword(kRaceType, network ? 2 : 3);
            break;
        case 2:
            guest.setDword(kRaceType, network ? 1 : 2);
            break;
        case 3:
            if (network)
                next = kStay;
            else
                guest.setDword(kRaceType, 1);
            break;
        case 4:
            next = kStay;
            break;
        default:
            close = false;
            next = 0;
            break;
        }
    }
    else if (answer == fe::kDialogEscape && guest.dword(kRaceDialogCanCancel) != 0)
    {
        close = true;
        next = kStay;
    }
    if (closes != 0)
        guest.setByte(closes, close ? 1 : 0);
    if (signedDword(guest, kPlayers) > 1)
        next = kStay;
    guest.leaveEdx(next);
    return next;
}

/* sub_452bd0: the race dialog: its buttons one fewer on the network, each with
 * its line of help; the last, Cancel, only if `canCancel`. */
x86::reg32 setupAskRaceBody(Guest& guest, x86::reg32 canCancel)
{
    constexpr x86::reg32 kLine = 0x559544, kButtons = 0x559548, kHelp = 0x55955c;
    const x86::reg32 network = guest.dword(kNetworkGame) != 0 ? 1 : 0;
    guest.setDword(kLine, 0);
    guest.setDword(kRaceDialogCanCancel, canCancel);
    guest.call(kMemset, fe::args({ kButtons, 0, 0x14 }));
    guest.call(kMemset, fe::args({ kHelp, 0, 0x14 }));
    guest.setDword(kLine, fe::text(guest, 0x10f));
    guest.setDword(kButtons, fe::text(guest, 0xd5));
    guest.setDword(kButtons + 4, fe::text(guest, 0xdc));
    const x86::reg32 third = 2 - network, fourth = 3 - network, fifth = 4 - network;
    guest.setDword(kButtons + 4 * third, fe::text(guest, 0xd7));
    guest.setDword(kButtons + 4 * fourth, fe::text(guest, 0xd6));
    guest.setDword(kButtons + 4 * fifth, fe::text(guest, 0xc6));
    guest.setDword(kHelp, fe::text(guest, 0x181));
    guest.setDword(kHelp + 4, fe::text(guest, 0x184));
    guest.setDword(kHelp + 4 * third, fe::text(guest, 0x183));
    guest.setDword(kHelp + 4 * fourth, fe::text(guest, 0x182));
    guest.setDword(kHelp + 4 * fifth, 0);
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = (canCancel != 0 ? 1 : 0) + 4 - network;
    question.buttons = kButtons;
    question.buttonHelp = kHelp;
    question.focus = canCancel != 0 ? guest.dword(kRaceType) : 0;
    question.column = 1;
    question.callback = 0x452ac0;
    fe::openQuestion(guest, question);
    return guest.eax();
}

/* sub_452d40 and sub_452f20: the buttons, two players side by side or one, each
 * off the network, on to the race dialog. */
x86::reg32 setupPlayersClicked(Guest& guest, x86::reg32 players)
{
    guest.setDword(kPlayers, players);
    guest.setDword(kNetworkGame, 0);
    guest.setDword(kNetworkConnected, 0);
    guest.setWord(kNetworkHost, 1);
    guest.call(kOpenRaceDialog, fe::args({ 1 }));
    return 2;
}

/* sub_452d80: the web site's "not possible" answered: the dialog closes. */
x86::reg32 setupWebSiteRefusedBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    if (answer != fe::kDialogEnter && answer != fe::kDialogEscape)
        return 0;
    fe::dialogFocus(guest);
    if (closes != 0)
        guest.setByte(closes, 1);
    return 2;
}

/* sub_452db0: whether the screen up is setup itself. */
x86::reg32 setupIsCurrentBody(Guest& guest)
{
    const x86::reg32 menu = guest.call(kCurrentMenu);
    return guest.call(kStricmp, fe::args({ guest.dword(menu + fe::menu::kName), kSetupName.address })) == 0 ? 1 : 0;
}

/* sub_452de0: the web site's question answered: Yes leaves the menus for it,
 * coming back by which screen it was asked on. */
x86::reg32 setupWebSiteAnsweredBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    if (answer == fe::kDialogEnter)
    {
        const x86::reg32 focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            guest.setDword(kWebSiteAnswer, guest.call(kSetupIsCurrent) != 0 ? 1 : 2);
            return 0xa;
        }
        if (closes != 0)
            guest.setByte(closes, focus == 1 ? 1 : 0);
        return 2;
    }
    if (answer == fe::kDialogEscape)
    {
        if (closes != 0)
            guest.setByte(closes, 1);
        return 2;
    }
    return 0;
}

/* sub_452e60: the web site's button: asked first, or refused if the machine
 * has no way to dial out. */
x86::reg32 setupWebSiteClickedBody(Guest& guest)
{
    constexpr x86::reg32 kLines = 0x664660, kButtons = 0x66466c;
    fe::Question question{};
    question.lines = kLines;
    question.buttons = kButtons;
    if (guest.call(kDialUpRunning) != 0 || guest.call(kDialUpEntries) != 0)
    {
        guest.setDword(kLines, fe::text(guest, 0x1c0));
        guest.setDword(kLines + 4, fe::text(guest, 0x1c1));
        guest.setDword(kButtons, fe::text(guest, 0xc4));
        guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
        question.lineCount = 2;
        question.buttonCount = 2;
        question.callback = 0x452de0;
    }
    else
    {
        guest.setDword(kLines, fe::text(guest, 0x1c2));
        guest.setDword(kButtons, fe::text(guest, 0xf9));
        question.lineCount = 1;
        question.buttonCount = 1;
        question.callback = 0x452d80;
    }
    fe::openQuestion(guest, question);
    return 0;
}

/* ---------------------------------------------------------------------------
 * The race's results
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kParticipants = 0x6647b0;
constexpr x86::reg32 kParticipantSize = 0x9ac;
constexpr x86::reg32 kGroupSize = 0x9ac0;           // 16 of them
constexpr x86::reg32 kGroupCounts = 0x677d30;
constexpr x86::reg32 kPartId = 0x1f0;
constexpr x86::reg32 kPartFlags = 0x200;            // byte
constexpr x86::reg32 kPartCar = 0x220;              // its car's record; +0x10 the car, +0x34 its name
constexpr x86::reg32 kPartRank = 0x238;
constexpr x86::reg32 kPartLapTime = 0x2b8;          // float
constexpr x86::reg32 kPartHuman = 0x2bc;
constexpr x86::reg32 kPartState = 0x2c4;
constexpr x86::reg32 kPartPoints = 0x2c8;
constexpr x86::reg32 kPartLaps = 0x2cc;
constexpr x86::reg32 kPartTime = 0x2d4;
constexpr x86::reg32 kCarRanksA = 0x6fd42e;         // int16 for each car
constexpr x86::reg32 kCarRanksB = 0x6fd44e;
constexpr x86::reg32 kSortOrders = 0x5595d8;        // int16[16] for each group: the participants in order, or -1
constexpr x86::reg32 kSortKey = 0x559618;
constexpr x86::reg32 kSortDescending = 0x55961c;

constexpr x86::reg32 kCompareParticipants = 0x455840;
constexpr x86::reg32 kQsort = 0x4ee4f8;             // eax the array, edx its count, ebx 4, ecx how to compare
constexpr x86::reg32 kSortParticipants = 0x455c10;
constexpr x86::reg32 kNthParticipant = 0x455cd0;
constexpr x86::reg32 kNthSorted = 0x455dd0;
constexpr x86::reg32 kParticipantById = 0x4554e0;
constexpr x86::reg32 kParticipantRanks = 0x455530;
constexpr x86::reg32 kParticipantMatches = 0x4557b0;

x86::reg32 participant(x86::reg32 group, std::int32_t index)
{
    return kParticipants + group * kGroupSize + x86::reg32(index) * kParticipantSize;
}

/* sub_4554e0: a group's participant by its id, or 0.  Edx left as the search
 * leaves it, which no caller reads. */
x86::reg32 participantByIdBody(Guest& guest, x86::reg32 group, x86::reg32 id)
{
    x86::reg32 left = id;
    for (std::int32_t i = 0; i < signedDword(guest, kGroupCounts + 4 * group); ++i)
    {
        left = x86::reg32(i) * kParticipantSize;
        const x86::reg32 found = participant(group, i);
        if (guest.dword(found + kPartId) == id)
        {
            guest.leaveEdx(left);
            return found;
        }
    }
    guest.leaveEdx(left);
    return 0;
}

/* sub_455530: its car's two ranks, into *a and *b, if there is one. */
x86::reg32 participantRanksBody(Guest& guest, x86::reg32 found, x86::reg32 a, x86::reg32 b)
{
    if (found == 0)
        return 0;
    guest.setDword(a, x86::reg32(signedWord(guest, 2 * guest.dword(guest.dword(found + kPartCar) + 0x10) + kCarRanksA)));
    const x86::reg32 rank = x86::reg32(signedWord(guest, 2 * guest.dword(guest.dword(found + kPartCar) + 0x10) + kCarRanksB));
    guest.setDword(b, rank);
    return rank;
}

/* sub_4557b0: whether it is what the mask asks: 4 and 8 flags it has, 1 and 2
 * flags it has not, 0x10 and 0x20 its state is or is not `state`. */
x86::reg32 participantMatchesBody(Guest& guest, x86::reg32 found, x86::reg32 mask, x86::reg32 state)
{
    const std::uint8_t flags = guest.byte(found + kPartFlags);
    if ((mask & 4) && !(flags & 2))
        return 0;
    if ((mask & 8) && !(flags & 4))
        return 0;
    if ((mask & 1) && (flags & 2))
        return 0;
    if ((mask & 2) && (flags & 4))
        return 0;
    if ((mask & 0x10) && state != guest.dword(found + kPartState))
        return 0;
    if ((mask & 0x20) && state == guest.dword(found + kPartState))
        return 0;
    return 1;
}

/* Bigger first: -1 when b's field is the bigger. */
std::int32_t biggerFirst(const Guest& guest, x86::reg32 a, x86::reg32 b, x86::reg32 field)
{
    const std::int32_t va = signedDword(guest, a + field), vb = signedDword(guest, b + field);
    return vb > va ? -1 : vb < va ? 1 : 0;
}

/* By a car's rank, and the same rank by who has flag 4 last. */
std::int32_t byCarRank(const Guest& guest, x86::reg32 a, x86::reg32 b, x86::reg32 ranks)
{
    const std::int32_t ra = signedWord(guest, 2 * guest.dword(guest.dword(a + kPartCar) + 0x10) + ranks);
    const std::int32_t rb = signedWord(guest, 2 * guest.dword(guest.dword(b + kPartCar) + 0x10) + ranks);
    if (ra != rb)
        return ra < rb ? -1 : 1;
    const bool fa = (guest.byte(a + kPartFlags) & 4) != 0, fb = (guest.byte(b + kPartFlags) & 4) != 0;
    return fa == fb ? 0 : fa ? 1 : -1;
}

/* sub_455840, for qsort: two participants in the order the sort asked for by
 * kSortKey (0 none, 1 rank, 2 lap time, 3 and 4 the car's ranks, 5 laps, 6
 * points, 7 time, 8 the car's name), ties by rank. */
x86::reg32 participantCompareBody(Guest& guest, x86::reg32 aAt, x86::reg32 bAt)
{
    const x86::reg32 a = guest.dword(aAt), b = guest.dword(bAt);
    std::int32_t order = 0;
    switch (guest.dword(kSortKey))
    {
    case 1:
        order = biggerFirst(guest, a, b, kPartRank);
        break;
    case 2:
    {
        // Three comparisons as the game makes them: NaN against anything is 0.
        fe::X87 x87(guest);
        const x86::Float fa = x87.load(a + kPartLapTime), fb = x87.load(b + kPartLapTime);
        if (x87.below(fa, fb))
            order = -1;
        if (x87.above(fa, fb))
            order = 1;
        guest.fpu().compare(fa, fb);
        if (guest.fpu().status.c3)
            order = 0;
        break;
    }
    case 3:
        order = byCarRank(guest, a, b, kCarRanksA);
        break;
    case 4:
        order = byCarRank(guest, a, b, kCarRanksB);
        break;
    case 5:
        order = biggerFirst(guest, a, b, kPartLaps);
        break;
    case 6:
        order = biggerFirst(guest, a, b, kPartPoints);
        break;
    case 7:
    {
        // Smaller first, those without a time last, then more laps first.
        const std::int32_t ta = signedDword(guest, a + kPartTime), tb = signedDword(guest, b + kPartTime);
        if (ta != 0 && tb != 0)
            order = ta < tb ? -1 : tb < ta ? 1 : 0;
        else if (ta != 0)
            order = -1;
        else if (tb != 0)
            order = 1;
        else
            order = -biggerFirst(guest, a, b, kPartLaps);
        break;
    }
    case 8:
        order = std::int32_t(guest.call(kStricmp, fe::args({ guest.dword(a + kPartCar) + 0x34,
                                                             guest.dword(b + kPartCar) + 0x34 })));
        break;
    default:
        break;
    }
    if (guest.dword(kSortDescending) == 1)
        order = -order;
    if (order == 0)
        order = biggerFirst(guest, a, b, kPartRank);
    guest.leaveEdx(x86::reg32(order));
    return x86::reg32(order);
}

/* sub_455c10: a group sorted, by a key and a way: the order's table filled,
 * -1 past its participants.  (What the loops leave in eax, ebx and edx goes
 * on, read by no caller.) */
x86::reg32 participantSortBody(Guest& guest, x86::reg32 group, x86::reg32 key, x86::reg32 descending)
{
    const x86::reg32 sorted = guest.frame();    // x86::reg32[16]
    const x86::reg32 order = kSortOrders + (group << 5);
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        guest.setWord(order + 2 * i, 0xffff);
        guest.setDword(sorted + 4 * i, participant(group, std::int32_t(i)));
    }
    guest.setDword(kSortKey, key);
    guest.setDword(kSortDescending, descending);
    const x86::reg32 count = guest.dword(kGroupCounts + 4 * group);
    guest.call(kQsort, fe::args({ sorted, count, 4, kCompareParticipants }));
    const std::int32_t n = signedDword(guest, kGroupCounts + 4 * group);
    if (n <= 0)
    {
        guest.leaveEdx(0);
        return guest.eax();
    }
    for (std::int32_t i = 0; i < n; ++i)
        for (std::int32_t j = 0; j < n; ++j)
            if (participant(group, j) == guest.dword(sorted + 4 * x86::reg32(i)))
                guest.setWord(order + 2 * x86::reg32(i), std::uint16_t(j));
    guest.leaveEbx(kParticipants + group * kGroupSize);
    guest.leaveEdx(x86::reg32(n));
    return x86::reg32(n);
}

/* sub_455cd0: the index-th in the order that matches a mask (0: any), or 0.
 * Past the participants, or past the 16 slots, none.  (Ebx, ecx and edx left
 * as its search leaves them, read by no caller.) */
x86::reg32 participantNthBody(Guest& guest, x86::reg32 group, x86::reg32 index, x86::reg32 mask, x86::reg32 state)
{
    if (std::int32_t(index) > 0x10)
        return 0;
    const x86::reg32 order = kSortOrders + (group << 5);
    const std::int32_t count = signedDword(guest, kGroupCounts + 4 * group);
    if (mask == 0)
    {
        guest.leaveEdx(index + index);
        const std::int32_t at = signedWord(guest, order + 2 * index);
        if (at < 0)
            return 0;
        guest.leaveEcx(x86::reg32(count));
        if (at > count)
            return 0;
        guest.leaveEdx(x86::reg32(at) * kParticipantSize);
        return participant(group, at);
    }
    x86::reg32 leftEbx = 0, leftEdx = index;
    x86::reg32 found = 0;
    for (x86::reg32 slot = 0; slot < 16; ++slot)
    {
        leftEdx = (group << 5) + 2 * slot;
        const std::uint16_t word = guest.word(order + 2 * slot);
        leftEbx = (leftEbx & 0xffff0000) | word;
        const std::int32_t at = std::int16_t(word);
        if (at < 0 || at > count)
            continue;
        const x86::reg32 candidate = participant(group, at);
        const x86::reg32 matches = guest.call(kParticipantMatches, fe::args({ candidate, mask, state }));
        leftEdx = mask;
        leftEbx = state;
        if (matches == 0)
            continue;
        if (found == index)
        {
            guest.leaveEbx(leftEbx);
            guest.leaveEcx(slot);
            guest.leaveEdx(leftEdx);
            return candidate;
        }
        ++found;
        leftEbx = found;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEcx(16);
    guest.leaveEdx(leftEdx);
    return 0;
}

/* sub_455dd0: the same for an order made for the occasion, the group's kept
 * as it was. */
x86::reg32 participantNthSortedBody(Guest& guest, x86::reg32 group, x86::reg32 index, x86::reg32 key,
                                    x86::reg32 descending, x86::reg32 mask, x86::reg32 state)
{
    const x86::reg32 order = kSortOrders + (group << 5);
    const x86::reg32 kept = guest.frame();      // std::int16_t[16]
    guest.call(kCopy, fe::args({ order, kept, 0x20 }));
    guest.call(kSortParticipants, fe::args({ group, key, descending }));
    const x86::reg32 found = guest.call(kNthParticipant, fe::args({ group, index, mask, state }));
    guest.call(kCopy, fe::args({ kept, order, 0x20 }));
    guest.leaveEcx(found);
    return found;
}

/* ---------------------------------------------------------------------------
 * records
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kRecordsFile = 0x559408;       // the records, loaded
constexpr x86::reg32 kRecordTimes = 0x55940c;       // the track's chosen, in it
constexpr x86::reg32 kRecordHolder = 0x559410;
constexpr x86::reg32 kRecordsDelete = 0x559424;     // the item
constexpr x86::reg32 kRecordsLoaded = 0x44f5d0;
constexpr x86::reg32 kRecordsSelect = 0x44f670;
constexpr x86::reg32 kRecordsRead = 0x44fa90;
constexpr x86::reg32 kRecordsClear = 0x44fbc0;
constexpr x86::reg32 kRecordsAnyOf = 0x44f8d0;
constexpr x86::reg32 kLoadRecords = 0x44f510;
constexpr x86::reg32 kSaveRecords = 0x44f5f0;
constexpr GuestString kRecordsDeleteItem{ 0x5395c0, "RECORD_DELETE" };
constexpr GuestString kRecordsSheet{ 0x5395d0, "RECORDS_SHEET" };

/* sub_44f670: a track's and a direction's records chosen.  Edx left at the
 * holder's, read by no caller. */
x86::reg32 recordsSelectBody(Guest& guest, x86::reg32 track, x86::reg32 kind)
{
    const x86::reg32 file = guest.dword(kRecordsFile);
    const x86::reg32 times = x86::reg32(std::int32_t(kind) % 3) * 0x15634 + file + 0x16e0 + track * 0x1f1c;
    const x86::reg32 offset = track * 0x214;
    const x86::reg32 holder = file + 4 + offset;
    guest.setDword(kRecordTimes, times);
    guest.setDword(kRecordHolder, holder);
    guest.leaveEdx(holder);
    return offset;
}

/* sub_44fa90: the holder of the chosen record: whether there is one, and its
 * names, into three buffers. */
x86::reg32 recordsReadBody(Guest& guest, x86::reg32 first, x86::reg32 second, x86::reg32 third)
{
    if (guest.dword(guest.dword(kRecordHolder)) != 0)
    {
        guest.copyString(first, guest.dword(kRecordHolder) + 8);
        guest.copyString(second, guest.dword(kRecordHolder) + 0x13);
        guest.copyString(third, guest.dword(kRecordHolder) + 0x113);
    }
    return guest.dword(guest.dword(kRecordHolder));
}

/* sub_44fbc0: the chosen records wiped. */
x86::reg32 recordsClearBody(Guest& guest)
{
    guest.setDword(guest.dword(kRecordTimes), 0);
    const x86::reg32 holder = guest.dword(kRecordHolder);
    guest.setDword(holder, 0);
    return holder;
}

/* sub_450f70, records as it opens. */
x86::reg32 recordsOnEnterBody(Guest& guest, x86::reg32 menu)
{
    Item erase = fe::findItem(guest, menu, kRecordsDeleteItem);
    guest.setDword(kRecordsDelete, erase.address());
    if (erase)
        erase.setOnClick(0x450eb0);
    const x86::reg32 language = guest.dword(kLanguage);
    if (language == 1 || language == 5)
    {
        if (Item sheet = fe::findItem(guest, menu, kRecordsSheet))
        {
            guest.setWord(sheet.address() + 0x44, 0x59);
            guest.setWord(sheet.address() + 0x46, 0x2a);
            guest.setWord(sheet.address() + 0x42, 0x48);
        }
    }
    return 0;
}

/* sub_450f20, records on every pass: delete is open while there is something
 * to delete. */
x86::reg32 recordsOnFrameBody(Guest& guest)
{
    constexpr x86::reg32 kChosenTrack = 0x559418, kChosenKind = 0x55941c;
    if (guest.dword(kRecordsDelete) == 0)
        return 0;
    bool any = false;
    if (guest.call(kRecordsLoaded) != 0)
        any = guest.call(kRecordsAnyOf, fe::args({ guest.dword(kChosenTrack), guest.dword(kChosenKind) })) != 0;
    Item(guest, guest.dword(kRecordsDelete)).setDisabled(!any);
    return 0;
}

/* sub_450eb0: delete's click: asked first. */
x86::reg32 recordsDeleteClickedBody(Guest& guest)
{
    constexpr x86::reg32 kButtons = 0x6645e4, kLine = 0x6645ec;
    guest.setDword(kButtons, fe::text(guest, 0xc4));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
    guest.setDword(kLine, fe::text(guest, 0x145));
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x450e50;
    fe::openQuestion(guest, question);
    return 2;
}

/* sub_450e50: the answer: Yes wipes them.  Either closes the dialog. */
x86::reg32 recordsDeleteAnsweredBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    if (answer == fe::kDialogEnter || answer == fe::kDialogEscape)
    {
        if (answer == fe::kDialogEnter && fe::dialogFocus(guest) == 0)
            guest.call(kRecordsClear);
        if (closes != 0)
            guest.setByte(closes, 1);
        return 2;
    }
    if (closes != 0)
        guest.setByte(closes, 0);
    return 0;
}

/* sub_4cef00: whether a track has a record's holder, the records' file loaded
 * for the moment if it is not. */
x86::reg32 recordsHolderBody(Guest& guest, x86::reg32 track)
{
    if (guest.byte(kDeveloperFlags) & kPracticeOnly)
        return 1;
    const x86::reg32 names = guest.frame();     // char[0x100]
    const x86::reg32 trackIndex = x86::reg32(std::int32_t(std::int16_t(track)));
    const bool loaded = guest.call(kRecordsLoaded) != 0;
    if (!loaded)
        guest.call(kLoadRecords);
    guest.call(kRecordsSelect, fe::args({ trackIndex, 0, names }));
    const x86::reg32 holder = guest.call(kRecordsRead, fe::args({ names, names, names }));
    if (!loaded)
        guest.call(kSaveRecords);
    return holder;
}

/* ---------------------------------------------------------------------------
 * postgame
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kSheetSummary = 0x5593f0;
constexpr x86::reg32 kSheetToggle = 0x5593f4;
constexpr x86::reg32 kSheetRecords = 0x5593f8;
constexpr x86::reg32 kSheetTournament = 0x5593fc;
constexpr x86::reg32 kSheetCop = 0x559400;
constexpr x86::reg32 kSheetPerformance = 0x559404;
constexpr x86::reg32 kSheetShown = 0x55936c;
constexpr x86::reg32 kSheetCycle = 0x559370;        // int16[], which sheets are toggled through, ending 0
// Each sheet's state word, and its fade's colour next to it: 3 hidden, 2 hiding,
// 1 shown.
constexpr x86::reg32 kSummaryState = 0x88;
constexpr x86::reg32 kRecordsState = 0xb8;
constexpr x86::reg32 kTableState = 0x7e;
constexpr x86::reg32 kPerformanceState = 0x6c;
constexpr std::uint16_t kSheetHidden = 3, kSheetHiding = 2, kSheetUp = 1;
constexpr std::uint16_t kSheetFadeColour = 0x2a8;
constexpr x86::reg32 kCyclePlain = 0x5593b0;
constexpr x86::reg32 kCycleTournament = 0x559390;
constexpr x86::reg32 kCycleDuel = 0x5593d0;

constexpr x86::reg32 kTrackNameIds = 0x44e152;      // int16[10]: a track's name's line, in the game's code segment
constexpr x86::reg32 kLaps = 0x6fd4b4;
constexpr x86::reg32 kFirstId = 0x6fd4f4;           // the first player's participant id
constexpr x86::reg32 kCanSave = 0x6fd3ac;
constexpr x86::reg32 kTrackLine = 0x6644c4;
constexpr x86::reg32 kTrackUnlocked = 0x556382;     // int16 for each track
constexpr x86::reg32 kTrackNames = 0x55636e;
constexpr x86::reg32 kFinishers = 0x4cef80;
constexpr x86::reg32 kFinishersCount = 0x4cf000;
constexpr x86::reg32 kReplayExcluded = 0x4cf040;
constexpr x86::reg32 kRaceAgain = 0x4cf090;
constexpr x86::reg32 kCheckAwards = 0x44e1a0;
constexpr x86::reg32 kAwardMessage = 0x44e170;
constexpr x86::reg32 kUnlockTrack = 0x442b60;
constexpr x86::reg32 kLeaveNetwork = 0x44e4c0;
constexpr x86::reg32 kAskLeave = 0x44e550;
constexpr x86::reg32 kSaveDialog = 0x447a30;        // eax what: 0 a knockout, 1 a tournament, 2 a replay, 3 a ghost
constexpr x86::reg32 kLeaveAll = 0x4a6a10;          // eax, edx: the machines told, and waited for
constexpr x86::reg32 kKeptReplay = 0x446990;
constexpr x86::reg32 kMessageById = 0x444be0;
constexpr x86::reg32 kDescriptionService = 0x4c9860;
constexpr x86::reg32 kSpokenStop = 0x4c99d0;

constexpr GuestString kPgRaceNow{ 0x539494, "racenow" };
constexpr GuestString kPgSpeech{ 0x53949c, "PG_SPEECH" };
constexpr GuestString kPgTrackAndLaps{ 0x5394a8, "TRACK_AND_LAPS" };
constexpr GuestString kPgTrackFormat{ 0x5394b8, "%d %s %s" };
constexpr GuestString kPgRecords{ 0x5394c4, "RECORDS_SHEET" };
constexpr GuestString kPgSummary{ 0x5394d4, "RACESUMMARY_SHEET" };
constexpr GuestString kPgTournament{ 0x5394e8, "TOURNAMENT_SHEET" };
constexpr GuestString kPgCop{ 0x5394fc, "COP_SHEET" };
constexpr GuestString kPgToggle{ 0x539508, "TOGGLE_SHEETS" };
constexpr GuestString kPgPerformance{ 0x539518, "PERFORMANCE_SHEET" };
constexpr GuestString kPgViewReplay{ 0x53952c, "viewreplay" };
constexpr GuestString kPgSaveReplay{ 0x539538, "savereplay" };
constexpr GuestString kPgSaveGhost{ 0x539544, "Save_Ghost" };
constexpr GuestString kPgSaveTournament{ 0x539550, "Save_Tourny" };
constexpr GuestString kPgSaveKnockout{ 0x53955c, "Save_KO" };
constexpr GuestString kPgMainMenu{ 0x539564, "Main_Menu" };
constexpr GuestString kPgQuit{ 0x539570, "Quit" };

/* sub_442b60: a bonus track open from now on. */
x86::reg32 unlockTrackBody(Guest& guest, x86::reg32 track)
{
    guest.setWord(2 * track + kTrackUnlocked, 1);
    guest.setWord(2 * track + kTrackNames, std::uint16_t(track + 0x66));
    return track + 0x66;
}

/* sub_44e170: a line of an award. */
x86::reg32 awardMessageBody(Guest& guest, x86::reg32 id)
{
    constexpr x86::reg32 kLine = 0x6644a8;
    guest.setDword(kLine, fe::text(guest, id));
    return openMessage(guest, 1, kLine, kLongMessage);
}

/* sub_44e1a0: what the race just won opens: a bonus track or car, and the line
 * that says so, the last one won.  (What a pass leaves in eax goes on as the
 * result, read by no caller.) */
x86::reg32 checkAwardsBody(Guest& guest)
{
    constexpr x86::reg32 kAwards = 0x6fd2a8;            // byte: what has been won
    constexpr x86::reg32 kBonusTrackOpen = 0x6fd2ac;
    constexpr x86::reg32 kHard = 0x6fd470;
    constexpr x86::reg32 kFirstTournamentTrack = 0x55d020;
    constexpr x86::reg32 kDuelsWon = 0x6fd2b0;          // int16, a bit for each track
    constexpr x86::reg32 kDuelsLost = 0x6fd2b2;
    constexpr x86::reg32 kLapsNeeded = 0x6fd4b8;
    constexpr std::int32_t kLastTrack = 8;
    const std::int32_t lastPlayer = guest.dword(kPlayers) == 1 ? 1 : 0;
    guest.call(kSortParticipants, fe::args({ 0, 0, 0 }));
    x86::reg32 message = 0;
    x86::reg32 left = 0;
    const auto win = [&](std::uint8_t bit) {
        guest.setByte(kAwards, std::uint8_t(guest.byte(kAwards) | bit));
    };
    const x86::reg32 ours = guest.frame(), theirs = guest.frame() + 4, second = guest.frame() + 8;
    for (std::int32_t player = 0; player <= lastPlayer; ++player)
    {
        const x86::reg32 id = guest.dword(kFirstId) + x86::reg32(player);
        const x86::reg32 type = guest.dword(kRaceType);
        left = type;
        if (type == 2)
        {
            const x86::reg32 found = guest.call(kParticipantById, fe::args({ 0, id }));
            left = found;
            if (found == 0)
                continue;
            const x86::reg32 track = guest.dword(kTrack);
            if (track != x86::reg32(kLastTrack) || guest.dword(found + kPartState) == 2)
                continue;
            guest.call(kUnlockTrack, fe::args({ track }));
            if (guest.dword(kBonusTrackOpen) == 0)
                message = 0x1b7;
            guest.setDword(kBonusTrackOpen, 1);
            left = guest.dword(kHard);
            if (left != 1)
                continue;
            if (!(guest.byte(kAwards) & 1))
                message = message == 0x1b7 ? 0x1b5 : 0x1b6;
            win(1);
        }
        else if (type == 1)
        {
            const x86::reg32 found = guest.call(kParticipantById, fe::args({ 0, id }));
            left = found;
            if (found == 0 || guest.dword(kFirstTournamentTrack) != guest.dword(kTrack))
                continue;
            const x86::reg32 best = guest.call(kNthSorted, fe::args({ 0, 0, 4, 1 }, { 0, 0 }));
            guest.setDword(ours, 0);
            guest.setDword(theirs, 0);
            guest.setDword(second, 0);
            guest.call(kParticipantRanks, fe::args({ best, theirs, ours }));
            guest.call(kParticipantRanks, fe::args({ found, theirs, second }));
            left = guest.dword(second);
            if (std::int32_t(left) < signedDword(guest, ours))
                continue;
            if (!(guest.byte(kAwards) & 4))
            {
                message = 0x1b4;
                win(4);
            }
            if (guest.dword(kHard) != 1 || (guest.byte(kAwards) & 2))
                continue;
            message = message == 0x1b4 ? 0x1b2 : 0x1b3;
            win(2);
        }
        else if (type == 3)
        {
            const std::int32_t track = signedDword(guest, kTrack);
            const std::uint32_t trackBit = 1u << (std::uint32_t(track) & 31);
            const x86::reg32 fastest = guest.call(kParticipantById, fe::args({ 1, id }));
            if (fastest != 0 && (guest.byte(fastest + kPartFlags) & 0x20)
                && signedDword(guest, kLapsNeeded) <= signedDword(guest, fastest + kPartLaps))
            {
                if (track <= kLastTrack)
                    guest.setWord(kDuelsWon, std::uint16_t(guest.word(kDuelsWon) | trackBit));
                if (guest.word(kDuelsWon) == 0x1ff)
                {
                    if (!(guest.byte(kAwards) & 8))
                        message = 0x1b8;
                    win(8);
                }
            }
            const x86::reg32 found = guest.call(kParticipantById, fe::args({ 0, id }));
            left = found;
            if (found == 0 || (guest.byte(found + kPartFlags) & 0x20) || guest.dword(found + kPartHuman) != 1
                || guest.dword(found + kPartState) == 4)
                continue;
            if (track <= kLastTrack)
            {
                left = trackBit;
                guest.setWord(kDuelsLost, std::uint16_t(guest.word(kDuelsLost) | trackBit));
            }
            if (guest.word(kDuelsLost) == 0x1ff)
            {
                if (!(guest.byte(kAwards) & 0x10))
                    message = 0x1b9;
                win(0x10);
            }
        }
    }
    if (message != 0)
        return guest.call(kAwardMessage, fe::args({ message }));
    return left;
}

/* sub_44e4c0: a network race left, by whichever way this machine has. */
x86::reg32 leaveNetworkBody(Guest& guest)
{
    if (signedDword(guest, kPlayers) > 1 && guest.dword(kNetworkConnected) != 0 && guest.word(kNetworkLeaving) == 0)
    {
        if (guest.word(kNetworkHost) != 0)
            guest.call(kLeaveAll, fe::args({ 0x28, 0x21 }));
        else
            guest.call(kCloseNetwork);
    }
    return guest.eax();
}

/* sub_44e510: "leave?" answered. */
x86::reg32 leaveAnsweredBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    if (fe::dialogFocus(guest) == 1)
    {
        guest.setByte(closes, 1);
        return 0;
    }
    guest.setByte(closes, 1);
    guest.call(kLeaveNetwork);
    return 0xb;
}

/* sub_44e550: "leave?" with Yes and No. */
x86::reg32 askLeaveBody(Guest& guest, x86::reg32 lineId)
{
    constexpr x86::reg32 kLine = 0x6644b4, kButtons = 0x6644ac;
    guest.setDword(kLine, fe::text(guest, lineId));
    guest.setDword(kButtons, fe::text(guest, 0xf9));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc6));
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x44e510;
    fe::openQuestion(guest, question);
    return guest.eax();
}

/* sub_44e5b0: Main Menu's click: asked first if a series has more to race. */
x86::reg32 mainMenuClickedBody(Guest& guest)
{
    if (guest.call(kRaceAgain) != 0)
    {
        const x86::reg32 type = guest.dword(kRaceType);
        if (type == 1 || type == 2)
        {
            guest.call(kAskLeave, fe::args({ type == 1 ? 0xfeu : 0xffu }));
            return 2;
        }
    }
    return 0xb;
}

/* sub_44e600: the same in a network race. */
x86::reg32 networkMainMenuClickedBody(Guest& guest)
{
    if (guest.word(kNetworkHost) == 0)
    {
        if (guest.dword(kNetworkConnected) != 0 && guest.word(kNetworkLeaving) == 0)
        {
            guest.call(kAskLeave, fe::args({ 0x101 }));
            return 2;
        }
        return 0xb;
    }
    if (guest.call(kRaceAgain) == 0)
    {
        guest.call(kLeaveNetwork);
        return 0xb;
    }
    const x86::reg32 type = guest.dword(kRaceType);
    guest.call(kAskLeave, fe::args({ type == 1 ? 0xfeu : type == 2 ? 0xffu : 0xfdu }));
    return 2;
}

/* sub_44e6c0: why a replay of this race cannot be kept. */
x86::reg32 cannotSaveReplayBody(Guest& guest)
{
    constexpr x86::reg32 kLines = 0x6644b8;
    guest.setDword(kLines, fe::text(guest, 0x1cd));
    guest.setDword(kLines + 4, fe::text(guest, 0x1ce));
    openMessage(guest, 2, kLines, kLongMessage);
    return 2;
}

/* sub_44e710: View Replay's click: the kept replay, the one just raced, the
 * two tables it loads over kept as they were. */
x86::reg32 viewReplayClickedBody(Guest& guest)
{
    constexpr x86::reg32 kCars = 0x6fd3bc, kCarsBytes = 0x50;
    constexpr x86::reg32 kOpponents = 0x6fd40c, kOpponentsBytes = 0x62;
    std::array<std::uint8_t, kCarsBytes> cars;
    std::array<std::uint8_t, kOpponentsBytes> opponents;
    for (x86::reg32 i = 0; i < kCarsBytes; ++i)
        cars[i] = guest.byte(kCars + i);
    for (x86::reg32 i = 0; i < kOpponentsBytes; ++i)
        opponents[i] = guest.byte(kOpponents + i);
    const x86::reg32 name = guest.frame() + 0xb4;
    guest.setByte(name, guest.byte(0x44e150));      // the empty string
    x86::reg32 result = 2;
    if (guest.call(kKeptReplay, fe::args({ name })) != 0)
        result = 7;
    else
        guest.call(kMessageById, fe::args({ 0x41 }));
    for (x86::reg32 i = 0; i < kCarsBytes; ++i)
        guest.setByte(kCars + i, cars[i]);
    for (x86::reg32 i = 0; i < kOpponentsBytes; ++i)
        guest.setByte(kOpponents + i, opponents[i]);
    return result;
}

/* sub_44e7a0: the toggle's click: the next sheet in the race's cycle. */
x86::reg32 toggleSheetsBody(Guest& guest)
{
    const x86::reg32 shown = guest.dword(kSheetShown);
    guest.setDword(kSheetShown, guest.word(2 * shown + kSheetCycle + 2) != 0 ? shown + 1 : 0);
    return 2;
}

/* sub_44efd0, postgame as it opens. */
x86::reg32 postgameOnEnterBody(Guest& guest, x86::reg32 menu)
{
    if (guest.call(kFinishers) == 0)
        guest.call(kCheckAwards);
    if (Item speech = fe::findItem(guest, menu, kPgSpeech))
        guest.setDword(speech.address() + kOnDraw, 0x44ed30);
    if (Item trackAndLaps = fe::findItem(guest, menu, kPgTrackAndLaps))
    {
        const x86::reg32 trackName = fe::text(guest, x86::reg32(signedWord(guest, kTrackNameIds + 2 * guest.dword(kTrack))));
        const x86::reg32 lapsWord = fe::text(guest, 5);
        guest.call(kSprintf, fe::args({}, { kTrackLine, kPgTrackFormat.address, guest.dword(kLaps), lapsWord, trackName }));
        trackAndLaps.setText(kTrackLine);
    }
    const x86::reg32 records = fe::findItem(guest, menu, kPgRecords).address();
    guest.setDword(kSheetRecords, records);
    const x86::reg32 summary = fe::findItem(guest, menu, kPgSummary).address();
    guest.setDword(kSheetSummary, summary);
    const x86::reg32 tournament = fe::findItem(guest, menu, kPgTournament).address();
    guest.setDword(kSheetTournament, tournament);
    const x86::reg32 cop = fe::findItem(guest, menu, kPgCop).address();
    guest.setDword(kSheetCop, cop);
    const x86::reg32 toggle = fe::findItem(guest, menu, kPgToggle).address();
    guest.setDword(kSheetToggle, toggle);
    const x86::reg32 performance = fe::findItem(guest, menu, kPgPerformance).address();
    guest.setDword(kSheetPerformance, performance);
    if (summary == 0 || toggle == 0 || records == 0 || tournament == 0 || cop == 0 || performance == 0)
        return 0;

    guest.setDword(toggle + fe::item::kOnClick, 0x44e7a0);
    const auto hideSheet = [&](x86::reg32 sheet, x86::reg32 state) {
        guest.setWord(sheet + state, kSheetHidden);
        guest.setWord(sheet + state + 2, kSheetFadeColour);
    };
    hideSheet(performance, kPerformanceState);
    hideSheet(records, kRecordsState);
    hideSheet(summary, kSummaryState);
    hideSheet(tournament, kTableState);
    hideSheet(cop, kTableState);

    const x86::reg32 type = guest.dword(kRaceType);
    if (type == 1)
    {
        guest.call(kCopy, fe::args({ kCycleTournament, kSheetCycle, 0x20 }));
        guest.setDword(kSheetShown, 3);
    }
    else if (type == 3 && guest.call(kNthParticipant, fe::args({ 1, 0, 1, 0 })) != 0)
    {
        guest.call(kCopy, fe::args({ kCycleDuel, kSheetCycle, 0x20 }));
        guest.setDword(kSheetShown, guest.call(kParticipantById, fe::args({ 1, guest.dword(kFirstId) })) != 0 ? 3 : 2);
    }
    else
    {
        guest.call(kCopy, fe::args({ kCyclePlain, kSheetCycle, 0x20 }));
        guest.setDword(kSheetShown, 2);
    }

    const std::int32_t players = signedDword(guest, kPlayers);
    if (Item viewReplay = fe::findItem(guest, menu, kPgViewReplay))
    {
        if (players > 1 || guest.dword(kCanSave) == 0)
            viewReplay.setDisabled(true);
        else
            viewReplay.setOnClick(0x44e710);
    }
    if (Item saveReplay = fe::findItem(guest, menu, kPgSaveReplay))
    {
        if (guest.dword(kCanSave) == 0)
            saveReplay.setDisabled(true);
        else if (guest.call(kReplayExcluded) != 0)
            saveReplay.setOnClick(0x44e6c0);
        else if (players > 1 && guest.dword(kNetworkGame) != 0)
            saveReplay.setDisabled(true);
        else
            saveReplay.setOnClick(0x44e6a0);
    }
    if (Item raceNow = fe::findItem(guest, menu, kPgRaceNow))
    {
        if (guest.call(kRaceAgain) == 0)
            raceNow.setDisabled(true);
        raceNow.setText(fe::text(guest, 0xf2));
        if (guest.dword(kRaceType) == 2 || guest.dword(kRaceType) == 1)
            raceNow.setText(fe::text(guest, 0xf1));
    }
    if (Item saveGhost = fe::findItem(guest, menu, kPgSaveGhost))
    {
        const x86::reg32 fastest = guest.call(kParticipantById, fe::args({ 1, guest.dword(kFirstId) }));
        const x86::reg32 raceType = guest.dword(kRaceType);
        if (guest.dword(kCanSave) != 0 && guest.dword(kPlayers) == 0 && raceType != 1 && raceType != 2 && fastest == 0)
        {
            saveGhost.setOnClick(0x44e4a0);
            saveGhost.setDisabled(false);
        }
        else
        {
            saveGhost.setDisabled(true);
        }
    }
    if (Item saveTournament = fe::findItem(guest, menu, kPgSaveTournament))
    {
        if (guest.dword(kRaceType) == 1 && players <= 1)
        {
            saveTournament.setOnClick(0x44e480);
            showAgain(guest, saveTournament.address());
            if (guest.call(kRaceAgain) == 0)
                saveTournament.setDisabled(true);
        }
        else
        {
            greyAndHide(guest, saveTournament.address());
        }
    }
    if (Item saveKnockout = fe::findItem(guest, menu, kPgSaveKnockout))
    {
        if (guest.dword(kRaceType) == 2 && players <= 1)
        {
            saveKnockout.setOnClick(0x44e460);
            showAgain(guest, saveKnockout.address());
            if (guest.call(kRaceAgain) == 0)
                saveKnockout.setDisabled(true);
        }
        else
        {
            greyAndHide(guest, saveKnockout.address());
        }
    }
    if (Item mainMenu = fe::findItem(guest, menu, kPgMainMenu))
    {
        mainMenu.setOnClick(players > 1 ? 0x44e600 : 0x44e5b0);
        mainMenu.setText(fe::text(guest, players > 1 && guest.word(kNetworkHost) == 0 ? 0x1a9 : 0x105));
    }
    if (Item quit = fe::findItem(guest, menu, kPgQuit))
        quit.setOnClick(0x44e690);
    const x86::reg32 current = guest.call(kCurrentMenu);
    if (Item item = fe::findItem(guest, current, kPgSummary))
        showAgain(guest, item.address());
    const x86::reg32 language = guest.dword(kLanguage);
    if (language == 1 || language == 5)
    {
        const x86::reg32 recordsSheet = guest.dword(kSheetRecords);
        guest.setWord(recordsSheet + 0x42, 0x48);
        guest.setWord(recordsSheet + 0x44, 0x59);
        guest.setWord(recordsSheet + 0x46, 0x2a);
        const x86::reg32 copSheet = guest.dword(kSheetCop);
        guest.setWord(copSheet + 0x3c, 0xa0);
        guest.setWord(copSheet + 0x3e, 0x8c);
    }
    return 0;
}

/* sub_44eda0, postgame on every pass: the network's end told; the sheet shown
 * coming up, the others hiding. */
x86::reg32 postgameOnFrameBody(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kLine = 0x6644c0;
    guest.call(kDescriptionService);
    if (signedDword(guest, kPlayers) > 1 && guest.word(kNetworkLeaving) != 0 && guest.dword(kNetworkConnected) != 0)
    {
        guest.setDword(kLine, fe::text(guest, 0x114));
        openMessage(guest, 1, kLine, kShortMessage);
        guest.setWord(kNetworkLost, 0);
        guest.call(kCloseNetwork);
    }
    if (signedDword(guest, kPlayers) > 1 && guest.dword(kNetworkConnected) == 0)
        if (Item raceNow = fe::findItem(guest, menu, kPgRaceNow))
            raceNow.setDisabled(true);
    for (const x86::reg32 slot : { kSheetSummary, kSheetToggle, kSheetRecords, kSheetTournament, kSheetCop, kSheetPerformance })
        if (guest.dword(slot) == 0)
            return 0;
    const auto hiding = [&](x86::reg32 slot, x86::reg32 state) {
        const x86::reg32 sheet = guest.dword(slot);
        if (guest.word(sheet + state) != kSheetHidden)
            guest.setWord(sheet + state, kSheetHiding);
    };
    hiding(kSheetPerformance, kPerformanceState);
    hiding(kSheetRecords, kRecordsState);
    hiding(kSheetSummary, kSummaryState);
    hiding(kSheetTournament, kTableState);
    hiding(kSheetCop, kTableState);
    for (const x86::reg32 slot : { kSheetRecords, kSheetPerformance, kSheetSummary, kSheetTournament, kSheetCop })
        Item(guest, guest.dword(slot)).setDisabled(true);
    const auto bringUp = [&](x86::reg32 slot, x86::reg32 state) {
        const x86::reg32 sheet = guest.dword(slot);
        guest.setWord(sheet + state, kSheetUp);
        Item(guest, sheet).setDisabled(false);
    };
    switch (guest.dword(kSheetShown))
    {
    case 0:
        bringUp(kSheetPerformance, kPerformanceState);
        break;
    case 1:
        bringUp(kSheetRecords, kRecordsState);
        break;
    case 2:
        bringUp(kSheetSummary, kSummaryState);
        break;
    case 3:
        if (guest.dword(kRaceType) == 1)
            bringUp(kSheetTournament, kTableState);
        else if (guest.dword(kRaceType) == 3)
            bringUp(kSheetCop, kTableState);
        break;
    default:
        break;
    }
    return 0;
}

/* sub_4cef80: whether anybody finished the race, as the developer's flag lets it. */
x86::reg32 finishersBody(Guest& guest)
{
    const x86::reg32 finisher = guest.call(kNthSorted, fe::args({ 0, 0, 0, 0 }, { 0x10, 1 }));
    if (guest.byte(kDeveloperFlags) & kNoFinishers)
        return 0;
    return finisher != 0 ? 1 : 0;
}

/* sub_4cf000: how many are still in. */
x86::reg32 finishersCountBody(Guest& guest)
{
    std::int32_t i = 0;
    while (i < signedDword(guest, kGroupCounts)
           && guest.call(kNthSorted, fe::args({ 0, x86::reg32(i), 0, 0 }, { 0x28, 2 })) != 0)
        ++i;
    return x86::reg32(i);
}

/* sub_4cf040: whether a machine in the network race cannot keep a replay. */
x86::reg32 replayExcludedBody(Guest& guest)
{
    constexpr x86::reg32 kMachineCount = 0x6fd510, kMachineExcluded = 0x6fd594;
    if (signedDword(guest, kPlayers) > 1)
        for (std::int32_t i = 0; i < signedDword(guest, kMachineCount); ++i)
            if (guest.dword(x86::reg32(i) * 0x6c + kMachineExcluded) != 0)
                return 1;
    return 0;
}

/* sub_4cf090: whether a series goes on with another race. */
x86::reg32 raceAgainBody(Guest& guest)
{
    constexpr x86::reg32 kLastKnockoutTrack = 0x6fd40a;     // int16
    constexpr x86::reg32 kLastTournamentTrack = 0x55d020;
    bool again = true;
    const x86::reg32 type = guest.dword(kRaceType);
    if (type == 1 || type == 2)
    {
        const bool lastTrack = type == 2 ? signedWord(guest, kLastKnockoutTrack) == signedDword(guest, kTrack)
                                         : guest.dword(kLastTournamentTrack) == guest.dword(kTrack);
        if (lastTrack)
            again = false;
        if (guest.call(kFinishersCount) == 0 || guest.call(kFinishers) != 0)
            again = false;
    }
    if (signedDword(guest, kPlayers) > 1 && (guest.dword(kNetworkConnected) == 0 || guest.word(kNetworkLeaving) != 0))
        again = false;
    return again ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * loadsave
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kReplayList = 0x5591c0;        // the list's item
constexpr x86::reg32 kDeleteButton = 0x5591c4;
constexpr x86::reg32 kLoadButton = 0x5591c8;
constexpr x86::reg32 kShownKind = 0x5591bc;         // 0 replays, 1 ghosts, 2 tournaments, 3 knockouts
constexpr x86::reg32 kDescription = 0x558d0c;       // ten lines of 0x78
constexpr x86::reg32 kDescriptionBytes = 0x4b0;
constexpr x86::reg32 kDescriptionLine2 = 0x558d84;
constexpr x86::reg32 kDescriptionLine3 = 0x558dfc;
constexpr x86::reg32 kDescriptionLine4 = 0x558e74;
constexpr x86::reg32 kKeptFolder = 0x7a2f34;
constexpr x86::reg32 kDeleteName = 0x6610a0;
constexpr x86::reg32 kListReplays = 0x4475a0;       // eax the list, edx what kind of file
constexpr x86::reg32 kSelectionMoved = 0x4471a0;
constexpr x86::reg32 kReadReplayInfo = 0x446060;
constexpr x86::reg32 kUnlink = 0x4e28a0;
constexpr x86::reg32 kDrawMenuText = 0x452190;
constexpr x86::reg32 kCarName = 0x439250;
constexpr x86::reg32 kCheckCars = 0x478090;
constexpr x86::reg32 kCheckOpponents = 0x478100;
constexpr x86::reg32 kLoadGhostFile = 0x477b50;
constexpr x86::reg32 kSaveConfig = 0x472820;
constexpr x86::reg32 kLoadFailed = 0x446bf0;
constexpr x86::reg32 kLoadKnockout = 0x4465e0;
constexpr x86::reg32 kLoadTournament = 0x446800;
constexpr x86::reg32 kLoadGhost = 0x4466a0;
constexpr x86::reg32 kKnockoutInfo = 0x4461d0;
constexpr x86::reg32 kTournamentInfo = 0x4462e0;
constexpr x86::reg32 kGhostInfo = 0x446420;
constexpr x86::reg32 kDeleteSelected = 0x446ea0;
constexpr x86::reg32 kRecordsHolder = 0x4cef00;

constexpr GuestString kExtReplay{ 0x538c14, "rp3" };
constexpr GuestString kExtTournament{ 0x538c10, "trn" };
constexpr GuestString kExtKnockout{ 0x538c18, "kot" };
constexpr GuestString kExtGhost{ 0x538bec, "gst" };
constexpr GuestString kKeptNameFormat{ 0x538c1c, "%s%s." };
constexpr GuestString kBoxTitleFormat{ 0x538c24, "%s..." };
constexpr GuestString kPlain{ 0x538bc0, "%s" };
constexpr GuestString kLapsFormat{ 0x538bc4, "%d %s" };
constexpr GuestString kTwoFormat{ 0x538bcc, "%s, %s" };
constexpr GuestString kRaceFormat{ 0x538bd4, "%s %d" };
constexpr GuestString kLsBox{ 0x538c2c, "DESCRIPTION_BOX" };
constexpr GuestString kLsReplays{ 0x538c3c, "replays" };
constexpr GuestString kLsDelete{ 0x538c44, "DELETE_FILE" };
constexpr GuestString kLsLoadNow{ 0x538c50, "loadnow" };
constexpr GuestString kLsShowTournaments{ 0x538c58, "SHOW_TOURNAMENTS" };
constexpr GuestString kLsShowKnockouts{ 0x538c6c, "SHOW_KNOCKOUTS" };
constexpr GuestString kLsShowGhosts{ 0x538c7c, "SHOW_GHOSTS" };
constexpr GuestString kLsShowReplays{ 0x538c88, "SHOW_REPLAYS" };

/* The name the list has selected. */
x86::reg32 selectedName(const Guest& guest, x86::reg32 list)
{
    return guest.dword(guest.dword(list + kListEntries) + 8 * guest.dword(list + kListSelected));
}

/* A file's extension after its path. */
void appendExtension(Guest& guest, x86::reg32 path, const GuestString& extension)
{
    guest.copyString(path + guest.length(path), extension.address);
}

/* The screen's kinds of file. */
const GuestString& extensionOf(x86::reg32 kind)
{
    static constexpr GuestString kExtensions[] = { kExtReplay, kExtGhost, kExtTournament, kExtKnockout };
    return kExtensions[kind];
}

/* A kept file's player's car, among its cars. */
x86::reg32 playerCarOf(const Guest& guest, x86::reg32 file)
{
    return guest.dword(file + guest.dword(file + 0x19b4) * 0x6c + 0x1a00);
}

/* sub_4461d0: a kept knockout's description. */
x86::reg32 knockoutInfoBody(Guest& guest, x86::reg32 path)
{
    if (guest.call(kFileExists, fe::args({ path })) == 0)
        return 0;
    const x86::reg32 file = guest.call(kLoadFileQuietly, fe::args({ path, 0x10 }));
    if (file == 0)
        return 0;
    const std::int32_t round = signedWord(guest, file + 0x187c);
    const std::int32_t track = signedWord(guest, file + 2 * x86::reg32(round) + 0x18be);
    if (track <= 8 && track >= 0 && guest.call(kCheckCars, fe::args({ file })) != 0
        && guest.call(kCheckOpponents, fe::args({ file })) != 0)
    {
        const x86::reg32 level = guest.dword(file + 0x1930) == 0 ? 0x190 : 0x191;
        const x86::reg32 car = playerCarOf(guest, file);
        const x86::reg32 kind = fe::text(guest, 0xd7);
        const x86::reg32 levelName = fe::text(guest, level);
        guest.call(kSprintf, fe::args({}, { kDescription, kTwoFormat.address, levelName, kind }));
        const x86::reg32 trackName = fe::text(guest, x86::reg32(track + 0x66));
        guest.call(kSprintf, fe::args({}, { kDescriptionLine2, kPlain.address, trackName }));
        const x86::reg32 carName = guest.call(kCarName, fe::args({ car }));
        guest.call(kSprintf, fe::args({}, { kDescriptionLine3, kPlain.address, carName }));
    }
    return guest.call(kFree, fe::args({ file }));
}

/* sub_4462e0: a kept tournament's description. */
x86::reg32 tournamentInfoBody(Guest& guest, x86::reg32 path)
{
    constexpr x86::reg32 kTournamentTracks = 0x55d004;
    if (guest.call(kFileExists, fe::args({ path })) == 0)
        return 0;
    const x86::reg32 file = guest.call(kLoadFileQuietly, fe::args({ path, 0x10 }));
    if (file == 0)
        return 0;
    const std::int32_t round = signedWord(guest, file + 0x18cc);
    const std::int32_t track = signedDword(guest, 4 * x86::reg32(round) + kTournamentTracks);
    if (track < 8 && track >= 0 && guest.call(kCheckCars, fe::args({ file })) != 0
        && guest.call(kCheckOpponents, fe::args({ file })) != 0)
    {
        const x86::reg32 level = guest.dword(file + 0x1930) == 0 ? 0x190 : 0x191;
        const x86::reg32 car = playerCarOf(guest, file);
        const x86::reg32 kind = fe::text(guest, 0xd6);
        const x86::reg32 levelName = fe::text(guest, level);
        guest.call(kSprintf, fe::args({}, { kDescription, kTwoFormat.address, levelName, kind }));
        const x86::reg32 trackName = fe::text(guest, x86::reg32(track + 0x66));
        guest.call(kSprintf, fe::args({}, { kDescriptionLine2, kPlain.address, trackName }));
        const x86::reg32 carName = guest.call(kCarName, fe::args({ car }));
        guest.call(kSprintf, fe::args({}, { kDescriptionLine3, kPlain.address, carName }));
        const x86::reg32 points = x86::reg32(signedWord(guest, file + 0x190e));
        const x86::reg32 pointsWord = fe::text(guest, 0xf0);
        guest.call(kSprintf, fe::args({}, { kDescriptionLine4, kRaceFormat.address, pointsWord, points }));
    }
    return guest.call(kFree, fe::args({ file }));
}

/* sub_446420: a kept ghost's description. */
x86::reg32 ghostInfoBody(Guest& guest, x86::reg32 path)
{
    if (guest.call(kFileExists, fe::args({ path })) == 0)
        return 0;
    const x86::reg32 file = guest.call(kLoadFileQuietly, fe::args({ path, 0x10 }));
    if (file == 0)
        return 0;
    const std::int32_t track = signedDword(guest, file + 0x196c);
    if (track <= 8 && track >= 0 && guest.call(kCheckCars, fe::args({ file })) != 0
        && guest.call(kCheckOpponents, fe::args({ file })) != 0)
    {
        const x86::reg32 car = playerCarOf(guest, file);
        const x86::reg32 trackName = fe::text(guest, x86::reg32(track + 0x66));
        guest.call(kSprintf, fe::args({}, { kDescription, kPlain.address, trackName }));
        const x86::reg32 lapsWord = fe::text(guest, 5);
        guest.call(kSprintf, fe::args({}, { kDescriptionLine2, kLapsFormat.address, guest.dword(file + 0x1974), lapsWord }));
        const x86::reg32 carName = guest.call(kCarName, fe::args({ car }));
        guest.call(kSprintf, fe::args({}, { kDescriptionLine3, kPlain.address, carName }));
    }
    return guest.call(kFree, fe::args({ file }));
}

/* What a file loaded over the race's setup must not touch: the player's
 * preferences and the two players' controller ids. */
struct KeptSettings
{
    static constexpr x86::reg32 kPreferences = 0x6fbb4c, kFirstPlayer = 0x6fd2dc, kSecondPlayer = 0x6fd348;
    std::array<x86::reg32, 0x5db> preferences;
    std::array<x86::reg32, 3> first, second;

    void keep(const Guest& guest)
    {
        for (x86::reg32 i = 0; i < preferences.size(); ++i)
            preferences[i] = guest.dword(kPreferences + 4 * i);
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            first[i] = guest.dword(kFirstPlayer + 4 * i);
            second[i] = guest.dword(kSecondPlayer + 4 * i);
        }
    }

    void putBack(Guest& guest) const
    {
        for (x86::reg32 i = 0; i < preferences.size(); ++i)
            guest.setDword(kPreferences + 4 * i, preferences[i]);
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            guest.setDword(kFirstPlayer + 4 * i, first[i]);
            guest.setDword(kSecondPlayer + 4 * i, second[i]);
        }
    }
};

/* sub_4465e0 and sub_446800, the same code: a kept tournament or knockout
 * loaded: the whole race's setup taken from it. */
x86::reg32 loadKeptSeriesBody(Guest& guest, x86::reg32 path)
{
    constexpr x86::reg32 kRaceSetup = 0x6fbb40, kRaceSetupBytes = 0x20f0;
    KeptSettings kept;
    kept.keep(guest);
    const x86::reg32 exists = guest.call(kFileExists, fe::args({ path }));
    if (exists == 0)
        return exists;
    const x86::reg32 file = guest.call(kLoadFileQuietly, fe::args({ path, 0x10 }));
    if (file == 0)
        return 0;
    x86::reg32 loaded = 0;
    if (guest.call(kCheckCars, fe::args({ file })) != 0)
    {
        guest.call(kCopy, fe::args({ file, kRaceSetup, kRaceSetupBytes }));
        kept.putBack(guest);
        loaded = 1;
    }
    guest.call(kFree, fe::args({ file }));
    return loaded;
}

/* sub_4466a0: a kept ghost loaded, and the track's record's holder looked up. */
x86::reg32 loadKeptGhostBody(Guest& guest, x86::reg32 path)
{
    constexpr x86::reg32 kTrackHolder = 0x6fd2b4, kTrackHolderToo = 0x6fbb50;
    KeptSettings kept;
    kept.keep(guest);
    guest.call(kSaveConfig);
    const x86::reg32 loaded = guest.call(kLoadGhostFile, fe::args({ path }));
    if (loaded == 0)
        return loaded;
    kept.putBack(guest);
    const x86::reg32 holder = guest.call(kRecordsHolder, fe::args({ x86::reg32(signedWord(guest, kTrack)) }));
    const x86::reg32 widened = x86::reg32(std::int32_t(std::int16_t(holder)));
    guest.setDword(kTrackHolder, widened);
    guest.setDword(kTrackHolderToo, widened);
    return 1;
}

/* sub_446ad0, sub_446b10, sub_446b60 and sub_446ba0: a Show button: the list
 * made of that kind of file. */
x86::reg32 showKind(Guest& guest, const GuestString& extension, x86::reg32 kind)
{
    const x86::reg32 list = guest.dword(kReplayList);
    if (list == 0)
        return 0;
    guest.call(kListReplays, fe::args({ list, extension.address }));
    const x86::reg32 again = guest.dword(kReplayList);
    guest.setDword(kShownKind, kind);
    guest.call(kSelectionMoved, fe::args({ again, guest.dword(again + kListSelected) }));
    return 2;
}

/* sub_446bf0: the file cannot be loaded. */
x86::reg32 loadFailedBody(Guest& guest)
{
    constexpr x86::reg32 kLines = 0x6611a0;
    guest.setDword(kLines, fe::text(guest, 0x41));
    guest.setDword(kLines + 4, fe::text(guest, 0x1c7));
    return openMessage(guest, 2, kLines, kLongMessage);
}

/* sub_446c50: Load Now's click: what the list has selected, loaded.  The race
 * just over (a line 0x143 in the list) has no file: its path empty. */
x86::reg32 loadNowBody(Guest& guest)
{
    const x86::reg32 list = guest.dword(kReplayList);
    if (list == 0)
        return 0;
    const std::int32_t selected = signedDword(guest, list + kListSelected);
    if (selected >= signedDword(guest, list + kListCount))
        return 2;
    if (selected < 0)
        return 0;
    const x86::reg32 path = guest.frame();      // char[0xfc]
    guest.call(kSprintf, fe::args({}, { path, kKeptNameFormat.address, kKeptFolder, selectedName(guest, list) }));
    const x86::reg32 kind = guest.dword(kShownKind);
    if (kind > 3)
        return 2;
    x86::reg32 loader;
    x86::reg32 success = 6;
    if (kind == 0)
    {
        const x86::reg32 keptName = fe::text(guest, 0x143);
        if (guest.call(kStrcmp, fe::args({ selectedName(guest, guest.dword(kReplayList)), keptName })) == 0)
            guest.setByte(path, 0);
        else if (guest.byte(path) != 0)
            appendExtension(guest, path, kExtReplay);
        loader = kKeptReplay;
        success = 7;
    }
    else
    {
        if (guest.byte(path) != 0)
            appendExtension(guest, path, extensionOf(kind));
        loader = kind == 2 ? kLoadTournament : kind == 3 ? kLoadKnockout : kLoadGhost;
    }
    if (guest.call(loader, fe::args({ path })) != 0)
        return success;
    guest.call(kLoadFailed);
    return 2;
}

/* sub_446ea0: the file selected deleted, the list made anew. */
x86::reg32 deleteSelectedBody(Guest& guest)
{
    const x86::reg32 list = guest.dword(kReplayList);
    if (signedDword(guest, list + kListSelected) >= signedDword(guest, list + kListCount))
    {
        guest.setByte(kDeleteName, 0);
        return list & 0xffff00ff;
    }
    guest.call(kSprintf, fe::args({}, { kDeleteName, kKeptNameFormat.address, kKeptFolder, selectedName(guest, list) }));
    const x86::reg32 kind = guest.dword(kShownKind);
    if (kind <= 3)
    {
        const GuestString& extension = extensionOf(kind);
        appendExtension(guest, kDeleteName, extension);
        guest.call(kUnlink, fe::args({ kDeleteName, extension.address }));
        // Its kind of file stays in edx from the call above.
        guest.call(kListReplays, fe::args({ guest.dword(kReplayList) }));
    }
    const x86::reg32 again = guest.dword(kReplayList);
    return guest.call(kSelectionMoved, fe::args({ again, guest.dword(again + kListSelected) }));
}

/* sub_446ff0: "delete?" answered. */
x86::reg32 deleteAnsweredBody(Guest& guest, x86::reg32 answer, x86::reg32 closes)
{
    const x86::reg32 focus = fe::dialogFocus(guest);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
    }
    else if (answer == fe::kDialogEnter && focus <= 1)
    {
        if (focus == 0)
            guest.call(kDeleteSelected);
        guest.setByte(closes, 1);
    }
    return 0;
}

/* sub_447030: Delete's click: asked first, naming the file. */
x86::reg32 deleteClickedBody(Guest& guest)
{
    constexpr x86::reg32 kLines = 0x6611a8, kButtons = 0x6611b0;
    const x86::reg32 list = guest.dword(kReplayList);
    if (signedDword(guest, list + kListSelected) >= signedDword(guest, list + kListCount))
    {
        guest.setByte(kDeleteName, 0);
        return 2;
    }
    guest.copyString(kDeleteName, selectedName(guest, list));
    guest.setDword(kLines, fe::text(guest, 0x144));
    guest.setDword(kLines + 4, kDeleteName);
    guest.setDword(kButtons, fe::text(guest, 0xc4));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
    fe::Question question{};
    question.lineCount = 2;
    question.lines = kLines;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x446ff0;
    fe::openQuestion(guest, question);
    return 2;
}

/* sub_4470e0: the description's box drawn: its title and its lines, those it
 * has.  (Eax left by the last line, read by no caller.) */
x86::reg32 drawDescriptionBody(Guest& guest, x86::reg32 item)
{
    const std::int32_t itemX = signedWord(guest, item + fe::item::kX);
    const std::int32_t itemY = signedWord(guest, item + fe::item::kY);
    const x86::reg32 title = guest.frame();     // char[0x100]
    const x86::reg32 heading = fe::text(guest, 0x1e0);
    guest.call(kSprintf, fe::args({}, { title, kBoxTitleFormat.address, heading }));
    const auto draw = [&](x86::reg32 text, std::int32_t x, std::int32_t y, x86::reg32 colour) {
        guest.call(kDrawMenuText, fe::args({ text, x86::reg32(x), x86::reg32(y), 0x12 }, { 3, colour }));
    };
    draw(title, itemX + 10, itemY, 0xff5abeff);
    x86::reg32 left = 0;
    for (x86::reg32 line = 0; line < 10; ++line)
    {
        const x86::reg32 offset = line * 0x78;
        left = offset;
        if (guest.byte(kDescription + offset) != 0)
        {
            draw(kDescription + offset, itemX + 20, itemY + 0x19 + 0x14 * std::int32_t(line), 0xff649dfd);
            left = guest.eax();
        }
    }
    return left;
}

/* sub_4471a0, the list's selection moved: the description of the one selected,
 * a kept file's own read from it.  (Edx left as the list's selection, read by
 * no caller.) */
x86::reg32 selectionMovedBody(Guest& guest)
{
    guest.call(kFillZero, fe::args({ kDescription, kDescriptionBytes }));
    const x86::reg32 list = guest.dword(kReplayList);
    guest.leaveEdx(list);
    if (list == 0)
        return guest.eax();
    const std::int32_t selected = signedDword(guest, list + kListSelected);
    guest.leaveEdx(x86::reg32(selected));
    if (selected >= signedDword(guest, list + kListCount) || selected < 0)
        return list;
    const x86::reg32 path = guest.frame();      // char[0xfc]
    guest.call(kSprintf, fe::args({}, { path, kKeptNameFormat.address, kKeptFolder, selectedName(guest, list) }));
    const x86::reg32 kind = guest.dword(kShownKind);
    if (kind > 3)
        return kind;
    if (kind == 0)
    {
        const x86::reg32 keptName = fe::text(guest, 0x143);
        if (guest.call(kStrcmp, fe::args({ selectedName(guest, guest.dword(kReplayList)), keptName })) == 0)
            guest.setByte(path, 0);
        else if (guest.byte(path) != 0)
            appendExtension(guest, path, kExtReplay);
        return guest.call(kReadReplayInfo, fe::args({ path }));
    }
    if (guest.byte(path) != 0)
        appendExtension(guest, path, extensionOf(kind));
    return guest.call(kind == 2 ? kTournamentInfo : kind == 3 ? kKnockoutInfo : kGhostInfo, fe::args({ path }));
}

/* sub_447470, loadsave as it opens: the replays listed first. */
x86::reg32 loadSaveOnEnterBody(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kReplayList, 0);
    guest.call(kFillZero, fe::args({ kDescription, kDescriptionBytes }));
    if (Item box = fe::findItem(guest, menu, kLsBox))
        guest.setDword(box.address() + kOnDrawBox, 0x4470e0);
    if (Item list = fe::findItem(guest, menu, kLsReplays))
    {
        guest.setDword(kReplayList, list.address());
        guest.setDword(list.address() + kListOnMove, kSelectionMoved);
    }
    Item erase = fe::findItem(guest, menu, kLsDelete);
    guest.setDword(kDeleteButton, erase.address());
    if (erase)
        erase.setOnClick(0x447030);
    Item load = fe::findItem(guest, menu, kLsLoadNow);
    guest.setDword(kLoadButton, load.address());
    if (load)
        load.setOnClick(0x446c50);
    if (Item show = fe::findItem(guest, menu, kLsShowTournaments))
        show.setOnClick(0x446ad0);
    if (Item show = fe::findItem(guest, menu, kLsShowKnockouts))
        show.setOnClick(0x446b10);
    if (Item show = fe::findItem(guest, menu, kLsShowGhosts))
        show.setOnClick(0x446ba0);
    if (Item show = fe::findItem(guest, menu, kLsShowReplays))
        show.setOnClick(0x446b60);
    if (const x86::reg32 list = guest.dword(kReplayList))
        guest.call(kListReplays, fe::args({ list, kExtReplay.address }));
    const x86::reg32 list = guest.dword(kReplayList);
    guest.setDword(kShownKind, 0);
    guest.call(kSelectionMoved, fe::args({ list, guest.dword(list + kListSelected) }));
    return 0;
}

/* sub_447370, loadsave on every pass: the box and the buttons as the list has
 * it, and the list's title. */
x86::reg32 loadSaveOnFrameBody(Guest& guest, x86::reg32 menu)
{
    if (Item box = fe::findItem(guest, menu, kLsBox))
        box.setHidden(guest.byte(kDescription) == 0);
    const auto nothingSelected = [&](x86::reg32 list) {
        const std::int32_t count = signedDword(guest, list + kListCount);
        return count == 0 || count <= signedDword(guest, list + kListSelected);
    };
    if (const x86::reg32 erase = guest.dword(kDeleteButton))
    {
        Item(guest, erase).setDisabled(false);
        bool close = nothingSelected(guest.dword(kReplayList));
        if (!close && guest.dword(kShownKind) == 0)
        {
            // The race just over has a line in the list, and no file.
            const x86::reg32 keptName = fe::text(guest, 0x143);
            const x86::reg32 name = selectedName(guest, guest.dword(kReplayList));
            close = guest.call(kStrcmp, fe::args({ name, keptName })) == 0;
        }
        if (close)
            Item(guest, guest.dword(kDeleteButton)).setDisabled(true);
    }
    if (const x86::reg32 load = guest.dword(kLoadButton))
        Item(guest, load).setDisabled(nothingSelected(guest.dword(kReplayList)));
    static constexpr std::uint32_t kTitles[] = { 0x13f, 0x140, 0x141, 0x142 };
    const x86::reg32 kind = guest.dword(kShownKind);
    if (kind <= 3)
    {
        const x86::reg32 title = fe::text(guest, kTitles[kind]);
        guest.setDword(guest.dword(kReplayList) + fe::item::kText, title);
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * credits
 * ------------------------------------------------------------------------- */

// The pages: 16 bytes each, a kind (0 and 1 a person's page with the picture
// and its signature, 2 and 3 lines of text, 4 a picture), its first line,
// its last line and the picture's name.
constexpr x86::reg32 kPages = 0x555758;
constexpr x86::reg32 kPageFirstLine = 0x55575c;
constexpr x86::reg32 kPageLastLine = 0x555760;
constexpr x86::reg32 kPagePicture = 0x555764;
constexpr x86::reg32 kCreditsArt = 0x555754;        // the pictures' file
constexpr x86::reg32 kCreditsPage = 0x555c28;       // the page asked for
constexpr x86::reg32 kCreditsX = 0x555c2c;
constexpr x86::reg32 kCreditsSpeed = 0x555c30;
constexpr x86::reg32 kCreditsShown = 0x555c34;      // the page drawn
constexpr x86::reg32 kCreditsCanvas = 0x555c38;
constexpr x86::reg32 kCreditsFont = 0x555c3c;
constexpr x86::reg32 kCreditsItalics = 0x555c40;
constexpr x86::reg32 kCreditsHeld = 0x555c44;       // passes the button has been down
constexpr x86::reg32 kCreditsState = 0x5f40c0;
// The states: sliding in, from the right or the left; sliding out, to the left
// or the right; still.
constexpr std::uint32_t kInFromRight = 0, kInFromLeft = 1, kOutToLeft = 2, kOutToRight = 3, kStill = 4;
constexpr std::int32_t kSlideStart = 0x280;
constexpr std::int32_t kSlideFar = 0x3c0;
constexpr std::int32_t kSpeedStart = 0x3f;
constexpr std::int32_t kSpeedMost = 0x3c;
constexpr x86::reg32 kBonusCarLine = 0x46a;         // a page only for those with the bonus car

constexpr x86::reg32 kRenderPage = 0x441cb0;
constexpr x86::reg32 kNextPage = 0x442370;
constexpr x86::reg32 kPreviousPage = 0x4423e0;
constexpr x86::reg32 kHaveBonusCar = 0x439ce0;
constexpr x86::reg32 kMakeCarId = 0x438ad0;
constexpr x86::reg32 kFindCarById = 0x439990;
constexpr x86::reg32 kLoadShapes = 0x44ac90;
constexpr x86::reg32 kReadMouse = 0x435310;
// The drawing library: a canvas opened, what is drawn goes to it.
constexpr x86::reg32 kOpenCanvas = 0x4ef190;        // eax width, edx height, ebx depth, ecx 0
constexpr x86::reg32 kBeginDrawing = 0x4eada0;
constexpr x86::reg32 kClearCanvas = 0x4ef1b0;
constexpr x86::reg32 kEndDrawing = 0x4ead90;
constexpr x86::reg32 kFindShape = 0x4ef1e0;
constexpr x86::reg32 kLocateShape = 0x4ef270;
constexpr x86::reg32 kDrawShape = 0x4ef3f0;         // eax the shape, edx x, ebx y
constexpr x86::reg32 kSetFont = 0x4ef520;
constexpr x86::reg32 kPackColour = 0x4ef760;
constexpr x86::reg32 kSetColours = 0x4ef830;        // eax the text's, edx its background's
constexpr x86::reg32 kDrawText = 0x4efe30;          // eax the text, edx x, ebx y
constexpr x86::reg32 kTextWidth = 0x4efef0;
constexpr x86::reg32 kCanvasTexture = 0x4d6390;     // eax the canvas's pixels, edx the texture to reuse
constexpr x86::reg32 kCloseCanvas = 0x4eff00;
constexpr std::int32_t kCanvasWidth = 0x260, kCanvasHeight = 0x140;
constexpr x86::reg32 kColourWhite = 0xffffff, kColourHeading = 0xffe440, kColourName = 0xffdf40;
constexpr x86::reg32 kDisplayDepth = 0x565010;      // byte

constexpr GuestString kCrSttl{ 0x537b90, "sttl" };
constexpr GuestString kCrVanc{ 0x537b98, "vanc" };
constexpr GuestString kCrDummy{ 0x537ba0, "dumm" };
constexpr GuestString kCrSignature{ 0x537ba8, "sig%d" };
constexpr GuestString kCrArtFormat{ 0x537bb0, "%scred.qfs" };
constexpr GuestString kCrNext{ 0x537bbc, "next" };
constexpr GuestString kCrPrevious{ 0x537bc4, "previous" };
constexpr GuestString kCrMore{ 0x537bd0, "More" };
constexpr GuestString kCrMoreFormat{ 0x537bd8, "%sIN_%d.fsh" };
constexpr GuestString kCrRb{ 0x537be4, "rb" };
constexpr GuestString kCrFont{ 0x537be8, "%scred.ffn" };
constexpr GuestString kCrItalics{ 0x537bf4, "%scredi.ffn" };
constexpr GuestString kCrItalics10{ 0x537c00, "%scred10i.ffn" };

/* sub_439ce0: whether the bonus car is installed. */
x86::reg32 haveBonusCarBody(Guest& guest)
{
    constexpr x86::reg32 kBonusCar = 0x25;
    const x86::reg32 id = guest.frame();
    guest.call(kMakeCarId, fe::args({ id, kBonusCar }));
    return guest.call(kFindCarById, fe::args({ id })) == x86::reg32(-1) ? 0 : 1;
}

void setColours(Guest& guest, x86::reg32 background, x86::reg32 colour)
{
    const x86::reg32 packedBackground = guest.call(kPackColour, fe::args({ background }));
    const x86::reg32 packed = guest.call(kPackColour, fe::args({ colour }));
    guest.call(kSetColours, fe::args({ packed, packedBackground }));
}

void drawText(Guest& guest, x86::reg32 text, std::int32_t x, std::int32_t y)
{
    guest.call(kDrawText, fe::args({ text, x86::reg32(x), x86::reg32(y) }));
}

/* A line with its shadow, a pixel off. */
void drawShadowed(Guest& guest, x86::reg32 text, std::int32_t x, std::int32_t y, x86::reg32 colour)
{
    setColours(guest, 0, 0);
    drawText(guest, text, x + 1, y + 1);
    setColours(guest, 0, colour);
    drawText(guest, text, x, y);
}

/* A shape in the canvas's middle. */
void drawCentred(Guest& guest, x86::reg32 shape)
{
    const std::int32_t y = (kCanvasHeight - signedWord(guest, shape + 6)) / 2;
    const std::int32_t x = (kCanvasWidth - signedWord(guest, shape + 4)) / 2;
    guest.call(kDrawShape, fe::args({ shape, x86::reg32(x), x86::reg32(y) }));
}

/* sub_441cb0: the page asked for, drawn into a canvas made into a texture,
 * `previous`'s reused: the texture.  A person's page, by the language: the
 * five titles down the side, and beside them the person's name and lines. */
x86::reg32 renderPageBody(Guest& guest, x86::reg32 previous)
{
    constexpr std::uint32_t kPersonTitles = 0x1c8;
    const x86::reg32 canvas = guest.call(kOpenCanvas, fe::args({ kCanvasWidth, kCanvasHeight, 0x10, 0 }));
    guest.call(kBeginDrawing);
    guest.call(kClearCanvas, fe::args({ guest.byte(kDisplayDepth) == 8 ? 0xffu : 0u }));
    const x86::reg32 page = guest.dword(kCreditsPage) << 4;
    const x86::reg32 kind = guest.dword(kPages + page);
    if (kind == 0 || kind == 1)
    {
        if (const x86::reg32 art = guest.dword(kCreditsArt))
        {
            drawCentred(guest, guest.call(kFindShape, fe::args({ art, kind == 1 ? kCrSttl.address : kCrVanc.address })));
            x86::reg32 picture = guest.call(kLocateShape, fe::args({ guest.dword(kCreditsArt), guest.dword(page + kPagePicture) }));
            if (picture == 0)
                picture = guest.call(kLocateShape, fe::args({ guest.dword(kCreditsArt), kCrDummy.address }));
            guest.call(kDrawShape, fe::args({ picture, 0x19c, 0x4c }));
            const x86::reg32 name = guest.frame() + 0x14;   // char[]
            const x86::reg32 number = x86::reg32(signedDword(guest, kCreditsPage) % 5 + 1);
            guest.call(kSprintf, fe::args({}, { name, kCrSignature.address, number }));
            const x86::reg32 signature = guest.call(kLocateShape, fe::args({ guest.dword(kCreditsArt), name }));
            if (signature != 0)
                guest.call(kDrawShape, fe::args({ signature, 0x19c, 0xfa }));
        }
        const x86::reg32 first = guest.dword(page + kPageFirstLine);
        if (guest.dword(kLanguage) != 0)
        {
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsItalics) }));
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const x86::reg32 title = fe::text(guest, kPersonTitles + i);
                drawShadowed(guest, title, 0x32, i * 30 + 0x7d, kColourHeading);
            }
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsFont) }));
            setColours(guest, 0, 0);
            drawText(guest, fe::text(guest, first), 0x33, 0x65);
            setColours(guest, 0, kColourName);
            drawText(guest, fe::text(guest, first), 0x32, 0x64);
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const x86::reg32 line = first + 1 + x86::reg32(i);
                setColours(guest, 0, 0);
                drawText(guest, fe::text(guest, line), 0x51, i * 30 + 0x8b);
                setColours(guest, 0, kColourWhite);
                drawText(guest, fe::text(guest, line), 0x50, i * 30 + 0x8a);
            }
        }
        else
        {
            std::array<x86::reg32, 5> widths;
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsItalics) }));
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const x86::reg32 title = fe::text(guest, kPersonTitles + i);
                drawShadowed(guest, title, 0x32, i * 25 + 0x8c, kColourWhite);
                widths[i] = guest.call(kTextWidth, fe::args({ title }));
            }
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsFont) }));
            setColours(guest, 0, 0);
            drawText(guest, fe::text(guest, first), 0x33, 0x74);
            setColours(guest, 0, kColourName);
            drawText(guest, fe::text(guest, first), 0x32, 0x73);
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const std::int32_t x = std::int32_t(widths[i]) + 0x3c;
                const x86::reg32 line = first + 1 + x86::reg32(i);
                setColours(guest, 0, 0);
                drawText(guest, fe::text(guest, line), x + 1, i * 25 + 0x8d);
                setColours(guest, 0, kColourWhite);
                drawText(guest, fe::text(guest, line), x, i * 25 + 0x8c);
            }
        }
    }
    else if (kind == 2)
    {
        // Lines down a column, marked in their first characters: '$' starts the
        // second column, '#' and '@' a heading, '#' with a line left after it.
        std::int32_t x = 0, y = 0x14;
        for (std::int32_t line = signedDword(guest, page + kPageFirstLine);
             line <= signedDword(guest, (guest.dword(kCreditsPage) << 4) + kPageLastLine); ++line)
        {
            x86::reg32 text = fe::text(guest, x86::reg32(line));
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsFont) }));
            x86::reg32 colour = kColourWhite;
            std::int32_t indent = 0x1e;
            bool blankAfter = false;
            for (;; ++text)
            {
                const std::uint8_t mark = guest.byte(text);
                if (mark == '$')
                {
                    x = 0x130;
                    y = 0x14;
                }
                else if (mark == '#')
                {
                    blankAfter = true;
                    colour = kColourHeading;
                    indent = 0;
                }
                else if (mark == '@')
                {
                    colour = kColourHeading;
                    indent = 0;
                }
                else
                {
                    break;
                }
            }
            const std::int32_t at = x + indent;
            setColours(guest, 0, 0);
            drawText(guest, text, at, y);
            setColours(guest, 0, colour);
            drawText(guest, text, at + 1, y + 1);
            y += blankAfter ? 0x28 : 0x14;
        }
    }
    else if (kind == 3)
    {
        // Lines in the middle.
        std::int32_t y = 0x14;
        for (std::int32_t line = signedDword(guest, page + kPageFirstLine);
             line <= signedDword(guest, (guest.dword(kCreditsPage) << 4) + kPageLastLine); ++line)
        {
            const x86::reg32 text = fe::text(guest, x86::reg32(line));
            guest.call(kSetFont, fe::args({ guest.dword(kCreditsFont) }));
            const std::int32_t x = 0x130 - std::int32_t(guest.call(kTextWidth, fe::args({ text }))) / 2;
            setColours(guest, 0, 0);
            drawText(guest, text, x, y);
            setColours(guest, 0, kColourWhite);
            drawText(guest, text, x + 1, y + 1);
            y += 0x14;
        }
    }
    else if (kind == 4)
    {
        if (const x86::reg32 art = guest.dword(kCreditsArt))
        {
            drawCentred(guest, guest.call(kFindShape, fe::args({ art, kCrSttl.address })));
            const x86::reg32 picture = guest.dword((guest.dword(kCreditsPage) << 4) + kPagePicture);
            drawCentred(guest, guest.call(kFindShape, fe::args({ guest.dword(kCreditsArt), picture })));
        }
    }
    guest.call(kEndDrawing);
    const x86::reg32 texture = guest.call(kCanvasTexture, fe::args({ guest.dword(canvas + 0x20), previous }));
    guest.call(kCloseCanvas, fe::args({ canvas, texture }));
    return texture;
}

/* sub_442370, next's click: on to the next page, once it is still; round to
 * the first after the last; past the bonus car's without the car. */
x86::reg32 nextPageBody(Guest& guest)
{
    if (guest.dword(kCreditsState) != kStill)
        return 0;
    const x86::reg32 page = guest.dword(kCreditsPage) + 1;
    const x86::reg32 line = guest.dword((page << 4) + kPageFirstLine);
    guest.setDword(kCreditsPage, page);
    if (line == 0)
        guest.setDword(kCreditsPage, 0);
    if (guest.call(kHaveBonusCar) == 0 && guest.dword((guest.dword(kCreditsPage) << 4) + kPageFirstLine) == kBonusCarLine)
        guest.setDword(kCreditsPage, guest.dword(kCreditsPage) + 1);
    guest.setDword(kCreditsState, kOutToLeft);
    return 0;
}

/* sub_4423e0, previous's click: back a page, round to the last from the first. */
x86::reg32 previousPageBody(Guest& guest)
{
    x86::reg32 page = guest.dword(kCreditsPage);
    if (guest.dword(kCreditsState) == kStill)
    {
        if (page != 0)
        {
            --page;
            guest.setDword(kCreditsPage, page);
            const x86::reg32 have = guest.call(kHaveBonusCar);
            page = guest.dword(kCreditsPage);
            if (have == 0 && guest.dword((page << 4) + kPageFirstLine) == kBonusCarLine)
                --page;
        }
        else
        {
            while (guest.dword((page << 4) + kPageFirstLine) != 0)
                ++page;
            --page;
        }
        guest.setDword(kCreditsState, kOutToRight);
    }
    guest.setDword(kCreditsPage, page);
    return 0;
}

/* sub_442850, credits as it opens: the slide's start, the fonts. */
x86::reg32 creditsOnEnterBody(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kMoviesFolder = 0x7a3234, kFontsFolder = 0x7a2e34;
    guest.setDword(kCreditsShown, 0);
    guest.setDword(kCreditsPage, 0);
    guest.setDword(kCreditsState, kInFromRight);
    guest.setDword(kCreditsSpeed, kSpeedStart);
    guest.setDword(kCreditsArt, 0);
    guest.setDword(kCreditsX, kSlideStart);
    const x86::reg32 name = guest.frame();      // char[0x100]
    if (Item more = fe::findItem(guest, menu, kCrMore))
    {
        guest.call(kSprintf, fe::args({}, { name, kCrMoreFormat.address, kMoviesFolder, 0 }));
        const x86::reg32 file = guest.call(kFopen, fe::args({ name, kCrRb.address }));
        if (file != 0)
            guest.call(kFclose, fe::args({ file }));
        else
            more.setDisabled(true);
    }
    if (Item next = fe::findItem(guest, menu, kCrNext))
        next.setOnClick(kNextPage);
    if (Item previous = fe::findItem(guest, menu, kCrPrevious))
        previous.setOnClick(kPreviousPage);
    guest.call(kSprintf, fe::args({}, { name, kCrFont.address, kFontsFolder }));
    guest.setDword(kCreditsFont, guest.call(kLoadFile, fe::args({ name, 0x10 })));
    guest.call(kSprintf, fe::args({}, { name, guest.dword(kLanguage) == 0 ? kCrItalics.address : kCrItalics10.address, kFontsFolder }));
    guest.setDword(kCreditsItalics, guest.call(kLoadFile, fe::args({ name, 0x10 })));
    return 0;
}

/* sub_442980, credits as it closes. */
x86::reg32 creditsOnExitBody(Guest& guest)
{
    if (const x86::reg32 art = guest.dword(kCreditsArt))
    {
        guest.call(kFree, fe::args({ art }));
        guest.setDword(kCreditsArt, 0);
    }
    guest.call(kFree, fe::args({ guest.dword(kCreditsItalics) }));
    guest.call(kFree, fe::args({ guest.dword(kCreditsFont) }));
    return 0;
}

/* sub_442580, credits on every pass: the page's slide.  A button held down turns
 * pages by itself and doubles the slide. */
x86::reg32 creditsOnFrameBody(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kArtFolder = 0x7a2db4;
    constexpr std::int32_t kAccelerate = 3;
    const x86::reg32 frame = guest.frame();
    const x86::reg32 name = frame;              // char[0x100]
    const x86::reg32 mouseDown = frame + 0x100, mouseX = frame + 0x104, mouseY = frame + 0x108;
    if (guest.dword(kCreditsArt) == 0)
    {
        guest.call(kSprintf, fe::args({}, { name, kCrArtFormat.address, kArtFolder }));
        guest.setDword(kCreditsArt, guest.call(kLoadShapes, fe::args({ name })));
        guest.setDword(kCreditsCanvas, guest.call(kRenderPage, fe::args({ 0 })));
    }
    guest.call(kReadMouse, fe::args({ mouseDown, mouseX, mouseY, 0 }));
    bool held = false;
    x86::reg32 item = 0;
    if (guest.word(menu + fe::menu::kSelected) != 0x64)
    {
        const std::int32_t selected = signedWord(guest, menu + fe::menu::kSelected);
        const std::int32_t first = signedWord(guest, menu + fe::menu::kFirstItem);
        item = fe::item::kTable + fe::item::kSize * x86::reg32(selected + first);
        if (guest.dword(item) == 5
            && (guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kCrNext.address })) != 0
                || guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kCrPrevious.address })) != 0)
            && guest.dword(mouseDown) != 0)
            held = true;
    }
    if (held)
    {
        const x86::reg32 passes = guest.dword(kCreditsHeld) + 1;
        guest.setDword(kCreditsHeld, passes);
        if (std::int32_t(passes) > 10)
        {
            if (guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kCrNext.address })) != 0)
                guest.call(kNextPage, fe::args({ 0 }));
            else
                guest.call(kPreviousPage);
        }
    }
    else
    {
        guest.setDword(kCreditsHeld, 0);
    }
    std::int32_t x = signedDword(guest, kCreditsX);
    std::int32_t speed = signedDword(guest, kCreditsSpeed);
    const auto turnPage = [&] {
        const x86::reg32 page = guest.dword(kCreditsPage);
        if (page != guest.dword(kCreditsShown))
        {
            const x86::reg32 canvas = guest.dword(kCreditsCanvas);
            guest.setDword(kCreditsShown, page);
            guest.setDword(kCreditsCanvas, guest.call(kRenderPage, fe::args({ canvas })));
        }
    };
    const auto doubled = [&] { return guest.dword(kCreditsHeld) != 0; };
    switch (guest.dword(kCreditsState))
    {
    case kOutToLeft:
        if (x <= -kSlideFar)
        {
            turnPage();
            x = kSlideStart;
            speed = kSpeedStart;
            guest.setDword(kCreditsState, kInFromRight);
        }
        else
        {
            if (speed == 0 && signedDword(guest, kCreditsHeld) > 10)
                speed = 10 + kAccelerate;
            else if (speed < kSpeedMost)
                speed += kAccelerate;
            x -= doubled() ? 2 * speed : speed;
        }
        break;
    case kOutToRight:
        if (x >= kSlideFar)
        {
            turnPage();
            x = -kSlideStart;
            speed = kSpeedStart;
            guest.setDword(kCreditsState, kInFromLeft);
        }
        else
        {
            if (speed == 0 && guest.dword(kCreditsHeld) != 0)
                speed = 10 + kAccelerate;
            else if (speed < kSpeedMost)
                speed += kAccelerate;
            x += doubled() ? 2 * speed : speed;
        }
        break;
    case kInFromRight:
        if (speed > kAccelerate)
            speed -= kAccelerate;
        x -= doubled() ? 2 * speed : speed;
        if (x <= 0)
        {
            guest.setDword(kCreditsState, kStill);
            x = 0;
            speed = 0;
        }
        break;
    case kInFromLeft:
        if (speed > kAccelerate)
            speed -= kAccelerate;
        x += doubled() ? 2 * speed : speed;
        if (x >= 0)
        {
            guest.setDword(kCreditsState, kStill);
            x = 0;
            speed = 0;
        }
        break;
    default:
        break;
    }
    guest.setDword(kCreditsX, x86::reg32(x));
    guest.setDword(kCreditsSpeed, x86::reg32(speed));
    return 0;
}

/* ---------------------------------------------------------------------------
 * credits1, the showcase's
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kGlamPage = 0x664794;
constexpr x86::reg32 kGlamPageShown = 0x664798;
constexpr x86::reg32 kGlamPageKept = 0x664780;
constexpr x86::reg32 kGlamTexts = 0x5563dc;         // int16: a page's title's line
constexpr x86::reg32 kGlamTitle = 0x664700;
constexpr x86::reg32 kGlamNextTitle = 0x664680;
constexpr x86::reg32 kGlamShowPage = 0x454d60;
constexpr GuestString kGlamSlider{ 0x5398d4, "ZoomSlider" };
constexpr GuestString kGlamNext{ 0x5398f4, "GlamNext" };
constexpr GuestString kGlamSub{ 0x5398e0, "GlamSub" };
constexpr GuestString kGlamPlain{ 0x5398e8, "%s" };
constexpr GuestString kGlamNextFormat{ 0x5398ec, "%s..." };
constexpr x86::reg32 kSliderSetOnMove = 0x46cac0;
constexpr x86::reg32 kSliderSetValue = 0x46c950;

std::uint32_t glamText(const Guest& guest, x86::reg32 page)
{
    return x86::reg32(signedWord(guest, kGlamTexts + 2 * page));
}

/* sub_454d60: the page's title, and on the button the next's, with "..." but in
 * the second language.  (Eax left as each way leaves it, read by no caller.) */
x86::reg32 glamShowPageBody(Guest& guest, x86::reg32 nextButton)
{
    Item sub = fe::findItem(guest, guest.call(kCurrentMenu), kGlamSub);
    const x86::reg32 title = fe::text(guest, glamText(guest, guest.dword(kGlamPage)));
    guest.call(kSprintf, fe::args({}, { kGlamTitle, kGlamPlain.address, title }));
    sub.setText(kGlamTitle);
    const x86::reg32 language = guest.dword(kLanguage);
    const x86::reg32 next = x86::reg32((signedDword(guest, kGlamPage) + 1) % 4);
    const x86::reg32 nextTitle = fe::text(guest, glamText(guest, next));
    x86::reg32 left;
    if (language == 1)
    {
        guest.copyString(kGlamNextTitle, nextTitle);
        left = nextTitle & 0xffffff00;
    }
    else
    {
        guest.call(kSprintf, fe::args({}, { kGlamNextTitle, kGlamNextFormat.address, nextTitle }));
        left = guest.eax();
    }
    guest.setDword(nextButton + fe::item::kText, kGlamNextTitle);
    return left;
}

/* sub_454e40: next's click. */
x86::reg32 glamNextBody(Guest& guest, x86::reg32 item)
{
    guest.setDword(kGlamPage, x86::reg32((signedDword(guest, kGlamPage) + 1) % 4));
    guest.call(kGlamShowPage, fe::args({ item }));
    return 0;
}

/* sub_454ea0, credits1 as it opens: from the first page, the one it was on kept. */
x86::reg32 glamOnEnterBody(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kGlamPageKept, guest.dword(kGlamPage));
    guest.setDword(kGlamPage, 0);
    guest.setDword(kGlamPageShown, 0);
    const x86::reg32 slider = fe::findItem(guest, menu, kGlamSlider).address();
    guest.call(kSliderSetOnMove, fe::args({ slider, 0x454cf0 }));
    guest.call(kSliderSetValue, fe::args({ slider }, { 0, 0 }));
    const x86::reg32 next = fe::findItem(guest, menu, kGlamNext).address();
    guest.setDword(next + fe::item::kOnClick, 0x454e40);
    guest.call(kGlamShowPage, fe::args({ next }));
    return 0;
}

/* sub_454e80, credits1 as it closes: the page it was on. */
x86::reg32 glamOnExitBody(Guest& guest)
{
    const x86::reg32 kept = guest.dword(kGlamPageKept);
    guest.setDword(kGlamPage, kept);
    guest.setDword(kGlamPageShown, kept);
    return 0;
}

/* sub_44e460, sub_44e480, sub_44e4a0 and sub_44e6a0: a Save button's click: the
 * dialog that asks for the name. */
void saveClicked(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, x86::reg32 what)
{
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        guest.call(kSaveDialog, fe::args({ what }));
        return x86::reg32(2);
    });
}

const GuestString kLevelStrings[] = {
    kCopAssists[0], kCopAssists[1], kDuelDisabled[0], kDuelDisabled[1], kDuelDisabled[2], kDuelDisabled[3],
    kDuelDisabled[4], kDuelDisabled[5], kDuelDisabled[6], kDuelDisabled[7], kDuelDisabled[8], kDuelDisabled[9],
};
const GuestString kCameraEnterStrings[] = { kCameraEscapeItem, kCameraAssists };
const GuestString kCameraExitStrings[] = { kCameraCancel };
const GuestString kSetupStrings[] = { kSetupOnePlayer, kSetupConnect, kSetupTwoPlayers, kSetupWebSite, kSetupLoad };
const GuestString kSetupNameStrings[] = { kSetupName };
const GuestString kRecordsStrings[] = { kRecordsDeleteItem, kRecordsSheet };
const GuestString kPostgameStrings[] = {
    kPgRaceNow, kPgSpeech, kPgTrackAndLaps, kPgTrackFormat, kPgRecords, kPgSummary, kPgTournament, kPgCop, kPgToggle,
    kPgPerformance, kPgViewReplay, kPgSaveReplay, kPgSaveGhost, kPgSaveTournament, kPgSaveKnockout, kPgMainMenu, kPgQuit,
};
const GuestString kPostgameFrameStrings[] = { kPgRaceNow };
const GuestString kKeptNameStrings[] = { kKeptNameFormat, kExtReplay, kExtTournament, kExtKnockout, kExtGhost };
const GuestString kInfoStrings[] = { kPlain, kLapsFormat, kTwoFormat, kRaceFormat };
const GuestString kLoadSaveStrings[] = {
    kLsBox, kLsReplays, kLsDelete, kLsLoadNow, kLsShowTournaments, kLsShowKnockouts, kLsShowGhosts, kLsShowReplays, kExtReplay,
};
const GuestString kLoadSaveFrameStrings[] = { kLsBox };
const GuestString kDrawDescriptionStrings[] = { kBoxTitleFormat };
const GuestString kRenderPageStrings[] = { kCrSttl, kCrVanc, kCrDummy, kCrSignature };
const GuestString kCreditsEnterStrings[] = { kCrNext, kCrPrevious, kCrMore, kCrMoreFormat, kCrRb, kCrFont, kCrItalics, kCrItalics10 };
const GuestString kCreditsFrameStrings[] = { kCrArtFormat, kCrNext, kCrPrevious };
const GuestString kGlamShowStrings[] = { kGlamSub, kGlamPlain, kGlamNextFormat };
const GuestString kGlamEnterStrings[] = { kGlamSlider, kGlamNext };

constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEdx = fe::kRestoreEbx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdxEsi = kEbxEcxEdx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEsiEdi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;

}

/* The screens' tables and the items' handlers enter these as the generated code
 * does: a menu in eax, an answer in eax with the byte that closes the dialog in
 * edx, an item in eax. */

/* sub_45dab0, racetype as it closes: the item selected is the race's type, of six. */
void frontend::raceTypeOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45dab0", 0, 4, nullptr, 0, 0 };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        if (std::int16_t(guest.word(menu + fe::menu::kSelected)) < 6)
            guest.setDword(kRaceType, x86::reg32(std::int32_t(guest.dword(menu + 0x18)) >> 16));
        return x86::reg32(0);
    });
}

void frontend::levelOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d630", 0, 12, kLevelStrings, std::size(kLevelStrings), kEcxEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return levelOnEnterBody(guest, menu); });
}

void frontend::setupOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452f80", 0, 24, kSetupStrings, std::size(kSetupStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setupOnEnterBody(guest, menu); });
}

void frontend::setupOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452f60", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return setupOnFrameBody(guest); });
}

void frontend::setupRaceAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452ac0", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setupRaceAnsweredBody(guest, answer, closes); });
}

void frontend::setupAskRace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452bd0", 0, 28, nullptr, 0 };
    const x86::reg32 canCancel = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setupAskRaceBody(guest, canCancel); });
}

void frontend::setupTwoPlayersClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452d40", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return setupPlayersClicked(guest, 1); });
}

void frontend::setupOnePlayerClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452f20", 0, 12, nullptr, 0, kEbxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return setupPlayersClicked(guest, 0); });
}

void frontend::setupWebSiteRefused(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452d80", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setupWebSiteRefusedBody(guest, answer, closes); });
}

void frontend::setupIsCurrent(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452db0", 0, 8, kSetupNameStrings, std::size(kSetupNameStrings), fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return setupIsCurrentBody(guest); });
}

void frontend::setupWebSiteAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452de0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setupWebSiteAnsweredBody(guest, answer, closes); });
}

void frontend::setupWebSiteClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_452e60", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return setupWebSiteClickedBody(guest); });
}

void frontend::resultsParticipantById(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4554e0", 0, 24, nullptr, 0, kEbxEcxEsiEdi };
    const x86::reg32 group = cpu.eax, id = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantByIdBody(guest, group, id); });
}

void frontend::resultsParticipantRanks(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455530", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 found = cpu.eax, a = cpu.edx, b = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantRanksBody(guest, found, a, b); });
}

void frontend::resultsParticipantMatches(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4557b0", 0, 4, nullptr, 0, 0 };
    const x86::reg32 found = cpu.eax, mask = cpu.edx, state = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantMatchesBody(guest, found, mask, state); });
}

void frontend::resultsCompare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455840", 0, 20, nullptr, 0, kEbxEcxEsiEdi };
    const x86::reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantCompareBody(guest, a, b); });
}

void frontend::resultsSort(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455c10", 0, 16 + 0x40, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const x86::reg32 group = cpu.eax, key = cpu.edx, descending = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantSortBody(guest, group, key, descending); });
}

void frontend::resultsNth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455cd0", 0, 12 + 0x14, nullptr, 0, kEsiEdi };
    const x86::reg32 group = cpu.eax, index = cpu.edx, mask = cpu.ebx, state = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return participantNthBody(guest, group, index, mask, state); });
}

void frontend::resultsNthSorted(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_455dd0", 8, 12 + 0x28, nullptr, 0, kEsiEdi };
    const x86::reg32 group = cpu.eax, index = cpu.edx, key = cpu.ebx, descending = cpu.ecx;
    const x86::reg32 mask = app->getMemory<x86::reg32>(cpu.esp + 4), state = app->getMemory<x86::reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return participantNthSortedBody(guest, group, index, key, descending, mask, state);
    });
}

void frontend::recordsLoaded(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_44f5d0: whether the records' file is in memory.
    static fe::Site site{ "sub_44f5d0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return x86::reg32(guest.dword(kRecordsFile) != 0 ? 1 : 0); });
}

void frontend::recordsSelect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44f670", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const x86::reg32 track = cpu.eax, kind = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsSelectBody(guest, track, kind); });
}

void frontend::recordsRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fa90", 0, 12, nullptr, 0, kEsiEdi };
    const x86::reg32 first = cpu.eax, second = cpu.edx, third = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsReadBody(guest, first, second, third); });
}

void frontend::recordsClear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44fbc0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsClearBody(guest); });
}

void frontend::recordsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450f70", 0, 12, kRecordsStrings, std::size(kRecordsStrings), kEcxEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsOnEnterBody(guest, menu); });
}

void frontend::recordsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_450fd0: nothing to undo.
    static fe::Site site{ "sub_450fd0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::recordsOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450f20", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsOnFrameBody(guest); });
}

void frontend::recordsDeleteClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450eb0", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordsDeleteClickedBody(guest); });
}

void frontend::recordsDeleteAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_450e50", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsDeleteAnsweredBody(guest, answer, closes); });
}

void frontend::recordsHolder(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cef00", 0, 12 + 0x100, nullptr, 0, kEbxEdx };
    const x86::reg32 track = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return recordsHolderBody(guest, track); });
}

void frontend::postgameUnlockTrack(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442b60", 0, 12, nullptr, 0, kEbxEdx };
    const x86::reg32 track = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return unlockTrackBody(guest, track); });
}

void frontend::postgameAwardMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e170", 0, 16, nullptr, 0, kEbxEcxEdx };
    const x86::reg32 id = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return awardMessageBody(guest, id); });
}

void frontend::postgameCheckAwards(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e1a0", 0, 24 + 0x14, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return checkAwardsBody(guest); });
}

void frontend::postgameSaveKnockoutClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e460", 0, 4, nullptr, 0, 0 };
    saveClicked(app, cpu, site, 0);
}

void frontend::postgameSaveTournamentClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e480", 0, 4, nullptr, 0, 0 };
    saveClicked(app, cpu, site, 1);
}

void frontend::postgameSaveGhostClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e4a0", 0, 4, nullptr, 0, 0 };
    saveClicked(app, cpu, site, 3);
}

void frontend::postgameSaveReplayClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e6a0", 0, 4, nullptr, 0, 0 };
    saveClicked(app, cpu, site, 2);
}

void frontend::postgameLeaveNetwork(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e4c0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return leaveNetworkBody(guest); });
}

void frontend::postgameLeaveAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e510", 0, 4, nullptr, 0, 0 };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return leaveAnsweredBody(guest, answer, closes); });
}

void frontend::postgameAskLeave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e550", 0, 16, nullptr, 0, kEbxEcxEdx };
    const x86::reg32 lineId = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return askLeaveBody(guest, lineId); });
}

void frontend::postgameMainMenuClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e5b0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return mainMenuClickedBody(guest); });
}

void frontend::postgameNetworkMainMenuClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e600", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return networkMainMenuClickedBody(guest); });
}

void frontend::postgameQuitClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_44e690: the menu loop told to quit.
    static fe::Site site{ "sub_44e690", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0xa); });
}

void frontend::postgameCannotSaveReplay(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e6c0", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return cannotSaveReplayBody(guest); });
}

void frontend::postgameViewReplayClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e710", 0, 20 + 0xb8, nullptr, 0, kEcxEdx | kEsiEdi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return viewReplayClickedBody(guest); });
}

void frontend::postgameToggleSheets(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44e7a0", 0, 12, nullptr, 0, kEbxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return toggleSheetsBody(guest); });
}

void frontend::postgameOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44efd0", 0, 24 + 0x18, kPostgameStrings, std::size(kPostgameStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return postgameOnEnterBody(guest, menu); });
}

void frontend::postgameOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_44f500: the words spoken over the sheets stopped.
    static fe::Site site{ "sub_44f500", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kSpokenStop);
        return x86::reg32(0);
    });
}

void frontend::postgameOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44eda0", 0, 24, kPostgameFrameStrings, std::size(kPostgameFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return postgameOnFrameBody(guest, menu); });
}

void frontend::postgameFinishers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cef80", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return finishersBody(guest); });
}

void frontend::postgameFinishersCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf000", 0, 20, nullptr, 0, kEbxEcxEdxEsi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return finishersCountBody(guest); });
}

void frontend::postgameReplayExcluded(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf040", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return replayExcludedBody(guest); });
}

void frontend::postgameRaceAgain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf090", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return raceAgainBody(guest); });
}

void frontend::loadSaveKnockoutInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4461d0", 0, 20, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return knockoutInfoBody(guest, path); });
}

void frontend::loadSaveTournamentInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4462e0", 0, 20, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return tournamentInfoBody(guest, path); });
}

void frontend::loadSaveGhostInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446420", 0, 20, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return ghostInfoBody(guest, path); });
}

void frontend::loadSaveLoadKnockout(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4465e0", 0, 24 + 0x178c, nullptr, 0 };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadKeptSeriesBody(guest, path); });
}

void frontend::loadSaveLoadTournament(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446800", 0, 24 + 0x178c, nullptr, 0 };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadKeptSeriesBody(guest, path); });
}

void frontend::loadSaveLoadGhost(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4466a0", 0, 20 + 0x1784, nullptr, 0, kEcxEdx | kEsiEdi };
    const x86::reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadKeptGhostBody(guest, path); });
}

void frontend::loadSaveShowTournaments(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446ad0", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return showKind(guest, kExtTournament, 2); });
}

void frontend::loadSaveShowKnockouts(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446b10", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return showKind(guest, kExtKnockout, 3); });
}

void frontend::loadSaveShowReplays(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446b60", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return showKind(guest, kExtReplay, 0); });
}

void frontend::loadSaveShowGhosts(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446ba0", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return showKind(guest, kExtGhost, 1); });
}

void frontend::loadSaveLoadFailed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446bf0", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return loadFailedBody(guest); });
}

void frontend::loadSaveLoadNow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446c50", 0, 24 + 0xfc, kKeptNameStrings, std::size(kKeptNameStrings) };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return loadNowBody(guest); });
}

void frontend::loadSaveDeleteSelected(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446ea0", 0, 24, kKeptNameStrings, std::size(kKeptNameStrings) };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return deleteSelectedBody(guest); });
}

void frontend::loadSaveDeleteAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_446ff0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const x86::reg32 answer = cpu.eax, closes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return deleteAnsweredBody(guest, answer, closes); });
}

void frontend::loadSaveDeleteClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447030", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return deleteClickedBody(guest); });
}

void frontend::loadSaveDrawDescription(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4470e0", 0, 24 + 0x104, kDrawDescriptionStrings, std::size(kDrawDescriptionStrings) };
    const x86::reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawDescriptionBody(guest, item); });
}

void frontend::loadSaveSelectionMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4471a0", 0, 20 + 0xfc, kKeptNameStrings, std::size(kKeptNameStrings), kEbxEcxEsiEdi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return selectionMovedBody(guest); });
}

void frontend::loadSaveOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447470", 0, 16, kLoadSaveStrings, std::size(kLoadSaveStrings), kEbxEcxEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadSaveOnEnterBody(guest, menu); });
}

void frontend::loadSaveOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_447590: nothing to undo.
    static fe::Site site{ "sub_447590", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::loadSaveOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447370", 0, 20, kLoadSaveFrameStrings, std::size(kLoadSaveFrameStrings),
                          kEbxEcxEdx | fe::kRestoreEdi };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadSaveOnFrameBody(guest, menu); });
}

void frontend::creditsHaveBonusCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_439ce0", 0, 8 + 0x10, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return haveBonusCarBody(guest); });
}

void frontend::creditsRenderPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_441cb0", 0, 24 + 0x40, kRenderPageStrings, std::size(kRenderPageStrings) };
    const x86::reg32 previous = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return renderPageBody(guest, previous); });
}

void frontend::creditsNextPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442370", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return nextPageBody(guest); });
}

void frontend::creditsPreviousPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4423e0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return previousPageBody(guest); });
}

void frontend::creditsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442850", 0, 20 + 0x100, kCreditsEnterStrings, std::size(kCreditsEnterStrings),
                          kEbxEcxEdx | fe::kRestoreEdi };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return creditsOnEnterBody(guest, menu); });
}

void frontend::creditsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442980", 0, 12, nullptr, 0, kEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return creditsOnExitBody(guest); });
}

void frontend::creditsOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442580", 0, 24 + 0x114, kCreditsFrameStrings, std::size(kCreditsFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return creditsOnFrameBody(guest, menu); });
}

void frontend::credits1ShowPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454d60", 0, 24, kGlamShowStrings, std::size(kGlamShowStrings) };
    const x86::reg32 nextButton = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return glamShowPageBody(guest, nextButton); });
}

void frontend::credits1NextClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454e40", 0, 20, nullptr, 0, kEbxEcxEdxEsi };
    const x86::reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return glamNextBody(guest, item); });
}

void frontend::credits1OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454ea0", 0, 16, kGlamEnterStrings, std::size(kGlamEnterStrings), kEbxEcxEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return glamOnEnterBody(guest, menu); });
}

void frontend::credits1OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454e80", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return glamOnExitBody(guest); });
}

}
