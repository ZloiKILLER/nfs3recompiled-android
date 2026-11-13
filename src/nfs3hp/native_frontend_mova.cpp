#include "native_frontend_mova.h"
#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstdint>

/* What is left of the movies' display that the native player (native_movie.cpp) still
 * calls, and the integer projection that sat beside it in the address map, decompiled:
 *
 *   the movie's display  its place and quality kept (sub_4df1f0, called whenever a
 *                        movie opens), the overlay surface let go (sub_4df310), the way
 *                        frames reach the screen (sub_4df340) and the two records of
 *                        the screen they read (sub_4eba80, sub_4eba90);
 *   the projection       the screen rectangle and the focal lengths of the game's older
 *                        integer projection (sub_4fd280, sub_4fd2b0, sub_4fd3f0).
 *
 * The player's generated body behind madPlay, the 16-bit surface path behind showFrame
 * and EA's stream and file libraries stay generated.
 *
 * Plan step 5: each function a typed core in frontend::mova (native_frontend_mova.h), in a frame
 * of its own where the game's function has one, and an adapter under the game's name for the calls
 * by address.  A core of this file calls another as C++ (guest::direct); the rest of the game
 * still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::reg32;
using x86::sreg32;

// What sub_4df1f0 keeps of the display.
constexpr reg32 kDisplayDevice = 0x8ca2c0;   // the screen's device the surface was made for
constexpr reg32 kDisplayHeight = 0x8ca2c4;
constexpr reg32 kDisplaySurface = 0x8ca2c8;  // the overlay surface, 0 for none
constexpr reg32 kDisplayQuality = 0x8ca2cc;
constexpr reg32 kDisplayWidth = 0x8ca2d0;
constexpr reg32 kDisplayMode = 0x8ca2d4;
constexpr reg32 kDisplayTicks = 0x8ca2d8;
constexpr reg32 kDisplayY = 0x8ca2dc;
constexpr reg32 kDisplayX = 0x8ca2e0;

// The ways a frame reaches the screen.
constexpr reg32 kModeOverlay = 0;    // the card's overlay
constexpr reg32 kModeStretched = 1;  // the card's stretching blit
constexpr reg32 kModeSoftware = 2;   // converted to the 16-bit surface

constexpr reg32 kScreenDevice = 0x9f3d3c;
constexpr reg32 kDirectDraw = 0x9f38d0;


// COM's methods, by their place in the object's table.
constexpr reg32 kRelease = 0x08;         // IUnknown::Release
constexpr reg32 kGetCaps = 0x2c;         // IDirectDraw2::GetCaps
constexpr reg32 kCapsSize = 0x16c;       // DDCAPS
constexpr reg32 kCapsFlagsByte = 5;      // dwCaps' second byte
constexpr std::uint8_t kCapsOverlay = 0x01, kCapsOverlayCantClip = 0x02;
constexpr std::uint8_t kCapsBlit = 0x20, kCapsBlitStretch = 0x40;

// The integer projection.
constexpr reg32 kHalfWidth = 0x564424, kHalfHeight = 0x564428;
constexpr reg32 kCentreX = 0x56442c, kCentreY = 0x564430;
constexpr reg32 kEdgeX = 0x564434, kEdgeY = 0x564438;
constexpr reg32 kFocalX = 0x56443c, kFocalY = 0x564440;
constexpr reg32 kAngleX = 0x564448, kAngleY = 0x56444c;
constexpr reg32 kAspect = 0x564450;      // 16.16: the y focal length from the x one's
constexpr reg32 kSineTable = 0x5672ac;   // a quarter turn, 0x100 steps, both ends
constexpr reg32 kAngleOf = 0x4e8f28;     // the angle of y over x

// A whole turn is 0x400 of the projection's angles.
constexpr reg32 kQuarterTurn = 0x100;
constexpr sreg32 kDegreesInTurn = 0x168;
constexpr unsigned kTurnBits = 10;

/* The game's sine: the quarter table reflected by the angle's bit 8, negated by its
 * bit 9.  The angle's higher bits are not looked at. */
reg32 tableSine(const Guest& guest, reg32 angle)
{
    const bool reflected = (angle & 0x100) != 0;
    const bool negative = (angle & 0x200) != 0;
    const reg32 step = reflected ? 0x100 - (angle & 0xff) : angle & 0xff;
    const reg32 value = guest.dword(kSineTable + 4 * step);
    return negative ? reg32(0) - value : value;
}

/* idiv of a sign-extended dividend, as the generated code does it. */
struct Division
{
    reg32 quotient;
    reg32 remainder;
};

Division divide(reg32 dividend, reg32 divisor)
{
    const std::int64_t wide = std::int64_t(sreg32(dividend));
    const std::int64_t by = std::int64_t(sreg32(divisor));
    return { reg32(wide / by), reg32(wide % by) };
}

/* A whole number of degrees as the projection's angle, cut towards zero. */
reg32 degreesToAngle(reg32 degrees)
{
    return reg32(sreg32(degrees << kTurnBits) / kDegreesInTurn);
}

}

/* sub_4eba80: the screen's device, as DirectDraw's library keeps it. */
x86::reg32 frontend::mova::screenDevice(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(kScreenDevice);
}

/* sub_4eba90: DirectDraw's object. */
x86::reg32 frontend::mova::directDraw(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(kDirectDraw);
}

/* sub_4df340: the way a frame reaches the screen, which the player looks at to know
 * whether it decodes the frames nothing is predicted from. */
x86::reg32 frontend::mova::displayMode(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(kDisplayMode);
}

/* sub_4df310: the overlay surface let go, when the screen's device is still the one it
 * was made for. */
x86::reg32 frontend::mova::displayClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 device = guest::direct<0x4eba80, screenDevice>(app, cpu);
    if (device != guest.dword(kDisplayDevice))
        return device;
    const reg32 surface = guest.dword(kDisplaySurface);
    if (surface == 0)
        return device;
    const reg32 table = guest.dword(surface);
    const reg32 result = guest.call(guest.dword(table + kRelease), args({}, { surface }));
    guest.setDword(kDisplaySurface, 0);
    return result;
}

/* sub_4df1f0: a movie's display opened: x in eax, y in edx, the frame's width in ebx and
 * height in ecx, its quality and "software" on the stack.  Software, as the game's only
 * caller always asks (sub_4960e0 pushes 1), keeps to the 16-bit surface; otherwise the
 * card's capabilities choose.  Eax, ebx, ecx and edx stay as the original leaves them. */
x86::reg32 frontend::mova::displayOpen(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height,
                                       reg32 quality, reg32 software)
{
    static constexpr reg32 kPushes = 12, kLocals = 0x2d8;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kPushes + kLocals);
    guest::direct<0x4df310, displayClose>(app, cpu);
    guest.setDword(kDisplayX, x);
    guest.setDword(kDisplayY, y);
    guest.setDword(kDisplayWidth, width);
    guest.setDword(kDisplayHeight, height);
    guest.setDword(kDisplayTicks, 0);
    guest.setDword(kDisplayDevice, 0);
    guest.setDword(kDisplayQuality, quality);
    guest.setDword(kDisplayMode, kModeSoftware);
    guest.leaveEdx(kModeSoftware);
    guest.leaveEbx(width);
    guest.leaveEcx(software);
    if (software != 0)
        return quality;

    const reg32 directDraw = guest::direct<0x4eba90, mova::directDraw>(app, cpu);
    const reg32 emulationCaps = guest.frame();
    const reg32 cardCaps = guest.frame() + kLocals - kCapsSize;
    guest.setDword(cardCaps, kCapsSize);
    guest.setDword(emulationCaps, kCapsSize);
    const reg32 table = guest.dword(directDraw);
    const reg32 failed = guest.call(guest.dword(table + kGetCaps),
                                    args({ directDraw, cardCaps, kCapsSize, table },
                                         { directDraw, cardCaps, emulationCaps }));
    if (failed != 0)
        return failed;
    const std::uint8_t caps = guest.byte(cardCaps + kCapsFlagsByte);
    // The byte read into ah, over the 0 GetCaps answered.
    const reg32 capsInAh = reg32(caps) << 8;
    if ((caps & kCapsBlit) != 0 && (caps & kCapsBlitStretch) != 0)
    {
        guest.setDword(kDisplayMode, kModeStretched);
        return capsInAh;
    }
    // And into dh, over what GetCaps left there.
    guest.leaveEdx((guest.edx() & 0xffff00ffu) | capsInAh);
    if ((caps & kCapsOverlay) != 0 && (caps & kCapsOverlayCantClip) != 0)
    {
        guest.setDword(kDisplayMode, kModeOverlay);
        return 0;
    }
    return capsInAh;
}

/* sub_4fd280: the projection's centre on the screen at eax, edx, and its far edges
 * from the half sizes it has.  Eax and edx stay as the original leaves them. */
x86::reg32 frontend::mova::projectionCentre(App* app, CPU& cpu, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    guest.setDword(kCentreX, x);
    guest.setDword(kEdgeX, x + guest.dword(kHalfWidth));
    const reg32 halfHeight = guest.dword(kHalfHeight);
    guest.setDword(kCentreY, y);
    const reg32 edgeY = y + halfHeight;
    guest.setDword(kEdgeY, edgeY);
    guest.leaveEdx(edgeY);
    return halfHeight;
}

/* sub_4fd2b0: the projection's view: the horizontal half angle in eax, the vertical in
 * edx, the width in ebx and the height in ecx.  Each focal length is half the view over
 * the tangent of its angle, by the game's sine table, the vertical one negative.  With
 * no vertical angle it comes from the horizontal focal length and the aspect, and the
 * vertical angle from it.  Eax, ebx, ecx and edx stay as the original leaves them. */
x86::reg32 frontend::mova::projectionView(App* app, CPU& cpu, reg32 angleX, reg32 angleY, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest.setDword(kAngleY, angleY);
    const reg32 halfWidth = reg32(sreg32(width) >> 1);
    guest.setDword(kAngleX, angleX);
    guest.setDword(kEdgeX, guest.dword(kCentreX) + halfWidth);
    const reg32 halfHeight = reg32(sreg32(height) >> 1);
    guest.setDword(kHalfWidth, halfWidth);
    guest.setDword(kHalfHeight, halfHeight);
    guest.setDword(kEdgeY, guest.dword(kCentreY) + halfHeight);

    const reg32 scaledX = halfWidth * tableSine(guest, angleX + kQuarterTurn);
    const reg32 sineX = tableSine(guest, angleX);
    const Division focalX = divide(scaledX, sineX);
    guest.setDword(kFocalX, focalX.quotient);

    if (angleY != 0)
    {
        const reg32 scaledY = reg32(0) - tableSine(guest, angleY + kQuarterTurn) * halfHeight;
        const reg32 sineY = tableSine(guest, angleY);
        const Division focalY = divide(scaledY, sineY);
        guest.setDword(kFocalY, focalY.quotient);
        guest.leaveEbx(scaledY);
        guest.leaveEcx(sineY);
        guest.leaveEdx(focalY.remainder);
        return focalY.quotient;
    }

    // 16.16 times, rounded by the bit shifted out.
    const std::int64_t product = std::int64_t(sreg32(focalX.quotient)) * std::int64_t(sreg32(guest.dword(kAspect)));
    const reg32 low = reg32(product), high = reg32(std::uint64_t(product) >> 32);
    const reg32 focalY = (high << 16) + (low >> 16) + ((low >> 15) & 1);
    const reg32 negative = reg32(0) - focalY;
    guest.setDword(kFocalY, negative);
    guest.leaveEbx(scaledX);
    guest.leaveEcx(sineX);
    const reg32 angle = guest.call(kAngleOf, args({ negative, halfHeight }));
    guest.setDword(kAngleY, angle);
    return angle;
}

/* sub_4fd3f0: the projection's view with its half angles in degrees, eax and edx, the
 * width in ebx and the height in ecx; eax what sub_4fd2b0 gives.  What that leaves in ebx,
 * ecx and edx its callers do not read: sub_4f8930 restores them, sub_4dbce0 loads what it
 * uses again (Watcom's callers read eax alone). */
x86::reg32 frontend::mova::projectionViewDegrees(App* app, CPU& cpu, reg32 degreesX, reg32 degreesY, reg32 width,
                                                 reg32 height)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const reg32 angleY = degreesToAngle(degreesY);
    const reg32 angleX = degreesToAngle(degreesX);
    return fe::direct<0x4fd2b0, projectionView>(app, cpu, angleX, angleY, width, height);
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

void frontend::movieScreenDevice(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4eba80", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return mova::screenDevice(app, cpu); });
}

void frontend::movieDirectDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4eba90", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return mova::directDraw(app, cpu); });
}

void frontend::movieDisplayMode(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4df340", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return mova::displayMode(app, cpu); });
}

void frontend::movieDisplayClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4df310", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return mova::displayClose(app, cpu); });
}

void frontend::movieDisplayOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4df1f0", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x = cpu.eax, y = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    const reg32 quality = app->getMemory<reg32>(cpu.esp + 4), software = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site,
                 [&](Guest&) { return mova::displayOpen(app, cpu, x, y, width, height, quality, software); });
}

void frontend::projectionCentre(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd280", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return mova::projectionCentre(app, cpu, x, y); });
}

void frontend::projectionView(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd2b0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 angleX = cpu.eax, angleY = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    fe::runWhole(app, cpu, site,
                 [&](Guest&) { return mova::projectionView(app, cpu, angleX, angleY, width, height); });
}

void frontend::projectionViewDegrees(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd3f0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 degreesX = cpu.eax, degreesY = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    fe::runWhole(app, cpu, site,
                 [&](Guest&) { return mova::projectionViewDegrees(app, cpu, degreesX, degreesY, width, height); });
}

}
