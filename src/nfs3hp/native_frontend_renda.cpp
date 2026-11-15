#include "native_frontend_renda.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_graphics.h"
#include "native_vertices.h"
#include <nfs3hp.h>
#include <cstring>

/* The race's sky and what stands with it, decompiled:
 *
 *   the fog's states  0xe and 0xf, the density and the colour, only on a driver
 *                     with a depth bias (voodoo2a's is 4): sub_470a60 to sub_470b90;
 *   the sky's rings   sub_479680, sub_4799d0 and sub_479b90, each a band of eight
 *                     quads round the camera, nine points by nine, in the gradient
 *                     tables' colours or a cloud texture from 0x6ff95c, the last
 *                     moved by the wind's table at 0x7001e8;
 *   the sun           sub_479ed0, its quad on the screen;
 *   the sky's colour  sub_47a4c0, the flat quad of a split screen in the tunnel,
 *                     and the fog's colour in state 3;
 *   the dome's grid   sub_47ae10, thirteen points by thirteen, made as the track
 *                     loads and smoothed in twenty passes;
 *   the weather       sub_47b5c0, each tick's flash, its cell on the grid and the
 *                     clouds' drift, and sub_47b3a0, the flash on the grid;
 *   the rest          the sky's polygon list's two ends, the quad's record, the
 *                     sparks on the screen, a string's width in the race's font.
 *
 * All of it stays in guest memory, what it calls is called by address, and the
 * x87's arithmetic goes through fp:: in the original's order.
 *
 * Plan step 5: each function a typed core in frontend::renda (native_frontend_renda.h), in a
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
using fe::X87;
using x86::Float;
using x86::reg32;
using x86::sreg32;

// The game's functions, by address.
constexpr reg32 kViewport = 0x4beb80;          // eax, edx, ebx, ecx where its x, y, width and height go
constexpr reg32 kAddVectors = 0x4dffb0;        // eax how many, edx and ebx them, ecx the sums
constexpr reg32 kSubtractVectors = 0x4e0000;   // likewise, edx less ebx
constexpr reg32 kRandom = 0x4e0d88;
constexpr reg32 kObjectVertices = 0x4bf4c0;    // eax how many, edx them, ebx the matrix, ecx the shift; the stack where
constexpr reg32 kThunder = 0x414e10;           // eax and edx its x and z, 16.16
constexpr reg32 kSetRenderState = 0x431900;    // eax the state, edx its value

// The render states.
constexpr reg32 kStateRaceFogColour = 3;
constexpr reg32 kStateFogDensity = 0xe;
constexpr reg32 kStateFogColour = 0xf;

// Their settings.
constexpr reg32 kDepthBias = 0x7a3a64;         // the driver's; voodoo2a's is 4
constexpr reg32 kFogAmountKept = 0x678b40;
constexpr reg32 kFogColourKept = 0x678b44;
constexpr reg32 kDriverOnCard = 0x7a3a60;
constexpr reg32 kDriverFlags = 0x7a3a58;       // word; voodoo2a's is 0x10
constexpr std::uint16_t kDriversAlphaFromNoise = 0x1e0;

// The race's clock and view.
constexpr reg32 kTicks = 0x7d3684;
constexpr reg32 kViewMatrix = 0x7a3d58;
constexpr reg32 kViewShift = 0x7a3d4c;
constexpr reg32 kScreenScaleX = 0x56009c, kScreenScaleY = 0x5600a0;
constexpr reg32 kScreenCentreX = 0x5600a4, kScreenCentreY = 0x5600a8;

// The screen's bounds, as a float's bits are compared.
constexpr reg32 kClipDepth = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504,
                kClipRight = 0x7d350c;

// A view: its height over the track, and its matrices.
constexpr reg32 kViewHeight = 0xc;
constexpr reg32 kViewSkyMatrix = 0x14;
constexpr reg32 kViewCameraMatrix = 0x44;

// A polygon's record (sub_4bbf80), with its four vertices after it.
constexpr reg32 kPolygonBytes = 0xa0;
constexpr reg32 kPolygonA = 4, kPolygonFlags = 6;
constexpr reg32 kPolygonVertices = 8;          // four pointers
constexpr reg32 kPolygonTexture = 0x18;
constexpr reg32 kPolygonVertex0 = 0x20;
constexpr reg32 kVertexBytes = 0x20;
constexpr reg32 kVertexDwords = kVertexBytes / 4;
constexpr reg32 kVertexW = 0xc, kVertexColour = 0x10, kVertexClip = 0x14, kVertexU = 0x18, kVertexV = 0x1c;
constexpr reg32 kPolygonNoDepth = 4;           // the quad's flag not set
constexpr reg32 kOpaque = 0xff000000;

constexpr reg32 kFloatOne = 0x3f800000;
constexpr reg32 kFloatTiny = 0x37800080;       // what stands in for a depth of zero
constexpr reg32 kNoSign = 0x7fffffff;
constexpr reg32 kSign = 0x80000000;

constexpr std::uint16_t kCompareBelow = 0x100;  // fcom's c0, what sahf makes cf

Float fld(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

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

/* What the jumps after fcom, fnstsw and sahf read. */
bool jumpAbove(std::uint16_t status) { return status == 0; }
bool jumpBelowOrEqual(std::uint16_t status) { return status != 0; }
bool jumpAboveOrEqual(std::uint16_t status) { return (status & kCompareBelow) == 0; }

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A generated function's stack argument, by its place, as its adapter reads it. */
reg32 stackWord(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

struct Vector
{
    float x, y, z;
};

/* The vector at `vector` by the matrix at `matrix`, as all these functions inline it:
 * each row's three products added in the matrix's order, a float's precision each. */
Vector rotated(const Guest& guest, reg32 vector, reg32 matrix)
{
    const Float x = fld(guest, vector), y = fld(guest, vector + 4), z = fld(guest, vector + 8);
    const Float a0 = fp::mul(x, fld(guest, matrix));
    const Float a1 = fp::mul(x, fld(guest, matrix + 4));
    const Float a2 = fp::mul(x, fld(guest, matrix + 8));
    const Float b0 = fp::mul(y, fld(guest, matrix + 0xc));
    const Float b1 = fp::mul(y, fld(guest, matrix + 0x10));
    const Float b2 = fp::mul(y, fld(guest, matrix + 0x14));
    const Float sum0 = fp::add(a0, b0);
    const Float sum1 = fp::add(a1, b1);
    const Float sum2 = fp::add(a2, b2);
    const Float c0 = fp::mul(z, fld(guest, matrix + 0x18));
    const Float c1 = fp::mul(z, fld(guest, matrix + 0x1c));
    const Float c2 = fp::mul(z, fld(guest, matrix + 0x20));
    return { float(fp::add(sum0, c0)), float(fp::add(sum1, c1)), float(fp::add(sum2, c2)) };
}

void putVector(Guest& guest, reg32 at, const Vector& v)
{
    guest.setFloat(at, v.x);
    guest.setFloat(at + 4, v.y);
    guest.setFloat(at + 8, v.z);
}

/* A vertex's clip code, as the game inlines it (0x4bf426): past the far plane 0x10,
 * else 8 above or 4 below, 1 left or 2 right, its float's bits compared as integers.
 * `bound` is the last bound read, what ecx keeps. */
struct Clip
{
    reg32 code;
    reg32 bound;
};

Clip clipOf(const Guest& guest, reg32 vertex)
{
    const reg32 w = guest.dword(vertex + kVertexW);
    const reg32 depth = guest.dword(kClipDepth);
    if ((w & kSign) != 0 || sreg32(w) >= sreg32(depth))
        return { 0x10, depth };
    reg32 code = 0;
    const reg32 y = guest.dword(vertex + 4);
    if ((y & kSign) != 0 || sreg32(y) < sreg32(guest.dword(kClipTop)))
        code |= 8;
    else if (sreg32(y) > sreg32(guest.dword(kClipBottom)))
        code |= 4;
    const reg32 x = guest.dword(vertex);
    const reg32 left = guest.dword(kClipLeft);
    if ((x & kSign) != 0 || sreg32(x) < sreg32(left))
        code |= 1;
    else if (sreg32(x) > sreg32(guest.dword(kClipRight)))
        code |= 2;
    return { code, left };
}

/* A quad for sub_478cb0: its four vertices by value on the stack, past what
 * Guest::call's arguments hold, its four colours and its flag. */
struct Quad
{
    reg32 vertices[4];
    reg32 colours[4];
    reg32 flag;
};

constexpr reg32 kQuadStackBytes = 4 * kVertexBytes + 4 * 4 + 4;

reg32 makeQuad(Guest& guest, x86::CPU& cpu, reg32 polygon, const Quad& quad)
{
    const reg32 esp = cpu.esp;
    cpu.esp -= kQuadStackBytes;
    for (reg32 i = 0; i < 4; ++i)
        copyDwords(guest, cpu.esp + i * kVertexBytes, quad.vertices[i], kVertexDwords);
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(cpu.esp + 4 * kVertexBytes + 4 * i, quad.colours[i]);
    guest.setDword(cpu.esp + 4 * kVertexBytes + 4 * 4, quad.flag);
    // The vertices, the colours and the flag on the stack below, where the record's core reads them.
    const reg32 result = guest::direct<0x478cb0, frontend::renda::quadRecord>(guest.app(), cpu, polygon);
    cpu.esp = esp;
    return result;
}

/* What is left of a hundred, kept to 1..100; the density's state takes five times it. */
reg32 fogShare(reg32 amount)
{
    const sreg32 share = sreg32(reg32(100) - amount);
    if (share < 1)
        return 1;
    return share > 100 ? 100 : reg32(share);
}

/* One vector added to another, taken from it, or scaled (the game's 0x4dffb0, 0x4e0000 and 0x4e0050,
 * vertices' typed cores). */
void addOne(Guest& guest, x86::CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), cpu, 1, a, b, out);
}

void subtractOne(Guest& guest, x86::CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(guest.app(), cpu, 1, a, b, out);
}

void scaleOne(Guest& guest, x86::CPU& cpu, reg32 in, reg32 factorBits, reg32 out)
{
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(guest.app(), cpu, 1, in, floatOf(factorBits), out);
}

/* The record the sky's list starts or ends with: 16 bytes, kind 5, none next, 4, and
 * what it hands the driver. */
constexpr reg32 kListMarkBytes = 0x10;

void fillListMark(Guest& guest, reg32 record, reg32 last)
{
    constexpr reg32 kMarkKind = 4, kMarkFour = 8, kMarkLast = 0xc;
    guest.setWord(record + kMarkKind, 5);
    guest.setDword(record, 0);
    guest.setDword(record + kMarkFour, 4);
    guest.setDword(record + kMarkLast, last);
}

constexpr reg32 kPointBytes = 0xc;
constexpr reg32 kRingPointCount = 9;
constexpr reg32 kRingSegments = 8;

constexpr reg32 kSkyWind = 0x55d2cc, kSkyCentre = 0x55d2b4, kSkyUp = 0x55d2c0, kSkyRadius = 0x55d1c8, kSkyTurn = 0x55d1cc;
constexpr reg32 kSkyBottom = 0x55d164, kSkyMiddle = 0x55d168, kSkySpread = 0x55d16c, kSkyTop = 0x55d170, kSkyTopSpread = 0x55d174;
constexpr reg32 kSkyGradient = 0x55d178, kSkyColoursLow = 0x55d17c, kSkyColoursHigh = 0x55d180;
constexpr reg32 kSkyTwoSided = 0x55d15c;

/* The segment of a ring, the turn's sixteenths on by `i`. */
reg32 ringSegment(reg32 start, reg32 i)
{
    const reg32 segment = start + i;
    return sreg32(segment) >= 0x10 ? segment - 0x10 : segment;
}

constexpr reg32 kGrid = 13;
constexpr reg32 kGridRowBytes = 0x9c;
constexpr reg32 kCloudPoints = 0x6ff9fc;       // 13 by 13 of x, y, z
constexpr reg32 kCloudCorners = 0x6ff170;      // 13 by 13 of u, v, colour
constexpr reg32 kCloudBrightest = 0x55d1f8, kCloudDepth = 0x55d1fc, kCloudNoise = 0x55d200;
constexpr reg32 kSkyCurveScale = 0x55d24c, kSkyCurveShift = 0x55d248;

constexpr reg32 kFlashTick = 0x55d154, kFlashOn = 0x55d250, kDriftX = 0x55d214, kDriftZ = 0x55d218;
constexpr reg32 kFlashBrightness = 0x55d25c, kFlashCellX = 0x55d254, kFlashCellZ = 0x55d258;

}

namespace frontend
{

/* ---------------------------------------------------------------------------
 * The fog's states
 * ------------------------------------------------------------------------- */

/* sub_470a60: the fog's amount, as sub_470ad0 was last given it. */
x86::reg32 renda::fogAmount(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(kFogAmountKept);
}

void rendFogAmount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470a60", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogAmount(app, cpu); });
}

/* sub_470aa0: state 0xf, eax, kept, on a driver with a depth bias. */
x86::reg32 renda::fogColour(App* app, CPU& cpu, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (guest.dword(kDepthBias) == 0)
        return colour;
    guest.setDword(kFogColourKept, colour);
    // sendState leaves the state's answer in eax.
    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateFogColour, colour);
    return guest.eax();
}

void rendFogColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470aa0", 0, 0, nullptr, 0 };
    const reg32 colour = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogColour(app, cpu, colour); });
}

/* sub_470ad0: the fog's amount, eax, kept, on a driver with a depth bias: state 0xe,
 * five times what is left of a hundred, none for none. */
x86::reg32 renda::fogDensity(App* app, CPU& cpu, reg32 amount)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (guest.dword(kDepthBias) == 0)
        return amount;
    guest.setDword(kFogAmountKept, amount);
    const reg32 share = amount != 0 ? fogShare(amount) : 0;
    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateFogDensity, share * 5);
    return guest.eax();
}

void rendFogDensity(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470ad0", 0, 0, nullptr, 0 };
    const reg32 amount = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogDensity(app, cpu, amount); });
}

/* sub_470b20: state 0xe likewise, times edx, the amount not kept. */
x86::reg32 renda::fogDensityTimes(App* app, CPU& cpu, reg32 amount, reg32 times)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (guest.dword(kDepthBias) == 0)
        return amount;
    // By address: what the render state leaves in edx is what this function leaves, which it does not save.
    return guest.call(kSetRenderState, args({ kStateFogDensity, fogShare(amount) * 5 * times }));
}

void rendFogDensityTimes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470b20", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 amount = cpu.eax;
    const reg32 times = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogDensityTimes(app, cpu, amount, times); });
}

/* sub_470b70: both: eax the colour, edx the amount. */
x86::reg32 renda::fogBoth(App* app, CPU& cpu, reg32 colour, reg32 amount)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x470ad0, fogDensity>(app, cpu, amount);
    return guest::direct<0x470aa0, fogColour>(app, cpu, colour);
}

void rendFogBoth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470b70", 0, 0, nullptr, 0 };
    const reg32 colour = cpu.eax;
    const reg32 amount = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogBoth(app, cpu, colour, amount); });
}

/* sub_470b90: as a race starts: the amount unknown, the colour none, state 0xe 0. */
x86::reg32 renda::fogReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0xc);
    guest.setDword(kFogAmountKept, 0xffffffff);
    guest.setDword(kFogColourKept, 0);
    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateFogDensity, 0);
    return guest.eax();
}

void rendFogReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_470b90", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::fogReset(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The race's font
 * ------------------------------------------------------------------------- */

/* sub_476fb0: how wide the string eax is drawn in the font at 0x679010, at the scale at
 * 0x679014: each character its width from the font's table, or 20 past its range, the
 * sum cut to an integer as it goes. */
x86::reg32 renda::stringWidth(App* app, CPU& cpu, reg32 text)
{
    constexpr reg32 kFont = 0x679010, kScale = 0x679014, kWidthOfNone = 0x53b0a0;
    constexpr reg32 kFontFirst = 0, kFontCount = 1, kFontWidths = 0x14;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x18);
    X87 x87(guest);
    std::int32_t width = 0;
    for (reg32 at = text; guest.byte(at) != 0; ++at)
    {
        const reg32 font = guest.dword(kFont);
        const reg32 character = guest.byte(at);
        const reg32 first = guest.byte(font + kFontFirst);
        const bool has = character >= first && sreg32(character) < sreg32(first + guest.byte(font + kFontCount));
        const reg32 widthAt = has ? (character << 5) + font + kFontWidths : kWidthOfNone;
        const Float scaled = fp::mul(fld(guest, kScale), fld(guest, widthAt));
        width = x87.truncated(fp::add(scaled, X87::integer(width)));
    }
    return reg32(width);
}

void rendStringWidth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_476fb0", 0, 0, nullptr, 0 };
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::stringWidth(app, cpu, text); });
}

/* ---------------------------------------------------------------------------
 * The sparks, on the screen
 * ------------------------------------------------------------------------- */

/* sub_4774b0: the current spark set's eight sparks ([0x55b088], if it is of kind 4): each
 * of kind 2 and alive that has run out goes, the others' places turned by the view
 * and put on the screen. */
x86::reg32 renda::sparksOnScreen(App* app, CPU& cpu)
{
    constexpr reg32 kCurrentSet = 0x55b088, kSets = 0x55b08c, kSetBytes = 0x238;
    constexpr reg32 kSetActive = 4, kSetAlive = 0x230;
    constexpr reg32 kSparkKind = 0x30, kSparkAlive = 0x3c, kSparkEnds = 0x40, kSparkWatched = 0x48;
    constexpr reg32 kSparkHidden = 0x54, kSparkX = 0x58, kSparkY = 0x5c, kSparkZ = 0x60, kSparkPlace = 0x64;
    constexpr reg32 kToScreenX = 0x53b0ac, kToScreenY = 0x53b0b0, kToScreenZ = 0x53b0b4;
    constexpr reg32 kFarthest = 0x42c80000;  // a hundred, as bits
    Guest guest(app, cpu);
    const reg32 current = guest.dword(kCurrentSet);
    const reg32 offset = current * kSetBytes;
    reg32 left = offset;  // what the game's eax holds at each way out
    const reg32 set = kSets + offset;
    if (current == 0xffffffff || guest.dword(set + kSetActive) == 0 || guest.dword(set) != 4)
        return left;
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 bytes = i << 6;
        const reg32 spark = set + bytes;
        left = spark;
        if (guest.dword(spark + kSparkKind) != 2 || guest.dword(spark + kSparkAlive) == 0)
            continue;
        bool over = false;
        const reg32 watched = guest.dword(spark + kSparkWatched);
        if (watched != 0)
            over = guest.dword(watched) == 0;
        else
            over = sreg32(guest.dword(kTicks)) > sreg32(guest.dword(spark + kSparkEnds));
        left = bytes;
        if (over)
        {
            guest.setDword(spark + kSparkAlive, 0);
            guest.setDword(set + kSetAlive, guest.dword(set + kSetAlive) - 1);
            continue;
        }
        const Vector turned = rotated(guest, spark + kSparkPlace, kViewMatrix);
        const float x = float(fp::add(Float(turned.x), fld(guest, kViewShift)));
        const float y = float(fp::add(Float(turned.y), fld(guest, kViewShift + 4)));
        float z = float(fp::add(Float(turned.z), fld(guest, kViewShift + 8)));
        guest.setDword(spark + kSparkHidden, 0);
        const reg32 zBits = bitsOf(z);
        if (jumpAbove(fp::compare(Float(0.0), Float(z))) || sreg32(zBits) > sreg32(kFarthest))
        {
            guest.setDword(spark + kSparkHidden, 1);
            continue;
        }
        if ((zBits & kNoSign) == 0)
            z = floatOf(kFloatTiny);
        const Float inverse = fp::div(Float(1.0), Float(z));
        const Float sx = fp::add(fp::mul(fp::mul(Float(x), inverse), fld(guest, kScreenScaleX)), fld(guest, kScreenCentreX));
        guest.setFloat(spark + kSparkX, float(fp::mul(sx, fld(guest, kToScreenX))));
        const Float sy = fp::add(fp::mul(fp::mul(inverse, Float(y)), fld(guest, kScreenScaleY)), fld(guest, kScreenCentreY));
        guest.setFloat(spark + kSparkY, float(fp::mul(sy, fld(guest, kToScreenY))));
        guest.setFloat(spark + kSparkZ, float(fp::mul(Float(z), fld(guest, kToScreenZ))));
    }
    return left;
}

void rendSparksOnScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4774b0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::sparksOnScreen(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * A quad, the view's rectangle and the sky's list
 * ------------------------------------------------------------------------- */

/* sub_478cb0: the polygon's record eax as a quad: four vertices by value and their colours,
 * opaque, on the stack, the texture's corners in order, no texture, and a flag: 0 no depth. */
x86::reg32 renda::quadRecord(App* app, CPU& cpu, reg32 polygon)
{
    constexpr reg32 kFrame = 0x20;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kFrame);
    // The four vertices, their colours and the flag where the caller put them, above the return address.
    const reg32 stack = guest.frame() + kFrame + 4;
    guest.setWord(polygon + kPolygonA, 0);
    guest.setWord(polygon + kPolygonFlags, 0);
    guest.setDword(polygon + kPolygonTexture, 0);
    const reg32 first = polygon + kPolygonVertex0;
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(polygon + kPolygonVertices + 4 * i, first + i * kVertexBytes);
    if (guest.dword(stack + 4 * kVertexBytes + 4 * 4) == 0)
        guest.setByte(polygon + kPolygonFlags, std::uint8_t(guest.byte(polygon + kPolygonFlags) | kPolygonNoDepth));
    for (reg32 i = 0; i < 4; ++i)
        copyDwords(guest, first + i * kVertexBytes, stack + i * kVertexBytes, kVertexDwords);
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(first + i * kVertexBytes + kVertexColour, guest.dword(stack + 4 * kVertexBytes + 4 * i) | kOpaque);
    static const reg32 kU[4] = { 0, kFloatOne, kFloatOne, 0 };
    static const reg32 kV[4] = { 0, 0, kFloatOne, kFloatOne };
    for (reg32 i = 0; i < 4; ++i)
    {
        guest.setDword(first + i * kVertexBytes + kVertexU, kU[i]);
        guest.setDword(first + i * kVertexBytes + kVertexV, kV[i]);
    }
    return first;
}

void rendMakeQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_478cb0", kQuadStackBytes, 0, nullptr, 0 };
    const reg32 polygon = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::quadRecord(app, cpu, polygon); });
}

/* sub_478dc0: the view's rectangle at 0x6fdc40, its right and bottom for its sizes. */
x86::reg32 renda::viewRectangle(App* app, CPU& cpu)
{
    constexpr reg32 kRight = 0x6fdc40, kLeft = 0x6fdc44, kTop = 0x6fdc48, kBottom = 0x6fdc4c;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x10);
    guest.call(kViewport, args({ kLeft, kTop, kRight, kBottom }));
    const reg32 left = guest.dword(kLeft);
    const reg32 right = guest.dword(kRight) + left;
    const reg32 bottom = guest.dword(kBottom) + guest.dword(kTop);
    guest.setDword(kRight, right);
    guest.setDword(kBottom, bottom);
    return left;
}

void rendViewRectangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_478dc0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::viewRectangle(app, cpu); });
}

/* sub_479080: the polygons' list started, with a mark first. */
x86::reg32 renda::skyListStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0xc);
    fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, cpu);
    const reg32 slot = guest.frame();
    const reg32 added = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kListMarkBytes);
    if (added == 0)
        return added;
    const reg32 record = guest.dword(slot);
    fillListMark(guest, record, 0);
    return record;
}

void rendSkyListStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479080", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyListStart(app, cpu); });
}

/* sub_4790d0: the list ended and kept for the pass eax, a new one started. */
x86::reg32 renda::skyListKeep(App* app, CPU& cpu, reg32 pass)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x18);
    const reg32 ends = guest.frame();
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, cpu, ends);
    const reg32 drawn = fe::direct<0x4bc030, frontend::rendd::drawListPlain>(app, cpu, pass, ends);
    fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, cpu);
    // eax as the code leaves it: the list's start leaves the eax it came with, the drawing's.
    return drawn;
}

void rendSkyListKeep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4790d0", 0, 0, nullptr, 0 };
    const reg32 pass = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyListKeep(app, cpu, pass); });
}

/* sub_479100: the list ended with a mark that hands back the driver's [0x7a3a60], its
 * two ends to esi. */
x86::reg32 renda::skyListEnd(App* app, CPU& cpu, reg32 out)
{
    constexpr reg32 kFrame = 0x1c;
    constexpr reg32 kSlot = 8;  // [ebp - 4]
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kFrame);
    const reg32 ends = guest.frame();
    const reg32 slot = ends + kSlot;
    if (fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kListMarkBytes) != 0)
        fillListMark(guest, guest.dword(slot), guest.dword(kDriverOnCard));
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, cpu, ends);
    copyDwords(guest, out, ends, 2);
    guest.setEsi(ends + 8);
    return out;
}

void rendSkyListEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479100", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyListEnd(app, cpu, out); });
}

/* ---------------------------------------------------------------------------
 * The sky's rings
 * ------------------------------------------------------------------------- */

/* sub_479160: a band's three points, to the third stack argument +0, +0xc and +0x18, from
 * the view eax: the wind's direction by the view's matrix, flat and made a unit, its angle
 * a turn to the fourth; the band's side, across it and up; each at its length and raised.
 * The stack: the rise, the upper's length, the points, the turn. */
x86::reg32 renda::bandPoints(App* app, CPU& cpu, reg32 view, reg32 riseBits, reg32 upperLength, reg32 points, reg32 turn)
{
    constexpr reg32 kFrame = 0x38;
    constexpr reg32 kPhase = 0x53b2c8, kTwoPi = 0x53b2cc, kSixteenth = 0x53b2d0;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kFrame);
    const reg32 towardsCentre = guest.frame() + 0xc, towardsUp = guest.frame();
    const reg32 side = points + 0x18, upper = points + 0xc;
    putVector(guest, points, rotated(guest, kSkyWind, view + kViewSkyMatrix));
    guest.setDword(points + 4, 0);
    fe::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, points, points);
    const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, cpu, guest.dword(points),
                                                                                  guest.dword(points + 8));
    const Float angle = fp::add(fp::add(heading, fld(guest, kPhase)), fld(guest, kSkyTurn));
    guest.setFloat(turn, float(angle));
    guest::direct<0x4964d0, frontend::rendc::wrapFraction>(app, cpu, turn);
    subtractOne(guest, cpu, kSkyCentre, points, towardsCentre);
    subtractOne(guest, cpu, kSkyUp, points, towardsUp);
    fe::direct<0x4e01a0, frontend::simd::unitCross>(app, cpu, towardsCentre, towardsUp, side);
    const float sideLength = float(fp::mul(fp::mul(fld(guest, kSkyRadius), fld(guest, kTwoPi)), fld(guest, kSixteenth)));
    scaleOne(guest, cpu, points, guest.dword(kSkyRadius), points);
    scaleOne(guest, cpu, kSkyUp, upperLength, upper);
    scaleOne(guest, cpu, side, bitsOf(sideLength), side);
    addOne(guest, cpu, upper, points, upper);
    addOne(guest, cpu, side, points, side);
    const Float a = fld(guest, points + 4), b = fld(guest, points + 0x10), c = fld(guest, points + 0x1c);
    const Float rise = Float(floatOf(riseBits));
    guest.setFloat(points + 4, float(fp::add(a, rise)));
    guest.setFloat(points + 0x10, float(fp::add(b, rise)));
    guest.setFloat(points + 0x1c, float(fp::add(rise, c)));
    return reg32(1);
}

void rendBandPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479160", 0x10, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    const reg32 rise = stackWord(app, cpu, 0);
    const reg32 upperLength = stackWord(app, cpu, 1);
    const reg32 points = stackWord(app, cpu, 2);
    const reg32 turn = stackWord(app, cpu, 3);
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::bandPoints(app, cpu, view, rise, upperLength, points, turn); });
}

/* sub_4792e0: eax points to ebx, on from the two at edx and as far apart. */
x86::reg32 renda::curvePoints(App* app, CPU& cpu, reg32 howMany, reg32 in, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x28);
    const sreg32 count = sreg32(howMany);
    const reg32 step = guest.frame();
    // By address: what the vectors' functions leave in ebx and edx is what this one leaves.
    guest.call(kSubtractVectors, args({ 1, in + kPointBytes, in, step }));
    copyDwords(guest, out, in, 3);
    copyDwords(guest, out + kPointBytes, in + kPointBytes, 3);
    reg32 left = in;
    for (sreg32 i = 2; i < count; ++i)
        left = guest.call(kAddVectors, args({ 1, out + reg32(i - 1) * kPointBytes, step, out + reg32(i) * kPointBytes }));
    return left;
}

void rendCurvePoints(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4792e0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 count = cpu.eax;
    const reg32 in = cpu.edx;
    const reg32 out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::curvePoints(app, cpu, count, in, out); });
}

/* sub_479370: eax points at ebx as vertices at ecx, by the view edx's camera matrix,
 * their clip codes to the first of the stack's arguments, the colour the second. */
x86::reg32 renda::bandVertices(App* app, CPU& cpu, reg32 count, reg32 view, reg32 points, reg32 vertices, reg32 codes, reg32 colour)
{
    Guest guest(app, cpu);
    const reg32 matrix = view + kViewCameraMatrix;
    reg32 vertex = vertices, result = count;
    while (--count != 0xffffffff)
    {
        const Vector turned = rotated(guest, points, matrix);
        reg32 zBits = bitsOf(turned.z);
        if ((zBits & kNoSign) == 0)
            zBits = kFloatTiny;
        const float inverse = float(fp::div(Float(1.0), Float(floatOf(zBits))));
        const Float scaleX = fp::mul(fld(guest, kScreenScaleX), Float(inverse));
        const Float scaleY = fp::mul(fld(guest, kScreenScaleY), Float(inverse));
        const Float y = fp::mul(scaleY, Float(turned.y));
        guest.setFloat(vertex + kVertexW, inverse);
        const Float x = fp::mul(scaleX, Float(turned.x));
        guest.setFloat(vertex, float(fp::add(x, fld(guest, kScreenCentreX))));
        guest.setDword(vertex + 8, zBits);
        guest.setFloat(vertex + 4, float(fp::add(y, fld(guest, kScreenCentreY))));
        const Clip clip = clipOf(guest, vertex);
        guest.setByte(vertex + kVertexClip, std::uint8_t(clip.code));
        guest.setDword(codes, guest.byte(vertex + kVertexClip));
        guest.setDword(vertex + kVertexColour, colour);
        guest.leaveEbx(clip.code);
        guest.leaveEcx(clip.bound);
        vertex += kVertexBytes;
        result = vertex;
        points += kPointBytes;
        codes += 4;
    }
    guest.leaveEdx(0xffffffff);
    return result;
}

void rendBandVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479370", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 count = cpu.eax;
    const reg32 view = cpu.edx;
    const reg32 points = cpu.ebx;
    const reg32 vertices = cpu.ecx;
    const reg32 codes = stackWord(app, cpu, 0);
    const reg32 colour = stackWord(app, cpu, 1);
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::bandVertices(app, cpu, count, view, points, vertices, codes, colour); });
}

/* sub_479500: a ring's two curves of nine points, from the view eax: the band's points
 * (sub_479160) taken back four of its steps and on by the fraction of a sixteenth the turn
 * stands at, the lower curve to the last argument, the upper to the one before.
 * The stack: its height, its upper's length, the turn, then the curves. */
x86::reg32 renda::ringPoints(App* app, CPU& cpu, reg32 view, reg32 height, reg32 upperLength, reg32 turn, reg32 upperCurve, reg32 lowerCurve)
{
    constexpr reg32 kFrame = 0x74;
    constexpr reg32 kEbp = 0x5c;
    constexpr reg32 kSixteen = 0x53b2d4;
    constexpr reg32 kFourBits = 0x40800000;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kFrame);
    X87 x87(guest);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 band = ebp - 0x5c, upper = ebp - 0x50, side = ebp - 0x44;
    const reg32 curve = ebp - 0x38, across = ebp - 0x20, moved = ebp - 0x14;
    const float rise = float(fp::sub(Float(floatOf(height)), fld(guest, view + kViewHeight)));
    const reg32 made = guest::direct<0x479160, bandPoints>(app, cpu, view, bitsOf(rise), upperLength, band, turn);
    if (made == 0)
        return made;
    subtractOne(guest, cpu, side, band, across);
    scaleOne(guest, cpu, across, kFourBits, moved);
    subtractOne(guest, cpu, band, moved, band);
    subtractOne(guest, cpu, upper, moved, upper);
    subtractOne(guest, cpu, side, moved, side);
    const Float sixteenths = fp::mul(fld(guest, turn), fld(guest, kSixteen));
    const float exact = float(sixteenths);
    const std::int32_t step = x87.truncated(sixteenths);
    const float part = float(fp::sub(Float(exact), X87::integer(step)));
    scaleOne(guest, cpu, across, bitsOf(part), moved);
    subtractOne(guest, cpu, band, moved, band);
    subtractOne(guest, cpu, upper, moved, upper);
    subtractOne(guest, cpu, side, moved, side);
    copyDwords(guest, curve, band, 3);
    copyDwords(guest, curve + kPointBytes, side, 3);
    guest::direct<0x4792e0, curvePoints>(app, cpu, kRingPointCount, curve, lowerCurve);
    subtractOne(guest, cpu, side, band, across);
    addOne(guest, cpu, upper, across, curve + kPointBytes);
    copyDwords(guest, curve, upper, 3);
    guest::direct<0x4792e0, curvePoints>(app, cpu, kRingPointCount, curve, upperCurve);
    return reg32(1);
}

void rendRingPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479500", 0x14, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    const reg32 height = stackWord(app, cpu, 0);
    const reg32 upperLength = stackWord(app, cpu, 1);
    const reg32 turn = stackWord(app, cpu, 2);
    const reg32 upperCurve = stackWord(app, cpu, 3);
    const reg32 lowerCurve = stackWord(app, cpu, 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::ringPoints(app, cpu, view, height, upperLength, turn, upperCurve, lowerCurve); });
}

/* sub_479680: the ring of the sky's gradient from the view eax, the gradient at the horizon
 * running to the two tables' colours, turned over for the far half of a two-sided sky. */
x86::reg32 renda::skyGradientRing(App* app, CPU& cpu, reg32 view)
{
    constexpr reg32 kEbp = 0x2ea;
    constexpr reg32 kSixteen = 0x53b2d8;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x384);
    X87 x87(guest);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 turn = ebp + 0x76, slot = ebp + 0x7a;
    const reg32 upperCurve = ebp - 0xaa, lowerCurve = ebp - 0x3e;
    const reg32 lower = ebp - 0x1ca, upper = ebp - 0x2ea;
    const float length = float(fp::sub(fld(guest, kSkyMiddle), fld(guest, kSkySpread)));
    const float height = float(fp::add(fld(guest, kSkyBottom), fld(guest, kSkySpread)));
    if (guest::direct<0x479500, ringPoints>(app, cpu, view, bitsOf(height), bitsOf(length), turn, upperCurve, lowerCurve) == 0)
        return reg32(0);
    const std::int32_t start = x87.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen)));
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, lowerCurve, lower, ebp + 0x52, 0xffffffff);
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, upperCurve, upper, ebp + 0x2e, 0xffffffff);
    reg32 left = 0;  // what eax is left with: the last record made, or the list's 0
    for (reg32 i = 0; i < kRingSegments; ++i)
    {
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const reg32 segment = ringSegment(reg32(start), i);
        const reg32 at = i * kVertexBytes;
        const reg32 low = guest.dword(segment * 4 + kSkyColoursLow), high = guest.dword(segment * 4 + kSkyColoursHigh);
        const reg32 gradient = guest.dword(kSkyGradient);
        Quad quad;
        if (guest.dword(kSkyTwoSided) != 0 && sreg32(segment) > 7)
            quad = { { upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes, lower + at },
                     { gradient, gradient, high, low }, 0 };
        else
            quad = { { lower + at, upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes },
                     { low, gradient, gradient, high }, 0 };
        left = makeQuad(guest, cpu, guest.dword(slot), quad);
    }
    return left;
}

void rendSkyGradientRing(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479680", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyGradientRing(app, cpu, view); });
}

/* sub_479860: the colour eax with the fog's share at 0x55d268 of the colour at 0x55d26c, on
 * a driver without a depth bias, which fogs none of its own; as it came on the others. */
x86::reg32 renda::foggedColour(App* app, CPU& cpu, reg32 colour)
{
    constexpr reg32 kShare = 0x55d268, kFogColourAt = 0x55d26c, kHalf = 0x53b2e0;
    Guest guest(app, cpu);
    if (guest.dword(kDepthBias) != 0)
        return colour;
    const reg32 fog = guest.dword(kFogColourAt);
    const Float share = fld(guest, kShare);
    const float rest = float(fp::sub(Float(1.0), share));
    // A channel: its share of both, kept to a byte.
    const auto blend = [&](unsigned shift) {
        const Float own = fp::mul(Float(sreg32((colour >> shift) & 0xff)), Float(rest));
        const float fogged = float(fp::mul(Float(sreg32((fog >> shift) & 0xff)), share));
        const Float value = fp::mul(fp::add(own, Float(fogged)), doubleAt(guest, kHalf));
        const sreg32 whole = fp::toInteger<sreg32>(value);
        return reg32(whole < 0 ? 0 : whole > 0xff ? 0xff : whole);
    };
    const reg32 red = blend(16), green = blend(8), blue = blend(0);
    return kOpaque | (red << 16) | (green << 8) | blue;
}

void rendFoggedColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479860", 0, 0, nullptr, 0 };
    const reg32 colour = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::foggedColour(app, cpu, colour); });
}

/* sub_4799d0: the ring of the fog's haze from the view eax, in the two fogged colours at
 * 0x55d1c4 and 0x55d1c0. */
x86::reg32 renda::skyHazeRing(App* app, CPU& cpu, reg32 view)
{
    constexpr reg32 kEbp = 0x2f2;
    constexpr reg32 kSixteen = 0x53b2e8, kHazeLow = 0x55d1c4, kHazeHigh = 0x55d1c0;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x38c);
    X87 x87(guest);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 turn = ebp + 0x72, slot = ebp + 0x6e;
    const reg32 upperCurve = ebp - 0xb2, lowerCurve = ebp - 0x46;
    const reg32 lower = ebp - 0x1d2, upper = ebp - 0x2f2;
    if (guest::direct<0x479500, ringPoints>(app, cpu, view, guest.dword(kSkyBottom), guest.dword(kSkySpread), turn,
                                            upperCurve, lowerCurve) == 0)
        return reg32(0);
    const std::int32_t start = x87.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen)));
    const reg32 low = guest::direct<0x479860, foggedColour>(app, cpu, guest.dword(kHazeLow));
    const reg32 high = guest::direct<0x479860, foggedColour>(app, cpu, guest.dword(kHazeHigh));
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, lowerCurve, lower, ebp + 0x4a, 0xffffffff);
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, upperCurve, upper, ebp + 0x26, 0xffffffff);
    reg32 left = 0;  // what eax is left with: the last record made, or the list's 0
    for (reg32 i = 0; i < kRingSegments; ++i)
    {
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const reg32 segment = ringSegment(reg32(start), i);
        const reg32 at = i * kVertexBytes;
        Quad quad;
        if (guest.dword(kSkyTwoSided) != 0 && sreg32(segment) > 7)
            quad = { { upper + at + kVertexBytes, upper + at, lower + at, lower + at + kVertexBytes },
                     { high, high, low, low }, 1 };
        else
            quad = { { upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes, lower + at },
                     { high, high, low, low }, 1 };
        left = makeQuad(guest, cpu, guest.dword(slot), quad);
    }
    return left;
}

void rendSkyHazeRing(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4799d0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyHazeRing(app, cpu, view); });
}

/* sub_479b90: the ring of the clouds from the view eax: three rows of nine points, the
 * upper two moved by the wind's table at 0x7001e8, each cell a textured polygon unless all
 * four of its corners are off one side of the screen. */
x86::reg32 renda::skyCloudRing(App* app, CPU& cpu, reg32 view)
{
    constexpr reg32 kEbp = 0x536;
    constexpr reg32 kWhiteInTheCode = 0x478310;  // 0xffffffff, read from among the code
    constexpr reg32 kSixteen = 0x53b2ec;
    constexpr reg32 kWind = 0x7001e8, kCloudTextures = 0x6ff95c;
    constexpr reg32 kHundredth = 0x3c23d70a;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x5d0);
    X87 x87(guest);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 colour = guest.dword(kWhiteInTheCode);
    const reg32 turn = ebp + 0x66, slot = ebp + 0x5e;
    const reg32 bottom = ebp - 0xfe, top = ebp - 0x1d6, middle = ebp - 0x92, above = ebp - 0x16a;
    const reg32 difference = ebp + 0x46, scaled = ebp + 0x52;
    const reg32 topVertices = ebp - 0x2f6, middleVertices = ebp - 0x416, aboveVertices = ebp - 0x536;
    const reg32 topCodes = ebp - 2, middleCodes = ebp + 0x22, aboveCodes = ebp - 0x26;
    const float length = float(fp::sub(fld(guest, kSkyTop), fld(guest, kSkyTopSpread)));
    const float height = float(fp::add(fld(guest, kSkyTopSpread), fld(guest, kSkyBottom)));
    if (guest::direct<0x479500, ringPoints>(app, cpu, view, bitsOf(height), bitsOf(length), turn, bottom, top) == 0)
        return reg32(0);
    const reg32 start = reg32(x87.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen))));
    for (reg32 i = 0; i < kRingSegments; ++i)
    {
        const reg32 segment = ringSegment(start, i);
        const reg32 at = i * kPointBytes;
        subtractOne(guest, cpu, bottom + at, top + at, difference);
        scaleOne(guest, cpu, difference, guest.dword(segment * 4 + kWind), scaled);
        addOne(guest, cpu, top + at, scaled, middle + at);
        addOne(guest, cpu, top + at + kPointBytes, scaled, above + at);
        subtractOne(guest, cpu, middle + at, above + at, difference);
        scaleOne(guest, cpu, difference, kHundredth, scaled);
        addOne(guest, cpu, middle + at, scaled, middle + at);
    }
    const reg32 fogged = guest::direct<0x479860, foggedColour>(app, cpu, colour);
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, top, topVertices, topCodes, fogged);
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, middle, middleVertices, middleCodes, fogged);
    guest::direct<0x479370, bandVertices>(app, cpu, kRingPointCount, view, above, aboveVertices, aboveCodes, fogged);
    reg32 left = 0;
    for (reg32 i = 0; i < kRingSegments; ++i)
    {
        const reg32 firstCode = guest.dword(topCodes + 4 * i);
        left = firstCode;
        if ((guest.dword(topCodes + 4 * (i + 1)) & firstCode & guest.dword(middleCodes + 4 * i) & guest.dword(aboveCodes + 4 * i)) != 0)
            continue;
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const reg32 polygon = guest.dword(slot);
        const reg32 segment = ringSegment(start, i);
        left = polygon + 0x40;
        guest.setWord(polygon + kPolygonA, 0);
        guest.setWord(polygon + kPolygonFlags, 0);
        guest.setDword(polygon + kPolygonTexture, guest.dword(segment * 4 + kCloudTextures));
        const reg32 first = polygon + kPolygonVertex0;
        for (reg32 v = 0; v < 4; ++v)
            guest.setDword(polygon + kPolygonVertices + 4 * v, first + v * kVertexBytes);
        const reg32 at = i * kVertexBytes;
        reg32 from[4];
        if (guest.dword(kSkyTwoSided) != 0 && sreg32(segment) > 7)
        {
            from[0] = aboveVertices + at, from[1] = middleVertices + at;
            from[2] = topVertices + at, from[3] = topVertices + at + kVertexBytes;
        }
        else
        {
            from[0] = middleVertices + at, from[1] = aboveVertices + at;
            from[2] = topVertices + at + kVertexBytes, from[3] = topVertices + at;
        }
        for (reg32 v = 0; v < 4; ++v)
            copyDwords(guest, first + v * kVertexBytes, from[v], kVertexDwords);
    }
    return left;
}

void rendSkyCloudRing(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479b90", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyCloudRing(app, cpu, view); });
}

/* ---------------------------------------------------------------------------
 * The sun
 * ------------------------------------------------------------------------- */

/* sub_479ed0: the sun's quad for the view eax, when the track has one ([0x55d27c]): the sun's
 * place on the screen, its corners round it by its angle -- towards its second point, or
 * from where it stands. */
x86::reg32 renda::sun(App* app, CPU& cpu, reg32 view)
{
    constexpr reg32 kEbp = 0x5a;
    constexpr reg32 kHasSun = 0x55d27c, kSunAt = 0x55d280, kSunTowards = 0x55d28c, kSunSize = 0x55d298, kSunAtCentre = 0x55d29c;
    constexpr reg32 kSunFlags = 0x55d2a0, kSunTexture = 0x6ff9cc;
    constexpr reg32 kEighth = 0x53b2f0;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0xf4);
    if (guest.dword(kHasSun) == 0)
        return view;
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 slot = ebp + 0x7a, points = ebp + 0x26, vertices = ebp - 0x5a;
    const reg32 matrix = view + kViewCameraMatrix;
    const Vector at = rotated(guest, kSunAt, matrix);
    Float angle;
    if (guest.dword(kSunAtCentre) == 0)
    {
        const Vector towards = rotated(guest, kSunTowards, matrix);
        const float x = float(fp::sub(Float(towards.x), Float(at.x)));
        const float y = float(fp::sub(Float(towards.y), Float(at.y)));
        const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, cpu, bitsOf(y), bitsOf(x));
        angle = fp::add(heading, doubleAt(guest, kEighth));
    }
    else
    {
        const std::uint16_t status = fp::compare(Float(0.0), Float(at.z));
        if (jumpAboveOrEqual(status))
            return (kSunAt & 0xffff0000) | status;
        angle = fp::div(Float(at.x), Float(at.z));
    }
    const reg32 angleBits = bitsOf(float(angle));
    const Float sineOfAngle = fe::direct<0x4ea930, frontend::simd::sineValue>(app, cpu, angleBits);
    const float sine = float(fp::mul(sineOfAngle, fld(guest, kSunSize)));
    const Float cosineOfAngle = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, cpu, angleBits);
    const Float cosine = fp::mul(cosineOfAngle, fld(guest, kSunSize));
    const Float x = Float(at.x), y = Float(at.y), s = Float(sine);
    const reg32 z = bitsOf(at.z);
    guest.setFloat(points, float(fp::sub(x, s)));
    guest.setDword(points + 8, z);
    guest.setFloat(points + 0x10, float(fp::add(y, s)));
    guest.setDword(points + 0x14, z);
    guest.setDword(points + 0x20, z);
    guest.setDword(points + 0x2c, z);
    guest.setFloat(points + 0x18, float(fp::add(x, s)));
    guest.setFloat(points + 0x28, float(fp::sub(y, s)));
    guest.setFloat(points + 4, float(fp::add(y, cosine)));
    guest.setFloat(points + 0xc, float(fp::add(x, cosine)));
    guest.setFloat(points + 0x1c, float(fp::sub(y, cosine)));
    guest.setFloat(points + 0x24, float(fp::sub(x, cosine)));
    fe::direct<0x4bf370, frontend::rendd::projectPoints>(app, cpu, 4, points, vertices);
    const reg32 lastCode = guest.byte(vertices + 3 * kVertexBytes + kVertexClip);
    reg32 codes = guest.byte(vertices + kVertexClip);
    codes &= guest.byte(vertices + kVertexBytes + kVertexClip);
    codes &= guest.byte(vertices + 2 * kVertexBytes + kVertexClip);
    if ((codes & lastCode) != 0)
        return lastCode;
    const reg32 added = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kPolygonBytes);
    if (added == 0)
        return added;
    const reg32 polygon = guest.dword(slot);
    const reg32 first = polygon + kPolygonVertex0;
    for (reg32 v = 0; v < 4; ++v)
        guest.setDword(polygon + kPolygonVertices + 4 * v, first + v * kVertexBytes);
    guest.setWord(polygon + kPolygonA, 0);
    guest.setWord(polygon + kPolygonFlags, std::uint16_t(guest.word(kSunFlags) | kPolygonNoDepth));
    guest.setDword(polygon + kPolygonTexture, kSunTexture);
    for (reg32 v = 0; v < 4; ++v)
    {
        copyDwords(guest, first + v * kVertexBytes, vertices + v * kVertexBytes, kVertexDwords);
        guest.setDword(first + v * kVertexBytes + kVertexColour, 0xffffffff);
    }
    return first + 3 * kVertexBytes;
}

void rendSun(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_479ed0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::sun(app, cpu, view); });
}

/* ---------------------------------------------------------------------------
 * The sky's colour
 * ------------------------------------------------------------------------- */

/* sub_47a4c0: whether the sky goes flat for the car eax's view: the switch [0x55d158], or
 * the camera in the tunnel of track 5.  In a split screen a black quad over the view's
 * rectangle, 0x3f7fff00 deep; alone, the fog goes black.  Otherwise the fog takes the sky's
 * gradient.  State 3 as it now stands either way. */
x86::reg32 renda::skyFlat(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kEbp = 0x1e;
    constexpr reg32 kFlatSky = 0x55d158;
    constexpr reg32 kTunnelTrack = 5;
    constexpr sreg32 kTunnelFirst = 0x38a, kTunnelLast = 0x428;
    constexpr reg32 kFogColourBytes = 0x6ff9f8;
    constexpr reg32 kFarDepth = 0x3f7fff00;
    constexpr reg32 kBlack = 0xff000000;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0xb8);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 node = fe::direct<0x4217e0, carstate::nodeOfCamera>(app, cpu, guest.dword(car + 4));
    reg32 flat = guest.dword(kFlatSky) != 0 ? 1 : 0;
    if (gameSettings(guest).track == kTunnelTrack && sreg32(node) >= kTunnelFirst && sreg32(node) <= kTunnelLast)
        flat = 1;
    if (gameSettings(guest).playMode == 1 && flat != 0)
    {
        const reg32 slot = ebp + 0x62;
        if (fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, cpu, slot, kPolygonBytes) != 0)
        {
            const reg32 leftAt = ebp + 0x66, topAt = ebp + 0x6a, widthAt = ebp + 0x6e, heightAt = ebp + 0x72;
            const reg32 corner0 = ebp + 2, corner1 = ebp + 0x42, corner2 = ebp + 0x22, corner3 = ebp - 0x1e;
            guest.call(kViewport, args({ leftAt, topAt, widthAt, heightAt }));
            const reg32 corners[4] = { corner0, corner1, corner2, corner3 };
            for (const reg32 corner : corners)
            {
                guest.setDword(corner + 8, kFarDepth);
                guest.setDword(corner + kVertexW, kFarDepth);
            }
            const reg32 left = guest.dword(leftAt), top = guest.dword(topAt);
            const reg32 right = left + guest.dword(widthAt), bottom = top + guest.dword(heightAt);
            const reg32 xs[4] = { left, right, right, left };
            const reg32 ys[4] = { top, top, bottom, bottom };
            for (reg32 i = 0; i < 4; ++i)
            {
                guest.setFloat(corners[i], float(Float(sreg32(xs[i]))));
                guest.setFloat(corners[i] + 4, float(Float(sreg32(ys[i]))));
                guest.setByte(corners[i] + kVertexClip, std::uint8_t(clipOf(guest, corners[i]).code));
            }
            makeQuad(guest, cpu, guest.dword(slot),
                     { { corner0, corner1, corner2, corner3 }, { kBlack, kBlack, kBlack, kBlack }, 0 });
        }
    }
    else if (flat != 0)
    {
        guest.setDword(kFogColourBytes, 0);
    }
    else
    {
        guest.setByte(kFogColourBytes + 3, 0xff);
        guest.setByte(kFogColourBytes + 2, guest.byte(kSkyGradient + 2));
        guest.setByte(kFogColourBytes + 1, guest.byte(kSkyGradient + 1));
        guest.setByte(kFogColourBytes, guest.byte(kSkyGradient));
    }
    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateRaceFogColour, guest.dword(kFogColourBytes));
    return flat;
}

void rendSkyFlat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47a4c0", 0, 0, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyFlat(app, cpu, car); });
}

/* ---------------------------------------------------------------------------
 * The dome's grid
 * ------------------------------------------------------------------------- */

/* sub_47adc0: the point eax's height from its distance: a cosine's curve down the sky. */
x86::reg32 renda::pointHeight(App* app, CPU& cpu, reg32 point)
{
    constexpr reg32 kQuarter = 0x53b3b8, kPerDegree = 0x53b3c0;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x18);
    const reg32 flat = guest.frame();
    guest.setDword(flat, guest.dword(point));
    guest.setDword(flat + 4, 0);
    guest.setDword(flat + 8, guest.dword(point + 8));
    const Float length = Float(fe::direct<0x4e06a0, vertices::vectorLength>(app, cpu, flat));
    const Float angle = fp::mul(fp::mul(length, doubleAt(guest, kQuarter)), doubleAt(guest, kPerDegree));
    const Float cosine = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, cpu, bitsOf(float(angle)));
    const Float curve = fp::mul(cosine, fld(guest, kSkyCurveScale));
    guest.setFloat(point + 4, float(fp::add(curve, fld(guest, kSkyCurveShift))));
    // eax as the code leaves it: the flat vector's address, which the length's function and the
    // cosine leave there.
    return flat;
}

void rendPointHeight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47adc0", 0, 0, nullptr, 0 };
    const reg32 point = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::pointHeight(app, cpu, point); });
}

/* sub_47ae10: the dome's grid made as the track loads: each point's place and height, its
 * brightness a bit of noise below the brightest, those made into hills (a root's curve),
 * smoothed twenty times, kept between nothing and the brightest, and the result lowering
 * each point and giving its colour. */
x86::reg32 renda::cloudGrid(App* app, CPU& cpu)
{
    constexpr reg32 kToSky = 0x53b3c8, kMinus128 = 0x53b3d0, kPerByte = 0x53b3d4, kHalf = 0x53b3d8, kFull = 0x53b3dc;
    constexpr reg32 kPerStep = 0x53b3e0, kOfByte = 0x53b3e8, kHalfDouble = 0x53b3f0;
    constexpr reg32 kPasses = 20;
    constexpr sreg32 kReach = 2;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x318);
    X87 x87(guest);
    const reg32 grid = guest.frame();
    const auto cell = [&](reg32 row, reg32 column) { return grid + (row * kGrid + column) * 4; };
    const auto gridAxis = [](reg32 i) { return Float(sreg32(((i * 225) << 17) - 0xbb80000)); };

    for (reg32 column = 0; column < kGrid; ++column)
    {
        for (reg32 row = 0; row < kGrid; ++row)
        {
            const reg32 point = kCloudPoints + row * kGridRowBytes + column * kPointBytes;
            const reg32 corner = kCloudCorners + row * kGridRowBytes + column * kPointBytes;
            const Float toSky = doubleAt(guest, kToSky);
            const Float x = fp::mul(gridAxis(row), toSky);
            const Float z = fp::mul(toSky, gridAxis(column));
            guest.setFloat(point, float(x));
            guest.setFloat(point + 8, float(z));
            guest.setFloat(corner, float(Float(sreg32(row))));
            guest.setFloat(corner + 4, float(Float(sreg32(column))));
            reg32 brightness;
            if (guest.dword(kCloudNoise) != 0)
            {
                const reg32 random = guest.call(kRandom);
                brightness = guest.dword(kCloudBrightest) - random % guest.dword(kCloudNoise);
            }
            else
            {
                brightness = guest.dword(kCloudBrightest);
            }
            guest.setFloat(cell(row, column), float(Float(sreg32(brightness))));
            guest::direct<0x47adc0, pointHeight>(app, cpu, point);
        }
    }

    for (reg32 column = 0; column < kGrid; ++column)
    {
        for (reg32 row = 0; row < kGrid; ++row)
        {
            const float centred = float(fp::mul(fp::add(fld(guest, cell(row, column)), fld(guest, kMinus128)), fld(guest, kPerByte)));
            const Float sign = Float(jumpBelowOrEqual(fp::compare(Float(0.0), Float(centred))) ? 1.0f : -1.0f);
            const Float magnitude = fe::direct<0x4ea920, frontend::simc::squareRootOf>(
                app, cpu, bitsOf(float(fp::mul(Float(centred), sign))));
            const float root = float(fp::mul(magnitude, sign));
            const Float scaled = fp::mul(fp::mul(fp::add(Float(1.0), Float(root)), fld(guest, kHalf)), fld(guest, kFull));
            guest.setFloat(cell(row, column), float(scaled));
        }
    }

    for (reg32 pass = 0; pass < kPasses; ++pass)
    {
        for (reg32 column = 0; column < kGrid; ++column)
        {
            for (reg32 row = 0; row < kGrid; ++row)
            {
                const reg32 own = guest.dword(cell(row, column));
                float sum = 0.0f, weight = 0.0f;
                for (sreg32 dc = -kReach; dc <= kReach; ++dc)
                {
                    for (sreg32 dr = -kReach; dr <= kReach; ++dr)
                    {
                        if (dc == 0 && dr == 0)
                            continue;
                        const sreg32 r = sreg32(row) + dr, c = sreg32(column) + dc;
                        if (r < 0 || c < 0 || r >= sreg32(kGrid) || c >= sreg32(kGrid))
                            continue;
                        const reg32 neighbour = cell(reg32(r), reg32(c));
                        // How far it is from the middle, squared: how much it pulls.
                        const float offset = float(fp::add(fld(guest, neighbour), fld(guest, kMinus128)));
                        const float size = jumpAboveOrEqual(fp::compare(Float(0.0), Float(offset))) ? float(-Float(offset)) : offset;
                        const Float share = fp::mul(Float(size), doubleAt(guest, kPerStep));
                        // The game's factor here is always one: its 0.5 at 0x47b08a is not reached.
                        const Float towards = fp::mul(fp::sub(fld(guest, neighbour), Float(floatOf(own))), Float(1.0f));
                        const Float pull = fp::mul(fp::mul(share, share), towards);
                        weight = float(fp::add(Float(1.0), Float(weight)));
                        sum = float(fp::add(pull, Float(sum)));
                    }
                }
                const float mean = float(fp::div(Float(sum), Float(weight)));
                if ((bitsOf(mean) & kNoSign) == 0)
                    continue;
                const float brightest = float(Float(sreg32(guest.dword(kCloudBrightest))));
                const float moved = float(fp::add(fp::mul(Float(mean), fld(guest, kHalf)), fld(guest, cell(row, column))));
                const float high = jumpAboveOrEqual(fp::compare(Float(brightest), Float(moved))) ? moved : brightest;
                reg32 kept = 0;
                if (jumpBelowOrEqual(fp::compare(Float(0.0), Float(high))))
                {
                    const Float again = Float(sreg32(guest.dword(kCloudBrightest)));
                    kept = jumpAboveOrEqual(fp::compare(again, Float(moved))) ? bitsOf(moved) : bitsOf(float(again));
                }
                guest.setDword(cell(row, column), kept);
            }
        }
    }

    reg32 left = 0;
    for (reg32 column = 0; column < kGrid; ++column)
    {
        for (reg32 row = 0; row < kGrid; ++row)
        {
            const reg32 point = kCloudPoints + row * kGridRowBytes + column * kPointBytes;
            const reg32 corner = kCloudCorners + row * kGridRowBytes + column * kPointBytes;
            const Float value = fld(guest, cell(row, column));
            const Float share = fp::mul(value, fld(guest, kOfByte));
            const Float depth = fp::mul(fp::mul(fld(guest, kSkyCurveScale), doubleAt(guest, kHalfDouble)),
                                        Float(sreg32(guest.dword(kCloudDepth))));
            const Float drop = fp::mul(share, depth);
            const std::int32_t whole = x87.truncated(value);
            const reg32 level = reg32(whole) & 0xff;
            const std::uint16_t flags = guest.word(kDriverFlags);
            guest.setFloat(point + 4, float(fp::sub(fld(guest, point + 4), drop)));
            reg32 alpha = level;
            if ((flags & kDriversAlphaFromNoise) != 0)
                alpha = guest.dword(kCloudBrightest) - reg32(sreg32(guest.dword(kCloudNoise)) >> 1);
            guest.setDword(corner + 8, (alpha << 24) | (level << 16) | (level << 8) | level);
            left = level;
        }
    }
    return left;
}

void rendCloudGrid(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47ae10", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::cloudGrid(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The sky's flashes
 * ------------------------------------------------------------------------- */

/* sub_47b350: as a race starts: the flashes' clock at the race's, none lit, no drift. */
x86::reg32 renda::flashesReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest.setDword(kFlashTick, guest.dword(kTicks));
    const reg32 result = guest::direct<0x494360, frontend::rendc::boltsClear>(app, cpu);
    guest.setDword(kFlashOn, 0);
    guest.setDword(kDriftX, 0);
    guest.setDword(kDriftZ, 0);
    return result;
}

void rendFlashesReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47b350", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::flashesReset(app, cpu); });
}

/* sub_47b390: the same, for another caller. */
x86::reg32 renda::flashesResetAgain(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest::direct<0x47b350, flashesReset>(app, cpu);
}

void rendFlashesResetAgain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47b390", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::flashesResetAgain(app, cpu); });
}

/* sub_47b3a0: the flash on the grid at 0x6fdc50: the three by three about its cell, each
 * as bright as its row's distance leaves it, if that is brighter than it is. */
x86::reg32 renda::flashOnGrid(App* app, CPU& cpu)
{
    constexpr reg32 kVertices = 0x6fdc50, kRowBytes = 0x1a0;
    constexpr reg32 kFalloff = 0x53b3f8;
    constexpr reg32 kDomeKind = 0x55d1f4;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x30);
    X87 x87(guest);
    sreg32 distance = -1;
    sreg32 row = sreg32(guest.dword(kFlashCellX) - 1);
    for (;;)
    {
        const reg32 rowEnd = guest.dword(kFlashCellX) + 2;
        if (row >= sreg32(rowEnd))
            return rowEnd;
        for (sreg32 column = sreg32(guest.dword(kFlashCellZ) - 1); column < sreg32(guest.dword(kFlashCellZ) + 2); ++column)
        {
            if (row < 0 || row >= sreg32(kGrid) || column < 0 || column >= sreg32(kGrid))
                continue;
            const reg32 vertex = kVertices + reg32(row) * kRowBytes + reg32(column) * kVertexBytes;
            const std::uint8_t now = guest.byte(vertex + kVertexColour);
            const reg32 away = distance > 0 ? reg32(distance) : reg32(0) - reg32(distance);
            const Float falloff = fp::mul(Float(sreg32(away)), doubleAt(guest, kFalloff));
            const Float kept = fp::sub(Float(1.0), falloff);
            const Float brightness = Float(sreg32(guest.byte(kFlashBrightness)));
            const std::uint8_t lit = std::uint8_t(x87.truncated(fp::mul(kept, brightness)));
            if (lit <= now)
                continue;
            const reg32 level = lit;
            const reg32 topByte = guest.dword(kDomeKind) == 2 ? guest.dword(vertex + kVertexColour) & kOpaque : level << 24;
            guest.setDword(vertex + kVertexColour, topByte | (level << 16) | (level << 8) | level);
        }
        ++distance;
        ++row;
    }
}

void rendFlashOnGrid(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47b3a0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::flashOnGrid(app, cpu); });
}

/* sub_47b4f0: edx points at ebx as vertices at the first of the stack's arguments, by the
 * view eax's camera matrix and its height, their colours and textures' corners from ecx,
 * moved by the stack's other two. */
x86::reg32 renda::domeVertices(App* app, CPU& cpu, reg32 view, reg32 count, reg32 points, reg32 corners, reg32 vertex,
                               reg32 moveUBits, reg32 moveVBits)
{
    constexpr reg32 kFrame = 0x20;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kFrame);
    const reg32 matrix = view + kViewCameraMatrix;
    const reg32 shift = guest.frame();
    guest.setDword(shift, 0);
    guest.setDword(shift + 8, 0);
    guest.setFloat(shift + 4, float(-fld(guest, view + kViewHeight)));
    putVector(guest, shift, rotated(guest, shift, matrix));
    const Float moveU = Float(floatOf(moveUBits)), moveV = Float(floatOf(moveVBits));
    // By address: the eax it leaves is this function's answer for no points, its core hands back none, and
    // the ebx it leaves this leaves too.
    reg32 result = guest.call(kObjectVertices, args({ count, points, matrix, shift }, { vertex }));
    while (--count != 0xffffffff)
    {
        guest.setDword(vertex + kVertexColour, guest.dword(corners + 8));
        guest.setFloat(vertex + kVertexU, float(fp::add(fld(guest, corners), moveU)));
        guest.setFloat(vertex + kVertexV, float(fp::add(fld(guest, corners + 4), moveV)));
        corners += kPointBytes;
        points += kPointBytes;
        guest.leaveEcx(vertex);
        guest.leaveEdx(points);
        vertex += kVertexBytes;
        result = vertex;
    }
    return result;
}

void rendDomeVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47b4f0", 0xc, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax;
    const reg32 count = cpu.edx;
    const reg32 points = cpu.ebx;
    const reg32 corners = cpu.ecx;
    const reg32 vertex = stackWord(app, cpu, 0);
    const reg32 moveU = stackWord(app, cpu, 1);
    const reg32 moveV = stackWord(app, cpu, 2);
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::domeVertices(app, cpu, view, count, points, corners, vertex, moveU, moveV); });
}

/* sub_47b5c0: the sky's weather up to the race's tick: every so many ticks a chance of a
 * flash somewhere round the camera (and its thunder); the newest flash's brightness and cell
 * on the grid; the clouds' drift, wrapped at one. */
x86::reg32 renda::skyWeather(App* app, CPU& cpu)
{
    constexpr reg32 kFlashChance = 0x55d260, kFlashEvery = 0x55d264;
    constexpr reg32 kDriftXStep = 0x55d20c, kDriftZStep = 0x55d210;
    constexpr reg32 kHalfWay = 0x53b400, kPerCell = 0x53b408, kRoundUp = 0x53b410;
    constexpr reg32 kSpread = 1500, kHalfSpread = 0x2ee;
    constexpr sreg32 kLastCell = 12;
    constexpr reg32 kEbp = 0x48;
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x60);
    X87 x87(guest);
    const reg32 ebp = guest.frame() + kEbp;
    const reg32 place = ebp - 0x3c, unit = ebp - 0x30, flash = ebp - 0x48, brightness = ebp - 0x24;
    for (;;)
    {
        const reg32 tick = guest.dword(kFlashTick);
        if (sreg32(tick) >= sreg32(guest.dword(kTicks)))
            return tick;
        const reg32 every = guest.dword(kFlashEvery);
        if (guest.dword(kFlashChance) != 0 && every != 0)
        {
            if (sreg32(tick) % sreg32(every) == 0 && guest.call(kRandom) % 100 < guest.dword(kFlashChance))
            {
                const reg32 x = guest.call(kRandom) % kSpread - kHalfSpread;
                const reg32 z = guest.call(kRandom) % kSpread - kHalfSpread;
                guest.setFloat(place, float(Float(sreg32(x))));
                guest.setDword(place + 4, 0);
                guest.setFloat(place + 8, float(Float(sreg32(z))));
                fe::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, place, unit);
                const float angle = float(fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(
                    app, cpu, guest.dword(unit + 8), guest.dword(unit)));
                guest::direct<0x47adc0, pointHeight>(app, cpu, place);
                guest::direct<0x4945a0, frontend::rendc::boltStrike>(app, cpu, guest.dword(place), guest.dword(place + 4),
                                                                     guest.dword(place + 8), bitsOf(angle));
                guest.call(kThunder, args({ x << 16, z << 16 }));
            }
            const reg32 lit = guest::direct<0x494390, frontend::rendc::boltGlow>(app, cpu, flash, brightness);
            guest.setDword(kFlashOn, lit);
            if (lit != 0)
            {
                const Float halfWay = doubleAt(guest, kHalfWay), perCell = doubleAt(guest, kPerCell);
                const Float cellX = fp::mul(fp::add(fld(guest, flash), halfWay), perCell);
                const Float cellZ = fp::mul(perCell, fp::add(halfWay, fld(guest, flash + 8)));
                guest.setByte(kFlashBrightness, guest.byte(brightness));
                const Float roundUp = doubleAt(guest, kRoundUp);
                const float atX = float(fp::add(cellX, roundUp));
                const float atZ = float(fp::add(roundUp, cellZ));
                const auto inGrid = [&](float at) {
                    const sreg32 cell = x87.truncated(Float(at));
                    if (cell >= kLastCell)
                        return reg32(kLastCell);
                    return reg32(cell > 0 ? cell : 0);
                };
                guest.setDword(kFlashCellX, inGrid(atX));
                guest.setDword(kFlashCellZ, inGrid(atZ));
            }
        }
        const float driftX = float(fp::add(fld(guest, kDriftXStep), fld(guest, kDriftX)));
        const float driftZ = float(fp::add(fld(guest, kDriftZStep), fld(guest, kDriftZ)));
        guest.setFloat(kDriftX, driftX);
        guest.setFloat(kDriftZ, driftZ);
        if (sreg32(guest.dword(kDriftX)) > sreg32(kFloatOne))
            guest.setDword(kDriftX, 0);
        if (sreg32(guest.dword(kDriftZ)) > sreg32(kFloatOne))
            guest.setDword(kDriftZ, 0);
        guest.setDword(kFlashTick, tick + 1);
    }
}

void rendSkyWeather(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47b5c0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return renda::skyWeather(app, cpu); });
}

}

}
