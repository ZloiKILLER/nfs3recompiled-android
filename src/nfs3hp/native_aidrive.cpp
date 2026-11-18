#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_carstate.h"
#include "native_frontend_sima.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_pursuit.h"
#include "native_racestart.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "port_conv.h"
#include "port_hud.h"
#include "port_sound_calls.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <type_traits>

/* The opponents' and the cops' driving, the middle of nfs3.exe's AI module
 * (0x404d30 to 0x408acb): the cops' Hot Pursuit -- their places at the start,
 * the racers they watch and clock, the siren and the roadblock a player's cop
 * calls, the pursuit started, kept up, ended and the bust, the state machine
 * that drives a cop (copDriver) -- the extra cars' (traffic and cops) coming
 * and going, the AI's tuning constants, and the computer's car's own physics:
 * its reverse timer and the stuck tests that start it, its engine's revs, its
 * pull and brake along the road, the way it heads for (its target, a slice
 * ahead, its lane), its steering on the road, backwards and off it, the step
 * that integrates its forces, the coast after the finish, the ride along with
 * the car it follows, and the per-frame step of every car the computer
 * drives (aiStep, which the car's physics calls, physics::carPhysics, where
 * its +0x200 has 2).  The game's code calls each function here by name.
 *
 * What these read of a car's record (0x9ac bytes, 0x5e1120 on): +0x1c its
 * slice on the track, +0x88 whether it is a ghost driven by its target, +0x89,
 * +0x8a whether it is held, +0x98 its place, +0xa4 its speed (world), +0xb8
 * its grip, +0xc0 its axes (a 3x3 matrix, the nose +0xd8), +0xe4 its spin,
 * +0x10c how upright it stands, +0x124 the road's axes under it, +0x160 the
 * track's kind and +0x18c its surface (0x5523d8's grip table), +0x164 the
 * race's stage (0 racing), +0x168, +0x1f0 its player, +0x200 its kind (1 a
 * racer, 2 the computer drives it, 4 a human's, 8, 0x10 traffic, 0x20 a cop),
 * +0x2bc its place in the race, +0x2c4 its state (3 finished, 4 busted),
 * +0x2e4..+0x2fc the step's acceleration and spin's change, +0x4e0..+0x4e8
 * the force in its axes, +0x4ec..+0x500 its speed and spin in its axes, the
 * driver's inputs +0x508.. (+0x509 the brake, +0x50b the gear, +0x50e the
 * siren's button, +0x512 the roadblock's), +0x528 its performance record
 * (0x5eabe0 + i * 0x4ac: gears, revs, the pull's table +0x2ec), +0x590 the
 * engine's revs, +0x5c0/+0x5c4 its skid, +0x620/+0x621 its AI flags, +0x624
 * the car it chases (a cop's) or follows, +0x628 its pursuit's record, +0x638
 * its lane, +0x64c the gear's change timer, +0x650 traffic's life, +0x658 its
 * reverses, +0x65c, +0x6d8 the player a traffic car is near, +0x6e0 a cop's
 * state, +0x6ec its timer, +0x6f0 how far over the limit a racer was clocked,
 * +0x710 a cop's flags (0x10 the siren, 0x40 the bust's grace), +0x714 its
 * roadblock, +0x720, +0x730 the racer nearest a cop, +0x734 the siren's
 * sound, +0x7b0 the speed it wants, +0x7b4 the lane's offset it wants, +0x7b8
 * the offset it has, +0x7bc its pull's share, +0x7c0 its speed along the road
 * (signed), +0x7c4 its size, +0x7c8 the offset it eases to, +0x7d0/+0x7d4 its
 * reverse's end and whether it reverses, +0x7d8 how long it has been stuck.
 *
 * Each is a typed function under a name of its own (the car, the cop, the slice as
 * parameters, C++ locals, a result), with a thunk under the game's name for
 * the callers still on registers and for the calls by address: it takes the registers,
 * calls the core and leaves eax, the x87's stack and the registers the code
 * pushed as the game's code does -- eax often what a core says the code leaves
 * there (a compare's status word in ax, a callee's eax), which no caller reads.
 * The x87's arithmetic is fp's, in the code's order of operands, a float kept
 * where the code stores one; a compare answers as sahf and the jump read the
 * status word (below, belowOrEqual, above: a NaN below and equal).  A frame
 * whose address a callee is handed stays on the guest's stack at the code's
 * depth (GuestFrame, atDepth), and so does a core whose own callee is.  The two
 * switches go by the targets nfs3.exe's tables hold (0x404d14, 0x405714).
 * The other typed files' functions are called as C++ through guest::direct,
 * but where what a thunk here leaves is what the callee leaves in a register
 * beyond what its header gives.  Every other function of the game's is called
 * by address, its arguments in registers and on the guest's stack as the code
 * hands them, the two of the AI that the port has as decompiled C++ (0x4064f0,
 * 0x4070c0) among them. */
namespace nfs3hp
{
namespace aidrive
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
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;
using i32 = std::int32_t;

// The race's: the players on this machine (1 a split screen), the second
// player's index, the race run backwards, the race's frame counter, whether
// the cars move yet, a Hot Pursuit option, the track's kind of AI, the
// traffic's lane voting on.
constexpr reg32 kRaceFrame = 0x7d3684;
constexpr reg32 kCarsMove = 0x7d3680;
// The cars: the cops (and their count), the racers, the humans' cars, the
// first car; the track's slices and their count; the roadblocks (0x50 each);
// the surfaces' grips (by surface and track's kind); the extra cars on the
// road.
constexpr reg32 kCops = 0x5efc08, kCopCount = 0x553078, kRacers = 0x5efc48, kRacerCount = 0x5efda0;
constexpr reg32 kHumans = 0x5efa48, kHumanCount = 0x5efda4, kFirstCar = 0x5efac8;
constexpr reg32 kSlices = 0x5dd948, kSliceCount = 0x5dd940, kRoadblocks = 0x7dd850, kSurfaceGrips = 0x5523d8;
constexpr reg32 kExtraCars = 0x56ebc0, kBustSpeed = 0x56ebb0;
// The game's random numbers (seed, product, multiplier).
constexpr reg32 kSeed = 0x554f10, kProduct = 0x554f14, kMultiplier = 0x554f18;
// The AI's step: its length (1/32), the substeps, the heading's error (aiHeading's), the
// speed's damping, the force and turn handed to the step (three floats).
constexpr reg32 kStep = 0x56ec58, kSubsteps = 0x56ec78, kHeadingError = 0x56ec60, kDamping = 0x56ec68;
constexpr reg32 kPush = 0x56ec6c, kPushY = 0x56ec70, kPushZ = 0x56ec74;
// What is called by address: the cops' sounds, the lane votes (0x40de30, its thunk's
// NFS_CAR_TRACE record); and of the typed files' those whose registers a thunk here leaves,
// which their headers do not give: the car at rest settled, the dot product where its eax is read
// after it.  The menu files' -- the opponents' pull and heading, the distance of two cars, a matrix
// transposed, the sirens, a turn skipped (native_frontend_sim*.cpp) -- are called as C++ (feCore),
// the eax they leave put back; so are chop and the fill (guest::direct).
constexpr reg32 kChop = 0x4dfd56;

// A car's fields, as the header has them.
constexpr reg32 kCarSlice = 0x1c, kCarNose = 0xd8, kCarMoving = 0x168, kCarKind = 0x200,
                kCarAiFlags = 0x620, kCarReverses = 0x658, kCarCopFlags = 0x710, kCarSirenSound = 0x734,
                kCarSirenButton = 0x50e, kCarLaneWanted = 0x7b4, kCarLaneOffset = 0x7b8,
                kCarSpeedSize = 0x7c4, kCarEasedOffset = 0x7c8, kCarReverseEnd = 0x7d0, kCarReversing = 0x7d4;
// +0x200's kinds: traffic, a cop.
constexpr reg8 kKindTraffic = 0x10, kKindCop = 0x20;
// +0x620's: the siren's button held since its press, the reverse timed.
constexpr reg8 kAiSirenHeld = 0x10, kAiTimedReverse = 0x40;
// +0x620's for traffic: placed on the road; counted among the extra cars on it.
constexpr reg8 kAiOnRoad = 2, kAiOnRoadExtra = 4;
// A car's: held (+0x89), its speed (+0xa4), its life as traffic (+0x650), the human it is near
// (+0x6d8), the speed it wants (+0x7b0).
constexpr reg32 kCarHeld = 0x89, kCarSpeed = 0xa4, kCarLife = 0x650, kCarNearHuman = 0x6d8, kCarSpeedWanted = 0x7b0;
// +0x621's: going against the race's way.
constexpr reg8 kAi2Against = 0x10;
// +0x710's: the siren on, the bust's grace.
constexpr reg8 kCopSiren = 0x10, kCopGrace = 0x40;
// +0x200's other kinds: a racer, the computer drives it, a human's.
constexpr reg8 kKindRacer = 1, kKindComputer = 2, kKindHuman = 4;
// +0x621's: put back on the road as its pursuit is dropped.
constexpr reg8 kAi2PutBack = 0x40;
// A car's: its place (+0x98), how upright (+0x10c), the race's stage (+0x164), its state
// (+0x2c4), its other AI flags (+0x621), the car it chases (+0x624), its pursuit's record (+0x628),
// a cop's state (+0x6e0), how far over the limit a racer was clocked (+0x6f0), the bust's grace's
// end (+0x720), the racer nearest a cop (+0x730).
constexpr reg32 kCarPlace = 0x98, kCarUpright = 0x10c, kCarStage = 0x164, kCarState = 0x2c4, kCarAiFlags2 = 0x621,
                kCarChased = 0x624, kCarPursuit = 0x628, kCarCopState = 0x6e0, kCarClocked = 0x6f0,
                kCarGraceEnd = 0x720, kCarNearestRacer = 0x730;
// +0x2c4's: finished, busted.
constexpr reg32 kStateFinished = 3, kStateBusted = 4;
// +0x6e0's: dropped from the pursuit; the chased car at its end, a computer's and a human's.
constexpr reg32 kCopStateDropped = 2, kCopStateComputerEnd = 6, kCopStateHumanEnd = 7;
// A pursuit's record: its stage (+0x1c, 2 the bust) and its frames' end (+0x28).
constexpr reg32 kPursuitStage = 0x1c, kPursuitEnd = 0x28, kPursuitBusting = 2;
// The pursuit's: joined (how 2: a chase); given up is airace::giveUpPursuitOf (0x401a80).
constexpr reg32 kJoinPursuit = 0x4018b0, kJoinChase = 2;
// 2.0 as a float's bits: how far over the limit a racer is chased.
constexpr i32 kTwoOverBits = 0x40000000;
// The bust's reach (a double, 7), how upright the cop stands (a double), the chase's reach (a
// double, 300); the grace's and the bust's frames.
constexpr reg32 kBustReach = 0x5361b4, kUprightEnough = 0x5361bc, kChaseReach = 0x5361c4;
constexpr reg32 kGraceFrames = 0x18, kBustFrames = 0x280;

// A slice's record (36 bytes at [kSlices]): its point (16.16 integers, +0) and its right (signed
// bytes, +0x18..+0x1a).
constexpr reg32 kSliceSize = 36, kSliceRight = 0x18;

// A car's: its forward speed (+0x4f4), its brake (+0x509: 0xff on), its performance (+0x528).
constexpr reg32 kCarForwardSpeed = 0x4f4, kCarBrake = 0x509, kCarPerformance = 0x528;

/* fcomp's answers as sahf and the jump read them: a NaN (unordered) sets c0 and c3, so it is
 * below and equal; written so in C++ (not a < b), they agree with the x87 on every value. */
bool below(const Float& a, const Float& b) { return !(a >= b); }
bool belowOrEqual(const Float& a, const Float& b) { return !(a > b); }
bool above(const Float& a, const Float& b) { return a > b; }

/* fnstsw ax after fcom: eax with its low word the compare's status word (c3, c2 and c0, as the
 * game's arithmetic has it, fp::compare). */
reg32 statusInto(reg32 eax, const Float& a, const Float& b)
{
    return (eax & 0xffff0000) | fp::compare(a, b);
}

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.  A reg32 result is the eax the function leaves: eax set to it. */
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

// The game's floats and doubles, as the x87 loads them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

/* 0x4e01f0 (vertices::dotProduct): the dot product of the vectors at a and b; nullopt when the
 * game stopped. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* 0x4e01f0, where the eax and edx it leaves are read after it (it leaves them as it was handed
 * them). */
struct DotLeft
{
    Float product;
    reg32 eax;  // the eax it leaves
    reg32 edx;  // and the edx
};

std::optional<DotLeft> dotLeavingEax(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const std::optional<Float> product = dot(app, cpu, a, b);
    if (!product)
        return std::nullopt;
    return DotLeft{*product, a, b};
}

/* 0x4965b0 (sima's): the distance from the place at a to the place at d; the eax it leaves to
 * `eax`, where asked. */
std::optional<Float> distance(App* app, CPU& cpu, reg32 a, reg32 d, reg32* eax = nullptr)
{
    const std::optional<frontend::sima::FloatLeft> apart =
        feCore<0x4965b0, frontend::sima::quickDistanceOf>(app, cpu, a, d);
    if (!apart)
        return std::nullopt;
    if (eax)
        *eax = apart->eax;
    return Float(apart->value);
}

/* The distance's size the code's way: fldz; fcompp; jae -- the distance worked out again either
 * way, negated where the first was not over 0 (a NaN is); the eax the second leaves to `eax`,
 * where asked. */
std::optional<Float> distanceApart(App* app, CPU& cpu, reg32 a, reg32 d, reg32* eax = nullptr)
{
    const auto first = distance(app, cpu, a, d);
    if (!first)
        return std::nullopt;
    const bool over = below(Float(0.0), *first);
    const auto again = distance(app, cpu, a, d, eax);
    if (!again)
        return std::nullopt;
    return over ? *again : -*again;
}

/* A float's bits. */
reg32 floatBits(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* Three dwords from `from` to `to`, as movsd thrice copies them (the direction flag clear). */
void copy3(App* app, reg32 from, reg32 to)
{
    for (reg32 i = 0; i < 12; i += 4)
        put(app, to + i, at(app, from + i));
}

/* Room on the guest's stack for what a callee is handed the address of: the game's frame
 * (`pushed` bytes of registers under esp on entry, then `bytes`), esp under it while the core
 * runs and back after (as native_collide.cpp's). */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu(cpu), esp(cpu.esp), base(cpu.esp - pushed)
    {
        cpu.esp = base - bytes;
    }
    ~GuestFrame() { cpu.esp = esp; }
    reg32 operator-(reg32 offset) const { return base - offset; }

private:
    CPU& cpu;
    const reg32 esp;
    const reg32 base;
};

/* A thunk's run of its core: every register but eax as the caller had it after it (the game's
 * code pops what it pushed; its other registers are its arguments, which no caller reads), eax
 * what the core says the code leaves there -- a core that answers only whether the game went on
 * leaves eax as it is -- and the return address and `popped` bytes of the caller's words popped.
 * False when the game stopped. */
template <typename Core>
bool asThunk(CPU& cpu, x86::Args& args, Core&& core, reg32 popped = 0)
{
    const reg32 esp = cpu.esp;
    std::optional<reg32> left;
    {
        const auto result = core();
        if constexpr (std::is_same_v<std::decay_t<decltype(result)>, bool>)
        {
            if (!result)
                return false;
        }
        else
        {
            if (!result)
                return false;
            left = reg32(*result);
        }
    }
    cpu.esp = esp;
    if (left)
        args.eax = *left;
    cpu.esp += 4 + popped;
    return true;
}

/* 0x4e0000 (vertices::offsetVectorList<false>) and 0x4dffb0 (<true>): the vector at a less (or
 * plus) the one at b, to `to` (one vector).  False when the game stopped. */
bool subtractVectors(App* app, CPU& cpu, reg32 a, reg32 b, reg32 to)
{
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, a, b, to);
    return !cpu.terminate;
}

bool addVectors(App* app, CPU& cpu, reg32 a, reg32 b, reg32 to)
{
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, a, b, to);
    return !cpu.terminate;
}

/* 0x4e06a0 (vertices::vectorLength): the length of the vector at v; nullopt when the game
 * stopped. */
std::optional<Float> length(App* app, CPU& cpu, reg32 v)
{
    const double size = guest::direct<0x4e06a0, vertices::vectorLength>(app, cpu, v);
    if (cpu.terminate)
        return std::nullopt;
    return Float(size);
}

/* 0x40e160 (aiplan::nodeAhead): the slice `count` on from car's, nullopt when the game stopped. */
std::optional<reg32> sliceAhead(App* app, CPU& cpu, reg32 car, reg32 count)
{
    const reg32 slice = guest::direct<0x40e160, aiplan::nodeAhead>(app, cpu, car, count);
    if (cpu.terminate)
        return std::nullopt;
    return slice;
}

// The look ahead by the pace (stdcall: 1.7 as a float's bits, 4 to 20 slices), the slices where the
// track's kind 0 takes it as it is, the curve past which the road no longer keeps straight (a
// double, 0.7).
constexpr reg32 kPaceScaleBits = 0x3fd9999a, kPaceLeast = 4, kPaceMost = 0x14, kStraightCurve = 0x53628c;
constexpr i32 kPaceFirstSlice = 0x225, kPaceLastSlice = 0x235;

/* 0x40e1b0 (aiplan::nodeAheadBySpeed): the slice car's pace gives, 1.7 by it, 4 to 20 on.
 * nullopt when the game stopped. */
std::optional<reg32> sliceByPace(App* app, CPU& cpu, reg32 car)
{
    float scale;
    std::memcpy(&scale, &kPaceScaleBits, sizeof scale);
    const reg32 slice =
        guest::direct<0x40e1b0, aiplan::nodeAheadBySpeed>(app, cpu, car, scale, i32(kPaceLeast), i32(kPaceMost));
    if (cpu.terminate)
        return std::nullopt;
    return slice;
}

/* 0x40ef70 (aiplan::directionDot): the road's curve at slice `at` against slice `from`; nullopt
 * when the game stopped. */
std::optional<Float> sliceCurve(App* app, CPU& cpu, reg32 from, reg32 to)
{
    const Float curve = guest::direct<0x40ef70, aiplan::directionDot>(app, cpu, from, to);
    if (cpu.terminate)
        return std::nullopt;
    return curve;
}

/* 0x4e0050 (vertices::scaleVectorsBy): the vector at `from` scaled by `by` to `to` (one vector).
 * False when the game stopped. */
bool scaleVector(App* app, CPU& cpu, reg32 from, float by, reg32 to)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, from, by, to);
    return !cpu.terminate;
}

// A car's: its axes (+0xc0, nine floats), the road's axes under it (+0x124, across first).
constexpr reg32 kCarAxes = 0xc0, kCarRoadAxes = 0x124;
// A car's +8.
constexpr reg32 kCarEight = 8;

/* Whether car is a racer ([+0x200] 1) in `state` ([+0x2c4]: 3 finished, 4 busted). */
bool racerIn(App* app, reg32 car, reg32 state)
{
    return (byte(app, car + kCarKind) & kKindRacer) != 0 && at(app, car + kCarState) == state;
}

/* 0x40e120 (aiplan::nodesBetween): the gap from slice `from` to slice `to`, nullopt when the game
 * stopped. */
std::optional<i32> sliceGap(App* app, CPU& cpu, reg32 from, reg32 to)
{
    const i32 gap = guest::direct<0x40e120, aiplan::nodesBetween>(app, cpu, from, to);
    if (cpu.terminate)
        return std::nullopt;
    return gap;
}

// The slices within which a car counts as near.
constexpr i32 kNearSlices = 0x32;

/* 0x4dfd56 (__CHP): `value` chopped to an integer, towards 0; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<Float> chop(App* app, CPU& cpu, const Float& value)
{
    return Float(guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value)));
}

/* 0x4064f0 (sima's): an opponent's pull for car; nullopt when the game stopped. */
std::optional<Float> pullOf(App* app, CPU& cpu, reg32 car)
{
    const std::optional<frontend::sima::FloatLeft> pull = feCore<0x4064f0, frontend::sima::enginePullOf>(app, cpu, car);
    if (!pull)
        return std::nullopt;
    return Float(pull->value);
}

}  // namespace

x86::reg32 copStartPlace(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 callRoadblock(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sirenSwitch(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 watchRacers(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 jumpBehindLeader(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 clockRacers(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 startPursuit(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 bustCheck(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 pursuitStep(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 dropPursuit(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copDriver(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nearCar(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nearHuman(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 humanNearSlice(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 trafficUnwatched(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 trafficStart(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 trafficStop(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 extraCarStep(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setDrivingConstants(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 drivingNothing(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setReverse(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 reverseCheck(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 reverseWhenSlow(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 reverseWhenTipped(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 engineRevs(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 brakeForce(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 speedAlongRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 laneError(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 glideAlongRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lookAheadSlice(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 sidewaysToRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 towardTarget(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 towardTargetAhead(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 towardSlice(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 towardSliceSlowing(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 towardLane(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 wayToGo(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 integrate(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 coastAfterFinish(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 steerOnRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 steerReversing(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 steerOffRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rideWithTarget(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 aiPhysics(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 easeLane(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 aiStep(App* app, x86::CPU& cpu, x86::Args& args);

// The start's slices behind the start: two, three for the fifth cop on.  A slice's record's
// lanes (+0x13, the low nibble) and its right width (+0x1b).  The place's across by the width
// (<< 15).  Its up and ahead (floats, fistp'd: 0x8000, 98304).  The cops' places by i % 4 (the
// code's table of targets).
constexpr reg32 kSliceLanes = 0x13, kSliceRightWidth = 0x1b;
constexpr reg32 kStartAhead = 0x5361ac, kStartUp = 0x5361b0, kStartTable = 0x404d14;
constexpr reg32 kStartAll = 0x404de6, kStartMirrored = 0x404dea, kStartHalf = 0x404df4;
constexpr reg32 kCopStartPushed = 0x10, kCopStartFrame = 0x14;

/* 0x42bc70 (racestart::gridPlaceOf): the grid's place `index`, its slice to `slice` and its place
 * to `place`.  False when the game stopped. */
bool gridPlace(App* app, CPU& cpu, reg32 index, reg32 slice, reg32 place)
{
    guest::direct<0x42bc70, racestart::gridPlaceOf>(app, cpu, index, slice, place);
    return !cpu.terminate;
}

/* The up's integer of a cop's start place, what placeCopAtStart's code leaves in eax. */
reg32 startUp(App* app) { return reg32(fp::toInteger<i32>(f32(app, kStartUp))); }

/* 0x404d30: cop's start (Hot Pursuit): found among the cops ([kCops], [kCopCount]) at index i,
 * its slice ([+0x1c]) is the start's (0, or the grid's first place's run backwards, worked out in
 * the code's frame), and the slice two behind it (three for the fifth cop on) goes to `sliceOut`;
 * its place `placeOut`: across by the slice's right width times its lanes, all of it, mirrored,
 * half of it or half of it mirrored by i % 4 (the table at 0x404d14); up 0x8000 and ahead 98304
 * whether found or not.  False when the game stopped (the thunk leaves eax the up's integer). */
bool placeCopAtStart(App* app, CPU& cpu, reg32 cop, reg32 sliceOut, reg32 placeOut)
{
    const GuestFrame frame(cpu, kCopStartPushed, kCopStartFrame);
    put(app, placeOut, 0);
    for (i32 i = 0; i < i32(at(app, kCopCount)); ++i)
    {
        if (at(app, kCops + reg32(i) * 4) != cop)
            continue;
        put(app, cop + kCarSlice, 0);
        if (gameSettings(app).backwards != 0)
        {
            if (!gridPlace(app, cpu, 0, frame - 8, frame - 0x14))
                return false;
            put(app, cop + kCarSlice, at(app, frame - 8));
        }
        else
            put(app, cop + kCarSlice, 0);
        const auto slice = sliceAhead(app, cpu, cop, i >= 4 ? reg32(-3) : reg32(-2));
        if (!slice)
            return false;
        put(app, sliceOut, *slice);
        const reg32 record = at(app, kSlices) + *slice * kSliceSize;
        const reg32 across = reg32(i32(reg32(byte(app, record + kSliceRightWidth)) << 15)
                                   * i32(byte(app, record + kSliceLanes) & 0xf));
        const reg32 target = at(app, kStartTable + reg32(i % 4) * 4);
        const auto half = [](reg32 value) { return reg32(i32(value - reg32(i32(value) >> 31)) >> 1); };
        if (target == kStartAll)
            put(app, placeOut, across);
        else if (target == kStartMirrored)
        {
            put(app, placeOut, across);
            put(app, placeOut, reg32(0) - across);
        }
        else if (target == kStartHalf)
            put(app, placeOut, half(across));
        else
            put(app, placeOut, half(reg32(0) - across));
        break;
    }
    const reg32 ahead = reg32(fp::toInteger<i32>(f32(app, kStartAhead)));
    put(app, placeOut + 8, ahead);
    put(app, placeOut + 4, startUp(app));
    return true;
}

/* Cop eax, the slice to [edx], the place to [ebx]. */
x86::reg32 copStartPlace(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax, slice = args.edx, place = args.ebx] {
        if (!placeCopAtStart(app, cpu, cop, slice, place))
            return false;
        args.eax = startUp(app);
        return true;
    });
    return args.eax;
}

// A player's cop's roadblock button (+0x512) and its press held (+0x513), its player (+0x1f0),
// its lane (+0x638), its roadblock (+0x714).  A roadblock's record (0x50 bytes at kRoadblocks):
// up (+0), its end (+0x4c).  Its frames up.  The cops' sounds: down, still up, set up.
constexpr reg32 kCarRoadblockButton = 0x512, kCarRoadblockHeld = 0x513, kCarPlayer = 0x1f0, kCarLane = 0x638,
                kCarRoadblock = 0x714;
constexpr reg32 kRoadblockSize = 0x50, kRoadblockEnd = 0x4c, kRoadblockFrames = 0xc0;
constexpr reg32 kSoundRoadblockDown = 0x11, kSoundRoadblockUp = 0x12, kSoundRoadblockSet = 0x10;
// The roadblock taken down and set up (a siren off and on, 0x4c9f50 and 0x4c9dd0, simc's); the
// radio told its side (pursuit::radioStoppedOf, 0x40b9c0).

/* 0x404e40: a player's cop calls a roadblock ([+0x512], its button, once a press: [+0x513]).  Its
 * roadblock ([kRoadblocks] + 0x50 * [+0x714]) up already: taken down if its time is past
 * ([+0x4c], 0x4c9f50), and the cops' sound 0x11 or, still up, 0x12; else set up at its slice in
 * its lane for 0xc0 frames (0x4c9dd0), 0x40b9c0 told the side (0 to 2 by its lane, the other way
 * round in a race run backwards), and the sound 0x10 -- each sound only for this machine's
 * player, or when one player plays.  What the code leaves in eax; nullopt when the game stopped. */
std::optional<reg32> callRoadblockFor(App* app, CPU& cpu, reg32 cop)
{
    const reg8 button = byte(app, cop + kCarRoadblockButton);
    if (button == 0)
    {
        putByte(app, cop + kCarRoadblockHeld, 0);
        return cop & 0xffff00ff;
    }
    if (byte(app, cop + kCarRoadblockHeld) != 0)
        return (cop & 0xffff00ff) | reg32(button) << 8;
    const reg32 index = at(app, cop + kCarRoadblock);
    const reg32 block = index * kRoadblockSize;
    reg32 left = block;
    reg32 sound = kSoundRoadblockUp;
    if (at(app, kRoadblocks + block) != 0)
    {
        if (i32(at(app, kRaceFrame)) >= i32(at(app, kRoadblocks + block + kRoadblockEnd)))
        {
            const auto down = feCore<0x4c9f50, frontend::simc::sirenOff>(app, cpu, index);
            if (!down)
                return std::nullopt;
            left = *down;
            sound = kSoundRoadblockDown;
        }
    }
    else
    {
        const reg32 lane = at(app, cop + kCarLane);
        reg32 side = i32(lane) < 6 ? 0 : i32(lane) > 7 ? 2 : 1;
        if (gameSettings(app).backwards != 0)
            side = 2 - side;
        const auto up = feCore<0x4c9dd0, frontend::simc::sirenOn>(app, cpu, at(app, cop + kCarRoadblock),
                                                                   at(app, cop + kCarSlice), lane,
                                                                   at(app, kRaceFrame) + kRoadblockFrames);
        if (!up)
            return std::nullopt;
        const auto told = guest::direct<0x40b9c0, pursuit::radioStoppedOf>(app, cpu, cop, side);
        if (!told)
            return std::nullopt;
        left = told->eax;
        sound = kSoundRoadblockSet;
    }
    if (gameSettings(app).localPlayer == at(app, cop + kCarPlayer) || i32(gameSettings(app).playMode) <= 1)
    {
        const port::Result played = guest::direct<0x418c30, port::copEffectPlayed>(app, cpu, cpu.esp - 4, i32(sound));
        if (cpu.terminate || !played)
            return std::nullopt;
        left = reg32(*played);
    }
    putByte(app, cop + kCarRoadblockHeld, 1);
    return left;
}

/* Cop eax. */
x86::reg32 callRoadblock(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return callRoadblockFor(app, cpu, cop); });
    return args.eax;
}

/* 0x404f80: a player's cop's siren switched by its button ([+0x50e]), once a press ([+0x620]
 * 0x10 held): the siren's flag ([+0x710] 0x10) turned over, its sound ([+0x734]) 1 as it comes
 * on, 0 as it goes off. */
void switchSiren(App* app, reg32 cop)
{
    if (byte(app, cop + kCarSirenButton) == 0)
    {
        putByte(app, cop + kCarAiFlags, byte(app, cop + kCarAiFlags) & ~kAiSirenHeld);
        return;
    }
    if ((byte(app, cop + kCarAiFlags) & kAiSirenHeld) != 0)
        return;
    const reg8 flags = byte(app, cop + kCarCopFlags);
    if ((flags & kCopSiren) != 0)
    {
        put(app, cop + kCarSirenSound, 0);
        putByte(app, cop + kCarCopFlags, flags & ~kCopSiren);
    }
    else
    {
        put(app, cop + kCarSirenSound, 1);
        putByte(app, cop + kCarCopFlags, flags | kCopSiren);
    }
    putByte(app, cop + kCarAiFlags, byte(app, cop + kCarAiFlags) | kAiSirenHeld);
}

/* Cop eax; the registers its code changes are popped. */
x86::reg32 sirenSwitch(App* app, x86::CPU& cpu, x86::Args& args)
{
    switchSiren(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

// The map's kinds of a racer: busted, finished, as it is, near, the nearest, the one chased.
constexpr reg32 kMapBusted = 1, kMapFinished = 2, kMapRacer = 3, kMapNear = 4, kMapNearest = 5, kMapChased = 7;
// A gap no racer's is.  The map's label set (port::hud::labelTextSet).
constexpr i32 kNoGap = 0x186a0;

/* 0x485180: racer `index` (its record) on player's map as `kind`.  What it leaves in eax, nullopt
 * when the game stopped. */
std::optional<reg32> showOnMap(App* app, CPU& cpu, reg32 player, reg32 index, reg32 kind, reg32 racer)
{
    const std::optional<reg32> shown =
        guest::direct<0x485180, port::hud::labelTextSetOf>(app, cpu, cpu.esp - 4, player, index, kind, racer);
    if (cpu.terminate)
        return std::nullopt;
    return shown;
}

/* 0x404ff0: cop watches the racers ([kRacers]): the nearest of them within 50 slices (the gap,
 * the distance of the places breaking a tie) to [+0x730]; where this machine's player drives it
 * (one player, or the camera's car, 0x422bd0) each racer goes on the map (finished, busted, near,
 * the one it chases, as it is) and, where it chases none, the nearest.  What the code leaves in
 * eax -- the last map's, gap's size, the tie's compare or the racer's kind in ah; nullopt when the
 * game stopped. */
std::optional<reg32> watchRacersFor(App* app, CPU& cpu, reg32 cop)
{
    const bool onePlayer = gameSettings(app).playMode == 1;
    const reg32 player = onePlayer ? at(app, cop + kCarPlayer) : 0;
    bool shown = true;
    if (!onePlayer)
    {
        const reg32 camera = guest::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, 0);
        if (cpu.terminate)
            return std::nullopt;
        shown = camera == cop;
    }
    put(app, cop + kCarNearestRacer, 0);
    const reg32 copSlice = at(app, cop + kCarSlice);
    reg32 left = copSlice;
    i32 nearestGap = kNoGap;
    reg32 nearestIndex = 0;  // set where [+0x730] is, before it is read
    const auto show = [&](reg32 index, reg32 kind, reg32 racer) {
        if (!shown)
            return true;
        const auto shownAs = showOnMap(app, cpu, player, index, kind, racer);
        if (shownAs)
            left = *shownAs;
        return shownAs.has_value();
    };
    for (i32 i = 0; i < i32(at(app, kRacerCount)); ++i)
    {
        const reg32 racer = at(app, kRacers + reg32(i) * 4);
        left = (left & 0xffff00ff) | reg32(byte(app, racer + kCarKind)) << 8;
        if (racerIn(app, racer, kStateFinished) || racerIn(app, racer, kStateBusted))
        {
            if (!show(reg32(i), racerIn(app, racer, kStateFinished) ? kMapFinished : kMapBusted, racer))
                return std::nullopt;
            continue;
        }
        const auto first = sliceGap(app, cpu, at(app, racer + kCarSlice), copSlice);
        if (!first)
            return std::nullopt;
        const auto gap = sliceGap(app, cpu, at(app, racer + kCarSlice), copSlice);
        if (!gap)
            return std::nullopt;
        left = *first > 0 ? reg32(*gap) : reg32(0) - reg32(*gap);
        reg32 kind = kMapRacer;
        if (i32(left) <= kNearSlices)
        {
            kind = kMapNear;
            bool nearest = false;
            if (i32(left) < nearestGap)
            {
                nearestGap = i32(left);
                nearest = true;
            }
            else if (i32(left) == nearestGap)
            {
                // As near as the nearest: the nearer of the two by distance.
                const auto mine = distanceApart(app, cpu, racer + kCarPlace, cop + kCarPlace);
                if (!mine)
                    return std::nullopt;
                const float mineF = float(*mine);
                reg32 eax;
                const auto theirs =
                    distanceApart(app, cpu, at(app, cop + kCarNearestRacer) + kCarPlace, cop + kCarPlace, &eax);
                if (!theirs)
                    return std::nullopt;
                const float theirsF = float(*theirs);
                left = statusInto(eax, Float(mineF), Float(theirsF));
                nearest = below(Float(mineF), Float(theirsF));
            }
            if (nearest)
            {
                nearestIndex = reg32(i);
                put(app, cop + kCarNearestRacer, racer);
            }
        }
        if (racer == at(app, cop + kCarChased))
            kind = kMapChased;
        if (!show(reg32(i), kind, racer))
            return std::nullopt;
    }
    const reg32 nearest = at(app, cop + kCarNearestRacer);
    if (nearest != 0 && at(app, cop + kCarChased) == 0 && !show(nearestIndex, kMapNearest, nearest))
        return std::nullopt;
    return left;
}

/* Cop eax. */
x86::reg32 watchRacers(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return watchRacersFor(app, cpu, cop); });
    return args.eax;
}

// A racer's place in the race (+0x2bc); none better than this.  The frames after which the cop
// jumps behind the leader.
constexpr reg32 kCarRacePlace = 0x2bc;
constexpr i32 kNoPlace = 0x2710, kJumpAfterFrames = 0x480;

/* Whether the racer is out of the chase: finished or busted. */
bool racerOut(App* app, reg32 racer)
{
    return racerIn(app, racer, kStateFinished) || racerIn(app, racer, kStateBusted);
}

/* 0x405220: cop put back behind the race's leader: the racer of the best place ([+0x2bc]) not
 * finished or busted; in a one-player Hot Pursuit with the option on after the first 0x480
 * frames, the cop's slice the one before the leader's; then put back on the road (0x429100) where
 * `putBack` asks it or a leader was found.  What the code leaves in eax -- the putting back's, the
 * slice, the last racer looked at or the cop; nullopt when the game stopped. */
std::optional<reg32> jumpBehind(App* app, CPU& cpu, reg32 cop, bool putBack)
{
    reg32 left = cop;
    i32 best = kNoPlace, leader = -1;
    for (i32 i = 0; i < i32(at(app, kRacerCount)); ++i)
    {
        const reg32 racer = at(app, kRacers + reg32(i) * 4);
        left = racer;
        if (racerOut(app, racer))
            continue;
        const i32 place = i32(at(app, racer + kCarRacePlace));
        if (best > place)
        {
            best = place;
            leader = i;
        }
    }
    if (gameSettings(app).playMode == 0 && gameSettings(app).raceOptionPursuit != 0
        && i32(at(app, kRaceFrame)) > kJumpAfterFrames && leader > -1)
    {
        reg32 slice = at(app, at(app, kRacers + reg32(leader) * 4) + kCarSlice) - 1;
        if (i32(slice) < 0)
            slice += at(app, kSliceCount);
        put(app, cop + kCarSlice, slice);
        left = slice;
    }
    if (putBack || leader > -1)
        return guest::direct<0x429100, racestart::resetCarOf>(app, cpu, cop);
    return left;
}

/* Cop eax, whether to put it back edx. */
x86::reg32 jumpBehindLeader(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax, putBack = args.edx != 0] { return jumpBehind(app, cpu, cop, putBack); });
    return args.eax;
}

/* 0x40ed40 (aiplan::trafficSpeed): slice's speed; nullopt when the game stopped. */
std::optional<Float> sliceSpeed(App* app, CPU& cpu, reg32 slice)
{
    const float speed = guest::direct<0x40ed40, aiplan::trafficSpeed>(app, cpu, slice);
    if (cpu.terminate)
        return std::nullopt;
    return Float(speed);
}

// A pursuit's record: marked for the racer clocked past 2 over (+0x14).
constexpr reg32 kPursuitClocked = 0x14;

/* 0x4052e0: cop clocks the racers within 50 slices of it: how far each goes over its slice's
 * speed (0x40ed40), the most of it kept in the racer's [+0x6f0]; past 2 (its bits compared as an
 * integer, as the code does), its pursuit's record ([+0x628]) marked ([+0x14] 1).  What the code
 * leaves in eax -- the last gap's size, clocking or record, or the cop; nullopt when the game
 * stopped. */
std::optional<reg32> clockRacersFor(App* app, CPU& cpu, reg32 cop)
{
    const reg32 copSlice = at(app, cop + kCarSlice);
    reg32 left = cop;
    for (i32 i = 0; i < i32(at(app, kRacerCount)); ++i)
    {
        const reg32 racer = at(app, kRacers + reg32(i) * 4);
        if (racerOut(app, racer))
            continue;
        const auto gap = sliceGap(app, cpu, at(app, racer + kCarSlice), copSlice);
        if (!gap)
            return std::nullopt;
        left = *gap <= 0 ? reg32(0) - reg32(*gap) : reg32(*gap);
        if (i32(left) > kNearSlices)
            continue;
        const auto speed = sliceSpeed(app, cpu, at(app, racer + kCarSlice));
        if (!speed)
            return std::nullopt;
        const float over = float(fp::sub(f32(app, racer + kCarSpeedSize), *speed));
        left = belowOrEqual(f32(app, racer + kCarClocked), Float(over)) ? floatBits(over)
                                                                         : at(app, racer + kCarClocked);
        put(app, racer + kCarClocked, left);
        if (i32(left) > kTwoOverBits)
        {
            left = at(app, racer + kCarPursuit);
            if (at(app, left + kPursuitClocked) == 0)
                put(app, left + kPursuitClocked, 1);
        }
    }
    return left;
}

/* Cop eax. */
x86::reg32 clockRacers(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return clockRacersFor(app, cpu, cop); });
    return args.eax;
}

/* 0x4018b0 (stdcall, its fourth on the stack): a pursuit joined -- `how`, the cop, the racer, 0.
 * What it gives, nullopt when the game stopped.  By address: startChase leaves what it gives in
 * eax, which airace::joinPursuitOf gives only as whether it joined. */
std::optional<reg32> joinPursuit(App* app, CPU& cpu, reg32 how, reg32 cop, reg32 racer)
{
    return guest::direct<0x4018b0, airace::joinPursuitLeaving>(app, cpu, how, cop, racer, 0.0f);
}

/* 0x4053b0: a cop with its siren on and no one to chase starts after the racer nearest it
 * ([+0x730]) where it was clocked past 2 over ([+0x6f0], a float's bits compared as an integer,
 * as the code does: any NaN with its sign clear is past) and is not finished or busted
 * (joinPursuit 2).  What the code leaves in eax: the cop, or what joinPursuit gives; nullopt
 * when the game stopped. */
std::optional<reg32> startChase(App* app, CPU& cpu, reg32 cop)
{
    if ((byte(app, cop + kCarCopFlags) & kCopSiren) == 0 || at(app, cop + kCarChased) != 0)
        return cop;
    const reg32 racer = at(app, cop + kCarNearestRacer);
    if (racer == 0 || i32(at(app, racer + kCarClocked)) <= kTwoOverBits || racerIn(app, racer, kStateFinished)
        || racerIn(app, racer, kStateBusted))
        return cop;
    return joinPursuit(app, cpu, kJoinChase, cop, racer);
}

/* Cop eax. */
x86::reg32 startPursuit(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return startChase(app, cpu, cop); });
    return args.eax;
}

/* Whether the car cop chases ([+0x624]) is at its own car's end: a human's in state 7, a
 * computer's in state 6 ([+0x6e0]) -- the code inlines it three times. */
bool chasedAtEnd(App* app, reg32 cop)
{
    const reg32 chased = at(app, cop + kCarChased);
    const bool human = (byte(app, chased + kCarKind) & kKindHuman) != 0;
    return at(app, chased + kCarCopState) == (human ? kCopStateHumanEnd : kCopStateComputerEnd);
}

/* 0x405430: whether cop busts the car it chases: within 7 of it, the car no faster than
 * [kBustSpeed], the race under way and the cop upright -- the first time (0x4127b0) a grace of
 * 0x18 frames ([+0x720], [+0x710] 0x40), after it the pursuit's record ([+0x628]) set to 2 with
 * 0x280 frames ([+0x28]) and the bust where the car is at its end; out of reach the grace is
 * gone, and a pursuit at 2 busts where the car is at its end and the frames are up.  nullopt
 * when the game stopped. */
std::optional<bool> busts(App* app, CPU& cpu, reg32 cop)
{
    if (at(app, cop + kCarChased) == 0)
        return false;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (at(app, pursuit + kPursuitStage) == kPursuitBusting && chasedAtEnd(app, cop))
        return true;
    const auto distance = distanceApart(app, cpu, at(app, cop + kCarChased) + kCarPlace, cop + kCarPlace);
    if (!distance)
        return std::nullopt;
    const Float apart = Float(float(*distance));
    bool inReach = false;
    if (below(apart, f64(app, kBustReach)))
    {
        const reg32 chased = at(app, cop + kCarChased);
        if (belowOrEqual(f32(app, chased + kCarSpeedSize), f32(app, kBustSpeed)) && half(app, cop + kCarStage) == 0)
            inReach = above(f32(app, cop + kCarUpright), f64(app, kUprightEnough));
    }
    if (inReach)
    {
        if ((byte(app, cop + kCarCopFlags) & kCopGrace) != 0)
        {
            const reg32 frame = at(app, kRaceFrame);
            if (i32(frame) > i32(at(app, cop + kCarGraceEnd)))
            {
                put(app, pursuit + kPursuitStage, kPursuitBusting);
                put(app, pursuit + kPursuitEnd, frame + kBustFrames);
                if (chasedAtEnd(app, cop))
                    return true;
            }
        }
        else
        {
            // (0x4127b0 only compares the car's flags, which no caller tests.)
            put(app, cop + kCarGraceEnd, at(app, kRaceFrame) + kGraceFrames);
            putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) | kCopGrace);
        }
        return false;
    }
    putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) & ~kCopGrace);
    return at(app, pursuit + kPursuitStage) == kPursuitBusting && chasedAtEnd(app, cop)
           && i32(at(app, kRaceFrame)) > i32(at(app, pursuit + kPursuitEnd));
}

/* Cop eax; eax 1 for the bust. */
x86::reg32 bustCheck(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return busts(app, cpu, cop); });
    return args.eax;
}

/* 0x405630: a pursuit's step: with the siren on, the car chased within 300 and not finished,
 * whether it is busted (busts); else the pursuit given up (0x401a80) and 0.  nullopt when the
 * game stopped. */
std::optional<bool> chaseStep(App* app, CPU& cpu, reg32 cop)
{
    bool keep = false;
    if ((byte(app, cop + kCarCopFlags) & kCopSiren) != 0)
    {
        const auto distance = distanceApart(app, cpu, at(app, cop + kCarChased) + kCarPlace, cop + kCarPlace);
        if (!distance)
            return std::nullopt;
        if (belowOrEqual(Float(float(*distance)), f64(app, kChaseReach)))
            keep = !racerIn(app, at(app, cop + kCarChased), kStateFinished);
    }
    if (keep)
        return busts(app, cpu, cop);
    if (!guest::direct<0x401a80, airace::giveUpPursuitOf>(app, cpu, cop))
        return std::nullopt;
    return false;
}

/* Cop eax; eax 1 for a bust. */
x86::reg32 pursuitStep(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return chaseStep(app, cpu, cop); });
    return args.eax;
}

/* 0x4056e0: car's pursuit dropped: the computer no longer drives it ([+0x200] 2), its state 2
 * ([+0x6e0]), its pursuit's record's 0; put back on the road (0x429100) where its flag says
 * ([+0x621] 0x40).  What the code leaves in eax (the car, or the putting back's), nullopt when
 * the game stopped. */
std::optional<reg32> dropChase(App* app, CPU& cpu, reg32 car)
{
    putByte(app, car + kCarKind, byte(app, car + kCarKind) & ~kKindComputer);
    const reg32 pursuit = at(app, car + kCarPursuit);
    put(app, car + kCarCopState, kCopStateDropped);
    put(app, pursuit + kPursuitStage, 0);
    if ((byte(app, car + kCarAiFlags2) & kAi2PutBack) != 0)
        return guest::direct<0x429100, racestart::resetCarOf>(app, cpu, car);
    return car;
}

/* Car eax. */
x86::reg32 dropPursuit(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return dropChase(app, cpu, car); });
    return args.eax;
}

// A cop's states (+0x6e0) by the code's table of targets (0x405714): the start (and any past 7),
// driving, a racer's driving, patrolling, chasing, the bust, out of the race, the end.
constexpr reg32 kCopStateTable = 0x405714;
constexpr reg32 kCopStart = 0x40575a, kCopDriving = 0x4057af, kCopRacing = 0x4057c2, kCopPatrol = 0x40583b,
                kCopChasing = 0x405888, kCopBusting = 0x4058ea, kCopOut = 0x405920, kCopEnd = 0x4059b0;
constexpr reg32 kCopStateRacing = 2, kCopStatePatrol = 3, kCopStateChasing = 4, kCopStateBust = 5,
                kCopStateOut = 6, kCopStateEnd = 7;
// A cop's timer (+0x6ec), its frames out of the race; its pursuit's record's say to drop it (+0x2c).
constexpr reg32 kCarCopTimer = 0x6ec, kCopOutFrames = 0x140, kPursuitKept = 0x2c;
// Upright enough (a double) and slow enough (a double) to leave the race's end.
constexpr reg32 kCopUpright = 0x5361cc, kCopSlow = 0x5361d4;
/* 0x405740: a cop's driver, by its state [+0x6e0] through the table at 0x405714: 0 the start (3
 * for a cop, its chase cleared; 2 else); 1 driving; 2 a racer's driving (with the lanes' votes, or
 * the three slices), 6 with 0x140 frames once the pursuit's say says so; 3 patrolling and 4
 * chasing (the racers watched and clocked, the siren, the roadblock, the pursuit started -- to 4
 * -- or kept up, 5 on a bust, 3 again without one to chase); 5 the bust (back to 3 at its end); 6
 * out of the race (7 or dropped once it stands upright and slow, or its time is up); 7 the end (to
 * 2 and put back where the say is no, dropped where its record says).  The store after state 0's
 * ret (0x405791) is never reached: nothing jumps there, the table neither.  What the code leaves
 * in eax; nullopt when the game stopped. */
std::optional<reg32> driveCop(App* app, CPU& cpu, reg32 cop)
{
    std::optional<reg32> left = cop;
    // The calls of the car's that leave their eax: a car's three slices, a pursuit's say, a cop stopping
    // by, a bust's end; false when the game stopped.
    const auto slicesLeaving = [&] {
        left = guest::direct<0x40e350, aiplan::findCarSlicesLeaving>(app, cpu, cop);
        return !cpu.terminate;
    };
    const auto sayLeaving = [&] {
        left = guest::direct<0x401500, airace::pulledOverOf>(app, cpu, cop);
        return !cpu.terminate;
    };
    const auto stopsByLeaving = [&] {
        left = guest::direct<0x401e40, airace::copStopsByLeaving>(app, cpu, cop);
        return left.has_value();
    };
    const auto bustEndLeaving = [&] {
        const std::optional<bool> over = guest::direct<0x401fb0, airace::stopOverOf>(app, cpu, cop);
        if (!over)
            return false;
        left = *over ? 1u : 0u;
        return true;
    };
    // The cop's lane votes acted on (0x40de30): its eax kept; false when the game stopped.
    const auto lanesDone = [&] { return bool(left = feCore<0x40de30, frontend::sima::laneSteer>(app, cpu, cop)); };
    // Those called as C++: the race's leader, a car's three slices, the cop stopped, the roadside
    // target and a lane's time counted; false when the game stopped.
    const auto raceLeader = [&] {
        guest::direct<0x404c80, airace::findLeaderOf>(app, cpu, cop);
        return !cpu.terminate;
    };
    const auto threeSlices = [&] {
        guest::direct<0x40e350, aiplan::findCarSlices>(app, cpu, cop);
        return !cpu.terminate;
    };
    const auto copStopped = [&] { return guest::direct<0x40ce70, aiplan::brakeToStopDone>(app, cpu, cop); };
    // The cops' watch: the racers clocked and watched, the siren, the roadblock.
    const auto watch = [&] {
        if (!raceLeader() || !threeSlices() || !clockRacersFor(app, cpu, cop) || !watchRacersFor(app, cpu, cop))
            return false;
        switchSiren(app, cop);
        return callRoadblockFor(app, cpu, cop).has_value();
    };
    const reg32 state = at(app, cop + kCarCopState);
    switch (state <= 7 ? at(app, kCopStateTable + state * 4) : kCopStart)
    {
    default:  // kCopStart; its compare of the play mode for this machine's player is never used
        left = state;
        if ((byte(app, cop + kCarKind) & kKindCop) != 0)
        {
            put(app, cop + kCarChased, 0);
            put(app, cop + kCarCopState, kCopStatePatrol);
        }
        else
            put(app, cop + kCarCopState, kCopStateRacing);
        break;
    case kCopDriving:
        // The three slices: its eax is what the code leaves.
        if (!raceLeader() || !slicesLeaving())
            return std::nullopt;
        break;
    case kCopRacing:
        if (!raceLeader())
            return std::nullopt;
        if (gameSettings(app).raceOptionLaneVotes != 0)
        {
            if (!guest::direct<0x40dd60, aiplan::beginPlanning>(app, cpu, cop)
                || !guest::direct<0x4096b0, pursuit::scoreLaneOf>(app, cpu, cop)
                || !guest::direct<0x40ddb0, aiplan::pickLane>(app, cpu, cop))
                return std::nullopt;
            guest::direct<0x409660, pursuit::growLaneTimesBy>(app, cpu);
            if (cpu.terminate || !lanesDone())
                return std::nullopt;
        }
        else if (!threeSlices())
            return std::nullopt;
        if (!sayLeaving())
            return std::nullopt;
        if (*left != 0)
        {
            const reg8 kind = byte(app, cop + kCarKind);
            put(app, cop + kCarCopState, kCopStateOut);
            left = at(app, kRaceFrame) + kCopOutFrames;
            put(app, cop + kCarCopTimer, *left);
            putByte(app, cop + kCarKind, kind | kKindComputer);
        }
        break;
    case kCopPatrol:
        if (!watch() || !(left = startChase(app, cpu, cop)))
            return std::nullopt;
        if (at(app, cop + kCarChased) != 0)
            put(app, cop + kCarCopState, kCopStateChasing);
        break;
    case kCopChasing:
    {
        if (!watch())
            return std::nullopt;
        const auto bust = chaseStep(app, cpu, cop);
        if (!bust)
            return std::nullopt;
        left = *bust ? 1 : 0;
        if (*bust)
        {
            put(app, cop + kCarCopState, kCopStateBust);
            if (!stopsByLeaving())
                return std::nullopt;
        }
        if (at(app, cop + kCarChased) == 0)
            put(app, cop + kCarCopState, kCopStatePatrol);
        break;
    }
    case kCopBusting:
        if (!raceLeader())
            return std::nullopt;
        put(app, cop + kCarSpeedWanted, 0);
        if (!copStopped() || !bustEndLeaving())
            return std::nullopt;
        if (*left != 0)
            put(app, cop + kCarCopState, kCopStatePatrol);
        break;
    case kCopOut:
    {
        if (!raceLeader() || !threeSlices())
            return std::nullopt;
        guest::direct<0x408b30, pursuit::stopAtRoadside>(app, cpu, cop);
        if (cpu.terminate)
            return std::nullopt;
        guest::direct<0x408bd0, pursuit::addLaneTime>(app, cpu, cop);
        if (cpu.terminate || !lanesDone())
            return std::nullopt;
        if (half(app, cop + kCarStage) != 0)
            break;
        const Float upright = f32(app, cop + kCarUpright);
        left = statusInto(*left, upright, f64(app, kCopUpright));
        if (!above(upright, f64(app, kCopUpright)))
            break;
        const Float speed = f32(app, cop + kCarSpeedSize);
        left = statusInto(*left, speed, f64(app, kCopSlow));
        if (!below(speed, f64(app, kCopSlow)))
        {
            left = at(app, kRaceFrame);
            if (i32(*left) <= i32(at(app, cop + kCarCopTimer)))
                break;
        }
        left = at(app, cop + kCarPursuit);
        if (at(app, *left + kPursuitKept) != 0)
        {
            put(app, cop + kCarCopState, kCopStateEnd);
            break;
        }
        if (!(left = dropChase(app, cpu, cop)))
            return std::nullopt;
        break;
    }
    case kCopEnd:
    {
        if (!raceLeader())
            return std::nullopt;
        put(app, cop + kCarSpeedWanted, 0);
        if (!copStopped() || !copStopped())
            return std::nullopt;
        const bool pulledOver = guest::direct<0x401500, airace::isPulledOver>(app, cpu, cop);
        if (cpu.terminate)
            return std::nullopt;
        if (!pulledOver)
        {
            const reg8 kind = byte(app, cop + kCarKind);
            put(app, cop + kCarCopState, kCopStateRacing);
            putByte(app, cop + kCarKind, kind & ~kKindComputer);
            if (!guest::direct<0x429100, racestart::resetCarOf>(app, cpu, cop))
                return std::nullopt;
        }
        left = at(app, cop + kCarPursuit);
        if (at(app, *left + kPursuitKept) == 0 && !(left = dropChase(app, cpu, cop)))
            return std::nullopt;
        break;
    }
    }
    return left;
}

/* Cop eax. */
x86::reg32 copDriver(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, cop = args.eax] { return driveCop(app, cpu, cop); });
    return args.eax;
}

// How near counts: the smaller gap's share (a float, a quarter) and the reach (a float, 366).
constexpr reg32 kNearShare = 0x5361dc, kNearReach = 0x5361e0;

/* The size of a float the code's way: fldz; fcomp; jae -- the value itself where 0 is below it
 * (or it is a NaN), else negated. */
float sizeOf(float value)
{
    return below(Float(0.0), Float(value)) ? value : float(-Float(value));
}

/* 0x405a20: whether the place at `place` is near car (none for no car): the bigger of the two
 * gaps across the ground (x, z) and a quarter of the smaller, rounded to a float, at most 366. */
bool nearPlace(App* app, reg32 place, reg32 car)
{
    if (car == 0)
        return false;
    const float acrossZ = sizeOf(float(fp::sub(f32(app, place + 8), f32(app, car + kCarPlace + 8))));
    const float acrossX = sizeOf(float(fp::sub(f32(app, place), f32(app, car + kCarPlace))));
    const float near = above(Float(acrossX), Float(acrossZ))
                           ? float(fp::add(fp::mul(Float(acrossZ), f32(app, kNearShare)), Float(acrossX)))
                           : float(fp::add(fp::mul(Float(acrossX), f32(app, kNearShare)), Float(acrossZ)));
    return belowOrEqual(Float(near), f32(app, kNearReach));
}

/* The place eax, car edx; eax 1 near. */
x86::reg32 nearCar(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = nearPlace(app, args.eax, args.edx) ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

/* 0x405af0: the first human's car ([kHumans]) near car's place (nearPlace), or 0.  The code
 * hands on a copy of the place in its frame: the same floats, which nothing writes meanwhile. */
reg32 humanNear(App* app, reg32 car)
{
    for (i32 i = 0; i < i32(at(app, kHumanCount)); ++i)
    {
        const reg32 human = at(app, kHumans + reg32(i) * 4);
        if (nearPlace(app, car + kCarPlace, human))
            return human;
    }
    return 0;
}

/* Car eax. */
x86::reg32 nearHuman(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = humanNear(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x405c80: the first human's car within 50 slices of `slice` (either way), or 0; nullopt when
 * the game stopped. */
std::optional<reg32> humanNearSliceOf(App* app, CPU& cpu, reg32 slice)
{
    for (i32 i = 0; i < i32(at(app, kHumanCount)); ++i)
    {
        const auto gap = sliceGap(app, cpu, at(app, at(app, kHumans + reg32(i) * 4) + kCarSlice), slice);
        if (!gap)
            return std::nullopt;
        const reg32 size = *gap <= 0 ? reg32(0) - reg32(*gap) : reg32(*gap);
        if (i32(size) < kNearSlices)
            return at(app, kHumans + reg32(i) * 4);
    }
    return 0;
}

/* Slice eax. */
x86::reg32 humanNearSlice(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, slice = args.eax] { return humanNearSliceOf(app, cpu, slice); });
    return args.eax;
}

/* 0x405cd0: whether traffic car (on the road, [+0x620] 2) is out of every human's sight: not
 * near the human it was near ([+0x6d8]) nor any other (humanNear, kept in [+0x6d8]).  The code
 * hands on a copy of the place in its frame, as humanNear; it also tests a cop's flag and never
 * uses it. */
bool unwatched(App* app, reg32 car)
{
    if ((byte(app, car + kCarAiFlags) & kAiOnRoad) == 0 || (byte(app, car + kCarKind) & kKindTraffic) == 0)
        return false;
    if (nearPlace(app, car + kCarPlace, at(app, car + kCarNearHuman)))
        return false;
    const reg32 human = humanNear(app, car);
    put(app, car + kCarNearHuman, human);
    return human == 0;
}

/* Car eax; eax 1 unwatched. */
x86::reg32 trafficUnwatched(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = unwatched(app, args.eax) ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

/* 0x40eed0 (aiplan::trackTuningDistance): traffic's life; nullopt when the game stopped. */
std::optional<reg32> trafficLife(App* app, CPU& cpu)
{
    const reg32 life = guest::direct<0x40eed0, aiplan::trackTuningDistance>(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    return life;
}

/* 0x405d40: extra car comes onto the road: near the first car ([+0x6d8]), its life a random
 * share (the game's random numbers, seed by multiplier) of what traffic's life gives, plus 1
 * ([+0x650]); counted among the extra cars, on the road ([+0x620] 4), [+0x89] 0.  Its life,
 * nullopt when the game stopped. */
std::optional<reg32> bringOn(App* app, CPU& cpu, reg32 car)
{
    put(app, car + kCarNearHuman, at(app, kFirstCar));
    const reg32 product = at(app, kSeed) * at(app, kMultiplier);
    put(app, kProduct, product);
    const reg32 seed = product & 0xffff;
    put(app, kSeed, seed);
    const reg32 share = (product & 0xffff00) >> 8;
    const auto life = trafficLife(app, cpu);
    if (!life)
        return std::nullopt;
    const reg32 frames = ((*life * share) >> 16) + 1;
    putByte(app, car + kCarHeld, 0);
    put(app, car + kCarLife, frames);
    put(app, kExtraCars, at(app, kExtraCars) + 1);
    putByte(app, car + kCarAiFlags, byte(app, car + kCarAiFlags) | kAiOnRoadExtra);
    return frames;
}

/* Car eax; eax its life with ah the flags' byte, as the code leaves it. */
x86::reg32 trafficStart(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax]() -> std::optional<reg32> {
        const auto life = bringOn(app, cpu, car);
        if (!life)
            return std::nullopt;
        return (*life & 0xffff00ff) | reg32(byte(app, car + kCarAiFlags)) << 8;
    });
    return args.eax;
}

/* 0x405dc0: extra car leaves the road: one fewer extra car, off the road ([+0x620] 4 cleared),
 * [+0x89] 1; a traffic car's (0x40c590, its kind as it came) and a cop's (0x4029c0, its kind as
 * that leaves it) own leaving.  What the code leaves in eax -- the last leaving's, or the car
 * with the flags' byte in its second -- nullopt when the game stopped. */
std::optional<reg32> takeOff(App* app, CPU& cpu, reg32 car)
{
    const reg8 flags = byte(app, car + kCarAiFlags) & ~kAiOnRoadExtra;
    const reg8 kind = byte(app, car + kCarKind);
    putByte(app, car + kCarHeld, 1);
    put(app, kExtraCars, at(app, kExtraCars) - 1);
    putByte(app, car + kCarAiFlags, flags);
    reg32 left = (car & 0xffff00ff) | reg32(flags) << 8;
    if ((kind & kKindTraffic) != 0)
    {
        const std::optional<reg32> placed = guest::direct<0x40c590, aiplan::placeTraffic>(app, cpu, car);
        if (!placed)
            return std::nullopt;
        left = *placed;
    }
    if ((byte(app, car + kCarKind) & kKindCop) != 0)
        return guest::direct<0x4029c0, airace::placeCopAtStartLeaving>(app, cpu, car);
    return left;
}

/* Car eax. */
x86::reg32 trafficStop(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return takeOff(app, cpu, car); });
    return args.eax;
}

/* 0x405e20: an extra car's step: a traffic car's (0x40c210), a cop's (the always-one rule,
 * airace::alwaysOneOf, 0x402c70), 0 for any other.  What the step gives, nullopt when the game stopped. */
std::optional<reg32> stepExtraCar(App* app, CPU& cpu, reg32 car)
{
    const reg8 kind = byte(app, car + kCarKind);
    if ((kind & kKindTraffic) != 0)
    {
        const std::optional<bool> done = guest::direct<0x40c210, pursuit::countdownDoneOf>(app, cpu, car);
        if (!done)
            return std::nullopt;
        return *done ? 1u : 0u;
    }
    if ((kind & kKindCop) != 0)
    {
        const reg32 one = guest::direct<0x402c70, airace::alwaysOneOf>(app, cpu);
        if (cpu.terminate)
            return std::nullopt;
        return one;
    }
    return 0;
}

x86::reg32 extraCarStep(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return stepExtraCar(app, cpu, car); });
    return args.eax;
}

/* 0x405e60: the AI's tuning, set as a race starts: the steering's gains and limits
 * (0x56ebd0..), the reversing's (0x56ec04..), the off-road's (0x56ec30..) -- floats' bits, in
 * the code's order of stores. */
constexpr reg32 kDrivingConstants[][2] = {
    {0x56ebd0, 0x40e00000}, {0x56ebd4, 0x40a00000}, {0x56ebd8, 0x42000000}, {0x56ebdc, 0x42180000},
    {0x56ebe0, 0x40800000}, {0x56ebe4, 0x40200000}, {0x56ebf0, 0x40a00000}, {0x56ebe8, 0x3f4ccccd},
    {0x56ebec, 0x3f4ccccd}, {0x56ebf4, 0x41b2cfdf}, {0x56ebf8, 0x3e041893}, {0x56ebfc, 0x3c449ba6},
    {0x56ec08, 0x42180000}, {0x56ec0c, 0x40400000}, {0x56ec04, 0x41800000}, {0x56ec10, 0x40200000},
    {0x56ec14, 0x3fcccccd}, {0x56ec18, 0x3f4ccccd}, {0x56ec24, 0x3c23d70a}, {0x56ec28, 0x3ca3d70a},
    {0x56ec1c, 0x41200000}, {0x56ec30, 0x42000000}, {0x56ec34, 0x42200000}, {0x56ec38, 0x40c00000},
    {0x56ec44, 0x3f800000}, {0x56ec3c, 0x40400000}, {0x56ec40, 0x40400000}, {0x56ec48, 0x41200000},
    {0x56ec50, 0x3e4ccccd}, {0x56ec54, 0x3d23d70a},
};

void tuneDriving(App* app)
{
    for (const auto& store : kDrivingConstants)
        put(app, store[0], store[1]);
}

/* eax left the last value stored; the code's other registers are popped. */
x86::reg32 setDrivingConstants(App* app, x86::CPU& cpu, x86::Args& args)
{
    tuneDriving(app);
    args.eax = std::end(kDrivingConstants)[-1][1];
    cpu.esp += 4;
    return args.eax;
}

/* 0x405fb0: an empty hook. */
x86::reg32 drivingNothing(App* app, x86::CPU& cpu, x86::Args& args)
{
    cpu.esp += 4;
    return args.eax;
}

// setReversing's: on, off, and (any other) turned over.
constexpr reg32 kReverseOn = 0, kReverseOff = 1, kReverseTurnOver = 2;

/* The reverse started: one more ([+0x658]), for `frames` where that is not 0 ([+0x7d0] its end,
 * timed in [+0x620]), else untimed.  The frame it ends at, or the reverses' count, as the code
 * leaves them in edx (`edx` where neither). */
reg32 startReverse(App* app, reg32 car, reg32 frames, bool countLeft, reg32 edx)
{
    const reg32 reverses = at(app, car + kCarReverses) + 1;
    put(app, car + kCarReversing, 1);
    put(app, car + kCarReverses, reverses);
    if (frames == 0)
    {
        putByte(app, car + kCarAiFlags, byte(app, car + kCarAiFlags) & ~kAiTimedReverse);
        return countLeft ? reverses : edx;
    }
    const reg32 end = at(app, kRaceFrame) + frames;
    put(app, car + kCarReverseEnd, end);
    putByte(app, car + kCarAiFlags, byte(app, car + kCarAiFlags) | kAiTimedReverse);
    return end;
}

/* 0x405fd0: car's reversing, by `how`: on (kReverseOn) for `frames` where that is not 0; off;
 * any other turned over -- off where it was on (left on while its time is not up), else on.
 * Returns what its code leaves in edx: the end frame, the reverses' count, the race's frame or
 * `how` itself. */
reg32 setReversing(App* app, reg32 car, reg32 how, reg32 frames)
{
    if (how == kReverseOn)
        return startReverse(app, car, frames, true, how);
    if (how == kReverseOff)
    {
        const reg8 flags = byte(app, car + kCarAiFlags);
        put(app, car + kCarReversing, 0);
        putByte(app, car + kCarAiFlags, flags & ~kAiTimedReverse);
        return how;
    }
    if (at(app, car + kCarReversing) != 1)
        return startReverse(app, car, frames, false, how);
    const reg8 flags = byte(app, car + kCarAiFlags);
    put(app, car + kCarReversing, 0);
    if ((flags & kAiTimedReverse) == 0)
        return how;
    const reg32 frame = at(app, kRaceFrame);
    if (i32(frame) < i32(at(app, car + kCarReverseEnd)))
        put(app, car + kCarReversing, 1);
    return frame;
}

/* Car eax, how edx, frames ebx; eax left the car. */
x86::reg32 setReverse(App* app, x86::CPU& cpu, x86::Args& args)
{
    setReversing(app, args.eax, args.edx, args.ebx);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4060d0: car's reverse ended where it is timed and its time is up (setReversing off); while
 * it reverses otherwise, its eased offset ([+0x7c8]) 0. */
void reverseCheckFor(App* app, reg32 car)
{
    if (at(app, car + kCarReversing) != 1)
        return;
    if ((byte(app, car + kCarAiFlags) & kAiTimedReverse) != 0
        && i32(at(app, kRaceFrame)) > i32(at(app, car + kCarReverseEnd)))
    {
        setReversing(app, car, kReverseOff, 0);
        return;
    }
    put(app, car + kCarEasedOffset, 0);
}

/* Car eax; the registers setReversing's code changes are popped. */
x86::reg32 reverseCheck(App* app, x86::CPU& cpu, x86::Args& args)
{
    reverseCheckFor(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

// The speed under which a car counts as stuck (doubles: 10 and, for the tipped test, its own),
// how far down its nose points then (a double, -0.5), the reverse's frames then.
constexpr reg32 kStuckSpeed = 0x5361ec, kTippedSpeed = 0x5361f4, kTippedDown = 0x5361fc;
constexpr reg32 kStuckFrames = 0x80;

/* Whether car counts as moving: [+0x168]'s float not 0 (either sign), tested as bits. */
bool moving(App* app, reg32 car)
{
    return (at(app, car + kCarMoving) & 0x7fffffff) != 0;
}

/* 0x406120: car, moving and slower than 10, turns its reversing over for 0x80 frames.  Whether
 * it did. */
bool reverseIfSlow(App* app, reg32 car)
{
    if (!moving(app, car) || !below(f32(app, car + kCarSpeedSize), f64(app, kStuckSpeed)))
        return false;
    setReversing(app, car, kReverseTurnOver, kStuckFrames);
    return true;
}

/* Car eax; eax left with the compare's status word in its low word where it was made and the
 * reverse not turned over. */
x86::reg32 reverseWhenSlow(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 car = args.eax;
    if (!reverseIfSlow(app, car) && moving(app, car))
        args.eax = statusInto(car, f32(app, car + kCarSpeedSize), f64(app, kStuckSpeed));
    cpu.esp += 4;
    return args.eax;
}

/* 0x406160: as reverseIfSlow, where its nose also points down past -0.5: its dot product with
 * the vector `down` (0x4e01f0).  False when the game stopped; eax and edx left as the code leaves
 * them (the compares' status words in ax, setReversing's edx). */
struct TippedLeft
{
    reg32 eax;
    std::optional<reg32> edx;  // changed only where the reverse was turned over
};

std::optional<TippedLeft> reverseTippedLeaving(App* app, CPU& cpu, reg32 car, reg32 down)
{
    if (!moving(app, car))
        return TippedLeft{car, std::nullopt};
    const Float speed = f32(app, car + kCarSpeedSize);
    if (!below(speed, f64(app, kTippedSpeed)))
        return TippedLeft{statusInto(car, speed, f64(app, kTippedSpeed)), std::nullopt};
    // By address: the compare's status word goes into the eax the dot product leaves.
    const auto nose = dotLeavingEax(app, cpu, car + kCarNose, down);
    if (!nose)
        return std::nullopt;
    if (!below(nose->product, f64(app, kTippedDown)))
        return TippedLeft{statusInto(nose->eax, nose->product, f64(app, kTippedDown)), nose->edx};
    return TippedLeft{car, setReversing(app, car, kReverseTurnOver, kStuckFrames)};
}

bool reverseIfTipped(App* app, CPU& cpu, reg32 car, reg32 down)
{
    return reverseTippedLeaving(app, cpu, car, down).has_value();
}

/* Car eax, the vector edx. */
x86::reg32 reverseWhenTipped(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 ebx = args.ebx, ecx = args.ecx, ebp = args.ebp;
    const auto left = reverseTippedLeaving(app, cpu, args.eax, args.edx);
    if (!left)
        return args.eax;
    args.eax = left->eax;
    if (left->edx)
        args.edx = *left->edx;
    args.ebx = ebx;
    args.ecx = ecx;
    args.ebp = ebp;
    cpu.esp += 4;
    return args.eax;
}

// The engine's: past the slice's speed by this (a double, 10) it aims high; the aims by the top
// revs (doubles: 0.4 and 0.8 high, 0.2 and 0.5 low), 200 short of the top where it is slow
// against the speed it wants (by this, a double).  The performance's gears' count (+8), a gear
// change's frames (+0x10), the gears' revs by speed (+0x94, reverse's first), its idle and top
// revs (+0x1c0, +0x1c4).  A car's gear (+0x50b), its gear change's frames (+0x64c), its revs
// (+0x590).  Nothing nearer than this (100000 as a float's bits).
constexpr reg32 kPastSliceSpeed = 0x536204, kAimLowFast = 0x53620c, kSlowAgainstWanted = 0x536214,
                kAimHighFast = 0x53621c, kAimLowSlow = 0x536224, kAimHighSlow = 0x53622c;
constexpr reg32 kTopShort = 0xc8;
constexpr reg32 kPerformanceGears = 8, kPerformanceChangeFrames = 0x10, kPerformanceGearRevs = 0x94,
                kPerformanceIdle = 0x1c0, kPerformanceTop = 0x1c4;
constexpr reg32 kCarGear = 0x50b, kCarGearChange = 0x64c, kCarRevs = 0x590;
constexpr reg32 kNoneNearerBits = 0x47c35000;
constexpr reg32 kEnginePushed = 0x18, kEngineFrame = 0x30;

/* 0x4061b0: the computer's car's engine: the revs it aims at -- its performance's top revs by 0.4
 * and 0.8 (or 200 short of them) while the speed it wants is past its slice's speed and 10, by
 * 0.2 and 0.5 else; while a gear change is under way ([+0x64c]) the first aim chopped, the
 * change's frames counted down by the substeps (__CHP keeps eax); reversing, the reverse gear's
 * revs at its speed; else the gear (2 up to [+8]) whose revs at its speed come nearest the second
 * aim, its change started ([+0x10]) where it is another; never under the idle ([+0x1c0]); the
 * revs ([+0x590]) an eighth of the way there.  Where no gear comes nearer than 100000 (none, or
 * NaNs) the code's revs are what its frame's slot holds and its gear the caller's ebx (`noGear`):
 * the frame stays at the code's depth for that.  What the code leaves in eax (the revs' step, or
 * the aim where they are there); nullopt when the game stopped. */
std::optional<reg32> runEngine(App* app, CPU& cpu, reg32 car, reg32 noGear)
{
    const GuestFrame frame(cpu, kEnginePushed, kEngineFrame);
    const float wanted = sizeOf(app->getMemory<float>(car + kCarSpeedWanted));
    const auto speed = sliceSpeed(app, cpu, at(app, car + kCarSlice));
    if (!speed)
        return std::nullopt;
    const reg32 performance = at(app, car + kCarPerformance);
    const Float top = Float(sreg32(at(app, performance + kPerformanceTop)));
    float firstAim, secondAim;
    if (below(fp::add(*speed, f64(app, kPastSliceSpeed)), Float(double(wanted))))
    {
        firstAim = float(fp::mul(top, f64(app, kAimLowFast)));
        const float wantedSize = sizeOf(app->getMemory<float>(car + kCarSpeedWanted));
        if (below(fp::add(f32(app, car + kCarSpeedSize), f64(app, kSlowAgainstWanted)), Float(wantedSize)))
            secondAim = float(Float(sreg32(at(app, at(app, car + kCarPerformance) + kPerformanceTop) - kTopShort)));
        else
            secondAim = float(fp::mul(Float(sreg32(at(app, at(app, car + kCarPerformance) + kPerformanceTop))),
                                      f64(app, kAimHighFast)));
    }
    else
    {
        firstAim = float(fp::mul(top, f64(app, kAimLowSlow)));
        secondAim = float(fp::mul(top, f64(app, kAimHighSlow)));
    }
    reg32 revs = at(app, frame - 8);
    reg32 gear = noGear;
    const reg32 changing = at(app, car + kCarGearChange);
    if (i32(changing) > 0)
    {
        const reg32 substeps = at(app, kSubsteps);
        const auto chopped = chop(app, cpu, Float(firstAim));
        if (!chopped)
            return std::nullopt;
        revs = reg32(fp::toInteger<i32>(*chopped));
        put(app, car + kCarGearChange, changing - substeps);
    }
    else
    {
        if (at(app, car + kCarReversing) == 1)
        {
            gear = 0;
            const auto chopped = chop(app, cpu,
                                      fp::mul(-f32(app, performance + kPerformanceGearRevs), f32(app, car + kCarSpeedSize)));
            if (!chopped)
                return std::nullopt;
            revs = reg32(fp::toInteger<i32>(*chopped));
        }
        else
        {
            float nearest;
            std::memcpy(&nearest, &kNoneNearerBits, sizeof nearest);
            for (reg32 i = 2; i32(i) < i32(at(app, performance + kPerformanceGears)); ++i)
            {
                const Float inGear = fp::mul(f32(app, performance + i * 4 + kPerformanceGearRevs),
                                             f32(app, car + kCarSpeedSize));
                const float inGearF = float(inGear);
                const float off = sizeOf(float(fp::sub(inGear, Float(secondAim))));
                if (below(Float(off), Float(nearest)))
                {
                    gear = i;
                    const auto chopped = chop(app, cpu, Float(inGearF));
                    if (!chopped)
                        return std::nullopt;
                    revs = reg32(fp::toInteger<i32>(*chopped));
                    nearest = off;
                }
            }
        }
        const reg32 idle = at(app, at(app, car + kCarPerformance) + kPerformanceIdle);
        if (i32(revs) < i32(idle))
            revs = idle;
        if (reg32(byte(app, car + kCarGear)) != gear)
        {
            put(app, car + kCarGearChange, at(app, at(app, car + kCarPerformance) + kPerformanceChangeFrames));
            putByte(app, car + kCarGear, reg8(gear));
        }
    }
    const reg32 now = at(app, car + kCarRevs);
    if (i32(revs) > i32(now))
    {
        const reg32 step = reg32(i32(revs - now) >> 3);
        put(app, car + kCarRevs, now + step);
        return step;
    }
    if (i32(revs) < i32(now))
    {
        const reg32 step = reg32(i32(now - revs) >> 3);
        put(app, car + kCarRevs, at(app, car + kCarRevs) - step);
        return step;
    }
    return revs;
}

/* Car eax. */
x86::reg32 engineRevs(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax, noGear = args.ebx] { return runEngine(app, cpu, car, noGear); });
    return args.eax;
}

// The brake's: none where its size is under this (a double, 0.5); doubled (a double, 2) for a cop
// and going backwards; by 1.25 (a double) where +0x200 8 asks; let off past this shortfall (a
// double, -5).  The performance's brake (+0x1d8).
constexpr reg32 kNoBrakeUnder = 0x536234, kBrakeDouble = 0x53623c, kBrakeEighth = 0x536244, kBrakeOffPast = 0x53624c;
constexpr reg32 kPerformanceBrake = 0x1d8;
// +0x200's: braking harder.
constexpr reg8 kKindHarder = 8;
/* Whether car brakes already ([+0x509] 0xff) with no speed wanted (its bits 0, either sign) and
 * the shortfall under 0.5 in size: then no brake. */
bool brakesAlready(App* app, reg32 car, float shortfall)
{
    return byte(app, car + kCarBrake) == 0xff && (at(app, car + kCarSpeedWanted) & 0x7fffffff) == 0
           && below(Float(sizeOf(shortfall)), f64(app, kNoBrakeUnder));
}

/* 0x4063e0: the computer's car's brake for `shortfall` -- 0 where it brakes already; else its
 * performance's brake, doubled for a cop and again going backwards, by 1.25 where +0x200 8 asks
 * (0x404610's say unused); the brake on ([+0x509] 0xff), let off where it moves and the
 * shortfall is past -5.  nullopt when the game stopped. */
std::optional<float> brakeOf(App* app, CPU& cpu, reg32 car, float shortfall)
{
    if (brakesAlready(app, car, shortfall))
        return 0.0f;
    const reg32 performance = at(app, car + kCarPerformance);
    putByte(app, car + kCarBrake, 0xff);
    float brake = app->getMemory<float>(performance + kPerformanceBrake);
    const reg8 kind = byte(app, car + kCarKind);
    const auto scale = [&](reg32 factor) { brake = float(fp::mul(Float(brake), f64(app, factor))); };
    if ((kind & kKindCop) != 0)
        scale(kBrakeDouble);
    if (above(Float(0.0), f32(app, car + kCarForwardSpeed)))
        scale(kBrakeDouble);
    if ((byte(app, car + kCarKind) & kKindHarder) != 0)
    {
        guest::direct<0x404610, airace::classRuleOf>(app, cpu, car);
        if (cpu.terminate)
            return std::nullopt;
        scale(kBrakeEighth);
    }
    if ((at(app, car + kCarSpeedSize) & 0x7fffffff) != 0 && above(Float(shortfall), f64(app, kBrakeOffPast)))
        putByte(app, car + kCarBrake, 0);
    return brake;
}

/* Car eax, the shortfall on the stack (ret 4); the brake onto the x87's stack.  eax the brake's
 * bits, or where it brakes already as the code's compares leave it. */
x86::reg32 brakeForce(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 car = args.eax;
    const float shortfall = app->getMemory<float>(cpu.esp + 4);
    asThunk(
        cpu, args,
        [&]() -> std::optional<reg32> {
            const bool already = brakesAlready(app, car, shortfall);
            const auto brake = brakeOf(app, cpu, car, shortfall);
            if (!brake)
                return std::nullopt;
            args.floatSet = true;
            args.floatResult = double(Float(*brake));
            if (!already)
                return floatBits(*brake);
            const reg32 left = below(Float(0.0), Float(shortfall)) ? floatBits(shortfall)
                                                                   : statusInto(car, Float(0.0), Float(shortfall));
            return statusInto(left, Float(sizeOf(shortfall)), f64(app, kNoBrakeUnder));
        },
        4);
    return args.eax;
}

/* Whether car goes against the race's way: its +0x621 0x10, turned over on a race run backwards. */
bool againstRace(App* app, reg32 car)
{
    return (((byte(app, car + kCarAiFlags2) & kAi2Against) != 0 ? 1u : 0u) ^ gameSettings(app).backwards) != 0;
}

// A car's: the force along its nose (+0x4e8), its speed along the road (+0x7c0).
constexpr reg32 kCarForce = 0x4e8, kCarSpeedAlong = 0x7c0;
// More than 1 short of the speed wanted: 1.0 as a float's bits.
constexpr i32 kOneBits = 0x3f800000;

/* 0x4066c0: the computer's car's speed along the road ([+0x7c0], signed by the way it goes, the
 * sign chopped to an integer): more than 1 short of the speed it wants ([+0x7b0]) -- the
 * shortfall's bits compared as an integer, as the code does: no negative float is past, any NaN
 * with its sign clear is -- its pull (0x4064f0) into [+0x4e8] and a step's worth of it added, no
 * further than the speed wanted; else its brake (brakeOf, the shortfall handed on), negated, the
 * same way; its size to [+0x7c4], whose bits the code leaves in eax.  nullopt when the game
 * stopped. */
std::optional<reg32> runAlongRoad(App* app, CPU& cpu, reg32 car)
{
    const auto chopped = chop(app, cpu, Float(againstRace(app, car) ? -1.0 : 1.0));
    if (!chopped)
        return std::nullopt;
    const i32 sign = fp::toInteger<i32>(*chopped);
    const Float signF = Float(float(Float(sign)));
    const float shortfall = float(fp::sub(fp::mul(f32(app, car + kCarSpeedWanted), signF),
                                          fp::mul(f32(app, car + kCarSpeedAlong), signF)));
    const bool pulling = i32(floatBits(shortfall)) > kOneBits;
    Float force;
    if (pulling)
    {
        const auto pull = pullOf(app, cpu, car);
        if (!pull)
            return std::nullopt;
        force = *pull;
    }
    else
    {
        const auto brake = brakeOf(app, cpu, car, shortfall);
        if (!brake)
            return std::nullopt;
        force = -Float(*brake);
    }
    put(app, car + kCarForce, floatBits(float(force)));
    put(app, car + kCarSpeedAlong,
        floatBits(float(fp::add(fp::mul(fp::mul(signF, f32(app, car + kCarForce)), f32(app, kStep)),
                                f32(app, car + kCarSpeedAlong)))));
    // No further than the speed wanted, the way it goes; the code copies the bits.
    const bool notOver = belowOrEqual(f32(app, car + kCarSpeedAlong), f32(app, car + kCarSpeedWanted));
    const bool keep = (sign > 0) == pulling ? notOver : !notOver;
    put(app, car + kCarSpeedAlong, at(app, car + (keep ? kCarSpeedAlong : kCarSpeedWanted)));
    const reg32 size = floatBits(sizeOf(app->getMemory<float>(car + kCarSpeedAlong)));
    put(app, car + kCarSpeedSize, size);
    return size;
}

/* Car eax. */
x86::reg32 speedAlongRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return runAlongRoad(app, cpu, car); });
    return args.eax;
}

// The lane's error's scale (a double).
constexpr reg32 kLaneErrorScale = 0x536284;

/* 0x406880: how far car is from the lane's offset it wants ([+0x7b4] - [+0x7b8]), scaled. */
Float laneErrorOf(App* app, reg32 car)
{
    return fp::mul(fp::sub(f32(app, car + kCarLaneWanted), f32(app, car + kCarLaneOffset)),
                   f64(app, kLaneErrorScale));
}

/* Car eax; the error onto the x87's stack. */
x86::reg32 laneError(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(laneErrorOf(app, args.eax));
    cpu.esp += 4;
    return args.eax;
}

// A car's: its spin (+0xe4), a ghost's way (+0x621 0x10).
constexpr reg32 kCarSpin = 0xe4;
// The car's way against the race's, handed on as it glides.
constexpr reg32 kGlidePushed = 0x18, kGlideFrame = 0x20;

/* 0x4068a0: the computer's car glides along the road, its speed set rather than driven (when a
 * ghost: [+0x88]): its speed along the road (runAlongRoad), its speed's vector the nose by it plus
 * the road's across by the lane's error (in the code's frame, 0x4e0050, 0x4dffb0), no spin, its
 * forward speed [+0x4f4]; then 0x4a0980 with its way against the race's.  The eax that leaves;
 * nullopt when the game stopped. */
std::optional<reg32> glide(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, kGlidePushed, kGlideFrame);
    const float error = float(laneErrorOf(app, car));
    if (!runAlongRoad(app, cpu, car))
        return std::nullopt;
    copy3(app, car + kCarRoadAxes, frame - 0x14);
    copy3(app, car + kCarNose, frame - 0x20);
    if (!scaleVector(app, cpu, frame - 0x14, error, frame - 0x14))
        return std::nullopt;
    const float speed = app->getMemory<float>(car + kCarSpeedSize);
    if (!scaleVector(app, cpu, frame - 0x20, speed, frame - 0x20)
        || !addVectors(app, cpu, frame - 0x20, frame - 0x14, car + kCarSpeed))
        return std::nullopt;
    put(app, car + kCarSpin, 0);
    put(app, car + kCarSpin + 4, 0);
    put(app, car + kCarSpin + 8, 0);
    put(app, car + kCarForwardSpeed, floatBits(float(f32(app, car + kCarSpeedAlong))));
    const std::optional<port::conv::Left> left =
        guest::direct<0x4a0980, port::conv::frameFromRoad>(app, cpu, car, againstRace(app, car) ? 1u : 0u);
    if (!left)
        return std::nullopt;
    return left->eax;
}

/* Car eax; eax as 0x4a0980 leaves it. */
x86::reg32 glideAlongRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return glide(app, cpu, car); });
    return args.eax;
}

/* 0x406980: the slice car looks ahead to: the slice its pace gives -- that one on the track's
 * kind 0 between slices 0x225 and 0x235 -- else from 4 slices on (back, against the race's way),
 * on while the road keeps straight (each slice's curve against the car's past 0.7 in size) up to
 * it.  nullopt when the game stopped. */
std::optional<reg32> lookAhead(App* app, CPU& cpu, reg32 car)
{
    const i32 step = againstRace(app, car) ? -1 : 1;
    const reg32 slice = at(app, car + kCarSlice);
    const auto paced = sliceByPace(app, cpu, car);
    if (!paced)
        return std::nullopt;
    if (gameSettings(app).track == 0 && i32(slice) >= kPaceFirstSlice && i32(slice) <= kPaceLastSlice)
        return *paced;
    // A slice on by `by`, wrapped round the track.
    const auto on = [&](reg32 from, i32 by) {
        const reg32 to = from + reg32(by);
        if (by >= 0)
            return i32(to) >= i32(at(app, kSliceCount)) ? to - at(app, kSliceCount) : to;
        return i32(to) < 0 ? to + at(app, kSliceCount) : to;
    };
    reg32 ahead = on(slice, step * 4);
    float curve = 1.0f;
    for (;;)
    {
        const auto gap = sliceGap(app, cpu, ahead, *paced);
        if (!gap)
            return std::nullopt;
        if (*gap == 0)
            return ahead;
        // The curve's size the code's way: 0 at most (or a NaN) it is kept, else negated.
        const float size = belowOrEqual(Float(0.0), Float(curve)) ? curve : float(-Float(curve));
        if (belowOrEqual(Float(size), f64(app, kStraightCurve)))
            return ahead;
        ahead = on(ahead, step);
        const auto next = sliceCurve(app, cpu, slice, ahead);
        if (!next)
            return std::nullopt;
        curve = float(*next);
    }
}

/* Car eax; eax the slice. */
x86::reg32 lookAheadSlice(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return lookAhead(app, cpu, car); });
    return args.eax;
}

// The slice's right's scale (a double, 1/65536).
constexpr reg32 kRightScale = 0x536294;
// The pushes of sidewaysToRoad's code and its frame.
constexpr reg32 kSidewaysPushed = 0xc, kSidewaysFrame = 0x10;

/* 0x406a80: car's speed across the road two slices on: its speed's vector (+0xa4) dotted with the
 * slice's right (by 512 and 1/65536), the right in the code's frame (handed to the dot product).
 * nullopt when the game stopped. */
std::optional<DotLeft> sidewaysSpeed(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, kSidewaysPushed, kSidewaysFrame);
    const auto slice = sliceAhead(app, cpu, car, 2);
    if (!slice)
        return std::nullopt;
    const reg32 right = at(app, kSlices) + *slice * kSliceSize + kSliceRight;
    const auto axis = [&](reg32 i) { return Float(i32(std::int8_t(byte(app, right + i))) * 512); };
    const Float scale = f64(app, kRightScale);
    put(app, frame - 0x10, floatBits(float(fp::mul(axis(0), scale))));
    put(app, frame - 0xc, floatBits(float(fp::mul(axis(1), scale))));
    put(app, frame - 8, floatBits(float(fp::mul(scale, axis(2)))));
    // By address: sidewaysToRoad leaves the eax the dot product leaves.
    return dotLeavingEax(app, cpu, car + kCarSpeed, frame - 0x10);
}

/* Car eax; the speed onto the x87's stack, eax as the dot product leaves it. */
x86::reg32 sidewaysToRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] {
        const auto speed = sidewaysSpeed(app, cpu, car);
        if (!speed)
            return false;
        args.eax = speed->eax;
        args.floatSet = true;
        args.floatResult = double(speed->product);
        return true;
    });
    return args.eax;
}

/* The pushes of the ways' code (ebx, ecx, edx, edi, ebp), the frames below them. */
constexpr reg32 kWayPushed = 0x14, kTowardTargetFrame = 0x10, kTowardAheadFrame = 0x1c, kTowardSliceFrame = 0x2c,
                kTowardSlowingFrame = 0x30;

/* A way's thunk: car eax, the way to [esi].  The code leaves eax the way's address and esi past
 * its own copy in its frame (`copied` under the pushes), the registers it pushed as they were (a
 * callee's no matter).  The x86's direction flag is clear, as Watcom's code keeps it: movsd
 * steps up. */
template <bool (*Way)(App*, CPU&, reg32, reg32)>
x86::reg32 wayThunk(App* app, CPU& cpu, x86::Args& args, reg32 copied)
{
    const reg32 car = args.eax, out = args.esi, esp = cpu.esp;
    const bool went = asThunk(cpu, args, [&]() -> std::optional<reg32> {
        if (!Way(app, cpu, car, out))
            return std::nullopt;
        return out;
    });
    if (went)
        args.esi = esp - kWayPushed - copied + 12;
    return args.eax;
}

/* 0x406b00: the way from car to the car it chases ([+0x624]), to `out`; whose offset it takes up
 * ([+0x7c8]).  The way is worked out in the code's frame.  False when the game stopped. */
bool wayToChased(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kTowardTargetFrame);
    const reg32 chased = at(app, car + kCarChased);
    if (!subtractVectors(app, cpu, chased + kCarPlace, car + kCarPlace, frame - 0x10))
        return false;
    put(app, car + kCarEasedOffset, at(app, at(app, car + kCarChased) + kCarLaneOffset));
    copy3(app, frame - 0x10, out);
    return true;
}

x86::reg32 towardTarget(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToChased>(app, cpu, args, kTowardTargetFrame);
}

// The share of the chased car's speed it is ahead by (a float, 0.4).
constexpr reg32 kAheadShare = 0x53629c;

/* 0x406b60: the way from car to where the car it chases will be (its place plus 0.4 of its
 * speed), to `out`; its offset taken up.  False when the game stopped. */
bool wayToChasedAhead(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kTowardAheadFrame);
    const reg32 chased = at(app, car + kCarChased);
    const Float share = f32(app, kAheadShare);
    put(app, frame - 0x10, floatBits(float(fp::mul(f32(app, chased + kCarSpeed), share))));
    put(app, frame - 0xc, floatBits(float(fp::mul(f32(app, chased + kCarSpeed + 4), share))));
    put(app, frame - 8, floatBits(float(fp::mul(share, f32(app, chased + kCarSpeed + 8)))));
    if (!addVectors(app, cpu, chased + kCarPlace, frame - 0x10, frame - 0x10)
        || !subtractVectors(app, cpu, frame - 0x10, car + kCarPlace, frame - 0x1c))
        return false;
    put(app, car + kCarEasedOffset, at(app, at(app, car + kCarChased) + kCarLaneOffset));
    copy3(app, frame - 0x1c, out);
    return true;
}

x86::reg32 towardTargetAhead(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToChasedAhead>(app, cpu, args, kTowardAheadFrame);
}

/* A slice's point (16.16 integers at the slice's record) by `scale` into `to`, three floats --
 * x and y by the scale, the scale by z, as the code's x87 has them. */
void slicePointTo(App* app, reg32 slice, reg32 scale, reg32 to)
{
    const reg32 point = at(app, kSlices) + slice * kSliceSize;
    const Float by = f64(app, scale);
    const auto axis = [&](reg32 i) { return Float(sreg32(at(app, point + i))); };
    const float x = float(fp::mul(axis(0), by)), y = float(fp::mul(axis(4), by)), z = float(fp::mul(by, axis(8)));
    put(app, to, floatBits(x));
    put(app, to + 4, floatBits(y));
    put(app, to + 8, floatBits(z));
}

// The slices' points' scales (doubles), and the speed by the way's length (a double, 32).
constexpr reg32 kSlicePointScale = 0x5362a4, kSlowingPointScale = 0x5362ac, kSpeedByWay = 0x5362b4;

/* 0x406c00: the way from car to its slice's point, to `out`.  False when the game stopped. */
bool wayToSlice(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kTowardSliceFrame);
    slicePointTo(app, at(app, car + kCarSlice), kSlicePointScale, frame - 0x20);
    if (!subtractVectors(app, cpu, frame - 0x20, car + kCarPlace, frame - 0x2c))
        return false;
    copy3(app, frame - 0x2c, out);
    return true;
}

x86::reg32 towardSlice(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToSlice>(app, cpu, args, kTowardSliceFrame);
}

/* 0x406c90: the way from car to its slice's point, to `out`; the speed it wants ([+0x7b0]) 32 by
 * the way's length (0x4e06a0): it slows as it comes.  False when the game stopped. */
bool wayToSliceSlowing(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kTowardSlowingFrame);
    slicePointTo(app, at(app, car + kCarSlice), kSlowingPointScale, frame - 0x24);
    if (!subtractVectors(app, cpu, frame - 0x24, car + kCarPlace, frame - 0x30))
        return false;
    const auto way = length(app, cpu, frame - 0x30);
    if (!way)
        return false;
    put(app, car + kCarSpeedWanted, floatBits(float(fp::mul(*way, f64(app, kSpeedByWay)))));
    copy3(app, frame - 0x30, out);
    return true;
}

x86::reg32 towardSliceSlowing(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToSliceSlowing>(app, cpu, args, kTowardSlowingFrame);
}

// The lane's point's scale (a double), the slices where the track's kind 0 keeps left, and by
// how much (-10 as a float's bits).
constexpr reg32 kLanePointScale = 0x5362bc, kKeepLeftBits = 0xc1200000;
constexpr i32 kKeepLeftFirst = 0x205, kKeepLeftLast = 0x20f;
constexpr reg32 kTowardLaneFrame = 0x4c;

/* 0x406d40: the way from car to its lane at the slice it looks ahead to (lookAhead), to `out`:
 * the slice's point plus the road's across by the offset it eases to (10 more to the left on the
 * track's kind 0 between slices 0x205 and 0x20f, keeping left).  False when the game stopped. */
bool wayToLane(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kTowardLaneFrame);
    const auto slice = lookAhead(app, cpu, car);
    if (!slice)
        return false;
    reg32 keepLeft = 0;
    if (i32(*slice) <= kKeepLeftLast && i32(*slice) >= kKeepLeftFirst && gameSettings(app).track == 0
        && !belowOrEqual(Float(0.0), f32(app, car + kCarEasedOffset)))
        keepLeft = kKeepLeftBits;
    slicePointTo(app, *slice, kLanePointScale, frame - 0x28);
    copy3(app, car + kCarRoadAxes, frame - 0x1c);
    float keepLeftF;
    std::memcpy(&keepLeftF, &keepLeft, sizeof keepLeftF);
    const float offset = float(fp::add(f32(app, car + kCarEasedOffset), Float(keepLeftF)));
    if (!scaleVector(app, cpu, frame - 0x1c, offset, frame - 0x40)
        || !addVectors(app, cpu, frame - 0x28, frame - 0x40, frame - 0x40)
        || !subtractVectors(app, cpu, frame - 0x40, car + kCarPlace, frame - 0x4c))
        return false;
    copy3(app, frame - 0x4c, out);
    return true;
}

x86::reg32 towardLane(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToLane>(app, cpu, args, kTowardLaneFrame);
}

// A car's: a cop's flags' second byte (+0x711: 2 leading), how long it has been stuck (+0x7d8).
// +0x621's: riding with the car chased, going for it, slowing to its slice.  How near a cop leads
// (slices).  Stuck under this speed (a double, 3), unstuck past that (a double, 10); put back
// past 1000 frames stuck or 8 reverses, the slice's point after 4 or every 0x40 frames stuck.
// Whether a car is off its way (0x40f110).  The lanes a slice has (+0x13: the low nibble right,
// the high left).
constexpr reg32 kCarCopFlags2 = 0x711, kCarStuck = 0x7d8;
constexpr reg8 kCopLeading = 2, kAi2RidesWith = 0x80, kAi2GoesFor = 0x20, kAi2Slowing = 8;
constexpr i32 kLeadSlices = 4;
constexpr reg32 kStuckUnder = 0x5362c4, kUnstuckPast = 0x5362cc;
constexpr i32 kStuckFramesOut = 0x3e8, kReversesOut = 8, kReversesToSlice = 4;
constexpr reg8 kStuckToSlice = 0x40;
constexpr reg32 kWayToGoFrame = 0x10;

/* 0x406e50: the way car goes, to `out`, each way worked out into the code's frame and copied.
 * Chasing a car ([+0x624]): straight at it where it rides with it ([+0x621] 0x80), where it leads
 * it (a cop with [+0x711] 2 within 4 slices: where it will be), or where [+0x621] 0x20 says; else
 * its lane.  Chasing none: its slice's point slowing where [+0x621] 8 says; where 0x40f110 says it
 * is off its way, stuck time counted ([+0x7d8], while slower than 3) -- its slice's point every
 * 0x40 of it, its lane else, put back on the road past 1000; else, not stuck: put back after 8
 * reverses ([+0x658], cleared past 10 of speed), its slice's point after 4 where its lane is past
 * the road's lanes, its lane else.  False when the game stopped. */
bool wayToGoFor(App* app, CPU& cpu, reg32 car, reg32 out)
{
    const GuestFrame frame(cpu, kWayPushed, kWayToGoFrame);
    // A way into the frame, called at the code's depth (its return address's slot).
    const auto way = [&](bool (*toward)(App*, CPU&, reg32, reg32)) {
        cpu.esp -= 4;
        const bool going = toward(app, cpu, car, frame - 0x10);
        cpu.esp += 4;
        return going;
    };
    bool going = true;
    if (at(app, car + kCarChased) != 0)
    {
        if ((byte(app, car + kCarAiFlags2) & kAi2RidesWith) != 0)
            going = way(wayToChased);
        else
        {
            bool led = false;
            if ((byte(app, car + kCarKind) & kKindCop) != 0 && (byte(app, car + kCarCopFlags2) & kCopLeading) != 0)
            {
                const auto first = sliceGap(app, cpu, at(app, car + kCarSlice), at(app, at(app, car + kCarChased) + kCarSlice));
                if (!first)
                    return false;
                const auto gap = sliceGap(app, cpu, at(app, car + kCarSlice), at(app, at(app, car + kCarChased) + kCarSlice));
                if (!gap)
                    return false;
                const reg32 size = *first > 0 ? reg32(*gap) : reg32(0) - reg32(*gap);
                if (i32(size) < kLeadSlices)
                {
                    led = true;
                    going = way(wayToChasedAhead);
                }
            }
            if (!led)
                going = way((byte(app, car + kCarAiFlags2) & kAi2GoesFor) != 0 ? wayToChased : wayToLane);
        }
    }
    else if ((byte(app, car + kCarAiFlags2) & kAi2Slowing) != 0)
        going = way(wayToSliceSlowing);
    else
    {
        const bool off = guest::direct<0x40f110, aiplan::boxedIn>(app, cpu, car);
        if (cpu.terminate)
            return false;
        if (off)
        {
            if (below(Float(sizeOf(app->getMemory<float>(car + kCarForwardSpeed))), f64(app, kStuckUnder)))
                put(app, car + kCarStuck, at(app, car + kCarStuck) + 1);
            if (!way((byte(app, car + kCarStuck) & kStuckToSlice) != 0 ? wayToSlice : wayToLane))
                return false;
            if (i32(at(app, car + kCarStuck)) > kStuckFramesOut)
            {
                if (!guest::direct<0x429100, racestart::resetCarOf>(app, cpu, car))
                    return false;
                put(app, car + kCarStuck, 0);
            }
        }
        else
        {
            const Float speed = f32(app, car + kCarSpeedSize);
            put(app, car + kCarStuck, 0);
            bool lane = true;
            if (above(speed, f64(app, kUnstuckPast)))
                put(app, car + kCarReverses, 0);
            else if (i32(at(app, car + kCarReverses)) > kReversesOut)
            {
                if (!guest::direct<0x429100, racestart::resetCarOf>(app, cpu, car))
                    return false;
                put(app, car + kCarReverses, 0);
                lane = false;
            }
            else if (i32(at(app, car + kCarReverses)) > kReversesToSlice)
            {
                const reg32 lanes = byte(app, at(app, kSlices) + at(app, car + kCarSlice) * kSliceSize + kSliceLanes);
                const i32 mine = i32(at(app, car + kCarLane));
                if (6 - i32((lanes >> 4) & 0xf) > mine || i32(lanes & 0xf) + 7 < mine)
                {
                    if (!way(wayToSlice))
                        return false;
                    lane = false;
                }
            }
            if (lane)
                going = way(wayToLane);
        }
    }
    if (!going)
        return false;
    copy3(app, frame - 0x10, out);
    return true;
}

/* Car eax, the way to [esi]. */
x86::reg32 wayToGo(App* app, x86::CPU& cpu, x86::Args& args)
{
    return wayThunk<wayToGoFor>(app, cpu, args, kWayToGoFrame);
}

// A car's: the force in its axes (+0x4e0), the step's acceleration (+0x2e8) and spin's change
// (+0x2f4), both in the world's.
constexpr reg32 kCarAxesForce = 0x4e0, kCarAcceleration = 0x2e8, kCarSpinChange = 0x2f4;
constexpr reg32 kIntegratePushed = 0x10, kIntegrateFrame = 0x58;

/* 0x4e0110 (simd's): the matrix at `matrix` transposed to `to`; false when the game stopped. */
bool transpose(App* app, CPU& cpu, reg32 matrix, reg32 to)
{
    return feCore<0x4e0110, frontend::simd::transpose>(app, cpu, matrix, to).has_value();
}

/* 0x4071a0: the computer's car's step integrated: the force in its axes (+0x4e0) and the push
 * handed on ([kPush], in the road's axes) into the world's (+0x2e8, through the axes transposed in
 * the code's frame), the spin's change (+0x2f4); its speed by a step of the acceleration, damped
 * ([kDamping], the substeps' times over), its spin by a step of the spin's change; each product
 * and sum in the code's order of operands.  What the code leaves in eax -- the substeps damped,
 * or the spin's change's last bits; nullopt when the game stopped. */
std::optional<reg32> integrateStep(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, kIntegratePushed, kIntegrateFrame);
    const reg32 axes = frame - 0x58, roadAxes = frame - 0x34;
    if (!transpose(app, cpu, car + kCarAxes, axes) || !transpose(app, cpu, car + kCarRoadAxes, roadAxes))
        return std::nullopt;
    const reg32 acceleration = car + kCarAcceleration;
    for (reg32 i = 0; i < 3; ++i)
    {
        const auto along = dot(app, cpu, car + kCarAxesForce, axes + 0xc * i);
        if (!along)
            return std::nullopt;
        put(app, acceleration + 4 * i, floatBits(float(*along)));
    }
    for (reg32 i = 0; i < 3; ++i)
    {
        const auto pushed = dot(app, cpu, kPush, roadAxes + 0xc * i);
        if (!pushed)
            return std::nullopt;
        put(app, acceleration + 4 * i, floatBits(float(fp::add(*pushed, f32(app, acceleration + 4 * i)))));
    }
    Float spin[3];
    for (reg32 i = 0; i < 3; ++i)
    {
        const auto turned = dot(app, cpu, car + kCarSpinChange, axes + 0xc * i);
        if (!turned)
            return std::nullopt;
        spin[i] = *turned;
    }
    const Float step = f32(app, kStep);
    const Float a0 = fp::mul(f32(app, acceleration), step), a1 = fp::mul(f32(app, acceleration + 4), step),
                a2 = fp::mul(step, f32(app, acceleration + 8));
    const reg32 spinChange = car + kCarSpinChange;
    put(app, spinChange, floatBits(float(spin[0])));
    put(app, spinChange + 4, floatBits(float(spin[1])));
    reg32 left = floatBits(float(spin[2]));
    put(app, spinChange + 8, left);
    const reg32 speed = car + kCarSpeed;
    const float vx = float(fp::add(a0, f32(app, speed))), vy = float(fp::add(a1, f32(app, speed + 4))),
                vz = float(fp::add(a2, f32(app, speed + 8)));
    put(app, speed, floatBits(vx));
    put(app, speed + 4, floatBits(vy));
    put(app, speed + 8, floatBits(vz));
    // Damped the substeps' times over, its bits not 0 (either sign).
    if ((at(app, kDamping) & 0x7fffffff) != 0)
    {
        const Float damping = f32(app, kDamping);
        for (left = 0; i32(left) < i32(at(app, kSubsteps)); ++left)
        {
            const float x = float(fp::mul(f32(app, speed), damping)), y = float(fp::mul(f32(app, speed + 4), damping)),
                        z = float(fp::mul(damping, f32(app, speed + 8)));
            put(app, speed + 4, floatBits(y));
            put(app, speed + 8, floatBits(z));
            put(app, speed, floatBits(x));
        }
    }
    const Float w0 = fp::mul(f32(app, spinChange), step), w1 = fp::mul(f32(app, spinChange + 4), step),
                w2 = fp::mul(step, f32(app, spinChange + 8));
    const reg32 spinning = car + kCarSpin;
    const float sx = float(fp::add(w0, f32(app, spinning))), sy = float(fp::add(w1, f32(app, spinning + 4))),
                sz = float(fp::add(w2, f32(app, spinning + 8)));
    put(app, spinning + 4, floatBits(sy));
    put(app, spinning + 8, floatBits(sz));
    put(app, spinning, floatBits(sx));
    return left;
}

/* Car eax. */
x86::reg32 integrate(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return integrateStep(app, cpu, car); });
    return args.eax;
}

/* The size of a float the code's other way: fldz; fcomp; ja -- the value itself where 0 is at
 * most it (or it is a NaN), else negated: -0 stays -0, where sizeOf makes it 0. */
float sizeBelow(float value)
{
    return belowOrEqual(Float(0.0), Float(value)) ? value : float(-Float(value));
}

/* The product the code inlines where a vector goes through a matrix: the vector at `v` times the
 * matrix at `m`, to `to` -- each (v0 m[j] + v1 m[3 + j]) + v2 m[6 + j], everything read before
 * the stores, as the code's x87 has it. */
void transformInto(App* app, reg32 v, reg32 m, reg32 to)
{
    float out[3];
    for (reg32 j = 0; j < 3; ++j)
        out[j] = float(fp::add(fp::add(fp::mul(f32(app, v), f32(app, m + 4 * j)),
                                       fp::mul(f32(app, v + 4), f32(app, m + 0xc + 4 * j))),
                               fp::mul(f32(app, v + 8), f32(app, m + 0x18 + 4 * j))));
    for (reg32 j = 0; j < 3; ++j)
        put(app, to + 4 * j, floatBits(out[j]));
}

// A car's: its grip (+0xb8), its speed in its axes (+0x4ec across, +0x4f4 forward) and spin
// (+0x4f8, +0x4fc its yaw), its skid (+0x5c0, +0x5c4), the throttle and its two flags (+0x508,
// +0x50c, +0x50d), the steering (+0x51c), its limiter (+0x594), its slip (+0x5b0).
constexpr reg32 kCarGrip = 0xb8, kCarAcross = 0x4ec, kCarSpinInAxes = 0x4f8, kCarYaw = 0x4fc, kCarSkid = 0x5c0,
                kCarSkidAhead = 0x5c4, kCarThrottle = 0x508, kCarShifting = 0x50c, kCarClutch = 0x50d,
                kCarSteering = 0x51c, kCarLimiter = 0x594, kCarSlip = 0x5b0;
// Coasting: upright from 0.9 (a float's bits, compared as an integer as the code does: no negative
// float, any NaN with its sign clear); the grip under which it is slippery (a double, 22.35); the
// forward speed's damping slippery and not, the across's, the yaw's (and the forward's halving),
// between these forward (doubles: 5 and -10), the skid's share of the speed, the yaw past which
// it skids more and the across's cut (doubles).
constexpr i32 kCoastUprightBits = 0x3f666666;
constexpr reg32 kCoastGrip = 0x536304, kCoastSlippery = 0x53630c, kCoastGripping = 0x536314,
                kCoastAcross = 0x53631c, kCoastHalf = 0x536324, kCoastHalvedUnder = 0x53632c,
                kCoastHalvedOver = 0x536334, kCoastSkidShare = 0x53633c, kCoastYawSkid = 0x536344;

/* 0x407380: a finished car coasts (while racing and upright: [+0x10c] at least 0.9): its pedals,
 * steering and skid let go, the brake on, its forward speed damped (0.95, 0.98 with grip past
 * 22.35), across 0.85 and its yaw 0.5, halved again between -10 and 5 forward; a skid of a tenth
 * of its speed, more with its yaw past 0.2, its across where that is the bigger (cut to 0.2); its
 * speed and spin back to the world's through its axes.  Whether it coasted. */
bool coast(App* app, reg32 car)
{
    if (half(app, car + kCarStage) != 0 || i32(at(app, car + kCarUpright)) < kCoastUprightBits)
        return false;
    for (const reg32 field : {kCarSkid, kCarSkidAhead, kCarLimiter, kCarSlip, kCarSteering})
        put(app, car + field, 0);
    for (const reg32 field : {kCarThrottle, kCarShifting, kCarClutch})
        putByte(app, car + field, 0);
    putByte(app, car + kCarBrake, 0xff);
    const bool slippery = below(f32(app, car + kCarGrip), f64(app, kCoastGrip));
    put(app, car + kCarForwardSpeed,
        floatBits(float(fp::mul(f32(app, car + kCarForwardSpeed), f64(app, slippery ? kCoastSlippery : kCoastGripping)))));
    const Float across = fp::mul(f32(app, car + kCarAcross), f64(app, kCoastAcross));
    const Float yaw = fp::mul(f32(app, car + kCarYaw), f64(app, kCoastHalf));
    const Float forward = f32(app, car + kCarForwardSpeed);
    put(app, car + kCarAcross, floatBits(float(across)));
    put(app, car + kCarYaw, floatBits(float(yaw)));
    const bool halved = below(forward, f64(app, kCoastHalvedUnder)) && above(forward, f64(app, kCoastHalvedOver));
    if (halved)
        put(app, car + kCarForwardSpeed, floatBits(float(fp::mul(forward, f64(app, kCoastHalf)))));
    else
        put(app, car + kCarSkidAhead,
            floatBits(float(fp::mul(Float(sizeBelow(app->getMemory<float>(car + kCarForwardSpeed))),
                                    f64(app, kCoastSkidShare)))));
    if (above(Float(sizeBelow(app->getMemory<float>(car + kCarYaw))), f64(app, kCoastYawSkid)))
        put(app, car + kCarSkidAhead,
            floatBits(float(fp::add(f32(app, car + kCarSkidAhead), Float(sizeBelow(app->getMemory<float>(car + kCarYaw)))))));
    const float acrossSize = sizeBelow(app->getMemory<float>(car + kCarAcross));
    if (above(Float(acrossSize), Float(sizeBelow(app->getMemory<float>(car + kCarForwardSpeed)))))
    {
        put(app, car + kCarSkid, floatBits(acrossSize));
        put(app, car + kCarSkidAhead, floatBits(acrossSize));
        put(app, car + kCarAcross, floatBits(float(fp::mul(f32(app, car + kCarAcross), f64(app, kCoastYawSkid)))));
    }
    transformInto(app, car + kCarAcross, car + kCarAxes, car + kCarSpeed);
    transformInto(app, car + kCarSpinInAxes, car + kCarAxes, car + kCarSpin);
    return true;
}

/* Car eax; eax left 0x18 past its speed in its axes where it coasted, as the code's products
 * leave it. */
x86::reg32 coastAfterFinish(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 car = args.eax;
    if (coast(app, car))
        args.eax = car + kCarAcross + 0x18;
    cpu.esp += 4;
    return args.eax;
}

/* The clamps the steering's code inlines (fst; fcomp; jbe and the like): the x87's value at most
 * the limit -- its float where it is not over it (or a NaN), else the limit; a float at most a
 * float, the same way; and at least the limit negated, where it is not under it. */
float atMostX87(const Float& value, float limit)
{
    return belowOrEqual(value, Float(limit)) ? float(value) : limit;
}

float atMostF(float value, float limit)
{
    return belowOrEqual(Float(value), Float(limit)) ? value : limit;
}

float atLeastNegated(float value, float limit)
{
    const float negated = float(-Float(limit));
    return belowOrEqual(Float(value), Float(negated)) ? negated : value;
}

/* A float of the game's data. */
float floatAt(App* app, reg32 address)
{
    return app->getMemory<float>(address);
}

// The steering's tuning (setDrivingConstants): the skid's across (7), the lane's gain and limit,
// the push's gains across and along, the heading's gain and limit, the yaw's limit and gain, the
// speed under which the turn is cut further (22.35), and by how much the lane's and the heading's.
constexpr reg32 kSkidAcross = 0x56ebd0, kLaneGain = 0x56ebd4, kLaneLimit = 0x56ebd8, kPushAcrossGain = 0x56ebdc,
                kPushAlongGain = 0x56ebe0, kHeadingGain = 0x56ebe4, kHeadingLimit = 0x56ebe8, kYawLimit = 0x56ebec,
                kYawGain = 0x56ebf0, kSlowTurn = 0x56ebf4, kSlowLaneBySpeed = 0x56ebf8, kSlowHeadingBySpeed = 0x56ebfc;
// The game's constants: a half (a double), the gains' growth with the forward speed (a float,
// 0.0015), the speed past which the turn drags (a double, 44.7), the racer's drag over its
// performance's (a double), the skid's share (a double).  A car's pull's share (+0x7bc), its
// performance's drag (+0x248).
constexpr reg32 kHalf = 0x53634c, kGainGrowth = 0x536354, kDragSpeed = 0x53635c, kRacerDrag = 0x536364,
                kSkidShare = 0x53636c;
constexpr reg32 kCarPullShare = 0x7bc, kPerformanceDrag = 0x248;
constexpr reg32 kSteerPushed = 0x18, kSteerOnRoadFrame = 0xfc;

/* A core of this file called as the code calls it: at its depth, the return address's slot below
 * esp (a callee of it is handed an address in its frame). */
template <class Core>
auto atDepth(CPU& cpu, Core&& core)
{
    cpu.esp -= 4;
    auto result = core();
    cpu.esp += 4;
    return result;
}

/* 0x4076b0: the computer's car steered on the road.  The yaw it wants: its heading's error
 * (aiHeading's, [kHeadingError]) by a gain, cut to a limit, plus its lane's error ([+0x7b8]
 * against the eased [+0x7c8]) by a gain, cut to a limit and to half its grip, cut further by its
 * speed under 22.35; its yaw's error ([+0x4fc]) by a gain and a limit: the spin's change
 * (+0x2f8).  The push sideways: its speed across the road (sidewaysSpeed) less the yaw wanted, by
 * the way it goes, cut to gains scaled where it pulls hard ([+0x7bc] past 1).  Along: its pull
 * (0x4064f0) or brake (brakeOf) by whether it wants more speed, less the turn's drag (its forward
 * speed past 44.7: the push by the speed's half, its performance's +0x248 for a racer), by the way
 * it goes; a skid past 7 across.  The push to [kPush] (in the road's axes), the forces in its own
 * axes cleared.  Each product and sum in the code's order of operands, a float where it stores
 * one.  The spin's change's bits (what the code leaves in eax); nullopt when the game stopped. */
std::optional<reg32> steerOn(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, kSteerPushed, kSteerOnRoadFrame);
    const i32 way = againstRace(app, car) ? -1 : 1;
    const auto sideways = atDepth(cpu, [&] { return sidewaysSpeed(app, cpu, car); });
    if (!sideways)
        return std::nullopt;
    const float across = float(sideways->product);
    const float one = 1.0f;

    // The yaw wanted: the heading's, cut.
    float yawWanted = atLeastNegated(
        atMostX87(fp::mul(-f32(app, kHeadingError), f32(app, kHeadingGain)), floatAt(app, kHeadingLimit)),
        floatAt(app, kHeadingLimit));
    // The lane's, cut to its limit and to half the grip.
    const float laneLimit = floatAt(app, kLaneLimit);
    float lane = atLeastNegated(
        atMostX87(fp::mul(-fp::sub(f32(app, car + kCarLaneOffset), f32(app, car + kCarEasedOffset)),
                          f32(app, kLaneGain)),
                  laneLimit),
        laneLimit);
    const float grip = float(fp::mul(f32(app, car + kCarGrip), f64(app, kHalf)));
    lane = atLeastNegated(atMostF(lane, grip), grip);
    // Both cut further by the speed where it is slow.
    if (below(f32(app, car + kCarSpeedSize), f32(app, kSlowTurn)))
    {
        const float byLane = float(fp::mul(f32(app, car + kCarSpeedSize), f32(app, kSlowLaneBySpeed)));
        lane = atLeastNegated(atMostF(lane, byLane), byLane);
        const float byHeading = float(fp::mul(f32(app, car + kCarSpeedSize), f32(app, kSlowHeadingBySpeed)));
        yawWanted = atLeastNegated(atMostF(yawWanted, byHeading), byHeading);
    }
    // The spin's change: the yaw's error by its gain and limit.
    const float yawLimit = floatAt(app, kYawLimit);
    const float turn = atLeastNegated(
        atMostX87(fp::mul(-fp::sub(f32(app, car + kCarYaw), Float(yawWanted)), f32(app, kYawGain)), yawLimit),
        yawLimit);

    // The push's gains (its limit, the across's; its gain, the along's) by its forward speed by
    // 0.0015 plus 1, scaled by its pull's share where that is past 1.
    const Float growth = fp::add(fp::mul(f32(app, car + kCarForwardSpeed), f32(app, kGainGrowth)), Float(1.0));
    float pushLimit = float(fp::mul(f32(app, kPushAcrossGain), growth));
    float pushGain = float(fp::mul(growth, f32(app, kPushAlongGain)));
    const Float share = f32(app, car + kCarPullShare);
    if (below(Float(1.0), share))
    {
        const float gain = float(fp::mul(share, Float(pushGain)));
        pushLimit = float(fp::mul(Float(pushLimit), share));
        pushGain = gain;
    }
    // The push across: the speed across less the lane's yaw, against the way it goes.
    const float push = atLeastNegated(
        atMostX87(fp::mul(fp::mul(fp::sub(Float(across), Float(lane)), Float(-way)), Float(pushGain)), pushLimit),
        pushLimit);
    const Float wayF = Float(way);
    const Float pushed = fp::mul(Float(push), Float(one));
    const float shortfall = float(fp::sub(fp::mul(f32(app, car + kCarSpeedWanted), wayF),
                                          fp::mul(wayF, f32(app, car + kCarSpeedAlong))));
    const float acrossPush = float(pushed);

    // Along: the pull, or the brake against it.
    Float force;
    if (below(Float(0.0), Float(shortfall)))
    {
        const auto pull = pullOf(app, cpu, car);
        if (!pull)
            return std::nullopt;
        force = *pull;
    }
    else
    {
        const auto brake = brakeOf(app, cpu, car, shortfall);
        if (!brake)
            return std::nullopt;
        force = -Float(*brake);
    }
    float along = float(force);
    const bool braking = above(Float(0.0), Float(along));
    // The turn's drag: the push across over half the forward speed (its size the code's way, in
    // doubles), its sign by the way the push goes; for a racer its performance's own.
    const Float forward = f32(app, car + kCarForwardSpeed);
    if (above(forward, Float(sizeBelow(floatAt(app, car + kCarAcross)))) && below(Float(0.0), Float(along))
        && above(forward, f64(app, kDragSpeed)))
    {
        const auto overHalf = [&] {
            const Float half = fp::mul(Float(double(floatAt(app, car + kCarForwardSpeed))), f64(app, kHalf));
            return fp::div(Float(acrossPush), belowOrEqual(Float(0.0), half) ? half : -half);
        };
        const Float drag = above(Float(0.0), overHalf()) ? -overHalf() : overHalf();
        float dragF = float(drag);
        if ((byte(app, car + kCarKind) & kKindRacer) != 0)
        {
            const reg32 performance = at(app, car + kCarPerformance);
            if ((at(app, performance + kPerformanceDrag) & 0x7fffffff) != 0)
                dragF = float(fp::div(f64(app, kRacerDrag), f32(app, performance + kPerformanceDrag)));
        }
        along = float(fp::sub(Float(along), Float(dragF)));
    }
    // Backwards the force along turned over: its sign bit flipped, as the code does.
    if (above(Float(0.0), f32(app, car + kCarForwardSpeed)))
        along = -along;
    along = float(fp::mul(Float(along), Float(one)));

    // A skid past 7 across, more of it braking.
    const Float past = fp::sub(Float(sizeOf(across)), f32(app, kSkidAcross));
    if (below(Float(0.0), past))
    {
        put(app, car + kCarSkid, floatBits(float(fp::mul(past, f64(app, kSkidShare)))));
        put(app, car + kCarSkidAhead, braking ? floatBits(float(fp::mul(past, f64(app, kHalf)))) : 0);
    }
    else
    {
        put(app, car + kCarSkidAhead, 0);
        put(app, car + kCarSkid, 0);
    }
    put(app, car + kCarAxesForce, 0);
    const float pushAcross = float(fp::mul(Float(acrossPush), wayF));
    put(app, car + kCarAxesForce + 4, 0);
    put(app, car + kCarAxesForce + 8, 0);
    put(app, car + kCarSpinChange, 0);
    const float pushAlong = float(fp::mul(wayF, Float(along)));
    put(app, car + kCarSpinChange + 4, floatBits(turn));
    put(app, kPushY, 0);
    put(app, car + kCarSpinChange + 8, 0);
    put(app, kPush, floatBits(pushAcross));
    put(app, kPushZ, floatBits(pushAlong));
    return floatBits(turn);
}

/* Car eax. */
x86::reg32 steerOnRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return steerOn(app, cpu, car); });
    return args.eax;
}

// The backwards steering's tuning (setDrivingConstants): the side's limit, the across's limit and
// gain, the heading's gain and limit, the yaw's limit and gain, the side's and the heading's
// shares of the grip.  The error's turn's scale and the speed wanted's cap (doubles; the cap
// 16.41, as the code's two dwords have it).
constexpr reg32 kBackSideLimit = 0x56ec04, kBackAcrossLimit = 0x56ec08, kBackAcrossGain = 0x56ec0c,
                kBackHeadingGain = 0x56ec10, kBackHeadingLimit = 0x56ec14, kBackYawLimit = 0x56ec18,
                kBackYawGain = 0x56ec1c, kBackSideGrip = 0x56ec24, kBackHeadingGrip = 0x56ec28;
constexpr reg32 kBackTurnScale = 0x536374, kBackSpeedCap = 0x53637c;
constexpr std::uint64_t kBackSpeedCapBits = 0x40306930be0ded29ull;

/* 0x407c30: the computer's car steered backwards (reversing, or moving backwards): its heading's
 * error by a gain and limit, cut to its grip; the sine of the error's turn by its forward speed,
 * cut to a limit and its grip; the yaw's (+0x2f8) and the across's (+0x4e0) errors by gains and
 * limits; along, its pull or brake by the speed wanted (its size, 16.41 at most) against its
 * forward speed -- reversing, against the speed wanted backwards, negated.  The push and the
 * spin's other parts cleared.  The spin's change's bits (what the code leaves in eax); nullopt
 * when the game stopped. */
std::optional<reg32> steerBack(App* app, CPU& cpu, reg32 car)
{
    // The heading's yaw, cut to a share of the grip.
    const float headingLimit = floatAt(app, kBackHeadingLimit);
    float heading = atLeastNegated(
        atMostX87(fp::mul(-f32(app, kHeadingError), f32(app, kBackHeadingGain)), headingLimit), headingLimit);
    const float headingGrip = sizeOf(float(fp::mul(f32(app, car + kCarGrip), f32(app, kBackHeadingGrip))));
    heading = atLeastNegated(atMostF(heading, headingGrip), headingGrip);
    // The side: the sine of the error's turn (by one more where it is under 0) by the forward
    // speed, cut to a limit and a share of the grip.
    const Float error = Float(double(floatAt(app, kHeadingError)));
    const Float sine = above(Float(0.0), error) ? -fp::sin(fp::mul(fp::add(Float(1.0), error), f64(app, kBackTurnScale)))
                                                : fp::sin(fp::mul(error, f64(app, kBackTurnScale)));
    const float sideLimit = floatAt(app, kBackSideLimit);
    float side = atLeastNegated(atMostF(float(fp::mul(sine, f32(app, car + kCarForwardSpeed))), sideLimit), sideLimit);
    const float sideGrip = sizeOf(float(fp::mul(f32(app, car + kCarGrip), f32(app, kBackSideGrip))));
    side = atLeastNegated(atMostF(side, sideGrip), sideGrip);
    // The yaw's and the across's errors by their gains and limits.
    const float yawLimit = floatAt(app, kBackYawLimit);
    const float turn = atLeastNegated(
        atMostX87(fp::mul(-fp::sub(f32(app, car + kCarYaw), Float(heading)), f32(app, kBackYawGain)), yawLimit),
        yawLimit);
    const float acrossLimit = floatAt(app, kBackAcrossLimit);
    const float acrossForce = atLeastNegated(
        atMostX87(fp::mul(-fp::sub(f32(app, car + kCarAcross), Float(side)), f32(app, kBackAcrossGain)), acrossLimit),
        acrossLimit);
    // Along: the speed wanted's size, 16.41 at most.
    const float wanted = sizeOf(floatAt(app, car + kCarSpeedWanted));
    double target = double(wanted);
    if (above(Float(wanted), f64(app, kBackSpeedCap)))
        std::memcpy(&target, &kBackSpeedCapBits, sizeof target);
    const float targetF = float(target);
    float force;
    if (at(app, car + kCarReversing) == 0)
    {
        const Float shortfall = fp::sub(Float(targetF), f32(app, car + kCarForwardSpeed));
        Float along;
        if (below(Float(1.0), shortfall))
        {
            const auto pull = pullOf(app, cpu, car);
            if (!pull)
                return std::nullopt;
            along = *pull;
        }
        else
        {
            const auto brake = brakeOf(app, cpu, car, float(shortfall));
            if (!brake)
                return std::nullopt;
            along = -Float(*brake);
        }
        force = float(along);
    }
    else
    {
        Float along;
        if (below(-Float(targetF), f32(app, car + kCarForwardSpeed)))
        {
            const auto pull = pullOf(app, cpu, car);
            if (!pull)
                return std::nullopt;
            along = *pull;
        }
        else
        {
            const auto brake = brakeOf(app, cpu, car, 0.0f);
            if (!brake)
                return std::nullopt;
            along = -Float(*brake);
        }
        // Its sign bit flipped, as the code does.
        force = -float(along);
    }
    put(app, car + kCarAxesForce + 4, 0);
    put(app, car + kCarAxesForce, floatBits(acrossForce));
    put(app, car + kCarSpinChange, 0);
    put(app, kPush, 0);
    put(app, car + kCarSpinChange + 8, 0);
    put(app, kPushY, 0);
    put(app, car + kCarAxesForce + 8, floatBits(force));
    put(app, kPushZ, 0);
    put(app, car + kCarSpinChange + 4, floatBits(turn));
    return floatBits(turn);
}

/* Car eax. */
x86::reg32 steerReversing(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return steerBack(app, cpu, car); });
    return args.eax;
}

// The off-road steering's tuning (setDrivingConstants): the side's limit, the across's limit and
// gain, the heading's gain and limit, the yaw's limit and gain, the side's and the heading's shares
// of the grip.  The game's constants (doubles): the heading by 10 (and the push along's share
// slipping in a race of few), the yaw's by 0.5 (and the kick), the frame's quarter steps against
// the grip, the push's shares slipping in a race of many.  The performance's pull table's first
// (+0x2ec).  A car's +8 (under 20 for the kick), its surface (+0x18c).  The skid's kick (10 as a
// float's bits).
constexpr reg32 kOffSideLimit = 0x56ec30, kOffAcrossLimit = 0x56ec34, kOffAcrossGain = 0x56ec38,
                kOffHeadingGain = 0x56ec3c, kOffHeadingLimit = 0x56ec40, kOffYawLimit = 0x56ec44,
                kOffYawGain = 0x56ec48, kOffSideGrip = 0x56ec50, kOffHeadingGrip = 0x56ec54;
constexpr reg32 kOffByTen = 0x536384, kOffHalf = 0x53638c, kOffQuarterSteps = 0x536394, kOffSlideMany = 0x53639c,
                kOffSlideTurn = 0x5363a4;
constexpr reg32 kPerformancePullFirst = 0x2ec, kCarSurface = 0x18c, kCarTrackKind = 0x160;
constexpr i32 kKickBefore = 0x14;
constexpr reg32 kSkidKickBits = 0x41200000;
// The grips' table: a row of three track kinds by surface.
constexpr reg32 kSurfaceRow = 12;

/* fcomp's equal as sahf's zero flag reads it: c3, set for equal and for unordered. */
bool equalOrUnordered(const Float& a, const Float& b)
{
    return !(a < b) && !(a > b);
}

/* 0x407fa0: the computer's car steered off the road (or onto it from the side): the damping off
 * ([kDamping] 1), its heading's error by a gain and limit, by 10 cut to its grip; the yaw's and the
 * across's errors by gains and limits; along, its pull (and where [+0x200] 8 asks at the first 20
 * of the race with the pull at its table's first, a random kick now and then for its player) or
 * its brake; a skid past 7 across -- on a slippery surface (its grip under 1 against the frame's
 * quarter steps) and where 0x404610 lets it, the push slipping (by 10 and 50 in a race of many).
 * The spin's change's bits (what the code leaves in eax); nullopt when the game stopped. */
std::optional<reg32> steerOff(App* app, CPU& cpu, reg32 car)
{
    const float scale = 1.0f;
    put(app, kDamping, floatBits(1.0f));
    // The heading's yaw, cut to a share of the grip; by 10 the side's, cut to its limit, the grip
    // and a share of it.
    const float headingLimit = floatAt(app, kOffHeadingLimit);
    const float yaw = atLeastNegated(
        atMostX87(fp::mul(-f32(app, kHeadingError), f32(app, kOffHeadingGain)), headingLimit), headingLimit);
    const float byTen = float(fp::mul(Float(yaw), f64(app, kOffByTen)));
    const float headingGrip = sizeOf(float(fp::mul(f32(app, car + kCarGrip), f32(app, kOffHeadingGrip))));
    const float heading = atLeastNegated(atMostF(yaw, headingGrip), headingGrip);
    const float sideLimit = floatAt(app, kOffSideLimit);
    float side = atLeastNegated(atMostF(byTen, sideLimit), sideLimit);
    side = atLeastNegated(atMostF(side, floatAt(app, car + kCarGrip)), floatAt(app, car + kCarGrip));
    const float sideGrip = sizeOf(float(fp::mul(f32(app, car + kCarGrip), f32(app, kOffSideGrip))));
    side = atLeastNegated(atMostF(side, sideGrip), sideGrip);
    // The yaw's and the across's errors by their gains and limits.
    const float yawLimit = floatAt(app, kOffYawLimit);
    float turn = atLeastNegated(
        atMostX87(fp::mul(fp::mul(-fp::sub(f32(app, car + kCarYaw), Float(heading)), f32(app, kOffYawGain)),
                          f64(app, kOffHalf)),
                  yawLimit),
        yawLimit);
    const float acrossLimit = floatAt(app, kOffAcrossLimit);
    float acrossForce = atLeastNegated(
        atMostX87(fp::mul(-fp::sub(f32(app, car + kCarAcross), Float(side)), f32(app, kOffAcrossGain)), acrossLimit),
        acrossLimit);
    // Along: the pull or the brake, by the speed wanted's size against the forward speed.
    const Float shortfall = fp::sub(Float(sizeOf(floatAt(app, car + kCarSpeedWanted))), f32(app, car + kCarForwardSpeed));
    float force;
    bool kicked = false;
    if (below(Float(1.0), shortfall))
    {
        const auto pull = pullOf(app, cpu, car);
        if (!pull)
            return std::nullopt;
        force = float(*pull);
        if ((byte(app, car + kCarKind) & kKindHarder) != 0 && i32(at(app, car + kCarEight)) < kKickBefore
            && equalOrUnordered(Float(force), f32(app, at(app, car + kCarPerformance) + kPerformancePullFirst)))
        {
            // The game's random numbers: a 1 in 7 share of the 16 bits against the car's player.
            const reg32 product = at(app, kSeed) * at(app, kMultiplier);
            put(app, kProduct, product);
            put(app, kSeed, product & 0xffff);
            const reg32 twice = ((product & 0xffff00) >> 8) * 2;
            if (((twice * 8 - twice) >> 16) == at(app, car + kCarPlayer))
            {
                const Float half = f64(app, kOffHalf);
                const Float pushed = fp::add(Float(acrossForce), half);
                acrossForce = float(pushed);
                turn = float(fp::add(fp::mul(half, pushed), Float(turn)));
                kicked = true;
            }
        }
    }
    else
    {
        const auto brake = brakeOf(app, cpu, car, float(shortfall));
        if (!brake)
            return std::nullopt;
        force = float(-Float(*brake));
    }
    // By the scale (1), and the turn by a half where that is under the scale.
    force = float(fp::mul(Float(force), Float(scale)));
    acrossForce = float(fp::mul(Float(acrossForce), Float(scale)));
    double share = double(scale);
    if (above(f64(app, kOffHalf), Float(double(scale))))
        share = 0.5;
    turn = float(fp::mul(Float(turn), Float(share)));
    // A skid past 7 across; slipping on a slippery surface.
    const Float past = fp::sub(Float(sizeOf(floatAt(app, car + kCarAcross))), f32(app, kSkidAcross));
    if (below(Float(0.0), past))
    {
        const reg32 pastBits = floatBits(float(past));
        put(app, car + kCarSkid, pastBits);
        put(app, car + kCarSkidAhead, pastBits);
        const Float grip = Float(double(floatAt(
            app, kSurfaceGrips + at(app, car + kCarSurface) * kSurfaceRow + at(app, car + kCarTrackKind) * 4)));
        if (above(Float(1.0), grip))
        {
            const Float steps = fp::mul(Float((i32(at(app, kRaceFrame)) >> 2) & 7), f64(app, kOffQuarterSteps));
            if (above(steps, grip) && (byte(app, car + kCarKind) & kKindHarder) != 0)
            {
                const reg32 kind = guest::direct<0x404610, airace::classRuleOf>(app, cpu, car);
                if (cpu.terminate)
                    return std::nullopt;
                const reg8 frame = byte(app, kRaceFrame);
                const bool slide = kind == 1 ? (frame & 0x20) != 0 : kind != 0 ? (frame & 0x60) == 0 : true;
                if (slide)
                {
                    const bool many = gameSettings(app).seriesRace == 1;
                    const Float across = many ? fp::mul(Float(acrossForce), f64(app, kOffSlideMany))
                                              : fp::mul(Float(acrossForce), f64(app, kOffByTen));
                    const Float turned = many ? fp::mul(f64(app, kOffSlideMany), Float(turn))
                                              : fp::mul(Float(turn), f64(app, kOffSlideTurn));
                    acrossForce = float(across);
                    turn = float(turned);
                }
            }
        }
    }
    else
    {
        put(app, car + kCarSkidAhead, 0);
        put(app, car + kCarSkid, 0);
    }
    if (kicked)
        put(app, car + kCarSkidAhead, kSkidKickBits);
    put(app, car + kCarAxesForce + 4, 0);
    put(app, car + kCarAxesForce, floatBits(acrossForce));
    put(app, car + kCarSpinChange, 0);
    put(app, kPush, 0);
    put(app, car + kCarSpinChange + 8, 0);
    put(app, kPushY, 0);
    put(app, car + kCarAxesForce + 8, floatBits(force));
    put(app, kPushZ, 0);
    put(app, car + kCarSpinChange + 4, floatBits(turn));
    return floatBits(turn);
}

/* Car eax. */
x86::reg32 steerOffRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return steerOff(app, cpu, car); });
    return args.eax;
}

// A car's: its +8, its axes (+0xc0, nine floats), +0x204 and +0x208, its track's place (+0xc,
// 0x78 bytes).
constexpr reg32 kCarTrackPlace = 0xc, kCar204 = 0x204, kCar208 = 0x208;
constexpr reg32 kTrackPlaceBytes = 0x78;

/* 0x4e070c: `bytes` at `to` zeroed.  False when the game stopped. */
bool fillZero(App* app, CPU& cpu, reg32 to, reg32 bytes)
{
    guest::direct<0x4e070c, ea::clearBytes>(app, cpu, to, bytes);
    return !cpu.terminate;
}

/* 0x408520: car rides with the car it follows ([+0x624], read again for each, as the code does):
 * its place, axes, spin, speed and +0x204/+0x208 copied, its track's place cleared and found anew
 * at the other's slice (0x41e940), its +8 and its brake the other's.  What the code leaves in eax
 * (the followed car with its brake's byte in al, or car where none); nullopt when the game
 * stopped. */
std::optional<reg32> rideWith(App* app, CPU& cpu, reg32 car)
{
    const auto followed = [&] { return at(app, car + kCarChased); };
    if (followed() == 0)
        return car;
    copy3(app, followed() + kCarPlace, car + kCarPlace);
    for (reg32 i = 0; i < 9 * 4; i += 4)
        put(app, car + kCarAxes + i, at(app, followed() + kCarAxes + i));
    copy3(app, followed() + kCarSpin, car + kCarSpin);
    copy3(app, followed() + kCarSpeed, car + kCarSpeed);
    put(app, car + kCar204, at(app, followed() + kCar204));
    put(app, car + kCar208, at(app, followed() + kCar208));
    if (!fillZero(app, cpu, car + kCarTrackPlace, kTrackPlaceBytes))
        return std::nullopt;
    put(app, car + kCarSlice, at(app, followed() + kCarSlice));
    put(app, car + kCarEight, at(app, followed() + kCarEight));
    guest::direct<0x41e940, trackdata::resetPlace>(app, cpu, at(app, car + kCarSlice), car + kCarTrackPlace);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 other = followed();
    putByte(app, car + kCarBrake, byte(app, other + kCarBrake));
    return (other & 0xffffff00) | byte(app, other + kCarBrake);
}

/* Car eax. */
x86::reg32 rideWithTarget(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return rideWith(app, cpu, car); });
    return args.eax;
}

/* 0x408870: the lane's offset car eases to ([+0x7c8]) a step toward the one it wants ([+0x7b4]),
 * no further.  The sum's compare is of the x87's value, not of the float stored.  What the code
 * leaves in eax: the offset wanted's bits where it went past, else the last compare's status word
 * in the car's low word. */
reg32 easeLaneBy(App* app, reg32 car, float step)
{
    const Float eased = f32(app, car + kCarEasedOffset), wanted = f32(app, car + kCarLaneWanted);
    if (eased == wanted || eased != eased || wanted != wanted)
        return statusInto(car, eased, wanted);
    const bool up = below(eased, wanted);
    const Float moved = up ? fp::add(eased, Float(step)) : fp::sub(eased, Float(step));
    put(app, car + kCarEasedOffset, floatBits(float(moved)));
    const Float limit = f32(app, car + kCarLaneWanted);
    if (up ? above(moved, limit) : below(moved, limit))
    {
        put(app, car + kCarEasedOffset, at(app, car + kCarLaneWanted));
        return at(app, car + kCarLaneWanted);
    }
    return statusInto(car, moved, limit);
}

/* Car eax, the step on the stack (ret 4). */
x86::reg32 easeLane(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = easeLaneBy(app, args.eax, app->getMemory<float>(cpu.esp + 4));
    cpu.esp += 8;
    return args.eax;
}

// The step's: a car's turn skipped (0x4be1a0, its kind 0x20 and the car's first word), the step's
// length (1/32 as a float's bits).  A car's: held (+0x8a), the step's acceleration's first
// (+0x2e4), a ghost driven by its target (+0x88), its speed and spin in its axes (+0x4ec on).
// +0x620's: off the road, riding with the car it follows.  Held under this upright (a double,
// 0.9); its revs down by 500 toward the idle.
constexpr reg32 kTurnKind = 0x20, kStepBits = 0x3d000000;
constexpr reg32 kCarHeldStill = 0x8a, kCarStepAcceleration = 0x2e4, kCarGhost = 0x88;
constexpr reg8 kAiOffRoad = 4, kAiRidesWith = 0x80;
constexpr reg32 kHeldUpright = 0x5363d4, kRevsDown = 0x1f4;
constexpr reg32 kStepPushed = 0x18, kStepFrame = 4;

// A car's: how it lies (+0xd0; tipped under 0.3 as a float's bits, compared as an integer as the
// code does), its other kind's byte (+0x201: 2 keeps it on the road), traffic's time before it
// drives (+0x65c).  A slice's left width (+0x17).  Off the road: the heading's error past this
// share of the surface's grip (a float), slower than 0.5, its grip past 2.5 (doubles) and sliding
// where its forward speed by this (a float) is under its speed across; backwards under 13.41 (a
// double).  The lane's step eased (0.2 as a float).
constexpr reg32 kCarLie = 0xd0, kCarKind2 = 0x201, kCarTrafficTime = 0x65c, kSliceLeftWidth = 0x17;
constexpr i32 kTippedBits = 0x3e99999a;
constexpr reg8 kKind2OnRoad = 2;
constexpr reg32 kOffHeadingShare = 0x5363ac, kOffSlow = 0x5363b4, kOffGrip = 0x5363bc, kOffSliding = 0x5363c4,
                kBackwardsUnder = 0x5363cc;
constexpr float kLaneStep = 0.2f;
constexpr reg32 kPhysicsPushed = 0x14, kPhysicsFrame = 0x10;

/* 0x408600: the computer's car's physics while racing: tipped over ([+0xd0] under 0.3) settled at
 * rest (0x4aa8e7); finished or busted, the coast and its revs; else its lane eased (by 0.2), its
 * revs, its reverse's end, its heading's error (0x4070c0), no speed wanted for traffic before its
 * time ([+0x65c]); then steered backwards (reversing, or going backwards), off the road (past the
 * road's edges, the heading's error past 0.04 of the surface's grip, slower than 0.5, before its
 * time, or sliding where its grip is past 2.5), backwards again under 13.41 where [+0x201] 2 or
 * on the road, on the road else -- and the step integrated.  `noGear` the caller's ebx, the
 * engine's gear where none comes nearer.  Its cores are called at the code's depth (their frames'
 * addresses handed on).  What the code leaves in eax; nullopt when the game stopped. */
std::optional<reg32> runPhysics(App* app, CPU& cpu, reg32 car, reg32 noGear)
{
    const GuestFrame frame(cpu, kPhysicsPushed, kPhysicsFrame);
    if (half(app, car + kCarStage) != 0)
        return car;
    if (i32(at(app, car + kCarLie)) < kTippedBits)
        return guest::direct<0x4aa8e7, handling::settleCar>(app, cpu, car);
    if (racerOut(app, car))
    {
        coast(app, car);
        return atDepth(cpu, [&] { return runEngine(app, cpu, car, noGear); });
    }
    easeLaneBy(app, car, kLaneStep);
    if (!atDepth(cpu, [&] { return runEngine(app, cpu, car, noGear); }))
        return std::nullopt;
    reverseCheckFor(app, car);
    if (!feCore<0x4070c0, frontend::sima::headingError>(app, cpu, car))
        return std::nullopt;
    if (i32(at(app, kRaceFrame)) < i32(at(app, car + kCarTrafficTime)) && (byte(app, car + kCarKind) & kKindTraffic) != 0)
        put(app, car + kCarSpeedWanted, 0);
    const float grip = floatAt(
        app, kSurfaceGrips + at(app, car + kCarSurface) * kSurfaceRow + at(app, car + kCarTrackKind) * 4);
    const auto steer = [&](std::optional<reg32> (*how)(App*, CPU&, reg32)) {
        return atDepth(cpu, [&] { return how(app, cpu, car); }).has_value();
    };
    const auto integrated = [&] { return atDepth(cpu, [&] { return integrateStep(app, cpu, car); }); };
    if (at(app, car + kCarReversing) == 1 || above(Float(0.0), f32(app, car + kCarForwardSpeed)))
    {
        if (!steer(steerBack))
            return std::nullopt;
        return integrated();
    }
    bool offRoad = false;
    if ((byte(app, car + kCarKind2) & kKind2OnRoad) == 0)
    {
        offRoad = true;
        const reg32 slice = at(app, kSlices) + at(app, car + kCarSlice) * kSliceSize;
        const reg8 lanes = byte(app, slice + kSliceLanes);
        const Float offset = f32(app, car + kCarLaneOffset);
        const reg32 left = reg32(0) - reg32(i32(reg32(byte(app, slice + kSliceLeftWidth)) << 15) * i32((lanes >> 4) & 0xf));
        const reg32 right = reg32(i32(reg32(byte(app, slice + kSliceRightWidth)) << 15) * i32(lanes & 0xf));
        if (!above(Float(sreg32(left)), offset) && !below(Float(sreg32(right)), offset)
            && !below(fp::mul(Float(grip), f32(app, kOffHeadingShare)), Float(sizeOf(floatAt(app, kHeadingError))))
            && !below(f32(app, car + kCarSpeedSize), f64(app, kOffSlow))
            && i32(at(app, kRaceFrame)) >= i32(at(app, car + kCarTrafficTime)))
            offRoad = above(f32(app, car + kCarGrip), f64(app, kOffGrip))
                      && below(fp::mul(f32(app, car + kCarForwardSpeed), f32(app, kOffSliding)),
                               Float(sizeOf(floatAt(app, car + kCarAcross))));
    }
    if (offRoad)
    {
        if (!steer(steerOff))
            return std::nullopt;
    }
    else if (below(f32(app, car + kCarSpeedSize), f64(app, kBackwardsUnder)))
    {
        if (!steer(steerBack))
            return std::nullopt;
    }
    else if (!steer(steerOn))
        return std::nullopt;
    return integrated();
}

/* Car eax; ebx the engine's gear where none comes nearer, as the caller's code leaves it. */
x86::reg32 aiPhysics(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax, noGear = args.ebx] { return runPhysics(app, cpu, car, noGear); });
    return args.eax;
}

/* 0x4088f0: the computer's car's step, each frame: nothing while 0x4be1a0 says so or it is off the
 * road ([+0x620] 4); the step's length 1/32, one substep; held ([+0x8a]) or tipped (upright under
 * 0.9): its slip and skid cleared, its revs down 500 toward the idle and settled at rest
 * (0x4aa8e7); before the cars move, its speed and spin 0; racing, its speed and spin in its own
 * axes (dot products with its axes), then it glides as a ghost ([+0x88]), rides with the car it
 * follows ([+0x620] 0x80) or drives (runPhysics, the engine's fallback gear the spin's address's
 * low byte, as the code's ebx has it there).  What the code leaves in eax; nullopt when the game
 * stopped. */
std::optional<reg32> stepCar(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, kStepPushed, kStepFrame);
    const auto skipped = feCore<0x4be1a0, frontend::simc::offBeat>(app, cpu, kTurnKind, at(app, car));
    if (!skipped)
        return std::nullopt;
    if (*skipped != 0)
        return *skipped;
    put(app, kStep, kStepBits);
    put(app, kSubsteps, 1);
    const reg8 flags = byte(app, car + kCarAiFlags);
    if ((flags & kAiOffRoad) != 0)
        return reg32(flags) << 8;
    // (A compare's status word the code leaves in ax here is overwritten on either way on.)
    if (byte(app, car + kCarHeldStill) != 0 || below(f32(app, car + kCarUpright), f64(app, kHeldUpright)))
    {
        put(app, car + kCarSlip, 0);
        put(app, car + kCarSkid, 0);
        put(app, car + kCarSkidAhead, 0);
        const reg32 performance = at(app, car + kCarPerformance);
        put(app, car + kCarLimiter, 0);
        const reg32 revs = at(app, car + kCarRevs);
        put(app, car + kCarRevs,
            i32(at(app, performance + kPerformanceIdle) + kRevsDown) < i32(revs) ? revs - kRevsDown
                                                                                 : at(app, performance + kPerformanceIdle));
        return guest::direct<0x4aa8e7, handling::settleCar>(app, cpu, car);
    }
    const reg32 move = at(app, kCarsMove);
    put(app, car + kCarStepAcceleration, 0);
    if (move == 0)
    {
        for (const reg32 field : {kCarSpeed + 4, kCarSpeed + 8, kCarSpin, kCarSpin + 4, kCarSpin + 8, kCarSpeed})
            put(app, car + field, 0);
        return 0;
    }
    if (half(app, car + kCarStage) != 0)
        return move;
    // Its speed and spin in its axes, each through the three axes in turn.
    reg32 to = car + kCarAcross;
    for (const reg32 vector : {kCarSpeed, kCarSpin})
        for (reg32 axis = 0; axis < 3; ++axis)
        {
            const auto along = dot(app, cpu, car + vector, car + kCarAxes + 0xc * axis);
            if (!along)
                return std::nullopt;
            put(app, to, floatBits(float(*along)));
            to += 4;
        }
    std::optional<reg32> driven;
    if (byte(app, car + kCarGhost) != 0)
        driven = atDepth(cpu, [&] { return glide(app, cpu, car); });
    else if ((byte(app, car + kCarAiFlags) & kAiRidesWith) != 0)
        driven = atDepth(cpu, [&] { return rideWith(app, cpu, car); });
    else
        driven = atDepth(cpu, [&] { return runPhysics(app, cpu, car, car + kCarSpin); });
    return driven;
}

/* Car eax. */
x86::reg32 aiStep(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, car = args.eax] { return stepCar(app, cpu, car); });
    return args.eax;
}

}  // namespace aidrive
}  // namespace nfs3hp
