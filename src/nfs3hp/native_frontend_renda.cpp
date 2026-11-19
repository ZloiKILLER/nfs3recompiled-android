#include "native_frontend_renda.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_watcom.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_graphics.h"
#include "native_vertices.h"
#include "port_text.h"
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

// The game's functions, by address.
constexpr u32 kRandom = 0x4e0d88;
constexpr u32 kObjectVertices = 0x4bf4c0;    // eax how many, edx them, ebx the matrix, ecx the shift; the stack where
constexpr u32 kThunder = 0x414e10;           // eax and edx its x and z, 16.16

// The render states.
constexpr u32 kStateRaceFogColour = 3;
constexpr u32 kStateFogDensity = 0xe;
constexpr u32 kStateFogColour = 0xf;

// Their settings.
constexpr u32 kDepthBias = 0x7a3a64;         // the driver's; voodoo2a's is 4
constexpr u32 kFogAmountKept = 0x678b40;
constexpr u32 kFogColourKept = 0x678b44;
constexpr u32 kDriverOnCard = 0x7a3a60;
constexpr u32 kDriverFlags = 0x7a3a58;       // word; voodoo2a's is 0x10
constexpr std::uint16_t kDriversAlphaFromNoise = 0x1e0;

// The race's clock and view.
constexpr u32 kTicks = 0x7d3684;
constexpr u32 kViewMatrix = 0x7a3d58;
constexpr u32 kViewShift = 0x7a3d4c;
constexpr u32 kScreenScaleX = 0x56009c, kScreenScaleY = 0x5600a0;
constexpr u32 kScreenCentreX = 0x5600a4, kScreenCentreY = 0x5600a8;

// The screen's bounds, as a float's bits are compared.
constexpr u32 kClipDepth = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504,
                kClipRight = 0x7d350c;

// A view: its height over the track, and its matrices.
constexpr u32 kViewHeight = 0xc;
constexpr u32 kViewSkyMatrix = 0x14;
constexpr u32 kViewCameraMatrix = 0x44;

// A polygon's record (sub_4bbf80), with its four vertices after it.
constexpr u32 kPolygonBytes = 0xa0;
constexpr u32 kPolygonA = 4, kPolygonFlags = 6;
constexpr u32 kPolygonVertices = 8;          // four pointers
constexpr u32 kPolygonTexture = 0x18;
constexpr u32 kPolygonVertex0 = 0x20;
constexpr u32 kVertexBytes = 0x20;
constexpr u32 kVertexDwords = kVertexBytes / 4;
constexpr u32 kVertexW = 0xc, kVertexColour = 0x10, kVertexClip = 0x14, kVertexU = 0x18, kVertexV = 0x1c;
constexpr u32 kPolygonNoDepth = 4;           // the quad's flag not set
constexpr u32 kOpaque = 0xff000000;

constexpr u32 kFloatOne = 0x3f800000;
constexpr u32 kFloatTiny = 0x37800080;       // what stands in for a depth of zero
constexpr u32 kNoSign = 0x7fffffff;
constexpr u32 kSign = 0x80000000;

constexpr std::uint16_t kCompareBelow = 0x100;  // fcom's c0, what sahf makes cf

Float fld(const Guest& guest, u32 address)
{
    return Float(guest.floatAt(address));
}

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

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

/* What the jumps after fcom, fnstsw and sahf read. */
bool jumpAbove(std::uint16_t status) { return status == 0; }
bool jumpBelowOrEqual(std::uint16_t status) { return status != 0; }
bool jumpAboveOrEqual(std::uint16_t status) { return (status & kCompareBelow) == 0; }

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A game function's stack argument, by its place, as its thunk reads it. */
u32 stackWord(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

struct Vector
{
    float x, y, z;
};

/* The vector at `vector` by the matrix at `matrix`, as all these functions inline it:
 * each row's three products added in the matrix's order, a float's precision each. */
Vector rotated(const Guest& guest, u32 vector, u32 matrix)
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

void putVector(Guest& guest, u32 at, const Vector& v)
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
    u32 code;
    u32 bound;
};

Clip clipOf(const Guest& guest, u32 vertex)
{
    const u32 w = guest.dword(vertex + kVertexW);
    const u32 depth = guest.dword(kClipDepth);
    if ((w & kSign) != 0 || s32(w) >= s32(depth))
        return { 0x10, depth };
    u32 code = 0;
    const u32 y = guest.dword(vertex + 4);
    if ((y & kSign) != 0 || s32(y) < s32(guest.dword(kClipTop)))
        code |= 8;
    else if (s32(y) > s32(guest.dword(kClipBottom)))
        code |= 4;
    const u32 x = guest.dword(vertex);
    const u32 left = guest.dword(kClipLeft);
    if ((x & kSign) != 0 || s32(x) < s32(left))
        code |= 1;
    else if (s32(x) > s32(guest.dword(kClipRight)))
        code |= 2;
    return { code, left };
}

/* A quad for sub_478cb0: its four vertices by value on the stack, past what
 * Guest::call's arguments hold, its four colours and its flag. */
struct Quad
{
    u32 vertices[4];
    u32 colours[4];
    u32 flag;
};

constexpr u32 kQuadStackBytes = 4 * kVertexBytes + 4 * 4 + 4;

u32 makeQuad(Guest& guest, nfs::ThreadState& ts, u32 polygon, const Quad& quad)
{
    const u32 esp = ts.sp;
    ts.sp -= kQuadStackBytes;
    for (u32 i = 0; i < 4; ++i)
        copyDwords(guest, ts.sp + i * kVertexBytes, quad.vertices[i], kVertexDwords);
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(ts.sp + 4 * kVertexBytes + 4 * i, quad.colours[i]);
    guest.setDword(ts.sp + 4 * kVertexBytes + 4 * 4, quad.flag);
    // The vertices, the colours and the flag on the stack below, where the record's core reads them.
    const u32 result = guest::direct<0x478cb0, frontend::renda::quadRecord>(guest.app(), ts, polygon);
    ts.sp = esp;
    return result;
}

/* What is left of a hundred, kept to 1..100; the density's state takes five times it. */
u32 fogShare(u32 amount)
{
    const s32 share = s32(u32(100) - amount);
    if (share < 1)
        return 1;
    return share > 100 ? 100 : u32(share);
}

/* One vector added to another, taken from it, or scaled (the game's 0x4dffb0, 0x4e0000 and 0x4e0050,
 * vertices' typed cores). */
void addOne(Guest& guest, nfs::ThreadState& ts, u32 a, u32 b, u32 out)
{
    fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(guest.app(), ts, 1, a, b, out);
}

void subtractOne(Guest& guest, nfs::ThreadState& ts, u32 a, u32 b, u32 out)
{
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(guest.app(), ts, 1, a, b, out);
}

void scaleOne(Guest& guest, nfs::ThreadState& ts, u32 in, u32 factorBits, u32 out)
{
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(guest.app(), ts, 1, in, floatOf(factorBits), out);
}

/* The record the sky's list starts or ends with: 16 bytes, kind 5, none next, 4, and
 * what it hands the driver. */
constexpr u32 kListMarkBytes = 0x10;

void fillListMark(Guest& guest, u32 record, u32 last)
{
    constexpr u32 kMarkKind = 4, kMarkFour = 8, kMarkLast = 0xc;
    guest.setWord(record + kMarkKind, 5);
    guest.setDword(record, 0);
    guest.setDword(record + kMarkFour, 4);
    guest.setDword(record + kMarkLast, last);
}

constexpr u32 kPointBytes = 0xc;
constexpr u32 kRingPointCount = 9;
constexpr u32 kRingSegments = 8;

constexpr u32 kSkyWind = 0x55d2cc, kSkyCentre = 0x55d2b4, kSkyUp = 0x55d2c0, kSkyRadius = 0x55d1c8, kSkyTurn = 0x55d1cc;
constexpr u32 kSkyBottom = 0x55d164, kSkyMiddle = 0x55d168, kSkySpread = 0x55d16c, kSkyTop = 0x55d170, kSkyTopSpread = 0x55d174;
constexpr u32 kSkyGradient = 0x55d178, kSkyColoursLow = 0x55d17c, kSkyColoursHigh = 0x55d180;
constexpr u32 kSkyTwoSided = 0x55d15c;

/* The segment of a ring, the turn's sixteenths on by `i`. */
u32 ringSegment(u32 start, u32 i)
{
    const u32 segment = start + i;
    return s32(segment) >= 0x10 ? segment - 0x10 : segment;
}

constexpr u32 kGrid = 13;
constexpr u32 kGridRowBytes = 0x9c;
constexpr u32 kCloudPoints = 0x6ff9fc;       // 13 by 13 of x, y, z
constexpr u32 kCloudCorners = 0x6ff170;      // 13 by 13 of u, v, colour
constexpr u32 kCloudBrightest = 0x55d1f8, kCloudDepth = 0x55d1fc, kCloudNoise = 0x55d200;
constexpr u32 kSkyCurveScale = 0x55d24c, kSkyCurveShift = 0x55d248;

constexpr u32 kFlashTick = 0x55d154, kFlashOn = 0x55d250, kDriftX = 0x55d214, kDriftZ = 0x55d218;
constexpr u32 kFlashBrightness = 0x55d25c, kFlashCellX = 0x55d254, kFlashCellZ = 0x55d258;

}

namespace frontend
{

/* ---------------------------------------------------------------------------
 * The fog's states
 * ------------------------------------------------------------------------- */

/* sub_470a60: the fog's amount, as sub_470ad0 was last given it. */
nfs::u32 renda::fogAmount(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kFogAmountKept);
}

nfs::u32 rendFogAmount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470a60", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogAmount(app, ts); });
}

/* sub_470aa0: state 0xf, eax, kept, on a driver with a depth bias. */
nfs::u32 renda::fogColour(App* app, ThreadState& ts, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (guest.dword(kDepthBias) == 0)
        return colour;
    guest.setDword(kFogColourKept, colour);
    return *fe::direct<0x431900, graphics::stateAnswer>(app, ts, kStateFogColour, colour);
}

nfs::u32 rendFogColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470aa0", 0, 0, nullptr, 0 };
    const u32 colour = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogColour(app, ts, colour); });
}

/* sub_470ad0: the fog's amount, eax, kept, on a driver with a depth bias: state 0xe,
 * five times what is left of a hundred, none for none. */
nfs::u32 renda::fogDensity(App* app, ThreadState& ts, u32 amount)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (guest.dword(kDepthBias) == 0)
        return amount;
    guest.setDword(kFogAmountKept, amount);
    const u32 share = amount != 0 ? fogShare(amount) : 0;
    return *fe::direct<0x431900, graphics::stateAnswer>(app, ts, kStateFogDensity, share * 5);
}

nfs::u32 rendFogDensity(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470ad0", 0, 0, nullptr, 0 };
    const u32 amount = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogDensity(app, ts, amount); });
}

/* sub_470b20: state 0xe likewise, times edx, the amount not kept. */
nfs::u32 renda::fogDensityTimes(App* app, ThreadState& ts, u32 amount, u32 times)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (guest.dword(kDepthBias) == 0)
        return amount;
    return *fe::direct<0x431900, graphics::stateAnswer>(app, ts, kStateFogDensity, fogShare(amount) * 5 * times);
}

nfs::u32 rendFogDensityTimes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470b20", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 amount = args.eax;
    const u32 times = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogDensityTimes(app, ts, amount, times); });
}

/* sub_470b70: both: eax the colour, edx the amount. */
nfs::u32 renda::fogBoth(App* app, ThreadState& ts, u32 colour, u32 amount)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x470ad0, fogDensity>(app, ts, amount);
    return guest::direct<0x470aa0, fogColour>(app, ts, colour);
}

nfs::u32 rendFogBoth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470b70", 0, 0, nullptr, 0 };
    const u32 colour = args.eax;
    const u32 amount = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogBoth(app, ts, colour, amount); });
}

/* sub_470b90: as a race starts: the amount unknown, the colour none, state 0xe 0. */
nfs::u32 renda::fogReset(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0xc);
    guest.setDword(kFogAmountKept, 0xffffffff);
    guest.setDword(kFogColourKept, 0);
    return *fe::direct<0x431900, graphics::stateAnswer>(app, ts, kStateFogDensity, 0);
}

nfs::u32 rendFogReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_470b90", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::fogReset(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The race's font
 * ------------------------------------------------------------------------- */

/* sub_476fb0: how wide the string eax is drawn in the font at 0x679010, at the scale at
 * 0x679014: each character its width from the font's table, or 20 past its range, the
 * sum cut to an integer as it goes. */
nfs::u32 renda::stringWidth(App* app, ThreadState& ts, u32 text)
{
    constexpr u32 kFont = 0x679010, kScale = 0x679014, kWidthOfNone = 0x53b0a0;
    constexpr u32 kFontFirst = 0, kFontCount = 1, kFontWidths = 0x14;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x18);
    FloatMath math(guest);
    std::int32_t width = 0;
    for (u32 at = text; guest.byte(at) != 0; ++at)
    {
        const u32 font = guest.dword(kFont);
        const u32 character = guest.byte(at);
        const u32 first = guest.byte(font + kFontFirst);
        const bool has = character >= first && s32(character) < s32(first + guest.byte(font + kFontCount));
        const u32 widthAt = has ? (character << 5) + font + kFontWidths : kWidthOfNone;
        const Float scaled = fp::mul(fld(guest, kScale), fld(guest, widthAt));
        width = math.truncated(fp::add(scaled, FloatMath::integer(width)));
    }
    return u32(width);
}

nfs::u32 rendStringWidth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_476fb0", 0, 0, nullptr, 0 };
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::stringWidth(app, ts, text); });
}

/* ---------------------------------------------------------------------------
 * The sparks, on the screen
 * ------------------------------------------------------------------------- */

/* sub_4774b0: the current spark set's eight sparks ([0x55b088], if it is of kind 4): each
 * of kind 2 and alive that has run out goes, the others' places turned by the view
 * and put on the screen. */
nfs::u32 renda::sparksOnScreen(App* app, ThreadState& ts)
{
    constexpr u32 kCurrentSet = 0x55b088, kSets = 0x55b08c, kSetBytes = 0x238;
    constexpr u32 kSetActive = 4, kSetAlive = 0x230;
    constexpr u32 kSparkKind = 0x30, kSparkAlive = 0x3c, kSparkEnds = 0x40, kSparkWatched = 0x48;
    constexpr u32 kSparkHidden = 0x54, kSparkX = 0x58, kSparkY = 0x5c, kSparkZ = 0x60, kSparkPlace = 0x64;
    constexpr u32 kToScreenX = 0x53b0ac, kToScreenY = 0x53b0b0, kToScreenZ = 0x53b0b4;
    constexpr u32 kFarthest = 0x42c80000;  // a hundred, as bits
    Guest guest(app, ts);
    const u32 current = guest.dword(kCurrentSet);
    const u32 offset = current * kSetBytes;
    u32 left = offset;  // what the game's eax holds at each way out
    const u32 set = kSets + offset;
    if (current == 0xffffffff || guest.dword(set + kSetActive) == 0 || guest.dword(set) != 4)
        return left;
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 bytes = i << 6;
        const u32 spark = set + bytes;
        left = spark;
        if (guest.dword(spark + kSparkKind) != 2 || guest.dword(spark + kSparkAlive) == 0)
            continue;
        bool over = false;
        const u32 watched = guest.dword(spark + kSparkWatched);
        if (watched != 0)
            over = guest.dword(watched) == 0;
        else
            over = s32(guest.dword(kTicks)) > s32(guest.dword(spark + kSparkEnds));
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
        const u32 zBits = bitsOf(z);
        if (jumpAbove(fp::compare(Float(0.0), Float(z))) || s32(zBits) > s32(kFarthest))
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

nfs::u32 rendSparksOnScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4774b0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::sparksOnScreen(app, ts); });
}

/* ---------------------------------------------------------------------------
 * A quad, the view's rectangle and the sky's list
 * ------------------------------------------------------------------------- */

/* sub_478cb0: the polygon's record eax as a quad: four vertices by value and their colours,
 * opaque, on the stack, the texture's corners in order, no texture, and a flag: 0 no depth. */
nfs::u32 renda::quadRecord(App* app, ThreadState& ts, u32 polygon)
{
    constexpr u32 kFrame = 0x20;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kFrame);
    // The four vertices, their colours and the flag where the caller put them, above the return address.
    const u32 stack = guest.frame() + kFrame + 4;
    guest.setWord(polygon + kPolygonA, 0);
    guest.setWord(polygon + kPolygonFlags, 0);
    guest.setDword(polygon + kPolygonTexture, 0);
    const u32 first = polygon + kPolygonVertex0;
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(polygon + kPolygonVertices + 4 * i, first + i * kVertexBytes);
    if (guest.dword(stack + 4 * kVertexBytes + 4 * 4) == 0)
        guest.setByte(polygon + kPolygonFlags, std::uint8_t(guest.byte(polygon + kPolygonFlags) | kPolygonNoDepth));
    for (u32 i = 0; i < 4; ++i)
        copyDwords(guest, first + i * kVertexBytes, stack + i * kVertexBytes, kVertexDwords);
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(first + i * kVertexBytes + kVertexColour, guest.dword(stack + 4 * kVertexBytes + 4 * i) | kOpaque);
    static const u32 kU[4] = { 0, kFloatOne, kFloatOne, 0 };
    static const u32 kV[4] = { 0, 0, kFloatOne, kFloatOne };
    for (u32 i = 0; i < 4; ++i)
    {
        guest.setDword(first + i * kVertexBytes + kVertexU, kU[i]);
        guest.setDword(first + i * kVertexBytes + kVertexV, kV[i]);
    }
    return first;
}

nfs::u32 rendMakeQuad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_478cb0", kQuadStackBytes, 0, nullptr, 0 };
    const u32 polygon = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::quadRecord(app, ts, polygon); });
}

/* sub_478dc0: the view's rectangle at 0x6fdc40, its right and bottom for its sizes. */
nfs::u32 renda::viewRectangle(App* app, ThreadState& ts)
{
    constexpr u32 kRight = 0x6fdc40, kLeft = 0x6fdc44, kTop = 0x6fdc48, kBottom = 0x6fdc4c;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x10);
    fe::direct<0x4beb80, port::text::viewportOf>(guest.app(), guest.ts(), kLeft, kTop, kRight, kBottom);
    const u32 left = guest.dword(kLeft);
    const u32 right = guest.dword(kRight) + left;
    const u32 bottom = guest.dword(kBottom) + guest.dword(kTop);
    guest.setDword(kRight, right);
    guest.setDword(kBottom, bottom);
    return left;
}

nfs::u32 rendViewRectangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_478dc0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::viewRectangle(app, ts); });
}

/* sub_479080: the polygons' list started, with a mark first. */
nfs::u32 renda::skyListStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0xc);
    fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, ts);
    const u32 slot = guest.frame();
    const u32 added = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kListMarkBytes);
    if (added == 0)
        return added;
    const u32 record = guest.dword(slot);
    fillListMark(guest, record, 0);
    return record;
}

nfs::u32 rendSkyListStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479080", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyListStart(app, ts); });
}

/* sub_4790d0: the list ended and kept for the pass eax, a new one started. */
nfs::u32 renda::skyListKeep(App* app, ThreadState& ts, u32 pass)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x18);
    const u32 ends = guest.frame();
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, ts, ends);
    const u32 drawn = fe::direct<0x4bc030, frontend::rendd::drawListPlain>(app, ts, pass, ends);
    fe::direct<0x4bbf20, frontend::rendd::openListStart>(app, ts);
    // eax as the code leaves it: the list's start leaves the eax it came with, the drawing's.
    return drawn;
}

nfs::u32 rendSkyListKeep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4790d0", 0, 0, nullptr, 0 };
    const u32 pass = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyListKeep(app, ts, pass); });
}

/* sub_479100: the list ended with a mark that hands back the driver's [0x7a3a60], its
 * two ends to esi. */
nfs::u32 renda::skyListEnd(App* app, ThreadState& ts, u32 out)
{
    constexpr u32 kFrame = 0x1c;
    constexpr u32 kSlot = 8;  // [ebp - 4]
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kFrame);
    const u32 ends = guest.frame();
    const u32 slot = ends + kSlot;
    if (fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kListMarkBytes) != 0)
        fillListMark(guest, guest.dword(slot), guest.dword(kDriverOnCard));
    fe::direct<0x4bbfc0, frontend::rendd::openListClose>(app, ts, ends);
    copyDwords(guest, out, ends, 2);
    return out;
}

nfs::u32 rendSkyListEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479100", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyListEnd(app, ts, out); });
}

/* ---------------------------------------------------------------------------
 * The sky's rings
 * ------------------------------------------------------------------------- */

/* sub_479160: a band's three points, to the third stack argument +0, +0xc and +0x18, from
 * the view eax: the wind's direction by the view's matrix, flat and made a unit, its angle
 * a turn to the fourth; the band's side, across it and up; each at its length and raised.
 * The stack: the rise, the upper's length, the points, the turn. */
nfs::u32 renda::bandPoints(App* app, ThreadState& ts, u32 view, u32 riseBits, u32 upperLength, u32 points, u32 turn)
{
    constexpr u32 kFrame = 0x38;
    constexpr u32 kPhase = 0x53b2c8, kTwoPi = 0x53b2cc, kSixteenth = 0x53b2d0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kFrame);
    const u32 towardsCentre = guest.frame() + 0xc, towardsUp = guest.frame();
    const u32 side = points + 0x18, upper = points + 0xc;
    putVector(guest, points, rotated(guest, kSkyWind, view + kViewSkyMatrix));
    guest.setDword(points + 4, 0);
    fe::direct<0x4e01c0, vertices::makeUnitVector>(app, ts, points, points);
    const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, ts, guest.dword(points),
                                                                                  guest.dword(points + 8));
    const Float angle = fp::add(fp::add(heading, fld(guest, kPhase)), fld(guest, kSkyTurn));
    guest.setFloat(turn, float(angle));
    guest::direct<0x4964d0, frontend::rendc::wrapFraction>(app, ts, turn);
    subtractOne(guest, ts, kSkyCentre, points, towardsCentre);
    subtractOne(guest, ts, kSkyUp, points, towardsUp);
    fe::direct<0x4e01a0, frontend::simd::unitCross>(app, ts, towardsCentre, towardsUp, side);
    const float sideLength = float(fp::mul(fp::mul(fld(guest, kSkyRadius), fld(guest, kTwoPi)), fld(guest, kSixteenth)));
    scaleOne(guest, ts, points, guest.dword(kSkyRadius), points);
    scaleOne(guest, ts, kSkyUp, upperLength, upper);
    scaleOne(guest, ts, side, bitsOf(sideLength), side);
    addOne(guest, ts, upper, points, upper);
    addOne(guest, ts, side, points, side);
    const Float a = fld(guest, points + 4), b = fld(guest, points + 0x10), c = fld(guest, points + 0x1c);
    const Float rise = Float(floatOf(riseBits));
    guest.setFloat(points + 4, float(fp::add(a, rise)));
    guest.setFloat(points + 0x10, float(fp::add(b, rise)));
    guest.setFloat(points + 0x1c, float(fp::add(rise, c)));
    return u32(1);
}

nfs::u32 rendBandPoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479160", 0x10, 0, nullptr, 0 };
    const u32 view = args.eax;
    const u32 rise = stackWord(app, ts, 0);
    const u32 upperLength = stackWord(app, ts, 1);
    const u32 points = stackWord(app, ts, 2);
    const u32 turn = stackWord(app, ts, 3);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::bandPoints(app, ts, view, rise, upperLength, points, turn); });
}

/* sub_4792e0: eax points to ebx, on from the two at edx and as far apart. */
nfs::u32 renda::curvePoints(App* app, ThreadState& ts, u32 howMany, u32 in, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x28);
    const s32 count = s32(howMany);
    const u32 step = guest.frame();
    // By address: what the vectors' functions leave in ebx and edx is what this one leaves.
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, in + kPointBytes, in, step);
    copyDwords(guest, out, in, 3);
    copyDwords(guest, out + kPointBytes, in + kPointBytes, 3);
    u32 left = in;
    for (s32 i = 2; i < count; ++i)
        left = fe::direct<0x4dffb0, vertices::offsetVectorsEax<true>>(app, ts, 1, out + u32(i - 1) * kPointBytes, step, out + u32(i) * kPointBytes);
    return left;
}

nfs::u32 rendCurvePoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4792e0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 count = args.eax;
    const u32 in = args.edx;
    const u32 out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::curvePoints(app, ts, count, in, out); });
}

/* sub_479370: eax points at ebx as vertices at ecx, by the view edx's camera matrix,
 * their clip codes to the first of the stack's arguments, the colour the second. */
nfs::u32 renda::bandVertices(App* app, ThreadState& ts, u32 count, u32 view, u32 points, u32 vertices, u32 codes, u32 colour)
{
    Guest guest(app, ts);
    const u32 matrix = view + kViewCameraMatrix;
    u32 vertex = vertices, result = count;
    while (--count != 0xffffffff)
    {
        const Vector turned = rotated(guest, points, matrix);
        u32 zBits = bitsOf(turned.z);
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
        vertex += kVertexBytes;
        result = vertex;
        points += kPointBytes;
        codes += 4;
    }
    return result;
}

nfs::u32 rendBandVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479370", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 count = args.eax;
    const u32 view = args.edx;
    const u32 points = args.ebx;
    const u32 vertices = args.ecx;
    const u32 codes = stackWord(app, ts, 0);
    const u32 colour = stackWord(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::bandVertices(app, ts, count, view, points, vertices, codes, colour); });
}

/* sub_479500: a ring's two curves of nine points, from the view eax: the band's points
 * (sub_479160) taken back four of its steps and on by the fraction of a sixteenth the turn
 * stands at, the lower curve to the last argument, the upper to the one before.
 * The stack: its height, its upper's length, the turn, then the curves. */
nfs::u32 renda::ringPoints(App* app, ThreadState& ts, u32 view, u32 height, u32 upperLength, u32 turn, u32 upperCurve, u32 lowerCurve)
{
    constexpr u32 kFrame = 0x74;
    constexpr u32 kEbp = 0x5c;
    constexpr u32 kSixteen = 0x53b2d4;
    constexpr u32 kFourBits = 0x40800000;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kFrame);
    FloatMath math(guest);
    const u32 ebp = guest.frame() + kEbp;
    const u32 band = ebp - 0x5c, upper = ebp - 0x50, side = ebp - 0x44;
    const u32 curve = ebp - 0x38, across = ebp - 0x20, moved = ebp - 0x14;
    const float rise = float(fp::sub(Float(floatOf(height)), fld(guest, view + kViewHeight)));
    const u32 made = guest::direct<0x479160, bandPoints>(app, ts, view, bitsOf(rise), upperLength, band, turn);
    if (made == 0)
        return made;
    subtractOne(guest, ts, side, band, across);
    scaleOne(guest, ts, across, kFourBits, moved);
    subtractOne(guest, ts, band, moved, band);
    subtractOne(guest, ts, upper, moved, upper);
    subtractOne(guest, ts, side, moved, side);
    const Float sixteenths = fp::mul(fld(guest, turn), fld(guest, kSixteen));
    const float exact = float(sixteenths);
    const std::int32_t step = math.truncated(sixteenths);
    const float part = float(fp::sub(Float(exact), FloatMath::integer(step)));
    scaleOne(guest, ts, across, bitsOf(part), moved);
    subtractOne(guest, ts, band, moved, band);
    subtractOne(guest, ts, upper, moved, upper);
    subtractOne(guest, ts, side, moved, side);
    copyDwords(guest, curve, band, 3);
    copyDwords(guest, curve + kPointBytes, side, 3);
    guest::direct<0x4792e0, curvePoints>(app, ts, kRingPointCount, curve, lowerCurve);
    subtractOne(guest, ts, side, band, across);
    addOne(guest, ts, upper, across, curve + kPointBytes);
    copyDwords(guest, curve, upper, 3);
    guest::direct<0x4792e0, curvePoints>(app, ts, kRingPointCount, curve, upperCurve);
    return u32(1);
}

nfs::u32 rendRingPoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479500", 0x14, 0, nullptr, 0 };
    const u32 view = args.eax;
    const u32 height = stackWord(app, ts, 0);
    const u32 upperLength = stackWord(app, ts, 1);
    const u32 turn = stackWord(app, ts, 2);
    const u32 upperCurve = stackWord(app, ts, 3);
    const u32 lowerCurve = stackWord(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::ringPoints(app, ts, view, height, upperLength, turn, upperCurve, lowerCurve); });
}

/* sub_479680: the ring of the sky's gradient from the view eax, the gradient at the horizon
 * running to the two tables' colours, turned over for the far half of a two-sided sky. */
nfs::u32 renda::skyGradientRing(App* app, ThreadState& ts, u32 view)
{
    constexpr u32 kEbp = 0x2ea;
    constexpr u32 kSixteen = 0x53b2d8;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x384);
    FloatMath math(guest);
    const u32 ebp = guest.frame() + kEbp;
    const u32 turn = ebp + 0x76, slot = ebp + 0x7a;
    const u32 upperCurve = ebp - 0xaa, lowerCurve = ebp - 0x3e;
    const u32 lower = ebp - 0x1ca, upper = ebp - 0x2ea;
    const float length = float(fp::sub(fld(guest, kSkyMiddle), fld(guest, kSkySpread)));
    const float height = float(fp::add(fld(guest, kSkyBottom), fld(guest, kSkySpread)));
    if (guest::direct<0x479500, ringPoints>(app, ts, view, bitsOf(height), bitsOf(length), turn, upperCurve, lowerCurve) == 0)
        return u32(0);
    const std::int32_t start = math.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen)));
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, lowerCurve, lower, ebp + 0x52, 0xffffffff);
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, upperCurve, upper, ebp + 0x2e, 0xffffffff);
    u32 left = 0;  // what eax is left with: the last record made, or the list's 0
    for (u32 i = 0; i < kRingSegments; ++i)
    {
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const u32 segment = ringSegment(u32(start), i);
        const u32 at = i * kVertexBytes;
        const u32 low = guest.dword(segment * 4 + kSkyColoursLow), high = guest.dword(segment * 4 + kSkyColoursHigh);
        const u32 gradient = guest.dword(kSkyGradient);
        Quad quad;
        if (guest.dword(kSkyTwoSided) != 0 && s32(segment) > 7)
            quad = { { upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes, lower + at },
                     { gradient, gradient, high, low }, 0 };
        else
            quad = { { lower + at, upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes },
                     { low, gradient, gradient, high }, 0 };
        left = makeQuad(guest, ts, guest.dword(slot), quad);
    }
    return left;
}

nfs::u32 rendSkyGradientRing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479680", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyGradientRing(app, ts, view); });
}

/* sub_479860: the colour eax with the fog's share at 0x55d268 of the colour at 0x55d26c, on
 * a driver without a depth bias, which fogs none of its own; as it came on the others. */
nfs::u32 renda::foggedColour(App* app, ThreadState& ts, u32 colour)
{
    constexpr u32 kShare = 0x55d268, kFogColourAt = 0x55d26c, kHalf = 0x53b2e0;
    Guest guest(app, ts);
    if (guest.dword(kDepthBias) != 0)
        return colour;
    const u32 fog = guest.dword(kFogColourAt);
    const Float share = fld(guest, kShare);
    const float rest = float(fp::sub(Float(1.0), share));
    // A channel: its share of both, kept to a byte.
    const auto blend = [&](unsigned shift) {
        const Float own = fp::mul(Float(s32((colour >> shift) & 0xff)), Float(rest));
        const float fogged = float(fp::mul(Float(s32((fog >> shift) & 0xff)), share));
        const Float value = fp::mul(fp::add(own, Float(fogged)), doubleAt(guest, kHalf));
        const s32 whole = fp::toInteger<s32>(value);
        return u32(whole < 0 ? 0 : whole > 0xff ? 0xff : whole);
    };
    const u32 red = blend(16), green = blend(8), blue = blend(0);
    return kOpaque | (red << 16) | (green << 8) | blue;
}

nfs::u32 rendFoggedColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479860", 0, 0, nullptr, 0 };
    const u32 colour = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::foggedColour(app, ts, colour); });
}

/* sub_4799d0: the ring of the fog's haze from the view eax, in the two fogged colours at
 * 0x55d1c4 and 0x55d1c0. */
nfs::u32 renda::skyHazeRing(App* app, ThreadState& ts, u32 view)
{
    constexpr u32 kEbp = 0x2f2;
    constexpr u32 kSixteen = 0x53b2e8, kHazeLow = 0x55d1c4, kHazeHigh = 0x55d1c0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x38c);
    FloatMath math(guest);
    const u32 ebp = guest.frame() + kEbp;
    const u32 turn = ebp + 0x72, slot = ebp + 0x6e;
    const u32 upperCurve = ebp - 0xb2, lowerCurve = ebp - 0x46;
    const u32 lower = ebp - 0x1d2, upper = ebp - 0x2f2;
    if (guest::direct<0x479500, ringPoints>(app, ts, view, guest.dword(kSkyBottom), guest.dword(kSkySpread), turn,
                                            upperCurve, lowerCurve) == 0)
        return u32(0);
    const std::int32_t start = math.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen)));
    const u32 low = guest::direct<0x479860, foggedColour>(app, ts, guest.dword(kHazeLow));
    const u32 high = guest::direct<0x479860, foggedColour>(app, ts, guest.dword(kHazeHigh));
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, lowerCurve, lower, ebp + 0x4a, 0xffffffff);
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, upperCurve, upper, ebp + 0x26, 0xffffffff);
    u32 left = 0;  // what eax is left with: the last record made, or the list's 0
    for (u32 i = 0; i < kRingSegments; ++i)
    {
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const u32 segment = ringSegment(u32(start), i);
        const u32 at = i * kVertexBytes;
        Quad quad;
        if (guest.dword(kSkyTwoSided) != 0 && s32(segment) > 7)
            quad = { { upper + at + kVertexBytes, upper + at, lower + at, lower + at + kVertexBytes },
                     { high, high, low, low }, 1 };
        else
            quad = { { upper + at, upper + at + kVertexBytes, lower + at + kVertexBytes, lower + at },
                     { high, high, low, low }, 1 };
        left = makeQuad(guest, ts, guest.dword(slot), quad);
    }
    return left;
}

nfs::u32 rendSkyHazeRing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4799d0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyHazeRing(app, ts, view); });
}

/* sub_479b90: the ring of the clouds from the view eax: three rows of nine points, the
 * upper two moved by the wind's table at 0x7001e8, each cell a textured polygon unless all
 * four of its corners are off one side of the screen. */
nfs::u32 renda::skyCloudRing(App* app, ThreadState& ts, u32 view)
{
    constexpr u32 kEbp = 0x536;
    constexpr u32 kWhiteInTheCode = 0x478310;  // 0xffffffff, read from among the code
    constexpr u32 kSixteen = 0x53b2ec;
    constexpr u32 kWind = 0x7001e8, kCloudTextures = 0x6ff95c;
    constexpr u32 kHundredth = 0x3c23d70a;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x5d0);
    FloatMath math(guest);
    const u32 ebp = guest.frame() + kEbp;
    const u32 colour = guest.dword(kWhiteInTheCode);
    const u32 turn = ebp + 0x66, slot = ebp + 0x5e;
    const u32 bottom = ebp - 0xfe, top = ebp - 0x1d6, middle = ebp - 0x92, above = ebp - 0x16a;
    const u32 difference = ebp + 0x46, scaled = ebp + 0x52;
    const u32 topVertices = ebp - 0x2f6, middleVertices = ebp - 0x416, aboveVertices = ebp - 0x536;
    const u32 topCodes = ebp - 2, middleCodes = ebp + 0x22, aboveCodes = ebp - 0x26;
    const float length = float(fp::sub(fld(guest, kSkyTop), fld(guest, kSkyTopSpread)));
    const float height = float(fp::add(fld(guest, kSkyTopSpread), fld(guest, kSkyBottom)));
    if (guest::direct<0x479500, ringPoints>(app, ts, view, bitsOf(height), bitsOf(length), turn, bottom, top) == 0)
        return u32(0);
    const u32 start = u32(math.truncated(fp::mul(fld(guest, turn), fld(guest, kSixteen))));
    for (u32 i = 0; i < kRingSegments; ++i)
    {
        const u32 segment = ringSegment(start, i);
        const u32 at = i * kPointBytes;
        subtractOne(guest, ts, bottom + at, top + at, difference);
        scaleOne(guest, ts, difference, guest.dword(segment * 4 + kWind), scaled);
        addOne(guest, ts, top + at, scaled, middle + at);
        addOne(guest, ts, top + at + kPointBytes, scaled, above + at);
        subtractOne(guest, ts, middle + at, above + at, difference);
        scaleOne(guest, ts, difference, kHundredth, scaled);
        addOne(guest, ts, middle + at, scaled, middle + at);
    }
    const u32 fogged = guest::direct<0x479860, foggedColour>(app, ts, colour);
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, top, topVertices, topCodes, fogged);
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, middle, middleVertices, middleCodes, fogged);
    guest::direct<0x479370, bandVertices>(app, ts, kRingPointCount, view, above, aboveVertices, aboveCodes, fogged);
    u32 left = 0;
    for (u32 i = 0; i < kRingSegments; ++i)
    {
        const u32 firstCode = guest.dword(topCodes + 4 * i);
        left = firstCode;
        if ((guest.dword(topCodes + 4 * (i + 1)) & firstCode & guest.dword(middleCodes + 4 * i) & guest.dword(aboveCodes + 4 * i)) != 0)
            continue;
        left = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kPolygonBytes);
        if (left == 0)
            continue;
        const u32 polygon = guest.dword(slot);
        const u32 segment = ringSegment(start, i);
        left = polygon + 0x40;
        guest.setWord(polygon + kPolygonA, 0);
        guest.setWord(polygon + kPolygonFlags, 0);
        guest.setDword(polygon + kPolygonTexture, guest.dword(segment * 4 + kCloudTextures));
        const u32 first = polygon + kPolygonVertex0;
        for (u32 v = 0; v < 4; ++v)
            guest.setDword(polygon + kPolygonVertices + 4 * v, first + v * kVertexBytes);
        const u32 at = i * kVertexBytes;
        u32 from[4];
        if (guest.dword(kSkyTwoSided) != 0 && s32(segment) > 7)
        {
            from[0] = aboveVertices + at, from[1] = middleVertices + at;
            from[2] = topVertices + at, from[3] = topVertices + at + kVertexBytes;
        }
        else
        {
            from[0] = middleVertices + at, from[1] = aboveVertices + at;
            from[2] = topVertices + at + kVertexBytes, from[3] = topVertices + at;
        }
        for (u32 v = 0; v < 4; ++v)
            copyDwords(guest, first + v * kVertexBytes, from[v], kVertexDwords);
    }
    return left;
}

nfs::u32 rendSkyCloudRing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479b90", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyCloudRing(app, ts, view); });
}

/* ---------------------------------------------------------------------------
 * The sun
 * ------------------------------------------------------------------------- */

/* sub_479ed0: the sun's quad for the view eax, when the track has one ([0x55d27c]): the sun's
 * place on the screen, its corners round it by its angle -- towards its second point, or
 * from where it stands. */
nfs::u32 renda::sun(App* app, ThreadState& ts, u32 view)
{
    constexpr u32 kEbp = 0x5a;
    constexpr u32 kHasSun = 0x55d27c, kSunAt = 0x55d280, kSunTowards = 0x55d28c, kSunSize = 0x55d298, kSunAtCentre = 0x55d29c;
    constexpr u32 kSunFlags = 0x55d2a0, kSunTexture = 0x6ff9cc;
    constexpr u32 kEighth = 0x53b2f0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0xf4);
    if (guest.dword(kHasSun) == 0)
        return view;
    const u32 ebp = guest.frame() + kEbp;
    const u32 slot = ebp + 0x7a, points = ebp + 0x26, vertices = ebp - 0x5a;
    const u32 matrix = view + kViewCameraMatrix;
    const Vector at = rotated(guest, kSunAt, matrix);
    Float angle;
    if (guest.dword(kSunAtCentre) == 0)
    {
        const Vector towards = rotated(guest, kSunTowards, matrix);
        const float x = float(fp::sub(Float(towards.x), Float(at.x)));
        const float y = float(fp::sub(Float(towards.y), Float(at.y)));
        const Float heading = fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, ts, bitsOf(y), bitsOf(x));
        angle = fp::add(heading, doubleAt(guest, kEighth));
    }
    else
    {
        const std::uint16_t status = fp::compare(Float(0.0), Float(at.z));
        if (jumpAboveOrEqual(status))
            return (kSunAt & 0xffff0000) | status;
        angle = fp::div(Float(at.x), Float(at.z));
    }
    const u32 angleBits = bitsOf(float(angle));
    const Float sineOfAngle = fe::direct<0x4ea930, frontend::simd::sineValue>(app, ts, angleBits);
    const float sine = float(fp::mul(sineOfAngle, fld(guest, kSunSize)));
    const Float cosineOfAngle = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, ts, angleBits);
    const Float cosine = fp::mul(cosineOfAngle, fld(guest, kSunSize));
    const Float x = Float(at.x), y = Float(at.y), s = Float(sine);
    const u32 z = bitsOf(at.z);
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
    fe::direct<0x4bf370, frontend::rendd::projectPoints>(app, ts, 4, points, vertices);
    const u32 lastCode = guest.byte(vertices + 3 * kVertexBytes + kVertexClip);
    u32 codes = guest.byte(vertices + kVertexClip);
    codes &= guest.byte(vertices + kVertexBytes + kVertexClip);
    codes &= guest.byte(vertices + 2 * kVertexBytes + kVertexClip);
    if ((codes & lastCode) != 0)
        return lastCode;
    const u32 added = fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kPolygonBytes);
    if (added == 0)
        return added;
    const u32 polygon = guest.dword(slot);
    const u32 first = polygon + kPolygonVertex0;
    for (u32 v = 0; v < 4; ++v)
        guest.setDword(polygon + kPolygonVertices + 4 * v, first + v * kVertexBytes);
    guest.setWord(polygon + kPolygonA, 0);
    guest.setWord(polygon + kPolygonFlags, std::uint16_t(guest.word(kSunFlags) | kPolygonNoDepth));
    guest.setDword(polygon + kPolygonTexture, kSunTexture);
    for (u32 v = 0; v < 4; ++v)
    {
        copyDwords(guest, first + v * kVertexBytes, vertices + v * kVertexBytes, kVertexDwords);
        guest.setDword(first + v * kVertexBytes + kVertexColour, 0xffffffff);
    }
    return first + 3 * kVertexBytes;
}

nfs::u32 rendSun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_479ed0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::sun(app, ts, view); });
}

/* ---------------------------------------------------------------------------
 * The sky's colour
 * ------------------------------------------------------------------------- */

/* sub_47a4c0: whether the sky goes flat for the car eax's view: the switch [0x55d158], or
 * the camera in the tunnel of track 5.  In a split screen a black quad over the view's
 * rectangle, 0x3f7fff00 deep; alone, the fog goes black.  Otherwise the fog takes the sky's
 * gradient.  State 3 as it now stands either way. */
nfs::u32 renda::skyFlat(App* app, ThreadState& ts, u32 car)
{
    constexpr u32 kEbp = 0x1e;
    constexpr u32 kFlatSky = 0x55d158;
    constexpr u32 kTunnelTrack = 5;
    constexpr s32 kTunnelFirst = 0x38a, kTunnelLast = 0x428;
    constexpr u32 kFogColourBytes = 0x6ff9f8;
    constexpr u32 kFarDepth = 0x3f7fff00;
    constexpr u32 kBlack = 0xff000000;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0xb8);
    const u32 ebp = guest.frame() + kEbp;
    const u32 node = fe::direct<0x4217e0, carstate::nodeOfCamera>(app, ts, guest.dword(car + 4));
    u32 flat = guest.dword(kFlatSky) != 0 ? 1 : 0;
    if (gameSettings(guest).track == kTunnelTrack && s32(node) >= kTunnelFirst && s32(node) <= kTunnelLast)
        flat = 1;
    if (gameSettings(guest).playMode == 1 && flat != 0)
    {
        const u32 slot = ebp + 0x62;
        if (fe::direct<0x4bbf80, frontend::rendd::openListTake>(app, ts, slot, kPolygonBytes) != 0)
        {
            const u32 leftAt = ebp + 0x66, topAt = ebp + 0x6a, widthAt = ebp + 0x6e, heightAt = ebp + 0x72;
            const u32 corner0 = ebp + 2, corner1 = ebp + 0x42, corner2 = ebp + 0x22, corner3 = ebp - 0x1e;
            fe::direct<0x4beb80, port::text::viewportOf>(guest.app(), guest.ts(), leftAt, topAt, widthAt, heightAt);
            const u32 corners[4] = { corner0, corner1, corner2, corner3 };
            for (const u32 corner : corners)
            {
                guest.setDword(corner + 8, kFarDepth);
                guest.setDword(corner + kVertexW, kFarDepth);
            }
            const u32 left = guest.dword(leftAt), top = guest.dword(topAt);
            const u32 right = left + guest.dword(widthAt), bottom = top + guest.dword(heightAt);
            const u32 xs[4] = { left, right, right, left };
            const u32 ys[4] = { top, top, bottom, bottom };
            for (u32 i = 0; i < 4; ++i)
            {
                guest.setFloat(corners[i], float(Float(s32(xs[i]))));
                guest.setFloat(corners[i] + 4, float(Float(s32(ys[i]))));
                guest.setByte(corners[i] + kVertexClip, std::uint8_t(clipOf(guest, corners[i]).code));
            }
            makeQuad(guest, ts, guest.dword(slot),
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
    fe::direct<0x431900, graphics::sendState>(app, ts, kStateRaceFogColour, guest.dword(kFogColourBytes));
    return flat;
}

nfs::u32 rendSkyFlat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47a4c0", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyFlat(app, ts, car); });
}

/* ---------------------------------------------------------------------------
 * The dome's grid
 * ------------------------------------------------------------------------- */

/* sub_47adc0: the point eax's height from its distance: a cosine's curve down the sky. */
nfs::u32 renda::pointHeight(App* app, ThreadState& ts, u32 point)
{
    constexpr u32 kQuarter = 0x53b3b8, kPerDegree = 0x53b3c0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x18);
    const u32 flat = guest.frame();
    guest.setDword(flat, guest.dword(point));
    guest.setDword(flat + 4, 0);
    guest.setDword(flat + 8, guest.dword(point + 8));
    const Float length = Float(fe::direct<0x4e06a0, vertices::vectorLength>(app, ts, flat));
    const Float angle = fp::mul(fp::mul(length, doubleAt(guest, kQuarter)), doubleAt(guest, kPerDegree));
    const Float cosine = fe::direct<0x4ea950, frontend::simd::cosineValue>(app, ts, bitsOf(float(angle)));
    const Float curve = fp::mul(cosine, fld(guest, kSkyCurveScale));
    guest.setFloat(point + 4, float(fp::add(curve, fld(guest, kSkyCurveShift))));
    // eax as the code leaves it: the flat vector's address, which the length's function and the
    // cosine leave there.
    return flat;
}

nfs::u32 rendPointHeight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47adc0", 0, 0, nullptr, 0 };
    const u32 point = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::pointHeight(app, ts, point); });
}

/* sub_47ae10: the dome's grid made as the track loads: each point's place and height, its
 * brightness a bit of noise below the brightest, those made into hills (a root's curve),
 * smoothed twenty times, kept between nothing and the brightest, and the result lowering
 * each point and giving its colour. */
nfs::u32 renda::cloudGrid(App* app, ThreadState& ts)
{
    constexpr u32 kToSky = 0x53b3c8, kMinus128 = 0x53b3d0, kPerByte = 0x53b3d4, kHalf = 0x53b3d8, kFull = 0x53b3dc;
    constexpr u32 kPerStep = 0x53b3e0, kOfByte = 0x53b3e8, kHalfDouble = 0x53b3f0;
    constexpr u32 kPasses = 20;
    constexpr s32 kReach = 2;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x318);
    FloatMath math(guest);
    const u32 grid = guest.frame();
    const auto cell = [&](u32 row, u32 column) { return grid + (row * kGrid + column) * 4; };
    const auto gridAxis = [](u32 i) { return Float(s32(((i * 225) << 17) - 0xbb80000)); };

    for (u32 column = 0; column < kGrid; ++column)
    {
        for (u32 row = 0; row < kGrid; ++row)
        {
            const u32 point = kCloudPoints + row * kGridRowBytes + column * kPointBytes;
            const u32 corner = kCloudCorners + row * kGridRowBytes + column * kPointBytes;
            const Float toSky = doubleAt(guest, kToSky);
            const Float x = fp::mul(gridAxis(row), toSky);
            const Float z = fp::mul(toSky, gridAxis(column));
            guest.setFloat(point, float(x));
            guest.setFloat(point + 8, float(z));
            guest.setFloat(corner, float(Float(s32(row))));
            guest.setFloat(corner + 4, float(Float(s32(column))));
            u32 brightness;
            if (guest.dword(kCloudNoise) != 0)
            {
                const u32 random = callAt(guest, kRandom);
                brightness = guest.dword(kCloudBrightest) - random % guest.dword(kCloudNoise);
            }
            else
            {
                brightness = guest.dword(kCloudBrightest);
            }
            guest.setFloat(cell(row, column), float(Float(s32(brightness))));
            guest::direct<0x47adc0, pointHeight>(app, ts, point);
        }
    }

    for (u32 column = 0; column < kGrid; ++column)
    {
        for (u32 row = 0; row < kGrid; ++row)
        {
            const float centred = float(fp::mul(fp::add(fld(guest, cell(row, column)), fld(guest, kMinus128)), fld(guest, kPerByte)));
            const Float sign = Float(jumpBelowOrEqual(fp::compare(Float(0.0), Float(centred))) ? 1.0f : -1.0f);
            const Float magnitude = fe::direct<0x4ea920, frontend::simc::squareRootOf>(
                app, ts, bitsOf(float(fp::mul(Float(centred), sign))));
            const float root = float(fp::mul(magnitude, sign));
            const Float scaled = fp::mul(fp::mul(fp::add(Float(1.0), Float(root)), fld(guest, kHalf)), fld(guest, kFull));
            guest.setFloat(cell(row, column), float(scaled));
        }
    }

    for (u32 pass = 0; pass < kPasses; ++pass)
    {
        for (u32 column = 0; column < kGrid; ++column)
        {
            for (u32 row = 0; row < kGrid; ++row)
            {
                const u32 own = guest.dword(cell(row, column));
                float sum = 0.0f, weight = 0.0f;
                for (s32 dc = -kReach; dc <= kReach; ++dc)
                {
                    for (s32 dr = -kReach; dr <= kReach; ++dr)
                    {
                        if (dc == 0 && dr == 0)
                            continue;
                        const s32 r = s32(row) + dr, c = s32(column) + dc;
                        if (r < 0 || c < 0 || r >= s32(kGrid) || c >= s32(kGrid))
                            continue;
                        const u32 neighbour = cell(u32(r), u32(c));
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
                const float brightest = float(Float(s32(guest.dword(kCloudBrightest))));
                const float moved = float(fp::add(fp::mul(Float(mean), fld(guest, kHalf)), fld(guest, cell(row, column))));
                const float high = jumpAboveOrEqual(fp::compare(Float(brightest), Float(moved))) ? moved : brightest;
                u32 kept = 0;
                if (jumpBelowOrEqual(fp::compare(Float(0.0), Float(high))))
                {
                    const Float again = Float(s32(guest.dword(kCloudBrightest)));
                    kept = jumpAboveOrEqual(fp::compare(again, Float(moved))) ? bitsOf(moved) : bitsOf(float(again));
                }
                guest.setDword(cell(row, column), kept);
            }
        }
    }

    u32 left = 0;
    for (u32 column = 0; column < kGrid; ++column)
    {
        for (u32 row = 0; row < kGrid; ++row)
        {
            const u32 point = kCloudPoints + row * kGridRowBytes + column * kPointBytes;
            const u32 corner = kCloudCorners + row * kGridRowBytes + column * kPointBytes;
            const Float value = fld(guest, cell(row, column));
            const Float share = fp::mul(value, fld(guest, kOfByte));
            const Float depth = fp::mul(fp::mul(fld(guest, kSkyCurveScale), doubleAt(guest, kHalfDouble)),
                                        Float(s32(guest.dword(kCloudDepth))));
            const Float drop = fp::mul(share, depth);
            const std::int32_t whole = math.truncated(value);
            const u32 level = u32(whole) & 0xff;
            const std::uint16_t flags = guest.word(kDriverFlags);
            guest.setFloat(point + 4, float(fp::sub(fld(guest, point + 4), drop)));
            u32 alpha = level;
            if ((flags & kDriversAlphaFromNoise) != 0)
                alpha = guest.dword(kCloudBrightest) - u32(s32(guest.dword(kCloudNoise)) >> 1);
            guest.setDword(corner + 8, (alpha << 24) | (level << 16) | (level << 8) | level);
            left = level;
        }
    }
    return left;
}

nfs::u32 rendCloudGrid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47ae10", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::cloudGrid(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The sky's flashes
 * ------------------------------------------------------------------------- */

/* sub_47b350: as a race starts: the flashes' clock at the race's, none lit, no drift. */
nfs::u32 renda::flashesReset(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest.setDword(kFlashTick, guest.dword(kTicks));
    const u32 result = guest::direct<0x494360, frontend::rendc::boltsClear>(app, ts);
    guest.setDword(kFlashOn, 0);
    guest.setDword(kDriftX, 0);
    guest.setDword(kDriftZ, 0);
    return result;
}

nfs::u32 rendFlashesReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47b350", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::flashesReset(app, ts); });
}

/* sub_47b390: the same, for another caller. */
nfs::u32 renda::flashesResetAgain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x47b350, flashesReset>(app, ts);
}

nfs::u32 rendFlashesResetAgain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47b390", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::flashesResetAgain(app, ts); });
}

/* sub_47b3a0: the flash on the grid at 0x6fdc50: the three by three about its cell, each
 * as bright as its row's distance leaves it, if that is brighter than it is. */
nfs::u32 renda::flashOnGrid(App* app, ThreadState& ts)
{
    constexpr u32 kVertices = 0x6fdc50, kRowBytes = 0x1a0;
    constexpr u32 kFalloff = 0x53b3f8;
    constexpr u32 kDomeKind = 0x55d1f4;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x30);
    FloatMath math(guest);
    s32 distance = -1;
    s32 row = s32(guest.dword(kFlashCellX) - 1);
    for (;;)
    {
        const u32 rowEnd = guest.dword(kFlashCellX) + 2;
        if (row >= s32(rowEnd))
            return rowEnd;
        for (s32 column = s32(guest.dword(kFlashCellZ) - 1); column < s32(guest.dword(kFlashCellZ) + 2); ++column)
        {
            if (row < 0 || row >= s32(kGrid) || column < 0 || column >= s32(kGrid))
                continue;
            const u32 vertex = kVertices + u32(row) * kRowBytes + u32(column) * kVertexBytes;
            const std::uint8_t now = guest.byte(vertex + kVertexColour);
            const u32 away = distance > 0 ? u32(distance) : u32(0) - u32(distance);
            const Float falloff = fp::mul(Float(s32(away)), doubleAt(guest, kFalloff));
            const Float kept = fp::sub(Float(1.0), falloff);
            const Float brightness = Float(s32(guest.byte(kFlashBrightness)));
            const std::uint8_t lit = std::uint8_t(math.truncated(fp::mul(kept, brightness)));
            if (lit <= now)
                continue;
            const u32 level = lit;
            const u32 topByte = guest.dword(kDomeKind) == 2 ? guest.dword(vertex + kVertexColour) & kOpaque : level << 24;
            guest.setDword(vertex + kVertexColour, topByte | (level << 16) | (level << 8) | level);
        }
        ++distance;
        ++row;
    }
}

nfs::u32 rendFlashOnGrid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47b3a0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::flashOnGrid(app, ts); });
}

/* sub_47b4f0: edx points at ebx as vertices at the first of the stack's arguments, by the
 * view eax's camera matrix and its height, their colours and textures' corners from ecx,
 * moved by the stack's other two. */
nfs::u32 renda::domeVertices(App* app, ThreadState& ts, u32 view, u32 count, u32 points, u32 corners, u32 vertex,
                               u32 moveUBits, u32 moveVBits)
{
    constexpr u32 kFrame = 0x20;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kFrame);
    const u32 matrix = view + kViewCameraMatrix;
    const u32 shift = guest.frame();
    guest.setDword(shift, 0);
    guest.setDword(shift + 8, 0);
    guest.setFloat(shift + 4, float(-fld(guest, view + kViewHeight)));
    putVector(guest, shift, rotated(guest, shift, matrix));
    const Float moveU = Float(floatOf(moveUBits)), moveV = Float(floatOf(moveVBits));
    // By address: the eax it leaves is this function's answer for no points, its core hands back none, and
    // the ebx it leaves this leaves too.
    u32 result = callAt(guest, kObjectVertices, args({ count, points, matrix, shift }, { vertex }));
    while (--count != 0xffffffff)
    {
        guest.setDword(vertex + kVertexColour, guest.dword(corners + 8));
        guest.setFloat(vertex + kVertexU, float(fp::add(fld(guest, corners), moveU)));
        guest.setFloat(vertex + kVertexV, float(fp::add(fld(guest, corners + 4), moveV)));
        corners += kPointBytes;
        points += kPointBytes;
        vertex += kVertexBytes;
        result = vertex;
    }
    return result;
}

nfs::u32 rendDomeVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47b4f0", 0xc, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax;
    const u32 count = args.edx;
    const u32 points = args.ebx;
    const u32 corners = args.ecx;
    const u32 vertex = stackWord(app, ts, 0);
    const u32 moveU = stackWord(app, ts, 1);
    const u32 moveV = stackWord(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::domeVertices(app, ts, view, count, points, corners, vertex, moveU, moveV); });
}

/* sub_47b5c0: the sky's weather up to the race's tick: every so many ticks a chance of a
 * flash somewhere round the camera (and its thunder); the newest flash's brightness and cell
 * on the grid; the clouds' drift, wrapped at one. */
nfs::u32 renda::skyWeather(App* app, ThreadState& ts)
{
    constexpr u32 kFlashChance = 0x55d260, kFlashEvery = 0x55d264;
    constexpr u32 kDriftXStep = 0x55d20c, kDriftZStep = 0x55d210;
    constexpr u32 kHalfWay = 0x53b400, kPerCell = 0x53b408, kRoundUp = 0x53b410;
    constexpr u32 kSpread = 1500, kHalfSpread = 0x2ee;
    constexpr s32 kLastCell = 12;
    constexpr u32 kEbp = 0x48;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x60);
    FloatMath math(guest);
    const u32 ebp = guest.frame() + kEbp;
    const u32 place = ebp - 0x3c, unit = ebp - 0x30, flash = ebp - 0x48, brightness = ebp - 0x24;
    for (;;)
    {
        const u32 tick = guest.dword(kFlashTick);
        if (s32(tick) >= s32(guest.dword(kTicks)))
            return tick;
        const u32 every = guest.dword(kFlashEvery);
        if (guest.dword(kFlashChance) != 0 && every != 0)
        {
            if (s32(tick) % s32(every) == 0 && callAt(guest, kRandom) % 100 < guest.dword(kFlashChance))
            {
                const u32 x = callAt(guest, kRandom) % kSpread - kHalfSpread;
                const u32 z = callAt(guest, kRandom) % kSpread - kHalfSpread;
                guest.setFloat(place, float(Float(s32(x))));
                guest.setDword(place + 4, 0);
                guest.setFloat(place + 8, float(Float(s32(z))));
                fe::direct<0x4e01c0, vertices::makeUnitVector>(app, ts, place, unit);
                const float angle = float(fe::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(
                    app, ts, guest.dword(unit + 8), guest.dword(unit)));
                guest::direct<0x47adc0, pointHeight>(app, ts, place);
                guest::direct<0x4945a0, frontend::rendc::boltStrike>(app, ts, guest.dword(place), guest.dword(place + 4),
                                                                     guest.dword(place + 8), bitsOf(angle));
                callAt(guest, kThunder, args({ x << 16, z << 16 }));
            }
            const u32 lit = guest::direct<0x494390, frontend::rendc::boltGlow>(app, ts, flash, brightness);
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
                    const s32 cell = math.truncated(Float(at));
                    if (cell >= kLastCell)
                        return u32(kLastCell);
                    return u32(cell > 0 ? cell : 0);
                };
                guest.setDword(kFlashCellX, inGrid(atX));
                guest.setDword(kFlashCellZ, inGrid(atZ));
            }
        }
        const float driftX = float(fp::add(fld(guest, kDriftXStep), fld(guest, kDriftX)));
        const float driftZ = float(fp::add(fld(guest, kDriftZStep), fld(guest, kDriftZ)));
        guest.setFloat(kDriftX, driftX);
        guest.setFloat(kDriftZ, driftZ);
        if (s32(guest.dword(kDriftX)) > s32(kFloatOne))
            guest.setDword(kDriftX, 0);
        if (s32(guest.dword(kDriftZ)) > s32(kFloatOne))
            guest.setDword(kDriftZ, 0);
        guest.setDword(kFlashTick, tick + 1);
    }
}

nfs::u32 rendSkyWeather(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_47b5c0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return renda::skyWeather(app, ts); });
}

}

}
