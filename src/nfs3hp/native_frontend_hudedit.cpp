#include "native_frontend.h"
#include <nfs3hp.h>
#include <iterator>

/* The screens where the player lays out the race's HUD, decompiled:
 *
 *   hud        MENUS/HUD.MNU: which instruments a race shows and in what form,
 *              for each player -- 0 alone, 1 and 2 in split screen -- and for
 *              each of them driving a racer or a cop;
 *   ps1prace   and ps1pcop, ps2p2r, ps2p2c, ps2p1c2r, ps2p1r2c, a race's pause
 *              screens: the same HUD dragged about, element by element, over
 *              the frozen race.  They share one pair of handlers: done keeps
 *              the places, leaving any other way puts them back, and reset
 *              takes the defaults from disc.
 *
 * Both edit the records at 0x6fbc50 (kRecords): one for each player and, in
 * it, one for a racer and one for a cop.  Each says what form each instrument
 * takes, and at +0x24 has the 23 rectangles the elements stand in.  hud works
 * on lists of its own, one setting for each player and cop, that it fills as it
 * opens and writes back as it closes; on every pass it compares them with what
 * its miniature of the screen shows, and shows or hides items where they differ.
 *
 * What the race draws from the records is the race's HUD: sub_4853c0 and the
 * drawing functions stay where they are, called by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kSplitScreen = 0x6fd3b0;      // 1 in split screen, past it over a network
constexpr x86::reg32 kRaceType = 0x6fd3b8;
constexpr std::uint32_t kHotPursuit = 3;
constexpr x86::reg32 kLanguage = 0x6fbb4c;

// The HUD's records.
constexpr x86::reg32 kRecords = 0x6fbc50;
constexpr x86::reg32 kRecordsBytes = 0x9d8;
constexpr x86::reg32 kPlayerRecord = 0x348;
constexpr x86::reg32 kCopRecord = 0x1a4;
constexpr x86::reg32 kShown = 0x00;                // the HUD is on at all
constexpr x86::reg32 kOpponentName = 0x04;
constexpr x86::reg32 kOpponentList = 0x08;
constexpr x86::reg32 kRearView = 0x0c;
constexpr x86::reg32 kMapScale = 0x10;             // a float, the scale's number from 1
constexpr x86::reg32 kSplitTime = 0x14;            // 0 none, otherwise its form from 1
constexpr x86::reg32 kSpeedometer = 0x18;          // likewise
constexpr x86::reg32 kTachometer = 0x1c;           // likewise
constexpr x86::reg32 kMapStyle = 0x20;             // likewise
constexpr x86::reg32 kRectangles = 0x24;           // 23 of 16 bytes: the elements' places
constexpr x86::reg32 kRectangleBytes = 16;
constexpr x86::reg32 kPlayerRectangles = 0x170;
constexpr x86::reg32 kLanguageRectangle = 0x194;   // one more, that depends on the language
constexpr std::int32_t kElements = 23;

constexpr x86::reg32 record(x86::reg32 player, x86::reg32 cop)
{
    return kRecords + player * kPlayerRecord + cop * kCopRecord;
}

// The game's.
constexpr x86::reg32 kCameraTarget = 0x422bd0;     // eax a player: what his camera follows
constexpr x86::reg32 kTargetFlags = 0x200;
constexpr std::uint8_t kTargetIsCop = 0x20;
constexpr x86::reg32 kSetCameraOverride = 0x423960;  // eax a player, edx the mode
constexpr std::uint32_t kCameraWhileEditing = 3;
constexpr std::uint32_t kCameraAfterEditing = 0x17;
constexpr x86::reg32 kDimBackground = 0x451530;
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;
constexpr x86::reg32 kFindItem = 0x442a40;
constexpr x86::reg32 kButtonKey = 0x461270;
constexpr x86::reg32 kScaleItem = 0x45c9d0;        // an element's item, to the screen's size
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kStrcmp = 0x4ee310;
constexpr x86::reg32 kLoadShapes = 0x44ac90;       // eax a path
constexpr x86::reg32 kFindShape = 0x4ef1e0;        // eax the file, edx a name
constexpr x86::reg32 kShapeTexture = 0x4d6390;     // eax a shape, edx the old texture
constexpr x86::reg32 kLoadFile = 0x4e0ec0;
constexpr x86::reg32 kFree = 0x4e1890;
constexpr x86::reg32 kStrtok = 0x4eff60;
constexpr x86::reg32 kSscanf = 0x4f0548;
// The race's HUD.
constexpr x86::reg32 kRebuildElement = 0x4853c0;   // eax a player, edx an element
constexpr x86::reg32 kRebuildBand = 0x4800a0;      // eax a player
constexpr x86::reg32 kHighlightElement = 0x482030;  // eax the element, -1 none; edx a player; ebx 0
constexpr x86::reg32 kSetHudPane = 0x4844c0;       // eax a player
// These functions, which call one another by address.
constexpr x86::reg32 kPlaceElement = 0x45ca90;
constexpr x86::reg32 kShowSpeedometer = 0x45dae0;
constexpr x86::reg32 kShowTachometer = 0x45deb0;
constexpr x86::reg32 kGreyMirrors = 0x45e160;
constexpr x86::reg32 kGreyOpponentNames = 0x45e210;
constexpr x86::reg32 kPlaceImages = 0x45e310;
constexpr x86::reg32 kShowScreenItems = 0x45e3d0;
constexpr x86::reg32 kSetScreenArt = 0x45f300;
constexpr x86::reg32 kLoadDefaults = 0x485680;

constexpr GuestString kTwo{ 0x53a814, "2" };
constexpr GuestString kNameAndChar{ 0x53a818, "%s%c" };
constexpr std::uint32_t kCharTwo = 0x32;
constexpr GuestString kTwoStrings{ 0x53a934, "%s%s" };
constexpr x86::reg32 kArtFolder = 0x7a2db4;

Item find(Guest& guest, x86::reg32 menu, x86::reg32 name)
{
    return Item(guest, guest.call(kFindItem, fe::args({ menu, name })));
}

void setHidden(Guest& guest, x86::reg32 menu, x86::reg32 name, bool hidden)
{
    if (Item item = find(guest, menu, name))
        item.setHidden(hidden);
}

void hide(Guest& guest, x86::reg32 menu, x86::reg32 name)
{
    setHidden(guest, menu, name, true);
}

void show(Guest& guest, x86::reg32 menu, x86::reg32 name)
{
    setHidden(guest, menu, name, false);
}

void copyBytes(Guest& guest, x86::reg32 to, x86::reg32 from, x86::reg32 count)
{
    for (x86::reg32 i = 0; i < count; ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

/* strcat, as the game inlines it. */
void append(Guest& guest, x86::reg32 to, x86::reg32 from)
{
    guest.copyString(to + guest.length(to), from);
}

/* An image's name in `buffer`, player 2's with a 2 after it: the game writes
 * this two ways, with sprintf here and with strcat in nameWithTwo. */
void imageName(Guest& guest, x86::reg32 buffer, x86::reg32 name, x86::reg32 player)
{
    if (player == 2)
        guest.call(kSprintf, fe::args({}, { buffer, kNameAndChar.address, name, kCharTwo }));
    else
        guest.copyString(buffer, name);
}

void nameWithTwo(Guest& guest, x86::reg32 buffer, x86::reg32 name, x86::reg32 player)
{
    guest.copyString(buffer, name);
    if (player == 2)
        append(guest, buffer, kTwo.address);
}

/* Whether a player's camera follows a cop: which half of his record counts. */
x86::reg32 copOf(Guest& guest, x86::reg32 player)
{
    const x86::reg32 target = guest.call(kCameraTarget, fe::args({ player }));
    return (guest.byte(target + kTargetFlags) & kTargetIsCop) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * ps1prace and the other pause screens
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kHudAlwaysDrawn = 0x559264;   // the race's HUD draws all its elements
constexpr x86::reg32 kEditDone = 0x66440c;         // done left the screen: the places stay
// The rectangles as the screen opened: the player's and, in split screen, player 2's.
constexpr x86::reg32 kSavedRectangles = 0x662990;
constexpr x86::reg32 kSavedRectangles2 = 0x662b00;
// The elements' rectangles in pixels, as the race's HUD has them, four ints each.
constexpr x86::reg32 kPlacedLeft = 0x749758;
constexpr x86::reg32 kPlacedTop = 0x74975c;
constexpr x86::reg32 kPlacedRight = 0x749760;
constexpr x86::reg32 kPlacedBottom = 0x749764;

constexpr GuestString kDone{ 0x539484, "done" };
constexpr GuestString kReset{ 0x53948c, "reset" };
constexpr x86::reg32 kDoneKeyHandler = 0x44de00;
constexpr x86::reg32 kResetKeyHandler = 0x44de20;
constexpr std::uint32_t kElementItem = 0x1f;       // the widget of an element dragged about
constexpr x86::reg32 kOnKey = 0x30;
constexpr std::int16_t kKeyEnter = 0x0d;
// An element's item.
constexpr x86::reg32 kElementNumber = 0x3c;        // player 2's from 23 on
constexpr x86::reg32 kElementRectangle = 0x40;
constexpr x86::reg32 kElementRight = 0x60;
constexpr x86::reg32 kElementBottom = 0x64;

x86::reg32 signedKey(x86::reg32 ebx)
{
    return x86::reg32(std::int32_t(std::int16_t(ebx)));
}

/* sub_44de80, as a pause screen of the HUD opens: the race dimmed, its cameras
 * set, the rectangles kept for going back, the two buttons given their keys. */
x86::reg32 onLayoutEnter(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 cop1 = copOf(guest, 0);
    const x86::reg32 cop2 = copOf(guest, 1);
    guest.call(kDimBackground, fe::args({ cop2, 1 }, { kFloat0_6 }));
    guest.setDword(kHudAlwaysDrawn, 1);
    guest.setDword(kEditDone, 0);
    guest.call(kSetCameraOverride, fe::args({ 0, kCameraWhileEditing }));
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kSetCameraOverride, fe::args({ 1, kCameraWhileEditing }));
    if (guest.dword(kSplitScreen) == 1)
    {
        copyBytes(guest, kSavedRectangles, record(1, cop1) + kRectangles, kPlayerRectangles);
        copyBytes(guest, kSavedRectangles2, record(2, cop2) + kRectangles, kPlayerRectangles);
    }
    else
    {
        copyBytes(guest, kSavedRectangles, record(0, cop1) + kRectangles, kPlayerRectangles);
    }
    if (Item done = fe::findItem(guest, menu, kDone))
        guest.setDword(done.address() + kOnKey, kDoneKeyHandler);
    if (Item reset = fe::findItem(guest, menu, kReset))
        guest.setDword(reset.address() + kOnKey, kResetKeyHandler);
    guest.call(kRebuildBand, fe::args({ 0 }));
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kRebuildBand, fe::args({ 1 }));
    return 0;
}

/* sub_44dfd0, as it closes: unless done left it, the rectangles back and the
 * HUD rebuilt from them; then the race as it was. */
x86::reg32 onLayoutExit(Guest& guest)
{
    const x86::reg32 cop1 = copOf(guest, 0);
    const x86::reg32 cop2 = copOf(guest, 1);
    if (guest.dword(kEditDone) == 0)
    {
        if (guest.dword(kSplitScreen) == 1)
        {
            copyBytes(guest, record(1, cop1) + kRectangles, kSavedRectangles, kPlayerRectangles);
            copyBytes(guest, record(2, cop2) + kRectangles, kSavedRectangles2, kPlayerRectangles);
        }
        else
        {
            copyBytes(guest, record(0, cop1) + kRectangles, kSavedRectangles, kPlayerRectangles);
        }
        for (x86::reg32 player = 0; player < (guest.dword(kSplitScreen) == 1 ? 2u : 1u); ++player)
        {
            for (std::int32_t element = 0; element < kElements; ++element)
                guest.call(kRebuildElement, fe::args({ player, x86::reg32(element) }));
            guest.call(kRebuildBand, fe::args({ player }));
        }
    }
    guest.call(kHighlightElement, fe::args({ 0xffffffff, 0, 0 }));
    guest.call(kHighlightElement, fe::args({ 0xffffffff, 1, 0 }));
    guest.setDword(kHudAlwaysDrawn, 0);
    guest.call(kSetCameraOverride, fe::args({ 0, kCameraAfterEditing }));
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kSetCameraOverride, fe::args({ 1, kCameraAfterEditing }));
    if (guest.dword(kSplitScreen) == 1)
    {
        guest.call(kSetHudPane, fe::args({ 0 }));
        guest.call(kSetHudPane, fe::args({ 1 }));
    }
    return 0;
}

/* sub_44de20, the reset button's key handler: Enter takes the defaults from
 * disc and puts every element of the screen in its new place.  The handler
 * leaves ebx its last item and edx that item's number. */
x86::reg32 resetKey(Guest& guest, x86::reg32 item, x86::reg32 menu, x86::reg32 ebx)
{
    if (std::int16_t(ebx) != kKeyEnter)
        return guest.call(kButtonKey, fe::args({ item, menu, signedKey(ebx) }));
    guest.call(kLoadDefaults);
    x86::reg32 index = (ebx & 0xffff0000) | guest.word(menu + fe::menu::kFirstItem);
    for (;; ++index)
    {
        const x86::reg32 number = signedKey(index);
        const x86::reg32 at = fe::item::kTable + fe::item::kSize * number;
        if (guest.dword(at) == 0)
        {
            guest.leaveEbx(index);
            guest.leaveEdx(number);
            return 2;
        }
        if (guest.dword(at) == kElementItem)
        {
            guest.call(kPlaceElement, fe::args({ at, at, index }));
            guest.call(kScaleItem, fe::args({ at, at, index }));
        }
    }
}

/* sub_45ca90: an element's item put where its rectangle and the race's HUD
 * have it; eax what it copied last. */
x86::reg32 placeElement(Guest& guest, x86::reg32 item)
{
    const std::int32_t number = std::int32_t(guest.dword(item + kElementNumber));
    const x86::reg32 player = number < kElements ? 0 : 1;
    const x86::reg32 element = x86::reg32(number) - (player != 0 ? x86::reg32(kElements) : 0);
    const x86::reg32 owner = (guest.dword(kSplitScreen) == 1 ? 1 : 0) + player;
    const x86::reg32 cop = copOf(guest, player);
    const x86::reg32 offset = element * kRectangleBytes;
    copyBytes(guest, item + kElementRectangle, record(owner, cop) + kRectangles + offset, kRectangleBytes);
    const x86::reg32 placed = offset + player * kPlayerRectangles;
    guest.setWord(item + fe::item::kX, guest.word(kPlacedLeft + placed));
    guest.setWord(item + fe::item::kY, guest.word(kPlacedTop + placed));
    guest.setDword(item + kElementRight, guest.dword(kPlacedRight + placed));
    const x86::reg32 bottom = guest.dword(kPlacedBottom + placed);
    guest.setDword(item + kElementBottom, bottom);
    return bottom;
}

constexpr x86::reg32 kLanguageLetters = 0x47bbb8;  // "egfsiw"
constexpr x86::reg32 kHudFolder = 0x7a29b4;
constexpr GuestString kDefaultsName{ 0x53b660, "%s%cdef.pos" };
constexpr GuestString kDefaultsSeparators{ 0x53b66c, ": -(,)" };
constexpr GuestString kScanName{ 0x53b674, "%s" };
constexpr GuestString kScanFloat{ 0x53b678, "%f" };
constexpr std::uint32_t kLoadFileFlags = 0x10;
constexpr x86::reg32 kLanguageRectangles = 0x55d454;  // 16 bytes by cop, by player, by language
constexpr x86::reg32 kRebuildAfterDefaults = 0x55d440;

/* sub_485680: the rectangles' defaults for this language, from a text file of
 * fields -- a name and four numbers for each of the 23 elements, for each
 * player and each of racer and cop.  Besides the reset button, the game's
 * start calls it (sub_485880). */
x86::reg32 loadDefaults(Guest& guest)
{
    const x86::reg32 path = guest.frame();         // char[0x100]
    const x86::reg32 name = guest.frame() + 0x100;
    const x86::reg32 letters = guest.frame() + 0x180;
    copyBytes(guest, letters, kLanguageLetters, 7);
    const std::uint32_t letter = guest.byte(letters + guest.dword(kLanguage));
    guest.call(kSprintf, fe::args({}, { path, kDefaultsName.address, kHudFolder, letter }));
    const x86::reg32 file = guest.call(kLoadFile, fe::args({ path, kLoadFileFlags }));
    if (file == 0)
        return 0;
    const x86::reg32 separators = kDefaultsSeparators.address;
    x86::reg32 token = guest.call(kStrtok, fe::args({ file, separators }));
    const auto scan = [&](x86::reg32 format, x86::reg32 to) {
        guest.call(kSscanf, fe::args({}, { token, format, to }));
        token = guest.call(kStrtok, fe::args({ 0, separators }));
    };
    for (x86::reg32 player = 0; player < 3; ++player)
    {
        for (x86::reg32 cop = 0; cop < 2; ++cop)
        {
            for (std::int32_t element = 0; element < kElements; ++element)
            {
                const x86::reg32 rectangle = record(player, cop) + kRectangles + element * kRectangleBytes;
                scan(kScanName.address, name);
                scan(kScanFloat.address, rectangle + 4);
                scan(kScanFloat.address, rectangle);
                scan(kScanFloat.address, rectangle + 0xc);
                scan(kScanFloat.address, rectangle + 8);
            }
        }
    }
    guest.call(kFree, fe::args({ file }));
    for (x86::reg32 player = 0; player < 3; ++player)
    {
        for (x86::reg32 cop = 0; cop < 2; ++cop)
        {
            const x86::reg32 from = kLanguageRectangles + guest.dword(kLanguage) * 0x60 + player * 0x20 + cop * 0x10;
            copyBytes(guest, record(player, cop) + kLanguageRectangle, from, kRectangleBytes);
        }
    }
    if (guest.dword(kRebuildAfterDefaults) == 0)
        return kCopRecord;   // what the last sum above left in eax
    for (x86::reg32 player = 0; player < 2; ++player)
    {
        for (std::int32_t element = 0; element < kElements; ++element)
            guest.call(kRebuildElement, fe::args({ player, x86::reg32(element) }));
    }
    guest.call(kRebuildBand, fe::args({ 0 }));
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kRebuildBand, fe::args({ 1 }));
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * hud
 * ------------------------------------------------------------------------- */

// hud's lists, a setting for each slot.  What a list holds is its entry from 0.
constexpr x86::reg32 kListMapStyle = 0x6787f8;     // 2 none
constexpr x86::reg32 kListTachometer = 0x678810;   // 3 none
constexpr x86::reg32 kListSplitTime = 0x678828;    // 5 none
constexpr x86::reg32 kListSpeedometer = 0x678840;  // 4 none
constexpr x86::reg32 kListMapScale = 0x678858;     // 0 to 3

constexpr x86::reg32 slot(x86::reg32 player, x86::reg32 cop)
{
    return player * 8 + cop * 4;
}

// What the miniature shows, by player: -1 for nothing yet.
constexpr x86::reg32 kShownTachometer = 0x678874;
constexpr x86::reg32 kShownRearView = 0x678880;
constexpr x86::reg32 kShownSpeedometer = 0x678884;
constexpr x86::reg32 kShownMapScale = 0x678890;
constexpr x86::reg32 kShownOpponentName = 0x67889c;
constexpr x86::reg32 kShownMapStyle = 0x6788a8;
constexpr x86::reg32 kShownChoice = 0x6788b4;
constexpr x86::reg32 kShownOpponentList = 0x6788bc;
constexpr x86::reg32 kShownOnOff = 0x6788c8;
constexpr x86::reg32 kShownSplitTime = 0x6788cc;
// The list of whose HUD is laid out: its texts' numbers, and the entry chosen.
constexpr x86::reg32 kChoices = 0x6788b8;
constexpr x86::reg32 kChoice = 0x678900;
constexpr x86::reg32 kOnOff = 0x678904;            // the on and off button's value
constexpr x86::reg32 kChoicesHotPursuit = 0x5562c2;   // racer, cop
constexpr x86::reg32 kChoicesSplitHotPursuit = 0x5562c8;  // each player's racer, cop
constexpr x86::reg32 kChoicesSplit = 0x5562d2;     // player 1, player 2
constexpr std::uint16_t kTextCop = 0x19e;
constexpr std::uint16_t kTextPlayer1Cop = 0x1a1;
constexpr std::uint16_t kTextPlayer2 = 0x1a3;
constexpr std::uint16_t kTextPlayer2Cop = 0x1a4;
// The miniature: half the 640x480 screen, and its corner on it.
constexpr x86::reg32 kMiniatureHeight = 0x6788d8;
constexpr x86::reg32 kMiniatureX = 0x6788dc;
constexpr x86::reg32 kMiniatureY = 0x6788e0;
constexpr x86::reg32 kMiniatureWidth = 0x6788e4;
constexpr x86::reg32 kMapShapePlayer2 = 0x6788e8;
constexpr x86::reg32 kMapShapePlayer1 = 0x6788ec;
// A copy of the records as the screen opened, for Cancel.
constexpr x86::reg32 kSavedRecords = 0x677e20;
// The images, 20 bytes each: a name and their place on the miniature, alone
// and in split screen.
constexpr x86::reg32 kImages = 0x559e5c;
constexpr x86::reg32 kImageBytes = 20;
constexpr x86::reg32 kImageLeft = 4;
constexpr x86::reg32 kImageTop = 8;
constexpr x86::reg32 kImageSplitLeft = 12;
constexpr x86::reg32 kImageSplitTop = 16;
constexpr x86::reg32 kImageCount = 17;
// The names of each player's and cop's eight buttons and their eight cascades.
constexpr x86::reg32 kControls = 0x559cdc;
constexpr x86::reg32 kControlCount = 16;
constexpr x86::reg32 kMirrorButton = 7;
constexpr x86::reg32 kMirrorCascade = 15;
// The map's shape names: four scales for each of two styles.
constexpr x86::reg32 kMapShapes = 0x559cbc;
// An item showing art.
constexpr x86::reg32 kItemArt = 0x3c;              // char*: the shape's name in the file
constexpr x86::reg32 kItemFile = 0x40;             // char*: the art's file
constexpr x86::reg32 kItemTexture = 0x44;          // int16

constexpr x86::reg32 control(x86::reg32 player, x86::reg32 cop, x86::reg32 index)
{
    return kControls + player * 0x80 + cop * 0x40 + index * 4;
}

constexpr x86::reg32 kFloatMinusOne = 0x53a914;

constexpr GuestString kAkph{ 0x45cb60, "akphImage" };
constexpr GuestString kDkph{ 0x45cb70, "dkphImage" };
constexpr GuestString kAmph{ 0x45cb80, "amphImage" };
constexpr GuestString kDmph{ 0x45cb90, "dmphImage" };
constexpr GuestString kArpm{ 0x45cba0, "arpmImage" };
constexpr GuestString kMrpm{ 0x45cbb0, "mrpmImage" };
constexpr GuestString kDrpm{ 0x45cbc0, "drpmImage" };
constexpr GuestString kRearImage{ 0x53a43c, "rearImage" };
constexpr GuestString kOppnImage{ 0x53a448, "oppnImage" };
constexpr GuestString kPosImage{ 0x53a454, "posImage" };
constexpr GuestString kLapsImage{ 0x53a4b4, "lapsImage" };
constexpr GuestString kTimeImage{ 0x53a4c0, "timeImage" };
constexpr GuestString kSplitImage{ 0x53a4cc, "splitImage" };
constexpr GuestString kOpplImage{ 0x53a4d8, "opplImage" };
constexpr GuestString kOppcImage{ 0x53a4e4, "oppcImage" };
constexpr GuestString kMapImage{ 0x53a4f0, "mapImage" };
constexpr GuestString kCopRadarImage{ 0x53a4fc, "copRadarImage" };
constexpr GuestString kPosImage2{ 0x53a820, "posImage2" };
constexpr GuestString kTimeImage2{ 0x53a82c, "timeImage2" };
constexpr GuestString kLapsImage2{ 0x53a838, "lapsImage2" };
constexpr GuestString kCredits{ 0x53a844, "credits" };
constexpr GuestString kSingleDefaults{ 0x53a84c, "singlePlayerDefaults" };
constexpr GuestString kSplitDefaults{ 0x53a864, "splitScreenDefaults" };
constexpr GuestString kHotSplitDefaults{ 0x53a878, "hotSplitScreenDefaults" };
constexpr GuestString kScreen{ 0x53a890, "screen" };
constexpr GuestString kPlayer1Selected{ 0x53a898, "player1selected" };
constexpr GuestString kPlayer2Selected{ 0x53a8a8, "player2selected" };
constexpr GuestString kArtBoth{ 0x53a8b8, "splb" };
constexpr GuestString kArtPlayer1{ 0x53a8c0, "spl1" };
constexpr GuestString kArtPlayer2{ 0x53a8c8, "spl2" };
constexpr GuestString kArtNeither{ 0x53a8d0, "splt" };
constexpr GuestString kArtOn{ 0x53a8d8, "scrb" };
constexpr GuestString kOppnImage2{ 0x53a8e0, "oppnImage2" };
constexpr GuestString kOpplImage2{ 0x53a8ec, "opplImage2" };
constexpr GuestString kCopRadarImage2{ 0x53a8f8, "copRadarImage2" };
constexpr GuestString kOppcImage2{ 0x53a908, "oppcImage2" };
constexpr GuestString kCancel{ 0x53a918, "CANCEL_HUD_OPTIONS" };
constexpr GuestString kArtOff{ 0x53a92c, "scrn" };
constexpr GuestString kMapImage2{ 0x53a93c, "mapImage2" };
constexpr GuestString kSplitImage2{ 0x53a948, "splitImage2" };

constexpr GuestString kSpeedometerForms[] = { kAkph, kDkph, kAmph, kDmph };
constexpr GuestString kTachometerForms[] = { kArpm, kMrpm, kDrpm };

/* sub_45dae0 and sub_45deb0, each with its own names in its own frame: an
 * instrument on the miniature, one of its forms by the setting or none, and
 * for the player alone the same images of player 2's hidden.  A setting past
 * the forms touches nothing.  Without a call to leave it, edx is the sum it
 * was. */
template <std::size_t Count>
x86::reg32 showInstrument(Guest& guest, x86::reg32 menu, x86::reg32 player, x86::reg32 cop,
                          const GuestString (&forms)[Count], x86::reg32 list)
{
    const x86::reg32 names = guest.frame();   // char[16] each
    const auto nameAt = [&](std::size_t i) { return names + x86::reg32(16 * i); };
    for (std::size_t i = 0; i < Count; ++i)
        copyBytes(guest, nameAt(i), forms[i].address, 16);
    const std::uint32_t setting = guest.dword(list + slot(player, cop));
    if (player == 2)
    {
        for (std::size_t i = 0; i < Count; ++i)
            append(guest, nameAt(i), kTwo.address);
    }
    bool called = false;
    x86::reg32 result = player * kPlayerRecord;
    if (guest.dword(record(player, cop)) != 0)
    {
        result = setting;
        if (setting <= Count)
        {
            // The forms are named last to first; setting == Count shows none.
            const std::size_t shown = Count - 1 - setting;
            for (std::size_t i = 0; i < Count; ++i)
                setHidden(guest, menu, nameAt(i), i != shown);
            called = true;
            result = guest.eax();
        }
    }
    if (player == 0)
    {
        for (std::size_t i = 0; i < Count; ++i)
            append(guest, nameAt(i), kTwo.address);
        for (std::size_t i = 0; i < Count; ++i)
            hide(guest, menu, nameAt(i));
        called = true;
        result = guest.eax();
    }
    if (!called)
        guest.leaveEdx(cop * kCopRecord);
    return result;
}

/* sub_45e160: the rear view's buttons and lists greyed out in split screen. */
constexpr GuestString kMirrorItems[] = {
    { 0x539f90, "mirrorButtonP1" },     { 0x53a028, "mirrorCascadeP1" },
    { 0x53a228, "mirrorButtonP2" },     { 0x53a2c0, "mirrorCascadeP2" },
    { 0x53a0d8, "mirrorButtonCOP_P1" }, { 0x53a190, "mirrorCascadeCOP_P1" },
    { 0x53a370, "mirrorButtonCOP_P2" }, { 0x53a428, "mirrorCascadeCOP_P2" },
};

/* sub_45e210: a cop's opponents' names and split times, greyed out: a cop has
 * neither. */
constexpr GuestString kCopItems[] = {
    { 0x539e28, "opponentNameButtonCOP" },    { 0x539ec8, "opponentNameCascadeCOP" },
    { 0x53a0a0, "opponentNameButtonCOP_P1" }, { 0x53a158, "opponentNameCascadeCOP_P1" },
    { 0x53a338, "opponentNameButtonCOP_P2" }, { 0x53a3f0, "opponentNameCascadeCOP_P2" },
    { 0x539e14, "splittimeButtonCOP" },       { 0x53a088, "splittimeButtonCOP_P1" },
    { 0x53a320, "splittimeButtonCOP_P2" },    { 0x539eb4, "splittimeCascadeCOP" },
    { 0x53a140, "splittimeCascadeCOP_P1" },   { 0x53a3d8, "splittimeCascadeCOP_P2" },
};

template <std::size_t Count>
x86::reg32 greyOut(Guest& guest, x86::reg32 menu, const GuestString (&names)[Count])
{
    for (const GuestString& name : names)
    {
        if (Item item = fe::findItem(guest, menu, name))
            item.setDisabled(true);
    }
    return guest.eax();
}

/* sub_45e310: the images in their places on the miniature. */
x86::reg32 placeImages(Guest& guest, x86::reg32 menu)
{
    const bool split = guest.dword(kSplitScreen) == 1;
    const x86::reg32 left = split ? kImageSplitLeft : kImageLeft;
    const x86::reg32 top = split ? kImageSplitTop : kImageTop;
    for (x86::reg32 i = 0; i < kImageCount; ++i)
    {
        const x86::reg32 image = kImages + kImageBytes * i;
        if (Item item = find(guest, menu, guest.dword(image)))
        {
            guest.setWord(item.address() + fe::item::kX,
                          std::uint16_t(guest.word(kMiniatureX) + guest.word(image + left)));
            guest.setWord(item.address() + fe::item::kY,
                          std::uint16_t(guest.word(kMiniatureY) + guest.word(image + top)));
        }
    }
    return guest.eax();
}

/* The miniature's corner added to an item's place. */
void moveToMiniature(Guest& guest, Item item)
{
    const x86::reg32 at = item.address();
    guest.setWord(at + fe::item::kX, std::uint16_t(guest.word(at + fe::item::kX) + guest.word(kMiniatureX)));
    guest.setWord(at + fe::item::kY, std::uint16_t(guest.word(at + fe::item::kY) + guest.word(kMiniatureY)));
}

/* sub_45e3d0: only this player's and cop's controls shown, and on the miniature
 * what they say.  With the HUD off, the controls are greyed and no image shows
 * but the rear view.  ebx and eax are what its sums leave. */
x86::reg32 showScreenItems(Guest& guest, x86::reg32 menu, x86::reg32 player, x86::reg32 cop)
{
    const x86::reg32 name = guest.frame();    // char[0x100]
    for (x86::reg32 p = 0; p < 3; ++p)
    {
        for (x86::reg32 c = 0; c < 2; ++c)
        {
            for (x86::reg32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(p, c, i)));
        }
    }
    const x86::reg32 fields = record(player, cop);
    if (guest.dword(fields + kShown) == 0)
    {
        for (x86::reg32 i = 0; i < kControlCount; ++i)
        {
            if (Item item = find(guest, menu, guest.dword(control(player, cop, i))))
            {
                item.setHidden(false);
                item.setDisabled(true);
            }
        }
        for (x86::reg32 i = 0; i < kImageCount; ++i)
        {
            imageName(guest, name, guest.dword(kImages + kImageBytes * i), player);
            hide(guest, menu, name);
        }
        guest.leaveEbx(kImageCount);
        if (player != 0)
            return guest.eax();
        if (Item item = find(guest, menu, guest.dword(control(0, cop, kMirrorButton))))
            item.setDisabled(false);
        if (Item item = find(guest, menu, guest.dword(control(0, cop, kMirrorCascade))))
            item.setDisabled(false);
        if (guest.dword(record(0, cop) + kRearView) == 0)
            return cop * kCopRecord;
        show(guest, menu, guest.dword(kImages));
        return guest.eax();
    }

    for (x86::reg32 i = 0; i < kControlCount; ++i)
    {
        if (Item item = find(guest, menu, guest.dword(control(player, cop, i))))
        {
            item.setHidden(false);
            item.setDisabled(false);
        }
    }
    guest.call(kGreyMirrors, fe::args({ menu }));
    guest.call(kGreyOpponentNames, fe::args({ menu }));
    if (guest.dword(record(0, cop) + kRearView) != 0 && player == 0)
        show(guest, menu, kRearImage.address);
    guest.call(kShowSpeedometer, fe::args({ player, cop, menu }));
    guest.call(kShowTachometer, fe::args({ player, cop, menu }));
    if (guest.dword(kListMapStyle + slot(player, cop)) != 2)
    {
        imageName(guest, name, kMapImage.address, player);
        show(guest, menu, name);
    }
    if (guest.dword(fields + kOpponentName) != 0)
    {
        imageName(guest, name, kOppnImage.address, player);
        setHidden(guest, menu, name, cop != 0);
    }
    nameWithTwo(guest, name, cop != 0 ? kOppcImage.address : kOpplImage.address, player);
    x86::reg32 ebx = menu;
    if (Item item = find(guest, menu, name))
    {
        item.setHidden(guest.dword(fields + kOpponentList) == 0);
        ebx = player * kPlayerRecord;
    }
    if (guest.dword(kListSplitTime + slot(player, cop)) != 5)
    {
        imageName(guest, name, kSplitImage.address, player);
        setHidden(guest, menu, name, cop != 0);
    }
    imageName(guest, name, kCopRadarImage.address, player);
    setHidden(guest, menu, name, cop == 0);
    guest.leaveEbx(ebx);
    if (player < 2)
    {
        setHidden(guest, menu, kPosImage.address, cop != 0);
        show(guest, menu, kTimeImage.address);
        show(guest, menu, kLapsImage.address);
    }
    if (player != 2)
        return cop * kCopRecord;
    setHidden(guest, menu, kPosImage2.address, cop != 0);
    show(guest, menu, kTimeImage2.address);
    show(guest, menu, kLapsImage2.address);
    return guest.eax();
}

/* sub_45e960, hud as it opens: the records kept for Cancel, the screen's
 * buttons for who plays, the miniature, and the lists from the records. */
x86::reg32 onHudEnter(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 name = guest.frame();    // char[0x100]
    copyBytes(guest, kSavedRecords, kRecords, kRecordsBytes);
    if (Item credits = fe::findItem(guest, menu, kCredits))
    {
        if (std::int32_t(guest.dword(kSplitScreen)) > 1)
            credits.setDisabled(true);
    }
    const bool hotPursuit = guest.dword(kRaceType) == kHotPursuit;
    if (guest.dword(kSplitScreen) == 1)
    {
        hide(guest, menu, kSingleDefaults.address);
        hide(guest, menu, hotPursuit ? kSplitDefaults.address : kHotSplitDefaults.address);
        guest.setDword(kChoices, hotPursuit ? kChoicesSplitHotPursuit : kChoicesSplit);
    }
    else
    {
        hide(guest, menu, kSplitDefaults.address);
        hide(guest, menu, kHotSplitDefaults.address);
        if (hotPursuit)
            guest.setDword(kChoices, kChoicesHotPursuit);
        else
            hide(guest, menu, kSingleDefaults.address);
    }
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        guest.setDword(kShownMapScale + 4 * i, 0xffffffff);
        guest.setDword(kShownSpeedometer + 4 * i, 0xffffffff);
        guest.setDword(kShownTachometer + 4 * i, 0xffffffff);
        guest.setDword(kShownMapStyle + 4 * i, 0xffffffff);
        guest.setDword(kShownSplitTime + 4 * i, 0xffffffff);
        guest.setDword(kShownOpponentName + 4 * i, 0xffffffff);
        guest.setDword(kShownOpponentList + 4 * i, 0xffffffff);
    }
    guest.setDword(kShownChoice, 0xffffffff);
    guest.setDword(kShownOnOff, 0xffffffff);
    guest.setDword(kShownRearView, 0xffffffff);
    if (guest.dword(kSplitScreen) == 1)
        guest.call(kShowScreenItems, fe::args({ menu, 2, 0 }));
    guest.setDword(kChoice, 0);
    if (Item screen = fe::findItem(guest, menu, kScreen))
    {
        guest.setDword(kMiniatureWidth, 0x140);
        guest.setDword(kMiniatureHeight, 0xf0);
        guest.setDword(kMiniatureX, signedKey(guest.word(screen.address() + fe::item::kX)));
        guest.setDword(kMiniatureY, signedKey(guest.word(screen.address() + fe::item::kY)));
    }
    // The items stay where they were put from one opening to the next: moved
    // to the miniature only if the first of them is not there yet.
    if (Item image = fe::findItem(guest, menu, kTimeImage2))
    {
        if (std::int16_t(guest.word(image.address() + fe::item::kX)) < std::int32_t(guest.dword(kMiniatureX)))
        {
            if (Item item = fe::findItem(guest, menu, kPlayer1Selected))
                moveToMiniature(guest, item);
            if (Item item = fe::findItem(guest, menu, kPlayer2Selected))
                moveToMiniature(guest, item);
            for (x86::reg32 i = 0; i < kImageCount; ++i)
            {
                guest.call(kSprintf,
                           fe::args({}, { name, kNameAndChar.address, guest.dword(kImages + kImageBytes * i), kCharTwo }));
                if (Item item = find(guest, menu, name))
                    moveToMiniature(guest, item);
            }
        }
    }
    if (guest.dword(kSplitScreen) == 1)
    {
        if (Item screen = fe::findItem(guest, menu, kScreen))
        {
            const x86::reg32 art = screen.address() + kItemArt;
            const bool player2On = guest.dword(record(2, 0) + kShown) != 0;
            if (guest.dword(record(1, 0) + kShown) != 0)
            {
                guest.setDword(kOnOff, 1);
                guest.setDword(art, player2On ? kArtBoth.address : kArtPlayer1.address);
            }
            else
            {
                guest.setDword(kOnOff, 0);
                guest.setDword(art, player2On ? kArtPlayer2.address : kArtNeither.address);
            }
        }
        hide(guest, menu, kRearImage.address);
        for (x86::reg32 c = 0; c < 2; ++c)
        {
            for (x86::reg32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(0, c, i)));
        }
        for (x86::reg32 c = 0; c < 2; ++c)
        {
            for (x86::reg32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(2, c, i)));
        }
        for (x86::reg32 i = 0; i < kControlCount; ++i)
            hide(guest, menu, guest.dword(control(1, 1, i)));
    }
    else
    {
        guest.setDword(kOnOff, guest.dword(kRecords + kShown));
        if (Item screen = fe::findItem(guest, menu, kScreen))
            guest.setDword(screen.address() + kItemArt, kArtOn.address);
        for (x86::reg32 p = 1; p < 3; ++p)
        {
            for (x86::reg32 c = 0; c < 2; ++c)
            {
                for (x86::reg32 i = 0; i < kControlCount; ++i)
                    hide(guest, menu, guest.dword(control(p, c, i)));
            }
        }
        for (x86::reg32 i = 0; i < kControlCount; ++i)
            hide(guest, menu, guest.dword(control(0, 1, i)));
    }
    guest.call(kPlaceImages, fe::args({ menu }));

    // A list's entries are the settings from 1; its last, the setting 0, none.
    const auto toList = [](x86::reg32 setting, x86::reg32 none) { return setting != 0 ? setting - 1 : none; };
    fe::X87 x87(guest);
    for (x86::reg32 p = 0; p < 3; ++p)
    {
        for (x86::reg32 c = 0; c < 2; ++c)
        {
            const x86::reg32 fields = record(p, c);
            const x86::reg32 at = slot(p, c);
            std::int32_t scale = x87.truncated(fp::add(x87.load(fields + kMapScale), x87.load(kFloatMinusOne)));
            if (scale < 0)
                scale = 0;
            if (scale > 3)
                scale = 3;
            guest.setDword(kListMapScale + at, x86::reg32(scale));
            guest.setDword(kListSpeedometer + at, toList(guest.dword(fields + kSpeedometer), 4));
            guest.setDword(kListTachometer + at, toList(guest.dword(fields + kTachometer), 3));
            guest.setDword(kListMapStyle + at, toList(guest.dword(fields + kMapStyle), 2));
            guest.setDword(kListSplitTime + at, toList(guest.dword(fields + kSplitTime), 5));
        }
    }

    if (guest.dword(kSplitScreen) == 1)
    {
        guest.call(kShowSpeedometer, fe::args({ 1, 0, menu }));
        guest.call(kShowTachometer, fe::args({ 1, 0, menu }));
        guest.call(kShowSpeedometer, fe::args({ 2, 0, menu }));
        guest.call(kShowTachometer, fe::args({ 2, 0, menu }));
        const auto hideIfOff = [&](const GuestString& image, x86::reg32 setting) {
            if (Item item = fe::findItem(guest, menu, image))
            {
                if (guest.dword(setting) == 0)
                    item.setHidden(true);
            }
        };
        hideIfOff(kOppnImage, record(1, 0) + kOpponentName);
        hideIfOff(kOpplImage, record(1, 0) + kOpponentList);
        hideIfOff(kOppnImage2, record(2, 0) + kOpponentName);
        hideIfOff(kOpplImage2, record(2, 0) + kOpponentList);
        hide(guest, menu, kCopRadarImage2.address);
        hide(guest, menu, kOppcImage2.address);
    }
    else
    {
        guest.call(kShowSpeedometer, fe::args({ 0, 0, menu }));
        guest.call(kShowTachometer, fe::args({ 0, 0, menu }));
        if (guest.dword(kRecords + kOpponentName) == 0)
            hide(guest, menu, kOppnImage.address);
        if (guest.dword(kRecords + kOpponentList) == 0)
            hide(guest, menu, kOpplImage.address);
        if (guest.dword(kRecords + kRearView) == 0)
            hide(guest, menu, kRearImage.address);
        if (guest.dword(kRecords + kSplitTime) == 0)
            hide(guest, menu, kSplitImage.address);
        for (x86::reg32 i = 0; i < kImageCount; ++i)
        {
            guest.call(kSprintf,
                       fe::args({}, { name, kNameAndChar.address, guest.dword(kImages + kImageBytes * i), kCharTwo }));
            hide(guest, menu, name);
        }
    }
    hide(guest, menu, kCopRadarImage.address);
    hide(guest, menu, kOppcImage.address);
    guest.call(kGreyMirrors, fe::args({ menu }));
    guest.call(kGreyOpponentNames, fe::args({ menu }));
    return 0;
}

/* sub_45f160, hud as it closes: Cancel puts the records back; any other way
 * out, the lists go into the records. */
x86::reg32 onHudExit(Guest& guest, x86::reg32 menu)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    const x86::reg32 item = fe::item::kTable + fe::item::kSize * x86::reg32(first + selected);
    if (guest.dword(item) == 5
        && guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kCancel.address })) != 0)
    {
        copyBytes(guest, kRecords, kSavedRecords, kRecordsBytes);
        return 0;
    }
    const auto toSetting = [](x86::reg32 entry, x86::reg32 none) { return entry != none ? entry + 1 : 0; };
    for (x86::reg32 p = 0; p < 3; ++p)
    {
        for (x86::reg32 c = 0; c < 2; ++c)
        {
            const x86::reg32 fields = record(p, c);
            const x86::reg32 at = slot(p, c);
            const x86::Float scale = fp::add(1.0, fe::X87::integer(std::int32_t(guest.dword(kListMapScale + at))));
            guest.setFloat(fields + kMapScale, float(scale));
            guest.setDword(fields + kSpeedometer, toSetting(guest.dword(kListSpeedometer + at), 4));
            guest.setDword(fields + kTachometer, toSetting(guest.dword(kListTachometer + at), 3));
            guest.setDword(fields + kMapStyle, toSetting(guest.dword(kListMapStyle + at), 2));
            guest.setDword(fields + kSplitTime, toSetting(guest.dword(kListSplitTime + at), 5));
        }
    }
    return 0;
}

/* sub_45f300: the miniature's art, by whose HUD is on (the other player's, in
 * split screen, by the art it has now), and whose frame is lit.  Loaded anew
 * only when it is another.  ebx its name, as it leaves it. */
x86::reg32 setScreenArt(Guest& guest, x86::reg32 menu, x86::reg32 player, x86::reg32 cop)
{
    const x86::reg32 path = guest.frame();    // char[0x100]
    const Item screen = fe::findItem(guest, menu, kScreen);
    if (!screen)
    {
        guest.leaveEbx(0);
        return 0;
    }
    const x86::reg32 at = screen.address();
    const bool on = guest.dword(record(player, cop)) != 0;
    x86::reg32 art;
    if (player == 1)
    {
        const std::uint8_t last = guest.byte(guest.dword(at + kItemArt) + 3);
        const bool otherOn = last == '2' || last == 'b';
        art = on ? (otherOn ? kArtBoth.address : kArtPlayer1.address)
                 : (otherOn ? kArtPlayer2.address : kArtNeither.address);
        show(guest, menu, kPlayer1Selected.address);
        hide(guest, menu, kPlayer2Selected.address);
    }
    else if (player == 2)
    {
        const std::uint8_t last = guest.byte(guest.dword(at + kItemArt) + 3);
        const bool otherOn = last == '1' || last == 'b';
        art = on ? (otherOn ? kArtBoth.address : kArtPlayer2.address)
                 : (otherOn ? kArtPlayer1.address : kArtNeither.address);
        hide(guest, menu, kPlayer1Selected.address);
        show(guest, menu, kPlayer2Selected.address);
    }
    else
    {
        art = on ? kArtOn.address : kArtOff.address;
        hide(guest, menu, kPlayer1Selected.address);
        hide(guest, menu, kPlayer2Selected.address);
    }
    guest.leaveEbx(art);
    if (guest.call(kStrcmp, fe::args({ art, guest.dword(at + kItemArt) })) == 0)
        return 0;
    guest.call(kSprintf, fe::args({}, { path, kTwoStrings.address, kArtFolder, guest.dword(at + kItemFile) }));
    const x86::reg32 file = guest.call(kLoadShapes, fe::args({ path }));
    const x86::reg32 old = signedKey(guest.word(at + kItemTexture));
    const x86::reg32 shape = guest.call(kFindShape, fe::args({ file, art }));
    const x86::reg32 texture = guest.call(kShapeTexture, fe::args({ shape, old }));
    guest.setWord(at + kItemTexture, std::uint16_t(texture));
    guest.setDword(at + kItemArt, art);
    return texture;
}

/* The map on the miniature, in its style and scale; part of hud's every pass. */
void showMap(Guest& guest, x86::reg32 menu, x86::reg32 player, x86::reg32 cop)
{
    const x86::reg32 name = guest.frame();
    const x86::reg32 path = guest.frame() + 0x100;
    const x86::reg32 at = slot(player, cop);
    guest.setDword(kShownMapScale + 4 * player, guest.dword(kListMapScale + at));
    guest.setDword(kShownMapStyle + 4 * player, guest.dword(kListMapStyle + at));
    guest.copyString(name, player == 2 ? kMapImage2.address : kMapImage.address);
    Item map = find(guest, menu, name);
    if (!map)
        return;
    const std::int32_t style = std::int32_t(guest.dword(kListMapStyle + at));
    if (style >= 2)
    {
        map.setHidden(true);
        return;
    }
    const x86::reg32 shapeNumber = guest.dword(kListMapScale + at) + x86::reg32(style) * 4;
    guest.setDword(player == 2 ? kMapShapePlayer2 : kMapShapePlayer1, shapeNumber);
    guest.call(kSprintf, fe::args({}, { path, kTwoStrings.address, kArtFolder, guest.dword(map.address() + kItemFile) }));
    const x86::reg32 file = guest.call(kLoadShapes, fe::args({ path }));
    const x86::reg32 old = signedKey(guest.word(map.address() + kItemTexture));
    const x86::reg32 shape = guest.call(kFindShape, fe::args({ file, guest.dword(kMapShapes + 4 * shapeNumber) }));
    guest.setWord(map.address() + kItemTexture, std::uint16_t(guest.call(kShapeTexture, fe::args({ shape, old }))));
    map.setHidden(false);
}

/* sub_45f4c0, hud on every pass: whose HUD is laid out, by the list's choice,
 * and then the items for what has changed. */
x86::reg32 onHudFrame(Guest& guest, x86::reg32 menu)
{
    const x86::reg32 name = guest.frame();    // char[0x100]
    const auto chosenText = [&] { return guest.word(guest.dword(kChoices) + 2 * guest.dword(kChoice)); };
    x86::reg32 player = 0;
    if (guest.dword(kSplitScreen) == 1)
    {
        const std::uint16_t text = chosenText();
        player = text == kTextPlayer2 || text == kTextPlayer2Cop ? 2 : 1;
    }
    x86::reg32 cop = 0;
    if (guest.dword(kRaceType) == kHotPursuit)
    {
        const std::uint16_t text = chosenText();
        cop = text == kTextCop || text == kTextPlayer1Cop || text == kTextPlayer2Cop ? 1 : 0;
    }
    const x86::reg32 fields = record(player, cop);
    const x86::reg32 at = slot(player, cop);
    const x86::reg32 own = 4 * player;

    if (guest.dword(kShownChoice) != guest.dword(kChoice))
    {
        guest.setDword(kShownChoice, guest.dword(kChoice));
        guest.setDword(kOnOff, guest.dword(fields + kShown));
        guest.setDword(kShownOnOff, 0xffffffff);
        guest.call(kSetScreenArt, fe::args({ menu, player, cop }));
        // Every image looked up, by both players' names, and nothing done with
        // them: the test of its flags that follows in the game goes nowhere.
        for (x86::reg32 i = 0; i < kImageCount; ++i)
        {
            const x86::reg32 image = guest.dword(kImages + kImageBytes * i);
            guest.copyString(name, image);
            find(guest, menu, name);
            guest.call(kSprintf, fe::args({}, { name, kNameAndChar.address, image, kCharTwo }));
            find(guest, menu, name);
        }
    }

    if (guest.dword(kShownOnOff) != guest.dword(kOnOff))
    {
        const std::uint32_t onOff = guest.dword(kOnOff);
        guest.setDword(kShownOnOff, onOff);
        guest.setDword(fields + kShown, onOff);
        guest.call(kShowScreenItems, fe::args({ menu, player, cop }));
        guest.call(kSetScreenArt, fe::args({ menu, player, cop }));
    }

    if (guest.dword(kSplitScreen) != 1)
    {
        const std::uint32_t rearView = guest.dword(record(0, cop) + kRearView);
        if (guest.dword(kShownRearView) != rearView)
        {
            guest.setDword(kShownRearView, rearView);
            setHidden(guest, menu, kRearImage.address, rearView == 0);
        }
    }

    if (guest.dword(fields + kShown) == 0)
        return 0;

    if (guest.dword(kListMapScale + at) != guest.dword(kShownMapScale + own)
        || guest.dword(kListMapStyle + at) != guest.dword(kShownMapStyle + own))
    {
        showMap(guest, menu, player, cop);
    }
    if (guest.dword(kListSpeedometer + at) != guest.dword(kShownSpeedometer + own))
    {
        guest.setDword(kShownSpeedometer + own, guest.dword(kListSpeedometer + at));
        guest.call(kShowSpeedometer, fe::args({ player, cop, menu }));
    }
    if (guest.dword(kListTachometer + at) != guest.dword(kShownTachometer + own))
    {
        guest.setDword(kShownTachometer + own, guest.dword(kListTachometer + at));
        guest.call(kShowTachometer, fe::args({ player, cop, menu }));
    }
    if (guest.dword(kListSplitTime + at) != guest.dword(kShownSplitTime + own))
    {
        guest.setDword(kShownSplitTime + own, guest.dword(kListSplitTime + at));
        guest.copyString(name, player == 2 ? kSplitImage2.address : kSplitImage.address);
        setHidden(guest, menu, name, guest.dword(kShownSplitTime + own) == 5);
    }
    if (guest.dword(fields + kOpponentName) != guest.dword(kShownOpponentName + own))
    {
        guest.setDword(kShownOpponentName + own, guest.dword(fields + kOpponentName));
        guest.copyString(name, player == 2 ? kOppnImage2.address : kOppnImage.address);
        setHidden(guest, menu, name, guest.dword(kShownOpponentName + own) == 0);
    }
    if (guest.dword(fields + kOpponentList) != guest.dword(kShownOpponentList + own))
    {
        guest.setDword(kShownOpponentList + own, guest.dword(fields + kOpponentList));
        nameWithTwo(guest, name, cop != 0 ? kOppcImage.address : kOpplImage.address, player);
        setHidden(guest, menu, name, guest.dword(kShownOpponentList + own) == 0);
    }
    // The opponents' other list, always hidden.
    nameWithTwo(guest, name, cop != 0 ? kOpplImage.address : kOppcImage.address, player);
    hide(guest, menu, name);
    return 0;
}

const GuestString kLayoutEnterStrings[] = { kDone, kReset };
const GuestString kDefaultsStrings[] = { kDefaultsName, kDefaultsSeparators, kScanName, kScanFloat };
const GuestString kSpeedometerStrings[] = { kAkph, kDkph, kAmph, kDmph, kTwo };
const GuestString kTachometerStrings[] = { kArpm, kMrpm, kDrpm, kTwo };
const GuestString kHudEnterStrings[] = {
    kCredits, kSingleDefaults, kSplitDefaults, kHotSplitDefaults, kScreen, kTimeImage2, kPlayer1Selected,
    kPlayer2Selected, kNameAndChar, kRearImage, kOppnImage, kOpplImage, kOppnImage2, kOpplImage2, kCopRadarImage2,
    kOppcImage2, kSplitImage, kCopRadarImage, kOppcImage, kArtBoth, kArtPlayer1, kArtPlayer2, kArtNeither, kArtOn,
};
const GuestString kHudExitStrings[] = { kCancel };
const GuestString kScreenItemsStrings[] = {
    kRearImage, kMapImage, kOppnImage, kOpplImage, kOppcImage, kSplitImage, kCopRadarImage, kPosImage, kTimeImage,
    kLapsImage, kPosImage2, kTimeImage2, kLapsImage2, kNameAndChar, kTwo,
};
const GuestString kScreenArtStrings[] = {
    kScreen, kPlayer1Selected, kPlayer2Selected, kArtBoth, kArtPlayer1, kArtPlayer2, kArtNeither, kArtOn, kArtOff,
    kTwoStrings,
};
const GuestString kHudFrameStrings[] = {
    kNameAndChar, kRearImage, kMapImage, kMapImage2, kTwoStrings, kSplitImage, kSplitImage2, kOppnImage, kOppnImage2,
    kOppcImage, kOpplImage, kTwo,
};

}

void frontend::hudEditDoneKey(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_44de00: done's key handler, Enter keeping the places as they are.
    static fe::Site site{ "sub_44de00", 0, 4, nullptr, 0, 0 };
    const x86::reg32 item = cpu.eax, menu = cpu.edx, ebx = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        if (std::int16_t(ebx) == kKeyEnter)
            guest.setDword(kEditDone, 1);
        return guest.call(kButtonKey, fe::args({ item, menu, signedKey(ebx) }));
    });
}

void frontend::hudEditResetKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44de20", 0, 4, nullptr, 0, 0 };
    const x86::reg32 item = cpu.eax, menu = cpu.edx, ebx = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resetKey(guest, item, menu, ebx); });
}

void frontend::hudEditLayoutOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44de80", 0, 28, kLayoutEnterStrings, std::size(kLayoutEnterStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onLayoutEnter(guest, menu); });
}

void frontend::hudEditLayoutOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44dfd0", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onLayoutExit(guest); });
}

void frontend::hudEditPlaceElement(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45ca90", 0, 24, nullptr, 0 };
    const x86::reg32 item = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return placeElement(guest, item); });
}

void frontend::hudEditShowSpeedometer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45dae0", 0, 16 + 0x48, kSpeedometerStrings, std::size(kSpeedometerStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 player = cpu.eax, cop = cpu.edx, menu = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return showInstrument(guest, menu, player, cop, kSpeedometerForms, kListSpeedometer);
    });
}

void frontend::hudEditShowTachometer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45deb0", 0, 16 + 0x38, kTachometerStrings, std::size(kTachometerStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 player = cpu.eax, cop = cpu.edx, menu = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        return showInstrument(guest, menu, player, cop, kTachometerForms, kListTachometer);
    });
}

void frontend::hudEditGreyMirrors(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45e160", 0, 12, kMirrorItems, std::size(kMirrorItems),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return greyOut(guest, menu, kMirrorItems); });
}

void frontend::hudEditGreyOpponentNames(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45e210", 0, 12, kCopItems, std::size(kCopItems), fe::kRestoreEcx | fe::kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return greyOut(guest, menu, kCopItems); });
}

void frontend::hudEditPlaceImages(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45e310", 0, 24, nullptr, 0 };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return placeImages(guest, menu); });
}

void frontend::hudEditShowScreenItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45e3d0", 0, 16 + 0x10c, kScreenItemsStrings, std::size(kScreenItemsStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 menu = cpu.eax, player = cpu.edx, cop = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return showScreenItems(guest, menu, player, cop); });
}

void frontend::hudEditOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45e960", 0, 24 + 0x108, kHudEnterStrings, std::size(kHudEnterStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onHudEnter(guest, menu); });
}

void frontend::hudEditOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45f160", 0, 28, kHudExitStrings, std::size(kHudExitStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onHudExit(guest, menu); });
}

void frontend::hudEditSetScreenArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45f300", 0, 16 + 0x104, kScreenArtStrings, std::size(kScreenArtStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 menu = cpu.eax, player = cpu.edx, cop = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setScreenArt(guest, menu, player, cop); });
}

void frontend::hudEditOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_45f4c0", 0, 24 + 0x15c, kHudFrameStrings, std::size(kHudFrameStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onHudFrame(guest, menu); });
}

void frontend::hudEditLoadDefaults(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_485680", 0, 24 + 0x18c, kDefaultsStrings, std::size(kDefaultsStrings) };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return loadDefaults(guest); });
}

}
