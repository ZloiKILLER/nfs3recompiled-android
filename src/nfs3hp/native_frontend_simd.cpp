#include "native_frontend_simd.h"
#include "game_settings.h"
#include "native_frontend.h"
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
#include "native_load_cardb.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_windows.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <winapi/kernel32.h>
#include <array>
#include <cstring>
#include <optional>

/* The small pieces of the simulation still generated, decompiled:
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
void simCameraChase(win32::WinApplication* app, x86::CPU& cpu);
void simCameraOrbit(win32::WinApplication* app, x86::CPU& cpu);
void simCameraChaseLow(win32::WinApplication* app, x86::CPU& cpu);
void simCameraCircleCar(win32::WinApplication* app, x86::CPU& cpu);
void simCameraLookAround(win32::WinApplication* app, x86::CPU& cpu);
void simCameraSideOfCar(win32::WinApplication* app, x86::CPU& cpu);
void simCameraEaseTurn(win32::WinApplication* app, x86::CPU& cpu);
void simCameraMoveTowards(win32::WinApplication* app, x86::CPU& cpu);
void simCameraBesideCar(win32::WinApplication* app, x86::CPU& cpu);
void simCameraOnCar(win32::WinApplication* app, x86::CPU& cpu);
void simCameraGlideTowards(win32::WinApplication* app, x86::CPU& cpu);
void simCameraPrepare(win32::WinApplication* app, x86::CPU& cpu);
void simBonusCarLocked(win32::WinApplication* app, x86::CPU& cpu);
void simHudDefaults(win32::WinApplication* app, x86::CPU& cpu);
void simTrackPointOnPlane(win32::WinApplication* app, x86::CPU& cpu);
void simFrameOfTriangle(win32::WinApplication* app, x86::CPU& cpu);
void simFixedNormalise(win32::WinApplication* app, x86::CPU& cpu);
void simOrFlags(win32::WinApplication* app, x86::CPU& cpu);
void simOpenWebPage(win32::WinApplication* app, x86::CPU& cpu);
void simRaceLeaveTasks(win32::WinApplication* app, x86::CPU& cpu);
void simRaceAddTasks(win32::WinApplication* app, x86::CPU& cpu);
void simTaskListNew(win32::WinApplication* app, x86::CPU& cpu);
void simTaskListAdd(win32::WinApplication* app, x86::CPU& cpu);
void simSetClipRect(win32::WinApplication* app, x86::CPU& cpu);
void simRaceOpen(win32::WinApplication* app, x86::CPU& cpu);
void simRaceRestart(win32::WinApplication* app, x86::CPU& cpu);
void simDepthOffsets(win32::WinApplication* app, x86::CPU& cpu);
void simNetSendPlace(win32::WinApplication* app, x86::CPU& cpu);
void simPlaceInLane(win32::WinApplication* app, x86::CPU& cpu);
void simRaceClearA(win32::WinApplication* app, x86::CPU& cpu);
void simRaceClearB(win32::WinApplication* app, x86::CPU& cpu);
void simViewProjection(win32::WinApplication* app, x86::CPU& cpu);
void simWeatherStep(win32::WinApplication* app, x86::CPU& cpu);
void simRainAmount(win32::WinApplication* app, x86::CPU& cpu);
void simVectorDistance(win32::WinApplication* app, x86::CPU& cpu);
void simTranspose(win32::WinApplication* app, x86::CPU& cpu);
void simCross(win32::WinApplication* app, x86::CPU& cpu);
void simUnitCross(win32::WinApplication* app, x86::CPU& cpu);
void simSine(win32::WinApplication* app, x86::CPU& cpu);
void simCosine(win32::WinApplication* app, x86::CPU& cpu);
void simSineCosine(win32::WinApplication* app, x86::CPU& cpu);
void simAngleOfFloats(win32::WinApplication* app, x86::CPU& cpu);
void simRotateX(win32::WinApplication* app, x86::CPU& cpu);
void simRotateY(win32::WinApplication* app, x86::CPU& cpu);
void simRotateZ(win32::WinApplication* app, x86::CPU& cpu);
void simFixedTranspose(win32::WinApplication* app, x86::CPU& cpu);
void simFixedMatrixTimes(win32::WinApplication* app, x86::CPU& cpu);
}

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg32;

constexpr std::uint8_t kRestoreNone = 0;

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its import's slot.  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered
bool jumpsEqual(reg16 status) { return (status & 0x4000) != 0; }  // je; true unordered

/* eax after fnstsw ax: only its low half changes. */
reg32 withStatus(reg32 eax, reg16 status) { return (eax & 0xffff0000) | status; }

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, reg32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, reg32 address) { return Float(sreg32(guest.dword(address))); }

/* fstp dword */
void store(Guest& guest, reg32 address, Float value) { guest.setFloat(address, float(value)); }

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The game's functions, by address.
constexpr reg32 kAddVectors = 0x4dffb0;
constexpr reg32 kNormalise = 0x4972f0, kUnitVector = 0x4e01c0, kVectorLength = 0x4e06a0, kArcTangent = 0x4e06d9;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kScaledIfSet = 0x423fb0, kDampVector = 0x424030;
constexpr reg32 kPlaceOnTrack = 0x420650;

// A car.
constexpr reg32 kCarFlags = 0x200;              // byte
constexpr reg32 kCarPlace = 0x98;               // three floats
constexpr reg32 kCarAxes = 0xc0;                // nine floats
constexpr reg32 kCarGrip = 0x10c;               // a float, compared by its bits
constexpr reg32 kCarLift = 0x118, kCarBack = 0x11c;
constexpr reg32 kCarDriving = 0x528;
constexpr reg32 kCarWheelsOff = 0x998;
constexpr reg32 kCarTowed = 0x9a4;              // what a flagged car rides on
constexpr reg32 kCarState = 0x2c4;
constexpr reg32 kLeastGripBits = 0x3f34fdf4;    // below it the car is off the ground: compared as ints

// A race camera.
constexpr reg32 kCameraCar = 0x04, kCameraTargetCar = 0x08, kCameraView = 0x0c;
constexpr reg32 kCameraPlace = 0x14, kCameraTarget = 0x20;
constexpr reg32 kCameraZoom = 0x54;
constexpr reg32 kCameraNode = 0x84;
constexpr reg32 kCameraWasAirborne = 0xfc;
constexpr reg32 kCameraAirWait = 0x10c;         // frames left to wait
constexpr reg32 kCameraOwn110 = 0x110, kCameraOwn114 = 0x114, kCameraOwn118 = 0x118, kCameraOwn11c = 0x11c;
constexpr reg32 kCameraOwn120 = 0x120, kCameraOwn124 = 0x124, kCameraOwn128 = 0x128;

// The race.
constexpr reg32 kStartShown = 0x7d36a8, kStarted = 0x7cd960, kFrozen = 0x5600ec;
constexpr reg32 kLookingBack = 0x79c4a0;        // reg32[2]: each view's
constexpr reg32 kViewHoldsNoTarget = 0x5e1010;  // [view * 8]: nonzero, the chase's target jumps
constexpr reg32 kViewInTunnel = 0x5e1014;       // [view * 8]: frames the view's been in a tunnel
constexpr reg32 kCarsByPlace = 0x5efac8;

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
    constexpr reg32 kSineUnit = 0x54b6b0;
    return fp::sin(fp::mul(Float(angle), doubleAt(guest, kSineUnit)));
}

Float cosineOf(const Guest& guest, float angle)
{
    constexpr reg32 kCosineUnit = 0x54b6b8;
    return fp::cos(fp::mul(Float(angle), doubleAt(guest, kCosineUnit)));
}

/* sub_4ea970: sine to `sineAt`, then cosine to `cosineAt`, of the float at `angleAt` read
 * each time: the sine's place may be the angle's. */
void sineCosineInto(Guest& guest, reg32 angleAt, reg32 sineAt, reg32 cosineAt)
{
    store(guest, sineAt, sineOf(guest, guest.floatAt(angleAt)));
    store(guest, cosineAt, cosineOf(guest, guest.floatAt(angleAt)));
}

// ---------------------------------------------------------------------------
// Vectors and matrices
// ---------------------------------------------------------------------------

/* The three floats at `v` by the nine at `m`, to `out`: each row's three products, then
 * their sums, the first row first, as all the cameras inline it. */
void vectorByMatrix(Guest& guest, reg32 v, reg32 m, reg32 out)
{
    Float sum[3];
    for (reg32 axis = 0; axis < 3; ++axis)
        sum[axis] = fp::mul(floatFrom(guest, v), floatFrom(guest, m + 4 * axis));
    for (reg32 row = 1; row < 3; ++row)
        for (reg32 axis = 0; axis < 3; ++axis)
            sum[axis] = fp::add(sum[axis], fp::mul(floatFrom(guest, v + 4 * row), floatFrom(guest, m + 12 * row + 4 * axis)));
    for (reg32 axis = 0; axis < 3; ++axis)
        store(guest, out + 4 * axis, sum[axis]);
}

/* sub_4e0160: a by b, to `out`, which may be either. */
void crossProduct(Guest& guest, reg32 a, reg32 b, reg32 out)
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
reg32 fixedTimes(reg32 a, reg32 b)
{
    const std::uint64_t product = std::uint64_t(std::int64_t(sreg32(a)) * std::int64_t(sreg32(b)));
    const reg32 low = reg32(product), high = reg32(product >> 32);
    return (high << 16) + (low >> 16) + ((low >> 15) & 1);
}

/* sub_4f4544: the three 16.16 ints at `a` by the nine at `m`, to `out`, a column at a time, a
 * 64-bit sum for each, read after the last was written: `out` may be either.  The last sum
 * is what eax and edx are left with. */
struct FixedSum
{
    reg32 eax, edx;
};

FixedSum fixedMatrixTimesInto(Guest& guest, reg32 a, reg32 m, reg32 out)
{
    FixedSum left{ 0, 0 };
    for (reg32 column = 0; column < 3; ++column)
    {
        std::uint64_t sum = 0;
        for (reg32 row = 0; row < 3; ++row)
            sum += std::uint64_t(std::int64_t(sreg32(guest.dword(a + 4 * row)))
                                 * std::int64_t(sreg32(guest.dword(m + 12 * row + 4 * column))));
        left.eax = reg32(sum >> 16);
        left.edx = reg32(sum >> 32);
        guest.setDword(out + 4 * column, left.eax);
    }
    return left;
}

/* sub_4f44e0's copy: the nine ints at `from` turned over to `to`, the two apart. */
void fixedTransposeInto(Guest& guest, reg32 from, reg32 to)
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
reg32 chaseRate(Guest& guest, reg32 camera, reg32 car, reg32 offset)
{
    if (sreg32(guest.dword(car + kCarGrip)) < sreg32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
            guest.setDword(camera + kCameraWasAirborne, sreg32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
        guest.setDword(camera + kCameraAirWait, 3);
    }
    else if (guest.dword(camera + kCameraAirWait) == 0)
    {
        guest.call(kScaledIfSet, args({ camera }, { guest.dword(camera + kCameraOwn11c) }));
        const reg32 rate = bitsOf(float(guest.fpu().take()));
        if (guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) == 0)
            guest.call(kDampVector, args({ car }, { guest.dword(camera + kCameraOwn120), offset }));
        return rate;
    }
    guest.setDword(camera + kCameraAirWait, guest.dword(camera + kCameraAirWait) - 1);
    return 0;
}

/* The end of the two chase cameras looking ahead: the target eases from where it was to the
 * point behind the car (it jumps there as a wait ends, or when the view holds none), the
 * camera stands at the car's place plus it. */
reg32 chaseFollow(x86::CPU& cpu, Guest& guest, reg32 camera, reg32 rate, reg32 place, reg32 point, reg32 from, reg32 difference, reg32 scaled)
{
    const reg32 target = camera + kCameraTarget;
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
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(guest.app(), cpu, 1, point, from, difference);
    // Handed `scaled` and `target` in ebx and ecx too, which it does not read: there as the code has them.
    guest.leaveEbx(scaled);
    guest.leaveEcx(target);
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(guest.app(), cpu, 1, difference, floatOf(rate), scaled);
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), cpu, 1, from, scaled, target);
    // By address: what it leaves in eax is this function's.
    return guest.call(kAddVectors, args({ 1, place, target, camera + kCameraPlace }));
}

}  // namespace

/* sub_4243a0: behind the car, higher as its table says, set back by the car's size. */
x86::reg32 frontend::simd::cameraChase(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x5c);
    constexpr reg32 kLocalOffset = 0x00, kLocalScaled = 0x0c, kLocalFrom = 0x18, kLocalPoint = 0x24, kLocalDifference = 0x30;
    constexpr reg32 kDrivingLift = 0x2c0, kLiftUnit = 0x536ebc, kMostLift = 0x536ec4, kSizeShare = 0x536ecc;
    constexpr double kLeastLift = 0x1.2p-13;

    const reg32 offset = guest.frame() + kLocalOffset;
    const reg32 point = guest.frame() + kLocalPoint;
    copyDwords(guest, offset, camera + kCameraOwn110, 3);
    const reg32 car = guest.dword(camera + kCameraCar);
    if (cameraHeld(guest))
        return camera;
    const reg32 rate = chaseRate(guest, camera, car, offset);

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
    const reg32 place = guest.dword(camera + kCameraCar) + kCarPlace;
    if (back)
        return guest.call(kAddVectors, args({ 1, place, point, camera + kCameraPlace }));
    return chaseFollow(cpu, guest, camera, rate, place, point, guest.frame() + kLocalFrom, guest.frame() + kLocalDifference,
                       guest.frame() + kLocalScaled);
}

/* sub_424690: round the car (or what it rides on) on the race's clock. */
x86::reg32 frontend::simd::cameraOrbit(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16 + 0x18);
    constexpr std::uint8_t kCarRides = 0x20;
    constexpr reg32 kClock = 0x7d3684, kClockUnit = 0x536ed0, kAcross = 0x536ed4, kUp = 0x536ed8, kAlong = 0x536edc;
    constexpr reg32 kLocalSine = 0x0c, kLocalAngle = 0x10, kLocalCosine = 0x14;

    reg32 car = guest.dword(camera + kCameraCar);
    if (guest.byte(car + kCarFlags) & kCarRides)
        car = guest.dword(car + kCarTowed);
    const float x = guest.floatAt(car + kCarPlace), y = guest.floatAt(car + kCarPlace + 4), z = guest.floatAt(car + kCarPlace + 8);
    guest.setDword(camera + kCameraTargetCar, car);
    guest.setDword(camera + kCameraCar, car);
    if (cameraHeld(guest))
        return car;

    const reg32 frame = guest.frame();
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
x86::reg32 frontend::simd::cameraChaseLow(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x78);
    constexpr reg32 kLocalScaled = 0x00, kLocalDifference = 0x0c, kLocalOffset = 0x18, kLocalFrom = 0x24, kLocalPoint = 0x30, kLocalPlace = 0x3c;
    constexpr reg32 kSizeShare = 0x536ee0, kCarTurnA = 0x4ec, kCarTurnB = 0x4f4, kSmallerShare = 0x536ee4, kDropUnit = 0x536ee8, kMostDrop = 0x536eec;
    constexpr sreg32 kTunnelFrames = 0x20;
    constexpr reg32 kTunnelUpBits = 0x3f800000, kTunnelBackBits = 0xc0500000;
    constexpr float kDropWhenMost = 6.0f;

    const reg32 frame = guest.frame();
    const reg32 offset = frame + kLocalOffset, point = frame + kLocalPoint, place = frame + kLocalPlace;
    copyDwords(guest, offset, camera + kCameraOwn110, 3);
    const reg32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, place, car + kCarPlace, 3);
    const reg32 lastPlaceBits = guest.dword(car + kCarPlace + 8);
    if (cameraHeld(guest))
        return lastPlaceBits;

    if (sreg32(guest.dword(kViewInTunnel + guest.dword(camera + kCameraView) * 8)) > kTunnelFrames)
    {
        guest.setDword(offset, 0);
        guest.setDword(offset + 8, kTunnelBackBits);
        guest.setDword(offset + 4, kTunnelUpBits);
    }
    const reg32 rate = chaseRate(guest, camera, car, offset);

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
        return guest.call(kAddVectors, args({ 1, place, point, camera + kCameraPlace }));
    return chaseFollow(cpu, guest, camera, rate, place, point, frame + kLocalFrom, frame + kLocalDifference, frame + kLocalScaled);
}

/* sub_424b50: round the car, the angle going on every so many frames. */
x86::reg32 frontend::simd::cameraCircleCar(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x2c);
    constexpr reg32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kStep = kCameraOwn11c;
    constexpr reg32 kEvery = kCameraOwn120, kCount = kCameraOwn124, kAngle = kCameraOwn128;
    constexpr reg32 kLocalPlace = 0x00, kLocalPoint = 0x0c, kLocalOffset = 0x18, kLocalSine = 0x24, kLocalCosine = 0x28;

    const reg32 frame = guest.frame();
    const reg32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, frame + kLocalPlace, car + kCarPlace, 3);
    const reg32 lastPlaceBits = guest.dword(car + kCarPlace + 8);
    if (cameraHeld(guest))
        return lastPlaceBits;

    if (sreg32(guest.dword(car + kCarGrip)) < sreg32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
        {
            guest.setDword(camera + kCameraWasAirborne, sreg32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, frame + kLocalPlace, camera + kCameraPlace,
                                                                     camera + kCameraTarget);
        }
        guest.setDword(camera + kCameraAirWait, 0x40);
    }
    const reg32 count = guest.dword(camera + kCount);
    guest.setDword(camera + kCount, count + 1);
    if (sreg32(count) > sreg32(guest.dword(camera + kEvery)))
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
    return guest.call(kAddVectors, args({ 1, frame + kLocalPlace, frame + kLocalPoint, camera + kCameraPlace }));
}

/* sub_424ce0: round the car by the player's keys, every other tick. */
x86::reg32 frontend::simd::cameraLookAround(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x20);
    constexpr reg32 kTicks = 0x79c1a4, kKeysHeld = 0x55301c, kKeyTurn = 0x553020;
    constexpr reg32 kAutoLeft = 0x536ef0, kAutoRight = 0x536ef4, kKeyUnit = 0x536ef8;
    constexpr reg32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kAuto = kCameraOwn11c;
    constexpr reg32 kAngle = kCameraOwn120, kLastTick = kCameraOwn124;
    constexpr reg32 kLocalOffset = 0x00, kLocalPoint = 0x0c, kLocalSine = 0x18, kLocalCosine = 0x1c;

    const reg32 frame = guest.frame();
    const reg32 ticks = guest.dword(kTicks);
    if (sreg32(ticks - guest.dword(camera + kLastTick)) < 2)
        return ticks;
    guest.setDword(camera + kLastTick, ticks);
    if (guest.dword(kKeysHeld) == 0 || guest.dword(kKeyTurn) != 0)
    {
        guest.setDword(camera + kAuto, 0);
        Float angle;
        if (guest.dword(kKeysHeld) != 0)
            angle = fp::add(fp::mul(integerFrom(guest, kKeyTurn), floatFrom(guest, kKeyUnit)), floatFrom(guest, camera + kAngle));
        else if (guest.dword(kKeyTurn) == reg32(-1))
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
    const reg32 car = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, frame + kLocalOffset, car + kCarAxes, frame + kLocalPoint);
    return guest.call(kAddVectors, args({ 1, car + kCarPlace, frame + kLocalPoint, camera + kCameraPlace }));
}

/* sub_424e30: at the side of the car, on the side its node says, the angle from a count of
 * the car's; it looks at where it stands. */
x86::reg32 frontend::simd::cameraSideOfCar(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x24);
    constexpr reg32 kCarCount = 0x524, kCarNode = 0x1c, kCountUnit = 0x536efc, kCountBase = 0x536f00;
    constexpr reg32 kRadiusAcross = kCameraOwn110, kHeight = kCameraOwn114, kRadiusAlong = kCameraOwn118, kAngle = kCameraOwn120;
    constexpr reg32 kOneBits = 0x3f800000, kMinusOneBits = 0xbf800000;
    constexpr reg32 kLocalOffset = 0x00, kLocalPoint = 0x0c, kLocalSine = 0x18, kLocalCosine = 0x1c, kLocalSide = 0x20;

    const reg32 frame = guest.frame();
    const reg32 car = guest.dword(camera + kCameraCar);
    store(guest, camera + kAngle, fp::add(fp::mul(integerFrom(guest, car + kCarCount), floatFrom(guest, kCountUnit)), floatFrom(guest, kCountBase)));
    guest.setDword(frame + kLocalSide, (guest.byte(car + kCarNode) & 1) ? kOneBits : kMinusOneBits);
    sineCosineInto(guest, camera + kAngle, frame + kLocalSine, frame + kLocalCosine);
    store(guest, frame + kLocalOffset,
          fp::mul(fp::mul(floatFrom(guest, frame + kLocalCosine), floatFrom(guest, frame + kLocalSide)), floatFrom(guest, camera + kRadiusAcross)));
    store(guest, frame + kLocalOffset + 8, fp::mul(floatFrom(guest, camera + kRadiusAlong), floatFrom(guest, frame + kLocalSine)));
    guest.setDword(frame + kLocalOffset + 4, guest.dword(camera + kHeight));
    const reg32 carNow = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, frame + kLocalOffset, carNow + kCarAxes, frame + kLocalPoint);
    const reg32 result = guest.call(kAddVectors, args({ 1, carNow + kCarPlace, frame + kLocalPoint, camera + kCameraPlace }));
    copyDwords(guest, camera + kCameraTarget, frame + kLocalPoint, 3);
    return result;
}

/* sub_424f30: the camera's matrix eased, in quaternions, from the turn it holds towards the
 * one it has now, by its rate; the first time the turn is only noted.  At the second of two
 * passes while the first is the one running, the holding ends. */
x86::reg32 frontend::simd::cameraEaseTurn(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x24);
    constexpr reg32 kCameraMatrix = 0x2c, kHeld = kCameraOwn114, kHolds = kCameraOwn124, kRate = kCameraOwn128;
    constexpr reg32 kPasses = 0x7cda58, kPass = 0x7cd94c;
    constexpr reg32 kLocalNow = 0x00, kLocalEased = 0x10;

    const reg32 frame = guest.frame();
    const reg32 result = guest::direct<0x4b73c0, simc::quatFromMatrix>(app, cpu, camera + kCameraMatrix, frame + kLocalNow);
    if (guest.dword(camera + kHolds) == 0)
    {
        copyDwords(guest, camera + kHeld, frame + kLocalNow, 4);
        guest.setDword(camera + kHolds, 1);
        return result;
    }
    const reg32 rate = guest.dword(camera + kRate);
    const reg32 passes = guest.dword(kPasses);
    if (passes == 2 && sreg32(passes) > sreg32(guest.dword(kPass)))
    {
        guest.setDword(camera + kHolds, 0);
        return rate;
    }
    // Handed the passes in ebx too, which it does not read: there as the code has it.
    guest.leaveEbx(passes);
    guest::direct<0x4b71e0, simc::quatSlerp>(app, cpu, camera + kHeld, frame + kLocalNow, rate, frame + kLocalEased);
    copyDwords(guest, camera + kHeld, frame + kLocalEased, 4);
    return guest::direct<0x4b7590, simc::matrixFromQuat>(app, cpu, frame + kLocalEased, camera + kCameraMatrix);
}

namespace
{

/* What sub_424fd0 and sub_425b40 begin with: the camera at the spot, and the distance from
 * its target car's place to the spot, asked once for its sign and once for itself. */
struct GoesTo
{
    float distance;
    reg32 eax;  // what the second call left there
};

GoesTo cameraGoesTo(x86::CPU& cpu, Guest& guest, reg32 camera, reg32 spot)
{
    copyDwords(guest, camera + kCameraPlace, spot, 3);
    guest::direct<0x4965b0, frontend::sima::quickDistance>(guest.app(), cpu,
                                                            guest.dword(camera + kCameraTargetCar) + kCarPlace, spot);
    const Float first = guest.fpu().take();
    const bool below = jumpsAbove(fp::compare(0.0, first));
    const reg32 eax = guest::direct<0x4965b0, frontend::sima::quickDistance>(
        guest.app(), cpu, guest.dword(camera + kCameraTargetCar) + kCarPlace, spot);
    const Float second = guest.fpu().take();
    return { float(below ? -second : second), eax };
}

}  // namespace

/* sub_424fd0: the camera to the spot, the zoom the distance over the stack's float if that is
 * something and the distance is past it, else none. */
x86::reg32 frontend::simd::cameraMoveTowards(App* app, CPU& cpu, reg32 camera, reg32 spot, reg32 near)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 4);
    const GoesTo goes = cameraGoesTo(cpu, guest, camera, spot);
    const float distance = goes.distance;
    const reg16 status = fp::compare(Float(distance), Float(floatOf(near)));
    if (jumpsAbove(status) && (near & 0x7fffffff) != 0)
        store(guest, camera + kCameraZoom, fp::div(Float(distance), Float(floatOf(near))));
    else
        guest.setDword(camera + kCameraZoom, 0);
    return withStatus(goes.eax, status);
}

/* sub_425b40: the camera to the spot, the zoom the stack's base and rate at that distance.
 * Each fifth time it prints the two to a buffer it then forgets, the developers' trace. */
x86::reg32 frontend::simd::cameraGlideTowards(App* app, CPU& cpu, reg32 camera, reg32 spot, reg32 base, reg32 rate)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 0x104);
    constexpr reg32 kCount = 0x553028, kFormat = 0x536f24;
    constexpr reg32 kLocalText = 0x00;
    constexpr reg32 kEveryFifth = 5;

    const reg32 frame = guest.frame();
    const GoesTo goes = cameraGoesTo(cpu, guest, camera, spot);
    const float distance = goes.distance;
    const reg32 count = guest.dword(kCount) + 1;
    guest.leaveEdx(count);
    const Float zoom = fp::add(fp::mul(Float(distance), Float(floatOf(rate))), Float(floatOf(base)));
    guest.setDword(kCount, count);
    store(guest, camera + kCameraZoom, zoom);
    if (count != kEveryFifth)
        return goes.eax;

    const double shownDistance = double(distance), shownZoom = double(guest.floatAt(camera + kCameraZoom));
    reg32 words[4];
    std::memcpy(words, &shownDistance, 8);
    std::memcpy(words + 2, &shownZoom, 8);
    const reg32 result = guest.call(kSprintf, args({}, { frame + kLocalText, kFormat, words[0], words[1], words[2], words[3] }));
    guest.setDword(kCount, 0);
    return result;
}

/* sub_425990: held to the car at the camera's offset, its third part's sign changed for good
 * each frame the view looks back; as the car leaves the ground the target noted, and a wait. */
x86::reg32 frontend::simd::cameraBesideCar(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x18);
    constexpr reg32 kLocalPoint = 0x00, kLocalPlace = 0x0c;

    const reg32 frame = guest.frame();
    const reg32 car = guest.dword(camera + kCameraCar);
    copyDwords(guest, frame + kLocalPlace, car + kCarPlace, 3);
    if (sreg32(guest.dword(car + kCarGrip)) < sreg32(kLeastGripBits))
    {
        if (guest.dword(camera + kCameraAirWait) == 0)
        {
            guest.setDword(camera + kCameraWasAirborne, sreg32(guest.dword(car + kCarWheelsOff)) > 0 ? 1 : 0);
            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, camera + kCameraPlace, frame + kLocalPlace,
                                                                     camera + kCameraTarget);
        }
        guest.setDword(camera + kCameraAirWait, 0x40);
    }
    if (guest.dword(kLookingBack + guest.dword(camera + kCameraView) * 4) != 0)
        guest.setByte(camera + kCameraOwn118 + 3, guest.byte(camera + kCameraOwn118 + 3) ^ 0x80);
    vectorByMatrix(guest, camera + kCameraOwn110, guest.dword(camera + kCameraCar) + kCarAxes, frame + kLocalPoint);
    return guest.call(kAddVectors, args({ 1, frame + kLocalPlace, frame + kLocalPoint, camera + kCameraPlace }));
}

/* sub_425ab0: on the car, at the camera's offset. */
x86::reg32 frontend::simd::cameraOnCar(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16 + 0x0c);
    const reg32 frame = guest.frame();
    const reg32 car = guest.dword(camera + kCameraCar);
    vectorByMatrix(guest, camera + kCameraOwn110, car + kCarAxes, frame);
    return guest.call(kAddVectors, args({ 1, car + kCarPlace, frame, camera + kCameraPlace }));
}

/* sub_426370: a mode about to step (eax its camera, edx the view, ebx the mode): the frames
 * the view has both car and camera in a tunnel counted, and but for mode 8 the wait of that
 * mode's camera put back. */
x86::reg32 frontend::simd::cameraPrepare(App* app, CPU& cpu, reg32 camera, reg32 view, reg32 mode)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    constexpr reg32 kBesideRoadCameras = 0x552f08, kWaiting = 0x124, kCarNode = 0x1c;
    constexpr reg32 kModeBesideRoad = 8;

    reg32 result = camera;
    const reg32 car = guest.dword(camera + kCameraCar);
    if (car != 0)
    {
        result = *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, cpu, car + kCarPlace, guest.dword(car + kCarNode));
        if (result != 0)
            result = *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, cpu, camera + kCameraPlace,
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
x86::reg32 frontend::simd::fixedNormalise(App* app, CPU& cpu, reg32 vector)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x18);
    constexpr sreg32 kMostPart = 0x1000000;
    constexpr reg32 kMostSquares = 0x40000000;
    constexpr reg32 kRootScale = 0x5661c4, kRootBase = 0x566248;

    sreg32 x = sreg32(guest.dword(vector)), y = sreg32(guest.dword(vector + 4)), z = sreg32(guest.dword(vector + 8));
    const auto over = [](sreg32 part) {
        // neg, on 0x80000000 too, which stays below and so passes.
        const reg32 size = part > 0 ? reg32(part) : 0u - reg32(part);
        return sreg32(size) > kMostPart;
    };
    while (over(x) || over(y) || over(z))
    {
        x >>= 1;
        y >>= 1;
        z >>= 1;
    }
    reg32 xx = fixedTimes(reg32(x), reg32(x));
    reg32 yy = fixedTimes(reg32(y), reg32(y));
    reg32 zz = fixedTimes(reg32(z), reg32(z));
    while ((xx >> 2) + (yy >> 2) + (zz >> 2) > kMostSquares)
    {
        x >>= 1;
        y >>= 1;
        z >>= 1;
        xx = fixedTimes(reg32(x), reg32(x));
        yy = fixedTimes(reg32(y), reg32(y));
        zz = fixedTimes(reg32(z), reg32(z));
    }
    reg32 length = xx + yy + zz;
    if (length != 0)
    {
        const reg32 top = reg32(31 - __builtin_clz(length));
        const std::uint64_t scaled = std::uint64_t(length) * guest.dword(kRootScale + top * 4);
        length = reg32(scaled >> 16) + guest.dword(kRootBase + top * 4);
    }
    if (length == 0)
        return 0;
    // Two divisions of 64 bits by 32, whole part then fraction, as idiv's low halves.
    const auto over16 = [&](sreg32 part) {
        const std::int64_t by = sreg32(length);
        const reg32 whole = reg32(std::int64_t(part) / by);
        const sreg32 rest = sreg32(std::int64_t(part) % by);
        return reg32((std::int64_t(rest) * 0x10000) / by) + (whole << 16);
    };
    guest.setDword(vector, over16(x));
    guest.setDword(vector + 4, over16(y));
    const reg32 lastPart = over16(z);
    guest.setDword(vector + 8, lastPart);
    return lastPart;
}

/* sub_4969c0: the height of the stack's point dropped onto the plane through the stack's
 * origin that the matrix eax holds flat: to the matrix's frame, its second part cleared,
 * back; the y of that, in eax. */
x86::reg32 frontend::simd::trackPointOnPlane(App* app, CPU& cpu, reg32 matrix, reg32 words)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x3c);
    constexpr reg32 kLocalTurned = 0x00, kLocalLocal = 0x24, kLocalBack = 0x30;

    const reg32 frame = guest.frame();
    const reg32 origin = words, point = words + 0x0c;
    for (reg32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, guest.dword(point + 4 * i) - guest.dword(origin + 4 * i));
    fixedTransposeInto(guest, matrix, frame + kLocalTurned);
    fixedMatrixTimesInto(guest, point, frame + kLocalTurned, frame + kLocalLocal);
    guest.setDword(frame + kLocalLocal + 4, 0);
    fixedMatrixTimesInto(guest, frame + kLocalLocal, matrix, frame + kLocalBack);
    return guest.dword(frame + kLocalBack + 4) + guest.dword(origin + 4);
}

/* sub_496a50: a frame from a triangle's three points at eax: its middle point to the first,
 * the normal, to the third, each made one long, to edx; any of no length, the identity. */
x86::reg32 frontend::simd::frameOfTriangle(App* app, CPU& cpu, reg32 points, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 0x28);
    constexpr reg32 kIdentity = 0x564498;
    constexpr reg32 kLocalNormal = 0x00, kLocalToThird = 0x0c, kLocalToFirst = 0x18;

    const reg32 frame = guest.frame();
    const reg32 toFirst = frame + kLocalToFirst, toThird = frame + kLocalToThird, normal = frame + kLocalNormal;
    for (reg32 i = 0; i < 3; ++i)
        store(guest, toFirst + 4 * i, fp::sub(floatFrom(guest, points + 4 * i), floatFrom(guest, points + 0x0c + 4 * i)));
    for (reg32 i = 0; i < 3; ++i)
        store(guest, toThird + 4 * i, fp::sub(floatFrom(guest, points + 0x18 + 4 * i), floatFrom(guest, points + 0x0c + 4 * i)));
    crossProduct(guest, toFirst, toThird, normal);
    guest.leaveEdx(toThird);
    for (const reg32 vector : { toFirst, normal, toThird })
    {
        guest.call(kNormalise, args({ vector }));
        const reg16 status = fp::compare(0.0, guest.fpu().take());
        if (jumpsEqual(status))
        {
            copyDwords(guest, out, kIdentity, 9);
            return withStatus(guest.eax(), status);
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
x86::reg32 frontend::simd::placeInLane(App* app, CPU& cpu, reg32 index, reg32 lane, reg32 away, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12 + 0xbc);
    constexpr reg32 kSlices = 0x5dd948, kSliceBytes = 36;
    constexpr reg32 kSliceSideBytes = 0x15, kSliceWidthNear = 0x17, kSliceWidthFar = 0x1b;
    constexpr sreg32 kFirstFarLane = 7;
    constexpr reg32 kWidthUnit = 0x18000;
    constexpr reg32 kScale = 0x540358;
    constexpr reg32 kLocalRecord = 0x00, kLocalPoint = 0x3a + 0x56, kLocalFloats = 0x3a + 0x62;

    const reg32 frame = guest.frame();
    const reg32 offset = index * kSliceBytes;
    const reg32 slice = guest.dword(kSlices) + offset;
    const reg32 centre[3] = { guest.dword(slice), guest.dword(slice + 4), guest.dword(slice + 8) };
    reg32 side[3];
    for (reg32 i = 0; i < 3; ++i)
        side[i] = reg32(sreg32(guest.dword(slice + kSliceSideBytes + i)) >> 24) << 9;

    guest.leaveEcx(index);
    const reg32 share = fe::direct<0x4c9a80, tail::laneOffset>(app, cpu, index, lane);
    const reg32 point = frame + kLocalPoint;
    for (reg32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, centre[i] + fixedTimes(share, side[i]));

    const reg32 sliceNow = guest.dword(kSlices) + offset;
    const reg32 width = reg32(guest.byte(sliceNow + (sreg32(lane) < kFirstFarLane ? kSliceWidthNear : kSliceWidthFar))) << 15;
    sreg32 across = sreg32(fixedTimes(width, kWidthUnit));
    if (away == 0)
        across = sreg32(0u - reg32(across));
    const reg32 half = reg32(across / 2);
    for (reg32 i = 0; i < 3; ++i)
        guest.setDword(point + 4 * i, guest.dword(point + 4 * i) + fixedTimes(half, side[i]));

    // By address: it leaves edx and ebx, which this function leaves its caller.
    guest.setEsi(frame + kLocalRecord);
    guest.call(kPlaceOnTrack, args({ point, 1, index }));

    const Float scale = doubleAt(guest, kScale);
    const Float x = fp::mul(integerFrom(guest, point), scale);
    const Float y = fp::mul(integerFrom(guest, point + 4), scale);
    const Float z = fp::mul(scale, integerFrom(guest, point + 8));
    const reg32 floats = frame + kLocalFloats;
    store(guest, floats, x);
    store(guest, floats + 4, y);
    store(guest, floats + 8, z);
    copyDwords(guest, out, floats, 3);
    guest.setEsi(floats + 12);
    return out;
}

namespace
{

// ---------------------------------------------------------------------------
// The race's list of tasks
// ---------------------------------------------------------------------------

/* A list: its most entries, its count, from +8 entries of 16 bytes, priority first, then the
 * function and its argument.  An unused one's priority is 0x7ffe. */
constexpr reg32 kListCount = 4, kListEntries = 8, kEntryBytes = 16;
constexpr reg32 kEntryPriority = 0, kEntryFunction = 4, kEntryArgument = 8;

}  // namespace

/* sub_4be1f0: a list for edx entries, from the game's memory. */
x86::reg32 frontend::simd::taskListNew(App* app, CPU& cpu, reg32 most)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    constexpr reg32 kName = 0x5401b0, kHeaderBytes = 0x18, kUnused = 0x7ffe;

    guest.leaveEcx(most);
    const reg32 list = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kName, (most << 4) + kHeaderBytes, 0);
    guest.setDword(list + kListCount, 0);
    guest.leaveEdx(list);
    guest.setDword(list, most);
    for (sreg32 i = 0; i < sreg32(guest.dword(list)); ++i)
    {
        const reg32 entry = list + kListEntries + reg32(i) * kEntryBytes;
        guest.setDword(entry + kEntryPriority, kUnused);
        guest.setDword(entry + kEntryFunction, 0);
    }
    return list;
}

/* sub_4be250: a function and its argument put in by priority, before the first of a greater
 * one, those after it moved up.  The registers left are the walk's. */
x86::reg32 frontend::simd::taskListAdd(App* app, CPU& cpu, reg32 list, reg32 function, reg32 argument, reg32 priority)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12 + 0x14);
    const reg32 count = guest.dword(list + kListCount) + 1;
    guest.setDword(list + kListCount, count);
    guest.leaveEcx(count);
    sreg32 slot = 0;
    for (sreg32 i = 0; i < sreg32(guest.dword(list + kListCount)); ++i)
    {
        slot = i;
        guest.leaveEcx(priority);
        if (sreg32(priority) >= sreg32(guest.dword(list + kListEntries + reg32(i) * kEntryBytes + kEntryPriority)))
            continue;
        sreg32 last = sreg32(guest.dword(list + kListCount) - 1);
        while (last > i)
        {
            copyDwords(guest, list + kListEntries + reg32(last) * kEntryBytes, list + kListEntries + reg32(last - 1) * kEntryBytes, 4);
            --last;
            guest.leaveEcx(0);
        }
        guest.leaveEbx(reg32(last));
        break;
    }
    const reg32 entry = list + kListEntries + reg32(slot) * kEntryBytes;
    guest.setDword(entry + kEntryPriority, priority);
    guest.setDword(entry + kEntryFunction, function);
    guest.setDword(entry + kEntryArgument, argument);
    guest.leaveEdx(reg32(slot) * kEntryBytes + list);
    return argument;
}

namespace
{

constexpr reg32 kRaceTasksA = 0x7d3690, kRaceTasksB = 0x7d368c;

}  // namespace

/* sub_4c45f0: the race opened: its flags cleared; once only, its lists and all its parts set up. */
x86::reg32 frontend::simd::raceOpen(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16 + 0x80);
    constexpr reg32 kFlagsOff[] = { 0x7d3694, 0x7d369c, 0x7d36a0, 0x7d36a8, 0x7d36ac, 0x7d36b0, 0x7d36a4, 0x7d3680, 0x7d3684 };
    constexpr reg32 kFlagOn = 0x7d3698;
    constexpr reg32 kCarLoadStep = 0x42ce70, kNameA = 0x5401ec, kNameB = 0x5401f4, kFormat = 0x5401fc, kGlyphsDirectory = 0x7a2934;
    constexpr reg32 kAlreadyOpen = 0x55e88c;
    constexpr reg32 kMostTasks = 0x90, kCarLoadPriority = 8;
    constexpr reg32 kLocalPath = 0x00;

    for (const reg32 flag : kFlagsOff)
        guest.setDword(flag, 0);
    guest.setDword(kFlagOn, 1);
    guest.leaveEdx(0);
    guest.leaveEcx(1);
    const reg32 alive = fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    if (guest.dword(kAlreadyOpen) != 0)
        return alive;
    guest.leaveEcx(kCarLoadPriority);
    // Each handed its list's name in eax too, which it does not read: there as the code has it.
    cpu.eax = kNameA;
    guest.setDword(kRaceTasksA, guest::direct<0x4be1f0, taskListNew>(app, cpu, kMostTasks));
    guest.leaveEbx(0);
    cpu.eax = kNameB;
    guest.setDword(kRaceTasksB, guest::direct<0x4be1f0, taskListNew>(app, cpu, kMostTasks));
    guest::direct<0x4be250, taskListAdd>(app, cpu, guest.dword(kRaceTasksA), kCarLoadStep, 0, kCarLoadPriority);
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    const reg32 frame = guest.frame();
    guest.call(kSprintf, args({}, { frame + kLocalPath, kFormat, kGlyphsDirectory }));
    guest.call(0x475240, args({ frame + kLocalPath, 0 }));
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    guest.call(0x4371d0, args({ gameSettings(guest).raceSeed }));
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    guest.call(0x42c830);
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    guest.call(0x4711e0);
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    // Each called in turn, as the code calls them: those with a typed core as C++.
    for (const reg32 step : { 0x404660u, 0x405e60u, 0x40e410u, 0x401220u, 0x40cb50u, 0x40c150u, 0x401320u, 0x408ad0u, 0x404c40u, 0x40a6f0u })
        guest.call(step);
    fe::direct<0x493ef0, rendc::conesLoad>(app, cpu);
    guest::direct<0x4c9d70, raceClearA>(app, cpu);
    fe::direct<0x4cdd00, loopb::standingsStart>(app, cpu);
    guest.call(0x4129c0);
    fe::direct<0x44b950, menua::racePauseMenuBegin>(app, cpu);
    return guest::direct<0x4bc910, raceAddTasks>(app, cpu);
}

/* sub_4c4740: the race started again: the HUD's pane put back if one was copied, its flags
 * cleared, all its parts told. */
x86::reg32 frontend::simd::raceRestart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20);
    constexpr reg32 kPaneCopied = 0x7d3694, kPaneBytes = 0x1a4;
    constexpr reg32 kFlagsOff[] = { 0x7d36a0, 0x7d36a8, 0x7d36ac, 0x7d36b0, 0x7d36a4, 0x7d3680, 0x7d3684, 0x7d369c };
    constexpr reg32 kTickNow = 0x79c4ac, kTickAtStart = 0x7d3688;

    if (guest.dword(kPaneCopied) != 0)
    {
        guest.leaveEcx(0);
        // By address: it leaves edx, which the calls below hand on.
        guest.call(0x4ea4f0, args({ SETTINGS_ADDRESS(hud[6]), SETTINGS_ADDRESS(hud), kPaneBytes }));
        guest.setDword(kPaneCopied, 0);
    }
    guest.leaveEbx(0);
    const reg32 kind = gameSettings(guest).replayMode;
    for (const reg32 flag : kFlagsOff)
        guest.setDword(flag, 0);
    if (kind == 0)
        gameSettings(guest).lastTickRaced = 0;
    guest.call(0x4371d0, args({ gameSettings(guest).raceSeed }));
    // Each called in turn, as the code calls them: those with a typed core as C++.
    fe::direct<0x4b7c20, loada::copKeepLight>(app, cpu);
    for (const reg32 step : { 0x42c530u, 0x40e0f0u, 0x401230u, 0x40cb20u, 0x40c140u, 0x4012c0u, 0x408ae0u, 0x404c50u, 0x404650u,
                              0x403970u, 0x405fb0u })
        guest.call(step);
    fe::direct<0x493e50, rendc::conesReset>(app, cpu);
    guest::direct<0x4c9db0, raceClearB>(app, cpu);
    guest::direct<0x4bc8b0, sime::effectsReset>(app, cpu);
    fe::direct<0x4cdd00, loopb::standingsStart>(app, cpu);
    guest.call(0x471000);
    const reg32 tick = guest.dword(kTickNow);
    guest.setDword(kTickAtStart, tick);
    return tick;
}

/* sub_4c7bd0: on a network, the frame's packet: 0xb, this player, the frame, a stamp, its
 * controls and every car's last; or, a game that does not send all its cars, those kept by
 * frame in a ring of 20 under the lock.  A player past the race's end is told on. */
x86::reg32 frontend::simd::netSendPlace(App* app, CPU& cpu, reg32 eax)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x20);
    constexpr reg32 kThisPlayer = 0x7a2276, kSendsAll = 0x7a22da;
    constexpr reg32 kNetCars = 0x5efb88, kCarStamp = 8;
    constexpr reg32 kLock = 0x560114, kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
    constexpr reg32 kRingFrames = 0x7dd120, kRingCars = 0x7dd110, kRingBytes = 20;
    constexpr sreg32 kRingLength = 20;
    constexpr reg32 kTakenControls = 0x4c6840;
    constexpr reg32 kPacketType = 0x0b, kWholePacket = 0x1c, kShortPacket = 0x0e, kMostCars = 7;
    constexpr reg32 kLocalPacket = 0x00, kLocalStamp = 0x08, kLocalCars = 0x0c, kLocalControls = 0x1c;
    constexpr reg32 kFinished = 3;

    if (sreg32(gameSettings(guest).replayMode) > 1)
        return eax;
    const reg32 players = gameSettings(guest).playMode;
    if (players == 1 || players == 0 || gameSettings(guest).networkGame == 0)
        return eax;

    const reg32 frame = guest.frame();
    const reg32 packet = frame + kLocalPacket;
    guest.setByte(packet, kPacketType);
    guest.setByte(packet + 1, guest.byte(kThisPlayer + 2));
    guest.setDword(packet + 4, eax);
    fe::direct<0x4c8310, neta::packControls>(app, cpu, frame + kLocalStamp);
    const reg32 player = reg32(sreg32(guest.dword(kThisPlayer)) >> 16);
    guest.call(kTakenControls, args({ player, frame + kLocalControls }));
    guest.setByte(frame + kLocalCars, guest.byte(frame + kLocalControls + 3));

    reg32 bytes;
    if (guest.word(kSendsAll) != 0)
    {
        for (sreg32 i = 0; i < sreg32(gameSettings(guest).opponentCount); ++i)
        {
            const reg32 car = guest.dword(kNetCars + reg32(i) * 4);
            guest.setWord(frame + kLocalCars + 2 * reg32(i + 1), guest.word(car + kCarStamp));
        }
        bytes = kWholePacket - 2 * (kMostCars - gameSettings(guest).opponentCount);
    }
    else
    {
        const reg32 slot = reg32((sreg32(eax) >> 5) % kRingLength);
        windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(guest, cpu, guest.dword(kLock));
        guest.setDword(kRingFrames + slot * kRingBytes, eax);
        for (sreg32 i = 0; i < sreg32(gameSettings(guest).opponentCount); ++i)
        {
            const reg32 car = guest.dword(kNetCars + reg32(i) * 4);
            guest.setWord(kRingCars + slot * kRingBytes + 2 * reg32(i), guest.word(car + kCarStamp));
        }
        windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(guest, cpu, guest.dword(kLock));
        bytes = kShortPacket;
    }
    const ea::Regs sent = *fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, bytes);
    // What it leaves in eax, edx and ebx, as the code leaves them for the call below.
    cpu.eax = sent.eax;
    guest.leaveEdx(sent.edx);
    guest.leaveEbx(sent.ebx);
    const reg32 car = guest.dword(kCarsByPlace + gameSettings(guest).localPlayer * 4);
    if (guest.dword(car + kCarState) != kFinished)
        return car;
    return fe::direct<0x4c7e50, neta::sendPlayerGone>(app, cpu, gameSettings(guest).localPlayer);
}

/* sub_485880: the HUD's six panes to their defaults, after the screen's own (sub_485680). */
x86::reg32 frontend::simd::hudDefaults(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20);
    constexpr reg32 kPaneBytes = 0x1a4;
    constexpr reg32 kDriverType = 0x7a3a58;
    constexpr std::uint8_t kDriverFlagTwo = 0x02;
    constexpr reg32 kFourBits = 0x40800000, kOneBits = 0x3f800000;

    guest.leaveEdx(1);
    // By address: what it leaves in edx is read below.
    guest.call(0x485680);
    const reg32 one = guest.edx();
    gameSettings(guest).cameraMode[0] = one;
    gameSettings(guest).cameraMode[1] = one;
    const bool flagTwo = (guest.byte(kDriverType) & kDriverFlagTwo) != 0;
    const reg32 style = gameSettings(guest).language != 0 ? 3 : 1;
    for (reg32 i = 0; i < 6; ++i)
    {
        HudRecord& pane = gameSettings(guest).hud[i];
        const bool odd = (i & 1) != 0;
        pane.shown = one;
        pane.opponentName = i == 0 ? (flagTwo ? 1 : 0) : odd ? 0 : one;
        const reg32 firstTwo = i < 2 && !flagTwo ? 1 : 0;
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
reg32 turnSource(Guest& guest, reg32 matrix, reg32 out)
{
    if (matrix != out)
        return matrix;
    copyDwords(guest, guest.frame(), out, 9);
    return guest.frame();
}

constexpr reg32 kTurnFrame = 24 + 0x2c;

}  // namespace

/* sub_4c9d70, sub_4c9db0: the 0x280 bytes at 0x7dd850 cleared, as a race opens and starts again. */
x86::reg32 frontend::simd::raceClearB(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kBlock = 0x7dd850, kBytes = 0x280, kFillZero = 0x4e070c;
    return guest.call(kFillZero, args({ kBlock, kBytes }));
}

// ---------------------------------------------------------------------------
// The race cameras
// ---------------------------------------------------------------------------

void frontend::simCameraChase(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4243a0", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraChase(app, cpu, camera); });
}

void frontend::simCameraOrbit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424690", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraOrbit(app, cpu, camera); });
}

void frontend::simCameraChaseLow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424740", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraChaseLow(app, cpu, camera); });
}

void frontend::simCameraCircleCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424b50", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraCircleCar(app, cpu, camera); });
}

void frontend::simCameraLookAround(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424ce0", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraLookAround(app, cpu, camera); });
}

void frontend::simCameraSideOfCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424e30", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraSideOfCar(app, cpu, camera); });
}

void frontend::simCameraEaseTurn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424f30", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraEaseTurn(app, cpu, camera); });
}

void frontend::simCameraMoveTowards(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_424fd0", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 camera = cpu.eax, spot = cpu.edx, near = guest::at(app, cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraMoveTowards(app, cpu, camera, spot, near); });
}

void frontend::simCameraBesideCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425990", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraBesideCar(app, cpu, camera); });
}

void frontend::simCameraOnCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425ab0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraOnCar(app, cpu, camera); });
}

void frontend::simCameraGlideTowards(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425b40", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 camera = cpu.eax, spot = cpu.edx;
    const reg32 base = guest::at(app, cpu.esp + 4), rate = guest::at(app, cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraGlideTowards(app, cpu, camera, spot, base, rate); });
}

void frontend::simCameraPrepare(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426370", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 camera = cpu.eax, view = cpu.edx, mode = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cameraPrepare(app, cpu, camera, view, mode); });
}

// ---------------------------------------------------------------------------
// The shared maths
// ---------------------------------------------------------------------------

/* sub_4e0090: the length of edx less eax (sub_4e06a0), on the x87. */
x86::reg32 frontend::simd::vectorDistance(App* app, CPU& cpu, reg32 from, reg32 to)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 difference = guest.frame();
    for (reg32 i = 0; i < 3; ++i)
        store(guest, difference + 4 * i, fp::sub(floatFrom(guest, to + 4 * i), floatFrom(guest, from + 4 * i)));
    // By address: what it leaves in eax is this function's.
    return guest.call(kVectorLength, args({ difference }));
}

void frontend::simVectorDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e0090", 0, 0, nullptr, 0, kRestoreNone };
    const reg32 from = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::vectorDistance(app, cpu, from, to); });
}

/* sub_4e0110: a matrix turned over, eax to edx, which may be the same: the order of the
 * moves is what lets it. */
x86::reg32 frontend::simd::transpose(App* app, CPU& cpu, reg32 from, reg32 to)
{
    Guest guest(app, cpu);
    guest.setDword(to, guest.dword(from));
    guest.setDword(to + 0x10, guest.dword(from + 0x10));
    guest.setDword(to + 0x20, guest.dword(from + 0x20));
    const float a = guest.floatAt(from + 0x0c);
    const reg32 aBack = guest.dword(from + 0x04);
    guest.setFloat(to + 0x04, a);
    guest.setDword(to + 0x0c, aBack);
    const float b = guest.floatAt(from + 0x18);
    const reg32 bBack = guest.dword(from + 0x08);
    guest.setFloat(to + 0x08, b);
    guest.setDword(to + 0x18, bBack);
    const float c = guest.floatAt(from + 0x1c);
    const reg32 cBack = guest.dword(from + 0x14);
    guest.setFloat(to + 0x14, c);
    guest.setDword(to + 0x1c, cBack);
    return from;
}

void frontend::simTranspose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e0110", 0, 0, nullptr, 0 };
    const reg32 from = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::transpose(app, cpu, from, to); });
}

/* sub_4e0160: a by b, to ebx. */
x86::reg32 frontend::simd::cross(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    Guest guest(app, cpu);
    crossProduct(guest, a, b, out);
    return a + 0x0c;
}

void frontend::simCross(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e0160", 0, 0, nullptr, 0, kRestoreNone };
    const reg32 a = cpu.eax, b = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::cross(app, cpu, a, b, out); });
}

/* sub_4e01a0: the cross product made one long (sub_4e01c0), to ebx. */
x86::reg32 frontend::simd::unitCross(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    const reg32 cross = guest.frame();
    crossProduct(guest, a, b, cross);
    guest.leaveEbx(cross);
    // By address: what it leaves in eax is this function's.
    return guest.call(kUnitVector, args({ cross, out }));
}

void frontend::simUnitCross(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e01a0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 a = cpu.eax, b = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::unitCross(app, cpu, a, b, out); });
}

/* sub_4ea930, sub_4ea950: the sine, the cosine, of the stack's float, on the x87. */
void frontend::simd::sine(App* app, CPU& cpu, reg32 angle)
{
    Guest guest(app, cpu);
    guest.fpu().push(sineOf(guest, floatOf(angle)));
}

void frontend::simd::cosine(App* app, CPU& cpu, reg32 angle)
{
    Guest guest(app, cpu);
    guest.fpu().push(cosineOf(guest, floatOf(angle)));
}

void frontend::simSine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea930", 4, 0, nullptr, 0 };
    const reg32 eax = cpu.eax, angle = guest::at(app, cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        simd::sine(app, cpu, angle);
        return eax;
    });
}

void frontend::simCosine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea950", 4, 0, nullptr, 0 };
    const reg32 eax = cpu.eax, angle = guest::at(app, cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        simd::cosine(app, cpu, angle);
        return eax;
    });
}

/* sub_4ea970: the sine and the cosine of the float at eax, to edx and ebx. */
x86::reg32 frontend::simd::sineCosine(App* app, CPU& cpu, reg32 angleAt, reg32 sineAt, reg32 cosineAt)
{
    Guest guest(app, cpu);
    sineCosineInto(guest, angleAt, sineAt, cosineAt);
    return angleAt;
}

void frontend::simSineCosine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea970", 0, 0, nullptr, 0 };
    const reg32 angleAt = cpu.eax, sineAt = cpu.edx, cosineAt = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::sineCosine(app, cpu, angleAt, sineAt, cosineAt); });
}

/* sub_4ea9c0: the angle of the stack's two floats in the game's units, on the x87. */
void frontend::simd::angleOfFloats(App* app, CPU& cpu, reg32 first, reg32 second)
{
    constexpr reg32 kUnitsPerRadian = 0x54b6c0;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    x86::FPU& fpu = guest.fpu();
    fpu.push(Float(floatOf(second)));
    fpu.push(Float(floatOf(first)));
    guest.call(kArcTangent);
    const Float angle = fp::mul(fpu.take(), doubleAt(guest, kUnitsPerRadian));
    fpu.push(angle);
}

void frontend::simAngleOfFloats(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea9c0", 8, 0, nullptr, 0 };
    const reg32 eax = cpu.eax, first = guest::at(app, cpu.esp + 4), second = guest::at(app, cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        simd::angleOfFloats(app, cpu, first, second);
        return eax;
    });
}

/* sub_4ea9e0: the matrix at eax turned by the stack's angle about its first row, to the
 * stack's second. */
x86::reg32 frontend::simd::rotateX(App* app, CPU& cpu, reg32 matrix, reg32 angleBits, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, kTurnFrame);
    const reg32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    guest.setDword(out, guest.dword(m));
    guest.setDword(out + 4, guest.dword(m + 4));
    const reg32 result = guest.dword(m + 8);
    guest.setDword(out + 8, result);
    for (reg32 i = 0; i < 3; ++i)
        store(guest, out + 0x0c + 4 * i, fp::add(fp::mul(floatFrom(guest, m + 0x0c + 4 * i), c), fp::mul(floatFrom(guest, m + 0x18 + 4 * i), s)));
    for (reg32 i = 0; i < 3; ++i)
        store(guest, out + 0x18 + 4 * i, fp::sub(fp::mul(floatFrom(guest, m + 0x18 + 4 * i), c), fp::mul(floatFrom(guest, m + 0x0c + 4 * i), s)));
    return result;
}

/* sub_4eaab0: about its second row. */
x86::reg32 frontend::simd::rotateY(App* app, CPU& cpu, reg32 matrix, reg32 angleBits, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, kTurnFrame);
    const reg32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    store(guest, out, fp::sub(fp::mul(floatFrom(guest, m), c), fp::mul(floatFrom(guest, m + 0x18), s)));
    store(guest, out + 4, fp::sub(fp::mul(floatFrom(guest, m + 4), c), fp::mul(floatFrom(guest, m + 0x1c), s)));
    store(guest, out + 8, fp::sub(fp::mul(floatFrom(guest, m + 8), c), fp::mul(floatFrom(guest, m + 0x20), s)));
    guest.setDword(out + 0x0c, guest.dword(m + 0x0c));
    guest.setDword(out + 0x10, guest.dword(m + 0x10));
    const reg32 result = guest.dword(m + 0x14);
    guest.setDword(out + 0x14, result);
    store(guest, out + 0x18, fp::add(fp::mul(floatFrom(guest, m + 0x18), c), fp::mul(floatFrom(guest, m), s)));
    store(guest, out + 0x1c, fp::add(fp::mul(floatFrom(guest, m + 4), s), fp::mul(floatFrom(guest, m + 0x1c), c)));
    store(guest, out + 0x20, fp::add(fp::mul(floatFrom(guest, m + 8), s), fp::mul(floatFrom(guest, m + 0x20), c)));
    return result;
}

/* sub_4eab80: about its third row. */
x86::reg32 frontend::simd::rotateZ(App* app, CPU& cpu, reg32 matrix, reg32 angleBits, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, kTurnFrame);
    const reg32 m = turnSource(guest, matrix, out);
    const float angle = floatOf(angleBits);
    const Float s = Float(float(sineOf(guest, angle))), c = Float(float(cosineOf(guest, angle)));
    store(guest, out, fp::add(fp::mul(floatFrom(guest, m + 0x0c), s), fp::mul(floatFrom(guest, m), c)));
    store(guest, out + 4, fp::add(fp::mul(floatFrom(guest, m + 4), c), fp::mul(floatFrom(guest, m + 0x10), s)));
    store(guest, out + 8, fp::add(fp::mul(floatFrom(guest, m + 8), c), fp::mul(floatFrom(guest, m + 0x14), s)));
    for (reg32 i = 0; i < 3; ++i)
        store(guest, out + 0x0c + 4 * i, fp::sub(fp::mul(floatFrom(guest, m + 0x0c + 4 * i), c), fp::mul(floatFrom(guest, m + 4 * i), s)));
    guest.setDword(out + 0x18, guest.dword(m + 0x18));
    guest.setDword(out + 0x1c, guest.dword(m + 0x1c));
    const reg32 result = guest.dword(m + 0x20);
    guest.setDword(out + 0x20, result);
    return result;
}

void frontend::simRotateX(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea9e0", 8, 0, nullptr, 0 };
    const reg32 matrix = cpu.eax, angle = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::rotateX(app, cpu, matrix, angle, out); });
}

void frontend::simRotateY(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4eaab0", 8, 0, nullptr, 0 };
    const reg32 matrix = cpu.eax, angle = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::rotateY(app, cpu, matrix, angle, out); });
}

void frontend::simRotateZ(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4eab80", 8, 0, nullptr, 0 };
    const reg32 matrix = cpu.eax, angle = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::rotateZ(app, cpu, matrix, angle, out); });
}

void frontend::simFrameOfTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_496a50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 points = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::frameOfTriangle(app, cpu, points, out); });
}

void frontend::simFixedNormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4970c0", 0, 0, nullptr, 0 };
    const reg32 vector = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::fixedNormalise(app, cpu, vector); });
}

/* sub_4f44e0: nine ints turned over, eax to edx; the two the same, in place. */
x86::reg32 frontend::simd::fixedTranspose(App* app, CPU& cpu, reg32 from, reg32 to)
{
    Guest guest(app, cpu);
    if (from != to)
    {
        fixedTransposeInto(guest, from, to);
        return guest.dword(from + 0x20);
    }
    reg32 swapped = 0;
    for (const auto& pair : { std::array<reg32, 2>{ 0x04, 0x0c }, std::array<reg32, 2>{ 0x08, 0x18 }, std::array<reg32, 2>{ 0x14, 0x1c } })
    {
        const reg32 first = guest.dword(from + pair[0]);
        swapped = guest.dword(from + pair[1]);
        guest.setDword(from + pair[1], first);
        guest.setDword(from + pair[0], swapped);
    }
    guest.leaveEdx(swapped);
    return from;
}

void frontend::simFixedTranspose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f44e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::fixedTranspose(app, cpu, from, to); });
}

/* sub_4f4544: three 16.16 ints by nine, to ebx; edx the last sum's high word. */
x86::reg32 frontend::simd::fixedMatrixTimes(App* app, CPU& cpu, reg32 a, reg32 m, reg32 out)
{
    Guest guest(app, cpu);
    const FixedSum left = fixedMatrixTimesInto(guest, a, m, out);
    guest.leaveEdx(left.edx);
    return left.eax;
}

void frontend::simFixedMatrixTimes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f4544", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 a = cpu.eax, m = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::fixedMatrixTimes(app, cpu, a, m, out); });
}

void frontend::simTrackPointOnPlane(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4969c0", 24, 0, nullptr, 0 };
    const reg32 matrix = cpu.eax, words = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::trackPointOnPlane(app, cpu, matrix, words); });
}

void frontend::simPlaceInLane(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9bc0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdi };
    const reg32 index = cpu.eax, lane = cpu.edx, away = cpu.ebx, out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::placeInLane(app, cpu, index, lane, away, out); });
}

// ---------------------------------------------------------------------------
// The race's list of tasks
// ---------------------------------------------------------------------------

void frontend::simTaskListNew(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4be1f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 most = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::taskListNew(app, cpu, most); });
}

void frontend::simTaskListAdd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4be250", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 list = cpu.eax, function = cpu.edx, argument = cpu.ebx, priority = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::taskListAdd(app, cpu, list, function, argument, priority); });
}

/* sub_4bc910: the cameras' step and the HUD's, on the race's second list, at 100. */
x86::reg32 frontend::simd::raceAddTasks(App* app, CPU& cpu)
{
    constexpr reg32 kCamerasStep = 0x426680, kHudStep = 0x4cb2c0, kPriority = 0x64;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    guest::direct<0x4be250, taskListAdd>(app, cpu, guest.dword(kRaceTasksB), kCamerasStep, 0, kPriority);
    return guest::direct<0x4be250, taskListAdd>(app, cpu, guest.dword(kRaceTasksB), kHudStep, 0, kPriority);
}

void frontend::simRaceAddTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc910", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceAddTasks(app, cpu); });
}

/* sub_4bc8e0: out of the race's frame: its end, the game kept alive, a flag down. */
x86::reg32 frontend::simd::raceLeaveTasks(App* app, CPU& cpu)
{
    constexpr reg32 kDrawing = 0x7a3d10;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    guest::direct<0x4d5720, simc::raceFrameEnd>(app, cpu);
    // The game's code writes edx, zeroed before the call, which keeps it.
    guest.leaveEdx(0);
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    guest.setDword(kDrawing, 0);
    fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    return fe::direct<0x4bbd70, rendd::arenaRewind>(app, cpu);
}

void frontend::simRaceLeaveTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc8e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceLeaveTasks(app, cpu); });
}

void frontend::simRaceOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c45f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceOpen(app, cpu); });
}

void frontend::simRaceRestart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c4740", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceRestart(app, cpu); });
}

void frontend::simNetSendPlace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c7bd0", 0, 0, nullptr, 0 };
    const reg32 eax = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::netSendPlace(app, cpu, eax); });
}

void frontend::simHudDefaults(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_485880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::hudDefaults(app, cpu); });
}

// ---------------------------------------------------------------------------
// One-line calls
// ---------------------------------------------------------------------------

/* sub_439cc0: whether a car is a locked bonus car: none without its record. */
x86::reg32 frontend::simd::bonusCarLocked(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kLockedRecord = 0x439c10;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, car);
    // The record in eax, as the call before left it there.
    return record != 0 ? guest.call(kLockedRecord, args({ record })) : record;
}

void frontend::simBonusCarLocked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_439cc0", 0, 0, nullptr, 0, kRestoreNone };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::bonusCarLocked(app, cpu, car); });
}

/* sub_49bfb0: eax's bits added to the flags at 0x55e5ec. */
x86::reg32 frontend::simd::orFlags(App* app, CPU& cpu, reg32 bits)
{
    constexpr reg32 kFlags = 0x55e5ec;
    Guest guest(app, cpu);
    guest.setDword(kFlags, guest.dword(kFlags) | bits);
    return bits;
}

void frontend::simOrFlags(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bfb0", 0, 0, nullptr, 0 };
    const reg32 bits = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::orFlags(app, cpu, bits); });
}

/* sub_4a4020: as the game ends, the web page the choice at 0x55e894 asks for. */
x86::reg32 frontend::simd::openWebPage(App* app, CPU& cpu)
{
    constexpr reg32 kChoice = 0x55e894, kShellOpen = 0x418cc0, kHomePage = 0x53ce9c, kDownloadPage = 0x53ceb8;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 choice = guest.dword(kChoice);
    if (choice == 1)
        return guest.call(kShellOpen, args({ kHomePage }));
    if (choice == 2)
        return guest.call(kShellOpen, args({ kDownloadPage }));
    return choice;
}

void frontend::simOpenWebPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a4020", 0, 0, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::openWebPage(app, cpu); });
}

/* sub_4bebb0: the clip rectangle kept and set (sub_4bebe0, handed the four as they came). */
x86::reg32 frontend::simd::setClipRect(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom)
{
    constexpr reg32 kKept = 0x7cda60, kSetClip = 0x4bebe0;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    guest.setDword(kKept, left);
    guest.setDword(kKept + 4, top);
    guest.setDword(kKept + 8, right);
    guest.setDword(kKept + 0x0c, bottom);
    return guest.call(kSetClip, args({ left, top, right, bottom }));
}

void frontend::simSetClipRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bebb0", 0, 0, nullptr, 0, kRestoreNone };
    const reg32 left = cpu.eax, top = cpu.edx, right = cpu.ebx, bottom = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::setClipRect(app, cpu, left, top, right, bottom); });
}

/* sub_4c7b10: under another name, the depth's offsets set (sub_4c7a60), every register as it came. */
x86::reg32 frontend::simd::depthOffsets(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return guest.call(0x4c7a60);
}

void frontend::simDepthOffsets(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c7b10", 0, 0, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::depthOffsets(app, cpu); });
}

x86::reg32 frontend::simd::raceClearA(App* app, CPU& cpu)
{
    constexpr reg32 kBlock = 0x7dd850, kBytes = 0x280, kFillZero = 0x4e070c;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return guest.call(kFillZero, args({ kBlock, kBytes }));
}

void frontend::simRaceClearA(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9d70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceClearA(app, cpu); });
}

void frontend::simRaceClearB(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9db0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::raceClearB(app, cpu); });
}

/* sub_4dbda0: the projection of a view (sub_4bed20): the view's own half of the screen with
 * two players, else the whole screen's. */
x86::reg32 frontend::simd::viewProjection(App* app, CPU& cpu, reg32 camera)
{
    constexpr reg32 kViews = 0x7cda80, kWholeScreen = 0x7cdb88, kHalfAngle = 0x78;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 view = gameSettings(guest).playMode == 1 ? (guest.dword(camera + 4) << 5) + kViews : kWholeScreen;
    return fe::direct<0x4bed20, loopb::useWithProjection>(app, cpu, view, kHalfAngle, gameSettings(guest).mirrored);
}

void frontend::simViewProjection(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dbda0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::viewProjection(app, cpu, camera); });
}

/* sub_4dcd10: under another name, the weather's step (sub_4dcdf0), every register as it came. */
x86::reg32 frontend::simd::weatherStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return fe::direct<0x4dcdf0, tail::weatherStart>(app, cpu);
}

void frontend::simWeatherStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dcd10", 0, 0, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::weatherStep(app, cpu); });
}

/* sub_4dcdd0: how hard it rains, none without the weather on. */
x86::reg32 frontend::simd::rainAmount(App* app, CPU& cpu)
{
    constexpr reg32 kRain = 0x8ca22c;
    Guest guest(app, cpu);
    return gameSettings(guest).weather != 0 ? guest.dword(kRain) : 0;
}

void frontend::simRainAmount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dcdd0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return simd::rainAmount(app, cpu); });
}

}
