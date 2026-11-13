#include "game_settings.h"
#include "native_ai_race.h"
#include "native_aidrive.h"
#include "native_aiplan.h"
#include "native_controls_race.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_trackdata.h"
#include "native_vertices.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <utility>

/* A car's race state between the frames' physics: put back on the road when
 * it is stuck or the player asks (respawnCar, resetCar, stuckCheck), its
 * wheels on the track (wheelCells, wheelSurface), their skid marks, dust and
 * sounds (wheelEffects and what it calls), the camera's shake (startShake,
 * updateShake), a crash's jolt, sounds and effects (crashEffects), and the
 * grid: the start's places, from TRKnnn\start.dat or worked out, and a car
 * put on its place with its race state cleared (loadStartGrid, placeOnGrid).
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (RACESTART_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
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
 * Each is a typed core (respawnCarOf, wheelSlipOf...) with an adapter under the game's name for
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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

// The race's.
constexpr reg32 kTrackFolder = 0x7a27b4;    // the tracks' folder, a string
constexpr reg32 kStartFile = 0x537318;      // "%sTRK%03x\start.dat"
// The track's slices: their count, their records (0x24 bytes each).
constexpr reg32 kSliceCount = 0x5dd940, kSlices = 0x5dd948;
// Putting a car back: slices on, the height above the road, whether a car may be.
constexpr reg32 kRespawnStep = 0x5efdcc, kRespawnHeight = 0x5efdd0, kResetAllowed = 0x7d3680;
// The stuck count's start, and the player's with kPlayerOption.
constexpr reg32 kStuckCount = 0x5efdc0, kPlayerStuckCount = 0x5efdc4;
// The shake's tuning, across and up; the spin in the air's (by the axes, by the steering).
constexpr reg32 kShakeAcross = 0x5efde0, kShakeUp = 0x5efde4;
constexpr reg32 kAirSpinZ = 0x5efda8, kAirSteerZ = 0x5efdac, kAirSpinX = 0x5efdb0, kAirSteerX = 0x5efdb4;
// The handling's scale on the grid: a player's, the human players'.
constexpr reg32 kHumanScale = 0x5efdb8, kPlayerScale = 0x5efdbc;
// A surface's record, 0x24 bytes, by the track (0x240) and the surface: the
// skid past which its effect shows, the speed its skid starts at and the
// speed over which it grows, its effect, whether it marks, its rumble, its
// sound, the back wheels' sound.
constexpr reg32 kSurfaces = 0x553080, kSurfaceSkidFrom = 0x553084, kSurfaceSkidOver = 0x553088;
constexpr reg32 kSurfaceEffect = 0x55308c, kSurfaceMarks = 0x553090, kSurfaceRumble = 0x553098;
constexpr reg32 kSurfaceSound = 0x55309c, kSurfaceRearSound = 0x5530a0;
// A crash's, by the other's kind (+0x170's low byte): the effect (3: by the
// speed), the sound; by its own kind (the high word): the channel; the speed
// past which its effect plays.
constexpr reg32 kCrashEffects = 0x553740, kCrashSounds = 0x553760, kCrashChannels = 0x553780;
constexpr reg32 kCrashSpeed = 0x5537b8;
// The grid: 12 places of a slice and a place (3 words); whether mirrored; the
// cars' places on it by their index (+0x1f8); the cars and the others on it.
constexpr reg32 kGrid = 0x5ef9b8, kGridY = 0x5ef9bc, kGridZ = 0x5ef9c0;
constexpr reg32 kGridMirrored = 0x55d000;

// A car's: the slice it is to be put back on (0 none), the frames since it was.
constexpr reg32 kCarRespawnSlice = 0x184, kCarSinceRespawn = 0x8d4;
// A car's: its speed, the track's index into the surfaces' table, its handling (+0x298 its
// rumble), its rumble.
constexpr reg32 kCarSpeed = 0xb8, kCarTrackSurfaces = 0x160, kCarHandling = 0x528, kCarRumble = 0x930;
constexpr reg32 kHandlingRumble = 0x298;
constexpr reg32 kTrackSurfacesSize = 0x240, kSurfaceSize = 0x24;
// A 64th (a float), a half (a double); 1.0's bits.
constexpr reg32 kSixtyFourth = 0x5371e0, kHalfTurn = 0x5371e8, kOneBits = 0x3f800000;
// A car's: how far it leans (+0x10c), its shake's two waves (an amplitude, a float, and the
// frames left, each).
constexpr reg32 kCarLean = 0x10c, kCarWaves = 0x8c4, kWaveSize = 8, kWaveFrames = 4;
// The lean under which a car shakes a little (a double, -0.9); the little shake's amplitude (a
// 240th, a float's bits); its frames, 24 and up to 11 more.
constexpr reg32 kIdleLean = 0x537200, kIdleAmplitude = 0x3b888889;
constexpr i32 kIdleFrames = 0x18, kIdleFramesSpread = 0xc;
// A twelfth (a float): the shake's wave's step a frame, and its fall.
constexpr reg32 kTwelfth = 0x537208;
// A car's: its flags (1 a player's, 4 the player's at this machine, 0x10 a wreck, 0x20 chased),
// its state (3 finished, 4 out), the flags of 0x621 (0x40 hit an object), what it hit.
constexpr reg32 kCarFlags = 0x200, kCarState = 0x2c4, kCarObjectFlags = 0x621, kCarHits = 0x99c;
constexpr reg8 kPlayersCar = 1, kThisMachines = 4, kWreck = 0x10, kChased = 0x20;
constexpr reg32 kFinished = 3, kOut = 4;
constexpr reg8 kHitObject = 0x40;
// The player's car against the scenery's objects; the object a car is against (its handle to
// [edx], 1 or 0 back); the object handed over; the car told of the hit (the handle in edx).
constexpr reg32 kPlayerObjects = 0x493ad0, kObjectAgainst = 0x4ca160, kObjectHit = 0x4c9f50, kCarHitObject = 0x401650;
// A car's: the race's stage (a word, 0 racing), its surface, its four wheels' cells on the track
// (0x78 bytes each: +0x14 and +0x18 whether on a piece of its own).
constexpr reg32 kCarStage = 0x164, kCarSurface = 0x18c, kCarWheels = 0x300, kWheelSize = 0x78;
constexpr reg32 kWheelPiece = 0x14, kWheelPieceToo = 0x18;
// The kind of polygon a place is on (trackdata::placeSurface): its low 6 bits the surface.
constexpr reg32 kSurfaceBits = 0x3f;
// A car's: its first word (its number, as the marks take it), its wheels' marks (a byte each,
// 1 drawing) and their sounds' (a byte each), its three scrape sounds' (bytes), its shake (five
// words from +0x920; +0x91c besides).
constexpr reg32 kCarNumber = 0, kCarMarks = 0x95c, kCarMarkSounds = 0x990, kCarScrapes = 0x994;
constexpr reg32 kCarShake = 0x920, kCarShakeToo = 0x91c;
// A wheel's mark cut (eax the car's number, edx the wheel); an event added to a car (eax the car,
// edx, ebx, ecx and three words: its fields, carEvent).
constexpr reg32 kCutMark = 0x4db860, kCarEvent = 0x4284e0;
// The grid's place's up (16.16).
constexpr reg32 kGridUp = 0x8000;

// A car's: its cell on the track (as a wheel's: +0x10 its slice, +0x14 and +0x18 on a piece of its
// own), its slice, its place (x, y up, z), its velocity, the frames it settles for, its spin,
// the vector +0x130 (the ground's up under it, as stuckCheck's dot reads it), its axes' up, how
// far over the road (+0x15c), the times it was put back, the reset's wait, its pursuer (Hot
// Pursuit), its stuck count, the throttle asked (a byte).
constexpr reg32 kCarCell = 0xc, kCarSlice = 0x1c, kCarPlace = 0x98, kCarVelocity = 0xa4, kCarSettleFrames = 0x174;
constexpr reg32 kCarSpin = 0xe4, kCarGroundUp = 0x130, kCarUp = 0xcc, kCarAboveRoad = 0x15c, kCarRespawns = 0x2c0;
constexpr reg32 kCarResetWait = 0x524, kCarPursuer = 0x624, kCarStuck = 0x8a0, kCarThrottleAsked = 0x504;
constexpr reg32 kHotPursuit = 3;
// A car put on a slice (eax the car, edx the slice); a car at rest settled; whether Hot Pursuit has
// a car out.
constexpr reg32 kPlaceOnSlice = 0x428d60, kSettle = 0x428970, kPursuitOut = 0x401500;
// The height in a tunnel by these (floats): a car put back, reset.
constexpr reg32 kHalfHeight = 0x5371d0, kResetHeight = 0x5371d4;
// A stuck car's spin's cut (a 64th), the upright its ground's up says (0.75); a dot product (eax,
// edx), onto the x87's stack.
constexpr reg32 kStuckSpinCut = 0x5371d8, kUpright = 0x5371dc, kDot = 0x4e01f0;

// The grid's places; a slice's record (0x24 bytes): its lanes (the low 4 bits of +0x13), its
// half widths across, one side's (+0x17) and the other's (+0x1b), bytes as the grid reads them.
constexpr reg32 kGridPlaces = 0xc, kSliceSize = 0x24, kSliceLanes = 0x13, kSliceLeftWidth = 0x17, kSliceRightWidth = 0x1b;
// The runtime's sprintf (cdecl); a file opened (eax the path, edx 0, ebx 1), a word read from it,
// closed.
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kOpenFile = 0x4dc0e0, kReadWord = 0x4dc1e0, kCloseFile = 0x4dc1b0;

// A car's: its axes' forward, its slides (whole), front and back, its wheels' skids (floats),
// their marks' last points (3 words each).  A wheel's cell's: its place, its slice, its heights
// (a pointer to 16-bit words).  A slice's heights (signed bytes, from +0x10).
constexpr reg32 kCarForward = 0xd8, kCarFrontSlide = 0x598, kCarBackSlide = 0x594, kCarSkids = 0x94c;
constexpr reg32 kCarLastPoints = 0x960, kWheelPlace = 4, kWheelSlice = 0x10, kWheelHeights = 0x1c;
constexpr reg32 kSliceHeights = 0x10;
// A mark's point drawn; a wheel's smoke (the place and the kind on the stack: 6).
constexpr reg32 kMarkPoint = 0x4db940, kWheelSmoke = 0x4cb010;
constexpr reg32 kSmoke = 6;
// The slip's (floats): its base, its share of the spin; the skid past which a back wheel smokes
// (a double, 5).
constexpr reg32 kSlipBase = 0x53720c, kSlipBySpin = 0x537210, kSmokingSkid = 0x537270;

// A car's: its axes (+0xc0 three vectors: across, up, forward), its half sizes (across, down,
// along).  The wheels' corners' shares of them (doubles: across 0.75, along a half); a 32nd (a
// float) of the velocity ahead; 16.16's one (a double).
constexpr reg32 kCarAxes = 0xc0, kCarHalfSizes = 0x114;
constexpr reg32 kWheelsAcross = 0x537218, kWheelsAlong = 0x537220, kThirtySecond = 0x537228, kToFixed = 0x537230;

// A car's: in the air (a byte), how hard it hit (a float, 0 none), its spin as its axes see it
// (three floats), its turn (+0x4e0 and +0x4e8, as the air's steering and the shake take them).  Its
// handling's shake (+0x294).
constexpr reg32 kCarInAir = 0x8b, kCarHit = 0x168, kCarSpinSeen = 0x4f8, kCarTurn = 0x4e0;
constexpr reg32 kHandlingShake = 0x294;
// A hit's share of the shake (a 45th, a float), the most of it (a double, 1.5); a matrix
// transposed (eax from, edx to).
constexpr reg32 kHitScale = 0x5371f0, kMostShake = 0x5371f8, kTranspose = 0x4e0110;

// Half (a float) of two wheels' skids; the most a wheel skids (a double, 5); the speeds (doubles)
// past which a wheel that hit something throws dust (5), more (20), sparks (10); 16.16's
// fraction (a double); the sparks' share of the speed (a double); the sparks (stdcall: the place
// and the speed, eax the car).
constexpr reg32 kSkidHalf = 0x537238, kMostSkid = 0x537268;
constexpr reg32 kDustSpeed = 0x537240, kMoreDustSpeed = 0x537248, kSparksSpeed = 0x537250, kFromFixed = 0x537258;
constexpr reg32 kSparksBySpeed = 0x537260, kSparks = 0x4cac20;

// A car's: the frames since what its wheels hit was kept, its wheels' slips (front, back).  The
// dust a wheel that hit throws (13); the lean under which the wheels do not skid (a double); the
// front slip's share of the skid, the most skid (doubles, 10).
constexpr reg32 kCarHitFrames = 0x9a0, kCarFrontSlip = 0x5c0, kCarBackSlip = 0x5c4, kHitDust = 0xd;
constexpr reg32 kSkidLean = 0x537278, kSkidBySlip = 0x537280, kMostWheelSkid = 0x537288;

// A car's: how high (+0x84), its driver's record (+0x50 its share of the rumble).  The shake's:
// gravity on the axes (a float), what its springs keep a frame, the pulls past which they move
// (across, up), the pulls' share (a float), the rumble's half, its shares across and up
// (doubles).
constexpr reg32 kCarAltitude = 0x84, kCarDriver = 0x220, kDriverShake = 0x50;
constexpr reg32 kShakeGravity = 0x537290, kSpringKeep = 0x537298, kAcrossPullFrom = 0x5372a0, kPullShare = 0x5372a8;
constexpr reg32 kUpPullFrom = 0x5372b0, kRumbleHalf = 0x5372b8, kRumbleAcross = 0x5372c0, kRumbleUp = 0x5372c8;

// A car's: where it sparkles when put back (a vector in its axes); its driver's +0x14 (0 to 49,
// as 0x4396e0 takes it).  The sparkle (eax the car), the burst (eax its kind) -- each stdcall, a
// place's three floats; the spin under which a car put back has settled (a double).
constexpr reg32 kCarRespawnOffset = 0x8d8, kDriverIndex = 0x14;
constexpr reg32 kSparkle = 0x4caea0, kSparkleBurst = 0x4cae00, kSettledSpin = 0x537310;

// A car's: its slice at the start (+4), its index among the race's (its grid slot's), its lap's
// state, whether it runs backwards, its stuck record (+0x28 its three start words; 0x104 bytes
// cleared at the start but the pointer and the 12 bytes after it), its lights (1 on, 2 the
// player's).  Its driver's three start words (+0x5c); the handling's scaled (+4).  The race's
// flag for the start records (2).
constexpr reg32 kCarStartSlice = 4, kCarIndex = 0x1f8, kCarLapState = 0x204, kCarBackwards = 0x208;
constexpr reg32 kCarStuckRecord = 0x8a4, kRecordStart = 0x28, kCarLights = 0x660, kDriverStart = 0x5c;
constexpr reg32 kHandlingScaled = 4;
constexpr std::uint8_t kStartRecords = 2;
// The handling scaled (eax the car, edx its number; stdcall the scale and three words); a car put
// on the road (eax, edx the slice, ebx the place, ecx backwards); a fill with zeros (eax where, edx
// how many); the loading's stop check.
constexpr reg32 kScaleHandling = 0x4a0d20, kPutOnRoad = 0x4a0aa0;
constexpr reg32 kFill = 0x4e070c, kLoadingStop = 0x4a33d0;

// A car's: out of the race and in it (bytes: it races with +0x88 clear and +0x89 set), its crash's
// kind (+0x170, with +0x168 and +0x16c cleared each frame).
constexpr reg32 kCarOut = 0x88, kCarIn = 0x89, kCarCrash = 0x170;

// A car's: where its crash hit (+0x178, three floats), its player (a byte).  A surface record's
// scrape's kind (+0x14: 3 sparks, else a burst's kind).  The crash's: the push by the hit (a
// double, a 15th), the push's scale (0.6's bits), the shake's frames the other way (35), the
// sparks by the hit (a double; also the speed past which a leaning car scrapes), its loudness by
// the hit, the sparks by the speed (floats), landing hard (a double), the height under which a
// car scrapes, the lean under which it scrapes, under which it touches (doubles), a kind 3
// crash's speed and hit past which it shows (a float).
constexpr reg32 kCarCrashPoint = 0x178, kCarPlayer = 0x1f0, kSurfaceScrape = 0x14;
constexpr reg32 kPushByHit = 0x5372d0, kPushScale = 0x3f19999a, kCrashShakeFrames = 0x23, kSparksByHit = 0x5372d8;
constexpr reg32 kCrashLoudness = 0x5372e0, kSparksBySpeedHit = 0x5372e4, kLandingHard = 0x5372e8;
constexpr reg32 kScrapeHeight = 0x5372f0, kScrapeLean = 0x5372f8, kTouchLean = 0x537300, kScrapeHit = 0x537308;
// A crash's noise (eax its kind, edx the player).
constexpr reg32 kCrashNoise = 0x4bd9d0;

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
x86::Float f32(App* app, reg32 a) { return x86::Float(app->getMemory<float>(a)); }
x86::Float f64(App* app, reg32 a) { return x86::Float(app->getMemory<double>(a)); }

/* A call by address as the code makes it: the return address's slot, and esp as the callee
 * leaves it.  False when the game stopped in it. */
bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

using x86::Float;

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

float f32At(App* app, reg32 a) { return app->getMemory<float>(a); }
void putFloat(App* app, reg32 a, float v) { app->getMemory<float>(a) = v; }

/* fcom then fnstsw ax: eax with its low word the compare's status word (c3, c2 and c0, as the
 * game's arithmetic has it, fp::compare). */
reg32 statusInto(reg32 eax, const Float& a, const Float& b)
{
    return (eax & 0xffff0000) | fp::compare(a, b);
}

/* A compare's answers as the code's jumps after sahf read them: a over b (ja: neither c0 nor c3,
 * so never for a NaN), a below b (jb: c0, so for a NaN too), a below or equal (jbe). */
bool fAbove(const Float& a, const Float& b) { return fp::compare(a, b) == 0; }
bool fBelow(const Float& a, const Float& b) { return (fp::compare(a, b) & 0x100) != 0; }
bool fBelowOrEqual(const Float& a, const Float& b) { return fp::compare(a, b) != 0; }

// The registers a function's code saves (pushes and pops).
constexpr reg32 kSaveEbx = 1, kSaveEcx = 2, kSaveEdx = 4, kSaveEsi = 8, kSaveEdi = 0x10, kSaveEbp = 0x20;

/* The registers a function's code saves and restores, as they were on entry whatever its callees
 * leave; the others left as its callees leave them. */
class Kept
{
public:
    Kept(CPU& cpu, reg32 saved)
        : cpu_(cpu), saved_(saved), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi),
          ebp_(cpu.ebp)
    {
    }
    ~Kept()
    {
        if (saved_ & kSaveEbx)
            cpu_.ebx = ebx_;
        if (saved_ & kSaveEcx)
            cpu_.ecx = ecx_;
        if (saved_ & kSaveEdx)
            cpu_.edx = edx_;
        if (saved_ & kSaveEsi)
            cpu_.esi = esi_;
        if (saved_ & kSaveEdi)
            cpu_.edi = edi_;
        if (saved_ & kSaveEbp)
            cpu_.ebp = ebp_;
    }

private:
    CPU& cpu_;
    const reg32 saved_;
    const reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* Room on the guest's stack for what a callee is handed the address of: the game's frame
 * (`pushed` bytes of registers under esp on entry, then `bytes`), esp under it while the core
 * runs and back after.  On entry esp is at the return address's slot, as the code's is. */
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

/* A call of a core of this file's as the code makes it: the return address's slot taken on the
 * guest's stack, so that the callee's frame lies where the code's does. */
class Calling
{
public:
    explicit Calling(CPU& cpu) : cpu_(cpu), esp_(cpu.esp) { cpu.esp -= 4; }
    ~Calling() { cpu_.esp = esp_; }

private:
    CPU& cpu_;
    const reg32 esp_;
};

/* The game's functions called by address, their arguments in registers as the code hands them:
 * eax as the callee leaves it, nullopt when the game stopped in one. */
std::optional<reg32> call(App* app, CPU& cpu, reg32 target, reg32 eax)
{
    cpu.eax = eax;
    if (!byAddress(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

std::optional<reg32> call(App* app, CPU& cpu, reg32 target, reg32 eax, reg32 edx)
{
    cpu.edx = edx;
    return call(app, cpu, target, eax);
}

std::optional<reg32> call(App* app, CPU& cpu, reg32 target, reg32 eax, reg32 edx, reg32 ebx)
{
    cpu.ebx = ebx;
    return call(app, cpu, target, eax, edx);
}

std::optional<reg32> call(App* app, CPU& cpu, reg32 target, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx)
{
    cpu.ecx = ecx;
    return call(app, cpu, target, eax, edx, ebx);
}

/* As call, with words pushed for it too, the first lowest (where the code's last push leaves it),
 * esp back after it. */
std::optional<reg32> callPushing(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> words)
{
    if (!guest::call(app, cpu, target, words))
        return std::nullopt;
    return cpu.eax;
}

// The game's sine of a stack float (stdcall), onto the x87's stack.
constexpr reg32 kSine = 0x4ea930;
// The runtime's rand.
constexpr reg32 kRand = 0x4eb1fc;

std::optional<Float> sine(App* app, CPU& cpu, float angle)
{
    if (!guest::call(app, cpu, kSine, {bitsOf(angle)}))
        return std::nullopt;
    return cpu.fpu.take();
}

std::optional<i32> rand(App* app, CPU& cpu)
{
    if (!byAddress(app, cpu, kRand))
        return std::nullopt;
    return i32(cpu.eax);
}

}  // namespace

void respawnCar(App* app, CPU& cpu);
void resetCar(App* app, CPU& cpu);
void stuckCheck(App* app, CPU& cpu);
void clearRespawn(App* app, CPU& cpu);
void shakeAmplitude(App* app, CPU& cpu);
void startShake(App* app, CPU& cpu);
void startIdleShake(App* app, CPU& cpu);
void shakeWave(App* app, CPU& cpu);
void objectContacts(App* app, CPU& cpu);
void wheelSlip(App* app, CPU& cpu);
void wheelCells(App* app, CPU& cpu);
void wheelSurface(App* app, CPU& cpu);
void endSkidMark(App* app, CPU& cpu);
void extendSkidMark(App* app, CPU& cpu);
void skidSounds(App* app, CPU& cpu);
void wheelSpray(App* app, CPU& cpu);
void surfaceEffects(App* app, CPU& cpu);
void wheelSkid(App* app, CPU& cpu);
void wheelEffects(App* app, CPU& cpu);
void updateShake(App* app, CPU& cpu);
void crashEffects(App* app, CPU& cpu);
void respawnEffects(App* app, CPU& cpu);
void stopCarEffects(App* app, CPU& cpu);
void carEffects(App* app, CPU& cpu);
void airborneSpin(App* app, CPU& cpu);
void updateCar(App* app, CPU& cpu);
void defaultGridPlace(App* app, CPU& cpu);
void loadStartGrid(App* app, CPU& cpu);
void gridPlace(App* app, CPU& cpu);
void placeOnGrid(App* app, CPU& cpu);

/* The slice `by` slices on from `slice`, wrapped round the track's: past the last when going on,
 * under 0 when going back, as the code wraps it. */
reg32 slicesOn(App* app, reg32 slice, reg32 by)
{
    reg32 target = slice + by;
    if (i32(by) >= 0)
    {
        const reg32 count = at(app, kSliceCount);
        if (i32(target) >= i32(count))
            target -= count;
    }
    else if (i32(target) < 0)
        target += at(app, kSliceCount);
    return target;
}

/* The car put on slice `slice` (0x428d60), lifted by kRespawnHeight -- by `tunnelScale` in a
 * tunnel (0x420bf0 of its place and slice; eax, edx and ebx at the call as each code leaves them
 * there, `resetting` 0x429100's) -- at rest and settled (0x428970), 0x40 at +0x174.  eax as the
 * settling leaves it; nullopt when the game stopped. */
std::optional<reg32> putBackOn(App* app, CPU& cpu, reg32 car, reg32 slice, reg32 tunnelScale, bool resetting)
{
    if (!call(app, cpu, kPlaceOnSlice, car, slice))
        return std::nullopt;
    float height = f32At(app, kRespawnHeight);
    if (resetting)
    {
        cpu.eax = at(app, car + kCarPlace + 8);
        cpu.edx = at(app, car + kCarPlace + 4);
        cpu.ebx = at(app, car + kCarPlace);
    }
    else
    {
        cpu.eax = at(app, car + kCarPlace + 4);
        cpu.edx = at(app, car + kCarPlace);
    }
    const std::optional<reg32> tunnel =
        guest::direct<0x420bf0, trackdata::placeInTunnel>(app, cpu, car + kCarPlace, at(app, car + kCarSlice));
    if (!tunnel)
        return std::nullopt;
    if (*tunnel != 0)
        height = float(fp::mul(Float(height), f32(app, tunnelScale)));
    put(app, car + kCarVelocity, 0);
    put(app, car + kCarVelocity + 4, 0);
    const Float raised = fp::add(f32(app, car + kCarPlace + 4), Float(height));
    put(app, car + kCarVelocity + 8, 0);
    putFloat(app, car + kCarPlace + 4, float(raised));
    const std::optional<reg32> settled = call(app, cpu, kSettle, car);
    if (!settled)
        return std::nullopt;
    put(app, car + kCarSettleFrames, 0x40);
    return settled;
}

/* The way the race runs: 1, or -1 backwards. */
reg32 raceDirection(App* app)
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
std::optional<reg32> respawnCarOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 direction = raceDirection(app);
    const reg32 times = at(app, car + kCarRespawns) + 1;
    put(app, car + kCarSinceRespawn, 0);
    put(app, car + kCarRespawns, times);
    const auto bits = [&](reg32 field) { return i32(at(app, car + field)); };
    if (bits(kCarLean) > i32(0x3f400000) && bits(kCarSpin) < i32(kOneBits) && bits(kCarSpin + 8) < i32(kOneBits))
    {
        const reg32 surface = at(app, car + kCarSurface);
        if (surface != 0xe && surface != 0 && at(app, car + kCarRespawnSlice) == 0 && half(app, car + kCarStage) <= 5
            && bits(kCarAboveRoad) < i32(0x3f19999a))
        {
            put(app, car + kCarSinceRespawn, 0);
            return direction;
        }
    }
    const reg32 asked = at(app, car + kCarRespawnSlice);
    const reg32 from = asked != 0 ? asked : at(app, car + kCarSlice);
    const reg32 by = direction * at(app, kRespawnStep);
    put(app, car + kCarRespawnSlice, 0);
    return putBackOn(app, cpu, car, slicesOn(app, from, by), kHalfHeight, false);
}

/* 0x428f90: respawnCarOf car eax. */
void respawnCar(App* app, CPU& cpu)
{
    const std::optional<reg32> left = respawnCarOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x429100's: the car put back on the road when the player asks: kRespawnStep slices on from its
 * own slice (putBackOn, by 0x5371d4 in a tunnel), the reset's wait 0x20 frames after -- unless it
 * is waiting yet, it is a player's car finished, no car may be (kResetAllowed), a slice is asked
 * for already, or in Hot Pursuit 0x401500 says no of its pursuer (+0x624, chased) or of itself.
 * eax as the code leaves it; nullopt when the game stopped. */
std::optional<reg32> resetCarOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 direction = raceDirection(app);
    if (at(app, car + kCarResetWait) != 0
        || ((byte(app, car + kCarFlags) & kPlayersCar) != 0 && at(app, car + kCarState) == kFinished)
        || at(app, kResetAllowed) == 0 || at(app, car + kCarRespawnSlice) != 0)
        return direction;
    if (gameSettings(app).raceType == kHotPursuit)
    {
        const reg32 pursuer = at(app, car + kCarPursuer);
        // By address: what the callee leaves in eax is what this leaves (airace::isPulledOver gives
        // back only whether).
        if ((byte(app, car + kCarFlags) & kChased) != 0 && pursuer != 0)
        {
            const std::optional<reg32> out = call(app, cpu, kPursuitOut, pursuer);
            if (!out || *out != 0)
                return out;
        }
        if ((byte(app, car + kCarFlags) & kPlayersCar) != 0)
        {
            const std::optional<reg32> out = call(app, cpu, kPursuitOut, car);
            if (!out || *out != 0)
                return out;
        }
    }
    const reg32 by = direction * at(app, kRespawnStep);
    const std::optional<reg32> left =
        putBackOn(app, cpu, car, slicesOn(app, at(app, car + kCarSlice), by), kResetHeight, true);
    if (!left)
        return std::nullopt;
    put(app, car + kCarResetWait, 0x20);
    return left;
}

/* 0x429100: resetCarOf car eax. */
void resetCar(App* app, CPU& cpu)
{
    const std::optional<reg32> left = resetCarOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
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
std::optional<reg32> stuckCheckOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const auto bits = [&](reg32 field) { return i32(at(app, car + field)); };
    const auto countDown = [&](reg32 by) { put(app, car + kCarStuck, at(app, car + kCarStuck) - by); };
    const auto asked = [&] { return at(app, car + kCarRespawnSlice) != 0; };
    if (bits(kCarStuck) > 0)
    {
        const reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, car + kCarCell, 1);
        if (cpu.terminate)
            return std::nullopt;
        const reg32 on = kind & kSurfaceBits;
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
                const std::optional<reg32> left = respawnCarOf(app, cpu, car);
                if (!left)
                    return std::nullopt;
                put(app, car + kCarStuck, 0);
                return left;
            }
            put(app, car + kCarStuck, 0);
        }
    }
    if (!call(app, cpu, kDot, car + kCarGroundUp, car + kCarUp))
        return std::nullopt;
    const float upright = float(cpu.fpu.take());
    if (bits(kCarStuck) > 0)
        return cpu.eax;
    const reg32 status = statusInto(cpu.eax, Float(upright), f32(app, kUpright));
    if (!fBelow(Float(upright), f32(app, kUpright)))
    {
        const reg32 surface = at(app, car + kCarSurface);
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
void stuckCheck(App* app, CPU& cpu)
{
    const std::optional<reg32> left = stuckCheckOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x4294c0's: the car's slice to be put back on and the frames since, cleared. */
void clearRespawnOf(App* app, reg32 car)
{
    put(app, car + kCarRespawnSlice, 0);
    put(app, car + kCarSinceRespawn, 0);
}

/* 0x4294c0: clearRespawnOf car eax. */
void clearRespawn(App* app, CPU& cpu)
{
    clearRespawnOf(app, cpu.eax);
    cpu.esp += 4;
}

/* The car's surface's record in the surfaces' table (kSurfaces on). */
reg32 surfaceRecord(App* app, reg32 car)
{
    return at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + at(app, car + kCarSurface) * kSurfaceSize;
}

/* 0x4294e0's: the car's rumble: its surface's rumble by its handling's, times sin(the speed's
 * 64th, 0 to 1, by 0.5) (0x4ea930).  The 64th is held to 1 by its bits, as the code does.  eax
 * as the code leaves it (the compare's status word, or the 64th's bits, as the sine leaves it);
 * nullopt when the game stopped. */
std::optional<reg32> shakeAmplitudeOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEbp);
    const Float speed = fp::mul(f32(app, car + kCarSpeed), f32(app, kSixtyFourth));
    const reg32 surface = surfaceRecord(app, car);
    const reg32 handling = at(app, car + kCarHandling);
    const Float rumble = fp::mul(f32(app, surface + kSurfaceRumble), f32(app, handling + kHandlingRumble));
    const float scaled = float(speed);
    putFloat(app, car + kCarRumble, float(rumble));
    reg32 phase = 0;
    cpu.eax = statusInto(handling, 0.0, Float(scaled));
    if (!fAbove(0.0, Float(scaled)))
    {
        phase = i32(bitsOf(scaled)) > i32(kOneBits) ? kOneBits : bitsOf(scaled);
        cpu.eax = phase;
    }
    const std::optional<Float> wave = sine(app, cpu, float(fp::mul(Float(floatOf(phase)), f64(app, kHalfTurn))));
    if (!wave)
        return std::nullopt;
    const float s = float(*wave);
    putFloat(app, car + kCarRumble, float(fp::mul(f32(app, car + kCarRumble), Float(s))));
    return cpu.eax;
}

/* 0x4294e0: shakeAmplitudeOf car eax. */
void shakeAmplitude(App* app, CPU& cpu)
{
    const std::optional<reg32> left = shakeAmplitudeOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* What startShakeOf leaves in eax, edx and ebx, none of which its code keeps. */
struct ShakeLeft
{
    reg32 eax, edx, ebx;
};

/* 0x4295a0's: the car's crash shake started, if it hit hard enough (how hard, +0x168, 5 or more
 * by its bits as an integer): two waves, across and up -- the shake's tuning by its handling's
 * (+0x294), by the hit (a 45th, 0 to 1.5) -- for `acrossFrames` and `upFrames` frames, each its
 * sign turned at random (rand's low byte over 0x80).  What the code leaves in eax, edx and ebx;
 * nullopt when the game stopped. */
std::optional<ShakeLeft> startShakeOf(App* app, CPU& cpu, reg32 car, reg32 acrossFrames, reg32 upFrames)
{
    const Kept kept(cpu, kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 handling = at(app, car + kCarHandling);
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
    const std::optional<i32> acrossSign = rand(app, cpu);
    if (!acrossSign)
        return std::nullopt;
    if (*acrossSign % 0x100 > 0x80)
        putByte(app, car + kCarWaves + 3, byte(app, car + kCarWaves + 3) ^ 0x80);
    const reg32 second = car + kCarWaves + kWaveSize;
    const float upAmplitude = float(fp::mul(Float(shared), Float(up)));
    put(app, second + kWaveFrames, upFrames);
    putFloat(app, second, upAmplitude);
    const std::optional<i32> upSign = rand(app, cpu);
    if (!upSign)
        return std::nullopt;
    reg32 rest = reg32(*upSign % 0x100);
    if (i32(rest) > 0x80)
    {
        const reg8 flipped = byte(app, second + 3) ^ 0x80;
        putByte(app, second + 3, flipped);
        rest = (rest & 0xffffff00) | flipped;
    }
    return ShakeLeft{reg32(*upSign / 0x100), rest, 0x100};
}

/* 0x4295a0: startShakeOf car eax, for edx and ebx frames. */
void startShake(App* app, CPU& cpu)
{
    const std::optional<ShakeLeft> left = startShakeOf(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.ebx = left->ebx;
    cpu.esp += 4;
}

/* 0x4296c0's: the car's little shake, leaning -0.9 or less (or a NaN) and racing: two small
 * waves, each of 24 to 35 frames (rand), each its sign turned at random (rand's low byte over
 * 0x80).  eax as the code leaves it: the compare's status word, or the last rand by 256;
 * nullopt when the game stopped. */
std::optional<reg32> startIdleShakeOf(App* app, CPU& cpu, reg32 car)
{
    const reg32 status = statusInto(car, f32(app, car + kCarLean), f64(app, kIdleLean));
    if (fAbove(f32(app, car + kCarLean), f64(app, kIdleLean)) || half(app, car + kCarStage) != 0)
        return status;
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEbp);
    i32 last = 0;
    for (const reg32 wave : {car + kCarWaves, car + kCarWaves + kWaveSize})
    {
        put(app, wave, kIdleAmplitude);
        const std::optional<i32> frames = rand(app, cpu);
        if (!frames)
            return std::nullopt;
        put(app, wave + kWaveFrames, reg32(*frames % kIdleFramesSpread + kIdleFrames));
        const std::optional<i32> sign = rand(app, cpu);
        if (!sign)
            return std::nullopt;
        if (*sign % 0x100 > 0x80)
            putByte(app, wave + 3, byte(app, wave + 3) ^ 0x80);
        last = *sign / 0x100;
    }
    return reg32(last);
}

/* 0x4296c0: startIdleShakeOf car eax. */
void startIdleShake(App* app, CPU& cpu)
{
    const std::optional<reg32> left = startIdleShakeOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x429790's: a shake's wave `frames` from its end: sin(frames / 12) (0x4ea930) by the
 * amplitude, by the frames (as a float), by a 12th.  Nullopt when the game stopped. */
std::optional<Float> shakeWaveOf(App* app, CPU& cpu, float amplitude, i32 frames)
{
    const Float count = Float(frames);
    const float counted = float(count);
    const std::optional<Float> wave = sine(app, cpu, float(fp::mul(count, f32(app, kTwelfth))));
    if (!wave)
        return std::nullopt;
    return fp::mul(fp::mul(fp::mul(*wave, Float(amplitude)), Float(counted)), f32(app, kTwelfth));
}

/* 0x429790 (stdcall: the amplitude, the frames left): shakeWaveOf, onto the x87's stack; eax
 * kept, as the code keeps it. */
void shakeWave(App* app, CPU& cpu)
{
    const reg32 eax = cpu.eax;
    const std::optional<Float> wave = shakeWaveOf(app, cpu, f32At(app, cpu.esp + 4), i32(at(app, cpu.esp + 8)));
    if (!wave)
        return;
    cpu.fpu.push(*wave);
    cpu.eax = eax;
    cpu.esp += 4 + 8;
}

/* 0x4297d0's: the car against the scenery's objects: the player's first (0x493ad0); then, a
 * player's car still racing (its state not finished, and not out unless the player's), the
 * object it is against (0x4ca160, its handle to a word of the frame's): handed over (0x4c9f50),
 * the hit's flags kept, 0x40 at +0x621, the car told (0x401650).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<reg32> objectContactsOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const Kept kept(cpu, kSaveEcx | kSaveEdx | kSaveEsi | kSaveEbp);
    const reg32 handle = frame - 4;
    reg32 left = car;
    if ((byte(app, car + kCarFlags) & kThisMachines) != 0)
    {
        const std::optional<reg32> done = call(app, cpu, kPlayerObjects, car);
        if (!done)
            return std::nullopt;
        left = *done;
    }
    const reg8 flags = byte(app, car + kCarFlags);
    if (half(app, car + kCarStage) != 0 || (flags & kPlayersCar) == 0 || at(app, car + kCarState) == kFinished
        || (at(app, car + kCarState) == kOut && (flags & kThisMachines) == 0))
        return left;
    const std::optional<reg32> against = call(app, cpu, kObjectAgainst, car, handle);
    if (!against)
        return std::nullopt;
    if (*against == 0)
        return reg32(0);
    // The hit's flags as the handing over leaves them in edx.
    if (!call(app, cpu, kObjectHit, at(app, handle), *against))
        return std::nullopt;
    const reg32 hit = cpu.edx;
    putByte(app, car + kCarObjectFlags, byte(app, car + kCarObjectFlags) | kHitObject);
    put(app, car + kCarHits, at(app, car + kCarHits) | hit);
    return call(app, cpu, kCarHitObject, car, at(app, handle));
}

/* 0x4297d0: objectContactsOf car eax. */
void objectContacts(App* app, CPU& cpu)
{
    const std::optional<reg32> left = objectContactsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
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
std::optional<Slip> wheelSlipOf(App* app, CPU& cpu, reg32 car, reg32 front)
{
    const GuestFrame frame(cpu, 0x14, 0x50);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 velocity = frame - 0x50, forward = frame - 0x44;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, velocity + k, at(app, car + kCarVelocity + k));
    const Float length = guest::direct<0x4e06a0, vertices::vectorLength>(app, cpu, car + kCarSpin);
    if (cpu.terminate)
        return std::nullopt;
    const float turning = float(fp::add(length, f32(app, car + kCarSpeed)));
    guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, velocity);
    if (cpu.terminate)
        return std::nullopt;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, forward + k, at(app, car + kCarForward + k));
    const float along = float(guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, velocity, forward));
    if (cpu.terminate)
        return std::nullopt;
    // Its size as the code takes it: kept when 0 is below it (or it is a NaN), else negated.
    const float size = fBelow(0.0, Float(along)) ? along : float(-Float(along));
    const float sideways = float(fp::sub(1.0, Float(size)));
    reg32 second = forward;  // edx as the dot product leaves it: its second vector
    reg32 slide;
    if (front != 0)
    {
        second = at(app, car + kCarFrontSlide);
        slide = i32(second) > 0 ? second : 0u - second;
    }
    else
    {
        const reg32 back = at(app, car + kCarBackSlide);
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
        return Slip{least, {reg32(bits >> 32), second}};
    }
    return Slip{1.0, {0, 0x3ff00000}};
}

/* 0x429880: wheelSlipOf car eax, a front wheel's when edx is not 0, onto the x87's stack. */
void wheelSlip(App* app, CPU& cpu)
{
    const std::optional<Slip> slip = wheelSlipOf(app, cpu, cpu.eax, cpu.edx);
    if (!slip)
        return;
    cpu.fpu.push(slip->value);
    cpu.eax = slip->left.eax;
    cpu.edx = slip->left.edx;
    cpu.esp += 4;
}

/* 0x4299a0's: the car's four wheels on the track: its corners -- across 0.75 of its half size
 * (+0x114) either way, down its height (+0x118), along half its length (+0x11c) either way --
 * turned by its axes and moved to where it will be (its place plus a 32nd of its velocity;
 * 0x4e0430, the corners, the place and what it makes in the frame), as 16.16 (each to the
 * nearest), each looked up from the car's slice (0x420650, into a record of the frame's) and
 * copied to the wheel's 0x78 bytes.  Nullopt when the game stopped. */
bool wheelCellsOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x104);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 record = frame - 0x104, turned = frame - 0x8c, corners = frame - 0x5c, moved = frame - 0x2c;
    const reg32 place = frame - 0x20;
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
    for (reg32 c = 0; c < 4; ++c)
        for (reg32 k = 0; k < 3; ++k)
            putFloat(app, corners + 0xc * c + 4 * k, corner[c][k]);
    const Float step = f32(app, kThirtySecond);
    putFloat(app, moved, float(fp::add(fp::mul(f32(app, car + kCarVelocity), step), f32(app, car + kCarPlace))));
    putFloat(app, moved + 4,
             float(fp::add(fp::mul(f32(app, car + kCarVelocity + 4), step), f32(app, car + kCarPlace + 4))));
    putFloat(app, moved + 8,
             float(fp::add(fp::mul(step, f32(app, car + kCarVelocity + 8)), f32(app, car + kCarPlace + 8))));
    guest::direct<0x4e0430, vertices::transformPointList>(app, cpu, 4, corners, car + kCarAxes, moved, turned);
    if (cpu.terminate)
        return false;
    for (reg32 wheel = 0; wheel < 4; ++wheel)
    {
        for (reg32 k = 0; k < 3; ++k)
            put(app, place + 4 * k,
                reg32(fp::toInteger<i32>(fp::mul(f32(app, turned + 0xc * wheel + 4 * k), f64(app, kToFixed)))));
        if (!guest::direct<0x420650, trackdata::placeAt>(app, cpu, place, 1, at(app, car + kCarSlice), record))
            return false;
        for (reg32 k = 0; k < kWheelSize; k += 4)
            put(app, car + kCarWheels + wheel * kWheelSize + k, at(app, record + k));
    }
    return true;
}

/* 0x4299a0: wheelCellsOf car eax; eax left as the code leaves it, at the last wheel's cell less
 * 0x300. */
void wheelCells(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!wheelCellsOf(app, cpu, car))
        return;
    cpu.eax = car + 3 * kWheelSize;
    cpu.esp += 4;
}

/* 0x429b40's: what wheel `wheel` of the car is on: the car's surface, or, on a piece of track of
 * its own with the race on, the wheel's (0x4207e0).  Nullopt when the game stopped. */
std::optional<reg32> wheelSurfaceOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const reg32 cell = car + kCarWheels + wheel * kWheelSize;
    if (half(app, car + kCarStage) != 0 || at(app, cell + kWheelPiece) == 0 || at(app, cell + kWheelPieceToo) == 0)
        return at(app, car + kCarSurface);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEbp);
    const reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, cell, 1);
    if (cpu.terminate)
        return std::nullopt;
    return kind & kSurfaceBits;
}

/* 0x429b40: wheelSurfaceOf car eax's wheel edx, in eax and edx both. */
void wheelSurface(App* app, CPU& cpu)
{
    const std::optional<reg32> surface = wheelSurfaceOf(app, cpu, cpu.eax, cpu.edx);
    if (!surface)
        return;
    cpu.eax = *surface;
    cpu.edx = *surface;
    cpu.esp += 4;
}

/* A wheel's cell's height under it, axis `k` (0 to 2): on a piece of its own its heights' 16-bit
 * word k (at +0x1c's) by 2, else its slice's byte (+0x10 on, signed) by 512. */
reg32 cellHeight(App* app, reg32 cell, reg32 k)
{
    if (at(app, cell + kWheelPiece) != 0)
        return reg32(i32(i16(half(app, at(app, cell + kWheelHeights) + 2 * k)))) * 2;
    const reg32 record = at(app, kSlices) + at(app, cell + kWheelSlice) * kSliceSize;
    return reg32(i32(i8(byte(app, record + kSliceHeights + k)))) << 9;
}

/* A mark's point drawn (0x4db940): the car's number and the wheel, the point and the heights
 * under it (frame words), the stack's `ending` and the slip.  edx as the callee leaves it;
 * nullopt when the game stopped. */
std::optional<reg32> markPoint(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 point, reg32 heights, reg32 ending,
                               float slip)
{
    cpu.eax = at(app, car + kCarNumber);
    cpu.edx = wheel;
    cpu.ebx = point;
    cpu.ecx = heights;
    if (!callPushing(app, cpu, kMarkPoint, {ending, bitsOf(slip)}))
        return std::nullopt;
    return cpu.edx;
}

/* 0x429ba0's: wheel `wheel`'s skid mark ended, if it has one: its last point, the heights under
 * it, how much it slips (wheelSlipOf, the front's slide for wheels 0 and 1) -- 0x4db940 with 1
 * -- and the mark and its sound off.  What the code leaves in eax and edx; nullopt when the game
 * stopped. */
std::optional<Left> endSkidMarkOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const GuestFrame frame(cpu, 0x14, 0x24);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    if (byte(app, car + wheel + kCarMarks) == 0)
        return Left{car + wheel, wheel};
    std::optional<Slip> slip;
    {
        const Calling calling(cpu);
        slip = wheelSlipOf(app, cpu, car, i32(wheel) < 2 ? 1 : 0);
    }
    if (!slip)
        return std::nullopt;
    const reg32 point = frame - 0x18, heights = frame - 0x24;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, point + k, at(app, car + wheel * 0xc + kCarLastPoints + k));
    const float slipped = float(slip->value);
    const reg32 cell = car + kCarWheels + wheel * kWheelSize;
    for (reg32 k = 0; k < 3; ++k)
        put(app, heights + 4 * k, cellHeight(app, cell, k));
    const std::optional<reg32> left = markPoint(app, cpu, car, wheel, point, heights, 1, slipped);
    if (!left)
        return std::nullopt;
    putByte(app, car + wheel + kCarMarks, 0);
    putByte(app, car + wheel + kCarMarkSounds, 0);
    return Left{car + wheel, *left};
}

/* 0x429ba0: endSkidMarkOf car eax's wheel edx. */
void endSkidMark(App* app, CPU& cpu)
{
    const std::optional<Left> left = endSkidMarkOf(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
}

/* 0x429d20's: wheel `wheel`'s skid mark drawn on to where it is: off the track's pieces, ended
 * (0x4db860); else a point (0x4db940 with 0) -- its place, the heights under it, how much it
 * slips (wheelSlipOf) -- kept as the last, the mark on and its sound the surface's (wheelSurfaceOf).
 * What the code leaves in eax and edx; nullopt when the game stopped. */
std::optional<Left> extendSkidMarkOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const GuestFrame frame(cpu, 0x14, 0x28);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    std::optional<Slip> slip;
    {
        const Calling calling(cpu);
        slip = wheelSlipOf(app, cpu, car, i32(wheel) < 2 ? 1 : 0);
    }
    if (!slip)
        return std::nullopt;
    const float slipped = float(slip->value);
    const reg32 cell = car + kCarWheels + wheel * kWheelSize;
    const reg32 point = frame - 0x28, heights = frame - 0x1c;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, point + k, at(app, cell + kWheelPlace + k));
    std::optional<reg32> surface;
    {
        const Calling calling(cpu);
        surface = wheelSurfaceOf(app, cpu, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    if (at(app, cell + kWheelPiece) == 0 || at(app, cell + kWheelPieceToo) == 0)
    {
        if (!call(app, cpu, kCutMark, at(app, car + kCarNumber), wheel))
            return std::nullopt;
        putByte(app, car + wheel + kCarMarks, 0);
        putByte(app, car + wheel + kCarMarkSounds, 0);
        return Left{car + wheel, cpu.edx};
    }
    for (reg32 k = 0; k < 3; ++k)
        put(app, heights + 4 * k, cellHeight(app, cell, k));
    if (!markPoint(app, cpu, car, wheel, point, heights, 0, slipped))
        return std::nullopt;
    for (reg32 k = 0; k < 0xc; k += 4)
        put(app, car + wheel * 0xc + kCarLastPoints + k, at(app, point + k));
    putByte(app, car + wheel + kCarMarks, 1);
    const reg32 sound = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + *surface * kSurfaceSize + kSurfaceSound;
    const reg8 heard = byte(app, sound);
    putByte(app, car + wheel + kCarMarkSounds, heard);
    return Left{((*surface * 9) & 0xffffff00) | heard, *surface};
}

/* 0x429d20: extendSkidMarkOf car eax's wheel edx. */
void extendSkidMark(App* app, CPU& cpu)
{
    const std::optional<Left> left = extendSkidMarkOf(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
}

/* An event added to the car (0x4284e0): edx, ebx and ecx its fields, the stack's three words
 * (the first a field, the second a time of 0 to 10, the third the last field).  eax as the callee
 * leaves it; nullopt when the game stopped. */
std::optional<reg32> carEvent(App* app, CPU& cpu, reg32 car, reg32 first, reg32 second, reg32 third,
                              std::initializer_list<reg32> words)
{
    cpu.eax = car;
    cpu.edx = first;
    cpu.ebx = second;
    cpu.ecx = third;
    return callPushing(app, cpu, kCarEvent, words);
}

/* 0x429f10's: the car's skid sounds: the front wheels' (their sounds +0x990, +0x991) and the
 * back's (+0x992, +0x993), events (0x4284e0) by how much they skid -- both: half of both, the
 * back's with the second's sound; one: its own -- or stopped once (+0x994, +0x995 whether on).
 * eax as the code leaves it; nullopt when the game stopped. */
std::optional<reg32> skidSoundsOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEbp);
    const auto sound = [&](reg32 wheel) { return reg32(byte(app, car + kCarMarkSounds + wheel)); };
    const auto skid = [&](reg32 wheel) { return car + kCarSkids + 4 * wheel; };
    const auto both = [&](reg32 wheel) {
        return bitsOf(float(fp::mul(fp::add(f32(app, skid(wheel)), f32(app, skid(wheel + 1))), f32(app, kSkidHalf))));
    };
    reg32 left = car;
    // The front wheels' (the event's edx 2 played, 3 stopped; ebx 8).
    {
        std::optional<reg32> played;
        bool playing = true;
        if (sound(0) != 0 && sound(1) != 0)
            played = carEvent(app, cpu, car, 2, 8, 6, {sound(0), both(0), 0});
        else if (sound(0) != 0)
            played = carEvent(app, cpu, car, 2, 8, 6, {sound(0), at(app, skid(0)), 0});
        else if (sound(1) != 0)
            played = carEvent(app, cpu, car, 2, 8, 6, {sound(1), at(app, skid(1)), 0});
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
            const std::optional<reg32> stopped = carEvent(app, cpu, car, 3, 8, 6, {0, 0, 0});
            if (!stopped)
                return std::nullopt;
            putByte(app, car + kCarScrapes, 0);
            left = *stopped;
        }
    }
    // The back's (edx 2 both, 4 one, 5 stopped; ebx 10).
    std::optional<reg32> played;
    if (sound(2) != 0 && sound(3) != 0)
        played = carEvent(app, cpu, car, 2, 0xa, 6, {sound(3), both(2), 0});
    else if (sound(2) != 0)
        played = carEvent(app, cpu, car, 4, 0xa, 6, {sound(2), at(app, skid(2)), 0});
    else if (sound(3) != 0)
        played = carEvent(app, cpu, car, 4, 0xa, 6, {sound(3), at(app, skid(3)), 0});
    else
    {
        if (byte(app, car + kCarScrapes + 1) == 0)
            return left;
        const std::optional<reg32> stopped = carEvent(app, cpu, car, 5, 0xa, 6, {0, 0, 0});
        if (stopped)
            putByte(app, car + kCarScrapes + 1, 0);
        return stopped;
    }
    if (played)
        putByte(app, car + kCarScrapes + 1, 1);
    return played;
}

/* 0x429f10: skidSoundsOf car eax. */
void skidSounds(App* app, CPU& cpu)
{
    const std::optional<reg32> left = skidSoundsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* An effect where wheel `wheel` of the car is (0x4cb010, stdcall: its place's three words and the
 * kind).  What the callee leaves in eax and edx; nullopt when the game stopped. */
std::optional<Left> wheelEffect(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 kind)
{
    const reg32 cell = car + kCarWheels + wheel * kWheelSize;
    cpu.eax = car;
    cpu.edx = wheel;
    if (!callPushing(app, cpu, kWheelSmoke,
                     {at(app, cell + kWheelPlace), at(app, cell + kWheelPlace + 4), at(app, cell + kWheelPlace + 8), kind}))
        return std::nullopt;
    return Left{cpu.eax, cpu.edx};
}

/* 0x42a130's: wheel `wheel` of the car hitting something (its bit of what the wheels hit), the
 * race on: dust past 5 (0x4cb010, 6), more past 20 (9), past 10 the sparks (0x4cac20) where the
 * wheel is (its 16.16 place as floats), by its speed, and their event (0x4284e0).  What the code
 * leaves in eax and edx; nullopt when the game stopped. */
std::optional<Left> wheelSprayOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 bit = reg32(1) << (wheel & 31);
    const reg32 hits = at(app, car + kCarHits);
    if ((bit & hits) == 0 || half(app, car + kCarStage) != 0)
        return Left{bit, hits};
    Left left{bit, hits};
    for (const auto& [past, kind] : {std::pair<reg32, reg32>{kDustSpeed, 6}, {kMoreDustSpeed, 9}})
    {
        left.eax = statusInto(left.eax, f32(app, car + kCarSpeed), f64(app, past));
        if (!fAbove(f32(app, car + kCarSpeed), f64(app, past)))
            continue;
        const std::optional<Left> dusted = wheelEffect(app, cpu, car, wheel, kind);
        if (!dusted)
            return std::nullopt;
        left = *dusted;
    }
    const float speed = f32At(app, car + kCarSpeed);
    left.eax = statusInto(left.eax, Float(speed), f64(app, kSparksSpeed));
    if (fBelowOrEqual(Float(speed), f64(app, kSparksSpeed)))
        return left;
    const reg32 cell = car + kCarWheels + wheel * kWheelSize;
    const Float scale = f64(app, kFromFixed);
    const Float bySpeed = fp::mul(Float(speed), f64(app, kSparksBySpeed));
    const Float x = fp::mul(Float(i32(at(app, cell + kWheelPlace))), scale);
    const Float y = fp::mul(Float(i32(at(app, cell + kWheelPlace + 4))), scale);
    const Float z = fp::mul(scale, Float(i32(at(app, cell + kWheelPlace + 8))));
    cpu.eax = car;
    if (!callPushing(app, cpu, kSparks, {bitsOf(float(x)), bitsOf(float(y)), bitsOf(float(z)), bitsOf(float(bySpeed))}))
        return std::nullopt;
    const std::optional<reg32> heard = carEvent(app, cpu, car, 1, 0xffffffff, 5, {0x12, 0, 0});
    if (!heard)
        return std::nullopt;
    return Left{*heard, cpu.edx};
}

/* 0x42a130: wheelSprayOf car eax's wheel edx. */
void wheelSpray(App* app, CPU& cpu)
{
    const std::optional<Left> left = wheelSprayOf(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
}

/* 0x42a280's: wheel `wheel`'s effect from its surface's record (wheelSurfaceOf; kSurfaces on, by
 * the track and the surface), where it has one: skidding past the surface's skid, the effect
 * where the wheel is (0x4cb010) and, on a surface without marks, its sound; else, faster than
 * the surface's skid starts (where its start and range are not 0), the effect, a back wheel's
 * sound, and how much it skids: (speed - start) / range by 5, 0 to 5.  What the code leaves in
 * eax and edx; nullopt when the game stopped. */
std::optional<Left> surfaceEffectsOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    std::optional<reg32> surface;
    {
        const Calling calling(cpu);
        surface = wheelSurfaceOf(app, cpu, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    const bool front = i32(wheel) < 2;
    const reg32 track = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize;
    const reg32 record = track + *surface * kSurfaceSize;
    const reg32 effect = at(app, record + kSurfaceEffect);
    if (effect == 0)
        return Left{0, *surface};
    const reg32 from = at(app, record + kSurfaceSkidFrom), over = at(app, record + kSurfaceSkidOver);
    const reg32 skid = car + wheel * 4 + kCarSkids, mark = car + wheel;
    const reg32 wheelRecord = car + wheel * kWheelSize;
    reg32 status = statusInto(skid - kCarSkids, f32(app, skid), f32(app, record + kSurfaces));
    if (fAbove(f32(app, skid), f32(app, record + kSurfaces)))
    {
        if (!wheelEffect(app, cpu, car, wheel, effect))
            return std::nullopt;
        const reg32 own = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + *surface * kSurfaceSize;
        if (at(app, own + kSurfaceMarks) != 0)
            return Left{own, at(app, car + kCarTrackSurfaces)};
        const reg8 heard = byte(app, own + kSurfaceSound);
        putByte(app, mark + kCarMarkSounds, heard);
        return Left{(own & 0xffffff00) | heard, mark};
    }
    if ((from & 0x7fffffff) == 0 || (over & 0x7fffffff) == 0)
        return Left{status, wheelRecord};
    status = statusInto(status, f32(app, car + kCarSpeed), Float(floatOf(from)));
    if (fBelowOrEqual(f32(app, car + kCarSpeed), Float(floatOf(from))))
        return Left{status, wheelRecord};
    const std::optional<Left> shown = wheelEffect(app, cpu, car, wheel, effect);
    if (!shown)
        return std::nullopt;
    reg32 left = shown->eax;
    if (!front)
    {
        const reg32 tracks = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize;
        const reg8 heard = byte(app, *surface * kSurfaceSize + tracks + kSurfaceRearSound);
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
    return Left{reg32(bits >> 32), skid - kCarSkids};
}

/* 0x42a280: surfaceEffectsOf car eax's wheel edx. */
void surfaceEffects(App* app, CPU& cpu)
{
    const std::optional<Left> left = surfaceEffectsOf(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
}

/* 0x42a480's: wheel `wheel`'s skid mark: drawn on while it skids on a surface that marks
 * (extendSkidMarkOf; the surface's low 4 bits) -- a back wheel skidding past 5 smokes (0x4cb010
 * at its place, 6) -- else ended (endSkidMarkOf).  What the code leaves in eax and edx; nullopt
 * when the game stopped. */
std::optional<Left> wheelSkidOf(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    const GuestFrame frame(cpu, 0x14, 0xc);  // as deep as the code's, for its callees' frames
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const bool front = i32(wheel) < 2;
    std::optional<reg32> surface;
    {
        const Calling calling(cpu);
        surface = wheelSurfaceOf(app, cpu, car, wheel);
    }
    if (!surface)
        return std::nullopt;
    const reg32 skid = car + wheel * 4 + kCarSkids;
    const reg32 record = at(app, car + kCarTrackSurfaces) * kTrackSurfacesSize + (*surface & 0xf) * kSurfaceSize;
    if ((at(app, skid) & 0x7fffffff) != 0 && at(app, record + kSurfaceMarks) != 0)
    {
        std::optional<Left> left;
        {
            const Calling calling(cpu);
            left = extendSkidMarkOf(app, cpu, car, wheel);
        }
        if (!left || front)
            return left;
        if (fBelowOrEqual(f32(app, skid), f64(app, kSmokingSkid)))
            return Left{statusInto(left->eax, f32(app, skid), f64(app, kSmokingSkid)), left->edx};
        const reg32 cell = car + kCarWheels + wheel * kWheelSize;
        cpu.eax = car;
        cpu.edx = wheel;
        const std::optional<reg32> smoked = callPushing(
            app, cpu, kWheelSmoke,
            {at(app, cell + kWheelPlace), at(app, cell + kWheelPlace + 4), at(app, cell + kWheelPlace + 8), kSmoke});
        if (!smoked)
            return std::nullopt;
        return Left{*smoked, cpu.edx};
    }
    const Calling calling(cpu);
    return endSkidMarkOf(app, cpu, car, wheel);
}

/* 0x42a480: wheelSkidOf car eax's wheel edx. */
void wheelSkid(App* app, CPU& cpu)
{
    const std::optional<Left> left = wheelSkidOf(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
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
std::optional<reg32> wheelEffectsOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x30);  // as deep as the code's, for its callees' frames
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    if ((byte(app, car + kCarObjectFlags) & kHitObject) == 0)
    {
        put(app, car + kCarHitFrames, 0);
        put(app, car + kCarHits, 0);
    }
    else
        put(app, car + kCarHitFrames, at(app, car + kCarHitFrames) + 1);
    if (half(app, car + kCarStage) == 0 && at(app, car + kCarHits) != 0 && i32(at(app, car + kCarHitFrames)) < 3)
    {
        if (at(app, car + kCarHitFrames) == 1 && !carEvent(app, cpu, car, 1, 0xffffffff, 6, {0x13, 0x41200000, 0}))
            return std::nullopt;
        for (reg32 wheel = 0; wheel < 4; ++wheel)
            if ((byte(app, car + kCarHits) & (1 << wheel)) != 0 && !wheelEffect(app, cpu, car, wheel, kHitDust))
                return std::nullopt;
    }
    for (reg32 wheel = 0; wheel < 4; ++wheel)
    {
        putByte(app, car + wheel + kCarMarkSounds, 0);
        const reg32 skid = car + wheel * 4 + kCarSkids;
        if (half(app, car + kCarStage) == 0 && !fBelow(f32(app, car + kCarLean), f64(app, kSkidLean)))
        {
            Float held;
            if (i32(wheel) < 2)
                held = heldSkid(app, fp::mul(f32(app, car + kCarFrontSlip), f64(app, kSkidBySlip)));
            else
                held = heldSkid(app, f32(app, car + kCarBackSlip));
            putFloat(app, skid, float(held));
            const Calling calling(cpu);
            if (!wheelSkidOf(app, cpu, car, wheel) || !surfaceEffectsOf(app, cpu, car, wheel)
                || !wheelSprayOf(app, cpu, car, wheel))
                return std::nullopt;
        }
        else
        {
            if (!call(app, cpu, kCutMark, at(app, car + kCarNumber), wheel))
                return std::nullopt;
            putByte(app, car + wheel + kCarMarks, 0);
            put(app, skid, 0);
        }
    }
    const Calling calling(cpu);
    return skidSoundsOf(app, cpu, car);
}

/* 0x42a560: wheelEffectsOf car eax. */
void wheelEffects(App* app, CPU& cpu)
{
    const std::optional<reg32> left = wheelEffectsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* A shake's spring at `field` held to `limit` either way, as the code holds it: under -limit (or
 * a NaN) -limit, over limit limit, else itself -- its bits, read again for the second compare. */
reg32 heldTo(App* app, reg32 field, float limit, const Float& first)
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
bool rumbleOn(App* app, CPU& cpu, reg32 car, reg32 field, reg32 share, bool byDriver)
{
    const std::optional<Float> random = guest::direct<0x4eb230, ea::randomFractionOf>(app, cpu);
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
std::optional<reg32> updateShakeOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 across = car + kCarShake, up = across + 4, acrossSpring = across + 8, upSpring = across + 0xc;
    reg32 left = 0;
    bool shaking = false;
    if (i32(at(app, car + kCarAltitude)) < i32(0x42700000) && (byte(app, car + kCarFlags) & kWreck) == 0)
    {
        const reg32 handling = at(app, car + kCarHandling);
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
        const reg32 acrossHeld = heldTo(app, acrossSpring, acrossLimit, f32(app, acrossSpring));
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
            for (const auto& [field, wave] : {std::pair<reg32, reg32>{up, kCarWaves + kWaveSize}, {across, kCarWaves}})
            {
                if (field == across)
                    putFloat(app, across, float(f32(app, acrossSpring)));
                const reg32 frames = at(app, car + wave + kWaveFrames);
                if (frames == 0)
                    continue;
                const std::optional<Float> added = shakeWaveOf(app, cpu, f32At(app, car + wave), i32(frames));
                if (!added)
                    return std::nullopt;
                putFloat(app, field, float(fp::add(*added, f32(app, field))));
            }
            const reg32 acrossShake = heldTo(app, across, upLimit, f32(app, across));
            const Float upFirst = f32(app, up);
            put(app, across, acrossShake);
            put(app, up, heldTo(app, up, upLimit, upFirst));
            const std::optional<reg32> rumbled = shakeAmplitudeOf(app, cpu, car);
            if (!rumbled)
                return std::nullopt;
            const reg32 raceType = gameSettings(app).raceType;
            const bool byDriver = i32(gameSettings(app).playMode) <= 1 && raceType != 1 && raceType != 2;
            if (!rumbleOn(app, cpu, car, across, kRumbleAcross, byDriver)
                || !rumbleOn(app, cpu, car, up, kRumbleUp, byDriver))
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
    const reg32 acrossFrames = at(app, car + kCarWaves + kWaveFrames);
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
    const reg32 upFrames = at(app, car + kCarWaves + kWaveSize + kWaveFrames);
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
void updateShake(App* app, CPU& cpu)
{
    const std::optional<reg32> left = updateShakeOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* A burst at a place (0x4cae00, stdcall: the place's three words read from `place`, eax its
 * kind); sparks there (0x4cac20, eax the car, a fourth word: how many).  eax as the callee leaves it; nullopt when the
 * game stopped. */
std::optional<reg32> burstAt(App* app, CPU& cpu, reg32 kind, reg32 place)
{
    cpu.eax = kind;
    return callPushing(app, cpu, kSparkleBurst, {at(app, place), at(app, place + 4), at(app, place + 8)});
}

std::optional<reg32> sparksAt(App* app, CPU& cpu, reg32 car, reg32 place, reg32 size)
{
    cpu.eax = car;
    return callPushing(app, cpu, kSparks, {at(app, place), at(app, place + 4), at(app, place + 8), size});
}

/* A crash's shake (startShakeOf) for rand's low 3 bits and `from` frames one way, 35 the other
 * (`acrossRandom` the across wave's the random one).  Nullopt when the game stopped. */
std::optional<reg32> crashShake(App* app, CPU& cpu, reg32 car, reg32 from, bool acrossRandom)
{
    const std::optional<i32> r = rand(app, cpu);
    if (!r)
        return std::nullopt;
    const reg32 frames = reg32(*r % 8) + from;
    const Calling calling(cpu);
    const std::optional<ShakeLeft> left =
        startShakeOf(app, cpu, car, acrossRandom ? frames : kCrashShakeFrames, acrossRandom ? kCrashShakeFrames : frames);
    if (!left)
        return std::nullopt;
    return left->eax;
}

/* A crash's noise (0x4bd9d0): eax its kind, edx the car's player. */
std::optional<reg32> crashNoise(App* app, CPU& cpu, reg32 car, reg32 kind)
{
    return call(app, cpu, kCrashNoise, kind, byte(app, car + kCarPlayer));
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
std::optional<reg32> crashEffectsOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x70);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 kind = at(app, car + kCarCrash) & 0xefff0000;
    const reg32 other = at(app, car + kCarCrash) & 0xff;
    const reg32 point = car + kCarCrashPoint;
    const auto hit = [&] { return at(app, car + kCarHit); };
    const bool forced = guest::direct<0x471dc0, controls::forceCar>(app, cpu, car);
    if (cpu.terminate)
        return std::nullopt;
    if (forced && (hit() & 0x7fffffff) != 0 && kind != 0x60000)
    {
        cpu.edx = hit();  // as the code holds it at the call: the jolt's frame keeps it
        if (!guest::direct<0x471270, controls::crashWith>(app, cpu, car, at(app, point), at(app, point + 4),
                                                          at(app, point + 8), hit()))
            return std::nullopt;
    }
    // Its surface's record, as the code copies it into the frame: the scrape's kind (+0x14), its
    // sound (+0x1c).
    const reg32 surface = kSurfaces + surfaceRecord(app, car);
    const reg32 scrapeKind = at(app, surface + kSurfaceScrape), scrapeSound = at(app, surface + kSurfaceSound - kSurfaces);
    if (i32(hit()) > i32(0x40a00000))
    {
        reg32 channel = at(app, reg32(i32(kind) >> 16) * 4 + kCrashChannels);
        if (((byte(app, car + kCarFlags) & kWreck) != 0 || i32(hit()) > i32(0x41f00000))
            && at(app, car + kCarSinceRespawn) == 0)
        {
            const auto pushed = [&] { return fp::toInteger<i32>(fp::mul(f32(app, car + kCarHit), f64(app, kPushByHit))); };
            i32 by = pushed();
            by = by > 0 ? pushed() : -pushed();
            put(app, car + kCarSinceRespawn, at(app, car + kCarSinceRespawn) + reg32(by));
            const reg32 offset = car + kCarRespawnOffset;
            const Float dx = fp::sub(f32(app, point), f32(app, car + kCarPlace));
            const Float dy = fp::sub(f32(app, point + 4), f32(app, car + kCarPlace + 4));
            const Float dz = fp::sub(f32(app, point + 8), f32(app, car + kCarPlace + 8));
            putFloat(app, offset, float(dx));
            putFloat(app, offset + 4, float(dy));
            putFloat(app, offset + 8, float(dz));
            const reg32 axes = frame - 0x4c;
            if (!call(app, cpu, kTranspose, car + kCarAxes, axes))
                return std::nullopt;
            Float seen[3];
            for (reg32 k = 0; k < 3; ++k)
                seen[k] = fp::mul(f32(app, offset), f32(app, axes + 4 * k));
            for (reg32 row = 1; row < 3; ++row)
            {
                Float by3[3];
                for (reg32 k = 0; k < 3; ++k)
                    by3[k] = fp::mul(f32(app, offset + 4 * row), f32(app, axes + 0xc * row + 4 * k));
                for (reg32 k = 0; k < 3; ++k)
                    seen[k] = fp::add(seen[k], by3[k]);
            }
            for (reg32 k = 0; k < 3; ++k)
                putFloat(app, offset + 4 * k, float(seen[k]));
            cpu.ebx = offset;  // as the code holds them at the call
            cpu.ecx = offset;
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, offset, floatOf(kPushScale), offset);
            if (cpu.terminate)
                return std::nullopt;
        }
        reg32 sound = 1;
        switch (kind)
        {
        case 0x40000:
            sound = at(app, other * 4 + kCrashSounds);
            if (!crashShake(app, cpu, car, 0x10, true) || !burstAt(app, cpu, at(app, other * 4 + kCrashEffects), point)
                || !crashNoise(app, cpu, car, 5))
                return std::nullopt;
            break;
        case 0x30000:
            sound = scrapeSound;
            if (i32(at(app, car + kCarLean)) < i32(0x3f666666))
                channel = 5;
            if (!crashShake(app, cpu, car, 0x10, false) || !crashNoise(app, cpu, car, 3))
                return std::nullopt;
            break;
        case 0x20000:
            if (!crashShake(app, cpu, car, 0x18, true))
                return std::nullopt;
            sound = scrapeSound;
            break;
        case 0x10000:
            sound = scrapeSound;
            if (scrapeKind == 3)
            {
                if (!sparksAt(app, cpu, car, point, at(app, car + kCarSpeed)))
                    return std::nullopt;
            }
            else if (!burstAt(app, cpu, scrapeKind, point))
                return std::nullopt;
            if (!crashShake(app, cpu, car, 0x10, false) || !crashNoise(app, cpu, car, 2))
                return std::nullopt;
            break;
        case 0x50000:
        {
            if (!crashShake(app, cpu, car, 0x18, false))
                return std::nullopt;
            const reg32 many = bitsOf(float(fp::mul(f32(app, car + kCarHit), f64(app, kSparksByHit))));
            if (!sparksAt(app, cpu, car, point, many)
                || !burstAt(app, cpu, 2, point))
                return std::nullopt;
            sound = other;
            cpu.ecx = other;
            if (!crashNoise(app, cpu, car, 4))
                return std::nullopt;
            break;
        }
        case 0x60000:
            sound = other;
            if (other != 0xb && other != 9
                && (!crashShake(app, cpu, car, 0x10, false) || !burstAt(app, cpu, 2, point)))
                return std::nullopt;
            break;
        default:
            break;
        }
        if (!carEvent(app, cpu, car, 1, 0xffffffff, channel,
                      {sound, bitsOf(float(fp::mul(f32(app, car + kCarHit), f32(app, kCrashLoudness)))), 0}))
            return std::nullopt;
    }
    // What this part leaves in eax is overwritten by the next.
    if ((hit() & 0x7fffffff) != 0 && kind == 0x40000)
    {
        std::optional<reg32> shown;
        bool showing = true;
        if (!fBelow(f32(app, car + kCarSpeed), f32(app, kCrashSpeed)))
        {
            if (i32(at(app, car + kCarAboveRoad)) < i32(0x40400000))
            {
                const reg32 effect = at(app, other * 4 + kCrashEffects);
                if (effect == 3)
                {
                    const reg32 many = bitsOf(float(fp::mul(f32(app, car + kCarSpeed), f32(app, kSparksBySpeedHit))));
                    shown = sparksAt(app, cpu, car, point, many);
                    if (shown)
                        shown = carEvent(app, cpu, car, 1, 0xffffffff, 5, {0x12, 0, 0});
                }
                else
                {
                    cpu.edx = car;
                    shown = burstAt(app, cpu, effect, point);
                }
            }
            else
                shown = burstAt(app, cpu, 2, point);
        }
        else
        {
            if (fBelowOrEqual(f32(app, car + kCarSpeed), f64(app, kLandingHard)))
                showing = !fBelowOrEqual(f32(app, car + kCarHit), f64(app, kLandingHard));
            if (showing)
                shown = burstAt(app, cpu, 6, point);
        }
        if (showing && !shown)
            return std::nullopt;
    }
    const auto scrapeOff = [&]() -> bool {
        if (byte(app, car + kCarScrapes + 2) == 0)
            return true;
        if (!carEvent(app, cpu, car, 5, 0xa, 5, {1, 0, 0}))
            return false;
        putByte(app, car + kCarScrapes + 2, 0);
        return true;
    };
    if (fBelow(f32(app, car + kCarAboveRoad), f64(app, kScrapeHeight)) && (at(app, car + kCarLean) & 0x7fffffff) != 0)
    {
        if (fAbove(f32(app, car + kCarSpeed), f64(app, kSparksByHit))
            && fBelow(f32(app, car + kCarLean), f64(app, kScrapeLean)))
        {
            if (!carEvent(app, cpu, car, 4, 0xa, 5, {scrapeSound, at(app, car + kCarSpeed), 0}))
                return std::nullopt;
            putByte(app, car + kCarScrapes + 2, 1);
        }
        else if (!scrapeOff())
            return std::nullopt;
        if (fBelow(f32(app, car + kCarLean), f64(app, kTouchLean)))
        {
            if (i32(at(app, car + kCarSpeed)) <= i32(0x40000000))
                return car;
            const reg32 down = frame - 0x1c, touch = frame - 0x28;
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, car + kCarUp,
                                                              f32At(app, car + kCarHalfSizes + 4), down);
            if (cpu.terminate)
                return std::nullopt;
            guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, car + kCarPlace, down, touch);
            if (cpu.terminate)
                return std::nullopt;
            if (scrapeKind == 3)
                return sparksAt(app, cpu, car, touch, at(app, car + kCarSpeed));
            return burstAt(app, cpu, scrapeKind, touch);
        }
        if ((hit() & 0x7fffffff) == 0 || kind != 0x30000)
            return car;
        const float sum = float(fp::add(f32(app, car + kCarSpeed), f32(app, car + kCarHit)));
        if (fBelowOrEqual(Float(sum), f32(app, kScrapeHit)))
            return statusInto(car, Float(sum), f32(app, kScrapeHit));
        if (scrapeKind == 3)
            return sparksAt(app, cpu, car, point, bitsOf(sum));
        return burstAt(app, cpu, scrapeKind, point);
    }
    if (!scrapeOff())
        return std::nullopt;
    return car;
}

/* 0x42ac40: crashEffectsOf car eax. */
void crashEffects(App* app, CPU& cpu)
{
    const std::optional<reg32> left = crashEffectsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x42b480's: the car just put back (the frames since counting up from 1): a sparkle where it is
 * (+0x8d8 by its axes, on its place: 0x4caea0 past 64 frames, else 0x4cae00 with 9 past 16, 6
 * before) unless 0x4396e0 says no of its driver's +0x14 (0 to 49); the count ended once it is
 * settled: racing, leaning 0.5 or more (by its bits), its spin about +0xe4 and +0xec each under
 * 0x537310.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<reg32> respawnEffectsOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 since = at(app, car + kCarSinceRespawn);
    if (i32(since) > 0)
        put(app, car + kCarSinceRespawn, since + 1);
    reg32 left = car;
    if (at(app, car + kCarSinceRespawn) != 0)
    {
        left = at(app, car + kCarDriver);
        bool shown = true;
        const reg32 index = at(app, at(app, car + kCarDriver) + kDriverIndex);
        if (i32(index) >= 0 && i32(index) <= 0x31)
        {
            const std::optional<reg32> no =
                guest::direct<0x4396e0, load::makerFlag>(app, cpu, at(app, at(app, car + kCarDriver) + kDriverIndex));
            if (!no)
                return std::nullopt;
            left = *no;
            if (*no != 0)
                shown = false;
        }
        if (shown)
        {
            const reg32 offset = car + kCarRespawnOffset, axes = car + kCarAxes;
            Float turned[3];
            for (reg32 k = 0; k < 3; ++k)
                turned[k] = fp::mul(f32(app, offset), f32(app, axes + 4 * k));
            for (reg32 row = 1; row < 3; ++row)
            {
                Float by[3];
                for (reg32 k = 0; k < 3; ++k)
                    by[k] = fp::mul(f32(app, offset + 4 * row), f32(app, axes + 0xc * row + 4 * k));
                for (reg32 k = 0; k < 3; ++k)
                    turned[k] = fp::add(turned[k], by[k]);
            }
            reg32 where[3];
            for (reg32 k = 0; k < 3; ++k)
                where[k] = bitsOf(float(fp::add(Float(float(turned[k])), f32(app, car + kCarPlace + 4 * k))));
            const reg32 frames = at(app, car + kCarSinceRespawn);
            std::optional<reg32> sparkled;
            if (i32(frames) > 0x40)
            {
                cpu.eax = car;
                sparkled = callPushing(app, cpu, kSparkle, {where[0], where[1], where[2]});
            }
            else
            {
                cpu.eax = i32(frames) > 0x10 ? 9 : 6;
                sparkled = callPushing(app, cpu, kSparkleBurst, {where[0], where[1], where[2]});
            }
            if (!sparkled)
                return std::nullopt;
            left = *sparkled;
        }
    }
    if (at(app, car + kCarSinceRespawn) == 0 || half(app, car + kCarStage) > 0
        || i32(at(app, car + kCarLean)) < i32(0x3f000000))
        return left;
    for (const reg32 about : {car + kCarSpin, car + kCarSpin + 8})
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
void respawnEffects(App* app, CPU& cpu)
{
    const std::optional<reg32> left = respawnEffectsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x42b660's: the car's wheels' marks cut (0x4db860), its skid and scrape sounds stopped (0x4284e0
 * with 3, 8, 6; 5, 10, 6; 5, 10, 5 and 1, 0, 0), its shake cleared.  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<reg32> stopCarEffectsOf(App* app, CPU& cpu, reg32 car)
{
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEbp);
    for (reg32 wheel = 0; wheel < 4; ++wheel)
    {
        if (!call(app, cpu, kCutMark, at(app, car + kCarNumber), wheel))
            return std::nullopt;
        putByte(app, car + wheel + kCarMarks, 0);
        putByte(app, car + wheel + kCarMarkSounds, 0);
    }
    reg32 left = car + 3;
    struct Scrape
    {
        reg32 edx, ebx, ecx;
    };
    static constexpr Scrape kScrapes[] = {{3, 8, 6}, {5, 0xa, 6}, {5, 0xa, 5}};
    for (reg32 k = 0; k < 3; ++k)
    {
        if (byte(app, car + kCarScrapes + k) == 0)
            continue;
        cpu.eax = car;
        cpu.edx = kScrapes[k].edx;
        cpu.ebx = kScrapes[k].ebx;
        cpu.ecx = kScrapes[k].ecx;
        const std::optional<reg32> stopped = callPushing(app, cpu, kCarEvent, {1, 0, 0});
        if (!stopped)
            return std::nullopt;
        left = *stopped;
        putByte(app, car + kCarScrapes + k, 0);
    }
    for (reg32 k = 0; k < 0x14; k += 4)
        put(app, car + kCarShake + k, 0);
    put(app, car + kCarWaves + kWaveFrames, 0);
    put(app, car + kCarWaves + kWaveSize + kWaveFrames, 0);
    put(app, car + kCarWaves, 0);
    put(app, car + kCarWaves + kWaveSize, 0);
    put(app, car + kCarShakeToo, 0);
    return left;
}

/* 0x42b660: stopCarEffectsOf car eax. */
void stopCarEffects(App* app, CPU& cpu)
{
    const std::optional<reg32> left = stopCarEffectsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x42b780's: the car's effects a frame: respawnEffectsOf, crashEffectsOf, wheelEffectsOf,
 * updateShakeOf.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<reg32> carEffectsOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 8, 0);  // as deep as the code's, for its callees' frames
    const Kept kept(cpu, kSaveEdx | kSaveEbp);
    const Calling calling(cpu);
    if (!respawnEffectsOf(app, cpu, car))
        return std::nullopt;
    if (!crashEffectsOf(app, cpu, car) || !wheelEffectsOf(app, cpu, car))
        return std::nullopt;
    return updateShakeOf(app, cpu, car);
}

/* 0x42b780: carEffectsOf car eax. */
void carEffects(App* app, CPU& cpu)
{
    const std::optional<reg32> left = carEffectsOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* A dot product (0x4e01f0) of the vectors at `a` and `b`, by address: eax as the callee leaves it
 * is the caller's to read in cpu (vertices::dotProduct does not give it back).  Nullopt when the
 * game stopped. */
std::optional<Float> dotLeavingEax(App* app, CPU& cpu, reg32 a, reg32 b)
{
    if (!call(app, cpu, kDot, a, b))
        return std::nullopt;
    return cpu.fpu.take();
}

/* The same as C++ (guest::direct), where nothing reads eax after it. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* 0x42b7b0's: the car in the air (+0x8b), not to be put back, not crashing, falling (its
 * velocity against its ground's up, 0x4e01f0, over 0): its spin as its axes see it (+0x4f8..
 * +0x500) taken by the air's tuning -- the player's steered by +0x4e0, +0x4e8 -- and turned back
 * into the world's (by the axes transposed into the frame, 0x4e0110).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<reg32> airborneSpinOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x10, 0x24);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEbp);
    if (byte(app, car + kCarInAir) == 0 || at(app, car + kCarRespawnSlice) != 0
        || (at(app, car + kCarHit) & 0x7fffffff) != 0)
        return car;
    const std::optional<Float> falling = dotLeavingEax(app, cpu, car + kCarVelocity, car + kCarGroundUp);
    if (!falling)
        return std::nullopt;
    if (!fBelow(0.0, *falling))
        return statusInto(cpu.eax, 0.0, *falling);
    const reg32 spin = car + kCarSpin, seen = car + kCarSpinSeen;
    for (reg32 k = 0; k < 8; k += 4)
    {
        const std::optional<Float> part = dot(app, cpu, spin, car + kCarAxes + 3 * k);
        if (!part)
            return std::nullopt;
        putFloat(app, seen + k, float(*part));
    }
    const std::optional<Float> ahead = dot(app, cpu, spin, car + kCarAxes + 0x18);
    if (!ahead)
        return std::nullopt;
    const Float acrossSpin = fp::mul(f32(app, kAirSpinX), f32(app, seen));
    putFloat(app, seen + 8, float(*ahead));
    const reg8 flags = byte(app, car + kCarFlags);
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
    const reg32 axes = frame - 0x24;
    if (!call(app, cpu, kTranspose, car + kCarAxes, axes))
        return std::nullopt;
    // The last one's eax is what the code leaves.
    for (reg32 k = 0; k < 0xc; k += 4)
    {
        const std::optional<Float> part = k < 8 ? dot(app, cpu, seen, axes + 3 * k)
                                                : dotLeavingEax(app, cpu, seen, axes + 3 * k);
        if (!part)
            return std::nullopt;
        putFloat(app, spin + k, float(*part));
    }
    return cpu.eax;
}

/* 0x42b7b0: airborneSpinOf car eax. */
void airborneSpin(App* app, CPU& cpu)
{
    const std::optional<reg32> left = airborneSpinOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x42b900's: the car's frame of race state: its reset's wait run down; while it is waiting to
 * start (+0x88, or not +0x89) put back on its slice if stuck or asked (respawnCarOf), its effects
 * stopped, its crash cleared (0x20 bytes at +0x168); else its wheels on the track (wheelCellsOf),
 * its crash's response (0x406120, 0x404a00), its spin in the air, the objects, the stuck count,
 * its effects, and its crash cleared.  eax as the code leaves it; nullopt when the game stopped. */
std::optional<reg32> updateCarOf(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x10, 0);  // as deep as the code's, for its callees' frames
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEbp);
    const reg32 wait = at(app, car + kCarResetWait);
    put(app, car + 0x7dc, 0);
    if (wait != 0)
        put(app, car + kCarResetWait, wait - 1);
    if (byte(app, car + kCarOut) != 0 || byte(app, car + kCarIn) == 0)
    {
        if (i32(at(app, car + kCarStuck)) > 0 || at(app, car + kCarRespawnSlice) != 0)
        {
            put(app, car + kCarStuck, 0);
            const Calling calling(cpu);
            if (!respawnCarOf(app, cpu, car))
                return std::nullopt;
        }
        put(app, car + kCarSinceRespawn, 0);
        {
            const Calling calling(cpu);
            if (!stopCarEffectsOf(app, cpu, car))
                return std::nullopt;
        }
        put(app, car + kCarResetWait, 0);
        return call(app, cpu, kFill, car + kCarHit, 0x20);
    }
    {
        const Calling calling(cpu);
        if (!wheelCellsOf(app, cpu, car))
            return std::nullopt;
    }
    if ((at(app, car + kCarHit) & 0x7fffffff) != 0 && at(app, car + kCarHit + 4) != 0)
    {
        guest::direct<0x406120, aidrive::reverseWhenSlowDone>(app, cpu, car);
        if (cpu.terminate || !guest::direct<0x404a00, airace::carContactDone>(app, cpu, car))
            return std::nullopt;
    }
    const Calling calling(cpu);
    if (!airborneSpinOf(app, cpu, car) || !objectContactsOf(app, cpu, car) || !stuckCheckOf(app, cpu, car))
        return std::nullopt;
    const std::optional<reg32> left = carEffectsOf(app, cpu, car);
    if (!left)
        return std::nullopt;
    put(app, car + kCarHit, 0);
    put(app, car + kCarHit + 4, 0);
    put(app, car + kCarCrash, 0);
    return left;
}

/* 0x42b900: updateCarOf car eax. */
void updateCar(App* app, CPU& cpu)
{
    const std::optional<reg32> left = updateCarOf(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* A slice's half width across (a byte at +0x17 the one side's, at +0x1b the other's, as 16.16
 * halved the way the code halves it: towards 0). */
reg32 halfWidth(App* app, reg32 slicePointer, reg32 field)
{
    const reg32 width = reg32(byte(app, at(app, kSlices) + at(app, slicePointer) * kSliceSize + field)) << 15;
    return reg32(i32(width - reg32(i32(width) >> 31)) >> 1);
}

/* 0x42ba00's: grid place `place` worked out: its slice to [slice] -- 3 a row from the start,
 * back from the last car's when the race runs backwards -- and its place to `where`: across by
 * the slice's half widths (one side or the other by the place's odd or even, swapped backwards;
 * where 0x40eca0 says the road is wide enough for two, by half its lanes' widths and the half
 * width again), 0x8000 up, 0 ahead.  What the code leaves in eax and edx; nullopt when the game
 * stopped. */
std::optional<Left> defaultGridPlaceOf(App* app, CPU& cpu, reg32 place, reg32 slice, reg32 where)
{
    const Kept kept(cpu, kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 backwards = gameSettings(app).backwards;
    reg32 row;
    if (backwards != 0)
    {
        const reg32 from = gameSettings(app).humanCount + gameSettings(app).opponentCount - 1 - place;
        row = from * 4 - from;
    }
    else
        row = place * 4 - place;
    put(app, slice, row + 1);
    reg32 side = place & 1;
    if (backwards != 0)
        side = side == 0 ? 1 : 0;
    // eax at the call as the code leaves it there: a stand-in of the check's hands it back now and then.
    cpu.eax = backwards;
    const reg32 wide = guest::direct<0x40eca0, aiplan::trackRoadWide>(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    reg32 across, second;  // and what the code leaves in edx
    if (wide != 0)
    {
        const reg32 record = at(app, kSlices) + at(app, slice) * kSliceSize;
        const reg32 lanes = byte(app, record + kSliceLanes) & 0xf;
        const reg32 width = reg32(byte(app, record + kSliceRightWidth)) << 15;
        const reg32 lanesAcross = reg32(i32(lanes) >> 1) * width;
        across = reg32(i32(width - reg32(i32(width) >> 31)) >> 1);
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
        const reg32 record = at(app, kSlices) + at(app, slice) * kSliceSize;
        const reg32 width = side != 0 ? reg32(byte(app, record + kSliceRightWidth)) << 15
                                      : 0u - (reg32(byte(app, record + kSliceLeftWidth)) << 15);
        second = reg32(i32(width) >> 31);
        across = reg32(i32(width - second) >> 1);
    }
    put(app, where, across);
    put(app, where + 8, 0);
    put(app, where + 4, kGridUp);
    return Left{across, second};
}

/* 0x42ba00: defaultGridPlaceOf place eax, to [edx] and [ebx]. */
void defaultGridPlace(App* app, CPU& cpu)
{
    const std::optional<Left> left = defaultGridPlaceOf(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.edx = left->edx;
    cpu.esp += 4;
}

/* 0x42bb80's: the start's grid, 12 places of 3 words at kGrid: from the track's start.dat
 * (kTrackFolder + "TRK%03x\start.dat" of the track, sprintf to a buffer of the frame's; whether
 * it is there 0x4e0e60, opened 0x4dc0e0, read a word at a time 0x4dc1e0, closed 0x4dc1b0), else
 * each worked out (defaultGridPlaceOf, into words of the frame's).  eax as the code leaves it;
 * nullopt when the game stopped. */
std::optional<reg32> loadStartGridOf(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0x210);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEbp);
    const reg32 path = frame - 0x210;
    const reg32 track = gameSettings(app).track;
    cpu.eax = path;
    cpu.edx = track;
    if (!callPushing(app, cpu, kSprintf, {path, kStartFile, kTrackFolder, track}))
        return std::nullopt;
    const std::optional<bool> there = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, path);
    if (!there)
        return std::nullopt;
    if (*there)
    {
        const std::optional<reg32> file = call(app, cpu, kOpenFile, path, 0, 1);
        if (!file)
            return std::nullopt;
        for (reg32 place = 0; place < kGridPlaces; ++place)
        {
            const reg32 entry = place * 3 * 4;
            for (const reg32 word : {kGrid, kGridY, kGridZ})
            {
                const std::optional<reg32> value = call(app, cpu, kReadWord, *file);
                if (!value)
                    return std::nullopt;
                put(app, entry + word, *value);
            }
        }
        return call(app, cpu, kCloseFile, *file);
    }
    const reg32 slice = frame - 4, where = frame - 0x10;
    reg32 left = 0;
    for (reg32 place = 0; place < kGridPlaces; ++place)
    {
        const Calling calling(cpu);
        if (!defaultGridPlaceOf(app, cpu, place, slice, where))
            return std::nullopt;
        const reg32 entry = place * 3 * 4;
        put(app, entry + kGrid, at(app, slice));
        put(app, entry + kGridY, at(app, where));
        left = at(app, where + 8);
        put(app, entry + kGridZ, left);
    }
    return left;
}

/* 0x42bb80: loadStartGridOf. */
void loadStartGrid(App* app, CPU& cpu)
{
    const std::optional<reg32> left = loadStartGridOf(app, cpu);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x42bc70's: grid place `place`: its slice to [slice], its place to `where` -- across, 0x8000
 * up, ahead -- across mirrored when kGridMirrored is 1.  What the code leaves in eax and edx:
 * the place's ahead, the mirrored flag. */
Left gridPlaceOf(App* app, reg32 place, reg32 slice, reg32 where)
{
    const reg32 entry = place * 3 * 4;
    put(app, slice, at(app, entry + kGrid));
    put(app, where, at(app, entry + kGridY));
    const reg32 ahead = at(app, entry + kGridZ);
    put(app, where + 4, kGridUp);
    const reg32 mirrored = at(app, kGridMirrored);
    put(app, where + 8, ahead);
    if (mirrored == 1)
        put(app, where, 0u - at(app, where));
    return {ahead, mirrored};
}

/* 0x42bc70: gridPlaceOf place eax, to [edx] and [ebx]. */
void gridPlace(App* app, CPU& cpu)
{
    const Left left = gridPlaceOf(app, cpu.eax, cpu.edx, cpu.ebx);
    cpu.eax = left.eax;
    cpu.edx = left.edx;
    cpu.esp += 4;
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
bool placeOnGridOf(App* app, CPU& cpu, reg32 car, reg32 number)
{
    const GuestFrame frame(cpu, 0x14, 0x2c);
    const Kept kept(cpu, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi | kSaveEbp);
    const reg32 place = frame - 0x2c, slice = frame - 0x14;
    reg32 words[3];
    float scaled;
    if ((byte(app, car + kCarFlags) & kThisMachines) != 0)
    {
        reg32 scale = kOneBits;
        if (gameSettings(app).raceOptionHazard != 0)
            scale = at(app, kPlayerScale);
        if (gameSettings(app).playMode == 0 && (byte(app, car + kCarFlags) & kChased) != 0)
            scale = at(app, kPlayerScale);
        if ((std::uint8_t(gameSettings(app).raceFlags) & kStartRecords) != 0)
            for (reg32 k = 0; k < 0xc; k += 4)
                put(app, at(app, car + kCarDriver) + kDriverStart + k,
                    at(app, at(app, car + kCarStuckRecord) + kRecordStart + k));
        const reg32 driver = at(app, car + kCarDriver);
        for (reg32 k = 0; k < 3; ++k)
            words[k] = at(app, driver + kDriverStart + 4 * k);
        scaled = float(fp::mul(f32(app, at(app, car + kCarHandling) + kHandlingScaled), Float(floatOf(scale))));
    }
    else
    {
        float scale = 1.0f;
        if ((byte(app, car + kCarFlags) & kPlayersCar) != 0)
            scale = float(fp::div(1.0, f32(app, kHumanScale)));
        const reg32 record = at(app, car + kCarStuckRecord);
        for (reg32 k = 0; k < 3; ++k)
            words[k] = at(app, record + kRecordStart + 4 * k);
        scaled = float(fp::mul(f32(app, at(app, car + kCarHandling) + kHandlingScaled), Float(scale)));
    }
    cpu.eax = car;
    cpu.edx = number;
    if (!callPushing(app, cpu, kScaleHandling, {bitsOf(scaled), words[0], words[1], words[2]}))
        return false;
    const reg8 flags = byte(app, car + kCarFlags);
    if ((flags & kThisMachines) != 0 && (flags & kChased) != 0)
    {
        if (!guest::direct<0x404d30, aidrive::placeCopAtStart>(app, cpu, car, slice, place))
            return false;
    }
    else if ((byte(app, car + kCarFlags) & (kWreck | kChased)) != 0)
    {
        for (reg32 k = 0; k < 0xc; k += 4)
            put(app, place + k, 0);
        put(app, slice, 0x64);
    }
    else
    {
        const Calling calling(cpu);
        gridPlaceOf(app, gameSettings(app).gridSlots[at(app, car + kCarIndex)], slice, place);
    }
    if (!call(app, cpu, kPutOnRoad, car, at(app, slice), place, gameSettings(app).backwards))
        return false;
    put(app, car + kCarLapState, 0);
    put(app, car + kCarBackwards, gameSettings(app).backwards != 0 ? 1 : 0);
    put(app, car + kCarStartSlice, at(app, car + kCarSlice));
    guest::direct<0x42ce10, raceload::setRaceDistance>(app, cpu, car);
    if (cpu.terminate)
        return false;
    // Its counts, flags and effects, in the code's order (a byte where it writes one).
    struct Cleared
    {
        reg32 offset, value;
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
            putByte(app, car + c.offset, reg8(c.value));
        else
            put(app, car + c.offset, c.value);
    }
    struct Fill
    {
        reg32 offset, size;
    };
    static constexpr Fill kFills[] = {{0x62c, 4}, {0x630, 0xac}, {0x6dc, 4}, {0x6e0, 0x2c}, {0x70c, 0xa4}};
    for (const Fill& f : kFills)
        if (!call(app, cpu, kFill, car + f.offset, f.size))
            return false;
    if (!byAddress(app, cpu, kLoadingStop))
        return false;
    put(app, car + 0x7dc, 0);
    if (!call(app, cpu, kFill, car + 0x7e0, 0x18))
        return false;
    put(app, car + 0x7d8, 0);
    put(app, car + kCarStuck, 0);
    const reg32 record = at(app, car + kCarStuckRecord);
    reg32 after[3];
    for (reg32 k = 0; k < 3; ++k)
        after[k] = at(app, car + kCarStuckRecord + 4 + 4 * k);
    if (!call(app, cpu, kFill, car + kCarStuckRecord, 0x104))
        return false;
    for (reg32 k = 0; k < 3; ++k)
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
void placeOnGrid(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    if (!placeOnGridOf(app, cpu, car, cpu.edx))
        return;
    cpu.eax = car;
    cpu.edx = car;
    cpu.esp += 4;
}

}  // namespace racestart
}  // namespace nfs3hp
