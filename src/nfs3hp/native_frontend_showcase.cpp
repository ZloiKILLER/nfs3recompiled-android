#include "port_view.h"
#include "native_frontend_showcase.h"
#include "native_watcom.h"
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
 * a frame of its own where the game's function has one (fe::Frame), and a thunk under the
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
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;

// The screens' state.
constexpr nfs::u32 kCar = 0x664794;              // the car to show
constexpr nfs::u32 kShownCar = 0x664798;         // the car shown
constexpr nfs::u32 kBackdrop = 0x55957c;         // which of the car's backdrops
constexpr nfs::u32 kShownBackdrop = 0x559584;
constexpr nfs::u32 kReloadArt = 0x559580;
constexpr nfs::u32 kCameFrom = 0x559578;         // char*, the screen before this one
constexpr nfs::u32 kResolvedCar = 0x5595a8;      // the last answer of sub_453500
constexpr nfs::u32 kSlide = 0x559588;            // the photograph shown, from 1
constexpr nfs::u32 kBlinkSince = 0x559594;
constexpr nfs::u32 kEra = 0x55958c;              // the history's page, 1 to 6
constexpr nfs::u32 kPageSince = 0x559590;
constexpr nfs::u32 kCompareLeftCar = 0x664788;
constexpr nfs::u32 kCompareRightCar = 0x664784;
constexpr nfs::u32 kCompareLeftShown = 0x66478c;
constexpr nfs::u32 kCompareRightShown = 0x664790;
constexpr nfs::u32 kCompareLeftPicture = 0x5595ac;
constexpr nfs::u32 kCompareRightPicture = 0x5595b0;
constexpr nfs::u32 kCompareFigures = 0x559570;   // nonzero: compare2's page, 0: compare's
constexpr nfs::u32 kCompare5d4 = 0x5595d4;
constexpr nfs::u32 kCompare574 = 0x559574;
constexpr nfs::u32 kCompareBar = 0x5595b8;       // the bar's texture
constexpr nfs::u32 kCompareArrows[] = { 0x66479c, 0x6647a2, 0x6647a0, 0x66479e };  // int16 each

// The game's.
constexpr nfs::u32 kOpponentsFirstCar = 4;
constexpr nfs::u32 kNetworkConnected = 0x7a22e0;
constexpr nfs::u32 kClock = 0x79c1a4;
constexpr nfs::u32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr nfs::u32 kVideoBits = 0x565010;        // byte, a pixel's
constexpr nfs::u32 kLastAction = 0x66296c;       // what left the last screen
constexpr nfs::u32 kAfterScreen = 0x559224;      // char*, the screen to go on to
constexpr nfs::u32 kCharTraits = 0x564ef0;       // the C library's, by character + 1
constexpr std::uint8_t kCharUpper = 0x40;
constexpr nfs::u32 kCarRootOwn = 0x7a28b4;       // the folder of the cars' own folders
constexpr nfs::u32 kCarRootCommon = 0x7a3234;    // the folder of the others' art
constexpr nfs::u32 kArtFolder = 0x7a2db4;

constexpr nfs::u32 kItemTexture = 0x44;          // int16 of an item: its picture
constexpr nfs::u32 kReopen = 9;                  // to the menu loop: closes without fading out
constexpr nfs::u32 kCarFileStemField = 0x315;    // of a car's record
constexpr nfs::u32 kFirstHistoryText = 0x17;     // the car's texts, a line an era

constexpr nfs::u32 kCarText = 0x439ab0;          // eax the car, edx the line
constexpr nfs::u32 kSprintf = 0x4df690;
constexpr nfs::u32 kStrcmp = 0x4ee310;
constexpr nfs::u32 kStrncmp = 0x4eb390;
constexpr nfs::u32 kTolower = 0x4f0ae0;
constexpr nfs::u32 kLocateShape = 0x4ef270;      // eax the file, edx a name, 0 when it has none by it
// A bitmap to paint a screen's picture into: made, drawn to, its shape at
// +0x20 made a texture, freed.
constexpr nfs::u32 kNewBitmap = 0x4ef190;        // eax wide, edx high, ebx bits, ecx
constexpr nfs::u32 kBitmapShape = 0x20;
constexpr nfs::u32 kFillRect = 0x4f0a90;         // eax x, edx y, ebx wide, ecx high, the colour on the stack
// A car's description, spoken.
constexpr nfs::u32 kDescriptionStop = 0x4c9490;
constexpr std::uint32_t kShowcaseVolume = 0x32;
constexpr std::uint32_t kHistoryVolume = 0x34;
// The interior's view.
constexpr nfs::u32 kSetInteriorZoom = 0x4646e0;  // a float on the stack

// This group's own, handed to the menu engine as handlers.
constexpr nfs::u32 kNextSlide = 0x453840;
constexpr nfs::u32 kPreviousSlide = 0x453860;
constexpr nfs::u32 kZoomMoved = 0x454cf0;

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
nfs::u32 itemTexture(Guest& guest, const Item& item)
{
    return nfs::u32(std::int32_t(std::int16_t(guest.word(item.address() + kItemTexture))));
}

void setItemTexture(Guest& guest, const Item& item, nfs::u32 texture)
{
    guest.setWord(item.address() + kItemTexture, std::uint16_t(texture));
}

/* A shape made textures, `replaced` the record to fill again (0 a new one); ecx as the code has it,
 * handed on. */
nfs::u32 shapeTexture(Guest& guest, ThreadState& ts, nfs::u32 shape, nfs::u32 replaced)
{
    return nfs::u32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(guest.app(), ts, ts.sp - 4, shape,
                                                                         std::int32_t(replaced), 0u));  // ecx handed on: it reads none
}

/* A shape of a loaded file as an item's picture. */
void setItemShape(Guest& guest, ThreadState& ts, const Item& item, nfs::u32 file, nfs::u32 name, nfs::u32 replaced)
{
    const nfs::u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(guest.app(), ts, file, name);
    setItemTexture(guest, item, shapeTexture(guest, ts, shape, replaced));
}

/* What a car's folder's name is at, 0 for none. */
nfs::u32 carFolder(Guest& guest, ThreadState& ts, nfs::u32 car)
{
    return *fe::direct<0x4392b0, load::folderOf>(guest.app(), ts, car);
}

/* Whether the file named at `path` is there. */
bool fileThere(Guest& guest, ThreadState& ts, nfs::u32 path)
{
    return *fe::direct<0x4e0e60, ea::fileThere>(guest.app(), ts, path);
}

/* The block at `body` given back: purgememadr's answer. */
nfs::u32 freeBlock(Guest& guest, ThreadState& ts, nfs::u32 body)
{
    return *fe::direct<0x4e1890, ea::purgeBlock>(guest.app(), ts, body);
}

}  // namespace

/* sub_439290: a car's file stem, in its record: 0 when it has no record. */
nfs::u32 frontend::showcase::carFileStem(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const nfs::u32 record = fe::direct<0x439a50, load::recordAt>(app, ts, car);
    return record != 0 ? record + kCarFileStemField : 0;
}

namespace
{

/* A car's art file's path: in its own folder or in the common one.  Whether
 * the car has a folder of its own comes back. */
bool formatCarPath(Guest& guest, ThreadState& ts, nfs::u32 path, nfs::u32 car, nfs::u32 folder,
                   const GuestString& own, const GuestString& common)
{
    if (*fe::direct<0x4395c0, load::flag4Of>(guest.app(), ts, car) != 0)
    {
        const nfs::u32 stem = guest::direct<0x439290, frontend::showcase::carFileStem>(guest.app(), ts, car);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, own.address, guest::Words{kCarRootOwn, folder, stem});
        return true;
    }
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, common.address, guest::Words{kCarRootCommon, folder});
    return false;
}

/* Whether a car has the art file: 1 or 0. */
nfs::u32 carFileExists(Guest& guest, ThreadState& ts, nfs::u32 path, nfs::u32 car, nfs::u32 folder,
                         const GuestString& own, const GuestString& common)
{
    formatCarPath(guest, ts, path, car, folder, own, common);
    return fileThere(guest, ts, path) ? 1 : 0;
}

/* The colour a cleared bitmap takes: the palette's last in 8 bits, else black. */
nfs::u32 clearColour(const Guest& guest)
{
    return guest.byte(kVideoBits) == 8 ? 0xff : 0;
}

/* ---------------------------------------------------------------------------
 * showcase
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_453160: whether the car has an interior to show: none without sound. */
nfs::u32 frontend::showcase::hasInterior(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x100);
    const nfs::u32 folder = carFolder(guest, ts, car);
    if (gameSettings(guest).installIsRemote != 0 || folder == 0)
        return 0;
    return carFileExists(guest, ts, guest.frame(), car, folder, kInteriorOwn, kInteriorCommon);
}

namespace
{

/* sub_453200, sub_453290: whether the car has photographs, a history. */
nfs::u32 hasArt(Guest& guest, ThreadState& ts, nfs::u32 car, const GuestString& own, const GuestString& common)
{
    const nfs::u32 folder = carFolder(guest, ts, car);
    if (folder == 0)
        return 0;
    return carFileExists(guest, ts, guest.frame(), car, folder, own, common);
}

/* The backdrop's mask and picture, by the number kBackdrop gives them, set on
 * the screen's items, from the file SH<car>.  `keepTexture`: each item's
 * picture is replaced where it is; otherwise a new one is taken.  The file's
 * freeing's answer, the eax it leaves. */
nfs::u32 loadBackdrop(Guest& guest, ThreadState& ts, nfs::u32 menu, nfs::u32 folder, bool keepTexture)
{
    const nfs::u32 path = guest.frame();
    formatCarPath(guest, ts, path, guest.dword(kCar), folder, kShowcaseOwn, kShowcaseCommon);
    const nfs::u32 art = fe::direct<0x4956f0, frontend::draw::loadQfsFlags>(guest.app(), ts, path, 0x10u);
    if (Item mask = fe::findItem(guest, menu, kShowcaseMask))
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kMaskNumbered.address, guest::Words{guest.dword(kBackdrop) + 1});
        setItemShape(guest, ts, mask, art, path, keepTexture ? itemTexture(guest, mask) : 0);
    }
    if (Item back = fe::findItem(guest, menu, kShowcaseBack))
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kBackNumbered.address, guest::Words{guest.dword(kBackdrop) + 1});
        setItemShape(guest, ts, back, art, path, keepTexture ? itemTexture(guest, back) : 0);
    }
    return freeBlock(guest, ts, art);
}

}  // namespace

/* sub_453320: the backdrop again, its number changed. */
nfs::u32 frontend::showcase::loadArt(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x54);
    const nfs::u32 folder = carFolder(guest, ts, guest.dword(kCar));
    if (folder == 0)
        return folder;   // eax as the lookup left it
    return loadBackdrop(guest, ts, menu, folder, true);
}

/* sub_453500: the car the screen before this one was about; -1 when it is a
 * screen of the showcase's own, which keeps the one it has; the last answer
 * after any other. */
nfs::u32 frontend::showcase::resolveCar(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const auto cameFrom = [&](const GuestString& name) {
        return fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), guest.dword(kCameFrom), name.address) == 0;
    };
    nfs::u32 car;
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
        car = opponent > 3 ? nfs::u32(opponent) - kOpponentsFirstCar : gameSettings(guest).players[0].car;
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
nfs::u32 frontend::showcase::showcaseEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x50);
    fe::direct<0x410460, port::musicMenuSixOf>(app, ts, ts.sp - 4, menu);  // eax: the menu, as it came
    if (guest::direct<0x453500, resolveCar>(app, ts) != 0xffffffff)
        guest.setDword(kCar, guest::direct<0x453500, resolveCar>(app, ts));

    const nfs::u32 folder = carFolder(guest, ts, guest.dword(kCar));
    guest.setDword(kSlide, 1);
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kShownBackdrop, guest.dword(kBackdrop));

    if (Item interior = fe::findItem(guest, menu, kInteriorName))
    {
        const bool has = guest::direct<0x453160, hasInterior>(app, ts, guest.dword(kShownCar)) != 0;
        interior.setDisabled(!(has && guest.dword(kNetworkConnected) == 0));
    }
    if (Item slides = fe::findItem(guest, menu, kSlidesName))
        slides.setDisabled(guest::direct<0x453200, hasSlides>(app, ts, guest.dword(kShownCar)) == 0);
    if (Item history = fe::findItem(guest, menu, kHistoryName))
        history.setDisabled(guest::direct<0x453290, hasHistory>(app, ts, guest.dword(kShownCar)) == 0);

    if (folder != 0)
        loadBackdrop(guest, ts, menu, folder, false);
    fe::direct<0x4c95f0, port::descriptionStarted>(app, ts, ts.sp - 4, guest.dword(kShownCar), kShowcaseVolume,
                                                   gameSettings(guest).language);
    return 0;
}

/* sub_4537d0, showcase on every pass: opened anew for another car; the
 * backdrop loaded again when its number changed. */
nfs::u32 frontend::showcase::showcaseFrame(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
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
        guest::direct<0x453320, loadArt>(app, ts, menu);
    }
    return 0;
}

namespace
{

/* sub_453830, sub_4543e0, showcase and history as they close: the description stopped. */
nfs::u32 stopDescription(Guest& guest)
{
    // By address: the esi it leaves this leaves too.
    guest.call(kDescriptionStop);
    return 0;
}

/* ---------------------------------------------------------------------------
 * slides
 * ------------------------------------------------------------------------- */

/* A photograph's number as its file's name has it: two figures, a leading 0. */
void formatSlideNumber(Guest& guest, nfs::u32 number, nfs::u32 slide)
{
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), number, kSlideNumber.address, guest::Words{slide});
    if (guest.byte(number) == ' ')
        guest.setByte(number, '0');
}

/* A photograph's file's path. */
void formatSlidePath(Guest& guest, ThreadState& ts, nfs::u32 path, nfs::u32 folder, nfs::u32 number)
{
    if (*fe::direct<0x4395c0, load::flag4Of>(guest.app(), ts, guest.dword(kShownCar)) != 0)
    {
        const nfs::u32 stem =
            guest::direct<0x439290, frontend::showcase::carFileStem>(guest.app(), ts, guest.dword(kShownCar));
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kSlideOwn.address, guest::Words{kCarRootOwn, folder, stem, number});
    }
    else
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kSlideCommon.address, guest::Words{kCarRootCommon, folder, number});
    }
}

}  // namespace

/* sub_453860, the button: the photograph before; from the first, the last that
 * exists of 19 and down. */
nfs::u32 frontend::showcase::previousSlide(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x10c);
    const nfs::u32 path = guest.frame();
    const nfs::u32 number = guest.frame() + 0x100;
    const nfs::u32 folder = carFolder(guest, ts, guest.dword(kShownCar));
    const std::int32_t slide = std::int32_t(guest.dword(kSlide));
    if (slide > 1)
    {
        guest.setDword(kSlide, nfs::u32(slide - 1));
        return kReopen;
    }
    guest.setDword(kSlide, 20);
    do
    {
        const nfs::u32 tried = guest.dword(kSlide) - 1;
        guest.setDword(kSlide, tried);
        formatSlideNumber(guest, number, tried);
        formatSlidePath(guest, ts, path, folder, number);
    } while (!fileThere(guest, ts, path));
    return kReopen;
}

/* sub_453c00, slides as it opens: its buttons; the photograph (the first, if
 * this one is not there) on its bitmap; on all but the first a line and the
 * band over it and no make or model, on the first those two from the file, the
 * model hidden, for them to take turns. */
nfs::u32 frontend::showcase::slidesEnter(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kLineColour = 0xff2141ad;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x64);
    const nfs::u32 path = guest.frame();
    const nfs::u32 number = guest.frame() + 0x50;
    const nfs::u32 folder = carFolder(guest, ts, guest.dword(kShownCar));
    if (Item next = fe::findItem(guest, menu, kNextSlideName))
        next.setOnClick(kNextSlide);
    if (Item previous = fe::findItem(guest, menu, kPrevSlideName))
        previous.setOnClick(kPreviousSlide);
    if (folder != 0)
    {
        formatSlideNumber(guest, number, guest.dword(kSlide));
        formatSlidePath(guest, ts, path, folder, number);
        if (!fileThere(guest, ts, path))
        {
            guest.setDword(kSlide, 1);
            formatSlideNumber(guest, number, 1);
            formatSlidePath(guest, ts, path, folder, number);
        }
        const nfs::u32 art = fe::direct<0x4956f0, frontend::draw::loadQfsFlags>(app, ts, path, 0x10u);
        const nfs::u32 back = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, kBack.address);
        // By address: the ecx it leaves reaches shapeTexture's, handed on.
        const nfs::u32 bitmap = guest.call(kNewBitmap, args({ 640, 480, 0x10, 0x10 }));
        fe::direct<0x4eada0, frontend::eaa::bitmapDrawInto>(app, ts, bitmap);
        fe::direct<0x4ef1b0, frontend::eab::drawClearClip>(app, ts, clearColour(guest));
        fe::direct<0x4f0a10, frontend::wata::blitShapeAtPosition>(app, ts, back);
        if (std::int32_t(guest.dword(kSlide)) > 1)
        {
            // By address: the ecx it leaves can reach shapeTexture's, handed on.
            guest.call(kFillRect, args({ 0, 0x39, 640, 3 }, { kLineColour }));
            freeBlock(guest, ts, art);
            fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kBandsFile.address,
                                                        guest::Words{kCarRootCommon});
            const nfs::u32 bands = fe::direct<0x4956f0, frontend::draw::loadQfsFlags>(app, ts, path, 0x10u);
            const nfs::u32 band = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, bands, kBand.address);
            fe::direct<0x4f09c0, frontend::wata::blitDispatchAt>(app, ts, band, 0u, 0x182u);
            freeBlock(guest, ts, bands);
            if (Item make = fe::findItem(guest, menu, kMake))
                setItemTexture(guest, make, 0);
            if (Item model = fe::findItem(guest, menu, kModel))
                setItemTexture(guest, model, 0);
        }
        else
        {
            if (Item make = fe::findItem(guest, menu, kMake))
            {
                setItemShape(guest, ts, make, art, kMake.address, 0);
                make.setHidden(false);
            }
            if (Item model = fe::findItem(guest, menu, kModel))
            {
                setItemShape(guest, ts, model, art, kModelShape.address, 0);
                model.setHidden(true);
            }
            freeBlock(guest, ts, art);
        }
        if (Item slides = fe::findItem(guest, menu, kSlidesName))
            setItemTexture(guest, slides, shapeTexture(guest, ts, guest.dword(bitmap + kBitmapShape), 0));
        fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, ts);
        fe::direct<0x4eff00, frontend::eab::windowBitmapFree>(app, ts, bitmap);
    }
    guest.setDword(kBlinkSince, guest.dword(kClock));
    return 0;
}

/* sub_453ef0, slides on every pass: make and model take turns, every 200 ticks. */
nfs::u32 frontend::showcase::slidesFrame(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kBlinkTicks = 200;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
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
nfs::u32 frontend::showcase::historyPage(App* app, ThreadState& ts, u32 backOut, u32 maskOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x110);
    const nfs::u32 path = guest.frame();
    const nfs::u32 folder = carFolder(guest, ts, guest.dword(kShownCar));
    // By address: the edx it leaves this leaves too, when it goes no further.
    const nfs::u32 text = guest.call(kCarText, args({ guest.dword(kShownCar), guest.dword(kEra) + kFirstHistoryText }));
    if (text == 0 || folder == 0)
        return 0;
    if (fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.ts(), text, kAspid.address, 5) == 0)
        return 0;
    const std::uint32_t length = guest.length(text);
    if (length < 4)
        return 0;
    const nfs::u32 year = text + length - 2;

    const bool own = formatCarPath(guest, ts, path, guest.dword(kShownCar), folder, kHistoryOwn, kHistoryCommon);
    // The folder went through edx on its way to sprintf, and this function does not restore edx.
    if (!fileThere(guest, ts, path))
        return 0;

    const nfs::u32 art = fe::direct<0x4956f0, frontend::draw::loadQfsFlags>(app, ts, path, 0u);
    const auto isLstm = [&] { return fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), folder, kLstm.address) == 0; };
    const auto locate = [&](nfs::u32 name) { return guest.call(kLocateShape, args({ art, name })); };
    nfs::u32 maskName;
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
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kBackOfYear.address, guest::Words{year});
        guest.setDword(backOut, locate(path));
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kMaskOfYear.address, guest::Words{year});
        maskName = path;
    }
    guest.setDword(maskOut, locate(maskName));
    if (guest.dword(backOut) == 0 || guest.dword(maskOut) == 0)
    {
        guest.setDword(backOut, locate(kBack.address));
        guest.setDword(maskOut, locate(kMask.address));
    }
    if (guest.dword(backOut) == 0 || guest.dword(maskOut) == 0)
    {
        freeBlock(guest, ts, art);
        return 0;
    }
    return art;
}

/* sub_454210: the page of this era or of the first after it that has one,
 * round the six; 0 when none has. */
nfs::u32 frontend::showcase::historyPageFrom(App* app, ThreadState& ts, u32 backOut, u32 maskOut)
{
    constexpr std::int32_t kEras = 6;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const nfs::u32 first = guest.dword(kEra);
    for (;;)
    {
        const nfs::u32 art = guest::direct<0x454000, historyPage>(app, ts, backOut, maskOut);
        if (art != 0)
            return art;
        const nfs::u32 next = guest.dword(kEra) + 1;
        guest.setDword(kEra, next);
        if (std::int32_t(next) > kEras)
            guest.setDword(kEra, 1);
        if (first == guest.dword(kEra))
        {
            return 0;
        }
    }
}

namespace
{

/* A page's shapes on the screen's two items, `keepTexture` as loadBackdrop's. */
void showHistoryPage(Guest& guest, ThreadState& ts, nfs::u32 menu, nfs::u32 art, nfs::u32 back, nfs::u32 mask,
                     bool keepTexture)
{
    if (Item item = fe::findItem(guest, menu, kHistoryMask))
    {
        const nfs::u32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, shapeTexture(guest, ts, guest.dword(mask), replaced));
    }
    if (Item item = fe::findItem(guest, menu, kHistoryBack))
    {
        const nfs::u32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, shapeTexture(guest, ts, guest.dword(back), replaced));
    }
    freeBlock(guest, ts, art);
}

}  // namespace

/* sub_454260, history as it opens: from the first era, its description spoken. */
nfs::u32 frontend::showcase::historyEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 8);
    const nfs::u32 back = guest.frame();
    const nfs::u32 mask = guest.frame() + 4;
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kEra, 1);
    if (const nfs::u32 art = guest::direct<0x454210, historyPageFrom>(app, ts, back, mask))
        showHistoryPage(guest, ts, menu, art, back, mask, false);
    guest.setDword(kPageSince, guest.dword(kClock));
    fe::direct<0x4c95f0, port::descriptionStarted>(app, ts, ts.sp - 4, guest.dword(kShownCar), kHistoryVolume,
                                                   gameSettings(guest).language);
    return 0;
}

/* sub_454310, history on every pass: opened anew for another car; the next
 * era's page every 600 ticks. */
nfs::u32 frontend::showcase::historyFrame(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kPageTicks = 600;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const nfs::u32 back = guest.frame();
    const nfs::u32 mask = guest.frame() + 4;
    if (guest.dword(kShownCar) != guest.dword(kCar))
        return kReopen;
    if (std::int32_t(guest.dword(kClock)) > std::int32_t(guest.dword(kPageSince) + kPageTicks))
    {
        guest.setDword(kEra, guest.dword(kEra) + 1);
        if (const nfs::u32 art = guest::direct<0x454210, historyPageFrom>(app, ts, back, mask))
            showHistoryPage(guest, ts, menu, art, back, mask, true);
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
nfs::u32 frontend::showcase::comparePicture(App* app, ThreadState& ts, u32 car, u32 replaced)
{
    constexpr std::int32_t kWide = 0x120, kHigh = 0xe0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x6c);
    const nfs::u32 path = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kTwoStrings.address,
                                                guest::Words{kArtFolder, kCompareFile.address});
    const nfs::u32 art =
        nfs::u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0u));  // ebx handed on: it reads none
    const nfs::u32 stem = guest::direct<0x439290, carFileStem>(app, ts, car);
    for (nfs::u32 at = stem; guest.byte(at) != 0; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (guest.byte(kCharTraits + std::uint8_t(c + 1)) & kCharUpper)
            guest.setByte(at, std::uint8_t(fe::direct<kTolower, watcom::clib::tolower>(guest.app(), guest.ts(), c)));
    }
    // By address: the ecx it leaves reaches shapeTexture's, handed on.
    const nfs::u32 bitmap = guest.call(kNewBitmap, args({ kWide, kHigh, 0x20, 0x10 }));
    fe::direct<0x4eada0, frontend::eaa::bitmapDrawInto>(app, ts, bitmap);
    fe::direct<0x4ef1b0, frontend::eab::drawClearClip>(app, ts, clearColour(guest));
    if (stem != 0)
    {
        if (const nfs::u32 shape = guest.call(kLocateShape, args({ art, stem })))
        {
            const std::int32_t left = kWide - std::int16_t(guest.word(shape + 4));
            const std::int32_t top = kHigh - std::int16_t(guest.word(shape + 6));
            fe::direct<0x4ef4c0, frontend::eab::shapeDrawKeyedAtHotspot>(app, ts, shape, nfs::u32(left / 2),
                                                                         nfs::u32(top));
            fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, ts);
        }
    }
    const nfs::u32 texture = shapeTexture(guest, ts, guest.dword(bitmap + kBitmapShape), replaced);
    fe::direct<0x4eff00, frontend::eab::windowBitmapFree>(app, ts, bitmap);
    return texture;
}

namespace
{

/* The two cars to compare, unless the screen comes back from the other page:
 * the one the screen before was about, and the opponent's on the right if it
 * is a car of the game's, or the same again.  sub_453500's -1 is taken for a
 * car; only a car 0 keeps the old one, as the game has it. */
void chooseCompareCars(Guest& guest, ThreadState& ts)
{
    if (guest.dword(kLastAction) == kReopen)
        return;
    if (guest::direct<0x453500, frontend::showcase::resolveCar>(guest.app(), ts) != 0)
        guest.setDword(kCompareLeftCar, guest::direct<0x453500, frontend::showcase::resolveCar>(guest.app(), ts));
    const std::int32_t opponent = std::int32_t(gameSettings(guest).opponentClass);
    guest.setDword(kCompareRightCar, opponent > 3 ? nfs::u32(opponent) - kOpponentsFirstCar : guest.dword(kCompareLeftCar));
}

}  // namespace

/* sub_454500, compare as it opens: the cars and their pictures. */
nfs::u32 frontend::showcase::compareEnter(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    chooseCompareCars(guest, ts);
    guest.setDword(kCompareLeftShown, guest.dword(kCompareLeftCar));
    guest.setDword(kCompareRightShown, guest.dword(kCompareRightCar));
    guest.setDword(kCompareLeftPicture,
                   guest::direct<0x4543f0, comparePicture>(app, ts, guest.dword(kCompareLeftCar), 0));
    guest.setDword(kCompareRightPicture,
                   guest::direct<0x4543f0, comparePicture>(app, ts, guest.dword(kCompareRightCar), 0));
    return 0;
}

/* sub_454690, compare on every pass: on to compare2 when asked, and a picture
 * again for a car changed. */
nfs::u32 frontend::showcase::compareFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    nfs::u32 result = 0;
    if (guest.dword(kCompareFigures) != 0)
    {
        guest.setDword(kAfterScreen, kCompare2Name.address);
        result = kReopen;
    }
    const auto follow = [&](nfs::u32 car, nfs::u32 shown, nfs::u32 picture) {
        const nfs::u32 now = guest.dword(car);
        if (now != guest.dword(shown))
        {
            guest.setDword(shown, now);
            guest.setDword(picture, guest::direct<0x4543f0, comparePicture>(app, ts, now, guest.dword(picture)));
        }
    };
    follow(kCompareLeftCar, kCompareLeftShown, kCompareLeftPicture);
    follow(kCompareRightCar, kCompareRightShown, kCompareRightPicture);
    return result;
}

/* sub_454580, compare2 as it opens: the cars, the bar and the four arrows. */
nfs::u32 frontend::showcase::compare2Enter(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x50);
    const nfs::u32 path = guest.frame();
    chooseCompareCars(guest, ts);
    guest.setDword(kCompare5d4, 0);
    guest.setDword(kCompare574, 0);
    fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kCompareArt.address, guest::Words{kArtFolder});
    const nfs::u32 art =
        nfs::u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0u));  // ebx handed on: it reads none
    const GuestString& bar = (guest.byte(kDisplayFlags) & kNoTranslucency) ? kBarOpaque : kBarBlended;
    const nfs::u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, art, bar.address);
    guest.setDword(kCompareBar, shapeTexture(guest, ts, shape, 0));
    for (std::size_t i = 0; i < std::size(kCompareArrows); ++i)
        guest.setWord(kCompareArrows[i], std::uint16_t(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(
                                             app, ts, ts.sp - 4, kArrowNames[i].address)));
    return 0;
}

/* sub_454660, compare2 on every pass: back to compare when asked. */
nfs::u32 frontend::showcase::compare2Frame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
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
nfs::u32 frontend::showcase::zoomMoved(App* app, ThreadState& ts, u32 slider)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const float value = float(fe::direct<0x46c9b0, frontend::glue::sliderValue>(app, ts, slider, 0));
    nfs::u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    // 0x4646e0 (port_view's storedIncremented) leaves eax as it came, which since sliderValue is called as C++
    // is no value of the game's: 0.
    port::view::storedIncremented(app, value);
    (void)bits;
    return 0;
}

/* sub_454d10, interior as it opens: the slider's handler, the slider at 0. */
nfs::u32 frontend::showcase::interiorEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const nfs::u32 slider = fe::findItem(guest, menu, kZoomSlider).address();
    fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, ts, slider, kZoomMoved);
    fe::direct<0x46c950, port::fields::sliderSetValueOf>(app, ts, ts.sp - 4 - 4 * 2, slider, 0, 0);
    return 0;
}

/* sub_453200, sub_453290: whether the car has photographs, a history. */
nfs::u32 frontend::showcase::hasSlides(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x100);
    return hasArt(guest, ts, car, kSlidesOwn, kSlidesCommon);
}

nfs::u32 frontend::showcase::hasHistory(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x100);
    return hasArt(guest, ts, car, kHistoryOwn, kHistoryCommon);
}

/* sub_453830, sub_4543e0, showcase and history as they close. */
nfs::u32 frontend::showcase::showcaseExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return stopDescription(guest);
}

nfs::u32 frontend::showcase::historyExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return stopDescription(guest);
}

/* sub_453840, the button: the next photograph; slides, opened again, goes
 * back to the first if there is no such one. */
nfs::u32 frontend::showcase::nextSlide(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
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
nfs::u32 nothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, fe::Site& site)
{
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

}

nfs::u32 frontend::showcaseCarFileStem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_439290", 0, 0, nullptr, 0, 0 };
    const nfs::u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::carFileStem(app, ts, car); });
}

nfs::u32 frontend::showcaseHasInterior(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453160", 0, 0, kHasInteriorStrings, std::size(kHasInteriorStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    const nfs::u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::hasInterior(app, ts, car); });
}

nfs::u32 frontend::showcaseHasSlides(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453200", 0, 0, kHasSlidesStrings, std::size(kHasSlidesStrings), kRestoreBcd };
    const nfs::u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::hasSlides(app, ts, car); });
}

nfs::u32 frontend::showcaseHasHistory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453290", 0, 0, kHasHistoryStrings, std::size(kHasHistoryStrings), kRestoreBcd };
    const nfs::u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::hasHistory(app, ts, car); });
}

nfs::u32 frontend::showcaseLoadArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453320", 0, 0, kLoadArtStrings, std::size(kLoadArtStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::loadArt(app, ts, menu); });
}

nfs::u32 frontend::showcaseResolveCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453500", 0, 0, kResolveStrings, std::size(kResolveStrings), kRestoreCd };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::resolveCar(app, ts); });
}

nfs::u32 frontend::showcaseOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4535e0", 0, 0, kShowcaseStrings, std::size(kShowcaseStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::showcaseEnter(app, ts, menu); });
}

nfs::u32 frontend::showcaseOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4537d0", 0, 0, nullptr, 0 };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::showcaseFrame(app, ts, menu); });
}

nfs::u32 frontend::showcaseOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453830", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::showcaseExit(app, ts); });
}

nfs::u32 frontend::showcaseNextSlide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453840", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::nextSlide(app, ts); });
}

nfs::u32 frontend::showcasePreviousSlide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453860", 0, 0, kPreviousStrings, std::size(kPreviousStrings),
                          kRestoreBcd | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::previousSlide(app, ts); });
}

nfs::u32 frontend::showcaseSlidesOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453c00", 0, 0, kSlidesStrings, std::size(kSlidesStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::slidesEnter(app, ts, menu); });
}

nfs::u32 frontend::showcaseSlidesOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453ef0", 0, 0, kBlinkStrings, std::size(kBlinkStrings), kRestoreBcd };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::slidesFrame(app, ts, menu); });
}

nfs::u32 frontend::showcaseSlidesOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_453f50", 0, 4, nullptr, 0, 0 };
    return nothing(app, ts, args, site);
}

nfs::u32 frontend::showcaseHistoryPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454000", 0, 0, kPageStrings, std::size(kPageStrings), kRestoreBcSiDi };
    const nfs::u32 back = args.eax, mask = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::historyPage(app, ts, back, mask); });
}

nfs::u32 frontend::showcaseHistoryPageFrom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454210", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const nfs::u32 back = args.eax, mask = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::historyPageFrom(app, ts, back, mask); });
}

nfs::u32 frontend::showcaseHistoryOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454260", 0, 0, kHistoryStrings, std::size(kHistoryStrings), kRestoreBcd | fe::kRestoreEsi };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::historyEnter(app, ts, menu); });
}

nfs::u32 frontend::showcaseHistoryOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454310", 0, 0, kHistoryStrings, std::size(kHistoryStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::historyFrame(app, ts, menu); });
}

nfs::u32 frontend::showcaseHistoryOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4543e0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::historyExit(app, ts); });
}

nfs::u32 frontend::showcaseComparePicture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4543f0", 0, 0, kPictureStrings, std::size(kPictureStrings), kRestoreBcSiDi };
    const nfs::u32 car = args.eax, replaced = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::comparePicture(app, ts, car, replaced); });
}

nfs::u32 frontend::showcaseCompareOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454500", 0, 0, nullptr, 0, kRestoreCd };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::compareEnter(app, ts); });
}

nfs::u32 frontend::showcaseCompareOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454690", 0, 0, kCompareFrameStrings, std::size(kCompareFrameStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::compareFrame(app, ts); });
}

nfs::u32 frontend::showcaseCompare2OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454580", 0, 0, kCompare2Strings, std::size(kCompare2Strings), kRestoreBcd };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::compare2Enter(app, ts); });
}

nfs::u32 frontend::showcaseCompare2OnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454660", 0, 0, kCompare2FrameStrings, std::size(kCompare2FrameStrings), kRestoreCd };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::compare2Frame(app, ts); });
}

nfs::u32 frontend::showcaseCompare2OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454710", 0, 4, nullptr, 0, 0 };
    return nothing(app, ts, args, site);
}

nfs::u32 frontend::showcaseZoomMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454cf0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const nfs::u32 slider = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::zoomMoved(app, ts, slider); });
}

nfs::u32 frontend::showcaseInteriorOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454d10", 0, 0, kInteriorStrings, std::size(kInteriorStrings), kRestoreCd };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return showcase::interiorEnter(app, ts, menu); });
}

nfs::u32 frontend::showcaseInteriorOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454d40", 0, 4, nullptr, 0, 0 };
    return nothing(app, ts, args, site);
}

nfs::u32 frontend::showcaseInteriorOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454d50", 0, 4, nullptr, 0, 0 };
    return nothing(app, ts, args, site);
}

}
