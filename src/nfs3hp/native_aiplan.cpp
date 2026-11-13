#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_load_track.h"
#include "native_pursuit.h"
#include "native_vertices.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <vector>
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
 * Each is a typed function under a name of its own (a core: named arguments,
 * its result returned, C++ locals) with an adapter under the game's name for
 * the dispatch and the callers still on registers: it takes the registers and
 * the words pushed, calls the core and leaves eax, the x87's stack and esp as
 * the game's code does (eax only where a caller can read it: aiplan.inc's
 * promises say which leave nothing there).  The x87's operations are fp's in
 * the code's order, a float kept where the code stores one.  The cores call
 * each other as C++, and other typed files' functions as C++ too
 * (guest::direct, their headers) but where the eax a callee leaves, which its
 * core does not give back, is what an adapter here leaves; every other
 * function of the game's by address (callGame: the registers and words
 * handed over, the rest kept), and a frame
 * whose address a callee is handed stays on the guest's stack at the code's
 * depth (GuestFrame, ReturnSlot).  The three jump tables (0x40c9d4, 0x40e400,
 * 0x40ed14) go by the labels nfs3.exe's tables hold. */
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
constexpr reg32 kClock = 0x7d3684;
// The lane planner's globals (0x56ee90..): 14 rows of votes for the three lanes about the car (left,
// its own, right; sumLaneVotes adds them up) -- row 0 the present lane's, 1 the close cars', 2 the
// road's edge, 3 the road's shape and the roadside, 4 the way ahead, 5 the obstacles, 6 the oncoming
// cars, 7 the cars ahead; the lane taken; the cars sensed ahead in the three lanes and behind (their
// gaps after each, then whether each is in that lane by its middle, as words); the way ahead's
// side (1 left, -1 right); a car behind driving against the track.
constexpr reg32 kLaneCostA = 0x56ee90, kLaneCostB = 0x56ee94, kLaneCostC = 0x56ee98;
constexpr reg32 kLaneCostLo = 0x56ee9c, kLaneCostHi = 0x56eea4, kLaneCostObj = 0x56eea8, kLaneCostObjB = 0x56eeb0;
constexpr reg32 kNodeCostB = 0x56eeb4, kNodeCostC = 0x56eeb8, kNodeCostD = 0x56eebc, kNodeCostE = 0x56eec0;
constexpr reg32 kObjCost = 0x56eecc, kCarCostLeft = 0x56eed8, kCarCostRight = 0x56eee4;
constexpr reg32 kChosenLane = 0x56ef74;
constexpr reg32 kSensed = 0x56ef38, kSensedRight = 0x56ef50;
constexpr reg32 kSensedFlag = 0x56ef7c, kSensedEdge = 0x56ef68, kSensedEdgeB = 0x56ef6e;
constexpr reg32 kSensedDir = 0x56ef78;
// The lane-width table (80 floats), the obstacles (8 of 0x50 bytes).
constexpr reg32 kLaneWidths = 0x56f0e0, kObstacles = 0x7dd850;
// The track's speed table loaded, its half, the failure flag.
constexpr reg32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kSpeedsReady = 0x56f228;
constexpr reg32 kTrafficSide = 0x56f230, kTrackWalked = 0x56f220;
// The catch-up tables (0x56ef80.. per track-number slot of 0x20), the start.
constexpr reg32 kTuning = 0x56ef80, kStartFlags = 0x55eb2e;
// The animation players (0x18 bytes each) and the seven paths loaded.
constexpr reg32 kAnims = 0x56f234, kAnimPaths = 0x570a34;
// The sound config: effects on, music on, the options, the result flag.
constexpr reg32 kEffectsOn = 0x55290c, kMusicOn = 0x552910, kMusicSerial = 0x552918;
constexpr reg32 kSoundInitOK = 0x5525a4;
constexpr reg32 kVoiceRecords = 0x571124;
// Constants of the game's.
constexpr reg32 kVolume = 0x53684c, kTicks = 0x79c1a4;

// What is called by address: the traffic's top speed (0x40bfe0, its eax
// left by slowForPolice), the AI-votes trace hook (0x40de30,
// apply_native_vertices), and the game's/other parts' natives (stood in for
// where they would run on a made-up world): a traffic car linked (0x404690,
// whose eax resetTrafficCar leaves) and vectors scaled (0x4e0050) where the
// eax they leave is an adapter's.  The other chunks' AI functions (0x40c160,
// 0x40c2f0, 0x40bff0, 0x40c270, 0x401240, 0x405cd0..0x405e20), the drive
// reset (0x4aa79f), the opponents' speeds (0x40e000), the free (0x4e1890),
// the trace (0x42d510) and vectors scaled elsewhere are called as C++
// (guest::direct).
constexpr reg32 kSpeedFloat = 0x40bfe0, kAiVotes = 0x40de30;
constexpr reg32 kCopChaseSet = 0x411750;
constexpr reg32 kPlaceOnRoad = 0x4a0aa0, kScaleAdd = 0x4e0050;
constexpr reg32 kMemset = 0x4e0640;
constexpr reg32 kChop = 0x4dfd56;
constexpr reg32 kPoseBlend = 0x4b70f0, kPoseMatrix = 0x4b7590, kPlaceBlend = 0x4b6a30, kQuatMatrix = 0x4b6ca0;
constexpr reg32 kSndInit = 0x4e2630, kSndCaps = 0x4e21b0, kSndRate = 0x4e2228, kBusInit = 0x418740;

// The game's floats and doubles where it keeps them, as the x87 loads them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

reg32 nodeAddr(App* app, reg32 index)
{
    // lea eax,[index*8]; add eax,index; shl eax,2; add [0x5dd948]: index*0x24.
    return at(app, kNodes) + index * 0x24;
}

void putFloat(App* app, reg32 a, float f) { app->getMemory<float>(a) = f; }

/* A float of the bits given, as the code's mov [frame], bits; fld [frame] has it. */
float floatOf(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

reg32 bitsOf(float f)
{
    reg32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

/* A signed byte, as movsx has it. */
i32 signedByte(App* app, reg32 a) { return i32(std::int8_t(byte(app, a))); }

/* What a frame keeps of its caller's registers across a call by address: every register but
 * eax -- the code pushes and pops what it uses, the game's callees keep the rest. */
class KeptRegisters
{
public:
    explicit KeptRegisters(CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp),
          esp_(cpu.esp)
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
    KeptRegisters(const KeptRegisters&) = delete;
    KeptRegisters& operator=(const KeptRegisters&) = delete;

private:
    CPU& cpu_;
    const reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_, esp_;
};

/* 0x4e1890 (ea::purgememadr): the handle freed, eax as the free leaves it -- what it gives back;
 * false when the game stopped. */
bool freeHandle(App* app, CPU& cpu, reg32 handle)
{
    const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, handle);
    if (!freed)
        return false;
    cpu.eax = *freed;
    return true;
}

// The game's constants the code reads where it has them.
constexpr reg32 kSixth = 0x5367b4;          // a double: 1/6
constexpr reg32 kFixedToFloat = 0x5367bc;   // a double: 1/65536
constexpr reg32 kQuarterLanes = 0x5367c4;   // a float: 4, the lane-width table's steps per unit
constexpr reg32 kHalfSlice = 0x5367cc;      // a double: 0.5
constexpr reg32 kLessHalfSlice = 0x5367d4;  // a double: -0.5
constexpr reg32 kDirectionScale = 0x5367f4; // a double: 1/65536

// A node's record (0x24 bytes): its lane bits' dword (the high half the bits), its edge slices (high
// nibble left, low right), its forward direction's x and z (signed bytes), its halves' widths.
constexpr reg32 kNodeLaneBits = 0xa, kNodeEdges = 0x13, kNodeDirectionX = 0x14, kNodeDirectionZ = 0x16,
                kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b, kNodeSize = 0x24;
// A track's tuning record (0x20 bytes at 0x56ef80 by the track): as fillTrackTuning fills it.
constexpr reg32 kTuningFirst = 0, kTuningDistance = 4, kTuningRoadWide = 8, kTuningSide = 0xc,
                kTuningSpeed = 0x14, kTuningSeriesSpeed = 0x18, kTuningLast = 0x1c, kTuningSize = 0x20;
constexpr i32 kTrackCount = 0xb;  // the tracks with a record, as the code bounds them
// The race's laps.
constexpr reg32 kLaps = 0x5efd94;
// A car's fields this code reads: its node, its place along the track and its speed along it, its
// flags (+0x621 bit 4: it drives against the track's direction), its slices, its place across the road
// (+0x7b8), its half width as a slice offset (+0x21c), its look-ahead (+0x7c4), its way (+0x7d4 1
// backwards).
constexpr reg32 kCarNode = 0x1c, kCarAlong = 8, kCarSpeedAlong = 0x1fc, kCarHalfWidth = 0x21c,
                kCarWayFlags = 0x621, kCarSlice = 0x638, kCarLeftSlice = 0x63c, kCarRightSlice = 0x640,
                kCarAcross = 0x7b8, kCarLookAhead = 0x7c4, kCarBackwards = 0x7d4;
constexpr reg8 kAgainstTrack = 0x10;
// The road's slices across, 0..13.
constexpr i32 kSlices = 0xe;
// A traffic car's (and a racer's) fields more: its forward vector, its velocity, its spin, its grip
// on the road, whether it is off the ground (a word), its kind (+0x200 bit 2: a player's), the brake,
// its traffic flags (+0x620: bit 1 to be placed, bit 2 out of play, bit 3 the horn), its loading step,
// its horn's end and tone, the player car it keeps near, its state (4 pulled over), the police's
// siren (+0x710 bit 4) and chase (+0x734), its speed asked (+0x7b0), its target place across (+0x7b4),
// its speed (+0x7c0).
constexpr reg32 kCarForward = 0xd8, kCarVelocity = 0xa4, kCarSpin = 0xe4, kCarGrip = 0x10c, kCarAirborne = 0x164,
                kCarKindFlags = 0x200, kCarBrake = 0x509, kCarTrafficFlags = 0x620, kCarLoading = 0x630,
                kCarHornEnd = 0x644, kCarHornTone = 0x660, kCarFollows = 0x6d8, kCarState = 0x6dc,
                kCarSirenFlags = 0x710, kCarChasing = 0x734, kCarTargetSpeed = 0x7b0, kCarTargetAcross = 0x7b4,
                kCarSpeed = 0x7c0;
constexpr reg8 kKindPlayer = 4, kTrafficInactive = 4, kTrafficHorn = 8, kSirenOn = 0x10;
constexpr reg32 kStateDriving = 1, kStatePullingOver = 3, kStatePulledOver = 4;
// More of a car's: its index among the cars, whether it is in play (a byte), its half length.
constexpr reg32 kCarIndex = 0x1f0, kCarInPlay = 0x89, kCarHalfLength = 0x114;
// A node's road edges (16.16 from its middle).
constexpr reg32 kNodeLeftEdge = 0x1c, kNodeRightEdge = 0x20;
// The sensed cars' gaps ahead (three dwords after the three cars at 0x56ef38) and behind (after the
// three behind at 0x56ef50).
constexpr reg32 kSensedGaps = 0x56ef44, kSensedBehindGaps = 0x56ef5c;
// The parked obstacles (8 of 0x50 bytes at 0x7dd850): +0 in use, +0x34 the node, +0x38 the slice.
constexpr reg32 kObstacleSize = 0x50, kObstacleNode = 0x34, kObstacleSlice = 0x38;
// The lane planner's constants, as the code reads them (doubles but the one float).
constexpr reg32 kSenseScale = 0x53670c, kSenseSixth = 0x536714;  // 2, 1/6: the sensing window
constexpr reg32 kToFixedC = 0x53671c;                            // 65536: the half length as 16.16
constexpr reg32 kEaseObstacle = 0x536724;                        // 0.9: the speed by an obstacle
constexpr reg32 kMeetScale = 0x53672c, kToFixedD = 0x536734;     // 6, 65536: the time to meet
constexpr reg32 kFromFixed = 0x53673c;                           // 1/65536
constexpr reg32 kMeetWeight = 0x536744;                          // a float: -35
constexpr reg32 kEighth = 0x53674c, kMeetLessHalf = 0x536754;    // 1/8, -0.5
constexpr reg32 kOutrunMost = 0x53675c, kOutrunWeight = 0x536764, kThird = 0x53676c;  // 3, -5, 1/3
constexpr reg32 kLaneFixed = 0x536774, kHalfWidthBack = 0x53677c;  // 1/65536, 0.5: the place across
constexpr reg32 kLaneReach = 0x536784, kLaneSixth = 0x53678c, kLaneLessHalf = 0x536794;  // 2.5, 1/6, -0.5
// The tuning's: the handicap's step (a float, -0.005), a quarter (a float), 0.98 and 0.99 (doubles); the
// opponents' speeds loaded (load::loadOpponentSpeeds); the highest target speed on the track.
constexpr reg32 kHandicapStep = 0x5367dc, kQuarter = 0x5367e0, kHandicapSlower = 0x5367e4, kHandicapSlow = 0x5367ec;
constexpr reg32 kTopTargetSpeed = 0x56f224;
// The animation players: 256 slots of 0x18 bytes.  A path: +4 its points (a word), +6 its frames per
// point (a word), from +8 its points of 0x14 bytes -- a place of three 16.16 dwords, an orientation of
// four shorts at +0xc.  1/65536 as a double, for the poses.
constexpr reg32 kAnimSlots = 0x100, kAnimSize = 0x18;
constexpr reg32 kPathPoints = 4, kPathFrames = 6, kPathPointList = 8, kPointSize = 0x14, kPointOrientation = 0xc;
constexpr reg32 kPoseScale = 0x536814;
// The game's random numbers: the seed, the product, the multiplier.
constexpr reg32 kRandomSeed = 0x554f10, kRandomProduct = 0x554f14, kRandomMultiplier = 0x554f18;
// A word the sensed cars' clearing zeroes with them.
constexpr reg32 kSensedRound = 0x56ebc0;
// The data among the code: the place's last part for placeOnTrack, the grid's place.
constexpr reg32 kPlaceOnTrackHeight = 0x40cb08, kGridPlace = 0x40cb0c;
// The game's constants: 20 and 23 (doubles: how fast the cars close for the horn, for a chase),
// 65536 twice (doubles), 0.9 (a double: a firm grip), 6.7 and 0.5 as float bits.
constexpr reg32 kHornClosing = 0x5366e4, kChaseClosing = 0x5366ec, kToFixedA = 0x5366f4, kToFixedB = 0x5366fc,
                kFirmGrip = 0x536704;
constexpr reg32 kPullOverSpeed = 0x40d6930c, kHalf = 0x3f000000;
constexpr reg32 kLinkTraffic = 0x404690;

/* fcom of a and b, as fnstsw leaves the status word: what a code that ends on a compare leaves in ax. */
reg16 statusWord(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.word;
}

/* 0x4dfd56 (__CHP): value to an integer towards 0, as a whole value through the x87's stack; nullopt
 * when the game stopped. */
std::optional<Float> chop(App* app, CPU& cpu, const Float& value)
{
    const KeptRegisters kept(cpu);
    cpu.fpu.push(value);
    if (!guest::call(app, cpu, kChop))
        return std::nullopt;
    return cpu.fpu.pop();
}

/* A call by address as the code makes it: the registers handed over (eax, edx, ebx, ecx in that
 * order) and the words pushed; every register but eax kept (KeptRegisters).  False when the game
 * stopped. */
bool callGame(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> registers,
              std::initializer_list<reg32> words = {})
{
    const KeptRegisters kept(cpu);
    reg32* const handed[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    std::size_t i = 0;
    for (const reg32 value : registers)
        *handed[i++] = value;
    return guest::call(app, cpu, target, words);
}

/* 0x404690 (airace::setRammingRules, below this file) on car (eax), by address: the eax it leaves
 * is resetTrafficCar's, and its core gives nothing back.  False when the game stopped. */
bool linkTraffic(App* app, CPU& cpu, reg32 car)
{
    return callGame(app, cpu, kLinkTraffic, {car});
}

/* 0x40bfe0: the traffic's top speed, off the x87's stack; nullopt when the game stopped.  By
 * address: slowForPolice leaves the eax it leaves, which pursuit's core does not give back. */
std::optional<Float> trafficTopSpeed(App* app, CPU& cpu, reg32 car)
{
    if (!callGame(app, cpu, kSpeedFloat, {car}))
        return std::nullopt;
    return cpu.fpu.pop();
}

/* 0x40de30: the lane votes of car added up and acted on (`kind` 0 driving, 2 pulling over). */
bool actOnVotes(App* app, CPU& cpu, reg32 car, reg32 kind)
{
    return callGame(app, cpu, kAiVotes, {car, kind});
}

/* 0x411750: a police chase called for car: 1, 100, 8. */
bool callChase(App* app, CPU& cpu, reg32 car)
{
    return callGame(app, cpu, kCopChaseSet, {car, 1, 0x64, 8});
}

/* 0x4a0aa0: car put on the road at node, at the place (three dwords) at `place`, its direction 1
 * against the track. */
bool placeOnRoad(App* app, CPU& cpu, reg32 car, reg32 node, reg32 place, reg32 direction)
{
    return callGame(app, cpu, kPlaceOnRoad, {car, node, place, direction});
}

/* 0x4e0050 (stdcall): `count` vectors from `from` scaled by the float of `scale`'s bits into `to`,
 * by address, for a caller that leaves the eax it leaves (brakeToStop). */
bool scaleVectors(App* app, CPU& cpu, reg32 count, reg32 from, reg32 scale, reg32 to)
{
    return callGame(app, cpu, kScaleAdd, {count, from}, {scale, to});
}

/* 0x4e0050 (vertices'): the same as C++ (guest::direct).  False when the game stopped. */
bool scaleVectorsDirect(App* app, CPU& cpu, reg32 count, reg32 from, reg32 scale, reg32 to)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, i32(count), from, floatOf(scale), to);
    return !cpu.terminate;
}

/* Room on the guest's stack for what a callee is handed the address of: the code's frame (`pushed`
 * bytes of registers under esp on entry, then `bytes`), esp under it while the core runs and back
 * after; an address in it as the code's ebp less the offset. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    reg32 operator-(reg32 offset) const { return base_ - offset; }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

/* The return address's slot (and `words` pushed before it) under esp, as a call to a sibling makes
 * them: a core that keeps a frame on the guest's stack is called at the code's depth. */
class ReturnSlot
{
public:
    explicit ReturnSlot(CPU& cpu, reg32 words = 0) : cpu_(cpu), esp_(cpu.esp) { cpu.esp -= 4 + 4 * words; }
    ~ReturnSlot() { cpu_.esp = esp_; }
    ReturnSlot(const ReturnSlot&) = delete;
    ReturnSlot& operator=(const ReturnSlot&) = delete;

private:
    CPU& cpu_;
    const reg32 esp_;
};

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

// The typed cores the adapters above call, where one is used before it is defined.
reg32 trackTrafficSide(App* app);
reg32 trackRoadWide(App* app);
reg32 sliceBit(reg32 slice);
void soundHorn(App* app, reg32 car);
void endHorn(App* app, reg32 car);
void clearRecord(App* app, reg32 car);
i32 nodesBetween(App* app, reg32 node, reg32 from);
void findCarSlices(App* app, reg32 car);
reg32 nodeAhead(App* app, reg32 car, reg32 steps);
reg32 laneBitsAt(App* app, reg32 node, reg32 mask);
void scoreRoad(App* app, reg32 car);
reg32 roadScoreLeft(App* app, reg32 car);
void scoreObstacles(App* app, reg32 car);
bool scoreCars(App* app, CPU& cpu, reg32 car);
bool beginPlanning(App* app, CPU& cpu, reg32 car);
void weighVotes(App* app);
bool reactToCars(App* app, CPU& cpu, reg32 car);
bool pickLane(App* app, CPU& cpu, reg32 car);
bool policeNear(App* app, reg32 car, reg32 distance);
void voteForRoadside(App* app, reg32 car);
bool reachedRoadside(App* app, reg32 car);
void penaliseCloseCars(App* app);
std::optional<reg32> stepLoading(App* app, CPU& cpu, reg32 car);
std::optional<bool> stopPulledOver(App* app, CPU& cpu, reg32 car);
bool animationOn(App* app, reg32 slot);

/* The player car fewest traffic cars keep near: for each of the [0x5efda4] players' cars (0x5efa48)
 * the active traffic cars (0x5efb48: +0x620 bit 2 clear) whose +0x6d8 is it are counted; the first
 * with the least (from 10000), or the first car [0x5efac8] when there are no players.  The players'
 * count written back, as the code does.  The code counts in a frame of 15; the game has 4 players at
 * most, and a count past 15 (which would run over the code's own frame) counts the first 15. */
reg32 leastFollowedPlayer(App* app)
{
    constexpr i32 kFrameCounts = 15;
    const i32 players = i32(at(app, kAiCount));
    const i32 counted = players < kFrameCounts ? players : kFrameCounts;
    i32 counts[kFrameCounts] = {};
    for (i32 player = 0; player < counted; ++player)
        for (i32 i = 0; i < i32(at(app, kCarCountB)); ++i)
        {
            const reg32 car = at(app, reg32(i) * 4 + kCarsByType);
            if ((byte(app, car + kCarTrafficFlags) & kTrafficInactive) != 0)
                continue;
            if (at(app, car + kCarFollows) == at(app, reg32(player) * 4 + kPlayerCars))
                ++counts[player];
        }
    reg32 least = at(app, kCars);
    i32 fewest = 10000;
    for (i32 player = 0; player < counted; ++player)
        if (fewest > counts[player])
        {
            fewest = counts[player];
            least = at(app, reg32(player) * 4 + kPlayerCars);
        }
    put(app, kAiCount, reg32(players));
    return least;
}

/* 0x40c4f0: leastFollowedPlayer; every register but eax kept. */
void leastTrafficPlayer(App* app, CPU& cpu)
{
    cpu.eax = leastFollowedPlayer(app);
    cpu.esp += 4;
}

/* Traffic car set going: the game's random numbers (the seed 0x554f10 by 0x554f18, 0..99 from the
 * product's middle bytes) make it drive against the track's direction (+0x621 bit 4) seven times in
 * ten, never where [0x56f228] (trackRoadWide) is set; the player car it keeps near (+0x6d8:
 * leastFollowedPlayer, or [0x56ebb8] with one player); then 0x40c160 and 0x40c2f0 (below this file)
 * place it.  False when the game stopped. */
bool placeTraffic(App* app, CPU& cpu, reg32 car)
{
    const reg32 product = at(app, kRandomSeed) * at(app, kRandomMultiplier);
    put(app, kRandomProduct, product);
    const reg32 middle = (at(app, kRandomProduct) & 0xffff00) >> 8;
    put(app, kRandomSeed, product & 0xffff);
    const reg32 roll = (middle * 100) >> 16;
    if (roll <= 0x45 && at(app, kSpeedsReady) == 0)
        putByte(app, car + kCarWayFlags, reg8(byte(app, car + kCarWayFlags) | kAgainstTrack));
    else
        putByte(app, car + kCarWayFlags, reg8(byte(app, car + kCarWayFlags) & ~kAgainstTrack));
    const reg32 follows = i32(at(app, kAiCount)) <= 1 ? at(app, kPlayerCarsB) : leastFollowedPlayer(app);
    put(app, car + kCarFollows, follows);
    if (!guest::direct<0x40c160, pursuit::followCarOf>(app, cpu, car))
        return false;
    // eax as 0x40c2f0 leaves it, what it gives back: placeTrafficCar's.
    const std::optional<reg32> routed = guest::direct<0x40c2f0, pursuit::trafficStepOf>(app, cpu, car);
    if (!routed)
        return false;
    cpu.eax = *routed;
    return true;
}

/* 0x40c590: placeTraffic for car eax; eax as 0x40c2f0 leaves it. */
void placeTrafficCar(App* app, CPU& cpu)
{
    if (placeTraffic(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* |value| as the code has it of a float it stored: kept when over 0 (or a NaN), negated else. */
float sizeOf(float value)
{
    return !(0.0 >= Float(value)) ? value : float(-Float(value));
}

/* Traffic car reacts to the cars about it.  While its horn sounds (+0x620 bit 3) it only waits for
 * it to end (endHorn).  A car driving against the track's direction, with a car ahead in its lane
 * ([0x56ef3c], its gap [0x56ef48] 7) going the other way (the speeds' signs, +0x7c0) that is a player's
 * (+0x200 bit 2): the horn when they close faster than 20, and faster than 23 a police chase called
 * (0x411750: 1, 100, 8).  Then, when a car about it drives against it [0x56ef7c], the horn for the
 * first player's car among the three sensed.  False when the game stopped. */
bool reactToCars(App* app, CPU& cpu, reg32 car)
{
    if ((byte(app, car + kCarTrafficFlags) & kTrafficHorn) != 0)
    {
        endHorn(app, car);
        return true;
    }
    if ((byte(app, car + kCarWayFlags) & kAgainstTrack) == 0)
        return true;
    const reg32 ahead = at(app, kSensed + 4);
    if (ahead != 0 && at(app, kSensedGaps + 4) == 7)
    {
        // fcompp of 0 with each speed: going back (0 over it) or not.
        const bool backwards = 0.0 > f32(app, car + kCarSpeed);
        const bool aheadBackwards = 0.0 > f32(app, ahead + kCarSpeed);
        if (backwards != aheadBackwards && (byte(app, ahead + kCarKindFlags) & kKindPlayer) != 0)
        {
            const float closing = float(fp::sub(f32(app, car + kCarSpeed), f32(app, ahead + kCarSpeed)));
            if (Float(sizeOf(closing)) > f64(app, kHornClosing))
                soundHorn(app, car);
            if (Float(sizeOf(closing)) > f64(app, kChaseClosing) && !callChase(app, cpu, car))
                return false;
        }
    }
    if (at(app, kSensedFlag) == 0)
        return true;
    for (reg32 i = 0; i < 3; ++i)
    {
        const reg32 sensed = at(app, i * 4 + kSensed);
        if (sensed != 0 && (byte(app, sensed + kCarKindFlags) & kKindPlayer) != 0)
        {
            soundHorn(app, car);
            return true;
        }
    }
    return true;
}

/* 0x40c630: reactToCars for car eax.  What the code leaves in eax (a compare's status word, the
 * sensed cars' index) no caller reads: its one caller is advanceCarAi's core; aiplan.inc's promise
 * says so. */
void reactToTraffic(App* app, CPU& cpu)
{
    if (reactToCars(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* What slowToPullOver did: the speed asked it compared, the top speed it had, and whether the speed
 * was cut to it. */
struct PullOver
{
    float asked;
    float topSpeed;
    bool cut;
};

/* Traffic car slowed to pull over: its speed +0x7b0 6.7, or 0x40bfe0's when that is less; its sign
 * flipped for a reversed race and again for a car against the track's direction.  Nullopt when the
 * game stopped. */
std::optional<PullOver> slowToPullOver(App* app, CPU& cpu, reg32 car)
{
    put(app, car + kCarTargetSpeed, kPullOverSpeed);
    const std::optional<Float> top = trafficTopSpeed(app, cpu, car);
    if (!top)
        return std::nullopt;
    const float asked = app->getMemory<float>(car + kCarTargetSpeed);
    const PullOver pullOver{asked, float(*top), Float(asked) > Float(float(*top))};
    if (pullOver.cut)
        putFloat(app, car + kCarTargetSpeed, pullOver.topSpeed);
    if (gameSettings(app).backwards != 0)
        putByte(app, car + kCarTargetSpeed + 3, reg8(byte(app, car + kCarTargetSpeed + 3) ^ 0x80));
    if ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0)
        putByte(app, car + kCarTargetSpeed + 3, reg8(byte(app, car + kCarTargetSpeed + 3) ^ 0x80));
    return pullOver;
}

/* 0x40c780: slowToPullOver for car eax; eax as the code leaves it: 0x40bfe0's with the compare's
 * status word in ax, or the top speed's bits where the speed was cut to it; ah the speed's top byte
 * as the reversed race's flip left it. */
void slowForPolice(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const std::optional<PullOver> pullOver = slowToPullOver(app, cpu, car);
    if (!pullOver)
        return;
    cpu.ax = statusWord(cpu, Float(pullOver->asked), Float(pullOver->topSpeed));
    if (pullOver->cut)
        cpu.eax = bitsOf(pullOver->topSpeed);
    if (gameSettings(app).backwards != 0)
        cpu.ah = reg8(byte(app, car + kCarTargetSpeed + 3)
                      ^ ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0x80 : 0));
    cpu.esp += 4;
}

/* How many active traffic cars have pulled over (+0x6dc 4). */
reg32 stoppedCount(App* app)
{
    reg32 stopped = 0;
    for (i32 i = 0; i < i32(at(app, kCarCountB)); ++i)
    {
        const reg32 car = at(app, reg32(i) * 4 + kCarsByType);
        if ((byte(app, car + kCarTrafficFlags) & kTrafficInactive) == 0 && at(app, car + kCarState) == kStatePulledOver)
            ++stopped;
    }
    return stopped;
}

/* 0x40c7f0: stoppedCount. */
void countStopped(App* app, CPU& cpu)
{
    cpu.eax = stoppedCount(app);
    cpu.esp += 4;
}

/* Whether a police car with its siren on (0x5efa88's, [0x5efd88] of them: +0x734 chasing or +0x710
 * bit 4) is within `distance` + 0x16 nodes of car -- two more for each car already pulled over
 * (stoppedCount). */
bool policeNear(App* app, reg32 car, reg32 distance)
{
    const reg32 node = at(app, car + kCarNode);
    const reg32 margin = stoppedCount(app) * 2;
    for (i32 i = 0; i < i32(at(app, kPlaceCount)); ++i)
    {
        const reg32 police = at(app, reg32(i) * 4 + kCarsByPlace);
        if (at(app, police + kCarChasing) == 0 && (byte(app, police + kCarSirenFlags) & kSirenOn) == 0)
            continue;
        i32 gap = nodesBetween(app, at(app, police + kCarNode), node);
        if (gap <= 0)
            gap = i32(0u - reg32(gap));
        if (gap < i32(distance + 0x16 + margin))
            return true;
    }
    return false;
}

/* 0x40c830: policeNear for car eax within edx. */
void nearPoliceCar(App* app, CPU& cpu)
{
    cpu.eax = policeNear(app, cpu.eax, cpu.edx) ? 1 : 0;
    cpu.esp += 4;
}

/* The three lane votes (0x56ee90..98) weighed ten times. */
void weighVotes(App* app)
{
    for (reg32 i = 0; i < 3; ++i)
        put(app, i * 4 + kLaneCostA, at(app, i * 4 + kLaneCostA) * 10);
}

/* 0x40c8a0: weighVotes; eax the last vote, as the code leaves it. */
void weighLaneVotes(App* app, CPU& cpu)
{
    weighVotes(app);
    cpu.eax = at(app, kLaneCostC);
    cpu.esp += 4;
}

/* A node's left and right edge slices: 6 less the high nibble of its +0x13, 7 more the low. */
i32 leftEdgeSlice(App* app, reg32 record) { return 6 - ((byte(app, record + kNodeEdges) >> 4) & 0xf); }
i32 rightEdgeSlice(App* app, reg32 record) { return (byte(app, record + kNodeEdges) & 0xf) + 7; }

/* A vote (+100) for pulling over: for the middle (0x56eeb8) when car's slice +0x638 is off its node's
 * road, else for the nearer roadside -- the left (0x56eeb4) or the right (0x56eebc). */
void voteForRoadside(App* app, reg32 car)
{
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    const i32 slice = i32(at(app, car + kCarSlice));
    if (leftEdgeSlice(app, record) > slice || rightEdgeSlice(app, record) < slice)
        put(app, kNodeCostC, at(app, kNodeCostC) + 0x64);
    else if (slice >= 7)
        put(app, kNodeCostD, at(app, kNodeCostD) + 0x64);
    else
        put(app, kNodeCostB, at(app, kNodeCostB) + 0x64);
}

/* 0x40c8d0: voteForRoadside for car eax; eax as the code leaves it: the node's record when the slice
 * is left of the left edge, else the right edge slice. */
void voteRoadside(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    voteForRoadside(app, car);
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    cpu.eax = leftEdgeSlice(app, record) > i32(at(app, car + kCarSlice)) ? record : reg32(rightEdgeSlice(app, record));
    cpu.esp += 4;
}

/* Whether car has reached the roadside: its place across (+0x7b8, as 16.16) beyond its node's left
 * edge (0x10000 less the left edge's slices times the left half's width) or right edge. */
bool reachedRoadside(App* app, reg32 car)
{
    const i32 across = fp::toInteger<i32>(fp::mul(f32(app, car + kCarAcross), f64(app, kToFixedA)));
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    const reg32 edges = byte(app, record + kNodeEdges);
    const reg32 left = 0x10000 - ((edges >> 4) & 0xf) * (reg32(byte(app, record + kNodeLeftWidth)) << 0xf);
    if (across < i32(left))
        return true;
    const reg32 right = (edges & 0xf) * (reg32(byte(app, record + kNodeRightWidth)) << 0xf) - 0x10000;
    return across > i32(right);
}

/* 0x40c950: reachedRoadside for car eax. */
void atRoadside(App* app, CPU& cpu)
{
    cpu.eax = reachedRoadside(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

/* Traffic car's driver, each tick: its loading steps first (stepLoading, called at the code's depth:
 * its frame is handed on; 0: not on the road).  Then by its state +0x6dc (the jump table at 0x40c9d4):
 * 0 (and past 5) made 1; 1 drives -- 0x40c270, the lanes planned (beginPlanning, weighVotes,
 * reactToCars, 0x40bff0, pickLane) and the votes acted on (0x40de30 with 0) -- and pulls over (3) when
 * a police car is near (policeNear by 0); 3 slows to the roadside (slowToPullOver, voteForRoadside,
 * the obstacles, the cars, the close ones, 0x40de30 with 2) -- back to 1 when the police car has gone
 * (by 2), to 4 at the roadside; 4 stands (its speed asked 0, stopPulledOver) until the police car has
 * gone (1); 2 and 5 nothing.  False when the game stopped. */
bool driveTraffic(App* app, CPU& cpu, reg32 car)
{
    std::optional<reg32> onRoad;
    {
        const GuestFrame frame(cpu, 0xc, 0);
        const ReturnSlot slot(cpu);
        onRoad = stepLoading(app, cpu, car);
    }
    if (!onRoad)
        return false;
    if (*onRoad == 0)
        return true;
    const reg32 state = at(app, car + kCarState);
    if (state == 0 || state == 1 || state > 5)
    {
        if (state != 1)
            put(app, car + kCarState, kStateDriving);
        if (!guest::direct<0x40c270, pursuit::trafficTargetOf>(app, cpu, car) || !beginPlanning(app, cpu, car))
            return false;
        weighVotes(app);
        if (!reactToCars(app, cpu, car))
            return false;
        guest::direct<0x40bff0, pursuit::followChasedCar>(app, cpu, car);
        if (cpu.terminate || !pickLane(app, cpu, car) || !actOnVotes(app, cpu, car, 0))
            return false;
        if (policeNear(app, car, 0))
            put(app, car + kCarState, kStatePullingOver);
        return true;
    }
    if (state == kStatePullingOver)
    {
        if (!beginPlanning(app, cpu, car) || !slowToPullOver(app, cpu, car))
            return false;
        voteForRoadside(app, car);
        scoreObstacles(app, car);
        if (!scoreCars(app, cpu, car))
            return false;
        penaliseCloseCars(app);
        if (!actOnVotes(app, cpu, car, 2))
            return false;
        if (!policeNear(app, car, 2))
            put(app, car + kCarState, kStateDriving);
        else if (reachedRoadside(app, car))
            put(app, car + kCarState, kStatePulledOver);
        return true;
    }
    if (state == kStatePulledOver)
    {
        put(app, car + kCarTargetSpeed, 0);
        if (!stopPulledOver(app, cpu, car))
            return false;
        if (!policeNear(app, car, 2))
            put(app, car + kCarState, kStateDriving);
    }
    return true;  // 2 and 5
}

/* 0x40c9f0: driveTraffic for car eax.  What the code leaves in eax (the last yes or no it had) no
 * caller reads: it is the car's tick hook (+0x600); aiplan.inc's promise says so. */
void advanceCarAi(App* app, CPU& cpu)
{
    if (driveTraffic(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* The sensed cars (0x48 bytes at 0x56ef38) cleared (0x4e0640) and [0x56ebc0] zeroed; false when the
 * game stopped. */
bool clearSensed(App* app, CPU& cpu)
{
    if (!callGame(app, cpu, kMemset, {kSensed, 0, 0x48}))
        return false;
    put(app, kSensedRound, 0);
    return true;
}

/* 0x40cb20: clearSensed; eax as the clearing leaves it. */
void clearSensedCars(App* app, CPU& cpu)
{
    if (clearSensed(app, cpu))
        cpu.esp += 4;
}

/* The track's traffic side [0x56f230] (trackTrafficSide) kept and [0x56ebc0] zeroed; the side. */
reg32 keepTrafficSide(App* app)
{
    const reg32 side = trackTrafficSide(app);
    put(app, kTrafficSide, side);
    put(app, kSensedRound, 0);
    return side;
}

/* 0x40cb50: keepTrafficSide. */
void cacheTrafficSide(App* app, CPU& cpu)
{
    cpu.eax = keepTrafficSide(app);
    cpu.esp += 4;
}

/* Traffic car reset: its record (clearRecord), then 0x404690 (below this file); false when the game
 * stopped. */
bool resetTraffic(App* app, CPU& cpu, reg32 car)
{
    clearRecord(app, car);
    return linkTraffic(app, cpu, car);
}

/* 0x40cb70: resetTraffic for car eax; eax as 0x404690 leaves it. */
void resetTrafficCar(App* app, CPU& cpu)
{
    if (resetTraffic(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* Traffic car's driving state cleared: states, targets, lanes and speeds zeroed, +0x7bc 1.0, its next
 * decision +0x634 the clock [0x7d3684] + 0x500, +0x710 bit 4 cleared; +0x620 bit 1 for a car of the
 * kinds +0x200 bit 4 or 5, +0x200 bit 1 cleared for a player's car (bit 2).  In the code's order. */
void clearRecord(App* app, reg32 car)
{
    for (const reg32 off : {0x630u, 0x6d8u, 0x64cu, 0x650u, 0x654u, 0x658u, 0x65cu, 0x6dcu, 0x6e0u, 0x6e4u, 0x6e8u,
                            0x2c8u, 0x6f0u, 0x704u, 0x708u, 0x70cu, 0x710u, 0x714u, 0x718u, 0x730u, 0x71cu, 0x720u,
                            0x724u, 0x734u, 0x7acu, 0x7b4u, 0x7b0u, 0x7c0u, 0x7c4u})
        put(app, car + off, 0);
    put(app, car + 0x7bc, 0x3f800000);
    put(app, car + kCarAcross, 0);
    put(app, car + kCarHalfWidth, 0);
    put(app, car + 0x7c8, 0);
    put(app, car + 0x624, 0);
    put(app, car + 0x628, 0);
    const reg32 decision = at(app, kClock) + 0x500;
    put(app, car + kCarBackwards, 0);
    put(app, car + 0x634, decision);
    const reg8 sirenFlags = byte(app, car + kCarSirenFlags);
    put(app, car + 0x62c, 0);
    putByte(app, car + kCarSirenFlags, reg8(sirenFlags & 0xef));
    put(app, car + kCarTrafficFlags, 0);
    const reg8 kind = byte(app, car + kCarKindFlags);
    put(app, car + 0x7d0, 0);
    if ((kind & 0x10) != 0)
        putByte(app, car + kCarTrafficFlags, reg8(byte(app, car + kCarTrafficFlags) | 2));
    if ((byte(app, car + kCarKindFlags) & 0x20) != 0)
        putByte(app, car + kCarTrafficFlags, reg8(byte(app, car + kCarTrafficFlags) | 2));
    const reg8 kindNow = byte(app, car + kCarKindFlags);
    if ((kindNow & kKindPlayer) != 0)
        putByte(app, car + kCarKindFlags, reg8(kindNow & 0xfd));
}

/* 0x40cb90: clearRecord for car eax. */
void clearTrafficRecord(App* app, CPU& cpu)
{
    clearRecord(app, cpu.eax);
    cpu.esp += 4;
}

/* Car put on the track at its node +0x1c and its target place across +0x7b4 (as 16.16, with 1 and
 * 0x40cb08's for the place's other two parts; 0x4a0aa0, its direction by the reversed race and +0x621
 * bit 4), its velocity +0xa4 its forward vector +0xd8 scaled by its look-ahead +0x7c4 (0x4e0050), its
 * spin and slips zeroed.  The place and the vector in the code's frame, whose addresses the two are
 * handed.  False when the game stopped. */
bool putOnTrack(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x1c);
    const reg32 place = frame - 0x10, vector = frame - 0x1c;
    const reg32 direction = ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 1 : 0) ^ gameSettings(app).backwards;
    put(app, place, reg32(fp::toInteger<i32>(fp::mul(f32(app, car + kCarTargetAcross), f64(app, kToFixedB)))));
    put(app, place + 4, 0x10000);
    put(app, place + 8, at(app, kPlaceOnTrackHeight));
    if (!placeOnRoad(app, cpu, car, at(app, car + kCarNode), place, direction))
        return false;
    for (reg32 i = 0; i < 0xc; i += 4)
        put(app, vector + i, at(app, car + kCarForward + i));
    if (!scaleVectorsDirect(app, cpu, 1, vector, at(app, car + kCarLookAhead), vector))
        return false;
    for (reg32 i = 0; i < 0xc; i += 4)
        put(app, car + kCarVelocity + i, at(app, vector + i));
    put(app, car + kCarSpin, 0);
    put(app, car + kCarSpin + 4, 0);
    put(app, car + kCarSpin + 8, 0);
    app->getMemory<reg16>(car + kCarAirborne) = 0;
    put(app, car + 0x5c0, 0);
    put(app, car + 0x5c4, 0);
    put(app, car + 0x64c, 0);
    return true;
}

/* 0x40cd80: putOnTrack for car eax, left in eax. */
void placeOnTrack(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!putOnTrack(app, cpu, car))
        return;
    cpu.eax = car;
    cpu.esp += 4;
}

/* The pulled-over car brought to a stop: once on the ground (+0x164 0) and well on the road's grip
 * (+0x10c over 0.9), its drive reset (0x4aa79f), its spin +0xe4 and velocity +0xa4 halved (0x4e0050),
 * +0x509 0xff.  Whether it was; nullopt when the game stopped. */
std::optional<bool> stopPulledOver(App* app, CPU& cpu, reg32 car)
{
    if (app->getMemory<reg16>(car + kCarAirborne) != 0)
        return false;
    if (!(f32(app, car + kCarGrip) > f64(app, kFirmGrip)))
        return false;
    put(app, car + 0x64c, 0);
    guest::direct<0x4aa79f, handling::resetDrive>(app, cpu, car);
    if (cpu.terminate)
        return std::nullopt;
    if (!scaleVectorsDirect(app, cpu, 1, car + kCarSpin, kHalf, car + kCarSpin))
        return std::nullopt;
    // By address: its eax is what brakeToStop leaves.
    if (!scaleVectors(app, cpu, 1, car + kCarVelocity, kHalf, car + kCarVelocity))
        return std::nullopt;
    putByte(app, car + kCarBrake, 0xff);
    return true;
}

/* 0x40ce70: stopPulledOver for car eax; eax as the code leaves it: the car, with the grip's compare's
 * status word in ax when that ended it, or the last scaling's. */
void brakeToStop(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const std::optional<bool> stopped = stopPulledOver(app, cpu, car);
    if (!stopped)
        return;
    if (!*stopped && app->getMemory<reg16>(car + kCarAirborne) == 0)
        cpu.ax = statusWord(cpu, f32(app, car + kCarGrip), f64(app, kFirmGrip));
    cpu.esp += 4;
}

/* Car's present slice +0x638 votes into the first row of lane votes (0x56ee90..98) for the lanes
 * about it, by its way against the track's (+0x621 bit 4, the reversed race) and the track's traffic
 * side [0x56f230]: with the track's direction on a side of 1, or against it on a side of -1, it keeps
 * right -- a lane left of slice 7 loses 3, the others gain 1; else it keeps left, the other way
 * round (the code's own few exceptions kept). */
void voteForCurrentLane(App* app, reg32 car)
{
    const bool against = (byte(app, car + kCarWayFlags) & kAgainstTrack) != 0;
    const reg32 side = at(app, kTrafficSide);
    const bool keepRight = ((((against ? 0u : 1u) ^ gameSettings(app).backwards) != 0 && side == 1)
                          || (((against ? 1u : 0u) ^ gameSettings(app).backwards) != 0 && side == 0xffffffff));
    const auto add = [&](reg32 vote, reg32 by) { put(app, vote, at(app, vote) + by); };
    if (keepRight)
    {
        add(kLaneCostA, i32(at(app, car + kCarSlice) - 1) < 7 ? reg32(-3) : 1);
        add(kLaneCostB, i32(at(app, car + kCarSlice)) < 7 ? reg32(-3) : 1);
        if (i32(at(app, car + kCarSlice) + 1) >= 7)
            add(kLaneCostC, 1);
    }
    else
    {
        if (i32(at(app, car + kCarSlice) - 1) < 7)
            add(kLaneCostA, 1);
        else
            add(kLaneCostB, i32(at(app, car + kCarSlice)) < 7 ? 1 : reg32(-3));
        add(kLaneCostC, i32(at(app, car + kCarSlice) + 1) < 7 ? 1 : reg32(-3));
    }
}

/* 0x40cee0: voteForCurrentLane for car eax; eax the slice right of the car's, as the code leaves
 * it. */
void voteCurrentLane(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    voteForCurrentLane(app, car);
    cpu.eax = at(app, car + kCarSlice) + 1;
    cpu.esp += 4;
}

/* Car senses the traffic about it.  Within a window of nodes set by its look-ahead (+0x7c4 by 2 and
 * 1/6, chopped; 10 at least), the other cars (in play, +0x89 set, +0x200 bit 6 clear) whose gap in
 * its direction of travel fits are gathered; then for each of the three lanes about its slice (-1, 0,
 * +1, from each one's sides' slices +0x63c..+0x640) the nearest ahead (0x56ef38, its gap 0x56ef44,
 * whether its middle is that lane 0x56ef68) and the nearest behind (0x56ef50, 0x56ef5c, 0x56ef6e) are
 * kept, and [0x56ef7c] raised for a car behind that drives against the track (+0x200 bit 5).  The
 * code gathers into a frame of 16; a C++ list here, the same for any count the game has.  False when
 * the game stopped. */
bool senseCars(App* app, CPU& cpu, reg32 car)
{
    const std::optional<Float> reach = chop(
        app, cpu, fp::mul(fp::mul(f32(app, car + kCarLookAhead), f64(app, kSenseScale)), f64(app, kSenseSixth)));
    if (!reach)
        return false;
    i32 window = fp::toInteger<i32>(*reach);
    if (window < 0xa)
        window = 0xa;
    const reg32 way = (((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u : 1u) ^ gameSettings(app).backwards) == 0
                          ? 0xffffffff
                          : 1;
    const reg32 node = at(app, car + kCarNode);
    std::vector<reg32> cars;
    std::vector<i32> gaps;
    for (i32 i = 0; i < i32(at(app, kCarCount)); ++i)
    {
        if (reg32(i) == at(app, car + kCarIndex))
            continue;
        const reg32 other = at(app, reg32(i) * 4 + kCars);
        if (byte(app, other + kCarInPlay) == 0 || (byte(app, other + kCarKindFlags) & 0x40) != 0)
            continue;
        const i32 gap = i32(reg32(nodesBetween(app, at(app, other + kCarNode), node)) * way);
        const i32 size = gap > 0 ? gap : i32(0u - reg32(gap));
        if (size > window)
            continue;
        cars.push_back(other);
        gaps.push_back(gap);
    }
    put(app, kSensedGaps + 4, reg32(window));
    put(app, kSensedGaps + 8, reg32(window));
    put(app, kSensedBehindGaps, reg32(window));
    put(app, kSensedGaps, reg32(window));
    put(app, kSensedBehindGaps, 0u - reg32(window));
    put(app, kSensedBehindGaps + 4, 0u - reg32(window));
    put(app, kSensedBehindGaps + 8, 0u - reg32(window));
    const reg32 slice = at(app, car + kCarSlice);
    for (std::size_t s = 0; s < cars.size(); ++s)
    {
        const reg32 other = cars[s];
        const i32 middle = i32(at(app, other + kCarSlice) - slice + 1);
        i32 first = middle, last = middle;
        if (at(app, other + kCarLeftSlice) != at(app, other + kCarRightSlice))
        {
            first = i32(at(app, other + kCarLeftSlice) - slice + 1);
            last = i32(at(app, other + kCarRightSlice) - slice + 1);
        }
        for (i32 lane = first; lane <= last; ++lane)
        {
            if (lane < 0 || lane > 2)
                continue;
            const i32 gap = gaps[s];
            if (gap >= 0)
            {
                if (gap >= i32(at(app, reg32(lane) * 4 + kSensedGaps)))
                    continue;
                put(app, reg32(lane) * 4 + kSensed, other);
                put(app, reg32(lane) * 4 + kSensedGaps, reg32(gap));
                app->getMemory<reg16>(reg32(lane) * 2 + kSensedEdge) = reg16(lane == middle ? 1 : 0);
            }
            else
            {
                if (gap <= i32(at(app, reg32(lane) * 4 + kSensedBehindGaps)))
                    continue;
                put(app, reg32(lane) * 4 + kSensedBehindGaps, reg32(gap));
                put(app, reg32(lane) * 4 + kSensedRight, other);
                app->getMemory<reg16>(reg32(lane) * 2 + kSensedEdgeB) = reg16(lane == middle ? 1 : 0);
                if ((byte(app, at(app, reg32(lane) * 4 + kSensedRight) + kCarKindFlags) & 0x20) != 0)
                    put(app, kSensedFlag, 1);
            }
        }
    }
    return true;
}

/* 0x40cfc0: senseCars for car eax.  What the code leaves in eax (the last lane's index) no caller
 * reads; aiplan.inc's promise says so. */
void senseTraffic(App* app, CPU& cpu)
{
    if (senseCars(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* Car's way to node `target` costed by lane: the chain of nodes from its own (nodeAhead 1.. until the
 * target, 0x13 at most), then for each of the three lanes about its slice (sliceBit of the slices -1,
 * 0, +1): no road there at the car's node, that lane's route cost [0x56eec0 + lane] cut by 1000 and the
 * way [0x56ef78] set (1 left, 0, -1 right) -- the rest not looked at; else the first node of the chain
 * without it cuts 0x41. */
void costLaneChange(App* app, reg32 car, reg32 target)
{
    reg32 masks[3];
    masks[0] = sliceBit(at(app, car + kCarSlice) - 1);
    masks[1] = sliceBit(at(app, car + kCarSlice));
    masks[2] = sliceBit(at(app, car + kCarSlice) + 1);
    reg32 chain[0x14];
    chain[0] = at(app, car + kCarNode);
    if (target == chain[0])
        return;
    i32 length = 1;
    while (true)
    {
        chain[length] = nodeAhead(app, car, reg32(length));
        if (target == chain[length])
            break;
        ++length;
        if (length >= 0x14)
            break;
    }
    for (i32 lane = 0; lane < 3; ++lane)
    {
        const reg32 cost = reg32(lane) * 4 + kNodeCostE;
        if (laneBitsAt(app, chain[0], masks[lane]) == 0)
        {
            put(app, cost, at(app, cost) - 0x3e8);
            put(app, kSensedDir, lane == 0 ? 1 : lane == 2 ? 0xffffffff : 0);
            return;
        }
        for (i32 step = 1; step < length; ++step)
            if (laneBitsAt(app, chain[step], masks[lane]) == 0)
            {
                put(app, cost, at(app, cost) - 0x41);
                break;
            }
    }
}

/* 0x40d1f0: costLaneChange for car eax towards node edx.  What the code leaves in eax no caller
 * reads; aiplan.inc's promise says so. */
void planLaneChange(App* app, CPU& cpu)
{
    costLaneChange(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* A node's edge slices as edgeLaneCost has them: 7 less the high nibble of its +0x13, 6 more the
 * low. */
i32 innerLeftSlice(App* app, reg32 record) { return 7 - ((byte(app, record + kNodeEdges) >> 4) & 0xf); }
i32 innerRightSlice(App* app, reg32 record) { return (byte(app, record + kNodeEdges) & 0xf) + 6; }

/* Car heading to node `target`: a player's car (+0x200 bit 0) on track 6 run forwards between its
 * nodes 0x127 and 0x168 has the second row's left vote (0x56eea8) cut by 0x28; else, when car's slice
 * +0x638 is off the target node's inner edges but within its own node's, that row's right (0x56eeb0,
 * slices 7 on) or left vote gains 0x28. */
void costEdgeLane(App* app, reg32 car, reg32 target)
{
    const reg8 kind = byte(app, car + kCarKindFlags);
    const i32 slice = i32(at(app, car + kCarSlice));
    if ((kind & 1) != 0 && gameSettings(app).backwards == 0 && gameSettings(app).track == 6)
    {
        const i32 node = i32(at(app, car + kCarNode));
        if (node > 0x126 && node < 0x169)
        {
            put(app, kLaneCostObj, at(app, kLaneCostObj) - 0x28);
            return;
        }
    }
    const reg32 targetRecord = nodeAddr(app, target);
    if (!(slice < innerLeftSlice(app, targetRecord)) && slice <= innerRightSlice(app, targetRecord))
        return;
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    if (slice < innerLeftSlice(app, record))
        return;
    if (slice <= innerRightSlice(app, record))
    {
        if (slice >= 7)
            put(app, kLaneCostObj, at(app, kLaneCostObj) + 0x28);
        else
            put(app, kLaneCostObjB, at(app, kLaneCostObjB) + 0x28);
    }
}

/* 0x40d2e0: costEdgeLane for car eax towards node edx.  What the code leaves in eax no caller reads;
 * aiplan.inc's promise says so. */
void edgeLaneCost(App* app, CPU& cpu)
{
    costEdgeLane(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* A small cut to the second row's outer votes when a car is sensed close ahead in the left lane
 * (0x56ef38, its gap under 2: 0x56ee9c less 4) or in the right one (0x56ef40: 0x56eea4). */
void penaliseCloseCars(App* app)
{
    if (at(app, kSensed) != 0 && i32(at(app, kSensedGaps)) < 2)
        put(app, kLaneCostLo, at(app, kLaneCostLo) - 4);
    if (at(app, kSensed + 8) != 0 && i32(at(app, kSensedGaps + 8)) < 2)
        put(app, kLaneCostHi, at(app, kLaneCostHi) - 4);
}

/* 0x40d9f0: penaliseCloseCars. */
void edgePenalty(App* app, CPU& cpu)
{
    penaliseCloseCars(app);
    cpu.esp += 4;
}

/* The 14 rows of lane votes summed by lane. */
struct LaneSums
{
    reg32 left, middle, right;
};

LaneSums laneSums(App* app)
{
    LaneSums sums{0, 0, 0};
    for (reg32 row = 0; row < 0xe; ++row)
    {
        sums.left += at(app, row * 0xc + kLaneCostA);
        sums.middle += at(app, row * 0xc + kLaneCostB);
        sums.right += at(app, row * 0xc + kLaneCostC);
    }
    return sums;
}

/* The lane [0x56ef74] car takes: its slice +0x638, one left when the left lane's sum beats the
 * middle's, one right when the right's beats the better of the two, kept within 0..13. */
void takeLane(App* app, reg32 car)
{
    const LaneSums sums = laneSums(app);
    put(app, kChosenLane, at(app, car + kCarSlice));
    i32 best = i32(sums.middle);
    if (i32(sums.left) > best)
    {
        put(app, kChosenLane, at(app, car + kCarSlice) - 1);
        best = i32(sums.left);
    }
    if (best < i32(sums.right))
        put(app, kChosenLane, at(app, car + kCarSlice) + 1);
    const i32 chosen = i32(at(app, kChosenLane));
    if (chosen < 0)
        put(app, kChosenLane, 0);
    else if (chosen >= kSlices)
        put(app, kChosenLane, kSlices - 1);
}

/* 0x40da30: takeLane for car eax; eax as the code leaves it: the better sum of the left two, the
 * slice right of the car's when the right lane won, 0 when the lane came under 0. */
void sumLaneVotes(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const LaneSums sums = laneSums(app);
    takeLane(app, car);
    const reg32 slice = at(app, car + kCarSlice);
    reg32 left = sums.middle, lane = slice;
    if (i32(sums.left) > i32(sums.middle))
    {
        lane = slice - 1;
        left = sums.left;
    }
    if (i32(left) < i32(sums.right))
    {
        lane = slice + 1;
        left = lane;
    }
    if (i32(lane) < 0)
        left = 0;
    cpu.eax = left;
    cpu.esp += 4;
}

/* The lane planner started for car: the votes (0xa8 bytes at 0x56ee90) and the sensed cars (0x48
 * at 0x56ef38) cleared (0x4e0640), its road slices found (findCarSlices), its present lane voted
 * (voteForCurrentLane) and the traffic sensed (senseCars).  False when the game stopped. */
bool beginPlanning(App* app, CPU& cpu, reg32 car)
{
    if (!callGame(app, cpu, kMemset, {kLaneCostA, 0, 0xa8}) || !callGame(app, cpu, kMemset, {kSensed, 0, 0x48}))
        return false;
    findCarSlices(app, car);
    voteForCurrentLane(app, car);
    return senseCars(app, cpu, car);
}

/* 0x40dd60: beginPlanning for car eax.  What the code leaves in eax (senseTraffic's) no caller
 * reads; aiplan.inc's promise says so. */
void beginLanePlanning(App* app, CPU& cpu)
{
    if (beginPlanning(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* Car picks its lane: it looks a number of nodes ahead by its look-ahead (+0x7c4 by 2.5 and 1/6,
 * less a half, rounded; 9 at least: nodeAhead), weighs the edge (costEdgeLane) and the way there
 * (costLaneChange), scores the lanes by the road (scoreRoad) and, but for a player's car (+0x200 bit
 * 2), by the obstacles, the cars and the close ones.  False when the game stopped. */
bool pickLane(App* app, CPU& cpu, reg32 car)
{
    const i32 reach = fp::toInteger<i32>(fp::add(
        fp::mul(fp::mul(f32(app, car + kCarLookAhead), f64(app, kLaneReach)), f64(app, kLaneSixth)), f64(app, kLaneLessHalf)));
    const reg32 target = nodeAhead(app, car, reg32(reach <= 9 ? 9 : reach));
    costEdgeLane(app, car, target);
    costLaneChange(app, car, target);
    scoreRoad(app, car);
    if ((byte(app, car + kCarKindFlags) & kKindPlayer) != 0)
        return true;
    scoreObstacles(app, car);
    if (!scoreCars(app, cpu, car))
        return false;
    penaliseCloseCars(app);
    return true;
}

/* 0x40ddb0: pickLane for car eax; eax as the code leaves it: the car, or for a player's car what
 * scoreLanesByRoad leaves. */
void chooseLane(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!pickLane(app, cpu, car))
        return;
    cpu.eax = (byte(app, car + kCarKindFlags) & kKindPlayer) != 0 ? roadScoreLeft(app, car) : car;
    cpu.esp += 4;
}

/* Car's horn sounded for 0x40 ticks (+0x620 bit 3, its end +0x644 past the clock), its tone +0x660
 * on 0 -> 1 -> 2, any other back to 0. */
void soundHorn(App* app, reg32 car)
{
    putByte(app, car + kCarTrafficFlags, reg8(byte(app, car + kCarTrafficFlags) | kTrafficHorn));
    put(app, car + kCarHornEnd, at(app, kClock) + 0x40);
    const reg32 tone = at(app, car + kCarHornTone);
    put(app, car + kCarHornTone, tone == 0 ? 1 : tone == 1 ? 2 : 0);
}

/* 0x40de50: soundHorn for car eax. */
void startHorn(App* app, CPU& cpu)
{
    soundHorn(app, cpu.eax);
    cpu.esp += 4;
}

/* Car's horn ended once the clock passes +0x644: +0x620 bit 3 cleared, its tone +0x660 back (1 -> 0,
 * 2 -> 1). */
void endHorn(App* app, reg32 car)
{
    if ((byte(app, car + kCarTrafficFlags) & kTrafficHorn) == 0)
        return;
    if (!(i32(at(app, kClock)) > i32(at(app, car + kCarHornEnd))))
        return;
    putByte(app, car + kCarTrafficFlags, reg8(byte(app, car + kCarTrafficFlags) & ~kTrafficHorn));
    const reg32 tone = at(app, car + kCarHornTone);
    if (tone == 1)
        put(app, car + kCarHornTone, 0);
    else if (tone == 2)
        put(app, car + kCarHornTone, 1);
}

/* 0x40deb0: endHorn for car eax. */
void stopHorn(App* app, CPU& cpu)
{
    endHorn(app, cpu.eax);
    cpu.esp += 4;
}

/* Traffic car's loading steps (+0x630; 0x401240 and 0x405cd0..0x405e20 are below this file): 0 when
 * 0x401240 says it is not in play; step 0 (and past 2) placed (for +0x620 bit 1: 0x4a0aa0 at node 0
 * with the place at 0x40cb0c, in the code's frame, then 0x405d40) -- 0; 1 started (0x405e20,
 * 0x405dc0) -- 1, or 0 until it can; 2 running -- 1, or taken off (0x405cd0, 0x405d40) -- 0.  Nullopt
 * when the game stopped. */
std::optional<reg32> stepLoading(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x10);
    const std::optional<bool> inPlay = guest::direct<0x401240, airace::wasReleased>(app, cpu, car);
    if (!inPlay)
        return std::nullopt;
    if (!*inPlay)
        return 0;
    const reg32 step = at(app, car + kCarLoading);
    if (step == 1)
    {
        const std::optional<reg32> canStart = guest::direct<0x405e20, aidrive::stepExtraCar>(app, cpu, car);
        if (!canStart)
            return std::nullopt;
        if (*canStart == 0)
            return 0;
        if (!guest::direct<0x405dc0, aidrive::takeOff>(app, cpu, car))
            return std::nullopt;
        put(app, car + kCarLoading, 2);
        return 1;
    }
    if (step == 2)
    {
        const bool done = guest::direct<0x405cd0, aidrive::unwatched>(app, cpu, car);
        if (cpu.terminate)
            return std::nullopt;
        if (!done)
            return 1;
        if (!guest::direct<0x405d40, aidrive::trafficStartDone>(app, cpu, car))
            return std::nullopt;
        put(app, car + kCarLoading, 1);
        return 0;
    }
    const reg8 flags = byte(app, car + kCarTrafficFlags);
    put(app, car + kCarLoading, 2);
    if ((flags & 2) != 0)
    {
        const reg32 place = frame - 0x10;
        for (reg32 i = 0; i < 0xc; i += 4)
            put(app, place + i, at(app, kGridPlace + i));
        if (!placeOnRoad(app, cpu, car, 0, place, gameSettings(app).backwards))
            return std::nullopt;
        if (!guest::direct<0x405d40, aidrive::trafficStartDone>(app, cpu, car))
            return std::nullopt;
        put(app, car + kCarLoading, 1);
    }
    return 0;
}

/* 0x40df10: stepLoading for car eax. */
void stepTrafficLoading(App* app, CPU& cpu)
{
    const std::optional<reg32> running = stepLoading(app, cpu, cpu.eax);
    if (!running)
        return;
    cpu.eax = *running;
    cpu.esp += 4;
}

/* What scoreLanesByRoad's code leaves in eax: its node's record, or its right edge slice when it got
 * as far as reading it. */
reg32 roadScoreLeft(App* app, reg32 car)
{
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    return i32(at(app, car + kCarSlice)) + 1 < leftEdgeSlice(app, record) ? record : reg32(rightEdgeSlice(app, record));
}

/* Car's lanes scored by the road's shape (the fourth row, 0x56eeb4/b8/bc).  At its node, a lane on an
 * edge slice is cut by 0x1e, and by 0x32 more when that edge (+0x1c or +0x20 less its slices times the
 * half's width) is nearer than the car's half length +0x114 (as 16.16, rounded, worked out where the
 * code does); a lane off the road is cut hard (1000). */
void scoreRoad(App* app, reg32 car)
{
    const reg32 record = nodeAddr(app, at(app, car + kCarNode));
    const i32 edges = byte(app, record + kNodeEdges);
    const i32 high = (edges >> 4) & 0xf, low = edges & 0xf;
    const i32 edgeLeft = 6 - high, edgeRight = low + 7;
    const i32 slice = i32(at(app, car + kCarSlice));
    const i32 left = slice - 1, right = slice + 1;
    const i32 leftEdge = i32(at(app, record + kNodeLeftEdge) - (reg32(byte(app, record + kNodeLeftWidth)) << 15) * reg32(high));
    const i32 rightEdge =
        i32(at(app, record + kNodeRightEdge) - (reg32(byte(app, record + kNodeRightWidth)) << 15) * reg32(low));
    const auto length = [&] { return fp::toInteger<i32>(fp::mul(f32(app, car + kCarHalfLength), f64(app, kToFixedC))); };
    const auto cut = [&](reg32 vote, reg32 by) { put(app, vote, at(app, vote) - by); };
    if (left == edgeLeft || left == edgeRight)
    {
        cut(kNodeCostB, 0x1e);
        if (leftEdge < length())
            cut(kNodeCostB, 0x32);
    }
    if (slice == edgeLeft || edgeRight == slice)
    {
        cut(kNodeCostC, 0x1e);
        if (slice < 7)
        {
            if (leftEdge < length())
                cut(kNodeCostC, 0x32);
        }
        else if (rightEdge < length())
            cut(kNodeCostC, 0x32);
    }
    if (right == edgeLeft || edgeRight == right)
    {
        cut(kNodeCostD, 0x1e);
        if (rightEdge < length())
            cut(kNodeCostD, 0x32);
    }
    if (left < edgeLeft || left > edgeRight)
        cut(kNodeCostB, 0x3e8);
    if (edgeLeft > slice || edgeRight < slice)
        cut(kNodeCostC, 0x3e8);
    if (right < edgeLeft || right > edgeRight)
        cut(kNodeCostD, 0x3e8);
}

/* 0x40d3d0: scoreRoad for car eax; eax as the code leaves it (roadScoreLeft). */
void scoreLanesByRoad(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    scoreRoad(app, car);
    cpu.eax = roadScoreLeft(app, car);
    cpu.esp += 4;
}

/* Car's lanes scored against the parked obstacles (0x7dd850, 8 of 0x50 bytes: +0 in use, +0x34 its
 * node, +0x38 its slice): for each in use 1 to 5 nodes ahead in the car's direction of travel, the
 * lane (of the three about the car's slice +0x638) it stands in has its obstacle vote (0x56eecc +
 * lane) cut by 0x32, and in the car's own lane its speed asked +0x7b0 is eased to nine tenths. */
void scoreObstacles(App* app, reg32 car)
{
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 obstacle = i * kObstacleSize + kObstacles;
        if (at(app, obstacle) == 0)
            continue;
        const reg32 way =
            (((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u : 1u) ^ gameSettings(app).backwards) == 0 ? 0xffffffff : 1;
        const i32 ahead = i32(reg32(nodesBetween(app, at(app, obstacle + kObstacleNode), at(app, car + kCarNode))) * way);
        if (ahead <= 0 || ahead > 5)
            continue;
        for (reg32 lane = 0; lane < 3; ++lane)
        {
            if (at(app, car + kCarSlice) + lane - 1 != at(app, obstacle + kObstacleSlice))
                continue;
            put(app, lane * 4 + kObjCost, at(app, lane * 4 + kObjCost) - 0x32);
            if (lane == 1)
                putFloat(app, car + kCarTargetSpeed, float(fp::mul(f32(app, car + kCarTargetSpeed), f64(app, kEaseObstacle))));
        }
    }
}

/* 0x40d720: scoreObstacles for car eax.  What the code leaves in eax no caller reads; aiplan.inc's
 * promise says so. */
void scoreLanesByObstacles(App* app, CPU& cpu)
{
    scoreObstacles(app, cpu.eax);
    cpu.esp += 4;
}

/* A cut halved towards 0, as the code's mov, sar 31, sub, sar 1 has it. */
reg32 halved(reg32 cut)
{
    return reg32(i32(cut - reg32(i32(cut) >> 31)) >> 1);
}

/* Car's lanes scored against the cars sensed ahead in each of the three lanes (0x56ef38).  One coming
 * the other way (the speeds' signs, +0x7c0) closing faster than 1: the time to meet (its gap by 6 and
 * 65536, chopped by 0x4dfd56, as 1/65536 over the closing speed) by -35 and 1/8, rounded, less from -35
 * cuts that lane's oncoming vote (0x56eed8 + lane).  One going the same way that the car's speed asked
 * +0x7b0 outruns, within 10 nodes: -8, or less the more it outruns under 3 (by -5/3, rounded, less 3),
 * with up to the gap past 5 back, cuts its ahead vote (0x56eee4 + lane).  Halved where the car is not
 * in that lane by its middle (0x56ef68).  False when the game stopped. */
bool scoreCars(App* app, CPU& cpu, reg32 car)
{
    for (reg32 lane = 0; lane < 3; ++lane)
    {
        const reg32 other = at(app, lane * 4 + kSensed);
        if (other == 0)
            continue;
        const bool backwards = 0.0 > f32(app, car + kCarSpeed);
        const bool otherBackwards = 0.0 > f32(app, other + kCarSpeed);
        if (backwards != otherBackwards)
        {
            const float closing = float(fp::sub(f32(app, car + kCarSpeed), f32(app, other + kCarSpeed)));
            if (1.0 >= Float(sizeOf(closing)))
                continue;
            const std::optional<Float> meet = chop(
                app, cpu, fp::mul(fp::mul(Float(i32(at(app, lane * 4 + kSensedGaps))), f64(app, kMeetScale)), f64(app, kToFixedD)));
            if (!meet)
                return false;
            const i32 meetFixed = fp::toInteger<i32>(*meet);
            const Float time = fp::div(fp::mul(Float(meetFixed), f64(app, kFromFixed)), Float(closing));
            const i32 scaled = fp::toInteger<i32>(
                fp::add(fp::mul(fp::mul(time, f32(app, kMeetWeight)), f64(app, kEighth)), f64(app, kMeetLessHalf)));
            reg32 cut = reg32(-35) - reg32(scaled);
            if (app->getMemory<reg16>(lane * 2 + kSensedEdge) == 0)
                cut = halved(cut);
            if (i32(cut) < 0)
                put(app, lane * 4 + kCarCostLeft, at(app, lane * 4 + kCarCostLeft) + cut);
            continue;
        }
        const Float outrun = fp::sub(Float(sizeOf(app->getMemory<float>(car + kCarTargetSpeed))),
                                     Float(sizeOf(app->getMemory<float>(other + kCarSpeed))));
        if (0.0 >= outrun)
            continue;
        const i32 gap = i32(at(app, lane * 4 + kSensedGaps));
        if (gap >= 0xa)
            continue;
        reg32 cut = 0xfffffff8;
        if (!(outrun >= f64(app, kOutrunMost)))
            cut = reg32(fp::toInteger<i32>(fp::add(fp::mul(fp::mul(outrun, f64(app, kOutrunWeight)), f64(app, kThird)),
                                                   f64(app, kMeetLessHalf))))
                  - 3;
        cut += reg32(gap - 5 <= 0 ? 0 : gap - 5);
        if (app->getMemory<reg16>(lane * 2 + kSensedEdge) == 0)
            cut = halved(cut);
        if (i32(cut) < 0)
            put(app, lane * 4 + kCarCostRight, at(app, lane * 4 + kCarCostRight) + cut);
    }
    return true;
}

/* 0x40d800: scoreCars for car eax.  What the code leaves in eax (a compare's status word and the
 * like) no caller reads; aiplan.inc's promise says so. */
void scoreLanesByCars(App* app, CPU& cpu)
{
    if (scoreCars(app, cpu, cpu.eax))
        cpu.esp += 4;
}

/* Car's target place across +0x7b4 set for the chosen lane [0x56ef74].  On an edge slice of its node
 * (6 less the high nibble of +0x13, 7 more the low) it is the lane's offset from the middle times the
 * half's width (+0x17 left of slice 7, +0x1b right, << 15, as 1/65536), less or plus the car's half
 * length +0x114; else the same with half the width back towards the middle.  Then it is kept within
 * the node's right edge (+0x20, as 1/65536) less the half length and its left edge (+0x1c) plus it,
 * and moved by the half length the planner's way [0x56ef78] (the other way for a car against the
 * track).  The nodes' pointer written back, as the code does. */
void steerForLane(App* app, reg32 car)
{
    const reg32 nodes = at(app, kNodes);
    const reg32 record = nodes + at(app, car + kCarNode) * kNodeSize;
    const reg32 target = car + kCarTargetAcross;
    const i32 edges = byte(app, record + kNodeEdges);
    const i32 high = (edges >> 4) & 0xf, low = edges & 0xf;
    const i32 chosen = i32(at(app, kChosenLane));
    const i32 leftWidth = i32(byte(app, record + kNodeLeftWidth)) << 15;
    const i32 rightWidth = i32(byte(app, record + kNodeRightWidth)) << 15;
    const Float scale = f64(app, kLaneFixed);
    const Float halfLength = f32(app, car + kCarHalfLength);
    if (6 - high == chosen || low + 7 == chosen)
    {
        if (chosen < 7)
        {
            putFloat(app, target, float(fp::mul(fp::mul(Float(leftWidth), scale), Float(chosen - 6))));
            putFloat(app, target, float(fp::sub(f32(app, target), halfLength)));
        }
        else
        {
            putFloat(app, target, float(fp::mul(fp::mul(Float(rightWidth), scale), Float(chosen - 7))));
            putFloat(app, target, float(fp::add(halfLength, f32(app, target))));
        }
    }
    else
    {
        const bool leftSide = chosen < 7;
        const i32 width = leftSide ? leftWidth : rightWidth;
        putFloat(app, target, float(fp::mul(fp::mul(Float(width), scale), Float(leftSide ? chosen - 6 : chosen - 7))));
        const Float back = fp::mul(fp::mul(scale, Float(width)), f64(app, kHalfWidthBack));
        putFloat(app, target, float(leftSide ? fp::sub(f32(app, target), back) : fp::add(back, f32(app, target))));
    }
    // The bounds go through doubles in the code's frame: exact, the values being the x87's.
    const Float rightBound = fp::sub(fp::mul(Float(i32(at(app, record + kNodeRightEdge))), scale), f32(app, car + kCarHalfLength));
    if (f32(app, target) > rightBound)
        putFloat(app, target, float(rightBound));
    const Float leftBound = fp::sub(f32(app, car + kCarHalfLength), fp::mul(Float(i32(at(app, record + kNodeLeftEdge))), scale));
    if (!(f32(app, target) >= leftBound))
        putFloat(app, target, float(leftBound));
    if (at(app, kSensedDir) != 0)
    {
        const reg32 way = (byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u - at(app, kSensedDir) : at(app, kSensedDir);
        putFloat(app, target, float(fp::add(fp::mul(Float(i32(way)), f32(app, car + kCarHalfLength)), f32(app, target))));
    }
    put(app, kNodes, nodes);
}

/* 0x40daf0: steerForLane for car eax.  What the code leaves in eax (the nodes' pointer, a compare's
 * status word in its low half) no caller reads; aiplan.inc's promise says so. */
void steerToLane(App* app, CPU& cpu)
{
    steerForLane(app, cpu.eax);
    cpu.esp += 4;
}

/* The computer cars' target speed at slice `slice`: the byte of the track's speed table [0x552558]
 * at slice / 2, or 64 when that table failed to load [0x56f22c]. */
float targetSpeed(App* app, reg32 slice)
{
    if (at(app, kSpeedsFailed) != 0)
        return 64.0f;
    return float(byte(app, at(app, kSpeeds) + (slice >> 1)));  // fild of a byte: exact
}

/* 0x40e080: targetSpeed at slice eax onto the x87's stack; eax the table's byte, as the code leaves
 * it. */
void targetSpeedAt(App* app, CPU& cpu)
{
    const reg32 slice = cpu.eax;
    cpu.fpu.push(Float(targetSpeed(app, slice)));
    if (at(app, kSpeedsFailed) == 0)
        cpu.eax = byte(app, at(app, kSpeeds) + (slice >> 1));
    cpu.esp += 4;
}

/* The road's kind at slice `slice`: the high nibble of the speed table's second half's (0x55255c)
 * byte there; 7 when the table failed to load. */
reg32 roadKind(App* app, reg32 slice)
{
    if (at(app, kSpeedsFailed) != 0)
        return 7;
    return reg32(byte(app, at(app, kSpeedsHalf) + slice) >> 4);
}

/* 0x40e0c0: roadKind at slice eax. */
void roadKindAt(App* app, CPU& cpu)
{
    cpu.eax = roadKind(app, cpu.eax);
    cpu.esp += 4;
}

/* The track's road kind kept for the race: whether it is wide ([0x56f228], trackRoadWide), its
 * traffic side ([0x56f230], trackTrafficSide), the walk's counter [0x56f220] reset.  The side. */
reg32 cacheRoad(App* app)
{
    put(app, kSpeedsReady, trackRoadWide(app));
    const reg32 side = trackTrafficSide(app);
    put(app, kTrafficSide, side);
    put(app, kTrackWalked, 0);
    return side;
}

/* 0x40e0f0: cacheRoad, the side in eax. */
void cacheRoadWidth(App* app, CPU& cpu)
{
    cpu.eax = cacheRoad(app);
    cpu.esp += 4;
}

/* Node `node` less node `from`, wrapped round the track's [0x5dd940] nodes into -n/2..n/2; the
 * count written back, as the code does. */
i32 nodesBetween(App* app, reg32 node, reg32 from)
{
    const i32 count = i32(at(app, kNodeCount));
    const i32 half = count / 2;  // (n - (n >> 31)) >> 1: towards 0
    i32 gap = i32(node - from);
    if (gap > half)
        gap = i32(reg32(gap) - reg32(count));
    else if (gap < -half)
        gap = i32(reg32(gap) + reg32(count));
    put(app, kNodeCount, reg32(count));
    return gap;
}

/* 0x40e120: nodesBetween node eax and node edx. */
void nodeGap(App* app, CPU& cpu)
{
    cpu.eax = reg32(nodesBetween(app, cpu.eax, cpu.edx));
    cpu.esp += 4;
}

/* The node `steps` nodes on from car's node +0x1c in its direction of travel -- back for a car with
 * the track in a forward race, or one against it in a reversed race -- wrapped round the track; the
 * count written back, as the code does. */
reg32 nodeAhead(App* app, reg32 car, reg32 steps)
{
    const reg32 count = at(app, kNodeCount);
    const reg32 forward = ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0 : 1) ^ gameSettings(app).backwards;
    if (forward == 0)
        steps = 0u - steps;
    reg32 node = steps + at(app, car + kCarNode);
    if (i32(steps) < 0)
    {
        if (i32(node) < 0)
            node += count;
    }
    else if (i32(node) >= i32(count))
        node -= count;
    put(app, kNodeCount, count);
    return node;
}

/* 0x40e160: nodeAhead for car eax by edx nodes. */
void nodeOffset(App* app, CPU& cpu)
{
    cpu.eax = nodeAhead(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* The node car will reach: its look-ahead +0x7c4 times `scale` and 1/6, rounded, kept within `least`
 * and `most` (the most winning), backwards when +0x7d4 is 1 (nodeAhead). */
reg32 nodeAheadBySpeed(App* app, reg32 car, float scale, i32 least, i32 most)
{
    const i32 reach = fp::toInteger<i32>(
        fp::mul(fp::mul(f32(app, car + kCarLookAhead), Float(scale)), f64(app, kSixth)));
    i32 steps = reach < least ? least : reach;
    if (steps > most)
        steps = most;
    if (at(app, car + kCarBackwards) == 1)
        steps = -steps;
    return nodeAhead(app, car, reg32(steps));
}

/* 0x40e1b0 (stdcall, 3 words: the scale, the least, the most): nodeAheadBySpeed for car eax. */
void nodesAheadBySpeed(App* app, CPU& cpu)
{
    cpu.eax = nodeAheadBySpeed(app, cpu.eax, floatOf(at(app, cpu.esp + 4)), i32(at(app, cpu.esp + 8)),
                               i32(at(app, cpu.esp + 0xc)));
    cpu.esp += 4 + 0xc;
}

/* Node `node`'s lane bits (the high half of its dword at +0xa, arithmetically) masked by `mask` --
 * which of the 16 lanes are road there. */
reg32 laneBitsAt(App* app, reg32 node, reg32 mask)
{
    return reg32(i32(at(app, nodeAddr(app, node) + kNodeLaneBits)) >> 16) & mask;
}

/* 0x40e200: laneBitsAt node eax by mask edx. */
void nodeLaneBits(App* app, CPU& cpu)
{
    cpu.eax = laneBitsAt(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* Whether node `node` has road in slice `slice`: laneBitsAt with the slice's bit (sliceBit). */
reg32 hasLane(App* app, reg32 node, reg32 slice)
{
    return laneBitsAt(app, node, sliceBit(slice));
}

/* 0x40e220: hasLane at node eax in slice edx. */
void nodeHasLane(App* app, CPU& cpu)
{
    cpu.eax = hasLane(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* Slice `slice`'s bit among a node's 16 lane bits: bit 15 - (slice + 1), slice + 1 (as the code's
 * 8 - (7 - slice)) kept within 0..15. */
reg32 sliceBit(reg32 slice)
{
    i32 lane = i32(8u - (7u - slice));
    if (lane < 0)
        lane = 0;
    else if (lane >= 0x10)
        lane = 0xf;
    return reg32(1) << (0xf - lane);
}

/* 0x40e270: sliceBit of slice eax. */
void laneBit(App* app, CPU& cpu)
{
    cpu.eax = sliceBit(cpu.eax);
    cpu.esp += 4;
}

/* The road slice (0..13) `offset` across the road falls in at node `node`: from the centre slice
 * (6 left of it, 7 right), the half's width (+0x17 / +0x1b, << 15, as 1/65536) by four the index
 * into the lane-width table 0x56f0e0, whose entry times the offset is the slices from the centre.
 * Left is an offset under 0 (fcom: a NaN goes right). */
reg32 roadSlice(App* app, reg32 node, float offset)
{
    const reg32 record = at(app, kNodes) + node * kNodeSize;
    const bool left = 0.0 > Float(offset);
    const i32 centre = left ? 6 : 7;
    const reg32 width = byte(app, record + (left ? kNodeLeftWidth : kNodeRightWidth));
    const float halfWidth = float(fp::mul(Float(i32(width << 0xf)), f64(app, kFixedToFloat)));
    const i32 index = fp::toInteger<i32>(fp::mul(Float(halfWidth), f32(app, kQuarterLanes)));
    const i32 slices = fp::toInteger<i32>(fp::mul(f32(app, reg32(index) * 4 + kLaneWidths), Float(offset)));
    i32 slice = centre + slices;
    if (slice < 0)
        slice = 0;
    if (slice > kSlices - 1)
        slice = kSlices - 1;
    return reg32(slice);
}

/* 0x40e2b0 (stdcall, 1 word: the offset across the road): roadSlice at node eax. */
void roadSliceIndex(App* app, CPU& cpu)
{
    cpu.eax = roadSlice(app, cpu.eax, floatOf(at(app, cpu.esp + 4)));
    cpu.esp += 4 + 4;
}

/* Car's three road slices at its node +0x1c (roadSlice): of its place across the road +0x7b8
 * (+0x638), and of its two sides, -+0x21c with half a slice back in (+0x63c, +0x640). */
void findCarSlices(App* app, reg32 car)
{
    const reg32 node = at(app, car + kCarNode);
    const reg32 middle = roadSlice(app, node, app->getMemory<float>(car + kCarAcross));
    const Float leftSide = fp::sub(f32(app, car + kCarAcross), f32(app, car + kCarHalfWidth));
    put(app, car + kCarSlice, middle);
    const reg32 left = roadSlice(app, node, float(fp::add(leftSide, f64(app, kHalfSlice))));
    const Float rightSide = fp::add(f32(app, car + kCarAcross), f32(app, car + kCarHalfWidth));
    put(app, car + kCarLeftSlice, left);
    const reg32 right = roadSlice(app, node, float(fp::add(rightSide, f64(app, kLessHalfSlice))));
    put(app, car + kCarRightSlice, right);
}

/* 0x40e350: findCarSlices for car eax; eax the last slice, as the code leaves it. */
void carSlices(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    findCarSlices(app, car);
    cpu.eax = at(app, car + kCarRightSlice);
    cpu.esp += 4;
}

/* A track's tuning record at `record`: `first` and `distance` (0xf and 0 in a knock-out race,
 * [0x6fd4fc] bit 3), whether the road is wide, the traffic's side, the two speeds and the last. */
void fillTuning(App* app, reg32 record, reg32 first, reg32 distance, reg32 roadWide, reg32 side, reg32 speed,
                reg32 seriesSpeed, reg32 last)
{
    if ((std::uint8_t(gameSettings(app).raceFlags) & 8) != 0)
    {
        put(app, record + kTuningDistance, 0);
        put(app, record + kTuningFirst, 0xf);
    }
    else
    {
        put(app, record + kTuningDistance, distance);
        put(app, record + kTuningFirst, first);
    }
    put(app, record + kTuningRoadWide, roadWide);
    put(app, record + kTuningSide, side);
    put(app, record + kTuningSpeed, speed);
    put(app, record + kTuningSeriesSpeed, seriesSpeed);
    put(app, record + kTuningLast, last);
}

/* 0x40e3c0 (stdcall, 4 words: the side, the speeds, the last): fillTuning of the record at eax with
 * edx, ebx and ecx. */
void fillTrackTuning(App* app, CPU& cpu)
{
    fillTuning(app, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4), at(app, cpu.esp + 8),
               at(app, cpu.esp + 0xc), at(app, cpu.esp + 0x10));
    cpu.esp += 4 + 0x10;
}

/* The race's tuning set up.  A handicap (-0.005 at night, and for each of the weather, the mirrored
 * and the reversed track) is summed; the lane-width table 0x56f0e0 filled (0, then 1 / (i / 4)); then
 * by the race type [0x6fd3b8] (the jump table at 0x40e400) the nine tracks' tuning records at 0x56ef80
 * filled (fillTuning: the side 1, the road not wide, 13.41 the last) -- fixed values for the single and
 * the cop races, for the two series by their variant: the handicap's (0.98, 0.99 or 1 more) for
 * variant 1, fixed values else -- the fourth record overridden by an option [0x55eb2e] bit 5; the
 * opponents' speeds loaded (0x40e000) and the highest of the computer's target speeds over the track's
 * nodes kept in [0x56f224].  False when the game stopped. */
bool setupTuning(App* app, CPU& cpu)
{
    constexpr reg32 kLast = 0x4156930c;  // 13.41
    const auto tune = [&](reg32 record, reg32 distance, reg32 first, reg32 speed, reg32 seriesSpeed) {
        fillTuning(app, record, first, distance, 0, 1, speed, seriesSpeed, kLast);
    };
    float handicap = floatOf(gameSettings(app).night != 0 ? 0xbba3d70a : 0);
    for (reg32 GameSettings::*flag : {&GameSettings::weather, &GameSettings::mirrored, &GameSettings::backwards})
        if (gameSettings(app).*flag != 0)
            handicap = float(fp::add(Float(handicap), f32(app, kHandicapStep)));
    put(app, kLaneWidths, 0);
    for (i32 i = 1; i < 0x50; ++i)
        putFloat(app, reg32(i) * 4 + kLaneWidths, float(fp::div(1.0, fp::mul(Float(i), f32(app, kQuarter)))));
    const reg32 raceType = gameSettings(app).raceType;
    if (raceType == 0)
    {
        static const reg32 rows[9][5] = {
            {0x56ef80, 0x28, 8, 0x3f51eb85, 0x3f70a3d7}, {0x56efa0, 0x78, 6, 0x3f570a3d, 0x3f75c28f},
            {0x56efc0, 0x50, 9, 0x3f570a3d, 0x3f75c28f}, {0x56efe0, 0x64, 5, 0x3f51eb85, 0x3f70a3d7},
            {0x56f000, 0x28, 7, 0x3f570a3d, 0x3f75c28f}, {0x56f020, 0x78, 6, 0x3f51eb85, 0x3f70a3d7},
            {0x56f040, 0x50, 5, 0x3f570a3d, 0x3f75c28f}, {0x56f060, 0x50, 5, 0x3f51eb85, 0x3f70a3d7},
            {0x56f080, 0x14, 0xc, 0x3f570a3d, 0x3f75c28f}};
        for (const auto& r : rows)
            tune(r[0], r[1], r[2], r[3], r[4]);
    }
    else if (raceType == 3)
    {
        static const reg32 rows[9][5] = {
            {0x56ef80, 0x28, 8, 0x3f47ae14, 0x3f6147ae}, {0x56efa0, 0x78, 6, 0x3f51eb85, 0x3f666666},
            {0x56efc0, 0x50, 9, 0x3f4ccccd, 0x3f59999a}, {0x56efe0, 0x64, 5, 0x3f4ccccd, 0x3f59999a},
            {0x56f000, 0x28, 7, 0x3f4ccccd, 0x3f59999a}, {0x56f020, 0x78, 6, 0x3f4ccccd, 0x3f59999a},
            {0x56f040, 0x50, 5, 0x3f4ccccd, 0x3f59999a}, {0x56f060, 0x50, 5, 0x3f4ccccd, 0x3f59999a},
            {0x56f080, 0x14, 0xc, 0x3f4ccccd, 0x3f547ae1}};
        for (const auto& r : rows)
            tune(r[0], r[1], r[2], r[3], r[4]);
    }
    else if (raceType <= 2)
    {
        // The two series: the knock-out (2) and the tournament (1).
        const bool knockout = raceType == 2;
        if (gameSettings(app).seriesVariant == 1)
        {
            const reg32 slower = bitsOf(float(fp::add(Float(handicap), f64(app, kHandicapSlower))));
            const reg32 slow = bitsOf(float(fp::add(Float(handicap), f64(app, kHandicapSlow))));
            const reg32 even = bitsOf(float(fp::add(1.0, Float(handicap))));
            tune(0x56ef80, 0, 0, 0, slower);
            tune(0x56efa0, 0, 0, 0, slow);
            tune(0x56efc0, 0, 0, 0, even);
            tune(0x56efe0, 0, 0, 0, slower);
            tune(0x56f000, 0, 0, 0, slow);
            tune(0x56f020, 0, 0, 0, slower);
            tune(0x56f040, 0, 0, 0, slow);
            tune(0x56f060, 0, 0, 0, slower);
            tune(0x56f080, 0, 0, 0, knockout ? even : slow);
        }
        else
        {
            static const reg32 knockouts[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                               0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f800000};
            static const reg32 tournaments[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                                 0x3f6b851f, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7};
            for (reg32 i = 0; i < 9; ++i)
                tune(kTuning + kTuningSize * i, 0, 0, 0, knockout ? knockouts[i] : tournaments[i]);
        }
    }
    if ((byte(app, kStartFlags) & 0x20) != 0)
        tune(0x56efe0, 0x64, 1, 0x3f666666, 0x3f800000);
    if (!guest::direct<0x40e000, load::loadSpeeds>(app, cpu))
        return false;
    const i32 nodes = i32(at(app, kNodeCount));
    put(app, kTopTargetSpeed, 0);
    for (i32 node = 0; node < nodes; ++node)
    {
        const float speed = targetSpeed(app, reg32(node));
        if (Float(speed) > f32(app, kTopTargetSpeed))
            putFloat(app, kTopTargetSpeed, speed);
    }
    return true;
}

/* 0x40e410: setupTuning.  What the code leaves in eax (a compare's status word) no caller reads;
 * aiplan.inc's promise says so. */
void setupTrackTuning(App* app, CPU& cpu)
{
    if (setupTuning(app, cpu))
        cpu.esp += 4;
}

/* The opponents' speed table [0x552558] freed and forgotten; false when the game stopped. */
bool freeSpeeds(App* app, CPU& cpu)
{
    const reg32 table = at(app, kSpeeds);
    if (table != 0)
    {
        if (!freeHandle(app, cpu, table))
            return false;
        put(app, kSpeeds, 0);
    }
    return true;
}

/* 0x40ec40: freeSpeeds; eax as the free leaves it. */
void freeSpeedTable(App* app, CPU& cpu)
{
    if (freeSpeeds(app, cpu))
        cpu.esp += 4;
}

/* The track's traffic side: its tuning record's +0xc; 1 past the records. */
reg32 trackTrafficSide(App* app)
{
    const i32 track = i32(gameSettings(app).track);
    if (track < kTrackCount)
        return at(app, reg32(track) * kTuningSize + kTuning + kTuningSide);
    return 1;
}

/* 0x40ec70: trackTrafficSide. */
void trafficSide(App* app, CPU& cpu)
{
    cpu.eax = trackTrafficSide(app);
    cpu.esp += 4;
}

/* Whether the track's road is wide: its tuning record's +8; past the records, whether the second
 * node's left edge slices (the high nibble of its +0x13) are none. */
reg32 trackRoadWide(App* app)
{
    const i32 track = i32(gameSettings(app).track);
    if (track < kTrackCount)
        return at(app, reg32(track) * kTuningSize + kTuning + kTuningRoadWide);
    return (byte(app, at(app, kNodes) + kNodeSize + kNodeEdges) >> 4) == 0 ? 1 : 0;
}

/* 0x40eca0: trackRoadWide. */
void roadIsWide(App* app, CPU& cpu)
{
    cpu.eax = trackRoadWide(app);
    cpu.esp += 4;
}

/* The traffic's speed at node `node` by the track [0x6fd4ac] (the jump table at 0x40ed14) and the
 * stretch of it: slower through each track's hard stretches.  The speeds as the code has their
 * bits. */
float trafficSpeed(App* app, reg32 node)
{
    reg32 bits = 0x41a0ee49;
    switch (gameSettings(app).track)
    {
    case 0: bits = node > 0x276 || node <= 0x1e8 ? 0x41b2cfdf : 0x4156930c; break;
    case 1:
        bits = node > 0x3e8 ? 0x41c4b176 : node > 0x316 ? 0x41fa5639 : node > 0xaa ? 0x41e874a2 : 0x41c4b176;
        break;
    case 2: bits = 0x41b2cfdf; break;
    case 3: bits = node > 0x56e || node <= 0x528 ? 0x41b1c77a : 0x415e3958; break;
    case 4: bits = node > 0x153 || node <= 0x127 ? 0x41b2cfdf : 0x410f0cb3; break;
    case 5: case 6: bits = 0x41b2cfdf; break;
    case 7: bits = node > 0x65e || node <= 0x60d ? 0x41b1c77a : 0x415e3958; break;
    case 8: bits = node > 0x3e8 || node <= 0x190 ? 0x41a0ee49 : 0x417a5639; break;
    default: break;  // 9, 10 and past the table
    }
    return floatOf(bits);
}

/* 0x40ed40: trafficSpeed at node eax onto the x87's stack. */
void trafficSpeedAt(App* app, CPU& cpu)
{
    cpu.fpu.push(Float(trafficSpeed(app, cpu.eax)));
    cpu.esp += 4;
}

/* The track's tuning speed: its record's +0x18 in the race types 1 and 2 or in a series race, else
 * +0x14. */
float trackTuningSpeed(App* app)
{
    const reg32 raceType = gameSettings(app).raceType;
    const reg32 record = gameSettings(app).track * kTuningSize + kTuning;
    const bool series = raceType == 1 || raceType == 2 || gameSettings(app).seriesRace != 0;
    return app->getMemory<float>(record + (series ? kTuningSeriesSpeed : kTuningSpeed));
}

/* 0x40ee80: trackTuningSpeed onto the x87's stack; every register kept. */
void tuningSpeed(App* app, CPU& cpu)
{
    cpu.fpu.push(Float(trackTuningSpeed(app)));
    cpu.esp += 4;
}

/* The track's tuning distance (its record's +4), +200 with the weather on and +400 at night, as
 * game_settings.h names the two. */
reg32 trackTuningDistance(App* app)
{
    reg32 distance = at(app, gameSettings(app).track * kTuningSize + kTuning + kTuningDistance);
    if (gameSettings(app).weather != 0)
        distance += 0xc8;
    if (gameSettings(app).night != 0)
        distance += 0x190;
    return distance;
}

/* 0x40eed0: trackTuningDistance. */
void tuningDistance(App* app, CPU& cpu)
{
    cpu.eax = trackTuningDistance(app);
    cpu.esp += 4;
}

/* The track's tuning record's +0. */
reg32 trackTuningFirst(App* app)
{
    return at(app, gameSettings(app).track * kTuningSize + kTuning + kTuningFirst);
}

/* 0x40ef10: trackTuningFirst. */
void tuningFirst(App* app, CPU& cpu)
{
    cpu.eax = trackTuningFirst(app);
    cpu.esp += 4;
}

/* A node's forward direction's part at +0x14 or +0x16 as the code loads it: the signed byte << 9. */
i32 directionPart(App* app, reg32 record, reg32 part)
{
    return i32(reg32(signedByte(app, record + part)) << 9);
}

/* The dot product of nodes `a`'s and `b`'s forward directions (directionPart, as 1/65536), in the
 * code's order: z's product, then x's added. */
Float directionDot(App* app, reg32 a, reg32 b)
{
    const reg32 nodes = at(app, kNodes);
    const reg32 recordB = nodes + b * kNodeSize, recordA = nodes + a * kNodeSize;
    const Float scale = f64(app, kDirectionScale);
    const Float xB = fp::mul(Float(directionPart(app, recordB, kNodeDirectionX)), scale);
    const Float xA = fp::mul(Float(directionPart(app, recordA, kNodeDirectionX)), scale);
    const Float x = fp::mul(xB, xA);
    const Float zB = fp::mul(Float(directionPart(app, recordB, kNodeDirectionZ)), scale);
    const Float zA = fp::mul(scale, Float(directionPart(app, recordA, kNodeDirectionZ)));
    return fp::add(fp::mul(zA, zB), x);
}

/* 0x40ef70: directionDot of nodes eax and edx onto the x87's stack; eax node eax's z part, as the
 * code leaves it. */
void nodeDirectionDot(App* app, CPU& cpu)
{
    const reg32 a = cpu.eax;
    cpu.fpu.push(directionDot(app, a, cpu.edx));
    cpu.eax = reg32(directionPart(app, at(app, kNodes) + a * kNodeSize, kNodeDirectionZ));
    cpu.esp += 4;
}

/* Whether node `node` lies in the run of `run` nodes from node `from` (wrapped round the track; a
 * negative run counts back), the ends' compares unsigned as the code's; the count written back. */
bool nodeInRun(App* app, reg32 node, reg32 from, reg32 run)
{
    const reg32 count = at(app, kNodeCount);
    reg32 end = from + run;
    if (i32(run) >= 0)
    {
        if (i32(end) >= i32(count))
            end -= count;
    }
    else if (i32(end) < 0)
        end += count;
    const bool in = end < from ? node >= from || node <= end : node >= from && node <= end;
    put(app, kNodeCount, count);
    return in;
}

/* 0x40f000: nodeInRun for node eax, from edx, ebx long. */
void nodeWithinRange(App* app, CPU& cpu)
{
    cpu.eax = nodeInRun(app, cpu.eax, cpu.edx, cpu.ebx) ? 1 : 0;
    cpu.esp += 4;
}

/* Whether car is within reach of `distance`: its place along the track +8 and 14 times its speed
 * along it +0x1fc against the distance and a margin by the race's laps [0x5efd94] (26 less in a
 * reversed race). */
bool carInReach(App* app, reg32 car, reg32 distance)
{
    reg32 margin = 0x190;
    if (i32(at(app, kLaps)) < 5)
        margin = 0xc8;
    if (i32(at(app, kLaps)) < 2)
        margin = 0x32;
    if (gameSettings(app).backwards != 0)
        margin -= 0x1a;
    const reg32 reached = at(app, car + kCarSpeedAlong) * 14 + at(app, car + kCarAlong);
    return !(i32(reached) > i32(margin + distance));
}

/* 0x40f050: carInReach for car eax of distance edx. */
void carWithinReach(App* app, CPU& cpu)
{
    cpu.eax = carInReach(app, cpu.eax, cpu.edx) ? 1 : 0;
    cpu.esp += 4;
}

/* 1, whatever it is asked. */
reg32 answerOne(App* app)
{
    (void)app;
    return 1;
}

/* 0x40f100: answerOne. */
void alwaysOne(App* app, CPU& cpu)
{
    cpu.eax = answerOne(app);
    cpu.esp += 4;
}

/* Whether car is boxed in: its slice +0x638 off the 14, or not within its node's edge slices and
 * neither neighbouring slice within them (and the 14); the nodes' pointer written back, as the code
 * does. */
bool boxedIn(App* app, reg32 car)
{
    const reg32 nodes = at(app, kNodes);
    const i32 slice = i32(at(app, car + kCarSlice));
    const i32 edges = byte(app, nodes + at(app, car + kCarNode) * kNodeSize + kNodeEdges);
    const i32 edgeLeft = 6 - ((edges >> 4) & 0xf), edgeRight = (edges & 0xf) + 7;
    bool boxed = true;
    if (slice >= 0 && slice < kSlices)
    {
        const auto free = [&](i32 s) { return s >= edgeLeft && s <= edgeRight && s >= 0 && s < kSlices; };
        if (!(edgeLeft > slice) && edgeRight >= slice)
            boxed = false;
        else if (free(slice - 1) || free(slice + 1))
            boxed = false;
    }
    put(app, kNodes, nodes);
    return boxed;
}

/* 0x40f110: boxedIn for car eax. */
void carBoxedIn(App* app, CPU& cpu)
{
    cpu.eax = boxedIn(app, cpu.eax) ? 1 : 0;
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

/* The seven animation paths (0x570a34) freed (0x4e1890) and forgotten; false when the game
 * stopped. */
bool freePaths(App* app, CPU& cpu)
{
    for (reg32 i = 0; i < 7; ++i)
    {
        const reg32 path = at(app, i * 4 + kAnimPaths);
        if (path == 0)
            continue;
        if (!freeHandle(app, cpu, path))
            return false;
        put(app, i * 4 + kAnimPaths, 0);
    }
    return true;
}

/* 0x40f330: freePaths; eax as the last free leaves it. */
void freeAnimationPaths(App* app, CPU& cpu)
{
    if (freePaths(app, cpu))
        cpu.esp += 4;
}

/* The first free animation slot (its path +0 none), or -1. */
reg32 freeAnimation(App* app)
{
    for (reg32 slot = 0; slot < kAnimSlots; ++slot)
        if (at(app, slot * kAnimSize + kAnims) == 0)
            return slot;
    return 0xffffffff;
}

/* 0x40f370: freeAnimation. */
void findFreeAnimation(App* app, CPU& cpu)
{
    cpu.eax = freeAnimation(app);
    cpu.esp += 4;
}

/* Path `path` (0..6 of the handles) started in a free slot at the clock, mode 2, flags 1, the third
 * word 0 -- the slot, or -1 when none is free or the path is not loaded (its slot's path written 0
 * then, as the code does). */
reg32 playAnimation(App* app, reg32 path)
{
    const reg32 slot = freeAnimation(app);
    if (slot == 0xffffffff)
        return slot;
    if (i32(path) >= i32(kAnimSlots))
        return 0xffffffff;
    const reg32 record = slot * kAnimSize;
    const reg32 handle = at(app, path * 4 + kAnimPaths);
    put(app, record + kAnims, handle);
    if (handle == 0)
        return 0xffffffff;
    put(app, record + kAnimStart, at(app, kClock));
    put(app, record + kAnimMode, 2);
    put(app, record + kAnimFlags, 1);
    put(app, record + kAnimThird, 0);
    return slot;
}

/* 0x40f3b0: playAnimation of path eax. */
void startAnimation(App* app, CPU& cpu)
{
    cpu.eax = playAnimation(app, cpu.eax);
    cpu.esp += 4;
}

/* Slot's mode, flags and third word set. */
void setAnimation(App* app, reg32 slot, reg32 mode, reg32 flags, reg32 third)
{
    put(app, slot * kAnimSize + kAnimMode, mode);
    put(app, slot * kAnimSize + kAnimFlags, flags);
    put(app, slot * kAnimSize + kAnimThird, third);
}

/* 0x40f430: setAnimation of slot eax to edx, ebx, ecx; eax three times the slot, as the code leaves
 * it. */
void setAnimationMode(App* app, CPU& cpu)
{
    setAnimation(app, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
    cpu.eax *= 3;
    cpu.esp += 4;
}

/* Slot stopped -- 1, or -1 when it was not playing. */
reg32 endAnimation(App* app, reg32 slot)
{
    const reg32 record = slot * kAnimSize;
    if (at(app, record + kAnims) == 0)
        return 0xffffffff;
    put(app, record + kAnimStart, 0xffffffff);
    put(app, record + kAnimMark, 0);
    put(app, record + kAnims, 0);
    return 1;
}

/* 0x40f460: endAnimation of slot eax. */
void stopAnimation(App* app, CPU& cpu)
{
    cpu.eax = endAnimation(app, cpu.eax);
    cpu.esp += 4;
}

/* A path's frames per point (+6: 1..400, else 6). */
i32 pathFrames(App* app, reg32 path)
{
    const reg16 frames = app->getMemory<reg16>(path + kPathFrames);
    return frames >= 1 && frames <= 0x190 ? i32(frames) : 6;
}

/* A path's points less one (+4, a word). */
i32 pathSpans(App* app, reg32 path)
{
    return i32(reg32(app->getMemory<reg16>(path + kPathPoints)) - 1);
}

/* cdq; idiv: the quotient and the remainder, as x86 has them. */
i32 quotient(i32 n, i32 by) { return i32(std::int64_t(n) / by); }
i32 remainder(i32 n, i32 by) { return i32(std::int64_t(n) % by); }

/* A point's orientation's part (one of four shorts from +0xc) as the code loads it: << 2. */
i32 orientationPart(App* app, reg32 point, reg32 part)
{
    return i32(reg32(i32(sreg16(app->getMemory<reg16>(point + kPointOrientation + part * 2)))) << 2);
}

/* Path's pose at `time` (in frames; mode 2 plays once): the two points the time falls between
 * (looping over the points, unless played once and past the end -- then 0) give two orientations
 * (four shorts, << 2, as 1/65536) and two places (16.16, as 1/65536), blended at the fraction of the
 * frame (0x4b70f0, `blendWord` handed on as its last word) and made a matrix at `matrix` (0x4b7590).
 * The orientations, the places and the blend's room in the code's frame, whose addresses the blend is
 * handed (and logs, in the check).  1; nullopt when the game stopped. */
std::optional<reg32> poseAt(App* app, CPU& cpu, reg32 path, reg32 mode, i32 time, reg32 blendWord, reg32 matrix)
{
    const GuestFrame frame(cpu, 0xc, 0x7c);
    const i32 frames = pathFrames(app, path);
    const i32 step = quotient(time, frames);
    if (mode == 2 && step >= i32(reg32(app->getMemory<reg16>(path + kPathPoints)) - 1))
        return 0;
    const i32 index = remainder(step, pathSpans(app, path));
    const i32 rest = remainder(time, frames);
    const reg32 point = path + kPathPointList + reg32(index) * kPointSize;
    const reg32 nextPoint = path + kPathPointList + reg32(index + 1) * kPointSize;
    const Float scale = f64(app, kPoseScale);
    const reg32 first = frame - 0x5c, second = frame - 0x6c, firstPlace = frame - 0x4c, secondPlace = frame - 0x34;
    // The first orientation, at -0x5c down to -0x50.
    Float parts[4];
    for (reg32 i = 0; i < 4; ++i)
        parts[i] = fp::mul(Float(orientationPart(app, point, i)), scale);
    for (reg32 i = 0; i < 4; ++i)
        putFloat(app, first + 4 * i, float(parts[i]));
    // The second, at -0x6c down to -0x60; the fraction of the frame between.
    const Float part0 = fp::mul(Float(orientationPart(app, nextPoint, 0)), scale);
    const Float fraction = fp::div(Float(rest), Float(frames));
    for (reg32 i = 1; i < 4; ++i)
        parts[i] = fp::mul(Float(orientationPart(app, nextPoint, i)), scale);
    putFloat(app, second, float(part0));
    for (reg32 i = 1; i < 4; ++i)
        putFloat(app, second + 4 * i, float(parts[i]));
    // The places, at -0x4c and -0x34 (the last part's product the other way round, as the code's).
    const auto place = [&](reg32 at_, reg32 to) {
        const Float x = fp::mul(Float(i32(at(app, at_))), scale);
        const Float y = fp::mul(Float(i32(at(app, at_ + 4))), scale);
        const Float z = fp::mul(scale, Float(i32(at(app, at_ + 8))));
        putFloat(app, to, float(x));
        putFloat(app, to + 4, float(y));
        putFloat(app, to + 8, float(z));
    };
    place(point, firstPlace);
    place(nextPoint, secondPlace);
    if (!callGame(app, cpu, kPoseBlend, {first, second, firstPlace, secondPlace},
                  {bitsOf(float(fraction)), frame - 0x7c, blendWord}))
        return std::nullopt;
    if (!callGame(app, cpu, kPoseMatrix, {frame - 0x7c, matrix}))
        return std::nullopt;
    return 1;
}

/* 0x40f4b0 (stdcall, 1 word: where the pose's matrix goes): poseAt for path eax at time ebx in mode
 * edx, ecx handed on. */
void animationPose(App* app, CPU& cpu)
{
    const std::optional<reg32> posed =
        poseAt(app, cpu, cpu.eax, cpu.edx, i32(cpu.ebx), cpu.ecx, at(app, cpu.esp + 4));
    if (!posed)
        return;
    cpu.eax = *posed;
    cpu.esp += 4 + 4;
}

/* Path's place at `time` (`mode` bit 1: played once -- 0 past the end): the two points' places blended
 * at the frame's 16.16 fraction (0x4b6a30, `blendWord` handed on; the places and the blend's room in
 * the code's frame, whose addresses it is handed); the time within the path's length and the length
 * (both in frames) to `timeOut` and `lengthOut`.  1; nullopt when the game stopped. */
std::optional<reg32> placeAt(App* app, CPU& cpu, reg32 path, reg32 mode, i32 time, reg32 blendWord, reg32 timeOut,
                             reg32 lengthOut)
{
    const GuestFrame frame(cpu, 0xc, 0x44);
    const i32 frames = pathFrames(app, path);
    const i32 step = quotient(time, frames);
    if ((mode & 2) != 0 && step >= i32(reg32(app->getMemory<reg16>(path + kPathPoints)) - 1))
        return 0;
    const i32 index = remainder(step, pathSpans(app, path));
    const i32 fraction = quotient(i32(reg32(remainder(time, frames)) << 0x10), frames);
    const reg32 first = frame - 0x44, second = frame - 0x38;
    for (reg32 i = 0; i < 0xc; i += 4)
        put(app, first + i, at(app, path + kPathPointList + reg32(index) * kPointSize + i));
    for (reg32 i = 0; i < 0xc; i += 4)
        put(app, second + i, at(app, path + kPathPointList + reg32(index + 1) * kPointSize + i));
    if (!callGame(app, cpu, kPlaceBlend, {frame - 0x1c, frame - 0x24, first, second},
                  {reg32(fraction), frame - 0x2c, blendWord}))
        return std::nullopt;
    const i32 length = i32(reg32(frames) * reg32(pathSpans(app, path)));
    put(app, timeOut, reg32(remainder(time, length)));
    put(app, lengthOut, reg32(frames) * reg32(pathSpans(app, path)));
    return 1;
}

/* 0x40f6d0 (stdcall, 2 words: where the time into the path goes, where its length goes): placeAt for
 * path eax at time ebx in mode edx, ecx handed on. */
void animationPlace(App* app, CPU& cpu)
{
    const std::optional<reg32> placed = placeAt(app, cpu, cpu.eax, cpu.edx, i32(cpu.ebx), cpu.ecx,
                                                at(app, cpu.esp + 4), at(app, cpu.esp + 8));
    if (!placed)
        return;
    cpu.eax = *placed;
    cpu.esp += 4 + 8;
}

/* Slot's progress: its frame (the clock since it started, by its path's frames per point) within the
 * path's spans to `frameOut`, the spans (its points less one) to `spansOut`; the spans. */
reg32 animationSpans(App* app, reg32 slot, reg32 frameOut, reg32 spansOut)
{
    const reg32 record = slot * kAnimSize;
    const i32 elapsed = i32(at(app, kClock) - at(app, record + kAnimStart));
    const reg32 path = at(app, record + kAnims);
    const i32 step = quotient(elapsed, pathFrames(app, path));
    put(app, frameOut, reg32(remainder(step, pathSpans(app, path))));
    const reg32 spans = reg32(pathSpans(app, path));
    put(app, spansOut, spans);
    return spans;
}

/* 0x40f800: animationSpans of slot eax to [edx] and [ebx]. */
void animationProgress(App* app, CPU& cpu)
{
    cpu.eax = animationSpans(app, cpu.eax, cpu.edx, cpu.ebx);
    cpu.esp += 4;
}

/* Path's last point: its place copied to `placeOut`, its orientation (in the code's frame, whose
 * address it is handed) made a matrix at `matrix` (0x4b6ca0).  False when the game stopped. */
bool lastPose(App* app, CPU& cpu, reg32 path, reg32 placeOut, reg32 matrix)
{
    const GuestFrame frame(cpu, 0xc, 8);
    const reg32 point = path + kPathPointList + (reg32(app->getMemory<reg16>(path + kPathPoints)) - 1) * kPointSize;
    put(app, frame - 8, at(app, point + kPointOrientation));
    put(app, frame - 4, at(app, point + kPointOrientation + 4));
    for (reg32 i = 0; i < 0xc; i += 4)
        put(app, placeOut + i, at(app, point + i));
    return callGame(app, cpu, kQuatMatrix, {frame - 8, matrix});
}

/* 0x40f880 (stdcall, 1 word: where the matrix goes): lastPose of path eax, its place to [ecx]; eax as
 * 0x4b6ca0 leaves it. */
void animationLastPose(App* app, CPU& cpu)
{
    if (lastPose(app, cpu, cpu.eax, cpu.ecx, at(app, cpu.esp + 4)))
        cpu.esp += 4 + 4;
}

/* Path `index`'s (of the handles) end pose (lastPose, at the code's depth, its place to `placeOut`,
 * its matrix to `pose`); mirrored when `flags` bit 0 is set: the matrix's first two rows swapped, the
 * second's negated -- as integers, the code's own way.  False when the game stopped. */
bool endPose(App* app, CPU& cpu, reg32 index, reg32 flags, reg32 placeOut, reg32 pose)
{
    {
        const GuestFrame frame(cpu, 8, 4);
        const ReturnSlot slot(cpu, 1);
        if (!lastPose(app, cpu, at(app, index * 4 + kAnimPaths), placeOut, pose))
            return false;
    }
    if ((flags & 1) == 0)
        return true;
    for (reg32 column = 0; column < 0xc; column += 4)
    {
        const reg32 second = at(app, pose + 0x18 + column), first = at(app, pose + 0xc + column);
        put(app, pose + 0xc + column, second);
        put(app, pose + 0x18 + column, first);
        put(app, pose + 0x18 + column, 0u - first);
    }
    return true;
}

/* 0x40f8d0 (stdcall, 1 word: the pose's matrix): endPose of path eax by flags ebx, its place to
 * [ecx]. */
void pathEndPose(App* app, CPU& cpu)
{
    if (!endPose(app, cpu, cpu.eax, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4)))
        return;
    cpu.eax = 1;
    cpu.esp += 4 + 4;
}

/* Slot stepped: -1 when it is not playing; its path's pose at the clock since it started (poseAt at
 * the code's depth, its mode, `blendWord` handed on, the pose to `pose`) -- when that is past the end
 * the slot stops (0); mirrored by its flags' bit 0 (the pose's first two rows swapped, the second's
 * negated, as floats).  1; nullopt when the game stopped. */
std::optional<reg32> stepAnimation(App* app, CPU& cpu, reg32 slot, reg32 blendWord, reg32 pose)
{
    if (!animationOn(app, slot))
        return 0xffffffff;
    const reg32 record = slot * kAnimSize;
    std::optional<reg32> posed;
    {
        const GuestFrame frame(cpu, 0x10, 8);
        const ReturnSlot returnSlot(cpu, 1);
        const i32 time = i32(at(app, kClock) - at(app, record + kAnimStart));
        posed = poseAt(app, cpu, at(app, record + kAnims), at(app, record + kAnimMode), time, blendWord, pose);
    }
    if (!posed)
        return std::nullopt;
    if (*posed == 0)
    {
        put(app, record + kAnims, 0);
        put(app, record + kAnimStart, 0xffffffff);
        return 0;
    }
    if ((byte(app, record + kAnimFlags) & 1) != 0)
    {
        const Float a = f32(app, pose + 0x18), b = f32(app, pose + 0x1c);
        const reg32 c = at(app, pose + 0xc);
        const Float d = f32(app, pose + 0x20);
        putFloat(app, pose + 0xc, float(a));
        putFloat(app, pose + 0x18, float(-Float(floatOf(c))));
        const reg32 e = at(app, pose + 0x10);
        putFloat(app, pose + 0x10, float(b));
        putFloat(app, pose + 0x1c, float(-Float(floatOf(e))));
        const reg32 g = at(app, pose + 0x14);
        putFloat(app, pose + 0x14, float(d));
        putFloat(app, pose + 0x20, float(-Float(floatOf(g))));
    }
    return 1;
}

/* 0x40f940: stepAnimation of slot eax, edx handed on, the pose to [ebx]. */
void animationStep(App* app, CPU& cpu)
{
    const std::optional<reg32> stepped = stepAnimation(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!stepped)
        return;
    cpu.eax = *stepped;
    cpu.esp += 4;
}

/* Whether slot is playing (its path +0 set). */
bool animationOn(App* app, reg32 slot)
{
    return at(app, slot * kAnimSize + kAnims) != 0;
}

/* 0x40fac0: animationOn for slot eax. */
void animationPlaying(App* app, CPU& cpu)
{
    cpu.eax = animationOn(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

/* The caps the library is started with for the speaker setup: 0x808 for setup 0 and speakers 0, 0x804
 * for 0/1, 0x802 for 1/0, 0x801 for 1/1; none for any other. */
std::optional<reg32> setupCaps(reg32 setup, reg32 speakers)
{
    if (speakers == 0 && setup == 0)
        return 0x808;
    if (speakers == 0 && setup == 1)
        return 0x804;
    if (speakers == 1 && setup == 0)
        return 0x802;
    if (speakers == 1 && setup == 1)
        return 0x801;
    return std::nullopt;
}

/* The sound library started (SNDinit, 0x4e2630, edx -1) with the setup's caps when `caps` has them
 * (0x20000 more in a 3D setup [0x5525a4]), else with -1; its answer.  For a setup none of the four the
 * library is left be and the code tests what its caller had in edx: `callerEdx`.  Nullopt when the game
 * stopped. */
std::optional<reg32> startLibrary(App* app, CPU& cpu, reg32 caps, reg32 setup, reg32 speakers, reg32 callerEdx)
{
    const std::optional<reg32> wanted = setupCaps(setup, speakers);
    if (!wanted)
        return callerEdx;
    reg32 asked = 0xffffffff;
    if ((caps & *wanted) == *wanted)
        asked = at(app, kSoundInitOK) != 0 ? 0x20000 | *wanted : *wanted;
    if (!callGame(app, cpu, kSndInit, {asked, 0xffffffff}))
        return std::nullopt;
    return cpu.eax;
}

/* The 3D setup's trace line (0x42d510, cdecl: 100, 1, 0x53684c): its words pushed as the code
 * pushes them, the trace (ea::gameTrace, compiled out) called as C++. */
bool traceSetup(App* app, CPU& cpu)
{
    const reg32 esp = cpu.esp;
    put(app, esp - 12, 0x64);
    put(app, esp - 8, 1);
    put(app, esp - 4, kVolume);
    cpu.esp = esp - 12;
    guest::direct<0x42d510, ea::traceOff>(app, cpu);
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    return true;
}

/* The sound brought up at the race's start: the 23 voice records' (12 bytes at 0x571124) first words
 * cleared; the setup (2 taken as 0, 3 the 3D setup [0x5525a4]) and the speakers; when effects or music
 * are on, the library started (startLibrary, from the caps 0x4e21b0 gives); failing that both are
 * turned off, else the rate set (SNDsetrate 0x4e2228: 0, or -50 unless the rate's kind is set) and the
 * effect bus set up (0x418740).  `callerEdx` as startLibrary has it.  False when the game stopped. */
bool bringUpSound(App* app, CPU& cpu, reg32 callerEdx)
{
    const GuestFrame frame(cpu, 0x18, 0);  // the callees at the code's depth: deeper down they log the stack
    for (reg32 i = 0; i < 0x17; ++i)
        put(app, i * 0xc + kVoiceRecords, 0);
    reg32 setup = gameSettings(app).soundSetup;
    const reg32 speakers = gameSettings(app).soundSurround;
    if (setup == 2)
        setup = 0;
    const reg32 rateKind = gameSettings(app).soundRateKind;
    if (setup == 3)
    {
        put(app, kSoundInitOK, 1);
        setup = 0;
    }
    else
        put(app, kSoundInitOK, 0);
    if (at(app, kEffectsOn) == 0 && at(app, kMusicOn) == 0)
        return true;
    if (!callGame(app, cpu, kSndCaps, {0xffffffff}))
        return false;
    const reg32 caps = cpu.eax;
    if (at(app, kSoundInitOK) != 0 && !traceSetup(app, cpu))
        return false;
    const std::optional<reg32> started = startLibrary(app, cpu, caps, setup, speakers, callerEdx);
    if (!started)
        return false;
    if (i32(*started) < 0)
    {
        put(app, kMusicOn, 0);
        put(app, kMusicSerial, 0);
        put(app, kEffectsOn, 0);
        return true;
    }
    return callGame(app, cpu, kSndRate, {rateKind != 0 ? 0u : 0xffffffceu}) && callGame(app, cpu, kBusInit, {});
}

/* 0x40fb10: bringUpSound.  What the code leaves in eax no caller reads; aiplan.inc's promise says
 * so. */
void startSound(App* app, CPU& cpu)
{
    if (bringUpSound(app, cpu, cpu.edx))
        cpu.esp += 4;
}

/* The sound restarted for `setup` from the options screen (2 taken as 0, 3 the 3D setup): when effects
 * or music are on the library started again (startLibrary); failing that both are turned off, else the
 * effect bus set up (0x418740) and the caps asked again.  `callerEdx` as startLibrary has it.  False
 * when the game stopped. */
bool restartLibrary(App* app, CPU& cpu, reg32 setup, reg32 callerEdx)
{
    const GuestFrame frame(cpu, 0x18, 0);  // the callees at the code's depth: deeper down they log the stack
    if (setup == 2)
        setup = 0;
    const reg32 speakers = gameSettings(app).soundSurround;
    if (setup == 3)
    {
        put(app, kSoundInitOK, 1);
        setup = 0;
    }
    else
        put(app, kSoundInitOK, 0);
    if (at(app, kEffectsOn) == 0 && at(app, kMusicOn) == 0)
        return true;
    if (!callGame(app, cpu, kSndCaps, {0xffffffff}))
        return false;
    const reg32 caps = cpu.eax;
    if (at(app, kSoundInitOK) != 0 && !traceSetup(app, cpu))
        return false;
    const std::optional<reg32> started = startLibrary(app, cpu, caps, setup, speakers, callerEdx);
    if (!started)
        return false;
    if (i32(*started) < 0)
    {
        put(app, kMusicOn, 0);
        put(app, kMusicSerial, 0);
        put(app, kEffectsOn, 0);
        return true;
    }
    return callGame(app, cpu, kBusInit, {}) && callGame(app, cpu, kSndCaps, {0xffffffff});
}

/* 0x40fcf0: restartLibrary for setup eax.  What the code leaves in eax no caller reads; aiplan.inc's
 * promise says so. */
void restartSound(App* app, CPU& cpu)
{
    if (restartLibrary(app, cpu, cpu.eax, cpu.edx))
        cpu.esp += 4;
}

}  // namespace aiplan
}  // namespace nfs3hp
