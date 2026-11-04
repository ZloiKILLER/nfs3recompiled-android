#include "native_frontend.h"
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
 * without fading out and goes on (kAfterScreen, if set, says where to). */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using fe::Item;

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
constexpr x86::reg32 kCar1 = 0x6fd2bc;             // player one's car
constexpr x86::reg32 kCar2 = 0x6fd328;             // player two's
constexpr x86::reg32 kOpponent = 0x6fd4d4;         // past the first four, a car of the game
constexpr x86::reg32 kOpponentsFirstCar = 4;
constexpr x86::reg32 kNoSound = 0x6fbb54;
constexpr x86::reg32 kLanguage = 0x6fbb4c;
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

constexpr x86::reg32 kCarRecord = 0x439a50;
constexpr x86::reg32 kCarFolder = 0x4392b0;
constexpr x86::reg32 kCarInOwnFolder = 0x4395c0;
constexpr x86::reg32 kCarFileStem = 0x439290;
constexpr x86::reg32 kCarText = 0x439ab0;          // eax the car, edx the line
constexpr x86::reg32 kMenuMusic = 0x410460;
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kFileExists = 0x4e0e60;
constexpr x86::reg32 kStrcmp = 0x4ee310;
constexpr x86::reg32 kStrncmp = 0x4eb390;
constexpr x86::reg32 kTolower = 0x4f0ae0;
constexpr x86::reg32 kFree = 0x4e1890;
constexpr x86::reg32 kLoadQfs = 0x4956f0;          // eax a path, edx what it asks of the memory
constexpr x86::reg32 kLoadShapes = 0x44ac90;       // eax a path: the language's version
constexpr x86::reg32 kFindShape = 0x4ef1e0;        // eax the file, edx a name
constexpr x86::reg32 kLocateShape = 0x4ef270;      // likewise, 0 when it has none by the name
constexpr x86::reg32 kShapeTexture = 0x4d6390;     // eax a shape, edx the texture it replaces
constexpr x86::reg32 kShapeByName = 0x4d5db0;
// A bitmap to paint a screen's picture into: made, drawn to, its shape at
// +0x20 made a texture, freed.
constexpr x86::reg32 kNewBitmap = 0x4ef190;        // eax wide, edx high, ebx bits, ecx
constexpr x86::reg32 kBitmapShape = 0x20;
constexpr x86::reg32 kDrawTo = 0x4eada0;           // eax the bitmap
constexpr x86::reg32 kDrawToScreen = 0x4ead90;
constexpr x86::reg32 kClear = 0x4ef1b0;            // eax the colour
constexpr x86::reg32 kPaintWhereItSays = 0x4f0a10; // eax a shape, at the place it holds
constexpr x86::reg32 kPaintAt = 0x4f09c0;          // eax a shape, edx x, ebx y
constexpr x86::reg32 kPaintOffset = 0x4ef4c0;      // eax a shape, edx x, ebx y, less its own offset
constexpr x86::reg32 kFillRect = 0x4f0a90;         // eax x, edx y, ebx wide, ecx high, the colour on the stack
constexpr x86::reg32 kFreeBitmap = 0x4eff00;
// A car's description, spoken.
constexpr x86::reg32 kDescriptionStart = 0x4c95f0; // eax the car, edx the volume, ebx the language
constexpr x86::reg32 kDescriptionStop = 0x4c9490;
constexpr std::uint32_t kShowcaseVolume = 0x32;
constexpr std::uint32_t kHistoryVolume = 0x34;
// The slider's.
constexpr x86::reg32 kSliderValue = 0x46c9b0;      // on the x87
constexpr x86::reg32 kSliderOnMove = 0x46cac0;     // eax the slider, edx its handler
constexpr x86::reg32 kSliderSetValue = 0x46c950;   // eax the slider, two on the stack
constexpr x86::reg32 kSetInteriorZoom = 0x4646e0;  // a float on the stack

// This group's own, called where they are.
constexpr x86::reg32 kHasInterior = 0x453160;
constexpr x86::reg32 kHasSlides = 0x453200;
constexpr x86::reg32 kHasHistory = 0x453290;
constexpr x86::reg32 kLoadShowcaseArt = 0x453320;
constexpr x86::reg32 kResolveCar = 0x453500;
constexpr x86::reg32 kNextSlide = 0x453840;
constexpr x86::reg32 kPreviousSlide = 0x453860;
constexpr x86::reg32 kHistoryPage = 0x454000;
constexpr x86::reg32 kHistoryPageFrom = 0x454210;
constexpr x86::reg32 kComparePicture = 0x4543f0;
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

/* A shape of a loaded file as an item's picture. */
void setItemShape(Guest& guest, const Item& item, x86::reg32 file, x86::reg32 name, x86::reg32 replaced)
{
    const x86::reg32 shape = guest.call(kFindShape, args({ file, name }));
    setItemTexture(guest, item, guest.call(kShapeTexture, args({ shape, replaced })));
}

/* sub_439290: a car's file stem, in its record: 0 when it has no record. */
x86::reg32 carFileStem(Guest& guest, x86::reg32 car)
{
    const x86::reg32 record = guest.call(kCarRecord, args({ car }));
    return record != 0 ? record + kCarFileStemField : 0;
}

/* A car's art file's path: in its own folder or in the common one.  Whether
 * the car has a folder of its own comes back. */
bool formatCarPath(Guest& guest, x86::reg32 path, x86::reg32 car, x86::reg32 folder,
                   const GuestString& own, const GuestString& common)
{
    if (guest.call(kCarInOwnFolder, args({ car })) != 0)
    {
        const x86::reg32 stem = guest.call(kCarFileStem, args({ car }));
        guest.call(kSprintf, args({}, { path, own.address, kCarRootOwn, folder, stem }));
        return true;
    }
    guest.call(kSprintf, args({}, { path, common.address, kCarRootCommon, folder }));
    return false;
}

/* Whether a car has the art file: 1 or 0. */
x86::reg32 carFileExists(Guest& guest, x86::reg32 path, x86::reg32 car, x86::reg32 folder,
                         const GuestString& own, const GuestString& common)
{
    formatCarPath(guest, path, car, folder, own, common);
    return guest.call(kFileExists, args({ path })) != 0 ? 1 : 0;
}

/* The colour a cleared bitmap takes: the palette's last in 8 bits, else black. */
x86::reg32 clearColour(const Guest& guest)
{
    return guest.byte(kVideoBits) == 8 ? 0xff : 0;
}

/* ---------------------------------------------------------------------------
 * showcase
 * ------------------------------------------------------------------------- */

/* sub_453160: whether the car has an interior to show: none without sound. */
x86::reg32 hasInterior(Guest& guest, x86::reg32 car)
{
    const x86::reg32 folder = guest.call(kCarFolder, args({ car }));
    if (guest.dword(kNoSound) != 0 || folder == 0)
        return 0;
    return carFileExists(guest, guest.frame(), car, folder, kInteriorOwn, kInteriorCommon);
}

/* sub_453200, sub_453290: whether the car has photographs, a history. */
x86::reg32 hasArt(Guest& guest, x86::reg32 car, const GuestString& own, const GuestString& common)
{
    const x86::reg32 folder = guest.call(kCarFolder, args({ car }));
    if (folder == 0)
        return 0;
    return carFileExists(guest, guest.frame(), car, folder, own, common);
}

/* The backdrop's mask and picture, by the number kBackdrop gives them, set on
 * the screen's items, from the file SH<car>.  `keepTexture`: each item's
 * picture is replaced where it is; otherwise a new one is taken. */
void loadBackdrop(Guest& guest, x86::reg32 menu, x86::reg32 folder, bool keepTexture)
{
    const x86::reg32 path = guest.frame();
    formatCarPath(guest, path, guest.dword(kCar), folder, kShowcaseOwn, kShowcaseCommon);
    const x86::reg32 art = guest.call(kLoadQfs, args({ path, 0x10 }));
    if (Item mask = fe::findItem(guest, menu, kShowcaseMask))
    {
        guest.call(kSprintf, args({}, { path, kMaskNumbered.address, guest.dword(kBackdrop) + 1 }));
        setItemShape(guest, mask, art, path, keepTexture ? itemTexture(guest, mask) : 0);
    }
    if (Item back = fe::findItem(guest, menu, kShowcaseBack))
    {
        guest.call(kSprintf, args({}, { path, kBackNumbered.address, guest.dword(kBackdrop) + 1 }));
        setItemShape(guest, back, art, path, keepTexture ? itemTexture(guest, back) : 0);
    }
    guest.call(kFree, args({ art }));
}

/* sub_453320: the backdrop again, its number changed. */
x86::reg32 loadShowcaseArt(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 folder = guest.call(kCarFolder, args({ guest.dword(kCar) }));
    if (folder != 0)
        loadBackdrop(guest, menu, folder, true);
    return guest.eax();
}

/* sub_453500: the car the screen before this one was about; -1 when it is a
 * screen of the showcase's own, which keeps the one it has; the last answer
 * after any other. */
x86::reg32 resolveCar(Guest& guest)
{
    const auto cameFrom = [&](const GuestString& name) {
        return guest.call(kStrcmp, args({ guest.dword(kCameFrom), name.address })) == 0;
    };
    x86::reg32 car;
    if (cameFrom(kCar1Name))
    {
        car = guest.dword(kCar1);
    }
    else if (cameFrom(kCar2Name))
    {
        car = guest.dword(kCar2);
    }
    else if (cameFrom(kOpponentName))
    {
        const std::int32_t opponent = std::int32_t(guest.dword(kOpponent));
        car = opponent > 3 ? x86::reg32(opponent) - kOpponentsFirstCar : guest.dword(kCar1);
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
x86::reg32 onShowcaseEnter(Guest& guest, x86::reg32 menu)
{
    guest.call(kMenuMusic);
    if (guest.call(kResolveCar) != 0xffffffff)
        guest.setDword(kCar, guest.call(kResolveCar));

    const x86::reg32 folder = guest.call(kCarFolder, args({ guest.dword(kCar) }));
    guest.setDword(kSlide, 1);
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kShownBackdrop, guest.dword(kBackdrop));

    if (Item interior = fe::findItem(guest, menu, kInteriorName))
    {
        const bool has = guest.call(kHasInterior, args({ guest.dword(kShownCar) })) != 0;
        interior.setDisabled(!(has && guest.dword(kNetworkConnected) == 0));
    }
    if (Item slides = fe::findItem(guest, menu, kSlidesName))
        slides.setDisabled(guest.call(kHasSlides, args({ guest.dword(kShownCar) })) == 0);
    if (Item history = fe::findItem(guest, menu, kHistoryName))
        history.setDisabled(guest.call(kHasHistory, args({ guest.dword(kShownCar) })) == 0);

    if (folder != 0)
        loadBackdrop(guest, menu, folder, false);
    guest.call(kDescriptionStart, args({ guest.dword(kShownCar), kShowcaseVolume, guest.dword(kLanguage) }));
    return 0;
}

/* sub_4537d0, showcase on every pass: opened anew for another car; the
 * backdrop loaded again when its number changed. */
x86::reg32 onShowcaseFrame(Guest& guest, x86::reg32 menu)
{
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
        guest.call(kLoadShowcaseArt, args({ menu }));
    }
    return 0;
}

/* sub_453830, sub_4543e0, showcase and history as they close: the description stopped. */
x86::reg32 stopDescription(Guest& guest)
{
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
void formatSlidePath(Guest& guest, x86::reg32 path, x86::reg32 folder, x86::reg32 number)
{
    if (guest.call(kCarInOwnFolder, args({ guest.dword(kShownCar) })) != 0)
    {
        const x86::reg32 stem = guest.call(kCarFileStem, args({ guest.dword(kShownCar) }));
        guest.call(kSprintf, args({}, { path, kSlideOwn.address, kCarRootOwn, folder, stem, number }));
    }
    else
    {
        guest.call(kSprintf, args({}, { path, kSlideCommon.address, kCarRootCommon, folder, number }));
    }
}

/* sub_453860, the button: the photograph before; from the first, the last that
 * exists of 19 and down. */
x86::reg32 previousSlide(Guest& guest)
{
    const x86::reg32 path = guest.frame();
    const x86::reg32 number = guest.frame() + 0x100;
    const x86::reg32 folder = guest.call(kCarFolder, args({ guest.dword(kShownCar) }));
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
        formatSlidePath(guest, path, folder, number);
    } while (guest.call(kFileExists, args({ path })) == 0);
    return kReopen;
}

/* sub_453c00, slides as it opens: its buttons; the photograph (the first, if
 * this one is not there) on its bitmap; on all but the first a line and the
 * band over it and no make or model, on the first those two from the file, the
 * model hidden, for them to take turns. */
x86::reg32 onSlidesEnter(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kLineColour = 0xff2141ad;
    const x86::reg32 path = guest.frame();
    const x86::reg32 number = guest.frame() + 0x50;
    const x86::reg32 folder = guest.call(kCarFolder, args({ guest.dword(kShownCar) }));
    if (Item next = fe::findItem(guest, menu, kNextSlideName))
        next.setOnClick(kNextSlide);
    if (Item previous = fe::findItem(guest, menu, kPrevSlideName))
        previous.setOnClick(kPreviousSlide);
    if (folder != 0)
    {
        formatSlideNumber(guest, number, guest.dword(kSlide));
        formatSlidePath(guest, path, folder, number);
        if (guest.call(kFileExists, args({ path })) == 0)
        {
            guest.setDword(kSlide, 1);
            formatSlideNumber(guest, number, 1);
            formatSlidePath(guest, path, folder, number);
        }
        const x86::reg32 art = guest.call(kLoadQfs, args({ path, 0x10 }));
        const x86::reg32 back = guest.call(kFindShape, args({ art, kBack.address }));
        const x86::reg32 bitmap = guest.call(kNewBitmap, args({ 640, 480, 0x10, 0x10 }));
        guest.call(kDrawTo, args({ bitmap }));
        guest.call(kClear, args({ clearColour(guest) }));
        guest.call(kPaintWhereItSays, args({ back }));
        if (std::int32_t(guest.dword(kSlide)) > 1)
        {
            guest.call(kFillRect, args({ 0, 0x39, 640, 3 }, { kLineColour }));
            guest.call(kFree, args({ art }));
            guest.call(kSprintf, args({}, { path, kBandsFile.address, kCarRootCommon }));
            const x86::reg32 bands = guest.call(kLoadQfs, args({ path, 0x10 }));
            const x86::reg32 band = guest.call(kFindShape, args({ bands, kBand.address }));
            guest.call(kPaintAt, args({ band, 0, 0x182 }));
            guest.call(kFree, args({ bands }));
            if (Item make = fe::findItem(guest, menu, kMake))
                setItemTexture(guest, make, 0);
            if (Item model = fe::findItem(guest, menu, kModel))
                setItemTexture(guest, model, 0);
        }
        else
        {
            if (Item make = fe::findItem(guest, menu, kMake))
            {
                setItemShape(guest, make, art, kMake.address, 0);
                make.setHidden(false);
            }
            if (Item model = fe::findItem(guest, menu, kModel))
            {
                setItemShape(guest, model, art, kModelShape.address, 0);
                model.setHidden(true);
            }
            guest.call(kFree, args({ art }));
        }
        if (Item slides = fe::findItem(guest, menu, kSlidesName))
            setItemTexture(guest, slides, guest.call(kShapeTexture, args({ guest.dword(bitmap + kBitmapShape), 0 })));
        guest.call(kDrawToScreen);
        guest.call(kFreeBitmap, args({ bitmap }));
    }
    guest.setDword(kBlinkSince, guest.dword(kClock));
    return 0;
}

/* sub_453ef0, slides on every pass: make and model take turns, every 200 ticks. */
x86::reg32 onSlidesFrame(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kBlinkTicks = 200;
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
x86::reg32 historyPage(Guest& guest, x86::reg32 backOut, x86::reg32 maskOut)
{
    const x86::reg32 path = guest.frame();
    const x86::reg32 folder = guest.call(kCarFolder, args({ guest.dword(kShownCar) }));
    const x86::reg32 text = guest.call(kCarText, args({ guest.dword(kShownCar), guest.dword(kEra) + kFirstHistoryText }));
    if (text == 0 || folder == 0)
        return 0;
    if (guest.call(kStrncmp, args({ text, kAspid.address, 5 })) == 0)
        return 0;
    const std::uint32_t length = guest.length(text);
    if (length < 4)
        return 0;
    const x86::reg32 year = text + length - 2;

    const bool own = formatCarPath(guest, path, guest.dword(kShownCar), folder, kHistoryOwn, kHistoryCommon);
    // The folder went through edx on its way to sprintf, and this function does not restore edx.
    if (!own)
        guest.leaveEdx(folder);
    if (guest.call(kFileExists, args({ path })) == 0)
        return 0;

    const x86::reg32 art = guest.call(kLoadQfs, args({ path, 0 }));
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
        guest.call(kFree, args({ art }));
        return 0;
    }
    return art;
}

/* sub_454210: the page of this era or of the first after it that has one,
 * round the six; 0 when none has. */
x86::reg32 historyPageFrom(Guest& guest, x86::reg32 backOut, x86::reg32 maskOut)
{
    constexpr std::int32_t kEras = 6;
    const x86::reg32 first = guest.dword(kEra);
    for (;;)
    {
        const x86::reg32 art = guest.call(kHistoryPage, args({ backOut, maskOut }));
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

/* A page's shapes on the screen's two items, `keepTexture` as loadBackdrop's. */
void showHistoryPage(Guest& guest, x86::reg32 menu, x86::reg32 art, x86::reg32 back, x86::reg32 mask, bool keepTexture)
{
    if (Item item = fe::findItem(guest, menu, kHistoryMask))
    {
        const x86::reg32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, guest.call(kShapeTexture, args({ guest.dword(mask), replaced })));
    }
    if (Item item = fe::findItem(guest, menu, kHistoryBack))
    {
        const x86::reg32 replaced = keepTexture ? itemTexture(guest, item) : 0;
        setItemTexture(guest, item, guest.call(kShapeTexture, args({ guest.dword(back), replaced })));
    }
    guest.call(kFree, args({ art }));
}

/* sub_454260, history as it opens: from the first era, its description spoken. */
x86::reg32 onHistoryEnter(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 back = guest.frame();
    const x86::reg32 mask = guest.frame() + 4;
    guest.setDword(kShownCar, guest.dword(kCar));
    guest.setDword(kEra, 1);
    if (const x86::reg32 art = guest.call(kHistoryPageFrom, args({ back, mask })))
        showHistoryPage(guest, menu, art, back, mask, false);
    guest.setDword(kPageSince, guest.dword(kClock));
    guest.call(kDescriptionStart, args({ guest.dword(kShownCar), kHistoryVolume, guest.dword(kLanguage) }));
    return 0;
}

/* sub_454310, history on every pass: opened anew for another car; the next
 * era's page every 600 ticks. */
x86::reg32 onHistoryFrame(Guest& guest, x86::reg32 menu)
{
    constexpr x86::reg32 kPageTicks = 600;
    const x86::reg32 back = guest.frame();
    const x86::reg32 mask = guest.frame() + 4;
    if (guest.dword(kShownCar) != guest.dword(kCar))
        return kReopen;
    if (std::int32_t(guest.dword(kClock)) > std::int32_t(guest.dword(kPageSince) + kPageTicks))
    {
        guest.setDword(kEra, guest.dword(kEra) + 1);
        if (const x86::reg32 art = guest.call(kHistoryPageFrom, args({ back, mask })))
            showHistoryPage(guest, menu, art, back, mask, true);
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
x86::reg32 comparePicture(Guest& guest, x86::reg32 car, x86::reg32 replaced)
{
    constexpr std::int32_t kWide = 0x120, kHigh = 0xe0;
    const x86::reg32 path = guest.frame();
    guest.call(kSprintf, args({}, { path, kTwoStrings.address, kArtFolder, kCompareFile.address }));
    const x86::reg32 art = guest.call(kLoadShapes, args({ path }));
    const x86::reg32 stem = guest.call(kCarFileStem, args({ car }));
    for (x86::reg32 at = stem; guest.byte(at) != 0; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (guest.byte(kCharTraits + std::uint8_t(c + 1)) & kCharUpper)
            guest.setByte(at, std::uint8_t(guest.call(kTolower, args({ c }))));
    }
    const x86::reg32 bitmap = guest.call(kNewBitmap, args({ kWide, kHigh, 0x20, 0x10 }));
    guest.call(kDrawTo, args({ bitmap }));
    guest.call(kClear, args({ clearColour(guest) }));
    if (stem != 0)
    {
        if (const x86::reg32 shape = guest.call(kLocateShape, args({ art, stem })))
        {
            const std::int32_t left = kWide - std::int16_t(guest.word(shape + 4));
            const std::int32_t top = kHigh - std::int16_t(guest.word(shape + 6));
            guest.call(kPaintOffset, args({ shape, x86::reg32(left / 2), x86::reg32(top) }));
            guest.call(kDrawToScreen);
        }
    }
    const x86::reg32 texture = guest.call(kShapeTexture, args({ guest.dword(bitmap + kBitmapShape), replaced }));
    guest.call(kFreeBitmap, args({ bitmap }));
    guest.leaveEdx(texture);
    return texture;
}

/* The two cars to compare, unless the screen comes back from the other page:
 * the one the screen before was about, and the opponent's on the right if it
 * is a car of the game's, or the same again.  sub_453500's -1 is taken for a
 * car; only a car 0 keeps the old one, as the game has it. */
void chooseCompareCars(Guest& guest)
{
    if (guest.dword(kLastAction) == kReopen)
        return;
    if (guest.call(kResolveCar) != 0)
        guest.setDword(kCompareLeftCar, guest.call(kResolveCar));
    const std::int32_t opponent = std::int32_t(guest.dword(kOpponent));
    guest.setDword(kCompareRightCar, opponent > 3 ? x86::reg32(opponent) - kOpponentsFirstCar : guest.dword(kCompareLeftCar));
}

/* sub_454500, compare as it opens: the cars and their pictures. */
x86::reg32 onCompareEnter(Guest& guest)
{
    chooseCompareCars(guest);
    guest.setDword(kCompareLeftShown, guest.dword(kCompareLeftCar));
    guest.setDword(kCompareRightShown, guest.dword(kCompareRightCar));
    guest.setDword(kCompareLeftPicture, guest.call(kComparePicture, args({ guest.dword(kCompareLeftCar), 0 })));
    guest.setDword(kCompareRightPicture, guest.call(kComparePicture, args({ guest.dword(kCompareRightCar), 0 })));
    return 0;
}

/* sub_454690, compare on every pass: on to compare2 when asked, and a picture
 * again for a car changed. */
x86::reg32 onCompareFrame(Guest& guest)
{
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
            guest.setDword(picture, guest.call(kComparePicture, args({ now, guest.dword(picture) })));
        }
    };
    follow(kCompareLeftCar, kCompareLeftShown, kCompareLeftPicture);
    follow(kCompareRightCar, kCompareRightShown, kCompareRightPicture);
    return result;
}

/* sub_454580, compare2 as it opens: the cars, the bar and the four arrows. */
x86::reg32 onCompare2Enter(Guest& guest)
{
    const x86::reg32 path = guest.frame();
    chooseCompareCars(guest);
    guest.setDword(kCompare5d4, 0);
    guest.setDword(kCompare574, 0);
    guest.call(kSprintf, args({}, { path, kCompareArt.address, kArtFolder }));
    const x86::reg32 art = guest.call(kLoadShapes, args({ path }));
    const GuestString& bar = (guest.byte(kDisplayFlags) & kNoTranslucency) ? kBarOpaque : kBarBlended;
    const x86::reg32 shape = guest.call(kFindShape, args({ art, bar.address }));
    guest.setDword(kCompareBar, guest.call(kShapeTexture, args({ shape, 0 })));
    for (std::size_t i = 0; i < std::size(kCompareArrows); ++i)
        guest.setWord(kCompareArrows[i], std::uint16_t(guest.call(kShapeByName, args({ kArrowNames[i].address }))));
    return 0;
}

/* sub_454660, compare2 on every pass: back to compare when asked. */
x86::reg32 onCompare2Frame(Guest& guest)
{
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
x86::reg32 zoomMoved(Guest& guest, x86::reg32 slider)
{
    guest.call(kSliderValue, args({ slider, 0 }));
    const float value = guest.popFloat();
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    guest.call(kSetInteriorZoom, args({}, { bits }));
    return guest.eax();
}

/* sub_454d10, interior as it opens: the slider's handler, the slider at 0. */
x86::reg32 onInteriorEnter(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 slider = fe::findItem(guest, menu, kZoomSlider).address();
    guest.call(kSliderOnMove, args({ slider, kZoomMoved }));
    guest.call(kSliderSetValue, args({ slider }, { 0, 0 }));
    return 0;
}

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
    static fe::Site site{ "sub_439290", 0, 4, nullptr, 0, 0 };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carFileStem(guest, car); });
}

void frontend::showcaseHasInterior(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453160", 0, 20 + 0x100, kHasInteriorStrings, std::size(kHasInteriorStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return hasInterior(guest, car); });
}

void frontend::showcaseHasSlides(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453200", 0, 16 + 0x100, kHasSlidesStrings, std::size(kHasSlidesStrings), kRestoreBcd };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return hasArt(guest, car, kSlidesOwn, kSlidesCommon); });
}

void frontend::showcaseHasHistory(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453290", 0, 16 + 0x100, kHasHistoryStrings, std::size(kHasHistoryStrings), kRestoreBcd };
    const x86::reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return hasArt(guest, car, kHistoryOwn, kHistoryCommon); });
}

void frontend::showcaseLoadArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453320", 0, 24 + 0x54, kLoadArtStrings, std::size(kLoadArtStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadShowcaseArt(guest, menu); });
}

void frontend::showcaseResolveCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453500", 0, 12, kResolveStrings, std::size(kResolveStrings), kRestoreCd };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return resolveCar(guest); });
}

void frontend::showcaseOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4535e0", 0, 24 + 0x50, kShowcaseStrings, std::size(kShowcaseStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onShowcaseEnter(guest, menu); });
}

void frontend::showcaseOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4537d0", 0, 24, nullptr, 0 };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onShowcaseFrame(guest, menu); });
}

void frontend::showcaseOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453830", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return stopDescription(guest); });
}

/* sub_453840, the button: the next photograph; slides, opened again, goes
 * back to the first if there is no such one. */
void frontend::showcaseNextSlide(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453840", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.setDword(kSlide, guest.dword(kSlide) + 1);
        return kReopen;
    });
}

void frontend::showcasePreviousSlide(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453860", 0, 20 + 0x10c, kPreviousStrings, std::size(kPreviousStrings),
                          kRestoreBcd | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return previousSlide(guest); });
}

void frontend::showcaseSlidesOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453c00", 0, 24 + 0x64, kSlidesStrings, std::size(kSlidesStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onSlidesEnter(guest, menu); });
}

void frontend::showcaseSlidesOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453ef0", 0, 16, kBlinkStrings, std::size(kBlinkStrings), kRestoreBcd };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onSlidesFrame(guest, menu); });
}

void frontend::showcaseSlidesOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_453f50", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

void frontend::showcaseHistoryPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454000", 0, 20 + 0x110, kPageStrings, std::size(kPageStrings), kRestoreBcSiDi };
    const x86::reg32 back = cpu.eax, mask = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return historyPage(guest, back, mask); });
}

void frontend::showcaseHistoryPageFrom(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454210", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const x86::reg32 back = cpu.eax, mask = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return historyPageFrom(guest, back, mask); });
}

void frontend::showcaseHistoryOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454260", 0, 20 + 8, kHistoryStrings, std::size(kHistoryStrings), kRestoreBcd | fe::kRestoreEsi };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onHistoryEnter(guest, menu); });
}

void frontend::showcaseHistoryOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454310", 0, 24 + 8, kHistoryStrings, std::size(kHistoryStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onHistoryFrame(guest, menu); });
}

void frontend::showcaseHistoryOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4543e0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return stopDescription(guest); });
}

void frontend::showcaseComparePicture(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4543f0", 0, 20 + 0x6c, kPictureStrings, std::size(kPictureStrings), kRestoreBcSiDi };
    const x86::reg32 car = cpu.eax, replaced = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return comparePicture(guest, car, replaced); });
}

void frontend::showcaseCompareOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454500", 0, 12, nullptr, 0, kRestoreCd };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onCompareEnter(guest); });
}

void frontend::showcaseCompareOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454690", 0, 20, kCompareFrameStrings, std::size(kCompareFrameStrings),
                          kRestoreBcd | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onCompareFrame(guest); });
}

void frontend::showcaseCompare2OnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454580", 0, 16 + 0x50, kCompare2Strings, std::size(kCompare2Strings), kRestoreBcd };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onCompare2Enter(guest); });
}

void frontend::showcaseCompare2OnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454660", 0, 12, kCompare2FrameStrings, std::size(kCompare2FrameStrings), kRestoreCd };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onCompare2Frame(guest); });
}

void frontend::showcaseCompare2OnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454710", 0, 4, nullptr, 0, 0 };
    nothing(app, cpu, site);
}

void frontend::showcaseZoomMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454cf0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const x86::reg32 slider = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return zoomMoved(guest, slider); });
}

void frontend::showcaseInteriorOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_454d10", 0, 12, kInteriorStrings, std::size(kInteriorStrings), kRestoreCd };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onInteriorEnter(guest, menu); });
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
