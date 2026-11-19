#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_loopa.h"
#include "native_frontend_sima.h"
#include "native_frontend_simb.h"
#include "native_frontend_simc.h"
#include "native_guest.h"
#include "native_handling.h"
#include "native_load_track.h"
#include "native_pursuit.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "port_snd_calls.h"
#include "port_sound_calls.h"
#include "port_sound_effects.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <vector>
#include <utility>

/* The traffic and the opponents' lane driving, the track's animation paths
 * and the sound's start-up, native whole (0x40c4f0..0x40fcf0).
 * The game's code calls each by the name it has here.
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
 * its result returned, C++ locals) with a thunk under the game's name for
 * the dispatch and the callers still on registers: it takes the registers and
 * the words pushed, calls the core and leaves eax, the x87's stack and esp as
 * the game's code does (eax only where a caller can read it: aiplan.inc's
 * promises say which leave nothing there).  The x87's operations are fp's in
 * the code's order, a float kept where the code stores one.  The cores call
 * each other as C++, and other typed files' functions as C++ too
 * (guest::direct, their headers) but where the eax a callee leaves, which its
 * core does not give back, is what a thunk here leaves; every other
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
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s16;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using Float = nfs::Float;
using i32 = std::int32_t;

// The cars: the list and its count, the per-type list, the players' cars.
constexpr u32 kCars = 0x5efac8, kCarCount = 0x5efd9c, kCarCountB = 0x5efd8c;
constexpr u32 kCarsByType = 0x5efb48, kPlayerCars = 0x5efa48, kCarsByPlace = 0x5efa88, kPlaceCount = 0x5efd88;
constexpr u32 kAiCount = 0x5efda4, kPlayerCarsB = 0x56ebb8;
// The track's nodes and their count.
constexpr u32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The race: reversed, split screen, the track, the type, the variant.
constexpr u32 kClock = 0x7d3684;
// The lane planner's globals (0x56ee90..): 14 rows of votes for the three lanes about the car (left,
// its own, right; sumLaneVotes adds them up) -- row 0 the present lane's, 1 the close cars', 2 the
// road's edge, 3 the road's shape and the roadside, 4 the way ahead, 5 the obstacles, 6 the oncoming
// cars, 7 the cars ahead; the lane taken; the cars sensed ahead in the three lanes and behind (their
// gaps after each, then whether each is in that lane by its middle, as words); the way ahead's
// side (1 left, -1 right); a car behind driving against the track.
constexpr u32 kLaneCostA = 0x56ee90, kLaneCostB = 0x56ee94, kLaneCostC = 0x56ee98;
constexpr u32 kLaneCostLo = 0x56ee9c, kLaneCostHi = 0x56eea4, kLaneCostObj = 0x56eea8, kLaneCostObjB = 0x56eeb0;
constexpr u32 kNodeCostB = 0x56eeb4, kNodeCostC = 0x56eeb8, kNodeCostD = 0x56eebc, kNodeCostE = 0x56eec0;
constexpr u32 kObjCost = 0x56eecc, kCarCostLeft = 0x56eed8, kCarCostRight = 0x56eee4;
constexpr u32 kChosenLane = 0x56ef74;
constexpr u32 kSensed = 0x56ef38, kSensedRight = 0x56ef50;
constexpr u32 kSensedFlag = 0x56ef7c, kSensedEdge = 0x56ef68, kSensedEdgeB = 0x56ef6e;
constexpr u32 kSensedDir = 0x56ef78;
// The lane-width table (80 floats), the obstacles (8 of 0x50 bytes).
constexpr u32 kLaneWidths = 0x56f0e0, kObstacles = 0x7dd850;
// The track's speed table loaded, its half, the failure flag.
constexpr u32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kSpeedsReady = 0x56f228;
constexpr u32 kTrafficSide = 0x56f230, kTrackWalked = 0x56f220;
// The catch-up tables (0x56ef80.. per track-number slot of 0x20), the start.
constexpr u32 kTuning = 0x56ef80, kStartFlags = 0x55eb2e;
// The animation players (0x18 bytes each) and the seven paths loaded.
constexpr u32 kAnims = 0x56f234, kAnimPaths = 0x570a34;
// The sound config: effects on, music on, the options, the result flag.
constexpr u32 kEffectsOn = 0x55290c, kMusicOn = 0x552910, kMusicSerial = 0x552918;
constexpr u32 kSoundInitOK = 0x5525a4;
constexpr u32 kVoiceRecords = 0x571124;
// Constants of the game's.
constexpr u32 kVolume = 0x53684c, kTicks = 0x79c1a4;

// What is called by address: the AI-votes trace hook (0x40de30, its
// thunk's NFS_CAR_TRACE record), the police chase (0x411750, no core), the
// traffic's top speed (0x40bfe0, its eax left by slowForPolice), and
// the game's/other parts' natives (stood in for where they would run on a
// made-up world): a traffic car linked (0x404690, whose eax resetTrafficCar
// leaves) and vectors scaled (0x4e0050) where the eax they leave is a
// thunk's.  The other chunks' AI functions (0x40c160, 0x40c2f0, 0x40bff0,
// 0x40c270, 0x401240, 0x405cd0..0x405e20),
// the drive reset (0x4aa79f), the opponents' speeds (0x40e000), the free
// (0x4e1890), the trace (0x42d510), memset (0x4e0640) and vectors scaled
// elsewhere are called as C++ (guest::direct); the menu files' (a car put on
// the road 0x4a0aa0, the poses 0x4b70f0, 0x4b7590, 0x4b6a30, 0x4b6ca0) too
// (feCore).
constexpr u32 kChop = 0x4dfd56;
constexpr u32 kSndCaps = 0x4e21b0;  // native_game.cpp's SNDcaps: no typed core

// The game's floats and doubles where it keeps them, as the x87 loads them.
Float f32(App* app, u32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, u32 a) { return Float(app->getMemory<double>(a)); }

u32 nodeAddr(App* app, u32 index)
{
    // lea eax,[index*8]; add eax,index; shl eax,2; add [0x5dd948]: index*0x24.
    return at(app, kNodes) + index * 0x24;
}

void putFloat(App* app, u32 a, float f) { app->getMemory<float>(a) = f; }

/* A float of the bits given, as the code's mov [frame], bits; fld [frame] has it. */
float floatOf(u32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

u32 bitsOf(float f)
{
    u32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

/* A signed byte, as movsx has it. */
i32 signedByte(App* app, u32 a) { return i32(std::int8_t(byte(app, a))); }

/* A thunk's run of its core: every register but eax as the caller had it after it (the game's
 * code pops what it pushed; its other registers are its arguments, which no caller reads), eax what
 * the core gives -- a core that gives nothing, or answers only whether the game went on (a bool),
 * leaves eax as it is -- and the return address and `popped` bytes of the caller's words popped.
 * False when the game stopped. */
template <typename T>
struct IsOptional : std::false_type
{
};

template <typename T>
struct IsOptional<std::optional<T>> : std::true_type
{
};

template <typename Core>
bool asThunk(ThreadState& ts, nfs::Args& args, Core&& core, u32 popped = 0)
{
    using Result = std::decay_t<decltype(core())>;
    const u32 esp = ts.sp;
    std::optional<u32> left;
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
            left = u32(*result);
        }
        else
            left = u32(core());
    }
    ts.sp = esp;
    if (left)
        args.eax = *left;
    ts.sp += 4 + popped;
    return true;
}

/* The caller's word `n` (from 1) on the guest's stack, above the return address: a stdcall
 * thunk's. */
u32 word(App* app, const ThreadState& ts, u32 n) { return at(app, ts.sp + 4 * n); }

/* 0x4e1890 (ea::purgememadr): the handle freed; the eax the free leaves, what it gives back; nullopt
 * when the game stopped. */
std::optional<u32> freeHandle(App* app, ThreadState& ts, u32 handle)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, ts, handle);
}

// The game's constants the code reads where it has them.
constexpr u32 kSixth = 0x5367b4;          // a double: 1/6
constexpr u32 kFixedToFloat = 0x5367bc;   // a double: 1/65536
constexpr u32 kQuarterLanes = 0x5367c4;   // a float: 4, the lane-width table's steps per unit
constexpr u32 kHalfSlice = 0x5367cc;      // a double: 0.5
constexpr u32 kLessHalfSlice = 0x5367d4;  // a double: -0.5
constexpr u32 kDirectionScale = 0x5367f4; // a double: 1/65536

// A node's record (0x24 bytes): its lane bits' dword (the high half the bits), its edge slices (high
// nibble left, low right), its forward direction's x and z (signed bytes), its halves' widths.
constexpr u32 kNodeLaneBits = 0xa, kNodeEdges = 0x13, kNodeDirectionX = 0x14, kNodeDirectionZ = 0x16,
                kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b, kNodeSize = 0x24;
// A track's tuning record (0x20 bytes at 0x56ef80 by the track): as fillTrackTuning fills it.
constexpr u32 kTuningFirst = 0, kTuningDistance = 4, kTuningRoadWide = 8, kTuningSide = 0xc,
                kTuningSpeed = 0x14, kTuningSeriesSpeed = 0x18, kTuningLast = 0x1c, kTuningSize = 0x20;
constexpr i32 kTrackCount = 0xb;  // the tracks with a record, as the code bounds them
// The race's laps.
constexpr u32 kLaps = 0x5efd94;
// A car's fields this code reads: its node, its place along the track and its speed along it, its
// flags (+0x621 bit 4: it drives against the track's direction), its slices, its place across the road
// (+0x7b8), its half width as a slice offset (+0x21c), its look-ahead (+0x7c4), its way (+0x7d4 1
// backwards).
constexpr u32 kCarNode = 0x1c, kCarAlong = 8, kCarSpeedAlong = 0x1fc, kCarHalfWidth = 0x21c,
                kCarWayFlags = 0x621, kCarSlice = 0x638, kCarLeftSlice = 0x63c, kCarRightSlice = 0x640,
                kCarAcross = 0x7b8, kCarLookAhead = 0x7c4, kCarBackwards = 0x7d4;
constexpr u8 kAgainstTrack = 0x10;
// The road's slices across, 0..13.
constexpr i32 kSlices = 0xe;
// A traffic car's (and a racer's) fields more: its forward vector, its velocity, its spin, its grip
// on the road, whether it is off the ground (a word), its kind (+0x200 bit 2: a player's), the brake,
// its traffic flags (+0x620: bit 1 to be placed, bit 2 out of play, bit 3 the horn), its loading step,
// its horn's end and tone, the player car it keeps near, its state (4 pulled over), the police's
// siren (+0x710 bit 4) and chase (+0x734), its speed asked (+0x7b0), its target place across (+0x7b4),
// its speed (+0x7c0).
constexpr u32 kCarForward = 0xd8, kCarVelocity = 0xa4, kCarSpin = 0xe4, kCarGrip = 0x10c, kCarAirborne = 0x164,
                kCarKindFlags = 0x200, kCarBrake = 0x509, kCarTrafficFlags = 0x620, kCarLoading = 0x630,
                kCarHornEnd = 0x644, kCarHornTone = 0x660, kCarFollows = 0x6d8, kCarState = 0x6dc,
                kCarSirenFlags = 0x710, kCarChasing = 0x734, kCarTargetSpeed = 0x7b0, kCarTargetAcross = 0x7b4,
                kCarSpeed = 0x7c0;
constexpr u8 kKindPlayer = 4, kTrafficInactive = 4, kTrafficHorn = 8, kSirenOn = 0x10;
constexpr u32 kStateDriving = 1, kStatePullingOver = 3, kStatePulledOver = 4;
// More of a car's: its index among the cars, whether it is in play (a byte), its half length.
constexpr u32 kCarIndex = 0x1f0, kCarInPlay = 0x89, kCarHalfLength = 0x114;
// A node's road edges (16.16 from its middle).
constexpr u32 kNodeLeftEdge = 0x1c, kNodeRightEdge = 0x20;
// The sensed cars' gaps ahead (three dwords after the three cars at 0x56ef38) and behind (after the
// three behind at 0x56ef50).
constexpr u32 kSensedGaps = 0x56ef44, kSensedBehindGaps = 0x56ef5c;
// The parked obstacles (8 of 0x50 bytes at 0x7dd850): +0 in use, +0x34 the node, +0x38 the slice.
constexpr u32 kObstacleSize = 0x50, kObstacleNode = 0x34, kObstacleSlice = 0x38;
// The lane planner's constants, as the code reads them (doubles but the one float).
constexpr u32 kSenseScale = 0x53670c, kSenseSixth = 0x536714;  // 2, 1/6: the sensing window
constexpr u32 kToFixedC = 0x53671c;                            // 65536: the half length as 16.16
constexpr u32 kEaseObstacle = 0x536724;                        // 0.9: the speed by an obstacle
constexpr u32 kMeetScale = 0x53672c, kToFixedD = 0x536734;     // 6, 65536: the time to meet
constexpr u32 kFromFixed = 0x53673c;                           // 1/65536
constexpr u32 kMeetWeight = 0x536744;                          // a float: -35
constexpr u32 kEighth = 0x53674c, kMeetLessHalf = 0x536754;    // 1/8, -0.5
constexpr u32 kOutrunMost = 0x53675c, kOutrunWeight = 0x536764, kThird = 0x53676c;  // 3, -5, 1/3
constexpr u32 kLaneFixed = 0x536774, kHalfWidthBack = 0x53677c;  // 1/65536, 0.5: the place across
constexpr u32 kLaneReach = 0x536784, kLaneSixth = 0x53678c, kLaneLessHalf = 0x536794;  // 2.5, 1/6, -0.5
// The tuning's: the handicap's step (a float, -0.005), a quarter (a float), 0.98 and 0.99 (doubles); the
// opponents' speeds loaded (load::loadOpponentSpeeds); the highest target speed on the track.
constexpr u32 kHandicapStep = 0x5367dc, kQuarter = 0x5367e0, kHandicapSlower = 0x5367e4, kHandicapSlow = 0x5367ec;
constexpr u32 kTopTargetSpeed = 0x56f224;
// The animation players: 256 slots of 0x18 bytes.  A path: +4 its points (a word), +6 its frames per
// point (a word), from +8 its points of 0x14 bytes -- a place of three 16.16 dwords, an orientation of
// four shorts at +0xc.  1/65536 as a double, for the poses.
constexpr u32 kAnimSlots = 0x100, kAnimSize = 0x18;
constexpr u32 kPathPoints = 4, kPathFrames = 6, kPathPointList = 8, kPointSize = 0x14, kPointOrientation = 0xc;
constexpr u32 kPoseScale = 0x536814;
// The game's random numbers: the seed, the product, the multiplier.
constexpr u32 kRandomSeed = 0x554f10, kRandomProduct = 0x554f14, kRandomMultiplier = 0x554f18;
// A word the sensed cars' clearing zeroes with them.
constexpr u32 kSensedRound = 0x56ebc0;
// The data among the code: the place's last part for placeOnTrack, the grid's place.
constexpr u32 kPlaceOnTrackHeight = 0x40cb08, kGridPlace = 0x40cb0c;
// The game's constants: 20 and 23 (doubles: how fast the cars close for the horn, for a chase),
// 65536 twice (doubles), 0.9 (a double: a firm grip), 6.7 and 0.5 as float bits.
constexpr u32 kHornClosing = 0x5366e4, kChaseClosing = 0x5366ec, kToFixedA = 0x5366f4, kToFixedB = 0x5366fc,
                kFirmGrip = 0x536704;
constexpr u32 kPullOverSpeed = 0x40d6930c, kHalf = 0x3f000000;

/* fcom of a and b, as fnstsw leaves the status word: what a code that ends on a compare leaves in ax. */
u16 statusWord(const Float& a, const Float& b)
{
    return fp::compare(a, b);
}

/* 0x4dfd56 (__CHP): value to an integer towards 0, as a whole value; never nullopt (the optional
 * is the callers' stop signal). */
std::optional<Float> chop(App* app, ThreadState& ts, const Float& value)
{
    return Float(guest::direct<kChop, watcom::clib::chop>(app, ts, double(value)));
}

/* A call by address (the callee has no typed core) as the code makes it: the registers handed over
 * (eax, edx, ebx, ecx in that order) and the words pushed (the first lowest); every register but eax
 * kept.  The eax the callee leaves; nullopt when the game stopped. */
std::optional<u32> callGame(App* app, ThreadState& ts, u32 target, std::initializer_list<u32> registers,
                              std::initializer_list<u32> words = {})
{
    return guest::pointerCall(app, ts, target, registers, words);
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.
 * A u32 result is the eax the function leaves, set to it; after any other, eax is left alone. */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    const auto run = [&] {
        return guest::direct<Address, Core>(app, ts, args...);
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
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = run();
            if constexpr (std::is_same_v<Result, nfs::u32>)
                (void)result;  // a menu core's result: the eax it leaves is no longer set (no caller reads it)
            else
                spoil();
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* 0x404690 (airace::setRammingRules, below this file) on car (eax), by address: the eax it leaves
 * is resetTrafficCar's, and its core gives nothing back.  Nullopt when the game stopped. */
std::optional<u32> linkTraffic(App* app, ThreadState& ts, u32 car)
{
    const u32 left = guest::direct<0x404690, airace::setRammingRulesLeaving>(app, ts, car);
    if (ts.terminate)
        return std::nullopt;
    return left;
}

/* What 0x40bfe0 leaves: the traffic's top speed, off the x87's stack, and its eax. */
struct TopSpeed
{
    Float speed;
    u32 eax;
};

/* 0x40bfe0: the traffic's top speed; nullopt when the game stopped.  Its eax: the car it was handed
 * (slowForPolice leaves it). */
std::optional<TopSpeed> trafficTopSpeed(App* app, ThreadState& ts, u32 car)
{
    const nfs::Float speed = guest::direct<0x40bfe0, pursuit::trafficTopSpeed>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return TopSpeed{speed, car};
}

/* 0x40de30: the lane votes of car added up and acted on (`kind` 0 driving, 2 pulling over). */
bool actOnVotes(App* app, ThreadState& ts, u32 car, u32 kind)
{
    (void)kind;  // (0x40de30 reads only eax)
    return feCore<0x40de30, frontend::sima::laneSteer>(app, ts, car).has_value();
}

/* 0x411750: a police chase called for car: 1, 100, 8. */
bool callChase(App* app, ThreadState& ts, u32 car)
{
    guest::direct<0x411750, port::copChaseSetOf>(app, ts, car, 1, 0x64, 8);
    return !ts.terminate;
}

/* 0x4a0aa0: car put on the road at node, at the place (three dwords) at `place`, its direction 1
 * against the track. */
bool placeOnRoad(App* app, ThreadState& ts, u32 car, u32 node, u32 place, u32 direction)
{
    return feCore<0x4a0aa0, frontend::simb::placeOnRoad>(app, ts, car, node, place, direction).has_value();
}

bool scaleVectorsDirect(App* app, ThreadState& ts, u32 count, u32 from, u32 scale, u32 to);

/* 0x4e0050 (stdcall): `count` vectors from `from` scaled by the float of `scale`'s bits into `to`,
 * by address, for a caller that leaves the eax it leaves (brakeToStop): that eax. */
std::optional<u32> scaleVectors(App* app, ThreadState& ts, u32 count, u32 from, u32 scale, u32 to)
{
    if (!scaleVectorsDirect(app, ts, count, from, scale, to))
        return std::nullopt;
    return i32(count) > 0 ? to + count * 12 : to;  // the vectors past the last, as the callee leaves it
}

/* 0x4e0050 (vertices'): the same as C++ (guest::direct).  False when the game stopped. */
bool scaleVectorsDirect(App* app, ThreadState& ts, u32 count, u32 from, u32 scale, u32 to)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, i32(count), from, floatOf(scale), to);
    return !ts.terminate;
}

/* Room on the guest's stack for what a callee is handed the address of: the code's frame (`pushed`
 * bytes of registers under esp on entry, then `bytes`), esp under it while the core runs and back
 * after; an address in it as the code's ebp less the offset. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* The return address's slot (and `words` pushed before it) under esp, as a call to a sibling makes
 * them: a core that keeps a frame on the guest's stack is called at the code's depth. */
class ReturnSlot
{
public:
    explicit ReturnSlot(ThreadState& ts, u32 words = 0) : ts_(ts), esp_(ts.sp) { ts.sp -= 4 + 4 * words; }
    ~ReturnSlot() { ts_.sp = esp_; }
    ReturnSlot(const ReturnSlot&) = delete;
    ReturnSlot& operator=(const ReturnSlot&) = delete;

private:
    ThreadState& ts_;
    const u32 esp_;
};

}  // namespace

// Forward declarations, in address order (address -> name).
nfs::u32 leastTrafficPlayer(App* app, nfs::ThreadState& ts, nfs::Args& args);  // 0x40c4f0
nfs::u32 placeTrafficCar(App* app, nfs::ThreadState& ts, nfs::Args& args);           // 0x40c590
nfs::u32 reactToTraffic(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40c630
nfs::u32 slowForPolice(App* app, nfs::ThreadState& ts, nfs::Args& args);             // 0x40c780
nfs::u32 countStopped(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40c7f0
nfs::u32 nearPoliceCar(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40c830
nfs::u32 weighLaneVotes(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40c8a0
nfs::u32 voteRoadside(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40c8d0
nfs::u32 atRoadside(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40c950
nfs::u32 advanceCarAi(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40c9f0
nfs::u32 clearSensedCars(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40cb20
nfs::u32 cacheTrafficSide(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40cb50
nfs::u32 resetTrafficCar(App* app, nfs::ThreadState& ts, nfs::Args& args);              // 0x40cb70
nfs::u32 clearTrafficRecord(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40cb90
nfs::u32 placeOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40cd80
nfs::u32 brakeToStop(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40ce70
nfs::u32 voteCurrentLane(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40cee0
nfs::u32 senseTraffic(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40cfc0
nfs::u32 planLaneChange(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40d1f0
nfs::u32 edgeLaneCost(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40d2e0
nfs::u32 scoreLanesByRoad(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40d3d0
nfs::u32 scoreLanesByObstacles(App* app, nfs::ThreadState& ts, nfs::Args& args); // 0x40d720
nfs::u32 scoreLanesByCars(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40d800
nfs::u32 edgePenalty(App* app, nfs::ThreadState& ts, nfs::Args& args);           // 0x40d9f0
nfs::u32 sumLaneVotes(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40da30
nfs::u32 steerToLane(App* app, nfs::ThreadState& ts, nfs::Args& args);           // 0x40daf0
nfs::u32 beginLanePlanning(App* app, nfs::ThreadState& ts, nfs::Args& args);     // 0x40dd60
nfs::u32 chooseLane(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40ddb0
nfs::u32 startHorn(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40de50
nfs::u32 stopHorn(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40deb0
nfs::u32 stepTrafficLoading(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40df10
nfs::u32 targetSpeedAt(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40e080
nfs::u32 roadKindAt(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40e0c0
nfs::u32 cacheRoadWidth(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40e0f0
nfs::u32 nodeGap(App* app, nfs::ThreadState& ts, nfs::Args& args);               // 0x40e120
nfs::u32 nodeOffset(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40e160
nfs::u32 nodesAheadBySpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);     // 0x40e1b0
nfs::u32 nodeLaneBits(App* app, nfs::ThreadState& ts, nfs::Args& args);               // 0x40e200
nfs::u32 nodeHasLane(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40e220
nfs::u32 laneBit(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40e270
nfs::u32 roadSliceIndex(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40e2b0
nfs::u32 carSlices(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40e350
nfs::u32 fillTrackTuning(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40e3c0
nfs::u32 setupTrackTuning(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40e410
nfs::u32 freeSpeedTable(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40ec40
nfs::u32 trafficSide(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40ec70
nfs::u32 roadIsWide(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40eca0
nfs::u32 trafficSpeedAt(App* app, nfs::ThreadState& ts, nfs::Args& args);    // 0x40ed40
nfs::u32 tuningSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40ee80
nfs::u32 tuningDistance(App* app, nfs::ThreadState& ts, nfs::Args& args);           // 0x40eed0
nfs::u32 tuningFirst(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40ef10
nfs::u32 nodeDirectionDot(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40ef70
nfs::u32 nodeWithinRange(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40f000
nfs::u32 carWithinReach(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40f050
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args);             // 0x40f100
nfs::u32 carBoxedIn(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40f110
nfs::u32 freeAnimationPaths(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40f330
nfs::u32 findFreeAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args);          // 0x40f370
nfs::u32 startAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args);               // 0x40f3b0
nfs::u32 setAnimationMode(App* app, nfs::ThreadState& ts, nfs::Args& args);         // 0x40f430
nfs::u32 stopAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40f460
nfs::u32 animationPose(App* app, nfs::ThreadState& ts, nfs::Args& args);       // 0x40f4b0
nfs::u32 animationPlace(App* app, nfs::ThreadState& ts, nfs::Args& args);        // 0x40f6d0
nfs::u32 animationProgress(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40f800
nfs::u32 animationLastPose(App* app, nfs::ThreadState& ts, nfs::Args& args);     // 0x40f880
nfs::u32 pathEndPose(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40f8d0
nfs::u32 animationStep(App* app, nfs::ThreadState& ts, nfs::Args& args);              // 0x40f940
nfs::u32 animationPlaying(App* app, nfs::ThreadState& ts, nfs::Args& args);            // 0x40fac0
nfs::u32 startSound(App* app, nfs::ThreadState& ts, nfs::Args& args);      // 0x40fb10
nfs::u32 restartSound(App* app, nfs::ThreadState& ts, nfs::Args& args);    // 0x40fcf0

// The typed cores the thunks above call, where one is used before it is defined.
u32 trackTrafficSide(App* app);
u32 trackRoadWide(App* app);
u32 sliceBit(u32 slice);
void soundHorn(App* app, u32 car);
void endHorn(App* app, u32 car);
void clearRecord(App* app, u32 car);
i32 nodesBetween(App* app, u32 node, u32 from);
void findCarSlices(App* app, u32 car);
u32 nodeAhead(App* app, u32 car, u32 steps);
u32 laneBitsAt(App* app, u32 node, u32 mask);
void scoreRoad(App* app, u32 car);
u32 roadScoreLeft(App* app, u32 car);
void scoreObstacles(App* app, u32 car);
bool scoreCars(App* app, ThreadState& ts, u32 car);
bool beginPlanning(App* app, ThreadState& ts, u32 car);
void weighVotes(App* app);
bool reactToCars(App* app, ThreadState& ts, u32 car);
bool pickLane(App* app, ThreadState& ts, u32 car);
bool policeNear(App* app, u32 car, u32 distance);
void voteForRoadside(App* app, u32 car);
bool reachedRoadside(App* app, u32 car);
void penaliseCloseCars(App* app);
std::optional<u32> stepLoading(App* app, ThreadState& ts, u32 car);
std::optional<bool> stopPulledOver(App* app, ThreadState& ts, u32 car);
bool animationOn(App* app, u32 slot);

/* The player car fewest traffic cars keep near: for each of the [0x5efda4] players' cars (0x5efa48)
 * the active traffic cars (0x5efb48: +0x620 bit 2 clear) whose +0x6d8 is it are counted; the first
 * with the least (from 10000), or the first car [0x5efac8] when there are no players.  The players'
 * count written back, as the code does.  The code counts in a frame of 15; the game has 4 players at
 * most, and a count past 15 (which would run over the code's own frame) counts the first 15. */
u32 leastFollowedPlayer(App* app)
{
    constexpr i32 kFrameCounts = 15;
    const i32 players = i32(at(app, kAiCount));
    const i32 counted = players < kFrameCounts ? players : kFrameCounts;
    i32 counts[kFrameCounts] = {};
    for (i32 player = 0; player < counted; ++player)
        for (i32 i = 0; i < i32(at(app, kCarCountB)); ++i)
        {
            const u32 car = at(app, u32(i) * 4 + kCarsByType);
            if ((byte(app, car + kCarTrafficFlags) & kTrafficInactive) != 0)
                continue;
            if (at(app, car + kCarFollows) == at(app, u32(player) * 4 + kPlayerCars))
                ++counts[player];
        }
    u32 least = at(app, kCars);
    i32 fewest = 10000;
    for (i32 player = 0; player < counted; ++player)
        if (fewest > counts[player])
        {
            fewest = counts[player];
            least = at(app, u32(player) * 4 + kPlayerCars);
        }
    put(app, kAiCount, u32(players));
    return least;
}

/* 0x40c4f0: leastFollowedPlayer; every register but eax kept. */
nfs::u32 leastTrafficPlayer(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return leastFollowedPlayer(app); });
    return args.eax;
}

/* Traffic car set going: the game's random numbers (the seed 0x554f10 by 0x554f18, 0..99 from the
 * product's middle bytes) make it drive against the track's direction (+0x621 bit 4) seven times in
 * ten, never where [0x56f228] (trackRoadWide) is set; the player car it keeps near (+0x6d8:
 * leastFollowedPlayer, or [0x56ebb8] with one player); then 0x40c160 and 0x40c2f0 (below this file)
 * place it.  The eax 0x40c2f0 leaves, what it gives back: placeTrafficCar's; nullopt when the game
 * stopped. */
std::optional<u32> placeTrafficEax(App* app, ThreadState& ts, u32 car)
{
    const u32 product = at(app, kRandomSeed) * at(app, kRandomMultiplier);
    put(app, kRandomProduct, product);
    const u32 middle = (at(app, kRandomProduct) & 0xffff00) >> 8;
    put(app, kRandomSeed, product & 0xffff);
    const u32 roll = (middle * 100) >> 16;
    if (roll <= 0x45 && at(app, kSpeedsReady) == 0)
        putByte(app, car + kCarWayFlags, u8(byte(app, car + kCarWayFlags) | kAgainstTrack));
    else
        putByte(app, car + kCarWayFlags, u8(byte(app, car + kCarWayFlags) & ~kAgainstTrack));
    const u32 follows = i32(at(app, kAiCount)) <= 1 ? at(app, kPlayerCarsB) : leastFollowedPlayer(app);
    put(app, car + kCarFollows, follows);
    if (!guest::direct<0x40c160, pursuit::followCarOf>(app, ts, car))
        return std::nullopt;
    return guest::direct<0x40c2f0, pursuit::trafficStepOf>(app, ts, car);
}

/* placeTrafficEax: the eax it leaves; nullopt when the game stopped. */
std::optional<u32> placeTraffic(App* app, ThreadState& ts, u32 car)
{
    return placeTrafficEax(app, ts, car);
}

/* 0x40c590: placeTraffic for car eax; eax as 0x40c2f0 leaves it. */
nfs::u32 placeTrafficCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return placeTrafficEax(app, ts, car); });
    return args.eax;
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
bool reactToCars(App* app, ThreadState& ts, u32 car)
{
    if ((byte(app, car + kCarTrafficFlags) & kTrafficHorn) != 0)
    {
        endHorn(app, car);
        return true;
    }
    if ((byte(app, car + kCarWayFlags) & kAgainstTrack) == 0)
        return true;
    const u32 ahead = at(app, kSensed + 4);
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
            if (Float(sizeOf(closing)) > f64(app, kChaseClosing) && !callChase(app, ts, car))
                return false;
        }
    }
    if (at(app, kSensedFlag) == 0)
        return true;
    for (u32 i = 0; i < 3; ++i)
    {
        const u32 sensed = at(app, i * 4 + kSensed);
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
nfs::u32 reactToTraffic(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return reactToCars(app, ts, car); });
    return args.eax;
}

/* What slowToPullOver did: the speed asked it compared, the top speed it had, and whether the speed
 * was cut to it. */
struct PullOver
{
    float asked;
    float topSpeed;
    bool cut;
    u32 eax;  // 0x40bfe0's
};

/* Traffic car slowed to pull over: its speed +0x7b0 6.7, or 0x40bfe0's when that is less; its sign
 * flipped for a reversed race and again for a car against the track's direction.  Nullopt when the
 * game stopped. */
std::optional<PullOver> slowToPullOver(App* app, ThreadState& ts, u32 car)
{
    put(app, car + kCarTargetSpeed, kPullOverSpeed);
    const std::optional<TopSpeed> top = trafficTopSpeed(app, ts, car);
    if (!top)
        return std::nullopt;
    const float asked = app->getMemory<float>(car + kCarTargetSpeed);
    const PullOver pullOver{asked, float(top->speed), Float(asked) > Float(float(top->speed)), top->eax};
    if (pullOver.cut)
        putFloat(app, car + kCarTargetSpeed, pullOver.topSpeed);
    if (gameSettings(app).backwards != 0)
        putByte(app, car + kCarTargetSpeed + 3, u8(byte(app, car + kCarTargetSpeed + 3) ^ 0x80));
    if ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0)
        putByte(app, car + kCarTargetSpeed + 3, u8(byte(app, car + kCarTargetSpeed + 3) ^ 0x80));
    return pullOver;
}

/* 0x40c780: slowToPullOver for car eax; eax as the code leaves it: 0x40bfe0's with the compare's
 * status word in ax, or the top speed's bits where the speed was cut to it; ah the speed's top byte
 * as the reversed race's flip left it. */
nfs::u32 slowForPolice(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax]() -> std::optional<u32> {
        const std::optional<PullOver> pullOver = slowToPullOver(app, ts, car);
        if (!pullOver)
            return std::nullopt;
        u32 left = (pullOver->eax & 0xffff0000u) | statusWord(Float(pullOver->asked), Float(pullOver->topSpeed));
        if (pullOver->cut)
            left = bitsOf(pullOver->topSpeed);
        if (gameSettings(app).backwards != 0)
            left = (left & 0xffff00ffu)
                   | u32(u8(byte(app, car + kCarTargetSpeed + 3)
                                ^ ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0x80 : 0)))
                         << 8;
        return left;
    });
    return args.eax;
}

/* How many active traffic cars have pulled over (+0x6dc 4). */
u32 stoppedCount(App* app)
{
    u32 stopped = 0;
    for (i32 i = 0; i < i32(at(app, kCarCountB)); ++i)
    {
        const u32 car = at(app, u32(i) * 4 + kCarsByType);
        if ((byte(app, car + kCarTrafficFlags) & kTrafficInactive) == 0 && at(app, car + kCarState) == kStatePulledOver)
            ++stopped;
    }
    return stopped;
}

/* 0x40c7f0: stoppedCount. */
nfs::u32 countStopped(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return stoppedCount(app); });
    return args.eax;
}

/* Whether a police car with its siren on (0x5efa88's, [0x5efd88] of them: +0x734 chasing or +0x710
 * bit 4) is within `distance` + 0x16 nodes of car -- two more for each car already pulled over
 * (stoppedCount). */
bool policeNear(App* app, u32 car, u32 distance)
{
    const u32 node = at(app, car + kCarNode);
    const u32 margin = stoppedCount(app) * 2;
    for (i32 i = 0; i < i32(at(app, kPlaceCount)); ++i)
    {
        const u32 police = at(app, u32(i) * 4 + kCarsByPlace);
        if (at(app, police + kCarChasing) == 0 && (byte(app, police + kCarSirenFlags) & kSirenOn) == 0)
            continue;
        i32 gap = nodesBetween(app, at(app, police + kCarNode), node);
        if (gap <= 0)
            gap = i32(0u - u32(gap));
        if (gap < i32(distance + 0x16 + margin))
            return true;
    }
    return false;
}

/* 0x40c830: policeNear for car eax within edx. */
nfs::u32 nearPoliceCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, distance = args.edx] { return policeNear(app, car, distance) ? 1 : 0; });
    return args.eax;
}

/* The three lane votes (0x56ee90..98) weighed ten times. */
void weighVotes(App* app)
{
    for (u32 i = 0; i < 3; ++i)
        put(app, i * 4 + kLaneCostA, at(app, i * 4 + kLaneCostA) * 10);
}

/* 0x40c8a0: weighVotes; eax the last vote, as the code leaves it. */
nfs::u32 weighLaneVotes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] {
        weighVotes(app);
        return at(app, kLaneCostC);
    });
    return args.eax;
}

/* A node's left and right edge slices: 6 less the high nibble of its +0x13, 7 more the low. */
i32 leftEdgeSlice(App* app, u32 record) { return 6 - ((byte(app, record + kNodeEdges) >> 4) & 0xf); }
i32 rightEdgeSlice(App* app, u32 record) { return (byte(app, record + kNodeEdges) & 0xf) + 7; }

/* A vote (+100) for pulling over: for the middle (0x56eeb8) when car's slice +0x638 is off its node's
 * road, else for the nearer roadside -- the left (0x56eeb4) or the right (0x56eebc). */
void voteForRoadside(App* app, u32 car)
{
    const u32 record = nodeAddr(app, at(app, car + kCarNode));
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
nfs::u32 voteRoadside(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        voteForRoadside(app, car);
        const u32 record = nodeAddr(app, at(app, car + kCarNode));
        return leftEdgeSlice(app, record) > i32(at(app, car + kCarSlice)) ? record : u32(rightEdgeSlice(app, record));
    });
    return args.eax;
}

/* Whether car has reached the roadside: its place across (+0x7b8, as 16.16) beyond its node's left
 * edge (0x10000 less the left edge's slices times the left half's width) or right edge. */
bool reachedRoadside(App* app, u32 car)
{
    const i32 across = fp::toInteger<i32>(fp::mul(f32(app, car + kCarAcross), f64(app, kToFixedA)));
    const u32 record = nodeAddr(app, at(app, car + kCarNode));
    const u32 edges = byte(app, record + kNodeEdges);
    const u32 left = 0x10000 - ((edges >> 4) & 0xf) * (u32(byte(app, record + kNodeLeftWidth)) << 0xf);
    if (across < i32(left))
        return true;
    const u32 right = (edges & 0xf) * (u32(byte(app, record + kNodeRightWidth)) << 0xf) - 0x10000;
    return across > i32(right);
}

/* 0x40c950: reachedRoadside for car eax. */
nfs::u32 atRoadside(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return reachedRoadside(app, car) ? 1 : 0; });
    return args.eax;
}

/* Traffic car's driver, each tick: its loading steps first (stepLoading, called at the code's depth:
 * its frame is handed on; 0: not on the road).  Then by its state +0x6dc (the jump table at 0x40c9d4):
 * 0 (and past 5) made 1; 1 drives -- 0x40c270, the lanes planned (beginPlanning, weighVotes,
 * reactToCars, 0x40bff0, pickLane) and the votes acted on (0x40de30 with 0) -- and pulls over (3) when
 * a police car is near (policeNear by 0); 3 slows to the roadside (slowToPullOver, voteForRoadside,
 * the obstacles, the cars, the close ones, 0x40de30 with 2) -- back to 1 when the police car has gone
 * (by 2), to 4 at the roadside; 4 stands (its speed asked 0, stopPulledOver) until the police car has
 * gone (1); 2 and 5 nothing.  False when the game stopped. */
bool driveTraffic(App* app, ThreadState& ts, u32 car)
{
    std::optional<u32> onRoad;
    {
        const GuestFrame frame(ts, 0xc, 0);
        const ReturnSlot slot(ts);
        onRoad = stepLoading(app, ts, car);
    }
    if (!onRoad)
        return false;
    if (*onRoad == 0)
        return true;
    const u32 state = at(app, car + kCarState);
    if (state == 0 || state == 1 || state > 5)
    {
        if (state != 1)
            put(app, car + kCarState, kStateDriving);
        if (!guest::direct<0x40c270, pursuit::trafficTargetOf>(app, ts, car) || !beginPlanning(app, ts, car))
            return false;
        weighVotes(app);
        if (!reactToCars(app, ts, car))
            return false;
        guest::direct<0x40bff0, pursuit::followChasedCar>(app, ts, car);
        if (ts.terminate || !pickLane(app, ts, car) || !actOnVotes(app, ts, car, 0))
            return false;
        if (policeNear(app, car, 0))
            put(app, car + kCarState, kStatePullingOver);
        return true;
    }
    if (state == kStatePullingOver)
    {
        if (!beginPlanning(app, ts, car) || !slowToPullOver(app, ts, car))
            return false;
        voteForRoadside(app, car);
        scoreObstacles(app, car);
        if (!scoreCars(app, ts, car))
            return false;
        penaliseCloseCars(app);
        if (!actOnVotes(app, ts, car, 2))
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
        if (!stopPulledOver(app, ts, car))
            return false;
        if (!policeNear(app, car, 2))
            put(app, car + kCarState, kStateDriving);
    }
    return true;  // 2 and 5
}

/* 0x40c9f0: driveTraffic for car eax.  What the code leaves in eax (the last yes or no it had) no
 * caller reads: it is the car's tick hook (+0x600); aiplan.inc's promise says so. */
nfs::u32 advanceCarAi(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return driveTraffic(app, ts, car); });
    return args.eax;
}

/* The sensed cars (0x48 bytes at 0x56ef38) cleared (0x4e0640) and [0x56ebc0] zeroed.  The eax the
 * clearing leaves, the buffer; nullopt when the game stopped. */
std::optional<u32> clearSensed(App* app, ThreadState& ts)
{
    const u32 cleared = guest::direct<0x4e0640, watcom::clib::memset>(app, ts, kSensed, 0, 0x48);
    if (ts.terminate)
        return std::nullopt;
    put(app, kSensedRound, 0);
    return cleared;
}

/* 0x40cb20: clearSensed; eax as the clearing leaves it. */
nfs::u32 clearSensedCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return clearSensed(app, ts); });
    return args.eax;
}

/* The track's traffic side [0x56f230] (trackTrafficSide) kept and [0x56ebc0] zeroed; the side. */
u32 keepTrafficSide(App* app)
{
    const u32 side = trackTrafficSide(app);
    put(app, kTrafficSide, side);
    put(app, kSensedRound, 0);
    return side;
}

/* 0x40cb50: keepTrafficSide. */
nfs::u32 cacheTrafficSide(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return keepTrafficSide(app); });
    return args.eax;
}

/* Traffic car reset: its record (clearRecord), then 0x404690 (below this file).  The eax 0x404690
 * leaves; nullopt when the game stopped. */
std::optional<u32> resetTrafficEax(App* app, ThreadState& ts, u32 car)
{
    clearRecord(app, car);
    return linkTraffic(app, ts, car);
}

/* resetTrafficEax for the callers that ask only whether the game went on.  False when it stopped. */
bool resetTraffic(App* app, ThreadState& ts, u32 car)
{
    return resetTrafficEax(app, ts, car).has_value();
}

/* 0x40cb70: resetTraffic for car eax; eax as 0x404690 leaves it. */
nfs::u32 resetTrafficCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return resetTrafficEax(app, ts, car); });
    return args.eax;
}

/* Traffic car's driving state cleared: states, targets, lanes and speeds zeroed, +0x7bc 1.0, its next
 * decision +0x634 the clock [0x7d3684] + 0x500, +0x710 bit 4 cleared; +0x620 bit 1 for a car of the
 * kinds +0x200 bit 4 or 5, +0x200 bit 1 cleared for a player's car (bit 2).  In the code's order. */
void clearRecord(App* app, u32 car)
{
    for (const u32 off : {0x630u, 0x6d8u, 0x64cu, 0x650u, 0x654u, 0x658u, 0x65cu, 0x6dcu, 0x6e0u, 0x6e4u, 0x6e8u,
                            0x2c8u, 0x6f0u, 0x704u, 0x708u, 0x70cu, 0x710u, 0x714u, 0x718u, 0x730u, 0x71cu, 0x720u,
                            0x724u, 0x734u, 0x7acu, 0x7b4u, 0x7b0u, 0x7c0u, 0x7c4u})
        put(app, car + off, 0);
    put(app, car + 0x7bc, 0x3f800000);
    put(app, car + kCarAcross, 0);
    put(app, car + kCarHalfWidth, 0);
    put(app, car + 0x7c8, 0);
    put(app, car + 0x624, 0);
    put(app, car + 0x628, 0);
    const u32 decision = at(app, kClock) + 0x500;
    put(app, car + kCarBackwards, 0);
    put(app, car + 0x634, decision);
    const u8 sirenFlags = byte(app, car + kCarSirenFlags);
    put(app, car + 0x62c, 0);
    putByte(app, car + kCarSirenFlags, u8(sirenFlags & 0xef));
    put(app, car + kCarTrafficFlags, 0);
    const u8 kind = byte(app, car + kCarKindFlags);
    put(app, car + 0x7d0, 0);
    if ((kind & 0x10) != 0)
        putByte(app, car + kCarTrafficFlags, u8(byte(app, car + kCarTrafficFlags) | 2));
    if ((byte(app, car + kCarKindFlags) & 0x20) != 0)
        putByte(app, car + kCarTrafficFlags, u8(byte(app, car + kCarTrafficFlags) | 2));
    const u8 kindNow = byte(app, car + kCarKindFlags);
    if ((kindNow & kKindPlayer) != 0)
        putByte(app, car + kCarKindFlags, u8(kindNow & 0xfd));
}

/* 0x40cb90: clearRecord for car eax. */
nfs::u32 clearTrafficRecord(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { clearRecord(app, car); });
    return args.eax;
}

/* Car put on the track at its node +0x1c and its target place across +0x7b4 (as 16.16, with 1 and
 * 0x40cb08's for the place's other two parts; 0x4a0aa0, its direction by the reversed race and +0x621
 * bit 4), its velocity +0xa4 its forward vector +0xd8 scaled by its look-ahead +0x7c4 (0x4e0050), its
 * spin and slips zeroed.  The place and the vector in the code's frame, whose addresses the two are
 * handed.  False when the game stopped. */
bool putOnTrack(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x1c);
    const u32 place = frame - 0x10, vector = frame - 0x1c;
    const u32 direction = ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 1 : 0) ^ gameSettings(app).backwards;
    put(app, place, u32(fp::toInteger<i32>(fp::mul(f32(app, car + kCarTargetAcross), f64(app, kToFixedB)))));
    put(app, place + 4, 0x10000);
    put(app, place + 8, at(app, kPlaceOnTrackHeight));
    if (!placeOnRoad(app, ts, car, at(app, car + kCarNode), place, direction))
        return false;
    for (u32 i = 0; i < 0xc; i += 4)
        put(app, vector + i, at(app, car + kCarForward + i));
    if (!scaleVectorsDirect(app, ts, 1, vector, at(app, car + kCarLookAhead), vector))
        return false;
    for (u32 i = 0; i < 0xc; i += 4)
        put(app, car + kCarVelocity + i, at(app, vector + i));
    put(app, car + kCarSpin, 0);
    put(app, car + kCarSpin + 4, 0);
    put(app, car + kCarSpin + 8, 0);
    app->getMemory<u16>(car + kCarAirborne) = 0;
    put(app, car + 0x5c0, 0);
    put(app, car + 0x5c4, 0);
    put(app, car + 0x64c, 0);
    return true;
}

/* 0x40cd80: putOnTrack for car eax, left in eax. */
nfs::u32 placeOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax]() -> std::optional<u32> {
        if (!putOnTrack(app, ts, car))
            return std::nullopt;
        return car;
    });
    return args.eax;
}

/* What stopPulledOverEax did: whether the car was stopped, and what the code leaves in eax -- the car
 * off the ground, the car with the grip's compare's status word in ax where that ended it, else the
 * last scaling's. */
struct Stop
{
    bool stopped;
    u32 eax;
};

/* The pulled-over car brought to a stop: once on the ground (+0x164 0) and well on the road's grip
 * (+0x10c over 0.9), its drive reset (0x4aa79f), its spin +0xe4 and velocity +0xa4 halved (0x4e0050),
 * +0x509 0xff.  Nullopt when the game stopped. */
std::optional<Stop> stopPulledOverEax(App* app, ThreadState& ts, u32 car)
{
    if (app->getMemory<u16>(car + kCarAirborne) != 0)
        return Stop{false, car};
    if (!(f32(app, car + kCarGrip) > f64(app, kFirmGrip)))
        return Stop{false, (car & 0xffff0000u) | statusWord(f32(app, car + kCarGrip), f64(app, kFirmGrip))};
    put(app, car + 0x64c, 0);
    guest::direct<0x4aa79f, handling::resetDrive>(app, ts, car);
    if (ts.terminate)
        return std::nullopt;
    if (!scaleVectorsDirect(app, ts, 1, car + kCarSpin, kHalf, car + kCarSpin))
        return std::nullopt;
    // By address: its eax is what brakeToStop leaves.
    const std::optional<u32> scaled = scaleVectors(app, ts, 1, car + kCarVelocity, kHalf, car + kCarVelocity);
    if (!scaled)
        return std::nullopt;
    putByte(app, car + kCarBrake, 0xff);
    return Stop{true, *scaled};
}

/* stopPulledOverEax: whether the car was stopped; nullopt when the game stopped. */
std::optional<bool> stopPulledOver(App* app, ThreadState& ts, u32 car)
{
    const std::optional<Stop> stop = stopPulledOverEax(app, ts, car);
    if (!stop)
        return std::nullopt;
    return stop->stopped;
}

/* 0x40ce70: stopPulledOver for car eax; eax as the code leaves it: the car, with the grip's compare's
 * status word in ax when that ended it, or the last scaling's. */
nfs::u32 brakeToStop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return stopPulledOverLeaving(app, ts, car); });
    return args.eax;
}

/* stopPulledOver with the eax the code leaves; nullopt when the game stopped. */
std::optional<u32> stopPulledOverLeaving(App* app, ThreadState& ts, u32 car)
{
    const std::optional<Stop> stop = stopPulledOverEax(app, ts, car);
    if (!stop)
        return std::nullopt;
    return stop->eax;
}

/* Car's present slice +0x638 votes into the first row of lane votes (0x56ee90..98) for the lanes
 * about it, by its way against the track's (+0x621 bit 4, the reversed race) and the track's traffic
 * side [0x56f230]: with the track's direction on a side of 1, or against it on a side of -1, it keeps
 * right -- a lane left of slice 7 loses 3, the others gain 1; else it keeps left, the other way
 * round (the code's own few exceptions kept). */
void voteForCurrentLane(App* app, u32 car)
{
    const bool against = (byte(app, car + kCarWayFlags) & kAgainstTrack) != 0;
    const u32 side = at(app, kTrafficSide);
    const bool keepRight = ((((against ? 0u : 1u) ^ gameSettings(app).backwards) != 0 && side == 1)
                          || (((against ? 1u : 0u) ^ gameSettings(app).backwards) != 0 && side == 0xffffffff));
    const auto add = [&](u32 vote, u32 by) { put(app, vote, at(app, vote) + by); };
    if (keepRight)
    {
        add(kLaneCostA, i32(at(app, car + kCarSlice) - 1) < 7 ? u32(-3) : 1);
        add(kLaneCostB, i32(at(app, car + kCarSlice)) < 7 ? u32(-3) : 1);
        if (i32(at(app, car + kCarSlice) + 1) >= 7)
            add(kLaneCostC, 1);
    }
    else
    {
        if (i32(at(app, car + kCarSlice) - 1) < 7)
            add(kLaneCostA, 1);
        else
            add(kLaneCostB, i32(at(app, car + kCarSlice)) < 7 ? 1 : u32(-3));
        add(kLaneCostC, i32(at(app, car + kCarSlice) + 1) < 7 ? 1 : u32(-3));
    }
}

/* 0x40cee0: voteForCurrentLane for car eax; eax the slice right of the car's, as the code leaves
 * it. */
nfs::u32 voteCurrentLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        voteForCurrentLane(app, car);
        return at(app, car + kCarSlice) + 1;
    });
    return args.eax;
}

/* Car senses the traffic about it.  Within a window of nodes set by its look-ahead (+0x7c4 by 2 and
 * 1/6, chopped; 10 at least), the other cars (in play, +0x89 set, +0x200 bit 6 clear) whose gap in
 * its direction of travel fits are gathered; then for each of the three lanes about its slice (-1, 0,
 * +1, from each one's sides' slices +0x63c..+0x640) the nearest ahead (0x56ef38, its gap 0x56ef44,
 * whether its middle is that lane 0x56ef68) and the nearest behind (0x56ef50, 0x56ef5c, 0x56ef6e) are
 * kept, and [0x56ef7c] raised for a car behind that drives against the track (+0x200 bit 5).  The
 * code gathers into a frame of 16; a C++ list here, the same for any count the game has.  False when
 * the game stopped. */
bool senseCars(App* app, ThreadState& ts, u32 car)
{
    const std::optional<Float> reach = chop(
        app, ts, fp::mul(fp::mul(f32(app, car + kCarLookAhead), f64(app, kSenseScale)), f64(app, kSenseSixth)));
    if (!reach)
        return false;
    i32 window = fp::toInteger<i32>(*reach);
    if (window < 0xa)
        window = 0xa;
    const u32 way = (((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u : 1u) ^ gameSettings(app).backwards) == 0
                          ? 0xffffffff
                          : 1;
    const u32 node = at(app, car + kCarNode);
    std::vector<u32> cars;
    std::vector<i32> gaps;
    for (i32 i = 0; i < i32(at(app, kCarCount)); ++i)
    {
        if (u32(i) == at(app, car + kCarIndex))
            continue;
        const u32 other = at(app, u32(i) * 4 + kCars);
        if (byte(app, other + kCarInPlay) == 0 || (byte(app, other + kCarKindFlags) & 0x40) != 0)
            continue;
        const i32 gap = i32(u32(nodesBetween(app, at(app, other + kCarNode), node)) * way);
        const i32 size = gap > 0 ? gap : i32(0u - u32(gap));
        if (size > window)
            continue;
        cars.push_back(other);
        gaps.push_back(gap);
    }
    put(app, kSensedGaps + 4, u32(window));
    put(app, kSensedGaps + 8, u32(window));
    put(app, kSensedBehindGaps, u32(window));
    put(app, kSensedGaps, u32(window));
    put(app, kSensedBehindGaps, 0u - u32(window));
    put(app, kSensedBehindGaps + 4, 0u - u32(window));
    put(app, kSensedBehindGaps + 8, 0u - u32(window));
    const u32 slice = at(app, car + kCarSlice);
    for (std::size_t s = 0; s < cars.size(); ++s)
    {
        const u32 other = cars[s];
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
                if (gap >= i32(at(app, u32(lane) * 4 + kSensedGaps)))
                    continue;
                put(app, u32(lane) * 4 + kSensed, other);
                put(app, u32(lane) * 4 + kSensedGaps, u32(gap));
                app->getMemory<u16>(u32(lane) * 2 + kSensedEdge) = u16(lane == middle ? 1 : 0);
            }
            else
            {
                if (gap <= i32(at(app, u32(lane) * 4 + kSensedBehindGaps)))
                    continue;
                put(app, u32(lane) * 4 + kSensedBehindGaps, u32(gap));
                put(app, u32(lane) * 4 + kSensedRight, other);
                app->getMemory<u16>(u32(lane) * 2 + kSensedEdgeB) = u16(lane == middle ? 1 : 0);
                if ((byte(app, at(app, u32(lane) * 4 + kSensedRight) + kCarKindFlags) & 0x20) != 0)
                    put(app, kSensedFlag, 1);
            }
        }
    }
    return true;
}

/* 0x40cfc0: senseCars for car eax.  What the code leaves in eax (the last lane's index) no caller
 * reads; aiplan.inc's promise says so. */
nfs::u32 senseTraffic(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return senseCars(app, ts, car); });
    return args.eax;
}

/* Car's way to node `target` costed by lane: the chain of nodes from its own (nodeAhead 1.. until the
 * target, 0x13 at most), then for each of the three lanes about its slice (sliceBit of the slices -1,
 * 0, +1): no road there at the car's node, that lane's route cost [0x56eec0 + lane] cut by 1000 and the
 * way [0x56ef78] set (1 left, 0, -1 right) -- the rest not looked at; else the first node of the chain
 * without it cuts 0x41. */
void costLaneChange(App* app, u32 car, u32 target)
{
    u32 masks[3];
    masks[0] = sliceBit(at(app, car + kCarSlice) - 1);
    masks[1] = sliceBit(at(app, car + kCarSlice));
    masks[2] = sliceBit(at(app, car + kCarSlice) + 1);
    u32 chain[0x14];
    chain[0] = at(app, car + kCarNode);
    if (target == chain[0])
        return;
    i32 length = 1;
    while (true)
    {
        chain[length] = nodeAhead(app, car, u32(length));
        if (target == chain[length])
            break;
        ++length;
        if (length >= 0x14)
            break;
    }
    for (i32 lane = 0; lane < 3; ++lane)
    {
        const u32 cost = u32(lane) * 4 + kNodeCostE;
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
nfs::u32 planLaneChange(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, target = args.edx] { costLaneChange(app, car, target); });
    return args.eax;
}

/* A node's edge slices as edgeLaneCost has them: 7 less the high nibble of its +0x13, 6 more the
 * low. */
i32 innerLeftSlice(App* app, u32 record) { return 7 - ((byte(app, record + kNodeEdges) >> 4) & 0xf); }
i32 innerRightSlice(App* app, u32 record) { return (byte(app, record + kNodeEdges) & 0xf) + 6; }

/* Car heading to node `target`: a player's car (+0x200 bit 0) on track 6 run forwards between its
 * nodes 0x127 and 0x168 has the second row's left vote (0x56eea8) cut by 0x28; else, when car's slice
 * +0x638 is off the target node's inner edges but within its own node's, that row's right (0x56eeb0,
 * slices 7 on) or left vote gains 0x28. */
void costEdgeLane(App* app, u32 car, u32 target)
{
    const u8 kind = byte(app, car + kCarKindFlags);
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
    const u32 targetRecord = nodeAddr(app, target);
    if (!(slice < innerLeftSlice(app, targetRecord)) && slice <= innerRightSlice(app, targetRecord))
        return;
    const u32 record = nodeAddr(app, at(app, car + kCarNode));
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
nfs::u32 edgeLaneCost(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, target = args.edx] { costEdgeLane(app, car, target); });
    return args.eax;
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
nfs::u32 edgePenalty(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { penaliseCloseCars(app); });
    return args.eax;
}

/* The 14 rows of lane votes summed by lane. */
struct LaneSums
{
    u32 left, middle, right;
};

LaneSums laneSums(App* app)
{
    LaneSums sums{0, 0, 0};
    for (u32 row = 0; row < 0xe; ++row)
    {
        sums.left += at(app, row * 0xc + kLaneCostA);
        sums.middle += at(app, row * 0xc + kLaneCostB);
        sums.right += at(app, row * 0xc + kLaneCostC);
    }
    return sums;
}

/* The lane [0x56ef74] car takes: its slice +0x638, one left when the left lane's sum beats the
 * middle's, one right when the right's beats the better of the two, kept within 0..13. */
void takeLane(App* app, u32 car)
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
nfs::u32 sumLaneVotes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        const LaneSums sums = laneSums(app);
        takeLane(app, car);
        const u32 slice = at(app, car + kCarSlice);
        u32 left = sums.middle, lane = slice;
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
        return left;
    });
    return args.eax;
}

/* The lane planner started for car: the votes (0xa8 bytes at 0x56ee90) and the sensed cars (0x48
 * at 0x56ef38) cleared (0x4e0640), its road slices found (findCarSlices), its present lane voted
 * (voteForCurrentLane) and the traffic sensed (senseCars).  False when the game stopped. */
bool beginPlanning(App* app, ThreadState& ts, u32 car)
{
    guest::direct<0x4e0640, watcom::clib::memset>(app, ts, kLaneCostA, 0, 0xa8);
    guest::direct<0x4e0640, watcom::clib::memset>(app, ts, kSensed, 0, 0x48);
    if (ts.terminate)
        return false;
    findCarSlices(app, car);
    voteForCurrentLane(app, car);
    return senseCars(app, ts, car);
}

/* 0x40dd60: beginPlanning for car eax.  What the code leaves in eax (senseTraffic's) no caller
 * reads; aiplan.inc's promise says so. */
nfs::u32 beginLanePlanning(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return beginPlanning(app, ts, car); });
    return args.eax;
}

/* Car picks its lane: it looks a number of nodes ahead by its look-ahead (+0x7c4 by 2.5 and 1/6,
 * less a half, rounded; 9 at least: nodeAhead), weighs the edge (costEdgeLane) and the way there
 * (costLaneChange), scores the lanes by the road (scoreRoad) and, but for a player's car (+0x200 bit
 * 2), by the obstacles, the cars and the close ones.  False when the game stopped. */
bool pickLane(App* app, ThreadState& ts, u32 car)
{
    const i32 reach = fp::toInteger<i32>(fp::add(
        fp::mul(fp::mul(f32(app, car + kCarLookAhead), f64(app, kLaneReach)), f64(app, kLaneSixth)), f64(app, kLaneLessHalf)));
    const u32 target = nodeAhead(app, car, u32(reach <= 9 ? 9 : reach));
    costEdgeLane(app, car, target);
    costLaneChange(app, car, target);
    scoreRoad(app, car);
    if ((byte(app, car + kCarKindFlags) & kKindPlayer) != 0)
        return true;
    scoreObstacles(app, car);
    if (!scoreCars(app, ts, car))
        return false;
    penaliseCloseCars(app);
    return true;
}

/* 0x40ddb0: pickLane for car eax; eax as the code leaves it: the car, or for a player's car what
 * scoreLanesByRoad leaves. */
nfs::u32 chooseLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax]() -> std::optional<u32> {
        if (!pickLane(app, ts, car))
            return std::nullopt;
        return (byte(app, car + kCarKindFlags) & kKindPlayer) != 0 ? roadScoreLeft(app, car) : car;
    });
    return args.eax;
}

/* Car's horn sounded for 0x40 ticks (+0x620 bit 3, its end +0x644 past the clock), its tone +0x660
 * on 0 -> 1 -> 2, any other back to 0. */
void soundHorn(App* app, u32 car)
{
    putByte(app, car + kCarTrafficFlags, u8(byte(app, car + kCarTrafficFlags) | kTrafficHorn));
    put(app, car + kCarHornEnd, at(app, kClock) + 0x40);
    const u32 tone = at(app, car + kCarHornTone);
    put(app, car + kCarHornTone, tone == 0 ? 1 : tone == 1 ? 2 : 0);
}

/* 0x40de50: soundHorn for car eax. */
nfs::u32 startHorn(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { soundHorn(app, car); });
    return args.eax;
}

/* Car's horn ended once the clock passes +0x644: +0x620 bit 3 cleared, its tone +0x660 back (1 -> 0,
 * 2 -> 1). */
void endHorn(App* app, u32 car)
{
    if ((byte(app, car + kCarTrafficFlags) & kTrafficHorn) == 0)
        return;
    if (!(i32(at(app, kClock)) > i32(at(app, car + kCarHornEnd))))
        return;
    putByte(app, car + kCarTrafficFlags, u8(byte(app, car + kCarTrafficFlags) & ~kTrafficHorn));
    const u32 tone = at(app, car + kCarHornTone);
    if (tone == 1)
        put(app, car + kCarHornTone, 0);
    else if (tone == 2)
        put(app, car + kCarHornTone, 1);
}

/* 0x40deb0: endHorn for car eax. */
nfs::u32 stopHorn(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { endHorn(app, car); });
    return args.eax;
}

/* Traffic car's loading steps (+0x630; 0x401240 and 0x405cd0..0x405e20 are below this file): 0 when
 * 0x401240 says it is not in play; step 0 (and past 2) placed (for +0x620 bit 1: 0x4a0aa0 at node 0
 * with the place at 0x40cb0c, in the code's frame, then 0x405d40) -- 0; 1 started (0x405e20,
 * 0x405dc0) -- 1, or 0 until it can; 2 running -- 1, or taken off (0x405cd0, 0x405d40) -- 0.  Nullopt
 * when the game stopped. */
std::optional<u32> stepLoading(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x10);
    const std::optional<bool> inPlay = guest::direct<0x401240, airace::wasReleased>(app, ts, car);
    if (!inPlay)
        return std::nullopt;
    if (!*inPlay)
        return 0;
    const u32 step = at(app, car + kCarLoading);
    if (step == 1)
    {
        const std::optional<u32> canStart = guest::direct<0x405e20, aidrive::stepExtraCar>(app, ts, car);
        if (!canStart)
            return std::nullopt;
        if (*canStart == 0)
            return 0;
        if (!guest::direct<0x405dc0, aidrive::takeOff>(app, ts, car))
            return std::nullopt;
        put(app, car + kCarLoading, 2);
        return 1;
    }
    if (step == 2)
    {
        const bool done = guest::direct<0x405cd0, aidrive::unwatched>(app, ts, car);
        if (ts.terminate)
            return std::nullopt;
        if (!done)
            return 1;
        if (!guest::direct<0x405d40, aidrive::trafficStartDone>(app, ts, car))
            return std::nullopt;
        put(app, car + kCarLoading, 1);
        return 0;
    }
    const u8 flags = byte(app, car + kCarTrafficFlags);
    put(app, car + kCarLoading, 2);
    if ((flags & 2) != 0)
    {
        const u32 place = frame - 0x10;
        for (u32 i = 0; i < 0xc; i += 4)
            put(app, place + i, at(app, kGridPlace + i));
        if (!placeOnRoad(app, ts, car, 0, place, gameSettings(app).backwards))
            return std::nullopt;
        if (!guest::direct<0x405d40, aidrive::trafficStartDone>(app, ts, car))
            return std::nullopt;
        put(app, car + kCarLoading, 1);
    }
    return 0;
}

/* 0x40df10: stepLoading for car eax. */
nfs::u32 stepTrafficLoading(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return stepLoading(app, ts, car); });
    return args.eax;
}

/* What scoreLanesByRoad's code leaves in eax: its node's record, or its right edge slice when it got
 * as far as reading it. */
u32 roadScoreLeft(App* app, u32 car)
{
    const u32 record = nodeAddr(app, at(app, car + kCarNode));
    return i32(at(app, car + kCarSlice)) + 1 < leftEdgeSlice(app, record) ? record : u32(rightEdgeSlice(app, record));
}

/* Car's lanes scored by the road's shape (the fourth row, 0x56eeb4/b8/bc).  At its node, a lane on an
 * edge slice is cut by 0x1e, and by 0x32 more when that edge (+0x1c or +0x20 less its slices times the
 * half's width) is nearer than the car's half length +0x114 (as 16.16, rounded, worked out where the
 * code does); a lane off the road is cut hard (1000). */
void scoreRoad(App* app, u32 car)
{
    const u32 record = nodeAddr(app, at(app, car + kCarNode));
    const i32 edges = byte(app, record + kNodeEdges);
    const i32 high = (edges >> 4) & 0xf, low = edges & 0xf;
    const i32 edgeLeft = 6 - high, edgeRight = low + 7;
    const i32 slice = i32(at(app, car + kCarSlice));
    const i32 left = slice - 1, right = slice + 1;
    const i32 leftEdge = i32(at(app, record + kNodeLeftEdge) - (u32(byte(app, record + kNodeLeftWidth)) << 15) * u32(high));
    const i32 rightEdge =
        i32(at(app, record + kNodeRightEdge) - (u32(byte(app, record + kNodeRightWidth)) << 15) * u32(low));
    const auto length = [&] { return fp::toInteger<i32>(fp::mul(f32(app, car + kCarHalfLength), f64(app, kToFixedC))); };
    const auto cut = [&](u32 vote, u32 by) { put(app, vote, at(app, vote) - by); };
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
nfs::u32 scoreLanesByRoad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        scoreRoad(app, car);
        return roadScoreLeft(app, car);
    });
    return args.eax;
}

/* Car's lanes scored against the parked obstacles (0x7dd850, 8 of 0x50 bytes: +0 in use, +0x34 its
 * node, +0x38 its slice): for each in use 1 to 5 nodes ahead in the car's direction of travel, the
 * lane (of the three about the car's slice +0x638) it stands in has its obstacle vote (0x56eecc +
 * lane) cut by 0x32, and in the car's own lane its speed asked +0x7b0 is eased to nine tenths. */
void scoreObstacles(App* app, u32 car)
{
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 obstacle = i * kObstacleSize + kObstacles;
        if (at(app, obstacle) == 0)
            continue;
        const u32 way =
            (((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u : 1u) ^ gameSettings(app).backwards) == 0 ? 0xffffffff : 1;
        const i32 ahead = i32(u32(nodesBetween(app, at(app, obstacle + kObstacleNode), at(app, car + kCarNode))) * way);
        if (ahead <= 0 || ahead > 5)
            continue;
        for (u32 lane = 0; lane < 3; ++lane)
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
nfs::u32 scoreLanesByObstacles(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { scoreObstacles(app, car); });
    return args.eax;
}

/* A cut halved towards 0, as the code's mov, sar 31, sub, sar 1 has it. */
u32 halved(u32 cut)
{
    return u32(i32(cut - u32(i32(cut) >> 31)) >> 1);
}

/* Car's lanes scored against the cars sensed ahead in each of the three lanes (0x56ef38).  One coming
 * the other way (the speeds' signs, +0x7c0) closing faster than 1: the time to meet (its gap by 6 and
 * 65536, chopped by 0x4dfd56, as 1/65536 over the closing speed) by -35 and 1/8, rounded, less from -35
 * cuts that lane's oncoming vote (0x56eed8 + lane).  One going the same way that the car's speed asked
 * +0x7b0 outruns, within 10 nodes: -8, or less the more it outruns under 3 (by -5/3, rounded, less 3),
 * with up to the gap past 5 back, cuts its ahead vote (0x56eee4 + lane).  Halved where the car is not
 * in that lane by its middle (0x56ef68).  False when the game stopped. */
bool scoreCars(App* app, ThreadState& ts, u32 car)
{
    for (u32 lane = 0; lane < 3; ++lane)
    {
        const u32 other = at(app, lane * 4 + kSensed);
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
                app, ts, fp::mul(fp::mul(Float(i32(at(app, lane * 4 + kSensedGaps))), f64(app, kMeetScale)), f64(app, kToFixedD)));
            if (!meet)
                return false;
            const i32 meetFixed = fp::toInteger<i32>(*meet);
            const Float time = fp::div(fp::mul(Float(meetFixed), f64(app, kFromFixed)), Float(closing));
            const i32 scaled = fp::toInteger<i32>(
                fp::add(fp::mul(fp::mul(time, f32(app, kMeetWeight)), f64(app, kEighth)), f64(app, kMeetLessHalf)));
            u32 cut = u32(-35) - u32(scaled);
            if (app->getMemory<u16>(lane * 2 + kSensedEdge) == 0)
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
        u32 cut = 0xfffffff8;
        if (!(outrun >= f64(app, kOutrunMost)))
            cut = u32(fp::toInteger<i32>(fp::add(fp::mul(fp::mul(outrun, f64(app, kOutrunWeight)), f64(app, kThird)),
                                                   f64(app, kMeetLessHalf))))
                  - 3;
        cut += u32(gap - 5 <= 0 ? 0 : gap - 5);
        if (app->getMemory<u16>(lane * 2 + kSensedEdge) == 0)
            cut = halved(cut);
        if (i32(cut) < 0)
            put(app, lane * 4 + kCarCostRight, at(app, lane * 4 + kCarCostRight) + cut);
    }
    return true;
}

/* 0x40d800: scoreCars for car eax.  What the code leaves in eax (a compare's status word and the
 * like) no caller reads; aiplan.inc's promise says so. */
nfs::u32 scoreLanesByCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return scoreCars(app, ts, car); });
    return args.eax;
}

/* Car's target place across +0x7b4 set for the chosen lane [0x56ef74].  On an edge slice of its node
 * (6 less the high nibble of +0x13, 7 more the low) it is the lane's offset from the middle times the
 * half's width (+0x17 left of slice 7, +0x1b right, << 15, as 1/65536), less or plus the car's half
 * length +0x114; else the same with half the width back towards the middle.  Then it is kept within
 * the node's right edge (+0x20, as 1/65536) less the half length and its left edge (+0x1c) plus it,
 * and moved by the half length the planner's way [0x56ef78] (the other way for a car against the
 * track).  The nodes' pointer written back, as the code does. */
void steerForLane(App* app, u32 car)
{
    const u32 nodes = at(app, kNodes);
    const u32 record = nodes + at(app, car + kCarNode) * kNodeSize;
    const u32 target = car + kCarTargetAcross;
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
        const u32 way = (byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0u - at(app, kSensedDir) : at(app, kSensedDir);
        putFloat(app, target, float(fp::add(fp::mul(Float(i32(way)), f32(app, car + kCarHalfLength)), f32(app, target))));
    }
    put(app, kNodes, nodes);
}

/* 0x40daf0: steerForLane for car eax.  What the code leaves in eax (the nodes' pointer, a compare's
 * status word in its low half) no caller reads; aiplan.inc's promise says so. */
nfs::u32 steerToLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { steerForLane(app, car); });
    return args.eax;
}

/* The computer cars' target speed at slice `slice`: the byte of the track's speed table [0x552558]
 * at slice / 2, or 64 when that table failed to load [0x56f22c]. */
float targetSpeed(App* app, u32 slice)
{
    if (at(app, kSpeedsFailed) != 0)
        return 64.0f;
    return float(byte(app, at(app, kSpeeds) + (slice >> 1)));  // fild of a byte: exact
}

/* 0x40e080: targetSpeed at slice eax onto the x87's stack; eax the table's byte, as the code leaves
 * it. */
nfs::u32 targetSpeedAt(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    float speed = 0;
    asThunk(ts, args, [&, slice = args.eax] {
        speed = targetSpeed(app, slice);
        return at(app, kSpeedsFailed) == 0 ? u32(byte(app, at(app, kSpeeds) + (slice >> 1))) : slice;
    });
    args.floatSet = true;
    args.floatResult = double(Float(speed));
    return args.eax;
}

/* The road's kind at slice `slice`: the high nibble of the speed table's second half's (0x55255c)
 * byte there; 7 when the table failed to load. */
u32 roadKind(App* app, u32 slice)
{
    if (at(app, kSpeedsFailed) != 0)
        return 7;
    return u32(byte(app, at(app, kSpeedsHalf) + slice) >> 4);
}

/* 0x40e0c0: roadKind at slice eax. */
nfs::u32 roadKindAt(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slice = args.eax] { return roadKind(app, slice); });
    return args.eax;
}

/* The track's road kind kept for the race: whether it is wide ([0x56f228], trackRoadWide), its
 * traffic side ([0x56f230], trackTrafficSide), the walk's counter [0x56f220] reset.  The side. */
u32 cacheRoad(App* app)
{
    put(app, kSpeedsReady, trackRoadWide(app));
    const u32 side = trackTrafficSide(app);
    put(app, kTrafficSide, side);
    put(app, kTrackWalked, 0);
    return side;
}

/* 0x40e0f0: cacheRoad, the side in eax. */
nfs::u32 cacheRoadWidth(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return cacheRoad(app); });
    return args.eax;
}

/* Node `node` less node `from`, wrapped round the track's [0x5dd940] nodes into -n/2..n/2; the
 * count written back, as the code does. */
i32 nodesBetween(App* app, u32 node, u32 from)
{
    const i32 count = i32(at(app, kNodeCount));
    const i32 half = count / 2;  // (n - (n >> 31)) >> 1: towards 0
    i32 gap = i32(node - from);
    if (gap > half)
        gap = i32(u32(gap) - u32(count));
    else if (gap < -half)
        gap = i32(u32(gap) + u32(count));
    put(app, kNodeCount, u32(count));
    return gap;
}

/* 0x40e120: nodesBetween node eax and node edx. */
nfs::u32 nodeGap(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, from = args.edx] { return u32(nodesBetween(app, node, from)); });
    return args.eax;
}

/* The node `steps` nodes on from car's node +0x1c in its direction of travel -- back for a car with
 * the track in a forward race, or one against it in a reversed race -- wrapped round the track; the
 * count written back, as the code does. */
u32 nodeAhead(App* app, u32 car, u32 steps)
{
    const u32 count = at(app, kNodeCount);
    const u32 forward = ((byte(app, car + kCarWayFlags) & kAgainstTrack) != 0 ? 0 : 1) ^ gameSettings(app).backwards;
    if (forward == 0)
        steps = 0u - steps;
    u32 node = steps + at(app, car + kCarNode);
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
nfs::u32 nodeOffset(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, steps = args.edx] { return nodeAhead(app, car, steps); });
    return args.eax;
}

/* The node car will reach: its look-ahead +0x7c4 times `scale` and 1/6, rounded, kept within `least`
 * and `most` (the most winning), backwards when +0x7d4 is 1 (nodeAhead). */
u32 nodeAheadBySpeed(App* app, u32 car, float scale, i32 least, i32 most)
{
    const i32 reach = fp::toInteger<i32>(
        fp::mul(fp::mul(f32(app, car + kCarLookAhead), Float(scale)), f64(app, kSixth)));
    i32 steps = reach < least ? least : reach;
    if (steps > most)
        steps = most;
    if (at(app, car + kCarBackwards) == 1)
        steps = -steps;
    return nodeAhead(app, car, u32(steps));
}

/* 0x40e1b0 (stdcall, 3 words: the scale, the least, the most): nodeAheadBySpeed for car eax. */
nfs::u32 nodesAheadBySpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, car = args.eax, scale = floatOf(word(app, ts, 1)), least = i32(word(app, ts, 2)),
         most = i32(word(app, ts, 3))] { return nodeAheadBySpeed(app, car, scale, least, most); },
        0xc);
    return args.eax;
}

/* Node `node`'s lane bits (the high half of its dword at +0xa, arithmetically) masked by `mask` --
 * which of the 16 lanes are road there. */
u32 laneBitsAt(App* app, u32 node, u32 mask)
{
    return u32(i32(at(app, nodeAddr(app, node) + kNodeLaneBits)) >> 16) & mask;
}

/* 0x40e200: laneBitsAt node eax by mask edx. */
nfs::u32 nodeLaneBits(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, mask = args.edx] { return laneBitsAt(app, node, mask); });
    return args.eax;
}

/* Whether node `node` has road in slice `slice`: laneBitsAt with the slice's bit (sliceBit). */
u32 hasLane(App* app, u32 node, u32 slice)
{
    return laneBitsAt(app, node, sliceBit(slice));
}

/* 0x40e220: hasLane at node eax in slice edx. */
nfs::u32 nodeHasLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, slice = args.edx] { return hasLane(app, node, slice); });
    return args.eax;
}

/* Slice `slice`'s bit among a node's 16 lane bits: bit 15 - (slice + 1), slice + 1 (as the code's
 * 8 - (7 - slice)) kept within 0..15. */
u32 sliceBit(u32 slice)
{
    i32 lane = i32(8u - (7u - slice));
    if (lane < 0)
        lane = 0;
    else if (lane >= 0x10)
        lane = 0xf;
    return u32(1) << (0xf - lane);
}

/* 0x40e270: sliceBit of slice eax. */
nfs::u32 laneBit(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slice = args.eax] { return sliceBit(slice); });
    return args.eax;
}

/* The road slice (0..13) `offset` across the road falls in at node `node`: from the centre slice
 * (6 left of it, 7 right), the half's width (+0x17 / +0x1b, << 15, as 1/65536) by four the index
 * into the lane-width table 0x56f0e0, whose entry times the offset is the slices from the centre.
 * Left is an offset under 0 (fcom: a NaN goes right). */
u32 roadSlice(App* app, u32 node, float offset)
{
    const u32 record = at(app, kNodes) + node * kNodeSize;
    const bool left = 0.0 > Float(offset);
    const i32 centre = left ? 6 : 7;
    const u32 width = byte(app, record + (left ? kNodeLeftWidth : kNodeRightWidth));
    const float halfWidth = float(fp::mul(Float(i32(width << 0xf)), f64(app, kFixedToFloat)));
    const i32 index = fp::toInteger<i32>(fp::mul(Float(halfWidth), f32(app, kQuarterLanes)));
    const i32 slices = fp::toInteger<i32>(fp::mul(f32(app, u32(index) * 4 + kLaneWidths), Float(offset)));
    i32 slice = centre + slices;
    if (slice < 0)
        slice = 0;
    if (slice > kSlices - 1)
        slice = kSlices - 1;
    return u32(slice);
}

/* 0x40e2b0 (stdcall, 1 word: the offset across the road): roadSlice at node eax. */
nfs::u32 roadSliceIndex(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, offset = floatOf(word(app, ts, 1))] { return roadSlice(app, node, offset); }, 4);
    return args.eax;
}

/* Car's three road slices at its node +0x1c (roadSlice): of its place across the road +0x7b8
 * (+0x638), and of its two sides, -+0x21c with half a slice back in (+0x63c, +0x640). */
void findCarSlices(App* app, u32 car)
{
    const u32 node = at(app, car + kCarNode);
    const u32 middle = roadSlice(app, node, app->getMemory<float>(car + kCarAcross));
    const Float leftSide = fp::sub(f32(app, car + kCarAcross), f32(app, car + kCarHalfWidth));
    put(app, car + kCarSlice, middle);
    const u32 left = roadSlice(app, node, float(fp::add(leftSide, f64(app, kHalfSlice))));
    const Float rightSide = fp::add(f32(app, car + kCarAcross), f32(app, car + kCarHalfWidth));
    put(app, car + kCarLeftSlice, left);
    const u32 right = roadSlice(app, node, float(fp::add(rightSide, f64(app, kLessHalfSlice))));
    put(app, car + kCarRightSlice, right);
}

/* 0x40e350: findCarSlices for car eax; eax the last slice, as the code leaves it. */
nfs::u32 carSlices(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return findCarSlicesLeaving(app, car); });
    return args.eax;
}

/* findCarSlices with the eax the code leaves: the last slice. */
u32 findCarSlicesLeaving(App* app, u32 car)
{
    findCarSlices(app, car);
    return at(app, car + kCarRightSlice);
}

/* A track's tuning record at `record`: `first` and `distance` (0xf and 0 in a knock-out race,
 * [0x6fd4fc] bit 3), whether the road is wide, the traffic's side, the two speeds and the last. */
void fillTuning(App* app, u32 record, u32 first, u32 distance, u32 roadWide, u32 side, u32 speed,
                u32 seriesSpeed, u32 last)
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
nfs::u32 fillTrackTuning(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, record = args.eax, first = args.edx, distance = args.ebx, roadWide = args.ecx, side = word(app, ts, 1),
         speed = word(app, ts, 2), seriesSpeed = word(app, ts, 3), last = word(app, ts, 4)] {
            fillTuning(app, record, first, distance, roadWide, side, speed, seriesSpeed, last);
        },
        0x10);
    return args.eax;
}

/* The race's tuning set up.  A handicap (-0.005 at night, and for each of the weather, the mirrored
 * and the reversed track) is summed; the lane-width table 0x56f0e0 filled (0, then 1 / (i / 4)); then
 * by the race type [0x6fd3b8] (the jump table at 0x40e400) the nine tracks' tuning records at 0x56ef80
 * filled (fillTuning: the side 1, the road not wide, 13.41 the last) -- fixed values for the single and
 * the cop races, for the two series by their variant: the handicap's (0.98, 0.99 or 1 more) for
 * variant 1, fixed values else -- the fourth record overridden by an option [0x55eb2e] bit 5; the
 * opponents' speeds loaded (0x40e000) and the highest of the computer's target speeds over the track's
 * nodes kept in [0x56f224].  False when the game stopped. */
bool setupTuning(App* app, ThreadState& ts)
{
    constexpr u32 kLast = 0x4156930c;  // 13.41
    const auto tune = [&](u32 record, u32 distance, u32 first, u32 speed, u32 seriesSpeed) {
        fillTuning(app, record, first, distance, 0, 1, speed, seriesSpeed, kLast);
    };
    float handicap = floatOf(gameSettings(app).night != 0 ? 0xbba3d70a : 0);
    for (u32 GameSettings::*flag : {&GameSettings::weather, &GameSettings::mirrored, &GameSettings::backwards})
        if (gameSettings(app).*flag != 0)
            handicap = float(fp::add(Float(handicap), f32(app, kHandicapStep)));
    put(app, kLaneWidths, 0);
    for (i32 i = 1; i < 0x50; ++i)
        putFloat(app, u32(i) * 4 + kLaneWidths, float(fp::div(1.0, fp::mul(Float(i), f32(app, kQuarter)))));
    const u32 raceType = gameSettings(app).raceType;
    if (raceType == 0)
    {
        static const u32 rows[9][5] = {
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
        static const u32 rows[9][5] = {
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
            const u32 slower = bitsOf(float(fp::add(Float(handicap), f64(app, kHandicapSlower))));
            const u32 slow = bitsOf(float(fp::add(Float(handicap), f64(app, kHandicapSlow))));
            const u32 even = bitsOf(float(fp::add(1.0, Float(handicap))));
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
            static const u32 knockouts[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                               0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f800000};
            static const u32 tournaments[9] = {0x3f6b851f, 0x3f70a3d7, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7,
                                                 0x3f6b851f, 0x3f70a3d7, 0x3f6b851f, 0x3f70a3d7};
            for (u32 i = 0; i < 9; ++i)
                tune(kTuning + kTuningSize * i, 0, 0, 0, knockout ? knockouts[i] : tournaments[i]);
        }
    }
    if ((byte(app, kStartFlags) & 0x20) != 0)
        tune(0x56efe0, 0x64, 1, 0x3f666666, 0x3f800000);
    if (!guest::direct<0x40e000, load::loadSpeeds>(app, ts))
        return false;
    const i32 nodes = i32(at(app, kNodeCount));
    put(app, kTopTargetSpeed, 0);
    for (i32 node = 0; node < nodes; ++node)
    {
        const float speed = targetSpeed(app, u32(node));
        if (Float(speed) > f32(app, kTopTargetSpeed))
            putFloat(app, kTopTargetSpeed, speed);
    }
    return true;
}

/* 0x40e410: setupTuning.  What the code leaves in eax (a compare's status word) no caller reads;
 * aiplan.inc's promise says so. */
nfs::u32 setupTrackTuning(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return setupTuning(app, ts); });
    return args.eax;
}

/* The opponents' speed table [0x552558] freed and forgotten.  What the code leaves in eax: the free's,
 * else `eax` as it was; nullopt when the game stopped. */
std::optional<u32> freeSpeeds(App* app, ThreadState& ts, u32 eax)
{
    const u32 table = at(app, kSpeeds);
    if (table != 0)
    {
        const std::optional<u32> freed = freeHandle(app, ts, table);
        if (!freed)
            return std::nullopt;
        put(app, kSpeeds, 0);
        return *freed;
    }
    return eax;
}

/* 0x40ec40: freeSpeeds; eax as the free leaves it. */
nfs::u32 freeSpeedTable(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, eax = args.eax] { return freeSpeeds(app, ts, eax); });
    return args.eax;
}

/* The track's traffic side: its tuning record's +0xc; 1 past the records. */
u32 trackTrafficSide(App* app)
{
    const i32 track = i32(gameSettings(app).track);
    if (track < kTrackCount)
        return at(app, u32(track) * kTuningSize + kTuning + kTuningSide);
    return 1;
}

/* 0x40ec70: trackTrafficSide. */
nfs::u32 trafficSide(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return trackTrafficSide(app); });
    return args.eax;
}

/* Whether the track's road is wide: its tuning record's +8; past the records, whether the second
 * node's left edge slices (the high nibble of its +0x13) are none. */
u32 trackRoadWide(App* app)
{
    const i32 track = i32(gameSettings(app).track);
    if (track < kTrackCount)
        return at(app, u32(track) * kTuningSize + kTuning + kTuningRoadWide);
    return (byte(app, at(app, kNodes) + kNodeSize + kNodeEdges) >> 4) == 0 ? 1 : 0;
}

/* 0x40eca0: trackRoadWide. */
nfs::u32 roadIsWide(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return trackRoadWide(app); });
    return args.eax;
}

/* The traffic's speed at node `node` by the track [0x6fd4ac] (the jump table at 0x40ed14) and the
 * stretch of it: slower through each track's hard stretches.  The speeds as the code has their
 * bits. */
float trafficSpeed(App* app, u32 node)
{
    u32 bits = 0x41a0ee49;
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
nfs::u32 trafficSpeedAt(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    float speed = 0;
    asThunk(ts, args, [&, node = args.eax] { speed = trafficSpeed(app, node); });
    args.floatSet = true;
    args.floatResult = double(Float(speed));
    return args.eax;
}

/* The track's tuning speed: its record's +0x18 in the race types 1 and 2 or in a series race, else
 * +0x14. */
float trackTuningSpeed(App* app)
{
    const u32 raceType = gameSettings(app).raceType;
    const u32 record = gameSettings(app).track * kTuningSize + kTuning;
    const bool series = raceType == 1 || raceType == 2 || gameSettings(app).seriesRace != 0;
    return app->getMemory<float>(record + (series ? kTuningSeriesSpeed : kTuningSpeed));
}

/* 0x40ee80: trackTuningSpeed onto the x87's stack; every register kept. */
nfs::u32 tuningSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    float speed = 0;
    asThunk(ts, args, [&] { speed = trackTuningSpeed(app); });
    args.floatSet = true;
    args.floatResult = double(Float(speed));
    return args.eax;
}

/* The track's tuning distance (its record's +4), +200 with the weather on and +400 at night, as
 * game_settings.h names the two. */
u32 trackTuningDistance(App* app)
{
    u32 distance = at(app, gameSettings(app).track * kTuningSize + kTuning + kTuningDistance);
    if (gameSettings(app).weather != 0)
        distance += 0xc8;
    if (gameSettings(app).night != 0)
        distance += 0x190;
    return distance;
}

/* 0x40eed0: trackTuningDistance. */
nfs::u32 tuningDistance(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return trackTuningDistance(app); });
    return args.eax;
}

/* The track's tuning record's +0. */
u32 trackTuningFirst(App* app)
{
    return at(app, gameSettings(app).track * kTuningSize + kTuning + kTuningFirst);
}

/* 0x40ef10: trackTuningFirst. */
nfs::u32 tuningFirst(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return trackTuningFirst(app); });
    return args.eax;
}

/* A node's forward direction's part at +0x14 or +0x16 as the code loads it: the signed byte << 9. */
i32 directionPart(App* app, u32 record, u32 part)
{
    return i32(u32(signedByte(app, record + part)) << 9);
}

/* The dot product of nodes `a`'s and `b`'s forward directions (directionPart, as 1/65536), in the
 * code's order: z's product, then x's added. */
Float directionDot(App* app, u32 a, u32 b)
{
    const u32 nodes = at(app, kNodes);
    const u32 recordB = nodes + b * kNodeSize, recordA = nodes + a * kNodeSize;
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
nfs::u32 nodeDirectionDot(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Float dot;
    asThunk(ts, args, [&, a = args.eax, b = args.edx] {
        dot = directionDot(app, a, b);
        return u32(directionPart(app, at(app, kNodes) + a * kNodeSize, kNodeDirectionZ));
    });
    args.floatSet = true;
    args.floatResult = double(dot);
    return args.eax;
}

/* Whether node `node` lies in the run of `run` nodes from node `from` (wrapped round the track; a
 * negative run counts back), the ends' compares unsigned as the code's; the count written back. */
bool nodeInRun(App* app, u32 node, u32 from, u32 run)
{
    const u32 count = at(app, kNodeCount);
    u32 end = from + run;
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
nfs::u32 nodeWithinRange(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, node = args.eax, from = args.edx, run = args.ebx] { return nodeInRun(app, node, from, run) ? 1 : 0; });
    return args.eax;
}

/* Whether car is within reach of `distance`: its place along the track +8 and 14 times its speed
 * along it +0x1fc against the distance and a margin by the race's laps [0x5efd94] (26 less in a
 * reversed race). */
bool carInReach(App* app, u32 car, u32 distance)
{
    u32 margin = 0x190;
    if (i32(at(app, kLaps)) < 5)
        margin = 0xc8;
    if (i32(at(app, kLaps)) < 2)
        margin = 0x32;
    if (gameSettings(app).backwards != 0)
        margin -= 0x1a;
    const u32 reached = at(app, car + kCarSpeedAlong) * 14 + at(app, car + kCarAlong);
    return !(i32(reached) > i32(margin + distance));
}

/* 0x40f050: carInReach for car eax of distance edx. */
nfs::u32 carWithinReach(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, distance = args.edx] { return carInReach(app, car, distance) ? 1 : 0; });
    return args.eax;
}

/* 1, whatever it is asked. */
u32 answerOne(App* app)
{
    (void)app;
    return 1;
}

/* 0x40f100: answerOne. */
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return answerOne(app); });
    return args.eax;
}

/* Whether car is boxed in: its slice +0x638 off the 14, or not within its node's edge slices and
 * neither neighbouring slice within them (and the 14); the nodes' pointer written back, as the code
 * does. */
bool boxedIn(App* app, u32 car)
{
    const u32 nodes = at(app, kNodes);
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
nfs::u32 carBoxedIn(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return boxedIn(app, car) ? 1 : 0; });
    return args.eax;
}

/* The track's animation paths (.can, loaded into the seven handles at
 * 0x570a34) and their players, 256 slots of 0x18 bytes at 0x56f234: +0 the
 * path playing, +4 the clock it started at, +8 a second mark, +0xc its mode,
 * +0x10 its flags (bit 0 mirrored), +0x14 a third word.  A path: +4 its
 * points' count, +6 the frames per point (1..400, else 6), from +8 its points
 * of 0x14 bytes -- a place of three fixed-point dwords and an orientation of
 * four shorts (+0xc). */
constexpr u32 kAnimStart = 0x56f238, kAnimMark = 0x56f23c, kAnimMode = 0x56f240, kAnimFlags = 0x56f244;
constexpr u32 kAnimThird = 0x56f248;

/* The seven animation paths (0x570a34) freed (0x4e1890) and forgotten; false when the game
 * stopped. */
std::optional<u32> freePathsEax(App* app, ThreadState& ts, u32 eax)
{
    for (u32 i = 0; i < 7; ++i)
    {
        const u32 path = at(app, i * 4 + kAnimPaths);
        if (path == 0)
            continue;
        const std::optional<u32> freed = freeHandle(app, ts, path);
        if (!freed)
            return std::nullopt;
        eax = *freed;
        put(app, i * 4 + kAnimPaths, 0);
    }
    return eax;
}

/* freePathsEax for the callers that ask only whether the game went on.  False when it stopped. */
bool freePaths(App* app, ThreadState& ts)
{
    return freePathsEax(app, ts, 0).has_value();
}

/* 0x40f330: freePaths; eax as the last free leaves it. */
nfs::u32 freeAnimationPaths(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, eax = args.eax] { return freePathsEax(app, ts, eax); });
    return args.eax;
}

/* The first free animation slot (its path +0 none), or -1. */
u32 freeAnimation(App* app)
{
    for (u32 slot = 0; slot < kAnimSlots; ++slot)
        if (at(app, slot * kAnimSize + kAnims) == 0)
            return slot;
    return 0xffffffff;
}

/* 0x40f370: freeAnimation. */
nfs::u32 findFreeAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return freeAnimation(app); });
    return args.eax;
}

/* Path `path` (0..6 of the handles) started in a free slot at the clock, mode 2, flags 1, the third
 * word 0 -- the slot, or -1 when none is free or the path is not loaded (its slot's path written 0
 * then, as the code does). */
u32 playAnimation(App* app, u32 path)
{
    const u32 slot = freeAnimation(app);
    if (slot == 0xffffffff)
        return slot;
    if (i32(path) >= i32(kAnimSlots))
        return 0xffffffff;
    const u32 record = slot * kAnimSize;
    const u32 handle = at(app, path * 4 + kAnimPaths);
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
nfs::u32 startAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, path = args.eax] { return playAnimation(app, path); });
    return args.eax;
}

/* Slot's mode, flags and third word set. */
void setAnimation(App* app, u32 slot, u32 mode, u32 flags, u32 third)
{
    put(app, slot * kAnimSize + kAnimMode, mode);
    put(app, slot * kAnimSize + kAnimFlags, flags);
    put(app, slot * kAnimSize + kAnimThird, third);
}

/* 0x40f430: setAnimation of slot eax to edx, ebx, ecx; eax three times the slot, as the code leaves
 * it. */
nfs::u32 setAnimationMode(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slot = args.eax, mode = args.edx, flags = args.ebx, third = args.ecx] {
        setAnimation(app, slot, mode, flags, third);
        return slot * 3;
    });
    return args.eax;
}

/* Slot stopped -- 1, or -1 when it was not playing. */
u32 endAnimation(App* app, u32 slot)
{
    const u32 record = slot * kAnimSize;
    if (at(app, record + kAnims) == 0)
        return 0xffffffff;
    put(app, record + kAnimStart, 0xffffffff);
    put(app, record + kAnimMark, 0);
    put(app, record + kAnims, 0);
    return 1;
}

/* 0x40f460: endAnimation of slot eax. */
nfs::u32 stopAnimation(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slot = args.eax] { return endAnimation(app, slot); });
    return args.eax;
}

/* A path's frames per point (+6: 1..400, else 6). */
i32 pathFrames(App* app, u32 path)
{
    const u16 frames = app->getMemory<u16>(path + kPathFrames);
    return frames >= 1 && frames <= 0x190 ? i32(frames) : 6;
}

/* A path's points less one (+4, a word). */
i32 pathSpans(App* app, u32 path)
{
    return i32(u32(app->getMemory<u16>(path + kPathPoints)) - 1);
}

/* cdq; idiv: the quotient and the remainder, as x86 has them. */
i32 quotient(i32 n, i32 by) { return i32(std::int64_t(n) / by); }
i32 remainder(i32 n, i32 by) { return i32(std::int64_t(n) % by); }

/* A point's orientation's part (one of four shorts from +0xc) as the code loads it: << 2. */
i32 orientationPart(App* app, u32 point, u32 part)
{
    return i32(u32(i32(s16(app->getMemory<u16>(point + kPointOrientation + part * 2)))) << 2);
}

/* Path's pose at `time` (in frames; mode 2 plays once): the two points the time falls between
 * (looping over the points, unless played once and past the end -- then 0) give two orientations
 * (four shorts, << 2, as 1/65536) and two places (16.16, as 1/65536), blended at the fraction of the
 * frame (0x4b70f0, `blendWord` handed on as its last word) and made a matrix at `matrix` (0x4b7590).
 * The orientations, the places and the blend's room in the code's frame, whose addresses the blend is
 * handed (and logs).  1; nullopt when the game stopped. */
std::optional<u32> poseAt(App* app, ThreadState& ts, u32 path, u32 mode, i32 time, u32 blendWord, u32 matrix)
{
    const GuestFrame frame(ts, 0xc, 0x7c);
    const i32 frames = pathFrames(app, path);
    const i32 step = quotient(time, frames);
    if (mode == 2 && step >= i32(u32(app->getMemory<u16>(path + kPathPoints)) - 1))
        return 0;
    const i32 index = remainder(step, pathSpans(app, path));
    const i32 rest = remainder(time, frames);
    const u32 point = path + kPathPointList + u32(index) * kPointSize;
    const u32 nextPoint = path + kPathPointList + u32(index + 1) * kPointSize;
    const Float scale = f64(app, kPoseScale);
    const u32 first = frame - 0x5c, second = frame - 0x6c, firstPlace = frame - 0x4c, secondPlace = frame - 0x34;
    // The first orientation, at -0x5c down to -0x50.
    Float parts[4];
    for (u32 i = 0; i < 4; ++i)
        parts[i] = fp::mul(Float(orientationPart(app, point, i)), scale);
    for (u32 i = 0; i < 4; ++i)
        putFloat(app, first + 4 * i, float(parts[i]));
    // The second, at -0x6c down to -0x60; the fraction of the frame between.
    const Float part0 = fp::mul(Float(orientationPart(app, nextPoint, 0)), scale);
    const Float fraction = fp::div(Float(rest), Float(frames));
    for (u32 i = 1; i < 4; ++i)
        parts[i] = fp::mul(Float(orientationPart(app, nextPoint, i)), scale);
    putFloat(app, second, float(part0));
    for (u32 i = 1; i < 4; ++i)
        putFloat(app, second + 4 * i, float(parts[i]));
    // The places, at -0x4c and -0x34 (the last part's product the other way round, as the code's).
    const auto place = [&](u32 at_, u32 to) {
        const Float x = fp::mul(Float(i32(at(app, at_))), scale);
        const Float y = fp::mul(Float(i32(at(app, at_ + 4))), scale);
        const Float z = fp::mul(scale, Float(i32(at(app, at_ + 8))));
        putFloat(app, to, float(x));
        putFloat(app, to + 4, float(y));
        putFloat(app, to + 8, float(z));
    };
    place(point, firstPlace);
    place(nextPoint, secondPlace);
    if (!feCore<0x4b70f0, frontend::simc::poseBlend>(app, ts, first, second, firstPlace, secondPlace,
                                                     bitsOf(float(fraction)), frame - 0x7c, blendWord))
        return std::nullopt;
    if (!feCore<0x4b7590, frontend::simc::matrixFromQuat>(app, ts, frame - 0x7c, matrix))
        return std::nullopt;
    return 1;
}

/* 0x40f4b0 (stdcall, 1 word: where the pose's matrix goes): poseAt for path eax at time ebx in mode
 * edx, ecx handed on. */
nfs::u32 animationPose(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, path = args.eax, mode = args.edx, time = i32(args.ebx), blendWord = args.ecx, matrix = word(app, ts, 1)] {
            return poseAt(app, ts, path, mode, time, blendWord, matrix);
        },
        4);
    return args.eax;
}

/* Path's place at `time` (`mode` bit 1: played once -- 0 past the end): the two points' places blended
 * at the frame's 16.16 fraction (0x4b6a30, `blendWord` handed on; the places and the blend's room in
 * the code's frame, whose addresses it is handed); the time within the path's length and the length
 * (both in frames) to `timeOut` and `lengthOut`.  1; nullopt when the game stopped. */
std::optional<u32> placeAt(App* app, ThreadState& ts, u32 path, u32 mode, i32 time, u32 blendWord, u32 timeOut,
                             u32 lengthOut)
{
    const GuestFrame frame(ts, 0xc, 0x44);
    const i32 frames = pathFrames(app, path);
    const i32 step = quotient(time, frames);
    if ((mode & 2) != 0 && step >= i32(u32(app->getMemory<u16>(path + kPathPoints)) - 1))
        return 0;
    const i32 index = remainder(step, pathSpans(app, path));
    const i32 fraction = quotient(i32(u32(remainder(time, frames)) << 0x10), frames);
    const u32 first = frame - 0x44, second = frame - 0x38;
    for (u32 i = 0; i < 0xc; i += 4)
        put(app, first + i, at(app, path + kPathPointList + u32(index) * kPointSize + i));
    for (u32 i = 0; i < 0xc; i += 4)
        put(app, second + i, at(app, path + kPathPointList + u32(index + 1) * kPointSize + i));
    if (!feCore<0x4b6a30, frontend::loopa::blend>(app, ts, frame - 0x1c, frame - 0x24, first, second,
                                                  u32(fraction), frame - 0x2c, blendWord))
        return std::nullopt;
    const i32 length = i32(u32(frames) * u32(pathSpans(app, path)));
    put(app, timeOut, u32(remainder(time, length)));
    put(app, lengthOut, u32(frames) * u32(pathSpans(app, path)));
    return 1;
}

/* 0x40f6d0 (stdcall, 2 words: where the time into the path goes, where its length goes): placeAt for
 * path eax at time ebx in mode edx, ecx handed on. */
nfs::u32 animationPlace(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, path = args.eax, mode = args.edx, time = i32(args.ebx), blendWord = args.ecx, timeOut = word(app, ts, 1),
         lengthOut = word(app, ts, 2)] { return placeAt(app, ts, path, mode, time, blendWord, timeOut, lengthOut); },
        8);
    return args.eax;
}

/* Slot's progress: its frame (the clock since it started, by its path's frames per point) within the
 * path's spans to `frameOut`, the spans (its points less one) to `spansOut`; the spans. */
u32 animationSpans(App* app, u32 slot, u32 frameOut, u32 spansOut)
{
    const u32 record = slot * kAnimSize;
    const i32 elapsed = i32(at(app, kClock) - at(app, record + kAnimStart));
    const u32 path = at(app, record + kAnims);
    const i32 step = quotient(elapsed, pathFrames(app, path));
    put(app, frameOut, u32(remainder(step, pathSpans(app, path))));
    const u32 spans = u32(pathSpans(app, path));
    put(app, spansOut, spans);
    return spans;
}

/* 0x40f800: animationSpans of slot eax to [edx] and [ebx]. */
nfs::u32 animationProgress(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slot = args.eax, frameOut = args.edx, spansOut = args.ebx] {
        return animationSpans(app, slot, frameOut, spansOut);
    });
    return args.eax;
}

/* Path's last point: its place copied to `placeOut`, its orientation (in the code's frame, whose
 * address it is handed) made a matrix at `matrix` (0x4b6ca0).  The eax 0x4b6ca0 leaves (animationLastPose
 * leaves it); nullopt when the game stopped. */
std::optional<u32> lastPose(App* app, ThreadState& ts, u32 path, u32 placeOut, u32 matrix)
{
    const GuestFrame frame(ts, 0xc, 8);
    const u32 point = path + kPathPointList + (u32(app->getMemory<u16>(path + kPathPoints)) - 1) * kPointSize;
    put(app, frame - 8, at(app, point + kPointOrientation));
    put(app, frame - 4, at(app, point + kPointOrientation + 4));
    for (u32 i = 0; i < 0xc; i += 4)
        put(app, placeOut + i, at(app, point + i));
    return feCore<0x4b6ca0, frontend::loopa::matrixOfRotation>(app, ts, frame - 8, matrix);
}

/* 0x40f880 (stdcall, 1 word: where the matrix goes): lastPose of path eax, its place to [ecx]; eax as
 * 0x4b6ca0 leaves it. */
nfs::u32 animationLastPose(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, path = args.eax, placeOut = args.ecx, matrix = word(app, ts, 1)] {
        return lastPose(app, ts, path, placeOut, matrix);
    }, 4);
    return args.eax;
}

/* Path `index`'s (of the handles) end pose (lastPose, at the code's depth, its place to `placeOut`,
 * its matrix to `pose`); mirrored when `flags` bit 0 is set: the matrix's first two rows swapped, the
 * second's negated -- as integers, the code's own way.  False when the game stopped. */
bool endPose(App* app, ThreadState& ts, u32 index, u32 flags, u32 placeOut, u32 pose)
{
    {
        const GuestFrame frame(ts, 8, 4);
        const ReturnSlot slot(ts, 1);
        if (!lastPose(app, ts, at(app, index * 4 + kAnimPaths), placeOut, pose))
            return false;
    }
    if ((flags & 1) == 0)
        return true;
    for (u32 column = 0; column < 0xc; column += 4)
    {
        const u32 second = at(app, pose + 0x18 + column), first = at(app, pose + 0xc + column);
        put(app, pose + 0xc + column, second);
        put(app, pose + 0x18 + column, first);
        put(app, pose + 0x18 + column, 0u - first);
    }
    return true;
}

/* 0x40f8d0 (stdcall, 1 word: the pose's matrix): endPose of path eax by flags ebx, its place to
 * [ecx]. */
nfs::u32 pathEndPose(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, index = args.eax, flags = args.ebx, placeOut = args.ecx, pose = word(app, ts, 1)]() -> std::optional<u32> {
            if (!endPose(app, ts, index, flags, placeOut, pose))
                return std::nullopt;
            return 1;
        },
        4);
    return args.eax;
}

/* Slot stepped: -1 when it is not playing; its path's pose at the clock since it started (poseAt at
 * the code's depth, its mode, `blendWord` handed on, the pose to `pose`) -- when that is past the end
 * the slot stops (0); mirrored by its flags' bit 0 (the pose's first two rows swapped, the second's
 * negated, as floats).  1; nullopt when the game stopped. */
std::optional<u32> stepAnimation(App* app, ThreadState& ts, u32 slot, u32 blendWord, u32 pose)
{
    if (!animationOn(app, slot))
        return 0xffffffff;
    const u32 record = slot * kAnimSize;
    std::optional<u32> posed;
    {
        const GuestFrame frame(ts, 0x10, 8);
        const ReturnSlot returnSlot(ts, 1);
        const i32 time = i32(at(app, kClock) - at(app, record + kAnimStart));
        posed = poseAt(app, ts, at(app, record + kAnims), at(app, record + kAnimMode), time, blendWord, pose);
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
        const u32 c = at(app, pose + 0xc);
        const Float d = f32(app, pose + 0x20);
        putFloat(app, pose + 0xc, float(a));
        putFloat(app, pose + 0x18, float(-Float(floatOf(c))));
        const u32 e = at(app, pose + 0x10);
        putFloat(app, pose + 0x10, float(b));
        putFloat(app, pose + 0x1c, float(-Float(floatOf(e))));
        const u32 g = at(app, pose + 0x14);
        putFloat(app, pose + 0x14, float(d));
        putFloat(app, pose + 0x20, float(-Float(floatOf(g))));
    }
    return 1;
}

/* 0x40f940: stepAnimation of slot eax, edx handed on, the pose to [ebx]. */
nfs::u32 animationStep(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slot = args.eax, blendWord = args.edx, pose = args.ebx] {
        return stepAnimation(app, ts, slot, blendWord, pose);
    });
    return args.eax;
}

/* Whether slot is playing (its path +0 set). */
bool animationOn(App* app, u32 slot)
{
    return at(app, slot * kAnimSize + kAnims) != 0;
}

/* 0x40fac0: animationOn for slot eax. */
nfs::u32 animationPlaying(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, slot = args.eax] { return animationOn(app, slot) ? 1 : 0; });
    return args.eax;
}

/* The caps the library is started with for the speaker setup: 0x808 for setup 0 and speakers 0, 0x804
 * for 0/1, 0x802 for 1/0, 0x801 for 1/1; none for any other. */
std::optional<u32> setupCaps(u32 setup, u32 speakers)
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
std::optional<u32> startLibrary(App* app, ThreadState& ts, u32 caps, u32 setup, u32 speakers, u32 callerEdx)
{
    const std::optional<u32> wanted = setupCaps(setup, speakers);
    if (!wanted)
        return callerEdx;
    u32 asked = 0xffffffff;
    if ((caps & *wanted) == *wanted)
        asked = at(app, kSoundInitOK) != 0 ? 0x20000 | *wanted : *wanted;
    const port::Result started = guest::direct<0x4e2630, port::libraryStarted>(app, ts, ts.sp - 4, asked, 0xffffffff);
    if (ts.terminate || !started)
        return std::nullopt;
    return u32(*started);
}

/* 0x418740 (the effect bus set up) as C++; false when the game stopped. */
bool busInitDone(App* app, ThreadState& ts)
{
    const port::Result done = guest::direct<0x418740, port::busInitOf>(app, ts, ts.sp - 4);
    return !ts.terminate && done;
}

/* The 3D setup's trace line (0x42d510, cdecl: 100, 1, 0x53684c): its words pushed as the code
 * pushes them, the trace (ea::gameTrace, compiled out) called as C++. */
bool traceSetup(App* app, ThreadState& ts)
{
    const u32 esp = ts.sp;
    put(app, esp - 12, 0x64);
    put(app, esp - 8, 1);
    put(app, esp - 4, kVolume);
    ts.sp = esp - 12;
    guest::direct<0x42d510, ea::traceOff>(app, ts);
    if (ts.terminate)
        return false;
    ts.sp = esp;
    return true;
}

/* The sound brought up at the race's start: the 23 voice records' (12 bytes at 0x571124) first words
 * cleared; the setup (2 taken as 0, 3 the 3D setup [0x5525a4]) and the speakers; when effects or music
 * are on, the library started (startLibrary, from the caps 0x4e21b0 gives); failing that both are
 * turned off, else the rate set (SNDsetrate 0x4e2228: 0, or -50 unless the rate's kind is set) and the
 * effect bus set up (0x418740).  `callerEdx` as startLibrary has it.  False when the game stopped. */
bool bringUpSound(App* app, ThreadState& ts, u32 callerEdx)
{
    const GuestFrame frame(ts, 0x18, 0);  // the callees at the code's depth: deeper down they log the stack
    for (u32 i = 0; i < 0x17; ++i)
        put(app, i * 0xc + kVoiceRecords, 0);
    u32 setup = gameSettings(app).soundSetup;
    const u32 speakers = gameSettings(app).soundSurround;
    if (setup == 2)
        setup = 0;
    const u32 rateKind = gameSettings(app).soundRateKind;
    if (setup == 3)
    {
        put(app, kSoundInitOK, 1);
        setup = 0;
    }
    else
        put(app, kSoundInitOK, 0);
    if (at(app, kEffectsOn) == 0 && at(app, kMusicOn) == 0)
        return true;
    const std::optional<u32> caps = callGame(app, ts, kSndCaps, {0xffffffff});
    if (!caps)
        return false;
    if (at(app, kSoundInitOK) != 0 && !traceSetup(app, ts))
        return false;
    const std::optional<u32> started = startLibrary(app, ts, *caps, setup, speakers, callerEdx);
    if (!started)
        return false;
    if (i32(*started) < 0)
    {
        put(app, kMusicOn, 0);
        put(app, kMusicSerial, 0);
        put(app, kEffectsOn, 0);
        return true;
    }
    guest::direct<0x4e2228, port::priorityOffsetSet>(app, ts, rateKind != 0 ? 0u : 0xffffffceu);
    return busInitDone(app, ts);
}

/* 0x40fb10: bringUpSound.  What the code leaves in eax no caller reads; aiplan.inc's promise says
 * so. */
nfs::u32 startSound(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, callerEdx = args.edx] { return bringUpSound(app, ts, callerEdx); });
    return args.eax;
}

/* The sound restarted for `setup` from the options screen (2 taken as 0, 3 the 3D setup): when effects
 * or music are on the library started again (startLibrary); failing that both are turned off, else the
 * effect bus set up (0x418740) and the caps asked again.  `callerEdx` as startLibrary has it.  False
 * when the game stopped. */
bool restartLibrary(App* app, ThreadState& ts, u32 setup, u32 callerEdx)
{
    const GuestFrame frame(ts, 0x18, 0);  // the callees at the code's depth: deeper down they log the stack
    if (setup == 2)
        setup = 0;
    const u32 speakers = gameSettings(app).soundSurround;
    if (setup == 3)
    {
        put(app, kSoundInitOK, 1);
        setup = 0;
    }
    else
        put(app, kSoundInitOK, 0);
    if (at(app, kEffectsOn) == 0 && at(app, kMusicOn) == 0)
        return true;
    const std::optional<u32> caps = callGame(app, ts, kSndCaps, {0xffffffff});
    if (!caps)
        return false;
    if (at(app, kSoundInitOK) != 0 && !traceSetup(app, ts))
        return false;
    const std::optional<u32> started = startLibrary(app, ts, *caps, setup, speakers, callerEdx);
    if (!started)
        return false;
    if (i32(*started) < 0)
    {
        put(app, kMusicOn, 0);
        put(app, kMusicSerial, 0);
        put(app, kEffectsOn, 0);
        return true;
    }
    return busInitDone(app, ts) && callGame(app, ts, kSndCaps, {0xffffffff});
}

/* 0x40fcf0: restartLibrary for setup eax.  What the code leaves in eax no caller reads; aiplan.inc's
 * promise says so. */
nfs::u32 restartSound(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, setup = args.eax, callerEdx = args.edx] { return restartLibrary(app, ts, setup, callerEdx); });
    return args.eax;
}

}  // namespace aiplan
}  // namespace nfs3hp
