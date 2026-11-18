#include "game_settings.h"
#include "native_aiplan.h"
#include "native_carstate.h"
#include "native_frontend_sima.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_guest.h"
#include "native_trackdata.h"
#include "native_vertices.h"

#include <array>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

/* The race's cameras, native whole.  The game's code calls each function here by name.
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
 * Each is a typed function under a name of its own (its core), with a thunk
 * under the game's name for the callers still on registers and for the calls by address
 * (asThunk): every register but eax as the caller had it, eax what the core
 * gives back -- what the code leaves there -- and, where the code leaves a
 * register that is no argument of its otherwise, that one so.  The cores call
 * each other as C++, and other typed files' functions as C++ too (guest::direct,
 * their headers) but where the eax a callee leaves, which its core does not give
 * back, is what a function here leaves or reads; every other function of the
 * game's is called by address, only its argument registers set (guest::watcom,
 * callGame) and its words on the stack as the code hands them, at the stack's
 * depth the code has it (Frame, Calling), so that the locals a callee is handed
 * a pointer to lie where the code's do.  The x87's arithmetic is fp's, in the
 * code's order of operands, a float kept where the code stores one; a status
 * word the code leaves in eax is fp::compare's.  The two switches go by the
 * targets nfs3.exe's tables hold (0x422a7c, 0x422c88). */
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
constexpr reg32 kCamera = 0x5e10b0, kOverride = 0x5e10b8;
constexpr reg32 kCameras = 0x552ec8;
constexpr reg32 kChanged = 0x552ff4;
// The cars, their count; the track's nodes and their count; each player's view.
constexpr reg32 kCars = 0x5efac8, kCarCount = 0x5efd9c;
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
constexpr reg32 kViews = 0x5dd950;
// Each player's car (by the second player's index, 0x6fd4f4), its record.
constexpr reg32 kPlayerCars = 0x5efa48;
// What is called by address.  The track's (native_trackdata.h) and the vectors' (native_vertices.h)
// functions are called as C++ (guest::direct) but where the registers they leave are what a caller
// here leaves or reads: a track place's node (0x41f6e0), the last dot product of pushOutOfCars
// (0x4e01f0), the last vectors added of blend (0x4dffb0) and matrices multiplied of turnHead
// (0x4e0280).
constexpr reg32 kTrackNode = 0x41f6e0;

void push(App* app, CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

// The x87's loads, as the code has them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

// A blend between two cameras' mode, whose record keeps the mode it goes to (+0x11c).
constexpr reg32 kBlend = 0x13;
// Each player's mode-0x13 record, the blend's state.
constexpr reg32 kBlendRecord = 0x552f60;
// The cycle of modes: each mode's next (25 dwords), and the next of the player's four choices.
constexpr reg32 kCycle = 0x5e103c, kCycleChoice = 0x5e10c0;
// Each player's mark around a mode's sound.
constexpr reg32 kSoundMark = 0x5e1010;
// A camera's fields: its mode, car, target, player, place, orientation, roll, zoom, its place
// on the track, its node; the blend's target mode.
constexpr reg32 kCamMode = 0, kCamCar = 4, kCamTarget = 8, kCamPlayer = 0xc, kCamPlace = 0x14,
                kCamOrientation = 0x2c, kCamRoll = 0x50, kCamZoom = 0x54, kCamTrackPlace = 0x74, kCamNode = 0xec,
                kCamBlendTarget = 0x11c;
// A car's place and orientation.
constexpr reg32 kCarPlace = 0x98, kCarOrientation = 0xc0;

/* A player's index as the cameras take it: its own in a split screen (the race's mode 1), else 0. */
reg32 playerIndex(App* app, reg32 player)
{
    return gameSettings(app).playMode == 1 ? player & 1 : 0;
}

reg32 cameraInUse(App* app, reg32 player) { return at(app, kCamera + player * 4); }
reg32 overridingCamera(App* app, reg32 player) { return at(app, kOverride + player * 4); }

/* The camera that counts for the player: the one overriding, else the one in use. */
reg32 activeCamera(App* app, reg32 player)
{
    const reg32 camera = overridingCamera(app, player);
    return camera != 0 ? camera : cameraInUse(app, player);
}

/* A camera's mode, 0 for none. */
reg32 modeOf(App* app, reg32 camera) { return camera != 0 ? at(app, camera + kCamMode) : 0; }

/* The active camera's mode: the overriding one's, the one in use's, or 0. */
reg32 activeMode(App* app, reg32 player)
{
    const reg32 camera = overridingCamera(app, player);
    return camera != 0 ? at(app, camera + kCamMode) : modeOf(app, cameraInUse(app, player));
}

/* Player's camera of a mode. */
reg32 cameraOfMode(App* app, reg32 player, reg32 mode) { return at(app, kCameras + mode * 8 + player * 4); }

/* A float through the x87, as fld and fstp move it: a signalling NaN comes out quiet. */
void copyFloat(App* app, reg32 to, reg32 from) { app->getMemory<float>(to) = float(f32(app, from)); }
void putFloat(App* app, reg32 a, float f) { app->getMemory<float>(a) = f; }

reg32 bitsOf(float f)
{
    reg32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

float floatOf(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

Float floatIn(reg32 bits) { return Float(floatOf(bits)); }

/* The guest's stack as the code's frame has it, for the locals a callee is handed a pointer into: `base` the code's ebp, the entry's esp less the registers
 * it saves; esp below its locals while it calls out, and as it was on leaving. */
class Frame
{
public:
    Frame(CPU& cpu, reg32 saved, reg32 locals) : cpu_(cpu), esp_(cpu.esp), base(cpu.esp - saved)
    {
        cpu.esp = base - locals;
    }
    ~Frame() { cpu_.esp = esp_; }

private:
    CPU& cpu_;
    const reg32 esp_;

public:
    const reg32 base;
};

/* A call of this file's own as the code makes it: the return address's slot, and the words it
 * pushes, taken on the guest's stack, so that the callee's frame lies where the code's does. */
class Calling
{
public:
    explicit Calling(CPU& cpu, reg32 words = 0) : cpu_(cpu), esp_(cpu.esp) { cpu.esp -= 4 + 4 * words; }
    ~Calling() { cpu_.esp = esp_; }

private:
    CPU& cpu_;
    const reg32 esp_;
};

/* A thunk's run of its core: every register but eax as the caller had it after it (the code
 * pushes and pops what it uses; its other registers are its arguments, which no caller reads), eax
 * what the core gives -- a core that gives nothing, or answers only whether the game went on (a
 * bool), leaves eax as it is -- and the return address and `popped` bytes of the caller's words
 * popped.  False when the game stopped. */
template <typename T>
struct IsOptional : std::false_type
{
};

template <typename T>
struct IsOptional<std::optional<T>> : std::true_type
{
};

template <typename Core>
bool asThunk(CPU& cpu, x86::Args& args, Core&& core, reg32 popped = 0)
{
    using Result = std::decay_t<decltype(core())>;
    const reg32 esp = cpu.esp;
    std::optional<reg32> left;
    {
        if constexpr (std::is_void_v<Result>)
            core();
        else if constexpr (std::is_same_v<Result, bool>)
        {
            if (!core())
                return false;
        }
        else if constexpr (IsOptional<Result>::value)
        {
            const Result result = core();
            if (!result)
                return false;
            left = reg32(*result);
        }
        else
            left = reg32(core());
    }
    cpu.esp = esp;
    if (left)
        args.eax = *left;
    cpu.esp += 4 + popped;
    return true;
}

/* The caller's word `n` (from 1) on the guest's stack, above the return address. */
reg32 word(App* app, const CPU& cpu, reg32 n) { return at(app, cpu.esp + 4 * n); }

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.  A reg32 result is the eax the function leaves, given back; eax is
 * spoilt there after it, so that code still reading what the callee left in eax differs there. */
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
            spoil();
            return cpu.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* 0x4dbde0 (simc's): the view set; what it leaves in eax. */
std::optional<reg32> setView(App* app, CPU& cpu, reg32 view)
{
    return feCore<0x4dbde0, frontend::simc::setView>(app, cpu, view);
}

/* 0x426450 (sima's): mode's sound for the player. */
bool modeSound(App* app, CPU& cpu, reg32 mode, reg32 player)
{
    return feCore<0x426450, frontend::sima::cameraStep>(app, cpu, mode, player).has_value();
}

/* An int's size as the code has it: the lowest int stays itself. */
reg32 magnitude(reg32 value)
{
    return sreg32(value) < 0 ? 0u - value : value;
}

// This file's own, defined below, as C++.
std::optional<reg32> putCameraMode(App* app, CPU& cpu, reg32 player, reg32 mode);
std::optional<reg32> putOverrideMode(App* app, CPU& cpu, reg32 player, reg32 mode);
void buildCycle(App* app);

}  // namespace

x86::reg32 idleHook(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewFromCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewFromScreenCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 chooseView(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 idleHookToo(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 pickChaseNode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 turnAround(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 attachCameras(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 headTurn(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraNode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraTrackPlace(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepOutOfCars(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepAboveRoad(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepOnTrack(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepFromRightWall(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepFromLeftWall(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keepAboveGround(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lookAtTarget(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blendCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 placeCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 switchCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraZoom(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewMatrix(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 updateCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraView(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cockpitShown(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraCar(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraTarget(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 initCameras(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setCameraMode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setOverrideMode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nextCycleMode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 buildCameraCycle(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 modeInCycle(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rememberMode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 cameraMode(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setModeAnew(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 settleCamera(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nearestSpot(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 scaledIfSet(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 bitsBroken(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 dampVector(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 chaseOffset(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 chaseOffsetLevel(App* app, x86::CPU& cpu, x86::Args& args);

/* 0x421360 and 0x421480: empty hooks. */
x86::reg32 idleHook(App* app, x86::CPU& cpu, x86::Args& args)
{
    (void)app;
    asThunk(cpu, args, [] {});
    return args.eax;
}

x86::reg32 idleHookToo(App* app, x86::CPU& cpu, x86::Args& args)
{
    return idleHook(app, cpu, args);
}

namespace
{

std::optional<reg32> updateView(App* app, CPU& cpu, reg32 player, reg32 view, reg32 flag);

/* 0x421380's: the view of the camera: updated (its flag at +0, its player +4, the view +8 --
 * updateView), then set (0x4dbde0); eax as that leaves it.  Nullopt when the game stopped. */
std::optional<reg32> viewOfCamera(App* app, CPU& cpu, reg32 view)
{
    const Frame frame(cpu, 0x10, 0);
    {
        const Calling calling(cpu);
        if (!updateView(app, cpu, at(app, view + 4), view + 8, at(app, view)))
            return std::nullopt;
    }
    return setView(app, cpu, view);
}

}  // namespace

/* 0x421380: viewOfCamera for view eax. */
x86::reg32 viewFromCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, view = args.eax] { return viewOfCamera(app, cpu, view); });
    return args.eax;
}

namespace
{

// A screen camera's (mode 8) frustum and corners, and the view's they are copied to.
constexpr reg32 kScreenCamera = 8;

/* 0x4213b0's: the view set (0x4dbde0); where its player's camera is of mode 8, the view's frustum
 * (0x56009c..) and its corners (0x564434..) from the camera's own (0x5e1020..); the player's mark
 * of a change cleared; eax the view's player, as the code leaves it.  Nullopt when the game
 * stopped. */
std::optional<reg32> screenView(App* app, CPU& cpu, reg32 view)
{
    const Frame frame(cpu, 0x10, 0);
    if (!setView(app, cpu, view))
        return std::nullopt;
    const reg32 camera = cameraInUse(app, at(app, view + 4));
    if (camera != 0 && at(app, camera + kCamMode) == kScreenCamera)
    {
        copyFloat(app, 0x56009c, 0x5e1020);
        put(app, 0x56443c, at(app, 0x5e10a0));
        copyFloat(app, 0x5600a0, 0x5e1038);
        put(app, 0x564440, at(app, 0x5e1034));
        copyFloat(app, 0x5600a4, 0x5e102c);
        put(app, 0x564434, at(app, 0x5e1024));
        copyFloat(app, 0x5600a8, 0x5e1030);
        put(app, 0x564438, at(app, 0x5e1028));
    }
    const reg32 player = at(app, view + 4);
    put(app, kChanged + player * 4, 0);
    return player;
}

}  // namespace

/* 0x4213b0: screenView of view eax. */
x86::reg32 viewFromScreenCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, view = args.eax] { return screenView(app, cpu, view); });
    return args.eax;
}

namespace
{

/* 0x421450's: the view kept (0x552ec4) and set the way `how` says: 0 by its camera, 2 as a
 * screen's, any other not at all; eax as the one taken leaves it, else the view.  Nullopt when the
 * game stopped. */
std::optional<reg32> chooseHow(App* app, CPU& cpu, reg32 view, reg32 how)
{
    const Frame frame(cpu, 4, 0);
    put(app, 0x552ec4, view);
    const Calling calling(cpu);
    if (how == 0)
        return viewOfCamera(app, cpu, view);
    if (how == 2)
        return screenView(app, cpu, view);
    return view;
}

}  // namespace

/* 0x421450: chooseHow for view eax, edx how. */
x86::reg32 chooseView(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, view = args.eax, how = args.edx] { return chooseHow(app, cpu, view, how); });
    return args.eax;
}

namespace
{

// The speed bands (8 dwords among the code) and the chase's state: each player's band, node
// ahead and a mark.
constexpr reg32 kSpeedBands = 0x4212b0, kChaseBand = 0x553014, kChaseNode = 0x55300c, kChaseMark = 0x553004;
// A camera's car's physics: its slice of the track, its speed.
constexpr reg32 kPhysSlice = 0x1c, kPhysSpeed = 0x7c0;

/* 0x421490's: player's chase camera's node ahead: the car's speed (scaled by 0x536e04) placed
 * among the bands (their index kept), and the node 3, 5 or 7 on from the car's (or 3 back, below
 * the first band) kept, wrapped round the track; the mark cleared.  Returns the node.  The code
 * looks up the bands in its frame's copy with no end: past the last band it would read its saved
 * registers, which only a speed past 0x8c0000 reaches, more than any car has; here it stops at
 * the last. */
reg32 chaseNode(App* app, reg32 player)
{
    const reg32 physics = at(app, cameraInUse(app, player) + kCamCar);
    const sreg32 speed = fp::toInteger<sreg32>(fp::mul(f32(app, physics + kPhysSpeed), f64(app, 0x536e04)));
    const reg32 index = playerIndex(app, player);
    const auto band = [&](reg32 i) { return at(app, kSpeedBands + i * 4); };
    reg32 level = 0;
    while (level < 7 && speed > sreg32(band(level + 1)))
        ++level;
    put(app, kChaseBand + index * 4, level);
    const sreg32 past = sreg32(reg32(speed) - band(level));
    const reg32 slice = at(app, physics + kPhysSlice);
    const reg32 count = at(app, kNodeCount);
    reg32 node;
    if (past < 0)
    {
        node = slice - 3;
        if (sreg32(node) < 0)
            node += count;
    }
    else
    {
        node = slice + (past < 0x30000 ? 3 : past < 0x140000 ? 5 : 7);
        if (sreg32(node) >= sreg32(count))
            node -= count;
    }
    put(app, kChaseNode + index * 4, node);
    put(app, kChaseMark + index * 4, 0);
    return node;
}

}  // namespace

/* 0x421490: chaseNode for player eax. */
x86::reg32 pickChaseNode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return chaseNode(app, player); });
    return args.eax;
}

namespace
{

/* 0x421570's: the orientation turned about its up axis into `to`: its first and last rows
 * negated (through the x87), the middle one copied; each read just before its write. */
void turnedAround(App* app, reg32 from, reg32 to)
{
    for (const reg32 k : {0u, 4u, 8u})
        putFloat(app, to + k, float(-f32(app, from + k)));
    for (const reg32 k : {0xcu, 0x10u, 0x14u})
        put(app, to + k, at(app, from + k));
    for (const reg32 k : {0x18u, 0x1cu, 0x20u})
        putFloat(app, to + k, float(-f32(app, from + k)));
}

}  // namespace

/* 0x421570: turnedAround from eax into edx. */
x86::reg32 turnAround(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, from = args.eax, to = args.edx] { turnedAround(app, from, to); });
    return args.eax;
}

namespace
{

// The race's cars and the cops' count; each player's pursuit's record.
constexpr reg32 kRaceCarCount = 0x5efd98, kCopCount = 0x5efda4, kPursuits = 0x552f50;
constexpr reg32 kPursuitMode = 0x11;

/* 0x4215c0's: player's cameras put on the car (if it is one of the race's): a pursuit's radio
 * stopped, the mode's sound (0x426310), the change marked where the car is another, and each camera
 * of modes 1 to 24 given the car, its place and its orientation; then mode 3.  eax as the code
 * leaves it: the cars' count where the car is none of them, else as putCameraMode leaves it.
 * Nullopt when the game stopped. */
std::optional<reg32> attach(App* app, CPU& cpu, reg32 player, reg32 car)
{
    const Frame frame(cpu, 0x14, 4);
    const reg32 index = playerIndex(app, player);
    const reg32 cars = at(app, kCopCount) + at(app, kRaceCarCount);
    if (sreg32(car) >= sreg32(cars))
        return cars;
    if (activeMode(app, index) == kPursuitMode)
    {
        const reg32 pursuit = at(app, kPursuits + index * 4);
        const reg32 animation = at(app, pursuit + 0x118);
        if (animation != 0xffffffff)
        {
            guest::direct<0x40f460, aiplan::endAnimation>(app, cpu, animation);
            if (cpu.terminate)
                return std::nullopt;
            put(app, pursuit + 0x118, 0xffffffff);
        }
    }
    if (!feCore<0x426310, frontend::sima::cameraBackFromWait>(app, cpu, index))
        return std::nullopt;
    for (const reg32 field : {kCamTarget, kCamCar})
        if (at(app, activeCamera(app, index) + field) != at(app, kCars + car * 4))
            put(app, kChanged + index * 4, 1);
    for (reg32 mode = 1; mode < 0x19; ++mode)
    {
        const reg32 camera = cameraOfMode(app, index, mode);
        const reg32 record = at(app, kCars + car * 4);
        put(app, camera + kCamCar, record);
        put(app, camera + kCamTarget, record);
        copyFloat(app, camera + kCamPlace, record + kCarPlace);
        copyFloat(app, camera + kCamPlace + 4, at(app, camera + kCamCar) + kCarPlace + 4);
        copyFloat(app, camera + kCamPlace + 8, at(app, camera + kCamCar) + kCarPlace + 8);
        const reg32 from = at(app, camera + kCamCar) + kCarOrientation;
        for (reg32 k = 0; k < 0x24; k += 4)
            put(app, camera + kCamOrientation + k, at(app, from + k));
    }
    const Calling calling(cpu);
    return putCameraMode(app, cpu, index, 3);
}

}  // namespace

/* 0x4215c0: attach player eax's cameras to car edx. */
x86::reg32 attachCameras(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax, car = args.edx] { return attach(app, cpu, player, car); });
    return args.eax;
}

namespace
{

// A camera's mode's constraints: the head's turn (1 by the steering, else by the other axis), the
// track's walls, the other cars, looking at its target.
constexpr reg32 kCamHead = 0x104, kCamWalls = 0xf4, kCamCars = 0xf8, kCamLooks = 0x108;
// The identity orientation.
constexpr reg32 kIdentity = 0x564498;

/* 0x421730's: the cockpit camera's head turned with the car's steering: the steering (its physics'
 * +0x920 and +0x924, by the head's mode) scaled (0x536e0c, 0x536e10; more past 60 at +0x5a4,
 * compared as bits as the code does), the turn's matrices about two axes (of the identity) into the
 * frame, multiplied into the camera's orientation (0x4e0280); eax as the last call leaves it.
 * Nullopt when the game stopped. */
std::optional<reg32> turnHead(App* app, CPU& cpu, reg32 camera)
{
    const Frame frame(cpu, 0x10, 0x74);
    const reg32 e = frame.base;
    const reg32 physics = at(app, camera + kCamCar);
    reg32 a, b, other;
    if (at(app, camera + kCamHead) == 1)
    {
        other = b = at(app, physics + 0x924);
        a = bitsOf(float(fp::mul(f32(app, physics + 0x920), f32(app, 0x536e0c))));
    }
    else
    {
        other = a = at(app, physics + 0x920);
        b = bitsOf(float(fp::mul(f32(app, physics + 0x924), f32(app, 0x536e10))));
    }
    if (sreg32(at(app, physics + 0x5a4)) > 0x42700000)
        b = bitsOf(float(fp::mul(floatIn(b), f32(app, 0x536e10))));
    // The turns' angles pushed where the code pushes them: simc's cores take their addresses.
    push(app, cpu, e - 0x74);
    push(app, cpu, b);
    if (!feCore<0x4ea820, frontend::simc::turnAboutLastRow>(app, cpu, kIdentity, cpu.esp, e - 0x74))
        return std::nullopt;
    cpu.esp += 8;
    push(app, cpu, e - 0x50);
    push(app, cpu, bitsOf(float(fp::add(floatIn(a), f32(app, camera + kCamRoll)))));
    if (!feCore<0x4ea680, frontend::simc::turnAboutFirstRow>(app, cpu, kIdentity, cpu.esp, e - 0x50))
        return std::nullopt;
    cpu.esp += 8;
    // The two turns' matrices multiplied.
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, e - 0x74, e - 0x50, e - 0x2c);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 left = guest::direct<0x4e0280, vertices::multiplyMatrixEax>(app, cpu, e - 0x2c, camera + kCamOrientation,
                                                                          camera + kCamOrientation);
    if (cpu.terminate)
        return std::nullopt;
    return left;
}

}  // namespace

/* 0x421730: turnHead for camera eax. */
x86::reg32 headTurn(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax] { return turnHead(app, cpu, camera); });
    return args.eax;
}

/* 0x4217e0's: the track's node player's camera keeps to: of the active camera -- for a blend
 * the camera of the mode it goes to -- its own node, or where that is negative its +0x84. */
reg32 nodeOfCamera(App* app, reg32 player)
{
    const reg32 camera = activeMode(app, player) == kBlend
                             ? cameraOfMode(app, player, at(app, activeCamera(app, player) + kCamBlendTarget))
                             : activeCamera(app, player);
    const reg32 node = at(app, camera + kCamNode);
    return sreg32(node) >= 0 ? node : at(app, camera + 0x84);
}

/* 0x4217e0: nodeOfCamera for player eax. */
x86::reg32 cameraNode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return nodeOfCamera(app, player); });
    return args.eax;
}

namespace
{

/* 0x421880's: camera's place on the track for its place: the place in the track's fixed point
 * (scaled by 0x536e14, in the frame), the car's place on the track (its physics' +0xc, 30 dwords)
 * as the start, then found (0x41e420) and its node (0x41f6e0); eax as the last call leaves it.
 * Nullopt when the game stopped. */
std::optional<reg32> findTrackPlace(App* app, CPU& cpu, reg32 camera, reg32 place)
{
    const Frame frame(cpu, 0x14, 0xc);
    const reg32 fixed = frame.base - 0xc;
    for (const reg32 k : {0u, 4u, 8u})
        put(app, fixed + k, reg32(fp::toInteger<sreg32>(fp::mul(f32(app, place + k), f64(app, 0x536e14)))));
    const reg32 from = at(app, camera + kCamCar) + 0xc, trackPlace = camera + kCamTrackPlace;
    for (reg32 k = 0; k < 0x78; k += 4)
        put(app, trackPlace + k, at(app, from + k));
    guest::direct<0x41e420, trackdata::followToNearest>(app, cpu, fixed, trackPlace);
    if (cpu.terminate)
        return std::nullopt;
    // Its eax is what findTrackPlace leaves.
    const std::optional<reg32> node = guest::direct<0x41f6e0, trackdata::bringPlace>(app, cpu, fixed, trackPlace, 1);
    if (cpu.terminate)
        return std::nullopt;
    return node;
}

}  // namespace

/* 0x421880: findTrackPlace for camera eax and place edx. */
x86::reg32 cameraTrackPlace(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] { return findTrackPlace(app, cpu, camera, place); });
    return args.eax;
}

namespace
{

// A car's: shown (+0x89), wrecked (+0x88), its axes (+0xc0, +0xcc, +0xd8), its half sizes
// (+0x114..), its size (+0x120).
constexpr reg32 kCarShown = 0x89, kCarWrecked = 0x88, kCarHalfSizes = 0x114, kCarSize = 0x120;

/* fcom then fnstsw ax: eax with its low word the compare's status word (c3, c2 and c0, as the
 * game's arithmetic has it, fp::compare). */
reg32 statusInto(reg32 eax, const Float& a, const Float& b)
{
    return (eax & 0xffff0000) | fp::compare(a, b);
}

/* 0x4e01f0 (vertices'): the dot product of the vectors. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* What the dot product by address leaves: the product, off the x87's stack, and its eax. */
struct Product
{
    Float value;
    reg32 eax;
};

/* The same, for a caller that keeps the eax it leaves (the vector a it was handed). */
std::optional<Product> dotByAddress(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Product{Float(product), a};
}

/* An x87 fabs in the code's two steps: only a value under 0 is negated, the rest kept to the bit. */
float sizeOf(float value)
{
    return 0.0 > Float(value) ? float(-Float(value)) : value;
}

/* 0x421900's: the place kept out of the cars: of each of the race's cars shown and not wrecked
 * near enough (the distance to it against its size and a margin), the place in the car's axes;
 * inside the box of its half sizes (plus 0x536e24) and inside the ellipse of its sides, it is
 * pushed out sideways onto it (a square root) and back into the world by the car's axes
 * transposed.  eax as the code leaves it (the compares' status words among it).  Nullopt when the
 * game stopped. */
std::optional<reg32> pushOutOfCars(App* app, CPU& cpu, reg32 place)
{
    const Frame frame(cpu, 0x18, 0x88);
    const reg32 e = frame.base - 0x82;
    const reg32 way = e + 0x2a, axes = e - 6, pushed = e + 0x36;
    reg32 left = place;
    for (reg32 i = 0; sreg32(i) < sreg32(at(app, kCarCount)); ++i)
    {
        const reg32 car = at(app, kCars + i * 4);
        const reg8 shown = byte(app, car + kCarShown);
        left = (left & 0xffff00ff) | reg32(shown) << 8;
        if (shown == 0 || byte(app, car + kCarWrecked) != 0)
            continue;
        const Float reach = fp::add(f32(app, car + kCarSize), f64(app, 0x536e1c));
        const auto measured = feCore<0x4965b0, frontend::sima::quickDistanceOf>(app, cpu, car + kCarPlace, place);
        if (!measured)
            return std::nullopt;
        const Float distance = measured->value;
        left = statusInto(measured->eax, distance, reach);
        if (distance > reach)
            continue;
        for (const reg32 k : {0u, 4u, 8u})
            putFloat(app, way + k, float(fp::sub(f32(app, place + k), f32(app, car + kCarPlace + k))));
        std::optional<Float> d = dot(app, cpu, way, car + kCarOrientation);
        if (!d)
            return std::nullopt;
        const float a = float(*d);
        if (!(d = dot(app, cpu, way, car + kCarOrientation + 0xc)))
            return std::nullopt;
        const float b = float(*d);
        if (!(d = dot(app, cpu, way, car + kCarOrientation + 0x18)))
            return std::nullopt;
        const float c = float(*d);
        const Float margin = f32(app, 0x536e24);
        const float sx = float(fp::add(f32(app, car + kCarHalfSizes), margin));
        const float sy = float(fp::add(f32(app, car + kCarHalfSizes + 4), margin));
        const float sz = float(fp::add(margin, f32(app, car + kCarHalfSizes + 8)));
        const float sizeA = sizeOf(a), sizeB = sizeOf(b), sizeC = sizeOf(c);
        left = bitsOf(sizeC);
        // Inside the box: each size under the box's (a NaN counts as under).
        left = statusInto(left, sizeA, sx);
        if (Float(sizeA) >= Float(sx))
            continue;
        left = statusInto(left, sizeB, sy);
        if (Float(sizeB) >= Float(sy))
            continue;
        left = statusInto(left, sizeC, sz);
        if (Float(sizeC) >= Float(sz))
            continue;
        // The side it is pushed out to.
        left = statusInto(left, 0.0, b);
        const bool positive = !(0.0 >= Float(b));
        if (positive)
            left = 0x3ff00000;
        const float side = positive ? 1.0f : -1.0f;
        const Float across = fp::div(fp::mul(Float(c), Float(c)), fp::mul(Float(sz), Float(sz)));
        const Float ahead = fp::div(fp::mul(Float(a), Float(a)), fp::mul(Float(sx), Float(sx)));
        const float rest = float(fp::sub(1.0, fp::add(across, ahead)));
        left = statusInto(left, 0.0, rest);
        if (0.0 > Float(rest))
            continue;
        const std::optional<double> rooted =
            feCore<0x4ea920, frontend::simc::squareRootOf>(app, cpu, bitsOf(float(fp::mul(fp::mul(Float(sy), Float(sy)), Float(rest)))));
        if (!rooted)
            return std::nullopt;
        const float root = float(*rooted);
        left = statusInto(left, sizeB, root);  // the root's eax: the one it is handed
        if (Float(sizeB) >= Float(root))
            continue;
        put(app, pushed, bitsOf(a));
        put(app, pushed + 8, bitsOf(c));
        putFloat(app, pushed + 4, float(fp::mul(Float(root), Float(side))));
        if (!feCore<0x4e0110, frontend::simd::transpose>(app, cpu, car + kCarOrientation, axes))
            return std::nullopt;
        if (!(d = dot(app, cpu, pushed, axes)))
            return std::nullopt;
        const float x = float(*d);
        if (!(d = dot(app, cpu, pushed, axes + 0xc)))
            return std::nullopt;
        const float y = float(*d);
        const std::optional<Product> last = dotByAddress(app, cpu, pushed, axes + 0x18);
        if (!last)
            return std::nullopt;
        const float z = float(last->value);
        left = last->eax;
        putFloat(app, place, float(fp::add(f32(app, car + kCarPlace), Float(x))));
        putFloat(app, place + 4, float(fp::add(f32(app, car + kCarPlace + 4), Float(y))));
        putFloat(app, place + 8, float(fp::add(f32(app, car + kCarPlace + 8), Float(z))));
    }
    return left;
}

}  // namespace

/* 0x421900: pushOutOfCars for the place eax. */
x86::reg32 keepOutOfCars(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, place = args.eax] { return pushOutOfCars(app, cpu, place); });
    return args.eax;
}

namespace
{
// A plane's height (0x4968a0) by address: the code takes it off the x87's stack as wide as it is there,
// where vertices' core gives back a float.
// A camera's: following a car (+0x88) on a surface it knows (+0x8c), the car's side of the road
// (+0xe0), the car's four points (+0xc8..+0xd4, a vector each).
constexpr reg32 kCamFollows = 0x88, kCamKnown = 0x8c, kCamSide = 0xe0, kCamPoints = 0xc8;
// A track node's height (+4, an int), the node's size; each node's surface's own height.
constexpr reg32 kNodeSize = 0x24, kSurfaceHeights = 0x552e54;
// The surfaces with no ground of their own.
constexpr reg32 kNoSurface = 0, kWaterSurface = 0xe;

/* 0x4968a0: the height of the plane (12 floats) at the place, from the plane's point (ret 0x18):
 * the point's words and the place's on the stack, the point's first lowest. */
std::optional<Float> planeHeight(App* app, CPU& cpu, reg32 plane, reg32 place, reg32 point)
{
    const auto word = [&](reg32 address) { return app->getMemory<float>(address); };
    const float height = guest::direct<0x4968a0, vertices::groundHeightAt>(
        app, cpu, plane, word(point), word(point + 4), word(point + 8), word(place), word(place + 4), word(place + 8));
    if (cpu.terminate)
        return std::nullopt;
    return Float(height);
}

/* The camera's car's plane, from its four points copied into the frame at `points`, into `plane`
 * (0x496a50).  False when the game stopped. */
bool carPlane(App* app, CPU& cpu, reg32 camera, reg32 points, reg32 plane)
{
    for (reg32 i = 0; i < 4; ++i)
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, points + i * 0xc + k, at(app, at(app, camera + kCamPoints + i * 4) + k));
    return feCore<0x496a50, frontend::simd::frameOfTriangle>(app, cpu, points, plane).has_value();
}

/* 0x421b90's: the camera's place kept above the road: the road's height there -- the car's plane's
 * where the camera follows a car on a surface it knows (and not off its side), else the node's
 * (scaled by 0x536e34) -- plus the surface's own (by inTunnel's say, else 25.0) and a margin
 * (0x536e2c); the height that where it is not under it (the code's compare, kept as it is).
 * eax as the code leaves it, the height.  Nullopt when the game stopped. */
std::optional<reg32> keepRoadHeight(App* app, CPU& cpu, reg32 camera, reg32 place)
{
    const Frame frame(cpu, 0x14, 0x60);
    const reg32 trackPlace = camera + kCamTrackPlace;
    const reg32 node = at(app, trackPlace + 0x10);
    const reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, trackPlace, 1) & 0x3f;
    if (cpu.terminate)
        return std::nullopt;
    // inTunnel of the place's words and the node (the place's y was the register it was pushed through: not read).
    const std::optional<reg32> tunnel = guest::direct<0x420bf0, trackdata::placeInTunnel>(app, cpu, place, node);
    if (!tunnel)
        return std::nullopt;
    const float own = *tunnel != 0 ? floatOf(at(app, at(app, kSurfaceHeights) + node * 4)) : 25.0f;
    Float height;
    if (at(app, camera + kCamFollows) != 0 && at(app, camera + kCamKnown) != 0 && at(app, camera + kCamSide) == 0
        && kind != kNoSurface && kind != kWaterSurface)
    {
        const reg32 plane = frame.base - 0x30;
        if (!carPlane(app, cpu, camera, frame.base - 0x60, plane))
            return std::nullopt;
        const std::optional<Float> h = planeHeight(app, cpu, plane, place, at(app, camera + kCamPoints));
        if (!h)
            return std::nullopt;
        height = *h;
    }
    else
        height = fp::mul(Float(sreg32(at(app, at(app, kNodes) + node * kNodeSize + 4))), f64(app, 0x536e34));
    const float limit = float(fp::add(fp::add(height, Float(own)), f64(app, 0x536e2c)));
    const reg32 result = f32(app, place + 4) >= Float(limit) ? bitsOf(limit) : at(app, place + 4);
    put(app, place + 4, result);
    return result;
}

}  // namespace

/* 0x421b90: keepRoadHeight for camera eax and place edx. */
x86::reg32 keepAboveRoad(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] { return keepRoadHeight(app, cpu, camera, place); });
    return args.eax;
}

namespace
{

// A car's physics' sides of the road (+0x2c, its halves at +4 and +6).
constexpr reg32 kPhysSides = 0x2c;

using Vector = std::array<reg32, 3>;

/* 0x421cc0's: the camera's place kept on the track, seen from the point `from`: unless the camera
 * may leave it (its car's side of the road, or a car on a surface it knows), the way from the
 * point (the near end) to the place (the far end) halved 10 times (0x536e3c), each middle tried on
 * the track (0x41e990, its place in the track's fixed point by 0x536e44, the camera's track place
 * copied into the frame) and on a known surface (0x4207e0); the place the near end, and its track
 * place found.  eax as the code leaves it.  Nullopt when the game stopped. */
std::optional<reg32> keepPlaceOnTrack(App* app, CPU& cpu, reg32 camera, reg32 place, const Vector& from)
{
    const Frame frame(cpu, 0x14, 0xbc);
    const reg32 e = frame.base - 0x7e;
    const reg32 fixed = e + 0x52, track = e - 0x3e;
    const reg32 trackPlace = camera + kCamTrackPlace;
    Vector nearEnd = from;
    reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, trackPlace, 1) & 0x3f;
    if (cpu.terminate)
        return std::nullopt;
    Vector farEnd = {at(app, place), at(app, place + 4), at(app, place + 8)};
    Vector middle = farEnd, last;
    for (reg32 k = 0; k < 0x78; k += 4)
        put(app, track + k, at(app, trackPlace + k));
    const reg32 side = at(app, camera + kCamSide);
    if (side != 0)
    {
        const reg32 physics = at(app, camera + kCamCar);
        const reg32 sides = at(app, physics + kPhysSides);
        if (sides != 0)
        {
            if (side == 1 && x86::sreg16(app->getMemory<reg16>(sides + 4)) >= 0)
                return physics;
            if (at(app, camera + kCamSide) == 2)
            {
                const reg32 again = at(app, at(app, camera + kCamCar) + kPhysSides);
                if (x86::sreg16(app->getMemory<reg16>(again + 6)) >= 0)
                    return again;
            }
        }
    }
    else if (at(app, camera + kCamFollows) != 0 && at(app, camera + kCamKnown) != 0 && kind != kNoSurface
             && kind != kWaterSurface)
        return kind;
    const Float half = f64(app, 0x536e3c), toFixed = f64(app, 0x536e44);
    for (int step = 0; step < 10; ++step)
    {
        const Float x = fp::mul(fp::add(floatIn(farEnd[0]), floatIn(nearEnd[0])), half);
        const Float y = fp::mul(fp::add(floatIn(farEnd[1]), floatIn(nearEnd[1])), half);
        const Float z = fp::mul(half, fp::add(floatIn(farEnd[2]), floatIn(nearEnd[2])));
        last = middle;
        middle = {bitsOf(float(x)), bitsOf(float(y)), bitsOf(float(z))};
        put(app, fixed, reg32(fp::toInteger<sreg32>(fp::mul(x, toFixed))));
        put(app, fixed + 4, reg32(fp::toInteger<sreg32>(fp::mul(floatIn(middle[1]), toFixed))));
        put(app, fixed + 8, reg32(fp::toInteger<sreg32>(fp::mul(floatIn(middle[2]), toFixed))));
        const std::optional<trackdata::Searched> searched =
            guest::direct<0x41e990, trackdata::searchQuad>(app, cpu, fixed, track);
        if (!searched)
            return std::nullopt;
        const reg32 onTrack = searched->found;
        if (onTrack != 0)
        {
            kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, track, 1) & 0x3f;
            if (cpu.terminate)
                return std::nullopt;
        }
        if (onTrack == 0 || kind == kNoSurface || kind == kWaterSurface || at(app, track + 0x14) == 0
            || at(app, track + 0x18) == 0)
            farEnd = middle;
        else
        {
            nearEnd = middle;
            farEnd = last;
            middle = last;
        }
    }
    for (reg32 k = 0; k < 3; ++k)
        put(app, place + k * 4, nearEnd[k]);
    const Calling calling(cpu);
    return findTrackPlace(app, cpu, camera, place);
}

}  // namespace

/* 0x421cc0: keepPlaceOnTrack for camera eax, place edx, from the point on the stack (ret 0xc). */
x86::reg32 keepOnTrack(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(
        cpu, args,
        [&, camera = args.eax, place = args.edx, from = Vector{word(app, cpu, 1), word(app, cpu, 2), word(app, cpu, 3)}] {
            return keepPlaceOnTrack(app, cpu, camera, place, from);
        },
        0xc);
    return args.eax;
}

namespace
{

/* sar r, 0x18; shl r, 9: a node's signed byte at its top, as the code
 * scales it. */
reg32 nodeByte(reg32 r)
{
    return reg32(sreg32(r) >> 0x18) << 9;
}

/* 0x421e80's and 0x421f90's body: the camera's place moved by the node's sideways vector (its
 * bytes at +0x15.., scaled by `scale` and `across`) -- toward the wall on one side, away on the
 * other -- its track place found from there, kept on the track from the moved-toward point, and
 * moved back; eax as the code leaves it.  Nullopt when the game stopped.  The node's bytes are
 * ints times constants, never NaN. */
std::optional<reg32> keepFromWall(App* app, CPU& cpu, reg32 camera, reg32 place, reg32 scale, reg32 across,
                                  bool right)
{
    const Frame frame(cpu, 0x14, 0x28);
    const reg32 moved = frame.base - 0x10;
    // fsub on the right's side where the left's has fadd, and the other way.
    const auto toward = [&](const Float& v, const Float& m) { return right ? fp::sub(v, m) : fp::add(v, m); };
    const auto away = [&](const Float& v, const Float& m) { return right ? fp::add(v, m) : fp::sub(v, m); };
    const reg32 record = at(app, kNodes) + at(app, camera + 0x84) * kNodeSize;
    const Float a = f64(app, scale), b = f64(app, across);
    const auto byteAt = [&](reg32 k) { return Float(sreg32(nodeByte(at(app, record + k)))); };
    const float dx = float(fp::mul(fp::mul(byteAt(0x15), a), b));
    const float dy = float(fp::mul(fp::mul(byteAt(0x16), a), b));
    const float dz = float(fp::mul(b, fp::mul(a, byteAt(0x17))));
    const Float px = f32(app, place), py = f32(app, place + 4), pz = f32(app, place + 8);
    const Vector towardPoint = {bitsOf(float(toward(px, dx))), bitsOf(float(toward(py, dy))),
                                bitsOf(float(toward(pz, dz)))};
    putFloat(app, moved, float(away(px, dx)));
    putFloat(app, moved + 4, float(away(py, dy)));
    putFloat(app, moved + 8, float(away(pz, dz)));
    {
        const Calling calling(cpu);
        if (!findTrackPlace(app, cpu, camera, moved))
            return std::nullopt;
    }
    {
        const Calling calling(cpu, 3);
        if (!keepPlaceOnTrack(app, cpu, camera, moved, towardPoint))
            return std::nullopt;
    }
    putFloat(app, place, float(toward(f32(app, moved), dx)));
    putFloat(app, place + 4, float(toward(f32(app, moved + 4), dy)));
    putFloat(app, place + 8, float(toward(f32(app, moved + 8), dz)));
    const Calling calling(cpu);
    return findTrackPlace(app, cpu, camera, place);
}

}  // namespace

/* 0x421e80: camera eax's place edx kept from the track's wall on one side (keepFromWall, by
 * 0x536e4c and 0x536e54). */
x86::reg32 keepFromRightWall(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] {
        return keepFromWall(app, cpu, camera, place, 0x536e4c, 0x536e54, true);
    });
    return args.eax;
}

/* 0x421f90: and on the other (0x536e5c, 0x536e64). */
x86::reg32 keepFromLeftWall(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] {
        return keepFromWall(app, cpu, camera, place, 0x536e5c, 0x536e64, false);
    });
    return args.eax;
}

namespace
{

// A node's bytes the ground's plane is made of, scaled by 0x536e74.
constexpr reg32 kGroundScale = 0x536e74;

/* 0x4220a0's: the camera's place kept above the ground under it: the car's plane's height there
 * where it follows a car on a known surface, else the node's plane's -- its place and two of its
 * vectors from its bytes (scaled by 0x536e74), into the frame -- plus 0x536e6c: no lower than that.
 * eax as the code leaves it, the height.  Nullopt when the game stopped.  The node's numbers are
 * ints times a constant, so never NaN: the products' order does not matter, but is kept. */
std::optional<reg32> keepGroundHeight(App* app, CPU& cpu, reg32 camera, reg32 place)
{
    const Frame frame(cpu, 0x14, 0x6c);
    const reg32 e = frame.base;
    const reg32 trackPlace = camera + kCamTrackPlace;
    const reg32 node = at(app, trackPlace + 0x10);
    const reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, trackPlace, 1) & 0x3f;
    if (cpu.terminate)
        return std::nullopt;
    const reg32 plane = e - 0x3c;
    std::optional<Float> height;
    if (at(app, camera + kCamFollows) != 0 && at(app, camera + kCamKnown) != 0 && kind != kNoSurface
        && kind != kWaterSurface)
    {
        if (!carPlane(app, cpu, camera, e - 0x6c, plane))
            return std::nullopt;
        height = planeHeight(app, cpu, plane, place, at(app, camera + kCamPoints));
    }
    else
    {
        const reg32 record = at(app, kNodes) + node * kNodeSize;
        const Float scale = f64(app, kGroundScale);
        const auto byteAt = [&](reg32 k) { return Float(sreg32(nodeByte(at(app, record + k)))); };
        const auto intAt = [&](reg32 k) { return Float(sreg32(at(app, record + k))); };
        putFloat(app, e - 0x3c, float(fp::mul(byteAt(0x15), scale)));
        putFloat(app, e - 0x38, float(fp::mul(byteAt(0x16), scale)));
        putFloat(app, e - 0x34, float(fp::mul(byteAt(0x17), scale)));
        const Float x = fp::mul(intAt(0), scale);
        putFloat(app, e - 0x30, float(fp::mul(byteAt(0xd), scale)));
        const Float y = fp::mul(intAt(4), scale);
        putFloat(app, e - 0x2c, float(fp::mul(byteAt(0xe), scale)));
        const Float z = fp::mul(intAt(8), scale);
        const Float f = fp::mul(scale, byteAt(0xf));
        putFloat(app, e - 0x18, float(x));
        putFloat(app, e - 0x14, float(y));
        putFloat(app, e - 0x10, float(z));
        putFloat(app, e - 0x28, float(f));
        putFloat(app, e - 0x24, float(fp::mul(byteAt(0x11), scale)));
        putFloat(app, e - 0x20, float(fp::mul(byteAt(0x12), scale)));
        putFloat(app, e - 0x1c, float(fp::mul(scale, byteAt(0x13))));
        height = planeHeight(app, cpu, plane, place, e - 0x18);
    }
    if (!height)
        return std::nullopt;
    const float limit = float(fp::add(*height, f64(app, 0x536e6c)));
    const reg32 result = !(f32(app, place + 4) > Float(limit)) ? bitsOf(limit) : at(app, place + 4);
    put(app, place + 4, result);
    return result;
}

}  // namespace

/* 0x4220a0: keepGroundHeight for camera eax and place edx. */
x86::reg32 keepAboveGround(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] { return keepGroundHeight(app, cpu, camera, place); });
    return args.eax;
}

namespace
{

constexpr reg32 kUpVector = 0x4212d0;
// Each player's camera that looks at another's turn (mode 10).
constexpr reg32 kTurnCameras = 0x552f18;

/* 0x422290's: the camera made to look at its target from the place: the ways from the target's
 * place to the place and to the camera's look-at point (+0x44) (0x4e0000), the second normalized
 * as the camera's forward (0x4e01c0), its dot product with the up (0x4e0210, into the frame), the
 * up (0x4212d0, or the camera's own +0x38 where the forward is too steep, 0x536e7c) crossed with
 * it into the side and the up
 * (0x4e01a0, 0x4e0160); then the camera's roll (+0x50, 0x4ea680) and for mode 10 its car's turn
 * (0x4eaab0).  eax as the code leaves it.  Nullopt when the game stopped. */
std::optional<reg32> lookAt(App* app, CPU& cpu, reg32 camera, reg32 place)
{
    const Frame frame(cpu, 0x14, 0x40);
    const reg32 e = frame.base;
    const reg32 forward = camera + 0x44, side = camera + kCamOrientation, up = camera + 0x38;
    const reg32 upWay = e - 0x40, fromTarget = e - 0x34, target = e - 0x28, length = e - 0x1c;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, upWay + k, at(app, kUpVector + k));
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, target + k, at(app, at(app, camera + kCamTarget) + kCarPlace + k));
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, target, place, fromTarget);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, target, place, forward);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, forward, forward);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4e0210, vertices::dotProductsOf>(app, cpu, 1, upWay, forward, length);
    if (cpu.terminate)
        return std::nullopt;
    // The dot product, as the call left it in the frame.
    if (Float(sizeOf(floatOf(at(app, length)))) > f64(app, 0x536e7c))
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, upWay + k, at(app, up + k));
    if (!feCore<0x4e01a0, frontend::simd::unitCross>(app, cpu, upWay, forward, side)
        || !feCore<0x4e0160, frontend::simd::cross>(app, cpu, forward, side, up))
        return std::nullopt;
    // The roll pushed where the code pushes it: simc's core takes its address.
    push(app, cpu, side);
    push(app, cpu, at(app, camera + kCamRoll));
    if (!feCore<0x4ea680, frontend::simc::turnAboutFirstRow>(app, cpu, side, cpu.esp, side))
        return std::nullopt;
    cpu.esp += 8;
    if (at(app, camera + kCamMode) != 0xa)
        return camera;
    // The orientation in edx, as the roll's call loaded it: the side's, the camera's.
    return feCore<0x4eaab0, frontend::simd::rotateY>(
        app, cpu, side, at(app, at(app, kTurnCameras + at(app, camera + kCamPlayer) * 4) + kCamBlendTarget), side);
}

}  // namespace

/* 0x422290: lookAt for camera eax from its place edx. */
x86::reg32 lookAtTarget(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, place = args.edx] { return lookAt(app, cpu, camera, place); });
    return args.eax;
}

namespace
{

// A blend's record: its time, the mode it leaves (0 none), the mode it goes to, the start's
// orientation as a quaternion, its place's offset.
constexpr reg32 kBlendTime = 0x110, kBlendFrom = 0x118, kBlendStart = 0x120, kBlendOffset = 0x130;

/* 0x4223c0's: player's view (the orientation and the place) blended from the camera it leaves into
 * the one it takes: a logistic weight of its time (by 0x536e84 and 0x536e88, through fprem, f2xm1
 * and fscale), the orientations as quaternions -- the old one turned round first where `turned` is
 * 1 -- slerped back into the orientation, the places mixed (0x4dffb0, 0x4e0050) into the place.
 * eax as the last call leaves it, the orientation's where there is no blend.  Nullopt when the game
 * stopped. */
std::optional<reg32> blend(App* app, CPU& cpu, reg32 orientation, reg32 place, reg32 player, reg32 turned)
{
    const Frame frame(cpu, 0xc, 0xb0);
    const reg32 e = frame.base - 0x82;
    const reg32 record = at(app, kBlendRecord + player * 4);
    if (at(app, record + kBlendFrom) == 0 || at(app, record + kCamBlendTarget) == 0)
        return orientation;
    const Float time = fp::mul(fp::add(f32(app, record + kBlendTime), f32(app, 0x536e84)), f32(app, 0x536e88));
    const Float power = fp::mul(Float(1.4426950408889634), time);
    const Float raised = fp::scale(fp::add(1.0, fp::f2xm1(fp::rem(power, 1.0))), power);
    const float weight = float(fp::div(1.0, fp::add(raised, 1.0)));
    const float rest = float(fp::sub(1.0, Float(weight)));
    const reg32 quat = e + 0x3a, carQuat = e + 0x2a, mixed = e + 0x1a;
    for (reg32 k = 0; k < 0x10; k += 4)
        put(app, quat + k, at(app, record + kBlendStart + k));
    if (turned == 1)
    {
        if (!feCore<0x4b7590, frontend::simc::matrixFromQuat>(app, cpu, quat, e - 0xa))
            return std::nullopt;
        turnedAround(app, e - 0xa, e - 0x2e);
        if (!feCore<0x4b73c0, frontend::simc::quatFromMatrix>(app, cpu, e - 0x2e, quat))
            return std::nullopt;
    }
    if (!feCore<0x4b73c0, frontend::simc::quatFromMatrix>(app, cpu, orientation, carQuat)
        || !feCore<0x4b7300, frontend::simc::quatNearer>(app, cpu, quat, carQuat))
        return std::nullopt;
    if (!feCore<0x4b71e0, frontend::simc::quatSlerp>(app, cpu, quat, carQuat, bitsOf(weight), mixed)
        || !feCore<0x4b7590, frontend::simc::matrixFromQuat>(app, cpu, mixed, orientation))
        return std::nullopt;
    const reg32 target = cameraOfMode(app, player, at(app, record + kCamBlendTarget));
    const reg32 offset = e + 0x62, scaledOffset = e + 0x4a, scaledPlace = e + 0x56;
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, record + kBlendOffset,
                                                               at(app, target + kCamCar) + kCarPlace, offset);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, offset, rest, scaledOffset);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, place, weight, scaledPlace);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 left = guest::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(app, cpu, 1, scaledOffset, scaledPlace, place);
    if (cpu.terminate)
        return std::nullopt;
    return left;
}

}  // namespace

/* 0x4223c0: blend for the orientation eax and the place edx of player ebx, turned round where ecx
 * is 1. */
x86::reg32 blendCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, orientation = args.eax, place = args.edx, player = args.ebx, turned = args.ecx] {
        return blend(app, cpu, orientation, place, player, turned);
    });
    return args.eax;
}

namespace
{

/* 0x422520's: the camera placed for the frame: its head turned if it turns, its place to `into`
 * and onto the track, then kept from the walls, on the track (from its car's place), above the
 * road and the ground, out of the cars, looking at its target, as its mode says; its orientation
 * to `matrix` -- its car's turned round where `turned` is 1, else its own -- and its track place
 * found again.  eax as that leaves it.  Nullopt when the game stopped. */
std::optional<reg32> place(App* app, CPU& cpu, reg32 camera, reg32 matrix, reg32 into, reg32 turned)
{
    const Frame frame(cpu, 0xc, 8);
    if (at(app, camera + kCamHead) != 0)
    {
        const Calling calling(cpu);
        if (!turnHead(app, cpu, camera))
            return std::nullopt;
    }
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, into + k, at(app, camera + kCamPlace + k));
    {
        const Calling calling(cpu);
        if (!findTrackPlace(app, cpu, camera, into))
            return std::nullopt;
    }
    if (at(app, camera + kCamWalls) != 0)
    {
        {
            const Calling calling(cpu);
            if (!keepFromWall(app, cpu, camera, into, 0x536e4c, 0x536e54, true))
                return std::nullopt;
        }
        {
            const Calling calling(cpu);
            if (!keepFromWall(app, cpu, camera, into, 0x536e5c, 0x536e64, false))
                return std::nullopt;
        }
        const reg32 car = at(app, camera + kCamCar);
        const Vector from = {at(app, car + kCarPlace), at(app, car + kCarPlace + 4), at(app, car + kCarPlace + 8)};
        {
            const Calling calling(cpu, 3);
            if (!keepPlaceOnTrack(app, cpu, camera, into, from))
                return std::nullopt;
        }
        {
            const Calling calling(cpu);
            if (!keepRoadHeight(app, cpu, camera, into))
                return std::nullopt;
        }
        const Calling calling(cpu);
        if (!keepGroundHeight(app, cpu, camera, into))
            return std::nullopt;
    }
    if (at(app, camera + kCamCars) != 0)
    {
        const Calling calling(cpu);
        if (!pushOutOfCars(app, cpu, into))
            return std::nullopt;
    }
    if (at(app, camera + kCamLooks) != 0)
    {
        const Calling calling(cpu);
        if (!lookAt(app, cpu, camera, into))
            return std::nullopt;
    }
    if (turned == 1)
        turnedAround(app, at(app, camera + kCamCar) + kCarOrientation, matrix);
    else
    {
        for (reg32 k = 0; k < 0x24; k += 4)
            put(app, matrix + k, at(app, camera + kCamOrientation + k));
    }
    const Calling calling(cpu);
    return findTrackPlace(app, cpu, camera, into);
}

}  // namespace

/* 0x422520: place for camera eax, the orientation to edx, the place to ebx, turned round where ecx
 * is 1. */
x86::reg32 placeCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax, matrix = args.edx, into = args.ebx, turned = args.ecx] {
        return place(app, cpu, camera, matrix, into, turned);
    });
    return args.eax;
}

namespace
{

/* 0x422610's: player's camera switched to the mode, blending: where the mode may follow the camera
 * in use (its +0x58 flags), with the same cars and the player's own car, the switch kept on the
 * blend's record (the mode it goes to, the start's orientation as a quaternion, its place's offset,
 * the time cleared, the mode left), the modes' sounds and the blend taken; else straight to the
 * mode (putCameraMode).  eax as the code leaves it: putCameraMode's, or the mode left.  Nullopt
 * when the game stopped. */
std::optional<reg32> switchTo(App* app, CPU& cpu, reg32 mode, reg32 player)
{
    const Frame frame(cpu, 0x14, 0x60);
    const reg32 e = frame.base;
    const reg32 record = at(app, kBlendRecord + player * 4);
    const reg32 inUse = cameraInUse(app, player);
    const reg32 current = modeOf(app, inUse);
    const reg32 owner = gameSettings(app).playMode == 1 ? player : gameSettings(app).localPlayer;
    const auto straight = [&] {
        const Calling calling(cpu);
        return putCameraMode(app, cpu, player, mode);
    };
    if (byte(app, cameraOfMode(app, player, current) + mode + 0x58) == 0)
        return straight();
    const reg32 next = cameraOfMode(app, player, mode);
    if (at(app, inUse + kCamCar) != at(app, next + kCamCar) || at(app, inUse + kCamTarget) != at(app, next + kCamTarget)
        || at(app, inUse + kCamTarget) != at(app, kCars + owner * 4) || current == kBlend)
        return straight();
    put(app, record + kCamBlendTarget, mode);
    if (current != 0x10 && current != 0x18 && !modeSound(app, cpu, current, player))
        return std::nullopt;
    const reg32 orientation = e - 0x60, fromPlace = e - 0x2c, toPlace = e - 0x20;
    {
        const Calling calling(cpu);
        if (!place(app, cpu, cameraOfMode(app, player, current), orientation, fromPlace, 0))
            return std::nullopt;
    }
    const reg32 start = record + kBlendStart;
    if (!feCore<0x4b73c0, frontend::simc::quatFromMatrix>(app, cpu, orientation, start)
        || !modeSound(app, cpu, mode, player))
        return std::nullopt;
    {
        const Calling calling(cpu);
        if (!place(app, cpu, cameraOfMode(app, player, mode), orientation, toPlace, 0))
            return std::nullopt;
    }
    const reg32 car = at(app, cameraOfMode(app, player, current) + kCamCar);
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, fromPlace, car + kCarPlace,
                                                                record + kBlendOffset);
    if (cpu.terminate
        || !feCore<0x4b73c0, frontend::simc::quatFromMatrix>(app, cpu, orientation, e - 0x3c)
        || !feCore<0x4b7300, frontend::simc::quatNearer>(app, cpu, e - 0x3c, start))
        return std::nullopt;
    {
        const Calling calling(cpu);
        if (!putCameraMode(app, cpu, player, kBlend))
            return std::nullopt;
    }
    put(app, record + kBlendTime, 0);
    put(app, record + kBlendFrom, current);
    return current;
}

}  // namespace

/* 0x422610: switchTo mode eax for player edx. */
x86::reg32 switchCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, mode = args.eax, player = args.edx] { return switchTo(app, cpu, mode, player); });
    return args.eax;
}

/* 0x4227e0's: for a view of mode 0 its player's active camera's zoom, where that is not 0. */
std::optional<reg32> ownZoom(App* app, reg32 view)
{
    if (at(app, view) != 0)
        return std::nullopt;
    const reg32 zoom = at(app, activeCamera(app, at(app, view + 4)) + kCamZoom);
    if ((zoom & 0x7fffffff) == 0)
        return std::nullopt;
    return zoom;
}

/* 0x4227e0: view eax's zoom onto the x87's stack, ownZoom or 1.0; eax the zoom's bits where it
 * is the camera's, as the code leaves it. */
x86::reg32 cameraZoom(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, view = args.eax] {
        const std::optional<reg32> own = ownZoom(app, view);
        args.floatSet = true;
        args.floatResult = double(Float(own ? floatOf(*own) : 1.0f));
        return own ? *own : view;
    });
    return args.eax;
}


namespace
{

/* The code's inlined product of the vector with the matrix (its columns) into `into`: three rows of
 * products summed. */
void turnByMatrix(App* app, const float (&v)[3], reg32 m, reg32 into)
{
    const auto c = [&](reg32 k) { return f32(app, m + k); };
    const Float x0 = fp::mul(Float(v[0]), c(0)), y0 = fp::mul(Float(v[0]), c(4)), z0 = fp::mul(Float(v[0]), c(8));
    const Float x1 = fp::add(x0, fp::mul(Float(v[1]), c(0xc))), y1 = fp::add(y0, fp::mul(Float(v[1]), c(0x10))),
                z1 = fp::add(z0, fp::mul(Float(v[1]), c(0x14)));
    const Float x = fp::add(x1, fp::mul(Float(v[2]), c(0x18))), y = fp::add(y1, fp::mul(Float(v[2]), c(0x1c))),
                z = fp::add(z1, fp::mul(Float(v[2]), c(0x20)));
    putFloat(app, into, float(x));
    putFloat(app, into + 4, float(y));
    putFloat(app, into + 8, float(z));
}

/* 0x422850's: the view's matrix: its orientation (+0xc) transposed to +0x3c (0x4e0110), its place
 * negated and turned by it into +0x30; eax its frame's base, the code's pointer past the vector.
 * Nullopt when the game stopped. */
std::optional<reg32> makeViewMatrix(App* app, CPU& cpu, reg32 view)
{
    const Frame frame(cpu, 0x14, 0xc);
    if (!feCore<0x4e0110, frontend::simd::transpose>(app, cpu, view + 0xc, view + 0x3c))
        return std::nullopt;
    const float v[3] = {float(-f32(app, view)), float(-f32(app, view + 4)), float(-f32(app, view + 8))};
    turnByMatrix(app, v, view + 0x3c, view + 0x30);
    return frame.base;
}

}  // namespace

/* 0x422850: makeViewMatrix for view eax. */
x86::reg32 viewMatrix(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, view = args.eax] { return makeViewMatrix(app, cpu, view); });
    return args.eax;
}

namespace
{

/* 0x4228e0's: player's view updated: where its camera is blending, the camera of the mode it goes
 * to stands in while it is placed; the active camera placed (`flag` its turn-round flag) into the
 * view's place and orientation, the blend applied; the view's matrix, and the view kept as the
 * player's where `flag` is 0.  eax as the code leaves it: the view's place among the players'
 * where it is kept, else the matrix's.  Nullopt when the game stopped. */
std::optional<reg32> updateView(App* app, CPU& cpu, reg32 player, reg32 view, reg32 flag)
{
    const Frame frame(cpu, 0x10, 0xc);
    const reg32 index = playerIndex(app, player);
    const reg32 inUse = cameraInUse(app, index);
    const bool blending = inUse != 0 && at(app, inUse + kCamMode) == kBlend;
    if (blending)
        put(app, kCamera + index * 4,
            cameraOfMode(app, index, at(app, at(app, kBlendRecord + index * 4) + kCamBlendTarget)));
    {
        const Calling calling(cpu);
        if (!place(app, cpu, activeCamera(app, index), view + 0xc, view, flag))
            return std::nullopt;
    }
    if (blending)
    {
        {
            const Calling calling(cpu);
            if (!blend(app, cpu, view + 0xc, view, index, flag))
                return std::nullopt;
        }
        put(app, kCamera + index * 4, inUse);
    }
    std::optional<reg32> matrix;
    {
        const Calling calling(cpu);
        if (!(matrix = makeViewMatrix(app, cpu, view)))
            return std::nullopt;
    }
    if (flag != 0)
        return matrix;
    for (reg32 k = 0; k < 0x60; k += 4)
        put(app, kViews + index * 0x60 + k, at(app, view + k));
    return index * 0x60;
}

}  // namespace

/* 0x4228e0: updateView for player eax, view edx, flag ebx. */
x86::reg32 updateCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax, view = args.edx, flag = args.ebx] { return updateView(app, cpu, player, view, flag); });
    return args.eax;
}

namespace
{

/* Where a view's place comes from: the camera, and its mode (or, for a blend, the mode it goes to). */
struct ViewSource
{
    reg32 camera, mode;
};

/* 0x4229c0's: player's view (0x5dd950, 0x60 bytes) copied to `to`, and its camera's place -- the
 * active camera's, or the camera of the mode a blend goes to -- over its first 12. */
ViewSource copyView(App* app, reg32 player, reg32 to)
{
    const reg32 index = playerIndex(app, player);
    for (reg32 k = 0; k < 0x60; k += 4)
        put(app, to + k, at(app, kViews + index * 0x60 + k));
    ViewSource source = {0, activeMode(app, index)};
    if (source.mode == kBlend)
    {
        source.mode = at(app, at(app, kBlendRecord + index * 4) + kCamBlendTarget);
        source.camera = cameraOfMode(app, index, source.mode);
    }
    else
        source.camera = activeCamera(app, index);
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, to + k, at(app, source.camera + kCamPlace + k));
    return source;
}

}  // namespace

/* 0x4229c0: copyView for player eax to edx; eax the camera and ebx its mode, as the code leaves
 * them (ebx no argument of its). */
x86::reg32 cameraView(App* app, x86::CPU& cpu, x86::Args& args)
{
    ViewSource source{};
    asThunk(cpu, args, [&, player = args.eax, to = args.edx] {
        source = copyView(app, player, to);
        return source.camera;
    });
    args.ebx = source.mode;
    return args.eax;
}

namespace
{

// The race's frames since the start; the replay's flags; the cockpit always shown; each player
// out of the race; the outside forced.
constexpr reg32 kRaceFrames = 0x7d3684, kReplayFlags = 0x7a3a58, kAlwaysCockpit = 0x559264, kPlayerOut = 0x79c4a0,
                kOutsideForced = 0x55fe30;

}  // namespace

/* 0x422aa0's: whether the player sees from inside its car: 320 frames into the race, the car
 * there and not one without a cockpit (+0x200 bit 2), no replay, and -- unless kAlwaysCockpit
 * says always -- not watching a replay (replayMode 2), the player not out, nothing forcing the
 * outside; then by the camera's mode (that of a blend's target), 1 to 6 (the table at 0x422a7c
 * sends each of them to the same place). */
bool insideView(App* app, reg32 player)
{
    const reg32 index = playerIndex(app, player);
    const reg32 car = at(app, cameraInUse(app, index) + kCamCar);
    if (!(sreg32(at(app, kRaceFrames)) >= 0x140 && car != 0 && (byte(app, car + 0x200) & 2) == 0
          && (app->getMemory<reg16>(kReplayFlags) & 0x180) == 0))
        return false;
    if (at(app, kAlwaysCockpit) != 0)
        return true;
    if (gameSettings(app).replayMode == 2 || at(app, kPlayerOut + index * 4) != 0 || at(app, kOutsideForced) != 0)
        return false;
    reg32 mode = activeMode(app, playerIndex(app, index));
    const reg32 camera = cameraInUse(app, index);
    if (camera != 0 && at(app, camera + kCamMode) == kBlend)
        mode = at(app, at(app, kBlendRecord + index * 4) + kCamBlendTarget);
    return mode - 1 <= 5;
}

/* 0x422aa0: insideView for player eax, 1 or 0. */
x86::reg32 cockpitShown(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return insideView(app, player) ? 1u : 0u; });
    return args.eax;
}

/* 0x422ba0's: the car player's camera rides with. */
reg32 carOfCamera(App* app, reg32 player)
{
    return at(app, cameraInUse(app, playerIndex(app, player)) + kCamCar);
}

/* 0x422ba0: carOfCamera for player eax. */
x86::reg32 cameraCar(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return carOfCamera(app, player); });
    return args.eax;
}

/* 0x422bd0's: the car player's camera looks at. */
reg32 targetOfCamera(App* app, reg32 player)
{
    return at(app, cameraInUse(app, playerIndex(app, player)) + kCamTarget);
}

/* 0x422bd0: targetOfCamera for player eax. */
x86::reg32 cameraTarget(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return targetOfCamera(app, player); });
    return args.eax;
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

namespace
{

// The zeros a camera's track place starts from (30 dwords among the code); the identity's place;
// each player's view's orientation and place; each player's camera of mode 16 for a car without
// its own; the race's settings' byte with the zoom's flag; the HUD's reset.
constexpr reg32 kZeros = 0x4212dc, kOrigin = 0x56448c, kWideView = 0x552ee0, kZoomFlags = 0x55eb2e;

/* 0x422cf0's: the race's cameras set up: each player's view the identity at the origin, the cycle
 * of modes built; for each player (two in a split screen) mode 0x10 taken, no override, and each
 * camera of modes 1 to 24 reset onto the player's car (the camera of mode 0x10 in use for a car of
 * +0x200 bit 0x20) with its mode's settings; the HUD's (0x41ffa0) and the chase's state cleared;
 * eax 0, as the code leaves it.  Nullopt when the game stopped. */
std::optional<reg32> setUpCameras(App* app, CPU& cpu)
{
    const Frame frame(cpu, 0x18, 0x7c);
    const reg32 split = gameSettings(app).playMode == 1 ? 1 : 0;
    reg32 zeros[0x1e];
    for (reg32 k = 0; k < 0x1e; ++k)
        zeros[k] = at(app, kZeros + k * 4);
    for (const reg32 view : {kViews, kViews + 0x60})
    {
        for (reg32 k = 0; k < 0x24; k += 4)
            put(app, view + 0xc + k, at(app, kIdentity + k));
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, view + k, at(app, kOrigin + k));
    }
    buildCycle(app);
    for (reg32 player = 0; sreg32(player) <= sreg32(split); ++player)
    {
        put(app, kSoundMark + player * 8, 0);
        put(app, kChanged + player * 4, 1);
        put(app, kSoundMark + 4 + player * 8, 0);
        {
            const Calling calling(cpu);
            if (!putCameraMode(app, cpu, player, 0x10))
                return std::nullopt;
        }
        {
            const Calling calling(cpu);
            if (!putOverrideMode(app, cpu, player, 0))
                return std::nullopt;
        }
        for (reg32 mode = 1; mode < 0x19; ++mode)
        {
            const reg32 camera = cameraOfMode(app, player, mode);
            const reg32 car = at(app, kPlayerCars + (split == 0 ? gameSettings(app).localPlayer : player) * 4);
            put(app, camera + kCamCar, car);
            put(app, camera + kCamTarget, car);
            if ((byte(app, car + 0x200) & 0x20) != 0)
                put(app, kCamera + player * 4, at(app, kWideView + player * 4));
            put(app, camera + 0x10, 0);
            put(app, camera + kCamMode, mode);
            put(app, camera + kCamPlayer, player);
            for (reg32 k = 0; k < 0x1e; ++k)
                put(app, camera + kCamTrackPlace + k * 4, zeros[k]);
            put(app, camera + kCamNode, 0xffffffff);
            const reg32 from = at(app, camera + kCamCar) + kCarOrientation;
            for (reg32 k = 0; k < 0x24; k += 4)
                put(app, camera + kCamOrientation + k, at(app, from + k));
            put(app, camera + kCamRoll, 0);
            copyFloat(app, camera + kCamPlace, at(app, camera + kCamCar) + kCarPlace);
            copyFloat(app, camera + kCamPlace + 4, at(app, camera + kCamCar) + kCarPlace + 4);
            const float z = float(f32(app, at(app, camera + kCamCar) + kCarPlace + 8));
            put(app, camera + 0x20, 0);
            put(app, camera + 0x24, 0);
            put(app, camera + 0x28, 0xc0400000);
            putFloat(app, camera + kCamPlace + 8, z);
            put(app, camera + kCamZoom, 0);
            // The flags of the modes it may hand over to, 25 bytes, cleared.
            for (reg32 k = 0; k < 0x19; ++k)
                putByte(app, camera + 0x58 + k, 0);
            for (const reg32 k : {0xf4u, 0xf8u, 0xfcu, 0x104u, 0x108u, 0x10cu, 0xf0u})
                put(app, camera + k, 0);
            const reg32 index = mode - 1;
            if (index == 15)
            {
                // Mode 16: its zoom by the race's settings.
                put(app, camera + 0x110, 0);
                if ((byte(app, kZoomFlags) & 0x20) != 0)
                    put(app, camera + 0x114, 4);
                else if (gameSettings(app).raceType == 0 && gameSettings(app).playMode != 4
                         && gameSettings(app).backwards == 0)
                    put(app, camera + 0x114, 0);
                else
                    put(app, camera + 0x114, gameSettings(app).backwards != 0 ? 5 : 4);
            }
            if (kModes[index] != nullptr)
                store(app, camera, kModes[index]);
        }
    }
    guest::direct<0x41ffa0, trackdata::setLanes>(app, cpu, 0x1e, 3);
    if (cpu.terminate)
        return std::nullopt;
    for (reg32 a = kChaseMark; a <= kChaseBand + 4; a += 4)
        put(app, a, 0);
    return 0;
}

}  // namespace

/* 0x422cf0: setUpCameras. */
x86::reg32 initCameras(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&] { return setUpCameras(app, cpu); });
    return args.eax;
}

namespace
{

/* 0x4238b0's: player's camera put to the mode: the change marked (unless either end is a blend,
 * or the mode the same); for modes 3 to 6 the mode's sound (with the player's mark around it
 * unless leaving a blend); the mode's camera in use; eax as the code leaves it, the player's
 * place among the cameras (its edx the mode's camera).  Nullopt when the game stopped. */
std::optional<reg32> putCameraMode(App* app, CPU& cpu, reg32 player, reg32 mode)
{
    const Frame frame(cpu, 0x14, 0);
    const reg32 current = activeMode(app, player);
    if (mode != kBlend && current != kBlend && current != mode)
        put(app, kChanged + player * 4, 1);
    if (mode == 4 || mode == 3 || mode == 6 || mode == 5)
    {
        if (current == kBlend)
        {
            if (!modeSound(app, cpu, mode, player))
                return std::nullopt;
        }
        else
        {
            put(app, kSoundMark + player * 8, 1);
            if (!modeSound(app, cpu, mode, player))
                return std::nullopt;
            put(app, kSoundMark + player * 8, 0);
        }
    }
    put(app, kCamera + player * 4, cameraOfMode(app, player, mode));
    return player * 4;
}

}  // namespace

/* 0x4238b0: putCameraMode for player eax and mode edx. */
x86::reg32 setCameraMode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax, mode = args.edx] { return putCameraMode(app, cpu, player, mode); });
    return args.eax;
}

namespace
{

/* 0x423960's: player's overriding camera put to the mode (0 none): the change marked as
 * putCameraMode does, the sound of modes 3 to 6 with the player's mark around it, the mode's camera
 * overriding; eax as putCameraMode's.  Nullopt when the game stopped. */
std::optional<reg32> putOverrideMode(App* app, CPU& cpu, reg32 player, reg32 mode)
{
    const Frame frame(cpu, 0x14, 0);
    const reg32 current = activeMode(app, player);
    const reg32 overriding = modeOf(app, overridingCamera(app, player));
    if ((mode != 0 || overriding != 0) && mode != kBlend && current != kBlend && current != mode)
        put(app, kChanged + player * 4, 1);
    if (mode == 4 || mode == 3 || mode == 6 || mode == 5)
    {
        put(app, kSoundMark + player * 8, 1);
        if (!modeSound(app, cpu, mode, player))
            return std::nullopt;
        put(app, kSoundMark + player * 8, 0);
    }
    put(app, kOverride + player * 4, cameraOfMode(app, player, mode));
    return player * 4;
}

}  // namespace

/* 0x423960: putOverrideMode for player eax and mode edx. */
x86::reg32 setOverrideMode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax, mode = args.edx] { return putOverrideMode(app, cpu, player, mode); });
    return args.eax;
}

namespace
{

/* 0x423a20's: the next of the four camera choices the player made, from the one after the last
 * (kCycleChoice, kept) or from the start: its mode, or 0 when none is left. */
reg32 nextChoice(App* app, bool fromStart)
{
    reg32 next = fromStart ? 0 : at(app, kCycleChoice);
    reg32 mode = 0;
    while (sreg32(next) < 4 && gameSettings(app).cameraViews[next] == 0)
        ++next;
    if (sreg32(next) < 4)
    {
        ++next;
        mode = gameSettings(app).cameraViews[next - 1];
    }
    put(app, kCycleChoice, next);
    return mode;
}

}  // namespace

/* 0x423a20: nextChoice, from the start where eax is not 0. */
x86::reg32 nextCycleMode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, fromStart = args.eax != 0] { return nextChoice(app, fromStart); });
    return args.eax;
}

namespace
{

/* 0x423a60's: the cycle of camera modes (kCycle, each mode's next): every mode to the first
 * choice (mode 2 without any), then each choice to the next. */
void buildCycle(App* app)
{
    const reg32 first = nextChoice(app, true);
    reg32 mode = first != 0 ? first : 2;
    for (reg32 each = 0; each < 0x19; ++each)
        put(app, kCycle + each * 4, mode);
    for (reg32 next; (next = nextChoice(app, false)) != 0; mode = next)
        put(app, kCycle + mode * 4, next);
}

}  // namespace

/* 0x423a60: buildCycle; eax 0 as the code leaves it. */
x86::reg32 buildCameraCycle(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&] {
        buildCycle(app);
        return 0u;
    });
    return args.eax;
}

namespace
{

/* 0x423ab0's: whether the mode is the next of any mode in the cycle. */
bool inCycle(App* app, reg32 mode)
{
    bool found = false;
    for (reg32 each = 0; each < 0x19; ++each)
        if (mode == at(app, kCycle + each * 4))
            found = true;
    return found;
}

}  // namespace

/* 0x423ab0: inCycle for mode eax, 1 or 0. */
x86::reg32 modeInCycle(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, mode = args.eax] { return inCycle(app, mode) ? 1u : 0u; });
    return args.eax;
}

namespace
{

/* 0x423ae0's: the mode kept as the player's last if it is in the cycle; whether it is. */
bool remember(App* app, reg32 player, reg32 mode)
{
    const bool found = inCycle(app, mode);
    if (found)
        gameSettings(app).cameraMode[player] = mode;
    return found;
}

}  // namespace

/* 0x423ae0: remember mode edx for player eax; eax 1 or 0. */
x86::reg32 rememberMode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax, mode = args.edx] { return remember(app, player, mode) ? 1u : 0u; });
    return args.eax;
}

/* 0x423b00's: player's camera's mode (activeMode). */
reg32 modeOfCamera(App* app, reg32 player)
{
    return activeMode(app, playerIndex(app, player));
}

/* 0x423b00: modeOfCamera for player eax. */
x86::reg32 cameraMode(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return modeOfCamera(app, player); });
    return args.eax;
}

namespace
{

/* 0x423b50's: player's camera put to the mode (putCameraMode), 0x5600ec cleared first; eax as
 * that leaves it.  Nullopt when the game stopped. */
std::optional<reg32> takeModeAnew(App* app, CPU& cpu, reg32 player, reg32 mode)
{
    const Frame frame(cpu, 0xc, 0);
    put(app, 0x5600ec, 0);
    const Calling calling(cpu);
    return putCameraMode(app, cpu, playerIndex(app, player), mode);
}

}  // namespace

/* 0x423b50: takeModeAnew for player eax and mode edx; edx the mode's camera as putCameraMode's
 * code leaves it (its code keeps ebx and ecx only; edx is no argument of its by its reading). */
x86::reg32 setModeAnew(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 player = args.eax, mode = args.edx;
    if (asThunk(cpu, args, [&] { return takeModeAnew(app, cpu, player, mode); }))
        args.edx = cameraOfMode(app, playerIndex(app, player), mode);
    return args.eax;
}

namespace
{

// The mode a network pursuit's camera goes to; each mode's for a cop's car.
constexpr reg32 kPursuitView = 0x5e1054, kCopViews = 0x552f90;
constexpr reg32 kChaseMode = 9;

/* 0x423b80's: player's camera brought back to what the player chose, where it looks at the
 * player's own car: a network pursuit's camera or a cop's (+0x2c4 3) to the mode for it, else on
 * to the next mode of the cycle (switchTo); a blend's target taken, a mode past 6 replaced, mode 9's
 * chase node picked, and the mode remembered.  eax as the code leaves it.  Nullopt when the game
 * stopped. */
std::optional<reg32> settle(App* app, CPU& cpu, reg32 player)
{
    const Frame frame(cpu, 0x18, 0);
    const reg32 index = playerIndex(app, player);
    const reg32 owner = gameSettings(app).playMode == 1 ? player : gameSettings(app).localPlayer;
    const reg32 inUse = cameraInUse(app, index);
    if (inUse != 0 && at(app, inUse + kCamMode) == 0x10)
        return 0x10;
    if (at(app, kCars + owner * 4) != at(app, cameraInUse(app, index) + kCamTarget))
        return cameraInUse(app, index);
    const auto anew = [&](reg32 mode) {
        const Calling calling(cpu);
        return takeModeAnew(app, cpu, index, mode);
    };
    bool settled = false, blendTaken = false;
    reg32 remembered = 0;
    if (sreg32(gameSettings(app).playMode) > 1 && at(app, cameraInUse(app, index) + kCamMode) == kPursuitMode)
    {
        if (!anew(at(app, kPursuitView)))
            return std::nullopt;
        settled = true;
    }
    if (!settled && gameSettings(app).playMode == 1)
    {
        const reg32 car = at(app, kPlayerCars + index * 4);
        if ((byte(app, car + 0x200) & 1) != 0 && at(app, car + 0x2c4) == 3)
        {
            if (!anew(at(app, kCopViews + modeOf(app, cameraInUse(app, index)) * 4)))
                return std::nullopt;
            settled = true;
        }
    }
    if (!settled && modeOf(app, cameraInUse(app, index)) != kBlend)
    {
        const reg32 next = at(app, kCycle + modeOf(app, cameraInUse(app, index)) * 4);
        {
            const Calling calling(cpu);
            if (!switchTo(app, cpu, next, index))
                return std::nullopt;
        }
        if (modeOf(app, cameraInUse(app, index)) == kBlend)
        {
            remembered = next;
            blendTaken = true;
        }
    }
    if (!blendTaken)
    {
        const reg32 blending = cameraInUse(app, index);
        if (blending != 0 && at(app, blending + kCamMode) == kBlend && !anew(at(app, blending + kCamBlendTarget)))
            return std::nullopt;
        const reg32 camera = cameraInUse(app, index);
        if (camera != 0 && sreg32(at(app, camera + kCamMode)) > 6)
        {
            const Calling calling(cpu);
            if (!putCameraMode(app, cpu, index, at(app, kPursuitView)))
                return std::nullopt;
        }
        if (modeOf(app, cameraInUse(app, index)) == kChaseMode)
            chaseNode(app, index);
        remembered = modeOf(app, cameraInUse(app, index));
    }
    return remember(app, index, remembered) ? 1 : 0;
}

}  // namespace

/* 0x423b80: settle player eax's camera. */
x86::reg32 settleCamera(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return settle(app, cpu, player); });
    return args.eax;
}

namespace
{

// The spots (0x1c bytes each: a place, a quaternion's halves at +0xa.., its node at +0x18) and their
// count; each player's camera that looks from them.
constexpr reg32 kSpots = 0x552e50, kSpotCount = 0x552e4c, kSpotCameras = 0x552f58;
constexpr reg32 kSpotSize = 0x1c, kSpotScale = 0x536e8c;

/* 0x423d90's: the spot nearest player's car along the track (by node, round the track), within 30
 * nodes; or -1 -- also where the car, seen from the spot's place and orientation (0x4e0000,
 * 0x4e01c0, 0x4b7590, 0x4b79c0, 0x4e0210, its result in the frame), is out of its view (its cosine
 * past 0.4, compared as bits as the code does).  Nullopt when the game stopped. */
std::optional<reg32> nearest(App* app, CPU& cpu, reg32 player)
{
    const Frame frame(cpu, 0x18, 0x60);
    const reg32 e = frame.base;
    const reg32 index = playerIndex(app, player);
    const reg32 node = at(app, at(app, at(app, kSpotCameras + index * 4) + kCamCar) + kPhysSlice);
    reg32 best = 0x1e, spot = 0xffffffff;
    for (reg32 i = 0; sreg32(i) < sreg32(at(app, kSpotCount)); ++i)
    {
        const reg32 apart = magnitude(node - at(app, at(app, kSpots) + i * kSpotSize + 0x18));
        const reg32 count = at(app, kNodeCount);
        const reg32 halfway = reg32(sreg32(count - reg32(sreg32(count) >> 31)) >> 1);
        const reg32 distance = sreg32(halfway) < sreg32(apart) ? at(app, kNodeCount) - apart : apart;
        if (sreg32(distance) < sreg32(best))
        {
            spot = i;
            best = distance;
        }
    }
    if (spot != 0xffffffff)
    {
        const Float scale = f64(app, kSpotScale);
        const reg32 spotAt = spot * kSpotSize;
        reg32 record = spotAt + at(app, kSpots);
        for (const reg32 k : {0u, 4u, 8u})
            putFloat(app, e - 0x2c + k, float(fp::mul(Float(sreg32(at(app, record + k))), scale)));
        const reg32 car = at(app, at(app, kSpotCameras + index * 4) + kCamCar);
        guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, e - 0x2c, car + kCarPlace, e - 0x20);
        if (cpu.terminate)
            return std::nullopt;
        guest::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, e - 0x20, e - 0x20);
        if (cpu.terminate)
            return std::nullopt;
        record = spotAt + at(app, kSpots);
        const auto half = [&](reg32 k) { return Float(sreg32(reg32(sreg32(at(app, record + k)) >> 0x10) << 2)); };
        for (const reg32 k : {0xau, 0xcu, 0xeu})
            putFloat(app, e - 0x3c + (k - 0xa) * 2, float(fp::mul(half(k), scale)));
        putFloat(app, e - 0x30, float(fp::mul(half(0x10), scale)));
        if (!feCore<0x4b7590, frontend::simc::matrixFromQuat>(app, cpu, e - 0x3c, e - 0x60)
            || !feCore<0x4b79c0, frontend::simc::matrixQuarterTurn>(app, cpu, e - 0x60))
            return std::nullopt;
        guest::direct<0x4e0210, vertices::dotProductsOf>(app, cpu, 1, e - 0x48, e - 0x20, e - 0x14);
        if (cpu.terminate)
            return std::nullopt;
        if (sreg32(at(app, e - 0x14)) > 0x3ecccccd)
            spot = 0xffffffff;
    }
    return spot;
}

}  // namespace

/* 0x423d90: the spot nearest player eax's car. */
x86::reg32 nearestSpot(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, player = args.eax] { return nearest(app, cpu, player); });
    return args.eax;
}

/* 0x423fb0's: the value scaled by 0x536e94 where the record's +4 is set, through a float. */
float scaledWhereSet(App* app, reg32 record, reg32 value)
{
    if (at(app, record + 4) != 0)
        return float(fp::mul(Float(floatOf(value)), f32(app, 0x536e94)));
    return floatOf(value);
}

/* 0x423fb0: scaledWhereSet for the record eax and the float on the stack (ret 4), onto the x87's
 * stack; eax the float's bits where the record's is not set, as the code leaves it. */
x86::reg32 scaledIfSet(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(
        cpu, args,
        [&, record = args.eax, value = word(app, cpu, 1)] {
            args.floatSet = true;
            args.floatResult = double(Float(scaledWhereSet(app, record, value)));
            return at(app, record + 4) == 0 ? value : record;
        },
        4);
    return args.eax;
}

namespace
{

/* 0x423fe0's: whether the low 16 bits of the value are not one run of set bits: each lowest bit
 * taken off in turn must be the one above the last. */
bool bitsNotOneRun(reg32 value)
{
    reg32 rest = (value - 1) & value;
    reg32 bit = value - rest;
    for (reg32 left = value; reg16(left) != 0; left = rest, bit += bit, rest &= rest - 1)
        if (reg32(reg16(left)) - reg32(reg16(rest)) != reg32(reg16(bit)))
            return true;
    return false;
}

}  // namespace

/* 0x423fe0: bitsNotOneRun of eax, 1 or 0. */
x86::reg32 bitsBroken(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, value = args.eax] { return bitsNotOneRun(value) ? 1u : 0u; });
    return args.eax;
}

namespace
{

/* 0x424030's: the vector damped by the weight, where that is not 0: by the car's slip (+0x4e8, at
 * most 3.0 by its bits, else -3.0 past 0x536e9c), through 0x536ea4 and 0x536ea8 -- its first and
 * last components by one factor, the middle by its square.  Returns eax as the code leaves it (the
 * car, or the compare's status word among it, or the slip's bits). */
reg32 damp(App* app, reg32 car, reg32 weight, reg32 vector)
{
    if ((weight & 0x7fffffff) == 0)
        return car;
    reg32 slipBits = at(app, car + 0x4e8);
    if (sreg32(slipBits) > 0x40400000)
        slipBits = 0x40400000;
    const float slip = floatOf(slipBits);
    const reg32 left = statusInto(car, Float(slip), f64(app, 0x536e9c));
    const bool over = Float(slip) > f64(app, 0x536e9c);
    const float s = over ? slip : -3.0f;
    const Float held = fp::mul(fp::sub(1.0, fp::mul(fp::add(Float(s), f32(app, 0x536ea4)), f32(app, 0x536ea8))),
                               Float(floatOf(weight)));
    const Float factor = fp::sub(1.0, held);
    const Float x = fp::mul(f32(app, vector), factor);
    const Float z = fp::mul(factor, f32(app, vector + 8));
    const Float y = fp::mul(fp::mul(factor, factor), f32(app, vector + 4));
    putFloat(app, vector, float(x));
    putFloat(app, vector + 8, float(z));
    putFloat(app, vector + 4, float(y));
    return over ? slipBits : left;
}

}  // namespace

/* 0x424030: damp, car eax, the weight and the vector on the stack (ret 8). */
x86::reg32 dampVector(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(
        cpu, args, [&, car = args.eax, weight = word(app, cpu, 1), vector = word(app, cpu, 2)] { return damp(app, car, weight, vector); },
        8);
    return args.eax;
}

namespace
{

/* 0x4240c0's and 0x424260's end: the camera's offset turned by its car's orientation into `turned`,
 * added to the car's place (in the frame at `carPlace`) into the camera's (0x4dffb0); its
 * orientation the car's, turned round for a player looking back; eax as the code leaves it: the
 * car's orientation, or the car.  Nullopt when the game stopped. */
std::optional<reg32> offsetFromCar(App* app, CPU& cpu, reg32 camera, const float (&offset)[3], reg32 turned,
                                   reg32 carPlace)
{
    turnByMatrix(app, offset, at(app, camera + kCamCar) + kCarOrientation, turned);
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, carPlace, turned, camera + kCamPlace);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 car = at(app, camera + kCamCar);
    if (at(app, kPlayerOut + at(app, camera + kCamPlayer) * 4) != 0)
    {
        turnedAround(app, car + kCarOrientation, camera + kCamOrientation);
        return car + kCarOrientation;
    }
    for (reg32 k = 0; k < 0x24; k += 4)
        put(app, camera + kCamOrientation + k, at(app, car + kCarOrientation + k));
    return car;
}

}  // namespace

namespace
{

// The race's replay (0 none).
constexpr reg32 kReplaying = 0x7922fc;

/* 0x4240c0's: chase camera placed behind its car: its offset (+0x110) -- in a race without a replay
 * its height from the car's (+0x11c, by 0x536eac) and none sideways, in a replay sideways by the
 * car's side of view (+0x9a8) -- its height scaled (its target's +0x10c), mirrored for a player
 * looking back, then offsetFromCar.  Its roll set.  eax as that leaves it; nullopt when the game
 * stopped. */
std::optional<reg32> chase(App* app, CPU& cpu, reg32 camera)
{
    const Frame frame(cpu, 0x18, 0x28);
    const reg32 e = frame.base;
    reg32 offset[3] = {at(app, camera + 0x110), at(app, camera + 0x114), at(app, camera + 0x118)};
    for (const reg32 k : {0u, 4u, 8u})
        put(app, e - 0x10 + k, at(app, at(app, camera + kCamCar) + kCarPlace + k));
    if (at(app, kReplaying) == 0)
    {
        put(app, camera + kCamRoll, 0x3ca3d70a);
        offset[0] = 0;
        offset[2] = bitsOf(float(fp::mul(f32(app, at(app, camera + kCamCar) + 0x11c), f64(app, 0x536eac))));
    }
    else
    {
        put(app, camera + kCamRoll, 0);
        const reg32 side = at(app, at(app, camera + kCamCar) + 0x9a8);
        if (side == 0)
            offset[0] = 0xbecccccd;
        else if (side == 1)
            offset[0] = 0x3ecccccd;
        else if (side == 2)
            offset[0] = 0;
    }
    offset[1] = bitsOf(float(fp::mul(floatIn(offset[1]), f32(app, at(app, camera + kCamTarget) + 0x10c))));
    if (at(app, kPlayerOut + at(app, camera + kCamPlayer) * 4) != 0)
        offset[2] ^= 0x80000000;
    const float v[3] = {floatOf(offset[0]), floatOf(offset[1]), floatOf(offset[2])};
    return offsetFromCar(app, cpu, camera, v, e - 0x28, e - 0x10);
}

}  // namespace

/* 0x4240c0: chase for camera eax. */
x86::reg32 chaseOffset(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax] { return chase(app, cpu, camera); });
    return args.eax;
}

namespace
{

/* 0x424260's: chase camera placed behind its car at its own offset (+0x110): its height scaled
 * (its target's +0x10c), its distance from the car's (+0x11c, by 0x536eb4), mirrored for a player
 * looking back, then offsetFromCar.  eax as that leaves it; nullopt when the game stopped. */
std::optional<reg32> chaseLevel(App* app, CPU& cpu, reg32 camera)
{
    const Frame frame(cpu, 0x18, 0x28);
    const reg32 e = frame.base;
    reg32 offset[3] = {at(app, camera + 0x110), at(app, camera + 0x114), at(app, camera + 0x118)};
    for (const reg32 k : {0u, 4u, 8u})
        put(app, e - 0x10 + k, at(app, at(app, camera + kCamCar) + kCarPlace + k));
    offset[1] = bitsOf(float(fp::mul(floatIn(offset[1]), f32(app, at(app, camera + kCamTarget) + 0x10c))));
    offset[2] = bitsOf(float(fp::mul(f32(app, at(app, camera + kCamCar) + 0x11c), f64(app, 0x536eb4))));
    if (at(app, kPlayerOut + at(app, camera + kCamPlayer) * 4) != 0)
        offset[2] ^= 0x80000000;
    const float v[3] = {floatOf(offset[0]), floatOf(offset[1]), floatOf(offset[2])};
    return offsetFromCar(app, cpu, camera, v, e - 0x1c, e - 0x10);
}

}  // namespace

/* 0x424260: chaseLevel for camera eax. */
x86::reg32 chaseOffsetLevel(App* app, x86::CPU& cpu, x86::Args& args)
{
    asThunk(cpu, args, [&, camera = args.eax] { return chaseLevel(app, cpu, camera); });
    return args.eax;
}

/* The typed cores the game's other code calls (native_carstate.h): each the function above it
 * under the game's address, its arguments and its eax as C++ values. */
std::optional<reg32> cameraViewChosen(App* app, CPU& cpu, reg32 view, reg32 how)
{
    return chooseHow(app, cpu, view, how);
}

reg32 chaseNodePicked(App* app, reg32 player)
{
    return chaseNode(app, player);
}

std::optional<reg32> camerasAttached(App* app, CPU& cpu, reg32 player, reg32 car)
{
    return attach(app, cpu, player, car);
}

std::optional<reg32> cameraLookedAt(App* app, CPU& cpu, reg32 camera, reg32 place)
{
    return lookAt(app, cpu, camera, place);
}

std::optional<reg32> cameraSwitched(App* app, CPU& cpu, reg32 mode, reg32 player)
{
    return switchTo(app, cpu, mode, player);
}

ViewCopy viewCopied(App* app, reg32 player, reg32 to)
{
    const ViewSource source = copyView(app, player, to);
    return ViewCopy{source.camera, source.mode};
}

std::optional<reg32> camerasSetUp(App* app, CPU& cpu)
{
    return setUpCameras(app, cpu);
}

std::optional<reg32> cameraModePut(App* app, CPU& cpu, reg32 player, reg32 mode)
{
    return putCameraMode(app, cpu, player, mode);
}

std::optional<reg32> overrideModePut(App* app, CPU& cpu, reg32 player, reg32 mode)
{
    return putOverrideMode(app, cpu, player, mode);
}

void cycleBuilt(App* app)
{
    buildCycle(app);
}

bool modeInTheCycle(App* app, reg32 mode)
{
    return inCycle(app, mode);
}

bool modeRemembered(App* app, reg32 player, reg32 mode)
{
    return remember(app, player, mode);
}

std::optional<reg32> cameraSettled(App* app, CPU& cpu, reg32 player)
{
    return settle(app, cpu, player);
}

std::optional<reg32> spotNearestTo(App* app, CPU& cpu, reg32 player)
{
    return nearest(app, cpu, player);
}

bool roadBitsGone(reg32 flags)
{
    return bitsNotOneRun(flags);
}

reg32 vectorDamped(App* app, reg32 car, reg32 weight, reg32 vector)
{
    return damp(app, car, weight, vector);
}

}  // namespace carstate
}  // namespace nfs3hp
