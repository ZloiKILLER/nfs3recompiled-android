#include "game_settings.h"
#include "native_aidrive.h"
#include "native_drivetrain.h"
#include "native_frontend_loopb.h"
#include "native_frontend_sima.h"
#include "native_frontend_simd.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_physics.h"
#include "native_vertices.h"
#include "diag_hooks.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>
#include <utility>

/* A car's physics, frame by frame: the step the race calls for each car
 * (carPhysics, whose address raceload stores at 0x42c429), the full step
 * (physicsStep) and the parts of it that are this file's -- the car's speed
 * in its own axes, its slip, the steering's angle, the spin's torque, the
 * damping of its speed and spin when it idles, coasts or is held still, its
 * grip sideways, the handbrake's drag, the yaw's damping.  The engine and the
 * gears (0x4aaed1), the driver's inputs (0x4a9bc4), the wheels' loads
 * (0x4aab41) and forces (0x4aca4f), and the car at rest (0x4aa8e7) are other
 * files' and are called by address.  The game's code calls each by the name it has here.
 *
 * The module (0x4a78d6 to 0x4af3a1) was compiled without optimisation: every
 * function opens with push ebx..ebp, mov ebp, esp and its frame, keeps the
 * car (eax on entry) in a local and reloads it before each use, and does its
 * x87 work through memory.  What these functions read of the car: +0xa4 its
 * speed's vector (world), +0xb0 and +0x100 the two its spin's torque is
 * scaled by, +0xe4 its spin (world), +0x134 how firmly it stands, +0x160 the
 * track's kind (1 and 2 the damped ones), +0x164 the race's stage (0
 * racing), +0x184 a slice it is to be put back on, +0x18c its surface,
 * +0x190 its axes (a 3x3 matrix), +0x1a0 the wheels' share, +0x200 its flags
 * (2: the AI drives it), +0x20c its surface's grip, +0x220 its choice
 * (+0x58 a tuning), +0x2e4..+0x2fc the step's acceleration and spin's
 * change, +0x4e0..+0x4e8 the force in its axes, +0x4ec..+0x4f4 its speed in
 * its axes (across, up, forward), +0x4f8..+0x500 its spin in its axes,
 * +0x508..+0x51c the driver's inputs (+0x508 and +0x509 the pedals, +0x50b
 * the gear, +0x50d the handbrake, +0x51c the steering, signed), +0x524 the
 * frames since it was put back, +0x528 its handling record, +0x590 the
 * engine's revs, +0x594 a flag of the wheels, +0x59c, +0x5a4 its speed on
 * the ground, signed as it goes, +0x5b0 its slip (across over forward),
 * +0x5bc the load's shift front to back, +0x5c0..+0x5d4 the wheels' loads,
 * +0x5d8 four wheels' grips, +0x5ec and +0x5f0 the pedals' travel, +0x5f4.
 *
 * Each is a typed function -- the car as a parameter, C++ locals, what it
 * gives back returned -- with a thunk under the game's name for the
 * callers still on registers and the dispatch, which leaves eax, the x87's
 * stack and its status word as the game's code does.  The x87's arithmetic is fp::'s, in the code's order, each store to
 * a float rounded where the code stores one.  Part of physicsStep's frame
 * (the car's axes, the four wheels' records) is handed by address to other
 * files' functions (0x4e0110, 0x4aca4f), so it stays on the guest's stack
 * where the game has it (GuestFrame); the rest is C++ locals.  Every
 * other function of the game's is called by address; the game's constants
 * (floats and doubles in its data, 0x53de34 to 0x53df34) are read where the
 * code reads them. */
namespace nfs3hp
{
namespace physics
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using i32 = std::int32_t;

// The race's: whether two players share a screen or it is a network race,
// the race's type (1 and 2 the series), the frame counter (the car's speed
// in its own axes from 0x200 on, after the countdown), a word compared and
// never used.
constexpr u32 kRaceFrame = 0x7d3684;
// Gravity in the car's axes, three floats, worked out each step.
constexpr u32 kGravity = 0x7a3a40, kGravityY = 0x7a3a44, kGravityZ = 0x7a3a48;
// The surfaces' grips, a float each by the car's surface.
constexpr u32 kSurfaceGrip = 0x55ed30;
// What these call by address: a vector's length, vectors scaled.  The AI's
// step, the car at rest, the engine, the driver's inputs, the wheels' loads and
// forces are native_aidrive.h's, native_handling.h's and native_drivetrain.h's,
// called as C++ (guest::direct), and so are a matrix transposed, two floats'
// length and the car's sounds (simd's, sima's, loopb's: feCore); so are
// vectors scaled (native_vertices.h's) where the
// eax they leave is not what the caller leaves -- where it is, and for the
// vector's length (dampYaw leaves its eax), they stay by address: vertices'
// cores give back nothing of eax.

u16 half(App* app, u32 a) { return app->getMemory<u16>(a); }
nfs::Float f32(App* app, u32 a) { return nfs::Float(app->getMemory<float>(a)); }
nfs::Float f64(App* app, u32 a) { return nfs::Float(app->getMemory<double>(a)); }

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.
 * A u32 result is the eax the function leaves, set to it; after any other, eax is left alone. */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    const auto run = [&] {
        return guest::direct<Address, Core>(app, ts, args...);
    };
    using Result = decltype(run());
    const auto spoil = [&] {};
    if constexpr (std::is_void_v<Result>)
    {
        try
        {
            run();
        }
        catch (const fe::Terminated&)
        {
            return false;
        }
        spoil();
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = run();
            if constexpr (std::is_same_v<Result, nfs::u32>)
                (void)result;  // a menu core's result: the eax it leaves is no longer set (no caller reads it)
            else
                spoil();
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* Room on the guest's stack for what a callee is handed the address of: the game's frame
 * (`pushed` bytes of registers under esp on entry, then `bytes`), esp under it while the core
 * runs and back after (as native_collide.cpp's). */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

// The car's fields the typed functions name (the file's comment above says what each is).
constexpr u32 kCarVelocity = 0xa4, kCarSpin = 0xe4, kCarSpinUp = 0xe8;
constexpr u32 kCarSpinScale = 0xb0, kCarSpinScaleToo = 0x100;
constexpr u32 kCarRestFlag = 0x8a, kCarRestMeasure = 0x10c;  // a byte set: at rest; else a float under 0.9: at rest
constexpr u32 kCarTrackKind = 0x160, kCarStage = 0x164, kCarFlags = 0x200, kCarChoice = 0x220;
constexpr u32 kCarForceUp = 0x4e4, kCarForceForward = 0x4e8;
constexpr u32 kCarSpeedAcross = 0x4ec, kCarSpeedForward = 0x4f4, kCarYaw = 0x4fc;
constexpr u32 kCarThrottle = 0x508, kCarBrake = 0x509, kCarGear = 0x50b, kCarHandbrake = 0x50d, kCarSteering = 0x51c;
constexpr u32 kCarHandling = 0x528, kCarRevs = 0x590, kCarWheelFlag = 0x594, kCarGroundSpeed = 0x5a4;
constexpr u32 kCarSlip = 0x5b0, kCarFrontLoad = 0x5c0, kCarBackLoad = 0x5c4, kCarLoadShift = 0x5bc;
constexpr u32 kCarBrakeTravel = 0x5ec, kCarThrottleTravel = 0x5f0, kCarQuiet = 0x5f4;
constexpr u32 kCarAiDriven = 2;  // in kCarFlags
constexpr u32 kChoiceTuning = 0x58;
constexpr u32 kHandlingIdleRevs = 0x1c0, kHandlingLock = 0x23c, kHandlingMass = 0x248, kHandlingSpinSpeed = 0x258;
constexpr u32 kGearReverse = 0, kGearNeutral = 1;
// Floats' bits the module compares with as integers.
constexpr u32 kHalfBits = 0x3f000000, kThreeQuartersBits = 0x3f400000, kOneBits = 0x3f800000, kTwoBits = 0x40000000;
constexpr u32 kFourBits = 0x40800000, kFiveBits = 0x40a00000, kTwentyBits = 0x41a00000;

float getFloat(App* app, u32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, u32 a, float f) { app->getMemory<float>(a) = f; }

u32 floatBits(float f)
{
    u32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

float floatOf(u32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

/* Three floats, as the car's vectors and the frame's hold them. */
using Vector = std::array<float, 3>;

Vector vectorAt(App* app, u32 a)
{
    return Vector{getFloat(app, a), getFloat(app, a + 4), getFloat(app, a + 8)};
}

void putVector(App* app, u32 a, const Vector& v)
{
    for (u32 k = 0; k < 3; ++k)
        putFloat(app, a + 4 * k, v[k]);
}

/* The product the module inlines wherever a vector goes through a matrix (its rows at `matrix`,
 * three floats each): [j] = (v0 m[j] + v1 m[3 + j]) + v2 m[6 + j], each a float -- the x87's sums
 * in the code's order (three products a row, faddp into the row before). */
Vector turned(App* app, const Vector& v, u32 matrix)
{
    Vector out;
    for (u32 j = 0; j < 3; ++j)
    {
        const nfs::Float first = fp::mul(nfs::Float(v[0]), f32(app, matrix + 4 * j));
        const nfs::Float second = fp::mul(nfs::Float(v[1]), f32(app, matrix + 0xc + 4 * j));
        const nfs::Float third = fp::mul(nfs::Float(v[2]), f32(app, matrix + 0x18 + 4 * j));
        out[j] = float(fp::add(fp::add(first, second), third));
    }
    return out;
}

/* The vector at `vector` through the matrix at `matrix`, to `to` (read whole before it is written). */
void turnInto(App* app, u32 vector, u32 matrix, u32 to)
{
    putVector(app, to, turned(app, vectorAt(app, vector), matrix));
}

/* The module's size of a float: negated only where under 0 (so never a NaN or -0), the rest kept
 * to the bit. */
float sizeOf(float value)
{
    return 0.0 > nfs::Float(value) ? float(-nfs::Float(value)) : value;
}

/* What the module's size of `value` leaves in eax: the car reloaded where it negates, else the
 * value's bits. */
u32 sizeLeaves(u32 car, float value)
{
    return 0.0 > nfs::Float(value) ? car : floatBits(value);
}

/* The module's compares of a float with a constant, as their bits are integers: a NaN or -0 by
 * its sign.  Kept as bits. */
bool bitsOver(float value, u32 bits) { return i32(floatBits(value)) > i32(bits); }
bool bitsUnder(float value, u32 bits) { return i32(floatBits(value)) < i32(bits); }

/* An int's size the module's way: the lowest int stays itself. */
u32 integerSize(u32 value) { return i32(value) < 0 ? 0u - value : value; }

/* The damped tracks: the track's kind 1 or 2. */
bool dampedTrack(App* app, u32 car)
{
    const u32 kind = at(app, car + kCarTrackKind);
    return kind == 1 || kind == 2;
}

/* A float field times a constant of the game's, back to it. */
void scaleBy(App* app, u32 field, u32 factor)
{
    putFloat(app, field, float(fp::mul(f32(app, field), f32(app, factor))));
}

/* The x87's status word as the last fcom here leaves it (c3, c2, c0: fp::compare's): kept for the
 * thunks' fnstsw ax. */
nfs::u16 g_status = 0;

/* fldz; fcomp value: the x87's status word as the module's compares with 0 leave it. */
void compareWithZero(float value)
{
    g_status = fp::compare(nfs::Float(0.0), nfs::Float(value));
}

/* 0x4e0050 for one vector, its own: the floats at `vector` times the float of `factorBits`.  The
 * callee's eax (the vector past its end) back; nullopt when the game stopped.  For the callers that
 * leave its eax. */
std::optional<u32> scaleVectorBy(App* app, ThreadState& ts, u32 vector, u32 factorBits)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, vector, floatOf(factorBits), vector);
    if (ts.terminate)
        return std::nullopt;
    return vector + 12;
}

/* 0x4967e0 (sima's), stdcall: the game's quick length of two floats; nullopt when the game
 * stopped. */
std::optional<nfs::Float> planarLength(App* app, ThreadState& ts, float first, float second)
{
    return feCore<0x4967e0, frontend::sima::planarLengthOf>(app, ts, floatBits(first), floatBits(second));
}

/* 0x4e0050 (vertices') for one vector, its own: the floats at `vector` times the float of
 * `factorBits`, as C++ (guest::direct), for a caller that does not leave what it leaves in eax.
 * False when the game stopped. */
bool scaleVectorOnly(App* app, ThreadState& ts, u32 vector, u32 factorBits)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, vector, floatOf(factorBits), vector);
    return !ts.terminate;
}

/* 0x4e0110 (simd's): the matrix at `matrix` transposed to `to`; false when the game stopped. */
bool transposeTo(App* app, ThreadState& ts, u32 matrix, u32 to)
{
    return feCore<0x4e0110, frontend::simd::transpose>(app, ts, matrix, to).has_value();
}


}  // namespace

nfs::u32 clearWheelForces(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 steerAngle(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sideSlip(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 groundSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 physicsStep(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carPhysics(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 settleLoadShift(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 driveForce(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 holdStill(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sideGrip(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 handbrakeDrag(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 yawDamping(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 idleDamping(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 coastDamping(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 spinTorque(App* app, nfs::ThreadState& ts, nfs::Args& args);

/* A wheel's record of the step's (0x30 bytes, in physicsStep's frame) cleared, all twelve of its
 * dwords: +0 to +0xc, +0x1c to +0x24, +0x10 to +0x18, +0x28 and +0x2c, in that order. */
void clearWheelRecord(App* app, u32 record)
{
    for (const u32 offset : {0x0u, 0x4u, 0x8u, 0xcu, 0x1cu, 0x20u, 0x24u, 0x10u, 0x14u, 0x18u, 0x28u, 0x2cu})
        put(app, record + offset, 0);
}

/* 0x4ad250: clearWheelRecord for the record at eax, which it leaves in eax. */
nfs::u32 clearWheelForces(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    clearWheelRecord(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kHandbrakeCutScale = 0x53de34, kHandbrakeCutBase = 0x53de38;
constexpr u32 kHandbrakeCutLeast = 0x3f0ccccd;  // 0.55
constexpr u32 kFasterBits = 0x41d5999a, kFastBits = 0x41566666;  // 26.7 and 13.4 forward
constexpr u32 kFasterSlipOver = 0x3d4ccccd, kFasterSlipBelow = 0x53de3c, kFasterSlipScale = 0x53de44;  // 0.05, doubles
constexpr u32 kFastSlipOver = 0x3e19999a, kFastSlipBelow = 0x53de4c, kFastSlipScale = 0x53de54;        // 0.15
constexpr u32 kSpeedyBits = 0x42200000;  // 40
constexpr u32 kSpeedShare = 0x53de5c;    // a double
constexpr u32 kShareMostBits = 0x3fc00000;  // 1.5
constexpr u32 kSteeringScale = 0x53de64, kLockScale = 0x53de68, kLockDivisor = 0x53de6c;
constexpr u32 kAngleLeast = 0x53de74;  // a double the code compares the angle with, for a clamp it never uses

/* The steering's angle -- 0 while the race is not under way, else the steering [+0x51c] as a
 * float, cut down: with the handbrake on while the car moves, by 0.x the faster it goes (no
 * less than 0.55) where the slip [+0x5b0] and the steering lean the same way; without it past
 * 26.7 and past 13.4 forward, by the slip's size where the slip is past 0.05 or 0.15 or the
 * steering against it; past 40 by the speed's share (1 to 1.5); then by the constants and the
 * handling's lock [+0x23c].  What the code clamps to -1..1 is never used: the unclamped angle is
 * what it gives back, as the code does (and its cmp of [0x6fd494], whose flags nothing reads). */
float steeringAngle(App* app, u32 car)
{
    if (half(app, car + kCarStage) != 0)
        return 0.0f;
    float angle = float(fp::add(nfs::Float(nfs::s32(at(app, car + kCarSteering))), 0.0));
    // The x87's "below" of a compare with 0, as the code's jae has it: over 0, or a NaN.
    const auto overZero = [](float value) { return !(0.0 >= nfs::Float(value)); };
    const auto underZero = [](float value) { return 0.0 > nfs::Float(value); };
    if (byte(app, car + kCarHandbrake) != 0 && overZero(sizeOf(getFloat(app, car + kCarSpeedForward))))
    {
        // The slip and the steering the same way?
        bool cut = false;
        if (underZero(getFloat(app, car + kCarSlip)))
            cut = overZero(angle);
        if (!cut && overZero(getFloat(app, car + kCarSlip)))
            cut = underZero(angle);
        if (cut)
        {
            const nfs::Float size = nfs::Float(sizeOf(getFloat(app, car + kCarSpeedForward)));
            const float factor = float(fp::sub(f32(app, kHandbrakeCutBase), fp::mul(size, f32(app, kHandbrakeCutScale))));
            const float least = bitsOver(factor, kHandbrakeCutLeast) ? factor : 0.55f;
            angle = float(fp::mul(nfs::Float(angle), nfs::Float(least)));
        }
    }
    else
    {
        // Past 26.7 forward, or past 13.4: the slip's own limits.
        const float forward = getFloat(app, car + kCarSpeedForward);
        const bool faster = bitsOver(forward, kFasterBits);
        if (faster || bitsOver(forward, kFastBits))
        {
            const u32 slipOver = faster ? kFasterSlipOver : kFastSlipOver;
            const u32 slipBelow = faster ? kFasterSlipBelow : kFastSlipBelow;
            const u32 slipScale = faster ? kFasterSlipScale : kFastSlipScale;
            bool cut = false;
            if (bitsOver(getFloat(app, car + kCarSlip), slipOver))
                cut = overZero(angle);
            // The slip under the double, or a NaN: the x87's carry.
            if (!cut && !(f32(app, car + kCarSlip) >= f64(app, slipBelow)))
                cut = underZero(angle);
            if (cut)
            {
                const float factor = float(fp::mul(nfs::Float(sizeOf(getFloat(app, car + kCarSlip))), f32(app, slipScale)));
                if (bitsUnder(factor, kOneBits))
                    angle = float(fp::mul(nfs::Float(angle), nfs::Float(factor)));
            }
        }
    }
    // Past 40, by the speed's share, 1 to 1.5.
    if (bitsOver(sizeOf(getFloat(app, car + kCarSpeedForward)), kSpeedyBits))
    {
        float share = float(fp::mul(nfs::Float(sizeOf(getFloat(app, car + kCarSpeedForward))), f64(app, kSpeedShare)));
        share = bitsOver(share, kOneBits) ? share : 1.0f;
        share = bitsOver(share, kShareMostBits) ? 1.5f : share;
        angle = float(fp::div(nfs::Float(angle), nfs::Float(share)));
    }
    angle = float(fp::mul(nfs::Float(angle), f32(app, kSteeringScale)));
    const u32 handling = at(app, car + kCarHandling);
    const nfs::Float lock = fp::div(fp::mul(f32(app, handling + kHandlingLock), f32(app, kLockScale)), f32(app, kLockDivisor));
    return float(fp::mul(lock, nfs::Float(angle)));
}

/* 0x4ad2e1: steeringAngle for car eax onto the x87's stack.  While racing the angle's bits left
 * in eax and the status word as its compare with the double at 0x53de74 leaves it; else the car
 * in eax. */
nfs::u32 steerAngle(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const float angle = steeringAngle(app, car);
    if (half(app, car + kCarStage) == 0)
    {
        g_status = fp::compare(nfs::Float(angle), f64(app, kAngleLeast));
        args.eax = floatBits(angle);
    }
    args.floatSet = true;
    args.floatResult = double(nfs::Float(angle));
    ts.sp += 4;
    return args.eax;
}

/* The car's slip -- its speed across over its speed forward -- where it goes faster than 5
 * either way, else 0. */
void updateSlip(App* app, u32 car)
{
    const float forward = getFloat(app, car + kCarSpeedForward);
    if (bitsOver(sizeOf(forward), kFiveBits))
        putFloat(app, car + kCarSlip, float(fp::div(f32(app, car + kCarSpeedAcross), nfs::Float(forward))));
    else
        put(app, car + kCarSlip, 0);
}

/* 0x4ad672: updateSlip for car eax, which it leaves in eax; the x87's status word as the size's
 * compare of the speed forward with 0 leaves it. */
nfs::u32 sideSlip(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    compareWithZero(getFloat(app, car + kCarSpeedForward));
    updateSlip(app, car);
    ts.sp += 4;
    return args.eax;
}

/* The car's speed on the ground -- the game's quick length (0x4967e0) of the sizes of its speeds
 * across and forward -- negative (its sign bit flipped) where it goes backwards.  False when the
 * game stopped. */
bool updateGroundSpeed(App* app, ThreadState& ts, u32 car)
{
    const float across = sizeOf(getFloat(app, car + kCarSpeedAcross));
    const float forward = sizeOf(getFloat(app, car + kCarSpeedForward));
    const std::optional<nfs::Float> length = planarLength(app, ts, across, forward);
    if (!length)
        return false;
    putFloat(app, car + kCarGroundSpeed, float(*length));
    if (0.0 > f32(app, car + kCarSpeedForward))
        putByte(app, car + kCarGroundSpeed + 3, u8(byte(app, car + kCarGroundSpeed + 3) ^ 0x80));
    return true;
}

/* 0x4ad6eb: updateGroundSpeed for car eax.  The last compare's (the speed forward with 0) status
 * word in the x87 and in ax, overwritten by the car in eax where it goes backwards. */
nfs::u32 groundSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (!updateGroundSpeed(app, ts, car))
        return args.eax;
    args.eax = car;
    compareWithZero(getFloat(app, car + kCarSpeedForward));
    args.eax = (args.eax & 0xffff0000u) | u32(g_status);
    if (0.0 > f32(app, car + kCarSpeedForward))
        args.eax = car;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kRestMeasureBits = 0x3f666666;  // 0.9
constexpr u32 kRevsSettle = 0x1f4;            // the revs fall by 500 a step at rest, to the idle's

/* What carPhysics's frame took of the guest's stack in the game's code: six registers saved and
 * its 8 bytes of locals, under them the call's return address.  physicsStep hands its callees
 * addresses in its own frame (the axes, the wheels' records), so that frame is put where the
 * game's code has it. */
constexpr u32 kCarPhysicsFrame = 0x20;

std::optional<u32> stepPhysics(App* app, ThreadState& ts, u32 car);

/* stepPhysics for car at the game's code's depth: its eax back; nullopt when the game stopped. */
std::optional<u32> fullStep(App* app, ThreadState& ts, u32 car)
{
    const u32 esp = ts.sp;
    ts.sp -= kCarPhysicsFrame + 4;
    const std::optional<u32> left = stepPhysics(app, ts, car);
    ts.sp = esp;
    return left;
}

/* The step the race calls for each car: the AI's step for a car the AI drives (0x4088f0); else a
 * car at rest ([+0x8a] set, or [+0x10c] under 0.9) has its wheels' flag, slip and loads cleared,
 * its revs fall to the idle's, and is settled (0x4aa8e7); any other has its full step; then its
 * sounds (0x4cdf10) in gear where [+0x5f4] is 0.  Returns what the game's code leaves in eax,
 * which nothing reads, nullopt when the game stopped. */
std::optional<u32> stepCar(App* app, ThreadState& ts, u32 car)
{
    if ((byte(app, car + kCarFlags) & kCarAiDriven) != 0)
        return guest::direct<0x4088f0, aidrive::stepCar>(app, ts, car);
    const bool rest = byte(app, car + kCarRestFlag) != 0 || bitsUnder(getFloat(app, car + kCarRestMeasure), kRestMeasureBits);
    if (rest)
    {
        for (const u32 offset : {kCarWheelFlag, kCarSlip, kCarFrontLoad, kCarBackLoad})
            put(app, car + offset, 0);
        const u32 idle = at(app, at(app, car + kCarHandling) + kHandlingIdleRevs);
        if (i32(idle + kRevsSettle) < i32(at(app, car + kCarRevs)))
            put(app, car + kCarRevs, at(app, car + kCarRevs) - kRevsSettle);
        else
            put(app, car + kCarRevs, idle);
        if (!guest::direct<0x4aa8e7, handling::settleCar>(app, ts, car))
            return std::nullopt;
    }
    else if (!fullStep(app, ts, car))
        return std::nullopt;
    u32 left = car;
    if (at(app, car + kCarQuiet) == 0 && byte(app, car + kCarGear) != 0)
    {
        const std::optional<u32> sounds = feCore<0x4cdf10, frontend::loopb::performanceTimes>(app, ts, car);
        if (!sounds)
            return std::nullopt;
        left = *sounds;
    }
    if (diag::carStepped)
        diag::carStepped(app, car);
    return left;
}

/* 0x4ae4c1: stepCar for car eax. */
nfs::u32 carPhysics(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = stepCar(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kShiftBackSettle = 0x53dea4, kShiftBackSettleDamped = 0x53dea8;
constexpr u32 kShiftForwardSettle = 0x53deac, kShiftForwardSettleDamped = 0x53deb0;

/* The load's shift [+0x5bc] settling: a shift back (under 0)
 * times 0x53dea4, and 0x53dea8 more on the damped tracks ([+0x160] 1 or 2);
 * a shift forward times 0x53deac with the wheels' flag [+0x594] 1, and
 * 0x53deb0 more on those tracks. */
void settleLoad(App* app, u32 car)
{
    const u32 shift = car + kCarLoadShift;
    const nfs::Float value = f32(app, shift);
    if (0.0 > value)
    {
        scaleBy(app, shift, kShiftBackSettle);
        if (dampedTrack(app, car))
            scaleBy(app, shift, kShiftBackSettleDamped);
        return;
    }
    // Over 0, or a NaN: the x87's carry, as the code's jae has it.
    if (0.0 >= value)
        return;
    if (at(app, car + kCarWheelFlag) == 1)
        scaleBy(app, shift, kShiftForwardSettle);
    if (dampedTrack(app, car))
        scaleBy(app, shift, kShiftForwardSettleDamped);
}

/* 0x4ae5c2: settleLoad for car eax; the x87's status word as the shift's compare with 0 leaves it, and in
 * ax over the car where the shift is 0 (the code's second compare, which it leaves on). */
nfs::u32 settleLoadShift(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const float shift = getFloat(app, car + kCarLoadShift);
    compareWithZero(shift);
    if (nfs::Float(shift) == 0.0)
        args.eax = (args.eax & 0xffff0000u) | u32(g_status);
    settleLoad(app, car);
    ts.sp += 4;
    return args.eax;
}

/* The drive's force [+0x4e8] over the handling's mass [+0x248];
 * off the damped tracks, in a single race on one screen, more by the
 * choice's tuning (its +0x58: past 1 in the low gears, under 1 always);
 * [+0x4e4] cleared. */
constexpr u32 kTuningLowGears = 0x53deb4, kTuningShortfall = 0x53deb8, kTuningShortfallScale = 0x53debc;
constexpr u32 kLowGearsTop = 3;  // the gears' bytes up to which the tuning past 1 counts

void applyDriveForce(App* app, u32 car)
{
    const u32 force = car + kCarForceForward;
    const u32 handling = at(app, car + kCarHandling);
    putFloat(app, force, float(fp::div(f32(app, force), f32(app, handling + kHandlingMass))));
    const u32 kind = at(app, car + kCarTrackKind);
    if (kind != 1 && kind != 2 && i32(gameSettings(app).playMode) <= 1 && gameSettings(app).raceType != 1
        && gameSettings(app).raceType != 2)
    {
        const u32 tuning = at(app, car + kCarChoice) + kChoiceTuning;
        if (bitsOver(getFloat(app, tuning), kOneBits) && byte(app, car + kCarGear) <= kLowGearsTop)
        {
            const nfs::Float more = fp::add(fp::mul(f32(app, tuning), f32(app, kTuningLowGears)), 1.0);
            putFloat(app, force, float(fp::mul(more, f32(app, force))));
        }
        else if (bitsUnder(getFloat(app, tuning), kOneBits))
        {
            const nfs::Float less =
                fp::add(fp::mul(fp::add(f32(app, tuning), f32(app, kTuningShortfall)), f32(app, kTuningShortfallScale)), 1.0);
            putFloat(app, force, float(fp::mul(less, f32(app, force))));
        }
    }
    put(app, car + kCarForceUp, 0);
}

/* 0x4ae68e: applyDriveForce for car eax, which it leaves in eax. */
nfs::u32 driveForce(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    applyDriveForce(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4ae79b: the car held still: both pedals down ([+0x5f0] at least 0.5,
 * [+0x5ec] at least 0.75), the steering past 64 either way, and no faster
 * than 5 -- its speed scaled by 0 (0x4e0050).  Returns what the game's code
 * leaves in eax, which nothing reads, nullopt when the game stopped; the
 * x87's status word as the speed's compare with 0 leaves it, where it
 * gets that far. */
constexpr i32 kSteeringHeld = 0x40;

std::optional<u32> holdCarStill(App* app, ThreadState& ts, u32 car)
{
    if (bitsUnder(getFloat(app, car + kCarThrottleTravel), kHalfBits))
        return car;
    if (bitsUnder(getFloat(app, car + kCarBrakeTravel), kThreeQuartersBits))
        return car;
    const u32 steering = at(app, car + kCarSteering);
    if (i32(integerSize(steering)) <= kSteeringHeld)
        return steering;
    const float forward = getFloat(app, car + kCarSpeedForward);
    compareWithZero(forward);
    if (bitsOver(sizeOf(forward), kFiveBits))
        return sizeLeaves(car, forward);
    return scaleVectorBy(app, ts, car + kCarVelocity, 0);
}

nfs::u32 holdStill(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = holdCarStill(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kGripBase = 0x53dec0, kGripScale = 0x53dec4, kGripOffset = 0x53dec8;

/* The car's grip sideways: its speed across [+0x4ec] scaled by 0.9 up to 1, 0.99 past 2, and in
 * between by its size (0x53dec0, 0x53dec4, 0x53dec8; the code's clamp of that to 0.9..0.99 is
 * never used, as the code does). */
void gripSideways(App* app, u32 car)
{
    const float size = sizeOf(getFloat(app, car + kCarSpeedAcross));
    float grip;
    if (!bitsOver(size, kOneBits))
        grip = 0.9f;
    else if (bitsOver(size, kTwoBits))
        grip = 0.99f;
    else
    {
        const float scaled = float(fp::mul(fp::add(nfs::Float(size), f32(app, kGripBase)), f32(app, kGripScale)));
        grip = float(fp::add(nfs::Float(scaled), f32(app, kGripOffset)));
    }
    putFloat(app, car + kCarSpeedAcross, float(fp::mul(f32(app, car + kCarSpeedAcross), nfs::Float(grip))));
}

/* 0x4ae85b: gripSideways for car eax, which it leaves in eax; the x87's status word as the size's
 * compare of the speed across with 0 leaves it. */
nfs::u32 sideGrip(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    compareWithZero(getFloat(app, car + kCarSpeedAcross));
    gripSideways(app, car);
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kHandbrakeDragDamped = 0x53decc, kHandbrakeDrag = 0x53ded0, kHandbrakeDragNeutral = 0x53ded4;

/* The handbrake's drag ([+0x50d]) on the speed forward [+0x4f4]: past 1, times 0x53decc on the
 * damped tracks or 0x53ded0; under 1 and in neutral, times 0x53ded4. */
void dragHandbrake(App* app, u32 car)
{
    if (byte(app, car + kCarHandbrake) == 0)
        return;
    const u32 forward = car + kCarSpeedForward;
    const float size = sizeOf(getFloat(app, forward));
    if (bitsOver(size, kOneBits))
        scaleBy(app, forward, dampedTrack(app, car) ? kHandbrakeDragDamped : kHandbrakeDrag);
    else if (bitsUnder(size, kOneBits) && byte(app, car + kCarGear) == kGearNeutral)
        scaleBy(app, forward, kHandbrakeDragNeutral);
}

/* 0x4ae98b: dragHandbrake for car eax.  The car left in eax, but for the size's at exactly 1; the
 * x87's status word as the size's compare with 0 leaves it, with the handbrake on. */
nfs::u32 handbrakeDrag(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (byte(app, car + kCarHandbrake) != 0)
    {
        const float forward = getFloat(app, car + kCarSpeedForward);
        compareWithZero(forward);
        if (floatBits(sizeOf(forward)) == kOneBits)
            args.eax = sizeLeaves(car, forward);
    }
    dragHandbrake(app, car);
    ts.sp += 4;
    return args.eax;
}

constexpr i32 kSteeringStill = 4;  // the steering's size under it: the yaw damped in reverse
constexpr u32 kYawStillDamping = 0x53ded8;
constexpr u32 kSlipBits = 0x3e4ccccd;  // 0.2
constexpr u32 kYawShareMost = 0x3f7ae148;  // 0.98
constexpr u32 kSlowSpinScale = 0x53dedc, kSlowSpinScaleToo = 0x53dee4;  // doubles
constexpr u32 kSlowShareMost = 0x53deec, kSlowSlipDamping = 0x53def0, kSlowSkidDamping = 0x53def4;
constexpr u32 kSkidSpeed = 0x53defc;  // a double
constexpr u32 kSkidSpinScale = 0x53df04, kSkidSpinScaleToo = 0x53df0c;  // doubles
constexpr u32 kSkidShareMost = 0x53df14, kSkidSlipDamping = 0x53df18, kSkidSkidDamping = 0x53df1c;
constexpr u32 kSkidSlowDamping = 0x3f333333, kSkidFastDamping = 0x3f4ccccd;  // 0.7 and 0.8

/* 0x4e06a0: the length of the vector at `vector` onto the x87's stack, with what the callee
 * leaves in eax; nullopt when the game stopped. */
struct Length
{
    nfs::Float length;
    u32 eax;
};

std::optional<Length> vectorLength(App* app, ThreadState& ts, u32 vector)
{
    const double length = guest::direct<0x4e06a0, vertices::vectorLength>(app, ts, vector);
    if (ts.terminate)
        return std::nullopt;
    return Length{nfs::Float(length), vector};  // eax as it went in
}

/* The x87's answers of fcom a, b with the status word it leaves: below
 * (the carry, so for a NaN too), above (neither carry nor zero). */
bool compareBelow(const nfs::Float& a, const nfs::Float& b)
{
    g_status = fp::compare(a, b);
    return (g_status & 0x0100) != 0;
}

bool compareAbove(const nfs::Float& a, const nfs::Float& b)
{
    g_status = fp::compare(a, b);
    return (g_status & 0x4100) == 0;
}

/* The yaw damped by the speed's share of the spin's own speed, or by the slip: the spin's size by
 * the two scales and the handling's [+0x258] is the limit; slower than it forward and across, the
 * yaw times the speed forward's share of it (no more than `most`, else 0.98), else by the slip's
 * size, `slipDamping` under 0.2, else `skidDamping`.  The status word as its last compare leaves it. */
void dampYawByShare(App* app, ThreadState& ts, u32 car, u32 scale, u32 scaleToo, u32 most, u32 slipDamping,
                    u32 skidDamping)
{
    const u32 yaw = car + kCarYaw;
    const u32 handling = at(app, car + kCarHandling);
    const float limit = float(fp::mul(fp::mul(fp::mul(nfs::Float(sizeOf(getFloat(app, yaw))), f64(app, scale)),
                                              f32(app, handling + kHandlingSpinSpeed)),
                                      f64(app, scaleToo)));
    bool share = false;
    if (compareBelow(nfs::Float(sizeOf(getFloat(app, car + kCarSpeedForward))), nfs::Float(limit)))
        share = compareBelow(nfs::Float(sizeOf(getFloat(app, car + kCarSpeedAcross))), nfs::Float(limit));
    if (share)
    {
        const float ratio = sizeOf(float(fp::div(f32(app, car + kCarSpeedForward), nfs::Float(limit))));
        const float factor = compareAbove(f32(app, most), nfs::Float(ratio)) ? ratio : 0.98f;
        putFloat(app, yaw, float(fp::mul(f32(app, yaw), nfs::Float(factor))));
    }
    else
    {
        const float slip = getFloat(app, car + kCarSlip);
        compareWithZero(slip);
        scaleBy(app, yaw, bitsUnder(sizeOf(slip), kSlipBits) ? slipDamping : skidDamping);
    }
}

/* The yaw's damping, the spin in the car's axes [+0x4fc]: held still (both pedals down, steering
 * past 64, no faster than 5) it stops; with the steering under 4 either way, in reverse, times
 * 0x53ded8.  Slower across than forward: dampYawByShare with the slow constants.  Faster across,
 * and the speed in its axes under 0x53defc long (0x4e06a0): the same with the skid's, and the
 * speed scaled (0x4e0050) by 0.7 under 5, 0.8 past.  Returns what the game's code leaves in eax,
 * which nothing reads, nullopt when the game stopped; the x87's status word as its last compare
 * leaves it. */
std::optional<u32> dampYaw(App* app, ThreadState& ts, u32 car)
{
    const u32 yaw = car + kCarYaw;
    const bool held = !bitsUnder(getFloat(app, car + kCarThrottleTravel), kHalfBits)
                      && !bitsUnder(getFloat(app, car + kCarBrakeTravel), kThreeQuartersBits)
                      && i32(integerSize(at(app, car + kCarSteering))) > kSteeringHeld
                      && !bitsOver(sizeOf(getFloat(app, car + kCarSpeedForward)), kFiveBits);
    if (held)
        put(app, yaw, 0);
    else if (i32(integerSize(at(app, car + kCarSteering))) < kSteeringStill && byte(app, car + kCarGear) == kGearReverse)
        scaleBy(app, yaw, kYawStillDamping);

    if (compareBelow(nfs::Float(sizeOf(getFloat(app, car + kCarSpeedAcross))),
                     nfs::Float(sizeOf(getFloat(app, car + kCarSpeedForward)))))
    {
        dampYawByShare(app, ts, car, kSlowSpinScale, kSlowSpinScaleToo, kSlowShareMost, kSlowSlipDamping, kSlowSkidDamping);
        return car;
    }
    // Faster across than forward.
    const float forward = getFloat(app, car + kCarSpeedForward);
    if (!compareAbove(nfs::Float(sizeOf(getFloat(app, car + kCarSpeedAcross))), nfs::Float(sizeOf(forward))))
        return (sizeLeaves(car, forward) & 0xffff0000) | g_status;
    const std::optional<Length> length = vectorLength(app, ts, car + kCarSpeedAcross);
    if (!length)
        return std::nullopt;
    if (!compareBelow(length->length, f64(app, kSkidSpeed)))
        return (length->eax & 0xffff0000) | g_status;
    dampYawByShare(app, ts, car, kSkidSpinScale, kSkidSpinScaleToo, kSkidShareMost, kSkidSlipDamping, kSkidSkidDamping);
    const float speed = getFloat(app, car + kCarSpeedForward);
    compareWithZero(speed);
    return scaleVectorBy(app, ts, car + kCarVelocity, bitsUnder(sizeOf(speed), kFiveBits) ? kSkidSlowDamping : kSkidFastDamping);
}

/* 0x4aea88: dampYaw for car eax. */
nfs::u32 yawDamping(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = dampYaw(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kPedalIdle = 0x20;           // both pedals' bytes under it: idling
constexpr u32 kIdleDamping = 0x3f4ccccd;   // 0.8

/* The car idling while racing (both pedals [+0x508], [+0x509] under 0x20): slower than 4 in
 * reverse -- or slower than 5 in a gear past neutral -- its speed and spin times 0.8 (0x4e0050).
 * Returns what the game's code leaves in eax, which nothing reads, nullopt when the game stopped;
 * the x87's status word as the speed's compare with 0 leaves it, where it gets that far. */
std::optional<u32> dampIdle(App* app, ThreadState& ts, u32 car)
{
    if (half(app, car + kCarStage) != 0 || byte(app, car + kCarThrottle) >= kPedalIdle
        || byte(app, car + kCarBrake) >= kPedalIdle)
        return car;
    const float forward = getFloat(app, car + kCarSpeedForward);
    const float size = sizeOf(forward);
    compareWithZero(forward);
    // The size over 0 (or a NaN: the x87's carry, as the code's jae has it) and under 4.
    const bool slow = !(0.0 >= nfs::Float(size)) && bitsUnder(size, kFourBits);
    if (!(slow && byte(app, car + kCarGear) == kGearReverse))
    {
        if (!bitsUnder(size, kFiveBits))
            return sizeLeaves(car, forward);
        if (byte(app, car + kCarGear) <= kGearNeutral)
            return car;
    }
    if (!scaleVectorOnly(app, ts, car + kCarVelocity, kIdleDamping))
        return std::nullopt;
    return scaleVectorBy(app, ts, car + kCarSpin, kIdleDamping);
}

/* 0x4aeffc: dampIdle for car eax. */
nfs::u32 idleDamping(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = dampIdle(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kCoastDamping = 0x3f7d70a4, kCoastDampingFast = 0x3f7f7cee;  // 0.99, 0.998
constexpr u32 kSteeringStraight = 0x20;

/* The car coasting in neutral on the level (gravity forward [kGravityZ] under 0.5 either way):
 * its speed and spin times 0.99, or 0.998 past 20 with the steering within 32 (0x4e0050).
 * Returns what the game's code leaves in eax, which nothing reads, nullopt when the game stopped;
 * the x87's status word as its last compare with 0 leaves it. */
std::optional<u32> dampCoast(App* app, ThreadState& ts, u32 car)
{
    if (byte(app, car + kCarGear) != kGearNeutral)
        return car;
    const float slope = getFloat(app, kGravityZ);
    compareWithZero(slope);
    if (!bitsUnder(sizeOf(slope), kHalfBits))
        // The size's own way, but with no car to reload: the compare's word in ax over the car.
        return 0.0 > nfs::Float(slope) ? (car & 0xffff0000) | g_status : floatBits(slope);
    const float forward = getFloat(app, car + kCarSpeedForward);
    compareWithZero(forward);
    const bool fast = !bitsUnder(sizeOf(forward), kTwentyBits)
                      && i32(integerSize(at(app, car + kCarSteering))) <= i32(kSteeringStraight);
    const u32 factor = fast ? kCoastDampingFast : kCoastDamping;
    if (!scaleVectorOnly(app, ts, car + kCarVelocity, factor))
        return std::nullopt;
    return scaleVectorBy(app, ts, car + kCarSpin, factor);
}

/* 0x4af140: dampCoast for car eax. */
nfs::u32 coastDamping(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = dampCoast(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kSpinTorqueScale = 0x53df20, kShareScale = 0x53df24, kShareBase = 0x53df28;
constexpr u32 kShareLeast = 0x53df2c;  // a double, 0.1
constexpr u32 kReverseTorque = 0x53df34;
constexpr double kTenth = 0.1;  // what the code writes where the share is under kShareLeast: 0x3fb999999999999a

/* The ground speed's share of the spin's torque, before its clamp. */
nfs::Float speedShare(App* app, u32 car)
{
    return fp::add(fp::mul(nfs::Float(sizeOf(getFloat(app, car + kCarGroundSpeed))), f32(app, kShareScale)),
                   f32(app, kShareBase));
}

/* The spin's torque: 0 while the race is not under way; else -(|[+0xe8]| x 0x53df20 x ([+0xb0]
 * x [+0x100])), times the ground speed's share (the double 0.1 at least, 1 at most), times
 * 0x53df34 more in reverse; negated again. */
float spinTorqueOf(App* app, u32 car)
{
    float torque = 0.0f;
    if (half(app, car + kCarStage) == 0)
    {
        const nfs::Float spin = fp::mul(nfs::Float(sizeOf(getFloat(app, car + kCarSpinUp))), f32(app, kSpinTorqueScale));
        const nfs::Float scale = fp::mul(f32(app, car + kCarSpinScale), f32(app, car + kCarSpinScaleToo));
        torque = float(-nfs::Float(fp::mul(spin, scale)));
        // Not under the double (so never a NaN): the share itself, else 0.1; then to a float.
        const nfs::Float share = speedShare(app, car);
        const double least = share >= f64(app, kShareLeast) ? double(share) : kTenth;
        const float shareFloat = float(least);
        const float factor = bitsOver(shareFloat, kOneBits) ? 1.0f : shareFloat;
        torque = float(fp::mul(nfs::Float(torque), nfs::Float(factor)));
        if (byte(app, car + kCarGear) == kGearReverse)
            torque = float(fp::mul(nfs::Float(torque), f32(app, kReverseTorque)));
    }
    return float(-nfs::Float(torque));
}

/* 0x4af254: spinTorqueOf for car eax onto the x87's stack, the car left in eax; the status word
 * as the share's compare with the double leaves it, while racing. */
nfs::u32 spinTorque(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (half(app, car + kCarStage) == 0)
        g_status = fp::compare(speedShare(app, car), f64(app, kShareLeast));
    args.floatSet = true;
    args.floatResult = double(nfs::Float(spinTorqueOf(app, car)));
    ts.sp += 4;
    return args.eax;
}

/* physicsStep's frame on the guest's stack, where the game's code has it: six registers saved
 * (kStepPushed), then kStepFrame bytes.  Its callees are handed two parts of it: the car's axes
 * transposed (0x4e0110 writes them, nine floats, kStepAxes under the saved registers) and the four
 * wheels' records (kRecordSize each from kStepRecords under them, the front's two first; 0x4aca4f
 * works on each).  The rest of the code's frame is C++ locals. */
constexpr u32 kStepPushed = 0x18, kStepFrame = 0x154, kStepAxes = 0x8c, kStepRecords = 0x14c, kRecordSize = 0x30;
constexpr u32 kWheels = 4, kFrontWheels = 2;
// A wheel's record, by native_handling.cpp's names: its drive (the engine's share, then the
// brakes'), its grip (its load's), the grip's share, the steering's angle, its speed in the car's
// axes (the air's drag, the spin's torque across), its forces across and along, 1 on the front.
constexpr u32 kRecordDrive = 0, kRecordGrip = 4, kRecordGripShare = 8, kRecordAngle = 0xc, kRecordSpeed = 0x10;
constexpr u32 kRecordForceAcross = 0x1c, kRecordForceAlong = 0x24, kRecordFront = 0x28;
// The car's: its axes (world), its surface and its grip by it, how firmly it stands, the frames
// since it was put back and the slice it is to be put back on, the force in its axes, its speed
// and spin in them, the step's acceleration and spin's change (world; +0x2e4 cleared with them),
// the wheels' turns, the wheels' share, the wheels' grips' shares and their grips (four floats).
constexpr u32 kCarAxes = 0x190, kCarSurface = 0x18c, kCarGrip = 0x20c, kCarStanding = 0x134;
constexpr u32 kCarPutBackFrames = 0x524, kCarPutBackSlice = 0x184;
constexpr u32 kCarForce = 0x4e0, kCarOwnSpeed = kCarSpeedAcross, kCarOwnSpin = 0x4f8;
constexpr u32 kCarThrottleOff = 0x2e4, kCarAcceleration = 0x2e8, kCarSpinChange = 0x2f4;
constexpr u32 kCarWheelTurns = 0x59c, kCarWheelsShare = 0x1a0, kCarGripShares = 0x5c8, kCarWheelGrips = 0x5d8;
// The handling's: the front's share of the engine's force, the brakes, the front's share of them
// and of the grip, the load's shift by the force forward.
constexpr u32 kHandlingFrontDrive = 0x1d0, kHandlingBrakes = 0x1d8, kHandlingFrontBrakes = 0x1dc;
constexpr u32 kHandlingFrontGrip = 0x234, kHandlingShiftByForce = 0x254;
// The game's: the air's drag (two factors); how firmly a car stands to be stepped (a double); the
// brakes held to the speed forward by it; the front's share of them more with the handbrake; the
// loads kept, less the wheels' share; half an axle's two wheels; gravity forward's part of the
// load's shift; the step's scale of the acceleration and the spin's change.
constexpr u32 kDragScale = 0x53de7c, kDragFactor = 0x53de80, kStandingLeast = 0x53de84, kBrakeHold = 0x53de8c;
constexpr u32 kHandbrakeFrontBrakes = 0x53de90, kLoadsKept = 0x53de94, kAxleHalf = 0x53de98, kSlopeShift = 0x53de9c;
constexpr u32 kStepScale = 0x53dea0;
constexpr i32 kCountdownEnd = 0x200;            // the race's frame from which the speed is in the car's axes
constexpr u32 kGravityDownBits = 0xc11cf5c3;  // -9.81
constexpr u32 kSignBit = 0x80000000;

/* 0x4ad7a3: the car's full step.  Its axes transposed into the frame
 * (0x4e0110); the force in its axes and the four wheels' records cleared;
 * its surface's grip; its speed and spin in its own axes (the speed from
 * the countdown's end on, the spin while racing); its slip and ground speed;
 * the air's drag on each axis; the spin's torque; gravity in its axes
 * (kGravity); the wheels' speeds from the drag and torque (none while not
 * racing); the driver's inputs (0x4a9bc4), the steering's angle and the
 * idle's damping; the engine (0x4aaed1) shared out to the wheels by the
 * handling's split.  Where the race is not under way, the car was just put
 * back or is to be, or it stands too lightly ([+0x134]), its loads and step
 * cleared and done.  Else the brakes ([+0x5ec], by the handling's, held to
 * the speed, against it, the front's share less with the handbrake) on each
 * wheel; the wheels' loads (0x4aab41) and grips under gravity -- none on the
 * ground, done; the load's shift and each wheel's force (0x4aca4f); the
 * forces scaled by the wheels' share, summed into the car's force, the
 * drive's; its acceleration and speed; held still, its grip sideways and the
 * handbrake's drag on its speed in its axes, back to the world's; the yaw's
 * torque from the wheels' forces, the spin and its damping, back too; and
 * the coasting's damping.  The cores of this file it calls leave the x87's
 * status word as their thunks do (the code calls those): the compares of
 * sideSlip, groundSpeed, spinTorque, steerAngle, settleLoadShift, sideGrip
 * and handbrakeDrag made here.  Returns what the game's code leaves in eax --
 * the car where it stops early, the last compare's status word (fnstsw ax
 * over the loop's count, 3) where no wheel is on the ground, else what the
 * coast's damping leaves; nullopt when the game stopped. */
std::optional<u32> stepPhysics(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, kStepPushed, kStepFrame);
    const u32 axes = frame - kStepAxes;
    const auto record = [&frame](u32 wheel) { return frame - kStepRecords + wheel * kRecordSize; };
    if (!transposeTo(app, ts, car + kCarAxes, axes))
        return std::nullopt;
    const u32 handling = at(app, car + kCarHandling);
    for (u32 k = 0; k < 3; ++k)
        put(app, car + kCarForce + 4 * k, 0);
    for (u32 wheel = 0; wheel < kWheels; ++wheel)
        clearWheelRecord(app, record(wheel));
    putFloat(app, car + kCarGrip, float(f32(app, kSurfaceGrip + 4 * at(app, car + kCarSurface))));
    if (i32(at(app, kRaceFrame)) >= kCountdownEnd)
        turnInto(app, car + kCarVelocity, axes, car + kCarOwnSpeed);
    else
        for (u32 k = 0; k < 3; ++k)
            put(app, car + kCarOwnSpeed + 4 * k, 0);
    if (half(app, car + kCarStage) == 0)
        turnInto(app, car + kCarSpin, axes, car + kCarOwnSpin);
    compareWithZero(getFloat(app, car + kCarSpeedForward));  // sideSlip's
    updateSlip(app, car);
    if (!updateGroundSpeed(app, ts, car))
        return std::nullopt;
    compareWithZero(getFloat(app, car + kCarSpeedForward));  // groundSpeed's
    // The air's drag on each axis.
    Vector drag;
    for (u32 k = 0; k < 3; ++k)
        drag[k] = float(fp::mul(-fp::mul(f32(app, car + kCarOwnSpeed + 4 * k), f32(app, kDragScale)), f32(app, kDragFactor)));
    if (half(app, car + kCarStage) == 0)  // spinTorque's
        g_status = fp::compare(speedShare(app, car), f64(app, kShareLeast));
    const float torque = spinTorqueOf(app, car);
    putVector(app, kGravity, turned(app, Vector{0.0f, floatOf(kGravityDownBits), 0.0f}, axes));
    // The wheels' speeds: the drag, the spin's torque across with it on the front, against it on
    // the back.
    if (half(app, car + kCarStage) != 0)
    {
        for (u32 wheel = 0; wheel < kWheels; ++wheel)
            for (u32 k = 0; k < 3; ++k)
                put(app, record(wheel) + kRecordSpeed + 4 * k, 0);
    }
    else
    {
        const float front = float(fp::add(nfs::Float(drag[0]), nfs::Float(torque)));
        const float back = float(fp::sub(nfs::Float(drag[0]), nfs::Float(torque)));
        for (u32 wheel = 0; wheel < kWheels; ++wheel)
            putVector(app, record(wheel) + kRecordSpeed, Vector{wheel < kFrontWheels ? front : back, drag[1], drag[2]});
    }
    if (!guest::direct<0x4a9bc4, drivetrain::readInputs>(app, ts, car))
        return std::nullopt;
    const float angle = steeringAngle(app, car);
    if (half(app, car + kCarStage) == 0)  // steerAngle's
        g_status = fp::compare(nfs::Float(angle), f64(app, kAngleLeast));
    if (!dampIdle(app, ts, car))
        return std::nullopt;
    for (u32 wheel = 0; wheel < kWheels; ++wheel)
        put(app, record(wheel) + kRecordFront, wheel < kFrontWheels ? 1 : 0);
    const auto putDrives = [&](float front, float back) {
        for (u32 wheel = 0; wheel < kWheels; ++wheel)
            putFloat(app, record(wheel) + kRecordDrive, wheel < kFrontWheels ? front : back);
    };
    // The engine's force, front and back by the handling's split.
    const std::optional<float> force = guest::direct<0x4aaed1, drivetrain::forceAtWheels>(app, ts, car);
    if (!force)
        return std::nullopt;
    const float frontDrive = float(fp::mul(nfs::Float(*force), f32(app, handling + kHandlingFrontDrive)));
    const float backDrive = float(fp::sub(nfs::Float(*force), nfs::Float(frontDrive)));
    putDrives(frontDrive, backDrive);
    // Standing too lightly: under the double, or a NaN (the x87's carry).
    if (half(app, car + kCarStage) != 0 || at(app, car + kCarPutBackFrames) != 0 || at(app, car + kCarPutBackSlice) != 0
        || compareBelow(f32(app, car + kCarStanding), f64(app, kStandingLeast)))
    {
        put(app, car + kCarBackLoad, 0);
        putFloat(app, car + kCarFrontLoad, float(f32(app, car + kCarBackLoad)));
        for (const u32 offset : {kCarSpinChange, kCarSpinChange + 4, kCarSpinChange + 8, kCarAcceleration})
            put(app, car + offset, 0);
        putFloat(app, car + kCarAcceleration + 8, float(f32(app, car + kCarAcceleration)));
        return car;
    }
    // The brakes, held to what stops the car this step (the code works the product out twice),
    // against its way: the sign flipped going forward (or the speed a NaN: the x87's carry).
    put(app, car + kCarThrottleOff, 0);
    float brake = float(fp::mul(f32(app, car + kCarBrakeTravel), f32(app, handling + kHandlingBrakes)));
    const nfs::Float stopping = fp::mul(f32(app, car + kCarSpeedForward), f32(app, kBrakeHold));
    const float limit = float(compareAbove(nfs::Float(0.0), stopping) ? -stopping : stopping);
    if (compareAbove(nfs::Float(brake), nfs::Float(limit)))
        brake = limit;
    if (compareBelow(nfs::Float(0.0), f32(app, car + kCarSpeedForward)))
        brake = floatOf(floatBits(brake) ^ kSignBit);
    // The front's share of them; with the handbrake, the handling's moved by kHandbrakeFrontBrakes.
    const float frontBrake =
        byte(app, car + kCarHandbrake) != 0
            ? float(fp::mul(fp::add(f32(app, handling + kHandlingFrontBrakes), f32(app, kHandbrakeFrontBrakes)),
                            nfs::Float(brake)))
            : float(fp::mul(nfs::Float(brake), f32(app, handling + kHandlingFrontBrakes)));
    putDrives(float(fp::add(nfs::Float(frontDrive), nfs::Float(frontBrake))),
              float(fp::add(fp::sub(nfs::Float(brake), nfs::Float(frontBrake)), nfs::Float(backDrive))));
    if (!guest::direct<0x4aab41, drivetrain::setWheelGrip>(app, ts, car))
        return std::nullopt;
    // Each wheel's grip under gravity: its load's.
    float grip[kWheels];
    for (u32 wheel = 0; wheel < kWheels; ++wheel)
    {
        grip[wheel] = float(fp::mul(-f32(app, kGravityY), f32(app, handling + kHandlingMass)));
        grip[wheel] = float(fp::mul(f32(app, car + kCarWheelGrips + 4 * wheel), nfs::Float(grip[wheel])));
    }
    // None of the four on the ground (over 0, or a NaN: the x87's carry): done.
    bool grounded = false;
    for (u32 wheel = 0; wheel < kWheels && !grounded; ++wheel)
        grounded = compareBelow(nfs::Float(0.0), nfs::Float(grip[wheel]));
    if (!grounded)
        return u32(g_status);
    const float front0 = float(fp::mul(nfs::Float(grip[0]), f32(app, handling + kHandlingFrontGrip)));
    const float front1 = float(fp::mul(nfs::Float(grip[1]), f32(app, handling + kHandlingFrontGrip)));
    const float back3 = float(fp::mul(fp::sub(1.0, f32(app, handling + kHandlingFrontGrip)), nfs::Float(grip[3])));
    const float back2 = float(fp::mul(fp::sub(1.0, f32(app, handling + kHandlingFrontGrip)), nfs::Float(grip[2])));
    compareWithZero(getFloat(app, car + kCarLoadShift));  // settleLoadShift's
    settleLoad(app, car);
    // The wheels' grips with the load's shift, front less it, back with it, by their shares.
    const auto putGrip = [&](u32 wheel, nfs::Float shifted) {
        const float kept = float(shifted);
        putFloat(app, record(wheel) + kRecordGrip,
                 float(fp::mul(nfs::Float(kept), f32(app, car + kCarGripShares + 4 * wheel))));
    };
    putGrip(0, fp::sub(nfs::Float(front0), f32(app, car + kCarLoadShift)));
    putGrip(1, fp::sub(nfs::Float(front1), f32(app, car + kCarLoadShift)));
    putGrip(3, fp::add(fp::sub(nfs::Float(grip[3]), nfs::Float(back3)), f32(app, car + kCarLoadShift)));
    putGrip(2, fp::add(fp::sub(nfs::Float(grip[2]), nfs::Float(back2)), f32(app, car + kCarLoadShift)));
    // The steering's angle on the front wheels.
    put(app, record(2) + kRecordAngle, 0);
    put(app, record(3) + kRecordAngle, 0);
    put(app, record(1) + kRecordAngle, floatBits(angle));
    put(app, record(0) + kRecordAngle, floatBits(angle));
    put(app, car + kCarWheelTurns, 0);
    // Each wheel's force, its grip's share in its record.
    for (u32 wheel = 0; wheel < kWheels; ++wheel)
    {
        putFloat(app, record(wheel) + kRecordGripShare, float(f32(app, car + kCarGripShares + 4 * wheel)));
        if (!guest::direct<0x4aca4f, handling::axleForcesSet>(app, ts, car, record(wheel)))
            return std::nullopt;
    }
    // Their forces by the wheels' share (the code stores them back in the records, which nothing
    // reads after).
    float across[kWheels], along[kWheels];
    for (u32 wheel = 0; wheel < kWheels; ++wheel)
    {
        across[wheel] = float(fp::mul(f32(app, car + kCarWheelsShare), f32(app, record(wheel) + kRecordForceAcross)));
        along[wheel] = float(fp::mul(f32(app, car + kCarWheelsShare), f32(app, record(wheel) + kRecordForceAlong)));
    }
    const float share = getFloat(app, car + kCarWheelsShare);
    for (const u32 offset : {kCarFrontLoad, kCarBackLoad})
        putFloat(app, car + offset, float(fp::mul(fp::sub(f32(app, kLoadsKept), nfs::Float(share)), f32(app, car + offset))));
    // The car's force: each axle's half, the front's and the back's (the wheels 3 and 2).
    const auto axles = [&](const float* forces) {
        return fp::add(fp::mul(fp::add(nfs::Float(forces[0]), nfs::Float(forces[1])), f32(app, kAxleHalf)),
                       fp::mul(fp::add(nfs::Float(forces[3]), nfs::Float(forces[2])), f32(app, kAxleHalf)));
    };
    putFloat(app, car + kCarForceForward, float(axles(along)));
    putFloat(app, car + kCarForce, float(axles(across)));
    applyDriveForce(app, car);
    const float slope = float(fp::mul(-f32(app, kGravityZ), f32(app, kSlopeShift)));
    const nfs::Float byForce = fp::mul(f32(app, car + kCarForceForward), f32(app, handling + kHandlingShiftByForce));
    putFloat(app, car + kCarLoadShift, float(fp::add(byForce, nfs::Float(slope))));
    // A change (the acceleration, the spin's) by the step's scale onto the car's vector.
    const auto stepOnto = [&](u32 change, u32 onto) {
        for (u32 k = 0; k < 3; ++k)
            putFloat(app, onto + 4 * k,
                     float(fp::add(fp::mul(f32(app, change + 4 * k), f32(app, kStepScale)), f32(app, onto + 4 * k))));
    };
    // The acceleration in the world's axes, onto the speed.
    turnInto(app, car + kCarForce, car + kCarAxes, car + kCarAcceleration);
    stepOnto(car + kCarAcceleration, car + kCarVelocity);
    if (!holdCarStill(app, ts, car))
        return std::nullopt;
    turnInto(app, car + kCarVelocity, axes, car + kCarOwnSpeed);
    compareWithZero(getFloat(app, car + kCarSpeedAcross));  // sideGrip's
    gripSideways(app, car);
    if (byte(app, car + kCarHandbrake) != 0)  // handbrakeDrag's
        compareWithZero(getFloat(app, car + kCarSpeedForward));
    dragHandbrake(app, car);
    turnInto(app, car + kCarOwnSpeed, car + kCarAxes, car + kCarVelocity);
    // The yaw's torque: the front wheels' forces across less the back's.
    const float yaw = float(fp::sub(fp::mul(fp::add(nfs::Float(across[0]), nfs::Float(across[1])), f32(app, kAxleHalf)),
                                    fp::mul(fp::add(nfs::Float(across[3]), nfs::Float(across[2])), f32(app, kAxleHalf))));
    const nfs::Float spinScale = fp::mul(f32(app, car + kCarSpinScale), f32(app, car + kCarSpinScaleToo));
    const float yawTorque = float(fp::mul(fp::mul(spinScale, f32(app, kStepScale)), nfs::Float(yaw)));
    putVector(app, car + kCarSpinChange, turned(app, Vector{0.0f, yawTorque, 0.0f}, car + kCarAxes));
    stepOnto(car + kCarSpinChange, car + kCarSpin);
    turnInto(app, car + kCarSpin, axes, car + kCarOwnSpin);
    if (!dampYaw(app, ts, car))
        return std::nullopt;
    turnInto(app, car + kCarOwnSpin, car + kCarAxes, car + kCarSpin);
    return dampCoast(app, ts, car);
}

/* 0x4ad7a3: stepPhysics for car eax. */
nfs::u32 physicsStep(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = stepPhysics(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

}  // namespace physics

}  // namespace nfs3hp
