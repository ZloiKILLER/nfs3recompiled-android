#include "native_guest.h"

#include <cstdint>

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
 * its +0x200 has 2).  disassemble_nfs3hp.py
 * binds each address to the function of that name here (AIDRIVE_NATIVES), and
 * the generated code calls it by name.  tools/game_checks holds each against
 * its generated code (aidrive.inc).
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
 * Written as their code is (Watcom's register calls, optimised: values kept
 * in registers between uses): each push and pop to the guest's stack, each
 * call with the return address's slot, the frame's locals in the guest's
 * memory, the registers as the code leaves them (ah, bl, bh where the code
 * writes a byte of one), the x87's stack pushed and popped as the x87 does,
 * its arithmetic through x86::FPU (add, sub, mul, div, sin, toInteger) in the
 * code's order of operands and rounded as the race's control word says, and a
 * compare's status word left in ax as fnstsw leaves it (sahf's below and
 * equal the C0 and C3 bits of it).  The two switches go by the targets
 * nfs3.exe's tables hold (0x404d14, 0x405714).  Every other function of the
 * game's is called by address, the two of the AI that the port has as
 * decompiled C++ (0x4064f0, 0x4070c0, native_frontend_sima.cpp) among them. */
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
constexpr reg32 kPlayMode = 0x6fd3b0, kSecondPlayer = 0x6fd4f4, kBackwards = 0x6fd4c0, kRaceFrame = 0x7d3684;
constexpr reg32 kCarsMove = 0x7d3680, kPursuitJump = 0x6fd4a8, kTrackAi = 0x6fd4ac, kLaneVotes = 0x6fd494;
constexpr reg32 kSingleRace = 0x6fd4f0;
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
// What is called by address: the opponents' pull and heading (native_frontend_sima.cpp),
// the racestart's car put back and grid place, the track's place found again,
// the cops' sounds, the camera's target car, the car at rest settled, chop,
// the fill, the distance of two cars, vectors, the slices' gaps, the speeds'
// table and a slice's curve.
constexpr reg32 kAiPull = 0x4064f0, kAiHeading = 0x4070c0, kResetCar = 0x429100, kGridPlace = 0x42bc70;
constexpr reg32 kResetPosition = 0x41e940, kCopEffect = 0x418c30, kCarFlagTest = 0x4127b0, kCameraTarget = 0x422bd0;
constexpr reg32 kSettleAtRest = 0x4aa8e7, kChop = 0x4dfd56, kFillZero = 0x4e070c, kDistance = 0x4965b0;
constexpr reg32 kAddVectors = 0x4dffb0, kSubtractVectors = 0x4e0000, kScaleVectors = 0x4e0050;
constexpr reg32 kTranspose = 0x4e0110, kDot = 0x4e01f0, kLength = 0x4e06a0;
constexpr reg32 kSliceGap = 0x40e120, kSliceAhead = 0x40e160, kSliceByPace = 0x40e1b0, kSliceSpeed = 0x40ed40;
constexpr reg32 kSliceCurve = 0x40ef70;

void push(App* app, CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(App* app, CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(App* app, CPU& cpu, void (*function)(App*, CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* movsd: a dword from [esi] to es:[edi], both stepped by the direction flag;
 * rep movsd: ecx of them. */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
    cpu.edi += step;
    cpu.esi += step;
}

void movsd3(App* app, CPU& cpu)
{
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
}

void repMovsd(App* app, CPU& cpu)
{
    for (; cpu.ecx != 0; --cpu.ecx)
        movsd(app, cpu);
}

// The x87's, as the code has them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

void fld(CPU& cpu, const Float& value) { cpu.fpu.push(value); }
void fild(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(Float(sreg32(at(app, a)))); }

/* fld st(i). */
void fldSt(CPU& cpu, int i)
{
    const Float value = cpu.fpu.st(i);
    cpu.fpu.push(value);
}

void fst(App* app, CPU& cpu, reg32 a) { app->getMemory<float>(a) = float(cpu.fpu.st(0)); }
void fst64(App* app, CPU& cpu, reg32 a) { app->getMemory<double>(a) = double(cpu.fpu.st(0)); }

void fstp(App* app, CPU& cpu, reg32 a)
{
    fst(app, cpu, a);
    cpu.fpu.pop();
}

void fstp64(App* app, CPU& cpu, reg32 a)
{
    fst64(app, cpu, a);
    cpu.fpu.pop();
}

/* fstp st(i): st(0) into st(i), popped. */
void fstpTo(CPU& cpu, int i)
{
    cpu.fpu.st(i) = cpu.fpu.st(0);
    cpu.fpu.pop();
}

void fxch(CPU& cpu, int i)
{
    const Float t = cpu.fpu.st(0);
    cpu.fpu.st(0) = cpu.fpu.st(i);
    cpu.fpu.st(i) = t;
}

// st(0) op= m (fadd m32 and the like; fadd st(i) and fmul st(i) with st(i)
// as m); fsubr, fdivr the other way: st(0) = m op st(0).
void fadd(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), m); }
void fsub(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), m); }
void fmul(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), m); }
void fdiv(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::div(cpu.fpu.st(0), m); }
void fsubr(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(m, cpu.fpu.st(0)); }
void fdivr(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::div(m, cpu.fpu.st(0)); }

// st(i) op= st(0), popped (faddp st(i) and the like).
void faddp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::add(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fsubp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::sub(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fmulp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::mul(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fchs(CPU& cpu) { cpu.fpu.st(0) = -cpu.fpu.st(0); }

/* fistp dword [a]. */
void fistp(App* app, CPU& cpu, reg32 a)
{
    put(app, a, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
}

/* sub esp, 4; fistp dword [esp]; pop eax: st(0) as an integer into eax. */
void fistpToEax(App* app, CPU& cpu)
{
    cpu.esp -= 4;
    fistp(app, cpu, cpu.esp);
    cpu.eax = pop(app, cpu);
}

/* fnstsw ax after a compare, and the flags sahf makes of it: below the
 * carry (C0), equal the zero flag (C3). */
struct Flags
{
    bool cf, zf;
    bool below() const { return cf; }
    bool belowOrEqual() const { return cf || zf; }
    bool above() const { return !cf && !zf; }
};

Flags status(CPU& cpu)
{
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

/* fcomp m; fnstsw ax: st(0) compared with m, popped. */
Flags fcomp(CPU& cpu, const Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.fpu.pop();
    return status(cpu);
}

/* fcompp; fnstsw ax: st(0) compared with st(1), both popped. */
Flags fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.fpu.pop();
    cpu.fpu.pop();
    return status(cpu);
}

/* The size of a float the code's way, into [to]:
 *     fldz; fcomp dword [from]; fnstsw ax; sahf; jae +
 *     mov eax, [from]; mov [to], eax; jmp ++
 * +:  fld dword [from]; fchs; fstp dword [to]
 * -- eax left the float's bits, or fnstsw's ax.  (`flipped`: the code's other
 * order, jbe to the move -- 0 below the value -- for the same result.) */
void magnitude(App* app, CPU& cpu, reg32 from, reg32 to)
{
    fld(cpu, Float(0.0));
    if (fcomp(cpu, f32(app, from)).below())
    {
        cpu.eax = at(app, from);
        put(app, to, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, from));
        fchs(cpu);
        fstp(app, cpu, to);
    }
}

/* fldz; fcomp dword [from]; ... jbe +: fld; fchs; fstp [to]; jmp ++;
 * +: mov eax, [from]; mov [to], eax -- the size with 0 compared the other
 * way round: where the value is 0 (or a NaN) the move. */
void magnitudeBelow(App* app, CPU& cpu, reg32 from, reg32 to)
{
    fld(cpu, Float(0.0));
    if (fcomp(cpu, f32(app, from)).belowOrEqual())
    {
        cpu.eax = at(app, from);
        put(app, to, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, from));
        fchs(cpu);
        fstp(app, cpu, to);
    }
}

/* A float clamped the code's way, into [to]:
 *     fld [value]; fcomp [limit]; ... jbe +: mov eax, [limit]; jmp ++
 *     +: mov eax, [value];  ++: mov [to], eax
 * -- the smaller of the two (the value where they are equal or unordered). */
void atMost(App* app, CPU& cpu, reg32 value, reg32 limit, reg32 to)
{
    fld(cpu, f32(app, value));
    cpu.eax = fcomp(cpu, f32(app, limit)).belowOrEqual() ? at(app, value) : at(app, limit);
    put(app, to, cpu.eax);
}

/* The car's distance to another's place (0x4965b0, onto the x87), its size
 * the code's way: fldz; fcompp; jae -- called again either way, negated
 * where the first was not over 0. */
bool distanceSize(App* app, CPU& cpu, reg32 a, reg32 d)
{
    cpu.eax = a;
    cpu.edx = d;
    if (!byAddress(app, cpu, kDistance))
        return false;
    fld(cpu, Float(0.0));
    const bool over = fcompp(cpu).below();
    cpu.eax = a;
    cpu.edx = d;
    if (!byAddress(app, cpu, kDistance))
        return false;
    if (!over)
        fchs(cpu);
    return true;
}

}  // namespace

void copStartPlace(App* app, CPU& cpu);
void callRoadblock(App* app, CPU& cpu);
void sirenSwitch(App* app, CPU& cpu);
void watchRacers(App* app, CPU& cpu);
void jumpBehindLeader(App* app, CPU& cpu);
void clockRacers(App* app, CPU& cpu);
void startPursuit(App* app, CPU& cpu);
void bustCheck(App* app, CPU& cpu);
void pursuitStep(App* app, CPU& cpu);
void dropPursuit(App* app, CPU& cpu);
void copDriver(App* app, CPU& cpu);
void nearCar(App* app, CPU& cpu);
void nearHuman(App* app, CPU& cpu);
void humanNearSlice(App* app, CPU& cpu);
void trafficUnwatched(App* app, CPU& cpu);
void trafficStart(App* app, CPU& cpu);
void trafficStop(App* app, CPU& cpu);
void extraCarStep(App* app, CPU& cpu);
void setDrivingConstants(App* app, CPU& cpu);
void drivingNothing(App* app, CPU& cpu);
void setReverse(App* app, CPU& cpu);
void reverseCheck(App* app, CPU& cpu);
void reverseWhenSlow(App* app, CPU& cpu);
void reverseWhenTipped(App* app, CPU& cpu);
void engineRevs(App* app, CPU& cpu);
void brakeForce(App* app, CPU& cpu);
void speedAlongRoad(App* app, CPU& cpu);
void laneError(App* app, CPU& cpu);
void glideAlongRoad(App* app, CPU& cpu);
void lookAheadSlice(App* app, CPU& cpu);
void sidewaysToRoad(App* app, CPU& cpu);
void towardTarget(App* app, CPU& cpu);
void towardTargetAhead(App* app, CPU& cpu);
void towardSlice(App* app, CPU& cpu);
void towardSliceSlowing(App* app, CPU& cpu);
void towardLane(App* app, CPU& cpu);
void wayToGo(App* app, CPU& cpu);
void integrate(App* app, CPU& cpu);
void coastAfterFinish(App* app, CPU& cpu);
void steerOnRoad(App* app, CPU& cpu);
void steerReversing(App* app, CPU& cpu);
void steerOffRoad(App* app, CPU& cpu);
void rideWithTarget(App* app, CPU& cpu);
void aiPhysics(App* app, CPU& cpu);
void easeLane(App* app, CPU& cpu);
void aiStep(App* app, CPU& cpu);

/* 0x404d30: cop eax's start (Hot Pursuit): found among the cops ([kCops],
 * [kCopCount]) at index i, its slice ([+0x1c]) is the start's (0, or the
 * grid's first place's run backwards, 0x42bc70), and the slice two behind
 * it (three for the fifth cop on, 0x40e160) goes to [edx]; its place [ebx]:
 * across by the slice's right width times its lanes (+0x1b, +0x13), all of
 * it, mirrored, half of it or half of it mirrored by i % 4 (the table at
 * 0x404d14); up 0x8000 and ahead 98304 (the constants' fistp) whether found
 * or not. */
void copStartPlace(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    cpu.edi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.esi = cpu.ebx;
    put(app, cpu.ebx, 0);
    for (cpu.ecx = 0; i32(cpu.ecx) < i32(at(app, kCopCount)); ++cpu.ecx)
    {
        cpu.ebx = at(app, cpu.ecx * 4 + kCops);
        if (cpu.edi != cpu.ebx)
            continue;
        cpu.eax = at(app, kBackwards);
        put(app, cpu.ebx + 0x1c, 0);
        if (cpu.eax != 0)
        {
            cpu.ebx = cpu.ebp - 0x14;
            cpu.edx = cpu.ebp - 8;
            cpu.eax = 0;
            if (!byAddress(app, cpu, kGridPlace))
                return;
            cpu.eax = at(app, cpu.ebp - 8);
            put(app, cpu.edi + 0x1c, cpu.eax);
        }
        else
            put(app, cpu.ebx + 0x1c, cpu.eax);
        cpu.edx = i32(cpu.ecx) >= 4 ? 0xfffffffd : 0xfffffffe;
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kSliceAhead))
            return;
        cpu.edx = at(app, cpu.ebp - 4);
        put(app, cpu.edx, cpu.eax);
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.ebx = 0;
        cpu.bl = byte(app, cpu.edx + cpu.eax * 4 + 0x1b);
        cpu.al = byte(app, cpu.edx + cpu.eax * 4 + 0x13);
        cpu.ebx <<= 15;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.ebx = reg32(std::int64_t(i32(cpu.ebx)) * std::int64_t(i32(cpu.eax)));
        // cdq by hand, idiv 4: the cop's index's remainder.
        cpu.edx = reg32(i32(cpu.ecx) >> 31);
        cpu.edi = 4;
        cpu.eax = cpu.ecx;
        {
            const std::int64_t dividend = std::int64_t((std::uint64_t(cpu.edx) << 32) | cpu.eax);
            cpu.eax = reg32(dividend / i32(cpu.edi));
            cpu.edx = reg32(dividend % i32(cpu.edi));
        }
        reg32 target = 0x404dfa;
        if (cpu.edx <= 3)
            target = at(app, cpu.edx * 4 + 0x404d14);
        switch (target)
        {
        case 0x404de6:
            put(app, cpu.esi, cpu.ebx);
            break;
        case 0x404dea:
            cpu.edx = cpu.ebx;
            put(app, cpu.esi, cpu.ebx);
            cpu.edx = reg32(0) - cpu.edx;
            put(app, cpu.esi, cpu.edx);
            break;
        case 0x404df4:
            cpu.edx = cpu.ebx;
            cpu.eax = cpu.ebx;
            cpu.edx = reg32(i32(cpu.edx) >> 31);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(i32(cpu.eax) >> 1);
            put(app, cpu.esi, cpu.eax);
            break;
        default:  // 0x404dfa
            cpu.edx = cpu.ebx;
            cpu.edx = reg32(0) - cpu.edx;
            cpu.eax = cpu.edx;
            cpu.edx = reg32(i32(cpu.edx) >> 31);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(i32(cpu.eax) >> 1);
            put(app, cpu.esi, cpu.eax);
            break;
        }
        break;
    }
    fld(cpu, f32(app, 0x5361ac));
    fistpToEax(app, cpu);
    fld(cpu, f32(app, 0x5361b0));
    put(app, cpu.esi + 8, cpu.eax);
    fistpToEax(app, cpu);
    put(app, cpu.esi + 4, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404e40: a player's cop calls a roadblock ([+0x512], its button, once a
 * press: [+0x513]).  Its roadblock ([kRoadblocks] + 0x50 * [+0x714]) up
 * already: taken down if its time is past ([+0x4c], 0x4c9f50), and the
 * cops' sound 0x11 or, still up, 0x12; else set up at its slice in its lane
 * for 0xc0 frames (0x4c9dd0), 0x40b9c0 told the side (0 to 2 by its lane,
 * the other way round in a race run backwards), and the sound 0x10 -- each
 * sound only for this machine's player, or when one player plays. */
void callRoadblock(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.ah = byte(app, cpu.eax + 0x512);
    if (cpu.ah == 0)
    {
        putByte(app, cpu.esi + 0x513, cpu.ah);
        return leave();
    }
    if (byte(app, cpu.esi + 0x513) != 0)
        return leave();
    cpu.ebx = at(app, cpu.esi + 0x714);
    cpu.eax = cpu.ebx * 4;
    cpu.eax += cpu.ebx;
    cpu.eax <<= 4;
    reg32 sound;
    if (at(app, cpu.eax + kRoadblocks) != 0)
    {
        cpu.edx = at(app, kRaceFrame);
        if (i32(cpu.edx) >= i32(at(app, cpu.eax + kRoadblocks + 0x4c)))
        {
            cpu.eax = cpu.ebx;
            if (!byAddress(app, cpu, 0x4c9f50))
                return;
            cpu.ebx = at(app, kSecondPlayer);
            sound = 0x11;
            if (cpu.ebx != at(app, cpu.esi + 0x1f0) && i32(at(app, kPlayMode)) > 1)
                sound = 0;
        }
        else
        {
            cpu.edx = at(app, kSecondPlayer);
            sound = 0x12;
            if (cpu.edx != at(app, cpu.esi + 0x1f0) && i32(at(app, kPlayMode)) > 1)
                sound = 0;
        }
    }
    else
    {
        cpu.ebx = at(app, cpu.esi + 0x638);
        if (i32(cpu.ebx) < 6)
            cpu.edi = 0;
        else if (i32(cpu.ebx) > 7)
            cpu.edi = 2;
        else
            cpu.edi = 1;
        if (at(app, kBackwards) != 0)
        {
            cpu.eax = 2;
            cpu.eax -= cpu.edi;
            cpu.edi = cpu.eax;
        }
        cpu.ecx = at(app, kRaceFrame);
        cpu.edx = at(app, cpu.esi + 0x1c);
        cpu.eax = at(app, cpu.esi + 0x714);
        cpu.ecx += 0xc0;
        if (!byAddress(app, cpu, 0x4c9dd0))
            return;
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x40b9c0))
            return;
        cpu.edx = at(app, kSecondPlayer);
        sound = 0x10;
        if (cpu.edx != at(app, cpu.esi + 0x1f0) && i32(at(app, kPlayMode)) > 1)
            sound = 0;
    }
    if (sound != 0)
    {
        cpu.eax = sound;
        if (!byAddress(app, cpu, kCopEffect))
            return;
    }
    putByte(app, cpu.esi + 0x513, 1);
    return leave();
}

/* 0x404f80: a player's cop's siren switched by its button ([+0x50e]), once a
 * press ([+0x620] 0x10 held): the siren's flag ([+0x710] 0x10) turned over,
 * its sound ([+0x734]) 1 as it comes on, 0 as it goes off. */
void sirenSwitch(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (byte(app, cpu.eax + 0x50e) == 0)
        putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) & 0xef);
    else if ((byte(app, cpu.eax + 0x620) & 0x10) == 0)
    {
        cpu.bl = byte(app, cpu.eax + 0x710);
        if ((cpu.bl & 0x10) != 0)
        {
            cpu.bh = cpu.bl;
            put(app, cpu.eax + 0x734, 0);
            cpu.bh &= 0xef;
            putByte(app, cpu.eax + 0x710, cpu.bh);
        }
        else
        {
            cpu.cl = cpu.bl;
            put(app, cpu.eax + 0x734, 1);
            cpu.cl |= 0x10;
            putByte(app, cpu.eax + 0x710, cpu.cl);
        }
        putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) | 0x10);
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404ff0: cop eax watches the racers ([kRacers]): the nearest of them
 * within 50 slices (0x40e120 the gap, the distance of the places breaking
 * a tie, 0x4965b0) to [+0x730]; where this machine's player drives it (one
 * player, or the camera's car, 0x422bd0) each racer goes on the map
 * (0x485180: player, index, kind -- 2 finished, 1 busted, 4 near, 7 the one
 * it chases, 3 else) and, where it chases none, the nearest as 5. */
void watchRacers(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.edx = 0x186a0;
    cpu.ecx = at(app, kPlayMode);
    put(app, cpu.ebp - 0xc, cpu.edx);
    if (cpu.ecx == 1)
        cpu.eax = at(app, cpu.eax + 0x1f0);
    else
        cpu.eax = 0;
    cpu.ebx = at(app, kPlayMode);
    put(app, cpu.ebp - 4, cpu.eax);
    if (cpu.ebx == 1)
        cpu.eax = 1;
    else
    {
        cpu.eax = 0;
        if (!byAddress(app, cpu, kCameraTarget))
            return;
        cpu.eax = cpu.esi == cpu.eax ? 1 : 0;
    }
    put(app, cpu.esi + 0x730, 0);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.esi + 0x1c);
    cpu.edi = 0;
    put(app, cpu.ebp - 0x10, cpu.eax);
    for (; i32(cpu.edi) < i32(at(app, kRacerCount)); ++cpu.edi)
    {
        cpu.ecx = at(app, cpu.edi * 4 + kRacers);
        cpu.edx = 3;
        cpu.ah = byte(app, cpu.ecx + 0x200);
        put(app, cpu.ebp - 0x14, cpu.edx);
        if ((cpu.ah & 1) != 0 && cpu.edx == at(app, cpu.ecx + 0x2c4))
        {
            if (at(app, cpu.ebp - 8) != 0)
            {
                cpu.ebx = 2;
                cpu.eax = at(app, cpu.ebp - 4);
                cpu.edx = cpu.edi;
                if (!byAddress(app, cpu, 0x485180))
                    return;
            }
            continue;
        }
        if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 4)
        {
            if (at(app, cpu.ebp - 8) == 0)
                continue;
            cpu.ebx = 1;
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = cpu.edi;
            if (!byAddress(app, cpu, 0x485180))
                return;
            continue;
        }
        cpu.edx = at(app, cpu.ebp - 0x10);
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kSliceGap))
            return;
        const bool ahead = i32(cpu.eax) > 0;
        cpu.edx = at(app, cpu.ebp - 0x10);
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kSliceGap))
            return;
        if (!ahead)
            cpu.eax = reg32(0) - cpu.eax;
        bool nearest = false;
        if (i32(cpu.eax) <= 0x32)
        {
            cpu.edx = 4;
            cpu.ebx = at(app, cpu.ebp - 0xc);
            put(app, cpu.ebp - 0x14, cpu.edx);
            if (i32(cpu.eax) < i32(cpu.ebx))
            {
                put(app, cpu.ebp - 0x1c, cpu.edi);
                put(app, cpu.ebp - 0xc, cpu.eax);
                nearest = true;
            }
            else if (cpu.eax == cpu.ebx)
            {
                // As near as the nearest: the nearer of the two by distance.
                cpu.eax = cpu.esi + 0x98;
                cpu.ebx = cpu.ecx + 0x98;
                put(app, cpu.ebp - 0x18, cpu.eax);
                if (!distanceSize(app, cpu, cpu.ebx, cpu.eax))
                    return;
                fstp(app, cpu, cpu.ebp - 0x24);
                cpu.eax = at(app, cpu.esi + 0x730);
                cpu.ebx = cpu.esi + 0x98;
                cpu.eax += 0x98;
                if (!distanceSize(app, cpu, cpu.eax, cpu.ebx))
                    return;
                fstp(app, cpu, cpu.ebp - 0x20);
                fld(cpu, f32(app, cpu.ebp - 0x24));
                if (fcomp(cpu, f32(app, cpu.ebp - 0x20)).below())
                {
                    put(app, cpu.ebp - 0x1c, cpu.edi);
                    nearest = true;
                }
            }
            if (nearest)
                put(app, cpu.esi + 0x730, cpu.ecx);
        }
        if (cpu.ecx == at(app, cpu.esi + 0x624))
            put(app, cpu.ebp - 0x14, 7);
        if (at(app, cpu.ebp - 8) != 0)
        {
            cpu.ebx = at(app, cpu.ebp - 0x14);
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = cpu.edi;
            if (!byAddress(app, cpu, 0x485180))
                return;
        }
    }
    cpu.ecx = at(app, cpu.esi + 0x730);
    if (cpu.ecx != 0 && at(app, cpu.esi + 0x624) == 0 && at(app, cpu.ebp - 8) != 0)
    {
        cpu.ebx = 5;
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 4);
        if (!byAddress(app, cpu, 0x485180))
            return;
    }
    return leave();
}

/* 0x405220: cop eax put back behind the race's leader: the racer of the
 * best place ([+0x2bc]) not finished or busted; in a one-player Hot Pursuit
 * with the option on ([kPursuitJump]) after the first 0x480 frames, the
 * cop's slice the one before the leader's; then put back on the road
 * (0x429100) if edx asks it or a leader was found. */
void jumpBehindLeader(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.ecx = cpu.eax;
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.esi = 0x2710;
    cpu.ebx = 0xffffffff;
    for (cpu.edx = 0; i32(cpu.edx) < i32(at(app, kRacerCount)); ++cpu.edx)
    {
        cpu.eax = at(app, cpu.edx * 4 + kRacers);
        if ((byte(app, cpu.eax + 0x200) & 1) != 0 && at(app, cpu.eax + 0x2c4) == 3)
            continue;
        if ((byte(app, cpu.eax + 0x200) & 1) != 0 && at(app, cpu.eax + 0x2c4) == 4)
            continue;
        cpu.edi = at(app, cpu.eax + 0x2bc);
        if (i32(cpu.esi) > i32(cpu.edi))
        {
            cpu.esi = cpu.edi;
            cpu.ebx = cpu.edx;
        }
    }
    if (at(app, kPlayMode) == 0 && at(app, kPursuitJump) != 0 && i32(at(app, kRaceFrame)) > 0x480
        && i32(cpu.ebx) > -1)
    {
        cpu.eax = at(app, cpu.ebx * 4 + kRacers);
        cpu.eax = at(app, cpu.eax + 0x1c);
        --cpu.eax;
        if (i32(cpu.eax) < 0)
            cpu.eax += at(app, kSliceCount);
        put(app, cpu.ecx + 0x1c, cpu.eax);
    }
    if (at(app, cpu.ebp - 4) != 0 || i32(cpu.ebx) > -1)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kResetCar))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4052e0: cop eax clocks the racers within 50 slices of it: how far each
 * goes over its slice's speed (0x40ed40), the most of it kept in the
 * racer's [+0x6f0]; past 2, its pursuit's record ([+0x628]) marked
 * ([+0x14] 1). */
void clockRacers(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.esi = at(app, cpu.eax + 0x1c);
    for (cpu.ebx = 0; i32(cpu.ebx) < i32(at(app, kRacerCount)); ++cpu.ebx)
    {
        cpu.ecx = at(app, cpu.ebx * 4 + kRacers);
        if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 3)
            continue;
        if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 4)
            continue;
        cpu.edx = cpu.esi;
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kSliceGap))
            return;
        if (i32(cpu.eax) <= 0)
            cpu.eax = reg32(0) - cpu.eax;
        if (i32(cpu.eax) > 0x32)
            continue;
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kSliceSpeed))
            return;
        fsubr(cpu, f32(app, cpu.ecx + 0x7c4));
        fld(cpu, f32(app, cpu.ecx + 0x6f0));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 4);
        if (fcomp(cpu, f32(app, cpu.ebp - 4)).belowOrEqual())
            cpu.eax = at(app, cpu.ebp - 4);
        else
            cpu.eax = at(app, cpu.ecx + 0x6f0);
        put(app, cpu.ebp - 8, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ecx + 0x6f0, cpu.eax);
        if (i32(cpu.eax) > 0x40000000)
        {
            cpu.eax = at(app, cpu.ecx + 0x628);
            if (at(app, cpu.eax + 0x14) == 0)
                put(app, cpu.eax + 0x14, 1);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4053b0: a cop with its siren on and no one to chase starts after the
 * racer nearest it ([+0x730]) where it was clocked past 2 over and is not
 * finished or busted (0x4018b0: 2, the cop, the racer; 0 on the stack). */
void startPursuit(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if ((byte(app, cpu.eax + 0x710) & 0x10) == 0 || at(app, cpu.eax + 0x624) != 0)
        return leave();
    cpu.ecx = at(app, cpu.eax + 0x730);
    if (cpu.ecx == 0)
        return leave();
    cpu.ebx = at(app, cpu.ecx + 0x6f0);
    cpu.edx = cpu.ecx;
    if (i32(cpu.ebx) <= 0x40000000)
        return leave();
    if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 3)
        return leave();
    cpu.edx = at(app, cpu.eax + 0x730);
    if ((byte(app, cpu.edx + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 4)
        return leave();
    cpu.edx = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x730);
    cpu.eax = 2;
    push(app, cpu, 0);
    if (!byAddress(app, cpu, 0x4018b0))
        return;
    return leave();
}

/* Whether the car a cop chases ([+0x624]) is at its own car's end: a human's
 * in state 7, a computer's in state 6 ([+0x6e0]) -- the code inlines it
 * three times, eax left 1 or 0. */
void targetCaught(App* app, CPU& cpu)
{
    cpu.eax = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.eax + 0x200) & 4) != 0)
        cpu.eax = at(app, cpu.eax + 0x6e0) == 7 ? 1 : 0;
    else
        cpu.eax = at(app, cpu.eax + 0x6e0) == 6 ? 1 : 0;
}

/* 0x405430: whether cop eax busts the car it chases (eax 1): within 7 of
 * it, the car no faster than [kBustSpeed], the race under way and the cop
 * upright -- the first time (0x4127b0) a grace of 0x18 frames ([+0x720],
 * [+0x710] 0x40), after it the pursuit's record ([+0x628]) set to 2 with
 * 0x280 frames ([+0x28]) and the bust where the car is at its end; out of
 * reach the grace is gone, and a pursuit at 2 busts where the car is at its
 * end and the frames are up. */
void bustCheck(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx == 0)
    {
        cpu.eax = 0;
        return leave();
    }
    cpu.esi = at(app, cpu.eax + 0x628);
    if (at(app, cpu.esi + 0x1c) == 2)
    {
        targetCaught(app, cpu);
        if (cpu.eax != 0)
        {
            cpu.eax = 1;
            return leave();
        }
    }
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.ebx = cpu.ecx + 0x98;
    cpu.eax += 0x98;
    if (!distanceSize(app, cpu, cpu.eax, cpu.ebx))
        return;
    fstp(app, cpu, cpu.ebp - 4);
    bool inReach = false;
    fld(cpu, f32(app, cpu.ebp - 4));
    if (fcomp(cpu, f64(app, 0x5361b4)).below())
    {
        cpu.eax = at(app, cpu.ecx + 0x624);
        fld(cpu, f32(app, cpu.eax + 0x7c4));
        if (!fcomp(cpu, f32(app, kBustSpeed)).above() && half(app, cpu.ecx + 0x164) == 0)
        {
            fld(cpu, f32(app, cpu.ecx + 0x10c));
            inReach = fcomp(cpu, f64(app, 0x5361bc)).above();
        }
    }
    if (inReach)
    {
        if ((byte(app, cpu.ecx + 0x710) & 0x40) != 0)
        {
            cpu.eax = at(app, kRaceFrame);
            if (i32(cpu.eax) > i32(at(app, cpu.ecx + 0x720)))
            {
                cpu.eax += 0x280;
                put(app, cpu.esi + 0x1c, 2);
                put(app, cpu.esi + 0x28, cpu.eax);
                targetCaught(app, cpu);
                if (cpu.eax != 0)
                {
                    cpu.eax = 1;
                    return leave();
                }
            }
        }
        else
        {
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kCarFlagTest))
                return;
            cpu.eax = at(app, kRaceFrame);
            cpu.bh = byte(app, cpu.ecx + 0x710);
            cpu.eax += 0x18;
            cpu.bh |= 0x40;
            put(app, cpu.ecx + 0x720, cpu.eax);
            putByte(app, cpu.ecx + 0x710, cpu.bh);
        }
        cpu.eax = 0;
        return leave();
    }
    putByte(app, cpu.ecx + 0x710, byte(app, cpu.ecx + 0x710) & 0xbf);
    if (at(app, cpu.esi + 0x1c) == 2)
    {
        targetCaught(app, cpu);
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, kRaceFrame);
            if (i32(cpu.eax) > i32(at(app, cpu.esi + 0x28)))
            {
                cpu.eax = 1;
                return leave();
            }
        }
    }
    cpu.eax = 0;
    return leave();
}

/* 0x405630: a pursuit's step: with the siren on, the car chased within 300
 * and not finished, whether it is busted (bustCheck); else the pursuit
 * given up (0x401a80) and eax 0. */
void pursuitStep(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.ecx = cpu.eax;
    bool keep = false;
    if ((byte(app, cpu.eax + 0x710) & 0x10) != 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x624);
        cpu.ebx = cpu.ecx + 0x98;
        cpu.eax += 0x98;
        if (!distanceSize(app, cpu, cpu.eax, cpu.ebx))
            return;
        fstp(app, cpu, cpu.ebp - 4);
        fld(cpu, f32(app, cpu.ebp - 4));
        if (!fcomp(cpu, f64(app, 0x5361c4)).above())
        {
            cpu.eax = at(app, cpu.ecx + 0x624);
            keep = !((byte(app, cpu.eax + 0x200) & 1) != 0 && at(app, cpu.eax + 0x2c4) == 3);
        }
    }
    cpu.eax = cpu.ecx;
    if (keep)
    {
        if (!direct(app, cpu, bustCheck))
            return;
    }
    else
    {
        if (!byAddress(app, cpu, 0x401a80))
            return;
        cpu.eax = 0;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4056e0: car eax's pursuit dropped: the computer no longer drives it
 * ([+0x200] 2), its state 2 ([+0x6e0]), its pursuit's record's 0; put back
 * on the road (0x429100) where its flag says ([+0x621] 0x40). */
void dropPursuit(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    putByte(app, cpu.eax + 0x200, byte(app, cpu.eax + 0x200) & 0xfd);
    cpu.edx = at(app, cpu.eax + 0x628);
    put(app, cpu.eax + 0x6e0, 2);
    put(app, cpu.edx + 0x1c, 0);
    if ((byte(app, cpu.eax + 0x621) & 0x40) != 0)
        if (!byAddress(app, cpu, kResetCar))
            return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405740: a cop's driver, by its state [+0x6e0] through the table at
 * 0x405714: 0 the start (3 for a cop, its chase cleared; 2 else); 1 driving
 * (0x404c80, 0x40e350); 2 a racer's driving (with the lanes' votes, or
 * 0x40e350), 6 with 0x140 frames once 0x401500 says so; 3 patrolling and 4
 * chasing (the racers watched and clocked, the siren, the roadblock, the
 * pursuit started -- to 4 -- or kept up, 5 on a bust, 3 again without one to
 * chase); 5 the bust (0x40ce70, 0x401fb0, back to 3); 6 out of the race
 * (7 or dropped once it stands upright and slow, or its time is up); 7 the
 * end (to 2 and put back where 0x401500 says no, dropped where its record
 * says).  The store after state 0's ret (0x405791, [eax + 0x6e0] = 1 with
 * eax the state) is never reached: nothing jumps there, the table neither. */
void copDriver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    // Each call of the car's: eax the car, by address; false where the game stopped.
    const auto call = [&](reg32 target) {
        cpu.eax = cpu.edx;
        return byAddress(app, cpu, target);
    };
    const auto mine = [&](void (*function)(App*, CPU&)) {
        cpu.eax = cpu.edx;
        return direct(app, cpu, function);
    };
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x6e0);
    const reg32 target = cpu.eax <= 7 ? at(app, cpu.eax * 4 + 0x405714) : 0x40575a;
    switch (target)
    {
    default:  // 0x40575a
        cpu.ecx = at(app, kSecondPlayer);
        if (cpu.ecx != at(app, cpu.edx + 0x1f0))
            static_cast<void>(at(app, kPlayMode));  // cmp dword [0x6fd3b0], 2: its flags never used
        if ((byte(app, cpu.edx + 0x200) & 0x20) != 0)
        {
            put(app, cpu.edx + 0x624, 0);
            put(app, cpu.edx + 0x6e0, 3);
        }
        else
            put(app, cpu.edx + 0x6e0, 2);
        break;
    case 0x4057af:
        if (!call(0x404c80) || !call(0x40e350))
            return;
        break;
    case 0x4057c2:
        if (!call(0x404c80))
            return;
        if (at(app, kLaneVotes) != 0)
        {
            if (!call(0x40dd60) || !call(0x4096b0) || !call(0x40ddb0) || !call(0x409660) || !call(0x40de30))
                return;
        }
        else if (!call(0x40e350))
            return;
        if (!call(0x401500))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, kRaceFrame);
            cpu.bl = byte(app, cpu.edx + 0x200);
            put(app, cpu.edx + 0x6e0, 6);
            cpu.eax += 0x140;
            cpu.bl |= 2;
            put(app, cpu.edx + 0x6ec, cpu.eax);
            putByte(app, cpu.edx + 0x200, cpu.bl);
        }
        break;
    case 0x40583b:
        if (!call(0x404c80) || !call(0x40e350) || !mine(clockRacers) || !mine(watchRacers) || !mine(sirenSwitch)
            || !mine(callRoadblock) || !mine(startPursuit))
            return;
        if (at(app, cpu.edx + 0x624) != 0)
            put(app, cpu.edx + 0x6e0, 4);
        break;
    case 0x405888:
        if (!call(0x404c80) || !call(0x40e350) || !mine(clockRacers) || !mine(watchRacers) || !mine(sirenSwitch)
            || !mine(callRoadblock) || !mine(pursuitStep))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = cpu.edx;
            put(app, cpu.edx + 0x6e0, 5);
            if (!byAddress(app, cpu, 0x401e40))
                return;
        }
        if (at(app, cpu.edx + 0x624) == 0)
            put(app, cpu.edx + 0x6e0, 3);
        break;
    case 0x4058ea:
        if (!call(0x404c80))
            return;
        cpu.eax = cpu.edx;
        put(app, cpu.edx + 0x7b0, 0);
        if (!byAddress(app, cpu, 0x40ce70) || !call(0x401fb0))
            return;
        if (cpu.eax != 0)
            put(app, cpu.edx + 0x6e0, 3);
        break;
    case 0x405920:
        if (!call(0x404c80) || !call(0x40e350) || !call(0x408b30) || !call(0x408bd0) || !call(0x40de30))
            return;
        if (half(app, cpu.edx + 0x164) != 0)
            break;
        fld(cpu, f32(app, cpu.edx + 0x10c));
        if (!fcomp(cpu, f64(app, 0x5361cc)).above())
            break;
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        if (!fcomp(cpu, f64(app, 0x5361d4)).below())
        {
            cpu.eax = at(app, kRaceFrame);
            if (i32(cpu.eax) <= i32(at(app, cpu.edx + 0x6ec)))
                break;
        }
        cpu.eax = at(app, cpu.edx + 0x628);
        if (at(app, cpu.eax + 0x2c) != 0)
        {
            put(app, cpu.edx + 0x6e0, 7);
            break;
        }
        if (!mine(dropPursuit))
            return;
        break;
    case 0x4059b0:
        if (!call(0x404c80))
            return;
        cpu.eax = cpu.edx;
        put(app, cpu.edx + 0x7b0, 0);
        if (!byAddress(app, cpu, 0x40ce70) || !call(0x40ce70) || !call(0x401500))
            return;
        if (cpu.eax == 0)
        {
            cpu.bh = byte(app, cpu.edx + 0x200);
            put(app, cpu.edx + 0x6e0, 2);
            cpu.bh &= 0xfd;
            cpu.eax = cpu.edx;
            putByte(app, cpu.edx + 0x200, cpu.bh);
            if (!byAddress(app, cpu, kResetCar))
                return;
        }
        cpu.eax = at(app, cpu.edx + 0x628);
        if (at(app, cpu.eax + 0x2c) == 0)
            if (!mine(dropPursuit))
                return;
        break;
    }
    return leave();
}

/* 0x405a20: whether the place at eax is near car edx (eax 1; 0 for no car):
 * the bigger of the two gaps across the ground (x, z) and a quarter of the
 * smaller under 366. */
void nearCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    if (cpu.edx == 0)
    {
        cpu.eax = 0;
        return leave();
    }
    cpu.eax = at(app, cpu.edx + 0x98);
    put(app, cpu.ebp - 0x24, cpu.eax);
    cpu.eax = at(app, cpu.edx + 0x9c);
    put(app, cpu.ebp - 0x20, cpu.eax);
    cpu.eax = at(app, cpu.edx + 0xa0);
    put(app, cpu.ebp - 0x1c, cpu.eax);
    fld(cpu, f32(app, cpu.ecx + 8));
    fsub(cpu, f32(app, cpu.ebp - 0x1c));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 0x14);
    if (fcomp(cpu, f32(app, cpu.ebp - 0x14)).below())
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        put(app, cpu.ebp - 0x18, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.ebp - 0x14));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 0x18);
    }
    fld(cpu, Float(0.0));
    fld(cpu, f32(app, cpu.ecx));
    cpu.eax = at(app, cpu.ebp - 0x18);
    fsub(cpu, f32(app, cpu.ebp - 0x24));
    put(app, cpu.ebp - 0x10, cpu.eax);
    fstp(app, cpu, cpu.ebp - 0xc);
    if (fcomp(cpu, f32(app, cpu.ebp - 0xc)).below())
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.ebp - 0xc));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 8);
    }
    fld(cpu, f32(app, cpu.ebp - 8));
    if (fcomp(cpu, f32(app, cpu.ebp - 0x10)).above())
    {
        fld(cpu, f32(app, cpu.ebp - 0x10));
        fmul(cpu, f32(app, 0x5361dc));
        fadd(cpu, f32(app, cpu.ebp - 8));
    }
    else
    {
        fld(cpu, f32(app, cpu.ebp - 8));
        fmul(cpu, f32(app, 0x5361dc));
        fadd(cpu, f32(app, cpu.ebp - 0x10));
    }
    fstp(app, cpu, cpu.ebp - 4);
    fld(cpu, f32(app, cpu.ebp - 4));
    cpu.eax = fcomp(cpu, f32(app, 0x5361e0)).belowOrEqual() ? 1 : 0;
    return leave();
}

/* 0x405af0: the first human's car ([kHumans]) near car eax's place
 * (nearCar), or 0. */
void nearHuman(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x98);
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x9c);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0xa0);
    put(app, cpu.ebp - 4, cpu.eax);
    bool found = false;
    for (cpu.ecx = 0; i32(cpu.ecx) < i32(at(app, kHumanCount)); ++cpu.ecx)
    {
        cpu.ebx = cpu.ecx * 4;
        cpu.eax = cpu.ebp - 0xc;
        cpu.edx = at(app, cpu.ebx + kHumans);
        if (!direct(app, cpu, nearCar))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, cpu.ebx + kHumans);
            found = true;
            break;
        }
    }
    if (!found)
        cpu.eax = 0;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405c80: the first human's car within 50 slices of slice eax (0x40e120),
 * or 0. */
void humanNearSlice(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    bool found = false;
    for (cpu.ecx = 0; i32(cpu.ecx) < i32(at(app, kHumanCount)); ++cpu.ecx)
    {
        cpu.eax = at(app, cpu.ecx * 4 + kHumans);
        cpu.edx = cpu.ebx;
        cpu.eax = at(app, cpu.eax + 0x1c);
        if (!byAddress(app, cpu, kSliceGap))
            return;
        if (i32(cpu.eax) <= 0)
            cpu.eax = reg32(0) - cpu.eax;
        if (i32(cpu.eax) < 0x32)
        {
            cpu.eax = at(app, cpu.ecx * 4 + kHumans);
            found = true;
            break;
        }
    }
    if (!found)
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405cd0: whether traffic car eax (on the road, [+0x620] 2) is out of
 * every human's sight (eax 1): not near the human it was near ([+0x6d8])
 * nor any other (nearHuman, kept in [+0x6d8]).  A cop's flag is tested and
 * never used. */
void trafficUnwatched(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.ecx = cpu.eax;
    bool unwatched = false;
    if ((byte(app, cpu.eax + 0x620) & 2) != 0)
    {
        if ((byte(app, cpu.ecx + 0x200) & 0x10) != 0)
        {
            cpu.eax = at(app, cpu.ecx + 0x98);
            put(app, cpu.ebp - 0xc, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 0x9c);
            put(app, cpu.ebp - 8, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 0xa0);
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.eax = cpu.ebp - 0xc;
            cpu.edx = at(app, cpu.ecx + 0x6d8);
            if (!direct(app, cpu, nearCar))
                return;
            if (cpu.eax == 0)
            {
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, nearHuman))
                    return;
                put(app, cpu.ecx + 0x6d8, cpu.eax);
                unwatched = cpu.eax == 0;
            }
        }
        if (!unwatched)
            static_cast<void>(byte(app, cpu.ecx + 0x200));  // test byte [ecx + 0x200], 0x20: its flags never used
    }
    cpu.eax = unwatched ? 1 : 0;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405d40: extra car eax comes onto the road: near the first car
 * ([+0x6d8]), its life a random share (the game's random numbers, seed by
 * multiplier) of what 0x40eed0 gives, plus 1 ([+0x650]); counted among the
 * extra cars, on the road ([+0x620] 4), [+0x89] 0. */
void trafficStart(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, kFirstCar);
    cpu.ecx = at(app, kMultiplier);
    put(app, cpu.edx + 0x6d8, cpu.eax);
    cpu.eax = at(app, kSeed);
    cpu.eax *= cpu.ecx;
    put(app, kProduct, cpu.eax);
    cpu.ecx = at(app, kProduct);
    cpu.eax &= 0xffff;
    cpu.ecx &= 0xffff00;
    put(app, kSeed, cpu.eax);
    cpu.ecx >>= 8;
    if (!byAddress(app, cpu, 0x40eed0))
        return;
    cpu.eax *= cpu.ecx;
    cpu.eax >>= 16;
    putByte(app, cpu.edx + 0x89, 0);
    ++cpu.eax;
    put(app, cpu.edx + 0x650, cpu.eax);
    cpu.ebx = at(app, kExtraCars);
    cpu.ah = byte(app, cpu.edx + 0x620);
    ++cpu.ebx;
    cpu.ah |= 4;
    put(app, kExtraCars, cpu.ebx);
    putByte(app, cpu.edx + 0x620, cpu.ah);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405dc0: extra car eax leaves the road: one fewer extra car, off the
 * road ([+0x620] 4 cleared), [+0x89] 1; a traffic car's (0x40c590) and a
 * cop's (0x4029c0) own leaving. */
void trafficStop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, kExtraCars);
    cpu.ah = byte(app, cpu.eax + 0x620);
    cpu.bl = byte(app, cpu.edx + 0x200);
    --cpu.ecx;
    cpu.ah &= 0xfb;
    putByte(app, cpu.edx + 0x89, 1);
    put(app, kExtraCars, cpu.ecx);
    putByte(app, cpu.edx + 0x620, cpu.ah);
    if ((cpu.bl & 0x10) != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x40c590))
            return;
    }
    if ((byte(app, cpu.edx + 0x200) & 0x20) != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x4029c0))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405e20: an extra car's step: a traffic car's (0x40c210), a cop's
 * (0x402c70), 0 for any other. */
void extraCarStep(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.dl = byte(app, cpu.eax + 0x200);
    if ((cpu.dl & 0x10) != 0)
    {
        if (!byAddress(app, cpu, 0x40c210))
            return;
    }
    else if ((cpu.dl & 0x20) != 0)
    {
        if (!byAddress(app, cpu, 0x402c70))
            return;
    }
    else
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405e60: the AI's tuning, set as a race starts: the steering's gains and
 * limits (0x56ebd0..), the reversing's (0x56ec04..), the off-road's
 * (0x56ec30..) -- floats, in the code's order of stores. */
void setDrivingConstants(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    static const reg32 stores[][2] = {
        {0x56ebd0, 0x40e00000}, {0x56ebd4, 0x40a00000}, {0x56ebd8, 0x42000000}, {0x56ebdc, 0x42180000},
        {0x56ebe0, 0x40800000}, {0x56ebe4, 0x40200000}, {0x56ebf0, 0x40a00000}, {0x56ebe8, 0x3f4ccccd},
        {0x56ebec, 0x3f4ccccd}, {0x56ebf4, 0x41b2cfdf}, {0x56ebf8, 0x3e041893}, {0x56ebfc, 0x3c449ba6},
        {0x56ec08, 0x42180000}, {0x56ec0c, 0x40400000}, {0x56ec04, 0x41800000}, {0x56ec10, 0x40200000},
        {0x56ec14, 0x3fcccccd}, {0x56ec18, 0x3f4ccccd}, {0x56ec24, 0x3c23d70a}, {0x56ec28, 0x3ca3d70a},
        {0x56ec1c, 0x41200000}, {0x56ec30, 0x42000000}, {0x56ec34, 0x42200000}, {0x56ec38, 0x40c00000},
        {0x56ec44, 0x3f800000}, {0x56ec3c, 0x40400000}, {0x56ec40, 0x40400000}, {0x56ec48, 0x41200000},
        {0x56ec50, 0x3e4ccccd}, {0x56ec54, 0x3d23d70a},
    };
    for (const auto& store : stores)
        put(app, store[0], store[1]);
    cpu.eax = 0x3d23d70a;  // the last value loaded; the others' registers are popped
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405fb0: an empty hook -- push ebp, mov ebp, esp, pop ebp. */
void drivingNothing(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x405fd0: car eax's reversing, by edx: 0 on (one more reverse, [+0x658]),
 * for ebx frames where ebx is not 0 ([+0x7d0], [+0x620] 0x40); 1 off; any
 * other turned over -- off where it was on (left on while its time is not
 * up), else on as 0.  It leaves bl or bh, cl and edx as its code does. */
void setReverse(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.edx == 0)
    {
        cpu.edx = at(app, cpu.eax + 0x658);
        put(app, cpu.eax + 0x7d4, 1);
        ++cpu.edx;
        put(app, cpu.eax + 0x658, cpu.edx);
        if (cpu.ebx != 0)
        {
            cpu.edx = at(app, kRaceFrame);
            cpu.edx += cpu.ebx;
            put(app, cpu.eax + 0x7d0, cpu.edx);
            putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) | 0x40);
        }
        else
            putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) & 0xbf);
        return leave();
    }
    if (cpu.edx == 1)
    {
        cpu.bl = byte(app, cpu.eax + 0x620);
        put(app, cpu.eax + 0x7d4, 0);
        cpu.bl &= 0xbf;
        putByte(app, cpu.eax + 0x620, cpu.bl);
        return leave();
    }
    cpu.ecx = at(app, cpu.eax + 0x7d4);
    if (cpu.ecx == 1)
    {
        cpu.bh = byte(app, cpu.eax + 0x620);
        put(app, cpu.eax + 0x7d4, 0);
        if ((cpu.bh & 0x40) == 0)
            return leave();
        cpu.edx = at(app, kRaceFrame);
        if (i32(cpu.edx) >= i32(at(app, cpu.eax + 0x7d0)))
            return leave();
        put(app, cpu.eax + 0x7d4, cpu.ecx);
        return leave();
    }
    cpu.esi = at(app, cpu.eax + 0x658);
    put(app, cpu.eax + 0x7d4, 1);
    ++cpu.esi;
    put(app, cpu.eax + 0x658, cpu.esi);
    if (cpu.ebx != 0)
    {
        cpu.edx = at(app, kRaceFrame);
        cpu.cl = byte(app, cpu.eax + 0x620);
        cpu.edx += cpu.ebx;
        cpu.cl |= 0x40;
        put(app, cpu.eax + 0x7d0, cpu.edx);
        putByte(app, cpu.eax + 0x620, cpu.cl);
    }
    else
        putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) & 0xbf);
    return leave();
}

/* 0x4060d0: car eax's reverse ended where its time is up (setReverse 1);
 * while it reverses without one, its eased offset ([+0x7c8]) 0. */
void reverseCheck(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, cpu.eax + 0x7d4) == 1)
    {
        if ((byte(app, cpu.eax + 0x620) & 0x40) != 0)
        {
            cpu.edx = at(app, kRaceFrame);
            if (i32(cpu.edx) > i32(at(app, cpu.eax + 0x7d0)))
            {
                cpu.edx = 1;
                cpu.ebx = 0;
                if (!direct(app, cpu, setReverse))
                    return;
                cpu.ebp = pop(app, cpu);
                cpu.edx = pop(app, cpu);
                cpu.ebx = pop(app, cpu);
                cpu.esp += 4;
                return;
            }
        }
        put(app, cpu.eax + 0x7c8, 0);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406120: car eax, where [+0x168] is not 0 and it goes slower than 10,
 * turns its reversing over for 0x80 frames (setReverse 2). */
void reverseWhenSlow(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if ((at(app, cpu.eax + 0x168) & 0x7fffffff) != 0)
    {
        fld(cpu, f32(app, cpu.eax + 0x7c4));
        if (fcomp(cpu, f64(app, 0x5361ec)).below())
        {
            cpu.ebx = 0x80;
            cpu.edx = 2;
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, setReverse))
                return;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406160: as reverseWhenSlow, where its nose also points down past -0.5
 * (its nose's dot product with the vector edx hands on, 0x4e01f0). */
void reverseWhenTipped(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if ((at(app, cpu.eax + 0x168) & 0x7fffffff) != 0)
    {
        fld(cpu, f32(app, cpu.eax + 0x7c4));
        if (fcomp(cpu, f64(app, 0x5361f4)).below())
        {
            cpu.eax = cpu.ecx + 0xd8;
            if (!byAddress(app, cpu, kDot))
                return;
            if (fcomp(cpu, f64(app, 0x5361fc)).below())
            {
                cpu.ebx = 0x80;
                cpu.edx = 2;
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, setReverse))
                    return;
            }
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4061b0: the computer's car's engine: the revs it aims at -- its
 * performance's top revs ([+0x528] + 0x1c4) by 0.4 and 0.8 (or 200 short of
 * them) while the speed it wants is past its slice's speed and 10, by 0.2
 * and 0.5 else; while a gear change is under way ([+0x64c]) the change's
 * frames counted down (the substeps, chopped); reversing, the reverse gear's
 * revs at its speed; else the gear (2 up to [+8]) whose revs at its speed
 * come nearest the aim, its change started ([+0x10]) where it is another;
 * never under the idle ([+0x1c0]); the revs ([+0x590]) an eighth of the way
 * there. */
void engineRevs(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    cpu.edx = cpu.eax;
    magnitude(app, cpu, cpu.edx + 0x7b0, cpu.ebp - 0x28);
    fld(cpu, f32(app, cpu.ebp - 0x28));
    cpu.eax = at(app, cpu.edx + 0x1c);
    fstp64(app, cpu, cpu.ebp - 0x30);
    if (!byAddress(app, cpu, kSliceSpeed))
        return;
    fadd(cpu, f64(app, 0x536204));
    if (fcomp(cpu, f64(app, cpu.ebp - 0x30)).below())
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        fild(app, cpu, cpu.eax + 0x1c4);
        fmul(cpu, f64(app, 0x53620c));
        fld(cpu, Float(0.0));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 0x1c);
        if (fcomp(cpu, f32(app, cpu.edx + 0x7b0)).below())
        {
            cpu.eax = at(app, cpu.edx + 0x7b0);
            put(app, cpu.ebp - 0x20, cpu.eax);
        }
        else
        {
            fld(cpu, f32(app, cpu.edx + 0x7b0));
            fchs(cpu);
            fstp(app, cpu, cpu.ebp - 0x20);
        }
        fld(cpu, f32(app, cpu.ebp - 0x20));
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        fadd(cpu, f64(app, 0x536214));
        if (fcompp(cpu).below())
        {
            cpu.eax = at(app, cpu.edx + 0x528);
            cpu.eax = at(app, cpu.eax + 0x1c4);
            cpu.eax -= 0xc8;
            put(app, cpu.ebp - 4, cpu.eax);
            fild(app, cpu, cpu.ebp - 4);
        }
        else
        {
            cpu.eax = at(app, cpu.edx + 0x528);
            fild(app, cpu, cpu.eax + 0x1c4);
            fmul(cpu, f64(app, 0x53621c));
        }
        fstp(app, cpu, cpu.ebp - 0x14);
    }
    else
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        fild(app, cpu, cpu.eax + 0x1c4);
        fldSt(cpu, 0);
        fmul(cpu, f64(app, 0x536224));
        fxch(cpu, 1);
        fmul(cpu, f64(app, 0x53622c));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 0x1c);
        fstp(app, cpu, cpu.ebp - 0x14);
    }
    cpu.ecx = at(app, cpu.edx + 0x64c);
    bool counted = false;
    if (i32(cpu.ecx) > 0)
    {
        fld(cpu, f32(app, cpu.ebp - 0x1c));
        cpu.eax = at(app, kSubsteps);
        cpu.ebx = cpu.ecx;
        if (!byAddress(app, cpu, kChop))
            return;
        cpu.ebx -= cpu.eax;
        fistp(app, cpu, cpu.ebp - 8);
        put(app, cpu.edx + 0x64c, cpu.ebx);
        counted = true;
    }
    else if (at(app, cpu.edx + 0x7d4) == 1)
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        fld(cpu, f32(app, cpu.eax + 0x94));
        fchs(cpu);
        fmul(cpu, f32(app, cpu.edx + 0x7c4));
        cpu.ebx = 0;
        if (!byAddress(app, cpu, kChop))
            return;
        fistp(app, cpu, cpu.ebp - 8);
    }
    else
    {
        cpu.edi = 0x47c35000;  // 100000.0f, the nearest so far
        cpu.ecx = 2;
        put(app, cpu.ebp - 0x24, cpu.edi);
        for (;; ++cpu.ecx)
        {
            cpu.eax = at(app, cpu.edx + 0x528);
            if (i32(cpu.ecx) >= i32(at(app, cpu.eax + 8)))
                break;
            fld(cpu, f32(app, cpu.eax + cpu.ecx * 4 + 0x94));
            fmul(cpu, f32(app, cpu.edx + 0x7c4));
            fst(app, cpu, cpu.ebp - 0x18);
            fsub(cpu, f32(app, cpu.ebp - 0x14));
            fld(cpu, Float(0.0));
            fxch(cpu, 1);
            fstp(app, cpu, cpu.ebp - 0xc);
            if (fcomp(cpu, f32(app, cpu.ebp - 0xc)).below())
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                put(app, cpu.ebp - 0x10, cpu.eax);
            }
            else
            {
                fld(cpu, f32(app, cpu.ebp - 0xc));
                fchs(cpu);
                fstp(app, cpu, cpu.ebp - 0x10);
            }
            fld(cpu, f32(app, cpu.ebp - 0x10));
            if (fcomp(cpu, f32(app, cpu.ebp - 0x24)).below())
            {
                fld(cpu, f32(app, cpu.ebp - 0x18));
                cpu.eax = at(app, cpu.ebp - 0x10);
                cpu.ebx = cpu.ecx;
                if (!byAddress(app, cpu, kChop))
                    return;
                fistp(app, cpu, cpu.ebp - 8);
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
        }
    }
    if (!counted)
    {
        cpu.ecx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edi = at(app, cpu.ecx + 0x1c0);
        if (i32(cpu.eax) < i32(cpu.edi))
            put(app, cpu.ebp - 8, cpu.edi);
        cpu.eax = 0;
        cpu.al = byte(app, cpu.edx + 0x50b);
        if (cpu.eax != cpu.ebx)
        {
            cpu.eax = at(app, cpu.edx + 0x528);
            cpu.eax = at(app, cpu.eax + 0x10);
            put(app, cpu.edx + 0x64c, cpu.eax);
            putByte(app, cpu.edx + 0x50b, cpu.bl);
        }
    }
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.ecx = at(app, cpu.edx + 0x590);
    if (i32(cpu.eax) > i32(cpu.ecx))
    {
        cpu.eax -= cpu.ecx;
        cpu.eax = reg32(i32(cpu.eax) >> 3);
        cpu.esi = cpu.ecx + cpu.eax;
        put(app, cpu.edx + 0x590, cpu.esi);
    }
    else if (i32(cpu.eax) < i32(cpu.ecx))
    {
        cpu.eax = cpu.ecx;
        cpu.eax -= at(app, cpu.ebp - 8);
        cpu.ebx = at(app, cpu.edx + 0x590);
        cpu.eax = reg32(i32(cpu.eax) >> 3);
        cpu.ebx -= cpu.eax;
        put(app, cpu.edx + 0x590, cpu.ebx);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4063e0 (eax the car, a float on the stack, ret 4): the computer's car's
 * brake onto the x87's stack -- 0 where it brakes already ([+0x509] 0xff)
 * with no speed wanted and the shortfall under 0.5 in size; else its
 * performance's brake ([+0x528] + 0x1d8), doubled for a cop and again going
 * backwards, by 1.25 where [+0x200] 8 asks (0x404610's say unused); the
 * brake let off ([+0x509] 0) where it moves and the shortfall is past -5. */
void brakeForce(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    cpu.edx = cpu.eax;
    bool none = false;
    if (byte(app, cpu.eax + 0x509) == 0xff && (at(app, cpu.edx + 0x7b0) & 0x7fffffff) == 0)
    {
        magnitude(app, cpu, cpu.ebp + 0x10, cpu.ebp - 0xc);
        fld(cpu, f32(app, cpu.ebp - 0xc));
        if (fcomp(cpu, f64(app, 0x536234)).below())
        {
            cpu.ebx = 0;
            put(app, cpu.ebp - 4, cpu.ebx);
            none = true;
        }
    }
    if (!none)
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        putByte(app, cpu.edx + 0x509, 0xff);
        cpu.eax = at(app, cpu.eax + 0x1d8);
        cpu.bl = byte(app, cpu.edx + 0x200);
        put(app, cpu.ebp - 8, cpu.eax);
        // fld [ebp - 8]; fld st(0); fmul qword [factor]; fstp st(1); fstp [ebp - 8]
        const auto scale = [&](reg32 factor) {
            fld(cpu, f32(app, cpu.ebp - 8));
            fldSt(cpu, 0);
            fmul(cpu, f64(app, factor));
            fstpTo(cpu, 1);
            fstp(app, cpu, cpu.ebp - 8);
        };
        if ((cpu.bl & 0x20) != 0)
            scale(0x53623c);
        fld(cpu, Float(0.0));
        if (fcomp(cpu, f32(app, cpu.edx + 0x4f4)).above())
            scale(0x53623c);
        if ((byte(app, cpu.edx + 0x200) & 8) != 0)
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, 0x404610))
                return;
            scale(0x536244);
        }
        if ((at(app, cpu.edx + 0x7c4) & 0x7fffffff) != 0)
        {
            fld(cpu, f32(app, cpu.ebp + 0x10));
            if (fcomp(cpu, f64(app, 0x53624c)).above())
                putByte(app, cpu.edx + 0x509, 0);
        }
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x4066c0: the computer's car's speed along the road ([+0x7c0], signed by
 * the way it goes: +0x621 0x10 against the race's way, chopped to an
 * integer): more than 1 short of the speed it wants ([+0x7b0]), its pull
 * (0x4064f0) into [+0x4e8] and a step's worth of it added, no further than
 * the speed wanted; else its brake (brakeForce, the shortfall handed on),
 * negated, the same way; its size to [+0x7c4]. */
void speedAlongRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2c;
    cpu.edx = cpu.eax;
    cpu.eax = (byte(app, cpu.eax + 0x621) & 0x10) != 0 ? 1 : 0;
    cpu.eax ^= at(app, kBackwards);
    if (cpu.eax != 0)
    {
        cpu.ebx = 0;
        cpu.esi = 0xbff00000;
        put(app, cpu.ebp - 0x2c, cpu.ebx);
        put(app, cpu.ebp - 0x28, cpu.esi);
    }
    else
    {
        put(app, cpu.ebp - 0x2c, cpu.eax);
        put(app, cpu.ebp - 0x28, 0x3ff00000);
    }
    fld(cpu, f64(app, cpu.ebp - 0x2c));
    if (!byAddress(app, cpu, kChop))
        return;
    fistp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.ebp - 4, cpu.eax);
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fild(app, cpu, cpu.ebp - 4);
    fstp(app, cpu, cpu.ebp - 8);
    fmul(cpu, f32(app, cpu.ebp - 8));
    fld(cpu, f32(app, cpu.edx + 0x7c0));
    fmul(cpu, f32(app, cpu.ebp - 8));
    fsubp(cpu, 1);
    fstp(app, cpu, cpu.ebp - 0x14);
    cpu.ecx = at(app, cpu.ebp - 0x14);
    if (i32(cpu.ecx) > 0x3f800000)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kAiPull))
            return;
        fld(cpu, f32(app, cpu.ebp - 8));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.edx + 0x4e8);
        fmul(cpu, f32(app, cpu.edx + 0x4e8));
        fmul(cpu, f32(app, kStep));
        fadd(cpu, f32(app, cpu.edx + 0x7c0));
        cpu.ebx = at(app, cpu.ebp - 0xc);
        fstp(app, cpu, cpu.edx + 0x7c0);
        fld(cpu, f32(app, cpu.edx + 0x7c0));
        const bool notAbove = fcomp(cpu, f32(app, cpu.edx + 0x7b0)).belowOrEqual();
        const reg32 local = i32(cpu.ebx) > 0 ? cpu.ebp - 0x1c : cpu.ebp - 0x24;
        if (i32(cpu.ebx) > 0)
            cpu.eax = notAbove ? at(app, cpu.edx + 0x7c0) : at(app, cpu.edx + 0x7b0);
        else
            cpu.eax = notAbove ? at(app, cpu.edx + 0x7b0) : at(app, cpu.edx + 0x7c0);
        put(app, local, cpu.eax);
        cpu.eax = at(app, local);
        put(app, cpu.edx + 0x7c0, cpu.eax);
    }
    else
    {
        cpu.eax = cpu.edx;
        push(app, cpu, cpu.ecx);
        if (!direct(app, cpu, brakeForce))
            return;
        fld(cpu, f32(app, cpu.ebp - 8));
        fxch(cpu, 1);
        fchs(cpu);
        fstp(app, cpu, cpu.edx + 0x4e8);
        fmul(cpu, f32(app, cpu.edx + 0x4e8));
        fmul(cpu, f32(app, kStep));
        fadd(cpu, f32(app, cpu.edx + 0x7c0));
        cpu.esi = at(app, cpu.ebp - 0xc);
        fstp(app, cpu, cpu.edx + 0x7c0);
        fld(cpu, f32(app, cpu.edx + 0x7c0));
        const bool notAbove = fcomp(cpu, f32(app, cpu.edx + 0x7b0)).belowOrEqual();
        const reg32 local = i32(cpu.esi) > 0 ? cpu.ebp - 0x20 : cpu.ebp - 0x10;
        if (i32(cpu.esi) > 0)
            cpu.eax = notAbove ? at(app, cpu.edx + 0x7b0) : at(app, cpu.edx + 0x7c0);
        else
            cpu.eax = notAbove ? at(app, cpu.edx + 0x7c0) : at(app, cpu.edx + 0x7b0);
        put(app, local, cpu.eax);
        cpu.eax = at(app, local);
        put(app, cpu.edx + 0x7c0, cpu.eax);
    }
    magnitude(app, cpu, cpu.edx + 0x7c0, cpu.ebp - 0x18);
    cpu.eax = at(app, cpu.ebp - 0x18);
    put(app, cpu.edx + 0x7c4, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406880: how far car eax is from the lane's offset it wants, by 8, onto
 * the x87's stack ([+0x7b4] - [+0x7b8]). */
void laneError(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    fld(cpu, f32(app, cpu.eax + 0x7b4));
    fsub(cpu, f32(app, cpu.eax + 0x7b8));
    fmul(cpu, f64(app, 0x536284));
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4068a0: the computer's car glides along the road, its speed set rather
 * than driven (when a ghost: [+0x88]): its speed along the road
 * (speedAlongRoad), its speed's vector the nose by it plus the road's
 * across by the lane's error (0x4e0050, 0x4dffb0), no spin, its forward
 * speed [+0x4f4]; then 0x4a0980 with its way against the race's. */
void glideAlongRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    put(app, cpu.ebp - 4, cpu.eax);
    if (!direct(app, cpu, laneError))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    fstp(app, cpu, cpu.ebp - 8);
    if (!direct(app, cpu, speedAlongRoad))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edi = cpu.ebp - 0x14;
    cpu.esi = cpu.eax + 0x124;
    movsd3(app, cpu);
    cpu.esi = cpu.eax + 0xd8;
    cpu.eax = cpu.ebp - 0x14;
    push(app, cpu, cpu.eax);
    cpu.edx = cpu.ebp - 0x14;
    cpu.edi = cpu.ebp - 0x20;
    push(app, cpu, at(app, cpu.ebp - 8));
    cpu.eax = 1;
    movsd3(app, cpu);
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.eax + 0x7c4);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = cpu.ebp - 0x20;
    cpu.ecx = at(app, cpu.ebp - 4);
    push(app, cpu, cpu.eax);
    cpu.ebx = cpu.ebp - 0x14;
    cpu.edx = cpu.ebp - 0x20;
    push(app, cpu, at(app, cpu.ebp - 8));
    cpu.eax = 1;
    cpu.ecx += 0xa4;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ebp - 0x20;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kAddVectors))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax + 0xe4, 0);
    put(app, cpu.eax + 0xe8, 0);
    put(app, cpu.eax + 0xec, 0);
    fld(cpu, f32(app, cpu.eax + 0x7c0));
    cpu.dl = byte(app, cpu.eax + 0x621);
    fstp(app, cpu, cpu.eax + 0x4f4);
    cpu.edx = (cpu.dl & 0x10) != 0 ? 1 : 0;
    cpu.ecx = at(app, kBackwards);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx ^= cpu.ecx;
    if (!byAddress(app, cpu, 0x4a0980))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406980: the slice car eax looks ahead to: the slice its pace gives
 * (0x40e1b0: 1.7, 4 to 20 slices) -- that one on the track's kind 0 between
 * slices 0x225 and 0x235 -- else from 4 slices on (back, against the race's
 * way), on while the road keeps straight (each slice's curve against the
 * car's, 0x40ef70, past 0.7 in size) up to it. */
void lookAheadSlice(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ecx = (byte(app, cpu.eax + 0x621) & 0x10) != 0 ? 1 : 0;
    cpu.ecx ^= at(app, kBackwards);
    cpu.esi = cpu.ecx != 0 ? 0xffffffff : 1;
    push(app, cpu, 0x14);
    push(app, cpu, 4);
    push(app, cpu, 0x3fd9999a);
    cpu.ecx = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kSliceByPace))
        return;
    cpu.ebx = at(app, kTrackAi);
    cpu.edi = cpu.eax;
    if (!(cpu.ebx == 0 && i32(cpu.ecx) > 0x224 && i32(cpu.ecx) < 0x236))
    {
        // A slice on by esi, wrapped round the track.
        const auto step = [&](reg32 by) {
            if (i32(by) >= 0)
            {
                cpu.edx = at(app, kSliceCount);
                if (i32(cpu.eax) >= i32(cpu.edx))
                    cpu.eax -= cpu.edx;
            }
            else if (i32(cpu.eax) < 0)
                cpu.eax += at(app, kSliceCount);
        };
        cpu.ebx = cpu.esi * 4;
        cpu.eax = cpu.ecx + cpu.ebx;
        step(cpu.ebx);
        cpu.ebx = cpu.eax;
        put(app, cpu.ebp - 4, 0x3f800000);
        for (;;)
        {
            cpu.edx = cpu.edi;
            cpu.eax = cpu.ebx;
            if (!byAddress(app, cpu, kSliceGap))
                return;
            if (cpu.eax == 0)
                break;
            magnitudeBelow(app, cpu, cpu.ebp - 4, cpu.ebp - 8);
            fld(cpu, f32(app, cpu.ebp - 8));
            if (fcomp(cpu, f64(app, 0x53628c)).belowOrEqual())
                break;
            cpu.eax = cpu.ebx + cpu.esi;
            step(cpu.esi);
            cpu.ebx = cpu.eax;
            cpu.edx = cpu.eax;
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kSliceCurve))
                return;
            fstp(app, cpu, cpu.ebp - 4);
        }
        cpu.eax = cpu.ebx;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406a80: car eax's speed across the road two slices on, onto the x87's
 * stack: its speed's vector (+0xa4) dotted with the slice's right (the
 * signed bytes +0x18..+0x1a, by 512 and 1/65536). */
void sidewaysToRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.ecx = cpu.eax;
    cpu.edx = 2;
    if (!byAddress(app, cpu, kSliceAhead))
        return;
    cpu.edx = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kSlices);
    cpu.eax <<= 2;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x15);
    cpu.edx = reg32(i32(cpu.edx) >> 24);
    cpu.edx <<= 9;
    put(app, cpu.ebp - 4, cpu.edx);
    fld(cpu, f64(app, 0x536294));
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, cpu.fpu.st(1));
    fstp(app, cpu, cpu.ebp - 0x10);
    cpu.edx = at(app, cpu.eax + 0x16);
    cpu.edx = reg32(i32(cpu.edx) >> 24);
    cpu.edx <<= 9;
    put(app, cpu.ebp - 4, cpu.edx);
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, cpu.fpu.st(1));
    fstp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, cpu.eax + 0x17);
    cpu.eax = reg32(i32(cpu.eax) >> 24);
    cpu.eax <<= 9;
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    fmulp(cpu, 1);
    cpu.edx = cpu.ebp - 0x10;
    cpu.eax = cpu.ecx + 0xa4;
    fstp(app, cpu, cpu.ebp - 8);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406b00: the way from car eax to the car it chases ([+0x624]), whose
 * offset it takes up ([+0x7c8]). */
void towardTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    put(app, cpu.ebp - 4, cpu.esi);
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.eax + 0x98;
    cpu.eax = at(app, cpu.eax + 0x624);
    cpu.edx = cpu.eax + 0x98;
    cpu.ecx = cpu.ebp - 0x10;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    cpu.eax = at(app, cpu.esi + 0x624);
    cpu.edi = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.eax + 0x7b8);
    put(app, cpu.esi + 0x7c8, cpu.edx);
    cpu.esi = cpu.ebp - 0x10;
    movsd3(app, cpu);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406b60: the way from car eax to where the car it chases will be (its
 * place plus 0.4 of its speed), its offset taken up. */
void towardTargetAhead(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    put(app, cpu.ebp - 4, cpu.esi);
    cpu.esi = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x624);
    fld(cpu, f32(app, 0x53629c));
    fld(cpu, f32(app, cpu.eax + 0xa4));
    fmul(cpu, cpu.fpu.st(1));
    fstp(app, cpu, cpu.ebp - 0x10);
    cpu.eax = at(app, cpu.esi + 0x624);
    fld(cpu, f32(app, cpu.eax + 0xa8));
    fmul(cpu, cpu.fpu.st(1));
    fstp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, cpu.esi + 0x624);
    fmul(cpu, f32(app, cpu.eax + 0xac));
    fstp(app, cpu, cpu.ebp - 8);
    cpu.eax = at(app, cpu.esi + 0x624);
    cpu.ecx = cpu.ebp - 0x10;
    cpu.edx = cpu.eax + 0x98;
    cpu.ebx = cpu.ebp - 0x10;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kAddVectors))
        return;
    cpu.ecx = cpu.ebp - 0x1c;
    cpu.edx = cpu.ebp - 0x10;
    cpu.eax = 1;
    cpu.ebx = cpu.esi + 0x98;
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    cpu.eax = at(app, cpu.esi + 0x624);
    cpu.edi = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.eax + 0x7b8);
    put(app, cpu.esi + 0x7c8, cpu.edx);
    cpu.esi = cpu.ebp - 0x1c;
    movsd3(app, cpu);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* A slice's point (16.16 integers at the slice's record) by `scale`, three
 * floats to `to` -- the code of 0x406c00, 0x406c90 and 0x406d40 from its
 * fild on:
 *     fild x; fld scale; fxch; fmul st(1); fild y; fmul st(2); fild z;
 *     fmulp st(3); ...; fxch st(1); fstp [to]; fstp [to + 4]; fstp [to + 8]
 * the integers through [ebp - 4], eax the next of them on the way. */
void slicePoint(App* app, CPU& cpu, reg32 point, reg32 scale)
{
    cpu.eax = at(app, point);
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    cpu.eax = at(app, point + 4);
    fld(cpu, f64(app, scale));
    fxch(cpu, 1);
    fmul(cpu, cpu.fpu.st(1));
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, point + 8);
    fild(app, cpu, cpu.ebp - 4);
    put(app, cpu.ebp - 4, cpu.eax);
    fmul(cpu, cpu.fpu.st(2));
    fild(app, cpu, cpu.ebp - 4);
    fmulp(cpu, 3);
}

/* 0x406c00: the way from car eax to its slice's point. */
void towardSlice(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2c;
    put(app, cpu.ebp - 8, cpu.esi);
    cpu.edx = cpu.eax;
    cpu.esi = at(app, cpu.eax + 0x1c);
    cpu.eax = cpu.esi * 8;
    cpu.eax += cpu.esi;
    cpu.esi = at(app, kSlices);
    cpu.eax <<= 2;
    cpu.edi = cpu.ebp - 0x14;
    cpu.esi += cpu.eax;
    movsd3(app, cpu);
    slicePoint(app, cpu, cpu.ebp - 0x14, 0x5362a4);
    cpu.ebx = cpu.edx + 0x98;
    cpu.ecx = cpu.ebp - 0x2c;
    cpu.edx = cpu.ebp - 0x20;
    cpu.edi = at(app, cpu.ebp - 8);
    cpu.esi = cpu.ebp - 0x2c;
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 0x20);
    fstp(app, cpu, cpu.ebp - 0x1c);
    cpu.eax = 1;
    fstp(app, cpu, cpu.ebp - 0x18);
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    movsd3(app, cpu);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406c90: the way from car eax to its slice's point, the speed it wants
 * ([+0x7b0]) 32 by the way's length (0x4e06a0): it slows as it comes. */
void towardSliceSlowing(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    put(app, cpu.ebp - 0xc, cpu.esi);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.esi = at(app, cpu.eax + 0x1c);
    cpu.eax = cpu.esi * 8;
    cpu.eax += cpu.esi;
    cpu.esi = at(app, kSlices);
    cpu.eax <<= 2;
    cpu.edi = cpu.ebp - 0x18;
    cpu.esi += cpu.eax;
    movsd3(app, cpu);
    slicePoint(app, cpu, cpu.ebp - 0x18, 0x5362ac);
    cpu.ebx = at(app, cpu.ebp - 8);
    cpu.ecx = cpu.ebp - 0x30;
    cpu.edx = cpu.ebp - 0x24;
    cpu.ebx += 0x98;
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 0x24);
    fstp(app, cpu, cpu.ebp - 0x20);
    cpu.eax = 1;
    fstp(app, cpu, cpu.ebp - 0x1c);
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    cpu.eax = cpu.ebp - 0x30;
    if (!byAddress(app, cpu, kLength))
        return;
    fmul(cpu, f64(app, 0x5362b4));
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.edi = at(app, cpu.ebp - 0xc);
    cpu.esi = cpu.ebp - 0x30;
    fstp(app, cpu, cpu.eax + 0x7b0);
    movsd3(app, cpu);
    cpu.eax = at(app, cpu.ebp - 0xc);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406d40: the way from car eax to its lane at the slice it looks ahead to
 * (lookAheadSlice): the slice's point plus the road's across by the offset
 * it eases to (10 more to the left on the track's kind 0 between slices
 * 0x205 and 0x20f, keeping left). */
void towardLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x4c;
    put(app, cpu.ebp - 0xc, cpu.esi);
    put(app, cpu.ebp - 8, cpu.eax);
    if (!direct(app, cpu, lookAheadSlice))
        return;
    cpu.edx = 0;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 0x10, cpu.edx);
    if (i32(cpu.eax) < 0x210 && i32(cpu.eax) > 0x204 && at(app, kTrackAi) == 0)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        fld(cpu, f32(app, cpu.eax + 0x7c8));
        fld(cpu, Float(0.0));
        if (!fcompp(cpu).belowOrEqual())
            put(app, cpu.ebp - 0x10, 0xc1200000);
    }
    cpu.eax = cpu.esi * 8;
    cpu.eax += cpu.esi;
    cpu.esi = at(app, kSlices);
    cpu.eax <<= 2;
    cpu.edi = cpu.ebp - 0x34;
    cpu.esi += cpu.eax;
    movsd3(app, cpu);
    slicePoint(app, cpu, cpu.ebp - 0x34, 0x5362bc);
    cpu.edx = cpu.ebp - 0x1c;
    cpu.ecx = cpu.ebp - 0x40;
    cpu.edi = cpu.ebp - 0x1c;
    cpu.esi = at(app, cpu.ebp - 8);
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 0x28);
    fstp(app, cpu, cpu.ebp - 0x24);
    cpu.eax = cpu.ebp - 0x40;
    fstp(app, cpu, cpu.ebp - 0x20);
    cpu.esi += 0x124;
    push(app, cpu, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    movsd3(app, cpu);
    cpu.esp -= 4;
    fld(cpu, f32(app, cpu.eax + 0x7c8));
    fadd(cpu, f32(app, cpu.ebp - 0x10));
    cpu.eax = 1;
    fstp(app, cpu, cpu.esp);
    cpu.ebx = cpu.ebp - 0x40;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ebp - 0x28;
    cpu.eax = 1;
    cpu.edi = at(app, cpu.ebp - 0xc);
    if (!byAddress(app, cpu, kAddVectors))
        return;
    cpu.ebx = at(app, cpu.ebp - 8);
    cpu.ecx = cpu.ebp - 0x4c;
    cpu.edx = cpu.ebp - 0x40;
    cpu.eax = 1;
    cpu.ebx += 0x98;
    cpu.esi = cpu.ebp - 0x4c;
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    movsd3(app, cpu);
    cpu.eax = at(app, cpu.ebp - 0xc);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x406e50: the way car eax goes, a vector to [esi] (esi saved in ebx,
 * eax left it).  Chasing a car ([+0x624]): straight at it where it rides
 * with it ([+0x621] 0x80), where it leads it (a cop with [+0x711] 2 within
 * 4 slices: where it will be), or where [+0x621] 0x20 says; else its lane.
 * Chasing none: its slice's point slowing where [+0x621] 8 says; where
 * 0x40f110 says it is off its way, stuck time counted ([+0x7d8], while
 * slower than 3) -- its slice's point every 0x40 of it, its lane else, put
 * back on the road past 1000; else, not stuck: put back after 8 reverses
 * ([+0x658], cleared past 10 of speed), its slice's point after 4 where its
 * lane is past the road's lanes, its lane else. */
void wayToGo(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.ebx = cpu.esi;
    cpu.ecx = cpu.eax;
    // A way of the car's into [ebp - 0x10]: false where the game stopped.
    const auto way = [&](void (*function)(App*, CPU&)) {
        cpu.esi = cpu.ebp - 0x10;
        cpu.eax = cpu.ecx;
        return direct(app, cpu, function);
    };
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx != 0)
    {
        if ((byte(app, cpu.eax + 0x621) & 0x80) != 0)
        {
            if (!way(towardTarget))
                return;
        }
        else
        {
            bool done = false;
            if ((byte(app, cpu.ecx + 0x200) & 0x20) != 0 && (byte(app, cpu.ecx + 0x711) & 2) != 0)
            {
                cpu.eax = at(app, cpu.ecx + 0x624);
                cpu.edx = at(app, cpu.eax + 0x1c);
                cpu.eax = at(app, cpu.ecx + 0x1c);
                if (!byAddress(app, cpu, kSliceGap))
                    return;
                const bool ahead = i32(cpu.eax) > 0;
                cpu.eax = at(app, cpu.ecx + 0x624);
                cpu.edx = at(app, cpu.eax + 0x1c);
                cpu.eax = at(app, cpu.ecx + 0x1c);
                if (!byAddress(app, cpu, kSliceGap))
                    return;
                if (!ahead)
                    cpu.eax = reg32(0) - cpu.eax;
                if (i32(cpu.eax) < 4)
                {
                    if (!way(towardTargetAhead))
                        return;
                    done = true;
                }
            }
            if (!done)
            {
                if ((byte(app, cpu.ecx + 0x621) & 0x20) != 0)
                {
                    if (!way(towardTarget))
                        return;
                }
                else if (!way(towardLane))
                    return;
            }
        }
    }
    else if ((byte(app, cpu.eax + 0x621) & 8) != 0)
    {
        if (!way(towardSliceSlowing))
            return;
    }
    else
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x40f110))
            return;
        if (cpu.eax != 0)
        {
            magnitude(app, cpu, cpu.ecx + 0x4f4, cpu.ebp - 4);
            fld(cpu, f32(app, cpu.ebp - 4));
            if (fcomp(cpu, f64(app, 0x5362c4)).below())
                put(app, cpu.ecx + 0x7d8, at(app, cpu.ecx + 0x7d8) + 1);
            if ((byte(app, cpu.ecx + 0x7d8) & 0x40) != 0)
            {
                if (!way(towardSlice))
                    return;
            }
            else if (!way(towardLane))
                return;
            if (i32(at(app, cpu.ecx + 0x7d8)) > 0x3e8)
            {
                cpu.eax = cpu.ecx;
                if (!byAddress(app, cpu, kResetCar))
                    return;
                put(app, cpu.ecx + 0x7d8, 0);
            }
        }
        else
        {
            fld(cpu, f32(app, cpu.ecx + 0x7c4));
            put(app, cpu.ecx + 0x7d8, cpu.edx);
            bool lane = true;
            if (fcomp(cpu, f64(app, 0x5362cc)).above())
                put(app, cpu.ecx + 0x658, cpu.edx);
            else
            {
                cpu.eax = at(app, cpu.ecx + 0x658);
                if (i32(cpu.eax) > 8)
                {
                    cpu.eax = cpu.ecx;
                    if (!byAddress(app, cpu, kResetCar))
                        return;
                    put(app, cpu.ecx + 0x658, cpu.edx);
                    lane = false;
                }
                else if (i32(cpu.eax) > 4)
                {
                    cpu.esi = at(app, cpu.ecx + 0x1c);
                    cpu.eax = cpu.esi * 8;
                    cpu.esi += cpu.eax;
                    cpu.eax = at(app, kSlices);
                    cpu.esi <<= 2;
                    cpu.eax += cpu.esi;
                    cpu.esi = byte(app, cpu.eax + 0x13);
                    cpu.esi = reg32(i32(cpu.esi) >> 4);
                    cpu.edx = 6;
                    cpu.esi &= 0xf;
                    cpu.edi = at(app, cpu.ecx + 0x638);
                    cpu.edx -= cpu.esi;
                    bool past = i32(cpu.edx) > i32(cpu.edi);
                    if (!past)
                    {
                        cpu.al = byte(app, cpu.eax + 0x13);
                        cpu.al &= 0xf;
                        cpu.eax &= 0xff;
                        cpu.eax += 7;
                        past = i32(cpu.eax) < i32(cpu.edi);
                    }
                    if (past)
                    {
                        if (!way(towardSlice))
                            return;
                        lane = false;
                    }
                }
            }
            if (lane && !way(towardLane))
                return;
        }
    }
    cpu.esi = cpu.ebp - 0x10;
    cpu.edi = cpu.ebx;
    movsd3(app, cpu);
    cpu.eax = cpu.ebx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4071a0: the computer's car's step integrated: the force in its axes
 * (+0x4e0) and the push handed on ([kPush], in the road's axes) into the
 * world's (+0x2e8, through the axes transposed, 0x4e0110), the spin's
 * change (+0x2f4); its speed by a step of the acceleration, damped
 * ([kDamping], the substeps' times over), its spin by a step of the spin's
 * change. */
void integrate(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x58;
    cpu.ecx = cpu.eax;
    cpu.edx = cpu.ebp - 0x58;
    cpu.eax += 0xc0;
    if (!byAddress(app, cpu, kTranspose))
        return;
    cpu.edx = cpu.ebp - 0x34;
    cpu.eax = cpu.ecx + 0x124;
    if (!byAddress(app, cpu, kTranspose))
        return;
    cpu.ebx = cpu.ecx + 0x4e0;
    cpu.edx = cpu.ebp - 0x58;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x4c;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x2e8);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x40;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x2ec);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x34;
    cpu.eax = kPush;
    fstp(app, cpu, cpu.ecx + 0x2f0);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.ebx = cpu.ecx + 0x2f4;
    cpu.edx = cpu.ebp - 0x28;
    fadd(cpu, f32(app, cpu.ecx + 0x2e8));
    cpu.eax = kPush;
    fstp(app, cpu, cpu.ecx + 0x2e8);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x1c;
    fadd(cpu, f32(app, cpu.ecx + 0x2ec));
    cpu.eax = kPush;
    fstp(app, cpu, cpu.ecx + 0x2ec);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x58;
    fadd(cpu, f32(app, cpu.ecx + 0x2f0));
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x2f0);
    if (!byAddress(app, cpu, kDot))
        return;
    fstp(app, cpu, cpu.ebp - 0x10);
    cpu.edx = cpu.ebp - 0x4c;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ebp - 0x40;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ebp - 0xc);
    if (!byAddress(app, cpu, kDot))
        return;
    fld(cpu, f32(app, kStep));
    fld(cpu, f32(app, cpu.ecx + 0x2e8));
    fmul(cpu, cpu.fpu.st(1));
    fld(cpu, f32(app, cpu.ecx + 0x2ec));
    fmul(cpu, cpu.fpu.st(2));
    fld(cpu, f32(app, cpu.ecx + 0x2f0));
    cpu.eax = at(app, cpu.ebp - 0x10);
    fmulp(cpu, 3);
    put(app, cpu.ecx + 0x2f4, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0xc);
    fxch(cpu, 3);
    fstp(app, cpu, cpu.ebp - 8);
    put(app, cpu.ecx + 0x2f8, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.ecx + 0x2fc, cpu.eax);
    fadd(cpu, f32(app, cpu.ecx + 0xa4));
    fxch(cpu, 2);
    fadd(cpu, f32(app, cpu.ecx + 0xa8));
    fxch(cpu, 1);
    fadd(cpu, f32(app, cpu.ecx + 0xac));
    fxch(cpu, 2);
    fstp(app, cpu, cpu.ecx + 0xa4);
    fstp(app, cpu, cpu.ecx + 0xa8);
    fstp(app, cpu, cpu.ecx + 0xac);
    cpu.edx = at(app, kDamping);
    if ((cpu.edx & 0x7fffffff) != 0)
    {
        cpu.eax = cpu.edx;
        put(app, cpu.ebp - 4, cpu.edx);
        cpu.eax ^= cpu.edx;
        while (i32(cpu.eax) < i32(at(app, kSubsteps)))
        {
            fld(cpu, f32(app, cpu.ebp - 4));
            fld(cpu, f32(app, cpu.ecx + 0xa4));
            fmul(cpu, cpu.fpu.st(1));
            fld(cpu, f32(app, cpu.ecx + 0xa8));
            fmul(cpu, cpu.fpu.st(2));
            fld(cpu, f32(app, cpu.ecx + 0xac));
            fmulp(cpu, 3);
            ++cpu.eax;
            fstp(app, cpu, cpu.ecx + 0xa8);
            fxch(cpu, 1);
            fstp(app, cpu, cpu.ecx + 0xac);
            fstp(app, cpu, cpu.ecx + 0xa4);
        }
    }
    fld(cpu, f32(app, kStep));
    fld(cpu, f32(app, cpu.ecx + 0x2f4));
    fmul(cpu, cpu.fpu.st(1));
    fld(cpu, f32(app, cpu.ecx + 0x2f8));
    fmul(cpu, cpu.fpu.st(2));
    fld(cpu, f32(app, cpu.ecx + 0x2fc));
    fmulp(cpu, 3);
    fxch(cpu, 1);
    fadd(cpu, f32(app, cpu.ecx + 0xe4));
    fxch(cpu, 1);
    fadd(cpu, f32(app, cpu.ecx + 0xe8));
    fxch(cpu, 2);
    fadd(cpu, f32(app, cpu.ecx + 0xec));
    fxch(cpu, 2);
    fstp(app, cpu, cpu.ecx + 0xe8);
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ecx + 0xec);
    fstp(app, cpu, cpu.ecx + 0xe4);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The product the code inlines where a vector goes through a matrix: the
 * vector at eax, row by row, times the matrix at edx -- [ebx + 4j] =
 * (v0 m[j] + v1 m[3 + j]) + v2 m[6 + j] -- the x87's stack as the code has
 * it (three products, fxch st(2), faddp st(3) three times), eax left 0xc on
 * (lea eax, [eax + 0xc]). */
void transform(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    for (reg32 row = 0; row < 3; ++row)
    {
        for (reg32 j = 0; j < 3; ++j)
        {
            fpu.push(f32(app, cpu.eax + 4 * row));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc * row + 4 * j));
        }
        fxch(cpu, 2);
        if (row != 0)
            for (int k = 0; k < 3; ++k)
                faddp(cpu, 3);
    }
    cpu.eax += 0xc;
    fstp(app, cpu, cpu.ebx);
    fstp(app, cpu, cpu.ebx + 4);
    fstp(app, cpu, cpu.ebx + 8);
}

/* A float clamped from above the code's way, the value on the x87's stack:
 *     fst [value]; fcomp [limit]; ... jbe +: mov eax, [limit]; jmp ++
 *     +: mov eax, [value];  ++: mov [to], eax
 * -- the value compared as the x87 holds it, before its store's rounding. */
void stAtMost(App* app, CPU& cpu, reg32 value, reg32 limit, reg32 to)
{
    fst(app, cpu, value);
    cpu.eax = fcomp(cpu, f32(app, limit)).belowOrEqual() ? at(app, value) : at(app, limit);
    put(app, to, cpu.eax);
}

/* And from below, by the limit negated through [negated]:
 *     fld [value]; fld [limit]; fchs; fstp [negated]; fcomp [negated]; ...
 *     jbe +: mov eax, [negated]; jmp ++;  +: ... mov eax, [value]
 *     ++: mov [to], eax */
void atLeastNegated(App* app, CPU& cpu, reg32 value, reg32 limit, reg32 negated, reg32 to)
{
    fld(cpu, f32(app, value));
    fld(cpu, f32(app, limit));
    fchs(cpu);
    fstp(app, cpu, negated);
    cpu.eax = fcomp(cpu, f32(app, negated)).belowOrEqual() ? at(app, negated) : at(app, value);
    put(app, to, cpu.eax);
}

/* A limit worked out on the x87's stack, the value clamped by it from above:
 *     fld [value]; fxch st(1); fstp [limit]; fcomp [limit]; ...
 *     jbe +: mov eax, [value]; ...  else mov eax, [limit]; mov [to], eax */
void atMostComputed(App* app, CPU& cpu, reg32 value, reg32 limit, reg32 to)
{
    fld(cpu, f32(app, value));
    fxch(cpu, 1);
    fstp(app, cpu, limit);
    cpu.eax = fcomp(cpu, f32(app, limit)).belowOrEqual() ? at(app, value) : at(app, limit);
    put(app, to, cpu.eax);
}

/* A limit's size worked out with 0 below it on the x87's stack:
 *     fldz; mov [moved], eax; fxch st(1); fstp [limit]; fcomp [limit]; jae +
 *     mov eax, [limit]; mov [size], eax; jmp ++;  +: fld; fchs; fstp [size]
 * (the code's steering's grip limits, eax handed on to [moved] first). */
void limitSize(App* app, CPU& cpu, reg32 moved, reg32 limit, reg32 size)
{
    fld(cpu, Float(0.0));
    put(app, moved, cpu.eax);
    fxch(cpu, 1);
    fstp(app, cpu, limit);
    if (fcomp(cpu, f32(app, limit)).below())
    {
        cpu.eax = at(app, limit);
        put(app, size, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, limit));
        fchs(cpu);
        fstp(app, cpu, size);
    }
}

/* fld [value]; fcomp [size]; ... jbe +: mov eax, [size]; ... else
 * mov eax, [value]; mov [to], eax -- with the size's bits handed on to
 * [moved] in between (mov eax, [size]; ... mov [moved], eax). */
void atMostSize(App* app, CPU& cpu, reg32 value, reg32 size, reg32 moved, reg32 to)
{
    cpu.eax = at(app, size);
    fld(cpu, f32(app, value));
    put(app, moved, cpu.eax);
    cpu.eax = fcomp(cpu, f32(app, size)).belowOrEqual() ? at(app, value) : at(app, size);
    put(app, to, cpu.eax);
}

/* The steering's epilogue: lea esp, [ebp + 0x82]; pop ebp .. ebx; ret. */
void steerLeave(App* app, CPU& cpu)
{
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

void steerEnter(App* app, CPU& cpu, reg32 frame)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= frame;
    cpu.ebp -= 0x82;
}

/* A copy of a double the code's way, by its two dwords through eax. */
void copyDouble(App* app, CPU& cpu, reg32 from, reg32 to)
{
    cpu.eax = at(app, from);
    put(app, to, cpu.eax);
    cpu.eax = at(app, from + 4);
    put(app, to + 4, cpu.eax);
}

/* A double's size the code's way: fld qword [ebp...] as computed, with 0
 * below it -- fldz; fxch st(1); fstp qword [value]; fcomp qword [value];
 * jbe +: fld; fchs; fstp qword [size]; jmp ++;  +: the two dwords moved. */
void doubleSize(App* app, CPU& cpu, reg32 value, reg32 size)
{
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp64(app, cpu, value);
    if (fcomp(cpu, f64(app, value)).belowOrEqual())
        copyDouble(app, cpu, value, size);
    else
    {
        fld(cpu, f64(app, value));
        fchs(cpu);
        fstp64(app, cpu, size);
    }
}

}  // namespace

/* 0x407380: a finished car coasts (while racing and upright: [+0x10c] at
 * least 0.9): its pedals, steering and skid let go, the brake on, its
 * forward speed damped (0.95, 0.98 with grip past 22.35), across 0.85 and
 * its yaw 0.5, halved again between -10 and 5 forward; a skid of a tenth of
 * its speed, more with its yaw past 0.2, its across where that is the
 * bigger (cut to 0.2); its speed and spin back to the world's through its
 * axes. */
void coastAfterFinish(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x34;
    cpu.ecx = cpu.eax;
    if (half(app, cpu.eax + 0x164) == 0 && i32(at(app, cpu.eax + 0x10c)) >= 0x3f666666)
    {
        put(app, cpu.eax + 0x5c0, 0);
        put(app, cpu.eax + 0x5c4, 0);
        put(app, cpu.eax + 0x594, 0);
        put(app, cpu.eax + 0x5b0, 0);
        put(app, cpu.eax + 0x51c, 0);
        putByte(app, cpu.eax + 0x508, 0);
        putByte(app, cpu.eax + 0x50c, 0);
        putByte(app, cpu.eax + 0x50d, 0);
        fld(cpu, f32(app, cpu.eax + 0xb8));
        putByte(app, cpu.eax + 0x509, 0xff);
        const bool slippery = fcomp(cpu, f64(app, 0x536304)).below();
        fld(cpu, f32(app, cpu.ecx + 0x4f4));
        fldSt(cpu, 0);
        fmul(cpu, f64(app, slippery ? 0x53630c : 0x536314));
        fstpTo(cpu, 1);
        fstp(app, cpu, cpu.ecx + 0x4f4);
        fld(cpu, f32(app, cpu.ecx + 0x4ec));
        fldSt(cpu, 0);
        fmul(cpu, f64(app, 0x53631c));
        fld(cpu, f32(app, cpu.ecx + 0x4fc));
        fldSt(cpu, 0);
        fmul(cpu, f64(app, 0x536324));
        fld(cpu, f32(app, cpu.ecx + 0x4f4));
        fst64(app, cpu, cpu.ebp - 0x24);
        fxch(cpu, 3);
        fstpTo(cpu, 4);
        fstpTo(cpu, 1);
        fxch(cpu, 2);
        fstp(app, cpu, cpu.ecx + 0x4ec);
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ecx + 0x4fc);
        bool halved = false;
        if (fcomp(cpu, f64(app, 0x53632c)).below())
        {
            fld(cpu, f64(app, cpu.ebp - 0x24));
            if (fcomp(cpu, f64(app, 0x536334)).above())
            {
                fld(cpu, f64(app, cpu.ebp - 0x24));
                fmul(cpu, f64(app, 0x536324));
                fstp(app, cpu, cpu.ecx + 0x4f4);
                halved = true;
            }
        }
        if (!halved)
        {
            magnitudeBelow(app, cpu, cpu.ecx + 0x4f4, cpu.ebp - 4);
            fld(cpu, f32(app, cpu.ebp - 4));
            fmul(cpu, f64(app, 0x53633c));
            fstp(app, cpu, cpu.ecx + 0x5c4);
        }
        magnitudeBelow(app, cpu, cpu.ecx + 0x4fc, cpu.ebp - 0xc);
        fld(cpu, f32(app, cpu.ebp - 0xc));
        if (fcomp(cpu, f64(app, 0x536344)).above())
        {
            fld(cpu, Float(0.0));
            cpu.ebx = cpu.ecx + 0x5c4;
            if (fcomp(cpu, f32(app, cpu.ecx + 0x4fc)).belowOrEqual())
            {
                cpu.eax = at(app, cpu.ecx + 0x4fc);
                put(app, cpu.ebp - 8, cpu.eax);
            }
            else
            {
                fld(cpu, f32(app, cpu.ecx + 0x4fc));
                fchs(cpu);
                fstp(app, cpu, cpu.ebp - 8);
            }
            fld(cpu, f32(app, cpu.ebx));
            fadd(cpu, f32(app, cpu.ebp - 8));
            fstp(app, cpu, cpu.ebx);
        }
        magnitudeBelow(app, cpu, cpu.ecx + 0x4ec, cpu.ebp - 0x10);
        magnitudeBelow(app, cpu, cpu.ecx + 0x4f4, cpu.ebp - 0x18);
        fld(cpu, f32(app, cpu.ebp - 0x10));
        if (fcomp(cpu, f32(app, cpu.ebp - 0x18)).above())
        {
            // The size of the speed across twice over -- the second time by
            // the flags of the first compare (jbe at 0x4075a4 with no
            // instruction setting them between).
            fld(cpu, Float(0.0));
            const bool notBelow = fcomp(cpu, f32(app, cpu.ecx + 0x4ec)).belowOrEqual();
            if (notBelow)
            {
                cpu.eax = at(app, cpu.ecx + 0x4ec);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
            else
            {
                fld(cpu, f32(app, cpu.ecx + 0x4ec));
                fchs(cpu);
                fstp(app, cpu, cpu.ebp - 0x1c);
            }
            cpu.eax = at(app, cpu.ebp - 0x1c);
            put(app, cpu.ecx + 0x5c0, cpu.eax);
            if (notBelow)
            {
                cpu.eax = at(app, cpu.ecx + 0x4ec);
                put(app, cpu.ebp - 0x14, cpu.eax);
            }
            else
            {
                fld(cpu, f32(app, cpu.ecx + 0x4ec));
                fchs(cpu);
                fstp(app, cpu, cpu.ebp - 0x14);
            }
            fld(cpu, f32(app, cpu.ecx + 0x4ec));
            cpu.eax = at(app, cpu.ebp - 0x14);
            fldSt(cpu, 0);
            fmul(cpu, f64(app, 0x536344));
            put(app, cpu.ecx + 0x5c4, cpu.eax);
            fstpTo(cpu, 1);
            fstp(app, cpu, cpu.ecx + 0x4ec);
        }
        cpu.ebx = cpu.ecx + 0xa4;
        cpu.esi = cpu.ecx + 0xc0;
        cpu.eax = cpu.ecx + 0x4ec;
        cpu.edx = cpu.esi;
        transform(app, cpu);
        cpu.ebx = cpu.ecx + 0xe4;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx + 0x4f8;
        transform(app, cpu);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4076b0: the computer's car steered on the road.  The yaw it wants: its
 * heading's error (aiHeading's, [kHeadingError]) by a gain, cut to a limit,
 * plus its lane's error ([+0x7b8] against the eased [+0x7c8]) by a gain, cut
 * to a limit and to half its grip, cut further by its speed under 22.35; its
 * yaw's error ([+0x4fc]) by a gain and a limit: the spin's change (+0x2f8).
 * The push sideways: its speed across the road (sidewaysToRoad) less the
 * yaw wanted, by the way it goes, cut to gains scaled where it pulls hard
 * ([+0x7bc] past 1).  Along: its pull (0x4064f0) or brake (brakeForce) by
 * whether it wants more speed, less the turn's drag (its forward speed past
 * 44.7: the push by the speed's half, its performance's +0x248 for a racer),
 * by the way it goes; a skid past 7 across.  The push to [kPush] (in the
 * road's axes), the forces in its own axes cleared. */
void steerOnRoad(App* app, CPU& cpu)
{
    steerEnter(app, cpu, 0xfc);
    const auto L = [&](i32 offset) { return cpu.ebp + reg32(offset); };
    cpu.edx = cpu.eax;
    cpu.ecx = 0;
    cpu.ah = byte(app, cpu.eax + 0x621);
    put(app, L(-0x1a), cpu.ecx);
    cpu.eax = (cpu.ah & 0x10) != 0 ? 1 : 0;
    cpu.eax ^= at(app, kBackwards);
    cpu.ecx = cpu.eax != 0 ? 0xffffffff : 1;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, sidewaysToRoad))
        return;
    fld(cpu, f32(app, kHeadingError));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ebe4));
    cpu.esi = 0x3f800000;
    fxch(cpu, 1);
    fstp(app, cpu, L(0x62));
    put(app, L(2), cpu.esi);
    stAtMost(app, cpu, L(0x7a), 0x56ebe8, L(0x36));
    atLeastNegated(app, cpu, L(0x36), 0x56ebe8, L(-0x1e), L(0x22));
    fld(cpu, f32(app, cpu.edx + 0x7b8));
    fsub(cpu, f32(app, cpu.edx + 0x7c8));
    cpu.eax = at(app, L(0x22));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ebd4));
    put(app, L(0x5a), cpu.eax);
    stAtMost(app, cpu, L(6), 0x56ebd8, L(0x52));
    atLeastNegated(app, cpu, L(0x52), 0x56ebd8, L(-6), L(0x46));
    fld(cpu, f32(app, cpu.edx + 0xb8));
    fmul(cpu, f64(app, 0x53634c));
    atMostComputed(app, cpu, L(0x46), L(0x2a), L(0x42));
    atLeastNegated(app, cpu, L(0x42), L(0x2a), L(-0xe), L(0x5e));
    cpu.eax = at(app, L(0x5e));
    fld(cpu, f32(app, cpu.edx + 0x7c4));
    put(app, L(0xe), cpu.eax);
    if (fcomp(cpu, f32(app, 0x56ebf4)).below())
    {
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        fmul(cpu, f32(app, 0x56ebf8));
        atMostComputed(app, cpu, L(0x5e), L(0x4e), L(0x26));
        atLeastNegated(app, cpu, L(0x26), L(0x4e), L(-0x16), L(-0x12));
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        fmul(cpu, f32(app, 0x56ebfc));
        cpu.eax = at(app, L(-0x12));
        fld(cpu, f32(app, L(0x5a)));
        put(app, L(0xe), cpu.eax);
        fxch(cpu, 1);
        fstp(app, cpu, L(0x4a));
        cpu.eax = fcomp(cpu, f32(app, L(0x4a))).belowOrEqual() ? at(app, L(0x5a)) : at(app, L(0x4a));
        put(app, L(0x3a), cpu.eax);
        atLeastNegated(app, cpu, L(0x3a), L(0x4a), L(-2), L(0x56));
        cpu.eax = at(app, L(0x56));
        put(app, L(0x5a), cpu.eax);
    }
    fld(cpu, f32(app, cpu.edx + 0x4fc));
    fsub(cpu, f32(app, L(0x5a)));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ebf0));
    stAtMost(app, cpu, L(0xa), 0x56ebec, L(0x2e));
    atLeastNegated(app, cpu, L(0x2e), 0x56ebec, L(0x12), L(-0x2e));
    // The push's gains: across ([0x56ebdc]) and along ([0x56ebe0]) by its
    // forward speed by 0.0015 plus 1, scaled by its pull's share where that
    // is past 1.
    fld(cpu, f32(app, cpu.edx + 0x4f4));
    fmul(cpu, f32(app, 0x536354));
    fld(cpu, f32(app, 0x56ebdc));
    fld(cpu, Float(1.0));
    faddp(cpu, 2);
    fmul(cpu, cpu.fpu.st(1));
    fld(cpu, f32(app, 0x56ebe0));
    fmulp(cpu, 2);
    fld(cpu, f32(app, cpu.edx + 0x7bc));
    cpu.eax = at(app, L(-0x2e));
    fld(cpu, Float(1.0));
    put(app, L(-0x22), cpu.eax);
    fxch(cpu, 2);
    fstp(app, cpu, L(0x6a));
    fxch(cpu, 2);
    fstp(app, cpu, L(0x32));
    if (fcompp(cpu).below())
    {
        fld(cpu, f32(app, cpu.edx + 0x7bc));
        fmul(cpu, f32(app, L(0x32)));
        fld(cpu, f32(app, L(0x6a)));
        fmul(cpu, f32(app, cpu.edx + 0x7bc));
        fxch(cpu, 1);
        fstp(app, cpu, L(0x32));
        fstp(app, cpu, L(0x6a));
    }
    cpu.eax = cpu.ecx;
    cpu.eax = reg32(0) - cpu.eax;
    fld(cpu, f32(app, L(0x62)));
    put(app, L(0x7e), cpu.eax);
    fsub(cpu, f32(app, L(0xe)));
    fild(app, cpu, L(0x7e));
    fmulp(cpu, 1);
    fmul(cpu, f32(app, L(0x32)));
    stAtMost(app, cpu, L(0x16), L(0x6a), L(0x3e));
    atLeastNegated(app, cpu, L(0x3e), L(0x6a), L(0x76), L(0x1a));
    put(app, L(0x7e), cpu.ecx);
    fild(app, cpu, L(0x7e));
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fmul(cpu, cpu.fpu.st(1));
    fld(cpu, f32(app, L(0x1a)));
    fmul(cpu, f32(app, L(2)));
    fld(cpu, f32(app, cpu.edx + 0x7c0));
    fmulp(cpu, 3);
    fld(cpu, Float(0.0));
    fxch(cpu, 3);
    fsubp(cpu, 2);
    fstp(app, cpu, L(0x66));
    fstp(app, cpu, L(-0x26));
    if (fcomp(cpu, f32(app, L(-0x26))).below())
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kAiPull))
            return;
    }
    else
    {
        cpu.eax = cpu.edx;
        push(app, cpu, at(app, L(-0x26)));
        if (!direct(app, cpu, brakeForce))
            return;
        fchs(cpu);
    }
    fstp(app, cpu, L(0x6e));
    fld(cpu, Float(0.0));
    if (fcomp(cpu, f32(app, L(0x6e))).above())
        put(app, L(-0x1a), 0x3f800000);
    magnitudeBelow(app, cpu, cpu.edx + 0x4ec, L(0x1e));
    fld(cpu, f32(app, cpu.edx + 0x4f4));
    if (fcomp(cpu, f32(app, L(0x1e))).above())
    {
        fld(cpu, Float(0.0));
        if (fcomp(cpu, f32(app, L(0x6e))).below())
        {
            fld(cpu, f32(app, cpu.edx + 0x4f4));
            fst64(app, cpu, L(-0x7a));
            if (fcomp(cpu, f64(app, 0x53635c)).above())
            {
                // The turn's drag: the push across over half the forward
                // speed, its sign kept by the way the push goes.
                fld(cpu, f64(app, L(-0x7a)));
                fmul(cpu, f64(app, 0x53634c));
                doubleSize(app, cpu, L(-0x4a), L(-0x72));
                fld(cpu, f32(app, L(0x66)));
                fdiv(cpu, f64(app, L(-0x72)));
                fld(cpu, Float(0.0));
                if (fcompp(cpu).above())
                {
                    fld(cpu, f32(app, cpu.edx + 0x4f4));
                    fmul(cpu, f64(app, 0x53634c));
                    doubleSize(app, cpu, L(-0x52), L(-0x6a));
                    fld(cpu, f32(app, L(0x66)));
                    fdiv(cpu, f64(app, L(-0x6a)));
                    fchs(cpu);
                }
                else
                {
                    fld(cpu, f32(app, cpu.edx + 0x4f4));
                    fmul(cpu, f64(app, 0x53634c));
                    doubleSize(app, cpu, L(-0x3a), L(-0x42));
                    fld(cpu, f32(app, L(0x66)));
                    fdiv(cpu, f64(app, L(-0x42)));
                }
                fstp64(app, cpu, L(-0x62));
                fld(cpu, f64(app, L(-0x62)));
                cpu.bl = byte(app, cpu.edx + 0x200);
                fstp(app, cpu, L(0x72));
                if ((cpu.bl & 1) != 0)
                {
                    cpu.eax = at(app, cpu.edx + 0x528);
                    if ((at(app, cpu.eax + 0x248) & 0x7fffffff) != 0)
                    {
                        fld(cpu, f32(app, cpu.eax + 0x248));
                        fdivr(cpu, f64(app, 0x536364));
                        fstp(app, cpu, L(0x72));
                    }
                }
                fld(cpu, f32(app, L(0x6e)));
                fsub(cpu, f32(app, L(0x72)));
                fstp(app, cpu, L(0x6e));
            }
        }
    }
    fld(cpu, Float(0.0));
    if (fcomp(cpu, f32(app, cpu.edx + 0x4f4)).above())
        putByte(app, L(0x71), byte(app, L(0x71)) ^ 0x80);
    fld(cpu, f32(app, L(0x6e)));
    fmul(cpu, f32(app, L(2)));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp(app, cpu, L(0x6e));
    if (fcomp(cpu, f32(app, L(0x62))).below())
    {
        cpu.eax = at(app, L(0x62));
        put(app, L(-0xa), cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, L(0x62)));
        fchs(cpu);
        fstp(app, cpu, L(-0xa));
    }
    fld(cpu, f32(app, L(-0xa)));
    fsub(cpu, f32(app, 0x56ebd0));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp64(app, cpu, L(-0x5a));
    if (fcomp(cpu, f64(app, L(-0x5a))).below())
    {
        fld(cpu, f64(app, L(-0x5a)));
        fmul(cpu, f64(app, 0x53636c));
        cpu.esi = at(app, L(-0x1a));
        fstp(app, cpu, cpu.edx + 0x5c0);
        if ((cpu.esi & 0x7fffffff) != 0)
        {
            fld(cpu, f64(app, L(-0x5a)));
            fmul(cpu, f64(app, 0x53634c));
            fstp(app, cpu, cpu.edx + 0x5c4);
        }
        else
            put(app, cpu.edx + 0x5c4, 0);
    }
    else
    {
        put(app, cpu.edx + 0x5c4, 0);
        put(app, cpu.edx + 0x5c0, 0);
    }
    fld(cpu, f32(app, L(0x66)));
    put(app, L(0x7e), cpu.ecx);
    put(app, cpu.edx + 0x4e0, 0);
    fild(app, cpu, L(0x7e));
    fxch(cpu, 1);
    fmul(cpu, cpu.fpu.st(1));
    put(app, cpu.edx + 0x4e4, 0);
    fld(cpu, f32(app, L(0x6e)));
    put(app, cpu.edx + 0x4e8, 0);
    cpu.edi = 0;
    cpu.eax = at(app, L(-0x22));
    put(app, cpu.edx + 0x2f4, cpu.edi);
    fmulp(cpu, 2);
    put(app, cpu.edx + 0x2f8, cpu.eax);
    put(app, kPushY, cpu.edi);
    put(app, cpu.edx + 0x2fc, cpu.edi);
    fstp(app, cpu, kPush);
    fstp(app, cpu, kPushZ);
    steerLeave(app, cpu);
}

/* 0x407c30: the computer's car steered backwards (reversing, or moving
 * backwards): its heading's error by a gain and limit, cut to its grip; the
 * sine of the error's turn by its forward speed, cut to a limit and its
 * grip; the yaw's (+0x2f8) and the across's (+0x4e0) errors by gains and
 * limits; along, its pull or brake by the speed wanted (its size, 16.41 at
 * most) against its forward speed -- reversing, against the speed wanted
 * backwards, negated.  The push and the spin's other parts cleared. */
void steerReversing(App* app, CPU& cpu)
{
    steerEnter(app, cpu, 0xb4);
    const auto L = [&](i32 offset) { return cpu.ebp + reg32(offset); };
    cpu.edx = cpu.eax;
    fld(cpu, f32(app, kHeadingError));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec10));
    stAtMost(app, cpu, L(0x36), 0x56ec14, L(0x6e));
    atLeastNegated(app, cpu, L(0x6e), 0x56ec14, L(6), L(0x26));
    fld(cpu, f32(app, cpu.edx + 0xb8));
    fmul(cpu, f32(app, 0x56ec28));
    cpu.eax = at(app, L(0x26));
    limitSize(app, cpu, L(0x2a), L(0x62), L(0x72));
    atMostSize(app, cpu, L(0x2a), L(0x72), L(-0x16), L(0x52));
    atLeastNegated(app, cpu, L(0x52), L(-0x16), L(0x32), L(0x16));
    cpu.eax = at(app, L(0x16));
    put(app, L(-0xe), cpu.eax);
    cpu.eax = at(app, kHeadingError);
    put(app, L(-0x12), cpu.eax);
    fld(cpu, Float(0.0));
    fld(cpu, f32(app, L(-0x12)));
    fstp64(app, cpu, L(-0x22));
    if (fcomp(cpu, f64(app, L(-0x22))).above())
    {
        fld(cpu, Float(1.0));
        fadd(cpu, f64(app, L(-0x22)));
        fmul(cpu, f64(app, 0x536374));
        cpu.fpu.st(0) = fp::sin(cpu.fpu.st(0));
        fchs(cpu);
    }
    else
    {
        fld(cpu, f64(app, L(-0x22)));
        fmul(cpu, f64(app, 0x536374));
        cpu.fpu.st(0) = fp::sin(cpu.fpu.st(0));
    }
    fmul(cpu, f32(app, cpu.edx + 0x4f4));
    fstp(app, cpu, L(0x5a));
    atMost(app, cpu, L(0x5a), 0x56ec04, L(0x4e));
    atLeastNegated(app, cpu, L(0x4e), 0x56ec04, L(0x12), L(0x7e));
    fld(cpu, f32(app, cpu.edx + 0xb8));
    fmul(cpu, f32(app, 0x56ec24));
    cpu.eax = at(app, L(0x7e));
    limitSize(app, cpu, L(0x6a), L(0x66), L(0x76));
    atMostSize(app, cpu, L(0x6a), L(0x76), L(-0x1a), L(0x5e));
    atLeastNegated(app, cpu, L(0x5e), L(-0x1a), L(0x1a), L(0x1e));
    fld(cpu, f32(app, cpu.edx + 0x4fc));
    fsub(cpu, f32(app, L(-0xe)));
    cpu.eax = at(app, L(0x1e));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec1c));
    put(app, L(-6), cpu.eax);
    stAtMost(app, cpu, L(0x42), 0x56ec18, L(0x4a));
    atLeastNegated(app, cpu, L(0x4a), 0x56ec18, L(0xe), L(0x2e));
    fld(cpu, f32(app, cpu.edx + 0x4ec));
    fsub(cpu, f32(app, L(-6)));
    cpu.eax = at(app, L(0x2e));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec0c));
    put(app, L(-2), cpu.eax);
    stAtMost(app, cpu, L(0x46), 0x56ec08, L(0x56));
    atLeastNegated(app, cpu, L(0x56), 0x56ec08, L(0xa), L(2));
    cpu.eax = at(app, L(2));
    fld(cpu, Float(0.0));
    put(app, L(-0xa), cpu.eax);
    if (fcomp(cpu, f32(app, cpu.edx + 0x7b0)).below())
    {
        cpu.eax = at(app, cpu.edx + 0x7b0);
        put(app, L(0x22), cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x7b0));
        fchs(cpu);
        fstp(app, cpu, L(0x22));
    }
    fld(cpu, f32(app, L(0x22)));
    fst64(app, cpu, L(-0x2a));
    if (fcomp(cpu, f64(app, 0x53637c)).above())
    {
        cpu.ecx = 0xbe0ded29;
        cpu.ebx = 0x40306930;
        put(app, L(-0x32), cpu.ecx);
        put(app, L(-0x2e), cpu.ebx);
    }
    else
        copyDouble(app, cpu, L(-0x2a), L(-0x32));
    fld(cpu, f64(app, L(-0x32)));
    cpu.esi = at(app, cpu.edx + 0x7d4);
    fstp(app, cpu, L(0x3e));
    if (cpu.esi == 0)
    {
        fld(cpu, f32(app, L(0x3e)));
        fsub(cpu, f32(app, cpu.edx + 0x4f4));
        fst(app, cpu, L(0x3a));
        fld(cpu, Float(1.0));
        if (fcompp(cpu).below())
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, kAiPull))
                return;
        }
        else
        {
            cpu.eax = cpu.edx;
            push(app, cpu, at(app, L(0x3a)));
            if (!direct(app, cpu, brakeForce))
                return;
            fchs(cpu);
        }
        fstp(app, cpu, L(0x7a));
    }
    else
    {
        fld(cpu, f32(app, L(0x3e)));
        fchs(cpu);
        if (fcomp(cpu, f32(app, cpu.edx + 0x4f4)).below())
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, kAiPull))
                return;
        }
        else
        {
            cpu.eax = cpu.edx;
            push(app, cpu, 0);
            if (!direct(app, cpu, brakeForce))
                return;
            fchs(cpu);
        }
        fstp(app, cpu, L(0x7a));
        putByte(app, L(0x7d), byte(app, L(0x7d)) ^ 0x80);
    }
    cpu.eax = at(app, L(-0xa));
    put(app, cpu.edx + 0x4e4, 0);
    cpu.edi = 0;
    put(app, cpu.edx + 0x4e0, cpu.eax);
    put(app, cpu.edx + 0x2f4, cpu.edi);
    put(app, kPush, cpu.edi);
    put(app, cpu.edx + 0x2fc, cpu.edi);
    cpu.eax = at(app, L(0x7a));
    put(app, kPushY, cpu.edi);
    put(app, cpu.edx + 0x4e8, cpu.eax);
    cpu.eax = at(app, L(-2));
    put(app, kPushZ, cpu.edi);
    put(app, cpu.edx + 0x2f8, cpu.eax);
    steerLeave(app, cpu);
}

/* 0x407fa0: the computer's car steered off the road (or onto it from the
 * side): the damping off ([kDamping] 1), its heading's error by a gain and
 * limit, by 10 cut to its grip; the yaw's and the across's errors by gains
 * and limits; along, its pull (and where [+0x200] 8 asks at the first 20 of
 * the race with the pull at its table's first, a random push now and then
 * for its player) or its brake; a skid past 7 across -- on a slippery
 * surface (its grip under 1 against the frame's quarter steps) and where
 * 0x404610 lets it, the push 1.1 along (10 and 50 in a race of many). */
void steerOffRoad(App* app, CPU& cpu)
{
    steerEnter(app, cpu, 0xdc);
    const auto L = [&](i32 offset) { return cpu.ebp + reg32(offset); };
    cpu.edx = cpu.eax;
    fld(cpu, f32(app, kHeadingError));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec3c));
    cpu.ecx = 0x3f800000;
    cpu.ebx = 0;
    put(app, L(0x22), cpu.ecx);
    put(app, kDamping, cpu.ecx);
    stAtMost(app, cpu, L(0x16), 0x56ec40, L(0x46));
    atLeastNegated(app, cpu, L(0x46), 0x56ec40, L(-6), L(0x3a));
    fld(cpu, f32(app, L(0x3a)));
    fmul(cpu, f64(app, 0x536384));
    fld(cpu, f32(app, cpu.edx + 0xb8));
    fmul(cpu, f32(app, 0x56ec54));
    cpu.eax = at(app, L(0x3a));
    fld(cpu, Float(0.0));
    put(app, L(0x76), cpu.eax);
    fxch(cpu, 2);
    fstp(app, cpu, L(0x1a));
    fstp(app, cpu, L(0x36));
    if (fcomp(cpu, f32(app, L(0x36))).below())
    {
        cpu.eax = at(app, L(0x36));
        put(app, L(0x5e), cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, L(0x36)));
        fchs(cpu);
        fstp(app, cpu, L(0x5e));
    }
    atMostSize(app, cpu, L(0x76), L(0x5e), L(-0x2e), L(0x26));
    atLeastNegated(app, cpu, L(0x26), L(-0x2e), L(-0x12), L(0x6e));
    cpu.eax = at(app, L(0x6e));
    fld(cpu, f32(app, L(0x1a)));
    put(app, L(-0x26), cpu.eax);
    cpu.eax = fcomp(cpu, f32(app, 0x56ec30)).belowOrEqual() ? at(app, L(0x1a)) : at(app, 0x56ec30);
    put(app, L(0x3e), cpu.eax);
    atLeastNegated(app, cpu, L(0x3e), 0x56ec30, L(-0x16), L(0x2a));
    atMost(app, cpu, L(0x2a), cpu.edx + 0xb8, L(0x4a));
    atLeastNegated(app, cpu, L(0x4a), cpu.edx + 0xb8, L(6), L(-0xa));
    fld(cpu, f32(app, cpu.edx + 0xb8));
    fmul(cpu, f32(app, 0x56ec50));
    cpu.eax = at(app, L(-0xa));
    limitSize(app, cpu, L(0x12), L(0x52), L(0x5a));
    atMostSize(app, cpu, L(0x12), L(0x5a), L(-0x32), L(0x7a));
    atLeastNegated(app, cpu, L(0x7a), L(-0x32), L(-2), L(0x72));
    fld(cpu, f32(app, cpu.edx + 0x4fc));
    fsub(cpu, f32(app, L(-0x26)));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec48));
    cpu.eax = at(app, L(0x72));
    fmul(cpu, f64(app, 0x53638c));
    put(app, L(-0x2a), cpu.eax);
    stAtMost(app, cpu, L(0x56), 0x56ec44, L(0x42));
    atLeastNegated(app, cpu, L(0x42), 0x56ec44, L(-0xe), L(-0x22));
    fld(cpu, f32(app, cpu.edx + 0x4ec));
    fsub(cpu, f32(app, L(-0x2a)));
    cpu.eax = at(app, L(-0x22));
    fchs(cpu);
    fmul(cpu, f32(app, 0x56ec38));
    put(app, L(0x66), cpu.eax);
    stAtMost(app, cpu, L(0x1e), 0x56ec34, L(0x2e));
    atLeastNegated(app, cpu, L(0x2e), 0x56ec34, L(0x32), L(2));
    cpu.eax = at(app, L(2));
    fld(cpu, Float(0.0));
    put(app, L(0x6a), cpu.eax);
    if (fcomp(cpu, f32(app, cpu.edx + 0x7b0)).below())
    {
        cpu.eax = at(app, cpu.edx + 0x7b0);
        put(app, L(-0x1a), cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x7b0));
        fchs(cpu);
        fstp(app, cpu, L(-0x1a));
    }
    fld(cpu, f32(app, L(-0x1a)));
    fsub(cpu, f32(app, cpu.edx + 0x4f4));
    fst(app, cpu, L(0xe));
    fld(cpu, Float(1.0));
    if (fcompp(cpu).below())
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kAiPull))
            return;
        cpu.ah = byte(app, cpu.edx + 0x200);
        fstp(app, cpu, L(0x62));
        if ((cpu.ah & 8) != 0 && i32(at(app, cpu.edx + 8)) < 0x14)
        {
            cpu.eax = at(app, cpu.edx + 0x528);
            fld(cpu, f32(app, L(0x62)));
            if (fcomp(cpu, f32(app, cpu.eax + 0x2ec)).zf)
            {
                // The game's random numbers: a 1 in 7 share of the 16 bits
                // against the car's player.
                cpu.ecx = at(app, kMultiplier);
                cpu.eax = at(app, kSeed);
                cpu.eax *= cpu.ecx;
                put(app, kProduct, cpu.eax);
                cpu.eax &= 0xffff;
                put(app, kSeed, cpu.eax);
                cpu.eax = at(app, kProduct);
                cpu.eax &= 0xffff00;
                cpu.eax >>= 8;
                cpu.eax += cpu.eax;
                cpu.ecx = cpu.eax;
                cpu.eax <<= 3;
                cpu.eax -= cpu.ecx;
                cpu.esi = at(app, cpu.edx + 0x1f0);
                cpu.eax >>= 16;
                if (cpu.eax == cpu.esi)
                {
                    fld(cpu, f32(app, L(0x6a)));
                    fldSt(cpu, 0);
                    fld(cpu, f64(app, 0x53638c));
                    fxch(cpu, 1);
                    fadd(cpu, cpu.fpu.st(1));
                    fstpTo(cpu, 2);
                    fxch(cpu, 1);
                    fst(app, cpu, L(0x6a));
                    fmulp(cpu, 1);
                    fld(cpu, f32(app, L(0x66)));
                    fxch(cpu, 1);
                    fadd(cpu, cpu.fpu.st(1));
                    cpu.ebx = 1;
                    fstpTo(cpu, 1);
                    fstp(app, cpu, L(0x66));
                }
            }
        }
    }
    else
    {
        cpu.eax = cpu.edx;
        push(app, cpu, at(app, L(0xe)));
        if (!direct(app, cpu, brakeForce))
            return;
        fchs(cpu);
        fstp(app, cpu, L(0x62));
    }
    fld(cpu, f32(app, L(0x22)));
    fld(cpu, f32(app, L(0x62)));
    fmul(cpu, cpu.fpu.st(1));
    fld(cpu, f32(app, L(0x6a)));
    fmul(cpu, cpu.fpu.st(2));
    fld(cpu, f64(app, 0x53638c));
    fxch(cpu, 3);
    fstp64(app, cpu, L(-0x4a));
    fxch(cpu, 1);
    fstp(app, cpu, L(0x62));
    fstp(app, cpu, L(0x6a));
    if (fcomp(cpu, f64(app, L(-0x4a))).above())
    {
        cpu.edi = 0;
        cpu.eax = 0x3fe00000;
        put(app, L(-0x52), cpu.edi);
    }
    else
    {
        cpu.eax = at(app, L(-0x4a));
        put(app, L(-0x52), cpu.eax);
        cpu.eax = at(app, L(-0x46));
    }
    put(app, L(-0x4e), cpu.eax);
    fld(cpu, f32(app, L(0x66)));
    fldSt(cpu, 0);
    fmul(cpu, f64(app, L(-0x52)));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstpTo(cpu, 2);
    fxch(cpu, 1);
    fstp(app, cpu, L(0x66));
    if (fcomp(cpu, f32(app, cpu.edx + 0x4ec)).below())
    {
        cpu.eax = at(app, cpu.edx + 0x4ec);
        put(app, L(-0x1e), cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x4ec));
        fchs(cpu);
        fstp(app, cpu, L(-0x1e));
    }
    fld(cpu, f32(app, L(-0x1e)));
    fsub(cpu, f32(app, 0x56ebd0));
    fst(app, cpu, L(0x4e));
    fld(cpu, Float(0.0));
    if (fcompp(cpu).below())
    {
        cpu.eax = at(app, L(0x4e));
        put(app, cpu.edx + 0x5c0, cpu.eax);
        cpu.ecx = at(app, cpu.edx + 0x18c);
        put(app, cpu.edx + 0x5c4, cpu.eax);
        cpu.eax = cpu.ecx * 4;
        cpu.eax -= cpu.ecx;
        cpu.ecx = cpu.eax * 4;
        cpu.eax = at(app, cpu.edx + 0x160);
        cpu.eax = at(app, cpu.ecx + cpu.eax * 4 + kSurfaceGrips);
        put(app, L(0xa), cpu.eax);
        fld(cpu, Float(1.0));
        fld(cpu, f32(app, L(0xa)));
        fstp64(app, cpu, L(-0x3a));
        if (fcomp(cpu, f64(app, L(-0x3a))).above())
        {
            cpu.eax = at(app, kRaceFrame);
            cpu.eax = reg32(i32(cpu.eax) >> 2);
            cpu.eax &= 7;
            put(app, L(0x7e), cpu.eax);
            fild(app, cpu, L(0x7e));
            fmul(cpu, f64(app, 0x536394));
            cpu.ecx = 0;
            if (fcomp(cpu, f64(app, L(-0x3a))).above() && (byte(app, cpu.edx + 0x200) & 8) != 0)
            {
                cpu.eax = cpu.edx;
                if (!byAddress(app, cpu, 0x404610))
                    return;
                bool slide = true;
                if (cpu.eax == 1)
                {
                    if ((byte(app, kRaceFrame) & 0x20) != 0)
                        cpu.ecx = 1;
                    slide = cpu.ecx != 0;
                }
                else if (cpu.eax != 0)
                {
                    if ((byte(app, kRaceFrame) & 0x60) == 0)
                        cpu.ecx = 1;
                    slide = cpu.ecx != 0;
                }
                if (slide)
                {
                    if (at(app, kSingleRace) == 1)
                    {
                        fld(cpu, f32(app, L(0x6a)));
                        fldSt(cpu, 0);
                        fld(cpu, f64(app, 0x53639c));
                        fxch(cpu, 1);
                        fmul(cpu, cpu.fpu.st(1));
                        fld(cpu, f32(app, L(0x66)));
                        cpu.fpu.st(2) = fp::mul(cpu.fpu.st(2), cpu.fpu.st(0));
                        fxch(cpu, 1);
                    }
                    else
                    {
                        fld(cpu, f32(app, L(0x6a)));
                        fldSt(cpu, 0);
                        fmul(cpu, f64(app, 0x536384));
                        fld(cpu, f32(app, L(0x66)));
                        fldSt(cpu, 0);
                        fmul(cpu, f64(app, 0x5363a4));
                        fxch(cpu, 2);
                    }
                    fstpTo(cpu, 3);
                    fxch(cpu, 1);
                    fstpTo(cpu, 1);
                    fxch(cpu, 1);
                    fstp(app, cpu, L(0x6a));
                    fstp(app, cpu, L(0x66));
                }
            }
        }
    }
    else
    {
        put(app, cpu.edx + 0x5c4, 0);
        put(app, cpu.edx + 0x5c0, 0);
    }
    if (cpu.ebx != 0)
        put(app, cpu.edx + 0x5c4, 0x41200000);
    cpu.eax = at(app, L(0x6a));
    put(app, cpu.edx + 0x4e4, 0);
    cpu.ebx = 0;
    put(app, cpu.edx + 0x4e0, cpu.eax);
    put(app, cpu.edx + 0x2f4, cpu.ebx);
    put(app, kPush, cpu.ebx);
    put(app, cpu.edx + 0x2fc, cpu.ebx);
    cpu.eax = at(app, L(0x62));
    put(app, kPushY, cpu.ebx);
    put(app, cpu.edx + 0x4e8, cpu.eax);
    cpu.eax = at(app, L(0x66));
    put(app, kPushZ, cpu.ebx);
    put(app, cpu.edx + 0x2f8, cpu.eax);
    steerLeave(app, cpu);
}

/* 0x408520: car eax rides with the car it follows ([+0x624]): its place,
 * axes, spin, speed and +0x204/+0x208 copied, its track's place cleared
 * (0x4e070c, 0x78 bytes) and found anew at the other's slice (0x41e940),
 * its +8 and its brake the other's. */
void rideWithTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.edi = cpu.ebx + 0x98;
        cpu.esi = cpu.eax + 0x98;
        movsd3(app, cpu);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.ecx = 9;
        cpu.edi = cpu.ebx + 0xc0;
        cpu.esi = cpu.eax + 0xc0;
        repMovsd(app, cpu);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.edi = cpu.ebx + 0xe4;
        cpu.esi = cpu.eax + 0xe4;
        movsd3(app, cpu);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.edi = cpu.ebx + 0xa4;
        cpu.esi = cpu.eax + 0xa4;
        movsd3(app, cpu);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.eax = at(app, cpu.eax + 0x204);
        put(app, cpu.ebx + 0x204, cpu.eax);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.edx = 0x78;
        cpu.eax = at(app, cpu.eax + 0x208);
        cpu.ecx = cpu.ebx + 0xc;
        put(app, cpu.ebx + 0x208, cpu.eax);
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFillZero))
            return;
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.eax = at(app, cpu.eax + 0x1c);
        put(app, cpu.ebx + 0x1c, cpu.eax);
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.eax = at(app, cpu.eax + 8);
        put(app, cpu.ebx + 8, cpu.eax);
        cpu.edx = cpu.ecx;
        cpu.eax = at(app, cpu.ebx + 0x1c);
        if (!byAddress(app, cpu, kResetPosition))
            return;
        cpu.eax = at(app, cpu.ebx + 0x624);
        cpu.al = byte(app, cpu.eax + 0x509);
        putByte(app, cpu.ebx + 0x509, cpu.al);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x408600: the computer's car's physics while racing: tipped over ([+0xd0]
 * under 0.3) settled at rest (0x4aa8e7); finished or busted, the coast and
 * its revs; else its lane eased (by 0.2), its revs, its reverse's end, its
 * heading's error (0x4070c0), no speed wanted for traffic before its time
 * ([+0x65c]); then steered backwards (reversing, or going backwards), off
 * the road (past the road's edges, the heading's error past 0.04 of the
 * surface's grip, faster than 0.5, after its time, and sliding where its grip
 * is past 2.5), backwards again under 13.41 where [+0x201] 2 or on the road,
 * on the road else -- and the step integrated. */
void aiPhysics(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto mine = [&](void (*function)(App*, CPU&)) {
        cpu.eax = cpu.edx;
        return direct(app, cpu, function);
    };
    cpu.edx = cpu.eax;
    if (half(app, cpu.eax + 0x164) != 0)
        return leave();
    if (i32(at(app, cpu.eax + 0xd0)) < 0x3e99999a)
    {
        if (!byAddress(app, cpu, kSettleAtRest))
            return;
        return leave();
    }
    const bool done = ((byte(app, cpu.eax + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 3)
                      || ((byte(app, cpu.edx + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 4);
    if (done)
    {
        if (!mine(coastAfterFinish) || !mine(engineRevs))
            return;
        return leave();
    }
    cpu.eax = cpu.edx;
    push(app, cpu, 0x3e4ccccd);
    if (!direct(app, cpu, easeLane) || !mine(engineRevs) || !mine(reverseCheck))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kAiHeading))
        return;
    cpu.edi = at(app, kRaceFrame);
    if (i32(cpu.edi) < i32(at(app, cpu.edx + 0x65c)) && (byte(app, cpu.edx + 0x200) & 0x10) != 0)
        put(app, cpu.edx + 0x7b0, 0);
    cpu.ecx = at(app, cpu.edx + 0x18c);
    cpu.eax = cpu.ecx * 4;
    cpu.eax -= cpu.ecx;
    cpu.ecx = cpu.eax * 4;
    cpu.eax = at(app, cpu.edx + 0x160);
    cpu.eax = at(app, cpu.ecx + cpu.eax * 4 + kSurfaceGrips);
    put(app, cpu.ebp - 0x10, cpu.eax);
    bool backwards = at(app, cpu.edx + 0x7d4) == 1;
    if (!backwards)
    {
        fld(cpu, f32(app, cpu.edx + 0x4f4));
        fld(cpu, Float(0.0));
        backwards = fcompp(cpu).above();
    }
    if (backwards)
    {
        if (!mine(steerReversing))
            return;
    }
    else
    {
        // 0: off the road; 1: backwards under 13.41, else on the road.
        int mode = 1;
        if ((byte(app, cpu.edx + 0x201) & 2) == 0)
        {
            cpu.ecx = at(app, cpu.edx + 0x1c);
            cpu.eax = cpu.ecx * 8;
            cpu.eax += cpu.ecx;
            cpu.ecx = at(app, kSlices);
            cpu.eax <<= 2;
            cpu.ecx += cpu.eax;
            cpu.eax = 0;
            cpu.al = byte(app, cpu.ecx + 0x13);
            cpu.ebx = cpu.eax;
            cpu.eax = 0;
            cpu.ebx = reg32(i32(cpu.ebx) >> 4);
            cpu.al = byte(app, cpu.ecx + 0x17);
            cpu.ebx &= 0xf;
            cpu.eax <<= 15;
            cpu.eax = reg32(std::int64_t(i32(cpu.eax)) * std::int64_t(i32(cpu.ebx)));
            cpu.eax = reg32(0) - cpu.eax;
            put(app, cpu.ebp - 4, cpu.eax);
            fild(app, cpu, cpu.ebp - 4);
            mode = 0;
            if (!fcomp(cpu, f32(app, cpu.edx + 0x7b8)).above())
            {
                cpu.eax = 0;
                cpu.al = byte(app, cpu.ecx + 0x1b);
                cpu.cl = byte(app, cpu.ecx + 0x13);
                cpu.eax <<= 15;
                cpu.cl &= 0xf;
                cpu.ecx &= 0xff;
                cpu.eax = reg32(std::int64_t(i32(cpu.eax)) * std::int64_t(i32(cpu.ecx)));
                put(app, cpu.ebp - 4, cpu.eax);
                fild(app, cpu, cpu.ebp - 4);
                if (!fcomp(cpu, f32(app, cpu.edx + 0x7b8)).below())
                {
                    magnitude(app, cpu, kHeadingError, cpu.ebp - 8);
                    fld(cpu, f32(app, cpu.ebp - 0x10));
                    fmul(cpu, f32(app, 0x5363ac));
                    if (!fcomp(cpu, f32(app, cpu.ebp - 8)).below())
                    {
                        fld(cpu, f32(app, cpu.edx + 0x7c4));
                        if (!fcomp(cpu, f64(app, 0x5363b4)).below())
                        {
                            cpu.ecx = at(app, kRaceFrame);
                            if (i32(cpu.ecx) >= i32(at(app, cpu.edx + 0x65c)))
                            {
                                mode = 1;
                                fld(cpu, f32(app, cpu.edx + 0xb8));
                                if (fcomp(cpu, f64(app, 0x5363bc)).above())
                                {
                                    magnitude(app, cpu, cpu.edx + 0x4ec, cpu.ebp - 0xc);
                                    fld(cpu, f32(app, cpu.edx + 0x4f4));
                                    fmul(cpu, f32(app, 0x5363c4));
                                    if (fcomp(cpu, f32(app, cpu.ebp - 0xc)).below())
                                        mode = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (mode == 0)
        {
            if (!mine(steerOffRoad) || !mine(integrate))
                return;
            return leave();
        }
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        if (fcomp(cpu, f64(app, 0x5363cc)).below())
        {
            if (!mine(steerReversing) || !mine(integrate))
                return;
            return leave();
        }
        if (!mine(steerOnRoad))
            return;
    }
    if (!mine(integrate))
        return;
    return leave();
}

/* 0x408870 (eax the car, a float on the stack, ret 4): the lane's offset it
 * eases to ([+0x7c8]) a step of the float toward the one it wants
 * ([+0x7b4]), no further. */
void easeLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    fld(cpu, f32(app, cpu.edx + 0x7c8));
    if (!fcomp(cpu, f32(app, cpu.eax + 0x7b4)).zf)
    {
        fld(cpu, f32(app, cpu.edx + 0x7c8));
        if (fcomp(cpu, f32(app, cpu.edx + 0x7b4)).below())
        {
            fld(cpu, f32(app, cpu.edx + 0x7c8));
            fadd(cpu, f32(app, cpu.ebp + 0xc));
            fst(app, cpu, cpu.edx + 0x7c8);
            if (fcomp(cpu, f32(app, cpu.edx + 0x7b4)).above())
            {
                cpu.eax = at(app, cpu.edx + 0x7b4);
                put(app, cpu.edx + 0x7c8, cpu.eax);
            }
        }
        else
        {
            fld(cpu, f32(app, cpu.edx + 0x7c8));
            fsub(cpu, f32(app, cpu.ebp + 0xc));
            fst(app, cpu, cpu.edx + 0x7c8);
            if (fcomp(cpu, f32(app, cpu.edx + 0x7b4)).below())
            {
                cpu.eax = at(app, cpu.edx + 0x7b4);
                put(app, cpu.edx + 0x7c8, cpu.eax);
            }
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x4088f0: the computer's car's step, each frame: nothing while 0x4be1a0
 * (0x20, its first word) says so or it is off the road ([+0x620] 4); the
 * step's length 1/32, one substep; held ([+0x8a]) or tipped (upright under
 * 0.9): its slip and skid cleared, its revs down 500 toward the idle and
 * settled at rest (0x4aa8e7); before the cars move, its speed and spin 0;
 * racing, its speed and spin in its own axes (dot products with its axes),
 * then it glides as a ghost ([+0x88]), rides with the car it follows
 * ([+0x620] 0x80) or drives (aiPhysics). */
void aiStep(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax);
    cpu.eax = 0x20;
    if (!byAddress(app, cpu, 0x4be1a0))
        return;
    if (cpu.eax != 0)
        return leave();
    cpu.ebx = 0x3d000000;
    cpu.edx = 1;
    cpu.ah = byte(app, cpu.ecx + 0x620);
    put(app, kStep, cpu.ebx);
    put(app, kSubsteps, cpu.edx);
    if ((cpu.ah & 4) != 0)
        return leave();
    bool held = byte(app, cpu.ecx + 0x8a) != 0;
    if (!held)
    {
        fld(cpu, f32(app, cpu.ecx + 0x10c));
        held = fcomp(cpu, f64(app, 0x5363d4)).below();
    }
    if (held)
    {
        put(app, cpu.ecx + 0x5b0, 0);
        put(app, cpu.ecx + 0x5c0, 0);
        put(app, cpu.ecx + 0x5c4, 0);
        cpu.eax = at(app, cpu.ecx + 0x528);
        put(app, cpu.ecx + 0x594, 0);
        cpu.edx = at(app, cpu.eax + 0x1c0);
        cpu.esi = at(app, cpu.ecx + 0x590);
        cpu.edx += 0x1f4;
        if (i32(cpu.edx) < i32(cpu.esi))
        {
            cpu.edi = cpu.esi - 0x1f4;
            put(app, cpu.ecx + 0x590, cpu.edi);
        }
        else
        {
            cpu.eax = at(app, cpu.eax + 0x1c0);
            put(app, cpu.ecx + 0x590, cpu.eax);
        }
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kSettleAtRest))
            return;
        return leave();
    }
    cpu.eax = at(app, kCarsMove);
    put(app, cpu.ecx + 0x2e4, 0);
    if (cpu.eax == 0)
    {
        put(app, cpu.ecx + 0xa8, cpu.eax);
        put(app, cpu.ecx + 0xac, cpu.eax);
        put(app, cpu.ecx + 0xe4, cpu.eax);
        put(app, cpu.ecx + 0xe8, cpu.eax);
        put(app, cpu.ecx + 0xec, cpu.eax);
        put(app, cpu.ecx + 0xa4, cpu.eax);
        return leave();
    }
    if (half(app, cpu.ecx + 0x164) != 0)
        return leave();
    cpu.esi = cpu.ecx + 0xc0;
    cpu.ebx = cpu.ecx + 0xa4;
    cpu.edx = cpu.esi;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.eax = cpu.ecx;
    cpu.eax += 0xcc;
    fstp(app, cpu, cpu.ecx + 0x4ec);
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebx;
    cpu.edi = cpu.ecx + 0xd8;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.edi;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x4f0);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.ebx = cpu.ecx + 0xe4;
    cpu.edx = cpu.esi;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x4f4);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x4f8);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.edi;
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0x4fc);
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.dh = byte(app, cpu.ecx + 0x88);
    fstp(app, cpu, cpu.ecx + 0x500);
    cpu.eax = cpu.ecx;
    if (cpu.dh != 0)
    {
        if (!direct(app, cpu, glideAlongRoad))
            return;
    }
    else if ((byte(app, cpu.ecx + 0x620) & 0x80) != 0)
    {
        if (!direct(app, cpu, rideWithTarget))
            return;
    }
    else if (!direct(app, cpu, aiPhysics))
        return;
    return leave();
}

}  // namespace aidrive
}  // namespace nfs3hp
