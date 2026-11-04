#include "native_guest.h"

#include <cstdint>
#include <tuple>
#include <utility>

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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack in cpu.fpu as the
 * code pushes and pops it (rounded as its control word says: x86::FPU's add,
 * sub, mul, div), and a compare's status word left in ax as fnstsw leaves it
 * (below and equal the flags sahf makes of it).  Every other function of the
 * game's is called by address; the game's constants (floats and doubles in
 * its data, 0x53dc90 to 0x53dd48) are read where the code reads them, and
 * the revs' drop by gear the code copies from among its own code (0x4a78b6)
 * read there too. */
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

// The race's.
constexpr reg32 kPlayMode = 0x6fd3b0;       // 0 one player, 1 split screen, 2 modem, 3 serial, 4 network
constexpr reg32 kRaceType = 0x6fd3b8;       // 0 a single race, 1 and 2 the series, 3 Hot Pursuit
constexpr reg32 kBackwards = 0x6fd4c0;      // the track run the other way: slices counted down
constexpr reg32 kLocalPlayer = 0x6fd4f4;    // the player at this machine
constexpr reg32 kLoadingFlags = 0x6fd4fc;   // 1: the cars' grip as a simulation's
constexpr reg32 kSimulation = 0x6fd4a4;     // the handling as a simulation's
constexpr reg32 kComputerDrives = 0x6fd490, kComputerDrivesToo = 0x6fd494;  // the player's car driven for him
// The players' records, 0x6c bytes each: 1 at +0 an automatic box.
constexpr reg32 kPlayerGearbox = 0x6fd548;
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
// The game's functions called.
constexpr reg32 kChop = 0x4dfd56;           // __CHP: st(0) to an integer, towards 0
constexpr reg32 kFloor = 0x4f1c30;          // floor of the double on the stack
constexpr reg32 kVectorLength = 0x4e06a0;   // the length of the vector at eax
constexpr reg32 kScaleVectors = 0x4e0050;   // eax vectors at edx scaled by [esp+4] to [esp+8]
constexpr reg32 kSliceHeading = 0x49fee0;   // the track's heading at slice eax
constexpr reg32 kSliceSpeed = 0x40e080;     // the computer's speed at slice eax, onto the x87
constexpr reg32 kCarKnock = 0x416d90;       // snd::carKnock: the gear's clunk
constexpr reg32 kGearChangeKick = 0x4714f0; // controls::gearChangeKick: the force feedback's kick
constexpr reg32 kWheelSurface = 0x429b40;   // racestart::wheelSurface: what wheel edx is on

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putHalf(App* app, reg32 a, reg16 v) { app->getMemory<reg16>(a) = v; }
x86::Float f32(App* app, reg32 a) { return x86::Float(app->getMemory<float>(a)); }
x86::Float f64(App* app, reg32 a) { return x86::Float(app->getMemory<double>(a)); }
x86::Float i32At(App* app, reg32 a) { return x86::Float(x86::sreg32(at(app, a))); }
x86::Float i16At(App* app, reg32 a) { return x86::Float(x86::sreg16(half(app, a))); }

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

/* fstp dword m, fstp qword m. */
void storeFloat(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<float>(a) = float(cpu.fpu.st(0));
    cpu.fpu.pop();
}

void storeDouble(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<double>(a) = double(cpu.fpu.st(0));
    cpu.fpu.pop();
}

/* fistp dword m, as the control word rounds. */
void storeInteger(App* app, CPU& cpu, reg32 a)
{
    put(app, a, reg32(fp::toInteger<i32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
}

/* sub esp, 4; fistp dword [esp]; pop eax. */
void integerToEax(App* app, CPU& cpu)
{
    cpu.esp -= 4;
    storeInteger(app, cpu, cpu.esp);
    cpu.eax = pop(app, cpu);
}

/* fcomp m: st(0) compared with m, popped; fcompp: with st(1), both popped.
 * The code's fnstsw ax follows each: the status word in ax.  below and
 * equal are the carry (C0) and zero (C3) flags sahf makes of ah. */
void fcomp(CPU& cpu, const x86::Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.fpu.pop();
    cpu.ax = cpu.fpu.status.word;
}

void fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.fpu.pop();
    cpu.fpu.pop();
    cpu.ax = cpu.fpu.status.word;
}

/* fldz; fcomp m; fnstsw ax. */
void zeroAgainst(CPU& cpu, const x86::Float& m)
{
    cpu.fpu.push(0.0);
    fcomp(cpu, m);
}

bool below(const CPU& cpu) { return (cpu.ah & 0x01) != 0; }
bool equal(const CPU& cpu) { return (cpu.ah & 0x40) != 0; }
bool above(const CPU& cpu) { return !below(cpu) && !equal(cpu); }

/* idiv: edx:eax divided by `by`, the quotient to eax and the remainder to edx. */
void idiv(CPU& cpu, reg32 by)
{
    const std::int64_t n = std::int64_t((std::uint64_t(cpu.edx) << 32) | cpu.eax);
    const std::int64_t d = i32(by);
    cpu.eax = reg32(n / d);
    cpu.edx = reg32(n % d);
}

/* sar edx, 0x1f; shl edx, n; sbb eax, edx; sar eax, n -- eax (edx holding
 * the same) divided by 2^n towards 0, as Watcom divides. */
void divideByShift(CPU& cpu, unsigned n)
{
    cpu.edx = reg32(i32(cpu.edx) >> 31);
    const reg32 carry = (cpu.edx >> (32 - n)) & 1;
    cpu.edx <<= n;
    cpu.eax = cpu.eax - cpu.edx - carry;
    cpu.eax = reg32(i32(cpu.eax) >> n);
}

/* The registers' pushes the module's functions open with -- ebx, ecx, edx,
 * esi, edi, ebp -- the frame, and their way out. */
void enter(App* app, CPU& cpu, reg32 frame)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= frame;
}

void leave(App* app, CPU& cpu)
{
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* The gear's clunk, as the code calls it: [ebp - frame] (the pan: split
 * screen's sides) and 0 pushed, ecx and ebx 0, kind 0xd, slot -1. */
bool gearKnock(App* app, CPU& cpu, reg32 frame)
{
    cpu.eax = at(app, cpu.ebp - frame);
    push(app, cpu, cpu.eax);
    push(app, cpu, 0);
    cpu.ecx = 0;
    cpu.ebx = 0;
    cpu.edx = 0xd;
    cpu.eax = 0xffffffff;
    return byAddress(app, cpu, kCarKnock);
}

}  // namespace

void engineTorque(App* app, CPU& cpu);
void automaticGears(App* app, CPU& cpu);
void gearLimit(App* app, CPU& cpu);
void computerDriver(App* app, CPU& cpu);
void autoReverse(App* app, CPU& cpu);
void driverInputs(App* app, CPU& cpu);
void wheelGrip(App* app, CPU& cpu);
void engineForce(App* app, CPU& cpu);

/* 0x4a83d3: the inputs of car eax when the computer drives it (another's, or
 * the player's when the game drives for him).  Its steering
 * ([+0x218], the asked +0x504 - +0x505 + 0x100, by 0x20 a frame at most) and
 * the speed it aims at: the computer's speed of the slice 7 ahead (back,
 * when the race runs backwards; 0x40e080) by the bend's scale -- the bend
 * [kComputerBend] the track's heading 10 slices on or back (0x49fee0) less
 * the car's, eased with +0x214 seven to one, against the handling's +0x2a4,
 * +0x2a8, +0x2ac.  The pedal [+0x520] (0x100 the middle) pushed towards the
 * speed, held at the revs the speed gives in between; to the throttle
 * (+0x508) over 0xff, else to the brake (+0x509). */
void computerDriver(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x7c);
    put(app, cpu.ebp - 0x2c, cpu.eax);
    put(app, cpu.ebp - 0x24, 0);
    put(app, cpu.ebp - 0x20, 0);
    put(app, cpu.ebp - 0x1c, 7);
    put(app, cpu.ebp - 0x18, 0);
    put(app, cpu.ebp - 0x14, 0x3f800000);
    put(app, cpu.ebp - 0x10, 0x42200000);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    if (at(app, kBackwards) != 0)
        put(app, cpu.ebp - 0x1c, 0xfffffff9);
    // The steering asked.
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.edx = byte(app, cpu.eax + 0x504);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.eax = byte(app, cpu.eax + 0x505);
    cpu.edx += 0x100;
    cpu.edx -= cpu.eax;
    put(app, cpu.ebp - 0x24, cpu.edx);
    // How far the car's heading is from the computer's line, both ways round.
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.eax = at(app, cpu.eax + 0x154);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    cpu.eax -= at(app, cpu.edx + 0x210);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.eax = at(app, cpu.eax + 0x154);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    cpu.eax -= at(app, cpu.edx + 0x210);
    put(app, cpu.ebp - 0x34, cpu.eax);
    if (i32(cpu.eax) < 0)
        put(app, cpu.ebp - 0x34, reg32(-i32(cpu.eax)));
    cpu.eax = at(app, cpu.ebp - 0x34);
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = 0x400;
    cpu.eax -= at(app, cpu.ebp - 0xc);
    if (i32(cpu.eax) < i32(at(app, cpu.ebp - 0xc)))
    {
        cpu.eax = 0x400;
        cpu.eax -= at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 0x30, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 0x30, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x30);
    put(app, cpu.ebp - 0xc, cpu.eax);
    // The track's heading 10 slices back when it faces the wrong way, else
    // 10 on, wrapped round.
    if (i32(at(app, cpu.ebp - 0xc)) > 0x100)
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax -= 0xa;
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax -= 0xa;
        if (i32(cpu.eax) < 0)
        {
            cpu.edx = at(app, kSliceCount);
            cpu.edx += cpu.eax;
            put(app, cpu.ebp - 0x38, cpu.edx);
        }
        else
        {
            put(app, cpu.ebp - 0x38, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x38);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += 0xa;
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += 0xa;
        if (i32(cpu.eax) >= i32(at(app, kSliceCount)))
            cpu.eax -= at(app, kSliceCount);
        put(app, cpu.ebp - 0x3c, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x3c);
    }
    if (!byAddress(app, cpu, kSliceHeading))
        return;
    put(app, cpu.ebp - 8, cpu.eax);
    // The bend, half a turn either way, eased.
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.eax = at(app, cpu.eax + 0x154);
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.edx -= cpu.eax;
    put(app, kComputerBend, cpu.edx);
    if (i32(at(app, kComputerBend)) > 0x200)
        put(app, kComputerBend, at(app, kComputerBend) - 0x400);
    else if (i32(at(app, kComputerBend)) < i32(0xfffffe00))
        put(app, kComputerBend, at(app, kComputerBend) + 0x400);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    cpu.edx = reg32(i32(at(app, cpu.edx + 0x214)) * 7);
    cpu.edx += at(app, kComputerBend);
    cpu.eax = cpu.edx;
    divideByShift(cpu, 3);
    put(app, kComputerBend, cpu.eax);
    cpu.eax = at(app, kComputerBend);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    put(app, cpu.edx + 0x214, cpu.eax);
    // The bend's scale of the speed.
    cpu.eax = at(app, kComputerBend);
    put(app, cpu.ebp - 0x40, cpu.eax);
    if (i32(cpu.eax) < 0)
        put(app, cpu.ebp - 0x40, reg32(-i32(cpu.eax)));
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.edx = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.ebp - 0x40);
    if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x2a4)))
    {
        cpu.eax = at(app, kComputerBend);
        put(app, cpu.ebp - 0x44, cpu.eax);
        if (i32(cpu.eax) < 0)
            put(app, cpu.ebp - 0x44, reg32(-i32(cpu.eax)));
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x44);
        if (i32(cpu.eax) < i32(at(app, cpu.edx + 0x2a8)))
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 0x2b0);
            put(app, cpu.ebp - 0x14, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, kComputerBend);
            put(app, cpu.ebp - 0x48, cpu.eax);
            if (i32(cpu.eax) < 0)
                put(app, cpu.ebp - 0x48, reg32(-i32(cpu.eax)));
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x48);
            if (i32(cpu.eax) < i32(at(app, cpu.edx + 0x2ac)))
            {
                cpu.eax = at(app, cpu.ebp - 0x2c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.eax = at(app, cpu.eax + 0x2b4);
                put(app, cpu.ebp - 0x14, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x2c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.eax = at(app, cpu.eax + 0x2b8);
                put(app, cpu.ebp - 0x14, cpu.eax);
            }
        }
    }
    // The slice 7 ahead (or back), wrapped round: its speed.
    if (i32(at(app, cpu.ebp - 0x1c)) >= 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += at(app, cpu.ebp - 0x1c);
        if (i32(cpu.eax) >= i32(at(app, kSliceCount)))
        {
            cpu.eax -= at(app, kSliceCount);
            put(app, cpu.ebp - 0x50, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0x50, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x50);
        put(app, cpu.ebp - 0x54, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        cpu.eax += at(app, cpu.ebp - 0x1c);
        if (i32(cpu.eax) < 0)
        {
            cpu.edx = at(app, kSliceCount);
            cpu.edx += cpu.eax;
            put(app, cpu.ebp - 0x4c, cpu.edx);
        }
        else
        {
            put(app, cpu.ebp - 0x4c, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x4c);
        put(app, cpu.ebp - 0x54, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x54);
    if (!byAddress(app, cpu, kSliceSpeed))
        return;
    storeFloat(app, cpu, cpu.ebp - 0x10);
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x14));
    storeFloat(app, cpu, cpu.ebp - 0x10);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x4f4));
    storeFloat(app, cpu, cpu.ebp - 0x18);
    // The steering eased: 0x20 a frame at most.
    cpu.eax = at(app, cpu.ebp - 0x24);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x218)))
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x218);
        cpu.eax += 0x20;
        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x24)))
        {
            cpu.eax = at(app, cpu.ebp - 0x24);
            put(app, cpu.ebp - 0x58, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = at(app, cpu.eax + 0x218);
            cpu.eax += 0x20;
            put(app, cpu.ebp - 0x58, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x58);
        put(app, cpu.ebp - 0x24, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        cpu.edx = at(app, cpu.ebp - 0x2c);
        if (i32(cpu.eax) < i32(at(app, cpu.edx + 0x218)))
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = at(app, cpu.eax + 0x218);
            cpu.eax -= 0x20;
            if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x24)))
            {
                cpu.eax = at(app, cpu.ebp - 0x2c);
                cpu.eax = at(app, cpu.eax + 0x218);
                cpu.eax -= 0x20;
                put(app, cpu.ebp - 0x5c, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x24);
                put(app, cpu.ebp - 0x5c, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x5c);
            put(app, cpu.ebp - 0x24, cpu.eax);
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    put(app, cpu.edx + 0x218, cpu.eax);
    // The pedal back to the middle when it is the wrong way round.
    zeroAgainst(cpu, f32(app, cpu.ebp - 0x18));
    bool middle = false;
    if (above(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        if (i32(at(app, cpu.eax + 0x520)) > 0x100)
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            put(app, cpu.eax + 0x520, 0x100);
            middle = true;
        }
    }
    if (!middle && i32(at(app, cpu.ebp - 0x18)) > i32(0x3f800000))
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        if (i32(at(app, cpu.eax + 0x520)) < 0x100)
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            put(app, cpu.eax + 0x520, 0x100);
        }
    }
    // Its step: down on it under the speed, off it over, else held where
    // the revs the speed gives in the gear are.
    if (i32(at(app, cpu.ebp - 0x18)) > i32(0x3f800000))
    {
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc90));
        integerToEax(app, cpu);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax += 4;
        if (i32(cpu.eax) > 0x10)
        {
            put(app, cpu.ebp - 0x60, 0x10);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.eax += 4;
            put(app, cpu.ebp - 0x60, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x60);
        put(app, cpu.ebp - 0x20, cpu.eax);
    }
    else
    {
        zeroAgainst(cpu, f32(app, cpu.ebp - 0x18));
        if (above(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x18));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc94));
            integerToEax(app, cpu);
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.eax -= 8;
            if (i32(cpu.eax) > -0x18)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                cpu.eax -= 8;
                put(app, cpu.ebp - 0x64, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - 0x64, 0xffffffe8);
            }
            cpu.eax = at(app, cpu.ebp - 0x64);
            put(app, cpu.ebp - 0x20, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0x6c, 0);
            put(app, cpu.ebp - 0x68, 0);
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.ecx = byte(app, cpu.eax + 0x50b);
            cpu.ecx <<= 2;
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.edx += cpu.ecx;
            cpu.eax = at(app, cpu.ebp - 0x2c);
            fpu.push(f32(app, cpu.eax + 0x4f4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
            integerToEax(app, cpu);
            put(app, cpu.ebp - 0x68, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = at(app, cpu.eax + 0x590);
            if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x1c0)))
            {
                cpu.eax = at(app, cpu.ebp - 0x2c);
                cpu.ecx = at(app, cpu.eax + 0x528);
                cpu.edx = at(app, cpu.ebp - 0x2c);
                cpu.eax = at(app, cpu.edx + 0x528);
                cpu.edx = at(app, cpu.ecx + 0x1c4);
                cpu.edx -= at(app, cpu.eax + 0x1c0);
                cpu.ecx = cpu.edx;
                cpu.edx = at(app, cpu.ebp - 0x2c);
                cpu.edx = at(app, cpu.edx + 0x528);
                cpu.eax = at(app, cpu.ebp - 0x68);
                cpu.eax -= at(app, cpu.edx + 0x1c0);
                cpu.edx = cpu.eax;
                cpu.edx = reg32(i32(cpu.edx) >> 31);
                idiv(cpu, cpu.ecx);
                put(app, cpu.ebp - 0x70, cpu.eax);
                fpu.push(i32At(app, cpu.ebp - 0x70));
                storeFloat(app, cpu, cpu.ebp - 0x6c);
            }
            else
            {
                put(app, cpu.ebp - 0x6c, 0);
            }
            fpu.push(f32(app, cpu.ebp - 0x6c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc98));
            integerToEax(app, cpu);
            cpu.edx = cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.edx += 0x100;
            put(app, cpu.eax + 0x520, cpu.edx);
        }
    }
    // The step taken (none held), as far as the steering asked lets it, 0 at least.
    cpu.eax = at(app, cpu.ebp - 0x2c);
    cpu.eax = at(app, cpu.eax + 0x520);
    cpu.eax += at(app, cpu.ebp - 0x20);
    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x24)))
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        put(app, cpu.ebp - 0x78, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x520);
        cpu.eax += at(app, cpu.ebp - 0x20);
        put(app, cpu.ebp - 0x78, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x78);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    put(app, cpu.edx + 0x520, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    if (i32(at(app, cpu.eax + 0x520)) > 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x520);
        put(app, cpu.ebp - 0x74, cpu.eax);
    }
    else
    {
        put(app, cpu.ebp - 0x74, 0);
    }
    cpu.eax = at(app, cpu.ebp - 0x74);
    cpu.edx = at(app, cpu.ebp - 0x2c);
    put(app, cpu.edx + 0x520, cpu.eax);
    // To the throttle over the middle, else to the brake.
    cpu.eax = at(app, cpu.ebp - 0x2c);
    if (i32(at(app, cpu.eax + 0x520)) >= 0xff)
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.eax = at(app, cpu.eax + 0x520);
        cpu.eax -= 0x100;
        if (i32(cpu.eax) < 0)
        {
            put(app, cpu.ebp - 0x7c, 0);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = at(app, cpu.eax + 0x520);
            cpu.eax -= 0x100;
            put(app, cpu.ebp - 0x7c, cpu.eax);
        }
        cpu.al = byte(app, cpu.ebp - 0x7c);
        cpu.edx = at(app, cpu.ebp - 0x2c);
        putByte(app, cpu.edx + 0x508, cpu.al);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        putByte(app, cpu.eax + 0x509, 0);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        putByte(app, cpu.eax + 0x508, 0);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.dl = byte(app, cpu.eax + 0x520);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        cpu.dh = 0xff;
        cpu.dh -= cpu.dl;
        putByte(app, cpu.eax + 0x509, cpu.dh);
    }
    leave(app, cpu);
}

/* 0x4a8a99: the engine's torque at revs edx for car eax, onto the x87's
 * stack: its handling's curve (+0x114, a float each 256 revs, 40 at most)
 * between the two points either side, or the point itself on one. */
void engineTorque(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x34;
    put(app, cpu.ebp - 0x34, cpu.eax);
    put(app, cpu.ebp - 0x30, cpu.edx);
    put(app, cpu.ebp - 0x10, 0);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    // The revs by 256, 0 to 40.
    fpu.push(i32At(app, cpu.ebp - 0x30));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc9c));
    storeFloat(app, cpu, cpu.ebp - 8);
    if (i32(at(app, cpu.ebp - 8)) > i32(0x42200000))
    {
        put(app, cpu.ebp - 8, 0x42200000);
    }
    else
    {
        zeroAgainst(cpu, f32(app, cpu.ebp - 8));
        if (above(cpu))
            put(app, cpu.ebp - 8, 0);
    }
    fpu.push(f32(app, cpu.ebp - 8));
    cpu.esp -= 8;
    storeDouble(app, cpu, cpu.esp);
    if (!byAddress(app, cpu, kFloor))
        return;
    storeFloat(app, cpu, cpu.ebp - 0x10);
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::abs(fpu.st(0));
    storeFloat(app, cpu, cpu.ebp - 0xc);
    if ((at(app, cpu.ebp - 0xc) & 0x7fffffff) != 0)
    {
        // Between two points.
        fpu.push(f32(app, cpu.ebp - 0x10));
        integerToEax(app, cpu);
        put(app, cpu.ebp - 0x24, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x24);
        ++cpu.eax;
        put(app, cpu.ebp - 0x20, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x34);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x24);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp - 0x1c, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x34);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x20);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp - 0x18, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x1c));
        storeFloat(app, cpu, cpu.ebp - 0x14);
        fpu.push(i32At(app, cpu.ebp - 0x24));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x14));
        fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x1c), fpu.st(0));
        storeFloat(app, cpu, cpu.ebp - 4);
        fpu.push(f32(app, cpu.ebp - 0x14));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 4));
        storeFloat(app, cpu, cpu.ebp - 0x28);
    }
    else
    {
        // On a point.
        cpu.eax = at(app, cpu.ebp - 0x34);
        cpu.edx = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.ebp - 8));
        integerToEax(app, cpu);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp - 0x28, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x28);
    put(app, cpu.ebp - 0x2c, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x2c));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4a90a0: the automatic box of car eax, in a forward gear: up a gear
 * (+0x50c cleared) past five sixths of the top revs while the gear above
 * is not the last but one (the last too, when the top gear's revs at the
 * handling's +0x1c8 are over the top), down two or one (+0x50c set) below
 * half the revs or seven eighths with the slip under 0.05 -- with the
 * throttle under 0.9 and the revs under the top, only down one.  The gear
 * changed: the last kept at +0x50a, the change's frames from the handling,
 * and its clunk played for the player at this machine (each car on split
 * screen). */
void automaticGears(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x40);
    put(app, cpu.ebp - 0x38, cpu.eax);
    for (reg32 at = 0x30; at >= 0x14; at -= 4)
        put(app, cpu.ebp - at, 0);
    // Five sixths and seven eighths of the top revs.
    cpu.edx = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.ecx = 6;
    cpu.eax = at(app, cpu.edx + 0x1c4);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx = reg32(i32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
    cpu.edx = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx -= cpu.eax;
    put(app, cpu.ebp - 0x10, cpu.edx);
    cpu.edx = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.eax = at(app, cpu.edx + 0x1c4);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    divideByShift(cpu, 3);
    cpu.edx = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx -= cpu.eax;
    put(app, cpu.ebp - 0xc, cpu.edx);
    put(app, cpu.ebp - 8, 0);
    cpu.eax = at(app, cpu.ebp - 0x38);
    if (byte(app, cpu.eax + 0x50b) < 2)
        return leave(app, cpu);
    // The slip, either way.
    cpu.eax = at(app, cpu.ebp - 0x38);
    zeroAgainst(cpu, f32(app, cpu.eax + 0x5b0));
    if (above(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        fpu.push(-f32(app, cpu.eax + 0x5b0));
        storeFloat(app, cpu, cpu.ebp - 0x3c);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x5b0);
        put(app, cpu.ebp - 0x3c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x3c);
    put(app, cpu.ebp - 8, cpu.eax);
    // The clunk's pan: split screen's sides.
    if (at(app, kPlayMode) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.ebp - 4, at(app, cpu.eax + 0x1f0) != 0 ? 0xc000 : 0x3fff);
    }
    else
    {
        put(app, cpu.ebp - 4, 0);
    }
    // The top gear's revs at the speed where the air drags harder: over the top, the top gear is used.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.edx = at(app, cpu.eax + 8);
    cpu.edx <<= 2;
    cpu.edx += at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(i32At(app, cpu.eax + 0x1c4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x544));
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    fcomp(cpu, f32(app, cpu.eax + 0x1c8));
    put(app, cpu.ebp - 0x14, above(cpu) ? 1 : 0);
    // The gears up one, down one and down two, where there are.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.eax + 8);
    --cpu.eax;
    cpu.eax -= at(app, cpu.ebp - 0x14);
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = byte(app, cpu.eax + 0x50b);
    if (i32(cpu.eax) < i32(cpu.edx))
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        ++cpu.eax;
        put(app, cpu.ebp - 0x18, cpu.eax);
    }
    else
    {
        cpu.eax = 0;
        cpu.edx = at(app, cpu.ebp - 0x38);
        cpu.al = byte(app, cpu.edx + 0x50b);
        put(app, cpu.ebp - 0x18, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    if (byte(app, cpu.eax + 0x50b) > 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        --cpu.eax;
        put(app, cpu.ebp - 0x20, cpu.eax);
    }
    else
    {
        cpu.eax = 0;
        cpu.edx = at(app, cpu.ebp - 0x38);
        cpu.al = byte(app, cpu.edx + 0x50b);
        put(app, cpu.ebp - 0x20, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    if (byte(app, cpu.eax + 0x50b) > 3)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        cpu.eax -= 2;
        put(app, cpu.ebp - 0x1c, cpu.eax);
    }
    else
    {
        cpu.eax = 0;
        cpu.edx = at(app, cpu.ebp - 0x38);
        cpu.al = byte(app, cpu.edx + 0x50b);
        put(app, cpu.ebp - 0x1c, cpu.eax);
    }
    // The revs at the wheels' speed in the gear and in each of them.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = byte(app, cpu.eax + 0x50b);
    cpu.eax <<= 2;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.edx += cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x38);
    fpu.push(f32(app, cpu.eax + 0x5a4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
    integerToEax(app, cpu);
    put(app, cpu.ebp - 0x30, cpu.eax);
    for (const reg32 gear : {0x18u, 0x20u})
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - gear);
        cpu.eax <<= 2;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, cpu.ebp - 0x38);
        fpu.push(f32(app, cpu.eax + 0x5a4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
        integerToEax(app, cpu);
        put(app, cpu.ebp - (gear == 0x18 ? 0x2c : 0x28), cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.edx = at(app, cpu.ebp - 0x1c);
    cpu.edx <<= 2;
    cpu.edx += cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x38);
    fpu.push(f32(app, cpu.eax + 0x5a4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
    integerToEax(app, cpu);
    put(app, cpu.ebp - 0x24, cpu.eax);

    // A change to the gear at [ebp - gear]: down (+0x50c) or up, its clunk.
    const auto change = [&](reg32 gear, reg8 down) {
        cpu.eax = at(app, cpu.ebp - 0x38);
        putByte(app, cpu.eax + 0x50c, down);
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.dl = byte(app, cpu.eax + 0x50b);
        cpu.eax = at(app, cpu.ebp - 0x38);
        putByte(app, cpu.eax + 0x50a, cpu.dl);
        cpu.al = byte(app, cpu.ebp - gear);
        cpu.edx = at(app, cpu.ebp - 0x38);
        putByte(app, cpu.edx + 0x50b, cpu.al);
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.dx = half(app, cpu.eax + 0x10);
        cpu.eax = at(app, cpu.ebp - 0x38);
        putHalf(app, cpu.eax + 0x510, cpu.dx);
        if (i32(at(app, kPlayMode)) >= 2)
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x1f0);
            if (cpu.eax != at(app, kLocalPlayer))
                return;
        }
        if (!gearKnock(app, cpu, 4))
            return;
    };
    // Half the top revs.
    const auto halfTop = [&] {
        cpu.edx = at(app, cpu.ebp - 0x38);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.edx + 0x1c4);
        cpu.edx = at(app, cpu.edx + 0x1c4);
        cpu.edx = reg32(i32(cpu.edx) >> 31);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(i32(cpu.eax) >> 1);
    };

    cpu.eax = at(app, cpu.ebp - 0x38);
    bool shifting = i32(at(app, cpu.eax + 0x5f0)) > i32(0x3f666666);
    if (!shifting)
    {
        cpu.eax = at(app, cpu.ebp - 0x30);
        put(app, cpu.ebp - 0x40, cpu.eax);
        if (i32(cpu.eax) < 0)
            put(app, cpu.ebp - 0x40, reg32(-i32(cpu.eax)));
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x40);
        shifting = i32(cpu.eax) > i32(at(app, cpu.edx + 0x1c4));
    }
    if (!shifting)
    {
        // Light on the throttle, under the top revs: only down one.
        halfTop();
        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)) && at(app, cpu.ebp - 0x20) == 2)
            goto downOne;
        cpu.eax = at(app, cpu.ebp - 0x28);
        if (i32(cpu.eax) >= i32(at(app, cpu.ebp - 0x10)) || i32(at(app, cpu.ebp - 0x20)) <= 2
            || i32(at(app, cpu.ebp - 8)) >= i32(0x3dcccccd))
            return leave(app, cpu);
    downOne:
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        if (cpu.eax == at(app, cpu.ebp - 0x20))
            return leave(app, cpu);
        change(0x20, 1);
        if (cpu.terminate)
            return;
        return leave(app, cpu);
    }
    // Up a gear.
    cpu.eax = at(app, cpu.ebp - 0x30);
    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0xc)))
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.eax + 8);
        --cpu.eax;
        cpu.eax -= at(app, cpu.ebp - 0x14);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        if (i32(cpu.eax) < i32(cpu.edx))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = byte(app, cpu.eax + 0x50b);
            if (cpu.eax != at(app, cpu.ebp - 0x18))
            {
                change(0x18, 0);
                if (cpu.terminate)
                    return;
            }
            return leave(app, cpu);
        }
    }
    // Down two.
    halfTop();
    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x24)) && at(app, cpu.ebp - 0x1c) == 2
        && i32(at(app, cpu.ebp - 8)) < i32(0x3d4ccccd))
        goto downTwo;
    cpu.eax = at(app, cpu.ebp - 0x24);
    if (i32(cpu.eax) < i32(at(app, cpu.ebp - 0x10)) && i32(at(app, cpu.ebp - 0x1c)) > 2
        && i32(at(app, cpu.ebp - 8)) < i32(0x3d4ccccd))
        goto downTwo;
    goto notTwo;
downTwo:
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = byte(app, cpu.eax + 0x50b);
    if (cpu.eax != at(app, cpu.ebp - 0x1c))
    {
        change(0x1c, 1);
        if (cpu.terminate)
            return;
        return leave(app, cpu);
    }
notTwo:
    // Down one.
    halfTop();
    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)) && at(app, cpu.ebp - 0x20) == 2)
    {
        fpu.push(f32(app, cpu.ebp - 8));
        fcomp(cpu, f64(app, 0x53dcac));
        if (below(cpu))
            goto downOneToo;
    }
    cpu.eax = at(app, cpu.ebp - 0x28);
    if (i32(cpu.eax) >= i32(at(app, cpu.ebp - 0x10)) || i32(at(app, cpu.ebp - 0x20)) <= 2
        || i32(at(app, cpu.ebp - 8)) >= i32(0x3d4ccccd))
        return leave(app, cpu);
downOneToo:
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = byte(app, cpu.eax + 0x50b);
    if (cpu.eax == at(app, cpu.ebp - 0x20))
        return leave(app, cpu);
    change(0x20, 1);
    if (cpu.terminate)
        return;
    leave(app, cpu);
}

/* 0x4a9769: car eax with a manual box kept out of its fifth gear and over
 * (to the fourth, the third the last) while the top revs in the first
 * forward gear's ratio (+0x554) are over its speed. */
void gearLimit(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 8);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(i32At(app, cpu.eax + 0x1c4));
    cpu.eax = at(app, cpu.ebp - 8);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x554));
    cpu.eax = at(app, cpu.ebp - 8);
    fcomp(cpu, f32(app, cpu.eax + 0x4f4));
    if (above(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x50b) > 4)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            putByte(app, cpu.eax + 0x50b, 4);
            cpu.eax = at(app, cpu.ebp - 8);
            putByte(app, cpu.eax + 0x50a, 3);
        }
    }
    leave(app, cpu);
}

/* 0x4a97cb: car eax put into reverse, and out of it, as its driver's pedals
 * say when it all but stands (its velocity's length, +0x4ec, under 0.5): a
 * player's car with the automatic box, once the race is under way and the
 * countdown over -- else nothing (+0x507 cleared).  The brake held
 * (over 0x80) with the throttle off in first or neutral: into reverse
 * (+0x506 0, +0x507 set); the throttle in reverse, or in neutral, back into
 * first; in reverse or first, off both, at idle: into neutral, at rest.
 * Reversing on its own, its pedals swapped.  Gives +0x507. */
void autoReverse(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x20);
    put(app, cpu.ebp - 8, cpu.eax);
    // The length of the velocity, either way: [ebp - local].
    const auto speed = [&](reg32 local) {
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax += 0x4ec;
        if (!byAddress(app, cpu, kVectorLength))
            return false;
        fpu.push(0.0);
        fcompp(cpu);
        const bool negative = above(cpu);
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax += 0x4ec;
        if (!byAddress(app, cpu, kVectorLength))
            return false;
        if (negative)
            fpu.st(0) = -fpu.st(0);
        storeFloat(app, cpu, cpu.ebp - local);
        return true;
    };
    // Whether the car is not a player's that finished.
    const auto racing = [&] {
        cpu.eax = at(app, cpu.ebp - 8);
        if ((byte(app, cpu.eax + 0x200) & 1) == 0)
            return true;
        cpu.eax = at(app, cpu.ebp - 8);
        return at(app, cpu.eax + 0x2c4) != 3;
    };
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
    bool automatic = false;
    if (at(app, cpu.eax + kPlayerGearbox) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if ((byte(app, cpu.eax + 0x200) & 1) == 0)
            automatic = true;
        else
        {
            cpu.eax = at(app, cpu.ebp - 8);
            automatic = at(app, cpu.eax + 0x2c4) != 3;
        }
    }
    if (!automatic || at(app, kRaceUnderWay) == 0 || i32(at(app, kRaceFrames)) < 0x23c)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x507, 0);
        put(app, cpu.ebp - 4, 0);
        cpu.eax = at(app, cpu.ebp - 4);
        return leave(app, cpu);
    }
    cpu.eax = at(app, cpu.ebp - 8);
    if (byte(app, cpu.eax + 0x50b) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x507, 0);
    }
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.dl = byte(app, cpu.eax + 0x506);
    cpu.eax = at(app, cpu.ebp - 8);
    if (cpu.dl != byte(app, cpu.eax + 0x50b))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x507, 0);
        goto swap;
    }
    // The brake held, the throttle off, in first or neutral: into reverse.
    if (!speed(0xc))
        return;
    if (i32(at(app, cpu.ebp - 0xc)) < i32(0x3f000000))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x505) > 0x80)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            if (byte(app, cpu.eax + 0x508) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                bool gear = byte(app, cpu.eax + 0x50b) == 2;
                if (!gear)
                {
                    cpu.eax = at(app, cpu.ebp - 8);
                    gear = byte(app, cpu.eax + 0x50b) == 1;
                }
                if (gear)
                {
                    put(app, cpu.ebp - 0x10, racing() ? 1 : 0);
                    if (at(app, cpu.ebp - 0x10) != 0)
                    {
                        cpu.eax = at(app, cpu.ebp - 8);
                        if (byte(app, cpu.eax + 0x507) == 0)
                        {
                            cpu.eax = at(app, cpu.ebp - 8);
                            putByte(app, cpu.eax + 0x506, 0);
                            cpu.eax = at(app, cpu.ebp - 8);
                            putByte(app, cpu.eax + 0x507, 1);
                            goto swap;
                        }
                    }
                }
            }
        }
    }
    // The throttle in reverse (its brake's pedal, swapped) or in neutral: into first.
    if (!speed(0x14))
        return;
    if (i32(at(app, cpu.ebp - 0x14)) < i32(0x3f000000))
    {
        bool pressed;
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x504) > 0x80)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            pressed = byte(app, cpu.eax + 0x50b) == 0;
            if (pressed)
                goto pedals;
        }
        cpu.eax = at(app, cpu.ebp - 8);
        pressed = false;
        if (byte(app, cpu.eax + 0x504) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            pressed = byte(app, cpu.eax + 0x50b) == 1;
        }
        if (!pressed)
            goto neutral;
    pedals:
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x508) == 0)
        {
            bool first = false;
            cpu.eax = at(app, cpu.ebp - 8);
            if (byte(app, cpu.eax + 0x50b) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                first = byte(app, cpu.eax + 0x507) != 0;
            }
            if (!first)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                if (byte(app, cpu.eax + 0x50b) == 1)
                {
                    cpu.eax = at(app, cpu.ebp - 8);
                    first = byte(app, cpu.eax + 0x507) == 0;
                }
            }
            if (first)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                putByte(app, cpu.eax + 0x506, 2);
                cpu.eax = at(app, cpu.ebp - 8);
                putByte(app, cpu.eax + 0x507, 0);
                goto swap;
            }
        }
    }
neutral:
    // The throttle in neutral with the automatic box: into first.
    cpu.eax = at(app, cpu.ebp - 8);
    if (byte(app, cpu.eax + 0x504) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x50b) == 1)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            if (byte(app, cpu.eax + 0x508) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
                if (at(app, cpu.eax + kPlayerGearbox) == 1)
                {
                    cpu.eax = at(app, cpu.ebp - 8);
                    putByte(app, cpu.eax + 0x506, 2);
                    cpu.eax = at(app, cpu.ebp - 8);
                    putByte(app, cpu.eax + 0x507, 0);
                    goto swap;
                }
            }
        }
    }
    // Off both in reverse or first, at idle: into neutral, at rest.
    if (!speed(0x18))
        return;
    if (i32(at(app, cpu.ebp - 0x18)) < i32(0x3f000000))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x508) == 0)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            if (byte(app, cpu.eax + 0x507) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                if (byte(app, cpu.eax + 0x504) < 8)
                {
                    cpu.eax = at(app, cpu.ebp - 8);
                    bool gear = byte(app, cpu.eax + 0x50b) == 0;
                    if (!gear)
                    {
                        cpu.eax = at(app, cpu.ebp - 8);
                        gear = byte(app, cpu.eax + 0x50b) == 2;
                    }
                    if (gear)
                    {
                        cpu.eax = at(app, cpu.ebp - 8);
                        cpu.eax = at(app, cpu.eax + 0x528);
                        cpu.edx = at(app, cpu.eax + 0x1c0);
                        cpu.edx += 0x32;
                        cpu.eax = at(app, cpu.ebp - 8);
                        if (i32(cpu.edx) > i32(at(app, cpu.eax + 0x590)))
                        {
                            cpu.eax = at(app, cpu.ebp - 8);
                            putByte(app, cpu.eax + 0x506, 1);
                            cpu.eax = at(app, cpu.ebp - 8);
                            putByte(app, cpu.eax + 0x507, 0);
                            cpu.eax = at(app, cpu.ebp - 8);
                            putByte(app, cpu.eax + 0x509, 0xff);
                            cpu.eax = at(app, cpu.ebp - 8);
                            put(app, cpu.eax + 0x4ec, 0);
                            cpu.eax = at(app, cpu.ebp - 8);
                            put(app, cpu.eax + 0x4f0, 0);
                            cpu.eax = at(app, cpu.ebp - 8);
                            put(app, cpu.eax + 0x4f4, 0);
                        }
                    }
                }
            }
        }
    }
swap:
    // Reversing on its own: the throttle's pedal brakes, the brake's drives.
    cpu.eax = at(app, cpu.ebp - 8);
    if (byte(app, cpu.eax + 0x507) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (byte(app, cpu.eax + 0x50b) == 0)
        {
            cpu.eax = 0;
            cpu.edx = at(app, cpu.ebp - 8);
            cpu.al = byte(app, cpu.edx + 0x504);
            put(app, cpu.ebp - 0x1c, cpu.eax);
            cpu.eax = 0;
            cpu.edx = at(app, cpu.ebp - 8);
            cpu.al = byte(app, cpu.edx + 0x505);
            put(app, cpu.ebp - 0x20, cpu.eax);
            cpu.al = byte(app, cpu.ebp - 0x1c);
            cpu.edx = at(app, cpu.ebp - 8);
            putByte(app, cpu.edx + 0x505, cpu.al);
            cpu.al = byte(app, cpu.ebp - 0x20);
            cpu.edx = at(app, cpu.ebp - 8);
            putByte(app, cpu.edx + 0x504, cpu.al);
        }
    }
    cpu.eax = 0;
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.al = byte(app, cpu.edx + 0x507);
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    leave(app, cpu);
}

/* 0x4a9bc4: the inputs of car eax, each frame.  Into reverse and out
 * (autoReverse) first.  A player's car that finished: let go, in neutral,
 * slowing (its speed and spin by 0.98, 0x4e0050) while racing.  The
 * computer's car, or the player's driven for him: computerDriver.  Else the
 * throttle (+0x508) eased towards what is asked (+0x504) by 16 a frame (more
 * with its driver's skill, in a single race), and the brake (+0x509) towards
 * +0x505 by the handling's steps (+0x1f0 down, +0x210 up, by how far down it
 * is) or at once; a gear change's frames counted down; the gear asked
 * (+0x506; first until the countdown is over, with the automatic box) put
 * in: by the automatic box, into reverse or neutral as asked, or forward
 * into the gear the speed gives -- the clunk played, +0x50c whether it went
 * down, the force feedback's kick (0x4714f0) with a manual one.  The
 * steering (+0x51c) eased towards what is asked (+0x518) by the handling's
 * steps (+0x240 away, +0x244 back to the middle) or at once.  The throttle,
 * brake and steering to 0..1 at +0x5f0, +0x5ec and +0x5e8. */
void driverInputs(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x80);
    put(app, cpu.ebp - 0x1c, cpu.eax);
    put(app, cpu.ebp - 4, 0);
    if (at(app, kPlayMode) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.ebp - 8, at(app, cpu.eax + 0x1f0) != 0 ? 0xc000 : 0x3fff);
    }
    else
    {
        put(app, cpu.ebp - 8, 0);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (!direct(app, cpu, autoReverse))
        return;
    bool finished = false;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if ((byte(app, cpu.eax + 0x200) & 1) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        finished = at(app, cpu.eax + 0x2c4) == 3;
    }
    if (finished)
    {
        // Finished: let go, in neutral, slowing.
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.eax + 0x51c, 0);
        for (const auto& [field, value] : {std::pair<reg32, reg8>{0x508, 0}, {0x50c, 0}, {0x504, 0}, {0x506, 1},
                                           {0x50b, 1}, {0x50d, 1}, {0x509, 0xff}})
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            putByte(app, cpu.eax + field, value);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (half(app, cpu.eax + 0x164) == 0)
        {
            for (const reg32 vector : {0xa4u, 0xe4u})
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax += vector;
                push(app, cpu, cpu.eax);
                push(app, cpu, 0x3f7ae148);
                cpu.edx = at(app, cpu.ebp - 0x1c);
                cpu.edx += vector;
                cpu.eax = 1;
                if (!byAddress(app, cpu, kScaleVectors))
                    return;
            }
        }
        goto ratios;
    }
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        bool notFinished = (byte(app, cpu.eax + 0x200) & 1) == 0;
        if (!notFinished)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            notFinished = at(app, cpu.eax + 0x2c4) != 3;
        }
        put(app, cpu.ebp - 0x20, notFinished ? 1 : 0);
    }
    if (at(app, cpu.ebp - 0x20) != 0 && (at(app, kComputerDrives) != 0 || at(app, kComputerDrivesToo) != 0))
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (!direct(app, cpu, computerDriver))
            return;
    }
    else
    {
        // The throttle eased: 16 a frame, more with the driver's skill in a single race.
        putByte(app, cpu.ebp - 0x24, 0x10);
        putByte(app, cpu.ebp - 0x28, 0x10);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x220);
        if (i32(at(app, cpu.eax + 0x48)) > i32(0x3f800000) && i32(at(app, kPlayMode)) <= 1
            && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x220);
            fpu.push(f32(app, cpu.eax + 0x48));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dcb4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dcb8));
            integerToEax(app, cpu);
            putByte(app, cpu.ebp - 0x24, reg8(byte(app, cpu.ebp - 0x24) + cpu.al));
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = byte(app, cpu.eax + 0x504);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = byte(app, cpu.eax + 0x508);
        cpu.edx -= cpu.eax;
        put(app, cpu.ebp - 0x10, cpu.edx);
        if (i32(at(app, cpu.ebp - 0x10)) >= 0)
        {
            cpu.edx = byte(app, cpu.ebp - 0x28);
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax += 0x508;
            put(app, cpu.ebp - 0x2c, cpu.eax);
            if (i32(cpu.edx) > i32(at(app, cpu.ebp - 0x10)))
            {
                cpu.eax = at(app, cpu.ebp - 0x10);
                put(app, cpu.ebp - 0x30, cpu.eax);
            }
            else
            {
                cpu.eax = byte(app, cpu.ebp - 0x28);
                put(app, cpu.ebp - 0x30, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x2c);
            cpu.eax = byte(app, cpu.eax);
            cpu.eax += at(app, cpu.ebp - 0x30);
            cpu.edx = at(app, cpu.ebp - 0x2c);
            putByte(app, cpu.edx, cpu.al);
        }
        else
        {
            cpu.ecx = byte(app, cpu.ebp - 0x24);
            cpu.edx = at(app, cpu.ebp - 0x10);
            cpu.edx = reg32(-i32(cpu.edx));
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax += 0x508;
            put(app, cpu.ebp - 0x34, cpu.eax);
            if (i32(cpu.ecx) > i32(cpu.edx))
            {
                cpu.eax = at(app, cpu.ebp - 0x10);
                put(app, cpu.ebp - 0x38, reg32(-i32(cpu.eax)));
            }
            else
            {
                cpu.eax = byte(app, cpu.ebp - 0x24);
                put(app, cpu.ebp - 0x38, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x34);
            cpu.eax = byte(app, cpu.eax);
            cpu.eax -= at(app, cpu.ebp - 0x38);
            cpu.edx = at(app, cpu.ebp - 0x34);
            putByte(app, cpu.edx, cpu.al);
        }
        // The brake eased by the handling's steps, by how far down it is (by 32), or at once.
        const auto step = [&] {
            cpu.edx = at(app, cpu.ebp - 0x1c);
            cpu.edx = byte(app, cpu.edx + 0x509);
            cpu.eax = cpu.edx;
            divideByShift(cpu, 5);
            cpu.eax <<= 2;
        };
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x220);
        if (at(app, cpu.eax + 0x2c) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.edx = byte(app, cpu.eax + 0x505);
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = byte(app, cpu.eax + 0x509);
            cpu.edx -= cpu.eax;
            put(app, cpu.ebp - 0x10, cpu.edx);
            if (i32(at(app, cpu.ebp - 0x10)) >= 0)
            {
                step();
                cpu.edx = cpu.eax;
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.eax += cpu.edx;
                fpu.push(f32(app, cpu.eax + 0x1f0));
                integerToEax(app, cpu);
                cpu.edx = cpu.eax;
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax += 0x509;
                put(app, cpu.ebp - 0x3c, cpu.eax);
                if (i32(cpu.edx) > i32(at(app, cpu.ebp - 0x10)))
                {
                    cpu.eax = at(app, cpu.ebp - 0x10);
                    put(app, cpu.ebp - 0x40, cpu.eax);
                }
                else
                {
                    step();
                    cpu.ecx = cpu.eax;
                    cpu.edx = at(app, cpu.ebp - 0x1c);
                    cpu.eax = at(app, cpu.edx + 0x528);
                    cpu.eax += cpu.ecx;
                    fpu.push(f32(app, cpu.eax + 0x1f0));
                    integerToEax(app, cpu);
                    put(app, cpu.ebp - 0x40, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.eax = byte(app, cpu.eax);
                cpu.eax += at(app, cpu.ebp - 0x40);
                cpu.edx = at(app, cpu.ebp - 0x3c);
                putByte(app, cpu.edx, cpu.al);
            }
            else
            {
                step();
                cpu.edx = cpu.eax;
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.eax += cpu.edx;
                fpu.push(f32(app, cpu.eax + 0x210));
                integerToEax(app, cpu);
                cpu.edx = cpu.eax;
                cpu.ecx = at(app, cpu.ebp - 0x10);
                cpu.ecx = reg32(-i32(cpu.ecx));
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax += 0x509;
                put(app, cpu.ebp - 0x44, cpu.eax);
                if (i32(cpu.edx) > i32(cpu.ecx))
                {
                    cpu.eax = at(app, cpu.ebp - 0x10);
                    put(app, cpu.ebp - 0x48, reg32(-i32(cpu.eax)));
                }
                else
                {
                    step();
                    cpu.ecx = cpu.eax;
                    cpu.edx = at(app, cpu.ebp - 0x1c);
                    cpu.eax = at(app, cpu.edx + 0x528);
                    cpu.eax += cpu.ecx;
                    fpu.push(f32(app, cpu.eax + 0x210));
                    integerToEax(app, cpu);
                    put(app, cpu.ebp - 0x48, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - 0x44);
                cpu.eax = byte(app, cpu.eax);
                cpu.eax -= at(app, cpu.ebp - 0x48);
                cpu.edx = at(app, cpu.ebp - 0x44);
                putByte(app, cpu.edx, cpu.al);
            }
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.dl = byte(app, cpu.eax + 0x505);
            cpu.eax = at(app, cpu.ebp - 0x1c);
            putByte(app, cpu.eax + 0x509, cpu.dl);
        }
    }
    // A gear change's frames counted down.
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (i16(half(app, cpu.eax + 0x510)) > 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        putHalf(app, cpu.eax + 0x510, reg16(half(app, cpu.eax + 0x510) - 1));
    }
    // The gear asked put in.
    if (at(app, kRaceUnderWay) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
        if (at(app, cpu.eax + kPlayerGearbox) == 1 && i32(at(app, kRaceFrames)) < 0x23c)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            putByte(app, cpu.eax + 0x506, 2);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.dl = byte(app, cpu.eax + 0x506);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (cpu.dl != byte(app, cpu.eax + 0x50b))
        {
            // The clunk for the player at this machine, or each on split screen.
            const auto knock = [&] {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x1f0);
                if (cpu.eax == at(app, kLocalPlayer) || at(app, kPlayMode) == 1)
                    return gearKnock(app, cpu, 8);
                return true;
            };
            // The change's frames, from the handling.
            const auto changeFrames = [&] {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.dx = half(app, cpu.eax + 0x10);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                putHalf(app, cpu.eax + 0x510, cpu.dx);
            };
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
            if (at(app, cpu.eax + kPlayerGearbox) == 1)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                if (byte(app, cpu.eax + 0x506) < 2)
                {
                    // Into reverse or neutral, as asked.
                    if (!knock())
                        return;
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    putByte(app, cpu.eax + 0x50c, 0);
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    cpu.dl = byte(app, cpu.eax + 0x50b);
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    putByte(app, cpu.eax + 0x50a, cpu.dl);
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    cpu.dl = byte(app, cpu.eax + 0x506);
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    putByte(app, cpu.eax + 0x50b, cpu.dl);
                    changeFrames();
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    bool forward = byte(app, cpu.eax + 0x506) == 2;
                    if (forward)
                    {
                        cpu.eax = at(app, cpu.ebp - 0x1c);
                        forward = byte(app, cpu.eax + 0x50b) < 2;
                    }
                    if (forward)
                    {
                        // Out of reverse or neutral: into the highest gear whose revs
                        // at the top are past the speed.
                        if (!knock())
                            return;
                        cpu.eax = at(app, cpu.ebp - 0x1c);
                        cpu.dl = byte(app, cpu.eax + 0x50b);
                        cpu.eax = at(app, cpu.ebp - 0x1c);
                        putByte(app, cpu.eax + 0x50a, cpu.dl);
                        put(app, cpu.ebp - 0xc, 2);
                        put(app, cpu.ebp - 0x14, 2);
                        for (;;)
                        {
                            cpu.eax = at(app, cpu.ebp - 0x1c);
                            cpu.edx = at(app, cpu.eax + 0x528);
                            cpu.eax = at(app, cpu.ebp - 0x14);
                            if (i32(cpu.eax) >= i32(at(app, cpu.edx + 8)))
                                break;
                            cpu.edx = at(app, cpu.ebp - 0x14);
                            cpu.edx <<= 2;
                            cpu.edx += at(app, cpu.ebp - 0x1c);
                            cpu.eax = at(app, cpu.ebp - 0x1c);
                            cpu.eax = at(app, cpu.eax + 0x528);
                            fpu.push(i32At(app, cpu.eax + 0x1c4));
                            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x54c));
                            cpu.eax = at(app, cpu.ebp - 0x1c);
                            fcomp(cpu, f32(app, cpu.eax + 0x4f4));
                            if (below(cpu))
                            {
                                cpu.eax = at(app, cpu.ebp - 0x14);
                                put(app, cpu.ebp - 0xc, cpu.eax);
                            }
                            cpu.eax = at(app, cpu.ebp - 0x14);
                            put(app, cpu.ebp - 0x14, at(app, cpu.ebp - 0x14) + 1);
                        }
                        cpu.eax = at(app, cpu.ebp - 0x1c);
                        putByte(app, cpu.eax + 0x50c, 0);
                        cpu.al = byte(app, cpu.ebp - 0xc);
                        cpu.edx = at(app, cpu.ebp - 0x1c);
                        putByte(app, cpu.edx + 0x50b, cpu.al);
                        changeFrames();
                    }
                }
            }
            else
            {
                // The manual box: the gear asked, down or up, and the wheel's kick.
                if (!knock())
                    return;
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.dl = byte(app, cpu.eax + 0x50b);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                bool down = cpu.dl > byte(app, cpu.eax + 0x506);
                if (down)
                {
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    down = byte(app, cpu.eax + 0x506) > 1;
                }
                cpu.eax = at(app, cpu.ebp - 0x1c);
                putByte(app, cpu.eax + 0x50c, down ? 1 : 0);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.dl = byte(app, cpu.eax + 0x50b);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                putByte(app, cpu.eax + 0x50a, cpu.dl);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.dl = byte(app, cpu.eax + 0x506);
                cpu.eax = at(app, cpu.ebp - 0x1c);
                putByte(app, cpu.eax + 0x50b, cpu.dl);
                changeFrames();
                cpu.eax = at(app, cpu.ebp - 0x1c);
                if (!byAddress(app, cpu, kGearChangeKick))
                    return;
            }
        }
    }
    // The steering eased by the handling's steps -- away from the middle,
    // back to it -- or at once.
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x220);
    if (at(app, cpu.eax + 0x24) != 0)
    {
        // Towards the middle when the steering and what is asked are either side of it.
        bool back = false;
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (i32(at(app, cpu.eax + 0x518)) >= 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            back = i32(at(app, cpu.eax + 0x51c)) < 0;
        }
        if (!back)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            if (i32(at(app, cpu.eax + 0x518)) <= 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                back = i32(at(app, cpu.eax + 0x51c)) > 0;
            }
        }
        // The steering moved by [ebp - 0x10] at most [ebp - 4], either way.
        const auto ease = [&](reg32 up, reg32 upKept, reg32 down, reg32 downKept) {
            if (i32(at(app, cpu.ebp - 0x10)) >= 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax += 0x51c;
                put(app, cpu.ebp - up, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 4);
                if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x10)))
                {
                    cpu.eax = at(app, cpu.ebp - 0x10);
                    put(app, cpu.ebp - upKept, cpu.eax);
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 4);
                    put(app, cpu.ebp - upKept, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - upKept);
                cpu.edx = at(app, cpu.ebp - up);
                put(app, cpu.edx, at(app, cpu.edx) + cpu.eax);
            }
            else
            {
                cpu.edx = at(app, cpu.ebp - 0x10);
                cpu.edx = reg32(-i32(cpu.edx));
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax += 0x51c;
                put(app, cpu.ebp - down, cpu.eax);
                if (i32(cpu.edx) < i32(at(app, cpu.ebp - 4)))
                {
                    cpu.eax = at(app, cpu.ebp - 0x10);
                    put(app, cpu.ebp - downKept, reg32(-i32(cpu.eax)));
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 4);
                    put(app, cpu.ebp - downKept, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - downKept);
                cpu.edx = at(app, cpu.ebp - down);
                put(app, cpu.edx, at(app, cpu.edx) - cpu.eax);
            }
        };
        if (back)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x51c);
            put(app, cpu.ebp - 0x10, cpu.eax);
            put(app, cpu.ebp - 0x10, reg32(-i32(at(app, cpu.ebp - 0x10))));
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(f32(app, cpu.eax + 0x244));
            integerToEax(app, cpu);
            put(app, cpu.ebp - 4, cpu.eax);
            ease(0x4c, 0x50, 0x54, 0x58);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x518);
            cpu.edx = at(app, cpu.ebp - 0x1c);
            cpu.eax -= at(app, cpu.edx + 0x51c);
            put(app, cpu.ebp - 0x10, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(f32(app, cpu.eax + 0x240));
            integerToEax(app, cpu);
            put(app, cpu.ebp - 4, cpu.eax);
            ease(0x5c, 0x60, 0x64, 0x68);
        }
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x518);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.eax + 0x51c, cpu.edx);
    }
ratios:
    // Before the race the brake held; out of a state 2 no throttle.
    if (at(app, kRaceUnderWay) == 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        putByte(app, cpu.eax + 0x509, 0xff);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (at(app, cpu.eax + 0x2e4) == 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        putByte(app, cpu.eax + 0x508, 0);
    }
    // The throttle and the brake by 255, 1 at most.
    for (const auto& [pedal, local, ratio] : {std::tuple<reg32, reg32, reg32>{0x508, 0x74, 0x5f0}, {0x509, 0x7c, 0x5ec}})
    {
        cpu.eax = 0;
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.al = byte(app, cpu.edx + pedal);
        put(app, cpu.ebp - 0x6c, cpu.eax);
        fpu.push(i16At(app, cpu.ebp - 0x6c));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53dcbc));
        fpu.push(1.0);
        fcompp(cpu);
        if (above(cpu))
        {
            cpu.eax = 0;
            cpu.edx = at(app, cpu.ebp - 0x1c);
            cpu.al = byte(app, cpu.edx + pedal);
            put(app, cpu.ebp - 0x6c, cpu.eax);
            fpu.push(i16At(app, cpu.ebp - 0x6c));
            fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53dcbc));
            storeDouble(app, cpu, cpu.ebp - local);
        }
        else
        {
            put(app, cpu.ebp - local, 0);
            put(app, cpu.ebp - local + 4, 0x3ff00000);
        }
        fpu.push(f64(app, cpu.ebp - local));
        if (pedal == 0x508)
        {
            cpu.edx = at(app, cpu.ebp - 0x1c);
            storeFloat(app, cpu, cpu.edx + ratio);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            storeFloat(app, cpu, cpu.eax + ratio);
        }
    }
    // The steering's, by 128 (the steering one more).
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x51c);
    ++cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x51c);
    ++cpu.eax;
    put(app, cpu.ebp - 0x80, cpu.eax);
    if (i32(cpu.eax) < 0)
        put(app, cpu.ebp - 0x80, reg32(-i32(cpu.eax)));
    fpu.push(i32At(app, cpu.ebp - 0x80));
    cpu.eax = at(app, cpu.ebp - 0x1c);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dcc0));
    storeFloat(app, cpu, cpu.eax + 0x5e8);
    leave(app, cpu);
}

/* 0x4aab41: the grip of car eax's wheels: its scale (+0x134, 0.75 to 1)
 * at each, by the surface each is on (wheelSurface, kSurfaceGrip's) as the
 * track's kind (+0x160) has it, 1.41 times as a simulation's; and the grip
 * growing with the speed past 10 (+0x5c8..+0x5d4, the handling's +0x24c,
 * 1.41 times as a simulation's, the back wheels' 1.5 times), else as the
 * handling has it. */
void wheelGrip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x24);
    put(app, cpu.ebp - 0x14, cpu.eax);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x14);
    cpu.eax = at(app, cpu.eax + 0x134);
    put(app, cpu.ebp - 4, cpu.eax);
    if (i32(at(app, cpu.ebp - 4)) > i32(0x3f800000))
        put(app, cpu.ebp - 4, 0x3f800000);
    else if (i32(at(app, cpu.ebp - 4)) < i32(0x3f400000))
        put(app, cpu.ebp - 4, 0x3f400000);
    for (const reg32 field : {0x5d8u, 0x5dcu, 0x5e4u, 0x5e0u})
    {
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = at(app, cpu.ebp - 0x14);
        put(app, cpu.edx + field, cpu.eax);
    }
    // Each wheel by its surface.
    put(app, cpu.ebp - 8, 0);
    while (i32(at(app, cpu.ebp - 8)) < 4)
    {
        cpu.edx = at(app, cpu.ebp - 8);
        cpu.eax = at(app, cpu.ebp - 0x14);
        if (!byAddress(app, cpu, kWheelSurface))
            return;
        put(app, cpu.ebp - 0xc, cpu.eax);
        // The wheel's grip and its surface's.
        const auto wheel = [&] {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.eax <<= 2;
            cpu.eax += at(app, cpu.ebp - 0x14);
            cpu.edx = at(app, cpu.ebp - 0xc);
            cpu.edx <<= 2;
        };
        cpu.eax = at(app, cpu.ebp - 0x14);
        const reg32 kind = at(app, cpu.eax + 0x160);
        if (kind == 2 || (cpu.eax = at(app, cpu.ebp - 0x14), at(app, cpu.eax + 0x160) == 1))
        {
            const reg32 first = kind == 2 ? 0x53dcd4 : 0x53dce4, second = kind == 2 ? 0x53dcdc : 0x53dcec;
            wheel();
            fpu.push(f32(app, cpu.edx + kSurfaceGrip));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 4));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, first));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, second));
            fpu.push(f32(app, cpu.eax + 0x5d8));
            storeDouble(app, cpu, cpu.ebp - 0x1c);
            fpu.st(0) = fp::add(fpu.st(0), f64(app, cpu.ebp - 0x1c));
            storeDouble(app, cpu, cpu.ebp - 0x1c);
            fpu.push(f64(app, cpu.ebp - 0x1c));
            storeFloat(app, cpu, cpu.eax + 0x5d8);
        }
        else
        {
            wheel();
            fpu.push(f32(app, cpu.edx + kSurfaceGrip));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dcf4));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x5d8));
            storeFloat(app, cpu, cpu.eax + 0x5d8);
        }
        wheel();
        fpu.push(f32(app, cpu.edx + kSurfaceGrip));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x5d8));
        storeFloat(app, cpu, cpu.eax + 0x5d8);
        if (at(app, kSimulation) != 0 || (byte(app, kLoadingFlags) & 1) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.eax <<= 2;
            cpu.eax += at(app, cpu.ebp - 0x14);
            fpu.push(f32(app, cpu.eax + 0x5d8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dcf8));
            storeFloat(app, cpu, cpu.eax + 0x5d8);
        }
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 8, at(app, cpu.ebp - 8) + 1);
    }
    // The grip with the speed, either way.
    cpu.eax = at(app, cpu.ebp - 0x14);
    zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
    if (above(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(-f32(app, cpu.eax + 0x4f4));
        storeFloat(app, cpu, cpu.ebp - 0x20);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax = at(app, cpu.eax + 0x4f4);
        put(app, cpu.ebp - 0x20, cpu.eax);
    }
    // 1 + the speed by a growth, to [car + field].
    const auto grow = [&](reg32 field) {
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(f32(app, cpu.eax + 0x4f4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x24));
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        storeFloat(app, cpu, cpu.eax + field);
    };
    // [car + from] copied to [car + to].
    const auto copy = [&](reg32 from, reg32 to) {
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(f32(app, cpu.eax + from));
        cpu.eax = at(app, cpu.ebp - 0x14);
        storeFloat(app, cpu, cpu.eax + to);
    };
    if (i32(at(app, cpu.ebp - 0x20)) > i32(0x41200000))
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.eax + 0x24c);
        put(app, cpu.ebp - 0x24, cpu.eax);
        if (at(app, kSimulation) != 0)
        {
            fpu.push(f32(app, cpu.ebp - 0x24));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dcfc));
            storeFloat(app, cpu, cpu.ebp - 0x24);
        }
        grow(0x5c8);
        grow(0x5cc);
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x24c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd00));
        storeFloat(app, cpu, cpu.ebp - 0x24);
        grow(0x5d4);
        grow(0x5d0);
    }
    else if (at(app, kSimulation) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x24c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd04));
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        storeFloat(app, cpu, cpu.eax + 0x5c8);
        copy(0x5c8, 0x5d0);
        copy(0x5d0, 0x5d4);
        copy(0x5d4, 0x5cc);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x24c));
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x14);
        storeFloat(app, cpu, cpu.eax + 0x5c8);
        copy(0x5c8, 0x5d0);
        copy(0x5d0, 0x5d4);
        copy(0x5d4, 0x5cc);
    }
    leave(app, cpu);
}

/* 0x4aaed1: car eax's engine for the frame, and its force at the wheels
 * onto the x87's stack.  The revs held to the top (+0x50d, the clutch
 * slipping, past a third over).  The drag: the air's by the speed cubed
 * (past the handling's +0x1c8 harder) by the car's +0x58c (its driver's in
 * a single race); the revs aimed at: the top revs by the throttle, the idle
 * at least.  In neutral, changing gear or before the start the engine runs
 * free -- up 250 a frame towards the aim, by the gear asked's step through
 * a change, down (by the handling's kind) or held at the idle -- with no
 * force.  In gear (the automatic box's change first, automaticGears, or
 * gearLimit) the revs the wheels give and the torque at them (engineTorque,
 * by +0x56c), held in each gear; the limiter (+0x594) cutting the revs by
 * gear (kRevsDrop) past five sixths of the top in a low gear, or when the
 * wheels spin backwards; the engine braking short of the aim, the clutch
 * slipping (+0x514 counting the wheelspin) where the revs would be too low;
 * else the revs following the wheels' within 200 and the force by the
 * throttle and the slip.  Below the idle the idle's torque.  The force: the
 * torque, less the drag the way the car goes (half of it backwards before
 * the start). */
void engineForce(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x108);
    put(app, cpu.ebp - 0x38, cpu.eax);
    for (reg32 at = 0x30; at >= 0x14; at -= 4)
        put(app, cpu.ebp - at, 0);
    // A sixth of the top revs.
    cpu.edx = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.ecx = 6;
    cpu.eax = at(app, cpu.edx + 0x1c4);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx = reg32(i32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 0xc, 0);
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x528);
    put(app, cpu.ebp - 8, cpu.eax);
    // The revs held to the top.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.eax + 0x590);
    cpu.eax = at(app, cpu.ebp - 8);
    if (i32(cpu.edx) > i32(at(app, cpu.eax + 0x1c4)))
    {
        put(app, cpu.ebp - 0x24, 1);
        cpu.ecx = 3;
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax = at(app, cpu.eax + 0x1c4);
        cpu.edx = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.edx + 0x1c4);
        cpu.edx = reg32(i32(cpu.edx) >> 31);
        idiv(cpu, cpu.ecx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edx += at(app, cpu.eax + 0x1c4);
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (i32(cpu.edx) < i32(at(app, cpu.eax + 0x590)))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            putByte(app, cpu.eax + 0x50d, 1);
        }
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.eax + 0x1c4);
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.eax + 0x590, cpu.edx);
    }
    // The drag.
    if (i32(at(app, kPlayMode)) > 1 || at(app, kRaceType) == 1 || at(app, kRaceType) == 2)
    {
        put(app, cpu.ebp - 0x18, 0x3e7aedcb);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x220);
        fpu.push(1.0);
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x54));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53dd08));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd0c));
        storeFloat(app, cpu, cpu.ebp - 0x18);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    fpu.push(f32(app, cpu.eax + 0x58c));
    for (int i = 0; i < 3; ++i)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x4f4));
    }
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x18));
    storeFloat(app, cpu, cpu.ebp - 0x18);
    // The speed, either way, to [ebp - local].
    const auto speed = [&](reg32 local) {
        cpu.eax = at(app, cpu.ebp - 0x38);
        zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            fpu.push(-f32(app, cpu.eax + 0x4f4));
            storeFloat(app, cpu, cpu.ebp - local);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x4f4);
            put(app, cpu.ebp - local, cpu.eax);
        }
    };
    speed(0x3c);
    cpu.eax = at(app, cpu.ebp - 8);
    fpu.push(f32(app, cpu.ebp - 0x3c));
    fcomp(cpu, f32(app, cpu.eax + 0x1c8));
    if (above(cpu))
    {
        put(app, cpu.ebp - 0x40, 0);
        speed(0x44);
        cpu.eax = at(app, cpu.ebp - 8);
        fpu.push(f32(app, cpu.ebp - 0x44));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x1c8));
        storeFloat(app, cpu, cpu.ebp - 0x40);
        fpu.push(f32(app, cpu.ebp - 0x40));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x40));
        storeFloat(app, cpu, cpu.ebp - 0x40);
        fpu.push(f32(app, cpu.ebp - 0x40));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd10));
        storeFloat(app, cpu, cpu.ebp - 0x40);
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x40));
        storeFloat(app, cpu, cpu.ebp - 0x18);
    }
    // The revs aimed at: the top by the throttle, the idle at least.
    cpu.eax = at(app, cpu.ebp - 8);
    fpu.push(i32At(app, cpu.eax + 0x1c4));
    cpu.eax = at(app, cpu.ebp - 0x38);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x5f0));
    cpu.eax = at(app, cpu.ebp - 8);
    fpu.push(i32At(app, cpu.eax + 0x1c0));
    fcompp(cpu);
    if (below(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        fpu.push(i32At(app, cpu.eax + 0x1c4));
        cpu.eax = at(app, cpu.ebp - 0x38);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x5f0));
        storeFloat(app, cpu, cpu.ebp - 0x48);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 8);
        fpu.push(i32At(app, cpu.eax + 0x1c0));
        storeFloat(app, cpu, cpu.ebp - 0x48);
    }
    fpu.push(f32(app, cpu.ebp - 0x48));
    if (!byAddress(app, cpu, kChop))
        return;
    storeInteger(app, cpu, cpu.ebp - 0x28);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax + 0x1c4);
    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.ebp - 0x4c, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax = at(app, cpu.eax + 0x1c4);
        put(app, cpu.ebp - 0x4c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x4c);
    put(app, cpu.ebp - 0x28, cpu.eax);

    // [car + 0x590] to [ebp - local], at the idle at least.
    const auto atLeastIdle = [&](reg32 local, reg32 kept) {
        cpu.eax = at(app, cpu.ebp - local);
        cpu.edx = at(app, cpu.ebp - 8);
        if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x1c0)))
        {
            cpu.eax = at(app, cpu.ebp - local);
            put(app, cpu.ebp - kept, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.eax = at(app, cpu.eax + 0x1c0);
            put(app, cpu.ebp - kept, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - kept);
        cpu.edx = at(app, cpu.ebp - 0x38);
        put(app, cpu.edx + 0x590, cpu.eax);
    };
    // The torque at the gear's: [ebp - 0x1c], held in each gear -- 10 in
    // first, -6 at most in reverse (-6.0 by the double at `reverse`), 8 in
    // the others.
    const auto holdTorque = [&](reg32 reverse, reg32 first, reg32 back, reg32 others) {
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (byte(app, cpu.eax + 0x50b) == 2)
        {
            put(app, cpu.ebp - first,
                i32(at(app, cpu.ebp - 0x1c)) > i32(0x41200000) ? 0x41200000 : (cpu.eax = at(app, cpu.ebp - 0x1c)));
            cpu.eax = at(app, cpu.ebp - first);
            put(app, cpu.ebp - 0x1c, cpu.eax);
            return;
        }
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (byte(app, cpu.eax + 0x50b) == 0)
        {
            fpu.push(f32(app, cpu.ebp - 0x1c));
            fcomp(cpu, f64(app, reverse));
            if (above(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                put(app, cpu.ebp - back, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - back, 0xc0c00000);
            }
            cpu.eax = at(app, cpu.ebp - back);
            put(app, cpu.ebp - 0x1c, cpu.eax);
            return;
        }
        put(app, cpu.ebp - others,
            i32(at(app, cpu.ebp - 0x1c)) > i32(0x41000000) ? 0x41000000 : (cpu.eax = at(app, cpu.ebp - 0x1c)));
        cpu.eax = at(app, cpu.ebp - others);
        put(app, cpu.ebp - 0x1c, cpu.eax);
    };

    {
        bool free = false;
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (byte(app, cpu.eax + 0x50b) == 1)
            free = true;
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            free = i16(half(app, cpu.eax + 0x510)) > 0;
            if (!free)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                free = half(app, cpu.eax + 0x164) != 0;
            }
        }
        if (free)
        {
            // The engine running free: no force but the drag.
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (half(app, cpu.eax + 0x164) != 0)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                cpu.eax = at(app, cpu.eax + 0x1c);
                put(app, cpu.ebp - 0x28, at(app, cpu.ebp - 0x28) + cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x590);
            bool up = false;
            if (i32(cpu.eax) < i32(at(app, cpu.ebp - 0x28)))
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                up = half(app, cpu.eax + 0x510) == 0;
            }
            if (up)
            {
                // Up 250 towards the aim.
                cpu.eax = at(app, cpu.ebp - 0x38);
                put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) + 0xfa);
                cpu.eax = at(app, cpu.ebp - 0x38);
                cpu.eax = at(app, cpu.eax + 0x590);
                if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
                {
                    cpu.eax = at(app, cpu.ebp - 0x28);
                    put(app, cpu.ebp - 0x50, cpu.eax);
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 0x38);
                    cpu.eax = at(app, cpu.eax + 0x590);
                    put(app, cpu.ebp - 0x50, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - 0x50);
                put(app, cpu.ebp - 0xc, cpu.eax);
                atLeastIdle(0xc, 0x54);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                if (i16(half(app, cpu.eax + 0x510)) > 0)
                {
                    cpu.eax = at(app, cpu.ebp - 0x38);
                    if (byte(app, cpu.eax + 0x50c) != 0)
                    {
                        // Shifting down: the revs up by the gear asked's step, the clutch in.
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        zeroAgainst(cpu, f32(app, cpu.eax + 0x5f0));
                        if (below(cpu))
                        {
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            const reg32 table = byte(app, cpu.eax + 0x509) > 0x40 ? 0x34 : 0x14;
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            cpu.eax = byte(app, cpu.eax + 0x506);
                            cpu.eax <<= 2;
                            cpu.eax += at(app, cpu.ebp - 8);
                            cpu.edx = at(app, cpu.eax + table);
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) + cpu.edx);
                        }
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        cpu.edx = at(app, cpu.eax + 0x590);
                        cpu.eax = at(app, cpu.ebp - 8);
                        if (i32(cpu.edx) > i32(at(app, cpu.eax + 0x1c4)))
                        {
                            cpu.eax = at(app, cpu.ebp - 8);
                            cpu.eax = at(app, cpu.eax + 0x1c4);
                            put(app, cpu.ebp - 0x58, cpu.eax);
                        }
                        else
                        {
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            cpu.eax = at(app, cpu.eax + 0x590);
                            put(app, cpu.ebp - 0x58, cpu.eax);
                        }
                        cpu.eax = at(app, cpu.ebp - 0x58);
                        put(app, cpu.ebp - 0xc, cpu.eax);
                        atLeastIdle(0xc, 0x5c);
                    }
                    else
                    {
                        // Shifting up: the revs down by the handling's kind.
                        put(app, cpu.ebp - 0x60, 0);
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        cpu.eax = at(app, cpu.eax + 0x528);
                        if (at(app, cpu.eax + 0x29c) == 8)
                        {
                            put(app, cpu.ebp - 0x60, at(app, cpu.ebp - 0x60) - 0x19);
                        }
                        else
                        {
                            cpu.eax = at(app, cpu.ebp - 8);
                            if (i32(at(app, cpu.eax + 0x10)) > 4)
                                put(app, cpu.ebp - 0x60, at(app, cpu.ebp - 0x60) - 0x32);
                            else
                                put(app, cpu.ebp - 0x60, at(app, cpu.ebp - 0x60) - 0x4b);
                        }
                        if (at(app, cpu.ebp - 0x24) != 0)
                            put(app, cpu.ebp - 0x60, at(app, cpu.ebp - 0x60) << 1);
                        cpu.eax = at(app, cpu.ebp - 0x60);
                        cpu.edx = at(app, cpu.ebp - 0x38);
                        put(app, cpu.edx + 0x590, at(app, cpu.edx + 0x590) - cpu.eax);
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        cpu.edx = at(app, cpu.eax + 0x590);
                        cpu.eax = at(app, cpu.ebp - 8);
                        if (i32(cpu.edx) > i32(at(app, cpu.eax + 0x1c0)))
                        {
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            cpu.eax = at(app, cpu.eax + 0x590);
                            put(app, cpu.ebp - 0x64, cpu.eax);
                        }
                        else
                        {
                            cpu.eax = at(app, cpu.ebp - 8);
                            cpu.eax = at(app, cpu.eax + 0x1c0);
                            put(app, cpu.ebp - 0x64, cpu.eax);
                        }
                        cpu.eax = at(app, cpu.ebp - 0x64);
                        cpu.edx = at(app, cpu.ebp - 0x38);
                        put(app, cpu.edx + 0x590, cpu.eax);
                    }
                }
                else
                {
                    // Down 150 to the aim.
                    cpu.eax = at(app, cpu.ebp - 0x38);
                    cpu.eax = at(app, cpu.eax + 0x590);
                    if (i32(cpu.eax) >= i32(at(app, cpu.ebp - 0x28)))
                    {
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) - 0x96);
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        cpu.eax = at(app, cpu.eax + 0x590);
                        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
                        {
                            cpu.eax = at(app, cpu.ebp - 0x38);
                            cpu.eax = at(app, cpu.eax + 0x590);
                            put(app, cpu.ebp - 0x68, cpu.eax);
                        }
                        else
                        {
                            cpu.eax = at(app, cpu.ebp - 0x28);
                            put(app, cpu.ebp - 0x68, cpu.eax);
                        }
                        cpu.eax = at(app, cpu.ebp - 0x68);
                        put(app, cpu.ebp - 0xc, cpu.eax);
                        atLeastIdle(0xc, 0x6c);
                    }
                }
            }
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x598, 0);
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x594, 0);
            goto force;
        }
    }

    // In gear: the box first.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
    if (at(app, cpu.eax + kPlayerGearbox) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (!direct(app, cpu, automaticGears))
            return;
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (!direct(app, cpu, gearLimit))
            return;
    }
    // The revs the wheels give in the gear, either way.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.edx = byte(app, cpu.eax + 0x50b);
    cpu.edx <<= 2;
    cpu.edx += at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.ebp - 0x38);
    fpu.push(f32(app, cpu.eax + 0x4f4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
    integerToEax(app, cpu);
    put(app, cpu.ebp - 0x20, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x20);
    put(app, cpu.ebp - 0x70, cpu.eax);
    if (i32(at(app, cpu.ebp - 0x20)) < 0)
        put(app, cpu.ebp - 0x70, reg32(-i32(cpu.eax)));
    cpu.eax = at(app, cpu.ebp - 0x70);
    put(app, cpu.ebp - 0x20, cpu.eax);
    // The torque at the top revs, held to the top, or at the revs, by the gear's.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = byte(app, cpu.eax + 0x50b);
    cpu.eax <<= 2;
    cpu.ecx = at(app, cpu.ebp - 0x38);
    cpu.ecx += cpu.eax;
    if (at(app, cpu.ebp - 0x24) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.eax + 0x1c4);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.edx = at(app, cpu.eax + 0x590);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    if (!direct(app, cpu, engineTorque))
        return;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x56c));
    storeFloat(app, cpu, cpu.ebp - 0x1c);
    holdTorque(0x53dd14, 0x74, 0x78, 0x7c);
    // How far the wheels' revs are from the aim, and from the engine's.
    cpu.eax = at(app, cpu.ebp - 0x28);
    cpu.eax -= at(app, cpu.ebp - 0x20);
    put(app, cpu.ebp - 0x30, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x30);
    put(app, cpu.ebp - 0x80, cpu.eax);
    if (i32(at(app, cpu.ebp - 0x30)) < 0)
        put(app, cpu.ebp - 0x80, reg32(-i32(cpu.eax)));
    if (i32(at(app, cpu.ebp - 0x80)) < 0x7d)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax = at(app, cpu.eax + 0x1c4);
        cpu.eax -= 0x12c;
        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
            put(app, cpu.ebp - 0x30, 0);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.eax = at(app, cpu.eax + 0x590);
    cpu.eax -= at(app, cpu.ebp - 0x20);
    put(app, cpu.ebp - 0x2c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x38);
    put(app, cpu.eax + 0x594, 0);
    cpu.eax = at(app, cpu.ebp - 0x38);
    put(app, cpu.eax + 0x598, 0);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    {
        // The limiter: over the top's sixth in a low gear without a change
        // under way, or the wheels spinning backwards on the throttle.
        bool limit = false;
        bool other = true;
        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x10)))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (half(app, cpu.eax + 0x510) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                if (byte(app, cpu.eax + 0x50b) <= 4)
                {
                    limit = true;
                    other = false;
                }
            }
        }
        if (other)
        {
            bool reverse = true;
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (byte(app, cpu.eax + 0x50b) > 1)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                fpu.push(f32(app, cpu.eax + 0x5a4));
                fcomp(cpu, f64(app, 0x53dd1c));
                if (below(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0x38);
                    if (byte(app, cpu.eax + 0x508) > 8)
                    {
                        limit = true;
                        reverse = false;
                    }
                }
            }
            if (reverse)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                if (byte(app, cpu.eax + 0x50b) == 0)
                {
                    cpu.eax = at(app, cpu.ebp - 0x38);
                    if (i32(at(app, cpu.eax + 0x5a4)) > i32(0x3e99999a))
                    {
                        cpu.eax = at(app, cpu.ebp - 0x38);
                        limit = byte(app, cpu.eax + 0x508) > 8;
                    }
                }
            }
        }
        if (limit)
        {
            put(app, cpu.ebp - 0x84, 0);
            cpu.ecx = 8;
            cpu.edi = cpu.ebp - 0xa4;
            cpu.esi = kRevsDrop;
            for (; cpu.ecx != 0; --cpu.ecx)
            {
                put(app, cpu.edi, at(app, cpu.esi));
                cpu.edi += 4;
                cpu.esi += 4;
            }
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x594, 1);
            if (i32(at(app, cpu.ebp - 0x28)) < 0x7d0)
            {
                put(app, cpu.ebp - 0x84, 0xc8);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                cpu.eax = byte(app, cpu.eax + 0x50b);
                cpu.eax <<= 2;
                cpu.eax = at(app, cpu.eax + cpu.ebp - 0xa4);
                put(app, cpu.ebp - 0x84, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax += 0x590;
            put(app, cpu.ebp - 0xa8, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x2c);
            if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x84)))
            {
                cpu.eax = at(app, cpu.ebp - 0x84);
                put(app, cpu.ebp - 0xac, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x2c);
                put(app, cpu.ebp - 0xac, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0xac);
            cpu.edx = at(app, cpu.ebp - 0xa8);
            put(app, cpu.edx, at(app, cpu.edx) - cpu.eax);
            goto idle;
        }
    }
    if (i32(at(app, cpu.ebp - 0x30)) < 0)
    {
        // Over the aim: the engine braking, the revs down to the wheels' --
        // the clutch slipping where they would fall too low.
        put(app, cpu.ebp - 0xb0, 0);
        fpu.push(-f32(app, cpu.ebp - 0x1c));
        cpu.eax = at(app, cpu.ebp - 8);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x250));
        storeFloat(app, cpu, cpu.ebp - 0x1c);
        // [ebp - 0x1c] made positive (to [ebp - local]), and back negative with `negate`.
        const auto positive = [&](reg32 local, bool negate) {
            zeroAgainst(cpu, f32(app, cpu.ebp - 0x1c));
            if (above(cpu))
            {
                fpu.push(-f32(app, cpu.ebp - 0x1c));
                storeFloat(app, cpu, cpu.ebp - local);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                put(app, cpu.ebp - local, cpu.eax);
            }
            if (negate)
            {
                fpu.push(-f32(app, cpu.ebp - local));
                storeFloat(app, cpu, cpu.ebp - 0x1c);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - local);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
        };
        bool forward = false;
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (byte(app, cpu.eax + 0x50b) > 1)
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
            forward = above(cpu);
        }
        if (forward)
        {
            positive(0xb4, false);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (byte(app, cpu.eax + 0x50b) < 1)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
                if (below(cpu))
                    positive(0xb8, true);
            }
        }
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = byte(app, cpu.eax + 0x50b);
        cpu.eax <<= 2;
        cpu.eax += at(app, cpu.ebp - 0x38);
        fpu.push(f32(app, cpu.eax + 0x54c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd24));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd28));
        if (!byAddress(app, cpu, kChop))
            return;
        storeInteger(app, cpu, cpu.ebp - 0xb0);
        cpu.edx = at(app, cpu.ebp - 0x2c);
        cpu.edx = reg32(-i32(cpu.edx));
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax += 0x590;
        put(app, cpu.ebp - 0xbc, cpu.eax);
        if (i32(cpu.edx) > i32(at(app, cpu.ebp - 0xb0)))
        {
            cpu.eax = at(app, cpu.ebp - 0xb0);
            put(app, cpu.ebp - 0xc0, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x2c);
            put(app, cpu.ebp - 0xc0, reg32(-i32(cpu.eax)));
        }
        cpu.eax = at(app, cpu.ebp - 0xc0);
        cpu.edx = at(app, cpu.ebp - 0xbc);
        put(app, cpu.edx, at(app, cpu.edx) + cpu.eax);
        if (at(app, cpu.ebp - 0x24) == 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x590);
            if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                cpu.eax = at(app, cpu.eax + 0x590);
                put(app, cpu.ebp - 0xc4, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x28);
                put(app, cpu.ebp - 0xc4, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0xc4);
            put(app, cpu.ebp - 0xc, cpu.eax);
            atLeastIdle(0xc, 0xc8);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.edx = at(app, cpu.eax + 0x1c4);
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x590, cpu.edx);
        }
        // The top revs at the speed in the gear, by 1.15: the clutch slipping under them.
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.edx = byte(app, cpu.eax + 0x50b);
        cpu.edx <<= 2;
        cpu.edx += at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(i32At(app, cpu.eax + 0x1c4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x54c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd2c));
        storeFloat(app, cpu, cpu.ebp - 0xcc);
        speed(0xd0);
        fpu.push(f32(app, cpu.ebp - 0xd0));
        fcomp(cpu, f32(app, cpu.ebp - 0xcc));
        bool slipping = false;
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            slipping = byte(app, cpu.eax + 0x50b) > 1;
        }
        if (slipping)
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x594, 1);
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.edx = byte(app, cpu.eax + 0x50b);
            cpu.edx <<= 2;
            cpu.edx += at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(i32At(app, cpu.eax + 0x1c4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x54c));
            storeFloat(app, cpu, cpu.ebp - 0xd4);
            speed(0xd8);
            fpu.push(f32(app, cpu.ebp - 0xd4));
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0xd8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            storeFloat(app, cpu, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.ebp - 0x38);
            putByte(app, cpu.eax + 0x50d, 1);
            // The wheelspin counted, 0x180 at most: by the track's kind.
            reg32 step, local;
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (at(app, cpu.eax + 0x160) == 1)
            {
                step = 3;
                local = 0xdc;
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                if (at(app, cpu.eax + 0x160) == 2)
                {
                    step = 4;
                    local = 0xe0;
                }
                else
                {
                    step = 1;
                    local = 0xe4;
                }
            }
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x514);
            cpu.eax += step;
            if (i32(cpu.eax) < 0x180)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                cpu.eax = at(app, cpu.eax + 0x514);
                cpu.eax += step;
                put(app, cpu.ebp - local, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - local, 0x180);
            }
            cpu.eax = at(app, cpu.ebp - local);
            cpu.edx = at(app, cpu.ebp - 0x38);
            put(app, cpu.edx + 0x514, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            if (byte(app, cpu.eax + 0x50d) == 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                put(app, cpu.eax + 0x514, 0);
            }
        }
    }
    else if (at(app, cpu.ebp - 0x30) == 0)
    {
        // At the aim: the revs the wheels', the force the drag's.
        cpu.eax = at(app, cpu.ebp - 0x20);
        cpu.edx = at(app, cpu.ebp - 8);
        if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x1c0)))
        {
            cpu.eax = at(app, cpu.ebp - 0x20);
            put(app, cpu.ebp - 0xe8, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.eax = at(app, cpu.eax + 0x1c0);
            put(app, cpu.ebp - 0xe8, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0xe8);
        cpu.edx = at(app, cpu.ebp - 0x38);
        put(app, cpu.edx + 0x590, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x18);
        put(app, cpu.ebp - 0x1c, cpu.eax);
    }
    else
    {
        // Under the aim: the revs towards the wheels' by 200 a frame, the
        // force by the throttle and the slip (1.5 times at most).
        if (i32(at(app, cpu.ebp - 0x2c)) > 0xc8)
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) - 0xc8);
        }
        else if (i32(at(app, cpu.ebp - 0x2c)) < i32(0xfffffed4))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) + 0xc8);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x20);
            cpu.edx = at(app, cpu.ebp - 8);
            if (i32(cpu.eax) > i32(at(app, cpu.edx + 0x1c0)))
            {
                cpu.eax = at(app, cpu.ebp - 0x20);
                put(app, cpu.ebp - 0xec, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 8);
                cpu.eax = at(app, cpu.eax + 0x1c0);
                put(app, cpu.ebp - 0xec, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0xec);
            cpu.edx = at(app, cpu.ebp - 0x38);
            put(app, cpu.edx + 0x590, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.eax + 0x590);
        if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)))
        {
            cpu.eax = at(app, cpu.ebp - 0x28);
            put(app, cpu.ebp - 0xf0, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x590);
            put(app, cpu.ebp - 0xf0, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0xf0);
        put(app, cpu.ebp - 0xc, cpu.eax);
        atLeastIdle(0xc, 0xf4);
        cpu.eax = at(app, cpu.ebp - 0x38);
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x5f0));
        storeFloat(app, cpu, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 0x38);
        zeroAgainst(cpu, f32(app, cpu.eax + 0x5b0));
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            fpu.push(-f32(app, cpu.eax + 0x5b0));
            storeFloat(app, cpu, cpu.ebp - 0xf8);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x38);
            cpu.eax = at(app, cpu.eax + 0x5b0);
            put(app, cpu.ebp - 0xf8, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0xf8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd30));
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        storeFloat(app, cpu, cpu.ebp - 0x14);
        fpu.push(f32(app, 0x53dd34));
        fcomp(cpu, f32(app, cpu.ebp - 0x14));
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x14);
            put(app, cpu.ebp - 0xfc, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0xfc, 0x3fc00000);
        }
        cpu.eax = at(app, cpu.ebp - 0xfc);
        put(app, cpu.ebp - 0x14, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x14));
        storeFloat(app, cpu, cpu.ebp - 0x1c);
    }
idle:
    // Under the idle: the idle's torque.
    cpu.eax = at(app, cpu.ebp - 0x38);
    cpu.edx = at(app, cpu.eax + 0x590);
    cpu.eax = at(app, cpu.ebp - 8);
    if (i32(cpu.edx) < i32(at(app, cpu.eax + 0x1c0)))
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.eax + 0x594, 0);
        cpu.eax = at(app, cpu.ebp - 0x38);
        cpu.ecx = byte(app, cpu.eax + 0x50b);
        cpu.ecx <<= 2;
        cpu.ecx += at(app, cpu.ebp - 0x38);
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.eax + 0x1c0);
        cpu.eax = at(app, cpu.ebp - 0x38);
        if (!direct(app, cpu, engineTorque))
            return;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x56c));
        storeFloat(app, cpu, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.eax + 0x1c0);
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.eax + 0x590, cpu.edx);
        holdTorque(0x53dd3c, 0x100, 0x104, 0x108);
    }
force:
    // The force: the torque less the drag the way the car goes.
    cpu.eax = at(app, cpu.ebp - 0x38);
    if (half(app, cpu.eax + 0x164) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            put(app, cpu.ebp - 4, cpu.eax);
        }
        else
        {
            fpu.push(-f32(app, cpu.ebp - 0x18));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd44));
            storeFloat(app, cpu, cpu.ebp - 4);
        }
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        zeroAgainst(cpu, f32(app, cpu.eax + 0x4f4));
        if (above(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x18));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd48));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            storeFloat(app, cpu, cpu.ebp - 4);
        }
        else
        {
            fpu.push(f32(app, cpu.ebp - 0x1c));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x18));
            storeFloat(app, cpu, cpu.ebp - 4);
        }
    }
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.ebp - 0x34, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x34));
    leave(app, cpu);
}

}  // namespace drivetrain
}  // namespace nfs3hp
