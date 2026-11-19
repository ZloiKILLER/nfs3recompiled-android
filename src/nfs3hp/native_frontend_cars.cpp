#include "native_frontend_cars.h"
#include "native_load_carmodels.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_frontend_eaa.h"
#include "native_frontend_glue.h"
#include "native_frontend_loada.h"
#include "native_frontend_loop.h"
#include "native_frontend_menub.h"
#include "native_frontend_misc.h"
#include "native_frontend_wata.h"
#include "native_load_cardb.h"
#include "port_buttons.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_sound_music.h"
#include "port_text.h"
#include <cstring>
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
 * frame by frame to c:\NN\NNNN.tga (sub_426870, sub_426b00).
 *
 * Plan step 5: each function a typed core in frontend::cars (native_frontend_cars.h), in a frame
 * of its own where the game's function has one (fe::Frame), and a thunk under the game's name
 * for the calls by address.  A core of this file calls another as C++ (guest::direct); the rest of
 * the game still by address. */
namespace nfs3hp
{

/* Port: the phone's keyboard, up while the game takes typing (nfs3hp_main.cpp,
 * the port's text entry change). */
void textEntry(bool taking);

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kInRace = 0x559234;          // the menus are a race's pause screens
constexpr std::uint32_t kHotPursuit = 3;

constexpr nfs::u32 kOnKey = 0x30;
constexpr nfs::u32 kListName = 0x64;            // a list's: the name of its entries (a button's kOnClick)
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr nfs::u32 kGoOn = 5;                   // to the selected button's nextmenu (sub_44b230)
// The screen closes without fading out, and opens again (sub_44b230).
constexpr nfs::u32 kReopen = 9;

// The menu engine's and the widgets'.
constexpr nfs::u32 kButtonKey = 0x461270;
constexpr nfs::u32 kListKey = 0x463ff0;
constexpr nfs::u32 kSliderValue = 0x46c9b0;     // eax the item, edx the end; the value on the x87
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;   // 0.6f
constexpr nfs::u32 kSprintf = 0x4df690;

constexpr nfs::u32 kShowcaseDownload = 0x452e60; // CAR_DOWNLOAD's click, in the original

/* A greyed item, and greyed and hidden, the state and flags bytes as one
 * word. */
void grey(Item item, bool greyed)
{
    item.setDisabled(greyed);
}

void greyAndHide(Guest& guest, Item item, bool greyedAndHidden)
{
    const nfs::u32 state = item.address() + fe::item::kState;
    const std::uint16_t word = guest.word(state);
    guest.setWord(state, std::uint16_t(greyedAndHidden ? word | 0x1001 : word & 0xeffe));
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

nfs::u32 passKey(Guest& guest, nfs::u32 handler, nfs::u32 item, nfs::u32 menu, std::uint16_t key)
{
    return callAt(guest, handler, fe::args({ item, menu, nfs::u32(std::int32_t(std::int16_t(key))) }));
}

/* ---------------------------------------------------------------------------
 * The music, back to the menus'
 * ------------------------------------------------------------------------- */

// Set while a music other than the menus' plays (a car's, sub_4104f0).
constexpr nfs::u32 kOtherMusic = 0x552594;
// The sound setting 6 plays no menu music.
constexpr std::uint32_t kNoMenuMusic = 6;
constexpr std::uint32_t kMenuTune = 0x16;

}  // namespace

/* sub_4104b0: as a car screen opens, the menus' music again if another plays. */
nfs::u32 frontend::cars::menuMusicBack(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (guest.dword(kOtherMusic) != 0)
    {
        guest.setDword(kOtherMusic, 0);
        if (gameSettings(guest).menuMusicAsked != kNoMenuMusic)
        {
            fe::direct<0x4109d0, port::musicStopped>(app, ts, ts.sp - 4, eax);
            return nfs::u32(*fe::direct<0x4104f0, port::musicStarted>(
                app, ts, ts.sp - 4, std::int32_t(kMenuTune), std::int32_t(gameSettings(guest).menuMusicVolume), 0));
        }
    }
    return eax;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The developer's recorder
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kDeveloperFlags = 0x55eb2f;  // byte
constexpr std::uint8_t kRecordFrames = 0x40;
// The 3D view ready to be recorded: the screen's model, the display.
constexpr nfs::u32 kModelReady = 0x564850;      // byte
constexpr nfs::u32 kDisplayReady = 0x564831;    // byte
// Where it stands: 0 not started; 1 and 2, the frames of a turn, one shot
// every other pass; 3 to 8 the next turns, with the view's flags.
constexpr nfs::u32 kRecordStep = 0x553038;
constexpr nfs::u32 kRecordTurn = 0x55302c;      // float: the model's turn, 1/32 a frame
constexpr nfs::u32 kRecordFrame = 0x5e10d0;     // 0 to 32 in a turn
constexpr nfs::u32 kRecordShot = 0x55303c;      // the next file's number in its hundred
constexpr nfs::u32 kRecordViewA = 0x553030;     // read by the 3D view
constexpr nfs::u32 kRecordViewB = 0x553034;
constexpr std::int32_t kFramesInTurn = 31;
constexpr double kTurnStep = 0.03125;             // at 0x536f94 and 0x536fac

constexpr nfs::u32 kMakeFolder = 0x4eb1d0;      // eax a path

constexpr GuestString kCarFrameName{ 0x536f64, "c:\\%02d\\%04d.tga" };
constexpr GuestString kTrackFrameName{ 0x536f78, "c:\\T%02d\\%04d.tga" };
constexpr GuestString kCarFolderName{ 0x536f8c, "c:\\%02d" };
constexpr GuestString kTrackFolderName{ 0x536f9c, "c:\\T%02d" };

/* sub_4267a0, sub_4267f0: the screen saved as the next frame, `first` and
 * on, in the folder of `folder`. */
nfs::u32 saveFrame(App* app, ThreadState& ts, const GuestString& format, nfs::u32 folder, nfs::u32 first)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0x12c);
    const nfs::u32 name = guest.frame();    // char[0x12c]
    const nfs::u32 number = first + guest.dword(kRecordShot);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, format.address, guest::Words{folder, number});
    const nfs::u32 next = guest.dword(kRecordShot) + 1;
    guest.setDword(kRecordShot, next);
    fe::direct<0x4ead20, frontend::wata::windowApply>(app, ts);
    fe::direct<0x4eb160, frontend::eaa::bitmapSaveTga>(app, ts, name);
    return fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, ts);
}

}  // namespace

nfs::u32 frontend::cars::saveCarFrame(App* app, ThreadState& ts, u32 folder, u32 first)
{
    return saveFrame(app, ts, kCarFrameName, folder, first);
}

nfs::u32 frontend::cars::saveTrackFrame(App* app, ThreadState& ts, u32 folder, u32 first)
{
    return saveFrame(app, ts, kTrackFrameName, folder, first);
}

namespace
{

/* The turn moved on by a frame; true at its end. */
bool nextFrame(Guest& guest)
{
    const nfs::u32 frame = guest.dword(kRecordFrame) + 1;
    guest.setDword(kRecordFrame, frame);
    guest.setFloat(kRecordTurn, float(fp::add(nfs::Float(guest.floatAt(kRecordTurn)), nfs::Float(kTurnStep))));
    return std::int32_t(frame) > kFramesInTurn;
}

/* A turn's frame for player one's car, from the hundred `first`.  The
 * original hands the car's id lookup edx as it has it (100, 200 or 7) and the
 * id's number `first` in edx; neither takes it. */
nfs::u32 recordCarFrame(Guest& guest, ThreadState& ts, nfs::u32 first)
{
    App* const app = guest.app();
    const nfs::u32 data = *fe::direct<0x439520, load::idOf>(app, ts, gameSettings(guest).players[0].car);
    const nfs::u32 folder = fe::direct<0x439660, load::numberOf>(app, ts, data);
    return guest::direct<0x4267a0, frontend::cars::saveCarFrame>(app, ts, folder, first);
}

bool recording(const Guest& guest)
{
    return (guest.byte(kDeveloperFlags) & kRecordFrames) != 0;
}

}  // namespace

/* sub_426870, on every pass of car1 and car2: player one's car turning,
 * three turns -- 100 on, 200 on with the view's first flag, 300 on with its
 * second -- a frame every other pass. */
nfs::u32 frontend::cars::recordCar(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x134);
    if (!recording(guest))
        return eax;
    const nfs::u32 step = guest.dword(kRecordStep);
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
        // The original hands the id lookup 1 in edx, which it does not take.
        const nfs::u32 data = *fe::direct<0x439520, load::idOf>(app, ts, gameSettings(guest).players[0].car);
        const nfs::u32 folder = fe::direct<0x439660, load::numberOf>(app, ts, data);
        const nfs::u32 name = guest.frame();    // char[0x134]
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kCarFolderName.address, guest::Words{folder});
        return fe::direct<kMakeFolder, watcom::clib::mkdir>(guest.app(), guest.ts(), name);
    }
    case 1:
    case 4:
    case 7:
        guest.setDword(kRecordStep, step + 1);
        return step;
    case 2:
    {
        guest.setDword(kRecordStep, 1);
        const nfs::u32 result = recordCarFrame(guest, ts, 100);
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
        const nfs::u32 result = recordCarFrame(guest, ts, 200);
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
        const nfs::u32 result = recordCarFrame(guest, ts, 300);
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
nfs::u32 frontend::cars::recordTrack(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x134);
    if (!recording(guest))
        return eax;
    const nfs::u32 step = guest.dword(kRecordStep);
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
        const nfs::u32 name = guest.frame();    // char[0x134]
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kTrackFolderName.address, guest::Words{gameSettings(guest).track});
        return fe::direct<kMakeFolder, watcom::clib::mkdir>(guest.app(), guest.ts(), name);
    }
    if (step == 1)
    {
        guest.setDword(kRecordStep, 2);
        return step;
    }
    if (step != 2)
        return step;
    guest.setDword(kRecordStep, 1);
    nfs::u32 result = guest::direct<0x4267f0, saveTrackFrame>(app, ts, gameSettings(guest).track, 100);
    if (nextFrame(guest))
    {
        guest.setDword(kRecordStep, 0);
        result = 0;
    }
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * car1 and car2
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kTuningFor = 0x553048;       // who the tuning screen is for: 1 player one, 2 player two
constexpr nfs::u32 kColourList = 0xb3;           // byte of the 3D model's item: its colours changed

constexpr std::uint32_t kPlayer1CarTitle = 0x1aa;  // two players on this machine
constexpr std::uint32_t kYourCarTitle = 0x102;

/* A player's screen: its car, its codelinks and the handlers it hands out. */
struct CarScreen
{
    std::uint32_t player;
    GuestString edit;            // CE_EDITPn, to the tuning
    GuestString model;           // CAR_SELECT_OBJ3DPn
    GuestString colours;         // CAR_COLOR_SELECT_Pn
    nfs::u32 editClicked;
    nfs::u32 colourClicked;
};

constexpr GuestString kPlayerCarTitle{ 0x537010, "playerCarTitle" };
constexpr GuestString kColourButton{ 0x537020, "CAR_COLOR_CHANGE_BUTTON" };
constexpr GuestString kDownload{ 0x537038, "CAR_DOWNLOAD" };
constexpr GuestString kHideThis{ 0x537048, "hidethis" };
constexpr GuestString kShowcase{ 0x537054, "showcase" };
constexpr GuestString kCompare{ 0x53706c, "compare" };

constexpr CarScreen kCar1{ 0, { 0x537004, "CE_EDITP1" }, { 0x536fb4, "CAR_SELECT_OBJ3DP1" },
                           { 0x536fc8, "CAR_COLOR_SELECT_P1" }, 0x426c20, 0x426c60 };
constexpr CarScreen kCar2{ 1, { 0x537060, "CE_EDITP2" }, { 0x536fdc, "CAR_SELECT_OBJ3DP2" },
                           { 0x536ff0, "CAR_COLOR_SELECT_P2" }, 0x426c40, 0x426ce0 };

/* The tuning button: not in a network race, nor in a series. */
void fitEditButton(Guest& guest, nfs::u32 menu, const CarScreen& screen)
{
    if (Item edit = fe::findItem(guest, menu, screen.edit))
    {
        const std::uint32_t type = gameSettings(guest).raceType;
        if (std::int32_t(gameSettings(guest).playMode) > 1 || type == 1 || type == 2)
            grey(edit, true);
        else
            edit.setOnClick(screen.editClicked);
    }
}

/* The showcase's button, for a car that has one. */
void fitShowcase(Guest& guest, ThreadState& ts, nfs::u32 menu, const CarScreen& screen)
{
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
        grey(showcase, fe::direct<0x4530c0, frontend::menub::carArtExists>(
                           guest.app(), ts, gameSettings(guest).players[screen.player].car) == 0);
}

}  // namespace

/* sub_426d60, car1 as it opens. */
nfs::u32 frontend::cars::car1Enter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest::direct<0x4104b0, menuMusicBack>(app, ts, 0);  // its result is not read
    fitEditButton(guest, menu, kCar1);
    if (Item title = fe::findItem(guest, menu, kPlayerCarTitle))
        title.setText(fe::text(guest, gameSettings(guest).playMode == 1 ? kPlayer1CarTitle : kYourCarTitle));
    if (Item colour = fe::findItem(guest, menu, kColourButton))
        colour.setOnClick(kCar1.colourClicked);
    // Port: Download Car hidden, as hidethis is -- there is nothing left to
    // download (the port's hidden download change); the original gives it its
    // click, kShowcaseDownload.
    if (Item download = fe::findItem(guest, menu, kDownload))
        download.setHidden(true);
    if (Item hidden = fe::findItem(guest, menu, kHideThis))
        hidden.setHidden(true);
    return 0;
}

/* sub_426ef0, car2 as it opens. */
nfs::u32 frontend::cars::car2Enter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest::direct<0x4104b0, menuMusicBack>(app, ts, 0);  // its result is not read
    fitEditButton(guest, menu, kCar2);
    if (Item colour = fe::findItem(guest, menu, kColourButton))
        colour.setOnClick(kCar2.colourClicked);
    fitShowcase(guest, ts, menu, kCar2);
    return 0;
}

namespace
{

/* Whether a car's colour can be changed: it has more than one. */
bool oneColour(Guest& guest, ThreadState& ts, std::uint32_t player)
{
    App* const app = guest.app();
    const nfs::u32 car = gameSettings(guest).players[player].car;
    return *fe::direct<0x4396b0, load::policeFlag>(app, ts, car) != 0
           || *fe::direct<0x439760, port::fields::colourCount>(app, ts, ts.sp - 4, car) == 1;
}

/* sub_426e20, sub_426f80: car1's and car2's every pass. */
nfs::u32 onCarFrame(App* app, ThreadState& ts, nfs::u32 menu, const CarScreen& screen)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest::direct<0x426870, frontend::cars::recordCar>(app, ts, 0);  // its result is not read
    fitShowcase(guest, ts, menu, screen);
    if (Item colours = fe::findItem(guest, menu, screen.colours))
        greyAndHide(guest, colours, oneColour(guest, ts, screen.player));
    if (Item button = fe::findItem(guest, menu, kColourButton))
        grey(button, oneColour(guest, ts, screen.player));
    return 0;
}

/* sub_426c20, sub_426c40: CE_EDITPn clicked, the tuning for its player. */
nfs::u32 onEditClicked(App* app, ThreadState& ts, nfs::u32 player)
{
    Guest guest(app, ts);
    guest.setDword(kTuningFor, player);
    return kGoOn;
}

/* sub_426c60, sub_426ce0: the colour button clicked, the car's next colour,
 * the model and the list told. */
nfs::u32 onColourClicked(App* app, ThreadState& ts, const CarScreen& screen)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const std::int32_t player = std::int32_t(screen.player);
    const auto car = [&] { return gameSettings(guest).players[screen.player].car; };
    const std::int32_t colour = *fe::direct<0x439790, port::fields::chosenColour>(app, ts, ts.sp - 4, car(), player);
    const std::int32_t count = *fe::direct<0x439760, port::fields::colourCount>(app, ts, ts.sp - 4, car());
    if (colour >= 0 && count > 0)
    {
        const std::int32_t next = (colour + 1) % count;
        fe::direct<0x4397d0, port::fields::colourSet>(app, ts, ts.sp - 4, car(), player, next);
        const auto currentMenu = [&] { return fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts); };
        if (Item model = fe::findItem(guest, currentMenu(), screen.model))
            guest.setByte(model.address() + kColourList, 1);
        if (Item colours = fe::findItem(guest, currentMenu(), screen.colours))
            fe::direct<0x456a20, port::fields::listRefreshed>(app, ts, ts.sp - 4, colours.address());
    }
    return fe::kDialogHandled;
}

}  // namespace

nfs::u32 frontend::cars::car1Frame(App* app, ThreadState& ts, u32 menu)
{
    return onCarFrame(app, ts, menu, kCar1);
}

nfs::u32 frontend::cars::car2Frame(App* app, ThreadState& ts, u32 menu)
{
    return onCarFrame(app, ts, menu, kCar2);
}

nfs::u32 frontend::cars::car1EditClicked(App* app, ThreadState& ts)
{
    return onEditClicked(app, ts, 1);
}

nfs::u32 frontend::cars::car2EditClicked(App* app, ThreadState& ts)
{
    return onEditClicked(app, ts, 2);
}

nfs::u32 frontend::cars::car1ColourClicked(App* app, ThreadState& ts)
{
    return onColourClicked(app, ts, kCar1);
}

nfs::u32 frontend::cars::car2ColourClicked(App* app, ThreadState& ts)
{
    return onColourClicked(app, ts, kCar2);
}

namespace
{

/* ---------------------------------------------------------------------------
 * opponent
 * ------------------------------------------------------------------------- */

/* The opponents: 1 to 3 the number of them, 0 none; past 3 every one is the
 * same car, the setting less 4. */
constexpr std::int32_t kOpponentCarBase = 4;
// The number of opponents the screen's art was laid out for.
constexpr nfs::u32 kOpponentsShown = 0x553040;

// The art: the opponents' picture, opp1.qfs to opp4.qfs -- the number written
// into the game's own string -- or the car's.
constexpr nfs::u32 kOpponentPicture = 0x537090;  // "oppN.qfs", N at +3
constexpr GuestString kCarPicture{ 0x537084, "carbg.qfs" };
constexpr nfs::u32 kPictureFile = 0x40;          // an item's: the picture it shows
constexpr nfs::u32 kLayer = 0x0a;                // int16

constexpr GuestString kBack{ 0x537074, "back" };
constexpr GuestString kMask{ 0x53707c, "mask" };
constexpr GuestString kTrafficButton{ 0x53709c, "TRAFFIC_BUTTON" };
constexpr GuestString kTrafficList{ 0x5370ac, "TRAFFIC_CASCADE" };
constexpr GuestString kOpponentCount{ 0x5370bc, "OPPNUM" };
constexpr GuestString kOpponentCountList{ 0x5370c4, "OPPNUM_CASCADE" };

/* The number of opponents, 0 for one car. */
std::int32_t opponentCount(const Guest& guest)
{
    const std::int32_t opponents = std::int32_t(gameSettings(guest).opponentClass);
    return opponents > 3 || opponents == 0 ? 0 : opponents;
}

}  // namespace

/* sub_427050: the showcase and the comparison, for one car. */
nfs::u32 frontend::cars::opponentFitCar(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
    {
        const std::int32_t opponents = std::int32_t(gameSettings(guest).opponentClass);
        if (opponents > 3)
            grey(showcase, fe::direct<0x4530c0, frontend::menub::carArtExists>(
                               app, ts, nfs::u32(opponents - kOpponentCarBase)) == 0);
        else
            grey(showcase, true);
    }
    Item compare = fe::findItem(guest, menu, kCompare);
    if (compare)
        grey(compare, std::int32_t(gameSettings(guest).opponentClass) <= 3);
    return compare.address();
}

/* sub_4270c0: the background and its mask, the opponents' picture or the
 * car's. */
nfs::u32 frontend::cars::opponentFitArt(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    const nfs::u32 back = fe::findItem(guest, menu, kBack).address();
    const nfs::u32 mask = fe::findItem(guest, menu, kMask).address();
    const std::int32_t opponents = std::int32_t(gameSettings(guest).opponentClass);
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
    const std::uint8_t digit = std::uint8_t(std::uint8_t(gameSettings(guest).opponentClass) + '0');
    guest.setByte(kOpponentPicture + 3, digit);
    nfs::u32 result = (mask & 0xffffff00) | digit;
    if (opponents == 3)
    {
        // With the bonus car three opponents look like four.
        result = fe::direct<0x439ce0, frontend::misc::creditsHaveBonusCar>(app, ts);
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
nfs::u32 frontend::cars::opponentEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest::direct<0x4270c0, opponentFitArt>(app, ts, menu);
    guest.setDword(kOpponentsShown, nfs::u32(opponentCount(guest)));
    guest::direct<0x427050, opponentFitCar>(app, ts, menu);
    return 0;
}

/* sub_427190, opponent on every pass: traffic only in a single race that has
 * it, the number of opponents not in Hot Pursuit; a change of the number
 * opens the screen again, for its art. */
nfs::u32 frontend::cars::opponentFrame(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    guest::direct<0x4104b0, menuMusicBack>(app, ts, 0);  // its result is not read
    auto trafficOff = [&]() {
        return gameSettings(guest).raceType != 0 || gameSettings(guest).trafficLevel == 2;
    };
    if (Item button = fe::findItem(guest, menu, kTrafficButton))
    {
        if (gameSettings(guest).raceType != kHotPursuit)
            grey(button, trafficOff());
    }
    if (Item list = fe::findItem(guest, menu, kTrafficList))
    {
        if (gameSettings(guest).raceType != kHotPursuit)
            greyAndHide(guest, list, trafficOff());
    }
    if (Item count = fe::findItem(guest, menu, kOpponentCount))
        grey(count, gameSettings(guest).raceType == kHotPursuit);
    if (Item list = fe::findItem(guest, menu, kOpponentCountList))
        greyAndHide(guest, list, gameSettings(guest).raceType == kHotPursuit);
    guest::direct<0x427050, opponentFitCar>(app, ts, menu);
    return nfs::u32(opponentCount(guest)) != guest.dword(kOpponentsShown) ? kReopen : 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * cedbasic: a car's tuning
 * ------------------------------------------------------------------------- */

/* Seven settings, each a float between its least and its most, edited as the
 * screen's sliders (0 to 1) show them.  The player's own are the game's
 * (sub_4dc550, a record for each player); a car with none has the defaults. */
constexpr std::uint32_t kSettings = 7;
constexpr nfs::u32 kTuning = 0x5e10e0;           // the settings edited
constexpr nfs::u32 kTuningDefaults = 0x55d024;
constexpr nfs::u32 kTuningMost = 0x55d040;
constexpr nfs::u32 kTuningLeast = 0x55d05c;
constexpr nfs::u32 kTuningShown = 0x5e10fc;      // the sliders as the screen opened
constexpr std::uint32_t kTuningBytes = 0x1c;

/* The advanced tuning, cedadv's: 50 records of 0x30 bytes, the one edited and
 * each player's own. */
constexpr nfs::u32 kAdvancedEdited = 0x8b9b60;
constexpr nfs::u32 kAdvancedPlayer1 = 0x8b9200;
constexpr nfs::u32 kAdvancedPlayer2 = 0x8b88a0;
constexpr std::uint32_t kAdvancedRecords = 50;
constexpr nfs::u32 kAdvancedRecordSize = 0x30;

// The player whose car is tuned: kPlayer1Car or kPlayer2Car.
constexpr nfs::u32 kTunedPlayer = 0x55304c;
constexpr nfs::u32 kTuningForLast = 0x553044;    // kTuningFor as the screen last opened
constexpr nfs::u32 kTuningMenu = 0x553050;
// The question before the defaults: its line and two buttons.
constexpr nfs::u32 kDefaultsLines = 0x553054;
constexpr nfs::u32 kDefaultsButtons = 0x553058;
constexpr std::uint32_t kDefaultsQuestion = 0x1e9;
constexpr std::uint32_t kYes = 0xc4;
constexpr std::uint32_t kNo = 0xc5;
// The lists of cars CE_SLCT_CAR shows, by their names.
constexpr nfs::u32 kCarsList = 0x553060;         // "cars"
constexpr nfs::u32 kCars2List = 0x553065;        // "car2"

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

constexpr nfs::u32 kSettingMoved[kSettings] = { 0x4273e0, 0x427470, 0x427500, 0x427590,
                                                  0x427620, 0x4276b0, 0x427740 };
constexpr nfs::u32 kDefaultsAnswered = 0x427d80;
constexpr nfs::u32 kDoneKey = 0x427e10;
constexpr nfs::u32 kCancelKey = 0x427ea0;
constexpr nfs::u32 kCarListKey = 0x427f40;
constexpr nfs::u32 kDefaultsKey = 0x427fb0;

/* The tuned player's record of the advanced tuning, 0 for neither. */
nfs::u32 advancedOf(const Guest& guest)
{
    const nfs::u32 player = guest.dword(kTunedPlayer);
    return player == SETTINGS_ADDRESS(players[0].car) ? kAdvancedPlayer1 : player == SETTINGS_ADDRESS(players[1].car) ? kAdvancedPlayer2 : 0;
}

/* The car tuned. */
nfs::u32 tunedCar(const Guest& guest)
{
    return guest.dword(guest.dword(kTunedPlayer));
}

/* The player's tuning record for a car, 0 for none. */
nfs::u32 playerTuning(Guest& guest, ThreadState& ts, nfs::u32 car)
{
    nfs::u32 data = 0;
    if (fe::direct<0x439a50, load::recordAt>(guest.app(), ts, car) != 0)
        data = *fe::direct<0x439520, load::idOf>(guest.app(), ts, car);
    if (data == 0)
        return 0;
    const nfs::u32 player = guest.dword(kTunedPlayer);
    if (player == SETTINGS_ADDRESS(players[0].car))
        return fe::direct<0x4dc550, frontend::loada::tuningOf>(guest.app(), ts, 0u, data);
    if (player == SETTINGS_ADDRESS(players[1].car))
        return fe::direct<0x4dc550, frontend::loada::tuningOf>(guest.app(), ts, 1u, data);
    return 0;
}

/* `count` bytes, a dword at a time, as rep movsd copies them. */
void copyDwords(Guest& guest, nfs::u32 to, nfs::u32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; i += 4)
        guest.setDword(to + i, guest.dword(from + i));
}

}  // namespace

/* sub_4272f0: a car's tuning to edit, the player's or the defaults. */
nfs::u32 frontend::cars::loadTuning(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    nfs::u32 from = playerTuning(guest, ts, car);
    if (from == 0)
        from = kTuningDefaults;
    copyDwords(guest, kTuning, from, kTuningBytes);
    return kTuningBytes;
}

/* sub_427370: the tuning edited, to the player's record for a car. */
nfs::u32 frontend::cars::saveTuning(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    nfs::u32 data = 0;
    if (fe::direct<0x439a50, load::recordAt>(app, ts, car) != 0)
        data = *fe::direct<0x439520, load::idOf>(app, ts, car);
    if (data == 0)
        return data;   // eax as the last call left it: 0
    const nfs::u32 player = guest.dword(kTunedPlayer);
    nfs::u32 to = 0;
    if (player == SETTINGS_ADDRESS(players[0].car))
        to = fe::direct<0x4dc550, frontend::loada::tuningOf>(app, ts, 0u, data);
    else if (player == SETTINGS_ADDRESS(players[1].car))
        to = fe::direct<0x4dc550, frontend::loada::tuningOf>(app, ts, 1u, data);
    else
        return data;
    if (to == 0)
        return 0;
    copyDwords(guest, to, kTuning, kTuningBytes);
    return kTuningBytes;
}

namespace
{

/* What jbe reads after fcom a, b: a not above b, or unordered. */
bool notAbove(Guest& guest, const nfs::Float& a, const nfs::Float& b)
{
    return !(a > b);
}

/* fcom a, b and fnstsw: the status word's condition bits (c0 below or unordered, c2 unordered, c3
 * equal or unordered); the rest of the word is as the x87 starts with it. */
std::uint16_t compareStatusWord(const nfs::Float& a, const nfs::Float& b)
{
    if (a == b)
        return 0x4000;
    if (a > b)
        return 0;
    if (a < b)
        return 0x0100;
    return 0x4500;
}

/* sub_4273e0 and the six after it: a setting's slider moved, the setting
 * from its least to its most. */
nfs::u32 onSettingMoved(App* app, ThreadState& ts, nfs::u32 item, std::uint32_t setting)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0x1c);
    const float least = guest.floatAt(kTuningLeast + 4 * setting);
    const float most = guest.floatAt(kTuningMost + 4 * setting);
    const float range = float(fp::sub(nfs::Float(most), nfs::Float(least)));
    const nfs::Float slider = fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, ts, item, 0);
    const float value = float(fp::add(fp::mul(slider, nfs::Float(range)), nfs::Float(least)));
    const float atLeast = notAbove(guest, nfs::Float(least), nfs::Float(value)) ? value : least;
    float result;
    if (!notAbove(guest, nfs::Float(atLeast), nfs::Float(most)))
        result = most;
    else
        result = notAbove(guest, nfs::Float(least), nfs::Float(value)) ? value : least;
    guest.setFloat(kTuning + 4 * setting, result);
    nfs::u32 bits;
    std::memcpy(&bits, &result, 4);
    return bits;
}

}  // namespace

nfs::u32 frontend::cars::engineMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 0);
}

nfs::u32 frontend::cars::brakesMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 1);
}

nfs::u32 frontend::cars::steeringMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 2);
}

nfs::u32 frontend::cars::gearsMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 3);
}

nfs::u32 frontend::cars::suspensionMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 4);
}

nfs::u32 frontend::cars::aeroMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 5);
}

nfs::u32 frontend::cars::tiresMoved(App* app, ThreadState& ts, u32 item)
{
    return onSettingMoved(app, ts, item, 6);
}

/* sub_4277d0: the sliders set to the settings. */
nfs::u32 frontend::cars::setSliders(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0xc);
    if (menu == 0)
        return 0;
    nfs::u32 eax = 0;   // as the last call leaves it
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        eax = slider.address();
        if (!slider)
            continue;
        const nfs::Float least = nfs::Float(guest.floatAt(kTuningLeast + 4 * i));
        const nfs::Float above = fp::sub(nfs::Float(guest.floatAt(kTuning + 4 * i)), least);
        const nfs::Float range = fp::sub(nfs::Float(guest.floatAt(kTuningMost + 4 * i)), least);
        const float value = float(fp::div(above, range));
        nfs::u32 bits;
        std::memcpy(&bits, &value, 4);
        eax = nfs::u32(*fe::direct<0x46c950, port::fields::sliderSetValueOf>(app, ts, ts.sp - 4 - 4 * 2,
                                                                                slider.address(), bits, 0));
    }
    return eax;
}

/* sub_4279b0: the sliders as they stand, kept to tell a change by. */
nfs::u32 frontend::cars::rememberSliders(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (menu == 0)
        return 0;
    for (std::uint32_t i = 0; i < kSettings; ++i)
        guest.setDword(kTuningShown + 4 * i, 0);
    u32 last = 0;   // the item looked for last: eax stays it through the slider's value, which is the answer
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        last = slider.address();
        if (!slider)
            continue;
        const double value = fe::direct<kSliderValue, frontend::glue::sliderValue>(app, ts, slider.address(), 0);
        guest.setFloat(kTuningShown + 4 * i, float(value));
    }
    return last;
}

/* sub_427ac0: whether a slider moved since kTuningShown, in al.  The first
 * slider missing counts as moved; the others are looked at while none has. */
nfs::u32 frontend::cars::slidersMoved(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (menu == 0)
        return 1;
    nfs::u32 eax = 0;
    std::uint8_t moved = 1;
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        if (i > 0 && moved != 0)
            break;
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        eax = slider.address();
        if (!slider)
            continue;
        const nfs::Float value = fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, ts, slider.address(), 0);
        const std::uint16_t status = compareStatusWord(value, nfs::Float(guest.floatAt(kTuningShown + 4 * i)));
        eax = (eax & 0xffff0000) | status;   // fnstsw ax
        moved = (status & 0x4000) != 0 ? 0 : 1;
    }
    return (eax & 0xffffff00) | moved;
}

/* sub_427c20: each slider told where its moves go, set to its setting and
 * remembered. */
nfs::u32 frontend::cars::bindSliders(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        if (Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]))
            fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, ts, slider.address(), kSettingMoved[i]);
    }
    guest::direct<0x4277d0, setSliders>(app, ts, menu);
    guest::direct<0x4279b0, rememberSliders>(app, ts, menu);
    return 0;
}

/* sub_427d00: the advanced tuning edited back to the player's own, when the
 * player has one; 0xe when not. */
nfs::u32 frontend::cars::loadAdvanced(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    textEntry(false);   // Port: as the port's text entry change has the original
    const nfs::u32 player = guest.dword(kTunedPlayer);
    nfs::u32 has = 0;
    if (player == SETTINGS_ADDRESS(players[0].car))
        has = *fe::direct<0x4dca50, load::specsSavedOf>(app, ts, 0);
    else if (player == SETTINGS_ADDRESS(players[1].car))
        has = *fe::direct<0x4dca50, load::specsSavedOf>(app, ts, 1);
    if (has == 0)
        return 0xe;
    if (const nfs::u32 own = advancedOf(guest))
        guest::direct<0x4dcb60, copyAdvanced>(app, ts, kAdvancedEdited, own);
    return 0;
}

/* sub_427d80, the defaults question answered: Yes puts the defaults back. */
nfs::u32 frontend::cars::defaultsAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer != fe::kDialogEnter)
        return 0;
    if (fe::dialogFocus(guest) == 0)
    {
        for (std::uint32_t i = 0; i < kSettings; ++i)
            guest.setFloat(kTuning + 4 * i, float(nfs::Float(guest.floatAt(kTuningDefaults + 4 * i))));
        guest::direct<0x4277d0, setSliders>(app, ts, guest.dword(kTuningMenu));
    }
    if (close != 0)
        guest.setByte(close, 1);
    return fe::kDialogHandled;
}

namespace
{

/* sub_427e10 (CEB_DONE, and racenow, exitprogram, optionsnow) and sub_427ea0
 * (CEB_CANCEL): Enter keeps the sliders' tuning when they moved.  Done takes
 * the player's own advanced tuning back into the one edited, when they
 * differ; Cancel keeps the one edited as the player's. */
nfs::u32 onLeaveTuningKey(App* app, ThreadState& ts, nfs::u32 item, nfs::u32 menu, std::uint16_t key, bool done)
{
    namespace cars = frontend::cars;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (!done)
        textEntry(false);   // Port: as the port's text entry change has the original
    if (key == kKeyEnter)
    {
        if ((guest::direct<0x427ac0, cars::slidersMoved>(app, ts, menu) & 0xff) != 0)
            guest::direct<0x427370, cars::saveTuning>(app, ts, tunedCar(guest));
        if (const nfs::u32 own = advancedOf(guest))
        {
            if ((guest::direct<0x4dcaf0, cars::advancedDiffers>(app, ts, own, kAdvancedEdited) & 0xff) != 0)
            {
                if (done)
                    guest::direct<0x427d00, cars::loadAdvanced>(app, ts);
                else
                    guest::direct<0x4dcb60, cars::copyAdvanced>(app, ts, own, kAdvancedEdited);
            }
        }
    }
    // By address: what it leaves in ebx and edx this leaves too.
    return passKey(guest, kButtonKey, item, menu, key);
}

}  // namespace

nfs::u32 frontend::cars::doneKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key)
{
    return onLeaveTuningKey(app, ts, item, menu, std::uint16_t(key), true);
}

nfs::u32 frontend::cars::cancelKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key)
{
    return onLeaveTuningKey(app, ts, item, menu, std::uint16_t(key), false);
}

/* sub_427f40, CE_SLCT_CAR's key: another car chosen, the old one's tuning
 * kept if the sliders moved, the new one's loaded. */
nfs::u32 frontend::cars::carListKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 ebx)
{
    const std::uint16_t key = std::uint16_t(ebx);
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    // The original hands it 0x1d in edx too, which it does not take but leaves: edx as this goes back
    // without a key passed on.
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, item) == 0)
        return 0;
    const nfs::u32 was = tunedCar(guest);
    const nfs::u32 result = passKey(guest, kListKey, item, menu, key);
    if (was != tunedCar(guest))
    {
        if ((guest::direct<0x427ac0, slidersMoved>(app, ts, menu) & 0xff) != 0)
            guest::direct<0x427370, saveTuning>(app, ts, was);
        guest::direct<0x4272f0, loadTuning>(app, ts, tunedCar(guest));
        guest::direct<0x427c20, bindSliders>(app, ts, menu);
    }
    return result;
}

/* sub_427fb0, CEB_DEFAULT's key: Enter asks first, No highlighted. */
nfs::u32 frontend::cars::defaultsKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 ebx)
{
    const std::uint16_t key = std::uint16_t(ebx);
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
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
nfs::u32 frontend::cars::bindDoneCancel(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    textEntry(true);   // Port: as the port's text entry change has the original
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
nfs::u32 frontend::cars::fitTuningButtons(App* app, ThreadState& ts, u32 menu, u32 tuningFor)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    if (menu == 0)
        return 0;
    guest::direct<0x428020, bindDoneCancel>(app, ts, menu);
    Item race = fe::findItem(guest, menu, kRaceNow);
    Item exit = fe::findItem(guest, menu, kExitProgram);
    Item options = fe::findItem(guest, menu, kOptionsNow);
    Item list = fe::findItem(guest, menu, kCarList);
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
    }
    if (list)
    {
        const nfs::u32 flags = list.address() + fe::item::kFlags;
        if (tuningFor >= 1 && tuningFor <= 3)
        {
            guest.setDword(list.address() + kOnKey, kCarListKey);
            guest.setDword(list.address() + kListName, tuningFor == 2 ? kCars2List : kCarsList);
            const std::uint8_t value = std::uint8_t(tuningFor == 3 ? guest.byte(flags) | fe::item::kHidden
                                                                   : guest.byte(flags) & ~fe::item::kHidden);
            guest.setByte(flags, value);
        }
        if (inRace(guest))
            list.setHidden(true);
    }
    return list.address();
}

/* sub_4281e0: the tuning screen made ready: whose car, the advanced tuning
 * to edit from the player's own, the settings loaded.  1 when it has no one
 * to tune for. */
nfs::u32 frontend::cars::prepareTuning(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    guest.setDword(kTuningMenu, menu);
    auto forPlayer = [&](nfs::u32 player, nfs::u32 own) {
        guest.setDword(kTunedPlayer, player);
        guest::direct<0x4dcb60, copyAdvanced>(app, ts, kAdvancedEdited, own);
    };
    switch (guest.dword(kTuningFor))
    {
    case 1:
        forPlayer(SETTINGS_ADDRESS(players[0].car), kAdvancedPlayer1);
        guest.setDword(kTuningForLast, 1);
        break;
    case 2:
        forPlayer(SETTINGS_ADDRESS(players[1].car), kAdvancedPlayer2);
        guest.setDword(kTuningForLast, 2);
        break;
    case 3:
        forPlayer(SETTINGS_ADDRESS(players[0].car), kAdvancedPlayer1);
        guest.setDword(kTuningForLast, 3);
        break;
    case 4:
        break;
    default:
    {
        // Back from cedadv: whom it was for last.
        const nfs::u32 last = guest.dword(kTuningForLast);
        if (last == 1 || last == 3)
            forPlayer(SETTINGS_ADDRESS(players[0].car), kAdvancedPlayer1);
        else if (last == 2)
            forPlayer(SETTINGS_ADDRESS(players[1].car), kAdvancedPlayer2);
        else
            return 1;
        guest.setDword(kTuningFor, guest.dword(kTuningForLast));
        break;
    }
    }
    const nfs::u32 loaded = guest::direct<0x4272f0, loadTuning>(app, ts, tunedCar(guest));
    if (inRace(guest))
    {
        // The shade where the original pushes it, just under the frame: dimmed rewrites it there.
        const nfs::u32 slot = guest.frame() - 4;
        guest.setDword(slot, kFloat0_6);
        fe::direct<0x451530, port::text::dimmed>(app, ts, slot, loaded);
    }
    const nfs::u32 tuningFor = guest.dword(kTuningFor);
    if (tuningFor >= 1 && tuningFor <= 3)
    {
        guest::direct<0x428060, fitTuningButtons>(app, ts, menu, tuningFor);
        return 0;
    }
    return tuningFor == 4 ? 0 : 1;
}

/* sub_428360, cedbasic as it opens. */
nfs::u32 frontend::cars::tuningEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    nfs::u32 result = guest::direct<0x4281e0, prepareTuning>(app, ts, menu);
    if (result != 0)
        return result;
    if (Item defaults = fe::findItem(guest, menu, kDefaults))
        guest.setDword(defaults.address() + kOnKey, kDefaultsKey);
    result = guest::direct<0x427c20, bindSliders>(app, ts, menu);
    if (Item advanced = fe::findItem(guest, menu, kAdvanced))
        advanced.setHidden(true);
    return result;
}

/* sub_4283b0: no one to tune for any more. */
nfs::u32 frontend::cars::tuningExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kTuningFor, 0);
    return 0;
}

/* sub_4dcaf0: whether two advanced tunings differ, in al: memcmp's answer
 * for the first record that does, its sign in eax. */
nfs::u32 frontend::cars::advancedDiffers(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    if (a == 0 || b == 0)
    {
        return a & 0xffff0000;
    }
    for (std::uint32_t i = 0; i < kAdvancedRecords; ++i)
    {
        const nfs::u32 offset = kAdvancedRecordSize * i;
        for (nfs::u32 j = 0; j < kAdvancedRecordSize; ++j)
        {
            const std::uint8_t x = guest.byte(a + offset + j);
            const std::uint8_t y = guest.byte(b + offset + j);
            if (x != y)
            {
                return x < y ? 0xffffff01 : 0x00000001;
            }
        }
    }
    return 0;
}

/* sub_4dcb60: one advanced tuning copied over another. */
nfs::u32 frontend::cars::copyAdvanced(App* app, ThreadState& ts, u32 to, u32 from)
{
    Guest guest(app, ts);
    if (to == 0 || from == 0)
        return to;
    for (std::uint32_t i = 0; i < kAdvancedRecords; ++i)
        copyDwords(guest, to + kAdvancedRecordSize * i, from + kAdvancedRecordSize * i, kAdvancedRecordSize);
    return kAdvancedRecordSize;
}

namespace
{

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

/* A core of these screens under the game's name: its menu in eax; an item, its
 * menu and the key in eax, edx and ebx. */
template <auto Core>
nfs::u32 wholeForMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, fe::Site& site, nfs::u32 menu)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return Core(app, ts, menu); });
}

template <auto Core>
nfs::u32 wholeForKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, fe::Site& site, nfs::u32 item, nfs::u32 menu, nfs::u32 key)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return Core(app, ts, item, menu, key); });
}

constexpr std::uint8_t kNone = 0;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEcxEdxEsi = fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi;
constexpr std::uint8_t kAllButEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi;
constexpr std::uint8_t kAllButEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kAllButEbx = fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi;

}

/* Each thunk: the game's function, what its ret pops, the strings it
 * hands to the game and the registers it pushes; its frame is its core's. */

nfs::u32 frontend::carMenuMusicBack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4104b0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::menuMusicBack(app, ts, eax); });
}

nfs::u32 frontend::carSaveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4267a0", 0, 0, kCarFrameStrings, std::size(kCarFrameStrings), fe::kRestoreEbx };
    const nfs::u32 folder = args.eax, first = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::saveCarFrame(app, ts, folder, first); });
}

nfs::u32 frontend::trackSaveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4267f0", 0, 0, kTrackFrameStrings, std::size(kTrackFrameStrings),
                          fe::kRestoreEbx };
    const nfs::u32 folder = args.eax, first = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::saveTrackFrame(app, ts, folder, first); });
}

nfs::u32 frontend::carRecord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426870", 0, 0, kRecorderStrings, std::size(kRecorderStrings) };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::recordCar(app, ts, eax); });
}

nfs::u32 frontend::trackRecord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static const GuestString strings[] = { kTrackFolderName };
    static fe::Site site{ "sub_426b00", 0, 0, strings, std::size(strings),
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::recordTrack(app, ts, eax); });
}

nfs::u32 frontend::car1OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426d60", 0, 0, kCar1EnterStrings, std::size(kCar1EnterStrings), kEcxEdxEsi };
    return wholeForMenu<cars::car1Enter>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::car2OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426ef0", 0, 0, kCar2EnterStrings, std::size(kCar2EnterStrings), kEcxEdxEsi };
    return wholeForMenu<cars::car2Enter>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::car1OnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426e20", 0, 0, kCar1FrameStrings, std::size(kCar1FrameStrings), kEbxEcxEdx };
    return wholeForMenu<cars::car1Frame>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::car2OnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426f80", 0, 0, kCar2FrameStrings, std::size(kCar2FrameStrings), kEbxEcxEdx };
    return wholeForMenu<cars::car2Frame>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::car1OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_426ee0: nothing to undo.
    static fe::Site site{ "sub_426ee0", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::car2OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_427040: likewise.
    static fe::Site site{ "sub_427040", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::car1EditClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426c20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::car1EditClicked(app, ts); });
}

nfs::u32 frontend::car2EditClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426c40", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::car2EditClicked(app, ts); });
}

nfs::u32 frontend::car1ColourClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426c60", 0, 0, kCar1ColourStrings, std::size(kCar1ColourStrings), kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::car1ColourClicked(app, ts); });
}

nfs::u32 frontend::car2ColourClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426ce0", 0, 0, kCar2ColourStrings, std::size(kCar2ColourStrings), kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::car2ColourClicked(app, ts); });
}

nfs::u32 frontend::opponentFitCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427050", 0, 0, kOpponentCarStrings, std::size(kOpponentCarStrings), kEbxEcxEdx };
    return wholeForMenu<cars::opponentFitCar>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::opponentFitArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4270c0", 0, 0, kOpponentArtStrings, std::size(kOpponentArtStrings), kAllButEsi };
    return wholeForMenu<cars::opponentFitArt>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::opponentOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4272a0", 0, 0, nullptr, 0, kEcxEdxEsi };
    return wholeForMenu<cars::opponentEnter>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::opponentOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_4272e0: nothing to undo.
    static fe::Site site{ "sub_4272e0", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::opponentOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427190", 0, 0, kOpponentFrameStrings, std::size(kOpponentFrameStrings), kAllButEsi };
    return wholeForMenu<cars::opponentFrame>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4272f0", 0, 0, nullptr, 0, kAllButEbx };
    return wholeForMenu<cars::loadTuning>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427370", 0, 0, nullptr, 0, kAllButEbx };
    return wholeForMenu<cars::saveTuning>(app, ts, args, site, args.eax);
}

namespace
{
template <auto Core>
nfs::u32 settingMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, fe::Site& site, nfs::u32 item)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return Core(app, ts, item); });
}
}

nfs::u32 frontend::tuningEngineMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4273e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::engineMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningBrakesMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427470", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::brakesMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningSteeringMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427500", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::steeringMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningGearsMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427590", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::gearsMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningSuspensionMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427620", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::suspensionMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningAeroMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4276b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::aeroMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningTiresMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427740", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::tiresMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningSetSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4277d0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    return wholeForMenu<cars::setSliders>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningRememberSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4279b0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    return wholeForMenu<cars::rememberSliders>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningSlidersMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427ac0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    return wholeForMenu<cars::slidersMoved>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningBindSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427c20", 0, 0, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    return wholeForMenu<cars::bindSliders>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningLoadAdvanced(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427d00", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::loadAdvanced(app, ts); });
}

nfs::u32 frontend::tuningDefaultsAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427d80", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const std::uint32_t answer = args.eax;
    const nfs::u32 close = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::defaultsAnswered(app, ts, answer, close); });
}

nfs::u32 frontend::tuningDoneKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427e10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    return wholeForKey<cars::doneKey>(app, ts, args, site, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::tuningCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427ea0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    return wholeForKey<cars::cancelKey>(app, ts, args, site, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::tuningCarListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427f40", 0, 0, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    return wholeForKey<cars::carListKey>(app, ts, args, site, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::tuningDefaultsKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_427fb0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey<cars::defaultsKey>(app, ts, args, site, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::tuningBindDoneCancel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_428020", 0, 0, kDoneCancelStrings, std::size(kDoneCancelStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx };
    return wholeForMenu<cars::bindDoneCancel>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningFitButtons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_428060", 0, 0, kTuningButtonStrings, std::size(kTuningButtonStrings), kAllButEdx };
    const nfs::u32 menu = args.eax;
    const nfs::u32 tuningFor = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::fitTuningButtons(app, ts, menu, tuningFor); });
}

nfs::u32 frontend::tuningPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4281e0", 0, 0, nullptr, 0 };
    return wholeForMenu<cars::prepareTuning>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_428360", 0, 0, kTuningEnterStrings, std::size(kTuningEnterStrings), kEbxEcxEdx };
    return wholeForMenu<cars::tuningEnter>(app, ts, args, site, args.eax);
}

nfs::u32 frontend::tuningOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4283b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::tuningExit(app, ts); });
}

nfs::u32 frontend::tuningAdvancedDiffers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dcaf0", 0, 0, nullptr, 0, kAllButEdx };
    const nfs::u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::advancedDiffers(app, ts, a, b); });
}

nfs::u32 frontend::tuningCopyAdvanced(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dcb60", 0, 0, nullptr, 0, kAllButEdx };
    const nfs::u32 to = args.eax, from = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return cars::copyAdvanced(app, ts, to, from); });
}

}
