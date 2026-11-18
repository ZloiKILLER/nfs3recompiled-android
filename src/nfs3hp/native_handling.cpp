#include "game_settings.h"
#include "native_frontend_rendc.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <type_traits>

/* A car's handling in the race, as nfs3.exe's physics (0x4a78d6..0x4af3a1)
 * has it: its handling made for the race from the player's setup
 * (applyTuning, from the cars' loading), the automatic gearbox's choice
 * (automaticGear), the engine's revs matched to the car's speed and the
 * drive's state cleared (matchRevsToSpeed, resetDriveState), a car at rest
 * settled (settleAtRest), and an axle's tyres' forces (axleForces and what it
 * calls: tyreGrip, tyreSlide, the setup's and the steering's shares).
 *
 * The car's record, as these functions use it: +0x15c and +0x10c how fast it goes and how it is on the
 * road, +0x160 the road's state (0 dry; 1 and 2 as the code tells them),
 * +0x1f0 its player, +0x200 its flags (1 a player's), +0x220 the player's
 * setup (+0x40 seven floats as kDefaultSetup has them, +0x58 one of them),
 * +0x2c4 the race's stage for it, +0x4ec and +0x4f4 its speed along its
 * axes, +0x506..+0x51c the driver's inputs (+0x50b the gear: 0 reverse, 1
 * neutral, 2 on the gears; +0x50d the handbrake; +0x514 how long the wheel
 * has been turned, to 0x180; +0x51c the steering), +0x528 its handling,
 * +0x52c..+0x56c by the gear, +0x590 the engine's revs, +0x594 the clutch,
 * +0x5a4 the wheels' turn, +0x5b0 the throttle, +0x5c0 and +0x5c4 the front
 * and back axles' slip, +0x5f0 the brake.  The handling's record: +8 the
 * gears' count, +0x94 the gears' ratios, +0x114 the torque's curve (0x29
 * floats), +0x1c0 and +0x1c4 the revs' idle and limit, the rest of it scales
 * of the handling.  An axle's record (edx): +0 its drive, +4 its grip, +0xc
 * its wheels' angle, +0x10 and +0x18 its speed across and along, +0x1c and
 * +0x24 its forces across and along, +0x28 the front's, +0x2c its slip.
 *
 * Each is a typed core (no registers, the guest's stack only for what a callee is handed) and an
 * thunk under the game's name, for the calls through a pointer and for callers still on registers.  The floats
 * are the game's to the bit: fp:: in the code's order and argument order, a float(...) where the
 * code stores one.  The vectors' functions (native_vertices.h's) are called as C++ (guest::direct)
 * but settleCar's last; every other function of the game's by address -- the rest of the
 * physics' too; the game's constants (floats and doubles in its data, 0x53dc40 to 0x53de40) are
 * read where the code reads them. */
namespace nfs3hp
{
namespace handling
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;

// The race's.
constexpr reg32 kFrame = 0x7d3684;          // the race's frame
// The setup the player's is held against: seven floats, the setup's middle.
constexpr reg32 kDefaultSetup = 0x55d024;
// The game's that these call by address: Watcom's chop and exp, the sine,
// atan2 and the square root of a stack float, a 2D turn by an angle; n
// vectors added (settleCar's last call, see there).  A vector's dot product
// and n vectors scaled, less others, added are vertices' (guest::direct).
constexpr reg32 kChop = 0x4dfd56;

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
x86::Float f32(App* app, reg32 a) { return x86::Float(app->getMemory<float>(a)); }
x86::Float f64(App* app, reg32 a) { return x86::Float(app->getMemory<double>(a)); }

auto floatAt(App* app, reg32 a) { return app->getMemory<float>(a); }

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* A float's bits as the int the code compares them as. */
i32 signedBits(float value) { return i32(bitsOf(value)); }

/* The float's size, as the code's "0 > x ? -x : x": a compare of the x87 that leaves neither
 * below nor equal is a plain > on every value, a NaN too (unordered sets both flags), and what
 * is not negated stays the same bits. */
float size(float value) { return 0.0 > x86::Float(value) ? float(-x86::Float(value)) : value; }

/* Over a link or in the series the setup does not count. */
bool linkOrSeries(App* app)
{
    return i32(gameSettings(app).playMode) > 1 || gameSettings(app).raceType == 1 || gameSettings(app).raceType == 2;
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated). */
template <reg32 Address, auto Core, typename... Args>
auto feCore(App* app, CPU& cpu, Args... args)
{
    const auto run = [&] {
        return guest::direct<Address, Core>(app, cpu, args...);
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
        return !cpu.terminate;
    }
    else
    {
        try
        {
            const Result result = run();
            if constexpr (std::is_same_v<Result, x86::reg32>)
                (void)result;  // a menu core's result: the eax it leaves is no longer set (no caller reads it)
            else
                spoil();
            return cpu.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* Watcom's exp, of a double on the stack, onto the x87. */
std::optional<x86::Float> watcomExp(App* app, CPU& cpu, x86::Float power)
{
    const reg32 sp = cpu.esp;
    cpu.esp -= 8;  // (the double the code pushes: the core is handed it)
    const double result = guest::direct<0x4f8116, watcom::clib::exponent>(app, cpu, double(power));
    cpu.esp = sp;
    if (cpu.terminate)
        return std::nullopt;
    return x86::Float(result);
}

/* Watcom's chop of the value, then fistp; never nullopt (the optional is the callers' stop
 * signal). */
std::optional<i32> chop(App* app, CPU& cpu, x86::Float value)
{
    const double whole = guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value));
    return fp::toInteger<i32>(x86::Float(whole));
}

/* The stack pointer on entry to a thunk: what its code leaves of the registers is the Args'. */
struct Saved
{
    reg32 esp;
    explicit Saved(const CPU& entry) : esp(entry.esp) {}
    void restore(CPU& back) const { back.esp = esp; }
};

// The car's record.
constexpr reg32 kCarSpeed = 0xa4, kCarSpin = 0xe4, kCarRoadState = 0x10c, kCarUpAxis = 0x130, kCarRoadSpeed = 0x15c, kCarRoad = 0x160;
constexpr reg32 kCarRaceStarted = 0x164, kCarPlayer = 0x1f0, kCarFlags = 0x200, kCarField20c = 0x20c, kCarSetup = 0x220, kCarRaceStage = 0x2c4;
constexpr reg32 kCarSpeedAlong = 0x4f4, kCarGear = 0x50b, kCarHandbrake = 0x50d, kCarWheelTurned = 0x514, kCarSteering = 0x51c;
constexpr reg32 kCarHandling = 0x528, kCarGearRevs = 0x52c, kCarGearInverse = 0x54c, kCarGearShare = 0x56c, kCarDrag = 0x58c;
constexpr reg32 kCarRevs = 0x590, kCarClutch = 0x594, kCarWheelTurns = 0x59c, kCarWheelsTurn = 0x5a4, kCarThrottle = 0x5b0;
constexpr reg32 kCarFrontSlip = 0x5c0, kCarBackSlip = 0x5c4, kCarBrake = 0x5f0;
constexpr std::uint8_t kCarFlagPlayer = 1, kCarFlagSetupMiddle = 2;
constexpr std::uint8_t kGearReverse = 0, kGearNeutral = 1;
constexpr reg32 kRaceStageFinished = 3;
constexpr i32 kWheelFullTime = 0x180, kFullLock = 0x7f;
// The player's setup: the sliders, as kDefaultSetup holds their middles.
constexpr reg32 kSetupSlider = 0x40, kSetupSliders = 7, kSetupSlipSlider = 0x58;
constexpr reg32 kSliderSecond = 4, kSliderThird = 8, kSliderFourth = 0xc, kSliderFifth = 0x10, kSliderSixth = 0x14;
// The car's handling record.
constexpr reg32 kHandlingShareBase = 4, kHandlingGears = 8, kHandlingRatios = 0x94, kHandlingGearShare = 0xd4, kHandlingTorque = 0x114;
constexpr reg32 kHandlingRevsIdle = 0x1c0, kHandlingRevsLimit = 0x1c4, kHandlingTopSpeed = 0x1c8, kHandlingDriveCut = 0x1d4;
constexpr reg32 kHandlingSideLimit = 0x23c, kHandlingDriveShare = 0x248;
constexpr i32 kTorquePoints = 0x29;
// An axle's record.
constexpr reg32 kAxleDrive = 0, kAxleGrip = 4, kAxleGripShare = 8, kAxleAngle = 0xc, kAxleAcross = 0x10, kAxleAlong = 0x18;
constexpr reg32 kAxleForceAcross = 0x1c, kAxleForceUp = 0x20, kAxleForceAlong = 0x24, kAxleFront = 0x28, kAxleSlip = 0x2c;
// The game's constants these read.
constexpr reg32 kTorqueSliderShift = 0x53dc68, kTorqueSliderScale = 0x53dc6c, kFirstSliderTopShare = 0x53dc70, kGearShareScale = 0x53dc7c;
constexpr reg32 kDownOneThrottle = 0x53dca4, kUprightLowest = 0x53dcc4;  // doubles
constexpr reg32 kStillBelow = 0x53dccc;
constexpr reg32 kSlipLimitScale = 0x53ddb4, kSlipLimitTop = 0x53ddb8;
constexpr reg32 kGripOtherShare = 0x53ddbc, kGripPullScale = 0x53ddc0, kGripPullTop = 0x53ddc4, kNeutralForceShare = 0x53ddc8;
constexpr reg32 kSlideOtherShare = 0x53ddcc, kAssistSlipShare = 0x53ddd0, kSlideRoad1Share = 0x53ddd4, kSlideRoad2Share = 0x53ddd8;
constexpr reg32 kSlidePullScale = 0x53dddc, kSlidePullTop = 0x53dde0, kHighRevsShare = 0x53dde4, kBrakeAlongShare = 0x53dde8;
constexpr reg32 kClutchSlipShare = 0x53ddec;
constexpr reg32 kLogisticShift = 0x53ddf4, kLogisticSteep = 0x53ddfc, kLogisticSize = 0x53de04;  // doubles
constexpr reg32 kSteeringShareScale = 0x53de0c;
constexpr reg32 kReverseAngleScale = 0x53de10, kReverseForceScale = 0x53de14, kGearAngleScale = 0x53de18, kGearForceScale = 0x53de1c;
constexpr reg32 kSideLimitScale = 0x53de20, kFullLockShare = 0x53de24, kLightBrakeShare = 0x53de28;
constexpr reg32 kFrontSideShare = 0x53de2c, kBackSideShare = 0x53de30;
constexpr std::uint8_t kRaceFlagTorque = 1, kRaceFlagFirstSet = 4, kRaceFlagSimulationGrip = 0x20;
// Floats' bits the code compares as ints, so that a NaN or a negative lands where it does there.
constexpr i32 kBits0_015 = 0x3c75c28f, kBits0_02 = 0x3ca3d70a, kBits0_05 = 0x3d4ccccd, kBits0_1 = 0x3dcccccd, kBits0_15 = 0x3e19999a;
constexpr i32 kBits0_2 = 0x3e4ccccd, kBits0_25 = 0x3e800000, kBits0_5 = 0x3f000000, kBits0_85 = 0x3f59999a, kBits0_9 = 0x3f666666;
constexpr i32 kBits0_95 = 0x3f733333, kBits1 = 0x3f800000, kBits5 = 0x40a00000, kBitsSlow = 0x41566666;
constexpr reg32 kBitsNotSign = 0x7fffffff;
// settleCar's vector, where the code's frame has it: under the entry's esp and over the new esp.
constexpr reg32 kPartBelowEntry = 0x54, kPartOverEsp = 0xc;

/* The setup's slider the slip goes by. */
reg32 slipSlider(App* app, reg32 car) { return at(app, car + kCarSetup) + kSetupSlipSlider; }

/* How quickly the slip follows, by the slider: a constant less it by another, 0.7 over a link or
 * in the series, 0.5 at least. */
float slipPull(App* app, reg32 car, reg32 scale, reg32 top)
{
    const float pull = linkOrSeries(app) ? 0.7f : float(fp::sub(f32(app, top), fp::mul(f32(app, slipSlider(app, car)), f32(app, scale))));
    return signedBits(pull) > kBits0_5 ? pull : 0.5f;
}

}  // namespace

x86::reg32 logisticCurve(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 steeringGrip(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setupSlipLimit(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setupGripLoss(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tyreGrip(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tyreSlide(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 axleForces(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 applyTuning(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 automaticGear(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 matchRevsToSpeed(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 resetDriveState(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 settleAtRest(App* app, x86::CPU& cpu, x86::Args& args);

/* 0x4ac994: a logistic curve, 1 - c / (1 + exp((x + a) * b)), with Watcom's exp by address. */
std::optional<float> logistic(App* app, CPU& cpu, float x)
{
    const auto power = watcomExp(app, cpu, fp::mul(fp::add(x, f64(app, kLogisticShift)), f64(app, kLogisticSteep)));
    if (!power)
        return std::nullopt;
    return float(fp::sub(1.0, fp::div(f64(app, kLogisticSize), fp::add(*power, 1.0))));
}

/* The thunk (stdcall: x, ret 4): onto the x87's stack, every register kept. */
x86::reg32 logisticCurve(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const auto value = logistic(app, cpu, floatAt(app, cpu.esp + 4));
    if (!value)
        return args.eax;
    saved.restore(cpu);
    args.floatSet = true;
    args.floatResult = double(x86::Float(*value));
    cpu.esp += 8;
    return args.eax;
}

/* 0x4ac9dd: the steering's share of the front's grip: 1 while the wheel has not been turned,
 * else the curve of how long it has (a whole lock's time being 1) by a constant. */
std::optional<float> steeringShare(App* app, CPU& cpu, reg32 car)
{
    const i32 turned = i32(at(app, car + kCarWheelTurned));
    if (turned <= 0)
        return 1.0f;
    const float time = float(fp::div(x86::Float(turned), x86::Float(kWheelFullTime)));
    const auto curve = logistic(app, cpu, time);
    if (!curve)
        return std::nullopt;
    return float(fp::mul(*curve, f32(app, kSteeringShareScale)));
}

/* The thunk: onto the x87's stack, its bits in eax, the other registers kept. */
x86::reg32 steeringGrip(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const auto share = steeringShare(app, cpu, args.eax);
    if (!share)
        return args.eax;
    saved.restore(cpu);
    args.eax = bitsOf(*share);
    args.floatSet = true;
    args.floatResult = double(x86::Float(*share));
    cpu.esp += 4;
    return args.eax;
}

/* 0x4ac027: how far the slip may go, by the setup's slider: 12 - 4 * it, chopped.  Over a link
 * or in the series the slider is put to 1 first. */
std::optional<i32> slipLimit(App* app, CPU& cpu, reg32 slider)
{
    if (linkOrSeries(app))
        floatAt(app, slider) = 1.0f;
    const float limit = float(fp::sub(f32(app, kSlipLimitTop), fp::mul(f32(app, slider), f32(app, kSlipLimitScale))));
    return chop(app, cpu, limit);
}

/* The thunk: slider at eax, the limit to eax. */
x86::reg32 setupSlipLimit(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const auto limit = slipLimit(app, cpu, args.eax);
    if (!limit)
        return args.eax;
    saved.restore(cpu);
    args.eax = reg32(*limit);
    cpu.esp += 4;
    return args.eax;
}

/* One of the grip's curves: 1 / max(top - (slider + shift) * scale, floor). The addresses are
 * the game's constants, `lowest` the code's own value of its floor. */
struct GripCurve
{
    reg32 shift, scale, top, floor;
    float lowest;
};

float gripCurve(App* app, reg32 slider, const GripCurve& curve)
{
    const x86::Float divisor = fp::sub(f32(app, curve.top), fp::mul(fp::add(f32(app, slider), f32(app, curve.shift)), f32(app, curve.scale)));
    const float held = divisor > f32(app, curve.floor) ? float(divisor) : curve.lowest;
    return float(fp::div(1.0, held));
}

/* 0x4abd9c: the grip lost to the setup's slider, by the car's road and whether the simulation's
 * handling is on (raceFlags 0x20): on two of the roads the simulation's is cut again.  Over a
 * link or in the series the slider is put to 1 first. */
float gripLoss(App* app, reg32 car, reg32 slider)
{
    if (linkOrSeries(app))
        floatAt(app, slider) = 1.0f;
    const reg32 road = at(app, car + kCarRoad);
    if ((std::uint8_t(gameSettings(app).raceFlags) & kRaceFlagSimulationGrip) != 0)
    {
        if (road == 2)
            return float(fp::mul(gripCurve(app, slider, { 0x53dd4c, 0x53dd50, 0x53dd54, 0x53dd58, 4.0f }), f32(app, 0x53dd5c)));
        if (road == 1)
            return float(fp::mul(gripCurve(app, slider, { 0x53dd60, 0x53dd64, 0x53dd68, 0x53dd6c, 6.0f }), f32(app, 0x53dd70)));
        return gripCurve(app, slider, { 0x53dd74, 0x53dd78, 0x53dd7c, 0x53dd80, 16.0f });
    }
    if (road == 2)
        return gripCurve(app, slider, { 0x53dd84, 0x53dd88, 0x53dd8c, 0x53dd90, 4.0f });
    if (road == 1)
        return gripCurve(app, slider, { 0x53dd94, 0x53dd98, 0x53dd9c, 0x53dda0, 6.0f });
    return gripCurve(app, slider, { 0x53dda4, 0x53dda8, 0x53ddac, 0x53ddb0, 16.0f });
}

/* The thunk: car eax, slider edx; onto the x87's stack, its bits in eax. */
x86::reg32 setupGripLoss(App* app, x86::CPU& cpu, x86::Args& args)
{
    const float loss = gripLoss(app, args.eax, args.edx);
    args.eax = bitsOf(loss);
    args.floatSet = true;
    args.floatResult = double(x86::Float(loss));
    cpu.esp += 4;
    return args.eax;
}

/* The axle's slip held to slipLimit.  False when the game stopped. */
bool holdSlip(App* app, CPU& cpu, reg32 car, reg32 axle)
{
    const auto limit = slipLimit(app, cpu, slipSlider(app, car));
    if (!limit)
        return false;
    const float most = float(x86::Float(*limit));
    const float slip = floatAt(app, axle + kAxleSlip);
    floatAt(app, axle + kAxleSlip) = slip > most ? most : slip;
    return true;
}

/* 0x4ac094: the axle's tyres gripping: the wheels' turns counted (1 for the front, 2 the back),
 * the slip pulled towards how far the larger of the speeds across and along (with a quarter of
 * the other) is past the grip -- or 0 -- and held to slipLimit; the force across the speed across
 * by the slip's share (0.75 past the grip lost) by steeringShare, the force along the drive by
 * it, both a 20th in neutral; the slip kept back in the car.  False when the game stopped. */
bool gripAxle(App* app, CPU& cpu, reg32 car, reg32 axle)
{
    const bool front = at(app, axle + kAxleFront) != 0;
    const reg32 carSlip = car + (front ? kCarFrontSlip : kCarBackSlip);
    put(app, car + kCarWheelTurns, at(app, car + kCarWheelTurns) + (front ? 1 : 2));
    floatAt(app, axle + kAxleSlip) = float(f32(app, carSlip));
    const float grip = float(fp::mul(f32(app, axle + kAxleGrip), f32(app, axle + kAxleGripShare)));
    const float across = size(floatAt(app, axle + kAxleAcross));
    const float along = size(floatAt(app, axle + kAxleAlong));
    const x86::Float otherShare = f32(app, kGripOtherShare);
    const float speed = across > along ? float(fp::add(fp::mul(along, otherShare), across))
                                       : float(fp::add(fp::mul(across, otherShare), along));
    if (speed > grip)
    {
        const float pull = slipPull(app, car, kGripPullScale, kGripPullTop);
        floatAt(app, axle + kAxleSlip) = float(fp::add(fp::mul(f32(app, axle + kAxleSlip), pull),
                                                       fp::mul(fp::sub(speed, grip), pull)));
    }
    else
    {
        put(app, axle + kAxleSlip, 0);
    }
    if (!holdSlip(app, cpu, car, axle))
        return false;
    const float loss = gripLoss(app, car, slipSlider(app, car));
    const float kept = float(fp::sub(grip, fp::mul(grip, loss)));
    float share = 0.75f;
    if (size(speed) > size(kept))
    {
        const x86::Float ratio = fp::div(kept, speed);
        share = 0.0 > ratio ? float(-ratio) : float(ratio);
    }
    const float forceAcross = float(fp::mul(f32(app, axle + kAxleAcross), share));
    const auto steering = steeringShare(app, cpu, car);
    if (!steering)
        return false;
    floatAt(app, axle + kAxleForceAcross) = float(fp::mul(*steering, forceAcross));
    put(app, axle + kAxleForceUp, 0);
    floatAt(app, axle + kAxleForceAlong) = float(fp::mul(f32(app, axle + kAxleDrive), share));
    if (byte(app, car + kCarGear) == kGearNeutral)
    {
        floatAt(app, axle + kAxleForceAcross) = float(fp::mul(f32(app, axle + kAxleForceAcross), f32(app, kNeutralForceShare)));
        floatAt(app, axle + kAxleForceAlong) = float(fp::mul(f32(app, axle + kAxleForceAlong), f32(app, kNeutralForceShare)));
    }
    floatAt(app, carSlip) = float(f32(app, axle + kAxleSlip));
    return true;
}

/* The thunk: car eax, axle edx; eax and edx as the game's code leaves them. */
x86::reg32 tyreGrip(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const reg32 car = args.eax;
    if (!gripAxle(app, cpu, car, args.edx))
        return args.eax;
    saved.restore(cpu);
    args.eax = car;
    args.edx = slipSlider(app, car);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4ac3af: the axle's tyres past their grip: the forces' larger (with a quarter of the other)
 * against the grip less what the setup loses of it.  Past it, the wheels spin or lock: with the
 * assist on the back axle and the throttle past 0.15, the slip every fourth frame and the forces
 * cut to the grip; else the slip from how far past, by the road's share, pulled in by the setup's
 * slider, and the forces cut -- along only with the assist, else by half again -- when on the
 * back, in a gear, braking hard with the revs high.  The back axle with the clutch in slips at
 * the forces' size, by 4 in first or second.  The slip held to slipLimit. */
bool slideAxle(App* app, CPU& cpu, reg32 car, reg32 axle)
{
    const bool front = at(app, axle + kAxleFront) != 0;
    const float across = size(floatAt(app, axle + kAxleForceAcross));
    const float along = size(floatAt(app, axle + kAxleForceAlong));
    const x86::Float otherShare = f32(app, kSlideOtherShare);
    const float force = across > along ? float(fp::add(across, float(fp::mul(along, otherShare))))
                                       : float(fp::add(along, float(fp::mul(across, otherShare))));
    floatAt(app, axle + kAxleSlip) = float(f32(app, car + (front ? kCarFrontSlip : kCarBackSlip)));
    const float grip = floatAt(app, axle + kAxleGrip);
    const float kept = float(fp::sub(grip, fp::mul(gripLoss(app, car, slipSlider(app, car)), grip)));
    const float loss = gripLoss(app, car, slipSlider(app, car));
    put(app, axle + kAxleSlip, 0);
    const reg32 forceAcross = axle + kAxleForceAcross, forceAlong = axle + kAxleForceAlong, slip = axle + kAxleSlip;
    if (force > kept)
    {
        const bool assist = gameSettings(app).raceOptionSimulation != 0 && !front;
        if (assist && i32(at(app, car + kCarThrottle)) > kBits0_15)
        {
            const float cut = (bitsOf(force) & kBitsNotSign) != 0 ? float(fp::div(kept, force)) : 1.0f;
            if (i32(at(app, kFrame)) % 4 == 0)
                put(app, slip, 0);
            else
                floatAt(app, slip) = float(fp::mul(kept, f32(app, kAssistSlipShare)));
            if (byte(app, car + kCarGear) > 2)
                floatAt(app, forceAcross) = float(fp::mul(f32(app, forceAcross), cut));
            floatAt(app, forceAlong) = float(fp::mul(f32(app, forceAlong), cut));
        }
        else
        {
            if (signedBits(size(floatAt(app, car + kCarSpeedAlong))) > kBits5 || at(app, car + kCarClutch) != 0)
                floatAt(app, slip) = float(fp::sub(force, kept));
            const x86::Float past = fp::mul(fp::sub(force, kept), loss);
            const x86::Float held = fp::mul(kept, loss);
            const float least = held > past ? float(past) : float(held);
            const reg32 road = at(app, car + kCarRoad);
            float lost = least;
            if (road == 1)
                lost = float(fp::mul(least, f32(app, kSlideRoad1Share)));
            else if (road == 2)
                lost = float(fp::mul(least, f32(app, kSlideRoad2Share)));
            const float room = float(fp::sub(kept, lost));
            const float forceSize = size(force);
            const float cut = forceSize > room && (bitsOf(force) & kBitsNotSign) != 0 ? float(fp::div(room, forceSize)) : 1.0f;
            const float pull = slipPull(app, car, kSlidePullScale, kSlidePullTop);
            floatAt(app, slip) = float(fp::add(fp::mul(f32(app, slip), pull), fp::mul(fp::sub(force, kept), pull)));
            if (!front && byte(app, car + kCarGear) == 2 && i32(at(app, car + kCarField20c)) >= kBits0_95
                && i32(at(app, car + kCarBrake)) >= kBits0_85)
            {
                const x86::Float high = fp::mul(x86::Float(i32(at(app, at(app, car + kCarHandling) + kHandlingRevsLimit))), f32(app, kHighRevsShare));
                // fcompp's "not below" is >= on every value: unordered sets below.
                if (x86::Float(i32(at(app, car + kCarRevs))) >= high)
                {
                    floatAt(app, forceAcross) = float(fp::mul(f32(app, forceAcross), cut));
                    if (gameSettings(app).raceOptionSimulation != 0)
                        floatAt(app, forceAlong) = float(fp::mul(f32(app, forceAlong), cut));
                    else
                        floatAt(app, forceAlong) = float(fp::mul(fp::mul(cut, f32(app, kBrakeAlongShare)), f32(app, forceAlong)));
                }
            }
        }
    }
    if (!front && at(app, car + kCarClutch) == 1)
    {
        floatAt(app, slip) = force;
        const std::uint8_t gear = byte(app, car + kCarGear);
        if (gear >= 2 && gear <= 3)
            floatAt(app, slip) = float(fp::mul(f32(app, slip), f32(app, kClutchSlipShare)));
    }
    return holdSlip(app, cpu, car, axle);
}

/* The thunk: car eax, axle edx; the slip's bits left in eax, the axle in edx. */
x86::reg32 tyreSlide(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const reg32 axle = args.edx;
    if (!slideAxle(app, cpu, args.eax, axle))
        return args.eax;
    saved.restore(cpu);
    args.eax = at(app, axle + kAxleSlip);
    cpu.esp += 4;
    return args.eax;
}

/* The 2D turn of the words at x and y by the axle's wheels' angle (rendc's, 0x496bf0). */
bool turnByWheels(App* app, CPU& cpu, reg32 axle, reg32 x, reg32 y)
{
    return feCore<0x496bf0, frontend::rendc::rotatePlane>(app, cpu, at(app, x), at(app, y), at(app, axle + kAxleAngle),
                                                         x, y)
        .has_value();
}

/* The game's sine of a float (simd's, 0x4ea930). */
std::optional<x86::Float> sine(App* app, CPU& cpu, float angle)
{
    return feCore<0x4ea930, frontend::simd::sineValue>(app, cpu, bitsOf(angle));
}

/* The force across a sliding axle's tyres: the sine, taken twice as the code does, of the slip's
 * angle by a scale, by the grip and a scale, made positive. */
std::optional<float> sideForce(App* app, CPU& cpu, float angle, float grip, reg32 angleScale, reg32 forceScale)
{
    const float scaled = float(fp::mul(angle, f32(app, angleScale)));
    const auto forOnce = [&]() -> std::optional<x86::Float> {
        const auto value = sine(app, cpu, scaled);
        if (!value)
            return std::nullopt;
        return fp::mul(fp::mul(*value, grip), f32(app, forceScale));
    };
    const auto first = forOnce();
    if (!first)
        return std::nullopt;
    const auto again = forOnce();
    if (!again)
        return std::nullopt;
    return 0.0 > *first ? float(-*again) : float(*again);
}

/* 0x4aca4f: the axle's tyres' forces.  Its grip less what the setup loses; its speeds turned into
 * the wheels' way when they are turned; the drive held back where the wheels would turn the car
 * the other way braking (and the mark of it); the drive by the handling's +0x248; the wheel's time
 * turned counted to a whole lock's while the handbrake is on.  With the handbrake on the back axle,
 * or on the front with the mark and the drive past the grip, gripping (gripAxle).  Else sliding:
 * the force across from the slip angle through a sine by the grip -- in reverse and in a gear each
 * its own -- held to the handling's +0x23c by 2, a part at full lock slow or when hardly braking,
 * held to the speed across, 0 on the front stopped with the handbrake, by a share for the axle;
 * then past the grip (slideAxle), the forces turned back, the slip kept in the car. */
AxleEnd axleTyres(App* app, CPU& cpu, reg32 car, reg32 axle)
{
    const reg32 handling = at(app, car + kCarHandling);
    put(app, axle + kAxleSlip, 0);
    const float grip = floatAt(app, axle + kAxleGrip);
    const float kept = float(fp::sub(grip, fp::mul(gripLoss(app, car, slipSlider(app, car)), grip)));
    const bool wheelsTurned = (at(app, axle + kAxleAngle) & kBitsNotSign) != 0;
    if (wheelsTurned && !turnByWheels(app, cpu, axle, axle + kAxleAcross, axle + kAxleAlong))
        return AxleEnd::Stopped;

    bool heldBack = false;
    const float drive = floatAt(app, axle + kAxleDrive);
    const float along = floatAt(app, axle + kAxleAlong);
    const bool lightBrake = i32(at(app, car + kCarBrake)) <= kBits0_25;
    const std::uint8_t gear = byte(app, car + kCarGear);
    // Each "below" of the code is !(>=) on every value: unordered sets it.
    if (0.0 > x86::Float(drive) && 0.0 > x86::Float(along))
    {
        if (lightBrake || gear != kGearReverse)
        {
            floatAt(app, axle + kAxleDrive) = drive > along ? drive : along;
            heldBack = true;
        }
    }
    else if (!(0.0 >= x86::Float(drive)) && !(0.0 >= x86::Float(along)))
    {
        if (lightBrake || gear <= kGearNeutral)
        {
            floatAt(app, axle + kAxleDrive) = drive > along ? along : drive;
            heldBack = true;
        }
    }
    floatAt(app, axle + kAxleDrive) = float(fp::mul(f32(app, handling + kHandlingDriveShare), f32(app, axle + kAxleDrive)));

    const bool handbrake = byte(app, car + kCarHandbrake) != 0;
    if (handbrake)
    {
        const reg32 road = at(app, car + kCarRoad);
        const reg32 time = at(app, car + kCarWheelTurned) + (road == 1 ? 3 : road == 2 ? 4 : 1);
        put(app, car + kCarWheelTurned, i32(time) < kWheelFullTime ? time : reg32(kWheelFullTime));
    }
    else
    {
        put(app, car + kCarWheelTurned, 0);
    }

    const bool front = at(app, axle + kAxleFront) != 0;
    const auto drivePastGrip = [&] { return size(floatAt(app, axle + kAxleDrive)) > floatAt(app, axle + kAxleGrip); };
    if ((handbrake && !front) || (handbrake && front && heldBack && drivePastGrip()))
        return gripAxle(app, cpu, car, axle) ? AxleEnd::Gripped : AxleEnd::Stopped;

    if (heldBack && drivePastGrip() && !handbrake && byte(app, handling + kHandlingDriveCut) != 0)
    {
        const x86::Float limit = f32(app, axle + kAxleGrip);
        const x86::Float held = f32(app, axle + kAxleDrive);
        if (held > limit)
            floatAt(app, axle + kAxleDrive) = float(limit);
        else if (-limit > held)
            floatAt(app, axle + kAxleDrive) = float(-limit);
        floatAt(app, axle + kAxleSlip) = i32(at(app, kFrame)) % 4 == 0 ? 3.0f : 0.0f;
    }

    const std::optional<double> turned = feCore<0x4ea9c0, frontend::simd::angleOfFloatsValue>(
        app, cpu, at(app, axle + kAxleAcross), bitsOf(size(floatAt(app, axle + kAxleAlong))));
    if (!turned)
        return AxleEnd::Stopped;
    const float angle = float(*turned);
    const auto force = gear == kGearReverse ? sideForce(app, cpu, angle, kept, kReverseAngleScale, kReverseForceScale)
                                            : sideForce(app, cpu, angle, kept, kGearAngleScale, kGearForceScale);
    if (!force)
        return AxleEnd::Stopped;
    float side = *force;
    const x86::Float most = fp::mul(f32(app, handling + kHandlingSideLimit), f32(app, kSideLimitScale));
    if (!(most >= x86::Float(side)))
        side = float(most);

    const i32 brakeBits = signedBits(size(floatAt(app, car + kCarBrake)));
    const bool slow = signedBits(size(floatAt(app, car + kCarSpeedAlong))) < kBitsSlow;
    const i32 steering = i32(at(app, car + kCarSteering));
    const i32 steeringSize = steering < 0 ? i32(0u - reg32(steering)) : steering;
    if (brakeBits < kBits0_02 && slow && steeringSize >= kFullLock)
        side = float(fp::mul(side, f32(app, kFullLockShare)));
    else if (brakeBits < kBits0_015 && slow)
        side = float(fp::mul(side, f32(app, kLightBrakeShare)));

    const float across = floatAt(app, axle + kAxleAcross);
    if (0.0 > x86::Float(across))
        side = !(-x86::Float(side) >= x86::Float(across)) ? across : float(-x86::Float(side));
    else
        side = across > side ? side : across;
    if (front)
    {
        if (handbrake && signedBits(size(floatAt(app, car + kCarSpeedAlong))) < kBits0_5)
            side = 0.0f;
        floatAt(app, axle + kAxleForceAcross) = float(fp::mul(side, f32(app, kFrontSideShare)));
    }
    else
    {
        floatAt(app, axle + kAxleForceAcross) = float(fp::mul(side, f32(app, kBackSideShare)));
    }
    put(app, axle + kAxleForceUp, 0);
    floatAt(app, axle + kAxleForceAlong) = float(f32(app, axle + kAxleDrive));

    if (!slideAxle(app, cpu, car, axle))
        return AxleEnd::Stopped;
    if (wheelsTurned && !turnByWheels(app, cpu, axle, axle + kAxleForceAcross, axle + kAxleForceAlong))
        return AxleEnd::Stopped;
    floatAt(app, car + (front ? kCarFrontSlip : kCarBackSlip)) = float(f32(app, axle + kAxleSlip));
    return AxleEnd::Slid;
}

/* The thunk: car eax, axle edx; eax the car, edx as gripping or sliding leaves it. */
x86::reg32 axleForces(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const reg32 car = args.eax;
    const AxleEnd end = axleTyres(app, cpu, car, args.edx);
    if (end == AxleEnd::Stopped)
        return args.eax;
    const reg32 slidEdx = args.edx;
    saved.restore(cpu);
    args.edx = end == AxleEnd::Gripped ? slipSlider(app, car) : slidEdx;
    cpu.esp += 4;
    return args.eax;
}

/* fcomp's "not equal" on every value: unordered sets the flag too, so a NaN does not differ. */
bool differs(x86::Float a, x86::Float b) { return a < b || a > b; }

/* A float held at a bound, compared as the code does, as its bits' int. */
void holdBitsAtLeast(App* app, reg32 a, i32 bits)
{
    const reg32 value = at(app, a);
    put(app, a, i32(value) > bits ? value : reg32(bits));
}

void holdBitsAtMost(App* app, reg32 a, i32 bits)
{
    const reg32 value = at(app, a);
    put(app, a, i32(value) > bits ? reg32(bits) : value);
}

/* The setup's seven sliders back to their middle. */
void setupToMiddle(App* app, reg32 setup)
{
    for (reg32 i = 0; i < kSetupSliders; ++i)
        put(app, setup + kSetupSlider + i * 4, at(app, kDefaultSetup + i * 4));
}

/* The torque curve moved about its middle by the first slider. */
void tuneTorqueCurve(App* app, reg32 handling, reg32 sliders)
{
    const i32 middle = i32(at(app, handling + kHandlingRevsLimit) + at(app, handling + kHandlingRevsIdle)) / 2 / 256 + 3;
    const i32 top = i32(at(app, handling + kHandlingRevsLimit)) / 256 + 3;
    for (i32 i = 0; i < kTorquePoints; ++i)
    {
        const reg32 point = handling + kHandlingTorque + reg32(i) * 4;
        const float part = float(fp::mul(fp::mul(fp::add(f32(app, sliders), f32(app, kTorqueSliderShift)), f32(app, kTorqueSliderScale)), f32(app, point)));
        const x86::Float position = fp::div(x86::Float(i32(reg32(middle) - reg32(i))), x86::Float(i32(reg32(top) - reg32(middle))));
        const float move = float(fp::mul(position, part));
        floatAt(app, point) = float(fp::add(f32(app, point), move));
    }
    const x86::Float share = fp::add(fp::mul(fp::sub(f32(app, sliders), f32(app, kDefaultSetup)), f32(app, kFirstSliderTopShare)), 1.0);
    floatAt(app, handling + kHandlingTopSpeed) = float(fp::mul(share, f32(app, handling + kHandlingTopSpeed)));
}

/* One gear's numbers in the car, from its ratio in the handling and the fourth slider. */
void tuneGear(App* app, reg32 car, reg32 handling, reg32 sliders, reg32 gear, bool tuned)
{
    const reg32 ratio = handling + kHandlingRatios + gear * 4;
    const reg32 shareBase = handling + kHandlingGearShare + gear * 4;
    const reg32 inverse = car + kCarGearInverse + gear * 4;
    const reg32 share = car + kCarGearShare + gear * 4;
    if ((std::uint8_t(gameSettings(app).raceFlags) & kRaceFlagTorque) != 0)
        holdBitsAtLeast(app, shareBase, kBits1);
    if (tuned && differs(f32(app, sliders + kSliderFourth), f32(app, kDefaultSetup + kSliderFourth)))
        floatAt(app, ratio) = float(fp::mul(f32(app, sliders + kSliderFourth), f32(app, ratio)));
    if ((at(app, ratio) & kBitsNotSign) != 0)
        floatAt(app, inverse) = float(fp::div(1.0, f32(app, ratio)));
    else
        floatAt(app, inverse) = 0.01f;
    floatAt(app, share) = float(fp::mul(fp::div(f32(app, ratio), f32(app, handling + kHandlingShareBase)), f32(app, kGearShareScale)));
    floatAt(app, share) = float(fp::mul(f32(app, shareBase), f32(app, share)));
    const x86::Float revs = fp::mul(x86::Float(i32(at(app, handling + kHandlingRevsLimit))), f32(app, inverse));
    floatAt(app, car + kCarGearRevs + gear * 4) = float(fp::div(1.0, revs));
}

/* 0x4a78d6: the car's handling made for the race from the player's setup (seven sliders), when
 * the raceload makes the car.  The setup put back to the middle for a car with flag 2, over a link
 * and in the series.  A human player's car takes the handling's second set unless raceFlags has 4.
 * Each slider off its middle -- but over a link or in the series -- moves its part of the handling,
 * held to its range: the third +0x23c, +0x234 and +0x258; the second +0x1dc (the computer-drives
 * option a 20th more); the fifth +0x254 (a 20th less) and +0x294 by its square root; the sixth
 * +0x24c and +0x1c8; the first the torque's curve about its middle and +0x1c8; the simulation's
 * handling (raceFlags 1) has the curve and +0x1c8 by a constant and each gear's +0xd4 held to 1.
 * Then by the gear: its ratio by the fourth, its inverse (a 100th for none), its share, the limit's
 * revs by it; +0x1c8 by the fourth; the top gear's drag (+0x58c) from the torque at the top speed.
 * False when the game stopped. */
bool tuneHandling(App* app, CPU& cpu, reg32 car)
{
    if ((byte(app, car + kCarFlags) & kCarFlagSetupMiddle) != 0)
        setupToMiddle(app, at(app, car + kCarSetup));
    if (linkOrSeries(app))
        setupToMiddle(app, at(app, car + kCarSetup));
    const reg32 setup = at(app, car + kCarSetup);
    const reg32 sliders = setup + kSetupSlider;
    const reg32 handling = at(app, car + kCarHandling);
    const bool tuned = !linkOrSeries(app);
    const bool computerDrives = gameSettings(app).raceOptionComputerDrives != 0;

    const reg32 entry = at(app, car + kCarPlayer) * 0x6c;
    if (gameSettings(app).entryAt(entry).transmission == 1 && (std::uint8_t(gameSettings(app).raceFlags) & kRaceFlagFirstSet) == 0)
    {
        put(app, handling + 8, at(app, handling + 0xc));
        floatAt(app, handling + 0x1b8) = float(f32(app, handling + 0x1bc));
        for (reg32 i = 0; i < 8; ++i)
        {
            floatAt(app, handling + i * 4 + 0x54) = float(f32(app, handling + i * 4 + 0x74));
            floatAt(app, handling + i * 4 + 0xd4) = float(f32(app, handling + i * 4 + 0xf4));
            floatAt(app, handling + i * 4 + 0x94) = float(f32(app, handling + i * 4 + 0xb4));
        }
    }

    const reg32 third = sliders + kSliderThird;
    if (differs(f32(app, third), f32(app, kDefaultSetup + kSliderThird)) && tuned)
    {
        floatAt(app, handling + 0x23c) = float(fp::mul(f32(app, third), f32(app, handling + 0x23c)));
        const float grip = float(fp::add(fp::mul(fp::sub(f32(app, third), f32(app, kDefaultSetup + kSliderThird)), f32(app, 0x53dc4c)), f32(app, kDefaultSetup + kSliderThird)));
        floatAt(app, handling + 0x234) = float(fp::mul(f32(app, handling + 0x234), grip));
        holdBitsAtLeast(app, handling + 0x234, 0x3eeb851f);
        holdBitsAtMost(app, handling + 0x234, 0x3f0f5c29);
        const float away = float(fp::mul(fp::sub(1.0, f32(app, third)), f32(app, 0x53dc50)));
        const float scaled = float(fp::mul(away, f32(app, handling + 0x258)));
        floatAt(app, handling + 0x258) = float(fp::add(f32(app, handling + 0x258), scaled));
    }

    const reg32 second = sliders + kSliderSecond;
    if (differs(f32(app, second), f32(app, kDefaultSetup + kSliderSecond)) && tuned)
        floatAt(app, handling + 0x1dc) = float(fp::sub(f32(app, handling + 0x1dc), f32(app, second)));
    if (computerDrives && tuned)
        floatAt(app, handling + 0x1dc) = float(fp::add(f32(app, handling + 0x1dc), f32(app, 0x53dc54)));
    holdBitsAtMost(app, handling + 0x1dc, 0x3f400000);
    holdBitsAtLeast(app, handling + 0x1dc, 0x3e99999a);

    const reg32 fifth = sliders + kSliderFifth;
    if (differs(f32(app, fifth), f32(app, kDefaultSetup + kSliderFifth)) && tuned)
    {
        floatAt(app, handling + 0x254) = float(fp::mul(f32(app, fifth), f32(app, handling + 0x254)));
        holdBitsAtLeast(app, handling + 0x254, 0x3e99999a);
        holdBitsAtMost(app, handling + 0x254, 0x3f333333);
        const std::optional<double> root = feCore<0x4ea920, frontend::simc::squareRootOf>(app, cpu, at(app, fifth));
        if (!root)
            return false;
        floatAt(app, handling + 0x294) = float(fp::div(f32(app, handling + 0x294), x86::Float(*root)));
    }
    if (computerDrives && tuned)
        floatAt(app, handling + 0x254) = float(fp::add(f32(app, handling + 0x254), f32(app, 0x53dc58)));

    const reg32 sixth = sliders + kSliderSixth;
    if (differs(f32(app, sixth), f32(app, kDefaultSetup + kSliderSixth)) && tuned)
    {
        const float scale = float(fp::add(fp::mul(fp::add(f32(app, sixth), f32(app, 0x53dc5c)), f32(app, 0x53dc60)), 1.0));
        floatAt(app, handling + 0x24c) = float(fp::mul(f32(app, handling + 0x24c), scale));
        const x86::Float share = fp::add(fp::mul(fp::sub(f32(app, sixth), f32(app, kDefaultSetup)), f32(app, 0x53dc64)), 1.0);
        floatAt(app, handling + kHandlingTopSpeed) = float(fp::div(f32(app, handling + kHandlingTopSpeed), share));
    }

    if (differs(f32(app, sliders), f32(app, kDefaultSetup)) && tuned)
        tuneTorqueCurve(app, handling, sliders);

    if ((std::uint8_t(gameSettings(app).raceFlags) & kRaceFlagTorque) != 0)
    {
        for (i32 i = 0; i < kTorquePoints; ++i)
        {
            const reg32 point = handling + kHandlingTorque + reg32(i) * 4;
            floatAt(app, point) = float(fp::mul(f32(app, point), f32(app, 0x53dc74)));
        }
        floatAt(app, handling + kHandlingTopSpeed) = float(fp::mul(f32(app, handling + kHandlingTopSpeed), f32(app, 0x53dc78)));
    }

    for (reg32 gear = 0; i32(gear) < i32(at(app, handling + kHandlingGears)); ++gear)
        tuneGear(app, car, handling, sliders, gear, tuned);

    if (differs(f32(app, sliders + kSliderFourth), f32(app, kDefaultSetup + kSliderFourth)) && tuned)
        floatAt(app, handling + kHandlingTopSpeed) = float(fp::div(f32(app, handling + kHandlingTopSpeed), f32(app, sliders + kSliderFourth)));

    // The top gear's drag, from the torque at the revs of the top speed.
    const reg32 gears = at(app, handling + kHandlingGears) << 2;
    const i32 revs = fp::toInteger<i32>(fp::mul(f32(app, handling + kHandlingTopSpeed), f32(app, handling + gears + 0x90)));
    float drag = floatAt(app, handling + kHandlingTorque + (reg32(revs / 256) << 2));
    drag = float(fp::mul(drag, f32(app, car + gears + 0x568)));
    drag = float(fp::div(drag, f32(app, handling + gears + 0xd0)));
    if (tuned)
    {
        const x86::Float slider = fp::div(fp::add(1.0, f32(app, setup + 0x54)), f32(app, 0x53dc80));
        floatAt(app, car + kCarDrag) = float(fp::div(drag, slider));
    }
    else
    {
        floatAt(app, car + kCarDrag) = float(fp::div(drag, f32(app, 0x53dc84)));
    }
    const x86::Float cube = fp::mul(fp::mul(fp::mul(f32(app, handling + kHandlingTopSpeed), f32(app, 0x53dc88)), f32(app, handling + kHandlingTopSpeed)), f32(app, handling + kHandlingTopSpeed));
    floatAt(app, car + kCarDrag) = float(fp::div(f32(app, car + kCarDrag), cube));
    floatAt(app, car + kCarDrag) = float(fp::mul(f32(app, car + kCarDrag), f32(app, 0x53dc8c)));
    return true;
}

/* The thunk: car eax, left in eax; the other registers kept. */
x86::reg32 applyTuning(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    if (!tuneHandling(app, cpu, args.eax))
        return args.eax;
    saved.restore(cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4a8bb9: the automatic gearbox's choice: 0 in reverse or neutral or for no change, 2 for a
 * player's car finished, else the gear it changes to.  The revs each gear would turn at (the
 * wheels' turn by its ratio) against the limit's 5/6 and 7/8 (up, but not past the top), and half
 * the limit (down: by one or two, by the throttle) -- with the wheels on the limit, or the brake
 * past 0.9, these; else only down by one. */
reg32 gearChoice(App* app, reg32 car)
{
    const std::uint8_t gear8 = byte(app, car + kCarGear);
    if (gear8 < 2)
        return 0;
    if ((byte(app, car + kCarFlags) & kCarFlagPlayer) != 0 && at(app, car + kCarRaceStage) == kRaceStageFinished)
        return 2;
    const reg32 gear = gear8;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 limit = i32(at(app, handling + kHandlingRevsLimit));
    const i32 halfLimit = limit / 2;
    const i32 fiveSixths = i32(reg32(limit) - reg32(limit / 6));
    const i32 sevenEighths = i32(reg32(limit) - reg32(limit / 8));
    const float throttle = size(floatAt(app, car + kCarThrottle));
    const reg32 gears = at(app, handling + kHandlingGears);
    const x86::Float lastGearTop = fp::mul(x86::Float(limit), f32(app, car + (gears << 2) + kCarGearInverse - 8));
    const reg32 topGear = gears - 1 - (lastGearTop > f32(app, handling + kHandlingTopSpeed) ? 1 : 0);
    const bool belowTop = i32(gear) < i32(topGear);
    const reg32 upGear = belowTop ? gear + 1 : gear;
    const reg32 downOne = gear8 > 2 ? gear - 1 : gear;
    const reg32 downTwo = gear8 > 3 ? gear - 2 : gear;
    const auto revsIn = [&](reg32 inGear) {
        return fp::toInteger<i32>(fp::mul(f32(app, car + kCarWheelsTurn), f32(app, handling + kHandlingRatios + (inGear << 2))));
    };
    const i32 revs = revsIn(gear);
    const i32 revsDownOne = revsIn(downOne);
    const i32 revsDownTwo = revsIn(downTwo);
    const i32 revsSize = revs < 0 ? i32(0u - reg32(revs)) : revs;

    // Down to a gear not the one it is in: the choice; else no change.
    const auto to = [&](reg32 target) { return gear != target ? target : 0; };
    if (i32(at(app, car + kCarBrake)) <= kBits0_9 && revsSize <= limit)
    {
        if ((halfLimit > revsDownOne && downOne == 2)
            || (revsDownOne < fiveSixths && i32(downOne) > 2 && signedBits(throttle) < kBits0_1))
            return to(downOne);
        return 0;
    }
    if (revs > sevenEighths && belowTop)
        return to(upGear);
    const bool easedOff = signedBits(throttle) < kBits0_05;
    if (((halfLimit > revsDownTwo && downTwo == 2) || (revsDownTwo < fiveSixths && i32(downTwo) > 2)) && easedOff && gear != downTwo)
        return downTwo;
    // fcomp's "below" is !(>=) on every value: unordered sets it.
    if ((halfLimit > revsDownOne && downOne == 2 && !(x86::Float(throttle) >= f64(app, kDownOneThrottle)))
        || (revsDownOne < fiveSixths && i32(downOne) > 2 && easedOff))
        return to(downOne);
    return 0;
}

/* The thunk: car eax, the choice to eax. */
x86::reg32 automaticGear(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = gearChoice(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* A dot product (vertices', 0x4e01f0), as the x87 leaves it stored to a float. */
std::optional<float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return float(product);
}

/* 0x4aa4f4: the car's speeds along its axes (dot products of its speed with two of its axes); the
 * clutch, the throttle and the axles' slip cleared.  The code then has a matching of the revs to
 * the speed which it cannot reach: its test wants neutral, its next the gear not neutral. */
bool matchToSpeed(App* app, CPU& cpu, reg32 car)
{
    const auto forward = dot(app, cpu, car + kCarSpeed, car + 0x190);
    if (!forward)
        return false;
    floatAt(app, car + 0x4ec) = *forward;
    const auto along = dot(app, cpu, car + kCarSpeed, car + 0x1a8);
    if (!along)
        return false;
    floatAt(app, car + kCarSpeedAlong) = *along;
    put(app, car + kCarClutch, 0);
    put(app, car + kCarThrottle, 0);
    put(app, car + kCarFrontSlip, 0);
    put(app, car + kCarBackSlip, 0);
    return true;
}

/* The thunk: car eax, left in eax; the other registers kept. */
x86::reg32 matchRevsToSpeed(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    if (!matchToSpeed(app, cpu, args.eax))
        return args.eax;
    saved.restore(cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4aa79f: the car's drive put back: +0x506 and the gear 2, +0x507 0 (the code has this for a
 * human player's car and for the others, the same) and its forces, speeds along the axes, the
 * clutch, the throttle and the axles' slip cleared. */
void resetDrive(App* app, reg32 car)
{
    putByte(app, car + 0x506, 2);
    putByte(app, car + kCarGear, 2);
    putByte(app, car + 0x507, 0);
    for (const reg32 cleared : { 0x2e8u, 0x2ecu, 0x2f0u, 0x2f4u, 0x2f8u, 0x2fcu, 0x4ecu, 0x4f0u, 0x4f4u, 0x4f8u, 0x4fcu, 0x500u,
                                 kCarClutch, kCarThrottle, kCarFrontSlip, kCarBackSlip, 0x2e4u })
        put(app, car + cleared, 0);
}

/* The thunk: car eax, left there. */
x86::reg32 resetDriveState(App* app, x86::CPU& cpu, x86::Args& args)
{
    resetDrive(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* One vector scaled (vertices', 0x4e0050) into the result. */
bool scaleVector(App* app, CPU& cpu, reg32 vector, float scale, reg32 result)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, vector, scale, result);
    return !cpu.terminate;
}

/* One vector less the other, or with it added, in place (vertices', 0x4e0000 and 0x4dffb0). */
template <bool Add>
bool vectorWith(App* app, CPU& cpu, reg32 vector, reg32 other)
{
    if constexpr (Add)
        guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, vector, other, vector);
    else
        guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, vector, other, vector);
    return !cpu.terminate;
}

/* The other added by address (n in eax, the vector edx, the other ebx, the result ecx): settleCar's
 * last call, whose eax -- the vector past its end -- is what settleCar leaves, which its callers
 * on registers see; vertices' core gives nothing back. */
bool addByAddress(App* app, CPU& cpu, reg32 vector, reg32 other, reg32& eax)
{
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, vector, other, vector);
    if (cpu.terminate)
        return false;
    eax = vector + 12;  // the vector past its end, as the thunk leaves it
    return true;
}

/* A vector's part along the up axis taken off, scaled by `keep`, and put back: `part` a vector
 * on the guest's stack the callees are handed.  `last`: settleCar's last, its add by address, whose
 * eax goes to `eax`. */
bool dampUpPart(App* app, CPU& cpu, reg32 vector, reg32 up, reg32 part, float along, float keep, reg32 scaleFrom, reg32 scaleTo,
                bool last, reg32& eax)
{
    if (!scaleVector(app, cpu, up, along, part) || !vectorWith<false>(app, cpu, vector, part)
        || !scaleVector(app, cpu, scaleFrom, keep, scaleTo))
        return false;
    return last ? addByAddress(app, cpu, vector, part, eax) : vectorWith<true>(app, cpu, vector, part);
}

/* 0x4aa8e7: the car settled when it is all but stopped and upright: its speed's and its spin's
 * parts off the up axis damped -- by 0.94 and 0.98 at the race's start, else by 0.98 and 0.998
 * unless it hardly moves (two of the speeds' sizes under the constant).  eax is left as the code
 * leaves it, which callers on registers can see: where it stops early, the status word of its last
 * compare in the low half.  That eax; nullopt when the game stopped. */
std::optional<reg32> settleCarLeaving(App* app, CPU& cpu, reg32 car)
{
    const reg32 up = car + kCarUpAxis;
    if (i32(at(app, car + kCarRoadSpeed)) > kBits0_2)
        return car;
    const x86::Float upward = f32(app, up + 4);
    const x86::Float lowest = f64(app, kUprightLowest);
    if (!(upward >= lowest))
        return (up & 0xffff0000u) | fp::compare(upward, lowest);
    const float rolling = floatAt(app, car + kCarRoadState);
    const auto speedUp = dot(app, cpu, car + 0xc0, up);
    if (!speedUp)
        return std::nullopt;
    const auto spinUp = dot(app, cpu, car + 0xd8, up);
    if (!spinUp)
        return std::nullopt;
    const float rollingSize = size(rolling), speedSize = size(*speedUp), spinSize = size(*spinUp);
    float keepSpeed = 0.94f, keepSpin = 0.98f;
    if (half(app, car + kCarRaceStarted) != 0)
    {
        const x86::Float still = f32(app, kStillBelow);
        const x86::Float lastSum = fp::add(speedSize, spinSize);
        if (!(fp::add(rollingSize, speedSize) > still) && !(fp::add(rollingSize, spinSize) > still) && !(lastSum > still))
            return (car & 0xffff0000u) | fp::compare(lastSum, still);
        keepSpeed = 0.98f;
        keepSpin = 0.998f;
    }
    // The vector the callees are handed, on the guest's stack, under the frame the code has.
    cpu.esp -= kPartBelowEntry;
    const reg32 part = cpu.esp + kPartOverEsp;
    const reg32 speed = car + kCarSpeed, spin = car + kCarSpin;
    const auto alongSpeed = dot(app, cpu, up, speed);
    if (!alongSpeed)
        return std::nullopt;
    reg32 eax = 0;
    if (!dampUpPart(app, cpu, speed, up, part, *alongSpeed, keepSpeed, speed, speed, false, eax))
        return std::nullopt;
    const auto alongSpin = dot(app, cpu, up, spin);
    if (!alongSpin)
        return std::nullopt;
    if (!dampUpPart(app, cpu, spin, up, part, *alongSpin, keepSpin, part, part, true, eax))
        return std::nullopt;
    return eax;
}

/* settleCarLeaving: the eax it leaves; nullopt when the game stopped. */
std::optional<reg32> settleCar(App* app, CPU& cpu, reg32 car)
{
    return settleCarLeaving(app, cpu, car);
}

/* The thunk: car eax; eax as settleCar leaves it, the other registers kept. */
x86::reg32 settleAtRest(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Saved saved(cpu);
    const std::optional<reg32> left = settleCarLeaving(app, cpu, args.eax);
    if (!left)
        return args.eax;
    saved.restore(cpu);
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

}  // namespace handling
}  // namespace nfs3hp
