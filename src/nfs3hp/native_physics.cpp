#include "native_guest.h"
#include "diag_hooks.h"

#include <cstdint>
#include <utility>

/* A car's physics, frame by frame: the step the race calls for each car
 * (carPhysics, whose address raceload stores at 0x42c429), the full step
 * (physicsStep) and the parts of it that are this file's -- the car's speed
 * in its own axes, its slip, the steering's angle, the spin's torque, the
 * damping of its speed and spin when it idles, coasts or is held still, its
 * grip sideways, the handbrake's drag, the yaw's damping.  The engine and the
 * gears (0x4aaed1), the driver's inputs (0x4a9bc4), the wheels' loads
 * (0x4aab41) and forces (0x4aca4f), and the car at rest (0x4aa8e7) are other
 * files' and are called by address.  disassemble_nfs3hp.py binds each
 * address to the function of that name here (PHYSICS_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code.
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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack in cpu.fpu as the
 * code pushes and pops it (rounded as its control word says, the race's:
 * x86::FPU's add, sub, mul, div), and a compare's status word left in ax as
 * fnstsw leaves it (below and equal the flags sahf makes of it).  Every other
 * function of the game's is called by address; the game's constants (floats
 * and doubles in its data, 0x53de34 to 0x53df34) are read where the code
 * reads them. */
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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;

// The race's: whether two players share a screen or it is a network race,
// the race's type (1 and 2 the series), the frame counter (the car's speed
// in its own axes from 0x200 on, after the countdown), a word compared and
// never used.
constexpr reg32 kPlayMode = 0x6fd3b0, kRaceType = 0x6fd3b8, kRaceFrame = 0x7d3684, kUnusedFlag = 0x6fd494;
// Gravity in the car's axes, three floats, worked out each step.
constexpr reg32 kGravity = 0x7a3a40, kGravityY = 0x7a3a44, kGravityZ = 0x7a3a48;
// The surfaces' grips, a float each by the car's surface.
constexpr reg32 kSurfaceGrip = 0x55ed30;
// What these call by address: a matrix transposed, a vector's length, two
// floats' length, vectors scaled; the AI's step, the car at rest, the engine,
// the driver's inputs, the wheels' loads and forces, the car's sounds.
constexpr reg32 kTranspose = 0x4e0110, kLength = 0x4e06a0, kPlanarLength = 0x4967e0, kScaleVectors = 0x4e0050;
constexpr reg32 kAiStep = 0x4088f0, kCarAtRest = 0x4aa8e7, kEngine = 0x4aaed1, kDriverInputs = 0x4a9bc4;
constexpr reg32 kWheelLoads = 0x4aab41, kWheelForces = 0x4aca4f, kCarSounds = 0x4cdf10;

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

/* The module's prologue -- push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp;
 * sub esp, frame -- and its epilogue with the ret. */
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

/* fcomp m: st(0) compared with m, popped, and the code's fnstsw ax after it;
 * fcompp: with st(1), both popped.  sahf's flags of ah: below is the carry
 * (C0), equal the zero flag (C3); above neither -- ja, jbe's other way. */
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

bool below(const CPU& cpu) { return (cpu.ah & 0x01) != 0; }
bool equal(const CPU& cpu) { return (cpu.ah & 0x40) != 0; }
bool above(const CPU& cpu) { return !below(cpu) && !equal(cpu); }

/* fldz; fcomp dword m; fnstsw ax. */
void zeroCompare(CPU& cpu, const x86::Float& m)
{
    cpu.fpu.push(0.0);
    fcomp(cpu, m);
}

/* fstp dword m, fstp qword m. */
void fstp(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<float>(a) = float(cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fstpDouble(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<double>(a) = double(cpu.fpu.st(0));
    cpu.fpu.pop();
}

/* The size of a float the module's way, into a local:
 *     fldz; fcomp dword [from]; fnstsw ax; sahf; jbe +
 *     [mov eax, [self]]; fld dword [from]; fchs; fstp dword [to]; jmp ++
 * +:  [mov eax, [self]]; mov eax, [from]; mov [to], eax
 * ++: -- the car reloaded from its local `self` where the code does (0: not),
 * so that eax is left the car, or the float's bits, or fnstsw's ax. */
void magnitude(App* app, CPU& cpu, reg32 from, reg32 to, reg32 self)
{
    zeroCompare(cpu, f32(app, from));
    if (above(cpu))
    {
        if (self != 0)
            cpu.eax = at(app, self);
        cpu.fpu.push(f32(app, from));
        cpu.fpu.st(0) = -cpu.fpu.st(0);
        fstp(app, cpu, to);
    }
    else
    {
        if (self != 0)
            cpu.eax = at(app, self);
        cpu.eax = at(app, from);
        put(app, to, cpu.eax);
    }
}

/* The size of an integer the same way: [to] the car's [from], negated (neg)
 * where it is under 0. */
void integerMagnitude(App* app, CPU& cpu, reg32 offset, reg32 to, reg32 self)
{
    cpu.eax = at(app, self);
    if (i32(at(app, cpu.eax + offset)) < 0)
    {
        cpu.eax = at(app, self);
        cpu.eax = at(app, cpu.eax + offset);
        put(app, to, cpu.eax);
        put(app, to, reg32(0) - at(app, to));
    }
    else
    {
        cpu.eax = at(app, self);
        cpu.eax = at(app, cpu.eax + offset);
        put(app, to, cpu.eax);
    }
}

/* The product the module inlines wherever a vector goes through a matrix:
 * the vector at eax, row by row, times the matrix at edx -- [ebx + 4j] =
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
        std::swap(fpu.st(0), fpu.st(2));
        if (row != 0)
            for (int k = 0; k < 3; ++k)
            {
                fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
                fpu.pop();
            }
    }
    cpu.eax += 0xc;
    fstp(app, cpu, cpu.ebx);
    fstp(app, cpu, cpu.ebx + 4);
    fstp(app, cpu, cpu.ebx + 8);
}

/* A float of the car's times a constant, back to it:
 *     mov eax, [self]; fld dword [eax + offset]; fmul dword [factor];
 *     fstp dword [eax + offset] */
void scaleField(App* app, CPU& cpu, reg32 self, reg32 offset, reg32 factor)
{
    cpu.eax = at(app, self);
    cpu.fpu.push(f32(app, cpu.eax + offset));
    cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), f32(app, factor));
    fstp(app, cpu, cpu.eax + offset);
}

/* A vector of the car's scaled (0x4e0050, one vector, stdcall): its own
 * address pushed, then the factor; the vector in edx too, eax 1. */
bool scaleVector(App* app, CPU& cpu, reg32 self, reg32 offset, reg32 factor)
{
    cpu.eax = at(app, self);
    cpu.eax += offset;
    push(app, cpu, cpu.eax);
    push(app, cpu, factor);
    cpu.edx = at(app, self);
    cpu.edx += offset;
    cpu.eax = 1;
    return byAddress(app, cpu, kScaleVectors);
}

}  // namespace

void clearWheelForces(App* app, CPU& cpu);
void steerAngle(App* app, CPU& cpu);
void sideSlip(App* app, CPU& cpu);
void groundSpeed(App* app, CPU& cpu);
void physicsStep(App* app, CPU& cpu);
void carPhysics(App* app, CPU& cpu);
void settleLoadShift(App* app, CPU& cpu);
void driveForce(App* app, CPU& cpu);
void holdStill(App* app, CPU& cpu);
void sideGrip(App* app, CPU& cpu);
void handbrakeDrag(App* app, CPU& cpu);
void yawDamping(App* app, CPU& cpu);
void idleDamping(App* app, CPU& cpu);
void coastDamping(App* app, CPU& cpu);
void spinTorque(App* app, CPU& cpu);

/* 0x4ad250: a wheel's record of the step's (0x30 bytes at eax, in
 * physicsStep's frame) cleared but for its +0x8: the dwords at +0 to +0xc,
 * +0x1c to +0x24, +0x10 to +0x18, +0x28 and +0x2c, in that order. */
void clearWheelForces(App* app, CPU& cpu)
{
    enter(app, cpu, 8);
    const reg32 self = cpu.ebp - 8;
    put(app, self, cpu.eax);
    for (const reg32 offset : {0x0u, 0x4u, 0x8u, 0xcu, 0x1cu, 0x20u, 0x24u, 0x10u, 0x14u, 0x18u, 0x28u, 0x2cu})
    {
        cpu.eax = at(app, self);
        put(app, cpu.eax + offset, 0);
    }
    leave(app, cpu);
}

/* 0x4ad2e1: the steering's angle onto the x87's stack -- 0 while the race is
 * not under way ([+0x164]), else the steering [+0x51c] as a float, cut down:
 * with the handbrake on ([+0x50d]) while the car moves, by 0.x the faster it
 * goes (no less than 0.55) where the slip [+0x5b0] and the steering lean the
 * same way; without it past 26.7 and past 13.4 forward, by the slip's size
 * where the slip is past 0.05 or 0.15 or the steering against it; past 40 by
 * the speed's share (1 to 1.5); then by the constants and the handling's
 * lock [+0x23c].  What it clamps to -1..1 ([ebp - 0x40]) is never used: the
 * unclamped angle is what it gives back, as the code does. */
void steerAngle(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x40);
    const reg32 self = cpu.ebp - 0x14;
    put(app, self, cpu.eax);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, self);
    if (half(app, cpu.eax + 0x164) != 0)
    {
        put(app, cpu.ebp - 0x10, 0);
        fpu.push(f32(app, cpu.ebp - 0x10));
        return leave(app, cpu);
    }
    static_cast<void>(at(app, kUnusedFlag));  // cmp dword [0x6fd494], 0: its flags never used
    cpu.eax = at(app, self);
    fpu.push(x86::Float(x86::sreg32(at(app, cpu.eax + 0x51c))));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 4));
    fstp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, self);
    bool handbrake = false;
    if (byte(app, cpu.eax + 0x50d) != 0)
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x18, self);
        zeroCompare(cpu, f32(app, cpu.ebp - 0x18));
        handbrake = below(cpu);
    }
    if (handbrake)
    {
        // 0x4ad37d: the slip and the steering the same way?
        bool cut = false;
        cpu.eax = at(app, self);
        zeroCompare(cpu, f32(app, cpu.eax + 0x5b0));
        if (above(cpu))
        {
            zeroCompare(cpu, f32(app, cpu.ebp - 0xc));
            cut = below(cpu);
        }
        if (!cut)
        {
            cpu.eax = at(app, self);
            zeroCompare(cpu, f32(app, cpu.eax + 0x5b0));
            if (below(cpu))
            {
                zeroCompare(cpu, f32(app, cpu.ebp - 0xc));
                cut = above(cpu);
            }
        }
        if (cut)
        {
            cpu.eax = at(app, self);
            magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x20, self);
            fpu.push(f32(app, cpu.ebp - 0x20));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de34));
            fpu.st(0) = fp::sub(f32(app, 0x53de38), fpu.st(0));
            fstp(app, cpu, cpu.ebp - 8);
            if (i32(at(app, cpu.ebp - 8)) > i32(0x3f0ccccd))
            {
                cpu.eax = at(app, cpu.ebp - 8);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
            else
                put(app, cpu.ebp - 0x1c, 0x3f0ccccd);
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            fstp(app, cpu, cpu.ebp - 0xc);
        }
    }
    else
    {
        // 0x4ad417: past 26.7 forward, or past 13.4, the slip's own limits.
        cpu.eax = at(app, self);
        reg32 slipOver = 0, slipBelow = 0, slipScale = 0, local = 0;
        bool fast = true;
        if (i32(at(app, cpu.eax + 0x4f4)) > i32(0x41d5999a))
        {
            slipOver = 0x3d4ccccd;
            slipBelow = 0x53de3c;
            slipScale = 0x53de44;
            local = cpu.ebp - 0x24;
        }
        else
        {
            cpu.eax = at(app, self);
            if (i32(at(app, cpu.eax + 0x4f4)) > i32(0x41566666))
            {
                slipOver = 0x3e19999a;
                slipBelow = 0x53de4c;
                slipScale = 0x53de54;
                local = cpu.ebp - 0x28;
            }
            else
                fast = false;
        }
        if (fast)
        {
            bool cut = false;
            cpu.eax = at(app, self);
            if (i32(at(app, cpu.eax + 0x5b0)) > i32(slipOver))
            {
                zeroCompare(cpu, f32(app, cpu.ebp - 0xc));
                cut = below(cpu);
            }
            if (!cut)
            {
                cpu.eax = at(app, self);
                fpu.push(f32(app, cpu.eax + 0x5b0));
                fcomp(cpu, f64(app, slipBelow));
                if (below(cpu))
                {
                    zeroCompare(cpu, f32(app, cpu.ebp - 0xc));
                    cut = above(cpu);
                }
            }
            if (cut)
            {
                cpu.eax = at(app, self);
                magnitude(app, cpu, cpu.eax + 0x5b0, local, self);
                fpu.push(f32(app, local));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, slipScale));
                fstp(app, cpu, cpu.ebp - 8);
                if (i32(at(app, cpu.ebp - 8)) < i32(0x3f800000))
                {
                    fpu.push(f32(app, cpu.ebp - 0xc));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
                    fstp(app, cpu, cpu.ebp - 0xc);
                }
            }
        }
    }
    // 0x4ad548: past 40, by the speed's share, 1 to 1.5.
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x2c, self);
    if (i32(at(app, cpu.ebp - 0x2c)) > i32(0x42200000))
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x38, self);
        fpu.push(f32(app, cpu.ebp - 0x38));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53de5c));
        fstp(app, cpu, cpu.ebp - 8);
        if (i32(at(app, cpu.ebp - 8)) > i32(0x3f800000))
        {
            cpu.eax = at(app, cpu.ebp - 8);
            put(app, cpu.ebp - 0x34, cpu.eax);
        }
        else
            put(app, cpu.ebp - 0x34, 0x3f800000);
        cpu.eax = at(app, cpu.ebp - 0x34);
        put(app, cpu.ebp - 8, cpu.eax);
        if (i32(at(app, cpu.ebp - 8)) > i32(0x3fc00000))
            put(app, cpu.ebp - 0x30, 0x3fc00000);
        else
        {
            cpu.eax = at(app, cpu.ebp - 8);
            put(app, cpu.ebp - 0x30, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x30);
        put(app, cpu.ebp - 8, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 8));
        fstp(app, cpu, cpu.ebp - 0xc);
    }
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de64));
    fstp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x23c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de68));
    fpu.st(0) = fp::div(fpu.st(0), f32(app, 0x53de6c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    fstp(app, cpu, cpu.ebp - 0xc);
    fpu.push(f32(app, cpu.ebp - 0xc));
    fcomp(cpu, f64(app, 0x53de74));
    if (below(cpu))
        put(app, cpu.ebp - 0x40, 0xbf800000);
    else
    {
        if (i32(at(app, cpu.ebp - 0xc)) > i32(0x3f800000))
            put(app, cpu.ebp - 0x3c, 0x3f800000);
        else
        {
            cpu.eax = at(app, cpu.ebp - 0xc);
            put(app, cpu.ebp - 0x3c, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x3c);
        put(app, cpu.ebp - 0x40, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.ebp - 0x10, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x10));
    leave(app, cpu);
}

/* 0x4ad672: the car's slip [+0x5b0] -- its speed across [+0x4ec] over its
 * speed forward [+0x4f4] -- where it goes faster than 5 either way, else 0. */
void sideSlip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0xc);
    const reg32 self = cpu.ebp - 0xc;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 4, self);
    if (i32(at(app, cpu.ebp - 4)) > i32(0x40a00000))
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0x4ec));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.eax + 0x4f4));
        cpu.eax = at(app, self);
        fstp(app, cpu, cpu.eax + 0x5b0);
    }
    else
    {
        cpu.eax = at(app, self);
        put(app, cpu.eax + 0x5b0, 0);
    }
    leave(app, cpu);
}

/* 0x4ad6eb: the car's speed on the ground [+0x5a4] -- the game's quick
 * length of its speeds across and forward (0x4967e0, of their sizes) --
 * negative (its sign bit flipped) where it goes backwards. */
void groundSpeed(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x18);
    const reg32 self = cpu.ebp - 0x18;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 8, self);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 4, self);
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.ebp - 0xc, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0xc));
    cpu.esp -= 4;
    fstp(app, cpu, cpu.esp);
    fpu.push(f32(app, cpu.ebp - 0x10));
    cpu.esp -= 4;
    fstp(app, cpu, cpu.esp);
    if (!byAddress(app, cpu, kPlanarLength))
        return;
    cpu.eax = at(app, self);
    fstp(app, cpu, cpu.eax + 0x5a4);
    cpu.eax = at(app, self);
    zeroCompare(cpu, f32(app, cpu.eax + 0x4f4));
    if (above(cpu))
    {
        cpu.eax = at(app, self);
        putByte(app, cpu.eax + 0x5a7, reg8(byte(app, cpu.eax + 0x5a7) ^ 0x80));
    }
    leave(app, cpu);
}

/* 0x4ad7a3: the car's full step.  Its axes transposed into the frame
 * (0x4e0110); the force in its axes and the four wheels' records cleared;
 * its surface's grip; its speed and spin in its own axes (the speed from
 * the countdown's end on, the spin while racing); its slip and ground speed;
 * the air's drag on each axis; the spin's torque; gravity in its axes
 * (kGravity); the front wheels' sideways forces from the drag and torque
 * (none while not racing); the driver's inputs (0x4a9bc4), the steering's
 * angle and the idle's damping; the engine (0x4aaed1) shared out to the
 * wheels by the handling's split.  Where the race is not under way, the car
 * was just put back or is to be, or it stands too lightly ([+0x134]), its
 * loads and step cleared and done.  Else the brakes ([+0x5ec], by the
 * handling's, held to the speed, against it, the handbrake's at the back)
 * on each wheel; the wheels' loads (0x4aab41) and grips under gravity -- none
 * on the ground, done; the load's shift and each wheel's force (0x4aca4f);
 * the forces scaled by the wheels' share, summed into the car's force, the
 * drive's; its acceleration and speed; held still, its grip sideways and the
 * handbrake's drag on its speed in its axes, back to the world's; the yaw's
 * torque from the wheels' forces, the spin and its damping, back too; and
 * the coasting's damping. */
void physicsStep(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x154);
    const reg32 self = cpu.ebp - 0x24;
    const reg32 handling = cpu.ebp - 0x10;
    const reg32 axes = cpu.ebp - 0x8c;
    const reg32 wheels = cpu.ebp - 0x14c;
    put(app, self, cpu.eax);
    for (const reg32 local : {0x14u, 0x28u, 0x2cu, 0x1cu, 0x18u, 0x10u, 0xcu, 8u, 4u})
        put(app, cpu.ebp - local, 0);
    cpu.edx = axes;
    cpu.eax = at(app, self);
    cpu.eax += 0x190;
    if (!byAddress(app, cpu, kTranspose))
        return;
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x528);
    put(app, handling, cpu.eax);
    for (const reg32 offset : {0x4e0u, 0x4e4u, 0x4e8u})
    {
        cpu.eax = at(app, self);
        put(app, cpu.eax + offset, 0);
    }
    for (const reg32 local : {0x40u, 0x3cu, 0x38u, 0x4cu, 0x48u, 0x44u, 0x58u, 0x54u, 0x50u})
        put(app, cpu.ebp - local, 0);
    for (put(app, cpu.ebp - 4, 0); i32(at(app, cpu.ebp - 4)) < 4;)
    {
        cpu.edx = at(app, cpu.ebp - 4) * 0x30;
        cpu.eax = wheels;
        cpu.eax += cpu.edx;
        if (!direct(app, cpu, clearWheelForces))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1);
    }
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x18c);
    cpu.eax <<= 2;
    fpu.push(f32(app, cpu.eax + kSurfaceGrip));
    cpu.eax = at(app, self);
    fstp(app, cpu, cpu.eax + 0x20c);
    if (i32(at(app, kRaceFrame)) >= 0x200)
    {
        cpu.ebx = at(app, self);
        cpu.ebx += 0x4ec;
        cpu.edx = axes;
        cpu.eax = at(app, self);
        cpu.eax += 0xa4;
        transform(app, cpu);
    }
    else
        for (const reg32 offset : {0x4ecu, 0x4f0u, 0x4f4u})
        {
            cpu.eax = at(app, self);
            put(app, cpu.eax + offset, 0);
        }
    cpu.eax = at(app, self);
    if (half(app, cpu.eax + 0x164) == 0)
    {
        cpu.ebx = at(app, self);
        cpu.ebx += 0x4f8;
        cpu.edx = axes;
        cpu.eax = at(app, self);
        cpu.eax += 0xe4;
        transform(app, cpu);
    }
    cpu.eax = at(app, self);
    if (!direct(app, cpu, sideSlip))
        return;
    cpu.eax = at(app, self);
    if (!direct(app, cpu, groundSpeed))
        return;
    // The air's drag on each axis.
    for (const auto& axis : {std::pair<reg32, reg32>{0x4ec, 0x58}, {0x4f0, 0x54}, {0x4f4, 0x50}})
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + axis.first));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de7c));
        fpu.st(0) = -fpu.st(0);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de80));
        fstp(app, cpu, cpu.ebp - axis.second);
    }
    cpu.eax = at(app, self);
    if (!direct(app, cpu, spinTorque))
        return;
    fstp(app, cpu, cpu.ebp - 0x14);
    // Gravity, (0, -9.81, 0), in the car's axes.
    put(app, cpu.ebp - 0x4c, 0);
    put(app, cpu.ebp - 0x48, 0xc11cf5c3);
    put(app, cpu.ebp - 0x44, 0);
    cpu.ebx = kGravity;
    cpu.edx = axes;
    cpu.eax = cpu.ebp - 0x4c;
    transform(app, cpu);
    cpu.eax = at(app, self);
    if (half(app, cpu.eax + 0x164) != 0)
    {
        for (const auto& slot : {std::pair<reg32, reg32>{0x10c, 0x13c}, {0x108, 0x138}, {0x104, 0x134},
                                 {0xdc, 0xac}, {0xd8, 0xa8}, {0xd4, 0xa4}})
        {
            put(app, cpu.ebp - slot.first, 0);
            cpu.eax = at(app, cpu.ebp - slot.first);
            put(app, cpu.ebp - slot.second, cpu.eax);
        }
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0x58));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x14));
        fstp(app, cpu, cpu.ebp - 0x10c);
        cpu.eax = at(app, cpu.ebp - 0x10c);
        put(app, cpu.ebp - 0x13c, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x54);
        put(app, cpu.ebp - 0x108, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x108);
        put(app, cpu.ebp - 0x138, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x50);
        put(app, cpu.ebp - 0x104, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x104);
        put(app, cpu.ebp - 0x134, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 0x58));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x14));
        fstp(app, cpu, cpu.ebp - 0xdc);
        cpu.eax = at(app, cpu.ebp - 0xdc);
        put(app, cpu.ebp - 0xac, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x54);
        put(app, cpu.ebp - 0xd8, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0xd8);
        put(app, cpu.ebp - 0xa8, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x50);
        put(app, cpu.ebp - 0xd4, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0xd4);
        put(app, cpu.ebp - 0xa4, cpu.eax);
    }
    cpu.eax = at(app, self);
    if (!byAddress(app, cpu, kDriverInputs))
        return;
    cpu.eax = at(app, self);
    if (!direct(app, cpu, steerAngle))
        return;
    fstp(app, cpu, cpu.ebp - 0xc);
    cpu.eax = at(app, self);
    if (!direct(app, cpu, idleDamping))
        return;
    put(app, cpu.ebp - 0xf4, 1);
    cpu.eax = at(app, cpu.ebp - 0xf4);
    put(app, cpu.ebp - 0x124, cpu.eax);
    put(app, cpu.ebp - 0xc4, 0);
    cpu.eax = at(app, cpu.ebp - 0xc4);
    put(app, cpu.ebp - 0x94, cpu.eax);
    cpu.eax = at(app, self);
    if (!byAddress(app, cpu, kEngine))
        return;
    fstp(app, cpu, cpu.ebp - 0x1c);
    // The engine's force, front and back by the handling's split.
    cpu.eax = at(app, handling);
    fpu.push(f32(app, cpu.ebp - 0x1c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1d0));
    fstp(app, cpu, cpu.ebp - 0x11c);
    cpu.eax = at(app, cpu.ebp - 0x11c);
    put(app, cpu.ebp - 0x14c, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x1c));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x14c));
    fstp(app, cpu, cpu.ebp - 0xec);
    cpu.eax = at(app, cpu.ebp - 0xec);
    put(app, cpu.ebp - 0xbc, cpu.eax);
    cpu.eax = at(app, self);
    bool stopped = true;
    if (half(app, cpu.eax + 0x164) == 0)
    {
        cpu.eax = at(app, self);
        if (at(app, cpu.eax + 0x524) == 0)
        {
            cpu.eax = at(app, self);
            if (at(app, cpu.eax + 0x184) == 0)
            {
                cpu.eax = at(app, self);
                fpu.push(f32(app, cpu.eax + 0x134));
                fcomp(cpu, f64(app, 0x53de84));
                stopped = below(cpu);
            }
        }
    }
    if (stopped)
    {
        // 0x4adc88: its loads and the step's acceleration cleared.
        cpu.eax = at(app, self);
        put(app, cpu.eax + 0x5c4, 0);
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0x5c4));
        cpu.eax = at(app, self);
        fstp(app, cpu, cpu.eax + 0x5c0);
        for (const reg32 offset : {0x2f4u, 0x2f8u, 0x2fcu, 0x2e8u})
        {
            cpu.eax = at(app, self);
            put(app, cpu.eax + offset, 0);
        }
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0x2e8));
        cpu.eax = at(app, self);
        fstp(app, cpu, cpu.eax + 0x2f0);
        return leave(app, cpu);
    }
    // 0x4adcf2: the brakes, held to what stops the car this step, against
    // its way.
    cpu.eax = at(app, self);
    put(app, cpu.eax + 0x2e4, 0);
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0x5ec));
    cpu.eax = at(app, handling);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1d8));
    fstp(app, cpu, cpu.ebp - 0x28);
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0x4f4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de8c));
    fpu.push(0.0);
    fcompp(cpu);
    const bool backwards = above(cpu);
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0x4f4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de8c));
    if (backwards)
        fpu.st(0) = -fpu.st(0);
    fstp(app, cpu, cpu.ebp - 0x154);
    cpu.eax = at(app, cpu.ebp - 0x154);
    put(app, cpu.ebp - 0x18, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x28));
    fcomp(cpu, f32(app, cpu.ebp - 0x18));
    if (above(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x18);
        put(app, cpu.ebp - 0x150, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.ebp - 0x150, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 0x150);
    put(app, cpu.ebp - 0x28, cpu.eax);
    cpu.eax = at(app, self);
    zeroCompare(cpu, f32(app, cpu.eax + 0x4f4));
    if (below(cpu))
        putByte(app, cpu.ebp - 0x25, reg8(byte(app, cpu.ebp - 0x25) ^ 0x80));
    // The back wheels' share: with the handbrake, more.
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x50d) != 0)
    {
        cpu.eax = at(app, handling);
        fpu.push(f32(app, cpu.eax + 0x1dc));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53de90));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x28));
        fstp(app, cpu, cpu.ebp - 0x2c);
    }
    else
    {
        cpu.eax = at(app, handling);
        fpu.push(f32(app, cpu.ebp - 0x28));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x1dc));
        fstp(app, cpu, cpu.ebp - 0x2c);
    }
    for (const reg32 local : {0x14cu, 0x11cu})
    {
        fpu.push(f32(app, cpu.ebp - local));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x2c));
        fstp(app, cpu, cpu.ebp - local);
    }
    for (const reg32 local : {0xbcu, 0xecu})
    {
        fpu.push(f32(app, cpu.ebp - 0x28));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x2c));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - local));
        fstp(app, cpu, cpu.ebp - local);
    }
    cpu.eax = at(app, self);
    if (!byAddress(app, cpu, kWheelLoads))
        return;
    // Each wheel's grip under gravity: its load's.
    for (put(app, cpu.ebp - 4, 0); i32(at(app, cpu.ebp - 4)) < 4;)
    {
        fpu.push(f32(app, kGravityY));
        fpu.st(0) = -fpu.st(0);
        cpu.eax = at(app, handling);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x248));
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax <<= 2;
        fstp(app, cpu, cpu.eax + reg32(-0x68) + cpu.ebp);
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax <<= 2;
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.edx <<= 2;
        cpu.edx += at(app, self);
        fpu.push(f32(app, cpu.edx + 0x5d8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + reg32(-0x68) + cpu.ebp));
        fstp(app, cpu, cpu.eax + reg32(-0x68) + cpu.ebp);
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1);
    }
    // None of the four on the ground: done.
    bool grounded = false;
    for (const reg32 local : {0x68u, 0x64u, 0x60u, 0x5cu})
    {
        zeroCompare(cpu, f32(app, cpu.ebp - local));
        if (below(cpu))
        {
            grounded = true;
            break;
        }
    }
    if (!grounded)
        return leave(app, cpu);
    cpu.eax = at(app, handling);
    fpu.push(f32(app, cpu.ebp - 0x68));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x234));
    fstp(app, cpu, cpu.ebp - 0x34);
    cpu.eax = at(app, handling);
    fpu.push(f32(app, cpu.ebp - 0x64));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x234));
    fstp(app, cpu, cpu.ebp - 0x30);
    cpu.eax = at(app, handling);
    fpu.push(1.0);
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x234));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x5c));
    fstp(app, cpu, cpu.ebp - 0x28);
    cpu.eax = at(app, handling);
    fpu.push(1.0);
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x234));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x60));
    fstp(app, cpu, cpu.ebp - 0x2c);
    cpu.eax = at(app, self);
    if (!direct(app, cpu, settleLoadShift))
        return;
    // The wheels' loads with the shift: front less it, back with it.
    for (const auto& wheel : {std::pair<reg32, reg32>{0x34, 0x148}, {0x30, 0x118}})
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.ebp - wheel.first));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x5bc));
        fstp(app, cpu, cpu.ebp - wheel.second);
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.ebp - wheel.second));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + (wheel.first == 0x34 ? 0x5c8 : 0x5cc)));
        fstp(app, cpu, cpu.ebp - wheel.second);
    }
    for (const auto& wheel : {std::pair<reg32, reg32>{0x5c, 0xb8}, {0x60, 0xe8}})
    {
        const reg32 low = wheel.first == 0x5c ? 0x28 : 0x2c;
        fpu.push(f32(app, cpu.ebp - wheel.first));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - low));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x5bc));
        fstp(app, cpu, cpu.ebp - wheel.second);
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.ebp - wheel.second));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + (wheel.first == 0x5c ? 0x5d4 : 0x5d0)));
        fstp(app, cpu, cpu.ebp - wheel.second);
    }
    put(app, cpu.ebp - 0xe0, 0);
    cpu.eax = at(app, cpu.ebp - 0xe0);
    put(app, cpu.ebp - 0xb0, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.ebp - 0x110, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x110);
    put(app, cpu.ebp - 0x140, cpu.eax);
    cpu.eax = at(app, self);
    put(app, cpu.eax + 0x59c, 0);
    // Each wheel's force (0x4aca4f), its grip at +8 of its record.
    for (put(app, cpu.ebp - 4, 0); i32(at(app, cpu.ebp - 4)) < 4;)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax <<= 2;
        cpu.eax += at(app, self);
        cpu.edx = at(app, cpu.ebp - 4) * 0x30;
        fpu.push(f32(app, cpu.eax + 0x5c8));
        fstp(app, cpu, cpu.edx + reg32(-0x144) + cpu.ebp);
        cpu.eax = at(app, cpu.ebp - 4) * 0x30;
        cpu.edx = wheels;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, self);
        if (!byAddress(app, cpu, kWheelForces))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1);
    }
    // Their forces by the wheels' share.
    for (put(app, cpu.ebp - 4, 0); i32(at(app, cpu.ebp - 4)) < 4;)
    {
        for (const reg32 local : {0x130u, 0x128u})
        {
            cpu.eax = at(app, cpu.ebp - 4) * 0x30;
            cpu.edx = at(app, self);
            fpu.push(f32(app, cpu.edx + 0x1a0));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + reg32(0) - local + cpu.ebp));
            fstp(app, cpu, cpu.eax + reg32(0) - local + cpu.ebp);
        }
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1);
    }
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x1a0);
    put(app, cpu.ebp - 8, cpu.eax);
    for (const reg32 offset : {0x5c0u, 0x5c4u})
    {
        fpu.push(f32(app, 0x53de94));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 8));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + offset));
        fstp(app, cpu, cpu.eax + offset);
    }
    // The car's force: the four wheels' forward (+0x24) and across (+0x1c).
    for (const auto& sum : {std::pair<reg32, reg32>{0x128, 0x4e8}, {0x130, 0x4e0}})
    {
        const reg32 first = sum.first, second = sum.first - 0x30, third = sum.first - 0x90, fourth = sum.first - 0x60;
        fpu.push(f32(app, cpu.ebp - first));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - second));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de98));
        fpu.push(f32(app, cpu.ebp - third));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - fourth));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de98));
        cpu.eax = at(app, self);
        fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
        fpu.pop();
        fstp(app, cpu, cpu.eax + sum.second);
    }
    cpu.eax = at(app, self);
    if (!direct(app, cpu, driveForce))
        return;
    fpu.push(f32(app, kGravityZ));
    fpu.st(0) = -fpu.st(0);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de9c));
    fstp(app, cpu, cpu.ebp - 0x18);
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0x4e8));
    cpu.eax = at(app, handling);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x254));
    cpu.eax = at(app, self);
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x18));
    fstp(app, cpu, cpu.eax + 0x5bc);
    // The acceleration in the world's axes, onto the speed.
    cpu.ebx = at(app, self);
    cpu.ebx += 0x2e8;
    cpu.edx = at(app, self);
    cpu.edx += 0x190;
    cpu.eax = at(app, self);
    cpu.eax += 0x4e0;
    transform(app, cpu);
    for (const auto& axis : {std::pair<reg32, reg32>{0x2e8, 0xa4}, {0x2ec, 0xa8}, {0x2f0, 0xac}})
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + axis.first));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dea0));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + axis.second));
        fstp(app, cpu, cpu.eax + axis.second);
    }
    cpu.eax = at(app, self);
    if (!direct(app, cpu, holdStill))
        return;
    cpu.ebx = at(app, self);
    cpu.ebx += 0x4ec;
    cpu.edx = axes;
    cpu.eax = at(app, self);
    cpu.eax += 0xa4;
    transform(app, cpu);
    cpu.eax = at(app, self);
    if (!direct(app, cpu, sideGrip))
        return;
    cpu.eax = at(app, self);
    if (!direct(app, cpu, handbrakeDrag))
        return;
    cpu.ebx = at(app, self);
    cpu.ebx += 0xa4;
    cpu.edx = at(app, self);
    cpu.edx += 0x190;
    cpu.eax = at(app, self);
    cpu.eax += 0x4ec;
    transform(app, cpu);
    // The yaw's torque: the front wheels' sideways forces less the back's.
    fpu.push(f32(app, cpu.ebp - 0x130));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x100));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de98));
    fpu.push(f32(app, cpu.ebp - 0xa0));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0xd0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53de98));
    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
    fpu.pop();
    fstp(app, cpu, cpu.ebp - 0x3c);
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0xb0));
    cpu.eax = at(app, self);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x100));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dea0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x3c));
    fstp(app, cpu, cpu.ebp - 0x3c);
    put(app, cpu.ebp - 0x40, 0);
    put(app, cpu.ebp - 0x38, 0);
    cpu.ebx = at(app, self);
    cpu.ebx += 0x2f4;
    cpu.edx = at(app, self);
    cpu.edx += 0x190;
    cpu.eax = cpu.ebp - 0x40;
    transform(app, cpu);
    for (const auto& axis : {std::pair<reg32, reg32>{0x2f4, 0xe4}, {0x2f8, 0xe8}, {0x2fc, 0xec}})
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + axis.first));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dea0));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + axis.second));
        fstp(app, cpu, cpu.eax + axis.second);
    }
    cpu.ebx = at(app, self);
    cpu.ebx += 0x4f8;
    cpu.edx = axes;
    cpu.eax = at(app, self);
    cpu.eax += 0xe4;
    transform(app, cpu);
    cpu.eax = at(app, self);
    if (!direct(app, cpu, yawDamping))
        return;
    cpu.ebx = at(app, self);
    cpu.ebx += 0xe4;
    cpu.edx = at(app, self);
    cpu.edx += 0x190;
    cpu.eax = at(app, self);
    cpu.eax += 0x4f8;
    transform(app, cpu);
    cpu.eax = at(app, self);
    if (!direct(app, cpu, coastDamping))
        return;
    leave(app, cpu);
}

/* 0x4ae4c1: a car's physics, each frame.  A car the AI drives ([+0x200]
 * flag 2): the AI's step (0x4088f0).  Else, off the ground ([+0x8a]) or
 * standing lightly ([+0x10c] under 0.9): its wheels' flag, slip and loads
 * cleared, the revs down 500 towards the handling's idle ([+0x1c0]), and the
 * car at rest (0x4aa8e7); or the full step.  Then its sounds (0x4cdf10)
 * unless [+0x5f4] or in neutral ([+0x50b] 0). */
void carPhysics(App* app, CPU& cpu)
{
    enter(app, cpu, 8);
    const reg32 self = cpu.ebp - 8;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    if ((byte(app, cpu.eax + 0x200) & 2) != 0)
    {
        cpu.eax = at(app, self);
        if (!byAddress(app, cpu, kAiStep))
            return;
        return leave(app, cpu);
    }
    cpu.eax = at(app, self);
    bool rest = true;
    if (byte(app, cpu.eax + 0x8a) == 0)
    {
        cpu.eax = at(app, self);
        rest = i32(at(app, cpu.eax + 0x10c)) < i32(0x3f666666);
    }
    if (rest)
    {
        for (const reg32 offset : {0x594u, 0x5b0u, 0x5c0u, 0x5c4u})
        {
            cpu.eax = at(app, self);
            put(app, cpu.eax + offset, 0);
        }
        cpu.eax = at(app, self);
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.edx = at(app, cpu.eax + 0x1c0);
        cpu.edx += 0x1f4;
        cpu.eax = at(app, self);
        if (i32(cpu.edx) < i32(at(app, cpu.eax + 0x590)))
        {
            cpu.eax = at(app, self);
            put(app, cpu.eax + 0x590, at(app, cpu.eax + 0x590) - 0x1f4);
        }
        else
        {
            cpu.eax = at(app, self);
            cpu.eax = at(app, cpu.eax + 0x528);
            cpu.edx = at(app, cpu.eax + 0x1c0);
            cpu.eax = at(app, self);
            put(app, cpu.eax + 0x590, cpu.edx);
        }
        cpu.eax = at(app, self);
        if (!byAddress(app, cpu, kCarAtRest))
            return;
    }
    else
    {
        cpu.eax = at(app, self);
        if (!direct(app, cpu, physicsStep))
            return;
    }
    cpu.eax = at(app, self);
    if (at(app, cpu.eax + 0x5f4) == 0)
    {
        cpu.eax = at(app, self);
        if (byte(app, cpu.eax + 0x50b) != 0)
        {
            cpu.eax = at(app, self);
            if (!byAddress(app, cpu, kCarSounds))
                return;
        }
    }
    if (diag::carStepped)
        diag::carStepped(app, at(app, self));
    leave(app, cpu);
}

/* 0x4ae5c2: the load's shift [+0x5bc] settling: a shift back (under 0)
 * times 0x53dea4, and 0x53dea8 more on the damped tracks ([+0x160] 1 or 2);
 * a shift forward times 0x53deac with the wheels' flag [+0x594] 1, and
 * 0x53deb0 more on those tracks. */
void settleLoadShift(App* app, CPU& cpu)
{
    enter(app, cpu, 8);
    const reg32 self = cpu.ebp - 8;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    zeroCompare(cpu, f32(app, cpu.eax + 0x5bc));
    const auto damped = [&] {
        cpu.eax = at(app, self);
        if (at(app, cpu.eax + 0x160) == 1)
            return true;
        cpu.eax = at(app, self);
        return at(app, cpu.eax + 0x160) == 2;
    };
    if (above(cpu))
    {
        scaleField(app, cpu, self, 0x5bc, 0x53dea4);
        if (damped())
            scaleField(app, cpu, self, 0x5bc, 0x53dea8);
        return leave(app, cpu);
    }
    cpu.eax = at(app, self);
    zeroCompare(cpu, f32(app, cpu.eax + 0x5bc));
    if (!below(cpu))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    if (at(app, cpu.eax + 0x594) == 1)
        scaleField(app, cpu, self, 0x5bc, 0x53deac);
    if (damped())
        scaleField(app, cpu, self, 0x5bc, 0x53deb0);
    leave(app, cpu);
}

/* 0x4ae68e: the drive's force [+0x4e8] over the handling's mass [+0x248];
 * off the damped tracks, in a single race on one screen, more by the
 * choice's tuning (its +0x58: past 1 in the low gears, under 1 always);
 * [+0x4e4] cleared. */
void driveForce(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0xc);
    const reg32 self = cpu.ebp - 0xc;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x528);
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 0x248));
    cpu.eax = at(app, self);
    fpu.st(0) = fp::div(f32(app, cpu.eax + 0x4e8), fpu.st(0));
    fstp(app, cpu, cpu.eax + 0x4e8);
    cpu.eax = at(app, self);
    bool tuned = false;
    if (at(app, cpu.eax + 0x160) != 1)
    {
        cpu.eax = at(app, self);
        if (at(app, cpu.eax + 0x160) != 2 && i32(at(app, kPlayMode)) <= 1 && at(app, kRaceType) != 1
            && at(app, kRaceType) != 2)
            tuned = true;
    }
    if (tuned)
    {
        cpu.eax = at(app, self);
        cpu.eax = at(app, cpu.eax + 0x220);
        bool high = false;
        if (i32(at(app, cpu.eax + 0x58)) > i32(0x3f800000))
        {
            cpu.eax = at(app, self);
            high = byte(app, cpu.eax + 0x50b) <= 3;
        }
        if (high)
        {
            cpu.eax = at(app, self);
            cpu.eax = at(app, cpu.eax + 0x220);
            fpu.push(f32(app, cpu.eax + 0x58));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53deb4));
            fpu.push(1.0);
            fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
            fpu.pop();
            cpu.eax = at(app, self);
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x4e8));
            fstp(app, cpu, cpu.eax + 0x4e8);
        }
        else
        {
            cpu.eax = at(app, self);
            cpu.eax = at(app, cpu.eax + 0x220);
            if (i32(at(app, cpu.eax + 0x58)) < i32(0x3f800000))
            {
                cpu.eax = at(app, self);
                cpu.eax = at(app, cpu.eax + 0x220);
                fpu.push(f32(app, cpu.eax + 0x58));
                fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53deb8));
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53debc));
                fpu.push(1.0);
                fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
                fpu.pop();
                cpu.eax = at(app, self);
                fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x4e8));
                fstp(app, cpu, cpu.eax + 0x4e8);
            }
        }
    }
    cpu.eax = at(app, self);
    put(app, cpu.eax + 0x4e4, 0);
    leave(app, cpu);
}

/* 0x4ae79b: the car held still: both pedals down ([+0x5f0] at least 0.5,
 * [+0x5ec] at least 0.75), the steering past 64 either way, and no faster
 * than 5 -- its speed scaled by 0 (0x4e0050). */
void holdStill(App* app, CPU& cpu)
{
    enter(app, cpu, 0x10);
    const reg32 self = cpu.ebp - 0x10;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    if (i32(at(app, cpu.eax + 0x5f0)) < i32(0x3f000000))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    if (i32(at(app, cpu.eax + 0x5ec)) < i32(0x3f400000))
        return leave(app, cpu);
    integerMagnitude(app, cpu, 0x51c, cpu.ebp - 8, self);
    if (i32(at(app, cpu.ebp - 8)) <= 0x40)
        return leave(app, cpu);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 4, self);
    if (i32(at(app, cpu.ebp - 4)) > i32(0x40a00000))
        return leave(app, cpu);
    if (!scaleVector(app, cpu, self, 0xa4, 0))
        return;
    leave(app, cpu);
}

/* 0x4ae85b: the car's grip sideways: its speed across [+0x4ec] scaled by 0.9
 * up to 1, 0.99 past 2, and in between by its size (0x53dec0, 0x53dec4,
 * 0x53dec8 -- what it clamps to 0.9..0.99 at [ebp - 0x1c] never used, as the
 * code does). */
void sideGrip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x20);
    const reg32 self = cpu.ebp - 0x10;
    put(app, self, cpu.eax);
    put(app, cpu.ebp - 8, 0);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 4, self);
    if (i32(at(app, cpu.ebp - 4)) <= i32(0x3f800000))
        put(app, cpu.ebp - 8, 0x3f666666);
    else
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x14, self);
        if (i32(at(app, cpu.ebp - 0x14)) > i32(0x40000000))
            put(app, cpu.ebp - 8, 0x3f7d70a4);
        else
        {
            cpu.eax = at(app, self);
            magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x20, self);
            fpu.push(f32(app, cpu.ebp - 0x20));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dec0));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53dec4));
            fstp(app, cpu, cpu.ebp - 8);
            fpu.push(f32(app, cpu.ebp - 8));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53dec8));
            fstp(app, cpu, cpu.ebp - 8);
            if (i32(at(app, cpu.ebp - 8)) < i32(0x3f666666))
                put(app, cpu.ebp - 0x1c, 0x3f666666);
            else
            {
                if (i32(at(app, cpu.ebp - 8)) > i32(0x3f7d70a4))
                    put(app, cpu.ebp - 0x18, 0x3f7d70a4);
                else
                {
                    cpu.eax = at(app, cpu.ebp - 8);
                    put(app, cpu.ebp - 0x18, cpu.eax);
                }
                cpu.eax = at(app, cpu.ebp - 0x18);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
        }
    }
    cpu.eax = at(app, self);
    fpu.push(f32(app, cpu.eax + 0x4ec));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    fstp(app, cpu, cpu.eax + 0x4ec);
    leave(app, cpu);
}

/* 0x4ae98b: the handbrake's drag ([+0x50d]) on the speed forward [+0x4f4]:
 * past 1, times 0x53decc on the damped tracks or 0x53ded0; under 1 and in
 * neutral ([+0x50b] 1), times 0x53ded4. */
void handbrakeDrag(App* app, CPU& cpu)
{
    enter(app, cpu, 0x10);
    const reg32 self = cpu.ebp - 8;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x50d) == 0)
        return leave(app, cpu);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0xc, self);
    if (i32(at(app, cpu.ebp - 0xc)) > i32(0x3f800000))
    {
        cpu.eax = at(app, self);
        bool damped = at(app, cpu.eax + 0x160) == 1;
        if (!damped)
        {
            cpu.eax = at(app, self);
            damped = at(app, cpu.eax + 0x160) == 2;
        }
        scaleField(app, cpu, self, 0x4f4, damped ? 0x53decc : 0x53ded0);
        return leave(app, cpu);
    }
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x10, self);
    if (i32(at(app, cpu.ebp - 0x10)) >= i32(0x3f800000))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x50b) == 1)
        scaleField(app, cpu, self, 0x4f4, 0x53ded4);
    leave(app, cpu);
}

/* 0x4aea88: the yaw's damping, the spin in the car's axes [+0x4fc]: held
 * still (both pedals down, steering past 64, no faster than 5) it stops;
 * with the steering under 4 either way, out of neutral, times 0x53ded8.
 * Slower across than forward: under the spin's own speed (by the handling's
 * [+0x258]) forward and across, by the speed's share of it (no more than
 * 0.98); else by the slip, 0x53def0 or 0x53def4.  Faster across, and the
 * speed in its axes over 0x53defc long (0x4e06a0): the same by 0x53df04 and
 * 0x53df0c, and the speed scaled (0x4e0050) by 0.7 under 5, 0.8 past. */
void yawDamping(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x68);
    const reg32 self = cpu.ebp - 0x10;
    put(app, self, cpu.eax);
    // As holdStill's: its spin cleared.
    bool held = false;
    cpu.eax = at(app, self);
    if (i32(at(app, cpu.eax + 0x5f0)) >= i32(0x3f000000))
    {
        cpu.eax = at(app, self);
        if (i32(at(app, cpu.eax + 0x5ec)) >= i32(0x3f400000))
        {
            integerMagnitude(app, cpu, 0x51c, cpu.ebp - 8, self);
            if (i32(at(app, cpu.ebp - 8)) > 0x40)
            {
                cpu.eax = at(app, self);
                magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 4, self);
                held = i32(at(app, cpu.ebp - 4)) <= i32(0x40a00000);
            }
        }
    }
    if (held)
    {
        cpu.eax = at(app, self);
        put(app, cpu.eax + 0x4fc, 0);
    }
    else
    {
        integerMagnitude(app, cpu, 0x51c, cpu.ebp - 0x14, self);
        if (i32(at(app, cpu.ebp - 0x14)) < 4)
        {
            cpu.eax = at(app, self);
            if (byte(app, cpu.eax + 0x50b) == 0)
                scaleField(app, cpu, self, 0x4fc, 0x53ded8);
        }
    }
    // 0x4aeb82.
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x1c, self);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x18, self);
    fpu.push(f32(app, cpu.ebp - 0x1c));
    fcomp(cpu, f32(app, cpu.ebp - 0x18));
    if (below(cpu))
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4fc, cpu.ebp - 0x28, self);
        fpu.push(f32(app, cpu.ebp - 0x28));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53dedc));
        cpu.eax = at(app, self);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 0x258));
        fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
        fpu.pop();
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53dee4));
        fstp(app, cpu, cpu.ebp - 0x2c);
        bool share = false;
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x24, self);
        fpu.push(f32(app, cpu.ebp - 0x24));
        fcomp(cpu, f32(app, cpu.ebp - 0x2c));
        if (below(cpu))
        {
            cpu.eax = at(app, self);
            magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x20, self);
            fpu.push(f32(app, cpu.ebp - 0x20));
            fcomp(cpu, f32(app, cpu.ebp - 0x2c));
            share = below(cpu);
        }
        if (share)
        {
            cpu.eax = at(app, self);
            fpu.push(f32(app, cpu.eax + 0x4f4));
            fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x2c));
            fstp(app, cpu, cpu.ebp - 0x38);
            magnitude(app, cpu, cpu.ebp - 0x38, cpu.ebp - 0x34, 0);
            cpu.eax = at(app, cpu.ebp - 0x34);
            put(app, cpu.ebp - 0x38, cpu.eax);
            fpu.push(f32(app, 0x53deec));
            fcomp(cpu, f32(app, cpu.ebp - 0x38));
            if (above(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x38);
                put(app, cpu.ebp - 0x30, cpu.eax);
            }
            else
                put(app, cpu.ebp - 0x30, 0x3f7ae148);
            cpu.eax = at(app, cpu.ebp - 0x30);
            put(app, cpu.ebp - 0x38, cpu.eax);
            cpu.eax = at(app, self);
            fpu.push(f32(app, cpu.eax + 0x4fc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x38));
            fstp(app, cpu, cpu.eax + 0x4fc);
        }
        else
        {
            cpu.eax = at(app, self);
            magnitude(app, cpu, cpu.eax + 0x5b0, cpu.ebp - 0x3c, self);
            scaleField(app, cpu, self, 0x4fc, i32(at(app, cpu.ebp - 0x3c)) < i32(0x3e4ccccd) ? 0x53def0 : 0x53def4);
        }
        return leave(app, cpu);
    }
    // 0x4aed74: faster across than forward.
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x40, self);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x44, self);
    fpu.push(f32(app, cpu.ebp - 0x40));
    fcomp(cpu, f32(app, cpu.ebp - 0x44));
    if (!above(cpu))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    cpu.eax += 0x4ec;
    if (!byAddress(app, cpu, kLength))
        return;
    fcomp(cpu, f64(app, 0x53defc));
    if (!below(cpu))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4fc, cpu.ebp - 0x50, self);
    fpu.push(f32(app, cpu.ebp - 0x50));
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53df04));
    cpu.eax = at(app, self);
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, cpu.eax + 0x258));
    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x53df0c));
    fstp(app, cpu, cpu.ebp - 0x54);
    bool share = false;
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x4c, self);
    fpu.push(f32(app, cpu.ebp - 0x4c));
    fcomp(cpu, f32(app, cpu.ebp - 0x54));
    if (below(cpu))
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4ec, cpu.ebp - 0x48, self);
        fpu.push(f32(app, cpu.ebp - 0x48));
        fcomp(cpu, f32(app, cpu.ebp - 0x54));
        share = below(cpu);
    }
    if (share)
    {
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0x4f4));
        fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x54));
        fstp(app, cpu, cpu.ebp - 0x60);
        magnitude(app, cpu, cpu.ebp - 0x60, cpu.ebp - 0x5c, 0);
        cpu.eax = at(app, cpu.ebp - 0x5c);
        put(app, cpu.ebp - 0x60, cpu.eax);
        fpu.push(f32(app, 0x53df14));
        fcomp(cpu, f32(app, cpu.ebp - 0x60));
        if (above(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x60);
            put(app, cpu.ebp - 0x58, cpu.eax);
        }
        else
            put(app, cpu.ebp - 0x58, 0x3f7ae148);
        cpu.eax = at(app, cpu.ebp - 0x58);
        put(app, cpu.ebp - 0x60, cpu.eax);
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0x4fc));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x60));
        fstp(app, cpu, cpu.eax + 0x4fc);
    }
    else
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x5b0, cpu.ebp - 0x64, self);
        scaleField(app, cpu, self, 0x4fc, i32(at(app, cpu.ebp - 0x64)) < i32(0x3e4ccccd) ? 0x53df18 : 0x53df1c);
    }
    // 0x4aef7a.
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x68, self);
    if (!scaleVector(app, cpu, self, 0xa4, i32(at(app, cpu.ebp - 0x68)) < i32(0x40a00000) ? 0x3f333333 : 0x3f4ccccd))
        return;
    leave(app, cpu);
}

/* 0x4aeffc: the car idling while racing (both pedals [+0x508], [+0x509]
 * under 0x20): slower than 4 out of neutral -- or slower than 5 in a gear
 * past it -- its speed and spin times 0.8 (0x4e0050). */
void idleDamping(App* app, CPU& cpu)
{
    enter(app, cpu, 0x14);
    const reg32 self = cpu.ebp - 8;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    if (half(app, cpu.eax + 0x164) != 0)
        return leave(app, cpu);
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x508) >= 0x20)
        return leave(app, cpu);
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x509) >= 0x20)
        return leave(app, cpu);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x14, self);
    bool damp = false, slow = false;
    zeroCompare(cpu, f32(app, cpu.ebp - 0x14));
    if (below(cpu))
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x10, self);
        slow = i32(at(app, cpu.ebp - 0x10)) < i32(0x40800000);
    }
    if (slow)
    {
        cpu.eax = at(app, self);
        damp = byte(app, cpu.eax + 0x50b) < 1;
    }
    if (!damp)
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0xc, self);
        if (i32(at(app, cpu.ebp - 0xc)) >= i32(0x40a00000))
            return leave(app, cpu);
        cpu.eax = at(app, self);
        if (byte(app, cpu.eax + 0x50b) <= 1)
            return leave(app, cpu);
    }
    if (!scaleVector(app, cpu, self, 0xa4, 0x3f4ccccd))
        return;
    if (!scaleVector(app, cpu, self, 0xe4, 0x3f4ccccd))
        return;
    leave(app, cpu);
}

/* 0x4af140: the car coasting in neutral ([+0x50b] 1) on the level (gravity
 * forward [kGravityZ] under 0.5 either way): its speed and spin times 0.99,
 * or 0.998 past 20 with the steering within 32 (0x4e0050). */
void coastDamping(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x18);
    const reg32 self = cpu.ebp - 0xc;
    put(app, self, cpu.eax);
    cpu.eax = at(app, self);
    if (byte(app, cpu.eax + 0x50b) != 1)
        return leave(app, cpu);
    magnitude(app, cpu, kGravityZ, cpu.ebp - 4, 0);
    if (i32(at(app, cpu.ebp - 4)) >= i32(0x3f000000))
        return leave(app, cpu);
    cpu.eax = at(app, self);
    magnitude(app, cpu, cpu.eax + 0x4f4, cpu.ebp - 0x14, self);
    bool fine = false;
    if (i32(at(app, cpu.ebp - 0x14)) >= i32(0x41a00000))
    {
        integerMagnitude(app, cpu, 0x51c, cpu.ebp - 0x10, self);
        fine = i32(at(app, cpu.ebp - 0x10)) <= 0x20;
    }
    put(app, cpu.ebp - 0x18, fine ? 0x3f7f7cee : 0x3f7d70a4);
    for (const reg32 offset : {0xa4u, 0xe4u})
    {
        cpu.eax = at(app, self);
        cpu.eax += offset;
        push(app, cpu, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 0x18));
        cpu.esp -= 4;
        fstp(app, cpu, cpu.esp);
        cpu.edx = at(app, self);
        cpu.edx += offset;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kScaleVectors))
            return;
    }
    leave(app, cpu);
}

/* 0x4af254: the spin's torque onto the x87's stack: 0 while the race is not
 * under way; else -(|[+0xe8]| x 0x53df20 x [+0xb0] x [+0x100]), times the
 * ground speed's share (0.1 at least -- the double 0.1 -- 1 at most), times
 * 0x53df34 more in reverse ([+0x50b] 0); negated again. */
void spinTorque(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    enter(app, cpu, 0x28);
    const reg32 self = cpu.ebp - 0x14;
    put(app, self, cpu.eax);
    put(app, cpu.ebp - 0xc, 0);
    put(app, cpu.ebp - 8, 0);
    put(app, cpu.ebp - 4, 0);
    cpu.eax = at(app, self);
    if (half(app, cpu.eax + 0x164) != 0)
        put(app, cpu.ebp - 0xc, 0);
    else
    {
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0xe8, cpu.ebp - 0x20, self);
        fpu.push(f32(app, cpu.ebp - 0x20));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53df20));
        cpu.eax = at(app, self);
        fpu.push(f32(app, cpu.eax + 0xb0));
        cpu.eax = at(app, self);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x100));
        fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
        fpu.pop();
        fpu.st(0) = -fpu.st(0);
        fstp(app, cpu, cpu.ebp - 0xc);
        cpu.eax = at(app, self);
        magnitude(app, cpu, cpu.eax + 0x5a4, cpu.ebp - 0x1c, self);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.ebp - 8, cpu.eax);
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53df24));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53df28));
        fcomp(cpu, f64(app, 0x53df2c));
        if (!below(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53df24));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x53df28));
            fstpDouble(app, cpu, cpu.ebp - 0x28);
        }
        else
        {
            put(app, cpu.ebp - 0x28, 0x9999999a);
            put(app, cpu.ebp - 0x24, 0x3fb99999);
        }
        fpu.push(f64(app, cpu.ebp - 0x28));
        fstp(app, cpu, cpu.ebp - 4);
        if (i32(at(app, cpu.ebp - 4)) > i32(0x3f800000))
            put(app, cpu.ebp - 0x18, 0x3f800000);
        else
        {
            cpu.eax = at(app, cpu.ebp - 4);
            put(app, cpu.ebp - 0x18, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0xc));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x18));
        fstp(app, cpu, cpu.ebp - 0xc);
        cpu.eax = at(app, self);
        if (byte(app, cpu.eax + 0x50b) == 0)
        {
            fpu.push(f32(app, cpu.ebp - 0xc));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x53df34));
            fstp(app, cpu, cpu.ebp - 0xc);
        }
    }
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = -fpu.st(0);
    fstp(app, cpu, cpu.ebp - 0x10);
    fpu.push(f32(app, cpu.ebp - 0x10));
    leave(app, cpu);
}

}  // namespace physics
}  // namespace nfs3hp
