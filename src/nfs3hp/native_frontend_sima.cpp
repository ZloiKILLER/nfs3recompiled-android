#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstring>

/* The simulation's generated code that was left under 0x4a0aa0, decompiled:
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
 * guest memory, what these call is called by address, and the locals the game's
 * functions read are where they were in the frame. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* NFS_CAR_TRACE's records of a driver's lane votes (native_vertices.cpp). */
bool aiVotes(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg32;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered
bool jumpsEqual(reg16 status) { return (status & 0x4000) != 0; }  // je; true unordered

/* eax after fnstsw ax: only its low half changes. */
reg32 withStatus(reg32 eax, reg16 status) { return (eax & 0xffff0000) | status; }

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A float loaded on the x87 and stored back, by way of a double in memory: the same but for
 * a signalling NaN, quieted.  Written out, for a compiler folds float(double(x)) to x. */
float throughX87(float value)
{
    const reg32 bits = bitsOf(value);
    const bool signalling = (bits & 0x7f800000) == 0x7f800000 && (bits & 0x007fffff) != 0 && (bits & 0x00400000) == 0;
    return signalling ? floatOf(bits | 0x00400000) : value;
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, reg32 address) { return Float(guest.floatAt(address)); }

/* fild dword */
Float integerFrom(const Guest& guest, reg32 address) { return Float(sreg32(guest.dword(address))); }

/* The original's own absolute value, inline at each of its uses: fldz; fcomp x and
 * the sign changed only below 0, so that -0 stays -0 and a NaN's bits stay as they
 * are.  `status` is what the comparison left, for the eax some callers go on with. */
float absolute(float value, reg16& status)
{
    status = fp::compare(0.0, Float(value));
    return jumpsAbove(status) ? float(-Float(value)) : value;
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* An index past the end of a ring of `count`, brought back once. */
sreg32 wrapped(sreg32 index, sreg32 count) { return index >= count ? index - count : index; }

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The track's slices.
constexpr reg32 kSliceCount = 0x5dd940;
constexpr reg32 kSlices = 0x5dd948;
constexpr reg32 kSliceBytes = 36;
constexpr reg32 kSliceFlags = 0x0c;          // word: the road's bits, for whether a camera may stay
constexpr reg32 kSliceLimits = 0x0e;         // byte: which of the road's two limits this slice holds a car to

reg32 slice(const Guest& guest, reg32 index) { return guest.dword(kSlices) + index * kSliceBytes; }

// The race.
constexpr reg32 kPlayers = 0x6fd3b0;         // on this machine, less one
constexpr reg32 kCars = 0x5efa48;            // reg32[]: the cars, the players first
constexpr reg32 kCarsByPlace = 0x5efac8;     // reg32[]
constexpr reg32 kCarCount = 0x6fd518;
constexpr reg32 kPlayerNow = 0x6fd4f4;       // the player, an index of kCars
constexpr reg32 kViewedCar = 0x6fd4f8;
constexpr reg32 kLocalCars = 0x5efda4;       // how many cars players drive here
constexpr reg32 kNetworkGame = 0x7a22da;     // word

// A car.
constexpr reg32 kCarPlaceOnTrack = 0x0c;     // what the track's functions take
constexpr reg32 kCarNode = 0x1c;
constexpr reg32 kCarDistance = 0x84;         // float: from the nearest player
constexpr reg32 kCarAsleep = 0x88;           // byte: past the players' reach
constexpr reg32 kCarInRace = 0x89;           // byte
constexpr reg32 kCarX = 0x98, kCarY = 0x9c, kCarZ = 0xa0;
constexpr reg32 kCarMatrix = 0xc0;
constexpr reg32 kCarHalfWidth = 0x114, kCarHalfHeight = 0x118, kCarHalfLength = 0x11c;
constexpr reg32 kCarSurface = 0x188, kCarSurfaceKind = 0x18c;
constexpr reg32 kCarFlags = 0x200;           // byte

// A race camera.
constexpr reg32 kCameraCar = 0x04;
constexpr reg32 kCameraView = 0x0c;          // 0 or 1: the player's half of the screen
constexpr reg32 kCameraX = 0x14, kCameraY = 0x18, kCameraZ = 0x1c;
constexpr reg32 kCameraTarget = 0x20;
constexpr reg32 kCameraMatrix = 0x2c;
constexpr reg32 kCameraZoom = 0x54;
constexpr reg32 kCameraNode = 0x84;
constexpr reg32 kCameraSurface = 0xec;
// What a mode keeps for itself: a path's started, kind and animation; a wait's time, tick and next mode.
constexpr reg32 kCameraOwn110 = 0x110, kCameraOwn114 = 0x114, kCameraOwn118 = 0x118, kCameraOwn11c = 0x11c;

constexpr reg32 kCamerasOfModes = 0x552ec8;  // reg32[view + mode * 2]: each mode's camera of each view
constexpr reg32 kCameraNow = 0x5e10b0;       // reg32[2]: each view's camera
constexpr reg32 kCameraForced = 0x5e10b8;    // reg32[2]: the one a forced mode has put over it, or 0
constexpr reg32 kChosenMode = 0x6fbbb8;      // reg32[2]: the mode each player chose
constexpr reg32 kFirstCycleMode = 0x5e1054;

// The game's functions, by address.
constexpr reg32 kSumLaneVotes = 0x40da30, kSteerToLane = 0x40daf0;
constexpr reg32 kWayToGo = 0x406e50;         // eax the car, esi where its floats go
constexpr reg32 kArcTangent = 0x4e06d9;      // st(1) by st(0), into st(0)
constexpr reg32 kArcTangentOfFloats = 0x4ea9c0;  // the stack's two floats, onto the x87; ret 8
constexpr reg32 kTruncate = 0x4dfd56;
constexpr reg32 kRoadBitsBroken = 0x423fe0;
constexpr reg32 kLanePosition = 0x41ffd0;    // eax a place on the track, edx three ints, ebx the side; a surface
constexpr reg32 kCameraOnRoad = 0x4bdc70, kCameraKeepRoad = 0x4bdcc0, kCameraOffRoad = 0x4bdbd0;
constexpr reg32 kMoveCameraTowards = 0x424fd0, kGlideCameraTowards = 0x425b40;
constexpr reg32 kLookAtTarget = 0x422290, kCameraSettled = 0x424f30;
constexpr reg32 kNearestSpot = 0x423d90;
constexpr reg32 kSetCameraMode = 0x4238b0, kSetForcedMode = 0x423960, kSwitchCamera = 0x422610, kModeInCycle = 0x423ab0;
constexpr reg32 kPrepareCamera = 0x426370;
constexpr reg32 kMatrixFromQuaternion = 0x4b7590, kOrthonormalise = 0x4b79c0;
constexpr reg32 kAngleParts = 0x4eac60;      // eax an angle, edx and ebx two ints
constexpr reg32 kStartAnimation = 0x40f3b0, kAnimationStep = 0x40f940, kAnimationPlaying = 0x40fac0;
constexpr reg32 kStopAnimation = 0x40f460, kPathEndPose = 0x40f8d0;
constexpr reg32 kMultiplyMatrices = 0x4e0280, kNormalise = 0x4972f0, kScaleVectors = 0x4e0050, kAddVectors = 0x4dffb0;
constexpr reg32 kTransformPoints = 0x4e0430, kDotOntoX87 = 0x4e01f0;
constexpr reg32 kAttachCameras = 0x4215c0, kRebuildHudView = 0x485640, kRebuildHudElement = 0x4853c0, kSetHudPane = 0x4844c0;
constexpr reg32 kMemmove = 0x4ea4f0;
constexpr reg32 kPositionFrame = 0x420230, kPositionSurface = 0x4207e0, kInTunnel = 0x420bf0;
constexpr reg32 kNodeGap = 0x40e120;
constexpr reg32 kCarNodeNow = 0x49fe70, kCarHeightOverRoad = 0x4a00d0, kCarLowestPoint = 0x4a0440, kClearRespawn = 0x4294c0;
constexpr reg32 kRainAt = 0x4dcdd0;

// And what is in this file: they call each other by address, as they did.
constexpr reg32 kCameraStep = 0x426450, kBackFromWait = 0x426310, kForcedMode = 0x426560;
constexpr reg32 kSliceHeading = 0x49fee0, kCarTrackFrame = 0x49ffa0, kCarOntoRoad = 0x4a0140;
constexpr reg32 kCarNearPlayers = 0x4a0580, kNodeGapOfCars = 0x4a00c0;

// ---------------------------------------------------------------------------
// The opponents
// ---------------------------------------------------------------------------

/* sub_4064f0: the pull of an opponent's engine at its speed, onto the x87. */
reg32 enginePull(Guest& guest, reg32 car)
{
    constexpr std::uint8_t kCarLimited = 0x01, kCarInTraffic = 0x10, kCarSlowed = 0x20;
    constexpr reg32 kDriving = 0x528;
    constexpr reg32 kLimitShare = 0x1cc, kPullBySpeed = 0x2ec;
    constexpr reg32 kThrottle = 0x7bc, kPedal = 0x7c0, kSpeed = 0x7c4;
    constexpr reg32 kMinusHalf = 0x536254, kTenthOff = 0x53625c, kBoost = 0x536260, kTwice = 0x536264, kThrice = 0x536268;
    constexpr reg32 kMostWhenStill = 0x53626c, kFastEnough = 0x536274, kHalf = 0x53627c;

    guest.setByte(car + 0x509, 0);
    if (guest.byte(car + kCarFlags) & kCarLimited)
    {
        // Held back between its limit and the limit's share of the throttle.
        const reg32 limitBits = guest.dword(guest.dword(car + kDriving) + kLimitShare);
        const Float limit = Float(floatOf(limitBits));
        reg16 status = fp::compare(fp::mul(limit, floatFrom(guest, car + kThrottle)), floatFrom(guest, car + kPedal));
        if (jumpsBelow(status))
        {
            guest.pushFloat(0.0f);
            return withStatus(limitBits, status);
        }
        status = fp::compare(floatFrom(guest, car + kPedal), limit);
        if (jumpsAbove(status))
        {
            guest.pushFloat(0.0f);
            return withStatus(limitBits, status);
        }
    }
    // The speed in m/s, a row of the car's table.
    sreg32 row = fp::toInteger<sreg32>(fp::add(floatFrom(guest, car + kSpeed), doubleAt(guest, kMinusHalf)));
    if (row > 0x6f)
        row = 0x6f;
    if (row <= 0)
        row = 0;
    float pull = guest.floatAt(guest.dword(car + kDriving) + reg32(row) * 4 + kPullBySpeed);
    if (guest.dword(0x6fd4f0) == 0 && sreg32(guest.dword(car + 8)) > 100)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kTenthOff)));
    if (guest.dword(0x6fd4cc) != 0 && row < 45)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kTenthOff)));
    if (sreg32(guest.dword(car + 0x6e4)) > 0)
        pull = float(fp::mul(Float(pull), floatFrom(guest, kBoost)));
    // The network's catch-up: a car of no player here, far from the player.
    if (guest.dword(0x6fd50c) != 0)
    {
        const reg32 driver = guest.dword(car + 0x1f0);
        if (driver != guest.dword(kPlayerNow)
            && (guest.word(kNetworkGame) == 0 || sreg32(driver) < sreg32(guest.dword(kCarCount))))
        {
            const reg32 player = guest.dword(kCars + guest.dword(kPlayerNow) * 4);
            const sreg32 gap = sreg32(guest.dword(car + kCarNode) - guest.dword(player + kCarNode));
            if (gap <= -3 || gap >= 15)
            {
                const reg32 factor = gap > -8 && gap < 30 ? kTwice : kThrice;
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
    guest.pushFloat(pull);
    return bitsOf(pull);
}

/* sub_4070c0: how far the way the driver wants to go (sub_406e50) is from the way
 * its car points, in turns, to 0x56ec60; the turn it means to make to 0x56ec64 and
 * the way itself to 0x56ec5c. */
reg32 headingError(Guest& guest, reg32 car)
{
    constexpr reg32 kPlannedTurn = 0x210;
    constexpr reg32 kWanted = 0x56ec64, kWay = 0x56ec5c, kError = 0x56ec60, kErrorSpare = 0x56ec68;
    constexpr reg32 kTurnsPerPositive = 0x5362d4, kTurnsPerNegative = 0x5362dc, kHalfTurn = 0x5362e4;
    constexpr reg32 kTurnsPerRadian = 0x5362ec, kMinusHalfTurn = 0x5362f4, kMinusOne = 0x5362fc;
    constexpr reg32 kLocalAcross = 0x00, kLocalAlong = 0x08;

    const reg32 frame = guest.frame();
    const reg32 planned = guest.dword(car + kPlannedTurn);
    guest.setDword(kErrorSpare, 0);
    const Float wanted = sreg32(planned) > 0
        ? fp::mul(integerFrom(guest, car + kPlannedTurn), doubleAt(guest, kTurnsPerPositive))
        : fp::add(fp::mul(Float(sreg32(planned + 0x200)), doubleAt(guest, kTurnsPerNegative)), doubleAt(guest, kHalfTurn));
    guest.setFloat(kWanted, float(wanted));

    guest.setEsi(frame);
    guest.call(kWayToGo, args({ car }));
    x86::FPU& fpu = guest.fpu();
    fpu.push(floatFrom(guest, frame + kLocalAlong));
    fpu.push(floatFrom(guest, frame + kLocalAcross));
    const reg32 eax = guest.call(kArcTangent);
    const Float way = fp::mul(fpu.take(), doubleAt(guest, kTurnsPerRadian));
    guest.setFloat(kWay, float(way));
    if (jumpsAbove(fp::compare(0.0, way)))
        guest.setFloat(kWay, float(fp::add(1.0, way)));

    const Float error = fp::sub(floatFrom(guest, kWanted), floatFrom(guest, kWay));
    guest.setFloat(kError, float(error));
    if (jumpsBelow(fp::compare(error, doubleAt(guest, kMinusHalfTurn))))
        guest.setFloat(kError, float(fp::add(1.0, error)));

    const Float kept = floatFrom(guest, kError);
    const reg16 status = fp::compare(kept, doubleAt(guest, kHalfTurn));
    if (jumpsAbove(status))
        guest.setFloat(kError, float(fp::add(kept, doubleAt(guest, kMinusOne))));
    return withStatus(eax, status);
}

// ---------------------------------------------------------------------------
// The race cameras
// ---------------------------------------------------------------------------

/* Whether the road's bits at either of two nodes have gone (sub_423fe0). */
bool roadBroken(Guest& guest, reg32 cameraNode, reg32 carNode)
{
    if (guest.call(kRoadBitsBroken, args({ guest.word(slice(guest, cameraNode) + kSliceFlags) })) != 0)
        return true;
    return guest.call(kRoadBitsBroken, args({ guest.word(slice(guest, carNode) + kSliceFlags) })) != 0;
}

/* The race not yet under way on the screen, or its cameras frozen. */
bool cameraHeld(const Guest& guest)
{
    constexpr reg32 kStartShown = 0x7d36a8, kStarted = 0x7cd960, kFrozen = 0x5600ec;
    if (guest.dword(kStartShown) != 0 && guest.dword(kStarted) == 0)
        return true;
    return guest.dword(kFrozen) != 0;
}

/* What the two lane cameras have in common: on the road and the road gone, the side
 * from the car's damage; the place in the lane there, in 16.16, to floats in `spot`,
 * the second one raised by [raise] if any. */
struct LaneView
{
    reg32 surface;
    bool moved;
};

LaneView laneView(Guest& guest, reg32 camera, reg32 sideField, reg32 scaleAt, reg32 raise, reg32 ints, reg32 spot)
{
    constexpr reg32 kCarDamage = 0x638;
    const reg32 place = guest.dword(camera + kCameraCar) + kCarPlaceOnTrack;
    if (guest.call(kCameraOnRoad, args({ guest.dword(camera + kCameraView) })) != 0
        && roadBroken(guest, guest.dword(camera + kCameraNode), guest.dword(place + kCarNode - kCarPlaceOnTrack)))
    {
        guest.setDword(camera + sideField, sreg32(guest.dword(guest.dword(camera + kCameraCar) + kCarDamage)) < 7 ? 0 : 1);
    }
    const reg32 surface = guest.call(kLanePosition, args({ place, ints, guest.dword(camera + sideField) }));
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

/* sub_425060: mode 8, beside the road near the car; a move past the place counts up,
 * and the third lets the zoom go. */
reg32 cameraBesideRoad(Guest& guest, reg32 camera)
{
    constexpr reg32 kSide = 0x130, kMoving = 0x124, kKeepZoom = 0x12c, kMoves = 0x553024, kScale = 0x536f04;
    constexpr reg32 kLocalInts = 0x00, kLocalSpot = 0x0c;
    constexpr sreg32 kMovesToLetGo = 3;

    if (cameraHeld(guest))
        return guest.eax();
    const reg32 frame = guest.frame();
    const LaneView view = laneView(guest, camera, kSide, kScale, 0, frame + kLocalInts, frame + kLocalSpot);
    if (view.moved)
    {
        const reg32 moves = guest.dword(kMoves) + 1;
        guest.setDword(camera + kMoving, 0);
        guest.setDword(kMoves, moves);
        if (sreg32(moves) < kMovesToLetGo)
        {
            guest.setDword(camera + kKeepZoom, 1);
        }
        else
        {
            guest.setDword(camera + kKeepZoom, 0);
            guest.setDword(kMoves, 0);
            guest.setDword(camera + kCameraZoom, 0);
        }
        guest.call(kCameraOffRoad, args({ guest.dword(camera + kCameraView) }));
    }
    guest.call(kMoveCameraTowards, args({ camera, frame + kLocalSpot }, { guest.dword(camera + kCameraOwn110) }));
    guest.setDword(camera + kCameraSurface, view.surface);
    guest.call(kLookAtTarget, args({ camera, camera + kCameraX }));
    return guest.call(kCameraSettled, args({ camera }));
}

/* sub_425bf0: mode 14, the same with a side and speeds of its own, higher up. */
reg32 cameraOverRoad(Guest& guest, reg32 camera)
{
    constexpr reg32 kScale = 0x536f2c, kRaise = 0x536f34;
    constexpr reg32 kLocalInts = 0x00, kLocalSpot = 0x0c;

    if (cameraHeld(guest))
        return guest.eax();
    const reg32 frame = guest.frame();
    const LaneView view = laneView(guest, camera, kCameraOwn11c, kScale, kRaise, frame + kLocalInts, frame + kLocalSpot);
    if (view.moved)
        guest.call(kCameraOffRoad, args({ guest.dword(camera + kCameraView) }));
    guest.call(kGlideCameraTowards, args({ camera, frame + kLocalSpot },
                                         { guest.dword(camera + kCameraOwn110), guest.dword(camera + kCameraOwn114) }));
    guest.setDword(camera + kCameraSurface, view.surface);
    guest.call(kLookAtTarget, args({ camera, camera + kCameraX }));
    return guest.call(kCameraSettled, args({ camera }));
}

/* sub_4251f0: mode 18, the nearest of the track's fixed spots, records of 28 bytes at
 * [0x552e50]: where it stands, its surface, whether it looks at the car or holds the
 * quaternion it has, and a zoom from its angle.  None near: mode 8. */
reg32 cameraNearestSpot(Guest& guest, reg32 camera)
{
    constexpr reg32 kSpots = 0x552e50, kSpotBytes = 28;
    constexpr reg32 kSpotSurface = 0x18, kSpotFlags = 0x16, kSpotAngle = 0x12;
    constexpr std::uint8_t kLooksAtCar = 0x01, kFollows = 0x02;
    constexpr reg32 kScale = 0x536f0c, kIdentity = 0x564498, kLeastZoom = 0x536f14;
    constexpr reg32 kCameraTargetCar = 8, kCameraLooking = 0x108;
    constexpr reg32 kLocalQuaternion = 0x00, kLocalPartA = 0x10, kLocalPartB = 0x14;
    constexpr reg32 kModeBesideRoad = 8;
    constexpr reg32 kLeastZoomBits = 0x3f333333, kFollowBits = 0x40c00000;

    const reg32 frame = guest.frame();
    const reg32 spot = guest.call(kNearestSpot, args({ guest.dword(camera + kCameraView) }));
    if (spot == reg32(-1))
    {
        guest.call(kSetCameraMode, args({ guest.dword(camera + kCameraView), kModeBesideRoad }));
        return guest.call(kCameraStep, args({ kModeBesideRoad, guest.dword(camera + kCameraView) }));
    }
    const reg32 record = guest.dword(kSpots) + spot * kSpotBytes;
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
        guest.call(kLookAtTarget, args({ camera, camera + kCameraX }));
    }
    else
    {
        guest.setDword(camera + kCameraLooking, 0);
        // The quaternion: four int16 from +0xc, each in 14.2 of 16.16.
        for (reg32 i = 0; i < 4; ++i)
        {
            const sreg32 part = (sreg32(guest.dword(record + 0x0a + 2 * i)) >> 16) * 4;
            guest.setFloat(frame + kLocalQuaternion + 4 * i,
                           float(i == 3 ? fp::mul(scale, Float(part)) : fp::mul(Float(part), scale)));
        }
        guest.call(kMatrixFromQuaternion, args({ frame + kLocalQuaternion, camera + kCameraMatrix }));
        guest.call(kOrthonormalise, args({ camera + kCameraMatrix }));
    }
    const reg32 again = guest.dword(kSpots) + spot * kSpotBytes;
    if (guest.byte(again + kSpotFlags) & kFollows)
        return guest.call(kMoveCameraTowards, args({ camera, camera + kCameraX }, { kFollowBits }));
    const sreg32 angle = (sreg32(guest.dword(again + kSpotAngle)) >> 16) * 512 / 360;
    guest.call(kAngleParts, args({ reg32(angle), frame + kLocalPartA, frame + kLocalPartB }));
    const Float ratio = fp::div(integerFrom(guest, frame + kLocalPartB), integerFrom(guest, frame + kLocalPartA));
    const reg32 zoom = jumpsBelow(fp::compare(ratio, floatFrom(guest, kLeastZoom))) ? kLeastZoomBits : bitsOf(float(ratio));
    guest.setDword(camera + kCameraZoom, zoom);
    return zoom;
}

/* As an animated path ends: the mode the player chose if it is in the cycle, else the
 * cycle's first. */
reg32 switchToChosenMode(Guest& guest, reg32 camera)
{
    const reg32 inCycle = guest.call(kModeInCycle, args({ guest.dword(kChosenMode + guest.dword(camera + kCameraView) * 4) }));
    const reg32 view = guest.dword(camera + kCameraView);
    const reg32 mode = inCycle != 0 ? guest.dword(kChosenMode + view * 4) : guest.dword(kFirstCycleMode);
    return guest.call(kSwitchCamera, args({ mode, view }));
}

/* sub_4253d0: mode 16, an animated path, its pose laid over the car's body when its
 * kind (4, 5) rides on the car, its target no farther than the car's reach. */
reg32 cameraPlayPath(Guest& guest, reg32 camera)
{
    constexpr reg32 kStarted = kCameraOwn110, kPath = kCameraOwn114, kAnimation = kCameraOwn118;
    constexpr reg32 kNone = reg32(-1);
    constexpr reg32 kCarBody = 0x124, kCarReach = 0x120;
    constexpr reg32 kLocalMatrix = 0x00;       // 9 floats: the orientation
    constexpr reg32 kLocalStepMatrix = 0x24;   // 9 floats
    constexpr reg32 kLocalTarget = 0x48;       // 3 floats
    constexpr reg32 kLocalStepPlace = 0x54;    // 3 floats
    constexpr reg32 kLocalCarPlace = 0x60;     // 3 floats
    constexpr reg32 kLocalDirection = 0x6c;    // 3 floats

    const reg32 frame = guest.frame();
    if (guest.dword(camera + kStarted) == 0)
    {
        const reg32 animation = guest.call(kStartAnimation, args({ guest.dword(camera + kPath) }));
        guest.setDword(camera + kAnimation, animation);
        if (animation == kNone)
            return switchToChosenMode(guest, camera);
        guest.setDword(camera + kStarted, 1);
    }
    if (guest.dword(camera + kAnimation) == kNone)
        guest.setDword(camera + kAnimation, guest.call(kStartAnimation, args({ guest.dword(camera + kPath) })));
    if (guest.dword(camera + kAnimation) == kNone)
        return guest.eax();

    guest.call(kAnimationStep, args({ guest.dword(camera + kAnimation), frame + kLocalStepPlace, frame + kLocalStepMatrix }));
    if (guest.call(kAnimationPlaying, args({ guest.dword(camera + kAnimation) })) == 0)
    {
        guest.call(kStopAnimation, args({ guest.dword(camera + kAnimation) }));
        guest.setDword(camera + kAnimation, kNone);
        return switchToChosenMode(guest, camera);
    }
    const reg32 path = guest.dword(camera + kPath);
    if (path == 5 || path == 4)
    {
        const reg32 body = guest.dword(camera + kCameraCar) + kCarBody;
        guest.call(kMultiplyMatrices, args({ frame + kLocalStepMatrix, body, frame + kLocalMatrix }));
        // The step's place by the body: each row's three products, then their sums, the first row first.
        const reg32 place = frame + kLocalStepPlace;
        Float sum[3];
        for (reg32 axis = 0; axis < 3; ++axis)
            sum[axis] = fp::mul(floatFrom(guest, place), floatFrom(guest, body + 4 * axis));
        for (reg32 row = 1; row < 3; ++row)
            for (reg32 axis = 0; axis < 3; ++axis)
                sum[axis] = fp::add(sum[axis], fp::mul(floatFrom(guest, place + 4 * row), floatFrom(guest, body + 12 * row + 4 * axis)));
        for (reg32 axis = 0; axis < 3; ++axis)
            guest.setFloat(frame + kLocalTarget + 4 * axis, float(sum[axis]));
        copyDwords(guest, frame + kLocalDirection, frame + kLocalTarget, 3);
        guest.call(kNormalise, args({ frame + kLocalDirection }));
        const Float length = guest.fpu().take();
        const float reach = float(fp::add(1.0, floatFrom(guest, guest.dword(camera + kCameraCar) + kCarReach)));
        if (jumpsBelow(fp::compare(length, Float(reach))))
        {
            guest.call(kScaleVectors, args({ 1, frame + kLocalDirection }, { bitsOf(reach), frame + kLocalDirection }));
            copyDwords(guest, frame + kLocalTarget, frame + kLocalDirection, 3);
        }
    }
    else
    {
        copyDwords(guest, frame + kLocalMatrix, frame + kLocalStepMatrix, 9);
        copyDwords(guest, frame + kLocalTarget, frame + kLocalStepPlace, 3);
    }
    const reg32 car = guest.dword(camera + kCameraCar);
    guest.setDword(frame + kLocalCarPlace, guest.dword(car + kCarX));
    guest.setDword(frame + kLocalCarPlace + 4, guest.dword(car + kCarY));
    guest.setDword(frame + kLocalCarPlace + 8, guest.dword(car + kCarZ));
    guest.call(kAddVectors, args({ 1, frame + kLocalCarPlace, frame + kLocalTarget, camera + kCameraX }));
    copyDwords(guest, camera + kCameraTarget, frame + kLocalTarget, 3);
    copyDwords(guest, camera + kCameraMatrix, frame + kLocalMatrix, 9);
    return camera;
}

/* A path's end pose, handed back in 16.16 to `placeInts` and `matrixInts`, as floats
 * to where the path's steps put theirs. */
void pathEndPoseAsFloats(Guest& guest, reg32 path, reg32 placeInts, reg32 matrixInts, reg32 place, reg32 matrix)
{
    constexpr reg32 kScale = 0x536f1c;
    guest.call(kPathEndPose, args({ path, 2, 1, placeInts }, { matrixInts }));
    const Float scale = doubleAt(guest, kScale);
    for (reg32 i = 0; i < 3; ++i)
        guest.setFloat(place + 4 * i, float(fp::mul(integerFrom(guest, placeInts + 4 * i), scale)));
    for (reg32 i = 0; i < 9; ++i)
        guest.setFloat(matrix + 4 * i, float(fp::mul(integerFrom(guest, matrixInts + 4 * i), scale)));
}

/* sub_425640: mode 17, the path at a race's end, its zoom fixed; when it is over, with
 * more than two players, the view handed to the first car still racing, the HUD with
 * it, else it stays on the path's end. */
reg32 cameraEndPath(Guest& guest, reg32 camera)
{
    constexpr reg32 kPath = kCameraOwn114, kAnimation = kCameraOwn118;
    constexpr reg32 kNone = reg32(-1);
    constexpr reg32 kEndZoomBits = 0x3f99999a;
    constexpr reg32 kCarFinished = 0x2c4;
    constexpr std::uint8_t kCarDone = 0x80, kCarRacing = 0x01;
    constexpr reg32 kPaneCopied = 0x7d3694, kPaneFrom = 0x6fbc50, kPaneTo = 0x6fc628, kPaneBytes = 0x1a4;
    constexpr reg32 kPaneOffsetA = 0x6fbd84, kPaneOffsetB = 0x6fbd88, kPaneScaleA = 0x6fbd8c, kPaneScaleB = 0x6fbd90;
    constexpr reg32 kPaneStyle = 0x6fbc70, kPaneScale = 0x6fbc60;
    constexpr reg32 kOneBits = 0x3f800000;
    constexpr reg32 kElementMap = 0x11;
    constexpr reg32 kLocalMatrixInts = 0x00;    // 9 ints, a pose asked of the path now
    constexpr reg32 kLocalMatrix = 0x24;        // 9 floats
    constexpr reg32 kLocalEndMatrixInts = 0x48; // 9 ints, the pose as the path ends
    constexpr reg32 kLocalEndPlaceInts = 0x6c;  // 3 ints
    constexpr reg32 kLocalPlaceInts = 0x78;     // 3 ints
    constexpr reg32 kLocalPlace = 0x84;         // 3 floats

    const reg32 frame = guest.frame();
    guest.setDword(camera + kCameraZoom, kEndZoomBits);
    if (guest.dword(camera + kAnimation) == kNone)
    {
        pathEndPoseAsFloats(guest, guest.dword(camera + kPath), frame + kLocalPlaceInts, frame + kLocalMatrixInts,
                            frame + kLocalPlace, frame + kLocalMatrix);
    }
    else
    {
        guest.call(kAnimationStep, args({ guest.dword(camera + kAnimation), frame + kLocalPlace, frame + kLocalMatrix }));
        if (guest.call(kAnimationPlaying, args({ guest.dword(camera + kAnimation) })) == 0)
        {
            guest.call(kStopAnimation, args({ guest.dword(camera + kAnimation) }));
            guest.setDword(camera + kAnimation, kNone);
            if (sreg32(guest.dword(kPlayers)) > 1)
            {
                sreg32 place = 0;
                for (; place < sreg32(guest.dword(kCarCount)); ++place)
                {
                    const reg32 car = guest.dword(kCarsByPlace + reg32(place) * 4);
                    if (guest.dword(car + kCarFinished) != 0)
                        continue;
                    const std::uint8_t flags = guest.byte(car + kCarFlags);
                    if ((flags & kCarDone) || !(flags & kCarRacing))
                        continue;
                    guest.setDword(kViewedCar, reg32(place));
                    guest.call(kAttachCameras, args({ 0, reg32(place) }));
                    guest.call(kRebuildHudView, args({ 0 }));
                    if (guest.dword(kPaneCopied) == 0)
                    {
                        guest.call(kMemmove, args({ kPaneFrom, kPaneTo, kPaneBytes }));
                        guest.setDword(kPaneCopied, 1);
                    }
                    guest.setDword(kPaneOffsetB, 0);
                    guest.setDword(kPaneOffsetA, 0);
                    guest.setDword(kPaneScaleB, kOneBits);
                    guest.setDword(kPaneScaleA, kOneBits);
                    guest.setDword(kPaneStyle, 2);
                    guest.setDword(kPaneScale, kOneBits);
                    guest.call(kRebuildHudElement, args({ 0, kElementMap }));
                    return guest.call(kSetHudPane, args({ 0 }));
                }
                return reg32(place);
            }
            pathEndPoseAsFloats(guest, guest.dword(camera + kPath), frame + kLocalEndPlaceInts, frame + kLocalEndMatrixInts,
                                frame + kLocalPlace, frame + kLocalMatrix);
        }
    }
    const reg32 result = guest.call(kAddVectors, args({ 1, guest.dword(camera + kCameraCar) + kCarX, frame + kLocalPlace, camera + kCameraX }));
    copyDwords(guest, camera + kCameraMatrix, frame + kLocalMatrix, 9);
    return result;
}

/* sub_425d50: mode 9, the flyover: along the track's slices at a gear chosen from the
 * car's pace by a table of eight, jumping a few nodes on when the car has drawn ahead of
 * it or back when fallen behind; a little lower the farther from the car.  The tables
 * are copied into its frame and read there, unchecked: a pace past the last gear reads
 * on into its locals, so those it has written are where they were. */
reg32 cameraFlyover(Guest& guest, reg32 camera)
{
    constexpr reg32 kPaceOfGear = 0x423d50, kStepsOfGear = 0x423d70, kGears = 8;
    constexpr reg32 kCounting = 0x7d3680;
    constexpr reg32 kStep = 0x553004, kNode = 0x55300c, kGear = 0x553014;
    constexpr reg32 kCarPace = 0x7c0;
    constexpr reg32 kPaceClose = 0x536f38, kPaceCloseBack = 0x536f3c;
    constexpr reg32 kJumpNear = 0x536f44, kJumpMiddle = 0x536f48, kJumpBackNear = 0x536f4c;
    constexpr reg32 kScale = 0x536f54, kQuarter = 0x536f5c, kMostDrop = 0x536f60, kHeight = 0x536f38;
    constexpr reg32 kMostDropBits = 0x40600000;
    constexpr reg32 kLocalPaces = 0x00, kLocalSteps = 0x20;
    constexpr reg32 kLocalPace = 0x88, kLocalOffGear = 0xa0;
    constexpr sreg32 kCarAhead = 1, kCarBehind = 2;
    constexpr reg32 kRoadKept = 9;

    const reg32 frame = guest.frame();
    copyDwords(guest, frame + kLocalPaces, kPaceOfGear, kGears);
    copyDwords(guest, frame + kLocalSteps, kStepsOfGear, kGears);
    const reg32 car = guest.dword(camera + kCameraCar);
    if (cameraHeld(guest))
        return guest.eax();
    const auto view = [&] { return guest.dword(camera + kCameraView); };
    const auto gearPace = [&] { return floatFrom(guest, frame + kLocalPaces + guest.dword(kGear + view() * 4) * 4); };

    if (guest.dword(kCounting) != 0)
        guest.setDword(kStep + view() * 4, guest.dword(kStep + view() * 4) + 1);

    sreg32 off = 0;
    if (guest.dword(kStep + view() * 4) == guest.dword(frame + kLocalSteps + guest.dword(kGear + view() * 4) * 4))
    {
        guest.setDword(kStep + view() * 4, 0);
        guest.setDword(kNode + view() * 4,
                       reg32(wrapped(sreg32(guest.dword(kNode + view() * 4) + 1), sreg32(guest.dword(kSliceCount)))));
        const reg32 paceBits = guest.dword(car + kCarPace);
        guest.setDword(frame + kLocalPace, paceBits);
        const Float pace = Float(floatOf(paceBits));
        const sreg32 gap = sreg32(guest.dword(car + kCarNode) - guest.dword(kNode + view() * 4));
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
            reg32 gear = 0;
            while (jumpsAbove(fp::compare(pace, floatFrom(guest, frame + kLocalPaces + gear * 4))))
                ++gear;
            guest.setDword(kGear + view() * 4, gear);
            const float paceOff = float(fp::sub(pace, gearPace()));
            if (guest.call(kCameraOnRoad, args({ view() })) != 0)
            {
                sreg32 node;
                if (off == kCarAhead)
                {
                    const sreg32 jump = jumpsBelow(fp::compare(Float(paceOff), floatFrom(guest, kJumpNear))) ? 3
                                      : jumpsBelow(fp::compare(Float(paceOff), floatFrom(guest, kJumpMiddle))) ? 5 : 7;
                    node = wrapped(sreg32(guest.dword(car + kCarNode)) + jump, sreg32(guest.dword(kSliceCount)));
                }
                else
                {
                    const sreg32 jump = jumpsAbove(fp::compare(Float(paceOff), doubleAt(guest, kJumpBackNear))) ? 3 : 5;
                    node = sreg32(guest.dword(car + kCarNode)) - jump;
                    if (node < 0)
                        node += sreg32(guest.dword(kSliceCount));
                }
                guest.setDword(kNode + view() * 4, reg32(node));
                guest.call(kCameraKeepRoad, args({ view(), kRoadKept }));
            }
        }
    }
    if (guest.call(kCameraOnRoad, args({ view() })) != 0
        && (off != 0 || roadBroken(guest, guest.dword(camera + kCameraNode), guest.dword(car + kCarNode))))
    {
        return guest.call(kCameraOffRoad, args({ view() }));
    }

    // Between this node's slice and the next's, by the steps gone.
    const reg32 nodeNow = guest.dword(kNode + view() * 4);
    const reg32 nodeNext = reg32(wrapped(sreg32(nodeNow + 1), sreg32(guest.dword(kSliceCount))));
    const reg32 from = slice(guest, nodeNow);
    const reg32 to = slice(guest, nodeNext);
    const sreg32 steps = sreg32(guest.dword(frame + kLocalSteps + guest.dword(kGear + view() * 4) * 4));
    const Float scale = doubleAt(guest, kScale);
    const Float stepsGone = integerFrom(guest, kStep + view() * 4);
    const auto along = [&](reg32 axis, bool scaleFirst) {
        const sreg32 each = sreg32(guest.dword(to + axis) - guest.dword(from + axis)) / steps;
        const Float start = scaleFirst ? fp::mul(scale, integerFrom(guest, from + axis)) : fp::mul(integerFrom(guest, from + axis), scale);
        const float eachOn = float(fp::mul(Float(each), scale));
        return float(fp::add(start, fp::mul(stepsGone, Float(eachOn))));
    };
    guest.setFloat(camera + kCameraX, along(0, false));
    guest.setFloat(camera + kCameraY, along(4, false));
    guest.setFloat(camera + kCameraZ, along(8, true));

    // How far from the car, the longer way and a quarter of the shorter.
    const reg32 carNow = guest.dword(camera + kCameraCar);
    const auto offBy = [&](reg32 carField, reg32 cameraField) {
        return float(fp::sub(floatFrom(guest, carNow + carField), floatFrom(guest, camera + cameraField)));
    };
    reg32 eax = carNow;
    // A way's size; its off stored, eax as the original leaves it after: that of the size's copy, or the comparison's.
    const auto sizeOf = [&](float offValue) {
        reg16 own;
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
    const reg16 capped = fp::compare(floatFrom(guest, kMostDrop), Float(drop));
    const bool overMost = jumpsBelow(capped);
    const reg32 dropBits = overMost ? kMostDropBits : bitsOf(drop);
    eax = overMost ? withStatus(eax, capped) : dropBits;
    const Float height = fp::sub(floatFrom(guest, kHeight), Float(floatOf(dropBits)));
    guest.setFloat(camera + kCameraY, float(fp::add(height, floatFrom(guest, camera + kCameraY))));
    return eax;
}

/* sub_4262a0: mode 19, runs the mode it was told for a while, then hands the view on. */
reg32 cameraWaitThenHandOn(Guest& guest, reg32 camera)
{
    constexpr reg32 kWaiting = kCameraOwn118, kNextMode = kCameraOwn11c;
    constexpr reg32 kTime = kCameraOwn110, kTick = kCameraOwn114;
    constexpr sreg32 kOneBits = 0x3f800000;  // compared as an integer
    if (guest.dword(camera + kWaiting) == 0)
        return guest.eax();
    const reg32 next = guest.dword(camera + kNextMode);
    if (next == 0)
        return guest.eax();
    if (sreg32(guest.dword(camera + kTime)) > kOneBits)
    {
        guest.call(kSetCameraMode, args({ guest.dword(camera + kCameraView), next }));
        return guest.call(kCameraStep, args({ guest.dword(camera + kNextMode), guest.dword(camera + kCameraView) }));
    }
    const reg32 result = guest.call(kCameraStep, args({ next, guest.dword(camera + kCameraView) }));
    guest.setFloat(camera + kTime, float(fp::add(floatFrom(guest, camera + kTick), floatFrom(guest, camera + kTime))));
    return result;
}

/* sub_426310: a view under mode 19 handed straight on to the mode it was waiting for. */
reg32 cameraBackFromWait(Guest& guest, reg32 view)
{
    constexpr reg32 kWaitCameras = 0x552f60, kWaitMode = 0x13;
    const reg32 now = guest.dword(kCameraNow + view * 4);
    if (now == 0)
        return guest.eax();
    const reg32 mode = guest.dword(now);
    if (mode != kWaitMode)
        return mode;
    const reg32 waiting = guest.dword(kWaitCameras + view * 4);
    guest.call(kSetCameraMode, args({ view, guest.dword(waiting + kCameraOwn11c) }));
    const reg32 camera = guest.dword(kCameraNow + view * 4);
    return guest.call(kCameraStep, args({ camera != 0 ? guest.dword(camera) : 0, view }));
}

/* sub_426450: a mode's camera of a view prepared (sub_426370) and stepped.  It leaves
 * edx the mode less one, as a jump table's index, for no caller's use. */
reg32 cameraStep(Guest& guest, reg32 mode, reg32 view)
{
    static constexpr reg32 kStepOfMode[24] = {
        0x4240c0, 0x424260, 0x424740, 0x4243a0, 0x424740, 0x4243a0, 0x424b50, 0x425060,
        0x425d50, 0x425990, 0x425ab0, 0x425ab0, 0x425ab0, 0x425bf0, 0,        0x4253d0,
        0x425640, 0x4251f0, 0x4262a0, 0x426290, 0x424690, 0,        0x424ce0, 0x424e30,
    };
    const reg32 camera = guest.dword(kCamerasOfModes + mode * 8 + view * 4);
    const reg32 prepared = guest.call(kPrepareCamera, args({ camera, view, mode }));
    const reg32 row = mode - 1;
    guest.leaveEdx(row);
    if (row > 0x17 || kStepOfMode[row] == 0)
        return prepared;
    return guest.call(kStepOfMode[row], args({ camera }));
}

/* sub_426560: the mode a view's car forces now: the attract's, a wreck's, a finish's,
 * one of being stopped or of having left the track; forcing none ends mode 0x18's. */
reg32 forcedMode(Guest& guest, reg32 view)
{
    constexpr reg32 kAttract = 0x7d36a0, kAttractOff = 0x559264;
    constexpr reg32 kCarWrecked = 0x9a4, kCarFinished = 0x99c, kCarStopped = 0x184, kCarLeftTrack = 0x524;
    constexpr reg32 kWreckCameras = 0x552f70;
    constexpr reg32 kModeAttract = 0x17, kModeWreck = 0x15, kModeFinished = 7, kModeStopped = 0x14, kModeOffTrack = 0x18;

    const reg32 forcedAt = kCameraForced + view * 4;
    const reg32 nowAt = kCameraNow + view * 4;
    const reg32 forced = guest.dword(forcedAt);
    const reg32 car = guest.dword((forced != 0 ? forced : guest.dword(nowAt)) + kCameraCar);
    const reg32 forcedBefore = forced != 0 ? guest.dword(forced) : 0;
    reg32 mode = 0;
    if (guest.dword(kAttract) != 0 && guest.dword(kAttractOff) == 0)
    {
        mode = kModeAttract;
        guest.call(kBackFromWait, args({ view }));
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
    else if (sreg32(guest.dword(car + kCarStopped)) > 0)
    {
        mode = kModeStopped;
    }
    else if (guest.dword(car + kCarLeftTrack) != 0 && guest.dword(kViewedCar) == guest.dword(kPlayerNow))
    {
        mode = kModeOffTrack;
        guest.call(kBackFromWait, args({ view }));
    }
    const reg32 result = guest.call(kSetForcedMode, args({ view, mode }));
    if (forcedBefore != kModeOffTrack || mode != 0)
        return result;
    const reg32 camera = guest.dword(nowAt);
    const reg32 back = camera != 0 ? guest.dword(camera) : 0;
    guest.call(kSetCameraMode, args({ view, kModeOffTrack }));
    return guest.call(kSwitchCamera, args({ back, view }));
}

/* sub_426680: every view's camera stepped; a car at the end of its race first given
 * mode 17, the end path, unless a path of its own is going. */
reg32 camerasStep(Guest& guest)
{
    constexpr reg32 kPathMode = 0x10, kEndMode = 0x11;
    constexpr reg32 kCarRaceState = 0x2c4, kRaceOver = 3;
    constexpr std::uint8_t kCarRacing = 0x01;
    constexpr reg32 kWonPath = 6, kLostPath = 1, kWonRace = 0x6fd4c0;
    constexpr reg32 kNone = reg32(-1);

    const sreg32 lastView = guest.dword(kPlayers) == 1 ? 1 : 0;
    reg32 result = 0;
    for (sreg32 index = 0; index <= lastView; ++index)
    {
        const reg32 view = reg32(index);
        const reg32 camera = guest.dword(kCameraNow + view * 4);
        const reg32 car = guest.dword(camera + kCameraCar);
        const reg32 mode = guest.dword(camera);
        const reg32 onPath = mode == kPathMode || mode == kEndMode ? guest.dword(camera + kCameraOwn11c) : 0;
        if ((guest.byte(car + kCarFlags) & kCarRacing) && guest.dword(car + kCarRaceState) == kRaceOver && onPath == 0)
        {
            guest.call(kSwitchCamera, args({ kEndMode, view }));
            const reg32 ending = guest.dword(kCameraNow + view * 4);
            guest.setDword(ending + kCameraOwn11c, 1);
            const reg32 animation = guest.dword(ending + kCameraOwn118);
            if (animation != kNone)
            {
                guest.call(kStopAnimation, args({ animation }));
                guest.setDword(ending + kCameraOwn118, kNone);
            }
            const reg32 path = guest.dword(kWonRace) != 0 ? kWonPath : kLostPath;
            guest.setDword(ending + kCameraOwn114, path);
            guest.setDword(ending + kCameraOwn118, guest.call(kStartAnimation, args({ path })));
        }
        guest.call(kForcedMode, args({ view }));
        const reg32 forced = guest.dword(kCameraForced + view * 4);
        const reg32 shown = forced != 0 ? forced : guest.dword(kCameraNow + view * 4);
        result = guest.call(kCameraStep, args({ shown != 0 ? guest.dword(shown) : 0, view }));
    }
    return result;
}

// ---------------------------------------------------------------------------
// Distances
// ---------------------------------------------------------------------------

/* sub_4965b0: the distance between two points, eax and edx, without a root: the
 * longest way and a quarter of each other's, onto the x87. */
reg32 quickDistance(Guest& guest, reg32 a, reg32 b)
{
    constexpr reg32 kQuarter = 0x53bedc;
    reg16 status;
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
    guest.pushFloat(whole);
    return withStatus(bitsOf(dz), status);
}

/* sub_4967e0: a length in the plane without a root, of two sizes on the stack: the
 * larger and a share of the smaller, the share by how near the smaller comes. */
void planarLength(Guest& guest, float a, float b)
{
    constexpr reg32 kQuarter = 0x53bee4, kHalf = 0x53bef4, kThreeQuarters = 0x53bf04;
    constexpr reg32 kShareSmall = 0x53beec, kShareLess = 0x53befc, kShareMore = 0x53bf0c, kShareLarge = 0x53bf14;
    const bool firstLarger = jumpsAbove(fp::compare(Float(a), Float(b)));
    const Float larger = Float(firstLarger ? a : b);
    const Float smaller = Float(firstLarger ? b : a);
    reg32 share = kShareLarge;
    if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kQuarter)), smaller)))
        share = kShareSmall;
    else if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kHalf)), smaller)))
        share = kShareLess;
    else if (jumpsAbove(fp::compare(fp::mul(larger, doubleAt(guest, kThreeQuarters)), smaller)))
        share = kShareMore;
    guest.pushFloat(float(fp::add(fp::mul(smaller, doubleAt(guest, share)), larger)));
}

// ---------------------------------------------------------------------------
// The cars on the track
// ---------------------------------------------------------------------------

/* sub_49fde0: whether a car leans over too far: with the race's hazard (0x6fd4a0) and a
 * car that has it (flag 4), past -0.9 or past 0.5 as integers read the float. */
reg32 carTiltedOver(Guest& guest, reg32 car)
{
    constexpr reg32 kLean = 0x10c, kHazardRace = 0x6fd4a0;
    constexpr std::uint8_t kHazardCar = 0x04;
    constexpr reg32 kMostBack = 0x53c794, kMost = 0x53c79c;
    constexpr sreg32 kHalfBits = 0x3f000000;
    if (guest.dword(kHazardRace) != 0 && (guest.byte(car + kCarFlags) & kHazardCar))
    {
        if (jumpsBelow(fp::compare(floatFrom(guest, car + kLean), doubleAt(guest, kMostBack))))
            return 1;
        return sreg32(guest.dword(car + kLean)) > kHalfBits ? 1 : 0;
    }
    reg16 status;
    const float size = absolute(guest.floatAt(car + kLean), status);
    return jumpsAbove(fp::compare(Float(size), floatFrom(guest, kMost))) ? 1 : 0;
}

/* sub_49fee0: the track's heading at a slice, as a fixed angle: atan2 of the way to the
 * next slice, cut to an integer. */
reg32 sliceHeading(Guest& guest, reg32 index)
{
    constexpr reg32 kScale = 0x53c7ac, kToFixed = 0x53c7b4;
    const reg32 next = reg32(wrapped(sreg32(index + 1), sreg32(guest.dword(kSliceCount))));
    const reg32 from = slice(guest, index);
    const reg32 to = slice(guest, next);
    const Float scale = doubleAt(guest, kScale);
    const Float x = fp::mul(Float(sreg32(guest.dword(to) - guest.dword(from))), scale);
    const Float z = fp::mul(scale, Float(sreg32(guest.dword(to + 8) - guest.dword(from + 8))));
    guest.call(kArcTangentOfFloats, args({}, { bitsOf(float(x)), bitsOf(float(z)) }));
    x86::FPU& fpu = guest.fpu();
    fpu.push(fp::mul(fpu.take(), doubleAt(guest, kToFixed)));
    guest.call(kTruncate);
    return reg32(fp::toInteger<sreg32>(fpu.take()));
}

/* sub_49ffa0: a car in the race given the track's frame where it is (sub_420230, in
 * 16.16, in the stack frame) as floats, and its heading. */
reg32 carTrackFrame(Guest& guest, reg32 car)
{
    constexpr reg32 kCarAxes = 0x124, kCarCentre = 0x148, kCarHeading = 0x154;
    constexpr reg32 kScale = 0x53c7bc;
    constexpr reg32 kLocalAxes = 0x00, kLocalCentre = 0x24;
    if (guest.byte(car + kCarInRace) == 0)
        return guest.eax();
    const reg32 frame = guest.frame();
    guest.setEsi(frame + kLocalCentre);
    guest.call(kPositionFrame, args({ car + kCarPlaceOnTrack, frame + kLocalAxes }));
    const Float scale = doubleAt(guest, kScale);
    for (reg32 i = 0; i < 3; ++i)
        guest.setFloat(car + kCarCentre + 4 * i, float(fp::mul(integerFrom(guest, frame + kLocalCentre + 4 * i), scale)));
    for (reg32 i = 0; i < 9; ++i)
    {
        const Float value = integerFrom(guest, frame + kLocalAxes + 4 * i);
        guest.setFloat(car + kCarAxes + 4 * i, float(i == 8 ? fp::mul(scale, value) : fp::mul(value, scale)));
    }
    const reg32 heading = guest.call(kSliceHeading, args({ guest.dword(car + kCarNode) }));
    guest.setDword(car + kCarHeading, heading);
    return heading;
}

/* sub_4a0140: a car moved back onto the road: its eight corners (sub_4e0430) held
 * against the road's plane at its slice and its two limits, each limit only where the
 * slice holds a car to it, by the corner past one the most. */
reg32 carOntoRoad(Guest& guest, reg32 car)
{
    constexpr reg32 kScale = 0x53c7c4;
    constexpr reg32 kSliceNormal = 0x15, kSliceAbove = 0x20, kSliceBelow = 0x1c;
    constexpr reg32 kLocalCorners = 0x00;  // 8 of 3 floats
    constexpr reg32 kLocalArm = 0x60;      // 3 floats
    constexpr reg32 kLocalNormal = 0x6c;   // 3 floats

    const reg32 frame = guest.frame();
    const reg32 corners = frame + kLocalCorners;
    const auto minus = [&](reg32 field) { return bitsOf(float(-floatFrom(guest, car + field))); };
    const auto plus = [&](reg32 field) { return guest.dword(car + field); };
    const reg32 corner[8][3] = {
        { minus(kCarHalfWidth), plus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), plus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), plus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), plus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), minus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), minus(kCarHalfHeight), plus(kCarHalfLength) },
        { plus(kCarHalfWidth), minus(kCarHalfHeight), minus(kCarHalfLength) },
        { minus(kCarHalfWidth), minus(kCarHalfHeight), minus(kCarHalfLength) },
    };
    for (reg32 i = 0; i < 8; ++i)
        for (reg32 axis = 0; axis < 3; ++axis)
            guest.setDword(corners + (i * 3 + axis) * 4, corner[i][axis]);
    const reg32 node = guest.dword(car + kCarNode);
    guest.call(kTransformPoints, args({ 8, corners, car + kCarMatrix, car + kCarX }, { corners }));

    const reg32 road = slice(guest, node);
    const Float scale = doubleAt(guest, kScale);
    const reg32 normal = frame + kLocalNormal;
    for (reg32 i = 0; i < 3; ++i)
    {
        // A signed byte of the slice, in 7.9 of 16.16.
        const sreg32 part = (sreg32(guest.dword(road + kSliceNormal + i)) >> 24) * 512;
        guest.setFloat(normal + 4 * i, float(fp::mul(Float(part), scale)));
    }
    const float centre[3] = { float(fp::mul(integerFrom(guest, road), scale)),
                              float(fp::mul(scale, integerFrom(guest, road + 4))),
                              float(fp::mul(integerFrom(guest, road + 8), scale)) };
    const float above = float(fp::mul(integerFrom(guest, road + kSliceAbove), scale));
    const float below = float(-fp::mul(scale, integerFrom(guest, road + kSliceBelow)));

    float most = 0.0f;
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 at = corners + i * 12;
        for (reg32 axis = 0; axis < 3; ++axis)
            guest.setFloat(frame + kLocalArm + 4 * axis, float(fp::sub(floatFrom(guest, at + 4 * axis), Float(centre[axis]))));
        guest.call(kDotOntoX87, args({ normal, frame + kLocalArm }));
        const Float height = guest.fpu().take();
        const float kept = float(height);
        const std::uint8_t limits = guest.byte(slice(guest, node) + kSliceLimits);
        float past = 0.0f;
        if (jumpsAbove(fp::compare(height, Float(above))) && (limits & 0x0f))
            past = float(fp::sub(Float(above), Float(kept)));
        else if (jumpsBelow(fp::compare(Float(kept), Float(below))) && ((limits >> 4) & 0x0f))
            past = float(fp::sub(Float(below), Float(kept)));
        reg16 status;
        const float pastSize = absolute(past, status);
        const float mostSize = absolute(most, status);
        if (jumpsAbove(fp::compare(Float(pastSize), Float(mostSize))))
            most = past;
    }
    const reg32 result = guest.call(kScaleVectors, args({ 1, normal }, { bitsOf(most), normal }));
    guest.setFloat(car + kCarX, float(fp::add(floatFrom(guest, normal), floatFrom(guest, car + kCarX))));
    guest.setFloat(car + kCarY, float(fp::add(floatFrom(guest, normal + 4), floatFrom(guest, car + kCarY))));
    guest.setFloat(car + kCarZ, float(fp::add(floatFrom(guest, normal + 8), floatFrom(guest, car + kCarZ))));
    return result;
}

/* A way from a car to a player, as sub_4a0580 sizes it: above 0 as it is, else its sign
 * changed -- so here 0 changes too, and a NaN does not. */
float sizeOfWay(Guest& guest, reg32 car, reg32 player, reg32 field)
{
    const float off = float(fp::sub(floatFrom(guest, car + field), floatFrom(guest, player + field)));
    return jumpsBelow(fp::compare(0.0, Float(off))) ? off : float(-Float(off));
}

/* The longer of two sizes and a quarter of the shorter. */
float planeDistance(Guest& guest, float x, float z)
{
    constexpr reg32 kQuarter = 0x53c7cc;
    return float(jumpsAbove(fp::compare(Float(x), Float(z))) ? fp::add(fp::mul(Float(z), floatFrom(guest, kQuarter)), Float(x))
                                                             : fp::add(fp::mul(Float(x), floatFrom(guest, kQuarter)), Float(z)));
}

/* sub_4a0580: how far a car is from the players of a one-machine race of at most two:
 * the larger of its distance and the nodes to go, six a node.  Far (366), it stays
 * asleep and stopped; near again, it wakes on the road. */
reg32 carNearPlayers(Guest& guest, reg32 car)
{
    constexpr reg32 kNodeScale = 0x53c7d4, kFar = 0x53c7dc;
    constexpr reg32 kLocalMost = 2;
    constexpr reg32 kCarSpinA = 0xe4, kCarSpinB = 0xec;
    constexpr reg32 kCarRest = 0x158, kCarUnder = 0x15c, kCarTiltA = 0x168, kCarTiltB = 0x16c;

    const reg32 players = guest.dword(kPlayers);
    if (players != 0)
    {
        guest.setByte(car + kCarAsleep, 0);
        guest.setDword(car + kCarDistance, 0);
        return guest.eax();
    }
    if (sreg32(guest.dword(kLocalCars)) > sreg32(kLocalMost))
    {
        guest.setByte(car + kCarAsleep, 0);
        guest.setDword(car + kCarDistance, players);
        return guest.eax();
    }
    const auto playerAt = [&] { return guest.dword(kCars + guest.dword(kPlayerNow) * 4); };
    const auto otherAt = [&] { return guest.dword(kCars + (1 - guest.dword(kPlayerNow)) * 4); };
    {
        const float awayX = sizeOfWay(guest, car, playerAt(), kCarX);
        const float awayZ = sizeOfWay(guest, car, playerAt(), kCarZ);
        guest.setFloat(car + kCarDistance, planeDistance(guest, awayX, awayZ));
    }
    sreg32 nodes = sreg32(guest.call(kNodeGapOfCars, args({ guest.dword(car + kCarNode), guest.dword(playerAt() + kCarNode) })));
    if (guest.dword(kLocalCars) == kLocalMost)
    {
        const float otherX = sizeOfWay(guest, car, otherAt(), kCarX);
        const float otherZ = sizeOfWay(guest, car, otherAt(), kCarZ);
        const float other = planeDistance(guest, otherX, otherZ);
        if (!jumpsAbove(fp::compare(Float(other), floatFrom(guest, car + kCarDistance))))
            guest.setFloat(car + kCarDistance, other);
        const sreg32 toOther = sreg32(guest.call(kNodeGapOfCars, args({ guest.dword(car + kCarNode), guest.dword(otherAt() + kCarNode) })));
        if (toOther <= nodes)
            nodes = toOther;
    }
    const Float byNodes = fp::mul(Float(nodes), doubleAt(guest, kNodeScale));
    const Float distance = floatFrom(guest, car + kCarDistance);
    const Float reach = jumpsAbove(fp::compare(distance, byNodes)) ? distance : byNodes;
    guest.setFloat(car + kCarDistance, float(reach));
    std::uint64_t reachBits;
    std::memcpy(&reachBits, &reach, sizeof reachBits);
    const reg16 status = fp::compare(reach, doubleAt(guest, kFar));
    reg32 eax = withStatus(reg32(reachBits >> 32), status);
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
        guest.call(kCarNodeNow, args({ car }));
        guest.call(kCarTrackFrame, args({ car }));
        guest.call(kCarOntoRoad, args({ car }));
        guest.call(kCarHeightOverRoad, args({ car }));
        x86::FPU& fpu = guest.fpu();
        const Float over = fpu.take();
        guest.setDword(car + kCarSurface, 1);
        guest.setDword(car + kCarTiltA, 0);
        guest.setDword(car + kCarTiltB, 0);
        guest.setFloat(car + kCarRest, float(over));
        const Float raised = fp::add(over, floatFrom(guest, car + kCarHalfHeight));
        guest.setDword(car + kCarSurfaceKind, guest.dword(car + kCarSurface));
        guest.setFloat(car + kCarY, float(raised));
        guest.call(kCarLowestPoint, args({ car }));
        const Float under = fpu.take();
        guest.setDword(car + kCarUnder, 0);
        guest.setFloat(car + kCarY, float(fp::sub(floatFrom(guest, car + kCarY), under)));
        eax = guest.call(kClearRespawn, args({ car }));
    }
    guest.setByte(car + kCarAsleep, 0);
    return eax;
}

/* sub_4a08c0: the frame's work on a car's place on the track, from the race's table of a
 * car's functions: the rain over it outside a tunnel, how near the players, its node and
 * frame; awake, its surface. */
reg32 carTrackEnvironment(Guest& guest, reg32 car)
{
    constexpr reg32 kRainy = 0x6fd4cc, kCarRain = 0x160, kCarStopped = 0x184, kCarMoving = 0x20, kCarWantsFrame = 0x76;
    if (guest.byte(car + kCarInRace) == 0)
        return guest.eax();
    const reg32 rainy = guest.dword(kRainy);
    if (rainy != 0)
    {
        guest.setDword(car + kCarRain, guest.call(kRainAt));
        const reg32 tunnel = guest.call(kInTunnel, args({}, { guest.dword(car + kCarX), guest.dword(car + kCarY),
                                                               guest.dword(car + kCarZ), guest.dword(car + kCarNode) }));
        if (tunnel != 0)
            guest.setDword(car + kCarRain, 0);
    }
    else
    {
        guest.setDword(car + kCarRain, rainy);
    }
    reg32 eax = guest.call(kCarNearPlayers, args({ car }));
    if (guest.dword(car + kCarStopped) != 0 && guest.dword(car + kCarMoving) == 0)
        return eax;
    eax = guest.call(kCarNodeNow, args({ car }));
    if (guest.word(car + kCarWantsFrame) != 0)
        eax = guest.call(kCarTrackFrame, args({ car }));
    if (guest.byte(car + kCarAsleep) == 0)
    {
        const reg32 surface = guest.call(kPositionSurface, args({ car + kCarPlaceOnTrack, 1 }));
        guest.setDword(car + kCarSurface, surface);
        eax = surface & 0x3f;
        guest.setDword(car + kCarSurfaceKind, eax);
    }
    return eax;
}

}  // namespace

// ---------------------------------------------------------------------------
// As the generated functions stood: their frames, what they restore
// ---------------------------------------------------------------------------

/* sub_40de30: an opponent's lane votes added up and acted on. */
void frontend::simLaneSteer(win32::WinApplication* app, x86::CPU& cpu)
{
    if (aiVotes(app, cpu))
        return;
    static fe::Site site{ "sub_40de30", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        guest.call(kSumLaneVotes, args({ car }));
        return guest.call(kSteerToLane, args({ car }));
    });
}

void frontend::simEnginePull(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4064f0", 0, 24 + 0x24, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return enginePull(guest, car); });
}

void frontend::simHeadingError(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4070c0", 0, 16 + 0x30, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return headingError(guest, car); });
}

void frontend::simCameraBesideRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425060", 0, 24 + 0x1c, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraBesideRoad(guest, camera); });
}

void frontend::simCameraNearestSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4251f0", 0, 24 + 0x30, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraNearestSpot(guest, camera); });
}

void frontend::simCameraPlayPath(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4253d0", 0, 24 + 0x84, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraPlayPath(guest, camera); });
}

void frontend::simCameraEndPath(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425640", 0, 24 + 0x94, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraEndPath(guest, camera); });
}

void frontend::simCameraOverRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425bf0", 0, 24 + 0x1c, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraOverRoad(guest, camera); });
}

void frontend::simCameraFlyover(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_425d50", 0, 24 + 0xb0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraFlyover(guest, camera); });
}

/* sub_426290: mode 20's step: nothing. */
void frontend::simCameraStill(win32::WinApplication*, x86::CPU& cpu)
{
    cpu.esp += 4;
}

void frontend::simCameraWaitThenHandOn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4262a0", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraWaitThenHandOn(guest, camera); });
}

void frontend::simCameraBackFromWait(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426310", 0, 20, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraBackFromWait(guest, view); });
}

void frontend::simCameraStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426450", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 mode = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cameraStep(guest, mode, view); });
}

void frontend::simCameraForcedMode(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426560", 0, 24, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return forcedMode(guest, view); });
}

void frontend::simCamerasStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_426680", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return camerasStep(guest); });
}

void frontend::simQuickDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4965b0", 0, 8 + 0x2c, nullptr, 0 };
    const reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return quickDistance(guest, a, b); });
}

void frontend::simPlanarLength(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4967e0", 8, 8 + 0x1c, nullptr, 0 };
    const float a = app->getMemory<float>(cpu.esp + 4), b = app->getMemory<float>(cpu.esp + 8);
    const reg32 eax = cpu.eax;  // it pushes eax too
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        planarLength(guest, a, b);
        return eax;
    });
}

void frontend::simCarTiltedOver(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49fde0", 0, 8 + 4, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carTiltedOver(guest, car); });
}

void frontend::simSliceHeading(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49fee0", 0, 24 + 8, nullptr, 0 };
    const reg32 index = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return sliceHeading(guest, index); });
}

void frontend::simCarTrackFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49ffa0", 0, 16 + 0x34, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carTrackFrame(guest, car); });
}

/* sub_4a00c0: how many nodes from eax to edx (sub_40e120), under another name. */
void frontend::simNodeGap(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a00c0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kNodeGap); });
}

void frontend::simCarOntoRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a0140", 0, 24 + 0xa4, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carOntoRoad(guest, car); });
}

void frontend::simCarNearPlayers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a0580", 0, 20 + 0x50, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carNearPlayers(guest, car); });
}

void frontend::simCarTrackEnvironment(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a08c0", 0, 24, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return carTrackEnvironment(guest, car); });
}

}
