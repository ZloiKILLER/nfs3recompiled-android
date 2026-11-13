#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_controls_input.h"
#include "native_controls_race.h"
#include "native_guest.h"
#include "native_trackdata.h"
#include "native_vertices.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>

/* The controls read in a race, and kept in config.dat: the force-feedback
 * controller a race plays its forces on -- the four effects the game makes on
 * it, a jolt, the engine's rumble, the road's rumble, the steering's spring --
 * what each frame of a race plays on them from the player's car, and the
 * controllers found compared with config.dat's.  disassemble_nfs3hp.py binds
 * each address to the function of that name here (CONTROLS_RACE_NATIVES),
 * and the generated code calls it by name.  tools/game_checks holds each
 * against its generated code.
 *
 * Each is a typed function under a name of its own, with an adapter under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * eax -- for some ebx, ecx and edx too, as the code leaves them, which no
 * caller reads -- the x87's stack and its status word as the game's code
 * does.  What they call of other files' typed functions for their answer
 * alone is called as C++ (guest::direct, their headers): the controllers
 * found and described, an axis's force and a device's gain set where nothing
 * keeps its eax, an effect created, the device sets, vectors less another,
 * what a car is on.  The rest of DirectInput's layer and of the game is called
 * by address, the registers kept (KeptRegisters), eax handed over as the code
 * has it where the callee may leave it: what the code keeps of an effect
 * started or stopped, of the actuators, of a gain set, is the eax their code
 * leaves (ebx and edx too of an effect's parameters set), which their headers
 * do not give; and an effect's number is handed as the code holds it, -1 for
 * none, which the callee's code passes over (its core takes one only).  An
 * effect's parameters are handed to it in the
 * caller's frame, and its check reads 16 words on from them -- past them into
 * the registers the frame saved, and into the stack its siblings left: so
 * each frame that holds them, and each a callee lays where a later one's
 * parameters lie, is on the guest's stack at the code's depth with what the
 * code pushes there (GuestFrame, Saved), the code's own locals written into
 * it.  The x87's operations are fp's, in the game's order, a float kept where
 * the game stores one; its compares set the status word, which the code
 * reads. */
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
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The race's force feedback: the controller's number, or -1; the car it is
// played from; a bump or a jump under way; then each controller's four
// effects' handles, words, 8 bytes a controller -- the jolt, the engine's
// rumble, the road's rumble, the steering's spring.  Read as dwords from two
// bytes before each and shifted down, as the code reads them.
constexpr reg32 kForceController = 0x678b50, kNoController = 0xffffffff;
constexpr reg32 kForceCar = 0x678b54;
constexpr reg32 kBumped = 0x678b58;
constexpr reg32 kAirborne = 0x678b5c;
constexpr reg32 kEffects = 0x678b60, kControllerEffects = 0x10, kEffectsEach = 4;
constexpr reg32 kJoltHandle = 0x678b5e, kEngineHandle = 0x678b60, kRoadHandle = 0x678b62, kSpringHandle = 0x678b64;
constexpr reg32 kInputStateBytes = 0xac;
constexpr reg32 kEnginePlaying = 0x678be0, kEnginePeriod = 0x678be4, kEngineMagnitude = 0x678be8;
constexpr reg32 kGearKick = 0x678bec;        // frames the gear change's kick has left
constexpr reg32 kSpringPlaying = 0x678bf0;
constexpr reg32 kSurface = 0x678bf4;         // what the car is on
constexpr reg32 kNextJolt = 0x678bf8;        // the frame a jolt may come again
// Controllers: 0x88 bytes each from 0x6fc970 -- flags (2 a wheel, 8 force
// feedback), the name at 4, its product at 0x38..0x40 (0x40 its axes' bits),
// the gain at 0x44, each axis's two forces from 0x48 -- how many, how many
// have force feedback; each force's strength, 0 off.
constexpr reg32 kControllerBytes = 0x88, kControllerDwords = 0x22, kControllers = 0x10;
constexpr reg32 kControllerName = 4, kControllerProduct = 0x38, kControllerAxes = 0x40, kControllerGain = 0x44;
constexpr reg32 kAxisForces = 0x48, kAxisCount = 8;
constexpr std::uint8_t kWheel = 2, kForceFeedback = 8;
constexpr reg32 kMicrosoft = 0x53aefc;       // "Microsoft": its devices' effects are played otherwise
// A car: its flags (0x10000 a blow from straight ahead), its place, its
// orientation, its speed and its speed ahead (both floats), its height off
// the ground (below 0 in the air), held still (a word), its steering, its
// slide, its lift, its gear (0 reverse, 1 neutral), its revs, its wheel's
// and its spring's centres, the wall it is against, its state, its handling
// (its own softer spring at 0x238), its slice, its side and its own surface.
constexpr reg32 kCarFlags = 0x170, kCarPlace = 0x98, kCarOrientation = 0xc0, kCarSpeed = 0xb8;
constexpr reg32 kCarSpeedAhead = 0x4f4, kCarHeight = 0x15c, kCarHeld = 0x164, kCarSteering = 0x4f8;
constexpr reg32 kCarSlide = 0x500, kCarLift = 0x5c0, kCarGear = 0x50b, kCarRevs = 0x590;
constexpr reg32 kCarWheel = 0x920, kCarCentre = 0x924, kCarWall = 0x99c, kCarState = 0x160;
constexpr reg32 kCarHandling = 0x528, kHandlingSoftSpring = 0x238;
constexpr reg32 kCarSlice = 0x1c, kCarSide = 0x7c, kCarOwnSurface = 0x18c;
constexpr reg32 kFromAhead = 0x10000, kFlagsKept = 0xefff0000;
// The race.
constexpr reg32 kPlayerCars = 0x5efa48;
constexpr reg32 kFrame = 0x5efe00;
constexpr reg32 kSteerScale = 0x5efde0, kSpringScale = 0x5efde4;
constexpr reg32 kClock = 0x5efda4;
constexpr reg32 kLastGear = 0x55b02c;        // 0x64: none yet
constexpr reg8 kNoGear = 0x64;
constexpr reg32 kJoltWait = 0x55b030;
constexpr reg32 kViewMode = 0x7cd94c, kBothViews = 2;
constexpr reg32 kRoadCamber = 0x79c4a5;
constexpr reg32 kTimers = 0x7d3690;
constexpr reg32 kSplitMode = 1, kLastMode = 4;
constexpr reg32 kNoPlayer = 0xffffffff;
// DirectInput's layer (another part's) called by address.
constexpr reg32 kActuatorsOn = 0x435d20, kActuatorsOff = 0x435d90, kSetGain = 0x435df0;
constexpr reg32 kStartEffect = 0x4361e0, kStopEffect = 0x436230;
constexpr reg32 kUnloadEffect = 0x436310, kSetEffect = 0x436330;
constexpr reg32 kAllDevices = 0x10;
// An effect's parameters set: played (1, started if it does not play); for ever.
constexpr reg32 kPlay = 1, kForEver = 0xffffff;
// The rest of the game.
constexpr reg32 kTranspose = 0x4e0110, kAtan2 = 0x4ea9c0;
constexpr reg32 kRunTimers = 0x4e7630, kAddTimer = 0x4be250;
constexpr reg32 kControllersChanged = 0x43c040;
constexpr reg32 kRaceForcesFrame = 0x471e10, kForcesEvery = 0xa;
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
constexpr sreg32 kTenThousand = 10000, kMinusTenThousandInt = -10000;

reg16 half(App* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(App* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

/* A float's bits as the x87 loads them, and a value stored as a float's. */
Float fromBits(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return Float(f);
}

reg32 bitsOf(const Float& value)
{
    const float f = float(value);
    reg32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

Float floatAt(App* app, reg32 address)
{
    return Float(app->getMemory<float>(address));
}

Float doubleAt(App* app, reg32 address)
{
    return Float(app->getMemory<double>(address));
}

/* fistp: as the control word rounds it. */
reg32 integerOf(const Float& value)
{
    return reg32(fp::toInteger<sreg32>(value));
}

/* fcom of a with b and fnstsw ax: the status word, which the code leaves in
 * ax; ja on it (a above b, neither below, equal nor unordered), jb (below or
 * unordered). */
reg16 compare(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.word;
}

bool above(reg16 status)
{
    return (status & 0x4100) == 0;
}

bool below(reg16 status)
{
    return (status & 0x0100) != 0;
}

reg32 withStatus(reg32 left, reg16 status)
{
    return (left & 0xffff0000) | status;
}

/* mov eax, [index*8 + slot]; sar eax, 0x10: the force controller's effect's
 * handle. */
reg32 handle(App* app, reg32 index, reg32 slot)
{
    return reg32(sreg32(at(app, index * 8 + slot)) >> 16);
}

/* movsx of a control's low byte: its kind; 1 an axis's half. */
constexpr reg32 kAxisKind = 1;

reg32 kindOf(reg32 control)
{
    return reg32(sreg32(control << 24) >> 24);
}

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

/* What a function's frame keeps of its caller's across a call: every register but eax. */
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

Saved savedOf(const CPU& cpu)
{
    return {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp};
}

/* A function's frame on the guest's stack at the code's depth: the values
 * it pushes, ebp's last (the frame's base), then `bytes` of locals. */
class GuestFrame
{
public:
    GuestFrame(App* app, CPU& cpu, std::initializer_list<reg32> pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp)
    {
        reg32 top = cpu.esp;
        for (const reg32 value : pushed)
        {
            top -= 4;
            put(app, top, value);
        }
        base_ = top;
        cpu_.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    reg32 operator-(reg32 offset) const { return base_ - offset; }
    reg32 operator+(reg32 offset) const { return base_ + offset; }
    reg32 base() const { return base_; }

private:
    CPU& cpu_;
    const reg32 esp_;
    reg32 base_;
};

/* A core calling a core whose frame's depth counts: the return address's
 * slot made, as the call does. */
class CallSlot
{
public:
    explicit CallSlot(CPU& cpu) : cpu_(cpu) { cpu_.esp -= 4; }
    ~CallSlot() { cpu_.esp += 4; }

private:
    CPU& cpu_;
};

/* A Watcom function called by address with its arguments in eax, edx, ebx,
 * ecx, in that order; eax as it leaves it.  nullopt when the game stopped. */
std::optional<reg32> call(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> arguments)
{
    const KeptRegisters kept(cpu);
    reg32* const registers[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    std::size_t i = 0;
    for (const reg32 argument : arguments)
        *registers[i++] = argument;
    if (!byAddress(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

/* The same, of a native called as the code calls it. */
std::optional<reg32> call(App* app, CPU& cpu, void (*function)(App*, CPU&), std::initializer_list<reg32> arguments)
{
    const KeptRegisters kept(cpu);
    reg32* const registers[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    std::size_t i = 0;
    for (const reg32 argument : arguments)
        *registers[i++] = argument;
    if (!direct(app, cpu, function))
        return std::nullopt;
    return cpu.eax;
}

/* __CHP: the value rounded toward zero. */
std::optional<Float> chop(App* app, CPU& cpu, const Float& value)
{
    const KeptRegisters kept(cpu);
    cpu.fpu.push(value);
    if (!direct(app, cpu, watcom::CHP))
        return std::nullopt;
    return cpu.fpu.take();
}

/* atan2 (0x4ea9c0, stdcall) of the two words, the first on top. */
std::optional<Float> atan2Of(App* app, CPU& cpu, reg32 first, reg32 second)
{
    const KeptRegisters kept(cpu);
    cpu.esp -= 4;
    put(app, cpu.esp, second);
    cpu.esp -= 4;
    put(app, cpu.esp, first);
    if (!byAddress(app, cpu, kAtan2))
        return std::nullopt;
    return cpu.fpu.take();
}

/* What setEffect leaves in eax, ebx and edx -- a stdcall that may change ebx,
 * ecx and edx -- which playJolt's callers keep. */
struct Leftover
{
    reg32 answer;
    reg32 ebxLeft;
    reg32 edxLeft;
};

/* Effect `effect`'s parameters set (0x436330, stdcall): `how` (1 played),
 * its direction at [directionAt], its duration at [durationAt], and the
 * stack's words -- its strength's percent's address, its parameters, its
 * button, its envelope -- pushed as the code pushes them. */
std::optional<Leftover> setEffect(App* app, CPU& cpu, reg32 effect, reg32 how, reg32 directionAt, reg32 durationAt,
                                  const std::array<reg32, 4>& words)
{
    const KeptRegisters kept(cpu);
    for (auto word = words.rbegin(); word != words.rend(); ++word)
    {
        cpu.esp -= 4;
        put(app, cpu.esp, *word);
    }
    cpu.eax = effect;
    cpu.edx = how;
    cpu.ebx = directionAt;
    cpu.ecx = durationAt;
    if (!byAddress(app, cpu, kSetEffect))
        return std::nullopt;
    return Leftover{cpu.eax, cpu.ebx, cpu.edx};
}

/* Whether controller's name has "Microsoft" in it (strstr): nonzero if so. */
std::optional<reg32> microsoftName(App* app, CPU& cpu, reg32 controller)
{
    return call(app, cpu, watcom::strstr, {controller * kControllerBytes + SETTINGS_ADDRESS(devices[0].name), kMicrosoft});
}

}  // namespace

/* 0x471dc0: whether `car` is the one the race's forces are played from:
 * there is a force controller, no replay, and its gain is not 0. */
bool forceCar(App* app, reg32 car)
{
    const reg32 played = at(app, kForceCar);
    if (played == 0 || car != played)
        return false;
    const reg32 controller = at(app, kForceController);
    if (controller == kNoController || gameSettings(app).replayMode != 0)
        return false;
    return gameSettings(app).deviceAt(controller * kControllerBytes).gain != 0;
}

namespace
{

/* 0x4720a0: the engine's rumble stopped, if it plays, and forgotten.
 * Returns eax as the code leaves it: the stop's, else `unchanged`. */
std::optional<reg32> stopEngine(App* app, CPU& cpu, reg32 unchanged)
{
    reg32 left = unchanged;
    if (at(app, kEnginePlaying) != 0)
    {
        const auto stopped = call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kEngineHandle)});
        if (!stopped)
            return std::nullopt;
        left = *stopped;
    }
    put(app, kEnginePeriod, 0);
    put(app, kEngineMagnitude, 0);
    put(app, kEnginePlaying, 0);
    return left;
}

/* 0x4720e0: the steering's spring stopped, if it plays.  eax as stopEngine. */
std::optional<reg32> stopSpring(App* app, CPU& cpu, reg32 unchanged)
{
    reg32 left = unchanged;
    if (at(app, kSpringPlaying) != 0)
    {
        const auto stopped = call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kSpringHandle)});
        if (!stopped)
            return std::nullopt;
        left = *stopped;
    }
    put(app, kSpringPlaying, 0);
    return left;
}

/* 0x471240: the engine's rumble and the steering's spring stopped, the gear
 * change's kick, the spring and the next jolt's frame cleared.  eax as
 * stopEngine. */
std::optional<reg32> stopEffects(App* app, CPU& cpu, reg32 unchanged)
{
    if (at(app, kForceController) == kNoController)
        return unchanged;
    const auto engine = stopEngine(app, cpu, unchanged);
    if (!engine)
        return std::nullopt;
    const auto spring = stopSpring(app, cpu, *engine);
    if (!spring)
        return std::nullopt;
    put(app, kSpringPlaying, 0);
    put(app, kNextJolt, 0);
    put(app, kGearKick, 0);
    return spring;
}

/* 0x470d10: every force-feedback device's gain set to `gain`, if a race has
 * a force controller.  eax as the code leaves it. */
std::optional<reg32> setGain(App* app, CPU& cpu, reg32 gain)
{
    if (at(app, kForceController) == kNoController)
        return gain;
    return call(app, cpu, kSetGain, {kAllDevices, gain});
}

/* 0x470c60: the force controller's gain set to `gain` and, but on a
 * Microsoft device, the engine's rumble started.  eax as the code leaves it. */
std::optional<reg32> startEngine(App* app, CPU& cpu, reg32 gain)
{
    if (at(app, kForceController) == kNoController)
        return gain;
    if (!setGain(app, cpu, gain))
        return std::nullopt;
    const auto microsoft = microsoftName(app, cpu, at(app, kForceController));
    if (!microsoft || *microsoft != 0)
        return microsoft;
    return call(app, cpu, kStartEffect, {handle(app, at(app, kForceController), kEngineHandle)});
}

/* 0x470bc0: the engine's rumble set to a steady one -- 5000 for 0x72e4,
 * every axis -- on the force controller, if there is one; a Microsoft
 * device's with its duration too.  Its parameters in its frame. */
std::optional<reg32> steadyEngine(App* app, CPU& cpu, reg32 unchanged)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp}, 0x20);
    const reg32 controller = at(app, kForceController);
    put(app, frame - 8, kForEver);
    put(app, frame - 4, 0);
    if (controller == kNoController)
        return unchanged;
    put(app, frame - 0x1c, 0);
    put(app, frame - 0x18, 0);
    put(app, frame - 0x14, 0x72e4);
    put(app, frame - 0x20, 0x1388);
    const auto microsoft = microsoftName(app, cpu, controller);
    if (!microsoft)
        return std::nullopt;
    const auto set = setEffect(app, cpu, handle(app, at(app, kForceController), kEngineHandle),
                               *microsoft != 0 ? 3 : kPlay, 0, frame - 8, {0, frame - 0x20, frame - 4, 0});
    if (!set)
        return std::nullopt;
    return set->answer;
}

/* 0x470cb0: the engine's rumble unloaded from the force controller, and its
 * parameters set to nothing. */
std::optional<reg32> silenceEngine(App* app, CPU& cpu, reg32 unchanged)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp}, 4);
    put(app, frame - 4, 0xffffffff);
    const reg32 controller = at(app, kForceController);
    if (controller == kNoController)
        return unchanged;
    if (!call(app, cpu, kUnloadEffect, {handle(app, controller, kEngineHandle)}))
        return std::nullopt;
    const auto set = setEffect(app, cpu, handle(app, at(app, kForceController), kEngineHandle), 0, 0, 0,
                               {0, 0, frame - 4, 0});
    if (!set)
        return std::nullopt;
    return set->answer;
}

/* 0x470d30: the force feedback's state cleared -- no force controller, every
 * effect's handle -1 -- then the first controller with force feedback made
 * the force controller, if any has. */
std::optional<bool> findController(App* app, CPU& cpu)
{
    if (!call(app, cpu, fillZero, {kForceController, kInputStateBytes}))
        return std::nullopt;
    put(app, kForceController, kNoController);
    for (reg32 controller = 0; controller < kControllerEffects; ++controller)
        for (reg32 effect = 0; effect < kEffectsEach; ++effect)
            putHalf(app, effect * 2 + controller * 8 + kEffects, 0xffff);
    if (gameSettings(app).forceDeviceCount != 0)
        for (reg32 device = 0; sreg32(device) < sreg32(gameSettings(app).deviceCount); ++device)
            if (std::uint8_t(gameSettings(app).deviceAt(device * kControllerBytes).kind) & kForceFeedback)
            {
                put(app, kForceController, device);
                return true;
            }
    return false;
}

/* 0x470dc0: the force controller found and its four effects created -- the
 * jolt, the engine's rumble, the road's rumble, the steering's spring; a
 * wheel's at 90 degrees -- and its gain set as the controller has it.  Each
 * effect's description in its frame, at -0x5c. */
std::optional<reg32> createEffects(App* app, CPU& cpu)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp}, 0x5c);
    const auto found = findController(app, cpu);
    if (!found)
        return std::nullopt;
    if (!*found)
        return 0;
    const reg32 description = frame - 0x5c;
    const auto create = [&] {
        return guest::direct<0x435f10, controls::makeEffect>(app, cpu, at(app, kForceController), description);
    };
    put(app, frame - 0x18, 0);
    put(app, frame - 0x5c, 0);
    put(app, frame - 0x1c, 0xffffffff);
    put(app, frame - 0x58, 0x2710);
    put(app, frame - 0x28, 0xc8);
    put(app, frame - 0x24, 0x2710);
    put(app, frame - 0x20, 2);
    const bool wheel = std::uint8_t(gameSettings(app).deviceAt(at(app, kForceController) * kControllerBytes).kind) & kWheel;
    put(app, frame - 0x14, wheel ? 0x5a : 0);
    // The jolt (a constant force).
    auto made = create();
    if (!made)
        return std::nullopt;
    // The engine's rumble (type 3).
    reg32 controller = at(app, kForceController);
    put(app, frame - 0x5c, 3);
    put(app, frame - 0x58, 0x2710);
    put(app, frame - 0x4c, 0x72e4);
    put(app, frame - 0x28, kForEver);
    putHalf(app, controller * 8 + kEffects, reg16(*made));
    put(app, frame - 0x54, 0);
    put(app, frame - 0x50, 0);
    made = create();
    if (!made)
        return std::nullopt;
    // The road's rumble (type 4).
    controller = at(app, kForceController);
    put(app, frame - 0x5c, 4);
    put(app, frame - 0x58, 0x7d0);
    put(app, frame - 0x4c, 0x186a0);
    put(app, frame - 0x28, kForEver);
    putHalf(app, controller * 8 + kEffects + 2, reg16(*made));
    put(app, frame - 0x54, 0);
    put(app, frame - 0x50, 0);
    made = create();
    if (!made)
        return std::nullopt;
    // The steering's spring (type 9).
    controller = at(app, kForceController);
    put(app, frame - 0x5c, 9);
    put(app, frame - 0x58, 0);
    put(app, frame - 0x44, 0);
    put(app, frame - 0x40, 0);
    putHalf(app, controller * 8 + kEffects + 4, reg16(*made));
    for (const reg32 local : {0x50u, 0x54u, 0x48u, 0x4cu, 0x38u, 0x3cu, 0x30u, 0x34u})
        put(app, frame - local, 0x2710);
    put(app, frame - 0x2c, 0);
    made = create();
    if (!made)
        return std::nullopt;
    controller = at(app, kForceController);
    putHalf(app, controller * 8 + kEffects + 6, reg16(*made));
    return setGain(app, cpu, gameSettings(app).deviceAt(at(app, kForceController) * kControllerBytes).gain);
}

/* 0x470f40, 0x470f80, 0x470fc0, the same code thrice: every effect of the
 * force controller's stopped, if there is one.  eax as the code leaves it. */
std::optional<reg32> stopAll(App* app, CPU& cpu, reg32 unchanged)
{
    if (at(app, kForceController) == kNoController)
        return unchanged;
    if (!stopEffects(app, cpu, unchanged))
        return std::nullopt;
    if (!call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kRoadHandle)}))
        return std::nullopt;
    return call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kJoltHandle)});
}

/* 0x471000, 0x471070: every effect stopped -- or, on a Microsoft device,
 * every device's actuators turned on or off instead (`actuators`). */
std::optional<reg32> stopAllOr(App* app, CPU& cpu, reg32 unchanged, reg32 actuators)
{
    const reg32 controller = at(app, kForceController);
    if (controller == kNoController)
        return unchanged;
    const auto microsoft = microsoftName(app, cpu, controller);
    if (!microsoft)
        return std::nullopt;
    if (*microsoft != 0)
        return call(app, cpu, actuators, {kAllDevices});
    if (!stopEffects(app, cpu, *microsoft))
        return std::nullopt;
    if (!call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kRoadHandle)}))
        return std::nullopt;
    return call(app, cpu, kStopEffect, {handle(app, at(app, kForceController), kJoltHandle)});
}

/* 0x4710f0: the car the force controller's player drives, to [0x678b54] and
 * returned: of the device set of the game's mode the first entry of that
 * controller (an axis, its controller in the top four bits) -- in two
 * players' mode its last, the player the entry's half says -- or 0.  The
 * port's change (tools/apply_force_feedback_car.py): the scan starts from
 * the player who would have had the pad, not from nobody, so a race driven on
 * keys still gets its car. */
std::optional<reg32> findForceCar(App* app, CPU& cpu)
{
    reg32 player = kNoPlayer;
    if (at(app, kForceController) != kNoController)
    {
        const reg32 mode = gameSettings(app).playMode;
        if (mode <= kLastMode)
        {
            const reg32 set = guest::direct<0x4904d0, controls::controlSets>(app, cpu, gameSettings(app).playMode);
            if (cpu.terminate)
                return std::nullopt;
            const auto onController = [&](reg32 entry) {
                const reg32 control = at(app, entry * 4 + set);
                return kindOf(control) == kAxisKind && control >> 28 == at(app, kForceController);
            };
            if (mode == kSplitMode)
            {
                player = 0;  // port: player one, unless an axis on the pad says otherwise
                for (reg32 entry = 0; sreg32(entry) < 0x1a; ++entry)
                    if (onController(entry))
                        player = sreg32(entry) >= 0xd ? 1 : 0;
            }
            else
            {
                player = gameSettings(app).viewedPlayer;  // port: the local player, unless an axis on the pad says otherwise
                for (reg32 entry = 0; sreg32(entry) < 0xd; ++entry)
                    if (onController(entry))
                        player = gameSettings(app).viewedPlayer;
            }
        }
    }
    put(app, kForceCar, player != kNoPlayer ? at(app, player * 4 + kPlayerCars) : 0);
    return at(app, kForceCar);
}

/* 0x4711e0: a race's forces begun, unless a replay is shown: its car found,
 * no bump nor jump, on the road; the frame's forces (0x471e10) put on the
 * game's timers every 10; the engine's and the spring's effects stopped. */
std::optional<reg32> startForces(App* app, CPU& cpu, reg32 unchanged)
{
    if (at(app, kForceController) == kNoController || gameSettings(app).replayMode != 0)
        return unchanged;
    const auto car = findForceCar(app, cpu);
    if (!car)
        return std::nullopt;
    const reg32 timers = at(app, kTimers);
    put(app, kForceCar, *car);
    put(app, kBumped, 0);
    put(app, kAirborne, 0);
    put(app, kSurface, 1);
    const auto added = call(app, cpu, kAddTimer, {timers, kRaceForcesFrame, *car, kForcesEvery});
    if (!added)
        return std::nullopt;
    return stopEffects(app, cpu, *added);
}

}  // namespace

/* 0x471fb0: every force-feedback device's actuators off, if a race has a
 * force controller. */
std::optional<reg32> actuatorsOff(App* app, CPU& cpu, reg32 unchanged)
{
    if (at(app, kForceController) == kNoController)
        return unchanged;
    return call(app, cpu, kActuatorsOff, {kAllDevices});
}

namespace
{

/* carMph's mph, and eax as the code leaves it: the speed's bits, the status
 * word of its compare in ax. */
struct Mph
{
    Float mph;
    reg32 eaxLeft;
};

/* 0x472110: how fast `car` goes, in mph: its speed (0xb8; 0x4f4 once the
 * race's clock is past its start), made positive, times 2.24. */
Mph carMph(App* app, CPU& cpu, reg32 car)
{
    const reg32 bits = sreg32(at(app, kClock)) > sreg32(gameSettings(app).localPlayer) ? at(app, car + kCarSpeedAhead)
                                                                                         : at(app, car + kCarSpeed);
    const reg16 status = compare(cpu, Float(0.0), fromBits(bits));
    const reg32 speed = above(status) ? bits ^ 0x80000000 : bits;  // its sign bit flipped below 0
    return {fp::mul(fromBits(speed), floatAt(app, kToMph)), withStatus(bits, status)};
}

/* 0x471650: how far `car` steers, for its spring: its wheel's (0x920), or
 * while it is held (the word at 0x164) the steering scale times 0x4f8 over 8
 * -- by 32500 over the scale, -10000 to 10000 -- to [amountAt], and how much
 * (its size) to [sizeAt], which it returns. */
std::optional<reg32> steering(App* app, CPU& cpu, reg32 car, reg32 amountAt, reg32 sizeAt)
{
    reg32 wheel = at(app, car + kCarWheel);
    if (half(app, car + kCarHeld) != 0)
    {
        Float held = floatAt(app, kSteerScale);
        held = fp::mul(held, floatAt(app, car + kCarSteering));
        held = fp::mul(held, doubleAt(app, kEighth));
        wheel = bitsOf(held);
    }
    const Float range = fp::div(floatAt(app, kSteerRange), floatAt(app, kSteerScale));
    const reg32 negated = bitsOf(-fromBits(wheel));
    const Float product = fp::mul(range, fromBits(negated));
    reg32 amount = bitsOf(product);
    if (above(compare(cpu, product, floatAt(app, kTenThousandF))))
        amount = kPlusTenThousand;
    else if (below(compare(cpu, fromBits(amount), doubleAt(app, kMinusTenThousand))))
        amount = kMinusTenThousandBits;
    const auto chopped = chop(app, cpu, fromBits(amount));
    if (!chopped)
        return std::nullopt;
    put(app, amountAt, integerOf(*chopped));
    reg32 size = at(app, amountAt);
    put(app, sizeAt, size);
    if (sreg32(size) < 0)
    {
        size = 0 - size;
        put(app, sizeAt, size);
    }
    return size;
}

/* 0x4716e0: the steering's spring of `car` played: centred where it steers,
 * as stiff as it steers far -- its own centre (0x924), or while held (0x164)
 * -0x500's times the spring's scale over 8, by 10000 over the scale; 250 to
 * 10000, a quarter of it while the car is off the ground (0x5c0 above 0) --
 * at the steering's strength.  Its parameters in its frame, at -0x44. */
std::optional<reg32> steeringSpring(App* app, CPU& cpu, reg32 car, const Saved& held)
{
    const GuestFrame frame(app, cpu, {held.ebx, held.ecx, held.edx, held.ebp}, 0x44);
    put(app, frame - 0x14, 0);
    put(app, frame - 0x10, 0);
    put(app, frame - 0xc, at(app, car + kCarCentre));
    if (half(app, car + kCarHeld) != 0)
    {
        // A step at a time, as the x87's st(0): the product of two NaNs is the first's.
        Float centre = -floatAt(app, car + kCarSlide);
        centre = fp::mul(centre, floatAt(app, kSpringScale));
        centre = fp::mul(centre, doubleAt(app, kSpringEighth));
        put(app, frame - 0xc, bitsOf(centre));
    }
    {
        const CallSlot slot(cpu);
        if (!steering(app, cpu, car, frame - 0x14, frame - 0x10))
            return std::nullopt;
    }
    const Float range = floatAt(app, kSpringRange);
    const Float scaled = fp::div(range, floatAt(app, kSpringScale));
    put(app, frame - 4, bitsOf(-floatAt(app, frame - 0xc)));
    const Float product = fp::mul(scaled, floatAt(app, frame - 4));
    put(app, frame - 4, bitsOf(product));
    if (above(compare(cpu, product, range)))
        put(app, frame - 4, kPlusTenThousand);
    else if (below(compare(cpu, floatAt(app, frame - 4), doubleAt(app, kSpringMinus))))
        put(app, frame - 4, kMinusTenThousandBits);
    const auto chopped = chop(app, cpu, floatAt(app, frame - 4));
    if (!chopped)
        return std::nullopt;
    put(app, frame - 8, integerOf(*chopped));
    reg32 stiffness = at(app, frame - 8);
    if (sreg32(stiffness) < 0)
        stiffness = 0 - stiffness;
    if (sreg32(stiffness) < 0xfa)
        stiffness = 0xfa;
    else if (sreg32(stiffness) > kTenThousand)
        stiffness = kTenThousand;
    if (below(compare(cpu, Float(0.0), floatAt(app, car + kCarLift))))
        stiffness = reg32(sreg32(stiffness) >> 2);
    for (const reg32 local : {0x40u, 0x3cu, 0x38u, 0x34u})
        put(app, frame - local, stiffness);
    put(app, frame - 0x44, at(app, frame - 8));
    put(app, frame - 0x2c, at(app, frame - 0x14));
    const reg32 size = at(app, frame - 0x10);
    for (const reg32 local : {0x28u, 0x24u, 0x20u, 0x1cu})
        put(app, frame - local, size);
    put(app, frame - 0x30, 0);
    put(app, frame - 0x18, 0);
    const auto set = setEffect(app, cpu, handle(app, at(app, kForceController), kSpringHandle), kPlay, 0, 0,
                               {SETTINGS_ADDRESS(forceBodyRoll), frame - 0x44, 0, 0});
    if (!set)
        return std::nullopt;
    put(app, kSpringPlaying, 1);
    return set->answer;
}

/* 0x471860: the steering's spring of `car` played as the road pulls it: its
 * centre off by what the car is on (the surface the frame's forces found) and
 * its camber -- the road's tilt while the car is off the ground, a wall's side
 * (0x99c) -- its stiffness 10000 less what the surface takes, less again for
 * the car's state (0x160); at the slide's strength.  Its parameters in its
 * frame, at -0x38. */
std::optional<reg32> slideSpring(App* app, CPU& cpu, reg32 car, const Saved& held)
{
    const GuestFrame frame(app, cpu, {held.ebx, held.ecx, held.edx, held.esi, held.edi, held.ebp}, 0x38);
    put(app, frame - 4, 0);
    put(app, frame - 8, 0);
    {
        const CallSlot slot(cpu);
        if (!steering(app, cpu, car, frame - 8, frame - 4))
            return std::nullopt;
    }
    const reg32 surface = at(app, kSurface) - 1;
    reg32 stiffness = 0x2710;
    reg32 side = 0;
    // The table at 0x471818, by the surface less 1.
    static const reg32 targets[15] = {0x4718cb, 0x4718a7, 0x471899, 0x4718cb, 0x4718a7, 0x4718b5, 0x4718cb, 0x4718c6,
                                      0x4718bc, 0x4718cb, 0x4718cb, 0x4718cb, 0x4718a7, 0x4718cb, 0x4718bc};
    switch (surface <= 0xe ? targets[surface] : 0x4718cb)
    {
    case 0x471899:
        stiffness -= 0x3e8;
        if (sreg32(stiffness) < 0)
            stiffness = 0;
        break;
    case 0x4718a7:
        stiffness -= 0x7d0;
        if (sreg32(stiffness) < 0)
            stiffness = 0;
        break;
    case 0x4718b5:
        stiffness = 0x2328;
        break;
    case 0x4718bc:
        stiffness = 0 - (stiffness - 0x7d0);
        break;
    case 0x4718c6:
        stiffness = 0xffffe2b4;
        break;
    default:
        break;
    }
    if (at(app, at(app, car + kCarHandling) + kHandlingSoftSpring) != 0)
        stiffness += sreg32(stiffness) < 0 ? 0x1f4 : 0xfffffe0c;
    if (below(compare(cpu, Float(0.0), floatAt(app, car + kCarLift))))
    {
        const sreg32 camber = sreg32(at(app, kRoadCamber)) >> 24;
        side = reg32(camber * kTenThousand / 0x7f);
        stiffness = 0 - stiffness;
    }
    if ((byte(app, car + kCarWall) & 3) != 0 && at(app, car + kCarWall) != 3)
        side = (byte(app, car + kCarWall) & 1) != 0 ? reg32(kMinusTenThousandInt) : reg32(kTenThousand);
    const reg32 state = at(app, car + kCarState);
    if (state == 1)
        stiffness -= 0x3e8;
    else if (state == 2)
    {
        stiffness -= 0x3e8;
        if (sreg32(stiffness) > 0)
            stiffness = 0 - stiffness;
    }
    if (sreg32(stiffness) > kTenThousand)
        stiffness = kTenThousand;
    else if (sreg32(stiffness) < kMinusTenThousandInt)
        stiffness = reg32(kMinusTenThousandInt);
    put(app, frame - 0x38, side);
    for (const reg32 local : {0x34u, 0x30u, 0x2cu, 0x28u})
        put(app, frame - local, stiffness);
    put(app, frame - 0x20, at(app, frame - 8));
    const reg32 size = at(app, frame - 4);
    for (const reg32 local : {0x1cu, 0x18u, 0x14u, 0x10u})
        put(app, frame - local, size);
    put(app, frame - 0x24, 0);
    put(app, frame - 0xc, 0);
    const auto set = setEffect(app, cpu, handle(app, at(app, kForceController), kSpringHandle), kPlay, 0, 0,
                               {SETTINGS_ADDRESS(forceRoadGrip), frame - 0x38, 0, 0});
    if (!set)
        return std::nullopt;
    put(app, kSpringPlaying, 1);
    return set->answer;
}

/* 0x4719f0: the road's rumble of `car` set by its engine's revs (0x590): its
 * period a million over (revs / 375 + 10), and a half; its magnitude 10
 * doubled each 180 revs past 5000, and 1700 -- 5000 at most -- at the road's
 * strength.  Its parameters in its frame, at -0x1c. */
std::optional<reg32> roadRumble(App* app, CPU& cpu, reg32 car, const Saved& held)
{
    const GuestFrame frame(app, cpu, {held.ebx, held.ecx, held.edx, held.esi, held.ebp}, 0x1c);
    const reg32 period = reg32(sreg32(at(app, car + kCarRevs)) / 0x177) + 0xa;
    reg32 excess = at(app, car + kCarRevs) - 0x1388;
    if (sreg32(excess) < 0)
        excess = 0;
    const reg32 doublings = reg32(sreg32(excess) / 0xb4);
    const reg32 magnitude = (0xau << (doublings & 31)) + 0x6a4;
    put(app, frame - 0x1c, magnitude);
    if (magnitude > 0x1388)
        put(app, frame - 0x1c, 0x1388);
    put(app, frame - 4, period);
    const Float wave = fp::div(doubleAt(app, kMillion), Float(sreg32(at(app, frame - 4))));
    put(app, frame - 0x18, 0);
    put(app, frame - 0x14, 0);
    const reg32 effect = handle(app, at(app, kForceController), kRoadHandle);
    const auto chopped = chop(app, cpu, fp::add(wave, doubleAt(app, kHalf)));
    if (!chopped)
        return std::nullopt;
    put(app, frame - 0x10, integerOf(*chopped));
    const auto set = setEffect(app, cpu, effect, kPlay, 0, 0, {SETTINGS_ADDRESS(forceEngine), frame - 0x1c, 0, 0});
    if (!set)
        return std::nullopt;
    return set->answer;
}

/* 0x471af0: the engine's rumble of `car` set by its speed: none below 2 mph,
 * else a period and magnitude of its speed's band -- changed by what the car
 * is on, the surface 0x420840 finds (else the car's own, 0x18c; 0x11 against
 * a wall), stopped on some -- played at the engine's strength if they
 * changed.  Its parameters in its frame, at -0x24. */
std::optional<reg32> engineRumble(App* app, CPU& cpu, reg32 car, const Saved& held)
{
    const GuestFrame frame(app, cpu, {held.ebx, held.ecx, held.edx, held.esi, held.edi, held.ebp}, 0x24);
    const Mph speed = carMph(app, cpu, car);
    put(app, frame - 4, bitsOf(speed.mph));
    const reg16 slowest = compare(cpu, speed.mph, floatAt(app, kSpeeds));
    reg32 left = withStatus(speed.eaxLeft, slowest);
    if (below(slowest))
        return stopEngine(app, cpu, left);
    // The speed's band: (magnitude, period) below each speed.
    static const reg32 bands[13][2] = {
        {0x9c4, 5}, {0xbb8, 6}, {0xdac, 7}, {0xfa0, 8}, {0x1194, 9}, {0x1388, 0xa}, {0x1194, 0xb},
        {0xfa0, 0xc}, {0xfa0, 0xd}, {0xdac, 0xe}, {0xdac, 0xf}, {0xdac, 0x10}, {0xbb8, 0x11}};
    reg32 band = 0;
    for (; band < 12; ++band)
        if (below(compare(cpu, floatAt(app, frame - 4), floatAt(app, kSpeeds + 4 + 4 * band))))
            break;
    reg32 magnitude = bands[band][0];
    reg32 period = bands[band][1];
    const reg32 under = guest::direct<0x420840, trackdata::groundUnder>(app, cpu, sreg32(at(app, car + kCarSlice)),
                                                                       sreg32(at(app, car + kCarSide)));
    if (cpu.terminate)
        return std::nullopt;
    put(app, kSurface, under);
    if (under == 0)
        put(app, kSurface, at(app, car + kCarOwnSurface));
    if (at(app, car + kCarWall) != 0)
        put(app, kSurface, 0x11);
    const reg32 surface = at(app, kSurface) - 1;
    left = surface;
    // The table at 0x471aa4, by the surface less 1.
    static const reg32 targets[18] = {0x471cbe, 0x471ce1, 0x471cc8, 0x471d32, 0x471cd6, 0x471d32,
                                      0x471d1d, 0x471cbe, 0x471d28, 0x471cbe, 0x471d32, 0x471d1d,
                                      0x471cd6, 0x471d2d, 0x471d28, 0x471cea, 0x471d08, 0x471cf3};
    bool play = true;
    switch (surface <= 0x11 ? targets[surface] : 0x471d2d)
    {
    case 0x471cbe:
    case 0x471d2d:
    {
        const auto stopped = stopEngine(app, cpu, left);
        if (!stopped)
            return std::nullopt;
        left = *stopped;
        play = false;
        break;
    }
    case 0x471d32:
        play = false;
        break;
    case 0x471cc8:
        period -= 2;
        magnitude -= 0x3e8;
        break;
    case 0x471cd6:
        period += 4;
        magnitude -= 0x5dc;
        break;
    case 0x471ce1:
        ++period;
        magnitude -= 0x1f4;
        break;
    case 0x471cea:
        ++period;
        magnitude += 0x3e8;
        break;
    case 0x471cf3:
        period -= 5;
        magnitude -= 0x7d0;
        if (period > 5)
            period = 5;
        break;
    case 0x471d08:
        period -= 5;
        magnitude += 0x1770;
        if (period > 0xa)
            period = 0xa;
        break;
    case 0x471d1d:
        period -= 5;
        magnitude += 0x7d0;
        break;
    case 0x471d28:
        period -= 3;
        break;
    default:
        break;
    }
    if (play && !(at(app, kEnginePlaying) != 0 && period == at(app, kEnginePeriod)
                  && magnitude == at(app, kEngineMagnitude)))
    {
        // fild qword: the period with a high dword of 0.
        put(app, frame - 0xc, period);
        put(app, frame - 8, 0);
        const Float wave = fp::div(doubleAt(app, kEngineMillion), Float(x86::sreg64(x86::reg64(period))));
        put(app, frame - 0x24, magnitude);
        put(app, frame - 0x20, 0);
        put(app, frame - 0x1c, 0);
        const reg32 effect = handle(app, at(app, kForceController), kEngineHandle);
        const auto chopped = chop(app, cpu, fp::add(wave, doubleAt(app, kEngineHalf)));
        if (!chopped)
            return std::nullopt;
        put(app, frame - 0x18, integerOf(*chopped));
        const auto set = setEffect(app, cpu, effect, kPlay, 0, 0, {SETTINGS_ADDRESS(forceRoad), frame - 0x24, 0, 0});
        if (!set)
            return std::nullopt;
        left = set->answer;
        put(app, kEnginePlaying, 1);
    }
    put(app, kEngineMagnitude, magnitude);
    put(app, kEnginePeriod, period);
    return left;
}

/* 0x471fd0: a jolt played at the jolts' strength, its direction at
 * [angleAt] (degrees), how long at [durationAt], how hard at [hardnessAt]
 * -- unless one played less than an eighth of the last one's length ago.  A
 * wheel's from the side: ahead and behind made left or right as the frame's
 * parity says, else the side it leans to.  Its frame keeps ecx, esi and edi
 * as `held` has them; eax, ebx and edx as the code leaves them. */
std::optional<Leftover> jolt(App* app, CPU& cpu, reg32 angleAt, reg32 durationAt, reg32 hardnessAt, const Saved& held)
{
    const GuestFrame frame(app, cpu, {held.ecx, held.esi, held.edi, held.ebp}, 0);
    if (gameSettings(app).forceCollision == 0)
        return Leftover{angleAt, hardnessAt, durationAt};
    const reg32 now = at(app, kFrame);
    if (sreg32(now) < sreg32(at(app, kNextJolt)))
        return Leftover{angleAt, hardnessAt, now};
    const reg32 wait = reg32(sreg32(at(app, durationAt)) >> 3);
    const reg32 next = at(app, kFrame) + wait;
    const reg32 controller = at(app, kForceController);
    put(app, kNextJolt, next);
    if (std::uint8_t(gameSettings(app).deviceAt(controller * kControllerBytes).kind) & kWheel)
    {
        const reg32 angle = at(app, angleAt);
        if (angle == 0 || angle == 0x168 || angle == 0xb4)
            put(app, angleAt, (at(app, kFrame) & 1) != 0 ? 0x5a : 0x10e);
        else if (sreg32(angle) > 0 && sreg32(angle) < 0xb4)
            put(app, angleAt, 0x5a);
        else
            put(app, angleAt, 0x10e);
    }
    return setEffect(app, cpu, handle(app, at(app, kForceController), kJoltHandle), kPlay, angleAt, durationAt,
                     {SETTINGS_ADDRESS(forceCollision), hardnessAt, 0, 0});
}

/* The registers a core holds as it calls jolt, its frame's base in ebp. */
Saved heldIn(const CPU& cpu, reg32 base)
{
    Saved held = savedOf(cpu);
    held.ebp = base;
    return held;
}

}  // namespace

/* 0x471270: the jolt of `car` hit, the blow's vector at the stack's words
 * (three floats) as the car sees it, its strength after them (a float): from
 * the blow's angle round the car (0 to 360 degrees), how long by its strength
 * (400 to 1000), how hard (5000 to 10000).  Its frame holds the vector, the
 * car's orientation transposed and the jolt's numbers. */
std::optional<reg32> crash(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp}, 0x64);
    const reg32 blow = frame + 0x1c, strengthAt = frame + 0x28;
    const reg32 vector = frame - 0x28, matrix = frame - 0x64;
    reg32 flags = at(app, car + kCarFlags);
    if (!forceCar(app, car))
        return 0;
    flags &= kFlagsKept;
    // The blow less the car's place, turned by the car's orientation's
    // transpose: as the car sees it.
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, blow, car + kCarPlace, vector);
    if (cpu.terminate)
        return std::nullopt;
    if (!call(app, cpu, kTranspose, {car + kCarOrientation, matrix, vector}))
        return std::nullopt;
    Float x = fp::mul(floatAt(app, vector), floatAt(app, matrix));
    Float y = fp::mul(floatAt(app, vector), floatAt(app, matrix + 4));
    Float z = fp::mul(floatAt(app, vector), floatAt(app, matrix + 8));
    for (reg32 row = 1; row < 3; ++row)
    {
        const reg32 a = vector + 4 * row, m = matrix + 0xc * row;
        const Float px = fp::mul(floatAt(app, a), floatAt(app, m));
        const Float py = fp::mul(floatAt(app, a), floatAt(app, m + 4));
        const Float pz = fp::mul(floatAt(app, a), floatAt(app, m + 8));
        x = fp::add(x, px);
        y = fp::add(y, py);
        z = fp::add(z, pz);
    }
    put(app, vector, bitsOf(x));
    put(app, vector + 4, bitsOf(y));
    put(app, vector + 8, bitsOf(z));
    // Its angle: atan2 of -z and x, in half turns, made 0 to 2.
    const auto angle = atan2Of(app, cpu, at(app, vector), bitsOf(-floatAt(app, vector + 8)));
    if (!angle)
        return std::nullopt;
    put(app, frame - 4, bitsOf(*angle));
    if (above(compare(cpu, Float(0.0), floatAt(app, frame - 4))))
        put(app, frame - 4, bitsOf(fp::add(floatAt(app, frame - 4), doubleAt(app, kTwo))));
    if (above(compare(cpu, Float(0.0), floatAt(app, frame - 4))))
        put(app, frame - 8, 0);
    else
    {
        put(app, frame - 0xc,
            above(compare(cpu, floatAt(app, frame - 4), floatAt(app, kTwoF))) ? kTwoBits : at(app, frame - 4));
        put(app, frame - 8, at(app, frame - 0xc));
    }
    // In degrees, 0 to 360.
    reg32 degrees = integerOf(fp::mul(floatAt(app, frame - 8), floatAt(app, kHalfTurnDegrees)));
    put(app, frame - 0x14, degrees);
    if (sreg32(degrees) < 0)
        degrees = 0;
    else if (sreg32(degrees) > 0x168)
        degrees = 0x168;
    // How long: 70 times the strength, 400 to 1000.
    const Float longer = fp::mul(floatAt(app, strengthAt), doubleAt(app, kImpactScale));
    put(app, frame - 0x14, degrees);
    reg32 duration = integerOf(longer);
    put(app, frame - 0x10, duration);
    if (duration < 0x190)
        duration = 0x190;
    else if (duration > 0x3e8)
        duration = 0x3e8;
    // How hard: 1000 times the strength, 5000 to 10000.
    put(app, frame - 0x10, duration);
    reg32 hardness = integerOf(floatAt(app, strengthAt)) * 1000;
    put(app, frame - 0x40, hardness);
    if (sreg32(hardness) < 0x1388)
        hardness = 0x1388;
    else if (sreg32(hardness) > kTenThousand)
        hardness = kTenThousand;
    put(app, frame - 0x40, hardness);
    // A blow from straight ahead (the car's flag): at 0 degrees.
    if (flags == kFromAhead)
        put(app, frame - 0x14, 0);
    const CallSlot slot(cpu);
    const auto played = jolt(app, cpu, frame - 0x14, frame - 0x10, frame - 0x40, heldIn(cpu, frame.base()));
    if (!played)
        return std::nullopt;
    return played->answer;
}

/* 0x471440: the jolt of `car` bumped, at its speed: from the right (45
 * degrees) for `side` above 0, else the left (315); the faster the shorter
 * (600 less twice the mph, 200 at least) and the harder (7000 and 16 times
 * the mph, 10000 at most); a softer, shorter one for `softness` 0, a softer
 * one for 2.  None at 0 mph.  Its numbers in its frame. */
std::optional<BumpLeft> bump(App* app, CPU& cpu, reg32 car, reg32 side, reg32 softness)
{
    const GuestFrame frame(app, cpu, {cpu.esi, cpu.edi, cpu.ebp}, 0x24);
    if (!forceCar(app, car))
        return BumpLeft{0, side, cpu.ecx, softness};
    const Mph speed = carMph(app, cpu, car);
    const auto chopped = chop(app, cpu, speed.mph);
    if (!chopped)
        return std::nullopt;
    put(app, frame - 4, integerOf(*chopped));
    const reg32 mph = at(app, frame - 4);
    if (mph == 0)
        return BumpLeft{speed.eaxLeft, side, 0, softness};
    const reg32 shorter = 0x258 - (mph + mph);
    put(app, frame - 8, shorter);
    if (shorter < 0xc8)
        put(app, frame - 8, 0xc8);
    const reg32 harder = (at(app, frame - 4) << 4) + 0x1b58;
    put(app, frame - 0x24, harder);
    if (sreg32(harder) > kTenThousand)
        put(app, frame - 0x24, kTenThousand);
    put(app, frame - 0xc, sreg32(side) > 0 ? 0x2d : 0x13b);
    if (softness == 0)
    {
        put(app, frame - 8, at(app, frame - 8) >> 1);
        put(app, frame - 0x24, reg32(sreg32(at(app, frame - 0x24)) >> 1));
    }
    if (softness == 2)
        put(app, frame - 0x24, reg32(sreg32(at(app, frame - 0x24)) >> 1));
    const CallSlot slot(cpu);
    const auto played = jolt(app, cpu, frame - 0xc, frame - 8, frame - 0x24, heldIn(cpu, frame.base()));
    if (!played)
        return std::nullopt;
    return BumpLeft{played->answer, played->ebxLeft, shorter, played->edxLeft};
}

/* 0x4714f0: the kick of `car`'s gear changed, its gear at 0x50b against the
 * last one's: into neutral or reverse a short rumble (once its speed ahead,
 * at 0x4f4, is past 1), into a gear a jolt from ahead (from behind shifting
 * down) as long and as hard as its engine's revs (0x590) say.  None below a
 * speed of 0.1, nor for the race's first gear.  Its numbers in its frame. */
std::optional<reg32> gearKick(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp}, 0x20);
    const reg8 lastGear = byte(app, kLastGear);
    if (!forceCar(app, car))
        return 0;
    if (byte(app, kLastGear) == kNoGear)
    {
        const reg8 gear = byte(app, car + kCarGear);
        putByte(app, kLastGear, gear);
        return gear;
    }
    const reg32 ahead = at(app, car + kCarSpeedAhead);
    if (sreg32(ahead) < 0x3dcccccd)  // 0.1f's bits
        return 1;
    const reg8 gear = byte(app, car + kCarGear);
    putByte(app, kLastGear, gear);
    if (gear < 2)
    {
        if (gear == 1 || sreg32(ahead) <= 0x3f800000)  // 1.0f's bits
            return gear;
        put(app, kGearKick, 0x20);
        put(app, frame - 0x1c, 0);
        put(app, frame - 0x18, 0);
        put(app, frame - 0x14, 0x9c40);
        put(app, frame - 0x20, 0x1388);
        const auto set = setEffect(app, cpu, handle(app, at(app, kForceController), kEngineHandle), kPlay, 0, 0,
                                   {0, frame - 0x20, 0, 0});
        if (!set)
            return std::nullopt;
        put(app, kEnginePlaying, 1);
        return set->answer;
    }
    put(app, frame - 8, lastGear > gear ? 0xb5 : 1);
    const sreg32 revs = sreg32(at(app, car + kCarRevs));
    const reg32 kick[4][2] = {{0x64, 0x1068}, {0x96, 0x1450}, {0xc8, 0x1838}, {0xfa, 0x1c20}};
    const int step = revs < 0x7d0 ? 0 : revs < 0xbb8 ? 1 : revs < 0xfa0 ? 2 : 3;
    put(app, frame - 4, kick[step][0]);
    put(app, frame - 0x20, kick[step][1]);
    const CallSlot slot(cpu);
    const auto played = jolt(app, cpu, frame - 8, frame - 4, frame - 0x20, heldIn(cpu, frame.base()));
    if (!played)
        return std::nullopt;
    return played->answer;
}

/* 0x471e10: a frame of the race's forces, on the game's timers: none while
 * two players race but in one view; while the car is off the ground (0x15c
 * below 0) the engine and the spring stopped, once; while bumped (0x164) a
 * jolt once it is over; then the gear change's kick counted down, the
 * engine's rumble, the steering's or the slide's spring, the road's
 * rumble -- each at its strength.  The jolt's numbers in its frame; what the
 * code holds in its registers as it calls is what its callees' frames keep
 * (`held`).  eax as the code leaves it. */
std::optional<reg32> forcesFrame(App* app, CPU& cpu, const Saved& entry)
{
    const GuestFrame frame(app, cpu, {entry.ebx, entry.ecx, entry.edx, entry.esi, entry.edi, entry.ebp}, 0x20);
    Saved held = entry;
    held.ebp = frame.base();
    const reg32 car = at(app, kForceCar);
    held.ecx = car;
    reg32 left = forceCar(app, car) ? 1 : 0;
    if (left == 0 || car != at(app, kForceCar))
        return left;
    const auto stopBoth = [&]() -> std::optional<reg32> {
        const auto engine = stopEngine(app, cpu, left);
        if (!engine)
            return std::nullopt;
        return stopSpring(app, cpu, *engine);
    };
    if (sreg32(gameSettings(app).replayMode) > 1 && at(app, kViewMode) != kBothViews)
        return stopBoth();
    const reg16 status = compare(cpu, Float(0.0), floatAt(app, car + kCarHeight));
    left = withStatus(left, status);
    if (above(status))
    {
        if (at(app, kAirborne) != 0)
            return left;
        const auto stopped = stopBoth();
        if (!stopped)
            return std::nullopt;
        put(app, kAirborne, 1);
        return stopped;
    }
    held.edx = 0;
    put(app, kAirborne, 0);
    if (half(app, car + kCarHeld) != 0)
    {
        const auto stopped = stopBoth();
        if (!stopped)
            return std::nullopt;
        put(app, kBumped, 1);
        return stopped;
    }
    if (at(app, kBumped) != 0)
    {
        put(app, frame - 0x20, 0x2710);
        put(app, frame - 8, 0x1f4);
        put(app, frame - 4, (at(app, kFrame) & 1) != 0 ? 0x2d : 0x13b);
        held.esi = 2;
        held.edi = 0x1f4;
        std::optional<Leftover> played;
        {
            const CallSlot slot(cpu);
            played = jolt(app, cpu, frame - 4, frame - 8, frame - 0x20, held);
        }
        if (!played)
            return std::nullopt;
        left = played->answer;
        held.ebx = played->ebxLeft;
        held.edx = played->edxLeft;
        held.edi = 0;
        put(app, kJoltWait, 2);
        put(app, kBumped, 0);
    }
    const reg32 kick = at(app, kGearKick);
    left = kick;
    if (kick != 0)
    {
        held.edx = kick - 1;
        put(app, kGearKick, kick - 1);
        if (kick - 1 == 0)
        {
            const auto stopped = stopEngine(app, cpu, left);
            if (!stopped)
                return std::nullopt;
            left = *stopped;
        }
    }
    // Each callee's frame at the depth the call makes.
    const auto play = [&](std::optional<reg32> (*effect)(App*, CPU&, reg32, const Saved&)) {
        const CallSlot slot(cpu);
        const auto played = effect(app, cpu, car, held);
        if (played)
            left = *played;
        return played.has_value();
    };
    if (at(app, kBumped) == 0 && at(app, kAirborne) == 0 && gameSettings(app).forceRoad != 0 && !play(engineRumble))
        return std::nullopt;
    const reg32 wait = at(app, kJoltWait);
    held.edx = wait;
    if (sreg32(wait) > 0)
    {
        left = wait - 1;
        put(app, kJoltWait, wait - 1);
    }
    else
    {
        if (gameSettings(app).forceBodyRoll != 0)
        {
            if (!play(steeringSpring))
                return std::nullopt;
        }
        else if (gameSettings(app).forceRoadGrip != 0 && !play(slideSpring))
            return std::nullopt;
        held.edi = 0;
        put(app, kJoltWait, 0);
    }
    if (gameSettings(app).forceEngine != 0 && !play(roadRumble))
        return std::nullopt;
    return left;
}

namespace
{

/* repne scasb's count from `from`: the bytes before the first 0 (the game's
 * code runs with the direction flag clear). */
reg32 lengthOf(App* app, reg32 from)
{
    reg32 length = 0;
    while (byte(app, from + length) != 0)
        ++length;
    return length;
}

}  // namespace

/* 0x472b60: the controllers found compared with config.dat's, as it is
 * loaded: each of the 16 DirectInput found copied over config.dat's record
 * where they differ -- its kind, its product, its name -- else its axes'
 * forces config.dat set handed on; the ones with force feedback counted and
 * their gain set; the records of controllers not found cleared.  Then the
 * force effects made, the device sets made afresh if the controllers
 * changed, and the menus told.  Its frame at the code's depth, for
 * createEffects'. */
std::optional<reg32> loadDevices(App* app, CPU& cpu)
{
    const GuestFrame frame(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp}, 0x14);
    const reg32 before = gameSettings(app).deviceCount;
    // (The code has the count config.dat has in edx at the call, which the callee does not read.)
    const std::optional<reg32> found = guest::direct<0x435600, controls::lookForJoysticks>(app, cpu);
    if (!found)
        return std::nullopt;
    gameSettings(app).deviceCount = *found;
    gameSettings(app).forceDeviceCount = 0;
    bool changed = before != *found;
    if (!call(app, cpu, kRunTimers, {0}))
        return std::nullopt;
    for (reg32 controller = 0; sreg32(controller) < sreg32(kControllers); ++controller)
    {
        // DirectInput's record of it, and config.dat's.
        const auto theirs = guest::direct<0x435690, controls::describe>(app, cpu, controller);
        if (!theirs)
            return std::nullopt;
        if (!call(app, cpu, kRunTimers, {0}))
            return std::nullopt;
        const reg32 record = *theirs;
        const reg32 mine = controller * kControllerBytes + SETTINGS_ADDRESS(devices);
        if (record == 0)
        {
            if (!call(app, cpu, fillZero, {mine, kControllerBytes}))
                return std::nullopt;
            continue;
        }
        bool same = false;
        if (at(app, mine) == at(app, record) && at(app, record + kControllerProduct) == at(app, mine + kControllerProduct)
            && at(app, record + kControllerProduct + 4) == at(app, mine + kControllerProduct + 4)
            && at(app, record + kControllerAxes) == at(app, mine + kControllerAxes))
        {
            const reg32 length = lengthOf(app, record + kControllerName);
            const auto compared =
                call(app, cpu, watcom::strncmp, {mine + kControllerName, record + kControllerName, length});
            if (!compared)
                return std::nullopt;
            same = *compared == 0;
        }
        if (!same)
        {
            // DirectInput's record over config.dat's.
            changed = true;
            for (reg32 i = 0; i < kControllerDwords; ++i)
                put(app, mine + i * 4, at(app, record + i * 4));
        }
        else
        {
            // The same controller: each of its 8 axes config.dat has forces
            // for, their two handed on where they differ.
            for (reg32 axis = 0; sreg32(axis) < sreg32(kAxisCount); ++axis)
            {
                if (((1u << (axis & 31)) & at(app, mine + kControllerAxes)) == 0)
                    continue;
                for (reg32 force = 0; force < 2; ++force)
                {
                    const reg32 wanted = at(app, mine + axis * 8 + kAxisForces + force * 4);
                    if (wanted != at(app, record + axis * 8 + kAxisForces + force * 4)
                        && !guest::direct<0x435b40, controls::zoneSet>(app, cpu, controller, axis, force, wanted))
                        return std::nullopt;
                }
            }
        }
        // One with force feedback counted, its gain set.
        if (byte(app, record) & kForceFeedback)
        {
            gameSettings(app).forceDeviceCount = gameSettings(app).forceDeviceCount + 1;
            if (!guest::direct<0x435df0, controls::gain>(app, cpu, controller, at(app, mine + kControllerGain)))
                return std::nullopt;
        }
    }
    std::optional<reg32> left;
    {
        const CallSlot slot(cpu);
        left = createEffects(app, cpu);
    }
    if (!left)
        return std::nullopt;
    if (changed)
    {
        if (!guest::direct<0x4904f0, controls::defaultSets>(app, cpu, 0u))
            return std::nullopt;
        left = guest::direct<0x4904f0, controls::defaultSets>(app, cpu, 1u);
        if (!left)
            return std::nullopt;
    }
    // The menus told, eax as the code has it, which the call may leave.
    return call(app, cpu, kControllersChanged, {*left});
}

namespace
{

/* The adapters' end: eax the core's answer, ret `popped` more. */
void answer(CPU& cpu, const std::optional<reg32>& result, reg32 popped = 0)
{
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4 + popped;
}

/* playEngineRumble's and its like's adapter: the effect of car eax, its
 * frame keeping the caller's registers. */
void playAdapter(App* app, CPU& cpu, std::optional<reg32> (*effect)(App*, CPU&, reg32, const Saved&))
{
    answer(cpu, effect(app, cpu, cpu.eax, savedOf(cpu)));
}

}  // namespace

void steadyEngineRumble(App* app, CPU& cpu);
void startEngineRumble(App* app, CPU& cpu);
void silenceEngineRumble(App* app, CPU& cpu);
void setForceGain(App* app, CPU& cpu);
void findForceController(App* app, CPU& cpu);
void createForceEffects(App* app, CPU& cpu);
void stopForces(App* app, CPU& cpu);
void stopForcesAgain(App* app, CPU& cpu);
void stopForcesOnceMore(App* app, CPU& cpu);
void stopForcesOrActuatorsOn(App* app, CPU& cpu);
void stopForcesOrActuatorsOff(App* app, CPU& cpu);
void forcePlayerCar(App* app, CPU& cpu);
void startRaceForces(App* app, CPU& cpu);
void stopRaceEffects(App* app, CPU& cpu);
void crashJolt(App* app, CPU& cpu);
void bumpJolt(App* app, CPU& cpu);
void gearChangeKick(App* app, CPU& cpu);
void steeringAmount(App* app, CPU& cpu);
void playSteeringSpring(App* app, CPU& cpu);
void playSlideSpring(App* app, CPU& cpu);
void playRoadRumble(App* app, CPU& cpu);
void playEngineRumble(App* app, CPU& cpu);
void isForceCar(App* app, CPU& cpu);
void raceForcesFrame(App* app, CPU& cpu);
void allActuatorsOff(App* app, CPU& cpu);
void playJolt(App* app, CPU& cpu);
void stopEngineRumble(App* app, CPU& cpu);
void stopSteeringSpring(App* app, CPU& cpu);
void carSpeedMph(App* app, CPU& cpu);
void loadControllers(App* app, CPU& cpu);

/* 0x470bc0: steadyEngine. */
void steadyEngineRumble(App* app, CPU& cpu)
{
    answer(cpu, steadyEngine(app, cpu, cpu.eax));
}

/* 0x470c60: startEngine at gain eax. */
void startEngineRumble(App* app, CPU& cpu)
{
    answer(cpu, startEngine(app, cpu, cpu.eax));
}

/* 0x470cb0: silenceEngine. */
void silenceEngineRumble(App* app, CPU& cpu)
{
    answer(cpu, silenceEngine(app, cpu, cpu.eax));
}

/* 0x470d10: setGain to eax. */
void setForceGain(App* app, CPU& cpu)
{
    answer(cpu, setGain(app, cpu, cpu.eax));
}

/* 0x470d30: findController: 1 if found, else 0. */
void findForceController(App* app, CPU& cpu)
{
    const auto found = findController(app, cpu);
    if (found)
        answer(cpu, reg32(*found ? 1 : 0));
}

/* 0x470dc0: createEffects. */
void createForceEffects(App* app, CPU& cpu)
{
    answer(cpu, createEffects(app, cpu));
}

/* 0x470f40: the forces stopped (as the race is paused). */
void stopForces(App* app, CPU& cpu)
{
    answer(cpu, stopAll(app, cpu, cpu.eax));
}

/* 0x470f80: the forces stopped, again. */
void stopForcesAgain(App* app, CPU& cpu)
{
    answer(cpu, stopAll(app, cpu, cpu.eax));
}

/* 0x470fc0: the forces stopped, a third time. */
void stopForcesOnceMore(App* app, CPU& cpu)
{
    answer(cpu, stopAll(app, cpu, cpu.eax));
}

/* 0x471000: the forces stopped, or a Microsoft device's actuators on. */
void stopForcesOrActuatorsOn(App* app, CPU& cpu)
{
    answer(cpu, stopAllOr(app, cpu, cpu.eax, kActuatorsOn));
}

/* 0x471070: the forces stopped, or a Microsoft device's actuators off. */
void stopForcesOrActuatorsOff(App* app, CPU& cpu)
{
    answer(cpu, stopAllOr(app, cpu, cpu.eax, kActuatorsOff));
}

/* 0x4710f0: findForceCar. */
void forcePlayerCar(App* app, CPU& cpu)
{
    answer(cpu, findForceCar(app, cpu));
}

/* 0x4711e0: startForces. */
void startRaceForces(App* app, CPU& cpu)
{
    answer(cpu, startForces(app, cpu, cpu.eax));
}

/* 0x471240: stopEffects. */
void stopRaceEffects(App* app, CPU& cpu)
{
    answer(cpu, stopEffects(app, cpu, cpu.eax));
}

/* 0x471270: crash of car eax; stdcall's ret 0x10. */
void crashJolt(App* app, CPU& cpu)
{
    answer(cpu, crash(app, cpu, cpu.eax), 0x10);
}

/* 0x471440: bump of car eax, side ebx, softness edx; ebx, ecx and edx as it
 * leaves them. */
void bumpJolt(App* app, CPU& cpu)
{
    const auto bumped = bump(app, cpu, cpu.eax, cpu.ebx, cpu.edx);
    if (!bumped)
        return;
    cpu.ebx = bumped->ebxLeft;
    cpu.ecx = bumped->ecxLeft;
    cpu.edx = bumped->edxLeft;
    answer(cpu, bumped->answer);
}

/* 0x4714f0: gearKick of car eax. */
void gearChangeKick(App* app, CPU& cpu)
{
    answer(cpu, gearKick(app, cpu, cpu.eax));
}

/* 0x471650: steering of car eax, to [ebx] and [edx]. */
void steeringAmount(App* app, CPU& cpu)
{
    answer(cpu, steering(app, cpu, cpu.eax, cpu.ebx, cpu.edx));
}

/* 0x4716e0: steeringSpring of car eax. */
void playSteeringSpring(App* app, CPU& cpu)
{
    playAdapter(app, cpu, steeringSpring);
}

/* 0x471860: slideSpring of car eax. */
void playSlideSpring(App* app, CPU& cpu)
{
    playAdapter(app, cpu, slideSpring);
}

/* 0x4719f0: roadRumble of car eax. */
void playRoadRumble(App* app, CPU& cpu)
{
    playAdapter(app, cpu, roadRumble);
}

/* 0x471af0: engineRumble of car eax. */
void playEngineRumble(App* app, CPU& cpu)
{
    playAdapter(app, cpu, engineRumble);
}

/* 0x471dc0: forceCar of car eax: 1 if so, else 0. */
void isForceCar(App* app, CPU& cpu)
{
    answer(cpu, reg32(forceCar(app, cpu.eax) ? 1 : 0));
}

/* 0x471e10: forcesFrame. */
void raceForcesFrame(App* app, CPU& cpu)
{
    answer(cpu, forcesFrame(app, cpu, savedOf(cpu)));
}

/* 0x471fb0: actuatorsOff. */
void allActuatorsOff(App* app, CPU& cpu)
{
    answer(cpu, actuatorsOff(app, cpu, cpu.eax));
}

/* 0x471fd0: jolt of the direction at [eax], the duration at [edx], the
 * hardness at [ebx]; ebx and edx as it leaves them. */
void playJolt(App* app, CPU& cpu)
{
    const auto played = jolt(app, cpu, cpu.eax, cpu.edx, cpu.ebx, savedOf(cpu));
    if (!played)
        return;
    cpu.ebx = played->ebxLeft;
    cpu.edx = played->edxLeft;
    answer(cpu, played->answer);
}

/* 0x4720a0: stopEngine. */
void stopEngineRumble(App* app, CPU& cpu)
{
    answer(cpu, stopEngine(app, cpu, cpu.eax));
}

/* 0x4720e0: stopSpring. */
void stopSteeringSpring(App* app, CPU& cpu)
{
    answer(cpu, stopSpring(app, cpu, cpu.eax));
}

/* 0x472110: carMph of car eax onto the x87's stack; ax left the status word
 * of its compare. */
void carSpeedMph(App* app, CPU& cpu)
{
    const Mph speed = carMph(app, cpu, cpu.eax);
    cpu.fpu.push(speed.mph);
    answer(cpu, speed.eaxLeft);
}

/* 0x472b60: loadDevices. */
void loadControllers(App* app, CPU& cpu)
{
    answer(cpu, loadDevices(app, cpu));
}

}  // namespace controls
}  // namespace nfs3hp
