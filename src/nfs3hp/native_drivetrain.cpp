#include "game_settings.h"
#include "native_aiplan.h"
#include "native_controls_race.h"
#include "native_drivetrain.h"
#include "native_frontend_sima.h"
#include "native_guest.h"
#include "native_racestart.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* A car's engine, gears and its driver's inputs, as the car physics' full
 * step (0x4ad7a3) has them each frame: the pedals and the steering eased
 * towards what the driver asks (driverInputs), or worked out for a car the
 * computer drives (computerDriver); the gears changed -- by the driver, by
 * the automatic box (automaticGears), into reverse and out of it when the car
 * stands (autoReverse), the top gears barred where the handling says
 * (gearLimit) -- the engine's revs followed and its force at the wheels
 * worked out from the torque curve (engineForce, engineTorque), and the
 * wheels' grip on their surfaces (wheelGrip).
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (DRIVETRAIN_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
 *
 * The car is the record at eax on entry; what these functions read of it, as
 * their code uses it: +0x1c its slice of the track, +0xa4 its speed's vector,
 * +0xe4 its spin, +0x134 its grip's scale, +0x154 its heading, +0x160 the
 * track's index into the surfaces' table, +0x164 the race's stage (0
 * racing), +0x1f0 its player, +0x200 its flags (1 a player's), +0x210..+0x218
 * the computer's steering, +0x220 its driver's record (+0x24 steering eased,
 * +0x2c brake eased, +0x48 its skill, +0x54 the drag's), +0x2c4 its state (3
 * finished), +0x2e4, +0x4ec its velocity (+0x4f4 ahead), +0x504 the throttle
 * asked, +0x505 the brake asked, +0x506 the gear asked, +0x507 reversing on
 * its own, +0x508 the throttle, +0x509 the brake, +0x50a the last gear,
 * +0x50b the gear (0 reverse, 1 neutral, 2 first...), +0x50c shifting down,
 * +0x50d the clutch slipping, +0x510 the frames of a gear change (a word),
 * +0x514 the wheelspin's count, +0x518 the steering asked, +0x51c the
 * steering, +0x520 the computer's pedal, +0x528 its handling, +0x544.. its
 * gears' ratios by speed (+0x54c by gear) and the torque's (+0x56c), +0x58c
 * its drag, +0x590 the engine's revs, +0x594 and +0x598 the revs' limiter,
 * +0x5a4 its wheels' speed, +0x5b0 its slip, +0x5c8..+0x5e4 the wheels' grip,
 * +0x5e8 the steering's, +0x5ec the brake's, +0x5f0 the throttle's (0 to 1).
 * Its handling (+0x528): +0x8 the gears' count, +0x10 a gear change's
 * frames, +0x14 and +0x34 the revs a frame by the gear asked, +0x94 the
 * gears' revs by speed, +0x114 the torque's curve (by 256 revs), +0x1c0 the
 * idle's revs, +0x1c4 the top revs, +0x1c8 the speed past which the air
 * drags harder, +0x1f0 and +0x210 the pedals' steps by how far down,
 * +0x240 and +0x244 the steering's steps, +0x24c the grip's growth with
 * speed, +0x250 the engine's braking, +0x29c its kind, +0x2a4..+0x2b8 the
 * computer's speed by the bend.
 *
 * Each is a typed function under a name of its own, with an adapter under the game's name for the
 * callers still on registers and the dispatch: it leaves eax, the x87's stack and its status word as
 * the game's code does.  Other typed files' functions are called as C++ (guest::direct); every other
 * function of the game's by address, its registers kept (KeptRegisters); the game's constants
 * (floats and doubles in its data, 0x53dc90 to 0x53dd48) are read where the code reads them, and
 * the revs' drop by gear among its own code (0x4a78b6) read there too.  The x87's operations are
 * fp's, in the game's order, a float kept where the game stores one. */
namespace nfs3hp
{
namespace drivetrain
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;
using i16 = std::int16_t;

// Whether the race is under way, and its frames since the start (the start's countdown at 0x23c).
constexpr reg32 kRaceUnderWay = 0x7d3680, kRaceFrames = 0x7d3684;
// The track's slices' count.
constexpr reg32 kSliceCount = 0x5dd940;
// The computer's last bend, kept between frames.
constexpr reg32 kComputerBend = 0x55ed84;
// The surfaces' grip, by what wheelSurface gives.
constexpr reg32 kSurfaceGrip = 0x55ed30;
// The revs' drop by gear when the limiter cuts in: 8 dwords among the code.
constexpr reg32 kRevsDrop = 0x4a78b6;
// The game's functions called by address.  A vector's length and vectors scaled (0x4e06a0,
// 0x4e0050), the computer's speed at a slice (0x40e080), what a wheel is on (0x429b40) and the force
// feedback's kick (0x4714f0) are other files' (their headers), called as C++ (guest::direct).
constexpr reg32 kChop = 0x4dfd56;           // __CHP: st(0) to an integer, towards 0
constexpr reg32 kFloor = 0x4f1c30;          // floor of the double on the stack
constexpr reg32 kCarKnock = 0x416d90;       // snd::carKnock: the gear's clunk

// A car's fields and its handling record's: see above.  The torque curve (kHandlingTorque): a float
// each 256 revs, 41 points.
constexpr reg32 kCarSlice = 0x1c, kCarSpeedVector = 0xa4, kCarSpinVector = 0xe4, kCarGripScale = 0x134,
                kCarHeading = 0x154, kCarTrackKind = 0x160, kCarRaceStage = 0x164, kCarPlayer = 0x1f0,
                kCarFlags = 0x200, kCarComputerHeading = 0x210, kCarComputerBend = 0x214,
                kCarComputerSteering = 0x218, kCarDriver = 0x220, kCarState = 0x2c4,
                kCarThrottleOffState = 0x2e4, kCarVelocity = 0x4ec, kCarVelocityAhead = 0x4f4,
                kCarThrottleAsked = 0x504, kCarBrakeAsked = 0x505, kCarGearAsked = 0x506, kCarReversing = 0x507,
                kCarThrottle = 0x508, kCarBrake = 0x509, kCarLastGear = 0x50a, kCarGear = 0x50b,
                kCarShiftingDown = 0x50c, kCarClutchSlipping = 0x50d, kCarGearChangeFrames = 0x510,
                kCarWheelspin = 0x514, kCarSteeringAsked = 0x518, kCarSteering = 0x51c,
                kCarComputerPedal = 0x520, kCarHandling = 0x528, kCarRatiosBySpeed = 0x544,
                kCarRatiosByGear = 0x54c, kCarFirstGearRatio = 0x554, kCarTorqueRatios = 0x56c,
                kCarDrag = 0x58c, kCarRevs = 0x590, kCarLimiterOn = 0x594, kCarLimiterCutting = 0x598,
                kCarWheelSpeed = 0x5a4, kCarSlip = 0x5b0, kCarGripGrowth = 0x5c8, kCarWheelGrip = 0x5d8,
                kCarSteeringEased = 0x5e8, kCarBrakeEased = 0x5ec, kCarThrottleEased = 0x5f0;
constexpr reg32 kHandlingGears = 8, kHandlingChangeFrames = 0x10, kHandlingDownSteps = 0x14,
                kHandlingDownStepsBraking = 0x34, kHandlingGearRevs = 0x94, kHandlingTorque = 0x114,
                kHandlingIdleRevs = 0x1c0, kHandlingTopRevs = 0x1c4, kHandlingDragSpeed = 0x1c8,
                kHandlingBrakeUp = 0x1f0, kHandlingBrakeDown = 0x210, kHandlingSteerAway = 0x240,
                kHandlingSteerBack = 0x244, kHandlingGripGrowth = 0x24c, kHandlingEngineBraking = 0x250,
                kHandlingKind = 0x29c, kHandlingBendSmall = 0x2a4, kHandlingBendMiddle = 0x2a8,
                kHandlingBendLarge = 0x2ac, kHandlingBendScales = 0x2b0;
// A driver's record: its steering and brake eased or at once, its skill.
constexpr reg32 kDriverSteeringEased = 0x24, kDriverBrakeEased = 0x2c, kDriverSkill = 0x48, kDriverDrag = 0x54;
// A player's car (+0x200) and a car that finished (+0x2c4).
constexpr reg32 kFlagPlayer = 1, kStateFinished = 3;
// The start's countdown, in frames.
constexpr i32 kCountdownFrames = 0x23c;
// The gears as +0x50b has them.
constexpr reg32 kReverse = 0, kNeutral = 1, kFirstGear = 2;
// How near the idle the revs count as idling.
constexpr reg32 kNearlyIdle = 0x32;
// The speed a car all but stands under, as bits: 0.5 as a float.
constexpr i32 kStandingSpeedBits = 0x3f000000;
// What the automatic box compares as bits: 0.9, 0.1 and 0.05 as floats.
constexpr i32 kThrottleNearlyFullBits = 0x3f666666, kSlipSomeBits = 0x3dcccccd, kSlipSmallBits = 0x3d4ccccd;
constexpr reg32 kSlipSmallDouble = 0x53dcac;  // a double: 0.05

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putHalf(App* app, reg32 a, reg16 v) { app->getMemory<reg16>(a) = v; }
x86::Float f32(App* app, reg32 a) { return x86::Float(app->getMemory<float>(a)); }
x86::Float f64(App* app, reg32 a) { return x86::Float(app->getMemory<double>(a)); }
x86::Float i32At(App* app, reg32 a) { return x86::Float(x86::sreg32(at(app, a))); }

void push(App* app, CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

/* A call as the code makes it: the return address's slot, and esp as the callee leaves it.
 * False when the game stopped in it. */
bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* What fnstsw ax; sahf makes of a compare: "above", no carry (C0) and no zero (C3). */
bool above(const CPU& cpu) { return (cpu.ah & 0x41) == 0; }

reg32 floatBits(float f)
{
    reg32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

x86::Float floatAt(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return x86::Float(f);
}

/* fcom's answers with what it leaves for the status word (the check compares it): a over b, not
 * so for a NaN; a below b, so for a NaN. */
bool compareAbove(CPU& cpu, const x86::Float& a, const x86::Float& b)
{
    cpu.fpu.compare(a, b);
    return !cpu.fpu.status.c0 && !cpu.fpu.status.c3;
}

bool compareBelow(CPU& cpu, const x86::Float& a, const x86::Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.c0;
}

/* An x87 fabs in the game's two steps: only a value under 0 is negated, so never a NaN, and the
 * rest is kept to the bit. */
float sizeOf(float value)
{
    return 0.0 > x86::Float(value) ? float(-x86::Float(value)) : value;
}

/* An int's size as the game's code has it: the lowest int stays itself. */
reg32 magnitude(reg32 value)
{
    return i32(value) < 0 ? 0u - value : value;
}

void putFloat(App* app, reg32 a, float f) { app->getMemory<float>(a) = f; }

/* What a function's frame keeps of its caller's across a call by address: every register but eax. */
class KeptRegisters
{
public:
    explicit KeptRegisters(CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp), esp_(cpu.esp)
    {
    }
    ~KeptRegisters()
    {
        cpu_.ebx = ebx_;
        cpu_.ecx = ecx_;
        cpu_.edx = edx_;
        cpu_.esi = esi_;
        cpu_.edi = edi_;
        cpu_.ebp = ebp_;
        cpu_.esp = esp_;
    }

private:
    CPU& cpu_;
    reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_, esp_;
};

/* The length of the vector at `vector` (vertices', 0x4e06a0); nullopt when the game stopped. */
std::optional<x86::Float> vectorLength(App* app, CPU& cpu, reg32 vector)
{
    const double length = guest::direct<0x4e06a0, vertices::vectorLength>(app, cpu, vector);
    if (cpu.terminate)
        return std::nullopt;
    return x86::Float(length);
}

/* __CHP: the value to an integer, towards 0, as a whole value; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<x86::Float> chop(App* app, CPU& cpu, const x86::Float& value)
{
    return x86::Float(guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value)));
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated).  tools/game_checks does not
 * build those files: there it goes by address, through its ByAddress, as the game's code calls it,
 * every register but eax kept.  A reg32 result is the eax the function leaves: eax set to it (in the
 * game too); after any other, eax spoilt there, so that code still reading what the callee left in
 * eax differs there. */
template <reg32 Address, auto Core, typename... Args>
auto feCore(App* app, CPU& cpu, Args... args)
{
    const auto run = [&] {
#ifdef NFS_GAME_CHECKS
        const guest::KeptRegisters kept(cpu);
        return guest::ByAddress<Core>::template call<Address>(app, cpu, args...);
#else
        return guest::direct<Address, Core>(app, cpu, args...);
#endif
    };
    using Result = decltype(run());
    const auto spoil = [&] {
#ifdef NFS_GAME_CHECKS
        cpu.eax = 0x5b0115edu;
#endif
    };
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
                cpu.eax = result;  // a menu core's result: the eax it leaves, as the call by address does
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

/* The track's heading at the slice (sima's, 0x49fee0); nullopt when the game stopped. */
std::optional<reg32> sliceHeading(App* app, CPU& cpu, reg32 slice)
{
    return feCore<0x49fee0, frontend::sima::sliceHeading>(app, cpu, slice);
}

/* The computer's speed at the slice (aiplan's, 0x40e080); nullopt when the game stopped. */
std::optional<x86::Float> sliceSpeed(App* app, CPU& cpu, reg32 slice)
{
    const float speed = guest::direct<0x40e080, aiplan::targetSpeed>(app, cpu, slice);
    if (cpu.terminate)
        return std::nullopt;
    return x86::Float(speed);
}

/* Whether the car's velocity's length, as its sign has it, is under 0.5, as bits: a NaN's sign
 * decides.  The vector's length asked twice as the game's code does; nullopt when the game stopped. */
std::optional<bool> isStanding(App* app, CPU& cpu, reg32 car)
{
    const std::optional<x86::Float> first = vectorLength(app, cpu, car + kCarVelocity);
    if (!first)
        return std::nullopt;
    const bool negative = compareAbove(cpu, 0.0, *first);
    const std::optional<x86::Float> second = vectorLength(app, cpu, car + kCarVelocity);
    if (!second)
        return std::nullopt;
    return i32(floatBits(float(negative ? -*second : *second))) < kStandingSpeedBits;
}

/* Whether the car is not a player's that finished. */
bool notFinished(App* app, reg32 car)
{
    return (byte(app, car + kCarFlags) & kFlagPlayer) == 0 || at(app, car + kCarState) != kStateFinished;
}

/* Whether the car's player has the automatic box. */
bool hasAutomaticBox(App* app, reg32 car)
{
    return gameSettings(app).entryAt(at(app, car + kCarPlayer) * 0x6c).transmission == 1;
}

/* 0x4e0050 (vertices') on one vector, in place: scaled by the float of those bits.  False when the
 * game stopped. */
bool scaleVector(App* app, CPU& cpu, reg32 vector, reg32 factorBits)
{
    float factor;
    std::memcpy(&factor, &factorBits, sizeof factor);
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, vector, factor, vector);
    return !cpu.terminate;
}

/* snd::carKnock, the gear's clunk: the pan (split screen's sides) and 0 on the stack, ecx and ebx
 * 0, kind 0xd, slot -1.  Every register but eax kept, as the callers' frames do. */
bool knockGear(App* app, CPU& cpu, reg32 pan)
{
    const KeptRegisters kept(cpu);
    push(app, cpu, pan);
    push(app, cpu, 0);
    cpu.ecx = 0;
    cpu.ebx = 0;
    cpu.edx = 0xd;
    cpu.eax = 0xffffffff;
    return byAddress(app, cpu, kCarKnock);
}

}  // namespace

/* The inputs of car when the computer drives it (another's, or the player's when the game drives
 * for him).  Its steering ([+0x218], the asked +0x504 - +0x505 + 0x100, by 0x20 a frame at most)
 * and the speed it aims at: the computer's speed of the slice 7 ahead (back, when the race runs
 * backwards; 0x40e080) by the bend's scale -- the bend [kComputerBend] the track's heading 10
 * slices on or back (0x49fee0) less the car's, eased with +0x214 seven to one, against the
 * handling's +0x2a4, +0x2a8, +0x2ac.  The pedal [+0x520] (0x100 the middle) pushed towards the
 * speed, held at the revs the speed gives in between; to the throttle (+0x508) over 0xff, else to
 * the brake (+0x509).  False when the game stopped.  The speed's gap compares as bits as the game's
 * does, a NaN's sign deciding; the x87's status word left as its compare leaves it. */
bool driveByComputer(App* app, CPU& cpu, reg32 car)
{
    constexpr i32 kLookAhead = 7, kBendLookAhead = 10;
    constexpr i32 kOneBits = 0x3f800000;
    constexpr i32 kFullTurn = 0x400, kHalfTurn = 0x200, kQuarterTurn = 0x100;
    constexpr i32 kPedalMiddle = 0x100, kSteeringStep = 0x20;
    constexpr reg32 kUnderSpeedScale = 0x53dc90, kOverSpeedScale = 0x53dc94, kHeldScale = 0x53dc98;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 slices = i32(at(app, kSliceCount));
    const i32 slice = i32(at(app, car + kCarSlice));
    const reg32 heading = at(app, car + kCarHeading);
    const i32 lookAhead = gameSettings(app).backwards != 0 ? -kLookAhead : kLookAhead;

    // The steering asked.
    i32 steering = i32(reg32(byte(app, car + kCarThrottleAsked)) + 0x100 - reg32(byte(app, car + kCarBrakeAsked)));

    // How far the car's heading is from the computer's line, both ways round.
    i32 off = i32(magnitude(heading - at(app, car + kCarComputerHeading)));
    const i32 otherWay = i32(reg32(kFullTurn) - reg32(off));
    if (otherWay < off)
        off = otherWay;

    // The track's heading 10 slices back when it faces the wrong way, else 10 on, wrapped round.
    i32 headingSlice;
    if (off > kQuarterTurn)
    {
        headingSlice = i32(reg32(slice) - kBendLookAhead);
        if (headingSlice < 0)
            headingSlice = i32(reg32(slices) + reg32(headingSlice));
    }
    else
    {
        headingSlice = i32(reg32(slice) + kBendLookAhead);
        if (headingSlice >= slices)
            headingSlice = i32(reg32(headingSlice) - reg32(slices));
    }
    const std::optional<reg32> trackHeading = sliceHeading(app, cpu, reg32(headingSlice));
    if (!trackHeading)
        return false;

    // The bend, half a turn either way, eased.
    i32 bend = i32(*trackHeading - heading);
    if (bend > kHalfTurn)
        bend = i32(reg32(bend) - kFullTurn);
    else if (bend < -kHalfTurn)
        bend = i32(reg32(bend) + kFullTurn);
    bend = i32(at(app, car + kCarComputerBend) * 7 + reg32(bend)) / 8;
    put(app, kComputerBend, reg32(bend));
    put(app, car + kCarComputerBend, reg32(bend));

    // The bend's scale of the speed.
    reg32 scale = reg32(kOneBits);
    const i32 bendSize = i32(magnitude(reg32(bend)));
    if (bendSize > i32(at(app, handling + kHandlingBendSmall)))
    {
        if (bendSize < i32(at(app, handling + kHandlingBendMiddle)))
            scale = at(app, handling + kHandlingBendScales);
        else if (bendSize < i32(at(app, handling + kHandlingBendLarge)))
            scale = at(app, handling + kHandlingBendScales + 4);
        else
            scale = at(app, handling + kHandlingBendScales + 8);
    }

    // The slice 7 ahead (or back), wrapped round: its speed.
    i32 speedSlice = i32(reg32(slice) + reg32(lookAhead));
    if (lookAhead >= 0)
    {
        if (speedSlice >= slices)
            speedSlice = i32(reg32(speedSlice) - reg32(slices));
    }
    else if (speedSlice < 0)
    {
        speedSlice = i32(reg32(slices) + reg32(speedSlice));
    }
    const std::optional<x86::Float> sliceTarget = sliceSpeed(app, cpu, reg32(speedSlice));
    if (!sliceTarget)
        return false;
    float target = float(*sliceTarget);
    target = float(fp::mul(x86::Float(target), floatAt(scale)));
    const float gap = float(fp::sub(x86::Float(target), f32(app, car + kCarVelocityAhead)));
    const i32 gapBits = i32(floatBits(gap));

    // The steering eased: 0x20 a frame at most.
    const i32 lastSteering = i32(at(app, car + kCarComputerSteering));
    if (steering > lastSteering)
    {
        const i32 most = i32(reg32(lastSteering) + kSteeringStep);
        if (!(most > steering))
            steering = most;
    }
    else if (steering < lastSteering)
    {
        const i32 most = i32(reg32(lastSteering) - kSteeringStep);
        if (most > steering)
            steering = most;
    }
    put(app, car + kCarComputerSteering, reg32(steering));

    // The pedal back to the middle when it is the wrong way round.
    const bool overSpeed = compareAbove(cpu, 0.0, x86::Float(gap));
    bool middle = false;
    if (overSpeed && i32(at(app, car + kCarComputerPedal)) > kPedalMiddle)
    {
        put(app, car + kCarComputerPedal, kPedalMiddle);
        middle = true;
    }
    if (!middle && gapBits > kOneBits && i32(at(app, car + kCarComputerPedal)) < kPedalMiddle)
        put(app, car + kCarComputerPedal, kPedalMiddle);

    // Its step: down on it under the speed, off it over, else held where the revs the speed gives
    // in the gear are.
    i32 step = 0;
    if (gapBits > kOneBits)
    {
        const i32 pushed = i32(reg32(fp::toInteger<i32>(fp::mul(x86::Float(gap), f32(app, kUnderSpeedScale)))) + 4);
        step = pushed > 0x10 ? 0x10 : pushed;
    }
    else if (overSpeed)
    {
        const i32 eased = i32(reg32(fp::toInteger<i32>(fp::mul(x86::Float(gap), f32(app, kOverSpeedScale)))) - 8);
        step = eased > -0x18 ? eased : -0x18;
    }
    else
    {
        const reg32 ratio = reg32(byte(app, car + kCarGear)) * 4 + handling + kHandlingGearRevs;
        const i32 revs = fp::toInteger<i32>(fp::mul(f32(app, car + kCarVelocityAhead), f32(app, ratio)));
        const i32 idle = i32(at(app, handling + kHandlingIdleRevs));
        float held = 0.0f;
        if (i32(at(app, car + kCarRevs)) > idle)
        {
            const i32 range = i32(at(app, handling + kHandlingTopRevs) - reg32(idle));
            const i32 over = i32(reg32(revs) - reg32(idle));
            held = float(x86::Float(i32(std::int64_t(over) / range)));
        }
        put(app, car + kCarComputerPedal,
            reg32(fp::toInteger<i32>(fp::mul(x86::Float(held), f32(app, kHeldScale)))) + kPedalMiddle);
    }

    // The step taken (none held), as far as the steering asked lets it, 0 at least.
    i32 pedal = i32(at(app, car + kCarComputerPedal) + reg32(step));
    if (pedal > steering)
        pedal = steering;
    if (pedal <= 0)
        pedal = 0;
    put(app, car + kCarComputerPedal, reg32(pedal));

    // To the throttle over the middle, else to the brake.
    if (pedal >= 0xff)
    {
        const i32 throttle = pedal - kPedalMiddle;
        putByte(app, car + kCarThrottle, reg8(throttle < 0 ? 0 : throttle));
        putByte(app, car + kCarBrake, 0);
    }
    else
    {
        putByte(app, car + kCarThrottle, 0);
        putByte(app, car + kCarBrake, reg8(0xff - pedal));
    }
    return true;
}

/* 0x4a83d3: driveByComputer for car eax, the car left in eax. */
void computerDriver(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!driveByComputer(app, cpu, car))
        return;
    cpu.eax = car;
    cpu.esp += 4;
}

constexpr reg32 kRevsToCurve = 0x53dc9c;  // a float: a curve point per rev
constexpr float kLastCurvePoint = 40.0f;

/* The engine's torque at `revs`: the curve between the two points either side, or the point
 * itself on one.  The line through the two points is worked out and then read at the position,
 * as the game does: this is not the same number as the two points' mix, in the last bits. */
float torqueAt(App* app, reg32 car, i32 revs)
{
    float position = float(fp::mul(x86::Float(revs), f32(app, kRevsToCurve)));
    if (position > kLastCurvePoint)
        position = kLastCurvePoint;
    else if (position < 0.0f)
        position = 0.0f;
    const reg32 curve = at(app, car + kCarHandling) + kHandlingTorque;
    const auto point = [&](i32 index) { return app->getMemory<float>(curve + reg32(index) * 4); };
    const float below = float(std::floor(position));
    const i32 index = i32(below);
    if (position == below)
        return point(index);
    const float low = point(index);
    const float slope = float(fp::sub(x86::Float(point(index + 1)), x86::Float(low)));
    const float start = float(fp::sub(x86::Float(low), fp::mul(x86::Float(index), x86::Float(slope))));
    return float(fp::add(fp::mul(x86::Float(slope), x86::Float(position)), x86::Float(start)));
}

/* 0x4a8a99: torqueAt for car eax at revs edx, onto the x87's stack, for the callers still on
 * registers; eax and edx as the game's code leaves them. */
void engineTorque(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const float torque = torqueAt(app, car, i32(cpu.edx));
    std::memcpy(&cpu.eax, &torque, sizeof torque);
    cpu.edx = at(app, car + kCarHandling);
    cpu.fpu.push(x86::Float(torque));
    cpu.esp += 4;
}

/* The automatic box of car, in a forward gear: up a gear (+0x50c cleared) past seven eighths of
 * the top revs while the gear above is not the last but one (the last too, when the top gear's
 * revs at the handling's +0x1c8 are over the top), down two or one (+0x50c set) below half the
 * revs or five sixths with the slip under 0.05 -- with the throttle under 0.9 and the revs under
 * the top, only down one, and with the slip under 0.1.  The gear changed: the last kept at +0x50a,
 * the change's frames from the handling, and its clunk played for the player at this machine (each
 * car on split screen).  Returns what the game's code leaves in eax, which nothing reads.
 * The x87's status word left as its compares leave it.  The slip and the throttle compare as bits
 * as the game's do, a NaN's sign deciding. */
reg32 shiftAutomatically(App* app, CPU& cpu, reg32 car)
{
    const reg32 handling = at(app, car + kCarHandling);
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const i32 halfTop = top / 2;
    const i32 fiveSixths = i32(reg32(top) - reg32(top / 6));
    const i32 sevenEighths = i32(reg32(top) - reg32(top / 8));
    const reg32 gear = byte(app, car + kCarGear);
    if (gear < kFirstGear)
        return car;

    // The slip, either way.  Only a value under 0 is negated: never a NaN.
    reg32 slip = at(app, car + kCarSlip);
    if (compareAbove(cpu, 0.0, f32(app, car + kCarSlip)))
        slip = floatBits(float(-f32(app, car + kCarSlip)));
    const bool slipSmall = i32(slip) < kSlipSmallBits;

    // The clunk's pan: split screen's sides.
    const reg32 pan = gameSettings(app).playMode == 1 ? (at(app, car + kCarPlayer) != 0 ? 0xc000 : 0x3fff) : 0;

    // The top gear's revs at the speed where the air drags harder: over the top, the top gear is used.
    const reg32 gears = at(app, handling + kHandlingGears);
    const x86::Float topGearSpeed =
        fp::mul(i32At(app, handling + kHandlingTopRevs), f32(app, gears * 4 + car + kCarRatiosBySpeed));
    const bool useTopGear = compareAbove(cpu, topGearSpeed, f32(app, handling + kHandlingDragSpeed));
    const i32 lastToShiftFrom = i32(gears - 1 - (useTopGear ? 1 : 0));

    // The gears up one, down one and down two, where there are.
    const reg32 oneUp = i32(gear) < lastToShiftFrom ? gear + 1 : gear;
    const reg32 oneDown = gear > kFirstGear ? gear - 1 : gear;
    const reg32 twoDown = gear > kFirstGear + 1 ? gear - 2 : gear;

    // The revs at the wheels' speed in a gear.
    const auto revsIn = [&](reg32 inGear) {
        return fp::toInteger<i32>(
            fp::mul(f32(app, car + kCarWheelSpeed), f32(app, inGear * 4 + handling + kHandlingGearRevs)));
    };
    const i32 revs = revsIn(gear);
    const i32 revsOneDown = revsIn(oneDown);
    const i32 revsTwoDown = revsIn(twoDown);

    const auto change = [&](reg32 to, reg8 down) -> reg32 {
        putByte(app, car + kCarShiftingDown, down);
        putByte(app, car + kCarLastGear, reg8(gear));
        putByte(app, car + kCarGear, reg8(to));
        putHalf(app, car + kCarGearChangeFrames, half(app, handling + kHandlingChangeFrames));
        if (i32(gameSettings(app).playMode) >= 2)
        {
            const reg32 player = at(app, car + kCarPlayer);
            if (player != gameSettings(app).localPlayer)
                return player;
        }
        knockGear(app, cpu, pan);
        return cpu.eax;
    };

    const reg32 revsAbove = revs < 0 ? 0u - reg32(revs) : reg32(revs);
    const bool hard = i32(at(app, car + kCarThrottleEased)) > kThrottleNearlyFullBits || i32(revsAbove) > top;
    if (!hard)
    {
        // Light on the throttle, under the top revs: only down one.
        const bool down = (halfTop > revsOneDown && oneDown == kFirstGear)
                          || (revsOneDown < fiveSixths && oneDown > kFirstGear && i32(slip) < kSlipSomeBits);
        if (!down)
            return reg32(revsOneDown);
        return gear == oneDown ? gear : change(oneDown, 1);
    }
    // Up a gear: oneUp is the gear above here.
    if (revs > sevenEighths && i32(gear) < lastToShiftFrom)
        return change(oneUp, 0);
    // Down two.
    if (slipSmall
        && ((halfTop > revsTwoDown && twoDown == kFirstGear) || (revsTwoDown < fiveSixths && twoDown > kFirstGear))
        && gear != twoDown)
        return change(twoDown, 1);
    // Down one.  A NaN's slip counts as below 0.05 in the first compare, as bits in the second.
    const bool toFirst = halfTop > revsOneDown && oneDown == kFirstGear
                         && compareBelow(cpu, floatAt(slip), f64(app, kSlipSmallDouble));
    if (!toFirst && !(revsOneDown < fiveSixths && oneDown > kFirstGear && slipSmall))
        return reg32(revsOneDown);
    return gear == oneDown ? gear : change(oneDown, 1);
}

/* 0x4a90a0: shiftAutomatically for car eax. */
void automaticGears(App* app, CPU& cpu)
{
    cpu.eax = shiftAutomatically(app, cpu, cpu.eax);
    cpu.esp += 4;
}

/* What gearLimit's bar compares with the car's speed: the top revs in the first forward gear's ratio. */
x86::Float firstGearTopSpeed(App* app, reg32 car)
{
    return fp::mul(i32At(app, at(app, car + kCarHandling) + kHandlingTopRevs), f32(app, car + kCarFirstGearRatio));
}

/* Car with a manual box kept out of its fifth gear and over (to the fourth, the third the last)
 * while the top revs in the first forward gear are over its speed.  A float compare, as the x87's:
 * neither answers "above" for a NaN. */
void barTopGears(App* app, reg32 car)
{
    if (firstGearTopSpeed(app, car) > f32(app, car + kCarVelocityAhead) && byte(app, car + kCarGear) > 4)
    {
        putByte(app, car + kCarGear, 4);
        putByte(app, car + kCarLastGear, 3);
    }
}

/* 0x4a9769: barTopGears for car eax; the compare's status word in the x87 and in ax, overwritten
 * by the car in eax when it says "above". */
void gearLimit(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    cpu.fpu.compare(firstGearTopSpeed(app, car), f32(app, car + kCarVelocityAhead));
    cpu.ax = cpu.fpu.status.word;
    if (above(cpu))
        cpu.eax = car;
    barTopGears(app, car);
    cpu.esp += 4;
}

/* The car's gear asked, when it all but stands (its velocity's length under 0.5), as its driver's
 * pedals say.  False when the game stopped. */
bool askGearStanding(App* app, CPU& cpu, reg32 car)
{
    const auto asked = [&](reg8 gear) {
        putByte(app, car + kCarGearAsked, gear);
        putByte(app, car + kCarReversing, 0);
    };
    const reg32 gear = byte(app, car + kCarGear);
    const reg32 throttleAsked = byte(app, car + kCarThrottleAsked);
    const bool throttleOff = byte(app, car + kCarThrottle) == 0;

    // The brake held, the throttle off, in first or neutral: into reverse.
    std::optional<bool> standing = isStanding(app, cpu, car);
    if (!standing)
        return false;
    if (*standing && byte(app, car + kCarBrakeAsked) > 0x80 && throttleOff && (gear == kFirstGear || gear == kNeutral)
        && notFinished(app, car) && byte(app, car + kCarReversing) == 0)
    {
        putByte(app, car + kCarGearAsked, kReverse);
        putByte(app, car + kCarReversing, 1);
        return true;
    }
    // The throttle in reverse (its brake's pedal, swapped) or in neutral: into first.
    standing = isStanding(app, cpu, car);
    if (!standing)
        return false;
    if (*standing && ((throttleAsked > 0x80 && gear == kReverse) || (throttleAsked != 0 && gear == kNeutral))
        && throttleOff
        && ((gear == kReverse && byte(app, car + kCarReversing) != 0)
            || (gear == kNeutral && byte(app, car + kCarReversing) == 0)))
    {
        asked(reg8(kFirstGear));
        return true;
    }
    // The throttle in neutral with the automatic box: into first.
    if (throttleAsked != 0 && gear == kNeutral && throttleOff && hasAutomaticBox(app, car))
    {
        asked(reg8(kFirstGear));
        return true;
    }
    // Off both in reverse or first, at idle: into neutral, at rest.
    standing = isStanding(app, cpu, car);
    if (!standing)
        return false;
    const reg32 handling = at(app, car + kCarHandling);
    if (*standing && throttleOff && byte(app, car + kCarReversing) == 0 && throttleAsked < 8
        && (gear == kReverse || gear == kFirstGear)
        && i32(at(app, handling + kHandlingIdleRevs) + kNearlyIdle) > i32(at(app, car + kCarRevs)))
    {
        asked(reg8(kNeutral));
        putByte(app, car + kCarBrake, 0xff);
        put(app, car + kCarVelocity, 0);
        put(app, car + kCarVelocity + 4, 0);
        put(app, car + kCarVelocity + 8, 0);
    }
    return true;
}

/* Car put into reverse, and out of it, as its driver's pedals say when it all but stands: a
 * player's car with the automatic box, once the race is under way and the countdown over -- else
 * nothing (+0x507 cleared).  Reversing on its own, its pedals swapped.  Gives +0x507.  The x87's
 * status word left as its compares leave it. */
reg32 reverseOnItsOwn(App* app, CPU& cpu, reg32 car)
{
    if (!hasAutomaticBox(app, car) || !notFinished(app, car) || at(app, kRaceUnderWay) == 0
        || i32(at(app, kRaceFrames)) < kCountdownFrames)
    {
        putByte(app, car + kCarReversing, 0);
        return 0;
    }
    if (byte(app, car + kCarGear) != kReverse)
        putByte(app, car + kCarReversing, 0);
    if (byte(app, car + kCarGearAsked) != byte(app, car + kCarGear))
        putByte(app, car + kCarReversing, 0);
    else if (!askGearStanding(app, cpu, car))
        return 0;
    // Reversing on its own: the throttle's pedal brakes, the brake's drives.
    if (byte(app, car + kCarReversing) != 0 && byte(app, car + kCarGear) == kReverse)
    {
        const reg8 throttleAsked = byte(app, car + kCarThrottleAsked), brakeAsked = byte(app, car + kCarBrakeAsked);
        putByte(app, car + kCarBrakeAsked, throttleAsked);
        putByte(app, car + kCarThrottleAsked, brakeAsked);
    }
    return byte(app, car + kCarReversing);
}

/* 0x4a97cb: reverseOnItsOwn for car eax, its answer in eax. */
void autoReverse(App* app, CPU& cpu)
{
    const reg32 reversing = reverseOnItsOwn(app, cpu, cpu.eax);
    if (cpu.terminate)
        return;
    cpu.eax = reversing;
    cpu.esp += 4;
}

/* A number moved towards a target: by the difference at most `step`, the sign of the difference.
 * What the two moves of the eased pedals and the steering have in common. */
i32 stepTowards(i32 difference, i32 stepUp, i32 stepDown)
{
    if (difference >= 0)
        return stepUp > difference ? difference : stepUp;
    const i32 away = i32(0u - reg32(difference));
    return i32(0u - reg32(stepDown > away ? away : stepDown));
}

/* The throttle (+0x508) eased towards what is asked (+0x504) by 16 a frame (down, more with its
 * driver's skill, in a single race), the brake (+0x509) towards +0x505 by the handling's steps
 * (+0x1f0 up, +0x210 down, by how far down it is, by 32) or at once.  The skill compares as bits
 * as the game's does. */
void easePedals(App* app, reg32 car)
{
    constexpr i32 kThrottleStep = 0x10;
    constexpr i32 kSkillOneBits = 0x3f800000;
    constexpr reg32 kSkillAdd = 0x53dcb4, kSkillScale = 0x53dcb8;
    const reg32 driver = at(app, car + kCarDriver);
    i32 throttleDown = kThrottleStep;
    if (i32(at(app, driver + kDriverSkill)) > kSkillOneBits && i32(gameSettings(app).playMode) <= 1
        && gameSettings(app).raceType != 1 && gameSettings(app).raceType != 2)
    {
        const i32 more = fp::toInteger<i32>(fp::mul(fp::add(f32(app, driver + kDriverSkill), f32(app, kSkillAdd)), f32(app, kSkillScale)));
        throttleDown = reg8(reg32(kThrottleStep) + reg32(more));
    }
    const reg8 throttle = byte(app, car + kCarThrottle);
    putByte(app, car + kCarThrottle,
            reg8(throttle + stepTowards(i32(byte(app, car + kCarThrottleAsked)) - i32(throttle), kThrottleStep, throttleDown)));

    if (at(app, driver + kDriverBrakeEased) == 0)
    {
        putByte(app, car + kCarBrake, byte(app, car + kCarBrakeAsked));
        return;
    }
    const reg32 brake = byte(app, car + kCarBrake);
    const reg32 steps = at(app, car + kCarHandling) + (brake >> 5) * 4;
    const i32 difference = i32(byte(app, car + kCarBrakeAsked)) - i32(brake);
    const i32 step = fp::toInteger<i32>(f32(app, steps + (difference >= 0 ? kHandlingBrakeUp : kHandlingBrakeDown)));
    putByte(app, car + kCarBrake, reg8(brake + stepTowards(difference, step, step)));
}

/* The gear asked (+0x506; first until the countdown is over, with the automatic box) put in: by
 * the automatic box, into reverse or neutral as asked, or forward into the gear the speed gives --
 * the clunk played, +0x50c whether it went down, the force feedback's kick (0x4714f0) with a
 * manual one.  False when the game stopped. */
bool putGearIn(App* app, CPU& cpu, reg32 car, reg32 pan)
{
    const bool automatic = hasAutomaticBox(app, car);
    if (automatic && i32(at(app, kRaceFrames)) < kCountdownFrames)
        putByte(app, car + kCarGearAsked, reg8(kFirstGear));
    const reg8 asked = byte(app, car + kCarGearAsked);
    const reg8 gear = byte(app, car + kCarGear);
    if (asked == gear)
        return true;
    const reg32 handling = at(app, car + kCarHandling);
    // The clunk for the player at this machine, or each on split screen.
    const auto knock = [&] {
        if (at(app, car + kCarPlayer) == gameSettings(app).localPlayer || gameSettings(app).playMode == 1)
            return knockGear(app, cpu, pan);
        return true;
    };
    const auto changeFrames = [&] { putHalf(app, car + kCarGearChangeFrames, half(app, handling + kHandlingChangeFrames)); };
    if (!automatic)
    {
        // The manual box: the gear asked, down or up, and the wheel's kick.
        if (!knock())
            return false;
        putByte(app, car + kCarShiftingDown, gear > asked && asked > kNeutral ? 1 : 0);
        putByte(app, car + kCarLastGear, gear);
        putByte(app, car + kCarGear, asked);
        changeFrames();
        return guest::direct<0x4714f0, controls::gearKick>(app, cpu, car).has_value();
    }
    if (asked < kFirstGear)
    {
        // Into reverse or neutral, as asked.
        if (!knock())
            return false;
        putByte(app, car + kCarShiftingDown, 0);
        putByte(app, car + kCarLastGear, gear);
        putByte(app, car + kCarGear, asked);
        changeFrames();
    }
    else if (asked == kFirstGear && gear < kFirstGear)
    {
        // Out of reverse or neutral: into the highest gear whose revs at the top are past the speed.
        if (!knock())
            return false;
        putByte(app, car + kCarLastGear, gear);
        reg32 chosen = kFirstGear;
        for (reg32 inGear = kFirstGear; i32(inGear) < i32(at(app, handling + kHandlingGears)); ++inGear)
        {
            const x86::Float topSpeed =
                fp::mul(i32At(app, handling + kHandlingTopRevs), f32(app, inGear * 4 + car + kCarRatiosByGear));
            if (compareBelow(cpu, topSpeed, f32(app, car + kCarVelocityAhead)))
                chosen = inGear;
        }
        putByte(app, car + kCarShiftingDown, 0);
        putByte(app, car + kCarGear, reg8(chosen));
        changeFrames();
    }
    return true;
}

/* The steering (+0x51c) eased towards what is asked (+0x518) by the handling's steps (+0x240 away,
 * +0x244 back to the middle, when the two are either side of it) or at once. */
void easeSteering(App* app, reg32 car)
{
    const reg32 asked = at(app, car + kCarSteeringAsked);
    const reg32 steering = at(app, car + kCarSteering);
    if (at(app, at(app, car + kCarDriver) + kDriverSteeringEased) == 0)
    {
        put(app, car + kCarSteering, asked);
        return;
    }
    const reg32 handling = at(app, car + kCarHandling);
    const bool back = (i32(asked) >= 0 && i32(steering) < 0) || (i32(asked) <= 0 && i32(steering) > 0);
    const i32 difference = i32(back ? 0u - steering : asked - steering);
    const i32 step = fp::toInteger<i32>(f32(app, handling + (back ? kHandlingSteerBack : kHandlingSteerAway)));
    put(app, car + kCarSteering, steering + reg32(stepTowards(difference, step, step)));
}

/* A pedal's byte by 255, 1 at most, to its field.  The x87's status word left as the compare does. */
void pedalRatio(App* app, CPU& cpu, reg32 car, reg32 pedal, reg32 ratio)
{
    constexpr reg32 kPedalScale = 0x53dcbc;
    const x86::Float value = fp::div(x86::Float(byte(app, car + pedal)), f32(app, kPedalScale));
    putFloat(app, car + ratio, float(compareAbove(cpu, 1.0, value) ? value : x86::Float(1.0)));
}

/* The inputs of car, each frame.  Into reverse and out (reverseOnItsOwn) first.  A player's car
 * that finished: let go, in neutral, slowing (its speed and spin by 0.98, 0x4e0050) while racing.
 * The computer's car, or the player's driven for him: computerDriver.  Else its pedals eased; a
 * gear change's frames counted down; the gear asked put in; the steering eased.  The throttle,
 * brake and steering to 0..1 at +0x5f0, +0x5ec and +0x5e8.  False when the game stopped. */
bool readInputs(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kSlowing = 0x3f7ae148;  // 0.98, as bits
    constexpr reg32 kSteeringScale = 0x53dcc0;
    const reg32 pan = gameSettings(app).playMode == 1 ? (at(app, car + kCarPlayer) != 0 ? 0xc000 : 0x3fff) : 0;
    reverseOnItsOwn(app, cpu, car);
    if (cpu.terminate)
        return false;
    if (!notFinished(app, car))
    {
        // Finished: let go, in neutral, slowing.
        put(app, car + kCarSteering, 0);
        putByte(app, car + kCarThrottle, 0);
        putByte(app, car + kCarShiftingDown, 0);
        putByte(app, car + kCarThrottleAsked, 0);
        putByte(app, car + kCarGearAsked, reg8(kNeutral));
        putByte(app, car + kCarGear, reg8(kNeutral));
        putByte(app, car + kCarClutchSlipping, 1);
        putByte(app, car + kCarBrake, 0xff);
        if (half(app, car + kCarRaceStage) == 0)
            for (const reg32 vector : {kCarSpeedVector, kCarSpinVector})
                if (!scaleVector(app, cpu, car + vector, kSlowing))
                    return false;
    }
    else
    {
        if (gameSettings(app).raceOptionComputerDrives != 0 || gameSettings(app).raceOptionLaneVotes != 0)
        {
            if (!driveByComputer(app, cpu, car))
                return false;
        }
        else
        {
            easePedals(app, car);
        }
        // A gear change's frames counted down.
        if (i16(half(app, car + kCarGearChangeFrames)) > 0)
            putHalf(app, car + kCarGearChangeFrames, reg16(half(app, car + kCarGearChangeFrames) - 1));
        if (at(app, kRaceUnderWay) != 0 && !putGearIn(app, cpu, car, pan))
            return false;
        easeSteering(app, car);
    }
    // Before the race the brake held; out of a state 2 no throttle.
    if (at(app, kRaceUnderWay) == 0)
        putByte(app, car + kCarBrake, 0xff);
    if (at(app, car + kCarThrottleOffState) == 2)
        putByte(app, car + kCarThrottle, 0);
    pedalRatio(app, cpu, car, kCarThrottle, kCarThrottleEased);
    pedalRatio(app, cpu, car, kCarBrake, kCarBrakeEased);
    // The steering's, by 128 (the steering one more).
    const reg32 steering = at(app, car + kCarSteering) + 1;
    const reg32 size = i32(steering) < 0 ? 0u - steering : steering;
    putFloat(app, car + kCarSteeringEased, float(fp::mul(x86::Float(i32(size)), f32(app, kSteeringScale))));
    return true;
}

/* 0x4a9bc4: readInputs for car eax, the car left in eax. */
void driverInputs(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!readInputs(app, cpu, car))
        return;
    cpu.eax = car;
    cpu.esp += 4;
}

/* The grip of car's wheels: its scale (+0x134, 0.75 to 1) at each, by the surface each is on
 * (wheelSurface, kSurfaceGrip's) as the track's kind (+0x160) has it, 1.41 times as a simulation's;
 * and the grip growing with the speed past 10 (+0x5c8..+0x5d4, the handling's +0x24c, 1.41 times
 * as a simulation's, the back wheels' 1.5 times), else as the handling has it.  False when the
 * game stopped.  The x87's status word left as its compare leaves it.  The scale and the speed
 * compare as bits as the game's do, a NaN's sign deciding. */
bool setWheelGrip(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kOne = 0x3f800000, kThreeQuarters = 0x3f400000, kTen = 0x41200000;
    constexpr reg32 kDoubleKind2First = 0x53dcd4, kDoubleKind2Second = 0x53dcdc, kDoubleKind1First = 0x53dce4,
                    kDoubleKind1Second = 0x53dcec, kKindOtherScale = 0x53dcf4, kSimulationGrip = 0x53dcf8,
                    kSimulationGrowth = 0x53dcfc, kBackWheelsGrowth = 0x53dd00, kSimulationBaseGrowth = 0x53dd04;
    reg32 scale = at(app, car + kCarGripScale);
    if (i32(scale) > i32(kOne))
        scale = kOne;
    else if (i32(scale) < i32(kThreeQuarters))
        scale = kThreeQuarters;
    for (const reg32 field : {kCarWheelGrip, kCarWheelGrip + 4, kCarWheelGrip + 0xc, kCarWheelGrip + 8})
        put(app, car + field, scale);

    // Each wheel by its surface.
    for (reg32 wheel = 0; wheel < 4; ++wheel)
    {
        const std::optional<reg32> surface = guest::direct<0x429b40, racestart::wheelSurfaceOf>(app, cpu, car, wheel);
        if (!surface)
            return false;
        const reg32 surfaceGrip = *surface * 4 + kSurfaceGrip;
        const reg32 grip = wheel * 4 + car + kCarWheelGrip;
        const reg32 kind = at(app, car + kCarTrackKind);
        if (kind == 2 || kind == 1)
        {
            const reg32 first = kind == 2 ? kDoubleKind2First : kDoubleKind1First;
            const reg32 second = kind == 2 ? kDoubleKind2Second : kDoubleKind1Second;
            const x86::Float scaled =
                fp::mul(fp::mul(fp::mul(f32(app, surfaceGrip), floatAt(scale)), f64(app, first)), f64(app, second));
            putFloat(app, grip, float(fp::add(scaled, f32(app, grip))));
        }
        else
        {
            const x86::Float scaled = fp::mul(fp::mul(f32(app, surfaceGrip), floatAt(scale)), f32(app, kKindOtherScale));
            putFloat(app, grip, float(fp::add(scaled, f32(app, grip))));
        }
        putFloat(app, grip, float(fp::mul(f32(app, surfaceGrip), f32(app, grip))));
        if (gameSettings(app).raceOptionSimulation != 0 || (std::uint8_t(gameSettings(app).raceFlags) & 1) != 0)
            putFloat(app, grip, float(fp::mul(f32(app, grip), f32(app, kSimulationGrip))));
    }

    // The grip with the speed, either way.  Only a value under 0 is negated: never a NaN.
    reg32 speed = at(app, car + kCarVelocityAhead);
    if (compareAbove(cpu, 0.0, f32(app, car + kCarVelocityAhead)))
        speed = floatBits(float(-f32(app, car + kCarVelocityAhead)));
    const reg32 handling = at(app, car + kCarHandling);
    // 1 + the speed by a growth, to a wheel's field.
    const auto grow = [&](reg32 field, float growth) {
        putFloat(app, car + field, float(fp::add(fp::mul(f32(app, car + kCarVelocityAhead), x86::Float(growth)), 1.0)));
    };
    // A field copied to another, through the x87.
    const auto copy = [&](reg32 from, reg32 to) { putFloat(app, car + to, float(f32(app, car + from))); };
    const auto sameAllRound = [&] {
        copy(kCarGripGrowth, kCarGripGrowth + 8);
        copy(kCarGripGrowth + 8, kCarGripGrowth + 0xc);
        copy(kCarGripGrowth + 0xc, kCarGripGrowth + 4);
    };
    if (i32(speed) > i32(kTen))
    {
        float growth = app->getMemory<float>(handling + kHandlingGripGrowth);
        if (gameSettings(app).raceOptionSimulation != 0)
            growth = float(fp::mul(x86::Float(growth), f32(app, kSimulationGrowth)));
        grow(kCarGripGrowth, growth);
        grow(kCarGripGrowth + 4, growth);
        growth = float(fp::mul(f32(app, handling + kHandlingGripGrowth), f32(app, kBackWheelsGrowth)));
        grow(kCarGripGrowth + 0xc, growth);
        grow(kCarGripGrowth + 8, growth);
    }
    else if (gameSettings(app).raceOptionSimulation != 0)
    {
        putFloat(app, car + kCarGripGrowth,
                 float(fp::add(fp::mul(f32(app, handling + kHandlingGripGrowth), f32(app, kSimulationBaseGrowth)), 1.0)));
        sameAllRound();
    }
    else
    {
        putFloat(app, car + kCarGripGrowth, float(fp::add(f32(app, handling + kHandlingGripGrowth), 1.0)));
        sameAllRound();
    }
    return true;
}

/* 0x4aab41: setWheelGrip for car eax, the car left in eax. */
void wheelGrip(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!setWheelGrip(app, cpu, car))
        return;
    cpu.eax = car;
    cpu.esp += 4;
}

/* The drag: the air's by the speed cubed (past the handling's +0x1c8 harder) by the car's +0x58c
 * (its driver's in a single race). */
float engineDrag(App* app, reg32 car)
{
    constexpr reg32 kRaceDragBits = 0x3e7aedcb;
    constexpr reg32 kDriverDragDivisor = 0x53dd08, kDriverDragScale = 0x53dd0c, kHarderDrag = 0x53dd10;
    const reg32 handling = at(app, car + kCarHandling);
    const GameSettings& settings = gameSettings(app);
    x86::Float scale = floatAt(kRaceDragBits);
    if (!(i32(settings.playMode) > 1 || settings.raceType == 1 || settings.raceType == 2))
    {
        const x86::Float driver = f32(app, at(app, car + kCarDriver) + kDriverDrag);
        scale = x86::Float(float(fp::mul(fp::div(fp::add(1.0, driver), f32(app, kDriverDragDivisor)), f32(app, kDriverDragScale))));
    }
    const x86::Float ahead = f32(app, car + kCarVelocityAhead);
    float drag = float(fp::mul(fp::mul(fp::mul(fp::mul(f32(app, car + kCarDrag), ahead), ahead), ahead), scale));
    const float speed = sizeOf(app->getMemory<float>(car + kCarVelocityAhead));
    if (x86::Float(speed) > f32(app, handling + kHandlingDragSpeed))
    {
        float past = float(fp::sub(x86::Float(speed), f32(app, handling + kHandlingDragSpeed)));
        past = float(fp::mul(x86::Float(past), x86::Float(past)));
        past = float(fp::mul(x86::Float(past), f32(app, kHarderDrag)));
        drag = float(fp::add(x86::Float(drag), x86::Float(past)));
    }
    return drag;
}

/* The revs aimed at: the top by the throttle, the idle at least, the top at most; nullopt when the
 * game stopped.  "Below" in the game's compare is so for a NaN too. */
std::optional<i32> revsAimedAt(App* app, CPU& cpu, reg32 car)
{
    const reg32 handling = at(app, car + kCarHandling);
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const x86::Float byThrottle = fp::mul(x86::Float(top), f32(app, car + kCarThrottleEased));
    const x86::Float idle = i32At(app, handling + kHandlingIdleRevs);
    const float aim = float(idle >= byThrottle ? idle : byThrottle);
    const std::optional<x86::Float> chopped = chop(app, cpu, x86::Float(aim));
    if (!chopped)
        return std::nullopt;
    const i32 revs = fp::toInteger<i32>(*chopped);
    return top > revs ? revs : top;
}

/* The engine running free, in neutral, changing gear or before the start: up 250 a frame towards
 * the aim, by the gear asked's step through a change down, by the handling's kind through a change
 * up, down 150 to the aim otherwise; the limiter off.  The revs at the idle at least. */
void engineRunsFree(App* app, reg32 car, i32 aim, bool overTop)
{
    constexpr i32 kFreeUp = 0xfa, kFreeDown = 0x96;
    constexpr reg32 kGearKindManyGears = 8;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 idle = i32(at(app, handling + kHandlingIdleRevs));
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const auto setRevs = [&](i32 revs) { put(app, car + kCarRevs, reg32(revs > idle ? revs : idle)); };
    if (half(app, car + kCarRaceStage) != 0)
        aim = i32(reg32(aim) + at(app, handling + 0x1c));
    const i32 revs = i32(at(app, car + kCarRevs));
    const i16 frames = i16(half(app, car + kCarGearChangeFrames));
    if (revs < aim && frames == 0)
    {
        const i32 up = i32(reg32(revs) + kFreeUp);
        setRevs(up > aim ? aim : up);
    }
    else if (frames > 0)
    {
        if (byte(app, car + kCarShiftingDown) != 0)
        {
            // Shifting down: the revs up by the gear asked's step, the clutch in.  A NaN's throttle counts as over 0.
            i32 stepped = revs;
            if (!(0.0 >= f32(app, car + kCarThrottleEased)))
            {
                const reg32 table = byte(app, car + kCarBrake) > 0x40 ? kHandlingDownStepsBraking : kHandlingDownSteps;
                stepped = i32(reg32(revs) + at(app, reg32(byte(app, car + kCarGearAsked)) * 4 + handling + table));
            }
            setRevs(stepped > top ? top : stepped);
        }
        else
        {
            // Shifting up: the revs by the handling's kind.
            reg32 step = at(app, handling + kHandlingKind) == kGearKindManyGears ? 0u - 0x19u
                         : i32(at(app, handling + kHandlingChangeFrames)) > 4    ? 0u - 0x32u
                                                                                 : 0u - 0x4bu;
            if (overTop)
                step <<= 1;
            setRevs(i32(reg32(revs) - step));
        }
    }
    else if (revs >= aim)
    {
        const i32 down = i32(reg32(revs) - kFreeDown);
        setRevs(down > aim ? down : aim);
    }
    put(app, car + kCarLimiterCutting, 0);
    put(app, car + kCarLimiterOn, 0);
}

/* The torque held in each gear: 10 in first, -6 at most in reverse (-6.0 by the double at
 * `reverseLimit`, a NaN's torque not over it), 8 in the others.  The first's and the others' limits
 * compare as bits as the game's do. */
float holdTorque(App* app, reg32 gear, float torque, reg32 reverseLimit)
{
    constexpr i32 kTenBits = 0x41200000, kEightBits = 0x41000000;
    if (gear == kFirstGear)
        return i32(floatBits(torque)) > kTenBits ? 10.0f : torque;
    if (gear == kReverse)
        return x86::Float(torque) > f64(app, reverseLimit) ? torque : -6.0f;
    return i32(floatBits(torque)) > kEightBits ? 8.0f : torque;
}

/* The torque's size by the gear's torque ratio, at the revs. */
float gearTorque(App* app, reg32 car, reg32 gear, i32 revs)
{
    return float(fp::mul(x86::Float(torqueAt(app, car, revs)), f32(app, gear * 4 + car + kCarTorqueRatios)));
}

/* Over the aim: the engine braking, the revs down to the wheels' -- the clutch slipping (+0x514
 * counting the wheelspin) where they would fall too low.  Gives the torque; nullopt when the game
 * stopped.  "Below" in the game's compare is so for a NaN too. */
std::optional<float> engineBraking(App* app, CPU& cpu, reg32 car, reg32 gear, float torque, i32 aim, i32 fromEngine, bool overTop)
{
    constexpr reg32 kBrakingFirst = 0x53dd24, kBrakingSecond = 0x53dd28, kSlippingScale = 0x53dd2c;
    constexpr i32 kWheelspinMost = 0x180;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const i32 idle = i32(at(app, handling + kHandlingIdleRevs));
    const reg32 ratio = gear * 4 + car + kCarRatiosByGear;
    torque = float(fp::mul(-x86::Float(torque), f32(app, handling + kHandlingEngineBraking)));
    const x86::Float ahead = f32(app, car + kCarVelocityAhead);
    if (gear > kNeutral && 0.0 > ahead)
        torque = sizeOf(torque);
    else if (gear < kNeutral && !(0.0 >= ahead))
        torque = float(-x86::Float(sizeOf(torque)));
    const std::optional<x86::Float> chopped =
        chop(app, cpu, fp::mul(fp::mul(f32(app, ratio), f32(app, kBrakingFirst)), f32(app, kBrakingSecond)));
    if (!chopped)
        return std::nullopt;
    const i32 most = fp::toInteger<i32>(*chopped);
    const i32 toWheels = i32(0u - reg32(fromEngine));
    const i32 revs = i32(at(app, car + kCarRevs) + reg32(toWheels > most ? most : toWheels));
    if (overTop)
        put(app, car + kCarRevs, reg32(top));
    else
    {
        const i32 aimed = revs > aim ? revs : aim;
        put(app, car + kCarRevs, reg32(aimed > idle ? aimed : idle));
    }
    // The top revs at the speed in the gear, by 1.15: the clutch slipping under them.
    const float slipping = float(fp::mul(fp::mul(x86::Float(top), f32(app, ratio)), f32(app, kSlippingScale)));
    const float speed = sizeOf(app->getMemory<float>(car + kCarVelocityAhead));
    if (x86::Float(speed) > x86::Float(slipping) && gear > kNeutral)
    {
        put(app, car + kCarLimiterOn, 1);
        const float topSpeed = float(fp::mul(x86::Float(top), f32(app, ratio)));
        torque = float(fp::mul(fp::div(x86::Float(topSpeed), x86::Float(speed)), x86::Float(torque)));
        putByte(app, car + kCarClutchSlipping, 1);
        // The wheelspin counted, 0x180 at most: by the track's kind.
        const reg32 kind = at(app, car + kCarTrackKind);
        const i32 spin = i32(at(app, car + kCarWheelspin) + (kind == 1 ? 3 : kind == 2 ? 4 : 1));
        put(app, car + kCarWheelspin, reg32(spin < kWheelspinMost ? spin : kWheelspinMost));
    }
    else if (byte(app, car + kCarClutchSlipping) == 0)
    {
        put(app, car + kCarWheelspin, 0);
    }
    return torque;
}

/* In gear, the box done: the revs the wheels give in it and the torque at them, held in each gear;
 * the limiter cutting the revs by gear (kRevsDrop) past a sixth over the wheels' in a low gear, or
 * when the wheels spin backwards; the engine braking over the aim; the revs following the wheels'
 * within 200 and the force by the throttle and the slip under it.  Below the idle the idle's torque.
 * Gives the torque; nullopt when the game stopped. */
std::optional<float> engineInGear(App* app, CPU& cpu, reg32 car, const float drag, i32 aim, i32 sixth, bool overTop)
{
    constexpr reg32 kReverseTorque = 0x53dd14, kIdleReverseTorque = 0x53dd3c, kWheelsStopped = 0x53dd1c, kSlipScale = 0x53dd30,
                    kSlipMost = 0x53dd34;
    constexpr i32 kReverseSpinBits = 0x3e99999a;
    constexpr i32 kNearAim = 0x7d, kNearTop = 0x12c, kLowAim = 0x7d0, kLowAimDrop = 0xc8, kFollow = 0xc8, kFollowOverBy = -300;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const i32 idle = i32(at(app, handling + kHandlingIdleRevs));
    const reg32 gear = byte(app, car + kCarGear);

    // The revs the wheels give in the gear, either way.
    const i32 wheels = i32(magnitude(reg32(fp::toInteger<i32>(
        fp::mul(f32(app, car + kCarVelocityAhead), f32(app, gear * 4 + handling + kHandlingGearRevs))))));
    // The torque at the top revs, held to the top, or at the revs, by the gear's.
    float torque = gearTorque(app, car, gear, overTop ? top : i32(at(app, car + kCarRevs)));
    torque = holdTorque(app, gear, torque, kReverseTorque);

    // How far the wheels' revs are from the aim, and from the engine's.
    i32 fromAim = i32(reg32(aim) - reg32(wheels));
    if (i32(magnitude(reg32(fromAim))) < kNearAim && i32(reg32(top) - kNearTop) > aim)
        fromAim = 0;
    const i32 fromEngine = i32(at(app, car + kCarRevs) - reg32(wheels));
    put(app, car + kCarLimiterOn, 0);
    put(app, car + kCarLimiterCutting, 0);

    // The limiter: over the top's sixth in a low gear without a change under way, or the wheels
    // spinning backwards on the throttle.
    const bool throttleOn = byte(app, car + kCarThrottle) > 8;
    bool limit = false;
    if (fromEngine > sixth && half(app, car + kCarGearChangeFrames) == 0 && gear <= 4)
        limit = true;
    else if (gear > kNeutral && !(f32(app, car + kCarWheelSpeed) >= f64(app, kWheelsStopped)) && throttleOn)
        limit = true;
    else if (gear == kReverse && i32(at(app, car + kCarWheelSpeed)) > kReverseSpinBits)
        limit = throttleOn;
    if (limit)
    {
        put(app, car + kCarLimiterOn, 1);
        const i32 drop = aim < kLowAim ? kLowAimDrop : i32(at(app, gear * 4 + kRevsDrop));
        put(app, car + kCarRevs, at(app, car + kCarRevs) - reg32(fromEngine > drop ? drop : fromEngine));
    }
    else if (fromAim < 0)
    {
        const std::optional<float> braking = engineBraking(app, cpu, car, gear, torque, aim, fromEngine, overTop);
        if (!braking)
            return std::nullopt;
        torque = *braking;
    }
    else if (fromAim == 0)
    {
        // At the aim: the revs the wheels', the force the drag's.
        put(app, car + kCarRevs, reg32(wheels > idle ? wheels : idle));
        torque = drag;
    }
    else
    {
        // Under the aim: the revs towards the wheels' by 200 a frame, the force by the throttle
        // and the slip (1.5 times at most).
        reg32 revs = at(app, car + kCarRevs);
        if (fromEngine > kFollow)
            revs -= kFollow;
        else if (fromEngine < kFollowOverBy)
            revs += kFollow;
        else
            revs = reg32(wheels > idle ? wheels : idle);
        const i32 held = i32(revs) > aim ? aim : i32(revs);
        put(app, car + kCarRevs, reg32(held > idle ? held : idle));
        torque = float(fp::mul(x86::Float(torque), f32(app, car + kCarThrottleEased)));
        const float slip = sizeOf(app->getMemory<float>(car + kCarSlip));
        const float scale = float(fp::add(fp::mul(x86::Float(slip), f32(app, kSlipScale)), 1.0));
        const float most = f32(app, kSlipMost) > x86::Float(scale) ? scale : 1.5f;
        torque = float(fp::mul(x86::Float(torque), x86::Float(most)));
    }

    // Under the idle: the idle's torque.
    if (i32(at(app, car + kCarRevs)) < idle)
    {
        put(app, car + kCarLimiterOn, 0);
        torque = gearTorque(app, car, gear, idle);
        put(app, car + kCarRevs, reg32(idle));
        torque = holdTorque(app, gear, torque, kIdleReverseTorque);
    }
    return torque;
}

/* Car's engine for the frame, and its force at the wheels; nullopt when the game stopped.  The
 * revs held to the top (+0x50d, the clutch slipping, past a third over); the drag, the revs aimed
 * at; in neutral, changing gear or before the start the engine runs free with no force but the
 * drag, in gear (the automatic box's change first, shiftAutomatically, or barTopGears)
 * engineInGear.  The force: the torque, less the drag the way the car goes (half of it backwards
 * before the start).  The compares before the last are plain: the last one leaves the x87's
 * status word, whatever the others left; each answers as the game's does for a NaN. */
std::optional<float> forceAtWheels(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kStartReverseDrag = 0x53dd44, kHalfDrag = 0x53dd48;
    const reg32 handling = at(app, car + kCarHandling);
    const i32 top = i32(at(app, handling + kHandlingTopRevs));
    const i32 sixth = top / 6;
    bool overTop = false;
    if (i32(at(app, car + kCarRevs)) > top)
    {
        overTop = true;
        if (i32(reg32(top / 3) + reg32(top)) < i32(at(app, car + kCarRevs)))
            putByte(app, car + kCarClutchSlipping, 1);
        put(app, car + kCarRevs, reg32(top));
    }
    const float drag = engineDrag(app, car);
    const std::optional<i32> aim = revsAimedAt(app, cpu, car);
    if (!aim)
        return std::nullopt;

    float torque = 0.0f;
    if (byte(app, car + kCarGear) == kNeutral || i16(half(app, car + kCarGearChangeFrames)) > 0
        || half(app, car + kCarRaceStage) != 0)
    {
        engineRunsFree(app, car, *aim, overTop);
    }
    else
    {
        if (hasAutomaticBox(app, car))
        {
            shiftAutomatically(app, cpu, car);
            if (cpu.terminate)
                return std::nullopt;
        }
        else
        {
            barTopGears(app, car);
        }
        const std::optional<float> inGear = engineInGear(app, cpu, car, drag, *aim, sixth, overTop);
        if (!inGear)
            return std::nullopt;
        torque = *inGear;
    }

    // The force: the torque less the drag the way the car goes.
    const bool backwards = compareAbove(cpu, 0.0, f32(app, car + kCarVelocityAhead));
    if (half(app, car + kCarRaceStage) != 0)
        return backwards ? drag : float(fp::mul(-x86::Float(drag), f32(app, kStartReverseDrag)));
    if (backwards)
        return float(fp::add(fp::mul(x86::Float(drag), f32(app, kHalfDrag)), x86::Float(torque)));
    return float(fp::sub(x86::Float(torque), x86::Float(drag)));
}

/* 0x4aaed1: forceAtWheels for car eax, onto the x87's stack and its bits in eax. */
void engineForce(App* app, CPU& cpu)
{
    const std::optional<float> force = forceAtWheels(app, cpu, cpu.eax);
    if (!force)
        return;
    cpu.eax = floatBits(*force);
    cpu.fpu.push(x86::Float(*force));
    cpu.esp += 4;
}

}  // namespace drivetrain
}  // namespace nfs3hp
