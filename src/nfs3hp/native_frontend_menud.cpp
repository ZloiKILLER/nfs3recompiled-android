#include "native_frontend_menud.h"
#include "native_frontend_dialogs.h"
#include "native_watcom.h"
#include "native_frontend.h"
#include "native_frontend_menua.h"
#include "native_frontend_menue.h"
#include "native_frontend_menuf.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"

#include "native_frontend_draw.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_loop.h"
#include "native_frontend_menub.h"
#include "native_graphics.h"
#include "port_buttons.h"
#include "port_draw2d.h"
#include "port_shapes.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <lib/app.h>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
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

/* What the menus lean on, past their screens' handlers, decompiled:
 *
 *   the pointer     its art loaded and let go as the menus start and end, and
 *                   the two questions the lists and the network screens ask of it;
 *   the effects     what stands behind the buttons (port_buttons.cpp draws those):
 *                   the band of moving lines, a screen's own animation, and the
 *                   logo with its glint sweeping over;
 *   the lobby       the network screen's table of players;
 *   a pause's end   the menu's state put away as a race goes on.
 *
 * The rest of the address range around them is the movie player's, the network
 * protocol's, the window library's and the race's.
 *
 * Plan step 5: each function a typed core in frontend::menud (native_frontend_menud.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using nfs::u32;

// The game's, shared.
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kStrcmp = 0x4ee310;
constexpr u32 kArtFolder = 0x7a2db4;

std::int32_t int16At(const Guest& guest, u32 address)
{
    return std::int16_t(guest.word(address));
}

/* ---------------------------------------------------------------------------
 * The pointer
 * ------------------------------------------------------------------------- */

constexpr u32 kPointerArt = 0x55e5fc;           // mouse.fsh while it is loaded, or 0
constexpr u32 kPointerPath = 0x79fbe4;
constexpr u32 kPointerX = 0x55e5e4;
constexpr u32 kPointerY = 0x55e5e8;
constexpr u32 kPointerHotX = 0x563d3c;
constexpr u32 kPointerHotY = 0x563d40;
constexpr std::uint32_t kLoadHigh = 0x10;
constexpr GuestString kPointerStrings[] = { { 0x53c5e4, "%smouse.fsh" } };

}  // namespace

/* sub_49c140, as the menus start: mouse.fsh made the pointer's textures, loaded
 * unless it already is, then the file let go. */
nfs::u32 frontend::menud::loadPointerArt(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (guest.dword(kPointerArt) == 0)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kPointerPath, kPointerStrings[0].address, guest::Words{kArtFolder});
        guest.setDword(kPointerArt, *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, kPointerPath, kLoadHigh));
    }
    guest::direct<0x4daa10, menue::loadPointer>(app, ts, guest.dword(kPointerArt));
    const u32 freed = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(kPointerArt));
    guest.setDword(kPointerArt, 0);
    guest.setDword(kPointerHotX, 0);
    guest.setDword(kPointerHotY, 0);
    return freed;
}

/* sub_49c2a0, as the menus end: the file let go, if it is still loaded. */
nfs::u32 frontend::menud::freePointerArt(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 art = guest.dword(kPointerArt);
    u32 result = eax;
    if (art != 0)
    {
        result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, art);
        guest.setDword(kPointerArt, 0);
    }
    return result;
}

/* sub_49c1c0: the line of a list under the pointer, the list's top at eax, lines
 * of edx each; -1 when it stands above the top.  edx left as the division leaves
 * it, the pointer's y with no division. */
nfs::u32 frontend::menud::pointerListLine(App* app, ThreadState& ts, std::int32_t top, std::int32_t lineHeight)
{
    Guest guest(app, ts);
    const std::int32_t pointerY = std::int32_t(guest.dword(kPointerY));
    if (top > pointerY)
    {
        return u32(-1);
    }
    const std::int32_t below = std::int32_t(u32(pointerY) - u32(top) - 1u);
    return u32(below / lineHeight);
}

/* sub_49c240: whether the pointer is inside the rectangle at eax, edx, ebx wide
 * and ecx high, none of its edges counted.  edx left at what it last looked at. */
nfs::u32 frontend::menud::pointerOverRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height)
{
    Guest guest(app, ts);
    const std::int32_t pointerY = std::int32_t(guest.dword(kPointerY));
    if (std::int32_t(y) >= pointerY)
        return 0;
    const u32 bottom = y + height;
    if (std::int32_t(bottom) <= pointerY)
        return 0;
    const std::int32_t pointerX = std::int32_t(guest.dword(kPointerX));
    if (std::int32_t(x) >= pointerX)
        return 0;
    return std::int32_t(x + width) > pointerX ? 1 : 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The effects
 * ------------------------------------------------------------------------- */

/* The band of moving lines behind a list (sub_4a47c0). */
constexpr u32 kBandClosing = 0x55eb10;            // nonzero: the band shrinks
constexpr u32 kBandBottom = 0x55eb0c;
constexpr u32 kBandLight = 0x55eb14;              // the row the moving light is on
constexpr std::int32_t kBandShortest = 0x60;
constexpr std::int32_t kBandTallest = 0x1df;
constexpr std::int32_t kBandStep = 0x14;
constexpr std::int32_t kBandLightCycle = 0x12c;
constexpr u32 kLitColour = 0xff70b0f0;
constexpr u32 kDimColour = 0xff4060a0;
constexpr std::int32_t kBandRows = 0x26;
constexpr std::int32_t kBandMiddleRow = 0x13;
constexpr std::int32_t kRowGap = 10;
constexpr std::int32_t kBandFirstY = 0x63;
constexpr std::int32_t kBandRowFits = 0x83;         // a row's y from the first, past the band's bottom it shows
// A row: two lines, 2 pixels high, a pixel apart.  The first rows' ends are the
// game's table, int16 each; the rest reach the screen's edge.
constexpr u32 kUpperLineLeft = 0x55e988, kLowerLineLeft = 0x55e98a;
constexpr u32 kUpperLineRight = 0x55ea20, kLowerLineRight = 0x55ea22;
constexpr std::int32_t kLastRowWithEnds = 3;
constexpr std::int32_t kScreenRight = 0x27f;
// The short lines at the top, fading to the right, where the light ends its way.
constexpr std::int32_t kTopRows = 0x12;
constexpr std::int32_t kTopFirstY = 7;
constexpr u32 kTopUpperRight = 0x55ea30, kTopLowerRight = 0x55ea32;
constexpr std::int32_t kTopLightStart = 0xc8;
constexpr u32 kTopFade = 0x20;

/* A line of the band, two pixels high: its ends `endColour`, its middle `midColour`; the drawing's eax. */
u32 drawLine(Guest& guest, nfs::ThreadState& ts, std::int32_t left, std::int32_t top, std::int32_t right, u32 endColour,
               u32 midColour)
{
    const u32 colours[4] = { endColour, midColour, midColour, endColour };
    return u32(*fe::direct<0x4d88f0, port::buttons::colouredRectangle>(
        guest.app(), ts, ts.sp - 4 - 4 * 4, port::Args{ u32(left), u32(top), u32(right), u32(top + 2) },
        colours));
}

}  // namespace

nfs::u32 frontend::menud::drawBand(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    // The band grows or shrinks to where it is going.
    const std::int32_t bandBottom = std::int32_t(guest.dword(kBandBottom));
    if (guest.dword(kBandClosing) != 0)
    {
        if (bandBottom > kBandShortest)
            guest.setDword(kBandBottom, u32(bandBottom - kBandStep));
    }
    else
    {
        if (bandBottom < kBandTallest)
            guest.setDword(kBandBottom, u32(bandBottom + kBandStep));
        if (std::int32_t(guest.dword(kBandBottom)) > kBandTallest)
            guest.setDword(kBandBottom, u32(kBandTallest));
    }
    // The light moves a row a pass, and starts again.
    const u32 light = guest.dword(kBandLight);
    guest.setDword(kBandLight, light != 0 ? light - 1 : u32(kBandLightCycle));

    // The rows, brightest in the middle, as far down as the band is.
    for (std::int32_t row = 0; row < kBandRows; ++row)
    {
        const std::int32_t fromEnd = row <= kBandMiddleRow ? row : kBandRows - row;
        const std::int32_t brightness = fromEnd * 224 / kBandMiddleRow + 0x1f;
        const u32 tint = u32(row) == guest.dword(kBandLight) ? kLitColour : kDimColour;
        const u32 colour = guest::direct<0x4d9180, menue::scaleColour>(app, ts, tint, brightness);
        if (row * kRowGap + kBandRowFits <= std::int32_t(guest.dword(kBandBottom)))
            continue;
        const std::int32_t y = row * kRowGap + kBandFirstY;
        const u32 slot = 4 * u32(row);
        const bool toEdge = row > kLastRowWithEnds;
        drawLine(guest, ts, int16At(guest, kUpperLineLeft + slot), y,
                 toEdge ? kScreenRight : int16At(guest, kUpperLineRight + slot), colour, colour);
        drawLine(guest, ts, int16At(guest, kLowerLineLeft + slot), y + 3,
                 toEdge ? kScreenRight : int16At(guest, kLowerLineRight + slot), colour, colour);
    }

    u32 drawn = 0;
    for (std::int32_t row = 0; row < kTopRows; ++row)
    {
        const u32 tint = std::int32_t(guest.dword(kBandLight)) - kTopLightStart == row ? kLitColour : kDimColour;
        const u32 faded = guest::direct<0x4d9180, menue::scaleColour>(app, ts, tint, std::int32_t(kTopFade));
        const std::int32_t y = row * kRowGap + kTopFirstY;
        const u32 slot = 4 * u32(row);
        drawLine(guest, ts, 0, y, int16At(guest, kTopUpperRight + slot), tint, faded);
        drawn = drawLine(guest, ts, 0, y + 3, int16At(guest, kTopLowerRight + slot), tint, faded);
    }
    return drawn;
}

namespace
{

/* A screen's own animation, every pass, by the screen's name (sub_4a49c0). */
constexpr GuestString kEffectScreens[] = {
    { 0x53cf78, "showcase" },
    { 0x53cf84, "compare2" },
    { 0x53cf90, "history" },
};

}  // namespace

nfs::u32 frontend::menud::drawScreenEffect(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, ts);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kEffectScreens[0].address) == 0)
        fe::direct<0x453940, menub::carStatsDraw>(app, ts);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kEffectScreens[1].address) == 0)
        fe::direct<0x454780, menub::carListDraw>(app, ts);
    u32 result = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kEffectScreens[2].address);
    if (result == 0)
        result = fe::direct<0x453f60, menub::carSpecsDraw>(app, ts);
    return result;
}

namespace
{

/* The logo: logo.qfs made a texture (sub_4a4390), its four pictures and its
 * glint for the screen shown (sub_4a4ba0), all of it drawn (sub_4a4c20). */
constexpr GuestString kLogoStrings[] = {
    { 0x53cf18, "%s%s" },
    { 0x53cf0c, "logo.qfs" },
    { 0x53cf20, "logo" },
};
constexpr GuestString kArtStrings[] = {
    { 0x53cfdc, "glw0" },
    { 0x53cfa4, "trkinfo" },
};
constexpr GuestString kGlintName = kArtStrings[0];
constexpr GuestString kTrackInfoScreen = kArtStrings[1];
constexpr u32 kFindShapeInFile = 0x4ef270;
constexpr u32 kDisplayFlags = 0x7a3a58;           // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr u32 kLogoTexture = 0x7a1b94;
constexpr u32 kLogoTextureSize = 0x100;
constexpr u32 kLogoTextureModeNoTranslucency = 3;

}  // namespace

nfs::u32 frontend::menud::startLogoTexture(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x100);
    const u32 path = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, path, kLogoStrings[0].address,
                                                guest::Words{kArtFolder, kLogoStrings[1].address});
    const u32 file = fe::direct<0x4956f0, draw::loadQfsFlags>(app, ts, path, 0u);
    const u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, file, kLogoStrings[2].address);
    const u32 mode = (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? kLogoTextureModeNoTranslucency : 0;
    fe::direct<0x4d6070, port::shapes::textureOf>(app, ts, ts.sp - 4, kLogoTexture, shape,
                                                  std::int32_t(kLogoTextureSize), mode);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

namespace
{

constexpr u32 kLogoPictureNames = 0x4a4050;       // the four shapes' names, kept in the code's segment
constexpr u32 kGlintShape = 0x8b7634;
constexpr u32 kLogoPictures = 0x55eb18;
constexpr std::int32_t kLogoPictureCount = 4;
constexpr u32 kMenuShapeFile = 0x559228;

}  // namespace

nfs::u32 frontend::menud::loadLogoPictures(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x10);
    const u32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, ts);
    guest.setDword(kGlintShape, u32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4,
                                                                                       kGlintName.address)));
    // Track info shows the first picture only.
    std::int32_t pictures = kLogoPictureCount;
    if (name != 0 && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kTrackInfoScreen.address) == 0)
        pictures = 1;
    u32 record = 0;
    for (std::int32_t i = 0; i < pictures; ++i)
    {
        const u32 slot = 4 * u32(i);
        const u32 shape = callAt(guest, kFindShapeInFile,
                                       args({ guest.dword(kMenuShapeFile), guest.dword(kLogoPictureNames + slot) }));
        // ecx as the call before left it, handed on.
        record = u32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, 0, 0));
        guest.setDword(kLogoPictures + slot, record);
    }
    return record;
}

namespace
{

/* sub_4a4c20: a logo item at x, y.  The item's clock crosses over the four
 * pictures, a whole turn in 0x400 steps.  Every 300 passes a glint comes: it
 * spins in from the right, grows over the logo, spinning, and shrinks again, a
 * flash either side of the logo at the height of it. */
constexpr u32 kItemShape = 0x44;                  // int16
constexpr u32 kItemClock = 0x46;                  // int16
constexpr std::int16_t kClockTurn = 0x400;
constexpr std::int16_t kClockStep = 2;
constexpr u32 kGlintCountdown = 0x55eb28;
constexpr u32 kGlintLeft = 0x7a1b98;              // where it stands as it comes in, else 0
constexpr std::int32_t kGlintCycle = 0x12c;
constexpr std::int32_t kInEnd = 0x34;               // its time as it comes in, then grows, then shrinks
constexpr std::int32_t kGrowEnd = 0x52;
constexpr std::int32_t kChangeLength = 0x1e;
constexpr std::int32_t kFlashAt = 0xf;
constexpr u32 kGlintComingY = 0x55ea78;           // int16 for each time, as it comes in
constexpr u32 kBigGlintKey = 0x2d;
constexpr float kBigGlint = 75.0f, kSmallGlint = 8.0f;
constexpr u32 kAdditiveBlend = 0x68;
constexpr u32 kDrawTurnedShape = 0x4d6730;        // shape, x, y, nudged left by ecx; the angle, the scale
constexpr u32 kDrawArtShape = 0x4d7160;           // shape, x, y, the colour
constexpr u32 kGlintGrowX = 0x46, kGlintShrinkX = 0x47, kGlintY = 0x3c;
constexpr u32 kFlashX = 0x47, kFlashY = 0x39, kFlashNudge = 0x1e;
constexpr std::int32_t kOtherFlashAngle = 0xb4;
constexpr std::int32_t kGrowFlashAngle = 0x1e, kShrinkFlashAngle = 0x37;
// The game's constants, doubles.
constexpr u32 kComingScale = 0x53cfe4, kOneHalf = 0x53cfec, kThirtieth = 0x53cffc;
constexpr u32 kGrowBias = 0x53cff4, kFive = 0x53d004, kShrinkBias = 0x53d024;
constexpr u32 kFlashScale = 0x53d00c, kFifteenth = 0x53d014, kFlashLeast = 0x53d01c;

double gameDouble(const Guest& guest, u32 address)
{
    const std::uint32_t words[2] = { guest.dword(address), guest.dword(address + 4) };
    double value;
    std::memcpy(&value, words, sizeof value);
    return value;
}

u32 floatBits(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

struct Logo
{
    Guest& guest;
    nfs::ThreadState& ts;
    u32 x, y;
    u32 picture, nextPicture;
    u32 pictureColour, nextPictureColour;

    void blend(bool additive)
    {
        fe::direct<0x431900, graphics::sendState>(guest.app(), ts, kAdditiveBlend, additive ? 1u : 0u);
    }
    void glint(u32 atX, u32 atY, u32 nudge, std::int32_t angle, float scale)
    {
        fe::direct<0x4d6730, port::buttons::turnedShape>(
            guest.app(), ts, ts.sp - 4 - 4 * 2, std::int32_t(guest.dword(kGlintShape)), std::int32_t(atX),
            std::int32_t(atY), std::int32_t(nudge), angle, floatBits(scale));
    }

        /* The function's last calls; each gives back the eax of its last one, which the function ends with. */
    u32 drawPictures()
    {
        u32 left = u32(*fe::direct<kDrawArtShape, port::shapes::drawArt>(
            guest.app(), ts, ts.sp - 4, std::int32_t(picture), std::int32_t(x), std::int32_t(y), pictureColour));
        if (nextPicture != 0)
            left = u32(*fe::direct<kDrawArtShape, port::shapes::drawArt>(
                guest.app(), ts, ts.sp - 4, std::int32_t(nextPicture), std::int32_t(x), std::int32_t(y),
                nextPictureColour));
        return left;
    }
    u32 flash(std::int32_t angle, float scale)
    {
                const auto glintByAddress = [&](std::int32_t turn) { glint(kFlashX, kFlashY, kFlashNudge, turn, scale); };
        blend(true);
        glintByAddress(angle);
        glintByAddress(angle + kOtherFlashAngle);
        return fe::direct<0x431900, graphics::stateAnswer>(guest.app(), ts, kAdditiveBlend, 0u).value_or(0);
    }
};

}  // namespace

nfs::u32 frontend::menud::drawLogo(App* app, ThreadState& ts, u32 item, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x2c);
    const u32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, ts);

    // The clock goes round.
    const std::uint16_t advanced = std::uint16_t(guest.word(item + kItemClock) + kClockStep);
    guest.setWord(item + kItemClock, std::int16_t(advanced) >= kClockTurn ? std::uint16_t(0) : advanced);
    const std::int32_t clock = int16At(guest, item + kItemClock);

    // The two pictures it is between, and how far: each as opaque as the other is not.
    const std::int32_t pictureNumber = clock / 256;
    const std::int32_t between = clock % 256;
    Logo logo{ guest, ts, x, y, 0, 0, 0, 0 };
    logo.picture = guest.dword(kLogoPictures + 4 * u32(pictureNumber));
    guest.setWord(item + kItemShape, std::uint16_t(logo.picture));
    logo.nextPicture = guest.dword(kLogoPictures + 4 * u32((pictureNumber + 1) % kLogoPictureCount));
    logo.pictureColour = (u32(0xff - between) << 24) | 0xffffff;
    logo.nextPictureColour = (u32(between) << 24) | 0xffffff;

    // Track info holds the first picture.
    if (name != 0 && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, kTrackInfoScreen.address) == 0)
    {
        logo.picture = guest.dword(kLogoPictures);
        logo.nextPicture = 0;
        logo.pictureColour = 0xffffffff;
        guest.setWord(item + kItemShape, std::uint16_t(logo.picture));
    }

    const double glintSize =
        fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(app, ts, kBigGlintKey) != 0 ? kBigGlint : kSmallGlint;

    // The glint's own clock.
    const u32 countdown = guest.dword(kGlintCountdown);
    guest.setDword(kGlintCountdown, countdown != 0 ? countdown - 1 : u32(kGlintCycle));
    const std::int32_t time = 2 * (kGlintCycle - std::int32_t(guest.dword(kGlintCountdown)));
    const std::int32_t angle = 10 * time;
    guest.setDword(kGlintLeft, 0);

    u32 result = 0;   // the eax of the last call by address
    if (time < kInEnd)
    {
        const double scaled = fp::mul(nfs::Float(time), gameDouble(guest, kComingScale));
        guest.setDword(kGlintLeft, u32(640 - angle));
        const float scale = float(fp::add(scaled, gameDouble(guest, kOneHalf)));
        logo.blend(true);
        logo.glint(guest.dword(kGlintLeft), u32(int16At(guest, kGlintComingY + 2 * u32(time))), 0, angle, scale);
        logo.blend(false);
        result = logo.drawPictures();
    }
    else if (time - kInEnd < kChangeLength)
    {
        const std::int32_t step = time - kInEnd;
        const double biased = fp::add(glintSize, gameDouble(guest, kGrowBias));
        const double grown = fp::mul(fp::mul(biased, nfs::Float(step)), gameDouble(guest, kThirtieth));
        const float scale = float(fp::add(grown, gameDouble(guest, kFive)));
        logo.blend(true);
        logo.glint(kGlintGrowX, kGlintY, 0, angle, scale);
        logo.blend(false);
        result = logo.drawPictures();
        if (step > kFlashAt)
        {
            const std::int32_t past = step - kFlashAt;
            const double flashed = fp::mul(fp::mul(nfs::Float(past), gameDouble(guest, kFlashScale)),
                                           gameDouble(guest, kFifteenth));
            result = logo.flash(past * 25 / kFlashAt + kGrowFlashAngle, float(fp::add(flashed, gameDouble(guest, kFlashLeast))));
        }
    }
    else if (time - kGrowEnd < kChangeLength)
    {
        const std::int32_t step = time - kGrowEnd;
        const double biased = fp::add(glintSize, gameDouble(guest, kShrinkBias));
        const double shrunk = fp::mul(fp::mul(biased, nfs::Float(step)), gameDouble(guest, kThirtieth));
        const float scale = float(fp::sub(glintSize, shrunk));
        logo.blend(true);
        logo.glint(kGlintShrinkX, kGlintY, 0, angle, scale);
        logo.blend(false);
        result = logo.drawPictures();
        if (step < kFlashAt)
        {
            const double flashed = fp::mul(fp::mul(nfs::Float(step), gameDouble(guest, kFlashScale)),
                                           gameDouble(guest, kFifteenth));
            result = logo.flash(step * 25 / kFlashAt + kShrinkFlashAngle, float(fp::sub(1.0, flashed)));
        }
    }
    else
    {
        result = logo.drawPictures();
    }
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The network screen's table of players (sub_49ae30)
 * ------------------------------------------------------------------------- */

constexpr u32 kMyPlayer = 0x7a2278;               // int16
constexpr u32 kPlayerCount = 0x7a2276;            // int16
constexpr u32 kNetworkKind = 0x7a22e4;            // 0 two machines, else up to eight
constexpr u32 kMachinePlayers = 0x7a227a;         // int16 each: a slot's player, -1 none
constexpr u32 kLobbyLock = 0x55ecb4;
constexpr u32 kPlayerMachine = 0x4a6690;
// A player's record, as they all see it.
constexpr u32 kPlayerRecords = 0x79f5f0;
constexpr u32 kPlayerRecordSize = 0x60;
constexpr u32 kRecordSlot = 0x04;
constexpr u32 kRecordReady = 0x08;                // byte
constexpr u32 kRecordName = 0x09;
constexpr u32 kRecordGears = 0x15;                // byte: 0 one line of the language, else the one before
constexpr u32 kRecordCarMake = 0x29;
constexpr u32 kRecordCarModel = 0x39;
constexpr u32 kRecordRed = 0x49, kRecordGreen = 0x4a, kRecordBlue = 0x4b;
constexpr u32 kRecordFlags = 0x50;                // byte
constexpr std::uint8_t kFlagIcon = 0x10;            // an icon instead of its colour
constexpr u32 kIconsDrawn = 0x55e5d0;
constexpr u32 kAllReady = 0x55e5b4;
constexpr u32 kLightOffsets = 0x55e5d4;           // int16: a ready light's place on its row
// The table.
constexpr u32 kFont = 0x12;
constexpr u32 kHeadColour = 0xff649dfd, kRowColour = 0xffffff;
constexpr u32 kAlignLeft = 0, kAlignCentre = 2;
constexpr u32 kHeadName = 0xc3, kHeadCar = 2, kHeadColour2 = 0x24, kHeadGears = 0x25;
constexpr u32 kColumnName = 2, kColumnCar = 0x60, kColumnColourHead = 0xef, kColumnGearsHead = 0x109;
constexpr u32 kColumnSwatch = 0xe9, kColumnSwatchEnd = 0xf8, kColumnGears = 0xff;
constexpr std::int32_t kRowsFromTop = 0x12, kRowHeight = 16, kSwatchBottom = 0xd;
constexpr u32 kSwatchWidth = 0xf, kSwatchHeight = 0xb, kSwatchFrame = 0xff488cfd;
constexpr u32 kNameWidth = 0x5f;
constexpr u32 kTextOpenSlot = 0x27, kTextGears = 0xe1;
constexpr u32 kComplaintText = 0xd;
constexpr GuestString kLobbyStrings[] = {
    { 0x53c57c, "..." },
    { 0x53c580, " " },
};


void lobbyText(Guest& guest, nfs::ThreadState& ts, u32 text, u32 x, std::int32_t y, u32 align, u32 colour)
{
    fe::direct<0x452190, port::text::menuTextDrawn>(guest.app(), ts, ts.sp - 4 - 4 * 2, text, std::int32_t(x), y,
                                                    kFont, std::int32_t(align), colour);
}

/* Line `number` of the language's text (edx, which only a name past the lines reads, as it is). */
u32 languageText(Guest& guest, nfs::ThreadState& ts, u32 number)
{
    return u32(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), ts, ts.sp - 4, std::int32_t(number),
                                                             0));
}

/* A row whose player's details have come. */
void drawPlayerRow(Guest& guest, nfs::ThreadState& ts, u32 x, std::int32_t slot, std::int32_t rowY)
{
    const u32 record = kPlayerRecords + kPlayerRecordSize * u32(slot);
    fe::enterLock(guest, ts, guest.dword(kLobbyLock));
    const u32 swatchX = x + kColumnSwatch;
    const u32 swatchY = u32(rowY + 2);
    if ((guest.byte(record + kRecordFlags) & kFlagIcon) != 0)
    {
        guest::direct<0x440b30, frontend::menua::drawPlayerIcon>(guest.app(), ts, swatchX, swatchY);
        guest.setDword(kIconsDrawn, guest.dword(kIconsDrawn) + 1);
    }
    else
    {
        const u32 colour = 0xff000000u | (u32(guest.byte(record + kRecordRed)) << 16)
            | (u32(guest.byte(record + kRecordGreen)) << 8) | guest.byte(record + kRecordBlue);
        fe::direct<0x4d8550, port::draw2d::gradient>(guest.app(), ts, ts.sp - 4 - 4 * 2, std::int32_t(swatchX),
                                                     std::int32_t(swatchY), std::int32_t(x + kColumnSwatchEnd),
                                                     rowY + kSwatchBottom, colour, colour);
        fe::direct<0x4d8930, port::draw2d::outline>(guest.app(), ts, ts.sp - 4 - 4, std::int32_t(swatchX),
                                                    std::int32_t(swatchY), std::int32_t(kSwatchWidth),
                                                    std::int32_t(kSwatchHeight), kSwatchFrame);
    }
    fe::direct<0x452120, frontend::menub::textDrawFitted>(guest.app(), ts, record + kRecordName, x + kColumnName,
                                                          u32(rowY), kFont, kAlignLeft, kRowColour, kNameWidth);
    const u32 gears = kTextGears + (guest.byte(record + kRecordGears) == 0 ? 1 : 0);
    lobbyText(guest, ts, languageText(guest, ts, gears), x + kColumnGears, rowY, kAlignLeft, kRowColour);

    // Its car, make and model, in the frame's buffer.
    const u32 car = guest.frame();
    guest.copyString(car, record + kRecordCarMake);
    if (guest.byte(car) != 0)
        guest.copyString(car + guest.length(car), kLobbyStrings[1].address);
    guest.copyString(car + guest.length(car), record + kRecordCarModel);
    lobbyText(guest, ts, car, x + kColumnCar, rowY, kAlignLeft, kRowColour);
    fe::leaveLock(guest, ts, guest.dword(kLobbyLock));

    // The host, at the top, has no light to show.
    if (slot != 0)
    {
        const std::int32_t lightOffset = int16At(guest, kLightOffsets + 2 * u32(slot));
        guest::direct<0x440b50, frontend::menua::drawPulsingIcon>(guest.app(), ts, u32(guest.byte(record + kRecordReady)),
                                                                  u32(slot), x + u32(lightOffset), swatchY);
    }
}

}  // namespace

nfs::u32 frontend::menud::drawLobbyPlayers(App* app, ThreadState& ts, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x128);
    /* sub_4a6690 keeps edx, 0 here, and that is what the game goes on to ask
     * about, so the message at the end never shows. */
        fe::direct<kPlayerMachine, ea::machineOfPlayerNumber>(app, ts, u32(int16At(guest, kMyPlayer)));
    const std::uint16_t complaint = 0;  // edx as sub_4a6690 keeps it: the 0 it is handed
    const std::int32_t slots = guest.dword(kNetworkKind) == 0 ? 2 : 8;

    const std::int32_t headY = std::int32_t(y + 2);
    lobbyText(guest, ts, languageText(guest, ts, kHeadName), x + kColumnName, headY, kAlignLeft, kHeadColour);
    lobbyText(guest, ts, languageText(guest, ts, kHeadCar), x + kColumnCar, headY, kAlignLeft, kHeadColour);
    lobbyText(guest, ts, languageText(guest, ts, kHeadColour2), x + kColumnColourHead, headY, kAlignCentre,
              kHeadColour);
    lobbyText(guest, ts, languageText(guest, ts, kHeadGears), x + kColumnGearsHead, headY, kAlignLeft,
              kHeadColour);
    guest.setDword(kIconsDrawn, 0);

    for (std::int32_t slot = 0; slot < slots; ++slot)
    {
        const std::int32_t rowY = std::int16_t(y + kRowsFromTop + kRowHeight * slot);
        const u32 machinePlayer = kMachinePlayers + 2 * u32(slot);
        const u32 recordSlot = kPlayerRecords + kPlayerRecordSize * u32(slot) + kRecordSlot;
        if (std::int16_t(guest.word(machinePlayer)) == -1)
        {
            // Open: a player who has gone is forgotten, under the lock.
            if (u32(slot) == guest.dword(recordSlot))
            {
                fe::enterLock(guest, ts, guest.dword(kLobbyLock));
                if (std::int16_t(guest.word(machinePlayer)) == -1)
                    guest.setDword(recordSlot, 0);
                fe::leaveLock(guest, ts, guest.dword(kLobbyLock));
            }
            lobbyText(guest, ts, languageText(guest, ts, kTextOpenSlot), x + kColumnName, rowY + 1, kAlignLeft,
                      kRowColour);
        }
        else if (u32(slot) != guest.dword(recordSlot))
        {
            // Joined, its details still on their way.
            lobbyText(guest, ts, kLobbyStrings[0].address, x + kColumnName, rowY, kAlignLeft, kRowColour);
        }
        else
        {
            drawPlayerRow(guest, ts, x, slot, rowY);
        }
    }

    /* With others in the game it asks each one's machine, and does nothing with
     * the answer: whether all are ready, kept at 0, is a check that lost its end. */
    guest.setDword(kAllReady, 0);
    std::int32_t last = slots;
    if (std::int16_t(guest.word(kPlayerCount)) >= 2)
    {
        std::int32_t player = 0;
        for (; player < std::int16_t(guest.word(kPlayerCount)); ++player)
                        fe::direct<kPlayerMachine, ea::machineOfPlayerNumber>(app, ts, u32(player));
        last = player;
    }
    if (complaint != 0)
    {
        return fe::direct<0x444be0, frontend::dialogs::messageById>(app, ts, kComplaintText);
    }
    return u32(last);
}

/* ---------------------------------------------------------------------------
 * A pause's end
 * ------------------------------------------------------------------------- */

/* sub_4bc670: what the race's view pass does as a pause ends. */
nfs::u32 frontend::menud::endPause(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x44be20, menub::menuLeave>(app, ts, 0);  // the menu no longer up, its dialog closed
    return guest::direct<0x4bc530, menuf::resetRaceMouse>(app, ts);
}

nfs::u32 frontend::menuPointerLoadArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c140", 0, 0, kPointerStrings, std::size(kPointerStrings),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::loadPointerArt(app, ts); });
}

nfs::u32 frontend::menuPointerFreeArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c2a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::freePointerArt(app, ts, eax); });
}

nfs::u32 frontend::menuPointerListLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c1c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const std::int32_t top = std::int32_t(args.eax), lineHeight = std::int32_t(args.edx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::pointerListLine(app, ts, top, lineHeight); });
}

nfs::u32 frontend::menuPointerOverRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c240", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const u32 x = args.eax, y = args.edx, width = args.ebx, height = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::pointerOverRect(app, ts, x, y, width, height); });
}

nfs::u32 frontend::effectsBandDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a47c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::drawBand(app, ts); });
}

nfs::u32 frontend::effectsScreenDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a49c0", 0, 0, kEffectScreens, std::size(kEffectScreens),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::drawScreenEffect(app, ts); });
}

nfs::u32 frontend::effectsLogoTextureStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4390", 0, 0, kLogoStrings, std::size(kLogoStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::startLogoTexture(app, ts); });
}

nfs::u32 frontend::effectsLoadScreenArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4ba0", 0, 0, kArtStrings, std::size(kArtStrings),
                          fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::loadLogoPictures(app, ts); });
}

nfs::u32 frontend::effectsLogoDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4c20", 0, 0, &kArtStrings[1], 1,
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 item = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::drawLogo(app, ts, item, x, y); });
}

nfs::u32 frontend::lobbyDrawPlayers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ae30", 0, 0, kLobbyStrings, std::size(kLobbyStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::drawLobbyPlayers(app, ts, x, y); });
}

nfs::u32 frontend::pauseEndRestorePointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc670", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menud::endPause(app, ts); });
}

}
