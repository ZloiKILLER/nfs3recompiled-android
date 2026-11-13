#include "native_frontend_showcase.h"
#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_draw.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_wata.h"
#include "native_load_cardb.h"
#include "port_buttons.h"
#include "port_fields.h"
#include "port_shapes.h"
#include "port_sound_music.h"
#include "port_sound_speech.h"
#include <nfs3hp.h>
#include <cstring>
#include <iterator>

/* The car showcase, decompiled: what a car's "more about it" buttons open.
 *
 *   showcase   MENUS/SHOWCASE.MNU: the car over its backdrop, and which of the
 *              three below it has art for;
 *   slides     SLIDES.MNU: the car's photographs (SL<car>NN.qfs), one at a
 *              time, painted into a 640x480 bitmap, make and model taking turns;
 *   history    the car's history, a page an era (H<car>.qfs), turned every ten
 *              seconds, its description spoken;
 *   interior   the cockpit (IN_<car>.fsh) and its zoom slider;
 *   compare    the two cars side by side, compare2 the same with their figures.
 *
 * A car's art is either in the common folder, named after its folder's name,
 * or in a folder of the car's own, named after its file stem: see formatCarPath.
 * The screens answer 9 where they are to be opened anew, for another car or
 * another photograph, or left for the other compare: the menu loop closes them
 * without fading out and goes on (kAfterScreen, if set, says where to).
 *
 * Plan step 5: each function a typed core in frontend::showcase (native_frontend_showcase.h), in
 * a frame of its own where the game's function has one (fe::Frame), and an adapter under the
 * game's name for the calls by address.  A core of this file calls another as C++ (guest::direct);
 * the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

// The screens' state.
constexpr x86::reg32 kCar = 0x664794;              // the car to show
constexpr x86::reg32 kShownCar = 0x664798;         // the car shown
constexpr x86::reg32 kBackdrop = 0x55957c;         // which of the car's backdrops
constexpr x86::reg32 kShownBackdrop = 0x559584;
constexpr x86::reg32 kReloadArt = 0x559580;
constexpr x86::reg32 kCameFrom = 0x559578;         // char*, the screen before this one
constexpr x86::reg32 kResolvedCar = 0x5595a8;      // the last answer of sub_453500
constexpr x86::reg32 kSlide = 0x559588;            // the photograph shown, from 1
constexpr x86::reg32 kBlinkSince = 0x559594;
constexpr x86::reg32 kEra = 0x55958c;              // the history's page, 1 to 6
constexpr x86::reg32 kPageSince = 0x559590;
constexpr x86::reg32 kCompareLeftCar = 0x664788;
constexpr x86::reg32 kCompareRightCar = 0x664784;
constexpr x86::reg32 kCompareLeftShown = 0x66478c;
constexpr x86::reg32 kCompareRightShown = 0x664790;
constexpr x86::reg32 kCompareLeftPicture = 0x5595ac;
constexpr x86::reg32 kCompareRightPicture = 0x5595b0;
constexpr x86::reg32 kCompareFigures = 0x559570;   // nonzero: compare2's page, 0: compare's
constexpr x86::reg32 kCompare5d4 = 0x5595d4;
constexpr x86::reg32 kCompare574 = 0x559574;
constexpr x86::reg32 kCompareBar = 0x5595b8;       // the bar's texture
constexpr x86::reg32 kCompareArrows[] = { 0x66479c, 0x6647a2, 0x6647a0, 0x66479e };  // int16 each

// The game's.
constexpr x86::reg32 kOpponentsFirstCar = 4;
constexpr x86::reg32 kNetworkConnected = 0x7a22e0;
constexpr x86::reg32 kClock = 0x79c1a4;
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr x86::reg32 kVideoBits = 0x565010;        // byte, a pixel's
constexpr x86::reg32 kLastAction = 0x66296c;       // what left the last screen
constexpr x86::reg32 kAfterScreen = 0x559224;      // char*, the screen to go on to
constexpr x86::reg32 kCharTraits = 0x564ef0;       // the C library's, by character + 1
constexpr std::uint8_t kCharUpper = 0x40;
constexpr x86::reg32 kCarRootOwn = 0x7a28b4;       // the folder of the cars' own folders
constexpr x86::reg32 kCarRootCommon = 0x7a3234;    // the folder of the others' art
constexpr x86::reg32 kArtFolder = 0x7a2db4;

constexpr x86::reg32 kItemTexture = 0x44;          // int16 of an item: its picture
constexpr x86::reg32 kReopen = 9;                  // to the menu loop: closes without fading out
constexpr x86::reg32 kCarFileStemField = 0x315;    // of a car's record
constexpr x86::reg32 kFirstHistoryText = 0x17;     // the car's texts, a line an era

constexpr x86::reg32 kCarInOwnFolder = 0x4395c0;
constexpr x86::reg32 kCarText = 0x439ab0;          // eax the car, edx the line
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kStrcmp = 0x4ee310;
constexpr x86::reg32 kStrncmp = 0x4eb390;
constexpr x86::reg32 kTolower = 0x4f0ae0;
constexpr x86::reg32 kLocateShape = 0x4ef270;      // eax the file, edx a name, 0 when it has none by it
// A bitmap to paint a screen's picture into: made, drawn to, its shape at
// +0x20 made a texture, freed.
constexpr x86::reg32 kNewBitmap = 0x4ef190;        // eax wide, edx high, ebx bits, ecx
constexpr x86::reg32 kBitmapShape = 0x20;
constexpr x86::reg32 kFillRect = 0x4f0a90;         // eax x, edx y, ebx wide, ecx high, the colour on the stack
// A car's description, spoken.
constexpr x86::reg32 kDescriptionStop = 0x4c9490;
constexpr std::uint32_t kShowcaseVolume = 0x32;
constexpr std::uint32_t kHistoryVolume = 0x34;
// The interior's view.
constexpr x86::reg32 kSetInteriorZoom = 0x4646e0;  // a float on the stack

// This group's own, handed to the menu engine as handlers.
constexpr x86::reg32 kNextSlide = 0x453840;
constexpr x86::reg32 kPreviousSlide = 0x453860;
constexpr x86::reg32 kZoomMoved = 0x454cf0;

// The strings.
constexpr GuestString kShowcaseOwn{ 0x539684, "%s%s\\SH%s.qfs" };
constexpr GuestString kShowcaseCommon{ 0x539694, "%sSH%s.qfs" };
constexpr GuestString kInteriorOwn{ 0x5396a0, "%s%s\\IN_%s.fsh" };
constexpr GuestString kInteriorCommon{ 0x5396b0, "%sIN_%s.fsh" };
constexpr GuestString kSlidesOwn{ 0x5396bc, "%s%s\\SL%s01.qfs" };
constexpr GuestString kSlidesCommon{ 0x5396cc, "%sSL%s01.qfs" };
constexpr GuestString kHistoryOwn{ 0x5396dc, "%s%s\\H%s.qfs" };
constexpr GuestString kHistoryCommon{ 0x5396ec, "%sH%s.qfs" };
constexpr GuestString kShowcaseMask{ 0x5396f8, "showcasemask" };
constexpr GuestString kMaskNumbered{ 0x539708, "msk%d" };
constexpr GuestString kShowcaseBack{ 0x539710, "showcaseback" };
constexpr GuestString kBackNumbered{ 0x539720, "bgn%d" };
constexpr GuestString kCompareName{ 0x539728, "compare" };
constexpr GuestString kCompare2Name{ 0x539730, "compare2" };
constexpr GuestString kCar1Name{ 0x53973c, "car1" };
constexpr GuestString kCar2Name{ 0x539744, "car2" };
constexpr GuestString kOpponentName{ 0x53974c, "opponent" };
constexpr GuestString kShowcaseName{ 0x539758, "showcase" };
constexpr GuestString kInteriorName{ 0x539764, "interior" };
constexpr GuestString kSlidesName{ 0x539770, "slides" };
constexpr GuestString kHistoryName{ 0x539778, "history" };
constexpr GuestString kSlideNumber{ 0x539780, "%2d" };
constexpr GuestString kSlideOwn{ 0x539784, "%s%s\\SL%s%s.qfs" };
constexpr GuestString kSlideCommon{ 0x539794, "%sSL%s%s.qfs" };
constexpr GuestString kNextSlideName{ 0x5397a4, "nextslide" };
constexpr GuestString kPrevSlideName{ 0x5397b0, "prevslide" };
constexpr GuestString kBack{ 0x5397bc, "back" };
constexpr GuestString kBandsFile{ 0x5397c4, "%sBands.qfs" };
constexpr GuestString kBand{ 0x5397d0, "band" };
constexpr GuestString kMake{ 0x5397d8, "make" };
constexpr GuestString kModel{ 0x5397e0, "model" };
constexpr GuestString kModelShape{ 0x5397e8, "modl" };
constexpr GuestString kAspid{ 0x5397f0, "Aspid" };
constexpr GuestString kLstm{ 0x5397f8, "Lstm" };
constexpr GuestString kBack8b{ 0x539800, "bk8b" };
constexpr GuestString kMask8b{ 0x539808, "mk8b" };
constexpr GuestString kBack89{ 0x539810, "bk89" };
constexpr GuestString kMask89{ 0x539818, "mk89" };
constexpr GuestString kBackOfYear{ 0x539820, "bk%s" };
constexpr GuestString kMaskOfYear{ 0x539828, "mk%s" };
constexpr GuestString kMask{ 0x539830, "mask" };
constexpr GuestString kHistoryMask{ 0x539838, "historymask" };
constexpr GuestString kHistoryBack{ 0x539844, "historyback" };
constexpr GuestString kCompareFile{ 0x539850, "compare.fsh" };
constexpr GuestString kTwoStrings{ 0x53985c, "%s%s" };
constexpr GuestString kCompareArt{ 0x539864, "%scompare.fsh" };
constexpr GuestString kBarOpaque{ 0x539874, "sofb" };
constexpr GuestString kBarBlended{ 0x53987c, "accb" };
constexpr GuestString kArrowNames[] = {
    { 0x539884, "bup1" }, { 0x53988c, "bdn1" }, { 0x539894, "yup1" }, { 0x53989c, "ydn1" },
};
constexpr GuestString kZoomSlider{ 0x5398d4, "ZoomSlider" };

/* ---------------------------------------------------------------------------
 * Small things they share
 * ------------------------------------------------------------------------- */

/* What an item holds of its picture, and its picture set. */
x86::reg32 itemTexture(Guest& guest, const Item& item)
{
    return x86::reg32(std::int32_t(std::int16_t(guest.word(item.address() + kItemTexture))));
}

void setItemTexture(Guest& guest, const Item& item, x86::reg32 texture)
{
    guest.setWord(item.address() + kItemTexture, std::uint16_t(texture));
}

/* A shape made textures, `replaced` the record to fill again (0 a new one); ecx as the code has it,
 * handed on. */
x86::reg32 shapeTexture(Guest& guest, CPU& cpu, x86::reg32 shape, x86::reg32 replaced)
{
    return x86::reg32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(guest.app(), cpu, cpu.esp - 4, shape,
                                                                         std::int32_t(replaced), cpu.ecx));
}

/* A shape of a loaded file as an item's picture. */
void setItemShape(Guest& guest, CPU& cpu, const Item& item, x86::reg32 file, x86::reg32 name, x86::reg32 replaced)
{
    const x86::reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(guest.app(), cpu, file, name);
    setItemTexture(guest, item, shapeTexture(guest, cpu, shape, replaced));
}

/* What a car's folder's name is at, 0 for none. */
x86::reg32 carFolder(Guest& guest, CPU& cpu, x86::reg32 car)
{
    return *fe::direct<0x4392b0, load::folderOf>(guest.app(), cpu, car);
}

/* Whether the file named at `path` is there. */
bool fileThere(Guest& guest, CPU& cpu, x86::reg32 path)
{
    return *fe::direct<0x4e0e60, ea::fileThere>(guest.app(), cpu, path);
}

/* The block at `body` given back: purgememadr's answer. */
x86::reg32 freeBlock(Guest& guest, CPU& cpu, x86::reg32 body)
{
    return *fe::direct<0x4e1890, ea::purgeBlock>(guest.app(), cpu, body);
}

}  // namespace

/* sub_439290: a car's file stem, in its record: 0 when it has no record. */
x86::reg32 frontend::showcase::carFileStem(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    const x86::reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, car);
    return record != 0 ? record + kCarFileStemField : 0;
}

namespace
{

/* A car's art file's path: in its own folder or in the common one.  Whether
 * the car has a folder of its own comes back. */
bool formatCarPath(Guest& guest, CPU& cpu, x86::reg32 path, x86::reg32 car, x86::reg32 folder,
                   const GuestString& own, const GuestString& common)
{
    if (guest.call(kCarInOwnFolder, args({ car })) != 0)
    {
        const x86::reg32 stem = guest::direct<0x439290, frontend::showcase::carFileStem>(guest.app(), cpu, car);
        guest.call(kSprintf, args({}, { path, own.address, kCarRootOwn, folder, stem }));
        return true;
    }
    guest.call(kSprintf, args({}, { path, common.address, kCarRootCommon, folder }));
    return false;
}

/* Whether a car has the art file: 1 or 0. */
x86::reg32 carFileExists(Guest& guest, CPU& cpu, x86::reg32 path, x86::reg32 car, x86::reg32 folder,
                         const GuestString& own, const GuestString& common)
{
    formatCarPath(guest, cpu, path, car, folder, own, common);
    return fileThere(guest, cpu, path) ? 1 : 0;
}

/* The colour a cleared bitmap takes: the palette's last in 8 bits, else black. */
x86::reg32 clearColour(const Guest& guest)
{
    return guest.byte(kVideoBits) == 8 ? 0xff : 0;
}

/* ---------------------------------------------------------------------------
 * showcase
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_453160: whether the car has an interior to show: none without sound. */
x86::reg32 frontend::showcase::hasInterior(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x100);
    const x86::reg32 folder = carFolder(guest, cpu, car);
    if (gameSettings(guest).installIsRemote != 0 || folder == 0)
        return 0;
    return carFileExists(guest, cpu, guest.frame(), car, folder, kInteriorOwn, kInteriorCommon);
}

namespace
{

/* sub_453200, sub_453290: whether the car has photographs, a history. */
x86::reg32 hasArt(Guest& guest, CPU& cpu, x86::reg32 car, const GuestString& own, const GuestString& common)
{
    const x86::reg32 folder = carFolder(guest, cpu, car);
    if (folder == 0)
        return 0;
    return carFileExists(guest, cpu, guest.frame(), car, folder, own, common);
}

/* The backdrop's mask and picture, by the number kBackdrop gives them, set on
 * the screen's items, from the file SH<car>.  `keepTexture`: each item's
 * picture is replaced where it is; otherwise a new one is taken.  The file's
 * freeing's answer, the eax it leaves. */
x86::reg32 loadBackdrop(Guest& guest, CPU& cpu, x86::reg32 menu, x86::reg32 folder, bool keepTexture)
{
    const x86::reg32 path = guest.frame();
    formatCarPath(guest, cpu, path, guest.dword(kCar), folder, kShowcaseOwn, kShowcaseCommon);
    guest.leaveEdx(0x10);  // what loadQfs hands on in edx to the file's loader: its flags
    const x86::reg32 art = fe::direct<0x4956f0, frontend::draw::loadQfs>(guest.app(), cpu, path);
    if (Item mask = fe::findItem(guest, menu, kShowcaseMask))
    {
        guest.call(kSprintf, args({}, { path, kMaskNumbered.address, guest.dword(kBackdrop) + 1 }));
        setItemShape(guest, cpu, mask, art, path, keepTexture ? itemTexture(guest, mask) : 0);
    }
    if (Item back = fe::findItem(guest, menu, kShowcaseBack))
    {
        guest.call(kSprintf, args({}, { path, kBackNumbered.address, guest.dword(kBackdrop) + 1 }));
        setItemShape(guest, cpu, back, art, path, keepTexture ? itemTexture(guest, back) : 0);
    }
    return freeBlock(guest, cpu, art);
}

}  // namespace

/* sub_453320: the backdrop again, its number changed. */
x86::reg32 frontend::showcase::loadArt(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x54);
    const x86::reg32 folder = carFolder(guest, cpu, guest.dword(kCar));
    if (folder == 0)
        return folder;   // eax as the lookup left it
    return loadBackdrop(guest, cpu, menu, folder, true);
}

/* sub_453500: the car the screen before this one was about; -1 when it is a
 * screen of the showcase's own, which keeps the one it has; the last answer
 * after any other. */
x86::reg32 frontend::showcase::resolveCar(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const auto cameFrom = [&](const GuestString& name) {
        return guest.call(kStrcmp, args({ guest.dword(kCameFrom), name.address })) == 0;
    };
    x86::reg32 car;
    if (cameFrom(kCar1Name))
    {
        car = gameSettings(guest).players[0].car;
    }
    else if (cameFrom(kCar2Name))
    {
        car = gameSettings(guest).players[1].car;
    }
    else if (cameFrom(kOpponentName))
    {
        const std::int32_t opponent = std::int32_t(gameSettings(guest).opponentClass);
        car = opponent > 3 ? x86::reg32(opponent) - kOpponentsFirstCar : gameSettings(guest).players[0].car;
    }
    else if (cameFrom(kShowcaseName) || cameFrom(kInteriorName))
    {
        car = 0xffffffff;
    }
    else
    {
        car = guest.dword(kResolvedCar);
    }
    guest.setDword(kResolvedCar, car);
    return car;
}

/* sub_4535e0, showcase as it opens: the car, its three buttons greyed out
 * where it has no art for them (the interior in a network game too), its
 * backdrop, its description spoken. */
x86::reg32 frontend::showcase::showcaseEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x50);
    fe::direct<0x410460, port::musicMenuSixOf>(app, cpu, cpu.esp - 4, cpu.eax);
    if (guest::direct<0x453500, resolveCar>(app, cpu) != 0xffffffff)
        guest.setDword(kCar, guest::direct<0x453500, resolveCar>(app, cpu));

    const x86::reg32 folder = carFolder(guest, cpu, guest.dword(kCar));
    guest.setDword(kSlide, 1);
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kShownBackdrop, guest.dword(kBackdrop));

    if (Item interior = fe::findItem(guest, menu, kInteriorName))
    {
        const bool has = guest::direct<0x453160, hasInterior>(app, cpu, guest.dword(kShownCar)) != 0;
        interior.setDisabled(!(has && guest.dword(kNetworkConnected) == 0));
    }
    if (Item slides = fe::findItem(guest, menu, kSlidesName))
        slides.setDisabled(guest::direct<0x453200, hasSlides>(app, cpu, guest.dword(kShownCar)) == 0);
    if (Item history = fe::findItem(guest, menu, kHistoryName))
        history.setDisabled(guest::direct<0x453290, hasHistory>(app, cpu, guest.dword(kShownCar)) == 0);

    if (folder != 0)
        loadBackdrop(guest, cpu, menu, folder, false);
    fe::direct<0x4c95f0, port::descriptionStarted>(app, cpu, cpu.esp - 4, guest.dword(kShownCar), kShowcaseVolume,
                                                   gameSettings(guest).language);
    return 0;
}

/* sub_4537d0, showcase on every pass: opened anew for another car; the
 * backdrop loaded again when its number changed. */
x86::reg32 frontend::showcase::showcaseFrame(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (guest.dword(kBackdrop) != guest.dword(kShownBackdrop))
    {
        guest.setDword(kShownBackdrop, guest.dword(kBackdrop));
        guest.setDword(kReloadArt, 1);
    }
    if (guest.dword(kCar) != guest.dword(kShownCar))
        return kReopen;
    if (guest.dword(kReloadArt) != 0)
    {
        guest.setDword(kReloadArt, 0);
        guest::direct<0x453320, loadArt>(app, cpu, menu);
    }
    return 0;
}

namespace
{

/* sub_453830, sub_4543e0, showcase and history as they close: the description stopped. */
x86::reg32 stopDescription(Guest& guest)
{
    // By address: the esi it leaves this leaves too.
    guest.call(kDescriptionStop);
    return 0;
}

/* ---------------------------------------------------------------------------
 * slides
 * ------------------------------------------------------------------------- */

/* A photograph's number as its file's name has it: two figures, a leading 0. */
void formatSlideNumber(Guest& guest, x86::reg32 number, x86::reg32 slide)
{
    guest.call(kSprintf, args({}, { number, kSlideNumber.address, slide }));
    if (guest.byte(number) == ' ')
        guest.setByte(number, '0');
}

/* A photograph's file's path. */
void formatSlidePath(Guest& guest, CPU& cpu, x86::reg32 path, x86::reg32 folder, x86::reg32 number)
{
    if (guest.call(kCarInOwnFolder, args({ guest.dword(kShownCar) })) != 0)
    {
        const x86::reg32 stem =
            guest::direct<0x439290, frontend::showcase::carFileStem>(guest.app(), cpu, guest.dword(kShownCar));
        guest.call(kSprintf, args({}, { path, kSlideOwn.address, kCarRootOwn, folder, stem, number }));
    }
    else
    {
        guest.call(kSprintf, args({}, { path, kSlideCommon.address, kCarRootCommon, folder, number }));
    }
}

}  // namespace

/* sub_453860, the button: the photograph before; from the first, the last that
 * exists of 19 and down. */
x86::reg32 frontend::showcase::previousSlide(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x10c);
    const x86::reg32 path = guest.frame();
    const x86::reg32 number = guest.frame() + 0x100;
    const x86::reg32 folder = carFolder(guest, cpu, guest.dword(kShownCar));
    const std::int32_t slide = std::int32_t(guest.dword(kSlide));
    if (slide > 1)
    {
        guest.setDword(kSlide, x86::reg32(slide - 1));
        return kReopen;
    }
    guest.setDword(kSlide, 20);
    do
    {
        const x86::reg32 tried = guest.dword(kSlide) - 1;
        guest.setDword(kSlide, tried);
        formatSlideNumber(guest, number, tried);
        formatSlidePath(guest, cpu, path, folder, number);
    } while (!fileThere(guest, cpu, path));
    return kReopen;
}

/* sub_453c00, slides as it opens: its buttons; the photograph (the first, if
 * this one is not there) on its bitmap; on all but the first a line and the
 * band over it and no make or model, on the first those two from the file, the
 * model hidden, for them to take turns. */
x86::reg32 frontend::showcase::slidesEnter(App* app, CPU& cpu, reg32 menu)
{
    constexpr x86::reg32 kLineColour = 0xff2141ad;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x64);
    const x86::reg32 path = guest.frame();
    const x86::reg32 number = guest.frame() + 0x50;
    const x86::reg32 folder = carFolder(guest, cpu, guest.dword(kShownCar));
    if (Item next = fe::findItem(guest, menu, kNextSlideName))
        next.setOnClick(kNextSlide);
    if (Item previous = fe::findItem(guest, menu, kPrevSlideName))
        previous.setOnClick(kPreviousSlide);
    if (folder != 0)
    {
        formatSlideNumber(guest, number, guest.dword(kSlide));
        formatSlidePath(guest, cpu, path, folder, number);
        if (!fileThere(guest, cpu, path))
        {
            guest.setDword(kSlide, 1);
            formatSlideNumber(guest, number, 1);
            formatSlidePath(guest, cpu, path, folder, number);
        }
        guest.leaveEdx(0x10);  // what loadQfs hands on in edx to the file's loader: its flags
        const x86::reg32 art = fe::direct<0x4956f0, frontend::draw::loadQfs>(app, cpu, path);
        const x86::reg32 back = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, art, kBack.address);
        // By address: the ecx it leaves reaches shapeTexture's, handed on.
        const x86::reg32 bitmap = guest.call(kNewBitmap, args({ 640, 480, 0x10, 0x10 }));
        fe::direct<0x4eada0, frontend::eaa::bitmapDrawInto>(app, cpu, bitmap);
        fe::direct<0x4ef1b0, frontend::eab::drawClearClip>(app, cpu, clearColour(guest));
        fe::direct<0x4f0a10, frontend::wata::blitShapeAtPosition>(app, cpu, back);
        if (std::int32_t(guest.dword(kSlide)) > 1)
        {
            // By address: the ecx it leaves can reach shapeTexture's, handed on.
            guest.call(kFillRect, args({ 0, 0x39, 640, 3 }, { kLineColour }));
            freeBlock(guest, cpu, art);
            guest.call(kSprintf, args({}, { path, kBandsFile.address, kCarRootCommon }));
            guest.leaveEdx(0x10);
            const x86::reg32 bands = fe::direct<0x4956f0, frontend::draw::loadQfs>(app, cpu, path);
            const x86::reg32 band = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, bands, kBand.address);
            // The blitter's shape, x and y in eax, edx and ebx, which the dispatcher hands on as they are.
            cpu.eax = band;
            guest.leaveEdx(0);
            guest.leaveEbx(0x182);
            fe::direct<0x4f09c0, frontend::wata::blitDispatch>(app, cpu);
            freeBlock(guest, cpu, bands);
            if (Item make = fe::findItem(guest, menu, kMake))
                setItemTexture(guest, make, 0);
            if (Item model = fe::findItem(guest, menu, kModel))
                setItemTexture(guest, model, 0);
        }
        else
        {
            if (Item make = fe::findItem(guest, menu, kMake))
            {
                setItemShape(guest, cpu, make, art, kMake.address, 0);
                make.setHidden(false);
            }
            if (Item model = fe::findItem(guest, menu, kModel))
            {
                setItemShape(guest, cpu, model, art, kModelShape.address, 0);
                model.setHidden(true);
            }
            freeBlock(guest, cpu, art);
        }
        if (Item slides = fe::findItem(guest, menu, kSlidesName))
            setItemTexture(guest, slides, shapeTexture(guest, cpu, guest.dword(bitmap + kBitmapShape), 0));
        fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, cpu);
        fe::direct<0x4eff00, frontend::eab::windowBitmapFree>(app, cpu, bitmap);
    }
    guest.setDword(kBlinkSince, guest.dword(kClock));
    return 0;
}

/* sub_453ef0, slides on every pass: make and model take turns, every 200 ticks. */
x86::reg32 frontend::showcase::slidesFrame(App* app, CPU& cpu, reg32 menu)
{
    constexpr x86::reg32 kBlinkTicks = 200;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (std::int32_t(guest.dword(kClock)) > std::int32_t(guest.dword(kBlinkSince) + kBlinkTicks))
    {
        const auto toggle = [&](const GuestString& name) {
            if (Item item = fe::findItem(guest, menu, name))
                guest.setByte(item.address() + fe::item::kFlags,
                              std::uint8_t(guest.byte(item.address() + fe::item::kFlags) ^ fe::item::kHidden));
        };
        toggle(kMake);
        toggle(kModel);
        guest.setDword(kBlinkSince, guest.dword(kClock));
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * history
 * ------------------------------------------------------------------------- */

/* sub_454000: the page of the car's history for this era, its picture and
 * mask (shapes in the file it hands back, to free) at the two addresses: 0
 * when the era has no text, the car is the Aspid, which has no history, or
 * there is no file.  They are named after the last two figures of the era's
 * text, a year, but for two of the Lstm's, and are "back" and "mask" when not
 * there. */
x86::reg32 frontend::showcase::historyPage(App* app, CPU& cpu, reg32 backOut, reg32 maskOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x110);
    const x86::reg32 path = guest.frame();
    const x86::reg32 folder = carFolder(guest, cpu, guest.dword(kShownCar));
    // By address: the edx it leaves this leaves too, when it goes no further.
    const x86::reg32 text = guest.call(kCarText, args({ guest.dword(kShownCar), guest.dword(kEra) + kFirstHistoryText }));
    if (text == 0 || folder == 0)
        return 0;
    if (guest.call(kStrncmp, args({ text, kAspid.address, 5 })) == 0)
        return 0;
    const std::uint32_t length = guest.length(text);
    if (length < 4)
        return 0;
    const x86::reg32 year = text + length - 2;

    const bool own = formatCarPath(guest, cpu, path, guest.dword(kShownCar), folder, kHistoryOwn, kHistoryCommon);
    // The folder went through edx on its way to sprintf, and this function does not restore edx.
    if (!own)
        guest.leaveEdx(folder);
    if (!fileThere(guest, cpu, path))
        return 0;

    guest.leaveEdx(0);  // what loadQfs hands on in edx to the file's loader: its flags
    const x86::reg32 art = fe::direct<0x4956f0, frontend::draw::loadQfs>(app, cpu, path);
    const auto isLstm = [&] { return guest.call(kStrcmp, args({ folder, kLstm.address })) == 0; };
    const auto locate = [&](x86::reg32 name) { return guest.call(kLocateShape, args({ art, name })); };
    x86::reg32 maskName;
    if (isLstm() && guest.dword(kEra) == 3)
    {
        guest.setDword(backOut, locate(kBack8b.address));
        maskName = kMask8b.address;
    }
    else if (isLstm() && guest.dword(kEra) == 4)
    {
        guest.setDword(backOut, locate(kBack89.address));
        maskName = kMask89.address;
    }
    else
    {
        guest.call(kSprintf, args({}, { path, kBackOfYear.address, year }));
        guest.setDword(backOut, locate(path));
        guest.call(kSprintf, args({}, { path, kMaskOfYear.address, year }));
        maskName = path;
    }
    guest.setDword(maskOut, locate(maskName));
    if (guest.dword(backOut) == 0 || guest.dword(maskOut) == 0)
    {
        guest.setDword(backOut, locate(kBack.address));
        guest.setDword(maskOut, locate(kMask.address));
    }
    guest.leaveEdx(maskOut);
    if (guest.dword(backOut) == 0 || guest.dword(maskOut) == 0)
    {
        freeBlock(guest, cpu, art);
        return 0;
    }
    return art;
}

/* sub_454210: the page of this era or of the first after it that has one,
 * round the six; 0 when none has. */
x86::reg32 frontend::showcase::historyPageFrom(App* app, CPU& cpu, reg32 backOut, reg32 maskOut)
{
    constexpr std::int32_t kEras = 6;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const x86::reg32 first = guest.dword(kEra);
    for (;;)
    {
        const x86::reg32 art = guest::direct<0x454000, historyPage>(app, cpu, backOut, maskOut);
        if (art != 0)
            return art;
        const x86::reg32 next = guest.dword(kEra) + 1;
        guest.setDword(kEra, next);
        if (std::int32_t(next) > kEras)
            guest.setDword(kEra, 1);
        if (first == guest.dword(kEra))
        {
            guest.leaveEdx(next);
            return 0;
        }
    }
}

namespace
{

/* A page's shapes on the screen's two items, `keepTexture` as loadBackdrop's. */
void showHistoryPage(Guest& guest, CPU& cpu, x86::reg32 menu, x86::reg32 art, x86::reg32 back, x86::reg32 mask,
                     bool keepTexture)
{
    if (Item item = fe::findItem(guest, menu, kHistoryMask))
    {
        const x86::reg32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, shapeTexture(guest, cpu, guest.dword(mask), replaced));
    }
    if (Item item = fe::findItem(guest, menu, kHistoryBack))
    {
        const x86::reg32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, shapeTexture(guest, cpu, guest.dword(back), replaced));
    }
    freeBlock(guest, cpu, art);
}

}  // namespace

/* sub_454260, history as it opens: from the first era, its description spoken. */
x86::reg32 frontend::showcase::historyEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 8);
    const x86::reg32 back = guest.frame();
    const x86::reg32 mask = guest.frame() + 4;
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kEra, 1);
    if (const x86::reg32 art = guest::direct<0x454210, historyPageFrom>(app, cpu, back, mask))
        showHistoryPage(guest, cpu, menu, art, back, mask, false);
    guest.setDword(kPageSince, guest.dword(kClock));
    fe::direct<0x4c95f0, port::descriptionStarted>(app, cpu, cpu.esp - 4, guest.dword(kShownCar), kHistoryVolume,
                                                   gameSettings(guest).language);
    return 0;
}

/* sub_454310, history on every pass: opened anew for another car; the next
 * era's page every 600 ticks. */
x86::reg32 frontend::showcase::historyFrame(App* app, CPU& cpu, reg32 menu)
{
    constexpr x86::reg32 kPageTicks = 600;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const x86::reg32 back = guest.frame();
    const x86::reg32 mask = guest.frame() + 4;
    if (guest.dword(kShownCar) != guest.dword(kCar))
        return kReopen;
    if (std::int32_t(guest.dword(kClock)) > std::int32_t(guest.dword(kPageSince) + kPageTicks))
    {
        guest.setDword(kEra, guest.dword(kEra) + 1);
        if (const x86::reg32 art = guest::direct<0x454210, historyPageFrom>(app, cpu, back, mask))
            showHistoryPage(guest, cpu, menu, art, back, mask, true);
        guest.setDword(kPageSince, guest.dword(kClock));
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * compare and compare2
 * ------------------------------------------------------------------------- */

/* sub_4543f0: a car's picture for compare, centred on the bottom of a bitmap
 * of 288x224, as the texture it replaces or a new one.  The picture goes by
 * the car's stem in lower case, which the record is left with. */
x86::reg32 frontend::showcase::comparePicture(App* app, CPU& cpu, reg32 car, reg32 replaced)
{
    constexpr std::int32_t kWide = 0x120, kHigh = 0xe0;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x6c);
    const x86::reg32 path = guest.frame();
    guest.call(kSprintf, args({}, { path, kTwoStrings.address, kArtFolder, kCompareFile.address }));
    const x86::reg32 art =
        x86::reg32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, cpu, cpu.esp - 4, path, cpu.ebx));
    const x86::reg32 stem = guest::direct<0x439290, carFileStem>(app, cpu, car);
    for (x86::reg32 at = stem; guest.byte(at) != 0; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (guest.byte(kCharTraits + std::uint8_t(c + 1)) & kCharUpper)
            guest.setByte(at, std::uint8_t(guest.call(kTolower, args({ c }))));
    }
    // By address: the ecx it leaves reaches shapeTexture's, handed on.
    const x86::reg32 bitmap = guest.call(kNewBitmap, args({ kWide, kHigh, 0x20, 0x10 }));
    fe::direct<0x4eada0, frontend::eaa::bitmapDrawInto>(app, cpu, bitmap);
    fe::direct<0x4ef1b0, frontend::eab::drawClearClip>(app, cpu, clearColour(guest));
    if (stem != 0)
    {
        if (const x86::reg32 shape = guest.call(kLocateShape, args({ art, stem })))
        {
            const std::int32_t left = kWide - std::int16_t(guest.word(shape + 4));
            const std::int32_t top = kHigh - std::int16_t(guest.word(shape + 6));
            fe::direct<0x4ef4c0, frontend::eab::shapeDrawKeyedAtHotspot>(app, cpu, shape, x86::reg32(left / 2),
                                                                         x86::reg32(top));
            fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, cpu);
        }
    }
    const x86::reg32 texture = shapeTexture(guest, cpu, guest.dword(bitmap + kBitmapShape), replaced);
    fe::direct<0x4eff00, frontend::eab::windowBitmapFree>(app, cpu, bitmap);
    guest.leaveEdx(texture);
    return texture;
}

namespace
{

/* The two cars to compare, unless the screen comes back from the other page:
 * the one the screen before was about, and the opponent's on the right if it
 * is a car of the game's, or the same again.  sub_453500's -1 is taken for a
 * car; only a car 0 keeps the old one, as the game has it. */
void chooseCompareCars(Guest& guest, CPU& cpu)
{
    if (guest.dword(kLastAction) == kReopen)
        return;
    if (guest::direct<0x453500, frontend::showcase::resolveCar>(guest.app(), cpu) != 0)
        guest.setDword(kCompareLeftCar, guest::direct<0x453500, frontend::showcase::resolveCar>(guest.app(), cpu));
    const std::int32_t opponent = std::int32_t(gameSettings(guest).opponentClass);
    guest.setDword(kCompareRightCar, opponent > 3 ? x86::reg32(opponent) - kOpponentsFirstCar : guest.dword(kCompareLeftCar));
}

}  // namespace

/* sub_454500, compare as it opens: the cars and their pictures. */
x86::reg32 frontend::showcase::compareEnter(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    chooseCompareCars(guest, cpu);
    guest.setDword(kCompareLeftShown, guest.dword(kCompareLeftCar));
    guest.setDword(kCompareRightShown, guest.dword(kCompareRightCar));
    guest.setDword(kCompareLeftPicture,
                   guest::direct<0x4543f0, comparePicture>(app, cpu, guest.dword(kCompareLeftCar), 0));
    guest.setDword(kCompareRightPicture,
                   guest::direct<0x4543f0, comparePicture>(app, cpu, guest.dword(kCompareRightCar), 0));
    return 0;
}

/* sub_454690, compare on every pass: on to compare2 when asked, and a picture
 * again for a car changed. */
x86::reg32 frontend::showcase::compareFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    x86::reg32 result = 0;
    if (guest.dword(kCompareFigures) != 0)
    {
        guest.setDword(kAfterScreen, kCompare2Name.address);
        result = kReopen;
    }
    const auto follow = [&](x86::reg32 car, x86::reg32 shown, x86::reg32 picture) {
        const x86::reg32 now = guest.dword(car);
        if (now != guest.dword(shown))
        {
            guest.setDword(shown, now);
            guest.setDword(picture, guest::direct<0x4543f0, comparePicture>(app, cpu, now, guest.dword(picture)));
        }
    };
    follow(kCompareLeftCar, kCompareLeftShown, kCompareLeftPicture);
    follow(kCompareRightCar, kCompareRightShown, kCompareRightPicture);
    return result;
}

/* sub_454580, compare2 as it opens: the cars, the bar and the four arrows. */
x86::reg32 frontend::showcase::compare2Enter(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x50);
    const x86::reg32 path = guest.frame();
    chooseCompareCars(guest, cpu);
    guest.setDword(kCompare5d4, 0);
    guest.setDword(kCompare574, 0);
    guest.call(kSprintf, args({}, { path, kCompareArt.address, kArtFolder }));
    const x86::reg32 art =
        x86::reg32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, cpu, cpu.esp - 4, path, cpu.ebx));
    const GuestString& bar = (guest.byte(kDisplayFlags) & kNoTranslucency) ? kBarOpaque : kBarBlended;
    const x86::reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, art, bar.address);
    guest.setDword(kCompareBar, shapeTexture(guest, cpu, shape, 0));
    for (std::size_t i = 0; i < std::size(kCompareArrows); ++i)
        guest.setWord(kCompareArrows[i], std::uint16_t(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(
                                             app, cpu, cpu.esp - 4, kArrowNames[i].address)));
    return 0;
}

/* sub_454660, compare2 on every pass: back to compare when asked. */
x86::reg32 frontend::showcase::compare2Frame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (guest.dword(kCompareFigures) != 0)
        return 0;
    guest.setDword(kAfterScreen, kCompareName.address);
    return kReopen;
}

/* ---------------------------------------------------------------------------
 * interior
 * ------------------------------------------------------------------------- */

/* sub_454cf0, the zoom slider moved: its value to the view, off the x87 and
 * as a float on the stack. */
x86::reg32 frontend::showcase::zoomMoved(App* app, CPU& cpu, reg32 slider)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, cpu, slider, 0);
    const float value = guest.popFloat();
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    // By address: the eax it leaves is this function's answer, and its core hands back none.
    guest.call(kSetInteriorZoom, args({}, { bits }));
    return guest.eax();
}

/* sub_454d10, interior as it opens: the slider's handler, the slider at 0. */
x86::reg32 frontend::showcase::interiorEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const x86::reg32 slider = fe::findItem(guest, menu, kZoomSlider).address();
    fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, cpu, slider, kZoomMoved);
    fe::direct<0x46c950, port::fields::sliderSetValueOf>(app, cpu, cpu.esp - 4 - 4 * 2, slider, 0, 0);
    return 0;
}

/* sub_453200, sub_453290: whether the car has photographs, a history. */
x86::reg32 frontend::showcase::hasSlides(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x100);
    return hasArt(guest, cpu, car, kSlidesOwn, kSlidesCommon);
}

x86::reg32 frontend::showcase::hasHistory(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x100);
    return hasArt(guest, cpu, car, kHistoryOwn, kHistoryCommon);
}

/* sub_453830, sub_4543e0, showcase and history as they close. */
x86::reg32 frontend::showcase::showcaseExit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return stopDescription(guest);
}

x86::reg32 frontend::showcase::historyExit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return stopDescription(guest);
}

/* sub_453840, the button: the next photograph; slides, opened again, goes
 * back to the first if there is no such one. */
x86::reg32 frontend::showcase::nextSlide(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kSlide, guest.dword(kSlide) + 1);
    return kReopen;
}

namespace
{

const GuestString kHasInteriorStrings[] = { kInteriorOwn, kInteriorCommon };
const GuestString kHasSlidesStrings[] = { kSlidesOwn, kSlidesCommon };
const GuestString kHasHistoryStrings[] = { kHistoryOwn, kHistoryCommon };
const GuestString kLoadArtStrings[] = { kShowcaseOwn, kShowcaseCommon, kShowcaseMask, kMaskNumbered, kShowcaseBack, kBackNumbered };
const GuestString kResolveStrings[] = { kCar1Name, kCar2Name, kOpponentName, kShowcaseName, kInteriorName };
const GuestString kShowcaseStrings[] = { kInteriorName, kSlidesName, kHistoryName, kShowcaseOwn, kShowcaseCommon,
                                         kShowcaseMask, kMaskNumbered, kShowcaseBack, kBackNumbered };
const GuestString kPreviousStrings[] = { kSlideNumber, kSlideOwn, kSlideCommon };
const GuestString kSlidesStrings[] = { kNextSlideName, kPrevSlideName, kSlideNumber, kSlideOwn, kSlideCommon, kBack,
                                       kBandsFile, kBand, kMake, kModel, kModelShape, kSlidesName };
const GuestString kBlinkStrings[] = { kMake, kModel };
const GuestString kPageStrings[] = { kAspid, kHistoryOwn, kHistoryCommon, kLstm, kBack8b, kMask8b, kBack89, kMask89,
                                     kBackOfYear, kMaskOfYear, kBack, kMask };
const GuestString kHistoryStrings[] = { kHistoryMask, kHistoryBack };
const GuestString kPictureStrings[] = { kTwoStrings, kCompareFile };
const GuestString kCompare2Strings[] = { kCompareArt, kBarOpaque, kBarBlended, kArrowNames[0], kArrowNames[1],
                                         kArrowNames[2], kArrowNames[3] };
const GuestString kCompareFrameStrings[] = { kCompare2Name };
const GuestString kCompare2FrameStrings[] = { kCompareName };
const GuestString kInteriorStrings[] = { kZoomSlider };

constexpr std::uint8_t kRestoreBcd = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kRestoreCd = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kRestoreBcSiDi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

/* A screen's handler with nothing to do: slides', compare2's and interior's. */
void nothing(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site)
{
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

}

void frontend::showcaseCarFileStem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_439290", 0, 0, nullptr, 0, 0 };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::carFileStem(app, cpu, car); });
}

void frontend::showcaseHasInterior(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453160", 0, 0, kHasInteriorStrings, std::size(kHasInteriorStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::hasInterior(app, cpu, car); });
}

void frontend::showcaseHasSlides(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453200", 0, 0, kHasSlidesStrings, std::size(kHasSlidesStrings), kRestoreBcd };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::hasSlides(app, cpu, car); });
}

void frontend::showcaseHasHistory(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453290", 0, 0, kHasHistoryStrings, std::size(kHasHistoryStrings), kRestoreBcd };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::hasHistory(app, cpu, car); });
}

void frontend::showcaseLoadArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453320", 0, 0, kLoadArtStrings, std::size(kLoadArtStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::loadArt(app, cpu, menu); });
}

void frontend::showcaseResolveCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453500", 0, 0, kResolveStrings, std::size(kResolveStrings), kRestoreCd };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::resolveCar(app, cpu); });
}

void frontend::showcaseOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4535e0", 0, 0, kShowcaseStrings, std::size(kShowcaseStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::showcaseEnter(app, cpu, menu); });
}

void frontend::showcaseOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4537d0", 0, 0, nullptr, 0 };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::showcaseFrame(app, cpu, menu); });
}

void frontend::showcaseOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453830", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::showcaseExit(app, cpu); });
}

void frontend::showcaseNextSlide(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453840", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::nextSlide(app, cpu); });
}

void frontend::showcasePreviousSlide(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453860", 0, 0, kPreviousStrings, std::size(kPreviousStrings),
                          kRestoreBcd | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::previousSlide(app, cpu); });
}

void frontend::showcaseSlidesOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453c00", 0, 0, kSlidesStrings, std::size(kSlidesStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::slidesEnter(app, cpu, menu); });
}

void frontend::showcaseSlidesOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453ef0", 0, 0, kBlinkStrings, std::size(kBlinkStrings), kRestoreBcd };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::slidesFrame(app, cpu, menu); });
}

void frontend::showcaseSlidesOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453f50", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

void frontend::showcaseHistoryPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454000", 0, 0, kPageStrings, std::size(kPageStrings), kRestoreBcSiDi };
    const x86::reg32 back = cpu.eax, mask = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::historyPage(app, cpu, back, mask); });
}

void frontend::showcaseHistoryPageFrom(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454210", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const x86::reg32 back = cpu.eax, mask = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::historyPageFrom(app, cpu, back, mask); });
}

void frontend::showcaseHistoryOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454260", 0, 0, kHistoryStrings, std::size(kHistoryStrings), kRestoreBcd | fe::kRestoreEsi };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::historyEnter(app, cpu, menu); });
}

void frontend::showcaseHistoryOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454310", 0, 0, kHistoryStrings, std::size(kHistoryStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::historyFrame(app, cpu, menu); });
}

void frontend::showcaseHistoryOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4543e0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::historyExit(app, cpu); });
}

void frontend::showcaseComparePicture(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4543f0", 0, 0, kPictureStrings, std::size(kPictureStrings), kRestoreBcSiDi };
    const x86::reg32 car = cpu.eax, replaced = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::comparePicture(app, cpu, car, replaced); });
}

void frontend::showcaseCompareOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454500", 0, 0, nullptr, 0, kRestoreCd };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::compareEnter(app, cpu); });
}

void frontend::showcaseCompareOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454690", 0, 0, kCompareFrameStrings, std::size(kCompareFrameStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::compareFrame(app, cpu); });
}

void frontend::showcaseCompare2OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454580", 0, 0, kCompare2Strings, std::size(kCompare2Strings), kRestoreBcd };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::compare2Enter(app, cpu); });
}

void frontend::showcaseCompare2OnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454660", 0, 0, kCompare2FrameStrings, std::size(kCompare2FrameStrings), kRestoreCd };
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::compare2Frame(app, cpu); });
}

void frontend::showcaseCompare2OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454710", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

void frontend::showcaseZoomMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454cf0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const x86::reg32 slider = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::zoomMoved(app, cpu, slider); });
}

void frontend::showcaseInteriorOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454d10", 0, 0, kInteriorStrings, std::size(kInteriorStrings), kRestoreCd };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return showcase::interiorEnter(app, cpu, menu); });
}

void frontend::showcaseInteriorOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454d40", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

void frontend::showcaseInteriorOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454d50", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

}
