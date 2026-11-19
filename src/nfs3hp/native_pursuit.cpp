#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_collide.h"
#include "native_ea_support.h"
#include "native_frontend_loada.h"
#include "native_frontend_neta.h"
#include "native_frontend_sima.h"
#include "native_frontend_tail.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_pursuit.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_watcom.h"
#include "native_trackdata.h"
#include "port_fields.h"
#include "port_menus.h"
#include "port_snd.h"
#include "port_snd_calls.h"
#include "port_snd_speech.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* Hot Pursuit's police, native whole: the cops' and the speeders' speeds in
 * a pursuit and the cop's state from frame to frame, the radio's phrases and
 * the speeches of the dispatcher and the cops ("a speeder, red Lamborghini,
 * 150+, heading for the town", "ticket", "arrest", "roadblock", "patrol",
 * "donuts", ...), and the racers' speech banks.
 *
 * A car's record (0x9a4 bytes and more) as these read it: +8 its progress
 * along the track (the leader's against it, the cop's against what it had),
 * +0x1c its node on the track, +0x98 its place, +0xa4 its speed's vector,
 * +0x10c, +0x114 its width, +0x164 the race's stage, +0x184 a slice
 * it is to be put back on, +0x1f0 its index, +0x200 its flags (1 a human's,
 * 4 a cop's, 0x20 the player's), +0x204/+0x208 its laps, +0x220 its choice
 * (+0x14 the car's model, +0x30 its pursuit class -- an index of the class
 * records at 0x552274, 0x18 bytes: +4 a flag, +8 how keen its cops are),
 * +0x2bc, +0x2c4 the player's kind of control, +0x509 the brake, +0x528 its
 * handling (+0x1cc its top speed), +0x5f4 its going backwards, +0x620 and
 * +0x621 the pursuit's flags (0x20 of +0x620: it is chased; of +0x621: 0x10
 * against the traffic, 0x40 the cop may reset it, 0x80 it is on the radio),
 * +0x624 the car a cop chases, +0x628 its pursuit record (+0x1c, +0x2c),
 * +0x638 its lane, +0x650 a countdown, +0x654, +0x660, +0x6d8 the car it
 * follows, +0x6e0 the cop's state (0 to 7, a switch's), +0x6e4 its chase's
 * count, +0x6ec, +0x700 its pace, +0x704/+0x708 the frame and the place of
 * its last progress, +0x710, +0x714 its radio's voice, +0x7b0 its target
 * speed (signed as it goes: the sign bit of +0x7b3 flipped where the track
 * runs backwards [0x6fd4c0] or the car goes against it), +0x7b4, +0x7b8,
 * +0x7bc its speed's factor, +0x7c0 its speed, +0x7c4 the speed's size,
 * +0x7d4.  The track's nodes are 0x24 bytes at [0x5dd948], [0x5dd940] of
 * them: +0x13 the lanes on either side (a nibble each), +0x17 and +0x1b the
 * road's widths either side, +0x11..+0x13 its heading's bytes.
 *
 * The pursuit's globals: 0x56eef0..0x56ef6e the chases (three: the cops'
 * cars 0x56ef38, their stages 0x56ef44, their counts 0x56ef5c, flags
 * 0x56ef68/0x56ef6e, the scores 0x56eef0, 0x56eefc, 0x56ef08, 0x56ef20,
 * 0x56ef14..0x56ef1c), 0x56eeb4..0x56eebc the lanes' times, 0x56ef3c the car
 * the cops chase, 0x56ef48 the chase's level, 0x56ebb4/0x56ebb8 the race's
 * leader and its kind.  The radio's: 0x56ec90 its phrases' names (8 bytes
 * each), 0x56ee28 each car's speech bank, 0x56ee68 and 0x5524cc each voice's
 * frames since it spoke, 0x5524c8 the frames since anyone did, 0x5524ec the
 * voice that spoke last, 0x5524f0 a car's index, 0x5524f4 a "help" pending,
 * 0x5524f8 and 0x552520 each voice's last place and its prefix, 0x552548 a
 * table of floats by a random number (0x554f10..0x554f18 its generator's
 * state), 0x7dd378 the cops' radio's distance.
 *
 * Each is a typed function (its arguments named, its locals C++'s, the guest's
 * memory where it is) under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the calls through a pointer: it takes
 * the registers, calls the typed function under the caller's registers
 * (asThunk: the code keeps them, its argument registers no caller reads) and
 * leaves eax, the x87's stack and the guest's stack as the game's code does.
 * The x87's arithmetic is fp's in the code's order of operands, a
 * float kept where the code stores one; a compare's status word in ax as
 * fp::compare has it.  Where a callee is handed a frame's address (a record
 * for the radar, a place, a line of speech, a path) the frame is on the
 * guest's stack at the code's depth (GuestFrame, CallSlot).  The switches go
 * by the targets nfs3.exe's tables hold (0x409be0, 0x40a3f0, 0x40a55c,
 * 0x40aab0).  The other typed files' functions are called as C++ through
 * guest::direct, but where what a thunk here leaves is what the callee
 * leaves in eax beyond what its header gives, or where the code hands it words
 * on the guest's stack; every other file's function is called by address, its
 * arguments in registers and on the guest's stack as the code hands them. */
namespace nfs3hp
{
namespace pursuit
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
using s16 = nfs::s16;

// The race's: its type (3 Hot Pursuit), its mode (1 a split screen), the
// track backwards, the frame counter, the pursuit's level and switches.
constexpr u32 kRaceFrame = 0x7d3684;
constexpr u32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The class records of the pursuit (0x18 bytes), by the car's +0x30.
constexpr u32 kClassKind = 0x552274, kClassFlag = 0x552278, kClassKeen = 0x55227c;
// What is called by address: the game's, the lane votes (0x40de30, its thunk's NFS_CAR_TRACE
// record), strncpy; and of the typed files' those whose eax a thunk
// here leaves, which their headers give only as whether (a cop near, a node within reach, the cop
// held, the car driven on), or that the code hands words on the guest's stack (the debug text, the
// tunnel's test, which takes a copy of the place).  The menu files' (the radar, the places named,
// the car put on its node and settled, the car ticketed: feCore), memset, strstr and the car's
// chosen colour are called as C++, the eax they leave given back.
constexpr u32 kDebugText = 0x42d520;
constexpr u32 kStrncpy = 0x4e0e30;
constexpr u32 kChop = 0x4dfd56;

u16 half(App* app, u32 a) { return app->getMemory<u16>(a); }
s32 s(u32 v) { return s32(v); }


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

/* 0x439790 (port::fields'): model's chosen colour for player; nullopt when the game stopped.  Its
 * esp the call's (no words pushed). */
std::optional<u32> chosenColour(App* app, ThreadState& ts, u32 model, u32 player)
{
    const port::Result colour =
        guest::direct<0x439790, port::fields::chosenColour>(app, ts, ts.sp - 4, model, std::int32_t(player));
    if (!colour || ts.terminate)
        return std::nullopt;
    return u32(*colour);
}

// The game's constants as the x87 loads them.
Float f32(App* app, u32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, u32 a) { return Float(app->getMemory<double>(a)); }

/* The node `node`'s record: 0x24 bytes each from [0x5dd948]. */
u32 nodeRecord(App* app, u32 node)
{
    return (node * 8 + node) * 4 + at(app, kNodes);
}

// ---- For the typed functions below: no registers, C++ locals, the guest's memory where it is. ----

using i32 = std::int32_t;

// A car's fields (see the comment at the top).
constexpr u32 kCarProgress = 8, kCarNode = 0x1c, kCarSpeedSize = 0x10c, kCarWidth = 0x114, kCarRaceStage = 0x164,
                kCarIndex = 0x1f0, kCarFlags = 0x200, kCarLapsDone = 0x204, kCarLapsStart = 0x208,
                kCarChoice = 0x220, kCarControl = 0x2c4, kCarBrake = 0x509, kCarHandling = 0x528,
                kCarBackwards = 0x5f4, kCarPursuitFlags = 0x620, kCarPursuitFlagsHigh = 0x621, kCarChased = 0x624,
                kCarRecord = 0x628, kCarLane = 0x638, kCarCountdown = 0x650, kCarJitter = 0x654, kCarNight = 0x660,
                kCarFollowed = 0x6d8, kCarCopState = 0x6e0, kCarChaseCount = 0x6e4, kCarStateUntil = 0x6ec,
                kCarPace = 0x700, kCarProgressFrame = 0x704, kCarProgressPlace = 0x708, kCarReach = 0x710,
                kCarVoice = 0x714, kCarTarget = 0x7b0, kCarTargetSign = 0x7b3, kCarAcross = 0x7b4,
                kCarSide = 0x7b8, kCarFactor = 0x7bc, kCarSpeed = 0x7c0, kCarSpeedAbs = 0x7c4, kCarOut = 0x7d4;
// The choice's (+0x220) model and pursuit class, the handling's top speed, the pursuit record's.
constexpr u32 kChoiceModel = 0x14, kChoiceClass = 0x30, kHandlingTopSpeed = 0x1cc, kRecordReset = 0x1c,
                kRecordHeld = 0x2c;
// Of +0x200: a human's car, a cop's, the player's.  Of +0x620: chased.  Of +0x621: against the
// traffic, may be reset, on the radio.
constexpr u8 kFlagHuman = 1, kFlagCop = 4, kFlagPlayer = 0x20, kFlagChased = 0x20, kFlagAgainst = 0x10,
               kFlagMayReset = 0x40, kFlagOnRadio = 0x80;
// A track node's (0x24 bytes): its lanes either side (a nibble each), the road's widths either side.
constexpr u32 kNodeSize = 0x24, kNodeLanes = 0x13, kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b;
// A class record's size.
constexpr u32 kClassSize = 0x18;
// 11 as a float's bits: the target speed of a car pulled over.
constexpr u32 kPulledOverBits = 0x41300000, kOneBits = 0x3f800000;
// The chases: their cops, stages, cars, counts, flags; the scores.
constexpr u32 kChaseCops = 0x56ef38, kChaseStages = 0x56ef44, kChaseCars = 0x56ef50, kChaseCounts = 0x56ef5c,
                kChaseOn = 0x56ef68, kChaseLive = 0x56ef6e, kScoreSpeed = 0x56eef0, kScoreFree = 0x56eefc,
                kScoreStage = 0x56ef08, kScoreLeft = 0x56ef14, kScoreOwn = 0x56ef18, kScoreRight = 0x56ef1c,
                kScoreClose = 0x56ef20;
constexpr i32 kChases = 3;
// The lanes' times: the left's, the outer lanes', the right's.
constexpr u32 kLaneTimeLeft = 0x56eeb4, kLaneTimeOuter = 0x56eeb8, kLaneTimeRight = 0x56eebc;
// The cars a cop chases (0x56ef3c), the chase's level, its other level.
constexpr u32 kChasedCar = 0x56ef3c, kChaseLevel = 0x56ef48, kChaseLevelToo = 0x56ec80;
// The cops' list and count.
constexpr u32 kCops = 0x5efa88, kCopCount = 0x5efd88;
// The radio's: its load, the car of [0x5524f0], each voice's frames since it spoke.
constexpr u32 kRadioLoad = 0x7dd378, kRadioCar = 0x5524f0, kVoiceQuiet = 0x56ee68, kVoiceSpoke = 0x5524cc,
                kRadioQuiet = 0x5524c8;
constexpr i32 kVoices = 8;

float f32At(App* app, u32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, u32 a, float f) { app->getMemory<float>(a) = f; }

u32 floatBits(float f)
{
    u32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}

float floatOf(u32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

/* A double's high dword, as the code copies one through eax. */
u32 highHalf(double d)
{
    std::uint64_t bits;
    std::memcpy(&bits, &d, sizeof bits);
    return u32(bits >> 32);
}

/* x87 compares as the jumps after fnstsw ax; sahf read them: below (carry) and below-or-equal also
 * for a NaN, above and above-or-equal not -- each as the C++ compare that agrees on every value. */
bool isBelow(double a, double b) { return !(a >= b); }
bool isBelowEqual(double a, double b) { return !(a > b); }
bool isAbove(double a, double b) { return a > b; }
bool isAboveEqual(double a, double b) { return a >= b; }

/* What fnstsw leaves in ax, the rest of eax kept. */
u32 withStatus(u32 eax, double a, double b) { return (eax & 0xffff0000u) | fp::compare(a, b); }

/* The sign of the car's target speed flipped, a byte as the code does it. */
void flipTarget(App* app, u32 car) { putByte(app, car + kCarTargetSign, byte(app, car + kCarTargetSign) ^ 0x80); }

/* The car's pursuit class's record's offset (its index times 0x18). */
u32 classOf(App* app, u32 car) { return at(app, at(app, car + kCarChoice) + kChoiceClass) * kClassSize; }

/* The keenness of the car's class's cops. */
i32 keenness(App* app, u32 car) { return i32(at(app, classOf(app, car) + kClassKeen)); }

/* The two outer lanes of a node: 6 less its lanes on the left, its lanes on the right and 7. */
i32 outerLeft(App* app, u32 n) { return 6 - ((byte(app, n + kNodeLanes) >> 4) & 0xf); }
i32 outerRight(App* app, u32 n) { return (byte(app, n + kNodeLanes) & 0xf) + 7; }

/* esp as deep as the game's code has it in its frame (its pushes, its locals' bytes), for the calls it
 * makes: a callee handed a frame's address, or one of this file's that hands one on, sees the same
 * addresses.  The guest's stack is not written but where a callee reads it. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

/* A call of one of this file's typed functions where the game's code calls: the return address's slot,
 * and the slots of the `words` it pushes before it. */
class CallSlot
{
public:
    explicit CallSlot(ThreadState& ts, u32 words = 0) : ts(ts), bytes(4 + 4 * words) { ts.sp -= bytes; }
    ~CallSlot() { ts.sp += bytes; }

private:
    ThreadState& ts;
    const u32 bytes;
};

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

/* The edi the running thunk's caller had: a voice past the 8 takes it for its handle (sayCallSignOf,
 * saySpeedOf), as the game's code keeps it across the radio's calls. */
u32 g_callerEdi = 0;

struct CallerEdi
{
    explicit CallerEdi(u32 edi) : saved(g_callerEdi) { g_callerEdi = edi; }
    ~CallerEdi() { g_callerEdi = saved; }
    const u32 saved;
};

template <typename Core>
bool asThunk(ThreadState& ts, nfs::Args& args, Core&& core, u32 popped = 0)
{
    const CallerEdi callerEdi(args.edi);
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

/* `pushed` on the guest's stack (pushed in that order), esp back as it was after: for a callee that finds
 * its words there (placeInTunnel).  The result of `call` (the callee, called as C++ with esp at the words). */
template <typename Call>
auto callPushing(ThreadState& ts, std::initializer_list<u32> pushed, App* app, Call call)
{
    const u32 esp = ts.sp;
    struct Restore
    {
        ThreadState& ts;
        u32 esp;
        ~Restore() { ts.sp = esp; }
    } restore{ts, esp};
    for (const u32 word : pushed)
    {
        ts.sp -= 4;
        put(app, ts.sp, word);
    }
    return call();
}

/* 0x40f050 (aiplan::carInReach) of the car and the distance 0: the eax it leaves, 1 where a cop is near; nullopt
 * when the game stopped. */
std::optional<u32> copNear(App* app, ThreadState& ts, u32 car)
{
    const bool near = guest::direct<0x40f050, aiplan::carInReach>(app, ts, car, 0);
    if (ts.terminate)
        return std::nullopt;
    return near ? 1u : 0u;
}

/* 0x40f000 (aiplan::nodeInRun): the eax it leaves, 1 where `node` lies in the run of `run` nodes from `from`;
 * nullopt when the game stopped. */
std::optional<u32> inReach(App* app, ThreadState& ts, u32 node, u32 from, u32 run)
{
    const bool within = guest::direct<0x40f000, aiplan::nodeInRun>(app, ts, node, from, run);
    if (ts.terminate)
        return std::nullopt;
    return within ? 1u : 0u;
}

/* 0x439780 (port::fields::customColourOf): car `car`'s custom colour's number; nullopt when the game stopped. */
std::optional<u32> customColourNumber(App* app, ThreadState& ts, u32 car)
{
    const port::Result number = guest::direct<0x439780, port::fields::customColourOf>(app, ts, ts.sp - 4, car);
    if (ts.terminate || !number)
        return std::nullopt;
    return u32(*number);
}

/* The car's pursuit class times 3, as the code leaves it in eax (it scales it by 8 for the record). */
u32 classTimesThree(App* app, u32 car) { return at(app, at(app, car + kCarChoice) + kChoiceClass) * 3; }

}  // namespace

nfs::u32 raceStartHook(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceEndHook(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 roadsideTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 countLaneTime(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pulledOverTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 classSpeedFactor(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 unitFactor(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radarFactor(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 unitFactorToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 leaderFactor(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nodeTargetSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 markChased(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 targetSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 targetSpeedOnly(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 matchChasedSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 scoreCloseCops(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 growLaneTimes(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 scoreLane(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 scoreChases(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioChaseCount(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 scoreSpeeds(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 takeOverCars(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 startReset(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 checkProgress(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copStep(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 loadRacerBanks(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 countRadioFrames(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioNoise(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayPhrase(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayCarPhrase(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 namePhrase(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 namePhrases(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioFree(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 quietVoice(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayVoicePhrase(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayCallSign(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayPhraseOnly(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayUnit(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayDescription(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioNothingToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sayLocation(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 saySpeed(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioSpotted(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioLost(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioBackup(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioPatrol(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioNothingThree(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioTicket(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioArrest(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioStopped(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioAmbulance(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioSpikes(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioHelp(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioResume(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 radioAbort(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 resetOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copRestart(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 topSpeedFactor(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 followChased(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceEndHookToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceStartHookToo(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 followCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 countdownDone(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 trafficTarget(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 trafficStep(App* app, nfs::ThreadState& ts, nfs::Args& args);

// The typed functions called before they are written.
u32 followChasedCar(App* app, u32 car);

std::optional<u32> resetOnTrackOf(App* app, ThreadState& ts, u32 car, u32 record);

/* 0x408ad0, 0x408ae0, 0x40c140, 0x40c150: the pursuit's hooks at the race's
 * start and end (the race's set-up 0x4c45f0 and its end 0x4c4740 call them),
 * empty -- push ebp, mov ebp, esp, pop ebp: nothing but the return. */
nfs::u32 raceStartHook(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

nfs::u32 raceEndHook(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return raceStartHook(app, ts, args);
}

nfs::u32 raceEndHookToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return raceStartHook(app, ts, args);
}

nfs::u32 raceStartHookToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return raceStartHook(app, ts, args);
}

/* The target speed of the car pulled over at the roadside: 0 where it is in one of its node's outer
 * lanes (6 less the lanes on the left, or the lanes on the right and 7), else 11; its sign flipped
 * where the track runs backwards, and again where the car goes backwards. */
void stopAtRoadside(App* app, u32 car)
{
    const u32 n = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    const bool outer = outerLeft(app, n) == lane || outerRight(app, n) == lane;
    put(app, car + kCarTarget, outer ? 0 : kPulledOverBits);
    if (gameSettings(app).backwards != 0)
        flipTarget(app, car);
    if (at(app, car + kCarBackwards) != 0)
        flipTarget(app, car);
}

/* 0x408b30: stopAtRoadside for the car eax; every register kept. */
nfs::u32 roadsideTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { stopAtRoadside(app, car); });
    return args.eax;
}

/* 150 frames more to the lanes' time of where the car is: the outer lanes' where it is in one of its
 * node's outer lanes, else the left's (lanes under 7) or the right's. */
void addLaneTime(App* app, u32 car)
{
    const u32 n = nodeRecord(app, at(app, car + kCarNode));
    const i32 lane = i32(at(app, car + kCarLane));
    const bool outer = outerLeft(app, n) == lane || outerRight(app, n) == lane;
    const u32 counter = outer ? kLaneTimeOuter : lane < 7 ? kLaneTimeLeft : kLaneTimeRight;
    put(app, counter, at(app, counter) + 0x96);
}

/* 0x408bd0: addLaneTime for the car eax; eax left as the code leaves it, the node's record where the
 * car is in the left outer lane, else the right outer lane (what it was compared with). */
nfs::u32 countLaneTime(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        addLaneTime(app, car);
        const u32 n = nodeRecord(app, at(app, car + kCarNode));
        return outerLeft(app, n) == i32(at(app, car + kCarLane)) ? n : u32(outerRight(app, n));
    });
    return args.eax;
}

// A car's side across the road to the road's units (a lane's width 2^15): a double.
constexpr u32 kSideToRoad = 0x5363dc;

/* The target speed of the car the cops stopped: 11 where it is within the track's first 20 nodes (the
 * way the track runs) with both its sides (+0x7b8 less and more its width) within the road either side
 * at its node (its lanes there times their widths), else 0; its sign flipped where the track runs
 * backwards.  Gives the last of the sides' places worked out (each worked out twice, as the code does
 * from the double it keeps): what the code leaves in eax. */
i32 stopForCops(App* app, u32 car)
{
    const bool backwards = gameSettings(app).backwards != 0;
    const i32 index = i32(at(app, car + kCarNode));
    const bool farFromStart = backwards ? index < i32(at(app, kNodeCount) - 0x14) : index > 0x14;
    const Float side = f32(app, car + kCarSide);
    const float low = float(fp::sub(side, f32(app, car + kCarWidth)));
    const float high = float(fp::add(side, f32(app, car + kCarWidth)));
    const u32 n = nodeRecord(app, u32(index));
    const auto leftEdge = [&] {
        return i32(0u - ((byte(app, n + kNodeLanes) >> 4) & 0xf) * (u32(byte(app, n + kNodeLeftWidth)) << 15));
    };
    const auto rightEdge = [&] {
        return i32((u32(byte(app, n + kNodeRightWidth)) << 15) * (byte(app, n + kNodeLanes) & 0xf));
    };
    const Float lowPlace = fp::mul(Float(low), f64(app, kSideToRoad));
    i32 place = fp::toInteger<i32>(lowPlace);
    bool stop = place < leftEdge();
    if (!stop)
    {
        place = fp::toInteger<i32>(lowPlace);
        stop = place > rightEdge();
    }
    if (!stop)
    {
        const Float highPlace = fp::mul(Float(high), f64(app, kSideToRoad));
        place = fp::toInteger<i32>(highPlace);
        stop = place < leftEdge();
        if (!stop)
        {
            place = fp::toInteger<i32>(highPlace);
            stop = place > rightEdge() || farFromStart;
        }
    }
    put(app, car + kCarTarget, stop ? 0 : kPulledOverBits);
    if (backwards)
        flipTarget(app, car);
    return place;
}

/* 0x408c50: stopForCops for the car eax; eax the last place, its ah the flipped byte where the track
 * runs backwards, as the code leaves it. */
nfs::u32 pulledOverTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] {
        const u32 place = u32(stopForCops(app, car));
        if (gameSettings(app).backwards == 0)
            return place;
        return (place & 0xffff00ffu) | u32(byte(app, car + kCarTargetSign)) << 8;
    });
    return args.eax;
}

// The class factor's: by the chase's kind 0 and 1 (floats), every second and fourth quarter of 512
// frames, the keenness's step and its start, a class of keenness 1's (doubles).
constexpr u32 kKindZeroFactor = 0x5363e4, kKindOneFactor = 0x5363e8, kSecondQuarter = 0x5363ec,
                kFourthQuarter = 0x5363f4, kKeenStep = 0x5363fc, kKeenOne = 0x536404;

/* Whether the class's factor is 1 at once: a single player's race of type 2. */
bool knockoutAlone(App* app)
{
    return gameSettings(app).raceType == 2 && s(gameSettings(app).playMode) < 1;
}

/* The factor of the car's speed by its pursuit's class: 1 in a single player's knockout; else 1 times
 * [0x5363e4] or [0x5363e8] by its chase's kind (0x404610: 0 or 1), then times [0x5363ec] or [0x5363f4]
 * every second and fourth quarter of 512 frames where its class has no flag, then by its class's
 * keenness -- a chased car's 1 + keen * [0x5363fc] ([0x5363f4] the 1), else [0x5363f4] for 0 and
 * [0x536404] for 1.  nullopt when the game stopped. */
std::optional<float> classFactor(App* app, ThreadState& ts, u32 car)
{
    float factor = 1.0f;
    if (knockoutAlone(app))
        return factor;
    const GuestFrame frame(ts, 0x10, 0x10);
    const u32 kind = guest::direct<0x404610, airace::classRuleOf>(app, ts, car);
    if (ts.terminate)
        return std::nullopt;
    if (kind == 0)
        factor = float(fp::mul(Float(factor), f32(app, kKindZeroFactor)));
    else if (kind == 1)
        factor = float(fp::mul(Float(factor), f32(app, kKindOneFactor)));
    const i32 quarter = (s(at(app, kRaceFrame)) >> 7) & 3;
    if (at(app, classOf(app, car) + kClassFlag) == 0)
    {
        if (quarter == 1)
            factor = float(fp::mul(Float(factor), f64(app, kSecondQuarter)));
        if (quarter == 3)
            factor = float(fp::mul(Float(factor), f64(app, kFourthQuarter)));
    }
    if ((byte(app, car + kCarPursuitFlags) & kFlagChased) != 0)
    {
        const Float keen = Float(s(at(app, classOf(app, car) + kClassKeen)));
        factor = float(fp::mul(fp::add(fp::mul(keen, f64(app, kKeenStep)), f64(app, kFourthQuarter)), Float(factor)));
    }
    else
    {
        const u32 keen = at(app, classOf(app, car) + kClassKeen);
        if (keen == 0 || keen == 1)
            factor = float(fp::mul(Float(factor), f64(app, keen == 0 ? kFourthQuarter : kKeenOne)));
    }
    return factor;
}

/* 0x408dc0: classFactor of the car eax onto the x87's stack; eax the car where the factor is 1 at once,
 * else the factor's bits, as the code leaves it. */
nfs::u32 classSpeedFactor(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<float> factor;
    const auto left = [&, car = args.eax]() -> std::optional<u32> {
        factor = classFactor(app, ts, car);
        if (!factor)
            return std::nullopt;
        return knockoutAlone(app) ? car : floatBits(*factor);
    };
    if (asThunk(ts, args, left))
    {
        args.floatSet = true;
        args.floatResult = double(Float(*factor));
    }
    return args.eax;
}

/* 0x408f00 and 0x409090: a factor of 1 onto the x87's stack (fld1) -- the targetSpeed's factors the
 * game leaves at 1. */
nfs::u32 unitFactor(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(Float(1.0));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 unitFactorToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return unitFactor(app, ts, args);
}

/* What a typed function gives with what the game's code leaves in eax beside it: no caller of the game's
 * reads that. */
struct FactorLeft
{
    float factor;
    u32 eax;
};

// The radar's record (0x4c8040 writes it, 0x20 bytes in the frame): the car ahead's speed (a float), how
// far ahead (a signed short), its kind.
constexpr u32 kRadarSpeed = 8, kRadarAhead = 0xc, kRadarKind = 0x10;
// The radar's factor's step by the distance (a float), and the speed under which a car close behind
// brakes, as a float's bits (3).
constexpr u32 kRadarStep = 0x53640c, kSlowBits = 0x40400000;

/* The factor of the car's speed by the radar's word on it (0x4c8040 into the frame's record: how far
 * the car ahead is, its kind), and what the code leaves in eax -- 1 but where the race is a network one
 * and the car is the network's to drive: a car well behind the one ahead (past 20 nodes on the last lap,
 * 50 else) is put back on the track where the radar says (resetOnTrack, where its kind is 1); else, but
 * for a car the player steers with a pad (+0x2c4 3), 1 + (2 - the distance) * [0x53640c], no less than
 * 0 -- a human's car close behind and slow braking and let back with the traffic, marked against the
 * traffic past 30 ahead, let go under 2.  nullopt when the game stopped. */
std::optional<FactorLeft> radarFactorOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x20);
    const u32 record = frame - 0x20;
    if (gameSettings(app).networkGame == 0)
        return FactorLeft{1.0f, car};
    if (half(app, 0x7a22da) != 0 && s(gameSettings(app).humanCount) <= s(at(app, car + kCarIndex)))
        return FactorLeft{1.0f, car};
    const std::optional<u32> word =
        feCore<0x4c8040, frontend::neta::playerControls>(app, ts, record, at(app, car + kCarIndex));
    if (!word)
        return std::nullopt;
    if (*word == 0)
        return FactorLeft{1.0f, 0};
    const u32 lastLap = gameSettings(app).laps - 1;
    const u32 behindMost = at(app, car + kCarLapsDone) - at(app, car + kCarLapsStart) == lastLap ? 0x14 : 0x32;
    const i32 ahead = s16(half(app, record + kRadarAhead));
    if (-ahead > s(behindMost))
    {
        if (at(app, record + kRadarKind) != 1)
            return FactorLeft{1.0f, behindMost};
        std::optional<u32> reset;
        {
            const CallSlot slot(ts);
            reset = resetOnTrackOf(app, ts, car, record);
        }
        if (!reset)
            return std::nullopt;
        return FactorLeft{1.0f, *reset};
    }
    if (at(app, car + kCarControl) == 3)
        return FactorLeft{1.0f, behindMost};
    if (s(gameSettings(app).humanCount) > s(at(app, car + kCarIndex)))
    {
        const i32 distance = ahead <= 0 ? -ahead : ahead;
        if (distance < 2 && s(at(app, record + kRadarSpeed)) < i32(kSlowBits))
        {
            putByte(app, car + kCarBrake, 0xff);
            put(app, car + kCarTarget, 0);
            putByte(app, car + kCarPursuitFlagsHigh, byte(app, car + kCarPursuitFlagsHigh) & ~kFlagAgainst);
        }
        else if (ahead > 0x1e)
            putByte(app, car + kCarPursuitFlagsHigh, byte(app, car + kCarPursuitFlagsHigh) | kFlagAgainst);
        else if (ahead < 2)
        {
            putByte(app, car + kCarBrake, 0);
            putByte(app, car + kCarPursuitFlagsHigh, byte(app, car + kCarPursuitFlagsHigh) & ~kFlagAgainst);
        }
    }
    const float factor = float(fp::add(fp::mul(Float(2 - ahead), f32(app, kRadarStep)), Float(1.0f)));
    const float kept = isAboveEqual(0.0, factor) ? 0.0f : factor;
    return FactorLeft{kept, isAboveEqual(0.0, factor) ? 0 : floatBits(factor)};
}

/* 0x408f10: radarFactorOf the car eax onto the x87's stack, eax as the code leaves it. */
nfs::u32 radarFactor(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<FactorLeft> factor;
    const auto left = [&, car = args.eax]() -> std::optional<u32> {
        factor = radarFactorOf(app, ts, car);
        if (!factor)
            return std::nullopt;
        return factor->eax;
    };
    if (asThunk(ts, args, left))
    {
        args.floatSet = true;
        args.floatResult = double(Float(factor->factor));
    }
    return args.eax;
}

// The race's leader and its kind.
constexpr u32 kLeader = 0x56ebb8, kLeaderKind = 0x56ebb4;

/* The factor of the car's speed by its place against the race's leader -- 1 without one; by the
 * leader's kind: 0 (the car behind by more than 5, 10 or 15 places, by the race's type and the series'
 * switches) 1.15 or 1.1 to catch up, 1 else; 1 (ahead by 100, 70 or 50) 0.85, 1 else; 2 by the
 * switches 0.95, 0.85, 0.8, 0.9.  No less than 1 in a knockout on track 8, no more than 1 in a race of
 * more than one player.  The leader written back as it was, as the code does. */
float leaderFactorOf(App* app, u32 car)
{
    const GameSettings& race = gameSettings(app);
    const u32 leader = at(app, kLeader);
    float factor = 1.0f;
    if (leader != 0)
    {
        const u32 kind = at(app, kLeaderKind);
        const bool series = race.raceType == 1 || race.raceType == 2;
        if (kind == 0)
        {
            const u32 lead = series && race.seriesVariant == 1 ? 5 : race.seriesRace == 1 ? 0xa : 0xf;
            if (s(lead + at(app, car + kCarProgress)) >= s(at(app, leader + kCarProgress)))
                factor = 1.0f;
            else
                factor = floatOf(series ? 0x3f933333 : 0x3f8ccccd);
        }
        else if (kind != 2)
        {
            const u32 lead = series && race.seriesVariant == 1 ? 0x64 : race.seriesRace == 1 ? 0x46 : 0x32;
            factor = s(lead + at(app, leader + kCarProgress)) <= s(at(app, car + kCarProgress)) ? floatOf(0x3f59999a) : 1.0f;
        }
        else if (race.seriesRace == 1)
            factor = floatOf(0x3f733333);
        else if (race.raceType == 3)
            factor = floatOf(0x3f59999a);
        else if (s(at(app, car + 0x2bc)) >= 6 && race.playMode == 0)
            factor = floatOf(0x3f4ccccd);
        else
            factor = floatOf(0x3f666666);
        // Each clamp through a double, as the code keeps it: 1 where the compare says so, else the factor.
        if (race.raceType == 2 && race.track == 8 && isAbove(1.0, factor))
            factor = 1.0f;
        if (s(race.playMode) > 1 && isBelow(1.0, factor))
            factor = 1.0f;
    }
    put(app, kLeader, leader);
    return factor;
}

/* 0x4090a0: leaderFactorOf the car eax onto the x87's stack; eax the factor's bits where there is a
 * leader, else the car, as the code leaves it. */
nfs::u32 leaderFactor(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    float factor = 0;
    asThunk(ts, args, [&, car = args.eax] {
        factor = leaderFactorOf(app, car);
        return at(app, kLeader) != 0 ? floatBits(factor) : car;
    });
    args.floatSet = true;
    args.floatResult = double(Float(factor));
    return args.eax;
}

/* The car's target speed: its node's (0x40ed40), signed as it goes (the track backwards, the car
 * against the traffic), then kept to the chased car's (followChasedCar).  What the code leaves in eax:
 * followChasedCar's; nullopt when the game stopped. */
std::optional<u32> nodeTarget(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    const float speed = guest::direct<0x40ed40, aiplan::trafficSpeed>(app, ts, at(app, car + kCarNode));
    if (ts.terminate)
        return std::nullopt;
    putFloat(app, car + kCarTarget, speed);
    if (gameSettings(app).backwards != 0)
        flipTarget(app, car);
    if ((byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0)
        flipTarget(app, car);
    return followChasedCar(app, car);
}

/* 0x409290: nodeTarget for the car eax, eax as the code leaves it. */
nfs::u32 nodeTargetSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return nodeTarget(app, ts, car); });
    return args.eax;
}

/* The car no longer marked chased (+0x620), but where one of the cops [0x5efa88..], [0x5efd88] of them,
 * chases it (+0x624). */
void releaseChase(App* app, u32 car)
{
    putByte(app, car + kCarPursuitFlags, byte(app, car + kCarPursuitFlags) & ~kFlagChased);
    for (i32 i = 0; i < s(at(app, kCopCount)); ++i)
    {
        const u32 chased = at(app, at(app, u32(i) * 4 + kCops) + kCarChased);
        if (car == chased)
        {
            putByte(app, chased + kCarPursuitFlags, byte(app, chased + kCarPursuitFlags) | kFlagChased);
            return;
        }
    }
}

/* 0x4092f0: releaseChase of the car eax. */
nfs::u32 markChased(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { releaseChase(app, car); });
    return args.eax;
}

// The computer's speed past which the pace eases it, its offset (doubles); the cops' chase speed
// against the traffic (a float).
constexpr u32 kEasedPast = 0x536414, kEasedOffset = 0x53641c, kAgainstSpeed = 0x56f224;

/* The car's target speed [+0x7b0] and its factor [+0x7bc]: the computer's speed at its node (0x40e080)
 * -- past [0x536414], with a pace +0x700 over 1, eased: (speed + [0x53641c]) * pace + [0x536414] -- or
 * [0x56f224] against the traffic; no more than its top speed times the track's pace (0x40ee80); times
 * the factors of its class, the leader, the radar (with two of 1 the game leaves) and the pace, the
 * product no more than 1 (1 where [0x6fd4fc] & 0x20); signed as it goes.  What the code leaves in eax
 * beside it; nullopt when the game stopped. */
std::optional<u32> targetSpeedOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0x3c);
    const u32 slice = at(app, car + kCarNode);
    const float pace = guest::direct<0x40ee80, aiplan::trackTuningSpeed>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    const bool against = (byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0;
    if (!against)
    {
        const Float speed = Float(guest::direct<0x40e080, aiplan::targetSpeed>(app, ts, slice));
        if (ts.terminate)
            return std::nullopt;
        putFloat(app, car + kCarTarget, float(speed));
        if (isAbove(speed, f64(app, kEasedPast)))
        {
            const Float carPace = f32(app, car + kCarPace);
            if (isBelow(1.0, carPace))
                putFloat(app, car + kCarTarget,
                         float(fp::add(fp::mul(fp::add(speed, f64(app, kEasedOffset)), carPace), f64(app, kEasedPast))));
        }
    }
    else
        put(app, car + kCarTarget, at(app, kAgainstSpeed));
    const float top = float(fp::mul(f32(app, at(app, car + kCarHandling) + kHandlingTopSpeed), Float(pace)));
    if (isAbove(f32(app, car + kCarTarget), top))
        putFloat(app, car + kCarTarget, top);
    std::optional<float> byClass;
    {
        const CallSlot slot(ts);
        byClass = classFactor(app, ts, car);
    }
    if (!byClass)
        return std::nullopt;
    const float first = 1.0f;
    const float byLeader = leaderFactorOf(app, car);
    const float second = 1.0f;
    std::optional<FactorLeft> byRadar;
    {
        const CallSlot slot(ts);
        byRadar = radarFactorOf(app, ts, car);
    }
    if (!byRadar)
        return std::nullopt;
    const Float product = fp::mul(fp::mul(Float(byRadar->factor),
                                          fp::mul(fp::mul(fp::mul(Float(*byClass), Float(first)), Float(byLeader)), Float(pace))),
                                  Float(second));
    putFloat(app, car + kCarFactor, float(product));
    const bool over = isBelow(1.0, product);
    putFloat(app, car + kCarFactor, float(over ? Float(1.0) : product));
    if ((std::uint8_t(gameSettings(app).raceFlags) & 0x20) != 0)
        put(app, car + kCarFactor, kOneBits);
    putFloat(app, car + kCarTarget, float(fp::mul(f32(app, car + kCarFactor), f32(app, car + kCarTarget))));
    if (gameSettings(app).backwards != 0)
        flipTarget(app, car);
    if ((byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0)
        flipTarget(app, car);
    return over ? withStatus(byRadar->eax, 1.0, product) : highHalf(double(product));
}

/* 0x409330: targetSpeedOf the car eax, eax as the code leaves it. */
nfs::u32 targetSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return targetSpeedOf(app, ts, car); });
    return args.eax;
}

/* targetSpeedOf, from a frame of its own (push ebp, mov ebp, esp). */
std::optional<u32> targetSpeedFramed(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 4, 0);
    const CallSlot slot(ts);
    return targetSpeedOf(app, ts, car);
}

/* 0x4094b0: targetSpeedFramed for the car eax, eax as the code leaves it. */
nfs::u32 targetSpeedOnly(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return targetSpeedFramed(app, ts, car); });
    return args.eax;
}

// The share of the gap a chase of level 2 eases back by (a double).
constexpr u32 kEaseBack = 0x536424;

/* The car's target speed kept to the chased car's [0x56ef3c] (not the player's) where both go the same
 * way and the target is past the chased car's speed: the chased car's speed at the chase's level 1 or
 * under, at 2 the target eased back by [0x536424] of the gap.  Gives what the code leaves in eax. */
u32 matchChased(App* app, u32 car)
{
    const u32 chased = at(app, kChasedCar);
    if (chased == 0 || (byte(app, chased + kCarFlags) & kFlagPlayer) != 0)
        return car;
    const bool carBack = isAbove(0.0, f32(app, car + kCarSpeed));
    const bool chasedBack = isAbove(0.0, f32(app, chased + kCarSpeed));
    if (carBack != chasedBack)
        return 1;
    const Float target = f32(app, car + kCarTarget);
    const bool forwards = isBelow(0.0, target);
    const u32 left = forwards ? floatBits(float(target)) : fp::compare(0.0, target);
    const float size = forwards ? float(target) : float(-target);
    const Float gap = fp::sub(Float(size), f32(app, chased + kCarSpeedAbs));
    if (isAboveEqual(0.0, gap))
        return withStatus(left, 0.0, gap);
    const u32 level = at(app, kChaseLevel);
    if (s(level) <= 1)
    {
        const u32 speed = at(app, chased + kCarSpeed);
        put(app, car + kCarTarget, speed);
        return speed;
    }
    if (level != 2)
        return level;
    const Float back = fp::mul(gap, f64(app, kEaseBack));
    const Float now = f32(app, car + kCarTarget);
    const bool down = isBelow(0.0, now);
    putFloat(app, car + kCarTarget, float(down ? fp::sub(now, back) : fp::add(now, back)));
    return withStatus(level, 0.0, now);
}

/* 0x4094d0: matchChased for the car eax, eax as the code leaves it. */
nfs::u32 matchChasedSpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return matchChased(app, car); });
    return args.eax;
}
/* For a car of a class keener than 2 with no cop near it (0x40f050): 15 more to each chase's score
 * [0x56ef20] whose car is a cop's, on, and 1 to 9 behind -- but where the chase's level [0x56ec80] is
 * 5 or more.  Gives what the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> scoreCloseCopsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0xc, 0);
    if (keenness(app, car) <= 2)
        return classTimesThree(app, car);
    const std::optional<u32> near = copNear(app, ts, car);
    if (!near)
        return std::nullopt;
    if (*near != 0)
        return *near;
    if (s(at(app, kChaseLevelToo)) >= 5)
        return 0;
    for (u32 i = 0; i < u32(kChases); ++i)
    {
        const u32 chaser = at(app, i * 4 + kChaseCars);
        if (chaser != 0 && half(app, i * 2 + kChaseLive) != 0 && (byte(app, chaser + kCarFlags) & kFlagCop) != 0)
        {
            const i32 behind = s(0u - at(app, i * 4 + kChaseCounts));
            if (behind < 0xa && behind > 0)
                put(app, i * 4 + kScoreClose, at(app, i * 4 + kScoreClose) + 0xf);
        }
    }
    return u32(kChases);
}

/* 0x4095d0: scoreCloseCopsOf the car eax, eax as the code leaves it. */
nfs::u32 scoreCloseCops(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return scoreCloseCopsOf(app, ts, car); });
    return args.eax;
}

// A frame's step of the lanes' times that have begun.
constexpr u32 kLaneTimeStep = 0x16;

/* The lanes' times that have begun, 22 on each. */
void growLaneTimesBy(App* app)
{
    for (const u32 time : {kLaneTimeLeft, kLaneTimeOuter, kLaneTimeRight})
        if (at(app, time) != 0)
            put(app, time, at(app, time) + kLaneTimeStep);
}

/* 0x409660: growLaneTimesBy; eax left 22, as the code leaves it. */
nfs::u32 growLaneTimes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] {
        growLaneTimesBy(app);
        return kLaneTimeStep;
    });
    return args.eax;
}

/* The score of the lane the car will take, 4 nodes on (0x40e160, 0x40e0c0), against its own [+0x638]:
 * 2 for a cop's (the lane walked into its node's lanes), else 6 for a chase of kind 0 (0x404610), 12 or
 * 10 by its class -- to the left's [0x56ef14], its own [0x56ef18] or the right's [0x56ef1c]; nothing
 * against the traffic, with a cop near, or without a lane.  Gives what the code leaves in eax; nullopt
 * when the game stopped. */
std::optional<u32> scoreLaneOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0);
    if ((byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0)
        return car;
    const std::optional<u32> near = copNear(app, ts, car);
    if (!near)
        return std::nullopt;
    if (*near != 0)
        return *near;
    const u32 ahead = guest::direct<0x40e160, aiplan::nodeAhead>(app, ts, car, 4);
    if (ts.terminate)
        return std::nullopt;
    const u32 laneFound = guest::direct<0x40e0c0, aiplan::roadKind>(app, ts, ahead);
    if (ts.terminate)
        return std::nullopt;
    i32 lane = i32(laneFound);
    if (lane == 0)
        return 0;
    u32 score;
    if ((byte(app, car + kCarFlags) & kFlagCop) != 0)
    {
        // Walked towards the middle (7) until within the node's lanes.
        for (;;)
        {
            const u32 n = nodeRecord(app, ahead);
            if (lane >= 7 - ((byte(app, n + kNodeLanes) >> 4) & 0xf) && lane <= (byte(app, n + kNodeLanes) & 0xf) + 6)
                break;
            lane += lane >= 7 ? -1 : 1;
        }
        score = 2;
    }
    else
    {
        const u32 kind = guest::direct<0x404610, airace::classRuleOf>(app, ts, car);
        if (ts.terminate)
            return std::nullopt;
        score = kind == 0 ? 6 : keenness(app, car) > 2 ? 0xc : 0xa;
    }
    const i32 own = s(at(app, car + kCarLane));
    const u32 into = lane < own ? kScoreLeft : lane == own ? kScoreOwn : kScoreRight;
    put(app, into, at(app, into) + score);
    return score;
}

/* 0x4096b0: scoreLaneOf the car eax, eax as the code leaves it. */
nfs::u32 scoreLane(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return scoreLaneOf(app, ts, car); });
    return args.eax;
}

/* The chases' scores where no cop is near the car (0x40f050): 5 more to [0x56eefc] for each chase
 * without a cop, -6 (-24 for the player's car, halved where the chase's flag 0x56ef68 is off) to
 * [0x56ef08] for each of stage under 3.  Gives what the code leaves in eax; nullopt when the game
 * stopped. */
std::optional<u32> scoreChasesOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    const std::optional<u32> near = copNear(app, ts, car);
    if (!near)
        return std::nullopt;
    if (*near != 0)
        return *near;
    u32 left = 0;
    for (u32 i = 0; i < u32(kChases); ++i)
    {
        const u32 cop = at(app, i * 4 + kChaseCops);
        left = cop;
        if (cop == 0)
        {
            put(app, i * 4 + kScoreFree, at(app, i * 4 + kScoreFree) + 5);
            continue;
        }
        if (s(at(app, i * 4 + kChaseStages)) >= 3)
            continue;
        i32 score = (byte(app, cop + kCarFlags) & kFlagPlayer) != 0 ? -0x18 : -6;
        if (half(app, i * 2 + kChaseOn) == 0)
        {
            score /= 2;
            left = u32(score);
        }
        put(app, i * 4 + kScoreStage, at(app, i * 4 + kScoreStage) + u32(score));
    }
    return left;
}

/* 0x4097c0: scoreChasesOf the car eax, eax as the code leaves it. */
nfs::u32 scoreChases(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return scoreChasesOf(app, ts, car); });
    return args.eax;
}

// The debug text's: its row's kind (0x4b) and column (0xf), its chase count's format ("V%d") and a blank.
constexpr u32 kDebugKind = 0x4b, kDebugColumn = 0xf, kChaseCountText = 0x53642c, kBlankText = 0x536430;

/* The car on the radio (+0x621 | 0x80) where it is of a keen class, has been chased (+0x6e4), is not
 * out (+0x7d4) and one of the chases of stage 0 to 4 is its cop's; then the debug text of its chase's
 * count ("V%d", or blank) at its index's row (0x42d520, which keeps every register).  Gives what the
 * code leaves in eax, the row; nullopt when the game stopped. */
std::optional<u32> radioChaseCountOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0);
    if (keenness(app, car) > 2 && s(at(app, car + kCarChaseCount)) > 0 && at(app, car + kCarOut) == 0
        && at(app, car + kCarChased) != 0)
    {
        for (u32 i = 0; i < u32(kChases); ++i)
        {
            if (at(app, car + kCarChased) != at(app, i * 4 + kChaseCops))
                continue;
            const i32 stage = s(at(app, i * 4 + kChaseStages));
            if (stage < 0)
                continue;
            if (stage < 5)
            {
                putByte(app, car + kCarPursuitFlagsHigh, byte(app, car + kCarPursuitFlagsHigh) | kFlagOnRadio);
                break;
            }
        }
    }
    const u32 row = at(app, car + kCarIndex) * 6;
    const bool counted = (byte(app, car + kCarPursuitFlagsHigh) & kFlagOnRadio) != 0;
    // The debug text's call (0x42d520) is an empty function: its words pushed for it are dead.
    guest::direct<0x42d520, raceload::noHook2>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return row;
}

/* 0x409840: radioChaseCountOf the car eax, eax as the code leaves it. */
nfs::u32 radioChaseCount(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioChaseCountOf(app, ts, car); });
    return args.eax;
}

// scoreSpeeds' scales: the speed's two and its offset; the scores' by the speed past the cop's, by the
// target past it, and of the stage's (doubles).
constexpr u32 kStageScale = 0x53643c, kStageScaleToo = 0x536444, kStageOffset = 0x53644c, kTargetScore = 0x536454,
                kStageScore = 0x53645c;

/* The chases' scores [0x56eef0] by speed, for a car of a class of 2 or under: for each chase with a
 * cop, by how much the car's speed and its target are past the cop's (times [0x53644c] and
 * [0x536454]), and where the chase's stage is under the car's speed's (|speed| * [0x53643c] *
 * [0x536444] + [0x53644c]) by [0x53645c] and [0x53644c] again.  Gives what the code leaves in eax. */
u32 scoreSpeedsOf(App* app, u32 car)
{
    if (keenness(app, car) > 2)
        return classTimesThree(app, car);
    const i32 stageBySpeed = fp::toInteger<i32>(fp::add(
        fp::mul(fp::mul(f32(app, car + kCarSpeedAbs), f64(app, kStageScale)), f64(app, kStageScaleToo)),
        f64(app, kStageOffset)));
    u32 left = u32(stageBySpeed);
    // A score where the value is over 0: the value times the scale, as an integer.
    const auto score = [&](double value, u32 scale, u32 into) {
        left = withStatus(left, 0.0, value);
        if (isAboveEqual(0.0, value))
            return;
        left = u32(fp::toInteger<i32>(fp::mul(value, f64(app, scale))));
        put(app, into, at(app, into) + left);
    };
    for (u32 i = 0; i < u32(kChases); ++i)
    {
        const u32 cop = at(app, i * 4 + kChaseCops);
        if (cop == 0)
            continue;
        const Float copSpeed = f32(app, cop + kCarSpeed);
        left = at(app, cop + kCarSpeed);
        const i32 stage = s(at(app, i * 4 + kChaseStages));
        const float bySpeed = float(fp::sub(f32(app, car + kCarSpeed), copSpeed));
        const float byTarget = float(fp::sub(f32(app, car + kCarTarget), copSpeed));
        score(bySpeed, kStageOffset, i * 4 + kScoreSpeed);
        score(byTarget, kTargetScore, i * 4 + kScoreSpeed);
        if (stage >= stageBySpeed)
            continue;
        score(bySpeed, kStageScore, i * 4 + kScoreSpeed);
        score(byTarget, kStageOffset, i * 4 + kScoreSpeed);
    }
    return left;
}

/* 0x409920: scoreSpeedsOf the car eax, eax as the code leaves it. */
nfs::u32 scoreSpeeds(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return scoreSpeedsOf(app, car); });
    return args.eax;
}

/* 0x409a80: 1 in eax -- the check of a cop's state 3, always done. */
nfs::u32 alwaysOne(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [] { return 1; });
    return args.eax;
}

// How far behind a car a cop takes it over: 25 nodes, 12 in the rain or snow, halved at night but for a
// car with its lights (+0x660), 25 for a car marked (+0x710 & 0x10); within 0x41 nodes.
constexpr i32 kTakeOverBehind = 0x19, kTakeOverBehindWet = 0xc;
constexpr u32 kTakeOverReach = 0x41;
constexpr u32 kTakeOverCars = 0x5efc08, kTakeOverCount = 0x553078;

/* For each of the cars [0x5efc08..], [0x553078] of them: where the cop is within reach of it (0x40f000,
 * its node against the node so far behind the car, 0x40e160), the cop takes it over (0x40f100).  Gives
 * what the last of these leaves in eax (0 for none); nullopt when the game stopped. */
std::optional<u32> takeOverCarsOf(App* app, ThreadState& ts, u32 cop)
{
    const GuestFrame frame(ts, 0x18, 4);
    u32 left = 0;
    for (u32 i = 0; s(i) < s(at(app, kTakeOverCount)); ++i)
    {
        const u32 car = at(app, i * 4 + kTakeOverCars);
        if (car == 0)
            continue;
        i32 behind = gameSettings(app).weather != 0 ? kTakeOverBehindWet : kTakeOverBehind;
        if (gameSettings(app).night != 0 && at(app, car + kCarNight) == 0)
            behind /= 2;
        if ((byte(app, car + kCarReach) & 0x10) != 0)
            behind = kTakeOverBehind;
        const u32 back = guest::direct<0x40e160, aiplan::nodeAhead>(app, ts, car, u32(-behind));
        if (ts.terminate)
            return std::nullopt;
        const std::optional<u32> reach = inReach(app, ts, at(app, cop + kCarNode), back, kTakeOverReach);
        if (!reach)
            return std::nullopt;
        left = *reach;
        if (*reach != 0)
        {
            const u32 taken = guest::direct<0x40f100, aiplan::answerOne>(app, ts);
            if (ts.terminate)
                return std::nullopt;
            left = taken;
        }
    }
    return left;
}

/* 0x409a90: takeOverCarsOf the cop eax, eax as the code leaves it. */
nfs::u32 takeOverCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, cop = args.eax] { return takeOverCarsOf(app, ts, cop); });
    return args.eax;
}

/* The cop back to state 1, its record's +0x1c cleared, and the car reset (0x429100) where it may be
 * (+0x621 & 0x40).  Gives what the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> startResetOf(App* app, ThreadState& ts, u32 cop)
{
    const GuestFrame frame(ts, 8, 0);
    const u32 record = at(app, cop + kCarRecord);
    put(app, cop + kCarCopState, 1);
    put(app, record + kRecordReset, 0);
    if ((byte(app, cop + kCarPursuitFlagsHigh) & kFlagMayReset) == 0)
        return cop;
    return guest::direct<0x429100, racestart::resetCarOf>(app, ts, cop);
}

/* 0x409b40: startResetOf the cop eax, eax as the code leaves it. */
nfs::u32 startReset(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, cop = args.eax] { return startResetOf(app, ts, cop); });
    return args.eax;
}

// The race's frames before a car stuck may be reset, and how long it is let stay so.
constexpr i32 kStuckAfter = 0x480, kStuckFor = 0x180;

/* The car's progress: where it has gained no place since [+0x708] (its record's +0x2c 0, the race past
 * 1152 frames, not a network one), 384 frames on from [+0x704] it is reset (0x429100, the frame noted);
 * else its place and the frame noted.  Gives what the code leaves in eax; nullopt when the game
 * stopped. */
std::optional<u32> checkProgressOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    bool stuck = s(at(app, car + kCarProgressPlace)) >= s(at(app, car + kCarProgress))
                 && at(app, at(app, car + kCarRecord) + kRecordHeld) == 0;
    const u32 now = stuck ? at(app, kRaceFrame) : 0;
    stuck = stuck && s(now) > kStuckAfter && gameSettings(app).networkGame == 0;
    if (stuck)
    {
        if (s(at(app, car + kCarProgressFrame) + kStuckFor) >= s(now))
            return car;
        put(app, car + kCarProgressFrame, now);
        return guest::direct<0x429100, racestart::resetCarOf>(app, ts, car);
    }
    put(app, car + kCarProgressPlace, at(app, car + kCarProgress));
    put(app, car + kCarProgressFrame, at(app, kRaceFrame));
    return car;
}

/* 0x409b70: checkProgressOf the car eax, eax as the code leaves it. */
nfs::u32 checkProgress(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return checkProgressOf(app, ts, car); });
    return args.eax;
}
// A cop's states (+0x6e0): chasing, taking a car over, at its node's speed, stopped, pulling a car
// over at the roadside, halted, waiting.
constexpr u32 kCopChasing = 1, kCopTakingOver = 2, kCopAtNodeSpeed = 3, kCopStopped = 4, kCopPullingOver = 5,
                kCopHalted = 6, kCopWaiting = 7;
// The cop's pace from its top speed: the offset and the scale (doubles).
constexpr u32 kPaceOffset = 0x536464, kPaceScale = 0x53646c;
// A car pulled over: moving under [0x536474] it is stopped; under [0x53647c] it is ticketed (doubles).
constexpr u32 kPulledOverMoving = 0x536474, kPulledOverSlow = 0x53647c;
// Each racer's tickets (0x14 bytes a racer), no more than 5; the frames a car is out of the race.
constexpr u32 kTickets = 0x56ea74, kMostTickets = 5, kOutFrames = 0x140;

/* The cop's step, while it is active (0x40df10) -- its radio mark cleared, then by its state [+0x6e0]:
 *   0 (and past 7): its pace from its top speed, state 1, and on as 1;
 *   1 chasing: its progress, then -- unless it is held (0x404b90) -- steered, its target speed, the
 *     scores, driven; out of the race (0x401500) state 5 for 320 frames (640 where it may be reset), a
 *     human's car on a pad state 4; state 2 where it takes over a car (takeOverCarsOf);
 *   2 the same, fewer of them, back to 1 where it takes over none;
 *   3 at its node's speed, then 1;
 *   4 stopped (stopForCops), the frame noted;
 *   5 pulled over at the roadside: once slow enough (or 320 frames on) the car's tickets counted up to
 *     5, then state 6 or reset (startResetOf);
 *   6 halted: 1 again when no longer out, reset when its record says so;
 *   7 the frame noted.
 * Gives what the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> copStepOf(App* app, ThreadState& ts, u32 cop)
{
    const GuestFrame frame(ts, 0x10, 0);
    // One of this file's that may call out, called as the code calls it.
    const auto own = [&](auto core) {
        const CallSlot slot(ts);
        return core(app, ts, cop);
    };
    // The car slowed for its class (airace::slowForClassOf), its eax the answer: false where the game stopped.
    u32 answer = 0;
    const auto resetHeld = [&] {
        const std::optional<bool> slowed = guest::direct<0x404b90, airace::slowForClassOf>(app, ts, cop);
        answer = slowed.value_or(false) ? 1 : 0;
        return slowed.has_value();
    };
    // The cop's lane votes acted on (0x40de30, sima::laneSteer): its eax the answer; false where the game stopped.
    const auto finish = [&] {
        const std::optional<u32> left = feCore<0x40de30, frontend::sima::laneSteer>(app, ts, cop);
        answer = left.value_or(0);
        return left.has_value();
    };
    // Other files' typed functions, as C++: steered, driven, the car's three slices, stopped; out of
    // the race (airace::isPulledOver) its answer; false where the game stopped.
    const auto steer = [&] { return guest::direct<0x40dd60, aiplan::beginPlanning>(app, ts, cop); };
    const auto drive = [&] { return guest::direct<0x40ddb0, aiplan::pickLane>(app, ts, cop); };
    const auto pullOver = [&] {
        guest::direct<0x40e350, aiplan::findCarSlices>(app, ts, cop);
        return !ts.terminate;
    };
    const auto stop = [&] { return guest::direct<0x40ce70, aiplan::brakeToStopDone>(app, ts, cop); };
    const auto out = [&] {
        answer = guest::direct<0x401500, airace::isPulledOver>(app, ts, cop) ? 1 : 0;
        return !ts.terminate;
    };
    // Out of the race (`out`): state 5 for 320 frames (another 320 where `longer` and the car may be
    // reset); a human's car steered with a pad (+0x2c4 3 or 4) state 4.
    const auto outOrPad = [&](u32 out, bool longer) {
        if (out != 0)
        {
            const u32 until = at(app, kRaceFrame) + kOutFrames;
            put(app, cop + kCarCopState, kCopPullingOver);
            const bool mayReset = (byte(app, cop + kCarPursuitFlagsHigh) & kFlagMayReset) != 0;
            put(app, cop + kCarStateUntil, until);
            if (longer && mayReset)
                put(app, cop + kCarStateUntil, until + kOutFrames);
        }
        const bool human = (byte(app, cop + kCarFlags) & kFlagHuman) != 0;
        if ((human && at(app, cop + kCarControl) == 3) || (human && at(app, cop + kCarControl) == 4))
            put(app, cop + kCarCopState, kCopStopped);
    };
    const auto noteFrame = [&] {
        const u32 now = at(app, kRaceFrame);
        put(app, cop + kCarProgressFrame, now);
        return now;
    };
    const std::optional<u32> active = guest::direct<0x40df10, aiplan::stepLoading>(app, ts, cop);
    if (!active)
        return std::nullopt;
    if (*active == 0)
        return 0;
    putByte(app, cop + kCarPursuitFlagsHigh, byte(app, cop + kCarPursuitFlagsHigh) & ~kFlagOnRadio);
    switch (at(app, cop + kCarCopState))
    {
    default:  // 0x409c2e
        put(app, cop + kCarCopState, kCopChasing);
        putFloat(app, cop + kCarPace,
                 float(fp::mul(fp::add(f32(app, at(app, cop + kCarHandling) + kHandlingTopSpeed), f64(app, kPaceOffset)),
                               f64(app, kPaceScale))));
        [[fallthrough]];
    case kCopChasing:  // 0x409c56
    {
        if (!own(checkProgressOf) || !resetHeld())
            return std::nullopt;
        if (answer != 0)
            return answer;
        if (!steer())
            return std::nullopt;
        releaseChase(app, cop);
        if (!own(targetSpeedOf) || !guest::direct<0x404840, airace::rivalTickDone>(app, ts, cop))
            return std::nullopt;
        matchChased(app, cop);
        scoreSpeedsOf(app, cop);
        if (!own(radioChaseCountOf) || !own(scoreChasesOf) || !own(scoreLaneOf) || !own(scoreCloseCopsOf)
            || !drive())
            return std::nullopt;
        growLaneTimesBy(app);
        if (!finish() || !out())
            return std::nullopt;
        outOrPad(answer, true);
        const std::optional<u32> taken = own(takeOverCarsOf);
        if (!taken)
            return std::nullopt;
        if (*taken != 0)
            put(app, cop + kCarCopState, kCopTakingOver);
        return *taken;
    }
    case kCopTakingOver:  // 0x409d4f
    {
        if (!own(checkProgressOf) || !resetHeld())
            return std::nullopt;
        if (answer != 0)
            return answer;
        if (!steer())
            return std::nullopt;
        releaseChase(app, cop);
        if (!own(targetSpeedFramed) || !own(scoreChasesOf) || !drive())
            return std::nullopt;
        growLaneTimesBy(app);
        if (!finish() || !out())
            return std::nullopt;
        outOrPad(answer, false);
        const std::optional<u32> taken = own(takeOverCarsOf);
        if (!taken)
            return std::nullopt;
        if (*taken == 0)
            put(app, cop + kCarCopState, kCopChasing);
        return *taken;
    }
    case kCopAtNodeSpeed:  // 0x409e08
        if (!own(checkProgressOf) || !steer() || !own(nodeTarget) || !drive() || !finish() || !out())
            return std::nullopt;
        outOrPad(answer, false);
        // (0x409a80's check, always 1.)
        put(app, cop + kCarCopState, kCopChasing);
        return 1;
    case kCopStopped:  // 0x409e9c
        stopForCops(app, cop);
        if (!steer() || !finish())
            return std::nullopt;
        return noteFrame();
    case kCopPullingOver:  // 0x409ec1
    {
        if (!pullOver())
            return std::nullopt;
        stopAtRoadside(app, cop);
        addLaneTime(app, cop);
        if (!finish())
            return std::nullopt;
        if (half(app, cop + kCarRaceStage) != 0)
            break;
        if (!isAbove(f32(app, cop + kCarSpeedSize), f64(app, kPulledOverMoving)))
            return noteFrame();
        if (!isBelow(f32(app, cop + kCarSpeedAbs), f64(app, kPulledOverSlow))
            && s(at(app, kRaceFrame)) <= s(at(app, cop + kCarStateUntil)))
            break;
        u32 tickets = at(app, at(app, cop + kCarIndex) * 0x14 + kTickets) + 1;
        if (s(tickets) >= s(kMostTickets))
            tickets = kMostTickets;
        put(app, at(app, cop + kCarIndex) * 0x14 + kTickets, tickets);
        if (at(app, at(app, cop + kCarRecord) + kRecordHeld) != 0)
        {
            put(app, cop + kCarCopState, kCopHalted);
            return noteFrame();
        }
        if (!own(startResetOf))
            return std::nullopt;
        return noteFrame();
    }
    case kCopHalted:  // 0x409fa2
        put(app, cop + kCarTarget, 0);
        if (!stop() || !out())
            return std::nullopt;
        if (answer == 0)
            put(app, cop + kCarCopState, kCopChasing);
        if (at(app, at(app, cop + kCarRecord) + kRecordHeld) == 0 && !own(startResetOf))
            return std::nullopt;
        break;
    case kCopWaiting:  // 0x409fdb
        break;
    }
    return noteFrame();
}

/* 0x409c00: copStepOf the cop eax, eax as the code leaves it. */
nfs::u32 copStep(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, cop = args.eax] { return copStepOf(app, ts, cop); });
    return args.eax;
}

namespace
{

/* strcpy in the guest's memory (the compiler's inline copy, two bytes a turn: the same bytes). */
void copyGuestString(App* app, u32 to, u32 from)
{
    for (;; ++to, ++from)
    {
        const u8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            return;
    }
}

/* strcat in the guest's memory (repne scasb for the end, then the copy). */
void appendGuestString(App* app, u32 to, u32 from)
{
    while (byte(app, to) != 0)
        ++to;
    copyGuestString(app, to, from);
}

// The racers [0x5efc88..] and their count, the speech's folder, the file's name's parts ("IPspch" and
// ".viv"), the language letters' table (7 words among the code), "Pace", the first bank, the banks' most.
constexpr u32 kRacers = 0x5efc88, kRacerCount = 0x5efd9c, kSpeechFolder = 0x7a28b4, kBankName = 0x5364c8,
                kBankSuffix = 0x5364d0, kLanguageLetters = 0x409ff0, kPaceName = 0x53652c;
constexpr u32 kFirstBank = 0xf, kTooManyBanks = 0x17, kTooManyText = 0x536534;
// Each car's speech bank, by its index.
constexpr u32 kSpeechBanks = 0x56ee28;
// The colour handed on for a custom one (whose trace, ea::traceOff, is compiled out: the words the
// code pushes for it nothing reads).
constexpr u32 kCustomColour = 0x63;
// A racer's entry in the race's settings (0x6c bytes).
constexpr u32 kEntrySize = 0x6c;
// The models with speech, 0 to 49.
constexpr i32 kLastSpeakingModel = 0x31;

}  // namespace

/* In a Hot Pursuit race (type 3), each racer's speech bank "<folder>\<car's folder>\IPspch<language
 * letter>.viv" (the letters of the table at 0x409ff0 by the language) loaded as bank 15 on (0x413750) for
 * the cars of models 0 to 49 that are not police cars, its colour's nearest (0x431280, from the racer's
 * colours) handed on -- 99 where the car's colour is the model's custom one (the split screen's players,
 * or the player's: traces) or, for the others, the model's name has "Pace" in it; each racer's bank noted
 * [0x56ee28]; "too many racer banks" printed at the 23rd.  The car's name is copied first into the frame
 * as the code does it, where nothing reads it: not copied.  What the code leaves in eax (`handedOn`, or
 * the racers' count); nullopt when the game stopped. */
std::optional<u32> loadRacerBanksOf(App* app, ThreadState& ts, u32 handedOn)
{
    const GuestFrame frame(ts, 0x18, 0x2a8);
    const u32 path = frame - 0x1a8;
    if (gameSettings(app).raceType != 3)
        return handedOn;
    u32 bank = kFirstBank;
    for (u32 i = 0;; ++i)
    {
        if (s(i) >= s(at(app, kRacerCount)))
            return i;
        const u32 racer = at(app, i * 4 + kRacers);
        const u32 index = at(app, racer + kCarIndex);
        const u32 model = at(app, at(app, racer + kCarChoice) + kChoiceModel);
        if (!guest::direct<0x439250, load::nameOf>(app, ts, model))
            return std::nullopt;
        const auto& colour = gameSettings(app).entryAt(i * kEntrySize).colour;
        const u32 nearestColour =
            guest::direct<0x431280, collide::nearestValue>(app, ts, colour[0], colour[1], colour[2]);
        if (ts.terminate)
            return std::nullopt;
        u32 handed = nearestColour;
        if (s(model) < 0 || s(model) > kLastSpeakingModel)
            continue;
        const std::optional<u32> police = guest::direct<0x4396b0, load::policeFlag>(app, ts, model);
        if (!police)
            return std::nullopt;
        if (*police != 0)
            continue;
        copyGuestString(app, path, kSpeechFolder);
        guest::direct<0x4529a0, port::menus::pathBackslashed>(app, ts, path);
        if (ts.terminate)
            return std::nullopt;
        const std::optional<u32> folder = guest::direct<0x4392b0, load::folderOf>(app, ts, model);
        if (!folder)
            return std::nullopt;
        appendGuestString(app, path, *folder);
        guest::direct<0x4529a0, port::menus::pathBackslashed>(app, ts, path);
        if (ts.terminate)
            return std::nullopt;
        appendGuestString(app, path, kBankName);
        appendGuestString(app, path, at(app, gameSettings(app).language * 4 + kLanguageLetters));
        appendGuestString(app, path, kBankSuffix);
        const u32 playMode = gameSettings(app).playMode;
        if ((index == 0 || index == 1) && playMode == 1)
        {
            const std::optional<u32> own = customColourNumber(app, ts, model);
            const std::optional<u32> custom = own ? chosenColour(app, ts, model, index) : std::nullopt;
            if (!custom)
                return std::nullopt;
            if (*own == *custom)
            {
                guest::direct<0x42d510, ea::traceOff>(app, ts);
                if (ts.terminate)
                    return std::nullopt;
                handed = kCustomColour;
            }
        }
        else if (gameSettings(app).localPlayer == index)
        {
            const std::optional<u32> own = customColourNumber(app, ts, model);
            const std::optional<u32> custom = own ? chosenColour(app, ts, model, 0) : std::nullopt;
            if (!custom)
                return std::nullopt;
            if (*own == *custom)
            {
                guest::direct<0x42d510, ea::traceOff>(app, ts);
                if (ts.terminate)
                    return std::nullopt;
                handed = kCustomColour;
            }
        }
        else
        {
            const std::optional<u32> name = guest::direct<0x439250, load::nameOf>(app, ts, model);
            const std::optional<u32> pace =
                name ? std::optional<u32>(guest::direct<0x4e0820, watcom::clib::strstr>(app, ts, *name, kPaceName))
                     : std::nullopt;
            if (!pace)
                return std::nullopt;
            if (*pace != 0)
                handed = kCustomColour;
        }
        guest::direct<0x413750, port::voiceBankLoaded>(app, ts, ts.sp - 4, path, i32(handed), i32(bank));
        if (ts.terminate)
            return std::nullopt;
        put(app, i * 4 + kSpeechBanks, bank);
        bank += 1;
        if (bank == kTooManyBanks)
        {
            const std::optional<u32> printed = guest::direct<0x4e08f0, ea::monoPrinted>(app, ts, 0u, kTooManyText, guest::Words{});
            if (!printed)
                return std::nullopt;
        }
    }
}

/* 0x40a010: loadRacerBanksOf; eax as the code leaves it. */
nfs::u32 loadRacerBanks(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, handedOn = args.eax] { return loadRacerBanksOf(app, ts, handedOn); });
    return args.eax;
}

/* The radio's frames counted: since anyone spoke [0x5524c8], and each of the 8 voices' [0x56ee68] and
 * [0x5524cc]. */
void tickRadioFrames(App* app)
{
    put(app, kRadioQuiet, at(app, kRadioQuiet) + 1);
    for (u32 voice = 0; voice < u32(kVoices); ++voice)
    {
        const u32 quiet = at(app, voice * 4 + kVoiceQuiet) + 1;
        const u32 spoke = at(app, voice * 4 + kVoiceSpoke) + 1;
        put(app, voice * 4 + kVoiceQuiet, quiet);
        put(app, voice * 4 + kVoiceSpoke, spoke);
    }
}

/* 0x40a330: tickRadioFrames; every register kept. */
nfs::u32 countRadioFrames(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { tickRadioFrames(app); });
    return args.eax;
}

namespace
{

/* The game's random number (0x4e0d88); nullopt when the game stopped. */
std::optional<u32> randomNumber(App* app, ThreadState& ts)
{
    const u32 drawn = guest::direct<0x4e0d88, port::nextRandom>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return drawn;
}

/* The speech's line at `line` said (0x4c85a0): what it leaves in eax; nullopt when the game stopped. */
std::optional<u32> sayLine(App* app, ThreadState& ts, u32 line)
{
    const port::Result said = guest::direct<0x4c85a0, port::speechLineSaid>(app, ts, ts.sp - 4, line);
    if (ts.terminate || !said)
        return std::nullopt;
    return u32(*said);
}

/* The game's random number's remainder by `by`: edx 0 before it, which it keeps, and the remainder of
 * edx:eax after it as div leaves it; nullopt when the game stopped. */
std::optional<u32> randomModulo(App* app, ThreadState& ts, u32 by)
{
    const std::optional<u32> r = randomNumber(app, ts);
    if (!r)
        return std::nullopt;
    return *r % by;
}

// A line of speech's record as 0x4c85a0 reads it, in the frame: the phrase, its volume, its pan, its
// side, the voice's handle, what was handed on, the mark, (a word never written,) the car.
constexpr u32 kLinePhrase = 0, kLineVolume = 4, kLinePan = 8, kLineSide = 0xc, kLineVoice = 0x10, kLineHanded = 0x14,
                kLineMark = 0x18, kLineCar = 0x20;
// The voices' handles (12 bytes each from 0x571124 by the voice), the dispatcher's, the noise's.
constexpr u32 kVoiceHandles = 0x571124, kDispatcherVoice = 0x5711cc, kNoiseVoice = 0x571160;
// The sides: a voice's, the dispatcher's; the volumes: a car's voice, the dispatcher's.
constexpr u32 kSideVoice = 0x3fff, kSideDispatcher = 0xc000, kVolumeVoice = 0x7f, kVolumeDispatcher = 0x6e;
constexpr u32 kPanMiddle = 0x40;
// The dispatcher's own phrase, and the marks: the dispatcher's own, a voice's own, any other.
constexpr u32 kDispatcherPhrase = 0x21, kMarkDispatcher = 7, kMarkVoice = 6, kMarkOther = 5;

/* A car's voice of the 8 by +0x714 mod 8 (signed, the switch at 0x40a3f0): its handle's voice (0xb to
 * 0xd) and its pan; none past it (negative). */
struct VoiceOfEight
{
    bool found;
    u32 voice;
    u32 pan;
};

VoiceOfEight voiceOfCar(App* app, u32 car)
{
    switch (s(at(app, car + kCarVoice)) % 8)
    {
    case 0:
    case 6: return {true, 0xb, 0x40};
    case 1: return {true, 0xd, 0x40};
    case 2: return {true, 0xc, 0x40};
    case 3: return {true, 0xb, 0x50};
    case 4: return {true, 0xd, 0x50};
    case 5: return {true, 0xc, 0x50};
    case 7: return {true, 0xc, 0x40};
    default: return {false, 0, 0};
    }
}

}  // namespace

/* What a burst of the radio's noise leaves in eax, and the phrase it said (in edx, which radioStopped
 * leaves). */
struct NoiseLeft
{
    u32 eax;
    u32 edx;
};

/* A burst of the radio's noise (phrase 0x20 + a random 0 to 8) for the car, `handedOn` handed on, on the
 * right or the left by `side`, at volume 127 (0x4c85a0) -- but in a replay.  What the code leaves in
 * eax and edx: the line's answer and the phrase; nullopt when the game stopped. */
std::optional<NoiseLeft> radioNoiseOf(App* app, ThreadState& ts, u32 handedOn, u32 side, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0x24);
    if (gameSettings(app).replayMode != 0)
        return NoiseLeft{handedOn, side};
    const u32 line = frame - 0x24;
    put(app, line + kLineSide, side != 0 ? kSideVoice : kSideDispatcher);
    put(app, line + kLineVolume, kVolumeVoice);
    put(app, line + kLinePan, kPanMiddle);
    const std::optional<u32> burst = randomModulo(app, ts, 9);
    if (!burst)
        return std::nullopt;
    const u32 phrase = *burst + 0x20;
    put(app, line + kLineHanded, handedOn);
    put(app, line + kLineCar, car);
    put(app, line + kLineMark, kMarkOther);
    put(app, line + kLineVoice, at(app, kNoiseVoice));
    put(app, line + kLinePhrase, phrase);
    const std::optional<u32> said = sayLine(app, ts, line);
    if (!said)
        return std::nullopt;
    return NoiseLeft{*said, phrase};
}

/* 0x40a380: radioNoiseOf for the car ebx, eax handed on, the side edx; eax as the code leaves it. */
nfs::u32 radioNoise(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto left = [&, handedOn = args.eax, side = args.edx, car = args.ebx]() -> std::optional<u32> {
        const std::optional<NoiseLeft> noise = radioNoiseOf(app, ts, handedOn, side, car);
        if (!noise)
            return std::nullopt;
        return noise->eax;
    };
    asThunk(ts, args, left);
    return args.eax;
}

/* The phrase said by the car (0x4c85a0), `handedOn` handed on -- by the dispatcher (volume 110,
 * centred, its own voice [0x5711cc]) or by the car's voice of the 8 (voiceOfCar: its handle [0x571124
 * + 12 voice] and its pan), the mark 7, 6 or 5 by the phrase (0x21 the dispatcher's own); nothing in a
 * replay.  A voice past the 8 (negative) leaves in the line what the caller left in esi and edi
 * (`keptSide`, `keptVoice`) and the pan's slot as it was, as the code does.  What the code leaves in
 * eax; nullopt when the game stopped. */
std::optional<u32> sayPhraseOf(App* app, ThreadState& ts, u32 handedOn, bool byVoice, u32 phrase, u32 car,
                                 u32 keptSide, u32 keptVoice)
{
    const GuestFrame frame(ts, 0xc, 0x34);
    if (gameSettings(app).replayMode != 0)
        return handedOn;
    const u32 line = frame - 0x34, pan = frame - 8;
    u32 side = kSideDispatcher, voice = keptVoice;
    if (byVoice)
    {
        const VoiceOfEight of = voiceOfCar(app, car);
        side = keptSide;
        if (of.found)
        {
            voice = of.voice;
            side = kSideVoice;
            put(app, pan, of.pan);
        }
    }
    put(app, line + kLineVolume, byVoice ? kVolumeVoice : kVolumeDispatcher);
    put(app, line + kLinePan, byVoice ? at(app, pan) : kPanMiddle);
    put(app, line + kLinePhrase, phrase);
    put(app, line + kLineSide, side);
    put(app, line + kLineVoice, byVoice ? at(app, voice * 12 + kVoiceHandles) : at(app, kDispatcherVoice));
    put(app, line + kLineHanded, handedOn);
    put(app, line + kLineMark, phrase == kDispatcherPhrase ? (byVoice ? kMarkVoice : kMarkDispatcher) : kMarkOther);
    put(app, line + kLineCar, car);
    return sayLine(app, ts, line);
}

/* 0x40a410: sayPhraseOf the phrase ebx by the car ecx, eax handed on, by its voice where edx; eax as the
 * code leaves it. */
nfs::u32 sayPhrase(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, handedOn = args.eax, byVoice = args.edx != 0, phrase = args.ebx, car = args.ecx, side = args.esi,
                 voice = args.edi] { return sayPhraseOf(app, ts, handedOn, byVoice, phrase, car, side, voice); });
    return args.eax;
}

/* The phrase of the car's own description said (0x4c85a0) with the speech bank's voice [0x571124 + 12
 * bank], `handedOn` handed on -- by the dispatcher (volume 110, centred) or by the car's voice of the 8
 * (the switch at 0x40a55c: the phrase moved on by 0, 0x10 or 0x20 by the voice, its pan), the mark as
 * sayPhraseOf's; nothing in a replay.  A voice past the 8 leaves in the line what the caller left in esi
 * and edi (`keptPhrase`, `keptSide`) and the pan's slot as it was.  What the code leaves in eax; nullopt
 * when the game stopped. */
std::optional<u32> sayCarPhraseOf(App* app, ThreadState& ts, u32 handedOn, bool byVoice, u32 phrase, u32 car,
                                    u32 bank, u32 keptPhrase, u32 keptSide)
{
    const GuestFrame frame(ts, 0xc, 0x34);
    if (gameSettings(app).replayMode != 0)
        return handedOn;
    const u32 line = frame - 0x34, pan = frame - 8;
    u32 said = phrase, side = kSideDispatcher;
    if (byVoice)
    {
        const VoiceOfEight of = voiceOfCar(app, car);
        said = keptPhrase;
        side = keptSide;
        if (of.found)
        {
            side = kSideVoice;
            said = phrase + (of.voice - 0xb) * 0x10;
            put(app, pan, of.pan);
        }
    }
    put(app, line + kLineVolume, byVoice ? kVolumeVoice : kVolumeDispatcher);
    put(app, line + kLinePan, byVoice ? at(app, pan) : kPanMiddle);
    put(app, line + kLineSide, side);
    put(app, line + kLinePhrase, said);
    put(app, line + kLineVoice, at(app, bank * 12 + kVoiceHandles));
    put(app, line + kLineHanded, handedOn);
    put(app, line + kLineMark, phrase == kDispatcherPhrase ? (byVoice ? kMarkVoice : kMarkDispatcher) : kMarkOther);
    put(app, line + kLineCar, car);
    return sayLine(app, ts, line);
}

/* 0x40a580: sayCarPhraseOf the phrase ebx by the car ecx, eax handed on, by its voice where edx, the
 * bank on the stack (ret 4); eax as the code leaves it. */
nfs::u32 sayCarPhrase(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(
        ts, args,
        [&, handedOn = args.eax, byVoice = args.edx != 0, phrase = args.ebx, car = args.ecx, bank = at(app, ts.sp + 4),
         keptPhrase = args.esi, keptSide = args.edi] {
            return sayCarPhraseOf(app, ts, handedOn, byVoice, phrase, car, bank, keptPhrase, keptSide);
        },
        4);
    return args.eax;
}

// The radio's phrases' names: 8 bytes each, 7 letters at most.
constexpr u32 kPhraseNames = 0x56ec90, kPhraseNameLength = 7;

/* The name of the radio's phrase from the text (strncpy, 7 at most).  What strncpy leaves in eax;
 * nullopt when the game stopped. */
std::optional<u32> namePhraseAs(App* app, ThreadState& ts, u32 phrase, u32 text)
{
    const GuestFrame frame(ts, 8, 0);
    const u32 buffer = guest::direct<0x4e0e30, watcom::clib::strncpy>(app, ts, phrase * 8 + kPhraseNames, text,
                                                                         kPhraseNameLength);
    if (ts.terminate)
        return std::nullopt;
    return buffer;
}

/* 0x40a6d0: namePhraseAs the phrase eax from the text edx; eax as strncpy leaves it. */
nfs::u32 namePhrase(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, phrase = args.eax, text = args.edx] { return namePhraseAs(app, ts, phrase, text); });
    return args.eax;
}

/* The radio's phrases named at the race's start (ALLU, u24, u31, GO24, 10-4, inPrst, join, resume,
 * abort, stopped, help, sight, ambul, ticket, arrest, block, patrol, the cars Corv, Lamb, Jaguar, Merced
 * in their colours, speeder, 100+, 150+, 200+, donuts, school, town), then the racers' speech banks
 * loaded (loadRacerBanks).  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> namePhrasesOf(App* app, ThreadState& ts)
{
    static const u32 kPhrases[][2] = {
        {0x536560, 0},    {0x536568, 1},    {0x53656c, 2},    {0x536570, 3},    {0x536578, 4},
        {0x536580, 5},    {0x536580, 6},    {0x536588, 7},    {0x536590, 8},    {0x536598, 9},
        {0x5365a0, 0xb},  {0x5365a8, 0xc},  {0x5365b0, 0xe},  {0x5365b8, 0xf},  {0x5365c0, 0x11},
        {0x5365c8, 0x12}, {0x5365d0, 0x13}, {0x5365d8, 0x15}, {0x5365e0, 0x16}, {0x5365e8, 0x17},
        {0x5365f0, 0x24}, {0x5365f8, 0x25}, {0x536600, 0x26}, {0x536608, 0x27}, {0x536610, 0x28},
        {0x536618, 0x29}, {0x536620, 0x2a}, {0x536628, 0x2b}, {0x536630, 0x2c}, {0x536638, 0x2d},
        {0x536640, 0x2e}, {0x536648, 0x2f}, {0x536650, 0x30}, {0x536658, 0x31}, {0x536660, 0x32},
    };
    const GuestFrame frame(ts, 8, 0);
    u32 left = 0;
    for (const auto& phrase : kPhrases)
    {
        const CallSlot slot(ts);
        const std::optional<u32> named = namePhraseAs(app, ts, phrase[1], phrase[0]);
        if (!named)
            return std::nullopt;
        left = *named;
    }
    const CallSlot slot(ts);
    return loadRacerBanksOf(app, ts, left);
}

/* 0x40a6f0: namePhrasesOf; eax as the code leaves it. */
nfs::u32 namePhrases(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&] { return namePhrasesOf(app, ts); });
    return args.eax;
}

// The radio's load past which no message of any weight is said.
constexpr i32 kRadioFull = 0xc;

/* Whether the radio is free for a message of the weight (with the radio's load [0x7dd378], under 12):
 * 2 where the car is the radio's [0x5524f0], 1 else, 0 where it is not. */
u32 radioFreeFor(App* app, u32 weight, u32 car)
{
    if (s(weight + at(app, kRadioLoad)) >= kRadioFull)
        return 0;
    return car == at(app, kRadioCar) ? 2 : 1;
}

/* 0x40a910: radioFreeFor the weight eax and the car edx, in eax. */
nfs::u32 radioFree(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, weight = args.eax, car = args.edx] { return radioFreeFor(app, weight, car); });
    return args.eax;
}

/* 0x40a940, 0x40aea0, 0x40b7f0: empty -- the radio's message done, a car's description's hook, and a
 * message of nothing (it pushes ebx and edx too): nothing but the return. */
nfs::u32 radioNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

nfs::u32 radioNothingToo(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioNothing(app, ts, args);
}

nfs::u32 radioNothingThree(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioNothing(app, ts, args);
}

/* 0x40b7f0's: a message of nothing for the cop: nothing at all. */
void radioNothingFor(App*, u32)
{
}

/* The voice's frames since it spoke [0x56ee68] back to 0. */
void quietVoiceOf(App* app, u32 voice)
{
    put(app, voice * 4 + kVoiceQuiet, 0);
}

/* 0x40a950: quietVoiceOf the voice eax where edx is 1. */
nfs::u32 quietVoice(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, voice = args.eax, quiet = args.edx == 1] {
        if (quiet)
            quietVoiceOf(app, voice);
    });
    return args.eax;
}

// The voice that spoke last.
constexpr u32 kLastVoice = 0x5524ec;
// A voice's phrases: its call, its unit's call, its own reply; the dispatcher's two replies.
constexpr u32 kCallSign = 0x10, kUnitCall = 0x14, kOwnReply = 0x24, kReplyAsked = 0x20, kReplyNotAsked = 0x22;

/* The car's voice's reply where it did not speak last [0x5524ec]: `asked`, now and then its unit's call
 * and then phrase 0x20; not asked, its own reply (+0x24) now and then, else now and then its unit's call
 * and then phrase 0x22 -- each by the dispatcher (sayPhraseOf).  What the code leaves in eax; nullopt when
 * the game stopped. */
std::optional<u32> sayVoicePhraseOf(App* app, ThreadState& ts, u32 car, bool asked)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (at(app, kLastVoice) == at(app, car + kCarVoice))
        return at(app, kLastVoice);
    // The dispatcher says it, 0 handed on (no voice of the 8: nothing kept in the line).
    const auto say = [&](u32 phrase) {
        const CallSlot slot(ts);
        return sayPhraseOf(app, ts, 0, false, phrase, car, 0, 0);
    };
    // Half the time and a little less: the random number's remainder by 100 past 50.
    const auto now = [&]() -> std::optional<bool> {
        const std::optional<u32> r = randomModulo(app, ts, 0x64);
        if (!r)
            return std::nullopt;
        return *r > 0x32;
    };
    u32 last = kReplyAsked;
    if (asked)
    {
        const std::optional<bool> unit = now();
        if (!unit || (*unit && !say(at(app, car + kCarVoice) + kUnitCall)))
            return std::nullopt;
    }
    else
    {
        const std::optional<bool> own = now();
        if (!own)
            return std::nullopt;
        if (*own)
            return say(at(app, car + kCarVoice) + kOwnReply);
        const std::optional<bool> unit = now();
        if (!unit || (*unit && !say(at(app, car + kCarVoice) + kUnitCall)))
            return std::nullopt;
        last = kReplyNotAsked;
    }
    return say(last);
}

/* 0x40a970: sayVoicePhraseOf the car eax, asked where edx; eax as the code leaves it. */
nfs::u32 sayVoicePhrase(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, asked = args.edx != 0] { return sayVoicePhraseOf(app, ts, car, asked); });
    return args.eax;
}

// The frames a voice keeps quiet before it gives its call again.
constexpr i32 kCallSignQuiet = 0x78;

/* The car's call sign (its voice + 0x10) said by its voice, where another spoke last or its voice has
 * been quiet 120 frames [0x5524cc], whose count starts again (a voice past the 8 takes the caller's edi
 * for its handle, as the code does).  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> sayCallSignOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0);
    const u32 voice = at(app, car + kCarVoice);
    if (at(app, kLastVoice) == voice && s(at(app, voice * 4 + kVoiceSpoke)) < kCallSignQuiet)
        return car;
    put(app, at(app, car + kCarVoice) * 4 + kVoiceSpoke, 0);
    const CallSlot slot(ts);
    return sayPhraseOf(app, ts, 0, true, at(app, car + kCarVoice) + kCallSign, car, 0, g_callerEdi);
}

/* 0x40aa20: sayCallSignOf the car eax; eax as the code leaves it. */
nfs::u32 sayCallSign(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return sayCallSignOf(app, ts, car); });
    return args.eax;
}

/* The phrase said by the dispatcher, of no car (no voice of the 8: nothing kept in the line).  What
 * the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> sayPhraseOnlyOf(App* app, ThreadState& ts, u32 phrase)
{
    const GuestFrame frame(ts, 0x10, 0);
    const CallSlot slot(ts);
    return sayPhraseOf(app, ts, 0, false, phrase, 0, 0, 0);
}

/* 0x40aa70: sayPhraseOnlyOf the phrase eax; eax as the code leaves it. */
nfs::u32 sayPhraseOnly(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, phrase = args.eax] { return sayPhraseOnlyOf(app, ts, phrase); });
    return args.eax;
}

/* The car's unit (its voice + 0x14) said by the dispatcher (nothing kept in the line).  What the code
 * leaves in eax; nullopt when the game stopped. */
std::optional<u32> sayUnitOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    const CallSlot slot(ts);
    return sayPhraseOf(app, ts, 0, false, at(app, car + kCarVoice) + kUnitCall, car, 0, 0);
}

/* 0x40aa90: sayUnitOf the car eax; eax as the code leaves it. */
nfs::u32 sayUnit(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return sayUnitOf(app, ts, car); });
    return args.eax;
}
// A "help" pending.
constexpr u32 kHelpPending = 0x5524f4;

/* The message of the kind (8 to 35, the switch at 0x40aab0) said about the car, by its voice (`whom` 1)
 * or the dispatcher (0): for 8 to 11 the chased car's description in its bank [0x56ee28] (sayCarPhraseOf,
 * phrases 0, 0x10, 0x11, 0x12; 11 by a voice with a "help" first, 0x24 on at random, unless one is
 * pending [0x5524f4]); else phrases of the kind, some picked at random of two to four (0x4e0d88): 12 and
 * 13 0x20.., 15 0x28.., 16 0x30.., 19 0x2c.. (of three), 20 0x2c.., 24 0xc.., 25 0x1e.., 26 0x1d, 27
 * 0x34../0x37.., 28 0x3a../0x33.., 29 0x31 and 30 0x32 (a "help" pending after), 31 to 33 0x37 or 0x39 to
 * 0x3b, 34 0x3c or 0x35.., 35 0x1a..; nothing for the rest.  What the code leaves in eax; nullopt when
 * the game stopped. */
std::optional<u32> sayDescriptionOf(App* app, ThreadState& ts, u32 whom, u32 car, u32 kind)
{
    const GuestFrame frame(ts, 0x10, 0);
    const bool byVoice = whom != 0;
    // A phrase said, 0 handed on (esi `side` -- whom but for kind 30 -- and edi the car, as the code
    // leaves them for a voice past the 8).
    const auto say = [&](u32 phrase, u32 side) {
        const CallSlot slot(ts);
        return sayPhraseOf(app, ts, 0, byVoice, phrase, car, side, car);
    };
    // One of two or four phrases on from `phrase`, at random.
    const auto pick = [&](u32 mask, u32 phrase) -> std::optional<u32> {
        const std::optional<u32> r = randomNumber(app, ts);
        if (!r)
            return std::nullopt;
        return say((*r & mask) + phrase, whom);
    };
    // A phrase of the chased car's own description, with its bank (pushed, ret 4).
    const auto sayCar = [&](u32 phrase) {
        const u32 bank = at(app, at(app, at(app, car + kCarChased) + kCarIndex) * 4 + kSpeechBanks);
        const CallSlot slot(ts, 1);
        return sayCarPhraseOf(app, ts, 0, byVoice, phrase, car, bank, whom, car);
    };
    const u32 none = whom;
    if (kind - 8 > 0x1b)
        return none;
    switch (kind)
    {
    case 8: return sayCar(byVoice ? 0x10 : 0);
    case 9: return sayCar(byVoice ? 0x11 : 0);
    case 10: return sayCar(0x12);
    case 11:
    {
        if (byVoice && at(app, kHelpPending) == 0 && !pick(3, 0x24))
            return std::nullopt;
        const std::optional<u32> left = sayCar(byVoice ? 0x11 : 0);
        if (!left)
            return std::nullopt;
        put(app, kHelpPending, 0);
        return left;
    }
    case 12:
    case 13: return pick(3, 0x20);
    case 15: return pick(3, 0x28);
    case 16: return pick(3, 0x30);
    case 19:
    {
        const std::optional<u32> r = randomModulo(app, ts, 3);
        if (!r)
            return std::nullopt;
        return say(*r + 0x2c, whom);
    }
    case 20: return pick(3, 0x2c);
    case 24: return pick(1, 0xc);
    case 25: return pick(1, 0x1e);
    case 26: return say(0x1d, whom);
    case 27: return pick(1, byVoice ? 0x37 : 0x34);
    case 28: return pick(1, byVoice ? 0x33 : 0x3a);
    case 29:
    case 30:
    {
        const std::optional<u32> left = kind == 29 ? say(0x31, whom) : say(0x32, 1);
        if (!left)
            return std::nullopt;
        put(app, kHelpPending, 1);
        return left;
    }
    case 31: return say(byVoice ? 0x39 : 0x37, whom);
    case 32: return say(byVoice ? 0x3a : 0x37, whom);
    case 33: return say(byVoice ? 0x3b : 0x37, whom);
    case 34: return byVoice ? pick(1, 0x35) : say(0x3c, whom);
    case 35: return pick(1, 0x1a);
    default: return none;  // 14, 17, 18, 21 to 23
    }
}

/* 0x40ab20: sayDescriptionOf the kind ebx about the car edx by whom eax; eax as the code leaves it. */
nfs::u32 sayDescription(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, whom = args.eax, car = args.edx, kind = args.ebx] { return sayDescriptionOf(app, ts, whom, car, kind); });
    return args.eax;
}

// Each voice's last place said and its prefix ("100+"-like).
constexpr u32 kVoicePlace = 0x5524f8, kVoicePrefix = 0x552520;
// A place's record (0x4c8550, 0x4c8470 write it, in the frame): its phrase, its distance, its kind.
constexpr u32 kPlacePhrase = 0, kPlaceDistance = 4, kPlaceKind = 8;
// A place's phrase said for the dispatcher (moved on by 0x20), the prefixes past 100 and 50.
constexpr u32 kPlaceAgain = 0x20, kPrefixFar = 0x7f, kPrefixNear = 0x7e;
constexpr u32 kNoPlace = 0xffffffff;

/* __CHP: the value to an integer, towards 0, as a whole value; never nullopt (the optional is the
 * callers' stop signal). */
std::optional<Float> chop(App* app, ThreadState& ts, Float value)
{
    return Float(guest::direct<kChop, watcom::clib::chop>(app, ts, double(value)));
}

/* Where the car is said to be, by its voice (`whom` 1) or the dispatcher (0): the dispatcher repeats the
 * voice's last place (with its prefix 0x20 on, the prefix's phrase first); else the place the car is
 * heading for, its node on by its speed / 6 times [0x7dd378] twice, mod the track's nodes: the nearest
 * named place there (0x4c8550, into the frame's record: its phrase, distance, kind) -- 0 or 0x20 on at
 * random for kind 2 -- or else the next (0x4c8470), with the prefixes 0x7f and 0x7e for a road of kind 1
 * past 100 and 50; each voice's last place and prefix noted [0x5524f8], [0x552520] -- but where the same
 * voice spoke last of the same place.  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> sayLocationOf(App* app, ThreadState& ts, u32 whom, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0x18);
    const u32 record = frame - 0x18;
    const auto voice = [&] { return at(app, car + kCarVoice); };
    // The phrase said by `whom`, 0 handed on (esi the car, edi whom, as the code has them).
    const auto say = [&](u32 phrase) {
        const CallSlot slot(ts);
        return sayPhraseOf(app, ts, 0, whom != 0, phrase, car, car, whom);
    };
    if (whom == 0)
    {
        const u32 slot = at(app, car + kCarVoice) * 4;
        if (at(app, slot + kVoicePlace) != kNoPlace)
        {
            if (at(app, kLastVoice) == voice())
                return slot;
            const u32 prefix = at(app, slot + kVoicePrefix);
            u32 phrase = at(app, slot + kVoicePlace);
            if (prefix != 0)
            {
                if (!say(prefix))
                    return std::nullopt;
                phrase = at(app, voice() * 4 + kVoicePlace) + kPlaceAgain;
            }
            if (!say(phrase))
                return std::nullopt;
            put(app, voice() * 4 + kVoicePlace, kNoPlace);
            return voice();
        }
    }
    const std::optional<Float> chopped = chop(app, ts, f32(app, car + kCarSpeed));
    if (!chopped)
        return std::nullopt;
    const i32 speed = fp::toInteger<i32>(*chopped);
    const u32 size = speed > 0 ? u32(speed) : 0u - u32(speed);
    const u32 ahead = u32(std::int64_t(s(size)) / 6) * at(app, kRadioLoad);
    const u32 onto = u32(std::int64_t(s(at(app, car + kCarNode) + ahead + ahead)) % s(at(app, kNodeCount)));
    const std::optional<u32> nearest = feCore<0x4c8550, frontend::tail::placeAtNode>(app, ts, onto, record);
    if (!nearest)
        return std::nullopt;
    // The same voice spoke last of the same place: what the code's compares leave in eax.
    const auto again = [&]() -> std::optional<u32> {
        if (at(app, kLastVoice) != voice())
            return std::nullopt;
        const u32 phrase = at(app, record + kPlacePhrase);
        if (phrase != at(app, voice() * 4 + kVoicePlace))
            return std::nullopt;
        return phrase;
    };
    if (*nearest != kNoPlace)
    {
        u32 said = at(app, record + kPlacePhrase);
        if (at(app, record + kPlaceKind) == 2)
        {
            const std::optional<u32> r = randomNumber(app, ts);
            if (!r)
                return std::nullopt;
            said = at(app, record + kPlacePhrase) + ((*r & 1) << 5);
        }
        if (const std::optional<u32> same = again())
            return same;
        if (!say(said))
            return std::nullopt;
        put(app, voice() * 4 + kVoicePlace, said);
        put(app, voice() * 4 + kVoicePrefix, 0);
        return voice();
    }
    const std::optional<u32> next = feCore<0x4c8470, frontend::tail::placeNearNode>(app, ts, onto, record);
    if (!next)
        return std::nullopt;
    if (*next == 0)
        return 0;
    if (const std::optional<u32> same = again())
        return same;
    for (const u32 prefix : {kPrefixFar, kPrefixNear})
    {
        if (!(s(at(app, record + kPlaceDistance)) > (prefix == kPrefixFar ? 0x64 : 0x32) && at(app, record + kPlaceKind) == 1))
            continue;
        if (!say(prefix) || !say(at(app, record + kPlacePhrase) + kPlaceAgain))
            return std::nullopt;
        put(app, voice() * 4 + kVoicePlace, at(app, record + kPlacePhrase));
        put(app, voice() * 4 + kVoicePrefix, prefix);
        return voice();
    }
    const u32 kind = at(app, record + kPlaceKind);
    if (kind == 0)
    {
        if (!say(at(app, record + kPlacePhrase) + kPlaceAgain))
            return std::nullopt;
        put(app, voice() * 4 + kVoicePlace, at(app, record + kPlacePhrase) + kPlaceAgain);
    }
    else if (kind == 2)
    {
        const std::optional<u32> r = randomNumber(app, ts);
        if (!r)
            return std::nullopt;
        const u32 said = at(app, record + kPlacePhrase) + ((*r & 1) << 5);
        if (!say(said))
            return std::nullopt;
        put(app, voice() * 4 + kVoicePlace, said);
    }
    else
    {
        if (!say(at(app, record + kPlacePhrase)))
            return std::nullopt;
        put(app, voice() * 4 + kVoicePlace, at(app, record + kPlacePhrase));
    }
    put(app, voice() * 4 + kVoicePrefix, 0);
    return voice();
}

/* 0x40aeb0: sayLocationOf the car edx by whom eax; eax as the code leaves it. */
nfs::u32 sayLocation(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, whom = args.eax, car = args.edx] { return sayLocationOf(app, ts, whom, car); });
    return args.eax;
}
// The speeds past which the speed is said (doubles), the phrases "200+", "150+", "100+" and the last.
constexpr u32 kSpeedSaid[] = {0x53666c, 0x536674, 0x53667c, 0x536684};
constexpr u32 kSpeedPhrases[] = {0x3f, 0x3e, 0x3d, 0x3c};

/* The speed of the car `about` said for the car, by its voice where `whom` (sayPhraseOf, a voice past
 * the 8 with the caller's edi for its handle, as the code does): the phrase
 * 0x3f, 0x3e, 0x3d or 0x3c past [0x53666c], [0x536674], [0x53667c], [0x536684]; none slower.  What the
 * code leaves in eax (the status word of the last compare where none is said); nullopt when the game
 * stopped. */
std::optional<u32> saySpeedOf(App* app, ThreadState& ts, u32 whom, u32 about, u32 car)
{
    const GuestFrame frame(ts, 0xc, 8);
    const Float speed = f32(app, about + kCarSpeedAbs);
    for (int k = 0; k < 4; ++k)
    {
        if (!isAbove(speed, f64(app, kSpeedSaid[k])))
            continue;
        const CallSlot slot(ts);
        const std::optional<u32> said = sayPhraseOf(app, ts, 0, whom != 0, kSpeedPhrases[k], car, whom, g_callerEdi);
        if (!said)
            return std::nullopt;
        return *said;
    }
    return withStatus(whom, speed, f64(app, kSpeedSaid[3]));
}

/* 0x40b190: saySpeedOf the car edx for the car ebx, by whom eax; eax as the code leaves it. */
nfs::u32 saySpeed(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, whom = args.eax, about = args.edx, car = args.ebx] { return saySpeedOf(app, ts, whom, about, car); });
    return args.eax;
}

namespace
{

// The radio's messages' steps, each as the code makes it (nullopt where the game stopped).

std::optional<u32> describe(App* app, ThreadState& ts, u32 whom, u32 car, u32 kind)
{
    const CallSlot slot(ts);
    return sayDescriptionOf(app, ts, whom, car, kind);
}

std::optional<u32> locate(App* app, ThreadState& ts, u32 whom, u32 car)
{
    const CallSlot slot(ts);
    return sayLocationOf(app, ts, whom, car);
}

std::optional<u32> callSign(App* app, ThreadState& ts, u32 car)
{
    const CallSlot slot(ts);
    return sayCallSignOf(app, ts, car);
}

std::optional<u32> reply(App* app, ThreadState& ts, u32 car, bool asked)
{
    const CallSlot slot(ts);
    return sayVoicePhraseOf(app, ts, car, asked);
}

std::optional<u32> dispatcher(App* app, ThreadState& ts, u32 phrase)
{
    const CallSlot slot(ts);
    return sayPhraseOnlyOf(app, ts, phrase);
}

std::optional<u32> unit(App* app, ThreadState& ts, u32 car)
{
    const CallSlot slot(ts);
    return sayUnitOf(app, ts, car);
}

std::optional<NoiseLeft> noiseFor(App* app, ThreadState& ts, u32 car, u32 side)
{
    const CallSlot slot(ts);
    return radioNoiseOf(app, ts, 0, side, car);
}

std::optional<u32> speedOf(App* app, ThreadState& ts, u32 car)
{
    const CallSlot slot(ts);
    return saySpeedOf(app, ts, 1, at(app, car + kCarChased), car);
}

/* The end of a message: the car's voice the one that spoke last; it is left in eax. */
u32 spokeLast(App* app, u32 car)
{
    const u32 voice = at(app, car + kCarVoice);
    put(app, kLastVoice, voice);
    return voice;
}

/* The chased car is a cop's. */
bool chasesCop(App* app, u32 car)
{
    return (byte(app, at(app, car + kCarChased) + kCarFlags) & kFlagCop) != 0;
}

// The frames since anyone spoke past which the dispatcher answers.
constexpr i32 kRadioLongQuiet = 0x3e8;

}  // namespace

/* A cop's sighting of the car it chases: the radio free (weight 0), the voice's quiet count cleared,
 * its call sign, the chased car's description (kind 8), where it heads, its speed now and then, the
 * noise; then the dispatcher's answer (phrase 0x10, the unit, the description, the place now and then),
 * the noise, the voice noted and the frames since anyone spoke cleared.  What the code leaves in eax;
 * nullopt when the game stopped. */
std::optional<u32> radioSpottedOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0);
    const std::optional<u32> place = randomModulo(app, ts, 0x64);
    if (!place)
        return std::nullopt;
    const std::optional<u32> speed = randomModulo(app, ts, 0x64);
    if (!speed)
        return std::nullopt;
    if (radioFreeFor(app, 0, 0) == 0)
        return 0;
    quietVoiceOf(app, at(app, car + kCarVoice));
    if (!callSign(app, ts, car) || !describe(app, ts, 1, car, 8) || !locate(app, ts, 1, car))
        return std::nullopt;
    if (s(*speed) > 0x32 && !speedOf(app, ts, car))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 1) || !dispatcher(app, ts, 0x10) || !unit(app, ts, car) || !describe(app, ts, 0, car, 8))
        return std::nullopt;
    if (s(*place) > 0x32 && !locate(app, ts, 0, car))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 0))
        return std::nullopt;
    const u32 voice = spokeLast(app, car);
    put(app, kRadioQuiet, 0);
    return voice;
}

/* 0x40b220: radioSpottedOf the cop eax; eax as the code leaves it. */
nfs::u32 radioSpotted(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioSpottedOf(app, ts, car); });
    return args.eax;
}

/* A cop's report that it is in pursuit of its chased car (a cop's; the radio free at weight 4): of the
 * car the radio is on (radioFreeFor 2: kind 10, short) or with its description (kind 9 or 10), place and,
 * the radio quiet for 1000 frames and free, the dispatcher's answer (phrase 0x11, the unit, kind 9, the
 * place now and then), else the voice's own reply.  What the code leaves in eax; nullopt when the game
 * stopped. */
std::optional<u32> radioPursuitOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    const u32 free = radioFreeFor(app, 4, 1);
    if (free == 0)
        return 0;
    if (!chasesCop(app, car))
        return at(app, car + kCarChased);
    quietVoiceOf(app, at(app, car + kCarVoice));
    bool answered = false;
    if (free == 2)
    {
        if (!callSign(app, ts, car) || !describe(app, ts, 1, car, 0xa) || !noiseFor(app, ts, car, 1))
            return std::nullopt;
    }
    else
    {
        if (!callSign(app, ts, car))
            return std::nullopt;
        const std::optional<u32> pick = randomModulo(app, ts, 0x64);
        if (!pick || !describe(app, ts, 1, car, *pick > 0x32 ? 9 : 0xa) || !locate(app, ts, 1, car)
            || !noiseFor(app, ts, car, 1))
            return std::nullopt;
        if (s(at(app, kRadioQuiet)) > kRadioLongQuiet && radioFreeFor(app, 3, 1) != 0)
        {
            if (!dispatcher(app, ts, 0x11) || !unit(app, ts, car) || !describe(app, ts, 0, car, 9))
                return std::nullopt;
            const std::optional<u32> where = randomModulo(app, ts, 0x64);
            if (!where || (*where > 0x32 && !locate(app, ts, 0, car)))
                return std::nullopt;
            put(app, kRadioQuiet, 0);
            answered = true;
        }
    }
    if ((!answered && !reply(app, ts, car, true)) || !noiseFor(app, ts, car, 0))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40b330: radioPursuitOf the cop eax; eax as the code leaves it. */
nfs::u32 radioPursuit(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioPursuitOf(app, ts, car); });
    return args.eax;
}

// The frames a voice keeps quiet before it says it lost the car.
constexpr i32 kLostQuiet = 0x1f4;

/* A cop's report that it lost the car (a cop's chased car, the radio free at weight 3): no voice last;
 * a voice quiet under 500 frames says nothing; else, but for the car the radio is on, its call sign,
 * its reply and the description of kind 0xc, the noise, its reply asked, the noise.  What the code
 * leaves in eax; nullopt when the game stopped. */
std::optional<u32> radioLostOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    const u32 free = radioFreeFor(app, 3, 3);
    if (free == 0)
        return 0;
    if (!chasesCop(app, car))
        return free;
    put(app, kLastVoice, 0xffffffff);
    const u32 voice = at(app, car + kCarVoice);
    if (s(at(app, voice * 4 + kVoiceQuiet)) < kLostQuiet)
        return voice;
    if (free == 2)
        return free;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, 0xc)
        || !noiseFor(app, ts, car, 1) || !reply(app, ts, car, true))
        return std::nullopt;
    const std::optional<NoiseLeft> noise = noiseFor(app, ts, car, 0);
    if (!noise)
        return std::nullopt;
    return noise->eax;
}

/* 0x40b480: radioLostOf the cop eax; eax as the code leaves it. */
nfs::u32 radioLost(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioLostOf(app, ts, car); });
    return args.eax;
}

/* A cop's call for backup on its chased car (a cop's; the radio free at weight 3): its call sign, its
 * reply and the description of kind 0xb, the place or the speed, the noise; now and then, or the radio
 * quiet 1000 frames, the dispatcher's answer (phrase 0x11, the unit, the description, the place) where
 * the radio is free, else the voice's reply; the noise.  What the code leaves in eax; nullopt when the
 * game stopped. */
std::optional<u32> radioBackupOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0);
    const std::optional<u32> place = randomModulo(app, ts, 0x64);
    if (!place)
        return std::nullopt;
    if (radioFreeFor(app, 3, 2) == 0)
        return 0;
    if (!chasesCop(app, car))
        return at(app, car + kCarChased);
    quietVoiceOf(app, at(app, car + kCarVoice));
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, 0xb))
        return std::nullopt;
    if (s(*place) > 0x32 ? !locate(app, ts, 1, car) : !speedOf(app, ts, car))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 1))
        return std::nullopt;
    const std::optional<u32> answer = randomModulo(app, ts, 0x64);
    if (!answer)
        return std::nullopt;
    bool answered = false;
    if ((*answer > 0x32 || s(at(app, kRadioQuiet)) > kRadioLongQuiet) && radioFreeFor(app, 3, 4) != 0)
    {
        if (!dispatcher(app, ts, 0x11) || !unit(app, ts, car) || !describe(app, ts, 0, car, 0xb))
            return std::nullopt;
        if (s(*place) > 0x32 && !locate(app, ts, 0, car))
            return std::nullopt;
        put(app, kRadioQuiet, 0);
        answered = true;
    }
    if ((!answered && !reply(app, ts, car, true)) || !noiseFor(app, ts, car, 0))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40b530: radioBackupOf the cop eax; eax as the code leaves it. */
nfs::u32 radioBackup(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioBackupOf(app, ts, car); });
    return args.eax;
}

/* A cop on patrol, the car it chases not a cop's (the radio free at weight 4): no voice last, its call
 * sign, its reply, the description of kind 0x13 (0x14 for a human's car on a pad), the noise, its reply
 * asked, the noise.  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> radioPatrolOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 4, 4) == 0)
        return 0;
    if (chasesCop(app, car))
        return at(app, car + kCarChased);
    put(app, kLastVoice, 0xffffffff);
    if (!callSign(app, ts, car) || !reply(app, ts, car, false))
        return std::nullopt;
    const u32 chased = at(app, car + kCarChased);
    const u32 kind = (byte(app, chased + kCarFlags) & kFlagHuman) != 0 && at(app, chased + kCarControl) == 4 ? 0x14 : 0x13;
    if (!describe(app, ts, 1, car, kind) || !noiseFor(app, ts, car, 1) || !reply(app, ts, car, true))
        return std::nullopt;
    const std::optional<NoiseLeft> noise = noiseFor(app, ts, car, 0);
    if (!noise)
        return std::nullopt;
    return noise->eax;
}

/* 0x40b670: radioPatrolOf the cop eax; eax as the code leaves it. */
nfs::u32 radioPatrol(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioPatrolOf(app, ts, car); });
    return args.eax;
}

/* A roadblock (the radio free at weight 4): no voice last, the call sign, the description of kind 0xf,
 * the place, the noise; the dispatcher's answer (phrase 0x12, the unit, kind 0x10, the place) where the
 * radio is still free, else the voice's reply; the noise.  What the code leaves in eax; nullopt when the
 * game stopped. */
std::optional<u32> radioRoadblockOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 4, 4) == 0)
        return 0;
    put(app, kLastVoice, 0xffffffff);
    if (!callSign(app, ts, car) || !describe(app, ts, 1, car, 0xf) || !locate(app, ts, 1, car)
        || !noiseFor(app, ts, car, 1))
        return std::nullopt;
    if (radioFreeFor(app, 4, 4) != 0)
    {
        if (!dispatcher(app, ts, 0x12) || !unit(app, ts, car) || !describe(app, ts, 0, car, 0x10)
            || !locate(app, ts, 0, car))
            return std::nullopt;
        put(app, kRadioQuiet, 0);
    }
    else if (!reply(app, ts, car, true))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 0))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40b720: radioRoadblockOf the cop eax; eax as the code leaves it. */
nfs::u32 radioRoadblock(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioRoadblockOf(app, ts, car); });
    return args.eax;
}

/* A message of the kind 0x1c (a ticket) or 0x1b (an arrest): the radio free at weight 3, the call
 * sign, the voice's reply and the description of the kind, the noise; the dispatcher's unit and
 * description where the radio is free at weight 2, else the voice's reply; the noise.  What the code
 * leaves in eax; nullopt when the game stopped. */
std::optional<u32> radioOfKindOf(App* app, ThreadState& ts, u32 car, u32 kind)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 3, 4) == 0)
        return 0;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, kind)
        || !noiseFor(app, ts, car, 1))
        return std::nullopt;
    if (radioFreeFor(app, 2, 4) != 0)
    {
        if (!unit(app, ts, car) || !describe(app, ts, 0, car, kind))
            return std::nullopt;
        put(app, kRadioQuiet, 0);
    }
    else if (!reply(app, ts, car, true))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 0))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40b860, 0x40b910: radioOfKindOf the cop eax, a ticket (0x1c) or an arrest (0x1b); eax as the code
 * leaves it. */
nfs::u32 radioOfKind(App* app, ThreadState& ts, nfs::Args& args, u32 kind)
{
    asThunk(ts, args, [&, car = args.eax] { return radioOfKindOf(app, ts, car, kind); });
    return args.eax;
}

// The messages' kinds: a ticket, an arrest.
constexpr u32 kTicketKind = 0x1c, kArrestKind = 0x1b;

/* 0x40b860's: radioOfKindOf the cop, a ticket. */
std::optional<u32> radioTicketOf(App* app, ThreadState& ts, u32 car)
{
    return radioOfKindOf(app, ts, car, kTicketKind);
}

/* 0x40b910's: radioOfKindOf the cop, an arrest. */
std::optional<u32> radioArrestOf(App* app, ThreadState& ts, u32 car)
{
    return radioOfKindOf(app, ts, car, kArrestKind);
}

/* 0x40b860: a ticket (kind 0x1c). */
nfs::u32 radioTicket(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioOfKind(app, ts, args, kTicketKind);
}

/* 0x40b910: an arrest (kind 0x1b). */
nfs::u32 radioArrest(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioOfKind(app, ts, args, kArrestKind);
}

/* The car stopped, `how` (the kinds 0x20, 0x1f -- said by the voice of the car -- and 0x21): the radio
 * free at weight 4, the call sign, the voice's reply, the description, the place, the noise; the radio
 * quiet 1000 frames and free, the dispatcher's phrase 0x10 and the description again, else the voice's
 * reply; the noise.  What the code leaves in eax and edx; nullopt when the game stopped. */
std::optional<StoppedLeft> radioStoppedOf(App* app, ThreadState& ts, u32 car, u32 how)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 4, 4) == 0)
        return StoppedLeft{0, 4};
    const u32 kind = how == 1 ? 0x1f : how == 0 ? 0x20 : 0x21;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, kind)
        || !locate(app, ts, 1, car) || !noiseFor(app, ts, car, 1))
        return std::nullopt;
    bool answered = false;
    if (s(at(app, kRadioQuiet)) > kRadioLongQuiet && radioFreeFor(app, 3, 1) != 0)
    {
        if (!dispatcher(app, ts, 0x10) || !describe(app, ts, 0, car, kind))
            return std::nullopt;
        put(app, kRadioQuiet, 0);
        answered = true;
    }
    if (!answered && !reply(app, ts, car, true))
        return std::nullopt;
    const std::optional<NoiseLeft> noise = noiseFor(app, ts, car, 0);
    if (!noise)
        return std::nullopt;
    return StoppedLeft{spokeLast(app, car), noise->edx};
}

/* 0x40b9c0: radioStoppedOf the car eax, how edx; eax as the code leaves it. */
nfs::u32 radioStopped(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto left = [&, car = args.eax, how = args.edx]() -> std::optional<u32> {
        const std::optional<StoppedLeft> stopped = radioStoppedOf(app, ts, car, how);
        if (!stopped)
            return std::nullopt;
        return stopped->eax;
    };
    asThunk(ts, args, left);
    return args.eax;
}

/* An ambulance called (kind 0x22): the radio free at weight 4, the call sign, the voice's reply, the
 * description, the place, the noise; the radio quiet 1000 frames and free, the dispatcher's phrase
 * 0x10, the description, the place now and then, else the voice's reply; the noise.  What the code
 * leaves in eax; nullopt when the game stopped. */
std::optional<u32> radioAmbulanceOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 4, 4) == 0)
        return 0;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, 0x22)
        || !locate(app, ts, 1, car) || !noiseFor(app, ts, car, 1))
        return std::nullopt;
    bool answered = false;
    if (s(at(app, kRadioQuiet)) > kRadioLongQuiet && radioFreeFor(app, 3, 1) != 0)
    {
        if (!dispatcher(app, ts, 0x10) || !describe(app, ts, 0, car, 0x22))
            return std::nullopt;
        const std::optional<u32> where = randomModulo(app, ts, 0x64);
        if (!where || (*where > 0x32 && !locate(app, ts, 0, car)))
            return std::nullopt;
        put(app, kRadioQuiet, 0);
        answered = true;
    }
    if ((!answered && !reply(app, ts, car, true)) || !noiseFor(app, ts, car, 0))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40bac0: radioAmbulanceOf the cop eax; eax as the code leaves it. */
nfs::u32 radioAmbulance(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioAmbulanceOf(app, ts, car); });
    return args.eax;
}

/* Spikes laid (kind 0x19, now and then 0x1a and the place too): the radio free at weight 4, the call
 * sign, the voice's reply, the noise.  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> radioSpikesOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 4, 4) == 0)
        return 0;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, 0x19))
        return std::nullopt;
    const std::optional<u32> more = randomModulo(app, ts, 0x64);
    if (!more || (*more > 0x32 && (!describe(app, ts, 1, car, 0x1a) || !locate(app, ts, 1, car))))
        return std::nullopt;
    if (!noiseFor(app, ts, car, 1))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40bba0: radioSpikesOf the cop eax; eax as the code leaves it. */
nfs::u32 radioSpikes(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return radioSpikesOf(app, ts, car); });
    return args.eax;
}

/* A cop's short message of the kind (the radio free at weight 3): the call sign, the voice's reply and
 * the description of the kind, the place where `place`, the noise.  What the code leaves in eax;
 * nullopt when the game stopped. */
std::optional<u32> radioShortOf(App* app, ThreadState& ts, u32 car, u32 kind, bool place)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (radioFreeFor(app, 3, 4) == 0)
        return 0;
    if (!callSign(app, ts, car) || !reply(app, ts, car, false) || !describe(app, ts, 1, car, kind)
        || (place && !locate(app, ts, 1, car)) || !noiseFor(app, ts, car, 1))
        return std::nullopt;
    return spokeLast(app, car);
}

/* 0x40bc30, 0x40bca0, 0x40bd00: radioShortOf the cop eax; eax as the code leaves it. */
nfs::u32 radioShort(App* app, ThreadState& ts, nfs::Args& args, u32 kind, bool place)
{
    asThunk(ts, args, [&, car = args.eax] { return radioShortOf(app, ts, car, kind, place); });
    return args.eax;
}

// The short messages' kinds: a call for help (with the place), the pursuit resumed, called off.
constexpr u32 kHelpKind = 0x23, kResumeKind = 0x1d, kAbortKind = 0x1e;

/* 0x40bc30's: radioShortOf the cop, a call for help with the place. */
std::optional<u32> radioHelpOf(App* app, ThreadState& ts, u32 car)
{
    return radioShortOf(app, ts, car, kHelpKind, true);
}

/* 0x40bca0's: radioShortOf the cop, the pursuit resumed. */
std::optional<u32> radioResumeOf(App* app, ThreadState& ts, u32 car)
{
    return radioShortOf(app, ts, car, kResumeKind, false);
}

/* 0x40bd00's: radioShortOf the cop, the pursuit called off. */
std::optional<u32> radioAbortOf(App* app, ThreadState& ts, u32 car)
{
    return radioShortOf(app, ts, car, kAbortKind, false);
}

/* 0x40bc30: a call for help (kind 0x23, with the place). */
nfs::u32 radioHelp(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioShort(app, ts, args, kHelpKind, true);
}

/* 0x40bca0: the pursuit resumed (kind 0x1d). */
nfs::u32 radioResume(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioShort(app, ts, args, kResumeKind, false);
}

/* 0x40bd00: the pursuit called off (kind 0x1e). */
nfs::u32 radioAbort(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return radioShort(app, ts, args, kAbortKind, false);
}

// resetOnTrack's: how far it puts a car at most; the height a car is put back at (a float), its scale
// in a tunnel (a float), the heading's scale (a double); a car's place (+0x98..+0xa0, +0x9c its height),
// its speed's vector (+0xa4..+0xac), and the frames of its settling (+0x174, +0x524).
constexpr i32 kFarthestReset = 0xc8;
constexpr u32 kResetHeight = 0x5efdd0, kTunnelHeight = 0x53668c, kHeadingScale = 0x536694;
constexpr u32 kCarPlace = 0x98, kCarHeight = 0x9c, kCarPlaceZ = 0xa0, kCarVelocity = 0xa4, kCarResetting = 0x184,
                kCarSettleFrames = 0x174, kCarSettleFramesToo = 0x524;
// The node's heading's bytes (the top byte of the dword at +0x11, +0x12, +0x13).
constexpr u32 kNodeHeading = 0x11;
// Whether the race is under way.
constexpr u32 kRaceUnderWay = 0x7d3680;

/* The network's car put back on the track by the radar's word in the record (radarFactorOf's: how far
 * behind, cut to -200 .. 200 there): onto the node that far on (0x428d60, wrapped round the track's
 * nodes), raised by [0x5efdd0] (by [0x53668c] more in a tunnel, 0x420bf0), its speed the radar's (+8,
 * ahead of a human's car) or the computer's at the node (0x40e080) along the node's heading bytes (by
 * [0x536694]); settled (0x428970), its frames +0x174 and +0x524, and ticketed (0x4c7fd0) -- in a race
 * under way where the car is not being put back already (+0x184).  What the code leaves in eax; nullopt
 * when the game stopped. */
std::optional<u32> resetOnTrackOf(App* app, ThreadState& ts, u32 car, u32 record)
{
    const GuestFrame frame(ts, 0x14, 0x14);
    if (at(app, kRaceUnderWay) == 0 || at(app, car + kCarResetting) != 0)
        return car;
    const auto ahead = [&] { return i32(s16(half(app, record + kRadarAhead))); };
    const auto setAhead = [&](i32 value) { app->getMemory<u16>(record + kRadarAhead) = u16(value); };
    const i32 far = ahead() <= 0 ? -ahead() : ahead();
    if (far >= kFarthestReset)
    {
        setAhead(ahead() > kFarthestReset ? kFarthestReset : ahead());
        setAhead(ahead() > -kFarthestReset ? ahead() : -kFarthestReset);
    }
    const i32 by = ahead();
    const u32 count = at(app, kNodeCount);
    i32 target = s(at(app, car + kCarNode) - u32(by));
    if (-by >= 0)
    {
        if (target >= s(count))
            target -= s(count);
    }
    else if (target < 0)
        target += s(count);
    if (!feCore<0x428d60, frontend::loada::carPlaceOnRoad>(app, ts, car, u32(target)))
        return std::nullopt;
    float height = f32At(app, kResetHeight);
    // eax at the call as the code leaves it there, the last word it pushes through it (+0xa0).
    const std::optional<u32> tunnel = callPushing(
        ts, {at(app, car + kCarNode), at(app, car + kCarPlaceZ), at(app, car + kCarHeight), at(app, car + kCarPlace)}, app,
        [&] {
            const std::optional<u32> in = guest::direct<0x420bf0, trackdata::placeInTunnel>(
                app, ts, ts.sp, at(app, ts.sp + 12));
            return ts.terminate ? std::optional<u32>() : in;
        });
    if (!tunnel)
        return std::nullopt;
    if (*tunnel != 0)
        height = float(fp::mul(Float(height), f32(app, kTunnelHeight)));
    putFloat(app, car + kCarHeight, float(fp::add(f32(app, car + kCarHeight), Float(height))));
    float speed;
    if (s16(half(app, record + kRadarAhead)) > 0 && s(gameSettings(app).humanCount) > s(at(app, car + kCarIndex)))
        speed = f32At(app, record + kRadarSpeed);
    else
    {
        speed = guest::direct<0x40e080, aiplan::targetSpeed>(app, ts, at(app, car + kCarNode));
        if (ts.terminate)
            return std::nullopt;
    }
    // The node's heading byte, signed, times 512.
    const u32 nodes = at(app, kNodes);
    const auto heading = [&](u32 k) {
        const u32 bits = u32(s(at(app, nodes + at(app, car + kCarNode) * kNodeSize + kNodeHeading + k)) >> 0x18) << 9;
        return Float(s(bits));
    };
    const Float scale = f64(app, kHeadingScale);
    putFloat(app, car + kCarVelocity, float(fp::mul(fp::mul(heading(0), scale), Float(speed))));
    putFloat(app, car + kCarVelocity + 4, float(fp::mul(fp::mul(heading(1), scale), Float(speed))));
    putFloat(app, car + kCarVelocity + 8, float(fp::mul(fp::mul(scale, heading(2)), Float(speed))));
    if (!feCore<0x428970, frontend::loada::carReset>(app, ts, car))
        return std::nullopt;
    put(app, car + kCarSettleFrames, 0x40);
    put(app, car + kCarSettleFramesToo, 0x20);
    const std::optional<u32> ticketed =
        feCore<0x4c7fd0, frontend::neta::clearRing>(app, ts, at(app, car + kCarIndex));
    if (!ticketed)
        return std::nullopt;
    return *ticketed;
}

/* 0x40bd80: resetOnTrackOf the car eax by the radar's record edx; eax as the code leaves it. */
nfs::u32 resetOnTrack(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax, record = args.edx] { return resetOnTrackOf(app, ts, car, record); });
    return args.eax;
}

// The chases' scores cleared at a cop's start (20 bytes); its marks cleared then, as a word at +0x620:
// 0x80 of +0x620 and 0x20 of +0x621.
constexpr u32 kStartScores = 0x56ea58, kStartScoresSize = 0x14;
constexpr u16 kRestartMarks = 0xdf7f;

/* The cop started again, while it is active (0x40df10): the chases' scores at 0x56ea58 cleared
 * (memset), its marks of being chased and of the radio cleared, its state 1, then steered, its target
 * speed, the scores, driven.  What the code leaves in eax; nullopt when the game stopped. */
std::optional<u32> copRestartOf(App* app, ThreadState& ts, u32 cop)
{
    const GuestFrame frame(ts, 0x10, 0);
    const std::optional<u32> active = guest::direct<0x40df10, aiplan::stepLoading>(app, ts, cop);
    if (!active)
        return std::nullopt;
    if (*active == 0)
        return 0;
    guest::direct<0x4e0640, watcom::clib::memset>(app, ts, kStartScores, 0, kStartScoresSize);
    if (ts.terminate)
        return std::nullopt;
    app->getMemory<u16>(cop + kCarPursuitFlags) = u16(half(app, cop + kCarPursuitFlags) & kRestartMarks);
    if (at(app, cop + kCarCopState) != kCopChasing)
        put(app, cop + kCarCopState, kCopChasing);
    if (!guest::direct<0x40dd60, aiplan::beginPlanning>(app, ts, cop))
        return std::nullopt;
    {
        const CallSlot slot(ts);
        if (!targetSpeedOf(app, ts, cop))
            return std::nullopt;
    }
    scoreSpeedsOf(app, cop);
    for (const auto score : {scoreChasesOf, scoreLaneOf})
    {
        const CallSlot slot(ts);
        if (!score(app, ts, cop))
            return std::nullopt;
    }
    if (!guest::direct<0x40ddb0, aiplan::pickLane>(app, ts, cop))
        return std::nullopt;
    return feCore<0x40de30, frontend::sima::laneSteer>(app, ts, cop);
}

/* 0x40bf50: copRestartOf the cop eax; eax as the code leaves it. */
nfs::u32 copRestart(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, cop = args.eax] { return copRestartOf(app, ts, cop); });
    return args.eax;
}

// The traffic's top speed (a double).
constexpr u32 kTrafficTop = 0x53669c;

/* The traffic's top speed. */
Float trafficTopSpeed(App* app)
{
    return f64(app, kTrafficTop);
}

/* 0x40bfe0: trafficTopSpeed onto the x87's stack. */
nfs::u32 topSpeedFactor(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(trafficTopSpeed(app));
    ts.sp += 4;
    return args.eax;
}

// followChasedCar's: the target's scale going the other way, the level's scale and offset by the chased
// car's speed, the chased car's speed past which it is kept to, the share's scale (doubles).
constexpr u32 kOppositeScale = 0x5366a4, kLevelScale = 0x5366ac, kLevelOffset = 0x5366b4, kKeptPast = 0x5366bc,
                kShareScale = 0x5366c4;

/* The car's target speed [+0x7b0] by the chased car's [0x56ef3c]: the two going opposite ways, times
 * [0x5366a4]; else, past the chased car's speed, kept to it -- the chased car's speed less or more its
 * share (level - [0x56ef48]) * [0x5366c4] / level, the level its speed times [0x5366ac] + [0x5366b4],
 * no less than 4 -- where the level reaches the chase's and the chased car is past [0x5366bc].  Gives
 * what the code leaves in eax. */
u32 followChasedCar(App* app, u32 car)
{
    const u32 chased = at(app, kChasedCar);
    if (chased == 0)
        return car;
    const bool carBack = isAbove(0.0, f32(app, car + kCarSpeed));
    const bool chasedBack = isAbove(0.0, f32(app, chased + kCarSpeed));
    if (carBack != chasedBack)
    {
        putFloat(app, car + kCarTarget, float(fp::mul(f32(app, car + kCarTarget), f64(app, kOppositeScale))));
        return 1;
    }
    const Float target = f32(app, car + kCarTarget);
    const bool forwards = isBelow(0.0, target);
    const u32 left = forwards ? floatBits(float(target)) : fp::compare(0.0, target);
    const float size = forwards ? float(target) : float(-target);
    const Float gap = fp::sub(Float(size), f32(app, chased + kCarSpeedAbs));
    if (isAboveEqual(0.0, gap))
        return withStatus(left, 0.0, gap);
    u32 level = u32(fp::toInteger<i32>(
        fp::add(fp::mul(f32(app, chased + kCarSpeed), f64(app, kLevelScale)), f64(app, kLevelOffset))));
    if (!(s(level) > 0))
        level = 0 - level;
    if (s(level) < 4)
        level = 4;
    if (s(level) < s(at(app, kChaseLevel)))
        return level;
    const Float chasedSize = f32(app, chased + kCarSpeedAbs);
    if (isBelow(chasedSize, f64(app, kKeptPast)))
        return withStatus(level, chasedSize, f64(app, kKeptPast));
    const u32 over = level - at(app, kChaseLevel);
    const float share = float(fp::div(fp::mul(Float(s(over)), f64(app, kShareScale)), Float(s(level))));
    const Float now = f32(app, car + kCarTarget);
    const bool down = isBelow(0.0, now);
    const Float speed = f32(app, chased + kCarSpeed);
    putFloat(app, car + kCarTarget, float(down ? fp::sub(speed, Float(share)) : fp::add(speed, Float(share))));
    return withStatus(over, 0.0, now);
}

/* 0x40bff0: followChasedCar for the car eax, eax as the code leaves it. */
nfs::u32 followChased(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return followChasedCar(app, car); });
    return args.eax;
}

// The speed of the car followed under which a car going backwards follows it backwards (a double).
constexpr u32 kFollowSlow = 0x5366cc;
// How far on the first node is looked for, and how much further each time (times the way).
constexpr i32 kFollowFirst = 0x3d, kFollowStep = 0x14;

/* The node of the car set to the first free one (0x405c80) from the car it follows [+0x6d8] (0x40e160),
 * 61 nodes on and 20 more each time -- backwards where the car followed goes backwards and the car is
 * not against the traffic, or past [0x5366cc] where it is; backwards too where the car followed goes
 * forwards no faster than that, the car with the traffic.  nullopt when the game stopped; else the
 * code leaves 0 in eax. */
std::optional<u32> followCarOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0);
    const u32 followed = at(app, car + kCarFollowed);
    i32 way = 1;
    if (followed != 0)
    {
        const bool against = (byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0;
        if (at(app, followed + kCarBackwards) != 0)
            way = !against || isAbove(f32(app, followed + kCarSpeedAbs), f64(app, kFollowSlow)) ? -1 : 1;
        else if (against)
            way = 1;
        else
            way = isBelowEqual(f32(app, followed + kCarSpeedAbs), f64(app, kFollowSlow)) ? -1 : 1;
    }
    u32 on = u32(way * kFollowFirst);
    u32 ahead;
    for (;;)
    {
        ahead = guest::direct<0x40e160, aiplan::nodeAhead>(app, ts, at(app, car + kCarFollowed), on);
        if (ts.terminate)
            return std::nullopt;
        on += u32(way * kFollowStep);
        const std::optional<u32> taken = guest::direct<0x405c80, aidrive::humanNearSliceOf>(app, ts, ahead);
        if (!taken)
            return std::nullopt;
        if (*taken == 0)
            break;
    }
    put(app, car + kCarNode, ahead);
    return 0;
}

/* 0x40c160: followCarOf the car eax; eax as the code leaves it. */
nfs::u32 followCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return followCarOf(app, ts, car); });
    return args.eax;
}

// The race's frame from which the countdowns run; the cars still in, the cars out.
constexpr i32 kCountdownFrom = 0x780;
constexpr u32 kCarsIn = 0x5efd8c, kCarsOut = 0x56ebc0;

/* Whether the car's countdown [+0x650] runs out this frame (past 1920 frames) with fewer places left
 * (0x40ef10) than the cars still in less those out; a countdown not begun set to 2.  nullopt when the
 * game stopped. */
std::optional<bool> countdownDoneOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);
    if (s(at(app, kRaceFrame)) < kCountdownFrom)
        return false;
    const u32 count = at(app, car + kCarCountdown);
    if (s(count) <= 0)
    {
        put(app, car + kCarCountdown, 2);
        return false;
    }
    put(app, car + kCarCountdown, count - 1);
    if (count - 1 != 0)
        return false;
    const u32 still = at(app, kCarsIn) - at(app, kCarsOut);
    const u32 places = guest::direct<0x40ef10, aiplan::trackTuningFirst>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return s(still) < s(places);
}

/* 0x40c210: countdownDoneOf the car eax, 1 or 0 in eax. */
nfs::u32 countdownDone(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return countdownDoneOf(app, ts, car); });
    return args.eax;
}

/* The traffic's car's target speed: its node's (0x40ed40) + 1, no more than the traffic's top speed,
 * signed as it goes.  What the code leaves in eax: the top's bits where it cut the target, else the car
 * with the compare's status word in ax -- its ah the flipped byte where the track runs backwards;
 * nullopt when the game stopped. */
std::optional<u32> trafficTargetOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0xc, 4);
    const float speed = guest::direct<0x40ed40, aiplan::trafficSpeed>(app, ts, at(app, car + kCarNode));
    if (ts.terminate)
        return std::nullopt;
    putFloat(app, car + kCarTarget, float(fp::add(Float(speed), 1.0)));
    const float top = float(trafficTopSpeed(app));
    const Float target = f32(app, car + kCarTarget);
    u32 left = withStatus(car, target, top);
    if (isAbove(target, top))
    {
        putFloat(app, car + kCarTarget, top);
        left = floatBits(top);
    }
    if (gameSettings(app).backwards != 0)
    {
        flipTarget(app, car);
        left = (left & 0xffff00ffu) | u32(byte(app, car + kCarTargetSign)) << 8;
    }
    if ((byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0)
        flipTarget(app, car);
    return left;
}

/* 0x40c270: trafficTargetOf the car eax; eax as the code leaves it. */
nfs::u32 trafficTarget(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return trafficTargetOf(app, ts, car); });
    return args.eax;
}

// The traffic's random numbers: the state, the last full product, the multiplier.
constexpr u32 kTrafficRandom = 0x554f10, kTrafficRandomFull = 0x554f14, kTrafficRandomStep = 0x554f18;
// The way the traffic drives on this track (1 or -1), the offsets a car is given across its lane (four
// floats), the width's scale and the place's offset across the lane (doubles).
constexpr u32 kTrafficWay = 0x56f230, kTrafficJitter = 0x552548, kLaneScale = 0x5366d4, kLaneOffset = 0x5366dc;

/* The traffic's random number stepped: the product of the state and the multiplier, its low half the
 * new state. */
u32 stepTrafficRandom(App* app)
{
    const u32 product = at(app, kTrafficRandom) * at(app, kTrafficRandomStep);
    put(app, kTrafficRandom, product & 0xffff);
    return product;
}

/* The traffic's car's step: its place across the road [+0x7b4] picked by the traffic's random numbers
 * within its lane's width at its node -- on the left or the right by which way it goes, the track's way
 * and [0x56f230] -- plus one of the four offsets of 0x552548 [+0x654]; its target speed (trafficTargetOf),
 * its speed [+0x7c0] and the speed's size [+0x7c4]; then driven (0x40cd80).  What the code leaves in eax;
 * nullopt when the game stopped. */
std::optional<u32> trafficStepOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0xc);
    const u32 index = at(app, car + kCarNode);
    const bool against = (byte(app, car + kCarPursuitFlagsHigh) & kFlagAgainst) != 0;
    const u32 backwards = gameSettings(app).backwards;
    const bool left = (((against ? 0u : 1u) ^ backwards) != 0 && at(app, kTrafficWay) == 1)
                      || (((against ? 1u : 0u) ^ backwards) != 0 && at(app, kTrafficWay) == 0xffffffff);
    const u32 n = nodeRecord(app, index);
    // The place across the lane: (the random's middle bytes times the lanes) / 2^16 + 1 widths of the
    // road's side (its width times [0x5366d4]), less (on the left) or more (on the right) [0x5366dc]
    // of a width.
    const u32 product = stepTrafficRandom(app);
    const u32 lanes = left ? byte(app, n + kNodeLanes) & 0xf : (byte(app, n + kNodeLanes) >> 4) & 0xf;
    const Float width =
        fp::mul(Float(s(u32(byte(app, n + (left ? kNodeRightWidth : kNodeLeftWidth))) << 15)), f64(app, kLaneScale));
    const Float lane = Float(s(((product & 0xffff00) >> 8) * lanes >> 0x10) + 1);
    const Float across = left ? fp::sub(fp::mul(lane, width), fp::mul(width, f64(app, kLaneOffset)))
                              : fp::add(fp::mul(-width, lane), fp::mul(width, f64(app, kLaneOffset)));
    putFloat(app, car + kCarAcross, float(across));
    const u32 jitterProduct = stepTrafficRandom(app);
    const u32 jitter = ((jitterProduct & 0xffff00) >> 8 << 2) >> 0x10;
    put(app, kTrafficRandomFull, jitterProduct);
    put(app, car + kCarJitter, at(app, jitter * 4 + kTrafficJitter));
    putFloat(app, car + kCarAcross, float(fp::add(f32(app, car + kCarJitter), f32(app, car + kCarAcross))));
    {
        const CallSlot slot(ts);
        if (!trafficTargetOf(app, ts, car))
            return std::nullopt;
    }
    const u32 full = at(app, kTrafficRandomFull);
    putFloat(app, car + kCarSpeed, float(f32(app, car + kCarTarget)));
    const Float speed = f32(app, car + kCarSpeed);
    if (isBelow(0.0, speed))
        put(app, car + kCarSpeedAbs, at(app, car + kCarSpeed));
    else
        putFloat(app, car + kCarSpeedAbs, float(-speed));
    put(app, kTrafficRandomFull, full);
    if (!guest::direct<0x40cd80, aiplan::putOnTrack>(app, ts, car))
        return std::nullopt;
    return car;  // (the car, as 0x40cd80 leaves it)
}

/* 0x40c2f0: trafficStepOf the car eax; eax as the code leaves it. */
nfs::u32 trafficStep(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    asThunk(ts, args, [&, car = args.eax] { return trafficStepOf(app, ts, car); });
    return args.eax;
}

}  // namespace pursuit
}  // namespace nfs3hp
