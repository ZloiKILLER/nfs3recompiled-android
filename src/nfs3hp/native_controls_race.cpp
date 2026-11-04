#include "native_guest.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <utility>

/* The controls read in a race, and kept in config.dat: the force-feedback
 * controller a race plays its forces on -- the four effects the game makes on
 * it, a jolt, the engine's rumble, the road's rumble, the steering's spring --
 * what each frame of a race plays on them from the player's car, and the
 * controllers found compared with config.dat's.  disassemble_nfs3hp.py binds
 * each address to the function of that name here (CONTROLS_RACE_NATIVES),
 * and the generated code calls it by name.
 *
 * Written as their code is, as native_frontend_text.cpp's: each push and pop
 * to the guest's stack, each call with the return address's slot, the
 * registers as the code leaves them, the x87's stack pushed and popped a
 * value at a time.  What they call of DirectInput's layer (the effects
 * created, started, stopped, their parameters set; the devices' gain and
 * actuators), of the device sets and of the rest of the game they call by
 * address.  tools/game_checks holds each against its generated code. */
namespace nfs3hp
{
void fillZero(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e070c, native_game.cpp

namespace watcom
{
void CHP(win32::WinApplication* app, x86::CPU& cpu);      // __CHP
void strstr(win32::WinApplication* app, x86::CPU& cpu);   // 0x4e0820
void strncmp(win32::WinApplication* app, x86::CPU& cpu);  // 0x4eb390
}

namespace controls
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg32;
using x86::sreg32;

// The race's force feedback: the controller's number, or -1; the car it is
// played from; a bump or a jump under way; then each controller's four
// effects' handles, words, 8 bytes a controller -- the jolt, the engine's
// rumble, the road's rumble, the steering's spring.  Read as dwords from two
// bytes before each and shifted down, as the code reads them.
constexpr reg32 kForceController = 0x678b50;
constexpr reg32 kForceCar = 0x678b54;
constexpr reg32 kBumped = 0x678b58;
constexpr reg32 kAirborne = 0x678b5c;
constexpr reg32 kEffects = 0x678b60;
constexpr reg32 kJoltHandle = 0x678b5e, kEngineHandle = 0x678b60, kRoadHandle = 0x678b62, kSpringHandle = 0x678b64;
constexpr reg32 kInputStateBytes = 0xac;
constexpr reg32 kEnginePlaying = 0x678be0, kEnginePeriod = 0x678be4, kEngineMagnitude = 0x678be8;
constexpr reg32 kGearKick = 0x678bec;        // frames the gear change's kick has left
constexpr reg32 kSpringPlaying = 0x678bf0;
constexpr reg32 kSurface = 0x678bf4;         // what the car is on
constexpr reg32 kNextJolt = 0x678bf8;        // the frame a jolt may come again
// Controllers: 0x88 bytes each from 0x6fc970 -- flags (2 a wheel, 8 force
// feedback), the name at 4, the gain at 0x44 -- how many, how many have force
// feedback; each force's strength, 0 off.
constexpr reg32 kControllers = 0x6fc970;
constexpr reg32 kControllerName = 0x6fc974;
constexpr reg32 kControllerGain = 0x6fc9b4;
constexpr reg32 kControllerBytes = 0x88;
constexpr reg32 kControllerCount = 0x6fd204;
constexpr reg32 kForceControllers = 0x6fd208;
constexpr reg32 kJoltStrength = 0x6fd1f0, kEngineStrength = 0x6fd1f4, kRoadStrength = 0x6fd1f8;
constexpr reg32 kSpringStrength = 0x6fd1fc, kSlideStrength = 0x6fd200;
constexpr reg32 kMicrosoft = 0x53aefc;       // "Microsoft": its devices' effects are played otherwise
// The race.
constexpr reg32 kReplay = 0x6fd3a0;          // nonzero: no forces
constexpr reg32 kGameMode = 0x6fd3b0;        // 1: two players on one screen
constexpr reg32 kPlayer = 0x6fd4f8;
constexpr reg32 kPlayerCars = 0x5efa48;
constexpr reg32 kFrame = 0x5efe00;
constexpr reg32 kSteerScale = 0x5efde0, kSpringScale = 0x5efde4;
constexpr reg32 kClock = 0x5efda4, kStartClock = 0x6fd4f4;
constexpr reg32 kLastGear = 0x55b02c;        // 0x64: none yet
constexpr reg32 kJoltWait = 0x55b030;
constexpr reg32 kViewMode = 0x7cd94c;
constexpr reg32 kRoadCamber = 0x79c4a5;
constexpr reg32 kTimers = 0x7d3690;
// DirectInput's layer and the device sets (another part's, by address).
constexpr reg32 kControllersFound = 0x435600, kControllerFound = 0x435690, kSetAxisForce = 0x435b40;
constexpr reg32 kActuatorsOn = 0x435d20, kActuatorsOff = 0x435d90, kSetGain = 0x435df0;
constexpr reg32 kCreateEffect = 0x435f10, kStartEffect = 0x4361e0, kStopEffect = 0x436230;
constexpr reg32 kUnloadEffect = 0x436310, kSetEffect = 0x436330;
constexpr reg32 kDeviceSet = 0x4904d0, kRebuildDeviceSet = 0x4904f0;
constexpr reg32 kAllDevices = 0x10;
// The rest of the game.
constexpr reg32 kSubtractVectors = 0x4e0000, kTranspose = 0x4e0110, kAtan2 = 0x4ea9c0;
constexpr reg32 kSurfaceUnder = 0x420840, kRunTimers = 0x4e7630, kAddTimer = 0x4be250;
constexpr reg32 kControllersChanged = 0x43c040;
// Their constants.
constexpr reg32 kTwo = 0x53af0c;             // double 2: a turn, in half turns
constexpr reg32 kTwoF = 0x53af14, kHalfTurnDegrees = 0x53af18;  // float 2, float 180
constexpr reg32 kImpactScale = 0x53af1c;     // double 70
constexpr reg32 kEighth = 0x53af24, kSteerRange = 0x53af2c, kTenThousandF = 0x53af30;
constexpr reg32 kMinusTenThousand = 0x53af34, kSpringEighth = 0x53af3c, kSpringRange = 0x53af44;
constexpr reg32 kSpringMinus = 0x53af4c;
constexpr reg32 kMillion = 0x53af54, kHalf = 0x53af5c;
constexpr reg32 kSpeeds = 0x53af64;          // floats: 2, 5, 10, 15, 20, 30, 40, 60, 80, 100, 140, 170, 200
constexpr reg32 kEngineMillion = 0x53af9c, kEngineHalf = 0x53afa4;
constexpr reg32 kToMph = 0x53afac;           // float 2.24
constexpr reg32 kPlusTenThousand = 0x461c4000, kMinusTenThousandBits = 0xc61c4000, kTwoBits = 0x40000000;

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

x86::reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<x86::reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, x86::reg16 value)
{
    app->getMemory<x86::reg16>(address) = value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

x86::Float loadFloat(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

x86::Float loadDouble(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

void storeFloat(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* fistp: st(0) as the control word rounds it, popped. */
void storeInteger(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    put(app, address, reg32(fp::toInteger<x86::sreg32>(cpu.fpu.st(0))));
    cpu.fpu.count -= 1;
}

void exchange(x86::CPU& cpu, int i)
{
    std::swap(cpu.fpu.st(0), cpu.fpu.st(i));
}

/* fcom's compare of st(0) with `with`, fnstsw ax, sahf. */
void compareTop(x86::CPU& cpu, const x86::Float& with)
{
    cpu.fpu.compare(cpu.fpu.st(0), with);
    cpu.ax = cpu.fpu.status.word;
    cpu.flags.lo = 0x02 | (cpu.ah & 0xd7);
}

/* The same, fcomp: popped after. */
void comparePop(x86::CPU& cpu, const x86::Float& with)
{
    compareTop(cpu, with);
    cpu.fpu.count -= 1;
}

bool belowOrEqual(const x86::CPU& cpu)
{
    return cpu.flags.cf || cpu.flags.zf;
}

/* push ebp, mov ebp, esp after the registers named, and sub esp, `locals`;
 * leave it the other way round, ret `popped`. */
void enter(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 locals)
{
    for (reg32* r : saved)
        push(app, cpu, *r);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= locals;
}

void leave(win32::WinApplication* app, x86::CPU& cpu, std::initializer_list<reg32*> saved, reg32 popped = 0)
{
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    for (auto r = std::rbegin(saved); r != std::rend(saved); ++r)
        **r = pop(app, cpu);
    cpu.esp += 4 + popped;
}

/* mov eax, [index*8 + slot]; sar eax, 0x10: the force controller's effect's
 * handle. */
reg32 handle(win32::WinApplication* app, reg32 index, reg32 slot)
{
    return reg32(sreg32(at(app, index * 8 + slot)) >> 16);
}

/* The force controller's name, at eax, and "Microsoft" at edx, for strstr. */
void nameAndMicrosoft(win32::WinApplication* app, x86::CPU& cpu, reg32 index)
{
    cpu.eax = index * kControllerBytes + kControllerName;
    cpu.edx = kMicrosoft;
}

/* Signed division of edx:eax by `divisor`, as idiv. */
void idiv(x86::CPU& cpu, reg32 divisor)
{
    const std::int64_t dividend = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
    cpu.eax = reg32(dividend / sreg32(divisor));
    cpu.edx = reg32(dividend % sreg32(divisor));
}

}  // namespace

void setForceGain(win32::WinApplication* app, x86::CPU& cpu);
void findForceController(win32::WinApplication* app, x86::CPU& cpu);
void stopRaceEffects(win32::WinApplication* app, x86::CPU& cpu);
void isForceCar(win32::WinApplication* app, x86::CPU& cpu);
void playJolt(win32::WinApplication* app, x86::CPU& cpu);
void stopEngineRumble(win32::WinApplication* app, x86::CPU& cpu);
void stopSteeringSpring(win32::WinApplication* app, x86::CPU& cpu);
void carSpeedMph(win32::WinApplication* app, x86::CPU& cpu);
void steeringAmount(win32::WinApplication* app, x86::CPU& cpu);
void playEngineRumble(win32::WinApplication* app, x86::CPU& cpu);
void playSteeringSpring(win32::WinApplication* app, x86::CPU& cpu);
void playSlideSpring(win32::WinApplication* app, x86::CPU& cpu);
void playRoadRumble(win32::WinApplication* app, x86::CPU& cpu);
void createForceEffects(win32::WinApplication* app, x86::CPU& cpu);

/* 0x470bc0: the engine's rumble set to a steady one -- 5000 for 0x72e4,
 * every axis -- on the force controller, if there is one; a Microsoft
 * device's with its duration too. */
void steadyEngineRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi}, 0x20);
    const reg32 ebp = cpu.ebp;
    cpu.ecx = 0xffffff;
    cpu.ebx = at(app, kForceController);
    cpu.edx = 0;
    put(app, ebp - 8, cpu.ecx);
    put(app, ebp - 4, cpu.edx);
    if (cpu.ebx != 0xffffffff)
    {
        cpu.esi = 0x1388;
        put(app, ebp - 0x1c, cpu.edx);
        put(app, ebp - 0x18, cpu.edx);
        cpu.edx = 0x72e4;
        put(app, ebp - 0x14, cpu.edx);
        nameAndMicrosoft(app, cpu, cpu.ebx);
        put(app, ebp - 0x20, cpu.esi);
        if (!direct(app, cpu, watcom::strstr))
            return;
        cpu.edx = cpu.eax != 0 ? 3 : 1;
        push(app, cpu, 0);
        push(app, cpu, ebp - 4);
        push(app, cpu, ebp - 0x20);
        cpu.ecx = ebp - 8;
        cpu.ebx = 0;
        push(app, cpu, 0);
        cpu.eax = handle(app, at(app, kForceController), kEngineHandle);
        if (!byAddress(app, cpu, kSetEffect))
            return;
    }
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi});
}

/* 0x470c60: the force controller's gain set to eax and, but on a Microsoft
 * device, the engine's rumble started. */
void startEngineRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kForceController) != 0xffffffff)
    {
        if (!direct(app, cpu, setForceGain))
            return;
        cpu.edx = at(app, kForceController);
        nameAndMicrosoft(app, cpu, cpu.edx);
        if (!direct(app, cpu, watcom::strstr))
            return;
        if (cpu.eax == 0)
        {
            cpu.eax = handle(app, at(app, kForceController), kEngineHandle);
            if (!byAddress(app, cpu, kStartEffect))
                return;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x470cb0: the engine's rumble unloaded from the force controller, and its
 * parameters set to nothing. */
void silenceEngineRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx}, 4);
    const reg32 ebp = cpu.ebp;
    cpu.edx = 0xffffffff;
    cpu.ecx = at(app, kForceController);
    put(app, ebp - 4, cpu.edx);
    if (cpu.ecx != cpu.edx)
    {
        cpu.eax = handle(app, cpu.ecx, kEngineHandle);
        if (!byAddress(app, cpu, kUnloadEffect))
            return;
        push(app, cpu, 0);
        cpu.ebx = 0;
        push(app, cpu, ebp - 4);
        cpu.edx = 0;
        push(app, cpu, 0);
        cpu.ecx = 0;
        push(app, cpu, 0);
        cpu.eax = handle(app, at(app, kForceController), kEngineHandle);
        if (!byAddress(app, cpu, kSetEffect))
            return;
    }
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx});
}

/* 0x470d10: every force-feedback device's gain set to eax, if a race has a
 * force controller. */
void setForceGain(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kForceController) != 0xffffffff)
    {
        cpu.edx = cpu.eax;
        cpu.eax = kAllDevices;
        if (!byAddress(app, cpu, kSetGain))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x470d30: the force feedback's state cleared -- no force controller, every
 * effect's handle -1 -- then the first controller with force feedback made
 * the force controller, if any has: 1, else 0. */
void findForceController(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = kInputStateBytes;
    cpu.eax = kForceController;
    if (!direct(app, cpu, fillZero))
        return;
    cpu.edx = 0xffffffff;
    cpu.eax = 0;
    put(app, kForceController, cpu.edx);
    for (;;)
    {
        for (cpu.edx = 0; x86::sreg32(cpu.edx) < 4;)
        {
            cpu.ecx = cpu.edx + cpu.edx;
            cpu.ebx = 0xffffffff;
            ++cpu.edx;
            putHalf(app, cpu.ecx + cpu.eax * 8 + kEffects, cpu.bx);
        }
        ++cpu.eax;
        if (x86::sreg32(cpu.eax) >= 0x10)
            break;
    }
    if (at(app, kForceControllers) != 0)
        for (cpu.edx = 0; x86::sreg32(cpu.edx) < x86::sreg32(at(app, kControllerCount)); ++cpu.edx)
        {
            cpu.eax = (cpu.edx << 4) + cpu.edx;
            if (byte(app, cpu.eax * 8 + kControllers) & 8)
            {
                cpu.eax = 1;
                put(app, kForceController, cpu.edx);
                return done();
            }
        }
    cpu.eax = 0;
    done();
}

/* 0x470dc0: the force controller found and its four effects created -- the
 * jolt, the engine's rumble, the road's rumble, the steering's spring; a
 * wheel's at 90 degrees -- and its gain set as the controller has it. */
void createForceEffects(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x5c);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}); };
    if (!direct(app, cpu, findForceController))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.edx = 0;
    put(app, ebp - 0x18, cpu.edx);
    put(app, ebp - 0x5c, cpu.edx);
    cpu.edx = 0xffffffff;
    cpu.ecx = 0x2710;
    put(app, ebp - 0x1c, cpu.edx);
    cpu.edx = at(app, kForceController);
    cpu.ebx = 0xc8;
    cpu.edi = 2;
    put(app, ebp - 0x58, cpu.ecx);
    cpu.eax = (cpu.edx << 4) + cpu.edx;
    put(app, ebp - 0x28, cpu.ebx);
    put(app, ebp - 0x24, cpu.ecx);
    cpu.dl = byte(app, cpu.eax * 8 + kControllers);
    put(app, ebp - 0x20, cpu.edi);
    if (cpu.dl & 2)
        put(app, ebp - 0x14, 0x5a);
    else
    {
        cpu.ebx = 0;
        put(app, ebp - 0x14, cpu.ebx);
    }
    // The jolt (a constant force).
    cpu.edx = ebp - 0x5c;
    cpu.esi = 3;
    cpu.edi = 0x2710;
    cpu.ecx = 0x72e4;
    cpu.eax = at(app, kForceController);
    cpu.ebx = 0xffffff;
    if (!byAddress(app, cpu, kCreateEffect))
        return;
    // The engine's rumble (type 3).
    cpu.edx = at(app, kForceController);
    put(app, ebp - 0x5c, cpu.esi);
    put(app, ebp - 0x58, cpu.edi);
    put(app, ebp - 0x4c, cpu.ecx);
    put(app, ebp - 0x28, cpu.ebx);
    cpu.esi = 4;
    cpu.edi = 0x7d0;
    putHalf(app, cpu.edx * 8 + kEffects, cpu.ax);
    cpu.eax = 0;
    cpu.edx = ebp - 0x5c;
    put(app, ebp - 0x54, cpu.eax);
    put(app, ebp - 0x50, cpu.eax);
    cpu.eax = at(app, kForceController);
    cpu.ecx = 0x186a0;
    if (!byAddress(app, cpu, kCreateEffect))
        return;
    // The road's rumble (type 4).
    cpu.edx = at(app, kForceController);
    put(app, ebp - 0x5c, cpu.esi);
    put(app, ebp - 0x58, cpu.edi);
    put(app, ebp - 0x4c, cpu.ecx);
    put(app, ebp - 0x28, cpu.ebx);
    cpu.esi = 9;
    putHalf(app, cpu.edx * 8 + kEffects + 2, cpu.ax);
    cpu.eax = 0;
    cpu.edx = ebp - 0x5c;
    put(app, ebp - 0x54, cpu.eax);
    put(app, ebp - 0x50, cpu.eax);
    cpu.eax = at(app, kForceController);
    cpu.edi = 0;
    if (!byAddress(app, cpu, kCreateEffect))
        return;
    // The steering's spring (type 9).
    cpu.edx = at(app, kForceController);
    put(app, ebp - 0x5c, cpu.esi);
    put(app, ebp - 0x58, cpu.edi);
    put(app, ebp - 0x44, cpu.edi);
    put(app, ebp - 0x40, cpu.edi);
    putHalf(app, cpu.edx * 8 + kEffects + 4, cpu.ax);
    cpu.eax = 0x2710;
    cpu.edx = ebp - 0x5c;
    for (const reg32 local : {0x50u, 0x54u, 0x48u, 0x4cu, 0x38u, 0x3cu, 0x30u, 0x34u})
        put(app, ebp - local, cpu.eax);
    cpu.eax = at(app, kForceController);
    put(app, ebp - 0x2c, cpu.edi);
    if (!byAddress(app, cpu, kCreateEffect))
        return;
    cpu.edx = at(app, kForceController);
    putHalf(app, cpu.edx * 8 + kEffects + 6, cpu.ax);
    cpu.edx = at(app, kForceController);
    cpu.eax = at(app, ((cpu.edx << 4) + cpu.edx) * 8 + kControllerGain);
    if (!direct(app, cpu, setForceGain))
        return;
    done();
}

namespace
{

/* 0x470f40, 0x470f80, 0x470fc0, the same code thrice: every effect of the
 * force controller's stopped, if there is one. */
void stopEveryEffect(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kForceController) != 0xffffffff)
    {
        if (!direct(app, cpu, stopRaceEffects))
            return;
        cpu.eax = handle(app, at(app, kForceController), kRoadHandle);
        if (!byAddress(app, cpu, kStopEffect))
            return;
        cpu.eax = handle(app, at(app, kForceController), kJoltHandle);
        if (!byAddress(app, cpu, kStopEffect))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x471000, 0x471070: every effect stopped -- or, on a Microsoft device,
 * every device's actuators turned on or off instead. */
void stopEveryEffectOr(win32::WinApplication* app, x86::CPU& cpu, reg32 actuators)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    auto done = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = at(app, kForceController);
    if (cpu.edx == 0xffffffff)
        return done();
    nameAndMicrosoft(app, cpu, cpu.edx);
    if (!direct(app, cpu, watcom::strstr))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = kAllDevices;
        if (!byAddress(app, cpu, actuators))
            return;
        return done();
    }
    if (!direct(app, cpu, stopRaceEffects))
        return;
    cpu.eax = handle(app, at(app, kForceController), kRoadHandle);
    if (!byAddress(app, cpu, kStopEffect))
        return;
    cpu.eax = handle(app, at(app, kForceController), kJoltHandle);
    if (!byAddress(app, cpu, kStopEffect))
        return;
    done();
}

}  // namespace

/* 0x470f40: the forces stopped (as the race is paused). */
void stopForces(win32::WinApplication* app, x86::CPU& cpu)
{
    stopEveryEffect(app, cpu);
}

/* 0x470f80: the forces stopped, again. */
void stopForcesAgain(win32::WinApplication* app, x86::CPU& cpu)
{
    stopEveryEffect(app, cpu);
}

/* 0x470fc0: the forces stopped, a third time. */
void stopForcesOnceMore(win32::WinApplication* app, x86::CPU& cpu)
{
    stopEveryEffect(app, cpu);
}

/* 0x471000: the forces stopped, or a Microsoft device's actuators on. */
void stopForcesOrActuatorsOn(win32::WinApplication* app, x86::CPU& cpu)
{
    stopEveryEffectOr(app, cpu, kActuatorsOn);
}

/* 0x471070: the forces stopped, or a Microsoft device's actuators off. */
void stopForcesOrActuatorsOff(win32::WinApplication* app, x86::CPU& cpu)
{
    stopEveryEffectOr(app, cpu, kActuatorsOff);
}

/* 0x4710f0: the car the force controller's player drives, to [0x678b54] and
 * eax: of the device set of the game's mode the first entry of that
 * controller (its low byte 1, its controller in the top four bits) -- in two
 * players' mode its last, the player the entry's half says -- or 0.  The
 * port's change (tools/apply_force_feedback_car.py): the scan starts from
 * the player who would have had the pad, not from nobody, so a race driven on
 * keys still gets its car. */
void forcePlayerCar(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0);
    cpu.edx = 0xffffffff;
    if (cpu.edx != at(app, kForceController))
    {
        cpu.eax = at(app, kGameMode);
        if (cpu.eax <= 4)
        {
            if (cpu.eax == 1)
            {
                cpu.eax = at(app, kGameMode);
                if (!byAddress(app, cpu, kDeviceSet))
                    return;
                cpu.ebx = cpu.eax;
                cpu.edx = 0;  // port: player one, unless an axis on the pad says otherwise
                for (cpu.eax = 0;;)
                {
                    cpu.ecx = cpu.eax * 4 + cpu.ebx;
                    cpu.esi = reg32(sreg32(at(app, cpu.ecx) << 24) >> 24);
                    if (cpu.esi == 1)
                    {
                        cpu.ecx = at(app, cpu.ecx);
                        cpu.esi = at(app, kForceController);
                        cpu.ecx >>= 28;
                        if (cpu.ecx == cpu.esi)
                            cpu.edx = sreg32(cpu.eax) >= 0xd ? 1 : 0;
                    }
                    ++cpu.eax;
                    if (sreg32(cpu.eax) >= 0x1a)
                        break;
                }
            }
            else
            {
                cpu.eax = at(app, kGameMode);
                if (!byAddress(app, cpu, kDeviceSet))
                    return;
                cpu.esi = cpu.eax;
                cpu.edx = at(app, kPlayer);  // port: the local player, unless an axis on the pad says otherwise
                for (cpu.eax = 0;;)
                {
                    cpu.ecx = cpu.eax * 4 + cpu.esi;
                    cpu.ebx = reg32(sreg32(at(app, cpu.ecx) << 24) >> 24);
                    if (cpu.ebx == 1)
                    {
                        cpu.ecx = at(app, cpu.ecx);
                        cpu.edi = at(app, kForceController);
                        cpu.ecx >>= 28;
                        if (cpu.ecx == cpu.edi)
                            cpu.edx = at(app, kPlayer);
                    }
                    ++cpu.eax;
                    if (sreg32(cpu.eax) >= 0xd)
                        break;
                }
            }
        }
    }
    cpu.eax = cpu.edx != 0xffffffff ? at(app, cpu.edx * 4 + kPlayerCars) : 0;
    put(app, kForceCar, cpu.eax);
    cpu.eax = at(app, kForceCar);
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi});
}

/* 0x4711e0: a race's forces begun, unless a replay is shown: its car found,
 * no bump nor jump, on the road; the frame's forces (0x471e10) put on the
 * game's timers every 10; the engine's and the spring's effects stopped. */
void startRaceForces(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi}, 0);
    if (at(app, kForceController) != 0xffffffff)
    {
        cpu.ecx = at(app, kReplay);
        if (cpu.ecx == 0)
        {
            if (!direct(app, cpu, forcePlayerCar))
                return;
            cpu.ebx = 1;
            cpu.edx = 0x471e10;
            cpu.esi = at(app, kTimers);
            put(app, kForceCar, cpu.eax);
            put(app, kBumped, cpu.ecx);
            put(app, kAirborne, cpu.ecx);
            cpu.ecx = 0xa;
            put(app, kSurface, cpu.ebx);
            cpu.ebx = cpu.eax;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kAddTimer))
                return;
            if (!direct(app, cpu, stopRaceEffects))
                return;
        }
    }
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi});
}

/* 0x471240: the engine's rumble and the steering's spring stopped, the gear
 * change's kick, the spring and the next jolt's frame cleared. */
void stopRaceEffects(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kForceController) != 0xffffffff)
    {
        if (!direct(app, cpu, stopEngineRumble))
            return;
        cpu.ecx = 0;
        if (!direct(app, cpu, stopSteeringSpring))
            return;
        put(app, kSpringPlaying, cpu.ecx);
        put(app, kNextJolt, cpu.ecx);
        put(app, kGearKick, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x471270: the jolt of the car at eax hit, the blow's vector [esp+4] on
 * (three floats) as the car sees it, its strength [esp+0x10] (a float): from
 * the blow's angle round the car (0 to 360 degrees), how long by its
 * strength (400 to 1000), how hard (5000 to 10000).  stdcall's ret 0x10. */
void crashJolt(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x64);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x10); };
    cpu.esi = cpu.eax;
    cpu.edi = at(app, cpu.eax + 0x170);
    if (!direct(app, cpu, isForceCar))
        return;
    cpu.edi &= 0xefff0000;
    if (cpu.eax == 0)
        return done();
    // The blow less the car's place, turned by the car's orientation's
    // transpose: as the car sees it.
    cpu.ecx = ebp - 0x28;
    cpu.edx = ebp + 0x1c;
    cpu.eax = 1;
    cpu.ebx = cpu.esi + 0x98;
    if (!byAddress(app, cpu, kSubtractVectors))
        return;
    cpu.edx = ebp - 0x64;
    cpu.eax = cpu.esi + 0xc0;
    cpu.ebx = ebp - 0x28;
    if (!byAddress(app, cpu, kTranspose))
        return;
    cpu.edx = ebp - 0x64;
    cpu.eax = ebp - 0x28;
    auto& fpu = cpu.fpu;
    for (reg32 row = 0; row < 3; ++row)
    {
        const reg32 a = cpu.eax + 4 * row, m = cpu.edx + 0xc * row;
        fpu.push(fp::mul(loadFloat(app, a), loadFloat(app, m)));
        fpu.push(fp::mul(loadFloat(app, a), loadFloat(app, m + 4)));
        fpu.push(fp::mul(loadFloat(app, a), loadFloat(app, m + 8)));
        exchange(cpu, 2);
        if (row == 0)
            continue;
        for (int i = 0; i < 3; ++i)
        {
            fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));     // faddp st(3)
            fpu.count -= 1;
        }
    }
    cpu.eax += 0xc;
    storeFloat(app, cpu.ebx, fpu.pop());
    storeFloat(app, cpu.ebx + 4, fpu.pop());
    storeFloat(app, cpu.ebx + 8, fpu.pop());
    // Its angle: atan2 of -z and x, in half turns, made 0 to 2.
    fpu.push(loadFloat(app, ebp - 0x20));
    cpu.esp -= 4;
    fpu.st(0) = -fpu.st(0);
    storeFloat(app, cpu.esp, fpu.pop());
    push(app, cpu, at(app, ebp - 0x28));
    if (!byAddress(app, cpu, kAtan2))
        return;
    fpu.push(0.0);
    exchange(cpu, 1);
    storeFloat(app, ebp - 4, fpu.pop());
    comparePop(cpu, loadFloat(app, ebp - 4));
    if (!belowOrEqual(cpu))
    {
        fpu.push(loadFloat(app, ebp - 4));
        fpu.push(fpu.st(0));
        fpu.st(0) = fp::add(fpu.st(0), loadDouble(app, kTwo));
        fpu.st(1) = fpu.st(0);                              // fstp st(1)
        fpu.count -= 1;
        storeFloat(app, ebp - 4, fpu.pop());
    }
    fpu.push(0.0);
    comparePop(cpu, loadFloat(app, ebp - 4));
    if (!belowOrEqual(cpu))
    {
        cpu.edx = 0;
        put(app, ebp - 8, cpu.edx);
    }
    else
    {
        fpu.push(loadFloat(app, ebp - 4));
        comparePop(cpu, loadFloat(app, kTwoF));
        if (!belowOrEqual(cpu))
            put(app, ebp - 0xc, kTwoBits);
        else
        {
            cpu.eax = at(app, ebp - 4);
            put(app, ebp - 0xc, cpu.eax);
        }
        cpu.eax = at(app, ebp - 0xc);
        put(app, ebp - 8, cpu.eax);
    }
    // In degrees, 0 to 360.
    fpu.push(fp::mul(loadFloat(app, ebp - 8), loadFloat(app, kHalfTurnDegrees)));
    cpu.esp -= 4;
    storeInteger(app, cpu, cpu.esp);
    cpu.eax = pop(app, cpu);
    put(app, ebp - 0x14, cpu.eax);
    if (sreg32(cpu.eax) < 0)
        cpu.eax = 0;
    else if (sreg32(cpu.eax) > 0x168)
        cpu.eax = 0x168;
    // How long: 70 times the strength, 400 to 1000.
    fpu.push(fp::mul(loadFloat(app, ebp + 0x28), loadDouble(app, kImpactScale)));
    put(app, ebp - 0x14, cpu.eax);
    cpu.esp -= 4;
    storeInteger(app, cpu, cpu.esp);
    cpu.eax = pop(app, cpu);
    put(app, ebp - 0x10, cpu.eax);
    if (cpu.eax < 0x190)
        cpu.eax = 0x190;
    else if (cpu.eax > 0x3e8)
        cpu.eax = 0x3e8;
    // How hard: 1000 times the strength, 5000 to 10000.
    fpu.push(loadFloat(app, ebp + 0x28));
    put(app, ebp - 0x10, cpu.eax);
    cpu.esp -= 4;
    storeInteger(app, cpu, cpu.esp);
    cpu.eax = pop(app, cpu);
    cpu.esi = cpu.eax;
    cpu.eax = ((cpu.eax << 2) - cpu.esi) << 3;
    cpu.eax = (cpu.eax + cpu.esi) << 3;
    cpu.esi = cpu.eax;
    cpu.eax = (cpu.eax << 2) + cpu.esi;
    put(app, ebp - 0x40, cpu.eax);
    if (sreg32(cpu.eax) < 0x1388)
        cpu.eax = 0x1388;
    else if (sreg32(cpu.eax) > 0x2710)
        cpu.eax = 0x2710;
    put(app, ebp - 0x40, cpu.eax);
    // A blow from straight ahead (the car's flag): at 0 degrees.
    if (cpu.edi == 0x10000)
    {
        cpu.edi = 0;
        put(app, ebp - 0x14, cpu.edi);
    }
    cpu.ebx = ebp - 0x40;
    cpu.edx = ebp - 0x10;
    cpu.eax = ebp - 0x14;
    if (!direct(app, cpu, playJolt))
        return;
    done();
}

/* 0x471440: the jolt of the car at eax bumped, at its speed: from the right
 * (45 degrees) for ebx above 0, else the left (315); the faster the shorter
 * (600 less twice the mph, 200 at least) and the harder (7000 and 16 times
 * the mph, 10000 at most); a softer, shorter one for edx 0, a softer one for
 * edx 2.  None at 0 mph. */
void bumpJolt(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.esi, &cpu.edi}, 0x24);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.esi, &cpu.edi}); };
    cpu.esi = cpu.eax;
    if (!direct(app, cpu, isForceCar))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, carSpeedMph) || !direct(app, cpu, watcom::CHP))
        return;
    storeInteger(app, cpu, ebp - 4);
    cpu.ecx = at(app, ebp - 4);
    if (cpu.ecx == 0)
        return done();
    cpu.eax = cpu.ecx + cpu.ecx;
    cpu.ecx = 0x258;
    cpu.ecx -= cpu.eax;
    put(app, ebp - 8, cpu.ecx);
    if (cpu.ecx < 0xc8)
        put(app, ebp - 8, 0xc8);
    cpu.eax = at(app, ebp - 4);
    cpu.eax <<= 4;
    cpu.eax += 0x1b58;
    put(app, ebp - 0x24, cpu.eax);
    if (sreg32(cpu.eax) > 0x2710)
        put(app, ebp - 0x24, 0x2710);
    put(app, ebp - 0xc, sreg32(cpu.ebx) > 0 ? 0x2d : 0x13b);
    if (cpu.edx == 0)
    {
        cpu.eax = at(app, ebp - 8);
        cpu.edi = at(app, ebp - 0x24);
        cpu.eax >>= 1;
        cpu.edi = reg32(sreg32(cpu.edi) >> 1);
        put(app, ebp - 8, cpu.eax);
        put(app, ebp - 0x24, cpu.edi);
    }
    if (cpu.edx == 2)
        put(app, ebp - 0x24, reg32(sreg32(at(app, ebp - 0x24)) >> 1));
    cpu.ebx = ebp - 0x24;
    cpu.edx = ebp - 8;
    cpu.eax = ebp - 0xc;
    if (!direct(app, cpu, playJolt))
        return;
    done();
}

/* 0x4714f0: the kick of the car at eax's gear changed, its gear at 0x50b
 * against the last one's: into neutral or reverse a short rumble (once its
 * throttle, at 0x4f4, is past 1), into a gear a jolt from ahead (from behind
 * shifting down) as long and as hard as its engine's revs (0x590) say.  None
 * below a throttle of 0.1, nor for the race's first gear. */
void gearChangeKick(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x20);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}); };
    cpu.edx = cpu.eax;
    cpu.bl = byte(app, kLastGear);
    if (!direct(app, cpu, isForceCar))
        return;
    if (cpu.eax == 0)
        return done();
    if (byte(app, kLastGear) == 0x64)
    {
        cpu.al = byte(app, cpu.edx + 0x50b);
        putByte(app, kLastGear, cpu.al);
        return done();
    }
    cpu.ecx = at(app, cpu.edx + 0x4f4);
    if (sreg32(cpu.ecx) < 0x3dcccccd)
        return done();
    cpu.al = byte(app, cpu.edx + 0x50b);
    putByte(app, kLastGear, cpu.al);
    if (cpu.al < 2)
    {
        if (cpu.al == 1 || sreg32(cpu.ecx) <= 0x3f800000)
            return done();
        cpu.esi = 0x20;
        cpu.edi = 0x1388;
        cpu.ecx = 0x9c40;
        cpu.eax = 0;
        cpu.edx = 0;
        cpu.ebx = 0;
        put(app, kGearKick, cpu.esi);
        push(app, cpu, cpu.eax);
        put(app, ebp - 0x1c, cpu.eax);
        put(app, ebp - 0x18, cpu.eax);
        push(app, cpu, cpu.eax);
        cpu.eax = ebp - 0x20;
        put(app, ebp - 0x14, cpu.ecx);
        push(app, cpu, cpu.eax);
        cpu.ecx = 0;
        push(app, cpu, cpu.edx);
        cpu.eax = handle(app, at(app, kForceController), kEngineHandle);
        cpu.edx = 1;
        put(app, ebp - 0x20, cpu.edi);
        if (!byAddress(app, cpu, kSetEffect))
            return;
        put(app, kEnginePlaying, 1);
        return done();
    }
    put(app, ebp - 8, cpu.bl > cpu.al ? 0xb5 : 1);
    cpu.eax = at(app, cpu.edx + 0x590);
    if (sreg32(cpu.eax) < 0x7d0)
    {
        cpu.ecx = 0x64;
        cpu.edx = 0x1068;
        put(app, ebp - 4, cpu.ecx);
        put(app, ebp - 0x20, cpu.edx);
    }
    else if (sreg32(cpu.eax) < 0xbb8)
    {
        cpu.edi = 0x96;
        cpu.esi = 0x1450;
        put(app, ebp - 4, cpu.edi);
        put(app, ebp - 0x20, cpu.esi);
    }
    else if (sreg32(cpu.eax) < 0xfa0)
    {
        cpu.ecx = 0xc8;
        cpu.edx = 0x1838;
        put(app, ebp - 4, cpu.ecx);
        put(app, ebp - 0x20, cpu.edx);
    }
    else
    {
        cpu.esi = 0xfa;
        cpu.ebx = 0x1c20;
        put(app, ebp - 4, cpu.esi);
        put(app, ebp - 0x20, cpu.ebx);
    }
    cpu.ebx = ebp - 0x20;
    cpu.edx = ebp - 4;
    cpu.eax = ebp - 8;
    if (!direct(app, cpu, playJolt))
        return;
    done();
}

/* 0x471650: how far the car at eax steers, for its spring: its wheel's
 * (0x920), or while it is held (the word at 0x164) the steering scale times
 * 0x4f8 over 8 -- by 32500 over the scale, -10000 to 10000 -- to [ebx], and
 * how much (its size) to [edx]. */
void steeringAmount(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    cpu.ecx = at(app, cpu.eax + 0x920);
    put(app, ebp - 8, cpu.ecx);
    if (half(app, cpu.eax + 0x164) != 0)
    {
        fpu.push(loadFloat(app, kSteerScale));
        fpu.st(0) = fp::mul(fpu.st(0), loadFloat(app, cpu.eax + 0x4f8));
        fpu.st(0) = fp::mul(fpu.st(0), loadDouble(app, kEighth));
        storeFloat(app, ebp - 8, fpu.pop());
    }
    fpu.push(loadFloat(app, kSteerRange));
    fpu.st(0) = fp::div(fpu.st(0), loadFloat(app, kSteerScale));
    fpu.push(loadFloat(app, ebp - 8));
    fpu.st(0) = -fpu.st(0);
    storeFloat(app, ebp - 4, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), loadFloat(app, ebp - 4));
    storeFloat(app, ebp - 4, fpu.st(0));
    comparePop(cpu, loadFloat(app, kTenThousandF));
    if (!belowOrEqual(cpu))
        put(app, ebp - 4, kPlusTenThousand);
    else
    {
        fpu.push(loadFloat(app, ebp - 4));
        comparePop(cpu, loadDouble(app, kMinusTenThousand));
        if (cpu.flags.cf)
            put(app, ebp - 4, kMinusTenThousandBits);
    }
    fpu.push(loadFloat(app, ebp - 4));
    if (!direct(app, cpu, watcom::CHP))
        return;
    storeInteger(app, cpu, cpu.ebx);
    cpu.eax = at(app, cpu.ebx);
    put(app, cpu.edx, cpu.eax);
    if (sreg32(cpu.eax) < 0)
    {
        cpu.eax = 0 - cpu.eax;
        put(app, cpu.edx, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4716e0: the steering's spring of the car at eax played: centred where
 * it steers, as stiff as it steers far -- its own centre (0x924), or while
 * held (0x164) -0x500's times the spring's scale over 8, by 10000 over the
 * scale; 250 to 10000, a quarter of it while the car is off the ground (0x5c0
 * above 0) -- at the steering's strength. */
void playSteeringSpring(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx}, 0x44);
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    cpu.ecx = cpu.eax;
    cpu.edx = 0;
    put(app, ebp - 0x14, cpu.edx);
    put(app, ebp - 0x10, cpu.edx);
    cpu.eax = at(app, cpu.eax + 0x924);
    cpu.dx = half(app, cpu.ecx + 0x164);
    put(app, ebp - 0xc, cpu.eax);
    if (cpu.dx != 0)
    {
        fpu.push(loadFloat(app, cpu.ecx + 0x500));
        fpu.st(0) = -fpu.st(0);
        fpu.st(0) = fp::mul(fpu.st(0), loadFloat(app, kSpringScale));
        fpu.st(0) = fp::mul(fpu.st(0), loadDouble(app, kSpringEighth));
        storeFloat(app, ebp - 0xc, fpu.pop());
    }
    cpu.ebx = ebp - 0x14;
    cpu.edx = ebp - 0x10;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, steeringAmount))
        return;
    fpu.push(loadFloat(app, kSpringRange));
    fpu.push(fpu.st(0));
    fpu.st(0) = fp::div(fpu.st(0), loadFloat(app, kSpringScale));
    fpu.push(loadFloat(app, ebp - 0xc));
    fpu.st(0) = -fpu.st(0);
    storeFloat(app, ebp - 4, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), loadFloat(app, ebp - 4));
    storeFloat(app, ebp - 4, fpu.st(0));
    compareTop(cpu, fpu.st(1));                             // fcompp
    fpu.count -= 2;
    if (!belowOrEqual(cpu))
        put(app, ebp - 4, kPlusTenThousand);
    else
    {
        fpu.push(loadFloat(app, ebp - 4));
        comparePop(cpu, loadDouble(app, kSpringMinus));
        if (cpu.flags.cf)
            put(app, ebp - 4, kMinusTenThousandBits);
    }
    fpu.push(loadFloat(app, ebp - 4));
    if (!direct(app, cpu, watcom::CHP))
        return;
    storeInteger(app, cpu, ebp - 8);
    cpu.edx = at(app, ebp - 8);
    if (sreg32(cpu.edx) < 0)
        cpu.edx = 0 - cpu.edx;
    if (sreg32(cpu.edx) < 0xfa)
        cpu.edx = 0xfa;
    else if (sreg32(cpu.edx) > 0x2710)
        cpu.edx = 0x2710;
    fpu.push(0.0);
    comparePop(cpu, loadFloat(app, cpu.ecx + 0x5c0));
    if (cpu.flags.cf)
        cpu.edx = reg32(sreg32(cpu.edx) >> 2);
    cpu.eax = at(app, ebp - 8);
    put(app, ebp - 0x40, cpu.edx);
    put(app, ebp - 0x3c, cpu.edx);
    put(app, ebp - 0x38, cpu.edx);
    put(app, ebp - 0x34, cpu.edx);
    cpu.ecx = 0;
    cpu.ebx = 0;
    put(app, ebp - 0x44, cpu.eax);
    cpu.edx = 0;
    cpu.eax = at(app, ebp - 0x14);
    push(app, cpu, cpu.edx);
    put(app, ebp - 0x2c, cpu.eax);
    cpu.eax = at(app, ebp - 0x10);
    push(app, cpu, cpu.edx);
    put(app, ebp - 0x28, cpu.eax);
    put(app, ebp - 0x24, cpu.eax);
    put(app, ebp - 0x20, cpu.eax);
    put(app, ebp - 0x1c, cpu.eax);
    cpu.eax = ebp - 0x44;
    put(app, ebp - 0x30, cpu.edx);
    push(app, cpu, cpu.eax);
    put(app, ebp - 0x18, cpu.edx);
    cpu.edx = 1;
    cpu.eax = handle(app, at(app, kForceController), kSpringHandle);
    push(app, cpu, kSpringStrength);
    if (!byAddress(app, cpu, kSetEffect))
        return;
    put(app, kSpringPlaying, 1);
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx});
}

/* 0x471860: the steering's spring of the car at eax played as the road
 * pulls it: its centre off by what the car is on (the surface the frame's
 * forces found) and its camber -- the road's tilt while the car is off the
 * ground, a wall's side (0x99c) -- its stiffness 10000 less what the surface
 * takes, less again for the car's state (0x160); at the slide's strength. */
void playSlideSpring(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x38);
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.edx = 0;
    cpu.ebx = ebp - 8;
    put(app, ebp - 4, cpu.edx);
    put(app, ebp - 8, cpu.edx);
    cpu.edx = ebp - 4;
    if (!direct(app, cpu, steeringAmount))
        return;
    cpu.eax = at(app, kSurface);
    cpu.ecx = 0x2710;
    --cpu.eax;
    cpu.edi = 0;
    // The table at 0x471818, by the surface less 1.
    static const reg32 targets[15] = {0x4718cb, 0x4718a7, 0x471899, 0x4718cb, 0x4718a7, 0x4718b5, 0x4718cb, 0x4718c6,
                                      0x4718bc, 0x4718cb, 0x4718cb, 0x4718cb, 0x4718a7, 0x4718cb, 0x4718bc};
    switch (cpu.eax <= 0xe ? targets[cpu.eax] : 0x4718cb)
    {
    case 0x471899:
        cpu.ecx -= 0x3e8;
        if (sreg32(cpu.ecx) < 0)
            cpu.ecx = 0;
        break;
    case 0x4718a7:
        cpu.ecx -= 0x7d0;
        if (sreg32(cpu.ecx) < 0)
            cpu.ecx = 0;
        break;
    case 0x4718b5:
        cpu.ecx = 0x2328;
        break;
    case 0x4718bc:
        cpu.ecx -= 0x7d0;
        cpu.ecx = 0 - cpu.ecx;
        break;
    case 0x4718c6:
        cpu.ecx = 0xffffe2b4;
        break;
    }
    cpu.eax = at(app, cpu.esi + 0x528);
    if (at(app, cpu.eax + 0x238) != 0)
    {
        cpu.eax = sreg32(cpu.ecx) < 0 ? 0x1f4 : 0xfffffe0c;
        cpu.ecx += cpu.eax;
    }
    cpu.fpu.push(0.0);
    comparePop(cpu, loadFloat(app, cpu.esi + 0x5c0));
    if (cpu.flags.cf)
    {
        cpu.eax = reg32(sreg32(at(app, kRoadCamber)) >> 24);
        cpu.edx = cpu.eax * 0x2710;
        cpu.ebx = 0x7f;
        cpu.eax = cpu.edx;
        cpu.edx = reg32(sreg32(cpu.edx) >> 31);
        idiv(cpu, cpu.ebx);
        cpu.ecx = 0 - cpu.ecx;
        cpu.edi = cpu.eax;
    }
    if ((byte(app, cpu.esi + 0x99c) & 3) != 0 && at(app, cpu.esi + 0x99c) != 3)
        cpu.edi = (byte(app, cpu.esi + 0x99c) & 1) != 0 ? 0xffffd8f0 : 0x2710;
    cpu.edx = at(app, cpu.esi + 0x160);
    if (cpu.edx == 1)
        cpu.ecx -= 0x3e8;
    else if (cpu.edx == 2)
    {
        cpu.ecx -= 0x3e8;
        if (sreg32(cpu.ecx) > 0)
            cpu.ecx = 0 - cpu.ecx;
    }
    if (sreg32(cpu.ecx) > 0x2710)
        cpu.ecx = 0x2710;
    else if (sreg32(cpu.ecx) < sreg32(0xffffd8f0))
        cpu.ecx = 0xffffd8f0;
    cpu.eax = at(app, ebp - 8);
    cpu.edx = 1;
    put(app, ebp - 0x38, cpu.edi);
    put(app, ebp - 0x34, cpu.ecx);
    put(app, ebp - 0x30, cpu.ecx);
    put(app, ebp - 0x2c, cpu.ecx);
    cpu.esi = 0;
    put(app, ebp - 0x28, cpu.ecx);
    push(app, cpu, cpu.esi);
    put(app, ebp - 0x20, cpu.eax);
    cpu.eax = at(app, ebp - 4);
    push(app, cpu, cpu.esi);
    put(app, ebp - 0x1c, cpu.eax);
    put(app, ebp - 0x18, cpu.eax);
    put(app, ebp - 0x14, cpu.eax);
    put(app, ebp - 0x10, cpu.eax);
    cpu.eax = ebp - 0x38;
    cpu.ebx = 0;
    push(app, cpu, cpu.eax);
    put(app, ebp - 0x24, cpu.esi);
    cpu.ecx = 0;
    cpu.eax = handle(app, at(app, kForceController), kSpringHandle);
    push(app, cpu, kSlideStrength);
    put(app, ebp - 0xc, cpu.esi);
    if (!byAddress(app, cpu, kSetEffect))
        return;
    put(app, kSpringPlaying, 1);
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi});
}

/* 0x4719f0: the road's rumble of the car at eax set by its engine's revs
 * (0x590): its period a million over (revs / 375 + 10), and a half; its
 * magnitude 10 doubled each 180 revs past 5000, and 1700 -- 5000 at most --
 * at the road's strength. */
void playRoadRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi}, 0x1c);
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    cpu.ecx = cpu.eax;
    cpu.ebx = 0x177;
    cpu.edx = at(app, cpu.ecx + 0x590);
    cpu.eax = at(app, cpu.eax + 0x590);
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.ebx);
    cpu.edx = at(app, cpu.ecx + 0x590);
    cpu.edx -= 0x1388;
    cpu.ebx = cpu.eax + 0xa;
    if (sreg32(cpu.edx) < 0)
        cpu.edx = 0;
    cpu.ecx = 0xb4;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
    cpu.cl = cpu.al;
    cpu.eax = 0xa;
    cpu.eax <<= cpu.cl & 31;
    cpu.eax += 0x6a4;
    put(app, ebp - 0x1c, cpu.eax);
    if (cpu.eax > 0x1388)
        put(app, ebp - 0x1c, 0x1388);
    put(app, ebp - 4, cpu.ebx);
    fpu.push(x86::Float(sreg32(at(app, ebp - 4))));
    fpu.st(0) = fp::div(loadDouble(app, kMillion), fpu.st(0));   // fdivr
    cpu.esi = 0;
    cpu.eax = ebp - 0x1c;
    push(app, cpu, cpu.esi);
    cpu.edx = 1;
    cpu.ecx = 0;
    push(app, cpu, cpu.esi);
    put(app, ebp - 0x18, cpu.esi);
    put(app, ebp - 0x14, cpu.esi);
    push(app, cpu, cpu.eax);
    cpu.eax = at(app, kForceController);
    cpu.ebx = 0;
    push(app, cpu, kRoadStrength);
    cpu.eax = at(app, cpu.eax * 8 + kRoadHandle);
    fpu.st(0) = fp::add(fpu.st(0), loadDouble(app, kHalf));
    cpu.eax = reg32(sreg32(cpu.eax) >> 16);
    if (!direct(app, cpu, watcom::CHP))
        return;
    storeInteger(app, cpu, ebp - 0x10);
    if (!byAddress(app, cpu, kSetEffect))
        return;
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi});
}

/* 0x471af0: the engine's rumble of the car at eax set by its speed: none
 * below 2 mph, else a period and magnitude of its speed's band -- changed by
 * what the car is on, the surface 0x420840 finds (else the car's own, 0x18c;
 * 0x11 against a wall), stopped on some -- played at the engine's strength
 * if they changed. */
void playEngineRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x24);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}); };
    auto& fpu = cpu.fpu;
    cpu.ecx = cpu.eax;
    if (!direct(app, cpu, carSpeedMph))
        return;
    storeFloat(app, ebp - 4, fpu.st(0));
    cpu.ebx = 0;
    comparePop(cpu, loadFloat(app, kSpeeds));
    if (cpu.flags.cf)
    {
        if (!direct(app, cpu, stopEngineRumble))
            return;
        return done();
    }
    // The speed's band: (magnitude, period) below each speed.
    static const reg32 bands[13][2] = {
        {0x9c4, 5}, {0xbb8, 6}, {0xdac, 7}, {0xfa0, 8}, {0x1194, 9}, {0x1388, 0xa}, {0x1194, 0xb},
        {0xfa0, 0xc}, {0xfa0, 0xd}, {0xdac, 0xe}, {0xdac, 0xf}, {0xdac, 0x10}, {0xbb8, 0x11}};
    reg32 band = 0;
    for (; band < 12; ++band)
    {
        fpu.push(loadFloat(app, ebp - 4));
        comparePop(cpu, loadFloat(app, kSpeeds + 4 + 4 * band));
        if (cpu.flags.cf)
            break;
    }
    cpu.edi = bands[band][0];
    cpu.esi = bands[band][1];
    cpu.edx = at(app, cpu.ecx + 0x7c);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    if (!byAddress(app, cpu, kSurfaceUnder))
        return;
    put(app, kSurface, cpu.eax);
    if (cpu.eax == 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x18c);
        put(app, kSurface, cpu.eax);
    }
    if (at(app, cpu.ecx + 0x99c) != 0)
        put(app, kSurface, 0x11);
    cpu.eax = at(app, kSurface);
    --cpu.eax;
    // The table at 0x471aa4, by the surface less 1.
    static const reg32 targets[18] = {0x471cbe, 0x471ce1, 0x471cc8, 0x471d32, 0x471cd6, 0x471d32,
                                      0x471d1d, 0x471cbe, 0x471d28, 0x471cbe, 0x471d32, 0x471d1d,
                                      0x471cd6, 0x471d2d, 0x471d28, 0x471cea, 0x471d08, 0x471cf3};
    bool play = true;
    switch (cpu.eax <= 0x11 ? targets[cpu.eax] : 0x471d2d)
    {
    case 0x471cbe:
    case 0x471d2d:
        if (!direct(app, cpu, stopEngineRumble))
            return;
        play = cpu.ebx != 0;
        break;
    case 0x471d32:
        play = cpu.ebx != 0;
        break;
    case 0x471cc8:
        cpu.esi -= 2;
        cpu.edi -= 0x3e8;
        break;
    case 0x471cd6:
        cpu.esi += 4;
        cpu.edi -= 0x5dc;
        break;
    case 0x471ce1:
        ++cpu.esi;
        cpu.edi -= 0x1f4;
        break;
    case 0x471cea:
        ++cpu.esi;
        cpu.edi += 0x3e8;
        break;
    case 0x471cf3:
        cpu.esi -= 5;
        cpu.edi -= 0x7d0;
        if (cpu.esi > 5)
            cpu.esi = 5;
        break;
    case 0x471d08:
        cpu.esi -= 5;
        cpu.edi += 0x1770;
        if (cpu.esi > 0xa)
            cpu.esi = 0xa;
        break;
    case 0x471d1d:
        cpu.esi -= 5;
        cpu.edi += 0x7d0;
        break;
    case 0x471d28:
        cpu.esi -= 3;
        break;
    }
    if (play && !(at(app, kEnginePlaying) != 0 && cpu.esi == at(app, kEnginePeriod)
                  && cpu.edi == at(app, kEngineMagnitude)))
    {
        cpu.ecx = 0;
        put(app, ebp - 0xc, cpu.esi);
        put(app, ebp - 8, cpu.ecx);
        fpu.push(x86::Float(x86::sreg64(app->getMemory<x86::reg64>(ebp - 0xc))));
        fpu.st(0) = fp::div(loadDouble(app, kEngineMillion), fpu.st(0));   // fdivr
        cpu.eax = ebp - 0x24;
        push(app, cpu, cpu.ecx);
        cpu.edx = 1;
        put(app, ebp - 0x24, cpu.edi);
        push(app, cpu, cpu.ecx);
        cpu.ebx = 0;
        put(app, ebp - 0x20, cpu.ecx);
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, kForceController);
        put(app, ebp - 0x1c, cpu.ecx);
        push(app, cpu, kEngineStrength);
        cpu.eax = at(app, cpu.eax * 8 + kEngineHandle);
        fpu.st(0) = fp::add(fpu.st(0), loadDouble(app, kEngineHalf));
        cpu.eax = reg32(sreg32(cpu.eax) >> 16);
        if (!direct(app, cpu, watcom::CHP))
            return;
        storeInteger(app, cpu, ebp - 0x18);
        if (!byAddress(app, cpu, kSetEffect))
            return;
        put(app, kEnginePlaying, 1);
    }
    put(app, kEngineMagnitude, cpu.edi);
    put(app, kEnginePeriod, cpu.esi);
    done();
}

/* 0x471dc0: whether the car at eax is the one the race's forces are played
 * from: there is a force controller, no replay, and its gain is not 0. */
void isForceCar(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    bool is = false;
    cpu.edx = at(app, kForceCar);
    if (cpu.edx != 0 && cpu.eax == cpu.edx)
    {
        cpu.ebx = at(app, kForceController);
        if (cpu.ebx != 0xffffffff && at(app, kReplay) == 0)
        {
            cpu.eax = (cpu.ebx << 4) + cpu.ebx;
            is = at(app, cpu.eax * 8 + kControllerGain) != 0;
        }
    }
    cpu.eax = is ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x471e10: a frame of the race's forces, on the game's timers: none while
 * two players race but in one view; while the car is off the ground (0x15c
 * below 0) the engine and the spring stopped, once; while bumped (0x164) a
 * jolt once it is over; then the gear change's kick counted down, the
 * engine's rumble, the steering's or the slide's spring, the road's
 * rumble -- each at its strength. */
void raceForcesFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x20);
    const reg32 ebp = cpu.ebp;
    auto done = [&] { leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}); };
    cpu.ecx = at(app, kForceCar);
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, isForceCar))
        return;
    if (cpu.eax == 0 || cpu.ecx != at(app, kForceCar))
        return done();
    if (sreg32(at(app, kReplay)) > 1 && at(app, kViewMode) != 2)
    {
        if (!direct(app, cpu, stopEngineRumble) || !direct(app, cpu, stopSteeringSpring))
            return;
        return done();
    }
    cpu.fpu.push(0.0);
    comparePop(cpu, loadFloat(app, cpu.ecx + 0x15c));
    if (!belowOrEqual(cpu))
    {
        if (at(app, kAirborne) != 0)
            return done();
        if (!direct(app, cpu, stopEngineRumble) || !direct(app, cpu, stopSteeringSpring))
            return;
        put(app, kAirborne, 1);
        return done();
    }
    cpu.edx = 0;
    put(app, kAirborne, cpu.edx);
    if (half(app, cpu.ecx + 0x164) != 0)
    {
        if (!direct(app, cpu, stopEngineRumble))
            return;
        cpu.ecx = 1;
        if (!direct(app, cpu, stopSteeringSpring))
            return;
        put(app, kBumped, cpu.ecx);
        return done();
    }
    if (at(app, kBumped) != 0)
    {
        cpu.esi = 0x2710;
        cpu.edi = 0x1f4;
        put(app, ebp - 0x20, cpu.esi);
        put(app, ebp - 8, cpu.edi);
        cpu.eax = at(app, kFrame);
        put(app, ebp - 4, (cpu.al & 1) != 0 ? 0x2d : 0x13b);
        cpu.ebx = ebp - 0x20;
        cpu.edx = ebp - 8;
        cpu.eax = ebp - 4;
        cpu.esi = 2;
        if (!direct(app, cpu, playJolt))
            return;
        cpu.edi = 0;
        put(app, kJoltWait, cpu.esi);
        put(app, kBumped, cpu.edi);
    }
    cpu.eax = at(app, kGearKick);
    if (cpu.eax != 0)
    {
        cpu.edx = cpu.eax - 1;
        put(app, kGearKick, cpu.edx);
        if (cpu.edx == 0 && !direct(app, cpu, stopEngineRumble))
            return;
    }
    if (at(app, kBumped) == 0 && at(app, kAirborne) == 0 && at(app, kEngineStrength) != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, playEngineRumble))
            return;
    }
    cpu.edx = at(app, kJoltWait);
    if (sreg32(cpu.edx) > 0)
    {
        cpu.eax = cpu.edx - 1;
        put(app, kJoltWait, cpu.eax);
    }
    else
    {
        if (at(app, kSpringStrength) != 0)
        {
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, playSteeringSpring))
                return;
        }
        else if (at(app, kSlideStrength) != 0)
        {
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, playSlideSpring))
                return;
        }
        cpu.edi = 0;
        put(app, kJoltWait, cpu.edi);
    }
    if (at(app, kRoadStrength) != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, playRoadRumble))
            return;
    }
    done();
}

/* 0x471fb0: every force-feedback device's actuators off, if a race has a
 * force controller. */
void allActuatorsOff(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kForceController) != 0xffffffff)
    {
        cpu.eax = kAllDevices;
        if (!byAddress(app, cpu, kActuatorsOff))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x471fd0: a jolt played at the jolts' strength, its direction at [eax]
 * (degrees), how long at [edx], how hard at [ebx] -- unless one played less
 * than an eighth of the last one's length ago.  A wheel's from the side:
 * ahead and behind made left or right as the frame's parity says, else the
 * side it leans to. */
void playJolt(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    cpu.esi = cpu.ebx;
    if (at(app, kJoltStrength) != 0)
    {
        cpu.edx = at(app, kFrame);
        if (sreg32(cpu.edx) >= sreg32(at(app, kNextJolt)))
        {
            cpu.ebx = reg32(sreg32(at(app, cpu.ecx)) >> 3);
            cpu.edx = at(app, kFrame);
            cpu.edx += cpu.ebx;
            cpu.ebx = at(app, kForceController);
            put(app, kNextJolt, cpu.edx);
            cpu.edx = (cpu.ebx << 4) + cpu.ebx;
            if (byte(app, cpu.edx * 8 + kControllers) & 2)
            {
                cpu.edi = at(app, cpu.eax);
                if (cpu.edi == 0 || cpu.edi == 0x168 || cpu.edi == 0xb4)
                {
                    cpu.edi = at(app, kFrame);
                    cpu.edx = (cpu.di & 1) != 0 ? 0x5a : 0x10e;
                    put(app, cpu.eax, cpu.edx);
                }
                else if (sreg32(cpu.edi) > 0 && sreg32(cpu.edi) < 0xb4)
                    put(app, cpu.eax, 0x5a);
                else
                    put(app, cpu.eax, 0x10e);
            }
            push(app, cpu, 0);
            push(app, cpu, 0);
            cpu.edx = at(app, kForceController);
            push(app, cpu, cpu.esi);
            cpu.ebx = cpu.eax;
            cpu.esi = at(app, cpu.edx * 8 + kJoltHandle);
            push(app, cpu, kJoltStrength);
            cpu.esi = reg32(sreg32(cpu.esi) >> 16);
            cpu.edx = 1;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kSetEffect))
                return;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4720a0: the engine's rumble stopped, if it plays, and forgotten. */
void stopEngineRumble(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kEnginePlaying) != 0)
    {
        cpu.eax = handle(app, at(app, kForceController), kEngineHandle);
        if (!byAddress(app, cpu, kStopEffect))
            return;
    }
    cpu.ecx = 0;
    put(app, kEnginePeriod, cpu.ecx);
    put(app, kEngineMagnitude, cpu.ecx);
    put(app, kEnginePlaying, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4720e0: the steering's spring stopped, if it plays. */
void stopSteeringSpring(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kSpringPlaying) != 0)
    {
        cpu.eax = handle(app, at(app, kForceController), kSpringHandle);
        if (!byAddress(app, cpu, kStopEffect))
            return;
    }
    cpu.ecx = 0;
    put(app, kSpringPlaying, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x472110: how fast the car at eax goes, in mph, onto the x87's stack: its
 * speed (0xb8; 0x4f4 once the race's clock is past its start), made
 * positive, times 2.24.  ax is left the x87's status word. */
void carSpeedMph(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 ebp = cpu.ebp;
    cpu.ecx = at(app, kClock);
    cpu.eax = sreg32(cpu.ecx) > sreg32(at(app, kStartClock)) ? at(app, cpu.eax + 0x4f4) : at(app, cpu.eax + 0xb8);
    put(app, ebp - 4, cpu.eax);
    cpu.fpu.push(0.0);
    comparePop(cpu, loadFloat(app, ebp - 4));
    if (!belowOrEqual(cpu))
        putByte(app, ebp - 1, byte(app, ebp - 1) ^ 0x80);
    cpu.fpu.push(fp::mul(loadFloat(app, ebp - 4), loadFloat(app, kToMph)));
    storeFloat(app, ebp - 4, cpu.fpu.st(0));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x472b60: the controllers found compared with config.dat's, as it is
 * loaded: each of the 16 DirectInput found copied over config.dat's record
 * where they differ -- its kind, its product, its name -- else its axes'
 * forces config.dat set handed on; the ones with force feedback counted and
 * their gain set; the records of controllers not found cleared.  Then the
 * force effects made, the device sets made afresh if the controllers
 * changed, and the menus told. */
void loadControllers(win32::WinApplication* app, x86::CPU& cpu)
{
    enter(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi}, 0x14);
    const reg32 ebp = cpu.ebp;
    cpu.edx = at(app, kControllerCount);
    if (!byAddress(app, cpu, kControllersFound))
        return;
    cpu.ecx = 0;
    put(app, kControllerCount, cpu.eax);
    put(app, kForceControllers, cpu.ecx);
    if (cpu.edx != cpu.eax)
        cpu.ecx = 1;
    cpu.eax = 0;
    if (!byAddress(app, cpu, kRunTimers))
        return;
    cpu.ebx = 0;
    put(app, ebp - 0x14, cpu.ecx);
    put(app, ebp - 4, cpu.ebx);
    for (;;)
    {
        // 0x472c96: controller [ebp-4] as DirectInput found it, and config.dat's.
        cpu.eax = at(app, ebp - 4);
        if (!byAddress(app, cpu, kControllerFound))
            return;
        cpu.edx = cpu.eax;
        put(app, ebp - 0x10, cpu.eax);
        cpu.eax = 0;
        cpu.ecx = at(app, ebp - 4);
        if (!byAddress(app, cpu, kRunTimers))
            return;
        cpu.eax = ((cpu.ecx << 4) + cpu.ecx) << 3;
        cpu.ebx = kControllers;
        cpu.ebx += cpu.eax;
        if (cpu.edx == 0)
        {
            cpu.edx = kControllerBytes;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, fillZero))
                return;
        }
        else
        {
            put(app, ebp - 0xc, cpu.ebx);
            cpu.eax = at(app, cpu.ebx);
            cpu.edi = at(app, cpu.edx);
            bool same = false;
            if (cpu.eax == cpu.edi)
            {
                cpu.ecx = at(app, cpu.edx + 0x38);
                if (cpu.ecx == at(app, cpu.ebx + 0x38))
                {
                    cpu.esi = at(app, cpu.edx + 0x3c);
                    if (cpu.esi == at(app, cpu.ebx + 0x3c))
                    {
                        cpu.edi = at(app, cpu.edx + 0x40);
                        if (cpu.edi == at(app, cpu.ebx + 0x40))
                        {
                            cpu.edx += 4;
                            cpu.edi = cpu.edx;
                            cpu.ecx = 0xffffffff;
                            cpu.eax = 0;
                            while (cpu.ecx != 0)
                            {
                                const x86::reg8 b = byte(app, cpu.ees + cpu.edi);
                                cpu.edi += cpu.flags.df ? reg32(-1) : 1;
                                --cpu.ecx;
                                if (b == 0)
                                    break;
                            }
                            cpu.ecx = ~cpu.ecx - 1;
                            cpu.eax = cpu.ebx + 4;
                            cpu.ebx = cpu.ecx;
                            if (!direct(app, cpu, watcom::strncmp))
                                return;
                            same = cpu.eax == 0;
                        }
                    }
                }
            }
            if (!same)
            {
                // 0x472bcd: DirectInput's record over config.dat's.
                cpu.eax = 1;
                cpu.ecx = 0x22;
                cpu.edi = at(app, ebp - 0xc);
                cpu.esi = at(app, ebp - 0x10);
                put(app, ebp - 0x14, cpu.eax);
                while (cpu.ecx != 0)
                {
                    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
                    if (cpu.flags.df)
                    {
                        cpu.edi -= 4;
                        cpu.esi -= 4;
                    }
                    else
                    {
                        cpu.edi += 4;
                        cpu.esi += 4;
                    }
                    --cpu.ecx;
                }
            }
            else
            {
                // 0x472be7: the same controller: each of its 8 axes config.dat
                // has forces for, their two handed on where they differ.
                put(app, ebp - 8, cpu.eax);
                for (; sreg32(at(app, ebp - 8)) < 8; put(app, ebp - 8, at(app, ebp - 8) + 1))
                {
                    cpu.edx = at(app, ebp - 0xc);
                    cpu.cl = byte(app, ebp - 8);
                    cpu.eax = 1;
                    cpu.ebx = at(app, cpu.edx + 0x40);
                    cpu.eax <<= cpu.cl & 31;
                    if ((cpu.eax & cpu.ebx) == 0)
                        continue;
                    cpu.edx = at(app, ebp - 8);
                    cpu.eax = at(app, ebp - 0xc);
                    cpu.edx <<= 3;
                    cpu.ecx = at(app, ebp - 0x10);
                    cpu.eax += cpu.edx;
                    cpu.ecx += cpu.edx;
                    cpu.edx = at(app, cpu.eax + 0x48);
                    if (cpu.edx != at(app, cpu.ecx + 0x48))
                    {
                        cpu.eax = at(app, ebp - 4);
                        cpu.ecx = cpu.edx;
                        cpu.ebx = 0;
                        cpu.edx = at(app, ebp - 8);
                        if (!byAddress(app, cpu, kSetAxisForce))
                            return;
                    }
                    cpu.eax = at(app, ebp - 8);
                    cpu.ecx = at(app, ebp - 0xc);
                    cpu.eax <<= 3;
                    cpu.edi = at(app, ebp - 0x10);
                    cpu.ecx += cpu.eax;
                    cpu.eax += cpu.edi;
                    cpu.edx = at(app, cpu.ecx + 0x4c);
                    if (cpu.edx != at(app, cpu.eax + 0x4c))
                    {
                        cpu.ebx = 1;
                        cpu.eax = at(app, ebp - 4);
                        cpu.ecx = cpu.edx;
                        cpu.edx = at(app, ebp - 8);
                        if (!byAddress(app, cpu, kSetAxisForce))
                            return;
                    }
                }
            }
            // 0x472c5c: one with force feedback counted, its gain set.
            cpu.eax = at(app, ebp - 0x10);
            if (byte(app, cpu.eax) & 8)
            {
                cpu.edi = at(app, kForceControllers);
                cpu.eax = at(app, ebp - 0xc);
                ++cpu.edi;
                cpu.edx = at(app, cpu.eax + 0x44);
                cpu.eax = at(app, ebp - 4);
                put(app, kForceControllers, cpu.edi);
                if (!byAddress(app, cpu, kSetGain))
                    return;
            }
        }
        put(app, ebp - 4, at(app, ebp - 4) + 1);
        if (sreg32(at(app, ebp - 4)) >= 0x10)
            break;
    }
    cpu.edx = at(app, ebp - 0x14);
    if (!direct(app, cpu, createForceEffects))
        return;
    if (cpu.edx != 0)
    {
        cpu.eax = 0;
        if (!byAddress(app, cpu, kRebuildDeviceSet))
            return;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kRebuildDeviceSet))
            return;
    }
    if (!byAddress(app, cpu, kControllersChanged))
        return;
    leave(app, cpu, {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi});
}

}  // namespace controls
}  // namespace nfs3hp
