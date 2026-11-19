#include "native_frontend_mova.h"
#include "native_frontend.h"
#include "port_sound_effects.h"

#include "os_video.h"

#include <nfs3hp.h>
#include <cstdint>
#include "native_guest.h"

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
 * The player's body behind madPlay, the 16-bit surface path behind showFrame
 * and EA's stream and file libraries stay the game's code.
 *
 * Plan step 5: each function a typed core in frontend::mova (native_frontend_mova.h), in a frame
 * of its own where the game's function has one, and a thunk under the game's name for the calls
 * by address.  A core of this file calls another as C++ (guest::direct); the rest of the game
 * still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;

// What sub_4df1f0 keeps of the display.
constexpr u32 kDisplayDevice = 0x8ca2c0;   // the screen's device the surface was made for
constexpr u32 kDisplayHeight = 0x8ca2c4;
constexpr u32 kDisplaySurface = 0x8ca2c8;  // the overlay surface, 0 for none
constexpr u32 kDisplayQuality = 0x8ca2cc;
constexpr u32 kDisplayWidth = 0x8ca2d0;
constexpr u32 kDisplayMode = 0x8ca2d4;
constexpr u32 kDisplayTicks = 0x8ca2d8;
constexpr u32 kDisplayY = 0x8ca2dc;
constexpr u32 kDisplayX = 0x8ca2e0;

// The ways a frame reaches the screen.
constexpr u32 kModeOverlay = 0;    // the card's overlay
constexpr u32 kModeStretched = 1;  // the card's stretching blit
constexpr u32 kModeSoftware = 2;   // converted to the 16-bit surface

constexpr u32 kScreenDevice = 0x9f3d3c;
constexpr u32 kDirectDraw = 0x9f38d0;


// The card's capabilities the movies ask for, dwCaps' second byte.
constexpr std::uint8_t kCapsOverlay = 0x01, kCapsOverlayCantClip = 0x02;
constexpr std::uint8_t kCapsBlit = 0x20, kCapsBlitStretch = 0x40;

// The integer projection.
constexpr u32 kHalfWidth = 0x564424, kHalfHeight = 0x564428;
constexpr u32 kCentreX = 0x56442c, kCentreY = 0x564430;
constexpr u32 kEdgeX = 0x564434, kEdgeY = 0x564438;
constexpr u32 kFocalX = 0x56443c, kFocalY = 0x564440;
constexpr u32 kAngleX = 0x564448, kAngleY = 0x56444c;
constexpr u32 kAspect = 0x564450;      // 16.16: the y focal length from the x one's
constexpr u32 kSineTable = 0x5672ac;   // a quarter turn, 0x100 steps, both ends
constexpr u32 kAngleOf = 0x4e8f28;     // the angle of y over x

// A whole turn is 0x400 of the projection's angles.
constexpr u32 kQuarterTurn = 0x100;
constexpr s32 kDegreesInTurn = 0x168;
constexpr unsigned kTurnBits = 10;

/* The game's sine: the quarter table reflected by the angle's bit 8, negated by its
 * bit 9.  The angle's higher bits are not looked at. */
u32 tableSine(const Guest& guest, u32 angle)
{
    const bool reflected = (angle & 0x100) != 0;
    const bool negative = (angle & 0x200) != 0;
    const u32 step = reflected ? 0x100 - (angle & 0xff) : angle & 0xff;
    const u32 value = guest.dword(kSineTable + 4 * step);
    return negative ? u32(0) - value : value;
}

/* idiv of a sign-extended dividend, as the game's code does it. */
struct Division
{
    u32 quotient;
    u32 remainder;
};

Division divide(u32 dividend, u32 divisor)
{
    const std::int64_t wide = std::int64_t(s32(dividend));
    const std::int64_t by = std::int64_t(s32(divisor));
    return { u32(wide / by), u32(wide % by) };
}

/* A whole number of degrees as the projection's angle, cut towards zero. */
u32 degreesToAngle(u32 degrees)
{
    return u32(s32(degrees << kTurnBits) / kDegreesInTurn);
}

}

/* sub_4eba80: the screen's device, as DirectDraw's library keeps it. */
nfs::u32 frontend::mova::screenDevice(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kScreenDevice);
}

/* sub_4eba90: DirectDraw's object. */
nfs::u32 frontend::mova::directDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kDirectDraw);
}

/* sub_4df340: the way a frame reaches the screen, which the player looks at to know
 * whether it decodes the frames nothing is predicted from. */
nfs::u32 frontend::mova::displayMode(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kDisplayMode);
}

/* sub_4df310: the overlay surface let go, when the screen's device is still the one it
 * was made for. */
nfs::u32 frontend::mova::displayClose(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 device = guest::direct<0x4eba80, screenDevice>(app, ts);
    if (device != guest.dword(kDisplayDevice))
        return device;
    const u32 surface = guest.dword(kDisplaySurface);
    if (surface == 0)
        return device;
    // Its references left: none, it is gone.
    os::video::release(app, surface);
    guest.setDword(kDisplaySurface, 0);
    return 0;
}

/* sub_4df1f0: a movie's display opened: x in eax, y in edx, the frame's width in ebx and
 * height in ecx, its quality and "software" on the stack.  Software, as the game's only
 * caller always asks (sub_4960e0 pushes 1), keeps to the 16-bit surface; otherwise the
 * card's capabilities choose.  Eax, ebx, ecx and edx stay as the original leaves them. */
nfs::u32 frontend::mova::displayOpen(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height,
                                       u32 quality, u32 software)
{
    static constexpr u32 kPushes = 12, kLocals = 0x2d8;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kPushes + kLocals);
    guest::direct<0x4df310, displayClose>(app, ts);
    guest.setDword(kDisplayX, x);
    guest.setDword(kDisplayY, y);
    guest.setDword(kDisplayWidth, width);
    guest.setDword(kDisplayHeight, height);
    guest.setDword(kDisplayTicks, 0);
    guest.setDword(kDisplayDevice, 0);
    guest.setDword(kDisplayQuality, quality);
    guest.setDword(kDisplayMode, kModeSoftware);
    if (software != 0)
        return quality;

    guest::direct<0x4eba90, mova::directDraw>(app, ts);
    // The card's capabilities (always had, 0): the port's display has no blits or overlays.
    const std::uint8_t caps = 0;
    // The byte read into ah, over the 0 GetCaps answered.
    const u32 capsInAh = u32(caps) << 8;
    if ((caps & kCapsBlit) != 0 && (caps & kCapsBlitStretch) != 0)
    {
        guest.setDword(kDisplayMode, kModeStretched);
        return capsInAh;
    }
    // And into dh, over what GetCaps left there.
    if ((caps & kCapsOverlay) != 0 && (caps & kCapsOverlayCantClip) != 0)
    {
        guest.setDword(kDisplayMode, kModeOverlay);
        return 0;
    }
    return capsInAh;
}

/* sub_4fd280: the projection's centre on the screen at eax, edx, and its far edges
 * from the half sizes it has.  Eax and edx stay as the original leaves them. */
nfs::u32 frontend::mova::projectionCentre(App* app, ThreadState& ts, u32 x, u32 y)
{
    Guest guest(app, ts);
    guest.setDword(kCentreX, x);
    guest.setDword(kEdgeX, x + guest.dword(kHalfWidth));
    const u32 halfHeight = guest.dword(kHalfHeight);
    guest.setDword(kCentreY, y);
    const u32 edgeY = y + halfHeight;
    guest.setDword(kEdgeY, edgeY);
    return halfHeight;
}

/* sub_4fd2b0: the projection's view: the horizontal half angle in eax, the vertical in
 * edx, the width in ebx and the height in ecx.  Each focal length is half the view over
 * the tangent of its angle, by the game's sine table, the vertical one negative.  With
 * no vertical angle it comes from the horizontal focal length and the aspect, and the
 * vertical angle from it.  Eax, ebx, ecx and edx stay as the original leaves them. */
nfs::u32 frontend::mova::projectionView(App* app, ThreadState& ts, u32 angleX, u32 angleY, u32 width, u32 height)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest.setDword(kAngleY, angleY);
    const u32 halfWidth = u32(s32(width) >> 1);
    guest.setDword(kAngleX, angleX);
    guest.setDword(kEdgeX, guest.dword(kCentreX) + halfWidth);
    const u32 halfHeight = u32(s32(height) >> 1);
    guest.setDword(kHalfWidth, halfWidth);
    guest.setDword(kHalfHeight, halfHeight);
    guest.setDword(kEdgeY, guest.dword(kCentreY) + halfHeight);

    const u32 scaledX = halfWidth * tableSine(guest, angleX + kQuarterTurn);
    const u32 sineX = tableSine(guest, angleX);
    const Division focalX = divide(scaledX, sineX);
    guest.setDword(kFocalX, focalX.quotient);

    if (angleY != 0)
    {
        const u32 scaledY = u32(0) - tableSine(guest, angleY + kQuarterTurn) * halfHeight;
        const u32 sineY = tableSine(guest, angleY);
        const Division focalY = divide(scaledY, sineY);
        guest.setDword(kFocalY, focalY.quotient);
        return focalY.quotient;
    }

    // 16.16 times, rounded by the bit shifted out.
    const std::int64_t product = std::int64_t(s32(focalX.quotient)) * std::int64_t(s32(guest.dword(kAspect)));
    const u32 low = u32(product), high = u32(std::uint64_t(product) >> 32);
    const u32 focalY = (high << 16) + (low >> 16) + ((low >> 15) & 1);
    const u32 negative = u32(0) - focalY;
    guest.setDword(kFocalY, negative);
        const u32 angle = guest::direct<kAngleOf, port::vectorAngle>(app, ts, std::int32_t(negative), std::int32_t(halfHeight));
    guest.setDword(kAngleY, angle);
    return angle;
}

/* sub_4fd3f0: the projection's view with its half angles in degrees, eax and edx, the
 * width in ebx and the height in ecx; eax what sub_4fd2b0 gives.  What that leaves in ebx,
 * ecx and edx its callers do not read: sub_4f8930 restores them, sub_4dbce0 loads what it
 * uses again (Watcom's callers read eax alone). */
nfs::u32 frontend::mova::projectionViewDegrees(App* app, ThreadState& ts, u32 degreesX, u32 degreesY, u32 width,
                                                 u32 height)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 angleY = degreesToAngle(degreesY);
    const u32 angleX = degreesToAngle(degreesX);
    return fe::direct<0x4fd2b0, projectionView>(app, ts, angleX, angleY, width, height);
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::movieScreenDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4eba80", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return mova::screenDevice(app, ts); });
}

nfs::u32 frontend::movieDirectDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4eba90", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return mova::directDraw(app, ts); });
}

nfs::u32 frontend::movieDisplayMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4df340", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return mova::displayMode(app, ts); });
}

nfs::u32 frontend::movieDisplayClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4df310", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return mova::displayClose(app, ts); });
}

nfs::u32 frontend::movieDisplayOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4df1f0", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x = args.eax, y = args.edx, width = args.ebx, height = args.ecx;
    const u32 quality = app->getMemory<u32>(ts.sp + 4), software = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return mova::displayOpen(app, ts, x, y, width, height, quality, software); });
}

nfs::u32 frontend::projectionCentre(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd280", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return mova::projectionCentre(app, ts, x, y); });
}

nfs::u32 frontend::projectionView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd2b0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 angleX = args.eax, angleY = args.edx, width = args.ebx, height = args.ecx;
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return mova::projectionView(app, ts, angleX, angleY, width, height); });
}

nfs::u32 frontend::projectionViewDegrees(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd3f0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 degreesX = args.eax, degreesY = args.edx, width = args.ebx, height = args.ecx;
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return mova::projectionViewDegrees(app, ts, degreesX, degreesY, width, height); });
}

}
