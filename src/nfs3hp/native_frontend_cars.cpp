#include "native_frontend.h"
#include <cstring>
#include <iterator>

/* The screens a race's cars are chosen on, decompiled:
 *
 *   car1, car2   MENUS/CAR1.MNU and CAR2.MNU, each player's car: its colour,
 *                its showcase, the way to its tuning;
 *   opponent     OPPONENT.MNU, the opponents: how many, which car, traffic;
 *   cedbasic     CEDBASIC.MNU, a car's tuning: seven sliders, from the car
 *                screens (a player's own car) or a race's pause screens.
 *
 * Each is a row of the handler table at 0x555c48 -- as the screen opens, as it
 * closes, on every pass of the menu loop -- and their buttons and lists are
 * handed handlers of their own, here too, with the few functions only these
 * screens call.  The cars themselves -- their names, colours, showcases and
 * tuning records -- are the game's, called by address.
 *
 * The car screens and the track screen also hold a developer's recorder: with
 * bit 0x40 of the byte at 0x55eb2f it saves the turning car, or the track,
 * frame by frame to c:\NN\NNNN.tga (sub_426870, sub_426b00). */
namespace nfs3hp
{

/* Port: the phone's keyboard, up while the game takes typing (nfs3hp_main.cpp,
 * tools/apply_text_entry.py). */
void textEntry(bool taking);

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kInRace = 0x559234;          // the menus are a race's pause screens
constexpr x86::reg32 kSplitScreen = 0x6fd3b0;     // 1: two players on this machine; 2: a network race
constexpr x86::reg32 kRaceType = 0x6fd3b8;        // 0 a single race, 1 and 2 the series, 3 Hot Pursuit
constexpr std::uint32_t kHotPursuit = 3;
// The players' cars: a record each, the car's number its first dword.
constexpr x86::reg32 kPlayer1Car = 0x6fd2bc;
constexpr x86::reg32 kPlayer2Car = 0x6fd328;

constexpr x86::reg32 kOnKey = 0x30;
constexpr x86::reg32 kListName = 0x64;            // a list's: the name of its entries (a button's kOnClick)
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr x86::reg32 kGoOn = 5;                   // to the selected button's nextmenu (sub_44b230)
// The screen closes without fading out, and opens again (sub_44b230).
constexpr x86::reg32 kReopen = 9;

// The menu engine's and the widgets'.
constexpr x86::reg32 kCurrentMenu = 0x44aeb0;
constexpr x86::reg32 kShownNow = 0x44a660;
constexpr x86::reg32 kButtonKey = 0x461270;
constexpr x86::reg32 kListKey = 0x463ff0;
constexpr x86::reg32 kSliderSetValue = 0x46c950;  // eax the item; the value and the end on the stack
constexpr x86::reg32 kSliderValue = 0x46c9b0;     // eax the item, edx the end; the value on the x87
constexpr x86::reg32 kSliderSetOnMove = 0x46cac0; // edx the function its moves go to
constexpr x86::reg32 kDimBackground = 0x451530;   // a float on the stack: a race's pause screen behind
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;   // 0.6f
constexpr x86::reg32 kSprintf = 0x4df690;

// The cars' (0x439...).
constexpr x86::reg32 kCarData = 0x439520;          // eax a car
constexpr x86::reg32 kCarFolder = 0x439660;        // eax what kCarData gave: the number of its folder
constexpr x86::reg32 kCarColourCount = 0x439760;   // eax a car
constexpr x86::reg32 kCarColour = 0x439790;        // eax a car, edx the player
constexpr x86::reg32 kSetCarColour = 0x4397d0;     // eax a car, edx the player, ebx the colour
constexpr x86::reg32 kCarOneColour = 0x4396b0;     // eax a car: nonzero when it comes in one colour
constexpr x86::reg32 kCarTunable = 0x439a50;       // eax a car
constexpr x86::reg32 kHasShowcase = 0x4530c0;      // eax a car
constexpr x86::reg32 kShowcaseDownload = 0x452e60; // CAR_DOWNLOAD's click, in the original
constexpr x86::reg32 kRefreshColourList = 0x456a20;// eax the colour list's item

/* A greyed item, and greyed and hidden, the state and flags bytes as one
 * word. */
void grey(Item item, bool greyed)
{
    item.setDisabled(greyed);
}

void greyAndHide(Guest& guest, Item item, bool greyedAndHidden)
{
    const x86::reg32 state = item.address() + fe::item::kState;
    const std::uint16_t word = guest.word(state);
    guest.setWord(state, std::uint16_t(greyedAndHidden ? word | 0x1001 : word & 0xeffe));
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

x86::reg32 passKey(Guest& guest, x86::reg32 handler, x86::reg32 item, x86::reg32 menu, std::uint16_t key)
{
    return guest.call(handler, fe::args({ item, menu, x86::reg32(std::int32_t(std::int16_t(key))) }));
}

/* ---------------------------------------------------------------------------
 * The music, back to the menus'
 * ------------------------------------------------------------------------- */

// Set while a music other than the menus' plays (a car's, sub_4104f0).
constexpr x86::reg32 kOtherMusic = 0x552594;
// The sound setting 6 plays no menu music.
constexpr x86::reg32 kSoundSetting1ec = 0x6fbbec;
constexpr std::uint32_t kNoMenuMusic = 6;
constexpr x86::reg32 kMenuMusicVolume = 0x6fbbe8;
constexpr x86::reg32 kStopMusic = 0x4109d0;
constexpr x86::reg32 kPlayMusic = 0x4104f0;       // eax the tune, edx the volume, ebx 0
constexpr std::uint32_t kMenuTune = 0x16;

/* sub_4104b0: as a car screen opens, the menus' music again if another plays. */
x86::reg32 menuMusicBack(Guest& guest)
{
    if (guest.dword(kOtherMusic) != 0)
    {
        guest.setDword(kOtherMusic, 0);
        if (guest.dword(kSoundSetting1ec) != kNoMenuMusic)
        {
            guest.call(kStopMusic);
            guest.call(kPlayMusic, fe::args({ kMenuTune, guest.dword(kMenuMusicVolume), 0 }));
        }
    }
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * The developer's recorder
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kDeveloperFlags = 0x55eb2f;  // byte
constexpr std::uint8_t kRecordFrames = 0x40;
// The 3D view ready to be recorded: the screen's model, the display.
constexpr x86::reg32 kModelReady = 0x564850;      // byte
constexpr x86::reg32 kDisplayReady = 0x564831;    // byte
// Where it stands: 0 not started; 1 and 2, the frames of a turn, one shot
// every other pass; 3 to 8 the next turns, with the view's flags.
constexpr x86::reg32 kRecordStep = 0x553038;
constexpr x86::reg32 kRecordTurn = 0x55302c;      // float: the model's turn, 1/32 a frame
constexpr x86::reg32 kRecordFrame = 0x5e10d0;     // 0 to 32 in a turn
constexpr x86::reg32 kRecordShot = 0x55303c;      // the next file's number in its hundred
constexpr x86::reg32 kRecordViewA = 0x553030;     // read by the 3D view
constexpr x86::reg32 kRecordViewB = 0x553034;
constexpr x86::reg32 kTrack = 0x6fd4ac;           // the track chosen
constexpr std::int32_t kFramesInTurn = 31;
constexpr double kTurnStep = 0.03125;             // at 0x536f94 and 0x536fac

constexpr x86::reg32 kMakeFolder = 0x4eb1d0;      // eax a path
constexpr x86::reg32 kLockScreen = 0x4ead20;
constexpr x86::reg32 kSaveScreen = 0x4eb160;      // eax a .tga's path
constexpr x86::reg32 kUnlockScreen = 0x4ead90;
constexpr x86::reg32 kSaveCarFrame = 0x4267a0;
constexpr x86::reg32 kSaveTrackFrame = 0x4267f0;

constexpr GuestString kCarFrameName{ 0x536f64, "c:\\%02d\\%04d.tga" };
constexpr GuestString kTrackFrameName{ 0x536f78, "c:\\T%02d\\%04d.tga" };
constexpr GuestString kCarFolderName{ 0x536f8c, "c:\\%02d" };
constexpr GuestString kTrackFolderName{ 0x536f9c, "c:\\T%02d" };

/* sub_4267a0, sub_4267f0: the screen saved as the next frame, `first` and
 * on, in the folder of `folder`. */
x86::reg32 saveFrame(Guest& guest, const GuestString& format, x86::reg32 folder, x86::reg32 first)
{
    const x86::reg32 name = guest.frame();    // char[0x12c]
    const x86::reg32 number = first + guest.dword(kRecordShot);
    guest.call(kSprintf, fe::args({}, { name, format.address, folder, number }));
    const x86::reg32 next = guest.dword(kRecordShot) + 1;
    guest.setDword(kRecordShot, next);
    // As the original leaves them for the calls after.
    guest.leaveEdx(number);
    guest.leaveEbx(next);
    guest.call(kLockScreen);
    guest.call(kSaveScreen, fe::args({ name }));
    return guest.call(kUnlockScreen);
}

/* The turn moved on by a frame; true at its end. */
bool nextFrame(Guest& guest)
{
    const x86::reg32 frame = guest.dword(kRecordFrame) + 1;
    guest.setDword(kRecordFrame, frame);
    x86::FPU& fpu = guest.fpu();
    guest.setFloat(kRecordTurn, float(fp::add(x86::Float(guest.floatAt(kRecordTurn)), x86::Float(kTurnStep))));
    return std::int32_t(frame) > kFramesInTurn;
}

/* A turn's frame for player one's car, from the hundred `first`; edx as the
 * original leaves it for the first call. */
x86::reg32 recordCarFrame(Guest& guest, x86::reg32 first, x86::reg32 edx)
{
    const x86::reg32 data = guest.call(kCarData, fe::args({ guest.dword(kPlayer1Car), edx }));
    const x86::reg32 folder = guest.call(kCarFolder, fe::args({ data, first }));
    return guest.call(kSaveCarFrame, fe::args({ folder, first }));
}

bool recording(const Guest& guest)
{
    return (guest.byte(kDeveloperFlags) & kRecordFrames) != 0;
}

/* sub_426870, on every pass of car1 and car2: player one's car turning,
 * three turns -- 100 on, 200 on with the view's first flag, 300 on with its
 * second -- a frame every other pass. */
x86::reg32 recordCar(Guest& guest)
{
    if (!recording(guest))
        return guest.eax();
    const x86::reg32 step = guest.dword(kRecordStep);
    switch (step)
    {
    case 0:
    {
        if (guest.byte(kModelReady) == 0 || guest.byte(kDisplayReady) == 0)
            return step;
        guest.setDword(kRecordStep, 1);
        guest.setDword(kRecordTurn, 0);
        guest.setDword(kRecordViewA, 1);
        guest.setDword(kRecordFrame, 0);
        guest.setDword(kRecordShot, 0);
        guest.setDword(kRecordViewB, 0);
        const x86::reg32 data = guest.call(kCarData, fe::args({ guest.dword(kPlayer1Car), 1 }));
        const x86::reg32 folder = guest.call(kCarFolder, fe::args({ data }));
        const x86::reg32 name = guest.frame();    // char[0x134]
        guest.call(kSprintf, fe::args({}, { name, kCarFolderName.address, folder }));
        return guest.call(kMakeFolder, fe::args({ name }));
    }
    case 1:
    case 4:
    case 7:
        guest.setDword(kRecordStep, step + 1);
        return step;
    case 2:
    {
        guest.setDword(kRecordStep, 1);
        const x86::reg32 result = recordCarFrame(guest, 100, 100);
        if (nextFrame(guest))
            guest.setDword(kRecordStep, 3);
        return result;
    }
    case 3:
    case 6:
        guest.setDword(kRecordViewA, 0);
        if (step == 6)
            guest.setDword(kRecordViewB, 1);
        guest.setDword(kRecordFrame, 0);
        guest.setDword(kRecordShot, 0);
        guest.setDword(kRecordStep, step + 1);
        guest.setDword(kRecordTurn, 0);
        return 0;
    case 5:
    {
        guest.setDword(kRecordStep, 4);
        const x86::reg32 result = recordCarFrame(guest, 200, 200);
        if (nextFrame(guest))
        {
            guest.setDword(kRecordViewA, 1);
            guest.setDword(kRecordTurn, 0);
            guest.setDword(kRecordStep, 6);
        }
        return result;
    }
    case 8:
    {
        guest.setDword(kRecordStep, 7);
        const x86::reg32 result = recordCarFrame(guest, 300, 7);
        if (nextFrame(guest))
        {
            guest.setDword(kRecordViewA, 1);
            guest.setDword(kRecordTurn, 0);
            guest.setDword(kRecordStep, 0);
        }
        return result;
    }
    default:
        return step;
    }
}

/* sub_426b00, on every pass of location: the track turning, one turn, a frame
 * every other pass, to c:\TNN. */
x86::reg32 recordTrack(Guest& guest)
{
    if (!recording(guest))
        return guest.eax();
    const x86::reg32 step = guest.dword(kRecordStep);
    if (step == 0)
    {
        if (guest.byte(kModelReady) == 0 || guest.byte(kDisplayReady) == 0)
            return 0;
        guest.setDword(kRecordStep, 1);
        guest.setDword(kRecordViewA, 1);
        guest.setDword(kRecordTurn, 0);
        guest.setDword(kRecordFrame, 0);
        guest.setDword(kRecordShot, 0);
        guest.setDword(kRecordViewB, 0);
        const x86::reg32 name = guest.frame();    // char[0x134]
        guest.call(kSprintf, fe::args({}, { name, kTrackFolderName.address, guest.dword(kTrack) }));
        return guest.call(kMakeFolder, fe::args({ name }));
    }
    if (step == 1)
    {
        guest.setDword(kRecordStep, 2);
        return step;
    }
    if (step != 2)
        return step;
    guest.setDword(kRecordStep, 1);
    x86::reg32 result = guest.call(kSaveTrackFrame, fe::args({ guest.dword(kTrack), 100 }));
    if (nextFrame(guest))
    {
        guest.setDword(kRecordStep, 0);
        result = 0;
    }
    return result;
}

/* ---------------------------------------------------------------------------
 * car1 and car2
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kTuningFor = 0x553048;       // who the tuning screen is for: 1 player one, 2 player two
constexpr x86::reg32 kColourList = 0xb3;           // byte of the 3D model's item: its colours changed

constexpr std::uint32_t kPlayer1CarTitle = 0x1aa;  // two players on this machine
constexpr std::uint32_t kYourCarTitle = 0x102;

/* A player's screen: its car, its codelinks and the handlers it hands out. */
struct CarScreen
{
    x86::reg32 car;
    std::uint32_t player;
    GuestString edit;            // CE_EDITPn, to the tuning
    GuestString model;           // CAR_SELECT_OBJ3DPn
    GuestString colours;         // CAR_COLOR_SELECT_Pn
    x86::reg32 editClicked;
    x86::reg32 colourClicked;
};

constexpr GuestString kPlayerCarTitle{ 0x537010, "playerCarTitle" };
constexpr GuestString kColourButton{ 0x537020, "CAR_COLOR_CHANGE_BUTTON" };
constexpr GuestString kDownload{ 0x537038, "CAR_DOWNLOAD" };
constexpr GuestString kHideThis{ 0x537048, "hidethis" };
constexpr GuestString kShowcase{ 0x537054, "showcase" };
constexpr GuestString kCompare{ 0x53706c, "compare" };

constexpr CarScreen kCar1{ kPlayer1Car, 0, { 0x537004, "CE_EDITP1" }, { 0x536fb4, "CAR_SELECT_OBJ3DP1" },
                           { 0x536fc8, "CAR_COLOR_SELECT_P1" }, 0x426c20, 0x426c60 };
constexpr CarScreen kCar2{ kPlayer2Car, 1, { 0x537060, "CE_EDITP2" }, { 0x536fdc, "CAR_SELECT_OBJ3DP2" },
                           { 0x536ff0, "CAR_COLOR_SELECT_P2" }, 0x426c40, 0x426ce0 };

/* The tuning button: not in a network race, nor in a series. */
void fitEditButton(Guest& guest, x86::reg32 menu, const CarScreen& screen)
{
    if (Item edit = fe::findItem(guest, menu, screen.edit))
    {
        const std::uint32_t type = guest.dword(kRaceType);
        if (std::int32_t(guest.dword(kSplitScreen)) > 1 || type == 1 || type == 2)
            grey(edit, true);
        else
            edit.setOnClick(screen.editClicked);
    }
}

/* The showcase's button, for a car that has one. */
void fitShowcase(Guest& guest, x86::reg32 menu, const CarScreen& screen)
{
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
        grey(showcase, guest.call(kHasShowcase, fe::args({ guest.dword(screen.car) })) == 0);
}

/* sub_426d60, car1 as it opens. */
x86::reg32 onCar1Enter(Guest& guest, x86::reg32 menu)
{
    guest.call(0x4104b0);
    fitEditButton(guest, menu, kCar1);
    if (Item title = fe::findItem(guest, menu, kPlayerCarTitle))
        title.setText(fe::text(guest, guest.dword(kSplitScreen) == 1 ? kPlayer1CarTitle : kYourCarTitle));
    if (Item colour = fe::findItem(guest, menu, kColourButton))
        colour.setOnClick(kCar1.colourClicked);
    // Port: Download Car hidden, as hidethis is -- there is nothing left to
    // download (tools/apply_hide_download.py); the original gives it its
    // click, kShowcaseDownload.
    if (Item download = fe::findItem(guest, menu, kDownload))
        download.setHidden(true);
    if (Item hidden = fe::findItem(guest, menu, kHideThis))
        hidden.setHidden(true);
    return 0;
}

/* sub_426ef0, car2 as it opens. */
x86::reg32 onCar2Enter(Guest& guest, x86::reg32 menu)
{
    guest.call(0x4104b0);
    fitEditButton(guest, menu, kCar2);
    if (Item colour = fe::findItem(guest, menu, kColourButton))
        colour.setOnClick(kCar2.colourClicked);
    fitShowcase(guest, menu, kCar2);
    return 0;
}

/* Whether a car's colour can be changed: it has more than one. */
bool oneColour(Guest& guest, x86::reg32 car)
{
    return guest.call(kCarOneColour, fe::args({ guest.dword(car) })) != 0
           || guest.call(kCarColourCount, fe::args({ guest.dword(car) })) == 1;
}

/* sub_426e20, sub_426f80: car1's and car2's every pass. */
x86::reg32 onCarFrame(Guest& guest, x86::reg32 menu, const CarScreen& screen)
{
    guest.call(0x426870);
    fitShowcase(guest, menu, screen);
    if (Item colours = fe::findItem(guest, menu, screen.colours))
        greyAndHide(guest, colours, oneColour(guest, screen.car));
    if (Item button = fe::findItem(guest, menu, kColourButton))
        grey(button, oneColour(guest, screen.car));
    return 0;
}

/* sub_426c20, sub_426c40: CE_EDITPn clicked, the tuning for its player. */
x86::reg32 onEditClicked(Guest& guest, x86::reg32 player)
{
    guest.setDword(kTuningFor, player);
    return kGoOn;
}

/* sub_426c60, sub_426ce0: the colour button clicked, the car's next colour,
 * the model and the list told. */
x86::reg32 onColourClicked(Guest& guest, const CarScreen& screen)
{
    const std::int32_t colour =
        std::int32_t(guest.call(kCarColour, fe::args({ guest.dword(screen.car), screen.player })));
    const std::int32_t count = std::int32_t(guest.call(kCarColourCount, fe::args({ guest.dword(screen.car) })));
    if (colour >= 0 && count > 0)
    {
        const x86::reg32 next = x86::reg32((colour + 1) % count);
        guest.call(kSetCarColour, fe::args({ guest.dword(screen.car), screen.player, next }));
        if (Item model = fe::findItem(guest, guest.call(kCurrentMenu), screen.model))
            guest.setByte(model.address() + kColourList, 1);
        if (Item colours = fe::findItem(guest, guest.call(kCurrentMenu), screen.colours))
            guest.call(kRefreshColourList, fe::args({ colours.address() }));
    }
    return fe::kDialogHandled;
}

/* ---------------------------------------------------------------------------
 * opponent
 * ------------------------------------------------------------------------- */

/* The opponents: 1 to 3 the number of them, 0 none; past 3 every one is the
 * same car, the setting less 4. */
constexpr x86::reg32 kOpponents = 0x6fd4d4;
constexpr x86::reg32 kTraffic = 0x6fd4d0;          // 2: none
constexpr std::int32_t kOpponentCarBase = 4;
// The number of opponents the screen's art was laid out for.
constexpr x86::reg32 kOpponentsShown = 0x553040;
constexpr x86::reg32 kLongSeries = 0x439ce0;       // three opponents look like four

// The art: the opponents' picture, opp1.qfs to opp4.qfs -- the number written
// into the game's own string -- or the car's.
constexpr x86::reg32 kOpponentPicture = 0x537090;  // "oppN.qfs", N at +3
constexpr GuestString kCarPicture{ 0x537084, "carbg.qfs" };
constexpr x86::reg32 kPictureFile = 0x40;          // an item's: the picture it shows
constexpr x86::reg32 kLayer = 0x0a;                // int16

constexpr GuestString kBack{ 0x537074, "back" };
constexpr GuestString kMask{ 0x53707c, "mask" };
constexpr GuestString kTrafficButton{ 0x53709c, "TRAFFIC_BUTTON" };
constexpr GuestString kTrafficList{ 0x5370ac, "TRAFFIC_CASCADE" };
constexpr GuestString kOpponentCount{ 0x5370bc, "OPPNUM" };
constexpr GuestString kOpponentCountList{ 0x5370c4, "OPPNUM_CASCADE" };

/* The number of opponents, 0 for one car. */
std::int32_t opponentCount(const Guest& guest)
{
    const std::int32_t opponents = std::int32_t(guest.dword(kOpponents));
    return opponents > 3 || opponents == 0 ? 0 : opponents;
}

/* sub_427050: the showcase and the comparison, for one car. */
x86::reg32 fitOpponentCar(Guest& guest, x86::reg32 menu)
{
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
    {
        const std::int32_t opponents = std::int32_t(guest.dword(kOpponents));
        if (opponents > 3)
            grey(showcase, guest.call(kHasShowcase, fe::args({ x86::reg32(opponents - kOpponentCarBase) })) == 0);
        else
            grey(showcase, true);
    }
    Item compare = fe::findItem(guest, menu, kCompare);
    if (compare)
        grey(compare, std::int32_t(guest.dword(kOpponents)) <= 3);
    return compare.address();
}

/* sub_4270c0: the background and its mask, the opponents' picture or the
 * car's. */
x86::reg32 fitOpponentArt(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 back = fe::findItem(guest, menu, kBack).address();
    const x86::reg32 mask = fe::findItem(guest, menu, kMask).address();
    const std::int32_t opponents = std::int32_t(guest.dword(kOpponents));
    if (opponents > 3 || opponents == 0)
    {
        guest.setWord(back + fe::item::kY, 0);
        guest.setWord(back + kLayer, 0xc);
        guest.setDword(back + kPictureFile, kCarPicture.address);
        guest.setWord(back + fe::item::kX, 0);
        guest.setWord(mask + fe::item::kX, 0x89);
        guest.setWord(mask + fe::item::kY, 0x4a);
        guest.setWord(mask + kLayer, 9);
        guest.setDword(mask + kPictureFile, kCarPicture.address);
        return mask;
    }
    const std::uint8_t digit = std::uint8_t(guest.byte(kOpponents) + '0');
    guest.setByte(kOpponentPicture + 3, digit);
    x86::reg32 result = (mask & 0xffffff00) | digit;
    if (opponents == 3)
    {
        result = guest.call(kLongSeries);
        if (result != 0)
            guest.setByte(kOpponentPicture + 3, '4');
    }
    guest.setWord(back + fe::item::kY, 0);
    guest.setWord(back + kLayer, 9);
    guest.setWord(back + fe::item::kX, 0x89);
    guest.setDword(back + kPictureFile, kOpponentPicture);
    guest.setWord(mask + fe::item::kX, 0);
    guest.setWord(mask + fe::item::kY, 0);
    guest.setWord(mask + kLayer, 0xc);
    guest.setDword(mask + kPictureFile, kOpponentPicture);
    return result;
}

/* sub_4272a0, opponent as it opens. */
x86::reg32 onOpponentEnter(Guest& guest, x86::reg32 menu)
{
    guest.call(0x4270c0, fe::args({ menu }));
    guest.setDword(kOpponentsShown, x86::reg32(opponentCount(guest)));
    guest.call(0x427050, fe::args({ menu }));
    return 0;
}

/* sub_427190, opponent on every pass: traffic only in a single race that has
 * it, the number of opponents not in Hot Pursuit; a change of the number
 * opens the screen again, for its art. */
x86::reg32 onOpponentFrame(Guest& guest, x86::reg32 menu)
{
    guest.call(0x4104b0);
    auto trafficOff = [&]() {
        return guest.dword(kRaceType) != 0 || guest.dword(kTraffic) == 2;
    };
    if (Item button = fe::findItem(guest, menu, kTrafficButton))
    {
        if (guest.dword(kRaceType) != kHotPursuit)
            grey(button, trafficOff());
    }
    if (Item list = fe::findItem(guest, menu, kTrafficList))
    {
        if (guest.dword(kRaceType) != kHotPursuit)
            greyAndHide(guest, list, trafficOff());
    }
    if (Item count = fe::findItem(guest, menu, kOpponentCount))
        grey(count, guest.dword(kRaceType) == kHotPursuit);
    if (Item list = fe::findItem(guest, menu, kOpponentCountList))
        greyAndHide(guest, list, guest.dword(kRaceType) == kHotPursuit);
    guest.call(0x427050, fe::args({ menu }));
    return x86::reg32(opponentCount(guest)) != guest.dword(kOpponentsShown) ? kReopen : 0;
}

/* ---------------------------------------------------------------------------
 * cedbasic: a car's tuning
 * ------------------------------------------------------------------------- */

/* Seven settings, each a float between its least and its most, edited as the
 * screen's sliders (0 to 1) show them.  The player's own are the game's
 * (sub_4dc550, a record for each player); a car with none has the defaults. */
constexpr std::uint32_t kSettings = 7;
constexpr x86::reg32 kTuning = 0x5e10e0;           // the settings edited
constexpr x86::reg32 kTuningDefaults = 0x55d024;
constexpr x86::reg32 kTuningMost = 0x55d040;
constexpr x86::reg32 kTuningLeast = 0x55d05c;
constexpr x86::reg32 kTuningShown = 0x5e10fc;      // the sliders as the screen opened
constexpr std::uint32_t kTuningBytes = 0x1c;
constexpr x86::reg32 kPlayerTuning = 0x4dc550;     // eax the player, edx the car's data: the record, or 0
constexpr x86::reg32 kPlayerHasAdvanced = 0x4dca50;// eax the player

/* The advanced tuning, cedadv's: 50 records of 0x30 bytes, the one edited and
 * each player's own. */
constexpr x86::reg32 kAdvancedEdited = 0x8b9b60;
constexpr x86::reg32 kAdvancedPlayer1 = 0x8b9200;
constexpr x86::reg32 kAdvancedPlayer2 = 0x8b88a0;
constexpr std::uint32_t kAdvancedRecords = 50;
constexpr x86::reg32 kAdvancedRecordSize = 0x30;

// The player whose car is tuned: kPlayer1Car or kPlayer2Car.
constexpr x86::reg32 kTunedPlayer = 0x55304c;
constexpr x86::reg32 kTuningForLast = 0x553044;    // kTuningFor as the screen last opened
constexpr x86::reg32 kTuningMenu = 0x553050;
// The question before the defaults: its line and two buttons.
constexpr x86::reg32 kDefaultsLines = 0x553054;
constexpr x86::reg32 kDefaultsButtons = 0x553058;
constexpr std::uint32_t kDefaultsQuestion = 0x1e9;
constexpr std::uint32_t kYes = 0xc4;
constexpr std::uint32_t kNo = 0xc5;
// The lists of cars CE_SLCT_CAR shows, by their names.
constexpr x86::reg32 kCarsList = 0x553060;         // "cars"
constexpr x86::reg32 kCars2List = 0x553065;        // "car2"

constexpr GuestString kSettingCodelinks[kSettings] = {
    { 0x5370d4, "CE_ENGINETUNE" }, { 0x5370e4, "CE_BRAKEBAL" }, { 0x5370f0, "CE_STEERINGSPD" },
    { 0x537100, "CE_BGRRATS" },    { 0x53710c, "CE_SUSPENSION" }, { 0x53711c, "CE_AERODYN" },
    { 0x537128, "CE_TIRES" },
};
constexpr GuestString kDone{ 0x537134, "CEB_DONE" };
constexpr GuestString kCancel{ 0x537140, "CEB_CANCEL" };
constexpr GuestString kRaceNow{ 0x53714c, "racenow" };
constexpr GuestString kExitProgram{ 0x537154, "exitprogram" };
constexpr GuestString kOptionsNow{ 0x537160, "optionsnow" };
constexpr GuestString kCarList{ 0x53716c, "CE_SLCT_CAR" };
constexpr GuestString kDefaults{ 0x537178, "CEB_DEFAULT" };
constexpr GuestString kAdvanced{ 0x537184, "CEB_GOTOADVANCED" };

constexpr x86::reg32 kSettingMoved[kSettings] = { 0x4273e0, 0x427470, 0x427500, 0x427590,
                                                  0x427620, 0x4276b0, 0x427740 };
constexpr x86::reg32 kLoadTuning = 0x4272f0;
constexpr x86::reg32 kSaveTuning = 0x427370;
constexpr x86::reg32 kSetSliders = 0x4277d0;
constexpr x86::reg32 kRememberSliders = 0x4279b0;
constexpr x86::reg32 kSlidersMoved = 0x427ac0;
constexpr x86::reg32 kBindSliders = 0x427c20;
constexpr x86::reg32 kLoadAdvanced = 0x427d00;
constexpr x86::reg32 kDefaultsAnswered = 0x427d80;
constexpr x86::reg32 kDoneKey = 0x427e10;
constexpr x86::reg32 kCancelKey = 0x427ea0;
constexpr x86::reg32 kCarListKey = 0x427f40;
constexpr x86::reg32 kDefaultsKey = 0x427fb0;
constexpr x86::reg32 kBindDoneCancel = 0x428020;
constexpr x86::reg32 kFitTuningButtons = 0x428060;
constexpr x86::reg32 kPrepareTuning = 0x4281e0;
constexpr x86::reg32 kAdvancedDiffers = 0x4dcaf0;
constexpr x86::reg32 kCopyAdvanced = 0x4dcb60;

/* The tuned player's record of the advanced tuning, 0 for neither. */
x86::reg32 advancedOf(const Guest& guest)
{
    const x86::reg32 player = guest.dword(kTunedPlayer);
    return player == kPlayer1Car ? kAdvancedPlayer1 : player == kPlayer2Car ? kAdvancedPlayer2 : 0;
}

/* The car tuned. */
x86::reg32 tunedCar(const Guest& guest)
{
    return guest.dword(guest.dword(kTunedPlayer));
}

/* The player's tuning record for a car, 0 for none. */
x86::reg32 playerTuning(Guest& guest, x86::reg32 car)
{
    x86::reg32 data = 0;
    if (guest.call(kCarTunable, fe::args({ car })) != 0)
        data = guest.call(kCarData, fe::args({ car }));
    if (data == 0)
        return 0;
    const x86::reg32 player = guest.dword(kTunedPlayer);
    if (player == kPlayer1Car)
        return guest.call(kPlayerTuning, fe::args({ 0, data }));
    if (player == kPlayer2Car)
        return guest.call(kPlayerTuning, fe::args({ 1, data }));
    return 0;
}

/* `count` bytes, a dword at a time, as rep movsd copies them. */
void copyDwords(Guest& guest, x86::reg32 to, x86::reg32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; i += 4)
        guest.setDword(to + i, guest.dword(from + i));
}

/* sub_4272f0: a car's tuning to edit, the player's or the defaults. */
x86::reg32 loadTuning(Guest& guest, x86::reg32 car)
{
    x86::reg32 from = playerTuning(guest, car);
    if (from == 0)
        from = kTuningDefaults;
    copyDwords(guest, kTuning, from, kTuningBytes);
    return kTuningBytes;
}

/* sub_427370: the tuning edited, to the player's record for a car. */
x86::reg32 saveTuning(Guest& guest, x86::reg32 car)
{
    x86::reg32 data = 0;
    if (guest.call(kCarTunable, fe::args({ car })) != 0)
        data = guest.call(kCarData, fe::args({ car }));
    if (data == 0)
        return guest.eax();
    const x86::reg32 player = guest.dword(kTunedPlayer);
    x86::reg32 to = 0;
    if (player == kPlayer1Car)
        to = guest.call(kPlayerTuning, fe::args({ 0, data }));
    else if (player == kPlayer2Car)
        to = guest.call(kPlayerTuning, fe::args({ 1, data }));
    else
        return data;
    if (to == 0)
        return 0;
    copyDwords(guest, to, kTuning, kTuningBytes);
    return kTuningBytes;
}

/* What jbe reads after fcom a, b: a not above b, or unordered. */
bool notAbove(Guest& guest, const x86::Float& a, const x86::Float& b)
{
    guest.fpu().compare(a, b);
    return guest.fpu().status.c0 || guest.fpu().status.c3;
}

/* sub_4273e0 and the six after it: a setting's slider moved, the setting
 * from its least to its most. */
x86::reg32 onSettingMoved(Guest& guest, x86::reg32 item, std::uint32_t setting)
{
    const float least = guest.floatAt(kTuningLeast + 4 * setting);
    const float most = guest.floatAt(kTuningMost + 4 * setting);
    x86::FPU& fpu = guest.fpu();
    const float range = float(fp::sub(x86::Float(most), x86::Float(least)));
    guest.call(kSliderValue, fe::args({ item, 0 }));
    const x86::Float slider = fpu.pop();
    const float value = float(fp::add(fp::mul(slider, x86::Float(range)), x86::Float(least)));
    const float atLeast = notAbove(guest, x86::Float(least), x86::Float(value)) ? value : least;
    float result;
    if (!notAbove(guest, x86::Float(atLeast), x86::Float(most)))
        result = most;
    else
        result = notAbove(guest, x86::Float(least), x86::Float(value)) ? value : least;
    guest.setFloat(kTuning + 4 * setting, result);
    x86::reg32 bits;
    std::memcpy(&bits, &result, 4);
    return bits;
}

/* sub_4277d0: the sliders set to the settings. */
x86::reg32 setSliders(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    x86::FPU& fpu = guest.fpu();
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        if (!slider)
            continue;
        const x86::Float least = x86::Float(guest.floatAt(kTuningLeast + 4 * i));
        const x86::Float above = fp::sub(x86::Float(guest.floatAt(kTuning + 4 * i)), least);
        const x86::Float range = fp::sub(x86::Float(guest.floatAt(kTuningMost + 4 * i)), least);
        const float value = float(fp::div(above, range));
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        guest.call(kSliderSetValue, fe::args({ slider.address() }, { bits, 0 }));
    }
    return guest.eax();
}

/* sub_4279b0: the sliders as they stand, kept to tell a change by. */
x86::reg32 rememberSliders(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    for (std::uint32_t i = 0; i < kSettings; ++i)
        guest.setDword(kTuningShown + 4 * i, 0);
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        if (!slider)
            continue;
        guest.call(kSliderValue, fe::args({ slider.address(), 0 }));
        guest.setFloat(kTuningShown + 4 * i, float(guest.fpu().pop()));
    }
    return guest.eax();
}

/* sub_427ac0: whether a slider moved since kTuningShown, in al.  The first
 * slider missing counts as moved; the others are looked at while none has. */
x86::reg32 slidersMoved(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 1;
    x86::reg32 eax = 0;
    std::uint8_t moved = 1;
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        if (i > 0 && moved != 0)
            break;
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        eax = slider.address();
        if (!slider)
            continue;
        guest.call(kSliderValue, fe::args({ slider.address(), 0 }));
        const x86::Float value = guest.fpu().pop();
        guest.fpu().compare(value, x86::Float(guest.floatAt(kTuningShown + 4 * i)));
        eax = (eax & 0xffff0000) | guest.fpu().status.word;   // fnstsw ax
        moved = guest.fpu().status.c3 ? 0 : 1;
    }
    return (eax & 0xffffff00) | moved;
}

/* sub_427c20: each slider told where its moves go, set to its setting and
 * remembered. */
x86::reg32 bindSliders(Guest& guest, x86::reg32 menu)
{
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        if (Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]))
            guest.call(kSliderSetOnMove, fe::args({ slider.address(), kSettingMoved[i] }));
    }
    guest.call(kSetSliders, fe::args({ menu }));
    guest.call(kRememberSliders, fe::args({ menu }));
    return 0;
}

/* sub_427d00: the advanced tuning edited back to the player's own, when the
 * player has one; 0xe when not. */
x86::reg32 loadAdvanced(Guest& guest)
{
    textEntry(false);   // Port: as tools/apply_text_entry.py has the original
    const x86::reg32 player = guest.dword(kTunedPlayer);
    x86::reg32 has = 0;
    if (player == kPlayer1Car)
        has = guest.call(kPlayerHasAdvanced, fe::args({ 0 }));
    else if (player == kPlayer2Car)
        has = guest.call(kPlayerHasAdvanced, fe::args({ 1 }));
    if (has == 0)
        return 0xe;
    if (const x86::reg32 own = advancedOf(guest))
        guest.call(kCopyAdvanced, fe::args({ kAdvancedEdited, own }));
    return 0;
}

/* sub_427d80, the defaults question answered: Yes puts the defaults back. */
x86::reg32 onDefaultsAnswered(Guest& guest, std::uint32_t answer, x86::reg32 close)
{
    if (answer != fe::kDialogEnter)
        return 0;
    if (fe::dialogFocus(guest) == 0)
    {
        for (std::uint32_t i = 0; i < kSettings; ++i)
            guest.setFloat(kTuning + 4 * i, float(x86::Float(guest.floatAt(kTuningDefaults + 4 * i))));
        guest.call(kSetSliders, fe::args({ guest.dword(kTuningMenu) }));
    }
    if (close != 0)
        guest.setByte(close, 1);
    return fe::kDialogHandled;
}

/* sub_427e10 (CEB_DONE, and racenow, exitprogram, optionsnow) and sub_427ea0
 * (CEB_CANCEL): Enter keeps the sliders' tuning when they moved.  Done takes
 * the player's own advanced tuning back into the one edited, when they
 * differ; Cancel keeps the one edited as the player's. */
x86::reg32 onLeaveTuningKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key, bool done)
{
    if (!done)
        textEntry(false);   // Port: as tools/apply_text_entry.py has the original
    if (key == kKeyEnter)
    {
        if ((guest.call(kSlidersMoved, fe::args({ menu })) & 0xff) != 0)
            guest.call(kSaveTuning, fe::args({ tunedCar(guest) }));
        if (const x86::reg32 own = advancedOf(guest))
        {
            if ((guest.call(kAdvancedDiffers, fe::args({ own, kAdvancedEdited })) & 0xff) != 0)
            {
                if (done)
                    guest.call(kLoadAdvanced);
                else
                    guest.call(kCopyAdvanced, fe::args({ own, kAdvancedEdited }));
            }
        }
    }
    return passKey(guest, kButtonKey, item, menu, key);
}

/* sub_427f40, CE_SLCT_CAR's key: another car chosen, the old one's tuning
 * kept if the sliders moved, the new one's loaded. */
x86::reg32 onCarListKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key)
{
    if (guest.call(kShownNow, fe::args({ item, 0x1d })) == 0)
        return 0;
    const x86::reg32 was = tunedCar(guest);
    const x86::reg32 result = passKey(guest, kListKey, item, menu, key);
    if (was != tunedCar(guest))
    {
        if ((guest.call(kSlidersMoved, fe::args({ menu })) & 0xff) != 0)
            guest.call(kSaveTuning, fe::args({ was }));
        guest.call(kLoadTuning, fe::args({ tunedCar(guest) }));
        guest.call(kBindSliders, fe::args({ menu }));
    }
    guest.leaveEbx(result);
    return result;
}

/* sub_427fb0, CEB_DEFAULT's key: Enter asks first, No highlighted. */
x86::reg32 onDefaultsKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key)
{
    if (key != kKeyEnter || item == 0 || menu == 0)
        return 0;
    guest.setDword(kDefaultsLines, fe::text(guest, kDefaultsQuestion));
    guest.setDword(kDefaultsButtons, fe::text(guest, kYes));
    guest.setDword(kDefaultsButtons + 4, fe::text(guest, kNo));
    fe::Question question;
    question.lineCount = 1;
    question.lines = kDefaultsLines;
    question.buttonCount = 2;
    question.buttons = kDefaultsButtons;
    question.buttonHelp = 0;
    question.focus = 1;
    question.column = 0;
    question.callback = kDefaultsAnswered;
    fe::openQuestion(guest, question);
    return fe::kDialogHandled;
}

/* sub_428020: CEB_DONE and CEB_CANCEL their keys, both or neither. */
x86::reg32 bindDoneCancel(Guest& guest, x86::reg32 menu)
{
    textEntry(true);   // Port: as tools/apply_text_entry.py has the original
    if (menu == 0)
        return 0;
    Item done = fe::findItem(guest, menu, kDone);
    Item cancel = fe::findItem(guest, menu, kCancel);
    if (done && cancel)
    {
        guest.setDword(done.address() + kOnKey, kDoneKey);
        guest.setDword(cancel.address() + kOnKey, kCancelKey);
    }
    return cancel.address();
}

/* sub_428060: the screen fitted to whom it is for -- player one's car (1),
 * player two's (2), or 3: the way out of the menus and the list of cars shown
 * only in the first two; nothing of it in a race. */
x86::reg32 fitTuningButtons(Guest& guest, x86::reg32 menu, x86::reg32 tuningFor)
{
    if (menu == 0)
        return 0;
    guest.call(kBindDoneCancel, fe::args({ menu }));
    Item race = fe::findItem(guest, menu, kRaceNow);
    Item exit = fe::findItem(guest, menu, kExitProgram);
    Item options = fe::findItem(guest, menu, kOptionsNow);
    Item list = fe::findItem(guest, menu, kCarList);
    x86::reg32 edx = guest.edx();
    if (race && exit && options)
    {
        for (Item item : { race, exit, options })
            guest.setDword(item.address() + kOnKey, kDoneKey);
        const bool shown = tuningFor == 1 || tuningFor == 2;
        for (Item item : { race, exit, options })
            item.setHidden(!shown);
        if (inRace(guest))
        {
            for (Item item : { race, exit, options })
                item.setHidden(true);
        }
        edx = race.address();
    }
    if (list)
    {
        const x86::reg32 flags = list.address() + fe::item::kFlags;
        if (tuningFor >= 1 && tuningFor <= 3)
        {
            guest.setDword(list.address() + kOnKey, kCarListKey);
            guest.setDword(list.address() + kListName, tuningFor == 2 ? kCars2List : kCarsList);
            const std::uint8_t value = std::uint8_t(tuningFor == 3 ? guest.byte(flags) | fe::item::kHidden
                                                                   : guest.byte(flags) & ~fe::item::kHidden);
            guest.setByte(flags, value);
            if (tuningFor == 1)
                edx = (edx & 0xffff00ff) | (x86::reg32(value) << 8);   // mov dh, ...
        }
        if (inRace(guest))
            list.setHidden(true);
    }
    guest.leaveEdx(edx);
    return list.address();
}

/* sub_4281e0: the tuning screen made ready: whose car, the advanced tuning
 * to edit from the player's own, the settings loaded.  1 when it has no one
 * to tune for. */
x86::reg32 prepareTuning(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kTuningMenu, menu);
    auto forPlayer = [&](x86::reg32 player, x86::reg32 own) {
        guest.setDword(kTunedPlayer, player);
        guest.call(kCopyAdvanced, fe::args({ kAdvancedEdited, own }));
    };
    switch (guest.dword(kTuningFor))
    {
    case 1:
        forPlayer(kPlayer1Car, kAdvancedPlayer1);
        guest.setDword(kTuningForLast, 1);
        break;
    case 2:
        forPlayer(kPlayer2Car, kAdvancedPlayer2);
        guest.setDword(kTuningForLast, 2);
        break;
    case 3:
        forPlayer(kPlayer1Car, kAdvancedPlayer1);
        guest.setDword(kTuningForLast, 3);
        break;
    case 4:
        break;
    default:
    {
        // Back from cedadv: whom it was for last.
        const x86::reg32 last = guest.dword(kTuningForLast);
        if (last == 1 || last == 3)
            forPlayer(kPlayer1Car, kAdvancedPlayer1);
        else if (last == 2)
            forPlayer(kPlayer2Car, kAdvancedPlayer2);
        else
            return 1;
        guest.setDword(kTuningFor, guest.dword(kTuningForLast));
        break;
    }
    }
    guest.call(kLoadTuning, fe::args({ tunedCar(guest) }));
    if (inRace(guest))
        guest.call(kDimBackground, fe::args({}, { kFloat0_6 }));
    const x86::reg32 tuningFor = guest.dword(kTuningFor);
    if (tuningFor >= 1 && tuningFor <= 3)
    {
        guest.call(kFitTuningButtons, fe::args({ menu, tuningFor }));
        return 0;
    }
    return tuningFor == 4 ? 0 : 1;
}

/* sub_428360, cedbasic as it opens. */
x86::reg32 onTuningEnter(Guest& guest, x86::reg32 menu)
{
    x86::reg32 result = guest.call(kPrepareTuning, fe::args({ menu }));
    if (result != 0)
        return result;
    if (Item defaults = fe::findItem(guest, menu, kDefaults))
        guest.setDword(defaults.address() + kOnKey, kDefaultsKey);
    result = guest.call(kBindSliders, fe::args({ menu, kAdvanced.address }));
    if (Item advanced = fe::findItem(guest, menu, kAdvanced))
        advanced.setHidden(true);
    return result;
}

/* sub_4dcaf0: whether two advanced tunings differ, in al: memcmp's answer
 * for the first record that does, its sign in eax. */
x86::reg32 advancedDiffers(Guest& guest, x86::reg32 a, x86::reg32 b)
{
    if (a == 0 || b == 0)
    {
        guest.leaveEdx(a);
        return a & 0xffff0000;
    }
    for (std::uint32_t i = 0; i < kAdvancedRecords; ++i)
    {
        const x86::reg32 offset = kAdvancedRecordSize * i;
        for (x86::reg32 j = 0; j < kAdvancedRecordSize; ++j)
        {
            const std::uint8_t x = guest.byte(a + offset + j);
            const std::uint8_t y = guest.byte(b + offset + j);
            if (x != y)
            {
                guest.leaveEdx(i);
                return x < y ? 0xffffff01 : 0x00000001;
            }
        }
    }
    guest.leaveEdx(kAdvancedRecords);
    return 0;
}

/* sub_4dcb60: one advanced tuning copied over another. */
x86::reg32 copyAdvanced(Guest& guest, x86::reg32 to, x86::reg32 from)
{
    if (to == 0 || from == 0)
        return to;
    for (std::uint32_t i = 0; i < kAdvancedRecords; ++i)
        copyDwords(guest, to + kAdvancedRecordSize * i, from + kAdvancedRecordSize * i, kAdvancedRecordSize);
    guest.leaveEdx(kAdvancedRecords);
    return kAdvancedRecordSize;
}

const GuestString kCar1EnterStrings[] = { kCar1.edit, kPlayerCarTitle, kColourButton, kDownload, kHideThis };
const GuestString kCar2EnterStrings[] = { kCar2.edit, kColourButton, kShowcase };
const GuestString kCar1FrameStrings[] = { kShowcase, kCar1.colours, kColourButton };
const GuestString kCar2FrameStrings[] = { kShowcase, kCar2.colours, kColourButton };
const GuestString kCar1ColourStrings[] = { kCar1.model, kCar1.colours };
const GuestString kCar2ColourStrings[] = { kCar2.model, kCar2.colours };
const GuestString kRecorderStrings[] = { kCarFolderName };
const GuestString kCarFrameStrings[] = { kCarFrameName };
const GuestString kTrackFrameStrings[] = { kTrackFrameName };
const GuestString kOpponentCarStrings[] = { kShowcase, kCompare };
const GuestString kOpponentArtStrings[] = { kBack, kMask, kCarPicture };
const GuestString kOpponentFrameStrings[] = { kTrafficButton, kTrafficList, kOpponentCount, kOpponentCountList };
const GuestString kSettingStrings[] = {
    kSettingCodelinks[0], kSettingCodelinks[1], kSettingCodelinks[2], kSettingCodelinks[3],
    kSettingCodelinks[4], kSettingCodelinks[5], kSettingCodelinks[6],
};
const GuestString kDoneCancelStrings[] = { kDone, kCancel };
const GuestString kTuningButtonStrings[] = { kRaceNow, kExitProgram, kOptionsNow, kCarList };
const GuestString kTuningEnterStrings[] = { kDefaults, kAdvanced };

template <typename Body>
void wholeForMenu(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, menu); });
}

template <typename Body>
void wholeForKey(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 item = cpu.eax;
    const x86::reg32 menu = cpu.edx;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, item, menu, key); });
}

constexpr std::uint8_t kNone = 0;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEcxEdxEsi = fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi;
constexpr std::uint8_t kAllButEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi;
constexpr std::uint8_t kAllButEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kAllButEbx = fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi;

}

/* Each stand-in: the generated function, what its ret pops, what its prologue
 * pushes, the strings it hands to the game and the registers it pushes. */

void frontend::carMenuMusicBack(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4104b0", 0, 16, nullptr, 0, kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return menuMusicBack(guest); });
}

void frontend::carSaveFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4267a0", 0, 8 + 0x12c, kCarFrameStrings, std::size(kCarFrameStrings), fe::kRestoreEbx };
    const x86::reg32 folder = cpu.eax, first = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return saveFrame(guest, kCarFrameName, folder, first); });
}

void frontend::trackSaveFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4267f0", 0, 8 + 0x12c, kTrackFrameStrings, std::size(kTrackFrameStrings),
                          fe::kRestoreEbx };
    const x86::reg32 folder = cpu.eax, first = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return saveFrame(guest, kTrackFrameName, folder, first); });
}

void frontend::carRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426870", 0, 24 + 0x134, kRecorderStrings, std::size(kRecorderStrings) };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordCar(guest); });
}

void frontend::trackRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    static const GuestString strings[] = { kTrackFolderName };
    static fe::Site site{ "sub_426b00", 0, 16 + 0x134, strings, std::size(strings),
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return recordTrack(guest); });
}

void frontend::car1OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426d60", 0, 16, kCar1EnterStrings, std::size(kCar1EnterStrings), kEcxEdxEsi };
    wholeForMenu(app, cpu, site, onCar1Enter);
}

void frontend::car2OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426ef0", 0, 16, kCar2EnterStrings, std::size(kCar2EnterStrings), kEcxEdxEsi };
    wholeForMenu(app, cpu, site, onCar2Enter);
}

void frontend::car1OnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426e20", 0, 16, kCar1FrameStrings, std::size(kCar1FrameStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, [](Guest& g, x86::reg32 menu) { return onCarFrame(g, menu, kCar1); });
}

void frontend::car2OnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426f80", 0, 16, kCar2FrameStrings, std::size(kCar2FrameStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, [](Guest& g, x86::reg32 menu) { return onCarFrame(g, menu, kCar2); });
}

void frontend::car1OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_426ee0: nothing to undo.
    static fe::Site site{ "sub_426ee0", 0, 4, nullptr, 0, kNone };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::car2OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_427040: likewise.
    static fe::Site site{ "sub_427040", 0, 4, nullptr, 0, kNone };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::car1EditClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426c20", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onEditClicked(guest, 1); });
}

void frontend::car2EditClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426c40", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onEditClicked(guest, 2); });
}

void frontend::car1ColourClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426c60", 0, 16, kCar1ColourStrings, std::size(kCar1ColourStrings), kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onColourClicked(guest, kCar1); });
}

void frontend::car2ColourClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426ce0", 0, 16, kCar2ColourStrings, std::size(kCar2ColourStrings), kEbxEcxEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onColourClicked(guest, kCar2); });
}

void frontend::opponentFitCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427050", 0, 16, kOpponentCarStrings, std::size(kOpponentCarStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, fitOpponentCar);
}

void frontend::opponentFitArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4270c0", 0, 20, kOpponentArtStrings, std::size(kOpponentArtStrings), kAllButEsi };
    wholeForMenu(app, cpu, site, fitOpponentArt);
}

void frontend::opponentOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4272a0", 0, 16, nullptr, 0, kEcxEdxEsi };
    wholeForMenu(app, cpu, site, onOpponentEnter);
}

void frontend::opponentOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4272e0: nothing to undo.
    static fe::Site site{ "sub_4272e0", 0, 4, nullptr, 0, kNone };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::opponentOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427190", 0, 20, kOpponentFrameStrings, std::size(kOpponentFrameStrings), kAllButEsi };
    wholeForMenu(app, cpu, site, onOpponentFrame);
}

void frontend::tuningLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4272f0", 0, 20, nullptr, 0, kAllButEbx };
    wholeForMenu(app, cpu, site, loadTuning);
}

void frontend::tuningSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427370", 0, 20, nullptr, 0, kAllButEbx };
    wholeForMenu(app, cpu, site, saveTuning);
}

namespace
{
template <std::uint32_t Setting>
void settingMoved(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site)
{
    const x86::reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onSettingMoved(guest, item, Setting); });
}
}

void frontend::tuningEngineMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4273e0", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<0>(app, cpu, site);
}

void frontend::tuningBrakesMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427470", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<1>(app, cpu, site);
}

void frontend::tuningSteeringMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427500", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<2>(app, cpu, site);
}

void frontend::tuningGearsMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427590", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<3>(app, cpu, site);
}

void frontend::tuningSuspensionMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427620", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<4>(app, cpu, site);
}

void frontend::tuningAeroMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4276b0", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<5>(app, cpu, site);
}

void frontend::tuningTiresMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427740", 0, 8 + 0x1c, nullptr, 0, fe::kRestoreEdx };
    settingMoved<6>(app, cpu, site);
}

void frontend::tuningSetSliders(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4277d0", 0, 12 + 0xc, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, setSliders);
}

void frontend::tuningRememberSliders(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4279b0", 0, 12, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, rememberSliders);
}

void frontend::tuningSlidersMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427ac0", 0, 16, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, slidersMoved);
}

void frontend::tuningBindSliders(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427c20", 0, 16, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, bindSliders);
}

void frontend::tuningLoadAdvanced(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427d00", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return loadAdvanced(guest); });
}

void frontend::tuningDefaultsAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427d80", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const std::uint32_t answer = cpu.eax;
    const x86::reg32 close = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onDefaultsAnswered(guest, answer, close); });
}

void frontend::tuningDoneKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427e10", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onLeaveTuningKey(g, i, m, k, true);
    });
}

void frontend::tuningCancelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427ea0", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onLeaveTuningKey(g, i, m, k, false);
    });
}

void frontend::tuningCarListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427f40", 0, 16, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    wholeForKey(app, cpu, site, onCarListKey);
}

void frontend::tuningDefaultsKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_427fb0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, onDefaultsKey);
}

void frontend::tuningBindDoneCancel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_428020", 0, 12, kDoneCancelStrings, std::size(kDoneCancelStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx };
    wholeForMenu(app, cpu, site, bindDoneCancel);
}

void frontend::tuningFitButtons(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_428060", 0, 28, kTuningButtonStrings, std::size(kTuningButtonStrings), kAllButEdx };
    const x86::reg32 menu = cpu.eax;
    const x86::reg32 tuningFor = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return fitTuningButtons(guest, menu, tuningFor); });
}

void frontend::tuningPrepare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4281e0", 0, 24, nullptr, 0 };
    wholeForMenu(app, cpu, site, prepareTuning);
}

void frontend::tuningOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_428360", 0, 16, kTuningEnterStrings, std::size(kTuningEnterStrings), kEbxEcxEdx };
    wholeForMenu(app, cpu, site, onTuningEnter);
}

void frontend::tuningOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4283b0: no one to tune for any more.
    static fe::Site site{ "sub_4283b0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.setDword(kTuningFor, 0);
        return x86::reg32(0);
    });
}

void frontend::tuningAdvancedDiffers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dcaf0", 0, 28, nullptr, 0, kAllButEdx };
    const x86::reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return advancedDiffers(guest, a, b); });
}

void frontend::tuningCopyAdvanced(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dcb60", 0, 24, nullptr, 0, kAllButEdx };
    const x86::reg32 to = cpu.eax, from = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return copyAdvanced(guest, to, from); });
}

}
