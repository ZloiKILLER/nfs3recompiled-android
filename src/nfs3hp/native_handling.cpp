#include "native_guest.h"

#include <cstdint>

/* A car's handling in the race, as nfs3.exe's physics (0x4a78d6..0x4af3a1)
 * has it: its handling made for the race from the player's setup
 * (applyTuning, from the cars' loading), the automatic gearbox's choice
 * (automaticGear), the engine's revs matched to the car's speed and the
 * drive's state cleared (matchRevsToSpeed, resetDriveState), a car at rest
 * settled (settleAtRest), and an axle's tyres' forces (axleForces and what it
 * calls: tyreGrip, tyreSlide, the setup's and the steering's shares).
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (HANDLING_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
 *
 * The module was compiled without optimisation: every function keeps its
 * argument (the car, eax) in its frame and loads it again before each use,
 * and its x87 work goes through memory.  The car's record, as these
 * functions use it: +0x15c and +0x10c how fast it goes and how it is on the
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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack in cpu.fpu as the
 * code pushes and pops it (rounded as the race's control word says: x86::FPU's
 * add, sub, mul, div), and a compare's status word left in ax as fnstsw
 * leaves it (below and equal the flags sahf makes of it).  Every other
 * function of the game's is called by address -- the rest of the physics'
 * too; the game's constants (floats and doubles in its data, 0x53dc40 to
 * 0x53de40) are read where the code reads them. */
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
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

// The race's.
constexpr reg32 kPlayMode = 0x6fd3b0;       // 0 one player, 1 split screen, 2 and on over a link
constexpr reg32 kRaceType = 0x6fd3b8;       // 0 a single race, 1 and 2 the series, 3 Hot Pursuit
constexpr reg32 kLoadingFlags = 0x6fd4fc;   // 1 and 0x20: the handling's own (the simulation's)
constexpr reg32 kAutoBox = 0x6fd490;        // the race's option the setup's grip goes by
constexpr reg32 kAssist = 0x6fd4a4;         // the race's option the tyres' slip goes by
constexpr reg32 kPlayers = 0x6fd548;        // the players' records, 0x6c each: 1 at the start a human's
constexpr reg32 kFrame = 0x7d3684;          // the race's frame
// The setup the player's is held against: seven floats, the setup's middle.
constexpr reg32 kDefaultSetup = 0x55d024;
// The game's that these call: Watcom's chop and exp, the sine, atan2 and the
// square root of a stack float, a vector's dot product, a 2D turn by an
// angle; n vectors scaled, less others, added.
constexpr reg32 kChop = 0x4dfd56, kExp = 0x4f8116, kSine = 0x4ea930, kAtan2 = 0x4ea9c0, kSqrt = 0x4ea920;
constexpr reg32 kDot = 0x4e01f0, kRotate = 0x496bf0, kScale = 0x4e0050, kSubtract = 0x4e0000, kAdd = 0x4dffb0;

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
x86::Float f32(App* app, reg32 a) { return x86::Float(app->getMemory<float>(a)); }
x86::Float f64(App* app, reg32 a) { return x86::Float(app->getMemory<double>(a)); }

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

/* fcomp m: st(0) compared with m, popped; fcompp: with st(1), both popped.
 * The code's fnstsw ax and sahf follow: below and equal are the carry (C0)
 * and zero (C3) flags sahf makes of ah. */
void fcomp(CPU& cpu, const x86::Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.fpu.pop();
}

void fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.fpu.pop();
    cpu.fpu.pop();
}

bool below(const CPU& cpu) { return (cpu.ah & 0x01) != 0; }
bool equal(const CPU& cpu) { return (cpu.ah & 0x40) != 0; }

/* idiv: edx:eax divided by `by`, the quotient to eax and the remainder to edx. */
void idiv(CPU& cpu, reg32 by)
{
    const std::int64_t n = std::int64_t((std::uint64_t(cpu.edx) << 32) | cpu.eax);
    const std::int64_t d = i32(by);
    cpu.eax = reg32(n / d);
    cpu.edx = reg32(n % d);
}

}  // namespace

void logisticCurve(App* app, CPU& cpu);
void steeringGrip(App* app, CPU& cpu);
void setupSlipLimit(App* app, CPU& cpu);
void setupGripLoss(App* app, CPU& cpu);
void tyreGrip(App* app, CPU& cpu);
void tyreSlide(App* app, CPU& cpu);
void axleForces(App* app, CPU& cpu);
void applyTuning(App* app, CPU& cpu);
void automaticGear(App* app, CPU& cpu);
void matchRevsToSpeed(App* app, CPU& cpu);
void resetDriveState(App* app, CPU& cpu);
void settleAtRest(App* app, CPU& cpu);

/* 0x4ac994 (stdcall: x, ret 4): a logistic curve onto the x87's stack,
 * 1 - c / (1 + exp((x + a) * b)) with the doubles at 0x53ddf4, 0x53ddfc and
 * 0x53de04 (Watcom's exp, 0x4f8116), through a float; every register kept. */
void logisticCurve(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.eax);
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
        cpu.eax = pop(app, cpu);
        cpu.esp += 4 + 4;
    };
    fpu.push(f32(app, cpu.ebp + 0x20));
    fpu.st(0) = fp::add(fpu.st(0), f64(app, 0x53ddf4));
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53ddfc));
    cpu.esp -= 8;
    app->getMemory<double>(cpu.esp) = double(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, kExp))
        return;
    fpu.push(1.0);
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.st(0) = fp::div(f64(app, 0x53de04), fpu.st(0));
    fpu.push(1.0);
    fpu.st(1) = fp::sub(fpu.st(0), fpu.st(1));
    fpu.pop();
    app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 4));
    return leave();
}

/* 0x4ac9dd: car eax's steering's share of the front's grip onto the x87's
 * stack: 1 while the wheel has not been turned ([+0x514] 0 or less), else
 * logisticCurve of the time it has (a 0x180th) by 0.75 (0x53de0c). */
void steeringGrip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
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
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 8, 0x3f800000);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x10);
    if (i32(at(app, cpu.eax + 0x514)) > 0)
    {
        put(app, cpu.ebp - 4, 0x180);
        cpu.eax = at(app, cpu.ebp - 0x10);
        fpu.push(x86::Float(i32(at(app, cpu.eax + 0x514))));
        fpu.push(x86::Float(i32(at(app, cpu.ebp - 4))));
        fpu.st(1) = fp::div(fpu.st(1), fpu.st(0));
        fpu.pop();
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        fpu.push(f32(app, cpu.ebp - 8));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fpu.st(0));
        fpu.pop();
        if (!direct(app, cpu, logisticCurve))
            return;
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de0c));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.ebp - 0xc, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0xc));
    return leave();
}

/* 0x4ac027: the setup's slip limit, an integer to eax, of the setup's slider
 * at [eax]: 12 - 4 * it, chopped (0x4dfd56).  Over a link or in the series
 * the slider is put to 1 first. */
void setupSlipLimit(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
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
    put(app, cpu.ebp - 0xc, cpu.eax);
    put(app, cpu.ebp - 4, 0);
    if (i32(at(app, kPlayMode)) > 1 || at(app, kRaceType) == 1 || at(app, kRaceType) == 2)
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.eax, 0x3f800000);
    }
    cpu.eax = at(app, cpu.ebp - 0xc);
    fpu.push(f32(app, cpu.eax));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddb4));
    fpu.st(0) = fp::sub(f32(app, 0x53ddb8), fpu.st(0));
    app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 4));
    if (!byAddress(app, cpu, kChop))
        return;
    put(app, cpu.ebp - 8, reg32(fp::toInteger<i32>(fpu.st(0))));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 8);
    return leave();
}

/* 0x4abd9c: the grip lost to the setup's slider at [edx], onto the x87's
 * stack, by car eax's road (+0x160: 2, 1, else) and whether the simulation's
 * handling is on (kLoadingFlags 0x20): 1 / max((s + a) * b - c, floor), by 3
 * on two of the roads in the simulation's.  Over a link or in the series the
 * slider is put to 1 first. */
void setupGripLoss(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 0xc, cpu.edx);
    put(app, cpu.ebp - 4, 0);
    if (i32(at(app, kPlayMode)) > 1 || at(app, kRaceType) == 1 || at(app, kRaceType) == 2)
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.eax, 0x3f800000);
    }
    if ((byte(app, kLoadingFlags) & 0x20) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x10);
        if (at(app, cpu.eax + 0x160) == 2)
        {
            cpu.eax = at(app, cpu.ebp - 0xc);
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd4c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd50));
            fpu.st(0) = fp::sub(f32(app, 0x53dd54), fpu.st(0));
            fcomp(cpu, f32(app, 0x53dd58));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd4c));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd50));
                fpu.st(0) = fp::sub(f32(app, 0x53dd54), fpu.st(0));
                app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                put(app, cpu.ebp - 0x14, 0x40800000);
            }
            fpu.push(1.0);
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x14));
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
            fpu.push(f32(app, cpu.ebp - 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd5c));
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x10);
            if (at(app, cpu.eax + 0x160) == 1)
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd60));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd64));
                fpu.st(0) = fp::sub(f32(app, 0x53dd68), fpu.st(0));
                fcomp(cpu, f32(app, 0x53dd6c));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0xc);
                    fpu.push(f32(app, cpu.eax));
                    fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd60));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd64));
                    fpu.st(0) = fp::sub(f32(app, 0x53dd68), fpu.st(0));
                    app->getMemory<float>(cpu.ebp - 0x18) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    put(app, cpu.ebp - 0x18, 0x40c00000);
                }
                fpu.push(1.0);
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x18));
                app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
                fpu.pop();
                fpu.push(f32(app, cpu.ebp - 4));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd70));
                app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd74));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd78));
                fpu.st(0) = fp::sub(f32(app, 0x53dd7c), fpu.st(0));
                fcomp(cpu, f32(app, 0x53dd80));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0xc);
                    fpu.push(f32(app, cpu.eax));
                    fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd74));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd78));
                    fpu.st(0) = fp::sub(f32(app, 0x53dd7c), fpu.st(0));
                    app->getMemory<float>(cpu.ebp - 0x1c) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    put(app, cpu.ebp - 0x1c, 0x41800000);
                }
                fpu.push(1.0);
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x1c));
                app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
                fpu.pop();
            }
        }
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x10);
        if (at(app, cpu.eax + 0x160) == 2)
        {
            cpu.eax = at(app, cpu.ebp - 0xc);
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd84));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd88));
            fpu.st(0) = fp::sub(f32(app, 0x53dd8c), fpu.st(0));
            fcomp(cpu, f32(app, 0x53dd90));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd84));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd88));
                fpu.st(0) = fp::sub(f32(app, 0x53dd8c), fpu.st(0));
                app->getMemory<float>(cpu.ebp - 0x20) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                put(app, cpu.ebp - 0x20, 0x40800000);
            }
            fpu.push(1.0);
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x20));
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x10);
            if (at(app, cpu.eax + 0x160) == 1)
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd94));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd98));
                fpu.st(0) = fp::sub(f32(app, 0x53dd9c), fpu.st(0));
                fcomp(cpu, f32(app, 0x53dda0));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0xc);
                    fpu.push(f32(app, cpu.eax));
                    fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dd94));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dd98));
                    fpu.st(0) = fp::sub(f32(app, 0x53dd9c), fpu.st(0));
                    app->getMemory<float>(cpu.ebp - 0x24) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    put(app, cpu.ebp - 0x24, 0x40c00000);
                }
                fpu.push(1.0);
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x24));
                app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dda4));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dda8));
                fpu.st(0) = fp::sub(f32(app, 0x53ddac), fpu.st(0));
                fcomp(cpu, f32(app, 0x53ddb0));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0xc);
                    fpu.push(f32(app, cpu.eax));
                    fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dda4));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dda8));
                    fpu.st(0) = fp::sub(f32(app, 0x53ddac), fpu.st(0));
                    app->getMemory<float>(cpu.ebp - 0x28) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    put(app, cpu.ebp - 0x28, 0x41800000);
                }
                fpu.push(1.0);
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x28));
                app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
                fpu.pop();
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.ebp - 8, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 8));
    return leave();
}

/* 0x4ac094: axle edx's tyres gripping (no handbrake on it): the wheels' turns
 * counted (+0x59c, by 1 for the front, 2 the back), the slip (+0x2c, from the
 * car's +0x5c0 front, +0x5c4 back) pulled towards how far the larger of the
 * speeds across and along (with a quarter of the other) is past the grip
 * (+4 by +8) -- by the setup's slider, 0.5 at least, 0.7 over a link or in the
 * series -- or 0, and held to setupSlipLimit; the force across (+0x1c) the
 * speed across by the slip's share (0.75 past the grip lost) by steeringGrip,
 * the force along (+0x24) the drive by it, both a 20th in neutral; the slip
 * kept back in the car. */
void tyreGrip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x4c;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp - 0x28, cpu.eax);
    put(app, cpu.ebp - 0x24, cpu.edx);
    put(app, cpu.ebp - 0x1c, 0);
    put(app, cpu.ebp - 0x18, 0);
    put(app, cpu.ebp - 0x14, 0);
    put(app, cpu.ebp - 0x10, 0);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x24);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.eax + 0x59c, at(app, cpu.eax + 0x59c) + 1);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.eax + 0x59c, at(app, cpu.eax + 0x59c) + 2);
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        fpu.push(f32(app, cpu.eax + 0x5c0));
        cpu.eax = at(app, cpu.ebp - 0x24);
        app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        fpu.push(f32(app, cpu.eax + 0x5c4));
        cpu.eax = at(app, cpu.ebp - 0x24);
        app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(f32(app, cpu.eax + 4));
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 8));
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x10));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x10));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x34) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        cpu.eax = at(app, cpu.eax + 0x10);
        put(app, cpu.ebp - 0x34, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x34);
    put(app, cpu.ebp - 0x18, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x18));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x18));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x30) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        cpu.eax = at(app, cpu.eax + 0x18);
        put(app, cpu.ebp - 0x30, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x30);
    put(app, cpu.ebp - 0x14, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x18));
    fcomp(cpu, f32(app, cpu.ebp - 0x14));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x14));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddbc));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x18));
        app->getMemory<float>(cpu.ebp - 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddbc));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x14));
        app->getMemory<float>(cpu.ebp - 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x2c);
    put(app, cpu.ebp - 0x1c, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x1c));
    fcomp(cpu, f32(app, cpu.ebp - 0x10));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        if (i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
        {
            cpu.eax = at(app, cpu.ebp - 0x28);
            cpu.eax = at(app, cpu.eax + 0x220);
            fpu.push(f32(app, cpu.eax + 0x58));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddc0));
            fpu.st(0) = fp::sub(f32(app, 0x53ddc4), fpu.st(0));
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            put(app, cpu.ebp - 4, 0x3f333333);
        }
        if (i32(at(app, cpu.ebp - 4)) > 0x3f000000)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            put(app, cpu.ebp - 0x38, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0x38, 0x3f000000);
        }
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x2c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 4));
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 4));
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        put(app, cpu.eax + 0x2c, 0);
    }
    cpu.eax = at(app, cpu.ebp - 0x28);
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.eax += 0x58;
    if (!direct(app, cpu, setupSlipLimit))
        return;
    put(app, cpu.ebp - 0x48, cpu.eax);
    fpu.push(x86::Float(i32(at(app, cpu.ebp - 0x48))));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(f32(app, cpu.eax + 0x2c));
    fcomp(cpu, f32(app, cpu.ebp - 0xc));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 0x44, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        cpu.eax = at(app, cpu.eax + 0x2c);
        put(app, cpu.ebp - 0x44, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x44);
    cpu.edx = at(app, cpu.ebp - 0x24);
    put(app, cpu.edx + 0x2c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x28);
    cpu.edx = at(app, cpu.eax + 0x220);
    cpu.edx += 0x58;
    cpu.eax = at(app, cpu.ebp - 0x28);
    if (!direct(app, cpu, setupGripLoss))
        return;
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x10), fpu.st(0));
    app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
    fpu.pop();
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ebp - 0x1c));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x40) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.ebp - 0x40, cpu.eax);
    }
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ebp - 0x14));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x14));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x14);
        put(app, cpu.ebp - 0x3c, cpu.eax);
    }
    fpu.push(f32(app, cpu.ebp - 0x40));
    fcomp(cpu, f32(app, cpu.ebp - 0x3c));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x14));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x1c));
        fpu.push(0.0);
        fcompp(cpu);
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x14));
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x4c) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            fpu.push(f32(app, cpu.ebp - 0x14));
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            app->getMemory<float>(cpu.ebp - 0x4c) = float(fpu.st(0));
            fpu.pop();
        }
        cpu.eax = at(app, cpu.ebp - 0x4c);
        put(app, cpu.ebp - 0x18, cpu.eax);
    }
    else
    {
        put(app, cpu.ebp - 0x18, 0x3f400000);
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(f32(app, cpu.eax + 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x18));
    app->getMemory<float>(cpu.ebp - 0x48) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x28);
    if (!direct(app, cpu, steeringGrip))
        return;
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x48));
    app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x24);
    put(app, cpu.eax + 0x20, 0);
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(f32(app, cpu.eax));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x18));
    cpu.eax = at(app, cpu.ebp - 0x24);
    app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x28);
    if (byte(app, cpu.eax + 0x50b) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x1c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddc8));
        app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x24));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddc8));
        app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x24);
        fpu.push(f32(app, cpu.eax + 0x2c));
        cpu.eax = at(app, cpu.ebp - 0x28);
        app->getMemory<float>(cpu.eax + 0x5c0) = float(fpu.st(0));
        fpu.pop();
        return leave();
    }
    cpu.eax = at(app, cpu.ebp - 0x24);
    fpu.push(f32(app, cpu.eax + 0x2c));
    cpu.eax = at(app, cpu.ebp - 0x28);
    app->getMemory<float>(cpu.eax + 0x5c4) = float(fpu.st(0));
    fpu.pop();
    return leave();
}

/* 0x4ac3af: axle edx's tyres past their grip: the forces' larger (with a
 * quarter of the other) against the grip less what the setup loses of it.
 * Past it, the wheels spin or lock: with the assist (kAssist) on the back
 * axle and the throttle past 0.15, the slip every fourth frame and the forces
 * cut to the grip; else the slip from how far past, by the road's share (1.5,
 * 2, or as it is), pulled in by the setup's slider, and the forces cut --
 * along only with the assist, else by half again -- when on the back, in a
 * gear, braking hard past 0.85 with the revs past 0.85 of the limit.  The
 * back axle with the clutch in (+0x594 1) in first or second slips at the
 * forces' size, by 4.  The slip held to setupSlipLimit. */
void tyreSlide(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x80;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp - 0x50, cpu.eax);
    put(app, cpu.ebp - 0x4c, cpu.edx);
    put(app, cpu.ebp - 0x44, 0);
    put(app, cpu.ebp - 0x40, 0);
    put(app, cpu.ebp - 0x3c, 0);
    put(app, cpu.ebp - 0x38, 0);
    put(app, cpu.ebp - 0x34, 0);
    put(app, cpu.ebp - 0x30, 0);
    put(app, cpu.ebp - 0x2c, 0);
    put(app, cpu.ebp - 0x28, 0);
    cpu.eax = at(app, cpu.ebp - 0x4c);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x1c));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(f32(app, cpu.eax + 0x1c));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x24) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        put(app, cpu.ebp - 0x24, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x4c);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x24));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(f32(app, cpu.eax + 0x24));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x20) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        cpu.eax = at(app, cpu.eax + 0x24);
        put(app, cpu.ebp - 0x20, cpu.eax);
    }
    fpu.push(f32(app, cpu.ebp - 0x24));
    fcomp(cpu, f32(app, cpu.ebp - 0x20));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x24));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x24));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x18) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            cpu.eax = at(app, cpu.eax + 0x24);
            put(app, cpu.ebp - 0x18, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddcc));
        app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x1c));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x1c));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            cpu.eax = at(app, cpu.eax + 0x1c);
            put(app, cpu.ebp - 0x10, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x14));
        app->getMemory<float>(cpu.ebp - 0x1c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x1c));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x1c));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            cpu.eax = at(app, cpu.eax + 0x1c);
            put(app, cpu.ebp - 0xc, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddcc));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x4c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x24));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x24));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            cpu.eax = at(app, cpu.eax + 0x24);
            put(app, cpu.ebp - 4, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 4));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 8));
        app->getMemory<float>(cpu.ebp - 0x1c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    put(app, cpu.ebp - 0x44, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x4c);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x50);
        fpu.push(f32(app, cpu.eax + 0x5c0));
        cpu.eax = at(app, cpu.ebp - 0x4c);
        app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x50);
        fpu.push(f32(app, cpu.eax + 0x5c4));
        cpu.eax = at(app, cpu.ebp - 0x4c);
        app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x4c);
    cpu.eax = at(app, cpu.eax + 4);
    put(app, cpu.ebp - 0x40, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x50);
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.edx = cpu.eax + 0x58;
    cpu.eax = at(app, cpu.ebp - 0x50);
    if (!direct(app, cpu, setupGripLoss))
        return;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x40));
    fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x40), fpu.st(0));
    app->getMemory<float>(cpu.ebp - 0x40) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x50);
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.edx = cpu.eax + 0x58;
    cpu.eax = at(app, cpu.ebp - 0x50);
    if (!direct(app, cpu, setupGripLoss))
        return;
    app->getMemory<float>(cpu.ebp - 0x38) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x4c);
    put(app, cpu.eax + 0x2c, 0);
    fpu.push(f32(app, cpu.ebp - 0x44));
    fcomp(cpu, f32(app, cpu.ebp - 0x40));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        if (at(app, kAssist) != 0 && (cpu.eax = at(app, cpu.ebp - 0x4c), at(app, cpu.eax + 0x28) == 0))
        {
            cpu.eax = at(app, cpu.ebp - 0x50);
            cpu.eax = at(app, cpu.ebp - 0x50);
            if (i32(at(app, cpu.eax + 0x5b0)) > 0x3e19999a)
            {
                put(app, cpu.ebp - 0x54, 1);
            }
            else
            {
                put(app, cpu.ebp - 0x54, 0);
            }
            cpu.eax = at(app, cpu.ebp - 0x54);
            put(app, cpu.ebp - 0x5c, cpu.eax);
            if (at(app, cpu.ebp - 0x5c) == 0)
                goto L_4ac67d;
            if ((at(app, cpu.ebp - 0x44) & 0x7fffffff) != 0)
            {
                fpu.push(f32(app, cpu.ebp - 0x40));
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x44));
                app->getMemory<float>(cpu.ebp - 0x34) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                put(app, cpu.ebp - 0x34, 0x3f800000);
            }
            cpu.ecx = 4;
            cpu.eax = at(app, kFrame);
            cpu.edx = at(app, kFrame);
            cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
            idiv(cpu, cpu.ecx);
            if (cpu.edx == 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x4c);
                put(app, cpu.eax + 0x2c, 0);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x4c);
                fpu.push(f32(app, cpu.ebp - 0x40));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddd0));
                app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0x50);
            if (byte(app, cpu.eax + 0x50b) > 2)
            {
                cpu.eax = at(app, cpu.ebp - 0x4c);
                fpu.push(f32(app, cpu.eax + 0x1c));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x34));
                app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x24));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x34));
            app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
L_4ac67d:
            cpu.eax = at(app, cpu.ebp - 0x50);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax + 0x4f4));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x50);
                fpu.push(f32(app, cpu.eax + 0x4f4));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x60) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x50);
                cpu.eax = at(app, cpu.eax + 0x4f4);
                put(app, cpu.ebp - 0x60, cpu.eax);
            }
            if (i32(at(app, cpu.ebp - 0x60)) > 0x40a00000 || (cpu.eax = at(app, cpu.ebp - 0x50), at(app, cpu.eax + 0x594) != 0))
            {
                cpu.eax = at(app, cpu.ebp - 0x4c);
                fpu.push(f32(app, cpu.ebp - 0x44));
                fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0x50);
            if (at(app, cpu.eax + 0x160) == 1)
            {
                fpu.push(f32(app, cpu.ebp - 0x44));
                fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                fpu.push(f32(app, cpu.ebp - 0x40));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                fcompp(cpu);
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    fpu.push(f32(app, cpu.ebp - 0x44));
                    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    app->getMemory<float>(cpu.ebp - 0x64) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    fpu.push(f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    app->getMemory<float>(cpu.ebp - 0x64) = float(fpu.st(0));
                    fpu.pop();
                }
                fpu.push(f32(app, cpu.ebp - 0x64));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddd4));
                app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x50);
                if (at(app, cpu.eax + 0x160) == 2)
                {
                    fpu.push(f32(app, cpu.ebp - 0x44));
                    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    fpu.push(f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    fcompp(cpu);
                    cpu.ax = fpu.status.word;
                    if (!below(cpu) && !equal(cpu))
                    {
                        fpu.push(f32(app, cpu.ebp - 0x44));
                        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                        app->getMemory<float>(cpu.ebp - 0x68) = float(fpu.st(0));
                        fpu.pop();
                    }
                    else
                    {
                        fpu.push(f32(app, cpu.ebp - 0x40));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                        app->getMemory<float>(cpu.ebp - 0x68) = float(fpu.st(0));
                        fpu.pop();
                    }
                    fpu.push(f32(app, cpu.ebp - 0x68));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddd8));
                    app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    fpu.push(f32(app, cpu.ebp - 0x44));
                    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    fpu.push(f32(app, cpu.ebp - 0x40));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                    fcompp(cpu);
                    cpu.ax = fpu.status.word;
                    if (!below(cpu) && !equal(cpu))
                    {
                        fpu.push(f32(app, cpu.ebp - 0x44));
                        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                        app->getMemory<float>(cpu.ebp - 0x6c) = float(fpu.st(0));
                        fpu.pop();
                    }
                    else
                    {
                        fpu.push(f32(app, cpu.ebp - 0x40));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
                        app->getMemory<float>(cpu.ebp - 0x6c) = float(fpu.st(0));
                        fpu.pop();
                    }
                    cpu.eax = at(app, cpu.ebp - 0x6c);
                    put(app, cpu.ebp - 0x3c, cpu.eax);
                }
            }
            fpu.push(f32(app, cpu.ebp - 0x40));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x3c));
            app->getMemory<float>(cpu.ebp - 0x2c) = float(fpu.st(0));
            fpu.pop();
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.ebp - 0x44));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                fpu.push(f32(app, cpu.ebp - 0x44));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x70) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x44);
                put(app, cpu.ebp - 0x70, cpu.eax);
            }
            fpu.push(f32(app, cpu.ebp - 0x70));
            fcomp(cpu, f32(app, cpu.ebp - 0x2c));
            cpu.ax = fpu.status.word;
            if ((!below(cpu) && !equal(cpu)) && (at(app, cpu.ebp - 0x44) & 0x7fffffff) != 0)
            {
                fpu.push(0.0);
                fcomp(cpu, f32(app, cpu.ebp - 0x44));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    fpu.push(f32(app, cpu.ebp - 0x44));
                    fpu.st(0) = -fpu.st(0);
                    app->getMemory<float>(cpu.ebp - 0x74) = float(fpu.st(0));
                    fpu.pop();
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 0x44);
                    put(app, cpu.ebp - 0x74, cpu.eax);
                }
                fpu.push(f32(app, cpu.ebp - 0x2c));
                fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x74));
                app->getMemory<float>(cpu.ebp - 0x34) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                put(app, cpu.ebp - 0x34, 0x3f800000);
            }
            if (i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
            {
                cpu.eax = at(app, cpu.ebp - 0x50);
                cpu.eax = at(app, cpu.eax + 0x220);
                fpu.push(f32(app, cpu.eax + 0x58));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dddc));
                fpu.st(0) = fp::sub(f32(app, 0x53dde0), fpu.st(0));
                app->getMemory<float>(cpu.ebp - 0x28) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                put(app, cpu.ebp - 0x28, 0x3f333333);
            }
            if (i32(at(app, cpu.ebp - 0x28)) > 0x3f000000)
            {
                cpu.eax = at(app, cpu.ebp - 0x28);
                put(app, cpu.ebp - 0x78, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - 0x78, 0x3f000000);
            }
            cpu.eax = at(app, cpu.ebp - 0x78);
            put(app, cpu.ebp - 0x28, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x2c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x28));
            fpu.push(f32(app, cpu.ebp - 0x44));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x40));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x28));
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x4c);
            if (at(app, cpu.eax + 0x28) == 0 && (cpu.eax = at(app, cpu.ebp - 0x50), byte(app, cpu.eax + 0x50b) == 2) && (cpu.eax = at(app, cpu.ebp - 0x50), i32(at(app, cpu.eax + 0x20c)) >= 0x3f733333) && (cpu.eax = at(app, cpu.ebp - 0x50), i32(at(app, cpu.eax + 0x5f0)) >= 0x3f59999a))
            {
                cpu.eax = at(app, cpu.ebp - 0x50);
                cpu.eax = at(app, cpu.eax + 0x528);
                fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c4))));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dde4));
                cpu.eax = at(app, cpu.ebp - 0x50);
                fpu.push(x86::Float(i32(at(app, cpu.eax + 0x590))));
                fcompp(cpu);
                cpu.ax = fpu.status.word;
                if (!below(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0x4c);
                    fpu.push(f32(app, cpu.eax + 0x1c));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x34));
                    app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
                    fpu.pop();
                    if (at(app, kAssist) != 0)
                    {
                        cpu.eax = at(app, cpu.ebp - 0x4c);
                        fpu.push(f32(app, cpu.eax + 0x24));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x34));
                        app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
                        fpu.pop();
                    }
                    else
                    {
                        fpu.push(f32(app, cpu.ebp - 0x34));
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dde8));
                        cpu.eax = at(app, cpu.ebp - 0x4c);
                        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x24));
                        app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
                        fpu.pop();
                    }
                }
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x4c);
    if (at(app, cpu.eax + 0x28) == 0 && (cpu.eax = at(app, cpu.ebp - 0x50), at(app, cpu.eax + 0x594) == 1))
    {
        cpu.eax = at(app, cpu.ebp - 0x44);
        cpu.edx = at(app, cpu.ebp - 0x4c);
        put(app, cpu.edx + 0x2c, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x50);
        if (byte(app, cpu.eax + 0x50b) >= 2 && (cpu.eax = at(app, cpu.ebp - 0x50), byte(app, cpu.eax + 0x50b) <= 3))
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            fpu.push(f32(app, cpu.eax + 0x2c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53ddec));
            app->getMemory<float>(cpu.eax + 0x2c) = float(fpu.st(0));
            fpu.pop();
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x50);
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.eax += 0x58;
    if (!direct(app, cpu, setupSlipLimit))
        return;
    put(app, cpu.ebp - 0x80, cpu.eax);
    fpu.push(x86::Float(i32(at(app, cpu.ebp - 0x80))));
    app->getMemory<float>(cpu.ebp - 0x30) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x4c);
    fpu.push(f32(app, cpu.eax + 0x2c));
    fcomp(cpu, f32(app, cpu.ebp - 0x30));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x30);
        put(app, cpu.ebp - 0x7c, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x4c);
        cpu.eax = at(app, cpu.eax + 0x2c);
        put(app, cpu.ebp - 0x7c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x7c);
    cpu.edx = at(app, cpu.ebp - 0x4c);
    put(app, cpu.edx + 0x2c, cpu.eax);
    return leave();
}

/* 0x4aca4f: axle edx's tyres' forces for car eax.  Its grip less what the
 * setup loses; its speeds turned into the wheels' way when they are turned
 * (0x496bf0); the drive held back where the wheels would turn the car the
 * other way braking (and the mark of it); the drive by the handling's
 * +0x248; the wheel's time turned counted to 0x180 while the handbrake is on.
 * Without the handbrake, or on the front, gripping (tyreGrip) -- also when
 * the drive is past the grip after the mark.  Else sliding: the force across
 * from the slip angle (atan2, 0x4ea9c0) through a sine (0x4ea930) by the
 * grip -- in neutral and in gear each its own -- held to the handling's
 * +0x23c by 2, a quarter at full lock (the steering past 0x7f) slow or when
 * hardly braking, held to the speed across, 0 on the front stopped with the
 * handbrake, by 0.8 or 1.4; then past the grip (tyreSlide), the forces turned
 * back (0x496bf0), the slip kept in the car. */
void axleForces(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x78;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp - 0x1c, cpu.eax);
    put(app, cpu.ebp - 0x18, cpu.edx);
    put(app, cpu.ebp - 0x10, 0);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x18);
    put(app, cpu.eax + 0x2c, 0);
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax = at(app, cpu.eax + 4);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.edx = at(app, cpu.eax + 0x220);
    cpu.edx += 0x58;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (!direct(app, cpu, setupGripLoss))
        return;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x10), fpu.st(0));
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x18);
    if ((at(app, cpu.eax + 0xc) & 0x7fffffff) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.eax += 0x18;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.eax += 0x10;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0xc));
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0x18));
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0x10));
        if (!byAddress(app, cpu, kRotate))
            return;
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x18));
        cpu.ax = fpu.status.word;
        if (below(cpu) || equal(cpu))
            goto L_4acb65;
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (i32(at(app, cpu.eax + 0x5f0)) <= 0x3e800000 || (cpu.eax = at(app, cpu.ebp - 0x1c), byte(app, cpu.eax + 0x50b) != 0))
        {
            put(app, cpu.ebp - 0x20, 1);
        }
        else
        {
            put(app, cpu.ebp - 0x20, 0);
        }
        if (at(app, cpu.ebp - 0x20) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.eax));
            cpu.eax = at(app, cpu.ebp - 0x18);
            fcomp(cpu, f32(app, cpu.eax + 0x18));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax);
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax + 0x18);
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x24);
            cpu.edx = at(app, cpu.ebp - 0x18);
            put(app, cpu.edx, cpu.eax);
            put(app, cpu.ebp - 4, 1);
        }
    }
    else
    {
L_4acb65:
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax));
        cpu.ax = fpu.status.word;
        if (below(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax + 0x18));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                if (i32(at(app, cpu.eax + 0x5f0)) <= 0x3e800000 || (cpu.eax = at(app, cpu.ebp - 0x1c), byte(app, cpu.eax + 0x50b) <= 1))
                {
                    put(app, cpu.ebp - 0x28, 1);
                }
                else
                {
                    put(app, cpu.ebp - 0x28, 0);
                }
                if (at(app, cpu.ebp - 0x28) != 0)
                {
                    cpu.eax = at(app, cpu.ebp - 0x18);
                    fpu.push(f32(app, cpu.eax));
                    cpu.eax = at(app, cpu.ebp - 0x18);
                    fcomp(cpu, f32(app, cpu.eax + 0x18));
                    cpu.ax = fpu.status.word;
                    if (!below(cpu) && !equal(cpu))
                    {
                        cpu.eax = at(app, cpu.ebp - 0x18);
                        cpu.eax = at(app, cpu.eax + 0x18);
                        put(app, cpu.ebp - 0x2c, cpu.eax);
                    }
                    else
                    {
                        cpu.eax = at(app, cpu.ebp - 0x18);
                        cpu.eax = at(app, cpu.eax);
                        put(app, cpu.ebp - 0x2c, cpu.eax);
                    }
                    cpu.eax = at(app, cpu.ebp - 0x2c);
                    cpu.edx = at(app, cpu.ebp - 0x18);
                    put(app, cpu.edx, cpu.eax);
                    put(app, cpu.ebp - 4, 1);
                }
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x248));
    cpu.eax = at(app, cpu.ebp - 0x18);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax));
    app->getMemory<float>(cpu.eax) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (byte(app, cpu.eax + 0x50d) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (at(app, cpu.eax + 0x160) == 1)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x514);
            cpu.eax += 3;
            if (i32(cpu.eax) < 0x180)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x514);
                cpu.eax += 3;
                put(app, cpu.ebp - 0x30, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - 0x30, 0x180);
            }
            cpu.eax = at(app, cpu.ebp - 0x30);
            cpu.edx = at(app, cpu.ebp - 0x1c);
            put(app, cpu.edx + 0x514, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            if (at(app, cpu.eax + 0x160) == 2)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x514);
                cpu.eax += 4;
                if (i32(cpu.eax) < 0x180)
                {
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    cpu.eax = at(app, cpu.eax + 0x514);
                    cpu.eax += 4;
                    put(app, cpu.ebp - 0x34, cpu.eax);
                }
                else
                {
                    put(app, cpu.ebp - 0x34, 0x180);
                }
                cpu.eax = at(app, cpu.ebp - 0x34);
                cpu.edx = at(app, cpu.ebp - 0x1c);
                put(app, cpu.edx + 0x514, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x514);
                ++cpu.eax;
                if (i32(cpu.eax) < 0x180)
                {
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    cpu.eax = at(app, cpu.eax + 0x514);
                    ++cpu.eax;
                    put(app, cpu.ebp - 0x38, cpu.eax);
                }
                else
                {
                    put(app, cpu.ebp - 0x38, 0x180);
                }
                cpu.eax = at(app, cpu.ebp - 0x38);
                cpu.edx = at(app, cpu.ebp - 0x1c);
                put(app, cpu.edx + 0x514, cpu.eax);
            }
        }
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.eax + 0x514, 0);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (byte(app, cpu.eax + 0x50d) == 0 || (cpu.eax = at(app, cpu.ebp - 0x18), at(app, cpu.eax + 0x28) != 0))
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (byte(app, cpu.eax + 0x50d) != 0 && (cpu.eax = at(app, cpu.ebp - 0x18), at(app, cpu.eax + 0x28) != 0) && at(app, cpu.ebp - 4) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                fpu.push(f32(app, cpu.eax));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax);
                put(app, cpu.ebp - 0x3c, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.ebp - 0x3c));
            fcomp(cpu, f32(app, cpu.eax + 4));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
                goto L_4acd3c;
        }
    }
    else
    {
L_4acd3c:
        cpu.edx = at(app, cpu.ebp - 0x18);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (!direct(app, cpu, tyreGrip))
            return;
        return leave();
    }
    if (at(app, cpu.ebp - 4) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x40) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            cpu.eax = at(app, cpu.eax);
            put(app, cpu.ebp - 0x40, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(f32(app, cpu.ebp - 0x40));
        fcomp(cpu, f32(app, cpu.eax + 4));
        cpu.ax = fpu.status.word;
        if ((!below(cpu) && !equal(cpu)) && (cpu.eax = at(app, cpu.ebp - 0x1c), byte(app, cpu.eax + 0x50d) == 0) && (cpu.eax = at(app, cpu.ebp - 0x1c), cpu.eax = at(app, cpu.eax + 0x528), byte(app, cpu.eax + 0x1d4) != 0))
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.eax));
            cpu.eax = at(app, cpu.ebp - 0x18);
            fcomp(cpu, f32(app, cpu.eax + 4));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                fpu.push(f32(app, cpu.eax + 4));
                cpu.eax = at(app, cpu.ebp - 0x18);
                app->getMemory<float>(cpu.eax) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                fpu.push(f32(app, cpu.eax + 4));
                fpu.st(0) = -fpu.st(0);
                cpu.eax = at(app, cpu.ebp - 0x18);
                fcomp(cpu, f32(app, cpu.eax));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0x18);
                    fpu.push(f32(app, cpu.eax + 4));
                    cpu.eax = at(app, cpu.ebp - 0x18);
                    fpu.st(0) = -fpu.st(0);
                    app->getMemory<float>(cpu.eax) = float(fpu.st(0));
                    fpu.pop();
                }
            }
            cpu.ecx = 4;
            cpu.eax = at(app, kFrame);
            cpu.edx = at(app, kFrame);
            cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
            idiv(cpu, cpu.ecx);
            if (cpu.edx == 0)
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                put(app, cpu.eax + 0x2c, 0x40400000);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                put(app, cpu.eax + 0x2c, 0);
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x18));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(f32(app, cpu.eax + 0x18));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x44) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.eax = at(app, cpu.eax + 0x18);
        put(app, cpu.ebp - 0x44, cpu.eax);
    }
    push(app, cpu, at(app, cpu.ebp - 0x44));
    cpu.eax = at(app, cpu.ebp - 0x18);
    push(app, cpu, at(app, cpu.eax + 0x10));
    if (!byAddress(app, cpu, kAtan2))
        return;
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (byte(app, cpu.eax + 0x50b) == 0)
    {
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de10));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fpu.st(0));
        fpu.pop();
        if (!byAddress(app, cpu, kSine))
            return;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de14));
        fpu.push(0.0);
        fcompp(cpu);
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de10));
            cpu.esp -= 4;
            app->getMemory<float>(cpu.esp) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, kSine))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de14));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x48) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de10));
            cpu.esp -= 4;
            app->getMemory<float>(cpu.esp) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, kSine))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de14));
            app->getMemory<float>(cpu.ebp - 0x48) = float(fpu.st(0));
            fpu.pop();
        }
        cpu.eax = at(app, cpu.ebp - 0x48);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de18));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fpu.st(0));
        fpu.pop();
        if (!byAddress(app, cpu, kSine))
            return;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de1c));
        fpu.push(0.0);
        fcompp(cpu);
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de18));
            cpu.esp -= 4;
            app->getMemory<float>(cpu.esp) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, kSine))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de1c));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x4c) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de18));
            cpu.esp -= 4;
            app->getMemory<float>(cpu.esp) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, kSine))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de1c));
            app->getMemory<float>(cpu.ebp - 0x4c) = float(fpu.st(0));
            fpu.pop();
        }
        cpu.eax = at(app, cpu.ebp - 0x4c);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x23c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de20));
    fcomp(cpu, f32(app, cpu.ebp - 8));
    cpu.ax = fpu.status.word;
    if (below(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x23c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de20));
        app->getMemory<float>(cpu.ebp - 0x5c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 0x5c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x5c);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x5f0));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        fpu.push(f32(app, cpu.eax + 0x5f0));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x58) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x5f0);
        put(app, cpu.ebp - 0x58, cpu.eax);
    }
    if (i32(at(app, cpu.ebp - 0x58)) < 0x3ca3d70a)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x4f4));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            fpu.push(f32(app, cpu.eax + 0x4f4));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x54) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x4f4);
            put(app, cpu.ebp - 0x54, cpu.eax);
        }
        if (i32(at(app, cpu.ebp - 0x54)) >= 0x41566666)
            goto L_4ad022;
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (i32(at(app, cpu.eax + 0x51c)) < 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x51c);
            put(app, cpu.ebp - 0x50, cpu.eax);
            put(app, cpu.ebp - 0x50, 0u - at(app, cpu.ebp - 0x50));
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x51c);
            put(app, cpu.ebp - 0x50, cpu.eax);
        }
        if (i32(at(app, cpu.ebp - 0x50)) < 0x7f)
            goto L_4ad022;
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de24));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
L_4ad022:
        cpu.eax = at(app, cpu.ebp - 0x1c);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x5f0));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            fpu.push(f32(app, cpu.eax + 0x5f0));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x64) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x5f0);
            put(app, cpu.ebp - 0x64, cpu.eax);
        }
        if (i32(at(app, cpu.ebp - 0x64)) < 0x3c75c28f)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax + 0x4f4));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                fpu.push(f32(app, cpu.eax + 0x4f4));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x60) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x4f4);
                put(app, cpu.ebp - 0x60, cpu.eax);
            }
            if (i32(at(app, cpu.ebp - 0x60)) < 0x41566666)
            {
                fpu.push(f32(app, cpu.ebp - 8));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de28));
                app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
                fpu.pop();
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x10));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 8));
            fpu.st(0) = -fpu.st(0);
            cpu.eax = at(app, cpu.ebp - 0x18);
            fcomp(cpu, f32(app, cpu.eax + 0x10));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax + 0x10);
                put(app, cpu.ebp - 0x68, cpu.eax);
            }
            else
            {
                fpu.push(f32(app, cpu.ebp - 8));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x68) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0x68);
            put(app, cpu.ebp - 8, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.eax + 0x10));
            fcomp(cpu, f32(app, cpu.ebp - 8));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 8);
                put(app, cpu.ebp - 0x6c, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax + 0x10);
                put(app, cpu.ebp - 0x6c, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x6c);
            put(app, cpu.ebp - 8, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        if (byte(app, cpu.eax + 0x50d) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax + 0x4f4));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                fpu.push(f32(app, cpu.eax + 0x4f4));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x70) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x4f4);
                put(app, cpu.ebp - 0x70, cpu.eax);
            }
            if (i32(at(app, cpu.ebp - 0x70)) < 0x3f000000)
            {
                put(app, cpu.ebp - 8, 0);
            }
        }
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de2c));
        app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.eax + 0x10));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 8));
            fpu.st(0) = -fpu.st(0);
            cpu.eax = at(app, cpu.ebp - 0x18);
            fcomp(cpu, f32(app, cpu.eax + 0x10));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax + 0x10);
                put(app, cpu.ebp - 0x74, cpu.eax);
            }
            else
            {
                fpu.push(f32(app, cpu.ebp - 8));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x74) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0x74);
            put(app, cpu.ebp - 8, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            fpu.push(f32(app, cpu.eax + 0x10));
            fcomp(cpu, f32(app, cpu.ebp - 8));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 8);
                put(app, cpu.ebp - 0x78, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                cpu.eax = at(app, cpu.eax + 0x10);
                put(app, cpu.ebp - 0x78, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x78);
            put(app, cpu.ebp - 8, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de30));
        app->getMemory<float>(cpu.eax + 0x1c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    put(app, cpu.eax + 0x20, 0);
    cpu.eax = at(app, cpu.ebp - 0x18);
    fpu.push(f32(app, cpu.eax));
    cpu.eax = at(app, cpu.ebp - 0x18);
    app->getMemory<float>(cpu.eax + 0x24) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if (!direct(app, cpu, tyreSlide))
        return;
    cpu.eax = at(app, cpu.ebp - 0x18);
    if ((at(app, cpu.eax + 0xc) & 0x7fffffff) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.eax += 0x24;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.eax += 0x1c;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0xc));
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0x24));
        cpu.eax = at(app, cpu.ebp - 0x18);
        push(app, cpu, at(app, cpu.eax + 0x1c));
        if (!byAddress(app, cpu, kRotate))
            return;
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    if (at(app, cpu.eax + 0x28) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        fpu.push(f32(app, cpu.eax + 0x2c));
        cpu.eax = at(app, cpu.ebp - 0x1c);
        app->getMemory<float>(cpu.eax + 0x5c0) = float(fpu.st(0));
        fpu.pop();
        return leave();
    }
    cpu.eax = at(app, cpu.ebp - 0x18);
    fpu.push(f32(app, cpu.eax + 0x2c));
    cpu.eax = at(app, cpu.ebp - 0x1c);
    app->getMemory<float>(cpu.eax + 0x5c4) = float(fpu.st(0));
    fpu.pop();
    return leave();
}

/* 0x4a78d6: car eax's handling made for the race from the player's setup
 * (the car's +0x220, +0x40 seven sliders), when the raceload makes the car.
 * The setup put back to the middle (kDefaultSetup) for a car with flag 2,
 * over a link and in the series.  A human player's car (kPlayers) takes the
 * handling's second set (+0xc, +0x1bc, +0x74, +0xb4, +0xf4 over +8, +0x1b8,
 * +0x54, +0x94, +0xd4) unless kLoadingFlags has 4.  Each slider off its
 * middle -- but over a link or in the series -- moves its part of the
 * handling, held to its range: the third +0x23c, +0x234 and +0x258; the
 * second +0x1dc (and kAutoBox a 20th more); the fifth +0x254 (kAutoBox a 20th
 * less) and +0x294 by its square root; the sixth +0x24c and +0x1c8; the first
 * the torque's curve about its middle and +0x1c8; the simulation's handling
 * (kLoadingFlags 1) has the curve and +0x1c8 by 1.75 and each gear's +0xd4
 * held to 1.  Then by the gear: its ratio by the fourth, its inverse
 * (+0x54c, a 100th for none), its share (+0x56c), the limit's revs by it
 * (+0x52c); +0x1c8 by the fourth; the top gear's drag (+0x58c) from the
 * torque at the top speed. */
void applyTuning(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x48;
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
    put(app, cpu.ebp - 0x1c, cpu.eax);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    if ((byte(app, cpu.eax + 0x200) & 2) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.esi = at(app, cpu.eax + 0x220);
        cpu.ecx = 7;
        cpu.edi = cpu.esi + 0x40;
        cpu.esi = kDefaultSetup;
        for (; cpu.ecx != 0; --cpu.ecx)
        {
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
        }
    }
    if (at(app, kRaceType) == 1 || at(app, kRaceType) == 2 || i32(at(app, kPlayMode)) > 1)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.esi = at(app, cpu.eax + 0x220);
        cpu.ecx = 7;
        cpu.edi = cpu.esi + 0x40;
        cpu.esi = kDefaultSetup;
        for (; cpu.ecx != 0; --cpu.ecx)
        {
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.eax += 0x40;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
    if (at(app, cpu.eax + 0x6fd548) == 1 && (byte(app, kLoadingFlags) & 4) == 0)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.edx + 0xc);
        put(app, cpu.eax + 8, cpu.edx);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.edx + 0x1bc));
        app->getMemory<float>(cpu.eax + 0x1b8) = float(fpu.st(0));
        fpu.pop();
        put(app, cpu.ebp - 0x14, 0);
        while (i32(at(app, cpu.ebp - 0x14)) < 8)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.ecx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += cpu.ecx;
            fpu.push(f32(app, cpu.edx + 0x74));
            app->getMemory<float>(cpu.eax + 0x54) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.ecx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += cpu.ecx;
            fpu.push(f32(app, cpu.edx + 0xf4));
            app->getMemory<float>(cpu.eax + 0xd4) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.ecx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += cpu.ecx;
            fpu.push(f32(app, cpu.edx + 0xb4));
            app->getMemory<float>(cpu.eax + 0x94) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x14);
            put(app, cpu.ebp - 0x14, at(app, cpu.ebp - 0x14) + 1);
        }
    }
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 8));
    fcomp(cpu, f32(app, 0x55d02c));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx + 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x23c));
        app->getMemory<float>(cpu.eax + 0x23c) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.eax + 8));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, 0x55d02c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc4c));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x55d02c));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x234));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
        app->getMemory<float>(cpu.eax + 0x234) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        if (i32(at(app, cpu.eax + 0x234)) > 0x3eeb851f)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 0x234);
            put(app, cpu.ebp - 0x24, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0x24, 0x3eeb851f);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x24);
        put(app, cpu.edx + 0x234, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        if (i32(at(app, cpu.eax + 0x234)) > 0x3f0f5c29)
        {
            put(app, cpu.ebp - 0x20, 0x3f0f5c29);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 0x234);
            put(app, cpu.ebp - 0x20, cpu.eax);
        }
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x20);
        put(app, cpu.edx + 0x234, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(1.0);
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc50));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x258));
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x258));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 8));
        app->getMemory<float>(cpu.eax + 0x258) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 4));
    fcomp(cpu, f32(app, 0x55d028));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx + 4));
        fpu.st(0) = fp::sub(f32(app, cpu.eax + 0x1dc), fpu.st(0));
        app->getMemory<float>(cpu.eax + 0x1dc) = float(fpu.st(0));
        fpu.pop();
    }
    if (at(app, kAutoBox) != 0 && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x1dc));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dc54));
        app->getMemory<float>(cpu.eax + 0x1dc) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    if (i32(at(app, cpu.eax + 0x1dc)) > 0x3f400000)
    {
        put(app, cpu.ebp - 0x2c, 0x3f400000);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.eax + 0x1dc);
        put(app, cpu.ebp - 0x2c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.edx = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.ebp - 0x2c);
    put(app, cpu.edx + 0x1dc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    if (i32(at(app, cpu.eax + 0x1dc)) > 0x3e99999a)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.eax + 0x1dc);
        put(app, cpu.ebp - 0x28, cpu.eax);
    }
    else
    {
        put(app, cpu.ebp - 0x28, 0x3e99999a);
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.edx = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.ebp - 0x28);
    put(app, cpu.edx + 0x1dc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 0x10));
    fcomp(cpu, f32(app, 0x55d034));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx + 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x254));
        app->getMemory<float>(cpu.eax + 0x254) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        if (i32(at(app, cpu.eax + 0x254)) > 0x3e99999a)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 0x254);
            put(app, cpu.ebp - 0x34, cpu.eax);
        }
        else
        {
            put(app, cpu.ebp - 0x34, 0x3e99999a);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x34);
        put(app, cpu.edx + 0x254, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        if (i32(at(app, cpu.eax + 0x254)) > 0x3f333333)
        {
            put(app, cpu.ebp - 0x30, 0x3f333333);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 0x254);
            put(app, cpu.ebp - 0x30, cpu.eax);
        }
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x30);
        put(app, cpu.edx + 0x254, cpu.eax);
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.ebp - 4);
        push(app, cpu, at(app, cpu.eax + 0x10));
        if (!byAddress(app, cpu, kSqrt))
            return;
        fpu.st(0) = fp::div(f32(app, cpu.edx + 0x294), fpu.st(0));
        app->getMemory<float>(cpu.edx + 0x294) = float(fpu.st(0));
        fpu.pop();
    }
    if (at(app, kAutoBox) != 0 && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x254));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dc58));
        app->getMemory<float>(cpu.eax + 0x254) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 0x14));
    fcomp(cpu, f32(app, 0x55d038));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.eax + 0x14));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dc5c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc60));
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x24c));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
        app->getMemory<float>(cpu.eax + 0x24c) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx + 0x14));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, kDefaultSetup));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc64));
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        fpu.st(0) = fp::div(f32(app, cpu.eax + 0x1c8), fpu.st(0));
        app->getMemory<float>(cpu.eax + 0x1c8) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax));
    fcomp(cpu, f32(app, kDefaultSetup));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        put(app, cpu.ebp - 0x40, 0);
        put(app, cpu.ebp - 0x3c, 0);
        put(app, cpu.ebp - 0x38, 0);
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.edx + 0x1c4);
        cpu.edx += at(app, cpu.eax + 0x1c0);
        cpu.eax = cpu.edx;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(i32(cpu.eax) >> 1);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        {
            const reg32 carry = (cpu.edx >> 24) & 1;
            cpu.edx <<= 8;
            cpu.eax -= cpu.edx + carry;
        }
        cpu.eax = reg32(i32(cpu.eax) >> 8);
        cpu.eax += 3;
        put(app, cpu.ebp - 0x40, cpu.eax);
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.eax = at(app, cpu.edx + 0x1c4);
        cpu.edx = at(app, cpu.edx + 0x1c4);
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        {
            const reg32 carry = (cpu.edx >> 24) & 1;
            cpu.edx <<= 8;
            cpu.eax -= cpu.edx + carry;
        }
        cpu.eax = reg32(i32(cpu.eax) >> 8);
        cpu.eax += 3;
        put(app, cpu.ebp - 0x3c, cpu.eax);
        put(app, cpu.ebp - 0x14, 0);
        while (i32(at(app, cpu.ebp - 0x14)) < 0x29)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 4);
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dc68));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc6c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
            app->getMemory<float>(cpu.ebp - 0x38) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x40);
            cpu.eax -= at(app, cpu.ebp - 0x14);
            put(app, cpu.ebp - 0x44, cpu.eax);
            fpu.push(x86::Float(i32(at(app, cpu.ebp - 0x44))));
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax -= at(app, cpu.ebp - 0x40);
            put(app, cpu.ebp - 0x44, cpu.eax);
            fpu.push(x86::Float(i32(at(app, cpu.ebp - 0x44))));
            fpu.st(1) = fp::div(fpu.st(1), fpu.st(0));
            fpu.pop();
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
            app->getMemory<float>(cpu.ebp - 0x38) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += cpu.edx;
            fpu.push(f32(app, cpu.eax + 0x114));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x38));
            app->getMemory<float>(cpu.eax + 0x114) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x14);
            put(app, cpu.ebp - 0x14, at(app, cpu.ebp - 0x14) + 1);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, kDefaultSetup));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc70));
        fpu.push(1.0);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1c8));
        app->getMemory<float>(cpu.eax + 0x1c8) = float(fpu.st(0));
        fpu.pop();
    }
    if ((byte(app, kLoadingFlags) & 1) != 0)
    {
        put(app, cpu.ebp - 0x14, 0);
        while (i32(at(app, cpu.ebp - 0x14)) < 0x29)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.eax += cpu.edx;
            fpu.push(f32(app, cpu.eax + 0x114));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc74));
            app->getMemory<float>(cpu.eax + 0x114) = float(fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0x14);
            put(app, cpu.ebp - 0x14, at(app, cpu.ebp - 0x14) + 1);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x1c8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc78));
        app->getMemory<float>(cpu.eax + 0x1c8) = float(fpu.st(0));
        fpu.pop();
    }
    put(app, cpu.ebp - 0x14, 0);
    for (;;)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x14);
        if (i32(cpu.eax) >= i32(at(app, cpu.edx + 8)))
            break;
        if ((byte(app, kLoadingFlags) & 1) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.eax += cpu.edx;
            if (i32(at(app, cpu.eax + 0xd4)) > 0x3f800000)
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.edx = at(app, cpu.ebp - 0x14);
                cpu.edx <<= 2;
                cpu.eax += cpu.edx;
                cpu.eax = at(app, cpu.eax + 0xd4);
                put(app, cpu.ebp - 0x48, cpu.eax);
            }
            else
            {
                put(app, cpu.ebp - 0x48, 0x3f800000);
            }
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x48);
            put(app, cpu.edx + 0xd4, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.eax + 0xc));
        fcomp(cpu, f32(app, 0x55d030));
        cpu.ax = fpu.status.word;
        if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x14);
            cpu.edx <<= 2;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, cpu.ebp - 4);
            fpu.push(f32(app, cpu.edx + 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x94));
            app->getMemory<float>(cpu.eax + 0x94) = float(fpu.st(0));
            fpu.pop();
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 0x14);
        cpu.edx <<= 2;
        cpu.eax += cpu.edx;
        if ((at(app, cpu.eax + 0x94) & 0x7fffffff) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += cpu.edx;
            fpu.push(1.0);
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.eax + 0x94));
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += at(app, cpu.ebp - 0x1c);
            app->getMemory<float>(cpu.eax + 0x54c) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x14);
            cpu.eax <<= 2;
            cpu.eax += at(app, cpu.ebp - 0x1c);
            put(app, cpu.eax + 0x54c, 0x3c23d70a);
        }
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax <<= 2;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.edx + 0x94));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.eax + 4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc7c));
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax <<= 2;
        cpu.eax += at(app, cpu.ebp - 0x1c);
        app->getMemory<float>(cpu.eax + 0x56c) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax <<= 2;
        cpu.eax += at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.ebp - 0x1c);
        cpu.edx = at(app, cpu.edx + 0x528);
        cpu.ecx = at(app, cpu.ebp - 0x14);
        cpu.ecx <<= 2;
        cpu.edx += cpu.ecx;
        fpu.push(f32(app, cpu.edx + 0xd4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x56c));
        app->getMemory<float>(cpu.eax + 0x56c) = float(fpu.st(0));
        fpu.pop();
        cpu.edx = at(app, cpu.ebp - 0x14);
        cpu.edx <<= 2;
        cpu.edx += at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c4))));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x54c));
        fpu.push(1.0);
        fpu.st(1) = fp::div(fpu.st(0), fpu.st(1));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x14);
        cpu.eax <<= 2;
        cpu.eax += at(app, cpu.ebp - 0x1c);
        app->getMemory<float>(cpu.eax + 0x52c) = float(fpu.st(0));
        fpu.pop();
        cpu.eax = at(app, cpu.ebp - 0x14);
        put(app, cpu.ebp - 0x14, at(app, cpu.ebp - 0x14) + 1);
    }
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 0xc));
    fcomp(cpu, f32(app, 0x55d030));
    cpu.ax = fpu.status.word;
    if (!equal(cpu) && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.edx + 0xc));
        fpu.st(0) = fp::div(f32(app, cpu.eax + 0x1c8), fpu.st(0));
        app->getMemory<float>(cpu.eax + 0x1c8) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.eax <<= 2;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.edx = at(app, cpu.eax + 0x528);
    cpu.edx += cpu.ecx;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x1c8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x90));
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
    fpu.pop();
    cpu.eax = pop(app, cpu);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.ecx = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.ebp - 0x10);
    cpu.edx = at(app, cpu.ebp - 0x10);
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    {
        const reg32 carry = (cpu.edx >> 24) & 1;
        cpu.edx <<= 8;
        cpu.eax -= cpu.edx + carry;
    }
    cpu.eax = reg32(i32(cpu.eax) >> 8);
    put(app, cpu.ebp - 0x44, cpu.eax);
    fpu.push(x86::Float(i32(at(app, cpu.ebp - 0x44))));
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
    fpu.pop();
    cpu.eax = pop(app, cpu);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.eax = at(app, cpu.eax + 0x114);
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.eax <<= 2;
    cpu.eax += at(app, cpu.ebp - 0x1c);
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x568));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.eax <<= 2;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    cpu.eax += cpu.edx;
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.eax + 0xd0));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    if (i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1 && at(app, kRaceType) != 2)
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        cpu.eax = at(app, cpu.eax + 0x220);
        fpu.push(1.0);
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x54));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53dc80));
        cpu.eax = at(app, cpu.ebp - 0x1c);
        fpu.st(0) = fp::div(f32(app, cpu.ebp - 0xc), fpu.st(0));
        app->getMemory<float>(cpu.eax + 0x58c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53dc84));
        app->getMemory<float>(cpu.eax + 0x58c) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x1c8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc88));
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1c8));
    cpu.eax = at(app, cpu.ebp - 0x1c);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1c8));
    cpu.eax = at(app, cpu.ebp - 0x1c);
    fpu.st(0) = fp::div(f32(app, cpu.eax + 0x58c), fpu.st(0));
    app->getMemory<float>(cpu.eax + 0x58c) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0x1c);
    fpu.push(f32(app, cpu.eax + 0x58c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dc8c));
    app->getMemory<float>(cpu.eax + 0x58c) = float(fpu.st(0));
    fpu.pop();
    return leave();
}

/* 0x4a8bb9: the automatic gearbox's choice for car eax, to eax: 0 in reverse
 * or neutral, 2 for a player's car finished, else the gear (+0x50b) as it is
 * or the one up or down it changes to.  The revs each gear would turn at
 * (the wheels' turn +0x5a4 by its ratio) against the limit's 5/6 and 7/8
 * (up, but not past the top), and half the limit (down: by one or two, by the
 * throttle) -- by the brake (+0x5f0 past 0.9) the revs' size against the
 * limit first. */
void automaticGear(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x44;
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
    put(app, cpu.ebp - 0x3c, cpu.eax);
    put(app, cpu.ebp - 0x34, 0);
    put(app, cpu.ebp - 0x30, 0);
    put(app, cpu.ebp - 0x2c, 0);
    put(app, cpu.ebp - 0x28, 0);
    put(app, cpu.ebp - 0x24, 0);
    put(app, cpu.ebp - 0x20, 0);
    put(app, cpu.ebp - 0x1c, 0);
    put(app, cpu.ebp - 0x18, 0);
    cpu.edx = at(app, cpu.ebp - 0x3c);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.ecx = 6;
    cpu.eax = at(app, cpu.edx + 0x1c4);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ecx);
    cpu.edx = at(app, cpu.ebp - 0x3c);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx -= cpu.eax;
    put(app, cpu.ebp - 0x14, cpu.edx);
    cpu.edx = at(app, cpu.ebp - 0x3c);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.eax = at(app, cpu.edx + 0x1c4);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    {
        const reg32 carry = (cpu.edx >> 29) & 1;
        cpu.edx <<= 3;
        cpu.eax -= cpu.edx + carry;
    }
    cpu.eax = reg32(i32(cpu.eax) >> 3);
    cpu.edx = at(app, cpu.ebp - 0x3c);
    cpu.edx = at(app, cpu.edx + 0x528);
    cpu.edx = at(app, cpu.edx + 0x1c4);
    cpu.edx -= cpu.eax;
    put(app, cpu.ebp - 0x10, cpu.edx);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, cpu.ebp - 0x3c);
    if (byte(app, cpu.eax + 0x50b) < 2)
    {
        put(app, cpu.ebp - 0x38, 0);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x3c);
        if ((byte(app, cpu.eax + 0x200) & 1) != 0 && (cpu.eax = at(app, cpu.ebp - 0x3c), at(app, cpu.eax + 0x2c4) == 3))
        {
            put(app, cpu.ebp - 0x38, 2);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x3c);
            fpu.push(0.0);
            fcomp(cpu, f32(app, cpu.eax + 0x5b0));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                fpu.push(f32(app, cpu.eax + 0x5b0));
                fpu.st(0) = -fpu.st(0);
                app->getMemory<float>(cpu.ebp - 0x40) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.eax = at(app, cpu.eax + 0x5b0);
                put(app, cpu.ebp - 0x40, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x40);
            put(app, cpu.ebp - 0xc, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.eax + 8);
            cpu.edx <<= 2;
            cpu.edx += at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c4))));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x544));
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            fcomp(cpu, f32(app, cpu.eax + 0x1c8));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                put(app, cpu.ebp - 0x18, 1);
            }
            else
            {
                put(app, cpu.ebp - 0x18, 0);
            }
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.eax + 8);
            --cpu.eax;
            cpu.eax -= at(app, cpu.ebp - 0x18);
            cpu.edx = cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.al = byte(app, cpu.eax + 0x50b);
            cpu.eax &= 0xff;
            if (i32(cpu.eax) < i32(cpu.edx))
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.eax + 0x50b);
                cpu.eax &= 0xff;
                ++cpu.eax;
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
            else
            {
                cpu.eax = 0;
                cpu.edx = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.edx + 0x50b);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x3c);
            if (byte(app, cpu.eax + 0x50b) > 2)
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.eax + 0x50b);
                cpu.eax &= 0xff;
                --cpu.eax;
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
            else
            {
                cpu.eax = 0;
                cpu.edx = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.edx + 0x50b);
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x3c);
            if (byte(app, cpu.eax + 0x50b) > 3)
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.eax + 0x50b);
                cpu.eax &= 0xff;
                cpu.eax -= 2;
                put(app, cpu.ebp - 0x20, cpu.eax);
            }
            else
            {
                cpu.eax = 0;
                cpu.edx = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.edx + 0x50b);
                put(app, cpu.ebp - 0x20, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.al = byte(app, cpu.eax + 0x50b);
            cpu.eax &= 0xff;
            cpu.eax <<= 2;
            cpu.edx = cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            fpu.push(f32(app, cpu.eax + 0x5a4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
            cpu.esp -= 4;
            put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
            cpu.eax = pop(app, cpu);
            put(app, cpu.ebp - 0x34, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x1c);
            cpu.eax <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            fpu.push(f32(app, cpu.eax + 0x5a4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
            cpu.esp -= 4;
            put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
            cpu.eax = pop(app, cpu);
            put(app, cpu.ebp - 0x30, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.edx = at(app, cpu.eax + 0x528);
            cpu.eax = at(app, cpu.ebp - 0x24);
            cpu.eax <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            fpu.push(f32(app, cpu.eax + 0x5a4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
            cpu.esp -= 4;
            put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
            cpu.eax = pop(app, cpu);
            put(app, cpu.ebp - 0x2c, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.ebp - 0x20);
            cpu.edx <<= 2;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 0x3c);
            fpu.push(f32(app, cpu.eax + 0x5a4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x94));
            cpu.esp -= 4;
            put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
            cpu.eax = pop(app, cpu);
            put(app, cpu.ebp - 0x28, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x3c);
            if (i32(at(app, cpu.eax + 0x5f0)) <= 0x3f666666)
            {
                if (i32(at(app, cpu.ebp - 0x34)) < 0)
                {
                    cpu.eax = at(app, cpu.ebp - 0x34);
                    put(app, cpu.ebp - 0x44, cpu.eax);
                    put(app, cpu.ebp - 0x44, 0u - at(app, cpu.ebp - 0x44));
                }
                else
                {
                    cpu.eax = at(app, cpu.ebp - 0x34);
                    put(app, cpu.ebp - 0x44, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.edx = at(app, cpu.eax + 0x528);
                cpu.eax = at(app, cpu.ebp - 0x44);
                if (i32(cpu.eax) <= i32(at(app, cpu.edx + 0x1c4)))
                    goto L_4a9025;
            }
            cpu.eax = at(app, cpu.ebp - 0x34);
            if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x10)))
            {
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.eax = at(app, cpu.eax + 0x528);
                cpu.eax = at(app, cpu.eax + 8);
                --cpu.eax;
                cpu.eax -= at(app, cpu.ebp - 0x18);
                cpu.edx = cpu.eax;
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.eax + 0x50b);
                cpu.eax &= 0xff;
                if (i32(cpu.eax) >= i32(cpu.edx))
                    goto L_4a8f2a;
                cpu.eax = at(app, cpu.ebp - 0x3c);
                cpu.al = byte(app, cpu.eax + 0x50b);
                cpu.eax &= 0xff;
                if (cpu.eax != at(app, cpu.ebp - 0x1c))
                {
                    cpu.eax = 0;
                    cpu.edx = at(app, cpu.ebp - 0x3c);
                    cpu.al = byte(app, cpu.edx + 0x50b);
                    put(app, cpu.ebp - 4, cpu.eax);
                    cpu.eax = at(app, cpu.ebp - 0x1c);
                    put(app, cpu.ebp - 8, cpu.eax);
                }
            }
            else
            {
L_4a8f2a:
                cpu.edx = at(app, cpu.ebp - 0x3c);
                cpu.edx = at(app, cpu.edx + 0x528);
                cpu.eax = at(app, cpu.edx + 0x1c4);
                cpu.edx = at(app, cpu.edx + 0x1c4);
                cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
                cpu.eax -= cpu.edx;
                cpu.eax = reg32(i32(cpu.eax) >> 1);
                if ((i32(cpu.eax) > i32(at(app, cpu.ebp - 0x28)) && at(app, cpu.ebp - 0x20) == 2 && i32(at(app, cpu.ebp - 0xc)) < 0x3d4ccccd) || ((cpu.eax = at(app, cpu.ebp - 0x28), i32(cpu.eax) < i32(at(app, cpu.ebp - 0x14))) && i32(at(app, cpu.ebp - 0x20)) > 2 && i32(at(app, cpu.ebp - 0xc)) < 0x3d4ccccd))
                {
                    cpu.eax = at(app, cpu.ebp - 0x3c);
                    cpu.al = byte(app, cpu.eax + 0x50b);
                    cpu.eax &= 0xff;
                    if (cpu.eax == at(app, cpu.ebp - 0x20))
                        goto L_4a8fa5;
                    cpu.eax = 0;
                    cpu.edx = at(app, cpu.ebp - 0x3c);
                    cpu.al = byte(app, cpu.edx + 0x50b);
                    put(app, cpu.ebp - 4, cpu.eax);
                    cpu.eax = at(app, cpu.ebp - 0x20);
                    put(app, cpu.ebp - 8, cpu.eax);
                }
                else
                {
L_4a8fa5:
                    cpu.edx = at(app, cpu.ebp - 0x3c);
                    cpu.edx = at(app, cpu.edx + 0x528);
                    cpu.eax = at(app, cpu.edx + 0x1c4);
                    cpu.edx = at(app, cpu.edx + 0x1c4);
                    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
                    cpu.eax -= cpu.edx;
                    cpu.eax = reg32(i32(cpu.eax) >> 1);
                    if (i32(cpu.eax) > i32(at(app, cpu.ebp - 0x2c)) && at(app, cpu.ebp - 0x24) == 2)
                    {
                        fpu.push(f32(app, cpu.ebp - 0xc));
                        fcomp(cpu, f64(app, 0x53dca4));
                        cpu.ax = fpu.status.word;
                        if (below(cpu))
                            goto L_4a8ff7;
                    }
                    cpu.eax = at(app, cpu.ebp - 0x2c);
                    if (i32(cpu.eax) < i32(at(app, cpu.ebp - 0x14)) && i32(at(app, cpu.ebp - 0x24)) > 2 && i32(at(app, cpu.ebp - 0xc)) < 0x3d4ccccd)
                    {
L_4a8ff7:
                        cpu.eax = at(app, cpu.ebp - 0x3c);
                        cpu.al = byte(app, cpu.eax + 0x50b);
                        cpu.eax &= 0xff;
                        if (cpu.eax != at(app, cpu.ebp - 0x24))
                        {
                            cpu.eax = 0;
                            cpu.edx = at(app, cpu.ebp - 0x3c);
                            cpu.al = byte(app, cpu.edx + 0x50b);
                            put(app, cpu.ebp - 4, cpu.eax);
                            cpu.eax = at(app, cpu.ebp - 0x24);
                            put(app, cpu.ebp - 8, cpu.eax);
                            goto L_4a908e;
L_4a9025:
                            cpu.edx = at(app, cpu.ebp - 0x3c);
                            cpu.edx = at(app, cpu.edx + 0x528);
                            cpu.eax = at(app, cpu.edx + 0x1c4);
                            cpu.edx = at(app, cpu.edx + 0x1c4);
                            cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
                            cpu.eax -= cpu.edx;
                            cpu.eax = reg32(i32(cpu.eax) >> 1);
                            if ((i32(cpu.eax) > i32(at(app, cpu.ebp - 0x2c)) && at(app, cpu.ebp - 0x24) == 2) || ((cpu.eax = at(app, cpu.ebp - 0x2c), i32(cpu.eax) < i32(at(app, cpu.ebp - 0x14))) && i32(at(app, cpu.ebp - 0x24)) > 2 && i32(at(app, cpu.ebp - 0xc)) < 0x3dcccccd))
                            {
                                cpu.eax = at(app, cpu.ebp - 0x3c);
                                cpu.al = byte(app, cpu.eax + 0x50b);
                                cpu.eax &= 0xff;
                                if (cpu.eax != at(app, cpu.ebp - 0x24))
                                {
                                    cpu.eax = 0;
                                    cpu.edx = at(app, cpu.ebp - 0x3c);
                                    cpu.al = byte(app, cpu.edx + 0x50b);
                                    put(app, cpu.ebp - 4, cpu.eax);
                                    cpu.eax = at(app, cpu.ebp - 0x24);
                                    put(app, cpu.ebp - 8, cpu.eax);
                                }
                            }
                        }
                    }
                }
            }
L_4a908e:
            cpu.eax = at(app, cpu.ebp - 8);
            put(app, cpu.ebp - 0x38, cpu.eax);
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    return leave();
}

/* 0x4aa4f4: car eax's speeds along its axes (+0x4ec, +0x4f4: dot products of
 * its speed with +0x190 and +0x1a8), and in a gear past second -- or in
 * first, or slow in second -- the engine's revs (+0x590) matched to the
 * speed by the gear's +0x52c, held to the idle (chopped); the clutch, the
 * throttle and the axles' slip cleared. */
void matchRevsToSpeed(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
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
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.edx = at(app, cpu.ebp - 0xc);
    cpu.edx += 0x190;
    cpu.eax = at(app, cpu.ebp - 0xc);
    cpu.eax += 0xa4;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.eax = at(app, cpu.ebp - 0xc);
    app->getMemory<float>(cpu.eax + 0x4ec) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 0xc);
    cpu.edx += 0x1a8;
    cpu.eax = at(app, cpu.ebp - 0xc);
    cpu.eax += 0xa4;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.eax = at(app, cpu.ebp - 0xc);
    app->getMemory<float>(cpu.eax + 0x4f4) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp - 0xc);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.eax + 0x4f4));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        fpu.push(f32(app, cpu.eax + 0x4f4));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        cpu.eax = at(app, cpu.eax + 0x4f4);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    if ((i32(at(app, cpu.ebp - 4)) >= 0x3f800000 || (cpu.eax = at(app, cpu.ebp - 0xc), byte(app, cpu.eax + 0x50b) != 2)) && (cpu.eax = at(app, cpu.ebp - 0xc), byte(app, cpu.eax + 0x50b) == 1))
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        if (byte(app, cpu.eax + 0x50b) != 1)
        {
            cpu.eax = at(app, cpu.ebp - 0xc);
            cpu.edx = 0;
            cpu.dl = byte(app, cpu.eax + 0x50b);
            cpu.edx <<= 2;
            cpu.edx += at(app, cpu.ebp - 0xc);
            cpu.eax = at(app, cpu.ebp - 0xc);
            fpu.push(f32(app, cpu.eax + 0x4f4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x52c));
            cpu.eax = at(app, cpu.ebp - 0xc);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c4))));
            fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, cpu.ebp - 0xc);
            cpu.eax = at(app, cpu.eax + 0x528);
            fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c0))));
            fcompp(cpu);
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                cpu.edx = 0;
                cpu.dl = byte(app, cpu.eax + 0x50b);
                cpu.edx <<= 2;
                cpu.edx += at(app, cpu.ebp - 0xc);
                cpu.eax = at(app, cpu.ebp - 0xc);
                fpu.push(f32(app, cpu.eax + 0x4f4));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x52c));
                cpu.eax = at(app, cpu.ebp - 0xc);
                cpu.eax = at(app, cpu.eax + 0x528);
                fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c4))));
                fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
                fpu.pop();
                app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
                fpu.pop();
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                cpu.eax = at(app, cpu.eax + 0x528);
                fpu.push(x86::Float(i32(at(app, cpu.eax + 0x1c0))));
                app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
                fpu.pop();
            }
            cpu.eax = at(app, cpu.ebp - 0xc);
            fpu.push(f32(app, cpu.ebp - 0x10));
            if (!byAddress(app, cpu, kChop))
                return;
            put(app, cpu.eax + 0x590, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
        }
    }
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.eax + 0x594, 0);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.eax + 0x5b0, 0);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.eax + 0x5c0, 0);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.eax + 0x5c4, 0);
    return leave();
}

/* 0x4aa79f: car eax's drive put back: +0x506 and the gear (+0x50b) 2, +0x507
 * 0 -- the same for a human player's car or not -- and its forces, speeds
 * along the axes, the clutch, the throttle and the axles' slip cleared. */
void resetDriveState(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
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
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax + 0x1f0) * 0x6c;
    if (at(app, cpu.eax + 0x6fd548) == 1)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x506, 2);
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x50b, 2);
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x507, 0);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x506, 2);
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x50b, 2);
        cpu.eax = at(app, cpu.ebp - 8);
        putByte(app, cpu.eax + 0x507, 0);
    }
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2e8, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2ec, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2f0, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2f4, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2f8, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2fc, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x4ec, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x4f0, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x4f4, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x4f8, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x4fc, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x500, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x594, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x5b0, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x5c0, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x5c4, 0);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.eax + 0x2e4, 0);
    return leave();
}

/* 0x4aa8e7: car eax settled when it is all but stopped ([+0x15c] 0.2 or less)
 * and upright (+0x130's up 0 or more): its speed's and its spin's parts off
 * the up axis damped -- by 0.94 and 0.98 at the race's start, else by 0.98
 * and 0.998 when it hardly moves (two of the speeds' sizes under 0.9) --
 * the vectors scaled, less and added by 0x4e0050, 0x4e0000, 0x4dffb0. */
void settleAtRest(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x3c;
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
    put(app, cpu.ebp - 0x18, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax += 0x130;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x18);
    if (i32(at(app, cpu.eax + 0x15c)) > 0x3e4ccccd)
        return leave();
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 4));
    fcomp(cpu, f64(app, 0x53dcc4));
    cpu.ax = fpu.status.word;
    if (below(cpu))
        return leave();
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax = at(app, cpu.eax + 0x10c);
    put(app, cpu.ebp - 0x20, cpu.eax);
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax += 0xc0;
    if (!byAddress(app, cpu, kDot))
        return;
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax += 0xd8;
    if (!byAddress(app, cpu, kDot))
        return;
    app->getMemory<float>(cpu.ebp - 0x1c) = float(fpu.st(0));
    fpu.pop();
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ebp - 0x20));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x20));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x20);
        put(app, cpu.ebp - 0x3c, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x3c);
    put(app, cpu.ebp - 0x20, cpu.eax);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ebp - 0xc));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x38) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 0x38, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x38);
    put(app, cpu.ebp - 0xc, cpu.eax);
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ebp - 0x1c));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x34) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.ebp - 0x34, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x34);
    put(app, cpu.ebp - 0x1c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x18);
    if (half(app, cpu.eax + 0x164) == 0)
    {
        put(app, cpu.ebp - 0x14, 0x3f70a3d7);
        put(app, cpu.ebp - 0x10, 0x3f7ae148);
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0x20));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0xc));
        fcomp(cpu, f32(app, 0x53dccc));
        cpu.ax = fpu.status.word;
        if (below(cpu) || equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x20));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            fcomp(cpu, f32(app, 0x53dccc));
            cpu.ax = fpu.status.word;
            if (below(cpu) || equal(cpu))
            {
                fpu.push(f32(app, cpu.ebp - 0xc));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x1c));
                fcomp(cpu, f32(app, 0x53dccc));
                cpu.ax = fpu.status.word;
                if (below(cpu) || equal(cpu))
                    return leave();
            }
        }
        put(app, cpu.ebp - 0x14, 0x3f7ae148);
        put(app, cpu.ebp - 0x10, 0x3f7f7cee);
    }
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xa4;
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kDot))
        return;
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = cpu.ebp - 0x30;
    push(app, cpu, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 8));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = 1;
    if (!byAddress(app, cpu, kScale))
        return;
    cpu.ecx = at(app, cpu.ebp - 0x18);
    cpu.ecx += 0xa4;
    cpu.ebx = cpu.ebp - 0x30;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xa4;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.eax = at(app, cpu.ebp - 0x18);
    cpu.eax += 0xa4;
    push(app, cpu, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x14));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xa4;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kScale))
        return;
    cpu.ecx = at(app, cpu.ebp - 0x18);
    cpu.ecx += 0xa4;
    cpu.ebx = cpu.ebp - 0x30;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xa4;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kAdd))
        return;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xe4;
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kDot))
        return;
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = cpu.ebp - 0x30;
    push(app, cpu, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 8));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = 1;
    if (!byAddress(app, cpu, kScale))
        return;
    cpu.ecx = at(app, cpu.ebp - 0x18);
    cpu.ecx += 0xe4;
    cpu.ebx = cpu.ebp - 0x30;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xe4;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.eax = cpu.ebp - 0x30;
    push(app, cpu, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x10));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    cpu.edx = cpu.ebp - 0x30;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kScale))
        return;
    cpu.ecx = at(app, cpu.ebp - 0x18);
    cpu.ecx += 0xe4;
    cpu.ebx = cpu.ebp - 0x30;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.edx += 0xe4;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kAdd))
        return;
    return leave();
}

}  // namespace handling
}  // namespace nfs3hp
