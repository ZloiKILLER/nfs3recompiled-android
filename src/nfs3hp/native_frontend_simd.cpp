#include "native_controls_queue.h"
#include "native_controls_race.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_raceload.h"
#include "native_collide.h"
#include "native_track.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_hudedit.h"
#include "native_trackdata.h"
#include "native_frontend_sima.h"
#include "native_frontend_simc.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menua.h"
#include "native_frontend_neta.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_sime.h"
#include "native_frontend_tail.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "port_sound_racesetup.h"
#include "native_load_cardb.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_carstate.h"
#include "native_graphics.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <lib/app.h>
#include <array>
#include <cstring>
#include <optional>

/* The small pieces of the simulation, decompiled:
 *
 *   the race cameras' steps  the modes of sub_426450's table not yet native: behind the
 *                            car (sub_4243a0, sub_424740), round it (sub_424690,
 *                            sub_424b50, sub_424ce0, sub_424e30), held on it (sub_425990,
 *                            sub_425ab0), and what their neighbours call: the zoom from
 *                            the distance (sub_424fd0, sub_425b40), a turn eased
 *                            (sub_424f30), a mode begun (sub_426370);
 *   shared maths             a distance, a matrix turned over, the cross product
 *                            (sub_4e0090..sub_4e01a0), sine and cosine of the game's angle
 *                            (sub_4ea930..sub_4ea9c0), a matrix turned about an axis
 *                            (sub_4ea9e0, sub_4eaab0, sub_4eab80), a frame from a triangle
 *                            (sub_496a50);
 *   16.16 maths              a vector made one long (sub_4970c0), a matrix turned over and
 *                            a vector by a matrix (sub_4f44e0, sub_4f4544), a point dropped
 *                            onto a plane (sub_4969c0), a place across the road (sub_4c9bc0);
 *   the race's list of tasks the list (sub_4be1f0, sub_4be250), the race's own put on it
 *                            (sub_4bc910), the race opened and started again (sub_4c45f0,
 *                            sub_4c4740, sub_4bc8e0);
 *   small things             the network's frame (sub_4c7bd0), the HUD's panes' defaults
 *                            (sub_485880), and a few one-line calls.
 *
 * This runs every frame and the replays must stay identical to the bit, so the x87's
 * arithmetic is written out through fp:: in the original's order, operand for operand,
 * each value stored and fetched back where the original stores and fetches it.  The state
 * stays in guest memory, what these call is called by address, and the locals the game's
 * functions read are where they were in the frame.  What each function leaves in eax and
 * in the registers it does not save is kept: Watcom's callers may go on with them. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace frontend
{
nfs::u32 simCameraChase(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraOrbit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraChaseLow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraCircleCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraLookAround(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraSideOfCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraEaseTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraMoveTowards(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraBesideCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraOnCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraGlideTowards(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCameraPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simBonusCarLocked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simHudDefaults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simTrackPointOnPlane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simFrameOfTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simFixedNormalise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simOrFlags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simOpenWebPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceLeaveTasks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceAddTasks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simTaskListNew(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simTaskListAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simSetClipRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceRestart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simDepthOffsets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simNetSendPlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simPlaceInLane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceClearA(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRaceClearB(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simViewProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simWeatherStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRainAmount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simVectorDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simTranspose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCross(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simUnitCross(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simSine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simCosine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simSineCosine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simAngleOfFloats(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRotateX(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRotateY(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simRotateZ(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simFixedTranspose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 simFixedMatrixTimes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}

namespace
{

using fe::args;
using fe::Guest;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::s32;

constexpr std::uint8_t kRestoreNone = 0;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(u16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(u16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered
bool jumpsEqual(u16 status) { return (status & 0x4000) != 0; }  // je; true unordered

/* eax after fnstsw ax: only its low half changes. */
u32 withStatus(u32 eax, u16 status) { return (eax & 0xffff0000) | status; }

u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, u32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, u32 address) { return Float(s32(guest.dword(address))); }

/* fstp dword */
void store(Guest& guest, u32 address, Float value) { guest.setFloat(address, float(value)); }

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The game's functions, by address.
constexpr u32 kNormalise = 0x4972f0, kUnitVector = 0x4e01c0, kVectorLength = 0x4e06a0, kArcTangent = 0x4e06d9;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kScaledIfSet = 0x423fb0, kDampVector = 0x424030;
constexpr u32 kPlaceOnTrack = 0x420650;

// A car.
constexpr u32 kCarFlags = 0x200;              // byte
constexpr u32 kCarPlace = 0x98;               // three floats
constexpr u32 kCarAxes = 0xc0;                // nine floats
constexpr u32 kCarGrip = 0x10c;               // a float, compared by its bits
constexpr u32 kCarLift = 0x118, kCarBack = 0x11c;
constexpr u32 kCarDriving = 0x528;
constexpr u32 kCarWheelsOff = 0x998;
constexpr u32 kCarTowed = 0x9a4;              // what a flagged car rides on
constexpr u32 kCarState = 0x2c4;
constexpr u32 kLeastGripBits = 0x3f34fdf4;    // below it the car is off the ground: compared as ints

// A race camera.
constexpr u32 kCameraCar = 0x04, kCameraTargetCar = 0x08, kCameraView = 0x0c;
constexpr u32 kCameraPlace = 0x14, kCameraTarget = 0x20;
constexpr u32 kCameraZoom = 0x54;
constexpr u32 kCameraNode = 0x84;
constexpr u32 kCameraWasAirborne = 0xfc;
constexpr u32 kCameraAirWait = 0x10c;         // frames left to wait
constexpr u32 kCameraOwn110 = 0x110, kCameraOwn114 = 0x114, kCameraOwn118 = 0x118, kCameraOwn11c = 0x11c;
constexpr u32 kCameraOwn120 = 0x120, kCameraOwn124 = 0x124, kCameraOwn128 = 0x128;

// The race.
constexpr u32 kStartShown = 0x7d36a8, kStarted = 0x7cd960, kFrozen = 0x5600ec;
constexpr u32 kLookingBack = 0x79c4a0;        // u32[2]: each view's
constexpr u32 kViewHoldsNoTarget = 0x5e1010;  // [view * 8]: nonzero, the chase's target jumps
constexpr u32 kViewInTunnel = 0x5e1014;       // [view * 8]: frames the view's been in a tunnel
constexpr u32 kCarsByPlace = 0x5efac8;

/* The race not yet under way on the screen, or its cameras frozen. */
bool cameraHeld(const Guest& guest)
{
    if (guest.dword(kStartShown) != 0 && guest.dword(kStarted) == 0)
        return true;
    return guest.dword(kFrozen) != 0;
}

// ---------------------------------------------------------------------------
// Sine and cosine of the game's angle
// ---------------------------------------------------------------------------

/* sub_4ea930's and sub_4ea950's: the x87's whole width, not rounded. */
Float sineOf(const Guest& guest, float angle)
{
    constexpr u32 kSineUnit = 0x54b6b0;
    return fp::sin(fp::mul(Float(angle), doubleAt(guest, kSineUnit)));
}

Float cosineOf(const Guest& guest, float angle)
{
    constexpr u32 kCosineUnit = 0x54b6b8;
    return fp::cos(fp::mul(Float(angle), doubleAt(guest, kCosineUnit)));
}

/* sub_4ea970: sine to `sineAt`, then cosine to `cosineAt`, of the float at `angleAt` read
 * each time: the sine's place may be the angle's. */
void sineCosineInto(Guest& guest, u32 angleAt, u32 sineAt, u32 cosineAt)
{
    store(guest, sineAt, sineOf(guest, guest.floatAt(angleAt)));
    store(guest, cosineAt, cosineOf(guest, guest.floatAt(angleAt)));
}

// ---------------------------------------------------------------------------
// Vectors and matrices
// ---------------------------------------------------------------------------

/* The three floats at `v` by the nine at `m`, to `out`: each row's three products, then
 * their sums, the first row first, as all the cameras inline it. */
void vectorByMatrix(Guest& guest, u32 v, u32 m, u32 out)
{
    Float sum[3];
    for (u32 axis = 0; axis < 3; ++axis)
        sum[axis] = fp::mul(floatFrom(guest, v), floatFrom(guest, m + 4 * axis));
    for (u32 row = 1; row < 3; ++row)
        for (u32 axis = 0; axis < 3; ++axis)
            sum[axis] = fp::add(sum[axis], fp::mul(floatFrom(guest, v + 4 * row), floatFrom(guest, m + 12 * row + 4 * axis)));
    for (u32 axis = 0; axis < 3; ++axis)
        store(guest, out + 4 * axis, sum[axis]);
}

/* sub_4e0160: a by b, to `out`, which may be either. */
void crossProduct(Guest& guest, u32 a, u32 b, u32 out)
{
    const Float x1 = fp::mul(floatFrom(guest, a + 4), floatFrom(guest, b + 8));
    const Float y1 = fp::mul(floatFrom(guest, a + 8), floatFrom(guest, b));
    const Float z1 = fp::mul(floatFrom(guest, a), floatFrom(guest, b + 4));
    const Float x2 = fp::mul(floatFrom(guest, a + 8), floatFrom(guest, b + 4));
    const Float y2 = fp::mul(floatFrom(guest, a), floatFrom(guest, b + 8));
    const Float z2 = fp::mul(floatFrom(guest, a + 4), floatFrom(guest, b));
    const Float x = fp::sub(x1, x2);
    const Float y = fp::sub(y1, y2);
    const Float z = fp::sub(z1, z2);
    store(guest, out, x);
    store(guest, out + 4, y);
    store(guest, out + 8, z);
}

/* The original's `imul edx; shl edx, 16; shr eax, 16; adc eax, edx`: a product in 16.16,
 * rounded by the last bit shifted out, in 32 bits. */
u32 fixedTimes(u32 a, u32 b)
{
    const std::uint64_t product = std::uint64_t(std::int64_t(s32(a)) * std::int64_t(s32(b)));
    const u32 low = u32(product), high = u32(product >> 32);
    return (high << 16) + (low >> 16) + ((low >> 15) & 1);
}

/* sub_4f4544: the three 16.16 ints at `a` by the nine at `m`, to `out`, a column at a time, a
 * 64-bit sum for each, read after the last was written: `out` may be either.  The last sum
 * is what eax and edx are left with. */
struct FixedSum
{
    u32 eax, edx;
};

FixedSum fixedMatrixTimesInto(Guest& guest, u32 a, u32 m, u32 out)
{
    FixedSum left{ 0, 0 };
    for (u32 column = 0; column < 3; ++column)
    {
        std::uint64_t sum = 0;
        for (u32 row = 0; row < 3; ++row)
            sum += std::uint64_t(std::int64_t(s32(guest.dword(a + 4 * row)))
                                 * std::int64_t(s32(guest.dword(m + 12 * row + 4 * column))));
        left.eax = u32(sum >> 16);
        left.edx = u32(sum >> 32);
        guest.setDword(out + 4 * column, left.eax);
    }
    return left;
}

/* sub_4f44e0's copy: the nine ints at `from` turned over to `to`, the two apart. */
void fixedTransposeInto(Guest& guest, u32 from, u32 to)
{
    guest.setDword(to, guest.dword(from));
    guest.setDword(to + 0x04, guest.dword(from + 0x0c));
    guest.setDword(to + 0x08, guest.dword(from + 0x18));
    guest.setDword(to + 0x0c, guest.dword(from + 0x04));
    guest.setDword(to + 0x10, guest.dword(from + 0x10));
    guest.setDword(to + 0x14, guest.dword(from + 0x1c));
    guest.setDword(to + 0x18, guest.dword(from + 0x08));
    guest.setDword(to + 0x1c, guest.dword(from + 0x14));
    guest.setDword(to + 0x20, guest.dword(from + 0x20));
}

// ---------------------------------------------------------------------------
// The race cameras
// ---------------------------------------------------------------------------

/* The original's absolute value, inline at each use: fldz; fcomp x, and the sign changed
 * only below 0, so that -0 stays -0 and a NaN stays as it is. */
float absolute(float value)
{
    return jumpsAbove(fp::compare(0.0, Float(value))) ? float(-Float(value)) : value;
}

/* A car off the ground, or on it again: how fast the two chase cameras follow, a float's
 * bits.  Off it, a wait begins (noting whether the car had left its wheels as it went); while
 * the wait lasts they stand still.  On the ground, the rate of the camera's own speed, and
 * unless the view looks back the offset damped by it. */
u32 chaseRate(Guest& guest, u32 camera, u32 car, u32 offset)
{
    if (s32(guest.dword(car + kCarGrip)) < s32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
            guest.setDword(camera + kCameraWasAirborne, s32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
        guest.setDword(camera + kCameraAirWait, 3);
    }
    else if (guest.dword(camera + kCameraAirWait) == 0)
    {
        const u32 own = guest.dword(camera + kCameraOwn11c);
        const u32 rate = bitsOf(guest::direct<kScaledIfSet, carstate::scaledWhereSet>(guest.app(), guest.ts(), camera, own));
        if (guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) == 0)
            fe::direct<kDampVector, carstate::vectorDamped>(guest.app(), guest.ts(), car, guest.dword(camera + kCameraOwn120), offset);
        return rate;
    }
    guest.setDword(camera + kCameraAirWait, guest.dword(camera + kCameraAirWait) - 1);
    return 0;
}

/* The end of the two chase cameras looking ahead: the target eases from where it was to the
 * point behind the car (it jumps there as a wait ends, or when the view holds none), the
 * camera stands at the car's place plus it. */
u32 chaseFollow(nfs::ThreadState& ts, Guest& guest, u32 camera, u32 rate, u32 place, u32 point, u32 from, u32 difference, u32 scaled)
{
    const u32 target = camera + kCameraTarget;
    if (guest.dword(camera + kCameraAirWait) == 1)
    {
        copyDwords(guest, from, point, 3);
        guest.setDword(camera + kCameraAirWait, 0);
    }
    else if (guest.dword(kViewHoldsNoTarget + guest.dword(camera + kCameraView) * 8) != 0)
    {
        copyDwords(guest, from, point, 3);
    }
    else
    {
        copyDwords(guest, from, target, 3);
    }
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(guest.app(), ts, 1, point, from, difference);
    // Handed `scaled` and `target` in ebx and ecx too, which it does not read: there as the code has them.
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(guest.app(), ts, 1, difference, floatOf(rate), scaled);
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), ts, 1, from, scaled, target);
    return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, place, target, camera + kCameraPlace);
}

}  // namespace

/* sub_4243a0: behind the car, higher as its table says, set back by the car's size. */
nfs::u32 frontend::simd::cameraChase(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x5c);
    constexpr u32 kLocalOffset = 0x00, kLocalScaled = 0x0c, kLocalFrom = 0x18, kLocalPoint = 0x24, kLocalDifference = 0x30;
    constexpr u32 kDrivingLift = 0x2c0, kLiftUnit = 0x536ebc, kMostLift = 0x536ec4, kSizeShare = 0x536ecc;
    constexpr double kLeastLift = 0x1.2p-13;

    const u32 offset = guest.frame() + kLocalOffset;
    const u32 point = guest.frame() + kLocalPoint;
    copyDwords(guest, offset, camera + kCameraOwn110, 3);
    const u32 car = guest.dword(camera + kCameraCar);
    if (cameraHeld(guest))
        return camera;
    const u32 rate = chaseRate(guest, camera, car, offset);

    const Float lift = fp::mul(integerFrom(guest, guest.dword(car + kCarDriving) + kDrivingLift), doubleAt(guest, kLiftUnit));
    const double added = jumpsBelow(fp::compare(lift, doubleAt(guest, kMostLift))) ? double(lift) : kLeastLift;
    store(guest, offset + 4, fp::add(floatFrom(guest, offset + 4), Float(added)));
    const Float share = floatFrom(guest, kSizeShare);
    store(guest, offset + 4, fp::add(fp::mul(floatFrom(guest, car + kCarLift), share), floatFrom(guest, offset + 4)));
    store(guest, offset + 8, fp::sub(floatFrom(guest, offset + 8), fp::mul(share, floatFrom(guest, car + kCarBack))));

    const bool back = guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) != 0;
    if (back)
        guest.setByte(offset + 0x0b, guest.byte(offset + 0x0b) ^ 0x80);
    vectorByMatrix(guest, offset, guest.dword(camera + kCameraCar) + kCarAxes, point);
    const u32 place = guest.dword(camera + kCameraCar) + kCarPlace;
    if (back)
        return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, place, point, camera + kCameraPlace);
    return chaseFollow(ts, guest, camera, rate, place, point, guest.frame() + kLocalFrom, guest.frame() + kLocalDifference,
                       guest.frame() + kLocalScaled);
}

/* sub_424690: round the car (or what it rides on) on the race's clock. */
nfs::u32 frontend::simd::cameraOrbit(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16 + 0x18);
    constexpr std::uint8_t kCarRides = 0x20;
    constexpr u32 kClock = 0x7d3684, kClockUnit = 0x536ed0, kAcross = 0x536ed4, kUp = 0x536ed8, kAlong = 0x536edc;
    constexpr u32 kLocalSine = 0x0c, kLocalAngle = 0x10, kLocalCosine = 0x14;

    u32 car = guest.dword(camera + kCameraCar);
    if (guest.byte(car + kCarFlags) & kCarRides)
        car = guest.dword(car + kCarTowed);
    const float x = guest.floatAt(car + kCarPlace), y = guest.floatAt(car + kCarPlace + 4), z = guest.floatAt(car + kCarPlace + 8);
    guest.setDword(camera + kCameraTargetCar, car);
    guest.setDword(camera + kCameraCar, car);
    if (cameraHeld(guest))
        return car;

    const u32 frame = guest.frame();
    store(guest, frame + kLocalAngle, fp::mul(integerFrom(guest, kClock), floatFrom(guest, kClockUnit)));
    sineCosineInto(guest, frame + kLocalAngle, frame + kLocalSine, frame + kLocalCosine);
    const Float sine = floatFrom(guest, frame + kLocalSine), cosine = floatFrom(guest, frame + kLocalCosine);
    store(guest, camera + kCameraPlace, fp::add(fp::mul(cosine, floatFrom(guest, kAcross)), Float(x)));
    store(guest, camera + kCameraPlace + 4, fp::add(fp::mul(sine, floatFrom(guest, kUp)), Float(y)));
    store(guest, camera + kCameraPlace + 8, fp::add(fp::mul(sine, floatFrom(guest, kAlong)), Float(z)));
    return frame + kLocalAngle;
}

/* sub_424740: behind the car and low, its own offset unless a tunnel has gone on past 32
 * frames, lower by how much the car's turned about two of its axes. */
nfs::u32 frontend::simd::cameraChaseLow(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x78);
    constexpr u32 kLocalScaled = 0x00, kLocalDifference = 0x0c, kLocalOffset = 0x18, kLocalFrom = 0x24, kLocalPoint = 0x30, kLocalPlace = 0x3c;
    constexpr u32 kSizeShare = 0x536ee0, kCarTurnA = 0x4ec, kCarTurnB = 0x4f4, kSmallerShare = 0x536ee4, kDropUnit = 0x536ee8, kMostDrop = 0x536eec;
    constexpr s32 kTunnelFrames = 0x20;
    constexpr u32 kTunnelUpBits = 0x3f800000, kTunnelBackBits = 0xc0500000;
    constexpr float kDropWhenMost = 6.0f;

    const u32 frame = guest.frame();
    const u32 offset = frame + kLocalOffset, point = frame + kLocalPoint, place = frame + kLocalPlace;
    copyDwords(guest, offset, camera + kCameraOwn110, 3);
    const u32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, place, car + kCarPlace, 3);
    const u32 lastPlaceBits = guest.dword(car + kCarPlace + 8);
    if (cameraHeld(guest))
        return lastPlaceBits;

    if (s32(guest.dword(kViewInTunnel + guest.dword(camera + kCameraView) * 8)) > kTunnelFrames)
    {
        guest.setDword(offset, 0);
        guest.setDword(offset + 8, kTunnelBackBits);
        guest.setDword(offset + 4, kTunnelUpBits);
    }
    const u32 rate = chaseRate(guest, camera, car, offset);

    const Float share = floatFrom(guest, kSizeShare);
    store(guest, offset + 4, fp::add(fp::mul(floatFrom(guest, car + kCarLift), share), floatFrom(guest, offset + 4)));
    store(guest, offset + 8, fp::sub(floatFrom(guest, offset + 8), fp::mul(share, floatFrom(guest, car + kCarBack))));

    // The larger of the two turns and a share of the smaller.
    const float turnA = absolute(guest.floatAt(car + kCarTurnA)), turnB = absolute(guest.floatAt(car + kCarTurnB));
    const float sum = float(jumpsAbove(fp::compare(Float(turnA), Float(turnB)))
                                ? fp::add(Float(turnA), Float(float(fp::mul(Float(turnB), floatFrom(guest, kSmallerShare)))))
                                : fp::add(Float(turnB), Float(float(fp::mul(Float(turnA), floatFrom(guest, kSmallerShare))))));
    const Float drop = fp::mul(Float(sum), floatFrom(guest, kDropUnit));
    const float dropped = jumpsAbove(fp::compare(drop, floatFrom(guest, kMostDrop))) ? kDropWhenMost : float(drop);
    store(guest, offset + 8, fp::sub(floatFrom(guest, offset + 8), Float(dropped)));

    const bool back = guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) != 0;
    if (back)
        guest.setByte(offset + 0x0b, guest.byte(offset + 0x0b) ^ 0x80);
    vectorByMatrix(guest, offset, guest.dword(camera + kCameraCar) + kCarAxes, point);
    if (back)
        return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, place, point, camera + kCameraPlace);
    return chaseFollow(ts, guest, camera, rate, place, point, frame + kLocalFrom, frame + kLocalDifference, frame + kLocalScaled);
}

/* sub_424b50: round the car, the angle going on every so many frames. */
nfs::u32 frontend::simd::cameraCircleCar(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x2c);
    constexpr u32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kStep = kCameraOwn11c;
    constexpr u32 kEvery = kCameraOwn120, kCount = kCameraOwn124, kAngle = kCameraOwn128;
    constexpr u32 kLocalPlace = 0x00, kLocalPoint = 0x0c, kLocalOffset = 0x18, kLocalSine = 0x24, kLocalCosine = 0x28;

    const u32 frame = guest.frame();
    const u32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, frame + kLocalPlace, car + kCarPlace, 3);
    const u32 lastPlaceBits = guest.dword(car + kCarPlace + 8);
    if (cameraHeld(guest))
        return lastPlaceBits;

    if (s32(guest.dword(car + kCarGrip)) < s32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
        {
            guest.setDword(camera + kCameraWasAirborne, s32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, frame + kLocalPlace, camera + kCameraPlace,
                                                                     camera + kCameraTarget);
        }
        guest.setDword(camera + kCameraAirWait, 0x40);
    }
    const u32 count = guest.dword(camera + kCount);
    guest.setDword(camera + kCount, count + 1);
    if (s32(count) > s32(guest.dword(camera + kEvery)))
    {
        const Float angle = fp::add(floatFrom(guest, camera + kStep), floatFrom(guest, camera + kAngle));
        guest.setDword(camera + kCount, 0);
        store(guest, camera + kAngle, angle);
    }
    sineCosineInto(guest, camera + kAngle, frame + kLocalSine, frame + kLocalCosine);
    const Float sine = floatFrom(guest, frame + kLocalSine), cosine = floatFrom(guest, frame + kLocalCosine);
    store(guest, frame + kLocalOffset, fp::mul(floatFrom(guest, camera + kRadiusAcross), cosine));
    store(guest, frame + kLocalOffset + 8, fp::mul(floatFrom(guest, camera + kRadiusAlong), sine));
    store(guest, frame + kLocalOffset + 4, fp::add(floatFrom(guest, camera + kHeight), sine));
    vectorByMatrix(guest, frame + kLocalOffset, guest.dword(camera + kCameraCar) + kCarAxes, frame + kLocalPoint);
    return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, frame + kLocalPlace, frame + kLocalPoint, camera + kCameraPlace);
}

/* sub_424ce0: round the car by the player's keys, every other tick. */
nfs::u32 frontend::simd::cameraLookAround(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x20);
    constexpr u32 kTicks = 0x79c1a4, kKeysHeld = 0x55301c, kKeyTurn = 0x553020;
    constexpr u32 kAutoLeft = 0x536ef0, kAutoRight = 0x536ef4, kKeyUnit = 0x536ef8;
    constexpr u32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kAuto = kCameraOwn11c;
    constexpr u32 kAngle = kCameraOwn120, kLastTick = kCameraOwn124;
    constexpr u32 kLocalOffset = 0x00, kLocalPoint = 0x0c, kLocalSine = 0x18, kLocalCosine = 0x1c;

    const u32 frame = guest.frame();
    const u32 ticks = guest.dword(kTicks);
    if (s32(ticks - guest.dword(camera + kLastTick)) < 2)
        return ticks;
    guest.setDword(camera + kLastTick, ticks);
    if (guest.dword(kKeysHeld) == 0 || guest.dword(kKeyTurn) != 0)
    {
        guest.setDword(camera + kAuto, 0);
        Float angle;
        if (guest.dword(kKeysHeld) != 0)
            angle = fp::add(fp::mul(integerFrom(guest, kKeyTurn), floatFrom(guest, kKeyUnit)), floatFrom(guest, camera + kAngle));
        else if (guest.dword(kKeyTurn) == u32(-1))
            angle = fp::add(floatFrom(guest, camera + kAngle), floatFrom(guest, kAutoLeft));
        else
            angle = fp::add(floatFrom(guest, camera + kAngle), floatFrom(guest, kAutoRight));
        store(guest, camera + kAngle, angle);
    }
    sineCosineInto(guest, camera + kAngle, frame + kLocalSine, frame + kLocalCosine);
    const Float sine = floatFrom(guest, frame + kLocalSine), cosine = floatFrom(guest, frame + kLocalCosine);
    store(guest, frame + kLocalOffset, fp::mul(floatFrom(guest, camera + kRadiusAcross), cosine));
    store(guest, frame + kLocalOffset + 8, fp::mul(floatFrom(guest, camera + kRadiusAlong), sine));
    store(guest, frame + kLocalOffset + 4, fp::add(floatFrom(guest, camera + kHeight), sine));
    const u32 car = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, frame + kLocalOffset, car + kCarAxes, frame + kLocalPoint);
    return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, car + kCarPlace, frame + kLocalPoint, camera + kCameraPlace);
}

/* sub_424e30: at the side of the car, on the side its node says, the angle from a count of
 * the car's; it looks at where it stands. */
nfs::u32 frontend::simd::cameraSideOfCar(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x24);
    constexpr u32 kCarCount = 0x524, kCarNode = 0x1c, kCountUnit = 0x536efc, kCountBase = 0x536f00;
    constexpr u32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kAngle = kCameraOwn120;
    constexpr u32 kOneBits = 0x3f800000, kMinusOneBits = 0xbf800000;
    constexpr u32 kLocalOffset = 0x00, kLocalPoint = 0x0c, kLocalSine = 0x18, kLocalCosine = 0x1c, kLocalSide = 0x20;

    const u32 frame = guest.frame();
    const u32 car = guest.dword(camera + kCameraCar);
    store(guest, camera + kAngle, fp::add(fp::mul(integerFrom(guest, car + kCarCount), floatFrom(guest, kCountUnit)), floatFrom(guest, kCountBase)));
    guest.setDword(frame + kLocalSide, (guest.byte(car + kCarNode) & 1) ? kOneBits : kMinusOneBits);
    sineCosineInto(guest, camera + kAngle, frame + kLocalSine, frame + kLocalCosine);
    store(guest, frame + kLocalOffset,
          fp::mul(fp::mul(floatFrom(guest, frame + kLocalCosine), floatFrom(guest, frame + kLocalSide)), floatFrom(guest, camera + kRadiusAcross)));
    store(guest, frame + kLocalOffset + 8, fp::mul(floatFrom(guest, camera + kRadiusAlong), floatFrom(guest, frame + kLocalSine)));
    guest.setDword(frame + kLocalOffset + 4, guest.dword(camera + kHeight));
    const u32 carNow = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, frame + kLocalOffset, carNow + kCarAxes, frame + kLocalPoint);
    const u32 result = fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, carNow + kCarPlace, frame + kLocalPoint, camera + kCameraPlace);
    copyDwords(guest, camera + kCameraTarget, frame + kLocalPoint, 3);
    return result;
}

/* sub_424f30: the camera's matrix eased, in quaternions, from the turn it holds towards the
 * one it has now, by its rate; the first time the turn is only noted.  At the second of two
 * passes while the first is the one running, the holding ends. */
nfs::u32 frontend::simd::cameraEaseTurn(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x24);
    constexpr u32 kCameraMatrix = 0x2c, kHeld = kCameraOwn114, kHolds = kCameraOwn124, kRate = kCameraOwn128;
    constexpr u32 kPasses = 0x7cda58, kPass = 0x7cd94c;
    constexpr u32 kLocalNow = 0x00, kLocalEased = 0x10;

    const u32 frame = guest.frame();
    const u32 result = guest::direct<0x4b73c0, simc::quatFromMatrix>(app, ts, camera + kCameraMatrix, frame + kLocalNow);
    if (guest.dword(camera + kHolds) == 0)
    {
        copyDwords(guest, camera + kHeld, frame + kLocalNow, 4);
        guest.setDword(camera + kHolds, 1);
        return result;
    }
    const u32 rate = guest.dword(camera + kRate);
    const u32 passes = guest.dword(kPasses);
    if (passes == 2 && s32(passes) > s32(guest.dword(kPass)))
    {
        guest.setDword(camera + kHolds, 0);
        return rate;
    }
    // Handed the passes in ebx too, which it does not read: there as the code has it.
    guest::direct<0x4b71e0, simc::quatSlerp>(app, ts, camera + kHeld, frame + kLocalNow, rate, frame + kLocalEased);
    copyDwords(guest, camera + kHeld, frame + kLocalEased, 4);
    return guest::direct<0x4b7590, simc::matrixFromQuat>(app, ts, frame + kLocalEased, camera + kCameraMatrix);
}

namespace
{

/* What sub_424fd0 and sub_425b40 begin with: the camera at the spot, and the distance from
 * its target car's place to the spot, asked once for its sign and once for itself. */
struct GoesTo
{
    float distance;
    u32 eax;  // what the second call left there
};

GoesTo cameraGoesTo(nfs::ThreadState& ts, Guest& guest, u32 camera, u32 spot)
{
    copyDwords(guest, camera + kCameraPlace, spot, 3);
    const Float first = Float(guest::direct<0x4965b0, frontend::sima::quickDistanceOf>(
                                   guest.app(), ts, guest.dword(camera + kCameraTargetCar) + kCarPlace, spot)
                                   .value);
    const bool below = jumpsAbove(fp::compare(0.0, first));
    const frontend::sima::FloatLeft again = guest::direct<0x4965b0, frontend::sima::quickDistanceOf>(
        guest.app(), ts, guest.dword(camera + kCameraTargetCar) + kCarPlace, spot);
    const Float second = Float(again.value);
    return { float(below ? -second : second), again.eax };
}

}  // namespace

/* sub_424fd0: the camera to the spot, the zoom the distance over the stack's float if that is
 * something and the distance is past it, else none. */
nfs::u32 frontend::simd::cameraMoveTowards(App* app, ThreadState& ts, u32 camera, u32 spot, u32 near)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20 + 4);
    const GoesTo goes = cameraGoesTo(ts, guest, camera, spot);
    const float distance = goes.distance;
    const u16 status = fp::compare(Float(distance), Float(floatOf(near)));
    if (jumpsAbove(status) && (near & 0x7fffffff) != 0)
        store(guest, camera + kCameraZoom, fp::div(Float(distance), Float(floatOf(near))));
    else
        guest.setDword(camera + kCameraZoom, 0);
    return withStatus(goes.eax, status);
}

/* sub_425b40: the camera to the spot, the zoom the stack's base and rate at that distance.
 * Each fifth time it prints the two to a buffer it then forgets, the developers' trace. */
nfs::u32 frontend::simd::cameraGlideTowards(App* app, ThreadState& ts, u32 camera, u32 spot, u32 base, u32 rate)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20 + 0x104);
    constexpr u32 kCount = 0x553028, kFormat = 0x536f24;
    constexpr u32 kLocalText = 0x00;
    constexpr u32 kEveryFifth = 5;

    const u32 frame = guest.frame();
    const GoesTo goes = cameraGoesTo(ts, guest, camera, spot);
    const float distance = goes.distance;
    const u32 count = guest.dword(kCount) + 1;
    const Float zoom = fp::add(fp::mul(Float(distance), Float(floatOf(rate))), Float(floatOf(base)));
    guest.setDword(kCount, count);
    store(guest, camera + kCameraZoom, zoom);
    if (count != kEveryFifth)
        return goes.eax;

    const double shownDistance = double(distance), shownZoom = double(guest.floatAt(camera + kCameraZoom));
    u32 words[4];
    std::memcpy(words, &shownDistance, 8);
    std::memcpy(words + 2, &shownZoom, 8);
    const u32 result = fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), frame + kLocalText, kFormat, guest::Words{words[0], words[1], words[2], words[3]});
    guest.setDword(kCount, 0);
    return result;
}

/* sub_425990: held to the car at the camera's offset, its third part's sign changed for good
 * each frame the view looks back; as the car leaves the ground the target noted, and a wait. */
nfs::u32 frontend::simd::cameraBesideCar(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x18);
    constexpr u32 kLocalPoint = 0x00, kLocalPlace = 0x0c;

    const u32 frame = guest.frame();
    const u32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, frame + kLocalPlace, car + kCarPlace, 3);
    if (s32(guest.dword(car + kCarGrip)) < s32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
        {
            guest.setDword(camera + kCameraWasAirborne, s32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, camera + kCameraPlace, frame + kLocalPlace,
                                                                     camera + kCameraTarget);
        }
        guest.setDword(camera + kCameraAirWait, 0x40);
    }
    if (guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) != 0)
        guest.setByte(camera + kCameraOwn118 + 3, guest.byte(camera + kCameraOwn118 + 3) ^ 0x80);
    vectorByMatrix(guest, camera + kCameraOwn110, guest.dword(camera + kCameraCar) + kCarAxes, frame + kLocalPoint);
    return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, frame + kLocalPlace, frame + kLocalPoint, camera + kCameraPlace);
}

/* sub_425ab0: on the car, at the camera's offset. */
nfs::u32 frontend::simd::cameraOnCar(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16 + 0x0c);
    const u32 frame = guest.frame();
    const u32 car = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, camera + kCameraOwn110, car + kCarAxes, frame);
    return fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(guest.app(), ts, 1, car + kCarPlace, frame, camera + kCameraPlace);
}

/* sub_426370: a mode about to step (eax its camera, edx the view, ebx the mode): the frames
 * the view has both car and camera in a tunnel counted, and but for mode 8 the wait of that
 * mode's camera put back. */
nfs::u32 frontend::simd::cameraPrepare(App* app, ThreadState& ts, u32 camera, u32 view, u32 mode)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    constexpr u32 kBesideRoadCameras = 0x552f08, kWaiting = 0x124, kCarNode = 0x1c;
    constexpr u32 kModeBesideRoad = 8;

    u32 result = camera;
    const u32 car = guest.dword(camera + kCameraCar);
    if (car != 0)
    {
        result = *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, ts, car + kCarPlace, guest.dword(car + kCarNode));
        if (result != 0)
            result = *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, ts, camera + kCameraPlace,
                                                                     guest.dword(camera + kCameraNode));
        if (result != 0)
            guest.setDword(kViewInTunnel + view * 8, guest.dword(kViewInTunnel + view * 8) + 1);
        else
            guest.setDword(kViewInTunnel + view * 8, 0);
    }
    if (mode != kModeBesideRoad)
    {
        result = guest.dword(kBesideRoadCameras + guest.dword(camera + kCameraView) * 4);
        guest.setDword(result + kWaiting, 0);
    }
    return result;
}

namespace
{

// ---------------------------------------------------------------------------
// The shared maths
// ---------------------------------------------------------------------------

}  // namespace

/* sub_4970c0: a vector of 16.16 made one long, in place: brought down until its squares fit,
 * the length from the square root's table by the sum's top bit, each part over it. */
nfs::u32 frontend::simd::fixedNormalise(App* app, ThreadState& ts, u32 vector)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x18);
    constexpr s32 kMostPart = 0x1000000;
    constexpr u32 kMostSquares = 0x40000000;
    constexpr u32 kRootScale = 0x5661c4, kRootBase = 0x566248;

    s32 x = s32(guest.dword(vector)), y = s32(guest.dword(vector + 4)), z = s32(guest.dword(vector + 8));
    const auto over = [](s32 part) {
        // neg, on 0x80000000 too, which stays below and so passes.
        const u32 size = part > 0 ? u32(part) : 0u - u32(part);
        return s32(size) > kMostPart;
    };
    while (over(x) || over(y) || over(z))
    {
        x >>= 1;
        y >>= 1;
        z >>= 1;
    }
    u32 xx = fixedTimes(u32(x), u32(x));
    u32 yy = fixedTimes(u32(y), u32(y));
    u32 zz = fixedTimes(u32(z), u32(z));
    while ((xx >> 2) + (yy >> 2) + (zz >> 2) > kMostSquares)
    {
        x >>= 1;
        y >>= 1;
        z >>= 1;
        xx = fixedTimes(u32(x), u32(x));
        yy = fixedTimes(u32(y), u32(y));
        zz = fixedTimes(u32(z), u32(z));
    }
    u32 length = xx + yy + zz;
    if (length != 0)
    {
        const u32 top = u32(31 - __builtin_clz(length));
        const std::uint64_t scaled = std::uint64_t(length) * guest.dword(kRootScale + top * 4);
        length = u32(scaled >> 16) + guest.dword(kRootBase + top * 4);
    }
    if (length == 0)
        return 0;
    // Two divisions of 64 bits by 32, whole part then fraction, as idiv's low halves.
    const auto over16 = [&](s32 part) {
        const std::int64_t by = s32(length);
        const u32 whole = u32(std::int64_t(part) / by);
        const s32 rest = s32(std::int64_t(part) % by);
        return u32((std::int64_t(rest) * 0x10000) / by) + (whole << 16);
    };
    guest.setDword(vector, over16(x));
    guest.setDword(vector + 4, over16(y));
    const u32 lastPart = over16(z);
    guest.setDword(vector + 8, lastPart);
    return lastPart;
}

/* sub_4969c0: the height of the stack's point dropped onto the plane through the stack's
 * origin that the matrix eax holds flat: to the matrix's frame, its second part cleared,
 * back; the y of that, in eax. */
nfs::u32 frontend::simd::trackPointOnPlane(App* app, ThreadState& ts, u32 matrix, u32 words)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x3c);
    constexpr u32 kLocalTurned = 0x00, kLocalLocal = 0x24, kLocalBack = 0x30;

    const u32 frame = guest.frame();
    const u32 origin = words, point = words + 0x0c;
    for (u32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, guest.dword(point + 4 * i) - guest.dword(origin + 4 * i));
    fixedTransposeInto(guest, matrix, frame + kLocalTurned);
    fixedMatrixTimesInto(guest, point, frame + kLocalTurned, frame + kLocalLocal);
    guest.setDword(frame + kLocalLocal + 4, 0);
    fixedMatrixTimesInto(guest, frame + kLocalLocal, matrix, frame + kLocalBack);
    return guest.dword(frame + kLocalBack + 4) + guest.dword(origin + 4);
}

/* sub_496a50: a frame from a triangle's three points at eax: its middle point to the first,
 * the normal, to the third, each made one long, to edx; any of no length, the identity. */
nfs::u32 frontend::simd::frameOfTriangle(App* app, ThreadState& ts, u32 points, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20 + 0x28);
    constexpr u32 kIdentity = 0x564498;
    constexpr u32 kLocalNormal = 0x00, kLocalToThird = 0x0c, kLocalToFirst = 0x18;

    const u32 frame = guest.frame();
    const u32 toFirst = frame + kLocalToFirst, toThird = frame + kLocalToThird, normal = frame + kLocalNormal;
    for (u32 i = 0; i < 3; ++i)
        store(guest, toFirst + 4 * i, fp::sub(floatFrom(guest, points + 4 * i), floatFrom(guest, points + 0x0c + 4 * i)));
    for (u32 i = 0; i < 3; ++i)
        store(guest, toThird + 4 * i, fp::sub(floatFrom(guest, points + 0x18 + 4 * i), floatFrom(guest, points + 0x0c + 4 * i)));
    crossProduct(guest, toFirst, toThird, normal);
    for (const u32 vector : { toFirst, normal, toThird })
    {
        const float length = guest::direct<kNormalise, vertices::normaliseVector>(app, ts, vector);
        const u16 status = fp::compare(0.0, Float(length));
        if (jumpsEqual(status))
        {
            copyDwords(guest, out, kIdentity, 9);
            return withStatus(vector, status);  // eax as the callee leaves it: the vector
        }
    }
    copyDwords(guest, out, toFirst, 3);
    copyDwords(guest, out + 0x0c, normal, 3);
    copyDwords(guest, out + 0x18, toThird, 3);
    return out;
}

/* sub_4c9bc0: the place edx lanes across the slice eax, in floats to esi: the slice's centre,
 * out along its side by the lane's share (sub_4c9a80), by half the road's width at that side,
 * one way or the other as ebx says; put on the track (sub_420650). */
nfs::u32 frontend::simd::placeInLane(App* app, ThreadState& ts, u32 index, u32 lane, u32 away, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12 + 0xbc);
    constexpr u32 kSlices = 0x5dd948, kSliceBytes = 36;
    constexpr u32 kSliceSideBytes = 0x15, kSliceWidthNear = 0x17, kSliceWidthFar = 0x1b;
    constexpr s32 kFirstFarLane = 7;
    constexpr u32 kWidthUnit = 0x18000;
    constexpr u32 kScale = 0x540358;
    constexpr u32 kLocalRecord = 0x00, kLocalPoint = 0x3a + 0x56, kLocalFloats = 0x3a + 0x62;

    const u32 frame = guest.frame();
    const u32 offset = index * kSliceBytes;
    const u32 slice = guest.dword(kSlices) + offset;
    const u32 centre[3] = { guest.dword(slice), guest.dword(slice + 4), guest.dword(slice + 8) };
    u32 side[3];
    for (u32 i = 0; i < 3; ++i)
        side[i] = u32(s32(guest.dword(slice + kSliceSideBytes + i)) >> 24) << 9;

    const u32 share = fe::direct<0x4c9a80, tail::laneOffset>(app, ts, index, lane);
    const u32 point = frame + kLocalPoint;
    for (u32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, centre[i] + fixedTimes(share, side[i]));

    const u32 sliceNow = guest.dword(kSlices) + offset;
    const u32 width = u32(guest.byte(sliceNow + (s32(lane) < kFirstFarLane ? kSliceWidthNear : kSliceWidthFar))) << 15;
    s32 across = s32(fixedTimes(width, kWidthUnit));
    if (away == 0)
        across = s32(0u - u32(across));
    const u32 half = u32(across / 2);
    for (u32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, guest.dword(point + 4 * i) + fixedTimes(half, side[i]));

    fe::direct<0x4205a0, trackdata::snapPlace>(app, ts, point, frame + kLocalRecord, 1u, index);

    const Float scale = doubleAt(guest, kScale);
    const Float x = fp::mul(integerFrom(guest, point), scale);
    const Float y = fp::mul(integerFrom(guest, point + 4), scale);
    const Float z = fp::mul(scale, integerFrom(guest, point + 8));
    const u32 floats = frame + kLocalFloats;
    store(guest, floats, x);
    store(guest, floats + 4, y);
    store(guest, floats + 8, z);
    copyDwords(guest, out, floats, 3);
    return out;
}

namespace
{

// ---------------------------------------------------------------------------
// The race's list of tasks
// ---------------------------------------------------------------------------

/* A list: its most entries, its count, from +8 entries of 16 bytes, priority first, then the
 * function and its argument.  An unused one's priority is 0x7ffe. */
constexpr u32 kListCount = 4, kListEntries = 8, kEntryBytes = 16;
constexpr u32 kEntryPriority = 0, kEntryFunction = 4, kEntryArgument = 8;

}  // namespace

/* sub_4be1f0: a list for edx entries, from the game's memory. */
nfs::u32 frontend::simd::taskListNew(App* app, ThreadState& ts, u32 most)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    constexpr u32 kName = 0x5401b0, kHeaderBytes = 0x18, kUnused = 0x7ffe;

    const u32 list = *fe::direct<0x4e1620, ea::reserved>(app, ts, kName, (most << 4) + kHeaderBytes, 0);
    guest.setDword(list + kListCount, 0);
    guest.setDword(list, most);
    for (s32 i = 0; i < s32(guest.dword(list)); ++i)
    {
        const u32 entry = list + kListEntries + u32(i) * kEntryBytes;
        guest.setDword(entry + kEntryPriority, kUnused);
        guest.setDword(entry + kEntryFunction, 0);
    }
    return list;
}

/* sub_4be250: a function and its argument put in by priority, before the first of a greater
 * one, those after it moved up.  The registers left are the walk's. */
nfs::u32 frontend::simd::taskListAdd(App* app, ThreadState& ts, u32 list, u32 function, u32 argument, u32 priority)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12 + 0x14);
    const u32 count = guest.dword(list + kListCount) + 1;
    guest.setDword(list + kListCount, count);
    s32 slot = 0;
    for (s32 i = 0; i < s32(guest.dword(list + kListCount)); ++i)
    {
        slot = i;
        if (s32(priority) >= s32(guest.dword(list + kListEntries + u32(i) * kEntryBytes + kEntryPriority)))
            continue;
        s32 last = s32(guest.dword(list + kListCount) - 1);
        while (last > i)
        {
            copyDwords(guest, list + kListEntries + u32(last) * kEntryBytes, list + kListEntries + u32(last - 1) * kEntryBytes, 4);
            --last;
        }
        break;
    }
    const u32 entry = list + kListEntries + u32(slot) * kEntryBytes;
    guest.setDword(entry + kEntryPriority, priority);
    guest.setDword(entry + kEntryFunction, function);
    guest.setDword(entry + kEntryArgument, argument);
    return argument;
}

namespace
{

constexpr u32 kRaceTasksA = 0x7d3690, kRaceTasksB = 0x7d368c;

}  // namespace

/* sub_4c45f0: the race opened: its flags cleared; once only, its lists and all its parts set up. */
nfs::u32 frontend::simd::raceOpen(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16 + 0x80);
    constexpr u32 kFlagsOff[] = { 0x7d3694, 0x7d369c, 0x7d36a0, 0x7d36a8, 0x7d36ac, 0x7d36b0, 0x7d36a4, 0x7d3680, 0x7d3684 };
    constexpr u32 kFlagOn = 0x7d3698;
    constexpr u32 kCarLoadStep = 0x42ce70, kNameA = 0x5401ec, kNameB = 0x5401f4, kFormat = 0x5401fc, kGlyphsDirectory = 0x7a2934;
    constexpr u32 kAlreadyOpen = 0x55e88c;
    constexpr u32 kMostTasks = 0x90, kCarLoadPriority = 8;
    constexpr u32 kLocalPath = 0x00;

    for (const u32 flag : kFlagsOff)
        guest.setDword(flag, 0);
    guest.setDword(kFlagOn, 1);
    const u32 alive = fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    if (guest.dword(kAlreadyOpen) != 0)
        return alive;
    guest.setDword(kRaceTasksA, guest::direct<0x4be1f0, taskListNew>(app, ts, kMostTasks));
    guest.setDword(kRaceTasksB, guest::direct<0x4be1f0, taskListNew>(app, ts, kMostTasks));
    guest::direct<0x4be250, taskListAdd>(app, ts, guest.dword(kRaceTasksA), kCarLoadStep, 0, kCarLoadPriority);
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    const u32 frame = guest.frame();
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), frame + kLocalPath, kFormat, guest::Words{kGlyphsDirectory});
    fx::callAt(guest, 0x475240, args({ frame + kLocalPath, 0 }));
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    fe::direct<0x4371d0, collide::seedTheHash>(guest.app(), guest.ts(), gameSettings(guest).raceSeed);
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    fe::direct<0x42c830, raceload::carTasksAdded>(app, ts, 0u);
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    fe::direct<0x4711e0, controls::raceForcesStarted>(app, ts, 0u);
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    // Each called in turn, as the code calls them: those with a typed core as C++.
    for (const u32 step : { 0x404660u, 0x405e60u, 0x40e410u, 0x401220u, 0x40cb50u, 0x40c150u, 0x401320u, 0x408ad0u, 0x404c40u, 0x40a6f0u })
        fx::callAt(guest, step);
    fe::direct<0x493ef0, rendc::conesLoad>(app, ts);
    guest::direct<0x4c9d70, raceClearA>(app, ts);
    fe::direct<0x4cdd00, loopb::standingsStart>(app, ts);
    fe::direct<0x4129c0, port::raceSoundInitOf>(app, ts, ts.sp - 4);
    fe::direct<0x44b950, menua::racePauseMenuBegin>(app, ts);
    return guest::direct<0x4bc910, raceAddTasks>(app, ts);
}

/* sub_4c4740: the race started again: the HUD's pane put back if one was copied, its flags
 * cleared, all its parts told. */
nfs::u32 frontend::simd::raceRestart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20);
    constexpr u32 kPaneCopied = 0x7d3694, kPaneBytes = 0x1a4;
    constexpr u32 kFlagsOff[] = { 0x7d36a0, 0x7d36a8, 0x7d36ac, 0x7d36b0, 0x7d36a4, 0x7d3680, 0x7d3684, 0x7d369c };
    constexpr u32 kTickNow = 0x79c4ac, kTickAtStart = 0x7d3688;

    if (guest.dword(kPaneCopied) != 0)
    {
        // By address: it leaves edx, which the calls below hand on.
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, SETTINGS_ADDRESS(hud), SETTINGS_ADDRESS(hud[6]), kPaneBytes);
        guest.setDword(kPaneCopied, 0);
    }
    const u32 kind = gameSettings(guest).replayMode;
    for (const u32 flag : kFlagsOff)
        guest.setDword(flag, 0);
    if (kind == 0)
        gameSettings(guest).lastTickRaced = 0;
    fe::direct<0x4371d0, collide::seedTheHash>(guest.app(), guest.ts(), gameSettings(guest).raceSeed);
    // Each called in turn, as the code calls them: those with a typed core as C++.
    fe::direct<0x4b7c20, loada::copKeepLight>(app, ts);
    for (const u32 step : { 0x42c530u, 0x40e0f0u, 0x401230u, 0x40cb20u, 0x40c140u, 0x4012c0u, 0x408ae0u, 0x404c50u, 0x404650u,
                              0x403970u, 0x405fb0u })
        fx::callAt(guest, step);
    fe::direct<0x493e50, rendc::conesReset>(app, ts);
    guest::direct<0x4c9db0, raceClearB>(app, ts);
    guest::direct<0x4bc8b0, sime::effectsReset>(app, ts);
    fe::direct<0x4cdd00, loopb::standingsStart>(app, ts);
    fe::direct<0x471000, controls::forcesStoppedOrActuatorsOn>(app, ts, 0u);
    const u32 tick = guest.dword(kTickNow);
    guest.setDword(kTickAtStart, tick);
    return tick;
}

/* sub_4c7bd0: on a network, the frame's packet: 0xb, this player, the frame, a stamp, its
 * controls and every car's last; or, a game that does not send all its cars, those kept by
 * frame in a ring of 20 under the lock.  A player past the race's end is told on. */
nfs::u32 frontend::simd::netSendPlace(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x20);
    constexpr u32 kThisPlayer = 0x7a2276, kSendsAll = 0x7a22da;
    constexpr u32 kNetCars = 0x5efb88, kCarStamp = 8;
    constexpr u32 kLock = 0x560114;
    constexpr u32 kRingFrames = 0x7dd120, kRingCars = 0x7dd110, kRingBytes = 20;
    constexpr s32 kRingLength = 20;
    constexpr u32 kPacketType = 0x0b, kWholePacket = 0x1c, kShortPacket = 0x0e, kMostCars = 7;
    constexpr u32 kLocalPacket = 0x00, kLocalStamp = 0x08, kLocalCars = 0x0c, kLocalControls = 0x1c;
    constexpr u32 kFinished = 3;

    if (s32(gameSettings(guest).replayMode) > 1)
        return eax;
    const u32 players = gameSettings(guest).playMode;
    if (players == 1 || players == 0 || gameSettings(guest).networkGame == 0)
        return eax;

    const u32 frame = guest.frame();
    const u32 packet = frame + kLocalPacket;
    guest.setByte(packet, kPacketType);
    guest.setByte(packet + 1, guest.byte(kThisPlayer + 2));
    guest.setDword(packet + 4, eax);
    fe::direct<0x4c8310, neta::packControls>(app, ts, frame + kLocalStamp);
    const u32 player = u32(s32(guest.dword(kThisPlayer)) >> 16);
    fe::direct<0x4c6840, controls::controlsCopied>(app, ts, player, frame + kLocalControls);
    guest.setByte(frame + kLocalCars, guest.byte(frame + kLocalControls + 3));

    u32 bytes;
    if (guest.word(kSendsAll) != 0)
    {
        for (s32 i = 0; i < s32(gameSettings(guest).opponentCount); ++i)
        {
            const u32 car = guest.dword(kNetCars + u32(i) * 4);
            guest.setWord(frame + kLocalCars + 2 * u32(i + 1), guest.word(car + kCarStamp));
        }
        bytes = kWholePacket - 2 * (kMostCars - gameSettings(guest).opponentCount);
    }
    else
    {
        const u32 slot = u32((s32(eax) >> 5) % kRingLength);
        fe::enterLock(guest, ts, guest.dword(kLock));
        guest.setDword(kRingFrames + slot * kRingBytes, eax);
        for (s32 i = 0; i < s32(gameSettings(guest).opponentCount); ++i)
        {
            const u32 car = guest.dword(kNetCars + u32(i) * 4);
            guest.setWord(kRingCars + slot * kRingBytes + 2 * u32(i), guest.word(car + kCarStamp));
        }
        fe::leaveLock(guest, ts, guest.dword(kLock));
        bytes = kShortPacket;
    }
    const ea::Regs sent = *fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, bytes);
    const u32 car = guest.dword(kCarsByPlace + gameSettings(guest).localPlayer * 4);
    if (guest.dword(car + kCarState) != kFinished)
        return car;
    return fe::direct<0x4c7e50, neta::sendPlayerGone>(app, ts, gameSettings(guest).localPlayer);
}

/* sub_485880: the HUD's six panes to their defaults, after the screen's own (sub_485680). */
nfs::u32 frontend::simd::hudDefaults(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20);
    constexpr u32 kPaneBytes = 0x1a4;
    constexpr u32 kDriverType = 0x7a3a58;
    constexpr std::uint8_t kDriverFlagTwo = 0x02;
    constexpr u32 kFourBits = 0x40800000, kOneBits = 0x3f800000;

    fe::direct<0x485680, frontend::hudedit::loadDefaults>(app, ts);
    const u32 one = 1;
    gameSettings(guest).cameraMode[0] = one;
    gameSettings(guest).cameraMode[1] = one;
    const bool flagTwo = (guest.byte(kDriverType) & kDriverFlagTwo) != 0;
    const u32 style = gameSettings(guest).language != 0 ? 3 : 1;
    for (u32 i = 0; i < 6; ++i)
    {
        HudRecord& pane = gameSettings(guest).hud[i];
        const bool odd = (i & 1) != 0;
        pane.shown = one;
        pane.opponentName = i == 0 ? (flagTwo ? 1 : 0) : odd ? 0 : one;
        const u32 firstTwo = i < 2 && !flagTwo ? 1 : 0;
        pane.opponentList = firstTwo;
        pane.rearView = firstTwo;
        pane.mapScaleBits = odd ? kOneBits : kFourBits;
        pane.splitTimeForm = 3;
        pane.speedometerForm = style;
        pane.tachometerForm = 1;
        pane.mapStyle = odd ? 2 : 1;
    }
    return 2;
}

namespace
{

/* The nine floats a turn about an axis reads: its own if it writes elsewhere, a copy in the
 * frame if it turns one in place. */
u32 turnSource(Guest& guest, u32 matrix, u32 out)
{
    if (matrix != out)
        return matrix;
    copyDwords(guest, guest.frame(), out, 9);
    return guest.frame();
}

constexpr u32 kTurnFrame = 24 + 0x2c;

}  // namespace

/* sub_4c9d70, sub_4c9db0: the 0x280 bytes at 0x7dd850 cleared, as a race opens and starts again. */
nfs::u32 frontend::simd::raceClearB(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kBlock = 0x7dd850, kBytes = 0x280, kFillZero = 0x4e070c;
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kBlock, kBytes);
}

// ---------------------------------------------------------------------------
// The race cameras
// ---------------------------------------------------------------------------

nfs::u32 frontend::simCameraChase(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4243a0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraChase(app, ts, camera); });
}

nfs::u32 frontend::simCameraOrbit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424690", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraOrbit(app, ts, camera); });
}

nfs::u32 frontend::simCameraChaseLow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424740", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraChaseLow(app, ts, camera); });
}

nfs::u32 frontend::simCameraCircleCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424b50", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraCircleCar(app, ts, camera); });
}

nfs::u32 frontend::simCameraLookAround(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424ce0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraLookAround(app, ts, camera); });
}

nfs::u32 frontend::simCameraSideOfCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424e30", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraSideOfCar(app, ts, camera); });
}

nfs::u32 frontend::simCameraEaseTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424f30", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraEaseTurn(app, ts, camera); });
}

nfs::u32 frontend::simCameraMoveTowards(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_424fd0", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 camera = args.eax, spot = args.edx, near = guest::at(app, ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraMoveTowards(app, ts, camera, spot, near); });
}

nfs::u32 frontend::simCameraBesideCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425990", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraBesideCar(app, ts, camera); });
}

nfs::u32 frontend::simCameraOnCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425ab0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraOnCar(app, ts, camera); });
}

nfs::u32 frontend::simCameraGlideTowards(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425b40", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 camera = args.eax, spot = args.edx;
    const u32 base = guest::at(app, ts.sp + 4), rate = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraGlideTowards(app, ts, camera, spot, base, rate); });
}

nfs::u32 frontend::simCameraPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426370", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 camera = args.eax, view = args.edx, mode = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cameraPrepare(app, ts, camera, view, mode); });
}

// ---------------------------------------------------------------------------
// The shared maths
// ---------------------------------------------------------------------------

/* sub_4e0090: the length of edx less eax (sub_4e06a0), on the x87. */
nfs::u32 frontend::simd::vectorDistance(App* app, ThreadState& ts, u32 from, u32 to, double& length)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    const u32 difference = guest.frame();
    for (u32 i = 0; i < 3; ++i)
        store(guest, difference + 4 * i, fp::sub(floatFrom(guest, to + 4 * i), floatFrom(guest, from + 4 * i)));
    length = fe::direct<0x4e06a0, vertices::vectorLength>(app, ts, difference);
    return difference;  // the eax it leaves
}

nfs::u32 frontend::simVectorDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e0090", 0, 0, nullptr, 0, kRestoreNone };
    const u32 from = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        double length = 0.0;
        const u32 difference = simd::vectorDistance(app, ts, from, to, length);
        args.floatSet = true;
        args.floatResult = length;
        return difference;
    });
}

/* sub_4e0110: a matrix turned over, eax to edx, which may be the same: the order of the
 * moves is what lets it. */
nfs::u32 frontend::simd::transpose(App* app, ThreadState& ts, u32 from, u32 to)
{
    Guest guest(app, ts);
    guest.setDword(to, guest.dword(from));
    guest.setDword(to + 0x10, guest.dword(from + 0x10));
    guest.setDword(to + 0x20, guest.dword(from + 0x20));
    const float a = guest.floatAt(from + 0x0c);
    const u32 aBack = guest.dword(from + 0x04);
    guest.setFloat(to + 0x04, a);
    guest.setDword(to + 0x0c, aBack);
    const float b = guest.floatAt(from + 0x18);
    const u32 bBack = guest.dword(from + 0x08);
    guest.setFloat(to + 0x08, b);
    guest.setDword(to + 0x18, bBack);
    const float c = guest.floatAt(from + 0x1c);
    const u32 cBack = guest.dword(from + 0x14);
    guest.setFloat(to + 0x14, c);
    guest.setDword(to + 0x1c, cBack);
    return from;
}

nfs::u32 frontend::simTranspose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e0110", 0, 0, nullptr, 0 };
    const u32 from = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::transpose(app, ts, from, to); });
}

/* sub_4e0160: a by b, to ebx. */
nfs::u32 frontend::simd::cross(App* app, ThreadState& ts, u32 a, u32 b, u32 out)
{
    Guest guest(app, ts);
    crossProduct(guest, a, b, out);
    return a + 0x0c;
}

nfs::u32 frontend::simCross(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e0160", 0, 0, nullptr, 0, kRestoreNone };
    const u32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::cross(app, ts, a, b, out); });
}

/* sub_4e01a0: the cross product made one long (sub_4e01c0), to ebx. */
nfs::u32 frontend::simd::unitCross(App* app, ThreadState& ts, u32 a, u32 b, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    const u32 cross = guest.frame();
    crossProduct(guest, a, b, cross);
    fe::direct<0x4e01c0, vertices::makeUnitVector>(app, ts, cross, out);
    return cross;  // the eax it leaves
}

nfs::u32 frontend::simUnitCross(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e01a0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::unitCross(app, ts, a, b, out); });
}

/* sub_4ea930, sub_4ea950: the sine, the cosine, of the stack's float, on the x87. */
double frontend::simd::sineValue(App* app, u32 angle)
{
    constexpr u32 kSineUnit = 0x54b6b0;
    return fp::sin(fp::mul(Float(floatOf(angle)), Float(app->getMemory<double>(kSineUnit))));
}

double frontend::simd::cosineValue(App* app, u32 angle)
{
    constexpr u32 kCosineUnit = 0x54b6b8;
    return fp::cos(fp::mul(Float(floatOf(angle)), Float(app->getMemory<double>(kCosineUnit))));
}

nfs::u32 frontend::simSine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea930", 4, 0, nullptr, 0 };
    const u32 eax = args.eax, angle = guest::at(app, ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = simd::sineValue(app, angle);
        return eax;
    });
}

nfs::u32 frontend::simCosine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea950", 4, 0, nullptr, 0 };
    const u32 eax = args.eax, angle = guest::at(app, ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = simd::cosineValue(app, angle);
        return eax;
    });
}

/* sub_4ea970: the sine and the cosine of the float at eax, to edx and ebx. */
nfs::u32 frontend::simd::sineCosine(App* app, ThreadState& ts, u32 angleAt, u32 sineAt, u32 cosineAt)
{
    Guest guest(app, ts);
    sineCosineInto(guest, angleAt, sineAt, cosineAt);
    return angleAt;
}

nfs::u32 frontend::simSineCosine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea970", 0, 0, nullptr, 0 };
    const u32 angleAt = args.eax, sineAt = args.edx, cosineAt = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::sineCosine(app, ts, angleAt, sineAt, cosineAt); });
}

/* sub_4ea9c0: the angle of the stack's two floats in the game's units, on the x87. */
double frontend::simd::angleOfFloatsValue(App* app, ThreadState& ts, u32 first, u32 second)
{
    constexpr u32 kUnitsPerRadian = 0x54b6c0;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    nfs::Args arc;
    arc.floatIn[1] = floatOf(second);
    arc.floatIn[0] = floatOf(first);
    arc.floatsIn = 2;
    fx::callAt(guest, kArcTangent, fe::Arguments(), &arc);
    return fp::mul(arc.floatResult, doubleAt(guest, kUnitsPerRadian));
}

nfs::u32 frontend::simAngleOfFloats(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea9c0", 8, 0, nullptr, 0 };
    const u32 eax = args.eax, first = guest::at(app, ts.sp + 4), second = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = simd::angleOfFloatsValue(app, ts, first, second);
        return eax;
    });
}

/* sub_4ea9e0: the matrix at eax turned by the stack's angle about its first row, to the
 * stack's second. */
nfs::u32 frontend::simd::rotateX(App* app, ThreadState& ts, u32 matrix, u32 angleBits, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, kTurnFrame);
    const u32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    guest.setDword(out, guest.dword(m));
    guest.setDword(out + 4, guest.dword(m + 4));
    const u32 result = guest.dword(m + 8);
    guest.setDword(out + 8, result);
    for (u32 i = 0; i < 3; ++i)
        store(guest, out + 0x0c + 4 * i, fp::add(fp::mul(floatFrom(guest, m + 0x0c + 4 * i), c), fp::mul(floatFrom(guest, m + 0x18 + 4 * i), s)));
    for (u32 i = 0; i < 3; ++i)
        store(guest, out + 0x18 + 4 * i, fp::sub(fp::mul(floatFrom(guest, m + 0x18 + 4 * i), c), fp::mul(floatFrom(guest, m + 0x0c + 4 * i), s)));
    return result;
}

/* sub_4eaab0: about its second row. */
nfs::u32 frontend::simd::rotateY(App* app, ThreadState& ts, u32 matrix, u32 angleBits, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, kTurnFrame);
    const u32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    store(guest, out, fp::sub(fp::mul(floatFrom(guest, m), c), fp::mul(floatFrom(guest, m + 0x18), s)));
    store(guest, out + 4, fp::sub(fp::mul(floatFrom(guest, m + 4), c), fp::mul(floatFrom(guest, m + 0x1c), s)));
    store(guest, out + 8, fp::sub(fp::mul(floatFrom(guest, m + 8), c), fp::mul(floatFrom(guest, m + 0x20), s)));
    guest.setDword(out + 0x0c, guest.dword(m + 0x0c));
    guest.setDword(out + 0x10, guest.dword(m + 0x10));
    const u32 result = guest.dword(m + 0x14);
    guest.setDword(out + 0x14, result);
    store(guest, out + 0x18, fp::add(fp::mul(floatFrom(guest, m + 0x18), c), fp::mul(floatFrom(guest, m), s)));
    store(guest, out + 0x1c, fp::add(fp::mul(floatFrom(guest, m + 4), s), fp::mul(floatFrom(guest, m + 0x1c), c)));
    store(guest, out + 0x20, fp::add(fp::mul(floatFrom(guest, m + 8), s), fp::mul(floatFrom(guest, m + 0x20), c)));
    return result;
}

/* sub_4eab80: about its third row. */
nfs::u32 frontend::simd::rotateZ(App* app, ThreadState& ts, u32 matrix, u32 angleBits, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, kTurnFrame);
    const u32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    store(guest, out, fp::add(fp::mul(floatFrom(guest, m + 0x0c), s), fp::mul(floatFrom(guest, m), c)));
    store(guest, out + 4, fp::add(fp::mul(floatFrom(guest, m + 4), c), fp::mul(floatFrom(guest, m + 0x10), s)));
    store(guest, out + 8, fp::add(fp::mul(floatFrom(guest, m + 8), c), fp::mul(floatFrom(guest, m + 0x14), s)));
    for (u32 i = 0; i < 3; ++i)
        store(guest, out + 0x0c + 4 * i, fp::sub(fp::mul(floatFrom(guest, m + 0x0c + 4 * i), c), fp::mul(floatFrom(guest, m + 4 * i), s)));
    guest.setDword(out + 0x18, guest.dword(m + 0x18));
    guest.setDword(out + 0x1c, guest.dword(m + 0x1c));
    const u32 result = guest.dword(m + 0x20);
    guest.setDword(out + 0x20, result);
    return result;
}

nfs::u32 frontend::simRotateX(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea9e0", 8, 0, nullptr, 0 };
    const u32 matrix = args.eax, angle = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::rotateX(app, ts, matrix, angle, out); });
}

nfs::u32 frontend::simRotateY(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4eaab0", 8, 0, nullptr, 0 };
    const u32 matrix = args.eax, angle = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::rotateY(app, ts, matrix, angle, out); });
}

nfs::u32 frontend::simRotateZ(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4eab80", 8, 0, nullptr, 0 };
    const u32 matrix = args.eax, angle = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::rotateZ(app, ts, matrix, angle, out); });
}

nfs::u32 frontend::simFrameOfTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_496a50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 points = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::frameOfTriangle(app, ts, points, out); });
}

nfs::u32 frontend::simFixedNormalise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4970c0", 0, 0, nullptr, 0 };
    const u32 vector = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::fixedNormalise(app, ts, vector); });
}

/* sub_4f44e0: nine ints turned over, eax to edx; the two the same, in place. */
nfs::u32 frontend::simd::fixedTranspose(App* app, ThreadState& ts, u32 from, u32 to)
{
    Guest guest(app, ts);
    if (from != to)
    {
        fixedTransposeInto(guest, from, to);
        return guest.dword(from + 0x20);
    }
    u32 swapped = 0;
    for (const auto& pair : { std::array<u32, 2>{ 0x04, 0x0c }, std::array<u32, 2>{ 0x08, 0x18 }, std::array<u32, 2>{ 0x14, 0x1c } })
    {
        const u32 first = guest.dword(from + pair[0]);
        swapped = guest.dword(from + pair[1]);
        guest.setDword(from + pair[1], first);
        guest.setDword(from + pair[0], swapped);
    }
    return from;
}

nfs::u32 frontend::simFixedTranspose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f44e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::fixedTranspose(app, ts, from, to); });
}

/* sub_4f4544: three 16.16 ints by nine, to ebx; edx the last sum's high word. */
nfs::u32 frontend::simd::fixedMatrixTimes(App* app, ThreadState& ts, u32 a, u32 m, u32 out)
{
    Guest guest(app, ts);
    const FixedSum left = fixedMatrixTimesInto(guest, a, m, out);
    return left.eax;
}

nfs::u32 frontend::simFixedMatrixTimes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4544", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 a = args.eax, m = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::fixedMatrixTimes(app, ts, a, m, out); });
}

nfs::u32 frontend::simTrackPointOnPlane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4969c0", 24, 0, nullptr, 0 };
    const u32 matrix = args.eax, words = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::trackPointOnPlane(app, ts, matrix, words); });
}

nfs::u32 frontend::simPlaceInLane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9bc0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdi };
    const u32 index = args.eax, lane = args.edx, away = args.ebx, out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::placeInLane(app, ts, index, lane, away, out); });
}

// ---------------------------------------------------------------------------
// The race's list of tasks
// ---------------------------------------------------------------------------

nfs::u32 frontend::simTaskListNew(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4be1f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 most = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::taskListNew(app, ts, most); });
}

nfs::u32 frontend::simTaskListAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4be250", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 list = args.eax, function = args.edx, argument = args.ebx, priority = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::taskListAdd(app, ts, list, function, argument, priority); });
}

/* sub_4bc910: the cameras' step and the HUD's, on the race's second list, at 100. */
nfs::u32 frontend::simd::raceAddTasks(App* app, ThreadState& ts)
{
    constexpr u32 kCamerasStep = 0x426680, kHudStep = 0x4cb2c0, kPriority = 0x64;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    guest::direct<0x4be250, taskListAdd>(app, ts, guest.dword(kRaceTasksB), kCamerasStep, 0, kPriority);
    return guest::direct<0x4be250, taskListAdd>(app, ts, guest.dword(kRaceTasksB), kHudStep, 0, kPriority);
}

nfs::u32 frontend::simRaceAddTasks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc910", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceAddTasks(app, ts); });
}

/* sub_4bc8e0: out of the race's frame: its end, the game kept alive, a flag down. */
nfs::u32 frontend::simd::raceLeaveTasks(App* app, ThreadState& ts)
{
    constexpr u32 kDrawing = 0x7a3d10;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    guest::direct<0x4d5720, simc::raceFrameEnd>(app, ts);
    // The game's code writes edx, zeroed before the call, which keeps it.
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    guest.setDword(kDrawing, 0);
    fe::direct<0x4a33d0, loada::stopCheck>(app, ts);
    return fe::direct<0x4bbd70, rendd::arenaRewind>(app, ts);
}

nfs::u32 frontend::simRaceLeaveTasks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc8e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceLeaveTasks(app, ts); });
}

nfs::u32 frontend::simRaceOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c45f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceOpen(app, ts); });
}

nfs::u32 frontend::simRaceRestart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4740", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceRestart(app, ts); });
}

nfs::u32 frontend::simNetSendPlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7bd0", 0, 0, nullptr, 0 };
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::netSendPlace(app, ts, eax); });
}

nfs::u32 frontend::simHudDefaults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_485880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::hudDefaults(app, ts); });
}

// ---------------------------------------------------------------------------
// One-line calls
// ---------------------------------------------------------------------------

/* sub_439cc0: whether a car is a locked bonus car: none without its record. */
nfs::u32 frontend::simd::bonusCarLocked(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 record = fe::direct<0x439a50, load::recordAt>(app, ts, car);
    // The record in eax, as the call before left it there.
    return record != 0 ? fe::direct<0x439c10, load::bonusLocked>(app, ts, record) : record;
}

nfs::u32 frontend::simBonusCarLocked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_439cc0", 0, 0, nullptr, 0, kRestoreNone };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::bonusCarLocked(app, ts, car); });
}

/* sub_49bfb0: eax's bits added to the flags at 0x55e5ec. */
nfs::u32 frontend::simd::orFlags(App* app, ThreadState& ts, u32 bits)
{
    constexpr u32 kFlags = 0x55e5ec;
    Guest guest(app, ts);
    guest.setDword(kFlags, guest.dword(kFlags) | bits);
    return bits;
}

nfs::u32 frontend::simOrFlags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bfb0", 0, 0, nullptr, 0 };
    const u32 bits = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::orFlags(app, ts, bits); });
}

/* sub_4a4020: as the game ends, the web page the choice at 0x55e894 asks for. */
nfs::u32 frontend::simd::openWebPage(App* app, ThreadState& ts)
{
    constexpr u32 kChoice = 0x55e894, kShellOpen = 0x418cc0, kHomePage = 0x53ce9c, kDownloadPage = 0x53ceb8;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 choice = guest.dword(kChoice);
    if (choice == 1)
        fe::direct<kShellOpen, track::openWithShell>(app, ts, kHomePage);
        return 1;
    if (choice == 2)
        fe::direct<kShellOpen, track::openWithShell>(app, ts, kDownloadPage);
        return 1;
    return choice;
}

nfs::u32 frontend::simOpenWebPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4020", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::openWebPage(app, ts); });
}

/* sub_4bebb0: the clip rectangle kept and set (sub_4bebe0, handed the four as they came). */
nfs::u32 frontend::simd::setClipRect(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom)
{
    constexpr u32 kKept = 0x7cda60, kSetClip = 0x4bebe0;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    guest.setDword(kKept, left);
    guest.setDword(kKept + 4, top);
    guest.setDword(kKept + 8, right);
    guest.setDword(kKept + 0x0c, bottom);
    return *fe::direct<0x4bebe0, graphics::clipSet>(app, ts, left, top, right, bottom);
}

nfs::u32 frontend::simSetClipRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bebb0", 0, 0, nullptr, 0, kRestoreNone };
    const u32 left = args.eax, top = args.edx, right = args.ebx, bottom = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::setClipRect(app, ts, left, top, right, bottom); });
}

/* sub_4c7b10: under another name, the depth's offsets set (sub_4c7a60), every register as it came. */
nfs::u32 frontend::simd::depthOffsets(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return fx::callAt(guest, 0x4c7a60);
}

nfs::u32 frontend::simDepthOffsets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7b10", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::depthOffsets(app, ts); });
}

nfs::u32 frontend::simd::raceClearA(App* app, ThreadState& ts)
{
    constexpr u32 kBlock = 0x7dd850, kBytes = 0x280, kFillZero = 0x4e070c;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kBlock, kBytes);
}

nfs::u32 frontend::simRaceClearA(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9d70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceClearA(app, ts); });
}

nfs::u32 frontend::simRaceClearB(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9db0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::raceClearB(app, ts); });
}

/* sub_4dbda0: the projection of a view (sub_4bed20): the view's own half of the screen with
 * two players, else the whole screen's. */
nfs::u32 frontend::simd::viewProjection(App* app, ThreadState& ts, u32 camera)
{
    constexpr u32 kViews = 0x7cda80, kWholeScreen = 0x7cdb88, kHalfAngle = 0x78;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    const u32 view = gameSettings(guest).playMode == 1 ? (guest.dword(camera + 4) << 5) + kViews : kWholeScreen;
    return fe::direct<0x4bed20, loopb::useWithProjection>(app, ts, view, kHalfAngle, gameSettings(guest).mirrored);
}

nfs::u32 frontend::simViewProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dbda0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::viewProjection(app, ts, camera); });
}

/* sub_4dcd10: under another name, the weather's step (sub_4dcdf0), every register as it came. */
nfs::u32 frontend::simd::weatherStep(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return fe::direct<0x4dcdf0, tail::weatherStart>(app, ts);
}

nfs::u32 frontend::simWeatherStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dcd10", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::weatherStep(app, ts); });
}

/* sub_4dcdd0: how hard it rains, none without the weather on. */
nfs::u32 frontend::simd::rainAmount(App* app, ThreadState& ts)
{
    constexpr u32 kRain = 0x8ca22c;
    Guest guest(app, ts);
    return gameSettings(guest).weather != 0 ? guest.dword(kRain) : 0;
}

nfs::u32 frontend::simRainAmount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dcdd0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simd::rainAmount(app, ts); });
}

}
