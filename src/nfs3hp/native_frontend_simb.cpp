#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstdint>
#include <cstring>

/* A car on the track, the part of the simulation at 0x4a0aa0..0x4a2f40 that was
 * still generated code, decompiled:
 *
 *   placed       a car put on a node of the road, its frames and its surface
 *                worked out from the node (sub_4a0aa0);
 *   the road     the probes round its box that find walls (sub_4a1210), the place
 *                it is about to reach and what the road there does to it
 *                (sub_4a1610), what going into something does to it and the shake
 *                of a car coming to rest (sub_4a1ce0), a tunnel's roof
 *                (sub_4a21a0), the downforce and the drag (sub_4a2260);
 *   its numbers  how its axes lie against the road's up (sub_4a2690), how far
 *                across the road it is (sub_4a2bc0);
 *   its turn     a turn by three angles as a matrix (sub_4a2cc0), and the steps a
 *                car's +0x614 points at, a thirty-second and a sixty-fourth of its
 *                spin (sub_4a2f10, sub_4a2f30).
 *
 * A replay is the same to the bit only if every sum here is the x87's, in its
 * order, so nothing is rearranged: each fp:: call is an instruction's, its operands
 * in the order the generated code has them, a value that went through a float on
 * the stack goes through a float here, and a dword moved whole stays a dword, which
 * keeps a NaN's bits as a load through the x87 would not.
 *
 * The car's state stays in guest memory.  What is handed to the game by address
 * lives in the frame, where the generated function had it. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg32;
using x86::sreg32;

// The game's functions, by address.
constexpr reg32 kResetPosition = 0x41e940;     // eax a node, edx a track position
constexpr reg32 kUpdatePosition = 0x41f6e0;    // eax a point in 16.16, edx a track position, ebx 1
constexpr reg32 kPositionFrame = 0x420230;     // eax a track position, edx its frame in 16.16, esi a point in 16.16
constexpr reg32 kPositionSurface = 0x4207e0;   // eax a track position, edx 1: its surface
constexpr reg32 kInTunnel = 0x420bf0;          // the stack a point and a node
constexpr reg32 kStartIdleShake = 0x4296c0;    // eax a car
constexpr reg32 kHitWall = 0x42ee10;           // eax a car, edx a point, ebx the wall's normal, ecx a point on it
constexpr reg32 kBoxHitsWall = 0x42efe0;       // eax a car, edx a normal, ebx a point, the stack a float
constexpr reg32 kNormalise = 0x4972f0;         // eax a vector; its length on the x87
constexpr reg32 kMayRest = 0x49fde0;           // eax a car
constexpr reg32 kRoadUnder = 0x49ffa0;         // eax a car, edx whether it faces back
constexpr reg32 kRoadRise = 0x4a0440;          // eax a car; on the x87
constexpr reg32 kNodeFlags = 0x4a08c0;         // eax a car
constexpr reg32 kFrameFromRoad = 0x4a0980;     // eax a car, edx whether it faces back
constexpr reg32 kLastFrameFromRoad = 0x4a0a10; // likewise
constexpr reg32 kFollowFrame = 0x4a0f30;       // eax a car, edx the road's up, the stack how upright it is
constexpr reg32 kEaseFrame = 0x4a1090;         // eax a car
constexpr reg32 kProbeWalls = 0x4a1210;        // eax a car
constexpr reg32 kClampSpin = 0x4a20c0;         // eax a car
constexpr reg32 kSpinStep = 0x4a2e20;          // eax a car, edx the steps a second
constexpr reg32 kBlendFrames = 0x4b7680;       // eax a frame, edx another, the stack the share and the result
constexpr reg32 kXRotation = 0x4dfd80;         // eax a matrix, the stack the angle
constexpr reg32 kYRotation = 0x4dfde0;
constexpr reg32 kZRotation = 0x4dfe40;
constexpr reg32 kAddVectors = 0x4dffb0;        // eax the count, edx and ebx the sides, ecx the result
constexpr reg32 kSubtractVectors = 0x4e0000;   // likewise
constexpr reg32 kScaleVectors = 0x4e0050;      // eax the count, edx them, the stack the factor and the result
constexpr reg32 kTranspose = 0x4e0110;         // eax a matrix, edx the result
constexpr reg32 kDot = 0x4e01f0;               // eax and edx vectors; on the x87
constexpr reg32 kMultiplyMatrices = 0x4e0280;  // eax and edx the sides, ebx the result
constexpr reg32 kTransformPoints = 0x4e0430;   // eax the count, edx them, ebx a matrix, ecx a shift, the stack the result

// The game's numbers.
constexpr reg32 kNodes = 0x5dd948;             // the road's nodes, kNodeBytes each
constexpr reg32 kRoofHeights = 0x552e54;       // floats, by node
constexpr reg32 kPlayers = 0x6fd3b0;
constexpr reg32 kRaceKind = 0x6fd3b8;
constexpr reg32 kClimbLimit = 0x5efdd8;
constexpr reg32 kBounceLimit = 0x5efddc;

// Its constants: doubles,
constexpr reg32 kFromFixed = 0x53c7e4;         // 1/65536, a copy for each of these functions
constexpr reg32 kFromFixedProbe = 0x53c81c;
constexpr reg32 kFromFixedFrame = 0x53c834;
constexpr reg32 kFromFixedSettle = 0x53c8bc;
constexpr reg32 kFromFixedAcross = 0x53c8e4;
constexpr reg32 kProbeShareWide = 0x53c7fc;    // 0.75
constexpr reg32 kProbeShareHigh = 0x53c804;    // 0.5
constexpr reg32 kProbeShareLong = 0x53c80c;    // 0.8
constexpr reg32 kToFixedProbe = 0x53c814;      // 65536
constexpr reg32 kToFixedAhead = 0x53c82c;      // 65536
constexpr reg32 kTiltEdge = 0x53c844;          // 0.02
constexpr reg32 kTiltLowest = 0x53c854;        // -0.5
constexpr reg32 kBounceUpright = 0x53c85c;     // 0.9
constexpr reg32 kBounceShare = 0x53c864;       // 0.1
constexpr reg32 kBounceMost = 0x53c86c;        // 0.2
constexpr reg32 kBounceBase = 0x53c874;        // 0.5
constexpr reg32 kBoxShare = 0x53c884;          // 0.75
constexpr reg32 kSpinToKick = 0x53c894;        // 5
constexpr reg32 kDownforceUpright = 0x53c8a4;  // 0.75
constexpr reg32 kDownforceDoubled = 0x53c8ac;  // 2
// and floats.
constexpr reg32 kThirtySecond = 0x53c824;      // 1/32, a step ahead
constexpr reg32 kTiltUpright = 0x53c83c;       // 0.9
constexpr reg32 kTiltMost = 0x53c84c;          // 0.5
constexpr reg32 kHitToSound = 0x53c87c;        // 4
constexpr reg32 kRestSpin = 0x53c88c;          // 0.6
constexpr reg32 kDragThirtySecond = 0x53c8b4;  // 1/32

// A float's bits, compared as an integer where the game does.
constexpr sreg32 kDownforceSpeedBits = 0x41200000;  // 10.0f
constexpr sreg32 kKickUprightBits = 0x3f666666;     // 0.9f
constexpr reg32 kPlusOne = 0x3f800000, kMinusOne = 0xbf800000;
constexpr reg32 kPlusHalf = 0x3f000000, kMinusHalf = 0xbf000000;
constexpr reg32 kDragOfUp = 0x3bc49ba6;             // 0.006f
constexpr reg32 kDragOfRight = 0x3b83126f;          // 0.004f
constexpr reg32 kDragOfForward = 0x3b03126f;        // 0.002f
constexpr reg32 kDragOfForwardNoDownforce = 0x3b83126f;  // 0.004f

// A car.
constexpr reg32 kNode = 0x08;                  // the node it was placed on
constexpr reg32 kTrackPosition = 0x0c;         // kPositionDwords of them
constexpr reg32 kCurrentNode = 0x1c;
constexpr reg32 kDetached = 0x88;              // byte; nonzero: the road is not looked up for it
constexpr reg32 kActive = 0x89;                // byte
constexpr reg32 kPlace = 0x98;                 // its point
constexpr reg32 kPlaceY = 0x9c;
constexpr reg32 kPlaceZ = 0xa0;
constexpr reg32 kVelocity = 0xa4;
constexpr reg32 kVelocityY = 0xa8;
constexpr reg32 kVelocityZ = 0xac;
constexpr reg32 kSpeed = 0xb8;
constexpr reg32 kAcross = 0xbc;                // how far across the road
constexpr reg32 kFrame = 0xc0;                 // its axes: right, up, forward
constexpr reg32 kUp = 0xcc;
constexpr reg32 kUpZ = 0xd4;
constexpr reg32 kForward = 0xd8;
constexpr reg32 kSpinX = 0xe4;
constexpr reg32 kSpinY = 0xe8;
constexpr reg32 kSpinZ = 0xec;
constexpr reg32 kRightAgainstRoad = 0x108;     // its axes along the road's up
constexpr reg32 kUpright = 0x10c;
constexpr reg32 kForwardAgainstRoad = 0x110;
constexpr reg32 kHalfWidth = 0x114;
constexpr reg32 kHalfHeight = 0x118;
constexpr reg32 kHalfLength = 0x11c;
constexpr reg32 kRoadFrame = 0x124;            // the road's axes: right, up, forward
constexpr reg32 kRoadUp = 0x130;
constexpr reg32 kRoadForward = 0x13c;
constexpr reg32 kRoadPoint = 0x148;
constexpr reg32 kRoadHeight = 0x158;
constexpr reg32 kClearance = 0x15c;            // over the road: not above 0, it has met it
constexpr reg32 kContacts = 0x164;             // int16
constexpr reg32 kSoundLevel = 0x168;
constexpr reg32 kSoundBand = 0x16c;
constexpr reg32 kSoundKind = 0x170;
constexpr reg32 kHitPoint = 0x178;
constexpr reg32 kSurfaceWhole = 0x188;
constexpr reg32 kSurface = 0x18c;
constexpr reg32 kFlags = 0x200;                // byte
constexpr reg32 kTuning = 0x220;               // its tuning's record
constexpr reg32 kData = 0x528;                 // its handling's numbers
constexpr reg32 kKickX = 0x5c0;
constexpr reg32 kKickY = 0x5c4;
constexpr std::uint8_t kFlagsDownforceOnce = 0x04;
constexpr std::uint8_t kFlagsNoDownforce = 0x10;
constexpr reg32 kSoundKindHit = 0x10000;

constexpr reg32 kTuningBounce = 0x50;
constexpr reg32 kDataBounce = 0x2e0;
constexpr reg32 kDataDownforce = 0x24c;

// A track position.
constexpr reg32 kPositionDwords = 0x1e;
constexpr reg32 kPositionPoint = 0x04;         // the point, 16.16
constexpr reg32 kPositionOnRoad = 0x14;
constexpr reg32 kPositionPolygon = 0x18;
constexpr reg32 kPositionNormal = 0x1c;        // to three int16s
constexpr reg32 kPositionOnPolygon = 0x24;     // a point of the polygon, 16.16
constexpr reg32 kPositionShoulder = 0x6a;      // int16
constexpr reg32 kPositionOff = 0x6c;

// A node.
constexpr reg32 kNodeBytes = 36;
constexpr reg32 kNodeUp = 0x10;                // three int8s, each of 512 steps of 16.16
constexpr reg32 kNodeForward = 0x14;
constexpr reg32 kNodeRight = 0x18;

constexpr reg32 kMatrixDwords = 9;
constexpr reg32 kSurfaceMask = 0x3f;
constexpr reg32 kSurfaceProbesPass = 0x0e;     // with 0, a surface no wall is looked for on

/* What fp::compare's status word says, as sahf hands it to a jump: jb, its
 * opposite jae; jbe, its opposite ja. */
constexpr bool jb(std::uint16_t word) { return (word & 0x0100) != 0; }
constexpr bool jbe(std::uint16_t word) { return (word & 0x4100) != 0; }

/* fcom, fnstsw ax: the word, and ax left holding it, which is what a function with
 * nothing to return leaves in eax when no other instruction writes it after. */
std::uint16_t fcom(reg32& eax, Float a, Float b)
{
    const std::uint16_t word = fp::compare(a, b);
    eax = (eax & 0xffff0000u) | word;
    return word;
}

Float load(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

void store(Guest& guest, reg32 address, Float value)
{
    guest.setFloat(address, float(value));
}

/* fild dword. */
Float integerAt(const Guest& guest, reg32 address)
{
    return Float(sreg32(guest.dword(address)));
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword of a float's constant. */
Float floatConstant(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

/* fstp dword to a place on the stack, then what a mov of it finds. */
reg32 bitsOf(Float value)
{
    const float stored = float(value);
    reg32 bits;
    std::memcpy(&bits, &stored, sizeof bits);
    return bits;
}

/* fld dword of what a mov put on the stack. */
Float fromBits(reg32 bits)
{
    float stored;
    std::memcpy(&stored, &bits, sizeof stored);
    return Float(stored);
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A part of a node's vector for fild: the int8 as 16.16's 512 steps, what the
 * generated code gets from the dword it tops, shifted down and up again. */
Float nodePart(const Guest& guest, reg32 node, reg32 part)
{
    return Float(sreg32(std::int8_t(guest.byte(node + part))) * 512);
}

/* sub_4e01f0: the result off the x87, in the x87's width. */
Float dotOf(Guest& guest, reg32 a, reg32 b)
{
    guest.call(kDot, args({ a, b }));
    return guest.fpu().take();
}

/* The vector times the matrix, as Watcom expands it, a row added at a time: every
 * read before any write, so `out` may be the vector or the matrix. */
void vectorByMatrix(Guest& guest, reg32 vector, reg32 matrix, reg32 out)
{
    const Float x = load(guest, vector), y = load(guest, vector + 4), z = load(guest, vector + 8);
    Float resultX = fp::mul(x, load(guest, matrix));
    Float resultY = fp::mul(x, load(guest, matrix + 4));
    Float resultZ = fp::mul(x, load(guest, matrix + 8));
    resultX = fp::add(resultX, fp::mul(y, load(guest, matrix + 0xc)));
    resultY = fp::add(resultY, fp::mul(y, load(guest, matrix + 0x10)));
    resultZ = fp::add(resultZ, fp::mul(y, load(guest, matrix + 0x14)));
    resultX = fp::add(resultX, fp::mul(z, load(guest, matrix + 0x18)));
    resultY = fp::add(resultY, fp::mul(z, load(guest, matrix + 0x1c)));
    resultZ = fp::add(resultZ, fp::mul(z, load(guest, matrix + 0x20)));
    store(guest, out, resultX);
    store(guest, out + 4, resultY);
    store(guest, out + 8, resultZ);
}

/* A magnitude, the two ways Watcom wrote it.  `fldz, fcomp, jbe`: what is not below
 * zero (a NaN, -0) moved as it is, the rest negated through the x87. */
reg32 magnitudeBelowNegated(Float value, reg32 bits)
{
    return jbe(fp::compare(0.0, value)) ? bits : bitsOf(-value);
}

/* `fldz, fcomp, jae`: what is above zero (and a NaN) moved as it is, the rest
 * negated, so that 0 comes out as -0. */
reg32 magnitudeNotAboveNegated(Float value, reg32 bits)
{
    return jb(fp::compare(0.0, value)) ? bits : bitsOf(-value);
}

/* sub_4a0aa0: the car put on a node of the road: eax the car, edx the node, ebx its
 * offset from the node in 16.16, in the road's frame, ecx whether it faces back.
 * Its track position reset, the road's frame taken from the node, its point from the
 * offset, then what is under it. */
reg32 placeOnRoad(Guest& guest, reg32 car, reg32 node, reg32 offset, reg32 facesBack)
{
    constexpr reg32 kInverse = 0x00, kInverseUp = 0x0c, kInverseForward = 0x18, kOffset = 0x24;
    const reg32 frame = guest.frame();
    const reg32 position = car + kTrackPosition;
    // In ebx all through, and left there.
    guest.leaveEbx(position);
    guest.call(kResetPosition, args({ node, position }));

    const reg32 nodeOffset = node * kNodeBytes;
    const reg32 row = guest.dword(kNodes) + nodeOffset;
    guest.setDword(car + kNode, node);
    const Float fromFixed = doubleAt(guest, kFromFixed);
    store(guest, car + kRoadFrame, fp::mul(nodePart(guest, row, kNodeRight), fromFixed));
    store(guest, car + kRoadFrame + 4, fp::mul(nodePart(guest, row, kNodeRight + 1), fromFixed));
    store(guest, car + kRoadFrame + 8, fp::mul(nodePart(guest, row, kNodeRight + 2), fromFixed));
    store(guest, car + kRoadUp, fp::mul(nodePart(guest, row, kNodeUp), fromFixed));
    store(guest, car + kRoadUp + 4, fp::mul(nodePart(guest, row, kNodeUp + 1), fromFixed));
    store(guest, car + kRoadUp + 8, fp::mul(nodePart(guest, row, kNodeUp + 2), fromFixed));
    store(guest, car + kRoadForward, fp::mul(nodePart(guest, row, kNodeForward), fromFixed));
    store(guest, car + kRoadForward + 4, fp::mul(fromFixed, nodePart(guest, row, kNodeForward + 1)));
    store(guest, car + kRoadForward + 8, fp::mul(nodePart(guest, row, kNodeForward + 2), fromFixed));
    for (const reg32 axis : { kRoadFrame, kRoadUp, kRoadForward })
    {
        guest.call(kNormalise, args({ car + axis }));
        guest.fpu().take();
    }
    guest.call(kTranspose, args({ car + kRoadFrame, frame + kInverse }));

    store(guest, frame + kOffset, fp::mul(integerAt(guest, offset), fromFixed));
    store(guest, frame + kOffset + 4, fp::mul(integerAt(guest, offset + 4), fromFixed));
    store(guest, frame + kOffset + 8, fp::mul(integerAt(guest, offset + 8), fromFixed));
    guest.setDword(car + kAcross, guest.dword(frame + kOffset));
    const float shiftX = float(dotOf(guest, frame + kOffset, frame + kInverse));
    const float shiftY = float(dotOf(guest, frame + kOffset, frame + kInverseUp));
    const float shiftZ = float(dotOf(guest, frame + kOffset, frame + kInverseForward));
    const reg32 nodePoint = guest.dword(kNodes) + nodeOffset;
    store(guest, car + kPlace, fp::add(fp::mul(integerAt(guest, nodePoint), fromFixed), Float(shiftX)));
    store(guest, car + kPlaceY, fp::add(fp::mul(integerAt(guest, nodePoint + 4), fromFixed), Float(shiftY)));
    store(guest, car + kPlaceZ, fp::add(fp::mul(integerAt(guest, nodePoint + 8), fromFixed), Float(shiftZ)));

    guest.call(kNodeFlags, args({ car }));
    guest.call(kRoadUnder, args({ car, facesBack }));
    guest.call(kFrameFromRoad, args({ car, facesBack }));
    guest.call(kLastFrameFromRoad, args({ car, facesBack }));

    store(guest, car + kRoadHeight, fp::mul(integerAt(guest, guest.dword(kNodes) + nodeOffset + 4), fromFixed));
    guest.call(kRoadRise, args({ car }));
    const Float rise = guest.fpu().take();
    guest.setDword(car + kClearance, 0);
    store(guest, car + kPlaceY, fp::sub(load(guest, car + kPlaceY), rise));

    const reg32 surface = guest.byte(car + kDetached) == 0 ? guest.call(kPositionSurface, args({ position, 1 })) : 1;
    guest.setDword(car + kSurfaceWhole, surface);
    guest.setDword(car + kSurface, surface & kSurfaceMask);
    return surface & kSurfaceMask;
}

/* sub_4a1210: eight points round the car's box looked up on the track; each on a
 * polygon with a surface, and on the far side of the polygon by its normal, is a hit
 * for sub_42ee10 to make of it.  1 when one of them hit. */
reg32 probeWalls(Guest& guest, reg32 car)
{
    constexpr reg32 kPosition = 0x00, kPoints = 0x78, kPolygonPoint = 0xd8, kFixed = 0xe4, kToPoint = 0xf0, kNormal = 0xfc;
    constexpr reg32 kPointBytes = 12;
    const reg32 frame = guest.frame();
    const auto point = [&](reg32 i) { return frame + kPoints + kPointBytes * i; };

    const reg32 widthBits = guest.dword(car + kHalfWidth);
    const reg32 heightBits = guest.dword(car + kHalfHeight);
    const reg32 lengthBits = guest.dword(car + kHalfLength);
    const Float width = load(guest, car + kHalfWidth);
    const Float height = load(guest, car + kHalfHeight);
    const Float length = load(guest, car + kHalfLength);
    const Float shareWide = doubleAt(guest, kProbeShareWide);
    const Float shareHigh = doubleAt(guest, kProbeShareHigh);
    const Float shareLong = doubleAt(guest, kProbeShareLong);
    // The four low corners, then four round its middle.
    store(guest, point(0), -width);
    store(guest, point(0) + 4, -height);
    guest.setDword(point(0) + 8, lengthBits);
    guest.setDword(point(1), widthBits);
    store(guest, point(1) + 4, -height);
    guest.setDword(point(1) + 8, lengthBits);
    guest.setDword(point(2), widthBits);
    store(guest, point(2) + 4, -height);
    store(guest, point(2) + 8, -length);
    store(guest, point(3), -width);
    store(guest, point(3) + 4, -height);
    store(guest, point(3) + 8, -length);
    store(guest, point(4), fp::mul(-width, shareWide));
    store(guest, point(4) + 4, fp::mul(height, shareHigh));
    store(guest, point(4) + 8, fp::mul(length, shareLong));
    store(guest, point(5), fp::mul(shareWide, width));
    store(guest, point(5) + 4, fp::mul(shareHigh, height));
    store(guest, point(5) + 8, fp::mul(length, shareLong));
    guest.setDword(point(6), widthBits);
    guest.setDword(point(6) + 4, heightBits);
    store(guest, point(6) + 8, fp::mul(-length, shareLong));
    store(guest, point(7), -width);
    guest.setDword(point(7) + 4, heightBits);
    store(guest, point(7) + 8, fp::mul(shareLong, -length));

    const Float toFixed = doubleAt(guest, kToFixedProbe);
    const Float fromFixed = doubleAt(guest, kFromFixedProbe);
    const reg32 position = frame + kPosition;
    const reg32 fixed = frame + kFixed;
    const reg32 normal = frame + kNormal;
    const reg32 polygonPoint = frame + kPolygonPoint;
    const reg32 toPoint = frame + kToPoint;
    reg32 hit = 0;
    for (reg32 i = 0; i < 8; ++i)
    {
        copyDwords(guest, position, car + kTrackPosition, kPositionDwords);
        const reg32 at = point(i);
        guest.call(kTransformPoints, args({ 1, at, car + kFrame, car + kPlace }, { at }));
        for (reg32 axis = 0; axis < 3; ++axis)
            guest.setDword(fixed + 4 * axis, reg32(fp::toInteger<sreg32>(fp::mul(load(guest, at + 4 * axis), toFixed))));
        copyDwords(guest, position + kPositionPoint, fixed, 3);
        guest.call(kUpdatePosition, args({ fixed, position, 1 }));
        const reg32 surface = guest.call(kPositionSurface, args({ position, 1 })) & kSurfaceMask;
        if (guest.dword(position + kPositionOnRoad) == 0 || guest.dword(position + kPositionPolygon) == 0)
            continue;
        if (guest.dword(position + kPositionOff) != 0 || surface == 0 || surface == kSurfaceProbesPass)
            continue;

        // The polygon's normal: each int16 doubled, in 16.16.
        const reg32 polygonNormal = guest.dword(position + kPositionNormal);
        for (reg32 axis = 0; axis < 3; ++axis)
        {
            const sreg32 part = sreg32(std::int16_t(guest.word(polygonNormal + 2 * axis))) * 2;
            store(guest, normal + 4 * axis, fp::mul(Float(part), fromFixed));
        }
        // From a point of the polygon to the probe, against that normal.
        const Float onPolygonX = fp::mul(integerAt(guest, position + kPositionOnPolygon), fromFixed);
        const Float onPolygonY = fp::mul(integerAt(guest, position + kPositionOnPolygon + 4), fromFixed);
        const Float onPolygonZ = fp::mul(fromFixed, integerAt(guest, position + kPositionOnPolygon + 8));
        const Float pointX = load(guest, at), pointY = load(guest, at + 4), pointZ = load(guest, at + 8);
        store(guest, polygonPoint, onPolygonX);
        store(guest, toPoint, fp::sub(pointX, load(guest, polygonPoint)));
        store(guest, polygonPoint + 4, onPolygonY);
        store(guest, polygonPoint + 8, onPolygonZ);
        store(guest, toPoint + 4, fp::sub(pointY, load(guest, polygonPoint + 4)));
        store(guest, toPoint + 8, fp::sub(pointZ, load(guest, polygonPoint + 8)));
        if (jbe(fp::compare(0.0, dotOf(guest, toPoint, normal))))
            continue;
        if (guest.call(kHitWall, args({ car, at, normal, polygonPoint })) != 0)
            hit = 1;
    }
    return hit;
}

/* sub_4a1610 (ApplyPredictedTrackContactCorrection in the nfs3-re names): the point
 * a thirty-second of its speed ahead of the bottom of the car's box, on the side it
 * is going to, looked up on the track.  On a road that rises sharply against the up
 * it had, it hits that as a wall; on any other, the road's frame goes half way to the
 * one there.  A car that has met the road is put back on it, and its speed down
 * through it, past a limit, stopped.  Then its spin, to bring its up to `wanted`, the
 * three floats it is called with, or to the road's when it has met it: nothing when
 * upright and nearly there.  Its stack arguments are written over, as the game does. */
reg32 predictContact(Guest& guest, reg32 car, reg32 wanted)
{
    constexpr reg32 kPosition = 0x00, kInverse = 0x78, kFrameAhead = 0x9c, kFrameAheadUp = 0xa8, kRoadCopy = 0xc0,
                    kRoadCopyUp = 0xcc, kFixedFrame = 0xe4, kScaledRight = 0x108, kUpAhead = 0x114, kOnRoad = 0x120,
                    kDirection = 0x12c, kFixedOther = 0x138, kPutBack = 0x144, kAhead = 0x150, kBoxOffset = 0x15c,
                    kFixed = 0x168, kScaledForward = 0x174;
    const reg32 frame = guest.frame();
    const reg32 roadCopy = frame + kRoadCopy;
    copyDwords(guest, roadCopy, car + kRoadFrame, kMatrixDwords);
    copyDwords(guest, frame + kDirection, car + kVelocity, 3);
    guest.call(kNormalise, args({ frame + kDirection }));
    guest.fpu().take();

    // The bottom of the box, as far out as the direction of its speed lies.
    guest.call(kScaleVectors, args({ 1, car + kFrame }, { guest.dword(car + kHalfWidth), frame + kScaledRight }));
    guest.call(kScaleVectors, args({ 1, car + kForward }, { guest.dword(car + kHalfLength), frame + kScaledForward }));
    const reg32 boxOffset = frame + kBoxOffset;
    store(guest, boxOffset, dotOf(guest, frame + kDirection, frame + kScaledRight));
    store(guest, boxOffset + 4, -load(guest, car + kHalfHeight));
    store(guest, boxOffset + 8, dotOf(guest, frame + kDirection, frame + kScaledForward));
    vectorByMatrix(guest, boxOffset, car + kFrame, boxOffset);

    // That point, a thirty-second of its speed on.
    const Float step = floatConstant(guest, kThirtySecond);
    const reg32 ahead = frame + kAhead;
    const Float aheadX = fp::add(load(guest, car + kPlace), load(guest, boxOffset));
    store(guest, ahead, fp::add(aheadX, fp::mul(load(guest, car + kVelocity), step)));
    const Float aheadY = fp::add(load(guest, car + kPlaceY), load(guest, boxOffset + 4));
    store(guest, ahead + 4, fp::add(aheadY, fp::mul(load(guest, car + kVelocityY), step)));
    const Float aheadZ = fp::add(load(guest, car + kPlaceZ), load(guest, boxOffset + 8));
    store(guest, ahead + 8, fp::add(fp::mul(step, load(guest, car + kVelocityZ)), aheadZ));

    const reg32 position = frame + kPosition;
    const reg32 fixed = frame + kFixed;
    copyDwords(guest, position, car + kTrackPosition, kPositionDwords);
    const Float toFixed = doubleAt(guest, kToFixedAhead);
    for (reg32 axis = 0; axis < 3; ++axis)
        guest.setDword(fixed + 4 * axis, reg32(fp::toInteger<sreg32>(fp::mul(load(guest, ahead + 4 * axis), toFixed))));
    copyDwords(guest, position + kPositionPoint, fixed, 3);
    guest.call(kUpdatePosition, args({ fixed, position, 1 }));
    const reg32 surface = guest.call(kPositionSurface, args({ position, 1 })) & kSurfaceMask;

    if (guest.dword(position + kPositionOnRoad) != 0 && guest.dword(position + kPositionPolygon) != 0
        && guest.word(position + kPositionShoulder) != 0 && surface != 0 && surface != kSurfaceProbesPass
        && guest.dword(position + kPositionOff) == 0)
    {
        // The road's frame there.
        guest.setEsi(frame + kFixedOther);
        guest.call(kPositionFrame, args({ position, frame + kFixedFrame }));
        const Float fromFixed = doubleAt(guest, kFromFixedFrame);
        for (reg32 i = 0; i < kMatrixDwords; ++i)
            store(guest, frame + kFrameAhead + 4 * i, fp::mul(integerAt(guest, frame + kFixedFrame + 4 * i), fromFixed));
        const reg32 upAhead = frame + kUpAhead;
        copyDwords(guest, upAhead, frame + kFrameAheadUp, 3);

        // How unlike the up it has, and whether it is going into it.
        const reg32 differenceBits = bitsOf(fp::sub(load(guest, wanted + 4), load(guest, upAhead + 4)));
        const float closeness = float(fp::sub(1.0, fromBits(magnitudeBelowNegated(fromBits(differenceBits), differenceBits))));
        const float into = float(dotOf(guest, car + kVelocity, upAhead));
        if (jb(fp::compare(Float(closeness), load(guest, kClimbLimit))) && !jbe(fp::compare(0.0, Float(into)))
            && !jbe(fp::compare(load(guest, wanted + 4), load(guest, upAhead + 4))))
        {
            guest.setDword(upAhead + 4, 0);
            guest.call(kNormalise, args({ upAhead }));
            guest.fpu().take();
            const reg32 reach = bitsOf(fp::mul(load(guest, car + kSpeed), floatConstant(guest, kThirtySecond)));
            guest.call(kBoxHitsWall, args({ car, upAhead, ahead }, { reach }));
        }
        else
        {
            guest.call(kBlendFrames, args({ car + kRoadFrame, frame + kFrameAhead }, { kPlusHalf, roadCopy }));
        }
    }

    if (!jb(fp::compare(0.0, load(guest, car + kClearance))))
    {
        const reg32 putBack = frame + kPutBack;
        copyDwords(guest, wanted, frame + kRoadCopyUp, 3);
        guest.call(kScaleVectors, args({ 1, wanted }, { bitsOf(-load(guest, car + kClearance)), putBack }));
        guest.call(kAddVectors, args({ 1, car + kPlace, putBack, car + kPlace }));
        guest.setDword(car + kClearance, 0);
        guest.call(kTranspose, args({ roadCopy, frame + kInverse }));
        const reg32 onRoad = frame + kOnRoad;
        vectorByMatrix(guest, car + kVelocity, frame + kInverse, onRoad);
        if (jb(fp::compare(load(guest, onRoad + 4), load(guest, kBounceLimit))))
            guest.setDword(onRoad + 4, 0);
        vectorByMatrix(guest, onRoad, roadCopy, car + kVelocity);
    }

    // Its up, or upside down its down, against the one wanted.
    const Float sign = fromBits(jb(fp::compare(0.0, load(guest, car + kUpright))) ? kPlusOne : kMinusOne);
    const reg32 aboutZBits = bitsOf(fp::sub(fp::mul(sign, load(guest, car + kUp)), load(guest, wanted)));
    const reg32 aboutXBits = bitsOf(fp::sub(load(guest, wanted + 8), fp::mul(sign, load(guest, car + kUpZ))));
    const Float aboutZ = fromBits(aboutZBits);
    const Float aboutX = fromBits(aboutXBits);

    const Float upright = fromBits(magnitudeBelowNegated(load(guest, car + kUpright), guest.dword(car + kUpright)));
    if (!jbe(fp::compare(upright, floatConstant(guest, kTiltUpright))))
    {
        const Float edge = doubleAt(guest, kTiltEdge);
        const bool tilted = !jbe(fp::compare(fromBits(magnitudeNotAboveNegated(aboutZ, aboutZBits)), edge))
                            || !jbe(fp::compare(fromBits(magnitudeNotAboveNegated(aboutX, aboutXBits)), edge));
        if (!tilted)
        {
            guest.setDword(car + kSpinX, 0);
            guest.setDword(car + kSpinZ, 0);
            return car;
        }
    }
    const Float most = floatConstant(guest, kTiltMost);
    const Float lowest = doubleAt(guest, kTiltLowest);
    const auto clamped = [&](Float value, reg32 bits) -> reg32 {
        if (jb(fp::compare(0.0, value)))
            return jbe(fp::compare(value, most)) ? bits : kPlusHalf;
        return jbe(fp::compare(value, lowest)) ? kMinusHalf : bits;
    };
    const reg32 spinZ = clamped(aboutZ, aboutZBits);
    const reg32 spinX = clamped(aboutX, aboutXBits);
    guest.setDword(car + kSpinZ, spinZ);
    guest.setDword(car + kSpinX, spinX);
    return spinX;
}

/* sub_4a1ce0: the car and what it is going into, edx the normal.  Going into it with
 * one of the probes (sub_4a1210) hitting a wall, its clearance is gone, and upright:
 * its speed up the road's up and its spin are cut (less for what its handling and, a
 * lone player's race, its tuning ask), and the hit's sound and place set.  Then a car
 * no longer going into it: hardly turning and allowed to rest (sub_49fde0), on the
 * road, it starts the idle shake (sub_4296c0); else, upright, a kick from its spin. */
reg32 reactToSurface(Guest& guest, reg32 car, reg32 normal)
{
    constexpr reg32 kInverse = 0x00, kOnRoad = 0x24, kScaledForward = 0x30, kScaledUp = 0x3c;
    const reg32 frame = guest.frame();
    const float into = float(dotOf(guest, car + kVelocity, normal));
    if (!jbe(fp::compare(0.0, Float(into))) && guest.call(kProbeWalls, args({ car })) != 0)
    {
        const Float upright = load(guest, car + kUpright);
        guest.setDword(car + kClearance, 0);
        if (!jbe(fp::compare(upright, doubleAt(guest, kBounceUpright))))
        {
            const reg32 data = guest.dword(car + kData);
            if (data != 0)
            {
                // The share of its speed that stays.
                const Float share = doubleAt(guest, kBounceShare);
                const reg32 race = guest.dword(kRaceKind);
                const float tuned = sreg32(guest.dword(kPlayers)) <= 1 && race != 1 && race != 2
                    ? float(fp::mul(fp::mul(load(guest, guest.dword(car + kTuning) + kTuningBounce), share),
                                    load(guest, data + kDataBounce)))
                    : float(fp::mul(load(guest, data + kDataBounce), share));
                const Float wide = Float(tuned);
                const Float amount = !jbe(fp::compare(0.0, wide)) ? 0.0
                    : !jbe(fp::compare(wide, doubleAt(guest, kBounceMost))) ? 0.2 : wide;
                const reg32 stays = bitsOf(fp::add(amount, doubleAt(guest, kBounceBase)));

                const reg32 onRoad = frame + kOnRoad;
                guest.call(kTranspose, args({ car + kRoadFrame, frame + kInverse }));
                vectorByMatrix(guest, car + kVelocity, frame + kInverse, onRoad);
                store(guest, onRoad + 4, fp::mul(load(guest, onRoad + 4), fromBits(stays)));
                vectorByMatrix(guest, onRoad, car + kRoadFrame, car + kVelocity);
                guest.call(kScaleVectors, args({ 1, car + kSpinX }, { stays, car + kSpinX }));
            }

            const reg32 hit = magnitudeBelowNegated(Float(into), bitsOf(Float(into)));
            const Float level = fp::mul(fromBits(hit), floatConstant(guest, kHitToSound));
            guest.setDword(car + kSoundBand, 0);
            guest.setDword(car + kSoundKind, guest.dword(car + kSurface) | kSoundKindHit);
            store(guest, car + kSoundLevel, level);
            // The hit's place: three quarters to the back and to the bottom of its box.
            const Float boxShare = doubleAt(guest, kBoxShare);
            const reg32 byHeight = bitsOf(fp::mul(load(guest, car + kHalfHeight), boxShare));
            guest.call(kScaleVectors, args({ 1, car + kUp }, { byHeight, frame + kScaledUp }));
            const reg32 byLength = bitsOf(fp::mul(load(guest, car + kHalfLength), boxShare));
            guest.call(kScaleVectors, args({ 1, car + kForward }, { byLength, frame + kScaledForward }));
            guest.call(kSubtractVectors, args({ 1, car + kPlace, frame + kScaledForward, car + kHitPoint }));
            guest.call(kSubtractVectors, args({ 1, car + kHitPoint, frame + kScaledUp, car + kHitPoint }));
        }
    }

    const Float resting = dotOf(guest, car + kVelocity, normal);
    reg32 eax = guest.eax();
    if (!jb(fcom(eax, resting, load(guest, kBounceLimit))))
        return eax;

    const Float restSpin = floatConstant(guest, kRestSpin);
    const auto spinIsSmall = [&](reg32 axis) {
        const Float value = load(guest, car + axis);
        reg32 magnitude;
        if (jbe(fcom(eax, 0.0, value)))
        {
            eax = guest.dword(car + axis);
            magnitude = eax;
        }
        else
        {
            magnitude = bitsOf(-value);
        }
        return jb(fcom(eax, fromBits(magnitude), restSpin));
    };
    if (spinIsSmall(kSpinX) && spinIsSmall(kSpinY) && spinIsSmall(kSpinZ))
    {
        eax = guest.call(kMayRest, args({ car }));
        if (eax != 0 && !jb(fcom(eax, 0.0, load(guest, car + kClearance))))
        {
            guest.setWord(car + kContacts, 0);
            return guest.call(kStartIdleShake, args({ car }));
        }
    }
    if (sreg32(guest.dword(car + kUpright)) > kKickUprightBits)
    {
        const Float toKick = doubleAt(guest, kSpinToKick);
        const Float spinY = load(guest, car + kSpinY);
        store(guest, car + kKickY, fp::mul(spinY, toKick));
        store(guest, car + kKickX, fp::mul(toKick, spinY));
    }
    return eax;
}

/* sub_4a21a0: a car in a tunnel (sub_420bf0) that has reached its node's roof: its
 * box against the road's point, moved up by that height, facing down, and the spin
 * that gives it clamped (sub_4a20c0). */
reg32 hitTunnelRoof(Guest& guest, reg32 car)
{
    constexpr reg32 kPoint = 0x00, kDown = 0x0c;
    const reg32 frame = guest.frame();
    const reg32 node = guest.dword(car + kCurrentNode);
    reg32 eax = guest.call(kInTunnel, args({}, { guest.dword(car + kPlace), guest.dword(car + kPlaceY),
                                                 guest.dword(car + kPlaceZ), node }));
    if (eax == 0)
        return eax;
    eax = guest.dword(guest.dword(kRoofHeights) + node * 4);
    const Float roof = fromBits(eax);
    const reg32 down = frame + kDown;
    for (reg32 axis = 0; axis < 3; ++axis)
        guest.setDword(down + 4 * axis, guest.dword(car + kRoadUp + 4 * axis) ^ 0x80000000u);
    if (!jb(fcom(eax, fp::add(load(guest, car + kRoadHeight), roof), load(guest, car + kPlaceY))))
        return eax;
    const reg32 point = frame + kPoint;
    copyDwords(guest, point, car + kRoadPoint, 3);
    store(guest, point + 4, fp::add(load(guest, point + 4), roof));
    guest.call(kBoxHitsWall, args({ car, down, point }, { 0 }));
    return guest.call(kClampSpin, args({ car }));
}

/* sub_4a2260: the downforce and the drag.  Upright and at a speed, going forward, a
 * car is pressed down its own up by its handling's number and its speed forward,
 * doubled unless its flags say.  With its wheels on something, it then loses speed in
 * the world's x and z, by the square of each, the more the more its axes lie along
 * that one. */
reg32 downforceAndDrag(Guest& guest, reg32 car)
{
    constexpr reg32 kInverse = 0x00, kOnCar = 0x24;
    const reg32 frame = guest.frame();
    reg32 eax = car;
    const reg32 data = guest.dword(car + kData);
    if (data == 0)
        return eax;

    if (!(guest.byte(car + kFlags) & kFlagsNoDownforce)
        && !jbe(fcom(eax, load(guest, car + kUpright), doubleAt(guest, kDownforceUpright)))
        && sreg32(guest.dword(car + kSpeed)) > kDownforceSpeedBits)
    {
        reg32 downforce = guest.dword(data + kDataDownforce);
        const float forward = float(dotOf(guest, car + kVelocity, car + kForward));
        if (!(guest.byte(car + kFlags) & kFlagsDownforceOnce))
            downforce = bitsOf(fp::mul(fromBits(downforce), doubleAt(guest, kDownforceDoubled)));
        const reg32 onCar = frame + kOnCar;
        guest.call(kTranspose, args({ car + kFrame, frame + kInverse }));
        vectorByMatrix(guest, car + kVelocity, frame + kInverse, onCar);
        if (jb(fp::compare(0.0, Float(forward))))
            store(guest, onCar + 4, fp::sub(load(guest, onCar + 4), fp::mul(fromBits(downforce), Float(forward))));
        vectorByMatrix(guest, onCar, car + kFrame, car + kVelocity);
        eax = onCar + 0xc;
    }

    if (guest.word(car + kContacts) == 0)
        return eax;

    const Float velocityX = load(guest, car + kVelocity);
    const Float velocityZ = load(guest, car + kVelocityZ);
    const Float ofUp = fromBits(kDragOfUp);
    const Float ofRight = fromBits(kDragOfRight);
    const Float ofForward = fromBits(guest.byte(car + kFlags) & kFlagsNoDownforce ? kDragOfForwardNoDownforce : kDragOfForward);
    const auto part = [&](reg32 offset) {
        return fromBits(magnitudeBelowNegated(load(guest, car + offset), guest.dword(car + offset)));
    };
    const Float rightX = part(kFrame), rightZ = part(kFrame + 8);
    const Float upX = part(kUp), upZ = part(kUp + 8);
    const Float forwardX = part(kForward);
    const reg32 forwardZBits = magnitudeBelowNegated(load(guest, car + kForward + 8), guest.dword(car + kForward + 8));
    const Float forwardZ = fromBits(forwardZBits);
    const Float dragX = fp::add(fp::add(fp::mul(ofUp, upX), fp::mul(ofRight, rightX)), fp::mul(ofForward, forwardX));
    const Float dragZ = fp::add(fp::add(fp::mul(ofUp, upZ), fp::mul(ofRight, rightZ)), fp::mul(ofForward, forwardZ));
    const Float scaledX = fp::mul(dragX, fp::mul(velocityX, velocityX));
    const Float scaledZ = fp::mul(dragZ, fp::mul(velocityZ, velocityZ));
    const Float step = floatConstant(guest, kDragThirtySecond);
    const reg32 lossX = bitsOf(fp::mul(scaledX, step));
    const reg32 lossZ = bitsOf(fp::mul(step, scaledZ));

    // Against the speed, what eax has left of it as the code goes.
    eax = forwardZBits;
    reg32 changeX = lossX;
    if (jb(fcom(eax, 0.0, load(guest, car + kVelocity))))
        changeX = bitsOf(-fromBits(lossX));
    else
        eax = lossX;
    store(guest, car + kVelocity, fp::add(load(guest, car + kVelocity), fromBits(changeX)));
    reg32 changeZ = lossZ;
    if (jb(fcom(eax, 0.0, load(guest, car + kVelocityZ))))
        changeZ = bitsOf(-fromBits(lossZ));
    else
        eax = lossZ;
    store(guest, car + kVelocityZ, fp::add(load(guest, car + kVelocityZ), fromBits(changeZ)));
    return eax;
}

/* sub_4a2690: an active car's numbers as it stands on the road: its axes along the
 * road's up, its frame following (sub_4a0f30, sub_4a1090), and how far across the road
 * it is.  The vector that takes for the road's right is all of one byte of the node,
 * its first, where sub_4a0aa0 takes three: so the game has it. */
reg32 settle(Guest& guest, reg32 car)
{
    constexpr reg32 kRoadUpCopy = 0x00, kFromNode = 0x0c, kNodePoint = 0x18, kNodeSide = 0x24;
    if (guest.byte(car + kActive) == 0 || guest.byte(car + kDetached) != 0)
        return car;
    const reg32 frame = guest.frame();
    const reg32 up = frame + kRoadUpCopy;
    copyDwords(guest, up, car + kRoadUp, 3);
    store(guest, car + kRightAgainstRoad, dotOf(guest, car + kFrame, up));
    store(guest, car + kUpright, dotOf(guest, car + kUp, up));
    store(guest, car + kForwardAgainstRoad, dotOf(guest, car + kForward, up));
    guest.call(kFollowFrame, args({ car, up }, { guest.dword(car + kUpright) }));
    guest.call(kEaseFrame, args({ car }));

    const Float fromFixed = doubleAt(guest, kFromFixedSettle);
    const reg32 point = guest.dword(kNodes) + guest.dword(car + kCurrentNode) * kNodeBytes;
    store(guest, frame + kNodePoint, fp::mul(integerAt(guest, point), fromFixed));
    store(guest, frame + kNodePoint + 4, fp::mul(integerAt(guest, point + 4), fromFixed));
    store(guest, frame + kNodePoint + 8, fp::mul(integerAt(guest, point + 8), fromFixed));
    guest.call(kSubtractVectors, args({ 1, car + kPlace, frame + kNodePoint, frame + kFromNode }));

    const Float side = nodePart(guest, guest.dword(kNodes) + guest.dword(car + kCurrentNode) * kNodeBytes, kNodeRight);
    store(guest, frame + kNodeSide, fp::mul(side, fromFixed));
    store(guest, frame + kNodeSide + 4, fp::mul(side, fromFixed));
    store(guest, frame + kNodeSide + 8, fp::mul(fromFixed, side));
    store(guest, car + kAcross, dotOf(guest, frame + kFromNode, frame + kNodeSide));
    return car;
}

/* sub_4a2bc0: how far across the road the car is at its node: its point from the
 * node's, along the road's right.  The answer stays on the x87. */
reg32 offsetAcross(Guest& guest, reg32 car)
{
    constexpr reg32 kFromNode = 0x00, kNodePoint = 0x18;
    const reg32 frame = guest.frame();
    const reg32 point = guest.dword(kNodes) + guest.dword(car + kCurrentNode) * kNodeBytes;
    const Float fromFixed = doubleAt(guest, kFromFixedAcross);
    store(guest, frame + kNodePoint, fp::mul(integerAt(guest, point), fromFixed));
    store(guest, frame + kNodePoint + 4, fp::mul(integerAt(guest, point + 4), fromFixed));
    store(guest, frame + kNodePoint + 8, fp::mul(fromFixed, integerAt(guest, point + 8)));
    guest.call(kSubtractVectors, args({ 1, car + kPlace, frame + kNodePoint, frame + kFromNode }));
    return guest.call(kDot, args({ car + kRoadFrame, frame + kFromNode }));
}

/* sub_4a2cc0: a turn by three angles, the stack's first three, as one matrix at `out`,
 * their magnitudes added to the float the fourth points at; 1 when one of them is not
 * nothing.  The turn about y is worked out in any case. */
reg32 turnMatrix(Guest& guest, reg32 out, reg32 stackArguments)
{
    constexpr reg32 kYOnly = 0x00, kZOnly = 0x24, kYAndX = 0x48, kXOnly = 0x6c;
    const reg32 frame = guest.frame();
    const reg32 angleX = guest.dword(stackArguments), angleY = guest.dword(stackArguments + 4),
                angleZ = guest.dword(stackArguments + 8);
    const reg32 total = guest.dword(stackArguments + 12);
    const Float x = fromBits(magnitudeNotAboveNegated(fromBits(angleX), angleX));
    const Float y = fromBits(magnitudeNotAboveNegated(fromBits(angleY), angleY));
    const Float z = fromBits(magnitudeNotAboveNegated(fromBits(angleZ), angleZ));
    store(guest, total, fp::add(fp::add(fp::add(x, y), z), load(guest, total)));

    guest.call(kYRotation, args({ frame + kYOnly }, { angleY }));
    reg32 turned = jb(fp::compare(0.0, y)) ? 1 : 0;
    if (jb(fp::compare(0.0, x)))
    {
        guest.call(kXRotation, args({ frame + kXOnly }, { angleX }));
        guest.call(kMultiplyMatrices, args({ frame + kXOnly, frame + kYOnly, frame + kYAndX }));
        turned = 1;
    }
    else
    {
        copyDwords(guest, frame + kYAndX, frame + kYOnly, kMatrixDwords);
    }
    if (jb(fp::compare(0.0, z)))
    {
        guest.call(kZRotation, args({ frame + kZOnly }, { angleZ }));
        guest.call(kMultiplyMatrices, args({ frame + kYAndX, frame + kZOnly, out }));
        turned = 1;
    }
    else
    {
        copyDwords(guest, out, frame + kYAndX, kMatrixDwords);
    }
    return turned;
}

// The registers each function pushes, of those a stand-in restores.
constexpr std::uint8_t kKeepsEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsAllButEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

}

void frontend::simTrackPlaceOnRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a0aa0", 0, 0x54, nullptr, 0, kKeepsEsiEdi };
    const reg32 car = cpu.eax, node = cpu.edx, offset = cpu.ebx, facesBack = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return placeOnRoad(guest, car, node, offset, facesBack); });
}

void frontend::simTrackProbeWalls(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a1210", 0, 0x13c, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return probeWalls(guest, car); });
}

void frontend::simTrackPredictContact(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a1610", 12, 0x1e4, nullptr, 0 };
    const reg32 car = cpu.eax, wanted = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return predictContact(guest, car, wanted); });
}

void frontend::simTrackReactToSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a1ce0", 0, 0x98, nullptr, 0, kKeepsAllButEdx };
    const reg32 car = cpu.eax, normal = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return reactToSurface(guest, car, normal); });
}

void frontend::simTrackTunnelRoof(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a21a0", 0, 0x34, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return hitTunnelRoof(guest, car); });
}

void frontend::simTrackDownforceAndDrag(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2260", 0, 0xd0, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return downforceAndDrag(guest, car); });
}

void frontend::simTrackSettle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2690", 0, 0x50, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return settle(guest, car); });
}

void frontend::simTrackOffsetAcross(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2bc0", 0, 0x44, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return offsetAcross(guest, car); });
}

void frontend::simTrackTurnMatrix(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2cc0", 16, 0xc4, nullptr, 0 };
    const reg32 out = cpu.eax, stackArguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return turnMatrix(guest, out, stackArguments); });
}

/* sub_4a2f10 and sub_4a2f30: what a car's +0x614 points at: a thirty-second of its
 * spin put on its frame, and a sixty-fourth (sub_4a2e20). */
void frontend::simTrackSpinThirtySecond(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2f10", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kSpinStep, args({ car, 0x20 })); });
}

void frontend::simTrackSpinSixtyFourth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a2f30", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kSpinStep, args({ car, 0x40 })); });
}

}
