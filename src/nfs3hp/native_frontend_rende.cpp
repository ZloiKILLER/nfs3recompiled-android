#include "native_frontend_rende.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_collide.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_loada.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_sime.h"
#include "native_graphics.h"
#include "native_vertices.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <cstring>
#include <iterator>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
nfs::u32 rendSkidAddSegment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkidMarksDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendPolygonDepths(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendPolygonsIntoBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendPolygonsSorted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendClearDepthBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDepthBucketAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDepthBucketsAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDepthBucketsGather(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendClearFarBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFarBucketsGather(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendClearBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDrawListAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBlinkingQuads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFadeParticles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendLoadParticleTextures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendClearParticles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendClearParticleStates(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendParticlesCameraChanged(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendParticlesPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDropsPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFormatBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendExpandPalettedShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendLoadShapeTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFitPowerOfTwo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCopyCabinBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendLoadEffectTextures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendLoadEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkidSubdivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkidStrip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSkidWheel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDefaultFieldOfView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSmokeEmit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSparksPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}
}

/* What the race's drawing does around the port's vertex and polygon code,
 * decompiled:
 *
 *   the skid marks         a wheel's sliding laid down as segments, shared out
 *                          among 64 strips of 16 (sub_4db940, sub_4db8b0,
 *                          sub_4db750, sub_4c6cb0), and drawn (sub_4c6f00);
 *   the depth buckets      polygons sorted by depth without the card's z-buffer
 *                          (sub_4c7180, sub_4c7260, sub_4c7380, sub_4c7400,
 *                          sub_4c7480, sub_4c7580, sub_4c76e0, sub_4c7940), and
 *                          the frame's clearing of them;
 *   the blinking quads     eight lights on the track (sub_4c9f70);
 *   the particles          their fade, their tables cleared, a camera's change
 *                          (sub_4cb220, sub_4cb6f0, sub_4cb730, sub_4cc540),
 *                          and the race frame's passes over them, the drops' and
 *                          the sparks' (sub_4cc5c0, sub_4cc600, sub_4dedf0);
 *   the smoke              a puff at a point of a view's (sub_4dd930);
 *   the textures           a shape made a texture (sub_4d4080), an 8-bit shape
 *                          widened (sub_4d3380), a bar of the cabin's strip
 *                          (sub_4d4440), and sfx.fsh's load (sub_4d5370,
 *                          sub_4cb590, sub_4d56f0).
 *
 * Their state stays in guest memory, where the port's functions read it.
 *
 * Plan step 5: each function a typed core in frontend::rende (native_frontend_rende.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * this round's three files (rendd, rende, loada) calls another as C++ (guest::direct), and the
 * typed cores of other files too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* The port's View Angle (the port's View Angle change), in nfs3hp_main.cpp. */
nfs::u32 viewAngleFor(nfs::App* app, nfs::u32 angle);

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using nfs::u32;
using nfs::s32;
using std::uint8_t;

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

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

/* A register's low byte and second byte, the rest as they are: what a function
 * leaves in the registers its callers do not count on. */
constexpr u32 withLow(u32 value, uint8_t low)
{
    return (value & 0xffffff00u) | low;
}

constexpr u32 withHigh(u32 value, uint8_t high)
{
    return (value & 0xffff00ffu) | (u32(high) << 8);
}

/* A float from its bits, as a stack argument holds it. */
float floatOfBits(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A float of the game's, as fld dword reads it. */
double floatOf(const Guest& guest, u32 address)
{
    return guest.floatAt(address);
}

/* A double of the game's constants, as fld and fmul qword read it. */
double doubleOf(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* What a jump after fcomp a, b takes: the word's c3, c2 and c0, unordered all of them. */
bool jumpsBelow(double a, double b)            // jb
{
    return (fp::compare(a, b) & 0x100) != 0;
}

bool jumpsBelowOrEqual(double a, double b)     // jbe
{
    return (fp::compare(a, b) & 0x4100) != 0;
}

/* `count` dwords from one place to another, as movsd moves them. */
void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 at = 0; at < count * 4; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
}

/* An index held to a table of `last` and one. */
u32 clampedBucket(s32 index, s32 last)
{
    return index < 0 ? 0 : index > last ? u32(last) : u32(index);
}

// The registers a function's pushes keep; the rest it leaves as they fall.
constexpr std::uint8_t kKeepsEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsEcxEsiEdi = fe::kRestoreEcx | kKeepsEsiEdi;
constexpr std::uint8_t kKeepsEdx = fe::kRestoreEdx;
constexpr std::uint8_t kKeepsEdxEdi = fe::kRestoreEdx | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsAllButEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi;

// The game's functions, by address.
constexpr u32 kDepthBuckets = 0x4c7790;      // eax a list, the stack its depth
constexpr u32 kSortedBuckets = 0x4c7610;
constexpr u32 kGatherSorted = 0x4c7320;      // esi where the list goes
constexpr u32 kFillZero = 0x4e070c;          // eax where, edx how many bytes
constexpr u32 kRandom = 0x4e0d88;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kStrstr = 0x4e0820;

// The race's data.
constexpr u32 kDisplayFlags = 0x7a3a58;      // byte; bit 2 a card with alpha
constexpr u32 kDrawnOnCard = 0x7a3a60;       // 0: polygons sorted here, no z-buffer
constexpr u32 kFrameTime = 0x7d3684;
constexpr u32 kVertexCode = 0x14;            // a corner's clip code, a byte

/* ---------------------------------------------------------------------------
 * The skid marks: sub_4db940, sub_4db8b0, sub_4db750, sub_4c6cb0, sub_4c6f00
 * ------------------------------------------------------------------------- */

constexpr u32 kSkidStrips = 0x560110;        // the strips' block
constexpr u32 kSkidStripBytes = 0x340;
constexpr u32 kSkidStripNow = 0x7d5754;      // the one being laid
constexpr u32 kSkidStripsUsed = 0x7d5758;    // how many have been, 64 at most
constexpr u32 kSkidDrawnAt = 0x7d5750;       // the time they were last drawn
constexpr s32 kSkidStripsMost = 0x40;
constexpr u32 kSkidSegmentsMost = 0x10;

// A strip.
constexpr u32 kStripOrigin = 0;              // a point, three floats
constexpr u32 kStripCount = 0xc;             // its segments
constexpr u32 kStripCorners = 0x10;          // four corners of 12 bytes a segment
constexpr u32 kStripCornersBytes = 48;
constexpr u32 kStripTextures = 0x310;        // a byte a segment: which texture
constexpr u32 kStripAlphas = 0x320;          // two bytes a segment: how dark at its ends

constexpr u32 kSkidLift = 0x540218;          // a float: how far above the road they lie
constexpr u32 kSkidFarApart = 0x54021c;      // a float: past this from the origin, a new strip
constexpr u32 kSkidTextures = 0x8b41c8;      // a texture's record, by a segment's byte

/* What sub_4c6cb0 asks of one axis: whether the two are as far apart as its bound. */
bool farApartOnAxis(const Guest& guest, u32 origin, u32 point, double bound)
{
    const float difference = float(fp::sub(floatOf(guest, origin), floatOf(guest, point)));
    const float size = jumpsBelowOrEqual(0.0, difference) ? difference : float(-double(difference));
    return !jumpsBelow(size, bound);
}

u32 skidStripOf(const Guest& guest, u32 index)
{
    return guest.dword(kSkidStrips) + index * kSkidStripBytes;
}

}  // namespace

/* sub_4c6cb0: a segment from `from` to `to` (two sides each, six floats) with
 * its ends' alphas, on the strip being laid, or a new one when that one is full
 * or far from here; `mode` 0 lays none, 2 is the textures' second pair. */
nfs::u32 frontend::rende::skidAddSegment(App* app, ThreadState& ts, u32 from, u32 to, u32 alphaFrom, u32 alphaTo, u32 mode)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 0x4c);
    constexpr u32 kToSides = 0, kFromSides = 0x18;
    const u32 frame = guest.frame();
    const double lift = floatOf(guest, kSkidLift);
    guest.setDword(frame + kToSides, guest.dword(to));
    guest.setFloat(frame + kToSides + 4, float(fp::add(floatOf(guest, to + 4), lift)));
    guest.setDword(frame + kToSides + 8, guest.dword(to + 8));
    guest.setDword(frame + kToSides + 0xc, guest.dword(to + 0xc));
    guest.setFloat(frame + kToSides + 0x10, float(fp::add(floatOf(guest, to + 0x10), lift)));
    guest.setDword(frame + kToSides + 0x14, guest.dword(to + 0x14));
    guest.setDword(frame + kFromSides, guest.dword(from));
    guest.setFloat(frame + kFromSides + 4, float(fp::add(floatOf(guest, from + 4), lift)));
    guest.setDword(frame + kFromSides + 8, guest.dword(from + 8));
    guest.setDword(frame + kFromSides + 0xc, guest.dword(from + 0xc));
    guest.setFloat(frame + kFromSides + 0x10, float(fp::add(lift, floatOf(guest, from + 0x10))));
    const u32 last = guest.dword(from + 0x14);
    guest.setDword(frame + kFromSides + 0x14, last);
    if (mode == 0)
        return last;

    u32 strip = skidStripOf(guest, guest.dword(kSkidStripNow));
    bool newStrip = guest.dword(strip + kStripCount) == kSkidSegmentsMost;
    if (!newStrip)
    {
        const double bound = floatOf(guest, kSkidFarApart);
        newStrip = farApartOnAxis(guest, strip, to, bound) || farApartOnAxis(guest, strip + 4, to + 4, bound)
                   || farApartOnAxis(guest, strip + 8, to + 8, bound);
    }
    if (newStrip)
    {
        const u32 used = guest.dword(kSkidStripsUsed);
        if (s32(used) < kSkidStripsMost)
        {
            guest.setDword(kSkidStripNow, used);
            guest.setDword(kSkidStripsUsed, used + 1);
        }
        else
        {
            const u32 now = guest.dword(kSkidStripNow);
            guest.setDword(kSkidStripNow, s32(now) < kSkidStripsMost - 1 ? now + 1 : 0);
        }
        strip = skidStripOf(guest, guest.dword(kSkidStripNow));
        guest.setDword(strip + kStripCount, 0);
    }
    if (newStrip || guest.dword(strip + kStripCount) == 0)
        copyDwords(guest, strip + kStripOrigin, to, 3);

    const u32 count = guest.dword(strip + kStripCount);
    const u32 corners = strip + kStripCorners + count * kStripCornersBytes, alphas = strip + kStripAlphas + 2 * count;
    // As the call by address leaves them, for the end (edx's high bytes with the texture).
    fe::direct<0x4c6c70, frontend::sime::skidSegmentPut>(app, ts, corners, alphas, frame + kFromSides, frame + kToSides,
                                                         alphaFrom & 0xff, alphaTo & 0xff);
    const u32 random = callAt(guest, kRandom);
    uint8_t texture = uint8_t(random & 1);
    const u32 countNow = guest.dword(strip + kStripCount);
    if (mode == 2)
    {
        texture = uint8_t(texture + 2);
    }
    guest.setByte(strip + kStripTextures + countNow, texture);
    guest.setDword(strip + kStripCount, guest.dword(strip + kStripCount) + 1);
    return strip;
}


/* sub_4db750: a piece of the mark from `from` to `to`, halved until it is short
 * (14) or three times halved, its alpha going from one end's to the other's. */
nfs::u32 frontend::rende::skidSubdivide(App* app, ThreadState& ts, u32 depth, u32 from, u32 to, u32 alphaFrom, u32 alphaToWord, u32 mode)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 0x28);
    const uint8_t alphaToIn = uint8_t(alphaToWord);
    constexpr u32 kShortEnough = 0x41600000;  // 14.0f
    constexpr u32 kHalf = 0x54970c;           // a double
    constexpr s32 kDeepest = 3;
    const uint8_t alphaTo = mode == 2 ? uint8_t(0) : alphaToIn;
    const u32 alphaEnd = alphaFrom & 0xff;
    if (fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, ts, from, to, kShortEnough) != 0
        || s32(depth) >= kDeepest)
        return guest::direct<0x4c6cb0, skidAddSegment>(app, ts, from, to, alphaEnd, u32(alphaTo), mode);

    const u32 middle = guest.frame();
    const double half = doubleOf(guest, kHalf);
    for (u32 at = 0; at < 0x18; at += 4)
    {
        const double sum = fp::add(floatOf(guest, from + at), floatOf(guest, to + at));
        guest.setFloat(middle + at, float(at == 0x14 ? fp::mul(half, sum) : fp::mul(sum, half)));
    }
    const u32 alphaMiddle = ((u32(alphaTo) + alphaEnd) >> 1) & 0xff;
    guest::direct<0x4db750, skidSubdivide>(app, ts, depth + 1, from, middle, alphaEnd, alphaMiddle, mode);
    return guest::direct<0x4db750, skidSubdivide>(app, ts, depth + 1, middle, to, alphaMiddle, u32(alphaTo), mode);
}


/* sub_4db8b0: the two sides of a mark at `centre`, as wide across the way from
 * `from` to `to` and the wheel's axle `axle`. */
nfs::u32 frontend::rende::skidStrip(App* app, ThreadState& ts, u32 centre, u32 from, u32 to, u32 axle, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 0x18);
    constexpr u32 kHalfWidth = 0x549714;      // a double
    const u32 way = guest.frame();
    const u32 across = way + 0xc;
    guest.setFloat(way, float(fp::sub(floatOf(guest, to), floatOf(guest, from))));
    guest.setFloat(way + 4, float(fp::sub(floatOf(guest, to + 4), floatOf(guest, from + 4))));
    guest.setFloat(way + 8, float(fp::sub(floatOf(guest, to + 8), floatOf(guest, from + 8))));
    fe::direct<0x4972f0, vertices::normaliseVector>(app, ts, way);
    // As the call by address leaves them.
    const u32 left = fe::direct<0x4e0160, frontend::simd::cross>(app, ts, way, axle, across);

    const double width = doubleOf(guest, kHalfWidth);
    guest.setFloat(out, float(fp::sub(floatOf(guest, centre), fp::mul(floatOf(guest, across), width))));
    guest.setFloat(out + 4, float(fp::sub(floatOf(guest, centre + 4), fp::mul(floatOf(guest, across + 4), width))));
    guest.setFloat(out + 8, float(fp::sub(floatOf(guest, centre + 8), fp::mul(floatOf(guest, across + 8), width))));
    guest.setFloat(out + 0xc, float(fp::add(fp::mul(floatOf(guest, across), width), floatOf(guest, centre))));
    guest.setFloat(out + 0x10, float(fp::add(fp::mul(floatOf(guest, across + 4), width), floatOf(guest, centre + 4))));
    guest.setFloat(out + 0x14, float(fp::add(fp::mul(width, floatOf(guest, across + 8)), floatOf(guest, centre + 8))));
    return left;
}

namespace
{

// A wheel's mark as it is laid.
constexpr u32 kWheelMarks = 0x8b7ba0;
constexpr u32 kWheelMarksPerCar = 0xd0;
constexpr u32 kWheelMarkBytes = 0x34;
constexpr u32 kMarkTime = 0;
constexpr u32 kMarkSides = 4;                // two sides, three floats each
constexpr u32 kMarkPoint = 0x1c;             // where it last was
constexpr u32 kMarkAlpha = 0x28;             // a byte
constexpr u32 kMarkSliding = 0x2c;           // whether it was, at its last call
constexpr u32 kMarkFresh = 0x30;             // nothing laid since it started

}  // namespace

/* sub_4db940: a car's wheel at `position` and with axle `axle` (16.16 integers),
 * sliding or not as `released` is clear or set, as hard as `strength`: a mark
 * begun, laid on or ended. */
nfs::u32 frontend::rende::skidWheel(App* app, ThreadState& ts, u32 car, u32 wheel, u32 position, u32 axle, u32 released, u32 strength)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 0x40);
    constexpr u32 kToFloat = 0x54971c, kToAlpha = 0x549724;   // doubles
    constexpr u32 kLayFrom = 0x3f000000;      // 0.5f: moved this far, more is laid
    constexpr u32 kEndFrom = 0x3e4ccccd;      // 0.2f
    const u32 frame = guest.frame();
    const u32 ebp = frame + 0x40;
    const u32 sides = ebp - 0x40, axleNow = ebp - 0x28, here = ebp - 0x1c;

    const double toFloat = doubleOf(guest, kToFloat);
    for (u32 at = 0; at < 12; at += 4)
        guest.setFloat(here + at, float(fp::mul(s32(guest.dword(position + at)), toFloat)));
    for (u32 at = 0; at < 8; at += 4)
        guest.setFloat(axleNow + at, float(fp::mul(s32(guest.dword(axle + at)), toFloat)));
    guest.setFloat(axleNow + 8, float(fp::mul(toFloat, s32(guest.dword(axle + 8)))));
    const u32 mark = kWheelMarks + (car & 0xf) * kWheelMarksPerCar + wheel * kWheelMarkBytes;

    // How hard, from 0 to 1, to an alpha of 0 to 255 by 143 >> 16.
    const u32 given = strength;
    const u32 atLeastZero = jumpsBelowOrEqual(0.0, floatOfBits(strength)) ? given : 0;
    float least;
    std::memcpy(&least, &atLeastZero, sizeof least);
    const float bounded = jumpsBelowOrEqual(1.0, least) ? 1.0f : least;
    const s32 scaled = fp::toInteger<s32>(fp::mul(bounded, doubleOf(guest, kToAlpha)));
    const u32 alpha = u32(s32(u32(scaled) * 143u) >> 16);

    if (released == 0)
    {
        const u32 since = guest.dword(kFrameTime) - guest.dword(mark + kMarkTime);
        if (s32(since) <= 0)
            return since;
    }
    const u32 now = guest.dword(kFrameTime);
    const u32 sliding = guest.dword(mark + kMarkSliding);
    guest.setDword(mark + kMarkTime, now);
    if (sliding == 0)
    {
        if (released != 0)
            return now;
        guest.setDword(mark + kMarkSliding, 1);
        copyDwords(guest, mark + kMarkSides, here, 3);
        copyDwords(guest, mark + kMarkSides + 12, here, 3);
        copyDwords(guest, mark + kMarkPoint, here, 3);
        guest.setDword(mark + kMarkFresh, 1);
        guest.setByte(mark + kMarkAlpha, 0);
        return mark;
    }

    const u32 point = mark + kMarkPoint;
    const u32 markSides = mark + kMarkSides;
    // Nonzero when the mark's point is within `limit` of here; edx as the call by address leaves it.
    const auto nearHere = [&](u32 limit) {
        return fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, ts, point, here, limit);
    };
    if (released != 0)
    {
        // The wheel has stopped sliding: the last of the mark, fading out.
        if (guest.dword(mark + kMarkFresh) == 0 && nearHere(kEndFrom) == 0)
        {
            guest::direct<0x4db8b0, skidStrip>(app, ts, here, point, here, axleNow, sides);
            guest::direct<0x4db750, skidSubdivide>(app, ts, 0u, markSides, sides, u32(guest.byte(mark + kMarkAlpha)), 0u, 2u);
            copyDwords(guest, markSides, sides, 3);
            copyDwords(guest, markSides + 12, sides + 12, 3);
            copyDwords(guest, point, here, 3);
        }
        guest.setDword(mark + kMarkSliding, 0);
        return mark;
    }

    // Sliding on: laid on when it has moved far enough.
    const u32 within = nearHere(kLayFrom);
    if (within != 0)
        return within;
    guest::direct<0x4db8b0, skidStrip>(app, ts, here, point, here, axleNow, sides);
    if (guest.dword(mark + kMarkFresh) != 0)
    {
        guest::direct<0x4db8b0, skidStrip>(app, ts, point, point, here, axleNow, markSides);
        guest.setDword(mark + kMarkFresh, 0);
    }
    guest::direct<0x4db750, skidSubdivide>(app, ts, 0u, markSides, sides, u32(guest.byte(mark + kMarkAlpha)), alpha & 0xff, 1u);
    guest.setDword(mark + kMarkSliding, 1);
    copyDwords(guest, markSides, sides, 3);
    copyDwords(guest, markSides + 12, sides + 12, 3);
    copyDwords(guest, point, here, 3);
    guest.setByte(mark + kMarkAlpha, uint8_t(alpha));
    return withLow(mark, uint8_t(alpha));
}

namespace
{

// A polygon of the lists.
constexpr u32 kPolygonNext = 0;
constexpr u32 kPolygonKind = 4;              // a word: 0 and 2 quads, 1 and 3 triangles
constexpr u32 kPolygonLayer = 6;             // a word
constexpr u32 kPolygonCorners = 8;           // four pointers
constexpr u32 kPolygonTexture = 0x18;
constexpr u32 kPolygonDepth = 0x1c;          // a float
constexpr u32 kPolygonBytes = 0x20;
constexpr u32 kVertexBytes = 0x20;
constexpr u32 kVertexDepth = 8;              // a float
constexpr u32 kVertexColour = 0x10;

}  // namespace

/* sub_4c6f00: the strips near the view, newest first, drawn as one list. */
nfs::u32 frontend::rende::skidMarksDraw(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x24);
    constexpr u32 kNearEnough = 0x42480000;   // 50.0f
    constexpr u32 kViewPosition = 8, kViewCamera = 0x38, kViewMatrix = 0x44;
    constexpr u32 kSkidDepth = 0x7dcfec;
    constexpr u32 kOpaque = 0xffffff;
    const u32 frame = guest.frame();
    const u32 verticesAt = frame, polygonAt = frame + 8;   // where the allocator puts its blocks
    u32 first = 0, last = 0;
    if (gameSettings(guest).effectsLevel == 2)
        return view;

    for (u32 index = guest.dword(kSkidStripsUsed); --index != u32(-1);)
    {
        const u32 strip = skidStripOf(guest, index);
        if (fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, ts, strip, view + kViewPosition,
                                                                      kNearEnough) == 0)
            continue;
        const u32 corners = guest.dword(strip + kStripCount) << 2;
        if (corners == 0)
            continue;
        if (guest::direct<0x4bbde0, rendd::arenaTake>(app, ts, verticesAt, corners << 5) == 0)
            continue;
        u32 vertices = guest.dword(verticesAt);
        u32 segment = guest.dword(strip + kStripCount);
        fe::direct<0x4bf4c0, vertices::objectVerticesOf>(app, ts, std::int32_t(corners), strip + kStripCorners, view + kViewMatrix,
                                                         view + kViewCamera, vertices);
        fe::direct<0x431c20, collide::layerNothing>(app, ts, segment, 5u);
        u32 texture = strip + kStripTextures, alpha = strip + kStripAlphas;
        while (--segment != u32(-1))
        {
            if (guest::direct<0x4bbde0, rendd::arenaTake>(app, ts, polygonAt, kPolygonBytes) != 0)
            {
                const u32 polygon = guest.dword(polygonAt);
                if (first == 0)
                    first = polygon;
                else
                    guest.setDword(last, polygon);
                guest.setDword(polygon + kPolygonNext, 0);
                last = polygon;
                guest.setWord(polygon + kPolygonKind, 0);
                guest.setWord(polygon + kPolygonLayer, 0);
                for (u32 corner = 0; corner < 4; ++corner)
                    guest.setDword(polygon + kPolygonCorners + 4 * corner, vertices + corner * kVertexBytes);
                guest.setDword(polygon + kPolygonTexture, guest.dword(kSkidTextures + 4 * guest.byte(texture)));
                const u32 colourNear = (u32(guest.byte(alpha)) << 24) | kOpaque;
                guest.setDword(vertices + kVertexBytes + kVertexColour, colourNear);
                guest.setDword(vertices + kVertexColour, colourNear);
                const u32 colourFar = (u32(guest.byte(alpha + 1)) << 24) | kOpaque;
                guest.setDword(vertices + 3 * kVertexBytes + kVertexColour, colourFar);
                guest.setDword(vertices + 2 * kVertexBytes + kVertexColour, colourFar);
            }
            ++texture;
            alpha += 2;
            vertices += 4 * kVertexBytes;
        }
    }
    if (last != 0)
        guest.setDword(last, 0);
    fe::direct<0x4c7790, vertices::bucketsDepthedInto>(guest.app(), guest.ts(), first, guest.dword(kSkidDepth));
    const u32 now = guest.dword(kFrameTime);
    guest.setDword(kSkidDrawnAt, now);
    return now;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The depth buckets: without the z-buffer polygons go back to front
 * ------------------------------------------------------------------------- */

constexpr u32 kDepthHeads = 0x7d5f5c;        // first and last of a list, by depth, 2000 of them
constexpr u32 kDepthLast = 0x7d5f60;
constexpr u32 kDepthBytes = 0x3e80;
constexpr u32 kDepthDeepest = 0x7d9dd4;
constexpr s32 kDepthMost = 0x7cf;
constexpr u32 kDepthBias = 0x7dcfdc;         // a float
constexpr u32 kSortHeads = 0x7d9ddc;         // sub_4c7380's, 800 of them
constexpr u32 kSortLast = 0x7d9de0;
constexpr u32 kSortBytes = 0x1900;
constexpr u32 kFarHeads = 0x7db6dc;          // 800 of them, on the card
constexpr u32 kFarBytes = 0x1900;
constexpr u32 kFarDeepest = 0x7dcfd4;
constexpr u32 kNearBuckets = 0x7d575c;
constexpr u32 kNearBytes = 0x800;
constexpr u32 kPendingList = 0x7d5f54;       // first and last

}  // namespace

/* sub_4c7180: each polygon's depth, the average of its corners'; the least of
 * them at `least`, and at `scale` 799 over their range, so that sub_4c7260 finds
 * each its place among 800. */
nfs::u32 frontend::rende::polygonDepths(App* app, ThreadState& ts, u32 list, u32 least, u32 scale)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16 + 0x10);
    constexpr u32 kQuarter = 0x540224, kThird = 0x540228, kBucketsLast = 0x54022c;    // floats
    constexpr u32 kFarthest = 0x497423f0, kNearest = 0xc97423f0;                      // 1e6 and -1e6
    const u32 most = guest.frame() + 8;   // [ebp-8]
    guest.setDword(least, kFarthest);
    guest.setDword(most, kNearest);
    u32 left = list;
    u32 polygon = list;
    for (; polygon != 0; polygon = guest.dword(polygon + kPolygonNext))
    {
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        double depth;
        const u32 corners = polygon + kPolygonCorners;
        if (kind == 0 || kind == 2)
        {
            depth = fp::add(floatOf(guest, guest.dword(corners) + kVertexDepth), floatOf(guest, guest.dword(corners + 4) + kVertexDepth));
            depth = fp::add(depth, floatOf(guest, guest.dword(corners + 8) + kVertexDepth));
            depth = fp::add(depth, floatOf(guest, guest.dword(corners + 0xc) + kVertexDepth));
            depth = fp::mul(depth, floatOf(guest, kQuarter));
        }
        else if (kind == 1 || kind == 3)
        {
            depth = fp::add(floatOf(guest, guest.dword(corners) + kVertexDepth), floatOf(guest, guest.dword(corners + 4) + kVertexDepth));
            depth = fp::add(depth, floatOf(guest, guest.dword(corners + 8) + kVertexDepth));
            depth = fp::mul(depth, floatOf(guest, kThird));
        }
        else
            continue;
        const float average = float(depth);
        if (jumpsBelow(average, guest.floatAt(least)))
            guest.setFloat(least, average);
        if (!jumpsBelowOrEqual(average, guest.floatAt(most)))
            guest.setFloat(most, average);
        guest.setFloat(polygon + kPolygonDepth, average);
        std::memcpy(&left, &average, sizeof left);
    }
    const float range = float(fp::sub(guest.floatAt(most), guest.floatAt(least)));
    u32 rangeBits;
    std::memcpy(&rangeBits, &range, sizeof rangeBits);
    if ((rangeBits & 0x7fffffff) == 0)
        guest.setDword(scale, polygon);
    else
        guest.setFloat(scale, float(fp::div(floatOf(guest, kBucketsLast), range)));
    return left;
}


/* sub_4c7260: each polygon into its bucket by depth, each bucket kept in order. */
nfs::u32 frontend::rende::polygonsIntoBuckets(App* app, ThreadState& ts, u32 list, u32 least, u32 scale)
{
    Guest guest(app, ts);
    u32 left = list;
    for (u32 polygon = list; polygon != 0;)
    {
        const u32 next = guest.dword(polygon + kPolygonNext);
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        left = next;
        if (kind <= 3)
        {
            const float depth = guest.floatAt(polygon + kPolygonDepth);
            const u32 bucket = u32(fp::toInteger<s32>(fp::mul(fp::sub(depth, floatOfBits(least)), floatOfBits(scale))));
            u32 head = guest.dword(kSortHeads + bucket * 8);
            u32 tail = guest.dword(kSortLast + bucket * 8);
            u32 before = 0;
            u32 at = head;
            while (at != 0 && !jumpsBelow(depth, guest.floatAt(at + kPolygonDepth)))
            {
                before = at;
                at = guest.dword(at + kPolygonNext);
            }
            if (at == head)
            {
                guest.setDword(polygon + kPolygonNext, head);
                head = polygon;
                if (tail == 0)
                    tail = polygon;
            }
            else if (at == 0)
            {
                guest.setDword(polygon + kPolygonNext, at);
                guest.setDword(tail, polygon);
                tail = polygon;
            }
            else
            {
                guest.setDword(before, polygon);
                guest.setDword(polygon + kPolygonNext, at);
            }
            guest.setDword(kSortHeads + bucket * 8, head);
            guest.setDword(kSortLast + bucket * 8, tail);
            left = bucket;
        }
        polygon = next;
    }
    return left;
}


/* sub_4c7380: a list sorted by depth, by the buckets: through esi, its first and
 * last. */
nfs::u32 frontend::rende::polygonsSorted(App* app, ThreadState& ts, u32 result, u32 list, u32 last)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x10);
    static_cast<void>(last);   // the list's last: not read
    const u32 ebp = guest.frame() + 0x10;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kSortHeads, kSortBytes);
    guest::direct<0x4c7180, polygonDepths>(app, ts, list, ebp - 8, ebp - 4);
    guest::direct<0x4c7260, polygonsIntoBuckets>(app, ts, list, guest.dword(ebp - 8), guest.dword(ebp - 4));
    callAt(guest, kGatherSorted, fe::Arguments(), ebp - 0x10);  // esi: where the callee puts the list
    copyDwords(guest, result, ebp - 0x10, 2);
    return result;
}


/* sub_4c7400: a list onto the bucket of a depth: `first`, `last`, `depth`. */
nfs::u32 frontend::rende::depthBucketAppend(App* app, ThreadState& ts, u32 view, u32 first, u32 last, u32 depth)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 8);
    const float depthValue = floatOfBits(depth);
    constexpr u32 kBucketsPerUnit = 0x540230;  // a float
    if (guest.dword(kDrawnOnCard) != 0)
        return view;
    const double place = fp::mul(fp::add(depthValue, floatOf(guest, kDepthBias)), floatOf(guest, kBucketsPerUnit));
    const u32 bucket = clampedBucket(fp::toInteger<s32>(place), kDepthMost) * 8;
    const u32 joined = guest.frame();
    const u32 left = guest::direct<0x4bbec0, rendd::joinLists>(app, ts, joined, guest.dword(kDepthHeads + bucket),
                                                                 guest.dword(kDepthLast + bucket), first, last);
    copyDwords(guest, kDepthHeads + bucket, joined, 2);
    return left;
}


/* sub_4c7480: each polygon of a list onto its own bucket, by its corners' depth
 * less `base`. */
nfs::u32 frontend::rende::depthBucketsAdd(App* app, ThreadState& ts, u32 list, u32 base)
{
    Guest guest(app, ts);
    constexpr u32 kQuarter = 0x540234, kThird = 0x540244;    // doubles
    constexpr u32 kBucketsPerUnit = 0x54023c;                // a float
    u32 left = list;
    if (guest.dword(kDrawnOnCard) != 0)
        return left;
    for (u32 polygon = list; polygon != 0;)
    {
        const u32 next = guest.dword(polygon + kPolygonNext);
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        guest.setDword(polygon + kPolygonNext, 0);
        u32 bucket = 0;
        const u32 corners = polygon + kPolygonCorners;
        if (kind == 0 || kind == 1)
        {
            double depth = fp::add(floatOf(guest, guest.dword(corners) + kVertexDepth), floatOf(guest, guest.dword(corners + 4) + kVertexDepth));
            depth = fp::add(depth, floatOf(guest, guest.dword(corners + 8) + kVertexDepth));
            if (kind == 0)
            {
                depth = fp::add(depth, floatOf(guest, guest.dword(corners + 0xc) + kVertexDepth));
                depth = fp::mul(depth, doubleOf(guest, kQuarter));
            }
            else
                depth = fp::mul(depth, doubleOf(guest, kThird));
            depth = fp::sub(depth, double(floatOfBits(base)));
            depth = fp::add(depth, floatOf(guest, kDepthBias));
            depth = fp::mul(depth, floatOf(guest, kBucketsPerUnit));
            bucket = clampedBucket(fp::toInteger<s32>(depth), kDepthMost);
        }
        bucket *= 8;
        if (guest.dword(kDepthHeads + bucket) != 0)
            guest.setDword(guest.dword(kDepthLast + bucket), polygon);
        else
            guest.setDword(kDepthHeads + bucket, polygon);
        guest.setDword(kDepthLast + bucket, polygon);
        left = bucket;
        polygon = next;
    }
    return left;
}


/* sub_4c7580: the depth buckets, deepest first, as one list, through esi. */
nfs::u32 frontend::rende::depthBucketsGather(App* app, ThreadState& ts, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x10);
    const u32 ebp = guest.frame() + 0x10;
    guest::direct<0x4bbe70, rendd::emptyList>(app, ts, ebp - 8);
    u32 first = guest.dword(ebp - 8), last = guest.dword(ebp - 4);
    if (guest.dword(kDrawnOnCard) == 0)
    {
        u32 bucket = kDepthDeepest;
        for (u32 i = 0; i < 2000; ++i, bucket -= 8)
        {
            if (first == 0)
            {
                first = guest.dword(bucket);
                last = guest.dword(bucket + 4);
                continue;
            }
            const u32 head = guest.dword(bucket);
            if (head != 0)
            {
                guest.setDword(last, head);
                last = guest.dword(bucket + 4);
            }
        }
    }
    guest.setDword(result, first);
    guest.setDword(result + 4, last);
    return result;
}


/* sub_4c76e0: the same of the far buckets, on the card. */
nfs::u32 frontend::rende::farBucketsGather(App* app, ThreadState& ts, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x10);
    const u32 ebp = guest.frame() + 0x10;
    guest::direct<0x4bbe70, rendd::emptyList>(app, ts, ebp - 0x10);
    u32 first = guest.dword(ebp - 0x10), last = guest.dword(ebp - 0xc);
    if (guest.dword(kDrawnOnCard) != 0)
    {
        u32 bucket = kFarDeepest;
        for (u32 i = 0; i < 800; ++i, bucket -= 8)
        {
            const u32 head = guest.dword(bucket);
            if (head == 0)
                continue;
            if (first != 0)
                guest.setDword(last, head);
            else
                first = head;
            last = guest.dword(bucket + 4);
        }
    }
    guest.setDword(result, first);
    guest.setDword(result + 4, last);
    return result;
}


/* sub_4c7940: a list for the frame: on the card with the list pending, else by
 * depth (sub_4c7480). */
nfs::u32 frontend::rende::drawListAdd(App* app, ThreadState& ts, u32 first, u32 last, u32 base)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16 + 8);
    if (first == 0)
        return 0;
    if (guest.dword(kDrawnOnCard) == 0)
        return guest::direct<0x4c7480, depthBucketsAdd>(app, ts, first, base);
    const u32 joined = guest.frame();
    const u32 left = guest::direct<0x4bbec0, rendd::joinLists>(app, ts, joined, guest.dword(kPendingList),
                                                                 guest.dword(kPendingList + 4), first, last);
    copyDwords(guest, kPendingList, joined, 2);
    return left;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The blinking quads: sub_4c9f70
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_4c9f70: the eight lights of the table at 0x7dd850, those lit and in view,
 * each a quad with the texture 0x7ddad0. */
nfs::u32 frontend::rende::blinkingQuads(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x98);
    constexpr u32 kLights = 0x7dd850, kLightBytes = 0x50, kLightCount = 8;
    constexpr u32 kLightCorners = 4, kLightAcross = 0x48, kLightOnUntil = 0x4c;
    constexpr u32 kTooFar = 0x540378;     // a double
    constexpr u32 kLightTexture = 0x7ddad0, kViewCamera = 0x38, kViewMatrix = 0x44, kDepth = 0x7dcfe0;
    constexpr u32 kColour = 0xffffffff;
    constexpr u32 kFirstV = 0x3ca3d70a, kOneV = 0x3f800000, kLastV = 0x3f7ae148;   // 0.02, 1, 0.98
    const u32 corners = guest.frame();
    const u32 list = corners + 0x80, quadAt = corners + 0x8c;

    guest::direct<0x4bbe70, rendd::emptyList>(app, ts, list);
    guest::direct<0x4bbf20, rendd::openListStart>(app, ts);
    for (u32 i = 0; i < kLightCount; ++i)
    {
        const u32 light = kLights + i * kLightBytes;
        if (guest.dword(light) == 0)
            continue;
        const u32 now = guest.dword(kFrameTime), until = guest.dword(light + kLightOnUntil);
        if (s32(now) < s32(until) && ((until - now) & 0x10) != 0)
            continue;
        fe::direct<0x4bf4c0, vertices::objectVerticesOf>(app, ts, 4, light + kLightCorners, view + kViewMatrix, view + kViewCamera,
                                                         corners);
        if (!jumpsBelowOrEqual(floatOf(guest, corners + kVertexDepth), doubleOf(guest, kTooFar)))
            continue;
        const u32 out = u32(guest.byte(corners + kVertexCode)) & guest.byte(corners + kVertexBytes + kVertexCode)
                          & guest.byte(corners + 2 * kVertexBytes + kVertexCode) & guest.byte(corners + 3 * kVertexBytes + kVertexCode);
        if (out != 0)
            continue;
        if (guest::direct<0x4bbf80, rendd::openListTake>(app, ts, quadAt, 0xa0u) == 0)
            continue;
        const u32 quad = guest.dword(quadAt);
        const u32 across = guest.dword(light + kLightAcross);
        const u32 places[4][2] = { { 0, kFirstV }, { across, kFirstV }, { across, kOneV }, { 0, kLastV } };
        for (u32 corner = 0; corner < 4; ++corner)
        {
            const u32 from = corners + corner * kVertexBytes, to = quad + kPolygonBytes + corner * kVertexBytes;
            copyDwords(guest, to, from, 8);
            guest.setDword(to + kVertexColour, kColour);
            guest.setDword(to + 0x18, places[corner][0]);
            guest.setDword(to + 0x1c, places[corner][1]);
        }
        guest.setWord(quad + kPolygonKind, 0);
        guest.setWord(quad + kPolygonLayer, 0xa);
        guest.setDword(quad + kPolygonTexture, kLightTexture);
        for (u32 corner = 0; corner < 4; ++corner)
            guest.setDword(quad + kPolygonCorners + 4 * corner, quad + kPolygonBytes + corner * kVertexBytes);
    }
    guest::direct<0x4bbfc0, rendd::openListClose>(app, ts, list);
    return guest::direct<0x4bc060, rendd::drawListAt>(app, ts, view, list, guest.dword(kDepth));
}

namespace
{

/* ---------------------------------------------------------------------------
 * The particles
 * ------------------------------------------------------------------------- */

constexpr u32 kParticleStates = 0x7ddfd0;    // by view and particle: its bytes
constexpr u32 kParticleStatesBytes = 0x12c00;
constexpr u32 kFades = 0x7f0bd0;             // a table of 200 a view
constexpr u32 kFadesBytes = 0x6400;
constexpr u32 kFadesPerView = 0x3200;
constexpr u32 kFadeBytes = 0x40;
constexpr u32 kFadeCount = 0x7f6fd0;         // by view: how many are alive
constexpr u32 kParticleCameras = 0x7f6fd8;   // by view: the camera they were drawn from

}  // namespace

/* sub_4cb220: a view's fading particles a step on: alpha in proportion to the
 * time left, those gone cleared, the living counted. */
nfs::u32 frontend::rende::fadeParticles(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    constexpr u32 kFadeLeft = 0, kFadeKind = 4, kFadeAlpha = 0x2b;
    constexpr u32 kKindLife = 0x560bcc, kKindAlpha = 0x560bdf;
    constexpr u32 kPerFadeTable = 200;
    guest.setDword(kFadeCount + view * 4, 0);
    u32 left = 0;
    for (u32 i = 0; i < kPerFadeTable; ++i)
    {
        const u32 fade = kFades + view * kFadesPerView + i * kFadeBytes;
        left = i << 6;
        const u32 time = guest.dword(fade + kFadeLeft);
        if (time == 0)
            continue;
        const u32 kind = guest.dword(fade + kFadeKind) << 7;
        const s32 life = s32(guest.dword(kKindLife + kind));
        const u32 whole = guest.byte(kKindAlpha + kind);
        guest.setDword(kFadeCount + view * 4, guest.dword(kFadeCount + view * 4) + 1);
        const s32 alpha = s32(whole * time) / life;
        guest.setByte(fade + kFadeAlpha, uint8_t(alpha));
        const uint8_t alphaNow = guest.byte(fade + kFadeAlpha);
        guest.setDword(fade + kFadeLeft, alphaNow != 0 ? guest.dword(fade + kFadeLeft) - 1 : 0);
        left = withHigh(u32(alpha), alphaNow);
    }
    return left;
}

namespace
{

/* sub_4cb590: each kind of particle's textures out of sfx.fsh, the night's when
 * it is night, and their lit forms. */
const GuestString kParticleNames[] = {
    { 0x540428, "SN%02d" },
    { 0x540430, "SP%02d" },
    { 0x540438, "SL%02d" },
};

}  // namespace

nfs::u32 frontend::rende::loadParticleTextures(App* app, ThreadState& ts, u32 file)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x100);
    constexpr u32 kKindCount = 0xe;
    constexpr u32 kKindHasTexture = 0x560b64, kKindHasLit = 0x560bc8;
    constexpr u32 kTextures = 0x7ddd68, kLitTextures = 0x7ddb00, kTextureBytes = 0x2c;
    constexpr u32 kFormat = 0x20;
    const u32 name = guest.frame();
    u32 left = 0;
    for (u32 kind = 0; kind < kKindCount; ++kind)
    {
        guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
        if (guest.dword(kKindHasTexture + (kind << 7)) != 0)
        {
            const u32 format = gameSettings(guest).night != 0 ? kParticleNames[0].address : kParticleNames[1].address;
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, format, guest::Words{kind});
            guest::direct<0x4d4080, loadShapeTexture>(app, ts, file, name, kFormat, kTextures + kind * kTextureBytes, 0u);
        }
        left = kind << 7;
        if (guest.dword(kKindHasLit + left) != 0)
        {
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kParticleNames[2].address, guest::Words{kind});
            left = guest::direct<0x4d4080, loadShapeTexture>(app, ts, file, name, kFormat, kLitTextures + kind * kTextureBytes, 0u);
        }
    }
    return left;
}


/* sub_4cc540: a view's camera changed, or its particles told to forget: each
 * particle's two state bytes for this view cleared.  What counts as the camera
 * is its kind, with [0x79c4a0] a camera of kinds 2 to 4 only its top byte. */
nfs::u32 frontend::rende::particlesCameraChanged(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kCameraLocked = 0x79c4a0, kForget = 0x552ff4;
    constexpr u32 kStateBytes = 0x7de048, kParticleCount = 0x258;
    const u32 mode = fe::direct<0x423b00, carstate::modeOfCamera>(app, ts, view);
    u32 camera = mode;
    if (s32(mode) <= 4 && s32(mode) >= 2 && guest.dword(kCameraLocked + view * 4) != 0)
        camera &= 0xff000000u;
    u32 left = view * 4;
    if (camera != guest.dword(kParticleCameras + left) || guest.dword(kForget + left) != 0)
    {
        for (u32 i = 0; i < kParticleCount; ++i)
        {
            guest.setByte(kStateBytes + (i << 7) + view * 2, 0);
            guest.setByte(kStateBytes + 1 + (i << 7) + view * 2, 0);
        }
        left = kParticleCount;
    }
    guest.setDword(kParticleCameras + view * 4, camera);
    return left;
}


/* sub_4cc5c0: the race frame's pass of the particles. */
nfs::u32 frontend::rende::particlesPass(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16 + 8);
    constexpr u32 kParticles = 0x4cbc90, kViewIndex = 4, kViewMatrix = 0x44, kDepth = 0x7dd000;
    if (gameSettings(guest).effectsLevel == 2)
        return view;
    const u32 list = guest.frame();
    guest::direct<0x4cc540, particlesCameraChanged>(app, ts, guest.dword(view + kViewIndex));
    callAt(guest, kParticles, args({ view, view + kViewMatrix }), list);  // esi: read by the particles' pass  // edx the view's matrix, which it reads
    return fe::direct<0x4c7610, vertices::bucketsSortedInto>(guest.app(), guest.ts(), guest.dword(list), guest.dword(kDepth));
}

namespace
{

/* What the race frame's passes leave: the list through esi, whose place they took. */
u32 handBackList(Guest& guest, u32 list, u32 result)
{
    copyDwords(guest, result, list, 2);
    return result;
}

}  // namespace

/* sub_4cc600: the frame's pass of the drops, on the first pass only, unless the
 * view has none. */
nfs::u32 frontend::rende::dropsPass(App* app, ThreadState& ts, u32 view, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 8);
    constexpr u32 kDrops = 0x4cb750;
    const u32 list = guest.frame();
    if (s32(gameSettings(guest).effectsLevel) >= 1)
    {
        guest::direct<0x4bbe70, rendd::emptyList>(app, ts, list);
    }
    else
    {
        guest::direct<0x4bbf20, rendd::openListStart>(app, ts);
        if (guest.dword(view) == 0)
            callAt(guest, kDrops, args({ view }));
        guest::direct<0x4bbfc0, rendd::openListClose>(app, ts, list);
    }
    return handBackList(guest, list, result);
}


/* sub_4dedf0: the frame's pass of the sparks, by the weather's kind of them. */
nfs::u32 frontend::rende::sparksPass(App* app, ThreadState& ts, u32 view, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 8);
    constexpr u32 kSparkKind = 0x8ca22c;
    const u32 list = guest.frame();
    guest::direct<0x4bbf20, rendd::openListStart>(app, ts);
    if (guest.dword(view) == 0 && guest.dword(kSparkKind) != 0 && gameSettings(guest).effectsLevel != 2)
    {
        fe::direct<0x4dbda0, frontend::simd::viewProjection>(app, ts, view);
        const u32 kind = guest.dword(kSparkKind);
        if (kind == 1 || kind == 2)
            fe::direct<0x4ddb40, render::sparksDrawn>(app, ts, view);
        fe::direct<0x4dbde0, frontend::simc::setView>(app, ts, view);
    }
    guest::direct<0x4bbfc0, rendd::openListClose>(app, ts, list);
    return handBackList(guest, list, result);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The smoke: sub_4dd930
 * ------------------------------------------------------------------------- */

}  // namespace

/* sub_4dd930: a puff at the point on the stack, seen from the view: in a free
 * place of the view's table, on the screen by 1/z, its life and size partly
 * chance.  1 when one was made. */
nfs::u32 frontend::rende::smokeEmit(App* app, ThreadState& ts, u32 view, u32 pointX, u32 pointY, u32 pointZ)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x34);
    constexpr u32 kHudOff = 0x7d36a0;
    constexpr u32 kPuffsPerView = 0x190, kPuffBytes = 0x14;
    constexpr u32 kPuffX = 0x8c08c0, kPuffY = 0x8c08c4, kPuffLife = 0x8c08c8, kPuffSize = 0x8c08cc, kPuffTurn = 0x8c08d0;
    constexpr u32 kPuffCount = 0x8ca278, kPuffLifeMost = 0x8ca27c, kPuffSizeMost = 0x8ca280;
    constexpr u32 kHalf = 0x5497d4;     // a double
    constexpr u32 kViewIndex = 4, kViewCamera = 0x38, kViewMatrix = 0x44;
    if (guest.dword(kHudOff) != 0)
        return 0;
    const u32 viewIndex = guest.dword(view + kViewIndex);
    const u32 table = viewIndex * kPuffsPerView;
    s32 place = 0;
    for (;; ++place)
    {
        if (place >= s32(guest.dword(kPuffCount)))
            return 0;
        if (guest.dword(table + u32(place) * kPuffBytes + kPuffLife) == 0)
            break;
    }
    const u32 puff = table + u32(place) * kPuffBytes;

    // The point by the view's matrix, its row at a time, then the camera's place.
    const u32 matrix = view + kViewMatrix;
    const double px = floatOfBits(pointX), py = floatOfBits(pointY), pz = floatOfBits(pointZ);
    const double xx = fp::mul(px, floatOf(guest, matrix)), xy = fp::mul(px, floatOf(guest, matrix + 4)), xz = fp::mul(px, floatOf(guest, matrix + 8));
    const double yx = fp::mul(py, floatOf(guest, matrix + 0xc)), yy = fp::mul(py, floatOf(guest, matrix + 0x10)), yz = fp::mul(py, floatOf(guest, matrix + 0x14));
    const double sumX = fp::add(xx, yx), sumY = fp::add(xy, yy), sumZ = fp::add(xz, yz);
    const double zx = fp::mul(pz, floatOf(guest, matrix + 0x18)), zy = fp::mul(pz, floatOf(guest, matrix + 0x1c)), zz = fp::mul(pz, floatOf(guest, matrix + 0x20));
    const float rotatedX = float(fp::add(sumX, zx)), rotatedY = float(fp::add(sumY, zy)), rotatedZ = float(fp::add(sumZ, zz));
    const float x = float(fp::add(rotatedX, floatOf(guest, view + kViewCamera)));
    const float y = float(fp::add(rotatedY, floatOf(guest, view + kViewCamera + 4)));
    const float z = float(fp::add(rotatedZ, floatOf(guest, view + kViewCamera + 8)));

    const double inverse = z != 0.0f ? fp::div(1.0, z) : 65535.0;
    const float scale = float(inverse);
    const s32 quarter = s32(guest.dword(kPuffLifeMost)) >> 2;
    const u32 spread = quarter > 1 ? u32(quarter) : 1;
    const u32 lessLife = callAt(guest, kRandom) % spread;
    guest.setFloat(puff + kPuffX, float(fp::mul(x, scale)));
    guest.setFloat(puff + kPuffY, float(fp::mul(scale, y)));
    guest.setDword(puff + kPuffLife, guest.dword(kPuffLifeMost) - lessLife);

    const double chance = double(*fe::direct<0x4eb230, ea::randomFractionOf>(app, ts));
    const double size = fp::mul(chance, floatOf(guest, kPuffSizeMost));
    const double least = fp::mul(floatOf(guest, kPuffSizeMost), doubleOf(guest, kHalf));
    const float sizeFloat = float(size);
    double chosen;
    if (jumpsBelow(size, least))
        chosen = least;
    else if (!jumpsBelowOrEqual(sizeFloat, guest.floatAt(kPuffSizeMost)))
        chosen = floatOf(guest, kPuffSizeMost);
    else
        chosen = sizeFloat;
    guest.setFloat(puff + kPuffSize, float(chosen));

    guest.setFloat(puff + kPuffTurn, float(*fe::direct<0x4eb230, ea::randomFractionOf>(app, ts)));
    return 1;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The textures
 * ------------------------------------------------------------------------- */

// A shape: its width and height, words, and its pixels.
constexpr u32 kShapeWidth = 4, kShapeHeight = 6, kShapePixels = 0x10;
constexpr uint8_t kShapeType16 = 0x6d;

// A texture's record.
constexpr u32 kTextureWidth = 0, kTextureHeight = 2, kTextureHandle = 4, kTextureFlags = 0x28, kTextureRoundRobin = 0x29;
constexpr uint8_t kTextureAdditive = 2, kTextureOwnPalette = 4;
constexpr u32 kRoundRobinNext = 0x563a8c;    // the next texture's place, a byte

}  // namespace

/* sub_4d3380: an 8-bit shape widened to 16 bits in place of the buffer `to`, by
 * its palette; 0xff transparent; the shape's kind made 0x6d. */
nfs::u32 frontend::rende::expandPalettedShape(App* app, ThreadState& ts, u32 shape, u32 to, u32 palette)
{
    Guest guest(app, ts);
    constexpr uint8_t kTransparent = 0xff;
    const s32 width = std::int16_t(guest.word(shape + kShapeWidth));
    const s32 height = std::int16_t(guest.word(shape + kShapeHeight));
    const s32 pixels = s32(u32(width) * u32(height));
    u32 from = shape + kShapePixels;
    u32 out = to;
    for (s32 i = 0; i < pixels; ++i, ++from, out += 2)
    {
        const uint8_t index = guest.byte(from);
        if (index == kTransparent)
        {
            guest.setWord(out, 0);
            continue;
        }
        const u32 colour = guest.dword(palette + u32(index) * 4);
        const u32 red = ((colour & 0xf00000) >> 12) | 0x8000;
        const u32 green = (colour & 0xf000) >> 8;
        const u32 blue = (colour & 0xf0) >> 4;
        guest.setWord(out, std::uint16_t(red | green | blue));
    }
    guest.setByte(shape, 0);
    guest.setByte(shape, kShapeType16);
    return shape;
}

namespace
{

/* sub_4d4080: a shape, by its name in a file or at edx with no file, made the
 * texture `record`, additive if its comment says so, with a place of the round
 * robin unless it brought its own palette. */
const GuestString kAdditive = { 0x54943c, "<additive>" };

}  // namespace

nfs::u32 frontend::rende::loadShapeTexture(App* app, ThreadState& ts, u32 file, u32 named, u32 format, u32 record, u32 second)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12 + 4);
    const u32 ownPalette = guest.frame();
    const u32 shape = file != 0 ? fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, file, named) : named;
    guest.setWord(record + kTextureWidth, guest.word(shape + kShapeWidth));
    guest.setWord(record + kTextureHeight, guest.word(shape + kShapeHeight));
    guest.setByte(record + kTextureFlags, 0);
    guest.setDword(record + kTextureHandle, *fe::direct<0x4d3f50, render::squareTextureMade>(app, ts, shape, ownPalette, record, format, second));

    u32 left = fe::direct<0x4f24e0, frontend::eab::shapeFindHotspot>(app, ts, shape);
    if (left != 0)
    {
        left = fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), left, kAdditive.address);
        if (left != 0)
        {
            const uint8_t flags = uint8_t(guest.byte(record + kTextureFlags) | kTextureAdditive);
            left = withHigh(left, flags);
            guest.setByte(record + kTextureFlags, flags);
            guest.setDword(ownPalette, 1);
        }
    }
    if (guest.dword(ownPalette) != 0)
    {
        const uint8_t flags = uint8_t(guest.byte(record + kTextureFlags) | kTextureOwnPalette);
        guest.setByte(record + kTextureRoundRobin, 0xff);
        guest.setByte(record + kTextureFlags, flags);
        return left;
    }
    const uint8_t place = guest.byte(kRoundRobinNext);
    guest.setByte(record + kTextureRoundRobin, place);
    const uint8_t next = uint8_t(place + 1);
    guest.setByte(kRoundRobinNext, next);
    return withLow(left, next);
}


/* sub_4d4440: a bar of the cabin's strip, the record `bar` of 36 at 0x792090: a
 * rectangle of the shape's 16-bit pixels into `buffer`, on its side if it says so. */
nfs::u32 frontend::rende::copyCabinBar(App* app, ThreadState& ts, u32 bar, u32 shape, u32 buffer)
{
    Guest guest(app, ts);
    constexpr u32 kBars = 0x792094;    // the record's fields from here
    constexpr u32 kStride = 0, kToX = 4, kToY = 8, kFromX = 0xc, kFromY = 0x10, kWidth = 0x14, kHeight = 0x18, kOnItsSide = 0x1c;
    const u32 index = bar * 9;
    const u32 fields = kBars + index * 4;
    u32 to = buffer + (guest.dword(fields + kToY) * guest.dword(fields + kStride) + guest.dword(fields + kToX)) * 2;
    const s32 shapeWidth = std::int16_t(guest.word(shape + kShapeWidth));
    u32 from = shape + kShapePixels + (guest.dword(fields + kFromY) * u32(shapeWidth) + guest.dword(fields + kFromX)) * 2;
    s32 row = 0;
    if (guest.dword(fields + kOnItsSide) != 0)
    {
        for (; row < s32(guest.dword(fields + kHeight)); ++row)
        {
            for (s32 column = 0; column < s32(guest.dword(fields + kWidth)); ++column)
            {
                const s32 across = std::int16_t(guest.word(shape + kShapeWidth));
                const u32 pixel = from + (u32(across) * u32(column) + u32(row)) * 2;
                guest.setWord(to + u32(column) * 2, guest.word(pixel));
            }
            to += guest.dword(fields + kStride) * 2;
        }
    }
    else
    {
        for (; row < s32(guest.dword(fields + kHeight)); ++row)
        {
            for (s32 column = 0; column < s32(guest.dword(fields + kWidth)); ++column)
                guest.setWord(to + u32(column) * 2, guest.word(from + u32(column) * 2));
            to += guest.dword(fields + kStride) * 2;
            from += u32(s32(std::int16_t(guest.word(shape + kShapeWidth)))) * 2;
        }
    }
    return index;
}

namespace
{

/* sub_4d5370: sfx.fsh's textures, each to its record: the lines, the shadow,
 * the skid marks, the font, the glows, the sparks, the particles' and the
 * pointer's and dials'; the loading bar on through 36 to 39. */
const GuestString kSfxStrings[] = {
    { 0x549458, "%ssfx.fsh" },
    { 0x549464, "lin%d" },
    { 0x54946c, "shad" },
    { 0x549474, "skd0" },
    { 0x54947c, "skd1" },
    { 0x549484, "skd2" },
    { 0x54948c, "skd3" },
    { 0x549494, "fntc" },
    { 0x54949c, "glw0" },
    { 0x5494a4, "glw1" },
    { 0x5494ac, "glw2" },
    { 0x5494b4, "glw3" },
    { 0x5494bc, "glw4" },
    { 0x5494c4, "spk8" },
    { 0x5494cc, "spkb" },
    { 0x5494d4, "mous" },
    { 0x5494dc, "c080" },
    { 0x5494e4, "c090" },
    { 0x5494ec, "c100" },
    { 0x5494f4, "c110" },
};

}  // namespace

nfs::u32 frontend::rende::loadEffectTextures(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x108);
    constexpr u32 kSfxFolder = 0x7a2934;
    constexpr u32 kLines = 0x8b4420, kLineCount = 10, kLineList = 0x8b41a0, kTextureBytes = 0x2c;
    constexpr u32 kShadow = 0x8b45d8, kShadowPointer = 0x8b4898;
    constexpr u32 kSkidRecords[] = { 0x8b4604, 0x8b4630, 0x8b465c, 0x8b4688 };
    constexpr u32 kFont = 0x67901c;
    constexpr u32 kGlows[] = { 0x8b47bc, 0x8b47e8, 0x8b4814, 0x8b4840, 0x8b486c };
    constexpr u32 kSparkTexture = 0x7ddad0;
    constexpr u32 kMouse = 0x8b41d8;
    constexpr u32 kDials[] = { 0x8b4338, 0x8b4364, 0x8b4390, 0x8b43bc };
    constexpr u32 kGlowSecond = 7, kFormat16 = 0x20, kFormatWide = 0x40, kFormat256 = 0x80;
    constexpr u32 kBlendState = 0xd;
    constexpr u32 kProgress36 = 0x42100000, kProgress37 = 0x42140000, kProgress38 = 0x42180000, kProgress39 = 0x421c0000;
    constexpr uint8_t kCardWithAlpha = 2, kKeepFlag = 8;
    const u32 name = guest.frame();
    const auto texture = [&](u32 file, u32 shape, u32 format, u32 record, u32 second) {
        return guest::direct<0x4d4080, frontend::rende::loadShapeTexture>(app, ts, file, shape, format, record, second);
    };
    // The bar's per cent, a float, in the word the code pushes, the bar handed its address.
    const auto progress = [&](u32 perCent) {
        const u32 pushed = ts.sp - 4;
        guest.setDword(pushed, perCent);
        fe::direct<0x495230, graphics::loadingProgress>(app, ts, pushed);
    };

    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, 0x549458, guest::Words{kSfxFolder});
    // Its flags in edx, as the call before leaves it.
    const u32 file = *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, 0);
    progress(kProgress36);
    for (u32 line = 0; line < kLineCount; ++line)
    {
        guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, 0x549464, guest::Words{line});
        const u32 record = kLines + line * kTextureBytes;
        texture(file, name, kFormat16, record, 0);
        guest.setByte(record + kTextureFlags, uint8_t(guest.byte(record + kTextureFlags) | kTextureOwnPalette));
        guest.setDword(kLineList + line * 4, record);
    }
    texture(file, 0x54946c, kFormat16, kShadow, 0);
    guest.setDword(kShadowPointer, kShadow);
    for (u32 skid = 0; skid < 4; ++skid)
    {
        texture(file, 0x549474 + skid * 8, kFormat16, kSkidRecords[skid], 0);
        guest.setDword(kSkidTextures + skid * 4, kSkidRecords[skid]);
    }
    progress(kProgress37);
    guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
    texture(file, 0x549494, kFormat16, kFont, 0);
    guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
    texture(file, 0x54949c, kFormatWide, kGlows[0], kGlowSecond);
    texture(file, 0x5494a4, kFormat16, kGlows[1], kGlowSecond);
    fe::direct<0x431900, graphics::sendState>(app, ts, kBlendState, 0u);
    texture(file, 0x5494ac, kFormat256, kGlows[2], kGlowSecond);
    fe::direct<0x431900, graphics::sendState>(app, ts, kBlendState, 1u);
    texture(file, 0x5494b4, kFormat16, kGlows[3], kGlowSecond);
    texture(file, 0x5494bc, kFormatWide, kGlows[4], kGlowSecond);
    if ((guest.byte(kDisplayFlags) & kCardWithAlpha) != 0)
    {
        const u32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, ts, file, 0x5494c4u);
        fe::direct<0x4d41a0, render::paletteTextured>(app, ts, shape, kSparkTexture);
    }
    else
        texture(file, 0x5494cc, kFormat16, kSparkTexture, 0);
    progress(kProgress38);
    guest::direct<0x4cb590, loadParticleTextures>(app, ts, file);
    progress(kProgress39);
    texture(file, 0x5494d4, kFormat16, kMouse, 0);
    u32 left = 0;
    for (u32 dial = 0; dial < 4; ++dial)
        left = texture(file, 0x5494dc + dial * 8, kFormat16, kDials[dial], 0);
    if (file != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
    for (const u32 glow : { kGlows[0], kGlows[4], kGlows[3] })
        guest.setByte(glow + kTextureFlags, uint8_t(guest.byte(glow + kTextureFlags) | kKeepFlag));
    return left;
}


/* sub_4d56f0: sfx.fsh loaded, the round robin from its start. */
nfs::u32 frontend::rende::loadEffects(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    guest.setByte(kRoundRobinNext, 0);
    guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
    guest::direct<0x4d5370, loadEffectTextures>(app, ts);
    return guest::direct<0x4a33d0, loada::stopCheck>(app, ts);
}


nfs::u32 frontend::rendSkidAddSegment(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c6cb0", 4, 0, nullptr, 0, kKeepsEsiEdi };
    const u32 from = args.eax, to = args.edx, alphaFrom = args.ebx, alphaTo = args.ecx;
    const u32 mode = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return rende::skidAddSegment(app, ts, from, to, alphaFrom, alphaTo, mode);
    });
}

nfs::u32 frontend::rendSkidMarksDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c6f00", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::skidMarksDraw(app, ts, view); });
}

nfs::u32 frontend::rendPolygonDepths(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7180", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const u32 list = args.eax, least = args.edx, scale = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::polygonDepths(app, ts, list, least, scale); });
}

nfs::u32 frontend::rendPolygonsIntoBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7260", 8, 0, nullptr, 0 };
    const u32 list = args.eax;
    const u32 least = stackArgument(app, ts, 0), scale = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::polygonsIntoBuckets(app, ts, list, least, scale); });
}

nfs::u32 frontend::rendPolygonsSorted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7380", 8, 0, nullptr, 0, kKeepsAllButEsi };
    const u32 result = args.esi;
    const u32 list = stackArgument(app, ts, 0), last = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::polygonsSorted(app, ts, result, list, last); });
}

/* sub_4c73e0: the depth buckets cleared, when the polygons are sorted here. */
nfs::u32 frontend::rende::clearDepthBuckets(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    if (guest.dword(kDrawnOnCard) == 0)
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kDepthHeads, kDepthBytes);
    return 0;  // nothing: the thunk leaves eax as it came
}

nfs::u32 frontend::rendClearDepthBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c73e0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { rende::clearDepthBuckets(app, ts); return args.eax; });
}

nfs::u32 frontend::rendDepthBucketAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7400", 12, 0, nullptr, 0 };
    const u32 view = args.eax;
    const u32 first = stackArgument(app, ts, 0), last = stackArgument(app, ts, 1), depth = stackArgument(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::depthBucketAppend(app, ts, view, first, last, depth); });
}

nfs::u32 frontend::rendDepthBucketsAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7480", 4, 0, nullptr, 0 };
    const u32 list = args.eax;
    const u32 base = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::depthBucketsAdd(app, ts, list, base); });
}

nfs::u32 frontend::rendDepthBucketsGather(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7580", 0, 0, nullptr, 0, kKeepsAllButEsi };
    const u32 result = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::depthBucketsGather(app, ts, result); });
}

/* sub_4c75f0: the far buckets cleared, on the card. */
nfs::u32 frontend::rende::clearFarBuckets(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    if (guest.dword(kDrawnOnCard) != 0)
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kFarHeads, kFarBytes);
    return 0;  // nothing: the thunk leaves eax as it came
}

nfs::u32 frontend::rendClearFarBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c75f0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { rende::clearFarBuckets(app, ts); return args.eax; });
}

nfs::u32 frontend::rendFarBucketsGather(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c76e0", 0, 0, nullptr, 0, kKeepsAllButEsi };
    const u32 result = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::farBucketsGather(app, ts, result); });
}

/* sub_4c7750: on the card, the near buckets cleared, and the depth ones when `depthToo`. */
nfs::u32 frontend::rende::clearBuckets(App* app, ThreadState& ts, u32 depthToo)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    if (guest.dword(kDrawnOnCard) != 0)
    {
        if (depthToo != 0)
            fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kDepthHeads, kDepthBytes);
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kNearBuckets, kNearBytes);
    }
    return depthToo;
}

nfs::u32 frontend::rendClearBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7750", 0, 0, nullptr, 0, kKeepsEdx };
    const u32 depthToo = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::clearBuckets(app, ts, depthToo); });
}

nfs::u32 frontend::rendDrawListAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7940", 12, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const u32 first = stackArgument(app, ts, 0), last = stackArgument(app, ts, 1), base = stackArgument(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::drawListAdd(app, ts, first, last, base); });
}

nfs::u32 frontend::rendBlinkingQuads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9f70", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::blinkingQuads(app, ts, view); });
}

nfs::u32 frontend::rendFadeParticles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cb220", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::fadeParticles(app, ts, view); });
}

nfs::u32 frontend::rendLoadParticleTextures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cb590", 0, 0, kParticleNames, std::size(kParticleNames) };
    const u32 file = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::loadParticleTextures(app, ts, file); });
}

/* sub_4cb6f0: the particles' states and fades cleared. */
nfs::u32 frontend::rende::clearParticles(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kParticleStates, kParticleStatesBytes);
    const u32 left = fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kFades, kFadesBytes);
    guest.setDword(kFadeCount, 0);
    guest.setDword(kFadeCount + 4, 0);
    return left;
}

nfs::u32 frontend::rendClearParticles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cb6f0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::clearParticles(app, ts); });
}

/* sub_4cb730: the particles' states cleared. */
nfs::u32 frontend::rende::clearParticleStates(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kParticleStates, kParticleStatesBytes);
}

nfs::u32 frontend::rendClearParticleStates(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cb730", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::clearParticleStates(app, ts); });
}

nfs::u32 frontend::rendParticlesCameraChanged(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc540", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::particlesCameraChanged(app, ts, view); });
}

nfs::u32 frontend::rendParticlesPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc5c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::particlesPass(app, ts, view); });
}

nfs::u32 frontend::rendDropsPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc600", 0, 0, nullptr, 0, kKeepsEdxEdi };
    const u32 view = args.eax, result = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::dropsPass(app, ts, view, result); });
}

/* sub_4d18b0: how many bits a texture's format has. */
nfs::u32 frontend::rende::formatBits(App* app, ThreadState& ts, u32 format)
{
    static_cast<void>(app);
    static_cast<void>(ts);
    return format == 2 ? 8 : format == 6 ? 0x20 : 0x10;
}

nfs::u32 frontend::rendFormatBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d18b0", 0, 0, nullptr, 0, kKeepsEdx };
    const u32 format = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::formatBits(app, ts, format); });
}

nfs::u32 frontend::rendExpandPalettedShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d3380", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const u32 shape = args.eax, to = args.edx, palette = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::expandPalettedShape(app, ts, shape, to, palette); });
}

nfs::u32 frontend::rendLoadShapeTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d4080", 4, 0, &kAdditive, 1, kKeepsEsiEdi };
    const u32 file = args.eax, named = args.edx, format = args.ebx, record = args.ecx;
    const u32 second = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return rende::loadShapeTexture(app, ts, file, named, format, record, second);
    });
}

/* sub_4d4120: the greatest power of two up to 256 no more than the lesser of the two, 1 at least. */
nfs::u32 frontend::rende::fitPowerOfTwo(App* app, ThreadState& ts, u32 a, u32 b)
{
    static_cast<void>(app);
    static_cast<void>(ts);
    const s32 least = s32(a) > s32(b) ? s32(b) : s32(a);
    for (s32 power = 0x100; power >= 2; power >>= 1)
        if (least >= power)
            return u32(power);
    return 1;
}

nfs::u32 frontend::rendFitPowerOfTwo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d4120", 0, 0, nullptr, 0 };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::fitPowerOfTwo(app, ts, a, b); });
}

nfs::u32 frontend::rendCopyCabinBar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d4440", 0, 0, nullptr, 0, kKeepsEsiEdi };
    const u32 bar = args.eax, shape = args.edx, buffer = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::copyCabinBar(app, ts, bar, shape, buffer); });
}

nfs::u32 frontend::rendLoadEffectTextures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d5370", 0, 0, kSfxStrings, std::size(kSfxStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::loadEffectTextures(app, ts); });
}

/* sub_4d56b0: an empty function; eax as it came (the thunk's). */
nfs::u32 frontend::rende::nothing(App* app, ThreadState& ts)
{
    static_cast<void>(app);
    static_cast<void>(ts);
    return 0;
}

nfs::u32 frontend::rendNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d56b0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { rende::nothing(app, ts); return args.eax; });
}

nfs::u32 frontend::rendLoadEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d56f0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::loadEffects(app, ts); });
}

nfs::u32 frontend::rendSkidSubdivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4db750", 8, 0, nullptr, 0, kKeepsEsiEdi };
    const u32 depth = args.eax, from = args.edx, to = args.ebx, alphaFrom = args.ecx;
    const u32 alphaTo = stackArgument(app, ts, 0), mode = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return rende::skidSubdivide(app, ts, depth, from, to, alphaFrom, alphaTo, mode);
    });
}

nfs::u32 frontend::rendSkidStrip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4db8b0", 4, 0, nullptr, 0, kKeepsEsiEdi };
    const u32 centre = args.eax, from = args.edx, to = args.ebx, axle = args.ecx;
    const u32 out = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::skidStrip(app, ts, centre, from, to, axle, out); });
}

nfs::u32 frontend::rendSkidWheel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4db940", 8, 0, nullptr, 0, kKeepsEsiEdi };
    const u32 car = args.eax, wheel = args.edx, position = args.ebx, axle = args.ecx;
    const u32 released = stackArgument(app, ts, 0), strength = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return rende::skidWheel(app, ts, car, wheel, position, axle, released, strength);
    });
}

/* sub_4dbcc0: the camera's horizontal angle by View Angle: 75 narrow, else 90 or the port's 105. */
nfs::u32 frontend::rende::defaultFieldOfView(App* app, ThreadState& ts)
{
    static_cast<void>(ts);
    return gameSettings(app).viewAngle == 1 ? 75 : viewAngleFor(app, 90);
}

nfs::u32 frontend::rendDefaultFieldOfView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dbcc0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::defaultFieldOfView(app, ts); });
}

nfs::u32 frontend::rendSmokeEmit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dd930", 12, 0, nullptr, 0 };
    const u32 view = args.eax;
    const u32 pointX = stackArgument(app, ts, 0), pointY = stackArgument(app, ts, 1), pointZ = stackArgument(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::smokeEmit(app, ts, view, pointX, pointY, pointZ); });
}

nfs::u32 frontend::rendSparksPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dedf0", 0, 0, nullptr, 0, kKeepsEdxEdi };
    const u32 view = args.eax, result = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rende::sparksPass(app, ts, view, result); });
}

}
