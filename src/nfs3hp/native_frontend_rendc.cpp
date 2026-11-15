#include "native_frontend_rendc.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_aiplan.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_draw.h"
#include "native_frontend_eab.h"
#include "native_frontend_loada.h"
#include "native_frontend_renda.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "port_draw2d.h"
#include "port_shapes.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <cstdint>
#include <cstring>
#include <iterator>

/* What the race draws besides the track and the cars' bodies, decompiled:
 *
 *   the cones        two rows of them along the road, one of the six aids at
 *                    [0x6fd498]: set out as a race loads, knocked over by the cars,
 *                    flying under their own physics, each drawn as the model "%scone";
 *   the lightning    a bolt made of a tree of up to 240 segments, jittered every
 *                    tick, and the glow its first segment gives the sky;
 *   a model drawn    a model's parts through their chain of transforms, its light,
 *                    colour and environment map;
 *   the object lists the ones the driver's objects are queued on;
 *   small ones       the blinking table of track lights, a rough 3D distance, the
 *                    sky sprites put back at the screen's edges, a float cut to its
 *                    fraction, a 2D rotation, a flat distance test and the picture
 *                    shown on the way back to the menus.
 *
 * All of the game's state stays in guest memory.  What each function leaves in eax
 * and in the registers it does not save is kept, the x87's status word in ax too on
 * the ways out that follow a `fnstsw ax`.
 *
 * Plan step 5: each function a typed core in frontend::rendc (native_frontend_rendc.h), in a
 * frame of its own (fe::Frame: what it calls runs at the stack addresses it did), and an adapter
 * under the game's name for the calls by address.  A core of this file calls another as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game
 * still by address. */
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
using fe::GuestString;
using fe::X87;
using x86::Float;
using x86::reg32;
using x86::sreg32;

/* A generated function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

Float floatIn(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

/* A float's bits, as fstp dword [esp] hands an argument over, and back. */
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

/* rep movsd: forwards, a dword at a time. */
void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* What a jump reads after `fcom a, b; fnstsw ax; sahf`: jb (jae's opposite) and jbe.
 * Unordered sets every flag, so it counts as below. */
bool isBelow(x86::reg16 status)
{
    return (status & 0x100) != 0;
}

bool isBelowOrEqual(x86::reg16 status)
{
    return (status & 0x4100) != 0;
}

/* eax after `fnstsw ax`. */
reg32 withStatus(reg32 eax, x86::reg16 status)
{
    return (eax & 0xffff0000u) | status;
}

/* The game's 16.16 product: imul, then the middle 32 bits, rounded half up by the
 * carry the shift leaves for adc. */
reg32 fixedMultiply(reg32 a, reg32 b)
{
    const std::int64_t product = std::int64_t(sreg32(a)) * std::int64_t(sreg32(b));
    const reg32 low = reg32(std::uint64_t(product));
    const reg32 high = reg32(std::uint64_t(product) >> 32);
    return (high << 16) + (low >> 16) + ((low >> 15) & 1);
}

// The game's functions, by address.
constexpr reg32 kDrawMirroredRecords = 0x4349d0;
constexpr reg32 kAppendBackslash = 0x4529a0;
constexpr reg32 kLoadModel = 0x49d450;
constexpr reg32 kShadeNormals = 0x49da30;
constexpr reg32 kEnvironmentCoords = 0x49db70;
constexpr reg32 kCullPolygons = 0x49dbf0;
constexpr reg32 kTakeWindowAndClear = 0x4d9120;
constexpr reg32 kFlipAndWait = 0x4d9140;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kMultiplyMatrices = 0x4e0280;  // eax, edx them; ebx the product
constexpr reg32 kTransformPoints = 0x4e0430;   // eax how many; edx them; ebx the matrix; ecx the offset; the stack the result
constexpr reg32 kFillZero = 0x4e070c;
constexpr reg32 kRandom = 0x4e0d88;
constexpr reg32 kFloor = 0x4f1c30;             // the stack a double; the x87 the result
constexpr reg32 kCeil = 0x4f2d78;

constexpr reg32 kTick = 0x7d3684;              // the race's ticks

/* ---------------------------------------------------------------------------
 * The cones
 * ------------------------------------------------------------------------- */

constexpr reg32 kSliceCount = 0x5dd940;        // the road's slices, a pair of cones each
constexpr reg32 kSlices = 0x5dd948;            // their records, 36 bytes
constexpr reg32 kSliceBytes = 36;
constexpr reg32 kConeStepTick = 0x55e49c;      // the tick the cones have been stepped to
constexpr reg32 kConeRows = 0x55e4a0;          // two arrays, one a side
constexpr reg32 kConeModel = 0x55e4a8;

// A slice's record.
constexpr reg32 kSliceLanes = 0x13;            // byte: how many lanes on each side, a nibble each
constexpr reg32 kSliceAcross = 0x15;           // the dwords from here hold three signed bytes in their tops
constexpr reg32 kSliceLeftWidth = 0x17;        // byte
constexpr reg32 kSliceRightWidth = 0x1b;       // byte
constexpr reg32 kSliceLeftLimit = 0x1c;
constexpr reg32 kSliceRightLimit = 0x20;

// A cone's record: 80 bytes.
constexpr reg32 kConeBytes = 80;
constexpr reg32 kConeDownTimer = 0x00;         // int16: while it runs the cone is gone; -1 it never stands
constexpr reg32 kConeFlightTimer = 0x02;       // int16: while it runs the cone flies
constexpr reg32 kConeColour = 0x04;
constexpr reg32 kConeMatrix = 0x08;
constexpr reg32 kConeHome = 0x2c;
constexpr reg32 kConePosition = 0x38;
constexpr reg32 kConeVelocity = 0x44;

// A car, for what the cones ask of it.
constexpr reg32 kCarSlice = 0x1c;
constexpr reg32 kCarPosition = 0x98;
constexpr reg32 kCarVelocity = 0xa4;
constexpr reg32 kCarSpeed = 0xb8;
constexpr reg32 kCarMatrix = 0xc0;
constexpr reg32 kCarHalfSize = 0x114;
constexpr reg32 kCarSkidTimer = 0x168;         // a float: 0 until a knock starts it
constexpr reg32 kCarSkidBlend = 0x16c;
constexpr reg32 kCarSkidKind = 0x170;
constexpr reg32 kCarSkidPlace = 0x178;
constexpr reg32 kCarFlags = 0x200;
constexpr reg32 kCarTeam = 0x2c4;
constexpr reg32 kCarConesHit = 0x2d8;
constexpr reg32 kCarConesGood = 0x2dc;
constexpr reg32 kCarConesBad = 0x2e0;
constexpr reg32 kCarAcross = 0x7b8;

reg32 coneRow(const Guest& guest, reg32 side)
{
    return guest.dword(kConeRows + 4 * side);
}

reg32 coneAt(const Guest& guest, reg32 side, reg32 slice)
{
    return coneRow(guest, side) + slice * kConeBytes;
}

sreg32 sliceCount(const Guest& guest)
{
    return sreg32(guest.dword(kSliceCount));
}

/* A slice near a car or a view: past the road's start or past its end, each only one
 * way, as the game wraps it. */
reg32 sliceAhead(const Guest& guest, reg32 from, sreg32 ahead)
{
    const reg32 slice = from + reg32(ahead);
    if (ahead < 0)
        return sreg32(slice) < 0 ? slice + guest.dword(kSliceCount) : slice;
    return sreg32(slice) >= sliceCount(guest) ? slice - guest.dword(kSliceCount) : slice;
}

}  // namespace

/* sub_493380: how far across its slice a kind of road puts a cone, in 16.16: at the
 * outermost lane two metres past its edge, else in the middle of the lane, and always
 * inside the slice's limits. */
x86::reg32 frontend::rendc::coneSideOffset(App* app, CPU& cpu, reg32 slice, reg32 kind)
{
    Guest guest(app, cpu);
    // The game writes [0x5dd948] back as it leaves, what it read: nothing changes.
    const reg32 record = guest.dword(kSlices) + slice * kSliceBytes;
    const std::uint8_t lanes = guest.byte(record + kSliceLanes);
    const bool outermost = kind == 6 - reg32((lanes >> 4) & 0xf) || kind == reg32(lanes & 0xf) + 7;
    reg32 offset;
    if (sreg32(kind) < 7)
    {
        const reg32 width = reg32(guest.byte(record + kSliceLeftWidth)) << 15;
        const reg32 lanesIn = (7 - kind) - 1;
        offset = (0 - lanesIn) * width - (outermost ? 0x20000 : width >> 1);
    }
    else
    {
        const reg32 width = reg32(guest.byte(record + kSliceRightWidth)) << 15;
        offset = (kind - 7) * width + (outermost ? 0x20000 : width >> 1);
    }
    const reg32 most = guest.dword(record + kSliceRightLimit) - 0x20000;
    if (sreg32(offset) > sreg32(most))
        offset = most;
    const reg32 least = 0x20000 - guest.dword(record + kSliceLeftLimit);
    if (sreg32(offset) < sreg32(least))
        offset = least;
    guest.leaveEdx(least);
    return offset;
}

/* sub_4934c0: where a cone of a side stands on a slice: the slice's centre moved across
 * by the kind of road's offset and four metres to its side, as floats at `out`. */
x86::reg32 frontend::rendc::coneEdgePoint(App* app, CPU& cpu, reg32 slice, reg32 side, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x38);
    const reg32 record = guest.dword(kSlices) + slice * kSliceBytes;
    const reg32 centreX = guest.dword(record), centreY = guest.dword(record + 4), centreZ = guest.dword(record + 8);
    // The across vector's three signed bytes, at the top of a dword each, to 16.16.
    const reg32 acrossX = reg32(sreg32(guest.dword(record + kSliceAcross)) >> 24) << 9;
    const reg32 acrossY = reg32(sreg32(guest.dword(record + kSliceAcross + 1)) >> 24) << 9;
    const reg32 acrossZ = reg32(sreg32(guest.dword(record + kSliceAcross + 2)) >> 24) << 9;

    const reg32 kind = fe::direct<0x40e0c0, aiplan::roadKind>(app, cpu, slice);
    const reg32 offset = guest::direct<0x493380, coneSideOffset>(app, cpu, slice, kind);
    const reg32 outwards = side != 0 ? 0x40000 : 0xfffc0000;

    const reg32 x = centreX + fixedMultiply(offset, acrossX) + fixedMultiply(outwards, acrossX);
    const reg32 y = centreY + fixedMultiply(offset, acrossY) + fixedMultiply(outwards, acrossY);
    const reg32 z = centreZ + fixedMultiply(offset, acrossZ) + fixedMultiply(outwards, acrossZ);

    const Float toMetres = doubleAt(guest, 0x53bd28);
    guest.setFloat(out, float(fp::mul(Float(sreg32(x)), toMetres)));
    guest.setFloat(out + 4, float(fp::mul(Float(sreg32(y)), toMetres)));
    guest.setFloat(out + 8, float(fp::mul(toMetres, Float(sreg32(z)))));
    guest.leaveEdx(y);
    // Where copying its local to `out` left esi.
    guest.setEsi(guest.frame() + 0x24);
    return out;
}

/* sub_493640: both rows of cones set out for a race: each cone on its slice, the rows
 * smoothed four times along the road, then each set down on the track, in the track's
 * light, or, off the road, never to stand. */
x86::reg32 frontend::rendc::conesPlace(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0xc0);
    const reg32 place = guest.frame();                // the place on the track's record
    const reg32 placeOffRoad = place + 0x6c;
    const reg32 point = place + 0x78;
    const reg32 fixedPoint = place + 0x84;
    const reg32 sum = place + 0x90;

    for (reg32 slice = 0; sreg32(slice) < sliceCount(guest); ++slice)
        for (reg32 side = 0; side < 2; ++side)
        {
            // `point` in esi, as the game's code hands it (ByAddress puts it there).
            guest::direct<0x4934c0, coneEdgePoint>(app, cpu, slice, side, point);
            copyDwords(guest, coneAt(guest, side, slice) + kConePosition, point, 3);
        }

    for (reg32 pass = 0; pass < 4; ++pass)
    {
        // A cone becomes the mean of itself and its neighbours.  The game's "before" for
        // the second side is the first side's own slice, not the one before it.
        reg32 before = guest.dword(kSliceCount) - 1;
        for (reg32 slice = 0; sreg32(slice) < sliceCount(guest); ++slice)
            for (reg32 side = 0; side < 2; ++side)
            {
                const reg32 count = guest.dword(kSliceCount);
                const reg32 after = sreg32(slice + 1) < sreg32(count) ? slice + 1 : slice + 1 - count;
                const reg32 cone = coneAt(guest, side, slice);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, cone + kConePosition,
                                                                       coneAt(guest, side, before) + kConePosition, sum);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, sum, coneAt(guest, side, after) + kConePosition,
                                                                       sum);
                const Float third = floatIn(guest, 0x53bd30);
                const Float x = fp::mul(floatIn(guest, sum), third);
                const Float y = fp::mul(floatIn(guest, sum + 4), third);
                const Float z = fp::mul(third, floatIn(guest, sum + 8));
                guest.setFloat(cone + kConePosition, float(x));
                guest.setFloat(cone + kConePosition + 4, float(y));
                guest.setFloat(cone + kConePosition + 8, float(z));
                before = slice;
            }
    }

    reg32 slice = 0;
    for (; sreg32(slice) < sliceCount(guest); ++slice)
        for (reg32 side = 0; side < 2; ++side)
        {
            const reg32 cone = coneAt(guest, side, slice);
            const Float toFixed = doubleAt(guest, 0x53bd38);
            for (reg32 axis = 0; axis < 3; ++axis)
                guest.setDword(fixedPoint + 4 * axis,
                               reg32(fp::toInteger<sreg32>(fp::mul(floatIn(guest, cone + kConePosition + 4 * axis), toFixed))));
            // `place` in esi, as the game's code hands it (ByAddress puts it there).
            fe::direct<0x420650, trackdata::placeAt>(app, cpu, fixedPoint, 1, slice, place);
            const reg32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, place, 1);

            const Float toMetres = doubleAt(guest, 0x53bd40);
            guest.setFloat(cone + kConePosition, float(fp::mul(Float(sreg32(guest.dword(fixedPoint))), toMetres)));
            guest.setFloat(cone + kConePosition + 4, float(fp::mul(Float(sreg32(guest.dword(fixedPoint + 4))), toMetres)));
            guest.setFloat(cone + kConePosition + 8, float(fp::mul(toMetres, Float(sreg32(guest.dword(fixedPoint + 8))))));

            const reg32 surface = fe::direct<0x4207e0, trackdata::placeSurface>(app, cpu, place, 1);
            if ((surface & 0x3f) == 0 || guest.dword(placeOffRoad) != 0)
            {
                guest.setWord(cone + kConeDownTimer, 0xffff);
                continue;
            }

            // Standing: a little above the road, half way from the track's light to white.
            guest.setFloat(cone + kConePosition + 4, float(fp::add(floatIn(guest, cone + kConePosition + 4), doubleAt(guest, 0x53bd48))));
            const auto lifted = [](reg32 channel) { return (((channel & 0xff) + 0xff) >> 1) & 0xff; };
            guest.setDword(cone + kConeColour, 0xff000000u | (lifted(reg32(sreg32(colour) >> 16)) << 16)
                                                   | (lifted(reg32(sreg32(colour) >> 8)) << 8) | lifted(colour));
            copyDwords(guest, cone + kConeHome, cone + kConePosition, 3);
            static const reg32 kIdentity[9] = { 0x3f800000, 0, 0, 0, 0x3f800000, 0, 0, 0, 0x3f800000 };
            for (reg32 i = 0; i < 9; ++i)
                guest.setDword(cone + kConeMatrix + 4 * i, kIdentity[i]);
        }
    return slice;
}

/* sub_493a30: the car's pass counted, good or bad: early in the race, or when the kind
 * of road is that under its middle or either edge, good. */
x86::reg32 frontend::rendc::coneCountPass(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 8);
    const reg32 slice = guest.dword(car + kCarSlice);
    const reg32 kind = fe::direct<0x40e0c0, aiplan::roadKind>(app, cpu, slice);
    const reg32 acrossBits = guest.dword(car + kCarAcross);
    const Float across = floatIn(guest, car + kCarAcross);
    const reg32 halfWidthBits = guest.dword(car + kCarHalfSize);
    const Float halfWidth = floatIn(guest, car + kCarHalfSize);
    const auto countGood = [&] { guest.setDword(car + kCarConesGood, guest.dword(car + kCarConesGood) + 1); };
    if (sreg32(guest.dword(kTick)) < 0x480)
    {
        countGood();
        return halfWidthBits;
    }
    // What eax is left with: the last of the slices asked for.
    const auto sliceAt = [&](float offset) { return fe::direct<0x40e2b0, aiplan::roadSlice>(app, cpu, slice, offset); };
    reg32 left = sliceAt(floatOf(acrossBits));
    bool good = kind == left;
    if (!good)
    {
        left = sliceAt(float(fp::sub(across, halfWidth)));
        good = kind == left;
    }
    if (!good)
    {
        left = sliceAt(float(fp::add(across, halfWidth)));
        good = kind == left;
    }
    if (good)
        countGood();
    else
        guest.setDword(car + kCarConesBad, guest.dword(car + kCarConesBad) + 1);
    return left;
}

namespace
{

/* Whether a point's coordinate is inside the box's half size: its size, as the game
 * takes it (turned round unless 0 is below it, a NaN as it is), below the half. */
bool insideHalf(const Guest& guest, float value, reg32 half)
{
    const float size = isBelow(fp::compare(0.0, Float(value))) ? value : float(-Float(value));
    return isBelow(fp::compare(Float(size), floatIn(guest, half)));
}

/* A cone knocked: it takes the car's speed and some of its nose, and spins at random
 * for a while (kConeFlightTimer); the car skids if it was not. */
void coneKnocked(Guest& guest, x86::CPU& cpu, reg32 car, reg32 cone, float localX)
{
    const reg32 impulse = guest.frame();
    const Float noseX = floatIn(guest, car + kCarMatrix), noseY = floatIn(guest, car + kCarMatrix + 4);
    const Float noseZ = floatIn(guest, car + kCarMatrix + 8);
    const Float push = doubleAt(guest, 0x53bd58);
    reg32 pushX = bitsOf(float(fp::mul(noseX, push)));
    reg32 pushY = bitsOf(float(fp::mul(noseY, push)));
    reg32 pushZ = bitsOf(float(fp::mul(push, noseZ)));

    if ((guest.dword(car + kCarSkidTimer) & 0x7fffffff) == 0)
    {
        guest.setDword(car + kCarSkidTimer, 0x41700000);
        guest.setDword(car + kCarSkidKind, 0x60009);
        guest.setDword(car + kCarSkidBlend, 0);
        const Float x = floatIn(guest, car + kCarPosition), y = floatIn(guest, car + kCarPosition + 4);
        const Float z = floatIn(guest, car + kCarPosition + 8);
        guest.setFloat(car + kCarSkidPlace, float(x));
        guest.setFloat(car + kCarSkidPlace + 4, float(y));
        guest.setFloat(car + kCarSkidPlace + 8, float(z));
    }

    // Hit from behind: the other way.
    if (!isBelowOrEqual(fp::compare(0.0, Float(localX))))
    {
        pushX ^= 0x80000000u;
        pushY ^= 0x80000000u;
        pushZ ^= 0x80000000u;
    }
    guest.setDword(impulse, pushX);
    guest.setDword(impulse + 4, pushY);
    guest.setDword(impulse + 8, pushZ);
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), cpu, 1, impulse, car + kCarVelocity, cone + kConeVelocity);

    // Up, by the car's speed, as far as ten metres a second.
    const Float pace = fp::mul(floatIn(guest, car + kCarSpeed), doubleAt(guest, 0x53bd60));
    const Float capped = isBelowOrEqual(fp::compare(1.0, pace)) ? 1.0 : pace;
    const Float up = fp::mul(capped, doubleAt(guest, 0x53bd68));
    guest.setFloat(cone + kConeVelocity + 4, float(fp::add(up, floatIn(guest, cone + kConeVelocity + 4))));
    guest.setWord(cone + kConeFlightTimer, 0x80);

    // Its matrix turned at random about each of its rows in turn.
    const reg32 matrix = cone + kConeMatrix;
    const auto randomAngle = [&] { return bitsOf(float(*fe::direct<0x4eb230, ea::randomFractionOf>(guest.app(), cpu))); };
    const reg32 pitch = randomAngle();
    fe::direct<0x4ea9e0, frontend::simd::rotateX>(guest.app(), cpu, matrix, pitch, matrix);
    const reg32 yaw = randomAngle();
    fe::direct<0x4eaab0, frontend::simd::rotateY>(guest.app(), cpu, matrix, yaw, matrix);
    const reg32 roll = randomAngle();
    fe::direct<0x4eab80, frontend::simd::rotateZ>(guest.app(), cpu, matrix, roll, matrix);
    if (!((guest.byte(car + kCarFlags) & 1) != 0 && guest.dword(car + kCarTeam) == 3))
        guest.setDword(car + kCarConesHit, guest.dword(car + kCarConesHit) + 1);
}

}  // namespace

/* sub_493ad0: the cones within three slices of a car doing 5 m/s and up, each tested
 * against the car's box in the car's axes; those inside it knocked. */
x86::reg32 frontend::rendc::conesHit(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x74);
    if (gameSettings(guest).raceOptionCones == 0)
        return car;
    const x86::reg16 moving = fp::compare(floatIn(guest, car + kCarSpeed), doubleAt(guest, 0x53bd50));
    if (isBelow(moving))
        return withStatus(car, moving);

    const reg32 fromCar = guest.frame() + 0x18;
    const reg32 halfSize = guest.frame() + 0x24;      // the car's, copied
    copyDwords(guest, halfSize, car + kCarHalfSize, 3);

    for (sreg32 ahead = -3; ahead < 3; ++ahead)
        for (reg32 side = 0; side < 2; ++side)
        {
            const reg32 slice = sliceAhead(guest, guest.dword(car + kCarSlice), ahead);
            const reg32 cone = coneAt(guest, side, slice);
            if (guest.word(cone + kConeDownTimer) != 0 || guest.word(cone + kConeFlightTimer) != 0)
                continue;

            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, car + kCarPosition, cone + kConePosition, fromCar);
            const Float v0 = floatIn(guest, fromCar), v1 = floatIn(guest, fromCar + 4), v2 = floatIn(guest, fromCar + 8);
            const auto m = [&](reg32 i) { return floatIn(guest, car + kCarMatrix + 4 * i); };
            const float localX = float(fp::add(fp::add(fp::mul(v0, m(0)), fp::mul(v1, m(3))), fp::mul(v2, m(6))));
            const float localY = float(fp::add(fp::add(fp::mul(v0, m(1)), fp::mul(v1, m(4))), fp::mul(v2, m(7))));
            const float localZ = float(fp::add(fp::add(fp::mul(v0, m(2)), fp::mul(v1, m(5))), fp::mul(v2, m(8))));
            if (!insideHalf(guest, localX, halfSize) || !insideHalf(guest, localY, halfSize + 4)
                || !insideHalf(guest, localZ, halfSize + 8))
                continue;
            coneKnocked(guest, cpu, car, cone, localX);
        }

    if ((guest.byte(car + kCarFlags) & 1) != 0 && guest.dword(car + kCarTeam) == 3)
        return car;
    return guest::direct<0x493a30, coneCountPass>(app, cpu, car);
}

/* sub_493e50: every cone back up at its home, for a restart. */
x86::reg32 frontend::rendc::conesReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (gameSettings(guest).raceOptionCones == 0)
        return guest.eax();
    reg32 left = guest.dword(kTick);
    guest.setDword(kConeStepTick, left);
    for (reg32 slice = 0; sreg32(slice) < sliceCount(guest); ++slice)
        for (reg32 side = 0; side < 2; ++side)
        {
            const reg32 cone = coneAt(guest, side, slice);
            guest.setWord(cone + kConeFlightTimer, 0);
            if (std::int16_t(guest.word(cone + kConeDownTimer)) > 0)
                guest.setWord(cone + kConeDownTimer, 0);
            copyDwords(guest, cone + kConePosition, cone + kConeHome, 3);
            left = coneRow(guest, side);
        }
    return left;
}

namespace
{

const GuestString kConeStrings[] = { { 0x53bd74, "%scone" } };

}  // namespace

/* sub_493ef0: the rows and the model, as the race loads, if not yet there. */
x86::reg32 frontend::rendc::conesLoad(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x200);
    if (gameSettings(guest).raceOptionCones == 0)
        return guest.eax();
    if (guest.dword(kConeRows) == 0)
    {
        for (reg32 side = 0; side < 2; ++side)
        {
            guest.setDword(kConeRows + 4 * side,
                           *fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x53bd70, guest.dword(kSliceCount) * kConeBytes, 0));
            fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), coneRow(guest, side), guest.dword(kSliceCount) * kConeBytes);
        }
        guest::direct<0x493640, conesPlace>(app, cpu);
    }
    if (guest.dword(kConeModel) == 0)
    {
        const reg32 name = guest.frame();
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kConeStrings[0].address, guest::Words{0x7a2934});
        guest.setDword(kConeModel, guest.call(kLoadModel, args({ name, 8, 0 })));
    }
    const reg32 now = guest.dword(kTick);
    guest.setDword(kConeStepTick, now);
    return now;
}

/* sub_493fb0: the rows and the model given back as the race ends. */
x86::reg32 frontend::rendc::conesFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (gameSettings(guest).raceOptionCones == 0)
        return guest.eax();
    // What eax is left with: as it came, or what the last free gave back.
    reg32 left = guest.eax();
    if (guest.dword(kConeRows) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(kConeRows));
        const reg32 second = guest.dword(kConeRows + 4);
        guest.setDword(kConeRows, 0);
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, second);
        guest.setDword(kConeRows + 4, 0);
    }
    if (guest.dword(kConeModel) != 0)
    {
        guest::direct<0x49d5a0, modelFree>(app, cpu, guest.dword(kConeModel));
        guest.setDword(kConeModel, 0);
        return 0;
    }
    return left;
}

/* sub_494010: every cone stepped as many ticks as have passed: a flying one under gravity
 * (bouncing at its home height, with friction), then gone for 512 ticks, then back at
 * its home. */
x86::reg32 frontend::rendc::conesStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    const reg32 gravity = guest.frame();
    copyDwords(guest, gravity, 0x493370, 3);
    for (;;)
    {
        const reg32 tick = guest.dword(kConeStepTick);
        if (sreg32(tick) >= sreg32(guest.dword(kTick)))
            return tick;
        guest.setDword(kConeStepTick, tick + 1);
        for (reg32 slice = 0; sreg32(slice) < sliceCount(guest); ++slice)
            for (reg32 side = 0; side < 2; ++side)
            {
                const reg32 cone = coneAt(guest, side, slice);
                const std::uint16_t flight = guest.word(cone + kConeFlightTimer);
                if (flight == 0)
                {
                    const std::uint16_t down = guest.word(cone + kConeDownTimer);
                    if (std::int16_t(down) > 0)
                    {
                        const std::uint16_t left = std::uint16_t(down - 1);
                        guest.setWord(cone + kConeDownTimer, left);
                        if (left == 0)
                            copyDwords(guest, cone + kConePosition, cone + kConeHome, 3);
                    }
                    continue;
                }
                const std::uint16_t left = std::uint16_t(flight - 1);
                guest.setWord(cone + kConeFlightTimer, left);
                if (left == 0)
                {
                    guest.setWord(cone + kConeDownTimer, 0x200);
                    continue;
                }

                const Float dt = floatIn(guest, 0x53bd7c);
                const Float stepX = fp::mul(floatIn(guest, cone + kConeVelocity), dt);
                const Float stepY = fp::mul(floatIn(guest, cone + kConeVelocity + 4), dt);
                const Float stepZ = fp::mul(dt, floatIn(guest, cone + kConeVelocity + 8));
                const Float x = fp::add(stepX, floatIn(guest, cone + kConePosition));
                const Float y = fp::add(stepY, floatIn(guest, cone + kConePosition + 4));
                const Float z = fp::add(stepZ, floatIn(guest, cone + kConePosition + 8));
                guest.setFloat(cone + kConePosition, float(x));
                guest.setFloat(cone + kConePosition + 4, float(y));
                guest.setFloat(cone + kConePosition + 8, float(z));
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, cone + kConeVelocity, gravity,
                                                                       cone + kConeVelocity);
                if (isBelow(fp::compare(floatIn(guest, cone + kConePosition + 4), floatIn(guest, cone + kConeHome + 4))))
                    guest.setFloat(cone + kConeVelocity + 4,
                                   float(-fp::mul(floatIn(guest, cone + kConeVelocity + 4), doubleAt(guest, 0x53bd80))));
                const Float friction = floatIn(guest, 0x53bd88);
                const Float slowX = fp::mul(floatIn(guest, cone + kConeVelocity), friction);
                const Float slowZ = fp::mul(friction, floatIn(guest, cone + kConeVelocity + 8));
                guest.setFloat(cone + kConeVelocity, float(slowX));
                guest.setFloat(cone + kConeVelocity + 8, float(slowZ));
                const reg32 matrix = cone + kConeMatrix;
                fe::direct<0x4ea9e0, frontend::simd::rotateX>(app, cpu, matrix, 0x3ccccccd, matrix);
                fe::direct<0x4eaab0, frontend::simd::rotateY>(app, cpu, matrix, 0xbccccccd, matrix);
                fe::direct<0x4eab80, frontend::simd::rotateZ>(app, cpu, matrix, 0x3c75c28f, matrix);
            }
    }
}

/* sub_494170: the cones 20 slices either way of the view's: stepped, then each standing
 * or flying one within 100 of the view queued as the model's part for its distance, in
 * the driver's objects at its depth.  With bit 2 of [0x7a3a58] it is drawn unlit. */
x86::reg32 frontend::rendc::conesDraw(App* app, CPU& cpu, reg32 view, reg32 centre)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x34);
    if (gameSettings(guest).raceOptionCones == 0)
        return guest.eax();
    const reg32 position = guest.frame();
    const reg32 inView = position + 0x0c;
    const reg32 inViewDepth = inView + 8;
    const reg32 objects = position + 0x18;

    guest::direct<0x49c410, objectsBegin>(app, cpu);
    guest::direct<0x494010, conesStep>(app, cpu);
    for (sreg32 ahead = -20; ahead < 20; ++ahead)
        for (reg32 side = 0; side < 2; ++side)
        {
            const reg32 cone = coneAt(guest, side, sliceAhead(guest, centre, ahead));
            copyDwords(guest, position, cone + kConePosition, 3);
            guest.leaveEdx(cone);
            if (guest.word(cone + kConeDownTimer) != 0)
                continue;
            // By address: what it leaves in edx is what the function leaves when the cone is too far.
            guest.call(kTransformPoints, args({ 1, position, view + 0x44, view + 0x38 }, { inView }));

            // The far test is on the float's bits, as the game has it: a negative one passes.
            const reg32 depthBits = guest.dword(inViewDepth);
            const Float depth = floatIn(guest, inViewDepth);
            if (isBelow(fp::compare(depth, doubleAt(guest, 0x53bd90))) || sreg32(depthBits) > sreg32(0x42c80000))
                continue;

            guest::direct<0x49d750, modelSetShading>(app, cpu, 1);
            guest::direct<0x49d8f0, modelSetColour>(app, cpu, guest.dword(cone + kConeColour));
            if ((guest.byte(0x7a3a58) & 2) != 0)
            {
                guest::direct<0x49d750, modelSetShading>(app, cpu, 0);
                guest::direct<0x49d8f0, modelSetColour>(app, cpu, 0xffffffff);
            }

            reg32 detail = 2;
            if (isBelow(fp::compare(depth, doubleAt(guest, 0x53bd98))))
                detail = 0;
            else if (sreg32(depthBits) < sreg32(0x42480000))
                detail = 1;

            const reg32 model = guest.dword(kConeModel);
            const reg32 spin = guest.word(cone + kConeFlightTimer) != 0 ? cone + kConeMatrix : 0;
            guest.setFloat(position + 4, float(fp::add(floatIn(guest, position + 4), floatIn(guest, model + 0x2c))));
            guest::direct<0x49de20, modelTransformPart>(app, cpu, model, detail, 0, model + 0xfc + detail * 12, spin, position,
                                                        view + 0x44, view + 0x38, 0);

            guest.leaveEdx(objects);
            // `objects` in esi, as the game's code hands it (ByAddress puts it there).
            guest::direct<0x49c4a0, objectsCopy>(app, cpu, objects);
            const float sortDepth = float(fp::sub(depth, floatIn(guest, 0x7dd000)));
            guest::direct<0x49c420, objectsDraw>(app, cpu, view, objects, bitsOf(sortDepth));
            guest::direct<0x49c410, objectsBegin>(app, cpu);
        }
    return guest::direct<0x49d750, modelSetShading>(app, cpu, 1);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The lightning: 240 records of 48 bytes, a bolt a tree of them
 * ------------------------------------------------------------------------- */

constexpr reg32 kBoltNodes = 0x79c570;
constexpr reg32 kBoltNodeBytes = 48;
constexpr reg32 kBoltNodeCount = 240;
constexpr reg32 kBoltAlive = 0x79f270;         // the root of the bolt alive, or 0
constexpr reg32 kBoltStepTick = 0x55e4c0;
constexpr reg32 kBoltLife = 0x55e4ac;          // how long a bolt lasts
constexpr reg32 kBoltStepLength = 0x55e4b0;
constexpr reg32 kBoltLength = 0x55e4b4;        // segments, root to tip
constexpr reg32 kBoltWidth = 0x55e4b8;
constexpr reg32 kBoltBend = 0x55e4bc;

// A node.
constexpr reg32 kNodeLife = 0x00;              // 0 a free one
constexpr reg32 kNodeIsRoot = 0x04;
constexpr reg32 kNodeWidth = 0x08;
constexpr reg32 kNodeAngle = 0x0c;
constexpr reg32 kNodePoint = 0x10;
constexpr reg32 kNodeBelow = 0x1c;             // how many nodes under it
constexpr reg32 kNodeNext = 0x20;              // the node it goes on to
constexpr reg32 kNodeFork = 0x24;              // the one it forks to

}  // namespace

/* sub_494390: the bolt alive's point at `out` and its brightness 0..255 at `brightness`,
 * by how much of its life is left; 1.  With none, 0. */
x86::reg32 frontend::rendc::boltGlow(App* app, CPU& cpu, reg32 out, reg32 brightness)
{
    Guest guest(app, cpu);
    const reg32 root = guest.dword(kBoltAlive);
    if (root == 0)
    {
        guest.leaveEdx(0);
        return 0;
    }
    copyDwords(guest, out, root + kNodePoint, 3);
    const std::int64_t scaled = sreg32(guest.dword(root + kNodeLife) * 510);
    const sreg32 life = sreg32(guest.dword(kBoltLife));
    const sreg32 level = sreg32(scaled / life);
    guest.setDword(brightness, level < 0xff ? reg32(level) : 0xff);
    guest.leaveEdx(reg32(sreg32(scaled % life)));
    return 1;
}

/* sub_4943e0: a free node, the last in the table; 0 when all are taken. */
x86::reg32 frontend::rendc::boltFreeNode(App* app, CPU& cpu)
{
    const Guest guest(app, cpu);
    reg32 found = 0;
    for (reg32 i = 0; i < kBoltNodeCount; ++i)
    {
        const reg32 node = kBoltNodes + i * kBoltNodeBytes;
        if (guest.dword(node + kNodeLife) == 0)
            found = node;
    }
    return found;
}

/* sub_494420: a node from `parent`, one segment on, put in `slot`: bent from its direction
 * by up to two parts of `bend`, narrower towards the tip, and the bolt going on from it
 * for `depth` more; at `forkAt`, unless the dice say no, a fork too, of half the depth.
 * 1 if it made one.  (`slot` is read each time as the game reads it: with a free node's
 * life 0 the tree can land on itself.) */
x86::reg32 frontend::rendc::boltBranch(App* app, CPU& cpu, reg32 parent, reg32 slot, reg32 depth, reg32 forkAt, reg32 bendBits)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x14);
    const float bend = floatOf(bendBits);
    if (depth == 0)
        return 0;
    const reg32 made = guest::direct<0x4943e0, boltFreeNode>(app, cpu);
    guest.setDword(slot, made);
    if (made == 0)
        return 0;
    guest.setDword(guest.dword(slot) + kNodeLife, guest.dword(parent + kNodeLife));
    guest.setDword(guest.dword(slot) + kNodeIsRoot, 0);

    const reg32 turn = guest.call(kRandom) % 5;
    const Float part = fp::mul(Float(bend), doubleAt(guest, 0x53bda0));
    const Float turned = fp::mul(part, Float(sreg32(turn - 2)));
    guest.setFloat(guest.dword(slot) + kNodeAngle, float(fp::add(turned, floatIn(guest, parent + kNodeAngle))));

    const Float cosine = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, cpu, guest.dword(guest.dword(slot) + kNodeAngle));
    const Float alongX = fp::mul(cosine, floatIn(guest, kBoltStepLength));
    guest.setFloat(guest.dword(slot) + kNodePoint, float(fp::add(alongX, floatIn(guest, parent + kNodePoint))));
    const Float sine = fe::direct<0x4ea930, frontend::simd::sineValue>(app, cpu, guest.dword(guest.dword(slot) + kNodeAngle));
    const Float alongZ = fp::mul(sine, floatIn(guest, kBoltStepLength));
    guest.setFloat(guest.dword(slot) + kNodePoint + 8, float(fp::add(alongZ, floatIn(guest, parent + kNodePoint + 8))));
    guest::direct<0x47adc0, frontend::renda::pointHeight>(app, cpu, guest.dword(slot) + kNodePoint);

    const Float gone = fp::div(Float(sreg32(depth)), Float(sreg32(guest.dword(kBoltLength))));
    const Float width = fp::mul(fp::sub(1.0, gone), floatIn(guest, kBoltWidth));
    guest.setFloat(guest.dword(slot) + kNodeWidth, float(width));
    guest.setDword(guest.dword(slot) + kNodeBelow, 0);

    const bool forks = depth == forkAt && guest.call(kRandom) % forkAt != 0;
    // What the last of the branches below gave back, which the game's code adds from eax.
    reg32 node, last;
    if (forks)
    {
        X87 x87(guest);
        const reg32 first = guest.dword(slot);
        const Float at = Float(sreg32(forkAt));
        const reg32 nextFork = reg32(x87.truncated(fp::mul(at, doubleAt(guest, 0x53bda8))));
        const reg32 below =
            guest::direct<0x494420, boltBranch>(app, cpu, first, first + kNodeNext, forkAt - 1, nextFork, guest.dword(kBoltBend));
        const Float wider = fp::mul(floatIn(guest, kBoltBend), floatIn(guest, 0x53bdb0));
        const Float forkAtFork = fp::mul(at, doubleAt(guest, 0x53bdb8));
        guest.setDword(first + kNodeBelow, guest.dword(first + kNodeBelow) + below);
        const reg32 forkFork = reg32(x87.truncated(forkAtFork));
        node = guest.dword(slot);
        last = guest::direct<0x494420, boltBranch>(app, cpu, node, node + kNodeFork, reg32(sreg32(depth) >> 1), forkFork,
                                                    bitsOf(float(wider)));
    }
    else
    {
        node = guest.dword(slot);
        last = guest::direct<0x494420, boltBranch>(app, cpu, node, node + kNodeNext, depth - 1, forkAt, guest.dword(kBoltBend));
    }
    guest.setDword(node + kNodeBelow, guest.dword(node + kNodeBelow) + last);
    return 1;
}

/* sub_4945a0: a bolt struck at a point (three floats, then its angle, on the stack): its
 * root and its tree; it is now the bolt alive.  What is left in eax is what the tree said. */
x86::reg32 frontend::rendc::boltStrike(App* app, CPU& cpu, reg32 x, reg32 y, reg32 z, reg32 angle)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const reg32 root = guest::direct<0x4943e0, boltFreeNode>(app, cpu);
    if (root == 0)
        return 0;
    X87 x87(guest);
    const Float forkAt = fp::mul(Float(sreg32(guest.dword(kBoltLength))), doubleAt(guest, 0x53bdc0));
    guest.setDword(root + kNodeLife, guest.dword(kBoltLife));
    guest.setDword(root + kNodeIsRoot, 1);
    guest.setDword(root + kNodePoint, x);
    guest.setDword(root + kNodePoint + 4, y);
    guest.setDword(root + kNodePoint + 8, z);
    guest.setDword(root + kNodeAngle, angle);
    const reg32 fork = reg32(x87.truncated(forkAt));
    guest.setDword(root + kNodeWidth, 0);
    guest.setDword(root + kNodeBelow, 0);
    const reg32 made =
        guest::direct<0x494420, boltBranch>(app, cpu, root, root + kNodeNext, guest.dword(kBoltLength), fork, guest.dword(kBoltBend));
    guest.setDword(kBoltAlive, root);
    guest.setDword(root + kNodeBelow, guest.dword(root + kNodeBelow) + made);
    return made;
}

/* sub_494630: every live node stepped as many ticks as have passed: nearer its end, and
 * moved a step of its width one way or the other in x and z.  The bolt alive is gone when
 * its root's life is. */
x86::reg32 frontend::rendc::boltsStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 4);
    for (;;)
    {
        const reg32 tick = guest.dword(kBoltStepTick);
        if (sreg32(tick) >= sreg32(guest.dword(kTick)))
            return tick;
        guest.setDword(kBoltStepTick, tick + 1);
        for (reg32 i = 0; i < kBoltNodeCount; ++i)
        {
            const reg32 node = kBoltNodes + i * kBoltNodeBytes;
            if (guest.dword(node + kNodeLife) == 0)
                continue;
            const sreg32 wayX = sreg32(guest.call(kRandom) % 3) - 1;
            const sreg32 wayZ = sreg32(guest.call(kRandom) % 3) - 1;
            const Float stepX = fp::mul(Float(wayX), floatIn(guest, node + kNodeWidth));
            const reg32 life = guest.dword(node + kNodeLife) - 1;
            guest.setDword(node + kNodeLife, life);
            const Float x = fp::add(stepX, floatIn(guest, node + kNodePoint));
            const Float stepZ = fp::mul(Float(wayZ), floatIn(guest, node + kNodeWidth));
            guest.setFloat(node + kNodePoint, float(x));
            guest.setFloat(node + kNodePoint + 8, float(fp::add(stepZ, floatIn(guest, node + kNodePoint + 8))));
            if (node == guest.dword(kBoltAlive) && life == 0)
                guest.setDword(kBoltAlive, 0);
        }
    }
}

namespace
{

/* ---------------------------------------------------------------------------
 * The picture on the way back to the menus: sub_495600
 * ------------------------------------------------------------------------- */

const GuestString kBackImageStrings[] = { { 0x53be48, "%s.qfs" }, { 0x53be20, "back" } };

}  // namespace

x86::reg32 frontend::rendc::backImage(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x100);
    guest.fpu().init();
    guest.fpu().setControl(guest.word(0x79f3f4));
    const reg32 path = guest.frame();
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, cpu, cpu.esp - 4, port::Args{ cpu.eax, cpu.edx, cpu.ebx, cpu.ecx });
    guest.copyString(path, 0x7a3134);
    guest.call(kAppendBackslash, args({ path }));
    const reg32 number = fe::direct<0x478290, frontend::loada::screenSizeNumber>(app, cpu);
    guest.call(kSprintf, args({}, { path + guest.length(path), kBackImageStrings[0].address, number }));
    // The flags in edx, which the core takes from there.
    guest.leaveEdx(0x10);
    const reg32 file = fe::direct<0x4956f0, draw::loadQfs>(app, cpu, path);
    if (file == 0)
        return 0;
    const reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, kBackImageStrings[1].address);
    // The game's code hands it 0 in ebx too, which it does not read.
    guest.setDword(0x8b7638, reg32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, cpu, cpu.esp - 4, shape, 0, cpu.ecx)));
    guest.call(kTakeWindowAndClear);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, cpu, cpu.esp - 4, std::int32_t(guest.dword(0x8b7638)), 0, 0);
    guest.call(kFlipAndWait);
    guest.call(kTakeWindowAndClear);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, cpu, cpu.esp - 4, std::int32_t(guest.dword(0x8b7638)), 0, 0);
    const reg32 freed = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, file);
    return reg32(*fe::direct<0x4daf80, port::draw2d::resetScreens>(app, cpu, cpu.esp - 4,
                                                                    port::Args{ freed, cpu.edx, cpu.ebx, cpu.ecx }));
}

namespace
{

/* ---------------------------------------------------------------------------
 * The small ones
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_491120: a light's colour from the table at 0x55e3dc (12 bytes: colour; a word, 0 for
 * no blink; the bit of the tick it blinks by; the tick's phase); 0 while it is out, and
 * always in the third setting of [0x6fbc30]. */
x86::reg32 frontend::rendc::blinkColour(App* app, CPU& cpu, reg32 light)
{
    const Guest guest(app, cpu);
    if (gameSettings(guest).effectsLevel == 2)
        return 0;
    const reg32 record = 0x55e3dc + light * 12;
    if (guest.word(record + 4) != 0)
    {
        const reg32 tick = guest.dword(kTick) + guest.byte(record + 7);
        if (((sreg32(tick) >> (guest.byte(record + 6) & 31)) & 1) != 0)
            return 0;
    }
    return guest.dword(record);
}

/* sub_4964d0: the float at `value` cut to its fraction, above 1 and below 0; the NaN of
 * those bits (either sign) becomes 0.  Between them it stays. */
x86::reg32 frontend::rendc::wrapFraction(App* app, CPU& cpu, reg32 value)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8 + 0x10);
    const reg32 bits = guest.dword(value);
    if ((bits | 0x80000000u) == 0xffc00000u)
    {
        guest.setDword(value, 0);
        return 0xffc00000u;
    }
    // eax goes on as it was, and the double on the stack.
    const auto call = [&](reg32 function, reg32 eax, Float argument) {
        const double wide = argument;
        std::uint64_t pieces;
        std::memcpy(&pieces, &wide, sizeof pieces);
        return guest.call(function, args({ eax }, { reg32(pieces), reg32(pieces >> 32) }));
    };
    if (sreg32(bits) > 0x3f800000)
    {
        const reg32 left = call(kFloor, bits, floatIn(guest, value));
        guest.setFloat(value, float(fp::sub(floatIn(guest, value), guest.fpu().take())));
        return left;
    }
    const x86::reg16 status = fp::compare(0.0, Float(floatOf(bits)));
    if (isBelowOrEqual(status))
        return withStatus(bits, status);
    const float size = float(-Float(floatOf(bits)));
    const reg32 left = call(kCeil, withStatus(bits, status), Float(size));
    guest.setFloat(value, float(fp::add(guest.fpu().take(), floatIn(guest, value))));
    return left;
}

/* sub_496550: a rough distance between two points of 16.16 numbers. */
x86::reg32 frontend::rendc::approximateDistance(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest guest(app, cpu);
    const auto size = [](reg32 value) { return sreg32(value) > 0 ? value : 0 - value; };
    const reg32 x = size(guest.dword(a) - guest.dword(b));
    const reg32 y = size(guest.dword(a + 4) - guest.dword(b + 4));
    reg32 z = size(guest.dword(a + 8) - guest.dword(b + 8));
    reg32 flat = sreg32(x) > sreg32(y) ? reg32(sreg32(y) >> 2) + x : reg32(sreg32(x) >> 2) + y;
    if (sreg32(flat) > sreg32(z))
        z = reg32(sreg32(z) >> 2);
    else
        flat = reg32(sreg32(flat) >> 2);
    guest.leaveEdx(z);
    return flat + z;
}

/* sub_496bf0: x and y turned by a fraction of a whole turn, to two floats. */
x86::reg32 frontend::rendc::rotatePlane(App* app, CPU& cpu, reg32 xBits, reg32 yBits, reg32 turn, reg32 outX, reg32 outY)
{
    Guest guest(app, cpu);
    const Float x = Float(floatOf(xBits)), y = Float(floatOf(yBits));
    const Float angle = fp::mul(Float(floatOf(turn)), doubleAt(guest, 0x53bf1c));
    const Float sine = fp::sin(angle);
    const Float cosine = fp::cos(angle);
    const Float turnedX = fp::sub(fp::mul(x, cosine), fp::mul(y, sine));
    const Float turnedY = fp::add(fp::mul(sine, x), fp::mul(cosine, y));
    guest.setFloat(outX, float(turnedX));
    guest.setFloat(outY, float(turnedY));
    return outY;
}

/* sub_496f20: whether two points are nearer on the ground (x and z) than a limit, by the
 * longer side and a quarter of the shorter. */
x86::reg32 frontend::rendc::withinFlatDistance(App* app, CPU& cpu, reg32 a, reg32 b, reg32 limitBits)
{
    const Guest guest(app, cpu);
    const Float limit = Float(floatOf(limitBits));
    const auto size = [](float value) {
        return isBelowOrEqual(fp::compare(0.0, Float(value))) ? value : float(-Float(value));
    };
    const float dx = size(float(fp::sub(floatIn(guest, a), floatIn(guest, b))));
    const float dz = size(float(fp::sub(floatIn(guest, a + 8), floatIn(guest, b + 8))));
    const Float quarter = floatIn(guest, 0x53bf24);
    const Float rough = isBelowOrEqual(fp::compare(Float(dx), Float(dz)))
        ? fp::add(fp::mul(Float(dx), quarter), Float(dz))
        : fp::add(fp::mul(Float(dz), quarter), Float(dx));
    return isBelow(fp::compare(Float(float(rough)), limit)) ? 1 : 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The driver's objects
 * ------------------------------------------------------------------------- */

constexpr reg32 kObjectsFirst = 0x7a3d08;
constexpr reg32 kObjectsChain = 0x7a3d0c;
constexpr reg32 kObjectNodeWords = 0x21;
constexpr reg32 kObjectNodeBytes = 0x84;
constexpr reg32 kObjectVertexWords = 8;
constexpr reg32 kObjectVertexBytes = 0x20;

}  // namespace

/* sub_49c420: the objects at `pair` drawn at a depth, the mirror's way when [0x7a3a60] says;
 * then the new pair of ends back into `pair`. */
x86::reg32 frontend::rendc::objectsDraw(App* app, CPU& cpu, reg32 view, reg32 pair, reg32 depth)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 8);
    if (guest.dword(0x7a3a60) == 0)
        fe::direct<0x4c7400, frontend::rende::depthBucketAppend>(app, cpu, view, guest.dword(pair), guest.dword(pair + 4), depth);
    else
        guest.call(kDrawMirroredRecords, args({ view }, { guest.dword(pair), guest.dword(pair + 4) }));
    const reg32 result = fe::direct<0x4bbe70, frontend::rendd::emptyList>(app, cpu, guest.frame());
    copyDwords(guest, pair, guest.frame(), 2);
    return result;
}

/* sub_49c4a0: the driver's chain of objects copied to fresh memory, with their vertices;
 * the copy's two ends at `out`. */
x86::reg32 frontend::rendc::objectsCopy(App* app, CPU& cpu, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x18);
    const reg32 frame = guest.frame();
    const reg32 made = frame + 8;
    reg32 first = 0, last = 0;
    const reg32 start = guest.dword(kObjectsFirst);
    if (start != 0)
        guest.setDword(start, 0);
    for (reg32 from = guest.dword(kObjectsChain); from != 0; from = guest.dword(from))
    {
        fe::direct<0x4bbde0, frontend::rendd::arenaTake>(app, cpu, made, 0x104);
        const reg32 copy = guest.dword(made);
        copyDwords(guest, copy, from, kObjectNodeWords);
        if (first != 0)
            guest.setDword(last, copy);
        else
            first = copy;
        last = copy;
        const reg32 vertices = copy + kObjectNodeBytes;
        guest.setDword(copy + 8, vertices);
        guest.setDword(copy + 0xc, vertices + kObjectVertexBytes);
        guest.setDword(copy + 0x10, vertices + 2 * kObjectVertexBytes);
        copyDwords(guest, vertices, guest.dword(from + 8), kObjectVertexWords);
        copyDwords(guest, vertices + kObjectVertexBytes, guest.dword(from + 0xc), kObjectVertexWords);
        copyDwords(guest, vertices + 2 * kObjectVertexBytes, guest.dword(from + 0x10), kObjectVertexWords);
    }
    guest.setDword(out, first);
    guest.setDword(out + 4, last);
    guest.setEsi(frame + 8);
    return out;
}

namespace
{

/* ---------------------------------------------------------------------------
 * A model drawn
 * ------------------------------------------------------------------------- */

constexpr reg32 kModelShading = 0x55e69c;      // 1: its vertices lit by their normals
constexpr reg32 kModelLight = 0x55e6a0;
constexpr reg32 kModelRed = 0x55e6ac;
constexpr reg32 kModelGreen = 0x55e6b0;
constexpr reg32 kModelBlue = 0x55e6b4;
constexpr reg32 kModelDarkRed = 0x55e6b8;
constexpr reg32 kModelDarkGreen = 0x55e6bc;
constexpr reg32 kModelDarkBlue = 0x55e6c0;
constexpr reg32 kModelBrightRed = 0x55e6c4;
constexpr reg32 kModelBrightGreen = 0x55e6c8;
constexpr reg32 kModelBrightBlue = 0x55e6cc;
constexpr reg32 kModelRangeRed = 0x55e6d0;
constexpr reg32 kModelRangeGreen = 0x55e6d4;
constexpr reg32 kModelRangeBlue = 0x55e6d8;
constexpr reg32 kModelAlphaHigh = 0x55e6e4;
constexpr reg32 kModelAlphaLow = 0x55e6e8;
constexpr reg32 kEnvironmentMatrix = 0x55e6ec;

// A model's record.
constexpr reg32 kModelPositions = 0x10;        // offsets from kModelData
constexpr reg32 kModelNormals = 0x14;
constexpr reg32 kModelVertices = 0x1c;
constexpr reg32 kModelCoords = 0x24;
constexpr reg32 kModelParts = 0xf8;
constexpr reg32 kModelPartPlaces = 0xfc;       // 12 bytes a part
constexpr reg32 kModelPartFirst = 0x3fc;       // a part's first vertex
constexpr reg32 kModelPartCount = 0x4fc;       // and how many
constexpr reg32 kModelData = 0x1f04;

/* The arguments of sub_49de20: three steps of transform, any of them absent. */
struct Chain
{
    reg32 matrix0, place0;
    reg32 matrix1, place1;
    reg32 matrix2, place2;
    reg32 environment;
};

}  // namespace

/* sub_49de20: a part's vertices transformed and projected, and its polygons culled: the
 * nearest depth, which its adapter leaves on the x87 (0 for none). */
frontend::rendc::Nearest frontend::rendc::modelTransformPart(App* app, CPU& cpu, reg32 model, reg32 part,
                                                             reg32 givenMatrix0, reg32 givenPlace0, reg32 givenMatrix1,
                                                             reg32 givenPlace1, reg32 givenMatrix2, reg32 givenPlace2,
                                                             reg32 environment)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 0xe0);
    const Chain chain{ givenMatrix0, givenPlace0, givenMatrix1, givenPlace1, givenMatrix2, givenPlace2, environment };
    const reg32 frame = guest.frame();
    const reg32 allThree = frame + 0x00;       // the game first copies 0x49c3e0 here: the product overwrites it
    const reg32 firstTwo = frame + 0x24;
    const reg32 forEnvironment = frame + 0x48;
    const reg32 identity = frame + 0x6c;
    const reg32 once = frame + 0x90;
    const reg32 zero = frame + 0x9c;
    const reg32 placed = frame + 0xa8;

    const reg32 redGreen = (guest.dword(kModelGreen) << 8) | (guest.dword(kModelRed) << 16) | 0xff000000u;
    const reg32 colour = guest.dword(kModelBlue) | redGreen;
    copyDwords(guest, identity, 0x564498, 9);
    copyDwords(guest, zero, 0x56448c, 3);

    // Left as the game leaves them on the ways out without a part.
    guest.leaveEdx(chain.place0);
    if (model == 0)
    {
        guest.leaveEcx(0);
        return { redGreen, 0.0 };
    }
    if (sreg32(part) >= sreg32(guest.dword(model + kModelParts)))
    {
        guest.leaveEcx(model);
        return { part, 0.0 };
    }

    const reg32 count = guest.dword(model + part * 4 + kModelPartCount);
    const reg32 first = guest.dword(model + part * 4 + kModelPartFirst);
    const reg32 data = model + kModelData;
    const reg32 positions = guest.dword(model + kModelPositions) + data + first * 12;
    const reg32 normals = guest.dword(model + kModelNormals) + data + first * 12;
    const reg32 vertices = guest.dword(model + kModelVertices) + data + first * 32;
    const reg32 coords = guest.dword(model + kModelCoords) + data + first * 12;

    const auto matrixOr = [&](reg32 matrix) { return matrix != 0 ? matrix : identity; };
    const auto placeOr = [&](reg32 place) { return place != 0 ? place : zero; };
    const reg32 matrix0 = matrixOr(chain.matrix0), matrix1 = matrixOr(chain.matrix1), matrix2 = matrixOr(chain.matrix2);
    const reg32 place0 = placeOr(chain.place0), place1 = placeOr(chain.place1), place2 = placeOr(chain.place2);

    fe::direct<0x4e0430, vertices::transformPointList>(app, cpu, 1, place0, matrix1, place1, once);
    fe::direct<0x4e0430, vertices::transformPointList>(app, cpu, 1, once, matrix2, place2, placed);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, matrix0, matrix1, firstTwo);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, firstTwo, matrix2, allThree);

    // The two words the game's call pushes, where it pushes them, for what reads this frame's leftovers
    // after it (sub_4b8740, after rendd's calls by address).
    guest.setDword(cpu.esp - 8, placed);
    guest.setDword(cpu.esp - 4, colour);
    const float nearest = fe::direct<0x49d9d0, vertices::colouredNearestZ>(app, cpu, sreg32(count), positions, vertices,
                                                                            allThree, placed, colour);

    const reg32 shading = guest.dword(kModelShading);
    guest.leaveEcx(shading);
    if (shading == 1)
        guest.call(kShadeNormals, args({ model, count, normals, coords }, { vertices, firstTwo }));
    if (chain.environment != 0)
    {
        fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, firstTwo, kEnvironmentMatrix, forEnvironment);
        guest.call(kEnvironmentCoords, args({ count, normals, coords, vertices }, { forEnvironment }));
    }
    guest.call(kCullPolygons, args({ model, part, chain.environment }));
    return { bitsOf(nearest), double(nearest) };
}

/* sub_49d5c0: all a model's parts, each at its place in the model: the nearest depth among
 * them, which its adapter leaves on the x87.  Its registers and stack go straight on to sub_49de20's
 * stack. */
frontend::rendc::Nearest frontend::rendc::modelTransformAll(App* app, CPU& cpu, reg32 model, reg32 matrix1,
                                                            reg32 place1, reg32 matrix2, reg32 place2, reg32 environment)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x14);
    reg32 left = model;
    guest.leaveEdx(0x477fff00);
    reg32 nearest = 0x477fff00;
    for (reg32 part = 0; sreg32(part) < sreg32(guest.dword(model + kModelParts)); ++part)
    {
        const reg32 partPlace = model + kModelPartPlaces + part * 12;
        const Nearest returned = guest::direct<0x49de20, modelTransformPart>(app, cpu, model, part, 0, partPlace, matrix1,
                                                                            place1, matrix2, place2, environment);
        const Float depth = returned.depth;
        const x86::reg16 status = fp::compare(depth, Float(floatOf(nearest)));
        if (isBelow(status))
        {
            nearest = bitsOf(float(depth));
            left = nearest;
        }
        else
            left = withStatus(returned.eax, status);
    }
    return { left, double(floatOf(nearest)) };
}

/* sub_49d780: the matrix an environment map turns by, from where the camera looks. */
x86::reg32 frontend::rendc::modelSetEnvironment(App* app, CPU& cpu, reg32 direction)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x54);
    const reg32 yaw = guest.frame() + 0x48;
    const reg32 flat = guest.frame() + 0x4c;
    const reg32 pitch = guest.frame() + 0x50;
    const reg32 yawMatrix = guest.frame() + 0x24;
    const reg32 pitchMatrix = guest.frame();

    const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, cpu, guest.dword(direction),
                                                                                  guest.dword(direction + 8));
    const Float z = floatIn(guest, direction + 8);
    const Float x = floatIn(guest, direction);
    const Float squares = fp::add(fp::mul(z, z), fp::mul(x, x));
    guest.setFloat(yaw, float(fp::add(heading, floatIn(guest, 0x53c68c))));
    guest.setFloat(flat, float(fe::direct<0x4ea920, frontend::simc::squareRootOf>(app, cpu, bitsOf(float(squares)))));
    const Float tilt = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, cpu, guest.dword(flat),
                                                                               guest.dword(direction + 4));
    guest.setFloat(pitch, float(fp::add(tilt, doubleAt(guest, 0x53c690))));
    // The angles by their places in the frame, which the game's code pushes copies of.
    fe::direct<0x4dfde0, ea::rotateAboutY>(app, cpu, yawMatrix, yaw);
    fe::direct<0x4dfd80, ea::rotateAboutX>(app, cpu, pitchMatrix, pitch);
    // By address: the eax it leaves is this function's answer, and its core hands back none.
    return guest.call(kMultiplyMatrices, args({ yawMatrix, pitchMatrix, kEnvironmentMatrix }));
}

/* sub_49d8f0: the colour a model is lit from (0xrrggbb): from a third of it (half, for
 * blue) to twice it. */
x86::reg32 frontend::rendc::modelSetColour(App* app, CPU& cpu, reg32 colour)
{
    Guest guest(app, cpu);
    const reg32 red = reg32(sreg32(colour) >> 16) & 0xff;
    const reg32 green = reg32(sreg32(colour) >> 8) & 0xff;
    const reg32 blue = colour & 0xff;
    const auto twice = [](reg32 channel) { return channel + channel > 0xff ? reg32(0xff) : channel + channel; };
    guest.setDword(kModelDarkRed, red / 3);
    guest.setDword(kModelDarkGreen, green / 3);
    guest.setDword(kModelDarkBlue, blue >> 1);
    guest.setDword(kModelBrightRed, twice(red));
    guest.setDword(kModelBrightGreen, twice(green));
    guest.setDword(kModelBrightBlue, twice(blue));
    guest.setDword(kModelRangeRed, twice(red) - red / 3);
    guest.setDword(kModelRangeGreen, twice(green) - green / 3);
    const reg32 rangeBlue = twice(blue) - (blue >> 1);
    guest.setDword(kModelRangeBlue, rangeBlue);
    guest.setDword(kModelBlue, blue);
    guest.setDword(kModelGreen, green);
    guest.setDword(kModelRed, red);
    return rangeBlue;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The sky's sprites
 * ------------------------------------------------------------------------- */

constexpr reg32 kRandomIndex = 0x57126b;       // a byte
constexpr reg32 kRandomTable = 0x552938;       // 256 bytes
constexpr reg32 kSpriteCycle = 0x55e984;
constexpr reg32 kSpriteKinds = 0x7a1b90;
constexpr reg32 kSpriteX = 0x55e95c;
constexpr reg32 kSpriteY = 0x7a1ac0;
constexpr reg32 kSpriteSpeedX = 0x7a1ae8;
constexpr reg32 kSpriteSpeedY = 0x7a1b10;
constexpr reg32 kSpriteKind = 0x7a1b38;
constexpr reg32 kSpriteScale = 0x7a1b60;
constexpr reg32 kBigSprite = 9;

/* The next of the game's table of random bytes. */
std::uint8_t nextRandomByte(Guest& guest)
{
    const std::uint8_t index = guest.byte(kRandomIndex);
    const std::uint8_t value = guest.byte(kRandomTable + index);
    guest.setByte(kRandomIndex, std::uint8_t(index + 1));
    return value;
}

}  // namespace

/* sub_4a4060: a whole number from one to the other, either way round. */
x86::reg32 frontend::rendc::skyRandomBetween(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest guest(app, cpu);
    const reg32 low = sreg32(b) < sreg32(a) ? b : a;
    const reg32 high = sreg32(b) < sreg32(a) ? a : b;
    const sreg32 range = sreg32(high - low + 1);
    const sreg32 value = nextRandomByte(guest);
    const reg32 remainder = reg32(value % range);
    guest.leaveEdx(remainder);
    return low + remainder;
}

/* sub_4a40c0: a sky sprite put back in at an edge of the screen: from one of the four
 * sides at random, moving across; the big one, the ninth, always in one place, its kinds
 * taken in turn.  Each comes back a fifth smaller, as if further away. */
x86::reg32 frontend::rendc::skySpritePlace(App* app, CPU& cpu, reg32 sprite)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const reg32 at = sprite * 4;
    const auto oneWayOrOther = [&] { return (nextRandomByte(guest) & 1) != 0 ? reg32(1) : reg32(0xffffffff); };
    const auto between = [&](reg32 a, reg32 b) { return guest::direct<0x4a4060, skyRandomBetween>(app, cpu, a, b); };
    guest.setDword(kSpriteKinds, 6);
    reg32 left = 0;
    if (sprite == kBigSprite)
    {
        const reg32 cycle = guest.dword(kSpriteCycle);
        guest.setDword(kSpriteKind + at, cycle);
        guest.setDword(kSpriteScale + at, 0x3fcccccd);
        guest.setDword(kSpriteY + at, 0x3c);
        guest.setDword(kSpriteCycle, reg32(sreg32(cycle + 1) % 6));
        guest.setDword(kSpriteX + at, 0x19a);
        guest.setDword(kSpriteSpeedX + at, 0);
        guest.setDword(kSpriteSpeedY + at, 3);
    }
    else
    {
        // The table of targets at 0x4a40a4 holds these four, in this order.
        switch (nextRandomByte(guest) % 4)
        {
        case 0:
            guest.setDword(kSpriteX + at, 0x96);
            guest.setDword(kSpriteY + at, between(0, 0x1c2));
            guest.setDword(kSpriteSpeedX + at, between(1, 4));
            guest.setDword(kSpriteSpeedY + at, oneWayOrOther());
            guest.setDword(kSpriteScale + at, 0x3f4ccccd);
            break;
        case 1:
            guest.setDword(kSpriteX + at, 0x280);
            guest.setDword(kSpriteY + at, between(0, 0x1c2));
            guest.setDword(kSpriteSpeedX + at, 0 - between(1, 4));
            guest.setDword(kSpriteSpeedY + at, oneWayOrOther());
            guest.setDword(kSpriteScale + at, 0x3f800000);
            break;
        case 2:
            guest.setDword(kSpriteX + at, between(0x96, 0x280));
            guest.setDword(kSpriteY + at, 0x1c2);
            guest.setDword(kSpriteSpeedX + at, oneWayOrOther());
            guest.setDword(kSpriteSpeedY + at, 0 - between(1, 4));
            guest.setDword(kSpriteScale + at, 0x3f4ccccd);
            break;
        default:
            guest.setDword(kSpriteX + at, between(0x96, 0));
            guest.setDword(kSpriteY + at, 0);
            guest.setDword(kSpriteSpeedX + at, oneWayOrOther());
            guest.setDword(kSpriteSpeedY + at, between(1, 4));
            guest.setDword(kSpriteScale + at, 0x3f800000);
            break;
        }
        const sreg32 value = nextRandomByte(guest);
        const sreg32 kinds = sreg32(guest.dword(kSpriteKinds));
        guest.setDword(kSpriteKind + at, reg32(value % kinds));
        left = reg32(value / kinds);
    }
    guest.setFloat(kSpriteScale + at, float(fp::mul(floatIn(guest, kSpriteScale + at), doubleAt(guest, 0x53cf04))));
    return left;
}

/* sub_494360: the bolts' table cleared, none alive, their clock at 0. */
x86::reg32 frontend::rendc::boltsClear(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const reg32 result = guest.call(kFillZero, args({ kBoltNodes, kBoltNodeCount * kBoltNodeBytes }));
    guest.setDword(kBoltStepTick, 0);
    guest.setDword(kBoltAlive, 0);
    return result;
}

/* sub_49c410: the driver's objects started afresh. */
x86::reg32 frontend::rendc::objectsBegin(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, cpu);
}

/* sub_49c470: the driver's objects' two ends taken, to `out` (esi). */
x86::reg32 frontend::rendc::objectsEnd(App* app, CPU& cpu, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 8);
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, cpu, guest.frame());
    copyDwords(guest, out, guest.frame(), 2);
    guest.setEsi(guest.frame() + 8);
    return out;
}

/* sub_49d5a0: a model's data and the model given back. */
x86::reg32 frontend::rendc::modelFree(App* app, CPU& cpu, reg32 model)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 data = guest.dword(model);
    if (data != 0)
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, data);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, model);
}

/* sub_49d720: the alphas a model's vertices are drawn with, the high one and the low. */
x86::reg32 frontend::rendc::modelSetAlphas(App* app, CPU& cpu, reg32 high, reg32 low)
{
    Guest guest(app, cpu);
    guest.setDword(kModelAlphaHigh, (high & 0xff) << 24);
    const reg32 alpha = ((low & 0xff) << 24) | 0xffffff;
    guest.setDword(kModelAlphaLow, alpha);
    return alpha;
}

/* sub_49d750: whether a model's vertices are lit by their normals. */
x86::reg32 frontend::rendc::modelSetShading(App* app, CPU& cpu, reg32 shading)
{
    Guest guest(app, cpu);
    guest.setDword(kModelShading, shading);
    return shading;
}

/* sub_49d760: the direction a model is lit from, three floats at `light`. */
x86::reg32 frontend::rendc::modelSetLight(App* app, CPU& cpu, reg32 light)
{
    Guest guest(app, cpu);
    copyDwords(guest, kModelLight, light, 3);
    return light;
}

/* ---------------------------------------------------------------------------
 * As the game calls them
 * ------------------------------------------------------------------------- */

void frontend::rendBlinkColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_491120", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 light = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::blinkColour(app, cpu, light); });
}

void frontend::rendConeSideOffset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493380", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 slice = cpu.eax, kind = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::coneSideOffset(app, cpu, slice, kind); });
}

void frontend::rendConeEdgePoint(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4934c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const reg32 slice = cpu.eax, side = cpu.edx, out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::coneEdgePoint(app, cpu, slice, side, out); });
}

void frontend::rendConesPlace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493640", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesPlace(app, cpu); });
}

void frontend::rendConeCountPass(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493a30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::coneCountPass(app, cpu, car); });
}

void frontend::rendConesHit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493ad0", 0, 0, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesHit(app, cpu, car); });
}

void frontend::rendConesReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493e50", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesReset(app, cpu); });
}

void frontend::rendConesLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493ef0", 0, 0, kConeStrings, std::size(kConeStrings), fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesLoad(app, cpu); });
}

void frontend::rendConesFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_493fb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesFree(app, cpu); });
}

void frontend::rendConesStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494010", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesStep(app, cpu); });
}

void frontend::rendConesDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494170", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax, slice = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::conesDraw(app, cpu, view, slice); });
}

void frontend::rendBoltsClear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494360", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltsClear(app, cpu); });
}

void frontend::rendBoltGlow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494390", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 out = cpu.eax, brightness = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltGlow(app, cpu, out, brightness); });
}

void frontend::rendBoltFreeNode(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4943e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltFreeNode(app, cpu); });
}

void frontend::rendBoltBranch(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494420", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 parent = cpu.eax, slot = cpu.edx, depth = cpu.ebx, forkAt = cpu.ecx;
    const reg32 bend = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltBranch(app, cpu, parent, slot, depth, forkAt, bend); });
}

void frontend::rendBoltStrike(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4945a0", 16, 0, nullptr, 0 };
    const reg32 x = stackArgument(app, cpu, 0), y = stackArgument(app, cpu, 1), z = stackArgument(app, cpu, 2);
    const reg32 angle = stackArgument(app, cpu, 3);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltStrike(app, cpu, x, y, z, angle); });
}

void frontend::rendBoltsStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_494630", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::boltsStep(app, cpu); });
}

void frontend::rendBackImage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_495600", 0, 0, kBackImageStrings, std::size(kBackImageStrings) };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::backImage(app, cpu); });
}

void frontend::rendWrapFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4964d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 value = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::wrapFraction(app, cpu, value); });
}

void frontend::rendApproxDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_496550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::approximateDistance(app, cpu, a, b); });
}

void frontend::rendRotatePlane(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_496bf0", 20, 0, nullptr, 0 };
    const reg32 x = stackArgument(app, cpu, 0), y = stackArgument(app, cpu, 1), turn = stackArgument(app, cpu, 2);
    const reg32 outX = stackArgument(app, cpu, 3), outY = stackArgument(app, cpu, 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::rotatePlane(app, cpu, x, y, turn, outX, outY); });
}

void frontend::rendWithinFlatDistance(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_496f20", 4, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 a = cpu.eax, b = cpu.edx;
    const reg32 limit = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::withinFlatDistance(app, cpu, a, b, limit); });
}

void frontend::rendObjectsBegin(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c410", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::objectsBegin(app, cpu); });
}

void frontend::rendObjectsDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c420", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax, pair = cpu.edx;
    const reg32 depth = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::objectsDraw(app, cpu, view, pair, depth); });
}

void frontend::rendObjectsEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c470", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::objectsEnd(app, cpu, out); });
}

void frontend::rendObjectsCopy(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49c4a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::objectsCopy(app, cpu, out); });
}

void frontend::rendModelFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d5a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 model = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelFree(app, cpu, model); });
}

std::optional<double> frontend::rendc::modelTransformAllCalled(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49d5c0", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 model = cpu.eax, matrix1 = cpu.edx, place1 = cpu.ebx, matrix2 = cpu.ecx;
    const reg32 place2 = stackArgument(app, cpu, 0), environment = stackArgument(app, cpu, 1);
    std::optional<double> depth;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        const Nearest nearest = modelTransformAll(app, cpu, model, matrix1, place1, matrix2, place2, environment);
        depth = nearest.depth;
        return nearest.eax;
    });
    return depth;
}

void frontend::rendModelTransformAll(win32::WinApplication* app, x86::CPU& cpu)
{
    const std::optional<double> depth = rendc::modelTransformAllCalled(app, cpu);
    if (depth)
        cpu.fpu.push(x86::Float(*depth));
}

void frontend::rendModelSetAlphas(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d720", 0, 0, nullptr, 0, 0 };
    const reg32 high = cpu.eax, low = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelSetAlphas(app, cpu, high, low); });
}

void frontend::rendModelSetShading(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d750", 0, 0, nullptr, 0, 0 };
    const reg32 shading = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelSetShading(app, cpu, shading); });
}

void frontend::rendModelSetLight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d760", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 light = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelSetLight(app, cpu, light); });
}

void frontend::rendModelSetEnvironment(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 direction = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelSetEnvironment(app, cpu, direction); });
}

void frontend::rendModelSetColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49d8f0", 0, 0, nullptr, 0 };
    const reg32 colour = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::modelSetColour(app, cpu, colour); });
}

std::optional<double> frontend::rendc::modelTransformPartCalled(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49de20", 20, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 model = cpu.eax, part = cpu.edx, matrix0 = cpu.ebx, place0 = cpu.ecx;
    const reg32 matrix1 = stackArgument(app, cpu, 0), place1 = stackArgument(app, cpu, 1);
    const reg32 matrix2 = stackArgument(app, cpu, 2), place2 = stackArgument(app, cpu, 3);
    const reg32 environment = stackArgument(app, cpu, 4);
    std::optional<double> depth;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        const Nearest nearest = modelTransformPart(app, cpu, model, part, matrix0, place0, matrix1, place1, matrix2,
                                                   place2, environment);
        depth = nearest.depth;
        return nearest.eax;
    });
    return depth;
}

void frontend::rendModelTransformPart(win32::WinApplication* app, x86::CPU& cpu)
{
    const std::optional<double> depth = rendc::modelTransformPartCalled(app, cpu);
    if (depth)
        cpu.fpu.push(x86::Float(*depth));
}

void frontend::rendSkyRandomBetween(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a4060", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 a = cpu.eax, b = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::skyRandomBetween(app, cpu, a, b); });
}

void frontend::rendSkySpritePlace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a40c0", 0, 0, nullptr, 0 };
    const reg32 sprite = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendc::skySpritePlace(app, cpu, sprite); });
}

}
