#include "native_guest.h"

#include <utility>

/* The race's cameras, native whole.  disassemble_nfs3hp.py binds each address
 * to the function of that name here (CARSTATE_NATIVES), and the generated code
 * calls it by name.  tools/game_checks holds each against its generated code.
 *
 * Each player (0, or 1 in a split screen: the code takes an index as
 * `index & 1` only where the race's mode [0x6fd3b0] is 1, else 0) has a camera of
 * each of 25 modes, records of 0x134 bytes the table at 0x552ec8 points at
 * (mode * 8 + player * 4), the one in use at 0x5e10b0 + player * 4 and one
 * that overrides it, if any, at 0x5e10b8.  A camera's record: its mode (+0),
 * the car it rides with (+4), the car it looks at (+8), its player (+0xc),
 * its place (+0x14), its orientation (+0x2c, 9 floats), its height (+0x50),
 * its zoom (+0x54), 25 flags of the modes it may hand over to (+0x58), its
 * place on the track (+0x74, 30 dwords), the track's node it keeps to (+0xec)
 * and the mode's own settings from +0xf0 on -- which constraints it keeps to
 * (+0xf4 the track's walls, +0xf8 the other cars, +0x104 the head's turn,
 * +0x108 looking at its target) and the mode's numbers (+0x110 to +0x130).
 * A car's record: its place (+0x98), its orientation (+0xc0), its node on the
 * track (+0x84), its speed (+0x7c0 in the car's physics, at +4).  The track's
 * nodes are 0x24 bytes at [0x5dd948], [0x5dd940] of them.  0x552ff4 is each
 * player's mark that its camera changed; 0x5dd950 each player's view, 0x60
 * bytes.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack pushed and popped as
 * the x87 does, its arithmetic through x86::FPU (add, sub, mul, div, toInteger)
 * in the code's order of operands.  The two switches go by the targets
 * nfs3.exe's tables hold (0x422a7c, 0x422c88).  Every other function of the
 * game's is called by address. */
namespace nfs3hp
{
namespace carstate
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

// The race's mode (1 a split screen); the cameras in use and those overriding them; the
// cameras of each mode; each player's mark of a change.
constexpr reg32 kPlayers = 0x6fd3b0;
constexpr reg32 kCamera = 0x5e10b0, kOverride = 0x5e10b8;
constexpr reg32 kCameras = 0x552ec8;
constexpr reg32 kChanged = 0x552ff4;
// The cars, their count; the track's nodes and their count; each player's view.
constexpr reg32 kCars = 0x5efac8, kCarCount = 0x5efd9c;
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
constexpr reg32 kViews = 0x5dd950;
// Each player's car (by the second player's index, 0x6fd4f4), its record.
constexpr reg32 kSecondPlayer = 0x6fd4f4, kPlayerCars = 0x5efa48;
// What is called by address.
constexpr reg32 kSetView = 0x4dbde0, kDot = 0x4e01f0, kTranspose = 0x4e0110;
constexpr reg32 kSurface = 0x4207e0, kTrackPlace = 0x41e420, kTrackNode = 0x41f6e0;
constexpr reg32 kSubtract = 0x4e0000, kNormalize = 0x4e01c0, kCross = 0x4e0160;
constexpr reg32 kCrossNormal = 0x4e01a0, kMatrix = 0x4e0210, kRotate = 0x4dffb0;
constexpr reg32 kScaleAdd = 0x4e0050, kMultiply = 0x4e0280;
constexpr reg32 kModeSound = 0x426450;

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

/* movsd: a dword from [esi] to es:[edi], both stepped by the direction flag;
 * rep movsd: ecx of them. */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
    cpu.edi += step;
    cpu.esi += step;
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

void fst(App* app, CPU& cpu, reg32 a) { app->getMemory<float>(a) = float(cpu.fpu.st(0)); }

void fstp(App* app, CPU& cpu, reg32 a)
{
    fst(app, cpu, a);
    cpu.fpu.pop();
}

void fstp64(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<double>(a) = double(cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fxch(CPU& cpu, int i)
{
    const Float t = cpu.fpu.st(0);
    cpu.fpu.st(0) = cpu.fpu.st(i);
    cpu.fpu.st(i) = t;
}

// st(0) op= m (fadd m32 and the like, fadd st(i) one operand).
void fadd(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), m); }
void fsub(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), m); }
void fmul(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), m); }

// st(i) op= st(0), popped (faddp st(i) and the like); the reversed forms
// st(i) = st(0) op st(i).
void faddp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::add(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fmulp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::mul(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fdivp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::div(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fsubrp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::sub(cpu.fpu.st(0), Float(cpu.fpu.st(i)));
    cpu.fpu.pop();
}

void fdivrp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::div(cpu.fpu.st(0), Float(cpu.fpu.st(i)));
    cpu.fpu.pop();
}

void fchs(CPU& cpu) { cpu.fpu.st(0) = -cpu.fpu.st(0); }

/* sub esp, 4; fistp dword [esp]; pop eax: st(0) as an integer into eax. */
void fistpToEax(App* app, CPU& cpu)
{
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
    cpu.eax = pop(app, cpu);
}

/* fcomp m; fnstsw ax (sahf): st(0) compared and popped, the status word in
 * ax; CF and ZF as sahf sets them. */
struct Flags
{
    bool cf, zf;
};

Flags fcomp(CPU& cpu, const Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.ax = cpu.fpu.status.word;
    cpu.fpu.pop();
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

}  // namespace

void idleHook(App* app, CPU& cpu);
void viewFromCamera(App* app, CPU& cpu);
void viewFromScreenCamera(App* app, CPU& cpu);
void chooseView(App* app, CPU& cpu);
void idleHookToo(App* app, CPU& cpu);
void pickChaseNode(App* app, CPU& cpu);
void turnAround(App* app, CPU& cpu);
void attachCameras(App* app, CPU& cpu);
void headTurn(App* app, CPU& cpu);
void cameraNode(App* app, CPU& cpu);
void cameraTrackPlace(App* app, CPU& cpu);
void keepOutOfCars(App* app, CPU& cpu);
void keepAboveRoad(App* app, CPU& cpu);
void keepOnTrack(App* app, CPU& cpu);
void keepFromRightWall(App* app, CPU& cpu);
void keepFromLeftWall(App* app, CPU& cpu);
void keepAboveGround(App* app, CPU& cpu);
void lookAtTarget(App* app, CPU& cpu);
void blendCamera(App* app, CPU& cpu);
void placeCamera(App* app, CPU& cpu);
void switchCamera(App* app, CPU& cpu);
void cameraZoom(App* app, CPU& cpu);
void viewMatrix(App* app, CPU& cpu);
void updateCamera(App* app, CPU& cpu);
void cameraView(App* app, CPU& cpu);
void cockpitShown(App* app, CPU& cpu);
void cameraCar(App* app, CPU& cpu);
void cameraTarget(App* app, CPU& cpu);
void initCameras(App* app, CPU& cpu);
void setCameraMode(App* app, CPU& cpu);
void setOverrideMode(App* app, CPU& cpu);
void nextCycleMode(App* app, CPU& cpu);
void buildCameraCycle(App* app, CPU& cpu);
void modeInCycle(App* app, CPU& cpu);
void rememberMode(App* app, CPU& cpu);
void cameraMode(App* app, CPU& cpu);
void setModeAnew(App* app, CPU& cpu);
void settleCamera(App* app, CPU& cpu);
void nearestSpot(App* app, CPU& cpu);
void scaledIfSet(App* app, CPU& cpu);
void bitsBroken(App* app, CPU& cpu);
void dampVector(App* app, CPU& cpu);
void chaseOffset(App* app, CPU& cpu);
void chaseOffsetLevel(App* app, CPU& cpu);

/* 0x421360 and 0x421480: empty hooks -- push ebp, mov ebp, esp, pop ebp. */
void idleHook(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

void idleHookToo(App* app, CPU& cpu)
{
    idleHook(app, cpu);
}

/* 0x421380: the view of the camera eax: the camera updated (its mode [eax],
 * car [eax + 4], view eax + 8 -- updateCamera), then the view set (0x4dbde0). */
void viewFromCamera(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edx = cpu.eax + 8;
    cpu.ebx = at(app, cpu.eax);
    cpu.eax = at(app, cpu.eax + 4);
    if (!direct(app, cpu, updateCamera))
        return;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kSetView))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4213b0: the view eax set (0x4dbde0); where its player's camera is of
 * mode 8, the view's frustum (0x56009c..) and its corners (0x564434..) from
 * the camera's own (0x5e1020..); the player's mark of a change cleared. */
void viewFromScreenCamera(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (!byAddress(app, cpu, kSetView))
        return;
    cpu.eax = at(app, cpu.edx + 4);
    cpu.eax <<= 2;
    cpu.ecx = at(app, cpu.eax + kCamera);
    if (cpu.ecx != 0)
    {
        cpu.eax = at(app, cpu.ecx);
        if (cpu.eax == 8)
        {
            fld(cpu, f32(app, 0x5e1020));
            fld(cpu, f32(app, 0x5e1038));
            fld(cpu, f32(app, 0x5e102c));
            fld(cpu, f32(app, 0x5e1030));
            cpu.eax = at(app, 0x5e10a0);
            fxch(cpu, 3);
            fstp(app, cpu, 0x56009c);
            put(app, 0x56443c, cpu.eax);
            cpu.eax = at(app, 0x5e1034);
            fxch(cpu, 1);
            fstp(app, cpu, 0x5600a0);
            put(app, 0x564440, cpu.eax);
            cpu.eax = at(app, 0x5e1024);
            fstp(app, cpu, 0x5600a4);
            put(app, 0x564434, cpu.eax);
            cpu.eax = at(app, 0x5e1028);
            fstp(app, cpu, 0x5600a8);
            put(app, 0x564438, cpu.eax);
        }
    }
    cpu.eax = at(app, cpu.edx + 4);
    cpu.ebx = 0;
    put(app, cpu.eax * 4 + kChanged, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421450: the view eax kept (0x552ec4) and set the way edx says: 0 by its
 * camera (viewFromCamera), 2 as a screen's (viewFromScreenCamera), any other
 * not at all. */
void chooseView(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, 0x552ec4, cpu.eax);
    if (cpu.edx == 0)
    {
        if (!direct(app, cpu, viewFromCamera))
            return;
    }
    else if (cpu.edx == 2)
    {
        if (!direct(app, cpu, viewFromScreenCamera))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421490: player eax's chase camera's node ahead: the car's speed (its
 * physics' +0x7c0, scaled by 0x536e04) placed among the bands at 0x4212b0
 * (their index to 0x553014), and the node 3, 5 or 7 on from the car's (or 3
 * back, below the first band) to 0x55300c, wrapped round the track; 0x553004
 * cleared. */
void pickChaseNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    cpu.edx = cpu.eax;
    cpu.ecx = 8;
    cpu.edi = cpu.ebp - 0x20;
    cpu.esi = 0x4212b0;
    cpu.eax = at(app, cpu.eax * 4 + kCamera);
    repMovsd(app, cpu);
    cpu.ecx = at(app, cpu.eax + 4);
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    fmul(cpu, f64(app, 0x536e04));
    fistpToEax(app, cpu);
    cpu.esi = at(app, kPlayers);
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.esi == 1 ? cpu.edx & cpu.esi : 0;
    cpu.eax = 0;
    while (sreg32(cpu.ebx) > sreg32(at(app, cpu.ebp + cpu.eax * 4 - 0x1c)))
        ++cpu.eax;
    cpu.esi = at(app, cpu.ebp + cpu.eax * 4 - 0x20);
    put(app, cpu.edx * 4 + 0x553014, cpu.eax);
    cpu.eax = cpu.ebx - cpu.esi;
    if (sreg32(cpu.eax) < 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x1c) - 3;
        if (sreg32(cpu.eax) < 0)
            cpu.eax += at(app, kNodeCount);
    }
    else if (sreg32(cpu.eax) < 0x30000)
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        cpu.ecx = at(app, kNodeCount);
        cpu.eax += 3;
        if (sreg32(cpu.eax) >= sreg32(cpu.ecx))
            cpu.eax -= cpu.ecx;
    }
    else if (sreg32(cpu.eax) < 0x140000)
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        cpu.esi = at(app, kNodeCount);
        cpu.eax += 5;
        if (sreg32(cpu.eax) >= sreg32(cpu.esi))
            cpu.eax -= cpu.esi;
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        cpu.ecx = at(app, kNodeCount);
        cpu.eax += 7;
        if (sreg32(cpu.eax) >= sreg32(cpu.ecx))
            cpu.eax -= cpu.ecx;
    }
    put(app, cpu.edx * 4 + 0x55300c, cpu.eax);
    cpu.esi = 0;
    put(app, cpu.edx * 4 + 0x553004, cpu.esi);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421570: the orientation at eax turned about its up axis into edx: its
 * first and last rows negated, the middle one copied. */
void turnAround(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (const reg32 k : {0u, 4u, 8u})
    {
        fld(cpu, f32(app, cpu.eax + k));
        fchs(cpu);
        fstp(app, cpu, cpu.edx + k);
    }
    for (const reg32 k : {0xcu, 0x10u, 0x14u})
    {
        cpu.ecx = at(app, cpu.eax + k);
        put(app, cpu.edx + k, cpu.ecx);
    }
    for (const reg32 k : {0x18u, 0x1cu, 0x20u})
    {
        fld(cpu, f32(app, cpu.eax + k));
        fchs(cpu);
        fstp(app, cpu, cpu.edx + k);
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4215c0: player eax's cameras put on car edx (if it is one of the
 * race's): a pursuit's radio stopped (mode 0x11, 0x40f460), the mode's sound
 * (0x426310), the change marked where the car is another, and each camera
 * of modes 1 to 24 given the car, its place and its orientation; then mode 3
 * (setCameraMode). */
void attachCameras(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 mode = cpu.ebp - 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = at(app, kPlayers);
    if (cpu.ecx == 1)
        cpu.ebx &= cpu.ecx;
    else
        cpu.ebx ^= cpu.eax;
    cpu.esi = at(app, 0x5efd98);
    cpu.eax = at(app, 0x5efda4);
    cpu.eax += cpu.esi;
    if (sreg32(cpu.edx) >= sreg32(cpu.eax))
        return leave();
    cpu.eax = cpu.ebx * 4;
    cpu.edi = at(app, cpu.eax + kOverride);
    if (cpu.edi != 0)
        cpu.eax = at(app, cpu.edi);
    else
    {
        cpu.esi = at(app, cpu.eax + kCamera);
        cpu.eax = cpu.esi != 0 ? at(app, cpu.esi) : 0;
    }
    if (cpu.eax == 0x11)
    {
        cpu.ecx = at(app, cpu.ebx * 4 + 0x552f50);
        cpu.edi = at(app, cpu.ecx + 0x118);
        if (cpu.edi != 0xffffffff)
        {
            cpu.eax = cpu.edi;
            if (!byAddress(app, cpu, 0x40f460))
                return;
            put(app, cpu.ecx + 0x118, 0xffffffff);
        }
    }
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, 0x426310))
        return;
    for (const reg32 field : {8u, 4u})
    {
        cpu.eax = cpu.ebx * 4;
        cpu.ecx = at(app, cpu.eax + kOverride);
        cpu.eax = cpu.ecx != 0 ? cpu.ecx : at(app, cpu.eax + kCamera);
        cpu.eax = at(app, cpu.eax + field);
        if (cpu.eax != at(app, cpu.edx * 4 + kCars))
            put(app, cpu.ebx * 4 + kChanged, 1);
    }
    put(app, mode, 1);
    do
    {
        cpu.ecx = at(app, mode);
        cpu.eax = cpu.ebx * 4;
        cpu.eax = at(app, cpu.eax + cpu.ecx * 8 + kCameras);
        cpu.esi = at(app, cpu.edx * 4 + kCars);
        put(app, cpu.eax + 4, cpu.esi);
        put(app, cpu.eax + 8, cpu.esi);
        cpu.ecx = cpu.esi;
        cpu.edi = cpu.eax + 0x2c;
        fld(cpu, f32(app, cpu.ecx + 0x98));
        cpu.esi = at(app, cpu.eax + 4);
        cpu.ecx = at(app, cpu.eax + 4);
        fstp(app, cpu, cpu.eax + 0x14);
        fld(cpu, f32(app, cpu.ecx + 0x9c));
        cpu.ecx = at(app, cpu.eax + 4);
        fstp(app, cpu, cpu.eax + 0x18);
        fld(cpu, f32(app, cpu.ecx + 0xa0));
        cpu.ecx = 9;
        fstp(app, cpu, cpu.eax + 0x1c);
        cpu.esi += 0xc0;
        repMovsd(app, cpu);
        put(app, mode, at(app, mode) + 1);
    } while (sreg32(at(app, mode)) < 0x19);
    cpu.edx = 3;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, setCameraMode))
        return;
    leave();
}

/* 0x421730: the cockpit camera eax's head turned with the car's steering:
 * the steering ([+0x920] and [+0x924] of the car's physics, by the head's
 * mode +0x104) scaled (0x536e0c, 0x536e10; more past 60 at +0x5a4), the
 * turn's matrices about two axes (0x4ea820, 0x4ea680, of the identity at
 * 0x564498) multiplied into the camera's orientation (0x4e0280). */
void headTurn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x74;
    const reg32 b = cpu.ebp - 4, a = cpu.ebp - 8;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.ecx + 0x104);
    cpu.eax = at(app, cpu.eax + 4);
    if (cpu.edx == 1)
    {
        fld(cpu, f32(app, cpu.eax + 0x920));
        cpu.edx = at(app, cpu.eax + 0x924);
        fmul(cpu, f32(app, 0x536e0c));
        put(app, b, cpu.edx);
        fstp(app, cpu, a);
    }
    else
    {
        fld(cpu, f32(app, cpu.eax + 0x924));
        cpu.edx = at(app, cpu.eax + 0x920);
        fmul(cpu, f32(app, 0x536e10));
        put(app, a, cpu.edx);
        fstp(app, cpu, b);
    }
    if (sreg32(at(app, cpu.eax + 0x5a4)) > 0x42700000)
    {
        fld(cpu, f32(app, b));
        fmul(cpu, f32(app, 0x536e10));
        fstp(app, cpu, b);
    }
    cpu.eax = cpu.ebp - 0x74;
    push(app, cpu, cpu.eax);
    push(app, cpu, at(app, b));
    cpu.eax = 0x564498;
    cpu.ebx = cpu.ebp - 0x2c;
    if (!byAddress(app, cpu, 0x4ea820))
        return;
    cpu.eax = cpu.ebp - 0x50;
    fld(cpu, f32(app, a));
    push(app, cpu, cpu.eax);
    fadd(cpu, f32(app, cpu.ecx + 0x50));
    cpu.esp -= 4;
    cpu.eax = 0x564498;
    fstp(app, cpu, cpu.esp);
    cpu.edx = cpu.ebp - 0x50;
    if (!byAddress(app, cpu, 0x4ea680))
        return;
    cpu.eax = cpu.ebp - 0x74;
    if (!byAddress(app, cpu, kMultiply))
        return;
    cpu.edx = cpu.ecx + 0x2c;
    cpu.eax = cpu.ebp - 0x2c;
    cpu.ebx = cpu.edx;
    if (!byAddress(app, cpu, kMultiply))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4217e0: the track's node player eax's camera keeps to: of the camera
 * overriding it or the one in use -- for mode 0x13 the camera of the mode it
 * stands in for ([+0x11c]) -- its own node (+0xec), or its car's (+0x84)
 * where that is negative. */
void cameraNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.ecx = at(app, cpu.eax + kOverride);
    if (cpu.ecx != 0)
    {
        cpu.eax = at(app, cpu.ecx);
        cpu.ecx = cpu.eax;
    }
    else
    {
        cpu.esi = at(app, cpu.eax + kCamera);
        if (cpu.esi != 0)
            cpu.ecx = at(app, cpu.esi);
    }
    cpu.eax = cpu.edx * 4;
    if (cpu.ecx == 0x13)
    {
        cpu.edi = at(app, cpu.eax + kOverride);
        cpu.eax = cpu.edi != 0 ? cpu.edi : at(app, cpu.eax + kCamera);
        cpu.eax = at(app, cpu.eax + 0x11c);
        cpu.ecx = cpu.eax * 8;
        cpu.eax = at(app, cpu.ecx + cpu.edx * 4 + kCameras);
    }
    else
    {
        cpu.edx = at(app, cpu.eax + kOverride);
        cpu.eax = cpu.edx != 0 ? cpu.edx : at(app, cpu.eax + kCamera);
    }
    cpu.ecx = at(app, cpu.eax + 0xec);
    cpu.eax = sreg32(cpu.ecx) >= 0 ? cpu.ecx : at(app, cpu.eax + 0x84);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421880: camera eax's place on the track for its place edx: the place in
 * the track's fixed point (scaled by 0x536e14), the car's place on the track
 * ([eax + 4] + 0xc, 30 dwords) as the start, then found (0x41e420) and its
 * node (0x41f6e0). */
void cameraTrackPlace(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.ebx = cpu.eax;
    for (const reg32 k : {0u, 4u, 8u})
    {
        fld(cpu, f32(app, cpu.edx + k));
        fmul(cpu, f64(app, 0x536e14));
        fistpToEax(app, cpu);
        put(app, cpu.ebp - 0xc + k, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebx + 4);
    cpu.ecx = 0x1e;
    cpu.edi = cpu.ebx + 0x74;
    cpu.esi = cpu.eax + 0xc;
    repMovsd(app, cpu);
    cpu.ecx = cpu.ebx + 0x74;
    cpu.eax = cpu.ebp - 0xc;
    cpu.edx = cpu.ecx;
    cpu.ebx = 1;
    if (!byAddress(app, cpu, kTrackPlace))
        return;
    cpu.eax = cpu.ebp - 0xc;
    cpu.edx = cpu.ecx;
    if (!byAddress(app, cpu, kTrackNode))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421900: the place eax kept out of the cars: of each of the race's cars
 * shown and not wrecked (+0x89, +0x88) near enough (0x4965b0 against its
 * size +0x120 and a margin), the place in the car's axes (0x4e01f0 with its
 * three rows); inside the box of its half sizes (+0x114.., plus 0x536e24),
 * and inside the ellipse of its sides, it is pushed out sideways onto it (a
 * square root, 0x4ea920) and back into the world (0x4e0110 transposes the
 * car's axes). */
void keepOutOfCars(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x88;
    cpu.ebp -= 0x82;
    const reg32 e = cpu.ebp;
    cpu.ebx = cpu.eax;
    for (cpu.esi = 0; sreg32(cpu.esi) < sreg32(at(app, kCarCount)); ++cpu.esi)
    {
        cpu.ecx = at(app, cpu.esi * 4 + kCars);
        cpu.ah = byte(app, cpu.ecx + 0x89);
        if (cpu.ah == 0)
            continue;
        if (byte(app, cpu.ecx + 0x88) != 0)
            continue;
        fld(cpu, f32(app, cpu.ecx + 0x120));
        cpu.eax = cpu.ecx + 0x98;
        fadd(cpu, f64(app, 0x536e1c));
        cpu.edx = cpu.ebx;
        fstp64(app, cpu, e + 0x5a);
        if (!byAddress(app, cpu, 0x4965b0))
            return;
        Flags f = fcomp(cpu, f64(app, e + 0x5a));
        if (!f.cf && !f.zf)
            continue;
        fld(cpu, f32(app, cpu.ebx));
        fsub(cpu, f32(app, cpu.ecx + 0x98));
        fstp(app, cpu, e + 0x2a);
        fld(cpu, f32(app, cpu.ebx + 4));
        fsub(cpu, f32(app, cpu.ecx + 0x9c));
        cpu.eax = e + 0x2a;
        fstp(app, cpu, e + 0x2e);
        fld(cpu, f32(app, cpu.ebx + 8));
        fsub(cpu, f32(app, cpu.ecx + 0xa0));
        cpu.edx = cpu.ecx + 0xc0;
        fstp(app, cpu, e + 0x32);
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.eax = e + 0x2a;
        cpu.edx = cpu.ecx + 0xcc;
        fstp(app, cpu, e + 0x42);
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.eax = e + 0x2a;
        cpu.edx = cpu.ecx + 0xd8;
        fstp(app, cpu, e + 0x46);
        if (!byAddress(app, cpu, kDot))
            return;
        fld(cpu, 0.0);
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x4a);
        fld(cpu, f32(app, cpu.ecx + 0x114));
        fld(cpu, f32(app, cpu.ecx + 0x118));
        fld(cpu, f32(app, cpu.ecx + 0x11c));
        fxch(cpu, 2);
        fld(cpu, f32(app, 0x536e24));
        fxch(cpu, 1);
        fadd(cpu, fpu.st(1));
        fxch(cpu, 2);
        fadd(cpu, fpu.st(1));
        fxch(cpu, 3);
        faddp(cpu, 1);
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x1e);
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x22);
        fstp(app, cpu, e + 0x26);
        // The absolute values of the three, through the frame.
        f = fcomp(cpu, f32(app, e + 0x42));
        if (!f.cf && !f.zf)
        {
            fld(cpu, f32(app, e + 0x42));
            fchs(cpu);
            fstp(app, cpu, e + 0x6e);
        }
        else
        {
            cpu.eax = at(app, e + 0x42);
            put(app, e + 0x6e, cpu.eax);
        }
        cpu.eax = at(app, e + 0x6e);
        fld(cpu, 0.0);
        put(app, e + 0x4e, cpu.eax);
        f = fcomp(cpu, f32(app, e + 0x46));
        if (!f.cf && !f.zf)
        {
            fld(cpu, f32(app, e + 0x46));
            fchs(cpu);
            fstp(app, cpu, e + 0x72);
        }
        else
        {
            cpu.eax = at(app, e + 0x46);
            put(app, e + 0x72, cpu.eax);
        }
        cpu.eax = at(app, e + 0x72);
        fld(cpu, 0.0);
        put(app, e + 0x52, cpu.eax);
        f = fcomp(cpu, f32(app, e + 0x4a));
        if (!f.cf && !f.zf)
        {
            fld(cpu, f32(app, e + 0x4a));
            fchs(cpu);
            fstp(app, cpu, e + 0x7e);
        }
        else
        {
            cpu.eax = at(app, e + 0x4a);
            put(app, e + 0x7e, cpu.eax);
        }
        cpu.eax = at(app, e + 0x7e);
        fld(cpu, f32(app, e + 0x4e));
        put(app, e + 0x56, cpu.eax);
        if (!fcomp(cpu, f32(app, e + 0x1e)).cf)
            continue;
        fld(cpu, f32(app, e + 0x52));
        if (!fcomp(cpu, f32(app, e + 0x22)).cf)
            continue;
        fld(cpu, f32(app, e + 0x7e));
        if (!fcomp(cpu, f32(app, e + 0x26)).cf)
            continue;
        // The side it is pushed out to: 1.0 or -1.0, a double in the frame.
        fld(cpu, 0.0);
        if (fcomp(cpu, f32(app, e + 0x46)).cf)
        {
            cpu.edi = 0;
            cpu.eax = 0x3ff00000;
            put(app, e + 0x62, cpu.edi);
            put(app, e + 0x66, cpu.eax);
        }
        else
        {
            cpu.edx = 0;
            cpu.edi = 0xbff00000;
            put(app, e + 0x62, cpu.edx);
            put(app, e + 0x66, cpu.edi);
        }
        fld(cpu, f32(app, e + 0x4a));
        fmul(cpu, fpu.st(0));
        fld(cpu, f32(app, e + 0x26));
        fmul(cpu, fpu.st(0));
        fdivp(cpu, 1);
        fld(cpu, f32(app, e + 0x42));
        fmul(cpu, fpu.st(0));
        fld(cpu, f32(app, e + 0x1e));
        fmul(cpu, fpu.st(0));
        fdivp(cpu, 1);
        fld(cpu, f64(app, e + 0x62));
        fxch(cpu, 1);
        faddp(cpu, 2);
        fld(cpu, 0.0);
        fld(cpu, 1.0);
        fsubrp(cpu, 3);
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x6a);
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x76);
        f = fcomp(cpu, f32(app, e + 0x76));
        if (!f.cf && !f.zf)
            continue;
        fld(cpu, f32(app, e + 0x22));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(0));
        fmul(cpu, f32(app, e + 0x76));
        cpu.esp -= 4;
        fstp(app, cpu, cpu.esp);
        if (!byAddress(app, cpu, 0x4ea920))
            return;
        fld(cpu, f32(app, e + 0x52));
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x7a);
        if (!fcomp(cpu, f32(app, e + 0x7a)).cf)
            continue;
        fld(cpu, f32(app, e + 0x7a));
        cpu.eax = at(app, e + 0x42);
        fmul(cpu, f32(app, e + 0x6a));
        put(app, e + 0x36, cpu.eax);
        cpu.eax = at(app, e + 0x4a);
        cpu.edx = e - 6;
        put(app, e + 0x3e, cpu.eax);
        cpu.eax = cpu.ecx + 0xc0;
        fstp(app, cpu, e + 0x3a);
        if (!byAddress(app, cpu, kTranspose))
            return;
        cpu.edx = e - 6;
        cpu.eax = e + 0x36;
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.edx = e + 6;
        cpu.eax = e + 0x36;
        fstp(app, cpu, e + 0x42);
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.edx = e + 0x12;
        cpu.eax = e + 0x36;
        fstp(app, cpu, e + 0x46);
        if (!byAddress(app, cpu, kDot))
            return;
        fld(cpu, f32(app, cpu.ecx + 0x98));
        fadd(cpu, f32(app, e + 0x42));
        fstp(app, cpu, cpu.ebx);
        fld(cpu, f32(app, cpu.ecx + 0x9c));
        fadd(cpu, f32(app, e + 0x46));
        fxch(cpu, 1);
        fstp(app, cpu, e + 0x4a);
        fstp(app, cpu, cpu.ebx + 4);
        fld(cpu, f32(app, cpu.ecx + 0xa0));
        fadd(cpu, f32(app, e + 0x4a));
        fstp(app, cpu, cpu.ebx + 8);
    }
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421b90: camera eax's place edx kept above the road: the road's height
 * there -- the car's plane's (0x496a50, 0x4968a0) where the camera follows a
 * car on a surface it knows, else the node's (+4 of [0x5dd948]'s, scaled by
 * 0x536e34) -- plus the surface's own (0x552e54's, by 0x420bf0's say, else
 * 25.0) and a margin (0x536e2c): the height no lower than that. */
void keepAboveRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x60;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.eax += 0x74;
    cpu.edx = 1;
    cpu.esi = at(app, cpu.eax + 0x10);
    if (!byAddress(app, cpu, kSurface))
        return;
    push(app, cpu, cpu.esi);
    cpu.edi = at(app, cpu.ebx + 8);
    push(app, cpu, cpu.edi);
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.ebx + 4);
    push(app, cpu, cpu.eax);
    cpu.edi = at(app, cpu.ebx);
    push(app, cpu, cpu.edi);
    cpu.edx &= 0x3f;
    if (!byAddress(app, cpu, 0x420bf0))
        return;
    if (cpu.eax != 0)
    {
        cpu.edi = at(app, 0x552e54);
        cpu.eax = cpu.esi * 4;
        cpu.eax += cpu.edi;
        cpu.eax = at(app, cpu.eax);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    else
        put(app, cpu.ebp - 4, 0x41c80000);
    if (at(app, cpu.ecx + 0x88) != 0 && at(app, cpu.ecx + 0x8c) != 0 && at(app, cpu.ecx + 0xe0) == 0
        && cpu.edx != 0 && cpu.edx != 0xe)
    {
        cpu.edi = cpu.ebp - 0x60;
        cpu.esi = at(app, cpu.ecx + 0xc8);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = cpu.ebp - 0x54;
        cpu.esi = at(app, cpu.ecx + 0xcc);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = cpu.ebp - 0x48;
        cpu.esi = at(app, cpu.ecx + 0xd0);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = cpu.ebp - 0x3c;
        cpu.esi = at(app, cpu.ecx + 0xd4);
        cpu.edx = cpu.ebp - 0x30;
        cpu.eax = cpu.ebp - 0x60;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        if (!byAddress(app, cpu, 0x496a50))
            return;
        cpu.eax = at(app, cpu.ebx + 8);
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, cpu.ebx + 4);
        push(app, cpu, cpu.edx);
        cpu.esi = at(app, cpu.ebx);
        cpu.eax = at(app, cpu.ecx + 0xc8);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.eax + 8);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.eax + 4);
        push(app, cpu, cpu.edx);
        cpu.ecx = at(app, cpu.eax);
        push(app, cpu, cpu.ecx);
        cpu.eax = cpu.ebp - 0x30;
        if (!byAddress(app, cpu, 0x4968a0))
            return;
    }
    else
    {
        cpu.eax = cpu.esi * 8;
        cpu.edx = at(app, kNodes);
        cpu.eax += cpu.esi;
        fild(app, cpu, cpu.edx + cpu.eax * 4 + 4);
        fmul(cpu, f64(app, 0x536e34));
    }
    fadd(cpu, f32(app, cpu.ebp - 4));
    fadd(cpu, f64(app, 0x536e2c));
    fstp(app, cpu, cpu.ebp - 8);
    fld(cpu, f32(app, cpu.ebx + 4));
    cpu.eax = !fcomp(cpu, f32(app, cpu.ebp - 8)).cf ? at(app, cpu.ebp - 8) : at(app, cpu.ebx + 4);
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.ebx + 4, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x421cc0: camera eax's place edx kept on the track, seen from the point on
 * the stack (3 floats, ret 0xc): unless the camera may leave it (its car's
 * side of the road, +0xe0, or a car on a surface it knows), the way from the
 * point to the place halved 10 times (0x536e3c), each half's middle tried on
 * the track (0x41e990, in its fixed point by 0x536e44) and on a known surface
 * (0x4207e0) -- the far end moved in where it is, the near end out where not;
 * the place the far end, and its track place found (cameraTrackPlace). */
void keepOnTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xbc;
    cpu.ebp -= 0x7e;
    const reg32 e = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp + 0x7e;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4 + 0xc;
    };
    cpu.ebx = cpu.eax;
    put(app, e + 0x76, cpu.edx);
    cpu.edi = e + 0x5e;
    cpu.esi = e + 0x96;
    cpu.edx = 1;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.eax += 0x74;
    cpu.edi = e + 0x46;
    cpu.esi = at(app, e + 0x76);
    if (!byAddress(app, cpu, kSurface))
        return;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.edi = e + 0x6a;
    cpu.esi = e + 0x46;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.ecx = 0x1e;
    cpu.edi = e - 0x3e;
    cpu.esi = cpu.ebx + 0x74;
    repMovsd(app, cpu);
    cpu.eax &= 0x3f;
    cpu.edx = at(app, cpu.ebx + 0xe0);
    put(app, e + 0x7a, cpu.eax);
    if (cpu.edx != 0)
    {
        cpu.eax = at(app, cpu.ebx + 4);
        cpu.ecx = at(app, cpu.eax + 0x2c);
        if (cpu.ecx != 0)
        {
            if (cpu.edx == 1 && x86::sreg16(app->getMemory<reg16>(cpu.ecx + 4)) >= 0)
                return leave();
            if (at(app, cpu.ebx + 0xe0) == 2)
            {
                cpu.eax = at(app, cpu.ebx + 4);
                cpu.eax = at(app, cpu.eax + 0x2c);
                if (x86::sreg16(app->getMemory<reg16>(cpu.eax + 6)) >= 0)
                    return leave();
            }
        }
    }
    else if (at(app, cpu.ebx + 0x88) != 0 && at(app, cpu.ebx + 0x8c) != 0)
    {
        cpu.ecx = at(app, e + 0x7a);
        if (cpu.ecx != 0 && cpu.ecx != 0xe)
            return leave();
    }
    cpu.ecx = 0xa;
    do
    {
        fld(cpu, f32(app, e + 0x4a));
        fld(cpu, f32(app, e + 0x46));
        fadd(cpu, f32(app, e + 0x5e));
        fxch(cpu, 1);
        fadd(cpu, f32(app, e + 0x62));
        fxch(cpu, 1);
        fld(cpu, f64(app, 0x536e3c));
        fxch(cpu, 1);
        fmul(cpu, cpu.fpu.st(1));
        fxch(cpu, 2);
        fmul(cpu, cpu.fpu.st(1));
        fld(cpu, f32(app, e + 0x4e));
        cpu.edi = e + 0x3a;
        fadd(cpu, f32(app, e + 0x66));
        cpu.esi = e + 0x6a;
        fmulp(cpu, 2);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        fxch(cpu, 2);
        fst(app, cpu, e + 0x6a);
        fmul(cpu, f64(app, 0x536e44));
        fxch(cpu, 2);
        fstp(app, cpu, e + 0x6e);
        fstp(app, cpu, e + 0x72);
        fistpToEax(app, cpu);
        fld(cpu, f32(app, e + 0x6e));
        fmul(cpu, f64(app, 0x536e44));
        put(app, e + 0x52, cpu.eax);
        fistpToEax(app, cpu);
        fld(cpu, f32(app, e + 0x72));
        fmul(cpu, f64(app, 0x536e44));
        cpu.edx = e - 0x3e;
        put(app, e + 0x56, cpu.eax);
        fistpToEax(app, cpu);
        put(app, e + 0x5a, cpu.eax);
        cpu.eax = e + 0x52;
        if (!byAddress(app, cpu, 0x41e990))
            return;
        cpu.esi = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.edx = 1;
            cpu.eax = e - 0x3e;
            if (!byAddress(app, cpu, kSurface))
                return;
            cpu.eax &= 0x3f;
            put(app, e + 0x7a, cpu.eax);
        }
        bool outside = cpu.esi == 0;
        if (!outside)
        {
            cpu.edi = at(app, e + 0x7a);
            outside = cpu.edi == 0 || cpu.edi == 0xe || at(app, e - 0x2a) == 0 || at(app, e - 0x26) == 0;
        }
        if (outside)
        {
            cpu.edi = e + 0x46;
            cpu.esi = e + 0x6a;
        }
        else
        {
            cpu.edi = e + 0x5e;
            cpu.esi = e + 0x6a;
            movsd(app, cpu);
            movsd(app, cpu);
            movsd(app, cpu);
            cpu.edi = e + 0x46;
            cpu.esi = e + 0x3a;
            movsd(app, cpu);
            movsd(app, cpu);
            movsd(app, cpu);
            cpu.edi = e + 0x6a;
            cpu.esi = e + 0x3a;
        }
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
    } while (--cpu.ecx != 0);
    cpu.edi = at(app, e + 0x76);
    cpu.esi = e + 0x5e;
    cpu.edx = at(app, e + 0x76);
    cpu.eax = cpu.ebx;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    if (!direct(app, cpu, cameraTrackPlace))
        return;
    leave();
}

namespace
{

/* sar r, 0x18; shl r, 9: a node's signed byte at its top, as the code
 * scales it. */
reg32 nodeByte(reg32 r)
{
    return reg32(sreg32(r) >> 0x18) << 9;
}

/* 0x421e80's and 0x421f90's body: the place edx of camera eax moved by the
 * node's sideways vector (its bytes at +0x15.., scaled by `scale` and
 * `across`), kept on the track from there (keepOnTrack) and moved back --
 * `toward` the wall on one side, the other way on the other. */
void keepFromWall(App* app, CPU& cpu, reg32 scale, reg32 across, bool right)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const reg32 e = cpu.ebp;
    // fsub on the right's side where the left's has fadd, and the other way.
    const auto toward = [&](const Float& m) { right ? fsub(cpu, m) : fadd(cpu, m); };
    const auto away = [&](const Float& m) { right ? fadd(cpu, m) : fsub(cpu, m); };
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x84);
    cpu.eax = cpu.edx * 8;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x15);
    cpu.edx = nodeByte(cpu.edx);
    put(app, e - 4, cpu.edx);
    fld(cpu, f64(app, scale));
    fild(app, cpu, e - 4);
    fmul(cpu, cpu.fpu.st(1));
    cpu.edx = at(app, cpu.eax + 0x16);
    fld(cpu, f64(app, across));
    fxch(cpu, 1);
    fmul(cpu, cpu.fpu.st(1));
    cpu.edx = reg32(sreg32(cpu.edx) >> 0x18);
    cpu.eax = at(app, cpu.eax + 0x17);
    cpu.edx <<= 9;
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
    put(app, e - 4, cpu.edx);
    cpu.eax <<= 9;
    fild(app, cpu, e - 4);
    put(app, e - 4, cpu.eax);
    fmul(cpu, cpu.fpu.st(3));
    fild(app, cpu, e - 4);
    fmulp(cpu, 4);
    fmul(cpu, cpu.fpu.st(2));
    fxch(cpu, 3);
    fmulp(cpu, 2);
    fld(cpu, f32(app, cpu.ecx));
    fld(cpu, f32(app, cpu.ecx + 4));
    fld(cpu, f32(app, cpu.ecx + 8));
    fld(cpu, f32(app, cpu.ecx));
    fxch(cpu, 4);
    fstp(app, cpu, e - 0x1c);
    cpu.edx = e - 0x10;
    fxch(cpu, 2);
    toward(f32(app, e - 0x1c));
    fxch(cpu, 3);
    away(f32(app, e - 0x1c));
    fxch(cpu, 3);
    fstp(app, cpu, e - 0x28);
    fxch(cpu, 2);
    fstp(app, cpu, e - 0x10);
    cpu.eax = cpu.ebx;
    fld(cpu, f32(app, cpu.ecx + 4));
    fxch(cpu, 4);
    fstp(app, cpu, e - 0x18);
    fxch(cpu, 2);
    fstp(app, cpu, e - 0x14);
    toward(f32(app, e - 0x18));
    fxch(cpu, 2);
    away(f32(app, e - 0x18));
    fxch(cpu, 2);
    fstp(app, cpu, e - 0x24);
    fxch(cpu, 1);
    fstp(app, cpu, e - 0xc);
    toward(f32(app, e - 0x14));
    fld(cpu, f32(app, cpu.ecx + 8));
    away(f32(app, e - 0x14));
    fxch(cpu, 1);
    fstp(app, cpu, e - 0x20);
    fstp(app, cpu, e - 8);
    if (!direct(app, cpu, cameraTrackPlace))
        return;
    cpu.edx = at(app, e - 0x20);
    push(app, cpu, cpu.edx);
    cpu.esi = at(app, e - 0x24);
    push(app, cpu, cpu.esi);
    cpu.edi = at(app, e - 0x28);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ebx;
    cpu.edx = e - 0x10;
    if (!direct(app, cpu, keepOnTrack))
        return;
    fld(cpu, f32(app, e - 0x10));
    toward(f32(app, e - 0x1c));
    fstp(app, cpu, cpu.ecx);
    fld(cpu, f32(app, e - 0xc));
    toward(f32(app, e - 0x18));
    cpu.edx = cpu.ecx;
    fstp(app, cpu, cpu.ecx + 4);
    fld(cpu, f32(app, e - 8));
    toward(f32(app, e - 0x14));
    cpu.eax = cpu.ebx;
    fstp(app, cpu, cpu.ecx + 8);
    if (!direct(app, cpu, cameraTrackPlace))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x421e80: camera eax's place edx kept from the track's wall on one side
 * (keepFromWall, by 0x536e4c and 0x536e54). */
void keepFromRightWall(App* app, CPU& cpu)
{
    keepFromWall(app, cpu, 0x536e4c, 0x536e54, true);
}

/* 0x421f90: and on the other (0x536e5c, 0x536e64). */
void keepFromLeftWall(App* app, CPU& cpu)
{
    keepFromWall(app, cpu, 0x536e5c, 0x536e64, false);
}

/* 0x4220a0: camera eax's place edx kept above the ground under it: the car's
 * plane's height there (0x496a50, 0x4968a0) where it follows a car on a known
 * surface, else the node's plane's -- its place and two of its vectors from
 * its bytes (scaled by 0x536e74) -- plus 0x536e6c: no lower than that. */
void keepAboveGround(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x6c;
    const reg32 e = cpu.ebp;
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax += 0x74;
    cpu.edx = 1;
    cpu.esi = at(app, cpu.eax + 0x10);
    if (!byAddress(app, cpu, kSurface))
        return;
    cpu.edx = at(app, cpu.ebx + 0x88);
    cpu.eax &= 0x3f;
    if (cpu.edx != 0 && at(app, cpu.ebx + 0x8c) != 0 && cpu.eax != 0 && cpu.eax != 0xe)
    {
        cpu.edi = e - 0x6c;
        cpu.esi = at(app, cpu.ebx + 0xc8);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = e - 0x60;
        cpu.esi = at(app, cpu.ebx + 0xcc);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = e - 0x54;
        cpu.esi = at(app, cpu.ebx + 0xd0);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = e - 0x48;
        cpu.esi = at(app, cpu.ebx + 0xd4);
        cpu.edx = e - 0x3c;
        cpu.eax = e - 0x6c;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        if (!byAddress(app, cpu, 0x496a50))
            return;
        cpu.eax = at(app, cpu.ecx + 8);
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, cpu.ecx + 4);
        push(app, cpu, cpu.edx);
        cpu.esi = at(app, cpu.ecx);
        cpu.eax = at(app, cpu.ebx + 0xc8);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.eax + 8);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.eax + 4);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, cpu.eax);
        push(app, cpu, cpu.ebx);
    }
    else
    {
        cpu.eax = cpu.esi * 8;
        cpu.eax += cpu.esi;
        cpu.edx = at(app, kNodes);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        // Each of the node's bytes into [ebp - 4] and onto the x87's stack.
        const auto nodeWord = [&](reg32 k) {
            cpu.edx = nodeByte(at(app, cpu.eax + k));
            put(app, e - 4, cpu.edx);
            fild(app, cpu, e - 4);
        };
        cpu.edx = nodeByte(at(app, cpu.eax + 0x15));
        put(app, e - 4, cpu.edx);
        fld(cpu, f64(app, 0x536e74));
        fild(app, cpu, e - 4);
        fmul(cpu, cpu.fpu.st(1));
        fild(app, cpu, cpu.eax);
        fild(app, cpu, cpu.eax + 4);
        fild(app, cpu, cpu.eax + 8);
        fxch(cpu, 3);
        fstp(app, cpu, e - 0x3c);
        nodeWord(0x16);
        fmul(cpu, cpu.fpu.st(4));
        fstp(app, cpu, e - 0x38);
        nodeWord(0x17);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 2);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 2);
        fstp(app, cpu, e - 0x34);
        nodeWord(0xd);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 1);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 1);
        fstp(app, cpu, e - 0x30);
        nodeWord(0xe);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 3);
        fmul(cpu, cpu.fpu.st(4));
        fxch(cpu, 3);
        fstp(app, cpu, e - 0x2c);
        nodeWord(0xf);
        fmulp(cpu, 4);
        fxch(cpu, 1);
        fstp(app, cpu, e - 0x18);
        fstp(app, cpu, e - 0x14);
        fstp(app, cpu, e - 0x10);
        fstp(app, cpu, e - 0x28);
        cpu.edx = nodeByte(at(app, cpu.eax + 0x11));
        put(app, e - 4, cpu.edx);
        fld(cpu, f64(app, 0x536e74));
        fild(app, cpu, e - 4);
        fmul(cpu, cpu.fpu.st(1));
        fstp(app, cpu, e - 0x24);
        nodeWord(0x12);
        fmul(cpu, cpu.fpu.st(1));
        fstp(app, cpu, e - 0x20);
        cpu.eax = nodeByte(at(app, cpu.eax + 0x13));
        put(app, e - 4, cpu.eax);
        fild(app, cpu, e - 4);
        fmulp(cpu, 1);
        fstp(app, cpu, e - 0x1c);
        cpu.esi = at(app, cpu.ecx + 8);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.ecx + 4);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.ecx);
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, e - 0x10);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, e - 0x14);
        push(app, cpu, cpu.ebx);
        cpu.esi = at(app, e - 0x18);
        push(app, cpu, cpu.esi);
    }
    cpu.eax = e - 0x3c;
    if (!byAddress(app, cpu, 0x4968a0))
        return;
    fadd(cpu, f64(app, 0x536e6c));
    fstp(app, cpu, e - 8);
    fld(cpu, f32(app, cpu.ecx + 4));
    const Flags f = fcomp(cpu, f32(app, e - 8));
    cpu.eax = f.cf || f.zf ? at(app, e - 8) : at(app, cpu.ecx + 4);
    put(app, e - 0xc, cpu.eax);
    cpu.eax = at(app, e - 0xc);
    put(app, cpu.ecx + 4, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x422290: camera eax made to look at its target from its place edx: the
 * ways from the target's place (+8's +0x98) to the place and to the camera's
 * look-at point (+0x44) (0x4e0000), the second normalized as the camera's
 * forward (0x4e01c0, its length to the frame), the up (0x4212d0, or the
 * camera's own +0x38 where the forward is too steep, 0x536e7c) crossed with
 * it into the side and the up (0x4e01a0, 0x4e0160); then the camera's roll
 * (+0x50, 0x4ea680) and for mode 10 its car's turn (0x4eaab0). */
void lookAtTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x40;
    const reg32 e = cpu.ebp;
    put(app, e - 4, cpu.eax);
    put(app, e - 0x10, cpu.edx);
    cpu.eax += 0x2c;
    put(app, e - 0x14, cpu.eax);
    cpu.eax = at(app, e - 4);
    cpu.eax += 0x38;
    put(app, e - 0x18, cpu.eax);
    cpu.eax = at(app, e - 4);
    cpu.edi = e - 0x40;
    cpu.eax += 0x44;
    cpu.esi = 0x4212d0;
    put(app, e - 8, cpu.eax);
    cpu.eax = at(app, e - 4);
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.eax = at(app, cpu.eax + 0x98);
    put(app, e - 0x28, cpu.eax);
    cpu.eax = at(app, e - 4);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.eax = at(app, cpu.eax + 0x9c);
    put(app, e - 0x24, cpu.eax);
    cpu.eax = at(app, e - 4);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.ecx = e - 0x34;
    cpu.ebx = cpu.edx;
    cpu.eax = at(app, cpu.eax + 0xa0);
    cpu.edx = e - 0x28;
    put(app, e - 0x20, cpu.eax);
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.ecx = at(app, e - 8);
    cpu.ebx = at(app, e - 0x10);
    cpu.edx = e - 0x28;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.edx = at(app, e - 8);
    cpu.ecx = e - 0x1c;
    cpu.eax = cpu.edx;
    cpu.ebx = at(app, e - 8);
    if (!byAddress(app, cpu, kNormalize))
        return;
    cpu.edx = e - 0x40;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kMatrix))
        return;
    fld(cpu, 0.0);
    Flags f = fcomp(cpu, f32(app, e - 0x1c));
    if (!f.cf && !f.zf)
    {
        fld(cpu, f32(app, e - 0x1c));
        fchs(cpu);
        fstp(app, cpu, e - 0xc);
    }
    else
    {
        cpu.eax = at(app, e - 0x1c);
        put(app, e - 0xc, cpu.eax);
    }
    fld(cpu, f32(app, e - 0xc));
    f = fcomp(cpu, f64(app, 0x536e7c));
    if (!f.cf && !f.zf)
    {
        cpu.edi = e - 0x40;
        cpu.esi = at(app, e - 0x18);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
    }
    cpu.ebx = at(app, e - 0x14);
    cpu.edx = at(app, e - 8);
    cpu.eax = e - 0x40;
    if (!byAddress(app, cpu, kCrossNormal))
        return;
    cpu.ebx = at(app, e - 0x18);
    cpu.edx = at(app, e - 0x14);
    cpu.eax = at(app, e - 8);
    if (!byAddress(app, cpu, kCross))
        return;
    cpu.edx = at(app, e - 4);
    cpu.edx += 0x2c;
    cpu.eax = at(app, e - 4);
    push(app, cpu, cpu.edx);
    push(app, cpu, at(app, cpu.eax + 0x50));
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, 0x4ea680))
        return;
    cpu.eax = at(app, e - 4);
    cpu.ecx = at(app, cpu.eax);
    if (cpu.ecx == 0xa)
    {
        cpu.eax = at(app, cpu.eax + 0xc);
        push(app, cpu, cpu.edx);
        cpu.eax = at(app, cpu.eax * 4 + 0x552f18);
        push(app, cpu, at(app, cpu.eax + 0x11c));
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x4eaab0))
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

/* 0x4223c0: player ebx's view (orientation eax, place edx) blended from the
 * camera it leaves into the one it takes (mode 0x13's record, 0x552f60): a
 * logistic weight of its time (+0x110, by 0x536e84 and 0x536e88, through
 * fprem, f2xm1 and fscale), the orientations as quaternions (0x4b7590,
 * 0x4b73c0 -- the old one turned round first, turnAround, where ecx is 1)
 * slerped (0x4b7300, 0x4b71e0) back into eax, the places mixed (0x4dffb0,
 * 0x4e0050) into edx. */
void blendCamera(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xb0;
    cpu.ebp -= 0x82;
    const reg32 e = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp + 0x82;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, e + 0x7a, cpu.eax);
    put(app, e + 0x76, cpu.edx);
    cpu.edx = at(app, cpu.ebx * 4 + 0x552f60);
    cpu.esi = at(app, cpu.edx + 0x118);
    put(app, e + 0x72, cpu.edx);
    if (cpu.esi == 0 || at(app, cpu.edx + 0x11c) == 0)
        return leave();
    fld(cpu, f32(app, cpu.edx + 0x110));
    fadd(cpu, f32(app, 0x536e84));
    fmul(cpu, f32(app, 0x536e88));
    fld(cpu, 1.0);
    fld(cpu, 1.4426950408889634);
    fmul(cpu, fpu.st(2));
    fpu.st(2) = Float(fpu.st(0));
    fpu.st(0) = fpu.rem(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::f2xm1(fpu.st(0));
    faddp(cpu, 1);
    fpu.st(0) = fp::scale(fpu.st(0), fpu.st(1));
    fpu.st(1) = Float(fpu.st(0));
    fpu.pop();
    fld(cpu, 1.0);
    faddp(cpu, 1);
    fld(cpu, 1.0);
    fdivrp(cpu, 1);
    fld(cpu, 1.0);
    cpu.edi = e + 0x3a;
    cpu.esi = cpu.edx + 0x120;
    fxch(cpu, 1);
    fstp(app, cpu, e + 0x7e);
    fsub(cpu, f32(app, e + 0x7e));
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    fstp(app, cpu, e + 0x6e);
    if (cpu.ecx == 1)
    {
        cpu.edx = e - 0xa;
        cpu.eax = e + 0x3a;
        if (!byAddress(app, cpu, 0x4b7590))
            return;
        cpu.edx = e - 0x2e;
        cpu.eax = e - 0xa;
        if (!direct(app, cpu, turnAround))
            return;
        cpu.edx = e + 0x3a;
        cpu.eax = e - 0x2e;
        if (!byAddress(app, cpu, 0x4b73c0))
            return;
    }
    cpu.edx = e + 0x2a;
    cpu.eax = at(app, e + 0x7a);
    if (!byAddress(app, cpu, 0x4b73c0))
        return;
    cpu.edx = e + 0x2a;
    cpu.eax = e + 0x3a;
    if (!byAddress(app, cpu, 0x4b7300))
        return;
    cpu.eax = e + 0x1a;
    push(app, cpu, cpu.eax);
    cpu.edx = e + 0x2a;
    cpu.eax = e + 0x3a;
    push(app, cpu, at(app, e + 0x7e));
    if (!byAddress(app, cpu, 0x4b71e0))
        return;
    cpu.edx = at(app, e + 0x7a);
    cpu.eax = e + 0x1a;
    if (!byAddress(app, cpu, 0x4b7590))
        return;
    cpu.eax = at(app, e + 0x72);
    cpu.eax = at(app, cpu.eax + 0x11c);
    cpu.ebx <<= 2;
    cpu.edx = at(app, e + 0x72);
    cpu.ebx = at(app, cpu.ebx + cpu.eax * 8 + kCameras);
    cpu.ecx = e + 0x62;
    cpu.edx += 0x130;
    cpu.ebx = at(app, cpu.ebx + 4);
    cpu.eax = 1;
    cpu.ebx += 0x98;
    if (!byAddress(app, cpu, kRotate))
        return;
    cpu.eax = e + 0x4a;
    push(app, cpu, cpu.eax);
    cpu.edx = e + 0x62;
    cpu.eax = 1;
    push(app, cpu, at(app, e + 0x6e));
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    cpu.eax = e + 0x56;
    push(app, cpu, cpu.eax);
    cpu.ecx = at(app, e + 0x76);
    cpu.edx = at(app, e + 0x76);
    push(app, cpu, at(app, e + 0x7e));
    cpu.eax = 1;
    cpu.ebx = e + 0x56;
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    cpu.edx = e + 0x4a;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kRotate))
        return;
    leave();
}

/* 0x422520: camera eax placed for the frame: its head turned (headTurn) if
 * it turns, its place (+0x14) to ebx and onto the track, then kept from the
 * walls, on the track, above the road and the ground (+0xf4), out of the
 * cars (+0xf8), looking at its target (+0x108); its orientation to edx --
 * its car's turned round where ecx is 1, else its own -- and its track place
 * found again. */
void placeCamera(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 camera = cpu.ebp - 4, matrix = cpu.ebp - 8;
    put(app, camera, cpu.eax);
    put(app, matrix, cpu.edx);
    if (at(app, cpu.eax + 0x104) != 0 && !direct(app, cpu, headTurn))
        return;
    cpu.eax = at(app, camera);
    cpu.edi = cpu.ebx;
    cpu.esi = cpu.eax + 0x14;
    cpu.edx = cpu.ebx;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    if (!direct(app, cpu, cameraTrackPlace))
        return;
    cpu.eax = at(app, camera);
    if (at(app, cpu.eax + 0xf4) != 0)
    {
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, keepFromRightWall))
            return;
        cpu.eax = at(app, camera);
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, keepFromLeftWall))
            return;
        cpu.eax = at(app, camera);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.edi = at(app, cpu.eax + 0xa0);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.eax + 0x9c);
        push(app, cpu, cpu.edx);
        cpu.esi = at(app, cpu.eax + 0x98);
        push(app, cpu, cpu.esi);
        cpu.eax = at(app, camera);
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, keepOnTrack))
            return;
        cpu.eax = at(app, camera);
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, keepAboveRoad))
            return;
        cpu.eax = at(app, camera);
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, keepAboveGround))
            return;
    }
    cpu.eax = at(app, camera);
    if (at(app, cpu.eax + 0xf8) != 0)
    {
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, keepOutOfCars))
            return;
    }
    cpu.eax = at(app, camera);
    if (at(app, cpu.eax + 0x108) != 0)
    {
        cpu.edx = cpu.ebx;
        if (!direct(app, cpu, lookAtTarget))
            return;
    }
    if (cpu.ecx == 1)
    {
        cpu.eax = at(app, camera);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.edx = at(app, matrix);
        cpu.eax += 0xc0;
        if (!direct(app, cpu, turnAround))
            return;
    }
    else
    {
        cpu.eax = at(app, camera);
        cpu.ecx = 9;
        cpu.edi = at(app, matrix);
        cpu.esi = cpu.eax + 0x2c;
        repMovsd(app, cpu);
    }
    cpu.eax = at(app, camera);
    cpu.edx = cpu.ebx;
    if (!direct(app, cpu, cameraTrackPlace))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x422610: player edx's camera switched to mode eax, blending: where the
 * mode may follow the camera in use, with the same cars and the player's own
 * car, the switch kept on mode 0x13's record (0x552f60: the mode, the start's
 * orientation +0x120 as a quaternion, its place's offset +0x130, the time
 * +0x110 cleared, the mode left +0x118), the modes' sounds (0x426450) and
 * mode 0x13 taken; else straight to the mode (setCameraMode). */
void switchCamera(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x60;
    const reg32 e = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto straight = [&] {
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, setCameraMode))
            return;
        leave();
    };
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.eax = cpu.edx * 4;
    cpu.edx = at(app, cpu.eax + 0x552f60);
    put(app, e - 8, cpu.edx);
    cpu.edx = at(app, cpu.eax + kCamera);
    cpu.eax = cpu.edx != 0 ? at(app, cpu.edx) : 0;
    cpu.ecx = at(app, kPlayers);
    put(app, e - 4, cpu.eax);
    cpu.ebx = cpu.ecx == 1 ? cpu.esi : at(app, kSecondPlayer);
    cpu.edx = at(app, e - 4);
    cpu.eax = cpu.esi * 4;
    cpu.edx = at(app, cpu.eax + cpu.edx * 8 + kCameras);
    if (byte(app, cpu.edx + cpu.edi + 0x58) == 0)
        return straight();
    cpu.edx = at(app, cpu.eax + cpu.edi * 8 + kCameras);
    cpu.eax = at(app, cpu.eax + kCamera);
    cpu.ecx = at(app, cpu.eax + 4);
    if (cpu.ecx != at(app, cpu.edx + 4))
        return straight();
    cpu.ecx = at(app, cpu.eax + 8);
    if (cpu.ecx != at(app, cpu.edx + 8))
        return straight();
    if (cpu.ecx != at(app, cpu.ebx * 4 + kCars))
        return straight();
    cpu.ecx = at(app, e - 4);
    if (cpu.ecx == 0x13)
        return straight();
    cpu.eax = at(app, e - 8);
    put(app, cpu.eax + 0x11c, cpu.edi);
    if (cpu.ecx != 0x10 && cpu.ecx != 0x18)
    {
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kModeSound))
            return;
    }
    cpu.eax = at(app, e - 4);
    cpu.ebx = e - 0x2c;
    cpu.edx = cpu.esi * 4;
    cpu.eax <<= 3;
    put(app, e - 0x14, cpu.edx);
    cpu.edx += cpu.eax;
    cpu.ecx = 0;
    put(app, e - 0x10, cpu.edx);
    cpu.eax = at(app, cpu.edx + kCameras);
    cpu.edx = e - 0x60;
    if (!direct(app, cpu, placeCamera))
        return;
    cpu.eax = at(app, e - 8);
    cpu.eax += 0x120;
    put(app, e - 0xc, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = e - 0x60;
    if (!byAddress(app, cpu, 0x4b73c0))
        return;
    cpu.edx = cpu.esi;
    cpu.eax = cpu.edi;
    if (!byAddress(app, cpu, kModeSound))
        return;
    cpu.edx = at(app, e - 0x14);
    cpu.eax = cpu.edi * 8;
    cpu.ebx = e - 0x20;
    cpu.eax += cpu.edx;
    cpu.ecx = 0;
    cpu.edx = e - 0x60;
    cpu.eax = at(app, cpu.eax + kCameras);
    if (!direct(app, cpu, placeCamera))
        return;
    cpu.eax = at(app, e - 0x10);
    cpu.eax = at(app, cpu.eax + kCameras);
    cpu.ecx = at(app, e - 8);
    cpu.eax = at(app, cpu.eax + 4);
    cpu.edx = e - 0x2c;
    cpu.ebx = cpu.eax + 0x98;
    cpu.ecx += 0x130;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.edx = e - 0x3c;
    cpu.eax = e - 0x60;
    if (!byAddress(app, cpu, 0x4b73c0))
        return;
    cpu.edx = at(app, e - 0xc);
    cpu.eax = e - 0x3c;
    if (!byAddress(app, cpu, 0x4b7300))
        return;
    cpu.edx = 0x13;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, setCameraMode))
        return;
    cpu.eax = at(app, e - 8);
    cpu.edx = at(app, e - 8);
    put(app, cpu.eax + 0x110, 0);
    cpu.eax = at(app, e - 4);
    put(app, cpu.edx + 0x118, cpu.eax);
    leave();
}

/* 0x4227e0: camera eax's zoom onto the x87's stack: for mode 0 its player's
 * camera's zoom (+0x54) where that is not 0, else 1.0. */
void cameraZoom(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 zoom = cpu.ebp - 4;
    bool own = false;
    if (at(app, cpu.eax) == 0)
    {
        cpu.edx = at(app, cpu.eax + 4);
        cpu.edx <<= 2;
        cpu.ecx = at(app, cpu.edx + kOverride);
        cpu.edx = cpu.ecx != 0 ? cpu.ecx : at(app, cpu.edx + kCamera);
        if ((at(app, cpu.edx + 0x54) & 0x7fffffff) != 0)
        {
            cpu.eax = at(app, cpu.eax + 4);
            cpu.eax <<= 2;
            cpu.esi = at(app, cpu.eax + kOverride);
            cpu.eax = cpu.esi != 0 ? cpu.esi : at(app, cpu.eax + kCamera);
            cpu.eax = at(app, cpu.eax + 0x54);
            put(app, zoom, cpu.eax);
            own = true;
        }
    }
    if (!own)
        put(app, zoom, 0x3f800000);
    fld(cpu, f32(app, zoom));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The code's inlined product of the vector at eax with the matrix at edx
 * (its columns), into ebx: three rows of products summed on the x87's stack,
 * eax stepped past the vector. */
void vectorTimesMatrix(App* app, CPU& cpu)
{
    const reg32 v = cpu.eax, m = cpu.edx;
    fld(cpu, f32(app, v));
    fmul(cpu, f32(app, m));
    fld(cpu, f32(app, v));
    fmul(cpu, f32(app, m + 4));
    fld(cpu, f32(app, v));
    fmul(cpu, f32(app, m + 8));
    fxch(cpu, 2);
    fld(cpu, f32(app, v + 4));
    fmul(cpu, f32(app, m + 0xc));
    fld(cpu, f32(app, v + 4));
    fmul(cpu, f32(app, m + 0x10));
    fld(cpu, f32(app, v + 4));
    fmul(cpu, f32(app, m + 0x14));
    fxch(cpu, 2);
    faddp(cpu, 3);
    faddp(cpu, 3);
    faddp(cpu, 3);
    fld(cpu, f32(app, v + 8));
    fmul(cpu, f32(app, m + 0x18));
    fld(cpu, f32(app, v + 8));
    fmul(cpu, f32(app, m + 0x1c));
    fld(cpu, f32(app, v + 8));
    fmul(cpu, f32(app, m + 0x20));
    fxch(cpu, 2);
    faddp(cpu, 3);
    faddp(cpu, 3);
    faddp(cpu, 3);
    cpu.eax = v + 0xc;
    fstp(app, cpu, cpu.ebx);
    fstp(app, cpu, cpu.ebx + 4);
    fstp(app, cpu, cpu.ebx + 8);
}

}  // namespace

/* 0x422850: the view eax's matrix: its orientation (+0xc) transposed to +0x3c
 * (0x4e0110), its place negated and turned by it into +0x30. */
void viewMatrix(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.eax + 0x3c;
    cpu.eax += 0xc;
    cpu.edx = cpu.esi;
    cpu.ebx = cpu.ecx + 0x30;
    if (!byAddress(app, cpu, kTranspose))
        return;
    fld(cpu, f32(app, cpu.ecx));
    cpu.eax = cpu.ebp - 0xc;
    fchs(cpu);
    cpu.edx = cpu.esi;
    fstp(app, cpu, cpu.ebp - 0xc);
    fld(cpu, f32(app, cpu.ecx + 4));
    fchs(cpu);
    fstp(app, cpu, cpu.ebp - 8);
    fld(cpu, f32(app, cpu.ecx + 8));
    fchs(cpu);
    fstp(app, cpu, cpu.ebp - 4);
    vectorTimesMatrix(app, cpu);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4228e0: player eax's view edx updated: where its camera is blending
 * (mode 0x13), the camera of the mode it goes to stands in while it is
 * placed; the camera (the overriding one, if any) placed (placeCamera, ebx
 * its turn-round flag) into the view's place and orientation, the blend
 * applied (blendCamera); the view's matrix (viewMatrix), and the view kept
 * as the player's (0x5dd950) where ebx is 0. */
void updateCamera(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 flag = cpu.ebp - 4, blending = cpu.ebp - 8, saved = cpu.ebp - 0xc;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    put(app, flag, cpu.ebx);
    cpu.edx = 0;
    cpu.ecx = at(app, kPlayers);
    put(app, blending, cpu.edx);
    if (cpu.ecx == 1)
        cpu.edi &= cpu.ecx;
    else
        cpu.edi ^= cpu.eax;
    cpu.eax = cpu.edi * 4;
    cpu.ebx = at(app, cpu.eax + kCamera);
    if (cpu.ebx != 0)
    {
        cpu.ecx = cpu.ebx;
        cpu.edx = at(app, cpu.ebx);
        if (cpu.edx == 0x13)
        {
            put(app, blending, 1);
            cpu.edx = at(app, cpu.eax + 0x552f60);
            cpu.edx = at(app, cpu.edx + 0x11c);
            cpu.edx = at(app, cpu.eax + cpu.edx * 8 + kCameras);
            put(app, saved, cpu.ebx);
            put(app, cpu.eax + kCamera, cpu.edx);
        }
    }
    cpu.ecx = at(app, flag);
    cpu.eax = cpu.edi * 4;
    cpu.ebx = cpu.esi;
    cpu.edx = cpu.esi + 0xc;
    cpu.eax = at(app, cpu.eax + kOverride) != 0 ? at(app, cpu.eax + kOverride) : at(app, cpu.eax + kCamera);
    if (!direct(app, cpu, placeCamera))
        return;
    if (at(app, blending) != 0)
    {
        cpu.ecx = at(app, flag);
        cpu.eax = cpu.esi + 0xc;
        cpu.ebx = cpu.edi;
        cpu.edx = cpu.esi;
        if (!direct(app, cpu, blendCamera))
            return;
        cpu.edx = at(app, saved);
        put(app, cpu.edi * 4 + kCamera, cpu.edx);
    }
    cpu.eax = cpu.esi;
    cpu.ebx = at(app, flag);
    if (!direct(app, cpu, viewMatrix))
        return;
    if (cpu.ebx == 0)
    {
        cpu.eax = cpu.edi * 4;
        cpu.eax -= cpu.edi;
        cpu.eax <<= 5;
        cpu.ecx = 0x18;
        cpu.edi = cpu.eax + kViews;
        repMovsd(app, cpu);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4229c0: player eax's view (0x5dd950, 0x60 bytes) copied to edx, and its
 * camera's place -- the overriding camera's, the mode a blend goes to, or the
 * camera in use -- over its first 12.  ebx left as the code has it. */
void cameraView(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 player = cpu.ebp - 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, player, cpu.eax);
    cpu.ecx = at(app, kPlayers);
    if (cpu.ecx == 1)
    {
        cpu.ebx = cpu.eax;
        cpu.ebx &= cpu.ecx;
        put(app, player, cpu.ebx);
    }
    else
    {
        cpu.esi = 0;
        put(app, player, cpu.esi);
    }
    cpu.ebx = at(app, player);
    cpu.eax = cpu.ebx * 4;
    cpu.eax -= cpu.ebx;
    cpu.ecx = 0x18;
    cpu.eax <<= 5;
    cpu.edi = cpu.edx;
    cpu.esi = cpu.eax + kViews;
    cpu.eax = cpu.ebx * 4;
    repMovsd(app, cpu);
    cpu.edi = at(app, cpu.eax + kOverride);
    if (cpu.edi != 0)
        cpu.ebx = at(app, cpu.edi);
    else
    {
        cpu.ebx = at(app, cpu.eax + kCamera);
        if (cpu.ebx != 0)
            cpu.ebx = at(app, cpu.ebx);
    }
    cpu.eax = at(app, player);
    cpu.eax <<= 2;
    if (cpu.ebx == 0x13)
    {
        cpu.ebx = at(app, cpu.eax + 0x552f60);
        cpu.ebx = at(app, cpu.ebx + 0x11c);
        cpu.eax = at(app, cpu.eax + cpu.ebx * 8 + kCameras);
    }
    else
    {
        cpu.esi = at(app, cpu.eax + kOverride);
        cpu.eax = cpu.esi != 0 ? cpu.esi : at(app, cpu.eax + kCamera);
    }
    cpu.edi = cpu.edx;
    cpu.esi = cpu.eax + 0x14;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    leave();
}

/* 0x422aa0: whether player eax sees from inside its car (1) or not (0): a
 * screen 320 wide or more, the car there and not one without a cockpit
 * (+0x200 bit 2), no replay (0x7a3a58), and -- unless 0x559264 says always --
 * not in a network race (0x6fd3a0 2), the player not out (0x79c4a0), nothing
 * forcing the outside (0x55fe30); then by the camera's mode (that of a blend's
 * target for 0x13), 1 to 6 by the table at 0x422a7c. */
void cockpitShown(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, cameraCar))
        return;
    cpu.ecx = at(app, kPlayers);
    cpu.edx = cpu.ecx == 1 ? cpu.edx & cpu.ecx : 0;
    if (sreg32(at(app, 0x7d3684)) >= 0x140 && cpu.eax != 0 && (byte(app, cpu.eax + 0x200) & 2) == 0
        && (app->getMemory<reg16>(0x7a3a58) & 0x180) == 0)
    {
        if (at(app, 0x559264) != 0)
        {
            cpu.eax = 1;
            return leave();
        }
        if (at(app, 0x6fd3a0) != 2)
        {
            cpu.ecx = cpu.edx * 4;
            cpu.eax = at(app, cpu.ecx + 0x79c4a0);
            if (cpu.eax == 0)
            {
                if (at(app, 0x55fe30) != 0)
                    return leave();
                cpu.eax = cpu.edx;
                if (!direct(app, cpu, cameraMode))
                    return;
                cpu.esi = at(app, cpu.ecx + kCamera);
                if (cpu.esi != 0)
                {
                    cpu.edx = at(app, cpu.esi);
                    if (cpu.edx == 0x13)
                    {
                        cpu.eax = at(app, cpu.ecx + 0x552f60);
                        cpu.eax = at(app, cpu.eax + 0x11c);
                    }
                }
                --cpu.eax;
                if (cpu.eax <= 5)
                {
                    // jmp [eax*4 + 0x422a7c]: every one of its six to 0x422afb.
                    cpu.eax = 1;
                    return leave();
                }
            }
        }
    }
    cpu.eax = 0;
    leave();
}

/* 0x422ba0: the car player eax's camera rides with (+4). */
void cameraCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kPlayers);
    cpu.eax = cpu.edx == 1 ? cpu.eax & cpu.edx : 0;
    cpu.eax = at(app, cpu.eax * 4 + kCamera);
    cpu.eax = at(app, cpu.eax + 4);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x422bd0: the car player eax's camera looks at (+8). */
void cameraTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kPlayers);
    cpu.eax = cpu.edx == 1 ? cpu.eax & cpu.edx : 0;
    cpu.eax = at(app, cpu.eax * 4 + kCamera);
    cpu.eax = at(app, cpu.eax + 8);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* A store of initCameras' switch: a dword, or a byte, at a camera's offset. */
struct Store
{
    reg32 offset, value;
    bool byte;
};

constexpr Store kEnd = {0, 0, false};

/* Each mode's settings, as the switch's cases at 0x422c88 store them, in
 * their order: modes 1 to 24 (15 and 20 have none; 16 is written out in
 * initCameras, it reads the race's settings). */
const Store kMode1[] = {{0x110, 0}, {0x114, 0x3f19999a}, {0x118, 0xbf800000}, {0x50, 0}, {0x104, 1}, {0x108, 0},
                        {0xf4, 0}, {0xf0, 0}, {0x69, 1, true}, kEnd};
const Store kMode2[] = {{0x110, 0}, {0x114, 0x3f19999a}, {0x118, 0x3f800000}, {0x50, 0x3cf5c28f}, {0x104, 2},
                        {0x108, 0}, {0xf4, 0}, {0xf0, 0}, {0x69, 1, true}, kEnd};
const Store kMode3[] = {{0x110, 0}, {0x114, 0x3fd33333}, {0x118, 0xc0b00000}, {0x11c, 0x3e800000},
                        {0x120, 0x3e4ccccd}, {0x50, 0x3d23d70a}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1},
                        {0xf8, 1}, {0x63, 1, true}, {0x5d, 1, true}, {0x5e, 1, true}, {0x5c, 1, true},
                        {0x69, 1, true}, {0x6d, 1, true}, {0x70, 1, true}, {0x64, 1, true}, {0x65, 1, true},
                        {0x5f, 1, true}, kEnd};
const Store kMode4[] = {{0x110, 0}, {0x114, 0x3f800000}, {0x118, 0xc0800000}, {0x11c, 0x3e800000},
                        {0x120, 0x3e4ccccd}, {0x50, 0x3cf5c28f}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1},
                        {0xf8, 1}, {0x5b, 1, true}, {0x5d, 1, true}, {0x5e, 1, true}, {0x69, 1, true},
                        {0x6d, 1, true}, {0x70, 1, true}, kEnd};
const Store kMode5[] = {{0x110, 0}, {0x114, 0x3fd33333}, {0x118, 0xc0980000}, {0x11c, 0x40000000}, {0x120, 0},
                        {0x50, 0x3d23d70a}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1}, {0xf8, 1},
                        {0x5c, 1, true}, {0x5b, 1, true}, {0x5e, 1, true}, {0x69, 1, true}, {0x6d, 1, true},
                        {0x70, 1, true}, kEnd};
const Store kMode6[] = {{0x110, 0}, {0x114, 0x3f800000}, {0x118, 0xc0500000}, {0x11c, 0x40000000}, {0x120, 0},
                        {0x50, 0x3cf5c28f}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1}, {0xf8, 1},
                        {0x5b, 1, true}, {0x5d, 1, true}, {0x5c, 1, true}, {0x69, 1, true}, {0x6d, 1, true},
                        {0x70, 1, true}, kEnd};
const Store kMode7[] = {{0x110, 0x40c00000}, {0x114, 0x40000000}, {0x118, 0x40c00000}, {0x11c, 0x3a83126f},
                        {0x120, 0xffffffff}, {0x124, 0}, {0x128, 0}, {0x50, 0}, {0x104, 0}, {0x108, 1}, {0xf4, 1},
                        {0xf0, 1}, {0xf8, 1}, {0x64, 1, true}, {0x65, 1, true}, {0x63, 1, true}, {0x69, 1, true},
                        kEnd};
const Store kMode8[] = {{0x110, 0x40c00000}, {0x12c, 1}, {0x124, 0}, {0x128, 0x3e4ccccd}, {0x130, 1}, {0x50, 0},
                        {0x104, 0}, {0x108, 0}, {0xf4, 0}, {0xf0, 0}, {0xf8, 1}, kEnd};
const Store kMode9[] = {{0x104, 0}, {0x108, 1}, {0xf4, 0}, {0xf0, 1}, {0xf8, 1}, {0x50, 0}, kEnd};
const Store kMode10[] = {{0x110, 0x40000000}, {0x114, 0x3ed58106}, {0x118, 0xc019999a}, {0x11c, 0x3e0b4396},
                         {0x50, 0x3c23d70a}, {0x104, 1}, {0x108, 1}, {0xf4, 0}, {0xf0, 1}, kEnd};
const Store kMode11[] = {{0x110, 0}, {0x114, 0x40000000}, {0x118, 0x3f19999a}, {0x11c, 0}, {0x50, 0x3e2e147b},
                         {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1}, {0x64, 1, true}, {0x65, 1, true},
                         {0x5b, 1, true}, {0x5f, 1, true}, {0x69, 1, true}, kEnd};
const Store kMode12[] = {{0x110, 0xc0200000}, {0x114, 0x3fa978d5}, {0x118, 0xc0200000}, {0x11c, 0},
                         {0x50, 0x3c54fdf4}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1}, {0x63, 1, true},
                         {0x65, 1, true}, {0x5f, 1, true}, {0x69, 1, true}, kEnd};
const Store kMode13[] = {{0x110, 0xc0200000}, {0x114, 0x3fa978d5}, {0x118, 0x40200000}, {0x11c, 0},
                         {0x50, 0x3c54fdf4}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf0, 1}, {0x63, 1, true},
                         {0x64, 1, true}, {0x5f, 1, true}, {0x69, 1, true}, kEnd};
const Store kMode14[] = {{0x110, 0x3fc00000}, {0x114, 0x3ba3d70a}, {0x118, 1}, {0x11c, 1}, {0x50, 0}, {0x104, 0},
                         {0x108, 1}, {0xf4, 0}, {0xf0, 0}, {0xf8, 1}, kEnd};
const Store kMode17[] = {{0x110, 0}, {0x114, 0}, {0x118, 0xffffffff}, {0x11c, 0}, {0x50, 0}, {0x104, 0}, {0x108, 0},
                         {0xf4, 1}, {0xf8, 1}, {0xf0, 1}, kEnd};
const Store kMode18[] = {{0x110, 1}, {0x50, 0}, {0x104, 0}, {0x108, 0}, {0xf4, 0}, {0xf0, 0}, {0xf8, 1}, kEnd};
const Store kMode19[] = {{0x110, 0}, {0x114, 0x3c23d70a}, {0x118, 0}, {0x11c, 0}, {0x50, 0}, {0x104, 0}, {0x108, 0},
                         {0xf4, 0}, {0xf0, 0}, {0xf8, 1}, kEnd};
const Store kMode21[] = {{0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf8, 1}, {0xf0, 1}, {0x50, 0x3a280000}, kEnd};
const Store kMode22[] = {{0x110, 0}, {0x114, 0x40400000}, {0x118, 0xc1200000}, {0x50, 0x3a280000}, {0x104, 0},
                         {0x108, 1}, {0xf4, 1}, {0xf8, 1}, {0xf0, 1}, kEnd};
const Store kMode23[] = {{0x110, 0x40900000}, {0x114, 0x3fc00000}, {0x118, 0x40900000}, {0x11c, 0}, {0x120, 0},
                         {0x124, 0}, {0x50, 0}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf8, 1}, {0xf0, 1}, kEnd};
const Store kMode24[] = {{0x110, 0x40c00000}, {0x114, 0x40000000}, {0x118, 0x40c00000}, {0x11c, 0}, {0x120, 0},
                         {0x50, 0x3cf5c28f}, {0x104, 0}, {0x108, 1}, {0xf4, 1}, {0xf8, 1}, {0xf0, 1},
                         {0x5b, 1, true}, {0x5c, 1, true}, {0x5d, 1, true}, {0x5e, 1, true}, kEnd};
// Mode 16's stores after its zoom's choice.
const Store kMode16[] = {{0x11c, 0}, {0x118, 0xffffffff}, {0x50, 0}, {0x104, 0}, {0x108, 0}, {0xf4, 0}, {0xf0, 0},
                         {0x5d, 1, true}, {0x5e, 1, true}, {0x5b, 1, true}, {0x5c, 1, true}, kEnd};

// The table's 24 cases (mode - 1), as 0x422c88 leads them.
const Store* const kModes[24] = {kMode1,  kMode2,  kMode3,  kMode4,  kMode5,  kMode6,  kMode7,  kMode8,
                                 kMode9,  kMode10, kMode11, kMode12, kMode13, kMode14, nullptr, kMode16,
                                 kMode17, kMode18, kMode19, nullptr, kMode21, kMode22, kMode23, kMode24};

void store(App* app, reg32 camera, const Store* s)
{
    for (; s->offset != 0; ++s)
    {
        if (s->byte)
            putByte(app, camera + s->offset, reg8(s->value));
        else
            put(app, camera + s->offset, s->value);
    }
}

}  // namespace

/* 0x422cf0: the race's cameras set up: each player's view (0x5dd950) the
 * identity at the origin (0x564498, 0x56448c), the cycle of modes built
 * (buildCameraCycle); for each player (two in a split screen) mode 0x10
 * taken, no override, and each camera of modes 1 to 24 reset onto the
 * player's car (the camera of mode 0x10 in use for a car of +0x200 bit 0x20)
 * with its mode's settings; the HUD's (0x41ffa0) and the chase's state
 * (0x553004..) cleared. */
void initCameras(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x7c;
    const reg32 split = cpu.ebp - 4;
    cpu.edx = at(app, kPlayers);
    cpu.ecx = cpu.edx == 1 ? cpu.edx : 0;
    cpu.edi = cpu.ebp - 0x7c;
    cpu.esi = 0x4212dc;
    put(app, split, cpu.ecx);
    cpu.ecx = 0x1e;
    repMovsd(app, cpu);
    cpu.ecx = 9;
    cpu.edi = 0x5dd95c;
    cpu.esi = 0x564498;
    repMovsd(app, cpu);
    cpu.edi = 0x5dd950;
    cpu.esi = 0x56448c;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.ecx = 9;
    cpu.edi = 0x5dd9bc;
    cpu.esi = 0x564498;
    repMovsd(app, cpu);
    cpu.edi = 0x5dd9b0;
    cpu.esi = 0x56448c;
    cpu.ebx = 0;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    if (!direct(app, cpu, buildCameraCycle))
        return;
    for (; sreg32(cpu.ebx) <= sreg32(at(app, split)); ++cpu.ebx)
    {
        cpu.esi = 1;
        cpu.edx = 0x10;
        cpu.edi = 0;
        cpu.eax = cpu.ebx;
        put(app, cpu.ebx * 8 + 0x5e1010, cpu.edi);
        put(app, cpu.ebx * 4 + kChanged, cpu.esi);
        put(app, cpu.ebx * 8 + 0x5e1014, cpu.edi);
        if (!direct(app, cpu, setCameraMode))
            return;
        cpu.eax = cpu.ebx;
        cpu.edx = 0;
        if (!direct(app, cpu, setOverrideMode))
            return;
        for (cpu.edx = cpu.esi; sreg32(cpu.edx) < 0x19; ++cpu.edx)
        {
            cpu.ecx = cpu.ebx * 4;
            cpu.eax = at(app, cpu.ecx + cpu.edx * 8 + kCameras);
            cpu.ecx = at(app, split);
            cpu.ecx = cpu.ecx == 0 ? at(app, kSecondPlayer) : cpu.ebx;
            cpu.esi = at(app, cpu.ecx * 4 + kPlayerCars);
            put(app, cpu.eax + 4, cpu.esi);
            put(app, cpu.eax + 8, cpu.esi);
            if ((byte(app, cpu.esi + 0x200) & 0x20) != 0)
            {
                cpu.esi = at(app, cpu.ebx * 4 + 0x552ee0);
                put(app, cpu.ebx * 4 + kCamera, cpu.esi);
            }
            put(app, cpu.eax + 0x10, 0);
            cpu.ecx = 0x1e;
            put(app, cpu.eax, cpu.edx);
            cpu.esi = cpu.ebp - 0x7c;
            cpu.edi = cpu.eax + 0x74;
            put(app, cpu.eax + 0xc, cpu.ebx);
            repMovsd(app, cpu);
            put(app, cpu.eax + 0xec, 0xffffffff);
            cpu.esi = at(app, cpu.eax + 4);
            cpu.ecx = 9;
            cpu.edi = cpu.eax + 0x2c;
            cpu.esi += 0xc0;
            repMovsd(app, cpu);
            cpu.ecx = at(app, cpu.eax + 4);
            put(app, cpu.eax + 0x50, 0);
            fld(cpu, f32(app, cpu.ecx + 0x98));
            cpu.ecx = at(app, cpu.eax + 4);
            fstp(app, cpu, cpu.eax + 0x14);
            fld(cpu, f32(app, cpu.ecx + 0x9c));
            cpu.ecx = at(app, cpu.eax + 4);
            fstp(app, cpu, cpu.eax + 0x18);
            fld(cpu, f32(app, cpu.ecx + 0xa0));
            put(app, cpu.eax + 0x20, 0);
            put(app, cpu.eax + 0x24, 0);
            put(app, cpu.eax + 0x28, 0xc0400000);
            fstp(app, cpu, cpu.eax + 0x1c);
            put(app, cpu.eax + 0x54, 0);
            // The flags of the modes it may hand over to, 25 bytes, cleared.
            cpu.ecx = 0;
            do
            {
                cpu.esi = cpu.eax + cpu.ecx;
                ++cpu.ecx;
                putByte(app, cpu.esi + 0x58, 0);
            } while (sreg32(cpu.ecx) < 0x19);
            put(app, cpu.eax + 0xf4, 0);
            put(app, cpu.eax + 0xf8, 0);
            put(app, cpu.eax + 0xfc, 0);
            put(app, cpu.eax + 0x104, 0);
            put(app, cpu.eax + 0x108, 0);
            put(app, cpu.eax + 0x10c, 0);
            cpu.ecx = cpu.edx - 1;
            put(app, cpu.eax + 0xf0, 0);
            if (cpu.ecx > 0x17)
                continue;
            const reg32 index = cpu.ecx;
            if (index == 15)
            {
                // Mode 16: its zoom by the race's settings.
                cpu.ecx = cpu.eax;
                put(app, cpu.eax + 0x110, 0);
                if ((byte(app, 0x55eb2e) & 0x20) != 0)
                    put(app, cpu.eax + 0x114, 4);
                else
                {
                    bool chosen = false;
                    if (at(app, 0x6fd3b8) == 0 && at(app, kPlayers) != 4)
                    {
                        cpu.esi = at(app, 0x6fd4c0);
                        if (cpu.esi == 0)
                        {
                            put(app, cpu.eax + 0x114, cpu.esi);
                            chosen = true;
                        }
                    }
                    if (!chosen)
                        put(app, cpu.ecx + 0x114, at(app, 0x6fd4c0) != 0 ? 5 : 4);
                }
            }
            if (kModes[index] != nullptr)
                store(app, cpu.eax, kModes[index]);
        }
    }
    cpu.edx = 0x1e;
    cpu.eax = 3;
    if (!byAddress(app, cpu, 0x41ffa0))
        return;
    cpu.eax = 0;
    for (reg32 at = 0x553004; at <= 0x553018; at += 4)
        put(app, at, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4238b0: player eax's camera put to mode edx: the change marked (unless
 * either end is a blend, 0x13, or the mode the same); for modes 3 to 6 the
 * mode's sound (0x426450, with 0x5e1010 the player's mark around it unless
 * leaving a blend); the mode's camera in use. */
void setCameraMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax <<= 2;
    cpu.edx = at(app, cpu.eax + kOverride);
    if (cpu.edx != 0)
        cpu.eax = at(app, cpu.edx);
    else
    {
        cpu.edi = at(app, cpu.eax + kCamera);
        cpu.eax = cpu.edi != 0 ? at(app, cpu.edi) : 0;
    }
    cpu.edx = cpu.eax;
    if (cpu.ecx != 0x13 && cpu.eax != 0x13 && cpu.eax != cpu.ecx)
        put(app, cpu.ebx * 4 + kChanged, 1);
    if (cpu.ecx == 4 || cpu.ecx == 3 || cpu.ecx == 6 || cpu.ecx == 5)
    {
        if (cpu.edx == 0x13)
        {
            cpu.edx = cpu.ebx;
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kModeSound))
                return;
        }
        else
        {
            cpu.edx = 1;
            cpu.eax = cpu.ecx;
            put(app, cpu.ebx * 8 + 0x5e1010, cpu.edx);
            cpu.edx = cpu.ebx;
            cpu.esi = 0;
            if (!byAddress(app, cpu, kModeSound))
                return;
            put(app, cpu.ebx * 8 + 0x5e1010, cpu.esi);
        }
    }
    cpu.eax = cpu.ebx * 4;
    cpu.edx = at(app, cpu.eax + cpu.ecx * 8 + kCameras);
    put(app, cpu.eax + kCamera, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423960: player eax's overriding camera put to mode edx (0 none): the
 * change marked as setCameraMode does, the sound of modes 3 to 6, the mode's
 * camera at 0x5e10b8. */
void setOverrideMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax <<= 2;
    cpu.edx = at(app, cpu.eax + kOverride);
    if (cpu.edx != 0)
        cpu.eax = at(app, cpu.edx);
    else
    {
        cpu.edi = at(app, cpu.eax + kCamera);
        cpu.eax = cpu.edi != 0 ? at(app, cpu.edi) : 0;
    }
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebx * 4;
    cpu.esi = at(app, cpu.eax + kOverride);
    cpu.eax = cpu.esi != 0 ? at(app, cpu.esi) : 0;
    if ((cpu.ecx != 0 || cpu.eax != 0) && cpu.ecx != 0x13 && cpu.edx != 0x13 && cpu.edx != cpu.ecx)
        put(app, cpu.ebx * 4 + kChanged, 1);
    if (cpu.ecx == 4 || cpu.ecx == 3 || cpu.ecx == 6 || cpu.ecx == 5)
    {
        cpu.eax = 1;
        cpu.edx = cpu.ebx;
        put(app, cpu.ebx * 8 + 0x5e1010, cpu.eax);
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kModeSound))
            return;
        cpu.edx = 0;
        put(app, cpu.ebx * 8 + 0x5e1010, cpu.edx);
    }
    cpu.eax = cpu.ebx * 4;
    cpu.edx = at(app, cpu.eax + cpu.ecx * 8 + kCameras);
    put(app, cpu.eax + kOverride, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423a20: the next of the four camera choices the player made (0x6fbbc0,
 * from the one after the last, 0x5e10c0, or from the start where eax is not
 * 0): its mode, or 0 when none is left. */
void nextCycleMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, 0x5e10c0);
    cpu.ecx = cpu.eax;
    cpu.eax = 0;
    if (cpu.ecx != 0)
        cpu.edx = 0;
    while (sreg32(cpu.edx) < 4 && at(app, cpu.edx * 4 + 0x6fbbc0) == 0)
        ++cpu.edx;
    if (sreg32(cpu.edx) < 4)
    {
        ++cpu.edx;
        cpu.eax = at(app, cpu.edx * 4 + 0x6fbbbc);
    }
    put(app, 0x5e10c0, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423a60: the cycle of camera modes (0x5e103c, each mode's next): every
 * mode to the first choice (mode 2 without any), then each choice to the
 * next. */
void buildCameraCycle(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    if (!direct(app, cpu, nextCycleMode))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
        cpu.edx = 2;
    cpu.eax = 1;
    put(app, cpu.eax * 4 + 0x5e1038, cpu.edx);
    while (sreg32(cpu.eax) < 0x19)
    {
        ++cpu.eax;
        put(app, cpu.eax * 4 + 0x5e1038, cpu.edx);
    }
    for (;;)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, nextCycleMode))
            return;
        if (cpu.eax == 0)
            break;
        put(app, cpu.edx * 4 + 0x5e103c, cpu.eax);
        cpu.edx = cpu.eax;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423ab0: whether mode eax is the next of any mode in the cycle. */
void modeInCycle(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = 0;
    cpu.edx = 0;
    do
    {
        if (cpu.ecx == at(app, cpu.eax * 4 + 0x5e103c))
            cpu.edx = 1;
        ++cpu.eax;
    } while (sreg32(cpu.eax) < 0x19);
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423ae0: mode edx kept as player eax's last (0x6fbbb8) if it is in the
 * cycle. */
void rememberMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, modeInCycle))
        return;
    if (cpu.eax != 0)
        put(app, cpu.ecx * 4 + 0x6fbbb8, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423b00: player eax's camera's mode: the overriding camera's, the one in
 * use's, or 0. */
void cameraMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kPlayers);
    cpu.eax = cpu.edx == 1 ? cpu.eax & cpu.edx : 0;
    cpu.eax <<= 2;
    cpu.ecx = at(app, cpu.eax + kOverride);
    if (cpu.ecx != 0)
        cpu.eax = at(app, cpu.ecx);
    else
    {
        cpu.esi = at(app, cpu.eax + kCamera);
        cpu.eax = cpu.esi != 0 ? at(app, cpu.esi) : 0;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423b50: player eax's camera put to mode edx (setCameraMode), 0x5600ec
 * cleared first. */
void setModeAnew(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, kPlayers);
    cpu.eax = cpu.ecx == 1 ? cpu.eax & cpu.ecx : 0;
    cpu.ebx = 0;
    put(app, 0x5600ec, cpu.ebx);
    if (!direct(app, cpu, setCameraMode))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423b80: player eax's camera brought back to what the player chose, where
 * it looks at the player's own car: a network pursuit's camera (0x11) or a
 * cop's (+0x2c4 3, by 0x552f90) to the mode for it (0x5e1054), else on to the
 * next mode of the cycle (switchCamera); a blend's target taken, a mode past
 * 6 replaced (0x5e1054), mode 9's chase node picked (pickChaseNode), and the
 * mode remembered (rememberMode). */
void settleCamera(App* app, CPU& cpu)
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
    cpu.ecx = cpu.eax;
    if (at(app, kPlayers) != 1)
        cpu.eax = at(app, kSecondPlayer);
    cpu.esi = at(app, kPlayers);
    cpu.ebx = cpu.eax;
    if (cpu.esi == 1)
        cpu.ecx &= cpu.esi;
    else
        cpu.ecx = 0;
    cpu.eax = cpu.ecx * 4;
    cpu.edi = at(app, cpu.eax + kCamera);
    if (cpu.edi != 0)
    {
        cpu.eax = at(app, cpu.edi);
        if (cpu.eax == 0x10)
            return leave();
    }
    cpu.eax = at(app, cpu.ecx * 4 + kCamera);
    cpu.esi = at(app, cpu.ebx * 4 + kCars);
    if (cpu.esi != at(app, cpu.eax + 8))
        return leave();
    bool settled = false;  // the jumps to 0x423ca9
    bool remembered = false;  // the jump to 0x423d37 with edx the mode
    if (sreg32(at(app, kPlayers)) > 1)
    {
        cpu.eax = at(app, cpu.eax);
        if (cpu.eax == 0x11)
        {
            cpu.edx = at(app, 0x5e1054);
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, setModeAnew))
                return;
            settled = true;
        }
    }
    if (!settled && at(app, kPlayers) == 1)
    {
        cpu.eax = cpu.ecx * 4;
        cpu.edx = at(app, cpu.eax + kPlayerCars);
        if ((byte(app, cpu.edx + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 3)
        {
            cpu.esi = at(app, cpu.eax + kCamera);
            cpu.eax = cpu.esi != 0 ? at(app, cpu.esi) : 0;
            cpu.edx = at(app, cpu.eax * 4 + 0x552f90);
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, setModeAnew))
                return;
            settled = true;
        }
    }
    if (!settled)
    {
        cpu.eax = cpu.ecx * 4;
        cpu.edi = at(app, cpu.eax + kCamera);
        bool blending = false;
        if (cpu.edi != 0)
        {
            cpu.eax = at(app, cpu.edi);
            blending = cpu.eax == 0x13;
        }
        if (!blending)
        {
            cpu.eax = cpu.ecx * 4;
            cpu.edx = at(app, cpu.eax + kCamera);
            cpu.eax = cpu.edx != 0 ? at(app, cpu.edx) : 0;
            cpu.ebx = at(app, cpu.eax * 4 + 0x5e103c);
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, switchCamera))
                return;
            cpu.eax = cpu.ecx * 4;
            cpu.esi = at(app, cpu.eax + kCamera);
            if (cpu.esi != 0)
            {
                cpu.eax = at(app, cpu.esi);
                if (cpu.eax == 0x13)
                {
                    cpu.edx = cpu.ebx;
                    remembered = true;
                }
            }
        }
    }
    if (!remembered)
    {
        cpu.eax = cpu.ecx * 4;
        cpu.edi = at(app, cpu.eax + kCamera);
        if (cpu.edi != 0)
        {
            cpu.eax = cpu.edi;
            cpu.edx = at(app, cpu.edi);
            if (cpu.edx == 0x13)
            {
                cpu.eax = cpu.ecx;
                cpu.edx = at(app, cpu.edi + 0x11c);
                if (!direct(app, cpu, setModeAnew))
                    return;
            }
        }
        cpu.eax = cpu.ecx * 4;
        cpu.edx = at(app, cpu.eax + kCamera);
        if (cpu.edx != 0)
        {
            cpu.eax = at(app, cpu.edx);
            if (sreg32(cpu.eax) > 6)
            {
                cpu.edx = at(app, 0x5e1054);
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, setCameraMode))
                    return;
            }
        }
        cpu.eax = cpu.ecx * 4;
        cpu.ebx = at(app, cpu.eax + kCamera);
        if (cpu.ebx != 0)
        {
            cpu.eax = at(app, cpu.ebx);
            if (cpu.eax == 9)
            {
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, pickChaseNode))
                    return;
            }
        }
        cpu.eax = cpu.ecx * 4;
        cpu.esi = at(app, cpu.eax + kCamera);
        if (cpu.esi != 0)
        {
            cpu.edx = at(app, cpu.esi);
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, rememberMode))
                return;
            return leave();
        }
        cpu.edx = 0;
    }
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, rememberMode))
        return;
    leave();
}

/* 0x423d90: the spot nearest player eax's car along the track (0x552e50's,
 * 0x1c bytes, [0x552e4c] of them, by node, round the track's [0x5dd940]),
 * within 30 nodes; or -1 -- also where the car, seen from the spot's place
 * and orientation (0x4e0000, 0x4e01c0, 0x4b7590, 0x4b79c0, 0x4e0210), is out
 * of its view (its cosine past 0.4). */
void nearestSpot(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x60;
    const reg32 e = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.edx = 0x1e;
    cpu.esi = 0xffffffff;
    cpu.ebx = at(app, kPlayers);
    put(app, e - 0xc, cpu.edx);
    if (cpu.ebx == 1)
        cpu.ecx &= cpu.ebx;
    else
        cpu.ecx ^= cpu.eax;
    cpu.eax = at(app, cpu.ecx * 4 + 0x552f58);
    cpu.eax = at(app, cpu.eax + 4);
    cpu.ebx = 0;
    cpu.edi = at(app, cpu.eax + 0x1c);
    for (; sreg32(cpu.ebx) < sreg32(at(app, 0x552e4c)); ++cpu.ebx)
    {
        put(app, e - 4, cpu.ebx);
        cpu.eax = cpu.ebx * 4;
        put(app, e - 4, cpu.eax);
        cpu.edx = at(app, e - 4);
        cpu.eax <<= 3;
        cpu.eax -= cpu.edx;
        cpu.edx = at(app, 0x552e50);
        cpu.eax += cpu.edx;
        cpu.edx = cpu.edi;
        cpu.edx -= at(app, cpu.eax + 0x18);
        if (sreg32(cpu.edx) > 0)
            put(app, e - 0x10, cpu.edx);
        else
        {
            cpu.eax = cpu.edx;
            put(app, e - 0x10, cpu.edx);
            cpu.eax = 0u - cpu.eax;
            put(app, e - 0x10, cpu.eax);
        }
        cpu.eax = at(app, kNodeCount);
        cpu.edx = cpu.eax;
        cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(sreg32(cpu.eax) >> 1);
        put(app, e - 4, cpu.eax);
        cpu.eax = cpu.ebx * 4;
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax -= cpu.edx;
        cpu.edx = at(app, e - 4);
        put(app, e - 8, cpu.eax);
        if (sreg32(cpu.edx) < sreg32(at(app, e - 0x10)))
        {
            cpu.edx = at(app, e - 8);
            cpu.eax = at(app, 0x552e50);
            cpu.eax += cpu.edx;
            cpu.edx = cpu.edi;
            cpu.edx -= at(app, cpu.eax + 0x18);
            cpu.eax = sreg32(cpu.edx) > 0 ? cpu.edx : 0u - cpu.edx;
            cpu.edx = at(app, kNodeCount);
            cpu.edx -= cpu.eax;
            cpu.eax = cpu.edx;
        }
        else
        {
            cpu.edx = at(app, e - 8);
            cpu.eax = at(app, 0x552e50);
            cpu.eax += cpu.edx;
            cpu.edx = cpu.edi;
            cpu.edx -= at(app, cpu.eax + 0x18);
            cpu.eax = sreg32(cpu.edx) > 0 ? cpu.edx : 0u - cpu.edx;
        }
        if (sreg32(cpu.eax) < sreg32(at(app, e - 0xc)))
        {
            cpu.esi = cpu.ebx;
            put(app, e - 0xc, cpu.eax);
        }
    }
    if (cpu.esi != 0xffffffff)
    {
        cpu.edi = cpu.esi * 4;
        cpu.eax = cpu.edi;
        cpu.edi <<= 3;
        cpu.edi -= cpu.eax;
        cpu.eax = at(app, 0x552e50);
        fild(app, cpu, cpu.edi + cpu.eax);
        fmul(cpu, f64(app, 0x536e8c));
        fstp(app, cpu, e - 0x2c);
        fild(app, cpu, cpu.edi + cpu.eax + 4);
        fmul(cpu, f64(app, 0x536e8c));
        fstp(app, cpu, e - 0x28);
        fild(app, cpu, cpu.edi + cpu.eax + 8);
        fmul(cpu, f64(app, 0x536e8c));
        cpu.eax = at(app, cpu.ecx * 4 + 0x552f58);
        fstp(app, cpu, e - 0x24);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.edx = e - 0x2c;
        cpu.ebx = cpu.eax + 0x98;
        cpu.ecx = e - 0x20;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kSubtract))
            return;
        cpu.edx = e - 0x20;
        cpu.eax = e - 0x20;
        if (!byAddress(app, cpu, kNormalize))
            return;
        cpu.eax = at(app, 0x552e50);
        // The spot's quaternion: four halves of its record, by 4.
        for (const reg32 k : {0xau, 0xcu, 0xeu})
        {
            cpu.edx = at(app, cpu.edi + cpu.eax + k);
            cpu.edx = reg32(sreg32(cpu.edx) >> 0x10);
            cpu.edx <<= 2;
            put(app, e - 4, cpu.edx);
            fild(app, cpu, e - 4);
            fmul(cpu, f64(app, 0x536e8c));
            fstp(app, cpu, e - 0x3c + (k - 0xa) * 2);
        }
        cpu.eax = at(app, cpu.edi + cpu.eax + 0x10);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.eax <<= 2;
        put(app, e - 4, cpu.eax);
        fild(app, cpu, e - 4);
        fmul(cpu, f64(app, 0x536e8c));
        fstp(app, cpu, e - 0x30);
        cpu.edx = e - 0x60;
        cpu.eax = e - 0x3c;
        cpu.ecx = e - 0x14;
        cpu.ebx = e - 0x20;
        if (!byAddress(app, cpu, 0x4b7590))
            return;
        cpu.eax = e - 0x60;
        cpu.edx = e - 0x48;
        if (!byAddress(app, cpu, 0x4b79c0))
            return;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kMatrix))
            return;
        if (sreg32(at(app, e - 0x14)) > 0x3ecccccd)
            cpu.esi = 0xffffffff;
    }
    cpu.eax = cpu.esi;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x423fb0: the float on the stack (ret 4) onto the x87's stack, scaled by
 * 0x536e94 where [eax + 4] is set. */
void scaledIfSet(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    if (at(app, cpu.eax + 4) != 0)
    {
        fld(cpu, f32(app, cpu.ebp + 8));
        fmul(cpu, f32(app, 0x536e94));
        fstp(app, cpu, cpu.ebp - 4);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp + 8);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x423fe0: whether the low 16 bits of eax are not one run of set bits (1),
 * or are (0): each lowest bit taken off in turn must be the one above the
 * last. */
void bitsBroken(App* app, CPU& cpu)
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
    cpu.edx = cpu.eax;
    --cpu.edx;
    cpu.ebx = cpu.eax;
    cpu.edx &= cpu.eax;
    cpu.ebx -= cpu.edx;
    while (cpu.ax != 0)
    {
        cpu.ecx = cpu.ax;
        cpu.eax = cpu.dx;
        cpu.ecx -= cpu.eax;
        cpu.eax = cpu.bx;
        if (cpu.ecx != cpu.eax)
        {
            cpu.eax = 1;
            return leave();
        }
        cpu.ecx = cpu.edx;
        cpu.eax = cpu.edx;
        --cpu.ecx;
        cpu.ebx += cpu.ebx;
        cpu.edx &= cpu.ecx;
    }
    cpu.eax = 0;
    leave();
}

/* 0x424030: the vector on the stack's second word damped by the first
 * (ret 8), where it is not 0: by the car eax's slip ([+0x4e8], at most 3.0,
 * else -3.0 past 0x536e9c), through 0x536ea4 and 0x536ea8 -- its first two
 * components by one weight, the last by the square. */
void dampVector(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 e = cpu.ebp;
    cpu.edx = at(app, e + 0x18);
    cpu.ecx = cpu.eax;
    if ((at(app, e + 0x14) & 0x7fffffff) != 0)
    {
        cpu.esi = at(app, cpu.ecx + 0x4e8);
        if (sreg32(cpu.esi) > 0x40400000)
            put(app, e - 4, 0x40400000);
        else
            put(app, e - 4, cpu.esi);
        fld(cpu, f32(app, e - 4));
        const Flags f = fcomp(cpu, f64(app, 0x536e9c));
        if (!f.cf && !f.zf)
        {
            cpu.eax = at(app, e - 4);
            put(app, e - 8, cpu.eax);
        }
        else
            put(app, e - 8, 0xc0400000);
        fld(cpu, f32(app, e - 8));
        fadd(cpu, f32(app, 0x536ea4));
        fmul(cpu, f32(app, 0x536ea8));
        fld(cpu, 1.0);
        fsubrp(cpu, 1);
        fmul(cpu, f32(app, e + 0x14));
        fld(cpu, f32(app, cpu.edx));
        fld(cpu, 1.0);
        fsubrp(cpu, 2);
        fmul(cpu, cpu.fpu.st(1));
        fld(cpu, Float(cpu.fpu.st(1)));
        fmul(cpu, cpu.fpu.st(2));
        fld(cpu, f32(app, cpu.edx + 8));
        fmulp(cpu, 3);
        fmul(cpu, f32(app, cpu.edx + 4));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.edx);
        fxch(cpu, 1);
        fstp(app, cpu, cpu.edx + 8);
        fstp(app, cpu, cpu.edx + 4);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

namespace
{

/* 0x4240c0's and 0x424260's end: the camera's offset (the frame's, at
 * `offset`) turned by its car's orientation into `turned`, added to the car's
 * place into the camera's (0x4dffb0); its orientation the car's, turned round
 * for a player looking back (0x79c4a0). */
bool offsetFromCar(App* app, CPU& cpu, reg32 offset, reg32 turned)
{
    const reg32 camera = cpu.ebp - 4;
    cpu.eax = at(app, camera);
    cpu.eax = at(app, cpu.eax + 4);
    cpu.ebx = turned;
    cpu.edx = cpu.eax + 0xc0;
    cpu.ecx = at(app, camera);
    cpu.eax = offset;
    cpu.ecx += 0x14;
    vectorTimesMatrix(app, cpu);
    cpu.ebx = turned;
    cpu.edx = cpu.ebp - 0x10;
    cpu.eax = 1;
    if (!byAddress(app, cpu, kRotate))
        return false;
    cpu.eax = at(app, camera);
    cpu.eax = at(app, cpu.eax + 0xc);
    if (at(app, cpu.eax * 4 + 0x79c4a0) != 0)
    {
        cpu.eax = at(app, camera);
        cpu.edx = at(app, camera);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.edx += 0x2c;
        cpu.eax += 0xc0;
        if (!direct(app, cpu, turnAround))
            return false;
    }
    else
    {
        cpu.eax = at(app, camera);
        cpu.edi = at(app, camera);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.ecx = 9;
        cpu.edi += 0x2c;
        cpu.esi = cpu.eax + 0xc0;
        repMovsd(app, cpu);
    }
    return true;
}

}  // namespace

/* 0x4240c0: chase camera eax placed behind its car: its offset (+0x110) --
 * in a race without a replay (0x7922fc 0) its height from the car's (+0x11c,
 * by 0x536eac) and none sideways, in a replay sideways by the car's side of
 * view (+0x9a8) -- its height scaled (+8's +0x10c), mirrored for a player
 * looking back (0x79c4a0), then offsetFromCar.  Its roll (+0x50) set. */
void chaseOffset(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const reg32 e = cpu.ebp;
    put(app, e - 4, cpu.eax);
    cpu.edi = e - 0x1c;
    cpu.esi = cpu.eax + 0x110;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    for (const reg32 k : {0u, 4u, 8u})
    {
        cpu.eax = at(app, e - 4);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.eax = at(app, cpu.eax + 0x98 + k);
        if (k == 8)
            cpu.edx = at(app, 0x7922fc);
        put(app, e - 0x10 + k, cpu.eax);
    }
    if (cpu.edx == 0)
    {
        cpu.eax = at(app, e - 4);
        put(app, cpu.eax + 0x50, 0x3ca3d70a);
        put(app, e - 0x1c, cpu.edx);
        cpu.eax = at(app, cpu.eax + 4);
        fld(cpu, f32(app, cpu.eax + 0x11c));
        fmul(cpu, f64(app, 0x536eac));
        fstp(app, cpu, e - 0x14);
    }
    else
    {
        cpu.eax = at(app, e - 4);
        put(app, cpu.eax + 0x50, 0);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.eax = at(app, cpu.eax + 0x9a8);
        if (cpu.eax == 0)
            put(app, e - 0x1c, 0xbecccccd);
        else if (cpu.eax == 1)
            put(app, e - 0x1c, 0x3ecccccd);
        else if (cpu.eax == 2)
        {
            cpu.edi = 0;
            put(app, e - 0x1c, cpu.edi);
        }
    }
    cpu.eax = at(app, e - 4);
    cpu.eax = at(app, cpu.eax + 8);
    fld(cpu, f32(app, e - 0x18));
    fmul(cpu, f32(app, cpu.eax + 0x10c));
    cpu.eax = at(app, e - 4);
    fstp(app, cpu, e - 0x18);
    cpu.eax = at(app, cpu.eax + 0xc);
    if (at(app, cpu.eax * 4 + 0x79c4a0) != 0)
        putByte(app, e - 0x11, byte(app, e - 0x11) ^ 0x80);
    if (!offsetFromCar(app, cpu, e - 0x1c, e - 0x28))
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

/* 0x424260: chase camera eax placed behind its car at its own offset
 * (+0x110): its height scaled (+8's +0x10c), its distance from the car's
 * (+0x11c, by 0x536eb4), mirrored for a player looking back, then
 * offsetFromCar. */
void chaseOffsetLevel(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const reg32 e = cpu.ebp;
    put(app, e - 4, cpu.eax);
    cpu.edi = e - 0x28;
    cpu.esi = cpu.eax + 0x110;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    for (const reg32 k : {0u, 4u, 8u})
    {
        cpu.eax = at(app, e - 4);
        cpu.eax = at(app, cpu.eax + 4);
        cpu.eax = at(app, cpu.eax + 0x98 + k);
        put(app, e - 0x10 + k, cpu.eax);
    }
    cpu.eax = at(app, e - 4);
    cpu.eax = at(app, cpu.eax + 8);
    fld(cpu, f32(app, e - 0x24));
    fmul(cpu, f32(app, cpu.eax + 0x10c));
    cpu.eax = at(app, e - 4);
    fstp(app, cpu, e - 0x24);
    cpu.eax = at(app, cpu.eax + 4);
    fld(cpu, f32(app, cpu.eax + 0x11c));
    fmul(cpu, f64(app, 0x536eb4));
    cpu.eax = at(app, e - 4);
    fstp(app, cpu, e - 0x20);
    cpu.eax = at(app, cpu.eax + 0xc);
    if (at(app, cpu.eax * 4 + 0x79c4a0) != 0)
        putByte(app, e - 0x1d, byte(app, e - 0x1d) ^ 0x80);
    if (!offsetFromCar(app, cpu, e - 0x28, e - 0x1c))
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

}  // namespace carstate
}  // namespace nfs3hp
