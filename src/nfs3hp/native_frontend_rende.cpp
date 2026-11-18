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
#include "thrash_calls.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <cstring>
#include <iterator>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
x86::reg32 rendDrawQuadClipped(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDrawQuadSplit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSkidAddSegment(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSkidMarksDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPolygonDepths(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPolygonsIntoBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPolygonsSorted(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendClearDepthBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDepthBucketAppend(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDepthBucketsAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDepthBucketsGather(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendClearFarBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFarBucketsGather(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendClearBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDrawListAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendBlinkingQuads(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFadeParticles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendLoadParticleTextures(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendClearParticles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendClearParticleStates(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendParticlesCameraChanged(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendParticlesPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDropsPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFormatBits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendExpandPalettedShape(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendLoadShapeTexture(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFitPowerOfTwo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendCopyCabinBar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendLoadEffectTextures(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendNothing(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendLoadEffects(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSkidSubdivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSkidStrip(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSkidWheel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendDefaultFieldOfView(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSmokeEmit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSparksPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}
}

/* What the race's drawing does around the port's vertex and polygon code,
 * decompiled:
 *
 *   a quad on the card     clipped by its corners' codes, drawn whole, as two
 *                          triangles or through the polygon clipper (sub_4c1900,
 *                          sub_4c19e0);
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
x86::reg32 viewAngleFor(win32::WinApplication* app, x86::reg32 angle);

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using x86::reg32;
using x86::sreg32;
using std::uint8_t;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline reg32 callAt(Guest& guest, reg32 function, const fe::Arguments& arguments = fe::Arguments(), reg32 esi = 0,
                    double* floatResult = nullptr)
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<reg32> result = guest::pointerCall(guest.app(), cpu, function, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

/* A game function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

/* A register's low byte and second byte, the rest as they are: what a function
 * leaves in the registers its callers do not count on. */
constexpr reg32 withLow(reg32 value, uint8_t low)
{
    return (value & 0xffffff00u) | low;
}

constexpr reg32 withHigh(reg32 value, uint8_t high)
{
    return (value & 0xffff00ffu) | (reg32(high) << 8);
}

/* A float from its bits, as a stack argument holds it. */
float floatOfBits(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A float of the game's, as fld dword reads it. */
double floatOf(const Guest& guest, reg32 address)
{
    return guest.floatAt(address);
}

/* A double of the game's constants, as fld and fmul qword read it. */
double doubleOf(const Guest& guest, reg32 address)
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
void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 at = 0; at < count * 4; at += 4)
        guest.setDword(to + at, guest.dword(from + at));
}

/* An index held to a table of `last` and one. */
reg32 clampedBucket(sreg32 index, sreg32 last)
{
    return index < 0 ? 0 : index > last ? reg32(last) : reg32(index);
}

// The registers a function's pushes keep; the rest it leaves as they fall.
constexpr std::uint8_t kKeepsEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsEcxEsiEdi = fe::kRestoreEcx | kKeepsEsiEdi;
constexpr std::uint8_t kKeepsEdx = fe::kRestoreEdx;
constexpr std::uint8_t kKeepsEdxEdi = fe::kRestoreEdx | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsAllButEsi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi;

// The game's functions, by address.
constexpr reg32 kClipPolygon = 0x4c2cd0;       // eax three corners, edx their codes, ebx the codes' or
constexpr reg32 kDepthBuckets = 0x4c7790;      // eax a list, the stack its depth
constexpr reg32 kSortedBuckets = 0x4c7610;
constexpr reg32 kGatherSorted = 0x4c7320;      // esi where the list goes
constexpr reg32 kFillZero = 0x4e070c;          // eax where, edx how many bytes
constexpr reg32 kRandom = 0x4e0d88;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kStrstr = 0x4e0820;

// The race's data.
constexpr reg32 kDisplayFlags = 0x7a3a58;      // byte; bit 2 a card with alpha
constexpr reg32 kDrawnOnCard = 0x7a3a60;       // 0: polygons sorted here, no z-buffer
constexpr reg32 kFrameTime = 0x7d3684;

/* ---------------------------------------------------------------------------
 * A quad on the card: sub_4c1900, sub_4c19e0
 * ------------------------------------------------------------------------- */

/* One of the driver's functions through the game's pointer to it (thrash_calls.h): its eax,
 * Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 driverCall(Guest& guest, Words... words)
{
    const std::optional<reg32> eax = guest::driver<Slot, Function>(guest.app(), guest.cpu(), words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

constexpr reg32 kVertexCode = 0x14;            // a corner's clip code, a byte

/* The frame both have: a triangle's three corners and the four corners' codes. */
constexpr reg32 kQuadFrame = 0x14;
constexpr reg32 kQuadCodes = 0xc;              // from the frame's bottom

/* The second triangle's corners and codes for the clipper. */
reg32 clipSecondTriangle(Guest& guest, reg32 a, reg32 c, reg32 d, uint8_t codeC, uint8_t codeD, uint8_t any)
{
    const reg32 corners = guest.frame();
    const reg32 codes = corners + kQuadCodes;
    guest.setDword(corners, a);
    guest.setDword(corners + 4, c);
    guest.setDword(corners + 8, d);
    guest.setByte(codes + 1, codeC);
    guest.setByte(codes + 2, codeD);
    return callAt(guest, kClipPolygon, args({ corners, codes, any }));
}

reg32 clipFirstTriangle(Guest& guest, reg32 a, reg32 b, reg32 c, uint8_t any)
{
    const reg32 corners = guest.frame();
    guest.setDword(corners, a);
    guest.setDword(corners + 4, b);
    guest.setDword(corners + 8, c);
    return callAt(guest, kClipPolygon, args({ corners, corners + kQuadCodes, any }));
}

/* The four corners' codes, in the frame where the clipper reads them. */
void noteQuadCodes(Guest& guest, reg32 a, reg32 b, reg32 c, reg32 d)
{
    const reg32 codes = guest.frame() + kQuadCodes;
    guest.setByte(codes, guest.byte(a + kVertexCode));
    guest.setByte(codes + 1, guest.byte(b + kVertexCode));
    guest.setByte(codes + 2, guest.byte(c + kVertexCode));
    guest.setByte(codes + 3, guest.byte(d + kVertexCode));
}

}  // namespace

/* sub_4c1900: the quad a b c d: nothing when one plane has all four out, the
 * driver's quad when none is out, else the triangles a b c and a c d, each
 * skipped, drawn or clipped by its own corners' codes. */
x86::reg32 frontend::rende::drawQuadClipped(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c, reg32 d)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + kQuadFrame);
    // eax as the code has it, a first and what each call leaves; the other registers are left, not read.
    reg32 eax = a;
    noteQuadCodes(guest, a, b, c, d);
    const reg32 codes = guest.frame() + kQuadCodes;
    const uint8_t codeA = guest.byte(codes), codeB = guest.byte(codes + 1), codeC = guest.byte(codes + 2), codeD = guest.byte(codes + 3);
    const uint8_t outOfFirst = uint8_t(codeA & codeB & codeC);
    const uint8_t anyOfFirst = uint8_t(codeA | codeB | codeC);
    if ((outOfFirst & codeD) != 0)
        return withHigh(withLow(eax, uint8_t(outOfFirst & codeD)), outOfFirst);
    if ((anyOfFirst | codeD) == 0)
        driverCall<thrash::kThrashDrawQuad, &port::driver::drawQuad>(guest, a, b, c, d);
        return eax;

    if (outOfFirst != 0)
        eax = withHigh(eax, outOfFirst);
    else if (anyOfFirst == 0)
        driverCall<thrash::kThrashDrawTri, &port::driver::drawTri>(guest, a, b, c);  // void: eax as it was
    else
        eax = clipFirstTriangle(guest, a, b, c, anyOfFirst);

    // The codes again: the clipper has had them.
    const uint8_t again = guest.byte(codes), againC = guest.byte(codes + 2), againD = guest.byte(codes + 3);
    const uint8_t outOfSecond = uint8_t(again & againC & againD);
    if (outOfSecond != 0)
        return withLow(eax, outOfSecond);
    const uint8_t anyOfSecond = uint8_t(again | againC | againD);
    if (anyOfSecond == 0)
    {
        driverCall<thrash::kThrashDrawTri, &port::driver::drawTri>(guest, a, c, d);
        return eax;
    }
    return clipSecondTriangle(guest, a, c, d, againC, againD, anyOfSecond);
}


/* sub_4c19e0: the same, never as one quad: the triangles a b c and a c d. */
x86::reg32 frontend::rende::drawQuadSplit(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c, reg32 d)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + kQuadFrame);
    // eax as the code has it, a first and what each call leaves; the other registers are left, not read.
    reg32 eax = a;
    noteQuadCodes(guest, a, b, c, d);
    const reg32 codes = guest.frame() + kQuadCodes;
    const uint8_t codeA = guest.byte(codes), codeB = guest.byte(codes + 1), codeC = guest.byte(codes + 2), codeD = guest.byte(codes + 3);
    const uint8_t outOfFirst = uint8_t(codeA & codeB & codeC);
    if ((outOfFirst & codeD) != 0)
        return withHigh(withLow(eax, outOfFirst), uint8_t(outOfFirst & codeD));

    if (outOfFirst == 0)
    {
        const uint8_t anyOfFirst = uint8_t(codeA | codeB | codeC);
        if (anyOfFirst == 0)
            driverCall<thrash::kThrashDrawTri, &port::driver::drawTri>(guest, a, b, c);  // void: eax as it was
        else
            eax = clipFirstTriangle(guest, a, b, c, anyOfFirst);
    }

    // The codes again: the clipper has had them.
    const uint8_t again = guest.byte(codes), againC = guest.byte(codes + 2), againD = guest.byte(codes + 3);
    const uint8_t outOfSecond = uint8_t(again & againC & againD);
    if (outOfSecond != 0)
        return withHigh(withLow(eax, outOfSecond), againD);
    const uint8_t anyOfSecond = uint8_t(again | againC | againD);
    if (anyOfSecond == 0)
    {
        driverCall<thrash::kThrashDrawTri, &port::driver::drawTri>(guest, a, c, d);
        return eax;
    }
    return clipSecondTriangle(guest, a, c, d, againC, againD, anyOfSecond);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The skid marks: sub_4db940, sub_4db8b0, sub_4db750, sub_4c6cb0, sub_4c6f00
 * ------------------------------------------------------------------------- */

constexpr reg32 kSkidStrips = 0x560110;        // the strips' block
constexpr reg32 kSkidStripBytes = 0x340;
constexpr reg32 kSkidStripNow = 0x7d5754;      // the one being laid
constexpr reg32 kSkidStripsUsed = 0x7d5758;    // how many have been, 64 at most
constexpr reg32 kSkidDrawnAt = 0x7d5750;       // the time they were last drawn
constexpr sreg32 kSkidStripsMost = 0x40;
constexpr reg32 kSkidSegmentsMost = 0x10;

// A strip.
constexpr reg32 kStripOrigin = 0;              // a point, three floats
constexpr reg32 kStripCount = 0xc;             // its segments
constexpr reg32 kStripCorners = 0x10;          // four corners of 12 bytes a segment
constexpr reg32 kStripCornersBytes = 48;
constexpr reg32 kStripTextures = 0x310;        // a byte a segment: which texture
constexpr reg32 kStripAlphas = 0x320;          // two bytes a segment: how dark at its ends

constexpr reg32 kSkidLift = 0x540218;          // a float: how far above the road they lie
constexpr reg32 kSkidFarApart = 0x54021c;      // a float: past this from the origin, a new strip
constexpr reg32 kSkidTextures = 0x8b41c8;      // a texture's record, by a segment's byte

/* What sub_4c6cb0 asks of one axis: whether the two are as far apart as its bound. */
bool farApartOnAxis(const Guest& guest, reg32 origin, reg32 point, double bound)
{
    const float difference = float(fp::sub(floatOf(guest, origin), floatOf(guest, point)));
    const float size = jumpsBelowOrEqual(0.0, difference) ? difference : float(-double(difference));
    return !jumpsBelow(size, bound);
}

reg32 skidStripOf(const Guest& guest, reg32 index)
{
    return guest.dword(kSkidStrips) + index * kSkidStripBytes;
}

}  // namespace

/* sub_4c6cb0: a segment from `from` to `to` (two sides each, six floats) with
 * its ends' alphas, on the strip being laid, or a new one when that one is full
 * or far from here; `mode` 0 lays none, 2 is the textures' second pair. */
x86::reg32 frontend::rende::skidAddSegment(App* app, CPU& cpu, reg32 from, reg32 to, reg32 alphaFrom, reg32 alphaTo, reg32 mode)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 0x4c);
    constexpr reg32 kToSides = 0, kFromSides = 0x18;
    const reg32 frame = guest.frame();
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
    const reg32 last = guest.dword(from + 0x14);
    guest.setDword(frame + kFromSides + 0x14, last);
    if (mode == 0)
        return last;

    reg32 strip = skidStripOf(guest, guest.dword(kSkidStripNow));
    bool newStrip = guest.dword(strip + kStripCount) == kSkidSegmentsMost;
    if (!newStrip)
    {
        const double bound = floatOf(guest, kSkidFarApart);
        newStrip = farApartOnAxis(guest, strip, to, bound) || farApartOnAxis(guest, strip + 4, to + 4, bound)
                   || farApartOnAxis(guest, strip + 8, to + 8, bound);
    }
    if (newStrip)
    {
        const reg32 used = guest.dword(kSkidStripsUsed);
        if (sreg32(used) < kSkidStripsMost)
        {
            guest.setDword(kSkidStripNow, used);
            guest.setDword(kSkidStripsUsed, used + 1);
        }
        else
        {
            const reg32 now = guest.dword(kSkidStripNow);
            guest.setDword(kSkidStripNow, sreg32(now) < kSkidStripsMost - 1 ? now + 1 : 0);
        }
        strip = skidStripOf(guest, guest.dword(kSkidStripNow));
        guest.setDword(strip + kStripCount, 0);
    }
    if (newStrip || guest.dword(strip + kStripCount) == 0)
        copyDwords(guest, strip + kStripOrigin, to, 3);

    const reg32 count = guest.dword(strip + kStripCount);
    const reg32 corners = strip + kStripCorners + count * kStripCornersBytes, alphas = strip + kStripAlphas + 2 * count;
    // As the call by address leaves them, for the end (edx's high bytes with the texture).
    fe::direct<0x4c6c70, frontend::sime::skidSegmentPut>(app, cpu, corners, alphas, frame + kFromSides, frame + kToSides,
                                                         alphaFrom & 0xff, alphaTo & 0xff);
    const reg32 random = callAt(guest, kRandom);
    uint8_t texture = uint8_t(random & 1);
    const reg32 countNow = guest.dword(strip + kStripCount);
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
x86::reg32 frontend::rende::skidSubdivide(App* app, CPU& cpu, reg32 depth, reg32 from, reg32 to, reg32 alphaFrom, reg32 alphaToWord, reg32 mode)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 0x28);
    const uint8_t alphaToIn = uint8_t(alphaToWord);
    constexpr reg32 kShortEnough = 0x41600000;  // 14.0f
    constexpr reg32 kHalf = 0x54970c;           // a double
    constexpr sreg32 kDeepest = 3;
    const uint8_t alphaTo = mode == 2 ? uint8_t(0) : alphaToIn;
    const reg32 alphaEnd = alphaFrom & 0xff;
    if (fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, cpu, from, to, kShortEnough) != 0
        || sreg32(depth) >= kDeepest)
        return guest::direct<0x4c6cb0, skidAddSegment>(app, cpu, from, to, alphaEnd, reg32(alphaTo), mode);

    const reg32 middle = guest.frame();
    const double half = doubleOf(guest, kHalf);
    for (reg32 at = 0; at < 0x18; at += 4)
    {
        const double sum = fp::add(floatOf(guest, from + at), floatOf(guest, to + at));
        guest.setFloat(middle + at, float(at == 0x14 ? fp::mul(half, sum) : fp::mul(sum, half)));
    }
    const reg32 alphaMiddle = ((reg32(alphaTo) + alphaEnd) >> 1) & 0xff;
    guest::direct<0x4db750, skidSubdivide>(app, cpu, depth + 1, from, middle, alphaEnd, alphaMiddle, mode);
    return guest::direct<0x4db750, skidSubdivide>(app, cpu, depth + 1, middle, to, alphaMiddle, reg32(alphaTo), mode);
}


/* sub_4db8b0: the two sides of a mark at `centre`, as wide across the way from
 * `from` to `to` and the wheel's axle `axle`. */
x86::reg32 frontend::rende::skidStrip(App* app, CPU& cpu, reg32 centre, reg32 from, reg32 to, reg32 axle, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 0x18);
    constexpr reg32 kHalfWidth = 0x549714;      // a double
    const reg32 way = guest.frame();
    const reg32 across = way + 0xc;
    guest.setFloat(way, float(fp::sub(floatOf(guest, to), floatOf(guest, from))));
    guest.setFloat(way + 4, float(fp::sub(floatOf(guest, to + 4), floatOf(guest, from + 4))));
    guest.setFloat(way + 8, float(fp::sub(floatOf(guest, to + 8), floatOf(guest, from + 8))));
    fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, way);
    // As the call by address leaves them.
    const reg32 left = fe::direct<0x4e0160, frontend::simd::cross>(app, cpu, way, axle, across);

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
constexpr reg32 kWheelMarks = 0x8b7ba0;
constexpr reg32 kWheelMarksPerCar = 0xd0;
constexpr reg32 kWheelMarkBytes = 0x34;
constexpr reg32 kMarkTime = 0;
constexpr reg32 kMarkSides = 4;                // two sides, three floats each
constexpr reg32 kMarkPoint = 0x1c;             // where it last was
constexpr reg32 kMarkAlpha = 0x28;             // a byte
constexpr reg32 kMarkSliding = 0x2c;           // whether it was, at its last call
constexpr reg32 kMarkFresh = 0x30;             // nothing laid since it started

}  // namespace

/* sub_4db940: a car's wheel at `position` and with axle `axle` (16.16 integers),
 * sliding or not as `released` is clear or set, as hard as `strength`: a mark
 * begun, laid on or ended. */
x86::reg32 frontend::rende::skidWheel(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 position, reg32 axle, reg32 released, reg32 strength)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 0x40);
    constexpr reg32 kToFloat = 0x54971c, kToAlpha = 0x549724;   // doubles
    constexpr reg32 kLayFrom = 0x3f000000;      // 0.5f: moved this far, more is laid
    constexpr reg32 kEndFrom = 0x3e4ccccd;      // 0.2f
    const reg32 frame = guest.frame();
    const reg32 ebp = frame + 0x40;
    const reg32 sides = ebp - 0x40, axleNow = ebp - 0x28, here = ebp - 0x1c;

    const double toFloat = doubleOf(guest, kToFloat);
    for (reg32 at = 0; at < 12; at += 4)
        guest.setFloat(here + at, float(fp::mul(sreg32(guest.dword(position + at)), toFloat)));
    for (reg32 at = 0; at < 8; at += 4)
        guest.setFloat(axleNow + at, float(fp::mul(sreg32(guest.dword(axle + at)), toFloat)));
    guest.setFloat(axleNow + 8, float(fp::mul(toFloat, sreg32(guest.dword(axle + 8)))));
    const reg32 mark = kWheelMarks + (car & 0xf) * kWheelMarksPerCar + wheel * kWheelMarkBytes;

    // How hard, from 0 to 1, to an alpha of 0 to 255 by 143 >> 16.
    const reg32 given = strength;
    const reg32 atLeastZero = jumpsBelowOrEqual(0.0, floatOfBits(strength)) ? given : 0;
    float least;
    std::memcpy(&least, &atLeastZero, sizeof least);
    const float bounded = jumpsBelowOrEqual(1.0, least) ? 1.0f : least;
    const sreg32 scaled = fp::toInteger<sreg32>(fp::mul(bounded, doubleOf(guest, kToAlpha)));
    const reg32 alpha = reg32(sreg32(reg32(scaled) * 143u) >> 16);

    if (released == 0)
    {
        const reg32 since = guest.dword(kFrameTime) - guest.dword(mark + kMarkTime);
        if (sreg32(since) <= 0)
            return since;
    }
    const reg32 now = guest.dword(kFrameTime);
    const reg32 sliding = guest.dword(mark + kMarkSliding);
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

    const reg32 point = mark + kMarkPoint;
    const reg32 markSides = mark + kMarkSides;
    // Nonzero when the mark's point is within `limit` of here; edx as the call by address leaves it.
    const auto nearHere = [&](reg32 limit) {
        return fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, cpu, point, here, limit);
    };
    if (released != 0)
    {
        // The wheel has stopped sliding: the last of the mark, fading out.
        if (guest.dword(mark + kMarkFresh) == 0 && nearHere(kEndFrom) == 0)
        {
            guest::direct<0x4db8b0, skidStrip>(app, cpu, here, point, here, axleNow, sides);
            guest::direct<0x4db750, skidSubdivide>(app, cpu, 0u, markSides, sides, reg32(guest.byte(mark + kMarkAlpha)), 0u, 2u);
            copyDwords(guest, markSides, sides, 3);
            copyDwords(guest, markSides + 12, sides + 12, 3);
            copyDwords(guest, point, here, 3);
        }
        guest.setDword(mark + kMarkSliding, 0);
        return mark;
    }

    // Sliding on: laid on when it has moved far enough.
    const reg32 within = nearHere(kLayFrom);
    if (within != 0)
        return within;
    guest::direct<0x4db8b0, skidStrip>(app, cpu, here, point, here, axleNow, sides);
    if (guest.dword(mark + kMarkFresh) != 0)
    {
        guest::direct<0x4db8b0, skidStrip>(app, cpu, point, point, here, axleNow, markSides);
        guest.setDword(mark + kMarkFresh, 0);
    }
    guest::direct<0x4db750, skidSubdivide>(app, cpu, 0u, markSides, sides, reg32(guest.byte(mark + kMarkAlpha)), alpha & 0xff, 1u);
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
constexpr reg32 kPolygonNext = 0;
constexpr reg32 kPolygonKind = 4;              // a word: 0 and 2 quads, 1 and 3 triangles
constexpr reg32 kPolygonLayer = 6;             // a word
constexpr reg32 kPolygonCorners = 8;           // four pointers
constexpr reg32 kPolygonTexture = 0x18;
constexpr reg32 kPolygonDepth = 0x1c;          // a float
constexpr reg32 kPolygonBytes = 0x20;
constexpr reg32 kVertexBytes = 0x20;
constexpr reg32 kVertexDepth = 8;              // a float
constexpr reg32 kVertexColour = 0x10;

}  // namespace

/* sub_4c6f00: the strips near the view, newest first, drawn as one list. */
x86::reg32 frontend::rende::skidMarksDraw(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x24);
    constexpr reg32 kNearEnough = 0x42480000;   // 50.0f
    constexpr reg32 kViewPosition = 8, kViewCamera = 0x38, kViewMatrix = 0x44;
    constexpr reg32 kSkidDepth = 0x7dcfec;
    constexpr reg32 kOpaque = 0xffffff;
    const reg32 frame = guest.frame();
    const reg32 verticesAt = frame, polygonAt = frame + 8;   // where the allocator puts its blocks
    reg32 first = 0, last = 0;
    if (gameSettings(guest).effectsLevel == 2)
        return view;

    for (reg32 index = guest.dword(kSkidStripsUsed); --index != reg32(-1);)
    {
        const reg32 strip = skidStripOf(guest, index);
        if (fe::direct<0x496f20, frontend::rendc::withinFlatDistance>(app, cpu, strip, view + kViewPosition,
                                                                      kNearEnough) == 0)
            continue;
        const reg32 corners = guest.dword(strip + kStripCount) << 2;
        if (corners == 0)
            continue;
        if (guest::direct<0x4bbde0, rendd::arenaTake>(app, cpu, verticesAt, corners << 5) == 0)
            continue;
        reg32 vertices = guest.dword(verticesAt);
        reg32 segment = guest.dword(strip + kStripCount);
        fe::direct<0x4bf4c0, vertices::objectVerticesOf>(app, cpu, std::int32_t(corners), strip + kStripCorners, view + kViewMatrix,
                                                         view + kViewCamera, vertices);
        fe::direct<0x431c20, collide::layerNothing>(app, cpu, segment, 5u);
        reg32 texture = strip + kStripTextures, alpha = strip + kStripAlphas;
        while (--segment != reg32(-1))
        {
            if (guest::direct<0x4bbde0, rendd::arenaTake>(app, cpu, polygonAt, kPolygonBytes) != 0)
            {
                const reg32 polygon = guest.dword(polygonAt);
                if (first == 0)
                    first = polygon;
                else
                    guest.setDword(last, polygon);
                guest.setDword(polygon + kPolygonNext, 0);
                last = polygon;
                guest.setWord(polygon + kPolygonKind, 0);
                guest.setWord(polygon + kPolygonLayer, 0);
                for (reg32 corner = 0; corner < 4; ++corner)
                    guest.setDword(polygon + kPolygonCorners + 4 * corner, vertices + corner * kVertexBytes);
                guest.setDword(polygon + kPolygonTexture, guest.dword(kSkidTextures + 4 * guest.byte(texture)));
                const reg32 colourNear = (reg32(guest.byte(alpha)) << 24) | kOpaque;
                guest.setDword(vertices + kVertexBytes + kVertexColour, colourNear);
                guest.setDword(vertices + kVertexColour, colourNear);
                const reg32 colourFar = (reg32(guest.byte(alpha + 1)) << 24) | kOpaque;
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
    fe::direct<0x4c7790, vertices::bucketsDepthedInto>(guest.app(), guest.cpu(), first, guest.dword(kSkidDepth));
    const reg32 now = guest.dword(kFrameTime);
    guest.setDword(kSkidDrawnAt, now);
    return now;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The depth buckets: without the z-buffer polygons go back to front
 * ------------------------------------------------------------------------- */

constexpr reg32 kDepthHeads = 0x7d5f5c;        // first and last of a list, by depth, 2000 of them
constexpr reg32 kDepthLast = 0x7d5f60;
constexpr reg32 kDepthBytes = 0x3e80;
constexpr reg32 kDepthDeepest = 0x7d9dd4;
constexpr sreg32 kDepthMost = 0x7cf;
constexpr reg32 kDepthBias = 0x7dcfdc;         // a float
constexpr reg32 kSortHeads = 0x7d9ddc;         // sub_4c7380's, 800 of them
constexpr reg32 kSortLast = 0x7d9de0;
constexpr reg32 kSortBytes = 0x1900;
constexpr reg32 kFarHeads = 0x7db6dc;          // 800 of them, on the card
constexpr reg32 kFarBytes = 0x1900;
constexpr reg32 kFarDeepest = 0x7dcfd4;
constexpr reg32 kNearBuckets = 0x7d575c;
constexpr reg32 kNearBytes = 0x800;
constexpr reg32 kPendingList = 0x7d5f54;       // first and last

}  // namespace

/* sub_4c7180: each polygon's depth, the average of its corners'; the least of
 * them at `least`, and at `scale` 799 over their range, so that sub_4c7260 finds
 * each its place among 800. */
x86::reg32 frontend::rende::polygonDepths(App* app, CPU& cpu, reg32 list, reg32 least, reg32 scale)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16 + 0x10);
    constexpr reg32 kQuarter = 0x540224, kThird = 0x540228, kBucketsLast = 0x54022c;    // floats
    constexpr reg32 kFarthest = 0x497423f0, kNearest = 0xc97423f0;                      // 1e6 and -1e6
    const reg32 most = guest.frame() + 8;   // [ebp-8]
    guest.setDword(least, kFarthest);
    guest.setDword(most, kNearest);
    reg32 left = list;
    reg32 polygon = list;
    for (; polygon != 0; polygon = guest.dword(polygon + kPolygonNext))
    {
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        double depth;
        const reg32 corners = polygon + kPolygonCorners;
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
    reg32 rangeBits;
    std::memcpy(&rangeBits, &range, sizeof rangeBits);
    if ((rangeBits & 0x7fffffff) == 0)
        guest.setDword(scale, polygon);
    else
        guest.setFloat(scale, float(fp::div(floatOf(guest, kBucketsLast), range)));
    return left;
}


/* sub_4c7260: each polygon into its bucket by depth, each bucket kept in order. */
x86::reg32 frontend::rende::polygonsIntoBuckets(App* app, CPU& cpu, reg32 list, reg32 least, reg32 scale)
{
    Guest guest(app, cpu);
    reg32 left = list;
    for (reg32 polygon = list; polygon != 0;)
    {
        const reg32 next = guest.dword(polygon + kPolygonNext);
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        left = next;
        if (kind <= 3)
        {
            const float depth = guest.floatAt(polygon + kPolygonDepth);
            const reg32 bucket = reg32(fp::toInteger<sreg32>(fp::mul(fp::sub(depth, floatOfBits(least)), floatOfBits(scale))));
            reg32 head = guest.dword(kSortHeads + bucket * 8);
            reg32 tail = guest.dword(kSortLast + bucket * 8);
            reg32 before = 0;
            reg32 at = head;
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
x86::reg32 frontend::rende::polygonsSorted(App* app, CPU& cpu, reg32 result, reg32 list, reg32 last)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x10);
    static_cast<void>(last);   // the list's last: not read
    const reg32 ebp = guest.frame() + 0x10;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kSortHeads, kSortBytes);
    guest::direct<0x4c7180, polygonDepths>(app, cpu, list, ebp - 8, ebp - 4);
    guest::direct<0x4c7260, polygonsIntoBuckets>(app, cpu, list, guest.dword(ebp - 8), guest.dword(ebp - 4));
    callAt(guest, kGatherSorted, fe::Arguments(), ebp - 0x10);  // esi: where the callee puts the list
    copyDwords(guest, result, ebp - 0x10, 2);
    return result;
}


/* sub_4c7400: a list onto the bucket of a depth: `first`, `last`, `depth`. */
x86::reg32 frontend::rende::depthBucketAppend(App* app, CPU& cpu, reg32 view, reg32 first, reg32 last, reg32 depth)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 8);
    const float depthValue = floatOfBits(depth);
    constexpr reg32 kBucketsPerUnit = 0x540230;  // a float
    if (guest.dword(kDrawnOnCard) != 0)
        return view;
    const double place = fp::mul(fp::add(depthValue, floatOf(guest, kDepthBias)), floatOf(guest, kBucketsPerUnit));
    const reg32 bucket = clampedBucket(fp::toInteger<sreg32>(place), kDepthMost) * 8;
    const reg32 joined = guest.frame();
    const reg32 left = guest::direct<0x4bbec0, rendd::joinLists>(app, cpu, joined, guest.dword(kDepthHeads + bucket),
                                                                 guest.dword(kDepthLast + bucket), first, last);
    copyDwords(guest, kDepthHeads + bucket, joined, 2);
    return left;
}


/* sub_4c7480: each polygon of a list onto its own bucket, by its corners' depth
 * less `base`. */
x86::reg32 frontend::rende::depthBucketsAdd(App* app, CPU& cpu, reg32 list, reg32 base)
{
    Guest guest(app, cpu);
    constexpr reg32 kQuarter = 0x540234, kThird = 0x540244;    // doubles
    constexpr reg32 kBucketsPerUnit = 0x54023c;                // a float
    reg32 left = list;
    if (guest.dword(kDrawnOnCard) != 0)
        return left;
    for (reg32 polygon = list; polygon != 0;)
    {
        const reg32 next = guest.dword(polygon + kPolygonNext);
        const std::uint16_t kind = guest.word(polygon + kPolygonKind);
        guest.setDword(polygon + kPolygonNext, 0);
        reg32 bucket = 0;
        const reg32 corners = polygon + kPolygonCorners;
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
            bucket = clampedBucket(fp::toInteger<sreg32>(depth), kDepthMost);
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
x86::reg32 frontend::rende::depthBucketsGather(App* app, CPU& cpu, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x10);
    const reg32 ebp = guest.frame() + 0x10;
    guest::direct<0x4bbe70, rendd::emptyList>(app, cpu, ebp - 8);
    reg32 first = guest.dword(ebp - 8), last = guest.dword(ebp - 4);
    if (guest.dword(kDrawnOnCard) == 0)
    {
        reg32 bucket = kDepthDeepest;
        for (reg32 i = 0; i < 2000; ++i, bucket -= 8)
        {
            if (first == 0)
            {
                first = guest.dword(bucket);
                last = guest.dword(bucket + 4);
                continue;
            }
            const reg32 head = guest.dword(bucket);
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
x86::reg32 frontend::rende::farBucketsGather(App* app, CPU& cpu, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x10);
    const reg32 ebp = guest.frame() + 0x10;
    guest::direct<0x4bbe70, rendd::emptyList>(app, cpu, ebp - 0x10);
    reg32 first = guest.dword(ebp - 0x10), last = guest.dword(ebp - 0xc);
    if (guest.dword(kDrawnOnCard) != 0)
    {
        reg32 bucket = kFarDeepest;
        for (reg32 i = 0; i < 800; ++i, bucket -= 8)
        {
            const reg32 head = guest.dword(bucket);
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
x86::reg32 frontend::rende::drawListAdd(App* app, CPU& cpu, reg32 first, reg32 last, reg32 base)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16 + 8);
    if (first == 0)
        return 0;
    if (guest.dword(kDrawnOnCard) == 0)
        return guest::direct<0x4c7480, depthBucketsAdd>(app, cpu, first, base);
    const reg32 joined = guest.frame();
    const reg32 left = guest::direct<0x4bbec0, rendd::joinLists>(app, cpu, joined, guest.dword(kPendingList),
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
x86::reg32 frontend::rende::blinkingQuads(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x98);
    constexpr reg32 kLights = 0x7dd850, kLightBytes = 0x50, kLightCount = 8;
    constexpr reg32 kLightCorners = 4, kLightAcross = 0x48, kLightOnUntil = 0x4c;
    constexpr reg32 kTooFar = 0x540378;     // a double
    constexpr reg32 kLightTexture = 0x7ddad0, kViewCamera = 0x38, kViewMatrix = 0x44, kDepth = 0x7dcfe0;
    constexpr reg32 kColour = 0xffffffff;
    constexpr reg32 kFirstV = 0x3ca3d70a, kOneV = 0x3f800000, kLastV = 0x3f7ae148;   // 0.02, 1, 0.98
    const reg32 corners = guest.frame();
    const reg32 list = corners + 0x80, quadAt = corners + 0x8c;

    guest::direct<0x4bbe70, rendd::emptyList>(app, cpu, list);
    guest::direct<0x4bbf20, rendd::openListStart>(app, cpu);
    for (reg32 i = 0; i < kLightCount; ++i)
    {
        const reg32 light = kLights + i * kLightBytes;
        if (guest.dword(light) == 0)
            continue;
        const reg32 now = guest.dword(kFrameTime), until = guest.dword(light + kLightOnUntil);
        if (sreg32(now) < sreg32(until) && ((until - now) & 0x10) != 0)
            continue;
        fe::direct<0x4bf4c0, vertices::objectVerticesOf>(app, cpu, 4, light + kLightCorners, view + kViewMatrix, view + kViewCamera,
                                                         corners);
        if (!jumpsBelowOrEqual(floatOf(guest, corners + kVertexDepth), doubleOf(guest, kTooFar)))
            continue;
        const reg32 out = reg32(guest.byte(corners + kVertexCode)) & guest.byte(corners + kVertexBytes + kVertexCode)
                          & guest.byte(corners + 2 * kVertexBytes + kVertexCode) & guest.byte(corners + 3 * kVertexBytes + kVertexCode);
        if (out != 0)
            continue;
        if (guest::direct<0x4bbf80, rendd::openListTake>(app, cpu, quadAt, 0xa0u) == 0)
            continue;
        const reg32 quad = guest.dword(quadAt);
        const reg32 across = guest.dword(light + kLightAcross);
        const reg32 places[4][2] = { { 0, kFirstV }, { across, kFirstV }, { across, kOneV }, { 0, kLastV } };
        for (reg32 corner = 0; corner < 4; ++corner)
        {
            const reg32 from = corners + corner * kVertexBytes, to = quad + kPolygonBytes + corner * kVertexBytes;
            copyDwords(guest, to, from, 8);
            guest.setDword(to + kVertexColour, kColour);
            guest.setDword(to + 0x18, places[corner][0]);
            guest.setDword(to + 0x1c, places[corner][1]);
        }
        guest.setWord(quad + kPolygonKind, 0);
        guest.setWord(quad + kPolygonLayer, 0xa);
        guest.setDword(quad + kPolygonTexture, kLightTexture);
        for (reg32 corner = 0; corner < 4; ++corner)
            guest.setDword(quad + kPolygonCorners + 4 * corner, quad + kPolygonBytes + corner * kVertexBytes);
    }
    guest::direct<0x4bbfc0, rendd::openListClose>(app, cpu, list);
    return guest::direct<0x4bc060, rendd::drawListAt>(app, cpu, view, list, guest.dword(kDepth));
}

namespace
{

/* ---------------------------------------------------------------------------
 * The particles
 * ------------------------------------------------------------------------- */

constexpr reg32 kParticleStates = 0x7ddfd0;    // by view and particle: its bytes
constexpr reg32 kParticleStatesBytes = 0x12c00;
constexpr reg32 kFades = 0x7f0bd0;             // a table of 200 a view
constexpr reg32 kFadesBytes = 0x6400;
constexpr reg32 kFadesPerView = 0x3200;
constexpr reg32 kFadeBytes = 0x40;
constexpr reg32 kFadeCount = 0x7f6fd0;         // by view: how many are alive
constexpr reg32 kParticleCameras = 0x7f6fd8;   // by view: the camera they were drawn from

}  // namespace

/* sub_4cb220: a view's fading particles a step on: alpha in proportion to the
 * time left, those gone cleared, the living counted. */
x86::reg32 frontend::rende::fadeParticles(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    constexpr reg32 kFadeLeft = 0, kFadeKind = 4, kFadeAlpha = 0x2b;
    constexpr reg32 kKindLife = 0x560bcc, kKindAlpha = 0x560bdf;
    constexpr reg32 kPerFadeTable = 200;
    guest.setDword(kFadeCount + view * 4, 0);
    reg32 left = 0;
    for (reg32 i = 0; i < kPerFadeTable; ++i)
    {
        const reg32 fade = kFades + view * kFadesPerView + i * kFadeBytes;
        left = i << 6;
        const reg32 time = guest.dword(fade + kFadeLeft);
        if (time == 0)
            continue;
        const reg32 kind = guest.dword(fade + kFadeKind) << 7;
        const sreg32 life = sreg32(guest.dword(kKindLife + kind));
        const reg32 whole = guest.byte(kKindAlpha + kind);
        guest.setDword(kFadeCount + view * 4, guest.dword(kFadeCount + view * 4) + 1);
        const sreg32 alpha = sreg32(whole * time) / life;
        guest.setByte(fade + kFadeAlpha, uint8_t(alpha));
        const uint8_t alphaNow = guest.byte(fade + kFadeAlpha);
        guest.setDword(fade + kFadeLeft, alphaNow != 0 ? guest.dword(fade + kFadeLeft) - 1 : 0);
        left = withHigh(reg32(alpha), alphaNow);
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

x86::reg32 frontend::rende::loadParticleTextures(App* app, CPU& cpu, reg32 file)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x100);
    constexpr reg32 kKindCount = 0xe;
    constexpr reg32 kKindHasTexture = 0x560b64, kKindHasLit = 0x560bc8;
    constexpr reg32 kTextures = 0x7ddd68, kLitTextures = 0x7ddb00, kTextureBytes = 0x2c;
    constexpr reg32 kFormat = 0x20;
    const reg32 name = guest.frame();
    reg32 left = 0;
    for (reg32 kind = 0; kind < kKindCount; ++kind)
    {
        guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
        if (guest.dword(kKindHasTexture + (kind << 7)) != 0)
        {
            const reg32 format = gameSettings(guest).night != 0 ? kParticleNames[0].address : kParticleNames[1].address;
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, format, guest::Words{kind});
            guest::direct<0x4d4080, loadShapeTexture>(app, cpu, file, name, kFormat, kTextures + kind * kTextureBytes, 0u);
        }
        left = kind << 7;
        if (guest.dword(kKindHasLit + left) != 0)
        {
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kParticleNames[2].address, guest::Words{kind});
            left = guest::direct<0x4d4080, loadShapeTexture>(app, cpu, file, name, kFormat, kLitTextures + kind * kTextureBytes, 0u);
        }
    }
    return left;
}


/* sub_4cc540: a view's camera changed, or its particles told to forget: each
 * particle's two state bytes for this view cleared.  What counts as the camera
 * is its kind, with [0x79c4a0] a camera of kinds 2 to 4 only its top byte. */
x86::reg32 frontend::rende::particlesCameraChanged(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kCameraLocked = 0x79c4a0, kForget = 0x552ff4;
    constexpr reg32 kStateBytes = 0x7de048, kParticleCount = 0x258;
    const reg32 mode = fe::direct<0x423b00, carstate::modeOfCamera>(app, cpu, view);
    reg32 camera = mode;
    if (sreg32(mode) <= 4 && sreg32(mode) >= 2 && guest.dword(kCameraLocked + view * 4) != 0)
        camera &= 0xff000000u;
    reg32 left = view * 4;
    if (camera != guest.dword(kParticleCameras + left) || guest.dword(kForget + left) != 0)
    {
        for (reg32 i = 0; i < kParticleCount; ++i)
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
x86::reg32 frontend::rende::particlesPass(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16 + 8);
    constexpr reg32 kParticles = 0x4cbc90, kViewIndex = 4, kViewMatrix = 0x44, kDepth = 0x7dd000;
    if (gameSettings(guest).effectsLevel == 2)
        return view;
    const reg32 list = guest.frame();
    guest::direct<0x4cc540, particlesCameraChanged>(app, cpu, guest.dword(view + kViewIndex));
    callAt(guest, kParticles, args({ view, view + kViewMatrix }), list);  // esi: read by the particles' pass  // edx the view's matrix, which it reads
    return fe::direct<0x4c7610, vertices::bucketsSortedInto>(guest.app(), guest.cpu(), guest.dword(list), guest.dword(kDepth));
}

namespace
{

/* What the race frame's passes leave: the list through esi, whose place they took. */
reg32 handBackList(Guest& guest, reg32 list, reg32 result)
{
    copyDwords(guest, result, list, 2);
    return result;
}

}  // namespace

/* sub_4cc600: the frame's pass of the drops, on the first pass only, unless the
 * view has none. */
x86::reg32 frontend::rende::dropsPass(App* app, CPU& cpu, reg32 view, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 8);
    constexpr reg32 kDrops = 0x4cb750;
    const reg32 list = guest.frame();
    if (sreg32(gameSettings(guest).effectsLevel) >= 1)
    {
        guest::direct<0x4bbe70, rendd::emptyList>(app, cpu, list);
    }
    else
    {
        guest::direct<0x4bbf20, rendd::openListStart>(app, cpu);
        if (guest.dword(view) == 0)
            callAt(guest, kDrops, args({ view }));
        guest::direct<0x4bbfc0, rendd::openListClose>(app, cpu, list);
    }
    return handBackList(guest, list, result);
}


/* sub_4dedf0: the frame's pass of the sparks, by the weather's kind of them. */
x86::reg32 frontend::rende::sparksPass(App* app, CPU& cpu, reg32 view, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 8);
    constexpr reg32 kSparkKind = 0x8ca22c;
    const reg32 list = guest.frame();
    guest::direct<0x4bbf20, rendd::openListStart>(app, cpu);
    if (guest.dword(view) == 0 && guest.dword(kSparkKind) != 0 && gameSettings(guest).effectsLevel != 2)
    {
        fe::direct<0x4dbda0, frontend::simd::viewProjection>(app, cpu, view);
        const reg32 kind = guest.dword(kSparkKind);
        if (kind == 1 || kind == 2)
            fe::direct<0x4ddb40, render::sparksDrawn>(app, cpu, view);
        fe::direct<0x4dbde0, frontend::simc::setView>(app, cpu, view);
    }
    guest::direct<0x4bbfc0, rendd::openListClose>(app, cpu, list);
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
x86::reg32 frontend::rende::smokeEmit(App* app, CPU& cpu, reg32 view, reg32 pointX, reg32 pointY, reg32 pointZ)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x34);
    constexpr reg32 kHudOff = 0x7d36a0;
    constexpr reg32 kPuffsPerView = 0x190, kPuffBytes = 0x14;
    constexpr reg32 kPuffX = 0x8c08c0, kPuffY = 0x8c08c4, kPuffLife = 0x8c08c8, kPuffSize = 0x8c08cc, kPuffTurn = 0x8c08d0;
    constexpr reg32 kPuffCount = 0x8ca278, kPuffLifeMost = 0x8ca27c, kPuffSizeMost = 0x8ca280;
    constexpr reg32 kHalf = 0x5497d4;     // a double
    constexpr reg32 kViewIndex = 4, kViewCamera = 0x38, kViewMatrix = 0x44;
    if (guest.dword(kHudOff) != 0)
        return 0;
    const reg32 viewIndex = guest.dword(view + kViewIndex);
    const reg32 table = viewIndex * kPuffsPerView;
    sreg32 place = 0;
    for (;; ++place)
    {
        if (place >= sreg32(guest.dword(kPuffCount)))
            return 0;
        if (guest.dword(table + reg32(place) * kPuffBytes + kPuffLife) == 0)
            break;
    }
    const reg32 puff = table + reg32(place) * kPuffBytes;

    // The point by the view's matrix, its row at a time, then the camera's place.
    const reg32 matrix = view + kViewMatrix;
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
    const sreg32 quarter = sreg32(guest.dword(kPuffLifeMost)) >> 2;
    const reg32 spread = quarter > 1 ? reg32(quarter) : 1;
    const reg32 lessLife = callAt(guest, kRandom) % spread;
    guest.setFloat(puff + kPuffX, float(fp::mul(x, scale)));
    guest.setFloat(puff + kPuffY, float(fp::mul(scale, y)));
    guest.setDword(puff + kPuffLife, guest.dword(kPuffLifeMost) - lessLife);

    const double chance = double(*fe::direct<0x4eb230, ea::randomFractionOf>(app, cpu));
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

    guest.setFloat(puff + kPuffTurn, float(*fe::direct<0x4eb230, ea::randomFractionOf>(app, cpu)));
    return 1;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The textures
 * ------------------------------------------------------------------------- */

// A shape: its width and height, words, and its pixels.
constexpr reg32 kShapeWidth = 4, kShapeHeight = 6, kShapePixels = 0x10;
constexpr uint8_t kShapeType16 = 0x6d;

// A texture's record.
constexpr reg32 kTextureWidth = 0, kTextureHeight = 2, kTextureHandle = 4, kTextureFlags = 0x28, kTextureRoundRobin = 0x29;
constexpr uint8_t kTextureAdditive = 2, kTextureOwnPalette = 4;
constexpr reg32 kRoundRobinNext = 0x563a8c;    // the next texture's place, a byte

}  // namespace

/* sub_4d3380: an 8-bit shape widened to 16 bits in place of the buffer `to`, by
 * its palette; 0xff transparent; the shape's kind made 0x6d. */
x86::reg32 frontend::rende::expandPalettedShape(App* app, CPU& cpu, reg32 shape, reg32 to, reg32 palette)
{
    Guest guest(app, cpu);
    constexpr uint8_t kTransparent = 0xff;
    const sreg32 width = std::int16_t(guest.word(shape + kShapeWidth));
    const sreg32 height = std::int16_t(guest.word(shape + kShapeHeight));
    const sreg32 pixels = sreg32(reg32(width) * reg32(height));
    reg32 from = shape + kShapePixels;
    reg32 out = to;
    for (sreg32 i = 0; i < pixels; ++i, ++from, out += 2)
    {
        const uint8_t index = guest.byte(from);
        if (index == kTransparent)
        {
            guest.setWord(out, 0);
            continue;
        }
        const reg32 colour = guest.dword(palette + reg32(index) * 4);
        const reg32 red = ((colour & 0xf00000) >> 12) | 0x8000;
        const reg32 green = (colour & 0xf000) >> 8;
        const reg32 blue = (colour & 0xf0) >> 4;
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

x86::reg32 frontend::rende::loadShapeTexture(App* app, CPU& cpu, reg32 file, reg32 named, reg32 format, reg32 record, reg32 second)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12 + 4);
    const reg32 ownPalette = guest.frame();
    const reg32 shape = file != 0 ? fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, named) : named;
    guest.setWord(record + kTextureWidth, guest.word(shape + kShapeWidth));
    guest.setWord(record + kTextureHeight, guest.word(shape + kShapeHeight));
    guest.setByte(record + kTextureFlags, 0);
    guest.setDword(record + kTextureHandle, *fe::direct<0x4d3f50, render::squareTextureMade>(app, cpu, shape, ownPalette, record, format, second));

    reg32 left = fe::direct<0x4f24e0, frontend::eab::shapeFindHotspot>(app, cpu, shape);
    if (left != 0)
    {
        left = fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.cpu(), left, kAdditive.address);
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
x86::reg32 frontend::rende::copyCabinBar(App* app, CPU& cpu, reg32 bar, reg32 shape, reg32 buffer)
{
    Guest guest(app, cpu);
    constexpr reg32 kBars = 0x792094;    // the record's fields from here
    constexpr reg32 kStride = 0, kToX = 4, kToY = 8, kFromX = 0xc, kFromY = 0x10, kWidth = 0x14, kHeight = 0x18, kOnItsSide = 0x1c;
    const reg32 index = bar * 9;
    const reg32 fields = kBars + index * 4;
    reg32 to = buffer + (guest.dword(fields + kToY) * guest.dword(fields + kStride) + guest.dword(fields + kToX)) * 2;
    const sreg32 shapeWidth = std::int16_t(guest.word(shape + kShapeWidth));
    reg32 from = shape + kShapePixels + (guest.dword(fields + kFromY) * reg32(shapeWidth) + guest.dword(fields + kFromX)) * 2;
    sreg32 row = 0;
    if (guest.dword(fields + kOnItsSide) != 0)
    {
        for (; row < sreg32(guest.dword(fields + kHeight)); ++row)
        {
            for (sreg32 column = 0; column < sreg32(guest.dword(fields + kWidth)); ++column)
            {
                const sreg32 across = std::int16_t(guest.word(shape + kShapeWidth));
                const reg32 pixel = from + (reg32(across) * reg32(column) + reg32(row)) * 2;
                guest.setWord(to + reg32(column) * 2, guest.word(pixel));
            }
            to += guest.dword(fields + kStride) * 2;
        }
    }
    else
    {
        for (; row < sreg32(guest.dword(fields + kHeight)); ++row)
        {
            for (sreg32 column = 0; column < sreg32(guest.dword(fields + kWidth)); ++column)
                guest.setWord(to + reg32(column) * 2, guest.word(from + reg32(column) * 2));
            to += guest.dword(fields + kStride) * 2;
            from += reg32(sreg32(std::int16_t(guest.word(shape + kShapeWidth)))) * 2;
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

x86::reg32 frontend::rende::loadEffectTextures(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x108);
    constexpr reg32 kSfxFolder = 0x7a2934;
    constexpr reg32 kLines = 0x8b4420, kLineCount = 10, kLineList = 0x8b41a0, kTextureBytes = 0x2c;
    constexpr reg32 kShadow = 0x8b45d8, kShadowPointer = 0x8b4898;
    constexpr reg32 kSkidRecords[] = { 0x8b4604, 0x8b4630, 0x8b465c, 0x8b4688 };
    constexpr reg32 kFont = 0x67901c;
    constexpr reg32 kGlows[] = { 0x8b47bc, 0x8b47e8, 0x8b4814, 0x8b4840, 0x8b486c };
    constexpr reg32 kSparkTexture = 0x7ddad0;
    constexpr reg32 kMouse = 0x8b41d8;
    constexpr reg32 kDials[] = { 0x8b4338, 0x8b4364, 0x8b4390, 0x8b43bc };
    constexpr reg32 kGlowSecond = 7, kFormat16 = 0x20, kFormatWide = 0x40, kFormat256 = 0x80;
    constexpr reg32 kBlendState = 0xd;
    constexpr reg32 kProgress36 = 0x42100000, kProgress37 = 0x42140000, kProgress38 = 0x42180000, kProgress39 = 0x421c0000;
    constexpr uint8_t kCardWithAlpha = 2, kKeepFlag = 8;
    const reg32 name = guest.frame();
    const auto texture = [&](reg32 file, reg32 shape, reg32 format, reg32 record, reg32 second) {
        return guest::direct<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, file, shape, format, record, second);
    };
    // The bar's per cent, a float, in the word the code pushes, the bar handed its address.
    const auto progress = [&](reg32 perCent) {
        const reg32 pushed = cpu.esp - 4;
        guest.setDword(pushed, perCent);
        fe::direct<0x495230, graphics::loadingProgress>(app, cpu, pushed);
    };

    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, 0x549458, guest::Words{kSfxFolder});
    // Its flags in edx, as the call before leaves it.
    const reg32 file = *fe::direct<0x4e0ec0, ea::loadWhole>(app, cpu, name, 0);
    progress(kProgress36);
    for (reg32 line = 0; line < kLineCount; ++line)
    {
        guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, 0x549464, guest::Words{line});
        const reg32 record = kLines + line * kTextureBytes;
        texture(file, name, kFormat16, record, 0);
        guest.setByte(record + kTextureFlags, uint8_t(guest.byte(record + kTextureFlags) | kTextureOwnPalette));
        guest.setDword(kLineList + line * 4, record);
    }
    texture(file, 0x54946c, kFormat16, kShadow, 0);
    guest.setDword(kShadowPointer, kShadow);
    for (reg32 skid = 0; skid < 4; ++skid)
    {
        texture(file, 0x549474 + skid * 8, kFormat16, kSkidRecords[skid], 0);
        guest.setDword(kSkidTextures + skid * 4, kSkidRecords[skid]);
    }
    progress(kProgress37);
    guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    texture(file, 0x549494, kFormat16, kFont, 0);
    guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    texture(file, 0x54949c, kFormatWide, kGlows[0], kGlowSecond);
    texture(file, 0x5494a4, kFormat16, kGlows[1], kGlowSecond);
    fe::direct<0x431900, graphics::sendState>(app, cpu, kBlendState, 0u);
    texture(file, 0x5494ac, kFormat256, kGlows[2], kGlowSecond);
    fe::direct<0x431900, graphics::sendState>(app, cpu, kBlendState, 1u);
    texture(file, 0x5494b4, kFormat16, kGlows[3], kGlowSecond);
    texture(file, 0x5494bc, kFormatWide, kGlows[4], kGlowSecond);
    if ((guest.byte(kDisplayFlags) & kCardWithAlpha) != 0)
    {
        const reg32 shape = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, 0x5494c4u);
        fe::direct<0x4d41a0, render::paletteTextured>(app, cpu, shape, kSparkTexture);
    }
    else
        texture(file, 0x5494cc, kFormat16, kSparkTexture, 0);
    progress(kProgress38);
    guest::direct<0x4cb590, loadParticleTextures>(app, cpu, file);
    progress(kProgress39);
    texture(file, 0x5494d4, kFormat16, kMouse, 0);
    reg32 left = 0;
    for (reg32 dial = 0; dial < 4; ++dial)
        left = texture(file, 0x5494dc + dial * 8, kFormat16, kDials[dial], 0);
    if (file != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, file);
    for (const reg32 glow : { kGlows[0], kGlows[4], kGlows[3] })
        guest.setByte(glow + kTextureFlags, uint8_t(guest.byte(glow + kTextureFlags) | kKeepFlag));
    return left;
}


/* sub_4d56f0: sfx.fsh loaded, the round robin from its start. */
x86::reg32 frontend::rende::loadEffects(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    guest.setByte(kRoundRobinNext, 0);
    guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
    guest::direct<0x4d5370, loadEffectTextures>(app, cpu);
    return guest::direct<0x4a33d0, loada::stopCheck>(app, cpu);
}


x86::reg32 frontend::rendDrawQuadClipped(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c1900", 0, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 a = args.eax, b = args.edx, c = args.ebx, d = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::drawQuadClipped(app, cpu, a, b, c, d); });
}

x86::reg32 frontend::rendDrawQuadSplit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c19e0", 0, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 a = args.eax, b = args.edx, c = args.ebx, d = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::drawQuadSplit(app, cpu, a, b, c, d); });
}

x86::reg32 frontend::rendSkidAddSegment(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c6cb0", 4, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 from = args.eax, to = args.edx, alphaFrom = args.ebx, alphaTo = args.ecx;
    const reg32 mode = stackArgument(app, cpu, 0);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return rende::skidAddSegment(app, cpu, from, to, alphaFrom, alphaTo, mode);
    });
}

x86::reg32 frontend::rendSkidMarksDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c6f00", 0, 0, nullptr, 0 };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::skidMarksDraw(app, cpu, view); });
}

x86::reg32 frontend::rendPolygonDepths(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7180", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 list = args.eax, least = args.edx, scale = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::polygonDepths(app, cpu, list, least, scale); });
}

x86::reg32 frontend::rendPolygonsIntoBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7260", 8, 0, nullptr, 0 };
    const reg32 list = args.eax;
    const reg32 least = stackArgument(app, cpu, 0), scale = stackArgument(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::polygonsIntoBuckets(app, cpu, list, least, scale); });
}

x86::reg32 frontend::rendPolygonsSorted(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7380", 8, 0, nullptr, 0, kKeepsAllButEsi };
    const reg32 result = args.esi;
    const reg32 list = stackArgument(app, cpu, 0), last = stackArgument(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::polygonsSorted(app, cpu, result, list, last); });
}

/* sub_4c73e0: the depth buckets cleared, when the polygons are sorted here. */
x86::reg32 frontend::rende::clearDepthBuckets(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    if (guest.dword(kDrawnOnCard) == 0)
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kDepthHeads, kDepthBytes);
    return 0;  // nothing: the thunk leaves eax as it came
}

x86::reg32 frontend::rendClearDepthBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c73e0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { rende::clearDepthBuckets(app, cpu); return args.eax; });
}

x86::reg32 frontend::rendDepthBucketAppend(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7400", 12, 0, nullptr, 0 };
    const reg32 view = args.eax;
    const reg32 first = stackArgument(app, cpu, 0), last = stackArgument(app, cpu, 1), depth = stackArgument(app, cpu, 2);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::depthBucketAppend(app, cpu, view, first, last, depth); });
}

x86::reg32 frontend::rendDepthBucketsAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7480", 4, 0, nullptr, 0 };
    const reg32 list = args.eax;
    const reg32 base = stackArgument(app, cpu, 0);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::depthBucketsAdd(app, cpu, list, base); });
}

x86::reg32 frontend::rendDepthBucketsGather(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7580", 0, 0, nullptr, 0, kKeepsAllButEsi };
    const reg32 result = args.esi;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::depthBucketsGather(app, cpu, result); });
}

/* sub_4c75f0: the far buckets cleared, on the card. */
x86::reg32 frontend::rende::clearFarBuckets(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    if (guest.dword(kDrawnOnCard) != 0)
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kFarHeads, kFarBytes);
    return 0;  // nothing: the thunk leaves eax as it came
}

x86::reg32 frontend::rendClearFarBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c75f0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { rende::clearFarBuckets(app, cpu); return args.eax; });
}

x86::reg32 frontend::rendFarBucketsGather(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c76e0", 0, 0, nullptr, 0, kKeepsAllButEsi };
    const reg32 result = args.esi;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::farBucketsGather(app, cpu, result); });
}

/* sub_4c7750: on the card, the near buckets cleared, and the depth ones when `depthToo`. */
x86::reg32 frontend::rende::clearBuckets(App* app, CPU& cpu, reg32 depthToo)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    if (guest.dword(kDrawnOnCard) != 0)
    {
        if (depthToo != 0)
            fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kDepthHeads, kDepthBytes);
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kNearBuckets, kNearBytes);
    }
    return depthToo;
}

x86::reg32 frontend::rendClearBuckets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7750", 0, 0, nullptr, 0, kKeepsEdx };
    const reg32 depthToo = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::clearBuckets(app, cpu, depthToo); });
}

x86::reg32 frontend::rendDrawListAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7940", 12, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 first = stackArgument(app, cpu, 0), last = stackArgument(app, cpu, 1), base = stackArgument(app, cpu, 2);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::drawListAdd(app, cpu, first, last, base); });
}

x86::reg32 frontend::rendBlinkingQuads(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c9f70", 0, 0, nullptr, 0 };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::blinkingQuads(app, cpu, view); });
}

x86::reg32 frontend::rendFadeParticles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cb220", 0, 0, nullptr, 0 };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::fadeParticles(app, cpu, view); });
}

x86::reg32 frontend::rendLoadParticleTextures(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cb590", 0, 0, kParticleNames, std::size(kParticleNames) };
    const reg32 file = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::loadParticleTextures(app, cpu, file); });
}

/* sub_4cb6f0: the particles' states and fades cleared. */
x86::reg32 frontend::rende::clearParticles(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kParticleStates, kParticleStatesBytes);
    const reg32 left = fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kFades, kFadesBytes);
    guest.setDword(kFadeCount, 0);
    guest.setDword(kFadeCount + 4, 0);
    return left;
}

x86::reg32 frontend::rendClearParticles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cb6f0", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::clearParticles(app, cpu); });
}

/* sub_4cb730: the particles' states cleared. */
x86::reg32 frontend::rende::clearParticleStates(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kParticleStates, kParticleStatesBytes);
}

x86::reg32 frontend::rendClearParticleStates(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cb730", 0, 0, nullptr, 0, kKeepsEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::clearParticleStates(app, cpu); });
}

x86::reg32 frontend::rendParticlesCameraChanged(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cc540", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::particlesCameraChanged(app, cpu, view); });
}

x86::reg32 frontend::rendParticlesPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cc5c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::particlesPass(app, cpu, view); });
}

x86::reg32 frontend::rendDropsPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cc600", 0, 0, nullptr, 0, kKeepsEdxEdi };
    const reg32 view = args.eax, result = args.esi;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::dropsPass(app, cpu, view, result); });
}

/* sub_4d18b0: how many bits a texture's format has. */
x86::reg32 frontend::rende::formatBits(App* app, CPU& cpu, reg32 format)
{
    static_cast<void>(app);
    static_cast<void>(cpu);
    return format == 2 ? 8 : format == 6 ? 0x20 : 0x10;
}

x86::reg32 frontend::rendFormatBits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d18b0", 0, 0, nullptr, 0, kKeepsEdx };
    const reg32 format = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::formatBits(app, cpu, format); });
}

x86::reg32 frontend::rendExpandPalettedShape(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d3380", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 shape = args.eax, to = args.edx, palette = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::expandPalettedShape(app, cpu, shape, to, palette); });
}

x86::reg32 frontend::rendLoadShapeTexture(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d4080", 4, 0, &kAdditive, 1, kKeepsEsiEdi };
    const reg32 file = args.eax, named = args.edx, format = args.ebx, record = args.ecx;
    const reg32 second = stackArgument(app, cpu, 0);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return rende::loadShapeTexture(app, cpu, file, named, format, record, second);
    });
}

/* sub_4d4120: the greatest power of two up to 256 no more than the lesser of the two, 1 at least. */
x86::reg32 frontend::rende::fitPowerOfTwo(App* app, CPU& cpu, reg32 a, reg32 b)
{
    static_cast<void>(app);
    static_cast<void>(cpu);
    const sreg32 least = sreg32(a) > sreg32(b) ? sreg32(b) : sreg32(a);
    for (sreg32 power = 0x100; power >= 2; power >>= 1)
        if (least >= power)
            return reg32(power);
    return 1;
}

x86::reg32 frontend::rendFitPowerOfTwo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d4120", 0, 0, nullptr, 0 };
    const reg32 a = args.eax, b = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::fitPowerOfTwo(app, cpu, a, b); });
}

x86::reg32 frontend::rendCopyCabinBar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d4440", 0, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 bar = args.eax, shape = args.edx, buffer = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::copyCabinBar(app, cpu, bar, shape, buffer); });
}

x86::reg32 frontend::rendLoadEffectTextures(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d5370", 0, 0, kSfxStrings, std::size(kSfxStrings) };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::loadEffectTextures(app, cpu); });
}

/* sub_4d56b0: an empty function; eax as it came (the thunk's). */
x86::reg32 frontend::rende::nothing(App* app, CPU& cpu)
{
    static_cast<void>(app);
    static_cast<void>(cpu);
    return 0;
}

x86::reg32 frontend::rendNothing(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d56b0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { rende::nothing(app, cpu); return args.eax; });
}

x86::reg32 frontend::rendLoadEffects(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d56f0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::loadEffects(app, cpu); });
}

x86::reg32 frontend::rendSkidSubdivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4db750", 8, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 depth = args.eax, from = args.edx, to = args.ebx, alphaFrom = args.ecx;
    const reg32 alphaTo = stackArgument(app, cpu, 0), mode = stackArgument(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return rende::skidSubdivide(app, cpu, depth, from, to, alphaFrom, alphaTo, mode);
    });
}

x86::reg32 frontend::rendSkidStrip(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4db8b0", 4, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 centre = args.eax, from = args.edx, to = args.ebx, axle = args.ecx;
    const reg32 out = stackArgument(app, cpu, 0);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::skidStrip(app, cpu, centre, from, to, axle, out); });
}

x86::reg32 frontend::rendSkidWheel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4db940", 8, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 car = args.eax, wheel = args.edx, position = args.ebx, axle = args.ecx;
    const reg32 released = stackArgument(app, cpu, 0), strength = stackArgument(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return rende::skidWheel(app, cpu, car, wheel, position, axle, released, strength);
    });
}

/* sub_4dbcc0: the camera's horizontal angle by View Angle: 75 narrow, else 90 or the port's 105. */
x86::reg32 frontend::rende::defaultFieldOfView(App* app, CPU& cpu)
{
    static_cast<void>(cpu);
    return gameSettings(app).viewAngle == 1 ? 75 : viewAngleFor(app, 90);
}

x86::reg32 frontend::rendDefaultFieldOfView(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dbcc0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::defaultFieldOfView(app, cpu); });
}

x86::reg32 frontend::rendSmokeEmit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd930", 12, 0, nullptr, 0 };
    const reg32 view = args.eax;
    const reg32 pointX = stackArgument(app, cpu, 0), pointY = stackArgument(app, cpu, 1), pointZ = stackArgument(app, cpu, 2);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::smokeEmit(app, cpu, view, pointX, pointY, pointZ); });
}

x86::reg32 frontend::rendSparksPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dedf0", 0, 0, nullptr, 0, kKeepsEdxEdi };
    const reg32 view = args.eax, result = args.esi;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rende::sparksPass(app, cpu, view, result); });
}

}
