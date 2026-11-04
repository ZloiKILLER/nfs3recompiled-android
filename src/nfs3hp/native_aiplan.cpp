#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <utility>

/* The traffic and the opponents' lane driving, the track's animation paths
 * and the sound's start-up, native whole (0x40c4f0..0x40fcf0).
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (AIPLAN_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code (aiplan.inc).
 *
 *  - The traffic cars' driver (advanceCarAi, 0x40c9f0, each tick through a
 *    traffic car's +0x600 hook): a state machine on +0x6dc (0/1 placed and
 *    planned, 3 driving, 4 stopped beside the road, 2/5 idle, by the jump
 *    table at 0x40c9d4) over the car's own loading steps (stepTrafficLoading,
 *    +0x630).  The car's record as this code reads it: +0x1c its node, +0x200
 *    and +0x620/+0x621 its flags (+0x621 bit 4: it drives against the
 *    track's direction), +0x638/+0x63c/+0x640 the road slices its middle and
 *    sides are in, +0x6d8 the player car it keeps near, +0x7b0 its speed
 *    factor, +0x7b4 its target place across the road, +0x7b8 its place
 *    across, +0x7c0 its speed, +0x7c4 how far it looks ahead, +0x114 its
 *    half width, +0x21c its half width as a slice offset.
 *  - The lane planner: beginLanePlanning (0x40dd60) clears the costs at
 *    0x56ee90 and the sensed cars at 0x56ef38, finds the car's slices
 *    (carSlices), votes for its present lane (voteCurrentLane) and senses
 *    the cars near it (senseTraffic); chooseLane (0x40ddb0) scores the lanes
 *    by the road's shape, the parked obstacles (0x7dd850, 8 of 0x50 bytes)
 *    and the cars; sumLaneVotes picks the lane [0x56ef74] and steerToLane sets
 *    +0x7b4 to it.
 *  - The track's nodes (0x24 bytes at [0x5dd948], [0x5dd940] of them): +0xa's
 *    high half the 16 lane bits, +0x13 the edge slices (high nibble left, low
 *    right), +0x14/+0x16 the direction, +0x17/+0x1b the halves' widths,
 *    +0x1c/+0x20 the road's edges.  nodeGap, nodeOffset, laneBit,
 *    nodeLaneBits, nodeHasLane, roadSliceIndex and friends; the computer
 *    cars' speed table (targetSpeedAt, roadKindAt); the nine tracks' tuning
 *    records (0x20 bytes at 0x56ef80, setupTrackTuning and the readers after it).
 *  - The track's animation paths (.can) and their players (0x56f234), the
 *    objects and cameras that move along them.
 *  - The sound library brought up for the speaker setup (startSound,
 *    restartSound).
 *
 * The name `aiplan` fits the first two; kept for the merge.
 *
 * Written as the code is: each push and pop on the guest's stack, each call
 * with the return address's slot, the frames' locals in the guest's memory,
 * the registers as the code leaves them, the x87 through x86::FPU in the
 * code's operand order (its stack pushed and popped as the x87 does).  The
 * three jump tables (0x40c9d4, 0x40e400, 0x40ed14) go by the labels nfs3.exe's
 * tables hold.  Siblings are called as C++; every other function of the
 * game's by address. */
namespace nfs3hp
{

namespace aiplan
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
using x86::sreg16;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;
using i32 = std::int32_t;

// The cars: the list and its count, the per-type list, the players' cars.
constexpr reg32 kCars = 0x5efac8, kCarCount = 0x5efd9c, kCarCountB = 0x5efd8c;
constexpr reg32 kCarsByType = 0x5efb48, kPlayerCars = 0x5efa48, kCarsByPlace = 0x5efa88, kPlaceCount = 0x5efd88;
constexpr reg32 kAiCount = 0x5efda4, kPlayerCarsB = 0x56ebb8;
// The track's nodes and their count.
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The race: reversed, split screen, the track, the type, the variant.
constexpr reg32 kBackwards = 0x6fd4c0, kSplitScreen = 0x6fd4c8, kNetworkRace = 0x6fd4cc;
constexpr reg32 kTrackNumber = 0x6fd4ac, kRaceType = 0x6fd3b8, kRaceVariant = 0x6fd470, kSecondPlayer = 0x6fd4f0;
constexpr reg32 kLoadFlags = 0x6fd4fc, kWeatherNear = 0x6fd4c4, kClock = 0x7d3684;
// The lane planner's globals (0x56ee90..): three sets of lane costs, the
// chosen lane, the sensed cars, the near/far spans and the block flags.
constexpr reg32 kLaneCostA = 0x56ee90, kLaneCostB = 0x56ee94, kLaneCostC = 0x56ee98;
constexpr reg32 kLaneCostLo = 0x56ee9c, kLaneCostHi = 0x56eea4, kLaneCostObj = 0x56eea8, kLaneCostObjB = 0x56eeb0;
constexpr reg32 kNodeCostB = 0x56eeb4, kNodeCostC = 0x56eeb8, kNodeCostD = 0x56eebc, kNodeCostE = 0x56eec0;
constexpr reg32 kObjCost = 0x56eecc, kCarCostLeft = 0x56eed8, kCarCostRight = 0x56eee4;
constexpr reg32 kChosenLane = 0x56ef74;
constexpr reg32 kSensed = 0x56ef38, kSensedCount = 0x56ef48, kSensedCountB = 0x56ef4c, kSensedFar = 0x56ef44;
constexpr reg32 kSensedRight = 0x56ef50, kSensedSpanLo = 0x56ef5c;
constexpr reg32 kSensedFlag = 0x56ef7c, kSensedEdge = 0x56ef68, kSensedEdgeB = 0x56ef6e;
constexpr reg32 kSensedDir = 0x56ef78, kSensedAll = 0x56ef40;
// The lane-width table (80 floats), the obstacles (8 of 0x50 bytes).
constexpr reg32 kLaneWidths = 0x56f0e0, kObstacles = 0x7dd850;
// The track's speed table loaded, its half, the failure flag.
constexpr reg32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kSpeedsReady = 0x56f228;
constexpr reg32 kTrafficSide = 0x56f230, kTrackWalked = 0x56f220, kTrackValid = 0x56f224;
// The catch-up tables (0x56ef80.. per track-number slot of 0x20), the start.
constexpr reg32 kTuning = 0x56ef80, kStartFlags = 0x55eb2e;
// The animation players (0x18 bytes each) and the seven paths loaded.
constexpr reg32 kAnims = 0x56f234, kAnimPaths = 0x570a34;
// The sound config: effects on, music on, the options, the result flag.
constexpr reg32 kEffectsOn = 0x55290c, kMusicOn = 0x552910, kMusicSerial = 0x552918;
constexpr reg32 kSoundSurround = 0x6fbbdc, kSoundSetupKind = 0x6fbbe0, kSoundInitOK = 0x5525a4;
constexpr reg32 kSoundMode = 0x6fbc00, kVoiceRecords = 0x571124;
// Constants of the game's.
constexpr reg32 kVolume = 0x53684c, kTicks = 0x79c1a4;

// What is called by address: other chunks' AI functions (0x40c160/0x40c2f0/
// 0x40bfe0/0x40bff0/0x40c270, below this chunk, stood in for), the AI-votes
// trace hook (0x40de30, apply_native_vertices), and the game's/other parts'
// natives (stood in for where they would run on a made-up world).
constexpr reg32 kRetargetA = 0x40c160, kRetargetB = 0x40c2f0, kPrepareCar = 0x40bff0;
constexpr reg32 kStepPrepare = 0x40c270, kSpeedFloat = 0x40bfe0, kAiVotes = 0x40de30;
constexpr reg32 kCopChaseSet = 0x411750;
constexpr reg32 kResetDrive = 0x4aa79f, kPlaceOnRoad = 0x4a0aa0, kScaleAdd = 0x4e0050;
constexpr reg32 kMemset = 0x4e0640, kFreeHandle = 0x4e1890;
constexpr reg32 kActive = 0x401240, kRecoverA = 0x405cd0, kRecoverB = 0x405d40, kRecoverC = 0x405dc0, kRecoverD = 0x405e20;
constexpr reg32 kChop = 0x4dfd56;
constexpr reg32 kPoseBlend = 0x4b70f0, kPoseMatrix = 0x4b7590, kPlaceBlend = 0x4b6a30, kQuatMatrix = 0x4b6ca0;
constexpr reg32 kSndInit = 0x4e2630, kSndCaps = 0x4e21b0, kSndRate = 0x4e2228, kBusInit = 0x418740;
constexpr reg32 kVolumeSet = 0x42d510;

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

/* A call to a sibling of this file by its C++ function (the return address's
 * slot, esp as the callee leaves it); false when the game stopped in it. */
bool direct(App* app, CPU& cpu, void (*function)(App*, CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

/* A call to anything else by address. */
bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* A loop's safepoint, as the generated code has one at each loop head. */
void safepoint(App* app, CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

/* movsd: a dword from [esi] to [edi], both moved on (the direction flag is
 * clear everywhere here). */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
}

// The x87, as the code has it.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

void fld(CPU& cpu, const Float& value) { cpu.fpu.push(value); }
void fld32(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(f32(app, a)); }
void fld64(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(f64(app, a)); }
void fild(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(Float(sreg32(at(app, a)))); }
void fild16(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(Float(sreg16(app->getMemory<reg16>(a)))); }
void fldz(CPU& cpu) { cpu.fpu.push(Float(0.0f)); }
void fld1(CPU& cpu) { cpu.fpu.push(Float(1.0f)); }

void fst32(App* app, CPU& cpu, reg32 a) { app->getMemory<float>(a) = float(cpu.fpu.st(0)); }
void fst64(App* app, CPU& cpu, reg32 a) { app->getMemory<double>(a) = double(cpu.fpu.st(0)); }
void fstp32(App* app, CPU& cpu, reg32 a) { fst32(app, cpu, a); cpu.fpu.pop(); }
void fstp64(App* app, CPU& cpu, reg32 a) { fst64(app, cpu, a); cpu.fpu.pop(); }

void fxch(CPU& cpu, int i)
{
    const Float t = cpu.fpu.st(0);
    cpu.fpu.st(0) = cpu.fpu.st(i);
    cpu.fpu.st(i) = t;
}

void fchs(CPU& cpu) { cpu.fpu.st(0) = -cpu.fpu.st(0); }

// st(0) op= m.
void fadd(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), m); }
void fsub(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), m); }
void fsubr(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(m, Float(cpu.fpu.st(0))); }
void fmul(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), m); }
void fdiv(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::div(cpu.fpu.st(0), m); }

// st(0) op= st(i); st(i) op= st(0) (no pop).
void fmulSt0(CPU& cpu, int i) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), Float(cpu.fpu.st(i))); }
void fmulSti(CPU& cpu, int i) { cpu.fpu.st(i) = fp::mul(cpu.fpu.st(i), Float(cpu.fpu.st(0))); }

// st(i) op= st(0), popped.
void faddp(CPU& cpu, int i) { cpu.fpu.st(i) = fp::add(cpu.fpu.st(i), cpu.fpu.st(0)); cpu.fpu.pop(); }
void fmulp(CPU& cpu, int i) { cpu.fpu.st(i) = fp::mul(cpu.fpu.st(i), cpu.fpu.st(0)); cpu.fpu.pop(); }
void fdivp(CPU& cpu, int i) { cpu.fpu.st(i) = fp::div(cpu.fpu.st(i), cpu.fpu.st(0)); cpu.fpu.pop(); }
void fsubrp(CPU& cpu, int i) { cpu.fpu.st(i) = fp::sub(cpu.fpu.st(0), Float(cpu.fpu.st(i))); cpu.fpu.pop(); }

/* fcom(p); fnstsw ax; sahf: the x87 status word into ax, CF (C0, below) and
 * ZF (C3, equal) as sahf leaves them. */
struct Flags
{
    bool cf, zf;
    bool below() const { return cf; }
    bool belowOrEqual() const { return cf || zf; }
    bool above() const { return !cf && !zf; }
    bool aboveOrEqual() const { return !cf; }
    bool equal() const { return zf; }
};

Flags readStatus(CPU& cpu)
{
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

Flags fcomp(CPU& cpu, const Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    const Flags f = readStatus(cpu);
    cpu.fpu.pop();
    return f;
}

Flags fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), Float(cpu.fpu.st(1)));
    const Flags f = readStatus(cpu);
    cpu.fpu.pop();
    cpu.fpu.pop();
    return f;
}

/* sub esp, 4; fistp dword [esp]; pop reg: st(0) rounded to an integer. */
reg32 fistpPop(App* app, CPU& cpu)
{
    const reg32 v = reg32(fp::toInteger<sreg32>(cpu.fpu.st(0)));
    cpu.fpu.pop();
    return v;
}

/* The code's cdq; idiv by: edx:eax / by signed, as x86 does. */
void idiv(CPU& cpu, reg32 by)
{
    const std::int64_t n = std::int64_t(sreg32(cpu.eax));
    cpu.eax = reg32(n / i32(by));
    cpu.edx = reg32(n % i32(by));
}

reg32 nodeAddr(App* app, reg32 index)
{
    // lea eax,[index*8]; add eax,index; shl eax,2; add [0x5dd948]: index*0x24.
    return at(app, kNodes) + index * 0x24;
}

}  // namespace

// Forward declarations, in address order (address -> name).
void leastTrafficPlayer(App* app, CPU& cpu);  // 0x40c4f0
void placeTrafficCar(App* app, CPU& cpu);           // 0x40c590
void reactToTraffic(App* app, CPU& cpu);       // 0x40c630
void slowForPolice(App* app, CPU& cpu);             // 0x40c780
void countStopped(App* app, CPU& cpu);       // 0x40c7f0
void nearPoliceCar(App* app, CPU& cpu);      // 0x40c830
void weighLaneVotes(App* app, CPU& cpu);        // 0x40c8a0
void voteRoadside(App* app, CPU& cpu);        // 0x40c8d0
void atRoadside(App* app, CPU& cpu);        // 0x40c950
void advanceCarAi(App* app, CPU& cpu);          // 0x40c9f0
void clearSensedCars(App* app, CPU& cpu);       // 0x40cb20
void cacheTrafficSide(App* app, CPU& cpu);       // 0x40cb50
void resetTrafficCar(App* app, CPU& cpu);              // 0x40cb70
void clearTrafficRecord(App* app, CPU& cpu);        // 0x40cb90
void placeOnTrack(App* app, CPU& cpu);        // 0x40cd80
void brakeToStop(App* app, CPU& cpu);       // 0x40ce70
void voteCurrentLane(App* app, CPU& cpu);       // 0x40cee0
void senseTraffic(App* app, CPU& cpu);          // 0x40cfc0
void planLaneChange(App* app, CPU& cpu);        // 0x40d1f0
void edgeLaneCost(App* app, CPU& cpu);          // 0x40d2e0
void scoreLanesByRoad(App* app, CPU& cpu);      // 0x40d3d0
void scoreLanesByObstacles(App* app, CPU& cpu); // 0x40d720
void scoreLanesByCars(App* app, CPU& cpu);      // 0x40d800
void edgePenalty(App* app, CPU& cpu);           // 0x40d9f0
void sumLaneVotes(App* app, CPU& cpu);          // 0x40da30
void steerToLane(App* app, CPU& cpu);           // 0x40daf0
void beginLanePlanning(App* app, CPU& cpu);     // 0x40dd60
void chooseLane(App* app, CPU& cpu);            // 0x40ddb0
void startHorn(App* app, CPU& cpu);         // 0x40de50
void stopHorn(App* app, CPU& cpu);        // 0x40deb0
void stepTrafficLoading(App* app, CPU& cpu);         // 0x40df10
void targetSpeedAt(App* app, CPU& cpu);         // 0x40e080
void roadKindAt(App* app, CPU& cpu);         // 0x40e0c0
void cacheRoadWidth(App* app, CPU& cpu);        // 0x40e0f0
void nodeGap(App* app, CPU& cpu);               // 0x40e120
void nodeOffset(App* app, CPU& cpu);            // 0x40e160
void nodesAheadBySpeed(App* app, CPU& cpu);     // 0x40e1b0
void nodeLaneBits(App* app, CPU& cpu);               // 0x40e200
void nodeHasLane(App* app, CPU& cpu);      // 0x40e220
void laneBit(App* app, CPU& cpu);        // 0x40e270
void roadSliceIndex(App* app, CPU& cpu);        // 0x40e2b0
void carSlices(App* app, CPU& cpu);            // 0x40e350
void fillTrackTuning(App* app, CPU& cpu);      // 0x40e3c0
void setupTrackTuning(App* app, CPU& cpu);          // 0x40e410
void freeSpeedTable(App* app, CPU& cpu);        // 0x40ec40
void trafficSide(App* app, CPU& cpu);       // 0x40ec70
void roadIsWide(App* app, CPU& cpu);            // 0x40eca0
void trafficSpeedAt(App* app, CPU& cpu);    // 0x40ed40
void tuningSpeed(App* app, CPU& cpu);         // 0x40ee80
void tuningDistance(App* app, CPU& cpu);           // 0x40eed0
void tuningFirst(App* app, CPU& cpu);          // 0x40ef10
void nodeDirectionDot(App* app, CPU& cpu);      // 0x40ef70
void nodeWithinRange(App* app, CPU& cpu);       // 0x40f000
void carWithinReach(App* app, CPU& cpu);        // 0x40f050
void alwaysOne(App* app, CPU& cpu);             // 0x40f100
void carBoxedIn(App* app, CPU& cpu);       // 0x40f110
void freeAnimationPaths(App* app, CPU& cpu);       // 0x40f330
void findFreeAnimation(App* app, CPU& cpu);          // 0x40f370
void startAnimation(App* app, CPU& cpu);               // 0x40f3b0
void setAnimationMode(App* app, CPU& cpu);         // 0x40f430
void stopAnimation(App* app, CPU& cpu);            // 0x40f460
void animationPose(App* app, CPU& cpu);       // 0x40f4b0
void animationPlace(App* app, CPU& cpu);        // 0x40f6d0
void animationProgress(App* app, CPU& cpu);      // 0x40f800
void animationLastPose(App* app, CPU& cpu);     // 0x40f880
void pathEndPose(App* app, CPU& cpu);            // 0x40f8d0
void animationStep(App* app, CPU& cpu);              // 0x40f940
void animationPlaying(App* app, CPU& cpu);            // 0x40fac0
void startSound(App* app, CPU& cpu);      // 0x40fb10
void restartSound(App* app, CPU& cpu);    // 0x40fcf0

/* 0x40c4f0: the player car fewest traffic cars keep near: for each of the
 * [0x5efda4] players' cars (0x5efa48) the traffic cars (0x5efb48, active:
 * +0x620 bit 2 clear) whose +0x6d8 is it, counted in the frame; the first
 * with the least (from 10000; [0x5efac8] when there are none).  Keeps every
 * register but eax. */
void leastTrafficPlayer(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x3c;
    cpu.edi = at(app, kAiCount);
    cpu.ebx = at(app, kCars);
    cpu.eax = 0;
    while (sreg32(cpu.eax) < sreg32(cpu.edi))
    {
        ++cpu.eax;
        cpu.edx = 0;
        put(app, cpu.ebp + cpu.eax * 4 - 0x40, cpu.edx);
    }
    cpu.eax = 0;
    for (; sreg32(cpu.eax) < sreg32(cpu.edi); ++cpu.eax)
    {
        cpu.edx = 0;
        for (; sreg32(cpu.edx) < sreg32(at(app, kCarCountB)); ++cpu.edx)
        {
            cpu.esi = at(app, cpu.edx * 4 + kCarsByType);
            if ((byte(app, cpu.esi + 0x620) & 4) != 0)
                continue;
            cpu.ecx = cpu.eax * 4;
            cpu.esi = at(app, cpu.esi + 0x6d8);
            if (cpu.esi != at(app, cpu.ecx + kPlayerCars))
                continue;
            put(app, cpu.ecx + cpu.ebp - 0x3c, at(app, cpu.ecx + cpu.ebp - 0x3c) + 1);
        }
    }
    cpu.ecx = 0x2710;
    cpu.eax = 0;
    for (; sreg32(cpu.eax) < sreg32(cpu.edi); ++cpu.eax)
    {
        cpu.edx = cpu.eax * 4;
        cpu.esi = at(app, cpu.edx + cpu.ebp - 0x3c);
        if (sreg32(cpu.ecx) <= sreg32(cpu.esi))
            continue;
        cpu.ecx = cpu.esi;
        cpu.ebx = at(app, cpu.edx + kPlayerCars);
    }
    cpu.eax = cpu.ebx;
    put(app, kAiCount, cpu.edi);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c590: the traffic car eax set going: the game's random numbers
 * (0x554f10 by 0x554f18, 0..99 from its middle bytes) make it drive against
 * the track's direction (+0x621 bit 4) seven times in ten, never where
 * [0x56f228] (roadIsWide) is set; the player car it keeps near (+0x6d8: leastTrafficPlayer,
 * or [0x56ebb8] with one player); then 0x40c160 and 0x40c2f0 (below this
 * chunk) place it. */
void placeTrafficCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, 0x554f18);
    cpu.eax = at(app, 0x554f10);
    cpu.eax = reg32(cpu.eax * cpu.ecx);
    put(app, 0x554f14, cpu.eax);
    cpu.ecx = at(app, 0x554f14);
    cpu.ecx &= 0xffff00;
    cpu.eax &= 0xffff;
    cpu.ecx >>= 8;
    put(app, 0x554f10, cpu.eax);
    cpu.eax = cpu.ecx * 4;
    cpu.eax -= cpu.ecx;
    cpu.eax <<= 3;
    cpu.eax += cpu.ecx;
    cpu.eax <<= 2;
    cpu.eax >>= 0x10;
    const bool setBit = cpu.eax <= 0x45 && at(app, kSpeedsReady) == 0;
    if (setBit)
        putByte(app, cpu.edx + 0x621, reg8(byte(app, cpu.edx + 0x621) | 0x10));
    else
        putByte(app, cpu.edx + 0x621, reg8(byte(app, cpu.edx + 0x621) & 0xef));
    if (sreg32(at(app, kAiCount)) <= 1)
        cpu.eax = at(app, kPlayerCarsB);
    else if (!direct(app, cpu, leastTrafficPlayer))
        return;
    put(app, cpu.edx + 0x6d8, cpu.eax);
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kRetargetA))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kRetargetB))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c630: the traffic car eax reacts to the cars about it.  While its horn
 * sounds (+0x620 bit 3) it only waits for it to end (stopHorn).  A car driving
 * against the track's direction, with a car straight ahead in its lane
 * ([0x56ef3c], [0x56ef48] 7) coming the other way that is a player's (+0x200
 * bit 2): the horn when they close faster than 20, and faster than 23 a
 * police chase called (0x411750: 8, 100, 1).  Then, when a car about it
 * drives against it [0x56ef7c], the horn for any player's car ahead. */
void reactToTraffic(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.esi = cpu.eax;
    const reg32 m8 = cpu.ebp - 8, mc = cpu.ebp - 0xc, m4 = cpu.ebp - 4;
    if ((byte(app, cpu.eax + 0x620) & 8) != 0)
    {
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, stopHorn))
            return;
        goto done;
    }
    if ((byte(app, cpu.esi + 0x621) & 0x10) == 0)
        goto done;
    cpu.edx = at(app, 0x56ef3c);
    if (cpu.edx == 0)
        goto sensed;
    if (at(app, kSensedCount) != 7)
        goto sensed;
    {
        fld32(app, cpu, cpu.esi + 0x7c0);
        fldz(cpu);
        const Flags a = fcompp(cpu);
        cpu.ecx = a.belowOrEqual() ? 0 : 1;
        fld32(app, cpu, cpu.edx + 0x7c0);
        fldz(cpu);
        const Flags b = fcompp(cpu);
        cpu.eax = b.belowOrEqual() ? 0 : 1;
        cpu.eax ^= cpu.ecx;
        if (cpu.eax == 0)
            goto sensed;
        if ((byte(app, cpu.edx + 0x200) & 4) == 0)
            goto sensed;
        // |speed(esi) - speed(edx)| into [ebp-0xc].
        fld32(app, cpu, cpu.esi + 0x7c0);
        fsub(cpu, f32(app, cpu.edx + 0x7c0));
        fldz(cpu);
        fxch(cpu, 1);
        fstp32(app, cpu, m8);
        const Flags d = fcomp(cpu, f32(app, m8));
        if (!d.aboveOrEqual())
        {
            cpu.eax = at(app, m8);
            put(app, mc, cpu.eax);
        }
        else
        {
            fld32(app, cpu, m8);
            fchs(cpu);
            fstp32(app, cpu, mc);
        }
        fld32(app, cpu, mc);
        const Flags e = fcomp(cpu, f64(app, 0x5366e4));  // 20.0
        if (!e.belowOrEqual())
        {
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, startHorn))
                return;
        }
        fldz(cpu);
        const Flags f = fcomp(cpu, f32(app, m8));
        if (!f.aboveOrEqual())
        {
            cpu.eax = at(app, m8);
            put(app, m4, cpu.eax);
        }
        else
        {
            fld32(app, cpu, m8);
            fchs(cpu);
            fstp32(app, cpu, m4);
        }
        fld32(app, cpu, m4);
        const Flags g = fcomp(cpu, f64(app, 0x5366ec));  // 23.0
        if (g.belowOrEqual())
            goto sensed;
        cpu.ecx = 8;
        cpu.ebx = 0x64;
        cpu.edx = 1;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kCopChaseSet))
            return;
    }
sensed:
    if (at(app, kSensedFlag) == 0)
        goto done;
    cpu.eax = 0;
    while (true)
    {
        cpu.edx = at(app, cpu.eax * 4 + kSensed);
        if (cpu.edx != 0 && (byte(app, cpu.edx + 0x200) & 4) != 0)
        {
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, startHorn))
                return;
            cpu.esp = cpu.ebp;
            cpu.ebp = pop(app, cpu);
            cpu.esi = pop(app, cpu);
            cpu.edx = pop(app, cpu);
            cpu.ecx = pop(app, cpu);
            cpu.ebx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
        ++cpu.eax;
        if (cpu.eax >= 3)
            break;
    }
done:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c780: the traffic car eax slowed to pull over: its speed +0x7b0 6.7,
 * or 0x40bfe0's (40) if that is less; its sign flipped for a reversed race
 * [0x6fd4c0] and for a car against the track's direction. */
void slowForPolice(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 m4 = cpu.ebp - 4;
    cpu.edx = cpu.eax;
    put(app, cpu.eax + 0x7b0, 0x40d6930c);
    if (!byAddress(app, cpu, kSpeedFloat))  // leaves a double on the x87 stack
        return;
    fstp32(app, cpu, m4);
    fld32(app, cpu, cpu.edx + 0x7b0);
    const Flags f = fcomp(cpu, f32(app, m4));
    if (!f.belowOrEqual())
    {
        cpu.eax = at(app, m4);
        put(app, cpu.edx + 0x7b0, cpu.eax);
    }
    if (at(app, kBackwards) != 0)
    {
        cpu.ah = reg8(byte(app, cpu.edx + 0x7b3) ^ 0x80);
        putByte(app, cpu.edx + 0x7b3, cpu.ah);
    }
    if ((byte(app, cpu.edx + 0x621) & 0x10) != 0)
    {
        cpu.bh = reg8(byte(app, cpu.edx + 0x7b3) ^ 0x80);
        putByte(app, cpu.edx + 0x7b3, cpu.bh);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c7f0: how many active traffic cars have pulled over (state 4). */
void countStopped(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 0;
    cpu.edx = 0;
    for (; sreg32(cpu.eax) < sreg32(at(app, kCarCountB)); ++cpu.eax)
    {
        cpu.ecx = at(app, cpu.eax * 4 + kCarsByType);
        if ((byte(app, cpu.ecx + 0x620) & 4) != 0)
            continue;
        if (at(app, cpu.ecx + 0x6dc) != 4)
            continue;
        ++cpu.edx;
    }
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c830: whether a police car with its siren on (0x5efa88's, [0x5efd88]
 * of them: +0x734 chasing or +0x710 bit 4) is within edx + 0x16 nodes of the
 * car eax -- two more for each car already pulled over (countStopped). */
void nearPoliceCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 m4 = cpu.ebp - 4;
    cpu.ebx = cpu.edx;
    cpu.esi = at(app, cpu.eax + 0x1c);
    if (!direct(app, cpu, countStopped))
        return;
    cpu.eax += cpu.eax;
    cpu.ecx = 0;
    put(app, m4, cpu.eax);
    for (; sreg32(cpu.ecx) < sreg32(at(app, kPlaceCount)); ++cpu.ecx)
    {
        cpu.eax = at(app, cpu.ecx * 4 + kCarsByPlace);
        if (at(app, cpu.eax + 0x734) == 0 && (byte(app, cpu.eax + 0x710) & 0x10) == 0)
            continue;
        cpu.edx = cpu.esi;
        cpu.eax = at(app, cpu.eax + 0x1c);
        if (!direct(app, cpu, nodeGap))
            return;
        if (sreg32(cpu.eax) <= 0)
            cpu.eax = reg32(0) - cpu.eax;
        cpu.edi = at(app, m4);
        cpu.edx = cpu.ebx + 0x16;
        cpu.edx += cpu.edi;
        if (sreg32(cpu.eax) >= sreg32(cpu.edx))
            continue;
        cpu.eax = 1;
        goto leave;
    }
    cpu.eax = 0;
leave:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c8a0: the three lane votes (0x56ee90..98) weighed ten times. */
void weighLaneVotes(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    for (; sreg32(cpu.edx) < 3; )
    {
        cpu.ecx = at(app, cpu.edx * 4 + kLaneCostA);
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        ++cpu.edx;
        cpu.eax += cpu.eax;
        put(app, cpu.edx * 4 + 0x56ee8c, cpu.eax);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c8d0: a vote (+100) for pulling over: for the middle (0x56eeb8) when
 * the car eax's slice +0x638 is off its node's road, else for the nearer
 * roadside -- the left (0x56eeb4) or the right (0x56eebc). */
void voteRoadside(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.eax + 0x13);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 4);
    cpu.ebx = 6;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    cpu.ecx = cpu.ebx;
    cpu.ebx = at(app, cpu.edx + 0x638);
    bool mid = false;
    if (sreg32(cpu.ecx) > sreg32(cpu.ebx))
        mid = true;
    else
    {
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax += 7;
        if (sreg32(cpu.eax) < sreg32(cpu.ebx))
            mid = true;
    }
    if (mid)
        put(app, kNodeCostC, at(app, kNodeCostC) + 0x64);
    else if (sreg32(cpu.ebx) >= 7)
        put(app, kNodeCostD, at(app, kNodeCostD) + 0x64);
    else
        put(app, kNodeCostB, at(app, kNodeCostB) + 0x64);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c950: whether the car eax has reached the roadside: its place across
 * (+0x7b8, as 16.16) beyond its node's left edge (0x10000 less the left edge
 * slices times the left half's width) or right edge; 1, else 0. */
void atRoadside(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    fld32(app, cpu, cpu.eax + 0x7b8);
    fmul(cpu, f64(app, 0x5366f4));  // 65536.0
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = fistpPop(app, cpu);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = 0;
    cpu.ebx = 0;
    cpu.cl = byte(app, cpu.eax + 0x13);
    cpu.bl = byte(app, cpu.eax + 0x17);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 4);
    cpu.ebx <<= 0xf;
    cpu.ecx &= 0xf;
    cpu.ecx = reg32(cpu.ecx * cpu.ebx);
    cpu.ecx = reg32(0) - cpu.ecx;
    cpu.ecx += 0x10000;
    bool outside = false;
    if (sreg32(cpu.edx) < sreg32(cpu.ecx))
        outside = true;
    else
    {
        cpu.ecx = 0;
        cpu.cl = byte(app, cpu.eax + 0x1b);
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.ecx <<= 0xf;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax = reg32(cpu.eax * cpu.ecx);
        cpu.eax -= 0x10000;
        if (sreg32(cpu.edx) > sreg32(cpu.eax))
            outside = true;
    }
    cpu.eax = outside ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40c9f0: the traffic car eax's driver, each tick.  Its loading steps
 * first (stepTrafficLoading; 0: not on the road).  Then by +0x6dc (the jump
 * table at 0x40c9d4): 0 (and past 5) made 1; 1 drives -- 0x40c270, the lane
 * planned and chosen (beginLanePlanning, weighLaneVotes, reactToTraffic,
 * 0x40bff0, chooseLane), the lane voted for (0x40de30) -- and pulls over (3)
 * when a police car is near; 3 slows to the roadside (slowForPolice,
 * voteRoadside and the other scores, 0x40de30 with 2) -- back to 1 when the
 * police car has gone, to 4 at the roadside; 4 stands (its speed 0,
 * brakeToStop) until the police car has gone (1); 2 and 5 nothing. */
void advanceCarAi(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (!direct(app, cpu, stepTrafficLoading))
        return;
    if (cpu.eax == 0)
        goto ret;
    cpu.eax = at(app, cpu.ecx + 0x6dc);
    if (cpu.eax > 5)
        goto prepare;
    switch (cpu.eax)
    {
    case 0: goto prepare;
    case 1: goto plan;
    case 3: goto drive;
    case 4: goto recover;
    default: goto ret;  // 2 and 5
    }
prepare:
    put(app, cpu.ecx + 0x6dc, 1);
plan:
    cpu.eax = cpu.ecx; if (!byAddress(app, cpu, kStepPrepare)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, beginLanePlanning)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, weighLaneVotes)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, reactToTraffic)) return;
    cpu.eax = cpu.ecx; if (!byAddress(app, cpu, kPrepareCar)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, chooseLane)) return;
    cpu.eax = cpu.ecx; cpu.edx = 0; if (!byAddress(app, cpu, kAiVotes)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, nearPoliceCar)) return;
    if (cpu.eax == 0)
        goto ret;
    put(app, cpu.ecx + 0x6dc, 3);
    goto ret;
drive:
    cpu.eax = cpu.ecx; if (!direct(app, cpu, beginLanePlanning)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, slowForPolice)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, voteRoadside)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, scoreLanesByObstacles)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, scoreLanesByCars)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, edgePenalty)) return;
    cpu.eax = cpu.ecx; cpu.edx = 2; if (!byAddress(app, cpu, kAiVotes)) return;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, nearPoliceCar)) return;
    if (cpu.eax == 0)
        goto resetState;
    cpu.eax = cpu.ecx; if (!direct(app, cpu, atRoadside)) return;
    if (cpu.eax == 0)
        goto ret;
    put(app, cpu.ecx + 0x6dc, 4);
    goto ret;
recover:
    cpu.edx = 2;
    cpu.eax = cpu.ecx;
    put(app, cpu.ecx + 0x7b0, 0);
    if (!direct(app, cpu, brakeToStop))
        return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, nearPoliceCar))
        return;
    if (cpu.eax != 0)
        goto ret;
resetState:
    put(app, cpu.ecx + 0x6dc, 1);
ret:
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cb20: the sensed cars (0x48 bytes at 0x56ef38) cleared (memset) and
 * [0x56ebc0] zeroed. */
void clearSensedCars(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = 0x48;
    cpu.eax = kSensed;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset))
        return;
    cpu.edx = 0;
    put(app, 0x56ebc0, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cb50: the track's traffic side [0x56f230] (trafficSide) kept and
 * [0x56ebc0] zeroed. */
void cacheTrafficSide(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, trafficSide))
        return;
    cpu.edx = 0;
    put(app, kTrafficSide, cpu.eax);
    put(app, 0x56ebc0, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cb70: the traffic car eax reset: its record (clearTrafficRecord),
 * then 0x404690 (below this chunk). */
void resetTrafficCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, clearTrafficRecord))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, 0x404690))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cb90: the traffic car eax's driving state cleared: states, targets,
 * lanes and speeds zeroed, +0x7bc 1.0, its next decision +0x634 the clock
 * [0x7d3684] + 0x500; +0x620 bit 1 for a car of the kinds +0x200 bit 4 or 5,
 * +0x200 bit 1 cleared for a player's car (bit 2). */
void clearTrafficRecord(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (const reg32 off : {0x630u, 0x6d8u, 0x64cu, 0x650u, 0x654u, 0x658u, 0x65cu, 0x6dcu, 0x6e0u, 0x6e4u, 0x6e8u,
                            0x2c8u, 0x6f0u, 0x704u, 0x708u, 0x70cu, 0x710u, 0x714u, 0x718u, 0x730u, 0x71cu, 0x720u,
                            0x724u, 0x734u, 0x7acu, 0x7b4u, 0x7b0u, 0x7c0u, 0x7c4u})
        put(app, cpu.eax + off, 0);
    put(app, cpu.eax + 0x7bc, 0x3f800000);
    put(app, cpu.eax + 0x7b8, 0);
    put(app, cpu.eax + 0x21c, 0);
    put(app, cpu.eax + 0x7c8, 0);
    put(app, cpu.eax + 0x624, 0);
    put(app, cpu.eax + 0x628, 0);
    cpu.edx = at(app, kClock);
    put(app, cpu.eax + 0x7d4, 0);
    cpu.edx += 0x500;
    put(app, cpu.eax + 0x634, cpu.edx);
    cpu.dl = byte(app, cpu.eax + 0x710);
    put(app, cpu.eax + 0x62c, 0);
    cpu.dl &= 0xef;
    putByte(app, cpu.eax + 0x710, cpu.dl);
    put(app, cpu.eax + 0x620, 0);
    cpu.dh = byte(app, cpu.eax + 0x200);
    put(app, cpu.eax + 0x7d0, 0);
    if ((cpu.dh & 0x10) != 0)
        putByte(app, cpu.eax + 0x620, reg8(byte(app, cpu.eax + 0x620) | 2));
    if ((byte(app, cpu.eax + 0x200) & 0x20) != 0)
        putByte(app, cpu.eax + 0x620, reg8(byte(app, cpu.eax + 0x620) | 2));
    cpu.ch = byte(app, cpu.eax + 0x200);
    if ((cpu.ch & 4) != 0)
    {
        cpu.dl = cpu.ch;
        cpu.dl &= 0xfd;
        putByte(app, cpu.eax + 0x200, cpu.dl);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cd80: the car eax put on the track at its node +0x1c and its target
 * place across +0x7b4 (as 16.16; 0x4a0aa0, its direction by the reversed race
 * and +0x621 bit 4), its velocity +0xa4 its forward vector +0xd8 times its
 * look-ahead +0x7c4 (0x4e0050), its spin and slips zeroed.  The place's other
 * two parts from 0x40cb00. */
void placeOnTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.edi = cpu.ebp - 0x10;
    cpu.esi = 0x40cb00;
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    if ((byte(app, cpu.eax + 0x621) & 0x10) != 0)
        cpu.eax = 1;
    else
        cpu.eax = 0;
    cpu.ecx = at(app, kBackwards);
    cpu.ecx ^= cpu.eax;
    cpu.eax = at(app, cpu.ebp - 4);
    fld32(app, cpu, cpu.eax + 0x7b4);
    fmul(cpu, f64(app, 0x5366fc));  // 65536.0
    cpu.edx = 0x10000;
    cpu.eax = fistpPop(app, cpu);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.ebp - 0xc, cpu.edx);
    cpu.ebx = cpu.ebp - 0x10;
    cpu.edx = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kPlaceOnRoad))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.esi = cpu.eax + 0xd8;
    cpu.eax = cpu.ebp - 0x1c;
    cpu.edi = cpu.ebp - 0x1c;
    push(app, cpu, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 4);
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    cpu.edx = cpu.ebp - 0x1c;
    push(app, cpu, at(app, cpu.eax + 0x7c4));
    cpu.eax = 1;
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.esi = cpu.ebp - 0x1c;
    cpu.edi = cpu.eax + 0xa4;
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    put(app, cpu.eax + 0xe4, 0);
    put(app, cpu.eax + 0xe8, 0);
    put(app, cpu.eax + 0xec, 0);
    app->getMemory<reg16>(cpu.eax + 0x164) = 0;
    put(app, cpu.eax + 0x5c0, 0);
    put(app, cpu.eax + 0x5c4, 0);
    put(app, cpu.eax + 0x64c, 0);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ce70: the pulled-over car eax brought to a stop: once on the ground
 * (+0x164 0) and well on the road's grip (+0x10c above 0.9), its drive reset
 * (0x4aa79f), its spin +0xe4 and velocity +0xa4 halved (0x4e0050), +0x509
 * 0xff. */
void brakeToStop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (app->getMemory<reg16>(cpu.eax + 0x164) != 0)
        goto done;
    {
        fld32(app, cpu, cpu.eax + 0x10c);
        const Flags f = fcomp(cpu, f64(app, 0x536704));  // 0.9
        if (f.belowOrEqual())
            goto done;
    }
    cpu.eax = cpu.ecx;
    put(app, cpu.ecx + 0x64c, 0);
    cpu.edx = cpu.ecx + 0xe4;
    if (!byAddress(app, cpu, kResetDrive))
        return;
    push(app, cpu, cpu.edx);
    cpu.eax = 1;
    push(app, cpu, 0x3f000000);
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    cpu.edx = cpu.ecx + 0xa4;
    push(app, cpu, cpu.edx);
    cpu.eax = 1;
    push(app, cpu, 0x3f000000);
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    putByte(app, cpu.ecx + 0x509, 0xff);
done:
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cee0: the car eax's current road slice +0x638 votes into the three lane
 * costs (kLaneCostA/B/C).  Which way it leans depends on its chasing flag
 * (+0x621 bit 0x10) against the reversed race [0x6fd4c0] and the cached road
 * width's sign [0x56f230]. */
void voteCurrentLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    // With the track's direction on a right-hand track ([0x56f230] 1), or
    // against it on a left-hand one (-1): the first way; else the second.
    const bool against = (byte(app, cpu.eax + 0x621) & 0x10) != 0;
    bool forward = false;
    cpu.edx = (against ? 0 : 1) ^ at(app, kBackwards);
    if (cpu.edx != 0 && at(app, kTrafficSide) == 1)
        forward = true;
    else
    {
        cpu.edx = (against ? 1 : 0) ^ at(app, kBackwards);
        forward = cpu.edx != 0 && at(app, kTrafficSide) == 0xffffffff;
    }
    if (forward)
    {
        cpu.edx = at(app, cpu.eax + 0x638);
        --cpu.edx;
        if (sreg32(cpu.edx) < 7)
            put(app, kLaneCostA, at(app, kLaneCostA) - 3);
        else
            put(app, kLaneCostA, at(app, kLaneCostA) + 1);
        if (sreg32(at(app, cpu.eax + 0x638)) < 7)
            put(app, kLaneCostB, at(app, kLaneCostB) - 3);
        else
            put(app, kLaneCostB, at(app, kLaneCostB) + 1);
        cpu.eax = at(app, cpu.eax + 0x638);
        ++cpu.eax;
        if (sreg32(cpu.eax) >= 7)
            put(app, kLaneCostC, at(app, kLaneCostC) + 1);
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x638);
        --cpu.edx;
        if (sreg32(cpu.edx) < 7)
            put(app, kLaneCostA, at(app, kLaneCostA) + 1);
        else if (sreg32(at(app, cpu.eax + 0x638)) < 7)
            put(app, kLaneCostB, at(app, kLaneCostB) + 1);
        else
            put(app, kLaneCostB, at(app, kLaneCostB) - 3);
        cpu.eax = at(app, cpu.eax + 0x638);
        ++cpu.eax;
        if (sreg32(cpu.eax) < 7)
            put(app, kLaneCostC, at(app, kLaneCostC) + 1);
        else
            put(app, kLaneCostC, at(app, kLaneCostC) - 3);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40cfc0: the car eax senses the traffic around it.  Within a node window
 * set by its heading speed (+0x7c4, chopped), it gathers the other cars
 * (active, +0x89 set, +0x200 bit 6 clear) whose wrapped node gap fits, into
 * local arrays, then for each fills the sensed-car globals: the nearest ahead
 * in each of three bands (kSensed/kSensedFar, +0x56ef68) and the nearest
 * behind (kSensedRight/kSensedSpanLo, +0x56ef6e), and raises [0x56ef7c] when an
 * oncoming car (+0x200 bit 5) is beside it. */
void senseTraffic(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x94;
    const reg32 carsAt = cpu.ebp - 0x94, gapsAt = cpu.ebp - 0x54;
    const reg32 m4 = cpu.ebp - 4, mc = cpu.ebp - 0xc, m8 = cpu.ebp - 8, m10 = cpu.ebp - 0x10, m14 = cpu.ebp - 0x14;
    cpu.ecx = cpu.eax;
    fld32(app, cpu, cpu.eax + 0x7c4);
    fmul(cpu, f64(app, 0x53670c));  // 2.0
    fmul(cpu, f64(app, 0x536714));  // 0.1667
    cpu.edx = 0;
    if (!byAddress(app, cpu, kChop))
        return;
    put(app, m4, fistpPop(app, cpu));
    cpu.ebx = at(app, m4);
    put(app, mc, cpu.edx);
    if (sreg32(cpu.ebx) < 0xa)
        put(app, m4, 0xa);
    if ((byte(app, cpu.ecx + 0x621) & 0x10) != 0)
        cpu.eax = 0;
    else
        cpu.eax = 1;
    cpu.eax ^= at(app, kBackwards);
    cpu.edx = cpu.eax == 0 ? 0xffffffff : 1;
    put(app, m14, cpu.edx);
    cpu.edi = at(app, cpu.ecx + 0x1c);
    cpu.ebx = 0;
    for (; sreg32(cpu.ebx) < sreg32(at(app, kCarCount)); ++cpu.ebx)
    {
        if (cpu.ebx == at(app, cpu.ecx + 0x1f0))
            continue;
        cpu.esi = at(app, cpu.ebx * 4 + kCars);
        if (byte(app, cpu.esi + 0x89) == 0)
            continue;
        if ((byte(app, cpu.esi + 0x200) & 0x40) != 0)
            continue;
        cpu.edx = cpu.edi;
        cpu.eax = at(app, cpu.esi + 0x1c);
        if (!direct(app, cpu, nodeGap))
            return;
        cpu.eax = reg32(sreg32(cpu.eax) * sreg32(at(app, m14)));
        if (sreg32(cpu.eax) > 0)
            cpu.edx = cpu.eax;
        else
        {
            cpu.edx = cpu.eax;
            cpu.edx = reg32(0) - cpu.edx;
        }
        if (sreg32(cpu.edx) > sreg32(at(app, m4)))
            continue;
        cpu.edx = at(app, mc);
        cpu.edx <<= 2;
        put(app, cpu.edx + carsAt, cpu.esi);
        cpu.esi = at(app, mc);
        ++cpu.esi;
        put(app, cpu.edx + gapsAt, cpu.eax);
        put(app, mc, cpu.esi);
    }
    cpu.eax = at(app, m4);
    cpu.esi = 0;
    put(app, kSensedCount, cpu.eax);
    put(app, kSensedCountB, cpu.eax);
    cpu.edi = cpu.eax;
    put(app, kSensedSpanLo, cpu.eax);
    cpu.edi = reg32(0) - cpu.edi;
    put(app, kSensedFar, cpu.eax);
    put(app, kSensedSpanLo, cpu.edi);
    put(app, 0x56ef60, cpu.edi);
    put(app, 0x56ef64, cpu.edi);
    for (; sreg32(cpu.esi) < sreg32(at(app, mc)); ++cpu.esi)
    {
        cpu.eax = at(app, cpu.ebp + cpu.esi * 4 - 0x94);
        cpu.edx = at(app, cpu.eax + 0x63c);
        if (cpu.edx == at(app, cpu.eax + 0x640))
        {
            cpu.ebx = at(app, cpu.ecx + 0x638);
            cpu.eax = at(app, cpu.eax + 0x638);
            cpu.eax -= cpu.ebx;
            ++cpu.eax;
            put(app, m8, cpu.eax);
            put(app, m10, cpu.eax);
            cpu.ebx = cpu.eax;
        }
        else
        {
            cpu.edi = at(app, cpu.ecx + 0x638);
            cpu.edx -= cpu.edi;
            cpu.ebx = cpu.edx + 1;
            cpu.edx = at(app, cpu.eax + 0x638);
            cpu.eax = at(app, cpu.eax + 0x640);
            cpu.edx -= cpu.edi;
            cpu.eax -= cpu.edi;
            ++cpu.edx;
            ++cpu.eax;
            put(app, m10, cpu.edx);
            put(app, m8, cpu.eax);
        }
        cpu.eax = cpu.ebx;
        for (; sreg32(cpu.eax) <= sreg32(at(app, m8)); ++cpu.eax)
        {
            if (sreg32(cpu.eax) < 0)
                continue;
            if (sreg32(cpu.eax) > 2)
                continue;
            cpu.edx = cpu.esi * 4;
            cpu.edi = at(app, cpu.edx + gapsAt);
            cpu.ebx = cpu.eax * 4;
            if (sreg32(cpu.edi) >= 0)
            {
                if (sreg32(cpu.edi) >= sreg32(at(app, cpu.ebx + kSensedFar)))
                    continue;
                cpu.edx = at(app, cpu.edx + carsAt);
                put(app, cpu.ebx + kSensed, cpu.edx);
                cpu.edx = at(app, m10);
                put(app, cpu.ebx + kSensedFar, cpu.edi);
                cpu.edx = cpu.eax == cpu.edx ? 1 : 0;
                app->getMemory<reg16>(cpu.eax * 2 + kSensedEdge) = reg16(cpu.edx);
            }
            else
            {
                if (sreg32(cpu.edi) <= sreg32(at(app, cpu.ebx + kSensedSpanLo)))
                    continue;
                cpu.edx = at(app, cpu.edx + carsAt);
                put(app, cpu.ebx + kSensedSpanLo, cpu.edi);
                put(app, cpu.ebx + kSensedRight, cpu.edx);
                cpu.edx = cpu.eax == at(app, m10) ? 1 : 0;
                app->getMemory<reg16>(cpu.eax * 2 + kSensedEdgeB) = reg16(cpu.edx);
                cpu.edx = at(app, cpu.eax * 4 + kSensedRight);
                if ((byte(app, cpu.edx + 0x200) & 0x20) != 0)
                    put(app, kSensedFlag, 1);
            }
        }
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

/* 0x40d1f0: whether the car eax can move toward node edx, and the cost of it.
 * It finds how many nodes ahead the target lies (nodeOffset 1..0x13), then along
 * three node columns (laneBit of the prev/cur/next slice) checks each
 * step with nodeLaneBits; a clear column cuts [0x56eec0+col] and fixes the lane
 * direction [0x56ef78], a blocked step adds a smaller cut. */
void planLaneChange(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x60;
    const reg32 m10 = cpu.ebp - 0x10, m4 = cpu.ebp - 4, chain = cpu.ebp - 0x60;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.eax = at(app, cpu.eax + 0x638);
    --cpu.eax;
    if (!direct(app, cpu, laneBit)) return;
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x638);
    if (!direct(app, cpu, laneBit)) return;
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x638);
    ++cpu.eax;
    if (!direct(app, cpu, laneBit)) return;
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    put(app, chain, cpu.eax);
    if (cpu.edx == cpu.eax)
        goto done;
    cpu.ebx = 1;
    while (true)
    {
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, nodeOffset)) return;
        put(app, cpu.ebp + cpu.ebx * 4 - 0x60, cpu.eax);
        if (cpu.esi == cpu.eax)
            break;
        ++cpu.ebx;
        if (sreg32(cpu.ebx) >= 0x14)
            break;
    }
    cpu.ecx = 0;
    for (; sreg32(cpu.ecx) < 3; ++cpu.ecx)
    {
        cpu.esi = cpu.ecx * 4;
        cpu.eax = at(app, chain);
        cpu.edx = at(app, cpu.esi + m10);
        if (!direct(app, cpu, nodeLaneBits)) return;
        if (cpu.eax == 0)
        {
            // Column clear the whole way: a big cut and the chosen direction
            // (left column 1, middle 0 -- the nodeLaneBits result -- right -1).
            put(app, cpu.esi + kNodeCostE, at(app, cpu.esi + kNodeCostE) - 0x3e8);
            if (cpu.ecx == 0)
                cpu.eax = 1;
            else if (cpu.ecx == 2)
                cpu.eax = 0xffffffff;
            else
                cpu.eax = 0;
            put(app, kSensedDir, cpu.eax);
            goto done;
        }
        cpu.esi = 1;
        for (; sreg32(cpu.esi) < sreg32(cpu.ebx); ++cpu.esi)
        {
            cpu.edi = cpu.ecx * 4;
            cpu.eax = at(app, cpu.edi + m10);
            put(app, m4, cpu.eax);
            cpu.edx = at(app, m4);
            cpu.eax = at(app, cpu.ebp + cpu.esi * 4 - 0x60);
            if (!direct(app, cpu, nodeLaneBits)) return;
            if (cpu.eax != 0)
                continue;
            put(app, cpu.edi + kNodeCostE, at(app, cpu.edi + kNodeCostE) - 0x41);
            break;
        }
    }
done:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40d2e0: a mid-road penalty for the car eax heading to node edx.  A player
 * car (+0x200 bit 0) in a specific stretch of track 6 gets one cut; otherwise,
 * when the car's slice +0x638 lies between the node's two edges and its own
 * node's edges, it cuts the object-lane costs (0x56eeb0 or 0x56eea8) by 0x28. */
void edgeLaneCost(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.bl = byte(app, cpu.ecx + 0x200);
    cpu.edx = at(app, cpu.ecx + 0x638);
    if ((cpu.bl & 1) != 0 && at(app, kBackwards) == 0 && at(app, kTrackNumber) == 6)
    {
        cpu.edi = at(app, cpu.ecx + 0x1c);
        if (sreg32(cpu.edi) > 0x126 && sreg32(cpu.edi) < 0x169)
        {
            put(app, kLaneCostObj, at(app, kLaneCostObj) - 0x28);
            goto done;
        }
    }
    cpu.ebx = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax += cpu.ebx;
    cpu.ebx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ebx;
    cpu.ebx = 0;
    cpu.bl = byte(app, cpu.eax + 0x13);
    cpu.ebx = reg32(sreg32(cpu.ebx) >> 4);
    cpu.esi = 7;
    cpu.ebx &= 0xf;
    cpu.esi -= cpu.ebx;
    if (sreg32(cpu.edx) < sreg32(cpu.esi))
        goto carNode;
    cpu.al = byte(app, cpu.eax + 0x13);
    cpu.al &= 0xf;
    cpu.eax &= 0xff;
    cpu.eax += 6;
    if (sreg32(cpu.edx) <= sreg32(cpu.eax))
        goto done;
carNode:
    cpu.ecx = at(app, cpu.ecx + 0x1c);
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.eax + 0x13);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 4);
    cpu.ebx = 7;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    if (sreg32(cpu.edx) < sreg32(cpu.ebx))
        goto done;
    cpu.al = byte(app, cpu.eax + 0x13);
    cpu.al &= 0xf;
    cpu.eax &= 0xff;
    cpu.eax += 6;
    if (sreg32(cpu.edx) <= sreg32(cpu.eax))
    {
        if (sreg32(cpu.edx) >= 7)
            put(app, kLaneCostObj, at(app, kLaneCostObj) + 0x28);
        else
            put(app, kLaneCostObjB, at(app, kLaneCostObjB) + 0x28);
    }
done:
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40d9f0: a small cut to the far lane costs when a car is sensed close
 * ahead (kSensed set, kSensedFar < 2 -> 0x56ee9c -= 4) or close behind
 * (kSensedAll set, kSensedCountB < 2 -> 0x56eea4 -= 4). */
void edgePenalty(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kSensed) != 0 && sreg32(at(app, kSensedFar)) < 2)
        put(app, kLaneCostLo, at(app, kLaneCostLo) - 4);
    if (at(app, kSensedAll) != 0 && sreg32(at(app, kSensedCountB)) < 2)
        put(app, kLaneCostHi, at(app, kLaneCostHi) - 4);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40da30: the 14 rows of lane costs summed into three columns, and the car
 * eax's chosen lane [0x56ef74] set to its current slice +0x638, shifted left or
 * right toward the column with the highest sum, clamped to 0..0xd. */
void sumLaneVotes(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 m8 = cpu.ebp - 8, m4 = cpu.ebp - 4, mc = cpu.ebp - 0xc;
    cpu.ecx = cpu.eax;
    cpu.edx = 0;
    put(app, m8, cpu.edx);
    put(app, m4, cpu.edx);
    put(app, mc, cpu.edx);
    for (; sreg32(cpu.edx) < 0xe; ++cpu.edx)
    {
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.edi = at(app, mc);
        cpu.ebx = at(app, cpu.eax * 4 + kLaneCostA);
        cpu.esi = at(app, m8);
        cpu.edi += cpu.ebx;
        put(app, mc, cpu.edi);
        cpu.ebx = at(app, cpu.eax * 4 + kLaneCostB);
        cpu.edi = at(app, m4);
        cpu.eax = at(app, cpu.eax * 4 + kLaneCostC);
        cpu.esi += cpu.ebx;
        cpu.edi += cpu.eax;
        put(app, m8, cpu.esi);
        put(app, m4, cpu.edi);
    }
    cpu.eax = at(app, cpu.ecx + 0x638);
    cpu.edx = at(app, mc);
    put(app, kChosenLane, cpu.eax);
    cpu.eax = at(app, m8);
    if (sreg32(cpu.edx) > sreg32(cpu.eax))
    {
        cpu.eax = at(app, cpu.ecx + 0x638);
        --cpu.eax;
        put(app, kChosenLane, cpu.eax);
        cpu.eax = cpu.edx;
    }
    if (sreg32(cpu.eax) < sreg32(at(app, m4)))
    {
        cpu.eax = at(app, cpu.ecx + 0x638);
        ++cpu.eax;
        put(app, kChosenLane, cpu.eax);
    }
    cpu.edi = at(app, kChosenLane);
    if (sreg32(cpu.edi) < 0)
    {
        cpu.eax = 0;
        put(app, kChosenLane, cpu.eax);
    }
    else if (sreg32(cpu.edi) >= 0xe)
        put(app, kChosenLane, 0xd);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40dd60: the lane planner started for car eax -- the cost globals (0xa8 at
 * 0x56ee90) and the sensed table (0x48 at kSensed) cleared, the car's road
 * slices computed (carSlices), its current lane voted (voteCurrentLane) and
 * the traffic sensed (senseTraffic). */
void beginLanePlanning(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.ebx = 0xa8;
    cpu.eax = 0x56ee90;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset)) return;
    cpu.ebx = 0x48;
    cpu.eax = kSensed;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset)) return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, carSlices)) return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, voteCurrentLane)) return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, senseTraffic)) return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ddb0: the car eax picks its lane.  It looks a speed-scaled number of
 * nodes ahead (nodeOffset of max(9, round(+0x7c4*2.5*0.1667-0.5))), weighs the
 * edge (edgeLaneCost) and the lane change (planLaneChange), scores the lanes
 * by the road (scoreLanesByRoad), and, unless flagged (+0x200 bit 2), by
 * obstacles, cars and edges. */
void chooseLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    fld32(app, cpu, cpu.eax + 0x7c4);
    fmul(cpu, f64(app, 0x536784));  // 2.5
    fmul(cpu, f64(app, 0x53678c));  // 0.1667
    fadd(cpu, f64(app, 0x536794));  // -0.5
    cpu.eax = fistpPop(app, cpu);
    if (sreg32(cpu.eax) <= 9)
        cpu.edx = 9;
    else
        cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, nodeOffset)) return;
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, edgeLaneCost)) return;
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, planLaneChange)) return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, scoreLanesByRoad)) return;
    if ((byte(app, cpu.ecx + 0x200) & 4) == 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, scoreLanesByObstacles)) return;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, scoreLanesByCars)) return;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, edgePenalty)) return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40de50: the car eax's horn sounded for 0x40 ticks (+0x620 bit 3, its end
 * +0x644 past the clock), its tone +0x660 on 0 -> 1 -> 2, any other back to 0. */
void startHorn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    putByte(app, cpu.eax + 0x620, reg8(byte(app, cpu.eax + 0x620) | 8));
    cpu.edx = at(app, kClock);
    cpu.edx += 0x40;
    put(app, cpu.eax + 0x644, cpu.edx);
    cpu.edx = at(app, cpu.eax + 0x660);
    if (cpu.edx == 0)
        put(app, cpu.eax + 0x660, 1);
    else if (cpu.edx == 1)
        put(app, cpu.eax + 0x660, 2);
    else
        put(app, cpu.eax + 0x660, 0);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40deb0: the car eax's horn ended once the clock passes +0x644: +0x620
 * bit 3 cleared, its tone +0x660 back (1 -> 0, 2 -> 1). */
void stopHorn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if ((byte(app, cpu.eax + 0x620) & 8) != 0)
    {
        cpu.edx = at(app, kClock);
        if (sreg32(cpu.edx) > sreg32(at(app, cpu.eax + 0x644)))
        {
            putByte(app, cpu.eax + 0x620, reg8(byte(app, cpu.eax + 0x620) & 0xf7));
            cpu.edx = at(app, cpu.eax + 0x660);
            if (cpu.edx == 1)
                put(app, cpu.eax + 0x660, 0);
            else if (cpu.edx == 2)
                put(app, cpu.eax + 0x660, 1);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40df10: the traffic car eax's loading steps (+0x630; 0x401240 and
 * 0x405cd0..0x405e20 are below this chunk): 0 when 0x401240 says it is not on
 * the road; state 0 placed on the track (for +0x620 bit 1: 0x4a0aa0 with the
 * place at 0x40cb0c, then 0x405d40) -- 0; 1 started (0x405e20, 0x405dc0) --
 * 1, or 0 until it can; 2 running -- 1, or taken off (0x405cd0, 0x405d40) -- 0. */
void stepTrafficLoading(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    put(app, cpu.ebp - 4, cpu.eax);
    if (!byAddress(app, cpu, kActive)) return;
    if (cpu.eax == 0)
        goto retZero;
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.eax = at(app, cpu.eax + 0x630);
    if (cpu.eax == 1)
        goto state1;
    if (cpu.eax == 2)
        goto state2;
    // state 0 (and anything else): place on the grid.
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.dl = byte(app, cpu.eax + 0x620);
    put(app, cpu.eax + 0x630, 2);
    if ((cpu.dl & 2) != 0)
    {
        cpu.edi = cpu.ebp - 0x10;
        cpu.ebx = cpu.ebp - 0x10;
        cpu.esi = 0x40cb0c;
        cpu.ecx = at(app, kBackwards);
        cpu.edx = 0;
        movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
        if (!byAddress(app, cpu, kPlaceOnRoad)) return;
        cpu.eax = at(app, cpu.ebp - 4);
        if (!byAddress(app, cpu, kRecoverB)) return;
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.eax + 0x630, 1);
    }
    goto retZero;
state1:
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kRecoverD)) return;
    if (cpu.eax == 0)
        goto ret;
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kRecoverC)) return;
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax + 0x630, 2);
    cpu.eax = 1;
    goto ret;
state2:
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kRecoverA)) return;
    if (cpu.eax == 0)
    {
        cpu.eax = 1;
        goto ret;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    if (!byAddress(app, cpu, kRecoverB)) return;
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax + 0x630, 1);
    cpu.eax = 0;
    goto ret;
retZero:
    cpu.eax = 0;
ret:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40d3d0: the car eax's lanes scored by the road's shape.  At its node the
 * left/centre/right lane costs (0x56eeb4/b8/bc) are trimmed where the car's
 * slice +0x638 sits on a slanted edge (-0x1e, then -0x32 when the edge is
 * nearer than the car is long +0x114), and a lane that the node's run does not
 * admit is cut hard (-0x3e8).  The node's +0x13 nibbles give the two edge
 * slices, +0x17/+0x1b/+0x1c/+0x20 the edge distances. */
void scoreLanesByRoad(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const reg32 carnode = at(app, car + 0x1c);
    const reg32 node = nodeAddr(app, carnode);
    const i32 b13 = byte(app, node + 0x13);
    const i32 high = (b13 >> 4) & 0xf, low = b13 & 0xf;
    const i32 edgeLo = 6 - high, edgeHi = low + 7;
    const i32 slice = sreg32(at(app, car + 0x638));
    const i32 sm1 = slice - 1, sp1 = slice + 1;
    const i32 leftEdge = sreg32(at(app, node + 0x1c) - reg32(reg32(byte(app, node + 0x17)) << 15) * reg32(high));
    const i32 rightEdge = sreg32(at(app, node + 0x20) - reg32(reg32(byte(app, node + 0x1b)) << 15) * reg32(low));
    const auto roundLen = [&]() -> i32 {
        fld32(app, cpu, car + 0x114);
        fmul(cpu, f64(app, 0x53671c));  // 65536.0
        return sreg32(fistpPop(app, cpu));
    };
    if (sm1 == edgeLo || sm1 == edgeHi)
    {
        put(app, 0x56eeb4, at(app, 0x56eeb4) - 0x1e);
        if (leftEdge < roundLen())
            put(app, 0x56eeb4, at(app, 0x56eeb4) - 0x32);
    }
    if (slice == edgeLo || edgeHi == slice)
    {
        put(app, 0x56eeb8, at(app, 0x56eeb8) - 0x1e);
        if (slice < 7)
        {
            if (leftEdge < roundLen())
                put(app, 0x56eeb8, at(app, 0x56eeb8) - 0x32);
        }
        else if (rightEdge < roundLen())
            put(app, 0x56eeb8, at(app, 0x56eeb8) - 0x32);
    }
    if (sp1 == edgeLo || edgeHi == sp1)
    {
        put(app, 0x56eebc, at(app, 0x56eebc) - 0x1e);
        if (rightEdge < roundLen())
            put(app, 0x56eebc, at(app, 0x56eebc) - 0x32);
    }
    if (sm1 < edgeLo || sm1 > edgeHi)
        put(app, 0x56eeb4, at(app, 0x56eeb4) - 0x3e8);
    if (edgeLo > slice || edgeHi < slice)
        put(app, 0x56eeb8, at(app, 0x56eeb8) - 0x3e8);
    if (sp1 < edgeLo || sp1 > edgeHi)
        put(app, 0x56eebc, at(app, 0x56eebc) - 0x3e8);
    // Its pushes and pops leave every register as it was but eax: the node's
    // address, or its right edge slice when it got as far as reading it.
    cpu.eax = sp1 < edgeLo ? node : reg32(edgeHi);
    cpu.esp += 4;
}

/* 0x40d720: the car eax's lanes scored against the fixed obstacles (0x7dd850,
 * up to 8 of 0x50 bytes).  For each live obstacle within 5 nodes ahead
 * (nodeGap, signed by the car's chasing flag and the reversed race) whose node
 * matches one of the three lanes around the car's slice +0x638, that lane's
 * object cost (0x56eecc) loses 0x32, and the centre lane also damps the car's
 * steer +0x7b0 by 0.9. */
void scoreLanesByObstacles(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ebx = cpu.eax;
    cpu.ecx = 0;
    while (true)
    {
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        cpu.eax <<= 4;  // ecx * 0x50
        if (at(app, cpu.eax + kObstacles) == 0)
            goto next;
        if ((byte(app, cpu.ebx + 0x621) & 0x10) != 0)
            cpu.eax = 0;
        else
            cpu.eax = 1;
        cpu.eax ^= at(app, kBackwards);
        cpu.edi = cpu.eax == 0 ? 0xffffffff : 1;
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        cpu.eax <<= 4;
        cpu.edx = at(app, cpu.ebx + 0x1c);
        cpu.eax = at(app, cpu.eax + kObstacles + 0x34);
        if (!direct(app, cpu, nodeGap)) return;
        cpu.eax = reg32(sreg32(cpu.eax) * sreg32(cpu.edi));
        if (sreg32(cpu.eax) <= 0)
            goto next;
        if (sreg32(cpu.eax) > 5)
            goto next;
        cpu.edx = 0;
        while (true)
        {
            cpu.eax = cpu.ecx * 4;
            cpu.edi = at(app, cpu.ebx + 0x638);
            cpu.esi = cpu.ecx + cpu.eax;
            cpu.edi += cpu.edx;
            cpu.esi <<= 4;
            cpu.eax = cpu.edi - 1;
            if (cpu.eax == at(app, cpu.esi + kObstacles + 0x38))
            {
                put(app, cpu.edx * 4 + kObjCost, at(app, cpu.edx * 4 + kObjCost) - 0x32);
                if (cpu.edx == 1)
                {
                    fld32(app, cpu, cpu.ebx + 0x7b0);
                    cpu.fpu.push(Float(cpu.fpu.st(0)));  // fld st(0)
                    fmul(cpu, f64(app, 0x536724));        // 0.9
                    cpu.fpu.st(1) = cpu.fpu.st(0);        // fstp st(1)
                    cpu.fpu.pop();
                    fstp32(app, cpu, cpu.ebx + 0x7b0);
                }
            }
            ++cpu.edx;
            if (sreg32(cpu.edx) >= 3)
                break;
        }
    next:
        ++cpu.ecx;
        if (sreg32(cpu.ecx) >= 8)
            break;
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

/* 0x40d800: the car eax's lanes scored against the sensed cars ahead
 * (kSensed, three bands).  An oncoming car (speeds of opposite sign) closing
 * faster than 1 cuts that band's left cost (0x56eed8) by a time-to-meet term
 * (its gap, chopped by 0x4dfd56, over the speed difference); a car going the
 * same way that the car's steer outruns cuts the right cost (0x56eee4).  Halved
 * where the band is not an edge (0x56ef68 words). */
void scoreLanesByCars(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    const reg32 m4 = cpu.ebp - 4, m8 = cpu.ebp - 8, mc = cpu.ebp - 0xc, m10 = cpu.ebp - 0x10;
    const reg32 m14 = cpu.ebp - 0x14, m18 = cpu.ebp - 0x18, m20 = cpu.ebp - 0x20;
    // edx halved toward zero, as the code's mov/sar/sub/sar does it.
    const auto halve = [&] {
        const i32 orig = sreg32(cpu.edx);
        const i32 s = orig >> 31;
        cpu.eax = reg32((orig - s) >> 1);
        cpu.edx = cpu.eax;
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = 0;
    for (; sreg32(cpu.ecx) < 3; ++cpu.ecx)
    {
        cpu.edx = at(app, cpu.ecx * 4 + kSensed);
        if (cpu.edx == 0)
            continue;
        fld32(app, cpu, cpu.ebx + 0x7c0);
        fldz(cpu);
        cpu.esi = fcompp(cpu).belowOrEqual() ? 0 : 1;
        fld32(app, cpu, cpu.edx + 0x7c0);
        fldz(cpu);
        cpu.eax = fcompp(cpu).belowOrEqual() ? 0 : 1;
        cpu.eax ^= cpu.esi;
        if (cpu.eax != 0)
        {
            // Oncoming.
            fld32(app, cpu, cpu.ebx + 0x7c0);
            fsub(cpu, f32(app, cpu.edx + 0x7c0));
            fldz(cpu);
            fxch(cpu, 1);
            fstp32(app, cpu, mc);
            if (!fcomp(cpu, f32(app, mc)).aboveOrEqual())
            {
                cpu.eax = at(app, mc);
                put(app, m14, cpu.eax);
            }
            else
            {
                fld32(app, cpu, mc);
                fchs(cpu);
                fstp32(app, cpu, m14);
            }
            fld32(app, cpu, m14);
            fld1(cpu);
            if (fcompp(cpu).aboveOrEqual())
                continue;
            fild(app, cpu, cpu.ecx * 4 + kSensedFar);
            fmul(cpu, f64(app, 0x53672c));  // 6.0
            fmul(cpu, f64(app, 0x536734));  // 65536.0
            if (!byAddress(app, cpu, kChop))
                return;
            put(app, m18, fistpPop(app, cpu));
            cpu.eax = at(app, m18);
            put(app, m4, cpu.eax);
            fild(app, cpu, m4);
            fmul(cpu, f64(app, 0x53673c));  // 1/65536
            fdiv(cpu, f32(app, mc));
            fmul(cpu, f32(app, 0x536744));  // -35.0
            fmul(cpu, f64(app, 0x53674c));  // 0.125
            fadd(cpu, f64(app, 0x536754));  // -0.5
            cpu.edx = 0xffffffdd;
            cpu.eax = fistpPop(app, cpu);
            const reg16 edge = app->getMemory<reg16>(cpu.ecx * 2 + kSensedEdge);
            cpu.si = edge;
            cpu.edx -= cpu.eax;
            if (edge == 0)
                halve();
            if (sreg32(cpu.edx) >= 0)
                continue;
            put(app, cpu.ecx * 4 + kCarCostLeft, at(app, cpu.ecx * 4 + kCarCostLeft) + cpu.edx);
            continue;
        }
        // Same way: |steer| against |its speed|.
        fldz(cpu);
        if (!fcomp(cpu, f32(app, cpu.ebx + 0x7b0)).aboveOrEqual())
        {
            cpu.eax = at(app, cpu.ebx + 0x7b0);
            put(app, m10, cpu.eax);
        }
        else
        {
            fld32(app, cpu, cpu.ebx + 0x7b0);
            fchs(cpu);
            fstp32(app, cpu, m10);
        }
        fldz(cpu);
        if (!fcomp(cpu, f32(app, cpu.edx + 0x7c0)).aboveOrEqual())
        {
            cpu.eax = at(app, cpu.edx + 0x7c0);
            put(app, m8, cpu.eax);
        }
        else
        {
            fld32(app, cpu, cpu.edx + 0x7c0);
            fchs(cpu);
            fstp32(app, cpu, m8);
        }
        fld32(app, cpu, m10);
        fsub(cpu, f32(app, m8));
        fldz(cpu);
        fxch(cpu, 1);
        fstp64(app, cpu, m20);
        if (fcomp(cpu, f64(app, m20)).aboveOrEqual())
            continue;
        cpu.esi = at(app, cpu.ecx * 4 + kSensedFar);
        if (sreg32(cpu.esi) >= 0xa)
            continue;
        fld64(app, cpu, m20);
        if (!fcomp(cpu, f64(app, 0x53675c)).below())  // 3.0
            cpu.edx = 0xfffffff8;
        else
        {
            fld64(app, cpu, m20);
            fmul(cpu, f64(app, 0x536764));  // -5.0
            fmul(cpu, f64(app, 0x53676c));  // 1/3
            fadd(cpu, f64(app, 0x536754));  // -0.5
            cpu.eax = fistpPop(app, cpu);
            cpu.edx = cpu.eax - 3;
        }
        cpu.eax = cpu.esi - 5;
        if (sreg32(cpu.eax) <= 0)
            cpu.eax = 0;
        const reg16 edge = app->getMemory<reg16>(cpu.ecx * 2 + kSensedEdge);
        cpu.di = edge;
        cpu.edx += cpu.eax;
        if (edge == 0)
            halve();
        if (sreg32(cpu.edx) >= 0)
            continue;
        put(app, cpu.ecx * 4 + kCarCostRight, at(app, cpu.ecx * 4 + kCarCostRight) + cpu.edx);
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

/* 0x40daf0: the car eax's target steer +0x7b4 set toward the chosen lane
 * [0x56ef74].  On the road (the lane between the node's two edge slices) it
 * is the lane's offset from the edge times the edge's width (+0x17 left of
 * slice 7, +0x1b right), less or plus the car's length +0x114; off the road
 * half an edge's width back.  Then it is clamped between the node's left
 * (+0x1c) and right (+0x20) bounds less the car's length, and nudged by the
 * planner's direction [0x56ef78] (negated for a chasing car).  The node base
 * [0x5dd948] is written back as read. */
void steerToLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2c;
    const reg32 mc = cpu.ebp - 0xc, m14 = cpu.ebp - 0x14, m1c = cpu.ebp - 0x1c, m2c = cpu.ebp - 0x2c;
    const reg32 base = at(app, kNodes);
    const reg32 car = cpu.eax;
    const reg32 carnode = at(app, car + 0x1c);
    const reg32 node = base + carnode * 0x24;
    const reg32 steer = car + 0x7b4;
    const i32 b13 = byte(app, node + 0x13);
    const i32 high = (b13 >> 4) & 0xf, low = b13 & 0xf;
    const i32 chosen = sreg32(at(app, kChosenLane));
    const auto pushInt = [&](i32 v) { cpu.fpu.push(Float(v)); };
    const i32 left = i32(byte(app, node + 0x17)) << 15, right = i32(byte(app, node + 0x1b)) << 15;
    if (6 - high == chosen || low + 7 == chosen)
    {
        if (chosen < 7)
        {
            pushInt(left);
            fmul(cpu, f64(app, 0x536774));  // 1/65536
            pushInt(chosen - 6);
            fmulp(cpu, 1);
            fld32(app, cpu, car + 0x114);
            fxch(cpu, 1);
            fstp32(app, cpu, steer);
            fsubr(cpu, f32(app, steer));
        }
        else
        {
            pushInt(right);
            fmul(cpu, f64(app, 0x536774));
            pushInt(chosen - 7);
            fmulp(cpu, 1);
            fld32(app, cpu, car + 0x114);
            fxch(cpu, 1);
            fstp32(app, cpu, steer);
            fadd(cpu, f32(app, steer));
        }
        fstp32(app, cpu, steer);
    }
    else
    {
        const bool leftSide = chosen < 7;
        const i32 width = leftSide ? left : right;
        pushInt(width);
        fld64(app, cpu, 0x536774);
        fxch(cpu, 1);
        fmulSt0(cpu, 1);
        pushInt(leftSide ? chosen - 6 : chosen - 7);
        fmulp(cpu, 1);
        fstp32(app, cpu, steer);
        pushInt(width);
        fmulp(cpu, 1);
        fmul(cpu, f64(app, 0x53677c));  // 0.5
        if (leftSide)
            fsubr(cpu, f32(app, steer));
        else
            fadd(cpu, f32(app, steer));
        fstp32(app, cpu, steer);
    }
    // The right bound: node +0x20 / 65536 - length.
    pushInt(sreg32(at(app, node + 0x20)));
    fmul(cpu, f64(app, 0x536774));
    fld32(app, cpu, car + 0x114);
    fstp64(app, cpu, m1c);
    fsub(cpu, f64(app, m1c));
    fld32(app, cpu, steer);
    fxch(cpu, 1);
    fstp64(app, cpu, mc);
    if (!fcomp(cpu, f64(app, mc)).belowOrEqual())
    {
        fld64(app, cpu, mc);
        fstp32(app, cpu, steer);
    }
    // The left bound: length - node +0x1c / 65536 (eax the nodes' base, its
    // low half the x87's status word after the compare).
    cpu.eax = base;
    pushInt(sreg32(at(app, node + 0x1c)));
    fmul(cpu, f64(app, 0x536774));
    fld32(app, cpu, car + 0x114);
    fxch(cpu, 1);
    fstp64(app, cpu, m2c);
    fsub(cpu, f64(app, m2c));
    fld32(app, cpu, steer);
    fxch(cpu, 1);
    fstp64(app, cpu, m14);
    if (!fcomp(cpu, f64(app, m14)).aboveOrEqual())
    {
        fld64(app, cpu, m14);
        fstp32(app, cpu, steer);
    }
    if (at(app, kSensedDir) != 0)
    {
        if ((byte(app, car + 0x621) & 0x10) != 0)
        {
            cpu.eax = reg32(0) - at(app, kSensedDir);
            pushInt(sreg32(cpu.eax));
        }
        else
            pushInt(sreg32(at(app, kSensedDir)));
        fmul(cpu, f32(app, car + 0x114));
        fadd(cpu, f32(app, steer));
        fstp32(app, cpu, steer);
    }
    put(app, kNodes, base);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e080: the computer cars' target speed at slice eax onto the x87's stack
 * -- the byte of the track's speed table [0x552558] at eax / 2, or 64 when
 * that table failed to load [0x56f22c]. */
void targetSpeedAt(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    if (at(app, kSpeedsFailed) != 0)
        put(app, cpu.ebp - 8, 0x42800000);
    else
    {
        cpu.edx = cpu.eax;
        cpu.eax = at(app, kSpeeds);
        cpu.edx >>= 1;
        cpu.edx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.edx);
        put(app, cpu.ebp - 4, cpu.eax);
        fild16(app, cpu, cpu.ebp - 4);
        fstp32(app, cpu, cpu.ebp - 8);
    }
    fld32(app, cpu, cpu.ebp - 8);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e0c0: the speed table's second half's (0x55255c) high nibble at slice
 * eax -- the road kind there; 7 when the table failed to load. */
void roadKindAt(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kSpeedsFailed) != 0)
        cpu.eax = 7;
    else
    {
        cpu.edx = at(app, kSpeedsHalf);
        cpu.al = byte(app, cpu.edx + cpu.eax);
        cpu.eax &= 0xff;
        cpu.eax = reg32(sreg32(cpu.eax) >> 4);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e0f0: the track's road kind cached for the race: whether it is wide
 * ([0x56f228], roadIsWide), its width sign ([0x56f230], trafficSide), the
 * walk's counter [0x56f220] reset. */
void cacheRoadWidth(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, roadIsWide)) return;
    put(app, kSpeedsReady, cpu.eax);
    if (!direct(app, cpu, trafficSide)) return;
    cpu.edx = 0;
    put(app, kTrafficSide, cpu.eax);
    put(app, kTrackWalked, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e120: node eax less node edx, wrapped round the track's [0x5dd940]
 * nodes into -n/2..n/2. */
void nodeGap(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, kNodeCount);
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ecx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    cpu.eax -= cpu.edx;
    cpu.eax = reg32(sreg32(cpu.eax) >> 1);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.esi;
    cpu.eax -= cpu.ebx;
    if (sreg32(cpu.eax) > sreg32(cpu.edx))
        cpu.eax -= cpu.ecx;
    else
    {
        cpu.edx = reg32(0) - cpu.edx;
        if (sreg32(cpu.eax) < sreg32(cpu.edx))
            cpu.eax += cpu.ecx;
    }
    put(app, kNodeCount, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e160: the node edx nodes on from the car eax's node +0x1c, in the car's
 * direction of travel (back for a non-chasing car in a forward race, or a
 * chasing car in a reversed one), wrapped round the track. */
void nodeOffset(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, kNodeCount);
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = (byte(app, cpu.ecx + 0x621) & 0x10) != 0 ? 0 : 1;
    cpu.edx ^= at(app, kBackwards);
    if (cpu.edx == 0)
        cpu.eax = reg32(0) - cpu.eax;
    if (sreg32(cpu.eax) < 0)
    {
        cpu.eax += at(app, cpu.ecx + 0x1c);
        if (sreg32(cpu.eax) < 0)
            cpu.eax += cpu.ebx;
    }
    else
    {
        cpu.eax += at(app, cpu.ecx + 0x1c);
        if (sreg32(cpu.eax) >= sreg32(cpu.ebx))
            cpu.eax -= cpu.ebx;
    }
    put(app, kNodeCount, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e1b0 (stdcall, 3 words: a scale, a least, a most): the node the car eax
 * will reach -- its heading speed +0x7c4 times the scale and 1/6, rounded,
 * kept within the least and most, backwards when +0x7d4 is 1 (nodeOffset). */
void nodesAheadBySpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, cpu.ebp + 0x1c);
    cpu.esi = at(app, cpu.ebp + 0x20);
    cpu.ecx = cpu.eax;
    fld32(app, cpu, cpu.eax + 0x7c4);
    fmul(cpu, f32(app, cpu.ebp + 0x18));
    fmul(cpu, f64(app, 0x5367b4));  // 1/6
    cpu.eax = fistpPop(app, cpu);
    cpu.edx = cpu.eax;
    if (sreg32(cpu.eax) < sreg32(cpu.ebx))
        cpu.edx = cpu.ebx;
    if (sreg32(cpu.edx) > sreg32(cpu.esi))
        cpu.edx = cpu.esi;
    if (at(app, cpu.ecx + 0x7d4) == 1)
        cpu.edx = reg32(0) - cpu.edx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, nodeOffset)) return;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x40e200: node eax's lane bits (the high half of its dword at +0xa) masked
 * by edx -- which of the 16 lanes are road there. */
void nodeLaneBits(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    cpu.edx = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kNodes);
    cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xa);
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
    cpu.eax &= cpu.ecx;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e220: whether node eax has road in slice edx (0..13, clamped to the 16
 * lane bits) -- nodeLaneBits with that slice's bit. */
void nodeHasLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.eax = 7;
    cpu.eax -= cpu.edx;
    cpu.edx = 8;
    cpu.edx -= cpu.eax;
    cpu.eax = cpu.edx;
    if (sreg32(cpu.edx) < 0)
        cpu.eax ^= cpu.edx;
    else if (sreg32(cpu.edx) >= 0x10)
        cpu.eax = 0xf;
    cpu.edx = 0xf;
    cpu.edx -= cpu.eax;
    cpu.cl = cpu.dl;
    cpu.edx = 1;
    cpu.eax = cpu.ebx;
    cpu.edx <<= (cpu.cl & 31);
    if (!direct(app, cpu, nodeLaneBits)) return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e270: the lane bit of slice eax (bit 15 - (slice + 1), clamped to the
 * 16 lanes). */
void laneBit(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 7;
    cpu.edx -= cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = 8;
    cpu.edx -= cpu.eax;
    cpu.eax = cpu.edx;
    if (sreg32(cpu.edx) < 0)
        cpu.eax ^= cpu.edx;
    else if (sreg32(cpu.edx) >= 0x10)
        cpu.eax = 0xf;
    cpu.edx = 0xf;
    cpu.edx -= cpu.eax;
    cpu.eax = 1;
    cpu.cl = cpu.dl;
    cpu.eax <<= (cpu.cl & 31);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e2b0 (stdcall, 1 word: an offset across the road): the road slice
 * (0..13) the offset falls in at node eax -- from the centre slice (6 left of
 * it, 7 right), the offset over the half's width (+0x17 / +0x1b, as 1/65536)
 * by the lane-width table 0x56f0e0. */
void roadSliceIndex(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ecx = cpu.eax * 8;
    cpu.ecx += cpu.eax;
    fldz(cpu);
    cpu.ecx <<= 2;
    if (!fcomp(cpu, f32(app, cpu.ebp + 0x10)).belowOrEqual())
    {
        cpu.eax = at(app, kNodes);
        cpu.edx = 6;
        cpu.al = byte(app, cpu.ecx + cpu.eax + 0x17);
    }
    else
    {
        cpu.eax = at(app, kNodes);
        cpu.edx = 7;
        cpu.al = byte(app, cpu.ecx + cpu.eax + 0x1b);
    }
    cpu.eax &= 0xff;
    cpu.eax <<= 0xf;
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, f64(app, 0x5367bc));  // 1/65536
    fstp32(app, cpu, cpu.ebp - 8);
    fld32(app, cpu, cpu.ebp - 8);
    fmul(cpu, f32(app, 0x5367c4));  // 4.0
    cpu.eax = fistpPop(app, cpu);
    fld32(app, cpu, cpu.eax * 4 + kLaneWidths);
    fmul(cpu, f32(app, cpu.ebp + 0x10));
    cpu.eax = fistpPop(app, cpu);
    cpu.edx += cpu.eax;
    cpu.eax = sreg32(cpu.edx) < 0 ? 0 : cpu.edx;
    if (sreg32(cpu.eax) > 0xd)
        cpu.eax = 0xd;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x40e350: the car eax's three road slices -- of its place across the road
 * +0x7b8 (+0x638), and of its two sides, +-0x21c less half a slice (+0x63c,
 * +0x640) -- at its node +0x1c (roadSliceIndex). */
void carSlices(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    push(app, cpu, at(app, cpu.eax + 0x7b8));
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, roadSliceIndex)) return;
    cpu.esp -= 4;
    fld32(app, cpu, cpu.edx + 0x7b8);
    fsub(cpu, f32(app, cpu.edx + 0x21c));
    put(app, cpu.edx + 0x638, cpu.eax);
    fadd(cpu, f64(app, 0x5367cc));  // 0.5
    cpu.eax = cpu.ecx;
    fstp32(app, cpu, cpu.esp);
    if (!direct(app, cpu, roadSliceIndex)) return;
    cpu.esp -= 4;
    fld32(app, cpu, cpu.edx + 0x7b8);
    fadd(cpu, f32(app, cpu.edx + 0x21c));
    put(app, cpu.edx + 0x63c, cpu.eax);
    fadd(cpu, f64(app, 0x5367d4));  // -0.5
    cpu.eax = cpu.ecx;
    fstp32(app, cpu, cpu.esp);
    if (!direct(app, cpu, roadSliceIndex)) return;
    put(app, cpu.edx + 0x640, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40e3c0 (stdcall, 4 words): a track's tuning record at eax (0x20 bytes):
 * edx and ebx (0xf and 0 in a knock-out race, [0x6fd4fc] bit 3), ecx, the four
 * words to +0xc, +0x14, +0x18, +0x1c. */
void fillTrackTuning(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if ((byte(app, kLoadFlags) & 8) != 0)
    {
        put(app, cpu.eax + 4, 0);
        put(app, cpu.eax, 0xf);
    }
    else
    {
        put(app, cpu.eax + 4, cpu.ebx);
        put(app, cpu.eax, cpu.edx);
    }
    cpu.edx = at(app, cpu.ebp + 8);
    put(app, cpu.eax + 8, cpu.ecx);
    put(app, cpu.eax + 0xc, cpu.edx);
    cpu.edx = at(app, cpu.ebp + 0xc);
    put(app, cpu.eax + 0x14, cpu.edx);
    cpu.edx = at(app, cpu.ebp + 0x10);
    put(app, cpu.eax + 0x18, cpu.edx);
    cpu.edx = at(app, cpu.ebp + 0x14);
    put(app, cpu.eax + 0x1c, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4 + 0x10;
}

/* 0x40e410: the race's opponent tuning set up.  A small handicap (-0.005 for
 * each of night, the network race, weather, the reversed race) is summed; the
 * lane-width table 0x56f0e0 filled (1 / (i / 4)); then by the race type
 * [0x6fd3b8] (a jump table, 0x40e400) each of the nine tracks' tuning records
 * at 0x56ef80 is filled (fillTrackTuning) -- fixed values for the single and
 * the cop races, the handicap-scaled ones for the variant [0x6fd470] 1 of the
 * other two -- one record overridden by an option [0x55eb2e] bit 5; the
 * opponents' speed table loaded (0x40e000) and its highest speed kept in
 * [0x56f224]. */
void setupTrackTuning(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x50;
    const reg32 m4 = cpu.ebp - 4, m8 = cpu.ebp - 8, mc = cpu.ebp - 0xc, m10 = cpu.ebp - 0x10;
    const reg32 m14 = cpu.ebp - 0x14, m18 = cpu.ebp - 0x18, m1c = cpu.ebp - 0x1c, m20 = cpu.ebp - 0x20;
    const reg32 m40 = cpu.ebp - 0x40, m50 = cpu.ebp - 0x50;
    constexpr reg32 kTop = 0x4156930c;
    // One record: the four words pushed last to first, the registers it takes.
    const auto entry = [&](reg32 record, reg32 ebx, reg32 edx, reg32 s1, reg32 s2, reg32 s3, reg32 s4) {
        push(app, cpu, s4);
        push(app, cpu, s3);
        push(app, cpu, s2);
        push(app, cpu, s1);
        cpu.eax = record;
        cpu.ebx = ebx;
        cpu.edx = edx;
        cpu.ecx = 0;
        return direct(app, cpu, fillTrackTuning);
    };
    cpu.edx = 0;
    cpu.ecx = at(app, kSplitScreen);
    put(app, m18, cpu.edx);
    if (cpu.ecx != 0)
        put(app, m18, 0xbba3d70a);
    for (const reg32 flag : {kNetworkRace, kWeatherNear, kBackwards})
        if (at(app, flag) != 0)
        {
            fld32(app, cpu, m18);
            fadd(cpu, f32(app, 0x5367dc));  // -0.005
            fstp32(app, cpu, m18);
        }
    cpu.edx = 0;
    cpu.eax = 1;
    put(app, kLaneWidths, cpu.edx);
    for (; sreg32(cpu.eax) < 0x50;)
    {
        put(app, m4, cpu.eax);
        fild(app, cpu, m4);
        fmul(cpu, f32(app, 0x5367e0));  // 0.25
        fld1(cpu);
        cpu.fpu.st(1) = fp::div(cpu.fpu.st(0), Float(cpu.fpu.st(1)));  // fdivrp st(1)
        cpu.fpu.pop();
        ++cpu.eax;
        fstp32(app, cpu, cpu.eax * 4 + 0x56f0dc);
    }
    cpu.eax = at(app, kRaceType);
    if (cpu.eax <= 3)
    {
        if (cpu.eax == 0)
        {
            static const reg32 rows[9][5] = {
                {0x56ef80, 0x28, 8, 0x3f51eb85, 0x3f70a3d7}, {0x56efa0, 0x78, 6, 0x3f570a3d, 0x3f75c28f},
                {0x56efc0, 0x50, 9, 0x3f570a3d, 0x3f75c28f}, {0x56efe0, 0x64, 5, 0x3f51eb85, 0x3f70a3d7},
                {0x56f000, 0x28, 7, 0x3f570a3d, 0x3f75c28f}, {0x56f020, 0x78, 6, 0x3f51eb85, 0x3f70a3d7},
                {0x56f040, 0x50, 5, 0x3f570a3d, 0x3f75c28f}, {0x56f060, 0x50, 5, 0x3f51eb85, 0x3f70a3d7},
                {0x56f080, 0x14, 0xc, 0x3f570a3d, 0x3f75c28f}};
            for (const auto& r : rows)
                if (!entry(r[0], r[1], r[2], 1, r[3], r[4], kTop)) return;
        }
        else if (cpu.eax == 3)
        {
            static const reg32 rows[9][5] = {
                {0x56ef80, 0x28, 8, 0x3f47ae14, 0x3f6147ae}, {0x56efa0, 0x78, 6, 0x3f51eb85, 0x3f666666},
                {0x56efc0, 0x50, 9, 0x3f4ccccd, 0x3f59999a}, {0x56efe0, 0x64, 5, 0x3f4ccccd, 0x3f59999a},
                {0x56f000, 0x28, 7, 0x3f4ccccd, 0x3f59999a}, {0x56f020, 0x78, 6, 0x3f4ccccd, 0x3f59999a},
                {0x56f040, 0x50, 5, 0x3f4ccccd, 0x3f59999a}, {0x56f060, 0x50, 5, 0x3f4ccccd, 0x3f59999a},
                {0x56f080, 0x14, 0xc, 0x3f4ccccd, 0x3f547ae1}};
            for (const auto& r : rows)
                if (!entry(r[0], r[1], r[2], 1, r[3], r[4], kTop)) return;
        }
        else
        {
            // 2 (0x40e770, the variant in ecx) or 1 (0x40e9a1, in ebx).
            const bool two = cpu.eax == 2;
            const reg32 variant = at(app, kRaceVariant);
            if (two)
                cpu.ecx = variant;
            else
                cpu.ebx = variant;
            if (variant == 1)
            {
                const reg32 x = two ? m40 : m50, a = two ? m20 : m8, b = two ? mc : m1c;
                fld32(app, cpu, m18);
                fst64(app, cpu, x);
                fadd(cpu, f64(app, 0x5367e4));  // 0.98
                fstp32(app, cpu, a);
                if (!entry(0x56ef80, 0, 0, 1, 0, at(app, a), kTop)) return;
                fld64(app, cpu, x);
                fadd(cpu, f64(app, 0x5367ec));  // 0.99
                fstp32(app, cpu, b);
                if (!entry(0x56efa0, 0, 0, 1, 0, at(app, b), kTop)) return;
                fld1(cpu);
                fadd(cpu, f64(app, x));
                reg32 c;
                if (two)
                {
                    fstp32(app, cpu, m10);
                    c = at(app, m10);
                }
                else
                {
                    // sub esp, 4; fstp dword [esp]: straight into the word pushed.
                    const float f = float(cpu.fpu.st(0));
                    std::memcpy(&c, &f, 4);
                    cpu.fpu.pop();
                }
                if (!entry(0x56efc0, 0, 0, 1, 0, c, kTop)) return;
                const reg32 last = two ? at(app, m10) : at(app, b);
                if (!entry(0x56efe0, 0, 0, 1, 0, at(app, a), kTop)) return;
                if (!entry(0x56f000, 0, 0, 1, 0, at(app, b), kTop)) return;
                if (!entry(0x56f020, 0, 0, 1, 0, at(app, a), kTop)) return;
                if (!entry(0x56f040, 0, 0, 1, 0, at(app, b), kTop)) return;
                if (!entry(0x56f060, 0, 0, 1, 0, at(app, a), kTop)) return;
                if (!entry(0x56f080, 0, 0, 1, 0, last, kTop)) return;
            }
            else
            {
                static const reg32 sTwo[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                              0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f800000};
                static const reg32 sOne[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                              0x3f6b851f, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7};
                for (reg32 i = 0; i < 9; ++i)
                    if (!entry(0x56ef80 + 0x20 * i, 0, 0, 1, 0, two ? sTwo[i] : sOne[i], kTop)) return;
            }
        }
    }
    if ((byte(app, kStartFlags) & 0x20) != 0)
        if (!entry(0x56efe0, 0x64, 1, 1, 0x3f666666, 0x3f800000, kTop)) return;
    if (!byAddress(app, cpu, 0x40e000)) return;  // load::loadOpponentSpeeds
    cpu.ecx = at(app, kNodeCount);
    cpu.esi = 0;
    cpu.edx = 0;
    put(app, kTrackValid, cpu.esi);
    for (; sreg32(cpu.edx) < sreg32(cpu.ecx); ++cpu.edx)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, targetSpeedAt)) return;
        fst32(app, cpu, m14);
        if (!fcomp(cpu, f32(app, kTrackValid)).belowOrEqual())
        {
            cpu.eax = at(app, m14);
            put(app, kTrackValid, cpu.eax);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ec40: the opponents' speed table [0x552558] freed (0x4e1890) and
 * forgotten. */
void freeSpeedTable(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kSpeeds);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kFreeHandle)) return;
        put(app, kSpeeds, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ec70: the track's traffic side (its tuning record's +0xc; 1 past the
 * nine tracks). */
void trafficSide(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kTrackNumber);
    if (sreg32(cpu.edx) < 0xb)
    {
        cpu.eax = cpu.edx << 5;
        cpu.eax = at(app, cpu.eax + 0x56ef8c);
    }
    else
        cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40eca0: whether the track's road is wide (its record's +8; past the nine
 * tracks, whether the second node's left edge slice is 0). */
void roadIsWide(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kTrackNumber);
    if (sreg32(cpu.edx) < 0xb)
    {
        cpu.eax = cpu.edx << 5;
        cpu.eax = at(app, cpu.eax + 0x56ef88);
    }
    else
    {
        cpu.eax = at(app, kNodes);
        cpu.al = byte(app, cpu.eax + 0x37);
        cpu.eax &= 0xff;
        cpu.eax = reg32(sreg32(cpu.eax) >> 4);
        cpu.eax = (cpu.al & 0xf) == 0 ? 1 : 0;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ed40: the traffic's speed at node eax onto the x87's stack, by the
 * track [0x6fd4ac] (the jump table at 0x40ed14) and the stretch of it --
 * slower through each track's hard stretches. */
void trafficSpeedAt(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edx = at(app, kTrackNumber);
    const reg32 n = cpu.eax;
    reg32 v = 0x41a0ee49;
    if (cpu.edx <= 0xa)
        switch (cpu.edx)
        {
        case 0: v = n > 0x276 || n <= 0x1e8 ? 0x41b2cfdf : 0x4156930c; break;
        case 1: v = n > 0x3e8 ? 0x41c4b176 : n > 0x316 ? 0x41fa5639 : n > 0xaa ? 0x41e874a2 : 0x41c4b176; break;
        case 2: v = 0x41b2cfdf; break;
        case 3: v = n > 0x56e || n <= 0x528 ? 0x41b1c77a : 0x415e3958; break;
        case 4: v = n > 0x153 || n <= 0x127 ? 0x41b2cfdf : 0x410f0cb3; break;
        case 5: case 6: v = 0x41b2cfdf; break;
        case 7: v = n > 0x65e || n <= 0x60d ? 0x41b1c77a : 0x415e3958; break;
        case 8: v = n > 0x3e8 || n <= 0x190 ? 0x41a0ee49 : 0x417a5639; break;
        default: break;  // 9, 10
        }
    put(app, cpu.ebp - 4, v);
    fld32(app, cpu, cpu.ebp - 4);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ee80: the track's tuning speed onto the x87's stack: its record's +0x18
 * in the race types 1 and 2 or with a second player [0x6fd4f0], else +0x14.
 * Keeps every register. */
void tuningSpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edx = at(app, kRaceType);
    if (cpu.edx == 1 || cpu.edx == 2)
    {
        cpu.eax = at(app, kTrackNumber) << 5;
        cpu.eax = at(app, cpu.eax + 0x56ef98);
    }
    else
    {
        cpu.eax = at(app, kTrackNumber);
        cpu.ebx = at(app, kSecondPlayer);
        cpu.eax <<= 5;
        cpu.eax = at(app, cpu.eax + (cpu.ebx != 0 ? 0x56ef98 : 0x56ef94));
    }
    put(app, cpu.ebp - 4, cpu.eax);
    fld32(app, cpu, cpu.ebp - 4);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.eax = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40eed0: the track's tuning distance (its record's +4), longer in a
 * network race (+200) and a split screen (+400). */
void tuningDistance(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kTrackNumber);
    cpu.eax <<= 5;
    cpu.edx = at(app, kNetworkRace);
    cpu.eax = at(app, cpu.eax + 0x56ef84);
    if (cpu.edx != 0)
        cpu.eax += 0xc8;
    if (at(app, kSplitScreen) != 0)
        cpu.eax += 0x190;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ef10: the track's tuning record's +0. */
void tuningFirst(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kTrackNumber);
    cpu.eax <<= 5;
    cpu.eax = at(app, cpu.eax + kTuning);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40ef70: the dot product of nodes eax's and edx's forward directions (their
 * signed bytes at +0x14 and +0x16, << 9, as 1/65536) onto the x87's stack. */
void nodeDirectionDot(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 m4 = cpu.ebp - 4;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx * 8;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.ebx = cpu.edx + cpu.eax;
    cpu.eax = reg32(sreg32(at(app, cpu.ebx + 0x11)) >> 0x18);
    cpu.eax <<= 9;
    put(app, m4, cpu.eax);
    cpu.eax = cpu.ecx;
    cpu.eax <<= 3;
    cpu.eax += cpu.ecx;
    cpu.eax <<= 2;
    fild(app, cpu, m4);
    cpu.edx += cpu.eax;
    fld64(app, cpu, 0x5367f4);  // 1/65536
    fxch(cpu, 1);
    fmulSt0(cpu, 1);
    cpu.eax = reg32(sreg32(at(app, cpu.edx + 0x11)) >> 0x18);
    cpu.eax <<= 9;
    put(app, m4, cpu.eax);
    cpu.eax = at(app, cpu.ebx + 0x13);
    fild(app, cpu, m4);
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
    fmulSt0(cpu, 2);
    cpu.eax <<= 9;
    fmulp(cpu, 1);
    put(app, m4, cpu.eax);
    cpu.eax = at(app, cpu.edx + 0x13);
    fild(app, cpu, m4);
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
    fmulSt0(cpu, 2);
    cpu.eax <<= 9;
    put(app, m4, cpu.eax);
    fild(app, cpu, m4);
    fmulp(cpu, 3);
    fmulp(cpu, 2);
    faddp(cpu, 1);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f000: whether node eax lies in the run of ebx nodes from node edx
 * (wrapped round the track; a negative run counts back). */
void nodeWithinRange(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = at(app, kNodeCount);
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx + cpu.ebx;
    if (sreg32(cpu.ebx) >= 0)
    {
        if (sreg32(cpu.eax) >= sreg32(cpu.esi))
            cpu.eax -= cpu.esi;
    }
    else if (sreg32(cpu.eax) < 0)
        cpu.eax += cpu.esi;
    bool in;
    if (cpu.eax < cpu.edx)
        in = cpu.ecx >= cpu.edx || cpu.ecx <= cpu.eax;
    else
        in = cpu.ecx >= cpu.edx && cpu.ecx <= cpu.eax;
    cpu.eax = in ? 1 : 0;
    put(app, kNodeCount, cpu.esi);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f050: whether the car eax is within reach of distance edx -- its place
 * +8 plus 14 times its speed +0x1fc against edx and a margin by the race's
 * laps [0x5efd94] (less in a reversed race). */
void carWithinReach(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.ecx = at(app, 0x5efd94);
    cpu.edx = 0x190;
    if (sreg32(cpu.ecx) < 5)
        cpu.edx = 0xc8;
    if (sreg32(at(app, 0x5efd94)) < 2)
        cpu.edx = 0x32;
    if (at(app, kBackwards) != 0)
        cpu.edx -= 0x1a;
    cpu.eax = at(app, cpu.ebx + 0x1fc);
    cpu.eax += cpu.eax;
    cpu.ecx = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax -= cpu.ecx;
    cpu.ecx = at(app, cpu.ebx + 8);
    cpu.edx += cpu.esi;
    cpu.eax += cpu.ecx;
    cpu.eax = sreg32(cpu.eax) > sreg32(cpu.edx) ? 0 : 1;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f100: 1. */
void alwaysOne(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f110: whether the car eax is boxed in: 1 when its slice +0x638 is off the
 * 14 lanes or is not road at its node, or neither neighbouring slice is road
 * (within the node's edge slices and the lanes); else 0. */
void carBoxedIn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = at(app, kNodes);
    cpu.edx = at(app, cpu.eax + 0x638);
    cpu.ecx = at(app, cpu.eax + 0x1c);
    const reg32 node = cpu.edi + cpu.ecx * 0x24;
    const i32 b13 = byte(app, node + 0x13);
    const i32 edgeLo = 6 - ((b13 >> 4) & 0xf), edgeHi = (b13 & 0xf) + 7;
    const i32 slice = sreg32(cpu.edx);
    bool boxed = true;
    if (slice >= 0 && slice < 0xe)
    {
        const auto free = [&](i32 s) { return s >= edgeLo && s <= edgeHi && s >= 0 && s < 0xe; };
        if (!(edgeLo > slice) && edgeHi >= slice)
            boxed = false;
        else if (free(slice - 1) || free(slice + 1))
            boxed = false;
    }
    cpu.edx = boxed ? 1 : 0;
    cpu.eax = cpu.edx;
    put(app, kNodes, cpu.edi);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* The track's animation paths (.can, loaded into the seven handles at
 * 0x570a34) and their players, 256 slots of 0x18 bytes at 0x56f234: +0 the
 * path playing, +4 the clock it started at, +8 a second mark, +0xc its mode,
 * +0x10 its flags (bit 0 mirrored), +0x14 a third word.  A path: +4 its
 * points' count, +6 the frames per point (1..400, else 6), from +8 its points
 * of 0x14 bytes -- a place of three fixed-point dwords and an orientation of
 * four shorts (+0xc). */
constexpr reg32 kAnimStart = 0x56f238, kAnimMark = 0x56f23c, kAnimMode = 0x56f240, kAnimFlags = 0x56f244;
constexpr reg32 kAnimThird = 0x56f248;

/* 0x40f330: the seven animation paths freed (0x4e1890) and forgotten. */
void freeAnimationPaths(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (cpu.edx = 0; sreg32(cpu.edx) < 7; ++cpu.edx)
    {
        cpu.ecx = cpu.edx * 4;
        cpu.ebx = at(app, cpu.ecx + kAnimPaths);
        if (cpu.ebx == 0)
            continue;
        cpu.eax = cpu.ebx;
        cpu.esi = 0;
        if (!byAddress(app, cpu, kFreeHandle)) return;
        put(app, cpu.ecx + kAnimPaths, cpu.esi);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f370: the first free animation slot, or -1. */
void findFreeAnimation(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (cpu.edx = 0; sreg32(cpu.edx) < 0x100; ++cpu.edx)
    {
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        if (at(app, cpu.eax * 8 + kAnims) == 0)
            break;
    }
    cpu.eax = cpu.edx == 0x100 ? 0xffffffff : cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f3b0: path eax (0..6 of the handles) started in a free slot at the
 * clock, mode 2, flags 1 -- the slot, or -1 when none is free or the path is
 * not loaded. */
void startAnimation(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (!direct(app, cpu, findFreeAnimation)) return;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0xffffffff)
    {
        if (sreg32(cpu.ecx) >= 0x100)
            cpu.eax = 0xffffffff;
        else
        {
            cpu.eax <<= 2;
            cpu.eax -= cpu.edx;
            cpu.ebx = cpu.ecx * 4;
            cpu.eax <<= 3;
            cpu.ecx = at(app, cpu.ebx + kAnimPaths);
            put(app, cpu.eax + kAnims, cpu.ecx);
            if (cpu.ecx == 0)
                cpu.eax = 0xffffffff;
            else
            {
                cpu.ecx = at(app, kClock);
                cpu.ebx = 2;
                put(app, cpu.eax + kAnimStart, cpu.ecx);
                cpu.esi = 1;
                put(app, cpu.eax + kAnimMode, cpu.ebx);
                cpu.edi = 0;
                put(app, cpu.eax + kAnimFlags, cpu.esi);
                put(app, cpu.eax + kAnimThird, cpu.edi);
                cpu.eax = cpu.edx;
            }
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

/* 0x40f430: slot eax's mode, flags and third word set from edx, ebx, ecx. */
void setAnimationMode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.edx;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    put(app, cpu.eax * 8 + kAnimMode, cpu.esi);
    put(app, cpu.eax * 8 + kAnimFlags, cpu.ebx);
    put(app, cpu.eax * 8 + kAnimThird, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f460: slot eax stopped -- 1, or -1 when it was not playing. */
void stopAnimation(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 3;
    if (at(app, cpu.eax + kAnims) == 0)
        cpu.eax = 0xffffffff;
    else
    {
        cpu.ebx = 0xffffffff;
        cpu.ecx = 0;
        put(app, cpu.eax + kAnimStart, cpu.ebx);
        put(app, cpu.eax + kAnimMark, cpu.ecx);
        put(app, cpu.eax + kAnims, cpu.ecx);
        cpu.eax = 1;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f4b0 (stdcall, 1 word: where the pose's matrix goes): path eax's pose at
 * time ebx (in frames, edx the mode: 2 plays once).  The two points the time
 * falls between (looping over the points, unless played once and past the
 * end -- then 0) give two orientations (four shorts, << 2, as 1/65536) and two
 * places (fixed point, as 1/65536); their blend at the fraction of the frame
 * (0x4b70f0, ecx handed on) is made a matrix at the word (0x4b7590).  1. */
void animationPose(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x7c;
    const auto m = [&](reg32 off) { return cpu.ebp - off; };
    const auto shortAt = [&](reg32 off) {  // mov eax, [ebp-off]; sar eax, 16; shl eax, 2
        return reg32(sreg32(at(app, m(off))) >> 0x10) << 2;
    };
    cpu.esi = cpu.eax;
    put(app, m(0x1c), cpu.edx);
    put(app, m(0x20), cpu.ecx);
    cpu.eax += 8;
    cpu.dx = app->getMemory<reg16>(cpu.esi + 6);
    put(app, m(0x10), cpu.eax);
    if (cpu.dx >= 1 && cpu.dx <= 0x190)
    {
        cpu.edi = 0;
        cpu.di = cpu.dx;
    }
    else
        cpu.edi = 6;
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ebx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.edi);
    cpu.edx = at(app, m(0x1c));
    cpu.ecx = cpu.eax;
    if (cpu.edx == 2)
    {
        cpu.eax = 0;
        cpu.ax = app->getMemory<reg16>(cpu.esi + 4);
        --cpu.eax;
        if (sreg32(cpu.ecx) >= sreg32(cpu.eax))
        {
            cpu.eax = 0;
            goto leave;
        }
    }
    cpu.eax = 0;
    cpu.ax = app->getMemory<reg16>(cpu.esi + 4);
    cpu.edx = cpu.ecx;
    cpu.esi = cpu.eax - 1;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    cpu.eax = cpu.ecx;
    idiv(cpu, cpu.esi);
    cpu.ecx = cpu.edx;
    cpu.eax = cpu.edx + 1;
    put(app, m(0x14), cpu.eax);
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ebx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, m(0x10));
    cpu.eax <<= 2;
    cpu.ecx += cpu.eax;
    put(app, m(8), cpu.edi);
    cpu.edi = m(0x28);
    cpu.esi = cpu.ecx + 0xc;
    movsd(app, cpu); movsd(app, cpu);
    // The first orientation, its four shorts as 1/65536.
    put(app, m(4), shortAt(0x2a));
    fild(app, cpu, m(4));
    fld64(app, cpu, 0x536814);
    fxch(cpu, 1);
    fmulSt0(cpu, 1);
    put(app, m(4), shortAt(0x28));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 2);
    put(app, m(4), shortAt(0x26));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 3);
    put(app, m(4), shortAt(0x24));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 4);
    cpu.ebx = at(app, m(0x10));
    put(app, m(0xc), cpu.edx);
    cpu.edx = at(app, m(0x14));
    cpu.edi = m(0x28);
    cpu.eax = cpu.edx * 4;
    fxch(cpu, 3);
    fstp32(app, cpu, m(0x5c));
    cpu.eax += cpu.edx;
    fxch(cpu, 1);
    fstp32(app, cpu, m(0x58));
    cpu.eax <<= 2;
    fstp32(app, cpu, m(0x54));
    cpu.ebx += cpu.eax;
    fstp32(app, cpu, m(0x50));
    cpu.esi = cpu.ebx + 0xc;
    movsd(app, cpu); movsd(app, cpu);
    // The second orientation, and the fraction of the frame.
    put(app, m(4), shortAt(0x2a));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 1);
    cpu.eax = at(app, m(0x28));
    fild(app, cpu, m(0xc));
    fild(app, cpu, m(8));
    fdivp(cpu, 1);
    put(app, m(4), shortAt(0x28));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 3);
    put(app, m(4), shortAt(0x26));
    fild(app, cpu, m(4));
    fmulSt0(cpu, 4);
    put(app, m(4), shortAt(0x24));
    cpu.eax = reg32(sreg32(at(app, m(0x24))) >> 0x10) << 2;
    fild(app, cpu, m(4));
    fmulSt0(cpu, 5);
    cpu.edi = m(0x40);
    cpu.esi = cpu.ecx;
    fxch(cpu, 4);
    fstp32(app, cpu, m(0x6c));
    fxch(cpu, 1);
    fstp32(app, cpu, m(0x68));
    fstp32(app, cpu, m(0x64));
    fxch(cpu, 1);
    fstp32(app, cpu, m(0x60));
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    // The first place.
    cpu.eax = at(app, m(0x40));
    put(app, m(4), cpu.eax);
    fild(app, cpu, m(4));
    cpu.eax = at(app, m(0x3c));
    fmulSt0(cpu, 2);
    put(app, m(4), cpu.eax);
    cpu.eax = at(app, m(0x38));
    fild(app, cpu, m(4));
    put(app, m(4), cpu.eax);
    fmulSt0(cpu, 3);
    fild(app, cpu, m(4));
    fmulp(cpu, 4);
    fxch(cpu, 2);
    fstp32(app, cpu, m(0x18));
    fstp32(app, cpu, m(0x4c));
    fstp32(app, cpu, m(0x48));
    fstp32(app, cpu, m(0x44));
    cpu.edi = m(0x40);
    cpu.esi = cpu.ebx;
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    // The second place.
    cpu.eax = at(app, m(0x40));
    put(app, m(4), cpu.eax);
    fild(app, cpu, m(4));
    cpu.eax = at(app, m(0x3c));
    fld64(app, cpu, 0x536814);
    fxch(cpu, 1);
    fmulSt0(cpu, 1);
    put(app, m(4), cpu.eax);
    cpu.eax = at(app, m(0x38));
    fild(app, cpu, m(4));
    put(app, m(4), cpu.eax);
    fmulSt0(cpu, 2);
    fild(app, cpu, m(4));
    fmulp(cpu, 3);
    cpu.ecx = at(app, m(0x20));
    cpu.edx = m(0x6c);
    cpu.ebx = m(0x4c);
    push(app, cpu, cpu.ecx);
    cpu.ecx = m(0x34);
    cpu.eax = m(0x7c);
    fxch(cpu, 1);
    fstp32(app, cpu, m(0x34));
    push(app, cpu, cpu.eax);
    fstp32(app, cpu, m(0x30));
    push(app, cpu, at(app, m(0x18)));
    cpu.eax = m(0x5c);
    fstp32(app, cpu, m(0x2c));
    if (!byAddress(app, cpu, kPoseBlend)) return;
    cpu.edx = at(app, cpu.ebp + 0x10);
    cpu.eax = m(0x7c);
    if (!byAddress(app, cpu, kPoseMatrix)) return;
    cpu.eax = 1;
leave:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x40f6d0 (stdcall, 2 words: where the time into the path goes, where its
 * length goes): path eax's place at time ebx (edx bit 1: played once -- 0
 * past the end): the two points' places blended at the frame's 16.16 fraction
 * (0x4b6a30, ecx handed on); the time within the path's length and the
 * length (both in frames) to the two words.  1. */
void animationPlace(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x44;
    const auto m = [&](reg32 off) { return cpu.ebp - off; };
    put(app, m(4), cpu.eax);
    cpu.esi = cpu.edx;
    put(app, m(0xc), cpu.ebx);
    put(app, m(0x10), cpu.ecx);
    cpu.dx = app->getMemory<reg16>(cpu.eax + 6);
    cpu.ebx = cpu.eax + 8;
    if (cpu.dx >= 1 && cpu.dx <= 0x190)
    {
        cpu.edx = at(app, m(4));
        cpu.eax = 0;
        cpu.ax = app->getMemory<reg16>(cpu.edx + 6);
        put(app, m(8), cpu.eax);
    }
    else
        put(app, m(8), 6);
    cpu.eax = at(app, m(0xc));
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, m(8));
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
    cpu.ecx = cpu.eax;
    cpu.esi &= 2;
    if (cpu.esi != 0)
    {
        cpu.edx = at(app, m(4));
        cpu.eax = 0;
        cpu.ax = app->getMemory<reg16>(cpu.edx + 4);
        --cpu.eax;
        if (sreg32(cpu.ecx) >= sreg32(cpu.eax))
        {
            cpu.edx = 0;
            goto leave;
        }
    }
    cpu.edx = at(app, m(4));
    cpu.eax = 0;
    cpu.ax = app->getMemory<reg16>(cpu.edx + 4);
    cpu.esi = cpu.eax - 1;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ecx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.esi);
    cpu.ecx = cpu.edx;
    cpu.eax = cpu.edx + 1;
    put(app, m(0x14), cpu.eax);
    cpu.eax = at(app, m(0xc));
    cpu.edx = cpu.eax;
    cpu.esi = at(app, m(8));
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.esi);
    cpu.edx <<= 0x10;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.esi);
    cpu.edx = cpu.ecx;
    cpu.ecx <<= 2;
    cpu.ecx += cpu.edx;
    cpu.ecx <<= 2;
    cpu.edi = m(0x44);
    cpu.esi = cpu.ebx + cpu.ecx;
    cpu.edx = at(app, m(0x14));
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    cpu.ecx = cpu.edx * 4;
    cpu.edi = m(0x38);
    cpu.ecx += cpu.edx;
    cpu.edx = at(app, m(0x10));
    cpu.ecx <<= 2;
    push(app, cpu, cpu.edx);
    cpu.esi = cpu.ebx + cpu.ecx;
    cpu.edx = m(0x2c);
    cpu.ecx = m(0x38);
    push(app, cpu, cpu.edx);
    cpu.ebx = m(0x44);
    cpu.edx = m(0x24);
    push(app, cpu, cpu.eax);
    cpu.eax = m(0x1c);
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    if (!byAddress(app, cpu, kPlaceBlend)) return;
    cpu.edx = at(app, m(4));
    cpu.eax = 0;
    cpu.ax = app->getMemory<reg16>(cpu.edx + 4);
    cpu.ecx = at(app, m(8));
    --cpu.eax;
    cpu.ecx = reg32(sreg32(cpu.ecx) * sreg32(cpu.eax));
    cpu.eax = at(app, m(0xc));
    cpu.edx = cpu.eax;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
    cpu.eax = at(app, cpu.ebp + 0x10);
    put(app, cpu.eax, cpu.edx);
    cpu.edx = at(app, m(4));
    cpu.eax = 0;
    cpu.ax = app->getMemory<reg16>(cpu.edx + 4);
    cpu.edx = at(app, m(8));
    --cpu.eax;
    cpu.edx = reg32(sreg32(cpu.edx) * sreg32(cpu.eax));
    cpu.eax = at(app, cpu.ebp + 0x14);
    put(app, cpu.eax, cpu.edx);
    cpu.edx = 1;
leave:
    cpu.eax = cpu.edx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x40f800: slot eax's progress -- its frame (the clock since it started, by
 * the path's frames per point) within the path's length to [edx], the length
 * (its points less one) to [ebx] and eax. */
void animationProgress(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.ecx = cpu.edx;
    cpu.esi = cpu.ebx;
    cpu.edx = cpu.eax * 4;
    cpu.edx -= cpu.eax;
    cpu.ebx = cpu.edx * 8;
    cpu.edx = at(app, kClock);
    cpu.edi = at(app, cpu.ebx + kAnimStart);
    cpu.ebx = at(app, cpu.ebx + kAnims);
    cpu.edx -= cpu.edi;
    cpu.di = app->getMemory<reg16>(cpu.ebx + 6);
    if (cpu.di >= 1 && cpu.di <= 0x190)
    {
        cpu.edi = 0;
        cpu.di = app->getMemory<reg16>(cpu.ebx + 6);
    }
    else
        cpu.edi = 6;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.edi);
    cpu.edx = cpu.eax;
    cpu.eax = 0;
    cpu.ax = app->getMemory<reg16>(cpu.ebx + 4);
    --cpu.eax;
    put(app, cpu.ebp - 4, cpu.eax);
    cpu.edi = at(app, cpu.ebp - 4);
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.edi);
    cpu.eax = 0;
    put(app, cpu.ecx, cpu.edx);
    cpu.ax = app->getMemory<reg16>(cpu.ebx + 4);
    --cpu.eax;
    put(app, cpu.esi, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f880 (stdcall, 1 word: where the matrix goes): path eax's last point --
 * its place copied to [ecx], its orientation made a matrix (0x4b6ca0). */
void animationLastPose(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ebx = cpu.eax + 8;
    cpu.ax = app->getMemory<reg16>(cpu.eax + 4);
    cpu.eax &= 0xffff;
    cpu.edx = cpu.eax - 1;
    cpu.eax = cpu.edx * 4;
    cpu.eax += cpu.edx;
    cpu.eax <<= 2;
    cpu.eax += cpu.ebx;
    cpu.edi = cpu.ebp - 8;
    cpu.esi = cpu.eax + 0xc;
    movsd(app, cpu); movsd(app, cpu);
    cpu.edi = cpu.ecx;
    cpu.esi = cpu.eax;
    cpu.edx = at(app, cpu.ebp + 0x10);
    cpu.eax = cpu.ebp - 8;
    movsd(app, cpu); movsd(app, cpu); movsd(app, cpu);
    if (!byAddress(app, cpu, kQuatMatrix)) return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x40f8d0 (stdcall, 1 word: the pose): path eax's (a handle's index) end
 * pose (animationLastPose, its place to [ecx]); mirrored (the pose's first
 * two rows swapped, one negated) when ebx's bit 0 is set.  1. */
void pathEndPose(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.esi = at(app, cpu.ebp + 0xc);
    put(app, cpu.ebp - 4, cpu.ebx);
    push(app, cpu, cpu.esi);
    cpu.ebx = at(app, kTicks);
    cpu.eax = at(app, cpu.eax * 4 + kAnimPaths);
    if (!direct(app, cpu, animationLastPose)) return;
    if ((byte(app, cpu.ebp - 4) & 1) != 0)
    {
        cpu.edx = at(app, cpu.esi + 0x18);
        cpu.eax = at(app, cpu.esi + 0xc);
        put(app, cpu.esi + 0xc, cpu.edx);
        cpu.edx = cpu.eax;
        put(app, cpu.esi + 0x18, cpu.eax);
        cpu.edx = reg32(0) - cpu.edx;
        put(app, cpu.esi + 0x18, cpu.edx);
        cpu.eax = at(app, cpu.esi + 0x10);
        cpu.edx = at(app, cpu.esi + 0x1c);
        cpu.ecx = cpu.eax;
        put(app, cpu.esi + 0x10, cpu.edx);
        cpu.ecx = reg32(0) - cpu.ecx;
        put(app, cpu.esi + 0x1c, cpu.eax);
        put(app, cpu.esi + 0x1c, cpu.ecx);
        cpu.edx = at(app, cpu.esi + 0x20);
        cpu.eax = at(app, cpu.esi + 0x14);
        put(app, cpu.esi + 0x14, cpu.edx);
        cpu.ebx = cpu.eax;
        put(app, cpu.esi + 0x20, cpu.eax);
        cpu.ebx = reg32(0) - cpu.ebx;
        put(app, cpu.esi + 0x20, cpu.ebx);
    }
    cpu.eax = 1;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x40f940: slot eax stepped: -1 when it is not playing; its path's pose at
 * the clock since it started (animationPose, its mode, edx handed on, the
 * pose to ebx) -- when that is past the end the slot stops (0); mirrored by
 * its flags' bit 0 (the pose's first two rows swapped, one negated, as
 * floats).  1. */
void animationStep(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 m4 = cpu.ebp - 4, m8 = cpu.ebp - 8;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.ebx;
    if (!direct(app, cpu, animationPlaying)) return;
    if (cpu.eax != 1)
    {
        cpu.eax = 0xffffffff;
        goto leave;
    }
    cpu.eax = cpu.edi;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edi;
    cpu.edi = cpu.eax * 8;
    push(app, cpu, cpu.ebx);
    cpu.eax = at(app, kClock);
    cpu.ebx = at(app, cpu.edi + kAnimMode);
    cpu.ecx = at(app, cpu.edi + kAnimStart);
    put(app, m8, cpu.ebx);
    cpu.eax -= cpu.ecx;
    cpu.ecx = cpu.edx;
    cpu.ebx = at(app, cpu.edi + kAnims);
    cpu.edx = at(app, m8);
    put(app, m8, cpu.ebx);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, m8);
    if (!direct(app, cpu, animationPose)) return;
    if (cpu.eax == 0)
    {
        cpu.ebx = 0xffffffff;
        put(app, cpu.edi + kAnims, cpu.eax);
        put(app, cpu.edi + kAnimStart, cpu.ebx);
        goto leave;
    }
    if ((byte(app, cpu.edi + kAnimFlags) & 1) != 0)
    {
        fld32(app, cpu, cpu.esi + 0x18);
        fld32(app, cpu, cpu.esi + 0x1c);
        cpu.eax = at(app, cpu.esi + 0xc);
        fld32(app, cpu, cpu.esi + 0x20);
        put(app, m4, cpu.eax);
        fxch(cpu, 2);
        fstp32(app, cpu, cpu.esi + 0xc);
        fld32(app, cpu, m4);
        fchs(cpu);
        cpu.eax = at(app, cpu.esi + 0x10);
        fstp32(app, cpu, cpu.esi + 0x18);
        put(app, m4, cpu.eax);
        fstp32(app, cpu, cpu.esi + 0x10);
        fld32(app, cpu, m4);
        fchs(cpu);
        cpu.eax = at(app, cpu.esi + 0x14);
        fstp32(app, cpu, cpu.esi + 0x1c);
        put(app, m4, cpu.eax);
        fstp32(app, cpu, cpu.esi + 0x14);
        fld32(app, cpu, m4);
        fchs(cpu);
        fstp32(app, cpu, cpu.esi + 0x20);
    }
    cpu.eax = 1;
leave:
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40fac0: whether slot eax is playing. */
void animationPlaying(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    cpu.eax = at(app, cpu.eax * 8 + kAnims) != 0 ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* The sound library started for the speaker setup: from the caps (SNDcaps,
 * 0x4e21b0), stereo (0x808) for setup 0/0, 0x804 for 0/1, 0x802 for 1/0,
 * 0x801 for 1/1 -- with 0x20000 in a 3D setup [0x5525a4] -- or -1 when the
 * caps lack it (SNDinit, 0x4e2630).  Shared by both entry points below;
 * edx the result (left as it was for any other setup). */
namespace
{

bool initForSetup(App* app, CPU& cpu)
{
    static const reg32 wants[4] = {0x808, 0x804, 0x802, 0x801};
    int which = -1;
    if (cpu.esi == 0 && cpu.ecx == 0)
        which = 0;
    else if (cpu.esi == 0 && cpu.ecx == 1)
        which = 1;
    else if (cpu.esi == 1 && cpu.ecx == 0)
        which = 2;
    else if (cpu.esi == 1 && cpu.ecx == cpu.esi)
        which = 3;
    if (which < 0)
        return true;
    const reg32 want = wants[which];
    cpu.eax = cpu.ebx & want;
    if (cpu.eax != want)
    {
        cpu.edx = 0xffffffff;
        cpu.eax = cpu.edx;
    }
    else if (at(app, kSoundInitOK) != 0)
    {
        cpu.edx = 0xffffffff;
        cpu.eax = 0x20000 | want;
    }
    else
        cpu.edx = 0xffffffff;
    if (!byAddress(app, cpu, kSndInit))
        return false;
    cpu.edx = cpu.eax;
    return true;
}

/* The 3D setup's trace line ("3D sound", ea::gameTrace, cdecl). */
bool traceSetup(App* app, CPU& cpu)
{
    push(app, cpu, kVolume);
    push(app, cpu, 1);
    push(app, cpu, 0x64);
    if (!byAddress(app, cpu, kVolumeSet))
        return false;
    cpu.esp += 0xc;
    return true;
}

}  // namespace

/* 0x40fb10: the sound brought up for the options at the race's start: the
 * 23 voice records at 0x571124 cleared; the setup [0x6fbc00] (2 taken as 0, 3
 * the 3D setup [0x5525a4]) and the speakers [0x6fbbdc]; when effects or music
 * are on, the library started (initForSetup); failing that both are turned
 * off, else the rate set (SNDsetrate: 0, or -50 unless [0x6fbbe0]) and the
 * effect bus set up (port::busInit). */
void startSound(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (cpu.ecx = 0; sreg32(cpu.ecx) < 0x17;)
    {
        cpu.ebx = cpu.ecx;
        cpu.eax = cpu.ecx * 4;
        cpu.eax -= cpu.ecx;
        cpu.ebx ^= cpu.ecx;
        ++cpu.ecx;
        put(app, cpu.eax * 4 + kVoiceRecords, cpu.ebx);
    }
    cpu.ecx = at(app, kSoundMode);
    cpu.esi = at(app, kSoundSurround);
    if (cpu.ecx == 2)
        cpu.ecx = 0;
    cpu.edi = at(app, kSoundSetupKind);
    if (cpu.ecx == 3)
    {
        cpu.eax = 1;
        cpu.ecx = 0;
        put(app, kSoundInitOK, cpu.eax);
    }
    else
    {
        cpu.ebx = 0;
        put(app, kSoundInitOK, cpu.ebx);
    }
    if (at(app, kEffectsOn) != 0 || at(app, kMusicOn) != 0)
    {
        cpu.eax = 0xffffffff;
        if (!byAddress(app, cpu, kSndCaps)) return;
        cpu.ebx = cpu.eax;
        if (at(app, kSoundInitOK) != 0 && !traceSetup(app, cpu)) return;
        if (!initForSetup(app, cpu)) return;
        if (sreg32(cpu.edx) < 0)
        {
            cpu.edi = 0;
            put(app, kMusicOn, cpu.edi);
            put(app, kMusicSerial, cpu.edi);
            put(app, kEffectsOn, cpu.edi);
        }
        else
        {
            cpu.eax = cpu.edi != 0 ? 0 : 0xffffffce;
            if (!byAddress(app, cpu, kSndRate)) return;
            if (!byAddress(app, cpu, kBusInit)) return;
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

/* 0x40fcf0: the sound restarted for setup eax from the options screen (2
 * taken as 0, 3 the 3D setup): when effects or music are on the library
 * started again (initForSetup); failing that both are turned off, else the
 * effect bus set up (port::busInit) and the caps asked again. */
void restartSound(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (cpu.eax == 2)
        cpu.ecx ^= cpu.eax;
    cpu.esi = at(app, kSoundSurround);
    if (cpu.ecx == 3)
    {
        put(app, kSoundInitOK, 1);
        cpu.ecx = 0;
    }
    else
    {
        cpu.ebx = 0;
        put(app, kSoundInitOK, cpu.ebx);
    }
    if (at(app, kEffectsOn) != 0 || at(app, kMusicOn) != 0)
    {
        cpu.eax = 0xffffffff;
        if (!byAddress(app, cpu, kSndCaps)) return;
        cpu.edi = at(app, kSoundInitOK);
        cpu.ebx = cpu.eax;
        if (cpu.edi != 0 && !traceSetup(app, cpu)) return;
        if (!initForSetup(app, cpu)) return;
        if (sreg32(cpu.edx) < 0)
        {
            cpu.esi = 0;
            put(app, kMusicOn, cpu.esi);
            put(app, kMusicSerial, cpu.esi);
            put(app, kEffectsOn, cpu.esi);
        }
        else
        {
            if (!byAddress(app, cpu, kBusInit)) return;
            cpu.eax = 0xffffffff;
            if (!byAddress(app, cpu, kSndCaps)) return;
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

}  // namespace aiplan
}  // namespace nfs3hp
