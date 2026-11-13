#include "game_settings.h"
#include "native_aiplan.h"
#include "native_collide.h"
#include "native_controls_pipeline.h"
#include "native_controls_race.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_graphics.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_load_cardb.h"
#include "native_load_saves.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_vertices.h"
#include "native_windows.h"
#include <winapi/kernel32.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

/* The race's cars brought in and kept up, and what goes with them (the
 * game's code from 0x42c2a0 to 0x42f550).  disassemble_nfs3hp.py binds each
 * address to the function of that name here (RACELOAD_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code.
 *
 * The loading: [0x6fd510] cars, each a record of 0x9ac bytes at 0x5e1120 on
 * and one of 0x4ac at 0x5eabe0 on (its performance, [car + 0x528]), its
 * choice -- 0x6c bytes at 0x6fd52c on, +0x14 the car's index -- at
 * [car + 0x220].  Each car's .viv (a car's own, the knock-out's, the
 * traffic's or a pursuit's, as the race and the index say) is loaded whole
 * to [0x553070] while the car is made from it -- its carp.txt read into the
 * performance record -- and freed after.  [0x5efac8] on hold the cars' list
 * (from 0x5efac4 + 4 for the code that counts from 1), [0x5efcc8] on the
 * cars by place and [0x5efc88] on by distance, [0x5efd9c] cars in all.  Each
 * car's handlers -- what moves it, steers it, draws it -- are the function
 * pointers at +0x5f8 to +0x61c, added to the race's task lists (0x4be250)
 * by their priority and taken off again (0x4be2f0).
 *
 * The race's frame: each car's speeds, distance along the track and laps,
 * the places sorted; its headlights' and its shadow's frames.  The race's
 * messages typed and received: a list of lines of 0x70 bytes (+0 who, +4 the
 * text, +0x68 the next, +0x6c the one before) under a lock of its own, its
 * head at [0x5537a0].  The race's controls are read on a thread of their
 * own, woken by the game's timer every fourth tick.  And the collisions
 * between cars and with the track's walls: the boxes' tests, the impulses,
 * the spins.
 *
 * Each is a typed core -- its arguments named, its result returned, its
 * locals C++'s -- and an adapter under the game's name, for the dispatch and
 * the callers still on registers, which leaves the registers and the x87's
 * stack as the code does (those the code pushes and pops kept by Kept).  The
 * x87's arithmetic is fp:: in the code's order, each store to a float a
 * float.  The other typed files' functions are called as C++ (guest::direct,
 * their headers), what the code reads in a register after one made from
 * what it gives back -- by address where it gives back less.  Every other
 * function of the game's, and Windows' through its imports, is called by
 * address, its arguments in the registers and on the guest's stack as the
 * code has them; a frame whose address a callee is
 * handed stays on the guest's stack as deep as the code has it (GuestFrame),
 * a core's call of a core making the return address's slot (Nested).  Where
 * a register the code leaves is seen by its callers -- a callee's, a word
 * pushed through it, an x87 status word in ax -- the core or the adapter
 * leaves it so, and says so. */
namespace nfs3hp
{
namespace raceload
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

// The loading's: the car's .viv loaded whole, the cars in the race, the
// cars' records, their performance records and their choices.
constexpr reg32 kCarViv = 0x553070, kCars = 0x5e1120, kCarBytes = 0x9ac;
constexpr reg32 kPerformances = 0x5eabe0, kPerformanceBytes = 0x4ac;
constexpr reg32 kCarsBytes = 0x9ac0, kPerformancesBytes = 0x4ac0;
// The cars' lists: in order, by place, by distance; how many; each car's
// slot of 0x258 bytes; each car's flag the unloading clears.
constexpr reg32 kCarList = 0x5efac8, kByPlace = 0x5efcc8, kByDistance = 0x5efc88, kCarCount = 0x5efd9c;
constexpr reg32 kCarSlots = 0x5ef6a0, kCarSlotsBytes = 0x258, kCarFlags = 0x5efd48;
// The race's counts the loading clears.
constexpr reg32 kRaceCounts[] = {0x5efd9c, 0x5efda0, 0x5efd98, 0x5efd90, 0x5efda4, 0x5efd94, 0x5efd8c, 0x5efd88,
                                 0x553074, 0x553078};
// The race: its kind's flags, the track, the pursuit's, the knock-out's,
// the player's car, the cars there are.
constexpr reg32 kDataPath = 0x7a28b4;
// The time step's: the options' bytes, the step.
constexpr reg32 kOption1 = 0x56484e, kOption2 = 0x56483a, kOption3 = 0x564850, kTimeStep = 0x55307c;
// The task lists the handlers go to, by kind.
constexpr reg32 kTasks = 0x7d3690, kDrawTasks = 0x7d368c;
// The game's strings: an empty name, "%sp", "%s.txt", the .vivs' paths.
constexpr reg32 kNoName = 0x53732c, kPerformanceName = 0x537330, kTextName = 0x537334;
constexpr reg32 kTrafficViv = 0x537348, kPursuitViv = 0x53735c, kKnockoutViv = 0x537378, kCarVivPath = 0x537388;
// The loading bar's: where the cars start it (75 per cent), 15 per cent for them all.
constexpr reg32 kBarStart = 0x42960000, kBarCars = 0x537340;
// What is called by address; the force feedback's handler (a task's).
constexpr reg32 kReadPerformance = 0x4b1380, kCarModel = 0x439710;
constexpr reg32 kSprintf = 0x4df690, kLoadingTick = 0x4a33d0, kCarShapes = 0x4b7d30;
constexpr reg32 kPlaceCar = 0x42bcb0, kCarReset = 0x40cb70, kCarPrepared = 0x4285f0;
constexpr reg32 kFillZero = 0x4e070c, kCarMade = 0x428700;
constexpr reg32 kMemset = 0x4e0640, kFreeShapes = 0x4b7eb0, kAddTask = 0x4be250, kRemoveTask = 0x4be2f0;
constexpr reg32 kForceTask = 0x471e10;
// The race's frame: the track's nodes and their records (0x24 bytes each),
// the race's frame counter, the camera's car and the race's light.
constexpr reg32 kTrackNodes = 0x5dd940, kTrackRecords = 0x5dd948, kFrameCounter = 0x7d3684;
constexpr reg32 kLight = 0x7a3a60;
// Its doubles: 1024 (a turn's heading), 2^-16 (a node's direction), 1/32 (the
// frame's fraction), a quarter.
constexpr reg32 kHeadingScale = 0x537398, kNodeScale = 0x5373a0, kThirtySecond = 0x5373a8, kQuarter = 0x5373b0;
// The message lines: their head (the lock, the first line, the last), the
// lines' numbers, a line's size and its block's name; the race's network:
// whether it is up, its mode, its machines (<< 16) and players' list; the
// mark of a message sent.  The two lines' zeros in nfs3.exe's code.
constexpr reg32 kLines = 0x5537a0, kLineSerial = 0x55379c, kLineBytes = 0x70, kLineName = 0x5373b8;
constexpr reg32 kNetworkUp = 0x7a22da, kNetworkMode = 0x7a22e4, kMachines = 0x7a2274, kPlayers = 0x7a21f0;
constexpr reg32 kMessageSent = 0x7a22a8, kEmptyCopy = 0x42d550, kEmptyLine = 0x42d5b4;
// The race controls' thread: its event and what it is set with, the ticks
// counted (each, every second, every fourth), whether the timer has it,
// the function it is given, the timer's count the start clears.
constexpr reg32 kControlsEvent = 0x5efdf0, kControlsThread = 0x7a3aac, kTicks = 0x5efdf4, kHalfTicks = 0x5efdf8;
constexpr reg32 kWakes = 0x5efdfc, kAllTicks = 0x5efe00, kTicking = 0x5537a4, kTickFunction = 0x42dc60;
constexpr reg32 kTimerCount = 0x79c4ac;
// The collisions': the two cars' frames one in the other (the axes, 9
// floats, and the place of each), the corner taken in, the contact's points.
constexpr reg32 kAxesBA = 0x5efee0, kAxesAB = 0x5efe90, kPlaceBA = 0x5efe60, kPlaceAB = 0x5efe6c;
constexpr reg32 kCorner0 = 0x5eff04, kCorner1 = 0x5efeb4, kCorner2 = 0x5efeb8;
constexpr reg32 kContact = 0x5efe78, kContactOther = 0x5efe84;
// Their numbers: 5 (a float), 0.75 the impulse's scale, 1/(2 pi), the spin's
// most (1.8), 0.9, the speed a wall damps (22), its damping (0.958), 0.975;
// a corner's fractions (0.75, 0.5, 0.8); a hit counted past 5.
constexpr reg32 kFiveTimes = 0x5373bc, kImpulseScale = 0x5537ac, kOneTurn = 0x5373c4, kMostSpin = 0x5537b0;
constexpr reg32 kNinetenths = 0x5373cc, kFastAlong = 0x5537b8, kWallDamping = 0x5537b4, kSlowZ = 0x5373d4;
constexpr reg32 kThreeQuarters = 0x5373dc, kHalf = 0x5373e4, kFourFifths = 0x5373ec, kFive = 0x5373f4;
// What else is called by address: a dot product (where its eax is read),
// vectors added (where the registers it leaves are), a vector crossed, a matrix transposed, turned; the car's speed, atan2,
// Watcom's __CHP; the messages' box closed, the text's fit, strncpy; the
// timers.
constexpr reg32 kDotProduct = 0x4e01f0, kAddVectors = 0x4dffb0;
constexpr reg32 kCross = 0x4e0160, kTranspose = 0x4e0110;
constexpr reg32 kTurnMatrix = 0x4eaab0, kTurnAxes = 0x4ea9e0;
constexpr reg32 kCarSpeed = 0x4a2bc0, kAtan2 = 0x4ea9c0, kChop = 0x4dfd56;
constexpr reg32 kCloseMessageBox = 0x436cf0;
constexpr reg32 kTextFits = 0x441900, kStrncpy = 0x4e0e30;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kAddTimer = 0x48ee30, kRemoveTimer = 0x48ee90;

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

x86::Float f32(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

x86::Float f64(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

/* fcomp, fcompp or fucompp's comparison, fnstsw ax and sahf: the x87's
 * status word in ax, and the flags sahf takes from it -- CF (C0) `below`,
 * ZF (C3) `equal`, both set when unordered. */
struct Status
{
    bool below, equal;
};

Status compare(x86::CPU& cpu, const x86::Float& a, const x86::Float& b)
{
    cpu.fpu.compare(a, b);
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.ax & 0x100) != 0, (cpu.ax & 0x4000) != 0};
}

using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::Float;
using i32 = std::int32_t;

// A car's record's (the race's, 0x9ac bytes): the node it was at, its
// distance into the race, its node, its place, the laps it has done and
// those it owes, its choice and its performance record.
constexpr reg32 kCarLastNode = 4, kCarDistance = 8, kCarNode = 0x1c, kCarLaps = 0x204, kCarLapsOwed = 0x208;
constexpr reg32 kCarChoice = 0x220, kCarPerformance = 0x528;
// A message line's: who sent it, its text, the next, the one before; the
// lines' head's: the lock, the first line, the last.
constexpr reg32 kLineFrom = 0, kLineText = 4, kLineNext = 0x68, kLineBefore = 0x6c;
constexpr reg32 kHeadLock = 0, kHeadFirst = 4, kHeadLast = 8;

/* The registers of its caller's a function of the game's pushes and pops,
 * put back as its code does when this goes: the adapters', and the cores'
 * around a call of another core. */
enum : unsigned { kEbx = 1, kEcx = 2, kEdx = 4, kEsi = 8, kEdi = 0x10 };
class Kept
{
public:
    Kept(CPU& cpu, unsigned which)
        : cpu_(cpu), which_(which), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi)
    {
    }
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;
    ~Kept()
    {
        if (which_ & kEbx)
            cpu_.ebx = ebx_;
        if (which_ & kEcx)
            cpu_.ecx = ecx_;
        if (which_ & kEdx)
            cpu_.edx = edx_;
        if (which_ & kEsi)
            cpu_.esi = esi_;
        if (which_ & kEdi)
            cpu_.edi = edi_;
    }

private:
    CPU& cpu_;
    const unsigned which_;
    const reg32 ebx_, ecx_, edx_, esi_, edi_;
};

/* A core's call of a core as the game's call is: the return address's slot
 * under esp while it runs, and the registers the callee's code keeps put
 * back after. */
class Nested
{
public:
    Nested(CPU& cpu, unsigned kept) : kept_(cpu, kept), cpu_(cpu) { cpu.esp -= 4; }
    Nested(const Nested&) = delete;
    Nested& operator=(const Nested&) = delete;
    ~Nested() { cpu_.esp += 4; }

private:
    const Kept kept_;
    CPU& cpu_;
};

/* Room on the guest's stack for what a callee is handed the address of: the
 * game's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    ~GuestFrame() { cpu_.esp = esp_; }
    reg32 operator-(reg32 offset) const { return base_ - offset; }
    /* The registers the code pushes, the first highest, where it pushes
     * them: for a callee's stand-in that logs the words past a frame's. */
    void pushed(App* app, std::initializer_list<reg32> registers) const
    {
        reg32 slot = esp_;
        for (const reg32 value : registers)
            put(app, slot -= 4, value);
    }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

reg32 floatBits(float f)
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

float f32At(App* app, reg32 address) { return app->getMemory<float>(address); }

void putFloat(App* app, reg32 address, float f) { app->getMemory<float>(address) = f; }

/* The length of the string at `text` in the guest's memory. */
reg32 lengthOf(App* app, reg32 text)
{
    reg32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    return length;
}

// The flags of the blocks the cars' loading and the message lines allocate.
constexpr reg32 kBlockFlags = 0x10;
// The register mask of every register but eax.
constexpr unsigned kAllButEax = 0x1f;

// The callees by address, their arguments in the registers and on the
// guest's stack as the code has them; the registers as each leaves them.
// The other typed files' as C++ (guest::direct), eax made from what one
// gives back where the code has it there.  False or nullopt when the game
// stopped in one.

bool loadingTick(App* app, CPU& cpu) { return guest::call(app, cpu, kLoadingTick); }

/* memset: `bytes` bytes at `to` the byte `value`. */
bool fill(App* app, CPU& cpu, reg32 to, reg32 value, reg32 bytes)
{
    cpu.eax = to;
    cpu.edx = value;
    cpu.ebx = bytes;
    return guest::call(app, cpu, kMemset);
}

/* A block of `bytes` allocated, named `name`, with the flags; or 0 (in eax
 * too). */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 flags)
{
    const std::optional<reg32> block = guest::direct<0x4e1620, ea::reserved>(app, cpu, name, bytes, flags);
    if (block)
        cpu.eax = *block;
    return block;
}

/* The block freed; eax what that gives back. */
bool freeBlock(App* app, CPU& cpu, reg32 block)
{
    const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    if (!freed)
        return false;
    cpu.eax = *freed;
    return true;
}

bool freeShapes(App* app, CPU& cpu, reg32 car)
{
    cpu.eax = car;
    return guest::call(app, cpu, kFreeShapes);
}

/* The event `event` set (0x4dfb20), eax what is posted with it as the code
 * has it then; eax what the setting gives back. */
bool postEvent(App* app, CPU& cpu, reg32 posted, reg32 event)
{
    cpu.eax = posted;
    const std::optional<reg32> set = guest::direct<0x4dfb20, ea::eventPosted>(app, cpu, event);
    if (!set)
        return false;
    cpu.eax = *set;
    return true;
}

/* EnterCriticalSection or LeaveCriticalSection (`Function`) of the lock
 * `lock`, eax `leftover` as the code has it then: the port's own, called as
 * C++ (native_windows.h); in the check through its import's slot `Slot`. */
template <reg32 Slot, auto Function>
bool lockCall(App* app, CPU& cpu, reg32 leftover, reg32 lock)
{
    cpu.eax = leftover;
    return guest::windows<Slot, Function>(app, cpu, lock).has_value();
}

/* The message lines' lock ([[0x5537a0]]) taken or let go of. */
template <reg32 Slot, auto Function>
bool lockLines(App* app, CPU& cpu)
{
    const reg32 head = at(app, kLines);
    return lockCall<Slot, Function>(app, cpu, head, at(app, head + kHeadLock));
}

/* A call of the game's on the car `car` in eax. */
bool carCall(App* app, CPU& cpu, reg32 target, reg32 car)
{
    cpu.eax = car;
    return guest::call(app, cpu, target);
}

/* racestart::placeOnGrid: the car on the grid at `slot`. */
bool placeCar(App* app, CPU& cpu, reg32 car, reg32 slot)
{
    cpu.eax = car;
    cpu.edx = slot;
    return guest::call(app, cpu, kPlaceCar);
}

/* The handler `handler` of the car added to the task list `list` at its
 * priority (0x4be250), or taken off (0x4be2f0, ebx the car as the code has
 * it). */
bool addTask(App* app, CPU& cpu, reg32 list, reg32 handler, reg32 car, reg32 priority)
{
    cpu.eax = list;
    cpu.edx = handler;
    cpu.ebx = car;
    cpu.ecx = priority;
    return guest::call(app, cpu, kAddTask);
}

bool removeTask(App* app, CPU& cpu, reg32 list, reg32 handler, reg32 car)
{
    cpu.eax = list;
    cpu.edx = handler;
    cpu.ebx = car;
    return guest::call(app, cpu, kRemoveTask);
}

/* The dot product of the vectors at `a` and `b` (0x4e01f0), off the x87's
 * stack: by address, for a caller that reads what it leaves in eax, which
 * vertices::dotProduct does not give back. */
std::optional<Float> dotProduct(App* app, CPU& cpu, reg32 a, reg32 b)
{
    cpu.eax = a;
    cpu.edx = b;
    if (!guest::call(app, cpu, kDotProduct))
        return std::nullopt;
    return cpu.fpu.take();
}

/* The same as C++, where nothing reads eax after it; edx `b`, as the code has
 * it then, which the callee keeps. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    cpu.edx = b;
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* 0x4ea290 (stdcall): the file named `name` found in the .viv `viv`, its
 * place in it to [place] and its size to [size]; eax the name's address it
 * gives back. */
bool findInViv(App* app, CPU& cpu, reg32 viv, reg32 name, reg32 place, reg32 size)
{
    const std::optional<reg32> found = guest::direct<0x4ea290, load::locatedEntry>(app, cpu, viv, name, 0, place, size);
    if (!found)
        return false;
    cpu.eax = *found;
    return true;
}

/* 0x4b1380: the performance text of `bytes` at `text` read into the
 * performance record `record`. */
bool readPerformance(App* app, CPU& cpu, reg32 record, reg32 text, reg32 bytes)
{
    cpu.eax = record;
    cpu.edx = text;
    cpu.ebx = bytes;
    cpu.ecx = 0;
    return guest::call(app, cpu, kReadPerformance);
}

/* sprintf (cdecl): its words, the buffer first. */
bool print(App* app, CPU& cpu, std::initializer_list<reg32> words)
{
    return guest::call(app, cpu, kSprintf, words);
}

/* 0x4b7d30: the car's shapes made from the .viv `viv`. */
bool carShapes(App* app, CPU& cpu, reg32 car, reg32 viv)
{
    cpu.eax = car;
    cpu.edx = viv;
    return guest::call(app, cpu, kCarShapes);
}

/* loadFile: the file at `path` loaded whole into a block with the flags; the
 * block, or 0 (in eax too). */
std::optional<reg32> loadFile(App* app, CPU& cpu, reg32 path, reg32 flags)
{
    const std::optional<reg32> block = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, path, flags);
    if (block)
        cpu.eax = *block;
    return block;
}

/* 0x4e070c: `bytes` bytes at `to` zeroed. */
bool fillZero(App* app, CPU& cpu, reg32 to, reg32 bytes)
{
    cpu.eax = to;
    cpu.edx = bytes;
    return guest::call(app, cpu, kFillZero);
}

/* 0x4a2bc0: a speed of the car's, off the x87's stack. */
std::optional<Float> carSpeed(App* app, CPU& cpu, reg32 car)
{
    if (!carCall(app, cpu, kCarSpeed, car))
        return std::nullopt;
    return cpu.fpu.take();
}

/* aiplan::roadSliceIndex (stdcall): the road's slice of the node `node` at
 * the speed of those bits (in eax too). */
std::optional<reg32> roadSliceIndex(App* app, CPU& cpu, reg32 node, reg32 speedBits)
{
    const reg32 slice = guest::direct<0x40e2b0, aiplan::roadSlice>(app, cpu, node, floatOf(speedBits));
    if (cpu.terminate)
        return std::nullopt;
    cpu.eax = slice;
    return slice;
}

/* atan2 (stdcall) of the floats of those bits, off the x87's stack. */
std::optional<Float> arcTangent(App* app, CPU& cpu, reg32 xBits, reg32 zBits)
{
    if (!guest::call(app, cpu, kAtan2, {xBits, zBits}))
        return std::nullopt;
    return cpu.fpu.take();
}

/* __CHP: the value to an integer, towards 0, as a whole value. */
std::optional<Float> chop(App* app, CPU& cpu, Float value)
{
    cpu.fpu.push(value);
    if (!guest::call(app, cpu, kChop))
        return std::nullopt;
    return cpu.fpu.take();
}

/* 0x4dffb0: the vector at `a` and the one at `b` added into `out` (1, the
 * vectors' count). */
bool addVectors(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, a, b, out);
    return !cpu.terminate;
}

/* The same by address (eax 1, edx `a`, ebx `b`, ecx `out`), for a caller
 * whose code leaves the registers as it leaves them: the collisions'. */
bool addVectorsLeaving(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    cpu.eax = 1;
    cpu.edx = a;
    cpu.ebx = b;
    cpu.ecx = out;
    return guest::call(app, cpu, kAddVectors);
}

/* 0x4e0050 (stdcall): the vector at `from` scaled by the float of those bits
 * into `to` (1, the vectors' count). */
bool scaleVectors(App* app, CPU& cpu, reg32 from, reg32 factorBits, reg32 to)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, from, floatOf(factorBits), to);
    return !cpu.terminate;
}

/* 0x4eaab0 or 0x4ea9e0 (stdcall): the matrix at `matrix` turned by the float
 * of those bits. */
bool turn(App* app, CPU& cpu, reg32 target, reg32 matrix, reg32 angleBits)
{
    cpu.eax = matrix;
    return guest::call(app, cpu, target, {angleBits, matrix});
}

/* 0x4e0110: the matrix at `from` transposed into `to`. */
bool transpose(App* app, CPU& cpu, reg32 from, reg32 to)
{
    cpu.eax = from;
    cpu.edx = to;
    return guest::call(app, cpu, kTranspose);
}

/* 0x4e06a0: the vector at `vector`'s length (vertices::vectorLength). */
std::optional<Float> normalize(App* app, CPU& cpu, reg32 vector)
{
    const double length = guest::direct<0x4e06a0, vertices::vectorLength>(app, cpu, vector);
    if (cpu.terminate)
        return std::nullopt;
    return Float(length);
}

/* 0x4972f0: the vector at `vector` normalized; its length
 * (vertices::normaliseVector). */
std::optional<Float> vectorLength(App* app, CPU& cpu, reg32 vector)
{
    const float length = guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, vector);
    if (cpu.terminate)
        return std::nullopt;
    return Float(length);
}

/* 0x4e0160: the vector at `a` crossed with the one at `b`, into `out`. */
bool cross(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    cpu.eax = a;
    cpu.edx = b;
    cpu.ebx = out;
    return guest::call(app, cpu, kCross);
}

/* Whether the text at `text` fits a message line (0x441900). */
std::optional<reg32> textFits(App* app, CPU& cpu, reg32 text)
{
    cpu.eax = text;
    if (!guest::call(app, cpu, kTextFits))
        return std::nullopt;
    return cpu.eax;
}

/* The text at `text` shown as a message line (0x436e10, channel 0); ebx as
 * the code has it then, or `most`; eax what the adding gives back. */
bool showLine(App* app, CPU& cpu, reg32 text)
{
    const reg32 added = guest::direct<0x436e10, collide::addTextResult>(app, cpu, 0, text);
    if (cpu.terminate)
        return false;
    cpu.eax = added;
    return true;
}

bool showLine(App* app, CPU& cpu, reg32 text, reg32 most)
{
    cpu.ebx = most;
    return showLine(app, cpu, text);
}

/* memmove: `bytes` bytes from `from` to `to`. */
bool moveMemory(App* app, CPU& cpu, reg32 from, reg32 to, reg32 bytes)
{
    return guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, from, bytes);
}

/* A packet of `bytes` at `data` sent to the machine `to` (0x4a60e0; 0
 * every other); eax, edx and ebx as it leaves them. */
bool sendPacket(App* app, CPU& cpu, reg32 to, reg32 data, reg32 bytes)
{
    const std::optional<ea::Regs> sent = guest::direct<0x4a60e0, ea::packetSent>(app, cpu, to, data, bytes);
    if (!sent)
        return false;
    cpu.eax = sent->eax;
    cpu.edx = sent->edx;
    cpu.ebx = sent->ebx;
    return true;
}

/* A lock made (0x4eb2e0); the lock (in eax too). */
std::optional<reg32> makeLock(App* app, CPU& cpu)
{
    const std::optional<reg32> lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
    if (lock)
        cpu.eax = *lock;
    return lock;
}

/* The lock freed (0x4eb360); eax what that gives back. */
bool freeLock(App* app, CPU& cpu, reg32 lock)
{
    const std::optional<reg32> freed = guest::direct<0x4eb360, ea::lockFreed>(app, cpu, lock);
    if (!freed)
        return false;
    cpu.eax = *freed;
    return true;
}

/* strncpy: at most `most` characters of `text` to `to`. */
bool copyText(App* app, CPU& cpu, reg32 to, reg32 text, reg32 most)
{
    cpu.eax = to;
    cpu.edx = text;
    cpu.ebx = most;
    return guest::call(app, cpu, kStrncpy);
}

/* An event made (0x4dfb00); the event (in eax too). */
std::optional<reg32> createEvent(App* app, CPU& cpu)
{
    const std::optional<reg32> event = guest::direct<0x4dfb00, ea::newAutoEvent>(app, cpu);
    if (event)
        cpu.eax = *event;
    return event;
}

/* The event waited for (0x4dfb70); what that gives back (in eax too). */
std::optional<reg32> waitEvent(App* app, CPU& cpu, reg32 event)
{
    const std::optional<reg32> waited = guest::direct<0x4dfb70, ea::eventWaited>(app, cpu, event);
    if (waited)
        cpu.eax = *waited;
    return waited;
}

/* port_timer's addCallback and removeCallback: `function` run with every
 * tick of the game's timer, or no longer. */
bool addTimer(App* app, CPU& cpu, reg32 function)
{
    cpu.eax = function;
    return guest::call(app, cpu, kAddTimer);
}

bool removeTimer(App* app, CPU& cpu, reg32 function)
{
    cpu.eax = function;
    return guest::call(app, cpu, kRemoveTimer);
}

}  // namespace

void loadCarPerformance(win32::WinApplication* app, x86::CPU& cpu);
void loadRaceCar(win32::WinApplication* app, x86::CPU& cpu);
void setCarHandlers(win32::WinApplication* app, x86::CPU& cpu);
void freeCarShapes(win32::WinApplication* app, x86::CPU& cpu);
void restartRaceCars(win32::WinApplication* app, x86::CPU& cpu);
void clearCarSlots(win32::WinApplication* app, x86::CPU& cpu);
void clearRaceCounts(win32::WinApplication* app, x86::CPU& cpu);
void clearCarList(win32::WinApplication* app, x86::CPU& cpu);
void loadRaceCars(win32::WinApplication* app, x86::CPU& cpu);
void setTimeStep(win32::WinApplication* app, x86::CPU& cpu);
void addCarTasks(win32::WinApplication* app, x86::CPU& cpu);
void openCarViv(win32::WinApplication* app, x86::CPU& cpu);
void closeCarViv(win32::WinApplication* app, x86::CPU& cpu);
void removeCarTasks(win32::WinApplication* app, x86::CPU& cpu);
void countLaps(win32::WinApplication* app, x86::CPU& cpu);
void carAxesSpeed(win32::WinApplication* app, x86::CPU& cpu);
void carForwardSpeed(win32::WinApplication* app, x86::CPU& cpu);
void carRaceDistance(win32::WinApplication* app, x86::CPU& cpu);
void updateRaceCars(win32::WinApplication* app, x86::CPU& cpu);
void frameFraction(win32::WinApplication* app, x86::CPU& cpu);
void placeHeadlights(win32::WinApplication* app, x86::CPU& cpu);
void placeCarShadow(win32::WinApplication* app, x86::CPU& cpu);
void sortRaceCars(win32::WinApplication* app, x86::CPU& cpu);
void emptyHook(win32::WinApplication* app, x86::CPU& cpu);
void emptyHook2(win32::WinApplication* app, x86::CPU& cpu);
void showMessage(win32::WinApplication* app, x86::CPU& cpu);
void sendMessage(win32::WinApplication* app, x86::CPU& cpu);
void openMessages(win32::WinApplication* app, x86::CPU& cpu);
void closeMessages(win32::WinApplication* app, x86::CPU& cpu);
void clearMessages(win32::WinApplication* app, x86::CPU& cpu);
void sendTypedMessage(win32::WinApplication* app, x86::CPU& cpu);
void newMessageLine(win32::WinApplication* app, x86::CPU& cpu);
void freeMessageLine(win32::WinApplication* app, x86::CPU& cpu);
void insertMessageLine(win32::WinApplication* app, x86::CPU& cpu);
void freeMessageLines(win32::WinApplication* app, x86::CPU& cpu);
void clearMessageLines(win32::WinApplication* app, x86::CPU& cpu);
void showMessageLines(win32::WinApplication* app, x86::CPU& cpu);
void appendMessageLine(win32::WinApplication* app, x86::CPU& cpu);
void closeMessageLines(win32::WinApplication* app, x86::CPU& cpu);
void openMessageLines(win32::WinApplication* app, x86::CPU& cpu);
void raceControlsThread(win32::WinApplication* app, x86::CPU& cpu);
void wakeRaceControls(win32::WinApplication* app, x86::CPU& cpu);
void raceControlsTick(win32::WinApplication* app, x86::CPU& cpu);
void startRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu);
void stopRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu);
void cornerInBox(win32::WinApplication* app, x86::CPU& cpu);
void cornerInBoxBack(win32::WinApplication* app, x86::CPU& cpu);
void cornerInBox2(win32::WinApplication* app, x86::CPU& cpu);
void cornerInBoxBack2(win32::WinApplication* app, x86::CPU& cpu);
void clampSpin(win32::WinApplication* app, x86::CPU& cpu);
void applyImpulse(win32::WinApplication* app, x86::CPU& cpu);
void bounceOffWall(win32::WinApplication* app, x86::CPU& cpu);
void hitWall(win32::WinApplication* app, x86::CPU& cpu);
void boxHitsWall(win32::WinApplication* app, x86::CPU& cpu);
void carsCollide(win32::WinApplication* app, x86::CPU& cpu);

// The cores other cores call before they are defined.
bool closeLines(App* app, CPU& cpu);
bool clearLines(App* app, CPU& cpu);
bool openLines(App* app, CPU& cpu);
bool appendLine(App* app, CPU& cpu, reg32 text);
std::optional<reg32> openViv(App* app, CPU& cpu, reg32 choice);
bool freeCarViv(App* app, CPU& cpu);
void sortPlaces(App* app);
reg32 sortedCount(App* app);

/* 0x42c2a0: the car eax's performance text, the file named edx in the car's
 * .viv ([0x553070]; 0x4ea290 finds its place and size), copied out into a
 * block of its own and read (0x4b1380) into the car's performance record
 * ([car + 0x528]); the block freed.  [car + 0x9a8] what 0x439710 gives for
 * the car's index (0 to 49), else 0. */
constexpr reg32 kChoiceIndex = 0x14, kCarModelSlot = 0x9a8;
constexpr i32 kCarIndexes = 0x32;  // the 50 cars' own

bool readCarPerformance(App* app, CPU& cpu, reg32 car, reg32 name)
{
    const GuestFrame frame(cpu, 0x14, 8);
    const reg32 size = frame - 8, place = frame - 4;
    put(app, size, 0);
    put(app, place, 0);
    if (!findInViv(app, cpu, at(app, kCarViv), name, place, size))
        return false;
    const reg32 from = at(app, place) + at(app, kCarViv);
    const std::optional<reg32> block = allocate(app, cpu, kNoName, at(app, size), kBlockFlags);
    if (!block || !moveMemory(app, cpu, from, *block, at(app, size))
        || !readPerformance(app, cpu, at(app, car + kCarPerformance), *block, at(app, size)))
        return false;
    if (*block != 0 && !freeBlock(app, cpu, *block))
        return false;
    const i32 index = i32(at(app, at(app, car + kCarChoice) + kChoiceIndex));
    reg32 model = 0;
    if (index >= 0 && index < kCarIndexes)
    {
        if (!carCall(app, cpu, kCarModel, reg32(index)))
            return false;
        model = cpu.eax;
    }
    put(app, car + kCarModelSlot, model);
    return true;
}

/* readCarPerformance of the car eax from the file edx; ebx, ecx, esi and edi
 * as the caller had them, eax the choice when the car is none of the 50 (as
 * the code leaves it). */
void loadCarPerformance(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEsi | kEdi);
    const reg32 car = cpu.eax;
    if (!readCarPerformance(app, cpu, car, cpu.edx))
        return;
    const reg32 choice = at(app, car + kCarChoice);
    const i32 index = i32(at(app, choice + kChoiceIndex));
    if (index < 0 || index >= kCarIndexes)
        cpu.eax = choice;
    cpu.esp += 4;
}

/* 0x42c350: the race's car ebx (if there are that many), its record eax,
 * made from its .viv: prepared (0x4285f0), its performance record edx, its
 * performance text ("<folder>p.txt", sprintf'd in the frame) read, its
 * drawing (0x4a78d6, unless +0x200 has 0x40) and shapes (0x4b7d30) made,
 * placed on the grid (0x42bcb0) and reset (0x40cb70) -- the loading bar's
 * tick (0x4a33d0) between. */
bool loadCar(App* app, CPU& cpu, reg32 car, reg32 slot, reg32 performance)
{
    if (i32(slot) >= i32(gameSettings(app).entryCount))
        return true;
    const GuestFrame frame(cpu, 8, 0x110);
    const reg32 shortName = frame - 0x14, textName = frame - 0x110;
    if (!carCall(app, cpu, kCarPrepared, car))
        return false;
    const reg32 index = at(app, at(app, car + kCarChoice) + kChoiceIndex);
    put(app, car + kCarPerformance, performance);
    const std::optional<reg32> prefix = guest::direct<0x439300, load::prefixOf>(app, cpu, index);
    if (!prefix)
        return false;
    cpu.eax = *prefix;
    if (!print(app, cpu, {shortName, kPerformanceName, *prefix}) || !print(app, cpu, {textName, kTextName, shortName}))
        return false;
    {
        const Nested nested(cpu, kEbx | kEcx | kEsi | kEdi);
        if (!readCarPerformance(app, cpu, car, textName))
            return false;
    }
    if (!loadingTick(app, cpu)
        || ((byte(app, car + 0x200) & 0x40) == 0 && !guest::direct<0x4a78d6, handling::tuneHandling>(app, cpu, car))
        || !loadingTick(app, cpu) || !carShapes(app, cpu, car, at(app, kCarViv)) || !loadingTick(app, cpu))
        return false;
    // The grid's and the reset's by address: what they leave in edx and eax
    // is what the car's loading leaves (placeOnGridOf and resetTraffic give
    // back only whether the game went on).
    if (!placeCar(app, cpu, car, slot) || !loadingTick(app, cpu))
        return false;
    return carCall(app, cpu, kCarReset, car);
}

/* loadCar eax at the slot ebx with the performance record edx; ecx as the
 * caller had it. */
void loadRaceCar(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!loadCar(app, cpu, cpu.eax, cpu.ebx, cpu.edx))
        return;
    cpu.esp += 4;
}

/* 0x42c400: the car eax's handlers -- what moves it, steers it, draws it --
 * as its kind is: a player's, else (+0x200 bit 1) the computer's -- a
 * pursuer's (0x10), traffic (0x20) -- and in a split screen race with more
 * than one player, every car but the player's that is not driven by one of
 * the people (+0x1f0) the network's, +0x201 bit 1 set. */
void setHandlers(App* app, reg32 car)
{
    put(app, car + 0x5fc, 0x4bd380);
    put(app, car + 0x600, 0x431430);
    put(app, car + 0x604, 0x4ce280);
    put(app, car + 0x608, 0x4ae4c1);
    put(app, car + 0x60c, 0x4a2830);
    put(app, car + 0x610, 0x4a2c60);
    put(app, car + 0x614, 0x4a2f30);
    put(app, car + 0x618, 0x4311d0);
    put(app, car + 0x61c, 0x42b900);
    const reg8 kind = byte(app, car + 0x200);
    put(app, car + 0x5f8, 0x4a08c0);
    if ((kind & 2) != 0)
    {
        put(app, car + 0x600, 0x409c00);
        put(app, car + 0x5fc, 0);
        if ((kind & 0x10) != 0)
            put(app, car + 0x600, 0x40c9f0);
        if ((kind & 0x20) != 0)
            put(app, car + 0x600, 0x4032e0);
        put(app, car + 0x614, 0x4a2f10);
        put(app, car + 0x608, 0x4088f0);
    }
    const auto& settings = gameSettings(app);
    if (settings.networkGame != 0 && sreg32(settings.playMode) > 1)
    {
        const reg32 driver = at(app, car + 0x1f0);
        if (driver != reg32(settings.localPlayer) && sreg32(driver) < sreg32(settings.racerCount))
        {
            put(app, car + 0x600, 0x40bf50);
            put(app, car + 0x608, 0x4088f0);
            const reg8 network = byte(app, car + 0x201);
            put(app, car + 0x614, 0x4a2f10);
            putByte(app, car + 0x201, reg8(network | 2));
        }
    }
}

/* setHandlers of eax. */
void setCarHandlers(App* app, CPU& cpu)
{
    setHandlers(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x42c520: the car eax's shapes freed (0x4b7eb0). */
void freeCarShapes(App* app, CPU& cpu)
{
    if (!freeShapes(app, cpu, cpu.eax))
        return;
    cpu.esp += 4;
}

/* 0x42c530: the race's cars back on the grid: the lists by place and by
 * distance the cars' list again, and each car placed (0x42bcb0), reset
 * (0x40cb70) and prepared (0x4285f0) afresh. */
bool restartCars(App* app, CPU& cpu)
{
    for (i32 i = 0; i < i32(at(app, kCarCount)); ++i)
    {
        const reg32 car = at(app, kCarList + reg32(i) * 4);
        put(app, kByPlace + reg32(i) * 4, car);
        put(app, kByDistance + reg32(i) * 4, car);
    }
    for (i32 i = 0; i < i32(at(app, kCarCount)); ++i)
    {
        const reg32 car = at(app, kCarList + reg32(i) * 4);
        if (!guest::direct<0x42bcb0, racestart::placeOnGridOf>(app, cpu, car, reg32(i))
            || !guest::direct<0x40cb70, aiplan::resetTraffic>(app, cpu, car) || !carCall(app, cpu, kCarPrepared, car))
            return false;
    }
    return true;
}

/* restartCars; ebx, ecx and edx as the caller had them, eax 0 (the first
 * loop's count) with no cars. */
void restartRaceCars(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    cpu.eax = 0;
    if (!restartCars(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42c660: the cars' slots (0x258 bytes at 0x5ef6a0) cleared (memset). */
bool clearSlots(App* app, CPU& cpu)
{
    return fill(app, cpu, kCarSlots, 0, kCarSlotsBytes);
}

/* clearSlots; ebx and edx as the caller had them. */
void clearCarSlots(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEdx);
    if (!clearSlots(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42c680: the loading bar's tick, and the race's counts -- the cars in
 * all among them -- 0. */
bool clearCounts(App* app, CPU& cpu)
{
    if (!loadingTick(app, cpu))
        return false;
    for (const reg32 count : kRaceCounts)
        put(app, count, 0);
    return true;
}

/* clearCounts; edx as the caller had it. */
void clearRaceCounts(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEdx);
    if (!clearCounts(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42c6d0: the cars' list's 16 pointers 0. */
void clearList(App* app)
{
    for (reg32 slot = 0; slot < 0x10; ++slot)
        put(app, kCarList + slot * 4, 0);
}

/* clearList; eax 16, the loop's count as the code leaves it. */
void clearCarList(App* app, CPU& cpu)
{
    clearList(app);
    cpu.eax = 0x10;
    cpu.esp += 4;
}

/* 0x42c700: the race's cars loaded: the last race's freed (0x42bb80), the
 * counts and the list cleared, the cars' and their performance records
 * zeroed (fillZero); then each car's .viv loaded, the car made (0x428700)
 * from its choice and loaded (loadRaceCar), the .viv freed -- the loading
 * bar from 75 per cent on, 15 per cent for them all -- and every car's
 * handlers set. */
bool loadCars(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0xc);
    const std::optional<reg32> grid = guest::direct<0x42bb80, racestart::loadStartGridOf>(app, cpu);
    if (!grid)
        return false;
    cpu.eax = *grid;
    if (!clearCounts(app, cpu))
        return false;
    clearList(app);
    float bar = 75.0f;  // per cent, where the cars start the loading bar
    if (!fillZero(app, cpu, kCars, kCarsBytes) || !fillZero(app, cpu, kPerformances, kPerformancesBytes))
        return false;
    for (i32 i = 0; i < i32(gameSettings(app).entryCount); ++i)
    {
        bar = float(fp::add(fp::div(f64(app, kBarCars), Float(i32(gameSettings(app).entryCount))), Float(bar)));
        {
            // The per cent in the word the code pushes, the bar handed its address.
            const GuestFrame pushed(cpu, 0, 4);
            put(app, pushed - 4, floatBits(bar));
            if (!guest::direct<0x495230, graphics::loadingProgress>(app, cpu, pushed - 4))
                return false;
        }
        if (!loadingTick(app, cpu))
            return false;
        const reg32 choice = SETTINGS_ADDRESS(entries) + reg32(i) * 0x6c;
        {
            const Nested nested(cpu, kEbx | kEdx | kEdi);
            if (!openViv(app, cpu, choice))
                return false;
        }
        const reg32 car = kCars + reg32(i) * kCarBytes;
        put(app, car + kCarChoice, choice);
        cpu.edx = reg32(i) * kPerformanceBytes;  // the record's offset, as the code has it then
        if (!carCall(app, cpu, kCarMade, car))
            return false;
        {
            const Nested nested(cpu, kEcx);
            if (!loadCar(app, cpu, at(app, kCarList + reg32(i) * 4), reg32(i), kPerformances + reg32(i) * kPerformanceBytes))
                return false;
        }
        const Nested nested(cpu, kEcx | kEdx);
        if (!freeCarViv(app, cpu))
            return false;
    }
    for (i32 i = 0; i < i32(gameSettings(app).entryCount); ++i)
    {
        if (!loadingTick(app, cpu))
            return false;
        setHandlers(app, at(app, kCarList + reg32(i) * 4));
    }
    return true;
}

/* loadCars; every register but eax as the caller had it, eax the last car's
 * (as its handlers were set) when there are cars. */
void loadRaceCars(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!loadCars(app, cpu))
        return;
    const i32 cars = i32(gameSettings(app).entryCount);
    if (cars > 0)
        cpu.eax = at(app, kCarList + reg32(cars - 1) * 4);
    cpu.esp += 4;
}

/* 0x42c7f0: the race's time step ([0x55307c]): 0.001 when the three
 * options' bytes (0x56484e and 0x56483a, then 0x564850) have a bit in
 * common, else 0.1. */
constexpr reg32 kFineStep = 0x3a83126f, kCoarseStep = 0x3dcccccd;  // 0.001f and 0.1f, as bits

void pickTimeStep(App* app)
{
    const bool fine = (byte(app, kOption1) & byte(app, kOption2) & byte(app, kOption3)) != 0;
    put(app, kTimeStep, fine ? kFineStep : kCoarseStep);
}

/* pickTimeStep; eax the third option's byte, as the code leaves it. */
void setTimeStep(App* app, CPU& cpu)
{
    pickTimeStep(app);
    cpu.eax = byte(app, kOption3);
    cpu.esp += 4;
}

/* 0x42c830: every car's handlers added to the race's task lists
 * (0x4be250: the list eax, the handler edx, the car ebx, its priority ecx):
 * +0x5f8 at 6, +0x600 at 0x15, +0x604 at 0x19 to the drawing's list (if
 * +0x200 has 8 or 4), +0x60c and +0x608 at 0x1e, +0x618 at 0x28, +0x61c at
 * 0x32, +0x610 at 0x1e to the drawing's, +0x614 at 0x1e to the drawing's
 * (+0x200 has 4) or the other. */
bool addTasks(App* app, CPU& cpu)
{
    for (i32 i = 0; i < i32(gameSettings(app).entryCount); ++i)
    {
        const reg32 car = at(app, kCarList + reg32(i) * 4);
        const auto add = [&](reg32 list, reg32 handler, reg32 priority) {
            return addTask(app, cpu, at(app, list), at(app, car + handler), car, priority);
        };
        if (!add(kTasks, 0x5f8, 6) || !add(kTasks, 0x600, 0x15))
            return false;
        if ((byte(app, car + 0x200) & 0xc) != 0 && !add(kDrawTasks, 0x604, 0x19))
            return false;
        if (!add(kTasks, 0x60c, 0x1e) || !add(kTasks, 0x608, 0x1e) || !add(kTasks, 0x618, 0x28)
            || !add(kTasks, 0x61c, 0x32) || !add(kDrawTasks, 0x610, 0x1e))
            return false;
        if (!add((byte(app, car + 0x200) & 4) != 0 ? kDrawTasks : kTasks, 0x614, 0x1e))
            return false;
    }
    return true;
}

/* addTasks; every register but eax as the caller had it. */
void addCarTasks(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!addTasks(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42c950: the .viv of the car whose choice is at eax loaded whole to
 * [0x553070] (loadFile, flags 0x10), its path sprintf'd into the frame from
 * the game's data path: in a knock-out race ([0x6fd4fc] bit 1) the track's
 * traffic -- a pursuit's for tracks 0x11 to 0x14 -- or the knock-out's
 * car.viv after that; else the car's own (index 0 to 49), a pursuit's
 * traffic (500 to 599) or the traffic's. */
std::optional<reg32> openViv(App* app, CPU& cpu, reg32 choice)
{
    const GuestFrame frame(cpu, 0x10, 0xfc);
    const reg32 path = frame - 0xfc;
    // The path of the car of index `index`, from its folder and its name.
    const auto carPath = [&](reg32 format, reg32 index) {
        const std::optional<reg32> folder = guest::direct<0x439300, load::prefixOf>(app, cpu, index);
        if (!folder)
            return false;
        const std::optional<reg32> name = guest::direct<0x4392b0, load::folderOf>(app, cpu, index);
        if (!name)
            return false;
        cpu.eax = *name;
        return print(app, cpu, {path, format, kDataPath, *name, *folder});
    };
    const auto& settings = gameSettings(app);
    if ((std::uint8_t(settings.raceFlags) & 2) != 0)
    {
        const i32 track = i32(settings.cheatTrack);
        if (track <= 0x10 ? !carPath(kTrafficViv, reg32(track + 0x32))
            : track <= 0x14 ? !carPath(kPursuitViv, reg32(track + 0x1e3))
                            : !print(app, cpu, {path, kKnockoutViv, kDataPath}))
            return std::nullopt;
    }
    else
    {
        const i32 index = i32(at(app, choice + kChoiceIndex));
        const reg32 format = index >= 0 && index < kCarIndexes ? kCarVivPath
                             : index >= 0x1f4 && index <= 0x257 ? kPursuitViv
                                                                : kTrafficViv;
        if (!carPath(format, reg32(index)))
            return std::nullopt;
    }
    const std::optional<reg32> viv = loadFile(app, cpu, path, kBlockFlags);
    if (viv)
        put(app, kCarViv, *viv);
    return viv;
}

/* openViv of the choice eax; ebx, edx and edi as the caller had them. */
void openCarViv(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEdx | kEdi);
    const std::optional<reg32> viv = openViv(app, cpu, cpu.eax);
    if (!viv)
        return;
    cpu.eax = *viv;
    cpu.esp += 4;
}

/* 0x42ca80: the car's .viv ([0x553070]) freed, if there is one, and 0. */
bool freeCarViv(App* app, CPU& cpu)
{
    const reg32 viv = at(app, kCarViv);
    if (viv == 0)
        return true;
    if (!freeBlock(app, cpu, viv))
        return false;
    put(app, kCarViv, 0);
    return true;
}

/* freeCarViv; ecx and edx as the caller had them. */
void closeCarViv(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!freeCarViv(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42cab0: every car's handlers taken off the race's task lists (0x4be2f0:
 * the list eax, the handler edx) -- +0x604 only if +0x200 has 1, the force
 * feedback's (0x471e10) if the car is the one it is played from -- its
 * shapes freed, its flag (0x5efd48) and its slot's first word (0x5ef6a0)
 * 0. */
std::optional<i32> removeTasks(App* app, CPU& cpu)
{
    i32 i = 0;
    for (; i < i32(at(app, kCarCount)); ++i)
    {
        const reg32 slot = reg32(i) * 4;
        const reg32 car = at(app, kCarList + slot);
        const auto remove = [&](reg32 list, reg32 handler) {
            return removeTask(app, cpu, at(app, list), at(app, car + handler), car);
        };
        if (!remove(kTasks, 0x5f8) || !remove(kTasks, 0x600))
            return std::nullopt;
        if ((byte(app, car + 0x200) & 1) != 0 && !remove(kDrawTasks, 0x604))
            return std::nullopt;
        if (!remove(kTasks, 0x608) || !remove(kTasks, 0x60c) || !remove(kDrawTasks, 0x610) || !remove(kTasks, 0x618)
            || !remove(kTasks, 0x61c))
            return std::nullopt;
        const bool forced = guest::direct<0x471dc0, controls::forceCar>(app, cpu, car);
        if (cpu.terminate || (forced && !removeTask(app, cpu, at(app, kTasks), kForceTask, car)))
            return std::nullopt;
        if (!remove((byte(app, car + 0x200) & 4) != 0 ? kDrawTasks : kTasks, 0x614))
            return std::nullopt;
        if (!freeShapes(app, cpu, car))
            return std::nullopt;
        if (at(app, kCarFlags + slot) != 0)
            put(app, kCarFlags + slot, 0);
        if (at(app, kCarSlots + slot) != 0)
            put(app, kCarSlots + slot, 0);
    }
    return i;
}

/* removeTasks; every register but eax as the caller had it, eax the last
 * car's place in the lists (its index times 4) as the code leaves it. */
void removeCarTasks(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    const std::optional<i32> cars = removeTasks(app, cpu);
    if (!cars)
        return;
    if (*cars > 0)
        cpu.eax = reg32(*cars - 1) * 4;
    cpu.esp += 4;
}

/* 0x42cc20: the car eax's laps counted as its node on the track (+0x1c)
 * passes the line, against the last (+4): forwards, from past node 500 to
 * 0 a lap on (+0x208 up), from 0 to 50 back past 500 a lap undone (+0x208
 * down, or +0x204 up, a lap lost, when there is none); backwards
 * ([0x6fd4c0]) the other way.  The node kept as the last. */
void countLap(App* app, reg32 car)
{
    const i32 node = i32(at(app, car + kCarNode)), last = i32(at(app, car + kCarLastNode));
    if (node == last)
        return;
    const auto nearStart = [](i32 at) { return at >= 0 && at <= 0x32; };
    const auto owe = [&] { put(app, car + kCarLapsOwed, at(app, car + kCarLapsOwed) + 1); };
    const auto done = [&] {
        const i32 owed = i32(at(app, car + kCarLapsOwed));
        if (owed > 0)
            put(app, car + kCarLapsOwed, reg32(owed - 1));
        else
            put(app, car + kCarLaps, at(app, car + kCarLaps) + 1);
    };
    if (gameSettings(app).backwards == 0)
    {
        if (node > 0x1f4 && last == 0)
            owe();
        else if (nearStart(node) && last > 0x1f4)
            done();
    }
    else if (nearStart(node) && last > 0x1f4)
        owe();
    else if (last <= 0x32 && node > 0x1f4)
        done();
    put(app, car + kCarLastNode, reg32(node));
}

/* countLap of eax. */
void countLaps(App* app, CPU& cpu)
{
    countLap(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x42cd00: the car eax's speed along its own axes, onto the x87's stack:
 * its velocity (+0x124) along each of its axes (+0xc0, +0xcc, +0xd8; a dot
 * product, 0x4e01f0) made positive and weighed by +0x114, +0x118, +0x11c,
 * summed. */
constexpr reg32 kAxesVelocity = 0x124, kCarAxes = 0xc0, kAxisWeights = 0x114;

/* The speed, and the last part along an axis as the code keeps it. */
struct AxesSpeed
{
    Float speed;
    float lastAlong;
};

std::optional<AxesSpeed> axesSpeed(App* app, CPU& cpu, reg32 car)
{
    float sum = 0.0f, along = 0.0f;
    Float speed = 0.0;
    for (reg32 axis = 0; axis < 3; ++axis)
    {
        // The last by address: what it leaves in eax is carAxesSpeed's.
        const std::optional<Float> part = axis < 2 ? dot(app, cpu, car + kAxesVelocity, car + kCarAxes + axis * 0xc)
                                                   : dotProduct(app, cpu, car + kAxesVelocity, car + kCarAxes + axis * 0xc);
        if (!part)
            return std::nullopt;
        along = float(*part);
        // fcomp of 0 with it, its size as the code makes it: negated when not
        // over 0 -- a NaN kept as it is, as the x87's unordered answer has it.
        const float size = Float(along) <= 0.0 ? float(-Float(along)) : along;
        const Float weighed = fp::mul(Float(size), f32(app, car + kAxisWeights + axis * 4));
        speed = axis == 0 ? weighed : fp::add(weighed, Float(sum));
        sum = float(speed);
    }
    return AxesSpeed{speed, along};
}

/* axesSpeed of eax onto the x87's stack; ecx and edx as the caller had them,
 * eax as the code leaves it: the last part's bits when over 0 (or a NaN),
 * else the x87's status word of that compare in ax. */
void carAxesSpeed(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    const std::optional<AxesSpeed> speed = axesSpeed(app, cpu, cpu.eax);
    if (!speed)
        return;
    if (compare(cpu, 0.0, Float(speed->lastAlong)).below)
        cpu.eax = floatBits(speed->lastAlong);
    cpu.fpu.push(speed->speed);
    cpu.esp += 4;
}

/* 0x42cdd0: the car eax's velocity (+0xa4) along its forward axis (+0x13c),
 * onto the x87's stack. */
constexpr reg32 kForwardVelocity = 0xa4, kForwardAxis = 0x13c;

Float forwardSpeed(App* app, reg32 car)
{
    const Float x = fp::mul(f32(app, car + kForwardVelocity), f32(app, car + kForwardAxis));
    const Float y = fp::mul(f32(app, car + kForwardVelocity + 4), f32(app, car + kForwardAxis + 4));
    const Float xy = fp::add(y, x);
    const Float z = fp::mul(f32(app, car + kForwardVelocity + 8), f32(app, car + kForwardAxis + 8));
    return fp::add(xy, z);
}

/* forwardSpeed of eax onto the x87's stack. */
void carForwardSpeed(App* app, CPU& cpu)
{
    cpu.fpu.push(forwardSpeed(app, cpu.eax));
    cpu.esp += 4;
}

/* 0x42ce10: the car eax's distance into the race (+8): its laps (+0x204)
 * times the track's nodes ([0x5dd940]) and its node -- backwards, the nodes
 * less its node; 0 while it owes laps (+0x208). */
void setRaceDistance(App* app, reg32 car)
{
    const reg32 nodes = at(app, kTrackNodes);
    if (i32(at(app, car + kCarLapsOwed)) > 0)
        put(app, car + kCarDistance, 0);
    else
    {
        const reg32 node = gameSettings(app).backwards != 0 ? nodes - at(app, car + kCarNode) : at(app, car + kCarNode);
        put(app, car + kCarDistance, at(app, car + kCarLaps) * nodes + node);
    }
    put(app, kTrackNodes, nodes);  // the count written back as it was, as the code does
}

/* setRaceDistance of eax. */
void carRaceDistance(App* app, CPU& cpu)
{
    setRaceDistance(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x42ce70: each car's race state for the frame, for those that are in it
 * (+0x89): its speed along its axes (+0x21c) and another of its speeds
 * (0x4a2bc0, +0x7b8) and what that is in its gearing (0x40e2b0, +0x638);
 * its heading (atan2 of +0x1a8, +0x1b0, times 1024, +0x210); its forward
 * speed (+0x7c0) and its size (+0x7c4); whether it is going the wrong way
 * (+0x5f4: the track node's forward direction against its forward axis,
 * the race's direction aside); and, if it is on the track (+0x74), its laps
 * and distance.  Then the places sorted. */
constexpr reg32 kCarInRace = 0x89, kCarOnTrack = 0x74, kCarWrongWay = 0x5f4, kCarHeading = 0x210;
constexpr reg32 kCarAxesSpeed = 0x21c, kCarSpeedSlot = 0x7b8, kCarSlice = 0x638, kCarForwardSpeed = 0x7c0;
constexpr reg32 kCarForwardSize = 0x7c4, kCarDirection = 0x1a8, kCarForwardAxis = 0xd8;

bool updateCars(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0x14);
    const reg32 direction = frame - 0x14;  // the node's forward direction, for the dot product
    for (i32 i = 0; i < i32(at(app, kCarCount)); ++i)
    {
        const reg32 car = at(app, kCarList + reg32(i) * 4);
        if (byte(app, car + kCarInRace) == 0)
            continue;
        const std::optional<AxesSpeed> axes = axesSpeed(app, cpu, car);
        if (!axes)
            return false;
        putFloat(app, car + kCarAxesSpeed, float(axes->speed));
        const std::optional<Float> speed = carSpeed(app, cpu, car);
        if (!speed)
            return false;
        putFloat(app, car + kCarSpeedSlot, float(*speed));
        const std::optional<reg32> gear = roadSliceIndex(app, cpu, at(app, car + kCarNode), at(app, car + kCarSpeedSlot));
        if (!gear)
            return false;
        const reg32 x = at(app, car + kCarDirection), z = at(app, car + kCarDirection + 8);
        put(app, car + kCarSlice, *gear);
        const std::optional<Float> angle = arcTangent(app, cpu, x, z);
        if (!angle)
            return false;
        const std::optional<Float> heading = chop(app, cpu, fp::mul(*angle, f64(app, kHeadingScale)));
        if (!heading)
            return false;
        put(app, car + kCarHeading, reg32(fp::toInteger<sreg32>(*heading)));
        const float forward = float(forwardSpeed(app, car));
        putFloat(app, car + kCarForwardSpeed, forward);
        // fcomp of 0 with it, its size as the code makes it: negated when not
        // over 0 -- a NaN kept as it is, as the x87's unordered answer has it.
        putFloat(app, car + kCarForwardSize, Float(forward) <= 0.0 ? float(-Float(forward)) : forward);
        // The node's direction: three signed bytes of its record (0x24 bytes
        // each), each << 9 times 2^-16.
        const reg32 records = at(app, kTrackRecords);
        const auto part = [&](reg32 offset) {
            const reg32 bits = at(app, records + at(app, car + kCarNode) * 0x24 + offset);
            return Float(sreg32(reg32(sreg32(bits) >> 0x18) << 9));
        };
        const Float scale = f64(app, kNodeScale);
        putFloat(app, direction, float(fp::mul(part(0x11), scale)));
        putFloat(app, direction + 4, float(fp::mul(part(0x12), scale)));
        putFloat(app, direction + 8, float(fp::mul(scale, part(0x13))));
        const std::optional<Float> along = dot(app, cpu, car + kCarForwardAxis, direction);
        if (!along)
            return false;
        // fcompp of 0 with it: the wrong way when it is not over 0, a NaN not.
        const reg32 wrong = *along <= 0.0 ? 1 : 0;
        put(app, car + kCarWrongWay, reg32(gameSettings(app).backwards) ^ wrong);
        if (half(app, car + kCarOnTrack) != 0)
        {
            countLap(app, car);
            setRaceDistance(app, car);
        }
    }
    sortPlaces(app);
    return true;
}

/* updateCars; ebx, ecx, edx and esi as the caller had them, eax as the
 * places' sort leaves it. */
void updateRaceCars(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx | kEsi);
    if (!updateCars(app, cpu))
        return;
    cpu.eax = sortedCount(app);
    cpu.esp += 4;
}

/* 0x42cff0: the frame counter ([0x7d3684]) modulo 32, in 32nds, onto the
 * x87's stack; eax the counter / 32. */
Float fractionOfFrame(App* app)
{
    return fp::mul(Float(i32(at(app, kFrameCounter)) % 0x20), f64(app, kThirtySecond));
}

/* fractionOfFrame onto the x87's stack; eax the counter / 32 as the code
 * leaves it. */
void frameFraction(App* app, CPU& cpu)
{
    cpu.eax = reg32(i32(at(app, kFrameCounter)) / 0x20);
    cpu.fpu.push(fractionOfFrame(app));
    cpu.esp += 4;
}

/* 0x42d020: the car eax's headlights' beam (if it has them, +0x734): its
 * point (+0x780) the car's place (+0x98) and its forward axis (+0xcc) scaled
 * by the frame's fraction (0x4dffb0); a matrix of the car's (+0x738, its
 * axes copied, sizes 3.5, 15 and 30, its colours) turned by the fraction
 * (0x4eaab0) and transposed into +0x75c; the point less the car's in it
 * (+0x78c). */
/* The matrix at `matrix` times `vector`, into `out`: the code's inlined
 * product, each row's three products summed in its order. */
void transformPoint(App* app, reg32 matrix, const float (&vector)[3], reg32 out)
{
    Float first[3], second[3], sum[3], third[3];
    for (reg32 k = 0; k < 3; ++k)
        first[k] = fp::mul(Float(vector[0]), f32(app, matrix + k * 4));
    for (reg32 k = 0; k < 3; ++k)
        second[k] = fp::mul(Float(vector[1]), f32(app, matrix + 0xc + k * 4));
    for (reg32 k = 0; k < 3; ++k)
        sum[k] = fp::add(first[k], second[k]);
    for (reg32 k = 0; k < 3; ++k)
        third[k] = fp::mul(Float(vector[2]), f32(app, matrix + 0x18 + k * 4));
    for (reg32 k = 0; k < 3; ++k)
        sum[k] = fp::add(sum[k], third[k]);
    for (reg32 k = 0; k < 3; ++k)
        putFloat(app, out + k * 4, float(sum[k]));
}

/* The point less the car's place, negated, as the code makes it. */
void backFrom(App* app, reg32 point, float (&back)[3])
{
    for (reg32 k = 0; k < 3; ++k)
        back[k] = float(-f32(app, point + k * 4));
}

std::optional<bool> placeLights(App* app, CPU& cpu, reg32 car)
{
    if (at(app, car + 0x734) == 0)
        return false;
    const float fraction = float(fractionOfFrame(app));
    if (!addVectors(app, cpu, car + 0x98, car + 0xcc, car + 0x780))
        return std::nullopt;
    for (reg32 k = 0; k < 9; ++k)
        put(app, car + 0x738 + k * 4, at(app, car + kCarAxes + k * 4));
    put(app, car + 0x798, 0x40600000);  // 3.5
    put(app, car + 0x79c, 0x41700000);  // 15
    put(app, car + 0x7a0, 0x41f00000);  // 30
    put(app, car + 0x7a4, 0xff8080ff);
    put(app, car + 0x7a8, 0xffff8080);
    if (!turn(app, cpu, kTurnMatrix, car + 0x738, floatBits(fraction)) || !transpose(app, cpu, car + 0x738, car + 0x75c))
        return std::nullopt;
    float back[3];
    backFrom(app, car + 0x780, back);
    transformPoint(app, car + 0x75c, back, car + 0x78c);
    return true;
}

/* placeLights of eax; every register but eax as the caller had it, eax the
 * address past the point's copy in the code's frame as its product leaves
 * it -- the car when it has no lights. */
void placeHeadlights(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    const std::optional<bool> placed = placeLights(app, cpu, cpu.eax);
    if (!placed)
        return;
    if (*placed)
        cpu.eax = cpu.esp - 0x20;
    cpu.esp += 4;
}

/* 0x42d160: the car eax's shadow and lights' frame (if +0x660; its
 * headlights' first, if +0x200 has 0x20 and it has them): a point ahead of
 * the car's place along its forward axis (+0x10c) and its up axis (+0x67c,
 * a quarter of +0x11c), to +0x6ac; a matrix of the car's axes (+0x664),
 * sized and coloured as the camera's car and the race's light have it, turned
 * (0x4ea9e0) and transposed into +0x688; the point less the car's in it
 * (+0x6b8). */
std::optional<bool> placeShadow(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x28);
    const reg32 point = frame - 0x28, step = frame - 0x10;
    if ((byte(app, car + 0x200) & 0x20) != 0 && at(app, car + 0x734) != 0 && !placeLights(app, cpu, car))
        return std::nullopt;
    if (at(app, car + 0x660) == 0)
        return false;
    for (reg32 k = 0; k < 3; ++k)
        put(app, point + k * 4, at(app, car + 0x98 + k * 4));
    if (!scaleVectors(app, cpu, car + 0xcc, at(app, car + 0x10c), step) || !addVectors(app, cpu, point, step, point))
        return std::nullopt;
    const float up = float(fp::mul(f32(app, car + 0x11c), f64(app, kQuarter)));
    if (!scaleVectors(app, cpu, car + 0x67c, floatBits(up), step) || !addVectors(app, cpu, point, step, point))
        return std::nullopt;
    for (reg32 k = 0; k < 3; ++k)
        put(app, car + 0x6ac + k * 4, at(app, point + k * 4));
    for (reg32 k = 0; k < 9; ++k)
        put(app, car + 0x664 + k * 4, at(app, car + kCarAxes + k * 4));
    // The camera's car in a race with the race's light on, or not.
    const bool lit = gameSettings(app).detailFlagFc == 1 && at(app, kLight) != 0;
    reg32 width, length, colour, angle;
    if (at(app, car + 0x660) == 1)
    {
        width = lit ? 0x41200000 : 0x41a00000;  // 10, 20
        length = 0x42200000;                    // 40
        colour = lit ? 0xb0b0b0b0 : 0xffffffff;
        angle = 0x3d800000;  // 1/16
    }
    else
    {
        width = lit ? 0x41a00000 : 0x42200000;  // 20, 40
        length = 0x42700000;                    // 60
        colour = lit ? 0xe0e0e0e0 : 0xffffffff;
        angle = 0x3d000000;  // 1/32
    }
    put(app, car + 0x6c8, width);
    put(app, car + 0x6cc, length);
    put(app, car + 0x6d0, colour);
    put(app, car + 0x6c4, 0x3f800000);  // 1
    if (!turn(app, cpu, kTurnAxes, car + 0x664, angle))
        return std::nullopt;
    put(app, car + 0x6d4, 0);
    if (!transpose(app, cpu, car + 0x664, car + 0x688))
        return std::nullopt;
    float back[3];
    backFrom(app, car + 0x6ac, back);
    transformPoint(app, car + 0x688, back, car + 0x6b8);
    return true;
}

/* placeShadow of eax; every register but eax as the caller had it, eax the
 * address past the point's copy in the code's frame as its product leaves
 * it -- the car when it has no shadow. */
void placeCarShadow(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    const reg32 car = cpu.eax;
    const std::optional<bool> placed = placeShadow(app, cpu, car);
    if (!placed)
        return;
    cpu.eax = *placed ? cpu.esp - 0x28 : car;
    cpu.esp += 4;
}

/* 0x42d3e0: the race's places: the cars by place sorted (insertion) by
 * their node (+0x1c) -- reversed when backwards, through the frame -- each
 * car's place (+0x1fc) set; the cars by distance sorted by +8. */
/* An insertion sort of the cars' list at `list` by the field `field`. */
void sortCars(App* app, reg32 list, reg32 field, i32 count)
{
    for (i32 i = 1; i < count; ++i)
    {
        const reg32 car = at(app, list + reg32(i) * 4);
        const i32 key = i32(at(app, car + field));
        i32 j = i - 1;
        for (; j >= 0; --j)
        {
            const reg32 moved = at(app, list + reg32(j) * 4);
            if (key >= i32(at(app, moved + field)))
                break;
            put(app, list + reg32(j + 1) * 4, moved);
        }
        put(app, list + reg32(j + 1) * 4, car);
    }
}

void sortPlaces(App* app)
{
    const i32 count = i32(at(app, kCarCount));
    sortCars(app, kByPlace, kCarNode, count);
    if (gameSettings(app).backwards != 0 && count > 0)
    {
        std::vector<reg32> reversed(static_cast<size_t>(count));
        for (i32 i = 0; i < count; ++i)
            reversed[size_t(i)] = at(app, kByPlace + reg32(count - 1 - i) * 4);
        for (i32 i = 0; i < count; ++i)
            put(app, kByPlace + reg32(i) * 4, reversed[size_t(i)]);
    }
    for (i32 i = 0; i < count; ++i)
        put(app, at(app, kByPlace + reg32(i) * 4) + 0x1fc, reg32(i));
    sortCars(app, kByDistance, kCarDistance, count);
    put(app, kCarCount, reg32(count));
}

/* What eax is left after sortPlaces: the last sort's count, 1 at least. */
reg32 sortedCount(App* app)
{
    const i32 count = i32(at(app, kCarCount));
    return count > 1 ? reg32(count) : 1;
}

void sortRaceCars(App* app, CPU& cpu)
{
    sortPlaces(app);
    cpu.eax = sortedCount(app);
    cpu.esp += 4;
}

/* 0x42d500: a hook of the game's that does nothing -- push ebp, mov ebp,
 * esp, pop ebp (a dozen callers). */
void noHook(App* app)
{
    (void)app;
}

void emptyHook(win32::WinApplication* app, x86::CPU& cpu)
{
    noHook(app);
    cpu.esp += 4;
}

/* 0x42d520: another that does nothing, the same. */
void noHook2(App* app)
{
    (void)app;
}

void emptyHook2(win32::WinApplication* app, x86::CPU& cpu)
{
    noHook2(app);
    cpu.esp += 4;
}

/* 0x42d620: the message text eax shown as the race's message lines
 * (0x436e10): whole if it fits (0x441900), else a line at a time -- copied
 * into the frame (100 bytes of it) and broken where the line no longer fits.
 * The two lines' 100 bytes start as the zeros nfs3.exe keeps at 0x42d550 and
 * 0x42d5b4. */
std::optional<bool> showText(App* app, CPU& cpu, reg32 text)
{
    const GuestFrame frame(cpu, 0x18, 0xcc);
    const reg32 copy = frame - 0xcc, line = frame - 0x68;
    constexpr reg32 kMost = 0x64;  // a line's bytes, and the text's copied
    for (reg32 i = 0; i < kMost; i += 4)
        put(app, copy + i, at(app, kEmptyCopy + i));
    for (reg32 i = 0; i < kMost; i += 4)
        put(app, line + i, at(app, kEmptyLine + i));
    const std::optional<reg32> fits = textFits(app, cpu, text);
    if (!fits)
        return std::nullopt;
    if (*fits != 0)
    {
        if (!showLine(app, cpu, text))
            return std::nullopt;
        return true;
    }
    if (!fill(app, cpu, line, 0, kMost) || !copyText(app, cpu, copy, text, kMost))
        return std::nullopt;
    const reg32 length = lengthOf(app, copy);
    reg32 count = 0;  // the line's characters
    for (reg32 next = 0; i32(next) < i32(length);)
    {
        ++count;
        putByte(app, line + count - 1, byte(app, copy + next));
        ++next;
        const std::optional<reg32> fit = textFits(app, cpu, line);
        if (!fit)
            return std::nullopt;
        if (*fit == 0)
        {
            // The line full: ended, shown, cleared, the character again.
            putByte(app, line + count, 0);
            if (!showLine(app, cpu, line, kMost))
                return std::nullopt;
            --next;
            if (!fill(app, cpu, line, 0, kMost))
                return std::nullopt;
            count = 0;
        }
        if (i32(next) >= i32(kMost))
            break;
    }
    if (lengthOf(app, line) == 0)
        return false;
    if (!showLine(app, cpu, line))
        return std::nullopt;
    return true;
}

/* showText of eax; every register but eax as the caller had it, eax 0 (the
 * code's length count's) when it showed nothing. */
void showMessage(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    const std::optional<bool> shown = showText(app, cpu, cpu.eax);
    if (!shown)
        return;
    if (!*shown)
        cpu.eax = 0;
    cpu.esp += 4;
}

/* 0x42d710: the message text edx (ebx long, after its first byte) sent to
 * every other machine of a network race (the players' list at 0x7a21f0 from
 * 1, but the machine eax) while the network is up ([0x7a22da], [0x7a22e4] 1),
 * and its text added to the message lines; [0x7a22a8] 1.  No caller but a
 * pointer. */
bool sendToOthers(App* app, CPU& cpu, reg32 from, reg32 text, reg32 bytes)
{
    if (half(app, kNetworkUp) != 0 && at(app, kNetworkMode) == 1)
        for (i32 machine = 1; machine < i32(at(app, kMachines)) >> 16; ++machine)
        {
            const reg32 player = at(app, kPlayers + reg32(machine) * 4);
            if (player != from && !sendPacket(app, cpu, player, text, bytes))
                return false;
        }
    {
        const Kept kept(cpu, kAllButEax);
        if (!appendLine(app, cpu, text + 1))
            return false;
    }
    putHalf(app, kMessageSent, 1);
    return true;
}

/* sendToOthers from eax of edx, ebx long; ecx, esi and edi as the caller had
 * them, ebx 1, edx as the code leaves it: the network's mode when that is not
 * 1, 1 with no other machines, the last machine's player when that is the
 * sender, else what the packet's call leaves (the caller's with the network
 * down). */
void sendMessage(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEsi | kEdi);
    const reg32 from = cpu.eax;
    if (!sendToOthers(app, cpu, from, cpu.edx, cpu.ebx))
        return;
    if (half(app, kNetworkUp) != 0)
    {
        const reg32 mode = at(app, kNetworkMode);
        const i32 machines = i32(at(app, kMachines)) >> 16;
        if (mode != 1)
            cpu.edx = mode;
        else if (machines <= 1)
            cpu.edx = 1;
        else if (at(app, kPlayers + reg32(machines - 1) * 4) == from)
            cpu.edx = from;
    }
    cpu.ebx = 1;
    cpu.esp += 4;
}

/* 0x42d790: the message box opened (0x436c40, its description in the frame:
 * 1, 10, 100, 0, 0, 0, 7, 1) and the message lines made afresh. */
bool openAllMessages(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0x20);
    // memmove's stand-in, which the box's opening calls on the description,
    // logs the words past it too: the registers the code pushed.
    frame.pushed(app, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    const reg32 box = frame - 0x20;
    constexpr reg32 kBox[] = {1, 0xa, 0x64, 0, 0, 0, 7, 1};
    for (reg32 i = 0; i < 8; ++i)
        put(app, box + i * 4, kBox[i]);
    const std::optional<reg32> opened = guest::direct<0x436c40, collide::openChannels>(app, cpu, box);
    if (!opened)
        return false;
    cpu.eax = *opened;
    const Kept kept(cpu, kAllButEax);
    return openLines(app, cpu);
}

/* openAllMessages; ebx, ecx, edx and esi as the caller had them. */
void openMessages(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx | kEsi);
    if (!openAllMessages(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42d7f0: the message box closed (0x436cf0) and the message lines with
 * it.  The box's closing by address: what it leaves in eax goes on to the
 * lines' lock (collide::closeChannels gives back only whether the game went
 * on). */
bool closeAllMessages(App* app, CPU& cpu)
{
    if (!guest::call(app, cpu, kCloseMessageBox))
        return false;
    const Kept kept(cpu, kAllButEax);
    return closeLines(app, cpu);
}

void closeMessages(App* app, CPU& cpu)
{
    if (!closeAllMessages(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42d800: the message box cleared (0x436d40) and the message lines after
 * the first. */
bool clearAllMessages(App* app, CPU& cpu)
{
    const reg32 cleared = guest::direct<0x436d40, collide::clearAllChannels>(app, cpu);
    if (cpu.terminate)
        return false;
    cpu.eax = cleared;
    const Kept kept(cpu, kAllButEax);
    return clearLines(app, cpu);
}

void clearMessages(App* app, CPU& cpu)
{
    if (!clearAllMessages(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42d810: the message text edx sent to the other machines (0x4a60e0, a
 * packet of a newline byte and the text, its ending too) and shown. */
std::optional<bool> sendTyped(App* app, CPU& cpu, reg32 text)
{
    const GuestFrame frame(cpu, 0x14, 0x68);
    const reg32 packet = frame - 0x68;
    const reg32 bytes = lengthOf(app, text) + 1;  // its ending too
    putByte(app, packet, 0xa);
    if (!moveMemory(app, cpu, text, packet + 1, bytes) || !sendPacket(app, cpu, 0, packet, bytes + 1))
        return std::nullopt;
    const Nested nested(cpu, kAllButEax);
    return showText(app, cpu, text);
}

/* sendTyped of edx; ebx, ecx, esi and edi as the caller had them, edx as the
 * packet's call leaves it, eax 0 when nothing was shown. */
void sendTypedMessage(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEsi | kEdi);
    const std::optional<bool> shown = sendTyped(app, cpu, cpu.edx);
    if (!shown)
        return;
    if (!*shown)
        cpu.eax = 0;
    cpu.esp += 4;
}

/* 0x42d860: a message line allocated (0x70 bytes) and cleared; or 0. */
std::optional<reg32> newLine(App* app, CPU& cpu)
{
    const std::optional<reg32> line = allocate(app, cpu, kLineName, kLineBytes, kBlockFlags);
    if (!line || (*line != 0 && !fill(app, cpu, *line, 0, kLineBytes)))
        return std::nullopt;
    return line;
}

/* newLine; ebx, ecx and edx as the caller had them. */
void newMessageLine(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const std::optional<reg32> line = newLine(app, cpu);
    if (!line)
        return;
    cpu.eax = *line;
    cpu.esp += 4;
}

/* 0x42d8a0: the message line eax freed, if there is one. */
bool freeLine(App* app, CPU& cpu, reg32 line)
{
    return line == 0 || freeBlock(app, cpu, line);
}

/* freeLine of eax. */
void freeMessageLine(App* app, CPU& cpu)
{
    if (!freeLine(app, cpu, cpu.eax))
        return;
    cpu.esp += 4;
}

/* 0x42d8b0: a message line of the text edx (100 bytes of it, strncpy), from
 * eax, put after the line ebx; the line, or 0 (no text, no memory). */
std::optional<reg32> insertLine(App* app, CPU& cpu, reg32 from, reg32 text, reg32 after)
{
    if (text == 0)
        return 0;
    const std::optional<reg32> line = newLine(app, cpu);
    if (!line || *line == 0)
        return line;
    put(app, *line + kLineFrom, from);
    if (!copyText(app, cpu, *line + kLineText, text, 0x64))
        return std::nullopt;
    const reg32 next = at(app, after + kLineNext);
    if (next != 0)
        put(app, next + kLineBefore, *line);
    put(app, *line + kLineNext, at(app, after + kLineNext));
    put(app, *line + kLineBefore, after);
    put(app, after + kLineNext, *line);
    return line;
}

/* insertLine from eax of edx after ebx; ecx, esi and edi as the caller had
 * them (ebx and edx as the code leaves them are not compared). */
void insertMessageLine(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEsi | kEdi);
    const std::optional<reg32> line = insertLine(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!line)
        return;
    cpu.eax = *line;
    cpu.esp += 4;
}

/* 0x42d910: the message line eax and every one after it freed. */
bool freeLines(App* app, CPU& cpu, reg32 line)
{
    while (line != 0)
    {
        const reg32 next = at(app, line + kLineNext);
        if (!freeLine(app, cpu, line))
            return false;
        line = next;
    }
    return true;
}

/* freeLines from eax; ecx and edx as the caller had them. */
void freeMessageLines(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!freeLines(app, cpu, cpu.eax))
        return;
    cpu.esp += 4;
}

/* The lines after the first, from `rest`, freed and the first the last
 * again. */
bool dropRest(App* app, CPU& cpu, reg32 rest)
{
    if (!freeLines(app, cpu, rest))
        return false;
    const reg32 head = at(app, kLines);
    put(app, at(app, head + kHeadFirst) + kLineNext, 0);
    put(app, head + kHeadLast, at(app, head + kHeadFirst));
    return true;
}

/* The lines' lock let go of and freed (0x4eb360), their head freed and 0. */
bool dropHead(App* app, CPU& cpu)
{
    if (!lockLines<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu)
        || !freeLock(app, cpu, at(app, at(app, kLines) + kHeadLock))
        || !freeBlock(app, cpu, at(app, kLines)))
        return false;
    put(app, kLines, 0);
    return true;
}

/* 0x42d940: the message lines after the first freed, under their lock. */
bool clearLines(App* app, CPU& cpu)
{
    if (!lockLines<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu))
        return false;
    const reg32 head = at(app, kLines);
    if (head != 0)
    {
        const reg32 first = at(app, head + kHeadFirst);
        if (first != 0)
        {
            const reg32 rest = at(app, first + kLineNext);
            if (rest != 0 && !dropRest(app, cpu, rest))
                return false;
        }
    }
    return lockLines<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu);
}

/* clearLines; every register but eax as the caller had it. */
void clearMessageLines(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!clearLines(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42d9b0: each message line after the first shown (showMessage), and
 * then freed, under their lock. */
bool showLines(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0);
    if (!lockLines<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu))
        return false;
    const reg32 head = at(app, kLines);
    if (head != 0 && at(app, head + kHeadFirst) != 0)
    {
        for (reg32 line = at(app, at(app, head + kHeadFirst) + kLineNext); line != 0; line = at(app, line + kLineNext))
        {
            const Nested nested(cpu, kAllButEax);
            if (!showText(app, cpu, line + kLineText))
                return false;
        }
        const reg32 rest = at(app, at(app, at(app, kLines) + kHeadFirst) + kLineNext);
        if (rest != 0 && !dropRest(app, cpu, rest))
            return false;
    }
    return lockLines<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu);
}

/* showLines; every register but eax as the caller had it. */
void showMessageLines(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!showLines(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42da30: the text eax added as the last message line, numbered by
 * [0x55379c], under their lock -- if there is a text and the lines are. */
bool appendLine(App* app, CPU& cpu, reg32 text)
{
    if (!lockLines<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu))
        return false;
    const reg32 head = at(app, kLines);
    if (text != 0 && head != 0 && at(app, head + kHeadFirst) != 0)
    {
        const reg32 last = at(app, head + kHeadLast);
        if (last != 0)
        {
            const reg32 serial = at(app, kLineSerial) + 1;
            put(app, kLineSerial, serial);
            const std::optional<reg32> line = insertLine(app, cpu, serial, text, last);
            if (!line)
                return false;
            if (*line != 0)
                put(app, at(app, kLines) + kHeadLast, *line);
        }
    }
    return lockLines<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu);
}

/* appendLine of eax; every register but eax as the caller had it. */
void appendMessageLine(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!appendLine(app, cpu, cpu.eax))
        return;
    cpu.esp += 4;
}

/* 0x42daf0: the message lines freed, under their lock, the lock freed
 * (0x4eb360) and the lines' head freed and 0. */
bool closeLines(App* app, CPU& cpu)
{
    const reg32 head = at(app, kLines);
    if (head == 0)
        return true;
    if (!lockCall<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, cpu.eax,
                                                                                 at(app, head + kHeadLock)))
        return false;
    const reg32 first = at(app, at(app, kLines) + kHeadFirst);
    if (first != 0)
    {
        if (!freeLines(app, cpu, first))
            return false;
        put(app, at(app, kLines) + kHeadFirst, 0);
        put(app, at(app, kLines) + kHeadLast, 0);
    }
    return dropHead(app, cpu);
}

/* closeLines; every register but eax as the caller had it. */
void closeMessageLines(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!closeLines(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42db70: the message lines made afresh: the old freed, their head (12
 * bytes: the lock, the first line, the last) allocated, its lock made
 * (0x4eb2e0) and, under it, an empty first line -- none, and all of it
 * freed again. */
bool openLines(App* app, CPU& cpu)
{
    if (!closeLines(app, cpu))
        return false;
    const std::optional<reg32> head = allocate(app, cpu, kLineName, 0xc, kBlockFlags);
    if (!head)
        return false;
    put(app, kLines, *head);
    if (*head == 0)
        return true;
    const std::optional<reg32> lock = makeLock(app, cpu);
    if (!lock)
        return false;
    put(app, at(app, kLines) + kHeadLock, *lock);
    if (!lockCall<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, *lock, *lock))
        return false;
    put(app, at(app, kLines) + kHeadFirst, 0);
    put(app, at(app, kLines) + kHeadLast, 0);
    const std::optional<reg32> line = newLine(app, cpu);
    if (!line)
        return false;
    if (*line == 0)
        return dropHead(app, cpu);
    put(app, at(app, kLines) + kHeadFirst, *line);
    put(app, at(app, kLines) + kHeadLast, *line);
    return lockLines<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu);
}

/* openLines; every register but eax as the caller had it. */
void openMessageLines(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    if (!openLines(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42dc20: the race controls' thread: an event made ([0x5efdf0]), then
 * for ever the event waited for and the race's controls read (0x48f550).
 * Its loop's head the generated code's safepoint. */
void raceControlsThread(App* app, CPU& cpu)
{
    const std::optional<reg32> event = createEvent(app, cpu);
    if (!event)
        return;
    put(app, kControlsEvent, *event);
    for (;;)
    {
        if (app->contextWanted())
            app->yieldContext(cpu);
        const std::optional<reg32> waited = waitEvent(app, cpu, at(app, kControlsEvent));
        if (!waited)
            return;
        const std::optional<reg32> read = guest::direct<0x48f550, controls::readControls>(app, cpu, *waited);
        if (!read)
            return;
        cpu.eax = *read;
    }
}

/* 0x42dc40: the race controls' event set (0x4dfb20, eax 0x7a3aac). */
bool wakeControls(App* app, CPU& cpu)
{
    return postEvent(app, cpu, kControlsThread, at(app, kControlsEvent));
}

/* wakeControls; edx as the caller had it. */
void wakeRaceControls(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEdx);
    if (!wakeControls(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42dc60: the game's timer's call: the ticks counted ([0x5efdf4],
 * [0x5efe00], eax the last), every second of them counted again
 * ([0x5efdf8]) and every second of those -- a fourth tick -- the race
 * controls woken ([0x5efdfc] counts them). */
bool countTick(App* app, CPU& cpu)
{
    put(app, kTicks, at(app, kTicks) + 1);
    put(app, kAllTicks, at(app, kAllTicks) + 1);
    if ((byte(app, kTicks) & 1) != 0)
        return true;
    put(app, kHalfTicks, at(app, kHalfTicks) + 1);
    if ((byte(app, kHalfTicks) & 1) != 0)
        return true;
    put(app, kWakes, at(app, kWakes) + 1);
    return wakeControls(app, cpu);
}

/* countTick; edx and esi as the caller had them, eax the ticks before this
 * one unless the event's call leaves its own. */
void raceControlsTick(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEdx | kEsi);
    cpu.eax = at(app, kAllTicks);
    if (!countTick(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42dcb0: the race controls' ticks from 0, and, the first time
 * ([0x5537a4]), raceControlsTick given to the game's timer (0x48ee30) and
 * the counts -- 0x79c4ac's too -- 0. */
bool startTick(App* app, CPU& cpu)
{
    put(app, kAllTicks, 0);
    if (at(app, kTicking) != 0)
        return true;
    put(app, kTicking, 1);
    if (!addTimer(app, cpu, kTickFunction))
        return false;
    for (const reg32 count : {kTicks, kHalfTicks, kWakes, kTimerCount})
        put(app, count, 0);
    return true;
}

/* startTick; ebx and edx as the caller had them. */
void startRaceControlsTick(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEdx);
    if (!startTick(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42dd00: raceControlsTick taken from the game's timer (0x48ee90), if it
 * was given. */
bool stopTick(App* app, CPU& cpu)
{
    if (at(app, kTicking) == 0)
        return true;
    put(app, kTicking, 0);
    return removeTimer(app, cpu, kTickFunction);
}

/* stopTick; ecx as the caller had it. */
void stopRaceControlsTick(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!stopTick(app, cpu))
        return;
    cpu.esp += 4;
}

/* 0x42dd60: whether the corner ecx, ebx, [esp+4] (each -1, 0 or 1, as
 * the box's axes go) of the car edx's box -- its half sizes +0x114, +0x118,
 * +0x11c -- is inside the car eax's box: the corner taken into eax's frame
 * by the axes at 0x5efee0 and the place at 0x5efe60 (the cars' frames one in
 * the other, made before) to 0x5eff04, 0x5efeb4, 0x5efeb8 and held against
 * eax's sizes.  Inside: 1, and the corner from edx's axes and sizes in the
 * world (0x5efe78, from edx's place +0x98) and from edx's middle (0x5efe84);
 * else 0.  stdcall's 4 bytes. */
bool cornerIn(App* app, reg32 box, reg32 other, i32 firstSign, i32 secondSign, i32 thirdSign)
{
    float first, second, third;
    const Float t1 = Float(i32(secondSign));
    second = float(t1);
    const Float t2 = fp::mul(t1, f32(app, kAxesBA + 4));
    const Float t3 = Float(i32(firstSign));
    first = float(t3);
    const Float t4 = fp::mul(t3, f32(app, kAxesBA));
    const Float t5 = Float(first);
    const Float t6 = fp::mul(t5, f32(app, kAxesBA + 12));
    const Float t7 = Float(first);
    const Float t8 = fp::mul(t7, f32(app, kAxesBA + 24));
    const Float t9 = Float(second);
    const Float t10 = fp::mul(t9, f32(app, kAxesBA + 16));
    const Float t11 = Float(second);
    const Float t12 = fp::add(t2, t4);
    const Float t13 = fp::mul(t11, f32(app, kAxesBA + 28));
    const Float t14 = Float(i32(thirdSign));
    third = float(t14);
    const Float t15 = fp::mul(t14, f32(app, kAxesBA + 8));
    const Float t16 = fp::add(t6, t10);
    const Float t17 = fp::add(t8, t13);
    const Float t18 = Float(third);
    const Float t19 = fp::mul(t18, f32(app, kAxesBA + 20));
    const Float t20 = Float(third);
    const Float t21 = fp::add(t12, t15);
    const Float t22 = fp::mul(t20, f32(app, kAxesBA + 32));
    const Float t23 = f32(app, kPlaceBA);
    const Float t24 = fp::add(t16, t19);
    const Float t25 = fp::add(t17, t22);
    const Float t26 = f32(app, kPlaceBA + 4);
    const Float t27 = fp::add(t21, t23);
    const Float t28 = f32(app, kPlaceBA + 8);
    putFloat(app, kCorner0, float(t27));
    const Float t29 = fp::add(t24, t26);
    const Float t30 = fp::add(t25, t28);
    putFloat(app, kCorner1, float(t29));
    putFloat(app, kCorner2, float(t30));
    if (t30 > f32(app, box + 0x11c))
        return false;
    const Float t31 = f32(app, box + 0x11c);
    const Float t32 = -t31;
    if (t32 > f32(app, kCorner2))
        return false;
    const Float t33 = f32(app, kCorner0);
    if (t33 > f32(app, box + 0x114))
        return false;
    const Float t34 = f32(app, box + 0x114);
    const Float t35 = -t34;
    if (t35 > f32(app, kCorner0))
        return false;
    const Float t36 = f32(app, kCorner1);
    if (t36 > f32(app, box + 0x118))
        return false;
    const Float t37 = f32(app, box + 0x118);
    const Float t38 = -t37;
    if (t38 > f32(app, kCorner1))
        return false;
    const Float t39 = Float(first);
    const Float t40 = fp::mul(t39, f32(app, other + 0xc0));
    const Float t41 = Float(first);
    const Float t42 = fp::mul(t41, f32(app, other + 0xc4));
    const Float t43 = Float(first);
    const Float t44 = fp::mul(t43, f32(app, other + 0xc8));
    const Float t45 = f32(app, other + 0xc0);
    const Float t46 = fp::mul(t45, f32(app, other + 0x114));
    const Float t47 = Float(second);
    const Float t48 = fp::mul(t47, f32(app, other + 0xcc));
    const Float t49 = f32(app, other + 0xc4);
    const Float t50 = fp::add(t40, t48);
    const Float t51 = fp::mul(t49, f32(app, other + 0x114));
    const Float t52 = Float(third);
    const Float t53 = fp::mul(t52, f32(app, other + 0xd8));
    const Float t54 = f32(app, other + 0xc8);
    const Float t55 = fp::mul(t54, f32(app, other + 0x114));
    const Float t56 = fp::add(t50, t53);
    const Float t57 = fp::mul(t46, Float(first));
    putFloat(app, kContact, float(t56));
    const Float t58 = Float(second);
    const Float t59 = fp::mul(t58, f32(app, other + 0xd0));
    const Float t60 = fp::mul(t51, Float(first));
    const Float t61 = fp::add(t42, t59);
    const Float t62 = fp::mul(t55, Float(first));
    const Float t63 = Float(third);
    const Float t64 = fp::mul(t63, f32(app, other + 0xdc));
    const Float t65 = Float(second);
    const Float t66 = fp::mul(t65, f32(app, other + 0xd4));
    const Float t67 = fp::add(t61, t64);
    const Float t68 = fp::add(t57, f32(app, other + 0x98));
    putFloat(app, kContact + 4, float(t67));
    const Float t69 = f32(app, other + 0xcc);
    const Float t70 = fp::mul(t69, f32(app, other + 0x118));
    const Float t71 = fp::add(t44, t66);
    const Float t72 = fp::add(t60, f32(app, other + 0x9c));
    const Float t73 = Float(third);
    const Float t74 = fp::mul(t73, f32(app, other + 0xe0));
    const Float t75 = f32(app, other + 0xd0);
    const Float t76 = fp::mul(t75, f32(app, other + 0x118));
    const Float t77 = fp::add(t71, t74);
    const Float t78 = fp::add(t62, f32(app, other + 0xa0));
    putFloat(app, kContact + 8, float(t77));
    const Float t79 = f32(app, other + 0xd4);
    const Float t80 = fp::mul(t79, f32(app, other + 0x118));
    const Float t81 = fp::mul(t70, Float(second));
    const Float t82 = fp::mul(t76, Float(second));
    const Float t83 = fp::add(t68, t81);
    const Float t84 = fp::mul(t80, Float(second));
    const Float t85 = f32(app, other + 0xd8);
    const Float t86 = fp::mul(t85, f32(app, other + 0x11c));
    const Float t87 = fp::add(t72, t82);
    const Float t88 = fp::add(t78, t84);
    const Float t89 = f32(app, other + 0xdc);
    const Float t90 = fp::mul(t89, f32(app, other + 0x11c));
    const Float t91 = f32(app, other + 0xe0);
    const Float t92 = fp::mul(t91, f32(app, other + 0x11c));
    const Float t93 = fp::mul(t86, Float(third));
    const Float t94 = fp::mul(t90, Float(third));
    const Float t95 = fp::mul(t92, Float(third));
    const Float t96 = fp::add(t83, t93);
    const Float t97 = fp::add(t87, t94);
    const Float t98 = fp::add(t88, t95);
    putFloat(app, kContactOther, float(t96));
    putFloat(app, kContactOther + 4, float(t97));
    putFloat(app, kContactOther + 8, float(t98));
    return true;
}

/* cornerIn of eax, edx, the signs ebx, ecx and [esp+4]: eax 1 inside, else 0. */
void cornerInBox(App* app, CPU& cpu)
{
    cpu.eax = cornerIn(app, cpu.eax, cpu.edx, i32(cpu.ebx), i32(cpu.ecx), i32(at(app, cpu.esp + 4))) ? 1 : 0;
    cpu.esp += 8;
}

/* 0x42e010: cornerInBox the other way round: the corner of the car eax's
 * box taken into the car edx's frame (the axes at 0x5efe90, less the place
 * at 0x5efe6c), held against edx's sizes; the corner from eax's axes and
 * sizes made negative (0x5efe78) and from eax's middle (0x5efe84). */
bool cornerInBack(App* app, reg32 box, reg32 other, i32 firstSign, i32 secondSign, i32 thirdSign)
{
    float first, second, third;
    const Float t1 = Float(i32(secondSign));
    second = float(t1);
    const Float t2 = fp::mul(t1, f32(app, kAxesAB + 12));
    const Float t3 = Float(i32(firstSign));
    first = float(t3);
    const Float t4 = fp::mul(t3, f32(app, kAxesAB));
    const Float t5 = Float(first);
    const Float t6 = fp::mul(t5, f32(app, kAxesAB + 4));
    const Float t7 = Float(first);
    const Float t8 = fp::mul(t7, f32(app, kAxesAB + 8));
    const Float t9 = Float(second);
    const Float t10 = fp::mul(t9, f32(app, kAxesAB + 16));
    const Float t11 = Float(second);
    const Float t12 = fp::add(t2, t4);
    const Float t13 = fp::mul(t11, f32(app, kAxesAB + 20));
    const Float t14 = Float(i32(thirdSign));
    third = float(t14);
    const Float t15 = fp::mul(t14, f32(app, kAxesAB + 24));
    const Float t16 = fp::add(t6, t10);
    const Float t17 = fp::add(t8, t13);
    const Float t18 = Float(third);
    const Float t19 = fp::mul(t18, f32(app, kAxesAB + 28));
    const Float t20 = Float(third);
    const Float t21 = fp::add(t12, t15);
    const Float t22 = fp::mul(t20, f32(app, kAxesAB + 32));
    const Float t23 = f32(app, kPlaceAB);
    const Float t24 = fp::add(t16, t19);
    const Float t25 = fp::add(t17, t22);
    const Float t26 = f32(app, kPlaceAB + 4);
    const Float t27 = fp::sub(t21, t23);
    const Float t28 = f32(app, kPlaceAB + 8);
    putFloat(app, kCorner0, float(t27));
    const Float t29 = fp::sub(t24, t26);
    const Float t30 = fp::sub(t25, t28);
    putFloat(app, kCorner1, float(t29));
    putFloat(app, kCorner2, float(t30));
    if (t30 > f32(app, other + 0x11c))
        return false;
    const Float t31 = f32(app, other + 0x11c);
    const Float t32 = -t31;
    if (t32 > f32(app, kCorner2))
        return false;
    const Float t33 = f32(app, kCorner0);
    if (t33 > f32(app, other + 0x114))
        return false;
    const Float t34 = f32(app, other + 0x114);
    const Float t35 = -t34;
    if (t35 > f32(app, kCorner0))
        return false;
    const Float t36 = f32(app, kCorner1);
    if (t36 > f32(app, other + 0x118))
        return false;
    const Float t37 = f32(app, other + 0x118);
    const Float t38 = -t37;
    if (t38 > f32(app, kCorner1))
        return false;
    const Float t39 = Float(first);
    const Float t40 = fp::mul(t39, f32(app, box + 0xc0));
    const Float t41 = Float(first);
    const Float t42 = fp::mul(t41, f32(app, box + 0xc4));
    const Float t43 = Float(first);
    const Float t44 = fp::mul(t43, f32(app, box + 0xc8));
    const Float t45 = f32(app, box + 0xc0);
    const Float t46 = fp::mul(t45, f32(app, box + 0x114));
    const Float t47 = Float(second);
    const Float t48 = fp::mul(t47, f32(app, box + 0xcc));
    const Float t49 = f32(app, box + 0xc4);
    const Float t50 = fp::add(t40, t48);
    const Float t51 = fp::mul(t49, f32(app, box + 0x114));
    const Float t52 = Float(third);
    const Float t53 = fp::mul(t52, f32(app, box + 0xd8));
    const Float t54 = f32(app, box + 0xc8);
    const Float t55 = fp::mul(t54, f32(app, box + 0x114));
    const Float t56 = fp::add(t50, t53);
    const Float t57 = fp::mul(t46, Float(first));
    const Float t58 = -t56;
    putFloat(app, kContact, float(t58));
    const Float t59 = Float(second);
    const Float t60 = fp::mul(t59, f32(app, box + 0xd0));
    const Float t61 = fp::mul(t51, Float(first));
    const Float t62 = fp::add(t42, t60);
    const Float t63 = fp::mul(t55, Float(first));
    const Float t64 = Float(third);
    const Float t65 = fp::mul(t64, f32(app, box + 0xdc));
    const Float t66 = Float(second);
    const Float t67 = fp::mul(t66, f32(app, box + 0xd4));
    const Float t68 = fp::add(t62, t65);
    const Float t69 = fp::add(t57, f32(app, box + 0x98));
    const Float t70 = -t68;
    putFloat(app, kContact + 4, float(t70));
    const Float t71 = f32(app, box + 0xcc);
    const Float t72 = fp::mul(t71, f32(app, box + 0x118));
    const Float t73 = fp::add(t44, t67);
    const Float t74 = fp::add(t61, f32(app, box + 0x9c));
    const Float t75 = Float(third);
    const Float t76 = fp::mul(t75, f32(app, box + 0xe0));
    const Float t77 = f32(app, box + 0xd0);
    const Float t78 = fp::mul(t77, f32(app, box + 0x118));
    const Float t79 = fp::add(t73, t76);
    const Float t80 = fp::add(t63, f32(app, box + 0xa0));
    const Float t81 = -t79;
    putFloat(app, kContact + 8, float(t81));
    const Float t82 = f32(app, box + 0xd4);
    const Float t83 = fp::mul(t82, f32(app, box + 0x118));
    const Float t84 = fp::mul(t72, Float(second));
    const Float t85 = fp::mul(t78, Float(second));
    const Float t86 = fp::add(t69, t84);
    const Float t87 = fp::mul(t83, Float(second));
    const Float t88 = f32(app, box + 0xd8);
    const Float t89 = fp::mul(t88, f32(app, box + 0x11c));
    const Float t90 = fp::add(t74, t85);
    const Float t91 = fp::add(t80, t87);
    const Float t92 = f32(app, box + 0xdc);
    const Float t93 = fp::mul(t92, f32(app, box + 0x11c));
    const Float t94 = f32(app, box + 0xe0);
    const Float t95 = fp::mul(t94, f32(app, box + 0x11c));
    const Float t96 = fp::mul(t89, Float(third));
    const Float t97 = fp::mul(t93, Float(third));
    const Float t98 = fp::mul(t95, Float(third));
    const Float t99 = fp::add(t86, t96);
    const Float t100 = fp::add(t90, t97);
    const Float t101 = fp::add(t91, t98);
    putFloat(app, kContactOther, float(t99));
    putFloat(app, kContactOther + 4, float(t100));
    putFloat(app, kContactOther + 8, float(t101));
    return true;
}

/* cornerInBack of eax, edx, the signs ebx, ecx and [esp+4]: eax 1 inside, else 0. */
void cornerInBoxBack(App* app, CPU& cpu)
{
    cpu.eax = cornerInBack(app, cpu.eax, cpu.edx, i32(cpu.ebx), i32(cpu.ecx), i32(at(app, cpu.esp + 4))) ? 1 : 0;
    cpu.esp += 8;
}

/* 0x42e2c0: cornerInBox with the corner from edx's axes worked out the
 * other way about -- each axis times its size first, then by the corner's
 * sign -- and stored in another order. */
bool cornerIn2(App* app, reg32 box, reg32 other, i32 firstSign, i32 secondSign, i32 thirdSign)
{
    float first, second, third;
    const Float t1 = Float(i32(secondSign));
    second = float(t1);
    const Float t2 = fp::mul(t1, f32(app, kAxesBA + 4));
    const Float t3 = Float(i32(firstSign));
    first = float(t3);
    const Float t4 = fp::mul(t3, f32(app, kAxesBA));
    const Float t5 = Float(first);
    const Float t6 = fp::mul(t5, f32(app, kAxesBA + 12));
    const Float t7 = Float(first);
    const Float t8 = fp::mul(t7, f32(app, kAxesBA + 24));
    const Float t9 = Float(second);
    const Float t10 = fp::mul(t9, f32(app, kAxesBA + 16));
    const Float t11 = Float(second);
    const Float t12 = fp::add(t2, t4);
    const Float t13 = fp::mul(t11, f32(app, kAxesBA + 28));
    const Float t14 = Float(i32(thirdSign));
    third = float(t14);
    const Float t15 = fp::mul(t14, f32(app, kAxesBA + 8));
    const Float t16 = fp::add(t6, t10);
    const Float t17 = fp::add(t8, t13);
    const Float t18 = Float(third);
    const Float t19 = fp::mul(t18, f32(app, kAxesBA + 20));
    const Float t20 = Float(third);
    const Float t21 = fp::add(t12, t15);
    const Float t22 = fp::mul(t20, f32(app, kAxesBA + 32));
    const Float t23 = f32(app, kPlaceBA);
    const Float t24 = fp::add(t16, t19);
    const Float t25 = fp::add(t17, t22);
    const Float t26 = f32(app, kPlaceBA + 4);
    const Float t27 = fp::add(t21, t23);
    const Float t28 = f32(app, kPlaceBA + 8);
    putFloat(app, kCorner0, float(t27));
    const Float t29 = fp::add(t24, t26);
    const Float t30 = fp::add(t25, t28);
    putFloat(app, kCorner1, float(t29));
    putFloat(app, kCorner2, float(t30));
    if (t30 > f32(app, box + 0x11c))
        return false;
    const Float t31 = f32(app, box + 0x11c);
    const Float t32 = -t31;
    if (t32 > f32(app, kCorner2))
        return false;
    const Float t33 = f32(app, kCorner0);
    if (t33 > f32(app, box + 0x114))
        return false;
    const Float t34 = f32(app, box + 0x114);
    const Float t35 = -t34;
    if (t35 > f32(app, kCorner0))
        return false;
    const Float t36 = f32(app, kCorner1);
    if (t36 > f32(app, box + 0x118))
        return false;
    const Float t37 = f32(app, box + 0x118);
    const Float t38 = -t37;
    if (t38 > f32(app, kCorner1))
        return false;
    const Float t39 = f32(app, other + 0xc0);
    const Float t40 = fp::mul(t39, f32(app, other + 0x114));
    const Float t41 = f32(app, other + 0xc4);
    const Float t42 = fp::mul(t41, f32(app, other + 0x114));
    const Float t43 = f32(app, other + 0xc8);
    const Float t44 = fp::mul(t43, f32(app, other + 0x114));
    const Float t45 = Float(first);
    const Float t46 = fp::mul(t40, t45);
    const Float t47 = fp::mul(t42, t45);
    const Float t48 = fp::add(t46, f32(app, other + 0x98));
    const Float t49 = fp::mul(t45, t44);
    const Float t50 = f32(app, other + 0xcc);
    const Float t51 = fp::mul(t50, f32(app, other + 0x118));
    const Float t52 = fp::add(t47, f32(app, other + 0x9c));
    const Float t53 = fp::add(t49, f32(app, other + 0xa0));
    const Float t54 = f32(app, other + 0xd0);
    const Float t55 = fp::mul(t54, f32(app, other + 0x118));
    const Float t56 = f32(app, other + 0xd4);
    const Float t57 = fp::mul(t56, f32(app, other + 0x118));
    const Float t58 = Float(second);
    const Float t59 = fp::mul(t51, t58);
    const Float t60 = fp::mul(t55, t58);
    const Float t61 = fp::add(t48, t59);
    const Float t62 = fp::mul(t57, t58);
    const Float t63 = f32(app, other + 0xd8);
    const Float t64 = fp::mul(t63, f32(app, other + 0x11c));
    const Float t65 = fp::add(t52, t60);
    const Float t66 = fp::add(t53, t62);
    const Float t67 = f32(app, other + 0xdc);
    const Float t68 = fp::mul(t67, f32(app, other + 0x11c));
    const Float t69 = f32(app, other + 0xe0);
    const Float t70 = fp::mul(t69, f32(app, other + 0x11c));
    const Float t71 = Float(third);
    const Float t72 = fp::mul(t64, t71);
    const Float t73 = fp::mul(t68, t71);
    const Float t74 = fp::mul(t71, t70);
    const Float t75 = fp::add(t61, t72);
    const Float t76 = fp::add(t65, t73);
    const Float t77 = fp::add(t74, t66);
    putFloat(app, kContactOther + 4, float(t76));
    putFloat(app, kContactOther + 8, float(t77));
    putFloat(app, kContactOther, float(t75));
    return true;
}

/* cornerIn2 of eax, edx, the signs ebx, ecx and [esp+4]: eax 1 inside, else 0. */
void cornerInBox2(App* app, CPU& cpu)
{
    cpu.eax = cornerIn2(app, cpu.eax, cpu.edx, i32(cpu.ebx), i32(cpu.ecx), i32(at(app, cpu.esp + 4))) ? 1 : 0;
    cpu.esp += 8;
}

/* 0x42e4f0: cornerInBoxBack worked out as cornerInBox2 is. */
bool cornerInBack2(App* app, reg32 box, reg32 other, i32 firstSign, i32 secondSign, i32 thirdSign)
{
    float first, second, third;
    const Float t1 = Float(i32(secondSign));
    second = float(t1);
    const Float t2 = fp::mul(t1, f32(app, kAxesAB + 12));
    const Float t3 = Float(i32(firstSign));
    first = float(t3);
    const Float t4 = fp::mul(t3, f32(app, kAxesAB));
    const Float t5 = Float(first);
    const Float t6 = fp::mul(t5, f32(app, kAxesAB + 4));
    const Float t7 = Float(first);
    const Float t8 = fp::mul(t7, f32(app, kAxesAB + 8));
    const Float t9 = Float(second);
    const Float t10 = fp::mul(t9, f32(app, kAxesAB + 16));
    const Float t11 = Float(second);
    const Float t12 = fp::add(t2, t4);
    const Float t13 = fp::mul(t11, f32(app, kAxesAB + 20));
    const Float t14 = Float(i32(thirdSign));
    third = float(t14);
    const Float t15 = fp::mul(t14, f32(app, kAxesAB + 24));
    const Float t16 = fp::add(t6, t10);
    const Float t17 = fp::add(t8, t13);
    const Float t18 = Float(third);
    const Float t19 = fp::mul(t18, f32(app, kAxesAB + 28));
    const Float t20 = Float(third);
    const Float t21 = fp::add(t12, t15);
    const Float t22 = fp::mul(t20, f32(app, kAxesAB + 32));
    const Float t23 = f32(app, kPlaceAB);
    const Float t24 = fp::add(t16, t19);
    const Float t25 = fp::add(t17, t22);
    const Float t26 = f32(app, kPlaceAB + 4);
    const Float t27 = fp::sub(t21, t23);
    const Float t28 = f32(app, kPlaceAB + 8);
    putFloat(app, kCorner0, float(t27));
    const Float t29 = fp::sub(t24, t26);
    const Float t30 = fp::sub(t25, t28);
    putFloat(app, kCorner1, float(t29));
    putFloat(app, kCorner2, float(t30));
    if (t30 > f32(app, other + 0x11c))
        return false;
    const Float t31 = f32(app, other + 0x11c);
    const Float t32 = -t31;
    if (t32 > f32(app, kCorner2))
        return false;
    const Float t33 = f32(app, kCorner0);
    if (t33 > f32(app, other + 0x114))
        return false;
    const Float t34 = f32(app, other + 0x114);
    const Float t35 = -t34;
    if (t35 > f32(app, kCorner0))
        return false;
    const Float t36 = f32(app, kCorner1);
    if (t36 > f32(app, other + 0x118))
        return false;
    const Float t37 = f32(app, other + 0x118);
    const Float t38 = -t37;
    if (t38 > f32(app, kCorner1))
        return false;
    const Float t39 = f32(app, box + 0xc0);
    const Float t40 = fp::mul(t39, f32(app, box + 0x114));
    const Float t41 = f32(app, box + 0xc4);
    const Float t42 = fp::mul(t41, f32(app, box + 0x114));
    const Float t43 = f32(app, box + 0xc8);
    const Float t44 = fp::mul(t43, f32(app, box + 0x114));
    const Float t45 = Float(first);
    const Float t46 = fp::mul(t40, t45);
    const Float t47 = fp::mul(t42, t45);
    const Float t48 = fp::add(t46, f32(app, box + 0x98));
    const Float t49 = fp::mul(t45, t44);
    const Float t50 = f32(app, box + 0xcc);
    const Float t51 = fp::mul(t50, f32(app, box + 0x118));
    const Float t52 = fp::add(t47, f32(app, box + 0x9c));
    const Float t53 = fp::add(t49, f32(app, box + 0xa0));
    const Float t54 = f32(app, box + 0xd0);
    const Float t55 = fp::mul(t54, f32(app, box + 0x118));
    const Float t56 = f32(app, box + 0xd4);
    const Float t57 = fp::mul(t56, f32(app, box + 0x118));
    const Float t58 = Float(second);
    const Float t59 = fp::mul(t51, t58);
    const Float t60 = fp::mul(t55, t58);
    const Float t61 = fp::add(t48, t59);
    const Float t62 = fp::mul(t57, t58);
    const Float t63 = f32(app, box + 0xd8);
    const Float t64 = fp::mul(t63, f32(app, box + 0x11c));
    const Float t65 = fp::add(t52, t60);
    const Float t66 = fp::add(t53, t62);
    const Float t67 = f32(app, box + 0xdc);
    const Float t68 = fp::mul(t67, f32(app, box + 0x11c));
    const Float t69 = f32(app, box + 0xe0);
    const Float t70 = fp::mul(t69, f32(app, box + 0x11c));
    const Float t71 = Float(third);
    const Float t72 = fp::mul(t64, t71);
    const Float t73 = fp::mul(t68, t71);
    const Float t74 = fp::mul(t71, t70);
    const Float t75 = fp::add(t61, t72);
    const Float t76 = fp::add(t65, t73);
    const Float t77 = fp::add(t74, t66);
    putFloat(app, kContactOther + 4, float(t76));
    putFloat(app, kContactOther + 8, float(t77));
    putFloat(app, kContactOther, float(t75));
    return true;
}

/* cornerInBack2 of eax, edx, the signs ebx, ecx and [esp+4]: eax 1 inside, else 0. */
void cornerInBoxBack2(App* app, CPU& cpu)
{
    cpu.eax = cornerInBack2(app, cpu.eax, cpu.edx, i32(cpu.ebx), i32(cpu.ecx), i32(at(app, cpu.esp + 4))) ? 1 : 0;
    cpu.esp += 8;
}

/* 0x42e720: the car eax's spin (+0xe4, +0xe8, +0xec) held within plus and
 * minus 1.8 ([0x5537b0]) about each axis, the next component's loads
 * interleaved with the last's store as the code has them. */
constexpr reg32 kCarSpin = 0xe4;

/* The spin's component at `field` held to minus and plus the most: the
 * least (minus the most, stored as a float) when under it or a NaN, the
 * most when over it, else itself -- the x87's compares' answers, a NaN most
 * keeping the component. */
reg32 heldComponent(App* app, reg32 field)
{
    const float least = float(-f32(app, kMostSpin));
    const Float value = f32(app, field);
    if (!(value >= Float(least)))
        return floatBits(least);
    return value > f32(app, kMostSpin) ? at(app, kMostSpin) : at(app, field);
}

/* Each component held; the last's bits. */
reg32 holdSpin(App* app, reg32 car)
{
    reg32 held = 0;
    for (reg32 k = 0; k < 3; ++k)
    {
        held = heldComponent(app, car + kCarSpin + k * 4);
        put(app, car + kCarSpin + k * 4, held);
    }
    return held;
}

/* holdSpin of eax; eax the last component as held, as the code leaves it. */
void clampSpin(App* app, CPU& cpu)
{
    cpu.eax = holdSpin(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x42e820: an impulse on the car eax at a point, as the 7 words on the
 * stack have it (ret 0x1c): the direction at [esp+4] (normalized, 0x4e06a0;
 * scaled by 0.75 when longer than five times the car's +0xb0), the point at
 * [esp+0x10] from the car's middle, the impulse's strength at [esp+0x1c].
 * Its push along the direction (against the point's speed (0x4972f0), the
 * car's +0xb4) added to the car's velocity (+0xa4) when it pushes; its turn
 * about the direction across the point (0x4e0160) added to the spin (+0xe4)
 * -- by the car's +0x120 -- when the point is off the middle; the spin held
 * (clampSpin).  What it pushed and turned by, summed, onto the x87's
 * stack. */
constexpr reg32 kCarVelocity = 0xa4, kCarSize = 0xb0, kCarPush = 0xb4, kCarTurning = 0x120;

/* What the impulse pushed and turned by, summed, and the spin's last
 * component as held. */
struct Impulse
{
    Float sum;
    reg32 lastSpin;
};

/* The impulse of the 7 words at `words` on the car. */
std::optional<Impulse> impulse(App* app, CPU& cpu, reg32 car, reg32 words)
{
    const GuestFrame frame(cpu, 0x18, 0x40);
    const reg32 direction = words, point = words + 0xc, strength = words + 0x18;
    const reg32 lever = frame - 0x28, along = frame - 0x1c, axis = frame - 0x34;
    const std::optional<Float> length = normalize(app, cpu, direction);
    if (!length)
        return std::nullopt;
    // fcompp: five times the size under the length (or unordered): scaled.
    if (!(fp::mul(f32(app, car + kCarSize), f32(app, kFiveTimes)) >= *length)
        && !scaleVectors(app, cpu, direction, at(app, kImpulseScale), direction))
        return std::nullopt;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, lever + k, at(app, point + k));
    const std::optional<Float> reach = vectorLength(app, cpu, lever);
    if (!reach)
        return std::nullopt;
    const float leverLength = float(*reach);
    const std::optional<Float> across = dot(app, cpu, direction, lever);
    if (!across)
        return std::nullopt;
    float pushed = float(fp::mul(fp::mul(-*across, f32(app, strength)), f32(app, car + kCarPush)));
    // fcomp of 0 with it: over 0 (or a NaN), the push added to the velocity.
    if (!(0.0 >= Float(pushed)))
    {
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, along + k, at(app, direction + k));
        if (!vectorLength(app, cpu, along))
            return std::nullopt;
        const float first = float(fp::mul(f32(app, along), Float(pushed)));
        const float second = float(fp::mul(f32(app, along + 4), Float(pushed)));
        const float third = float(fp::mul(Float(pushed), f32(app, along + 8)));
        const Float x = fp::add(f32(app, car + kCarVelocity), Float(first));
        const Float y = fp::add(f32(app, car + kCarVelocity + 4), Float(second));
        const Float z = fp::add(f32(app, car + kCarVelocity + 8), Float(third));
        putFloat(app, car + kCarVelocity, float(x));
        putFloat(app, car + kCarVelocity + 4, float(y));
        putFloat(app, car + kCarVelocity + 8, float(z));
    }
    else
        pushed = 0.0f;
    float turned = 0.0f;
    // The lever's length as bits: anything but a zero (a NaN too) turns.
    if ((floatBits(leverLength) & 0x7fffffff) != 0)
    {
        if (!cross(app, cpu, lever, direction, axis))
            return std::nullopt;
        const std::optional<Float> size = normalize(app, cpu, axis);
        if (!size)
            return std::nullopt;
        const Float inverse = fp::div(Float(1.0), f32(app, car + kCarTurning));
        const Float sized = fp::mul(*size, f32(app, car + kCarPush));
        const Float strong = fp::mul(inverse, f32(app, strength));
        turned = float(fp::mul(sized, f32(app, strength)));
        const float factor = float(fp::mul(fp::mul(strong, f64(app, kOneTurn)), f32(app, car + kCarPush)));
        if (!scaleVectors(app, cpu, axis, floatBits(factor), axis))
            return std::nullopt;
        for (reg32 k = 0; k < 0xc; k += 4)
            putFloat(app, car + kCarSpin + k, float(fp::add(f32(app, car + kCarSpin + k), f32(app, axis + k))));
    }
    const reg32 held = holdSpin(app, car);
    return Impulse{fp::add(Float(pushed), Float(turned)), held};
}

/* impulse of the words above the return address on the car eax, onto the
 * x87's stack (ret 0x1c); every register but eax as the caller had it, eax
 * the spin's last component as held. */
void applyImpulse(App* app, CPU& cpu)
{
    const Kept kept(cpu, kAllButEax);
    const std::optional<Impulse> done = impulse(app, cpu, cpu.eax, cpu.esp + 4);
    if (!done)
        return;
    cpu.eax = done->lastSpin;
    cpu.fpu.push(done->sum);
    cpu.esp += 4 + 0x1c;
}

/* 0x42e9c0: the car eax bounced off a wall of normal edx with the strength
 * [esp+4] (ret 4): its velocity (+0xa4), spin (+0xe4) and the normal taken
 * into the car's frame (its axes +0xc0 transposed); the velocity's part along
 * the normal taken off, the rest damped -- each component of 22 or more by
 * 0.958, the last by 0.975 when the car's up axis (+0xcc) is more than 0.9
 * along the normal -- and the part along the normal put back; both back into
 * the world. */
constexpr reg32 kCarUp = 0xcc;

/* The vector at `vector` read as the code's product reads it, then the
 * product (transformPoint) into `out`. */
void transformAt(App* app, reg32 matrix, reg32 vector, reg32 out)
{
    const float read[3] = {f32At(app, vector), f32At(app, vector + 4), f32At(app, vector + 8)};
    transformPoint(app, matrix, read, out);
}

/* The part of the vector at `vector` along the unit vector at `along` (`dot`
 * times it) stored at `part` and taken off the vector, in the code's
 * order. */
void takePart(App* app, reg32 along, Float dot, reg32 vector, reg32 part)
{
    const Float scaled[3] = {fp::mul(f32(app, along), dot), fp::mul(f32(app, along + 4), dot),
                             fp::mul(dot, f32(app, along + 8))};
    const Float kept[3] = {f32(app, vector), f32(app, vector + 4), f32(app, vector + 8)};
    for (reg32 k = 0; k < 3; ++k)
        putFloat(app, part + k * 4, float(scaled[k]));
    Float rest[3];
    for (reg32 k = 0; k < 3; ++k)
        rest[k] = fp::sub(kept[k], f32(app, part + k * 4));
    for (reg32 k = 0; k < 3; ++k)
        putFloat(app, vector + k * 4, float(rest[k]));
}

/* The part at `part` added back to the vector at `vector`. */
void addPart(App* app, reg32 vector, reg32 part)
{
    const Float kept[3] = {f32(app, vector), f32(app, vector + 4), f32(app, vector + 8)};
    Float sum[3];
    for (reg32 k = 0; k < 3; ++k)
        sum[k] = fp::add(kept[k], f32(app, part + k * 4));
    for (reg32 k = 0; k < 3; ++k)
        putFloat(app, vector + k * 4, float(sum[k]));
}

/* The component at `component` damped by `damping` when its size (negated
 * when under 0, a NaN kept, as the code's compare has it) is over the speed
 * a wall damps. */
void damp(App* app, reg32 component, const Float& damping)
{
    const float value = f32At(app, component);
    const float size = 0.0 > Float(value) ? float(-Float(value)) : value;
    if (Float(size) > f32(app, kFastAlong))
        putFloat(app, component, float(fp::mul(f32(app, component), damping)));
}

bool bounce(App* app, CPU& cpu, reg32 car, reg32 normal)
{
    const GuestFrame frame(cpu, 0x10, 0x78);
    const reg32 axes = frame - 0x78, across = frame - 0x54, velocity = frame - 0x48, spin = frame - 0x3c;
    const reg32 part = frame - 0x30;
    if (!transpose(app, cpu, car + kCarAxes, axes))
        return false;
    transformAt(app, axes, car + kCarVelocity, velocity);
    transformAt(app, axes, car + kCarSpin, spin);
    transformAt(app, axes, normal, across);
    const std::optional<Float> into = dot(app, cpu, velocity, across);
    if (!into)
        return false;
    takePart(app, across, *into, velocity, part);
    const std::optional<Float> up = dot(app, cpu, car + kCarUp, normal);
    if (!up)
        return false;
    // fcomp: the up axis over 0.9 along the normal (a NaN not).
    const bool upright = *up > f64(app, kNinetenths);
    damp(app, velocity, f32(app, kWallDamping));
    damp(app, velocity + 4, f32(app, kWallDamping));
    damp(app, velocity + 8, upright ? f64(app, kSlowZ) : f32(app, kWallDamping));
    addPart(app, velocity, part);
    const std::optional<Float> turning = dot(app, cpu, spin, across);
    if (!turning)
        return false;
    takePart(app, across, *turning, spin, part);
    if (!scaleVectors(app, cpu, part, at(app, kWallDamping), part))
        return false;
    addPart(app, spin, part);
    transformAt(app, car + kCarAxes, velocity, car + kCarVelocity);
    transformAt(app, car + kCarAxes, spin, car + kCarSpin);
    return true;
}

/* bounce of the car off the normal as the code's call leaves the registers:
 * ebx, ecx and esi as they were, eax past the spin's copy in its frame (as
 * its last product leaves it), edx the car's axes. */
bool bounceAsCalled(App* app, CPU& cpu, reg32 car, reg32 normal)
{
    const reg32 frameEnd = cpu.esp - 0x40;
    const Kept kept(cpu, kEbx | kEcx | kEsi);
    if (!bounce(app, cpu, car, normal))
        return false;
    cpu.eax = frameEnd;
    cpu.edx = car + kCarAxes;
    return true;
}

/* bounceAsCalled of the car eax off the normal edx; [esp+4], its strength,
 * the code does not read (ret 4). */
void bounceOffWall(App* app, CPU& cpu)
{
    if (!bounceAsCalled(app, cpu, cpu.eax, cpu.edx))
        return;
    cpu.esp += 8;
}

/* 0x42ee10: the car eax hitting a wall at the point edx, its normal ebx,
 * the wall's point ecx: the car pushed out along the normal if the point is
 * behind the wall; the point's speed (the car's velocity +0xa4 and its spin
 * +0xe4, a turn's 2 pi, across the point from the car's place) along the
 * normal; if it goes into the wall, an impulse against it (applyImpulse)
 * -- kept at +0x168 -- and, if there was one, the hit noted (+0x16c 0,
 * +0x170 +0x18c's with 0x30000, the point at +0x178) and the car bounced
 * (bounceOffWall).  1, or 0 when it was moving away. */
constexpr reg32 kCarPlace = 0x98, kCarImpact = 0x168, kCarHitBy = 0x16c, kCarHitKind = 0x170, kCarHitAt = 0x178;
constexpr reg32 kCarHitFlags = 0x18c, kTurnBits = 0x40c90fd9, kOneBits = 0x3f800000;  // 2 pi, 1

/* 0x4e0000: the vector at `b` taken from the one at `a`, into `out` (1, the
 * vectors' count). */
bool subtract(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, a, b, out);
    return !cpu.terminate;
}

/* The point's speed in the car: its velocity, and its spin (a turn's 2 pi)
 * across the point from its place (`lever`, the point less the place) --
 * `lever`, `spin` and `speed` the frame's.  The registers as the adding
 * leaves them, which hitWall and carsCollide leave. */
bool spinSpeed(App* app, CPU& cpu, reg32 car, reg32 lever, reg32 spin, reg32 speed)
{
    return scaleVectors(app, cpu, car + kCarSpin, kTurnBits, spin) && cross(app, cpu, spin, lever, speed)
           && addVectorsLeaving(app, cpu, speed, car + kCarVelocity, speed);
}

bool pointSpeed(App* app, CPU& cpu, reg32 car, reg32 point, reg32 lever, reg32 spin, reg32 speed)
{
    return subtract(app, cpu, point, car + kCarPlace, lever) && spinSpeed(app, cpu, car, lever, spin, speed);
}

std::optional<bool> hit(App* app, CPU& cpu, reg32 car, reg32 point, reg32 normal, reg32 wall)
{
    const GuestFrame frame(cpu, 0xc, 0x4c);
    const reg32 into = frame - 0x40, lever = frame - 0x28, spin = frame - 0x1c, speed = frame - 0x4c;
    const reg32 push = frame - 0x34;
    for (reg32 k = 0; k < 0xc; k += 4)
        putFloat(app, into + k, float(fp::sub(f32(app, point + k), f32(app, wall + k))));
    const std::optional<Float> depthDot = dot(app, cpu, into, normal);
    if (!depthDot)
        return std::nullopt;
    const float depth = float(*depthDot);
    // fcomp of 0 with it: behind the wall (under 0, a NaN not), pushed out.
    if (0.0 > Float(depth))
    {
        if (!scaleVectors(app, cpu, normal, floatBits(float(-Float(depth))), into))
            return std::nullopt;
        for (reg32 k = 0; k < 0xc; k += 4)
            putFloat(app, car + kCarPlace + k, float(fp::add(f32(app, car + kCarPlace + k), f32(app, into + k))));
    }
    if (!pointSpeed(app, cpu, car, point, lever, spin, speed))
        return std::nullopt;
    for (reg32 k = 0; k < 0xc; k += 4)
        putFloat(app, push + k, float(fp::mul(f32(app, speed + k), f32(app, car + kCarSize))));
    const std::optional<Float> alongDot = dot(app, cpu, push, normal);
    if (!alongDot)
        return std::nullopt;
    const float along = float(*alongDot);
    // fcomp of 0 with it: moving away (not under 0, or a NaN), no impulse.
    if (!(0.0 > Float(along)))
        return false;
    if (!scaleVectors(app, cpu, normal, floatBits(float(-Float(along))), push))
        return std::nullopt;
    // The impulse's 7 words as the code pushes them -- the push, the lever,
    // 1 -- through ebx, ecx and edx, which it leaves holding the lever's
    // first two and the push's first.
    std::optional<Impulse> done;
    {
        const GuestFrame words(cpu, 0, 0x1c);  // popped by the impulse's ret 0x1c
        const reg32 first = words - 0x1c;
        put(app, first + 0x18, kOneBits);
        for (reg32 k = 3; k-- > 0;)
            put(app, first + 0xc + k * 4, at(app, lever + k * 4));
        for (reg32 k = 3; k-- > 0;)
            put(app, first + k * 4, at(app, push + k * 4));
        cpu.ebx = at(app, lever);
        cpu.ecx = at(app, lever + 4);
        cpu.edx = at(app, push);
        const Nested nested(cpu, kAllButEax);
        done = impulse(app, cpu, car, first);
    }
    if (!done)
        return std::nullopt;
    putFloat(app, car + kCarImpact, float(done->sum));
    // fcompp of 0 with it: a hit unless it is 0 (or a NaN).
    if (done->sum < 0.0 || done->sum > 0.0)
    {
        put(app, car + kCarHitBy, 0);
        const reg32 kind = at(app, car + kCarHitFlags) | 0x30000;
        put(app, car + kCarHitKind, kind);
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, car + kCarHitAt + k, at(app, point + k));
        cpu.ecx = kind;  // as the code leaves it
        const GuestFrame strength(cpu, 0, 4);
        put(app, strength - 4, at(app, car + kCarImpact));
        const Nested nested(cpu, 0);
        if (!bounceAsCalled(app, cpu, car, normal))
            return std::nullopt;
    }
    return true;
}

/* hit of the car eax at the point edx, the normal ebx, the wall's point ecx:
 * eax 1, or 0 when it moved away; esi and edi as the caller had them, ebx,
 * ecx and edx as the code leaves them. */
void hitWall(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEsi | kEdi);
    const std::optional<bool> hitIt = hit(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
    if (!hitIt)
        return;
    cpu.eax = *hitIt ? 1 : 0;
    cpu.esp += 4;
}

/* 0x42efe0: the car eax's box against a wall of normal edx through the
 * point ebx, within the distance [esp+4] (ret 4): its 8 corners -- 4 of its
 * bottom when it is on the ground (+0x164 0) -- taken into the world
 * (0x4e0430, by its axes +0xc0 and place +0x98), the one furthest into the
 * wall found; if it is nearer than the distance, the car hits the wall there
 * (hitWall).  The spin held (clampSpin).  What hitWall gave, or 0. */
// +0x164: the car's hard hits (carsCollide counts them); while it has any, its box is tried
// at 8 corners, else at its bottom's 4.
constexpr reg32 kCarHardHits = 0x164, kCarHalfSizes = 0x114;

/* The box's corners the code tries, into `corners` (3 floats each): 8 --
 * the box's four and four more within it -- off the ground, the bottom's
 * four on it; their count. */
reg32 boxCorners(App* app, reg32 car, reg32 corners)
{
    const auto size = [&](reg32 k) { return f32(app, car + kCarHalfSizes + k * 4); };
    const auto bits = [&](reg32 k) { return at(app, car + kCarHalfSizes + k * 4); };
    const auto neg = [&](reg32 k) { return floatBits(float(-size(k))); };
    const auto set = [&](reg32 corner, reg32 x, reg32 y, reg32 z) {
        put(app, corners + corner * 0xc, x);
        put(app, corners + corner * 0xc + 4, y);
        put(app, corners + corner * 0xc + 8, z);
    };
    if (half(app, car + kCarHardHits) == 0)
    {
        set(0, neg(0), 0, bits(2));
        set(1, bits(0), 0, bits(2));
        set(2, bits(0), 0, neg(2));
        set(3, neg(0), 0, neg(2));
        return 4;
    }
    set(0, neg(0), neg(1), bits(2));
    set(1, bits(0), neg(1), bits(2));
    set(2, bits(0), neg(1), neg(2));
    set(3, neg(0), neg(1), neg(2));
    const Float threeQuarters = f64(app, kThreeQuarters), oneHalf = f64(app, kHalf), fourFifths = f64(app, kFourFifths);
    const reg32 x4 = floatBits(float(fp::mul(-size(0), threeQuarters)));
    const reg32 y4 = floatBits(float(fp::mul(size(1), oneHalf)));
    const reg32 z4 = floatBits(float(fp::mul(size(2), fourFifths)));
    set(4, x4, y4, z4);
    const reg32 x5 = floatBits(float(fp::mul(threeQuarters, size(0))));
    const reg32 y5 = floatBits(float(fp::mul(oneHalf, size(1))));
    const reg32 z5 = floatBits(float(fp::mul(size(2), fourFifths)));
    set(5, x5, y5, z5);
    set(6, bits(0), bits(1), floatBits(float(fp::mul(-size(2), fourFifths))));
    set(7, neg(0), bits(1), floatBits(float(fp::mul(fourFifths, -size(2)))));
    return 8;
}

std::optional<reg32> boxHits(App* app, CPU& cpu, reg32 car, reg32 normal, reg32 point, reg32 reach)
{
    const GuestFrame frame(cpu, 0x10, 0x94);
    const reg32 corners = frame - 0x94, nearest = frame - 0x34, apart = frame - 0x28;
    const reg32 count = boxCorners(app, car, corners);
    guest::direct<0x4e0430, vertices::transformPointList>(app, cpu, i32(count), corners, car + kCarAxes,
                                                          car + kCarPlace, corners);
    if (cpu.terminate)
        return std::nullopt;
    float best = f32At(app, reach);
    for (reg32 i = 0; i < count; ++i)
    {
        const reg32 corner = corners + i * 0xc;
        for (reg32 k = 0; k < 0xc; k += 4)
            putFloat(app, apart + k, float(fp::sub(f32(app, corner + k), f32(app, point + k))));
        const std::optional<Float> distance = dot(app, cpu, apart, normal);
        if (!distance)
            return std::nullopt;
        // fcomp with the best: under it (or a NaN), the nearest so far.
        if (!(*distance >= Float(best)))
        {
            best = float(*distance);
            for (reg32 k = 0; k < 0xc; k += 4)
                put(app, nearest + k, at(app, corner + k));
        }
    }
    cpu.ebx = (count - 1) * 0xc;  // the last corner's place, as the loop leaves it
    reg32 result = 0;
    // fcomp with the reach: under it (or a NaN), the car hits the wall there.
    if (!(Float(best) >= f32(app, reach)))
    {
        const Nested nested(cpu, kEsi | kEdi);
        const std::optional<bool> hitIt = hit(app, cpu, car, nearest, normal, point);
        if (!hitIt)
            return std::nullopt;
        result = *hitIt ? 1 : 0;
    }
    holdSpin(app, car);
    return result;
}

/* boxHits of the car eax against the normal edx through the point ebx
 * within [esp+4] (ret 4); ecx, esi and edi as the caller had them, ebx and
 * edx as the code leaves them. */
void boxHitsWall(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEsi | kEdi);
    const std::optional<reg32> result = boxHits(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.esp + 4);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 8;
}

/* 0x42f280: the cars eax and edx in collision at the point ebx: each one's
 * point's speed (its velocity, and its spin -- a turn's 2 pi -- across the
 * point from its place); the two cars' masses (+0xb0) and the speeds against
 * each other; where they close, an impulse on each against the other
 * (applyImpulse, kept at +0x168) and, if there was one, the hit noted
 * (+0x170 0x50005, +0x16c the other car, the point at +0x178) and, if it
 * was more than 5, +0x164 counted up. */
constexpr reg32 kHitByCar = 0x50005;

/* The impulse of the 7 words at `words` on `car` -- kept at +0x168 -- and,
 * if there was one, its hit by `other` at `point` noted and, past 5, the
 * car's hard hits (+0x164) counted.  eax and edx as the code leaves them:
 * the car, its compares' status words in ax, the other car once noted. */
bool impulseFrom(App* app, CPU& cpu, reg32 car, reg32 other, reg32 point, reg32 words)
{
    std::optional<Impulse> done;
    {
        const Nested nested(cpu, kAllButEax);
        done = impulse(app, cpu, car, words);
    }
    if (!done)
        return false;
    putFloat(app, car + kCarImpact, float(done->sum));
    cpu.eax = car;
    // fcompp of 0 with it: none (or a NaN), no hit.
    if (compare(cpu, 0.0, done->sum).equal)
        return true;
    cpu.eax = car;
    cpu.edx = other;
    put(app, car + kCarHitKind, kHitByCar);
    put(app, car + kCarHitBy, other);
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, car + kCarHitAt + k, at(app, point + k));
    // fcomp with 5: over it (a NaN not), a hard hit.
    const Status s = compare(cpu, f32(app, car + kCarImpact), f64(app, kFive));
    if (!s.below && !s.equal)
    {
        cpu.eax = car;
        putHalf(app, car + kCarHardHits, reg16(half(app, car + kCarHardHits) + 1));
    }
    return true;
}

/* The impulse's 7 words as the code pushes them -- the direction at
 * `direction`, the lever at `lever`, the strength's bits -- under esp, and
 * ebx, ecx and edx as the pushes leave them (the lever's). */
void pushImpulse(App* app, CPU& cpu, reg32 first, reg32 direction, reg32 lever, reg32 strengthBits)
{
    put(app, first + 0x18, strengthBits);
    for (reg32 k = 3; k-- > 0;)
        put(app, first + 0xc + k * 4, at(app, lever + k * 4));
    for (reg32 k = 3; k-- > 0;)
        put(app, first + k * 4, at(app, direction + k * 4));
    cpu.edx = at(app, lever + 8);
    cpu.ecx = at(app, lever + 4);
    cpu.ebx = at(app, lever);
}

bool respond(App* app, CPU& cpu, reg32 a, reg32 b, reg32 point)
{
    const GuestFrame frame(cpu, 0x10, 0x7c);
    const reg32 leverA = frame - 0x7c, leverB = frame - 0x70, speedA = frame - 0x64, speedB = frame - 0x34;
    const reg32 pushA = frame - 0x28, pushB = frame - 0x4c, apart = frame - 0x40, share = frame - 0x10;
    if (!subtract(app, cpu, point, a + kCarPlace, leverA) || !subtract(app, cpu, point, b + kCarPlace, leverB)
        || !spinSpeed(app, cpu, a, leverA, frame - 0x1c, speedA) || !spinSpeed(app, cpu, b, leverB, frame - 0x58, speedB))
        return false;
    // The speeds against each other weighed by the masses, the masses'
    // share, the line between the cars: in the code's order.
    const Float t1 = f32(app, speedA);
    const Float t2 = f32(app, a + kCarSize);
    const Float t3 = fp::sub(t1, f32(app, speedB));
    const Float t4 = fp::add(t2, f32(app, b + kCarSize));
    const Float t5 = fp::mul(t3, f32(app, a + kCarSize));
    const Float t6 = f32(app, speedA + 4);
    const Float t7 = fp::sub(t6, f32(app, speedB + 4));
    putFloat(app, pushB, float(t5));
    const Float t9 = fp::mul(t7, f32(app, a + kCarSize));
    const Float t10 = fp::div(Float(1.0), t4);
    const Float t11 = f32(app, speedA + 8);
    const Float t12 = fp::sub(t11, f32(app, speedB + 8));
    putFloat(app, pushB + 4, float(t9));
    const Float t13 = fp::mul(t12, f32(app, a + kCarSize));
    const Float t14 = f32(app, speedB);
    const Float t15 = fp::sub(t14, f32(app, speedA));
    putFloat(app, pushB + 8, float(t13));
    const Float t16 = fp::mul(t15, f32(app, b + kCarSize));
    const Float t17 = f32(app, speedB + 4);
    const Float t18 = fp::sub(t17, f32(app, speedA + 4));
    putFloat(app, pushA, float(t16));
    const Float t19 = fp::mul(t18, f32(app, b + kCarSize));
    const Float t20 = f32(app, speedB + 8);
    const Float t21 = fp::sub(t20, f32(app, speedA + 8));
    putFloat(app, pushA + 4, float(t19));
    const Float t22 = fp::mul(t21, f32(app, b + kCarSize));
    putFloat(app, pushA + 8, float(t22));
    const Float t23 = f32(app, b + kCarPlace);
    const Float t24 = fp::sub(t23, f32(app, a + kCarPlace));
    putFloat(app, apart, float(t24));
    const Float t25 = f32(app, b + kCarPlace + 4);
    const Float t26 = fp::sub(t25, f32(app, a + kCarPlace + 4));
    putFloat(app, apart + 4, float(t26));
    const Float t27 = f32(app, b + kCarPlace + 8);
    putFloat(app, share, float(t10));
    const Float t28 = fp::sub(t27, f32(app, a + kCarPlace + 8));
    putFloat(app, apart + 8, float(t28));
    const std::optional<Float> closing = dot(app, cpu, apart, pushA);
    if (!closing)
        return false;
    // fcompp of 0 with it, its status word in ax as the code leaves it:
    // under 0 (a NaN not), an impulse on the first car.
    const Status s = compare(cpu, 0.0, *closing);
    if (!s.below && !s.equal)
    {
        const GuestFrame words(cpu, 0, 0x1c);
        pushImpulse(app, cpu, words - 0x1c, pushA, leverA,
                    floatBits(float(fp::mul(f32(app, share), f32(app, a + kCarSize)))));
        if (!impulseFrom(app, cpu, a, b, point, words - 0x1c))
            return false;
    }
    // By address: what it leaves in eax is what the collision's code leaves.
    const std::optional<Float> opening = dotProduct(app, cpu, apart, pushB);
    if (!opening)
        return false;
    // fcompp of 0 with it, as above: over 0 (or a NaN), one on the second.
    if (compare(cpu, 0.0, *opening).below)
    {
        const GuestFrame words(cpu, 0, 0x1c);
        pushImpulse(app, cpu, words - 0x1c, pushB, leverB,
                    floatBits(float(fp::mul(f32(app, share), f32(app, b + kCarSize)))));
        if (!impulseFrom(app, cpu, b, a, point, words - 0x1c))
            return false;
    }
    return true;
}

/* respond of the cars eax and edx at the point ebx; ecx, esi and edi as
 * the caller had them, eax, ebx and edx as the code leaves them. */
void carsCollide(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEsi | kEdi);
    if (!respond(app, cpu, cpu.eax, cpu.edx, cpu.ebx))
        return;
    cpu.esp += 4;
}

}  // namespace raceload
}  // namespace nfs3hp
