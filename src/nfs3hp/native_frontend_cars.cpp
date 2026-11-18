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
using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kInRace = 0x559234;          // the menus are a race's pause screens
constexpr std::uint32_t kHotPursuit = 3;

constexpr x86::reg32 kOnKey = 0x30;
constexpr x86::reg32 kListName = 0x64;            // a list's: the name of its entries (a button's kOnClick)
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr x86::reg32 kGoOn = 5;                   // to the selected button's nextmenu (sub_44b230)
// The screen closes without fading out, and opens again (sub_44b230).
constexpr x86::reg32 kReopen = 9;

// The menu engine's and the widgets'.
constexpr x86::reg32 kButtonKey = 0x461270;
constexpr x86::reg32 kListKey = 0x463ff0;
constexpr x86::reg32 kSliderValue = 0x46c9b0;     // eax the item, edx the end; the value on the x87
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;   // 0.6f
constexpr x86::reg32 kSprintf = 0x4df690;

constexpr x86::reg32 kShowcaseDownload = 0x452e60; // CAR_DOWNLOAD's click, in the original

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
    return callAt(guest, handler, fe::args({ item, menu, x86::reg32(std::int32_t(std::int16_t(key))) }));
}

/* ---------------------------------------------------------------------------
 * The music, back to the menus'
 * ------------------------------------------------------------------------- */

// Set while a music other than the menus' plays (a car's, sub_4104f0).
constexpr x86::reg32 kOtherMusic = 0x552594;
// The sound setting 6 plays no menu music.
constexpr std::uint32_t kNoMenuMusic = 6;
constexpr std::uint32_t kMenuTune = 0x16;

}  // namespace

/* sub_4104b0: as a car screen opens, the menus' music again if another plays. */
x86::reg32 frontend::cars::menuMusicBack(App* app, CPU& cpu, reg32 eax)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (guest.dword(kOtherMusic) != 0)
    {
        guest.setDword(kOtherMusic, 0);
        if (gameSettings(guest).menuMusicAsked != kNoMenuMusic)
        {
            fe::direct<0x4109d0, port::musicStopped>(app, cpu, cpu.esp - 4, eax);
            return x86::reg32(*fe::direct<0x4104f0, port::musicStarted>(
                app, cpu, cpu.esp - 4, std::int32_t(kMenuTune), std::int32_t(gameSettings(guest).menuMusicVolume), 0));
        }
    }
    return eax;
}

namespace
{

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
constexpr std::int32_t kFramesInTurn = 31;
constexpr double kTurnStep = 0.03125;             // at 0x536f94 and 0x536fac

constexpr x86::reg32 kMakeFolder = 0x4eb1d0;      // eax a path

constexpr GuestString kCarFrameName{ 0x536f64, "c:\\%02d\\%04d.tga" };
constexpr GuestString kTrackFrameName{ 0x536f78, "c:\\T%02d\\%04d.tga" };
constexpr GuestString kCarFolderName{ 0x536f8c, "c:\\%02d" };
constexpr GuestString kTrackFolderName{ 0x536f9c, "c:\\T%02d" };

/* sub_4267a0, sub_4267f0: the screen saved as the next frame, `first` and
 * on, in the folder of `folder`. */
x86::reg32 saveFrame(App* app, CPU& cpu, const GuestString& format, x86::reg32 folder, x86::reg32 first)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8 + 0x12c);
    const x86::reg32 name = guest.frame();    // char[0x12c]
    const x86::reg32 number = first + guest.dword(kRecordShot);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, format.address, guest::Words{folder, number});
    const x86::reg32 next = guest.dword(kRecordShot) + 1;
    guest.setDword(kRecordShot, next);
    fe::direct<0x4ead20, frontend::wata::windowApply>(app, cpu);
    fe::direct<0x4eb160, frontend::eaa::bitmapSaveTga>(app, cpu, name);
    return fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, cpu);
}

}  // namespace

x86::reg32 frontend::cars::saveCarFrame(App* app, CPU& cpu, reg32 folder, reg32 first)
{
    return saveFrame(app, cpu, kCarFrameName, folder, first);
}

x86::reg32 frontend::cars::saveTrackFrame(App* app, CPU& cpu, reg32 folder, reg32 first)
{
    return saveFrame(app, cpu, kTrackFrameName, folder, first);
}

namespace
{

/* The turn moved on by a frame; true at its end. */
bool nextFrame(Guest& guest)
{
    const x86::reg32 frame = guest.dword(kRecordFrame) + 1;
    guest.setDword(kRecordFrame, frame);
    guest.setFloat(kRecordTurn, float(fp::add(x86::Float(guest.floatAt(kRecordTurn)), x86::Float(kTurnStep))));
    return std::int32_t(frame) > kFramesInTurn;
}

/* A turn's frame for player one's car, from the hundred `first`.  The
 * original hands the car's id lookup edx as it has it (100, 200 or 7) and the
 * id's number `first` in edx; neither takes it. */
x86::reg32 recordCarFrame(Guest& guest, CPU& cpu, x86::reg32 first)
{
    App* const app = guest.app();
    const x86::reg32 data = *fe::direct<0x439520, load::idOf>(app, cpu, gameSettings(guest).players[0].car);
    const x86::reg32 folder = fe::direct<0x439660, load::numberOf>(app, cpu, data);
    return guest::direct<0x4267a0, frontend::cars::saveCarFrame>(app, cpu, folder, first);
}

bool recording(const Guest& guest)
{
    return (guest.byte(kDeveloperFlags) & kRecordFrames) != 0;
}

}  // namespace

/* sub_426870, on every pass of car1 and car2: player one's car turning,
 * three turns -- 100 on, 200 on with the view's first flag, 300 on with its
 * second -- a frame every other pass. */
x86::reg32 frontend::cars::recordCar(App* app, CPU& cpu, reg32 eax)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x134);
    if (!recording(guest))
        return eax;
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
        // The original hands the id lookup 1 in edx, which it does not take.
        const x86::reg32 data = *fe::direct<0x439520, load::idOf>(app, cpu, gameSettings(guest).players[0].car);
        const x86::reg32 folder = fe::direct<0x439660, load::numberOf>(app, cpu, data);
        const x86::reg32 name = guest.frame();    // char[0x134]
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kCarFolderName.address, guest::Words{folder});
        return fe::direct<kMakeFolder, watcom::clib::mkdir>(guest.app(), guest.cpu(), name);
    }
    case 1:
    case 4:
    case 7:
        guest.setDword(kRecordStep, step + 1);
        return step;
    case 2:
    {
        guest.setDword(kRecordStep, 1);
        const x86::reg32 result = recordCarFrame(guest, cpu, 100);
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
        const x86::reg32 result = recordCarFrame(guest, cpu, 200);
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
        const x86::reg32 result = recordCarFrame(guest, cpu, 300);
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
x86::reg32 frontend::cars::recordTrack(App* app, CPU& cpu, reg32 eax)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x134);
    if (!recording(guest))
        return eax;
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
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kTrackFolderName.address, guest::Words{gameSettings(guest).track});
        return fe::direct<kMakeFolder, watcom::clib::mkdir>(guest.app(), guest.cpu(), name);
    }
    if (step == 1)
    {
        guest.setDword(kRecordStep, 2);
        return step;
    }
    if (step != 2)
        return step;
    guest.setDword(kRecordStep, 1);
    x86::reg32 result = guest::direct<0x4267f0, saveTrackFrame>(app, cpu, gameSettings(guest).track, 100);
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

constexpr x86::reg32 kTuningFor = 0x553048;       // who the tuning screen is for: 1 player one, 2 player two
constexpr x86::reg32 kColourList = 0xb3;           // byte of the 3D model's item: its colours changed

constexpr std::uint32_t kPlayer1CarTitle = 0x1aa;  // two players on this machine
constexpr std::uint32_t kYourCarTitle = 0x102;

/* A player's screen: its car, its codelinks and the handlers it hands out. */
struct CarScreen
{
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

constexpr CarScreen kCar1{ 0, { 0x537004, "CE_EDITP1" }, { 0x536fb4, "CAR_SELECT_OBJ3DP1" },
                           { 0x536fc8, "CAR_COLOR_SELECT_P1" }, 0x426c20, 0x426c60 };
constexpr CarScreen kCar2{ 1, { 0x537060, "CE_EDITP2" }, { 0x536fdc, "CAR_SELECT_OBJ3DP2" },
                           { 0x536ff0, "CAR_COLOR_SELECT_P2" }, 0x426c40, 0x426ce0 };

/* The tuning button: not in a network race, nor in a series. */
void fitEditButton(Guest& guest, x86::reg32 menu, const CarScreen& screen)
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
void fitShowcase(Guest& guest, CPU& cpu, x86::reg32 menu, const CarScreen& screen)
{
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
        grey(showcase, fe::direct<0x4530c0, frontend::menub::carArtExists>(
                           guest.app(), cpu, gameSettings(guest).players[screen.player].car) == 0);
}

}  // namespace

/* sub_426d60, car1 as it opens. */
x86::reg32 frontend::cars::car1Enter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest::direct<0x4104b0, menuMusicBack>(app, cpu, 0);  // its result is not read
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
x86::reg32 frontend::cars::car2Enter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest::direct<0x4104b0, menuMusicBack>(app, cpu, 0);  // its result is not read
    fitEditButton(guest, menu, kCar2);
    if (Item colour = fe::findItem(guest, menu, kColourButton))
        colour.setOnClick(kCar2.colourClicked);
    fitShowcase(guest, cpu, menu, kCar2);
    return 0;
}

namespace
{

/* Whether a car's colour can be changed: it has more than one. */
bool oneColour(Guest& guest, CPU& cpu, std::uint32_t player)
{
    App* const app = guest.app();
    const x86::reg32 car = gameSettings(guest).players[player].car;
    return *fe::direct<0x4396b0, load::policeFlag>(app, cpu, car) != 0
           || *fe::direct<0x439760, port::fields::colourCount>(app, cpu, cpu.esp - 4, car) == 1;
}

/* sub_426e20, sub_426f80: car1's and car2's every pass. */
x86::reg32 onCarFrame(App* app, CPU& cpu, x86::reg32 menu, const CarScreen& screen)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest::direct<0x426870, frontend::cars::recordCar>(app, cpu, 0);  // its result is not read
    fitShowcase(guest, cpu, menu, screen);
    if (Item colours = fe::findItem(guest, menu, screen.colours))
        greyAndHide(guest, colours, oneColour(guest, cpu, screen.player));
    if (Item button = fe::findItem(guest, menu, kColourButton))
        grey(button, oneColour(guest, cpu, screen.player));
    return 0;
}

/* sub_426c20, sub_426c40: CE_EDITPn clicked, the tuning for its player. */
x86::reg32 onEditClicked(App* app, CPU& cpu, x86::reg32 player)
{
    Guest guest(app, cpu);
    guest.setDword(kTuningFor, player);
    return kGoOn;
}

/* sub_426c60, sub_426ce0: the colour button clicked, the car's next colour,
 * the model and the list told. */
x86::reg32 onColourClicked(App* app, CPU& cpu, const CarScreen& screen)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const std::int32_t player = std::int32_t(screen.player);
    const auto car = [&] { return gameSettings(guest).players[screen.player].car; };
    const std::int32_t colour = *fe::direct<0x439790, port::fields::chosenColour>(app, cpu, cpu.esp - 4, car(), player);
    const std::int32_t count = *fe::direct<0x439760, port::fields::colourCount>(app, cpu, cpu.esp - 4, car());
    if (colour >= 0 && count > 0)
    {
        const std::int32_t next = (colour + 1) % count;
        fe::direct<0x4397d0, port::fields::colourSet>(app, cpu, cpu.esp - 4, car(), player, next);
        const auto currentMenu = [&] { return fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, cpu); };
        if (Item model = fe::findItem(guest, currentMenu(), screen.model))
            guest.setByte(model.address() + kColourList, 1);
        if (Item colours = fe::findItem(guest, currentMenu(), screen.colours))
            fe::direct<0x456a20, port::fields::listRefreshed>(app, cpu, cpu.esp - 4, colours.address());
    }
    return fe::kDialogHandled;
}

}  // namespace

x86::reg32 frontend::cars::car1Frame(App* app, CPU& cpu, reg32 menu)
{
    return onCarFrame(app, cpu, menu, kCar1);
}

x86::reg32 frontend::cars::car2Frame(App* app, CPU& cpu, reg32 menu)
{
    return onCarFrame(app, cpu, menu, kCar2);
}

x86::reg32 frontend::cars::car1EditClicked(App* app, CPU& cpu)
{
    return onEditClicked(app, cpu, 1);
}

x86::reg32 frontend::cars::car2EditClicked(App* app, CPU& cpu)
{
    return onEditClicked(app, cpu, 2);
}

x86::reg32 frontend::cars::car1ColourClicked(App* app, CPU& cpu)
{
    return onColourClicked(app, cpu, kCar1);
}

x86::reg32 frontend::cars::car2ColourClicked(App* app, CPU& cpu)
{
    return onColourClicked(app, cpu, kCar2);
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
constexpr x86::reg32 kOpponentsShown = 0x553040;

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
    const std::int32_t opponents = std::int32_t(gameSettings(guest).opponentClass);
    return opponents > 3 || opponents == 0 ? 0 : opponents;
}

}  // namespace

/* sub_427050: the showcase and the comparison, for one car. */
x86::reg32 frontend::cars::opponentFitCar(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (Item showcase = fe::findItem(guest, menu, kShowcase))
    {
        const std::int32_t opponents = std::int32_t(gameSettings(guest).opponentClass);
        if (opponents > 3)
            grey(showcase, fe::direct<0x4530c0, frontend::menub::carArtExists>(
                               app, cpu, x86::reg32(opponents - kOpponentCarBase)) == 0);
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
x86::reg32 frontend::cars::opponentFitArt(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    const x86::reg32 back = fe::findItem(guest, menu, kBack).address();
    const x86::reg32 mask = fe::findItem(guest, menu, kMask).address();
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
    x86::reg32 result = (mask & 0xffffff00) | digit;
    if (opponents == 3)
    {
        // With the bonus car three opponents look like four.
        result = fe::direct<0x439ce0, frontend::misc::creditsHaveBonusCar>(app, cpu);
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
x86::reg32 frontend::cars::opponentEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest::direct<0x4270c0, opponentFitArt>(app, cpu, menu);
    guest.setDword(kOpponentsShown, x86::reg32(opponentCount(guest)));
    guest::direct<0x427050, opponentFitCar>(app, cpu, menu);
    return 0;
}

/* sub_427190, opponent on every pass: traffic only in a single race that has
 * it, the number of opponents not in Hot Pursuit; a change of the number
 * opens the screen again, for its art. */
x86::reg32 frontend::cars::opponentFrame(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    guest::direct<0x4104b0, menuMusicBack>(app, cpu, 0);  // its result is not read
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
    guest::direct<0x427050, opponentFitCar>(app, cpu, menu);
    return x86::reg32(opponentCount(guest)) != guest.dword(kOpponentsShown) ? kReopen : 0;
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
constexpr x86::reg32 kTuning = 0x5e10e0;           // the settings edited
constexpr x86::reg32 kTuningDefaults = 0x55d024;
constexpr x86::reg32 kTuningMost = 0x55d040;
constexpr x86::reg32 kTuningLeast = 0x55d05c;
constexpr x86::reg32 kTuningShown = 0x5e10fc;      // the sliders as the screen opened
constexpr std::uint32_t kTuningBytes = 0x1c;

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
constexpr x86::reg32 kDefaultsAnswered = 0x427d80;
constexpr x86::reg32 kDoneKey = 0x427e10;
constexpr x86::reg32 kCancelKey = 0x427ea0;
constexpr x86::reg32 kCarListKey = 0x427f40;
constexpr x86::reg32 kDefaultsKey = 0x427fb0;

/* The tuned player's record of the advanced tuning, 0 for neither. */
x86::reg32 advancedOf(const Guest& guest)
{
    const x86::reg32 player = guest.dword(kTunedPlayer);
    return player == SETTINGS_ADDRESS(players[0].car) ? kAdvancedPlayer1 : player == SETTINGS_ADDRESS(players[1].car) ? kAdvancedPlayer2 : 0;
}

/* The car tuned. */
x86::reg32 tunedCar(const Guest& guest)
{
    return guest.dword(guest.dword(kTunedPlayer));
}

/* The player's tuning record for a car, 0 for none. */
x86::reg32 playerTuning(Guest& guest, CPU& cpu, x86::reg32 car)
{
    x86::reg32 data = 0;
    if (fe::direct<0x439a50, load::recordAt>(guest.app(), cpu, car) != 0)
        data = *fe::direct<0x439520, load::idOf>(guest.app(), cpu, car);
    if (data == 0)
        return 0;
    const x86::reg32 player = guest.dword(kTunedPlayer);
    if (player == SETTINGS_ADDRESS(players[0].car))
        return fe::direct<0x4dc550, frontend::loada::tuningOf>(guest.app(), cpu, 0u, data);
    if (player == SETTINGS_ADDRESS(players[1].car))
        return fe::direct<0x4dc550, frontend::loada::tuningOf>(guest.app(), cpu, 1u, data);
    return 0;
}

/* `count` bytes, a dword at a time, as rep movsd copies them. */
void copyDwords(Guest& guest, x86::reg32 to, x86::reg32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; i += 4)
        guest.setDword(to + i, guest.dword(from + i));
}

}  // namespace

/* sub_4272f0: a car's tuning to edit, the player's or the defaults. */
x86::reg32 frontend::cars::loadTuning(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    x86::reg32 from = playerTuning(guest, cpu, car);
    if (from == 0)
        from = kTuningDefaults;
    copyDwords(guest, kTuning, from, kTuningBytes);
    return kTuningBytes;
}

/* sub_427370: the tuning edited, to the player's record for a car. */
x86::reg32 frontend::cars::saveTuning(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    x86::reg32 data = 0;
    if (fe::direct<0x439a50, load::recordAt>(app, cpu, car) != 0)
        data = *fe::direct<0x439520, load::idOf>(app, cpu, car);
    if (data == 0)
        return data;   // eax as the last call left it: 0
    const x86::reg32 player = guest.dword(kTunedPlayer);
    x86::reg32 to = 0;
    if (player == SETTINGS_ADDRESS(players[0].car))
        to = fe::direct<0x4dc550, frontend::loada::tuningOf>(app, cpu, 0u, data);
    else if (player == SETTINGS_ADDRESS(players[1].car))
        to = fe::direct<0x4dc550, frontend::loada::tuningOf>(app, cpu, 1u, data);
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
bool notAbove(Guest& guest, const x86::Float& a, const x86::Float& b)
{
    return !(a > b);
}

/* fcom a, b and fnstsw: the status word's condition bits (c0 below or unordered, c2 unordered, c3
 * equal or unordered); the rest of the word is as the x87 starts with it. */
std::uint16_t compareStatusWord(const x86::Float& a, const x86::Float& b)
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
x86::reg32 onSettingMoved(App* app, CPU& cpu, x86::reg32 item, std::uint32_t setting)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8 + 0x1c);
    const float least = guest.floatAt(kTuningLeast + 4 * setting);
    const float most = guest.floatAt(kTuningMost + 4 * setting);
    const float range = float(fp::sub(x86::Float(most), x86::Float(least)));
    const x86::Float slider = fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, cpu, item, 0);
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

}  // namespace

x86::reg32 frontend::cars::engineMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 0);
}

x86::reg32 frontend::cars::brakesMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 1);
}

x86::reg32 frontend::cars::steeringMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 2);
}

x86::reg32 frontend::cars::gearsMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 3);
}

x86::reg32 frontend::cars::suspensionMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 4);
}

x86::reg32 frontend::cars::aeroMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 5);
}

x86::reg32 frontend::cars::tiresMoved(App* app, CPU& cpu, reg32 item)
{
    return onSettingMoved(app, cpu, item, 6);
}

/* sub_4277d0: the sliders set to the settings. */
x86::reg32 frontend::cars::setSliders(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0xc);
    if (menu == 0)
        return 0;
    x86::reg32 eax = 0;   // as the last call leaves it
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        eax = slider.address();
        if (!slider)
            continue;
        const x86::Float least = x86::Float(guest.floatAt(kTuningLeast + 4 * i));
        const x86::Float above = fp::sub(x86::Float(guest.floatAt(kTuning + 4 * i)), least);
        const x86::Float range = fp::sub(x86::Float(guest.floatAt(kTuningMost + 4 * i)), least);
        const float value = float(fp::div(above, range));
        x86::reg32 bits;
        std::memcpy(&bits, &value, 4);
        eax = x86::reg32(*fe::direct<0x46c950, port::fields::sliderSetValueOf>(app, cpu, cpu.esp - 4 - 4 * 2,
                                                                                slider.address(), bits, 0));
    }
    return eax;
}

/* sub_4279b0: the sliders as they stand, kept to tell a change by. */
x86::reg32 frontend::cars::rememberSliders(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (menu == 0)
        return 0;
    for (std::uint32_t i = 0; i < kSettings; ++i)
        guest.setDword(kTuningShown + 4 * i, 0);
    reg32 last = 0;   // the item looked for last: eax stays it through the slider's value, which is the answer
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]);
        last = slider.address();
        if (!slider)
            continue;
        const double value = fe::direct<kSliderValue, frontend::glue::sliderValue>(app, cpu, slider.address(), 0);
        guest.setFloat(kTuningShown + 4 * i, float(value));
    }
    return last;
}

/* sub_427ac0: whether a slider moved since kTuningShown, in al.  The first
 * slider missing counts as moved; the others are looked at while none has. */
x86::reg32 frontend::cars::slidersMoved(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
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
        const x86::Float value = fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, cpu, slider.address(), 0);
        const std::uint16_t status = compareStatusWord(value, x86::Float(guest.floatAt(kTuningShown + 4 * i)));
        eax = (eax & 0xffff0000) | status;   // fnstsw ax
        moved = (status & 0x4000) != 0 ? 0 : 1;
    }
    return (eax & 0xffffff00) | moved;
}

/* sub_427c20: each slider told where its moves go, set to its setting and
 * remembered. */
x86::reg32 frontend::cars::bindSliders(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    for (std::uint32_t i = 0; i < kSettings; ++i)
    {
        if (Item slider = fe::findItem(guest, menu, kSettingCodelinks[i]))
            fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, cpu, slider.address(), kSettingMoved[i]);
    }
    guest::direct<0x4277d0, setSliders>(app, cpu, menu);
    guest::direct<0x4279b0, rememberSliders>(app, cpu, menu);
    return 0;
}

/* sub_427d00: the advanced tuning edited back to the player's own, when the
 * player has one; 0xe when not. */
x86::reg32 frontend::cars::loadAdvanced(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    textEntry(false);   // Port: as the port's text entry change has the original
    const x86::reg32 player = guest.dword(kTunedPlayer);
    x86::reg32 has = 0;
    if (player == SETTINGS_ADDRESS(players[0].car))
        has = *fe::direct<0x4dca50, load::specsSavedOf>(app, cpu, 0);
    else if (player == SETTINGS_ADDRESS(players[1].car))
        has = *fe::direct<0x4dca50, load::specsSavedOf>(app, cpu, 1);
    if (has == 0)
        return 0xe;
    if (const x86::reg32 own = advancedOf(guest))
        guest::direct<0x4dcb60, copyAdvanced>(app, cpu, kAdvancedEdited, own);
    return 0;
}

/* sub_427d80, the defaults question answered: Yes puts the defaults back. */
x86::reg32 frontend::cars::defaultsAnswered(App* app, CPU& cpu, reg32 answer, reg32 close)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (answer != fe::kDialogEnter)
        return 0;
    if (fe::dialogFocus(guest) == 0)
    {
        for (std::uint32_t i = 0; i < kSettings; ++i)
            guest.setFloat(kTuning + 4 * i, float(x86::Float(guest.floatAt(kTuningDefaults + 4 * i))));
        guest::direct<0x4277d0, setSliders>(app, cpu, guest.dword(kTuningMenu));
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
x86::reg32 onLeaveTuningKey(App* app, CPU& cpu, x86::reg32 item, x86::reg32 menu, std::uint16_t key, bool done)
{
    namespace cars = frontend::cars;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (!done)
        textEntry(false);   // Port: as the port's text entry change has the original
    if (key == kKeyEnter)
    {
        if ((guest::direct<0x427ac0, cars::slidersMoved>(app, cpu, menu) & 0xff) != 0)
            guest::direct<0x427370, cars::saveTuning>(app, cpu, tunedCar(guest));
        if (const x86::reg32 own = advancedOf(guest))
        {
            if ((guest::direct<0x4dcaf0, cars::advancedDiffers>(app, cpu, own, kAdvancedEdited) & 0xff) != 0)
            {
                if (done)
                    guest::direct<0x427d00, cars::loadAdvanced>(app, cpu);
                else
                    guest::direct<0x4dcb60, cars::copyAdvanced>(app, cpu, own, kAdvancedEdited);
            }
        }
    }
    // By address: what it leaves in ebx and edx this leaves too.
    return passKey(guest, kButtonKey, item, menu, key);
}

}  // namespace

x86::reg32 frontend::cars::doneKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key)
{
    return onLeaveTuningKey(app, cpu, item, menu, std::uint16_t(key), true);
}

x86::reg32 frontend::cars::cancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key)
{
    return onLeaveTuningKey(app, cpu, item, menu, std::uint16_t(key), false);
}

/* sub_427f40, CE_SLCT_CAR's key: another car chosen, the old one's tuning
 * kept if the sliders moved, the new one's loaded. */
x86::reg32 frontend::cars::carListKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 ebx)
{
    const std::uint16_t key = std::uint16_t(ebx);
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    // The original hands it 0x1d in edx too, which it does not take but leaves: edx as this goes back
    // without a key passed on.
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, cpu, item) == 0)
        return 0;
    const x86::reg32 was = tunedCar(guest);
    const x86::reg32 result = passKey(guest, kListKey, item, menu, key);
    if (was != tunedCar(guest))
    {
        if ((guest::direct<0x427ac0, slidersMoved>(app, cpu, menu) & 0xff) != 0)
            guest::direct<0x427370, saveTuning>(app, cpu, was);
        guest::direct<0x4272f0, loadTuning>(app, cpu, tunedCar(guest));
        guest::direct<0x427c20, bindSliders>(app, cpu, menu);
    }
    return result;
}

/* sub_427fb0, CEB_DEFAULT's key: Enter asks first, No highlighted. */
x86::reg32 frontend::cars::defaultsKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 ebx)
{
    const std::uint16_t key = std::uint16_t(ebx);
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
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
x86::reg32 frontend::cars::bindDoneCancel(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
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
x86::reg32 frontend::cars::fitTuningButtons(App* app, CPU& cpu, reg32 menu, reg32 tuningFor)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 28);
    if (menu == 0)
        return 0;
    guest::direct<0x428020, bindDoneCancel>(app, cpu, menu);
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
        const x86::reg32 flags = list.address() + fe::item::kFlags;
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
x86::reg32 frontend::cars::prepareTuning(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    guest.setDword(kTuningMenu, menu);
    auto forPlayer = [&](x86::reg32 player, x86::reg32 own) {
        guest.setDword(kTunedPlayer, player);
        guest::direct<0x4dcb60, copyAdvanced>(app, cpu, kAdvancedEdited, own);
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
        const x86::reg32 last = guest.dword(kTuningForLast);
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
    const x86::reg32 loaded = guest::direct<0x4272f0, loadTuning>(app, cpu, tunedCar(guest));
    if (inRace(guest))
    {
        // The shade where the original pushes it, just under the frame: dimmed rewrites it there.
        const x86::reg32 slot = guest.frame() - 4;
        guest.setDword(slot, kFloat0_6);
        fe::direct<0x451530, port::text::dimmed>(app, cpu, slot, loaded);
    }
    const x86::reg32 tuningFor = guest.dword(kTuningFor);
    if (tuningFor >= 1 && tuningFor <= 3)
    {
        guest::direct<0x428060, fitTuningButtons>(app, cpu, menu, tuningFor);
        return 0;
    }
    return tuningFor == 4 ? 0 : 1;
}

/* sub_428360, cedbasic as it opens. */
x86::reg32 frontend::cars::tuningEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    x86::reg32 result = guest::direct<0x4281e0, prepareTuning>(app, cpu, menu);
    if (result != 0)
        return result;
    if (Item defaults = fe::findItem(guest, menu, kDefaults))
        guest.setDword(defaults.address() + kOnKey, kDefaultsKey);
    result = guest::direct<0x427c20, bindSliders>(app, cpu, menu);
    if (Item advanced = fe::findItem(guest, menu, kAdvanced))
        advanced.setHidden(true);
    return result;
}

/* sub_4283b0: no one to tune for any more. */
x86::reg32 frontend::cars::tuningExit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kTuningFor, 0);
    return 0;
}

/* sub_4dcaf0: whether two advanced tunings differ, in al: memcmp's answer
 * for the first record that does, its sign in eax. */
x86::reg32 frontend::cars::advancedDiffers(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest guest(app, cpu);
    if (a == 0 || b == 0)
    {
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
                return x < y ? 0xffffff01 : 0x00000001;
            }
        }
    }
    return 0;
}

/* sub_4dcb60: one advanced tuning copied over another. */
x86::reg32 frontend::cars::copyAdvanced(App* app, CPU& cpu, reg32 to, reg32 from)
{
    Guest guest(app, cpu);
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
x86::reg32 wholeForMenu(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, fe::Site& site, x86::reg32 menu)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return Core(app, cpu, menu); });
}

template <auto Core>
x86::reg32 wholeForKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, fe::Site& site, x86::reg32 item, x86::reg32 menu, x86::reg32 key)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return Core(app, cpu, item, menu, key); });
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

x86::reg32 frontend::carMenuMusicBack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4104b0", 0, 0, nullptr, 0, kEbxEcxEdx };
    const x86::reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::menuMusicBack(app, cpu, eax); });
}

x86::reg32 frontend::carSaveFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4267a0", 0, 0, kCarFrameStrings, std::size(kCarFrameStrings), fe::kRestoreEbx };
    const x86::reg32 folder = args.eax, first = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::saveCarFrame(app, cpu, folder, first); });
}

x86::reg32 frontend::trackSaveFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4267f0", 0, 0, kTrackFrameStrings, std::size(kTrackFrameStrings),
                          fe::kRestoreEbx };
    const x86::reg32 folder = args.eax, first = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::saveTrackFrame(app, cpu, folder, first); });
}

x86::reg32 frontend::carRecord(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426870", 0, 0, kRecorderStrings, std::size(kRecorderStrings) };
    const x86::reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::recordCar(app, cpu, eax); });
}

x86::reg32 frontend::trackRecord(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static const GuestString strings[] = { kTrackFolderName };
    static fe::Site site{ "sub_426b00", 0, 0, strings, std::size(strings),
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi };
    const x86::reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::recordTrack(app, cpu, eax); });
}

x86::reg32 frontend::car1OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426d60", 0, 0, kCar1EnterStrings, std::size(kCar1EnterStrings), kEcxEdxEsi };
    return wholeForMenu<cars::car1Enter>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::car2OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426ef0", 0, 0, kCar2EnterStrings, std::size(kCar2EnterStrings), kEcxEdxEsi };
    return wholeForMenu<cars::car2Enter>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::car1OnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426e20", 0, 0, kCar1FrameStrings, std::size(kCar1FrameStrings), kEbxEcxEdx };
    return wholeForMenu<cars::car1Frame>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::car2OnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426f80", 0, 0, kCar2FrameStrings, std::size(kCar2FrameStrings), kEbxEcxEdx };
    return wholeForMenu<cars::car2Frame>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::car1OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    // sub_426ee0: nothing to undo.
    static fe::Site site{ "sub_426ee0", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, cpu, args, site, [](Guest&) { return x86::reg32(0); });
}

x86::reg32 frontend::car2OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    // sub_427040: likewise.
    static fe::Site site{ "sub_427040", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, cpu, args, site, [](Guest&) { return x86::reg32(0); });
}

x86::reg32 frontend::car1EditClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426c20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::car1EditClicked(app, cpu); });
}

x86::reg32 frontend::car2EditClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426c40", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::car2EditClicked(app, cpu); });
}

x86::reg32 frontend::car1ColourClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426c60", 0, 0, kCar1ColourStrings, std::size(kCar1ColourStrings), kEbxEcxEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::car1ColourClicked(app, cpu); });
}

x86::reg32 frontend::car2ColourClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_426ce0", 0, 0, kCar2ColourStrings, std::size(kCar2ColourStrings), kEbxEcxEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::car2ColourClicked(app, cpu); });
}

x86::reg32 frontend::opponentFitCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427050", 0, 0, kOpponentCarStrings, std::size(kOpponentCarStrings), kEbxEcxEdx };
    return wholeForMenu<cars::opponentFitCar>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::opponentFitArt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4270c0", 0, 0, kOpponentArtStrings, std::size(kOpponentArtStrings), kAllButEsi };
    return wholeForMenu<cars::opponentFitArt>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::opponentOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4272a0", 0, 0, nullptr, 0, kEcxEdxEsi };
    return wholeForMenu<cars::opponentEnter>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::opponentOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    // sub_4272e0: nothing to undo.
    static fe::Site site{ "sub_4272e0", 0, 4, nullptr, 0, kNone };
    return fe::runThunk(app, cpu, args, site, [](Guest&) { return x86::reg32(0); });
}

x86::reg32 frontend::opponentOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427190", 0, 0, kOpponentFrameStrings, std::size(kOpponentFrameStrings), kAllButEsi };
    return wholeForMenu<cars::opponentFrame>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningLoad(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4272f0", 0, 0, nullptr, 0, kAllButEbx };
    return wholeForMenu<cars::loadTuning>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningSave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427370", 0, 0, nullptr, 0, kAllButEbx };
    return wholeForMenu<cars::saveTuning>(app, cpu, args, site, args.eax);
}

namespace
{
template <auto Core>
x86::reg32 settingMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, fe::Site& site, x86::reg32 item)
{
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return Core(app, cpu, item); });
}
}

x86::reg32 frontend::tuningEngineMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4273e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::engineMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningBrakesMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427470", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::brakesMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningSteeringMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427500", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::steeringMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningGearsMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427590", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::gearsMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningSuspensionMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427620", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::suspensionMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningAeroMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4276b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::aeroMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningTiresMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427740", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return settingMoved<cars::tiresMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningSetSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4277d0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    return wholeForMenu<cars::setSliders>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningRememberSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4279b0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEcxEdx };
    return wholeForMenu<cars::rememberSliders>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningSlidersMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427ac0", 0, 0, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    return wholeForMenu<cars::slidersMoved>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningBindSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427c20", 0, 0, kSettingStrings, std::size(kSettingStrings), kEbxEcxEdx };
    return wholeForMenu<cars::bindSliders>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningLoadAdvanced(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427d00", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::loadAdvanced(app, cpu); });
}

x86::reg32 frontend::tuningDefaultsAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427d80", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const std::uint32_t answer = args.eax;
    const x86::reg32 close = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::defaultsAnswered(app, cpu, answer, close); });
}

x86::reg32 frontend::tuningDoneKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427e10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    return wholeForKey<cars::doneKey>(app, cpu, args, site, args.eax, args.edx, args.ebx);
}

x86::reg32 frontend::tuningCancelKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427ea0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    return wholeForKey<cars::cancelKey>(app, cpu, args, site, args.eax, args.edx, args.ebx);
}

x86::reg32 frontend::tuningCarListKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427f40", 0, 0, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    return wholeForKey<cars::carListKey>(app, cpu, args, site, args.eax, args.edx, args.ebx);
}

x86::reg32 frontend::tuningDefaultsKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_427fb0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey<cars::defaultsKey>(app, cpu, args, site, args.eax, args.edx, args.ebx);
}

x86::reg32 frontend::tuningBindDoneCancel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428020", 0, 0, kDoneCancelStrings, std::size(kDoneCancelStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx };
    return wholeForMenu<cars::bindDoneCancel>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningFitButtons(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428060", 0, 0, kTuningButtonStrings, std::size(kTuningButtonStrings), kAllButEdx };
    const x86::reg32 menu = args.eax;
    const x86::reg32 tuningFor = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::fitTuningButtons(app, cpu, menu, tuningFor); });
}

x86::reg32 frontend::tuningPrepare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4281e0", 0, 0, nullptr, 0 };
    return wholeForMenu<cars::prepareTuning>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_428360", 0, 0, kTuningEnterStrings, std::size(kTuningEnterStrings), kEbxEcxEdx };
    return wholeForMenu<cars::tuningEnter>(app, cpu, args, site, args.eax);
}

x86::reg32 frontend::tuningOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4283b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::tuningExit(app, cpu); });
}

x86::reg32 frontend::tuningAdvancedDiffers(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dcaf0", 0, 0, nullptr, 0, kAllButEdx };
    const x86::reg32 a = args.eax, b = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::advancedDiffers(app, cpu, a, b); });
}

x86::reg32 frontend::tuningCopyAdvanced(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dcb60", 0, 0, nullptr, 0, kAllButEdx };
    const x86::reg32 to = args.eax, from = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return cars::copyAdvanced(app, cpu, to, from); });
}

}
