#include "native_frontend.h"
#include <iterator>

/* The main menu, the screen a race is set up on -- the track, the car, the
 * opponents, the players' names, the driving aids -- before Race Now, in its
 * three forms:
 *
 *   main      MENUS/MAIN.MNU, one player;
 *   mainsplt  MAINSPLT.MNU, two players on one machine (split screen);
 *   mainmult  MAINMULT.MNU, a race over a network.
 *
 * Each is a row of the handler table at 0x555c48: a handler as the screen
 * opens, one as it closes, one on every pass of the menu loop.  Their buttons
 * add a few more: a player's name opens a dialog whose answer comes back to a
 * function of its own, and mainmult's Back asks before leaving a network game.
 *
 * What the screens do is little: they fit their items to the kind of race.
 * Everything else -- drawing, the keys, the lists that pop up, Race Now -- is
 * the menu engine's, driven by the .MNU. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

/* ---------------------------------------------------------------------------
 * What the three share
 * ------------------------------------------------------------------------- */

/* The kind of race, the index of the racetype screen's item (sub_45dab0).
 * Hot Pursuit is 3: a race of it loads the police's speech (sub_40a010).  1
 * and 2 are the two series, which run their own tracks, so the track is not
 * chosen here; which is the tournament and which the knockout is inferred --
 * these screens treat them alike but for gamelevel1's help line. */
enum RaceType : std::uint32_t
{
    kSingleRace = 0,
    kTournament = 1,
    kKnockout = 2,
    kHotPursuit = 3,
};

// The game's state the screens read and set.
constexpr x86::reg32 kRaceType = 0x6fd3b8;
// A single race's six driving aids, a dword each, and Hot Pursuit's one.
constexpr x86::reg32 kSingleRaceAids = 0x6fd474;
constexpr std::uint32_t kSingleRaceAidCount = 6;
constexpr x86::reg32 kHotPursuitAid = 0x6fd48c;
// 1 while two players race on this machine; mainsplt sets it.
constexpr x86::reg32 kSplitScreen = 0x6fd3b0;
// Nonzero (int16) while the game is set up to race others over a network
// (sub_49e220).
constexpr x86::reg32 kNetworkGame = 0x7a22dc;

// The game's functions the screens call.
// Plays the attract-mode movie once the menu has been left alone long
// enough, and says whether it did.
constexpr x86::reg32 kAttractMode = 0x496200;
// Whether a name is one to keep: 1 to 10 characters.
constexpr x86::reg32 kNameValid = 0x4721f0;
// Leaves the network game: the session closed, the network let go.
constexpr x86::reg32 kLeaveNetworkGame = 0x4a65e0;
// Zeroes a run of bytes: eax where, edx how many (Watcom's memset at 0).
constexpr x86::reg32 kFillZero = 0x4e070c;

// Lines of the language's text (sub_4d1850), by what they are on these screens.
constexpr std::uint32_t kOpponentsTextSeries = 0x1eb;      // MAIN_OPPONENT_BUTTON, a tournament or knockout
constexpr std::uint32_t kOpponentsListSeries = 0x1ec;      // MAIN_OPPONENT_CASCADE, likewise
constexpr std::uint32_t kOpponentsText = 0x1ed;            // MAIN_OPPONENT_BUTTON, any other race
constexpr std::uint32_t kOpponentsList = 0x1ee;            // MAIN_OPPONENT_CASCADE, likewise
constexpr std::uint32_t kAidsOnText = 0x196;               // ASSISTS, an aid on
constexpr std::uint32_t kAidsOffText = 0x197;              // ASSISTS, none
constexpr std::uint32_t kLevelHelpTournament = 0x180;      // gamelevel1's help line
constexpr std::uint32_t kLevelHelpKnockout = 0x17f;
constexpr std::uint32_t kNetworkGameEndedText = 0x114;     // the message as a network game is left
// The labels the three screens' shiftme is placed after.
constexpr std::uint32_t kMainShiftLabel = 0x4b8;
constexpr std::uint32_t kSplitShiftLabel = 0x4ce;
constexpr std::uint32_t kMultiShiftLabel = 0x5b1;

// The font the label before shiftme is drawn in, and where its column starts.
constexpr std::uint32_t kLabelFont = 0x24;
constexpr std::int32_t kLabelX = 0x76;

RaceType raceType(const Guest& guest)
{
    return RaceType(guest.dword(kRaceType));
}

// A tournament or a knockout: a series of races on tracks of its own.
bool isSeries(RaceType type)
{
    return type == kTournament || type == kKnockout;
}

// Whether any of a single race's aids is on.
bool singleRaceAidsOn(const Guest& guest)
{
    for (std::uint32_t i = 0; i < kSingleRaceAidCount; ++i)
    {
        if (guest.dword(kSingleRaceAids + 4 * i) != 0)
            return true;
    }
    return false;
}

/* ASSISTS: "on" or "off", greyed out when off. */
void showAids(Guest& guest, Item aids, bool on)
{
    aids.setText(fe::text(guest, on ? kAidsOnText : kAidsOffText));
    aids.setDisabled(!on);
}

/* shiftme is moved to stand after its label: half the label's width right of
 * x = 0x76. */
void placeAfterLabel(Guest& guest, Item shifted, std::uint32_t label)
{
    const std::int32_t width = fe::textWidth(guest, fe::text(guest, label), kLabelFont);
    shifted.setX(std::int16_t(width / 2 + kLabelX));
}

/* A player's name dialog: the buffer it edits, its two lines and two
 * buttons (char* each), the text of its first line, and the function its
 * answer goes to -- the generated one this file stands in for, whose address
 * the dialog keeps and calls. */
struct NameDialog
{
    x86::reg32 name;             // 10 characters and a zero
    x86::reg32 lines;
    x86::reg32 buttons;
    std::uint32_t firstLine;
    x86::reg32 answer;
};
constexpr std::uint32_t kPlayerNameLength = 10;
constexpr std::uint32_t kNameDialogLine2 = 0xf8;
constexpr std::uint32_t kNameDialogOk = 0xf9;
constexpr std::uint32_t kNameDialogCancel = 0xc6;
constexpr NameDialog kPlayer1Name{ 0x6fd2fc, 0x678908, 0x678910, 0xc9, 0x45cdd0 };
constexpr NameDialog kPlayer2Name{ 0x6fd368, 0x678918, 0x678920, 0xca, 0x45ce60 };

// The players' name buttons' handlers, whose addresses the screens hand out.
constexpr x86::reg32 kPlayer1NameButton = 0x45cef0;
constexpr x86::reg32 kPlayer2NameButton = 0x45cf70;

/* sub_45cef0, sub_45cf70: a player's name button, a dialog to type it in. */
x86::reg32 onPlayerName(Guest& guest, const NameDialog& player)
{
    guest.setDword(player.lines, fe::text(guest, player.firstLine));
    guest.setDword(player.lines + 4, fe::text(guest, kNameDialogLine2));
    guest.setDword(player.buttons, fe::text(guest, kNameDialogOk));
    guest.setDword(player.buttons + 4, fe::text(guest, kNameDialogCancel));
    fe::Dialog dialog;
    dialog.lineCount = 2;
    dialog.lines = player.lines;
    dialog.buttonCount = 2;
    dialog.buttons = player.buttons;
    dialog.focus = 0;
    dialog.edit = player.name;
    dialog.editLength = kPlayerNameLength;
    dialog.editMode = 1;
    dialog.callback = player.answer;
    fe::openDialog(guest, dialog);
    return fe::kDialogHandled;
}

/* sub_45cdd0, sub_45ce60: the name dialog answered.  Enter keeps a name of 1
 * to 10 characters, Escape leaves the old one.  The dialog stays up for a
 * name that will not do.
 *
 * From the mouse, Enter with the second button (Cancel) highlighted closes
 * the dialog as Escape does.  From the keys, Enter keeps the name whichever
 * button is highlighted, Cancel too -- as the original does. */
x86::reg32 onPlayerNameAnswer(Guest& guest, const NameDialog& player, std::uint32_t answer, x86::reg32 close)
{
    guest.setByte(close, 0);
    const x86::reg32 typed = fe::dialogEditText(guest);
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus != 0 && guest.dword(fe::kInputFromMouse) != 0)
        {
            if (focus == 1)
            {
                guest.setByte(close, 1);
                return fe::kDialogHandled;
            }
            return 0;
        }
        if (guest.call(kNameValid, fe::args({ typed })) == 0)
            return 0;
        guest.copyString(player.name, typed);
        guest.setByte(close, 1);
        return fe::kDialogHandled;
    }
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(close, 1);
        return fe::kDialogHandled;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * main and mainsplt
 * ------------------------------------------------------------------------- */

// Their items, by the codelinks MAIN.MNU and MAINSPLT.MNU give them (one copy
// of the strings for both).
constexpr GuestString kOpponentButton{ 0x53a674, "MAIN_OPPONENT_BUTTON" };
constexpr GuestString kOpponentCars{ 0x53a68c, "opcars" };
constexpr GuestString kLocationButton{ 0x53a694, "MAIN_LOCATION_BUTTON" };
constexpr GuestString kLocationCascade{ 0x53a6ac, "MAIN_LOCATION_CASCADE" };
constexpr GuestString kOpponentCascade{ 0x53a6c4, "MAIN_OPPONENT_CASCADE" };
constexpr GuestString kSetPlayerName{ 0x53a6dc, "SET_PLAYERNAME1" };
constexpr GuestString kShowPlayerName{ 0x53a6ec, "SHOW_PLAYERNAME1" };
constexpr GuestString kAids{ 0x53a700, "ASSISTS" };
constexpr GuestString kAidsButton{ 0x53a708, "ASSISTS_BUTTON" };
constexpr GuestString kShiftMe{ 0x53a718, "shiftme" };
constexpr GuestString kGameLevel1{ 0x53a720, "gamelevel1" };
constexpr GuestString kGameLevel2{ 0x53a72c, "gamelevel2" };
constexpr GuestString kSetPlayerName2{ 0x53a738, "SET_PLAYERNAME2" };
constexpr GuestString kShowPlayerName2{ 0x53a748, "SHOW_PLAYERNAME2" };

/* A series runs its own tracks: the track cannot be chosen.  The opponents'
 * button and list are labelled for the kind of race. */
void fitTrackAndOpponents(Guest& guest, x86::reg32 menu, RaceType type)
{
    if (isSeries(type))
    {
        if (Item location = fe::findItem(guest, menu, kLocationButton))
            location.setDisabled(true);
        if (Item location = fe::findItem(guest, menu, kLocationCascade))
            location.setDisabled(true);
        if (Item opponents = fe::findItem(guest, menu, kOpponentButton))
            opponents.setText(fe::text(guest, kOpponentsTextSeries));
        if (Item opponents = fe::findItem(guest, menu, kOpponentCascade))
            opponents.setText(fe::text(guest, kOpponentsListSeries));
    }
    else
    {
        if (Item opponents = fe::findItem(guest, menu, kOpponentButton))
            opponents.setText(fe::text(guest, kOpponentsText));
        if (Item opponents = fe::findItem(guest, menu, kOpponentCascade))
            opponents.setText(fe::text(guest, kOpponentsList));
    }
}

/* Player one's name: the button that changes it, the line that shows it. */
void fitPlayerName(Guest& guest, x86::reg32 menu)
{
    if (Item button = fe::findItem(guest, menu, kSetPlayerName))
        button.setOnClick(kPlayer1NameButton);
    if (Item name = fe::findItem(guest, menu, kShowPlayerName))
        name.setText(kPlayer1Name.name);
}

/* The level of a series, shown only for one. */
void fitGameLevel(Guest& guest, x86::reg32 menu, RaceType type)
{
    if (Item level = fe::findItem(guest, menu, kGameLevel1))
    {
        level.setHidden(!isSeries(type));
        if (type == kTournament)
            level.setHelp(fe::text(guest, kLevelHelpTournament));
        else if (type == kKnockout)
            level.setHelp(fe::text(guest, kLevelHelpKnockout));
        else
            level.setHelp(0);
    }
    if (Item level = fe::findItem(guest, menu, kGameLevel2))
        level.setHidden(!isSeries(type));
}

/* sub_45d050, main as it opens: each item fitted to the kind of race. */
x86::reg32 onMainEnter(Guest& guest, x86::reg32 menu)
{
    const RaceType type = raceType(guest);
    fitTrackAndOpponents(guest, menu, type);
    fitPlayerName(guest, menu);

    // The driving aids: a single race's, or Hot Pursuit's own.  Their button
    // only in those two kinds of race.
    if (Item aids = fe::findItem(guest, menu, kAids))
        showAids(guest, aids,
                 type == kSingleRace ? singleRaceAidsOn(guest)
                                     : type == kHotPursuit && guest.dword(kHotPursuitAid) != 0);
    if (Item button = fe::findItem(guest, menu, kAidsButton))
        button.setDisabled(type != kSingleRace && type != kHotPursuit);

    if (Item shifted = fe::findItem(guest, menu, kShiftMe))
        placeAfterLabel(guest, shifted, kMainShiftLabel);
    fitGameLevel(guest, menu, type);

    // Back here from a network game: it is over.
    if (guest.word(kNetworkGame) != 0)
    {
        fe::messageBox(guest, kNetworkGameEndedText);
        guest.call(kLeaveNetworkGame);
    }
    return 0;
}

/* The rows a screen's items stand in, by their ypos: x (int16 every four
 * bytes, the arc the buttons follow) and y, shared by every screen.  Read as
 * each item is laid out, after the screen's own handler has run (sub_44a960:
 * the button's, the cascade's and the text's methods, sub_461900, sub_4623a0,
 * sub_4600b0). */
constexpr x86::reg32 kRowX = 0x559c6c;
constexpr x86::reg32 kRowY = 0x559c94;
constexpr std::size_t kRows = 8;
constexpr std::int16_t kOriginalRowX[kRows] = { 12, 5, 2, 1, 4, 9, 16, 27 };
constexpr std::int16_t kOriginalRowY[kRows] = { 138, 173, 208, 243, 278, 313, 348, 383 };
/* Port: mainsplt's eight rows stand 23 pixels higher, x following the arc up,
 * as Modern Patch has them (its nfs3.exe, the same two tables).  The original
 * puts them where main's are, and two players' names and a series' level
 * take the screen down to the buttons at its foot. */
constexpr std::int16_t kSplitRowX[kRows] = { 18, 8, 2, 0, 1, 3, 10, 19 };
constexpr std::int16_t kSplitRowY[kRows] = { 115, 150, 185, 220, 255, 290, 325, 360 };

void setRows(Guest& guest, const std::int16_t (&x)[kRows], const std::int16_t (&y)[kRows])
{
    for (std::size_t i = 0; i < kRows; ++i)
    {
        guest.setWord(kRowX + 4 * x86::reg32(i), std::uint16_t(x[i]));
        guest.setWord(kRowY + 4 * x86::reg32(i), std::uint16_t(y[i]));
    }
}

/* sub_45d360, mainsplt as it opens: two players on this machine, the second
 * one's name too.  The aids are a single race's only. */
x86::reg32 onSplitEnter(Guest& guest, x86::reg32 menu)
{
    setRows(guest, kSplitRowX, kSplitRowY);
    guest.setDword(kSplitScreen, 1);
    if (Item shifted = fe::findItem(guest, menu, kShiftMe))
        placeAfterLabel(guest, shifted, kSplitShiftLabel);

    const RaceType type = raceType(guest);
    fitTrackAndOpponents(guest, menu, type);
    fitPlayerName(guest, menu);

    // Player two's name: shown, and its button working.
    if (Item button = fe::findItem(guest, menu, kSetPlayerName2))
    {
        button.setOnClick(kPlayer2NameButton);
        button.setDisabled(false);
        button.setHidden(false);
    }
    if (Item name = fe::findItem(guest, menu, kShowPlayerName2))
    {
        name.setText(kPlayer2Name.name);
        name.setDisabled(false);
        name.setHidden(false);
    }

    fitGameLevel(guest, menu, type);
    if (Item button = fe::findItem(guest, menu, kAidsButton))
        button.setDisabled(type != kSingleRace);
    if (Item aids = fe::findItem(guest, menu, kAids))
        showAids(guest, aids, type == kSingleRace && singleRaceAidsOn(guest));
    return 0;
}

/* sub_45cff0, sub_45d300: main's and mainsplt's every pass of the menu loop. */
x86::reg32 onFrame(Guest& guest, x86::reg32 menu)
{
    // Left alone long enough, the screen gives way to the attract-mode movie.
    if (guest.call(kAttractMode, fe::args({ menu })) != 0)
        return fe::kMenuMoviePlayed;

    // The opponents' button pops up the list of the series' cars in a series,
    // nothing in any other race.
    const RaceType type = raceType(guest);
    if (Item opponents = fe::findItem(guest, menu, kOpponentButton))
        opponents.setListName(isSeries(type) ? kOpponentCars.address : 0);
    return 0;
}

/* ---------------------------------------------------------------------------
 * mainmult
 * ------------------------------------------------------------------------- */

/* The network screen chooses the kind of race from a list of its own, six
 * lines, each a race type and, for a series, a variant:
 *
 *   0  single race      2  type 2, variant 0      4  type 1, variant 0
 *   1  Hot Pursuit      3  type 2, variant 1      5  type 1, variant 1
 *
 * The machine that sets the race up chooses; the others follow the race type
 * it sends them. */
constexpr x86::reg32 kRaceListLine = 0x5556d8;         // the line chosen
constexpr x86::reg32 kRaceListLineShown = 0x5556dc;    // the line the screen was last fitted to
constexpr x86::reg32 kRaceVariant = 0x6fd470;          // a series' variant, 0 or 1
// Whether each line of the list can be chosen: seven int16 (1 can), and the
// list's name after them.
constexpr x86::reg32 kRaceListChoices = 0x555730;
constexpr std::uint32_t kRaceListChoiceCount = 7;
// Nonzero while a network game is set up (sub_43f320 sets it, sub_4a65e0
// clears it).
constexpr x86::reg32 kNetworkGameSetUp = 0x6fd50c;
// How this machine joined it (sub_43f320).  With 2 only the race already
// chosen stays on the list.
constexpr x86::reg32 kNetworkJoin = 0x7a22e4;
constexpr std::uint32_t kNetworkJoinFollow = 2;
// Nonzero (int16) on the machine that sets the race up: only it chooses the
// track and the opponents.
constexpr x86::reg32 kNetworkHost = 0x7a22da;
// Nonzero while connected to the others: leaving the screen asks first.
constexpr x86::reg32 kNetworkConnected = 0x7a22e0;

// The question before leaving: its line and its two buttons, char* each.
constexpr x86::reg32 kLeaveLines = 0x5f40b0;
constexpr x86::reg32 kLeaveButtons = 0x5f40b4;
constexpr std::uint32_t kLeaveQuestion = 0x101;
constexpr std::uint32_t kLeaveYes = 0xc4;
constexpr std::uint32_t kLeaveNo = 0xc5;

// The functions of this screen whose addresses it hands out or calls:
// the Back button's handler, its question's answer, and the two below.
constexpr x86::reg32 kMultiBackButton = 0x4400d0;
constexpr x86::reg32 kMultiLeaveAnswer = 0x43fcb0;
constexpr x86::reg32 kMultiApplyRaceList = 0x440160;
constexpr x86::reg32 kMultiFollowRaceType = 0x440390;

// Its items, by the codelinks MAINMULT.MNU gives them (its own copy of the
// strings, and of the list's name).
constexpr GuestString kMultiLocationButton{ 0x53784c, "MAIN_LOCATION_BUTTON" };
constexpr GuestString kMultiLocationCascade{ 0x537864, "MAIN_LOCATION_CASCADE" };
constexpr GuestString kMultiOpponentButton{ 0x53787c, "MAIN_OPPONENT_BUTTON" };
constexpr GuestString kMultiOpponentCascade{ 0x537894, "MAIN_OPPONENT_CASCADE" };
constexpr GuestString kMultiAidsButton{ 0x5378ac, "ASSISTS_BUTTON" };
constexpr GuestString kMultiAids{ 0x5378bc, "ASSISTS" };
constexpr GuestString kMultiSetPlayerName{ 0x5378c4, "SET_PLAYERNAME1" };
constexpr GuestString kMultiShowPlayerName{ 0x5378d4, "SHOW_PLAYERNAME1" };
constexpr GuestString kMultiBack{ 0x5378e8, "escape" };
constexpr GuestString kMultiShiftMe{ 0x5378f0, "shiftme" };
constexpr GuestString kMultiOpponentCars{ 0x55573e, "opcars" };

/* sub_440390: the list's line for the race type the game has -- what the
 * machines that do not set the race up follow.  Any other type leaves it.
 * The race type is what it leaves in eax. */
x86::reg32 onFollowRaceType(Guest& guest)
{
    std::uint32_t line = guest.dword(kRaceListLine);
    const bool firstVariant = guest.dword(kRaceVariant) == 0;
    const RaceType type = raceType(guest);
    switch (type)
    {
    case kSingleRace:
        line = 0;
        break;
    case kHotPursuit:
        line = 1;
        break;
    case kKnockout:
        line = firstVariant ? 2 : 3;
        break;
    case kTournament:
        line = firstVariant ? 4 : 5;
        break;
    }
    guest.setDword(kRaceListLine, line);
    return type;
}

/* sub_440160: the race type and variant of the list's line, and the items
 * fitted to them.  Only the machine that sets the race up chooses the track
 * and the opponents, and not for a series.  The aids are a single race's. */
x86::reg32 onApplyRaceList(Guest& guest, x86::reg32 menu)
{
    switch (guest.dword(kRaceListLine))
    {
    case 0:
        guest.setDword(kRaceType, kSingleRace);
        break;
    case 1:
        guest.setDword(kRaceType, kHotPursuit);
        break;
    case 2:
    case 3:
        guest.setDword(kRaceVariant, guest.dword(kRaceListLine) - 2);
        guest.setDword(kRaceType, kKnockout);
        break;
    case 4:
    case 5:
        guest.setDword(kRaceVariant, guest.dword(kRaceListLine) - 4);
        guest.setDword(kRaceType, kTournament);
        break;
    }

    const GuestString* choices[] = { &kMultiLocationButton, &kMultiLocationCascade, &kMultiOpponentButton,
                                     &kMultiOpponentCascade };
    for (const GuestString* codelink : choices)
    {
        if (Item item = fe::findItem(guest, menu, *codelink))
            item.setDisabled(guest.word(kNetworkHost) == 0 || isSeries(raceType(guest)));
    }
    if (Item button = fe::findItem(guest, menu, kMultiAidsButton))
        button.setDisabled(raceType(guest) != kSingleRace);
    if (Item aids = fe::findItem(guest, menu, kMultiAids))
        showAids(guest, aids, raceType(guest) == kSingleRace && singleRaceAidsOn(guest));
    return 0;
}

/* sub_440470, mainmult as it opens: which lines of the race list can be
 * chosen, the list's line for the race the game has, and the screen fitted
 * to it. */
x86::reg32 onMultiEnter(Guest& guest, x86::reg32 menu)
{
    for (std::uint32_t i = 0; i < kRaceListChoiceCount; ++i)
        guest.setWord(kRaceListChoices + 2 * i, 1);
    if (guest.dword(kNetworkGameSetUp) != 0)
    {
        // No Hot Pursuit over a network.
        guest.setWord(kRaceListChoices + 2 * 1, 0);
        if (raceType(guest) == kHotPursuit)
        {
            guest.setDword(kRaceListLine, 0);
            guest.setDword(kRaceType, kSingleRace);
        }
        // Following another machine's race: only that race on the list.  The
        // line is worked out as the game does, which is not the list's own
        // numbering for type 1 (onFollowRaceType).
        if (guest.dword(kNetworkJoin) == kNetworkJoinFollow)
        {
            const RaceType type = raceType(guest);
            guest.call(kFillZero, fe::args({ kRaceListChoices, 2 * kRaceListChoiceCount }));
            x86::reg32 line = type;
            if (type == kTournament)
                line += 1;
            if (std::int32_t(type) >= 2)
                line += guest.dword(kRaceVariant);
            guest.setWord(kRaceListChoices + 2 * line, 1);
        }
    }
    guest.call(kMultiFollowRaceType);
    guest.setDword(kRaceListLineShown, guest.dword(kRaceListLine));
    guest.call(kMultiApplyRaceList, fe::args({ menu }));

    if (Item button = fe::findItem(guest, menu, kMultiSetPlayerName))
        button.setOnClick(kPlayer1NameButton);
    if (Item name = fe::findItem(guest, menu, kMultiShowPlayerName))
        name.setText(kPlayer1Name.name);
    if (Item back = fe::findItem(guest, menu, kMultiBack))
        back.setOnClick(kMultiBackButton);
    if (Item shifted = fe::findItem(guest, menu, kMultiShiftMe))
        placeAfterLabel(guest, shifted, kMultiShiftLabel);
    return 0;
}

/* sub_4403f0, mainmult's every pass: a machine that does not set the race up
 * follows the race type sent to it; a line changed on the list is applied. */
x86::reg32 onMultiFrame(Guest& guest, x86::reg32 menu)
{
    if (guest.word(kNetworkHost) == 0)
        guest.call(kMultiFollowRaceType);
    const std::uint32_t line = guest.dword(kRaceListLine);
    if (line != guest.dword(kRaceListLineShown))
    {
        guest.setDword(kRaceListLineShown, line);
        guest.call(kMultiApplyRaceList, fe::args({ menu }));
    }

    // The opponents' button pops up the list of the series' cars in a series.
    const RaceType type = raceType(guest);
    if (Item opponents = fe::findItem(guest, menu, kMultiOpponentButton))
        opponents.setListName(isSeries(type) ? kMultiOpponentCars.address : 0);
    return 0;
}

/* sub_4400d0, mainmult's Back (the ipx screen's too): connected to others, a
 * question first; otherwise straight back to the first screen. */
x86::reg32 onMultiBack(Guest& guest)
{
    if (guest.dword(kNetworkConnected) == 0)
        return fe::kMenuBackToFirst;
    guest.setDword(kLeaveLines, fe::text(guest, kLeaveQuestion));
    guest.setDword(kLeaveButtons, fe::text(guest, kLeaveYes));
    guest.setDword(kLeaveButtons + 4, fe::text(guest, kLeaveNo));
    fe::Question question;
    question.lineCount = 1;
    question.lines = kLeaveLines;
    question.buttonCount = 2;
    question.buttons = kLeaveButtons;
    question.buttonHelp = 0;
    question.focus = 0;
    question.column = 0;
    question.callback = kMultiLeaveAnswer;
    fe::openQuestion(guest, question);
    return 0;
}

/* sub_43fcb0, the question answered: Enter on Yes leaves the network game's
 * screens for the first one; Enter on No, or Escape, closes the question.
 * `close`, when there is one, keeps the question up when cleared. */
x86::reg32 onMultiLeaveAnswer(Guest& guest, std::uint32_t answer, x86::reg32 close)
{
    auto setClose = [&](std::uint8_t value) {
        if (close != 0)
            guest.setByte(close, value);
    };
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            setClose(1);
            return fe::kMenuBackToFirst;
        }
        setClose(focus == 1 ? 1 : 0);
        return fe::kDialogHandled;
    }
    if (answer == fe::kDialogEscape)
    {
        setClose(1);
        return fe::kDialogHandled;
    }
    return 0;
}

const GuestString kMainEnterStrings[] = {
    kOpponentButton, kLocationButton, kLocationCascade, kOpponentCascade, kSetPlayerName, kShowPlayerName,
    kAids,           kAidsButton,     kShiftMe,         kGameLevel1,      kGameLevel2,
};
const GuestString kSplitEnterStrings[] = {
    kOpponentButton, kLocationButton, kLocationCascade, kOpponentCascade, kSetPlayerName, kShowPlayerName,
    kSetPlayerName2, kShowPlayerName2, kAids,           kAidsButton,      kShiftMe,       kGameLevel1,
    kGameLevel2,
};
const GuestString kFrameStrings[] = { kOpponentButton, kOpponentCars };
const GuestString kMultiEnterStrings[] = { kMultiSetPlayerName, kMultiShowPlayerName, kMultiBack, kMultiShiftMe };
const GuestString kMultiApplyStrings[] = {
    kMultiLocationButton, kMultiLocationCascade, kMultiOpponentButton,
    kMultiOpponentCascade, kMultiAidsButton,     kMultiAids,
};
const GuestString kMultiFrameStrings[] = { kMultiOpponentButton, kMultiOpponentCars };

}

/* Each stand-in: the generated function, what its ret pops, what its prologue
 * pushes, and the strings it hands to the game. */

bool mainMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d050", 0, 20, kMainEnterStrings, std::size(kMainEnterStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onMainEnter(guest, menu); });
}

bool mainMenuOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_45d2f0: nothing to undo.
    static fe::Site site{ "sub_45d2f0", 0, 4, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

bool mainMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45cff0", 0, 12, kFrameStrings, std::size(kFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onFrame(guest, menu); });
}

bool mainMenuOnPlayerName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45cef0", 0, 16, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest& guest) { return onPlayerName(guest, kPlayer1Name); });
}

bool mainMenuPlayerNameAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45cdd0", 0, 16, nullptr, 0 };
    const std::uint32_t answer = cpu.eax;
    const x86::reg32 close = cpu.edx;
    return fe::run(app, cpu, site,
                   [&](Guest& guest) { return onPlayerNameAnswer(guest, kPlayer1Name, answer, close); });
}

bool splitMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45d360", 0, 24, kSplitEnterStrings, std::size(kSplitEnterStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onSplitEnter(guest, menu); });
}

bool splitMenuOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_45d620: nothing to undo but the port's rows, back for the other
    // screens.
    static fe::Site site{ "sub_45d620", 0, 4, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest& guest) {
        setRows(guest, kOriginalRowX, kOriginalRowY);
        return x86::reg32(0);
    });
}

bool splitMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_45d300: main's, instruction for instruction.
    static fe::Site site{ "sub_45d300", 0, 12, kFrameStrings, std::size(kFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onFrame(guest, menu); });
}

bool splitMenuOnPlayer2Name(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45cf70", 0, 16, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest& guest) { return onPlayerName(guest, kPlayer2Name); });
}

bool splitMenuPlayer2NameAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45ce60", 0, 16, nullptr, 0 };
    const std::uint32_t answer = cpu.eax;
    const x86::reg32 close = cpu.edx;
    return fe::run(app, cpu, site,
                   [&](Guest& guest) { return onPlayerNameAnswer(guest, kPlayer2Name, answer, close); });
}

bool multiMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440470", 0, 20, kMultiEnterStrings, std::size(kMultiEnterStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onMultiEnter(guest, menu); });
}

bool multiMenuOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4405b0: nothing to undo.
    static fe::Site site{ "sub_4405b0", 0, 4, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

bool multiMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4403f0", 0, 16, kMultiFrameStrings, std::size(kMultiFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onMultiFrame(guest, menu); });
}

bool multiMenuApplyRaceList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440160", 0, 24, kMultiApplyStrings, std::size(kMultiApplyStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onApplyRaceList(guest, menu); });
}

bool multiMenuFollowRaceType(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_440390", 0, 8, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest& guest) { return onFollowRaceType(guest); });
}

bool multiMenuOnBack(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4400d0", 0, 16, nullptr, 0 };
    return fe::run(app, cpu, site, [](Guest& guest) { return onMultiBack(guest); });
}

bool multiMenuLeaveAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43fcb0", 0, 8, nullptr, 0 };
    const std::uint32_t answer = cpu.eax;
    const x86::reg32 close = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return onMultiLeaveAnswer(guest, answer, close); });
}

}
