#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_controls_race.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend_eaa.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rende.h"
#include "native_frontend_simb.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_sime.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "port_conv.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

/* A car's race state between the frames' physics: put back on the road when
 * it is stuck or the player asks (respawnCar, resetCar, stuckCheck), its
 * wheels on the track (wheelCells, wheelSurface), their skid marks, dust and
 * sounds (wheelEffects and what it calls), the camera's shake (startShake,
 * updateShake), a crash's jolt, sounds and effects (crashEffects), and the
 * grid: the start's places, from TRKnnn\start.dat or worked out, and a car
 * put on its place with its race state cleared (loadStartGrid, placeOnGrid).
 * The game's code calls each by the name it has here.
 *
 * The car is the record at eax on entry (0x9a4 bytes and more); what these
 * functions read of it, as their code uses it: +0x1c its slice of the track,
 * +0x98 its place, +0xa4 its speed's vector and +0xb8 its speed, +0xc0 its
 * axes, +0xe4..+0xec its spin, +0x160 the track's index into the surfaces'
 * table, +0x164 the race's stage (0 racing), +0x168 how hard it last hit,
 * +0x170 the hit's kind, +0x184 a slice to be put back on, +0x18c its
 * surface, +0x200 its flags (1 a player's, 4 the player's, 0x10 a wreck),
 * +0x300 four wheels of 0x78 bytes (their cells on the track: +0x4 the place,
 * +0x10 the slice, +0x14 and +0x18 on a piece of track, +0x1c its heights),
 * +0x528 its handling, +0x8a0 the stuck count, +0x8c4..+0x8d0 the shake's
 * two waves, +0x8d4 the frames since it was put back, +0x920..+0x930 the
 * shake, +0x94c four wheels' skids, +0x95c..+0x996 their marks and sounds,
 * +0x99c what the wheels hit.  The surfaces' table at 0x553080 is the
 * game's own: 0x240 bytes a track, 0x24 a surface (kSurfaces on).
 *
 * Each is a typed core (respawnCarOf, wheelSlipOf...) with a thunk under the game's name for
 * the callers on registers and the dispatch: it leaves eax (and edx, ebx where the code does not
 * keep them) and the x87's stack as the game's code does -- a compare's status word among them,
 * which is why a core returns what the code leaves (Left) besides what it works out.  A core
 * keeps the registers the code saves (Kept), lays a frame on the guest's stack at the code's
 * depth where a callee is handed an address in it (GuestFrame, Calling for the cores it calls),
 * calls the other typed files' functions as C++ (guest::direct, their headers) but where it reads
 * what one leaves in a register, and every other function of the game's by address, its arguments
 * in registers and on the guest's stack as the code hands them.  The x87's operations are fp's, in the code's order, a
 * float kept where the code stores one; the game's constants (floats and doubles in its data,
 * 0x5371d0 to 0x537310) are read where the code reads them. */
namespace nfs3hp
{
namespace racestart
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
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

// The race's.
constexpr u32 kTrackFolder = 0x7a27b4;    // the tracks' folder, a string
constexpr u32 kStartFile = 0x537318;      // "%sTRK%03x\start.dat"
// The track's slices: their count, their records (0x24 bytes each).
constexpr u32 kSliceCount = 0x5dd940, kSlices = 0x5dd948;
// Putting a car back: slices on, the height above the road, whether a car may be.
constexpr u32 kRespawnStep = 0x5efdcc, kRespawnHeight = 0x5efdd0, kResetAllowed = 0x7d3680;
// The stuck count's start, and the player's with kPlayerOption.
constexpr u32 kStuckCount = 0x5efdc0, kPlayerStuckCount = 0x5efdc4;
// The shake's tuning, across and up; the spin in the air's (by the axes, by the steering).
constexpr u32 kShakeAcross = 0x5efde0, kShakeUp = 0x5efde4;
constexpr u32 kAirSpinZ = 0x5efda8, kAirSteerZ = 0x5efdac, kAirSpinX = 0x5efdb0, kAirSteerX = 0x5efdb4;
// The handling's scale on the grid: a player's, the human players'.
constexpr u32 kHumanScale = 0x5efdb8, kPlayerScale = 0x5efdbc;
// A surface's record, 0x24 bytes, by the track (0x240) and the surface: the
// skid past which its effect shows, the speed its skid starts at and the
// speed over which it grows, its effect, whether it marks, its rumble, its
// sound, the back wheels' sound.
constexpr u32 kSurfaces = 0x553080, kSurfaceSkidFrom = 0x553084, kSurfaceSkidOver = 0x553088;
constexpr u32 kSurfaceEffect = 0x55308c, kSurfaceMarks = 0x553090, kSurfaceRumble = 0x553098;
constexpr u32 kSurfaceSound = 0x55309c, kSurfaceRearSound = 0x5530a0;
// A crash's, by the other's kind (+0x170's low byte): the effect (3: by the
// speed), the sound; by its own kind (the high word): the channel; the speed
// past which its effect plays.
constexpr u32 kCrashEffects = 0x553740, kCrashSounds = 0x553760, kCrashChannels = 0x553780;
constexpr u32 kCrashSpeed = 0x5537b8;
// The grid: 12 places of a slice and a place (3 words); whether mirrored; the
// cars' places on it by their index (+0x1f8); the cars and the others on it.
constexpr u32 kGrid = 0x5ef9b8, kGridY = 0x5ef9bc, kGridZ = 0x5ef9c0;
constexpr u32 kGridMirrored = 0x55d000;

// A car's: the slice it is to be put back on (0 none), the frames since it was.
constexpr u32 kCarRespawnSlice = 0x184, kCarSinceRespawn = 0x8d4;
// A car's: its speed, the track's index into the surfaces' table, its handling (+0x298 its
// rumble), its rumble.
constexpr u32 kCarSpeed = 0xb8, kCarTrackSurfaces = 0x160, kCarHandling = 0x528, kCarRumble = 0x930;
constexpr u32 kHandlingRumble = 0x298;
constexpr u32 kTrackSurfacesSize = 0x240, kSurfaceSize = 0x24;
// A 64th (a float), a half (a double); 1.0's bits.
constexpr u32 kSixtyFourth = 0x5371e0, kHalfTurn = 0x5371e8, kOneBits = 0x3f800000;
// A car's: how far it leans (+0x10c), its shake's two waves (an amplitude, a float, and the
// frames left, each).
constexpr u32 kCarLean = 0x10c, kCarWaves = 0x8c4, kWaveSize = 8, kWaveFrames = 4;
// The lean under which a car shakes a little (a double, -0.9); the little shake's amplitude (a
// 240th, a float's bits); its frames, 24 and up to 11 more.
constexpr u32 kIdleLean = 0x537200, kIdleAmplitude = 0x3b888889;
constexpr i32 kIdleFrames = 0x18, kIdleFramesSpread = 0xc;
// A twelfth (a float): the shake's wave's step a frame, and its fall.
constexpr u32 kTwelfth = 0x537208;
// A car's: its flags (1 a player's, 4 the player's at this machine, 0x10 a wreck, 0x20 chased),
// its state (3 finished, 4 out), the flags of 0x621 (0x40 hit an object), what it hit.
constexpr u32 kCarFlags = 0x200, kCarState = 0x2c4, kCarObjectFlags = 0x621, kCarHits = 0x99c;
constexpr u8 kPlayersCar = 1, kThisMachines = 4, kWreck = 0x10, kChased = 0x20;
constexpr u32 kFinished = 3, kOut = 4;
constexpr u8 kHitObject = 0x40;
// The object handed over (the hit's flags left in edx); the car told of the hit (the handle in
// edx).
constexpr u32 kObjectHit = 0x4c9f50, kCarHitObject = 0x401650;
// A car's: the race's stage (a word, 0 racing), its surface, its four wheels' cells on the track
// (0x78 bytes each: +0x14 and +0x18 whether on a piece of its own).
constexpr u32 kCarStage = 0x164, kCarSurface = 0x18c, kCarWheels = 0x300, kWheelSize = 0x78;
constexpr u32 kWheelPiece = 0x14, kWheelPieceToo = 0x18;
// The kind of polygon a place is on (trackdata::placeSurface): its low 6 bits the surface.
constexpr u32 kSurfaceBits = 0x3f;
// A car's: its first word (its number, as the marks take it), its wheels' marks (a byte each,
// 1 drawing) and their sounds' (a byte each), its three scrape sounds' (bytes), its shake (five
// words from +0x920; +0x91c besides).
constexpr u32 kCarNumber = 0, kCarMarks = 0x95c, kCarMarkSounds = 0x990, kCarScrapes = 0x994;
constexpr u32 kCarShake = 0x920, kCarShakeToo = 0x91c;
// A wheel's mark cut (eax the car's number, edx the wheel); an event added to a car (eax the car,
// edx, ebx, ecx and three words: its fields, carEvent).
constexpr u32 kCutMark = 0x4db860, kCarEvent = 0x4284e0;
// The grid's place's up (16.16).
constexpr u32 kGridUp = 0x8000;

// A car's: its cell on the track (as a wheel's: +0x10 its slice, +0x14 and +0x18 on a piece of its
// own), its slice, its place (x, y up, z), its velocity, the frames it settles for, its spin,
// the vector +0x130 (the ground's up under it, as stuckCheck's dot reads it), its axes' up, how
// far over the road (+0x15c), the times it was put back, the reset's wait, its pursuer (Hot
// Pursuit), its stuck count, the throttle asked (a byte).
constexpr u32 kCarCell = 0xc, kCarSlice = 0x1c, kCarPlace = 0x98, kCarVelocity = 0xa4, kCarSettleFrames = 0x174;
constexpr u32 kCarSpin = 0xe4, kCarGroundUp = 0x130, kCarUp = 0xcc, kCarAboveRoad = 0x15c, kCarRespawns = 0x2c0;
constexpr u32 kCarResetWait = 0x524, kCarPursuer = 0x624, kCarStuck = 0x8a0, kCarThrottleAsked = 0x504;
constexpr u32 kHotPursuit = 3;
// Whether Hot Pursuit has a car out.
constexpr u32 kPursuitOut = 0x401500;
// The height in a tunnel by these (floats): a car put back, reset.
constexpr u32 kHalfHeight = 0x5371d0, kResetHeight = 0x5371d4;
// A stuck car's spin's cut (a 64th), the upright its ground's up says (0.75); a dot product (eax,
// edx), onto the x87's stack.
constexpr u32 kStuckSpinCut = 0x5371d8, kUpright = 0x5371dc, kDot = 0x4e01f0;

// The grid's places; a slice's record (0x24 bytes): its lanes (the low 4 bits of +0x13), its
// half widths across, one side's (+0x17) and the other's (+0x1b), bytes as the grid reads them.
constexpr u32 kGridPlaces = 0xc, kSliceSize = 0x24, kSliceLanes = 0x13, kSliceLeftWidth = 0x17, kSliceRightWidth = 0x1b;
// The runtime's sprintf (cdecl); a file opened (eax the path, edx 0, ebx 1), a word read from it,
// closed.

// A car's: its axes' forward, its slides (whole), front and back, its wheels' skids (floats),
// their marks' last points (3 words each).  A wheel's cell's: its place, its slice, its heights
// (a pointer to 16-bit words).  A slice's heights (signed bytes, from +0x10).
constexpr u32 kCarForward = 0xd8, kCarFrontSlide = 0x598, kCarBackSlide = 0x594, kCarSkids = 0x94c;
constexpr u32 kCarLastPoints = 0x960, kWheelPlace = 4, kWheelSlice = 0x10, kWheelHeights = 0x1c;
constexpr u32 kSliceHeights = 0x10;
// A mark's point drawn; a wheel's smoke (the place and the kind on the stack: 6).
constexpr u32 kMarkPoint = 0x4db940, kWheelSmoke = 0x4cb010;
constexpr u32 kSmoke = 6;
// The slip's (floats): its base, its share of the spin; the skid past which a back wheel smokes
// (a double, 5).
constexpr u32 kSlipBase = 0x53720c, kSlipBySpin = 0x537210, kSmokingSkid = 0x537270;

// A car's: its axes (+0xc0 three vectors: across, up, forward), its half sizes (across, down,
// along).  The wheels' corners' shares of them (doubles: across 0.75, along a half); a 32nd (a
// float) of the velocity ahead; 16.16's one (a double).
constexpr u32 kCarAxes = 0xc0, kCarHalfSizes = 0x114;
constexpr u32 kWheelsAcross = 0x537218, kWheelsAlong = 0x537220, kThirtySecond = 0x537228, kToFixed = 0x537230;

// A car's: in the air (a byte), how hard it hit (a float, 0 none), its spin as its axes see it
// (three floats), its turn (+0x4e0 and +0x4e8, as the air's steering and the shake take them).  Its
// handling's shake (+0x294).
constexpr u32 kCarInAir = 0x8b, kCarHit = 0x168, kCarSpinSeen = 0x4f8, kCarTurn = 0x4e0;
constexpr u32 kHandlingShake = 0x294;
// A hit's share of the shake (a 45th, a float), the most of it (a double, 1.5).
constexpr u32 kHitScale = 0x5371f0, kMostShake = 0x5371f8;

// Half (a float) of two wheels' skids; the most a wheel skids (a double, 5); the speeds (doubles)
// past which a wheel that hit something throws dust (5), more (20), sparks (10); 16.16's
// fraction (a double); the sparks' share of the speed (a double); the sparks (stdcall: the place
// and the speed, eax the car).
constexpr u32 kSkidHalf = 0x537238, kMostSkid = 0x537268;
constexpr u32 kDustSpeed = 0x537240, kMoreDustSpeed = 0x537248, kSparksSpeed = 0x537250, kFromFixed = 0x537258;
constexpr u32 kSparksBySpeed = 0x537260, kSparks = 0x4cac20;

// A car's: the frames since what its wheels hit was kept, its wheels' slips (front, back).  The
// dust a wheel that hit throws (13); the lean under which the wheels do not skid (a double); the
// front slip's share of the skid, the most skid (doubles, 10).
constexpr u32 kCarHitFrames = 0x9a0, kCarFrontSlip = 0x5c0, kCarBackSlip = 0x5c4, kHitDust = 0xd;
constexpr u32 kSkidLean = 0x537278, kSkidBySlip = 0x537280, kMostWheelSkid = 0x537288;

// A car's: how high (+0x84), its driver's record (+0x50 its share of the rumble).  The shake's:
// gravity on the axes (a float), what its springs keep a frame, the pulls past which they move
// (across, up), the pulls' share (a float), the rumble's half, its shares across and up
// (doubles).
constexpr u32 kCarAltitude = 0x84, kCarDriver = 0x220, kDriverShake = 0x50;
constexpr u32 kShakeGravity = 0x537290, kSpringKeep = 0x537298, kAcrossPullFrom = 0x5372a0, kPullShare = 0x5372a8;
constexpr u32 kUpPullFrom = 0x5372b0, kRumbleHalf = 0x5372b8, kRumbleAcross = 0x5372c0, kRumbleUp = 0x5372c8;

// A car's: where it sparkles when put back (a vector in its axes); its driver's +0x14 (0 to 49,
// as 0x4396e0 takes it).  The sparkle (eax the car), the burst (eax its kind) -- each stdcall, a
// place's three floats; the spin under which a car put back has settled (a double).
constexpr u32 kCarRespawnOffset = 0x8d8, kDriverIndex = 0x14;
constexpr u32 kSparkle = 0x4caea0, kSparkleBurst = 0x4cae00, kSettledSpin = 0x537310;

// A car's: its slice at the start (+4), its index among the race's (its grid slot's), its lap's
// state, whether it runs backwards, its stuck record (+0x28 its three start words; 0x104 bytes
// cleared at the start but the pointer and the 12 bytes after it), its lights (1 on, 2 the
// player's).  Its driver's three start words (+0x5c); the handling's scaled (+4).  The race's
// flag for the start records (2).
constexpr u32 kCarStartSlice = 4, kCarIndex = 0x1f8, kCarLapState = 0x204, kCarBackwards = 0x208;
constexpr u32 kCarStuckRecord = 0x8a4, kRecordStart = 0x28, kCarLights = 0x660, kDriverStart = 0x5c;
constexpr u32 kHandlingScaled = 4;
constexpr std::uint8_t kStartRecords = 2;
// The handling scaled (eax the car, edx its number; stdcall the scale and three words); a fill with
// zeros (eax where, edx how many).
constexpr u32 kScaleHandling = 0x4a0d20;
constexpr u32 kFill = 0x4e070c;

// A car's: out of the race and in it (bytes: it races with +0x88 clear and +0x89 set), its crash's
// kind (+0x170, with +0x168 and +0x16c cleared each frame).
constexpr u32 kCarOut = 0x88, kCarIn = 0x89, kCarCrash = 0x170;

// A car's: where its crash hit (+0x178, three floats), its player (a byte).  A surface record's
// scrape's kind (+0x14: 3 sparks, else a burst's kind).  The crash's: the push by the hit (a
// double, a 15th), the push's scale (0.6's bits), the shake's frames the other way (35), the
// sparks by the hit (a double; also the speed past which a leaning car scrapes), its loudness by
// the hit, the sparks by the speed (floats), landing hard (a double), the height under which a
// car scrapes, the lean under which it scrapes, under which it touches (doubles), a kind 3
// crash's speed and hit past which it shows (a float).
constexpr u32 kCarCrashPoint = 0x178, kCarPlayer = 0x1f0, kSurfaceScrape = 0x14;
constexpr u32 kPushByHit = 0x5372d0, kPushScale = 0x3f19999a, kCrashShakeFrames = 0x23, kSparksByHit = 0x5372d8;
constexpr u32 kCrashLoudness = 0x5372e0, kSparksBySpeedHit = 0x5372e4, kLandingHard = 0x5372e8;
constexpr u32 kScrapeHeight = 0x5372f0, kScrapeLean = 0x5372f8, kTouchLean = 0x537300, kScrapeHit = 0x537308;

u16 half(App* app, u32 a) { return app->getMemory<u16>(a); }
nfs::Float f32(App* app, u32 a) { return nfs::Float(app->getMemory<float>(a)); }
nfs::Float f64(App* app, u32 a) { return nfs::Float(app->getMemory<double>(a)); }

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.  Its result is the core's: nothing reads a register after it. */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    const auto run = [&] {
        return guest::direct<Address, Core>(app, ts, args...);
    };
    using Result = decltype(run());
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
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = run();
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* A thunk's core run with every register but eax as the thunk was handed it, whatever the
 * calls by address it makes leave: the registers the game's function saves among them. */
template <typename Body>
auto keeping(ThreadState& ts, Body body)
{
    struct Restore
    {
        ThreadState& ts;
        u32 esp;
        ~Restore() { ts.sp = esp; }
    } restore{ts, ts.sp};
    return body();
}

/* Words the game's code pushes for a callee that takes their address (a menu file's core, through
 * feCore): put where those pushes put them, below esp, and esp below them while this lives, so the
 * callee's frame lies where the game's does. */
class Pushed
{
public:
    Pushed(App* app, ThreadState& ts, std::initializer_list<u32> words)
        : ts_(ts), esp_(ts.sp), address_(ts.sp - 4 * u32(words.size()))
    {
        u32 at = address_;
        for (const u32 word : words)
        {
            guest::put(app, at, word);
            at += 4;
        }
        ts.sp = address_;
    }
    ~Pushed() { ts_.sp = esp_; }
    Pushed(const Pushed&) = delete;
    Pushed& operator=(const Pushed&) = delete;
    u32 address() const { return address_; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 address_;
};

using nfs::Float;

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

float f32At(App* app, u32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, u32 a, float v) { app->getMemory<float>(a) = v; }

/* fcom then fnstsw ax: eax with its low word the compare's status word (c3, c2 and c0, as the
 * game's arithmetic has it, fp::compare). */
u32 statusInto(u32 eax, const Float& a, const Float& b)
{
    return (eax & 0xffff0000) | fp::compare(a, b);
}

/* A compare's answers as the code's jumps after sahf read them: a over b (ja: neither c0 nor c3,
 * so never for a NaN), a below b (jb: c0, so for a NaN too), a below or equal (jbe). */
bool fAbove(const Float& a, const Float& b) { return fp::compare(a, b) == 0; }
bool fBelow(const Float& a, const Float& b) { return (fp::compare(a, b) & 0x100) != 0; }
bool fBelowOrEqual(const Float& a, const Float& b) { return fp::compare(a, b) != 0; }

/* Room on the guest's stack for what a callee is handed the address of: the game's frame
 * (`pushed` bytes of registers under esp on entry, then `bytes`), esp under it while the core
 * runs and back after.  On entry esp is at the return address's slot, as the code's is. */
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

/* A call of a core of this file's as the code makes it: the return address's slot taken on the
 * guest's stack, so that the callee's frame lies where the code's does. */
class Calling
{
public:
    explicit Calling(ThreadState& ts) : ts_(ts), esp_(ts.sp) { ts.sp -= 4; }
    ~Calling() { ts_.sp = esp_; }

private:
    ThreadState& ts_;
    const u32 esp_;
};

/* A dot product (0x4e01f0) of the vectors at `a` and `b`, as C++; nullopt when the game stopped.
 * Its code leaves eax as it came, `a`. */
std::optional<Float> dot(App* app, ThreadState& ts, u32 a, u32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, ts, a, b);
    if (ts.terminate)
        return std::nullopt;
    return Float(product);
}

/* The game's sine of a float (0x4ea930, simd's; it leaves eax as it was). */
std::optional<Float> sine(App* app, ThreadState& ts, float angle)
{
    return feCore<0x4ea930, frontend::simd::sineValue>(app, ts, bitsOf(angle));
}

/* The runtime's rand (0x4eb1fc). */
std::optional<i32> rand(App* app, ThreadState& ts)
{
    const u32 value = guest::direct<0x4eb1fc, watcom::clib::rand>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return i32(value);
}

}  // namespace

nfs::u32 respawnCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 resetCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stuckCheck(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearRespawn(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shakeAmplitude(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 startShake(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 startIdleShake(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shakeWave(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 objectContacts(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelSlip(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelCells(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelSurface(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 endSkidMark(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 extendSkidMark(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 skidSounds(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelSpray(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 surfaceEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelSkid(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 wheelEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 updateShake(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 crashEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 respawnEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stopCarEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carEffects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 airborneSpin(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 updateCar(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 defaultGridPlace(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 loadStartGrid(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 gridPlace(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 placeOnGrid(App* app, nfs::ThreadState& ts, nfs::Args& args);

/* The slice `by` slices on from `slice`, wrapped round the track's: past the last when going on,
 * under 0 when going back, as the code wraps it. */
u32 slicesOn(App* app, u32 slice, u32 by)
{
    u32 target = slice + by;
    if (i32(by) >= 0)
    {
        const u32 count = at(app, kSliceCount);
        if (i32(target) >= i32(count))
            target -= count;
    }
    else if (i32(target) < 0)
        target += at(app, kSliceCount);
    return target;
}

/* The car put on slice `slice` (0x428d60), lifted by kRespawnHeight -- by `tunnelScale` in a
 * tunnel (0x420bf0 of its place and slice) -- at rest and settled (0x428970), 0x40 at +0x174.  eax
 * as the settling leaves it; nullopt when the game stopped. */
std::optional<u32> putBackOn(App* app, ThreadState& ts, u32 car, u32 slice, u32 tunnelScale)
{
    if (!feCore<0x428d60, frontend::loada::carPlaceOnRoad>(app, ts, car, slice))
        return std::nullopt;
    float height = f32At(app, kRespawnHeight);
    const std::optional<u32> tunnel =
        guest::direct<0x420bf0, trackdata::placeInTunnel>(app, ts, car + kCarPlace, at(app, car + kCarSlice));
    if (!tunnel)
        return std::nullopt;
    if (*tunnel != 0)
        height = float(fp::mul(Float(height), f32(app, tunnelScale)));
    put(app, car + kCarVelocity, 0);
    put(app, car + kCarVelocity + 4, 0);
    const Float raised = fp::add(f32(app, car + kCarPlace + 4), Float(height));
    put(app, car + kCarVelocity + 8, 0);
    putFloat(app, car + kCarPlace + 4, float(raised));
    const std::optional<u32> settled = feCore<0x428970, frontend::loada::carReset>(app, ts, car);
    if (!settled)
        return std::nullopt;
    put(app, car + kCarSettleFrames, 0x40);
    return settled;
}

/* Where the game's fill (0x4e070c, native_game.cpp's fillCore) leaves eax after `count` bytes at
 * `at`: past them, but on a last odd byte -- its alignment's steps, its blocks of 8, its tail's. */
u32 fillEnd(u32 at, i32 left)
{
    if ((at & 7) != 0)
    {
        if ((at & 1) != 0 && left >= 1)
        {
            at += 1;
            left -= 1;
        }
        if ((at & 2) != 0 && left >= 2)
        {
            at += 2;
            left -= 2;
        }
        if ((at & 4) != 0 && left >= 4)
        {
            at += 4;
            left -= 4;
        }
    }
    if (left >= 0)
    {
        at += u32(left) & ~u32(7);
        if ((left & 4) != 0)
            at += 4;
        if ((left & 2) != 0)
            at += 2;
    }
    return at;
}

/* The way the race runs: 1, or -1 backwards. */
u32 raceDirection(App* app)
{
    return gameSettings(app).backwards != 0 ? 0xffffffff : 1;
}

/* 0x428f90's: the car put back on the road -- the times it was, one more.  Not (the frames since
 * just cleared) when it leans over 0.75, its spin (+0xe4, +0xec) under 1, its surface not 0 or
 * 0xe, no slice asked for, its stage 5 or less and +0x15c under 0.6 (each compared by its bits
 * as an integer, as the code does).  Else kRespawnStep slices on from its slice (or the one
 * asked for, cleared) -- back when the race runs backwards -- wrapped round the track, and put
 * back there (putBackOn, half the height in a tunnel).  eax as the code leaves it (the way, or
 * as the settling leaves it); nullopt when the game stopped. */
std::optional<u32> respawnCarOf(App* app, ThreadState& ts, u32 car)
{
    const u32 direction = raceDirection(app);
    const u32 times = at(app, car + kCarRespawns) + 1;
    put(app, car + kCarSinceRespawn, 0);
    put(app, car + kCarRespawns, times);
    const auto bits = [&](u32 field) { return i32(at(app, car + field)); };
    if (bits(kCarLean) > i32(0x3f400000) && bits(kCarSpin) < i32(kOneBits) && bits(kCarSpin + 8) < i32(kOneBits))
    {
        const u32 surface = at(app, car + kCarSurface);
        if (surface != 0xe && surface != 0 && at(app, car + kCarRespawnSlice) == 0 && half(app, car + kCarStage) <= 5
            && bits(kCarAboveRoad) < i32(0x3f19999a))
        {
            put(app, car + kCarSinceRespawn, 0);
            return direction;
        }
    }
    const u32 asked = at(app, car + kCarRespawnSlice);
    const u32 from = asked != 0 ? asked : at(app, car + kCarSlice);
    const u32 by = direction * at(app, kRespawnStep);
    put(app, car + kCarRespawnSlice, 0);
    return putBackOn(app, ts, car, slicesOn(app, from, by), kHalfHeight);
}

/* 0x428f90: respawnCarOf car eax. */
nfs::u32 respawnCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return respawnCarOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x429100's: the car put back on the road when the player asks: kRespawnStep slices on from its
 * own slice (putBackOn, by 0x5371d4 in a tunnel), the reset's wait 0x20 frames after -- unless it
 * is waiting yet, it is a player's car finished, no car may be (kResetAllowed), a slice is asked
 * for already, or in Hot Pursuit 0x401500 says no of its pursuer (+0x624, chased) or of itself.
 * eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> resetCarOf(App* app, ThreadState& ts, u32 car)
{
    const u32 direction = raceDirection(app);
    if (at(app, car + kCarResetWait) != 0
        || ((byte(app, car + kCarFlags) & kPlayersCar) != 0 && at(app, car + kCarState) == kFinished)
        || at(app, kResetAllowed) == 0 || at(app, car + kCarRespawnSlice) != 0)
        return direction;
    if (gameSettings(app).raceType == kHotPursuit)
    {
        const u32 pursuer = at(app, car + kCarPursuer);
        // What the callee leaves in eax is what this leaves.
        if ((byte(app, car + kCarFlags) & kChased) != 0 && pursuer != 0)
        {
            const u32 out = guest::direct<kPursuitOut, airace::pulledOverOf>(app, ts, pursuer);
            if (ts.terminate)
                return std::nullopt;
            if (out != 0)
                return out;
        }
        if ((byte(app, car + kCarFlags) & kPlayersCar) != 0)
        {
            const u32 out = guest::direct<kPursuitOut, airace::pulledOverOf>(app, ts, car);
            if (ts.terminate)
                return std::nullopt;
            if (out != 0)
                return out;
        }
    }
    const u32 by = direction * at(app, kRespawnStep);
    const std::optional<u32> left =
        putBackOn(app, ts, car, slicesOn(app, at(app, car + kCarSlice), by), kResetHeight);
    if (!left)
        return std::nullopt;
    put(app, car + kCarResetWait, 0x20);
    return left;
}

/* 0x429100: resetCarOf car eax. */
nfs::u32 resetCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return resetCarOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x429260's: the car's stuck count run down while it is above 0: by one unless the player's car
 * leans 0.1 or more with no slice asked for; by two more when its cell is on no piece (+0x20,
 * +0x24) or what it is on (0x4207e0 of its cell) is 0 or 0xe; its spin a 64th less when +0x15c
 * is under 5 and it leans under 0.5, racing; by eight when |its fall| plus its speed is under 1;
 * by one more for the player's car at its throttle asked over 200 leaning under 0.1 (the leans
 * and +0x15c compared by their bits as integers, as the code does).  At 0 or below it is put
 * back (respawnCarOf), but a wreck, and the count is 0.  The count at 0 or below, reset: to
 * kStuckCount (kPlayerStuckCount for the player in the hazard race), 0x200 when a slice is asked
 * for -- but not when the dot of +0x130 and its up (0x4e01f0) is 0.75 or more on a surface of
 * its own.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> stuckCheckOf(App* app, ThreadState& ts, u32 car)
{
    const auto bits = [&](u32 field) { return i32(at(app, car + field)); };
    const auto countDown = [&](u32 by) { put(app, car + kCarStuck, at(app, car + kCarStuck) - by); };
    const auto asked = [&] { return at(app, car + kCarRespawnSlice) != 0; };
    if (bits(kCarStuck) > 0)
    {
        const u32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, ts, car + kCarCell, 1);
        if (ts.terminate)
            return std::nullopt;
        const u32 on = kind & kSurfaceBits;
        if (bits(kCarLean) < i32(0x3dcccccd) || asked() || (byte(app, car + kCarFlags) & kThisMachines) == 0)
            countDown(1);
        if (at(app, car + kCarCell + kWheelPiece) == 0 || at(app, car + kCarCell + kWheelPieceToo) == 0 || on == 0
            || on == 0xe)
            countDown(2);
        if (bits(kCarAboveRoad) < i32(0x40a00000) && !asked() && half(app, car + kCarStage) == 0
            && bits(kCarLean) < i32(0x3f000000))
        {
            const Float k = f32(app, kStuckSpinCut);
            const Float x = fp::mul(f32(app, car + kCarSpin), k);
            const Float y = fp::mul(f32(app, car + kCarSpin + 4), k);
            const Float z = fp::mul(k, f32(app, car + kCarSpin + 8));
            const Float nx = fp::sub(f32(app, car + kCarSpin), x);
            const Float ny = fp::sub(f32(app, car + kCarSpin + 4), y);
            const Float nz = fp::sub(f32(app, car + kCarSpin + 8), z);
            putFloat(app, car + kCarSpin + 4, float(ny));
            putFloat(app, car + kCarSpin + 8, float(nz));
            putFloat(app, car + kCarSpin, float(nx));
        }
        // |the fall| as the code takes it: kept when 0 is below it (or it is a NaN), else negated.
        const float fall = f32At(app, car + kCarVelocity + 4);
        const float size = fBelow(0.0, Float(fall)) ? fall : float(-Float(fall));
        if (fAbove(1.0, fp::add(f32(app, car + kCarSpeed), Float(size))) && !asked() && half(app, car + kCarStage) == 0)
            countDown(8);
        if ((byte(app, car + kCarFlags) & kThisMachines) != 0 && byte(app, car + kCarThrottleAsked) > 0xc8 && !asked()
            && bits(kCarLean) < i32(0x3dcccccd))
            countDown(1);
        if (bits(kCarStuck) <= 0)
        {
            if ((byte(app, car + kCarFlags) & kWreck) == 0)
            {
                const std::optional<u32> left = respawnCarOf(app, ts, car);
                if (!left)
                    return std::nullopt;
                put(app, car + kCarStuck, 0);
                return left;
            }
            put(app, car + kCarStuck, 0);
        }
    }
    const std::optional<Float> uprightDot = dot(app, ts, car + kCarGroundUp, car + kCarUp);
    if (!uprightDot)
        return std::nullopt;
    const float upright = float(*uprightDot);
    // eax as the dot product leaves it: its first vector.
    const u32 dotted = car + kCarGroundUp;
    if (bits(kCarStuck) > 0)
        return dotted;
    const u32 status = statusInto(dotted, Float(upright), f32(app, kUpright));
    if (!fBelow(Float(upright), f32(app, kUpright)))
    {
        const u32 surface = at(app, car + kCarSurface);
        if (surface != 0 && surface != 0xe && !asked())
            return status;
    }
    if (asked())
    {
        put(app, car + kCarStuck, 0x200);
        return status;
    }
    put(app, car + kCarStuck, at(app, kStuckCount));
    if ((byte(app, car + kCarFlags) & kThisMachines) != 0 && gameSettings(app).raceOptionHazard != 0)
    {
        put(app, car + kCarStuck, at(app, kPlayerStuckCount));
        return at(app, kPlayerStuckCount);
    }
    return at(app, kStuckCount);
}

/* 0x429260: stuckCheckOf car eax. */
nfs::u32 stuckCheck(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return stuckCheckOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4294c0's: the car's slice to be put back on and the frames since, cleared. */
void clearRespawnOf(App* app, u32 car)
{
    put(app, car + kCarRespawnSlice, 0);
    put(app, car + kCarSinceRespawn, 0);
}

/* 0x4294c0: clearRespawnOf car eax. */
nfs::u32 clearRespawn(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    clearRespawnOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* The car's surface's record in the surfaces' table (kSurfaces on). */
u32 surfaceRecord(App* app, u32 car)
{
    return at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + at(app, car + kCarSurface) * kSurfaceSize;
}

/* 0x4294e0's: the car's rumble: its surface's rumble by its handling's, times sin(the speed's
 * 64th, 0 to 1, by 0.5) (0x4ea930).  The 64th is held to 1 by its bits, as the code does.  eax
 * as the code leaves it (the compare's status word, or the 64th's bits, as the sine leaves it);
 * nullopt when the game stopped. */
std::optional<u32> shakeAmplitudeOf(App* app, ThreadState& ts, u32 car)
{
    const Float speed = fp::mul(f32(app, car + kCarSpeed), f32(app, kSixtyFourth));
    const u32 surface = surfaceRecord(app, car);
    const u32 handling = at(app, car + kCarHandling);
    const Float rumble = fp::mul(f32(app, surface + kSurfaceRumble), f32(app, handling + kHandlingRumble));
    const float scaled = float(speed);
    putFloat(app, car + kCarRumble, float(rumble));
    u32 phase = 0;
    // eax as the code leaves it: the sine (0x4ea930) keeps it.
    u32 left = statusInto(handling, 0.0, Float(scaled));
    if (!fAbove(0.0, Float(scaled)))
    {
        phase = i32(bitsOf(scaled)) > i32(kOneBits) ? kOneBits : bitsOf(scaled);
        left = phase;
    }
    const std::optional<Float> wave = sine(app, ts, float(fp::mul(Float(floatOf(phase)), f64(app, kHalfTurn))));
    if (!wave)
        return std::nullopt;
    const float s = float(*wave);
    putFloat(app, car + kCarRumble, float(fp::mul(f32(app, car + kCarRumble), Float(s))));
    return left;
}

/* 0x4294e0: shakeAmplitudeOf car eax. */
nfs::u32 shakeAmplitude(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return shakeAmplitudeOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* What startShakeOf leaves in eax, edx and ebx, none of which its code keeps. */
struct ShakeLeft
{
    u32 eax, edx, ebx;
};

/* 0x4295a0's: the car's crash shake started, if it hit hard enough (how hard, +0x168, 5 or more
 * by its bits as an integer): two waves, across and up -- the shake's tuning by its handling's
 * (+0x294), by the hit (a 45th, 0 to 1.5) -- for `acrossFrames` and `upFrames` frames, each its
 * sign turned at random (rand's low byte over 0x80).  What the code leaves in eax, edx and ebx;
 * nullopt when the game stopped. */
std::optional<ShakeLeft> startShakeOf(App* app, ThreadState& ts, u32 car, u32 acrossFrames, u32 upFrames)
{
    const u32 handling = at(app, car + kCarHandling);
    const float across = float(fp::mul(f32(app, kShakeAcross), f32(app, handling + kHandlingShake)));
    const float up = float(fp::mul(f32(app, kShakeUp), f32(app, handling + kHandlingShake)));
    if (i32(at(app, car + kCarHit)) < i32(0x40a00000))
        return ShakeLeft{handling, acrossFrames, upFrames};
    const float hit = float(fp::mul(f32(app, car + kCarHit), f32(app, kHitScale)));
    Float share = 0.0;
    if (!fAbove(0.0, Float(hit)))
        share = fAbove(Float(hit), f64(app, kMostShake)) ? 1.5 : Float(hit);
    const float shared = float(share);
    put(app, car + kCarWaves + kWaveFrames, acrossFrames);
    putFloat(app, car + kCarWaves, float(fp::mul(share, Float(across))));
    const std::optional<i32> acrossSign = rand(app, ts);
    if (!acrossSign)
        return std::nullopt;
    if (*acrossSign % 0x100 > 0x80)
        putByte(app, car + kCarWaves + 3, byte(app, car + kCarWaves + 3) ^ 0x80);
    const u32 second = car + kCarWaves + kWaveSize;
    const float upAmplitude = float(fp::mul(Float(shared), Float(up)));
    put(app, second + kWaveFrames, upFrames);
    putFloat(app, second, upAmplitude);
    const std::optional<i32> upSign = rand(app, ts);
    if (!upSign)
        return std::nullopt;
    u32 rest = u32(*upSign % 0x100);
    if (i32(rest) > 0x80)
    {
        const u8 flipped = byte(app, second + 3) ^ 0x80;
        putByte(app, second + 3, flipped);
        rest = (rest & 0xffffff00) | flipped;
    }
    return ShakeLeft{u32(*upSign / 0x100), rest, 0x100};
}

/* 0x4295a0: startShakeOf car eax, for edx and ebx frames. */
nfs::u32 startShake(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<ShakeLeft> left =
        keeping(ts, [&] { return startShakeOf(app, ts, args.eax, args.edx, args.ebx); });
    if (!left)
        return args.eax;
    args.eax = left->eax;
    args.edx = left->edx;
    args.ebx = left->ebx;
    ts.sp += 4;
    return args.eax;
}

/* 0x4296c0's: the car's little shake, leaning -0.9 or less (or a NaN) and racing: two small
 * waves, each of 24 to 35 frames (rand), each its sign turned at random (rand's low byte over
 * 0x80).  eax as the code leaves it: the compare's status word, or the last rand by 256;
 * nullopt when the game stopped. */
std::optional<u32> startIdleShakeOf(App* app, ThreadState& ts, u32 car)
{
    const u32 status = statusInto(car, f32(app, car + kCarLean), f64(app, kIdleLean));
    if (fAbove(f32(app, car + kCarLean), f64(app, kIdleLean)) || half(app, car + kCarStage) != 0)
        return status;
    i32 last = 0;
    for (const u32 wave : {car + kCarWaves, car + kCarWaves + kWaveSize})
    {
        put(app, wave, kIdleAmplitude);
        const std::optional<i32> frames = rand(app, ts);
        if (!frames)
            return std::nullopt;
        put(app, wave + kWaveFrames, u32(*frames % kIdleFramesSpread + kIdleFrames));
        const std::optional<i32> sign = rand(app, ts);
        if (!sign)
            return std::nullopt;
        if (*sign % 0x100 > 0x80)
            putByte(app, wave + 3, byte(app, wave + 3) ^ 0x80);
        last = *sign / 0x100;
    }
    return u32(last);
}

/* 0x4296c0: startIdleShakeOf car eax. */
nfs::u32 startIdleShake(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return startIdleShakeOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x429790's: a shake's wave `frames` from its end: sin(frames / 12) (0x4ea930) by the
 * amplitude, by the frames (as a float), by a 12th.  Nullopt when the game stopped. */
std::optional<Float> shakeWaveOf(App* app, ThreadState& ts, float amplitude, i32 frames)
{
    const Float count = Float(frames);
    const float counted = float(count);
    const std::optional<Float> wave = sine(app, ts, float(fp::mul(count, f32(app, kTwelfth))));
    if (!wave)
        return std::nullopt;
    return fp::mul(fp::mul(fp::mul(*wave, Float(amplitude)), Float(counted)), f32(app, kTwelfth));
}

/* 0x429790 (stdcall: the amplitude, the frames left): shakeWaveOf, onto the x87's stack; eax
 * kept, as the code keeps it. */
nfs::u32 shakeWave(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = args.eax;
    const std::optional<Float> wave =
        keeping(ts, [&] { return shakeWaveOf(app, ts, f32At(app, ts.sp + 4), i32(at(app, ts.sp + 8))); });
    if (!wave)
        return args.eax;
    args.floatSet = true;
    args.floatResult = double(*wave);
    args.eax = eax;
    ts.sp += 4 + 8;
    return args.eax;
}

/* 0x4297d0's: the car against the scenery's objects: the player's first (0x493ad0); then, a
 * player's car still racing (its state not finished, and not out unless the player's), the
 * object it is against (0x4ca160, its handle to a word of the frame's): handed over (0x4c9f50),
 * the hit's flags kept, 0x40 at +0x621, the car told (0x401650).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<u32> objectContactsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 handle = frame - 4;
    u32 left = car;
    if ((byte(app, car + kCarFlags) & kThisMachines) != 0)
    {
        const std::optional<u32> done = feCore<0x493ad0, frontend::rendc::conesHit>(app, ts, car);
        if (!done)
            return std::nullopt;
        left = *done;
    }
    const u8 flags = byte(app, car + kCarFlags);
    if (half(app, car + kCarStage) != 0 || (flags & kPlayersCar) == 0 || at(app, car + kCarState) == kFinished
        || (at(app, car + kCarState) == kOut && (flags & kThisMachines) == 0))
        return left;
    const std::optional<u32> against = feCore<0x4ca160, frontend::sime::stripUnderWheels>(app, ts, car, handle);
    if (!against)
        return std::nullopt;
    if (*against == 0)
        return u32(0);
    // The hit's flags as the handing over leaves them in edx: as it came, the object found (its
    // code keeps edx).
    if (!feCore<kObjectHit, frontend::simc::sirenOff>(app, ts, at(app, handle)))
        return std::nullopt;
    const u32 hit = *against;
    putByte(app, car + kCarObjectFlags, byte(app, car + kCarObjectFlags) | kHitObject);
    put(app, car + kCarHits, at(app, car + kCarHits) | hit);
    return guest::direct<kCarHitObject, airace::stopForCopLeaving>(app, ts, car, at(app, handle));
}

/* 0x4297d0: objectContactsOf car eax. */
nfs::u32 objectContacts(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return objectContactsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* What wheelSlipOf works out, and what the code leaves in eax and edx. */
struct Slip
{
    Float value;
    Left left;
};

/* 0x429880's: how much a wheel of the car slips (a front one's when `front` is not 0), 0 to 1:
 * its spin's length (0x4e06a0) plus its speed, by 0x537210 and by how sideways it goes (1 less
 * |the dot of its velocity, normalised (0x4972f0), with its axes' forward|, 0x4e01f0, the two
 * copied into the frame), plus
 * 0x53720c and its slide (+0x598 a front wheel's, +0x594 a back's, whole, its size) by it.  eax
 * and edx as the code leaves them; nullopt when the game stopped. */
std::optional<Slip> wheelSlipOf(App* app, ThreadState& ts, u32 car, u32 front)
{
    const GuestFrame frame(ts, 0x14, 0x50);
    const u32 velocity = frame - 0x50, forward = frame - 0x44;
    for (u32 k = 0; k < 0xc; k += 4)
        put(app, velocity + k, at(app, car + kCarVelocity + k));
    const Float length = guest::direct<0x4e06a0, vertices::vectorLength>(app, ts, car + kCarSpin);
    if (ts.terminate)
        return std::nullopt;
    const float turning = float(fp::add(length, f32(app, car + kCarSpeed)));
    guest::direct<0x4972f0, vertices::normaliseVector>(app, ts, velocity);
    if (ts.terminate)
        return std::nullopt;
    for (u32 k = 0; k < 0xc; k += 4)
        put(app, forward + k, at(app, car + kCarForward + k));
    const float along = float(guest::direct<0x4e01f0, vertices::dotProduct>(app, ts, velocity, forward));
    if (ts.terminate)
        return std::nullopt;
    // Its size as the code takes it: kept when 0 is below it (or it is a NaN), else negated.
    const float size = fBelow(0.0, Float(along)) ? along : float(-Float(along));
    const float sideways = float(fp::sub(1.0, Float(size)));
    u32 second = forward;  // edx as the dot product leaves it: its second vector
    u32 slide;
    if (front != 0)
    {
        second = at(app, car + kCarFrontSlide);
        slide = i32(second) > 0 ? second : 0u - second;
    }
    else
    {
        const u32 back = at(app, car + kCarBackSlide);
        slide = i32(back) > 0 ? back : 0u - back;
    }
    const Float base = f32(app, kSlipBase);
    const Float slid = fp::add(base, fp::mul(Float(i32(slide)), base));
    const float slip = float(fp::add(slid, fp::mul(fp::mul(Float(turning), f32(app, kSlipBySpin)), Float(sideways))));
    const Float least = fAbove(0.0, Float(slip)) ? 0.0 : Float(slip);
    if (fAbove(1.0, least))
    {
        std::uint64_t bits;
        std::memcpy(&bits, &least, sizeof bits);
        return Slip{least, {u32(bits >> 32), second}};
    }
    return Slip{1.0, {0, 0x3ff00000}};
}

/* 0x429880: wheelSlipOf car eax, a front wheel's when edx is not 0, onto the x87's stack. */
nfs::u32 wheelSlip(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Slip> slip = keeping(ts, [&] { return wheelSlipOf(app, ts, args.eax, args.edx); });
    if (!slip)
        return args.eax;
    args.floatSet = true;
    args.floatResult = double(slip->value);
    args.eax = slip->left.eax;
    args.edx = slip->left.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x4299a0's: the car's four wheels on the track: its corners -- across 0.75 of its half size
 * (+0x114) either way, down its height (+0x118), along half its length (+0x11c) either way --
 * turned by its axes and moved to where it will be (its place plus a 32nd of its velocity;
 * 0x4e0430, the corners, the place and what it makes in the frame), as 16.16 (each to the
 * nearest), each looked up from the car's slice (0x420650, into a record of the frame's) and
 * copied to the wheel's 0x78 bytes.  Nullopt when the game stopped. */
bool wheelCellsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x104);
    const u32 record = frame - 0x104, turned = frame - 0x8c, corners = frame - 0x5c, moved = frame - 0x2c;
    const u32 place = frame - 0x20;
    const float across = f32At(app, car + kCarHalfSizes), down = f32At(app, car + kCarHalfSizes + 4);
    const float along = f32At(app, car + kCarHalfSizes + 8);
    const Float alongShare = f64(app, kWheelsAlong), acrossShare = f64(app, kWheelsAcross);
    const Float ahead = fp::mul(Float(along), alongShare);
    const Float right = fp::mul(Float(across), acrossShare);
    const Float left = fp::mul(acrossShare, -Float(across));
    const Float behind = fp::mul(alongShare, -Float(along));
    const float under = float(-Float(down));
    const float corner[4][3] = {{float(left), under, float(ahead)},
                                {float(right), under, float(ahead)},
                                {float(right), under, float(behind)},
                                {float(left), under, float(behind)}};
    for (u32 c = 0; c < 4; ++c)
        for (u32 k = 0; k < 3; ++k)
            putFloat(app, corners + 0xc * c + 4 * k, corner[c][k]);
    const Float step = f32(app, kThirtySecond);
    putFloat(app, moved, float(fp::add(fp::mul(f32(app, car + kCarVelocity), step), f32(app, car + kCarPlace))));
    putFloat(app, moved + 4,
             float(fp::add(fp::mul(f32(app, car + kCarVelocity + 4), step), f32(app, car + kCarPlace + 4))));
    putFloat(app, moved + 8,
             float(fp::add(fp::mul(step, f32(app, car + kCarVelocity + 8)), f32(app, car + kCarPlace + 8))));
    guest::direct<0x4e0430, vertices::transformPointList>(app, ts, 4, corners, car + kCarAxes, moved, turned);
    if (ts.terminate)
        return false;
    for (u32 wheel = 0; wheel < 4; ++wheel)
    {
        for (u32 k = 0; k < 3; ++k)
            put(app, place + 4 * k,
                u32(fp::toInteger<i32>(fp::mul(f32(app, turned + 0xc * wheel + 4 * k), f64(app, kToFixed)))));
        if (!guest::direct<0x420650, trackdata::placeAt>(app, ts, place, 1, at(app, car + kCarSlice), record))
            return false;
        for (u32 k = 0; k < kWheelSize; k += 4)
            put(app, car + kCarWheels + wheel * kWheelSize + k, at(app, record + k));
    }
    return true;
}

/* 0x4299a0: wheelCellsOf car eax; eax left as the code leaves it, at the last wheel's cell less
 * 0x300. */
nfs::u32 wheelCells(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (!keeping(ts, [&] { return wheelCellsOf(app, ts, car); }))
        return args.eax;
    args.eax = car + 3 * kWheelSize;
    ts.sp += 4;
    return args.eax;
}

/* 0x429b40's: what wheel `wheel` of the car is on: the car's surface, or, on a piece of track of
 * its own with the race on, the wheel's (0x4207e0).  Nullopt when the game stopped. */
std::optional<u32> wheelSurfaceOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    const u32 cell = car + kCarWheels + wheel * kWheelSize;
    if (half(app, car + kCarStage) != 0 || at(app, cell + kWheelPiece) == 0 || at(app, cell + kWheelPieceToo) == 0)
        return at(app, car + kCarSurface);
    const u32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, ts, cell, 1);
    if (ts.terminate)
        return std::nullopt;
    return kind & kSurfaceBits;
}

/* 0x429b40: wheelSurfaceOf car eax's wheel edx, in eax and edx both. */
nfs::u32 wheelSurface(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> surface = keeping(ts, [&] { return wheelSurfaceOf(app, ts, args.eax, args.edx); });
    if (!surface)
        return args.eax;
    args.eax = *surface;
    args.edx = *surface;
    ts.sp += 4;
    return args.eax;
}

/* A wheel's cell's height under it, axis `k` (0 to 2): on a piece of its own its heights' 16-bit
 * word k (at +0x1c's) by 2, else its slice's byte (+0x10 on, signed) by 512. */
u32 cellHeight(App* app, u32 cell, u32 k)
{
    if (at(app, cell + kWheelPiece) != 0)
        return u32(i32(i16(half(app, at(app, cell + kWheelHeights) + 2 * k)))) * 2;
    const u32 record = at(app, kSlices) + at(app, cell + kWheelSlice) * kSliceSize;
    return u32(i32(i8(byte(app, record + kSliceHeights + k)))) << 9;
}

/* A mark's point drawn (0x4db940): the car's number and the wheel, the point and the heights
 * under it (frame words), the stack's `ending` and the slip.  False when the game stopped. */
bool markPoint(App* app, ThreadState& ts, u32 car, u32 wheel, u32 point, u32 heights, u32 ending, float slip)
{
    return feCore<kMarkPoint, frontend::rende::skidWheel>(app, ts, at(app, car + kCarNumber), wheel, point,
                                                           heights, ending, bitsOf(slip))
        .has_value();
}

/* 0x429ba0's: wheel `wheel`'s skid mark ended, if it has one: its last point, the heights under
 * it, how much it slips (wheelSlipOf, the front's slide for wheels 0 and 1) -- 0x4db940 with 1
 * -- and the mark and its sound off.  What the code leaves in eax (edx, which its code does not
 * keep, is no caller's); nullopt when the game stopped. */
std::optional<u32> endSkidMarkOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    const GuestFrame frame(ts, 0x14, 0x24);
    if (byte(app, car + wheel + kCarMarks) == 0)
        return car + wheel;
    std::optional<Slip> slip;
    {
        const Calling calling(ts);
        slip = wheelSlipOf(app, ts, car, i32(wheel) < 2 ? 1 : 0);
    }
    if (!slip)
        return std::nullopt;
    const u32 point = frame - 0x18, heights = frame - 0x24;
    for (u32 k = 0; k < 0xc; k += 4)
        put(app, point + k, at(app, car + wheel * 0xc + kCarLastPoints + k));
    const float slipped = float(slip->value);
    const u32 cell = car + kCarWheels + wheel * kWheelSize;
    for (u32 k = 0; k < 3; ++k)
        put(app, heights + 4 * k, cellHeight(app, cell, k));
    if (!markPoint(app, ts, car, wheel, point, heights, 1, slipped))
        return std::nullopt;
    putByte(app, car + wheel + kCarMarks, 0);
    putByte(app, car + wheel + kCarMarkSounds, 0);
    return car + wheel;
}

/* 0x429ba0: endSkidMarkOf car eax's wheel edx. */
nfs::u32 endSkidMark(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return endSkidMarkOf(app, ts, args.eax, args.edx); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x429d20's: wheel `wheel`'s skid mark drawn on to where it is: off the track's pieces, ended
 * (0x4db860); else a point (0x4db940 with 0) -- its place, the heights under it, how much it
 * slips (wheelSlipOf) -- kept as the last, the mark on and its sound the surface's (wheelSurfaceOf).
 * What the code leaves in eax and edx (the wheel where the cut leaves it: edx is no caller's);
 * nullopt when the game stopped. */
std::optional<Left> extendSkidMarkOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    const GuestFrame frame(ts, 0x14, 0x28);
    std::optional<Slip> slip;
    {
        const Calling calling(ts);
        slip = wheelSlipOf(app, ts, car, i32(wheel) < 2 ? 1 : 0);
    }
    if (!slip)
        return std::nullopt;
    const float slipped = float(slip->value);
    const u32 cell = car + kCarWheels + wheel * kWheelSize;
    const u32 point = frame - 0x28, heights = frame - 0x1c;
    for (u32 k = 0; k < 0xc; k += 4)
        put(app, point + k, at(app, cell + kWheelPlace + k));
    std::optional<u32> surface;
    {
        const Calling calling(ts);
        surface = wheelSurfaceOf(app, ts, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    if (at(app, cell + kWheelPiece) == 0 || at(app, cell + kWheelPieceToo) == 0)
    {
        if (!feCore<kCutMark, frontend::sime::wheelMarkCut>(app, ts, at(app, car + kCarNumber), wheel))
            return std::nullopt;
        putByte(app, car + wheel + kCarMarks, 0);
        putByte(app, car + wheel + kCarMarkSounds, 0);
        return Left{car + wheel, wheel};
    }
    for (u32 k = 0; k < 3; ++k)
        put(app, heights + 4 * k, cellHeight(app, cell, k));
    if (!markPoint(app, ts, car, wheel, point, heights, 0, slipped))
        return std::nullopt;
    for (u32 k = 0; k < 0xc; k += 4)
        put(app, car + wheel * 0xc + kCarLastPoints + k, at(app, point + k));
    putByte(app, car + wheel + kCarMarks, 1);
    const u32 sound = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + *surface * kSurfaceSize + kSurfaceSound;
    const u8 heard = byte(app, sound);
    putByte(app, car + wheel + kCarMarkSounds, heard);
    return Left{((*surface * 9) & 0xffffff00) | heard, *surface};
}

/* 0x429d20: extendSkidMarkOf car eax's wheel edx. */
nfs::u32 extendSkidMark(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Left> left = keeping(ts, [&] { return extendSkidMarkOf(app, ts, args.eax, args.edx); });
    if (!left)
        return args.eax;
    args.eax = left->eax;
    args.edx = left->edx;
    ts.sp += 4;
    return args.eax;
}

/* An event added to the car (0x4284e0, loada's) as C++: edx, ebx and ecx its fields, the stack's
 * three words (the first a field, the second a time of 0 to 10, the third the last field).  eax as
 * the callee leaves it; nullopt when the game stopped. */
std::optional<u32> carEvent(App* app, ThreadState& ts, u32 car, u32 first, u32 second, u32 third,
                              std::initializer_list<u32> words)
{
    const u32* const word = words.begin();
    return feCore<0x4284e0, frontend::loada::carAddEvent>(app, ts, car, first, second, third, word[0], word[1],
                                                          word[2]);
}

/* 0x429f10's: the car's skid sounds: the front wheels' (their sounds +0x990, +0x991) and the
 * back's (+0x992, +0x993), events (0x4284e0) by how much they skid -- both: half of both, the
 * back's with the second's sound; one: its own -- or stopped once (+0x994, +0x995 whether on).
 * eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> skidSoundsOf(App* app, ThreadState& ts, u32 car)
{
    const auto sound = [&](u32 wheel) { return u32(byte(app, car + kCarMarkSounds + wheel)); };
    const auto skid = [&](u32 wheel) { return car + kCarSkids + 4 * wheel; };
    const auto both = [&](u32 wheel) {
        return bitsOf(float(fp::mul(fp::add(f32(app, skid(wheel)), f32(app, skid(wheel + 1))), f32(app, kSkidHalf))));
    };
    u32 left = car;
    // The front wheels' (the event's edx 2 played, 3 stopped; ebx 8).
    {
        std::optional<u32> played;
        bool playing = true;
        if (sound(0) != 0 && sound(1) != 0)
            played = carEvent(app, ts, car, 2, 8, 6, {sound(0), both(0), 0});
        else if (sound(0) != 0)
            played = carEvent(app, ts, car, 2, 8, 6, {sound(0), at(app, skid(0)), 0});
        else if (sound(1) != 0)
            played = carEvent(app, ts, car, 2, 8, 6, {sound(1), at(app, skid(1)), 0});
        else
            playing = false;
        if (playing)
        {
            if (!played)
                return std::nullopt;
            putByte(app, car + kCarScrapes, 1);
            left = *played;
        }
        else if (byte(app, car + kCarScrapes) != 0)
        {
            const std::optional<u32> stopped = carEvent(app, ts, car, 3, 8, 6, {0, 0, 0});
            if (!stopped)
                return std::nullopt;
            putByte(app, car + kCarScrapes, 0);
            left = *stopped;
        }
    }
    // The back's (edx 2 both, 4 one, 5 stopped; ebx 10).
    std::optional<u32> played;
    if (sound(2) != 0 && sound(3) != 0)
        played = carEvent(app, ts, car, 2, 0xa, 6, {sound(3), both(2), 0});
    else if (sound(2) != 0)
        played = carEvent(app, ts, car, 4, 0xa, 6, {sound(2), at(app, skid(2)), 0});
    else if (sound(3) != 0)
        played = carEvent(app, ts, car, 4, 0xa, 6, {sound(3), at(app, skid(3)), 0});
    else
    {
        if (byte(app, car + kCarScrapes + 1) == 0)
            return left;
        const std::optional<u32> stopped = carEvent(app, ts, car, 5, 0xa, 6, {0, 0, 0});
        if (stopped)
            putByte(app, car + kCarScrapes + 1, 0);
        return stopped;
    }
    if (played)
        putByte(app, car + kCarScrapes + 1, 1);
    return played;
}

/* 0x429f10: skidSoundsOf car eax. */
nfs::u32 skidSounds(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return skidSoundsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* An effect where wheel `wheel` of the car is (0x4cb010, stdcall: its place's three words and the
 * kind, pushed).  What the callee leaves in eax; nullopt when the game stopped. */
std::optional<u32> wheelEffect(App* app, ThreadState& ts, u32 car, u32 wheel, u32 kind)
{
    const u32 cell = car + kCarWheels + wheel * kWheelSize;
    const Pushed pushed(app, ts,
                        {at(app, cell + kWheelPlace), at(app, cell + kWheelPlace + 4), at(app, cell + kWheelPlace + 8), kind});
    return feCore<kWheelSmoke, frontend::sime::wheelParticles>(app, ts, car, wheel, pushed.address());
}

/* 0x42a130's: wheel `wheel` of the car hitting something (its bit of what the wheels hit), the
 * race on: dust past 5 (0x4cb010, 6), more past 20 (9), past 10 the sparks (0x4cac20) where the
 * wheel is (its 16.16 place as floats), by its speed, and their event (0x4284e0).  What the code
 * leaves in eax (edx, which its code does not keep, is no caller's); nullopt when the game
 * stopped. */
std::optional<u32> wheelSprayOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    const u32 bit = u32(1) << (wheel & 31);
    const u32 hits = at(app, car + kCarHits);
    if ((bit & hits) == 0 || half(app, car + kCarStage) != 0)
        return bit;
    u32 left = bit;
    for (const auto& [past, kind] : {std::pair<u32, u32>{kDustSpeed, 6}, {kMoreDustSpeed, 9}})
    {
        left = statusInto(left, f32(app, car + kCarSpeed), f64(app, past));
        if (!fAbove(f32(app, car + kCarSpeed), f64(app, past)))
            continue;
        const std::optional<u32> dusted = wheelEffect(app, ts, car, wheel, kind);
        if (!dusted)
            return std::nullopt;
        left = *dusted;
    }
    const float speed = f32At(app, car + kCarSpeed);
    left = statusInto(left, Float(speed), f64(app, kSparksSpeed));
    if (fBelowOrEqual(Float(speed), f64(app, kSparksSpeed)))
        return left;
    const u32 cell = car + kCarWheels + wheel * kWheelSize;
    const Float scale = f64(app, kFromFixed);
    const Float bySpeed = fp::mul(Float(speed), f64(app, kSparksBySpeed));
    const Float x = fp::mul(Float(i32(at(app, cell + kWheelPlace))), scale);
    const Float y = fp::mul(Float(i32(at(app, cell + kWheelPlace + 4))), scale);
    const Float z = fp::mul(scale, Float(i32(at(app, cell + kWheelPlace + 8))));
    {
        const Pushed pushed(app, ts, {bitsOf(float(x)), bitsOf(float(y)), bitsOf(float(z)), bitsOf(float(bySpeed))});
        if (!feCore<kSparks, frontend::sime::sparks>(app, ts, car, pushed.address()))
            return std::nullopt;
    }
    return carEvent(app, ts, car, 1, 0xffffffff, 5, {0x12, 0, 0});
}

/* 0x42a130: wheelSprayOf car eax's wheel edx. */
nfs::u32 wheelSpray(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return wheelSprayOf(app, ts, args.eax, args.edx); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42a280's: wheel `wheel`'s effect from its surface's record (wheelSurfaceOf; kSurfaces on, by
 * the track and the surface), where it has one: skidding past the surface's skid, the effect
 * where the wheel is (0x4cb010) and, on a surface without marks, its sound; else, faster than
 * the surface's skid starts (where its start and range are not 0), the effect, a back wheel's
 * sound, and how much it skids: (speed - start) / range by 5, 0 to 5.  What the code leaves in
 * eax and edx; nullopt when the game stopped. */
std::optional<Left> surfaceEffectsOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    std::optional<u32> surface;
    {
        const Calling calling(ts);
        surface = wheelSurfaceOf(app, ts, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    const bool front = i32(wheel) < 2;
    const u32 track = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize;
    const u32 record = track + *surface * kSurfaceSize;
    const u32 effect = at(app, record + kSurfaceEffect);
    if (effect == 0)
        return Left{0, *surface};
    const u32 from = at(app, record + kSurfaceSkidFrom), over = at(app, record + kSurfaceSkidOver);
    const u32 skid = car + wheel * 4 + kCarSkids, mark = car + wheel;
    const u32 wheelRecord = car + wheel * kWheelSize;
    u32 status = statusInto(skid - kCarSkids, f32(app, skid), f32(app, record + kSurfaces));
    if (fAbove(f32(app, skid), f32(app, record + kSurfaces)))
    {
        if (!wheelEffect(app, ts, car, wheel, effect))
            return std::nullopt;
        const u32 own = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + *surface * kSurfaceSize;
        if (at(app, own + kSurfaceMarks) != 0)
            return Left{own, at(app, car + kCarTrackSurfaces)};
        const u8 heard = byte(app, own + kSurfaceSound);
        putByte(app, mark + kCarMarkSounds, heard);
        return Left{(own & 0xffffff00) | heard, mark};
    }
    if ((from & 0x7fffffff) == 0 || (over & 0x7fffffff) == 0)
        return Left{status, wheelRecord};
    status = statusInto(status, f32(app, car + kCarSpeed), Float(floatOf(from)));
    if (fBelowOrEqual(f32(app, car + kCarSpeed), Float(floatOf(from))))
        return Left{status, wheelRecord};
    const std::optional<u32> shown = wheelEffect(app, ts, car, wheel, effect);
    if (!shown)
        return std::nullopt;
    u32 left = *shown;
    if (!front)
    {
        const u32 tracks = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize;
        const u8 heard = byte(app, *surface * kSurfaceSize + tracks + kSurfaceRearSound);
        putByte(app, mark + kCarMarkSounds, heard);
        left = (tracks & 0xffffff00) | heard;
    }
    const Float skidding = fp::mul(
        fp::div(fp::sub(f32(app, car + kCarSpeed), Float(floatOf(from))), Float(floatOf(over))), f64(app, kMostSkid));
    putFloat(app, skid, float(skidding));
    const Float stored = f32(app, skid);
    if (fAbove(0.0, stored))
    {
        putFloat(app, skid, 0.0f);
        return Left{statusInto(left, 0.0, stored), 0};
    }
    const Float held = fAbove(stored, f64(app, kMostSkid)) ? 5.0 : stored;
    std::uint64_t bits;
    std::memcpy(&bits, &held, sizeof bits);
    putFloat(app, skid, float(held));
    return Left{u32(bits >> 32), skid - kCarSkids};
}

/* 0x42a280: surfaceEffectsOf car eax's wheel edx. */
nfs::u32 surfaceEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Left> left = keeping(ts, [&] { return surfaceEffectsOf(app, ts, args.eax, args.edx); });
    if (!left)
        return args.eax;
    args.eax = left->eax;
    args.edx = left->edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x42a480's: wheel `wheel`'s skid mark: drawn on while it skids on a surface that marks
 * (extendSkidMarkOf; the surface's low 4 bits) -- a back wheel skidding past 5 smokes (0x4cb010
 * at its place, 6) -- else ended (endSkidMarkOf).  What the code leaves in eax and edx (the wheel
 * where a callee leaves it: edx is no caller's); nullopt when the game stopped. */
std::optional<Left> wheelSkidOf(App* app, ThreadState& ts, u32 car, u32 wheel)
{
    const GuestFrame frame(ts, 0x14, 0xc);  // as deep as the code's, for its callees' frames
    const bool front = i32(wheel) < 2;
    std::optional<u32> surface;
    {
        const Calling calling(ts);
        surface = wheelSurfaceOf(app, ts, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    const u32 skid = car + wheel * 4 + kCarSkids;
    const u32 record = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + (*surface & 0xf) * kSurfaceSize;
    if ((at(app, skid) & 0x7fffffff) != 0 && at(app, record + kSurfaceMarks) != 0)
    {
        std::optional<Left> left;
        {
            const Calling calling(ts);
            left = extendSkidMarkOf(app, ts, car, wheel);
        }
        if (!left || front)
            return left;
        if (fBelowOrEqual(f32(app, skid), f64(app, kSmokingSkid)))
            return Left{statusInto(left->eax, f32(app, skid), f64(app, kSmokingSkid)), left->edx};
        const std::optional<u32> smoked = wheelEffect(app, ts, car, wheel, kSmoke);
        if (!smoked)
            return std::nullopt;
        return Left{*smoked, wheel};
    }
    const Calling calling(ts);
    const std::optional<u32> ended = endSkidMarkOf(app, ts, car, wheel);
    if (!ended)
        return std::nullopt;
    return Left{*ended, wheel};
}

/* 0x42a480: wheelSkidOf car eax's wheel edx. */
nfs::u32 wheelSkid(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Left> left = keeping(ts, [&] { return wheelSkidOf(app, ts, args.eax, args.edx); });
    if (!left)
        return args.eax;
    args.eax = left->eax;
    args.edx = left->edx;
    ts.sp += 4;
    return args.eax;
}

/* A wheel's skid by its slide: 0 under 0, 10 over. */
Float heldSkid(App* app, const Float& slide)
{
    if (fAbove(0.0, slide))
        return 0.0;
    return fAbove(slide, f64(app, kMostWheelSkid)) ? 10.0 : slide;
}

/* 0x42a560's: the car's wheels: what it hit (kept while +0x621 has 0x40, the frames since
 * counted at +0x9a0): the first frame an event, the first two the wheels' dust (0x4cb010, 13);
 * then each wheel, its sound off, on the ground with the race on (leaning not under 0x537278):
 * its skid from the slides (+0x5c0 front by 0x537280, +0x5c4 back; 0 to 10), its mark, its
 * surface's effect, what it hit (wheelSkidOf, surfaceEffectsOf, wheelSprayOf); else its mark cut
 * (0x4db860) and its skid none.  Then the sounds (skidSoundsOf).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<u32> wheelEffectsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x30);  // as deep as the code's, for its callees' frames
    if ((byte(app, car + kCarObjectFlags) & kHitObject) == 0)
    {
        put(app, car + kCarHitFrames, 0);
        put(app, car + kCarHits, 0);
    }
    else
        put(app, car + kCarHitFrames, at(app, car + kCarHitFrames) + 1);
    if (half(app, car + kCarStage) == 0 && at(app, car + kCarHits) != 0 && i32(at(app, car + kCarHitFrames)) < 3)
    {
        if (at(app, car + kCarHitFrames) == 1 && !carEvent(app, ts, car, 1, 0xffffffff, 6, {0x13, 0x41200000, 0}))
            return std::nullopt;
        for (u32 wheel = 0; wheel < 4; ++wheel)
            if ((byte(app, car + kCarHits) & (1 << wheel)) != 0 && !wheelEffect(app, ts, car, wheel, kHitDust))
                return std::nullopt;
    }
    for (u32 wheel = 0; wheel < 4; ++wheel)
    {
        putByte(app, car + wheel + kCarMarkSounds, 0);
        const u32 skid = car + wheel * 4 + kCarSkids;
        if (half(app, car + kCarStage) == 0 && !fBelow(f32(app, car + kCarLean), f64(app, kSkidLean)))
        {
            Float held;
            if (i32(wheel) < 2)
                held = heldSkid(app, fp::mul(f32(app, car + kCarFrontSlip), f64(app, kSkidBySlip)));
            else
                held = heldSkid(app, f32(app, car + kCarBackSlip));
            putFloat(app, skid, float(held));
            const Calling calling(ts);
            if (!wheelSkidOf(app, ts, car, wheel) || !surfaceEffectsOf(app, ts, car, wheel)
                || !wheelSprayOf(app, ts, car, wheel))
                return std::nullopt;
        }
        else
        {
            if (!feCore<0x4db860, frontend::sime::wheelMarkCut>(app, ts, at(app, car + kCarNumber), wheel))
                return std::nullopt;
            putByte(app, car + wheel + kCarMarks, 0);
            put(app, skid, 0);
        }
    }
    const Calling calling(ts);
    return skidSoundsOf(app, ts, car);
}

/* 0x42a560: wheelEffectsOf car eax. */
nfs::u32 wheelEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return wheelEffectsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* A shake's spring at `field` held to `limit` either way, as the code holds it: under -limit (or
 * a NaN) -limit, over limit limit, else itself -- its bits, read again for the second compare. */
u32 heldTo(App* app, u32 field, float limit, const Float& first)
{
    const float below = float(-Float(limit));
    if (fBelow(first, Float(below)))
        return bitsOf(below);
    return fAbove(f32(app, field), Float(limit)) ? bitsOf(limit) : at(app, field);
}

/* An x87 fabs in the code's two steps: only a value under 0 is negated, the rest kept to the bit. */
float sizeOf(float value)
{
    return fAbove(0.0, Float(value)) ? float(-Float(value)) : value;
}

/* The rumble's random part on a shake (0x4eb230, a fraction; its code keeps eax): the fraction by
 * the rumble less the rumble by 0x5372b8, by `share` (and by the driver's +0x50 when `byDriver`),
 * added to the shake at `field`.  Nullopt when the game stopped. */
bool rumbleOn(App* app, ThreadState& ts, u32 car, u32 field, u32 share, bool byDriver)
{
    const std::optional<Float> random = guest::direct<0x4eb230, ea::randomFractionOf>(app, ts);
    if (!random)
        return false;
    const Float fraction = *random;
    Float rumble = fp::sub(fp::mul(fraction, f32(app, car + kCarRumble)),
                           fp::mul(f32(app, car + kCarRumble), f64(app, kRumbleHalf)));
    rumble = fp::mul(rumble, f64(app, share));
    if (byDriver)
        rumble = fp::mul(rumble, f32(app, at(app, car + kCarDriver) + kDriverShake));
    putFloat(app, field, float(fp::add(rumble, f32(app, field))));
    return true;
}

/* 0x42a7d0's: the car's camera shake (+0x920 across, +0x924 up, +0x928 and +0x92c their
 * springs): the springs let go by 0x537298 and pulled by the car's turn (+0x4e0, +0x4e8) less
 * gravity on its axes, past 0x5372a0 (0x5372b0) by its share, held to the shake's size (the
 * tuning by the handling's); racing, the crash waves added (shakeWaveOf), each held, the road's
 * rumble (shakeAmplitudeOf, rumbleOn) on top -- by the driver's share but in a series or a race of
 * more than one player.  None for a wreck or high above (+0x84 60 or more, by its bits).  The
 * waves' frames run down, two a frame for a light car (its shake under 0.5).  eax as the code
 * leaves it; nullopt when the game stopped. */
std::optional<u32> updateShakeOf(App* app, ThreadState& ts, u32 car)
{
    const u32 across = car + kCarShake, up = across + 4, acrossSpring = across + 8, upSpring = across + 0xc;
    u32 left = 0;
    bool shaking = false;
    if (i32(at(app, car + kCarAltitude)) < i32(0x42700000) && (byte(app, car + kCarFlags) & kWreck) == 0)
    {
        const u32 handling = at(app, car + kCarHandling);
        const Float acrossSize = fp::mul(f32(app, kShakeAcross), f32(app, handling + kHandlingShake));
        const float turnAcross = f32At(app, car + kCarTurn), turnAhead = f32At(app, car + kCarTurn + 8);
        const Float upSize = fp::mul(f32(app, kShakeUp), f32(app, handling + kHandlingShake));
        const Float gravity = f32(app, kShakeGravity);
        const Float gravityAhead = fp::mul(f32(app, car + kCarAxes + 0x1c), gravity);
        const Float gravityAcross = fp::mul(gravity, f32(app, car + kCarAxes + 4));
        const Float acrossLet = fp::mul(f32(app, acrossSpring), f64(app, kSpringKeep));
        const float acrossLimit = float(acrossSize), upLimit = float(upSize);
        const float pullAhead = float(fp::sub(Float(turnAhead), gravityAhead));
        const float pullAcross = float(fp::sub(Float(turnAcross), gravityAcross));
        putFloat(app, acrossSpring, float(acrossLet));
        if (fAbove(Float(sizeOf(pullAhead)), f64(app, kAcrossPullFrom)))
            putFloat(app, acrossSpring,
                     float(fp::sub(f32(app, acrossSpring),
                                   fp::mul(fp::mul(Float(pullAhead), Float(acrossLimit)), f32(app, kPullShare)))));
        const u32 acrossHeld = heldTo(app, acrossSpring, acrossLimit, f32(app, acrossSpring));
        const Float upLet = fp::mul(f32(app, upSpring), f64(app, kSpringKeep));
        put(app, acrossSpring, acrossHeld);
        putFloat(app, upSpring, float(upLet));
        if (fAbove(Float(sizeOf(pullAcross)), f64(app, kUpPullFrom)))
            putFloat(app, upSpring,
                     float(fp::add(fp::mul(fp::mul(Float(pullAcross), Float(upLimit)), f32(app, kPullShare)),
                                   f32(app, upSpring))));
        left = heldTo(app, upSpring, upLimit, f32(app, upSpring));
        put(app, upSpring, left);
        if (half(app, car + kCarStage) == 0)
        {
            shaking = true;
            putFloat(app, up, float(f32(app, upSpring)));
            for (const auto& [field, wave] : {std::pair<u32, u32>{up, kCarWaves + kWaveSize}, {across, kCarWaves}})
            {
                if (field == across)
                    putFloat(app, across, float(f32(app, acrossSpring)));
                const u32 frames = at(app, car + wave + kWaveFrames);
                if (frames == 0)
                    continue;
                const std::optional<Float> added = shakeWaveOf(app, ts, f32At(app, car + wave), i32(frames));
                if (!added)
                    return std::nullopt;
                putFloat(app, field, float(fp::add(*added, f32(app, field))));
            }
            const u32 acrossShake = heldTo(app, across, upLimit, f32(app, across));
            const Float upFirst = f32(app, up);
            put(app, across, acrossShake);
            put(app, up, heldTo(app, up, upLimit, upFirst));
            const std::optional<u32> rumbled = shakeAmplitudeOf(app, ts, car);
            if (!rumbled)
                return std::nullopt;
            const u32 raceType = gameSettings(app).raceType;
            const bool byDriver = i32(gameSettings(app).playMode) <= 1 && raceType != 1 && raceType != 2;
            if (!rumbleOn(app, ts, car, across, kRumbleAcross, byDriver)
                || !rumbleOn(app, ts, car, up, kRumbleUp, byDriver))
                return std::nullopt;
            // eax as the code holds it across the fractions, which keep it: the driver's record by
            // the driver's share, else the amplitude's.
            left = byDriver ? at(app, car + kCarDriver) : *rumbled;
        }
    }
    if (!shaking)
    {
        put(app, car + kCarRumble, 0);
        put(app, across, at(app, car + kCarRumble));
        put(app, up, at(app, across));
        left = at(app, across);
    }
    // The waves' frames.
    const auto light = [&] { return i32(at(app, at(app, car + kCarHandling) + kHandlingShake)) < i32(0x3f000000); };
    const u32 acrossFrames = at(app, car + kCarWaves + kWaveFrames);
    if (i32(acrossFrames) > 0)
    {
        left = at(app, car + kCarHandling);
        if (light())
            put(app, car + kCarWaves + kWaveFrames, i32(acrossFrames) > 1 ? acrossFrames - 2 : acrossFrames - 1);
        else
        {
            left = acrossFrames - 1;
            put(app, car + kCarWaves + kWaveFrames, left);
        }
    }
    const u32 upFrames = at(app, car + kCarWaves + kWaveSize + kWaveFrames);
    if (i32(upFrames) <= 0)
        return left;
    left = at(app, car + kCarHandling);
    if (light())
    {
        if (i32(upFrames) > 1)
        {
            put(app, car + kCarWaves + kWaveSize + kWaveFrames, upFrames - 2);
            return left;
        }
        put(app, car + kCarWaves + kWaveSize + kWaveFrames, upFrames - 1);
        return upFrames - 1;
    }
    put(app, car + kCarWaves + kWaveSize + kWaveFrames, upFrames - 1);
    return left;
}

/* 0x42a7d0: updateShakeOf car eax. */
nfs::u32 updateShake(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return updateShakeOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* A burst at a place (0x4cae00, stdcall: the place's three words read from `place`, eax its
 * kind); sparks there (0x4cac20, eax the car, a fourth word: how many).  eax as the callee leaves it; nullopt when the
 * game stopped. */
std::optional<u32> burstAt(App* app, ThreadState& ts, u32 kind, u32 place)
{
    return feCore<0x4cae00, frontend::sime::crashPuff>(app, ts, kind, place);
}

std::optional<u32> sparksAt(App* app, ThreadState& ts, u32 car, u32 place, u32 size)
{
    const Pushed pushed(app, ts, {at(app, place), at(app, place + 4), at(app, place + 8), size});
    return feCore<kSparks, frontend::sime::sparks>(app, ts, car, pushed.address());
}

/* A crash's shake (startShakeOf) for rand's low 3 bits and `from` frames one way, 35 the other
 * (`acrossRandom` the across wave's the random one).  Nullopt when the game stopped. */
std::optional<u32> crashShake(App* app, ThreadState& ts, u32 car, u32 from, bool acrossRandom)
{
    const std::optional<i32> r = rand(app, ts);
    if (!r)
        return std::nullopt;
    const u32 frames = u32(*r % 8) + from;
    const Calling calling(ts);
    const std::optional<ShakeLeft> left =
        startShakeOf(app, ts, car, acrossRandom ? frames : kCrashShakeFrames, acrossRandom ? kCrashShakeFrames : frames);
    if (!left)
        return std::nullopt;
    return left->eax;
}

/* A crash's noise (0x4bd9d0, loopb's): its kind, the car's player. */
std::optional<u32> crashNoise(App* app, ThreadState& ts, u32 car, u32 kind)
{
    return feCore<0x4bd9d0, frontend::loopb::noteEvent>(app, ts, kind, u32(byte(app, car + kCarPlayer)));
}

/* 0x42ac40's: the car's crash (+0x170: its own kind in the high word -- 0x10000000 aside -- the
 * other's in the low byte): the force controller's jolt (0x471270 of the hit's place and
 * strength, for the force car, 0x471dc0, but a kind 6 crash); hit hard (over 5, by its bits):
 * pushed (the frames since put back by a 15th of the hit, wrecked or over 30 and not just put
 * back; the place hit seen from the car, its axes transposed into the frame, 0x4e0110, and
 * scaled, 0x4e0050), the shake (crashShake) and the crash's effects by its own kind (bursts,
 * sparks, noises), and its event (0x4284e0, the kind's channel); hit a car of kind 4 at the crash
 * speed or more its kind's effect (with the event of sparks for 3), landing hard or fast a burst
 * 6; low enough (+0x15c under 0x5372f0) and leaning, scraping the road its sound on and off
 * (+0x996), leaning past 0x537300 its sparks or effect where it touches (0x4e0050 and 0x4dffb0
 * into the frame), else a kind 3 crash hard enough the same at the crash's place.  eax as the
 * code leaves it; nullopt when the game stopped. */
std::optional<u32> crashEffectsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x70);
    const u32 kind = at(app, car + kCarCrash) & 0xefff0000;
    const u32 other = at(app, car + kCarCrash) & 0xff;
    const u32 point = car + kCarCrashPoint;
    const auto hit = [&] { return at(app, car + kCarHit); };
    const bool forced = guest::direct<0x471dc0, controls::forceCar>(app, ts, car);
    if (ts.terminate)
        return std::nullopt;
    if (forced && (hit() & 0x7fffffff) != 0 && kind != 0x60000)
    {
        if (!guest::direct<0x471270, controls::crashWith>(app, ts, car, at(app, point), at(app, point + 4),
                                                          at(app, point + 8), hit()))
            return std::nullopt;
    }
    // Its surface's record, as the code copies it into the frame: the scrape's kind (+0x14), its
    // sound (+0x1c).
    const u32 surface = kSurfaces + surfaceRecord(app, car);
    const u32 scrapeKind = at(app, surface + kSurfaceScrape), scrapeSound = at(app, surface + kSurfaceSound - kSurfaces);
    if (i32(hit()) > i32(0x40a00000))
    {
        u32 channel = at(app, u32(i32(kind) >> 16) * 4 + kCrashChannels);
        if (((byte(app, car + kCarFlags) & kWreck) != 0 || i32(hit()) > i32(0x41f00000))
            && at(app, car + kCarSinceRespawn) == 0)
        {
            const auto pushed = [&] { return fp::toInteger<i32>(fp::mul(f32(app, car + kCarHit), f64(app, kPushByHit))); };
            i32 by = pushed();
            by = by > 0 ? pushed() : -pushed();
            put(app, car + kCarSinceRespawn, at(app, car + kCarSinceRespawn) + u32(by));
            const u32 offset = car + kCarRespawnOffset;
            const Float dx = fp::sub(f32(app, point), f32(app, car + kCarPlace));
            const Float dy = fp::sub(f32(app, point + 4), f32(app, car + kCarPlace + 4));
            const Float dz = fp::sub(f32(app, point + 8), f32(app, car + kCarPlace + 8));
            putFloat(app, offset, float(dx));
            putFloat(app, offset + 4, float(dy));
            putFloat(app, offset + 8, float(dz));
            const u32 axes = frame - 0x4c;
            if (!feCore<0x4e0110, frontend::simd::transpose>(app, ts, car + kCarAxes, axes))
                return std::nullopt;
            Float seen[3];
            for (u32 k = 0; k < 3; ++k)
                seen[k] = fp::mul(f32(app, offset), f32(app, axes + 4 * k));
            for (u32 row = 1; row < 3; ++row)
            {
                Float by3[3];
                for (u32 k = 0; k < 3; ++k)
                    by3[k] = fp::mul(f32(app, offset + 4 * row), f32(app, axes + 0xc * row + 4 * k));
                for (u32 k = 0; k < 3; ++k)
                    seen[k] = fp::add(seen[k], by3[k]);
            }
            for (u32 k = 0; k < 3; ++k)
                putFloat(app, offset + 4 * k, float(seen[k]));
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, offset, floatOf(kPushScale), offset);
            if (ts.terminate)
                return std::nullopt;
        }
        u32 sound = 1;
        switch (kind)
        {
        case 0x40000:
            sound = at(app, other * 4 + kCrashSounds);
            if (!crashShake(app, ts, car, 0x10, true) || !burstAt(app, ts, at(app, other * 4 + kCrashEffects), point)
                || !crashNoise(app, ts, car, 5))
                return std::nullopt;
            break;
        case 0x30000:
            sound = scrapeSound;
            if (i32(at(app, car + kCarLean)) < i32(0x3f666666))
                channel = 5;
            if (!crashShake(app, ts, car, 0x10, false) || !crashNoise(app, ts, car, 3))
                return std::nullopt;
            break;
        case 0x20000:
            if (!crashShake(app, ts, car, 0x18, true))
                return std::nullopt;
            sound = scrapeSound;
            break;
        case 0x10000:
            sound = scrapeSound;
            if (scrapeKind == 3)
            {
                if (!sparksAt(app, ts, car, point, at(app, car + kCarSpeed)))
                    return std::nullopt;
            }
            else if (!burstAt(app, ts, scrapeKind, point))
                return std::nullopt;
            if (!crashShake(app, ts, car, 0x10, false) || !crashNoise(app, ts, car, 2))
                return std::nullopt;
            break;
        case 0x50000:
        {
            if (!crashShake(app, ts, car, 0x18, false))
                return std::nullopt;
            const u32 many = bitsOf(float(fp::mul(f32(app, car + kCarHit), f64(app, kSparksByHit))));
            if (!sparksAt(app, ts, car, point, many)
                || !burstAt(app, ts, 2, point))
                return std::nullopt;
            sound = other;
            if (!crashNoise(app, ts, car, 4))
                return std::nullopt;
            break;
        }
        case 0x60000:
            sound = other;
            if (other != 0xb && other != 9
                && (!crashShake(app, ts, car, 0x10, false) || !burstAt(app, ts, 2, point)))
                return std::nullopt;
            break;
        default:
            break;
        }
        if (!carEvent(app, ts, car, 1, 0xffffffff, channel,
                      {sound, bitsOf(float(fp::mul(f32(app, car + kCarHit), f32(app, kCrashLoudness)))), 0}))
            return std::nullopt;
    }
    // What this part leaves in eax is overwritten by the next.
    if ((hit() & 0x7fffffff) != 0 && kind == 0x40000)
    {
        std::optional<u32> shown;
        bool showing = true;
        if (!fBelow(f32(app, car + kCarSpeed), f32(app, kCrashSpeed)))
        {
            if (i32(at(app, car + kCarAboveRoad)) < i32(0x40400000))
            {
                const u32 effect = at(app, other * 4 + kCrashEffects);
                if (effect == 3)
                {
                    const u32 many = bitsOf(float(fp::mul(f32(app, car + kCarSpeed), f32(app, kSparksBySpeedHit))));
                    shown = sparksAt(app, ts, car, point, many);
                    if (shown)
                        shown = carEvent(app, ts, car, 1, 0xffffffff, 5, {0x12, 0, 0});
                }
                else
                    shown = burstAt(app, ts, effect, point);
            }
            else
                shown = burstAt(app, ts, 2, point);
        }
        else
        {
            if (fBelowOrEqual(f32(app, car + kCarSpeed), f64(app, kLandingHard)))
                showing = !fBelowOrEqual(f32(app, car + kCarHit), f64(app, kLandingHard));
            if (showing)
                shown = burstAt(app, ts, 6, point);
        }
        if (showing && !shown)
            return std::nullopt;
    }
    const auto scrapeOff = [&]() -> bool {
        if (byte(app, car + kCarScrapes + 2) == 0)
            return true;
        if (!carEvent(app, ts, car, 5, 0xa, 5, {1, 0, 0}))
            return false;
        putByte(app, car + kCarScrapes + 2, 0);
        return true;
    };
    if (fBelow(f32(app, car + kCarAboveRoad), f64(app, kScrapeHeight)) && (at(app, car + kCarLean) & 0x7fffffff) != 0)
    {
        if (fAbove(f32(app, car + kCarSpeed), f64(app, kSparksByHit))
            && fBelow(f32(app, car + kCarLean), f64(app, kScrapeLean)))
        {
            if (!carEvent(app, ts, car, 4, 0xa, 5, {scrapeSound, at(app, car + kCarSpeed), 0}))
                return std::nullopt;
            putByte(app, car + kCarScrapes + 2, 1);
        }
        else if (!scrapeOff())
            return std::nullopt;
        if (fBelow(f32(app, car + kCarLean), f64(app, kTouchLean)))
        {
            if (i32(at(app, car + kCarSpeed)) <= i32(0x40000000))
                return car;
            const u32 down = frame - 0x1c, touch = frame - 0x28;
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, car + kCarUp,
                                                              f32At(app, car + kCarHalfSizes + 4), down);
            if (ts.terminate)
                return std::nullopt;
            guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, car + kCarPlace, down, touch);
            if (ts.terminate)
                return std::nullopt;
            if (scrapeKind == 3)
                return sparksAt(app, ts, car, touch, at(app, car + kCarSpeed));
            return burstAt(app, ts, scrapeKind, touch);
        }
        if ((hit() & 0x7fffffff) == 0 || kind != 0x30000)
            return car;
        const float sum = float(fp::add(f32(app, car + kCarSpeed), f32(app, car + kCarHit)));
        if (fBelowOrEqual(Float(sum), f32(app, kScrapeHit)))
            return statusInto(car, Float(sum), f32(app, kScrapeHit));
        if (scrapeKind == 3)
            return sparksAt(app, ts, car, point, bitsOf(sum));
        return burstAt(app, ts, scrapeKind, point);
    }
    if (!scrapeOff())
        return std::nullopt;
    return car;
}

/* 0x42ac40: crashEffectsOf car eax. */
nfs::u32 crashEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return crashEffectsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42b480's: the car just put back (the frames since counting up from 1): a sparkle where it is
 * (+0x8d8 by its axes, on its place: 0x4caea0 past 64 frames, else 0x4cae00 with 9 past 16, 6
 * before) unless 0x4396e0 says no of its driver's +0x14 (0 to 49); the count ended once it is
 * settled: racing, leaning 0.5 or more (by its bits), its spin about +0xe4 and +0xec each under
 * 0x537310.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> respawnEffectsOf(App* app, ThreadState& ts, u32 car)
{
    const u32 since = at(app, car + kCarSinceRespawn);
    if (i32(since) > 0)
        put(app, car + kCarSinceRespawn, since + 1);
    u32 left = car;
    if (at(app, car + kCarSinceRespawn) != 0)
    {
        left = at(app, car + kCarDriver);
        bool shown = true;
        const u32 index = at(app, at(app, car + kCarDriver) + kDriverIndex);
        if (i32(index) >= 0 && i32(index) <= 0x31)
        {
            const std::optional<u32> no =
                guest::direct<0x4396e0, load::makerFlag>(app, ts, at(app, at(app, car + kCarDriver) + kDriverIndex));
            if (!no)
                return std::nullopt;
            left = *no;
            if (*no != 0)
                shown = false;
        }
        if (shown)
        {
            const u32 offset = car + kCarRespawnOffset, axes = car + kCarAxes;
            Float turned[3];
            for (u32 k = 0; k < 3; ++k)
                turned[k] = fp::mul(f32(app, offset), f32(app, axes + 4 * k));
            for (u32 row = 1; row < 3; ++row)
            {
                Float by[3];
                for (u32 k = 0; k < 3; ++k)
                    by[k] = fp::mul(f32(app, offset + 4 * row), f32(app, axes + 0xc * row + 4 * k));
                for (u32 k = 0; k < 3; ++k)
                    turned[k] = fp::add(turned[k], by[k]);
            }
            u32 where[3];
            for (u32 k = 0; k < 3; ++k)
                where[k] = bitsOf(float(fp::add(Float(float(turned[k])), f32(app, car + kCarPlace + 4 * k))));
            const u32 frames = at(app, car + kCarSinceRespawn);
            std::optional<u32> sparkled;
            {
                const Pushed pushed(app, ts, {where[0], where[1], where[2]});
                if (i32(frames) > 0x40)
                    sparkled = feCore<kSparkle, frontend::sime::respawnSparkle>(app, ts, car, pushed.address());
                else
                    sparkled = feCore<kSparkleBurst, frontend::sime::crashPuff>(app, ts, i32(frames) > 0x10 ? 9u : 6u,
                                                                               pushed.address());
            }
            if (!sparkled)
                return std::nullopt;
            left = *sparkled;
        }
    }
    if (at(app, car + kCarSinceRespawn) == 0 || half(app, car + kCarStage) > 0
        || i32(at(app, car + kCarLean)) < i32(0x3f000000))
        return left;
    for (const u32 about : {car + kCarSpin, car + kCarSpin + 8})
    {
        left = statusInto(left, 0.0, f32(app, about));
        if (!fAbove(0.0, f32(app, about)))
            left = at(app, about);
        const float size = sizeOf(f32At(app, about));
        left = statusInto(left, Float(size), f64(app, kSettledSpin));
        if (!fBelow(Float(size), f64(app, kSettledSpin)))
            return left;
    }
    put(app, car + kCarSinceRespawn, 0);
    return left;
}

/* 0x42b480: respawnEffectsOf car eax. */
nfs::u32 respawnEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return respawnEffectsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42b660's: the car's wheels' marks cut (0x4db860), its skid and scrape sounds stopped (0x4284e0
 * with 3, 8, 6; 5, 10, 6; 5, 10, 5 and 1, 0, 0), its shake cleared.  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<u32> stopCarEffectsOf(App* app, ThreadState& ts, u32 car)
{
    for (u32 wheel = 0; wheel < 4; ++wheel)
    {
        if (!feCore<0x4db860, frontend::sime::wheelMarkCut>(app, ts, at(app, car + kCarNumber), wheel))
            return std::nullopt;
        putByte(app, car + wheel + kCarMarks, 0);
        putByte(app, car + wheel + kCarMarkSounds, 0);
    }
    u32 left = car + 3;
    struct Scrape
    {
        u32 edx, ebx, ecx;
    };
    static constexpr Scrape kScrapes[] = {{3, 8, 6}, {5, 0xa, 6}, {5, 0xa, 5}};
    for (u32 k = 0; k < 3; ++k)
    {
        if (byte(app, car + kCarScrapes + k) == 0)
            continue;
        const std::optional<u32> stopped =
            carEvent(app, ts, car, kScrapes[k].edx, kScrapes[k].ebx, kScrapes[k].ecx, {1, 0, 0});
        if (!stopped)
            return std::nullopt;
        left = *stopped;
        putByte(app, car + kCarScrapes + k, 0);
    }
    for (u32 k = 0; k < 0x14; k += 4)
        put(app, car + kCarShake + k, 0);
    put(app, car + kCarWaves + kWaveFrames, 0);
    put(app, car + kCarWaves + kWaveSize + kWaveFrames, 0);
    put(app, car + kCarWaves, 0);
    put(app, car + kCarWaves + kWaveSize, 0);
    put(app, car + kCarShakeToo, 0);
    return left;
}

/* 0x42b660: stopCarEffectsOf car eax. */
nfs::u32 stopCarEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return stopCarEffectsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42b780's: the car's effects a frame: respawnEffectsOf, crashEffectsOf, wheelEffectsOf,
 * updateShakeOf.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> carEffectsOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 8, 0);  // as deep as the code's, for its callees' frames
    const Calling calling(ts);
    if (!respawnEffectsOf(app, ts, car))
        return std::nullopt;
    if (!crashEffectsOf(app, ts, car) || !wheelEffectsOf(app, ts, car))
        return std::nullopt;
    return updateShakeOf(app, ts, car);
}

/* 0x42b780: carEffectsOf car eax. */
nfs::u32 carEffects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return carEffectsOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42b7b0's: the car in the air (+0x8b), not to be put back, not crashing, falling (its
 * velocity against its ground's up, 0x4e01f0, over 0): its spin as its axes see it (+0x4f8..
 * +0x500) taken by the air's tuning -- the player's steered by +0x4e0, +0x4e8 -- and turned back
 * into the world's (by the axes transposed into the frame, 0x4e0110).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<u32> airborneSpinOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0x24);
    if (byte(app, car + kCarInAir) == 0 || at(app, car + kCarRespawnSlice) != 0
        || (at(app, car + kCarHit) & 0x7fffffff) != 0)
        return car;
    const std::optional<Float> falling = dot(app, ts, car + kCarVelocity, car + kCarGroundUp);
    if (!falling)
        return std::nullopt;
    // eax as the dot product leaves it: its first vector.
    if (!fBelow(0.0, *falling))
        return statusInto(car + kCarVelocity, 0.0, *falling);
    const u32 spin = car + kCarSpin, seen = car + kCarSpinSeen;
    for (u32 k = 0; k < 8; k += 4)
    {
        const std::optional<Float> part = dot(app, ts, spin, car + kCarAxes + 3 * k);
        if (!part)
            return std::nullopt;
        putFloat(app, seen + k, float(*part));
    }
    const std::optional<Float> ahead = dot(app, ts, spin, car + kCarAxes + 0x18);
    if (!ahead)
        return std::nullopt;
    const Float acrossSpin = fp::mul(f32(app, kAirSpinX), f32(app, seen));
    putFloat(app, seen + 8, float(*ahead));
    const u8 flags = byte(app, car + kCarFlags);
    const Float aheadSpin = fp::mul(f32(app, kAirSpinZ), f32(app, seen + 8));
    putFloat(app, seen, float(acrossSpin));
    putFloat(app, seen + 8, float(aheadSpin));
    if ((flags & kThisMachines) != 0)
    {
        const Float steerAhead = fp::mul(f32(app, car + kCarTurn), f32(app, kAirSteerZ));
        const Float steerAcross = fp::mul(f32(app, car + kCarTurn + 8), f32(app, kAirSteerX));
        const Float newAhead = fp::sub(f32(app, seen + 8), steerAhead);
        const Float newAcross = fp::add(steerAcross, f32(app, seen));
        putFloat(app, seen + 8, float(newAhead));
        putFloat(app, seen, float(newAcross));
    }
    const u32 axes = frame - 0x24;
    if (!feCore<0x4e0110, frontend::simd::transpose>(app, ts, car + kCarAxes, axes))
        return std::nullopt;
    // The last one's eax is what the code leaves: its first vector.
    for (u32 k = 0; k < 0xc; k += 4)
    {
        const std::optional<Float> part = dot(app, ts, seen, axes + 3 * k);
        if (!part)
            return std::nullopt;
        putFloat(app, spin + k, float(*part));
    }
    return seen;
}

/* 0x42b7b0: airborneSpinOf car eax. */
nfs::u32 airborneSpin(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return airborneSpinOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42b900's: the car's frame of race state: its reset's wait run down; while it is waiting to
 * start (+0x88, or not +0x89) put back on its slice if stuck or asked (respawnCarOf), its effects
 * stopped, its crash cleared (0x20 bytes at +0x168); else its wheels on the track (wheelCellsOf),
 * its crash's response (0x406120, 0x404a00), its spin in the air, the objects, the stuck count,
 * its effects, and its crash cleared.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<u32> updateCarOf(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);  // as deep as the code's, for its callees' frames
    const u32 wait = at(app, car + kCarResetWait);
    put(app, car + 0x7dc, 0);
    if (wait != 0)
        put(app, car + kCarResetWait, wait - 1);
    if (byte(app, car + kCarOut) != 0 || byte(app, car + kCarIn) == 0)
    {
        if (i32(at(app, car + kCarStuck)) > 0 || at(app, car + kCarRespawnSlice) != 0)
        {
            put(app, car + kCarStuck, 0);
            const Calling calling(ts);
            if (!respawnCarOf(app, ts, car))
                return std::nullopt;
        }
        put(app, car + kCarSinceRespawn, 0);
        {
            const Calling calling(ts);
            if (!stopCarEffectsOf(app, ts, car))
                return std::nullopt;
        }
        put(app, car + kCarResetWait, 0);
        guest::direct<kFill, ea::clearBytes>(app, ts, car + kCarHit, 0x20u);
        if (ts.terminate)
            return std::nullopt;
        return fillEnd(car + kCarHit, 0x20);
    }
    {
        const Calling calling(ts);
        if (!wheelCellsOf(app, ts, car))
            return std::nullopt;
    }
    if ((at(app, car + kCarHit) & 0x7fffffff) != 0 && at(app, car + kCarHit + 4) != 0)
    {
        guest::direct<0x406120, aidrive::reverseWhenSlowDone>(app, ts, car);
        if (ts.terminate || !guest::direct<0x404a00, airace::carContactDone>(app, ts, car))
            return std::nullopt;
    }
    const Calling calling(ts);
    if (!airborneSpinOf(app, ts, car) || !objectContactsOf(app, ts, car) || !stuckCheckOf(app, ts, car))
        return std::nullopt;
    const std::optional<u32> left = carEffectsOf(app, ts, car);
    if (!left)
        return std::nullopt;
    put(app, car + kCarHit, 0);
    put(app, car + kCarHit + 4, 0);
    put(app, car + kCarCrash, 0);
    return left;
}

/* 0x42b900: updateCarOf car eax. */
nfs::u32 updateCar(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return updateCarOf(app, ts, args.eax); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* A slice's half width across (a byte at +0x17 the one side's, at +0x1b the other's, as 16.16
 * halved the way the code halves it: towards 0). */
u32 halfWidth(App* app, u32 slicePointer, u32 field)
{
    const u32 width = u32(byte(app, at(app, kSlices) + at(app, slicePointer) * kSliceSize + field)) << 15;
    return u32(i32(width - u32(i32(width) >> 31)) >> 1);
}

/* 0x42ba00's: grid place `place` worked out: its slice to [slice] -- 3 a row from the start,
 * back from the last car's when the race runs backwards -- and its place to `where`: across by
 * the slice's half widths (one side or the other by the place's odd or even, swapped backwards;
 * where 0x40eca0 says the road is wide enough for two, by half its lanes' widths and the half
 * width again), 0x8000 up, 0 ahead.  What the code leaves in eax and edx; nullopt when the game
 * stopped. */
std::optional<Left> defaultGridPlaceOf(App* app, ThreadState& ts, u32 place, u32 slice, u32 where)
{
    const u32 backwards = gameSettings(app).backwards;
    u32 row;
    if (backwards != 0)
    {
        const u32 from = gameSettings(app).humanCount + gameSettings(app).opponentCount - 1 - place;
        row = from * 4 - from;
    }
    else
        row = place * 4 - place;
    put(app, slice, row + 1);
    u32 side = place & 1;
    if (backwards != 0)
        side = side == 0 ? 1 : 0;
    const u32 wide = guest::direct<0x40eca0, aiplan::trackRoadWide>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    u32 across, second;  // and what the code leaves in edx
    if (wide != 0)
    {
        const u32 record = at(app, kSlices) + at(app, slice) * kSliceSize;
        const u32 lanes = byte(app, record + kSliceLanes) & 0xf;
        const u32 width = u32(byte(app, record + kSliceRightWidth)) << 15;
        const u32 lanesAcross = u32(i32(lanes) >> 1) * width;
        across = u32(i32(width - u32(i32(width) >> 31)) >> 1);
        if (side != 0)
            across += width;
        put(app, where, across);
        if (side != 0)
        {
            across = halfWidth(app, slice, kSliceRightWidth) + lanesAcross;
            second = 0;
        }
        else
        {
            across = lanesAcross - halfWidth(app, slice, kSliceRightWidth);
            second = across;
        }
    }
    else
    {
        const u32 record = at(app, kSlices) + at(app, slice) * kSliceSize;
        const u32 width = side != 0 ? u32(byte(app, record + kSliceRightWidth)) << 15
                                      : 0u - (u32(byte(app, record + kSliceLeftWidth)) << 15);
        second = u32(i32(width) >> 31);
        across = u32(i32(width - second) >> 1);
    }
    put(app, where, across);
    put(app, where + 8, 0);
    put(app, where + 4, kGridUp);
    return Left{across, second};
}

/* 0x42ba00: defaultGridPlaceOf place eax, to [edx] and [ebx]. */
nfs::u32 defaultGridPlace(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<Left> left =
        keeping(ts, [&] { return defaultGridPlaceOf(app, ts, args.eax, args.edx, args.ebx); });
    if (!left)
        return args.eax;
    args.eax = left->eax;
    args.edx = left->edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x42bb80's: the start's grid, 12 places of 3 words at kGrid: from the track's start.dat
 * (kTrackFolder + "TRK%03x\start.dat" of the track, sprintf to a buffer of the frame's; whether
 * it is there 0x4e0e60, opened 0x4dc0e0, read a word at a time 0x4dc1e0, closed 0x4dc1b0), else
 * each worked out (defaultGridPlaceOf, into words of the frame's).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<u32> loadStartGridOf(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0x210);
    const u32 path = frame - 0x210;
    const u32 track = gameSettings(app).track;
    guest::direct<0x4df690, watcom::clib::sprintf>(app, ts, path, kStartFile, guest::Words{kTrackFolder, track});
    if (ts.terminate)
        return std::nullopt;
    const std::optional<bool> there = guest::direct<0x4e0e60, ea::fileThere>(app, ts, path);
    if (!there)
        return std::nullopt;
    if (*there)
    {
        const std::optional<u32> file = feCore<0x4dc0e0, frontend::eaa::udffOpen>(app, ts, path, 0u, 1u);
        if (!file)
            return std::nullopt;
        for (u32 place = 0; place < kGridPlaces; ++place)
        {
            const u32 entry = place * 3 * 4;
            for (const u32 word : {kGrid, kGridY, kGridZ})
            {
                const std::optional<u32> value = feCore<0x4dc1e0, frontend::eaa::udffReadWord>(app, ts, *file);
                if (!value)
                    return std::nullopt;
                put(app, entry + word, *value);
            }
        }
        return feCore<0x4dc1b0, frontend::eaa::udffClose>(app, ts, *file);
    }
    const u32 slice = frame - 4, where = frame - 0x10;
    u32 left = 0;
    for (u32 place = 0; place < kGridPlaces; ++place)
    {
        const Calling calling(ts);
        if (!defaultGridPlaceOf(app, ts, place, slice, where))
            return std::nullopt;
        const u32 entry = place * 3 * 4;
        put(app, entry + kGrid, at(app, slice));
        put(app, entry + kGridY, at(app, where));
        left = at(app, where + 8);
        put(app, entry + kGridZ, left);
    }
    return left;
}

/* 0x42bb80: loadStartGridOf. */
nfs::u32 loadStartGrid(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = keeping(ts, [&] { return loadStartGridOf(app, ts); });
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x42bc70's: grid place `place`: its slice to [slice], its place to `where` -- across, 0x8000
 * up, ahead -- across mirrored when kGridMirrored is 1.  What the code leaves in eax and edx:
 * the place's ahead, the mirrored flag. */
Left gridPlaceOf(App* app, u32 place, u32 slice, u32 where)
{
    const u32 entry = place * 3 * 4;
    put(app, slice, at(app, entry + kGrid));
    put(app, where, at(app, entry + kGridY));
    const u32 ahead = at(app, entry + kGridZ);
    put(app, where + 4, kGridUp);
    const u32 mirrored = at(app, kGridMirrored);
    put(app, where + 8, ahead);
    if (mirrored == 1)
        put(app, where, 0u - at(app, where));
    return {ahead, mirrored};
}

/* 0x42bc70: gridPlaceOf place eax, to [edx] and [ebx]. */
nfs::u32 gridPlace(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Left left = gridPlaceOf(app, args.eax, args.edx, args.ebx);
    args.eax = left.eax;
    args.edx = left.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x42bcb0's: the car put on its grid place and its race state set to the start's: its handling
 * scaled for the players (0x4a0d20, with `number` its first word: the handling's +4 by the scale,
 * a player's at this machine by 1 or kPlayerScale (the hazard race, chased in a one-player race)
 * with its driver's three words (+0x5c, from the stuck record's +0x28 with the start records'
 * flag), the others' by 1 over kHumanScale for a player's car with the stuck record's three
 * words), its place (a chasing player's at this machine 0x404d30's, a wreck's or a chased car's
 * slice 100, else its grid slot's, gridPlaceOf, in the frame), put there (0x4a0aa0), its slice,
 * every count, flag and effect cleared (fills of 0x4e070c too, 0x4a33d0 between), the stuck
 * record kept (+0x8a4 and its 12 bytes after), its lights on in the weather and at night (+0x660,
 * 2 the player's at this machine).  False when the game stopped. */
bool placeOnGridOf(App* app, ThreadState& ts, u32 car, u32 number)
{
    const GuestFrame frame(ts, 0x14, 0x2c);
    const u32 place = frame - 0x2c, slice = frame - 0x14;
    u32 words[3];
    float scaled;
    if ((byte(app, car + kCarFlags) & kThisMachines) != 0)
    {
        u32 scale = kOneBits;
        if (gameSettings(app).raceOptionHazard != 0)
            scale = at(app, kPlayerScale);
        if (gameSettings(app).playMode == 0 && (byte(app, car + kCarFlags) & kChased) != 0)
            scale = at(app, kPlayerScale);
        if ((std::uint8_t(gameSettings(app).raceFlags) & kStartRecords) != 0)
            for (u32 k = 0; k < 0xc; k += 4)
                put(app, at(app, car + kCarDriver) + kDriverStart + k,
                    at(app, at(app, car + kCarStuckRecord) + kRecordStart + k));
        const u32 driver = at(app, car + kCarDriver);
        for (u32 k = 0; k < 3; ++k)
            words[k] = at(app, driver + kDriverStart + 4 * k);
        scaled = float(fp::mul(f32(app, at(app, car + kCarHandling) + kHandlingScaled), Float(floatOf(scale))));
    }
    else
    {
        float scale = 1.0f;
        if ((byte(app, car + kCarFlags) & kPlayersCar) != 0)
            scale = float(fp::div(1.0, f32(app, kHumanScale)));
        const u32 record = at(app, car + kCarStuckRecord);
        for (u32 k = 0; k < 3; ++k)
            words[k] = at(app, record + kRecordStart + 4 * k);
        scaled = float(fp::mul(f32(app, at(app, car + kCarHandling) + kHandlingScaled), Float(scale)));
    }
    {
        const Pushed pushed(app, ts, {bitsOf(scaled), words[0], words[1], words[2]});
        if (!guest::direct<kScaleHandling, port::conv::scaleHandling>(app, ts, car, number, pushed.address()))
            return false;
    }
    const u8 flags = byte(app, car + kCarFlags);
    if ((flags & kThisMachines) != 0 && (flags & kChased) != 0)
    {
        if (!guest::direct<0x404d30, aidrive::placeCopAtStart>(app, ts, car, slice, place))
            return false;
    }
    else if ((byte(app, car + kCarFlags) & (kWreck | kChased)) != 0)
    {
        for (u32 k = 0; k < 0xc; k += 4)
            put(app, place + k, 0);
        put(app, slice, 0x64);
    }
    else
    {
        const Calling calling(ts);
        gridPlaceOf(app, gameSettings(app).gridSlots[at(app, car + kCarIndex)], slice, place);
    }
    if (!feCore<0x4a0aa0, frontend::simb::placeOnRoad>(app, ts, car, at(app, slice), place,
                                                        u32(gameSettings(app).backwards)))
        return false;
    put(app, car + kCarLapState, 0);
    put(app, car + kCarBackwards, gameSettings(app).backwards != 0 ? 1 : 0);
    put(app, car + kCarStartSlice, at(app, car + kCarSlice));
    guest::direct<0x42ce10, raceload::setRaceDistance>(app, ts, car);
    if (ts.terminate)
        return false;
    // Its counts, flags and effects, in the code's order (a byte where it writes one).
    struct Cleared
    {
        u32 offset, value;
        bool isByte;
    };
    static constexpr Cleared kCleared[] = {
    {0x20c, 0x3f800000, false}, {0x210, 0, false}, {0x214, 0, false}, {0x2e4, 0, false},
    {0x218, 0x100, false}, {0x998, 0, false}, {0x1fc, 0, false}, {0x5f4, 0, false}, {0x2e8, 0, false},
    {0x2ec, 0, false}, {0x2f0, 0, false}, {0x2f4, 0, false}, {0x2f8, 0, false}, {0x2fc, 0, false},
    {0x4e0, 0, false}, {0x4e4, 0, false}, {0x4e8, 0, false}, {0x4ec, 0, false}, {0x4f0, 0, false},
    {0x4f4, 0, false}, {0x4f8, 0, false}, {0x4fc, 0, false}, {0x500, 0, false}, {0x590, 0, false},
    {0x594, 0, false}, {0x598, 0, false}, {0x59c, 0, false}, {0x5a0, 0, false}, {0x5a4, 0, false},
    {0x5a8, 0, false}, {0x5ac, 0, false}, {0x5b0, 0, false}, {0x5b4, 0, true}, {0x5b8, 0, false},
    {0x5bc, 0, false}, {0x5c0, 0, false}, {0x5c8, 0, false}, {0x5cc, 0, false}, {0x5d4, 0, false},
    {0x5d0, 0, false}, {0x5d8, 0, false}, {0x5dc, 0, false}, {0x5e4, 0, false}, {0x5e0, 0, false},
    {0x5e8, 0, false}, {0x5ec, 0, false}, {0x5f0, 0, false}, {0x7c0, 0, false}, {0x7c4, 0, false},
    {0x524, 0, false}, {0x504, 0, true}, {0x505, 0, true}, {0x506, 1, true}, {0x518, 0, false},
    {0x507, 0, true}, {0x508, 0, true}, {0x509, 0, true}, {0x51c, 0, false}, {0x50b, 1, true},
    {0x50a, 1, true}, {0x50d, 0, true}, {0x514, 0, false}, {0x50e, 0, true}, {0x50f, 0, true},
    {0x520, 0x100, false}, {0x95c, 0, true}, {0x95d, 0, true}, {0x95e, 0, true}, {0x95f, 0, true},
    {0x994, 0, true}, {0x995, 0, true}, {0x997, 0, true}, {0x94c, 0, false}, {0x5c4, 0, false},
    {0x950, 0, false}, {0x954, 0, false}, {0x958, 0, false}, {0x990, 0, true}, {0x991, 0, true},
    {0x992, 0, true}, {0x993, 0, true}, {0x5b4, 0, true}, {0x5b8, 0, false},
    };
    for (const Cleared& c : kCleared)
    {
        if (c.isByte)
            putByte(app, car + c.offset, u8(c.value));
        else
            put(app, car + c.offset, c.value);
    }
    struct Fill
    {
        u32 offset, size;
    };
    static constexpr Fill kFills[] = {{0x62c, 4}, {0x630, 0xac}, {0x6dc, 4}, {0x6e0, 0x2c}, {0x70c, 0xa4}};
    const auto fill = [&](u32 from, u32 bytes) {
        guest::direct<0x4e070c, ea::clearBytes>(app, ts, from, bytes);
        return !ts.terminate;
    };
    for (const Fill& f : kFills)
        if (!fill(car + f.offset, f.size))
            return false;
    if (!feCore<0x4a33d0, frontend::loada::stopCheck>(app, ts))
        return false;
    put(app, car + 0x7dc, 0);
    if (!fill(car + 0x7e0, 0x18))
        return false;
    put(app, car + 0x7d8, 0);
    put(app, car + kCarStuck, 0);
    const u32 record = at(app, car + kCarStuckRecord);
    u32 after[3];
    for (u32 k = 0; k < 3; ++k)
        after[k] = at(app, car + kCarStuckRecord + 4 + 4 * k);
    if (!fill(car + kCarStuckRecord, 0x104))
        return false;
    for (u32 k = 0; k < 3; ++k)
        put(app, car + kCarStuckRecord + 4 + 4 * k, after[k]);
    put(app, car + kCarLights, 0);
    put(app, car + kCarStuckRecord, record);
    if (gameSettings(app).weather != 0)
        put(app, car + kCarLights, 1);
    if (gameSettings(app).night != 0)
        put(app, car + kCarLights, (byte(app, car + kCarFlags) & kThisMachines) != 0 ? 2 : 1);
    put(app, car + 0x734, 0);
    return true;
}

/* 0x42bcb0: placeOnGridOf car eax, edx its number; eax and edx left as the car. */
nfs::u32 placeOnGrid(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 car = args.eax;
    if (!keeping(ts, [&] { return placeOnGridOf(app, ts, car, args.edx); }))
        return args.eax;
    args.eax = car;
    args.edx = car;
    ts.sp += 4;
    return args.eax;
}

}  // namespace racestart
}  // namespace nfs3hp
