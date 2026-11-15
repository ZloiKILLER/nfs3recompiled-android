#include "native_frontend_menud.h"
#include "native_watcom.h"
#include "native_frontend.h"
#include "native_frontend_menua.h"
#include "native_frontend_menue.h"
#include "native_frontend_menuf.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_draw.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_loop.h"
#include "native_frontend_menub.h"
#include "native_graphics.h"
#include "native_windows.h"
#include "host_threads.h"
#include "port_buttons.h"
#include "port_draw2d.h"
#include "port_shapes.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <lib/winapp.h>
#include <winapi/types.h>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>

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
 * addresses it did), and an adapter under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using x86::reg32;

// The game's, shared.
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kStrcmp = 0x4ee310;
constexpr reg32 kArtFolder = 0x7a2db4;

std::int32_t int16At(const Guest& guest, reg32 address)
{
    return std::int16_t(guest.word(address));
}

/* ---------------------------------------------------------------------------
 * The pointer
 * ------------------------------------------------------------------------- */

constexpr reg32 kPointerArt = 0x55e5fc;           // mouse.fsh while it is loaded, or 0
constexpr reg32 kPointerPath = 0x79fbe4;
constexpr reg32 kPointerX = 0x55e5e4;
constexpr reg32 kPointerY = 0x55e5e8;
constexpr reg32 kPointerHotX = 0x563d3c;
constexpr reg32 kPointerHotY = 0x563d40;
constexpr std::uint32_t kLoadHigh = 0x10;
constexpr GuestString kPointerStrings[] = { { 0x53c5e4, "%smouse.fsh" } };

}  // namespace

/* sub_49c140, as the menus start: mouse.fsh made the pointer's textures, loaded
 * unless it already is, then the file let go. */
x86::reg32 frontend::menud::loadPointerArt(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (guest.dword(kPointerArt) == 0)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), kPointerPath, kPointerStrings[0].address, guest::Words{kArtFolder});
        guest.setDword(kPointerArt, *fe::direct<0x4e0ec0, ea::loadWhole>(app, cpu, kPointerPath, kLoadHigh));
    }
    guest::direct<0x4daa10, menue::loadPointer>(app, cpu, guest.dword(kPointerArt));
    const reg32 freed = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(kPointerArt));
    guest.setDword(kPointerArt, 0);
    guest.setDword(kPointerHotX, 0);
    guest.setDword(kPointerHotY, 0);
    return freed;
}

/* sub_49c2a0, as the menus end: the file let go, if it is still loaded. */
x86::reg32 frontend::menud::freePointerArt(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 art = guest.dword(kPointerArt);
    reg32 result = guest.eax();
    if (art != 0)
    {
        result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, art);
        guest.setDword(kPointerArt, 0);
    }
    return result;
}

/* sub_49c1c0: the line of a list under the pointer, the list's top at eax, lines
 * of edx each; -1 when it stands above the top.  edx left as the division leaves
 * it, the pointer's y with no division. */
x86::reg32 frontend::menud::pointerListLine(App* app, CPU& cpu, std::int32_t top, std::int32_t lineHeight)
{
    Guest guest(app, cpu);
    const std::int32_t pointerY = std::int32_t(guest.dword(kPointerY));
    if (top > pointerY)
    {
        guest.leaveEdx(reg32(pointerY));
        return reg32(-1);
    }
    const std::int32_t below = std::int32_t(reg32(pointerY) - reg32(top) - 1u);
    guest.leaveEdx(reg32(below % lineHeight));
    return reg32(below / lineHeight);
}

/* sub_49c240: whether the pointer is inside the rectangle at eax, edx, ebx wide
 * and ecx high, none of its edges counted.  edx left at what it last looked at. */
x86::reg32 frontend::menud::pointerOverRect(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    const std::int32_t pointerY = std::int32_t(guest.dword(kPointerY));
    guest.leaveEdx(y);
    if (std::int32_t(y) >= pointerY)
        return 0;
    const reg32 bottom = y + height;
    guest.leaveEdx(bottom);
    if (std::int32_t(bottom) <= pointerY)
        return 0;
    const std::int32_t pointerX = std::int32_t(guest.dword(kPointerX));
    guest.leaveEdx(reg32(pointerX));
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
constexpr reg32 kBandClosing = 0x55eb10;            // nonzero: the band shrinks
constexpr reg32 kBandBottom = 0x55eb0c;
constexpr reg32 kBandLight = 0x55eb14;              // the row the moving light is on
constexpr std::int32_t kBandShortest = 0x60;
constexpr std::int32_t kBandTallest = 0x1df;
constexpr std::int32_t kBandStep = 0x14;
constexpr std::int32_t kBandLightCycle = 0x12c;
constexpr reg32 kLitColour = 0xff70b0f0;
constexpr reg32 kDimColour = 0xff4060a0;
constexpr std::int32_t kBandRows = 0x26;
constexpr std::int32_t kBandMiddleRow = 0x13;
constexpr std::int32_t kRowGap = 10;
constexpr std::int32_t kBandFirstY = 0x63;
constexpr std::int32_t kBandRowFits = 0x83;         // a row's y from the first, past the band's bottom it shows
// A row: two lines, 2 pixels high, a pixel apart.  The first rows' ends are the
// game's table, int16 each; the rest reach the screen's edge.
constexpr reg32 kUpperLineLeft = 0x55e988, kLowerLineLeft = 0x55e98a;
constexpr reg32 kUpperLineRight = 0x55ea20, kLowerLineRight = 0x55ea22;
constexpr std::int32_t kLastRowWithEnds = 3;
constexpr std::int32_t kScreenRight = 0x27f;
// The short lines at the top, fading to the right, where the light ends its way.
constexpr std::int32_t kTopRows = 0x12;
constexpr std::int32_t kTopFirstY = 7;
constexpr reg32 kTopUpperRight = 0x55ea30, kTopLowerRight = 0x55ea32;
constexpr std::int32_t kTopLightStart = 0xc8;
constexpr reg32 kTopFade = 0x20;

/* A line of the band, two pixels high: its ends `endColour`, its middle `midColour`; the drawing's eax. */
reg32 drawLine(Guest& guest, x86::CPU& cpu, std::int32_t left, std::int32_t top, std::int32_t right, reg32 endColour,
               reg32 midColour)
{
    const reg32 colours[4] = { endColour, midColour, midColour, endColour };
    return reg32(*fe::direct<0x4d88f0, port::buttons::colouredRectangle>(
        guest.app(), cpu, cpu.esp - 4 - 4 * 4, port::Args{ reg32(left), reg32(top), reg32(right), reg32(top + 2) },
        colours));
}

}  // namespace

x86::reg32 frontend::menud::drawBand(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    // The band grows or shrinks to where it is going.
    const std::int32_t bandBottom = std::int32_t(guest.dword(kBandBottom));
    if (guest.dword(kBandClosing) != 0)
    {
        if (bandBottom > kBandShortest)
            guest.setDword(kBandBottom, reg32(bandBottom - kBandStep));
    }
    else
    {
        if (bandBottom < kBandTallest)
            guest.setDword(kBandBottom, reg32(bandBottom + kBandStep));
        if (std::int32_t(guest.dword(kBandBottom)) > kBandTallest)
            guest.setDword(kBandBottom, reg32(kBandTallest));
    }
    // The light moves a row a pass, and starts again.
    const reg32 light = guest.dword(kBandLight);
    guest.setDword(kBandLight, light != 0 ? light - 1 : reg32(kBandLightCycle));

    // The rows, brightest in the middle, as far down as the band is.
    for (std::int32_t row = 0; row < kBandRows; ++row)
    {
        const std::int32_t fromEnd = row <= kBandMiddleRow ? row : kBandRows - row;
        const std::int32_t brightness = fromEnd * 224 / kBandMiddleRow + 0x1f;
        const reg32 tint = reg32(row) == guest.dword(kBandLight) ? kLitColour : kDimColour;
        const reg32 colour = guest::direct<0x4d9180, menue::scaleColour>(app, cpu, tint, brightness);
        if (row * kRowGap + kBandRowFits <= std::int32_t(guest.dword(kBandBottom)))
            continue;
        const std::int32_t y = row * kRowGap + kBandFirstY;
        const reg32 slot = 4 * reg32(row);
        const bool toEdge = row > kLastRowWithEnds;
        drawLine(guest, cpu, int16At(guest, kUpperLineLeft + slot), y,
                 toEdge ? kScreenRight : int16At(guest, kUpperLineRight + slot), colour, colour);
        drawLine(guest, cpu, int16At(guest, kLowerLineLeft + slot), y + 3,
                 toEdge ? kScreenRight : int16At(guest, kLowerLineRight + slot), colour, colour);
    }

    reg32 drawn = 0;
    for (std::int32_t row = 0; row < kTopRows; ++row)
    {
        const reg32 tint = std::int32_t(guest.dword(kBandLight)) - kTopLightStart == row ? kLitColour : kDimColour;
        const reg32 faded = guest::direct<0x4d9180, menue::scaleColour>(app, cpu, tint, std::int32_t(kTopFade));
        const std::int32_t y = row * kRowGap + kTopFirstY;
        const reg32 slot = 4 * reg32(row);
        drawLine(guest, cpu, 0, y, int16At(guest, kTopUpperRight + slot), tint, faded);
        drawn = drawLine(guest, cpu, 0, y + 3, int16At(guest, kTopLowerRight + slot), tint, faded);
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

x86::reg32 frontend::menud::drawScreenEffect(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, cpu);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), name, kEffectScreens[0].address) == 0)
        fe::direct<0x453940, menub::carStatsDraw>(app, cpu);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), name, kEffectScreens[1].address) == 0)
        fe::direct<0x454780, menub::carListDraw>(app, cpu);
    reg32 result = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), name, kEffectScreens[2].address);
    if (result == 0)
        result = fe::direct<0x453f60, menub::carSpecsDraw>(app, cpu);
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
constexpr reg32 kFindShapeInFile = 0x4ef270;
constexpr reg32 kDisplayFlags = 0x7a3a58;           // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr reg32 kLogoTexture = 0x7a1b94;
constexpr reg32 kLogoTextureSize = 0x100;
constexpr reg32 kLogoTextureModeNoTranslucency = 3;

}  // namespace

x86::reg32 frontend::menud::startLogoTexture(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x100);
    const reg32 path = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(app, cpu, path, kLogoStrings[0].address,
                                                guest::Words{kArtFolder, kLogoStrings[1].address});
    guest.leaveEdx(0);  // what loadQfs hands on in edx to the file's loader: its flags
    const reg32 file = fe::direct<0x4956f0, draw::loadQfs>(app, cpu, path);
    const reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, kLogoStrings[2].address);
    const reg32 mode = (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? kLogoTextureModeNoTranslucency : 0;
    fe::direct<0x4d6070, port::shapes::textureOf>(app, cpu, cpu.esp - 4, kLogoTexture, shape,
                                                  std::int32_t(kLogoTextureSize), mode);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, file);
}

namespace
{

constexpr reg32 kLogoPictureNames = 0x4a4050;       // the four shapes' names, kept in the code's segment
constexpr reg32 kGlintShape = 0x8b7634;
constexpr reg32 kLogoPictures = 0x55eb18;
constexpr std::int32_t kLogoPictureCount = 4;
constexpr reg32 kMenuShapeFile = 0x559228;

}  // namespace

x86::reg32 frontend::menud::loadLogoPictures(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x10);
    const reg32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, cpu);
    guest.setDword(kGlintShape, reg32(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, cpu, cpu.esp - 4,
                                                                                       kGlintName.address)));
    // Track info shows the first picture only.
    std::int32_t pictures = kLogoPictureCount;
    if (name != 0 && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), name, kTrackInfoScreen.address) == 0)
        pictures = 1;
    reg32 record = 0;
    for (std::int32_t i = 0; i < pictures; ++i)
    {
        const reg32 slot = 4 * reg32(i);
        const reg32 shape = guest.call(kFindShapeInFile,
                                       args({ guest.dword(kMenuShapeFile), guest.dword(kLogoPictureNames + slot) }));
        // ecx as the call before left it, handed on.
        record = reg32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, cpu, cpu.esp - 4, shape, 0, cpu.ecx));
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
constexpr reg32 kItemShape = 0x44;                  // int16
constexpr reg32 kItemClock = 0x46;                  // int16
constexpr std::int16_t kClockTurn = 0x400;
constexpr std::int16_t kClockStep = 2;
constexpr reg32 kGlintCountdown = 0x55eb28;
constexpr reg32 kGlintLeft = 0x7a1b98;              // where it stands as it comes in, else 0
constexpr std::int32_t kGlintCycle = 0x12c;
constexpr std::int32_t kInEnd = 0x34;               // its time as it comes in, then grows, then shrinks
constexpr std::int32_t kGrowEnd = 0x52;
constexpr std::int32_t kChangeLength = 0x1e;
constexpr std::int32_t kFlashAt = 0xf;
constexpr reg32 kGlintComingY = 0x55ea78;           // int16 for each time, as it comes in
constexpr reg32 kBigGlintKey = 0x2d;
constexpr float kBigGlint = 75.0f, kSmallGlint = 8.0f;
constexpr reg32 kSetRenderState = 0x431900;         // eax the state, edx its value
constexpr reg32 kAdditiveBlend = 0x68;
constexpr reg32 kDrawTurnedShape = 0x4d6730;        // shape, x, y, nudged left by ecx; the angle, the scale
constexpr reg32 kDrawArtShape = 0x4d7160;           // shape, x, y, the colour
constexpr reg32 kGlintGrowX = 0x46, kGlintShrinkX = 0x47, kGlintY = 0x3c;
constexpr reg32 kFlashX = 0x47, kFlashY = 0x39, kFlashNudge = 0x1e;
constexpr std::int32_t kOtherFlashAngle = 0xb4;
constexpr std::int32_t kGrowFlashAngle = 0x1e, kShrinkFlashAngle = 0x37;
// The game's constants, doubles.
constexpr reg32 kComingScale = 0x53cfe4, kOneHalf = 0x53cfec, kThirtieth = 0x53cffc;
constexpr reg32 kGrowBias = 0x53cff4, kFive = 0x53d004, kShrinkBias = 0x53d024;
constexpr reg32 kFlashScale = 0x53d00c, kFifteenth = 0x53d014, kFlashLeast = 0x53d01c;

double gameDouble(const Guest& guest, reg32 address)
{
    const std::uint32_t words[2] = { guest.dword(address), guest.dword(address + 4) };
    double value;
    std::memcpy(&value, words, sizeof value);
    return value;
}

reg32 floatBits(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

struct Logo
{
    Guest& guest;
    x86::CPU& cpu;
    reg32 x, y;
    reg32 picture, nextPicture;
    reg32 pictureColour, nextPictureColour;

    void blend(bool additive)
    {
        fe::direct<0x431900, graphics::sendState>(guest.app(), cpu, kAdditiveBlend, additive ? 1u : 0u);
    }
    void glint(reg32 atX, reg32 atY, reg32 nudge, std::int32_t angle, float scale)
    {
        fe::direct<0x4d6730, port::buttons::turnedShape>(
            guest.app(), cpu, cpu.esp - 4 - 4 * 2, std::int32_t(guest.dword(kGlintShape)), std::int32_t(atX),
            std::int32_t(atY), std::int32_t(nudge), angle, floatBits(scale));
    }

    /* The function's last calls, by address: what they leave in ebx and edx, the function leaves. */
    void drawPictures()
    {
        guest.call(kDrawArtShape, args({ picture, x, y, pictureColour }));
        if (nextPicture != 0)
            guest.call(kDrawArtShape, args({ nextPicture, x, y, nextPictureColour }));
    }
    void flash(std::int32_t angle, float scale)
    {
        const auto glintByAddress = [&](std::int32_t turn) {
            guest.call(kDrawTurnedShape, args({ guest.dword(kGlintShape), kFlashX, kFlashY, kFlashNudge },
                                              { reg32(turn), floatBits(scale) }));
        };
        blend(true);
        glintByAddress(angle);
        glintByAddress(angle + kOtherFlashAngle);
        guest.call(kSetRenderState, args({ kAdditiveBlend, 0 }));
    }
};

}  // namespace

x86::reg32 frontend::menud::drawLogo(App* app, CPU& cpu, reg32 item, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x2c);
    const reg32 name = fe::direct<0x44ae00, loop::currentMenuName>(app, cpu);

    // The clock goes round.
    const std::uint16_t advanced = std::uint16_t(guest.word(item + kItemClock) + kClockStep);
    guest.setWord(item + kItemClock, std::int16_t(advanced) >= kClockTurn ? std::uint16_t(0) : advanced);
    const std::int32_t clock = int16At(guest, item + kItemClock);

    // The two pictures it is between, and how far: each as opaque as the other is not.
    const std::int32_t pictureNumber = clock / 256;
    const std::int32_t between = clock % 256;
    Logo logo{ guest, cpu, x, y, 0, 0, 0, 0 };
    logo.picture = guest.dword(kLogoPictures + 4 * reg32(pictureNumber));
    guest.setWord(item + kItemShape, std::uint16_t(logo.picture));
    logo.nextPicture = guest.dword(kLogoPictures + 4 * reg32((pictureNumber + 1) % kLogoPictureCount));
    logo.pictureColour = (reg32(0xff - between) << 24) | 0xffffff;
    logo.nextPictureColour = (reg32(between) << 24) | 0xffffff;

    // Track info holds the first picture.
    if (name != 0 && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), name, kTrackInfoScreen.address) == 0)
    {
        logo.picture = guest.dword(kLogoPictures);
        logo.nextPicture = 0;
        logo.pictureColour = 0xffffffff;
        guest.setWord(item + kItemShape, std::uint16_t(logo.picture));
    }

    const double glintSize =
        fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(app, cpu, kBigGlintKey) != 0 ? kBigGlint : kSmallGlint;

    // The glint's own clock.
    const reg32 countdown = guest.dword(kGlintCountdown);
    guest.setDword(kGlintCountdown, countdown != 0 ? countdown - 1 : reg32(kGlintCycle));
    const std::int32_t time = 2 * (kGlintCycle - std::int32_t(guest.dword(kGlintCountdown)));
    const std::int32_t angle = 10 * time;
    guest.setDword(kGlintLeft, 0);

    if (time < kInEnd)
    {
        const double scaled = fp::mul(x86::Float(time), gameDouble(guest, kComingScale));
        guest.setDword(kGlintLeft, reg32(640 - angle));
        const float scale = float(fp::add(scaled, gameDouble(guest, kOneHalf)));
        logo.blend(true);
        logo.glint(guest.dword(kGlintLeft), reg32(int16At(guest, kGlintComingY + 2 * reg32(time))), 0, angle, scale);
        logo.blend(false);
        logo.drawPictures();
    }
    else if (time - kInEnd < kChangeLength)
    {
        const std::int32_t step = time - kInEnd;
        const double biased = fp::add(glintSize, gameDouble(guest, kGrowBias));
        const double grown = fp::mul(fp::mul(biased, x86::Float(step)), gameDouble(guest, kThirtieth));
        const float scale = float(fp::add(grown, gameDouble(guest, kFive)));
        logo.blend(true);
        logo.glint(kGlintGrowX, kGlintY, 0, angle, scale);
        logo.blend(false);
        logo.drawPictures();
        if (step > kFlashAt)
        {
            const std::int32_t past = step - kFlashAt;
            const double flashed = fp::mul(fp::mul(x86::Float(past), gameDouble(guest, kFlashScale)),
                                           gameDouble(guest, kFifteenth));
            logo.flash(past * 25 / kFlashAt + kGrowFlashAngle, float(fp::add(flashed, gameDouble(guest, kFlashLeast))));
        }
    }
    else if (time - kGrowEnd < kChangeLength)
    {
        const std::int32_t step = time - kGrowEnd;
        const double biased = fp::add(glintSize, gameDouble(guest, kShrinkBias));
        const double shrunk = fp::mul(fp::mul(biased, x86::Float(step)), gameDouble(guest, kThirtieth));
        const float scale = float(fp::sub(glintSize, shrunk));
        logo.blend(true);
        logo.glint(kGlintShrinkX, kGlintY, 0, angle, scale);
        logo.blend(false);
        logo.drawPictures();
        if (step < kFlashAt)
        {
            const double flashed = fp::mul(fp::mul(x86::Float(step), gameDouble(guest, kFlashScale)),
                                           gameDouble(guest, kFifteenth));
            logo.flash(step * 25 / kFlashAt + kShrinkFlashAngle, float(fp::sub(1.0, flashed)));
        }
    }
    else
    {
        logo.drawPictures();
    }
    return guest.eax();
}

namespace
{

/* ---------------------------------------------------------------------------
 * The network screen's table of players (sub_49ae30)
 * ------------------------------------------------------------------------- */

constexpr reg32 kMyPlayer = 0x7a2278;               // int16
constexpr reg32 kPlayerCount = 0x7a2276;            // int16
constexpr reg32 kNetworkKind = 0x7a22e4;            // 0 two machines, else up to eight
constexpr reg32 kMachinePlayers = 0x7a227a;         // int16 each: a slot's player, -1 none
constexpr reg32 kLobbyLock = 0x55ecb4;
constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588;   // the import table's
constexpr reg32 kPlayerMachine = 0x4a6690;
// A player's record, as they all see it.
constexpr reg32 kPlayerRecords = 0x79f5f0;
constexpr reg32 kPlayerRecordSize = 0x60;
constexpr reg32 kRecordSlot = 0x04;
constexpr reg32 kRecordReady = 0x08;                // byte
constexpr reg32 kRecordName = 0x09;
constexpr reg32 kRecordGears = 0x15;                // byte: 0 one line of the language, else the one before
constexpr reg32 kRecordCarMake = 0x29;
constexpr reg32 kRecordCarModel = 0x39;
constexpr reg32 kRecordRed = 0x49, kRecordGreen = 0x4a, kRecordBlue = 0x4b;
constexpr reg32 kRecordFlags = 0x50;                // byte
constexpr std::uint8_t kFlagIcon = 0x10;            // an icon instead of its colour
constexpr reg32 kIconsDrawn = 0x55e5d0;
constexpr reg32 kAllReady = 0x55e5b4;
constexpr reg32 kLightOffsets = 0x55e5d4;           // int16: a ready light's place on its row
// The table.
constexpr reg32 kFont = 0x12;
constexpr reg32 kHeadColour = 0xff649dfd, kRowColour = 0xffffff;
constexpr reg32 kAlignLeft = 0, kAlignCentre = 2;
constexpr reg32 kHeadName = 0xc3, kHeadCar = 2, kHeadColour2 = 0x24, kHeadGears = 0x25;
constexpr reg32 kColumnName = 2, kColumnCar = 0x60, kColumnColourHead = 0xef, kColumnGearsHead = 0x109;
constexpr reg32 kColumnSwatch = 0xe9, kColumnSwatchEnd = 0xf8, kColumnGears = 0xff;
constexpr std::int32_t kRowsFromTop = 0x12, kRowHeight = 16, kSwatchBottom = 0xd;
constexpr reg32 kSwatchWidth = 0xf, kSwatchHeight = 0xb, kSwatchFrame = 0xff488cfd;
constexpr reg32 kNameWidth = 0x5f;
constexpr reg32 kTextOpenSlot = 0x27, kTextGears = 0xe1;
constexpr reg32 kComplaintText = 0xd;
constexpr GuestString kLobbyStrings[] = {
    { 0x53c57c, "..." },
    { 0x53c580, " " },
};

/* The lobby's lock entered or left: Windows' function the port's own, called as C++
 * (native_windows.h); in the check through its import's slot.  Terminated when the game stopped
 * in it, as Guest::call. */
template <reg32 Slot, auto Function>
void lobbyLock(Guest& guest, x86::CPU& cpu)
{
    if (!guest::windows<Slot, Function>(guest.app(), cpu, guest.dword(kLobbyLock)))
        throw fe::Terminated();
}

void lobbyText(Guest& guest, x86::CPU& cpu, reg32 text, reg32 x, std::int32_t y, reg32 align, reg32 colour)
{
    fe::direct<0x452190, port::text::menuTextDrawn>(guest.app(), cpu, cpu.esp - 4 - 4 * 2, text, std::int32_t(x), y,
                                                    kFont, std::int32_t(align), colour);
}

/* Line `number` of the language's text (edx, which only a name past the lines reads, as it is). */
reg32 languageText(Guest& guest, x86::CPU& cpu, reg32 number)
{
    return reg32(*fe::direct<0x4d1850, port::text::lineText>(guest.app(), cpu, cpu.esp - 4, std::int32_t(number),
                                                             cpu.edx));
}

/* A row whose player's details have come. */
void drawPlayerRow(Guest& guest, x86::CPU& cpu, reg32 x, std::int32_t slot, std::int32_t rowY)
{
    const reg32 record = kPlayerRecords + kPlayerRecordSize * reg32(slot);
    lobbyLock<kEnterLock, &host::enterLock>(guest, cpu);
    const reg32 swatchX = x + kColumnSwatch;
    const reg32 swatchY = reg32(rowY + 2);
    if ((guest.byte(record + kRecordFlags) & kFlagIcon) != 0)
    {
        guest::direct<0x440b30, frontend::menua::drawPlayerIcon>(guest.app(), cpu, swatchX, swatchY);
        guest.setDword(kIconsDrawn, guest.dword(kIconsDrawn) + 1);
    }
    else
    {
        const reg32 colour = 0xff000000u | (reg32(guest.byte(record + kRecordRed)) << 16)
            | (reg32(guest.byte(record + kRecordGreen)) << 8) | guest.byte(record + kRecordBlue);
        fe::direct<0x4d8550, port::draw2d::gradient>(guest.app(), cpu, cpu.esp - 4 - 4 * 2, std::int32_t(swatchX),
                                                     std::int32_t(swatchY), std::int32_t(x + kColumnSwatchEnd),
                                                     rowY + kSwatchBottom, colour, colour);
        fe::direct<0x4d8930, port::draw2d::outline>(guest.app(), cpu, cpu.esp - 4 - 4, std::int32_t(swatchX),
                                                    std::int32_t(swatchY), std::int32_t(kSwatchWidth),
                                                    std::int32_t(kSwatchHeight), kSwatchFrame);
    }
    fe::direct<0x452120, frontend::menub::textDrawFitted>(guest.app(), cpu, record + kRecordName, x + kColumnName,
                                                          reg32(rowY), kFont, kAlignLeft, kRowColour, kNameWidth);
    const reg32 gears = kTextGears + (guest.byte(record + kRecordGears) == 0 ? 1 : 0);
    lobbyText(guest, cpu, languageText(guest, cpu, gears), x + kColumnGears, rowY, kAlignLeft, kRowColour);

    // Its car, make and model, in the frame's buffer.
    const reg32 car = guest.frame();
    guest.copyString(car, record + kRecordCarMake);
    if (guest.byte(car) != 0)
        guest.copyString(car + guest.length(car), kLobbyStrings[1].address);
    guest.copyString(car + guest.length(car), record + kRecordCarModel);
    lobbyText(guest, cpu, car, x + kColumnCar, rowY, kAlignLeft, kRowColour);
    lobbyLock<kLeaveLock, &host::leaveLock>(guest, cpu);

    // The host, at the top, has no light to show.
    if (slot != 0)
    {
        const std::int32_t lightOffset = int16At(guest, kLightOffsets + 2 * reg32(slot));
        guest::direct<0x440b50, frontend::menua::drawPulsingIcon>(guest.app(), cpu, reg32(guest.byte(record + kRecordReady)),
                                                                  reg32(slot), x + reg32(lightOffset), swatchY);
    }
}

}  // namespace

x86::reg32 frontend::menud::drawLobbyPlayers(App* app, CPU& cpu, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x128);
    /* sub_4a6690 keeps edx, 0 here, and that is what the game goes on to ask
     * about, so the message at the end never shows. */
    guest.call(kPlayerMachine, args({ reg32(int16At(guest, kMyPlayer)), 0 }));
    const std::uint16_t complaint = std::uint16_t(guest.edx());
    const std::int32_t slots = guest.dword(kNetworkKind) == 0 ? 2 : 8;

    const std::int32_t headY = std::int32_t(y + 2);
    lobbyText(guest, cpu, languageText(guest, cpu, kHeadName), x + kColumnName, headY, kAlignLeft, kHeadColour);
    lobbyText(guest, cpu, languageText(guest, cpu, kHeadCar), x + kColumnCar, headY, kAlignLeft, kHeadColour);
    lobbyText(guest, cpu, languageText(guest, cpu, kHeadColour2), x + kColumnColourHead, headY, kAlignCentre,
              kHeadColour);
    lobbyText(guest, cpu, languageText(guest, cpu, kHeadGears), x + kColumnGearsHead, headY, kAlignLeft,
              kHeadColour);
    guest.setDword(kIconsDrawn, 0);

    for (std::int32_t slot = 0; slot < slots; ++slot)
    {
        const std::int32_t rowY = std::int16_t(y + kRowsFromTop + kRowHeight * slot);
        const reg32 machinePlayer = kMachinePlayers + 2 * reg32(slot);
        const reg32 recordSlot = kPlayerRecords + kPlayerRecordSize * reg32(slot) + kRecordSlot;
        if (std::int16_t(guest.word(machinePlayer)) == -1)
        {
            // Open: a player who has gone is forgotten, under the lock.
            if (reg32(slot) == guest.dword(recordSlot))
            {
                lobbyLock<kEnterLock, &host::enterLock>(guest, cpu);
                if (std::int16_t(guest.word(machinePlayer)) == -1)
                    guest.setDword(recordSlot, 0);
                lobbyLock<kLeaveLock, &host::leaveLock>(guest, cpu);
            }
            lobbyText(guest, cpu, languageText(guest, cpu, kTextOpenSlot), x + kColumnName, rowY + 1, kAlignLeft,
                      kRowColour);
        }
        else if (reg32(slot) != guest.dword(recordSlot))
        {
            // Joined, its details still on their way.
            lobbyText(guest, cpu, kLobbyStrings[0].address, x + kColumnName, rowY, kAlignLeft, kRowColour);
        }
        else
        {
            drawPlayerRow(guest, cpu, x, slot, rowY);
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
            guest.call(kPlayerMachine, args({ reg32(player), reg32(player) }));
        last = player;
    }
    guest.leaveEdx(reg32(last));
    if (complaint != 0)
    {
        fe::messageBox(guest, kComplaintText);
        return guest.eax();
    }
    return reg32(last);
}

/* ---------------------------------------------------------------------------
 * A pause's end
 * ------------------------------------------------------------------------- */

/* sub_4bc670: what the race's view pass does as a pause ends. */
x86::reg32 frontend::menud::endPause(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    fe::direct<0x44be20, menub::menuLeave>(app, cpu);  // the menu no longer up, its dialog closed
    return guest::direct<0x4bc530, menuf::resetRaceMouse>(app, cpu);
}

void frontend::menuPointerLoadArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c140", 0, 0, kPointerStrings, std::size(kPointerStrings),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::loadPointerArt(app, cpu); });
}

void frontend::menuPointerFreeArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c2a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::freePointerArt(app, cpu); });
}

void frontend::menuPointerListLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c1c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const std::int32_t top = std::int32_t(cpu.eax), lineHeight = std::int32_t(cpu.edx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::pointerListLine(app, cpu, top, lineHeight); });
}

void frontend::menuPointerOverRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c240", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const reg32 x = cpu.eax, y = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::pointerOverRect(app, cpu, x, y, width, height); });
}

void frontend::effectsBandDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a47c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::drawBand(app, cpu); });
}

void frontend::effectsScreenDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a49c0", 0, 0, kEffectScreens, std::size(kEffectScreens),
                          fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::drawScreenEffect(app, cpu); });
}

void frontend::effectsLogoTextureStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a4390", 0, 0, kLogoStrings, std::size(kLogoStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::startLogoTexture(app, cpu); });
}

void frontend::effectsLoadScreenArt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a4ba0", 0, 0, kArtStrings, std::size(kArtStrings),
                          fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::loadLogoPictures(app, cpu); });
}

void frontend::effectsLogoDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a4c20", 0, 0, &kArtStrings[1], 1,
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 item = cpu.eax, x = cpu.edx, y = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::drawLogo(app, cpu, item, x, y); });
}

void frontend::lobbyDrawPlayers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49ae30", 0, 0, kLobbyStrings, std::size(kLobbyStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::drawLobbyPlayers(app, cpu, x, y); });
}

void frontend::pauseEndRestorePointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc670", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return menud::endPause(app, cpu); });
}

}
