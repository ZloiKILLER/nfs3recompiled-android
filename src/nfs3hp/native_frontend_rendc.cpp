#include "native_frontend_rendc.h"
#include "native_load_carmodels.h"
#include "native_collide.h"
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
#include "native_graphics.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstdint>
#include <cstring>
#include <iterator>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
nfs::u32 rendBlinkColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConeSideOffset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConeEdgePoint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesPlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConeCountPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesHit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendConesDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltsClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltGlow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltFreeNode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltBranch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltStrike(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBoltsStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBackImage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendWrapFraction(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendApproxDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendRotatePlane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendWithinFlatDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendObjectsBegin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendObjectsDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendObjectsEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendObjectsCopy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelTransformAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelSetAlphas(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelSetShading(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelSetLight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelSetEnvironment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelSetColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendModelTransformPart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkyRandomBetween(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkySpritePlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}
}

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
 * frame of its own (fe::Frame: what it calls runs at the stack addresses it did), and a thunk
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
using nfs::Float;
using nfs::u32;
using nfs::s32;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline u32 callAt(Guest& guest, u32 function, const fe::Arguments& arguments = fe::Arguments(), u32 esi = 0,
                    double* floatResult = nullptr)
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<u32> result = guest::pointerCall(guest.app(), ts, function, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

/* fild, fld and sub_4dfd56 with fistp (fe::FloatMath's) as plain values: no x87 behind them. */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}
    static Float integer(std::int32_t value) { return Float(s32(value)); }
    Float load(u32 address) const { return Float(m_guest.floatAt(address)); }
    std::int32_t truncated(const Float& value)
    {
        const double chopped = guest::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<s32>(chopped);  // fistp
    }

private:
    Guest& m_guest;
};

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

Float floatIn(const Guest& guest, u32 address)
{
    return Float(guest.floatAt(address));
}

/* A float's bits, as fstp dword [esp] hands an argument over, and back. */
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

/* rep movsd: forwards, a dword at a time. */
void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* What a jump reads after `fcom a, b; fnstsw ax; sahf`: jb (jae's opposite) and jbe.
 * Unordered sets every flag, so it counts as below. */
bool isBelow(nfs::u16 status)
{
    return (status & 0x100) != 0;
}

bool isBelowOrEqual(nfs::u16 status)
{
    return (status & 0x4100) != 0;
}

/* eax after `fnstsw ax`. */
u32 withStatus(u32 eax, nfs::u16 status)
{
    return (eax & 0xffff0000u) | status;
}

/* The game's 16.16 product: imul, then the middle 32 bits, rounded half up by the
 * carry the shift leaves for adc. */
u32 fixedMultiply(u32 a, u32 b)
{
    const std::int64_t product = std::int64_t(s32(a)) * std::int64_t(s32(b));
    const u32 low = u32(std::uint64_t(product));
    const u32 high = u32(std::uint64_t(product) >> 32);
    return (high << 16) + (low >> 16) + ((low >> 15) & 1);
}

// The game's functions, by address.
constexpr u32 kDrawMirroredRecords = 0x4349d0;
constexpr u32 kAppendBackslash = 0x4529a0;
constexpr u32 kShadeNormals = 0x49da30;
constexpr u32 kEnvironmentCoords = 0x49db70;
constexpr u32 kCullPolygons = 0x49dbf0;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kMultiplyMatrices = 0x4e0280;  // eax, edx them; ebx the product
constexpr u32 kTransformPoints = 0x4e0430;   // eax how many; edx them; ebx the matrix; ecx the offset; the stack the result
constexpr u32 kFillZero = 0x4e070c;
constexpr u32 kRandom = 0x4e0d88;
constexpr u32 kFloor = 0x4f1c30;             // the stack a double; the x87 the result
constexpr u32 kCeil = 0x4f2d78;

constexpr u32 kTick = 0x7d3684;              // the race's ticks

/* ---------------------------------------------------------------------------
 * The cones
 * ------------------------------------------------------------------------- */

constexpr u32 kSliceCount = 0x5dd940;        // the road's slices, a pair of cones each
constexpr u32 kSlices = 0x5dd948;            // their records, 36 bytes
constexpr u32 kSliceBytes = 36;
constexpr u32 kConeStepTick = 0x55e49c;      // the tick the cones have been stepped to
constexpr u32 kConeRows = 0x55e4a0;          // two arrays, one a side
constexpr u32 kConeModel = 0x55e4a8;

// A slice's record.
constexpr u32 kSliceLanes = 0x13;            // byte: how many lanes on each side, a nibble each
constexpr u32 kSliceAcross = 0x15;           // the dwords from here hold three signed bytes in their tops
constexpr u32 kSliceLeftWidth = 0x17;        // byte
constexpr u32 kSliceRightWidth = 0x1b;       // byte
constexpr u32 kSliceLeftLimit = 0x1c;
constexpr u32 kSliceRightLimit = 0x20;

// A cone's record: 80 bytes.
constexpr u32 kConeBytes = 80;
constexpr u32 kConeDownTimer = 0x00;         // int16: while it runs the cone is gone; -1 it never stands
constexpr u32 kConeFlightTimer = 0x02;       // int16: while it runs the cone flies
constexpr u32 kConeColour = 0x04;
constexpr u32 kConeMatrix = 0x08;
constexpr u32 kConeHome = 0x2c;
constexpr u32 kConePosition = 0x38;
constexpr u32 kConeVelocity = 0x44;

// A car, for what the cones ask of it.
constexpr u32 kCarSlice = 0x1c;
constexpr u32 kCarPosition = 0x98;
constexpr u32 kCarVelocity = 0xa4;
constexpr u32 kCarSpeed = 0xb8;
constexpr u32 kCarMatrix = 0xc0;
constexpr u32 kCarHalfSize = 0x114;
constexpr u32 kCarSkidTimer = 0x168;         // a float: 0 until a knock starts it
constexpr u32 kCarSkidBlend = 0x16c;
constexpr u32 kCarSkidKind = 0x170;
constexpr u32 kCarSkidPlace = 0x178;
constexpr u32 kCarFlags = 0x200;
constexpr u32 kCarTeam = 0x2c4;
constexpr u32 kCarConesHit = 0x2d8;
constexpr u32 kCarConesGood = 0x2dc;
constexpr u32 kCarConesBad = 0x2e0;
constexpr u32 kCarAcross = 0x7b8;

u32 coneRow(const Guest& guest, u32 side)
{
    return guest.dword(kConeRows + 4 * side);
}

u32 coneAt(const Guest& guest, u32 side, u32 slice)
{
    return coneRow(guest, side) + slice * kConeBytes;
}

s32 sliceCount(const Guest& guest)
{
    return s32(guest.dword(kSliceCount));
}

/* A slice near a car or a view: past the road's start or past its end, each only one
 * way, as the game wraps it. */
u32 sliceAhead(const Guest& guest, u32 from, s32 ahead)
{
    const u32 slice = from + u32(ahead);
    if (ahead < 0)
        return s32(slice) < 0 ? slice + guest.dword(kSliceCount) : slice;
    return s32(slice) >= sliceCount(guest) ? slice - guest.dword(kSliceCount) : slice;
}

}  // namespace

/* sub_493380: how far across its slice a kind of road puts a cone, in 16.16: at the
 * outermost lane two metres past its edge, else in the middle of the lane, and always
 * inside the slice's limits. */
nfs::u32 frontend::rendc::coneSideOffset(App* app, ThreadState& ts, u32 slice, u32 kind)
{
    Guest guest(app, ts);
    // The game writes [0x5dd948] back as it leaves, what it read: nothing changes.
    const u32 record = guest.dword(kSlices) + slice * kSliceBytes;
    const std::uint8_t lanes = guest.byte(record + kSliceLanes);
    const bool outermost = kind == 6 - u32((lanes >> 4) & 0xf) || kind == u32(lanes & 0xf) + 7;
    u32 offset;
    if (s32(kind) < 7)
    {
        const u32 width = u32(guest.byte(record + kSliceLeftWidth)) << 15;
        const u32 lanesIn = (7 - kind) - 1;
        offset = (0 - lanesIn) * width - (outermost ? 0x20000 : width >> 1);
    }
    else
    {
        const u32 width = u32(guest.byte(record + kSliceRightWidth)) << 15;
        offset = (kind - 7) * width + (outermost ? 0x20000 : width >> 1);
    }
    const u32 most = guest.dword(record + kSliceRightLimit) - 0x20000;
    if (s32(offset) > s32(most))
        offset = most;
    const u32 least = 0x20000 - guest.dword(record + kSliceLeftLimit);
    if (s32(offset) < s32(least))
        offset = least;
    return offset;
}

/* sub_4934c0: where a cone of a side stands on a slice: the slice's centre moved across
 * by the kind of road's offset and four metres to its side, as floats at `out`. */
nfs::u32 frontend::rendc::coneEdgePoint(App* app, ThreadState& ts, u32 slice, u32 side, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x38);
    const u32 record = guest.dword(kSlices) + slice * kSliceBytes;
    const u32 centreX = guest.dword(record), centreY = guest.dword(record + 4), centreZ = guest.dword(record + 8);
    // The across vector's three signed bytes, at the top of a dword each, to 16.16.
    const u32 acrossX = u32(s32(guest.dword(record + kSliceAcross)) >> 24) << 9;
    const u32 acrossY = u32(s32(guest.dword(record + kSliceAcross + 1)) >> 24) << 9;
    const u32 acrossZ = u32(s32(guest.dword(record + kSliceAcross + 2)) >> 24) << 9;

    const u32 kind = fe::direct<0x40e0c0, aiplan::roadKind>(app, ts, slice);
    const u32 offset = guest::direct<0x493380, coneSideOffset>(app, ts, slice, kind);
    const u32 outwards = side != 0 ? 0x40000 : 0xfffc0000;

    const u32 x = centreX + fixedMultiply(offset, acrossX) + fixedMultiply(outwards, acrossX);
    const u32 y = centreY + fixedMultiply(offset, acrossY) + fixedMultiply(outwards, acrossY);
    const u32 z = centreZ + fixedMultiply(offset, acrossZ) + fixedMultiply(outwards, acrossZ);

    const Float toMetres = doubleAt(guest, 0x53bd28);
    guest.setFloat(out, float(fp::mul(Float(s32(x)), toMetres)));
    guest.setFloat(out + 4, float(fp::mul(Float(s32(y)), toMetres)));
    guest.setFloat(out + 8, float(fp::mul(toMetres, Float(s32(z)))));
    // Where copying its local to `out` left esi.
    return out;
}

/* sub_493640: both rows of cones set out for a race: each cone on its slice, the rows
 * smoothed four times along the road, then each set down on the track, in the track's
 * light, or, off the road, never to stand. */
nfs::u32 frontend::rendc::conesPlace(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc0);
    const u32 place = guest.frame();                // the place on the track's record
    const u32 placeOffRoad = place + 0x6c;
    const u32 point = place + 0x78;
    const u32 fixedPoint = place + 0x84;
    const u32 sum = place + 0x90;

    for (u32 slice = 0; s32(slice) < sliceCount(guest); ++slice)
        for (u32 side = 0; side < 2; ++side)
        {
            // `point` in esi, as the game's code hands it.
            guest::direct<0x4934c0, coneEdgePoint>(app, ts, slice, side, point);
            copyDwords(guest, coneAt(guest, side, slice) + kConePosition, point, 3);
        }

    for (u32 pass = 0; pass < 4; ++pass)
    {
        // A cone becomes the mean of itself and its neighbours.  The game's "before" for
        // the second side is the first side's own slice, not the one before it.
        u32 before = guest.dword(kSliceCount) - 1;
        for (u32 slice = 0; s32(slice) < sliceCount(guest); ++slice)
            for (u32 side = 0; side < 2; ++side)
            {
                const u32 count = guest.dword(kSliceCount);
                const u32 after = s32(slice + 1) < s32(count) ? slice + 1 : slice + 1 - count;
                const u32 cone = coneAt(guest, side, slice);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, cone + kConePosition,
                                                                       coneAt(guest, side, before) + kConePosition, sum);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, sum, coneAt(guest, side, after) + kConePosition,
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

    u32 slice = 0;
    for (; s32(slice) < sliceCount(guest); ++slice)
        for (u32 side = 0; side < 2; ++side)
        {
            const u32 cone = coneAt(guest, side, slice);
            const Float toFixed = doubleAt(guest, 0x53bd38);
            for (u32 axis = 0; axis < 3; ++axis)
                guest.setDword(fixedPoint + 4 * axis,
                               u32(fp::toInteger<s32>(fp::mul(floatIn(guest, cone + kConePosition + 4 * axis), toFixed))));
            // `place` in esi, as the game's code hands it.
            fe::direct<0x420650, trackdata::placeAt>(app, ts, fixedPoint, 1, slice, place);
            const u32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, ts, place, 1);

            const Float toMetres = doubleAt(guest, 0x53bd40);
            guest.setFloat(cone + kConePosition, float(fp::mul(Float(s32(guest.dword(fixedPoint))), toMetres)));
            guest.setFloat(cone + kConePosition + 4, float(fp::mul(Float(s32(guest.dword(fixedPoint + 4))), toMetres)));
            guest.setFloat(cone + kConePosition + 8, float(fp::mul(toMetres, Float(s32(guest.dword(fixedPoint + 8))))));

            const u32 surface = fe::direct<0x4207e0, trackdata::placeSurface>(app, ts, place, 1);
            if ((surface & 0x3f) == 0 || guest.dword(placeOffRoad) != 0)
            {
                guest.setWord(cone + kConeDownTimer, 0xffff);
                continue;
            }

            // Standing: a little above the road, half way from the track's light to white.
            guest.setFloat(cone + kConePosition + 4, float(fp::add(floatIn(guest, cone + kConePosition + 4), doubleAt(guest, 0x53bd48))));
            const auto lifted = [](u32 channel) { return (((channel & 0xff) + 0xff) >> 1) & 0xff; };
            guest.setDword(cone + kConeColour, 0xff000000u | (lifted(u32(s32(colour) >> 16)) << 16)
                                                   | (lifted(u32(s32(colour) >> 8)) << 8) | lifted(colour));
            copyDwords(guest, cone + kConeHome, cone + kConePosition, 3);
            static const u32 kIdentity[9] = { 0x3f800000, 0, 0, 0, 0x3f800000, 0, 0, 0, 0x3f800000 };
            for (u32 i = 0; i < 9; ++i)
                guest.setDword(cone + kConeMatrix + 4 * i, kIdentity[i]);
        }
    return slice;
}

/* sub_493a30: the car's pass counted, good or bad: early in the race, or when the kind
 * of road is that under its middle or either edge, good. */
nfs::u32 frontend::rendc::coneCountPass(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 8);
    const u32 slice = guest.dword(car + kCarSlice);
    const u32 kind = fe::direct<0x40e0c0, aiplan::roadKind>(app, ts, slice);
    const u32 acrossBits = guest.dword(car + kCarAcross);
    const Float across = floatIn(guest, car + kCarAcross);
    const u32 halfWidthBits = guest.dword(car + kCarHalfSize);
    const Float halfWidth = floatIn(guest, car + kCarHalfSize);
    const auto countGood = [&] { guest.setDword(car + kCarConesGood, guest.dword(car + kCarConesGood) + 1); };
    if (s32(guest.dword(kTick)) < 0x480)
    {
        countGood();
        return halfWidthBits;
    }
    // What eax is left with: the last of the slices asked for.
    const auto sliceAt = [&](float offset) { return fe::direct<0x40e2b0, aiplan::roadSlice>(app, ts, slice, offset); };
    u32 left = sliceAt(floatOf(acrossBits));
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
bool insideHalf(const Guest& guest, float value, u32 half)
{
    const float size = isBelow(fp::compare(0.0, Float(value))) ? value : float(-Float(value));
    return isBelow(fp::compare(Float(size), floatIn(guest, half)));
}

/* A cone knocked: it takes the car's speed and some of its nose, and spins at random
 * for a while (kConeFlightTimer); the car skids if it was not. */
void coneKnocked(Guest& guest, nfs::ThreadState& ts, u32 car, u32 cone, float localX)
{
    const u32 impulse = guest.frame();
    const Float noseX = floatIn(guest, car + kCarMatrix), noseY = floatIn(guest, car + kCarMatrix + 4);
    const Float noseZ = floatIn(guest, car + kCarMatrix + 8);
    const Float push = doubleAt(guest, 0x53bd58);
    u32 pushX = bitsOf(float(fp::mul(noseX, push)));
    u32 pushY = bitsOf(float(fp::mul(noseY, push)));
    u32 pushZ = bitsOf(float(fp::mul(push, noseZ)));

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
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), ts, 1, impulse, car + kCarVelocity, cone + kConeVelocity);

    // Up, by the car's speed, as far as ten metres a second.
    const Float pace = fp::mul(floatIn(guest, car + kCarSpeed), doubleAt(guest, 0x53bd60));
    const Float capped = isBelowOrEqual(fp::compare(1.0, pace)) ? 1.0 : pace;
    const Float up = fp::mul(capped, doubleAt(guest, 0x53bd68));
    guest.setFloat(cone + kConeVelocity + 4, float(fp::add(up, floatIn(guest, cone + kConeVelocity + 4))));
    guest.setWord(cone + kConeFlightTimer, 0x80);

    // Its matrix turned at random about each of its rows in turn.
    const u32 matrix = cone + kConeMatrix;
    const auto randomAngle = [&] { return bitsOf(float(*fe::direct<0x4eb230, ea::randomFractionOf>(guest.app(), ts))); };
    const u32 pitch = randomAngle();
    fe::direct<0x4ea9e0, frontend::simd::rotateX>(guest.app(), ts, matrix, pitch, matrix);
    const u32 yaw = randomAngle();
    fe::direct<0x4eaab0, frontend::simd::rotateY>(guest.app(), ts, matrix, yaw, matrix);
    const u32 roll = randomAngle();
    fe::direct<0x4eab80, frontend::simd::rotateZ>(guest.app(), ts, matrix, roll, matrix);
    if (!((guest.byte(car + kCarFlags) & 1) != 0 && guest.dword(car + kCarTeam) == 3))
        guest.setDword(car + kCarConesHit, guest.dword(car + kCarConesHit) + 1);
}

}  // namespace

/* sub_493ad0: the cones within three slices of a car doing 5 m/s and up, each tested
 * against the car's box in the car's axes; those inside it knocked. */
nfs::u32 frontend::rendc::conesHit(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x74);
    if (gameSettings(guest).raceOptionCones == 0)
        return car;
    const nfs::u16 moving = fp::compare(floatIn(guest, car + kCarSpeed), doubleAt(guest, 0x53bd50));
    if (isBelow(moving))
        return withStatus(car, moving);

    const u32 fromCar = guest.frame() + 0x18;
    const u32 halfSize = guest.frame() + 0x24;      // the car's, copied
    copyDwords(guest, halfSize, car + kCarHalfSize, 3);

    for (s32 ahead = -3; ahead < 3; ++ahead)
        for (u32 side = 0; side < 2; ++side)
        {
            const u32 slice = sliceAhead(guest, guest.dword(car + kCarSlice), ahead);
            const u32 cone = coneAt(guest, side, slice);
            if (guest.word(cone + kConeDownTimer) != 0 || guest.word(cone + kConeFlightTimer) != 0)
                continue;

            fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, car + kCarPosition, cone + kConePosition, fromCar);
            const Float v0 = floatIn(guest, fromCar), v1 = floatIn(guest, fromCar + 4), v2 = floatIn(guest, fromCar + 8);
            const auto m = [&](u32 i) { return floatIn(guest, car + kCarMatrix + 4 * i); };
            const float localX = float(fp::add(fp::add(fp::mul(v0, m(0)), fp::mul(v1, m(3))), fp::mul(v2, m(6))));
            const float localY = float(fp::add(fp::add(fp::mul(v0, m(1)), fp::mul(v1, m(4))), fp::mul(v2, m(7))));
            const float localZ = float(fp::add(fp::add(fp::mul(v0, m(2)), fp::mul(v1, m(5))), fp::mul(v2, m(8))));
            if (!insideHalf(guest, localX, halfSize) || !insideHalf(guest, localY, halfSize + 4)
                || !insideHalf(guest, localZ, halfSize + 8))
                continue;
            coneKnocked(guest, ts, car, cone, localX);
        }

    if ((guest.byte(car + kCarFlags) & 1) != 0 && guest.dword(car + kCarTeam) == 3)
        return car;
    return guest::direct<0x493a30, coneCountPass>(app, ts, car);
}

/* sub_493e50: every cone back up at its home, for a restart. */
nfs::u32 frontend::rendc::conesReset(App* app, ThreadState& ts)
{
    return conesResetKeeping(app, ts, 0);
}

nfs::u32 frontend::rendc::conesResetKeeping(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    if (gameSettings(guest).raceOptionCones == 0)
        return eax;
    u32 left = guest.dword(kTick);
    guest.setDword(kConeStepTick, left);
    for (u32 slice = 0; s32(slice) < sliceCount(guest); ++slice)
        for (u32 side = 0; side < 2; ++side)
        {
            const u32 cone = coneAt(guest, side, slice);
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
nfs::u32 frontend::rendc::conesLoad(App* app, ThreadState& ts)
{
    return conesLoadKeeping(app, ts, 0);
}

nfs::u32 frontend::rendc::conesLoadKeeping(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x200);
    if (gameSettings(guest).raceOptionCones == 0)
        return eax;
    if (guest.dword(kConeRows) == 0)
    {
        for (u32 side = 0; side < 2; ++side)
        {
            guest.setDword(kConeRows + 4 * side,
                           *fe::direct<0x4e1620, ea::reserved>(app, ts, 0x53bd70, guest.dword(kSliceCount) * kConeBytes, 0));
            fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), coneRow(guest, side), guest.dword(kSliceCount) * kConeBytes);
        }
        guest::direct<0x493640, conesPlace>(app, ts);
    }
    if (guest.dword(kConeModel) == 0)
    {
        const u32 name = guest.frame();
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kConeStrings[0].address, guest::Words{0x7a2934});
        guest.setDword(kConeModel, *fe::direct<0x49d450, load::modelLoaded>(guest.app(), guest.ts(), name, 8, 0));
    }
    const u32 now = guest.dword(kTick);
    guest.setDword(kConeStepTick, now);
    return now;
}

/* sub_493fb0: the rows and the model given back as the race ends. */
nfs::u32 frontend::rendc::conesFree(App* app, ThreadState& ts)
{
    return conesFreeKeeping(app, ts, 0);
}

nfs::u32 frontend::rendc::conesFreeKeeping(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (gameSettings(guest).raceOptionCones == 0)
        return eax;
    // What eax is left with: as it came, or what the last free gave back.
    u32 left = eax;
    if (guest.dword(kConeRows) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(kConeRows));
        const u32 second = guest.dword(kConeRows + 4);
        guest.setDword(kConeRows, 0);
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, second);
        guest.setDword(kConeRows + 4, 0);
    }
    if (guest.dword(kConeModel) != 0)
    {
        guest::direct<0x49d5a0, modelFree>(app, ts, guest.dword(kConeModel));
        guest.setDword(kConeModel, 0);
        return 0;
    }
    return left;
}

/* sub_494010: every cone stepped as many ticks as have passed: a flying one under gravity
 * (bouncing at its home height, with friction), then gone for 512 ticks, then back at
 * its home. */
nfs::u32 frontend::rendc::conesStep(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    const u32 gravity = guest.frame();
    copyDwords(guest, gravity, 0x493370, 3);
    for (;;)
    {
        const u32 tick = guest.dword(kConeStepTick);
        if (s32(tick) >= s32(guest.dword(kTick)))
            return tick;
        guest.setDword(kConeStepTick, tick + 1);
        for (u32 slice = 0; s32(slice) < sliceCount(guest); ++slice)
            for (u32 side = 0; side < 2; ++side)
            {
                const u32 cone = coneAt(guest, side, slice);
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
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, cone + kConeVelocity, gravity,
                                                                       cone + kConeVelocity);
                if (isBelow(fp::compare(floatIn(guest, cone + kConePosition + 4), floatIn(guest, cone + kConeHome + 4))))
                    guest.setFloat(cone + kConeVelocity + 4,
                                   float(-fp::mul(floatIn(guest, cone + kConeVelocity + 4), doubleAt(guest, 0x53bd80))));
                const Float friction = floatIn(guest, 0x53bd88);
                const Float slowX = fp::mul(floatIn(guest, cone + kConeVelocity), friction);
                const Float slowZ = fp::mul(friction, floatIn(guest, cone + kConeVelocity + 8));
                guest.setFloat(cone + kConeVelocity, float(slowX));
                guest.setFloat(cone + kConeVelocity + 8, float(slowZ));
                const u32 matrix = cone + kConeMatrix;
                fe::direct<0x4ea9e0, frontend::simd::rotateX>(app, ts, matrix, 0x3ccccccd, matrix);
                fe::direct<0x4eaab0, frontend::simd::rotateY>(app, ts, matrix, 0xbccccccd, matrix);
                fe::direct<0x4eab80, frontend::simd::rotateZ>(app, ts, matrix, 0x3c75c28f, matrix);
            }
    }
}

/* sub_494170: the cones 20 slices either way of the view's: stepped, then each standing
 * or flying one within 100 of the view queued as the model's part for its distance, in
 * the driver's objects at its depth.  With bit 2 of [0x7a3a58] it is drawn unlit. */
nfs::u32 frontend::rendc::conesDraw(App* app, ThreadState& ts, u32 view, u32 centre)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x34);
    if (gameSettings(guest).raceOptionCones == 0)
        return view;  // eax as it came
    const u32 position = guest.frame();
    const u32 inView = position + 0x0c;
    const u32 inViewDepth = inView + 8;
    const u32 objects = position + 0x18;

    guest::direct<0x49c410, objectsBegin>(app, ts);
    guest::direct<0x494010, conesStep>(app, ts);
    for (s32 ahead = -20; ahead < 20; ++ahead)
        for (u32 side = 0; side < 2; ++side)
        {
            const u32 cone = coneAt(guest, side, sliceAhead(guest, centre, ahead));
            copyDwords(guest, position, cone + kConePosition, 3);
            if (guest.word(cone + kConeDownTimer) != 0)
                continue;
            // By address: what it leaves in edx is what the function leaves when the cone is too far.
            fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 1, position, view + 0x44, view + 0x38, inView);

            // The far test is on the float's bits, as the game has it: a negative one passes.
            const u32 depthBits = guest.dword(inViewDepth);
            const Float depth = floatIn(guest, inViewDepth);
            if (isBelow(fp::compare(depth, doubleAt(guest, 0x53bd90))) || s32(depthBits) > s32(0x42c80000))
                continue;

            guest::direct<0x49d750, modelSetShading>(app, ts, 1);
            guest::direct<0x49d8f0, modelSetColour>(app, ts, guest.dword(cone + kConeColour));
            if ((guest.byte(0x7a3a58) & 2) != 0)
            {
                guest::direct<0x49d750, modelSetShading>(app, ts, 0);
                guest::direct<0x49d8f0, modelSetColour>(app, ts, 0xffffffff);
            }

            u32 detail = 2;
            if (isBelow(fp::compare(depth, doubleAt(guest, 0x53bd98))))
                detail = 0;
            else if (s32(depthBits) < s32(0x42480000))
                detail = 1;

            const u32 model = guest.dword(kConeModel);
            const u32 spin = guest.word(cone + kConeFlightTimer) != 0 ? cone + kConeMatrix : 0;
            guest.setFloat(position + 4, float(fp::add(floatIn(guest, position + 4), floatIn(guest, model + 0x2c))));
            guest::direct<0x49de20, modelTransformPart>(app, ts, model, detail, 0, model + 0xfc + detail * 12, spin, position,
                                                        view + 0x44, view + 0x38, 0);

            // `objects` in esi, as the game's code hands it.
            guest::direct<0x49c4a0, objectsCopy>(app, ts, objects);
            const float sortDepth = float(fp::sub(depth, floatIn(guest, 0x7dd000)));
            guest::direct<0x49c420, objectsDraw>(app, ts, view, objects, bitsOf(sortDepth));
            guest::direct<0x49c410, objectsBegin>(app, ts);
        }
    return guest::direct<0x49d750, modelSetShading>(app, ts, 1);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The lightning: 240 records of 48 bytes, a bolt a tree of them
 * ------------------------------------------------------------------------- */

constexpr u32 kBoltNodes = 0x79c570;
constexpr u32 kBoltNodeBytes = 48;
constexpr u32 kBoltNodeCount = 240;
constexpr u32 kBoltAlive = 0x79f270;         // the root of the bolt alive, or 0
constexpr u32 kBoltStepTick = 0x55e4c0;
constexpr u32 kBoltLife = 0x55e4ac;          // how long a bolt lasts
constexpr u32 kBoltStepLength = 0x55e4b0;
constexpr u32 kBoltLength = 0x55e4b4;        // segments, root to tip
constexpr u32 kBoltWidth = 0x55e4b8;
constexpr u32 kBoltBend = 0x55e4bc;

// A node.
constexpr u32 kNodeLife = 0x00;              // 0 a free one
constexpr u32 kNodeIsRoot = 0x04;
constexpr u32 kNodeWidth = 0x08;
constexpr u32 kNodeAngle = 0x0c;
constexpr u32 kNodePoint = 0x10;
constexpr u32 kNodeBelow = 0x1c;             // how many nodes under it
constexpr u32 kNodeNext = 0x20;              // the node it goes on to
constexpr u32 kNodeFork = 0x24;              // the one it forks to

}  // namespace

/* sub_494390: the bolt alive's point at `out` and its brightness 0..255 at `brightness`,
 * by how much of its life is left; 1.  With none, 0. */
nfs::u32 frontend::rendc::boltGlow(App* app, ThreadState& ts, u32 out, u32 brightness)
{
    Guest guest(app, ts);
    const u32 root = guest.dword(kBoltAlive);
    if (root == 0)
    {
        return 0;
    }
    copyDwords(guest, out, root + kNodePoint, 3);
    const std::int64_t scaled = s32(guest.dword(root + kNodeLife) * 510);
    const s32 life = s32(guest.dword(kBoltLife));
    const s32 level = s32(scaled / life);
    guest.setDword(brightness, level < 0xff ? u32(level) : 0xff);
    return 1;
}

/* sub_4943e0: a free node, the last in the table; 0 when all are taken. */
nfs::u32 frontend::rendc::boltFreeNode(App* app, ThreadState& ts)
{
    const Guest guest(app, ts);
    u32 found = 0;
    for (u32 i = 0; i < kBoltNodeCount; ++i)
    {
        const u32 node = kBoltNodes + i * kBoltNodeBytes;
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
nfs::u32 frontend::rendc::boltBranch(App* app, ThreadState& ts, u32 parent, u32 slot, u32 depth, u32 forkAt, u32 bendBits)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x14);
    const float bend = floatOf(bendBits);
    if (depth == 0)
        return 0;
    const u32 made = guest::direct<0x4943e0, boltFreeNode>(app, ts);
    guest.setDword(slot, made);
    if (made == 0)
        return 0;
    guest.setDword(guest.dword(slot) + kNodeLife, guest.dword(parent + kNodeLife));
    guest.setDword(guest.dword(slot) + kNodeIsRoot, 0);

    const u32 turn = callAt(guest, kRandom) % 5;
    const Float part = fp::mul(Float(bend), doubleAt(guest, 0x53bda0));
    const Float turned = fp::mul(part, Float(s32(turn - 2)));
    guest.setFloat(guest.dword(slot) + kNodeAngle, float(fp::add(turned, floatIn(guest, parent + kNodeAngle))));

    const Float cosine = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, ts, guest.dword(guest.dword(slot) + kNodeAngle));
    const Float alongX = fp::mul(cosine, floatIn(guest, kBoltStepLength));
    guest.setFloat(guest.dword(slot) + kNodePoint, float(fp::add(alongX, floatIn(guest, parent + kNodePoint))));
    const Float sine = fe::direct<0x4ea930, frontend::simd::sineValue>(app, ts, guest.dword(guest.dword(slot) + kNodeAngle));
    const Float alongZ = fp::mul(sine, floatIn(guest, kBoltStepLength));
    guest.setFloat(guest.dword(slot) + kNodePoint + 8, float(fp::add(alongZ, floatIn(guest, parent + kNodePoint + 8))));
    guest::direct<0x47adc0, frontend::renda::pointHeight>(app, ts, guest.dword(slot) + kNodePoint);

    const Float gone = fp::div(Float(s32(depth)), Float(s32(guest.dword(kBoltLength))));
    const Float width = fp::mul(fp::sub(1.0, gone), floatIn(guest, kBoltWidth));
    guest.setFloat(guest.dword(slot) + kNodeWidth, float(width));
    guest.setDword(guest.dword(slot) + kNodeBelow, 0);

    const bool forks = depth == forkAt && callAt(guest, kRandom) % forkAt != 0;
    // What the last of the branches below gave back, which the game's code adds from eax.
    u32 node, last;
    if (forks)
    {
        FloatMath math(guest);
        const u32 first = guest.dword(slot);
        const Float at = Float(s32(forkAt));
        const u32 nextFork = u32(math.truncated(fp::mul(at, doubleAt(guest, 0x53bda8))));
        const u32 below =
            guest::direct<0x494420, boltBranch>(app, ts, first, first + kNodeNext, forkAt - 1, nextFork, guest.dword(kBoltBend));
        const Float wider = fp::mul(floatIn(guest, kBoltBend), floatIn(guest, 0x53bdb0));
        const Float forkAtFork = fp::mul(at, doubleAt(guest, 0x53bdb8));
        guest.setDword(first + kNodeBelow, guest.dword(first + kNodeBelow) + below);
        const u32 forkFork = u32(math.truncated(forkAtFork));
        node = guest.dword(slot);
        last = guest::direct<0x494420, boltBranch>(app, ts, node, node + kNodeFork, u32(s32(depth) >> 1), forkFork,
                                                    bitsOf(float(wider)));
    }
    else
    {
        node = guest.dword(slot);
        last = guest::direct<0x494420, boltBranch>(app, ts, node, node + kNodeNext, depth - 1, forkAt, guest.dword(kBoltBend));
    }
    guest.setDword(node + kNodeBelow, guest.dword(node + kNodeBelow) + last);
    return 1;
}

/* sub_4945a0: a bolt struck at a point (three floats, then its angle, on the stack): its
 * root and its tree; it is now the bolt alive.  What is left in eax is what the tree said. */
nfs::u32 frontend::rendc::boltStrike(App* app, ThreadState& ts, u32 x, u32 y, u32 z, u32 angle)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const u32 root = guest::direct<0x4943e0, boltFreeNode>(app, ts);
    if (root == 0)
        return 0;
    FloatMath math(guest);
    const Float forkAt = fp::mul(Float(s32(guest.dword(kBoltLength))), doubleAt(guest, 0x53bdc0));
    guest.setDword(root + kNodeLife, guest.dword(kBoltLife));
    guest.setDword(root + kNodeIsRoot, 1);
    guest.setDword(root + kNodePoint, x);
    guest.setDword(root + kNodePoint + 4, y);
    guest.setDword(root + kNodePoint + 8, z);
    guest.setDword(root + kNodeAngle, angle);
    const u32 fork = u32(math.truncated(forkAt));
    guest.setDword(root + kNodeWidth, 0);
    guest.setDword(root + kNodeBelow, 0);
    const u32 made =
        guest::direct<0x494420, boltBranch>(app, ts, root, root + kNodeNext, guest.dword(kBoltLength), fork, guest.dword(kBoltBend));
    guest.setDword(kBoltAlive, root);
    guest.setDword(root + kNodeBelow, guest.dword(root + kNodeBelow) + made);
    return made;
}

/* sub_494630: every live node stepped as many ticks as have passed: nearer its end, and
 * moved a step of its width one way or the other in x and z.  The bolt alive is gone when
 * its root's life is. */
nfs::u32 frontend::rendc::boltsStep(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    for (;;)
    {
        const u32 tick = guest.dword(kBoltStepTick);
        if (s32(tick) >= s32(guest.dword(kTick)))
            return tick;
        guest.setDword(kBoltStepTick, tick + 1);
        for (u32 i = 0; i < kBoltNodeCount; ++i)
        {
            const u32 node = kBoltNodes + i * kBoltNodeBytes;
            if (guest.dword(node + kNodeLife) == 0)
                continue;
            const s32 wayX = s32(callAt(guest, kRandom) % 3) - 1;
            const s32 wayZ = s32(callAt(guest, kRandom) % 3) - 1;
            const Float stepX = fp::mul(Float(wayX), floatIn(guest, node + kNodeWidth));
            const u32 life = guest.dword(node + kNodeLife) - 1;
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

nfs::u32 frontend::rendc::backImage(App* app, ThreadState& ts, u32 eax, u32 edx, u32 ebx, u32 ecx)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x100);
    const u32 path = guest.frame();
    fe::direct<0x4daf80, port::draw2d::resetScreens>(app, ts, ts.sp - 4, port::Args{ eax, edx, ebx, ecx });
    guest.copyString(path, 0x7a3134);
    callAt(guest, kAppendBackslash, args({ path }));
    const u32 number = fe::direct<0x478290, frontend::loada::screenSizeNumber>(app, ts);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path + guest.length(path), kBackImageStrings[0].address, guest::Words{number});
    const u32 file = fe::direct<0x4956f0, draw::loadQfsFlags>(app, ts, path, 0x10u);
    if (file == 0)
        return 0;
    const u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, file, kBackImageStrings[1].address);
    // The game's code hands it 0 in ebx too, which it does not read.
    guest.setDword(0x8b7638, u32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, ts, ts.sp - 4, shape, 0, ecx)));
    fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, ts);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, std::int32_t(guest.dword(0x8b7638)), 0, 0);
    fe::direct<0x4d9140, graphics::flipAndWait>(app, ts);
    fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, ts);
    fe::direct<0x4d7990, port::shapes::drawShapeOf>(app, ts, ts.sp - 4, std::int32_t(guest.dword(0x8b7638)), 0, 0);
    const u32 freed = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
    return u32(*fe::direct<0x4daf80, port::draw2d::resetScreens>(app, ts, ts.sp - 4,
                                                                    port::Args{ freed, 0x10, ebx, ecx }));
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
nfs::u32 frontend::rendc::blinkColour(App* app, ThreadState& ts, u32 light)
{
    const Guest guest(app, ts);
    if (gameSettings(guest).effectsLevel == 2)
        return 0;
    const u32 record = 0x55e3dc + light * 12;
    if (guest.word(record + 4) != 0)
    {
        const u32 tick = guest.dword(kTick) + guest.byte(record + 7);
        if (((s32(tick) >> (guest.byte(record + 6) & 31)) & 1) != 0)
            return 0;
    }
    return guest.dword(record);
}

/* sub_4964d0: the float at `value` cut to its fraction, above 1 and below 0; the NaN of
 * those bits (either sign) becomes 0.  Between them it stays. */
nfs::u32 frontend::rendc::wrapFraction(App* app, ThreadState& ts, u32 value)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0x10);
    const u32 bits = guest.dword(value);
    if ((bits | 0x80000000u) == 0xffc00000u)
    {
        guest.setDword(value, 0);
        return 0xffc00000u;
    }
    // eax goes on as it was, and the double on the stack.
    double result = 0.0;  // what the callee gives back on the x87's top
    const auto call = [&](u32 function, u32 eax, Float argument) {
        const double wide = argument;
        std::uint64_t pieces;
        std::memcpy(&pieces, &wide, sizeof pieces);
        return callAt(guest, function, args({ eax }, { u32(pieces), u32(pieces >> 32) }), 0, &result);
    };
    if (s32(bits) > 0x3f800000)
    {
        const u32 left = call(kFloor, bits, floatIn(guest, value));
        guest.setFloat(value, float(fp::sub(floatIn(guest, value), result)));
        return left;
    }
    const nfs::u16 status = fp::compare(0.0, Float(floatOf(bits)));
    if (isBelowOrEqual(status))
        return withStatus(bits, status);
    const float size = float(-Float(floatOf(bits)));
    const u32 left = call(kCeil, withStatus(bits, status), Float(size));
    guest.setFloat(value, float(fp::add(result, floatIn(guest, value))));
    return left;
}

/* sub_496550: a rough distance between two points of 16.16 numbers. */
nfs::u32 frontend::rendc::approximateDistance(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    const auto size = [](u32 value) { return s32(value) > 0 ? value : 0 - value; };
    const u32 x = size(guest.dword(a) - guest.dword(b));
    const u32 y = size(guest.dword(a + 4) - guest.dword(b + 4));
    u32 z = size(guest.dword(a + 8) - guest.dword(b + 8));
    u32 flat = s32(x) > s32(y) ? u32(s32(y) >> 2) + x : u32(s32(x) >> 2) + y;
    if (s32(flat) > s32(z))
        z = u32(s32(z) >> 2);
    else
        flat = u32(s32(flat) >> 2);
    return flat + z;
}

/* sub_496bf0: x and y turned by a fraction of a whole turn, to two floats. */
nfs::u32 frontend::rendc::rotatePlane(App* app, ThreadState& ts, u32 xBits, u32 yBits, u32 turn, u32 outX, u32 outY)
{
    Guest guest(app, ts);
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
nfs::u32 frontend::rendc::withinFlatDistance(App* app, ThreadState& ts, u32 a, u32 b, u32 limitBits)
{
    const Guest guest(app, ts);
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

constexpr u32 kObjectsFirst = 0x7a3d08;
constexpr u32 kObjectsChain = 0x7a3d0c;
constexpr u32 kObjectNodeWords = 0x21;
constexpr u32 kObjectNodeBytes = 0x84;
constexpr u32 kObjectVertexWords = 8;
constexpr u32 kObjectVertexBytes = 0x20;

}  // namespace

/* sub_49c420: the objects at `pair` drawn at a depth, the mirror's way when [0x7a3a60] says;
 * then the new pair of ends back into `pair`. */
nfs::u32 frontend::rendc::objectsDraw(App* app, ThreadState& ts, u32 view, u32 pair, u32 depth)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 8);
    if (guest.dword(0x7a3a60) == 0)
        fe::direct<0x4c7400, frontend::rende::depthBucketAppend>(app, ts, view, guest.dword(pair), guest.dword(pair + 4), depth);
    else
        fe::direct<kDrawMirroredRecords, collide::listDrawnMirrored>(guest.app(), guest.ts(), view, guest.dword(pair));
    const u32 result = fe::direct<0x4bbe70, frontend::rendd::emptyList>(app, ts, guest.frame());
    copyDwords(guest, pair, guest.frame(), 2);
    return result;
}

/* sub_49c4a0: the driver's chain of objects copied to fresh memory, with their vertices;
 * the copy's two ends at `out`. */
nfs::u32 frontend::rendc::objectsCopy(App* app, ThreadState& ts, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x18);
    const u32 frame = guest.frame();
    const u32 made = frame + 8;
    u32 first = 0, last = 0;
    const u32 start = guest.dword(kObjectsFirst);
    if (start != 0)
        guest.setDword(start, 0);
    for (u32 from = guest.dword(kObjectsChain); from != 0; from = guest.dword(from))
    {
        fe::direct<0x4bbde0, frontend::rendd::arenaTake>(app, ts, made, 0x104);
        const u32 copy = guest.dword(made);
        copyDwords(guest, copy, from, kObjectNodeWords);
        if (first != 0)
            guest.setDword(last, copy);
        else
            first = copy;
        last = copy;
        const u32 vertices = copy + kObjectNodeBytes;
        guest.setDword(copy + 8, vertices);
        guest.setDword(copy + 0xc, vertices + kObjectVertexBytes);
        guest.setDword(copy + 0x10, vertices + 2 * kObjectVertexBytes);
        copyDwords(guest, vertices, guest.dword(from + 8), kObjectVertexWords);
        copyDwords(guest, vertices + kObjectVertexBytes, guest.dword(from + 0xc), kObjectVertexWords);
        copyDwords(guest, vertices + 2 * kObjectVertexBytes, guest.dword(from + 0x10), kObjectVertexWords);
    }
    guest.setDword(out, first);
    guest.setDword(out + 4, last);
    return out;
}

namespace
{

/* ---------------------------------------------------------------------------
 * A model drawn
 * ------------------------------------------------------------------------- */

constexpr u32 kModelShading = 0x55e69c;      // 1: its vertices lit by their normals
constexpr u32 kModelLight = 0x55e6a0;
constexpr u32 kModelRed = 0x55e6ac;
constexpr u32 kModelGreen = 0x55e6b0;
constexpr u32 kModelBlue = 0x55e6b4;
constexpr u32 kModelDarkRed = 0x55e6b8;
constexpr u32 kModelDarkGreen = 0x55e6bc;
constexpr u32 kModelDarkBlue = 0x55e6c0;
constexpr u32 kModelBrightRed = 0x55e6c4;
constexpr u32 kModelBrightGreen = 0x55e6c8;
constexpr u32 kModelBrightBlue = 0x55e6cc;
constexpr u32 kModelRangeRed = 0x55e6d0;
constexpr u32 kModelRangeGreen = 0x55e6d4;
constexpr u32 kModelRangeBlue = 0x55e6d8;
constexpr u32 kModelAlphaHigh = 0x55e6e4;
constexpr u32 kModelAlphaLow = 0x55e6e8;
constexpr u32 kEnvironmentMatrix = 0x55e6ec;

// A model's record.
constexpr u32 kModelPositions = 0x10;        // offsets from kModelData
constexpr u32 kModelNormals = 0x14;
constexpr u32 kModelVertices = 0x1c;
constexpr u32 kModelCoords = 0x24;
constexpr u32 kModelParts = 0xf8;
constexpr u32 kModelPartPlaces = 0xfc;       // 12 bytes a part
constexpr u32 kModelPartFirst = 0x3fc;       // a part's first vertex
constexpr u32 kModelPartCount = 0x4fc;       // and how many
constexpr u32 kModelData = 0x1f04;

/* The arguments of sub_49de20: three steps of transform, any of them absent. */
struct Chain
{
    u32 matrix0, place0;
    u32 matrix1, place1;
    u32 matrix2, place2;
    u32 environment;
};

}  // namespace

/* sub_49de20: a part's vertices transformed and projected, and its polygons culled: the
 * nearest depth, which its thunk leaves on the x87 (0 for none). */
frontend::rendc::Nearest frontend::rendc::modelTransformPart(App* app, ThreadState& ts, u32 model, u32 part,
                                                             u32 givenMatrix0, u32 givenPlace0, u32 givenMatrix1,
                                                             u32 givenPlace1, u32 givenMatrix2, u32 givenPlace2,
                                                             u32 environment)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 0xe0);
    const Chain chain{ givenMatrix0, givenPlace0, givenMatrix1, givenPlace1, givenMatrix2, givenPlace2, environment };
    const u32 frame = guest.frame();
    const u32 allThree = frame + 0x00;       // the game first copies 0x49c3e0 here: the product overwrites it
    const u32 firstTwo = frame + 0x24;
    const u32 forEnvironment = frame + 0x48;
    const u32 identity = frame + 0x6c;
    const u32 once = frame + 0x90;
    const u32 zero = frame + 0x9c;
    const u32 placed = frame + 0xa8;

    const u32 redGreen = (guest.dword(kModelGreen) << 8) | (guest.dword(kModelRed) << 16) | 0xff000000u;
    const u32 colour = guest.dword(kModelBlue) | redGreen;
    copyDwords(guest, identity, 0x564498, 9);
    copyDwords(guest, zero, 0x56448c, 3);

    // Left as the game leaves them on the ways out without a part.
    if (model == 0)
    {
        return { redGreen, 0.0 };
    }
    if (s32(part) >= s32(guest.dword(model + kModelParts)))
    {
        return { part, 0.0 };
    }

    const u32 count = guest.dword(model + part * 4 + kModelPartCount);
    const u32 first = guest.dword(model + part * 4 + kModelPartFirst);
    const u32 data = model + kModelData;
    const u32 positions = guest.dword(model + kModelPositions) + data + first * 12;
    const u32 normals = guest.dword(model + kModelNormals) + data + first * 12;
    const u32 vertices = guest.dword(model + kModelVertices) + data + first * 32;
    const u32 coords = guest.dword(model + kModelCoords) + data + first * 12;

    const auto matrixOr = [&](u32 matrix) { return matrix != 0 ? matrix : identity; };
    const auto placeOr = [&](u32 place) { return place != 0 ? place : zero; };
    const u32 matrix0 = matrixOr(chain.matrix0), matrix1 = matrixOr(chain.matrix1), matrix2 = matrixOr(chain.matrix2);
    const u32 place0 = placeOr(chain.place0), place1 = placeOr(chain.place1), place2 = placeOr(chain.place2);

    fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 1, place0, matrix1, place1, once);
    fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 1, once, matrix2, place2, placed);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, matrix0, matrix1, firstTwo);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, firstTwo, matrix2, allThree);

    // The two words the game's call pushes, where it pushes them, for what reads this frame's leftovers
    // after it (sub_4b8740, after rendd's calls by address).
    guest.setDword(ts.sp - 8, placed);
    guest.setDword(ts.sp - 4, colour);
    const float nearest = fe::direct<0x49d9d0, vertices::colouredNearestZ>(app, ts, s32(count), positions, vertices,
                                                                            allThree, placed, colour);

    const u32 shading = guest.dword(kModelShading);
    if (shading == 1)
        callAt(guest, kShadeNormals, args({ model, count, normals, coords }, { vertices, firstTwo }));
    if (chain.environment != 0)
    {
        fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, firstTwo, kEnvironmentMatrix, forEnvironment);
        callAt(guest, kEnvironmentCoords, args({ count, normals, coords, vertices }, { forEnvironment }));
    }
    callAt(guest, kCullPolygons, args({ model, part, chain.environment }));
    return { bitsOf(nearest), double(nearest) };
}

/* sub_49d5c0: all a model's parts, each at its place in the model: the nearest depth among
 * them, which its thunk leaves on the x87.  Its registers and stack go straight on to sub_49de20's
 * stack. */
frontend::rendc::Nearest frontend::rendc::modelTransformAll(App* app, ThreadState& ts, u32 model, u32 matrix1,
                                                            u32 place1, u32 matrix2, u32 place2, u32 environment)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x14);
    u32 left = model;
    u32 nearest = 0x477fff00;
    for (u32 part = 0; s32(part) < s32(guest.dword(model + kModelParts)); ++part)
    {
        const u32 partPlace = model + kModelPartPlaces + part * 12;
        const Nearest returned = guest::direct<0x49de20, modelTransformPart>(app, ts, model, part, 0, partPlace, matrix1,
                                                                            place1, matrix2, place2, environment);
        const Float depth = returned.depth;
        const nfs::u16 status = fp::compare(depth, Float(floatOf(nearest)));
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
nfs::u32 frontend::rendc::modelSetEnvironment(App* app, ThreadState& ts, u32 direction)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x54);
    const u32 yaw = guest.frame() + 0x48;
    const u32 flat = guest.frame() + 0x4c;
    const u32 pitch = guest.frame() + 0x50;
    const u32 yawMatrix = guest.frame() + 0x24;
    const u32 pitchMatrix = guest.frame();

    const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, ts, guest.dword(direction),
                                                                                  guest.dword(direction + 8));
    const Float z = floatIn(guest, direction + 8);
    const Float x = floatIn(guest, direction);
    const Float squares = fp::add(fp::mul(z, z), fp::mul(x, x));
    guest.setFloat(yaw, float(fp::add(heading, floatIn(guest, 0x53c68c))));
    guest.setFloat(flat, float(fe::direct<0x4ea920, frontend::simc::squareRootOf>(app, ts, bitsOf(float(squares)))));
    const Float tilt = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, ts, guest.dword(flat),
                                                                               guest.dword(direction + 4));
    guest.setFloat(pitch, float(fp::add(tilt, doubleAt(guest, 0x53c690))));
    // The angles by their places in the frame, which the game's code pushes copies of.
    fe::direct<0x4dfde0, ea::rotateAboutY>(app, ts, yawMatrix, yaw);
    fe::direct<0x4dfd80, ea::rotateAboutX>(app, ts, pitchMatrix, pitch);
    // By address: the eax it leaves is this function's answer, and its core hands back none.
    return callAt(guest, kMultiplyMatrices, args({ yawMatrix, pitchMatrix, kEnvironmentMatrix }));
}

/* sub_49d8f0: the colour a model is lit from (0xrrggbb): from a third of it (half, for
 * blue) to twice it. */
nfs::u32 frontend::rendc::modelSetColour(App* app, ThreadState& ts, u32 colour)
{
    Guest guest(app, ts);
    const u32 red = u32(s32(colour) >> 16) & 0xff;
    const u32 green = u32(s32(colour) >> 8) & 0xff;
    const u32 blue = colour & 0xff;
    const auto twice = [](u32 channel) { return channel + channel > 0xff ? u32(0xff) : channel + channel; };
    guest.setDword(kModelDarkRed, red / 3);
    guest.setDword(kModelDarkGreen, green / 3);
    guest.setDword(kModelDarkBlue, blue >> 1);
    guest.setDword(kModelBrightRed, twice(red));
    guest.setDword(kModelBrightGreen, twice(green));
    guest.setDword(kModelBrightBlue, twice(blue));
    guest.setDword(kModelRangeRed, twice(red) - red / 3);
    guest.setDword(kModelRangeGreen, twice(green) - green / 3);
    const u32 rangeBlue = twice(blue) - (blue >> 1);
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

constexpr u32 kRandomIndex = 0x57126b;       // a byte
constexpr u32 kRandomTable = 0x552938;       // 256 bytes
constexpr u32 kSpriteCycle = 0x55e984;
constexpr u32 kSpriteKinds = 0x7a1b90;
constexpr u32 kSpriteX = 0x55e95c;
constexpr u32 kSpriteY = 0x7a1ac0;
constexpr u32 kSpriteSpeedX = 0x7a1ae8;
constexpr u32 kSpriteSpeedY = 0x7a1b10;
constexpr u32 kSpriteKind = 0x7a1b38;
constexpr u32 kSpriteScale = 0x7a1b60;
constexpr u32 kBigSprite = 9;

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
nfs::u32 frontend::rendc::skyRandomBetween(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    const u32 low = s32(b) < s32(a) ? b : a;
    const u32 high = s32(b) < s32(a) ? a : b;
    const s32 range = s32(high - low + 1);
    const s32 value = nextRandomByte(guest);
    const u32 remainder = u32(value % range);
    return low + remainder;
}

/* sub_4a40c0: a sky sprite put back in at an edge of the screen: from one of the four
 * sides at random, moving across; the big one, the ninth, always in one place, its kinds
 * taken in turn.  Each comes back a fifth smaller, as if further away. */
nfs::u32 frontend::rendc::skySpritePlace(App* app, ThreadState& ts, u32 sprite)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const u32 at = sprite * 4;
    const auto oneWayOrOther = [&] { return (nextRandomByte(guest) & 1) != 0 ? u32(1) : u32(0xffffffff); };
    const auto between = [&](u32 a, u32 b) { return guest::direct<0x4a4060, skyRandomBetween>(app, ts, a, b); };
    guest.setDword(kSpriteKinds, 6);
    u32 left = 0;
    if (sprite == kBigSprite)
    {
        const u32 cycle = guest.dword(kSpriteCycle);
        guest.setDword(kSpriteKind + at, cycle);
        guest.setDword(kSpriteScale + at, 0x3fcccccd);
        guest.setDword(kSpriteY + at, 0x3c);
        guest.setDword(kSpriteCycle, u32(s32(cycle + 1) % 6));
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
        const s32 value = nextRandomByte(guest);
        const s32 kinds = s32(guest.dword(kSpriteKinds));
        guest.setDword(kSpriteKind + at, u32(value % kinds));
        left = u32(value / kinds);
    }
    guest.setFloat(kSpriteScale + at, float(fp::mul(floatIn(guest, kSpriteScale + at), doubleAt(guest, 0x53cf04))));
    return left;
}

/* sub_494360: the bolts' table cleared, none alive, their clock at 0. */
nfs::u32 frontend::rendc::boltsClear(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 result = fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kBoltNodes, kBoltNodeCount * kBoltNodeBytes);
    guest.setDword(kBoltStepTick, 0);
    guest.setDword(kBoltAlive, 0);
    return result;
}

/* sub_49c410: the driver's objects started afresh. */
nfs::u32 frontend::rendc::objectsBegin(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, ts);
}

/* sub_49c470: the driver's objects' two ends taken, to `out` (esi). */
nfs::u32 frontend::rendc::objectsEnd(App* app, ThreadState& ts, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 8);
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, ts, guest.frame());
    copyDwords(guest, out, guest.frame(), 2);
    return out;
}

/* sub_49d5a0: a model's data and the model given back. */
nfs::u32 frontend::rendc::modelFree(App* app, ThreadState& ts, u32 model)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 data = guest.dword(model);
    if (data != 0)
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, data);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, model);
}

/* sub_49d720: the alphas a model's vertices are drawn with, the high one and the low. */
nfs::u32 frontend::rendc::modelSetAlphas(App* app, ThreadState& ts, u32 high, u32 low)
{
    Guest guest(app, ts);
    guest.setDword(kModelAlphaHigh, (high & 0xff) << 24);
    const u32 alpha = ((low & 0xff) << 24) | 0xffffff;
    guest.setDword(kModelAlphaLow, alpha);
    return alpha;
}

/* sub_49d750: whether a model's vertices are lit by their normals. */
nfs::u32 frontend::rendc::modelSetShading(App* app, ThreadState& ts, u32 shading)
{
    Guest guest(app, ts);
    guest.setDword(kModelShading, shading);
    return shading;
}

/* sub_49d760: the direction a model is lit from, three floats at `light`. */
nfs::u32 frontend::rendc::modelSetLight(App* app, ThreadState& ts, u32 light)
{
    Guest guest(app, ts);
    copyDwords(guest, kModelLight, light, 3);
    return light;
}

/* ---------------------------------------------------------------------------
 * As the game calls them
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::rendBlinkColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_491120", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 light = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::blinkColour(app, ts, light); });
}

nfs::u32 frontend::rendConeSideOffset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493380", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 slice = args.eax, kind = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::coneSideOffset(app, ts, slice, kind); });
}

nfs::u32 frontend::rendConeEdgePoint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4934c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const u32 slice = args.eax, side = args.edx, out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::coneEdgePoint(app, ts, slice, side, out); });
}

nfs::u32 frontend::rendConesPlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493640", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesPlace(app, ts); });
}

nfs::u32 frontend::rendConeCountPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493a30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::coneCountPass(app, ts, car); });
}

nfs::u32 frontend::rendConesHit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493ad0", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesHit(app, ts, car); });
}

nfs::u32 frontend::rendConesReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493e50", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesResetKeeping(app, ts, args.eax); });
}

nfs::u32 frontend::rendConesLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493ef0", 0, 0, kConeStrings, std::size(kConeStrings), fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesLoadKeeping(app, ts, args.eax); });
}

nfs::u32 frontend::rendConesFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_493fb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesFreeKeeping(app, ts, args.eax); });
}

nfs::u32 frontend::rendConesStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494010", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesStep(app, ts); });
}

nfs::u32 frontend::rendConesDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494170", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax, slice = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::conesDraw(app, ts, view, slice); });
}

nfs::u32 frontend::rendBoltsClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494360", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltsClear(app, ts); });
}

nfs::u32 frontend::rendBoltGlow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494390", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 out = args.eax, brightness = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltGlow(app, ts, out, brightness); });
}

nfs::u32 frontend::rendBoltFreeNode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4943e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltFreeNode(app, ts); });
}

nfs::u32 frontend::rendBoltBranch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494420", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 parent = args.eax, slot = args.edx, depth = args.ebx, forkAt = args.ecx;
    const u32 bend = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltBranch(app, ts, parent, slot, depth, forkAt, bend); });
}

nfs::u32 frontend::rendBoltStrike(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4945a0", 16, 0, nullptr, 0 };
    const u32 x = stackArgument(app, ts, 0), y = stackArgument(app, ts, 1), z = stackArgument(app, ts, 2);
    const u32 angle = stackArgument(app, ts, 3);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltStrike(app, ts, x, y, z, angle); });
}

nfs::u32 frontend::rendBoltsStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_494630", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::boltsStep(app, ts); });
}

nfs::u32 frontend::rendBackImage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_495600", 0, 0, kBackImageStrings, std::size(kBackImageStrings) };
    const u32 eax = args.eax, edx = args.edx, ebx = args.ebx, ecx = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::backImage(app, ts, eax, edx, ebx, ecx); });
}

nfs::u32 frontend::rendWrapFraction(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4964d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 value = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::wrapFraction(app, ts, value); });
}

nfs::u32 frontend::rendApproxDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_496550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::approximateDistance(app, ts, a, b); });
}

nfs::u32 frontend::rendRotatePlane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_496bf0", 20, 0, nullptr, 0 };
    const u32 x = stackArgument(app, ts, 0), y = stackArgument(app, ts, 1), turn = stackArgument(app, ts, 2);
    const u32 outX = stackArgument(app, ts, 3), outY = stackArgument(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::rotatePlane(app, ts, x, y, turn, outX, outY); });
}

nfs::u32 frontend::rendWithinFlatDistance(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_496f20", 4, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 a = args.eax, b = args.edx;
    const u32 limit = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::withinFlatDistance(app, ts, a, b, limit); });
}

nfs::u32 frontend::rendObjectsBegin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c410", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::objectsBegin(app, ts); });
}

nfs::u32 frontend::rendObjectsDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c420", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax, pair = args.edx;
    const u32 depth = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::objectsDraw(app, ts, view, pair, depth); });
}

nfs::u32 frontend::rendObjectsEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c470", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::objectsEnd(app, ts, out); });
}

nfs::u32 frontend::rendObjectsCopy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c4a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::objectsCopy(app, ts, out); });
}

nfs::u32 frontend::rendModelFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d5a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 model = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelFree(app, ts, model); });
}

std::optional<double> frontend::rendc::modelTransformAllCalled(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d5c0", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 model = args.eax, matrix1 = args.edx, place1 = args.ebx, matrix2 = args.ecx;
    const u32 place2 = stackArgument(app, ts, 0), environment = stackArgument(app, ts, 1);
    std::optional<double> depth;
    fe::runThunk(app, ts, args, site, [&](Guest&) {
        const Nearest nearest = modelTransformAll(app, ts, model, matrix1, place1, matrix2, place2, environment);
        depth = nearest.depth;
        return nearest.eax;
    });
    return depth;
}

nfs::u32 frontend::rendModelTransformAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<double> depth = rendc::modelTransformAllCalled(app, ts, args);
    if (depth)
    {
        args.floatSet = true;
        args.floatResult = double(*depth);
    }
    return args.eax;
}

nfs::u32 frontend::rendModelSetAlphas(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d720", 0, 0, nullptr, 0, 0 };
    const u32 high = args.eax, low = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelSetAlphas(app, ts, high, low); });
}

nfs::u32 frontend::rendModelSetShading(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d750", 0, 0, nullptr, 0, 0 };
    const u32 shading = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelSetShading(app, ts, shading); });
}

nfs::u32 frontend::rendModelSetLight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d760", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 light = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelSetLight(app, ts, light); });
}

nfs::u32 frontend::rendModelSetEnvironment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const u32 direction = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelSetEnvironment(app, ts, direction); });
}

nfs::u32 frontend::rendModelSetColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49d8f0", 0, 0, nullptr, 0 };
    const u32 colour = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::modelSetColour(app, ts, colour); });
}

std::optional<double> frontend::rendc::modelTransformPartCalled(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49de20", 20, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 model = args.eax, part = args.edx, matrix0 = args.ebx, place0 = args.ecx;
    const u32 matrix1 = stackArgument(app, ts, 0), place1 = stackArgument(app, ts, 1);
    const u32 matrix2 = stackArgument(app, ts, 2), place2 = stackArgument(app, ts, 3);
    const u32 environment = stackArgument(app, ts, 4);
    std::optional<double> depth;
    fe::runThunk(app, ts, args, site, [&](Guest&) {
        const Nearest nearest = modelTransformPart(app, ts, model, part, matrix0, place0, matrix1, place1, matrix2,
                                                   place2, environment);
        depth = nearest.depth;
        return nearest.eax;
    });
    return depth;
}

nfs::u32 frontend::rendModelTransformPart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<double> depth = rendc::modelTransformPartCalled(app, ts, args);
    if (depth)
    {
        args.floatSet = true;
        args.floatResult = double(*depth);
    }
    return args.eax;
}

nfs::u32 frontend::rendSkyRandomBetween(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a4060", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::skyRandomBetween(app, ts, a, b); });
}

nfs::u32 frontend::rendSkySpritePlace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a40c0", 0, 0, nullptr, 0 };
    const u32 sprite = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendc::skySpritePlace(app, ts, sprite); });
}

}
