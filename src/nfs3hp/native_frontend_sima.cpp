#include "native_frontend_sima.h"
#include "port_conv.h"
#include "native_racestart.h"
#include "native_carstate.h"
#include "game_settings.h"
#include "native_aiplan.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudx.h"
#include "native_frontend_tail.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "port_hud.h"
#include "native_watcom.h"
#include <nfs3hp.h>
#include <cstring>

/* The simulation's code that was left under 0x4a0aa0, decompiled:
 *
 *   the opponents    the pull of an engine at its speed (sub_4064f0), how far the
 *                    way a driver wants to go is from the way it points
 *                    (sub_4070c0), the lane votes acted on (sub_40de30);
 *   the race cameras the step of each camera mode, by the modes' table
 *                    (sub_426450): beside the road (sub_425060, sub_425bf0), the
 *                    nearest fixed spot (sub_4251f0), the animated paths (sub_4253d0,
 *                    sub_425640), the flyover (sub_425d50), the wait of a mode that
 *                    hands on to another (sub_4262a0, sub_426310); the modes a car's
 *                    state forces (sub_426560) and the step of all views (sub_426680);
 *   the cars         how far a car is from the players and what it does about it
 *                    (sub_4a0580), its frame on the track (sub_49ffa0, sub_49fee0),
 *                    set back onto the road (sub_4a0140), the frame's work on it from
 *                    the race's table of a car's functions (sub_4a08c0), and what
 *                    these use: a quick distance (sub_4965b0), a quick length in the
 *                    plane (sub_4967e0), whether a car leans over (sub_49fde0).
 *
 * This runs every frame and the replays must stay identical to the bit, so the
 * x87's arithmetic is written out through fp:: in the original's order, each value
 * stored and fetched back where the original stores and fetches it: a float kept
 * in memory has lost the width one left on the x87 still has.  The state stays in
 * guest memory, and the locals the game's functions read are where they were in the
 * frame.
 *
 * Plan step 5: each function a typed core in frontend::sima (native_frontend_sima.h), in
 * a frame of its own (fe::Frame: what it calls runs at the stack addresses it did), and an
 * thunk under the game's name for the calls by address.  A core of the simulation's files
 * calls another as C++ (guest::direct), and the typed cores of other files too (fe::direct);
 * the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* NFS_CAR_TRACE's records of a driver's lane votes (native_vertices.cpp). */
bool aiVotes(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers);

namespace
{

using fe::args;
using fe::Guest;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::s32;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(u16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(u16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered
bool jumpsEqual(u16 status) { return (status & 0x4000) != 0; }  // je; true unordered

/* eax after fnstsw ax: only its low half changes. */
u32 withStatus(u32 eax, u16 status) { return (eax & 0xffff0000) | status; }

u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A float loaded on the x87 and stored back, by way of a double in memory: the same but for
 * a signalling NaN, quieted.  Written out, for a compiler folds float(double(x)) to x. */
float throughX87(float value)
{
    const u32 bits = bitsOf(value);
    const bool signalling = (bits & 0x7f800000) == 0x7f800000 && (bits & 0x007fffff) != 0 && (bits & 0x00400000) == 0;
    return signalling ? floatOf(bits | 0x00400000) : value;
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, u32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, u32 address) { return Float(s32(guest.dword(address))); }

/* The original's own absolute value, inline at each of its uses: fldz; fcomp x and
 * the sign changed only below 0, so that -0 stays -0 and a NaN's bits stay as they
 * are.  `status` is what the comparison left, for the eax some callers go on with. */
float absolute(float value, u16& status)
{
    status = fp::compare(0.0, Float(value));
    return jumpsAbove(status) ? float(-Float(value)) : value;
}

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* An index past the end of a ring of `count`, brought back once. */
s32 wrapped(s32 index, s32 count) { return index >= count ? index - count : index; }

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The track's slices.
constexpr u32 kSliceCount = 0x5dd940;
constexpr u32 kSlices = 0x5dd948;
constexpr u32 kSliceBytes = 36;
constexpr u32 kSliceFlags = 0x0c;          // word: the road's bits, for whether a camera may stay
constexpr u32 kSliceLimits = 0x0e;         // byte: which of the road's two limits this slice holds a car to

u32 slice(const Guest& guest, u32 index) { return guest.dword(kSlices) + index * kSliceBytes; }

// The race.
constexpr u32 kCars = 0x5efa48;            // u32[]: the cars, the players first
constexpr u32 kCarsByPlace = 0x5efac8;     // u32[]
constexpr u32 kLocalCars = 0x5efda4;       // how many cars players drive here
constexpr u32 kNetworkGame = 0x7a22da;     // word

// A car.
constexpr u32 kCarPlaceOnTrack = 0x0c;     // what the track's functions take
constexpr u32 kCarNode = 0x1c;
constexpr u32 kCarDistance = 0x84;         // float: from the nearest player
constexpr u32 kCarAsleep = 0x88;           // byte: past the players' reach
constexpr u32 kCarInRace = 0x89;           // byte
constexpr u32 kCarX = 0x98, kCarY = 0x9c, kCarZ = 0xa0;
constexpr u32 kCarMatrix = 0xc0;
constexpr u32 kCarHalfWidth = 0x114, kCarHalfHeight = 0x118, kCarHalfLength = 0x11c;
constexpr u32 kCarSurface = 0x188, kCarSurfaceKind = 0x18c;
constexpr u32 kCarFlags = 0x200;           // byte

// A race camera.
constexpr u32 kCameraCar = 0x04;
constexpr u32 kCameraView = 0x0c;          // 0 or 1: the player's half of the screen
constexpr u32 kCameraX = 0x14, kCameraY = 0x18, kCameraZ = 0x1c;
constexpr u32 kCameraTarget = 0x20;
constexpr u32 kCameraMatrix = 0x2c;
constexpr u32 kCameraZoom = 0x54;
constexpr u32 kCameraNode = 0x84;
constexpr u32 kCameraSurface = 0xec;
// What a mode keeps for itself: a path's started, kind and animation; a wait's time, tick and next mode.
constexpr u32 kCameraOwn110 = 0x110, kCameraOwn114 = 0x114, kCameraOwn118 = 0x118, kCameraOwn11c = 0x11c;

constexpr u32 kCamerasOfModes = 0x552ec8;  // u32[view + mode * 2]: each mode's camera of each view
constexpr u32 kCameraNow = 0x5e10b0;       // u32[2]: each view's camera
constexpr u32 kCameraForced = 0x5e10b8;    // u32[2]: the one a forced mode has put over it, or 0
constexpr u32 kFirstCycleMode = 0x5e1054;

// The game's functions, by address.
constexpr u32 kSumLaneVotes = 0x40da30, kSteerToLane = 0x40daf0;
constexpr u32 kWayToGo = 0x406e50;         // eax the car, esi where its floats go
constexpr u32 kArcTangent = 0x4e06d9;      // st(1) by st(0), into st(0)
constexpr u32 kTruncate = 0x4dfd56;
constexpr u32 kRoadBitsBroken = 0x423fe0;
constexpr u32 kLanePosition = 0x41ffd0;    // eax a place on the track, edx three ints, ebx the side; a surface
constexpr u32 kLookAtTarget = 0x422290;
constexpr u32 kNearestSpot = 0x423d90;
constexpr u32 kSetCameraMode = 0x4238b0, kSetForcedMode = 0x423960, kSwitchCamera = 0x422610, kModeInCycle = 0x423ab0;
constexpr u32 kAnimationPlaying = 0x40fac0, kPathEndPose = 0x40f8d0;
constexpr u32 kScaleVectors = 0x4e0050, kAddVectors = 0x4dffb0;
constexpr u32 kAttachCameras = 0x4215c0;
constexpr u32 kMemmove = 0x4ea4f0;
constexpr u32 kNodeGap = 0x40e120;
constexpr u32 kCarNodeNow = 0x49fe70, kCarHeightOverRoad = 0x4a00d0, kCarLowestPoint = 0x4a0440, kClearRespawn = 0x4294c0;

}  // namespace

// ---------------------------------------------------------------------------
// The opponents
// ---------------------------------------------------------------------------

/* sub_40de30: an opponent's lane votes added up and acted on. */
nfs::u32 frontend::sima::laneSteer(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    fe::direct<kSumLaneVotes, aiplan::takeLane>(guest.app(), guest.ts(), car);
    fe::direct<kSteerToLane, aiplan::steerForLane>(guest.app(), guest.ts(), car);
    return car;  // eax as the thunk of 0x40daf0 leaves it: the car it was handed
}

/* sub_4064f0: the pull of an opponent's engine at its speed, onto the x87. */
frontend::sima::FloatLeft frontend::sima::enginePullOf(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    constexpr std::uint8_t kCarLimited = 0x01, kCarInTraffic = 0x10, kCarSlowed = 0x20;
    constexpr u32 kDriving = 0x528;
    constexpr u32 kLimitShare = 0x1cc, kPullBySpeed = 0x2ec;
    constexpr u32 kThrottle = 0x7bc, kPedal = 0x7c0, kSpeed = 0x7c4;
    constexpr u32 kMinusHalf = 0x536254, kTenthOff = 0x53625c, kBoost = 0x536260, kTwice = 0x536264, kThrice = 0x536268;
    constexpr u32 kMostWhenStill = 0x53626c, kFastEnough = 0x536274, kHalf = 0x53627c;

    guest.setByte(car + 0x509, 0);
    if (guest.byte(car + kCarFlags) & kCarLimited)
    {
        // Held back between its limit and the limit's share of the throttle.
        const u32 limitBits = guest.dword(guest.dword(car + kDriving) + kLimitShare);
        const Float limit = Float(floatOf(limitBits));
        u16 status = fp::compare(fp::mul(limit, floatFrom(guest, car + kThrottle)), floatFrom(guest, car + kPedal));
        if (jumpsBelow(status))
            return { 0.0, withStatus(limitBits, status) };
        status = fp::compare(floatFrom(guest, car + kPedal), limit);
        if (jumpsAbove(status))
            return { 0.0, withStatus(limitBits, status) };
    }
    // The speed in m/s, a row of the car's table.
    s32 row = fp::toInteger<s32>(fp::add(floatFrom(guest, car + kSpeed), doubleAt(guest, kMinusHalf)));
    if (row > 0x6f)
        row = 0x6f;
    if (row <= 0)
        row = 0;
    float pull = guest.floatAt(guest.dword(car + kDriving) + u32(row) * 4 + kPullBySpeed);
    if (gameSettings(guest).seriesRace == 0 && s32(guest.dword(car + 8)) > 100)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kTenthOff)));
    if (gameSettings(guest).weather != 0 && row < 45)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kTenthOff)));
    if (s32(guest.dword(car + 0x6e4)) > 0)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kBoost)));
    // The network's catch-up: a car of no player here, far from the player.
    if (gameSettings(guest).networkGame != 0)
    {
        const u32 driver = guest.dword(car + 0x1f0);
        if (driver != gameSettings(guest).localPlayer
            && (guest.word(kNetworkGame) == 0 || s32(driver) < s32(gameSettings(guest).humanCount)))
        {
            const u32 player = guest.dword(kCars + gameSettings(guest).localPlayer * 4);
            const s32 gap = s32(guest.dword(car + kCarNode) - guest.dword(player + kCarNode));
            if (gap <= -3 || gap >= 15)
            {
                const u32 factor = gap > -8 && gap < 30 ? kTwice : kThrice;
                pull = float(fp::mul(Float(pull), floatFrom(guest, factor)));
            }
        }
    }
    if ((guest.byte(car + kCarFlags) & kCarInTraffic) && row < 2)
    {
        pull = jumpsAbove(fp::compare(Float(pull), doubleAt(guest, kMostWhenStill))) ? 12.0f : throughX87(pull);
    }
    if ((guest.byte(0x55eb2e) & 0x20) && (guest.byte(car + kCarFlags) & kCarSlowed)
        && jumpsAbove(fp::compare(floatFrom(guest, car + kSpeed), doubleAt(guest, kFastEnough))))
    {
        pull = float(fp::mul(Float(pull), doubleAt(guest, kHalf)));
    }
    return { double(pull), bitsOf(pull) };
}

/* sub_4070c0: how far the way the driver wants to go (sub_406e50) is from the way
 * its car points, in turns, to 0x56ec60; the turn it means to make to 0x56ec64 and
 * the way itself to 0x56ec5c. */
nfs::u32 frontend::sima::headingError(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16 + 0x30);
    constexpr u32 kPlannedTurn = 0x210;
    constexpr u32 kWanted = 0x56ec64, kWay = 0x56ec5c, kError = 0x56ec60, kErrorSpare = 0x56ec68;
    constexpr u32 kTurnsPerPositive = 0x5362d4, kTurnsPerNegative = 0x5362dc, kHalfTurn = 0x5362e4;
    constexpr u32 kTurnsPerRadian = 0x5362ec, kMinusHalfTurn = 0x5362f4, kMinusOne = 0x5362fc;
    constexpr u32 kLocalAcross = 0x00, kLocalAlong = 0x08;

    const u32 frame = guest.frame();
    const u32 planned = guest.dword(car + kPlannedTurn);
    guest.setDword(kErrorSpare, 0);
    const Float wanted = s32(planned) > 0
        ? fp::mul(integerFrom(guest, car + kPlannedTurn), doubleAt(guest, kTurnsPerPositive))
        : fp::add(fp::mul(Float(s32(planned + 0x200)), doubleAt(guest, kTurnsPerNegative)), doubleAt(guest, kHalfTurn));
    guest.setFloat(kWanted, float(wanted));

    nfs::Args toGo;
    toGo.esi = frame;
    fx::callAt(guest, kWayToGo, args({ car }), &toGo);
    nfs::Args arc;
    arc.floatIn[1] = floatFrom(guest, frame + kLocalAlong);
    arc.floatIn[0] = floatFrom(guest, frame + kLocalAcross);
    arc.floatsIn = 2;
    const u32 eax = fx::callAt(guest, kArcTangent, fe::Arguments(), &arc);
    const Float way = fp::mul(arc.floatResult, doubleAt(guest, kTurnsPerRadian));
    guest.setFloat(kWay, float(way));
    if (jumpsAbove(fp::compare(0.0, way)))
        guest.setFloat(kWay, float(fp::add(1.0, way)));

    const Float error = fp::sub(floatFrom(guest, kWanted), floatFrom(guest, kWay));
    guest.setFloat(kError, float(error));
    if (jumpsBelow(fp::compare(error, doubleAt(guest, kMinusHalfTurn))))
        guest.setFloat(kError, float(fp::add(1.0, error)));

    const Float kept = floatFrom(guest, kError);
    const u16 status = fp::compare(kept, doubleAt(guest, kHalfTurn));
    if (jumpsAbove(status))
        guest.setFloat(kError, float(fp::add(kept, doubleAt(guest, kMinusOne))));
    return withStatus(eax, status);
}

// ---------------------------------------------------------------------------
// The race cameras
// ---------------------------------------------------------------------------

namespace
{

/* Whether the road's bits at either of two nodes have gone (sub_423fe0). */
bool roadBroken(Guest& guest, u32 cameraNode, u32 carNode)
{
    if (carstate::roadBitsGone(guest.word(slice(guest, cameraNode) + kSliceFlags)))
        return true;
    return carstate::roadBitsGone(guest.word(slice(guest, carNode) + kSliceFlags));
}

/* The race not yet under way on the screen, or its cameras frozen. */
bool cameraHeld(const Guest& guest)
{
    constexpr u32 kStartShown = 0x7d36a8, kStarted = 0x7cd960, kFrozen = 0x5600ec;
    if (guest.dword(kStartShown) != 0 && guest.dword(kStarted) == 0)
        return true;
    return guest.dword(kFrozen) != 0;
}

/* What the two lane cameras have in common: on the road and the road gone, the side
 * from the car's damage; the place in the lane there, in 16.16, to floats in `spot`,
 * the second one raised by [raise] if any. */
struct LaneView
{
    u32 surface;
    bool moved;
};

LaneView laneView(nfs::ThreadState& ts, Guest& guest, u32 camera, u32 sideField, u32 scaleAt, u32 raise, u32 ints,
                  u32 spot)
{
    constexpr u32 kCarDamage = 0x638;
    const u32 place = guest.dword(camera + kCameraCar) + kCarPlaceOnTrack;
    if (guest::direct<0x4bdc70, frontend::simc::tvShotOver>(guest.app(), ts, guest.dword(camera + kCameraView)) != 0
        && roadBroken(guest, guest.dword(camera + kCameraNode), guest.dword(place + kCarNode - kCarPlaceOnTrack)))
    {
        guest.setDword(camera + sideField, s32(guest.dword(guest.dword(camera + kCameraCar) + kCarDamage)) < 7 ? 0 : 1);
    }
    const u32 surface = fe::direct<kLanePosition, trackdata::laneNodePlaced>(guest.app(), guest.ts(), place, ints, guest.dword(camera + sideField));
    const Float scale = doubleAt(guest, scaleAt);
    const Float x = fp::mul(integerFrom(guest, ints), scale);
    const Float y = fp::mul(integerFrom(guest, ints + 4), scale);
    const Float z = fp::mul(scale, integerFrom(guest, ints + 8));
    guest.setFloat(spot, float(x));
    guest.setFloat(spot + 8, float(z));
    guest.setFloat(spot + 4, float(raise != 0 ? fp::add(y, floatFrom(guest, raise)) : y));
    const bool moved = !jumpsEqual(fp::compare(floatFrom(guest, camera + kCameraX), floatFrom(guest, spot)))
        || !jumpsEqual(fp::compare(floatFrom(guest, camera + kCameraY), floatFrom(guest, spot + 4)))
        || !jumpsEqual(fp::compare(floatFrom(guest, camera + kCameraZ), floatFrom(guest, spot + 8)));
    return { surface, moved };
}

}  // namespace

/* sub_425060: mode 8, beside the road near the car; a move past the place counts up,
 * and the third lets the zoom go. */
nfs::u32 frontend::sima::cameraBesideRoad(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x1c);
    constexpr u32 kSide = 0x130, kMoving = 0x124, kKeepZoom = 0x12c, kMoves = 0x553024, kScale = 0x536f04;
    constexpr u32 kLocalInts = 0x00, kLocalSpot = 0x0c;
    constexpr s32 kMovesToLetGo = 3;

    if (cameraHeld(guest))
        return camera;
    const u32 frame = guest.frame();
    const LaneView view = laneView(ts, guest, camera, kSide, kScale, 0, frame + kLocalInts, frame + kLocalSpot);
    if (view.moved)
    {
        const u32 moves = guest.dword(kMoves) + 1;
        guest.setDword(camera + kMoving, 0);
        guest.setDword(kMoves, moves);
        if (s32(moves) < kMovesToLetGo)
        {
            guest.setDword(camera + kKeepZoom, 1);
        }
        else
        {
            guest.setDword(camera + kKeepZoom, 0);
            guest.setDword(kMoves, 0);
            guest.setDword(camera + kCameraZoom, 0);
        }
        guest::direct<0x4bdbd0, simc::tvNextCamera>(app, ts, guest.dword(camera + kCameraView));
    }
    guest::direct<0x424fd0, simd::cameraMoveTowards>(app, ts, camera, frame + kLocalSpot, guest.dword(camera + kCameraOwn110));
    guest.setDword(camera + kCameraSurface, view.surface);
    fe::direct<kLookAtTarget, carstate::cameraLookedAt>(guest.app(), guest.ts(), camera, camera + kCameraX);
    return guest::direct<0x424f30, simd::cameraEaseTurn>(app, ts, camera);
}

/* sub_425bf0: mode 14, the same with a side and speeds of its own, higher up. */
nfs::u32 frontend::sima::cameraOverRoad(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x1c);
    constexpr u32 kScale = 0x536f2c, kRaise = 0x536f34;
    constexpr u32 kLocalInts = 0x00, kLocalSpot = 0x0c;

    if (cameraHeld(guest))
        return camera;
    const u32 frame = guest.frame();
    const LaneView view = laneView(ts, guest, camera, kCameraOwn11c, kScale, kRaise, frame + kLocalInts, frame + kLocalSpot);
    if (view.moved)
        guest::direct<0x4bdbd0, simc::tvNextCamera>(app, ts, guest.dword(camera + kCameraView));
    guest::direct<0x425b40, simd::cameraGlideTowards>(app, ts, camera, frame + kLocalSpot, guest.dword(camera + kCameraOwn110),
                                                      guest.dword(camera + kCameraOwn114));
    guest.setDword(camera + kCameraSurface, view.surface);
    fe::direct<kLookAtTarget, carstate::cameraLookedAt>(guest.app(), guest.ts(), camera, camera + kCameraX);
    return guest::direct<0x424f30, simd::cameraEaseTurn>(app, ts, camera);
}

/* sub_4251f0: mode 18, the nearest of the track's fixed spots, records of 28 bytes at
 * [0x552e50]: where it stands, its surface, whether it looks at the car or holds the
 * quaternion it has, and a zoom from its angle.  None near: mode 8. */
nfs::u32 frontend::sima::cameraNearestSpot(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x30);
    constexpr u32 kSpots = 0x552e50, kSpotBytes = 28;
    constexpr u32 kSpotSurface = 0x18, kSpotFlags = 0x16, kSpotAngle = 0x12;
    constexpr std::uint8_t kLooksAtCar = 0x01, kFollows = 0x02;
    constexpr u32 kScale = 0x536f0c, kIdentity = 0x564498, kLeastZoom = 0x536f14;
    constexpr u32 kCameraTargetCar = 8, kCameraLooking = 0x108;
    constexpr u32 kLocalQuaternion = 0x00, kLocalPartA = 0x10, kLocalPartB = 0x14;
    constexpr u32 kModeBesideRoad = 8;
    constexpr u32 kLeastZoomBits = 0x3f333333, kFollowBits = 0x40c00000;

    const u32 frame = guest.frame();
    const u32 spot = *fe::direct<kNearestSpot, carstate::spotNearestTo>(guest.app(), guest.ts(), guest.dword(camera + kCameraView));
    if (spot == u32(-1))
    {
        fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), guest.dword(camera + kCameraView), kModeBesideRoad);
        return guest::direct<0x426450, cameraStep>(app, ts, kModeBesideRoad, guest.dword(camera + kCameraView));
    }
    const u32 record = guest.dword(kSpots) + spot * kSpotBytes;
    guest.setDword(camera + kCameraSurface, guest.dword(record + kSpotSurface));
    const Float scale = doubleAt(guest, kScale);
    guest.setFloat(camera + kCameraX, float(fp::mul(integerFrom(guest, record), scale)));
    guest.setFloat(camera + kCameraY, float(fp::mul(integerFrom(guest, record + 4), scale)));
    guest.setFloat(camera + kCameraZ, float(fp::mul(scale, integerFrom(guest, record + 8))));
    if (guest.byte(record + kSpotFlags) & kLooksAtCar)
    {
        guest.setDword(camera + kCameraTargetCar, guest.dword(camera + kCameraCar));
        guest.setDword(camera + kCameraLooking, 1);
        copyDwords(guest, camera + kCameraMatrix, kIdentity, 9);
        fe::direct<kLookAtTarget, carstate::cameraLookedAt>(guest.app(), guest.ts(), camera, camera + kCameraX);
    }
    else
    {
        guest.setDword(camera + kCameraLooking, 0);
        // The quaternion: four int16 from +0xc, each in 14.2 of 16.16.
        for (u32 i = 0; i < 4; ++i)
        {
            const s32 part = (s32(guest.dword(record + 0x0a + 2 * i)) >> 16) * 4;
            guest.setFloat(frame + kLocalQuaternion + 4 * i,
                           float(i == 3 ? fp::mul(scale, Float(part)) : fp::mul(Float(part), scale)));
        }
        guest::direct<0x4b7590, simc::matrixFromQuat>(app, ts, frame + kLocalQuaternion, camera + kCameraMatrix);
        guest::direct<0x4b79c0, simc::matrixQuarterTurn>(app, ts, camera + kCameraMatrix);
    }
    const u32 again = guest.dword(kSpots) + spot * kSpotBytes;
    if (guest.byte(again + kSpotFlags) & kFollows)
        return guest::direct<0x424fd0, simd::cameraMoveTowards>(app, ts, camera, camera + kCameraX, kFollowBits);
    const s32 angle = (s32(guest.dword(again + kSpotAngle)) >> 16) * 512 / 360;
    fe::direct<0x4eac60, tail::angleParts>(app, ts, u32(angle), frame + kLocalPartA, frame + kLocalPartB);
    const Float ratio = fp::div(integerFrom(guest, frame + kLocalPartB), integerFrom(guest, frame + kLocalPartA));
    const u32 zoom = jumpsBelow(fp::compare(ratio, floatFrom(guest, kLeastZoom))) ? kLeastZoomBits : bitsOf(float(ratio));
    guest.setDword(camera + kCameraZoom, zoom);
    return zoom;
}

namespace
{

/* As an animated path ends: the mode the player chose if it is in the cycle, else the
 * cycle's first. */
u32 switchToChosenMode(Guest& guest, u32 camera)
{
    const bool inCycle = fe::direct<kModeInCycle, carstate::modeInTheCycle>(guest.app(), guest.ts(), gameSettings(guest).cameraMode[guest.dword(camera + kCameraView)]);
    const u32 view = guest.dword(camera + kCameraView);
    const u32 mode = inCycle ? gameSettings(guest).cameraMode[view] : guest.dword(kFirstCycleMode);
    return *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), mode, view);
}

}  // namespace

/* sub_4253d0: mode 16, an animated path, its pose laid over the car's body when its
 * kind (4, 5) rides on the car, its target no farther than the car's reach. */
nfs::u32 frontend::sima::cameraPlayPath(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x84);
    constexpr u32 kStarted = kCameraOwn110, kPath = kCameraOwn114, kAnimation = kCameraOwn118;
    constexpr u32 kNone = u32(-1);
    constexpr u32 kCarBody = 0x124, kCarReach = 0x120;
    constexpr u32 kLocalMatrix = 0x00;       // 9 floats: the orientation
    constexpr u32 kLocalStepMatrix = 0x24;   // 9 floats
    constexpr u32 kLocalTarget = 0x48;       // 3 floats
    constexpr u32 kLocalStepPlace = 0x54;    // 3 floats
    constexpr u32 kLocalCarPlace = 0x60;     // 3 floats
    constexpr u32 kLocalDirection = 0x6c;    // 3 floats

    const u32 frame = guest.frame();
    if (guest.dword(camera + kStarted) == 0)
    {
        const u32 animation = fe::direct<0x40f3b0, aiplan::playAnimation>(app, ts, guest.dword(camera + kPath));
        guest.setDword(camera + kAnimation, animation);
        if (animation == kNone)
            return switchToChosenMode(guest, camera);
        guest.setDword(camera + kStarted, 1);
    }
    if (guest.dword(camera + kAnimation) == kNone)
        guest.setDword(camera + kAnimation, fe::direct<0x40f3b0, aiplan::playAnimation>(app, ts, guest.dword(camera + kPath)));
    if (guest.dword(camera + kAnimation) == kNone)
        return kNone;  // what that start left in eax: it found no slot

    fe::direct<0x40f940, aiplan::stepAnimation>(app, ts, guest.dword(camera + kAnimation), frame + kLocalStepPlace,
                                                 frame + kLocalStepMatrix);
    if (!fe::direct<kAnimationPlaying, aiplan::animationOn>(guest.app(), guest.ts(), guest.dword(camera + kAnimation)))
    {
        fe::direct<0x40f460, aiplan::endAnimation>(app, ts, guest.dword(camera + kAnimation));
        guest.setDword(camera + kAnimation, kNone);
        return switchToChosenMode(guest, camera);
    }
    const u32 path = guest.dword(camera + kPath);
    if (path == 5 || path == 4)
    {
        const u32 body = guest.dword(camera + kCameraCar) + kCarBody;
        fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, frame + kLocalStepMatrix, body, frame + kLocalMatrix);
        // The step's place by the body: each row's three products, then their sums, the first row first.
        const u32 place = frame + kLocalStepPlace;
        Float sum[3];
        for (u32 axis = 0; axis < 3; ++axis)
            sum[axis] = fp::mul(floatFrom(guest, place), floatFrom(guest, body + 4 * axis));
        for (u32 row = 1; row < 3; ++row)
            for (u32 axis = 0; axis < 3; ++axis)
                sum[axis] = fp::add(sum[axis], fp::mul(floatFrom(guest, place + 4 * row), floatFrom(guest, body + 12 * row + 4 * axis)));
        for (u32 axis = 0; axis < 3; ++axis)
            guest.setFloat(frame + kLocalTarget + 4 * axis, float(sum[axis]));
        copyDwords(guest, frame + kLocalDirection, frame + kLocalTarget, 3);
        const Float length = Float(fe::direct<0x4972f0, vertices::normaliseVector>(app, ts, frame + kLocalDirection));
        const float reach = float(fp::add(1.0, floatFrom(guest, guest.dword(camera + kCameraCar) + kCarReach)));
        if (jumpsBelow(fp::compare(length, Float(reach))))
        {
            fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, frame + kLocalDirection, reach, frame + kLocalDirection);
            copyDwords(guest, frame + kLocalTarget, frame + kLocalDirection, 3);
        }
    }
    else
    {
        copyDwords(guest, frame + kLocalMatrix, frame + kLocalStepMatrix, 9);
        copyDwords(guest, frame + kLocalTarget, frame + kLocalStepPlace, 3);
    }
    const u32 car = guest.dword(camera + kCameraCar);
    guest.setDword(frame + kLocalCarPlace, guest.dword(car + kCarX));
    guest.setDword(frame + kLocalCarPlace + 4, guest.dword(car + kCarY));
    guest.setDword(frame + kLocalCarPlace + 8, guest.dword(car + kCarZ));
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, frame + kLocalCarPlace, frame + kLocalTarget,
                                                           camera + kCameraX);
    copyDwords(guest, camera + kCameraTarget, frame + kLocalTarget, 3);
    copyDwords(guest, camera + kCameraMatrix, frame + kLocalMatrix, 9);
    return camera;
}

namespace
{

/* A path's end pose, handed back in 16.16 to `placeInts` and `matrixInts`, as floats
 * to where the path's steps put theirs. */
void pathEndPoseAsFloats(Guest& guest, u32 path, u32 placeInts, u32 matrixInts, u32 place, u32 matrix)
{
    constexpr u32 kScale = 0x536f1c;
    fe::direct<kPathEndPose, aiplan::endPose>(guest.app(), guest.ts(), path, 1, placeInts, matrixInts);
    const Float scale = doubleAt(guest, kScale);
    for (u32 i = 0; i < 3; ++i)
        guest.setFloat(place + 4 * i, float(fp::mul(integerFrom(guest, placeInts + 4 * i), scale)));
    for (u32 i = 0; i < 9; ++i)
        guest.setFloat(matrix + 4 * i, float(fp::mul(integerFrom(guest, matrixInts + 4 * i), scale)));
}

}  // namespace

/* sub_425640: mode 17, the path at a race's end, its zoom fixed; when it is over, with
 * more than two players, the view handed to the first car still racing, the HUD with
 * it, else it stays on the path's end. */
nfs::u32 frontend::sima::cameraEndPath(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0x94);
    constexpr u32 kPath = kCameraOwn114, kAnimation = kCameraOwn118;
    constexpr u32 kNone = u32(-1);
    constexpr u32 kEndZoomBits = 0x3f99999a;
    constexpr u32 kCarFinished = 0x2c4;
    constexpr std::uint8_t kCarDone = 0x80, kCarRacing = 0x01;
    constexpr u32 kPaneCopied = 0x7d3694, kPaneBytes = 0x1a4;
    constexpr u32 kOneBits = 0x3f800000;
    constexpr u32 kElementMap = 0x11;
    constexpr u32 kLocalMatrixInts = 0x00;    // 9 ints, a pose asked of the path now
    constexpr u32 kLocalMatrix = 0x24;        // 9 floats
    constexpr u32 kLocalEndMatrixInts = 0x48; // 9 ints, the pose as the path ends
    constexpr u32 kLocalEndPlaceInts = 0x6c;  // 3 ints
    constexpr u32 kLocalPlaceInts = 0x78;     // 3 ints
    constexpr u32 kLocalPlace = 0x84;         // 3 floats

    const u32 frame = guest.frame();
    guest.setDword(camera + kCameraZoom, kEndZoomBits);
    if (guest.dword(camera + kAnimation) == kNone)
    {
        pathEndPoseAsFloats(guest, guest.dword(camera + kPath), frame + kLocalPlaceInts, frame + kLocalMatrixInts,
                            frame + kLocalPlace, frame + kLocalMatrix);
    }
    else
    {
        fe::direct<0x40f940, aiplan::stepAnimation>(app, ts, guest.dword(camera + kAnimation), frame + kLocalPlace,
                                                     frame + kLocalMatrix);
        if (!fe::direct<kAnimationPlaying, aiplan::animationOn>(guest.app(), guest.ts(), guest.dword(camera + kAnimation)))
        {
            fe::direct<0x40f460, aiplan::endAnimation>(app, ts, guest.dword(camera + kAnimation));
            guest.setDword(camera + kAnimation, kNone);
            if (s32(gameSettings(guest).playMode) > 1)
            {
                s32 place = 0;
                for (; place < s32(gameSettings(guest).humanCount); ++place)
                {
                    const u32 car = guest.dword(kCarsByPlace + u32(place) * 4);
                    if (guest.dword(car + kCarFinished) != 0)
                        continue;
                    const std::uint8_t flags = guest.byte(car + kCarFlags);
                    if ((flags & kCarDone) || !(flags & kCarRacing))
                        continue;
                    gameSettings(guest).viewedPlayer = u32(place);
                    fe::direct<kAttachCameras, carstate::camerasAttached>(guest.app(), guest.ts(), 0, u32(place));
                    fe::direct<0x485640, hudx::rebuildView>(app, ts, 0);
                    if (guest.dword(kPaneCopied) == 0)
                    {
                        // By address: it leaves edx and ebx, which the calls after it hand on.
                        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, SETTINGS_ADDRESS(hud[6]), SETTINGS_ADDRESS(hud), kPaneBytes);
                        guest.setDword(kPaneCopied, 1);
                    }
                    gameSettings(guest).hud[0].rects[17].leftBits = 0;
                    gameSettings(guest).hud[0].rects[17].topBits = 0;
                    gameSettings(guest).hud[0].rects[17].rightBits = kOneBits;
                    gameSettings(guest).hud[0].rects[17].bottomBits = kOneBits;
                    gameSettings(guest).hud[0].mapStyle = 2;
                    gameSettings(guest).hud[0].mapScaleBits = kOneBits;
                    fe::direct<0x4853c0, frontend::hudr::rebuildElement>(app, ts, 0u, kElementMap);
                    // (0x4844c0 pushes edx, ebx and ecx and sets them before it reads them: only eax is its.)
                    return u32(*fe::direct<0x4844c0, port::hud::hudPaneSetOf>(
                        app, ts, ts.sp - 4, port::Args{ 0, 0, 0, 0 }));
                }
                return u32(place);
            }
            pathEndPoseAsFloats(guest, guest.dword(camera + kPath), frame + kLocalEndPlaceInts, frame + kLocalEndMatrixInts,
                                frame + kLocalPlace, frame + kLocalMatrix);
        }
    }
    const u32 result = fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(app, ts, 1, guest.dword(camera + kCameraCar) + kCarX, frame + kLocalPlace, camera + kCameraX);
    copyDwords(guest, camera + kCameraMatrix, frame + kLocalMatrix, 9);
    return result;
}

/* sub_425d50: mode 9, the flyover: along the track's slices at a gear chosen from the
 * car's pace by a table of eight, jumping a few nodes on when the car has drawn ahead of
 * it or back when fallen behind; a little lower the farther from the car.  The tables
 * are copied into its frame and read there, unchecked: a pace past the last gear reads
 * on into its locals, so those it has written are where they were. */
nfs::u32 frontend::sima::cameraFlyover(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0xb0);
    constexpr u32 kPaceOfGear = 0x423d50, kStepsOfGear = 0x423d70, kGears = 8;
    constexpr u32 kCounting = 0x7d3680;
    constexpr u32 kStep = 0x553004, kNode = 0x55300c, kGear = 0x553014;
    constexpr u32 kCarPace = 0x7c0;
    constexpr u32 kPaceClose = 0x536f38, kPaceCloseBack = 0x536f3c;
    constexpr u32 kJumpNear = 0x536f44, kJumpMiddle = 0x536f48, kJumpBackNear = 0x536f4c;
    constexpr u32 kScale = 0x536f54, kQuarter = 0x536f5c, kMostDrop = 0x536f60, kHeight = 0x536f38;
    constexpr u32 kMostDropBits = 0x40600000;
    constexpr u32 kLocalPaces = 0x00, kLocalSteps = 0x20;
    constexpr u32 kLocalPace = 0x88, kLocalOffGear = 0xa0;
    constexpr s32 kCarAhead = 1, kCarBehind = 2;
    constexpr u32 kRoadKept = 9;

    const u32 frame = guest.frame();
    copyDwords(guest, frame + kLocalPaces, kPaceOfGear, kGears);
    copyDwords(guest, frame + kLocalSteps, kStepsOfGear, kGears);
    const u32 car = guest.dword(camera + kCameraCar);
    if (cameraHeld(guest))
        return camera;
    const auto view = [&] { return guest.dword(camera + kCameraView); };
    const auto gearPace = [&] { return floatFrom(guest, frame + kLocalPaces + guest.dword(kGear + view() * 4) * 4); };

    if (guest.dword(kCounting) != 0)
        guest.setDword(kStep + view() * 4, guest.dword(kStep + view() * 4) + 1);

    s32 off = 0;
    if (guest.dword(kStep + view() * 4) == guest.dword(frame + kLocalSteps + guest.dword(kGear + view() * 4) * 4))
    {
        guest.setDword(kStep + view() * 4, 0);
        guest.setDword(kNode + view() * 4,
                       u32(wrapped(s32(guest.dword(kNode + view() * 4) + 1), s32(guest.dword(kSliceCount)))));
        const u32 paceBits = guest.dword(car + kCarPace);
        guest.setDword(frame + kLocalPace, paceBits);
        const Float pace = Float(floatOf(paceBits));
        const s32 gap = s32(guest.dword(car + kCarNode) - guest.dword(kNode + view() * 4));
        const float offGear = float(fp::sub(pace, gearPace()));
        guest.setFloat(frame + kLocalOffGear, offGear);
        if (gap > 7
            || (gap > 4 && jumpsBelow(fp::compare(Float(offGear), floatFrom(guest, kPaceClose)))
                && jumpsBelow(fp::compare(0.0, Float(offGear)))))
        {
            off = kCarAhead;
        }
        else if (gap < -6
                 || (gap < -4 && jumpsAbove(fp::compare(Float(offGear), doubleAt(guest, kPaceCloseBack)))
                     && jumpsAbove(fp::compare(0.0, Float(offGear)))))
        {
            off = kCarBehind;
        }
        if (off != 0)
        {
            u32 gear = 0;
            while (jumpsAbove(fp::compare(pace, floatFrom(guest, frame + kLocalPaces + gear * 4))))
                ++gear;
            guest.setDword(kGear + view() * 4, gear);
            const float paceOff = float(fp::sub(pace, gearPace()));
            if (guest::direct<0x4bdc70, simc::tvShotOver>(app, ts, view()) != 0)
            {
                s32 node;
                if (off == kCarAhead)
                {
                    const s32 jump = jumpsBelow(fp::compare(Float(paceOff), floatFrom(guest, kJumpNear))) ? 3
                                      : jumpsBelow(fp::compare(Float(paceOff), floatFrom(guest, kJumpMiddle))) ? 5 : 7;
                    node = wrapped(s32(guest.dword(car + kCarNode)) + jump, s32(guest.dword(kSliceCount)));
                }
                else
                {
                    const s32 jump = jumpsAbove(fp::compare(Float(paceOff), doubleAt(guest, kJumpBackNear))) ? 3 : 5;
                    node = s32(guest.dword(car + kCarNode)) - jump;
                    if (node < 0)
                        node += s32(guest.dword(kSliceCount));
                }
                guest.setDword(kNode + view() * 4, u32(node));
                guest::direct<0x4bdcc0, simc::tvStartShot>(app, ts, view(), kRoadKept);
            }
        }
    }
    if (guest::direct<0x4bdc70, simc::tvShotOver>(app, ts, view()) != 0
        && (off != 0 || roadBroken(guest, guest.dword(camera + kCameraNode), guest.dword(car + kCarNode))))
    {
        return guest::direct<0x4bdbd0, simc::tvNextCamera>(app, ts, view());
    }

    // Between this node's slice and the next's, by the steps gone.
    const u32 nodeNow = guest.dword(kNode + view() * 4);
    const u32 nodeNext = u32(wrapped(s32(nodeNow + 1), s32(guest.dword(kSliceCount))));
    const u32 from = slice(guest, nodeNow);
    const u32 to = slice(guest, nodeNext);
    const s32 steps = s32(guest.dword(frame + kLocalSteps + guest.dword(kGear + view() * 4) * 4));
    const Float scale = doubleAt(guest, kScale);
    const Float stepsGone = integerFrom(guest, kStep + view() * 4);
    const auto along = [&](u32 axis, bool scaleFirst) {
        const s32 each = s32(guest.dword(to + axis) - guest.dword(from + axis)) / steps;
        const Float start = scaleFirst ? fp::mul(scale, integerFrom(guest, from + axis)) : fp::mul(integerFrom(guest, from + axis), scale);
        const float eachOn = float(fp::mul(Float(each), scale));
        return float(fp::add(start, fp::mul(stepsGone, Float(eachOn))));
    };
    guest.setFloat(camera + kCameraX, along(0, false));
    guest.setFloat(camera + kCameraY, along(4, false));
    guest.setFloat(camera + kCameraZ, along(8, true));

    // How far from the car, the longer way and a quarter of the shorter.
    const u32 carNow = guest.dword(camera + kCameraCar);
    const auto offBy = [&](u32 carField, u32 cameraField) {
        return float(fp::sub(floatFrom(guest, carNow + carField), floatFrom(guest, camera + cameraField)));
    };
    u32 eax = carNow;
    // A way's size; its off stored, eax as the original leaves it after: that of the size's copy, or the comparison's.
    const auto sizeOf = [&](float offValue) {
        u16 own;
        const float size = absolute(offValue, own);
        eax = jumpsAbove(own) ? withStatus(carNow, own) : bitsOf(offValue);
        return size;
    };
    const float awayX = sizeOf(offBy(kCarX, kCameraX));
    const float awayZ = sizeOf(offBy(kCarZ, kCameraZ));
    Float sum;
    if (jumpsAbove(fp::compare(Float(awayX), Float(awayZ))))
    {
        const float quarter = float(fp::mul(Float(sizeOf(offBy(kCarZ, kCameraZ))), floatFrom(guest, kQuarter)));
        sum = fp::add(Float(sizeOf(offBy(kCarX, kCameraX))), Float(quarter));
    }
    else
    {
        const float quarter = float(fp::mul(Float(sizeOf(offBy(kCarX, kCameraX))), floatFrom(guest, kQuarter)));
        sum = fp::add(Float(sizeOf(offBy(kCarZ, kCameraZ))), Float(quarter));
    }
    const float distance = float(sum);
    const float drop = float(fp::mul(Float(distance), floatFrom(guest, kQuarter)));
    const u16 capped = fp::compare(floatFrom(guest, kMostDrop), Float(drop));
    const bool overMost = jumpsBelow(capped);
    const u32 dropBits = overMost ? kMostDropBits : bitsOf(drop);
    eax = overMost ? withStatus(eax, capped) : dropBits;
    const Float height = fp::sub(floatFrom(guest, kHeight), Float(floatOf(dropBits)));
    guest.setFloat(camera + kCameraY, float(fp::add(height, floatFrom(guest, camera + kCameraY))));
    return eax;
}

/* sub_4262a0: mode 19, runs the mode it was told for a while, then hands the view on. */
nfs::u32 frontend::sima::cameraWaitThenHandOn(App* app, ThreadState& ts, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    constexpr u32 kWaiting = kCameraOwn118, kNextMode = kCameraOwn11c;
    constexpr u32 kTime = kCameraOwn110, kTick = kCameraOwn114;
    constexpr s32 kOneBits = 0x3f800000;  // compared as an integer
    if (guest.dword(camera + kWaiting) == 0)
        return camera;
    const u32 next = guest.dword(camera + kNextMode);
    if (next == 0)
        return camera;
    if (s32(guest.dword(camera + kTime)) > kOneBits)
    {
        fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), guest.dword(camera + kCameraView), next);
        return guest::direct<0x426450, cameraStep>(app, ts, guest.dword(camera + kNextMode), guest.dword(camera + kCameraView));
    }
    const u32 result = guest::direct<0x426450, cameraStep>(app, ts, next, guest.dword(camera + kCameraView));
    guest.setFloat(camera + kTime, float(fp::add(floatFrom(guest, camera + kTick), floatFrom(guest, camera + kTime))));
    return result;
}

/* sub_426310: a view under mode 19 handed straight on to the mode it was waiting for. */
nfs::u32 frontend::sima::cameraBackFromWait(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20);
    constexpr u32 kWaitCameras = 0x552f60, kWaitMode = 0x13;
    const u32 now = guest.dword(kCameraNow + view * 4);
    if (now == 0)
        return view;
    const u32 mode = guest.dword(now);
    if (mode != kWaitMode)
        return mode;
    const u32 waiting = guest.dword(kWaitCameras + view * 4);
    fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), view, guest.dword(waiting + kCameraOwn11c));
    const u32 camera = guest.dword(kCameraNow + view * 4);
    return guest::direct<0x426450, cameraStep>(app, ts, camera != 0 ? guest.dword(camera) : 0, view);
}

/* sub_426450: a mode's camera of a view prepared (sub_426370) and stepped.  It leaves
 * edx the mode less one, as a jump table's index, for no caller's use. */
nfs::u32 frontend::sima::cameraStep(App* app, ThreadState& ts, u32 mode, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    static constexpr u32 kStepOfMode[24] = {
        0x4240c0, 0x424260, 0x424740, 0x4243a0, 0x424740, 0x4243a0, 0x424b50, 0x425060,
        0x425d50, 0x425990, 0x425ab0, 0x425ab0, 0x425ab0, 0x425bf0, 0,        0x4253d0,
        0x425640, 0x4251f0, 0x4262a0, 0x426290, 0x424690, 0,        0x424ce0, 0x424e30,
    };
    const u32 camera = guest.dword(kCamerasOfModes + mode * 8 + view * 4);
    const u32 prepared = guest::direct<0x426370, simd::cameraPrepare>(app, ts, camera, view, mode);
    const u32 row = mode - 1;
    if (row > 0x17 || kStepOfMode[row] == 0)
        return prepared;
    // Through the jump table: by address.
    return fx::callAt(guest, kStepOfMode[row], args({ camera }));
}

/* sub_426560: the mode a view's car forces now: the attract's, a wreck's, a finish's,
 * one of being stopped or of having left the track; forcing none ends mode 0x18's. */
nfs::u32 frontend::sima::forcedMode(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    constexpr u32 kAttract = 0x7d36a0, kAttractOff = 0x559264;
    constexpr u32 kCarWrecked = 0x9a4, kCarFinished = 0x99c, kCarStopped = 0x184, kCarLeftTrack = 0x524;
    constexpr u32 kWreckCameras = 0x552f70;
    constexpr u32 kModeAttract = 0x17, kModeWreck = 0x15, kModeFinished = 7, kModeStopped = 0x14, kModeOffTrack = 0x18;

    const u32 forcedAt = kCameraForced + view * 4;
    const u32 nowAt = kCameraNow + view * 4;
    const u32 forced = guest.dword(forcedAt);
    const u32 car = guest.dword((forced != 0 ? forced : guest.dword(nowAt)) + kCameraCar);
    const u32 forcedBefore = forced != 0 ? guest.dword(forced) : 0;
    u32 mode = 0;
    if (guest.dword(kAttract) != 0 && guest.dword(kAttractOff) == 0)
    {
        mode = kModeAttract;
        guest::direct<0x426310, cameraBackFromWait>(app, ts, view);
    }
    else if (guest.dword(car + kCarWrecked) != 0)
    {
        mode = kModeWreck;
        guest.setDword(guest.dword(kWreckCameras + view * 4) + kCameraCar,
                       guest.dword((forced != 0 ? forced : guest.dword(nowAt)) + kCameraCar));
    }
    else if (guest.dword(car + kCarFinished) != 0)
    {
        mode = kModeFinished;
    }
    else if (s32(guest.dword(car + kCarStopped)) > 0)
    {
        mode = kModeStopped;
    }
    else if (guest.dword(car + kCarLeftTrack) != 0 && gameSettings(guest).viewedPlayer == gameSettings(guest).localPlayer)
    {
        mode = kModeOffTrack;
        guest::direct<0x426310, cameraBackFromWait>(app, ts, view);
    }
    const u32 result = *fe::direct<kSetForcedMode, carstate::overrideModePut>(guest.app(), guest.ts(), view, mode);
    if (forcedBefore != kModeOffTrack || mode != 0)
        return result;
    const u32 camera = guest.dword(nowAt);
    const u32 back = camera != 0 ? guest.dword(camera) : 0;
    fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), view, kModeOffTrack);
    return *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), back, view);
}

/* sub_426680: every view's camera stepped; a car at the end of its race first given
 * mode 17, the end path, unless a path of its own is going. */
nfs::u32 frontend::sima::camerasStep(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    constexpr u32 kPathMode = 0x10, kEndMode = 0x11;
    constexpr u32 kCarRaceState = 0x2c4, kRaceOver = 3;
    constexpr std::uint8_t kCarRacing = 0x01;
    constexpr u32 kWonPath = 6, kLostPath = 1;
    constexpr u32 kNone = u32(-1);

    const s32 lastView = gameSettings(guest).playMode == 1 ? 1 : 0;
    u32 result = 0;
    for (s32 index = 0; index <= lastView; ++index)
    {
        const u32 view = u32(index);
        const u32 camera = guest.dword(kCameraNow + view * 4);
        const u32 car = guest.dword(camera + kCameraCar);
        const u32 mode = guest.dword(camera);
        const u32 onPath = mode == kPathMode || mode == kEndMode ? guest.dword(camera + kCameraOwn11c) : 0;
        if ((guest.byte(car + kCarFlags) & kCarRacing) && guest.dword(car + kCarRaceState) == kRaceOver && onPath == 0)
        {
            fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), kEndMode, view);
            const u32 ending = guest.dword(kCameraNow + view * 4);
            guest.setDword(ending + kCameraOwn11c, 1);
            const u32 animation = guest.dword(ending + kCameraOwn118);
            if (animation != kNone)
            {
                fe::direct<0x40f460, aiplan::endAnimation>(app, ts, animation);
                guest.setDword(ending + kCameraOwn118, kNone);
            }
            const u32 path = gameSettings(guest).backwards != 0 ? kWonPath : kLostPath;
            guest.setDword(ending + kCameraOwn114, path);
            guest.setDword(ending + kCameraOwn118, fe::direct<0x40f3b0, aiplan::playAnimation>(app, ts, path));
        }
        guest::direct<0x426560, forcedMode>(app, ts, view);
        const u32 forced = guest.dword(kCameraForced + view * 4);
        const u32 shown = forced != 0 ? forced : guest.dword(kCameraNow + view * 4);
        result = guest::direct<0x426450, cameraStep>(app, ts, shown != 0 ? guest.dword(shown) : 0, view);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Distances
// ---------------------------------------------------------------------------

/* sub_4965b0: the distance between two points, eax and edx, without a root: the
 * longest way and a quarter of each other's, onto the x87. */
frontend::sima::FloatLeft frontend::sima::quickDistanceOf(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    constexpr u32 kQuarter = 0x53bedc;
    u16 status;
    const float dx = absolute(float(fp::sub(floatFrom(guest, a), floatFrom(guest, b))), status);
    const float dy = absolute(float(fp::sub(floatFrom(guest, a + 4), floatFrom(guest, b + 4))), status);
    const float dz = absolute(float(fp::sub(floatFrom(guest, a + 8), floatFrom(guest, b + 8))), status);
    const Float quarter = floatFrom(guest, kQuarter);
    const float flat = float(jumpsAbove(fp::compare(Float(dx), Float(dy)))
                                 ? fp::add(fp::mul(Float(dy), quarter), Float(dx))
                                 : fp::add(fp::mul(Float(dx), quarter), Float(dy)));
    status = fp::compare(Float(flat), Float(dz));
    const float whole = float(jumpsAbove(status) ? fp::add(fp::mul(Float(dz), quarter), Float(flat))
                                                 : fp::add(fp::mul(Float(flat), quarter), Float(dz)));
    return { double(whole), withStatus(bitsOf(dz), status) };
}

/* sub_4967e0: a length in the plane without a root, of two sizes on the stack: the
 * larger and a share of the smaller, the share by how near the smaller comes. */
double frontend::sima::planarLengthOf(App* app, ThreadState& ts, u32 aBits, u32 bBits)
{
    Guest guest(app, ts);
    const float a = floatOf(aBits), b = floatOf(bBits);
    constexpr u32 kQuarter = 0x53bee4, kHalf = 0x53bef4, kThreeQuarters = 0x53bf04;
    constexpr u32 kShareSmall = 0x53beec, kShareLess = 0x53befc, kShareMore = 0x53bf0c, kShareLarge = 0x53bf14;
    const bool firstLarger = jumpsAbove(fp::compare(Float(a), Float(b)));
    const Float larger = Float(firstLarger ? a : b);
    const Float smaller = Float(firstLarger ? b : a);
    u32 share = kShareLarge;
    if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kQuarter)), smaller)))
        share = kShareSmall;
    else if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kHalf)), smaller)))
        share = kShareLess;
    else if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kThreeQuarters)), smaller)))
        share = kShareMore;
    return double(float(fp::add(fp::mul(smaller, doubleAt(guest, share)), larger)));
}

// ---------------------------------------------------------------------------
// The cars on the track
// ---------------------------------------------------------------------------

/* sub_49fde0: whether a car leans over too far: with the race's hazard (0x6fd4a0) and a
 * car that has it (flag 4), past -0.9 or past 0.5 as integers read the float. */
nfs::u32 frontend::sima::carTiltedOver(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    constexpr u32 kLean = 0x10c;
    constexpr std::uint8_t kHazardCar = 0x04;
    constexpr u32 kMostBack = 0x53c794, kMost = 0x53c79c;
    constexpr s32 kHalfBits = 0x3f000000;
    if (gameSettings(guest).raceOptionHazard != 0 && (guest.byte(car + kCarFlags) & kHazardCar))
    {
        if (jumpsBelow(fp::compare(floatFrom(guest, car + kLean), doubleAt(guest, kMostBack))))
            return 1;
        return s32(guest.dword(car + kLean)) > kHalfBits ? 1 : 0;
    }
    u16 status;
    const float size = absolute(guest.floatAt(car + kLean), status);
    return jumpsAbove(fp::compare(Float(size), floatFrom(guest, kMost))) ? 1 : 0;
}

/* sub_49fee0: the track's heading at a slice, as a fixed angle: atan2 of the way to the
 * next slice, cut to an integer. */
nfs::u32 frontend::sima::sliceHeading(App* app, ThreadState& ts, u32 index)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 8);
    constexpr u32 kScale = 0x53c7ac, kToFixed = 0x53c7b4;
    const u32 next = u32(wrapped(s32(index + 1), s32(guest.dword(kSliceCount))));
    const u32 from = slice(guest, index);
    const u32 to = slice(guest, next);
    const Float scale = doubleAt(guest, kScale);
    const Float x = fp::mul(Float(s32(guest.dword(to) - guest.dword(from))), scale);
    const Float z = fp::mul(scale, Float(s32(guest.dword(to + 8) - guest.dword(from + 8))));
    const double angle = guest::direct<0x4ea9c0, simd::angleOfFloatsValue>(app, ts, bitsOf(float(x)), bitsOf(float(z)));
    const double whole =
        guest::direct<kTruncate, watcom::clib::chop>(app, ts, double(fp::mul(Float(angle), doubleAt(guest, kToFixed))));
    return u32(fp::toInteger<s32>(Float(whole)));
}

/* sub_49ffa0: a car in the race given the track's frame where it is (sub_420230, in
 * 16.16, in the stack frame) as floats, and its heading. */
nfs::u32 frontend::sima::carTrackFrame(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16 + 0x34);
    constexpr u32 kCarAxes = 0x124, kCarCentre = 0x148, kCarHeading = 0x154;
    constexpr u32 kScale = 0x53c7bc;
    constexpr u32 kLocalAxes = 0x00, kLocalCentre = 0x24;
    if (guest.byte(car + kCarInRace) == 0)
        return car;
    const u32 frame = guest.frame();
    fe::direct<0x420230, trackdata::takeFrame>(app, ts, car + kCarPlaceOnTrack, frame + kLocalAxes, frame + kLocalCentre);
    const Float scale = doubleAt(guest, kScale);
    for (u32 i = 0; i < 3; ++i)
        guest.setFloat(car + kCarCentre + 4 * i, float(fp::mul(integerFrom(guest, frame + kLocalCentre + 4 * i), scale)));
    for (u32 i = 0; i < 9; ++i)
    {
        const Float value = integerFrom(guest, frame + kLocalAxes + 4 * i);
        guest.setFloat(car + kCarAxes + 4 * i, float(i == 8 ? fp::mul(scale, value) : fp::mul(value, scale)));
    }
    const u32 heading = guest::direct<0x49fee0, sliceHeading>(app, ts, guest.dword(car + kCarNode));
    guest.setDword(car + kCarHeading, heading);
    return heading;
}

/* sub_4a0140: a car moved back onto the road: its eight corners (sub_4e0430) held
 * against the road's plane at its slice and its two limits, each limit only where the
 * slice holds a car to it, by the corner past one the most. */
nfs::u32 frontend::sima::carOntoRoad(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24 + 0xa4);
    constexpr u32 kScale = 0x53c7c4;
    constexpr u32 kSliceNormal = 0x15, kSliceAbove = 0x20, kSliceBelow = 0x1c;
    constexpr u32 kLocalCorners = 0x00;  // 8 of 3 floats
    constexpr u32 kLocalArm = 0x60;      // 3 floats
    constexpr u32 kLocalNormal = 0x6c;   // 3 floats

    const u32 frame = guest.frame();
    const u32 corners = frame + kLocalCorners;
    const auto minus = [&](u32 field) { return bitsOf(float(-floatFrom(guest, car + field))); };
    const auto plus = [&](u32 field) { return guest.dword(car + field); };
    const u32 corner[8][3] = {
        { minus(kCarHalfWidth), plus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), plus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), plus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), plus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), minus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), minus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), minus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), minus(kCarHalfHeight), minus(kCarHalfLength) },
    };
    for (u32 i = 0; i < 8; ++i)
        for (u32 axis = 0; axis < 3; ++axis)
            guest.setDword(corners + (i * 3 + axis) * 4, corner[i][axis]);
    const u32 node = guest.dword(car + kCarNode);
    fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 8, corners, car + kCarMatrix, car + kCarX, corners);

    const u32 road = slice(guest, node);
    const Float scale = doubleAt(guest, kScale);
    const u32 normal = frame + kLocalNormal;
    for (u32 i = 0; i < 3; ++i)
    {
        // A signed byte of the slice, in 7.9 of 16.16.
        const s32 part = (s32(guest.dword(road + kSliceNormal + i)) >> 24) * 512;
        guest.setFloat(normal + 4 * i, float(fp::mul(Float(part), scale)));
    }
    const float centre[3] = { float(fp::mul(integerFrom(guest, road), scale)),
                              float(fp::mul(scale, integerFrom(guest, road + 4))),
                              float(fp::mul(integerFrom(guest, road + 8), scale)) };
    const float above = float(fp::mul(integerFrom(guest, road + kSliceAbove), scale));
    const float below = float(-fp::mul(scale, integerFrom(guest, road + kSliceBelow)));

    float most = 0.0f;
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 at = corners + i * 12;
        for (u32 axis = 0; axis < 3; ++axis)
            guest.setFloat(frame + kLocalArm + 4 * axis, float(fp::sub(floatFrom(guest, at + 4 * axis), Float(centre[axis]))));
        const Float height = Float(fe::direct<0x4e01f0, vertices::dotProduct>(app, ts, normal, frame + kLocalArm));
        const float kept = float(height);
        const std::uint8_t limits = guest.byte(slice(guest, node) + kSliceLimits);
        float past = 0.0f;
        if (jumpsAbove(fp::compare(height, Float(above))) && (limits & 0x0f))
            past = float(fp::sub(Float(above), Float(kept)));
        else if (jumpsBelow(fp::compare(Float(kept), Float(below))) && ((limits >> 4) & 0x0f))
            past = float(fp::sub(Float(below), Float(kept)));
        u16 status;
        const float pastSize = absolute(past, status);
        const float mostSize = absolute(most, status);
        if (jumpsAbove(fp::compare(Float(pastSize), Float(mostSize))))
            most = past;
    }
    const u32 result = fe::direct<0x4e0050, vertices::scaleVectorsEax>(app, ts, 1, normal, most, normal);
    guest.setFloat(car + kCarX, float(fp::add(floatFrom(guest, normal), floatFrom(guest, car + kCarX))));
    guest.setFloat(car + kCarY, float(fp::add(floatFrom(guest, normal + 4), floatFrom(guest, car + kCarY))));
    guest.setFloat(car + kCarZ, float(fp::add(floatFrom(guest, normal + 8), floatFrom(guest, car + kCarZ))));
    return result;
}

namespace
{

/* A way from a car to a player, as sub_4a0580 sizes it: above 0 as it is, else its sign
 * changed -- so here 0 changes too, and a NaN does not. */
float sizeOfWay(Guest& guest, u32 car, u32 player, u32 field)
{
    const float off = float(fp::sub(floatFrom(guest, car + field), floatFrom(guest, player + field)));
    return jumpsBelow(fp::compare(0.0, Float(off))) ? off : float(-Float(off));
}

/* The longer of two sizes and a quarter of the shorter. */
float planeDistance(Guest& guest, float x, float z)
{
    constexpr u32 kQuarter = 0x53c7cc;
    return float(jumpsAbove(fp::compare(Float(x), Float(z))) ? fp::add(fp::mul(Float(z), floatFrom(guest, kQuarter)), Float(x))
                                                             : fp::add(fp::mul(Float(x), floatFrom(guest, kQuarter)), Float(z)));
}

}  // namespace

/* sub_4a0580: how far a car is from the players of a one-machine race of at most two:
 * the larger of its distance and the nodes to go, six a node.  Far (366), it stays
 * asleep and stopped; near again, it wakes on the road. */
nfs::u32 frontend::sima::carNearPlayers(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20 + 0x50);
    constexpr u32 kNodeScale = 0x53c7d4, kFar = 0x53c7dc;
    constexpr u32 kLocalMost = 2;
    constexpr u32 kCarSpinA = 0xe4, kCarSpinB = 0xec;
    constexpr u32 kCarRest = 0x158, kCarUnder = 0x15c, kCarTiltA = 0x168, kCarTiltB = 0x16c;

    const u32 players = gameSettings(guest).playMode;
    if (players != 0)
    {
        guest.setByte(car + kCarAsleep, 0);
        guest.setDword(car + kCarDistance, 0);
        return car;
    }
    if (s32(guest.dword(kLocalCars)) > s32(kLocalMost))
    {
        guest.setByte(car + kCarAsleep, 0);
        guest.setDword(car + kCarDistance, players);
        return car;
    }
    const auto playerAt = [&] { return guest.dword(kCars + gameSettings(guest).localPlayer * 4); };
    const auto otherAt = [&] { return guest.dword(kCars + (1 - gameSettings(guest).localPlayer) * 4); };
    {
        const float awayX = sizeOfWay(guest, car, playerAt(), kCarX);
        const float awayZ = sizeOfWay(guest, car, playerAt(), kCarZ);
        guest.setFloat(car + kCarDistance, planeDistance(guest, awayX, awayZ));
    }
    s32 nodes = s32(guest::direct<0x4a00c0, nodeGap>(app, ts, guest.dword(car + kCarNode), guest.dword(playerAt() + kCarNode)));
    if (guest.dword(kLocalCars) == kLocalMost)
    {
        const float otherX = sizeOfWay(guest, car, otherAt(), kCarX);
        const float otherZ = sizeOfWay(guest, car, otherAt(), kCarZ);
        const float other = planeDistance(guest, otherX, otherZ);
        if (!jumpsAbove(fp::compare(Float(other), floatFrom(guest, car + kCarDistance))))
            guest.setFloat(car + kCarDistance, other);
        const s32 toOther =
            s32(guest::direct<0x4a00c0, nodeGap>(app, ts, guest.dword(car + kCarNode), guest.dword(otherAt() + kCarNode)));
        if (toOther <= nodes)
            nodes = toOther;
    }
    const Float byNodes = fp::mul(Float(nodes), doubleAt(guest, kNodeScale));
    const Float distance = floatFrom(guest, car + kCarDistance);
    const Float reach = jumpsAbove(fp::compare(distance, byNodes)) ? distance : byNodes;
    guest.setFloat(car + kCarDistance, float(reach));
    std::uint64_t reachBits;
    std::memcpy(&reachBits, &reach, sizeof reachBits);
    const u16 status = fp::compare(reach, doubleAt(guest, kFar));
    u32 eax = withStatus(u32(reachBits >> 32), status);
    if (jumpsAbove(status))
    {
        if (guest.byte(car + kCarAsleep) != 1)
        {
            guest.setDword(car + kCarSurface, 1);
            eax = guest.dword(car + kCarSurface);
            guest.setDword(car + kCarSurfaceKind, eax);
        }
        guest.setDword(car + kCarSpinA, 0);
        guest.setDword(car + kCarSpinB, 0);
    }
    else if (guest.byte(car + kCarAsleep) != 0)
    {
        guest.setByte(car + kCarAsleep, 0);
        fe::direct<kCarNodeNow, port::conv::carNodeNow>(guest.app(), guest.ts(), car);
        guest::direct<0x49ffa0, carTrackFrame>(app, ts, car);
        guest::direct<0x4a0140, carOntoRoad>(app, ts, car);
        const Float over = fe::direct<kCarHeightOverRoad, port::conv::carHeightOverRoad>(guest.app(), guest.ts(), car)->st0;
        guest.setDword(car + kCarSurface, 1);
        guest.setDword(car + kCarTiltA, 0);
        guest.setDword(car + kCarTiltB, 0);
        guest.setFloat(car + kCarRest, float(over));
        const Float raised = fp::add(over, floatFrom(guest, car + kCarHalfHeight));
        guest.setDword(car + kCarSurfaceKind, guest.dword(car + kCarSurface));
        guest.setFloat(car + kCarY, float(raised));
        const Float under = fe::direct<kCarLowestPoint, port::conv::carLowestPoint>(guest.app(), guest.ts(), car)->st0;
        guest.setDword(car + kCarUnder, 0);
        guest.setFloat(car + kCarY, float(fp::sub(floatFrom(guest, car + kCarY), under)));
        fe::direct<kClearRespawn, racestart::clearRespawnOf>(guest.app(), guest.ts(), car);
        eax = car;  // as the thunk of 0x4294c0 leaves it
    }
    guest.setByte(car + kCarAsleep, 0);
    return eax;
}

/* sub_4a08c0: the frame's work on a car's place on the track, from the race's table of a
 * car's functions: the rain over it outside a tunnel, how near the players, its node and
 * frame; awake, its surface. */
nfs::u32 frontend::sima::carTrackEnvironment(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    constexpr u32 kCarRain = 0x160, kCarStopped = 0x184, kCarMoving = 0x20, kCarWantsFrame = 0x76;
    if (guest.byte(car + kCarInRace) == 0)
        return car;
    const u32 rainy = gameSettings(guest).weather;
    if (rainy != 0)
    {
        guest.setDword(car + kCarRain, guest::direct<0x4dcdd0, simd::rainAmount>(app, ts));
        const u32 tunnel = *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, ts, car + kCarX, guest.dword(car + kCarNode));
        if (tunnel != 0)
            guest.setDword(car + kCarRain, 0);
    }
    else
    {
        guest.setDword(car + kCarRain, rainy);
    }
    u32 eax = guest::direct<0x4a0580, carNearPlayers>(app, ts, car);
    if (guest.dword(car + kCarStopped) != 0 && guest.dword(car + kCarMoving) == 0)
        return eax;
    eax = fe::direct<kCarNodeNow, port::conv::carNodeNow>(guest.app(), guest.ts(), car)->eax;
    if (guest.word(car + kCarWantsFrame) != 0)
        eax = guest::direct<0x49ffa0, carTrackFrame>(app, ts, car);
    if (guest.byte(car + kCarAsleep) == 0)
    {
        const u32 surface = fe::direct<0x4207e0, trackdata::placeSurface>(app, ts, car + kCarPlaceOnTrack, 1);
        guest.setDword(car + kCarSurface, surface);
        eax = surface & 0x3f;
        guest.setDword(car + kCarSurfaceKind, eax);
    }
    return eax;
}

/* sub_4a00c0: how many nodes from eax to edx (sub_40e120), under another name. */
nfs::u32 frontend::sima::nodeGap(App* app, ThreadState& ts, u32 node, u32 from)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    // By address: it leaves edx (the track's count) for this function's callers, which restores nothing.
    return u32(fe::direct<kNodeGap, aiplan::nodesBetween>(guest.app(), guest.ts(), node, from));
}

// ---------------------------------------------------------------------------
// As the game's functions stood: their frames, what they restore
// ---------------------------------------------------------------------------

/* sub_40de30: an opponent's lane votes added up and acted on. */
nfs::u32 frontend::simLaneSteer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (aiVotes(app, ts, args))
        return args.eax;
    static fe::Site site{ "sub_40de30", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::laneSteer(app, ts, car); });
}

nfs::u32 frontend::simEnginePull(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4064f0", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const sima::FloatLeft left = sima::enginePullOf(app, ts, car);
        args.floatSet = true;
        args.floatResult = left.value;
        return left.eax;
    });
}

nfs::u32 frontend::simHeadingError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4070c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::headingError(app, ts, car); });
}

nfs::u32 frontend::simCameraBesideRoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425060", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraBesideRoad(app, ts, camera); });
}

nfs::u32 frontend::simCameraNearestSpot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4251f0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraNearestSpot(app, ts, camera); });
}

nfs::u32 frontend::simCameraPlayPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4253d0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraPlayPath(app, ts, camera); });
}

nfs::u32 frontend::simCameraEndPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425640", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraEndPath(app, ts, camera); });
}

nfs::u32 frontend::simCameraOverRoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425bf0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraOverRoad(app, ts, camera); });
}

nfs::u32 frontend::simCameraFlyover(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_425d50", 0, 0, nullptr, 0 };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraFlyover(app, ts, camera); });
}

/* sub_426290: mode 20's step: nothing, eax as it came. */
nfs::u32 frontend::sima::cameraStill(App*, ThreadState&, u32 camera)
{
    return camera;
}

nfs::u32 frontend::simCameraStill(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = sima::cameraStill(app, ts, args.eax);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::simCameraWaitThenHandOn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4262a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 camera = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraWaitThenHandOn(app, ts, camera); });
}

nfs::u32 frontend::simCameraBackFromWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426310", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraBackFromWait(app, ts, view); });
}

nfs::u32 frontend::simCameraStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426450", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 mode = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::cameraStep(app, ts, mode, view); });
}

nfs::u32 frontend::simCameraForcedMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426560", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::forcedMode(app, ts, view); });
}

nfs::u32 frontend::simCamerasStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_426680", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::camerasStep(app, ts); });
}

nfs::u32 frontend::simQuickDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4965b0", 0, 0, nullptr, 0 };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const sima::FloatLeft left = sima::quickDistanceOf(app, ts, a, b);
        args.floatSet = true;
        args.floatResult = left.value;
        return left.eax;
    });
}

nfs::u32 frontend::simPlanarLength(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4967e0", 8, 0, nullptr, 0 };
    const u32 a = app->getMemory<u32>(ts.sp + 4), b = app->getMemory<u32>(ts.sp + 8);
    const u32 eax = args.eax;  // it pushes eax too
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = sima::planarLengthOf(app, ts, a, b);
        return eax;
    });
}

nfs::u32 frontend::simCarTiltedOver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49fde0", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::carTiltedOver(app, ts, car); });
}

nfs::u32 frontend::simSliceHeading(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49fee0", 0, 0, nullptr, 0 };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::sliceHeading(app, ts, index); });
}

nfs::u32 frontend::simCarTrackFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ffa0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::carTrackFrame(app, ts, car); });
}

nfs::u32 frontend::simNodeGap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a00c0", 0, 0, nullptr, 0, 0 };
    const u32 node = args.eax, from = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::nodeGap(app, ts, node, from); });
}

nfs::u32 frontend::simCarOntoRoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a0140", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::carOntoRoad(app, ts, car); });
}

nfs::u32 frontend::simCarNearPlayers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a0580", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::carNearPlayers(app, ts, car); });
}

nfs::u32 frontend::simCarTrackEnvironment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a08c0", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return sima::carTrackEnvironment(app, ts, car); });
}

}
