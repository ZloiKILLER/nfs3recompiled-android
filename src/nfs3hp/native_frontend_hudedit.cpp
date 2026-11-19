#include "native_frontend_hudedit.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_eab.h"

#include "native_frontend.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudz.h"
#include "native_frontend_menuf.h"
#include "native_frontend_menus.h"
#include "port_buttons.h"
#include "port_hud.h"
#include "port_menus.h"
#include "port_shapes.h"
#include "port_text.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <iterator>

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

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
 * drawing functions stay where they are, called by address.
 *
 * Plan step 5: each function a typed core in frontend::hudedit (native_frontend_hudedit.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the game's
 * name for the calls by address.  A core of this file calls another as C++ (guest::direct), and
 * the port's and the libraries' typed cores too (fe::direct) but where the screen goes on with the
 * registers the call leaves: sub_442a40's (find), sub_461270's (the key handlers) and sub_4d6390's
 * from sub_45f300.  The rest of the game still by address. */
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

constexpr std::uint32_t kHotPursuit = 3;

// The HUD's records.
constexpr nfs::u32 kRecordsBytes = 0x9d8;
constexpr nfs::u32 kPlayerRecord = 0x348;
constexpr nfs::u32 kCopRecord = 0x1a4;
constexpr nfs::u32 kRectangles = 0x24;           // 23 of 16 bytes: the elements' places
constexpr nfs::u32 kRectangleBytes = 16;
constexpr nfs::u32 kPlayerRectangles = 0x170;
constexpr nfs::u32 kLanguageRectangle = 0x194;   // one more, that depends on the language
constexpr std::int32_t kElements = 23;

// The game's.
constexpr nfs::u32 kTargetFlags = 0x200;
constexpr std::uint8_t kTargetIsCop = 0x20;
constexpr nfs::u32 kSetCameraOverride = 0x423960;  // eax a player, edx the mode
constexpr std::uint32_t kCameraWhileEditing = 3;
constexpr std::uint32_t kCameraAfterEditing = 0x17;
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
constexpr nfs::u32 kFindItem = 0x442a40;
constexpr nfs::u32 kButtonKey = 0x461270;        // leaves ebx and edx, which the key handlers hand on
constexpr nfs::u32 kSprintf = 0x4df690;
constexpr nfs::u32 kStrcmp = 0x4ee310;
constexpr nfs::u32 kFindShape = 0x4ef1e0;        // eax the file, edx a name
constexpr nfs::u32 kShapeTexture = 0x4d6390;     // eax a shape, edx the old texture; leaves edx
constexpr nfs::u32 kStrtok = 0x4eff60;
constexpr nfs::u32 kSscanf = 0x4f0548;
// The race's HUD.

constexpr GuestString kTwo{ 0x53a814, "2" };
constexpr GuestString kNameAndChar{ 0x53a818, "%s%c" };
constexpr std::uint32_t kCharTwo = 0x32;
constexpr GuestString kTwoStrings{ 0x53a934, "%s%s" };
constexpr nfs::u32 kArtFolder = 0x7a2db4;

/* By address: the screens go on with what it leaves -- the item in eax, the name in edx (sub_45dae0,
 * sub_45e310 and sub_45e3d0 return them) -- and its core (frontend::menus::findItem) hands back the
 * item alone. */
nfs::u32 findAt(Guest& guest, nfs::u32 menu, nfs::u32 name)
{
    return fe::direct<kFindItem, frontend::menus::findItem>(guest.app(), guest.ts(), menu, name);
}

Item find(Guest& guest, nfs::u32 menu, nfs::u32 name)
{
    return Item(guest, findAt(guest, menu, name));
}

// What each gives back is the item the code looked for last (0 for none): where a screen's function goes on with
// "what the calls left" in eax.
nfs::u32 setHidden(Guest& guest, nfs::u32 menu, nfs::u32 name, bool hidden)
{
    const nfs::u32 found = findAt(guest, menu, name);
    if (Item item = Item(guest, found))
        item.setHidden(hidden);
    return found;
}

nfs::u32 hide(Guest& guest, nfs::u32 menu, nfs::u32 name)
{
    return setHidden(guest, menu, name, true);
}

nfs::u32 show(Guest& guest, nfs::u32 menu, nfs::u32 name)
{
    return setHidden(guest, menu, name, false);
}

void copyBytes(Guest& guest, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    for (nfs::u32 i = 0; i < count; ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

/* strcat, as the game inlines it. */
void append(Guest& guest, nfs::u32 to, nfs::u32 from)
{
    guest.copyString(to + guest.length(to), from);
}

/* An image's name in `buffer`, player 2's with a 2 after it: the game writes
 * this two ways, with sprintf here and with strcat in nameWithTwo. */
void imageName(Guest& guest, nfs::u32 buffer, nfs::u32 name, nfs::u32 player)
{
    if (player == 2)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, kNameAndChar.address, guest::Words{name, kCharTwo});
    else
        guest.copyString(buffer, name);
}

void nameWithTwo(Guest& guest, nfs::u32 buffer, nfs::u32 name, nfs::u32 player)
{
    guest.copyString(buffer, name);
    if (player == 2)
        append(guest, buffer, kTwo.address);
}

/* Whether a player's camera follows a cop: which half of his record counts. */
nfs::u32 copOf(Guest& guest, nfs::ThreadState& ts, nfs::u32 player)
{
    const nfs::u32 target = fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), ts, player);
    return (guest.byte(target + kTargetFlags) & kTargetIsCop) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * ps1prace and the other pause screens
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kHudAlwaysDrawn = 0x559264;   // the race's HUD draws all its elements
constexpr nfs::u32 kEditDone = 0x66440c;         // done left the screen: the places stay
// The rectangles as the screen opened: the player's and, in split screen, player 2's.
constexpr nfs::u32 kSavedRectangles = 0x662990;
constexpr nfs::u32 kSavedRectangles2 = 0x662b00;
// The elements' rectangles in pixels, as the race's HUD has them, four ints each.
constexpr nfs::u32 kPlacedLeft = 0x749758;
constexpr nfs::u32 kPlacedTop = 0x74975c;
constexpr nfs::u32 kPlacedRight = 0x749760;
constexpr nfs::u32 kPlacedBottom = 0x749764;

constexpr GuestString kDone{ 0x539484, "done" };
constexpr GuestString kReset{ 0x53948c, "reset" };
constexpr nfs::u32 kDoneKeyHandler = 0x44de00;
constexpr nfs::u32 kResetKeyHandler = 0x44de20;
constexpr std::uint32_t kElementItem = 0x1f;       // the widget of an element dragged about
constexpr nfs::u32 kOnKey = 0x30;
constexpr std::int16_t kKeyEnter = 0x0d;
// An element's item.
constexpr nfs::u32 kElementNumber = 0x3c;        // player 2's from 23 on
constexpr nfs::u32 kElementRectangle = 0x40;
constexpr nfs::u32 kElementRight = 0x60;
constexpr nfs::u32 kElementBottom = 0x64;

nfs::u32 signedKey(nfs::u32 ebx)
{
    return nfs::u32(std::int32_t(std::int16_t(ebx)));
}

constexpr nfs::u32 kLanguageLetters = 0x47bbb8;  // "egfsiw"
constexpr nfs::u32 kHudFolder = 0x7a29b4;
constexpr GuestString kDefaultsName{ 0x53b660, "%s%cdef.pos" };
constexpr GuestString kDefaultsSeparators{ 0x53b66c, ": -(,)" };
constexpr GuestString kScanName{ 0x53b674, "%s" };
constexpr GuestString kScanFloat{ 0x53b678, "%f" };
constexpr std::uint32_t kLoadFileFlags = 0x10;
constexpr nfs::u32 kLanguageRectangles = 0x55d454;  // 16 bytes by cop, by player, by language
constexpr nfs::u32 kRebuildAfterDefaults = 0x55d440;

/* ---------------------------------------------------------------------------
 * hud
 * ------------------------------------------------------------------------- */

// hud's lists, a setting for each slot.  What a list holds is its entry from 0.
constexpr nfs::u32 kListMapStyle = 0x6787f8;     // 2 none
constexpr nfs::u32 kListTachometer = 0x678810;   // 3 none
constexpr nfs::u32 kListSplitTime = 0x678828;    // 5 none
constexpr nfs::u32 kListSpeedometer = 0x678840;  // 4 none
constexpr nfs::u32 kListMapScale = 0x678858;     // 0 to 3

constexpr nfs::u32 slot(nfs::u32 player, nfs::u32 cop)
{
    return player * 8 + cop * 4;
}

// What the miniature shows, by player: -1 for nothing yet.
constexpr nfs::u32 kShownTachometer = 0x678874;
constexpr nfs::u32 kShownRearView = 0x678880;
constexpr nfs::u32 kShownSpeedometer = 0x678884;
constexpr nfs::u32 kShownMapScale = 0x678890;
constexpr nfs::u32 kShownOpponentName = 0x67889c;
constexpr nfs::u32 kShownMapStyle = 0x6788a8;
constexpr nfs::u32 kShownChoice = 0x6788b4;
constexpr nfs::u32 kShownOpponentList = 0x6788bc;
constexpr nfs::u32 kShownOnOff = 0x6788c8;
constexpr nfs::u32 kShownSplitTime = 0x6788cc;
// The list of whose HUD is laid out: its texts' numbers, and the entry chosen.
constexpr nfs::u32 kChoices = 0x6788b8;
constexpr nfs::u32 kChoice = 0x678900;
constexpr nfs::u32 kOnOff = 0x678904;            // the on and off button's value
constexpr nfs::u32 kChoicesHotPursuit = 0x5562c2;   // racer, cop
constexpr nfs::u32 kChoicesSplitHotPursuit = 0x5562c8;  // each player's racer, cop
constexpr nfs::u32 kChoicesSplit = 0x5562d2;     // player 1, player 2
constexpr std::uint16_t kTextCop = 0x19e;
constexpr std::uint16_t kTextPlayer1Cop = 0x1a1;
constexpr std::uint16_t kTextPlayer2 = 0x1a3;
constexpr std::uint16_t kTextPlayer2Cop = 0x1a4;
// The miniature: half the 640x480 screen, and its corner on it.
constexpr nfs::u32 kMiniatureHeight = 0x6788d8;
constexpr nfs::u32 kMiniatureX = 0x6788dc;
constexpr nfs::u32 kMiniatureY = 0x6788e0;
constexpr nfs::u32 kMiniatureWidth = 0x6788e4;
constexpr nfs::u32 kMapShapePlayer2 = 0x6788e8;
constexpr nfs::u32 kMapShapePlayer1 = 0x6788ec;
// A copy of the records as the screen opened, for Cancel.
constexpr nfs::u32 kSavedRecords = 0x677e20;
// The images, 20 bytes each: a name and their place on the miniature, alone
// and in split screen.
constexpr nfs::u32 kImages = 0x559e5c;
constexpr nfs::u32 kImageBytes = 20;
constexpr nfs::u32 kImageLeft = 4;
constexpr nfs::u32 kImageTop = 8;
constexpr nfs::u32 kImageSplitLeft = 12;
constexpr nfs::u32 kImageSplitTop = 16;
constexpr nfs::u32 kImageCount = 17;
// The names of each player's and cop's eight buttons and their eight cascades.
constexpr nfs::u32 kControls = 0x559cdc;
constexpr nfs::u32 kControlCount = 16;
constexpr nfs::u32 kMirrorButton = 7;
constexpr nfs::u32 kMirrorCascade = 15;
// The map's shape names: four scales for each of two styles.
constexpr nfs::u32 kMapShapes = 0x559cbc;
// An item showing art.
constexpr nfs::u32 kItemArt = 0x3c;              // char*: the shape's name in the file
constexpr nfs::u32 kItemFile = 0x40;             // char*: the art's file
constexpr nfs::u32 kItemTexture = 0x44;          // int16

constexpr nfs::u32 control(nfs::u32 player, nfs::u32 cop, nfs::u32 index)
{
    return kControls + player * 0x80 + cop * 0x40 + index * 4;
}

constexpr nfs::u32 kFloatMinusOne = 0x53a914;

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
nfs::u32 showInstrument(Guest& guest, nfs::u32 menu, nfs::u32 player, nfs::u32 cop,
                          const GuestString (&forms)[Count], nfs::u32 list)
{
    const nfs::u32 names = guest.frame();   // char[16] each
    const auto nameAt = [&](std::size_t i) { return names + nfs::u32(16 * i); };
    for (std::size_t i = 0; i < Count; ++i)
        copyBytes(guest, nameAt(i), forms[i].address, 16);
    const std::uint32_t setting = guest.dword(list + slot(player, cop));
    if (player == 2)
    {
        for (std::size_t i = 0; i < Count; ++i)
            append(guest, nameAt(i), kTwo.address);
    }
    bool called = false;
    nfs::u32 found = 0;   // the item the last look-up found
    nfs::u32 result = player * kPlayerRecord;
    if (gameSettings(guest).hudOf(player, cop).shown != 0)
    {
        result = setting;
        if (setting <= Count)
        {
            // The forms are named last to first; setting == Count shows none.
            const std::size_t shown = Count - 1 - setting;
            for (std::size_t i = 0; i < Count; ++i)
                found = setHidden(guest, menu, nameAt(i), i != shown);
            called = true;
            result = found;
        }
    }
    if (player == 0)
    {
        for (std::size_t i = 0; i < Count; ++i)
            append(guest, nameAt(i), kTwo.address);
        for (std::size_t i = 0; i < Count; ++i)
            found = hide(guest, menu, nameAt(i));
        called = true;
        result = found;
    }
    if (!called)
        {}
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
nfs::u32 greyOut(Guest& guest, nfs::u32 menu, const GuestString (&names)[Count])
{
    nfs::u32 found = 0;
    for (const GuestString& name : names)
    {
        found = findAt(guest, menu, name.address);
        if (Item item = Item(guest, found))
            item.setDisabled(true);
    }
    return found;
}

/* The miniature's corner added to an item's place. */
void moveToMiniature(Guest& guest, Item item)
{
    const nfs::u32 at = item.address();
    guest.setWord(at + fe::item::kX, std::uint16_t(guest.word(at + fe::item::kX) + guest.word(kMiniatureX)));
    guest.setWord(at + fe::item::kY, std::uint16_t(guest.word(at + fe::item::kY) + guest.word(kMiniatureY)));
}

/* The map on the miniature, in its style and scale; part of hud's every pass. */
void showMap(Guest& guest, nfs::ThreadState& ts, nfs::u32 menu, nfs::u32 player, nfs::u32 cop)
{
    const nfs::u32 name = guest.frame();
    const nfs::u32 path = guest.frame() + 0x100;
    const nfs::u32 at = slot(player, cop);
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
    const nfs::u32 shapeNumber = guest.dword(kListMapScale + at) + nfs::u32(style) * 4;
    guest.setDword(player == 2 ? kMapShapePlayer2 : kMapShapePlayer1, shapeNumber);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), ts, path, kTwoStrings.address,
                                                guest::Words{kArtFolder, guest.dword(map.address() + kItemFile)});
    const nfs::u32 file =
        nfs::u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(guest.app(), ts, ts.sp - 4, path, 0));
    const nfs::u32 old = signedKey(guest.word(map.address() + kItemTexture));
        const nfs::u32 shape = fe::direct<kFindShape, frontend::eab::shapeFileFind>(
        guest.app(), ts, file, guest.dword(kMapShapes + 4 * shapeNumber));
    const std::int32_t texture = *fe::direct<0x4d6390, port::shapes::artTexturesOf>(guest.app(), ts, ts.sp - 4, shape,
                                                                                    std::int32_t(old), 0);
    guest.setWord(map.address() + kItemTexture, std::uint16_t(texture));
    map.setHidden(false);
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

/* sub_44de00: done's key handler, Enter keeping the places as they are. */
nfs::u32 frontend::hudedit::doneKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (std::int16_t(key) == kKeyEnter)
        guest.setDword(kEditDone, 1);
        return nfs::u32(*fe::direct<kButtonKey, port::buttons::buttonKeyed>(app, ts, ts.sp - 4, item, signedKey(key)));
}

/* sub_44de80, as a pause screen of the HUD opens: the race dimmed, its cameras
 * set, the rectangles kept for going back, the two buttons given their keys. */
nfs::u32 frontend::hudedit::layoutEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    const nfs::u32 cop1 = copOf(guest, ts, 0);
    const nfs::u32 cop2 = copOf(guest, ts, 1);
    // The shade pushed where the game's code pushes it, under the frame; the edx of 1 its code sets
    // is for kHudAlwaysDrawn after the call, not the callee's.
    const nfs::u32 shade = guest.frame() - 4;
    guest.setDword(shade, kFloat0_6);
    fe::direct<0x451530, port::text::dimmed>(app, ts, shade, cop2);
    guest.setDword(kHudAlwaysDrawn, 1);
    guest.setDword(kEditDone, 0);
    fe::direct<kSetCameraOverride, carstate::overrideModePut>(guest.app(), guest.ts(), 0, kCameraWhileEditing);
    if (gameSettings(guest).playMode == 1)
        fe::direct<kSetCameraOverride, carstate::overrideModePut>(guest.app(), guest.ts(), 1, kCameraWhileEditing);
    if (gameSettings(guest).playMode == 1)
    {
        copyBytes(guest, kSavedRectangles, hudRecordAddress(1, cop1) + kRectangles, kPlayerRectangles);
        copyBytes(guest, kSavedRectangles2, hudRecordAddress(2, cop2) + kRectangles, kPlayerRectangles);
    }
    else
    {
        copyBytes(guest, kSavedRectangles, hudRecordAddress(0, cop1) + kRectangles, kPlayerRectangles);
    }
    if (Item done = fe::findItem(guest, menu, kDone))
        guest.setDword(done.address() + kOnKey, kDoneKeyHandler);
    if (Item reset = fe::findItem(guest, menu, kReset))
        guest.setDword(reset.address() + kOnKey, kResetKeyHandler);
    fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, 0);
    if (gameSettings(guest).playMode == 1)
        fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, 1);
    return 0;
}

/* sub_44dfd0, as it closes: unless done left it, the rectangles back and the
 * HUD rebuilt from them; then the race as it was. */
nfs::u32 frontend::hudedit::layoutExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const nfs::u32 cop1 = copOf(guest, ts, 0);
    const nfs::u32 cop2 = copOf(guest, ts, 1);
    if (guest.dword(kEditDone) == 0)
    {
        if (gameSettings(guest).playMode == 1)
        {
            copyBytes(guest, hudRecordAddress(1, cop1) + kRectangles, kSavedRectangles, kPlayerRectangles);
            copyBytes(guest, hudRecordAddress(2, cop2) + kRectangles, kSavedRectangles2, kPlayerRectangles);
        }
        else
        {
            copyBytes(guest, hudRecordAddress(0, cop1) + kRectangles, kSavedRectangles, kPlayerRectangles);
        }
        for (nfs::u32 player = 0; player < (gameSettings(guest).playMode == 1 ? 2u : 1u); ++player)
        {
            for (std::int32_t element = 0; element < kElements; ++element)
                fe::direct<0x4853c0, frontend::hudr::rebuildElement>(app, ts, player, nfs::u32(element));
            fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, player);
        }
    }
    // No element highlighted (-1) in either view, in the part's own colour (0).
    fe::direct<0x482030, frontend::hudz::setGridHighlight>(app, ts, 0xffffffff, nfs::u32(0), nfs::u32(0));
    fe::direct<0x482030, frontend::hudz::setGridHighlight>(app, ts, 0xffffffff, nfs::u32(1), nfs::u32(0));
    guest.setDword(kHudAlwaysDrawn, 0);
    fe::direct<kSetCameraOverride, carstate::overrideModePut>(guest.app(), guest.ts(), 0, kCameraAfterEditing);
    if (gameSettings(guest).playMode == 1)
        fe::direct<kSetCameraOverride, carstate::overrideModePut>(guest.app(), guest.ts(), 1, kCameraAfterEditing);
    if (gameSettings(guest).playMode == 1)
    {
        fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ 0, 0, 0, 0 });
        fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ 1, 0, 0, 0 });
    }
    return 0;
}

/* sub_44de20, the reset button's key handler: Enter takes the defaults from
 * disc and puts every element of the screen in its new place.  The handler
 * leaves ebx its last item and edx that item's number. */
nfs::u32 frontend::hudedit::resetKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
        if (std::int16_t(key) != kKeyEnter)
        return nfs::u32(*fe::direct<kButtonKey, port::buttons::buttonKeyed>(app, ts, ts.sp - 4, item, signedKey(key)));
    guest::direct<0x485680, loadDefaults>(app, ts);
    nfs::u32 index = (key & 0xffff0000) | guest.word(menu + fe::menu::kFirstItem);
    for (;; ++index)
    {
        const nfs::u32 number = signedKey(index);
        const nfs::u32 at = fe::item::kTable + fe::item::kSize * number;
        if (guest.dword(at) == 0)
        {
            return 2;
        }
        if (guest.dword(at) == kElementItem)
        {
            guest::direct<0x45ca90, placeElement>(app, ts, at);
            // edx and ebx as the game's code has them (the item, the index), unread.
            fe::direct<0x45c9d0, frontend::menuf::elementToRecord>(app, ts, at);
        }
    }
}

/* sub_45ca90: an element's item put where its rectangle and the race's HUD
 * have it; eax what it copied last. */
nfs::u32 frontend::hudedit::placeElement(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const std::int32_t number = std::int32_t(guest.dword(item + kElementNumber));
    const nfs::u32 player = number < kElements ? 0 : 1;
    const nfs::u32 element = nfs::u32(number) - (player != 0 ? nfs::u32(kElements) : 0);
    const nfs::u32 owner = (gameSettings(guest).playMode == 1 ? 1 : 0) + player;
    const nfs::u32 cop = copOf(guest, ts, player);
    const nfs::u32 offset = element * kRectangleBytes;
    copyBytes(guest, item + kElementRectangle, hudRecordAddress(owner, cop) + kRectangles + offset, kRectangleBytes);
    const nfs::u32 placed = offset + player * kPlayerRectangles;
    guest.setWord(item + fe::item::kX, guest.word(kPlacedLeft + placed));
    guest.setWord(item + fe::item::kY, guest.word(kPlacedTop + placed));
    guest.setDword(item + kElementRight, guest.dword(kPlacedRight + placed));
    const nfs::u32 bottom = guest.dword(kPlacedBottom + placed);
    guest.setDword(item + kElementBottom, bottom);
    return bottom;
}

/* sub_485680: the rectangles' defaults for this language, from a text file of
 * fields -- a name and four numbers for each of the 23 elements, for each
 * player and each of racer and cop.  Besides the reset button, the game's
 * start calls it (sub_485880). */
nfs::u32 frontend::hudedit::loadDefaults(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x18c);
    const nfs::u32 path = guest.frame();         // char[0x100]
    const nfs::u32 name = guest.frame() + 0x100;
    const nfs::u32 letters = guest.frame() + 0x180;
    copyBytes(guest, letters, kLanguageLetters, 7);
    const std::uint32_t letter = guest.byte(letters + gameSettings(guest).language);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kDefaultsName.address, guest::Words{kHudFolder, letter});
    const nfs::u32 file = *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, path, kLoadFileFlags);
    if (file == 0)
        return 0;
    const nfs::u32 separators = kDefaultsSeparators.address;
    nfs::u32 token = fe::direct<kStrtok, watcom::clib::strtok>(guest.app(), guest.ts(), file, separators);
    const auto scan = [&](nfs::u32 format, nfs::u32 to) {
        fe::direct<kSscanf, watcom::clib::sscanf>(guest.app(), guest.ts(), token, format, guest::Words{to});
        token = fe::direct<kStrtok, watcom::clib::strtok>(guest.app(), guest.ts(), 0, separators);
    };
    for (nfs::u32 player = 0; player < 3; ++player)
    {
        for (nfs::u32 cop = 0; cop < 2; ++cop)
        {
            for (std::int32_t element = 0; element < kElements; ++element)
            {
                const nfs::u32 rectangle = hudRecordAddress(player, cop) + kRectangles + element * kRectangleBytes;
                scan(kScanName.address, name);
                scan(kScanFloat.address, rectangle + 4);
                scan(kScanFloat.address, rectangle);
                scan(kScanFloat.address, rectangle + 0xc);
                scan(kScanFloat.address, rectangle + 8);
            }
        }
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
    for (nfs::u32 player = 0; player < 3; ++player)
    {
        for (nfs::u32 cop = 0; cop < 2; ++cop)
        {
            const nfs::u32 from = kLanguageRectangles + gameSettings(guest).language * 0x60 + player * 0x20 + cop * 0x10;
            copyBytes(guest, hudRecordAddress(player, cop) + kLanguageRectangle, from, kRectangleBytes);
        }
    }
    if (guest.dword(kRebuildAfterDefaults) == 0)
        return kCopRecord;   // what the last sum above left in eax
    for (nfs::u32 player = 0; player < 2; ++player)
    {
        for (std::int32_t element = 0; element < kElements; ++element)
            fe::direct<0x4853c0, frontend::hudr::rebuildElement>(app, ts, player, nfs::u32(element));
    }
    nfs::u32 rebuilt = *fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, 0);
    if (gameSettings(guest).playMode == 1)
        rebuilt = *fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, 1);
    return rebuilt;
}

/* sub_45e310: the images in their places on the miniature. */
nfs::u32 frontend::hudedit::placeImages(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const bool split = gameSettings(guest).playMode == 1;
    const nfs::u32 left = split ? kImageSplitLeft : kImageLeft;
    const nfs::u32 top = split ? kImageSplitTop : kImageTop;
    nfs::u32 found = 0;
    for (nfs::u32 i = 0; i < kImageCount; ++i)
    {
        const nfs::u32 image = kImages + kImageBytes * i;
        found = findAt(guest, menu, guest.dword(image));
        if (Item item = Item(guest, found))
        {
            guest.setWord(item.address() + fe::item::kX,
                          std::uint16_t(guest.word(kMiniatureX) + guest.word(image + left)));
            guest.setWord(item.address() + fe::item::kY,
                          std::uint16_t(guest.word(kMiniatureY) + guest.word(image + top)));
        }
    }
    return found;
}

/* sub_45e3d0: only this player's and cop's controls shown, and on the miniature
 * what they say.  With the HUD off, the controls are greyed and no image shows
 * but the rear view.  ebx and eax are what its sums leave. */
nfs::u32 frontend::hudedit::showScreenItems(App* app, ThreadState& ts, u32 menu, u32 player, u32 cop)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x10c);
    const nfs::u32 name = guest.frame();    // char[0x100]
    for (nfs::u32 p = 0; p < 3; ++p)
    {
        for (nfs::u32 c = 0; c < 2; ++c)
        {
            for (nfs::u32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(p, c, i)));
        }
    }
    const HudRecord& fields = gameSettings(guest).hudOf(player, cop);
    if (fields.shown == 0)
    {
        for (nfs::u32 i = 0; i < kControlCount; ++i)
        {
            if (Item item = find(guest, menu, guest.dword(control(player, cop, i))))
            {
                item.setHidden(false);
                item.setDisabled(true);
            }
        }
        nfs::u32 last = 0;   // the item the last look-up found
        for (nfs::u32 i = 0; i < kImageCount; ++i)
        {
            imageName(guest, name, guest.dword(kImages + kImageBytes * i), player);
            last = hide(guest, menu, name);
        }
        if (player != 0)
            return last;
        if (Item item = find(guest, menu, guest.dword(control(0, cop, kMirrorButton))))
            item.setDisabled(false);
        if (Item item = find(guest, menu, guest.dword(control(0, cop, kMirrorCascade))))
            item.setDisabled(false);
        if (gameSettings(guest).hudOf(0, cop).rearView == 0)
            return cop * kCopRecord;
        return show(guest, menu, guest.dword(kImages));
    }

    for (nfs::u32 i = 0; i < kControlCount; ++i)
    {
        if (Item item = find(guest, menu, guest.dword(control(player, cop, i))))
        {
            item.setHidden(false);
            item.setDisabled(false);
        }
    }
    guest::direct<0x45e160, greyMirrors>(app, ts, menu);
    guest::direct<0x45e210, greyOpponentNames>(app, ts, menu);
    if (gameSettings(guest).hudOf(0, cop).rearView != 0 && player == 0)
        show(guest, menu, kRearImage.address);
    guest::direct<0x45dae0, showSpeedometer>(app, ts, player, cop, menu);
    guest::direct<0x45deb0, showTachometer>(app, ts, player, cop, menu);
    if (guest.dword(kListMapStyle + slot(player, cop)) != 2)
    {
        imageName(guest, name, kMapImage.address, player);
        show(guest, menu, name);
    }
    if (fields.opponentName != 0)
    {
        imageName(guest, name, kOppnImage.address, player);
        setHidden(guest, menu, name, cop != 0);
    }
    nameWithTwo(guest, name, cop != 0 ? kOppcImage.address : kOpplImage.address, player);
    nfs::u32 ebx = menu;
    if (Item item = find(guest, menu, name))
    {
        item.setHidden(fields.opponentList == 0);
        ebx = player * kPlayerRecord;
    }
    if (guest.dword(kListSplitTime + slot(player, cop)) != 5)
    {
        imageName(guest, name, kSplitImage.address, player);
        setHidden(guest, menu, name, cop != 0);
    }
    imageName(guest, name, kCopRadarImage.address, player);
    setHidden(guest, menu, name, cop == 0);
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
    return show(guest, menu, kLapsImage2.address);
}

/* sub_45e960, hud as it opens: the records kept for Cancel, the screen's
 * buttons for who plays, the miniature, and the lists from the records. */
nfs::u32 frontend::hudedit::hudEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x108);
    const nfs::u32 name = guest.frame();    // char[0x100]
    copyBytes(guest, kSavedRecords, SETTINGS_ADDRESS(hud), kRecordsBytes);
    if (Item credits = fe::findItem(guest, menu, kCredits))
    {
        if (std::int32_t(gameSettings(guest).playMode) > 1)
            credits.setDisabled(true);
    }
    const bool hotPursuit = gameSettings(guest).raceType == kHotPursuit;
    if (gameSettings(guest).playMode == 1)
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
    for (nfs::u32 i = 0; i < 3; ++i)
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
    if (gameSettings(guest).playMode == 1)
        guest::direct<0x45e3d0, showScreenItems>(app, ts, menu, 2, 0);
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
            for (nfs::u32 i = 0; i < kImageCount; ++i)
            {
                fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kNameAndChar.address, guest::Words{guest.dword(kImages + kImageBytes * i), kCharTwo});
                if (Item item = find(guest, menu, name))
                    moveToMiniature(guest, item);
            }
        }
    }
    if (gameSettings(guest).playMode == 1)
    {
        if (Item screen = fe::findItem(guest, menu, kScreen))
        {
            const nfs::u32 art = screen.address() + kItemArt;
            const bool player2On = gameSettings(guest).hudOf(2, 0).shown != 0;
            if (gameSettings(guest).hudOf(1, 0).shown != 0)
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
        for (nfs::u32 c = 0; c < 2; ++c)
        {
            for (nfs::u32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(0, c, i)));
        }
        for (nfs::u32 c = 0; c < 2; ++c)
        {
            for (nfs::u32 i = 0; i < kControlCount; ++i)
                hide(guest, menu, guest.dword(control(2, c, i)));
        }
        for (nfs::u32 i = 0; i < kControlCount; ++i)
            hide(guest, menu, guest.dword(control(1, 1, i)));
    }
    else
    {
        guest.setDword(kOnOff, gameSettings(guest).hud[0].shown);
        if (Item screen = fe::findItem(guest, menu, kScreen))
            guest.setDword(screen.address() + kItemArt, kArtOn.address);
        for (nfs::u32 p = 1; p < 3; ++p)
        {
            for (nfs::u32 c = 0; c < 2; ++c)
            {
                for (nfs::u32 i = 0; i < kControlCount; ++i)
                    hide(guest, menu, guest.dword(control(p, c, i)));
            }
        }
        for (nfs::u32 i = 0; i < kControlCount; ++i)
            hide(guest, menu, guest.dword(control(0, 1, i)));
    }
    guest::direct<0x45e310, placeImages>(app, ts, menu);

    // A list's entries are the settings from 1; its last, the setting 0, none.
    const auto toList = [](nfs::u32 setting, nfs::u32 none) { return setting != 0 ? setting - 1 : none; };
    FloatMath x87(guest);
    for (nfs::u32 p = 0; p < 3; ++p)
    {
        for (nfs::u32 c = 0; c < 2; ++c)
        {
            HudRecord& fields = gameSettings(guest).hudOf(p, c);
            const nfs::u32 at = slot(p, c);
            std::int32_t scale = x87.truncated(fp::add(nfs::Float(fields.mapScale), x87.load(kFloatMinusOne)));
            if (scale < 0)
                scale = 0;
            if (scale > 3)
                scale = 3;
            guest.setDword(kListMapScale + at, nfs::u32(scale));
            guest.setDword(kListSpeedometer + at, toList(fields.speedometerForm, 4));
            guest.setDword(kListTachometer + at, toList(fields.tachometerForm, 3));
            guest.setDword(kListMapStyle + at, toList(fields.mapStyle, 2));
            guest.setDword(kListSplitTime + at, toList(fields.splitTimeForm, 5));
        }
    }

    if (gameSettings(guest).playMode == 1)
    {
        guest::direct<0x45dae0, showSpeedometer>(app, ts, 1, 0, menu);
        guest::direct<0x45deb0, showTachometer>(app, ts, 1, 0, menu);
        guest::direct<0x45dae0, showSpeedometer>(app, ts, 2, 0, menu);
        guest::direct<0x45deb0, showTachometer>(app, ts, 2, 0, menu);
        const auto hideIfOff = [&](const GuestString& image, nfs::u32 setting) {
            if (Item item = fe::findItem(guest, menu, image))
            {
                if (setting == 0)
                    item.setHidden(true);
            }
        };
        hideIfOff(kOppnImage, gameSettings(guest).hudOf(1, 0).opponentName);
        hideIfOff(kOpplImage, gameSettings(guest).hudOf(1, 0).opponentList);
        hideIfOff(kOppnImage2, gameSettings(guest).hudOf(2, 0).opponentName);
        hideIfOff(kOpplImage2, gameSettings(guest).hudOf(2, 0).opponentList);
        hide(guest, menu, kCopRadarImage2.address);
        hide(guest, menu, kOppcImage2.address);
    }
    else
    {
        guest::direct<0x45dae0, showSpeedometer>(app, ts, 0, 0, menu);
        guest::direct<0x45deb0, showTachometer>(app, ts, 0, 0, menu);
        if (gameSettings(guest).hud[0].opponentName == 0)
            hide(guest, menu, kOppnImage.address);
        if (gameSettings(guest).hud[0].opponentList == 0)
            hide(guest, menu, kOpplImage.address);
        if (gameSettings(guest).hud[0].rearView == 0)
            hide(guest, menu, kRearImage.address);
        if (gameSettings(guest).hud[0].splitTimeForm == 0)
            hide(guest, menu, kSplitImage.address);
        for (nfs::u32 i = 0; i < kImageCount; ++i)
        {
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kNameAndChar.address, guest::Words{guest.dword(kImages + kImageBytes * i), kCharTwo});
            hide(guest, menu, name);
        }
    }
    hide(guest, menu, kCopRadarImage.address);
    hide(guest, menu, kOppcImage.address);
    guest::direct<0x45e160, greyMirrors>(app, ts, menu);
    guest::direct<0x45e210, greyOpponentNames>(app, ts, menu);
    return 0;
}

/* sub_45f160, hud as it closes: Cancel puts the records back; any other way
 * out, the lists go into the records. */
nfs::u32 frontend::hudedit::hudExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    const nfs::u32 item = fe::item::kTable + fe::item::kSize * nfs::u32(first + selected);
    if (guest.dword(item) == 5
        && *fe::direct<0x4429c0, port::menus::codelinkMatched>(app, ts, ts.sp - 4,
                                                                guest.dword(item + fe::item::kCodelink),
                                                                kCancel.address, 0) != 0)
    {
        copyBytes(guest, SETTINGS_ADDRESS(hud), kSavedRecords, kRecordsBytes);
        return 0;
    }
    const auto toSetting = [](nfs::u32 entry, nfs::u32 none) { return entry != none ? entry + 1 : 0; };
    for (nfs::u32 p = 0; p < 3; ++p)
    {
        for (nfs::u32 c = 0; c < 2; ++c)
        {
            HudRecord& fields = gameSettings(guest).hudOf(p, c);
            const nfs::u32 at = slot(p, c);
            const nfs::Float scale = fp::add(1.0, FloatMath::integer(std::int32_t(guest.dword(kListMapScale + at))));
            fields.mapScale = float(scale);
            fields.speedometerForm = toSetting(guest.dword(kListSpeedometer + at), 4);
            fields.tachometerForm = toSetting(guest.dword(kListTachometer + at), 3);
            fields.mapStyle = toSetting(guest.dword(kListMapStyle + at), 2);
            fields.splitTimeForm = toSetting(guest.dword(kListSplitTime + at), 5);
        }
    }
    return 0;
}

/* sub_45f300: the miniature's art, by whose HUD is on (the other player's, in
 * split screen, by the art it has now), and whose frame is lit.  Loaded anew
 * only when it is another.  ebx its name, as it leaves it. */
nfs::u32 frontend::hudedit::setScreenArt(App* app, ThreadState& ts, u32 menu, u32 player, u32 cop)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x104);
    const nfs::u32 path = guest.frame();    // char[0x100]
    const Item screen = fe::findItem(guest, menu, kScreen);
    if (!screen)
    {
        return 0;
    }
    const nfs::u32 at = screen.address();
    const bool on = gameSettings(guest).hudOf(player, cop).shown != 0;
    nfs::u32 art;
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
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), art, guest.dword(at + kItemArt)) == 0)
        return 0;
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), ts, path, kTwoStrings.address,
                                                guest::Words{kArtFolder, guest.dword(at + kItemFile)});
    const nfs::u32 file =
        nfs::u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, path, 0));
    const nfs::u32 old = signedKey(guest.word(at + kItemTexture));
        const nfs::u32 shape = fe::direct<kFindShape, frontend::eab::shapeFileFind>(app, ts, file, art);
    const nfs::u32 texture =
        nfs::u32(*fe::direct<kShapeTexture, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, std::int32_t(old), 0));
    guest.setWord(at + kItemTexture, std::uint16_t(texture));
    guest.setDword(at + kItemArt, art);
    return texture;
}

/* sub_45f4c0, hud on every pass: whose HUD is laid out, by the list's choice,
 * and then the items for what has changed. */
nfs::u32 frontend::hudedit::hudFrame(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x15c);
    const nfs::u32 name = guest.frame();    // char[0x100]
    const auto chosenText = [&] { return guest.word(guest.dword(kChoices) + 2 * guest.dword(kChoice)); };
    nfs::u32 player = 0;
    if (gameSettings(guest).playMode == 1)
    {
        const std::uint16_t text = chosenText();
        player = text == kTextPlayer2 || text == kTextPlayer2Cop ? 2 : 1;
    }
    nfs::u32 cop = 0;
    if (gameSettings(guest).raceType == kHotPursuit)
    {
        const std::uint16_t text = chosenText();
        cop = text == kTextCop || text == kTextPlayer1Cop || text == kTextPlayer2Cop ? 1 : 0;
    }
    HudRecord& fields = gameSettings(guest).hudOf(player, cop);
    const nfs::u32 at = slot(player, cop);
    const nfs::u32 own = 4 * player;

    if (guest.dword(kShownChoice) != guest.dword(kChoice))
    {
        guest.setDword(kShownChoice, guest.dword(kChoice));
        guest.setDword(kOnOff, fields.shown);
        guest.setDword(kShownOnOff, 0xffffffff);
        guest::direct<0x45f300, setScreenArt>(app, ts, menu, player, cop);
        // Every image looked up, by both players' names, and nothing done with
        // them: the test of its flags that follows in the game goes nowhere.
        for (nfs::u32 i = 0; i < kImageCount; ++i)
        {
            const nfs::u32 image = guest.dword(kImages + kImageBytes * i);
            guest.copyString(name, image);
            find(guest, menu, name);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kNameAndChar.address, guest::Words{image, kCharTwo});
            find(guest, menu, name);
        }
    }

    if (guest.dword(kShownOnOff) != guest.dword(kOnOff))
    {
        const std::uint32_t onOff = guest.dword(kOnOff);
        guest.setDword(kShownOnOff, onOff);
        fields.shown = onOff;
        guest::direct<0x45e3d0, showScreenItems>(app, ts, menu, player, cop);
        guest::direct<0x45f300, setScreenArt>(app, ts, menu, player, cop);
    }

    if (gameSettings(guest).playMode != 1)
    {
        const std::uint32_t rearView = gameSettings(guest).hudOf(0, cop).rearView;
        if (guest.dword(kShownRearView) != rearView)
        {
            guest.setDword(kShownRearView, rearView);
            setHidden(guest, menu, kRearImage.address, rearView == 0);
        }
    }

    if (fields.shown == 0)
        return 0;

    if (guest.dword(kListMapScale + at) != guest.dword(kShownMapScale + own)
        || guest.dword(kListMapStyle + at) != guest.dword(kShownMapStyle + own))
    {
        showMap(guest, ts, menu, player, cop);
    }
    if (guest.dword(kListSpeedometer + at) != guest.dword(kShownSpeedometer + own))
    {
        guest.setDword(kShownSpeedometer + own, guest.dword(kListSpeedometer + at));
        guest::direct<0x45dae0, showSpeedometer>(app, ts, player, cop, menu);
    }
    if (guest.dword(kListTachometer + at) != guest.dword(kShownTachometer + own))
    {
        guest.setDword(kShownTachometer + own, guest.dword(kListTachometer + at));
        guest::direct<0x45deb0, showTachometer>(app, ts, player, cop, menu);
    }
    if (guest.dword(kListSplitTime + at) != guest.dword(kShownSplitTime + own))
    {
        guest.setDword(kShownSplitTime + own, guest.dword(kListSplitTime + at));
        guest.copyString(name, player == 2 ? kSplitImage2.address : kSplitImage.address);
        setHidden(guest, menu, name, guest.dword(kShownSplitTime + own) == 5);
    }
    if (fields.opponentName != guest.dword(kShownOpponentName + own))
    {
        guest.setDword(kShownOpponentName + own, fields.opponentName);
        guest.copyString(name, player == 2 ? kOppnImage2.address : kOppnImage.address);
        setHidden(guest, menu, name, guest.dword(kShownOpponentName + own) == 0);
    }
    if (fields.opponentList != guest.dword(kShownOpponentList + own))
    {
        guest.setDword(kShownOpponentList + own, fields.opponentList);
        nameWithTwo(guest, name, cop != 0 ? kOppcImage.address : kOpplImage.address, player);
        setHidden(guest, menu, name, guest.dword(kShownOpponentList + own) == 0);
    }
    // The opponents' other list, always hidden.
    nameWithTwo(guest, name, cop != 0 ? kOpplImage.address : kOppcImage.address, player);
    hide(guest, menu, name);
    return 0;
}

/* sub_45dae0: the speedometer on the miniature. */
nfs::u32 frontend::hudedit::showSpeedometer(App* app, ThreadState& ts, u32 player, u32 cop, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x48);
    return showInstrument(guest, menu, player, cop, kSpeedometerForms, kListSpeedometer);
}

/* sub_45deb0: the tachometer on the miniature. */
nfs::u32 frontend::hudedit::showTachometer(App* app, ThreadState& ts, u32 player, u32 cop, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x38);
    return showInstrument(guest, menu, player, cop, kTachometerForms, kListTachometer);
}

/* sub_45e160: the rear view's buttons and lists greyed out. */
nfs::u32 frontend::hudedit::greyMirrors(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return greyOut(guest, menu, kMirrorItems);
}

/* sub_45e210: a cop's opponents' names and split times greyed out. */
nfs::u32 frontend::hudedit::greyOpponentNames(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return greyOut(guest, menu, kCopItems);
}

nfs::u32 frontend::hudEditDoneKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44de00", 0, 0, nullptr, 0, 0 };
    const nfs::u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::doneKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::hudEditResetKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44de20", 0, 0, nullptr, 0, 0 };
    const nfs::u32 item = args.eax, menu = args.edx, key = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::resetKey(app, ts, item, menu, key); });
}

nfs::u32 frontend::hudEditLayoutOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44de80", 0, 0, kLayoutEnterStrings, std::size(kLayoutEnterStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::layoutEnter(app, ts, menu); });
}

nfs::u32 frontend::hudEditLayoutOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44dfd0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::layoutExit(app, ts); });
}

nfs::u32 frontend::hudEditPlaceElement(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45ca90", 0, 0, nullptr, 0 };
    const nfs::u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::placeElement(app, ts, item); });
}

nfs::u32 frontend::hudEditShowSpeedometer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45dae0", 0, 0, kSpeedometerStrings, std::size(kSpeedometerStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 player = args.eax, cop = args.edx, menu = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::showSpeedometer(app, ts, player, cop, menu); });
}

nfs::u32 frontend::hudEditShowTachometer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45deb0", 0, 0, kTachometerStrings, std::size(kTachometerStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 player = args.eax, cop = args.edx, menu = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::showTachometer(app, ts, player, cop, menu); });
}

nfs::u32 frontend::hudEditGreyMirrors(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45e160", 0, 0, kMirrorItems, std::size(kMirrorItems),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::greyMirrors(app, ts, menu); });
}

nfs::u32 frontend::hudEditGreyOpponentNames(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45e210", 0, 0, kCopItems, std::size(kCopItems), fe::kRestoreEcx | fe::kRestoreEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::greyOpponentNames(app, ts, menu); });
}

nfs::u32 frontend::hudEditPlaceImages(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45e310", 0, 0, nullptr, 0 };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::placeImages(app, ts, menu); });
}

nfs::u32 frontend::hudEditShowScreenItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45e3d0", 0, 0, kScreenItemsStrings, std::size(kScreenItemsStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 menu = args.eax, player = args.edx, cop = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::showScreenItems(app, ts, menu, player, cop); });
}

nfs::u32 frontend::hudEditOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45e960", 0, 0, kHudEnterStrings, std::size(kHudEnterStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::hudEnter(app, ts, menu); });
}

nfs::u32 frontend::hudEditOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45f160", 0, 0, kHudExitStrings, std::size(kHudExitStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::hudExit(app, ts, menu); });
}

nfs::u32 frontend::hudEditSetScreenArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45f300", 0, 0, kScreenArtStrings, std::size(kScreenArtStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 menu = args.eax, player = args.edx, cop = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::setScreenArt(app, ts, menu, player, cop); });
}

nfs::u32 frontend::hudEditOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45f4c0", 0, 0, kHudFrameStrings, std::size(kHudFrameStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::hudFrame(app, ts, menu); });
}

nfs::u32 frontend::hudEditLoadDefaults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_485680", 0, 0, kDefaultsStrings, std::size(kDefaultsStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudedit::loadDefaults(app, ts); });
}

}
