#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_frontend_sima.h"
#include "native_frontend_simb.h"
#include "native_frontend_simc.h"
#include "native_guest.h"
#include "native_pursuit.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* The police of a Hot Pursuit race, native whole.  disassemble_nfs3hp.py
 * binds each address to the function of that name here (AI_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code (ai_race.inc).
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
 * handed their address, GuestFrame) and an adapter under the game's name for
 * the callers still on registers: it takes the registers, calls the core and
 * leaves eax (edx, an x87 value) as the game's code does -- where that is a
 * leftover of the code's own (a status word fnstsw put in ax, a callee's
 * result, a loop's count), worked out from what the core says it did.  The
 * x87's arithmetic stays fp:: in the code's order of operands, each store to a
 * float a float.  The four switches go by the targets nfs3.exe's tables hold
 * (0x403018, 0x4032a4, 0x40466c, 0x40482c).  The other typed files' functions
 * (the opponents' driving's, the radio's, the track's places, the vectors') are
 * called as C++ through guest::direct, but where an adapter here leaves what
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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;

// The race's clock; the track run backwards; the track's nodes and their count.
constexpr reg32 kClock = 0x7d3684;
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The cops and their count; the racers and theirs; the pursuits; the orders.
constexpr reg32 kCops = 0x5efa88, kCopCount = 0x5efd88;
constexpr reg32 kRacers = 0x5efc48, kRacerCount = 0x5efda0;
constexpr reg32 kPursuits = 0x56e410, kOrders = 0x56ea10;
// The race's random numbers: the seed, the product, the multiplier.
constexpr reg32 kSeed = 0x554f10, kProduct = 0x554f14, kMultiplier = 0x554f18;
// What a cop sizes up of its target each tick (0x56ea58..0x56ea6b): the
// target turning, its side, the gap in nodes, the lane against the cop's,
// behind it; the lanes' votes.
constexpr reg32 kTurning = 0x56ea58, kSide = 0x56ea5c, kGap = 0x56ea60, kLaneVote = 0x56ea64,
                kBehind = 0x56ea68;
// The radio's messages by a cop's radio level (+0x71c); the tables by a
// car's tickets (16 bytes each: the gaps a cop gives up at, the stop's
// length, the next pursuit's wait); a cop's siren state (0x50 bytes each).
constexpr reg32 kRadioMessages = 0x55224c, kByTickets = 0x5521ac, kSirens = 0x7dd850;
// The ramming rules of each racer (0x14 bytes each).
constexpr reg32 kRamming = 0x56ea70;

// What is called by address: the game's, ea::yRotation (whose frame on the guest's stack sits
// under the angle the code pushes), the lane votes (0x40de30, its adapter's NFS_CAR_TRACE record);
// and the typed files' calls whose adapters here leave what the callee leaves in a register beyond
// what its header gives -- the nodes' gap's edx, the vectors' subtraction's eax and edx, the
// timer's, the stop's, the horn's end's, a node's and a ram's eax.  The other typed files'
// functions are called as C++ (guest::direct), the menu files' (the distance, a key, the sirens,
// a car settled) too (feCore), their eax put back where the code leaves it.
constexpr reg32 kNodeGap = 0x40e120;
constexpr reg32 kChop = 0x4dfd56, kYRotation = 0x4dfde0, kSubtract = 0x4e0000;
constexpr reg32 kCarRadio = 0x4127c0, kPlayerCopRadio = 0x412880;
constexpr reg32 kCopChaseSet = 0x411750;
constexpr reg32 kParkCar = 0x4a0980;
constexpr reg32 kSetTimer = 0x405fd0, kSteer = 0x40de30, kStop = 0x40ce70;
constexpr reg32 kAtNode = 0x40f000, kRamWait = 0x40f050, kRivalNear = 0x40deb0;

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated).  tools/game_checks does not
 * build those files: there it goes by address, through its ByAddress, as the game's code calls it,
 * every register but eax kept.  A reg32 result is the eax the function leaves: eax set to it (in the
 * game too); after any other, eax spoilt there, so that code still reading what the callee left in
 * eax differs there. */
template <reg32 Address, auto Core, typename... Args>
auto feCore(App* app, CPU& cpu, Args... args)
{
    const auto run = [&] {
#ifdef NFS_GAME_CHECKS
        const guest::KeptRegisters kept(cpu);
        return guest::ByAddress<Core>::template call<Address>(app, cpu, args...);
#else
        return guest::direct<Address, Core>(app, cpu, args...);
#endif
    };
    using Result = decltype(run());
    const auto spoil = [&] {
#ifdef NFS_GAME_CHECKS
        cpu.eax = 0x5b0115edu;
#endif
    };
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
            if constexpr (std::is_same_v<Result, x86::reg32>)
                cpu.eax = result;  // a menu core's result: the eax it leaves, as the call by address does
            else
                spoil();
            return cpu.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* A menu core's result (feCore's) put in eax, as its call by address left it: false when the game
 * stopped. */
bool leftInEax(CPU& cpu, const std::optional<reg32>& eax)
{
    if (!eax)
        return false;
    cpu.eax = *eax;
    return true;
}

reg16 word(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putWord(App* app, reg32 a, reg16 value) { app->getMemory<reg16>(a) = value; }

// A float or a double of the guest's as the x87 loads it.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

reg32 neg(reg32 v) { return reg32(0) - v; }

reg32 imul(reg32 a, reg32 b) { return reg32(std::int64_t(sreg32(a)) * std::int64_t(sreg32(b))); }

using i32 = std::int32_t;

// A car's fields (the cops' own among them; see the head of the file).
constexpr reg32 kCarNode = 0x1c, kCarFlags = 0x200, kCarHeading = 0x210, kCarDriver = 0x220,
                kCarTickets = 0x2c8, kCarState = 0x2c4, kCarAhead = 0x4f4, kCarHandling = 0x528,
                kCarPlayerSide = 0x5f4, kCarInput = 0x620, kCarSide = 0x621, kCarChased = 0x624,
                kCarPursuit = 0x628, kCarWatch = 0x62c, kCarLane = 0x638, kCarDriverState = 0x6e0,
                kCarSpeeding = 0x6f0, kCarCopFlags = 0x710, kCarCopKind = 0x711, kCarCopIndex = 0x714,
                kCarRadioLevel = 0x71c, kCarRadioDue = 0x724, kCarOrder = 0x7ac, kCarWanted = 0x7b0,
                kCarWantedSign = 0x7b3, kCarSpeed = 0x7c4, kCarSpotOn = 0x8e4, kCarSpotSet = 0x8e8;
// A car's flags (+0x200): a racer, a player's, one that rams; +0x621's other side of the road;
// +0x711's a fast cop.
constexpr reg8 kFlagRacer = 1, kFlagPlayer = 4, kFlagRams = 8, kFlagFinished = 0x20, kSideOther = 0x10,
               kCopFast = 1;
// A pursuit's fields: its state (none, a chase, pulled over), the stop's end, the cops on it, their cars.
constexpr reg32 kPursuitState = 0x1c, kPursuitStopEnds = 0x28, kPursuitCopCount = 0x2c, kPursuitCops = 0x30;
constexpr reg32 kNoPursuit = 0, kChase = 1, kPulledOver = 2;
// A handling record's top speed; a driver's model kind of the fast cops.
constexpr reg32 kHandlingTopSpeed = 0x1c8, kDriverModel = 0x14, kFastCopModel = 0x1f8;

// A car's watch of its input's bit 0 (+0x62c): free, held; the key asked with nothing held.
constexpr reg32 kWatchFree = 1, kWatchHeld = 2, kReleaseKey = 8;
constexpr reg32 kStoppingSpeed = 0x56ea50, kStoppingSpeedBits = 0x4132cfdf;  // a float, 11.18
constexpr reg32 kRadioOn = 0x56ea54;
constexpr i32 kTopRadioLevel = 8;
constexpr reg32 kFastCopScale = 0x536044;  // a double
constexpr i32 kFarGap = 10000;
constexpr reg32 kRadioSpeed = 0x53604c;  // a double: the least speed the radio calls about
constexpr reg32 kRadioCallTicks = 0x180;
constexpr reg8 kRaceCopRadio = 0x10;
constexpr i32 kStopGap = 10;
constexpr reg32 kStopTicks = 0x140;
// An order's kind, given at a node; a pursuit's counts of the cops sent (+4 + kind * 4).
constexpr reg8 kOrderAtNode = 2;
constexpr reg32 kOrderKind = 0, kOrderTaken = 1, kOrderNode = 2, kOrderWord = 4, kOrderSide = 6, kOrderFlag = 7;
constexpr reg32 kCarOrderFrom = 0x730;
// A pursuit's count of the cops that gave up, its roadblock's car; the cops' joining.
constexpr reg32 kPursuitGetaways = 0, kPursuitWorst = 4, kPursuitGaveUp = 0x14, kPursuitStarted = 0x20,
                kPursuitRadioDue = 0x24, kPursuitStoppedBy = 0x50, kPursuitCalls = 0x54, kPursuitRoadblock = 0x58,
                kPursuitCaller = 0x5c;
constexpr reg32 kCarJoin1 = 0x728, kCarJoin2 = 0x72c, kCarOnPursuit = 0x734;
constexpr reg8 kCopSent = 4, kCopRoadblock = 8, kCopChasing = 0x10;
constexpr reg32 kSirenSize = 0x50, kFirstRadioTicks = 0x280;
// joinPursuit's kinds: by the cop's own sighting, a player's cop.
constexpr reg32 kJoinSighted = 0, kJoinPlayerCop = 3;
constexpr reg32 kOutOfRace = 3, kOutOfPursuits = 4;  // a car's states (+0x2c4) of one out
constexpr reg32 kSpeedingBitsWanted = 0x40000000;   // 2.0's bits
constexpr i32 kPlayerCopReach = 20;
// What a cop sizes up of its target: turning, on its own side, behind; the lanes' vote.
constexpr reg32 kLaneLeft = 0, kLaneSame = 1, kLaneRight = 2;
constexpr reg32 kNotOnList = 8;  // more than a pursuit's list holds
constexpr reg8 kSideNoGetaway = 0x40;
constexpr reg8 kCopStopping = 0x80, kCopFlagsDropped = 0x6f;
constexpr reg32 kCarPlace = 0x98, kCarSpotPlace = 0x910, kCarSpotHeight = 0x914, kCarHeight = 0x118,
                kCarSpotSide = 0x190, kCarSpotTurn = 0x8ec;
// The heading's turn to radians (1/1024 of a turn) above 0, and from -0x200 on; the spot's quarter
// turn and offset, right (left negated).
constexpr reg32 kHeadingScale = 0x536054, kHeadingScaleLow = 0x53605c, kHeadingStartLow = 0x536064;
constexpr reg32 kSpotTurnBits = 0x3e4ccccd, kSpotTurnLeftBits = 0xbe4ccccd;  // 0.2, -0.2
constexpr reg32 kSpotOffsetBits = 0x40000000, kSpotOffsetLeftBits = 0xc0000000;  // 2, -2
constexpr reg32 kHalfTurn = 0x200;
constexpr reg32 kIdentity = 0x564498;   // a matrix, nine floats
constexpr reg32 kNodeScale = 0x53606c;  // a double: a node's fixed point to the world's
constexpr reg32 kNodeSize = 0x24, kCarFacing = 0x1a8;
constexpr reg32 kPlaceStill = 0x6c, kPlaceOnRoad = 0x14, kPlacePolygon = 0x18;  // a place on the track's
constexpr reg8 kPolygonKindMask = 0x3f, kPolygonNoStop = 0xe;
constexpr reg32 kCarTally = 0x2cc, kCarFines = 0x2d0, kCarWonAt = 0x2d4, kCarNextChase = 0x634,
                kCarStoppedBy = 0x9a4;
constexpr reg32 kTicketWait = kByTickets + 8, kTicketStop = kByTickets + 0xc;
constexpr reg32 kWonTicks = 0x200;
constexpr reg8 kSideKeep = 0xbf;
constexpr reg32 kFarBits = 0x461c4000;  // 10000
constexpr reg32 kPlayerCopStopTicks = 0xc0;
// The classes' table (24 bytes a class, by a model's +0x30): two values, the rule, the kind.
constexpr reg32 kModelClass = 0x30, kClassSize = 0x18, kClassValue = 0x55226c, kClassRule = 0x552274,
                kClassKind = 0x55227c, kClassValueToo = 0x552280;
constexpr reg32 kRammingSize = 0x14, kCarPlaceInRace = 0x1f0;
constexpr reg32 kRamGrip = 0x3f70a3d7;  // 0.94
constexpr reg32 kCarForward = 0x10c, kCarAirborne = 0x164;
constexpr reg32 kCarHit = 0x16c, kCarHitForce = 0x168;
constexpr reg32 kHardHitBits = 0x41700000;  // 15
constexpr reg32 kHitForce = 0x53619c, kClassSlowing = 0x5361a4;  // doubles
constexpr reg32 kCatchUpSpeed = 0x56ebb0;
constexpr reg32 kCatchUpFastBits = 0x41a0ee49, kCatchUpSlowBits = 0x4132cfdf;  // 20.12, 11.18
// The players' cars and their count; the other racers and theirs; the leader, and whether others
// have more laps.
constexpr reg32 kPlayers = 0x5efa48, kPlayerCount = 0x5efda4, kOthers = 0x5efb88, kOtherCount = 0x5efd94,
                kLeader = 0x56ebb8, kLeaderBehind = 0x56ebb4;
constexpr reg32 kCarLaps = 8, kCarPlayerIndex = 0x1f4;
constexpr reg8 kPursuitStoppedCall = 8;
constexpr reg8 kCopToldStop = 0x10;
constexpr reg32 kPursuitSize = 0x60, kPursuitsBytes = 0x600;
constexpr reg32 kNodeStride = 20, kSideOrder = 0x56f230, kSideOrderSet = 0x56f228;
constexpr reg32 kNodeLanes = 0x13, kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b;
constexpr reg32 kWidthScale = 0x5360dc, kLaneShare = 0x5360e4;  // doubles
constexpr reg32 kCarSideways = 0x7b4, kCarLaneOffset = 0x654, kCarSignedSpeed = 0x7c0;
constexpr reg32 kOneBits = 0x3f800000, kPatrolFactorBits = 0x3f9851ec;  // 1, 1.19
constexpr reg32 kRaceCars = 0x5efcc8, kRaceCarCount = 0x5efd9c, kCarWatched = 0x6d8;
constexpr reg32 kOncomingSpeed = 0x5360d4;  // a double
constexpr i32 kSideSwitchGap = 12;
constexpr reg32 kCopsWantedOver = 0x536104;  // a double: over the node's speed by it
constexpr reg32 kHitRatio = 0x5360ec;  // a double
constexpr reg32 kCarDriverStep = 0x70c, kParkedStep = 6;
constexpr reg32 kJoinHit = 1;
constexpr reg32 kVoteLeft = 0x56eeb4, kVoteOff = 0x56eeb8, kVoteRight = 0x56eebc, kVote = 0x64;
constexpr reg32 kParkReach = 0x5360f4;  // a double
constexpr reg32 kQuarterTurnBits = 0x3e800000, kThreeQuartersBits = 0x3f400000;  // 0.25, 0.75
constexpr reg32 kCarOrientation = 0xc0;
constexpr reg8 kSideRoadblock = 8;
constexpr reg32 kSirenNode = 0x34;
constexpr reg8 kCopPassed = 0x20;
constexpr i32 kPassedGap = -0x1e, kDoneGap = -0x32, kMissedGap = -0x14, kBehindGap = -5;
constexpr reg32 kWaitingStep = 0xa;
constexpr reg8 kOrderCutOff = 3;

constexpr reg32 kCarToldSpeed = 0x7d4;
// The chase's speeds: by the gap behind and ahead for a cop told to keep by it, the target's by
// the leader's and another's, the cop's slowest before its timer, the stop's speed (-9.83), the
// target's ahead and its difference by the gap (0x5360b4, a float).
constexpr reg32 kToldBehind = 0x53608c, kToldAhead = 0x536094, kLeaderShare = 0x53609c, kOtherShare = 0x5360a4,
                kTimerSpeed = 0x5360ac, kAheadOver = 0x5360bc, kAheadByGap = 0x5360b4;
constexpr reg32 kChaseLeader = 0x56ef54, kBrakeSpeedBits = 0xc11d5ac4;
constexpr i32 kTimerGap = 7, kTopGap = 8;
// The lanes' votes against (70) and for (10, 30), by the lane against the cop's.
constexpr reg32 kLaneAgainst = 0x56eed8, kLaneFor = 0x56ef2c;
constexpr reg32 kSlowTarget = 0x5360cc;  // a double
constexpr reg8 kCopHolds = 2;
constexpr i32 kFarAhead = 5;
constexpr reg32 kFirstCar = 0x5efac8, kCarMarked = 0x50d, kCarRamFor = 0x6e4, kCarRammed = 0x6e8;
constexpr reg32 kRival = 0x56ef3c, kRivalsBetween = 0x56ef48, kRivalSlower = 0x536194;  // a double
constexpr reg32 kRamTicks = 0xc350;
constexpr reg8 kInputNoLane = 8;
constexpr reg8 kRaceNoRams = 0x20;
constexpr reg32 kRaceSwitches = 0x55eb2e, kCarHitUntil = 0x65c;
constexpr reg32 kHitBits = 0x40000000;  // 2
constexpr reg8 kFlagTimesHits = 2;
constexpr reg32 kRamAsking = 0x64;

constexpr i32 kSpotReach = 0xf, kOrderReach = 5;
constexpr i32 kTimeToJoin = 0x3c0, kTimeAtOrder = 0x100;
constexpr reg32 kCarWaitedAt = 0x728, kCarWaitedFrom = 0x72c;
constexpr i32 kRoadblockAhead = 0x32, kRoadblockBefore = 6, kRoadblockHeat = 4;
constexpr reg8 kPursuitRoadblockCall = 2;
constexpr reg32 kGiveUpGap = kByTickets, kFastGiveUpGap = kByTickets + 4;
constexpr reg32 kNightShare = 0x5360c4;  // a double
constexpr reg32 kCarBraking = 0x660;
constexpr i32 kChaseGap = 0xf;
// A cop's order kinds and its driver's steps (+0x70c) for them.
constexpr reg8 kOrderToNode = 0, kOrderHold = 1, kOrderRoadblock = 3, kOrderSecondCar = 4;
constexpr reg32 kStepToNode = 1, kStepHold = 2, kStepAtNode = 3, kStepRoadblock = 7, kStepSecondCar = 8;
constexpr reg32 kStoppedReach = 0x536074, kStopReach = 0x53607c, kStopRolling = 0x536084;  // doubles
constexpr reg32 kCarRadioCallDue = 0x720, kCarThrottleNow = 0x508;
constexpr reg8 kCopCalledStop = 0x40;
constexpr reg32 kStopCallTicks = 0xc0, kStopCallSoon = 0xa0, kStopCallLate = 0xe0, kStopCallEarlier = 0x13;
constexpr i32 kLapsApart = 0x3c;
constexpr reg32 kBackupAlone = 0x280, kBackupReach = 0xc8;
constexpr reg8 kPursuitBackupCall = 1, kPursuitSecondCall = 4;
constexpr reg8 kCopFastKind = 1;

constexpr i32 kTwoCarsHeat = 3, kTwoCarsApart = 0x190;
constexpr reg8 kOrderTwoCars = 4;
constexpr reg16 kCopsBoxing = 0x410;
constexpr reg32 kStepNew = 0, kStepPatrol = 1, kStepToBlock = 7, kStepToSecond = 8, kStepAtSecond = 9,
                kStepParking = 0xa, kStepParked = 0xb, kStepLanes = 3, kStepOffRoad = 4, kStepChase = 5,
                kStepStop = 6;
constexpr reg32 kTwoBits = 0x40000000, kSixBits = 0x40c00000, kSixAndHalfBits = 0x40d00000;  // 2, 6, 6.5
constexpr reg32 kParkSpeed = 0x5360fc;  // a double
constexpr reg32 kSizingBytes = 0x14, kCallTicks = 0x140;
constexpr reg8 kCopToNode = 0x20;

reg32 floatBits(float f)
{
    reg32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

float floatAt(App* app, reg32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, reg32 a, float f) { app->getMemory<float>(a) = f; }

/* fcom's answers, with what they leave in the status word: a over b, not so for a NaN; a below b,
 * so for a NaN (jbe's and jb's after fnstsw ax; sahf). */
bool compareAbove(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return !cpu.fpu.status.c0 && !cpu.fpu.status.c3;
}

bool compareBelow(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.c0;
}

/* eax as fnstsw ax leaves it: the status word in its low half. */
reg32 withStatus(const CPU& cpu, reg32 eax) { return (eax & 0xffff0000) | cpu.fpu.status.word; }

/* What a function's frame keeps of its caller's across a call by address: every register but eax. */
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

/* 0x40e120 (aiplan::nodesBetween): the gap in nodes to `node` from `from`, signed; nullopt when
 * the game stopped.  Where `edx` asks for what it leaves in edx (it does not keep it), which
 * nodesBetween does not give, by address. */
std::optional<i32> nodeGap(App* app, CPU& cpu, reg32 node, reg32 from, reg32* edx = nullptr)
{
    if (!edx)
    {
        const i32 gap = guest::direct<0x40e120, aiplan::nodesBetween>(app, cpu, node, from);
        if (cpu.terminate)
            return std::nullopt;
        return gap;
    }
    const KeptRegisters kept(cpu);
    cpu.eax = node;
    cpu.edx = from;
    if (!byAddress(app, cpu, kNodeGap))
        return std::nullopt;
    *edx = cpu.edx;
    return i32(cpu.eax);
}

/* The gap's size as the code asks it: twice, the second negated where the first was not over 0;
 * the second's edx to `edx`, where asked. */
std::optional<i32> nodeGapSize(App* app, CPU& cpu, reg32 node, reg32 from, reg32* edx = nullptr)
{
    const std::optional<i32> first = nodeGap(app, cpu, node, from);
    if (!first)
        return std::nullopt;
    const std::optional<i32> second = nodeGap(app, cpu, node, from, edx);
    if (!second)
        return std::nullopt;
    return *first > 0 ? *second : i32(neg(reg32(*second)));
}

/* 0x40ed40 (aiplan::trafficSpeed): the speed the track asks at node; nullopt when the game
 * stopped. */
std::optional<Float> nodeSpeed(App* app, CPU& cpu, reg32 node)
{
    const float speed = guest::direct<0x40ed40, aiplan::trafficSpeed>(app, cpu, node);
    if (cpu.terminate)
        return std::nullopt;
    return Float(speed);
}

/* 0x4be1a0 (simc's): the state of key `key` of the input `input`, in eax too; nullopt when the
 * game stopped. */
std::optional<reg32> keyState(App* app, CPU& cpu, reg32 key, reg32 input)
{
    const std::optional<reg32> state = feCore<0x4be1a0, frontend::simc::offBeat>(app, cpu, key, input);
    if (!leftInEax(cpu, state))
        return std::nullopt;
    return state;
}

/* A call by address with the registers `set` puts in, eax left as the callee leaves it (a word
 * of the radio, a siren, a sound): false when the game stopped. */
template <typename Set>
bool callWith(App* app, CPU& cpu, reg32 target, Set set)
{
    const KeptRegisters kept(cpu);
    set();
    return byAddress(app, cpu, target);
}

/* An order's node and second word, each a signed word. */
reg32 orderNode(App* app, reg32 car) { return reg32(i32(at(app, at(app, car + kCarOrder))) >> 16); }
reg32 orderWord(App* app, reg32 car) { return reg32(i32(at(app, at(app, car + kCarOrder) + 2)) >> 16); }

/* 0x4c9f50 (simc's): cop `index`'s siren off, eax as it leaves it; false when the game stopped. */
bool sirenOff(App* app, CPU& cpu, reg32 index)
{
    return leftInEax(cpu, feCore<0x4c9f50, frontend::simc::sirenOff>(app, cpu, index));
}

/* __CHP: the value to an integer, towards 0, as a whole value; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<Float> chop(App* app, CPU& cpu, const Float& value)
{
    return Float(guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value)));
}


/* 0x4965b0 (sima's): the distance from the place at `from` to that at `to`, eax as it leaves it;
 * nullopt when the game stopped. */
std::optional<Float> distance(App* app, CPU& cpu, reg32 from, reg32 to)
{
    const std::optional<frontend::sima::FloatLeft> left =
        feCore<0x4965b0, frontend::sima::quickDistanceOf>(app, cpu, from, to);
    if (!left)
        return std::nullopt;
    cpu.eax = left->eax;
    return Float(left->value);
}

/* Its size as the code asks it: twice, the second negated where the first was not over 0. */
std::optional<Float> distanceSize(App* app, CPU& cpu, reg32 from, reg32 to)
{
    const std::optional<Float> first = distance(app, cpu, from, to);
    if (!first)
        return std::nullopt;
    const bool positive = compareBelow(cpu, 0.0, *first);
    const std::optional<Float> second = distance(app, cpu, from, to);
    if (!second)
        return std::nullopt;
    return positive ? *second : -*second;
}

/* 0x4e0000 (vertices::offsetVectorList<false>): `count` vectors at `vector` less those at
 * `other`, to `result`; false when the game stopped. */
bool subtractVectors(App* app, CPU& cpu, i32 count, reg32 vector, reg32 other, reg32 result)
{
    guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, count, vector, other, result);
    return !cpu.terminate;
}

/* 0x4e0000 by address, for pullOverSpot, which leaves the eax and edx it leaves (it does not keep
 * edx) -- which its header does not give: that edx, nullopt when the game stopped. */
std::optional<reg32> subtractVectorsLeavingEdx(App* app, CPU& cpu, reg32 count, reg32 vector, reg32 other,
                                               reg32 result)
{
    const KeptRegisters kept(cpu);
    cpu.eax = count;
    cpu.edx = vector;
    cpu.ebx = other;
    cpu.ecx = result;
    if (!byAddress(app, cpu, kSubtract))
        return std::nullopt;
    return cpu.edx;
}

/* 0x4e0050 (vertices::scaleVectorsBy): `count` vectors at `vector` scaled by `scale`, to `result`;
 * false when the game stopped. */
bool scaleVectors(App* app, CPU& cpu, i32 count, reg32 vector, float scale, reg32 result)
{
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, count, vector, scale, result);
    return !cpu.terminate;
}

/* 0x4e01f0 (vertices::dotProduct): the dot product of the vectors at a and b; nullopt when the game
 * stopped. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* 0x4dfde0: the matrix at `matrix` a turn about y by `angle` (its bits, on the stack); false
 * when the game stopped.  By address: ea::rotateAboutY reads the angle where the code pushes it,
 * and its frame on the guest's stack, under that word, is what it hands the sine's function. */
bool yRotation(App* app, CPU& cpu, reg32 matrix, reg32 angle)
{
    const KeptRegisters kept(cpu);
    cpu.eax = matrix;
    return guest::through(app, cpu, kYRotation, {angle});
}

/* 0x4205a0 (trackdata::snapPlace): the place at `place` snapped to the track (`followed`, about
 * node `node`, as the code has them in edx and ebx), its place on the track left at `onTrack`;
 * false when the game stopped. */
bool snapToTrack(App* app, CPU& cpu, reg32 place, reg32 followed, reg32 node, reg32 onTrack)
{
    return guest::direct<0x4205a0, trackdata::snapPlace>(app, cpu, place, onTrack, followed, node);
}

/* The frame of a core that hands its locals' addresses to callees, kept on the guest's stack at
 * the original's depth: `pushed` bytes of registers below the return address's slot, then `bytes`
 * of locals.  frame - n is the original's ebp - n. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    reg32 operator-(reg32 offset) const { return base_ - offset; }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

/* A core of this file called as the game's code calls it: the return address's slot below esp,
 * where its frame on the guest's stack goes below. */
template <typename Core>
auto asCalled(CPU& cpu, Core core)
{
    cpu.esp -= 4;
    const auto result = core();
    cpu.esp += 4;
    return result;
}

/* 0x405fd0 (aidrive::setReversing): car's driver's timer (`how`, `frames` its words); false when
 * the game stopped. */
bool setTimer(App* app, CPU& cpu, reg32 car, reg32 how, reg32 frames)
{
    guest::direct<0x405fd0, aidrive::setReversing>(app, cpu, car, how, frames);
    return !cpu.terminate;
}

/* setTimer by address, where an adapter leaves the eax it leaves, which its header does not
 * give. */
bool setTimerLeavingEax(App* app, CPU& cpu, reg32 car, reg32 how, reg32 frames)
{
    return callWith(app, cpu, kSetTimer, [&] {
        cpu.eax = car;
        cpu.edx = how;
        cpu.ebx = frames;
    });
}

/* 0x405220 (aidrive::jumpBehind): car's driver reset (`putBack` its mode); false when the game
 * stopped. */
bool resetDriver(App* app, CPU& cpu, reg32 car, bool putBack)
{
    return guest::direct<0x405220, aidrive::jumpBehind>(app, cpu, car, putBack).has_value();
}


float floatOf(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

/* The race's random numbers: the seed times the multiplier to 0x554f14, its low 16 bits the next
 * seed; the product's bits 8..23, read back. */
reg32 nextRandom(App* app)
{
    const reg32 product = imul(at(app, kSeed), at(app, kMultiplier));
    put(app, kProduct, product);
    put(app, kSeed, product & 0xffff);
    return (at(app, kProduct) & 0xffff00) >> 8;
}

/* 0x405c80 (aidrive::humanNearSliceOf): whether node is taken (a human's car near it, or 0);
 * nullopt when the game stopped. */
std::optional<reg32> nodeBlocked(App* app, CPU& cpu, reg32 node)
{
    return guest::direct<0x405c80, aidrive::humanNearSliceOf>(app, cpu, node);
}

/* 0x40e160 (aiplan::nodeAhead): the node `count` on from car's; nullopt when the game stopped. */
std::optional<reg32> nodeOn(App* app, CPU& cpu, reg32 car, reg32 count)
{
    const reg32 node = guest::direct<0x40e160, aiplan::nodeAhead>(app, cpu, car, count);
    if (cpu.terminate)
        return std::nullopt;
    return node;
}

/* 0x4e0640 (watcom's): memset of `count` bytes at `to` to `value`, eax as it leaves it (the
 * buffer); false when the game stopped. */
bool clearBytes(App* app, CPU& cpu, reg32 to, reg32 value, reg32 count)
{
    cpu.eax = guest::direct<0x4e0640, watcom::clib::memset>(app, cpu, to, value, count);
    return !cpu.terminate;
}


/* 0x4e0280 (vertices::multiplyMatrix): the matrix at `to` the product of those at `a` and `b`;
 * false when the game stopped. */
bool multiply(App* app, CPU& cpu, reg32 a, reg32 b, reg32 to)
{
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, a, b, to);
    return !cpu.terminate;
}

/* A call by address with cop in eax, the radio's words and the like; false when the game
 * stopped. */
bool callFor(App* app, CPU& cpu, reg32 target, reg32 car)
{
    return callWith(app, cpu, target, [&] { cpu.eax = car; });
}

/* A node gap the track's way: negated where it runs backwards. */
i32 alongTrack(App* app, i32 gap) { return gameSettings(app).backwards != 0 ? i32(neg(reg32(gap))) : gap; }

/* 0x40f050: what is left of car's ram's wait (edx its asking); nullopt when the game stopped.  By
 * address: rivalTick and carContact leave the eax it leaves, where its header's
 * aiplan::carInReach gives only whether. */
std::optional<reg32> ramWait(App* app, CPU& cpu, reg32 car, reg32 asking)
{
    const KeptRegisters kept(cpu);
    cpu.eax = car;
    cpu.edx = asking;
    if (!byAddress(app, cpu, kRamWait))
        return std::nullopt;
    return cpu.eax;
}



/* 0x40ce70 (aiplan::brakeToStopDone): car brought to a stop where it is pulled over; false when
 * the game stopped.  By address (callFor kStop) where an adapter leaves the eax it leaves, which
 * its header does not give. */
bool stopCar(App* app, CPU& cpu, reg32 car)
{
    return guest::direct<0x40ce70, aiplan::brakeToStopDone>(app, cpu, car);
}

/* 0x40dd60 (aiplan::beginPlanning), 0x40ddb0 (aiplan::pickLane): car steered at its target, its
 * lanes chosen; false when the game stopped. */
bool steerAtTarget(App* app, CPU& cpu, reg32 car)
{
    return guest::direct<0x40dd60, aiplan::beginPlanning>(app, cpu, car);
}

bool chooseLanes(App* app, CPU& cpu, reg32 car)
{
    return guest::direct<0x40ddb0, aiplan::pickLane>(app, cpu, car);
}

// The radio's message of a cop's level.
reg32 radioMessage(App* app, reg32 cop) { return at(app, at(app, cop + kCarRadioLevel) * 4 + kRadioMessages); }

}  // namespace

void nothingAtStart(App* app, CPU& cpu);
void nothingAtLoad(App* app, CPU& cpu);
void releaseEdge(App* app, CPU& cpu);
void setUpCops(App* app, CPU& cpu);
void radioOn(App* app, CPU& cpu);
void radioLevelDown(App* app, CPU& cpu);
void radioLevelUp(App* app, CPU& cpu);
void copTopSpeed(App* app, CPU& cpu);
void nearestCopGap(App* app, CPU& cpu);
void setWantedSpeed(App* app, CPU& cpu);
void noteSpeeding(App* app, CPU& cpu);
void pulledOver(App* app, CPU& cpu);
void copRadioCall(App* app, CPU& cpu);
void ticketRadio(App* app, CPU& cpu);
void stopIfCopNear(App* app, CPU& cpu);
void stopForCop(App* app, CPU& cpu);
void sizeUpTarget(App* app, CPU& cpu);
void joinPursuit(App* app, CPU& cpu);
void leaveCopList(App* app, CPU& cpu);
void giveUpPursuit(App* app, CPU& cpu);
void pullOverSpot(App* app, CPU& cpu);
void planPullOver(App* app, CPU& cpu);
void endPullOver(App* app, CPU& cpu);
void copStopsBy(App* app, CPU& cpu);
void stopOver(App* app, CPU& cpu);
void giveTicket(App* app, CPU& cpu);
void targetStopping(App* app, CPU& cpu);
void chaseSpeed(App* app, CPU& cpu);
void pursueTarget(App* app, CPU& cpu);
void votePursuitLane(App* app, CPU& cpu);
void sideTowardNode(App* app, CPU& cpu);
void keepSideOfTarget(App* app, CPU& cpu);
void placeCopAtStart(App* app, CPU& cpu);
void placeCop(App* app, CPU& cpu);
void alwaysOne(App* app, CPU& cpu);
void carReversing(App* app, CPU& cpu);
void copHitByCar(App* app, CPU& cpu);
void voteRoadLane(App* app, CPU& cpu);
void spotSpeeder(App* app, CPU& cpu);
void laneOffRoad(App* app, CPU& cpu);
void atOrderNode(App* app, CPU& cpu);
void sideOfOrder(App* app, CPU& cpu);
void takeOrder(App* app, CPU& cpu);
void parkCop(App* app, CPU& cpu);
void drivePatrol(App* app, CPU& cpu);
void copDriver(App* app, CPU& cpu);
void giveOrder(App* app, CPU& cpu);
void spreadCops(App* app, CPU& cpu);
void radioStopped(App* app, CPU& cpu);
void radioStop(App* app, CPU& cpu);
void initPursuits(App* app, CPU& cpu);
void callRoadblock(App* app, CPU& cpu);
void callTwoCars(App* app, CPU& cpu);
void roadblockSiren(App* app, CPU& cpu);
void watchRoadblock(App* app, CPU& cpu);
void watchSecondCar(App* app, CPU& cpu);
void copsWanted(App* app, CPU& cpu);
void radioJoined(App* app, CPU& cpu);
void radioGiveUp(App* app, CPU& cpu);
void releaseCops(App* app, CPU& cpu);
void dropCop(App* app, CPU& cpu);
void sendCopAhead(App* app, CPU& cpu);
void callBackup(App* app, CPU& cpu);
void orderTaken(App* app, CPU& cpu);
void nothingForCop(App* app, CPU& cpu);
void classRule(App* app, CPU& cpu);
void nothingAtEnd(App* app, CPU& cpu);
void nothingAtEndToo(App* app, CPU& cpu);
void setRammingRules(App* app, CPU& cpu);
void classValue(App* app, CPU& cpu);
void classValueToo(App* app, CPU& cpu);
void rivalTick(App* app, CPU& cpu);
void carContact(App* app, CPU& cpu);
void slowForClass(App* app, CPU& cpu);
void nothingAtStartToo(App* app, CPU& cpu);
void catchUpSpeed(App* app, CPU& cpu);
void findLeader(App* app, CPU& cpu);

namespace
{

/* push ebp; mov ebp, esp; pop ebp; ret: nothing seen but the return. */
void emptyHook(App*, CPU& cpu)
{
    cpu.esp += 4;
}

}  // namespace

/* 0x401220, 0x401230, 0x404580, 0x404650, 0x404c40: empty hooks of the race's start and load and
 * of the cops' -- push ebp, mov ebp, esp, pop ebp: nothing but the return. */
void nothingAtStart(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

void nothingAtLoad(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x401240: whether car's input bit 0 (+0x620) was let go: so when it has gone off since it was
 * held (the watch back from held to free), not while it is on (the watch held); with it off and
 * nothing held, so when key 8 of the car's input ([+0]) is up (0x4be1a0 says 0).  nullopt when the
 * game stopped. */
std::optional<bool> wasReleased(App* app, CPU& cpu, reg32 car)
{
    const reg32 watch = at(app, car + kCarWatch);
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
    const std::optional<reg32> key = keyState(app, cpu, kReleaseKey, at(app, car));
    if (!key)
        return std::nullopt;
    return *key == 0;
}

/* 0x401240: wasReleased for car eax, 1 or 0 in eax. */
void releaseEdge(App* app, CPU& cpu)
{
    const std::optional<bool> released = wasReleased(app, cpu, cpu.eax);
    if (!released)
        return;
    cpu.eax = *released ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4012c0: the race's cops set up: each its index (+0x714) and its order (+0x7ac, 0x56ea10 +
 * index * 8), the fast ones (their driver's model +0x14 0x1f8) marked (+0x711 bit 1); the
 * stopping speed (0x56ea50) 11.18.  The index the count stopped at: the count, 0 for none. */
reg32 numberCops(App* app)
{
    reg32 i = 0;
    for (; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 cop = at(app, i * 4 + kCops);
        put(app, cop + kCarCopIndex, i);
        const reg32 driver = at(app, cop + kCarDriver);
        put(app, cop + kCarOrder, kOrders + i * 8);
        if (at(app, driver + kDriverModel) == kFastCopModel)
            putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopFast);
    }
    put(app, kStoppingSpeed, kStoppingSpeedBits);
    return i;
}

/* 0x4012c0: numberCops, the count it stopped at in eax. */
void setUpCops(App* app, CPU& cpu)
{
    cpu.eax = numberCops(app);
    cpu.esp += 4;
}

/* 0x401320: the cops' radio on (0x56ea54). */
void radioOn(App* app, CPU& cpu)
{
    put(app, kRadioOn, 1);
    cpu.esp += 4;
}

/* 0x401330, 0x401350: cop's radio level (+0x71c) one down, to 0; one up, to 8. */
void lowerRadioLevel(App* app, reg32 cop)
{
    const reg32 level = at(app, cop + kCarRadioLevel);
    if (i32(level) > 0)
        put(app, cop + kCarRadioLevel, level - 1);
}

void raiseRadioLevel(App* app, reg32 cop)
{
    const reg32 level = at(app, cop + kCarRadioLevel);
    if (level < reg32(kTopRadioLevel))
        put(app, cop + kCarRadioLevel, level + 1);
}

/* 0x401330, 0x401350: for cop eax. */
void radioLevelDown(App* app, CPU& cpu)
{
    lowerRadioLevel(app, cpu.eax);
    cpu.esp += 4;
}

void radioLevelUp(App* app, CPU& cpu)
{
    raiseRadioLevel(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x401370: cop's top speed: its handling's (+0x528's +0x1c8), scaled by 0x536044 for a fast
 * one. */
float copTopSpeedOf(App* app, reg32 cop)
{
    const reg32 handling = at(app, cop + kCarHandling);
    if (byte(app, cop + kCarCopKind) & kCopFast)
        return float(fp::mul(f32(app, handling + kHandlingTopSpeed), f64(app, kFastCopScale)));
    return floatAt(app, handling + kHandlingTopSpeed);
}

/* What the code of copTopSpeed leaves in eax: the handling for a fast cop, the speed's bits for
 * another. */
reg32 copTopSpeedEax(App* app, reg32 cop)
{
    const reg32 handling = at(app, cop + kCarHandling);
    return byte(app, cop + kCarCopKind) & kCopFast ? handling : at(app, handling + kHandlingTopSpeed);
}

/* 0x401370: copTopSpeedOf for cop eax onto the x87's stack. */
void copTopSpeed(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const float top = copTopSpeedOf(app, cop);
    cpu.eax = copTopSpeedEax(app, cop);
    cpu.fpu.push(Float(top));
    cpu.esp += 4;
}

/* 0x4013b0: the gap in nodes from car to the nearest cop, 10000 at most; nullopt when the game
 * stopped. */
std::optional<i32> nearestCopGapOf(App* app, CPU& cpu, reg32 car)
{
    i32 nearest = kFarGap;
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 cop = at(app, i * 4 + kCops);
        const std::optional<i32> gap = nodeGapSize(app, cpu, at(app, cop + kCarNode), at(app, car + kCarNode));
        if (!gap)
            return std::nullopt;
        if (*gap < nearest)
            nearest = *gap;
    }
    return nearest;
}

/* 0x4013b0: nearestCopGapOf car eax, in eax. */
void nearestCopGap(App* app, CPU& cpu)
{
    const std::optional<i32> gap = nearestCopGapOf(app, cpu, cpu.eax);
    if (!gap)
        return;
    cpu.eax = reg32(*gap);
    cpu.esp += 4;
}

/* The flips of car's wanted speed's sign (+0x7b3's top bit) the cops' code does after setting it:
 * once for the track run backwards, once for the car on the other side of the road (+0x621 bit
 * 0x10). */
void flipWantedSign(App* app, reg32 car)
{
    if (gameSettings(app).backwards != 0)
        putByte(app, car + kCarWantedSign, byte(app, car + kCarWantedSign) ^ 0x80);
    if (byte(app, car + kCarSide) & kSideOther)
        putByte(app, car + kCarWantedSign, byte(app, car + kCarWantedSign) ^ 0x80);
}

/* The byte the first of those flips leaves in its byte register, from the sign byte after both. */
reg8 firstFlip(App* app, reg32 car)
{
    const reg8 sign = byte(app, car + kCarWantedSign);
    return byte(app, car + kCarSide) & kSideOther ? reg8(sign ^ 0x80) : sign;
}

/* 0x401430: cop's wanted speed (+0x7b0): its node's speed times `factor` plus `offset`, its top
 * speed at most (copTopSpeedOf), its sign flipped as flipWantedSign says.  Whether the top speed
 * capped it; nullopt when the game stopped. */
std::optional<bool> setWantedSpeedOf(App* app, CPU& cpu, reg32 cop, float factor, float offset)
{
    const std::optional<Float> speed = nodeSpeed(app, cpu, at(app, cop + kCarNode));
    if (!speed)
        return std::nullopt;
    putFloat(app, cop + kCarWanted, float(fp::add(fp::mul(*speed, Float(factor)), Float(offset))));
    const float top = copTopSpeedOf(app, cop);
    const bool capped = compareAbove(cpu, f32(app, cop + kCarWanted), Float(top));
    if (capped)
        putFloat(app, cop + kCarWanted, top);
    flipWantedSign(app, cop);
    return capped;
}

/* Car's speeding (+0x6f0) kept at its worst: `speed` less its node's speed, if more.  False when
 * the game stopped. */
bool keepWorstSpeeding(App* app, CPU& cpu, reg32 car, float speed)
{
    const std::optional<Float> limit = nodeSpeed(app, cpu, at(app, car + kCarNode));
    if (!limit)
        return false;
    const float over = float(fp::sub(Float(speed), *limit));
    const bool worse = !compareAbove(cpu, f32(app, car + kCarSpeeding), Float(over));
    put(app, car + kCarSpeeding, worse ? floatBits(over) : at(app, car + kCarSpeeding));
    return true;
}

/* 0x4014b0: car's speeding kept at its worst against its speed (+0x7c4); for no car nothing.
 * False when the game stopped. */
bool noteSpeedingOf(App* app, CPU& cpu, reg32 car)
{
    return car == 0 || keepWorstSpeeding(app, cpu, car, floatAt(app, car + kCarSpeed));
}

/* 0x4014b0: noteSpeedingOf car eax; eax the speeding kept, 0 for no car. */
void noteSpeeding(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!noteSpeedingOf(app, cpu, car))
        return;
    cpu.eax = car != 0 ? at(app, car + kCarSpeeding) : 0;
    cpu.esp += 4;
}

/* 0x401500: whether car's pursuit has it pulled over. */
bool isPulledOver(App* app, reg32 car)
{
    return at(app, at(app, car + kCarPursuit) + kPursuitState) == kPulledOver;
}

/* 0x401500: isPulledOver car eax, 1 or 0 in eax. */
void pulledOver(App* app, CPU& cpu)
{
    cpu.eax = isPulledOver(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

// How a cop's call about a car went: no radio in the race, the car pulled over, too slow, the
// last call not past, called.
enum class RadioCall { NoRadio, PulledOver, TooSlow, TooSoon, Called };

/* 0x401520: cop's call on the radio about car (snd::carRadio, the message of its radio level),
 * where the race has the cops' radio (0x6fd4fc bit 0x10), the car is not pulled over and goes at
 * 0x53604c or more, and the cop's last call (+0x724) is past; the next 0x180 ticks on.  nullopt
 * when the game stopped. */
std::optional<RadioCall> callAboutCar(App* app, CPU& cpu, reg32 cop, reg32 car)
{
    if (!(std::uint8_t(gameSettings(app).raceFlags) & kRaceCopRadio))
        return RadioCall::NoRadio;
    if (at(app, at(app, car + kCarPursuit) + kPursuitState) == kPulledOver)
        return RadioCall::PulledOver;
    if (compareBelow(cpu, f32(app, car + kCarSpeed), f64(app, kRadioSpeed)))
        return RadioCall::TooSlow;
    if (!(i32(at(app, kClock)) > i32(at(app, cop + kCarRadioDue))))
        return RadioCall::TooSoon;
    if (!callWith(app, cpu, kCarRadio, [&] {
            cpu.eax = cop;
            cpu.edx = radioMessage(app, cop);
        }))
        return std::nullopt;
    put(app, cop + kCarRadioDue, at(app, kClock) + kRadioCallTicks);
    return RadioCall::Called;
}

/* 0x401520: callAboutCar for cop eax and car edx; eax and edx as the code leaves them on each way
 * out. */
void copRadioCall(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax, car = cpu.edx;
    const std::optional<RadioCall> call = callAboutCar(app, cpu, cop, car);
    if (!call)
        return;
    switch (*call)
    {
    case RadioCall::NoRadio:
        break;
    case RadioCall::PulledOver:
        cpu.eax = at(app, car + kCarPursuit);
        break;
    case RadioCall::TooSlow:
        cpu.eax = withStatus(cpu, at(app, car + kCarPursuit));
        break;
    case RadioCall::TooSoon:
        cpu.eax = at(app, kClock);
        break;
    case RadioCall::Called:
        cpu.eax = at(app, cop + kCarRadioDue);
        cpu.edx = radioMessage(app, cop);
        break;
    }
    cpu.esp += 4;
}

/* 0x401580: cop's radio to the player of car about its ticket (snd::playerCopRadio): the message
 * of its radio level, the tickets the car has left ([0x6fd4bc] less +0x2c8).  False when the game
 * stopped. */
bool radioTicket(App* app, CPU& cpu, reg32 cop, reg32 car)
{
    const reg32 left = gameSettings(app).pursuitOutCount - at(app, car + kCarTickets);
    return callWith(app, cpu, kPlayerCopRadio, [&] {
        cpu.eax = cop;
        cpu.edx = radioMessage(app, cop);
        cpu.ebx = left;
    });
}

/* 0x401580: radioTicket for cop eax and car edx; eax as the radio leaves it, edx its message. */
void ticketRadio(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!radioTicket(app, cpu, cop, cpu.edx))
        return;
    cpu.edx = radioMessage(app, cop);
    cpu.esp += 4;
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
std::optional<LastGap> stopIfCopNearOf(App* app, CPU& cpu, reg32 car)
{
    LastGap last;
    const reg32 pursuit = at(app, car + kCarPursuit);
    if (pursuit == 0 || at(app, pursuit + kPursuitState) == kNoPursuit || (byte(app, car + kCarFlags) & kFlagFinished))
        return last;
    for (reg32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
    {
        const reg32 cop = at(app, pursuit + i * 4 + kPursuitCops);
        const std::optional<i32> gap = nodeGapSize(app, cpu, at(app, car + kCarNode), at(app, cop + kCarNode));
        if (!gap)
            return std::nullopt;
        last = {true, *gap};
        if (*gap < kStopGap)
        {
            const reg32 now = at(app, kClock);
            put(app, pursuit + kPursuitState, kPulledOver);
            put(app, pursuit + kPursuitStopEnds, now + kStopTicks);
            break;
        }
    }
    return last;
}

/* 0x4015b0: stopIfCopNearOf car eax; eax as the code leaves it: the car where no cop was looked at,
 * the stop's end where one stopped it, else the last gap. */
void stopIfCopNear(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const std::optional<LastGap> last = stopIfCopNearOf(app, cpu, car);
    if (!last)
        return;
    if (last->any)
        cpu.eax = last->gap < kStopGap ? at(app, at(app, car + kCarPursuit) + kPursuitStopEnds) : reg32(last->gap);
    cpu.esp += 4;
}

/* 0x403830: car (not a player's) given an order -- its kind, node, second word, side and flag --
 * from car `from` (+0x730), not yet taken. */
void giveOrderOf(App* app, reg32 car, reg32 from, reg8 kind, reg16 node, reg16 word2, reg8 side, reg8 flag)
{
    const reg32 order = at(app, car + kCarOrder);
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
 * flag byte; ret 0xc.  eax the order; edx, for a car given it, the word's with the flag byte. */
void giveOrder(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const reg32 order = at(app, car + kCarOrder);
    const bool player = byte(app, car + kCarFlags) & kFlagPlayer;
    const reg32 word2 = at(app, cpu.esp + 4);
    const reg8 side = byte(app, cpu.esp + 8), flag = byte(app, cpu.esp + 0xc);
    giveOrderOf(app, car, cpu.edx, cpu.bl, cpu.cx, reg16(word2), side, flag);
    cpu.eax = order;
    if (!player)
        cpu.edx = (word2 & 0xffffff00) | flag;
    cpu.esp += 4 + 0xc;
}

/* 0x404070: cop's word on the radio as it joins (0x40b220; 0x40b530 for a car that got away
 * before, from a cop not a player's), or as a second (0x40b330).  What the radio's code leaves in
 * eax; nullopt when the game stopped. */
std::optional<reg32> radioJoinedOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 chase = at(app, cop + kCarPursuit);
    if (at(app, chase + kPursuitCopCount) != 1)
        return guest::direct<0x40b330, pursuit::radioPursuitOf>(app, cpu, cop);
    if (at(app, chase + kPursuitGaveUp) != 0 && (byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return guest::direct<0x40b530, pursuit::radioBackupOf>(app, cpu, cop);
    return guest::direct<0x40b220, pursuit::radioSpottedOf>(app, cpu, cop);
}

/* 0x404070: radioJoinedOf cop eax; eax as the radio leaves it. */
void radioJoined(App* app, CPU& cpu)
{
    const std::optional<reg32> eax = radioJoinedOf(app, cpu, cpu.eax);
    if (!eax)
        return;
    cpu.eax = *eax;
    cpu.esp += 4;
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
    reg32 radioEax;
};

std::optional<Joining> joinPursuitWith(App* app, CPU& cpu, reg32 kind, reg32 cop, reg32 car, float speed)
{
    if (at(app, cop + kCarPursuit) != 0)
        return Joining{false, 0};
    put(app, cop + kCarJoin1, 0);
    const reg8 flags = byte(app, cop + kCarFlags);
    put(app, cop + kCarJoin2, 0);
    if ((flags & kFlagPlayer) == 0 && at(app, cop + kCarOrderFrom) != 0)
    {
        if (byte(app, cop + kCarCopKind) & kCopSent)
            putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopSent);
        const reg8 copKind = byte(app, cop + kCarCopKind);
        if (copKind & kCopRoadblock)
        {
            putByte(app, cop + kCarCopKind, copKind & ~kCopRoadblock);
            const reg32 from = at(app, cop + kCarOrderFrom);
            const reg32 fromPursuit = from != 0 ? at(app, from + kCarPursuit) : 0;
            if (fromPursuit != 0)
            {
                put(app, fromPursuit + kPursuitRoadblock, 0);
                const reg32 index = at(app, cop + kCarCopIndex);
                if (at(app, index * kSirenSize + kSirens) != 0 && !sirenOff(app, cpu, at(app, cop + kCarCopIndex)))
                    return std::nullopt;
            }
        }
    }
    const reg32 pursuit = at(app, car + kCarPursuit);
    if (at(app, pursuit + kPursuitState) == kNoPursuit)
    {
        put(app, pursuit + kPursuitState, kChase);
        put(app, pursuit + kPursuitStopEnds, 0);
        put(app, pursuit + kPursuitCopCount, 0);
        put(app, pursuit + kPursuitStoppedBy, 0);
        put(app, pursuit + kPursuitCalls, 0);
        const reg32 now = at(app, kClock);
        put(app, pursuit + kPursuitCaller, 0);
        put(app, pursuit + kPursuitStarted, now);
        put(app, pursuit + kPursuitRadioDue, now + kFirstRadioTicks);
    }
    put(app, cop + kCarOnPursuit, 1);
    put(app, cop + kCarPursuit, pursuit);
    put(app, cop + kCarChased, car);
    putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) | kCopChasing);
    const reg32 count = at(app, pursuit + kPursuitCopCount);
    put(app, pursuit + count * 4 + kPursuitCops, cop);
    put(app, pursuit + kPursuitCopCount, at(app, pursuit + kPursuitCopCount) + 1);
    if (kind == kJoinSighted)
    {
        if (!keepWorstSpeeding(app, cpu, car, speed))
            return std::nullopt;
    }
    else if (kind == kJoinPlayerCop)
        return Joining{true, 0};
    else
        put(app, pursuit + kind * 4 + 4, at(app, pursuit + kind * 4 + 4) + 1);
    const std::optional<reg32> radio = radioJoinedOf(app, cpu, cop);
    if (!radio)
        return std::nullopt;
    return Joining{true, *radio};
}

/* joinPursuitWith's whether it joined (native_ai_race.h). */
std::optional<bool> joinPursuitOf(App* app, CPU& cpu, reg32 kind, reg32 cop, reg32 car, float speed)
{
    const std::optional<Joining> joining = joinPursuitWith(app, cpu, kind, cop, car, speed);
    if (!joining)
        return std::nullopt;
    return joining->joined;
}

/* 0x4018b0: joinPursuitWith kind eax, cop edx, car ebx, the speed on the stack; ret 4.  eax the
 * kind where the cop was on a pursuit, the pursuit for a player's cop, else as the radio leaves
 * it. */
void joinPursuit(App* app, CPU& cpu)
{
    const reg32 kind = cpu.eax, car = cpu.ebx;
    const std::optional<Joining> joining = joinPursuitWith(app, cpu, kind, cpu.edx, car, floatAt(app, cpu.esp + 4));
    if (!joining)
        return;
    if (joining->joined)
        cpu.eax = kind == kJoinPlayerCop ? at(app, car + kCarPursuit) : joining->radioEax;
    cpu.esp += 4 + 4;
}

// How a racer's stop for a cop went: nothing more, the cop chasing already, the cop joined, the
// player's cop too far.
enum class StopWay { Nothing, Chasing, Joined, TooFar };

struct CopStop
{
    StopWay way;
    reg32 kind = 0;       // the join's kind
    bool joined = false;  // joinPursuitOf's
    i32 gap = 0;          // the player's cop's
    reg32 edx = 0;        // what the gap's function left in edx, for the adapter
};

/* 0x401650: racer stops for cop `index`: the cop's roadblock mark cleared; the racer's pursuit
 * pulled over, unless the racer is out (+0x2c4 3 or 4); a cop chasing no one joins (joinPursuitOf,
 * kind 0 -- for a player's cop kind 3 and only within 20 nodes) where the racer is speeding or
 * chased; the stop's end 0x140 ticks on if any cop is on it.  nullopt when the game stopped. */
std::optional<CopStop> stopForCopOf(App* app, CPU& cpu, reg32 racer, reg32 index)
{
    const reg32 cop = at(app, index * 4 + kCops);
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopRoadblock);
    if ((byte(app, racer + kCarFlags) & kFlagRacer) == 0)
        return CopStop{StopWay::Nothing};
    const reg32 pursuit = at(app, racer + kCarPursuit);
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
            const std::optional<bool> joined = joinPursuitOf(app, cpu, kJoinSighted, cop, racer, 0.0f);
            if (!joined)
                return std::nullopt;
            stop = {StopWay::Joined, kJoinSighted, *joined};
        }
        stopEnds();
        return stop;
    }
    if (!wanted && at(app, pursuit + kPursuitGaveUp) == 0)
        return CopStop{StopWay::Nothing};
    if (at(app, cop + kCarChased) != 0)
        return CopStop{StopWay::Nothing};
    reg32 edx = 0;
    const std::optional<i32> gap = nodeGapSize(app, cpu, at(app, racer + kCarNode), at(app, cop + kCarNode), &edx);
    if (!gap)
        return std::nullopt;
    if (*gap >= kPlayerCopReach)
        return CopStop{StopWay::TooFar, 0, false, *gap, edx};
    const std::optional<bool> joined = joinPursuitOf(app, cpu, kJoinPlayerCop, cop, racer, 0.0f);
    if (!joined)
        return std::nullopt;
    stopEnds();
    return CopStop{StopWay::Joined, kJoinPlayerCop, *joined};
}

/* 0x401650: stopForCopOf racer eax, cop index edx; eax and edx as the code leaves them on each way
 * out. */
void stopForCop(App* app, CPU& cpu)
{
    const reg32 racer = cpu.eax, index = cpu.edx;
    const reg32 cop = at(app, index * 4 + kCops);
    const std::optional<CopStop> stop = stopForCopOf(app, cpu, racer, index);
    if (!stop)
        return;
    switch (stop->way)
    {
    case StopWay::Nothing:
        break;
    case StopWay::TooFar:
        cpu.eax = reg32(stop->gap);
        cpu.edx = stop->edx;
        break;
    case StopWay::Chasing:
        cpu.eax = at(app, cop + kCarChased);
        break;
    case StopWay::Joined:
        cpu.edx = cop;
        if (!stop->joined)
            cpu.eax = stop->kind;
        else if (stop->kind == kJoinPlayerCop)
            cpu.eax = at(app, racer + kCarPursuit);
        break;
    }
    const reg32 pursuit = at(app, racer + kCarPursuit);
    if ((stop->way == StopWay::Chasing || stop->way == StopWay::Joined) && at(app, pursuit + kPursuitCopCount) != 0)
        cpu.eax = at(app, pursuit + kPursuitStopEnds);
    cpu.esp += 4;
}

/* stopForCopOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool stopForCopDone(App* app, CPU& cpu, reg32 racer, reg32 index)
{
    return stopForCopOf(app, cpu, racer, index).has_value();
}

/* 0x401780: cop sizes up the car it chases, for this tick's decisions: the car turning one way
 * (0x56ea58, its +0x4f4 over 0), on its own side of the road (0x56ea5c: a player's by +0x5f4,
 * another by +0x621 bit 0x10, against the track's way), the gap to it in nodes, signed by the
 * cop's side (0x56ea60), whether it is behind (0x56ea68), its lane against the cop's (0x56ea64: 0
 * left of it, 1 the same, 2 right).  Whether it chases one; nullopt when the game stopped. */
std::optional<bool> sizeUpTargetOf(App* app, CPU& cpu, reg32 cop)
{
    reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    put(app, kTurning, compareBelow(cpu, 0.0, f32(app, target + kCarAhead)) ? 1 : 0);
    target = at(app, cop + kCarChased);
    reg32 side = byte(app, target + kCarFlags) & kFlagPlayer ? (at(app, target + kCarPlayerSide) == 0 ? 1 : 0)
                                                             : ((byte(app, target + kCarSide) & kSideOther) == 0 ? 1 : 0);
    side ^= gameSettings(app).backwards;
    put(app, kSide, side != 0 ? 1 : 0);
    const std::optional<i32> gap = nodeGap(app, cpu, at(app, at(app, cop + kCarChased) + kCarNode), at(app, cop + kCarNode));
    if (!gap)
        return std::nullopt;
    const reg8 copSide = byte(app, cop + kCarSide);
    put(app, kGap, reg32(*gap));
    const reg32 way = (((copSide & kSideOther) == 0 ? 1u : 0u) ^ gameSettings(app).backwards) != 0 ? 1 : 0xffffffff;
    const reg32 signedGap = imul(at(app, kGap), way);
    put(app, kGap, signedGap);
    put(app, kBehind, i32(signedGap) < 0 ? 1 : 0);
    const i32 ownLane = i32(at(app, cop + kCarLane));
    const i32 targetLane = i32(at(app, at(app, cop + kCarChased) + kCarLane));
    put(app, kLaneVote, targetLane < ownLane ? kLaneLeft : targetLane == ownLane ? kLaneSame : kLaneRight);
    return true;
}

/* 0x401780: sizeUpTargetOf cop eax; eax the target where there is one. */
void sizeUpTarget(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<bool> chasing = sizeUpTargetOf(app, cpu, cop);
    if (!chasing)
        return;
    if (*chasing)
        cpu.eax = at(app, cop + kCarChased);
    cpu.esp += 4;
}

/* 0x401a20: cop taken off its pursuit's list: those after it moved down one, the count one less
 * (whether it was there or not), the last slot cleared. */
void takeOffList(App* app, reg32 cop)
{
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit == 0)
        return;
    reg32 found = kNotOnList, count = 0;
    for (reg32 i = 0;; ++i)
    {
        count = at(app, pursuit + kPursuitCopCount);
        if (i32(i) >= i32(count))
            break;
        const reg32 slot = pursuit + i * 4 + kPursuitCops;
        if (found != kNotOnList)
            put(app, slot - 4, at(app, slot));
        else if (cop == at(app, slot))
            found = i;
    }
    put(app, pursuit + kPursuitCopCount, count - 1);
    put(app, pursuit + (count - 1) * 4 + kPursuitCops, 0);
}

/* 0x401a20: takeOffList cop eax; eax its pursuit. */
void leaveCopList(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    takeOffList(app, cop);
    cpu.eax = at(app, cop + kCarPursuit);
    cpu.esp += 4;
}

/* 0x4040b0: cop's word on the radio as it gives up (0x40b480) and its order (kind 2 at its node,
 * 6, by side 1).  False when the game stopped. */
bool radioGiveUpOf(App* app, CPU& cpu, reg32 cop)
{
    if (!guest::direct<0x40b480, pursuit::radioLostOf>(app, cpu, cop))
        return false;
    giveOrderOf(app, cop, 0, kOrderAtNode, reg16(at(app, cop + kCarNode)), 6, 1, 0);
    return true;
}

/* 0x4040b0: radioGiveUpOf cop eax; eax its order. */
void radioGiveUp(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!radioGiveUpOf(app, cpu, cop))
        return;
    cpu.eax = at(app, cop + kCarOrder);
    cpu.esp += 4;
}

/* 0x401a80: cop gives up its pursuit (not one that has its target pulled over, unless the cop is
 * a player's): its word on the radio and its order (radioGiveUpOf), off the list (takeOffList); the
 * last cop off a chase the car gets away from -- unless +0x621 bit 0x40 -- counts the getaway (+0,
 * +0x14, the state back to 0) and the car's worst speeding (+4, chopped); the cop's pursuit
 * cleared, its radio level up.  False when the game stopped. */
bool giveUpPursuitOf(App* app, CPU& cpu, reg32 cop)
{
    if (at(app, cop + kCarChased) == 0)
        return true;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit == 0)
        return true;
    if (at(app, pursuit + kPursuitState) == kPulledOver && (byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return true;
    if (!radioGiveUpOf(app, cpu, cop))
        return false;
    takeOffList(app, cop);
    if (at(app, pursuit + kPursuitCopCount) == 0 && (byte(app, at(app, cop + kCarChased) + kCarSide) & kSideNoGetaway) == 0)
    {
        const reg32 getaways = at(app, pursuit + kPursuitGetaways);
        const reg32 gaveUp = at(app, pursuit + kPursuitGaveUp);
        put(app, pursuit + kPursuitState, kNoPursuit);
        put(app, pursuit + kPursuitGetaways, getaways + 1);
        put(app, pursuit + kPursuitGaveUp, gaveUp + 1);
        const std::optional<Float> worst = chop(app, cpu, f32(app, at(app, cop + kCarChased) + kCarSpeeding));
        if (!worst)
            return false;
        const i32 speeding = fp::toInteger<sreg32>(*worst);
        const i32 kept = i32(at(app, pursuit + kPursuitWorst));
        put(app, pursuit + kPursuitWorst, reg32(kept > speeding ? kept : speeding));
    }
    put(app, cop + kCarPursuit, 0);
    put(app, cop + kCarChased, 0);
    const reg8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarOnPursuit, 0);
    putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
    raiseRadioLevel(app, cop);
    return true;
}

/* 0x401a80: giveUpPursuitOf cop eax. */
void giveUpPursuit(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!giveUpPursuitOf(app, cpu, cop))
        return;
    cpu.eax = cop;
    cpu.esp += 4;
}

/* 0x401e20: car's pull-over spot off. */
void endPullOverOf(App* app, reg32 car)
{
    put(app, car + kCarSpotSet, 0);
    put(app, car + kCarSpotOn, 0);
}

/* 0x401e20: for car eax. */
void endPullOver(App* app, CPU& cpu)
{
    endPullOverOf(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x4042b0: a player's cop drops off its pursuit (takeOffList) and is cleared of it.  Whether it
 * did: a player's. */
bool dropCopOf(App* app, reg32 cop)
{
    if ((byte(app, cop + kCarFlags) & kFlagPlayer) == 0)
        return false;
    takeOffList(app, cop);
    put(app, cop + kCarPursuit, 0);
    put(app, cop + kCarChased, 0);
    put(app, cop + kCarOrderFrom, 0);
    const reg8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarOnPursuit, 0);
    putByte(app, cop + kCarCopFlags, copFlags & kCopFlagsDropped);
    return true;
}

/* 0x4042b0: dropCopOf cop eax; eax the pursuit it was on, where it dropped. */
void dropCop(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (dropCopOf(app, cop))
        cpu.eax = pursuit;
    cpu.esp += 4;
}

/* 0x401b50: the spot car pulls over to, beside it on the side `left` says (0.2 of a turn's quarter
 * and 2 to the right, or the same to the left): its heading (+0x210, 1/1024 of a turn) turned by
 * that into the orientation at +0x8ec (ea::yRotation), the offset along it from its place to
 * +0x910, less its height (+0x118) at +0x914.  The offset's vector on the guest's stack, handed to
 * the vectors' functions.  The edx the vectors' subtraction leaves (its adapter's); nullopt when
 * the game stopped. */
std::optional<reg32> pullOverSpotOf(App* app, CPU& cpu, reg32 car, bool left)
{
    const GuestFrame frame(cpu, 0x14, 0x24);
    const reg32 offset = frame - 0x24;
    const i32 heading = i32(at(app, car + kCarHeading));
    const Float angle = heading > 0 ? fp::mul(Float(heading), f64(app, kHeadingScale))
                                    : fp::add(fp::mul(Float(heading + i32(kHalfTurn)), f64(app, kHeadingScaleLow)),
                                              f64(app, kHeadingStartLow));
    // Through a double, as the code stores it (exact), then a float.
    const float heading32 = float(double(angle));
    const reg32 turn = left ? kSpotTurnLeftBits : kSpotTurnBits;
    const reg32 along = left ? kSpotOffsetLeftBits : kSpotOffsetBits;
    const float spotTurn = float(fp::add(Float(heading32), Float(floatOf(turn))));
    if (!yRotation(app, cpu, car + kCarSpotTurn, floatBits(spotTurn)))
        return std::nullopt;
    if (!scaleVectors(app, cpu, 1, car + kCarSpotSide, floatOf(along), offset))
        return std::nullopt;
    const std::optional<reg32> edx =
        subtractVectorsLeavingEdx(app, cpu, 1, car + kCarPlace, offset, car + kCarSpotPlace);
    if (!edx)
        return std::nullopt;
    putFloat(app, car + kCarSpotHeight, float(fp::sub(f32(app, car + kCarSpotHeight), f32(app, car + kCarHeight))));
    return edx;
}

/* 0x401b50: pullOverSpotOf car eax, on the side edx (not 0 for the left); eax and edx as the
 * vectors' subtraction leaves them. */
void pullOverSpot(App* app, CPU& cpu)
{
    const std::optional<reg32> edx = pullOverSpotOf(app, cpu, cpu.eax, cpu.edx != 0);
    if (!edx)
        return;
    cpu.edx = *edx;
    cpu.esp += 4;
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
std::optional<reg32> planPullOverOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x14, 0x130);
    // The original's ebp - n, its ebp lowered by 0x82.
    const auto local = [&](reg32 n) { return frame - 0x82 + n; };
    const reg32 normal = local(0x42), toCar = local(0x4e), place = local(0x5a), right = local(0x66);
    const reg32 onTrack = local(0) - 0x36, otherOnTrack = local(0) - 0xae;
    const reg32 node = at(app, car + kCarNode);
    reg32 side = 0;
    for (reg32 i = 0; i < 9; ++i)
        put(app, car + kCarSpotTurn + i * 4, at(app, kIdentity + i * 4));
    const reg32 record = at(app, kNodes) + node * kNodeSize;
    const Float scale = f64(app, kNodeScale);
    const struct
    {
        reg32 from, to;
    } bytes[] = {{0x15, normal}, {0x16, normal + 4}, {0x17, normal + 8}, {0x11, right}, {0x12, right + 4}, {0x13, right + 8}};
    for (const auto& b : bytes)
        putFloat(app, b.to, float(fp::mul(Float(i32(reg32(i32(at(app, record + b.from)) >> 24) << 9)), scale)));
    putFloat(app, place, float(fp::mul(Float(i32(at(app, record))), scale)));
    putFloat(app, place + 4, float(fp::mul(Float(i32(at(app, record + 4))), scale)));
    putFloat(app, place + 8, float(fp::mul(scale, Float(i32(at(app, record + 8))))));
    if (!subtractVectors(app, cpu, 1, place, car + kCarPlace, toCar))
        return std::nullopt;
    const std::optional<Float> across = dot(app, cpu, toCar, normal);
    if (!across)
        return std::nullopt;
    if (compareBelow(cpu, 0.0, *across))
        side = 1;
    const std::optional<Float> facing = dot(app, cpu, car + kCarFacing, right);
    if (!facing)
        return std::nullopt;
    if (compareAbove(cpu, 0.0, *facing))
        side ^= 1;
    if (!asCalled(cpu, [&] { return pullOverSpotOf(app, cpu, car, side != 0).has_value(); }))
        return std::nullopt;
    if (!snapToTrack(app, cpu, car + kCarSpotPlace, 1, node, onTrack))
        return std::nullopt;
    // The spot kept where its place on the track is still, on the road and on a polygon of a kind
    // not 0 nor 14.
    bool other = true;
    reg32 kept = 0;
    if (at(app, onTrack + kPlaceStill) == 0 && at(app, onTrack + kPlaceOnRoad) != 0)
    {
        const reg32 polygon = at(app, onTrack + kPlacePolygon);
        if (polygon != 0 && (byte(app, polygon + 1) & kPolygonKindMask) != 0)
        {
            other = (byte(app, polygon + 1) & kPolygonKindMask) == kPolygonNoStop;
            kept = other ? 0 : polygon;
        }
    }
    if (other)
    {
        side ^= 1;
        if (!asCalled(cpu, [&] { return pullOverSpotOf(app, cpu, car, side != 0).has_value(); }))
            return std::nullopt;
        if (!snapToTrack(app, cpu, car + kCarSpotPlace, 1, node, otherOnTrack))
            return std::nullopt;
    }
    put(app, car + kCarSpotSet, 1);
    put(app, car + kCarSpotOn, 1);
    return kept;
}

/* 0x401c20: planPullOverOf car edx; eax the car, edx the kept spot's polygon, else the last snap's
 * mode. */
void planPullOver(App* app, CPU& cpu)
{
    const reg32 car = cpu.edx;
    const std::optional<reg32> polygon = planPullOverOf(app, cpu, car);
    if (!polygon)
        return;
    cpu.eax = car;
    cpu.edx = *polygon != 0 ? *polygon : 1;
    cpu.esp += 4;
}

/* 0x4040e0: the pursuit of cop's target over: the radio's word (0x40b670); every cop set on that
 * car let go (its order and roadblock, its siren off, the pursuit's roadblock cleared, a new order
 * of kind 2); every cop on the pursuit off it (a timer 0x405fd0, its radio level down) -- a
 * player's reset as a driver (0x405220) with its siren off, another given a new order -- and its
 * slot cleared.  False when the game stopped. */
bool releaseCopsOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (!guest::direct<0x40b670, pursuit::radioPatrolOf>(app, cpu, cop))
        return false;
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 other = at(app, i * 4 + kCops);
        const reg8 flags = byte(app, other + kCarFlags);
        putByte(app, other + 0x8a, 0);
        if (flags & kFlagPlayer)
            continue;
        const reg32 index = at(app, other + kCarCopIndex);
        if (at(app, other + kCarOrderFrom) != at(app, cop + kCarChased))
            continue;
        const reg8 copKind = byte(app, other + kCarCopKind);
        put(app, other + kCarOrderFrom, 0);
        if (copKind & kCopRoadblock)
        {
            putByte(app, other + kCarCopKind, copKind & ~kCopRoadblock);
            if (at(app, index * kSirenSize + kSirens) != 0 && !sirenOff(app, cpu, index))
                return false;
            put(app, pursuit + kPursuitRoadblock, 0);
        }
        const reg32 side = at(app, other + kCarCopIndex) & 1;
        putWord(app, other + kCarCopFlags, reg16(word(app, other + kCarCopFlags) & 0xfbef));
        put(app, other + kCarOnPursuit, 0);
        giveOrderOf(app, other, 0, kOrderAtNode, 0, 0, reg8(side), 0);
    }
    for (reg32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
    {
        const reg32 on = at(app, pursuit + i * 4 + kPursuitCops);
        if (on != 0)
        {
            put(app, on + kCarPursuit, 0);
            put(app, on + kCarChased, 0);
            put(app, on + kCarOrderFrom, 0);
            const reg8 copFlags = byte(app, on + kCarCopFlags);
            put(app, on + kCarOnPursuit, 0);
            putByte(app, on + kCarCopFlags, copFlags & kCopFlagsDropped);
            if (!setTimer(app, cpu, on, 1, 0))
                return false;
            lowerRadioLevel(app, on);
            if (byte(app, on + kCarFlags) & kFlagPlayer)
            {
                if (!resetDriver(app, cpu, on, false) || !sirenOff(app, cpu, at(app, on + kCarCopIndex)))
                    return false;
            }
            else
                giveOrderOf(app, on, 0, kOrderAtNode, 0, 0, reg8(at(app, on + kCarCopIndex) & 1), 0);
        }
        put(app, pursuit + i * 4 + kPursuitCops, 0);
    }
    return true;
}

/* 0x4040e0: releaseCopsOf cop eax; eax its pursuit. */
void releaseCops(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (!releaseCopsOf(app, cpu, cop))
        return;
    cpu.eax = pursuit;
    cpu.esp += 4;
}

/* 0x401ff0: cop gives the car it stopped its ticket: the pursuit's count and worst speeding kept,
 * the cop's tally (+0x2cc, its time at the race's limit 0x6fd4b8) and fines (+0x2d0, the speeding
 * chopped), the car's tickets one more -- out of the race at [0x6fd4bc] (+0x2c4 4) -- its speeding
 * cleared, its next pursuit's wait (0x5521b4 by its tickets); both cars' marks of the stop
 * cleared, the spot off, the cops let go (releaseCopsOf) and the pursuit ended.  False when the
 * game stopped. */
bool giveTicketOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 car = at(app, cop + kCarChased);
    const reg32 pursuit = at(app, cop + kCarPursuit);
    putByte(app, car + kCarSide, byte(app, car + kCarSide) & kSideKeep);
    const reg32 getaways = at(app, pursuit + kPursuitGetaways);
    put(app, pursuit + kPursuitGaveUp, 0);
    put(app, pursuit + kPursuitGetaways, getaways + 1);
    const i32 kept = i32(at(app, pursuit + kPursuitWorst));
    const std::optional<Float> worst = chop(app, cpu, f32(app, car + kCarSpeeding));
    if (!worst)
        return false;
    const i32 speeding = fp::toInteger<sreg32>(*worst);
    put(app, pursuit + kPursuitWorst, reg32(kept > speeding ? kept : speeding));
    const reg32 tally = at(app, cop + kCarTally) + 1;
    const reg32 wins = gameSettings(app).pursuitWinCount;
    put(app, cop + kCarTally, tally);
    if (tally == wins)
        put(app, cop + kCarWonAt, at(app, kClock) - kWonTicks);
    const reg32 tickets = at(app, car + kCarTickets);
    const std::optional<Float> fine = chop(app, cpu, f32(app, car + kCarSpeeding));
    if (!fine)
        return false;
    put(app, car + kCarTickets, tickets + 1);
    put(app, cop + kCarFines, at(app, cop + kCarFines) + reg32(fp::toInteger<sreg32>(*fine)));
    const reg32 out = gameSettings(app).pursuitOutCount;
    const reg32 now = at(app, car + kCarTickets);
    put(app, car + kCarSpeeding, 0);
    if (!(i32(now) < i32(out)))
        put(app, car + kCarState, kOutOfPursuits);
    const reg32 waitAt = at(app, car + kCarTickets) << 4;
    const reg32 clock = at(app, kClock);
    const reg32 wait = at(app, waitAt + kTicketWait);
    put(app, car + kCarStoppedBy, 0);
    put(app, car + kCarNextChase, clock + wait);
    put(app, cop + kCarStoppedBy, 0);
    putByte(app, car + 0x8a, 0);
    endPullOverOf(app, car);
    if (!releaseCopsOf(app, cpu, cop))
        return false;
    put(app, pursuit + kPursuitCopCount, 0);
    put(app, pursuit + kPursuitStoppedBy, 0);
    put(app, pursuit + kPursuitState, kNoPursuit);
    return true;
}

/* 0x401ff0: giveTicketOf cop eax; eax its pursuit (releaseCops's). */
void giveTicket(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (!giveTicketOf(app, cpu, cop))
        return;
    cpu.eax = pursuit;
    cpu.esp += 4;
}

/* 0x401fb0: whether cop is done with a stop: so without a pursuit or a target, not while the stop
 * lasts (+0x28); at its end the ticket (giveTicketOf) and so.  nullopt when the game stopped. */
std::optional<bool> stopOverOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (pursuit != 0 && at(app, cop + kCarChased) != 0)
    {
        if (i32(at(app, kClock)) <= i32(at(app, pursuit + kPursuitStopEnds)))
            return false;
        if (!giveTicketOf(app, cpu, cop))
            return std::nullopt;
    }
    return true;
}

/* 0x401fb0: stopOverOf cop eax, 1 or 0 in eax. */
void stopOver(App* app, CPU& cpu)
{
    const std::optional<bool> over = stopOverOf(app, cpu, cpu.eax);
    if (!over)
        return;
    cpu.eax = *over ? 1 : 0;
    cpu.esp += 4;
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
std::optional<StopBy> copStopsByOf(App* app, CPU& cpu, reg32 cop)
{
    // The original's frame, below which planPullOverOf's goes.
    const GuestFrame frame(cpu, 0x18, 0x14);
    const reg32 pursuit = at(app, cop + kCarPursuit);
    reg32 stopper = cop;
    const reg32 count = at(app, pursuit + kPursuitCopCount);
    const reg32 car = at(app, cop + kCarChased);
    if (i32(count) > 1 && (byte(app, cop + kCarFlags) & kFlagPlayer))
    {
        float nearest = floatOf(kFarBits);
        if (at(app, pursuit + kPursuitStoppedBy) != 0)
            stopper = 0;
        else
            for (reg32 i = 0; i32(i) < i32(at(app, pursuit + kPursuitCopCount)); ++i)
            {
                const reg32 other = at(app, pursuit + i * 4 + kPursuitCops);
                const std::optional<Float> apart = distanceSize(app, cpu, other + kCarPlace, car + kCarPlace);
                if (!apart)
                    return std::nullopt;
                const float d = float(*apart);
                if (compareBelow(cpu, Float(d), Float(nearest)))
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
    const reg8 copFlags = byte(app, stopper + kCarCopFlags) | kCopStopping;
    putByte(app, stopper + kCarCopFlags, copFlags);
    putByte(app, stopper + kCarCopFlags, copFlags & ~kCopChasing);
    const reg32 stoppedBy = at(app, pursuit + kPursuitStoppedBy) + 1;
    put(app, pursuit + kPursuitStoppedBy, stoppedBy);
    if (stoppedBy != 1)
        return StopBy::Counted;
    const reg32 ends = byte(app, stopper + kCarFlags) & kFlagPlayer
                           ? at(app, kClock) + kPlayerCopStopTicks
                           : at(app, kClock) + at(app, (at(app, car + kCarTickets) << 4) + kTicketStop);
    put(app, pursuit + kPursuitStopEnds, ends);
    if ((byte(app, car + kCarFlags) & kFlagPlayer) && !radioTicket(app, cpu, stopper, car))
        return std::nullopt;
    if (!asCalled(cpu, [&] { return planPullOverOf(app, cpu, car).has_value(); }))
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
std::optional<Chase> chaseSpeedOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 target = at(app, cop + kCarChased);
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
        const reg32 share = target == at(app, kChaseLeader) ? kLeaderShare : kOtherShare;
        putFloat(app, cop + kCarWanted, float(fp::mul(f32(app, target + kCarSpeed), f64(app, share))));
        way = Chase::BehindFast;
        if (compareBelow(cpu, f32(app, cop + kCarSpeed), f64(app, kTimerSpeed)))
        {
            const i32 gap = i32(at(app, kGap));
            way = Chase::BehindNear;
            if ((gap > 0 ? gap : i32(neg(reg32(gap)))) > kTimerGap)
            {
                // By address: chaseSpeed leaves the eax it leaves.
                if (!setTimerLeavingEax(app, cpu, cop, 0, 0))
                    return std::nullopt;
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
            keepTop = compareAbove(cpu, f32(app, at(app, cop + kCarChased) + kCarSpeed), Float(top));
        if (keepTop)
        {
            putFloat(app, cop + kCarWanted, top);
            way = Chase::AheadTop;
        }
        else
        {
            const reg32 chased = at(app, cop + kCarChased);
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
LaneVote votePursuitLaneOf(App* app, CPU& cpu, reg32 cop)
{
    const auto vote = [&](reg32 table, reg32 amount) {
        const reg32 at_ = at(app, kLaneVote) * 4 + table;
        put(app, at_, at(app, at_) + amount);
    };
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return LaneVote::NoTarget;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (compareBelow(cpu, f32(app, target + kCarSpeed), f64(app, kSlowTarget)) && at(app, cop + kCarToldSpeed) == 0)
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
    const bool targetBack = compareAbove(cpu, 0.0, f32(app, at(app, cop + kCarChased) + kCarSignedSpeed));
    const bool copBack = compareAbove(cpu, 0.0, f32(app, cop + kCarSignedSpeed));
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
    const auto add = [&](reg32 slot, reg32 amount) { return at(app, kLaneFor + slot * 4) + amount; };
    switch (at(app, kLaneVote))
    {
    case kLaneLeft:
    {
        const reg32 next = add(1, 0x1e), own = add(0, 0xa);
        put(app, kLaneFor + 4, next);
        put(app, kLaneFor, own);
        break;
    }
    case kLaneSame:
    {
        const reg32 own = add(1, 0xa), right = add(2, 0x1e), left = add(0, 0x1e);
        put(app, kLaneFor + 4, own);
        put(app, kLaneFor + 8, right);
        put(app, kLaneFor, left);
        break;
    }
    case kLaneRight:
    {
        const reg32 own = add(2, 0xa), next = add(1, 0x1e);
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
void votePursuitLane(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    switch (votePursuitLaneOf(app, cpu, cop))
    {
    case LaneVote::NoTarget:
        break;
    case LaneVote::Level:
        cpu.eax = withStatus(cpu, cop);
        break;
    case LaneVote::Behind:
    case LaneVote::Oncoming:
    case LaneVote::Far:
        cpu.eax = at(app, kLaneVote);
        break;
    case LaneVote::Lanes:
        cpu.eax = at(app, kLaneVote) == kLaneSame ? at(app, kLaneFor + 8) : at(app, kLaneVote);
        break;
    }
    cpu.esp += 4;
}

// The ways rivalTickOf goes: not a racer that rams, no rival near, not slower than it wants, its
// wait not over, the rival not a player's or not now, the chase's sound.
enum class Rivalry { NotRammer, NoRival, Fast, Waiting, NotNow, ChaseSound };

struct RivalOutcome
{
    Rivalry way;
    bool belowZero = false;  // 0 below its wanted speed (or a NaN), the first compare's
};

/* 0x404840: racer's rivalry, each tick: between ticks 0x480 and 0x4c0, where the first car is
 * marked (+0x50d) in the last 3 nodes, it rams the first player's car for 50000 ticks; a racer
 * that rams counts its wait (+0x6e4) down, looks for its rival (0x40deb0) and, its rival near
 * (0x56ef3c, no more than one between, 0x56ef48) and itself slower by 0x536194 than it wants, its
 * wait over (0x40f050) -- its lane by the rival (0x40de50) unless +0x620 bit 8 -- against a
 * player's car now and then (the clock's 0xc0 bits) a cop chase's sound (snd::copChaseSet) of a
 * random kind of four.  nullopt when the game stopped. */
std::optional<RivalOutcome> rivalTickOf(App* app, CPU& cpu, reg32 racer)
{
    const i32 now = i32(at(app, kClock));
    if (now > 0x480 && now < 0x4c0)
    {
        const reg32 first = at(app, kFirstCar);
        if (byte(app, first + kCarMarked) != 0)
        {
            const i32 node = i32(at(app, first + kCarNode));
            if (i32(at(app, kNodeCount) - 3) < node && node < i32(at(app, kNodeCount)))
            {
                const reg32 player = at(app, kPlayers);
                put(app, racer + kCarRamFor, kRamTicks);
                put(app, racer + kCarChased, player);
            }
        }
    }
    if ((byte(app, racer + kCarFlags) & kFlagRams) == 0)
        return RivalOutcome{Rivalry::NotRammer};
    const reg32 wait = at(app, racer + kCarRamFor);
    if (i32(wait) > 0)
        put(app, racer + kCarRamFor, wait - 1);
    // By address: rivalTick leaves the eax it leaves (aiplan::endHorn's header gives none).
    if (!callFor(app, cpu, kRivalNear, racer))
        return std::nullopt;
    if (at(app, kRival) == 0 || i32(at(app, kRivalsBetween)) > 1)
        return RivalOutcome{Rivalry::NoRival};
    const bool belowZero = compareBelow(cpu, 0.0, f32(app, racer + kCarWanted));
    const reg32 wanted = belowZero ? at(app, racer + kCarWanted) : floatBits(float(-f32(app, racer + kCarWanted)));
    if (!compareBelow(cpu, fp::add(f32(app, racer + kCarSpeed), f64(app, kRivalSlower)), Float(floatOf(wanted))))
        return RivalOutcome{Rivalry::Fast, belowZero};
    const std::optional<reg32> waiting = ramWait(app, cpu, racer, 0);
    if (!waiting)
        return std::nullopt;
    if (*waiting != 0)
        return RivalOutcome{Rivalry::Waiting};
    if ((byte(app, racer + kCarInput) & kInputNoLane) == 0)
    {
        guest::direct<0x40de50, aiplan::soundHorn>(app, cpu, racer);
        if (cpu.terminate)
            return std::nullopt;
    }
    if ((byte(app, at(app, kRival) + kCarFlags) & kFlagPlayer) == 0 || (byte(app, kClock) & 0xc0))
        return RivalOutcome{Rivalry::NotNow};
    // jmp [kind * 4 + 0x40482c]: 0x4049a6, 0x4049b7, 0x4049c8, 0x4049d9.
    const reg32 kind = ((nextRandom(app) * 20) >> 16) & 3;
    static constexpr struct
    {
        reg32 edx, ebx, ecx;
    } kSounds[] = {{3, 0x18, 8}, {3, 0x10, 5}, {2, 0x50, 8}, {1, 0x64, 8}};
    if (!callWith(app, cpu, kCopChaseSet, [&] {
            cpu.ecx = kSounds[kind].ecx;
            cpu.ebx = kSounds[kind].ebx;
            cpu.edx = kSounds[kind].edx;
            cpu.eax = racer;
        }))
        return std::nullopt;
    return RivalOutcome{Rivalry::ChaseSound};
}

/* 0x404840: rivalTickOf racer eax.  eax as the code leaves it on each way out: not a racer that
 * rams, the car the window's test came to (worked out before: what it reads is written by nothing
 * before it); as the callee last called leaves it; slower not, its eax or the wanted speed's bits
 * with the compare's status word; the rival. */
void rivalTick(App* app, CPU& cpu)
{
    const reg32 racer = cpu.eax;
    reg32 window = racer;
    const i32 now = i32(at(app, kClock));
    if (now > 0x480 && now < 0x4c0)
    {
        window = at(app, kFirstCar);
        const i32 node = i32(at(app, window + kCarNode));
        if (byte(app, window + kCarMarked) != 0 && i32(at(app, kNodeCount) - 3) < node && node < i32(at(app, kNodeCount)))
            window = at(app, kPlayers);
    }
    const std::optional<RivalOutcome> rivalry = rivalTickOf(app, cpu, racer);
    if (!rivalry)
        return;
    switch (rivalry->way)
    {
    case Rivalry::NotRammer:
        cpu.eax = window;
        break;
    case Rivalry::Fast:
        cpu.eax = withStatus(cpu, rivalry->belowZero ? at(app, racer + kCarWanted) : cpu.eax);
        break;
    case Rivalry::NotNow:
        cpu.eax = at(app, kRival);
        break;
    case Rivalry::NoRival:
    case Rivalry::Waiting:
    case Rivalry::ChaseSound:
        break;
    }
    cpu.esp += 4;
}

/* rivalTickOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool rivalTickDone(App* app, CPU& cpu, reg32 racer)
{
    return rivalTickOf(app, cpu, racer).has_value();
}

/* 0x404560: whether car's order is new (now taken) or was taken. */
bool takeNewOrder(App* app, reg32 car)
{
    const reg32 order = at(app, car + kCarOrder);
    if (byte(app, order + kOrderTaken) != 0)
        return false;
    putByte(app, order + kOrderTaken, 1);
    return true;
}

/* 0x404560: takeNewOrder car eax, 1 or 0 in eax. */
void orderTaken(App* app, CPU& cpu)
{
    cpu.eax = takeNewOrder(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

void nothingForCop(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404610: car's class's rule (0x552274 by its model's +0x30), 1 in place of 0 in the race's
 * modes 1 and 2 (0x6fd3b8). */
reg32 classRuleOf(App* app, reg32 car)
{
    const reg32 kind = at(app, at(app, car + kCarDriver) + kModelClass);
    const reg32 race = gameSettings(app).raceType;
    const reg32 rule = at(app, kind * kClassSize + kClassRule);
    return (race == 1 || race == 2) && rule == 0 ? 1 : rule;
}

/* 0x404610: classRuleOf car eax, in eax. */
void classRule(App* app, CPU& cpu)
{
    cpu.eax = classRuleOf(app, cpu.eax);
    cpu.esp += 4;
}

void nothingAtEnd(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404660: nothingAtEnd called: nothing but the return. */
void nothingAtEndToo(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404690: racer's (not a player's) ramming rules, its record at 0x56ea70 + 0x14 by its place
 * (+0x1f0): 0.94, 0, and by its class's kind (0x55227c, the table at 0x40466c) the hits it takes
 * (0x6e/0x64/0x3c/0x14/15/0xa), the ticks it waits after them (0x40..0xc0) and its level (5, 4, 3,
 * 1), one less in the race's mode of 0x6fd4f0 1.  Its place read again for each record's word, as
 * the code does. */
void setRammingRulesOf(App* app, reg32 racer)
{
    if (byte(app, racer + kCarFlags) & kFlagPlayer)
        return;
    const auto record = [&] { return at(app, racer + kCarPlaceInRace) * kRammingSize + kRamming; };
    put(app, record(), kRamGrip);
    put(app, record() + 4, 0);
    put(app, record() + 8, 0xf);
    put(app, record() + 0xc, 0x3c);
    const reg32 kind = at(app, at(app, at(app, racer + kCarDriver) + kModelClass) * kClassSize + kClassKind);
    reg32 hits = 0xf, wait = 0, level = 0;
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
void setRammingRules(App* app, CPU& cpu)
{
    const reg32 racer = cpu.eax;
    setRammingRulesOf(app, racer);
    if ((byte(app, racer + kCarFlags) & kFlagPlayer) == 0)
        cpu.eax = at(app, racer + kCarPlaceInRace) * 5;
    cpu.esp += 4;
}

/* 0x4047f0, 0x404810: a value of the class of the model (+0x30): the table's at 0x55226c, at
 * 0x552280. */
reg32 classValueOf(App* app, reg32 model)
{
    return at(app, at(app, model + kModelClass) * kClassSize + kClassValue);
}

reg32 classValueTooOf(App* app, reg32 model)
{
    return at(app, at(app, model + kModelClass) * kClassSize + kClassValueToo);
}

/* 0x4047f0, 0x404810: for the model at eax, in eax. */
void classValue(App* app, CPU& cpu)
{
    cpu.eax = classValueOf(app, cpu.eax);
    cpu.esp += 4;
}

void classValueToo(App* app, CPU& cpu)
{
    cpu.eax = classValueTooOf(app, cpu.eax);
    cpu.esp += 4;
}

/* 1, whatever it is asked. */
reg32 alwaysOneOf(App* app)
{
    (void)app;
    return 1;
}

/* 0x402c70: alwaysOneOf. */
void alwaysOne(App* app, CPU& cpu)
{
    cpu.eax = alwaysOneOf(app);
    cpu.esp += 4;
}

/* 0x402c80: whether car goes backwards (+0x10c below 0) on the ground (+0x164 0): 0 over its
 * speed, so not for a NaN, as jbe after fcomp has it. */
bool isReversing(App* app, reg32 car)
{
    return Float(0.0) > f32(app, car + kCarForward) && word(app, car + kCarAirborne) == 0;
}

/* 0x402c80: isReversing car eax, 1 or 0 in eax. */
void carReversing(App* app, CPU& cpu)
{
    cpu.eax = isReversing(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4028e0: car onto the side of the road toward node (+0x621 bit 0x10 set where the node is
 * behind it).  The gap; what the gap's function left in edx to `edx`, where asked; nullopt when
 * the game stopped. */
std::optional<i32> sideTowardNodeOf(App* app, CPU& cpu, reg32 car, reg32 node, reg32* edx = nullptr)
{
    const std::optional<i32> gap = nodeGap(app, cpu, node, at(app, car + kCarNode), edx);
    if (!gap)
        return std::nullopt;
    if (*gap >= 0)
        putByte(app, car + kCarSide, byte(app, car + kCarSide) & ~kSideOther);
    else
        putByte(app, car + kCarSide, byte(app, car + kCarSide) | kSideOther);
    return gap;
}

/* 0x4028e0: sideTowardNodeOf car eax, node edx; eax the gap, edx as the gap's function left it. */
void sideTowardNode(App* app, CPU& cpu)
{
    reg32 edx = 0;
    const std::optional<i32> gap = sideTowardNodeOf(app, cpu, cpu.eax, cpu.edx, &edx);
    if (!gap)
        return;
    cpu.eax = reg32(*gap);
    cpu.edx = edx;
    cpu.esp += 4;
}

/* 0x403000: car onto the side of the road toward its order's node.  The gap; nullopt when the
 * game stopped. */
std::optional<i32> sideOfOrderOf(App* app, CPU& cpu, reg32 car)
{
    return sideTowardNodeOf(app, cpu, car, orderNode(app, car));
}

/* 0x403000: sideOfOrderOf car eax; eax the gap. */
void sideOfOrder(App* app, CPU& cpu)
{
    const std::optional<i32> gap = sideOfOrderOf(app, cpu, cpu.eax);
    if (!gap)
        return;
    cpu.eax = reg32(*gap);
    cpu.esp += 4;
}

/* 0x40f000: whether car at `node` is at node `at` with the second word `word2` (0x40f000); nullopt
 * when the game stopped. */
std::optional<reg32> atNode(App* app, CPU& cpu, reg32 node, reg32 target, reg32 word2)
{
    const KeptRegisters kept(cpu);
    cpu.eax = node;
    cpu.edx = target;
    cpu.ebx = word2;
    if (!byAddress(app, cpu, kAtNode))
        return std::nullopt;
    return cpu.eax;
}

/* 0x402fd0: whether car is at its order's node (0x40f000, the order's node and second word). */
std::optional<reg32> atOrderNodeOf(App* app, CPU& cpu, reg32 car)
{
    const reg32 word2 = orderWord(app, car);
    return atNode(app, cpu, at(app, car + kCarNode), orderNode(app, car), word2);
}

/* 0x402fd0: atOrderNodeOf car eax, in eax. */
void atOrderNode(App* app, CPU& cpu)
{
    const std::optional<reg32> there = atOrderNodeOf(app, cpu, cpu.eax);
    if (!there)
        return;
    cpu.eax = *there;
    cpu.esp += 4;
}

/* 0x404b90: racer slowed for its class (its wanted speed by 0x5361a4): touched by a car (+0x16c)
 * -- in the race mode of 0x6fd4f0 1 hard enough (15, as bits) -- either the hit's force over
 * 0x53619c (chopped), or its class's rule 0 (classRuleOf) and now and then (the clock's bits 5..7
 * 0).  Whether it was; nullopt when the game stopped. */
std::optional<bool> slowForClassOf(App* app, CPU& cpu, reg32 car)
{
    i32 slowed = 0;
    if (at(app, car + kCarHit) == 0
        || (gameSettings(app).seriesRace == 1 && i32(at(app, car + kCarHitForce)) < i32(kHardHitBits)))
        return false;
    if (compareAbove(cpu, f32(app, car + kCarHitForce), f64(app, kHitForce)))
    {
        const std::optional<Float> force = chop(app, cpu, f32(app, car + kCarHitForce));
        if (!force)
            return std::nullopt;
        slowed = fp::toInteger<sreg32>(*force);
    }
    if (classRuleOf(app, car) == 0)
        slowed |= ((i32(at(app, kClock)) >> 5) & 7) == 0 ? 1 : 0;
    if (slowed == 0)
        return false;
    putFloat(app, car + kCarWanted, float(fp::mul(f32(app, car + kCarWanted), f64(app, kClassSlowing))));
    return true;
}

/* 0x404b90: slowForClassOf racer eax, 1 or 0 in eax. */
void slowForClass(App* app, CPU& cpu)
{
    const std::optional<bool> slowed = slowForClassOf(app, cpu, cpu.eax);
    if (!slowed)
        return;
    cpu.eax = *slowed ? 1 : 0;
    cpu.esp += 4;
}

void nothingAtStartToo(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404c50: the catch-up speed (0x56ebb0): 20.12 while fewer players (0x6fd518) than 0x6fd520,
 * else 11.18. */
void setCatchUpSpeed(App* app)
{
    const reg32 humans = gameSettings(app).humanCount;
    put(app, kCatchUpSpeed, i32(humans) < i32(gameSettings(app).opponentCount) ? kCatchUpFastBits : kCatchUpSlowBits);
}

/* 0x404c50: setCatchUpSpeed. */
void catchUpSpeed(App* app, CPU& cpu)
{
    setCatchUpSpeed(app);
    cpu.esp += 4;
}

/* 0x404c80: the leader among the players' cars (0x5efa48, [0x5efda4]), asked by the last of them
 * (+0x1f4): the one of the most laps (+8; a finished one's 1) to 0x56ebb8; then by the other
 * racers (0x5efb88, [0x5efd94]) how many have more laps -- 0x56ebb4 1 some, 2 all, 0 none. */
void findLeaderOf(App* app, reg32 car)
{
    const reg32 others = at(app, kOtherCount);
    i32 most = 0;
    if (at(app, kPlayerCount) - 1 == at(app, car + kCarPlayerIndex))
        for (reg32 i = 0; i32(i) < i32(at(app, kPlayerCount)); ++i)
        {
            const reg32 player = at(app, i * 4 + kPlayers);
            if (byte(app, player + kCarFlags) & kFlagFinished)
                put(app, player + kCarLaps, 1);
            const i32 laps = i32(at(app, player + kCarLaps));
            if (!(most > laps))
            {
                most = laps;
                put(app, kLeader, player);
            }
        }
    reg32 behind = 0;
    if (others != 0)
    {
        reg32 more = 0;
        for (reg32 i = 0; i32(i) < i32(others); ++i)
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
void findLeader(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const reg32 others = at(app, kOtherCount), players = at(app, kPlayerCount);
    reg32 eax = car;
    if (others != 0)
        eax = i32(others) > 0 ? others : 0;
    else if (players - 1 == at(app, car + kCarPlayerIndex) && i32(players) > 0)
        eax = at(app, (players - 1) * 4 + kPlayers);
    findLeaderOf(app, car);
    cpu.eax = eax;
    cpu.esp += 4;
}

/* 0x403910: cop's word on the radio that its target has stopped (0x40bc30), once for the pursuit
 * (+0x54 bit 8).  False when the game stopped. */
bool radioStoppedOf(App* app, CPU& cpu, reg32 cop)
{
    if (byte(app, at(app, at(app, cop + kCarChased) + kCarPursuit) + kPursuitCalls) & kPursuitStoppedCall)
        return true;
    if (!guest::direct<0x40bc30, pursuit::radioHelpOf>(app, cpu, cop))
        return false;
    const reg32 pursuit = at(app, at(app, cop + kCarChased) + kCarPursuit);
    putByte(app, pursuit + kPursuitCalls, byte(app, pursuit + kPursuitCalls) | kPursuitStoppedCall);
    return true;
}

/* 0x403910: radioStoppedOf cop eax; eax its target's pursuit. */
void radioStopped(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!radioStoppedOf(app, cpu, cop))
        return;
    cpu.eax = at(app, at(app, cop + kCarChased) + kCarPursuit);
    cpu.esp += 4;
}

/* 0x403950: cop's word on the radio that it stopped (0x40bba0), once (+0x711 bit 0x10).  What
 * the code leaves in eax: the radio's, else the cop; nullopt when the game stopped. */
std::optional<reg32> radioStopOf(App* app, CPU& cpu, reg32 cop)
{
    if (byte(app, cop + kCarCopKind) & kCopToldStop)
        return cop;
    const std::optional<reg32> eax = guest::direct<0x40bba0, pursuit::radioSpikesOf>(app, cpu, cop);
    if (!eax)
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) | kCopToldStop);
    return eax;
}

/* 0x403950: radioStopOf cop eax; eax as the radio leaves it, else the cop. */
void radioStop(App* app, CPU& cpu)
{
    const std::optional<reg32> eax = radioStopOf(app, cpu, cpu.eax);
    if (!eax)
        return;
    cpu.eax = *eax;
    cpu.esp += 4;
}

/* 0x403870: the cops spread round the track at the race's start: each its order of kind 2 at a
 * node a share of the track on from the last, less a random quarter of it, the side by its
 * index. */
void spreadCopsOf(App* app)
{
    const i32 share = i32(at(app, kNodeCount)) / i32(at(app, kCopCount) + 1);
    reg32 node = 0;
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 r = nextRandom(app);
        node += reg32(share) - (imul(r, reg32(share >> 2)) >> 16);
        giveOrderOf(app, at(app, i * 4 + kCops), 0, kOrderAtNode, reg16(node), 5, reg8(i & 1), 0);
    }
}

/* What spreadCops's code leaves in eax: the last cop's order, else the share. */
reg32 spreadCopsEax(App* app)
{
    const i32 count = i32(at(app, kCopCount));
    return count > 0 ? at(app, at(app, reg32(count - 1) * 4 + kCops) + kCarOrder)
                     : reg32(i32(at(app, kNodeCount)) / (count + 1));
}

/* 0x403870: spreadCopsOf. */
void spreadCops(App* app, CPU& cpu)
{
    spreadCopsOf(app);
    cpu.eax = spreadCopsEax(app);
    cpu.esp += 4;
}

/* 0x403970: the race's pursuits cleared (0x56e410, 16 of 0x60 bytes) and each racer given its
 * own; the cops spread (spreadCopsOf).  False when the game stopped. */
bool initPursuitsOf(App* app, CPU& cpu)
{
    if (!clearBytes(app, cpu, kPursuits, 0, kPursuitsBytes))
        return false;
    for (reg32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
        put(app, at(app, i * 4 + kRacers) + kCarPursuit, kPursuits + i * kPursuitSize);
    spreadCopsOf(app);
    return true;
}

/* 0x403970: initPursuitsOf; eax as spreadCops leaves it. */
void initPursuits(App* app, CPU& cpu)
{
    if (!initPursuitsOf(app, cpu))
        return;
    cpu.eax = spreadCopsEax(app);
    cpu.esp += 4;
}

/* 0x403270: car on patrol: steered at its target (0x40dd60), its wanted speed by `factor` and
 * `offset` (setWantedSpeedOf), clear of the others (0x40bff0), its lanes (0x40ddb0), steered
 * (0x40de30).  False when the game stopped. */
bool drivePatrolOf(App* app, CPU& cpu, reg32 car, float factor, float offset)
{
    if (!steerAtTarget(app, cpu, car) || !setWantedSpeedOf(app, cpu, car, factor, offset))
        return false;
    guest::direct<0x40bff0, pursuit::followChasedCar>(app, cpu, car);
    if (cpu.terminate)
        return false;
    return chooseLanes(app, cpu, car) && callFor(app, cpu, kSteer, car);
}

/* 0x403270: drivePatrolOf car eax, the factor and the offset on the stack; ret 8.  eax as the
 * steering leaves it. */
void drivePatrol(App* app, CPU& cpu)
{
    if (!drivePatrolOf(app, cpu, cpu.eax, floatAt(app, cpu.esp + 4), floatAt(app, cpu.esp + 8)))
        return;
    cpu.esp += 4 + 8;
}

/* 0x402a00: cop put at its order's node (the first of them free, 0x405c80, 20 nodes on each time,
 * 0x40e160): its sideways place (+0x7b4) a random share of the side's lanes (the node's +0x13)
 * and width (+0x1b right, +0x17 left, by 0x5360dc and 0x5360e4) -- the side the order (0x56f230,
 * 0x56f228) and the car's say -- plus 1; the patrol's speed (setWantedSpeedOf 1.19, 0) its speed
 * too, the chase off (0x40cd80), a random radio level 0..2.  False when the game stopped. */
bool placeCopOf(App* app, CPU& cpu, reg32 cop)
{
    reg32 node = orderNode(app, cop);
    for (;;)
    {
        put(app, cop + kCarNode, node);
        const std::optional<reg32> blocked = nodeBlocked(app, cpu, node);
        if (!blocked)
            return false;
        if (*blocked == 0)
            break;
        const std::optional<reg32> next = nodeOn(app, cpu, cop, kNodeStride);
        if (!next)
            return false;
        node = *next;
    }
    if (at(app, kSideOrderSet) != 0)
        putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
    const reg32 backwards = gameSettings(app).backwards;
    bool right = (((byte(app, cop + kCarSide) & kSideOther) == 0 ? 1u : 0u) ^ backwards) != 0 && at(app, kSideOrder) == 1;
    if (!right)
        right = (((byte(app, cop + kCarSide) & kSideOther) != 0 ? 1u : 0u) ^ backwards) != 0
                && at(app, kSideOrder) == 0xffffffff;
    const reg32 record = at(app, kNodes) + node * kNodeSize;
    Float sideways;
    if (right)
    {
        const Float width = fp::mul(Float(i32(reg32(byte(app, record + kNodeRightWidth)) << 15)), f64(app, kWidthScale));
        const reg32 r = nextRandom(app);
        const reg32 lanes = byte(app, record + kNodeLanes) & 0xf;
        const reg32 share = (imul(lanes, r) >> 16) + 1;
        sideways = fp::sub(fp::mul(Float(i32(share)), width), fp::mul(width, f64(app, kLaneShare)));
    }
    else
    {
        const Float width = fp::mul(Float(i32(reg32(byte(app, record + kNodeLeftWidth)) << 15)), f64(app, kWidthScale));
        const reg32 r = nextRandom(app);
        const reg32 lanes = (byte(app, record + kNodeLanes) >> 4) & 0xf;
        const reg32 share = (imul(r, lanes) >> 16) + 1;
        sideways = fp::add(fp::mul(-width, Float(i32(share))), fp::mul(width, f64(app, kLaneShare)));
    }
    putFloat(app, cop + kCarSideways, float(sideways));
    put(app, cop + kCarLaneOffset, kOneBits);
    putFloat(app, cop + kCarSideways, float(fp::add(f32(app, cop + kCarLaneOffset), f32(app, cop + kCarSideways))));
    if (!setWantedSpeedOf(app, cpu, cop, floatOf(kPatrolFactorBits), 0.0f))
        return false;
    put(app, cop + kCarSignedSpeed, at(app, cop + kCarWanted));
    const reg32 speed = compareBelow(cpu, 0.0, f32(app, cop + kCarSignedSpeed))
                            ? at(app, cop + kCarSignedSpeed)
                            : floatBits(float(-f32(app, cop + kCarSignedSpeed)));
    put(app, cop + kCarSpeed, speed);
    const reg8 copFlags = byte(app, cop + kCarCopFlags);
    put(app, cop + kCarChased, 0);
    putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
    put(app, cop + kCarOnPursuit, 0);
    if (!guest::direct<0x40cd80, aiplan::putOnTrack>(app, cpu, cop))
        return false;
    put(app, cop + kCarRadioLevel, (nextRandom(app) * 3) >> 16);
    return true;
}

/* 0x402a00: placeCopOf cop eax; eax its radio level. */
void placeCop(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!placeCopOf(app, cpu, cop))
        return;
    cpu.eax = at(app, cop + kCarRadioLevel);
    cpu.esp += 4;
}

/* 0x4029c0: cop placed at the race's start: its side cleared, the last player's car of the race
 * (0x5efcc8, [0x5efd9c] of them) its watch (+0x6d8), then placeCopOf.  False when the game
 * stopped. */
bool placeCopAtStartOf(App* app, CPU& cpu, reg32 cop)
{
    putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideOther);
    for (reg32 i = at(app, kRaceCarCount) - 1; i32(i) >= 0; --i)
    {
        const reg32 car = at(app, i * 4 + kRaceCars);
        if (byte(app, car + kCarFlags) & kFlagPlayer)
        {
            put(app, cop + kCarWatched, car);
            break;
        }
    }
    return placeCopOf(app, cpu, cop);
}

/* 0x4029c0: placeCopAtStartOf cop eax; eax as placeCop leaves it. */
void placeCopAtStart(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    if (!placeCopAtStartOf(app, cpu, cop))
        return;
    cpu.eax = at(app, cop + kCarRadioLevel);
    cpu.esp += 4;
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
std::optional<KeptSide> keepSideOfTargetOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return KeptSide{SideKeep::NoTarget};
    if (compareBelow(cpu, f32(app, target + kCarSpeed), f64(app, kOncomingSpeed)))
        return KeptSide{SideKeep::Slow};
    const bool targetBack = compareAbove(cpu, 0.0, f32(app, at(app, cop + kCarChased) + kCarSignedSpeed));
    const bool copBack = compareAbove(cpu, 0.0, f32(app, cop + kCarSignedSpeed));
    if (copBack == targetBack)
        return KeptSide{SideKeep::SameWay};
    const std::optional<i32> gap =
        nodeGap(app, cpu, at(app, cop + kCarNode), at(app, at(app, cop + kCarChased) + kCarNode));
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
void keepSideOfTarget(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<KeptSide> kept = keepSideOfTargetOf(app, cpu, cop);
    if (!kept)
        return;
    switch (kept->way)
    {
    case SideKeep::NoTarget:
        break;
    case SideKeep::Slow:
        cpu.eax = withStatus(cpu, cop);
        break;
    case SideKeep::SameWay:
        cpu.eax = 0;
        break;
    case SideKeep::Kept:
        cpu.eax = reg32(kept->gap);
        break;
    }
    cpu.esp += 4;
}

/* 0x403f80: how many more cops racer wants (0 for none), cop asking (0 none): none for a racer
 * out of the race; all of them for the car the cop was set on; for one that never got away and is
 * not over its node's speed by 0x536104 nor speeding past 2 (as bits), none; else up to a third of
 * the cops (at least 1) -- all but one of them for a player's car -- less those on it.  nullopt
 * when the game stopped. */
std::optional<reg32> copsWantedOf(App* app, CPU& cpu, reg32 cop, reg32 racer)
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
    const reg32 pursuit = at(app, racer + kCarPursuit);
    if (cop != 0 && racer == at(app, cop + kCarOrderFrom))
        return at(app, kCopCount);
    if (at(app, pursuit + kPursuitGaveUp) == 0)
    {
        const std::optional<Float> speed = nodeSpeed(app, cpu, at(app, racer + kCarNode));
        if (!speed)
            return std::nullopt;
        const Float limit = fp::add(*speed, f64(app, kCopsWantedOver));
        if (!compareAbove(cpu, f32(app, racer + kCarSpeed), limit)
            && !(i32(at(app, racer + kCarSpeeding)) > i32(kSpeedingBitsWanted)))
            return 0;
    }
    const reg32 more = reg32(player ? allButOne : third) - at(app, pursuit + kPursuitCopCount);
    return i32(more) > 0 ? more : 0;
}

/* What copsWanted's code leaves in edx: 4 for a racer out of the race, else its pursuit. */
reg32 copsWantedEdx(App* app, reg32 racer)
{
    return at(app, racer + kCarState) == kOutOfPursuits ? kOutOfPursuits : at(app, racer + kCarPursuit);
}

/* 0x403f80: copsWantedOf cop eax, racer edx; eax the count, edx as the code leaves it. */
void copsWanted(App* app, CPU& cpu)
{
    const reg32 racer = cpu.edx;
    const std::optional<reg32> wanted = copsWantedOf(app, cpu, cpu.eax, racer);
    if (!wanted)
        return;
    cpu.eax = *wanted;
    cpu.edx = copsWantedEdx(app, racer);
    cpu.esp += 4;
}

// How a hit on a cop went: nothing, the racer slower, too soon to chase it, the cop kept to its
// own, switched to the racer, the racer not wanted, joined.
enum class CopHit { Nothing, Slower, TooSoon, Kept, Switched, NotWanted, Joined };

struct HitOutcome
{
    CopHit way;
    bool joined = false;  // joinPursuitWith's
    reg32 radioEax = 0;   // what the join's radio word left in eax
};

/* 0x402cb0: cop (not a player's) hit by racer: if the racer is no slower than 0x5360ec of the cop
 * (or the cop near still) and may be chased, a cop on another player's car -- not one with its
 * target pulled over that is parked (+0x70c 6) or alone -- gives that one up for this one; a free
 * cop takes it on where it wants to (copsWantedOf); each with its radio level up and
 * joinPursuitOf kind 1.  nullopt when the game stopped. */
std::optional<HitOutcome> copHitByCarOf(App* app, CPU& cpu, reg32 cop, reg32 racer)
{
    if ((byte(app, racer + kCarFlags) & kFlagRacer) == 0 || (byte(app, cop + kCarFlags) & kFlagPlayer))
        return HitOutcome{CopHit::Nothing};
    if (!compareAbove(cpu, 1.0, f32(app, cop + kCarSpeed))
        && !compareAbove(cpu, fp::div(f32(app, racer + kCarSpeed), f32(app, cop + kCarSpeed)), f64(app, kHitRatio)))
        return HitOutcome{CopHit::Slower};
    if (i32(at(app, kClock)) <= i32(at(app, racer + kCarNextChase)))
        return HitOutcome{CopHit::TooSoon};
    if (at(app, cop + kCarChased) != 0)
    {
        const reg32 pursuit = at(app, cop + kCarPursuit);
        if (at(app, pursuit + kPursuitState) == kPulledOver
            && (at(app, cop + kCarDriverStep) == kParkedStep || at(app, pursuit + kPursuitCopCount) == 0))
            return HitOutcome{CopHit::Kept};
        if (racer == at(app, cop + kCarChased) || (byte(app, racer + kCarFlags) & kFlagPlayer) == 0)
            return HitOutcome{CopHit::Kept};
        if (!giveUpPursuitOf(app, cpu, cop))
            return std::nullopt;
        raiseRadioLevel(app, cop);
        const std::optional<Joining> joining = joinPursuitWith(app, cpu, kJoinHit, cop, racer, 0.0f);
        if (!joining)
            return std::nullopt;
        return HitOutcome{CopHit::Switched, joining->joined, joining->radioEax};
    }
    const std::optional<reg32> wanted = copsWantedOf(app, cpu, cop, racer);
    if (!wanted)
        return std::nullopt;
    if (*wanted == 0)
        return HitOutcome{CopHit::NotWanted};
    raiseRadioLevel(app, cop);
    const std::optional<Joining> joining = joinPursuitWith(app, cpu, kJoinHit, cop, racer, 0.0f);
    if (!joining)
        return std::nullopt;
    return HitOutcome{CopHit::Joined, joining->joined, joining->radioEax};
}

/* A node's record, its lanes to the left (the high nibble of +0x13) and to the right (the low). */
reg32 nodeRecord(App* app, reg32 node) { return at(app, kNodes) + node * kNodeSize; }
i32 lanesLeft(App* app, reg32 record) { return i32((byte(app, record + kNodeLanes) >> 4) & 0xf); }
i32 lanesRight(App* app, reg32 record) { return i32(byte(app, record + kNodeLanes) & 0xf); }

/* 0x402d90: car's lane votes against the road's edge: 100 against (0x56eeb8) a lane off the
 * node's lanes; else 100 for the lane to the left (0x56eeb4, a lane below 7) or the right
 * (0x56eebc). */
void voteRoadLaneOf(App* app, reg32 car)
{
    const reg32 record = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    const bool off = 6 - lanesLeft(app, record) > lane || !(lanesRight(app, record) + 7 >= lane);
    const reg32 vote = off ? kVoteOff : lane < 7 ? kVoteLeft : kVoteRight;
    put(app, vote, at(app, vote) + kVote);
}

/* 0x402d90: voteRoadLaneOf car eax; eax as the code leaves it: the node's record where the lane
 * is off to the left, else the right's lanes plus 7. */
void voteRoadLane(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const reg32 record = nodeRecord(app, at(app, car + kCarNode));
    const reg32 eax = 6 - lanesLeft(app, record) > i32(at(app, car + kCarLane)) ? record
                                                                                : reg32(lanesRight(app, record) + 7);
    voteRoadLaneOf(app, car);
    cpu.eax = eax;
    cpu.esp += 4;
}

/* 0x402f70: whether car's lane is off its node's lanes. */
bool laneOffRoadOf(App* app, reg32 car)
{
    const reg32 record = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    return 7 - lanesLeft(app, record) > lane || lanesRight(app, record) + 6 < lane;
}

/* 0x402f70: laneOffRoadOf car eax, 1 or 0 in eax. */
void laneOffRoad(App* app, CPU& cpu)
{
    cpu.eax = laneOffRoadOf(app, cpu.eax) ? 1 : 0;
    cpu.esp += 4;
}

// How a cop's parking went: a racer near, parked.
enum class Park { RacerNear, Parked };

/* 0x403180: cop parks at its roadblock, unless a racer is within 0x5360f4: put down there
 * (0x4a0980), turned across the road (a quarter or three quarters of a turn by the order's +7,
 * ea::yRotation into the frame's matrix, the orientation by it with 0x4e0280), settled (0x4a2690),
 * its wanted speed 0, its mark of the roadblock (+0x621 bit 8) off, stopped (0x40ce70).  The
 * matrix on the guest's stack, handed to the callees.  nullopt when the game stopped. */
std::optional<Park> parkCopOf(App* app, CPU& cpu, reg32 cop)
{
    const GuestFrame frame(cpu, 0x18, 0x28);
    const reg32 turn = frame - 0x28;
    for (reg32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
    {
        const std::optional<Float> apart = distanceSize(app, cpu, cop + kCarPlace, at(app, i * 4 + kRacers) + kCarPlace);
        if (!apart)
            return std::nullopt;
        if (compareBelow(cpu, Float(float(*apart)), f64(app, kParkReach)))
            return Park::RacerNear;
    }
    if (!callWith(app, cpu, kParkCar, [&] {
            cpu.eax = cop;
            cpu.edx = gameSettings(app).backwards;
        }))
        return std::nullopt;
    const reg32 angle = byte(app, at(app, cop + kCarOrder) + kOrderFlag) != 0 ? kQuarterTurnBits : kThreeQuartersBits;
    if (!yRotation(app, cpu, turn, angle) || !multiply(app, cpu, cop + kCarOrientation, turn, cop + kCarOrientation)
        || !leftInEax(cpu, feCore<0x4a2690, frontend::simb::settle>(app, cpu, cop)))
        return std::nullopt;
    const reg8 side = byte(app, cop + kCarSide);
    put(app, cop + kCarWanted, 0);
    putByte(app, cop + kCarSide, side & ~kSideRoadblock);
    // By address: parkCop leaves the eax it leaves.
    if (!callFor(app, cpu, kStop, cop))
        return std::nullopt;
    return Park::Parked;
}

/* 0x403180: parkCopOf cop eax; eax as the stop leaves it, or with a racer near the distance's
 * with the compare's status word. */
void parkCop(App* app, CPU& cpu)
{
    const std::optional<Park> park = parkCopOf(app, cpu, cpu.eax);
    if (!park)
        return;
    if (*park == Park::RacerNear)
        cpu.eax = withStatus(cpu, cpu.eax);
    cpu.esp += 4;
}

/* 0x403d30: cop at its roadblock with its siren off: the siren on (0x4c9dd0: its index, its node,
 * lane 6 or 7, the clock), the roadblock's side on the radio (0x40b9c0: a side 0..2 by the
 * index's two low bits, mirrored with the lane where the track runs backwards), its count of them
 * (+0x710's low nibble) one up to 15, its mark of passing off.  False when the game stopped. */
bool roadblockSirenOf(App* app, CPU& cpu, reg32 cop)
{
    if (at(app, at(app, cop + kCarCopIndex) * kSirenSize + kSirens) != 0)
        return true;
    const reg32 index = at(app, cop + kCarCopIndex);
    reg32 lane = index & 1 ? 6 : 7, side = index & 1 ? 0 : 2;
    if (index & 2)
        side = 1;
    if (gameSettings(app).backwards != 0)
    {
        side = 2 - side;
        lane = 0xe - lane;
    }
    if (!leftInEax(cpu, feCore<0x4c9dd0, frontend::simc::sirenOn>(app, cpu, at(app, cop + kCarCopIndex),
                                                                    at(app, cop + kCarNode), lane, at(app, kClock))))
        return false;
    if (!guest::direct<0x40b9c0, pursuit::radioStoppedOf>(app, cpu, cop, side))
        return false;
    if (i32(at(app, cop + kCarCopFlags) & 0xf) < 0xf)
        put(app, cop + kCarCopFlags, at(app, cop + kCarCopFlags) + 1);
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopPassed);
    return true;
}

/* 0x403d30: roadblockSirenOf cop eax; eax as the code leaves it (worked out before, as the callees
 * write none of it): the siren's record's offset where it was on, else the count before. */
void roadblockSiren(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 siren = at(app, cop + kCarCopIndex) * kSirenSize;
    const reg32 eax = at(app, siren + kSirens) != 0 ? siren : at(app, cop + kCarCopFlags) & 0xf;
    if (!roadblockSirenOf(app, cpu, cop))
        return;
    cpu.eax = eax;
    cpu.esp += 4;
}

// How a watch from a roadblock or a second car went: still to watch, the car past (quiet, or on
// the radio now), too far round the track, done.
enum class WatchWay { Watching, Passed, Radioed, TooFar, Done };

struct Watch
{
    WatchWay way;
    i32 value = 0;      // the gap, or the quarter's negation too far
    reg32 radioEax = 0;  // what the radio's word left in eax, where it was said now
};

/* The new order of kind 2 by cop's index a watch ends with. */
void orderAfterWatch(App* app, reg32 cop)
{
    giveOrderOf(app, cop, 0, kOrderAtNode, 0, 0, reg8(at(app, cop + kCarCopIndex) & 1), 0);
}

/* 0x403df0: cop at a roadblock watches the car it is set for (+0x730) against the siren's node
 * (+0x34 of its siren's record): within 30 nodes past it, the radio's word that the car passed
 * (0x40bca0, once, +0x711 bit 0x20); 50 nodes past it (but not a quarter of the track), the siren
 * off, the roadblock done (the pursuit's roadblock car cleared) and a new order of kind 2 by its
 * index.  nullopt when the game stopped. */
std::optional<Watch> watchRoadblockOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 index = at(app, cop + kCarCopIndex);
    const reg32 carNode = at(app, at(app, cop + kCarOrderFrom) + kCarNode);
    const std::optional<i32> gap = nodeGap(app, cpu, at(app, index * kSirenSize + kSirens + kSirenNode), carNode);
    if (!gap)
        return std::nullopt;
    const i32 along = alongTrack(app, *gap);
    WatchWay way = WatchWay::Watching;
    reg32 radioEax = 0;
    if (along < 0 && along > kPassedGap)
    {
        way = WatchWay::Passed;
        const reg8 copKind = byte(app, cop + kCarCopKind);
        if ((copKind & kCopPassed) == 0)
        {
            putByte(app, cop + kCarCopKind, copKind | kCopPassed);
            const std::optional<reg32> radio = guest::direct<0x40bca0, pursuit::radioResumeOf>(app, cpu, cop);
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
    if (!sirenOff(app, cpu, index))
        return std::nullopt;
    const reg8 copKind = byte(app, cop + kCarCopKind);
    const reg32 car = at(app, cop + kCarOrderFrom);
    putByte(app, cop + kCarCopKind, copKind & ~kCopRoadblock);
    if (car != 0)
    {
        const reg32 pursuit = at(app, car + kCarPursuit);
        if (cop == at(app, pursuit + kPursuitRoadblock))
            put(app, pursuit + kPursuitRoadblock, 0);
    }
    orderAfterWatch(app, cop);
    return Watch{WatchWay::Done};
}

/* 0x403df0: watchRoadblockOf cop eax; eax as the code leaves it on each way out: the gap (its ah
 * the cop's +0x711 where the car had passed already, as the radio leaves it where it is told now),
 * the quarter, the order. */
void watchRoadblock(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Watch> watch = watchRoadblockOf(app, cpu, cop);
    if (!watch)
        return;
    switch (watch->way)
    {
    case WatchWay::Watching:
    case WatchWay::TooFar:
        cpu.eax = reg32(watch->value);
        break;
    case WatchWay::Passed:
        cpu.eax = (reg32(watch->value) & 0xffff00ff) | reg32(byte(app, cop + kCarCopKind)) << 8;
        break;
    case WatchWay::Radioed:
        cpu.eax = watch->radioEax;
        break;
    case WatchWay::Done:
        cpu.eax = at(app, cop + kCarOrder);
        break;
    }
    cpu.esp += 4;
}

/* 0x403ed0: cop waiting as a second car watches its car (+0x730): 5 nodes past it (but not a
 * quarter of the track), its wait over -- within 20 nodes, parked (step 10), the radio's word that
 * it missed (0x40bd00) -- and a new order of kind 2 by its index.  nullopt when the game
 * stopped. */
std::optional<Watch> watchSecondCarOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 car = at(app, cop + kCarOrderFrom);
    const std::optional<i32> gap = nodeGap(app, cpu, at(app, cop + kCarNode), at(app, car + kCarNode));
    if (!gap)
        return std::nullopt;
    const i32 along = alongTrack(app, *gap);
    if (along > kBehindGap)
        return Watch{WatchWay::Watching, *gap};
    const i32 quarter = -(i32(at(app, kNodeCount)) / 4);
    if (along < quarter)
        return Watch{WatchWay::TooFar, quarter};
    if (along < 0 && along > kMissedGap && at(app, cop + kCarDriverStep) == kWaitingStep
        && !guest::direct<0x40bd00, pursuit::radioAbortOf>(app, cpu, cop))
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopSent);
    putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideRoadblock);
    orderAfterWatch(app, cop);
    return Watch{WatchWay::Done};
}

/* 0x403ed0: watchSecondCarOf cop eax; eax the gap, the quarter or the order. */
void watchSecondCar(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Watch> watch = watchSecondCarOf(app, cpu, cop);
    if (!watch)
        return;
    cpu.eax = watch->way == WatchWay::Done ? at(app, cop + kCarOrder) : reg32(watch->value);
    cpu.esp += 4;
}

/* 0x404340: cop sent to cut off car: an order of kind 3 at the node midway between them, wrapped
 * round the track, 8 and flag 0.  False when the game stopped. */
bool sendCopAheadOf(App* app, CPU& cpu, reg32 cop, reg32 car)
{
    const reg32 copNode = at(app, cop + kCarNode);
    const std::optional<i32> gap = nodeGap(app, cpu, at(app, car + kCarNode), copNode);
    if (!gap)
        return false;
    const i32 half = (*gap - (*gap >> 31)) >> 1;
    reg32 node = reg32(half) + copNode;
    if (half >= 0)
    {
        const reg32 count = at(app, kNodeCount);
        if (i32(node) >= i32(count))
            node -= count;
    }
    else if (i32(node) < 0)
        node += at(app, kNodeCount);
    giveOrderOf(app, cop, car, kOrderCutOff, reg16(node), 8, 0, 0);
    return true;
}

/* 0x404340: sendCopAheadOf cop eax, car edx; eax the order, edx as giveOrder leaves it. */
void sendCopAhead(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax, car = cpu.edx;
    const bool player = byte(app, cop + kCarFlags) & kFlagPlayer;
    if (!sendCopAheadOf(app, cpu, cop, car))
        return;
    cpu.eax = at(app, cop + kCarOrder);
    cpu.edx = player ? car : 0;
    cpu.esp += 4;
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
    reg32 radioEax = 0;  // what the join's radio word left in eax
};

/* 0x402e10: cop looks for a speeder: of the racers that may be chased and are in the race, within
 * 15 nodes, each's speeding noted (noteSpeedingOf), the one the most cops are wanted for
 * (copsWantedOf); the cop joins its pursuit (joinPursuitOf kind 0) right away where its siren is
 * off and it was not told to wait (+0x711 bit 4) -- else unless it has waited less than 0x3c0
 * ticks (+0x72c) and the car is its order's, within 5 nodes, and has not been 0x100 ticks there
 * (+0x728).  No one found, both waits off.  nullopt when the game stopped. */
std::optional<Spot> spotSpeederOf(App* app, CPU& cpu, reg32 cop)
{
    Spot spot{SpotWay::None};
    const reg32 copNode = at(app, cop + kCarNode);
    i32 most = 0;
    reg32 speeder = 0;
    for (reg32 i = 0; i32(i) < i32(at(app, kRacerCount)); ++i)
    {
        const reg32 racer = at(app, i * 4 + kRacers);
        spot.last = Look::NotNow;
        if (i32(at(app, kClock)) <= i32(at(app, racer + kCarNextChase)))
            continue;
        if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfRace)
            continue;
        if ((byte(app, racer + kCarFlags) & kFlagRacer) && at(app, racer + kCarState) == kOutOfPursuits)
            continue;
        const std::optional<i32> gap = nodeGap(app, cpu, at(app, racer + kCarNode), copNode);
        if (!gap)
            return std::nullopt;
        const i32 size = *gap > 0 ? *gap : i32(neg(reg32(*gap)));
        spot.last = Look::Far;
        spot.value = size;
        if (size > kSpotReach)
            continue;
        if (!noteSpeedingOf(app, cpu, racer))
            return std::nullopt;
        const std::optional<reg32> wanted = copsWantedOf(app, cpu, cop, racer);
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
    const reg32 index = at(app, cop + kCarCopIndex);
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
            if ((spot.gap > 0 ? spot.gap : i32(neg(reg32(spot.gap)))) >= kOrderReach)
                return spot;
            if (at(app, cop + kCarWaitedAt) == 0)
                put(app, cop + kCarWaitedAt, at(app, kClock));
            spot.wait = Wait::TooSoon;
            if (!(i32(at(app, kClock) - at(app, cop + kCarWaitedAt)) > kTimeAtOrder))
                return spot;
        }
    }
    const std::optional<Joining> joining = joinPursuitWith(app, cpu, kJoinSighted, cop, speeder, 0.0f);
    if (!joining)
        return std::nullopt;
    spot.way = SpotWay::Joined;
    spot.joined = joining->joined;
    spot.radioEax = joining->radioEax;
    return spot;
}

/* 0x4039c0: cop calls in a roadblock, unless the race has no cops of its own (0x6fd51c), it chases
 * no one, the pursuit has one, or the pursuit is not yet hot enough (cops, getaways, tickets and
 * time over 4): the first time only the radio's call (0x40b910, +0x54 bit 2, the caller +0x5c);
 * then the free cop (not a player's, no order) ahead of the target more than 50 nodes and least
 * far (under three quarters of the track) gets the order (kind 0 with 3, 6 nodes before it) and is
 * the roadblock.  Whether it got one; nullopt when the game stopped. */
std::optional<bool> callRoadblockOf(App* app, CPU& cpu, reg32 cop)
{
    i32 nearest = i32(at(app, kNodeCount) * 3) / 4;
    reg32 roadblock = 0;
    if (gameSettings(app).copCount != 0)
        return false;
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    if (at(app, pursuit + kPursuitRoadblock) != 0)
        return false;
    const reg32 heat = at(app, pursuit + kPursuitGaveUp) + (at(app, pursuit + kPursuitCopCount) - 1)
                       + (at(app, target + kCarTickets) - 2)
                       + reg32(i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) >> 11);
    if (i32(heat) < kRoadblockHeat)
        return false;
    if ((byte(app, pursuit + kPursuitCalls) & kPursuitRoadblockCall) == 0)
    {
        if (!guest::direct<0x40b910, pursuit::radioArrestOf>(app, cpu, cop))
            return std::nullopt;
        const reg32 own = at(app, cop + kCarPursuit);
        putByte(app, own + kPursuitCalls, byte(app, own + kPursuitCalls) | kPursuitRoadblockCall);
        put(app, pursuit + kPursuitCaller, cop);
        return false;
    }
    const reg32 targetNode = at(app, target + kCarNode);
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0
            || (byte(app, other + kCarCopKind) & kCopRoadblock))
            continue;
        const reg32 ahead = alongTrack(app, i32(at(app, other + kCarNode) - targetNode));
        if (i32(ahead) <= kRoadblockAhead || i32(ahead) >= nearest)
            continue;
        roadblock = other;
        nearest = i32(ahead);
    }
    if (roadblock == 0)
        return false;
    reg32 node = at(app, roadblock + kCarNode) - kRoadblockBefore;
    if (i32(node) < 0)
        node += at(app, kNodeCount);
    putByte(app, roadblock + kCarCopKind, byte(app, roadblock + kCarCopKind) & ~kCopSent);
    giveOrderOf(app, roadblock, at(app, cop + kCarChased), 0, reg16(node), 3, 0, 0);
    putByte(app, roadblock + kCarCopKind, byte(app, roadblock + kCarCopKind) | kCopRoadblock);
    put(app, pursuit + kPursuitRoadblock, roadblock);
    return true;
}

/* 0x4039c0: callRoadblockOf cop eax, 1 or 0 in eax. */
void callRoadblock(App* app, CPU& cpu)
{
    const std::optional<bool> called = callRoadblockOf(app, cpu, cpu.eax);
    if (!called)
        return;
    cpu.eax = *called ? 1 : 0;
    cpu.esp += 4;
}

std::optional<i32> sideOfOrderOf(App* app, CPU& cpu, reg32 car);
std::optional<reg32> atOrderNodeOf(App* app, CPU& cpu, reg32 car);

/* What setWantedSpeed's code leaves in eax after setWantedSpeedOf for cop: the top speed's bits
 * where it capped, else copTopSpeed's eax with the compare's status word; ah the first flip's
 * byte where the track runs backwards. */
reg32 setWantedSpeedEax(App* app, const CPU& cpu, reg32 cop, bool capped)
{
    reg32 eax = capped ? floatBits(copTopSpeedOf(app, cop)) : withStatus(cpu, copTopSpeedEax(app, cop));
    if (gameSettings(app).backwards != 0)
        eax = (eax & 0xffff00ff) | reg32(firstFlip(app, cop)) << 8;
    return eax;
}

/* 0x401430: setWantedSpeedOf for cop eax, the factor and the offset on the stack; ret 8. */
void setWantedSpeed(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<bool> capped =
        setWantedSpeedOf(app, cpu, cop, floatAt(app, cpu.esp + 4), floatAt(app, cpu.esp + 8));
    if (!capped)
        return;
    cpu.eax = setWantedSpeedEax(app, cpu, cop, *capped);
    cpu.esp += 4 + 8;
}

/* What chaseSpeed's code leaves in eax after chaseSpeedOf for cop went `way`: the cop, the
 * target, it with the compare's status word, the gap's size, as the timer leaves it, the top
 * speed's bits; ah the first flip's byte where the track runs backwards and there is a target. */
reg32 chaseSpeedEax(App* app, const CPU& cpu, reg32 cop, Chase way)
{
    const reg32 target = at(app, cop + kCarChased);
    reg32 eax = cop;
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
        eax = withStatus(cpu, cop);
        break;
    case Chase::BehindNear:
    {
        const i32 gap = i32(at(app, kGap));
        eax = gap > 0 ? reg32(gap) : neg(reg32(gap));
        break;
    }
    case Chase::BehindTimer:
        eax = cpu.eax;
        break;
    case Chase::AheadTop:
        eax = floatBits(copTopSpeedOf(app, cop));
        break;
    }
    if (gameSettings(app).backwards != 0)
        eax = (eax & 0xffff00ff) | reg32(firstFlip(app, cop)) << 8;
    return eax;
}

/* 0x402420: chaseSpeedOf cop eax. */
void chaseSpeed(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Chase> way = chaseSpeedOf(app, cpu, cop);
    if (!way)
        return;
    cpu.eax = chaseSpeedEax(app, cpu, cop, *way);
    cpu.esp += 4;
}

// The ways pursueTargetOf goes: no target, given up, the chase's speed, behind the car, its top
// speed, and that slowed at night.
enum class Pursue { NoTarget, GaveUp, Chasing, Behind, Top, TopAtNight };

struct Pursuit
{
    Pursue way;
    bool capped = false;           // setWantedSpeedOf's, for the patrol's speed
    Chase chase = Chase::NoTarget;  // chaseSpeedOf's
};

/* 0x4025c0: cop after the car it chases: too far behind (more than the car's tickets allow,
 * 0x5521ac or for a fast cop 0x5521b0) or the car out of the race, it gives up (giveUpPursuitOf),
 * a timer (0x405fd0), its side flipped and the patrol's speed (setWantedSpeedOf 1.19, 0); pulled
 * over or within 15 nodes, the chase's speed (chaseSpeedOf); ahead of the car, onto the car's side
 * and the patrol's speed; otherwise its top speed and a timer -- at night (0x6fd4c8) 0x5360c4 of
 * it for a player's car not braking (+0x660).  nullopt when the game stopped. */
std::optional<Pursuit> pursueTargetOf(App* app, CPU& cpu, reg32 cop)
{
    const auto patrol = [&](Pursue way) -> std::optional<Pursuit> {
        const std::optional<bool> capped = setWantedSpeedOf(app, cpu, cop, floatOf(kPatrolFactorBits), 0.0f);
        if (!capped)
            return std::nullopt;
        return Pursuit{way, *capped};
    };
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return patrol(Pursue::NoTarget);
    const i32 signedGap = i32(at(app, kGap));
    const reg32 gap = signedGap > 0 ? reg32(signedGap) : neg(reg32(signedGap));
    const reg32 pursuit = at(app, cop + kCarPursuit);
    bool giveUp = (byte(app, target + kCarFlags) & kFlagRacer) && at(app, target + kCarState) == kOutOfRace;
    if (!giveUp)
    {
        const reg32 table = byte(app, cop + kCarCopKind) & kCopFast ? kFastGiveUpGap : kGiveUpGap;
        giveUp = gap > at(app, (at(app, target + kCarTickets) << 4) + table);
    }
    if (giveUp)
    {
        if (!giveUpPursuitOf(app, cpu, cop) || !setTimer(app, cpu, cop, 1, 0))
            return std::nullopt;
        putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideOther);
        return patrol(Pursue::GaveUp);
    }
    if (at(app, pursuit + kPursuitState) == kPulledOver || !(i32(gap) > kChaseGap))
    {
        const std::optional<Chase> chase = chaseSpeedOf(app, cpu, cop);
        if (!chase)
            return std::nullopt;
        return Pursuit{Pursue::Chasing, false, *chase};
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
    if (!setTimerLeavingEax(app, cpu, cop, 1, 0))
        return std::nullopt;
    flipWantedSign(app, cop);
    if (gameSettings(app).night == 0)
        return Pursuit{Pursue::Top};
    const reg32 chased = at(app, cop + kCarChased);
    if ((byte(app, chased + kCarFlags) & kFlagPlayer) == 0 || at(app, chased + kCarBraking) != 0)
        return Pursuit{Pursue::TopAtNight};
    putFloat(app, cop + kCarWanted, float(fp::mul(f32(app, cop + kCarWanted), f64(app, kNightShare))));
    return Pursuit{Pursue::TopAtNight};
}

/* 0x4025c0: pursueTargetOf cop eax; eax as the code leaves it on each way out: setWantedSpeed's
 * or chaseSpeed's; with its top speed the timer's, its al the wanted speed's sign byte where the
 * car is on the other side (the flips' second), at night the target. */
void pursueTarget(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Pursuit> pursuit = pursueTargetOf(app, cpu, cop);
    if (!pursuit)
        return;
    switch (pursuit->way)
    {
    case Pursue::NoTarget:
    case Pursue::GaveUp:
    case Pursue::Behind:
        cpu.eax = setWantedSpeedEax(app, cpu, cop, pursuit->capped);
        break;
    case Pursue::Chasing:
        cpu.eax = chaseSpeedEax(app, cpu, cop, pursuit->chase);
        break;
    case Pursue::Top:
        if (byte(app, cop + kCarSide) & kSideOther)
            cpu.al = byte(app, cop + kCarWantedSign);
        break;
    case Pursue::TopAtNight:
        cpu.eax = at(app, cop + kCarChased);
        break;
    }
    cpu.esp += 4;
}

/* 0x403030: cop takes its order, by the table at 0x403018: 0 to its node (step 3 there, else on
 * toward it, step 1); 1 step 2; 3 a roadblock at its node (step 7, the roadblock's radio
 * 0x40b7f0); 4 to its node for a second car (step 8; 10 once there); any other its side by the
 * order's (+6), step 1.  False when the game stopped. */
bool takeOrderOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 order = at(app, cop + kCarOrder);
    const auto mark = [&](reg32 step) {
        put(app, cop + kCarOnPursuit, 1);
        const reg8 copFlags = byte(app, cop + kCarCopFlags);
        put(app, cop + kCarDriverStep, step);
        putByte(app, cop + kCarCopFlags, copFlags | kCopChasing);
    };
    // jmp [kind * 4 + 0x403018]: 0x403075, 0x4030a9, 0x40304e, 0x4030ba, 0x40310e.
    switch (byte(app, order + kOrderKind))
    {
    case kOrderToNode:
    {
        const std::optional<reg32> there = atOrderNodeOf(app, cpu, cop);
        if (!there)
            return false;
        if (*there == 0)
        {
            if (!sideOfOrderOf(app, cpu, cop))
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
        const std::optional<reg32> there = atOrderNodeOf(app, cpu, cop);
        if (!there)
            return false;
        if (*there != 0)
            return true;
        if (!sideOfOrderOf(app, cpu, cop))
            return false;
        if (at(app, cop + kCarDriverStep) == kStepRoadblock)
            return true;
        const reg8 copFlags = byte(app, cop + kCarCopFlags);
        put(app, cop + kCarDriverStep, kStepRoadblock);
        putByte(app, cop + kCarCopFlags, copFlags | kCopChasing);
        put(app, cop + kCarOnPursuit, 1);
        guest::direct<0x40b7f0, pursuit::radioNothingFor>(app, cpu, cop);
        return !cpu.terminate;
    }
    case kOrderSecondCar:
    {
        const std::optional<reg32> there = atOrderNodeOf(app, cpu, cop);
        if (!there)
            return false;
        if (*there != 0)
        {
            mark(kWaitingStep);
            return true;
        }
        if (!sideOfOrderOf(app, cpu, cop))
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
void takeOrder(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 kind = byte(app, at(app, cop + kCarOrder) + kOrderKind);
    if (!takeOrderOf(app, cpu, cop))
        return;
    cpu.eax = kind;
    cpu.esp += 4;
}

/* 0x403b20: cop calls in two cars to box in a player's car, unless the race has no roadblocks
 * (0x55eb2e bit 0x20, 0x6fd51c) or the pursuit is not hot enough (over 3): the radio's call once
 * (0x40b860, +0x54 bit 4, the caller +0x5c); the two free cops ahead of the car more than 50
 * nodes (under three quarters of the track), the nearest two, if within 400 nodes of each other,
 * each get the order (kind 4, the node midway between them, the farther flagged) and are marked
 * (+0x710 0x410, +0x734).  Whether they did; nullopt when the game stopped. */
std::optional<bool> callTwoCarsOf(App* app, CPU& cpu, reg32 cop)
{
    i32 nearestAhead = i32(at(app, kNodeCount) * 3) / 4;
    reg32 nearest = 0, runnerUp = 0;
    if (byte(app, kRaceSwitches) & kRaceNoRams)
        return false;
    if (gameSettings(app).copCount != 0)
        return false;
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0 || (byte(app, target + kCarFlags) & kFlagPlayer) == 0)
        return false;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    const reg32 heat = at(app, pursuit + kPursuitGaveUp) + (at(app, pursuit + kPursuitCopCount) - 1)
                       + at(app, target + kCarTickets)
                       + reg32(i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) >> 11);
    if (i32(heat) < kTwoCarsHeat)
        return false;
    if ((byte(app, pursuit + kPursuitCalls) & kPursuitSecondCall) == 0)
    {
        if (!guest::direct<0x40b860, pursuit::radioTicketOf>(app, cpu, cop))
            return std::nullopt;
        const reg32 own = at(app, cop + kCarPursuit);
        putByte(app, own + kPursuitCalls, byte(app, own + kPursuitCalls) | kPursuitSecondCall);
        put(app, pursuit + kPursuitCaller, cop);
    }
    const reg32 targetNode = at(app, at(app, cop + kCarChased) + kCarNode);
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0)
            continue;
        const reg8 otherKind = byte(app, other + kCarCopKind);
        if ((otherKind & kCopRoadblock) || (otherKind & kCopSent))
            continue;
        const std::optional<i32> gap = nodeGap(app, cpu, at(app, other + kCarNode), targetNode);
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
    const std::optional<i32> apart = nodeGap(app, cpu, at(app, runnerUp + kCarNode), at(app, nearest + kCarNode));
    if (!apart)
        return std::nullopt;
    if (*apart >= kTwoCarsApart)
        return false;
    const i32 half = (*apart - (*apart >> 31)) >> 1;
    reg32 node = reg32(half) + at(app, nearest + kCarNode);
    if (half >= 0)
    {
        const reg32 count = at(app, kNodeCount);
        if (i32(node) >= i32(count))
            node -= count;
    }
    else if (i32(node) < 0)
        node += at(app, kNodeCount);
    const auto mark = [&](reg32 car) {
        const reg16 copFlags = word(app, car + kCarCopFlags);
        put(app, car + kCarOnPursuit, 1);
        putWord(app, car + kCarCopFlags, copFlags | kCopsBoxing);
    };
    giveOrderOf(app, runnerUp, at(app, cop + kCarChased), kOrderTwoCars, reg16(node), 0, 0, 1);
    mark(runnerUp);
    giveOrderOf(app, nearest, at(app, cop + kCarChased), kOrderTwoCars, reg16(node), 0, 0, 0);
    mark(nearest);
    return true;
}

/* 0x403b20: callTwoCarsOf cop eax, 1 or 0 in eax. */
void callTwoCars(App* app, CPU& cpu)
{
    const std::optional<bool> called = callTwoCarsOf(app, cpu, cpu.eax);
    if (!called)
        return;
    cpu.eax = *called ? 1 : 0;
    cpu.esp += 4;
}

/* 0x402110's test of whether car has stopped for the cops: its driver's state (+0x6e0) 7 for a
 * player's car, 6 for another. */
bool hasStoppedForCops(App* app, reg32 car)
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
std::optional<bool> targetStoppingOf(App* app, CPU& cpu, reg32 cop)
{
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return false;
    const reg32 pursuit = at(app, cop + kCarPursuit);
    const auto apart = [&] {
        return distanceSize(app, cpu, at(app, cop + kCarChased) + kCarPlace, cop + kCarPlace);
    };
    if (at(app, pursuit + kPursuitState) == kPulledOver && hasStoppedForCops(app, target))
    {
        const std::optional<Float> d = apart();
        if (!d)
            return std::nullopt;
        if (compareBelow(cpu, Float(float(*d)), f64(app, kStoppedReach)))
            return true;
    }
    const float reach =
        float(fp::mul(Float(i32(at(app, pursuit + kPursuitStoppedBy)) < 3 ? 1 : 2), f64(app, kStopReach)));
    const std::optional<Float> d = apart();
    if (!d)
        return std::nullopt;
    bool close = compareBelow(cpu, Float(float(*d)), Float(reach));
    if (close)
        close = !compareAbove(cpu, f32(app, at(app, cop + kCarChased) + kCarSpeed), f32(app, kStoppingSpeed));
    if (close)
        close = word(app, cop + kCarAirborne) == 0;
    if (close)
        close = compareAbove(cpu, f32(app, cop + kCarForward), f64(app, kStopRolling));
    if (close)
    {
        if ((byte(app, cop + kCarCopFlags) & kCopCalledStop) == 0)
        {
            const reg32 car = at(app, cop + kCarChased);
            reg32 wait = kStopCallTicks;
            if ((byte(app, car + kCarFlags) & kFlagPlayer) == 0)
            {
                const i32 laps = i32(at(app, car + kCarLaps) - at(app, at(app, kLeader) + kCarLaps));
                if (laps > kLapsApart)
                    wait = kStopCallSoon;
                else if (laps < -kLapsApart)
                    wait = kStopCallLate;
            }
            if (!callAboutCar(app, cpu, cop, at(app, cop + kCarChased)))
                return std::nullopt;
            const reg32 due = at(app, kClock) + wait;
            const reg8 copFlags = byte(app, cop + kCarCopFlags);
            put(app, cop + kCarRadioCallDue, due);
            putByte(app, cop + kCarCopFlags, copFlags | kCopCalledStop);
            return false;
        }
        const reg32 now = at(app, kClock);
        const reg32 due = at(app, cop + kCarRadioCallDue);
        if (i32(now) > i32(due))
        {
            put(app, pursuit + kPursuitState, kPulledOver);
            put(app, pursuit + kPursuitStopEnds, now + kStopTicks);
            return hasStoppedForCops(app, at(app, cop + kCarChased));
        }
        const reg32 car = at(app, cop + kCarChased);
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
void targetStopping(App* app, CPU& cpu)
{
    const std::optional<bool> stopped = targetStoppingOf(app, cpu, cpu.eax);
    if (!stopped)
        return;
    cpu.eax = *stopped ? 1 : 0;
    cpu.esp += 4;
}

// How callBackupOf went: no target, no more cops wanted, the nearest of the others sent last,
// else the cops looked along (the fast ones maybe sent among them).
enum class Backup { NoTarget, NotWanted, NearestSent, Looked };

struct BackupCall
{
    Backup way;
    reg32 sent = 0;  // the nearest, sent last
};

/* 0x4043a0: cop calls backup for its target where more cops are wanted (copsWantedOf): the first
 * time, after 0x280 ticks alone, the radio's call (0x40b720, +0x54 bit 1); then of the free cops
 * not at roadblocks, the fast ones (+0x711 bit 1) ahead of the target within 8 times its tickets'
 * gap are each sent to cut it off (sendCopAheadOf), and of the others the nearest on the other
 * side of it than the cop, within 200 nodes and twice the gap, is sent -- unless it is setting up
 * a roadblock (step 7).  nullopt when the game stopped. */
std::optional<BackupCall> callBackupOf(App* app, CPU& cpu, reg32 cop)
{
    i32 nearest = kBackupReach;
    reg32 chosen = 0;
    const reg32 target = at(app, cop + kCarChased);
    if (target == 0)
        return BackupCall{Backup::NoTarget};
    const std::optional<reg32> wanted = copsWantedOf(app, cpu, 0, target);
    if (!wanted)
        return std::nullopt;
    if (*wanted == 0)
        return BackupCall{Backup::NotWanted};
    const reg32 pursuit = at(app, cop + kCarPursuit);
    const reg8 calls = byte(app, pursuit + kPursuitCalls);
    if ((calls & (kPursuitBackupCall | kPursuitRoadblockCall | kPursuitSecondCall)) == 0
        && i32(at(app, kClock) - at(app, pursuit + kPursuitStarted)) > i32(kBackupAlone)
        && at(app, pursuit + kPursuitCopCount) == 1)
    {
        if (!guest::direct<0x40b720, pursuit::radioRoadblockOf>(app, cpu, cop))
            return std::nullopt;
        putByte(app, pursuit + kPursuitCalls, byte(app, pursuit + kPursuitCalls) | kPursuitBackupCall);
    }
    const reg32 targetNode = at(app, target + kCarNode);
    const std::optional<i32> own = nodeGap(app, cpu, at(app, cop + kCarNode), targetNode);
    if (!own)
        return std::nullopt;
    for (reg32 i = 0; i32(i) < i32(at(app, kCopCount)); ++i)
    {
        const reg32 other = at(app, i * 4 + kCops);
        if ((byte(app, other + kCarFlags) & kFlagPlayer) || at(app, other + kCarChased) != 0)
            continue;
        const reg8 otherKind = byte(app, other + kCarCopKind);
        if ((otherKind & kCopRoadblock) || (otherKind & kCopSent))
            continue;
        const std::optional<i32> gap = nodeGap(app, cpu, at(app, other + kCarNode), targetNode);
        if (!gap)
            return std::nullopt;
        const reg32 size = *gap > 0 ? reg32(*gap) : neg(reg32(*gap));
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
            if (!sendCopAheadOf(app, cpu, other, target))
                return std::nullopt;
            continue;
        }
        if (i32(reg32(*own) ^ reg32(*gap)) >= 0)
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
        if (!sendCopAheadOf(app, cpu, chosen, target))
            return std::nullopt;
        return BackupCall{Backup::NearestSent, chosen};
    }
    return BackupCall{Backup::Looked};
}

/* What spotSpeeder's code leaves in eax after spotSpeederOf for cop: none found, what the last look
 * left (the cop's node for no racers, the clock, the gap's size, the cops wanted, the gap);
 * waiting, the ticks waited, the gap's size; joined, the join's (its kind where the cop was on a
 * pursuit, else as the radio leaves it). */
reg32 spotSpeederEax(App* app, const CPU& cpu, reg32 cop, const Spot& spot)
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
            return reg32(spot.value);
        }
        break;
    case SpotWay::Waiting:
        switch (spot.wait)
        {
        case Wait::NotOrders:
            return at(app, kClock) - at(app, cop + kCarWaitedFrom);
        case Wait::TooFar:
            return spot.gap > 0 ? reg32(spot.gap) : neg(reg32(spot.gap));
        case Wait::TooSoon:
            return at(app, kClock) - at(app, cop + kCarWaitedAt);
        }
        break;
    case SpotWay::Joined:
        return spot.joined ? spot.radioEax : kJoinSighted;
    }
    return cpu.eax;
}

/* 0x402e10: spotSpeederOf cop eax. */
void spotSpeeder(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Spot> spot = spotSpeederOf(app, cpu, cop);
    if (!spot)
        return;
    cpu.eax = spotSpeederEax(app, cpu, cop, *spot);
    cpu.esp += 4;
}

/* What copStopsBy's code's choice of the cop that stops leaves in eax, read before
 * copStopsByOf for cop (nothing in the look writes): the cop where there was none to make, 10000's
 * bits where one had stopped already, else the count looked along. */
reg32 copStopsByChoice(App* app, reg32 cop)
{
    const reg32 pursuit = at(app, cop + kCarPursuit);
    const reg32 count = at(app, pursuit + kPursuitCopCount);
    if (i32(count) > 1 && (byte(app, cop + kCarFlags) & kFlagPlayer))
        return at(app, pursuit + kPursuitStoppedBy) != 0 ? kFarBits : count;
    return cop;
}

/* What copStopsBy's code leaves in eax after copStopsByOf for cop went `way`: dropCop's (the
 * pursuit for a player's cop, else the cop), the choice's, the car it stops by. */
reg32 copStopsByEax(App* app, reg32 cop, StopBy way, reg32 choice, reg32 pursuit, bool player)
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
void copStopsBy(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const reg32 choice = copStopsByChoice(app, cop), pursuit = at(app, cop + kCarPursuit);
    const bool player = byte(app, cop + kCarFlags) & kFlagPlayer;
    const std::optional<StopBy> way = copStopsByOf(app, cpu, cop);
    if (!way)
        return;
    cpu.eax = copStopsByEax(app, cop, *way, choice, pursuit, player);
    cpu.esp += 4;
}

/* copStopsByOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool copStopsByDone(App* app, CPU& cpu, reg32 cop)
{
    return copStopsByOf(app, cpu, cop).has_value();
}

/* What callBackup's code leaves in eax after callBackupOf for cop: the cop with no target, 0 with
 * no more wanted, the order of the nearest sent last, else the cops' count looked along. */
reg32 callBackupEax(App* app, reg32 cop, const BackupCall& call)
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
    return count > 0 ? reg32(count) : 0;
}

/* 0x4043a0: callBackupOf cop eax. */
void callBackup(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<BackupCall> call = callBackupOf(app, cpu, cop);
    if (!call)
        return;
    cpu.eax = callBackupEax(app, cop, *call);
    cpu.esp += 4;
}

// What copDriverOf's last step was, for what its code leaves in eax: a value of the step's, that
// with the compare's status word, as the callee by address last called leaves it, or one of the
// file's own whose code leaves its own.
enum class DriveEnd { Value, Status, Callee, Spotted, StopsBy, Backup };

struct Drive
{
    DriveEnd end;
    reg32 value = 0;  // Value's and Status's: the step's result the code leaves (a gap, a flag, an order)
    Spot spot{SpotWay::None};
    StopBy stopsBy = StopBy::Counted;
    reg32 choice = 0, pursuit = 0;
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
std::optional<Drive> copDriverOf(App* app, CPU& cpu, reg32 cop)
{
    // The original's frame (its six registers pushed), below which parkCopOf's and copStopsByOf's
    // go: the matrix and the places they hand on are at the original's depth.
    const GuestFrame frame(cpu, 0x18, 0);
    const auto value = [](reg32 v) { return Drive{DriveEnd::Value, v}; };
    const auto spotted = [](const Spot& s) {
        Drive d{DriveEnd::Spotted};
        d.spot = s;
        return d;
    };
    const auto patrol = [&](reg32 factor, reg32 offset) {
        return drivePatrolOf(app, cpu, cop, floatOf(factor), floatOf(offset));
    };
    const auto chasing = [&] { return at(app, cop + kCarChased) != 0; };
    const auto step = [&](reg32 s) { put(app, cop + kCarDriverStep, s); };
    const auto call = [&](reg32 target) { return callFor(app, cpu, target, cop); };
    const std::optional<reg32> ready = guest::direct<0x40df10, aiplan::stepLoading>(app, cpu, cop);
    if (!ready)
        return std::nullopt;
    if (*ready == 0)
        return value(0);
    if (!clearBytes(app, cpu, kTurning, 0, kSizingBytes))
        return std::nullopt;
    putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopHolds);
    if (isReversing(app, cop))
    {
        if (!radioStopOf(app, cpu, cop))
            return std::nullopt;
    }
    else
        putByte(app, cop + kCarCopKind, byte(app, cop + kCarCopKind) & ~kCopToldStop);
    if (at(app, at(app, cop + kCarCopIndex) * kSirenSize + kSirens) != 0 && !watchRoadblockOf(app, cpu, cop))
        return std::nullopt;
    if ((byte(app, cop + kCarCopKind) & kCopSent) && !watchSecondCarOf(app, cpu, cop))
        return std::nullopt;
    if (takeNewOrder(app, cop) && !takeOrderOf(app, cpu, cop))
        return std::nullopt;
    const reg32 now = at(app, cop + kCarDriverStep);
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
        const std::optional<Spot> spot = spotSpeederOf(app, cpu, cop);
        if (!spot)
            return std::nullopt;
        if (chasing())
        {
            step(kStepChase);
            return spotted(*spot);
        }
        const reg32 kind = byte(app, at(app, cop + kCarOrder) + kOrderKind);
        if (!takeOrderOf(app, cpu, cop))
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
        const std::optional<Spot> spot = spotSpeederOf(app, cpu, cop);
        if (!spot)
            return std::nullopt;
        const reg32 target = at(app, cop + kCarChased);
        if (target != 0)
        {
            step(kStepChase);
            if (now != kStepHold)
                return spotted(*spot);
        }
        const std::optional<reg32> there = atOrderNodeOf(app, cpu, cop);
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
                if (compareAbove(cpu, f32(app, cop + kCarSpeed), f64(app, kParkSpeed)))
                    return Drive{DriveEnd::Status, *there};
                step(kStepParking);
                const std::optional<reg32> radio = guest::direct<0x40bac0, pursuit::radioAmbulanceOf>(app, cpu, cop);
                if (!radio)
                    return std::nullopt;
                return value(*radio);
            }
            const reg32 order = at(app, cop + kCarOrder);
            putByte(app, order + kOrderKind, kOrderAtNode);
            step(kStepPatrol);
            const reg8 copFlags = byte(app, cop + kCarCopFlags);
            put(app, cop + kCarOnPursuit, target);
            putByte(app, cop + kCarCopFlags, copFlags & ~kCopChasing);
            return value(order);
        }
        const std::optional<i32> gap = sideOfOrderOf(app, cpu, cop);
        if (!gap)
            return std::nullopt;
        if (now == kStepHold)
            putByte(app, cop + kCarCopFlags, byte(app, cop + kCarCopFlags) | kCopToNode);
        return value(reg32(*gap));
    }
    case kStepParking:
    case kStepParked:
    {
        if (now == kStepParking)
        {
            const std::optional<reg32> there = atOrderNodeOf(app, cpu, cop);
            if (!there)
                return std::nullopt;
            if (*there != 0)
            {
                putByte(app, cop + kCarSide, byte(app, cop + kCarSide) | kSideRoadblock);
                if (!asCalled(cpu, [&] { return parkCopOf(app, cpu, cop); }))
                    return std::nullopt;
                if (byte(app, cop + kCarSide) & kSideRoadblock)
                {
                    if (!steerAtTarget(app, cpu, cop) || !chooseLanes(app, cpu, cop) || !call(kSteer))
                        return std::nullopt;
                }
                else
                    step(kStepParked);
            }
            else
            {
                putByte(app, cop + kCarSide, byte(app, cop + kCarSide) & ~kSideRoadblock);
                if (!sideOfOrderOf(app, cpu, cop) || !patrol(0, kSixBits))
                    return std::nullopt;
            }
        }
        const std::optional<Spot> spot = spotSpeederOf(app, cpu, cop);
        if (!spot)
            return std::nullopt;
        // The last by address: its eax is what the step leaves.
        if (now == kStepParked && (!stopCar(app, cpu, cop) || !stopCar(app, cpu, cop) || !call(kStop)))
            return std::nullopt;
        if (chasing())
        {
            const reg8 side = byte(app, cop + kCarSide);
            step(kStepChase);
            putByte(app, cop + kCarSide, side & ~kSideRoadblock);
        }
        return now == kStepParked ? Drive{DriveEnd::Callee} : spotted(*spot);
    }
    case kStepLanes:
    {
        if (!steerAtTarget(app, cpu, cop) || !setWantedSpeedOf(app, cpu, cop, 0.0f, floatOf(kSixAndHalfBits)))
            return std::nullopt;
        voteRoadLaneOf(app, cop);
        guest::direct<0x40d720, aiplan::scoreObstacles>(app, cpu, cop);
        if (cpu.terminate || !guest::direct<0x40d800, aiplan::scoreCars>(app, cpu, cop))
            return std::nullopt;
        guest::direct<0x40d9f0, aiplan::penaliseCloseCars>(app, cpu);
        if (cpu.terminate || !call(kSteer))
            return std::nullopt;
        reg32 result = 0;
        if (laneOffRoadOf(app, cop))
        {
            const reg8 copKind = byte(app, cop + kCarCopKind);
            step(kStepOffRoad);
            result = 1 | reg32(copKind) << 8;
            if (copKind & kCopRoadblock)
            {
                const reg32 siren = at(app, cop + kCarCopIndex) * kSirenSize;
                result = at(app, siren + kSirens) != 0 ? siren : at(app, cop + kCarCopFlags) & 0xf;
                if (!roadblockSirenOf(app, cpu, cop))
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
        if (!stopCar(app, cpu, cop))
            return std::nullopt;
        const std::optional<Spot> spot = spotSpeederOf(app, cpu, cop);
        if (!spot)
            return std::nullopt;
        if (chasing())
            step(kStepChase);
        return spotted(*spot);
    }
    case kStepChase:
    {
        if (!steerAtTarget(app, cpu, cop) || !sizeUpTargetOf(app, cpu, cop) || !keepSideOfTargetOf(app, cpu, cop)
            || !pursueTargetOf(app, cpu, cop) || !noteSpeedingOf(app, cpu, at(app, cop + kCarChased))
            || !chooseLanes(app, cpu, cop))
            return std::nullopt;
        votePursuitLaneOf(app, cpu, cop);
        if (!call(kSteer))
            return std::nullopt;
        const std::optional<bool> stopped = targetStoppingOf(app, cpu, cop);
        if (!stopped)
            return std::nullopt;
        if (*stopped)
        {
            step(kStepStop);
            Drive d{DriveEnd::StopsBy};
            d.choice = copStopsByChoice(app, cop);
            d.pursuit = at(app, cop + kCarPursuit);
            d.player = byte(app, cop + kCarFlags) & kFlagPlayer;
            const std::optional<StopBy> way = asCalled(cpu, [&] { return copStopsByOf(app, cpu, cop); });
            if (!way)
                return std::nullopt;
            d.stopsBy = *way;
            return d;
        }
        const reg32 target = at(app, cop + kCarChased);
        if (target == 0)
        {
            step(kStepPatrol);
            return value(0);
        }
        if (isReversing(app, target))
        {
            if (!radioStoppedOf(app, cpu, cop))
                return std::nullopt;
            return value(at(app, at(app, cop + kCarChased) + kCarPursuit));
        }
        if (gameSettings(app).copCount != 0)
            return value(0);
        const reg32 pursuit = at(app, cop + kCarPursuit);
        if (i32(at(app, kClock) - at(app, pursuit + kPursuitRadioDue)) <= i32(kCallTicks))
            return value(pursuit);
        const reg32 clock = at(app, kClock);
        const reg32 caller = at(app, pursuit + kPursuitCaller);
        put(app, pursuit + kPursuitRadioDue, clock);
        if (cop == caller)
            put(app, pursuit + kPursuitCaller, 0);
        else
        {
            const std::optional<bool> roadblock = callRoadblockOf(app, cpu, cop);
            if (!roadblock)
                return std::nullopt;
            if (*roadblock)
                return value(1);
            const std::optional<bool> twoCars = callTwoCarsOf(app, cpu, cop);
            if (!twoCars)
                return std::nullopt;
            if (*twoCars)
                return value(1);
        }
        const std::optional<BackupCall> backup = callBackupOf(app, cpu, cop);
        if (!backup)
            return std::nullopt;
        Drive d{DriveEnd::Backup};
        d.backup = *backup;
        return d;
    }
    case kStepStop:
    {
        put(app, cop + kCarWanted, 0);
        if (!stopCar(app, cpu, cop))
            return std::nullopt;
        const std::optional<bool> over = stopOverOf(app, cpu, cop);
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
void copDriver(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax;
    const std::optional<Drive> drive = copDriverOf(app, cpu, cop);
    if (!drive)
        return;
    switch (drive->end)
    {
    case DriveEnd::Value:
        cpu.eax = drive->value;
        break;
    case DriveEnd::Status:
        cpu.eax = withStatus(cpu, drive->value);
        break;
    case DriveEnd::Callee:
        break;
    case DriveEnd::Spotted:
        cpu.eax = spotSpeederEax(app, cpu, cop, drive->spot);
        break;
    case DriveEnd::StopsBy:
        cpu.eax = copStopsByEax(app, cop, drive->stopsBy, drive->choice, drive->pursuit, drive->player);
        break;
    case DriveEnd::Backup:
        cpu.eax = callBackupEax(app, cop, drive->backup);
        break;
    }
    cpu.esp += 4;
}

/* copDriverOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool copDriverDone(App* app, CPU& cpu, reg32 cop)
{
    return copDriverOf(app, cpu, cop).has_value();
}

/* What copHitByCar's code leaves in eax on each way out of copHitByCarOf: the cop (with the
 * compare's status word where the racer was slower), the clock, the cop's pursuit, 0, the join's
 * kind where the cop was on a pursuit, else as the callee last called (the radio) leaves it. */
reg32 copHitEax(App* app, const CPU& cpu, reg32 cop, const HitOutcome& hit)
{
    switch (hit.way)
    {
    case CopHit::Nothing:
        return cop;
    case CopHit::Slower:
        return withStatus(cpu, cop);
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
    return cpu.eax;
}

/* 0x402cb0: copHitByCarOf cop eax, racer edx; eax and edx as the code leaves them on each way
 * out. */
void copHitByCar(App* app, CPU& cpu)
{
    const reg32 cop = cpu.eax, racer = cpu.edx;
    const std::optional<HitOutcome> hit = copHitByCarOf(app, cpu, cop, racer);
    if (!hit)
        return;
    cpu.eax = copHitEax(app, cpu, cop, *hit);
    if (hit->way == CopHit::NotWanted)
        cpu.edx = copsWantedEdx(app, racer);
    else if (hit->way == CopHit::Switched || hit->way == CopHit::Joined)
        cpu.edx = cop;
    cpu.esp += 4;
}

// What carContactOf did, for what its code leaves in eax.
struct Contact
{
    std::optional<HitOutcome> hit;  // copHitByCarOf's, where it was called
    bool sized = false;             // the ram's first test passed: al the other car's flags
    std::optional<reg32> wait;      // the ram's wait asked (0x40f050)
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
std::optional<Contact> carContactOf(App* app, CPU& cpu, reg32 car)
{
    Contact contact;
    const reg32 other = at(app, car + kCarHit);
    if ((byte(app, other + kCarFlags) & kFlagRacer) && (byte(app, car + kCarFlags) & kFlagFinished)
        && i32(at(app, car + kCarHitForce)) > i32(kHitBits))
    {
        contact.hit = copHitByCarOf(app, cpu, car, other);
        if (!contact.hit)
            return std::nullopt;
    }
    bool ram = (byte(app, kRaceSwitches) & kRaceNoRams) == 0 && !(i32(at(app, car + kCarRamFor)) > 0);
    if (ram)
    {
        contact.sized = true;
        const reg8 flags = byte(app, other + kCarFlags);
        ram = (flags & kFlagPlayer) && (byte(app, car + kCarFlags) & kFlagRams) && (flags & kFlagFinished) == 0
              && gameSettings(app).seriesRace == 1;
    }
    if (ram)
    {
        contact.wait = ramWait(app, cpu, car, kRamAsking);
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
            const reg32 rules = at(app, car + kCarPlaceInRace) * kRammingSize + kRamming;
            if (i32(at(app, car + kCarRammed)) > i32(at(app, rules + 8)))
            {
                const reg32 wait = at(app, rules + 0xc);
                put(app, car + kCarRammed, 0);
                put(app, car + kCarRamFor, wait);
                contact.over = true;
            }
            if (byte(app, other + kCarFlags) & kFlagRams)
            {
                const reg32 race = gameSettings(app).raceType;
                if (race == 1 || race == 2)
                    put(app, car + kCarRamFor, 0);
            }
        }
    }
    if (byte(app, car + kCarFlags) & kFlagTimesHits)
    {
        const reg8 flags = byte(app, other + kCarFlags);
        reg32 force = reg32(fp::toInteger<sreg32>(f32(app, car + kCarHitForce)));
        reg32 by;
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
void carContact(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    const reg32 other = at(app, car + kCarHit);
    const std::optional<Contact> contact = carContactOf(app, cpu, car);
    if (!contact)
        return;
    if (contact->timed)
        cpu.eax = at(app, car + kCarHitUntil);
    else if (contact->wait)
    {
        const reg32 rules = at(app, car + kCarPlaceInRace) * kRammingSize;
        if (*contact->wait == 0)
            cpu.eax = contact->over ? at(app, rules + kRamming + 0xc) : rules;
    }
    else
    {
        reg32 eax = contact->hit ? copHitEax(app, cpu, car, *contact->hit) : car;
        if (contact->sized)
            eax = (eax & 0xffffff00) | byte(app, other + kCarFlags);
        cpu.eax = eax;
    }
    cpu.esp += 4;
}

/* carContactOf for a caller that asks only whether the game went on (native_ai_race.h). */
bool carContactDone(App* app, CPU& cpu, reg32 car)
{
    return carContactOf(app, cpu, car).has_value();
}

}  // namespace airace
}  // namespace nfs3hp
