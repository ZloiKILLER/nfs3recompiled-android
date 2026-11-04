#include "native_guest.h"

#include <cstdint>
#include <utility>

/* A car's race state between the frames' physics: put back on the road when
 * it is stuck or the player asks (respawnCar, resetCar, stuckCheck), its
 * wheels on the track (wheelCells, wheelSurface), their skid marks, dust and
 * sounds (wheelEffects and what it calls), the camera's shake (startShake,
 * updateShake), a crash's jolt, sounds and effects (crashEffects), and the
 * grid: the start's places, from TRKnnn\start.dat or worked out, and a car
 * put on its place with its race state cleared (loadStartGrid, placeOnGrid).
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (RACESTART_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
 *
 * The car is the record at eax on entry (0x9a4 bytes and more); what these
 * functions read of it, as their code uses it: +0x1c its slice of the track,
 * +0x98 its place, +0xa4 its speed's vector and +0xb8 its speed, +0xc0 its
 * axes, +0xe4..+0xec its spin, +0x160 the track's index into the surfaces'
 * table, +0x164 the race's stage (0 racing), +0x168 how hard it last hit,
 * +0x170 the hit's kind, +0x184 a slice to be put back on, +0x18c its
 * surface, +0x200 its flags (1 a player's, 4 the player's, 0x10 a wreck),
 * +0x300 four wheels of 0x78 bytes (their cells on the track: +0x4 the place,
 * +0x10 the slice, +0x14 and +0x18 on a piece of track, +0x1c its heights),
 * +0x528 its handling, +0x8a0 the stuck count, +0x8c4..+0x8d0 the shake's
 * two waves, +0x8d4 the frames since it was put back, +0x920..+0x930 the
 * shake, +0x94c four wheels' skids, +0x95c..+0x996 their marks and sounds,
 * +0x99c what the wheels hit.  The surfaces' table at 0x553080 is the
 * game's own: 0x240 bytes a track, 0x24 a surface (kSurfaces on).
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack in cpu.fpu as the
 * code pushes and pops it (rounded as its control word says: x86::FPU's add,
 * sub, mul, div), and a compare's status word left in ax as fnstsw leaves it
 * (below and equal the flags sahf makes of it).  Every other function of the
 * game's is called by address; the game's constants (floats and doubles in
 * its data, 0x5371d0 to 0x537310) are read where the code reads them. */
namespace nfs3hp
{
namespace racestart
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
constexpr reg32 kBackwards = 0x6fd4c0;      // the track run the other way: slices counted down
constexpr reg32 kRaceType = 0x6fd3b8;       // 0 a single race, 1 and 2 the series, 3 Hot Pursuit
constexpr reg32 kPlayMode = 0x6fd3b0;       // 0 one player, 1 split screen, 2 modem, 3 serial, 4 network
constexpr reg32 kTrack = 0x6fd4ac;          // the track chosen
constexpr reg32 kTrackFolder = 0x7a27b4;    // the tracks' folder, a string
constexpr reg32 kStartFile = 0x537318;      // "%sTRK%03x\start.dat"
constexpr reg32 kLoadingFlags = 0x6fd4fc;   // 2: a car started where its start record (+0x8a4) says
constexpr reg32 kNight = 0x6fd4c8;
constexpr reg32 kLightsAlways = 0x6fd4cc;   // the cars' lights on whatever the time
// The track's slices: their count, their records (0x24 bytes each).
constexpr reg32 kSliceCount = 0x5dd940, kSlices = 0x5dd948;
// Putting a car back: slices on, the height above the road, whether a car may be.
constexpr reg32 kRespawnStep = 0x5efdcc, kRespawnHeight = 0x5efdd0, kResetAllowed = 0x7d3680;
// The stuck count's start, and the player's with kPlayerOption.
constexpr reg32 kStuckCount = 0x5efdc0, kPlayerStuckCount = 0x5efdc4, kPlayerOption = 0x6fd4a0;
// The shake's tuning, across and up; the spin in the air's (by the axes, by the steering).
constexpr reg32 kShakeAcross = 0x5efde0, kShakeUp = 0x5efde4;
constexpr reg32 kAirSpinZ = 0x5efda8, kAirSteerZ = 0x5efdac, kAirSpinX = 0x5efdb0, kAirSteerX = 0x5efdb4;
// The handling's scale on the grid: a player's, the human players'.
constexpr reg32 kHumanScale = 0x5efdb8, kPlayerScale = 0x5efdbc;
// A surface's record, 0x24 bytes, by the track (0x240) and the surface: the
// skid past which its effect shows, the speed its skid starts at and the
// speed over which it grows, its effect, whether it marks, its rumble, its
// sound, the back wheels' sound.
constexpr reg32 kSurfaces = 0x553080, kSurfaceSkidFrom = 0x553084, kSurfaceSkidOver = 0x553088;
constexpr reg32 kSurfaceEffect = 0x55308c, kSurfaceMarks = 0x553090, kSurfaceRumble = 0x553098;
constexpr reg32 kSurfaceSound = 0x55309c, kSurfaceRearSound = 0x5530a0;
// A crash's, by the other's kind (+0x170's low byte): the effect (3: by the
// speed), the sound; by its own kind (the high word): the channel; the speed
// past which its effect plays.
constexpr reg32 kCrashEffects = 0x553740, kCrashSounds = 0x553760, kCrashChannels = 0x553780;
constexpr reg32 kCrashSpeed = 0x5537b8;
// The grid: 12 places of a slice and a place (3 words); whether mirrored; the
// cars' places on it by their index (+0x1f8); the cars and the others on it.
constexpr reg32 kGrid = 0x5ef9b8, kGridY = 0x5ef9bc, kGridZ = 0x5ef9c0;
constexpr reg32 kGridMirrored = 0x55d000, kGridSlots = 0x6fdbec, kGridCars = 0x6fd518, kGridOthers = 0x6fd520;

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putHalf(App* app, reg32 a, reg16 v) { app->getMemory<reg16>(a) = v; }
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

void respawnCar(App* app, CPU& cpu);
void resetCar(App* app, CPU& cpu);
void stuckCheck(App* app, CPU& cpu);
void clearRespawn(App* app, CPU& cpu);
void shakeAmplitude(App* app, CPU& cpu);
void startShake(App* app, CPU& cpu);
void startIdleShake(App* app, CPU& cpu);
void shakeWave(App* app, CPU& cpu);
void objectContacts(App* app, CPU& cpu);
void wheelSlip(App* app, CPU& cpu);
void wheelCells(App* app, CPU& cpu);
void wheelSurface(App* app, CPU& cpu);
void endSkidMark(App* app, CPU& cpu);
void extendSkidMark(App* app, CPU& cpu);
void skidSounds(App* app, CPU& cpu);
void wheelSpray(App* app, CPU& cpu);
void surfaceEffects(App* app, CPU& cpu);
void wheelSkid(App* app, CPU& cpu);
void wheelEffects(App* app, CPU& cpu);
void updateShake(App* app, CPU& cpu);
void crashEffects(App* app, CPU& cpu);
void respawnEffects(App* app, CPU& cpu);
void stopCarEffects(App* app, CPU& cpu);
void carEffects(App* app, CPU& cpu);
void airborneSpin(App* app, CPU& cpu);
void updateCar(App* app, CPU& cpu);
void defaultGridPlace(App* app, CPU& cpu);
void loadStartGrid(App* app, CPU& cpu);
void gridPlace(App* app, CPU& cpu);
void placeOnGrid(App* app, CPU& cpu);

/* 0x428f90: car eax put back on the road -- [+0x2c0], the times it was, one
 * more.  Not (+0x8d4 just cleared) when [+0x10c] is over 0.75, its spin
 * [+0xe4] and [+0xec] under 1, its surface not 0 or 0xe, no slice asked
 * for, [+0x164] 5 or less and [+0x15c] under 0.6 (each compared as an
 * integer, as the code does).  Else [kRespawnStep] slices on from its slice (or the
 * one at +0x184, cleared) -- back when the race runs backwards -- wrapped
 * round the track; placed there (0x428d60), lifted [kRespawnHeight] (half
 * of it where 0x420bf0 says so), at rest (0x428970); 0x40 at +0x174. */
void respawnCar(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
    if (at(app, kBackwards) != 0)
    {
        cpu.eax = 0xffffffff;
    }
    else
    {
        cpu.eax = 1;
    }
    cpu.edx = cpu.eax;
    cpu.ebx = at(app, cpu.ecx + 0x2c0);
    put(app, cpu.ecx + 0x8d4, 0);
    ++cpu.ebx;
    cpu.esi = at(app, cpu.ecx + 0x10c);
    put(app, cpu.ecx + 0x2c0, cpu.ebx);
    if (i32(cpu.esi) > i32(0x3f400000) && i32(at(app, cpu.ecx + 0xe4)) < i32(0x3f800000) && i32(at(app, cpu.ecx + 0xec)) < i32(0x3f800000))
    {
        cpu.ebx = at(app, cpu.ecx + 0x18c);
        if (cpu.ebx != 0xe && cpu.ebx != 0)
        {
            cpu.edi = at(app, cpu.ecx + 0x184);
            if (cpu.edi == 0 && half(app, cpu.ecx + 0x164) <= 5 && i32(at(app, cpu.ecx + 0x15c)) < i32(0x3f19999a))
            {
                put(app, cpu.ecx + 0x8d4, cpu.edi);
                return leave();
            }
        }
    }
    cpu.ebx = at(app, cpu.ecx + 0x184);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    if (cpu.ebx != 0)
        cpu.eax = cpu.ebx;
    cpu.edx *= at(app, kRespawnStep);
    put(app, cpu.ecx + 0x184, 0);
    cpu.eax += cpu.edx;
    if (i32(cpu.edx) >= 0)
    {
        cpu.edi = at(app, kSliceCount);
        if (i32(cpu.eax) >= i32(cpu.edi))
            cpu.eax -= cpu.edi;
    }
    else if (i32(cpu.eax) < 0)
        cpu.eax += at(app, kSliceCount);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x428d60))
        return;
    cpu.esi = at(app, cpu.ecx + 0x1c);
    push(app, cpu, cpu.esi);
    cpu.edi = at(app, cpu.ecx + 0xa0);
    cpu.eax = at(app, kRespawnHeight);
    push(app, cpu, cpu.edi);
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x9c);
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ecx + 0x98);
    push(app, cpu, cpu.edx);
    if (!byAddress(app, cpu, 0x420bf0))
        return;
    if (cpu.eax != 0)
    {
        fpu.push(f32(app, cpu.ebp - 4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5371d0));
        app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
        fpu.pop();
    }
    put(app, cpu.ecx + 0xa4, 0);
    put(app, cpu.ecx + 0xa8, 0);
    fpu.push(f32(app, cpu.ecx + 0x9c));
    put(app, cpu.ecx + 0xac, 0);
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 4));
    cpu.eax = cpu.ecx;
    app->getMemory<float>(cpu.ecx + 0x9c) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x428970))
        return;
    put(app, cpu.ecx + 0x174, 0x40);
    return leave();
}

/* 0x429100: car eax put back on the road when the player asks: as
 * respawnCar, from its own slice, unless it was just now ([+0x524], left
 * 0x20 after), it is a player's car finished ([+0x2c4] 3), it may not be
 * ([kResetAllowed]), a slice is asked for already, or in Hot Pursuit
 * 0x401500 says no of its pursuer ([+0x624], flag 0x20) or of itself. */
void resetCar(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
    if (at(app, kBackwards) != 0)
    {
        cpu.eax = 0xffffffff;
    }
    else
    {
        cpu.eax = 1;
    }
    cpu.ebx = at(app, cpu.ecx + 0x524);
    cpu.edx = cpu.eax;
    if (cpu.ebx != 0)
        return leave();
    if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 3)
        return leave();
    if (at(app, kResetAllowed) == 0)
        return leave();
    if (at(app, cpu.ecx + 0x184) != 0)
        return leave();
    if (at(app, kRaceType) == 3)
    {
        if ((byte(app, cpu.ecx + 0x200) & 0x20) != 0)
        {
            cpu.esi = at(app, cpu.ecx + 0x624);
            if (cpu.esi != 0)
            {
                cpu.eax = cpu.esi;
                if (!byAddress(app, cpu, 0x401500))
                    return;
                if (cpu.eax != 0)
                    return leave();
            }
        }
        if ((byte(app, cpu.ecx + 0x200) & 1) != 0)
        {
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, 0x401500))
                return;
            if (cpu.eax != 0)
                return leave();
        }
    }
    cpu.edx *= at(app, kRespawnStep);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    cpu.eax += cpu.edx;
    if (i32(cpu.edx) >= 0)
    {
        cpu.edx = at(app, kSliceCount);
        if (i32(cpu.eax) >= i32(cpu.edx))
            cpu.eax -= cpu.edx;
    }
    else if (i32(cpu.eax) < 0)
        cpu.eax += at(app, kSliceCount);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x428d60))
        return;
    cpu.edi = at(app, cpu.ecx + 0x1c);
    cpu.eax = at(app, kRespawnHeight);
    push(app, cpu, cpu.edi);
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0xa0);
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ecx + 0x9c);
    push(app, cpu, cpu.edx);
    cpu.ebx = at(app, cpu.ecx + 0x98);
    push(app, cpu, cpu.ebx);
    if (!byAddress(app, cpu, 0x420bf0))
        return;
    if (cpu.eax != 0)
    {
        fpu.push(f32(app, cpu.ebp - 4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5371d4));
        app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
        fpu.pop();
    }
    put(app, cpu.ecx + 0xa4, 0);
    put(app, cpu.ecx + 0xa8, 0);
    fpu.push(f32(app, cpu.ecx + 0x9c));
    put(app, cpu.ecx + 0xac, 0);
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 4));
    cpu.eax = cpu.ecx;
    app->getMemory<float>(cpu.ecx + 0x9c) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x428970))
        return;
    put(app, cpu.ecx + 0x174, 0x40);
    put(app, cpu.ecx + 0x524, 0x20);
    return leave();
}

/* 0x429260: car eax's stuck count (+0x8a0) run down while it is above 0:
 * by one unless the player's car has [+0x10c] 0.1 or more and no slice asked
 * for; by two more when +0x20 or +0x24 is 0 or what it is on (0x4207e0 of
 * +0xc) is 0 or 0xe; its spin (+0xe4..+0xec) a 64th less when [+0x15c] is
 * under 5 and [+0x10c] under 0.5, racing; by eight when |[+0xa8]| plus
 * [+0xb8] is under 1; by one more for the player's car at [+0x504] over 200
 * with [+0x10c] under 0.1.  At 0 or below it is put back (respawnCar), but a
 * wreck, and the count is 0.  The count at 0 or below, reset: to
 * [kStuckCount] ([kPlayerStuckCount] for the player with [kPlayerOption]),
 * 0x200 when a slice is asked for -- but not when the dot of +0x130 and
 * +0xcc (0x4e01f0) is 0.75 or more on a surface of its own. */
void stuckCheck(App* app, CPU& cpu)
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
    cpu.ecx = cpu.eax;
    if (i32(at(app, cpu.eax + 0x8a0)) > i32(0))
    {
        cpu.edx = 1;
        cpu.eax += 0xc;
        if (!byAddress(app, cpu, 0x4207e0))
            return;
        cpu.eax &= 0x3f;
        cpu.ebx = at(app, cpu.ecx + 0x10c);
        if (i32(cpu.ebx) < i32(0x3dcccccd) || at(app, cpu.ecx + 0x184) != 0 || (byte(app, cpu.ecx + 0x200) & 4) == 0)
            put(app, cpu.ecx + 0x8a0, at(app, cpu.ecx + 0x8a0) - 1);
        if (at(app, cpu.ecx + 0x20) == 0 || at(app, cpu.ecx + 0x24) == 0 || cpu.eax == 0 || cpu.eax == 0xe)
            put(app, cpu.ecx + 0x8a0, at(app, cpu.ecx + 0x8a0) - 2);
        if (i32(at(app, cpu.ecx + 0x15c)) < i32(0x40a00000) && at(app, cpu.ecx + 0x184) == 0 && half(app, cpu.ecx + 0x164) == 0 && i32(at(app, cpu.ecx + 0x10c)) < i32(0x3f000000))
        {
            fpu.push(f32(app, 0x5371d8));
            fpu.push(f32(app, cpu.ecx + 0xe4));
            fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
            fpu.push(f32(app, cpu.ecx + 0xe8));
            fpu.st(0) = fp::mul(fpu.st(0), fpu.st(2));
            fpu.push(f32(app, cpu.ecx + 0xec));
            fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
            fpu.pop();
            std::swap(fpu.st(0), fpu.st(1));
            fpu.st(0) = fp::sub(f32(app, cpu.ecx + 0xe4), fpu.st(0));
            std::swap(fpu.st(0), fpu.st(1));
            fpu.st(0) = fp::sub(f32(app, cpu.ecx + 0xe8), fpu.st(0));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(0) = fp::sub(f32(app, cpu.ecx + 0xec), fpu.st(0));
            std::swap(fpu.st(0), fpu.st(2));
            app->getMemory<float>(cpu.ecx + 0xe8) = float(fpu.st(0));
            fpu.pop();
            std::swap(fpu.st(0), fpu.st(1));
            app->getMemory<float>(cpu.ecx + 0xec) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.ecx + 0xe4) = float(fpu.st(0));
            fpu.pop();
        }
        fpu.push(0.0);
        fcomp(cpu, f32(app, cpu.ecx + 0xa8));
        cpu.ax = fpu.status.word;
        if (below(cpu))
        {
            cpu.eax = at(app, cpu.ecx + 0xa8);
            put(app, cpu.ebp - 4, cpu.eax);
        }
        else
        {
            fpu.push(f32(app, cpu.ecx + 0xa8));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
        }
        fpu.push(f32(app, cpu.ecx + 0xb8));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 4));
        fpu.push(1.0);
        fcompp(cpu);
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu) && at(app, cpu.ecx + 0x184) == 0 && half(app, cpu.ecx + 0x164) == 0)
            put(app, cpu.ecx + 0x8a0, at(app, cpu.ecx + 0x8a0) - 8);
        if ((byte(app, cpu.ecx + 0x200) & 4) != 0 && byte(app, cpu.ecx + 0x504) > 0xc8 && at(app, cpu.ecx + 0x184) == 0 && i32(at(app, cpu.ecx + 0x10c)) < i32(0x3dcccccd))
            put(app, cpu.ecx + 0x8a0, at(app, cpu.ecx + 0x8a0) - 1);
        if (i32(at(app, cpu.ecx + 0x8a0)) <= i32(0))
        {
            if ((byte(app, cpu.ecx + 0x200) & 0x10) == 0)
            {
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, respawnCar))
                    return;
                put(app, cpu.ecx + 0x8a0, 0);
                return leave();
            }
            put(app, cpu.ecx + 0x8a0, 0);
        }
    }
    cpu.edx = cpu.ecx + 0xcc;
    cpu.eax = cpu.ecx + 0x130;
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    cpu.esi = at(app, cpu.ecx + 0x8a0);
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.pop();
    if (i32(cpu.esi) > 0)
        return leave();
    fpu.push(f32(app, cpu.ebp - 8));
    fcomp(cpu, f32(app, 0x5371dc));
    cpu.ax = fpu.status.word;
    if (!below(cpu))
    {
        cpu.edi = at(app, cpu.ecx + 0x18c);
        if (cpu.edi != 0 && cpu.edi != 0xe && at(app, cpu.ecx + 0x184) == 0)
            return leave();
    }
    if (at(app, cpu.ecx + 0x184) != 0)
    {
        put(app, cpu.ecx + 0x8a0, 0x200);
    }
    else
    {
        cpu.eax = at(app, kStuckCount);
        put(app, cpu.ecx + 0x8a0, cpu.eax);
        if ((byte(app, cpu.ecx + 0x200) & 4) != 0 && at(app, kPlayerOption) != 0)
        {
            cpu.eax = at(app, kPlayerStuckCount);
            put(app, cpu.ecx + 0x8a0, cpu.eax);
        }
    }
    return leave();
}

/* 0x4294c0: car eax's slice to be put back on and time since, cleared. */
void clearRespawn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, cpu.eax + 0x184, 0);
    put(app, cpu.eax + 0x8d4, 0);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x4294e0: car eax's rumble (+0x930): its speed by a 64th, by its
 * surface's rumble and its handling's (+0x298), times sin(the speed's 64th,
 * 0 to 1, by 0.5) (0x4ea930). */
void shakeAmplitude(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x160);
    fpu.push(f32(app, cpu.edx + 0xb8));
    cpu.ebx = cpu.ecx * 8;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5371e0));
    cpu.ebx += cpu.ecx;
    cpu.ecx = at(app, cpu.eax + 0x18c);
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ebx <<= 6;
    cpu.eax <<= 2;
    cpu.ecx = cpu.ebx + cpu.eax;
    cpu.eax = at(app, cpu.edx + 0x528);
    fpu.push(f32(app, cpu.ecx + kSurfaceRumble));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x298));
    fpu.push(0.0);
    std::swap(fpu.st(0), fpu.st(2));
    app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
    fpu.pop();
    app->getMemory<float>(cpu.edx + 0x930) = float(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f32(app, cpu.ebp - 4));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.ecx = 0;
        put(app, cpu.ebp - 8, cpu.ecx);
    }
    else
    {
        cpu.ebx = at(app, cpu.ebp - 4);
        if (i32(cpu.ebx) > i32(0x3f800000))
        {
            put(app, cpu.ebp - 0xc, 0x3f800000);
        }
        else
        {
            put(app, cpu.ebp - 0xc, cpu.ebx);
        }
        cpu.eax = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5371e8));
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    push(app, cpu, at(app, cpu.ebp - 0x10));
    if (!byAddress(app, cpu, 0x4ea930))
        return;
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.edx + 0x930));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    app->getMemory<float>(cpu.edx + 0x930) = float(fpu.st(0));
    fpu.pop();
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x4295a0: car eax's crash shake started, if it hit hard enough ([+0x168]
 * 5 or more): two waves (+0x8c4 and +0x8cc), across and up -- the shake's
 * tuning by its handling's (+0x294), by the hit (a 45th, 0 to 1.5) -- for
 * edx and ebx frames, each its sign turned at random (rand). */
void startShake(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x528);
    fpu.push(f32(app, kShakeAcross));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x294));
    fpu.push(f32(app, kShakeUp));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x294));
    cpu.esi = at(app, cpu.ecx + 0x168);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
    fpu.pop();
    if (i32(cpu.esi) < i32(0x40a00000))
        return leave();
    fpu.push(f32(app, cpu.ecx + 0x168));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5371f0));
    fpu.push(0.0);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f32(app, cpu.ebp - 0xc));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.edi = 0;
        put(app, cpu.ebp - 0x18, cpu.edi);
        put(app, cpu.ebp - 0x14, cpu.edi);
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0xc));
        app->getMemory<double>(cpu.ebp - 0x28) = double(fpu.st(0));
        fcomp(cpu, f64(app, 0x5371f8));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.esi = 0;
            cpu.edi = 0x3ff80000;
            put(app, cpu.ebp - 0x20, cpu.esi);
            put(app, cpu.ebp - 0x1c, cpu.edi);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x28);
            put(app, cpu.ebp - 0x20, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x24);
            put(app, cpu.ebp - 0x1c, cpu.eax);
        }
        cpu.eax = at(app, cpu.ebp - 0x20);
        put(app, cpu.ebp - 0x18, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x1c);
        put(app, cpu.ebp - 0x14, cpu.eax);
    }
    fpu.push(f64(app, cpu.ebp - 0x18));
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    put(app, cpu.ecx + 0x8c8, cpu.edx);
    app->getMemory<float>(cpu.ecx + 0x8c4) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.esi = 0x100;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.esi);
    if (i32(cpu.edx) > i32(0x80))
    {
        cpu.ah = byte(app, cpu.ecx + 0x8c7);
        cpu.ah ^= 0x80;
        putByte(app, cpu.ecx + 0x8c7, cpu.ah);
    }
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 4));
    put(app, cpu.ecx + 0x8d0, cpu.ebx);
    app->getMemory<float>(cpu.ecx + 0x8cc) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = 0x100;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ebx);
    if (i32(cpu.edx) <= i32(0x80))
        return leave();
    cpu.dl = byte(app, cpu.ecx + 0x8cf);
    cpu.dl ^= 0x80;
    putByte(app, cpu.ecx + 0x8cf, cpu.dl);
    return leave();
}

/* 0x4296c0: car eax's little shake ([+0x10c] -0.9 or less, racing): two
 * small waves (0x3b888889, a 240th), each 24 to 35 frames, each its sign at
 * random. */
void startIdleShake(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
    cpu.ecx = cpu.eax;
    fpu.push(f32(app, cpu.eax + 0x10c));
    fcomp(cpu, f64(app, 0x537200));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
        return leave();
    if (half(app, cpu.ecx + 0x164) != 0)
        return leave();
    put(app, cpu.ecx + 0x8c4, 0x3b888889);
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = 0xc;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ebx);
    cpu.edx += 0x18;
    put(app, cpu.ecx + 0x8c8, cpu.edx);
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = 0x100;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ebx);
    if (i32(cpu.edx) > i32(0x80))
    {
        cpu.ah = byte(app, cpu.ecx + 0x8c7);
        cpu.ah ^= 0x80;
        putByte(app, cpu.ecx + 0x8c7, cpu.ah);
    }
    put(app, cpu.ecx + 0x8cc, 0x3b888889);
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = 0xc;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ebx);
    cpu.edx += 0x18;
    put(app, cpu.ecx + 0x8d0, cpu.edx);
    if (!byAddress(app, cpu, 0x4eb1fc))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = 0x100;
    cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ebx);
    if (i32(cpu.edx) <= i32(0x80))
        return leave();
    cpu.dl = byte(app, cpu.ecx + 0x8cf);
    cpu.dl ^= 0x80;
    putByte(app, cpu.ecx + 0x8cf, cpu.dl);
    return leave();
}

/* 0x429790 (stdcall: the amplitude, the frames left): a shake's wave, onto
 * the x87's stack: sin(frames / 12) (0x4ea930) by the amplitude, by the
 * frames, by a 12th. */
void shakeWave(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.eax = at(app, cpu.ebp + 0x10);
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(x86::Float(i32(at(app, cpu.ebp - 4))));
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x537208));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4ea930))
        return;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp + 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x537208));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.eax = pop(app, cpu);
    cpu.esp += 4 + 8;
    return;
}

/* 0x4297d0: car eax against the scenery's objects: the player's first
 * (0x493ad0); then, a player's car still racing ([+0x2c4] not 3, and not 4
 * unless the player's), the object it is against (0x4ca160): handed over
 * (0x4c9f50), the hit's flags kept at +0x99c, 0x40 at +0x621 (0x401650). */
void objectContacts(App* app, CPU& cpu)
{
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
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    if ((byte(app, cpu.eax + 0x200) & 4) != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x493ad0))
            return;
    }
    if (half(app, cpu.ecx + 0x164) != 0)
        return leave();
    cpu.dl = byte(app, cpu.ecx + 0x200);
    if ((cpu.dl & 1) == 0)
        return leave();
    if ((cpu.dl & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 3)
        return leave();
    if ((byte(app, cpu.ecx + 0x200) & 1) != 0 && at(app, cpu.ecx + 0x2c4) == 4 && (byte(app, cpu.ecx + 0x200) & 4) == 0)
        return leave();
    cpu.edx = cpu.ebp - 4;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x4ca160))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
        return leave();
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, 0x4c9f50))
        return;
    cpu.al = byte(app, cpu.ecx + 0x621);
    cpu.esi = at(app, cpu.ecx + 0x99c);
    cpu.al |= 0x40;
    cpu.esi |= cpu.edx;
    putByte(app, cpu.ecx + 0x621, cpu.al);
    put(app, cpu.ecx + 0x99c, cpu.esi);
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x401650))
        return;
    return leave();
}

/* 0x429880: how much a wheel slips, 0 to 1 onto the x87's stack: the
 * car's speed less its length along its axis (0x4e06a0 of +0xe4, +0xb8),
 * how sideways it goes (1 less the dot of its normalised speed (0x4972f0)
 * with +0xd8), the wheels' slide (+0x598 front if edx, +0x594 back, whole). */
void wheelSlip(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x50;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.edi = cpu.ebp - 0x50;
    cpu.esi = cpu.ecx + 0xa4;
    cpu.edx = cpu.ebp - 0x44;
    cpu.eax += 0xe4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    if (!byAddress(app, cpu, 0x4e06a0))
        return;
    cpu.eax = cpu.ebp - 0x50;
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ecx + 0xb8));
    cpu.edi = cpu.ebp - 0x44;
    app->getMemory<float>(cpu.ebp - 0x18) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4972f0))
        return;
    cpu.esi = cpu.ecx + 0xd8;
    fpu.pop();
    cpu.eax = cpu.ebp - 0x50;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    fpu.push(0.0);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f32(app, cpu.ebp - 8));
    cpu.ax = fpu.status.word;
    if (below(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 0x14, cpu.eax);
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 8));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
        fpu.pop();
    }
    fpu.push(1.0);
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x14));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    if (cpu.ebx != 0)
    {
        cpu.edx = at(app, cpu.ecx + 0x598);
        if (i32(cpu.edx) > 0)
        {
            cpu.eax = cpu.edx;
        }
        else
        {
            cpu.eax = cpu.edx;
            cpu.eax = 0u - cpu.eax;
        }
    }
    else
    {
        cpu.ebx = at(app, cpu.ecx + 0x594);
        if (i32(cpu.ebx) > 0)
        {
            cpu.eax = cpu.ebx;
        }
        else
        {
            cpu.eax = cpu.ebx;
            cpu.eax = 0u - cpu.eax;
        }
    }
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(f32(app, 0x53720c));
    fpu.push(x86::Float(i32(at(app, cpu.ebp - 4))));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x18));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x537210));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.push(0.0);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<double>(cpu.ebp - 0x28) = double(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f64(app, cpu.ebp - 0x28));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.esi = 0;
        put(app, cpu.ebp - 0x30, cpu.esi);
        put(app, cpu.ebp - 0x2c, cpu.esi);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.ebp - 0x30, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x24);
        put(app, cpu.ebp - 0x2c, cpu.eax);
    }
    fpu.push(f64(app, cpu.ebp - 0x30));
    fpu.push(1.0);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<double>(cpu.ebp - 0x38) = double(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f64(app, cpu.ebp - 0x38));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 0x38);
        put(app, cpu.ebp - 0x20, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x34);
        put(app, cpu.ebp - 0x1c, cpu.eax);
    }
    else
    {
        cpu.eax = 0;
        cpu.edx = 0x3ff00000;
        put(app, cpu.ebp - 0x20, cpu.eax);
        put(app, cpu.ebp - 0x1c, cpu.edx);
    }
    fpu.push(f64(app, cpu.ebp - 0x20));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x4299a0: car eax's four wheels on the track: its corners -- across 0.75
 * of +0x114 either way, down +0x118, along half of +0x11c either way --
 * turned by its axes and moved to where it will be (its place plus a 32nd of
 * its speed, 0x4e0430), as 16.16, each looked up from its slice (0x420650)
 * into the wheel's 0x78 bytes at +0x300. */
void wheelCells(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x104;
    cpu.ebp -= 0x82;
    const auto leave = [&] {
        cpu.esp = cpu.ebp + 0x82;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp + 0x7e, cpu.eax);
    cpu.edi = cpu.ebp + 0x6e;
    cpu.esi = cpu.eax + 0x114;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    fpu.push(f64(app, 0x537220));
    fpu.push(f32(app, cpu.ebp + 0x76));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.push(f64(app, 0x537218));
    fpu.push(f32(app, cpu.ebp + 0x6e));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.push(f32(app, cpu.ebp + 0x6e));
    fpu.st(0) = -fpu.st(0);
    fpu.st(2) = fp::mul(fpu.st(2), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp + 0x76));
    fpu.st(0) = -fpu.st(0);
    fpu.st(4) = fp::mul(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp + 0x72));
    fpu.st(0) = -fpu.st(0);
    app->getMemory<float>(cpu.ebp + 0x2a) = float(fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, cpu.ebp + 0x2a);
    std::swap(fpu.st(0), fpu.st(2));
    app->getMemory<float>(cpu.ebp + 0x2e) = float(fpu.st(0));
    fpu.pop();
    put(app, cpu.ebp + 0x36, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x2e);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.ebp + 0x32) = float(fpu.st(0));
    fpu.pop();
    put(app, cpu.ebp + 0x3a, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x32);
    put(app, cpu.ebp + 0x3e, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x2a);
    app->getMemory<float>(cpu.ebp + 0x26) = float(fpu.st(0));
    fpu.pop();
    put(app, cpu.ebp + 0x42, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x26);
    put(app, cpu.ebp + 0x4a, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x2a);
    app->getMemory<float>(cpu.ebp + 0x46) = float(fpu.st(0));
    fpu.pop();
    put(app, cpu.ebp + 0x4e, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x46);
    put(app, cpu.ebp + 0x52, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x7e);
    fpu.push(f32(app, 0x537228));
    fpu.push(f32(app, cpu.eax + 0xa4));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x98));
    app->getMemory<float>(cpu.ebp + 0x56) = float(fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.eax + 0xa8));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x9c));
    app->getMemory<float>(cpu.ebp + 0x5a) = float(fpu.st(0));
    fpu.pop();
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xac));
    cpu.ebx = at(app, cpu.ebp + 0x7e);
    cpu.ecx = cpu.ebp + 0x56;
    cpu.edx = cpu.ebp + 0x26;
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0xa0));
    cpu.eax = cpu.ebp - 0xa;
    cpu.ebx += 0xc0;
    push(app, cpu, cpu.eax);
    cpu.eax = 4;
    app->getMemory<float>(cpu.ebp + 0x5e) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4e0430))
        return;
    cpu.edx = 0;
    put(app, cpu.ebp + 0x7a, cpu.edx);
    // Each wheel (the count in the frame at +0x7a): its corner as 16.16, its
    // cell from the car's slice, copied to the wheel.
    do
    {
        cpu.edx = at(app, cpu.ebp + 0x7a);
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.edx = cpu.eax * 4;
        fpu.push(f32(app, cpu.edx + cpu.ebp - 0xa));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537230));
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
        fpu.pop();
        cpu.eax = pop(app, cpu);
        fpu.push(f32(app, cpu.edx + cpu.ebp - 6));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537230));
        put(app, cpu.ebp + 0x62, cpu.eax);
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
        fpu.pop();
        cpu.eax = pop(app, cpu);
        fpu.push(f32(app, cpu.edx + cpu.ebp - 2));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537230));
        put(app, cpu.ebp + 0x66, cpu.eax);
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
        fpu.pop();
        cpu.eax = pop(app, cpu);
        put(app, cpu.ebp + 0x6a, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7e);
        cpu.esi = cpu.ebp - 0x82;
        cpu.edx = 1;
        cpu.ebx = at(app, cpu.eax + 0x1c);
        cpu.eax = cpu.ebp + 0x62;
        if (!byAddress(app, cpu, 0x420650))
            return;
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax <<= 3;
        cpu.edx = cpu.eax;
        cpu.eax <<= 4;
        cpu.ebx = at(app, cpu.ebp + 0x7e);
        cpu.eax -= cpu.edx;
        cpu.eax += cpu.ebx;
        cpu.ecx = 0x1e;
        cpu.esi = cpu.ebp - 0x82;
        cpu.edi = cpu.eax + 0x300;
        for (; cpu.ecx != 0; --cpu.ecx)
        {
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
        }
        put(app, cpu.ebp + 0x7a, at(app, cpu.ebp + 0x7a) + 1);
    } while (i32(at(app, cpu.ebp + 0x7a)) < i32(4));
    return leave();
}

/* 0x429b40: what wheel edx of car eax is on: the car's surface, or, on a
 * piece of track of its own with the race on, the wheel's (0x4207e0). */
void wheelSurface(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.si = half(app, cpu.eax + 0x164);
    cpu.edx = at(app, cpu.eax + 0x18c);
    if (cpu.si == 0)
    {
        cpu.ebx <<= 3;
        cpu.eax = cpu.ebx;
        cpu.eax <<= 4;
        cpu.eax -= cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax += cpu.ecx;
        if (at(app, cpu.eax + 0x314) != 0 && at(app, cpu.eax + 0x318) != 0)
        {
            cpu.eax = cpu.ecx + 0x300;
            cpu.edx = 1;
            cpu.eax += cpu.ebx;
            if (!byAddress(app, cpu, 0x4207e0))
                return;
            cpu.edx = cpu.eax;
            cpu.edx &= 0x3f;
        }
    }
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x429ba0: wheel edx's skid mark ended, if it has one (+0x95c): its last
 * point, the track's height under it, how much it slips (wheelSlip) --
 * 0x4db940 with 1 -- and the mark and its sound (+0x990) off. */
void endSkidMark(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
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
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, cpu.ebp - 4, cpu.eax);
    put(app, cpu.ebp - 8, cpu.edx);
    cpu.eax += cpu.edx;
    if (byte(app, cpu.eax + 0x95c) == 0)
        return leave();
    cpu.ecx = at(app, cpu.ebp - 8);
    if (i32(cpu.ecx) < i32(2))
    {
        cpu.edx = 1;
    }
    else
    {
        cpu.edx = 0;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    if (!direct(app, cpu, wheelSlip))
        return;
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.eax <<= 2;
    cpu.eax += cpu.ebx;
    cpu.esi = cpu.eax + 0x960;
    cpu.eax = cpu.edx * 8;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.eax -= cpu.edx;
    cpu.edi = cpu.ebp - 0x18;
    cpu.eax += cpu.ebx;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    cpu.edi = at(app, cpu.eax + 0x314);
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    if (cpu.edi != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = reg32(i32(i16(half(app, cpu.eax))));
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xd);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    put(app, cpu.ebp - 0x24, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.eax -= cpu.edx;
    cpu.eax += at(app, cpu.ebp - 4);
    if (at(app, cpu.eax + 0x314) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = at(app, cpu.eax);
        cpu.eax = reg32(i32(cpu.eax) >> 0x10);
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xe);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    put(app, cpu.ebp - 0x20, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.eax -= cpu.edx;
    cpu.eax += cpu.ebx;
    if (at(app, cpu.eax + 0x314) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = at(app, cpu.eax + 2);
        cpu.eax = reg32(i32(cpu.eax) >> 0x10);
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xf);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    push(app, cpu, at(app, cpu.ebp - 0xc));
    cpu.ecx = cpu.ebp - 0x24;
    cpu.ebx = cpu.ebp - 0x18;
    put(app, cpu.ebp - 0x1c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.ebp - 8);
    push(app, cpu, 1);
    cpu.eax = at(app, cpu.eax);
    cpu.edi = at(app, cpu.ebp - 8);
    if (!byAddress(app, cpu, 0x4db940))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax += cpu.edi;
    putByte(app, cpu.eax + 0x95c, 0);
    putByte(app, cpu.eax + 0x990, 0);
    return leave();
}

/* 0x429d20: wheel edx's skid mark drawn on to where it is: off the track's
 * pieces, ended (0x4db860); else a point (0x4db940 with 0) -- its place, the
 * height under it, how much it slips -- kept as the last (+0x960), the mark
 * on and its sound (+0x990) the surface's. */
void extendSkidMark(App* app, CPU& cpu)
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
    put(app, cpu.ebp - 4, cpu.eax);
    put(app, cpu.ebp - 8, cpu.edx);
    if (i32(cpu.edx) < i32(2))
    {
        cpu.edx = 1;
    }
    else
    {
        cpu.edx = 0;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    if (!direct(app, cpu, wheelSlip))
        return;
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.ecx = at(app, cpu.ebp - 4);
    cpu.eax -= cpu.edx;
    cpu.edi = cpu.ebp - 0x28;
    cpu.ecx += cpu.eax;
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.esi = cpu.ecx + 0x304;
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    if (!direct(app, cpu, wheelSurface))
        return;
    cpu.ebx = at(app, cpu.ecx + 0x314);
    put(app, cpu.ebp - 0x10, cpu.eax);
    if (cpu.ebx != 0)
    {
        if (at(app, cpu.ecx + 0x318) != 0)
            goto l429db0;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax);
    cpu.edi = at(app, cpu.ebp - 8);
    if (!byAddress(app, cpu, 0x4db860))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax += cpu.edi;
    putByte(app, cpu.eax + 0x95c, 0);
    putByte(app, cpu.eax + 0x990, 0);
    return leave();
l429db0:
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.eax -= cpu.edx;
    cpu.eax += at(app, cpu.ebp - 4);
    if (at(app, cpu.eax + 0x314) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = reg32(i32(i16(half(app, cpu.eax))));
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xd);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    put(app, cpu.ebp - 0x1c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.eax -= cpu.edx;
    cpu.eax += cpu.ebx;
    if (at(app, cpu.eax + 0x314) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = at(app, cpu.eax);
        cpu.eax = reg32(i32(cpu.eax) >> 0x10);
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, kSlices);
        cpu.edx <<= 2;
        cpu.eax = at(app, cpu.edx + cpu.eax + 0xe);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    put(app, cpu.ebp - 0x18, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.edi = at(app, cpu.ebp - 4);
    cpu.eax -= cpu.edx;
    cpu.eax += cpu.edi;
    if (at(app, cpu.eax + 0x314) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x31c);
        cpu.eax = at(app, cpu.eax + 2);
        cpu.eax = reg32(i32(cpu.eax) >> 0x10);
        cpu.eax += cpu.eax;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x310);
        cpu.eax = cpu.edx * 8;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, kSlices);
        cpu.edx <<= 2;
        cpu.eax = at(app, cpu.edx + cpu.eax + 0xf);
        cpu.eax = reg32(i32(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    push(app, cpu, at(app, cpu.ebp - 0xc));
    cpu.ecx = cpu.ebp - 0x1c;
    cpu.ebx = cpu.ebp - 0x28;
    put(app, cpu.ebp - 0x14, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    push(app, cpu, 0);
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = at(app, cpu.eax);
    if (!byAddress(app, cpu, 0x4db940))
        return;
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.ecx = at(app, cpu.ebp - 4);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.esi = cpu.ebp - 0x28;
    cpu.edi = cpu.eax + 0x960;
    cpu.ecx += cpu.edx;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
    cpu.edx = at(app, cpu.ebp - 4);
    putByte(app, cpu.ecx + 0x95c, 1);
    cpu.edx = at(app, cpu.edx + 0x160);
    cpu.ebx = cpu.edx * 8;
    cpu.ebx += cpu.edx;
    cpu.edx = at(app, cpu.ebp - 0x10);
    cpu.eax = cpu.edx * 8;
    cpu.ebx <<= 6;
    cpu.eax += cpu.edx;
    cpu.al = byte(app, cpu.ebx + cpu.eax * 4 + kSurfaceSound);
    putByte(app, cpu.ecx + 0x990, cpu.al);
    return leave();
}

/* 0x429f10: car eax's skid sounds: the front wheels' (+0x990, +0x991) and
 * the back's (+0x992, +0x993) each a sound (0x4284e0) by how much they skid
 * -- both, half of both; or stopped once (+0x994, +0x995 whether on). */
void skidSounds(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
    cpu.esi = cpu.eax;
    if (byte(app, cpu.eax + 0x990) != 0 && byte(app, cpu.esi + 0x991) != 0)
    {
        fpu.push(f32(app, cpu.esi + 0x94c));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x950));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x537238));
        push(app, cpu, 0);
        cpu.eax = 0;
        cpu.ecx = 6;
        cpu.al = byte(app, cpu.esi + 0x990);
        app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
        fpu.pop();
        push(app, cpu, at(app, cpu.ebp - 4));
        cpu.ebx = 8;
        push(app, cpu, cpu.eax);
        cpu.edx = 2;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x994, 1);
        goto l42a013;
    }
    cpu.dh = byte(app, cpu.esi + 0x990);
    if (cpu.dh != 0)
    {
        push(app, cpu, 0);
        cpu.eax = at(app, cpu.esi + 0x94c);
        push(app, cpu, cpu.eax);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = 0;
        cpu.ecx = 6;
        cpu.al = cpu.dh;
        cpu.ebx = 8;
        push(app, cpu, cpu.eax);
        cpu.edx = 2;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x994, 1);
    }
    else
    {
        cpu.bl = byte(app, cpu.esi + 0x991);
        if (cpu.bl != 0)
        {
            push(app, cpu, 0);
            cpu.eax = at(app, cpu.esi + 0x950);
            push(app, cpu, cpu.eax);
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.eax = 0;
            cpu.ecx = 6;
            cpu.al = cpu.bl;
            cpu.edx = 2;
            push(app, cpu, cpu.eax);
            cpu.ebx = 8;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, 0x4284e0))
                return;
            putByte(app, cpu.esi + 0x994, 1);
        }
        else if (byte(app, cpu.esi + 0x994) != 0)
        {
            push(app, cpu, 0);
            cpu.ecx = 6;
            push(app, cpu, 0);
            cpu.ebx = 8;
            push(app, cpu, 0);
            cpu.edx = 3;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, 0x4284e0))
                return;
            putByte(app, cpu.esi + 0x994, 0);
        }
    }
l42a013:
    if (byte(app, cpu.esi + 0x992) != 0)
    {
        cpu.ch = byte(app, cpu.esi + 0x993);
        if (cpu.ch != 0)
        {
            fpu.push(f32(app, cpu.esi + 0x954));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x958));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x537238));
            push(app, cpu, 0);
            cpu.eax = 0;
            cpu.ebx = 0xa;
            cpu.al = cpu.ch;
            app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
            fpu.pop();
            push(app, cpu, at(app, cpu.ebp - 4));
            cpu.edx = 2;
            push(app, cpu, cpu.eax);
            cpu.ecx = 6;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, 0x4284e0))
                return;
            putByte(app, cpu.esi + 0x995, 1);
            return leave();
        }
    }
    if (byte(app, cpu.esi + 0x992) != 0)
    {
        push(app, cpu, 0);
        cpu.eax = at(app, cpu.esi + 0x954);
        push(app, cpu, cpu.eax);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = 0;
        cpu.ecx = 6;
        cpu.al = byte(app, cpu.esi + 0x992);
        cpu.ebx = 0xa;
        push(app, cpu, cpu.eax);
        cpu.edx = 4;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x995, 1);
        return leave();
    }
    if (byte(app, cpu.esi + 0x993) != 0)
    {
        push(app, cpu, 0);
        cpu.eax = at(app, cpu.esi + 0x958);
        push(app, cpu, cpu.eax);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = 0;
        cpu.ecx = 6;
        cpu.al = byte(app, cpu.esi + 0x993);
        cpu.ebx = 0xa;
        push(app, cpu, cpu.eax);
        cpu.edx = 4;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x995, 1);
        return leave();
    }
    if (byte(app, cpu.esi + 0x995) == 0)
        return leave();
    push(app, cpu, 0);
    cpu.ecx = 6;
    push(app, cpu, 0);
    cpu.ebx = 0xa;
    push(app, cpu, 0);
    cpu.edx = 5;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4284e0))
        return;
    putByte(app, cpu.esi + 0x995, 0);
    return leave();
}

/* 0x42a130: wheel edx of car eax hitting something (its bit of +0x99c), the
 * race on: dust past 5 (0x4cb010, 6), more past 20 (9), past 10 the sparks
 * (0x4cac20) where the wheel is, by its speed, and their sound (0x4284e0). */
void wheelSpray(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.eax = 1;
    cpu.cl = cpu.dl;
    cpu.edx = at(app, cpu.esi + 0x99c);
    cpu.eax = cpu.eax << (cpu.cl & 31);
    if ((cpu.eax & cpu.edx) == 0)
        return leave();
    if (half(app, cpu.esi + 0x164) != 0)
        return leave();
    fpu.push(f32(app, cpu.esi + 0xb8));
    fcomp(cpu, f64(app, 0x537240));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = cpu.ebx;
        cpu.eax <<= 3;
        cpu.edx = cpu.eax;
        cpu.eax <<= 4;
        cpu.eax -= cpu.edx;
        push(app, cpu, 6);
        cpu.ecx = at(app, cpu.esi + cpu.eax + 0x30c);
        push(app, cpu, cpu.ecx);
        cpu.edi = at(app, cpu.esi + cpu.eax + 0x308);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.esi + cpu.eax + 0x304);
        push(app, cpu, cpu.edx);
        cpu.eax = cpu.esi;
        cpu.edx = cpu.ebx;
        if (!byAddress(app, cpu, 0x4cb010))
            return;
    }
    fpu.push(f32(app, cpu.esi + 0xb8));
    fcomp(cpu, f64(app, 0x537248));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.eax = cpu.ebx * 8;
        cpu.edx = cpu.eax;
        cpu.eax <<= 4;
        cpu.eax -= cpu.edx;
        push(app, cpu, 9);
        cpu.ecx = at(app, cpu.esi + cpu.eax + 0x30c);
        push(app, cpu, cpu.ecx);
        cpu.edi = at(app, cpu.esi + cpu.eax + 0x308);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.esi + cpu.eax + 0x304);
        push(app, cpu, cpu.edx);
        cpu.eax = cpu.esi;
        cpu.edx = cpu.ebx;
        if (!byAddress(app, cpu, 0x4cb010))
            return;
    }
    fpu.push(f32(app, cpu.esi + 0xb8));
    app->getMemory<double>(cpu.ebp - 8) = double(fpu.st(0));
    fcomp(cpu, f64(app, 0x537250));
    cpu.ax = fpu.status.word;
    if (below(cpu) || equal(cpu))
        return leave();
    cpu.ebx <<= 3;
    cpu.eax = cpu.ebx;
    cpu.eax <<= 4;
    fpu.push(f64(app, cpu.ebp - 8));
    cpu.eax -= cpu.ebx;
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537260));
    fpu.push(f64(app, 0x537258));
    fpu.push(x86::Float(i32(at(app, cpu.esi + cpu.eax + 0x304))));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.push(x86::Float(i32(at(app, cpu.esi + cpu.eax + 0x308))));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(2));
    fpu.push(x86::Float(i32(at(app, cpu.esi + cpu.eax + 0x30c))));
    fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
    fpu.pop();
    cpu.esp -= 4;
    std::swap(fpu.st(0), fpu.st(2));
    app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
    fpu.pop();
    cpu.ecx = at(app, cpu.ebp - 0xc);
    std::swap(fpu.st(0), fpu.st(2));
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    push(app, cpu, cpu.ecx);
    app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
    fpu.pop();
    cpu.ebx = at(app, cpu.ebp - 0x10);
    push(app, cpu, cpu.ebx);
    app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
    fpu.pop();
    cpu.edi = at(app, cpu.ebp - 0x14);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4cac20))
        return;
    push(app, cpu, 0);
    cpu.edx = 1;
    push(app, cpu, 0);
    cpu.ecx = 5;
    push(app, cpu, 0x12);
    cpu.ebx = 0xffffffff;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4284e0))
        return;
    return leave();
}

/* 0x42a280: wheel edx's effect from its surface's record (0x553080 on: by
 * the track, the surface): skidding more than the surface's start, the
 * effect where the wheel is (0x4cb010) and, with no marks, its sound; else,
 * faster than the surface's skid starts, the effect, the back wheels' sound,
 * and how much it skids: (speed - start) / range by 5, 0 to 5 (+0x94c). */
void surfaceEffects(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    if (!direct(app, cpu, wheelSurface))
        return;
    cpu.edi = cpu.eax;
    if (i32(cpu.ebx) < i32(2))
    {
        put(app, cpu.ebp - 0x14, 1);
    }
    else
    {
        cpu.esi = 0;
        put(app, cpu.ebp - 0x14, cpu.esi);
    }
    cpu.edx = at(app, cpu.ecx + 0x160);
    cpu.esi = cpu.edx * 8;
    cpu.esi += cpu.edx;
    cpu.edx = cpu.edi;
    cpu.edi <<= 3;
    cpu.edi += cpu.edx;
    cpu.esi <<= 6;
    cpu.edi <<= 2;
    cpu.esi += cpu.edi;
    cpu.eax = at(app, cpu.esi + kSurfaceEffect);
    put(app, cpu.ebp - 4, cpu.eax);
    if (cpu.eax == 0)
        return leave();
    cpu.eax = at(app, cpu.esi + kSurfaceSkidFrom);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.esi + kSurfaceSkidOver);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = cpu.ebx * 4;
    cpu.edx = cpu.ecx + cpu.eax;
    cpu.eax = cpu.ecx + cpu.ebx;
    put(app, cpu.ebp - 0x18, cpu.eax);
    cpu.eax = cpu.ebx * 8;
    put(app, cpu.ebp - 0xc, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.eax <<= 4;
    cpu.eax -= cpu.edx;
    cpu.edx = cpu.ecx + cpu.eax;
    cpu.eax = at(app, cpu.ebp - 0xc);
    fpu.push(f32(app, cpu.eax + 0x94c));
    fcomp(cpu, f32(app, cpu.esi + kSurfaces));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.esi = at(app, cpu.ebp - 4);
        push(app, cpu, cpu.esi);
        cpu.eax = at(app, cpu.edx + 0x30c);
        push(app, cpu, cpu.eax);
        cpu.esi = at(app, cpu.edx + 0x308);
        push(app, cpu, cpu.esi);
        cpu.eax = at(app, cpu.edx + 0x304);
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x4cb010))
            return;
        cpu.edx = at(app, cpu.ecx + 0x160);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax <<= 6;
        cpu.eax += cpu.edi;
        if (at(app, cpu.eax + kSurfaceMarks) != 0)
            return leave();
        cpu.edx = at(app, cpu.ebp - 0x18);
        cpu.al = byte(app, cpu.eax + kSurfaceSound);
        putByte(app, cpu.edx + 0x990, cpu.al);
        return leave();
    }
    if ((at(app, cpu.ebp - 8) & 0x7fffffff) == 0)
        return leave();
    if ((at(app, cpu.ebp - 0x10) & 0x7fffffff) == 0)
        return leave();
    fpu.push(f32(app, cpu.ecx + 0xb8));
    fcomp(cpu, f32(app, cpu.ebp - 8));
    cpu.ax = fpu.status.word;
    if (below(cpu) || equal(cpu))
        return leave();
    cpu.eax = at(app, cpu.ebp - 4);
    push(app, cpu, cpu.eax);
    cpu.esi = at(app, cpu.edx + 0x30c);
    push(app, cpu, cpu.esi);
    cpu.eax = at(app, cpu.edx + 0x308);
    push(app, cpu, cpu.eax);
    cpu.esi = at(app, cpu.edx + 0x304);
    push(app, cpu, cpu.esi);
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x4cb010))
        return;
    if (at(app, cpu.ebp - 0x14) == 0)
    {
        cpu.edx = at(app, cpu.ecx + 0x160);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax <<= 6;
        cpu.edx = at(app, cpu.ebp - 0x18);
        cpu.al = byte(app, cpu.edi + cpu.eax + kSurfaceRearSound);
        putByte(app, cpu.edx + 0x990, cpu.al);
    }
    fpu.push(f32(app, cpu.ecx + 0xb8));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::div(fpu.st(0), f32(app, cpu.ebp - 0x10));
    cpu.edx = cpu.ebx * 4;
    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537268));
    cpu.edx += cpu.ecx;
    fpu.push(0.0);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.edx + 0x94c) = float(fpu.st(0));
    fpu.pop();
    fcomp(cpu, f32(app, cpu.edx + 0x94c));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        cpu.edx = 0;
        put(app, cpu.ebp - 0x28, cpu.edx);
        put(app, cpu.ebp - 0x24, cpu.edx);
    }
    else
    {
        fpu.push(f32(app, cpu.edx + 0x94c));
        app->getMemory<double>(cpu.ebp - 0x20) = double(fpu.st(0));
        fcomp(cpu, f64(app, 0x537268));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            cpu.edi = 0;
            cpu.eax = 0x40140000;
            put(app, cpu.ebp - 0x30, cpu.edi);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x20);
            put(app, cpu.ebp - 0x30, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x1c);
        }
        put(app, cpu.ebp - 0x2c, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x30);
        put(app, cpu.ebp - 0x28, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x2c);
        put(app, cpu.ebp - 0x24, cpu.eax);
    }
    fpu.push(f64(app, cpu.ebp - 0x28));
    app->getMemory<float>(cpu.ecx + cpu.ebx * 4 + 0x94c) = float(fpu.st(0));
    fpu.pop();
    return leave();
}

/* 0x42a480: wheel edx's skid mark: drawn on while it skids on a surface that
 * marks (extendSkidMark) -- a back wheel skidding past 5 smokes (0x4cb010,
 * 6) -- else ended (endSkidMark). */
void wheelSkid(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
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
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    if (i32(cpu.edx) < i32(2))
    {
        cpu.esi = 1;
    }
    else
    {
        cpu.esi = 0;
    }
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, wheelSurface))
        return;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = cpu.ecx * 4;
    cpu.edi = cpu.ebx + cpu.eax;
    if ((at(app, cpu.edi + 0x94c) & 0x7fffffff) != 0)
    {
        cpu.edx = at(app, cpu.ebx + 0x160);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax <<= 6;
        put(app, cpu.ebp - 0xc, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax &= 0xf;
        put(app, cpu.ebp - 8, cpu.eax);
        cpu.edx = at(app, cpu.ebp - 8);
        cpu.eax <<= 3;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, cpu.ebp - 0xc);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        if (at(app, cpu.eax + kSurfaceMarks) != 0)
        {
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, extendSkidMark))
                return;
            if (cpu.esi != 0)
                return leave();
            fpu.push(f32(app, cpu.edi + 0x94c));
            fcomp(cpu, f64(app, 0x537270));
            cpu.ax = fpu.status.word;
            if (below(cpu) || equal(cpu))
                return leave();
            cpu.eax = cpu.ecx * 8;
            cpu.edx = cpu.eax;
            cpu.eax <<= 4;
            cpu.eax -= cpu.edx;
            push(app, cpu, 6);
            cpu.esi = at(app, cpu.ebx + cpu.eax + 0x30c);
            push(app, cpu, cpu.esi);
            cpu.edi = at(app, cpu.ebx + cpu.eax + 0x308);
            push(app, cpu, cpu.edi);
            cpu.edx = at(app, cpu.ebx + cpu.eax + 0x304);
            push(app, cpu, cpu.edx);
            cpu.eax = cpu.ebx;
            cpu.edx = cpu.ecx;
            if (!byAddress(app, cpu, 0x4cb010))
                return;
            return leave();
        }
    }
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, endSkidMark))
        return;
    return leave();
}

/* 0x42a560: car eax's wheels: what it hit (+0x99c, kept while +0x621 has
 * 0x40, +0x9a0 the frames since): the first frame a sound, the first two the
 * wheels' dust (0x4cb010, 13); then each wheel, on the ground with the race
 * on: its skid (+0x94c) from the slides (+0x5c0 front, +0x5c4 back, by 7.5,
 * 0 to 10), its mark, its surface's effect, what it hit (wheelSkid,
 * surfaceEffects, wheelSpray); in the air its mark cut (0x4db860).  Then the
 * sounds (skidSounds). */
void wheelEffects(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    cpu.esi = cpu.eax;
    if ((byte(app, cpu.eax + 0x621) & 0x40) == 0)
    {
        put(app, cpu.esi + 0x9a0, 0);
        put(app, cpu.esi + 0x99c, 0);
    }
    else
    {
        put(app, cpu.esi + 0x9a0, at(app, cpu.esi + 0x9a0) + 1);
    }
    if (half(app, cpu.esi + 0x164) == 0 && at(app, cpu.esi + 0x99c) != 0)
    {
        cpu.ebx = at(app, cpu.esi + 0x9a0);
        if (i32(cpu.ebx) < i32(3))
        {
            if (cpu.ebx == 1)
            {
                push(app, cpu, 0);
                cpu.ecx = 6;
                push(app, cpu, 0x41200000);
                cpu.ebx = 0xffffffff;
                push(app, cpu, 0x13);
                cpu.edx = 1;
                cpu.eax = cpu.esi;
                if (!byAddress(app, cpu, 0x4284e0))
                    return;
            }
            if ((byte(app, cpu.esi + 0x99c) & 1) != 0)
            {
                push(app, cpu, 0xd);
                cpu.eax = at(app, cpu.esi + 0x30c);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.esi + 0x308);
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.esi + 0x304);
                push(app, cpu, cpu.ecx);
                cpu.eax = cpu.esi;
                cpu.edx = 0;
                if (!byAddress(app, cpu, 0x4cb010))
                    return;
            }
            if ((byte(app, cpu.esi + 0x99c) & 2) != 0)
            {
                push(app, cpu, 0xd);
                cpu.ebx = at(app, cpu.esi + 0x384);
                push(app, cpu, cpu.ebx);
                cpu.edi = at(app, cpu.esi + 0x380);
                push(app, cpu, cpu.edi);
                cpu.eax = at(app, cpu.esi + 0x37c);
                push(app, cpu, cpu.eax);
                cpu.edx = 1;
                cpu.eax = cpu.esi;
                if (!byAddress(app, cpu, 0x4cb010))
                    return;
            }
            if ((byte(app, cpu.esi + 0x99c) & 4) != 0)
            {
                push(app, cpu, 0xd);
                cpu.edx = at(app, cpu.esi + 0x3fc);
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.esi + 0x3f8);
                push(app, cpu, cpu.ecx);
                cpu.ebx = at(app, cpu.esi + 0x3f4);
                push(app, cpu, cpu.ebx);
                cpu.eax = cpu.esi;
                cpu.edx = 2;
                if (!byAddress(app, cpu, 0x4cb010))
                    return;
            }
            if ((byte(app, cpu.esi + 0x99c) & 8) != 0)
            {
                push(app, cpu, 0xd);
                cpu.edi = at(app, cpu.esi + 0x474);
                push(app, cpu, cpu.edi);
                cpu.eax = at(app, cpu.esi + 0x470);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.esi + 0x46c);
                push(app, cpu, cpu.edx);
                cpu.eax = cpu.esi;
                cpu.edx = 3;
                if (!byAddress(app, cpu, 0x4cb010))
                    return;
            }
        }
    }
    // Each wheel: its sound off; on the ground with the race on, its skid
    // and effects, else its mark cut and its skid none.
    cpu.ecx = 0;
    for (;;)
    {
        cpu.eax = cpu.esi + cpu.ecx;
        putByte(app, cpu.eax + 0x990, 0);
        bool skids = false;
        if (half(app, cpu.esi + 0x164) == 0)
        {
            fpu.push(f32(app, cpu.esi + 0x10c));
            fcomp(cpu, f64(app, 0x537278));
            cpu.ax = fpu.status.word;
            skids = !below(cpu);
        }
        if (skids)
        {
            if (i32(cpu.ecx) < i32(2))
            {
                fpu.push(f32(app, cpu.esi + 0x5c0));
                fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537280));
                fpu.push(0.0);
                std::swap(fpu.st(0), fpu.st(1));
                app->getMemory<double>(cpu.ebp - 0x20) = double(fpu.st(0));
                fpu.pop();
                fcomp(cpu, f64(app, cpu.ebp - 0x20));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.ebx = 0;
                    put(app, cpu.ebp - 0x10, cpu.ebx);
                    put(app, cpu.ebp - 0xc, cpu.ebx);
                }
                else
                {
                    fpu.push(f64(app, cpu.ebp - 0x20));
                    fcomp(cpu, f64(app, 0x537288));
                    cpu.ax = fpu.status.word;
                    if (!below(cpu) && !equal(cpu))
                    {
                        cpu.eax = 0;
                        cpu.edx = 0x40240000;
                        put(app, cpu.ebp - 0x30, cpu.eax);
                        put(app, cpu.ebp - 0x2c, cpu.edx);
                    }
                    else
                    {
                        cpu.eax = at(app, cpu.ebp - 0x20);
                        put(app, cpu.ebp - 0x30, cpu.eax);
                        cpu.eax = at(app, cpu.ebp - 0x1c);
                        put(app, cpu.ebp - 0x2c, cpu.eax);
                    }
                    cpu.eax = at(app, cpu.ebp - 0x30);
                    put(app, cpu.ebp - 0x10, cpu.eax);
                    cpu.eax = at(app, cpu.ebp - 0x2c);
                    put(app, cpu.ebp - 0xc, cpu.eax);
                }
                fpu.push(f64(app, cpu.ebp - 0x10));
            }
            else
            {
                fpu.push(0.0);
                fpu.push(f32(app, cpu.esi + 0x5c4));
                app->getMemory<double>(cpu.ebp - 0x18) = double(fpu.st(0));
                fpu.pop();
                fcomp(cpu, f64(app, cpu.ebp - 0x18));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.ebx = 0;
                    put(app, cpu.ebp - 0x28, cpu.ebx);
                    put(app, cpu.ebp - 0x24, cpu.ebx);
                }
                else
                {
                    fpu.push(f64(app, cpu.ebp - 0x18));
                    fcomp(cpu, f64(app, 0x537288));
                    cpu.ax = fpu.status.word;
                    if (!below(cpu) && !equal(cpu))
                    {
                        cpu.eax = 0;
                        cpu.edx = 0x40240000;
                        put(app, cpu.ebp - 8, cpu.eax);
                        put(app, cpu.ebp - 4, cpu.edx);
                    }
                    else
                    {
                        cpu.eax = at(app, cpu.ebp - 0x18);
                        put(app, cpu.ebp - 8, cpu.eax);
                        cpu.eax = at(app, cpu.ebp - 0x14);
                        put(app, cpu.ebp - 4, cpu.eax);
                    }
                    cpu.eax = at(app, cpu.ebp - 8);
                    put(app, cpu.ebp - 0x28, cpu.eax);
                    cpu.eax = at(app, cpu.ebp - 4);
                    put(app, cpu.ebp - 0x24, cpu.eax);
                }
                fpu.push(f64(app, cpu.ebp - 0x28));
            }
            app->getMemory<float>(cpu.esi + cpu.ecx * 4 + 0x94c) = float(fpu.st(0));
            fpu.pop();
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, wheelSkid))
                return;
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, surfaceEffects))
                return;
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, wheelSpray))
                return;
        }
        else
        {
            cpu.edx = cpu.ecx;
            cpu.eax = at(app, cpu.esi);
            if (!byAddress(app, cpu, 0x4db860))
                return;
            cpu.eax = cpu.esi + cpu.ecx;
            putByte(app, cpu.eax + 0x95c, 0);
            put(app, cpu.esi + cpu.ecx * 4 + 0x94c, 0);
        }
        ++cpu.ecx;
        if (i32(cpu.ecx) >= i32(4))
            break;
    }
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, skidSounds))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x42a7d0: car eax's camera shake (+0x920 across, +0x924 up, +0x928 and
 * +0x92c their springs): the springs pulled by the car's turn (+0x4e0, +0x4e8
 * less gravity on its axes) and let go by a quarter, clamped to the shake's
 * size, the crash waves added (shakeWave), the road's rumble (shakeAmplitude,
 * randomFraction) on top -- smaller in a series or two player race.  None for
 * a wreck or high above ([+0x84] 60).  The waves' frames run down, two a
 * frame for a light car. */
void updateShake(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x54;
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
    cpu.edx = cpu.eax;
    if (i32(at(app, cpu.eax + 0x84)) < i32(0x42700000) && (byte(app, cpu.eax + 0x200) & 0x10) == 0)
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        fpu.push(f32(app, kShakeAcross));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x294));
        fpu.push(f32(app, kShakeUp));
        cpu.edi = cpu.ebp - 0x54;
        cpu.esi = cpu.edx + 0x4e0;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x294));
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
        fpu.push(f32(app, 0x537290));
        fpu.push(f32(app, cpu.edx + 0xdc));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        fpu.push(f32(app, cpu.edx + 0xc4));
        fpu.st(2) = fp::mul(fpu.st(2), fpu.st(0));
        fpu.pop();
        fpu.push(f32(app, cpu.edx + 0x928));
        fpu.push(fpu.st(0));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537298));
        fpu.push(0.0);
        std::swap(fpu.st(0), fpu.st(6));
        app->getMemory<float>(cpu.ebp - 0x1c) = float(fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(4));
        app->getMemory<float>(cpu.ebp - 0x18) = float(fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x4c), fpu.st(0));
        std::swap(fpu.st(0), fpu.st(2));
        fpu.st(0) = fp::sub(f32(app, cpu.ebp - 0x54), fpu.st(0));
        std::swap(fpu.st(0), fpu.st(3));
        fpu.st(1) = fpu.st(0);
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        app->getMemory<float>(cpu.ebp - 0x4c) = float(fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        app->getMemory<float>(cpu.ebp - 0x54) = float(fpu.st(0));
        fpu.pop();
        app->getMemory<float>(cpu.edx + 0x928) = float(fpu.st(0));
        fpu.pop();
        fcomp(cpu, f32(app, cpu.ebp - 0x4c));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x4c));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x4c);
            put(app, cpu.ebp - 0xc, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0xc));
        fcomp(cpu, f64(app, 0x5372a0));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x4c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x1c));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5372a8));
            fpu.st(0) = fp::sub(f32(app, cpu.edx + 0x928), fpu.st(0));
            app->getMemory<float>(cpu.edx + 0x928) = float(fpu.st(0));
            fpu.pop();
        }
        fpu.push(f32(app, cpu.edx + 0x928));
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x20) = float(fpu.st(0));
        fpu.pop();
        fcomp(cpu, f32(app, cpu.ebp - 0x20));
        cpu.ax = fpu.status.word;
        if (below(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x20);
        }
        else
        {
            fpu.push(f32(app, cpu.edx + 0x928));
            fcomp(cpu, f32(app, cpu.ebp - 0x1c));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x1c);
            }
            else
            {
                cpu.eax = at(app, cpu.edx + 0x928);
            }
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 4);
        }
        put(app, cpu.ebp - 0x14, cpu.eax);
        fpu.push(f32(app, cpu.edx + 0x92c));
        fpu.push(fpu.st(0));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x537298));
        cpu.eax = at(app, cpu.ebp - 0x14);
        fpu.push(0.0);
        put(app, cpu.edx + 0x928, cpu.eax);
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(2) = fpu.st(0);
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        app->getMemory<float>(cpu.edx + 0x92c) = float(fpu.st(0));
        fpu.pop();
        fcomp(cpu, f32(app, cpu.ebp - 0x54));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x54));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x3c) = float(fpu.st(0));
            fpu.pop();
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x54);
            put(app, cpu.ebp - 0x3c, cpu.eax);
        }
        fpu.push(f32(app, cpu.ebp - 0x3c));
        fcomp(cpu, f64(app, 0x5372b0));
        cpu.ax = fpu.status.word;
        if (!below(cpu) && !equal(cpu))
        {
            fpu.push(f32(app, cpu.ebp - 0x54));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x18));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5372a8));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x92c));
            app->getMemory<float>(cpu.edx + 0x92c) = float(fpu.st(0));
            fpu.pop();
        }
        fpu.push(f32(app, cpu.edx + 0x92c));
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 0x40) = float(fpu.st(0));
        fpu.pop();
        fcomp(cpu, f32(app, cpu.ebp - 0x40));
        cpu.ax = fpu.status.word;
        if (below(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 0x40);
        }
        else
        {
            fpu.push(f32(app, cpu.edx + 0x92c));
            fcomp(cpu, f32(app, cpu.ebp - 0x18));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
            }
            else
            {
                cpu.eax = at(app, cpu.edx + 0x92c);
            }
            put(app, cpu.ebp - 0x2c, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x2c);
        }
        put(app, cpu.ebp - 0x30, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x30);
        cpu.bx = half(app, cpu.edx + 0x164);
        put(app, cpu.edx + 0x92c, cpu.eax);
        if (cpu.bx == 0)
        {
            fpu.push(f32(app, cpu.edx + 0x92c));
            cpu.ebx = at(app, cpu.edx + 0x8d0);
            app->getMemory<float>(cpu.edx + 0x924) = float(fpu.st(0));
            fpu.pop();
            if (cpu.ebx != 0)
            {
                push(app, cpu, cpu.ebx);
                push(app, cpu, at(app, cpu.edx + 0x8cc));
                if (!direct(app, cpu, shakeWave))
                    return;
                fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x924));
                app->getMemory<float>(cpu.edx + 0x924) = float(fpu.st(0));
                fpu.pop();
            }
            fpu.push(f32(app, cpu.edx + 0x928));
            cpu.edi = at(app, cpu.edx + 0x8c8);
            app->getMemory<float>(cpu.edx + 0x920) = float(fpu.st(0));
            fpu.pop();
            if (cpu.edi != 0)
            {
                push(app, cpu, cpu.edi);
                push(app, cpu, at(app, cpu.edx + 0x8c4));
                if (!direct(app, cpu, shakeWave))
                    return;
                fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x920));
                app->getMemory<float>(cpu.edx + 0x920) = float(fpu.st(0));
                fpu.pop();
            }
            fpu.push(f32(app, cpu.edx + 0x920));
            fpu.push(f32(app, cpu.ebp - 0x18));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x28) = float(fpu.st(0));
            fpu.pop();
            fcomp(cpu, f32(app, cpu.ebp - 0x28));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x28);
            }
            else
            {
                fpu.push(f32(app, cpu.edx + 0x920));
                fcomp(cpu, f32(app, cpu.ebp - 0x18));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0x18);
                }
                else
                {
                    cpu.eax = at(app, cpu.edx + 0x920);
                }
                put(app, cpu.ebp - 8, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 8);
            }
            put(app, cpu.ebp - 0x10, cpu.eax);
            fpu.push(f32(app, cpu.ebp - 0x18));
            fpu.push(f32(app, cpu.edx + 0x924));
            cpu.eax = at(app, cpu.ebp - 0x10);
            std::swap(fpu.st(0), fpu.st(1));
            fpu.st(0) = -fpu.st(0);
            app->getMemory<float>(cpu.ebp - 0x24) = float(fpu.st(0));
            fpu.pop();
            put(app, cpu.edx + 0x920, cpu.eax);
            fcomp(cpu, f32(app, cpu.ebp - 0x24));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 0x24);
            }
            else
            {
                fpu.push(f32(app, cpu.edx + 0x924));
                fcomp(cpu, f32(app, cpu.ebp - 0x18));
                cpu.ax = fpu.status.word;
                if (!below(cpu) && !equal(cpu))
                {
                    cpu.eax = at(app, cpu.ebp - 0x18);
                }
                else
                {
                    cpu.eax = at(app, cpu.edx + 0x924);
                }
                put(app, cpu.ebp - 0x34, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 0x34);
            }
            put(app, cpu.ebp - 0x38, cpu.eax);
            cpu.eax = at(app, cpu.ebp - 0x38);
            put(app, cpu.edx + 0x924, cpu.eax);
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, shakeAmplitude))
                return;
            if (i32(at(app, kPlayMode)) <= i32(1))
            {
                cpu.ebx = at(app, kRaceType);
                if (cpu.ebx != 1 && cpu.ebx != 2)
                {
                    if (!byAddress(app, cpu, 0x4eb230))
                        return;
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x930));
                    fpu.push(f32(app, cpu.edx + 0x930));
                    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372b8));
                    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
                    fpu.pop();
                    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372c0));
                    cpu.eax = at(app, cpu.edx + 0x220);
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x50));
                    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x920));
                    app->getMemory<float>(cpu.edx + 0x920) = float(fpu.st(0));
                    fpu.pop();
                    if (!byAddress(app, cpu, 0x4eb230))
                        return;
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x930));
                    fpu.push(f32(app, cpu.edx + 0x930));
                    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372b8));
                    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
                    fpu.pop();
                    fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372c8));
                    cpu.eax = at(app, cpu.edx + 0x220);
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x50));
                    goto l42ab5c;
                }
            }
            if (!byAddress(app, cpu, 0x4eb230))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x930));
            fpu.push(f32(app, cpu.edx + 0x930));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372b8));
            fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
            fpu.pop();
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372c0));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x920));
            app->getMemory<float>(cpu.edx + 0x920) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, 0x4eb230))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x930));
            fpu.push(f32(app, cpu.edx + 0x930));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372b8));
            fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
            fpu.pop();
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372c8));
l42ab5c:
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x924));
            app->getMemory<float>(cpu.edx + 0x924) = float(fpu.st(0));
            fpu.pop();
            goto l42abb0;
        }
        put(app, cpu.edx + 0x930, 0);
        cpu.eax = at(app, cpu.edx + 0x930);
        put(app, cpu.edx + 0x920, cpu.eax);
        cpu.eax = at(app, cpu.edx + 0x920);
        put(app, cpu.edx + 0x924, cpu.eax);
        goto l42abb0;
    }
    put(app, cpu.edx + 0x930, 0);
    cpu.eax = at(app, cpu.edx + 0x930);
    put(app, cpu.edx + 0x920, cpu.eax);
    cpu.eax = at(app, cpu.edx + 0x920);
    put(app, cpu.edx + 0x924, cpu.eax);
l42abb0:
    cpu.edi = at(app, cpu.edx + 0x8c8);
    if (i32(cpu.edi) > 0)
    {
        cpu.eax = at(app, cpu.edx + 0x528);
        if (i32(at(app, cpu.eax + 0x294)) < i32(0x3f000000))
        {
            if (i32(cpu.edi) > i32(1))
            {
                cpu.esi = cpu.edi - 2;
                put(app, cpu.edx + 0x8c8, cpu.esi);
            }
            else
            {
                --cpu.edi;
                put(app, cpu.edx + 0x8c8, cpu.edi);
            }
        }
        else
        {
            cpu.eax = cpu.edi - 1;
            put(app, cpu.edx + 0x8c8, cpu.eax);
        }
    }
    cpu.ecx = at(app, cpu.edx + 0x8d0);
    if (i32(cpu.ecx) <= 0)
        return leave();
    cpu.eax = at(app, cpu.edx + 0x528);
    if (i32(at(app, cpu.eax + 0x294)) < i32(0x3f000000))
    {
        if (i32(cpu.ecx) > i32(1))
        {
            cpu.edi = cpu.ecx - 2;
            put(app, cpu.edx + 0x8d0, cpu.edi);
            return leave();
        }
        cpu.eax = cpu.ecx - 1;
        put(app, cpu.edx + 0x8d0, cpu.eax);
        return leave();
    }
    --cpu.ecx;
    put(app, cpu.edx + 0x8d0, cpu.ecx);
    return leave();
}

/* 0x42ac40: car eax's crash (+0x170: its kind in the high word, the other
 * car's in the low byte): the force controller's jolt (controls::crashJolt);
 * hit hard ([+0x168] over 5): pushed (+0x8d4 a 15th of the hit), turned
 * round its axes (+0x8d8, 0x4e0110, 0x4e0050), the shake (startShake) and
 * the crash's sound and effects by its kind (0x4cae00, 0x4cac20, 0x4bd9d0,
 * 0x4284e0); a car hit fast (22) its kind's effect, landing or scraping the
 * scenery its sparks, its scrape's sound on and off (+0x996). */
void crashEffects(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x70;
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
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.eax + 0x170);
    cpu.eax &= 0xefff0000;
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.eax + 0x170);
    cpu.eax &= 0xff;
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, 0x471dc0))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = at(app, cpu.eax + 0x168);
        if ((cpu.edx & 0x7fffffff) != 0 && at(app, cpu.ebp - 8) != 0x60000)
        {
            cpu.ebx = at(app, cpu.eax + 0x180);
            push(app, cpu, cpu.edx);
            push(app, cpu, cpu.ebx);
            cpu.esi = at(app, cpu.eax + 0x17c);
            push(app, cpu, cpu.esi);
            cpu.edi = at(app, cpu.eax + 0x178);
            push(app, cpu, cpu.edi);
            if (!byAddress(app, cpu, 0x471270))
                return;
        }
    }
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.eax + 0x160);
    cpu.ecx = cpu.edx * 8;
    cpu.ecx += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x18c);
    cpu.eax = cpu.edx * 8;
    cpu.eax += cpu.edx;
    cpu.ecx <<= 6;
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.edi = cpu.ebp - 0x70;
    cpu.ecx = 9;
    cpu.esi = cpu.eax + kSurfaces;
    cpu.eax = at(app, cpu.ebp - 4);
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
    if (i32(at(app, cpu.eax + 0x168)) > i32(0x40a00000))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        cpu.eax = reg32(i32(cpu.eax) >> 0x10);
        cpu.esi = at(app, cpu.eax * 4 + kCrashChannels);
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.dl = byte(app, cpu.eax + 0x200);
        if ((cpu.dl & 0x10) == 0)
        {
            if (i32(at(app, cpu.eax + 0x168)) <= i32(0x41f00000))
                goto l42ae43;
        }
        cpu.eax = at(app, cpu.ebp - 4);
        if (at(app, cpu.eax + 0x8d4) == 0)
        {
            fpu.push(f32(app, cpu.eax + 0x168));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372d0));
            cpu.edx = at(app, cpu.ebp - 4);
            cpu.esp -= 4;
            put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
            fpu.pop();
            cpu.eax = pop(app, cpu);
            cpu.edx += 0x8d4;
            if (i32(cpu.eax) > 0)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                fpu.push(f32(app, cpu.eax + 0x168));
                fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372d0));
                cpu.esp -= 4;
                put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
                fpu.pop();
                cpu.eax = pop(app, cpu);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 4);
                fpu.push(f32(app, cpu.eax + 0x168));
                fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372d0));
                cpu.esp -= 4;
                put(app, cpu.esp, reg32(fp::toInteger<i32>(fpu.st(0))));
                fpu.pop();
                cpu.eax = pop(app, cpu);
                cpu.eax = 0u - cpu.eax;
            }
            cpu.edi = at(app, cpu.edx);
            cpu.ecx = at(app, cpu.ebp - 4);
            cpu.edi += cpu.eax;
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.ecx += 0x8d8;
            cpu.eax += 0xc0;
            put(app, cpu.edx, cpu.edi);
            cpu.ebx = cpu.ecx;
            fpu.push(f32(app, cpu.eax + 0xb8));
            fpu.push(f32(app, cpu.eax + 0xbc));
            fpu.push(f32(app, cpu.eax + 0xc0));
            cpu.edx = cpu.ebp - 0x4c;
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax - 0x28));
            std::swap(fpu.st(0), fpu.st(1));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax - 0x24));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax - 0x20));
            std::swap(fpu.st(0), fpu.st(1));
            app->getMemory<float>(cpu.eax + 0x818) = float(fpu.st(0));
            fpu.pop();
            std::swap(fpu.st(0), fpu.st(1));
            app->getMemory<float>(cpu.eax + 0x81c) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.eax + 0x820) = float(fpu.st(0));
            fpu.pop();
            if (!byAddress(app, cpu, 0x4e0110))
                return;
            cpu.edx = cpu.ebp - 0x4c;
            cpu.eax = cpu.ecx;
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx));
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 4));
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 8));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x10));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x14));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x18));
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x1c));
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x20));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            cpu.eax = cpu.eax + 0xc;
            app->getMemory<float>(cpu.ebx) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.ebx + 4) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.ebx + 8) = float(fpu.st(0));
            fpu.pop();
            push(app, cpu, cpu.ecx);
            cpu.eax = 1;
            cpu.edx = cpu.ecx;
            push(app, cpu, 0x3f19999a);
            if (!byAddress(app, cpu, 0x4e0050))
                return;
        }
l42ae43:
        cpu.eax = at(app, cpu.ebp - 8);
        if (cpu.eax >= 0x30000)
        {
            if (cpu.eax <= 0x30000)
                goto l42af06;
            if (cpu.eax >= 0x50000)
            {
                if (cpu.eax <= 0x50000)
                    goto l42b00e;
                if (cpu.eax == 0x60000)
                    goto l42b09a;
                goto l42b0ec;
            }
            if (cpu.eax == 0x40000)
                goto l42ae9d;
            goto l42b0ec;
        }
        if (cpu.eax < 0x10000)
            goto l42b0ec;
        if (cpu.eax <= 0x10000)
            goto l42af83;
        if (cpu.eax == 0x20000)
            goto l42af5a;
        goto l42b0ec;
l42ae9d:
        cpu.edi = at(app, cpu.ebp - 0xc);
        cpu.ecx = at(app, cpu.edi * 4 + kCrashSounds);
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = 0x23;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx += 0x10;
        if (!direct(app, cpu, startShake))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = at(app, cpu.eax + 0x180);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, cpu.eax + 0x17c);
        push(app, cpu, cpu.ebx);
        cpu.edx = at(app, cpu.eax + 0x178);
        push(app, cpu, cpu.edx);
        cpu.eax = at(app, cpu.edi * 4 + kCrashEffects);
        if (!byAddress(app, cpu, 0x4cae00))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 0x1f0);
        cpu.eax = 5;
        if (!byAddress(app, cpu, 0x4bd9d0))
            return;
        goto l42b0f1;
l42af06:
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.ebx = at(app, cpu.eax + 0x10c);
        cpu.ecx = at(app, cpu.ebp - 0x54);
        if (i32(cpu.ebx) < i32(0x3f666666))
            cpu.esi = 5;
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = cpu.edx + 0x10;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0x23;
        if (!direct(app, cpu, startShake))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 0x1f0);
        cpu.eax = 3;
        if (!byAddress(app, cpu, 0x4bd9d0))
            return;
        goto l42b0f1;
l42af5a:
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = 0x23;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx += 0x18;
        cpu.ecx = at(app, cpu.ebp - 0x54);
        if (!direct(app, cpu, startShake))
            return;
        goto l42b0f1;
l42af83:
        cpu.edi = at(app, cpu.ebp - 0x5c);
        cpu.ecx = at(app, cpu.ebp - 0x54);
        if (cpu.edi == 3)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = at(app, cpu.eax + 0x180);
            push(app, cpu, at(app, cpu.eax + 0xb8));
            push(app, cpu, cpu.edx);
            cpu.ebx = at(app, cpu.eax + 0x17c);
            push(app, cpu, cpu.ebx);
            cpu.edi = at(app, cpu.eax + 0x178);
            push(app, cpu, cpu.edi);
            if (!byAddress(app, cpu, 0x4cac20))
                return;
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = at(app, cpu.eax + 0x180);
            push(app, cpu, cpu.edx);
            cpu.ebx = at(app, cpu.eax + 0x17c);
            push(app, cpu, cpu.ebx);
            cpu.edi = at(app, cpu.eax + 0x178);
            push(app, cpu, cpu.edi);
            cpu.eax = at(app, cpu.ebp - 0x5c);
            if (!byAddress(app, cpu, 0x4cae00))
                return;
        }
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = cpu.edx + 0x10;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0x23;
        if (!direct(app, cpu, startShake))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 0x1f0);
        cpu.eax = 2;
        if (!byAddress(app, cpu, 0x4bd9d0))
            return;
        goto l42b0f1;
l42b00e:
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = cpu.edx + 0x18;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0x23;
        if (!direct(app, cpu, startShake))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.eax + 0x168));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, 0x5372d8));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fpu.st(0));
        fpu.pop();
        cpu.edx = at(app, cpu.eax + 0x180);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, cpu.eax + 0x17c);
        push(app, cpu, cpu.ebx);
        cpu.edi = at(app, cpu.eax + 0x178);
        push(app, cpu, cpu.edi);
        if (!byAddress(app, cpu, 0x4cac20))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = at(app, cpu.eax + 0x180);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, cpu.eax + 0x17c);
        push(app, cpu, cpu.ebx);
        cpu.edi = at(app, cpu.eax + 0x178);
        push(app, cpu, cpu.edi);
        cpu.eax = 2;
        if (!byAddress(app, cpu, 0x4cae00))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 0x1f0);
        cpu.eax = 4;
        cpu.ecx = at(app, cpu.ebp - 0xc);
        if (!byAddress(app, cpu, 0x4bd9d0))
            return;
        goto l42b0f1;
l42b09a:
        cpu.ecx = at(app, cpu.ebp - 0xc);
        if (cpu.ecx == 0xb)
            goto l42b0f1;
        if (cpu.ecx == 9)
            goto l42b0f1;
        if (!byAddress(app, cpu, 0x4eb1fc))
            return;
        cpu.edx = cpu.eax;
        cpu.ebx = 8;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        idiv(cpu, cpu.ebx);
        cpu.ebx = cpu.edx + 0x10;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = 0x23;
        if (!direct(app, cpu, startShake))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.ebx = at(app, cpu.eax + 0x180);
        push(app, cpu, cpu.ebx);
        cpu.edi = at(app, cpu.eax + 0x17c);
        push(app, cpu, cpu.edi);
        cpu.edx = at(app, cpu.eax + 0x178);
        push(app, cpu, cpu.edx);
        cpu.eax = 2;
        if (!byAddress(app, cpu, 0x4cae00))
            return;
        goto l42b0f1;
l42b0ec:
        cpu.ecx = 1;
l42b0f1:
        cpu.eax = at(app, cpu.ebp - 4);
        push(app, cpu, 0);
        fpu.push(f32(app, cpu.eax + 0x168));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5372e0));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fpu.st(0));
        fpu.pop();
        cpu.ebx = 0xffffffff;
        push(app, cpu, cpu.ecx);
        cpu.edx = 1;
        cpu.ecx = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    if ((at(app, cpu.eax + 0x168) & 0x7fffffff) != 0 && at(app, cpu.ebp - 8) == 0x40000)
    {
        fpu.push(f32(app, cpu.eax + 0xb8));
        fcomp(cpu, f32(app, kCrashSpeed));
        cpu.ax = fpu.status.word;
        if (!below(cpu))
        {
            cpu.eax = at(app, cpu.ebp - 4);
            if (i32(at(app, cpu.eax + 0x15c)) < i32(0x40400000))
            {
                cpu.eax = at(app, cpu.ebp - 0xc);
                cpu.eax <<= 2;
                if (at(app, cpu.eax + kCrashEffects) == 3)
                {
                    cpu.eax = at(app, cpu.ebp - 4);
                    fpu.push(f32(app, cpu.eax + 0xb8));
                    fpu.st(0) = fp::mul(fpu.st(0), f32(app, 0x5372e4));
                    cpu.esp -= 4;
                    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
                    fpu.pop();
                    cpu.edx = at(app, cpu.eax + 0x180);
                    push(app, cpu, cpu.edx);
                    cpu.ecx = at(app, cpu.eax + 0x17c);
                    push(app, cpu, cpu.ecx);
                    cpu.ebx = at(app, cpu.eax + 0x178);
                    push(app, cpu, cpu.ebx);
                    if (!byAddress(app, cpu, 0x4cac20))
                        return;
                    push(app, cpu, 0);
                    cpu.edx = 1;
                    push(app, cpu, 0);
                    cpu.ecx = 5;
                    push(app, cpu, 0x12);
                    cpu.ebx = 0xffffffff;
                    cpu.eax = at(app, cpu.ebp - 4);
                    if (!byAddress(app, cpu, 0x4284e0))
                        return;
                }
                else
                {
                    cpu.edx = at(app, cpu.ebp - 4);
                    cpu.esi = at(app, cpu.edx + 0x180);
                    push(app, cpu, cpu.esi);
                    cpu.edi = at(app, cpu.edx + 0x17c);
                    push(app, cpu, cpu.edi);
                    cpu.ecx = at(app, cpu.edx + 0x178);
                    push(app, cpu, cpu.ecx);
                    cpu.eax = at(app, cpu.eax + kCrashEffects);
                    if (!byAddress(app, cpu, 0x4cae00))
                        return;
                }
            }
            else
            {
                cpu.ebx = at(app, cpu.eax + 0x180);
                push(app, cpu, cpu.ebx);
                cpu.esi = at(app, cpu.eax + 0x17c);
                push(app, cpu, cpu.esi);
                cpu.edi = at(app, cpu.eax + 0x178);
                push(app, cpu, cpu.edi);
                cpu.eax = 2;
                if (!byAddress(app, cpu, 0x4cae00))
                    return;
            }
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 4);
            fpu.push(f32(app, cpu.eax + 0xb8));
            fcomp(cpu, f64(app, 0x5372e8));
            cpu.ax = fpu.status.word;
            if (below(cpu) || equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 4);
                fpu.push(f32(app, cpu.eax + 0x168));
                fcomp(cpu, f64(app, 0x5372e8));
                cpu.ax = fpu.status.word;
                if (below(cpu) || equal(cpu))
                    goto l42b252;
            }
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = at(app, cpu.eax + 0x180);
            push(app, cpu, cpu.edx);
            cpu.ecx = at(app, cpu.eax + 0x17c);
            push(app, cpu, cpu.ecx);
            cpu.ebx = at(app, cpu.eax + 0x178);
            push(app, cpu, cpu.ebx);
            cpu.eax = 6;
            if (!byAddress(app, cpu, 0x4cae00))
                return;
        }
    }
l42b252:
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.eax + 0x15c));
    fcomp(cpu, f64(app, 0x5372f0));
    cpu.ax = fpu.status.word;
    if (below(cpu))
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if ((at(app, cpu.eax + 0x10c) & 0x7fffffff) != 0)
        {
            fpu.push(f32(app, cpu.eax + 0xb8));
            fcomp(cpu, f64(app, 0x5372d8));
            cpu.ax = fpu.status.word;
            if (!below(cpu) && !equal(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 4);
                fpu.push(f32(app, cpu.eax + 0x10c));
                fcomp(cpu, f64(app, 0x5372f8));
                cpu.ax = fpu.status.word;
                if (below(cpu))
                {
                    push(app, cpu, 0);
                    cpu.eax = at(app, cpu.ebp - 4);
                    cpu.edi = at(app, cpu.ebp - 0x54);
                    push(app, cpu, at(app, cpu.eax + 0xb8));
                    cpu.ecx = 5;
                    push(app, cpu, cpu.edi);
                    cpu.ebx = 0xa;
                    cpu.edx = 4;
                    if (!byAddress(app, cpu, 0x4284e0))
                        return;
                    cpu.eax = at(app, cpu.ebp - 4);
                    putByte(app, cpu.eax + 0x996, 1);
                    goto l42b2fe;
                }
            }
            cpu.eax = at(app, cpu.ebp - 4);
            if (byte(app, cpu.eax + 0x996) != 0)
            {
                push(app, cpu, 0);
                push(app, cpu, 0);
                cpu.ecx = 5;
                push(app, cpu, 1);
                cpu.ebx = 0xa;
                cpu.edx = cpu.ecx;
                if (!byAddress(app, cpu, 0x4284e0))
                    return;
                cpu.eax = at(app, cpu.ebp - 4);
                putByte(app, cpu.eax + 0x996, 0);
            }
l42b2fe:
            cpu.eax = at(app, cpu.ebp - 4);
            fpu.push(f32(app, cpu.eax + 0x10c));
            fcomp(cpu, f64(app, 0x537300));
            cpu.ax = fpu.status.word;
            if (below(cpu))
            {
                cpu.eax = at(app, cpu.ebp - 4);
                if (i32(at(app, cpu.eax + 0xb8)) <= i32(0x40000000))
                    return leave();
                cpu.eax = cpu.ebp - 0x1c;
                push(app, cpu, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 4);
                cpu.ecx = cpu.ebp - 0x28;
                cpu.edx = cpu.eax + 0xcc;
                push(app, cpu, at(app, cpu.eax + 0x118));
                cpu.eax = 1;
                cpu.ebx = cpu.ebp - 0x1c;
                if (!byAddress(app, cpu, 0x4e0050))
                    return;
                cpu.edx = at(app, cpu.ebp - 4);
                cpu.eax = 1;
                cpu.edx += 0x98;
                if (!byAddress(app, cpu, 0x4dffb0))
                    return;
                cpu.ecx = at(app, cpu.ebp - 0x5c);
                if (cpu.ecx == 3)
                {
                    cpu.eax = at(app, cpu.ebp - 4);
                    cpu.ebx = at(app, cpu.ebp - 0x20);
                    push(app, cpu, at(app, cpu.eax + 0xb8));
                    push(app, cpu, cpu.ebx);
                    cpu.esi = at(app, cpu.ebp - 0x24);
                    push(app, cpu, cpu.esi);
                    cpu.edi = at(app, cpu.ebp - 0x28);
                    push(app, cpu, cpu.edi);
                    if (!byAddress(app, cpu, 0x4cac20))
                        return;
                    return leave();
                }
                cpu.eax = at(app, cpu.ebp - 0x20);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.ebp - 0x24);
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.ebp - 0x28);
                push(app, cpu, cpu.ecx);
                cpu.eax = at(app, cpu.ebp - 0x5c);
                if (!byAddress(app, cpu, 0x4cae00))
                    return;
                return leave();
            }
            cpu.eax = at(app, cpu.ebp - 4);
            if ((at(app, cpu.eax + 0x168) & 0x7fffffff) == 0)
                return leave();
            if (at(app, cpu.ebp - 8) != 0x30000)
                return leave();
            fpu.push(f32(app, cpu.eax + 0xb8));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0x168));
            app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
            fcomp(cpu, f32(app, 0x537308));
            cpu.ax = fpu.status.word;
            if (below(cpu) || equal(cpu))
                return leave();
            if (at(app, cpu.ebp - 0x5c) == 3)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                cpu.edx = at(app, cpu.eax + 0x180);
                push(app, cpu, at(app, cpu.ebp - 0x10));
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.eax + 0x17c);
                push(app, cpu, cpu.ecx);
                cpu.ebx = at(app, cpu.eax + 0x178);
                push(app, cpu, cpu.ebx);
                if (!byAddress(app, cpu, 0x4cac20))
                    return;
                return leave();
            }
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.esi = at(app, cpu.eax + 0x180);
            push(app, cpu, cpu.esi);
            cpu.edi = at(app, cpu.eax + 0x17c);
            push(app, cpu, cpu.edi);
            cpu.edx = at(app, cpu.eax + 0x178);
            push(app, cpu, cpu.edx);
            cpu.eax = at(app, cpu.ebp - 0x5c);
            if (!byAddress(app, cpu, 0x4cae00))
                return;
            return leave();
        }
    }
    cpu.eax = at(app, cpu.ebp - 4);
    if (byte(app, cpu.eax + 0x996) == 0)
        return leave();
    push(app, cpu, 0);
    push(app, cpu, 0);
    cpu.ecx = 5;
    push(app, cpu, 1);
    cpu.ebx = 0xa;
    cpu.edx = cpu.ecx;
    if (!byAddress(app, cpu, 0x4284e0))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    putByte(app, cpu.eax + 0x996, 0);
    return leave();
}

/* 0x42b480: car eax just put back (+0x8d4 counting up from 1): sparkle
 * where it is (0x4caea0 for 64 frames, 0x4cae00 9 then 6) unless 0x4396e0
 * says no of its driver; the count ended once it settles on the ground. */
void respawnEffects(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
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
    cpu.edx = at(app, cpu.eax + 0x8d4);
    if (i32(cpu.edx) > 0)
    {
        cpu.ebx = cpu.edx + 1;
        put(app, cpu.eax + 0x8d4, cpu.ebx);
    }
    if (at(app, cpu.ecx + 0x8d4) != 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x220);
        cpu.edx = 1;
        cpu.edi = at(app, cpu.eax + 0x14);
        if (i32(cpu.edi) >= 0 && i32(cpu.edi) <= i32(0x31))
        {
            cpu.eax = at(app, cpu.ecx + 0x220);
            cpu.eax = at(app, cpu.eax + 0x14);
            if (!byAddress(app, cpu, 0x4396e0))
                return;
            if (cpu.eax != 0)
                cpu.edx = 0;
        }
        if (cpu.edx != 0)
        {
            cpu.edi = cpu.ebp - 0x14;
            cpu.edx = cpu.ecx + 0xc0;
            cpu.esi = cpu.ecx + 0x8d8;
            cpu.ebx = cpu.ebp - 0x14;
            cpu.eax = cpu.ecx + 0x8d8;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx));
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 4));
            fpu.push(f32(app, cpu.eax));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 8));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x10));
            fpu.push(f32(app, cpu.eax + 4));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x14));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x18));
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x1c));
            fpu.push(f32(app, cpu.eax + 8));
            fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x20));
            std::swap(fpu.st(0), fpu.st(2));
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
            fpu.pop();
            cpu.eax = cpu.eax + 0xc;
            app->getMemory<float>(cpu.ebx) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.ebx + 4) = float(fpu.st(0));
            fpu.pop();
            app->getMemory<float>(cpu.ebx + 8) = float(fpu.st(0));
            fpu.pop();
            fpu.push(f32(app, cpu.ebp - 0x14));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ecx + 0x98));
            fpu.push(f32(app, cpu.ebp - 0x10));
            std::swap(fpu.st(0), fpu.st(1));
            app->getMemory<float>(cpu.ebp - 0x14) = float(fpu.st(0));
            fpu.pop();
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ecx + 0x9c));
            fpu.push(f32(app, cpu.ebp - 0xc));
            std::swap(fpu.st(0), fpu.st(1));
            app->getMemory<float>(cpu.ebp - 0x10) = float(fpu.st(0));
            fpu.pop();
            fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ecx + 0xa0));
            app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
            fpu.pop();
            cpu.esi = at(app, cpu.ecx + 0x8d4);
            if (i32(cpu.esi) > i32(0x40))
            {
                cpu.edi = at(app, cpu.ebp - 0xc);
                push(app, cpu, cpu.edi);
                cpu.eax = at(app, cpu.ebp - 0x10);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.ebp - 0x14);
                push(app, cpu, cpu.edx);
                cpu.eax = cpu.ecx;
                if (!byAddress(app, cpu, 0x4caea0))
                    return;
            }
            else
            {
                if (i32(cpu.esi) > i32(0x10))
                {
                    cpu.esi = at(app, cpu.ebp - 0xc);
                    push(app, cpu, cpu.esi);
                    cpu.edi = at(app, cpu.ebp - 0x10);
                    push(app, cpu, cpu.edi);
                    cpu.eax = at(app, cpu.ebp - 0x14);
                    push(app, cpu, cpu.eax);
                    cpu.eax = 9;
                }
                else
                {
                    cpu.edx = at(app, cpu.ebp - 0xc);
                    push(app, cpu, cpu.edx);
                    cpu.ebx = at(app, cpu.ebp - 0x10);
                    push(app, cpu, cpu.ebx);
                    cpu.esi = at(app, cpu.ebp - 0x14);
                    push(app, cpu, cpu.esi);
                    cpu.eax = 6;
                }
                if (!byAddress(app, cpu, 0x4cae00))
                    return;
            }
        }
    }
    if (at(app, cpu.ecx + 0x8d4) == 0)
        return leave();
    if (half(app, cpu.ecx + 0x164) > 0)
        return leave();
    if (i32(at(app, cpu.ecx + 0x10c)) < i32(0x3f000000))
        return leave();
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ecx + 0xe4));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ecx + 0xe4));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 8) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0xe4);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    fpu.push(f32(app, cpu.ebp - 8));
    fcomp(cpu, f64(app, 0x537310));
    cpu.ax = fpu.status.word;
    if (!below(cpu))
        return leave();
    fpu.push(0.0);
    fcomp(cpu, f32(app, cpu.ecx + 0xec));
    cpu.ax = fpu.status.word;
    if (!below(cpu) && !equal(cpu))
    {
        fpu.push(f32(app, cpu.ecx + 0xec));
        fpu.st(0) = -fpu.st(0);
        app->getMemory<float>(cpu.ebp - 4) = float(fpu.st(0));
        fpu.pop();
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0xec);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fpu.push(f32(app, cpu.ebp - 4));
    fcomp(cpu, f64(app, 0x537310));
    cpu.ax = fpu.status.word;
    if (!below(cpu))
        return leave();
    put(app, cpu.ecx + 0x8d4, 0);
    return leave();
}

/* 0x42b660: car eax's wheels' marks cut (0x4db860), its skid and scrape
 * sounds stopped, its shake cleared. */
void stopCarEffects(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.ecx = 0;
    do
    {
        cpu.edx = cpu.ecx;
        cpu.eax = at(app, cpu.esi);
        if (!byAddress(app, cpu, 0x4db860))
            return;
        cpu.eax = cpu.esi + cpu.ecx;
        putByte(app, cpu.eax + 0x95c, 0);
        ++cpu.ecx;
        putByte(app, cpu.eax + 0x990, 0);
    } while (i32(cpu.ecx) < i32(4));
    if (byte(app, cpu.esi + 0x994) != 0)
    {
        push(app, cpu, 0);
        cpu.ecx = 6;
        push(app, cpu, 0);
        cpu.ebx = 8;
        push(app, cpu, 1);
        cpu.edx = 3;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x994, 0);
    }
    if (byte(app, cpu.esi + 0x995) != 0)
    {
        push(app, cpu, 0);
        cpu.ecx = 6;
        push(app, cpu, 0);
        cpu.ebx = 0xa;
        push(app, cpu, 1);
        cpu.edx = 5;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x995, 0);
    }
    if (byte(app, cpu.esi + 0x996) != 0)
    {
        push(app, cpu, 0);
        cpu.ecx = 5;
        push(app, cpu, 0);
        cpu.ebx = 0xa;
        push(app, cpu, 1);
        cpu.eax = cpu.esi;
        cpu.edx = cpu.ecx;
        if (!byAddress(app, cpu, 0x4284e0))
            return;
        putByte(app, cpu.esi + 0x996, 0);
    }
    put(app, cpu.esi + 0x920, 0);
    put(app, cpu.esi + 0x924, 0);
    put(app, cpu.esi + 0x928, 0);
    put(app, cpu.esi + 0x92c, 0);
    put(app, cpu.esi + 0x930, 0);
    put(app, cpu.esi + 0x8c8, 0);
    put(app, cpu.esi + 0x8d0, 0);
    put(app, cpu.esi + 0x8c4, 0);
    put(app, cpu.esi + 0x8cc, 0);
    put(app, cpu.esi + 0x91c, 0);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x42b780: car eax's effects a frame: respawnEffects, crashEffects,
 * wheelEffects, updateShake. */
void carEffects(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, respawnEffects))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, crashEffects))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, wheelEffects))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, updateShake))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x42b7b0: car eax in the air ([+0x8b]), not to be put back, not crashing,
 * falling (its speed against its up, 0x4e01f0): its spin (+0xe4) as its
 * axes see it (+0x4f8.. ) taken by the air's tuning -- the player's steered
 * by +0x4e0, +0x4e8 -- and turned back into the world's. */
void airborneSpin(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    if (byte(app, cpu.eax + 0x8b) == 0)
        return leave();
    if (at(app, cpu.ecx + 0x184) != 0)
        return leave();
    if ((at(app, cpu.ecx + 0x168) & 0x7fffffff) != 0)
        return leave();
    cpu.edx = cpu.ecx + 0x130;
    cpu.eax = cpu.ecx + 0xa4;
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    fpu.push(0.0);
    fcompp(cpu);
    cpu.ax = fpu.status.word;
    if (!below(cpu))
        return leave();
    cpu.edx = cpu.ecx + 0xc0;
    cpu.ebx = cpu.ecx + 0xe4;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    cpu.edx = cpu.ecx + 0xcc;
    cpu.eax = cpu.ebx;
    app->getMemory<float>(cpu.ecx + 0x4f8) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    cpu.edx = cpu.ecx + 0xd8;
    cpu.eax = cpu.ebx;
    app->getMemory<float>(cpu.ecx + 0x4fc) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    fpu.push(f32(app, kAirSpinX));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x4f8));
    fpu.push(f32(app, kAirSpinZ));
    std::swap(fpu.st(0), fpu.st(2));
    app->getMemory<float>(cpu.ecx + 0x500) = float(fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x500));
    cpu.dl = byte(app, cpu.ecx + 0x200);
    std::swap(fpu.st(0), fpu.st(1));
    app->getMemory<float>(cpu.ecx + 0x4f8) = float(fpu.st(0));
    fpu.pop();
    app->getMemory<float>(cpu.ecx + 0x500) = float(fpu.st(0));
    fpu.pop();
    if ((cpu.dl & 4) != 0)
    {
        fpu.push(f32(app, cpu.ecx + 0x4e0));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAirSteerZ));
        fpu.push(f32(app, cpu.ecx + 0x4e8));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAirSteerX));
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::sub(f32(app, cpu.ecx + 0x500), fpu.st(0));
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ecx + 0x4f8));
        std::swap(fpu.st(0), fpu.st(1));
        app->getMemory<float>(cpu.ecx + 0x500) = float(fpu.st(0));
        fpu.pop();
        app->getMemory<float>(cpu.ecx + 0x4f8) = float(fpu.st(0));
        fpu.pop();
    }
    cpu.edx = cpu.ebp - 0x24;
    cpu.eax = cpu.ecx + 0xc0;
    if (!byAddress(app, cpu, 0x4e0110))
        return;
    cpu.ebx = cpu.ecx + 0x4f8;
    cpu.edx = cpu.ebp - 0x24;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    cpu.edx = cpu.ebp - 0x18;
    cpu.eax = cpu.ebx;
    app->getMemory<float>(cpu.ecx + 0xe4) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    cpu.edx = cpu.ebp - 0xc;
    cpu.eax = cpu.ebx;
    app->getMemory<float>(cpu.ecx + 0xe8) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4e01f0))
        return;
    app->getMemory<float>(cpu.ecx + 0xec) = float(fpu.st(0));
    fpu.pop();
    return leave();
}

/* 0x42b900: car eax's frame of race state: its reset's wait run down
 * (+0x524); while it is waiting to start ([+0x88] or not [+0x89]) put back
 * on its slice if stuck, its effects stopped, its crash cleared (0x20 bytes
 * at +0x168); else its wheels on the track (wheelCells), its crash's
 * response (0x406120, 0x404a00), its spin in the air, the objects, the stuck
 * count, its effects, and its crash cleared. */
void updateCar(App* app, CPU& cpu)
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
    cpu.ecx = at(app, cpu.eax + 0x524);
    put(app, cpu.eax + 0x7dc, 0);
    if (cpu.ecx != 0)
    {
        cpu.ebx = cpu.ecx - 1;
        put(app, cpu.eax + 0x524, cpu.ebx);
    }
    if (byte(app, cpu.edx + 0x88) == 0)
    {
        if (byte(app, cpu.edx + 0x89) != 0)
            goto l42b98a;
    }
    if (i32(at(app, cpu.edx + 0x8a0)) <= i32(0))
    {
        if (at(app, cpu.edx + 0x184) == 0)
            goto l42b95a;
    }
    cpu.eax = cpu.edx;
    put(app, cpu.edx + 0x8a0, 0);
    if (!direct(app, cpu, respawnCar))
        return;
l42b95a:
    cpu.eax = cpu.edx;
    put(app, cpu.edx + 0x8d4, 0);
    if (!direct(app, cpu, stopCarEffects))
        return;
    cpu.eax = cpu.edx + 0x168;
    put(app, cpu.edx + 0x524, 0);
    cpu.edx = 0x20;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    return leave();
l42b98a:
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, wheelCells))
        return;
    if ((at(app, cpu.edx + 0x168) & 0x7fffffff) != 0 && at(app, cpu.edx + 0x16c) != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x406120))
            return;
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x404a00))
            return;
    }
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, airborneSpin))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, objectContacts))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, stuckCheck))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, carEffects))
        return;
    put(app, cpu.edx + 0x168, 0);
    put(app, cpu.edx + 0x16c, 0);
    put(app, cpu.edx + 0x170, 0);
    return leave();
}

/* 0x42ba00: grid place eax worked out: its slice to [edx] -- 3 a row from
 * the start, back from the last when the race runs backwards -- and its
 * place to [ebx]: across by the slice's half widths (left or right by the
 * place's side, swapped backwards; 0x40eca0 says whether the road is wide
 * enough for two), 0x8000 up, 0 ahead. */
void defaultGridPlace(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edi = cpu.eax;
    cpu.ecx = cpu.edx;
    if (at(app, kBackwards) != 0)
    {
        cpu.edx = at(app, kGridCars);
        cpu.edx += at(app, kGridOthers);
        --cpu.edx;
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
    }
    else
    {
        cpu.eax <<= 2;
        cpu.eax -= cpu.edi;
    }
    ++cpu.eax;
    put(app, cpu.ecx, cpu.eax);
    cpu.eax = at(app, kBackwards);
    cpu.edi &= 1;
    if (cpu.eax != 0)
    {
        if (cpu.edi == 0)
        {
            cpu.edi = 1;
        }
        else
        {
            cpu.edi = 0;
        }
    }
    if (!byAddress(app, cpu, 0x40eca0))
        return;
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.ecx);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kSlices);
        cpu.eax <<= 2;
        cpu.esi = cpu.edx + cpu.eax;
        cpu.al = byte(app, cpu.esi + 0x13);
        cpu.edx = 0;
        cpu.al &= 0xf;
        cpu.dl = cpu.al;
        cpu.eax = cpu.edx;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(i32(cpu.eax) >> 1);
        cpu.esi = byte(app, cpu.esi + 0x1b);
        cpu.esi <<= 0xf;
        cpu.eax *= cpu.esi;
        cpu.edx = cpu.esi;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = cpu.esi;
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(i32(cpu.eax) >> 1);
        if (cpu.edi != 0)
            cpu.eax += cpu.esi;
        put(app, cpu.ebx, cpu.eax);
        if (cpu.edi != 0)
        {
            cpu.edx = at(app, cpu.ecx);
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, kSlices);
            cpu.eax <<= 2;
            cpu.eax += cpu.edx;
            cpu.edx = 0;
            cpu.dl = byte(app, cpu.eax + 0x1b);
            cpu.edx <<= 0xf;
            cpu.eax = cpu.edx;
            cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(i32(cpu.eax) >> 1);
            cpu.eax += at(app, cpu.ebp - 4);
        }
        else
        {
            cpu.edx = at(app, cpu.ecx);
            cpu.eax = cpu.edx * 8;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, kSlices);
            cpu.edx <<= 2;
            cpu.eax += cpu.edx;
            cpu.edx = 0;
            cpu.dl = byte(app, cpu.eax + 0x1b);
            cpu.edx <<= 0xf;
            cpu.eax = cpu.edx;
            cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(i32(cpu.eax) >> 1);
            cpu.edx = at(app, cpu.ebp - 4);
            cpu.edx -= cpu.eax;
            cpu.eax = cpu.edx;
        }
    }
    else
    {
        if (cpu.edi != 0)
        {
            cpu.edx = at(app, cpu.ecx);
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, kSlices);
            cpu.eax <<= 2;
            cpu.eax += cpu.edx;
            cpu.edx = 0;
            cpu.dl = byte(app, cpu.eax + 0x1b);
            cpu.edx <<= 0xf;
        }
        else
        {
            cpu.edx = at(app, cpu.ecx);
            cpu.eax = cpu.edx * 8;
            cpu.edx += cpu.eax;
            cpu.eax = at(app, kSlices);
            cpu.edx <<= 2;
            cpu.al = byte(app, cpu.edx + cpu.eax + 0x17);
            cpu.edx = cpu.eax;
            cpu.edx &= 0xff;
            cpu.edx <<= 0xf;
            cpu.edx = 0u - cpu.edx;
        }
        cpu.eax = cpu.edx;
        cpu.edx = reg32(i32(cpu.edx) >> 0x1f);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(i32(cpu.eax) >> 1);
    }
    put(app, cpu.ebx, cpu.eax);
    put(app, cpu.ebx + 8, 0);
    put(app, cpu.ebx + 4, 0x8000);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

/* 0x42bb80: the start's grid, 12 places of 3 words at kGrid: from the
 * track's start.dat (kTrackFolder + "TRK%03x\start.dat" of kTrack, read a
 * word at a time, 0x4dc0e0, 0x4dc1e0), else each worked out
 * (defaultGridPlace). */
void loadStartGrid(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x210;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = at(app, kTrack);
    push(app, cpu, cpu.edx);
    push(app, cpu, kTrackFolder);
    push(app, cpu, kStartFile);
    cpu.eax = cpu.ebp - 0x210;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, 0x4df690))
        return;
    cpu.esp += 0x10;
    cpu.eax = cpu.ebp - 0x210;
    if (!byAddress(app, cpu, 0x4e0e60))
        return;
    if (cpu.eax != 0)
    {
        cpu.ebx = 1;
        cpu.eax = cpu.ebp - 0x210;
        cpu.edx = 0;
        if (!byAddress(app, cpu, 0x4dc0e0))
            return;
        cpu.ecx = 0;
        cpu.edx = cpu.eax;
        // Each place's three words, read from the file.
        do
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, 0x4dc1e0))
                return;
            cpu.esi = cpu.eax;
            cpu.eax = cpu.ecx * 4;
            cpu.eax -= cpu.ecx;
            cpu.ebx = cpu.eax * 4;
            cpu.eax = cpu.edx;
            put(app, cpu.ebx + kGrid, cpu.esi);
            if (!byAddress(app, cpu, 0x4dc1e0))
                return;
            put(app, cpu.ebx + kGridY, cpu.eax);
            cpu.eax = cpu.edx;
            ++cpu.ecx;
            if (!byAddress(app, cpu, 0x4dc1e0))
                return;
            put(app, cpu.ebx + kGridZ, cpu.eax);
        } while (i32(cpu.ecx) < i32(0xc));
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x4dc1b0))
            return;
        return leave();
    }
    // No file: each place worked out.
    cpu.ecx = 0;
    do
    {
        cpu.ebx = cpu.ebp - 0x10;
        cpu.edx = cpu.ebp - 4;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, defaultGridPlace))
            return;
        cpu.eax = cpu.ecx * 4;
        cpu.eax -= cpu.ecx;
        cpu.edx = cpu.eax * 4;
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.edx + kGrid, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x10);
        put(app, cpu.edx + kGridY, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 8);
        ++cpu.ecx;
        put(app, cpu.edx + kGridZ, cpu.eax);
    } while (i32(cpu.ecx) < i32(0xc));
    return leave();
}

/* 0x42bc70: grid place eax: its slice to [edx], its place to [ebx] (0x8000
 * up), across mirrored when kGridMirrored is 1. */
void gridPlace(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.ecx;
    cpu.ecx = at(app, cpu.eax * 4 + kGrid);
    put(app, cpu.edx, cpu.ecx);
    cpu.edx = at(app, cpu.eax * 4 + kGridY);
    put(app, cpu.ebx, cpu.edx);
    cpu.eax = at(app, cpu.eax * 4 + kGridZ);
    put(app, cpu.ebx + 4, 0x8000);
    cpu.edx = at(app, kGridMirrored);
    put(app, cpu.ebx + 8, cpu.eax);
    if (cpu.edx != 1)
        return leave();
    put(app, cpu.ebx, 0u - at(app, cpu.ebx));
    return leave();
}

/* 0x42bcb0: car eax put on its grid place and its race state set to the
 * start's: its handling scaled for the players (0x4a0d20), its place (a
 * wreck's or a player's last, kGridSlots' else, gridPlace, 0x404d30), put
 * there (0x4a0aa0), its slice, every count, flag and effect cleared (fills
 * of 0x4e070c too), the stuck count's record kept, its lights on at night
 * (+0x660). */
void placeOnGrid(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2c;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.bl = byte(app, cpu.eax + 0x200);
    if ((cpu.bl & 4) != 0)
    {
        cpu.ecx = 0x3f800000;
        cpu.ebx = at(app, kPlayerOption);
        put(app, cpu.ebp - 8, cpu.ecx);
        if (cpu.ebx != 0)
        {
            cpu.eax = at(app, kPlayerScale);
            put(app, cpu.ebp - 8, cpu.eax);
        }
        if (at(app, kPlayMode) == 0)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            if ((byte(app, cpu.eax + 0x200) & 0x20) != 0)
            {
                cpu.eax = at(app, kPlayerScale);
                put(app, cpu.ebp - 8, cpu.eax);
            }
        }
        if ((byte(app, kLoadingFlags) & 2) != 0)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.esi = at(app, cpu.eax + 0x8a4);
            cpu.eax = at(app, cpu.eax + 0x220);
            cpu.esi = cpu.esi + 0x28;
            cpu.edi = cpu.eax + 0x5c;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.esi += 4;
            cpu.edi += 4;
        }
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax = at(app, cpu.eax + 0x220);
        push(app, cpu, at(app, cpu.eax + 0x64));
        push(app, cpu, at(app, cpu.eax + 0x60));
        push(app, cpu, at(app, cpu.eax + 0x5c));
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    }
    else
    {
        put(app, cpu.ebp - 0xc, 0x3f800000);
        if ((cpu.bl & 1) != 0)
        {
            fpu.push(f32(app, kHumanScale));
            fpu.push(1.0);
            fpu.st(1) = fp::div(fpu.st(0), fpu.st(1));
            fpu.pop();
            app->getMemory<float>(cpu.ebp - 0xc) = float(fpu.st(0));
            fpu.pop();
        }
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax = at(app, cpu.eax + 0x8a4);
        push(app, cpu, at(app, cpu.eax + 0x30));
        push(app, cpu, at(app, cpu.eax + 0x2c));
        push(app, cpu, at(app, cpu.eax + 0x28));
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.eax = at(app, cpu.eax + 0x528);
        fpu.push(f32(app, cpu.eax + 4));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    }
    cpu.esp -= 4;
    cpu.eax = at(app, cpu.ebp - 4);
    app->getMemory<float>(cpu.esp) = float(fpu.st(0));
    fpu.pop();
    if (!byAddress(app, cpu, 0x4a0d20))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.dl = byte(app, cpu.eax + 0x200);
    if ((cpu.dl & 4) != 0 && (cpu.dl & 0x20) != 0)
    {
        cpu.ebx = cpu.ebp - 0x2c;
        cpu.edx = cpu.ebp - 0x14;
        if (!byAddress(app, cpu, 0x404d30))
            return;
        goto l42be04;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.bl = byte(app, cpu.eax + 0x200);
    if ((cpu.bl & 0x10) != 0)
    {
        cpu.edx = 0;
        cpu.eax = 0x64;
        put(app, cpu.ebp - 0x2c, cpu.edx);
        put(app, cpu.ebp - 0x28, cpu.edx);
        put(app, cpu.ebp - 0x24, cpu.edx);
        put(app, cpu.ebp - 0x14, cpu.eax);
    }
    else if ((cpu.bl & 0x20) != 0)
    {
        cpu.edi = 0;
        cpu.esi = 0x64;
        put(app, cpu.ebp - 0x2c, cpu.edi);
        put(app, cpu.ebp - 0x28, cpu.edi);
        put(app, cpu.ebp - 0x24, cpu.edi);
        put(app, cpu.ebp - 0x14, cpu.esi);
    }
    else
    {
        cpu.eax = at(app, cpu.eax + 0x1f8);
        cpu.ebx = cpu.ebp - 0x2c;
        cpu.edx = cpu.ebp - 0x14;
        cpu.eax = at(app, cpu.eax * 4 + kGridSlots);
        if (!direct(app, cpu, gridPlace))
            return;
    }
l42be04:
    cpu.ebx = cpu.ebp - 0x2c;
    cpu.edx = at(app, cpu.ebp - 0x14);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.ecx = at(app, kBackwards);
    if (!byAddress(app, cpu, 0x4a0aa0))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax + 0x204, 0);
    cpu.ecx = at(app, kBackwards);
    if (cpu.ecx != 0)
    {
        put(app, cpu.eax + 0x208, 1);
    }
    else
    {
        put(app, cpu.eax + 0x208, cpu.ecx);
    }
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.eax + 0x1c);
    put(app, cpu.edx + 4, cpu.eax);
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, 0x42ce10))
        return;
    put(app, cpu.edx + 0x20c, 0x3f800000);
    put(app, cpu.edx + 0x210, 0);
    put(app, cpu.edx + 0x214, 0);
    put(app, cpu.edx + 0x2e4, 0);
    put(app, cpu.edx + 0x218, 0x100);
    put(app, cpu.edx + 0x998, 0);
    put(app, cpu.edx + 0x1fc, 0);
    put(app, cpu.edx + 0x5f4, 0);
    put(app, cpu.edx + 0x2e8, 0);
    put(app, cpu.edx + 0x2ec, 0);
    put(app, cpu.edx + 0x2f0, 0);
    put(app, cpu.edx + 0x2f4, 0);
    put(app, cpu.edx + 0x2f8, 0);
    put(app, cpu.edx + 0x2fc, 0);
    put(app, cpu.edx + 0x4e0, 0);
    put(app, cpu.edx + 0x4e4, 0);
    put(app, cpu.edx + 0x4e8, 0);
    put(app, cpu.edx + 0x4ec, 0);
    put(app, cpu.edx + 0x4f0, 0);
    put(app, cpu.edx + 0x4f4, 0);
    put(app, cpu.edx + 0x4f8, 0);
    put(app, cpu.edx + 0x4fc, 0);
    put(app, cpu.edx + 0x500, 0);
    put(app, cpu.edx + 0x590, 0);
    put(app, cpu.edx + 0x594, 0);
    put(app, cpu.edx + 0x598, 0);
    put(app, cpu.edx + 0x59c, 0);
    put(app, cpu.edx + 0x5a0, 0);
    put(app, cpu.edx + 0x5a4, 0);
    put(app, cpu.edx + 0x5a8, 0);
    put(app, cpu.edx + 0x5ac, 0);
    put(app, cpu.edx + 0x5b0, 0);
    putByte(app, cpu.edx + 0x5b4, 0);
    put(app, cpu.edx + 0x5b8, 0);
    put(app, cpu.edx + 0x5bc, 0);
    put(app, cpu.edx + 0x5c0, 0);
    put(app, cpu.edx + 0x5c8, 0);
    put(app, cpu.edx + 0x5cc, 0);
    put(app, cpu.edx + 0x5d4, 0);
    put(app, cpu.edx + 0x5d0, 0);
    put(app, cpu.edx + 0x5d8, 0);
    put(app, cpu.edx + 0x5dc, 0);
    put(app, cpu.edx + 0x5e4, 0);
    put(app, cpu.edx + 0x5e0, 0);
    put(app, cpu.edx + 0x5e8, 0);
    put(app, cpu.edx + 0x5ec, 0);
    put(app, cpu.edx + 0x5f0, 0);
    put(app, cpu.edx + 0x7c0, 0);
    put(app, cpu.edx + 0x7c4, 0);
    put(app, cpu.edx + 0x524, 0);
    putByte(app, cpu.edx + 0x504, 0);
    putByte(app, cpu.edx + 0x505, 0);
    putByte(app, cpu.edx + 0x506, 1);
    put(app, cpu.edx + 0x518, 0);
    putByte(app, cpu.edx + 0x507, 0);
    putByte(app, cpu.edx + 0x508, 0);
    putByte(app, cpu.edx + 0x509, 0);
    put(app, cpu.edx + 0x51c, 0);
    putByte(app, cpu.edx + 0x50b, 1);
    putByte(app, cpu.edx + 0x50a, 1);
    putByte(app, cpu.edx + 0x50d, 0);
    put(app, cpu.edx + 0x514, 0);
    putByte(app, cpu.edx + 0x50e, 0);
    putByte(app, cpu.edx + 0x50f, 0);
    put(app, cpu.edx + 0x520, 0x100);
    putByte(app, cpu.edx + 0x95c, 0);
    putByte(app, cpu.edx + 0x95d, 0);
    putByte(app, cpu.edx + 0x95e, 0);
    putByte(app, cpu.edx + 0x95f, 0);
    putByte(app, cpu.edx + 0x994, 0);
    putByte(app, cpu.edx + 0x995, 0);
    putByte(app, cpu.edx + 0x997, 0);
    put(app, cpu.edx + 0x94c, 0);
    put(app, cpu.edx + 0x5c4, 0);
    put(app, cpu.edx + 0x950, 0);
    put(app, cpu.edx + 0x954, 0);
    put(app, cpu.edx + 0x958, 0);
    putByte(app, cpu.edx + 0x990, 0);
    putByte(app, cpu.edx + 0x991, 0);
    putByte(app, cpu.edx + 0x992, 0);
    putByte(app, cpu.edx + 0x993, 0);
    putByte(app, cpu.edx + 0x5b4, 0);
    cpu.eax = cpu.edx + 0x62c;
    put(app, cpu.edx + 0x5b8, 0);
    cpu.edx = 4;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = 0xac;
    cpu.eax += 0x630;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = 4;
    cpu.eax += 0x6dc;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = 0x2c;
    cpu.eax += 0x6e0;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = 0xa4;
    cpu.eax += 0x70c;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    if (!byAddress(app, cpu, 0x4a33d0))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax += 0x7e0;
    cpu.edx = 0x18;
    put(app, cpu.eax - 4, 0);
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.ecx = 0xc;
    put(app, cpu.eax + 0x7d8, 0);
    cpu.edi = cpu.ebp - 0x20;
    put(app, cpu.eax + 0x8a0, 0);
    cpu.ebx += 0x8a8;
    cpu.eax = at(app, cpu.eax + 0x8a4);
    cpu.esi = cpu.ebx;
    put(app, cpu.ebp - 0x10, cpu.eax);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.ecx >>= 2;
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
    cpu.cl = cpu.al;
    cpu.cl &= 3;
    for (; cpu.ecx != 0; --cpu.ecx)
        putByte(app, cpu.edi++, byte(app, cpu.esi++));
    cpu.edi = pop(app, cpu);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = 0x104;
    cpu.eax += 0x8a4;
    if (!byAddress(app, cpu, 0x4e070c))
        return;
    cpu.ecx = 0xc;
    cpu.esi = cpu.ebp - 0x20;
    cpu.edi = cpu.ebx;
    cpu.edx = at(app, cpu.ebp - 4);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.ecx >>= 2;
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
    cpu.cl = cpu.al;
    cpu.cl &= 3;
    for (; cpu.ecx != 0; --cpu.ecx)
        putByte(app, cpu.edi++, byte(app, cpu.esi++));
    cpu.edi = pop(app, cpu);
    cpu.eax = at(app, cpu.ebp - 0x10);
    put(app, cpu.edx + 0x660, 0);
    put(app, cpu.edx + 0x8a4, cpu.eax);
    cpu.ebx = at(app, kLightsAlways);
    if (cpu.ebx != 0)
        put(app, cpu.edx + 0x660, 1);
    if (at(app, kNight) != 0)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if ((byte(app, cpu.eax + 0x200) & 4) != 0)
        {
            cpu.eax = 2;
        }
        else
        {
            cpu.eax = 1;
        }
        cpu.edx = at(app, cpu.ebp - 4);
        put(app, cpu.edx + 0x660, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax + 0x734, 0);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
    return;
}

}  // namespace racestart
}  // namespace nfs3hp
