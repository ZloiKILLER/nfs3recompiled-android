#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_frontend_sima.h"
#include "native_frontend_simb.h"
#include "native_frontend_simc.h"
#include "native_ea_threads.h"
#include "native_guest.h"
#include "native_pursuit.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "port_conv.h"
#include "port_sound_calls.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* The police of a Hot Pursuit race, native whole.
 *
 * What the code at 0x401220..0x404d12 is: not the opponents' driving (that is
 * 0x405000 on) but the cops' -- the patrol cars' driver (copDriver, a state
 * machine of twelve states at +0x70c), the pursuit of a speeder, the pull
 * over and the ticket, the roadblocks and the backup the radio calls in, the
 * cops' radio messages; and a few rules of the racers' own (the ramming
 * limits of a car class, the race leader, the catch-up speed).
 *
 * The cops are the cars of the table at 0x5efa88 ([0x5efd88] of them), each
 * with its index (+0x714) and its order -- 8 bytes at 0x56ea10 + index * 8
 * (+0x7ac): the order's kind (+0), whether it was taken (+1), a track node
 * (+2, a word), a second word (+4), a side (+6) and a flag (+7).  The racers
 * a cop may chase are those at 0x5efc48 ([0x5efda0] of them); each has its
 * pursuit, a record of 0x60 bytes at 0x56e410 + i * 0x60 (+0x628): how many
 * times it got away (+0), its worst speeding (+4), the cops who gave up
 * (+0x14), the pursuit's state (+0x1c: 0 none, 1 a chase, 2 pulled over),
 * when it started (+0x20), its radio's last call (+0x24), when the stop
 * ends (+0x28), the cops on it (+0x2c, their cars from +0x30), how many of
 * them have stopped by it (+0x50), the radio calls made (+0x54: backup 1,
 * roadblock 2, a second car 4, the stop 8), the roadblock's car (+0x58) and
 * the cop that called (+0x5c).  A cop's +0x624 is the car it chases and its
 * +0x628 that car's pursuit.  A car's: its node on the track (+0x1c), its
 * flags (+0x200: 1 a racer, 4 a player's, 8 a racer that rams; +0x620,
 * +0x621 bit 0x10 its side of the road, +0x710/+0x711 the cop's), its lane
 * (+0x638), its speeding (+0x6f0), its tickets (+0x2c8), its wanted speed
 * (+0x7b0), its speeds (+0x7c0 signed, +0x7c4), its pull-over spot (+0x8e4
 * on, +0x910 the place), the car it stopped by (+0x9a4), when it may be
 * chased again (+0x634).  [0x7d3684] is the race's clock in ticks;
 * [0x6fd4c0] that the race runs the track backwards; 0x554f10.. the race's
 * random numbers (seed * [0x554f18], the low 16 bits kept); the track's
 * nodes are 0x24 bytes at [0x5dd948], [0x5dd940] of them (their lanes at
 * +0x13, left in the high nibble, right in the low; their widths at +0x17,
 * +0x1b).
 *
 * Each function is a typed C++ core (fooOf and the like: named arguments, a
 * result, C++ locals; frames stay on the guest's stack only where a callee is
 * handed their address, GuestFrame) and a thunk under the game's name for
 * the callers still on registers: it takes the registers, calls the core and
 * leaves eax (edx, an x87 value) as the game's code does -- where that is a
 * leftover of the code's own (a status word fnstsw put in ax, a callee's
 * result, a loop's count), worked out from what the core says it did.  The
 * x87's arithmetic stays fp:: in the code's order of operands, each store to a
 * float a float.  The four switches go by the targets nfs3.exe's tables hold
 * (0x403018, 0x4032a4, 0x40466c, 0x40482c).  The other typed files' functions
 * (the opponents' driving's, the radio's, the track's places, the vectors') are
 * called as C++ through guest::direct, but where a thunk here leaves what
 * the callee leaves in a register beyond what its header gives; every other
 * function of the game's is called by address, its arguments in registers as
 * the code passes them. */
namespace nfs3hp
{
namespace airace
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
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using Float = nfs::Float;

// The race's clock; the track run backwards; the track's nodes and their count.
constexpr u32 kClock = 0x7d3684;
constexpr u32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The cops and their count; the racers and theirs; the pursuits; the orders.
constexpr u32 kCops = 0x5efa88, kCopCount = 0x5efd88;
constexpr u32 kRacers = 0x5efc48, kRacerCount = 0x5efda0;
constexpr u32 kPursuits = 0x56e410, kOrders = 0x56ea10;
// The race's random numbers: the seed, the product, the multiplier.
constexpr u32 kSeed = 0x554f10, kProduct = 0x554f14, kMultiplier = 0x554f18;
// What a cop sizes up of its target each tick (0x56ea58..0x56ea6b): the
// target turning, its side, the gap in nodes, the lane against the cop's,
// behind it; the lanes' votes.
constexpr u32 kTurning = 0x56ea58, kSide = 0x56ea5c, kGap = 0x56ea60, kLaneVote = 0x56ea64,
                kBehind = 0x56ea68;
// The radio's messages by a cop's radio level (+0x71c); the tables by a
// car's tickets (16 bytes each: the gaps a cop gives up at, the stop's
// length, the next pursuit's wait); a cop's siren state (0x50 bytes each).
constexpr u32 kRadioMessages = 0x55224c, kByTickets = 0x5521ac, kSirens = 0x7dd850;
// The ramming rules of each racer (0x14 bytes each).
constexpr u32 kRamming = 0x56ea70;

// What is called by address: the game's, ea::yRotation (whose frame on the guest's stack sits
// under the angle the code pushes), the lane votes (0x40de30, its thunk's NFS_CAR_TRACE record);
// and the typed files' calls whose thunks here leave what the callee leaves in a register beyond
// what its header gives -- the vectors' subtraction's, the timer's, the stop's, the horn's end's,
// a node's and a ram's eax.  The other typed files' functions are called as C++ (guest::direct),
// the menu files' (the distance, a key, the sirens, a car settled) too (feCore), their eax put
// back where the code leaves it.
constexpr u32 kChop = 0x4dfd56, kSubtract = 0x4e0000;
constexpr u32 kSetTimer = 0x405fd0;

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated). */
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

/* A menu core's result (feCore's) put in eax, as its call by address left it: false when the game
 * stopped. */
bool leftInEax(ThreadState&, const std::optional<u32>& eax)
{
    return bool(eax);
}

u16 word(App* app, u32 a) { return app->getMemory<u16>(a); }
void putWord(App* app, u32 a, u16 value) { app->getMemory<u16>(a) = value; }

// A float or a double of the guest's as the x87 loads it.
Float f32(App* app, u32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, u32 a) { return Float(app->getMemory<double>(a)); }

u32 neg(u32 v) { return u32(0) - v; }

u32 imul(u32 a, u32 b) { return u32(std::int64_t(s32(a)) * std::int64_t(s32(b))); }

using i32 = std::int32_t;

// A car's fields (the cops' own among them; see the head of the file).
constexpr u32 kCarNode = 0x1c, kCarFlags = 0x200, kCarHeading = 0x210, kCarDriver = 0x220,
                kCarTickets = 0x2c8, kCarState = 0x2c4, kCarAhead = 0x4f4, kCarHandling = 0x528,
                kCarPlayerSide = 0x5f4, kCarInput = 0x620, kCarSide = 0x621, kCarChased = 0x624,
                kCarPursuit = 0x628, kCarWatch = 0x62c, kCarLane = 0x638, kCarDriverState = 0x6e0,
                kCarSpeeding = 0x6f0, kCarCopFlags = 0x710, kCarCopKind = 0x711, kCarCopIndex = 0x714,
                kCarRadioLevel = 0x71c, kCarRadioDue = 0x724, kCarOrder = 0x7ac, kCarWanted = 0x7b0,
                kCarWantedSign = 0x7b3, kCarSpeed = 0x7c4, kCarSpotOn = 0x8e4, kCarSpotSet = 0x8e8;
// A car's flags (+0x200): a racer, a player's, one that rams; +0x621's other side of the road;
// +0x711's a fast cop.
constexpr u8 kFlagRacer = 1, kFlagPlayer = 4, kFlagRams = 8, kFlagFinished = 0x20, kSideOther = 0x10,
               kCopFast = 1;
// A pursuit's fields: its state (none, a chase, pulled over), the stop's end, the cops on it, their cars.
constexpr u32 kPursuitState = 0x1c, kPursuitStopEnds = 0x28, kPursuitCopCount = 0x2c, kPursuitCops = 0x30;
constexpr u32 kNoPursuit = 0, kChase = 1, kPulledOver = 2;
// A handling record's top speed; a driver's model kind of the fast cops.
constexpr u32 kHandlingTopSpeed = 0x1c8, kDriverModel = 0x14, kFastCopModel = 0x1f8;

// A car's watch of its input's bit 0 (+0x62c): free, held; the key asked with nothing held.
constexpr u32 kWatchFree = 1, kWatchHeld = 2, kReleaseKey = 8;
constexpr u32 kStoppingSpeed = 0x56ea50, kStoppingSpeedBits = 0x4132cfdf;  // a float, 11.18
constexpr u32 kRadioOn = 0x56ea54;
constexpr i32 kTopRadioLevel = 8;
constexpr u32 kFastCopScale = 0x536044;  // a double
constexpr i32 kFarGap = 10000;
constexpr u32 kRadioSpeed = 0x53604c;  // a double: the least speed the radio calls about
constexpr u32 kRadioCallTicks = 0x180;
constexpr u8 kRaceCopRadio = 0x10;
constexpr i32 kStopGap = 10;
constexpr u32 kStopTicks = 0x140;
// An order's kind, given at a node; a pursuit's counts of the cops sent (+4 + kind * 4).
constexpr u8 kOrderAtNode = 2;
constexpr u32 kOrderKind = 0, kOrderTaken = 1, kOrderNode = 2, kOrderWord = 4, kOrderSide = 6, kOrderFlag = 7;
constexpr u32 kCarOrderFrom = 0x730;
// A pursuit's count of the cops that gave up, its roadblock's car; the cops' joining.
constexpr u32 kPursuitGetaways = 0, kPursuitWorst = 4, kPursuitGaveUp = 0x14, kPursuitStarted = 0x20,
                kPursuitRadioDue = 0x24, kPursuitStoppedBy = 0x50, kPursuitCalls = 0x54, kPursuitRoadblock = 0x58,
                kPursuitCaller = 0x5c;
constexpr u32 kCarJoin1 = 0x728, kCarJoin2 = 0x72c, kCarOnPursuit = 0x734;
constexpr u8 kCopSent = 4, kCopRoadblock = 8, kCopChasing = 0x10;
constexpr u32 kSirenSize = 0x50, kFirstRadioTicks = 0x280;
// joinPursuit's kinds: by the cop's own sighting, a player's cop.
constexpr u32 kJoinSighted = 0, kJoinPlayerCop = 3;
constexpr u32 kOutOfRace = 3, kOutOfPursuits = 4;  // a car's states (+0x2c4) of one out
constexpr u32 kSpeedingBitsWanted = 0x40000000;   // 2.0's bits
constexpr i32 kPlayerCopReach = 20;
// What a cop sizes up of its target: turning, on its own side, behind; the lanes' vote.
constexpr u32 kLaneLeft = 0, kLaneSame = 1, kLaneRight = 2;
constexpr u32 kNotOnList = 8;  // more than a pursuit's list holds
constexpr u8 kSideNoGetaway = 0x40;
constexpr u8 kCopStopping = 0x80, kCopFlagsDropped = 0x6f;
constexpr u32 kCarPlace = 0x98, kCarSpotPlace = 0x910, kCarSpotHeight = 0x914, kCarHeight = 0x118,
                kCarSpotSide = 0x190, kCarSpotTurn = 0x8ec;
// The heading's turn to radians (1/1024 of a turn) above 0, and from -0x200 on; the spot's quarter
// turn and offset, right (left negated).
constexpr u32 kHeadingScale = 0x536054, kHeadingScaleLow = 0x53605c, kHeadingStartLow = 0x536064;
constexpr u32 kSpotTurnBits = 0x3e4ccccd, kSpotTurnLeftBits = 0xbe4ccccd;  // 0.2, -0.2
constexpr u32 kSpotOffsetBits = 0x40000000, kSpotOffsetLeftBits = 0xc0000000;  // 2, -2
constexpr u32 kHalfTurn = 0x200;
constexpr u32 kIdentity = 0x564498;   // a matrix, nine floats
constexpr u32 kNodeScale = 0x53606c;  // a double: a node's fixed point to the world's
constexpr u32 kNodeSize = 0x24, kCarFacing = 0x1a8;
constexpr u32 kPlaceStill = 0x6c, kPlaceOnRoad = 0x14, kPlacePolygon = 0x18;  // a place on the track's
constexpr u8 kPolygonKindMask = 0x3f, kPolygonNoStop = 0xe;
constexpr u32 kCarTally = 0x2cc, kCarFines = 0x2d0, kCarWonAt = 0x2d4, kCarNextChase = 0x634,
                kCarStoppedBy = 0x9a4;
constexpr u32 kTicketWait = kByTickets + 8, kTicketStop = kByTickets + 0xc;
constexpr u32 kWonTicks = 0x200;
constexpr u8 kSideKeep = 0xbf;
constexpr u32 kFarBits = 0x461c4000;  // 10000
constexpr u32 kPlayerCopStopTicks = 0xc0;
// The classes' table (24 bytes a class, by a model's +0x30): two values, the rule, the kind.
constexpr u32 kModelClass = 0x30, kClassSize = 0x18, kClassValue = 0x55226c, kClassRule = 0x552274,
                kClassKind = 0x55227c, kClassValueToo = 0x552280;
constexpr u32 kRammingSize = 0x14, kCarPlaceInRace = 0x1f0;
constexpr u32 kRamGrip = 0x3f70a3d7;  // 0.94
constexpr u32 kCarForward = 0x10c, kCarAirborne = 0x164;
constexpr u32 kCarHit = 0x16c, kCarHitForce = 0x168;
constexpr u32 kHardHitBits = 0x41700000;  // 15
constexpr u32 kHitForce = 0x53619c, kClassSlowing = 0x5361a4;  // doubles
constexpr u32 kCatchUpSpeed = 0x56ebb0;
constexpr u32 kCatchUpFastBits = 0x41a0ee49, kCatchUpSlowBits = 0x4132cfdf;  // 20.12, 11.18
// The players' cars and their count; the other racers and theirs; the leader, and whether others
// have more laps.
constexpr u32 kPlayers = 0x5efa48, kPlayerCount = 0x5efda4, kOthers = 0x5efb88, kOtherCount = 0x5efd94,
                kLeader = 0x56ebb8, kLeaderBehind = 0x56ebb4;
constexpr u32 kCarLaps = 8, kCarPlayerIndex = 0x1f4;
constexpr u8 kPursuitStoppedCall = 8;
constexpr u8 kCopToldStop = 0x10;
constexpr u32 kPursuitSize = 0x60, kPursuitsBytes = 0x600;
constexpr u32 kNodeStride = 20, kSideOrder = 0x56f230, kSideOrderSet = 0x56f228;
constexpr u32 kNodeLanes = 0x13, kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b;
constexpr u32 kWidthScale = 0x5360dc, kLaneShare = 0x5360e4;  // doubles
constexpr u32 kCarSideways = 0x7b4, kCarLaneOffset = 0x654, kCarSignedSpeed = 0x7c0;
constexpr u32 kOneBits = 0x3f800000, kPatrolFactorBits = 0x3f9851ec;  // 1, 1.19
constexpr u32 kRaceCars = 0x5efcc8, kRaceCarCount = 0x5efd9c, kCarWatched = 0x6d8;
constexpr u32 kOncomingSpeed = 0x5360d4;  // a double
constexpr i32 kSideSwitchGap = 12;
constexpr u32 kCopsWantedOver = 0x536104;  // a double: over the node's speed by it
constexpr u32 kHitRatio = 0x5360ec;  // a double
constexpr u32 kCarDriverStep = 0x70c, kParkedStep = 6;
constexpr u32 kJoinHit = 1;
constexpr u32 kVoteLeft = 0x56eeb4, kVoteOff = 0x56eeb8, kVoteRight = 0x56eebc, kVote = 0x64;
constexpr u32 kParkReach = 0x5360f4;  // a double
constexpr u32 kQuarterTurnBits = 0x3e800000, kThreeQuartersBits = 0x3f400000;  // 0.25, 0.75
constexpr u32 kCarOrientation = 0xc0;
constexpr u8 kSideRoadblock = 8;
constexpr u32 kSirenNode = 0x34;
constexpr u8 kCopPassed = 0x20;
constexpr i32 kPassedGap = -0x1e, kDoneGap = -0x32, kMissedGap = -0x14, kBehindGap = -5;
constexpr u32 kWaitingStep = 0xa;
constexpr u8 kOrderCutOff = 3;

constexpr u32 kCarToldSpeed = 0x7d4;
// The chase's speeds: by the gap behind and ahead for a cop told to keep by it, the target's by
// the leader's and another's, the cop's slowest before its timer, the stop's speed (-9.83), the
// target's ahead and its difference by the gap (0x5360b4, a float).
constexpr u32 kToldBehind = 0x53608c, kToldAhead = 0x536094, kLeaderShare = 0x53609c, kOtherShare = 0x5360a4,
                kTimerSpeed = 0x5360ac, kAheadOver = 0x5360bc, kAheadByGap = 0x5360b4;
constexpr u32 kChaseLeader = 0x56ef54, kBrakeSpeedBits = 0xc11d5ac4;
constexpr i32 kTimerGap = 7, kTopGap = 8;
// The lanes' votes against (70) and for (10, 30), by the lane against the cop's.
constexpr u32 kLaneAgainst = 0x56eed8, kLaneFor = 0x56ef2c;
constexpr u32 kSlowTarget = 0x5360cc;  // a double
constexpr u8 kCopHolds = 2;
constexpr i32 kFarAhead = 5;
constexpr u32 kFirstCar = 0x5efac8, kCarMarked = 0x50d, kCarRamFor = 0x6e4, kCarRammed = 0x6e8;
constexpr u32 kRival = 0x56ef3c, kRivalsBetween = 0x56ef48, kRivalSlower = 0x536194;  // a double
constexpr u32 kRamTicks = 0xc350;
constexpr u8 kInputNoLane = 8;
constexpr u8 kRaceNoRams = 0x20;
constexpr u32 kRaceSwitches = 0x55eb2e, kCarHitUntil = 0x65c;
constexpr u32 kHitBits = 0x40000000;  // 2
constexpr u8 kFlagTimesHits = 2;
constexpr u32 kRamAsking = 0x64;

constexpr i32 kSpotReach = 0xf, kOrderReach = 5;
constexpr i32 kTimeToJoin = 0x3c0, kTimeAtOrder = 0x100;
constexpr u32 kCarWaitedAt = 0x728, kCarWaitedFrom = 0x72c;
constexpr i32 kRoadblockAhead = 0x32, kRoadblockBefore = 6, kRoadblockHeat = 4;
constexpr u8 kPursuitRoadblockCall = 2;
constexpr u32 kGiveUpGap = kByTickets, kFastGiveUpGap = kByTickets + 4;
constexpr u32 kNightShare = 0x5360c4;  // a double
constexpr u32 kCarBraking = 0x660;
constexpr i32 kChaseGap = 0xf;
// A cop's order kinds and its driver's steps (+0x70c) for them.
constexpr u8 kOrderToNode = 0, kOrderHold = 1, kOrderRoadblock = 3, kOrderSecondCar = 4;
constexpr u32 kStepToNode = 1, kStepHold = 2, kStepAtNode = 3, kStepRoadblock = 7, kStepSecondCar = 8;
constexpr u32 kStoppedReach = 0x536074, kStopReach = 0x53607c, kStopRolling = 0x536084;  // doubles
constexpr u32 kCarRadioCallDue = 0x720, kCarThrottleNow = 0x508;
constexpr u8 kCopCalledStop = 0x40;
constexpr u32 kStopCallTicks = 0xc0, kStopCallSoon = 0xa0, kStopCallLate = 0xe0, kStopCallEarlier = 0x13;
constexpr i32 kLapsApart = 0x3c;
constexpr u32 kBackupAlone = 0x280, kBackupReach = 0xc8;
constexpr u8 kPursuitBackupCall = 1, kPursuitSecondCall = 4;
constexpr u8 kCopFastKind = 1;

constexpr i32 kTwoCarsHeat = 3, kTwoCarsApart = 0x190;
constexpr u8 kOrderTwoCars = 4;
constexpr u16 kCopsBoxing = 0x410;
constexpr u32 kStepNew = 0, kStepPatrol = 1, kStepToBlock = 7, kStepToSecond = 8, kStepAtSecond = 9,
                kStepParking = 0xa, kStepParked = 0xb, kStepLanes = 3, kStepOffRoad = 4, kStepChase = 5,
                kStepStop = 6;
constexpr u32 kTwoBits = 0x40000000, kSixBits = 0x40c00000, kSixAndHalfBits = 0x40d00000;  // 2, 6, 6.5
constexpr u32 kParkSpeed = 0x5360fc;  // a double
constexpr u32 kSizingBytes = 0x14, kCallTicks = 0x140;
constexpr u8 kCopToNode = 0x20;

u32 floatBits(float f)
{
    u32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

float floatAt(App* app, u32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, u32 a, float f) { app->getMemory<float>(a) = f; }

/* The x87's status word as the last fcom here leaves it (c3, c2, c0: fp::compare's): kept for the
 * thunks' fnstsw ax (withStatus), which go with the dispatcher. */
u16 g_status = 0;

/* fcom's answers, with what they leave in the status word: a over b, not so for a NaN; a below b,
 * so for a NaN (jbe's and jb's after fnstsw ax; sahf). */
bool compareAbove(const Float& a, const Float& b)
{
    g_status = fp::compare(a, b);
    return !(g_status & 0x0100) && !(g_status & 0x4000);
}

bool compareBelow(const Float& a, const Float& b)
{
    g_status = fp::compare(a, b);
    return (g_status & 0x0100) != 0;
}

/* eax as fnstsw ax leaves it: the status word in its low half. */
u32 withStatus(u32 eax) { return (eax & 0xffff0000) | g_status; }

/* 0x40e120 (aiplan::nodesBetween): the gap in nodes to `node` from `from`, signed; nullopt when
 * the game stopped. */
std::optional<i32> nodeGap(App* app, ThreadState& ts, u32 node, u32 from)
{
    const i32 gap = guest::direct<0x40e120, aiplan::nodesBetween>(app, ts, node, from);
    if (ts.terminate)
        return std::nullopt;
    return gap;
}

/* The gap's size as the code asks it: twice, the second negated where the first was not over 0. */
std::optional<i32> nodeGapSize(App* app, ThreadState& ts, u32 node, u32 from)
{
    const std::optional<i32> first = nodeGap(app, ts, node, from);
    if (!first)
        return std::nullopt;
    const std::optional<i32> second = nodeGap(app, ts, node, from);
    if (!second)
        return std::nullopt;
    return *first > 0 ? *second : i32(neg(u32(*second)));
}

/* 0x40ed40 (aiplan::trafficSpeed): the speed the track asks at node; nullopt when the game
 * stopped. */
std::optional<Float> nodeSpeed(App* app, ThreadState& ts, u32 node)
{
    const float speed = guest::direct<0x40ed40, aiplan::trafficSpeed>(app, ts, node);
    if (ts.terminate)
        return std::nullopt;
    return Float(speed);
}

/* 0x4be1a0 (simc's): the state of key `key` of the input `input`, in eax too; nullopt when the
 * game stopped. */
std::optional<u32> keyState(App* app, ThreadState& ts, u32 key, u32 input)
{
    const std::optional<u32> state = feCore<0x4be1a0, frontend::simc::offBeat>(app, ts, key, input);
    if (!leftInEax(ts, state))
        return std::nullopt;
    return state;
}

/* An order's node and second word, each a signed word. */
u32 orderNode(App* app, u32 car) { return u32(i32(at(app, at(app, car + kCarOrder))) >> 16); }
u32 orderWord(App* app, u32 car) { return u32(i32(at(app, at(app, car + kCarOrder) + 2)) >> 16); }

/* 0x4c9f50 (simc's): cop `index`'s siren off, eax as it leaves it; false when the game stopped. */
bool sirenOff(App* app, ThreadState& ts, u32 index)
{
    return leftInEax(ts, feCore<0x4c9f50, frontend::simc::sirenOff>(app, ts, index));
}

/* __CHP: the value to an integer, towards 0, as a whole value; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<Float> chop(App* app, ThreadState& ts, const Float& value)
{
    return Float(guest::direct<kChop, watcom::clib::chop>(app, ts, double(value)));
}


/* 0x4965b0 (sima's): the distance from the place at `from` to that at `to`, eax as it leaves it;
 * nullopt when the game stopped. */
std::optional<Float> distance(App* app, ThreadState& ts, u32 from, u32 to)
{
    const std::optional<frontend::sima::FloatLeft> left =
        feCore<0x4965b0, frontend::sima::quickDistanceOf>(app, ts, from, to);
    if (!left)
        return std::nullopt;
    return Float(left->value);
}

/* Its size as the code asks it: twice, the second negated where the first was not over 0. */
std::optional<Float> distanceSize(App* app, ThreadState& ts, u32 from, u32 to)
{
    const std::optional<Float> first = distance(app, ts, from, to);
    if (!first)
        return std::nullopt;
    const bool positive = compareBelow(0.0, *first);
    const std::optional<Float> second = distance(app, ts, from, to);
    if (!second)
        return std::nullopt;
    return positive ? *second : -*second;
}

/* 0x4e0000 (vertices::offsetVectorList<false>): `count` vectors at `vector` less those at
 * `other`, to `result`; false when the game stopped. */
bool subtractVectors(App* app, ThreadState& ts, i32 count, u32 vector, u32 other, u32 result)
{
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, count, vector, other, result);
    return !ts.terminate;
}

/* 0x4e0000 by address, for pullOverSpot, which leaves the eax it leaves -- which its header does
 * not give: that eax, nullopt when the game stopped. */
std::optional<u32> subtractVectorsLeavingEax(App* app, ThreadState& ts, u32 count, u32 vector, u32 other,
                                               u32 result)
{
    const u32 left = guest::direct<0x4e0000, vertices::offsetVectorsEax<false>>(app, ts, count, vector, other, result);
    if (ts.terminate)
        return std::nullopt;
    return left;
}

/* 0x4e0050 (vertices::scaleVectorsBy): `count` vectors at `vector` scaled by `scale`, to `result`;
 * false when the game stopped. */
bool scaleVectors(App* app, ThreadState& ts, i32 count, u32 vector, float scale, u32 result)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, count, vector, scale, result);
    return !ts.terminate;
}

/* 0x4e01f0 (vertices::dotProduct): the dot product of the vectors at a and b; nullopt when the game
 * stopped. */
std::optional<Float> dot(App* app, ThreadState& ts, u32 a, u32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, ts, a, b);
    if (ts.terminate)
        return std::nullopt;
    return Float(product);
}

/* 0x4205a0 (trackdata::snapPlace): the place at `place` snapped to the track (`followed`, about
 * node `node`, as the code has them in edx and ebx), its place on the track left at `onTrack`;
 * false when the game stopped. */
bool snapToTrack(App* app, ThreadState& ts, u32 place, u32 followed, u32 node, u32 onTrack)
{
    return guest::direct<0x4205a0, trackdata::snapPlace>(app, ts, place, onTrack, followed, node);
}

/* The frame of a core that hands its locals' addresses to callees, kept on the guest's stack at
 * the original's depth: `pushed` bytes of registers below the return address's slot, then `bytes`
 * of locals.  frame - n is the original's ebp - n. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* 0x4dfde0: the matrix at `matrix` a turn about y by `angle` (its bits, in the word the code
 * pushes, handed by its address: ea::rotateAboutY hands the sine's function its frame under it);
 * false when the game stopped. */
bool yRotation(App* app, ThreadState& ts, u32 matrix, u32 angle)
{
    const GuestFrame pushed(ts, 0, 4);
    guest::put(app, pushed - 4, angle);
    return guest::direct<0x4dfde0, ea::rotateAboutY>(app, ts, matrix, pushed - 4).has_value();
}

/* A core of this file called as the game's code calls it: the return address's slot below esp,
 * where its frame on the guest's stack goes below. */
template <typename Core>
auto asCalled(ThreadState& ts, Core core)
{
    ts.sp -= 4;
    const auto result = core();
    ts.sp += 4;
    return result;
}

/* 0x405fd0 (aidrive::setReversing): car's driver's timer (`how`, `frames` its words); false when
 * the game stopped. */
bool setTimer(App* app, ThreadState& ts, u32 car, u32 how, u32 frames)
{
    guest::direct<0x405fd0, aidrive::setReversing>(app, ts, car, how, frames);
    return !ts.terminate;
}

/* setTimer by address, where a thunk leaves the eax it leaves, which its header does not
 * give. */
std::optional<u32> setTimerLeavingEax(App* app, ThreadState& ts, u32 car, u32 how, u32 frames)
{
    const u32 left = guest::direct<0x405fd0, aidrive::setReversing>(app, ts, car, how, frames);
    if (ts.terminate)
        return std::nullopt;
    return left;
}

/* 0x405220 (aidrive::jumpBehind): car's driver reset (`putBack` its mode); false when the game
 * stopped. */
bool resetDriver(App* app, ThreadState& ts, u32 car, bool putBack)
{
    return guest::direct<0x405220, aidrive::jumpBehind>(app, ts, car, putBack).has_value();
}


float floatOf(u32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

/* The race's random numbers: the seed times the multiplier to 0x554f14, its low 16 bits the next
 * seed; the product's bits 8..23, read back. */
u32 nextRandom(App* app)
{
    const u32 product = imul(at(app, kSeed), at(app, kMultiplier));
    put(app, kProduct, product);
    put(app, kSeed, product & 0xffff);
    return (at(app, kProduct) & 0xffff00) >> 8;
}

/* 0x405c80 (aidrive::humanNearSliceOf): whether node is taken (a human's car near it, or 0);
 * nullopt when the game stopped. */
std::optional<u32> nodeBlocked(App* app, ThreadState& ts, u32 node)
{
    return guest::direct<0x405c80, aidrive::humanNearSliceOf>(app, ts, node);
}

/* 0x40e160 (aiplan::nodeAhead): the node `count` on from car's; nullopt when the game stopped. */
std::optional<u32> nodeOn(App* app, ThreadState& ts, u32 car, u32 count)
{
    const u32 node = guest::direct<0x40e160, aiplan::nodeAhead>(app, ts, car, count);
    if (ts.terminate)
        return std::nullopt;
    return node;
}

/* 0x4e0640 (watcom's): memset of `count` bytes at `to` to `value`, eax as it leaves it (the
 * buffer); false when the game stopped. */
bool clearBytes(App* app, ThreadState& ts, u32 to, u32 value, u32 count)
{
    guest::direct<0x4e0640, watcom::clib::memset>(app, ts, to, value, count);
    return !ts.terminate;
}


/* 0x4e0280 (vertices::multiplyMatrix): the matrix at `to` the product of those at `a` and `b`;
 * false when the game stopped. */
bool multiply(App* app, ThreadState& ts, u32 a, u32 b, u32 to)
{
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, a, b, to);
    return !ts.terminate;
}

/* A node gap the track's way: negated where it runs backwards. */
i32 alongTrack(App* app, i32 gap) { return gameSettings(app).backwards != 0 ? i32(neg(u32(gap))) : gap; }

/* 0x40f050 (aiplan::carInReach): what is left of car's ram's wait (edx its asking), the eax the
 * code leaves: 1 where the car is within reach, else 0; nullopt when the game stopped. */
std::optional<u32> ramWait(App* app, ThreadState& ts, u32 car, u32 asking)
{
    const bool within = guest::direct<0x40f050, aiplan::carInReach>(app, ts, car, asking);
    if (ts.terminate)
        return std::nullopt;
    return within ? 1u : 0u;
}



/* 0x40ce70 (aiplan::brakeToStopDone): car brought to a stop where it is pulled over; false when
 * the game stopped. */
bool stopCar(App* app, ThreadState& ts, u32 car)
{
    return guest::direct<0x40ce70, aiplan::brakeToStopDone>(app, ts, car);
}

/* stopCar with the eax the code leaves (aiplan::stopPulledOverLeaving) put in eax. */
bool stopCarLeaving(App* app, ThreadState& ts, u32 car)
{
    const std::optional<u32> left = guest::direct<0x40ce70, aiplan::stopPulledOverLeaving>(app, ts, car);
    if (ts.terminate || !left)
        return false;
    return true;
}

/* 0x40dd60 (aiplan::beginPlanning), 0x40ddb0 (aiplan::pickLane): car steered at its target, its
 * lanes chosen; false when the game stopped. */
bool steerAtTarget(App* app, ThreadState& ts, u32 car)
{
    return guest::direct<0x40dd60, aiplan::beginPlanning>(app, ts, car);
}

bool chooseLanes(App* app, ThreadState& ts, u32 car)
{
    return guest::direct<0x40ddb0, aiplan::pickLane>(app, ts, car);
}

// The radio's message of a cop's level.
u32 radioMessage(App* app, u32 cop) { return at(app, at(app, cop + kCarRadioLevel) * 4 + kRadioMessages); }

}  // namespace

nfs::u32 nothingAtStart(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingAtLoad(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 releaseEdge(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setUpCops(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioOn(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioLevelDown(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioLevelUp(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copTopSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nearestCopGap(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setWantedSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 noteSpeeding(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pulledOver(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copRadioCall(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ticketRadio(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stopIfCopNear(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stopForCop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sizeUpTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 joinPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 leaveCopList(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 giveUpPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pullOverSpot(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 planPullOver(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 endPullOver(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copStopsBy(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stopOver(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 giveTicket(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 targetStopping(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 chaseSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pursueTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 votePursuitLane(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sideTowardNode(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keepSideOfTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 placeCopAtStart(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 placeCop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carReversing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copHitByCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 voteRoadLane(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 spotSpeeder(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 laneOffRoad(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 atOrderNode(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sideOfOrder(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 takeOrder(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 parkCop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drivePatrol(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copDriver(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 giveOrder(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 spreadCops(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioStopped(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioStop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 initPursuits(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 callRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 callTwoCars(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 roadblockSiren(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watchRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watchSecondCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copsWanted(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioJoined(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioGiveUp(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 releaseCops(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 dropCop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sendCopAhead(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 callBackup(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 orderTaken(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingForCop(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 classRule(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingAtEnd(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingAtEndToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setRammingRules(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 classValue(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 classValueToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rivalTick(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carContact(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 slowForClass(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingAtStartToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 catchUpSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findLeader(App* app, nfs::ThreadState& ts, nfs::Args& args);

namespace
{

/* push ebp; mov ebp, esp; pop ebp; ret: nothing seen but the return. */
nfs::u32 emptyHook(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* 0x401220, 0x401230, 0x404580, 0x404650, 0x404c40: empty hooks of the race's start and load and
 * of the cops' -- push ebp, mov ebp, esp, pop ebp: nothing but the return. */
nfs::u32 nothingAtStart(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

nfs::u32 nothingAtLoad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

/* 0x401240: whether car's input bit 0 (+0x620) was let go: so when it has gone off since it was
 * held (the watch back from held to free), not while it is on (the watch held); with it off and
 * nothing held, so when key 8 of the car's input ([+0]) is up (0x4be1a0 says 0).  nullopt when the
 * game stopped. */
std::optional<bool> wasReleased(App* app, ThreadState& ts, u32 car)
{
    const u32 watch = at(app, car + kCarWatch);
    if (watch == kWatchHeld)
    {
        if (byte(app, car + kCarInput) & 1)
            return false;
        put(app, car + kCarWatch, kWatchFree);
        return true;
    }
    if (watch != kWatchFree)
        put(app, car + kCarWatch, kWatchFree);
    if (byte(app, car + kCarInput) & 1)
    {
        put(app, car + kCarWatch, kWatchHeld);
        return false;
    }
    const std::optional<u32> key = keyState(app, ts, kReleaseKey, at(app, car));
    if (!key)
        return std::nullopt;
    return *key == 0;
}

/* 0x401240: wasReleased for car eax, 1 or 0 in eax. */
nfs::u32 releaseEdge(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> released = wasReleased(app, ts, args.eax);
    if (!released)
        return args.eax;
    args.eax = *released ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4012c0: the race's cops set up: each its index (+0x714) and its order (+0x7ac, 0x56ea10 +
 * index * 8), the fast ones (their driver's model +0x14 0x1f8) marked (+0x711 bit 1); the
 * stopping speed (0x56ea50) 11.18.  The index the count stopped at: the count, 0 for none. */
u32 numberCops(App* app)
{
    u32 i = 0;
    for (; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 cop = at(app, i * 4 + kCops);
        put(app, cop + kCarCopIndex, i);
        const u32 driver = at(app, cop + kCarDriver);
        put(app, cop + kCarOrder, kOrders + i * 8);
        if (at(app, driver + kDriverModel) == kFastCopModel)
            putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopFast);
    }
    put(app, kStoppingSpeed, kStoppingSpeedBits);
    return i;
}

/* 0x4012c0: numberCops, the count it stopped at in eax. */
nfs::u32 setUpCops(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = numberCops(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x401320: the cops' radio on (0x56ea54). */
nfs::u32 radioOn(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    put(app, kRadioOn, 1);
    ts.sp += 4;
    return args.eax;
}

/* 0x401330, 0x401350: cop's radio level (+0x71c) one down, to 0; one up, to 8. */
void lowerRadioLevel(App* app, u32 cop)
{
    const u32 level = at(app, cop + kCarRadioLevel);
    if (i32(level) > 0)
        put(app, cop + kCarRadioLevel, level - 1);
}

void raiseRadioLevel(App* app, u32 cop)
{
    const u32 level = at(app, cop + kCarRadioLevel);
    if (level < u32(kTopRadioLevel))
        put(app, cop + kCarRadioLevel, level + 1);
}

/* 0x401330, 0x401350: for cop eax. */
nfs::u32 radioLevelDown(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    lowerRadioLevel(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 radioLevelUp(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    raiseRadioLevel(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x401370: cop's top speed: its handling's (+0x528's +0x1c8), scaled by 0x536044 for a fast
 * one. */
float copTopSpeedOf(App* app, u32 cop)
{
    const u32 handling = at(app, cop + kCarHandling);
    if (byte(app, cop + kCarCopKind) & kCopFast)
        return float(fp::mul(f32(app, handling + kHandlingTopSpeed), f64(app, kFastCopScale)));
    return floatAt(app, handling + kHandlingTopSpeed);
}

/* What the code of copTopSpeed leaves in eax: the handling for a fast cop, the speed's bits for
 * another. */
u32 copTopSpeedEax(App* app, u32 cop)
{
    const u32 handling = at(app, cop + kCarHandling);
    return byte(app, cop + kCarCopKind) & kCopFast ? handling : at(app, handling + kHandlingTopSpeed);
}

/* 0x401370: copTopSpeedOf for cop eax onto the x87's stack. */
nfs::u32 copTopSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const float top = copTopSpeedOf(app, cop);
    args.eax = copTopSpeedEax(app, cop);
    args.floatSet = true;
    args.floatResult = double(Float(top));
    ts.sp += 4;
    return args.eax;
}

/* 0x4013b0: the gap in nodes from car to the nearest cop, 10000 at most; nullopt when the game
 * stopped. */
std::optional<i32> nearestCopGapOf(App* app, ThreadState& ts, u32 car)
{
    i32 nearest = kFarGap;
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 cop = at(app, i * 4 + kCops);
        const std::optional<i32> gap = nodeGapSize(app, ts, at(app, cop + kCarNode), at(app, car + kCarNode));
        if (!gap)
            return std::nullopt;
        if (*gap < nearest)
            nearest = *gap;
    }
    return nearest;
}

/* 0x4013b0: nearestCopGapOf car eax, in eax. */
nfs::u32 nearestCopGap(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<i32> gap = nearestCopGapOf(app, ts, args.eax);
    if (!gap)
        return args.eax;
    args.eax = u32(*gap);
    ts.sp += 4;
    return args.eax;
}

/* The flips of car's wanted speed's sign (+0x7b3's top bit) the cops' code does after setting it:
 * once for the track run backwards, once for the car on the other side of the road (+0x621 bit
 * 0x10). */
void flipWantedSign(App* app, u32 car)
{
    if (gameSettings(app).backwards != 0)
        putByte(app, car + kCarWantedSign, byte(app, car + kCarWantedSign) ^ 0x80);
    if (byte(app, car + kCarSide) & kSideOther)
        putByte(app, car + kCarWantedSign, byte(app, car + kCarWantedSign) ^ 0x80);
}

/* The byte the first of those flips leaves in its byte register, from the sign byte after both. */
u8 firstFlip(App* app, u32 car)
{
    const u8 sign = byte(app, car + kCarWantedSign);
    return byte(app, car + kCarSide) & kSideOther ? u8(sign ^ 0x80) : sign;
}

/* 0x401430: cop's wanted speed (+0x7b0): its node's speed times `factor` plus `offset`, its top
 * speed at most (copTopSpeedOf), its sign flipped as flipWantedSign says.  Whether the top speed
 * capped it; nullopt when the game stopped. */
std::optional<bool> setWantedSpeedOf(App* app, ThreadState& ts, u32 cop, float factor, float offset)
{
    const std::optional<Float> speed = nodeSpeed(app, ts, at(app, cop + kCarNode));
    if (!speed)
        return std::nullopt;
    putFloat(app, cop + kCarWanted, float(fp::add(fp::mul(*speed, Float(factor)), Float(offset))));
    const float top = copTopSpeedOf(app, cop);
    const bool capped = compareAbove(f32(app, cop + kCarWanted), Float(top));
    if (capped)
        putFloat(app, cop + kCarWanted, top);
    flipWantedSign(app, cop);
    return capped;
}

/* Car's speeding (+0x6f0) kept at its worst: `speed` less its node's speed, if more.  False when
 * the game stopped. */
bool keepWorstSpeeding(App* app, ThreadState& ts, u32 car, float speed)
{
    const std::optional<Float> limit = nodeSpeed(app, ts, at(app, car + kCarNode));
    if (!limit)
        return false;
    const float over = float(fp::sub(Float(speed), *limit));
    const bool worse = !compareAbove(f32(app, car + kCarSpeeding), Float(over));
    put(app, car + kCarSpeeding, worse ? floatBits(over) : at(app, car + kCarSpeeding));
    return true;
}

/* 0x4014b0: car's speeding kept at its worst against its speed (+0x7c4); for no car nothing.
 * False when the game stopped. */
bool noteSpeedingOf(App* app, ThreadState& ts, u32 car)
{
    return car == 0 || keepWorstSpeeding(app, ts, car, floatAt(app, car + kCarSpeed));
}

/* 0x4014b0: noteSpeedingOf car eax; eax the speeding kept, 0 for no car. */
nfs::u32 noteSpeeding(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (!noteSpeedingOf(app, ts, car))
        return args.eax;
    args.eax = car != 0 ? at(app, car + kCarSpeeding) : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x401500: whether car's pursuit has it pulled over. */
bool isPulledOver(App* app, u32 car)
{
    return at(app, at(app, car + kCarPursuit) + kPursuitState) == kPulledOver;
}

/* isPulledOver as its code leaves it in eax (native_ai_race.h). */
u32 pulledOverOf(App* app, u32 car)
{
    return isPulledOver(app, car) ? 1 : 0;
}

/* 0x401500: isPulledOver car eax, 1 or 0 in eax. */
nfs::u32 pulledOver(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = pulledOverOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

// How a cop's call about a car went: no radio in the race, the car pulled over, too slow, the
// last call not past, called.
enum class RadioCall { NoRadio, PulledOver, TooSlow, TooSoon, Called };

/* 0x401520: cop's call on the radio about car (snd::carRadio, the message of its radio level),
 * where the race has the cops' radio (0x6fd4fc bit 0x10), the car is not pulled over and goes at
 * 0x53604c or more, and the cop's last call (+0x724) is past; the next 0x180 ticks on.  nullopt
 * when the game stopped. */
std::optional<RadioCall> callAboutCar(App* app, ThreadState& ts, u32 cop, u32 car)
{
    if (!(std::uint8_t(gameSettings(app).raceFlags) & kRaceCopRadio))
        return RadioCall::NoRadio;
    if (at(app, at(app, car + kCarPursuit) + kPursuitState) == kPulledOver)
        return RadioCall::PulledOver;
    if (compareBelow(f32(app, car + kCarSpeed), f64(app, kRadioSpeed)))
        return RadioCall::TooSlow;
    if (!(i32(at(app, kClock)) > i32(at(app, cop + kCarRadioDue))))
        return RadioCall::TooSoon;
    const port::Result called = guest::direct<0x4127c0, port::carRadioCalled>(app, ts, ts.sp - 4, cop);
    if (ts.terminate || !called)
        return std::nullopt;
    put(app, cop + kCarRadioDue, at(app, kClock) + kRadioCallTicks);
    return RadioCall::Called;
}

/* 0x401520: callAboutCar for cop eax and car edx; eax as the code leaves it on each way out. */
nfs::u32 copRadioCall(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax, car = args.edx;
    const std::optional<RadioCall> call = callAboutCar(app, ts, cop, car);
    if (!call)
        return args.eax;
    switch (*call)
    {
    case RadioCall::NoRadio:
        break;
    case RadioCall::PulledOver:
        args.eax = at(app, car + kCarPursuit);
        break;
    case RadioCall::TooSlow:
        args.eax = withStatus(at(app, car + kCarPursuit));
        break;
    case RadioCall::TooSoon:
        args.eax = at(app, kClock);
        break;
    case RadioCall::Called:
        args.eax = at(app, cop + kCarRadioDue);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x401580: cop's radio to the player of car about its ticket (snd::playerCopRadio): the message
 * of its radio level, the tickets the car has left ([0x6fd4bc] less +0x2c8).  False when the game
 * stopped. */
std::optional<u32> radioTicketLeft(App* app, ThreadState& ts, u32 cop, u32 car)
{
    const u32 left = gameSettings(app).pursuitOutCount - at(app, car + kCarTickets);
    const port::Result told =
        guest::direct<0x412880, port::playerCopRadioCalled>(app, ts, ts.sp - 4, cop, i32(left));
    if (ts.terminate || !told)
        return std::nullopt;
    return u32(*told);  // as the radio leaves it
}

bool radioTicket(App* app, ThreadState& ts, u32 cop, u32 car)
{
    return radioTicketLeft(app, ts, cop, car).has_value();
}

/* 0x401580: radioTicket for cop eax and car edx; eax as the radio leaves it. */
nfs::u32 ticketRadio(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> told = radioTicketLeft(app, ts, args.eax, args.edx);
    if (!told)
        return args.eax;
    args.eax = *told;
    ts.sp += 4;
    return args.eax;
}

/* The last of the cops on a pursuit a look along them came to: whether any, its gap. */
struct LastGap
{
    bool any = false;
    i32 gap = 0;
};

/* 0x4015b0: car pulled over (its pursuit's state 2, the stop's end 0x140 ticks on) when one of the
 * cops on it is less than 10 nodes away -- unless it is not chased or is marked +0x200 bit 0x20.
 * The last cop's gap looked at; nullopt when the game stopped. */
std::optional<LastGap> stopIfCopNearOf(App* app, ThreadState& ts, u32 car)
{
    LastGap last;
    const u32 pursuit = at(app, car + kCarPursuit);
    if (pursuit == 0 || at(app, pursuit + kPursuitState) == kNoPursuit || (byte(app, car + kCarFlags) & kFlagFinished))
        return last;
    for (u32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
    {
        const u32 cop = at(app, pursuit + i * 4 + kPursuitCops);
        const std::optional<i32> gap = nodeGapSize(app, ts, at(app, car + kCarNode), at(app, cop + kCarNode));
        if (!gap)
            return std::nullopt;
        last = {true, *gap};
        if (*gap < kStopGap)
        {
            const u32 now = at(app, kClock);
            put(app, pursuit + kPursuitState, kPulledOver);
            put(app, pursuit + kPursuitStopEnds, now + kStopTicks);
            break;
        }
    }
    return last;
}

/* 0x4015b0: stopIfCopNearOf car eax; eax as the code leaves it: the car where no cop was looked at,
 * the stop's end where one stopped it, else the last gap. */
nfs::u32 stopIfCopNear(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const std::optional<LastGap> last = stopIfCopNearOf(app, ts, car);
    if (!last)
        return args.eax;
    if (last->any)
        args.eax = last->gap < kStopGap ? at(app, at(app, car + kCarPursuit) + kPursuitStopEnds) : u32(last->gap);
    ts.sp += 4;
    return args.eax;
}

/* 0x403830: car (not a player's) given an order -- its kind, node, second word, side and flag --
 * from car `from` (+0x730), not yet taken. */
void giveOrderOf(App* app, u32 car, u32 from, u8 kind, u16 node, u16 word2, u8 side, u8 flag)
{
    const u32 order = at(app, car + kCarOrder);
    if (byte(app, car + kCarFlags) & kFlagPlayer)
        return;
    put(app, car + kCarOrderFrom, from);
    putByte(app, order + kOrderTaken, 0);
    putByte(app, order + kOrderKind, kind);
    putWord(app, order + kOrderNode, node);
    putWord(app, order + kOrderWord, word2);
    putByte(app, order + kOrderSide, side);
    putByte(app, order + kOrderFlag, flag);
}

/* 0x403830: giveOrderOf car eax from edx, the kind bl, node cx, the stack's word, side byte and
 * flag byte; ret 0xc.  eax the order. */
nfs::u32 giveOrder(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const u32 order = at(app, car + kCarOrder);
    const u16 word2 = u16(at(app, ts.sp + 4));
    const u8 side = byte(app, ts.sp + 8), flag = byte(app, ts.sp + 0xc);
    giveOrderOf(app, car, args.edx, nfs::u8(args.ebx), nfs::u16(args.ecx), word2, side, flag);
    args.eax = order;
    ts.sp += 4 + 0xc;
    return args.eax;
}

/* 0x404070: cop's word on the radio as it joins (0x40b220; 0x40b530 for a car that got away
 * before, from a cop not a player's), or as a second (0x40b330).  What the radio's code leaves in
 * eax; nullopt when the game stopped. */
std::optional<u32> radioJoinedOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 chase = at(app, cop + kCarPursuit);
    if (at(app, chase + kPursuitCopCount) != 1)
        return guest::direct<0x40b330, pursuit::radioPursuitOf>(app, ts, cop);
    if (at(app, chase + kPursuitGaveUp) != 0 && (byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return guest::direct<0x40b530, pursuit::radioBackupOf>(app, ts, cop);
    return guest::direct<0x40b220, pursuit::radioSpottedOf>(app, ts, cop);
}

/* 0x404070: radioJoinedOf cop eax; eax as the radio leaves it. */
nfs::u32 radioJoined(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> eax = radioJoinedOf(app, ts, args.eax);
    if (!eax)
        return args.eax;
    args.eax = *eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x4018b0: cop joins the pursuit of car (`kind`: 0 by its own sighting, 1 or 2 counted at the
 * pursuit's +8/+0xc, 3 for a player's cop): a cop that had an order lets it go (its siren,
 * 0x4c9f50, off where it was on, the roadblock's car of the order's pursuit cleared); the pursuit
 * started if it was not (state 1, its clock and radio's times); the cop marked as on it and added
 * to its list.  Kind 0 also keeps the car's speeding at its worst against `speed`; then the
 * radio's word (radioJoinedOf).  Whether it joined: not a cop on a pursuit already, and what the
 * radio's word left in eax where it was said; nullopt when the game stopped. */
struct Joining
{
    bool joined;
    u32 radioEax;
};

std::optional<Joining> joinPursuitWith(App* app, ThreadState& ts, u32 kind, u32 cop, u32 car, float speed)
{
    if (at(app, cop + kCarPursuit) != 0)
        return Joining{false, 0};
    put(app, cop + kCarJoin1, 0);
    const u8 flags = byte(app, cop + kCarFlags);
    put(app, cop + kCarJoin2, 0);
    if ((flags & kFlagPlayer) == 0 && at(app, cop + kCarOrderFrom) != 0)
    {
        if (byte(app, cop + kCarCopKind) & kCopSent)
            putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopSent);
        const u8 copKind = byte(app, cop + kCarCopKind);
        if (copKind & kCopRoadblock)
        {
            putByte(app, cop + kCarCopKind, copKind & ~kCopRoadblock);
            const u32 from = at(app, cop + kCarOrderFrom);
            const u32 fromPursuit = from != 0 ? at(app, from + kCarPursuit) : 0;
            if (fromPursuit != 0)
            {
                put(app, fromPursuit + kPursuitRoadblock, 0);
                const u32 index = at(app, cop + kCarCopIndex);
                if (at(app, index * kSirenSize + kSirens) != 0 && !sirenOff(app, ts, at(app, cop + kCarCopIndex)))
                    return std::nullopt;
            }
        }
    }
    const u32 pursuit = at(app, car + kCarPursuit);
    if (at(app, pursuit + kPursuitState) == kNoPursuit)
    {
        put(app, pursuit + kPursuitState, kChase);
        put(app, pursuit + kPursuitStopEnds, 0);
        put(app, pursuit + kPursuitCopCount, 0);
        put(app, pursuit + kPursuitStoppedBy, 0);
        put(app, pursuit + kPursuitCalls, 0);
        const u32 now = at(app, kClock);
        put(app, pursuit + kPursuitCaller, 0);
        put(app, pursuit + kPursuitStarted, now);
        put(app, pursuit + kPursuitRadioDue, now + kFirstRadioTicks);
    }
    put(app, cop + kCarOnPursuit, 1);
    put(app, cop + kCarPursuit, pursuit);
    put(app, cop + kCarChased, car);
    putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) | kCopChasing);
    const u32 count = at(app, pursuit + kPursuitCopCount);
    put(app, pursuit + count * 4 + kPursuitCops, cop);
    put(app, pursuit + kPursuitCopCount, at(app, pursuit + kPursuitCopCount) + 1);
    if (kind == kJoinSighted)
    {
        if (!keepWorstSpeeding(app, ts, car, speed))
            return std::nullopt;
    }
    else if (kind == kJoinPlayerCop)
        return Joining{true, 0};
    else
        put(app, pursuit + kind * 4 + 4, at(app, pursuit + kind * 4 + 4) + 1);
    const std::optional<u32> radio = radioJoinedOf(app, ts, cop);
    if (!radio)
        return std::nullopt;
    return Joining{true, *radio};
}

/* joinPursuitWith's whether it joined (native_ai_race.h). */
std::optional<bool> joinPursuitOf(App* app, ThreadState& ts, u32 kind, u32 cop, u32 car, float speed)
{
    const std::optional<Joining> joining = joinPursuitWith(app, ts, kind, cop, car, speed);
    if (!joining)
        return std::nullopt;
    return joining->joined;
}

/* joinPursuitWith and the eax the game's code leaves (native_ai_race.h). */
std::optional<u32> joinPursuitLeaving(App* app, ThreadState& ts, u32 kind, u32 cop, u32 car, float speed)
{
    const std::optional<Joining> joining = joinPursuitWith(app, ts, kind, cop, car, speed);
    if (!joining)
        return std::nullopt;
    if (!joining->joined)
        return kind;
    return kind == kJoinPlayerCop ? at(app, car + kCarPursuit) : joining->radioEax;
}

/* 0x4018b0: joinPursuitLeaving kind eax, cop edx, car ebx, the speed on the stack; ret 4. */
nfs::u32 joinPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left =
        joinPursuitLeaving(app, ts, args.eax, args.edx, args.ebx, floatAt(app, ts.sp + 4));
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4 + 4;
    return args.eax;
}

// How a racer's stop for a cop went: nothing more, the cop chasing already, the cop joined, the
// player's cop too far.
enum class StopWay { Nothing, Chasing, Joined, TooFar };

struct CopStop
{
    StopWay way;
    u32 kind = 0;       // the join's kind
    bool joined = false;  // joinPursuitWith's
    i32 gap = 0;          // the player's cop's
    u32 radioEax = 0;   // joinPursuitWith's: eax as the radio leaves it
};

/* 0x401650: racer stops for cop `index`: the cop's roadblock mark cleared; the racer's pursuit
 * pulled over, unless the racer is out (+0x2c4 3 or 4); a cop chasing no one joins (joinPursuitOf,
 * kind 0 -- for a player's cop kind 3 and only within 20 nodes) where the racer is speeding or
 * chased; the stop's end 0x140 ticks on if any cop is on it.  nullopt when the game stopped. */
std::optional<CopStop> stopForCopOf(App* app, ThreadState& ts, u32 racer, u32 index)
{
    const u32 cop = at(app, index * 4 + kCops);
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopRoadblock);
    if ((byte(app, racer + kCarFlags) & kFlagRacer) == 0)
        return CopStop{StopWay::Nothing};
    const u32 pursuit = at(app, racer + kCarPursuit);
    put(app, pursuit + kPursuitState, kPulledOver);
    if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfRace)
        return CopStop{StopWay::Nothing};
    if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfPursuits)
        return CopStop{StopWay::Nothing};
    const bool wanted = i32(at(app, racer + kCarSpeeding)) > i32(kSpeedingBitsWanted)
                        || at(app, pursuit + kPursuitCopCount) != 0;
    const auto stopEnds = [&] {
        if (at(app, pursuit + kPursuitCopCount) != 0)
            put(app, pursuit + kPursuitStopEnds, at(app, kClock) + kStopTicks);
    };
    if ((byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
    {
        if (!wanted && at(app, pursuit + kPursuitGaveUp) == 0)
            return CopStop{StopWay::Nothing};
        CopStop stop{StopWay::Chasing};
        if (at(app, cop + kCarChased) == 0)
        {
            const std::optional<Joining> joined = joinPursuitWith(app, ts, kJoinSighted, cop, racer, 0.0f);
            if (!joined)
                return std::nullopt;
            stop = {StopWay::Joined, kJoinSighted, joined->joined, 0, joined->radioEax};
        }
        stopEnds();
        return stop;
    }
    if (!wanted && at(app, pursuit + kPursuitGaveUp) == 0)
        return CopStop{StopWay::Nothing};
    if (at(app, cop + kCarChased) != 0)
        return CopStop{StopWay::Nothing};
    const std::optional<i32> gap = nodeGapSize(app, ts, at(app, racer + kCarNode), at(app, cop + kCarNode));
    if (!gap)
        return std::nullopt;
    if (*gap >= kPlayerCopReach)
        return CopStop{StopWay::TooFar, 0, false, *gap};
    const std::optional<Joining> joined = joinPursuitWith(app, ts, kJoinPlayerCop, cop, racer, 0.0f);
    if (!joined)
        return std::nullopt;
    stopEnds();
    return CopStop{StopWay::Joined, kJoinPlayerCop, joined->joined, 0, joined->radioEax};
}

/* stopForCopOf and the eax its code leaves on each way out: the racer as it came (nothing more),
 * the gap (too far), the cop's chased car (chasing), the kind (not joined), the pursuit (a
 * player's cop joined) or as the radio leaves it -- the stop's end where a cop is on it
 * (native_ai_race.h). */
std::optional<u32> stopForCopLeaving(App* app, ThreadState& ts, u32 racer, u32 index)
{
    const u32 cop = at(app, index * 4 + kCops);
    const std::optional<CopStop> stop = stopForCopOf(app, ts, racer, index);
    if (!stop)
        return std::nullopt;
    u32 eax = racer;
    switch (stop->way)
    {
    case StopWay::Nothing:
        break;
    case StopWay::TooFar:
        eax = u32(stop->gap);
        break;
    case StopWay::Chasing:
        eax = at(app, cop + kCarChased);
        break;
    case StopWay::Joined:
        if (!stop->joined)
            eax = stop->kind;
        else if (stop->kind == kJoinPlayerCop)
            eax = at(app, racer + kCarPursuit);
        else
            eax = stop->radioEax;
        break;
    }
    const u32 pursuit = at(app, racer + kCarPursuit);
    if ((stop->way == StopWay::Chasing || stop->way == StopWay::Joined) && at(app, pursuit + kPursuitCopCount) != 0)
        eax = at(app, pursuit + kPursuitStopEnds);
    return eax;
}

/* 0x401650: stopForCopLeaving racer eax, cop index edx. */
nfs::u32 stopForCop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = stopForCopLeaving(app, ts, args.eax, args.edx);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* stopForCopOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool stopForCopDone(App* app, ThreadState& ts, u32 racer, u32 index)
{
    return stopForCopOf(app, ts, racer, index).has_value();
}

/* 0x401780: cop sizes up the car it chases, for this tick's decisions: the car turning one way
 * (0x56ea58, its +0x4f4 over 0), on its own side of the road (0x56ea5c: a player's by +0x5f4,
 * another by +0x621 bit 0x10, against the track's way), the gap to it in nodes, signed by the
 * cop's side (0x56ea60), whether it is behind (0x56ea68), its lane against the cop's (0x56ea64: 0
 * left of it, 1 the same, 2 right).  Whether it chases one; nullopt when the game stopped. */
std::optional<bool> sizeUpTargetOf(App* app, ThreadState& ts, u32 cop)
{
    u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    put(app, kTurning, compareBelow(0.0, f32(app, target + kCarAhead)) ? 1 : 0);
    target = at(app, cop + kCarChased);
    u32 side = byte(app, target + kCarFlags) & kFlagPlayer ? (at(app, target + kCarPlayerSide) == 0 ? 1 : 0)
                                                             : ((byte(app, target + kCarSide) & kSideOther) == 0 ? 1 : 0);
    side ^= gameSettings(app).backwards;
    put(app, kSide, side != 0 ? 1 : 0);
    const std::optional<i32> gap = nodeGap(app, ts, at(app, at(app, cop + kCarChased) + kCarNode), at(app, cop + kCarNode));
    if (!gap)
        return std::nullopt;
    const u8 copSide = byte(app, cop + kCarSide);
    put(app, kGap, u32(*gap));
    const u32 way = (((copSide & kSideOther) == 0 ? 1u : 0u) ^ gameSettings(app).backwards) != 0 ? 1 : 0xffffffff;
    const u32 signedGap = imul(at(app, kGap), way);
    put(app, kGap, signedGap);
    put(app, kBehind, i32(signedGap) < 0 ? 1 : 0);
    const i32 ownLane = i32(at(app, cop + kCarLane));
    const i32 targetLane = i32(at(app, at(app, cop + kCarChased) + kCarLane));
    put(app, kLaneVote, targetLane < ownLane ? kLaneLeft : targetLane == ownLane ? kLaneSame : kLaneRight);
    return true;
}

/* 0x401780: sizeUpTargetOf cop eax; eax the target where there is one. */
nfs::u32 sizeUpTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<bool> chasing = sizeUpTargetOf(app, ts, cop);
    if (!chasing)
        return args.eax;
    if (*chasing)
        args.eax = at(app, cop + kCarChased);
    ts.sp += 4;
    return args.eax;
}

/* 0x401a20: cop taken off its pursuit's list: those after it moved down one, the count one less
 * (whether it was there or not), the last slot cleared. */
void takeOffList(App* app, u32 cop)
{
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit == 0)
        return;
    u32 found = kNotOnList, count = 0;
    for (u32 i = 0;; ++i)
    {
        count = at(app, pursuit + kPursuitCopCount);
        if (i32(i) >= i32(count))
            break;
        const u32 slot = pursuit + i * 4 + kPursuitCops;
        if (found != kNotOnList)
            put(app, slot - 4, at(app, slot));
        else if (cop == at(app, slot))
            found = i;
    }
    put(app, pursuit + kPursuitCopCount, count - 1);
    put(app, pursuit + (count - 1) * 4 + kPursuitCops, 0);
}

/* 0x401a20: takeOffList cop eax; eax its pursuit. */
nfs::u32 leaveCopList(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    takeOffList(app, cop);
    args.eax = at(app, cop + kCarPursuit);
    ts.sp += 4;
    return args.eax;
}

/* 0x4040b0: cop's word on the radio as it gives up (0x40b480) and its order (kind 2 at its node,
 * 6, by side 1).  False when the game stopped. */
bool radioGiveUpOf(App* app, ThreadState& ts, u32 cop)
{
    if (!guest::direct<0x40b480, pursuit::radioLostOf>(app, ts, cop))
        return false;
    giveOrderOf(app, cop, 0, kOrderAtNode, u16(at(app, cop + kCarNode)), 6, 1, 0);
    return true;
}

/* 0x4040b0: radioGiveUpOf cop eax; eax its order. */
nfs::u32 radioGiveUp(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    if (!radioGiveUpOf(app, ts, cop))
        return args.eax;
    args.eax = at(app, cop + kCarOrder);
    ts.sp += 4;
    return args.eax;
}

/* 0x401a80: cop gives up its pursuit (not one that has its target pulled over, unless the cop is
 * a player's): its word on the radio and its order (radioGiveUpOf), off the list (takeOffList); the
 * last cop off a chase the car gets away from -- unless +0x621 bit 0x40 -- counts the getaway (+0,
 * +0x14, the state back to 0) and the car's worst speeding (+4, chopped); the cop's pursuit
 * cleared, its radio level up.  False when the game stopped. */
bool giveUpPursuitOf(App* app, ThreadState& ts, u32 cop)
{
    if (at(app, cop + kCarChased) == 0)
        return true;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit == 0)
        return true;
    if (at(app, pursuit + kPursuitState) == kPulledOver && (byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return true;
    if (!radioGiveUpOf(app, ts, cop))
        return false;
    takeOffList(app, cop);
    if (at(app, pursuit + kPursuitCopCount) == 0 && (byte(app, at(app, cop + kCarChased) + kCarSide) & kSideNoGetaway) == 0)
    {
        const u32 getaways = at(app, pursuit + kPursuitGetaways);
        const u32 gaveUp = at(app, pursuit + kPursuitGaveUp);
        put(app, pursuit + kPursuitState, kNoPursuit);
        put(app, pursuit + kPursuitGetaways, getaways + 1);
        put(app, pursuit + kPursuitGaveUp, gaveUp + 1);
        const std::optional<Float> worst = chop(app, ts, f32(app, at(app, cop + kCarChased) + kCarSpeeding));
        if (!worst)
            return false;
        const i32 speeding = fp::toInteger<s32>(*worst);
        const i32 kept = i32(at(app, pursuit + kPursuitWorst));
        put(app, pursuit + kPursuitWorst, u32(kept > speeding ? kept : speeding));
    }
    put(app, cop + kCarPursuit, 0);
    put(app, cop + kCarChased, 0);
    const u8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarOnPursuit, 0);
    putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
    raiseRadioLevel(app, cop);
    return true;
}

/* 0x401a80: giveUpPursuitOf cop eax. */
nfs::u32 giveUpPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    if (!giveUpPursuitOf(app, ts, cop))
        return args.eax;
    args.eax = cop;
    ts.sp += 4;
    return args.eax;
}

/* 0x401e20: car's pull-over spot off. */
void endPullOverOf(App* app, u32 car)
{
    put(app, car + kCarSpotSet, 0);
    put(app, car + kCarSpotOn, 0);
}

/* 0x401e20: for car eax. */
nfs::u32 endPullOver(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    endPullOverOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4042b0: a player's cop drops off its pursuit (takeOffList) and is cleared of it.  Whether it
 * did: a player's. */
bool dropCopOf(App* app, u32 cop)
{
    if ((byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return false;
    takeOffList(app, cop);
    put(app, cop + kCarPursuit, 0);
    put(app, cop + kCarChased, 0);
    put(app, cop + kCarOrderFrom, 0);
    const u8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarOnPursuit, 0);
    putByte(app, cop + kCarCopFlags, copFlags & kCopFlagsDropped);
    return true;
}

/* 0x4042b0: dropCopOf cop eax; eax the pursuit it was on, where it dropped. */
nfs::u32 dropCop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (dropCopOf(app, cop))
        args.eax = pursuit;
    ts.sp += 4;
    return args.eax;
}

/* 0x401b50: the spot car pulls over to, beside it on the side `left` says (0.2 of a turn's quarter
 * and 2 to the right, or the same to the left): its heading (+0x210, 1/1024 of a turn) turned by
 * that into the orientation at +0x8ec (ea::yRotation), the offset along it from its place to
 * +0x910, less its height (+0x118) at +0x914.  The offset's vector on the guest's stack, handed to
 * the vectors' functions.  The eax the vectors' subtraction leaves (its thunk's); nullopt when
 * the game stopped. */
std::optional<u32> pullOverSpotOf(App* app, ThreadState& ts, u32 car, bool left)
{
    const GuestFrame frame(ts, 0x14, 0x24);
    const u32 offset = frame - 0x24;
    const i32 heading = i32(at(app, car + kCarHeading));
    const Float angle = heading > 0 ? fp::mul(Float(heading), f64(app, kHeadingScale))
                                    : fp::add(fp::mul(Float(heading + i32(kHalfTurn)), f64(app, kHeadingScaleLow)),
                                              f64(app, kHeadingStartLow));
    // Through a double, as the code stores it (exact), then a float.
    const float heading32 = float(double(angle));
    const u32 turn = left ? kSpotTurnLeftBits : kSpotTurnBits;
    const u32 along = left ? kSpotOffsetLeftBits : kSpotOffsetBits;
    const float spotTurn = float(fp::add(Float(heading32), Float(floatOf(turn))));
    if (!yRotation(app, ts, car + kCarSpotTurn, floatBits(spotTurn)))
        return std::nullopt;
    if (!scaleVectors(app, ts, 1, car + kCarSpotSide, floatOf(along), offset))
        return std::nullopt;
    const std::optional<u32> eax =
        subtractVectorsLeavingEax(app, ts, 1, car + kCarPlace, offset, car + kCarSpotPlace);
    if (!eax)
        return std::nullopt;
    putFloat(app, car + kCarSpotHeight, float(fp::sub(f32(app, car + kCarSpotHeight), f32(app, car + kCarHeight))));
    return eax;
}

/* 0x401b50: pullOverSpotOf car eax, on the side edx (not 0 for the left); eax as the vectors'
 * subtraction leaves it. */
nfs::u32 pullOverSpot(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> eax = pullOverSpotOf(app, ts, args.eax, args.edx != 0);
    if (!eax)
        return args.eax;
    args.eax = *eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x401c20: where car pulls over: the identity (0x564498) to its +0x8ec; its node's normal, right
 * and place (the node's signed bytes and fixed point, scaled by 0x53606c); the side -- the road's
 * right of the car (its place against the node's by the normal) against the car's own facing
 * (+0x1a8 by the right) -- its spot there (pullOverSpotOf), snapped to the track
 * (trackdata::snapToTrack, which leaves the spot's place on the track in the frame); where that
 * place says the spot is no good (its +0x6c set, its +0x14 0, no polygon at +0x18, or the
 * polygon's kind -- its second byte's low six bits -- 0 or 14), the other side's.  The spot on
 * (+0x8e4, +0x8e8).  The vectors and places on the guest's stack, handed to the callees.  The
 * polygon of the first side's spot where it was kept, 0 for the other side's; nullopt when the
 * game stopped. */
std::optional<u32> planPullOverOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0x130);
    // The original's ebp - n, its ebp lowered by 0x82.
    const auto local = [&](u32 n) { return frame - 0x82 + n; };
    const u32 normal = local(0x42), toCar = local(0x4e), place = local(0x5a), right = local(0x66);
    const u32 onTrack = local(0) - 0x36, otherOnTrack = local(0) - 0xae;
    const u32 node = at(app, car + kCarNode);
    u32 side = 0;
    for (u32 i = 0; i < 9; ++i)
        put(app, car + kCarSpotTurn + i * 4, at(app, kIdentity + i * 4));
    const u32 record = at(app, kNodes) + node * kNodeSize;
    const Float scale = f64(app, kNodeScale);
    const struct
    {
        u32 from, to;
    } bytes[] = {{0x15, normal}, {0x16, normal + 4}, {0x17, normal + 8}, {0x11, right}, {0x12, right + 4}, {0x13, right + 8}};
    for (const auto& b : bytes)
        putFloat(app, b.to, float(fp::mul(Float(i32(u32(i32(at(app, record + b.from)) >> 24) << 9)), scale)));
    putFloat(app, place, float(fp::mul(Float(i32(at(app, record))), scale)));
    putFloat(app, place + 4, float(fp::mul(Float(i32(at(app, record + 4))), scale)));
    putFloat(app, place + 8, float(fp::mul(scale, Float(i32(at(app, record + 8))))));
    if (!subtractVectors(app, ts, 1, place, car + kCarPlace, toCar))
        return std::nullopt;
    const std::optional<Float> across = dot(app, ts, toCar, normal);
    if (!across)
        return std::nullopt;
    if (compareBelow(0.0, *across))
        side = 1;
    const std::optional<Float> facing = dot(app, ts, car + kCarFacing, right);
    if (!facing)
        return std::nullopt;
    if (compareAbove(0.0, *facing))
        side ^= 1;
    if (!asCalled(ts, [&] { return pullOverSpotOf(app, ts, car, side != 0).has_value(); }))
        return std::nullopt;
    if (!snapToTrack(app, ts, car + kCarSpotPlace, 1, node, onTrack))
        return std::nullopt;
    // The spot kept where its place on the track is still, on the road and on a polygon of a kind
    // not 0 nor 14.
    bool other = true;
    u32 kept = 0;
    if (at(app, onTrack + kPlaceStill) == 0 && at(app, onTrack + kPlaceOnRoad) != 0)
    {
        const u32 polygon = at(app, onTrack + kPlacePolygon);
        if (polygon != 0 && (byte(app, polygon + 1) & kPolygonKindMask) != 0)
        {
            other = (byte(app, polygon + 1) & kPolygonKindMask) == kPolygonNoStop;
            kept = other ? 0 : polygon;
        }
    }
    if (other)
    {
        side ^= 1;
        if (!asCalled(ts, [&] { return pullOverSpotOf(app, ts, car, side != 0).has_value(); }))
            return std::nullopt;
        if (!snapToTrack(app, ts, car + kCarSpotPlace, 1, node, otherOnTrack))
            return std::nullopt;
    }
    put(app, car + kCarSpotSet, 1);
    put(app, car + kCarSpotOn, 1);
    return kept;
}

/* 0x401c20: planPullOverOf car edx; eax the car. */
nfs::u32 planPullOver(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.edx;
    if (!planPullOverOf(app, ts, car))
        return args.eax;
    args.eax = car;
    ts.sp += 4;
    return args.eax;
}

/* 0x4040e0: the pursuit of cop's target over: the radio's word (0x40b670); every cop set on that
 * car let go (its order and roadblock, its siren off, the pursuit's roadblock cleared, a new order
 * of kind 2); every cop on the pursuit off it (a timer 0x405fd0, its radio level down) -- a
 * player's reset as a driver (0x405220) with its siren off, another given a new order -- and its
 * slot cleared.  False when the game stopped. */
bool releaseCopsOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (!guest::direct<0x40b670, pursuit::radioPatrolOf>(app, ts, cop))
        return false;
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 other = at(app, i * 4 + kCops);
        const u8 flags = byte(app, other + kCarFlags);
        putByte(app, other + 0x8a, 0);
        if (flags & kFlagPlayer)
            continue;
        const u32 index = at(app, other + kCarCopIndex);
        if (at(app, other + kCarOrderFrom) != at(app, cop + kCarChased))
            continue;
        const u8 copKind = byte(app, other + kCarCopKind);
        put(app, other + kCarOrderFrom, 0);
        if (copKind & kCopRoadblock)
        {
            putByte(app, other + kCarCopKind, copKind & ~kCopRoadblock);
            if (at(app, index * kSirenSize + kSirens) != 0 && !sirenOff(app, ts, index))
                return false;
            put(app, pursuit + kPursuitRoadblock, 0);
        }
        const u32 side = at(app, other + kCarCopIndex) & 1;
        putWord(app, other + kCarCopFlags, u16(word(app, other + kCarCopFlags) & 0xfbef));
        put(app, other + kCarOnPursuit, 0);
        giveOrderOf(app, other, 0, kOrderAtNode, 0, 0, u8(side), 0);
    }
    for (u32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
    {
        const u32 on = at(app, pursuit + i * 4 + kPursuitCops);
        if (on != 0)
        {
            put(app, on + kCarPursuit, 0);
            put(app, on + kCarChased, 0);
            put(app, on + kCarOrderFrom, 0);
            const u8 copFlags = byte(app, on + kCarCopFlags);
            put(app, on + kCarOnPursuit, 0);
            putByte(app, on + kCarCopFlags, copFlags & kCopFlagsDropped);
            if (!setTimer(app, ts, on, 1, 0))
                return false;
            lowerRadioLevel(app, on);
            if (byte(app, on + kCarFlags) & kFlagPlayer)
            {
                if (!resetDriver(app, ts, on, false) || !sirenOff(app, ts, at(app, on + kCarCopIndex)))
                    return false;
            }
            else
                giveOrderOf(app, on, 0, kOrderAtNode, 0, 0, u8(at(app, on + kCarCopIndex) & 1), 0);
        }
        put(app, pursuit + i * 4 + kPursuitCops, 0);
    }
    return true;
}

/* 0x4040e0: releaseCopsOf cop eax; eax its pursuit. */
nfs::u32 releaseCops(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (!releaseCopsOf(app, ts, cop))
        return args.eax;
    args.eax = pursuit;
    ts.sp += 4;
    return args.eax;
}

/* 0x401ff0: cop gives the car it stopped its ticket: the pursuit's count and worst speeding kept,
 * the cop's tally (+0x2cc, its time at the race's limit 0x6fd4b8) and fines (+0x2d0, the speeding
 * chopped), the car's tickets one more -- out of the race at [0x6fd4bc] (+0x2c4 4) -- its speeding
 * cleared, its next pursuit's wait (0x5521b4 by its tickets); both cars' marks of the stop
 * cleared, the spot off, the cops let go (releaseCopsOf) and the pursuit ended.  False when the
 * game stopped. */
bool giveTicketOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 car = at(app, cop + kCarChased);
    const u32 pursuit = at(app, cop + kCarPursuit);
    putByte(app, car + kCarSide, byte(app, car + kCarSide) & kSideKeep);
    const u32 getaways = at(app, pursuit + kPursuitGetaways);
    put(app, pursuit + kPursuitGaveUp, 0);
    put(app, pursuit + kPursuitGetaways, getaways + 1);
    const i32 kept = i32(at(app, pursuit + kPursuitWorst));
    const std::optional<Float> worst = chop(app, ts, f32(app, car + kCarSpeeding));
    if (!worst)
        return false;
    const i32 speeding = fp::toInteger<s32>(*worst);
    put(app, pursuit + kPursuitWorst, u32(kept > speeding ? kept : speeding));
    const u32 tally = at(app, cop + kCarTally) + 1;
    const u32 wins = gameSettings(app).pursuitWinCount;
    put(app, cop + kCarTally, tally);
    if (tally == wins)
        put(app, cop + kCarWonAt, at(app, kClock) - kWonTicks);
    const u32 tickets = at(app, car + kCarTickets);
    const std::optional<Float> fine = chop(app, ts, f32(app, car + kCarSpeeding));
    if (!fine)
        return false;
    put(app, car + kCarTickets, tickets + 1);
    put(app, cop + kCarFines, at(app, cop + kCarFines) + u32(fp::toInteger<s32>(*fine)));
    const u32 out = gameSettings(app).pursuitOutCount;
    const u32 now = at(app, car + kCarTickets);
    put(app, car + kCarSpeeding, 0);
    if (!(i32(now) < i32(out)))
        put(app, car + kCarState, kOutOfPursuits);
    const u32 waitAt = at(app, car + kCarTickets) << 4;
    const u32 clock = at(app, kClock);
    const u32 wait = at(app, waitAt + kTicketWait);
    put(app, car + kCarStoppedBy, 0);
    put(app, car + kCarNextChase, clock + wait);
    put(app, cop + kCarStoppedBy, 0);
    putByte(app, car + 0x8a, 0);
    endPullOverOf(app, car);
    if (!releaseCopsOf(app, ts, cop))
        return false;
    put(app, pursuit + kPursuitCopCount, 0);
    put(app, pursuit + kPursuitStoppedBy, 0);
    put(app, pursuit + kPursuitState, kNoPursuit);
    return true;
}

/* 0x401ff0: giveTicketOf cop eax; eax its pursuit (releaseCops's). */
nfs::u32 giveTicket(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (!giveTicketOf(app, ts, cop))
        return args.eax;
    args.eax = pursuit;
    ts.sp += 4;
    return args.eax;
}

/* 0x401fb0: whether cop is done with a stop: so without a pursuit or a target, not while the stop
 * lasts (+0x28); at its end the ticket (giveTicketOf) and so.  nullopt when the game stopped. */
std::optional<bool> stopOverOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit != 0 && at(app, cop + kCarChased) != 0)
    {
        if (i32(at(app, kClock)) <= i32(at(app, pursuit + kPursuitStopEnds)))
            return false;
        if (!giveTicketOf(app, ts, cop))
            return std::nullopt;
    }
    return true;
}

/* 0x401fb0: stopOverOf cop eax, 1 or 0 in eax. */
nfs::u32 stopOver(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> over = stopOverOf(app, ts, args.eax);
    if (!over)
        return args.eax;
    args.eax = *over ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

// How a cop at a car pulled over went: dropped off, counted as one more stopped by, the one that
// stops by it.
enum class StopBy { Dropped, Counted, Stops };

/* 0x401e40: cop at a car pulled over: of several cops on a player's car the nearest (0x4965b0,
 * under 10000) stops by it, the others drop off (dropCopOf) -- all of them once one has; the one
 * that stops (+0x710 bit 0x80) is counted (+0x50) and the first sets the stop's end (0xc0 ticks
 * for a player's cop, else the car's tickets' wait 0x5521b8), radios a player's car its ticket
 * (radioTicket), plans its spot (planPullOverOf), and the two cars are marked as stopped by each
 * other (+0x9a4).  nullopt when the game stopped. */
std::optional<StopBy> copStopsByOf(App* app, ThreadState& ts, u32 cop)
{
    // The original's frame, below which planPullOverOf's goes.
    const GuestFrame frame(ts, 0x18, 0x14);
    const u32 pursuit = at(app, cop + kCarPursuit);
    u32 stopper = cop;
    const u32 count = at(app, pursuit + kPursuitCopCount);
    const u32 car = at(app, cop + kCarChased);
    if (i32(count) > 1 && (byte(app, cop + kCarFlags) & kFlagPlayer))
    {
        float nearest = floatOf(kFarBits);
        if (at(app, pursuit + kPursuitStoppedBy) != 0)
            stopper = 0;
        else
            for (u32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
            {
                const u32 other = at(app, pursuit + i * 4 + kPursuitCops);
                const std::optional<Float> apart = distanceSize(app, ts, other + kCarPlace, car + kCarPlace);
                if (!apart)
                    return std::nullopt;
                const float d = float(*apart);
                if (compareBelow(Float(d), Float(nearest)))
                {
                    nearest = d;
                    stopper = at(app, pursuit + i * 4 + kPursuitCops);
                }
            }
    }
    if (cop != stopper)
    {
        dropCopOf(app, cop);
        return StopBy::Dropped;
    }
    const u8 copFlags = byte(app, stopper + kCarCopFlags) | kCopStopping;
    putByte(app, stopper + kCarCopFlags, copFlags);
    putByte(app, stopper + kCarCopFlags, copFlags & ~kCopChasing);
    const u32 stoppedBy = at(app, pursuit + kPursuitStoppedBy) + 1;
    put(app, pursuit + kPursuitStoppedBy, stoppedBy);
    if (stoppedBy != 1)
        return StopBy::Counted;
    const u32 ends = byte(app, stopper + kCarFlags) & kFlagPlayer
                           ? at(app, kClock) + kPlayerCopStopTicks
                           : at(app, kClock) + at(app, (at(app, car + kCarTickets) << 4) + kTicketStop);
    put(app, pursuit + kPursuitStopEnds, ends);
    if ((byte(app, car + kCarFlags) & kFlagPlayer) && !radioTicket(app, ts, stopper, car))
        return std::nullopt;
    if (!asCalled(ts, [&] { return planPullOverOf(app, ts, car).has_value(); }))
        return std::nullopt;
    put(app, car + kCarStoppedBy, stopper);
    put(app, stopper + kCarStoppedBy, car);
    return StopBy::Stops;
}

// The ways chaseSpeedOf goes, for what its code leaves in eax.
enum class Chase { NoTarget, ToldBehind, ToldAhead, BehindFast, BehindNear, BehindTimer, AheadTop, AheadMatch };

/* 0x402420: cop's wanted speed in a chase: a cop told to (+0x7d4 1) keeps by the gap -- behind its
 * target, the gap times 0x53608c; else the target's speed (+0x7c0) times one and the gap by
 * 0x536094; otherwise behind it, the target's speed times 0x53609c for the leader's car (0x56ef54)
 * or 0x5360a4 for another, and, slower than 0x5360ac and more than 7 nodes off, a timer (0x405fd0)
 * and -9.83; ahead of it, its top speed, or within 8 nodes and faster than the target the target's
 * speed and 0x5360bc and the difference by the gap.  The sign flipped as flipWantedSign says.  The
 * way it went; nullopt when the game stopped. */
std::optional<Chase> chaseSpeedOf(App* app, ThreadState& ts, u32 cop, u32& timerEax)
{
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return Chase::NoTarget;
    Chase way;
    if (at(app, cop + kCarToldSpeed) == 1)
    {
        const Float byGap = Float(i32(at(app, kGap)));
        if (at(app, kBehind) != 0)
        {
            putFloat(app, cop + kCarWanted, float(fp::mul(byGap, f64(app, kToldBehind))));
            way = Chase::ToldBehind;
        }
        else
        {
            const Float share = fp::mul(byGap, f64(app, kToldAhead));
            put(app, cop + kCarWanted, at(app, target + kCarSignedSpeed));
            putFloat(app, cop + kCarWanted, float(fp::mul(fp::add(share, 1.0), f32(app, cop + kCarWanted))));
            way = Chase::ToldAhead;
        }
    }
    else if (at(app, kBehind) != 0)
    {
        const u32 share = target == at(app, kChaseLeader) ? kLeaderShare : kOtherShare;
        putFloat(app, cop + kCarWanted, float(fp::mul(f32(app, target + kCarSpeed), f64(app, share))));
        way = Chase::BehindFast;
        if (compareBelow(f32(app, cop + kCarSpeed), f64(app, kTimerSpeed)))
        {
            const i32 gap = i32(at(app, kGap));
            way = Chase::BehindNear;
            if ((gap > 0 ? gap : i32(neg(u32(gap)))) > kTimerGap)
            {
                // By address: chaseSpeed leaves the eax it leaves.
                const std::optional<u32> left = setTimerLeavingEax(app, ts, cop, 0, 0);
                if (!left)
                    return std::nullopt;
                timerEax = *left;
                put(app, cop + kCarWanted, kBrakeSpeedBits);
                way = Chase::BehindTimer;
            }
        }
    }
    else
    {
        const float top = copTopSpeedOf(app, cop);
        const i32 gap = i32(at(app, kGap));
        bool keepTop = gap > kTopGap;
        if (!keepTop)
            keepTop = compareAbove(f32(app, at(app, cop + kCarChased) + kCarSpeed), Float(top));
        if (keepTop)
        {
            putFloat(app, cop + kCarWanted, top);
            way = Chase::AheadTop;
        }
        else
        {
            const u32 chased = at(app, cop + kCarChased);
            const Float over = fp::mul(fp::mul(fp::sub(Float(top), f32(app, chased + kCarSpeed)), Float(gap)),
                                       f32(app, kAheadByGap));
            putFloat(app, cop + kCarWanted, float(over));
            putFloat(app, cop + kCarWanted,
                     float(fp::add(fp::add(f32(app, chased + kCarSpeed), f64(app, kAheadOver)), f32(app, cop + kCarWanted))));
            way = Chase::AheadMatch;
        }
    }
    flipWantedSign(app, cop);
    return way;
}

// The ways votePursuitLaneOf goes: no target, behind it, level with it, going the other way, far
// ahead, by the lanes.
enum class LaneVote { NoTarget, Behind, Level, Oncoming, Far, Lanes };

/* 0x402750: cop's votes for a lane (0x56eed8.., 0x56ef2c..) in a chase: slower than 0x5360cc and
 * not told otherwise, marked (+0x711 bit 2); behind its target, 10 for the target's lane; level
 * with it, the mark; going the other way than the target, 70 against its lane; far ahead (more
 * than 5 nodes) or the target pulled over, 10 for its lane; else, by the target's lane against the
 * cop's, 10 and 30 for its own and the next. */
LaneVote votePursuitLaneOf(App* app, ThreadState& ts, u32 cop)
{
    const auto vote = [&](u32 table, u32 amount) {
        const u32 at_ = at(app, kLaneVote) * 4 + table;
        put(app, at_, at(app, at_) + amount);
    };
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return LaneVote::NoTarget;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (compareBelow(f32(app, target + kCarSpeed), f64(app, kSlowTarget)) && at(app, cop + kCarToldSpeed) == 0)
        putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopHolds);
    if (at(app, kBehind) != 0)
    {
        vote(kLaneFor, 0xa);
        return LaneVote::Behind;
    }
    if (at(app, kGap) == 0)
    {
        putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopHolds);
        return LaneVote::Level;
    }
    const bool targetBack = compareAbove(0.0, f32(app, at(app, cop + kCarChased) + kCarSignedSpeed));
    const bool copBack = compareAbove(0.0, f32(app, cop + kCarSignedSpeed));
    if (copBack != targetBack)
    {
        vote(kLaneAgainst, 0x46);
        return LaneVote::Oncoming;
    }
    if (i32(at(app, kGap)) > kFarAhead || at(app, pursuit + kPursuitState) == kPulledOver)
    {
        vote(kLaneFor, 0xa);
        return LaneVote::Far;
    }
    const auto add = [&](u32 slot, u32 amount) { return at(app, kLaneFor + slot * 4) + amount; };
    switch (at(app, kLaneVote))
    {
    case kLaneLeft:
    {
        const u32 next = add(1, 0x1e), own = add(0, 0xa);
        put(app, kLaneFor + 4, next);
        put(app, kLaneFor, own);
        break;
    }
    case kLaneSame:
    {
        const u32 own = add(1, 0xa), right = add(2, 0x1e), left = add(0, 0x1e);
        put(app, kLaneFor + 4, own);
        put(app, kLaneFor + 8, right);
        put(app, kLaneFor, left);
        break;
    }
    case kLaneRight:
    {
        const u32 own = add(2, 0xa), next = add(1, 0x1e);
        put(app, kLaneFor + 8, own);
        put(app, kLaneFor + 4, next);
        break;
    }
    default:
        break;
    }
    return LaneVote::Lanes;
}

/* 0x402750: votePursuitLaneOf cop eax; eax as the code leaves it on each way out: the cop (with
 * the compare's status word level with the target), the vote's lane, the last count added to. */
nfs::u32 votePursuitLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    switch (votePursuitLaneOf(app, ts, cop))
    {
    case LaneVote::NoTarget:
        break;
    case LaneVote::Level:
        args.eax = withStatus(cop);
        break;
    case LaneVote::Behind:
    case LaneVote::Oncoming:
    case LaneVote::Far:
        args.eax = at(app, kLaneVote);
        break;
    case LaneVote::Lanes:
        args.eax = at(app, kLaneVote) == kLaneSame ? at(app, kLaneFor + 8) : at(app, kLaneVote);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

// The ways rivalTickOf goes: not a racer that rams, no rival near, not slower than it wants, its
// wait not over, the rival not a player's or not now, the chase's sound.
enum class Rivalry { NotRammer, NoRival, Fast, Waiting, NotNow, ChaseSound };

struct RivalOutcome
{
    Rivalry way;
    bool belowZero = false;  // 0 below its wanted speed (or a NaN), the first compare's
    u32 eax = 0;           // the eax the code leaves where its last callee does
};

/* 0x404840: racer's rivalry, each tick: between ticks 0x480 and 0x4c0, where the first car is
 * marked (+0x50d) in the last 3 nodes, it rams the first player's car for 50000 ticks; a racer
 * that rams counts its wait (+0x6e4) down, looks for its rival (0x40deb0) and, its rival near
 * (0x56ef3c, no more than one between, 0x56ef48) and itself slower by 0x536194 than it wants, its
 * wait over (0x40f050) -- its lane by the rival (0x40de50) unless +0x620 bit 8 -- against a
 * player's car now and then (the clock's 0xc0 bits) a cop chase's sound (snd::copChaseSet) of a
 * random kind of four.  nullopt when the game stopped. */
std::optional<RivalOutcome> rivalTickOf(App* app, ThreadState& ts, u32 racer)
{
    const i32 now = i32(at(app, kClock));
    if (now > 0x480 && now < 0x4c0)
    {
        const u32 first = at(app, kFirstCar);
        if (byte(app, first + kCarMarked) != 0)
        {
            const i32 node = i32(at(app, first + kCarNode));
            if (i32(at(app, kNodeCount) - 3) < node && node < i32(at(app, kNodeCount)))
            {
                const u32 player = at(app, kPlayers);
                put(app, racer + kCarRamFor, kRamTicks);
                put(app, racer + kCarChased, player);
            }
        }
    }
    if ((byte(app, racer + kCarFlags) & kFlagRams) == 0)
        return RivalOutcome{Rivalry::NotRammer};
    const u32 wait = at(app, racer + kCarRamFor);
    if (i32(wait) > 0)
        put(app, racer + kCarRamFor, wait - 1);
    // (aiplan::endHorn leaves the eax it was given: the racer.)
    guest::direct<0x40deb0, aiplan::endHorn>(app, ts, racer);
    if (ts.terminate)
        return std::nullopt;
    if (at(app, kRival) == 0 || i32(at(app, kRivalsBetween)) > 1)
        return RivalOutcome{Rivalry::NoRival, false, racer};
    const bool belowZero = compareBelow(0.0, f32(app, racer + kCarWanted));
    const u32 wanted = belowZero ? at(app, racer + kCarWanted) : floatBits(float(-f32(app, racer + kCarWanted)));
    if (!compareBelow(fp::add(f32(app, racer + kCarSpeed), f64(app, kRivalSlower)), Float(floatOf(wanted))))
        return RivalOutcome{Rivalry::Fast, belowZero, racer};
    const std::optional<u32> waiting = ramWait(app, ts, racer, 0);
    if (!waiting)
        return std::nullopt;
    if (*waiting != 0)
        return RivalOutcome{Rivalry::Waiting, false, *waiting};
    if ((byte(app, racer + kCarInput) & kInputNoLane) == 0)
    {
        guest::direct<0x40de50, aiplan::soundHorn>(app, ts, racer);
        if (ts.terminate)
            return std::nullopt;
    }
    if ((byte(app, at(app, kRival) + kCarFlags) & kFlagPlayer) == 0 || (byte(app, kClock) & 0xc0))
        return RivalOutcome{Rivalry::NotNow};
    // jmp [kind * 4 + 0x40482c]: 0x4049a6, 0x4049b7, 0x4049c8, 0x4049d9.
    const u32 kind = ((nextRandom(app) * 20) >> 16) & 3;
    static constexpr struct
    {
        u32 edx, ebx, ecx;
    } kSounds[] = {{3, 0x18, 8}, {3, 0x10, 5}, {2, 0x50, 8}, {1, 0x64, 8}};
    const u32 chaseEnd = guest::direct<0x411750, port::copChaseSetOf>(app, ts, racer, kSounds[kind].edx,
                                                                         i32(kSounds[kind].ebx), i32(kSounds[kind].ecx));
    if (ts.terminate)
        return std::nullopt;
    return RivalOutcome{Rivalry::ChaseSound, false, chaseEnd};
}

/* 0x404840: rivalTickOf racer eax.  eax as the code leaves it on each way out: not a racer that
 * rams, the car the window's test came to (worked out before: what it reads is written by nothing
 * before it); as the callee last called leaves it; slower not, its eax or the wanted speed's bits
 * with the compare's status word; the rival. */
nfs::u32 rivalTick(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 racer = args.eax;
    u32 window = racer;
    const i32 now = i32(at(app, kClock));
    if (now > 0x480 && now < 0x4c0)
    {
        window = at(app, kFirstCar);
        const i32 node = i32(at(app, window + kCarNode));
        if (byte(app, window + kCarMarked) != 0 && i32(at(app, kNodeCount) - 3) < node && node < i32(at(app, kNodeCount)))
            window = at(app, kPlayers);
    }
    const std::optional<RivalOutcome> rivalry = rivalTickOf(app, ts, racer);
    if (!rivalry)
        return args.eax;
    switch (rivalry->way)
    {
    case Rivalry::NotRammer:
        args.eax = window;
        break;
    case Rivalry::Fast:
        args.eax = withStatus(rivalry->belowZero ? at(app, racer + kCarWanted) : rivalry->eax);
        break;
    case Rivalry::NotNow:
        args.eax = at(app, kRival);
        break;
    case Rivalry::NoRival:
    case Rivalry::Waiting:
    case Rivalry::ChaseSound:
        args.eax = rivalry->eax;
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* rivalTickOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool rivalTickDone(App* app, ThreadState& ts, u32 racer)
{
    return rivalTickOf(app, ts, racer).has_value();
}

/* 0x404560: whether car's order is new (now taken) or was taken. */
bool takeNewOrder(App* app, u32 car)
{
    const u32 order = at(app, car + kCarOrder);
    if (byte(app, order + kOrderTaken) != 0)
        return false;
    putByte(app, order + kOrderTaken, 1);
    return true;
}

/* 0x404560: takeNewOrder car eax, 1 or 0 in eax. */
nfs::u32 orderTaken(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = takeNewOrder(app, args.eax) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 nothingForCop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

/* 0x404610: car's class's rule (0x552274 by its model's +0x30), 1 in place of 0 in the race's
 * modes 1 and 2 (0x6fd3b8). */
u32 classRuleOf(App* app, u32 car)
{
    const u32 kind = at(app, at(app, car + kCarDriver) + kModelClass);
    const u32 race = gameSettings(app).raceType;
    const u32 rule = at(app, kind * kClassSize + kClassRule);
    return (race == 1 || race == 2) && rule == 0 ? 1 : rule;
}

/* 0x404610: classRuleOf car eax, in eax. */
nfs::u32 classRule(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = classRuleOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 nothingAtEnd(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

/* 0x404660: nothingAtEnd called: nothing but the return. */
nfs::u32 nothingAtEndToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

/* 0x404690: racer's (not a player's) ramming rules, its record at 0x56ea70 + 0x14 by its place
 * (+0x1f0): 0.94, 0, and by its class's kind (0x55227c, the table at 0x40466c) the hits it takes
 * (0x6e/0x64/0x3c/0x14/15/0xa), the ticks it waits after them (0x40..0xc0) and its level (5, 4, 3,
 * 1), one less in the race's mode of 0x6fd4f0 1.  Its place read again for each record's word, as
 * the code does. */
void setRammingRulesOf(App* app, u32 racer)
{
    if (byte(app, racer + kCarFlags) & kFlagPlayer)
        return;
    const auto record = [&] { return at(app, racer + kCarPlaceInRace) * kRammingSize + kRamming; };
    put(app, record(), kRamGrip);
    put(app, record() + 4, 0);
    put(app, record() + 8, 0xf);
    put(app, record() + 0xc, 0x3c);
    const u32 kind = at(app, at(app, at(app, racer + kCarDriver) + kModelClass) * kClassSize + kClassKind);
    u32 hits = 0xf, wait = 0, level = 0;
    // jmp [kind * 4 + 0x40466c]: 0x404734, 0x404745, 0x404756, 0x404767, 0x404778, 0x404784.
    switch (kind > 5 ? 2 : kind)
    {
    case 0: wait = 0x40; level = 5; hits = 0x6e; break;
    case 1: wait = 0x60; level = 4; hits = 0x64; break;
    case 2: wait = 0x60; level = 3; hits = 0x3c; break;
    case 3: wait = 0x80; level = 1; hits = 0x14; break;
    case 4: wait = 0x80; level = 1; break;
    default: wait = 0xc0; level = 1; hits = 0xa; break;
    }
    put(app, record() + 8, hits);
    put(app, record() + 0xc, wait);
    if (gameSettings(app).seriesRace == 1)
        level -= 1;
    put(app, record() + 0x10, level);
}

/* 0x404690: setRammingRulesOf racer eax; eax, for one not a player's, its place times 5. */
nfs::u32 setRammingRules(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = setRammingRulesLeaving(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* setRammingRulesOf with the eax the code leaves: the racer, for a player's, else its place times 5. */
u32 setRammingRulesLeaving(App* app, u32 racer)
{
    setRammingRulesOf(app, racer);
    if ((byte(app, racer + kCarFlags) & kFlagPlayer) == 0)
        return at(app, racer + kCarPlaceInRace) * 5;
    return racer;
}

/* 0x4047f0, 0x404810: a value of the class of the model (+0x30): the table's at 0x55226c, at
 * 0x552280. */
u32 classValueOf(App* app, u32 model)
{
    return at(app, at(app, model + kModelClass) * kClassSize + kClassValue);
}

u32 classValueTooOf(App* app, u32 model)
{
    return at(app, at(app, model + kModelClass) * kClassSize + kClassValueToo);
}

/* stopIfCopNearOf for a caller that asks only whether the game went on (eax as the thunk leaves it: the car). */
bool carStoppedIfCopNear(App* app, ThreadState& ts, u32 car)
{
    return stopIfCopNearOf(app, ts, car).has_value();
}

/* 0x4047f0, 0x404810: for the model at eax, in eax. */
nfs::u32 classValue(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = classValueOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 classValueToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = classValueTooOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 1, whatever it is asked. */
u32 alwaysOneOf(App* app)
{
    (void)app;
    return 1;
}

/* 0x402c70: alwaysOneOf. */
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = alwaysOneOf(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x402c80: whether car goes backwards (+0x10c below 0) on the ground (+0x164 0): 0 over its
 * speed, so not for a NaN, as jbe after fcomp has it. */
bool isReversing(App* app, u32 car)
{
    return Float(0.0) > f32(app, car + kCarForward) && word(app, car + kCarAirborne) == 0;
}

/* 0x402c80: isReversing car eax, 1 or 0 in eax. */
nfs::u32 carReversing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = isReversing(app, args.eax) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4028e0: car onto the side of the road toward node (+0x621 bit 0x10 set where the node is
 * behind it).  The gap; nullopt when the game stopped. */
std::optional<i32> sideTowardNodeOf(App* app, ThreadState& ts, u32 car, u32 node)
{
    const std::optional<i32> gap = nodeGap(app, ts, node, at(app, car + kCarNode));
    if (!gap)
        return std::nullopt;
    if (*gap >= 0)
        putByte(app, car + kCarSide, byte(app, car + kCarSide) & ~kSideOther);
    else
        putByte(app, car + kCarSide, byte(app, car + kCarSide) | kSideOther);
    return gap;
}

/* 0x4028e0: sideTowardNodeOf car eax, node edx; eax the gap. */
nfs::u32 sideTowardNode(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<i32> gap = sideTowardNodeOf(app, ts, args.eax, args.edx);
    if (!gap)
        return args.eax;
    args.eax = u32(*gap);
    ts.sp += 4;
    return args.eax;
}

/* 0x403000: car onto the side of the road toward its order's node.  The gap; nullopt when the
 * game stopped. */
std::optional<i32> sideOfOrderOf(App* app, ThreadState& ts, u32 car)
{
    return sideTowardNodeOf(app, ts, car, orderNode(app, car));
}

/* 0x403000: sideOfOrderOf car eax; eax the gap. */
nfs::u32 sideOfOrder(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<i32> gap = sideOfOrderOf(app, ts, args.eax);
    if (!gap)
        return args.eax;
    args.eax = u32(*gap);
    ts.sp += 4;
    return args.eax;
}

/* 0x40f000: whether car at `node` is at node `at` with the second word `word2` (0x40f000); nullopt
 * when the game stopped. */
std::optional<u32> atNode(App* app, ThreadState& ts, u32 node, u32 target, u32 word2)
{
    const bool there = guest::direct<0x40f000, aiplan::nodeInRun>(app, ts, node, target, word2);
    if (ts.terminate)
        return std::nullopt;
    return there ? 1u : 0u;
}

/* 0x402fd0: whether car is at its order's node (0x40f000, the order's node and second word). */
std::optional<u32> atOrderNodeOf(App* app, ThreadState& ts, u32 car)
{
    const u32 word2 = orderWord(app, car);
    return atNode(app, ts, at(app, car + kCarNode), orderNode(app, car), word2);
}

/* 0x402fd0: atOrderNodeOf car eax, in eax. */
nfs::u32 atOrderNode(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> there = atOrderNodeOf(app, ts, args.eax);
    if (!there)
        return args.eax;
    args.eax = *there;
    ts.sp += 4;
    return args.eax;
}

/* 0x404b90: racer slowed for its class (its wanted speed by 0x5361a4): touched by a car (+0x16c)
 * -- in the race mode of 0x6fd4f0 1 hard enough (15, as bits) -- either the hit's force over
 * 0x53619c (chopped), or its class's rule 0 (classRuleOf) and now and then (the clock's bits 5..7
 * 0).  Whether it was; nullopt when the game stopped. */
std::optional<bool> slowForClassOf(App* app, ThreadState& ts, u32 car)
{
    i32 slowed = 0;
    if (at(app, car + kCarHit) == 0
        || (gameSettings(app).seriesRace == 1 && i32(at(app, car + kCarHitForce)) < i32(kHardHitBits)))
        return false;
    if (compareAbove(f32(app, car + kCarHitForce), f64(app, kHitForce)))
    {
        const std::optional<Float> force = chop(app, ts, f32(app, car + kCarHitForce));
        if (!force)
            return std::nullopt;
        slowed = fp::toInteger<s32>(*force);
    }
    if (classRuleOf(app, car) == 0)
        slowed |= ((i32(at(app, kClock)) >> 5) & 7) == 0 ? 1 : 0;
    if (slowed == 0)
        return false;
    putFloat(app, car + kCarWanted, float(fp::mul(f32(app, car + kCarWanted), f64(app, kClassSlowing))));
    return true;
}

/* 0x404b90: slowForClassOf racer eax, 1 or 0 in eax. */
nfs::u32 slowForClass(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> slowed = slowForClassOf(app, ts, args.eax);
    if (!slowed)
        return args.eax;
    args.eax = *slowed ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 nothingAtStartToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return emptyHook(app, ts, args);
}

/* 0x404c50: the catch-up speed (0x56ebb0): 20.12 while fewer players (0x6fd518) than 0x6fd520,
 * else 11.18. */
void setCatchUpSpeed(App* app)
{
    const u32 humans = gameSettings(app).humanCount;
    put(app, kCatchUpSpeed, i32(humans) < i32(gameSettings(app).opponentCount) ? kCatchUpFastBits : kCatchUpSlowBits);
}

/* 0x404c50: setCatchUpSpeed. */
nfs::u32 catchUpSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    setCatchUpSpeed(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x404c80: the leader among the players' cars (0x5efa48, [0x5efda4]), asked by the last of them
 * (+0x1f4): the one of the most laps (+8; a finished one's 1) to 0x56ebb8; then by the other
 * racers (0x5efb88, [0x5efd94]) how many have more laps -- 0x56ebb4 1 some, 2 all, 0 none. */
void findLeaderOf(App* app, u32 car)
{
    const u32 others = at(app, kOtherCount);
    i32 most = 0;
    if (at(app, kPlayerCount) - 1 == at(app, car + kCarPlayerIndex))
        for (u32 i = 0; i32(i) < i32(at(app, kPlayerCount)); ++i)
        {
            const u32 player = at(app, i * 4 + kPlayers);
            if (byte(app, player + kCarFlags) & kFlagFinished)
                put(app, player + kCarLaps, 1);
            const i32 laps = i32(at(app, player + kCarLaps));
            if (!(most > laps))
            {
                most = laps;
                put(app, kLeader, player);
            }
        }
    u32 behind = 0;
    if (others != 0)
    {
        u32 more = 0;
        for (u32 i = 0; i32(i) < i32(others); ++i)
            if (most < i32(at(app, at(app, i * 4 + kOthers) + kCarLaps)))
            {
                behind = 1;
                more += 1;
            }
        if (more == others)
            behind = 2;
    }
    put(app, kOtherCount, others);
    put(app, kLeaderBehind, behind);
}

/* 0x404c80: findLeaderOf car eax.  eax as the code leaves it (worked out before, as nothing it
 * reads for it is written): the others' count where there are any (0 for fewer than none), else
 * the last player looked at, else the car. */
nfs::u32 findLeader(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const u32 others = at(app, kOtherCount), players = at(app, kPlayerCount);
    u32 eax = car;
    if (others != 0)
        eax = i32(others) > 0 ? others : 0;
    else if (players - 1 == at(app, car + kCarPlayerIndex) && i32(players) > 0)
        eax = at(app, (players - 1) * 4 + kPlayers);
    findLeaderOf(app, car);
    args.eax = eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x403910: cop's word on the radio that its target has stopped (0x40bc30), once for the pursuit
 * (+0x54 bit 8).  False when the game stopped. */
bool radioStoppedOf(App* app, ThreadState& ts, u32 cop)
{
    if (byte(app, at(app, at(app, cop + kCarChased) + kCarPursuit) + kPursuitCalls) & kPursuitStoppedCall)
        return true;
    if (!guest::direct<0x40bc30, pursuit::radioHelpOf>(app, ts, cop))
        return false;
    const u32 pursuit = at(app, at(app, cop + kCarChased) + kCarPursuit);
    putByte(app, pursuit + kPursuitCalls, byte(app, pursuit + kPursuitCalls) | kPursuitStoppedCall);
    return true;
}

/* 0x403910: radioStoppedOf cop eax; eax its target's pursuit. */
nfs::u32 radioStopped(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    if (!radioStoppedOf(app, ts, cop))
        return args.eax;
    args.eax = at(app, at(app, cop + kCarChased) + kCarPursuit);
    ts.sp += 4;
    return args.eax;
}

/* 0x403950: cop's word on the radio that it stopped (0x40bba0), once (+0x711 bit 0x10).  What
 * the code leaves in eax: the radio's, else the cop; nullopt when the game stopped. */
std::optional<u32> radioStopOf(App* app, ThreadState& ts, u32 cop)
{
    if (byte(app, cop + kCarCopKind) & kCopToldStop)
        return cop;
    const std::optional<u32> eax = guest::direct<0x40bba0, pursuit::radioSpikesOf>(app, ts, cop);
    if (!eax)
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopToldStop);
    return eax;
}

/* 0x403950: radioStopOf cop eax; eax as the radio leaves it, else the cop. */
nfs::u32 radioStop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> eax = radioStopOf(app, ts, args.eax);
    if (!eax)
        return args.eax;
    args.eax = *eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x403870: the cops spread round the track at the race's start: each its order of kind 2 at a
 * node a share of the track on from the last, less a random quarter of it, the side by its
 * index. */
void spreadCopsOf(App* app)
{
    const i32 share = i32(at(app, kNodeCount)) / i32(at(app, kCopCount) + 1);
    u32 node = 0;
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 r = nextRandom(app);
        node += u32(share) - (imul(r, u32(share >> 2)) >> 16);
        giveOrderOf(app, at(app, i * 4 + kCops), 0, kOrderAtNode, u16(node), 5, u8(i & 1), 0);
    }
}

/* What spreadCops's code leaves in eax: the last cop's order, else the share. */
u32 spreadCopsEax(App* app)
{
    const i32 count = i32(at(app, kCopCount));
    return count > 0 ? at(app, at(app, u32(count - 1) * 4 + kCops) + kCarOrder)
                     : u32(i32(at(app, kNodeCount)) / (count + 1));
}

/* 0x403870: spreadCopsOf. */
nfs::u32 spreadCops(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    spreadCopsOf(app);
    args.eax = spreadCopsEax(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x403970: the race's pursuits cleared (0x56e410, 16 of 0x60 bytes) and each racer given its
 * own; the cops spread (spreadCopsOf).  False when the game stopped. */
bool initPursuitsOf(App* app, ThreadState& ts)
{
    if (!clearBytes(app, ts, kPursuits, 0, kPursuitsBytes))
        return false;
    for (u32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
        put(app, at(app, i * 4 + kRacers) + kCarPursuit, kPursuits + i * kPursuitSize);
    spreadCopsOf(app);
    return true;
}

/* 0x403970: initPursuitsOf; eax as spreadCops leaves it. */
nfs::u32 initPursuits(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!initPursuitsOf(app, ts))
        return args.eax;
    args.eax = spreadCopsEax(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x403270: car on patrol: steered at its target (0x40dd60), its wanted speed by `factor` and
 * `offset` (setWantedSpeedOf), clear of the others (0x40bff0), its lanes (0x40ddb0), steered
 * (0x40de30).  False when the game stopped. */
bool drivePatrolOf(App* app, ThreadState& ts, u32 car, float factor, float offset)
{
    if (!steerAtTarget(app, ts, car) || !setWantedSpeedOf(app, ts, car, factor, offset))
        return false;
    guest::direct<0x40bff0, pursuit::followChasedCar>(app, ts, car);
    if (ts.terminate)
        return false;
    return chooseLanes(app, ts, car) && feCore<0x40de30, frontend::sima::laneSteer>(app, ts, car).has_value();
}

/* 0x403270: drivePatrolOf car eax, the factor and the offset on the stack; ret 8.  eax as the
 * steering leaves it. */
nfs::u32 drivePatrol(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!drivePatrolOf(app, ts, args.eax, floatAt(app, ts.sp + 4), floatAt(app, ts.sp + 8)))
        return args.eax;
    ts.sp += 4 + 8;
    return args.eax;
}

/* 0x402a00: cop put at its order's node (the first of them free, 0x405c80, 20 nodes on each time,
 * 0x40e160): its sideways place (+0x7b4) a random share of the side's lanes (the node's +0x13)
 * and width (+0x1b right, +0x17 left, by 0x5360dc and 0x5360e4) -- the side the order (0x56f230,
 * 0x56f228) and the car's say -- plus 1; the patrol's speed (setWantedSpeedOf 1.19, 0) its speed
 * too, the chase off (0x40cd80), a random radio level 0..2.  False when the game stopped. */
bool placeCopOf(App* app, ThreadState& ts, u32 cop)
{
    u32 node = orderNode(app, cop);
    for (;;)
    {
        put(app, cop + kCarNode, node);
        const std::optional<u32> blocked = nodeBlocked(app, ts, node);
        if (!blocked)
            return false;
        if (*blocked == 0)
            break;
        const std::optional<u32> next = nodeOn(app, ts, cop, kNodeStride);
        if (!next)
            return false;
        node = *next;
    }
    if (at(app, kSideOrderSet) != 0)
        putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
    const u32 backwards = gameSettings(app).backwards;
    bool right = (((byte(app, cop + kCarSide) & kSideOther) == 0 ? 1u : 0u) ^ backwards) != 0 && at(app, kSideOrder) == 1;
    if (!right)
        right = (((byte(app, cop + kCarSide) & kSideOther) != 0 ? 1u : 0u) ^ backwards) != 0
                && at(app, kSideOrder) == 0xffffffff;
    const u32 record = at(app, kNodes) + node * kNodeSize;
    Float sideways;
    if (right)
    {
        const Float width = fp::mul(Float(i32(u32(byte(app, record + kNodeRightWidth)) << 15)), f64(app, kWidthScale));
        const u32 r = nextRandom(app);
        const u32 lanes = byte(app, record + kNodeLanes) & 0xf;
        const u32 share = (imul(lanes, r) >> 16) + 1;
        sideways = fp::sub(fp::mul(Float(i32(share)), width), fp::mul(width, f64(app, kLaneShare)));
    }
    else
    {
        const Float width = fp::mul(Float(i32(u32(byte(app, record + kNodeLeftWidth)) << 15)), f64(app, kWidthScale));
        const u32 r = nextRandom(app);
        const u32 lanes = (byte(app, record + kNodeLanes) >> 4) & 0xf;
        const u32 share = (imul(r, lanes) >> 16) + 1;
        sideways = fp::add(fp::mul(-width, Float(i32(share))), fp::mul(width, f64(app, kLaneShare)));
    }
    putFloat(app, cop + kCarSideways, float(sideways));
    put(app, cop + kCarLaneOffset, kOneBits);
    putFloat(app, cop + kCarSideways, float(fp::add(f32(app, cop + kCarLaneOffset), f32(app, cop + kCarSideways))));
    if (!setWantedSpeedOf(app, ts, cop, floatOf(kPatrolFactorBits), 0.0f))
        return false;
    put(app, cop + kCarSignedSpeed, at(app, cop + kCarWanted));
    const u32 speed = compareBelow(0.0, f32(app, cop + kCarSignedSpeed))
                            ? at(app, cop + kCarSignedSpeed)
                            : floatBits(float(-f32(app, cop + kCarSignedSpeed)));
    put(app, cop + kCarSpeed, speed);
    const u8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarChased, 0);
    putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
    put(app, cop + kCarOnPursuit, 0);
    if (!guest::direct<0x40cd80, aiplan::putOnTrack>(app, ts, cop))
        return false;
    put(app, cop + kCarRadioLevel, (nextRandom(app) * 3) >> 16);
    return true;
}

/* 0x402a00: placeCopOf cop eax; eax its radio level. */
nfs::u32 placeCop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    if (!placeCopOf(app, ts, cop))
        return args.eax;
    args.eax = at(app, cop + kCarRadioLevel);
    ts.sp += 4;
    return args.eax;
}

/* 0x4029c0: cop placed at the race's start: its side cleared, the last player's car of the race
 * (0x5efcc8, [0x5efd9c] of them) its watch (+0x6d8), then placeCopOf.  False when the game
 * stopped. */
bool placeCopAtStartOf(App* app, ThreadState& ts, u32 cop)
{
    putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
    for (u32 i = at(app, kRaceCarCount) - 1; i32(i) >= 0; --i)
    {
        const u32 car = at(app, i * 4 + kRaceCars);
        if (byte(app, car + kCarFlags) & kFlagPlayer)
        {
            put(app, cop + kCarWatched, car);
            break;
        }
    }
    return placeCopOf(app, ts, cop);
}

/* 0x4029c0: placeCopAtStartOf cop eax; eax as placeCop leaves it. */
nfs::u32 placeCopAtStart(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = placeCopAtStartLeaving(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* placeCopAtStartOf with the eax the code leaves; nullopt when the game stopped. */
std::optional<u32> placeCopAtStartLeaving(App* app, ThreadState& ts, u32 cop)
{
    if (!placeCopAtStartOf(app, ts, cop))
        return std::nullopt;
    return at(app, cop + kCarRadioLevel);
}

// How a cop's keeping to its target's side went: no target, the target slow, both the same way,
// kept.
enum class SideKeep { NoTarget, Slow, SameWay, Kept };

struct KeptSide
{
    SideKeep way;
    i32 gap = 0;
};

/* 0x402910: cop going the other way than the car it chases (both faster than 0x5360d4): onto the
 * car's side once it is 12 nodes past it.  nullopt when the game stopped. */
std::optional<KeptSide> keepSideOfTargetOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return KeptSide{SideKeep::NoTarget};
    if (compareBelow(f32(app, target + kCarSpeed), f64(app, kOncomingSpeed)))
        return KeptSide{SideKeep::Slow};
    const bool targetBack = compareAbove(0.0, f32(app, at(app, cop + kCarChased) + kCarSignedSpeed));
    const bool copBack = compareAbove(0.0, f32(app, cop + kCarSignedSpeed));
    if (copBack == targetBack)
        return KeptSide{SideKeep::SameWay};
    const std::optional<i32> gap =
        nodeGap(app, ts, at(app, cop + kCarNode), at(app, at(app, cop + kCarChased) + kCarNode));
    if (!gap)
        return std::nullopt;
    const i32 along = alongTrack(app, *gap);
    if (at(app, kSide) != 0)
    {
        if (along < kSideSwitchGap)
            putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
    }
    else if (along > -kSideSwitchGap)
        putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideOther);
    return KeptSide{SideKeep::Kept, *gap};
}

/* 0x402910: keepSideOfTargetOf cop eax; eax as the code leaves it on each way out: the cop, it
 * with the compare's status word, 0, the gap. */
nfs::u32 keepSideOfTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<KeptSide> kept = keepSideOfTargetOf(app, ts, cop);
    if (!kept)
        return args.eax;
    switch (kept->way)
    {
    case SideKeep::NoTarget:
        break;
    case SideKeep::Slow:
        args.eax = withStatus(cop);
        break;
    case SideKeep::SameWay:
        args.eax = 0;
        break;
    case SideKeep::Kept:
        args.eax = u32(kept->gap);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x403f80: how many more cops racer wants (0 for none), cop asking (0 none): none for a racer
 * out of the race; all of them for the car the cop was set on; for one that never got away and is
 * not over its node's speed by 0x536104 nor speeding past 2 (as bits), none; else up to a third of
 * the cops (at least 1) -- all but one of them for a player's car -- less those on it.  nullopt
 * when the game stopped. */
std::optional<u32> copsWantedOf(App* app, ThreadState& ts, u32 cop, u32 racer)
{
    i32 third = i32(at(app, kCopCount)) / 3;
    if (!(third > 1))
        third = 1;
    i32 allButOne = i32(at(app, kCopCount) - 1);
    if (!(allButOne > 1))
        allButOne = 1;
    const bool player = (at(app, racer + kCarFlags) & kFlagPlayer) != 0;
    if (at(app, racer + kCarState) == kOutOfPursuits)
        return 0;
    const u32 pursuit = at(app, racer + kCarPursuit);
    if (cop != 0 && racer == at(app, cop + kCarOrderFrom))
        return at(app, kCopCount);
    if (at(app, pursuit + kPursuitGaveUp) == 0)
    {
        const std::optional<Float> speed = nodeSpeed(app, ts, at(app, racer + kCarNode));
        if (!speed)
            return std::nullopt;
        const Float limit = fp::add(*speed, f64(app, kCopsWantedOver));
        if (!compareAbove(f32(app, racer + kCarSpeed), limit)
            && !(i32(at(app, racer + kCarSpeeding)) > i32(kSpeedingBitsWanted)))
            return 0;
    }
    const u32 more = u32(player ? allButOne : third) - at(app, pursuit + kPursuitCopCount);
    return i32(more) > 0 ? more : 0;
}

/* 0x403f80: copsWantedOf cop eax, racer edx; eax the count. */
nfs::u32 copsWanted(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> wanted = copsWantedOf(app, ts, args.eax, args.edx);
    if (!wanted)
        return args.eax;
    args.eax = *wanted;
    ts.sp += 4;
    return args.eax;
}

// How a hit on a cop went: nothing, the racer slower, too soon to chase it, the cop kept to its
// own, switched to the racer, the racer not wanted, joined.
enum class CopHit { Nothing, Slower, TooSoon, Kept, Switched, NotWanted, Joined };

struct HitOutcome
{
    CopHit way;
    bool joined = false;  // joinPursuitWith's
    u32 radioEax = 0;   // what the join's radio word left in eax
};

/* 0x402cb0: cop (not a player's) hit by racer: if the racer is no slower than 0x5360ec of the cop
 * (or the cop near still) and may be chased, a cop on another player's car -- not one with its
 * target pulled over that is parked (+0x70c 6) or alone -- gives that one up for this one; a free
 * cop takes it on where it wants to (copsWantedOf); each with its radio level up and
 * joinPursuitOf kind 1.  nullopt when the game stopped. */
std::optional<HitOutcome> copHitByCarOf(App* app, ThreadState& ts, u32 cop, u32 racer)
{
    if ((byte(app, racer + kCarFlags) & kFlagRacer) == 0 || (byte(app, cop + kCarFlags) & kFlagPlayer))
        return HitOutcome{CopHit::Nothing};
    if (!compareAbove(1.0, f32(app, cop + kCarSpeed))
        && !compareAbove(fp::div(f32(app, racer + kCarSpeed), f32(app, cop + kCarSpeed)), f64(app, kHitRatio)))
        return HitOutcome{CopHit::Slower};
    if (i32(at(app, kClock)) <= i32(at(app, racer + kCarNextChase)))
        return HitOutcome{CopHit::TooSoon};
    if (at(app, cop + kCarChased) != 0)
    {
        const u32 pursuit = at(app, cop + kCarPursuit);
        if (at(app, pursuit + kPursuitState) == kPulledOver
            && (at(app, cop + kCarDriverStep) == kParkedStep || at(app, pursuit + kPursuitCopCount) == 0))
            return HitOutcome{CopHit::Kept};
        if (racer == at(app, cop + kCarChased) || (byte(app, racer + kCarFlags) & kFlagPlayer) == 0)
            return HitOutcome{CopHit::Kept};
        if (!giveUpPursuitOf(app, ts, cop))
            return std::nullopt;
        raiseRadioLevel(app, cop);
        const std::optional<Joining> joining = joinPursuitWith(app, ts, kJoinHit, cop, racer, 0.0f);
        if (!joining)
            return std::nullopt;
        return HitOutcome{CopHit::Switched, joining->joined, joining->radioEax};
    }
    const std::optional<u32> wanted = copsWantedOf(app, ts, cop, racer);
    if (!wanted)
        return std::nullopt;
    if (*wanted == 0)
        return HitOutcome{CopHit::NotWanted};
    raiseRadioLevel(app, cop);
    const std::optional<Joining> joining = joinPursuitWith(app, ts, kJoinHit, cop, racer, 0.0f);
    if (!joining)
        return std::nullopt;
    return HitOutcome{CopHit::Joined, joining->joined, joining->radioEax};
}

/* A node's record, its lanes to the left (the high nibble of +0x13) and to the right (the low). */
u32 nodeRecord(App* app, u32 node) { return at(app, kNodes) + node * kNodeSize; }
i32 lanesLeft(App* app, u32 record) { return i32((byte(app, record + kNodeLanes) >> 4) & 0xf); }
i32 lanesRight(App* app, u32 record) { return i32(byte(app, record + kNodeLanes) & 0xf); }

/* 0x402d90: car's lane votes against the road's edge: 100 against (0x56eeb8) a lane off the
 * node's lanes; else 100 for the lane to the left (0x56eeb4, a lane below 7) or the right
 * (0x56eebc). */
void voteRoadLaneOf(App* app, u32 car)
{
    const u32 record = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    const bool off = 6 - lanesLeft(app, record) > lane || !(lanesRight(app, record) + 7 >= lane);
    const u32 vote = off ? kVoteOff : lane < 7 ? kVoteLeft : kVoteRight;
    put(app, vote, at(app, vote) + kVote);
}

/* 0x402d90: voteRoadLaneOf car eax; eax as the code leaves it: the node's record where the lane
 * is off to the left, else the right's lanes plus 7. */
nfs::u32 voteRoadLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const u32 record = nodeRecord(app, at(app, car + kCarNode));
    const u32 eax = 6 - lanesLeft(app, record) > i32(at(app, car + kCarLane)) ? record
                                                                                : u32(lanesRight(app, record) + 7);
    voteRoadLaneOf(app, car);
    args.eax = eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x402f70: whether car's lane is off its node's lanes. */
bool laneOffRoadOf(App* app, u32 car)
{
    const u32 record = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    return 7 - lanesLeft(app, record) > lane || lanesRight(app, record) + 6 < lane;
}

/* 0x402f70: laneOffRoadOf car eax, 1 or 0 in eax. */
nfs::u32 laneOffRoad(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = laneOffRoadOf(app, args.eax) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

// How a cop's parking went: a racer near, parked.
enum class Park { RacerNear, Parked };

/* 0x403180: cop parks at its roadblock, unless a racer is within 0x5360f4: put down there
 * (0x4a0980), turned across the road (a quarter or three quarters of a turn by the order's +7,
 * ea::yRotation into the frame's matrix, the orientation by it with 0x4e0280), settled (0x4a2690),
 * its wanted speed 0, its mark of the roadblock (+0x621 bit 8) off, stopped (0x40ce70).  The
 * matrix on the guest's stack, handed to the callees.  nullopt when the game stopped. */
std::optional<Park> parkCopOf(App* app, ThreadState& ts, u32 cop)
{
    const GuestFrame frame(ts, 0x18, 0x28);
    const u32 turn = frame - 0x28;
    for (u32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
    {
        const std::optional<Float> apart = distanceSize(app, ts, cop + kCarPlace, at(app, i * 4 + kRacers) + kCarPlace);
        if (!apart)
            return std::nullopt;
        if (compareBelow(Float(float(*apart)), f64(app, kParkReach)))
            return Park::RacerNear;
    }
    if (!guest::direct<0x4a0980, port::conv::frameFromRoad>(app, ts, cop, u32(gameSettings(app).backwards)))
        return std::nullopt;
    const u32 angle = byte(app, at(app, cop + kCarOrder) + kOrderFlag) != 0 ? kQuarterTurnBits : kThreeQuartersBits;
    if (!yRotation(app, ts, turn, angle) || !multiply(app, ts, cop + kCarOrientation, turn, cop + kCarOrientation)
        || !leftInEax(ts, feCore<0x4a2690, frontend::simb::settle>(app, ts, cop)))
        return std::nullopt;
    const u8 side = byte(app, cop + kCarSide);
    put(app, cop + kCarWanted, 0);
    putByte(app, cop + kCarSide, side & ~kSideRoadblock);
    // parkCop leaves the eax the stop leaves.
    if (!stopCarLeaving(app, ts, cop))
        return std::nullopt;
    return Park::Parked;
}

/* 0x403180: parkCopOf cop eax; eax as the stop leaves it, or with a racer near the distance's
 * with the compare's status word. */
nfs::u32 parkCop(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Park> park = parkCopOf(app, ts, args.eax);
    if (!park)
        return args.eax;
    if (*park == Park::RacerNear)
        args.eax = withStatus(args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x403d30: cop at its roadblock with its siren off: the siren on (0x4c9dd0: its index, its node,
 * lane 6 or 7, the clock), the roadblock's side on the radio (0x40b9c0: a side 0..2 by the
 * index's two low bits, mirrored with the lane where the track runs backwards), its count of them
 * (+0x710's low nibble) one up to 15, its mark of passing off.  False when the game stopped. */
bool roadblockSirenOf(App* app, ThreadState& ts, u32 cop)
{
    if (at(app, at(app, cop + kCarCopIndex) * kSirenSize + kSirens) != 0)
        return true;
    const u32 index = at(app, cop + kCarCopIndex);
    u32 lane = index & 1 ? 6 : 7, side = index & 1 ? 0 : 2;
    if (index & 2)
        side = 1;
    if (gameSettings(app).backwards != 0)
    {
        side = 2 - side;
        lane = 0xe - lane;
    }
    if (!leftInEax(ts, feCore<0x4c9dd0, frontend::simc::sirenOn>(app, ts, at(app, cop + kCarCopIndex),
                                                                    at(app, cop + kCarNode), lane, at(app, kClock))))
        return false;
    if (!guest::direct<0x40b9c0, pursuit::radioStoppedOf>(app, ts, cop, side))
        return false;
    if (i32(at(app, cop + kCarCopFlags) & 0xf) < 0xf)
        put(app, cop + kCarCopFlags, at(app, cop + kCarCopFlags) + 1);
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopPassed);
    return true;
}

/* 0x403d30: roadblockSirenOf cop eax; eax as the code leaves it (worked out before, as the callees
 * write none of it): the siren's record's offset where it was on, else the count before. */
nfs::u32 roadblockSiren(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const u32 siren = at(app, cop + kCarCopIndex) * kSirenSize;
    const u32 eax = at(app, siren + kSirens) != 0 ? siren : at(app, cop + kCarCopFlags) & 0xf;
    if (!roadblockSirenOf(app, ts, cop))
        return args.eax;
    args.eax = eax;
    ts.sp += 4;
    return args.eax;
}

// How a watch from a roadblock or a second car went: still to watch, the car past (quiet, or on
// the radio now), too far round the track, done.
enum class WatchWay { Watching, Passed, Radioed, TooFar, Done };

struct Watch
{
    WatchWay way;
    i32 value = 0;      // the gap, or the quarter's negation too far
    u32 radioEax = 0;  // what the radio's word left in eax, where it was said now
};

/* The new order of kind 2 by cop's index a watch ends with. */
void orderAfterWatch(App* app, u32 cop)
{
    giveOrderOf(app, cop, 0, kOrderAtNode, 0, 0, u8(at(app, cop + kCarCopIndex) & 1), 0);
}

/* 0x403df0: cop at a roadblock watches the car it is set for (+0x730) against the siren's node
 * (+0x34 of its siren's record): within 30 nodes past it, the radio's word that the car passed
 * (0x40bca0, once, +0x711 bit 0x20); 50 nodes past it (but not a quarter of the track), the siren
 * off, the roadblock done (the pursuit's roadblock car cleared) and a new order of kind 2 by its
 * index.  nullopt when the game stopped. */
std::optional<Watch> watchRoadblockOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 index = at(app, cop + kCarCopIndex);
    const u32 carNode = at(app, at(app, cop + kCarOrderFrom) + kCarNode);
    const std::optional<i32> gap = nodeGap(app, ts, at(app, index * kSirenSize + kSirens + kSirenNode), carNode);
    if (!gap)
        return std::nullopt;
    const i32 along = alongTrack(app, *gap);
    WatchWay way = WatchWay::Watching;
    u32 radioEax = 0;
    if (along < 0 && along > kPassedGap)
    {
        way = WatchWay::Passed;
        const u8 copKind = byte(app, cop + kCarCopKind);
        if ((copKind & kCopPassed) == 0)
        {
            putByte(app, cop + kCarCopKind, copKind | kCopPassed);
            const std::optional<u32> radio = guest::direct<0x40bca0, pursuit::radioResumeOf>(app, ts, cop);
            if (!radio)
                return std::nullopt;
            way = WatchWay::Radioed;
            radioEax = *radio;
        }
    }
    if (along > kDoneGap)
        return Watch{way, *gap, radioEax};
    const i32 quarter = -(i32(at(app, kNodeCount)) / 4);
    if (along < quarter)
        return Watch{WatchWay::TooFar, quarter};
    if (!sirenOff(app, ts, index))
        return std::nullopt;
    const u8 copKind = byte(app, cop + kCarCopKind);
    const u32 car = at(app, cop + kCarOrderFrom);
    putByte(app, cop + kCarCopKind, copKind & ~kCopRoadblock);
    if (car != 0)
    {
        const u32 pursuit = at(app, car + kCarPursuit);
        if (cop == at(app, pursuit + kPursuitRoadblock))
            put(app, pursuit + kPursuitRoadblock, 0);
    }
    orderAfterWatch(app, cop);
    return Watch{WatchWay::Done};
}

/* 0x403df0: watchRoadblockOf cop eax; eax as the code leaves it on each way out: the gap (its ah
 * the cop's +0x711 where the car had passed already, as the radio leaves it where it is told now),
 * the quarter, the order. */
nfs::u32 watchRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<Watch> watch = watchRoadblockOf(app, ts, cop);
    if (!watch)
        return args.eax;
    switch (watch->way)
    {
    case WatchWay::Watching:
    case WatchWay::TooFar:
        args.eax = u32(watch->value);
        break;
    case WatchWay::Passed:
        args.eax = (u32(watch->value) & 0xffff00ff) | u32(byte(app, cop + kCarCopKind)) << 8;
        break;
    case WatchWay::Radioed:
        args.eax = watch->radioEax;
        break;
    case WatchWay::Done:
        args.eax = at(app, cop + kCarOrder);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x403ed0: cop waiting as a second car watches its car (+0x730): 5 nodes past it (but not a
 * quarter of the track), its wait over -- within 20 nodes, parked (step 10), the radio's word that
 * it missed (0x40bd00) -- and a new order of kind 2 by its index.  nullopt when the game
 * stopped. */
std::optional<Watch> watchSecondCarOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 car = at(app, cop + kCarOrderFrom);
    const std::optional<i32> gap = nodeGap(app, ts, at(app, cop + kCarNode), at(app, car + kCarNode));
    if (!gap)
        return std::nullopt;
    const i32 along = alongTrack(app, *gap);
    if (along > kBehindGap)
        return Watch{WatchWay::Watching, *gap};
    const i32 quarter = -(i32(at(app, kNodeCount)) / 4);
    if (along < quarter)
        return Watch{WatchWay::TooFar, quarter};
    if (along < 0 && along > kMissedGap && at(app, cop + kCarDriverStep) == kWaitingStep
        && !guest::direct<0x40bd00, pursuit::radioAbortOf>(app, ts, cop))
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopSent);
    putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideRoadblock);
    orderAfterWatch(app, cop);
    return Watch{WatchWay::Done};
}

/* 0x403ed0: watchSecondCarOf cop eax; eax the gap, the quarter or the order. */
nfs::u32 watchSecondCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<Watch> watch = watchSecondCarOf(app, ts, cop);
    if (!watch)
        return args.eax;
    args.eax = watch->way == WatchWay::Done ? at(app, cop + kCarOrder) : u32(watch->value);
    ts.sp += 4;
    return args.eax;
}

/* 0x404340: cop sent to cut off car: an order of kind 3 at the node midway between them, wrapped
 * round the track, 8 and flag 0.  False when the game stopped. */
bool sendCopAheadOf(App* app, ThreadState& ts, u32 cop, u32 car)
{
    const u32 copNode = at(app, cop + kCarNode);
    const std::optional<i32> gap = nodeGap(app, ts, at(app, car + kCarNode), copNode);
    if (!gap)
        return false;
    const i32 half = (*gap - (*gap >> 31)) >> 1;
    u32 node = u32(half) + copNode;
    if (half >= 0)
    {
        const u32 count = at(app, kNodeCount);
        if (i32(node) >= i32(count))
            node -= count;
    }
    else if (i32(node) < 0)
        node += at(app, kNodeCount);
    giveOrderOf(app, cop, car, kOrderCutOff, u16(node), 8, 0, 0);
    return true;
}

/* 0x404340: sendCopAheadOf cop eax, car edx; eax the order. */
nfs::u32 sendCopAhead(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    if (!sendCopAheadOf(app, ts, cop, args.edx))
        return args.eax;
    args.eax = at(app, cop + kCarOrder);
    ts.sp += 4;
    return args.eax;
}

// How spotSpeederOf went: none found, still waiting, joined; how its look at the last racer
// ended, where none was found: no racers, not now (it may not be chased or is out), too far, not
// more cops wanted than the best, the best so far; how a wait ended: within 0x3c0 ticks the car
// not the order's, too far from it, not 0x100 ticks there.
enum class SpotWay { None, Waiting, Joined };
enum class Look { NoRacers, NotNow, Far, NotMore, Best };
enum class Wait { NotOrders, TooFar, TooSoon };

struct Spot
{
    SpotWay way;
    Look last = Look::NoRacers;
    i32 value = 0;  // the last look's: its gap's size (Far), the cops wanted (NotMore), its gap (Best)
    Wait wait = Wait::NotOrders;
    i32 gap = 0;    // the speeder's
    bool joined = false;
    u32 radioEax = 0;  // what the join's radio word left in eax
};

/* 0x402e10: cop looks for a speeder: of the racers that may be chased and are in the race, within
 * 15 nodes, each's speeding noted (noteSpeedingOf), the one the most cops are wanted for
 * (copsWantedOf); the cop joins its pursuit (joinPursuitOf kind 0) right away where its siren is
 * off and it was not told to wait (+0x711 bit 4) -- else unless it has waited less than 0x3c0
 * ticks (+0x72c) and the car is its order's, within 5 nodes, and has not been 0x100 ticks there
 * (+0x728).  No one found, both waits off.  nullopt when the game stopped. */
std::optional<Spot> spotSpeederOf(App* app, ThreadState& ts, u32 cop)
{
    Spot spot{SpotWay::None};
    const u32 copNode = at(app, cop + kCarNode);
    i32 most = 0;
    u32 speeder = 0;
    for (u32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
    {
        const u32 racer = at(app, i * 4 + kRacers);
        spot.last = Look::NotNow;
        if (i32(at(app, kClock)) <= i32(at(app, racer + kCarNextChase)))
            continue;
        if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfRace)
            continue;
        if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfPursuits)
            continue;
        const std::optional<i32> gap = nodeGap(app, ts, at(app, racer + kCarNode), copNode);
        if (!gap)
            return std::nullopt;
        const i32 size = *gap > 0 ? *gap : i32(neg(u32(*gap)));
        spot.last = Look::Far;
        spot.value = size;
        if (size > kSpotReach)
            continue;
        if (!noteSpeedingOf(app, ts, racer))
            return std::nullopt;
        const std::optional<u32> wanted = copsWantedOf(app, ts, cop, racer);
        if (!wanted)
            return std::nullopt;
        spot.last = Look::NotMore;
        spot.value = i32(*wanted);
        if (i32(*wanted) <= most)
            continue;
        most = i32(*wanted);
        speeder = racer;
        spot.gap = *gap;
        spot.last = Look::Best;
        spot.value = *gap;
    }
    if (speeder == 0)
    {
        put(app, cop + kCarWaitedAt, 0);
        put(app, cop + kCarWaitedFrom, 0);
        return spot;
    }
    const u32 index = at(app, cop + kCarCopIndex);
    if (at(app, index * kSirenSize + kSirens) != 0 || (byte(app, cop + kCarCopKind) & kCopSent))
    {
        if (at(app, cop + kCarWaitedFrom) == 0)
            put(app, cop + kCarWaitedFrom, at(app, kClock));
        if (!(i32(at(app, kClock) - at(app, cop + kCarWaitedFrom)) > kTimeToJoin))
        {
            spot.way = SpotWay::Waiting;
            spot.wait = Wait::NotOrders;
            if (speeder != at(app, cop + kCarOrderFrom))
                return spot;
            spot.wait = Wait::TooFar;
            if ((spot.gap > 0 ? spot.gap : i32(neg(u32(spot.gap)))) >= kOrderReach)
                return spot;
            if (at(app, cop + kCarWaitedAt) == 0)
                put(app, cop + kCarWaitedAt, at(app, kClock));
            spot.wait = Wait::TooSoon;
            if (!(i32(at(app, kClock) - at(app, cop + kCarWaitedAt)) > kTimeAtOrder))
                return spot;
        }
    }
    const std::optional<Joining> joining = joinPursuitWith(app, ts, kJoinSighted, cop, speeder, 0.0f);
    if (!joining)
        return std::nullopt;
    spot.way = SpotWay::Joined;
    spot.joined = joining->joined;
    spot.radioEax = joining->radioEax;
    return spot;
}

/* 0x4039c0: cop calls in a roadblock, unless the race has no cops of its own (0x6fd51c), it chases
 * no one, the pursuit has one, or the pursuit is not yet hot enough (cops, getaways, tickets and
 * time over 4): the first time only the radio call at 0x40b910 , +0x54 bit 2, the caller +0x5c);
 * then the free cop (not a player's, no order) ahead of the target more than 50 nodes and least
 * far (under three quarters of the track) gets the order (kind 0 with 3, 6 nodes before it) and is
 * the roadblock.  Whether it got one; nullopt when the game stopped. */
std::optional<bool> callRoadblockOf(App* app, ThreadState& ts, u32 cop)
{
    i32 nearest = i32(at(app, kNodeCount) * 3) / 4;
    u32 roadblock = 0;
    if (gameSettings(app).copCount != 0)
        return false;
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    const u32 pursuit = at(app, cop + kCarPursuit);
    if (at(app, pursuit + kPursuitRoadblock) != 0)
        return false;
    const u32 heat = at(app, pursuit + kPursuitGaveUp) + (at(app, pursuit + kPursuitCopCount) - 1)
                       + (at(app, target + kCarTickets) - 2)
                       + u32(i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) >> 11);
    if (i32(heat) < kRoadblockHeat)
        return false;
    if ((byte(app, pursuit + kPursuitCalls) & kPursuitRoadblockCall) == 0)
    {
        if (!guest::direct<0x40b910, pursuit::radioArrestOf>(app, ts, cop))
            return std::nullopt;
        const u32 own = at(app, cop + kCarPursuit);
        putByte(app, own + kPursuitCalls, byte(app, own + kPursuitCalls) | kPursuitRoadblockCall);
        put(app, pursuit + kPursuitCaller, cop);
        return false;
    }
    const u32 targetNode = at(app, target + kCarNode);
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0
            || (byte(app, other + kCarCopKind) & kCopRoadblock))
            continue;
        const u32 ahead = alongTrack(app, i32(at(app, other + kCarNode) - targetNode));
        if (i32(ahead) <= kRoadblockAhead || i32(ahead) >= nearest)
            continue;
        roadblock = other;
        nearest = i32(ahead);
    }
    if (roadblock == 0)
        return false;
    u32 node = at(app, roadblock + kCarNode) - kRoadblockBefore;
    if (i32(node) < 0)
        node += at(app, kNodeCount);
    putByte(app, roadblock + kCarCopKind, byte(app, roadblock + kCarCopKind) & ~kCopSent);
    giveOrderOf(app, roadblock, at(app, cop + kCarChased), 0, u16(node), 3, 0, 0);
    putByte(app, roadblock + kCarCopKind, byte(app, roadblock + kCarCopKind) | kCopRoadblock);
    put(app, pursuit + kPursuitRoadblock, roadblock);
    return true;
}

/* 0x4039c0: callRoadblockOf cop eax, 1 or 0 in eax. */
nfs::u32 callRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> called = callRoadblockOf(app, ts, args.eax);
    if (!called)
        return args.eax;
    args.eax = *called ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

std::optional<i32> sideOfOrderOf(App* app, ThreadState& ts, u32 car);
std::optional<u32> atOrderNodeOf(App* app, ThreadState& ts, u32 car);

/* What setWantedSpeed's code leaves in eax after setWantedSpeedOf for cop: the top speed's bits
 * where it capped, else copTopSpeed's eax with the compare's status word; ah the first flip's
 * byte where the track runs backwards. */
u32 setWantedSpeedEax(App* app, const ThreadState& ts, u32 cop, bool capped)
{
    u32 eax = capped ? floatBits(copTopSpeedOf(app, cop)) : withStatus(copTopSpeedEax(app, cop));
    if (gameSettings(app).backwards != 0)
        eax = (eax & 0xffff00ff) | u32(firstFlip(app, cop)) << 8;
    return eax;
}

/* 0x401430: setWantedSpeedOf for cop eax, the factor and the offset on the stack; ret 8. */
nfs::u32 setWantedSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<bool> capped =
        setWantedSpeedOf(app, ts, cop, floatAt(app, ts.sp + 4), floatAt(app, ts.sp + 8));
    if (!capped)
        return args.eax;
    args.eax = setWantedSpeedEax(app, ts, cop, *capped);
    ts.sp += 4 + 8;
    return args.eax;
}

/* What chaseSpeed's code leaves in eax after chaseSpeedOf for cop went `way`: the cop, the
 * target, it with the compare's status word, the gap's size, as the timer leaves it, the top
 * speed's bits; ah the first flip's byte where the track runs backwards and there is a target. */
u32 chaseSpeedEax(App* app, u32 cop, Chase way, u32 timerEax)
{
    const u32 target = at(app, cop + kCarChased);
    u32 eax = cop;
    switch (way)
    {
    case Chase::NoTarget:
        return cop;
    case Chase::ToldBehind:
        break;
    case Chase::ToldAhead:
    case Chase::AheadMatch:
        eax = target;
        break;
    case Chase::BehindFast:
        eax = withStatus(cop);
        break;
    case Chase::BehindNear:
    {
        const i32 gap = i32(at(app, kGap));
        eax = gap > 0 ? u32(gap) : neg(u32(gap));
        break;
    }
    case Chase::BehindTimer:
        eax = timerEax;  // what the timer's setting left
        break;
    case Chase::AheadTop:
        eax = floatBits(copTopSpeedOf(app, cop));
        break;
    }
    if (gameSettings(app).backwards != 0)
        eax = (eax & 0xffff00ff) | u32(firstFlip(app, cop)) << 8;
    return eax;
}

/* 0x402420: chaseSpeedOf cop eax. */
nfs::u32 chaseSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    u32 timerEax = 0;
    const std::optional<Chase> way = chaseSpeedOf(app, ts, cop, timerEax);
    if (!way)
        return args.eax;
    args.eax = chaseSpeedEax(app, cop, *way, timerEax);
    ts.sp += 4;
    return args.eax;
}

// The ways pursueTargetOf goes: no target, given up, the chase's speed, behind the car, its top
// speed, and that slowed at night.
enum class Pursue { NoTarget, GaveUp, Chasing, Behind, Top, TopAtNight };

struct Pursuit
{
    Pursue way;
    bool capped = false;           // setWantedSpeedOf's, for the patrol's speed
    Chase chase = Chase::NoTarget;  // chaseSpeedOf's
    u32 eax = 0;                  // what the timer's setting left in eax, for the chase's or the top's
};

/* 0x4025c0: cop after the car it chases: too far behind (more than the car's tickets allow,
 * 0x5521ac or for a fast cop 0x5521b0) or the car out of the race, it gives up (giveUpPursuitOf),
 * a timer (0x405fd0), its side flipped and the patrol's speed (setWantedSpeedOf 1.19, 0); pulled
 * over or within 15 nodes, the chase's speed (chaseSpeedOf); ahead of the car, onto the car's side
 * and the patrol's speed; otherwise its top speed and a timer -- at night (0x6fd4c8) 0x5360c4 of
 * it for a player's car not braking (+0x660).  nullopt when the game stopped. */
std::optional<Pursuit> pursueTargetOf(App* app, ThreadState& ts, u32 cop)
{
    const auto patrol = [&](Pursue way) -> std::optional<Pursuit> {
        const std::optional<bool> capped = setWantedSpeedOf(app, ts, cop, floatOf(kPatrolFactorBits), 0.0f);
        if (!capped)
            return std::nullopt;
        return Pursuit{way, *capped};
    };
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return patrol(Pursue::NoTarget);
    const i32 signedGap = i32(at(app, kGap));
    const u32 gap = signedGap > 0 ? u32(signedGap) : neg(u32(signedGap));
    const u32 pursuit = at(app, cop + kCarPursuit);
    bool giveUp = (byte(app, target + kCarFlags) & kFlagRacer) && at(app, target + kCarState) == kOutOfRace;
    if (!giveUp)
    {
        const u32 table = byte(app, cop + kCarCopKind) & kCopFast ? kFastGiveUpGap : kGiveUpGap;
        giveUp = gap > at(app, (at(app, target + kCarTickets) << 4) + table);
    }
    if (giveUp)
    {
        if (!giveUpPursuitOf(app, ts, cop) || !setTimer(app, ts, cop, 1, 0))
            return std::nullopt;
        putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideOther);
        return patrol(Pursue::GaveUp);
    }
    if (at(app, pursuit + kPursuitState) == kPulledOver || !(i32(gap) > kChaseGap))
    {
        u32 timerEax = 0;
        const std::optional<Chase> chase = chaseSpeedOf(app, ts, cop, timerEax);
        if (!chase)
            return std::nullopt;
        return Pursuit{Pursue::Chasing, false, *chase, timerEax};
    }
    if (at(app, kBehind) != 0)
    {
        if (at(app, kSide) != 0)
            putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideOther);
        else
            putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
        return patrol(Pursue::Behind);
    }
    putFloat(app, cop + kCarWanted, copTopSpeedOf(app, cop));
    // By address: pursueTarget leaves the eax it leaves.
    const std::optional<u32> timerEax = setTimerLeavingEax(app, ts, cop, 1, 0);
    if (!timerEax)
        return std::nullopt;
    flipWantedSign(app, cop);
    if (gameSettings(app).night == 0)
        return Pursuit{Pursue::Top, false, Chase::NoTarget, *timerEax};
    const u32 chased = at(app, cop + kCarChased);
    if ((byte(app, chased + kCarFlags) & kFlagPlayer) == 0 || at(app, chased + kCarBraking) != 0)
        return Pursuit{Pursue::TopAtNight};
    putFloat(app, cop + kCarWanted, float(fp::mul(f32(app, cop + kCarWanted), f64(app, kNightShare))));
    return Pursuit{Pursue::TopAtNight};
}

/* 0x4025c0: pursueTargetOf cop eax; eax as the code leaves it on each way out: setWantedSpeed's
 * or chaseSpeed's; with its top speed the timer's, its al the wanted speed's sign byte where the
 * car is on the other side (the flips' second), at night the target. */
nfs::u32 pursueTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<Pursuit> pursuit = pursueTargetOf(app, ts, cop);
    if (!pursuit)
        return args.eax;
    switch (pursuit->way)
    {
    case Pursue::NoTarget:
    case Pursue::GaveUp:
    case Pursue::Behind:
        args.eax = setWantedSpeedEax(app, ts, cop, pursuit->capped);
        break;
    case Pursue::Chasing:
        args.eax = chaseSpeedEax(app, cop, pursuit->chase, pursuit->eax);
        break;
    case Pursue::Top:
        args.eax = pursuit->eax;  // what the timer's setting left (its low byte the sign's where the side says)
        if (byte(app, cop + kCarSide) & kSideOther)
            args.eax = (args.eax & ~0xffu) | byte(app, cop + kCarWantedSign);
        break;
    case Pursue::TopAtNight:
        args.eax = at(app, cop + kCarChased);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x403030: cop takes its order, by the table at 0x403018: 0 to its node (step 3 there, else on
 * toward it, step 1); 1 step 2; 3 a roadblock at its node (step 7, the roadblock's radio
 * 0x40b7f0); 4 to its node for a second car (step 8; 10 once there); any other its side by the
 * order's (+6), step 1.  False when the game stopped. */
bool takeOrderOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 order = at(app, cop + kCarOrder);
    const auto mark = [&](u32 step) {
        put(app, cop + kCarOnPursuit, 1);
        const u8 copFlags = byte(app, cop + kCarCopFlags);
        put(app, cop + kCarDriverStep, step);
        putByte(app, cop + kCarCopFlags, copFlags | kCopChasing);
    };
    // jmp [kind * 4 + 0x403018]: 0x403075, 0x4030a9, 0x40304e, 0x4030ba, 0x40310e.
    switch (byte(app, order + kOrderKind))
    {
    case kOrderToNode:
    {
        const std::optional<u32> there = atOrderNodeOf(app, ts, cop);
        if (!there)
            return false;
        if (*there == 0)
        {
            if (!sideOfOrderOf(app, ts, cop))
                return false;
            put(app, cop + kCarDriverStep, kStepToNode);
        }
        else
            put(app, cop + kCarDriverStep, kStepAtNode);
        return true;
    }
    case kOrderHold:
        put(app, cop + kCarDriverStep, kStepHold);
        return true;
    case kOrderRoadblock:
    {
        const std::optional<u32> there = atOrderNodeOf(app, ts, cop);
        if (!there)
            return false;
        if (*there != 0)
            return true;
        if (!sideOfOrderOf(app, ts, cop))
            return false;
        if (at(app, cop + kCarDriverStep) == kStepRoadblock)
            return true;
        const u8 copFlags = byte(app, cop + kCarCopFlags);
        put(app, cop + kCarDriverStep, kStepRoadblock);
        putByte(app, cop + kCarCopFlags, copFlags | kCopChasing);
        put(app, cop + kCarOnPursuit, 1);
        guest::direct<0x40b7f0, pursuit::radioNothingFor>(app, ts, cop);
        return !ts.terminate;
    }
    case kOrderSecondCar:
    {
        const std::optional<u32> there = atOrderNodeOf(app, ts, cop);
        if (!there)
            return false;
        if (*there != 0)
        {
            mark(kWaitingStep);
            return true;
        }
        if (!sideOfOrderOf(app, ts, cop))
            return false;
        if (at(app, cop + kCarDriverStep) != kStepSecondCar)
            mark(kStepSecondCar);
        return true;
    }
    default:
        if (byte(app, order + kOrderSide) == 0)
            putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
        else
            putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideOther);
        put(app, cop + kCarDriverStep, kStepToNode);
        return true;
    }
}

/* 0x403030: takeOrderOf cop eax; eax its order's kind. */
nfs::u32 takeOrder(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const u32 kind = byte(app, at(app, cop + kCarOrder) + kOrderKind);
    if (!takeOrderOf(app, ts, cop))
        return args.eax;
    args.eax = kind;
    ts.sp += 4;
    return args.eax;
}

/* 0x403b20: cop calls in two cars to box in a player's car, unless the race has no roadblocks
 * (0x55eb2e bit 0x20, 0x6fd51c) or the pursuit is not hot enough (over 3): the radio's call once
 * (0x40b860, +0x54 bit 4, the caller +0x5c); the two free cops ahead of the car more than 50
 * nodes (under three quarters of the track), the nearest two, if within 400 nodes of each other,
 * each get the order (kind 4, the node midway between them, the farther flagged) and are marked
 * (+0x710 0x410, +0x734).  Whether they did; nullopt when the game stopped. */
std::optional<bool> callTwoCarsOf(App* app, ThreadState& ts, u32 cop)
{
    i32 nearestAhead = i32(at(app, kNodeCount) * 3) / 4;
    u32 nearest = 0, runnerUp = 0;
    if (byte(app, kRaceSwitches) & kRaceNoRams)
        return false;
    if (gameSettings(app).copCount != 0)
        return false;
    const u32 target = at(app, cop + kCarChased);
    if (target == 0 || (byte(app, target + kCarFlags) & kFlagPlayer) == 0)
        return false;
    const u32 pursuit = at(app, cop + kCarPursuit);
    const u32 heat = at(app, pursuit + kPursuitGaveUp) + (at(app, pursuit + kPursuitCopCount) - 1)
                       + at(app, target + kCarTickets)
                       + u32(i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) >> 11);
    if (i32(heat) < kTwoCarsHeat)
        return false;
    if ((byte(app, pursuit + kPursuitCalls) & kPursuitSecondCall) == 0)
    {
        if (!guest::direct<0x40b860, pursuit::radioTicketOf>(app, ts, cop))
            return std::nullopt;
        const u32 own = at(app, cop + kCarPursuit);
        putByte(app, own + kPursuitCalls, byte(app, own + kPursuitCalls) | kPursuitSecondCall);
        put(app, pursuit + kPursuitCaller, cop);
    }
    const u32 targetNode = at(app, at(app, cop + kCarChased) + kCarNode);
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0)
            continue;
        const u8 otherKind = byte(app, other + kCarCopKind);
        if ((otherKind & kCopRoadblock) || (otherKind & kCopSent))
            continue;
        const std::optional<i32> gap = nodeGap(app, ts, at(app, other + kCarNode), targetNode);
        if (!gap)
            return std::nullopt;
        const i32 ahead = alongTrack(app, *gap);
        if (ahead <= kRoadblockAhead || ahead >= nearestAhead)
            continue;
        if (nearest != 0)
            runnerUp = nearest;
        nearest = other;
        nearestAhead = ahead;
    }
    if (nearest == 0 || runnerUp == 0)
        return false;
    const std::optional<i32> apart = nodeGap(app, ts, at(app, runnerUp + kCarNode), at(app, nearest + kCarNode));
    if (!apart)
        return std::nullopt;
    if (*apart >= kTwoCarsApart)
        return false;
    const i32 half = (*apart - (*apart >> 31)) >> 1;
    u32 node = u32(half) + at(app, nearest + kCarNode);
    if (half >= 0)
    {
        const u32 count = at(app, kNodeCount);
        if (i32(node) >= i32(count))
            node -= count;
    }
    else if (i32(node) < 0)
        node += at(app, kNodeCount);
    const auto mark = [&](u32 car) {
        const u16 copFlags = word(app, car + kCarCopFlags);
        put(app, car + kCarOnPursuit, 1);
        putWord(app, car + kCarCopFlags, copFlags | kCopsBoxing);
    };
    giveOrderOf(app, runnerUp, at(app, cop + kCarChased), kOrderTwoCars, u16(node), 0, 0, 1);
    mark(runnerUp);
    giveOrderOf(app, nearest, at(app, cop + kCarChased), kOrderTwoCars, u16(node), 0, 0, 0);
    mark(nearest);
    return true;
}

/* 0x403b20: callTwoCarsOf cop eax, 1 or 0 in eax. */
nfs::u32 callTwoCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> called = callTwoCarsOf(app, ts, args.eax);
    if (!called)
        return args.eax;
    args.eax = *called ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x402110's test of whether car has stopped for the cops: its driver's state (+0x6e0) 7 for a
 * player's car, 6 for another. */
bool hasStoppedForCops(App* app, u32 car)
{
    return at(app, car + kCarDriverState) == (byte(app, car + kCarFlags) & kFlagPlayer ? 7u : 6u);
}

/* 0x402110: whether cop has its target stopped or is still at it: with the car pulled over and
 * stopped (hasStoppedForCops), so once it is within 0x536074; otherwise the cop near enough (1 or
 * 2 times 0x53607c, by the cops already stopped by it), the car slow (0x56ea50), on the ground
 * (+0x164) and the cop's +0x10c over 0x536084: the first time it calls the stop on the radio
 * (callAboutCar; its next call 0xa0..0xe0 ticks on by the car's lap against the leader's) and
 * marks it (+0x710 bit 0x40); later, past that time, it pulls the car over (state 2, 0x140 ticks)
 * and has it if the car has stopped; a player's car not yet braking (+0x508 under 12) gets 0x13
 * ticks less.  Else the mark cleared and, pulled over, so once the car has stopped and the stop is
 * past.  nullopt when the game stopped. */
std::optional<bool> targetStoppingOf(App* app, ThreadState& ts, u32 cop)
{
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    const u32 pursuit = at(app, cop + kCarPursuit);
    const auto apart = [&] {
        return distanceSize(app, ts, at(app, cop + kCarChased) + kCarPlace, cop + kCarPlace);
    };
    if (at(app, pursuit + kPursuitState) == kPulledOver && hasStoppedForCops(app, target))
    {
        const std::optional<Float> d = apart();
        if (!d)
            return std::nullopt;
        if (compareBelow(Float(float(*d)), f64(app, kStoppedReach)))
            return true;
    }
    const float reach =
        float(fp::mul(Float(i32(at(app, pursuit + kPursuitStoppedBy)) < 3 ? 1 : 2), f64(app, kStopReach)));
    const std::optional<Float> d = apart();
    if (!d)
        return std::nullopt;
    bool close = compareBelow(Float(float(*d)), Float(reach));
    if (close)
        close = !compareAbove(f32(app, at(app, cop + kCarChased) + kCarSpeed), f32(app, kStoppingSpeed));
    if (close)
        close = word(app, cop + kCarAirborne) == 0;
    if (close)
        close = compareAbove(f32(app, cop + kCarForward), f64(app, kStopRolling));
    if (close)
    {
        if ((byte(app, cop + kCarCopFlags) & kCopCalledStop) == 0)
        {
            const u32 car = at(app, cop + kCarChased);
            u32 wait = kStopCallTicks;
            if ((byte(app, car + kCarFlags) & kFlagPlayer) == 0)
            {
                const i32 laps = i32(at(app, car + kCarLaps) - at(app, at(app, kLeader) + kCarLaps));
                if (laps > kLapsApart)
                    wait = kStopCallSoon;
                else if (laps < -kLapsApart)
                    wait = kStopCallLate;
            }
            if (!callAboutCar(app, ts, cop, at(app, cop + kCarChased)))
                return std::nullopt;
            const u32 due = at(app, kClock) + wait;
            const u8 copFlags = byte(app, cop + kCarCopFlags);
            put(app, cop + kCarRadioCallDue, due);
            putByte(app, cop + kCarCopFlags, copFlags | kCopCalledStop);
            return false;
        }
        const u32 now = at(app, kClock);
        const u32 due = at(app, cop + kCarRadioCallDue);
        if (i32(now) > i32(due))
        {
            put(app, pursuit + kPursuitState, kPulledOver);
            put(app, pursuit + kPursuitStopEnds, now + kStopTicks);
            return hasStoppedForCops(app, at(app, cop + kCarChased));
        }
        const u32 car = at(app, cop + kCarChased);
        if ((byte(app, car + kCarFlags) & kFlagPlayer) && byte(app, car + kCarThrottleNow) < 0xc)
            put(app, cop + kCarRadioCallDue, due - kStopCallEarlier);
        return false;
    }
    putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) & ~kCopCalledStop);
    if (at(app, pursuit + kPursuitState) != kPulledOver)
        return false;
    if (!hasStoppedForCops(app, at(app, cop + kCarChased)))
        return false;
    return i32(at(app, kClock)) > i32(at(app, pursuit + kPursuitStopEnds));
}

/* 0x402110: targetStoppingOf cop eax, 1 or 0 in eax. */
nfs::u32 targetStopping(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<bool> stopped = targetStoppingOf(app, ts, args.eax);
    if (!stopped)
        return args.eax;
    args.eax = *stopped ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

// How callBackupOf went: no target, no more cops wanted, the nearest of the others sent last,
// else the cops looked along (the fast ones maybe sent among them).
enum class Backup { NoTarget, NotWanted, NearestSent, Looked };

struct BackupCall
{
    Backup way;
    u32 sent = 0;  // the nearest, sent last
};

/* 0x4043a0: cop calls backup for its target where more cops are wanted (copsWantedOf): the first
 * time, after 0x280 ticks alone, the radio call at 0x40b720 , +0x54 bit 1); then of the free cops
 * not at roadblocks, the fast ones (+0x711 bit 1) ahead of the target within 8 times its tickets'
 * gap are each sent to cut it off (sendCopAheadOf), and of the others the nearest on the other
 * side of it than the cop, within 200 nodes and twice the gap, is sent -- unless it is setting up
 * a roadblock (step 7).  nullopt when the game stopped. */
std::optional<BackupCall> callBackupOf(App* app, ThreadState& ts, u32 cop)
{
    i32 nearest = kBackupReach;
    u32 chosen = 0;
    const u32 target = at(app, cop + kCarChased);
    if (target == 0)
        return BackupCall{Backup::NoTarget};
    const std::optional<u32> wanted = copsWantedOf(app, ts, 0, target);
    if (!wanted)
        return std::nullopt;
    if (*wanted == 0)
        return BackupCall{Backup::NotWanted};
    const u32 pursuit = at(app, cop + kCarPursuit);
    const u8 calls = byte(app, pursuit + kPursuitCalls);
    if ((calls & (kPursuitBackupCall | kPursuitRoadblockCall | kPursuitSecondCall)) == 0
        && i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) > i32(kBackupAlone)
        && at(app, pursuit + kPursuitCopCount) == 1)
    {
        if (!guest::direct<0x40b720, pursuit::radioRoadblockOf>(app, ts, cop))
            return std::nullopt;
        putByte(app, pursuit + kPursuitCalls, byte(app, pursuit + kPursuitCalls) | kPursuitBackupCall);
    }
    const u32 targetNode = at(app, target + kCarNode);
    const std::optional<i32> own = nodeGap(app, ts, at(app, cop + kCarNode), targetNode);
    if (!own)
        return std::nullopt;
    for (u32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const u32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0)
            continue;
        const u8 otherKind = byte(app, other + kCarCopKind);
        if ((otherKind & kCopRoadblock) || (otherKind & kCopSent))
            continue;
        const std::optional<i32> gap = nodeGap(app, ts, at(app, other + kCarNode), targetNode);
        if (!gap)
            return std::nullopt;
        const u32 size = *gap > 0 ? u32(*gap) : neg(u32(*gap));
        if (byte(app, other + kCarCopKind) & kCopFastKind)
        {
            if (at(app, other + kCarDriverStep) == kStepRoadblock)
                continue;
            if (gameSettings(app).backwards != 0 && *gap >= 0)
                continue;
            if (gameSettings(app).backwards == 0 && *gap < 0)
                continue;
            if ((at(app, (at(app, target + kCarTickets) << 4) + kByTickets) << 3) < size)
                continue;
            if (!sendCopAheadOf(app, ts, other, target))
                return std::nullopt;
            continue;
        }
        if (i32(u32(*own) ^ u32(*gap)) >= 0)
            continue;
        if (i32(size) >= nearest)
            continue;
        if (at(app, (at(app, target + kCarTickets) << 4) + kByTickets) * 2 < size)
            continue;
        chosen = other;
        nearest = i32(size);
    }
    if (chosen != 0 && at(app, chosen + kCarDriverStep) != kStepRoadblock)
    {
        if (!sendCopAheadOf(app, ts, chosen, target))
            return std::nullopt;
        return BackupCall{Backup::NearestSent, chosen};
    }
    return BackupCall{Backup::Looked};
}

/* What spotSpeeder's code leaves in eax after spotSpeederOf for cop: none found, what the last look
 * left (the cop's node for no racers, the clock, the gap's size, the cops wanted, the gap);
 * waiting, the ticks waited, the gap's size; joined, the join's (its kind where the cop was on a
 * pursuit, else as the radio leaves it). */
u32 spotSpeederEax(App* app, const ThreadState& ts, u32 cop, const Spot& spot)
{
    switch (spot.way)
    {
    case SpotWay::None:
        switch (spot.last)
        {
        case Look::NoRacers:
            return at(app, cop + kCarNode);
        case Look::NotNow:
            return at(app, kClock);
        case Look::Far:
        case Look::NotMore:
        case Look::Best:
            return u32(spot.value);
        }
        break;
    case SpotWay::Waiting:
        switch (spot.wait)
        {
        case Wait::NotOrders:
            return at(app, kClock) - at(app, cop + kCarWaitedFrom);
        case Wait::TooFar:
            return spot.gap > 0 ? u32(spot.gap) : neg(u32(spot.gap));
        case Wait::TooSoon:
            return at(app, kClock) - at(app, cop + kCarWaitedAt);
        }
        break;
    case SpotWay::Joined:
        return spot.joined ? spot.radioEax : kJoinSighted;
    }
    return cop;  // (not reached: every way above returns)
}

/* 0x402e10: spotSpeederOf cop eax. */
nfs::u32 spotSpeeder(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<Spot> spot = spotSpeederOf(app, ts, cop);
    if (!spot)
        return args.eax;
    args.eax = spotSpeederEax(app, ts, cop, *spot);
    ts.sp += 4;
    return args.eax;
}

/* What copStopsBy's code's choice of the cop that stops leaves in eax, read before
 * copStopsByOf for cop (nothing in the look writes): the cop where there was none to make, 10000's
 * bits where one had stopped already, else the count looked along. */
u32 copStopsByChoice(App* app, u32 cop)
{
    const u32 pursuit = at(app, cop + kCarPursuit);
    const u32 count = at(app, pursuit + kPursuitCopCount);
    if (i32(count) > 1 && (byte(app, cop + kCarFlags) & kFlagPlayer))
        return at(app, pursuit + kPursuitStoppedBy) != 0 ? kFarBits : count;
    return cop;
}

/* What copStopsBy's code leaves in eax after copStopsByOf for cop went `way`: dropCop's (the
 * pursuit for a player's cop, else the cop), the choice's, the car it stops by. */
u32 copStopsByEax(App* app, u32 cop, StopBy way, u32 choice, u32 pursuit, bool player)
{
    switch (way)
    {
    case StopBy::Dropped:
        return player ? pursuit : cop;
    case StopBy::Counted:
        return choice;
    case StopBy::Stops:
        return at(app, cop + kCarChased);
    }
    return cop;
}

/* 0x401e40: copStopsByOf cop eax. */
nfs::u32 copStopsBy(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = copStopsByLeaving(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* copStopsByDone with the eax the code leaves; nullopt when the game stopped. */
std::optional<u32> copStopsByLeaving(App* app, ThreadState& ts, u32 cop)
{
    const u32 choice = copStopsByChoice(app, cop), pursuit = at(app, cop + kCarPursuit);
    const bool player = byte(app, cop + kCarFlags) & kFlagPlayer;
    const std::optional<StopBy> way = copStopsByOf(app, ts, cop);
    if (!way)
        return std::nullopt;
    return copStopsByEax(app, cop, *way, choice, pursuit, player);
}

/* copStopsByOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool copStopsByDone(App* app, ThreadState& ts, u32 cop)
{
    return copStopsByOf(app, ts, cop).has_value();
}

/* What callBackup's code leaves in eax after callBackupOf for cop: the cop with no target, 0 with
 * no more wanted, the order of the nearest sent last, else the cops' count looked along. */
u32 callBackupEax(App* app, u32 cop, const BackupCall& call)
{
    switch (call.way)
    {
    case Backup::NoTarget:
        return cop;
    case Backup::NotWanted:
        return 0;
    case Backup::NearestSent:
        return at(app, call.sent + kCarOrder);
    case Backup::Looked:
        break;
    }
    const i32 count = i32(at(app, kCopCount));
    return count > 0 ? u32(count) : 0;
}

/* 0x4043a0: callBackupOf cop eax. */
nfs::u32 callBackup(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<BackupCall> call = callBackupOf(app, ts, cop);
    if (!call)
        return args.eax;
    args.eax = callBackupEax(app, cop, *call);
    ts.sp += 4;
    return args.eax;
}

// What copDriverOf's last step was, for what its code leaves in eax: a value of the step's, that
// with the compare's status word, as the callee by address last called leaves it, or one of the
// file's own whose code leaves its own.
enum class DriveEnd { Value, Status, Callee, Spotted, StopsBy, Backup };

struct Drive
{
    DriveEnd end;
    u32 value = 0;  // Value's and Status's: the step's result the code leaves (a gap, a flag, an order)
    Spot spot{SpotWay::None};
    StopBy stopsBy = StopBy::Counted;
    u32 choice = 0, pursuit = 0;
    bool player = false;
    BackupCall backup{Backup::NoTarget};
};

/* 0x4032e0: a cop's driver, each tick, where its driver is ready (0x40df10): the tick's sizing up
 * cleared (0x56ea58..), its slow mark off; backwards on the ground, the radio's word of the stop
 * (radioStopOf), else that word's mark off; its roadblock watched where its siren is on
 * (watchRoadblockOf) and its second car's where it waits (watchSecondCarOf); a new order taken
 * (takeOrderOf).  Then by its step (+0x70c, the table at 0x4032a4): 1 patrol, chasing whoever it
 * spots (spotSpeederOf; step 5 once it chases, else the order again); 7 and 8 to the order's node
 * (to 1, or 9, where it is there); 9 at the node, slowly (10 once stopped, the spikes' radio); 10
 * parked at a roadblock (parkCopOf; 11 when it cannot, else steering on); 11 stopped; 2 patrol to
 * the order's node; 3 driving the road's lanes (4 off the road, the roadblock's siren); 4 stopped;
 * 5 the chase (sizeUpTargetOf, keepSideOfTargetOf, pursueTargetOf, the target's speeding,
 * votePursuitLaneOf, targetStoppingOf -- step 6 and copStopsByOf once it has the car; the stop's
 * word, or now and then backup, a roadblock or a second car on the radio); 6 at the stop
 * (stopOverOf; 1 after); any other 1.  Chasing someone, every patrolling step goes to 5.  nullopt
 * when the game stopped. */
std::optional<Drive> copDriverOf(App* app, ThreadState& ts, u32 cop)
{
    // The original's frame (its six registers pushed), below which parkCopOf's and copStopsByOf's
    // go: the matrix and the places they hand on are at the original's depth.
    const GuestFrame frame(ts, 0x18, 0);
    const auto value = [](u32 v) { return Drive{DriveEnd::Value, v}; };
    const auto spotted = [](const Spot& s) {
        Drive d{DriveEnd::Spotted};
        d.spot = s;
        return d;
    };
    const auto patrol = [&](u32 factor, u32 offset) {
        return drivePatrolOf(app, ts, cop, floatOf(factor), floatOf(offset));
    };
    const auto chasing = [&] { return at(app, cop + kCarChased) != 0; };
    const auto step = [&](u32 s) { put(app, cop + kCarDriverStep, s); };
    const std::optional<u32> ready = guest::direct<0x40df10, aiplan::stepLoading>(app, ts, cop);
    if (!ready)
        return std::nullopt;
    if (*ready == 0)
        return value(0);
    if (!clearBytes(app, ts, kTurning, 0, kSizingBytes))
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopHolds);
    if (isReversing(app, cop))
    {
        if (!radioStopOf(app, ts, cop))
            return std::nullopt;
    }
    else
        putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopToldStop);
    if (at(app, at(app, cop + kCarCopIndex) * kSirenSize + kSirens) != 0 && !watchRoadblockOf(app, ts, cop))
        return std::nullopt;
    if ((byte(app, cop + kCarCopKind) & kCopSent) && !watchSecondCarOf(app, ts, cop))
        return std::nullopt;
    if (takeNewOrder(app, cop) && !takeOrderOf(app, ts, cop))
        return std::nullopt;
    const u32 now = at(app, cop + kCarDriverStep);
    // jmp [step * 4 + 0x4032a4]: 0x403380, 0x40338a, 0x4035f1, 0x403654, 0x4036d1,
    // 0x403707, 0x4037f4, 0x4033c5, 0x40343d, 0x403496, 0x40350b, 0x4035a8.
    switch (now > kStepParked ? kStepNew : now)
    {
    case kStepNew:
        step(kStepPatrol);
        [[fallthrough]];
    case kStepPatrol:
    {
        if (!patrol(kPatrolFactorBits, 0))
            return std::nullopt;
        const std::optional<Spot> spot = spotSpeederOf(app, ts, cop);
        if (!spot)
            return std::nullopt;
        if (chasing())
        {
            step(kStepChase);
            return spotted(*spot);
        }
        const u32 kind = byte(app, at(app, cop + kCarOrder) + kOrderKind);
        if (!takeOrderOf(app, ts, cop))
            return std::nullopt;
        return value(kind);
    }
    case kStepToBlock:
    case kStepToSecond:
    case kStepAtSecond:
    case kStepHold:
    {
        const bool slow = now == kStepAtSecond;
        if (!(now == kStepHold ? patrol(kPatrolFactorBits, 0) : slow ? patrol(0, kSixBits) : patrol(kTwoBits, 0)))
            return std::nullopt;
        const std::optional<Spot> spot = spotSpeederOf(app, ts, cop);
        if (!spot)
            return std::nullopt;
        const u32 target = at(app, cop + kCarChased);
        if (target != 0)
        {
            step(kStepChase);
            if (now != kStepHold)
                return spotted(*spot);
        }
        const std::optional<u32> there = atOrderNodeOf(app, ts, cop);
        if (!there)
            return std::nullopt;
        if (now == kStepHold)
        {
            if (*there != 0)
            {
                putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) & ~kCopToNode);
                return value(*there);
            }
            if (byte(app, cop + kCarCopFlags) & kCopToNode)
                return value(*there);
        }
        else if (*there != 0)
        {
            if (now == kStepToSecond)
            {
                step(kStepAtSecond);
                return value(*there);
            }
            if (now == kStepAtSecond)
            {
                if (compareAbove(f32(app, cop + kCarSpeed), f64(app, kParkSpeed)))
                    return Drive{DriveEnd::Status, *there};
                step(kStepParking);
                const std::optional<u32> radio = guest::direct<0x40bac0, pursuit::radioAmbulanceOf>(app, ts, cop);
                if (!radio)
                    return std::nullopt;
                return value(*radio);
            }
            const u32 order = at(app, cop + kCarOrder);
            putByte(app, order + kOrderKind, kOrderAtNode);
            step(kStepPatrol);
            const u8 copFlags = byte(app, cop + kCarCopFlags);
            put(app, cop + kCarOnPursuit, target);
            putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
            return value(order);
        }
        const std::optional<i32> gap = sideOfOrderOf(app, ts, cop);
        if (!gap)
            return std::nullopt;
        if (now == kStepHold)
            putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) | kCopToNode);
        return value(u32(*gap));
    }
    case kStepParking:
    case kStepParked:
    {
        if (now == kStepParking)
        {
            const std::optional<u32> there = atOrderNodeOf(app, ts, cop);
            if (!there)
                return std::nullopt;
            if (*there != 0)
            {
                putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideRoadblock);
                if (!asCalled(ts, [&] { return parkCopOf(app, ts, cop); }))
                    return std::nullopt;
                if (byte(app, cop + kCarSide) & kSideRoadblock)
                {
                    if (!steerAtTarget(app, ts, cop) || !chooseLanes(app, ts, cop) || !feCore<0x40de30, frontend::sima::laneSteer>(app, ts, cop).has_value())
                        return std::nullopt;
                }
                else
                    step(kStepParked);
            }
            else
            {
                putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideRoadblock);
                if (!sideOfOrderOf(app, ts, cop) || !patrol(0, kSixBits))
                    return std::nullopt;
            }
        }
        const std::optional<Spot> spot = spotSpeederOf(app, ts, cop);
        if (!spot)
            return std::nullopt;
        // The last leaves its eax: what the step leaves.
        if (now == kStepParked && (!stopCar(app, ts, cop) || !stopCar(app, ts, cop) || !stopCarLeaving(app, ts, cop)))
            return std::nullopt;
        if (chasing())
        {
            const u8 side = byte(app, cop + kCarSide);
            step(kStepChase);
            putByte(app, cop + kCarSide, side & ~kSideRoadblock);
        }
        return now == kStepParked ? Drive{DriveEnd::Callee} : spotted(*spot);
    }
    case kStepLanes:
    {
        if (!steerAtTarget(app, ts, cop) || !setWantedSpeedOf(app, ts, cop, 0.0f, floatOf(kSixAndHalfBits)))
            return std::nullopt;
        voteRoadLaneOf(app, cop);
        guest::direct<0x40d720, aiplan::scoreObstacles>(app, ts, cop);
        if (ts.terminate || !guest::direct<0x40d800, aiplan::scoreCars>(app, ts, cop))
            return std::nullopt;
        guest::direct<0x40d9f0, aiplan::penaliseCloseCars>(app, ts);
        if (ts.terminate || !feCore<0x40de30, frontend::sima::laneSteer>(app, ts, cop).has_value())
            return std::nullopt;
        u32 result = 0;
        if (laneOffRoadOf(app, cop))
        {
            const u8 copKind = byte(app, cop + kCarCopKind);
            step(kStepOffRoad);
            result = 1 | u32(copKind) << 8;
            if (copKind & kCopRoadblock)
            {
                const u32 siren = at(app, cop + kCarCopIndex) * kSirenSize;
                result = at(app, siren + kSirens) != 0 ? siren : at(app, cop + kCarCopFlags) & 0xf;
                if (!roadblockSirenOf(app, ts, cop))
                    return std::nullopt;
            }
        }
        if (chasing())
            step(kStepChase);
        return value(result);
    }
    case kStepOffRoad:
    {
        put(app, cop + kCarWanted, 0);
        if (!stopCar(app, ts, cop))
            return std::nullopt;
        const std::optional<Spot> spot = spotSpeederOf(app, ts, cop);
        if (!spot)
            return std::nullopt;
        if (chasing())
            step(kStepChase);
        return spotted(*spot);
    }
    case kStepChase:
    {
        if (!steerAtTarget(app, ts, cop) || !sizeUpTargetOf(app, ts, cop) || !keepSideOfTargetOf(app, ts, cop)
            || !pursueTargetOf(app, ts, cop) || !noteSpeedingOf(app, ts, at(app, cop + kCarChased))
            || !chooseLanes(app, ts, cop))
            return std::nullopt;
        votePursuitLaneOf(app, ts, cop);
        if (!feCore<0x40de30, frontend::sima::laneSteer>(app, ts, cop).has_value())
            return std::nullopt;
        const std::optional<bool> stopped = targetStoppingOf(app, ts, cop);
        if (!stopped)
            return std::nullopt;
        if (*stopped)
        {
            step(kStepStop);
            Drive d{DriveEnd::StopsBy};
            d.choice = copStopsByChoice(app, cop);
            d.pursuit = at(app, cop + kCarPursuit);
            d.player = byte(app, cop + kCarFlags) & kFlagPlayer;
            const std::optional<StopBy> way = asCalled(ts, [&] { return copStopsByOf(app, ts, cop); });
            if (!way)
                return std::nullopt;
            d.stopsBy = *way;
            return d;
        }
        const u32 target = at(app, cop + kCarChased);
        if (target == 0)
        {
            step(kStepPatrol);
            return value(0);
        }
        if (isReversing(app, target))
        {
            if (!radioStoppedOf(app, ts, cop))
                return std::nullopt;
            return value(at(app, at(app, cop + kCarChased) + kCarPursuit));
        }
        if (gameSettings(app).copCount != 0)
            return value(0);
        const u32 pursuit = at(app, cop + kCarPursuit);
        if (i32(at(app, kClock) - at(app, pursuit + kPursuitRadioDue)) <= i32(kCallTicks))
            return value(pursuit);
        const u32 clock = at(app, kClock);
        const u32 caller = at(app, pursuit + kPursuitCaller);
        put(app, pursuit + kPursuitRadioDue, clock);
        if (cop == caller)
            put(app, pursuit + kPursuitCaller, 0);
        else
        {
            const std::optional<bool> roadblock = callRoadblockOf(app, ts, cop);
            if (!roadblock)
                return std::nullopt;
            if (*roadblock)
                return value(1);
            const std::optional<bool> twoCars = callTwoCarsOf(app, ts, cop);
            if (!twoCars)
                return std::nullopt;
            if (*twoCars)
                return value(1);
        }
        const std::optional<BackupCall> backup = callBackupOf(app, ts, cop);
        if (!backup)
            return std::nullopt;
        Drive d{DriveEnd::Backup};
        d.backup = *backup;
        return d;
    }
    case kStepStop:
    {
        put(app, cop + kCarWanted, 0);
        if (!stopCar(app, ts, cop))
            return std::nullopt;
        const std::optional<bool> over = stopOverOf(app, ts, cop);
        if (!over)
            return std::nullopt;
        if (*over)
            step(kStepPatrol);
        return value(*over ? 1 : 0);
    }
    }
    return value(now);
}

/* 0x4032e0: copDriverOf cop eax; eax as the code's last step leaves it. */
nfs::u32 copDriver(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<Drive> drive = copDriverOf(app, ts, cop);
    if (!drive)
        return args.eax;
    switch (drive->end)
    {
    case DriveEnd::Value:
        args.eax = drive->value;
        break;
    case DriveEnd::Status:
        args.eax = withStatus(drive->value);
        break;
    case DriveEnd::Callee:
        break;
    case DriveEnd::Spotted:
        args.eax = spotSpeederEax(app, ts, cop, drive->spot);
        break;
    case DriveEnd::StopsBy:
        args.eax = copStopsByEax(app, cop, drive->stopsBy, drive->choice, drive->pursuit, drive->player);
        break;
    case DriveEnd::Backup:
        args.eax = callBackupEax(app, cop, drive->backup);
        break;
    }
    ts.sp += 4;
    return args.eax;
}

/* copDriverOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool copDriverDone(App* app, ThreadState& ts, u32 cop)
{
    return copDriverOf(app, ts, cop).has_value();
}

/* What copHitByCar's code leaves in eax on each way out of copHitByCarOf: the cop (with the
 * compare's status word where the racer was slower), the clock, the cop's pursuit, 0, the join's
 * kind where the cop was on a pursuit, else as the callee last called (the radio) leaves it. */
u32 copHitEax(App* app, const ThreadState& ts, u32 cop, const HitOutcome& hit)
{
    switch (hit.way)
    {
    case CopHit::Nothing:
        return cop;
    case CopHit::Slower:
        return withStatus(cop);
    case CopHit::TooSoon:
        return at(app, kClock);
    case CopHit::Kept:
        return at(app, cop + kCarPursuit);
    case CopHit::NotWanted:
        return 0;
    case CopHit::Switched:
    case CopHit::Joined:
        return hit.joined ? hit.radioEax : kJoinHit;
    }
    return cop;  // (not reached: every way above returns)
}

/* 0x402cb0: copHitByCarOf cop eax, racer edx; eax as the code leaves it on each way out. */
nfs::u32 copHitByCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 cop = args.eax;
    const std::optional<HitOutcome> hit = copHitByCarOf(app, ts, cop, args.edx);
    if (!hit)
        return args.eax;
    args.eax = copHitEax(app, ts, cop, *hit);
    ts.sp += 4;
    return args.eax;
}

// What carContactOf did, for what its code leaves in eax.
struct Contact
{
    std::optional<HitOutcome> hit;  // copHitByCarOf's, where it was called
    bool sized = false;             // the ram's first test passed: al the other car's flags
    std::optional<u32> wait;      // the ram's wait asked (0x40f050)
    bool over = false;              // its hits past the rules': the wait from them
    bool timed = false;             // the hit's time set (+0x65c)
};

/* 0x404a00: car touched by the car at +0x16c: a cop (bit 0x20) hit hard (over 2, as bits) by a
 * racer, copHitByCarOf; a racer that rams (bit 8) touching a player's car in the race mode of
 * 0x6fd4f0 1 -- not where the race's switch 0x55eb2e has bit 0x20, its wait (+0x6e4) not over or
 * the other out -- its ram's wait (0x40f050, 100) over: its count of hits on that car (+0x6e8) one
 * more, past the rules' (0x56ea70 by its place) its wait from them, none against a racer that rams
 * in the race's modes 1 and 2; a car that times its hits (bit 2) the time it was hit until
 * (+0x65c): the force (fistp) by 16 from an out player's, by 7 (twice for a player's) from
 * another.  nullopt when the game stopped. */
std::optional<Contact> carContactOf(App* app, ThreadState& ts, u32 car)
{
    Contact contact;
    const u32 other = at(app, car + kCarHit);
    if ((byte(app, other + kCarFlags) & kFlagRacer) && (byte(app, car + kCarFlags) & kFlagFinished)
        && i32(at(app, car + kCarHitForce)) > i32(kHitBits))
    {
        contact.hit = copHitByCarOf(app, ts, car, other);
        if (!contact.hit)
            return std::nullopt;
    }
    bool ram = (byte(app, kRaceSwitches) & kRaceNoRams) == 0 && !(i32(at(app, car + kCarRamFor)) > 0);
    if (ram)
    {
        contact.sized = true;
        const u8 flags = byte(app, other + kCarFlags);
        ram = (flags & kFlagPlayer) && (byte(app, car + kCarFlags) & kFlagRams) && (flags & kFlagFinished) == 0
              && gameSettings(app).seriesRace == 1;
    }
    if (ram)
    {
        contact.wait = ramWait(app, ts, car, kRamAsking);
        if (!contact.wait)
            return std::nullopt;
        if (*contact.wait == 0)
        {
            put(app, car + kCarRamFor, 0);
            if (other != at(app, car + kCarChased))
            {
                put(app, car + kCarRammed, 0);
                put(app, car + kCarChased, other);
            }
            put(app, car + kCarRammed, at(app, car + kCarRammed) + 1);
            const u32 rules = at(app, car + kCarPlaceInRace) * kRammingSize + kRamming;
            if (i32(at(app, car + kCarRammed)) > i32(at(app, rules + 8)))
            {
                const u32 wait = at(app, rules + 0xc);
                put(app, car + kCarRammed, 0);
                put(app, car + kCarRamFor, wait);
                contact.over = true;
            }
            if (byte(app, other + kCarFlags) & kFlagRams)
            {
                const u32 race = gameSettings(app).raceType;
                if (race == 1 || race == 2)
                    put(app, car + kCarRamFor, 0);
            }
        }
    }
    if (byte(app, car + kCarFlags) & kFlagTimesHits)
    {
        const u8 flags = byte(app, other + kCarFlags);
        u32 force = u32(fp::toInteger<s32>(f32(app, car + kCarHitForce)));
        u32 by;
        if ((flags & kFlagFinished) && (flags & kFlagPlayer))
            by = force << 4;
        else
        {
            if (byte(app, other + kCarFlags) & kFlagPlayer)
                force += force;
            by = force * 7;
        }
        put(app, car + kCarHitUntil, at(app, kClock) + by);
        contact.timed = true;
    }
    return contact;
}

/* 0x404a00: carContactOf car eax.  eax as the code leaves it: the hit's time; the ram's wait (as
 * it leaves it), the rules' wait or the rules' offset; else copHitByCar's or the car, its al the
 * other car's flags where the ram's first test passed. */
nfs::u32 carContact(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    const u32 other = at(app, car + kCarHit);
    const std::optional<Contact> contact = carContactOf(app, ts, car);
    if (!contact)
        return args.eax;
    if (contact->timed)
        args.eax = at(app, car + kCarHitUntil);
    else if (contact->wait)
    {
        const u32 rules = at(app, car + kCarPlaceInRace) * kRammingSize;
        if (*contact->wait == 0)
            args.eax = contact->over ? at(app, rules + kRamming + 0xc) : rules;
    }
    else
    {
        u32 eax = contact->hit ? copHitEax(app, ts, car, *contact->hit) : car;
        if (contact->sized)
            eax = (eax & 0xffffff00) | byte(app, other + kCarFlags);
        args.eax = eax;
    }
    ts.sp += 4;
    return args.eax;
}

/* carContactOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool carContactDone(App* app, ThreadState& ts, u32 car)
{
    return carContactOf(app, ts, car).has_value();
}

}  // namespace airace
}  // namespace nfs3hp
