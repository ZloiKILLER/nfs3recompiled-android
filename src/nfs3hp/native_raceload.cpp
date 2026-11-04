#include "native_guest.h"

#include <utility>

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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them; the x87's stack pushed and popped as
 * the code has it, every operation through cpu.fpu.  Every other function of
 * the game's, and Windows' through its imports, is called by address. */
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
constexpr reg32 kCarViv = 0x553070, kRaceCars = 0x6fd510, kCars = 0x5e1120, kCarBytes = 0x9ac;
constexpr reg32 kPerformances = 0x5eabe0, kPerformanceBytes = 0x4ac, kChoices = 0x6fd52c;
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
constexpr reg32 kRaceFlags = 0x6fd4fc, kRaceTrack = 0x6fd500, kRaceSplit = 0x6fd50c, kRacePlayers = 0x6fd3b0;
constexpr reg32 kPlayerCar = 0x6fd4f4, kHumanCars = 0x6fd514, kBackwards = 0x6fd4c0;
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
// What is called by address.
constexpr reg32 kFindInViv = 0x4ea290, kAllocate = 0x4e1620, kFree = 0x4e1890, kMoveMemory = 0x4ea4f0;
constexpr reg32 kReadPerformance = 0x4b1380, kCarFolder = 0x439300, kCarName = 0x4392b0, kCarModel = 0x439710;
constexpr reg32 kSprintf = 0x4df690, kLoadingTick = 0x4a33d0, kCarDrawn = 0x4a78d6, kCarShapes = 0x4b7d30;
constexpr reg32 kPlaceCar = 0x42bcb0, kCarReset = 0x40cb70, kCarPrepared = 0x4285f0, kCarsFreed = 0x42bb80;
constexpr reg32 kFillZero = 0x4e070c, kLoadingBar = 0x495230, kCarMade = 0x428700, kLoadFile = 0x4e0ec0;
constexpr reg32 kMemset = 0x4e0640, kFreeShapes = 0x4b7eb0, kAddTask = 0x4be250, kRemoveTask = 0x4be2f0;
constexpr reg32 kIsForceCar = 0x471dc0, kForceTask = 0x471e10;
// The race's frame: the track's nodes and their records (0x24 bytes each),
// the race's frame counter, the camera's car and the race's light.
constexpr reg32 kTrackNodes = 0x5dd940, kTrackRecords = 0x5dd948, kFrameCounter = 0x7d3684;
constexpr reg32 kCameraCar = 0x6fbc3c, kLight = 0x7a3a60;
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
// What else is called by address: the vectors' and matrices' (a dot
// product, a vector scaled and added, scaled, less another, crossed,
// normalized, its length; a matrix transposed, turned; points transformed),
// the car's speed and gearing, atan2, Watcom's __CHP; the messages' box and
// lines, the text's fit, strncpy, a packet sent; the locks, events, timers;
// the race's controls read.
constexpr reg32 kDotProduct = 0x4e01f0, kScaledSum = 0x4dffb0, kScaleVectors = 0x4e0050, kSubtract = 0x4e0000;
constexpr reg32 kCross = 0x4e0160, kNormalize = 0x4e06a0, kVectorLength = 0x4972f0, kTranspose = 0x4e0110;
constexpr reg32 kTurnMatrix = 0x4eaab0, kTurnAxes = 0x4ea9e0, kTransformPoints = 0x4e0430;
constexpr reg32 kCarSpeed = 0x4a2bc0, kCarGearing = 0x40e2b0, kAtan2 = 0x4ea9c0, kChop = 0x4dfd56;
constexpr reg32 kOpenMessageBox = 0x436c40, kCloseMessageBox = 0x436cf0, kClearMessageBox = 0x436d40;
constexpr reg32 kShowLine = 0x436e10, kTextFits = 0x441900, kStrncpy = 0x4e0e30, kSendPacket = 0x4a60e0;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kMakeLock = 0x4eb2e0, kFreeLock = 0x4eb360, kCreateEvent = 0x4dfb00, kPostEvent = 0x4dfb20;
constexpr reg32 kWaitEvent = 0x4dfb70, kAddTimer = 0x48ee30, kRemoveTimer = 0x48ee90, kReadControls = 0x48f550;

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

void putF32(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
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

/* movsd `count` times: a dword from [esi] to [edi], both on. */
void moveDwords(win32::WinApplication* app, x86::CPU& cpu, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.edi += 4;
        cpu.esi += 4;
    }
}

/* rep movsd: ecx dwords from [esi] to [edi], ecx left 0. */
void repMoveDwords(win32::WinApplication* app, x86::CPU& cpu)
{
    moveDwords(app, cpu, cpu.ecx);
    cpu.ecx = 0;
}

/* sub ecx, ecx; dec ecx; xor eax, eax; repne scasb; not ecx; dec ecx: the
 * string at edi's length in ecx, eax 0, edi past its ending. */
void stringLength(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    do
    {
        --cpu.ecx;
    } while (byte(app, cpu.edi++) != cpu.al && cpu.ecx != 0);
    cpu.ecx = ~cpu.ecx;
    --cpu.ecx;
}

/* The matrix at edx times the vector at eax, into [ebx]: the code's
 * inlined product, a row's three products summed each (st(0) to [ebx]
 * first), eax left past the vector. */
void matrixTimesVector(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
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
    for (int i = 0; i < 3; ++i)
    {
        fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
        fpu.pop();
    }
    fpu.push(f32(app, cpu.eax + 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x18));
    fpu.push(f32(app, cpu.eax + 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x1c));
    fpu.push(f32(app, cpu.eax + 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x20));
    std::swap(fpu.st(0), fpu.st(2));
    for (int i = 0; i < 3; ++i)
    {
        fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
        fpu.pop();
    }
    cpu.eax += 0xc;
    putF32(app, cpu.ebx, fpu.pop());
    putF32(app, cpu.ebx + 4, fpu.pop());
    putF32(app, cpu.ebx + 8, fpu.pop());
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

/* 0x42c2a0: the car eax's performance text, the file named edx in the car's
 * .viv ([0x553070]; 0x4ea290 finds its place and size), copied out into a
 * block of its own and read (0x4b1380) into the car's performance record
 * ([car + 0x528]); the block freed.  [car + 0x9a8] what 0x439710 gives for
 * the car's index (0 to 49), else 0. */
void loadCarPerformance(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 size = cpu.ebp - 8, place = cpu.ebp - 4;
    cpu.edi = cpu.eax;
    cpu.eax = size;
    cpu.ecx = 0;
    cpu.ebx = 0;
    put(app, size, cpu.ecx);
    put(app, place, cpu.ecx);
    push(app, cpu, cpu.eax);
    cpu.ecx = place;
    cpu.eax = at(app, kCarViv);
    if (!byAddress(app, cpu, kFindInViv))
        return;
    cpu.ecx = at(app, place);
    cpu.ebx = 0x10;
    cpu.eax = at(app, kCarViv);
    cpu.edx = at(app, size);
    cpu.ecx += cpu.eax;
    cpu.eax = kNoName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = at(app, size);
    cpu.esi = cpu.eax;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kMoveMemory))
        return;
    cpu.ebx = at(app, size);
    cpu.edx = cpu.esi;
    cpu.eax = at(app, cpu.edi + 0x528);
    cpu.ecx = 0;
    if (!byAddress(app, cpu, kReadPerformance))
        return;
    if (cpu.esi != 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.eax = at(app, cpu.edi + 0x220);
    cpu.esi = at(app, cpu.eax + 0x14);
    if (sreg32(cpu.esi) >= 0 && sreg32(cpu.esi) <= 0x31)
    {
        cpu.eax = at(app, cpu.edi + 0x220);
        cpu.eax = at(app, cpu.eax + 0x14);
        if (!byAddress(app, cpu, kCarModel))
            return;
        put(app, cpu.edi + 0x9a8, cpu.eax);
    }
    else
        put(app, cpu.edi + 0x9a8, 0);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c350: the race's car ebx (if there are that many), its record eax,
 * made from its .viv: prepared (0x4285f0), its performance record edx, its
 * performance text ("<folder>p.txt", sprintf'd in the frame) read, its
 * drawing (0x4a78d6, unless +0x200 has 0x40) and shapes (0x4b7d30) made,
 * placed on the grid (0x42bcb0) and reset (0x40cb70) -- the loading bar's
 * tick (0x4a33d0) between. */
void loadRaceCar(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x110;
    cpu.ecx = cpu.eax;
    if (sreg32(cpu.ebx) < sreg32(at(app, kRaceCars)))
    {
        if (!byAddress(app, cpu, kCarPrepared))
            return;
        cpu.eax = at(app, cpu.ecx + 0x220);
        put(app, cpu.ecx + 0x528, cpu.edx);
        cpu.eax = at(app, cpu.eax + 0x14);
        if (!byAddress(app, cpu, kCarFolder))
            return;
        push(app, cpu, cpu.eax);
        push(app, cpu, kPerformanceName);
        cpu.eax = cpu.ebp - 0x14;
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0xc;
        cpu.eax = cpu.ebp - 0x14;
        push(app, cpu, cpu.eax);
        push(app, cpu, kTextName);
        cpu.eax = cpu.ebp - 0x110;
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0xc;
        cpu.edx = cpu.ebp - 0x110;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, loadCarPerformance) || !byAddress(app, cpu, kLoadingTick))
            return;
        if ((byte(app, cpu.ecx + 0x200) & 0x40) == 0)
        {
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kCarDrawn))
                return;
        }
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        cpu.edx = at(app, kCarViv);
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kCarShapes) || !byAddress(app, cpu, kLoadingTick))
            return;
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kPlaceCar) || !byAddress(app, cpu, kLoadingTick))
            return;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kCarReset))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c400: the car eax's handlers -- what moves it, steers it, draws it --
 * as its kind is: a player's, else (+0x200 bit 1) the computer's -- a
 * pursuer's (0x10), traffic (0x20) -- and in a split screen race with more
 * than one player, every car but the player's that is not driven by one of
 * the people (+0x1f0) the network's, +0x201 bit 1 set. */
void setCarHandlers(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, cpu.eax + 0x5fc, 0x4bd380);
    put(app, cpu.eax + 0x600, 0x431430);
    put(app, cpu.eax + 0x604, 0x4ce280);
    put(app, cpu.eax + 0x608, 0x4ae4c1);
    put(app, cpu.eax + 0x60c, 0x4a2830);
    put(app, cpu.eax + 0x610, 0x4a2c60);
    put(app, cpu.eax + 0x614, 0x4a2f30);
    put(app, cpu.eax + 0x618, 0x4311d0);
    put(app, cpu.eax + 0x61c, 0x42b900);
    cpu.dl = byte(app, cpu.eax + 0x200);
    put(app, cpu.eax + 0x5f8, 0x4a08c0);
    if ((cpu.dl & 2) != 0)
    {
        put(app, cpu.eax + 0x600, 0x409c00);
        cpu.dh = byte(app, cpu.eax + 0x200);
        put(app, cpu.eax + 0x5fc, 0);
        if ((cpu.dh & 0x10) != 0)
            put(app, cpu.eax + 0x600, 0x40c9f0);
        if ((byte(app, cpu.eax + 0x200) & 0x20) != 0)
            put(app, cpu.eax + 0x600, 0x4032e0);
        put(app, cpu.eax + 0x614, 0x4a2f10);
        put(app, cpu.eax + 0x608, 0x4088f0);
    }
    if (at(app, kRaceSplit) != 0 && sreg32(at(app, kRacePlayers)) > 1)
    {
        cpu.ebx = at(app, kPlayerCar);
        cpu.edx = at(app, cpu.eax + 0x1f0);
        if (cpu.edx != cpu.ebx && sreg32(cpu.edx) < sreg32(at(app, kHumanCars)))
        {
            put(app, cpu.eax + 0x600, 0x40bf50);
            put(app, cpu.eax + 0x608, 0x4088f0);
            cpu.bh = byte(app, cpu.eax + 0x201);
            put(app, cpu.eax + 0x614, 0x4a2f10);
            cpu.bh |= 2;
            putByte(app, cpu.eax + 0x201, cpu.bh);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c520: the car eax's shapes freed (0x4b7eb0). */
void freeCarShapes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, kFreeShapes))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c530: the race's cars back on the grid: the lists by place and by
 * distance the cars' list again, and each car placed (0x42bcb0), reset
 * (0x40cb70) and prepared (0x4285f0) afresh. */
void restartRaceCars(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 0;
    while (sreg32(cpu.eax) < sreg32(at(app, kCarCount)))
    {
        ++cpu.eax;
        cpu.ebx = at(app, cpu.eax * 4 + kCarList - 4);
        put(app, cpu.eax * 4 + kByPlace - 4, cpu.ebx);
        put(app, cpu.eax * 4 + kByDistance - 4, cpu.ebx);
    }
    cpu.ebx = 0;
    while (sreg32(cpu.ebx) < sreg32(at(app, kCarCount)))
    {
        cpu.ecx = at(app, cpu.ebx * 4 + kCarList);
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kPlaceCar))
            return;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kCarReset))
            return;
        cpu.eax = cpu.ecx;
        ++cpu.ebx;
        if (!byAddress(app, cpu, kCarPrepared))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c660: the cars' slots (0x258 bytes at 0x5ef6a0) cleared (memset). */
void clearCarSlots(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = kCarSlotsBytes;
    cpu.eax = kCarSlots;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c680: the loading bar's tick, and the race's counts -- the cars in
 * all among them -- 0. */
void clearRaceCounts(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kLoadingTick))
        return;
    for (const reg32 count : kRaceCounts)
        put(app, count, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c6d0: the cars' list's 16 pointers 0. */
void clearCarList(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    cpu.edx = 0;
    put(app, cpu.eax * 4 + kCarList - 4, cpu.edx);
    while (sreg32(cpu.eax) < 0x10)
    {
        ++cpu.eax;
        cpu.edx = 0;
        put(app, cpu.eax * 4 + kCarList - 4, cpu.edx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c700: the race's cars loaded: the last race's freed (0x42bb80), the
 * counts and the list cleared, the cars' and their performance records
 * zeroed (fillZero); then each car's .viv loaded, the car made (0x428700)
 * from its choice and loaded (loadRaceCar), the .viv freed -- the loading
 * bar from 75 per cent on, 15 per cent for them all -- and every car's
 * handlers set. */
void loadRaceCars(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 bar = cpu.ebp - 4;
    if (!byAddress(app, cpu, kCarsFreed) || !direct(app, cpu, clearRaceCounts) || !direct(app, cpu, clearCarList))
        return;
    cpu.edx = kBarStart;
    cpu.esi = kCars;
    put(app, bar, cpu.edx);
    cpu.eax = cpu.esi;
    cpu.edx = kCarsBytes;
    cpu.edi = kPerformances;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.edx = kPerformancesBytes;
    cpu.eax = cpu.edi;
    cpu.ecx = 0;
    if (!byAddress(app, cpu, kFillZero))
        return;
    x86::FPU& fpu = cpu.fpu;
    while (sreg32(cpu.ecx) < sreg32(at(app, kRaceCars)))
    {
        fpu.push(x86::Float(sreg32(at(app, kRaceCars))));
        fpu.st(0) = fp::div(f64(app, kBarCars), fpu.st(0));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, bar));
        putF32(app, bar, fpu.pop());
        push(app, cpu, at(app, bar));
        if (!byAddress(app, cpu, kLoadingBar) || !byAddress(app, cpu, kLoadingTick))
            return;
        // The choice's: ecx * 0x6c, as the code works it out.
        cpu.eax = cpu.ecx * 4;
        cpu.eax -= cpu.ecx;
        cpu.eax <<= 2;
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax += cpu.edx;
        cpu.edx = kChoices;
        cpu.edx += cpu.eax;
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, openCarViv))
            return;
        cpu.eax = cpu.ecx * kCarBytes;
        cpu.eax += cpu.esi;
        put(app, cpu.eax + 0x220, cpu.edx);
        cpu.edx = cpu.ecx * kPerformanceBytes;
        if (!byAddress(app, cpu, kCarMade))
            return;
        cpu.eax = at(app, cpu.ecx * 4 + kCarList);
        cpu.ebx = cpu.ecx;
        cpu.edx += cpu.edi;
        if (!direct(app, cpu, loadRaceCar) || !direct(app, cpu, closeCarViv))
            return;
        ++cpu.ecx;
    }
    cpu.edx = 0;
    while (sreg32(cpu.edx) < sreg32(at(app, kRaceCars)))
    {
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        cpu.eax = at(app, cpu.edx * 4 + kCarList);
        if (!direct(app, cpu, setCarHandlers))
            return;
        ++cpu.edx;
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

/* 0x42c7f0: the race's time step ([0x55307c]): 0.001 when the three
 * options' bytes (0x56484e and 0x56483a, then 0x564850) have a bit in
 * common, else 0.1. */
void setTimeStep(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 0;
    cpu.edx = 0;
    cpu.al = byte(app, kOption1);
    cpu.dl = byte(app, kOption2);
    cpu.edx &= cpu.eax;
    cpu.eax = 0;
    cpu.al = byte(app, kOption3);
    put(app, kTimeStep, (cpu.edx & cpu.eax) != 0 ? 0x3a83126f : 0x3dcccccd);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c830: every car's handlers added to the race's task lists
 * (0x4be250: the list eax, the handler edx, the car ebx, its priority ecx):
 * +0x5f8 at 6, +0x600 at 0x15, +0x604 at 0x19 to the drawing's list (if
 * +0x200 has 8 or 4), +0x60c and +0x608 at 0x1e, +0x618 at 0x28, +0x61c at
 * 0x32, +0x610 at 0x1e to the drawing's, +0x614 at 0x1e to the drawing's
 * (+0x200 has 4) or the other. */
void addCarTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = 0;
    const auto add = [&](reg32 priority, reg32 list, reg32 handler) {
        cpu.ecx = priority;
        cpu.eax = at(app, list);
        cpu.ebx = cpu.esi;
        cpu.edx = at(app, cpu.esi + handler);
        return byAddress(app, cpu, kAddTask);
    };
    while (sreg32(cpu.edi) < sreg32(at(app, kRaceCars)))
    {
        cpu.ecx = 6;
        cpu.esi = at(app, cpu.edi * 4 + kCarList);
        cpu.eax = at(app, kTasks);
        cpu.ebx = cpu.esi;
        cpu.edx = at(app, cpu.esi + 0x5f8);
        if (!byAddress(app, cpu, kAddTask) || !add(0x15, kTasks, 0x600))
            return;
        cpu.ah = byte(app, cpu.esi + 0x200);
        if (((cpu.ah & 8) != 0 || (cpu.ah & 4) != 0) && !add(0x19, kDrawTasks, 0x604))
            return;
        if (!add(0x1e, kTasks, 0x60c) || !add(0x1e, kTasks, 0x608) || !add(0x28, kTasks, 0x618)
            || !add(0x32, kTasks, 0x61c) || !add(0x1e, kDrawTasks, 0x610))
            return;
        cpu.ecx = 0x1e;
        cpu.eax = at(app, (byte(app, cpu.esi + 0x200) & 4) != 0 ? kDrawTasks : kTasks);
        cpu.ebx = cpu.esi;
        cpu.edx = at(app, cpu.esi + 0x614);
        if (!byAddress(app, cpu, kAddTask))
            return;
        ++cpu.edi;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42c950: the .viv of the car whose choice is at eax loaded whole to
 * [0x553070] (loadFile, flags 0x10), its path sprintf'd into the frame from
 * the game's data path: in a knock-out race ([0x6fd4fc] bit 1) the track's
 * traffic -- a pursuit's for tracks 0x11 to 0x14 -- or the knock-out's
 * car.viv after that; else the car's own (index 0 to 49), a pursuit's
 * traffic (500 to 599) or the traffic's. */
void openCarViv(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xfc;
    cpu.ebp -= 0x17a;
    const reg32 path = cpu.ebp + 0x7e;
    cpu.edx = cpu.eax;
    // The path's: the folder (0x439300) of index eax pushed, its name
    // (0x4392b0) pushed, the data path and the format: sprintf.
    const auto folderAndName = [&](reg32 format) {
        cpu.eax = at(app, cpu.edx + 0x14);
        if (!byAddress(app, cpu, kCarFolder))
            return false;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.edx + 0x14);
        if (!byAddress(app, cpu, kCarName))
            return false;
        push(app, cpu, cpu.eax);
        push(app, cpu, kDataPath);
        push(app, cpu, format);
        return true;
    };
    bool printed = false;
    if ((byte(app, kRaceFlags) & 2) != 0)
    {
        cpu.edx = at(app, kRaceTrack);
        if (sreg32(cpu.edx) <= 0x10)
        {
            cpu.eax = cpu.edx + 0x32;
            if (!byAddress(app, cpu, kCarFolder))
                return;
            push(app, cpu, cpu.eax);
            cpu.eax = at(app, kRaceTrack);
            cpu.eax += 0x32;
            if (!byAddress(app, cpu, kCarName))
                return;
            push(app, cpu, cpu.eax);
            push(app, cpu, kDataPath);
            push(app, cpu, kTrafficViv);
        }
        else if (sreg32(cpu.edx) <= 0x14)
        {
            cpu.eax = cpu.edx + 0x1e3;
            if (!byAddress(app, cpu, kCarFolder))
                return;
            push(app, cpu, cpu.eax);
            cpu.eax = at(app, kRaceTrack);
            cpu.eax += 0x1e3;
            if (!byAddress(app, cpu, kCarName))
                return;
            push(app, cpu, cpu.eax);
            push(app, cpu, kDataPath);
            push(app, cpu, kPursuitViv);
        }
        else
        {
            push(app, cpu, kDataPath);
            push(app, cpu, kKnockoutViv);
            cpu.eax = path;
            push(app, cpu, cpu.eax);
            if (!byAddress(app, cpu, kSprintf))
                return;
            cpu.esp += 0xc;
            printed = true;
        }
    }
    else
    {
        cpu.ebx = at(app, cpu.edx + 0x14);
        if (sreg32(cpu.ebx) >= 0 && sreg32(cpu.ebx) <= 0x31)
        {
            if (!folderAndName(kCarVivPath))
                return;
        }
        else
        {
            cpu.edi = at(app, cpu.edx + 0x14);
            if (!folderAndName(sreg32(cpu.edi) >= 0x1f4 && sreg32(cpu.edi) <= 0x257 ? kPursuitViv : kTrafficViv))
                return;
        }
    }
    if (!printed)
    {
        cpu.eax = path;
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0x14;
    }
    cpu.edx = 0x10;
    cpu.eax = path;
    if (!byAddress(app, cpu, kLoadFile))
        return;
    put(app, kCarViv, cpu.eax);
    cpu.esp = cpu.ebp + 0x17a;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42ca80: the car's .viv ([0x553070]) freed, if there is one, and 0. */
void closeCarViv(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kCarViv);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, kCarViv, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42cab0: every car's handlers taken off the race's task lists (0x4be2f0:
 * the list eax, the handler edx) -- +0x604 only if +0x200 has 1, the force
 * feedback's (0x471e10) if the car is the one it is played from -- its
 * shapes freed, its flag (0x5efd48) and its slot's first word (0x5ef6a0)
 * 0. */
void removeCarTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = 0;
    // A handler of the car at [list + esi] taken off the list at `tasks`.
    const auto remove = [&](reg32 list, reg32 tasks, reg32 handler) {
        cpu.ebx = at(app, list);
        cpu.eax = at(app, tasks);
        cpu.edx = at(app, cpu.ebx + handler);
        return byAddress(app, cpu, kRemoveTask);
    };
    while (sreg32(cpu.ecx) < sreg32(at(app, kCarCount)))
    {
        const reg32 car = cpu.ecx * 4 + kCarList;
        if (!remove(car, kTasks, 0x5f8) || !remove(car, kTasks, 0x600))
            return;
        cpu.ebx = at(app, car);
        if ((byte(app, cpu.ebx + 0x200) & 1) != 0)
        {
            cpu.eax = at(app, kDrawTasks);
            cpu.edx = at(app, cpu.ebx + 0x604);
            if (!byAddress(app, cpu, kRemoveTask))
                return;
        }
        cpu.esi = cpu.ecx * 4;
        if (!remove(cpu.esi + kCarList, kTasks, 0x608) || !remove(cpu.esi + kCarList, kTasks, 0x60c)
            || !remove(cpu.esi + kCarList, kDrawTasks, 0x610) || !remove(cpu.esi + kCarList, kTasks, 0x618)
            || !remove(cpu.esi + kCarList, kTasks, 0x61c))
            return;
        cpu.eax = at(app, cpu.esi + kCarList);
        if (!byAddress(app, cpu, kIsForceCar))
            return;
        if (cpu.eax != 0)
        {
            cpu.edx = kForceTask;
            cpu.eax = at(app, kTasks);
            cpu.ebx = at(app, cpu.esi + kCarList);
            if (!byAddress(app, cpu, kRemoveTask))
                return;
        }
        cpu.ebx = at(app, cpu.ecx * 4 + kCarList);
        cpu.eax = at(app, (byte(app, cpu.ebx + 0x200) & 4) != 0 ? kDrawTasks : kTasks);
        cpu.edx = at(app, cpu.ebx + 0x614);
        if (!byAddress(app, cpu, kRemoveTask))
            return;
        cpu.ebx = cpu.ecx * 4;
        cpu.edx = cpu.ecx;
        cpu.eax = at(app, cpu.ebx + kCarList);
        if (!direct(app, cpu, freeCarShapes))
            return;
        if (at(app, cpu.ebx + kCarFlags) != 0)
        {
            cpu.edi = 0;
            put(app, cpu.ebx + kCarFlags, cpu.edi);
        }
        cpu.eax = cpu.ecx * 4;
        if (at(app, cpu.eax + kCarSlots) != 0)
        {
            cpu.ebx = 0;
            put(app, cpu.eax + kCarSlots, cpu.ebx);
        }
        ++cpu.ecx;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42cc20: the car eax's laps counted as its node on the track (+0x1c)
 * passes the line, against the last (+4): forwards, from past node 500 to
 * 0 a lap on (+0x208 up), from 0 to 50 back past 500 a lap undone (+0x208
 * down, or +0x204 up, a lap lost, when there is none); backwards
 * ([0x6fd4c0]) the other way.  The node kept as the last. */
void countLaps(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.edx = at(app, cpu.eax + 4);
    if (cpu.edx != cpu.ecx)
    {
        const auto lapOn = [&] { put(app, cpu.eax + 0x208, at(app, cpu.eax + 0x208) + 1); };
        const auto lapBack = [&](reg32& reg, reg32& less) {
            reg = at(app, cpu.eax + 0x208);
            if (sreg32(reg) > 0)
            {
                less = reg - 1;
                put(app, cpu.eax + 0x208, less);
            }
            else
                put(app, cpu.eax + 0x204, at(app, cpu.eax + 0x204) + 1);
        };
        if (at(app, kBackwards) == 0)
        {
            if (sreg32(cpu.ecx) > 0x1f4 && cpu.edx == 0)
                lapOn();
            else
            {
                cpu.ecx = at(app, cpu.eax + 0x1c);
                if (sreg32(cpu.ecx) >= 0 && sreg32(cpu.ecx) <= 0x32 && sreg32(at(app, cpu.eax + 4)) > 0x1f4)
                    lapBack(cpu.edi, cpu.edx);
            }
        }
        else if (sreg32(cpu.ecx) >= 0 && sreg32(cpu.ecx) <= 0x32 && sreg32(cpu.edx) > 0x1f4)
            lapOn();
        else if (sreg32(at(app, cpu.eax + 4)) <= 0x32 && sreg32(at(app, cpu.eax + 0x1c)) > 0x1f4)
            lapBack(cpu.esi, cpu.edi);
        cpu.edx = at(app, cpu.eax + 0x1c);
        put(app, cpu.eax + 4, cpu.edx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42cd00: the car eax's speed along its own axes, onto the x87's stack:
 * its velocity (+0x124) along each of its axes (+0xc0, +0xcc, +0xd8; a dot
 * product, 0x4e01f0) made positive and weighed by +0x114, +0x118, +0x11c,
 * summed. */
void carAxesSpeed(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    x86::FPU& fpu = cpu.fpu;
    const reg32 sum = cpu.ebp - 4;
    // The dot product on the x87's stack to `along`, its size to `size`.
    const auto positive = [&](reg32 along, reg32 size) {
        fpu.push(x86::Float(0.0));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, along, fpu.pop());
        const Status s = compare(cpu, fpu.st(0), f32(app, along));
        fpu.pop();
        if (!s.below)
        {
            fpu.push(f32(app, along));
            fpu.st(0) = -fpu.st(0);
            putF32(app, size, fpu.pop());
        }
        else
        {
            cpu.eax = at(app, along);
            put(app, size, cpu.eax);
        }
    };
    cpu.ecx = cpu.eax;
    cpu.edx = cpu.eax + 0xc0;
    cpu.eax += 0x124;
    if (!byAddress(app, cpu, kDotProduct))
        return;
    positive(cpu.ebp - 0xc, cpu.ebp - 0x14);
    fpu.push(f32(app, cpu.ebp - 0x14));
    cpu.edx = cpu.ecx + 0xcc;
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x114));
    cpu.eax = cpu.ecx + 0x124;
    putF32(app, sum, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    positive(cpu.ebp - 0x10, cpu.ebp - 0x18);
    fpu.push(f32(app, cpu.ebp - 0x18));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x118));
    cpu.edx = cpu.ecx + 0xd8;
    fpu.st(0) = fp::add(fpu.st(0), f32(app, sum));
    cpu.eax = cpu.ecx + 0x124;
    putF32(app, sum, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    positive(cpu.ebp - 8, cpu.ebp - 0x1c);
    fpu.push(f32(app, cpu.ebp - 0x1c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0x11c));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, sum));
    putF32(app, sum, fpu.st(0));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42cdd0: the car eax's velocity (+0xa4) along its forward axis (+0x13c),
 * onto the x87's stack. */
void carForwardSpeed(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    x86::FPU& fpu = cpu.fpu;
    fpu.push(f32(app, cpu.eax + 0xa4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x13c));
    fpu.push(f32(app, cpu.eax + 0xa8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x140));
    fpu.push(f32(app, cpu.eax + 0xac));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), fpu.st(2));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x144));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(2) = fpu.st(0);
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42ce10: the car eax's distance into the race (+8): its laps (+0x204)
 * times the track's nodes ([0x5dd940]) and its node -- backwards, the nodes
 * less its node; 0 while it owes laps (+0x208). */
void carRaceDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, kTrackNodes);
    if (sreg32(at(app, cpu.eax + 0x208)) > 0)
        put(app, cpu.eax + 8, 0);
    else
    {
        cpu.ecx = at(app, cpu.eax + 0x204);
        cpu.ecx *= cpu.ebx;
        if (at(app, kBackwards) != 0)
        {
            cpu.edx = cpu.ebx;
            cpu.edx -= at(app, cpu.eax + 0x1c);
        }
        else
            cpu.edx = at(app, cpu.eax + 0x1c);
        cpu.ecx += cpu.edx;
        put(app, cpu.eax + 8, cpu.ecx);
    }
    put(app, kTrackNodes, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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
void updateRaceCars(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    x86::FPU& fpu = cpu.fpu;
    const reg32 node = cpu.ebp - 4, forward = cpu.ebp - 0x14;
    cpu.ebx = 0;
    while (sreg32(cpu.ebx) < sreg32(at(app, kCarCount)))
    {
        cpu.ecx = at(app, cpu.ebx * 4 + kCarList);
        cpu.ah = byte(app, cpu.ecx + 0x89);
        if (cpu.ah != 0)
        {
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, carAxesSpeed))
                return;
            cpu.eax = cpu.ecx;
            putF32(app, cpu.ecx + 0x21c, fpu.pop());
            if (!byAddress(app, cpu, kCarSpeed))
                return;
            putF32(app, cpu.ecx + 0x7b8, fpu.pop());
            cpu.eax = at(app, cpu.ecx + 0x1c);
            push(app, cpu, at(app, cpu.ecx + 0x7b8));
            if (!byAddress(app, cpu, kCarGearing))
                return;
            push(app, cpu, at(app, cpu.ecx + 0x1b0));
            push(app, cpu, at(app, cpu.ecx + 0x1a8));
            put(app, cpu.ecx + 0x638, cpu.eax);
            if (!byAddress(app, cpu, kAtan2))
                return;
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, kHeadingScale));
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kChop))
                return;
            put(app, cpu.ecx + 0x210, reg32(fp::toInteger<sreg32>(fpu.pop())));
            if (!direct(app, cpu, carForwardSpeed))
                return;
            fpu.push(x86::Float(0.0));
            std::swap(fpu.st(0), fpu.st(1));
            putF32(app, cpu.ecx + 0x7c0, fpu.pop());
            Status s = compare(cpu, fpu.st(0), f32(app, cpu.ecx + 0x7c0));
            fpu.pop();
            if (!s.below)
            {
                fpu.push(f32(app, cpu.ecx + 0x7c0));
                fpu.st(0) = -fpu.st(0);
                putF32(app, cpu.ebp - 8, fpu.pop());
            }
            else
            {
                cpu.eax = at(app, cpu.ecx + 0x7c0);
                put(app, cpu.ebp - 8, cpu.eax);
            }
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.edx = at(app, cpu.ecx + 0x1c);
            put(app, cpu.ecx + 0x7c4, cpu.eax);
            // The node's direction: three signed bytes of its record (0x24
            // bytes each at [0x5dd948]), each times 2^-16 after << 9.
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, kTrackRecords);
            cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0x11);
            cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
            cpu.eax <<= 9;
            put(app, node, cpu.eax);
            fpu.push(f64(app, kNodeScale));
            fpu.push(x86::Float(sreg32(at(app, node))));
            fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
            putF32(app, forward, fpu.pop());
            for (const reg32 part : {0x12u, 0x13u})
            {
                cpu.esi = at(app, cpu.ecx + 0x1c);
                cpu.eax = cpu.esi * 8;
                cpu.eax += cpu.esi;
                cpu.eax = at(app, cpu.edx + cpu.eax * 4 + part);
                cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
                cpu.eax <<= 9;
                put(app, node, cpu.eax);
                fpu.push(x86::Float(sreg32(at(app, node))));
                if (part == 0x12)
                {
                    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
                    putF32(app, forward + 4, fpu.pop());
                }
                else
                {
                    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
                    fpu.pop();
                }
            }
            cpu.edx = forward;
            cpu.eax = cpu.ecx + 0xd8;
            putF32(app, forward + 8, fpu.pop());
            if (!byAddress(app, cpu, kDotProduct))
                return;
            fpu.push(x86::Float(0.0));
            s = compare(cpu, fpu.st(0), fpu.st(1));
            fpu.pop();
            fpu.pop();
            cpu.edx = !s.below ? 1 : 0;
            cpu.eax = at(app, kBackwards);
            cpu.eax ^= cpu.edx;
            cpu.dx = half(app, cpu.ecx + 0x74);
            put(app, cpu.ecx + 0x5f4, cpu.eax);
            if (cpu.dx != 0)
            {
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, countLaps))
                    return;
                cpu.eax = cpu.ecx;
                if (!direct(app, cpu, carRaceDistance))
                    return;
            }
        }
        ++cpu.ebx;
    }
    if (!direct(app, cpu, sortRaceCars))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42cff0: the frame counter ([0x7d3684]) modulo 32, in 32nds, onto the
 * x87's stack; eax the counter / 32. */
void frameFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.eax = at(app, kFrameCounter);
    cpu.edx = cpu.eax;
    cpu.ecx = 0x20;
    cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
    const sreg32 counter = sreg32(cpu.eax);
    cpu.eax = reg32(counter / 0x20);
    cpu.edx = reg32(counter % 0x20);
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), f64(app, kThirtySecond));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d020: the car eax's headlights' beam (if it has them, +0x734): its
 * point (+0x780) the car's place (+0x98) and its forward axis (+0xcc) scaled
 * by the frame's fraction (0x4dffb0); a matrix of the car's (+0x738, its
 * axes copied, sizes 3.5, 15 and 30, its colours) turned by the fraction
 * (0x4eaab0) and transposed into +0x75c; the point less the car's in it
 * (+0x78c). */
void placeHeadlights(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    x86::FPU& fpu = cpu.fpu;
    const reg32 car = cpu.ebp - 4;
    put(app, car, cpu.eax);
    if (at(app, cpu.eax + 0x734) != 0)
    {
        if (!direct(app, cpu, frameFraction))
            return;
        cpu.ecx = at(app, car);
        cpu.ebx = at(app, car);
        cpu.edx = at(app, car);
        cpu.eax = 1;
        cpu.ecx += 0x780;
        cpu.ebx += 0xcc;
        cpu.edx += 0x98;
        putF32(app, cpu.ebp - 8, fpu.pop());
        if (!byAddress(app, cpu, kScaledSum))
            return;
        cpu.eax = at(app, car);
        cpu.ecx = 9;
        cpu.edi = cpu.eax + 0x738;
        cpu.esi = cpu.eax + 0xc0;
        repMoveDwords(app, cpu);
        put(app, cpu.eax + 0x798, 0x40600000);
        put(app, cpu.eax + 0x79c, 0x41700000);
        cpu.esi = at(app, car);
        put(app, cpu.eax + 0x7a0, 0x41f00000);
        cpu.ecx = cpu.eax + 0x738;
        put(app, cpu.eax + 0x7a4, 0xff8080ff);
        push(app, cpu, cpu.ecx);
        put(app, cpu.eax + 0x7a8, 0xffff8080);
        cpu.eax = cpu.ecx;
        push(app, cpu, at(app, cpu.ebp - 8));
        cpu.esi += 0x75c;
        if (!byAddress(app, cpu, kTurnMatrix))
            return;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kTranspose))
            return;
        cpu.eax = at(app, car);
        cpu.edx = cpu.esi;
        fpu.push(f32(app, cpu.eax + 0x780));
        fpu.st(0) = -fpu.st(0);
        cpu.ebx = cpu.eax + 0x78c;
        putF32(app, cpu.ebp - 0x14, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x784));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp - 0x10, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x788));
        fpu.st(0) = -fpu.st(0);
        cpu.eax = cpu.ebp - 0x14;
        putF32(app, cpu.ebp - 0xc, fpu.pop());
        matrixTimesVector(app, cpu);
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

/* 0x42d160: the car eax's shadow and lights' frame (if +0x660; its
 * headlights' first, if +0x200 has 0x20 and it has them): a point ahead of
 * the car's place along its forward axis (+0x10c) and its up axis (+0x67c,
 * a quarter of +0x11c), to +0x6ac; a matrix of the car's axes (+0x664),
 * sized and coloured as the camera's car and the race's light have it, turned
 * (0x4ea9e0) and transposed into +0x688; the point less the car's in it
 * (+0x6b8). */
void placeCarShadow(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    x86::FPU& fpu = cpu.fpu;
    const reg32 car = cpu.ebp - 4, point = cpu.ebp - 0x28, step = cpu.ebp - 0x10;
    put(app, car, cpu.eax);
    if ((byte(app, cpu.eax + 0x200) & 0x20) != 0 && at(app, cpu.eax + 0x734) != 0)
    {
        if (!direct(app, cpu, placeHeadlights))
            return;
    }
    cpu.eax = at(app, car);
    if (at(app, cpu.eax + 0x660) != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x98);
        put(app, point, cpu.eax);
        cpu.eax = at(app, car);
        cpu.eax = at(app, cpu.eax + 0x9c);
        put(app, point + 4, cpu.eax);
        cpu.eax = at(app, car);
        cpu.eax = at(app, cpu.eax + 0xa0);
        put(app, point + 8, cpu.eax);
        cpu.eax = step;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, car);
        cpu.ecx = point;
        cpu.edx = cpu.eax + 0xcc;
        push(app, cpu, at(app, cpu.eax + 0x10c));
        cpu.eax = 1;
        cpu.ebx = step;
        if (!byAddress(app, cpu, kScaleVectors))
            return;
        cpu.edx = point;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kScaledSum))
            return;
        cpu.eax = step;
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, car);
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kQuarter));
        cpu.ecx = point;
        cpu.edx = cpu.eax + 0x67c;
        cpu.esp -= 4;
        cpu.eax = 1;
        putF32(app, cpu.esp, fpu.pop());
        cpu.ebx = step;
        if (!byAddress(app, cpu, kScaleVectors))
            return;
        cpu.edx = point;
        cpu.eax = 1;
        if (!byAddress(app, cpu, kScaledSum))
            return;
        cpu.eax = at(app, car);
        cpu.esi = point;
        cpu.edi = cpu.eax + 0x6ac;
        moveDwords(app, cpu, 3);
        cpu.ecx = 9;
        cpu.edi = cpu.eax + 0x664;
        cpu.esi = cpu.eax + 0xc0;
        repMoveDwords(app, cpu);
        cpu.ebx = at(app, cpu.eax + 0x660);
        // The camera's car in a race with the race's light on, or not.
        const bool lit = at(app, kLight) != 0;
        reg32 turn;
        if (cpu.ebx == 1)
        {
            cpu.eax = at(app, car);
            if (cpu.ebx == at(app, kCameraCar) && lit)
            {
                put(app, cpu.eax + 0x6c8, 0x41200000);
                put(app, cpu.eax + 0x6cc, 0x42200000);
                put(app, cpu.eax + 0x6d0, 0xb0b0b0b0);
            }
            else
            {
                put(app, cpu.eax + 0x6c8, 0x41a00000);
                put(app, cpu.eax + 0x6cc, 0x42200000);
                put(app, cpu.eax + 0x6d0, 0xffffffff);
            }
            turn = 0x3d800000;
        }
        else
        {
            cpu.eax = at(app, car);
            if (at(app, kCameraCar) == 1 && lit)
            {
                put(app, cpu.eax + 0x6c8, 0x41a00000);
                put(app, cpu.eax + 0x6cc, 0x42700000);
                put(app, cpu.eax + 0x6d0, 0xe0e0e0e0);
            }
            else
            {
                put(app, cpu.eax + 0x6c8, 0x42200000);
                put(app, cpu.eax + 0x6cc, 0x42700000);
                put(app, cpu.eax + 0x6d0, 0xffffffff);
            }
            turn = 0x3d000000;
        }
        put(app, cpu.eax + 0x6c4, 0x3f800000);
        cpu.eax = at(app, car);
        cpu.eax += 0x664;
        push(app, cpu, cpu.eax);
        push(app, cpu, turn);
        if (!byAddress(app, cpu, kTurnAxes))
            return;
        cpu.eax = at(app, car);
        cpu.ecx = cpu.eax + 0x688;
        cpu.eax += 0x664;
        cpu.edx = cpu.ecx;
        put(app, cpu.eax + 0x70, 0);
        if (!byAddress(app, cpu, kTranspose))
            return;
        cpu.eax = at(app, car);
        cpu.edx = cpu.ecx;
        fpu.push(f32(app, cpu.eax + 0x6ac));
        fpu.st(0) = -fpu.st(0);
        cpu.ebx = cpu.eax + 0x6b8;
        putF32(app, cpu.ebp - 0x1c, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x6b0));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp - 0x18, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x6b4));
        fpu.st(0) = -fpu.st(0);
        cpu.eax = cpu.ebp - 0x1c;
        putF32(app, cpu.ebp - 0x14, fpu.pop());
        matrixTimesVector(app, cpu);
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

/* 0x42d3e0: the race's places: the cars by place sorted (insertion) by
 * their node (+0x1c) -- reversed when backwards, through the frame -- each
 * car's place (+0x1fc) set; the cars by distance sorted by +8. */
void sortRaceCars(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x60;
    cpu.ecx = at(app, kCarCount);
    // An insertion sort of the list at `list` by the field at `field`, the
    // car taken out and its key kept in the frame at `carSlot` and `keySlot`.
    const auto sort = [&](reg32 list, reg32 field, reg32 carSlot, reg32 keySlot, reg32& index, reg32& moved) {
        cpu.eax = 1;
        while (sreg32(cpu.eax) < sreg32(cpu.ecx))
        {
            cpu.edx = at(app, cpu.eax * 4 + list);
            cpu.ebx = at(app, cpu.edx + field);
            put(app, carSlot, cpu.edx);
            put(app, keySlot, cpu.ebx);
            cpu.edx = cpu.eax - 1;
            while (sreg32(cpu.edx) >= 0)
            {
                index = cpu.edx * 4;
                moved = at(app, index + list);
                cpu.edi = at(app, keySlot);
                if (sreg32(cpu.edi) >= sreg32(at(app, moved + field)))
                    break;
                --cpu.edx;
                put(app, index + list + 4, moved);
            }
            cpu.ebx = at(app, carSlot);
            ++cpu.eax;
            put(app, cpu.edx * 4 + list + 4, cpu.ebx);
        }
    };
    sort(kByPlace, 0x1c, cpu.ebp - 0x10, cpu.ebp - 0xc, cpu.esi, cpu.ebx);
    if (at(app, kBackwards) != 0)
    {
        cpu.eax = 0;
        while (sreg32(cpu.eax) < sreg32(cpu.ecx))
        {
            cpu.edx = cpu.ecx - 1;
            cpu.edx -= cpu.eax;
            ++cpu.eax;
            cpu.edx = at(app, cpu.edx * 4 + kByPlace);
            put(app, cpu.ebp + cpu.eax * 4 - 0x64, cpu.edx);
        }
        cpu.eax = 0;
        while (sreg32(cpu.eax) < sreg32(cpu.ecx))
        {
            ++cpu.eax;
            cpu.ebx = at(app, cpu.ebp + cpu.eax * 4 - 0x64);
            put(app, cpu.eax * 4 + kByPlace - 4, cpu.ebx);
        }
    }
    cpu.eax = 0;
    while (sreg32(cpu.eax) < sreg32(cpu.ecx))
    {
        cpu.edx = at(app, cpu.eax * 4 + kByPlace);
        put(app, cpu.edx + 0x1fc, cpu.eax);
        ++cpu.eax;
    }
    sort(kByDistance, 8, cpu.ebp - 4, cpu.ebp - 8, cpu.ebx, cpu.esi);
    put(app, kCarCount, cpu.ecx);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d500: a hook of the game's that does nothing -- push ebp, mov ebp,
 * esp, pop ebp (a dozen callers). */
void emptyHook(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d520: another that does nothing, the same. */
void emptyHook2(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d620: the message text eax shown as the race's message lines
 * (0x436e10): whole if it fits (0x441900), else a line at a time -- copied
 * into the frame (100 bytes of it) and broken where the line no longer fits.
 * The two lines' 100 bytes start as the zeros nfs3.exe keeps at 0x42d550 and
 * 0x42d5b4. */
void showMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xcc;
    cpu.ebp -= 0x82;
    const reg32 text = cpu.ebp + 0x7e, copy = cpu.ebp - 0x4a, line = cpu.ebp + 0x1a;
    put(app, text, cpu.eax);
    cpu.ecx = 0x19;
    cpu.edi = copy;
    cpu.esi = kEmptyCopy;
    repMoveDwords(app, cpu);
    cpu.ecx = 0x19;
    cpu.edi = line;
    cpu.esi = kEmptyLine;
    repMoveDwords(app, cpu);
    if (!byAddress(app, cpu, kTextFits))
        return;
    bool show = true;
    if (cpu.eax != 0)
        cpu.edx = at(app, text);
    else
    {
        cpu.ebx = 0x64;
        cpu.eax = line;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
        cpu.ebx = 0x64;
        cpu.edx = at(app, text);
        cpu.eax = copy;
        cpu.edi = copy;
        if (!byAddress(app, cpu, kStrncpy))
            return;
        stringLength(app, cpu);
        cpu.edi = cpu.ecx;
        cpu.edx = 0;
        cpu.ecx = 0;
        while (sreg32(cpu.ecx) < sreg32(cpu.edi))
        {
            ++cpu.edx;
            cpu.al = byte(app, cpu.ecx + copy);
            putByte(app, cpu.edx + line - 1, cpu.al);
            cpu.eax = line;
            ++cpu.ecx;
            if (!byAddress(app, cpu, kTextFits))
                return;
            if (cpu.eax == 0)
            {
                // The line full: ended, shown, cleared, the character again.
                cpu.ah = 0;
                putByte(app, cpu.edx + line, cpu.ah);
                cpu.edx = line;
                cpu.eax = 0;
                cpu.ebx = 0x64;
                if (!byAddress(app, cpu, kShowLine))
                    return;
                cpu.eax = line;
                cpu.edx = 0;
                --cpu.ecx;
                if (!byAddress(app, cpu, kMemset))
                    return;
                cpu.edx = 0;
            }
            if (sreg32(cpu.ecx) >= 0x64)
                break;
        }
        cpu.edi = line;
        stringLength(app, cpu);
        show = cpu.ecx != 0;
        if (show)
            cpu.edx = line;
    }
    if (show)
    {
        cpu.eax = 0;
        if (!byAddress(app, cpu, kShowLine))
            return;
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

/* 0x42d710: the message text edx (ebx long, after its first byte) sent to
 * every other machine of a network race (the players' list at 0x7a21f0 from
 * 1, but the machine eax) while the network is up ([0x7a22da], [0x7a22e4] 1),
 * and its text added to the message lines; [0x7a22a8] 1.  No caller but a
 * pointer. */
void sendMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 from = cpu.ebp - 8, message = cpu.ebp - 4;
    put(app, from, cpu.eax);
    cpu.esi = cpu.edx;
    cpu.edi = cpu.ebx;
    put(app, message, cpu.edx);
    if (half(app, kNetworkUp) != 0)
    {
        cpu.edx = at(app, kNetworkMode);
        if (cpu.edx == 1)
        {
            cpu.ecx = cpu.edx;
            for (;;)
            {
                cpu.eax = at(app, kMachines);
                cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
                if (sreg32(cpu.ecx) >= sreg32(cpu.eax))
                    break;
                cpu.eax = cpu.ecx * 4;
                cpu.ebx = at(app, from);
                cpu.edx = at(app, cpu.eax + kPlayers);
                if (cpu.edx != cpu.ebx)
                {
                    cpu.eax = cpu.edx;
                    cpu.ebx = cpu.edi;
                    cpu.edx = cpu.esi;
                    if (!byAddress(app, cpu, kSendPacket))
                        return;
                }
                ++cpu.ecx;
            }
        }
    }
    cpu.eax = at(app, message);
    ++cpu.eax;
    cpu.ebx = 1;
    if (!direct(app, cpu, appendMessageLine))
        return;
    putHalf(app, kMessageSent, cpu.bx);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d790: the message box opened (0x436c40, its description in the frame:
 * 1, 10, 100, 0, 0, 0, 7, 1) and the message lines made afresh. */
void openMessages(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    cpu.edx = 1;
    cpu.ecx = 0xa;
    cpu.ebx = 0x64;
    cpu.eax = cpu.ebp - 0x20;
    cpu.esi = 0;
    put(app, cpu.ebp - 0x20, cpu.edx);
    put(app, cpu.ebp - 0x1c, cpu.ecx);
    put(app, cpu.ebp - 0x18, cpu.ebx);
    put(app, cpu.ebp - 0x14, cpu.esi);
    put(app, cpu.ebp - 0x10, cpu.esi);
    put(app, cpu.ebp - 0xc, cpu.esi);
    cpu.edx = 7;
    cpu.ecx = 1;
    put(app, cpu.ebp - 8, cpu.edx);
    put(app, cpu.ebp - 4, cpu.ecx);
    if (!byAddress(app, cpu, kOpenMessageBox) || !direct(app, cpu, openMessageLines))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d7f0: the message box closed (0x436cf0) and the message lines with
 * it. */
void closeMessages(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, kCloseMessageBox) || !direct(app, cpu, closeMessageLines))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d800: the message box cleared (0x436d40) and the message lines after
 * the first. */
void clearMessages(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, kClearMessageBox) || !direct(app, cpu, clearMessageLines))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d810: the message text edx sent to the other machines (0x4a60e0, a
 * packet of a newline byte and the text, its ending too) and shown. */
void sendTypedMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x68;
    const reg32 packet = cpu.ebp - 0x68;
    cpu.esi = cpu.edx;
    cpu.edi = cpu.edx;
    stringLength(app, cpu);
    cpu.ah = 0xa;
    cpu.edx = packet + 1;
    ++cpu.ecx;
    putByte(app, packet, cpu.ah);
    cpu.ebx = cpu.ecx;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kMoveMemory))
        return;
    cpu.edx = packet;
    cpu.ebx = cpu.ecx + 1;
    cpu.eax = 0;
    if (!byAddress(app, cpu, kSendPacket))
        return;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, showMessage))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d860: a message line allocated (0x70 bytes) and cleared; or 0. */
void newMessageLine(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = 0x10;
    cpu.edx = kLineBytes;
    cpu.eax = kLineName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ecx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.ebx = kLineBytes;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
    }
    cpu.eax = cpu.ecx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d8a0: the message line eax freed, if there is one. */
void freeMessageLine(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (cpu.eax != 0 && !byAddress(app, cpu, kFree))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d8b0: a message line of the text edx (100 bytes of it, strncpy), from
 * eax, put after the line ebx; the line, or 0 (no text, no memory). */
void insertMessageLine(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.ecx = cpu.ebx;
    if (cpu.edx == 0)
        cpu.eax = 0;
    else
    {
        if (!direct(app, cpu, newMessageLine))
            return;
        cpu.esi = cpu.eax;
        cpu.edi = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.ebx = 0x64;
            put(app, cpu.esi, cpu.eax);
            cpu.eax = cpu.esi + 4;
            if (!byAddress(app, cpu, kStrncpy))
                return;
            cpu.edx = at(app, cpu.ecx + 0x68);
            if (cpu.edx != 0)
                put(app, cpu.edx + 0x6c, cpu.esi);
            cpu.eax = at(app, cpu.ecx + 0x68);
            put(app, cpu.edi + 0x68, cpu.eax);
            put(app, cpu.edi + 0x6c, cpu.ecx);
            cpu.eax = cpu.edi;
            put(app, cpu.ecx + 0x68, cpu.edi);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d910: the message line eax and every one after it freed. */
void freeMessageLines(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.eax + 0x68);
        if (!direct(app, cpu, freeMessageLine))
            return;
        while (cpu.edx != 0)
        {
            cpu.eax = cpu.edx;
            cpu.ecx = at(app, cpu.edx + 0x68);
            if (!direct(app, cpu, freeMessageLine))
                return;
            cpu.edx = cpu.ecx;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The message lines' lock ([[0x5537a0]]) taken or let go of: its address
 * pushed from `reg`, EnterCriticalSection or LeaveCriticalSection called. */
bool linesLock(win32::WinApplication* app, x86::CPU& cpu, reg32& reg, reg32 slot)
{
    cpu.eax = at(app, kLines);
    reg = at(app, cpu.eax);
    push(app, cpu, reg);
    return byAddress(app, cpu, at(app, slot));
}

/* The lines after the first freed and the first the last again. */
bool dropLines(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, freeMessageLines))
        return false;
    cpu.eax = at(app, kLines);
    cpu.edx = at(app, cpu.eax + 4);
    put(app, cpu.edx + 0x68, 0);
    cpu.edx = at(app, cpu.eax + 4);
    put(app, cpu.eax + 8, cpu.edx);
    return true;
}

}  // namespace

/* 0x42d940: the message lines after the first freed, under their lock. */
void clearMessageLines(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!linesLock(app, cpu, cpu.edx, kEnterCriticalSection))
        return;
    cpu.ecx = at(app, kLines);
    if (cpu.ecx != 0)
    {
        cpu.ebx = at(app, cpu.ecx + 4);
        cpu.eax = cpu.ecx;
        if (cpu.ebx != 0)
        {
            cpu.esi = at(app, cpu.ebx + 0x68);
            cpu.eax = cpu.ebx;
            if (cpu.esi != 0 && !dropLines(app, cpu))
                return;
        }
    }
    if (!linesLock(app, cpu, cpu.edi, kLeaveCriticalSection))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42d9b0: each message line after the first shown (showMessage), and
 * then freed, under their lock. */
void showMessageLines(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!linesLock(app, cpu, cpu.edx, kEnterCriticalSection))
        return;
    cpu.ecx = at(app, kLines);
    if (cpu.ecx != 0)
    {
        cpu.ebx = at(app, cpu.ecx + 4);
        cpu.eax = cpu.ecx;
        if (cpu.ebx != 0)
        {
            cpu.edx = at(app, cpu.ebx + 0x68);
            while (cpu.edx != 0)
            {
                cpu.eax = cpu.edx + 4;
                if (!direct(app, cpu, showMessage))
                    return;
                cpu.edx = at(app, cpu.edx + 0x68);
            }
            cpu.eax = at(app, kLines);
            cpu.eax = at(app, cpu.eax + 4);
            cpu.esi = at(app, cpu.eax + 0x68);
            if (cpu.esi != 0 && !dropLines(app, cpu))
                return;
        }
    }
    if (!linesLock(app, cpu, cpu.edi, kLeaveCriticalSection))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42da30: the text eax added as the last message line, numbered by
 * [0x55379c], under their lock -- if there is a text and the lines are. */
void appendMessageLine(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    if (!linesLock(app, cpu, cpu.edx, kEnterCriticalSection))
        return;
    if (cpu.esi != 0)
    {
        cpu.ecx = at(app, kLines);
        if (cpu.ecx != 0)
        {
            cpu.ebx = at(app, cpu.ecx + 4);
            cpu.eax = cpu.ecx;
            if (cpu.ebx != 0)
            {
                cpu.edi = at(app, cpu.ecx + 8);
                if (cpu.edi != 0)
                {
                    cpu.eax = at(app, kLineSerial);
                    cpu.ebx = cpu.edi;
                    ++cpu.eax;
                    cpu.edx = cpu.esi;
                    put(app, kLineSerial, cpu.eax);
                    if (!direct(app, cpu, insertMessageLine))
                        return;
                    if (cpu.eax != 0)
                    {
                        cpu.ebx = at(app, kLines);
                        put(app, cpu.ebx + 8, cpu.eax);
                    }
                }
            }
        }
    }
    if (!linesLock(app, cpu, cpu.edx, kLeaveCriticalSection))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42daf0: the message lines freed, under their lock, the lock freed
 * (0x4eb360) and the lines' head freed and 0. */
void closeMessageLines(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kLines);
    if (cpu.edx != 0)
    {
        cpu.ecx = at(app, cpu.edx);
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kEnterCriticalSection)))
            return;
        cpu.eax = at(app, kLines);
        cpu.ebx = at(app, cpu.eax + 4);
        if (cpu.ebx != 0)
        {
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, freeMessageLines))
                return;
            cpu.eax = at(app, kLines);
            put(app, cpu.eax + 4, 0);
            put(app, cpu.eax + 8, 0);
        }
        if (!linesLock(app, cpu, cpu.esi, kLeaveCriticalSection))
            return;
        cpu.eax = at(app, kLines);
        cpu.eax = at(app, cpu.eax);
        if (!byAddress(app, cpu, kFreeLock))
            return;
        cpu.eax = at(app, kLines);
        cpu.edi = 0;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, kLines, cpu.edi);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42db70: the message lines made afresh: the old freed, their head (12
 * bytes: the lock, the first line, the last) allocated, its lock made
 * (0x4eb2e0) and, under it, an empty first line -- none, and all of it
 * freed again. */
void openMessageLines(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, closeMessageLines))
        return;
    cpu.ebx = 0x10;
    cpu.edx = 0xc;
    cpu.eax = kLineName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    put(app, kLines, cpu.eax);
    if (cpu.eax != 0)
    {
        if (!byAddress(app, cpu, kMakeLock))
            return;
        cpu.edx = at(app, kLines);
        push(app, cpu, cpu.eax);
        put(app, cpu.edx, cpu.eax);
        if (!byAddress(app, cpu, at(app, kEnterCriticalSection)))
            return;
        cpu.eax = at(app, kLines);
        put(app, cpu.eax + 4, 0);
        put(app, cpu.eax + 8, 0);
        if (!direct(app, cpu, newMessageLine))
            return;
        cpu.edx = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, kLines);
            cpu.ebx = at(app, cpu.eax);
            put(app, cpu.eax + 4, cpu.edx);
            push(app, cpu, cpu.ebx);
            put(app, cpu.eax + 8, cpu.edx);
            if (!byAddress(app, cpu, at(app, kLeaveCriticalSection)))
                return;
        }
        else
        {
            if (!linesLock(app, cpu, cpu.esi, kLeaveCriticalSection))
                return;
            cpu.eax = at(app, kLines);
            cpu.eax = at(app, cpu.eax);
            if (!byAddress(app, cpu, kFreeLock))
                return;
            cpu.eax = at(app, kLines);
            cpu.edi = 0;
            if (!byAddress(app, cpu, kFree))
                return;
            put(app, kLines, cpu.edi);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42dc20: the race controls' thread: an event made ([0x5efdf0]), then
 * for ever the event waited for and the race's controls read (0x48f550).
 * Its loop's head the generated code's safepoint. */
void raceControlsThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, kCreateEvent))
        return;
    put(app, kControlsEvent, cpu.eax);
    for (;;)
    {
        if (app->contextWanted())
            app->yieldContext(cpu);
        cpu.eax = at(app, kControlsEvent);
        if (!byAddress(app, cpu, kWaitEvent) || !byAddress(app, cpu, kReadControls))
            return;
    }
}

/* 0x42dc40: the race controls' event set (0x4dfb20, eax 0x7a3aac). */
void wakeRaceControls(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = kControlsThread;
    cpu.edx = at(app, kControlsEvent);
    if (!byAddress(app, cpu, kPostEvent))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42dc60: the game's timer's call: the ticks counted ([0x5efdf4],
 * [0x5efe00], eax the last), every second of them counted again
 * ([0x5efdf8]) and every second of those -- a fourth tick -- the race
 * controls woken ([0x5efdfc] counts them). */
void raceControlsTick(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, kTicks, at(app, kTicks) + 1);
    cpu.eax = at(app, kAllTicks);
    put(app, kAllTicks, at(app, kAllTicks) + 1);
    if ((byte(app, kTicks) & 1) == 0)
    {
        put(app, kHalfTicks, at(app, kHalfTicks) + 1);
        if ((byte(app, kHalfTicks) & 1) == 0)
        {
            cpu.esi = at(app, kWakes);
            cpu.eax = kControlsThread;
            ++cpu.esi;
            cpu.edx = at(app, kControlsEvent);
            put(app, kWakes, cpu.esi);
            if (!byAddress(app, cpu, kPostEvent))
                return;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42dcb0: the race controls' ticks from 0, and, the first time
 * ([0x5537a4]), raceControlsTick given to the game's timer (0x48ee30) and
 * the counts -- 0x79c4ac's too -- 0. */
void startRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    put(app, kAllTicks, cpu.edx);
    if (at(app, kTicking) == 0)
    {
        cpu.ebx = 1;
        cpu.eax = kTickFunction;
        put(app, kTicking, cpu.ebx);
        if (!byAddress(app, cpu, kAddTimer))
            return;
        put(app, kTicks, cpu.edx);
        put(app, kHalfTicks, cpu.edx);
        put(app, kWakes, cpu.edx);
        put(app, kTimerCount, cpu.edx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42dd00: raceControlsTick taken from the game's timer (0x48ee90), if it
 * was given. */
void stopRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kTicking) != 0)
    {
        cpu.ecx = 0;
        cpu.eax = kTickFunction;
        put(app, kTicking, cpu.ecx);
        if (!byAddress(app, cpu, kRemoveTimer))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
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
void cornerInBox(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    // Not inside: 0.
    const auto miss = [&] {
        cpu.eax = 0;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 8;
    };
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.ecx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0xc, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 4));
    put(app, cpu.ebp - 4, cpu.ebx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 8, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 12));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 24));
    cpu.eax = at(app, cpu.ebp + 0xc);
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 16));
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 28));
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0x10, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 8));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 20));
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 32));
    fpu.push(f32(app, kPlaceBA));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceBA + 4));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceBA + 8));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, kCorner0, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kCorner1, fpu.pop());
    putF32(app, kCorner2, fpu.st(0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x11c));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x11c));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner2));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x114));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner0));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner1));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x118));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x118));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner1));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc0));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc4));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xc8));
    fpu.push(f32(app, cpu.edx + 0xc0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xcc));
    fpu.push(f32(app, cpu.edx + 0xc4));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xd8));
    fpu.push(f32(app, cpu.edx + 0xc8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(6) = fp::add(fpu.st(6), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(5));
    putF32(app, kContact, fpu.pop());
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xd0));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xdc));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xd4));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x98));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, kContact + 4, fpu.pop());
    fpu.push(f32(app, cpu.edx + 0xcc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x9c));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0xe0));
    fpu.push(f32(app, cpu.edx + 0xd0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0xa0));
    std::swap(fpu.st(0), fpu.st(3));
    putF32(app, kContact + 8, fpu.pop());
    fpu.push(f32(app, cpu.edx + 0xd4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    fpu.push(f32(app, cpu.edx + 0xd8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.edx + 0xdc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    fpu.push(f32(app, cpu.edx + 0xe0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kContactOther, fpu.pop());
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kContactOther + 4, fpu.pop());
    putF32(app, kContactOther + 8, fpu.pop());
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x42e010: cornerInBox the other way round: the corner of the car eax's
 * box taken into the car edx's frame (the axes at 0x5efe90, less the place
 * at 0x5efe6c), held against edx's sizes; the corner from eax's axes and
 * sizes made negative (0x5efe78) and from eax's middle (0x5efe84). */
void cornerInBoxBack(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    // Not inside: 0.
    const auto miss = [&] {
        cpu.eax = 0;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 8;
    };
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.ecx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 8, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 12));
    put(app, cpu.ebp - 4, cpu.ebx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0xc, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 4));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 8));
    cpu.eax = at(app, cpu.ebp + 0xc);
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 16));
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 20));
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0x10, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 24));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 28));
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 32));
    fpu.push(f32(app, kPlaceAB));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceAB + 4));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(4) = fp::sub(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceAB + 8));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, kCorner0, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kCorner1, fpu.pop());
    putF32(app, kCorner2, fpu.st(0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x11c));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x11c));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner2));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x114));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner0));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner1));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x118));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x118));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner1));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xc0));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xc4));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xc8));
    fpu.push(f32(app, cpu.esi + 0xc0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xcc));
    fpu.push(f32(app, cpu.esi + 0xc4));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xd8));
    fpu.push(f32(app, cpu.esi + 0xc8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(6) = fp::add(fpu.st(6), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(0) = -fpu.st(0);
    putF32(app, kContact, fpu.pop());
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xd0));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0xc));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xdc));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xd4));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x98));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(0) = -fpu.st(0);
    putF32(app, kContact + 4, fpu.pop());
    fpu.push(f32(app, cpu.esi + 0xcc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x9c));
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0xe0));
    fpu.push(f32(app, cpu.esi + 0xd0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0xa0));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = -fpu.st(0);
    putF32(app, kContact + 8, fpu.pop());
    fpu.push(f32(app, cpu.esi + 0xd4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 8));
    fpu.push(f32(app, cpu.esi + 0xd8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.esi + 0xdc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    fpu.push(f32(app, cpu.esi + 0xe0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x10));
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kContactOther, fpu.pop());
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kContactOther + 4, fpu.pop());
    putF32(app, kContactOther + 8, fpu.pop());
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x42e2c0: cornerInBox with the corner from edx's axes worked out the
 * other way about -- each axis times its size first, then by the corner's
 * sign -- and stored in another order. */
void cornerInBox2(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    // Not inside: 0.
    const auto miss = [&] {
        cpu.eax = 0;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 8;
    };
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.ecx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0x10, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 4));
    put(app, cpu.ebp - 4, cpu.ebx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0xc, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 12));
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 24));
    cpu.eax = at(app, cpu.ebp + 0xc);
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 16));
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 28));
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 8, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 8));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 20));
    fpu.push(f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesBA + 32));
    fpu.push(f32(app, kPlaceBA));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceBA + 4));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(4) = fp::add(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceBA + 8));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, kCorner0, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kCorner1, fpu.pop());
    putF32(app, kCorner2, fpu.st(0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x11c));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x11c));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner2));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x114));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner0));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner1));
    s = compare(cpu, fpu.st(0), f32(app, cpu.esi + 0x118));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0x118));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner1));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0xc0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.push(f32(app, cpu.edx + 0xc4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.push(f32(app, cpu.edx + 0xc8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x114));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.push(f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x98));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.edx + 0xcc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0x9c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.edx + 0xa0));
    fpu.push(f32(app, cpu.edx + 0xd0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    fpu.push(f32(app, cpu.edx + 0xd4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x118));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(6) = fp::add(fpu.st(6), fpu.st(0));
    fpu.pop();
    fpu.st(4) = fp::mul(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.edx + 0xd8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.edx + 0xdc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    fpu.push(f32(app, cpu.edx + 0xe0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.edx + 0x11c));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.push(f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
    fpu.pop();
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    putF32(app, kContactOther + 4, fpu.pop());
    putF32(app, kContactOther + 8, fpu.pop());
    putF32(app, kContactOther, fpu.pop());
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x42e4f0: cornerInBoxBack worked out as cornerInBox2 is. */
void cornerInBoxBack2(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    // Not inside: 0.
    const auto miss = [&] {
        cpu.eax = 0;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 8;
    };
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.ecx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0xc, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 12));
    put(app, cpu.ebp - 4, cpu.ebx);
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 8, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 4));
    fpu.push(f32(app, cpu.ebp - 8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 8));
    cpu.eax = at(app, cpu.ebp + 0xc);
    fpu.push(f32(app, cpu.ebp - 0xc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 16));
    put(app, cpu.ebp - 4, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 20));
    fpu.push(x86::Float(sreg32(at(app, cpu.ebp - 4))));
    putF32(app, cpu.ebp - 0x10, fpu.st(0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 24));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x10));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 28));
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kAxesAB + 32));
    fpu.push(f32(app, kPlaceAB));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceAB + 4));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(4) = fp::sub(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, kPlaceAB + 8));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, kCorner0, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, kCorner1, fpu.pop());
    putF32(app, kCorner2, fpu.st(0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x11c));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x11c));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner2));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner0));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x114));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x114));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner0));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, kCorner1));
    s = compare(cpu, fpu.st(0), f32(app, cpu.edx + 0x118));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.edx + 0x118));
    fpu.st(0) = -fpu.st(0);
    s = compare(cpu, fpu.st(0), f32(app, kCorner1));
    fpu.pop();
    if (!s.below && !s.equal)
        return miss();
    fpu.push(f32(app, cpu.esi + 0xc0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.push(f32(app, cpu.esi + 0xc4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    fpu.push(f32(app, cpu.esi + 0xc8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x114));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.push(f32(app, cpu.ebp - 8));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x98));
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.esi + 0xcc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0x9c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.esi + 0xa0));
    fpu.push(f32(app, cpu.esi + 0xd0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    fpu.push(f32(app, cpu.esi + 0xd4));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x118));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.push(f32(app, cpu.ebp - 0xc));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(6) = fp::add(fpu.st(6), fpu.st(0));
    fpu.pop();
    fpu.st(4) = fp::mul(fpu.st(4), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.esi + 0xd8));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(3) = fp::add(fpu.st(3), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(1) = fp::add(fpu.st(1), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.esi + 0xdc));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    fpu.push(f32(app, cpu.esi + 0xe0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.esi + 0x11c));
    std::swap(fpu.st(0), fpu.st(4));
    fpu.push(f32(app, cpu.ebp - 0x10));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    std::swap(fpu.st(0), fpu.st(5));
    fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
    fpu.pop();
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(5) = fp::add(fpu.st(5), fpu.st(0));
    fpu.pop();
    std::swap(fpu.st(0), fpu.st(3));
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));
    fpu.pop();
    putF32(app, kContactOther + 4, fpu.pop());
    putF32(app, kContactOther + 8, fpu.pop());
    putF32(app, kContactOther, fpu.pop());
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x42e720: the car eax's spin (+0xe4, +0xe8, +0xec) held within plus and
 * minus 1.8 ([0x5537b0]) about each axis, the next component's loads
 * interleaved with the last's store as the code has them. */
void clampSpin(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    x86::FPU& fpu = cpu.fpu;
    cpu.edx = cpu.eax;
    // A component at `field` held: `least` the frame's slot of minus the
    // most, `held` that of the most or the component; eax what it is held to.
    const auto hold = [&](reg32 field, reg32 least, reg32 held) {
        const Status s = compare(cpu, fpu.st(0), f32(app, least));
        fpu.pop();
        if (!s.below)
        {
            fpu.push(f32(app, cpu.edx + field));
            const Status t = compare(cpu, fpu.st(0), f32(app, kMostSpin));
            fpu.pop();
            cpu.eax = t.below || t.equal ? at(app, cpu.edx + field) : at(app, kMostSpin);
            put(app, held, cpu.eax);
            cpu.eax = at(app, held);
        }
        else
            cpu.eax = at(app, least);
    };
    fpu.push(f32(app, cpu.eax + 0xe4));
    fpu.push(f32(app, kMostSpin));
    fpu.st(0) = -fpu.st(0);
    putF32(app, cpu.ebp - 0x24, fpu.pop());
    hold(0xe4, cpu.ebp - 0x24, cpu.ebp - 8);
    put(app, cpu.ebp - 0xc, cpu.eax);
    fpu.push(f32(app, kMostSpin));
    fpu.push(f32(app, cpu.edx + 0xe8));
    cpu.eax = at(app, cpu.ebp - 0xc);
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = -fpu.st(0);
    putF32(app, cpu.ebp - 4, fpu.pop());
    put(app, cpu.edx + 0xe4, cpu.eax);
    hold(0xe8, cpu.ebp - 4, cpu.ebp - 0x18);
    put(app, cpu.ebp - 0x20, cpu.eax);
    fpu.push(f32(app, kMostSpin));
    fpu.push(f32(app, cpu.edx + 0xec));
    cpu.eax = at(app, cpu.ebp - 0x20);
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = -fpu.st(0);
    putF32(app, cpu.ebp - 0x14, fpu.pop());
    put(app, cpu.edx + 0xe8, cpu.eax);
    hold(0xec, cpu.ebp - 0x14, cpu.ebp - 0x10);
    put(app, cpu.ebp - 0x1c, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0x1c);
    put(app, cpu.edx + 0xec, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
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
void applyImpulse(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x40;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.ebp + 0x1c;
    if (!byAddress(app, cpu, kNormalize))
        return;
    fpu.push(f32(app, cpu.ecx + 0xb0));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, kFiveTimes));
    s = compare(cpu, fpu.st(0), fpu.st(1));
    fpu.pop();
    fpu.pop();
    if (s.below)
    {
        cpu.eax = cpu.ebp + 0x1c;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.ebp + 0x1c;
        cpu.eax = 1;
        push(app, cpu, at(app, kImpulseScale));
        if (!byAddress(app, cpu, kScaleVectors))
            return;
    }
    cpu.edi = cpu.ebp - 0x28;
    cpu.esi = cpu.ebp + 0x28;
    cpu.eax = cpu.ebp - 0x28;
    cpu.edx = cpu.ebp - 0x28;
    moveDwords(app, cpu, 3);
    if (!byAddress(app, cpu, kVectorLength))
        return;
    cpu.eax = cpu.ebp + 0x1c;
    putF32(app, cpu.ebp - 0xc, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.st(0) = -fpu.st(0);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp + 0x34));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0xb4));
    fpu.push(x86::Float(0.0));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 4, fpu.pop());
    s = compare(cpu, fpu.st(0), f32(app, cpu.ebp - 4));
    fpu.pop();
    if (s.below)
    {
        cpu.edi = cpu.ebp - 0x1c;
        cpu.esi = cpu.ebp + 0x1c;
        cpu.eax = cpu.ebp - 0x1c;
        moveDwords(app, cpu, 3);
        if (!byAddress(app, cpu, kVectorLength))
            return;
        fpu.push(f32(app, cpu.ebp - 4));
        fpu.push(f32(app, cpu.ebp - 0x1c));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        fpu.push(f32(app, cpu.ebp - 0x18));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(2));
        fpu.push(f32(app, cpu.ebp - 0x14));
        fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(3));
        fpu.pop();
        fpu.push(f32(app, cpu.ecx + 0xa4));
        fpu.push(f32(app, cpu.ecx + 0xa8));
        fpu.push(f32(app, cpu.ecx + 0xac));
        std::swap(fpu.st(0), fpu.st(3));
        putF32(app, cpu.ebp - 0x40, fpu.pop());
        std::swap(fpu.st(0), fpu.st(4));
        putF32(app, cpu.ebp - 0x3c, fpu.pop());
        std::swap(fpu.st(0), fpu.st(2));
        putF32(app, cpu.ebp - 0x38, fpu.pop());
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x40));
        std::swap(fpu.st(0), fpu.st(2));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x3c));
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x38));
        std::swap(fpu.st(0), fpu.st(2));
        putF32(app, cpu.ecx + 0xa4, fpu.pop());
        putF32(app, cpu.ecx + 0xa8, fpu.pop());
        putF32(app, cpu.ecx + 0xac, fpu.pop());
    }
    else
    {
        cpu.edx = 0;
        put(app, cpu.ebp - 4, cpu.edx);
    }
    if ((at(app, cpu.ebp - 0xc) & 0x7fffffff) != 0)
    {
        cpu.ebx = cpu.ebp - 0x34;
        cpu.edx = cpu.ebp + 0x1c;
        cpu.eax = cpu.ebp - 0x28;
        if (!byAddress(app, cpu, kCross))
            return;
        cpu.eax = cpu.ebp - 0x34;
        if (!byAddress(app, cpu, kNormalize))
            return;
        fpu.push(f32(app, cpu.ecx + 0x120));
        fpu.push(x86::Float(1.0));
        fpu.st(1) = fp::div(fpu.st(0), fpu.st(1));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0xb4));
        fpu.push(f32(app, cpu.ebp + 0x34));
        fpu.st(2) = fp::mul(fpu.st(2), fpu.st(0));
        fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kOneTurn));
        cpu.eax = cpu.ebp - 0x34;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ecx + 0xb4));
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.ebp - 0x34;
        cpu.esp -= 4;
        cpu.eax = 1;
        putF32(app, cpu.esp, fpu.pop());
        putF32(app, cpu.ebp - 8, fpu.pop());
        if (!byAddress(app, cpu, kScaleVectors))
            return;
        fpu.push(f32(app, cpu.ecx + 0xe4));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x34));
        fpu.push(f32(app, cpu.ecx + 0xe8));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ecx + 0xe4, fpu.pop());
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x30));
        fpu.push(f32(app, cpu.ecx + 0xec));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ecx + 0xe8, fpu.pop());
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x2c));
        putF32(app, cpu.ecx + 0xec, fpu.pop());
    }
    else
    {
        cpu.esi = 0;
        put(app, cpu.ebp - 8, cpu.esi);
    }
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, clampSpin))
        return;
    fpu.push(f32(app, cpu.ebp - 4));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 8));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 0x1c;
}

/* 0x42e9c0: the car eax bounced off a wall of normal edx with the strength
 * [esp+4] (ret 4): its velocity (+0xa4), spin (+0xe4) and the normal taken
 * into the car's frame (its axes +0xc0 transposed); the velocity's part along
 * the normal taken off, the rest damped -- each component of 22 or more by
 * 0.958, the last by 0.975 when the car's up axis (+0xcc) is more than 0.9
 * along the normal -- and the part along the normal put back; both back into
 * the world. */
void bounceOffWall(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x78;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edx = cpu.ebp - 0x78;
    cpu.eax += 0xc0;
    cpu.ebx = cpu.ebp - 0x48;
    if (!byAddress(app, cpu, kTranspose))
        return;
    cpu.edx = cpu.ebp - 0x78;
    cpu.eax = cpu.ecx + 0xa4;
    matrixTimesVector(app, cpu);
    cpu.ebx = cpu.ebp - 0x3c;
    cpu.edx = cpu.ebp - 0x78;
    cpu.eax = cpu.ecx + 0xe4;
    matrixTimesVector(app, cpu);
    cpu.ebx = cpu.ebp - 0x54;
    cpu.edx = cpu.ebp - 0x78;
    cpu.eax = cpu.esi;
    matrixTimesVector(app, cpu);
    cpu.edx = cpu.ebp - 0x54;
    cpu.eax = cpu.ebp - 0x48;
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(f32(app, cpu.ebp - 0x54));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.push(f32(app, cpu.ebp - 0x50));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(2));
    fpu.push(f32(app, cpu.ebp - 0x4c));
    fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x48));
    fpu.push(f32(app, cpu.ebp - 0x44));
    fpu.push(f32(app, cpu.ebp - 0x40));
    cpu.eax = cpu.ecx + 0xcc;
    cpu.edx = cpu.esi;
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, cpu.ebp - 0x30, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    putF32(app, cpu.ebp - 0x2c, fpu.pop());
    std::swap(fpu.st(0), fpu.st(3));
    putF32(app, cpu.ebp - 0x28, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x30));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x2c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x28));
    std::swap(fpu.st(0), fpu.st(2));
    putF32(app, cpu.ebp - 0x48, fpu.pop());
    putF32(app, cpu.ebp - 0x44, fpu.pop());
    putF32(app, cpu.ebp - 0x40, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    s = compare(cpu, fpu.st(0), f64(app, kNinetenths));
    fpu.pop();
    // The component at `component`: its size to `size`, and whether that is
    // past [0x5537b8] -- it is then damped by [0x5537b4] (dampFully).
    const auto damp = [&](reg32 component, reg32 size) {
        fpu.push(x86::Float(0.0));
        s = compare(cpu, fpu.st(0), f32(app, component));
        fpu.pop();
        if (!s.below && !s.equal)
        {
            fpu.push(f32(app, component));
            fpu.st(0) = -fpu.st(0);
            putF32(app, size, fpu.pop());
        }
        else
        {
            cpu.eax = at(app, component);
            put(app, size, cpu.eax);
        }
        fpu.push(f32(app, size));
        s = compare(cpu, fpu.st(0), f32(app, kFastAlong));
        fpu.pop();
        return !s.below && !s.equal;
    };
    const auto dampFully = [&](reg32 component) {
        fpu.push(f32(app, component));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, kWallDamping));
        putF32(app, component, fpu.pop());
    };
    if (!s.below && !s.equal)
    {
        if (damp(cpu.ebp - 0x48, cpu.ebp - 8))
            dampFully(cpu.ebp - 0x48);
        if (damp(cpu.ebp - 0x44, cpu.ebp - 4))
            dampFully(cpu.ebp - 0x44);
        if (damp(cpu.ebp - 0x40, cpu.ebp - 0x10))
        {
            fpu.push(f32(app, cpu.ebp - 0x40));
            fpu.push(x86::Float(fpu.st(0)));
            fpu.st(0) = fp::mul(fpu.st(0), f64(app, kSlowZ));
            fpu.st(1) = fpu.st(0);
            fpu.pop();
            putF32(app, cpu.ebp - 0x40, fpu.pop());
        }
    }
    else
    {
        if (damp(cpu.ebp - 0x48, cpu.ebp - 0xc))
            dampFully(cpu.ebp - 0x48);
        if (damp(cpu.ebp - 0x44, cpu.ebp - 0x18))
            dampFully(cpu.ebp - 0x44);
        if (damp(cpu.ebp - 0x40, cpu.ebp - 0x14))
            dampFully(cpu.ebp - 0x40);
    }
    fpu.push(f32(app, cpu.ebp - 0x48));
    fpu.push(f32(app, cpu.ebp - 0x44));
    fpu.push(f32(app, cpu.ebp - 0x40));
    cpu.edx = cpu.ebp - 0x54;
    cpu.eax = cpu.ebp - 0x3c;
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x30));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x2c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x28));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x48, fpu.pop());
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x44, fpu.pop());
    putF32(app, cpu.ebp - 0x40, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(f32(app, cpu.ebp - 0x54));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
    fpu.push(f32(app, cpu.ebp - 0x50));
    fpu.st(0) = fp::mul(fpu.st(0), fpu.st(2));
    cpu.ebx = cpu.ecx + 0xa4;
    fpu.push(f32(app, cpu.ebp - 0x4c));
    fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
    fpu.pop();
    cpu.esi = cpu.ecx + 0xc0;
    fpu.push(f32(app, cpu.ebp - 0x3c));
    cpu.eax = cpu.ebp - 0x30;
    fpu.push(f32(app, cpu.ebp - 0x38));
    push(app, cpu, cpu.eax);
    fpu.push(f32(app, cpu.ebp - 0x34));
    cpu.edx = cpu.ebp - 0x30;
    push(app, cpu, at(app, kWallDamping));
    std::swap(fpu.st(0), fpu.st(4));
    putF32(app, cpu.ebp - 0x30, fpu.pop());
    std::swap(fpu.st(0), fpu.st(2));
    putF32(app, cpu.ebp - 0x2c, fpu.pop());
    std::swap(fpu.st(0), fpu.st(3));
    putF32(app, cpu.ebp - 0x28, fpu.pop());
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x30));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x2c));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x28));
    std::swap(fpu.st(0), fpu.st(2));
    putF32(app, cpu.ebp - 0x3c, fpu.pop());
    putF32(app, cpu.ebp - 0x38, fpu.pop());
    putF32(app, cpu.ebp - 0x34, fpu.pop());
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    fpu.push(f32(app, cpu.ebp - 0x3c));
    fpu.push(f32(app, cpu.ebp - 0x38));
    fpu.push(f32(app, cpu.ebp - 0x34));
    cpu.eax = cpu.ebp - 0x48;
    cpu.edx = cpu.esi;
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x30));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x2c));
    std::swap(fpu.st(0), fpu.st(2));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x28));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x3c, fpu.pop());
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x38, fpu.pop());
    putF32(app, cpu.ebp - 0x34, fpu.pop());
    matrixTimesVector(app, cpu);
    cpu.eax = cpu.ebp - 0x3c;
    cpu.ebx = cpu.ecx + 0xe4;
    cpu.edx = cpu.esi;
    matrixTimesVector(app, cpu);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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
void hitWall(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x4c;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.esi = cpu.edx;
    put(app, cpu.ebp - 8, cpu.ebx);
    fpu.push(f32(app, cpu.esi));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ecx));
    putF32(app, cpu.ebp - 0x40, fpu.pop());
    fpu.push(f32(app, cpu.esi + 4));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ecx + 4));
    cpu.eax = cpu.ebp - 0x40;
    putF32(app, cpu.ebp - 0x3c, fpu.pop());
    fpu.push(f32(app, cpu.esi + 8));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ecx + 8));
    cpu.edx = cpu.ebx;
    putF32(app, cpu.ebp - 0x38, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(x86::Float(0.0));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0xc, fpu.pop());
    s = compare(cpu, fpu.st(0), f32(app, cpu.ebp - 0xc));
    fpu.pop();
    if (!s.below && !s.equal)
    {
        cpu.eax = cpu.ebp - 0x40;
        fpu.push(f32(app, cpu.ebp - 0xc));
        push(app, cpu, cpu.eax);
        fpu.st(0) = -fpu.st(0);
        cpu.edx = cpu.ebx;
        cpu.esp -= 4;
        cpu.eax = 1;
        putF32(app, cpu.esp, fpu.pop());
        if (!byAddress(app, cpu, kScaleVectors))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.eax + 0x98));
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x40));
        fpu.push(f32(app, cpu.eax + 0x9c));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.eax + 0x98, fpu.pop());
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x3c));
        fpu.push(f32(app, cpu.eax + 0xa0));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.eax + 0x9c, fpu.pop());
        fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.ebp - 0x38));
        putF32(app, cpu.eax + 0xa0, fpu.pop());
    }
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.ecx = cpu.ebp - 0x28;
    cpu.eax = 1;
    cpu.edx = cpu.esi;
    cpu.ebx += 0x98;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.eax = cpu.ebp - 0x1c;
    cpu.edx = at(app, cpu.ebp - 4);
    push(app, cpu, cpu.eax);
    cpu.edx += 0xe4;
    push(app, cpu, 0x40c90fd9);
    cpu.eax = 1;
    cpu.ebx = cpu.ebp - 0x4c;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ebp - 0x28;
    cpu.eax = cpu.ebp - 0x1c;
    cpu.ecx = cpu.ebp - 0x4c;
    if (!byAddress(app, cpu, kCross))
        return;
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.edx = cpu.ebp - 0x4c;
    cpu.eax = 1;
    cpu.ebx += 0xa4;
    if (!byAddress(app, cpu, kScaledSum))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.ebp - 0x4c));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x48));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x34, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x44));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x30, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    cpu.edx = at(app, cpu.ebp - 8);
    cpu.eax = cpu.ebp - 0x34;
    putF32(app, cpu.ebp - 0x2c, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(x86::Float(0.0));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x10, fpu.pop());
    s = compare(cpu, fpu.st(0), f32(app, cpu.ebp - 0x10));
    fpu.pop();
    if (s.below || s.equal)
    {
        cpu.eax = 0;
    }
    else
    {
        fpu.push(f32(app, cpu.ebp - 0x10));
        cpu.eax = cpu.ebp - 0x34;
        cpu.edx = at(app, cpu.ebp - 8);
        push(app, cpu, cpu.eax);
        fpu.st(0) = -fpu.st(0);
        cpu.esp -= 4;
        cpu.eax = 1;
        putF32(app, cpu.esp, fpu.pop());
        if (!byAddress(app, cpu, kScaleVectors))
            return;
        cpu.edx = at(app, cpu.ebp - 0x20);
        push(app, cpu, 0x3f800000);
        push(app, cpu, cpu.edx);
        cpu.ecx = at(app, cpu.ebp - 0x24);
        push(app, cpu, cpu.ecx);
        cpu.ebx = at(app, cpu.ebp - 0x28);
        push(app, cpu, cpu.ebx);
        cpu.edi = at(app, cpu.ebp - 0x2c);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.ebp - 0x30);
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, cpu.ebp - 0x34);
        push(app, cpu, cpu.edx);
        cpu.eax = at(app, cpu.ebp - 4);
        if (!direct(app, cpu, applyImpulse))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        putF32(app, cpu.eax + 0x168, fpu.st(0));
        fpu.push(x86::Float(0.0));
        s = compare(cpu, fpu.st(0), fpu.st(1));
        fpu.pop();
        fpu.pop();
        if (!s.equal)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.ecx = at(app, cpu.eax + 0x18c);
            put(app, cpu.eax + 0x16c, 0);
            cpu.ecx |= 0x30000;
            cpu.edi = cpu.eax + 0x178;
            put(app, cpu.eax + 0x170, cpu.ecx);
            moveDwords(app, cpu, 3);
            cpu.edx = at(app, cpu.ebp - 8);
            push(app, cpu, at(app, cpu.eax + 0x168));
            if (!direct(app, cpu, bounceOffWall))
                return;
        }
        cpu.eax = 1;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x42efe0: the car eax's box against a wall of normal edx through the
 * point ebx, within the distance [esp+4] (ret 4): its 8 corners -- 4 of its
 * bottom when it is on the ground (+0x164 0) -- taken into the world
 * (0x4e0430, by its axes +0xc0 and place +0x98), the one furthest into the
 * wall found; if it is nearer than the distance, the car hits the wall there
 * (hitWall).  The spin held (clampSpin).  What hitWall gave, or 0. */
void boxHitsWall(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x94;
    cpu.ebp -= 0x7e;
    put(app, cpu.ebp + 0x7a, cpu.eax);
    put(app, cpu.ebp + 0x66, cpu.edx);
    put(app, cpu.ebp + 0x76, cpu.ebx);
    cpu.edx = 0;
    put(app, cpu.ebp + 0x62, cpu.edx);
    if (half(app, cpu.eax + 0x164) != 0)
    {
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp - 0x16, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x118));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp - 0x12, fpu.pop());
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, cpu.ebp - 0xe, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp - 0xa, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        fpu.push(f32(app, cpu.eax + 0x118));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp - 6, fpu.pop());
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, cpu.ebp - 2, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp + 2, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        fpu.push(f32(app, cpu.eax + 0x118));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 6, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0xa, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0xe, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x118));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0x12, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0x16, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(0) = -fpu.st(0);
        fpu.push(f64(app, kThreeQuarters));
        std::swap(fpu.st(0), fpu.st(1));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x1a, fpu.pop());
        fpu.push(f64(app, kHalf));
        fpu.push(f32(app, cpu.eax + 0x118));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x1e, fpu.pop());
        fpu.push(f64(app, kFourFifths));
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x22, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(3) = fp::mul(fpu.st(3), fpu.st(0));
        fpu.pop();
        std::swap(fpu.st(0), fpu.st(2));
        putF32(app, cpu.ebp + 0x26, fpu.pop());
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0x118));
        putF32(app, cpu.ebp + 0x2a, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x2e, fpu.pop());
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp + 0x32, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax = at(app, cpu.eax + 0x118);
        put(app, cpu.ebp + 0x36, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        fpu.st(0) = fp::mul(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x3a, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0x3e, fpu.pop());
        cpu.eax = at(app, cpu.eax + 0x118);
        put(app, cpu.ebp + 0x42, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.ecx = 8;
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        fpu.st(1) = fp::mul(fpu.st(1), fpu.st(0));
        fpu.pop();
        put(app, cpu.ebp + 0x72, cpu.ecx);
        putF32(app, cpu.ebp + 0x46, fpu.pop());
    }
    else
    {
        fpu.push(f32(app, cpu.eax + 0x114));
        cpu.esi = 0;
        fpu.st(0) = -fpu.st(0);
        put(app, cpu.ebp - 0x12, cpu.esi);
        putF32(app, cpu.ebp - 0x16, fpu.pop());
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, cpu.ebp - 0xe, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, cpu.ebp - 0xa, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        put(app, cpu.ebp - 6, cpu.esi);
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, cpu.ebp - 2, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.eax = at(app, cpu.eax + 0x114);
        cpu.ebx = 4;
        put(app, cpu.ebp + 2, cpu.eax);
        cpu.eax = at(app, cpu.ebp + 0x7a);
        put(app, cpu.ebp + 6, cpu.esi);
        put(app, cpu.ebp + 0x72, cpu.ebx);
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0xa, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x114));
        fpu.st(0) = -fpu.st(0);
        put(app, cpu.ebp + 0x12, cpu.esi);
        putF32(app, cpu.ebp + 0xe, fpu.pop());
        fpu.push(f32(app, cpu.eax + 0x11c));
        fpu.st(0) = -fpu.st(0);
        putF32(app, cpu.ebp + 0x16, fpu.pop());
    }
    cpu.eax = cpu.ebp - 0x16;
    cpu.ecx = at(app, cpu.ebp + 0x7a);
    cpu.ebx = at(app, cpu.ebp + 0x7a);
    cpu.edx = cpu.ebp - 0x16;
    push(app, cpu, cpu.eax);
    cpu.ecx += 0x98;
    cpu.ebx += 0xc0;
    cpu.eax = at(app, cpu.ebp + 0x72);
    if (!byAddress(app, cpu, kTransformPoints))
        return;
    cpu.eax = at(app, cpu.ebp + 0x92);
    cpu.ecx = 0;
    put(app, cpu.ebp + 0x6e, cpu.eax);
    while (sreg32(cpu.ecx) < sreg32(at(app, cpu.ebp + 0x72)))
    {
        cpu.eax = cpu.ecx * 4;
        cpu.eax -= cpu.ecx;
        cpu.ebx = cpu.eax * 4;
        cpu.edx = at(app, cpu.ebp + 0x66);
        cpu.eax = at(app, cpu.ebp + 0x76);
        fpu.push(f32(app, cpu.ebx + cpu.ebp - 0x16));
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax));
        fpu.push(f32(app, cpu.ebx + cpu.ebp - 0x12));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x56, fpu.pop());
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 4));
        fpu.push(f32(app, cpu.ebx + cpu.ebp - 0xe));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp + 0x5a, fpu.pop());
        fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 8));
        cpu.eax = cpu.ebp + 0x56;
        putF32(app, cpu.ebp + 0x5e, fpu.pop());
        if (!byAddress(app, cpu, kDotProduct))
            return;
        putF32(app, cpu.ebp + 0x6a, fpu.st(0));
        s = compare(cpu, fpu.st(0), f32(app, cpu.ebp + 0x6e));
        fpu.pop();
        if (s.below)
        {
            cpu.eax = at(app, cpu.ebp + 0x6a);
            cpu.edi = cpu.ebp + 0x4a;
            cpu.esi = cpu.ebx + cpu.ebp - 0x16;
            put(app, cpu.ebp + 0x6e, cpu.eax);
            moveDwords(app, cpu, 3);
        }
        ++cpu.ecx;
    }
    fpu.push(f32(app, cpu.ebp + 0x6e));
    s = compare(cpu, fpu.st(0), f32(app, cpu.ebp + 0x92));
    fpu.pop();
    if (s.below)
    {
        cpu.ecx = at(app, cpu.ebp + 0x76);
        cpu.ebx = at(app, cpu.ebp + 0x66);
        cpu.edx = cpu.ebp + 0x4a;
        cpu.eax = at(app, cpu.ebp + 0x7a);
        if (!direct(app, cpu, hitWall))
            return;
        put(app, cpu.ebp + 0x62, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp + 0x7a);
    if (!direct(app, cpu, clampSpin))
        return;
    cpu.eax = at(app, cpu.ebp + 0x62);
    cpu.esp = cpu.ebp + 0x7e;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 8;
}

/* 0x42f280: the cars eax and edx in collision at the point ebx: each one's
 * point's speed (its velocity, and its spin -- a turn's 2 pi -- across the
 * point from its place); the two cars' masses (+0xb0) and the speeds against
 * each other; where they close, an impulse on each against the other
 * (applyImpulse, kept at +0x168) and, if there was one, the hit noted
 * (+0x170 0x50005, +0x16c the other car, the point at +0x178) and, if it
 * was more than 5, +0x164 counted up. */
void carsCollide(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Status s;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x7c;
    put(app, cpu.ebp - 4, cpu.eax);
    put(app, cpu.ebp - 8, cpu.edx);
    put(app, cpu.ebp - 0xc, cpu.ebx);
    cpu.ecx = cpu.ebp - 0x7c;
    cpu.ebx = cpu.eax + 0x98;
    cpu.edx = at(app, cpu.ebp - 0xc);
    cpu.eax = 1;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.ebx = at(app, cpu.ebp - 8);
    cpu.ecx = cpu.ebp - 0x70;
    cpu.edx = at(app, cpu.ebp - 0xc);
    cpu.eax = 1;
    cpu.ebx += 0x98;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.eax = cpu.ebp - 0x1c;
    cpu.edx = at(app, cpu.ebp - 4);
    push(app, cpu, cpu.eax);
    cpu.edx += 0xe4;
    push(app, cpu, 0x40c90fd9);
    cpu.eax = 1;
    cpu.ebx = cpu.ebp - 0x64;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ebp - 0x7c;
    cpu.eax = cpu.ebp - 0x1c;
    cpu.ecx = cpu.ebp - 0x64;
    if (!byAddress(app, cpu, kCross))
        return;
    cpu.ebx = at(app, cpu.ebp - 4);
    cpu.edx = cpu.ebp - 0x64;
    cpu.eax = 1;
    cpu.ebx += 0xa4;
    if (!byAddress(app, cpu, kScaledSum))
        return;
    cpu.eax = cpu.ebp - 0x58;
    cpu.edx = at(app, cpu.ebp - 8);
    push(app, cpu, cpu.eax);
    cpu.edx += 0xe4;
    push(app, cpu, 0x40c90fd9);
    cpu.eax = 1;
    cpu.ebx = cpu.ebp - 0x34;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ebp - 0x70;
    cpu.eax = cpu.ebp - 0x58;
    cpu.ecx = cpu.ebp - 0x34;
    if (!byAddress(app, cpu, kCross))
        return;
    cpu.ebx = at(app, cpu.ebp - 8);
    cpu.edx = cpu.ebp - 0x34;
    cpu.eax = 1;
    cpu.ebx += 0xa4;
    if (!byAddress(app, cpu, kScaledSum))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.push(f32(app, cpu.ebp - 0x64));
    fpu.push(f32(app, cpu.eax + 0xb0));
    cpu.eax = at(app, cpu.ebp - 8);
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x34));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::add(fpu.st(0), f32(app, cpu.eax + 0xb0));
    cpu.eax = at(app, cpu.ebp - 4);
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x60));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x30));
    fpu.push(x86::Float(1.0));
    std::swap(fpu.st(0), fpu.st(2));
    putF32(app, cpu.ebp - 0x4c, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    std::swap(fpu.st(0), fpu.st(1));
    fpu.st(2) = fp::div(fpu.st(0), fpu.st(2));
    fpu.pop();
    fpu.push(f32(app, cpu.ebp - 0x5c));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x2c));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x48, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x34));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x64));
    cpu.eax = at(app, cpu.ebp - 8);
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x44, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x30));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x60));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x28, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    fpu.push(f32(app, cpu.ebp - 0x2c));
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.ebp - 0x5c));
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x24, fpu.pop());
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
    putF32(app, cpu.ebp - 0x20, fpu.pop());
    fpu.push(f32(app, cpu.eax + 0x98));
    cpu.eax = at(app, cpu.ebp - 4);
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x98));
    cpu.eax = at(app, cpu.ebp - 8);
    putF32(app, cpu.ebp - 0x40, fpu.pop());
    fpu.push(f32(app, cpu.eax + 0x9c));
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = cpu.ebp - 0x28;
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0x9c));
    cpu.eax = at(app, cpu.ebp - 8);
    putF32(app, cpu.ebp - 0x3c, fpu.pop());
    fpu.push(f32(app, cpu.eax + 0xa0));
    cpu.eax = at(app, cpu.ebp - 4);
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, cpu.ebp - 0x10, fpu.pop());
    fpu.st(0) = fp::sub(fpu.st(0), f32(app, cpu.eax + 0xa0));
    cpu.eax = cpu.ebp - 0x40;
    putF32(app, cpu.ebp - 0x38, fpu.pop());
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(x86::Float(0.0));
    s = compare(cpu, fpu.st(0), fpu.st(1));
    fpu.pop();
    fpu.pop();
    if (!s.below && !s.equal)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        fpu.push(f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
        cpu.esp -= 4;
        putF32(app, cpu.esp, fpu.pop());
        cpu.edx = at(app, cpu.ebp - 0x74);
        push(app, cpu, cpu.edx);
        cpu.ecx = at(app, cpu.ebp - 0x78);
        push(app, cpu, cpu.ecx);
        cpu.ebx = at(app, cpu.ebp - 0x7c);
        push(app, cpu, cpu.ebx);
        cpu.esi = at(app, cpu.ebp - 0x20);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.ebp - 0x24);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.ebp - 0x28);
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        if (!direct(app, cpu, applyImpulse))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        putF32(app, cpu.eax + 0x168, fpu.st(0));
        fpu.push(x86::Float(0.0));
        s = compare(cpu, fpu.st(0), fpu.st(1));
        fpu.pop();
        fpu.pop();
        if (!s.equal)
        {
            cpu.eax = at(app, cpu.ebp - 4);
            cpu.edx = at(app, cpu.ebp - 8);
            put(app, cpu.eax + 0x170, 0x50005);
            cpu.esi = at(app, cpu.ebp - 0xc);
            cpu.edi = cpu.eax + 0x178;
            put(app, cpu.eax + 0x16c, cpu.edx);
            moveDwords(app, cpu, 3);
            fpu.push(f32(app, cpu.eax + 0x168));
            s = compare(cpu, fpu.st(0), f64(app, kFive));
            fpu.pop();
            if (!s.below && !s.equal)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                putHalf(app, cpu.eax + 0x164, half(app, cpu.eax + 0x164) + 1);
            }
        }
    }
    cpu.edx = cpu.ebp - 0x4c;
    cpu.eax = cpu.ebp - 0x40;
    if (!byAddress(app, cpu, kDotProduct))
        return;
    fpu.push(x86::Float(0.0));
    s = compare(cpu, fpu.st(0), fpu.st(1));
    fpu.pop();
    fpu.pop();
    if (s.below)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        fpu.push(f32(app, cpu.ebp - 0x10));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.eax + 0xb0));
        cpu.esp -= 4;
        putF32(app, cpu.esp, fpu.pop());
        cpu.edx = at(app, cpu.ebp - 0x68);
        push(app, cpu, cpu.edx);
        cpu.ecx = at(app, cpu.ebp - 0x6c);
        push(app, cpu, cpu.ecx);
        cpu.ebx = at(app, cpu.ebp - 0x70);
        push(app, cpu, cpu.ebx);
        cpu.esi = at(app, cpu.ebp - 0x44);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.ebp - 0x48);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.ebp - 0x4c);
        push(app, cpu, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 8);
        if (!direct(app, cpu, applyImpulse))
            return;
        cpu.eax = at(app, cpu.ebp - 8);
        putF32(app, cpu.eax + 0x168, fpu.st(0));
        fpu.push(x86::Float(0.0));
        s = compare(cpu, fpu.st(0), fpu.st(1));
        fpu.pop();
        fpu.pop();
        if (!s.equal)
        {
            cpu.eax = at(app, cpu.ebp - 8);
            cpu.edx = at(app, cpu.ebp - 4);
            put(app, cpu.eax + 0x170, 0x50005);
            cpu.esi = at(app, cpu.ebp - 0xc);
            cpu.edi = cpu.eax + 0x178;
            put(app, cpu.eax + 0x16c, cpu.edx);
            moveDwords(app, cpu, 3);
            fpu.push(f32(app, cpu.eax + 0x168));
            s = compare(cpu, fpu.st(0), f64(app, kFive));
            fpu.pop();
            if (!s.below && !s.equal)
            {
                cpu.eax = at(app, cpu.ebp - 8);
                putHalf(app, cpu.eax + 0x164, half(app, cpu.eax + 0x164) + 1);
            }
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace raceload
}  // namespace nfs3hp
