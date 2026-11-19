#include <nfs3hp.h>
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_sime.h"
#include "native_render.h"
#include "native_thrash.h"
#include "native_track.h"
#include "native_vertices.h"
#include "thrash_calls.h"
#include <SDL3/SDL.h>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <optional>
#include <string>
#include <type_traits>
#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#endif

/* a * b + c fused into one fma rounds once where the x87 rounds twice; and the
 * fast path reads the host's exception flags, so its arithmetic must stay
 * between the clear and the test. */
#if defined(__clang__)
#pragma clang fp contract(off)
#pragma STDC FENV_ACCESS ON
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#pragma fenv_access(on)
#endif

/* The game's hottest vertex and polygon loops, native whole since phase 2
 * (2026-10-27): each address of the game's is a function of that name in
 * nfs3hp::vertices, at the end of this file, which the game's code calls by
 * name; a call through a pointer finds it too.  They do the original's arithmetic in
 * the original's order, rounded as the game's code rounds it, at single
 * precision.  They carry the
 * port's changes to their code: the mirror's car scale (mirrorCarScale) and
 * its headlight light on the road (mirrorAsMain), the alpha intensity's
 * scaling on this driver.
 * (The hooks left at the end -- arenaReset, aiVotes, the [COMM] traces --
 * only watch, and always hand the call on to the game's code.)
 *
 * The arithmetic is the emulated FPU's own (SingleMath: FPU::add, sub, mul, div
 * and sqrt, which round as the control word asks -- a float's 24 bits at
 * single precision, the double as it is at 53 or 64), on values kept as wide
 * as the x87 stack keeps them; a float is made only where the original stores
 * one (fstp dword), an integer only through FPU::toInteger (fistp, in the
 * control word's rounding).
 *
 * At single precision to nearest the hottest loops (FloatMath) run as floats
 * instead: with float inputs, float arithmetic gives the x87's bits -- a double
 * holds every product and sum of two floats well enough that rounding it again
 * to a float is the correct rounding -- as long as nothing leaves a float's
 * range.  One track vertex does: a product past the largest float, an infinity
 * the x87 never sees (2026-09-25).  So each such call runs as floats, and when
 * the host raises overflow, underflow or invalid on the way, runs again through
 * the emulated FPU's own operations.
 *
 * Memory comes out as the game's code leaves it, all but the stack below
 * the returned esp; the x87 stack and control word too; of the registers,
 * eax where the game's callers read it and every one the code keeps -- all
 * but its own register arguments, as Watcom's callers have them.  The flags
 * and the FPU status word, which no caller reads after a call, are left as
 * they come but where eax takes the status (fnstsw ax). */
namespace nfs3hp
{

// nfs3hp_main.cpp: a view pass's kind as the main view's where the mirror is drawn in full.
nfs::u32 mirrorAsMain(nfs::u32 kind);
double mirrorCarScale(double scale);

namespace vertices
{

namespace
{

/* The typed natives' words (plan step 5): a core with named parameters that keeps its locals in
 * C++, and a thunk that takes the registers and the stack words and leaves what the game's
 * callers read -- eax where one reads it, esp past the words its ret takes, every other register
 * as it came -- and the x87 as the game's code does.  A core that calls out is handed the esp it
 * is entered at (its return address's slot), and makes each call at the depth the code makes it,
 * its frame where the code has it. */
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;
using i32 = std::int32_t;

// Cores further down that the ones before them call straight, where the game's code calls them.
u32 clipTriangleOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c);
u32 projectedTriangleOf(App* app, u32 slot, u32 a, u32 b, u32 c);
u32 clipQuadOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c, u32 d);
void clipTriangleByZOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c);
u32 sortedBucketsOf(App* app, ThreadState& ts, u32 esp, u32 list, u32 start);

inline float asFloat(nfs::u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

inline nfs::u32 asBits(float value)
{
    nfs::u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The arithmetic as floats: exact at single precision to nearest while
 * nothing over- or underflows (singleNearest). */
struct FloatMath
{
    typedef float Value;
    static Value add(Value a, Value b) { return a + b; }
    static Value sub(Value a, Value b) { return a - b; }
    static Value mul(Value a, Value b) { return a * b; }
    static Value div(Value a, Value b) { return a / b; }
    static Value sqrt(Value a) { return std::sqrt(a); }
};

/* The arithmetic as the game's code does it, through the emulated FPU:
 * each result rounded as the control word asks at the time (FPU::add and the
 * rest), whatever the operands -- a float from memory or a double -- and with
 * the x87's range.  Exact in every mode; what the natives call M. */
struct SingleMath
{
    typedef double Value;
    SingleMath() {}
    Value add(Value a, Value b) const { return double(fp::add(nfs::Float(a), nfs::Float(b))); }
    Value sub(Value a, Value b) const { return double(fp::sub(nfs::Float(a), nfs::Float(b))); }
    Value mul(Value a, Value b) const { return double(fp::mul(nfs::Float(a), nfs::Float(b))); }
    Value div(Value a, Value b) const { return double(fp::div(nfs::Float(a), nfs::Float(b))); }
    Value sqrt(Value a) const { return double(fp::sqrt(nfs::Float(a))); }
};

/* Whether the FPU rounds to a float's 24 bits and to nearest, where float
 * arithmetic is the x87's (FloatMath). */
inline bool singleNearest(const nfs::ThreadState&)
{
    return true;  // the game's arithmetic is at 24 bits, to nearest, in every mode it runs in (fp::)
}

/* What the host raises when a float run left a float's range (or came from a value that had):
 * the run is then made again through the emulated FPU.  Every output of such a run is a function
 * of inputs it does not write, so the second run simply writes over the first. */
constexpr int kOutOfRange = FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID;

/* Two products of differences as the x87 works them out,
 *   one = (p - q) * (r - s),  two = (t - u) * (v - w),  all eight floats,
 * for a comparison of the two -- a polygon's facing -- on the x87 stack, never
 * stored.  At single precision to nearest: a difference of floats comes out
 * of float arithmetic as the x87 has it, unless it overflows; a product of two
 * floats is exact in a double; and rounding each to 24 bits cannot swap two
 * that lie further apart than 2^-20 of the larger.  So the exact products are
 * returned, which compare as the rounded ones would, and the emulated FPU's
 * rounding is done only for a near tie or an infinity -- and in every other
 * mode, where the differences and products are the double's. */
struct CrossProducts
{
    double two;
    double one;
};

inline CrossProducts crossProducts(nfs::ThreadState& ts, float p, float q, float r, float s, float t, float u, float v,
                                   float w)
{
    if (singleNearest(ts))
    {
        const float a = p - q;
        const float b = r - s;
        const float c = t - u;
        const float d = v - w;
        const double one = double(a) * double(b);
        const double two = double(c) * double(d);
        if (std::isfinite(a) && std::isfinite(b) && std::isfinite(c) && std::isfinite(d)
            && std::fabs(two - one) > 0x1p-20 * std::fmax(std::fabs(one), std::fabs(two)))
            return {two, one};
    }
    const SingleMath M;
    return {M.mul(M.sub(t, u), M.sub(v, w)), M.mul(M.sub(p, q), M.sub(r, s))};
}

/* A typed core of another file (native_frontend_*.h, native_carstate.h, native_track.h) at `Address`, called as C++ from
 * the depth `esp` through guest::direct, which keeps the game's address: what the callee reads
 * -- its register arguments, then the stack words it takes -- reaches it as parameters, and
 * nothing is set in a register.  Every register but eax is back as it was after.  The callee's eax (a bool core's 1 or
 * 0), or none when the game stopped in it. */
template <u32 Address, auto Core, typename... A>
std::optional<u32> coreAt(App* app, ThreadState& ts, u32 esp, A... args)
{
    ts.sp = esp;
    try
    {
        const auto result = guest::direct<Address, Core>(app, ts, args...);
        if (ts.terminate)
            return std::nullopt;
        if constexpr (std::is_same_v<std::decay_t<decltype(result)>, std::optional<bool>>)
            return result ? std::optional<u32>(*result ? 1u : 0u) : std::nullopt;
        else
            return std::optional<u32>(result);
    }
    catch (const fe::Terminated&)  // a menu core's stop, as its thunk (fe::runThunk) takes it
    {
        return std::nullopt;
    }
}

/* sub_4bbde0, Render_GetTm's buffer taken on by `bytes` from what `slot` holds, from the depth
 * `esp`: 1, or 0 when the buffer has run out.  Its message then (Error, sub_401010) is handed
 * the ebp it pushes, the caller's frame as the code has it: `ebp`. */
std::optional<u32> commitArena(App* app, ThreadState& ts, u32 esp, u32 ebp, u32 slot, u32 bytes)
{
    static_cast<void>(ebp);  // only what the message's push of ebp leaves below esp: nothing written for it
    return coreAt<0x4bbde0, frontend::rendd::arenaTake>(app, ts, esp, slot, bytes);
}

/* What fnstsw ax leaves in eax (`eax` before it) after an fcomp of `a` with `b`: the x87's status
 * word as the compare leaves it, over eax's low half -- where a caller reads that eax. */
u32 statusAfterCompare(u32 eax, double a, double b)
{
    return (eax & 0xffff0000) | fp::compare(a, b);
}

/* One of THRASH's exports through the game's pointer to it (guest::driver, thrash_calls.h: the port's
 * driver as C++ where the pointer leads to it), from the depth
 * `esp`: stdcall, only `words`; every register but eax kept. */
template <u32 Slot, auto Fn, typename... W>
std::optional<u32> driverAt(App* app, ThreadState& ts, u32 esp, W... words)
{
    ts.sp = esp;
    return guest::driver<Slot, Fn>(app, ts, words...);
}

/* driverAt for the callers that go on with the export's eax: its result; a void one's leaves eax as it was, `eax`. */
template <u32 Slot, auto Fn, typename... W>
std::optional<u32> driverKeeping(App* app, ThreadState& ts, u32 esp, u32 eax, W... words)
{
    const std::optional<u32> result = driverAt<Slot, Fn>(app, ts, esp, words...);
    if (!result)
        return result;
    if constexpr (std::is_void_v<decltype(Fn(app, ts, u32(words)...))>)
        return eax;
    else
        return result;
}

/* sub_41a3e0: the colour of a track vertex, darkened by how far off the view's
 * axis it lies and by the fog past [0x552e04].  Written to `out`. */
template <typename Math>
void shadeVertex(nfs::App* app, const Math& math, float x, float y, float z, nfs::u32 colour,
                 nfs::u32 out)
{
    const float inverse = float(math.div(1.0f, z));
    const float absX = 0.0f > x ? -x : x;
    const float absY = 0.0f > y ? -y : y;
    const float offX = float(math.mul(absX, inverse));
    const float offY = float(math.mul(inverse, absY));
    float fog = 1.0f;
    const float fogStart = app->getMemory<float>(nfs::u32(0x552e04));
    if (z > fogStart)
        fog = float(math.sub(1.0f, math.mul(math.sub(z, fogStart), app->getMemory<float>(nfs::u32(0x552e10)))));
    // Each clamped to [0, 1] -- the upper bound by the bits, as a signed integer.
    const float clampX = 0.0f > offX ? 0.0f : (nfs::s32(asBits(offX)) > 0x3f800000 ? 1.0f : offX);
    const float clampY = 0.0f > offY ? 0.0f : (nfs::s32(asBits(offY)) > 0x3f800000 ? 1.0f : offY);
    // On the x87 stack all the way to the fistp ([0x536c74] 0.5f, [0x536c7c] 65536.0).
    const double light = double(math.mul(math.mul(math.sub(1.0f, math.mul(math.add(clampX, clampY), 0.5f)), fog),
                                         65536.0f));
    // fistp as the game's code has it: the control word's rounding, the
    // indefinite past an integer's range.
    nfs::u32 level = nfs::u32(fp::toInteger<nfs::s32>(nfs::Float(light)));
    if (nfs::s32(level) >= 0x10000)
        level = 0xffff;
    level = (level >> 8) & 0xff;
    const nfs::u32 redBlue = (colour & 0xff00ff) * level;
    const nfs::u32 alphaGreen = ((colour & 0xff00ff00) >> 8) * level;
    app->getMemory<nfs::u32>(out) = (alphaGreen & 0xff00ff00) | ((redBlue >> 8) & 0xff00ff);
}

/* The clip code of a point in view space, as sub_41a550 builds it: 0x10 nearer
 * than [0x552e0c], 0x20 past [0x552e08], 8/4 above/below, 2/1 right/left. */
nfs::u8 clipCode(float x, float y, float z, float nearest, float farthest)
{
    nfs::u8 code = 0;
    if (!(z >= nearest))
        code |= 0x10;
    if (z > farthest)
        code |= 0x20;
    if (y > z)
        code |= 8;
    else if (-z > y)
        code |= 4;
    if (x > z)
        code |= 2;
    else if (-z > x)
        code |= 1;
    return code;
}

/* sub_41a550's work: `count` track vertices at `in` (3 floats each) by the view
 * `view` -- 3x3 matrix at +0x28, position at +0x58, colours at +0x70/+0x74 --
 * into view space and onto the screen at `out` (0x1c bytes each), with the
 * 0x20-byte records from `record` on.  A vertex behind the view is mirrored
 * into the second view when +0x74 is set. */
template <typename Math>
void trackVertexList(App* app, const Math& math, u32 view, u32 count, u32 in, u32 out, u32 record)
{
    const float* m = &app->getMemory<float>(view + 0x28);
    const float* t = &app->getMemory<float>(view + 0x58);
    const nfs::u32 frontColour = app->getMemory<nfs::u32>(view + 0x70);
    const nfs::u32 backColour = app->getMemory<nfs::u32>(view + 0x74);
    const float nearest = app->getMemory<float>(nfs::u32(0x552e0c));
    const float farthest = app->getMemory<float>(nfs::u32(0x552e08));

    for (u32 left = count; left != 0; --left, record += 0x20, out += 0x1c, in += 0xc)
    {
        app->getMemory<nfs::u16>(out + 6) = 0x10;
        app->getMemory<nfs::u32>(out + 0x18) = 0;
        app->getMemory<nfs::u32>(out + 8) = record;
        app->getMemory<nfs::u16>(out + 4) = 0x10;
        app->getMemory<nfs::u32>(out + 0x14) = 0;

        // Rotated, stored, then moved: each a float in memory in between.
        const float* v = &app->getMemory<float>(in);
        const float rx = float(math.add(math.add(math.mul(v[0], m[0]), math.mul(v[1], m[3])), math.mul(v[2], m[6])));
        const float ry = float(math.add(math.add(math.mul(v[0], m[1]), math.mul(v[1], m[4])), math.mul(v[2], m[7])));
        const float rz = float(math.add(math.add(math.mul(v[0], m[2]), math.mul(v[1], m[5])), math.mul(v[2], m[8])));
        const float x = float(math.add(rx, t[0]));
        const float y = float(math.add(ry, t[1]));
        nfs::u32 zBits = asBits(float(math.add(rz, t[2])));
        if ((zBits & 0x7fffffff) == 0)
            zBits = 0x37800080;
        const float z = asFloat(zBits);
        const float inverse = nfs::s32(zBits) < 0x3f800000 ? 1.0f : float(math.div(1.0f, z));
        app->getMemory<float>(out + 0xc) = float(math.mul(math.add(math.mul(x, inverse), 1.0f), 0.5f));
        app->getMemory<float>(out + 0x10) = float(math.mul(math.add(math.mul(y, inverse), 1.0f), 0.5f));
        app->getMemory<float>(out) = float(math.div(app->getMemory<float>(record + 0xc), inverse));

        if (!(0.0f >= z))
        {
            app->getMemory<nfs::u16>(out + 4) = clipCode(x, y, z, nearest, farthest);
            if (!(z >= farthest))
                shadeVertex(app, math, x, y, z, frontColour, out + 0x14);
        }
        else if (backColour != 0)
        {
            const float bx = asFloat(asBits(x) ^ 0x80000000);
            const float by = asFloat(asBits(y) ^ 0x80000000);
            const float bz = asFloat(zBits ^ 0x80000000);
            app->getMemory<nfs::u16>(out + 6) = clipCode(bx, by, bz, nearest, farthest);
            if (!(bz >= farthest))
                shadeVertex(app, math, bx, by, bz, backColour, out + 0x18);
        }
    }
}

/* sub_41a550's work: trackVertexList as floats at single precision to nearest, once more through
 * the emulated FPU's operations should a value leave a float's range (it writes only what it works
 * out from what it reads); in any other mode the FPU's way at once. */
void trackVerticesOf(App* app, ThreadState& ts, u32 view, u32 count, u32 in, u32 out, u32 record)
{
    if (singleNearest(ts))
    {
        std::feclearexcept(kOutOfRange);
        trackVertexList(app, FloatMath(), view, count, in, out, record);
        if (!std::fetestexcept(kOutOfRange))
            return;
    }
    trackVertexList(app, SingleMath(), view, count, in, out, record);
}

/* sub_4bf4c0's clip code of one projected vertex, from the bits of its floats
 * compared as signed integers: 0x10 when 1/z is negative or past [0x7d34f8],
 * else 8/4 for y before [0x7d3504]/past [0x7d3500] and 1/2 for x before
 * [0x7d34fc]/past [0x7d350c]. */
nfs::u8 objectClip(nfs::App* app, nfs::u32 out)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 inverse = word(out + 0xc);
    if ((inverse & 0x80000000) || nfs::s32(inverse) >= nfs::s32(word(0x7d34f8)))
        return 0x10;
    nfs::u8 code = 0;
    // One axis: below `low` (or negative) sets `under`, past `high` sets `over`.
    auto axis = [&](nfs::u32 value, nfs::u32 low, nfs::u32 high, nfs::u8 under, nfs::u8 over) {
        if ((value & 0x80000000) || nfs::s32(value) < nfs::s32(low))
            code |= under;
        else if (nfs::s32(value) > nfs::s32(high))
            code |= over;
    };
    axis(word(out + 4), word(0x7d3504), word(0x7d3500), 8, 4);
    axis(word(out), word(0x7d34fc), word(0x7d350c), 1, 2);
    return code;
}

/* What objectClip's code leaves in eax for the vertex at `out`: its 1/z when that is behind the
 * near plane, else its x -- the last word the code looks at, as sub_4bf4c0 returns. */
u32 objectClipLeft(App* app, u32 out)
{
    const u32 inverse = app->getMemory<u32>(out + 0xc);
    if ((inverse & 0x80000000) || nfs::s32(inverse) >= nfs::s32(app->getMemory<u32>(0x7d34f8)))
        return inverse;
    return app->getMemory<u32>(out);
}

/* sub_4bf4c0's work: `count` (at least 1) object vertices from `in`, 3 floats
 * each, turned by the 3x3 `matrix` and moved by `position` into view space and
 * onto the screen at `out`, 0x20 bytes each: x, y, z, 1/z, and the clip code at
 * +0x14.  Pipelined as the original is: a vertex is moved while the one before
 * it is projected. */
template <typename Math>
void objectVertexList(App* app, const Math& math, i32 count, u32 in, u32 matrix, u32 position, u32 out)
{
    const float* m = &app->getMemory<float>(matrix);
    const float* t = &app->getMemory<float>(position);
    const float scaleX = app->getMemory<float>(nfs::u32(0x56009c));
    const float scaleY = app->getMemory<float>(nfs::u32(0x5600a0));
    const float centreX = app->getMemory<float>(nfs::u32(0x5600a4));
    const float centreY = app->getMemory<float>(nfs::u32(0x5600a8));

    // Rotated and moved on the x87 stack, stored once; z of exactly 1 made tiny.
    auto move = [&](nfs::u32 from, nfs::u32 to) {
        const float* v = &app->getMemory<float>(from);
        const auto x = math.add(math.add(math.mul(v[0], m[0]), math.mul(v[1], m[3])), math.mul(v[2], m[6]));
        const auto y = math.add(math.add(math.mul(v[0], m[1]), math.mul(v[1], m[4])), math.mul(v[2], m[7]));
        const auto z = math.add(math.add(math.mul(v[0], m[2]), math.mul(v[1], m[5])), math.mul(v[2], m[8]));
        app->getMemory<float>(to) = float(math.add(x, t[0]));
        app->getMemory<float>(to + 4) = float(math.add(y, t[1]));
        app->getMemory<float>(to + 8) = float(math.add(z, t[2]));
        if (app->getMemory<nfs::u32>(to + 8) == 0x3f800000)
            app->getMemory<nfs::u32>(to + 8) = 0x37800080;
        return float(math.div(1.0f, app->getMemory<float>(to + 8)));
    };
    auto project = [&](nfs::u32 at, float inverse) {
        const auto overX = math.mul(scaleX, inverse);
        const auto overY = math.mul(scaleY, inverse);
        const auto y = math.mul(overY, app->getMemory<float>(at + 4));
        const auto x = math.mul(overX, app->getMemory<float>(at));
        app->getMemory<float>(at + 0xc) = inverse;
        app->getMemory<float>(at) = float(math.add(x, centreX));
        app->getMemory<float>(at + 4) = float(math.add(y, centreY));
    };

    float inverse = move(in, out);
    for (nfs::s32 left = count - 1; left > 0; --left)
    {
        in += 0xc;
        const float next = move(in, out + 0x20);
        project(out, inverse);
        inverse = next;
        app->getMemory<nfs::u8>(out + 0x14) = objectClip(app, out);
        out += 0x20;
    }
    project(out, inverse);
    app->getMemory<nfs::u8>(out + 0x14) = objectClip(app, out);
}

/* sub_4e03b0: vectors turned by a 3x3 matrix, the row vector on the left.
 *   eax  count; edx  the vectors in; ebx  the matrix; ecx  the vectors out
 * All three of a vector are read before it is written, so it turns in place as
 * well.  Called from a dozen places; a twentieth of a split-screen race. */
template <typename Math>
void rotateVectorList(App* app, const Math& math, i32 count, u32 in, u32 matrix, u32 out)
{
    for (i32 k = 0; k < count; ++k, in += 0xc, out += 0xc)
    {
        const float* v = &app->getMemory<float>(in);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        const float* m = &app->getMemory<float>(matrix);
        const float x = float(math.add(math.add(math.mul(v0, m[0]), math.mul(v1, m[3])), math.mul(v2, m[6])));
        const float y = float(math.add(math.add(math.mul(v0, m[1]), math.mul(v1, m[4])), math.mul(v2, m[7])));
        const float z = float(math.add(math.add(math.mul(v0, m[2]), math.mul(v1, m[5])), math.mul(v2, m[8])));
        float* o = &app->getMemory<float>(out);
        o[0] = x;
        o[1] = y;
        o[2] = z;
    }
}

/* sub_49db70: sphere-map texture coordinates of a car's vertices.
 *   eax  count; edx  the normals; ebx  where they go, turned; ecx  the
 *   vertices, 0x20 bytes each; [esp+4]  the matrix
 * The normals are turned first (sub_4e03b0); then for each, with |z| and
 *   s = 1 / sqrt((1 + |z|) * [0x53c6a8]) * [0x53c6b0], a double,
 *   u = x * s + [0x53c6b8] at +0x18 and v = s * y + [0x53c6b8] at +0x1c. */
template <typename Math>
void envMapLoop(nfs::App* app, const Math& math, nfs::u32 count, nfs::u32 in, nfs::u32 out)
{
    typedef typename Math::Value V;
    const V scale = app->getMemory<float>(nfs::u32(0x53c6a8));
    const double wide = app->getMemory<double>(nfs::u32(0x53c6b0));
    const V centre = app->getMemory<float>(nfs::u32(0x53c6b8));
    for (nfs::u32 left = count; left != 0; --left, in += 0xc, out += 0x20)
    {
        const float z = app->getMemory<float>(in + 8);
        // |z| through [ebp-4], a float
        V absZ = z;
        if (0.0f > z)
            absZ = -z;
        const V root = math.sqrt(math.mul(math.add(V(1), absZ), scale));
        // fmul qword: the one operand that is no float, rounded by the FPU
        // (as floats, a result past a float's range raises overflow here)
        const V s = V(SingleMath().mul(double(math.div(V(1), root)), wide));
        const V x = app->getMemory<float>(in);
        app->getMemory<float>(out + 0x18) = float(math.add(math.mul(x, s), centre));
        const V y = app->getMemory<float>(in + 4);
        app->getMemory<float>(out + 0x1c) = float(math.add(math.mul(s, y), centre));
    }
}

/* sub_49db70's work: `count` normals at `normals` turned by `matrix` into `turned`, then the
 * coordinates of the vertices at `vertices` from them (envMapLoop) -- at single precision to
 * nearest as floats, and once more the emulated FPU's way should one leave their range: the loop
 * only writes what it works out from what it reads.  In any other mode the FPU's way at once. */
void envMapCoordsOf(App* app, ThreadState& ts, u32 count, u32 normals, u32 turned, u32 vertices, u32 matrix)
{
    turnVectors(app, ts, i32(count), normals, matrix, turned);
    if (singleNearest(ts))
    {
        std::feclearexcept(kOutOfRange);
        envMapLoop(app, FloatMath(), count, turned, vertices);
        if (!std::fetestexcept(kOutOfRange))
            return;
    }
    envMapLoop(app, SingleMath(), count, turned, vertices);
}

nfs::u32 envMapCoordsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    envMapCoordsOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, app->getMemory<u32>(ts.sp + 4));
    ts.sp += 4 + 4;
    return args.eax;
}

/* sub_49da30: a car's vertices lit by the sun.
 *   eax  the car: records of 0x84 bytes at [eax], this part's [eax+4]
 *   edx  count; ebx  the normals; ecx  where they go, turned
 *   [esp+4]  the vertices, 0x20 bytes each; [esp+8]  the matrix
 * The normals are turned (sub_4e03b0) and dotted with the light at 0x55e6a0
 * into the part's record (sub_4e0210).  A vertex facing the light gets
 *   n = round(d * [0x53c6a0]), a double;  each of r, g and b
 *   base + (n * slope >> 16), capped
 * at +0x10, the others the base colour alone. */
void shadeNormalsOf(App* app, ThreadState& ts, u32 car, u32 count, u32 normals, u32 turned, u32 vertices,
                    u32 matrix)
{
    auto global = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 dots = global(car) + global(car + 4) * 0x84;
    turnVectors(app, ts, i32(count), normals, matrix, turned);
    dotProductsOf(app, ts, i32(count), turned, 0x55e6a0, dots);

    const double factor = app->getMemory<double>(nfs::u32(0x53c6a0));
    nfs::u32 in = dots;
    nfs::u32 out = vertices;
    for (nfs::u32 left = count; left != 0; --left, out += 0x20, in += 4)
    {
        const double d = app->getMemory<float>(in);
        if (0.0 < d || d != d)
        {
            // fmul qword, fistp: the FPU's rounding, then the control word's
            const nfs::u32 n = nfs::u32(fp::toInteger<nfs::s32>(nfs::Float(SingleMath().mul(d, factor))));
            nfs::u32 red = global(0x55e6b8) + ((n * global(0x55e6d0)) >> 16);
            if (red > global(0x55e6c4))
                red = global(0x55e6c4);
            nfs::u32 green = global(0x55e6bc) + ((global(0x55e6d4) * n) >> 16);
            if (green > global(0x55e6c8))
                green = global(0x55e6c8);
            nfs::u32 blue = ((global(0x55e6d8) * n) >> 16) + global(0x55e6c0);
            if (blue > global(0x55e6cc))
                blue = global(0x55e6cc);
            app->getMemory<nfs::u32>(out + 0x10) = (red << 16) | global(0x55e6e4) | (green << 8) | blue;
        }
        else
            app->getMemory<nfs::u32>(out + 0x10) =
                (global(0x55e6bc) << 8) | global(0x55e6e4) | (global(0x55e6b8) << 16) | global(0x55e6c0);
    }
}

nfs::u32 shadeNormalsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    shadeNormalsOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, app->getMemory<u32>(esp + 4),
                   app->getMemory<u32>(esp + 8));
    ts.sp = esp + 4 + 8;
    return args.eax;
}

/* sub_49dbf0: those of a part's polygons that face the view and are not off
 * the screen, into the draw list.
 *   eax  the car or object: polygons of 0x84 bytes at [eax], the part's first
 *   at [eax+edx*4+0x5fc], how many at [eax+edx*4+0x6fc];  edx  the part
 *   ebx  nonzero to take the vertices' texture coordinates into the polygon
 * A polygon faces away when its corners' cross product on the screen says so,
 * flipped by [0x554e4c]; bit 0 of +7 makes it two-sided.  The draw list runs
 * from [0x7a3d0c] through each polygon's first dword to [0x7a3d08]. */
void cullPolygonsOf(App* app, ThreadState& ts, u32 object, u32 part, u32 flag)
{
    auto reg32At = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto reg8At = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 slot = object + part * 4;
    const nfs::u32 count = reg32At(slot + 0x6fc);
    if (count == 0)
        return;

    // 0 when the cross product of a, b and c says the polygon faces the view.
    auto facing = [&](nfs::u32 polygon) -> nfs::u32 {
        const nfs::u32 a = reg32At(polygon + 8);
        const nfs::u32 b = reg32At(polygon + 0xc);
        const nfs::u32 c = reg32At(polygon + 0x10);
        const CrossProducts cross = crossProducts(ts, floatAt(c + 4), floatAt(b + 4), floatAt(a), floatAt(b),
                                                  floatAt(a + 4), floatAt(b + 4), floatAt(c), floatAt(b));
        return reg32At(0x554e4c) ^ (cross.two > cross.one ? 1 : 0);
    };

    nfs::u32 polygon = reg32At(slot + 0x5fc) * 0x84 + reg32At(object);
    nfs::u32 ahead = polygon;
    nfs::u32 current = (reg8At(polygon + 7) & 1) ? 0 : facing(polygon);
    for (nfs::s32 i = 0; i < nfs::s32(count); ++i)
    {
        nfs::u32 next = 0;
        if (i < nfs::s32(count - 1))
        {
            const bool twoSided = reg8At(ahead + 0x8b) & 1;
            ahead += 0x84;
            next = twoSided ? 0 : facing(ahead);
        }
        const nfs::u32 edx = polygon;
        if (current == 0
            && !(reg8At(reg32At(edx + 8) + 0x14) & reg8At(reg32At(edx + 0xc) + 0x14) & reg8At(reg32At(edx + 0x10) + 0x14)))
        {
            if (reg32At(0x7a3d0c) == 0)
                reg32At(0x7a3d0c) = edx;
            else
                reg32At(reg32At(0x7a3d08)) = edx;
            reg8At(edx + 7) |= 0x80;
            const nfs::u32 texture = reg8At(edx + 6) & 0x3f;
            reg32At(0x7a3d08) = edx;
            reg32At(edx + 0x18) = reg32At(object + texture * 4 + 0x1e04);
            nfs::u32 colour;
            if (!(reg8At(edx + 7) & 2))
            {
                reg32At(edx + 0x40) = reg32At(reg32At(edx + 8) + 0x10);
                reg32At(edx + 0x44) = reg32At(reg32At(edx + 0xc) + 0x10);
                colour = reg32At(reg32At(edx + 0x10) + 0x10);
            }
            else
            {
                colour = reg32At(0x55e6e8);
                reg32At(edx + 0x40) = colour;
                reg32At(edx + 0x44) = colour;
            }
            reg32At(edx + 0x48) = colour;
            if (flag == 0)
            {
                reg8At(edx + 7) &= 0xbf;
            }
            else if (!(reg8At(edx + 7) & 4))
            {
                reg8At(edx + 7) |= 0x40;
                // fld / fstp dword: a float through the FPU and back
                auto copy = [&](nfs::u32 to, nfs::u32 vertex, nfs::u32 at) {
                    app->getMemory<float>(to) = float(double(app->getMemory<float>(reg32At(edx + vertex) + at)));
                };
                copy(edx + 0x54, 8, 0x18);
                copy(edx + 0x58, 8, 0x1c);
                copy(edx + 0x5c, 0xc, 0x18);
                copy(edx + 0x60, 0xc, 0x1c);
                copy(edx + 0x64, 0x10, 0x18);
                copy(edx + 0x68, 0x10, 0x1c);
                reg32At(edx + 0x50) = flag;
                const nfs::u32 shade = reg32At((reg8At(edx + 7) & 8) ? 0x55e6e0 : 0x55e6dc);
                reg32At(edx + 0x74) = shade;
                reg32At(edx + 0x78) = shade;
                reg32At(edx + 0x7c) = shade;
            }
        }
        current = next;
        polygon = ahead;
    }
}

nfs::u32 cullPolygonsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    cullPolygonsOf(app, ts, args.eax, args.edx, args.ebx);
    ts.sp += 4;
    return args.eax;
}

/* The depth for Glide of a 1/w whose bits are `w` (taken as 0 when negative or
 * past 1), as the drawing works it out: 1 - w, times [0x560094] (`scale`),
 * plus [0x560098] (`offset`), each step through a float in memory.  `t` is
 * the first step's float, `depth` the last. */
struct DepthSteps
{
    float t;
    float depth;
};

inline DepthSteps depthSteps(const SingleMath& M, nfs::u32 w, float scale, float offset)
{
    float t = 0.0f;
    if (!(w & 0x80000000) && nfs::s32(w) <= 0x3f800000)
        t = float(M.sub(1.0, asFloat(w)));
    const float scaled = float(M.mul(scale, t));
    return { t, float(M.add(offset, scaled)) };
}

/* A vertex's depth for Glide from its 1/z, as sub_434380 (and the clipper)
 * write it over z (depthSteps). */
void polygonDepth(nfs::App* app, const SingleMath& M, nfs::u32 vertex, float scale, float offset)
{
    app->getMemory<float>(vertex + 8) = depthSteps(M, app->getMemory<nfs::u32>(vertex + 0xc), scale, offset).depth;
}

/* polygonDepth of each vertex, with [0x560094] and [0x560098] as the drawing reads them. */
void cornerDepths(App* app, std::initializer_list<u32> vertices)
{
    const float scale = app->getMemory<float>(nfs::u32(0x560094));
    const float offset = app->getMemory<float>(nfs::u32(0x560098));
    for (const u32 vertex : vertices)
        polygonDepth(app, SingleMath(), vertex, scale, offset);
}

/* sub_434380: the frame's polygons to the renderer, in the order of their list
 *   eax  the first; each polygon's first dword is the next, the word at +4 its
 *   kind, and the list goes on while that is 3; its vertices at +8, +0xc, +0x10
 * Two passes: every polygon marked 0x80 in byte 7, with the colours at +0x40
 * and, when it has a texture at +0x18, the coordinates at +0x20; then every one
 * marked 0x40 -- a second, blended layer; the mark is cleared -- with +0x74,
 * +0x50 and +0x54.  Blending ([0x554e58], state 0x68) and the texture
 * ([0x554e54], state 1) go through THRASH_setstate ([0x9ef96c]) when they
 * change.  A polygon with a vertex off the screen goes to the clipper
 * (sub_4c11b0); the rest get their depth (polygonDepth) and go to
 * THRASH_drawtri ([0x9ef974]).  Returns the polygon the list ended on.
 * Entered at `esp`; its frame below six pushes and 0x18 bytes. */
u32 submitPolygonsOf(App* app, ThreadState& ts, u32 esp, u32 first)
{
    auto reg32At = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto reg8At = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const u32 base = esp - 24 - 0x18;  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18

    nfs::u32 polygon = first;
    for (int pass = 0; pass < 2; ++pass)
    {
        const nfs::u8 mark = pass == 0 ? 0x80 : 0x40;
        const nfs::u32 blend = pass == 0 ? 0 : 1;
        const nfs::u32 colours = pass == 0 ? 0x40 : 0x74;
        const nfs::u32 texturing = pass == 0 ? 0x18 : 0x50;
        const nfs::u32 coordinates = pass == 0 ? 0x20 : 0x54;
        polygon = first;
        while (true)
        {
            const nfs::u8 marks = reg8At(polygon + 7);
            if (marks & mark)
            {
                if (pass == 1)
                    reg8At(polygon + 7) = nfs::u8(marks & 0xbf);
                if (reg32At(0x554e58) != blend)
                {
                    reg32At(0x554e58) = blend;
                    if (!driverAt<thrash::kThrashSetState, &port::driver::setState>(app, ts,
                        base, 0x68, blend).has_value())
                        return 0;
                }
                for (int i = 0; i < 3; ++i)
                    reg32At(reg32At(polygon + 8 + 4 * i) + 0x10) = reg32At(polygon + colours + 4 * i);
                const nfs::u32 texture = reg32At(polygon + texturing);
                if (texture != 0)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        reg32At(reg32At(polygon + 8 + 4 * i) + 0x18) = reg32At(polygon + coordinates + 8 * i);
                        reg32At(reg32At(polygon + 8 + 4 * i) + 0x1c) = reg32At(polygon + coordinates + 8 * i + 4);
                    }
                }
                if (reg32At(0x554e54) != texture)
                {
                    reg32At(0x554e54) = texture;
                    if (!driverAt<thrash::kThrashSetState, &port::driver::setState>(app, ts,
                        base, 1, texture).has_value())
                        return 0;
                }
                const nfs::u32 corners[3] = { reg32At(polygon + 8), reg32At(polygon + 0xc), reg32At(polygon + 0x10) };
                if (reg32At(corners[0] + 0x14) != 0 || reg32At(corners[1] + 0x14) != 0
                    || reg32At(corners[2] + 0x14) != 0)
                {
                    clipTriangleOf(app, ts, base - 4, corners[0], corners[1], corners[2]);
                    if (ts.terminate)
                        return 0;
                }
                else
                {
                    cornerDepths(app, { corners[0], corners[1], corners[2] });
                    if (!driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, ts,
                        base, corners[0], corners[1], corners[2]).has_value())
                        return 0;
                }
            }
            polygon = reg32At(polygon);
            if (polygon == 0 || app->getMemory<nfs::u16>(polygon + 4) != 3)
                break;
        }
    }
    return polygon;
}

nfs::u32 submitPolygonsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = submitPolygonsOf(app, ts, ts.sp, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_41ae90: one piece of track's polygons -- four corners each -- drawn.
 *   eax  the view, its mode at +0; edx  handed to the drawing; ebx, ecx  the
 *   piece: its polygon count at 0x571370 + ebx*0x5c0 + ecx*4, the polygons,
 *   14 bytes each, 0x2c further on
 *   [esp+4]  the vertex records, 0x1c bytes each; [esp+8], [esp+0xc]  the
 *   drawn list's tail and head
 * A polygon is dropped when its first corner is past the far plane
 * ([[0x5dd830]+0x30]), when all four corners share a clip bit, or when both of
 * its triangles face away (unless it is two-sided or a corner is behind the
 * view).  The rest are drawn as two triangles (sub_41a360, into a buffer taken
 * from the frame's arena by sub_4bbe40) when the view draws track and the
 * texture is not a special one; otherwise each corner only raises its
 * vertex's colour to the record's.  Returns how many triangles went out.
 *
 * Entered at `esp`; only the arena slots the callees are pointed at live in
 * the guest frame (three pushes, then 0x48 bytes), the rest stays here. */
u32 trackPolygonsOf(App* app, ThreadState& ts, u32 esp, u32 view, u32 pieceRow, u32 pieceColumn, u32 records,
                      u32 tail, u32 head)
{
    auto reg32At = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto reg16At = [app](nfs::u32 address) { return app->getMemory<nfs::u16>(address); };
    auto reg8At = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 ebp = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    const nfs::u32 base = ebp - 0x48;
    const nfs::u32 arena = ebp - 0x48;
    const nfs::u32 cursor = ebp - 0x40;
    const nfs::u32 piece = pieceRow * 0x5c0 + 0x571370;
    const nfs::u32 count = reg32At(piece + pieceColumn * 4);
    nfs::u32 polygon = reg32At(piece + pieceColumn * 4 + 0x2c);

    if (!coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, arena))
        return 0;
    reg32At(cursor) = reg32At(arena);
    const auto room = coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, ebp - 0x3c);
    if (!room)
        return 0;
    // no room in the arena, nothing drawn
    const nfs::s32 polygons = *room != 0 ? nfs::s32(count) : 0;

    nfs::u32 total = 0;
    // A corner whose record is on the screen raises its vertex's colour.
    auto raise = [&](nfs::u32 record) {
        if (reg16At(record + 4) != 0)
            return;
        const nfs::u32 vertex = reg32At(record + 8);
        for (const nfs::u32 at : { 0x16u, 0x15u, 0x14u })
        {
            const nfs::u8 own = reg8At(record + at);
            const nfs::u8 vertexByte = reg8At(vertex + at - 4);
            reg8At(vertex + at - 4) = own > vertexByte ? own : vertexByte;
        }
    };

    for (nfs::s32 i = 0; i < polygons; ++i, polygon += 0xe)
    {
        const nfs::u32 rec0 = records + nfs::u32(nfs::s32(nfs::s16(reg16At(polygon)))) * 0x1c;
        const nfs::u32 rec1 = records + nfs::u32(nfs::s32(reg32At(polygon)) >> 16) * 0x1c;
        const nfs::u32 rec2 = records + nfs::u32(nfs::s32(reg32At(polygon + 2)) >> 16) * 0x1c;
        const nfs::u32 rec3 = records + nfs::u32(nfs::s32(reg32At(polygon + 4)) >> 16) * 0x1c;
        const nfs::u8 flags = reg8At(polygon + 0xc);
        const nfs::u32 t0 = reg32At(rec0 + 8);
        const double farthest = floatAt(reg32At(0x5dd830) + 0x30);
        const double z0 = floatAt(t0 + 8);
        if (z0 > farthest)
            continue;

        const nfs::u32 c0 = reg8At(t0 + 0x14);
        const nfs::u32 t1 = reg32At(rec1 + 8);
        const nfs::u32 c1 = reg8At(t1 + 0x14);
        const nfs::u32 t2 = reg32At(rec2 + 8);
        const nfs::u32 c2 = reg8At(t2 + 0x14);
        const nfs::u32 c3 = reg8At(reg32At(rec3 + 8) + 0x14);
        if (c3 & c0 & c1 & c2)
            continue;
        if (!(flags & 0x10) && !((c0 | c1 | c2 | c3) & 0x10))
        {
            const CrossProducts p = crossProducts(ts, floatAt(t2 + 4), floatAt(t1 + 4), floatAt(t0), floatAt(t1),
                                                  floatAt(t0 + 4), floatAt(t1 + 4), floatAt(t2), floatAt(t1));
            const nfs::u32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
            if (back ^ reg32At(0x554e4c))
            {
                const nfs::u32 t3 = reg32At(rec3 + 8);
                const nfs::u32 t2again = reg32At(rec2 + 8);
                const nfs::u32 t0again = reg32At(rec0 + 8);
                const CrossProducts q = crossProducts(ts, floatAt(t3 + 4), floatAt(t2again + 4), floatAt(t0again),
                                                      floatAt(t2again), floatAt(t0again + 4), floatAt(t2again + 4),
                                                      floatAt(t3), floatAt(t2again));
                const nfs::u32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                if (backToo ^ reg32At(0x554e4c))
                    continue;
            }
        }

        bool draw = false;
        if (mirrorAsMain(reg32At(view)) != 1)
        {
            const nfs::u32 texture = nfs::u32(nfs::s32(reg32At(polygon + 6)) >> 16);
            draw = !(reg8At(reg32At(0x552df4) + texture * 47 + 0x28) & 4) && gameSettings(app).detailFlagFc == 1
                   && reg32At(0x7a3a60) != 0;
        }
        if (draw)
        {
            total += projectedTriangleOf(app, cursor, rec0, rec1, rec2);
            total += projectedTriangleOf(app, cursor, rec0, rec2, rec3);
            continue;
        }
        for (const nfs::u32 record : { rec0, rec1, rec2, rec3 })
            raise(record);
    }

    // The arena gets what went out, and the list its new end.
    app->getMemory<u32>(cursor) = reg32At(tail);
    if (total != 0)
    {
        if (!commitArena(app, ts, base, ebp, arena, total * 0xa0))
            return 0;
        const nfs::u32 last = reg32At(cursor);
        app->getMemory<u32>(last != 0 ? last : head) = reg32At(arena);
        if (!coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, cursor))
            return 0;
        app->getMemory<u32>(cursor) = reg32At(cursor) - 0xa0;
    }
    app->getMemory<u32>(tail) = reg32At(cursor);
    return total;
}

nfs::u32 trackPolygonsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    const u32 total = trackPolygonsOf(app, ts, esp, args.eax, args.ebx, args.ecx, app->getMemory<u32>(esp + 4),
                                        app->getMemory<u32>(esp + 8), app->getMemory<u32>(esp + 12));
    if (ts.terminate)
        return args.eax;
    args.eax = total;
    ts.sp = esp + 4 + 0xc;
    return args.eax;
}

/* An x87 "indefinite" of either sign: a corner the clipper will not draw. */
inline bool indefinite(nfs::u32 bits)
{
    return (bits | 0x80000000) == 0xffc00000;
}

/* sub_4c2cd0: a triangle clipped to the view and drawn as a fan.
 *   eax  its three vertices (pointers to the game's records: x, y at +0, +4,
 *        w at +0xc, colour at +0x10, s, t at +0x18, +0x1c); bl  their clip
 *        codes or-ed together; edx  the three codes
 * The corners go to work records at 0x7d3570, nine floats each -- x, y, w,
 * s*w, t*w, then a, r, g, b -- and are clipped Sutherland-Hodgman fashion
 * against the near plane (code 0x10, w against [0x7d34f0]), the top and the
 * bottom (4, 8: y against [0x7d34dc], [0x7d34e0]) and the left and the right
 * (1, 2: x against [0x7d34f4], [0x7d34ec]), each only when a code has its bit.
 * Every pass lists its polygon's pointers and codes after the last one's
 * (0x7d0e78, 0x7d0db0); the vertices it makes are records at 0x7d1198.  Each
 * made vertex is a - (a - b) * t of its edge's two in every field; t as the
 * original has it, a float in memory for the near plane, kept on the x87 for
 * the rest.  The polygon left becomes Glide vertices at 0x7cdbb0 -- x, y,
 * depth, w, colour packed again, s/w-ish, t/w-ish as THRASH wants them -- and
 * goes to THRASH_drawtrifan ([0x9ef960]).  Returns -1 when a pass leaves
 * fewer than three corners, 0 otherwise (also when the first corner is an
 * indefinite and nothing is drawn).  `esp` is where it is entered (its frame:
 * four pushes, then 0xc8 bytes).
 *
 * The arithmetic is the emulated FPU's (SingleMath), in whatever precision the
 * game has it at, every float stored where the original stores one. */
u32 clipPolygonOf(App* app, ThreadState& ts, u32 esp, u32 corners, u32 cornerCodes, nfs::u8 clipCodes)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto load = [app](nfs::u32 address) { return double(app->getMemory<float>(address)); };
    auto store = [app](nfs::u32 address, double value) { app->getMemory<float>(address) = float(value); };

    {
        // Every field read before any is written.
        float fields[3][9];
        for (int i = 0; i < 3; ++i)
        {
            const nfs::u32 from = word(corners + 4 * i);
            const double w = load(from + 0xc);
            const nfs::u32 colour = word(from + 0x10);
            fields[i][0] = app->getMemory<float>(from);
            fields[i][1] = app->getMemory<float>(from + 4);
            fields[i][2] = float(w);
            fields[i][3] = float(M.mul(load(from + 0x18), w));
            fields[i][4] = float(M.mul(load(from + 0x1c), w));
            fields[i][5] = float(colour >> 24);
            fields[i][6] = float(colour >> 16 & 0xff);
            fields[i][7] = float(colour >> 8 & 0xff);
            fields[i][8] = float(colour & 0xff);
        }
        std::memcpy(&app->getMemory<nfs::u8>(0x7d3570), fields, sizeof fields);
    }

    nfs::u32 list = 0x5600d4;    // the corners' pointers, the work records'
    nfs::u32 codes = cornerCodes; // their clip codes
    nfs::u32 outList = 0x7d0e78;
    nfs::u32 outCodes = 0x7d0db0;
    nfs::u32 count = 3;
    nfs::u32 made = 0;

    const double nearW = load(0x7d34f0);
    const double top = load(0x7d34dc);
    const double bottom = load(0x7d34e0);
    const double left = load(0x7d34f4);
    const double right = load(0x7d34ec);
    auto codeX = [&](nfs::u32 vertex, nfs::u32 code) {
        const double x = load(vertex);
        if (!(x >= left))
            byte(code) |= 1;
        else if (x > right)
            byte(code) |= 2;
    };
    auto between = [&](nfs::u32 a, nfs::u32 b, nfs::u32 made, nfs::u32 field, double t) {
        const double from = load(a + field);
        store(made + field, M.sub(from, M.mul(M.sub(from, load(b + field)), t)));
    };

    /* One pass: the corners outside `bit` dropped, a vertex made where an edge
     * crosses it.  False when fewer than three are left; the lists move on. */
    auto pass = [&](nfs::u8 bit, auto&& makeVertex) -> bool {
        nfs::u32 kept = 0;
        nfs::u32 previous = count - 1;
        for (nfs::u32 current = 0; current < count; previous = current++)
        {
            const nfs::u8 previousCode = byte(codes + previous);
            if (!(previousCode & bit))
            {
                word(outList + 4 * kept) = word(list + 4 * previous);
                byte(outCodes + kept) = previousCode;
                ++kept;
            }
            if ((previousCode ^ byte(codes + current)) & bit)
            {
                const nfs::u32 vertex = 0x7d1198 + made * 0x24;
                makeVertex(word(list + 4 * previous), word(list + 4 * current), vertex, outCodes + kept, previousCode);
                word(outList + 4 * kept) = vertex;
                ++made;
                ++kept;
            }
        }
        if (kept < 3)
            return false;
        codes = outCodes;
        outCodes += kept;
        list = outList;
        outList += 4 * kept;
        count = kept;
        return true;
    };
    auto orCodes = [&]() {
        clipCodes = 0;
        for (nfs::u32 i = 0; i < count; ++i)
            clipCodes |= byte(codes + i);
    };

    if (clipCodes & 0x10)
    {
        const bool kept = pass(0x10, [&](nfs::u32 a, nfs::u32 b, nfs::u32 vertex, nfs::u32 code, nfs::u8) {
            const double aW = load(a + 8);
            const double inverse = double(float(M.div(1.0, M.sub(aW, load(b + 8)))));
            double t = double(float(M.mul(M.sub(aW, nearW), inverse)));
            for (nfs::u32 field : { 0u, 4u, 0xcu, 0x10u })
                between(a, b, vertex, field, t);
            if (nfs::s32(asBits(float(t))) > 0x3f800000 || 0.0 > t)
                t = double(float(M.mul(aW, inverse)));
            for (nfs::u32 field : { 0x14u, 0x18u, 0x1cu, 0x20u })
                between(a, b, vertex, field, t);
            word(vertex + 8) = word(0x7d34f0);
            byte(code) = 0x10;
            const double y = load(vertex + 4);
            if (!(y >= bottom))
                byte(code) |= 8;
            else if (y > top)
                byte(code) |= 4;
            codeX(vertex, code);
        });
        if (!kept)
            return 0xffffffff;
        orCodes();
    }

    if (clipCodes & 0xc)
    {
        for (const nfs::u8 bit : { nfs::u8(4), nfs::u8(8) })
        {
            if (!(clipCodes & bit))
                continue;
            const double edge = bit == 4 ? top : bottom;
            const nfs::u32 edgeBits = word(bit == 4 ? 0x7d34dc : 0x7d34e0);
            const bool kept = pass(bit, [&](nfs::u32 a, nfs::u32 b, nfs::u32 vertex, nfs::u32 code,
                                            nfs::u8 previousCode) {
                const double aY = load(a + 4);
                const double t = M.div(M.sub(aY, edge), M.sub(aY, load(b + 4)));
                for (nfs::u32 field : { 0u, 8u, 0xcu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u })
                    between(a, b, vertex, field, t);
                word(vertex + 4) = edgeBits;
                byte(code) = nfs::u8((previousCode & 0xf0) | bit);
                codeX(vertex, code);
            });
            if (!kept)
                return 0xffffffff;
        }
        orCodes();
    }

    if (clipCodes & 3)
    {
        for (const nfs::u8 bit : { nfs::u8(1), nfs::u8(2) })
        {
            if (!(clipCodes & bit))
                continue;
            const double edge = bit == 1 ? left : right;
            const nfs::u32 edgeBits = word(bit == 1 ? 0x7d34f4 : 0x7d34ec);
            const bool kept = pass(bit, [&](nfs::u32 a, nfs::u32 b, nfs::u32 vertex, nfs::u32 code,
                                            nfs::u8 previousCode) {
                const double aX = load(a);
                const double t = M.div(M.sub(aX, edge), M.sub(aX, load(b)));
                for (nfs::u32 field : { 4u, 8u, 0xcu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u })
                    between(a, b, vertex, field, t);
                word(vertex) = edgeBits;
                byte(code) = nfs::u8((previousCode & 0xfc) | bit);
            });
            if (!kept)
                return 0xffffffff;
        }
    }

    // A first corner with an indefinite x, y or w: nothing drawn.
    for (nfs::u32 field : { 0u, 4u, 8u })
        if (indefinite(word(word(list) + field)))
            return 0;

    const double scale = load(0x560094);
    const double offset = load(0x560098);
    nfs::u32 fan = 0x7cdbb0;
    nfs::u32 drawn = 0;
    for (nfs::u32 i = 0; i < count; ++i)
    {
        const nfs::u32 vertex = word(list + 4 * i);
        const double w = load(vertex + 8);
        const double inverse = double(float(M.div(1.0, w)));
        const nfs::u32 wBits = word(vertex + 8);
        float depth = 0.0f;
        if (!(wBits & 0x80000000) && nfs::s32(wBits) <= 0x3f800000)
            depth = float(M.sub(1.0, w));
        word(fan) = word(vertex);
        word(fan + 4) = word(vertex + 4);
        const float scaled = float(M.mul(scale, depth));
        store(fan + 8, M.add(offset, scaled));
        store(fan + 0xc, w);
        auto channel = [&](nfs::u32 field) {
            return nfs::u32(fp::toInteger<nfs::s32>(nfs::Float(load(vertex + field)))) & 0xff;
        };
        word(fan + 0x10) = channel(0x14) << 24 | channel(0x18) << 16 | channel(0x1c) << 8 | channel(0x20);
        word(fan + 0x14) = 0;
        store(fan + 0x18, M.mul(load(vertex + 0xc), inverse));
        store(fan + 0x1c, M.mul(load(vertex + 0x10), inverse));
        if (!indefinite(word(fan)) && !indefinite(word(fan + 4)) && !indefinite(word(fan + 8))
            && !indefinite(word(fan + 0xc)))
        {
            fan += 0x20;
            ++drawn;
        }
    }

    // From its frame below the four pushes.
    driverAt<thrash::kThrashDrawTriFan, &port::driver::drawTriFan>(app, ts, esp - 16 - 0xc8, drawn - 2, 0x7cdbb0);
    return 0;
}

/* The thunk of a clipper's entry: eax the corners, edx their codes, bl the codes or-ed. */
nfs::u32 clipPolygonNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = clipPolygonOf(app, ts, ts.sp, args.eax, args.edx, nfs::u8(args.ebx));
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* The first indefinite (either sign) among x, y, depth, w, u, v of the
 * vertices, as or eax, 0x80000000 leaves it; 0 if none. */
nfs::u32 firstIndefinite(nfs::App* app, std::initializer_list<nfs::u32> vertices)
{
    for (nfs::u32 vertex : vertices)
        for (nfs::u32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
        {
            const nfs::u32 value = app->getMemory<nfs::u32>(vertex + field) | 0x80000000;
            if (value == 0xffc00000)
                return value;
        }
    return 0;
}

/* sub_4c11b0: a triangle (eax, edx, ebx) to the screen.  Nothing when its
 * corners share a clip bit (the byte at +0x14); through the clipper
 * (sub_4c2cd0) when one of them has one; otherwise each gets its depth
 * (polygonDepth) and the three go to THRASH_drawtri ([0x9ef974]).  Returns
 * what the code leaves in eax: the shared bits over a's low byte, else the
 * callee's.  Entered at `esp`; its frame below four pushes. */
u32 clipTriangleOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const nfs::u8 codes[3] = { byte(a + 0x14), byte(b + 0x14), byte(c + 0x14) };
    const nfs::u32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    for (int i = 0; i < 3; ++i)
        byte(frame - 4 + nfs::u32(i)) = codes[i];

    const nfs::u8 shared = codes[0] & codes[1] & codes[2];
    if (shared)
        return (a & 0xffffff00) | shared;
    const nfs::u8 any = codes[0] | codes[1] | codes[2];
    if (any)
    {
        word(frame - 0x1c) = a;
        word(frame - 0x18) = b;
        word(frame - 0x14) = c;
        return clipPolygonOf(app, ts, frame - 0x1c - 4, frame - 0x1c, frame - 4, any);
    }
    cornerDepths(app, { a, b, c });
    return driverKeeping<thrash::kThrashDrawTri, &port::driver::drawTri>(app, ts, frame - 0x1c, a, a, b, c).value_or(0);
}

nfs::u32 clipTriangleNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = clipTriangleOf(app, ts, ts.sp, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_4c12e0: a quad (eax, edx, ebx, ecx: a, b, c, d) to the screen.  Nothing
 * when the four share a clip bit; THRASH_drawquad ([0x9ef94c]) with their
 * depths when none has one; otherwise triangles a b c and a c d, each skipped
 * when its corners share a bit, clipped when one has one, else drawn with
 * THRASH_drawtri unless a field of a corner is an indefinite.  Returns what
 * the code leaves in eax: the codes it and-ed and or-ed in its low bytes, the
 * indefinite it found, or the last callee's.  Entered at `esp`; its frame
 * below three pushes. */
u32 clipQuadOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c, u32 d)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const nfs::u8 ca = byte(a + 0x14), cb = byte(b + 0x14), cc = byte(c + 0x14), cd = byte(d + 0x14);
    const nfs::u32 frame = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    byte(frame - 4) = ca;
    byte(frame - 3) = cb;
    byte(frame - 2) = cc;
    byte(frame - 1) = cd;

    const nfs::u8 three = ca & cb & cc;
    const nfs::u8 all = three & cd;
    if (all)
        return (a & 0xffff0000) | nfs::u32(three) << 8 | all;
    const nfs::u8 first = ca | cb | cc;
    const nfs::u8 any = first | cd;
    if (!any)
    {
        cornerDepths(app, { a, b, c, d });
        return driverKeeping<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, ts,
            frame - 0x3c, a, a, b, c, d).value_or(0);
    }

    // The first triangle, a b c.
    nfs::u32 eax = (a & 0xffff0000) | nfs::u32(three) << 8 | first;
    if (!three)
    {
        if (first)
        {
            word(frame - 0x3c) = a;
            word(frame - 0x38) = b;
            word(frame - 0x34) = c;
            eax = clipPolygonOf(app, ts, frame - 0x3c - 4, frame - 0x3c, frame - 4, first);
        }
        else
        {
            cornerDepths(app, { a, b, c });
            eax = firstIndefinite(app, { a, b, c });
            if (!eax)
                eax = driverKeeping<thrash::kThrashDrawTri, &port::driver::drawTri>(app, ts,
                    frame - 0x3c, a, a, b, c).value_or(0);
        }
        if (ts.terminate)
            return 0;
    }

    // The second, a c d.
    const nfs::u8 shared = ca & cc & cd;
    if (shared)
        return (eax & 0xffff0000) | nfs::u32(cc) << 8 | shared;
    const nfs::u8 second = ca | cc | cd;
    if (second)
    {
        byte(frame - 3) = cc;
        byte(frame - 2) = cd;
        word(frame - 0x3c) = a;
        word(frame - 0x38) = c;
        word(frame - 0x34) = d;
        return clipPolygonOf(app, ts, frame - 0x3c - 4, frame - 0x3c, frame - 4, second);
    }
    cornerDepths(app, { a, c, d });
    if (const nfs::u32 found = firstIndefinite(app, { a, c, d }))
        return found;
    return driverKeeping<thrash::kThrashDrawTri, &port::driver::drawTri>(app, ts, frame - 0x3c, a, a, c, d).value_or(0);
}

nfs::u32 clipQuadNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = clipQuadOf(app, ts, ts.sp, args.eax, args.edx, args.ebx, args.ecx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* A corner of sub_4c1aa0's triangle, which carries z at +8 rather than a
 * depth: the depth of 1/z written there as polygonDepth works it out (1/z
 * taken as 65535 for a z of 0), the clip code's dword at +0x14 cleared, and
 * 1/w at +0xc held to 0..1. */
void zCornerDepth(nfs::App* app, const SingleMath& M, nfs::u32 vertex, float scale, float offset)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 z = word(vertex + 8);
    // 1/z through [ebp-0xc], a float
    const float inverse = (z & 0x7fffffff) ? float(M.div(1.0, asFloat(z))) : asFloat(0x477fff00);
    word(vertex + 0x14) = 0;
    app->getMemory<float>(vertex + 8) = depthSteps(M, asBits(inverse), scale, offset).depth;
    const nfs::u32 w = word(vertex + 0xc);
    const float oow = asFloat(w);
    if (0.0f > oow)
        word(vertex + 0xc) = 0;
    else if (nfs::s32(w) > 0x3f800000)
        word(vertex + 0xc) = 0x3f800000;
}

/* sub_4c1aa0: a triangle (eax, edx, ebx) whose corners carry z, to the
 * screen -- as sub_4c11b0, but each corner drawn gets its depth from 1/z
 * (zCornerDepth), and the clipping goes to sub_4bf790.  Nothing when an
 * x, y, depth, 1/w, u or v of a corner is an indefinite.  Entered at `esp`;
 * its frame below four pushes. */
void clipTriangleByZOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const nfs::u8 codes[3] = { byte(a + 0x14), byte(b + 0x14), byte(c + 0x14) };
    const nfs::u32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    for (int i = 0; i < 3; ++i)
        byte(frame - 4 + nfs::u32(i)) = codes[i];

    if (codes[0] & codes[1] & codes[2])
        return;
    const nfs::u8 any = codes[0] | codes[1] | codes[2];
    if (any)
    {
        word(frame - 0x4c) = a;
        word(frame - 0x48) = b;
        word(frame - 0x44) = c;
        coreAt<0x4bf790, render::clippedByZ>(app, ts, frame - 0x4c, frame - 0x4c, frame - 4, any);
        return;
    }
    const float scale = app->getMemory<float>(nfs::u32(0x560094));
    const float offset = app->getMemory<float>(nfs::u32(0x560098));
    for (const u32 corner : { a, b, c })
        zCornerDepth(app, SingleMath(), corner, scale, offset);
    if (firstIndefinite(app, { a, b, c }))
        return;
    driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, ts, frame - 0x4c, a, b, c);
}

nfs::u32 clipTriangleByZNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    clipTriangleByZOf(app, ts, ts.sp, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* sub_4c1e20: a mesh of `count` quads (a car's) to the screen: `vertices`, 0x20 bytes each, and
 * the quads at `quads`, four vertex numbers each.  Nothing when every corner of every quad shares a
 * clip bit; when none has one, every corner gets its depth (polygonDepth, inline) and the mesh goes
 * whole to THRASH_drawquadmesh ([0x9ef930]); otherwise each quad to sub_4c12e0.  Returns what the
 * code leaves in eax: past the quads looked at, the quads drawn, or THRASH's.  Entered at `esp`;
 * its frame below four pushes. */
u32 quadMeshOf(App* app, ThreadState& ts, u32 esp, nfs::s32 count, u32 vertices, u32 quads)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const nfs::u32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    auto corner = [&](nfs::u32 quad, nfs::u32 i) { return vertices + (word(quad + i * 4) << 5); };
    // The codes of the first quad, then of the rest: or and and over all.
    nfs::u8 any = 0;
    nfs::u8 all = 0xff;
    nfs::s32 n = 0;
    do
    {
        const nfs::u32 quad = quads + nfs::u32(n) * 0x10;
        for (nfs::u32 i = 0; i < 4; ++i)
        {
            const nfs::u8 code = byte(corner(quad, i) + 0x14);
            any |= code;
            all &= code;
        }
        ++n;
    } while (n < count);
    if (all)
        return quads + nfs::u32(n) * 0x10;
    if (any)
    {
        nfs::s32 i = 0;
        for (; i < count; ++i)
        {
            const nfs::u32 quad = quads + nfs::u32(i) * 0x10;
            clipQuadOf(app, ts, frame - 0x30 - 4, corner(quad, 0), corner(quad, 1), corner(quad, 2), corner(quad, 3));
            if (ts.terminate)
                return 0;
        }
        return nfs::u32(i);
    }
    const float scale = app->getMemory<float>(nfs::u32(0x560094));
    const float offset = app->getMemory<float>(nfs::u32(0x560098));
    for (nfs::s32 i = 0; i < count; ++i)
    {
        const nfs::u32 quad = quads + nfs::u32(i) * 0x10;
        for (nfs::u32 k = 0; k < 4; ++k)
            polygonDepth(app, SingleMath(), corner(quad, k), scale, offset);
    }
    return driverKeeping<thrash::kThrashDrawQuadMesh, &port::driver::drawQuadMesh>(app, ts,
        frame - 0x30, nfs::u32(count), nfs::u32(count), vertices, quads).value_or(0);
}

nfs::u32 quadMeshNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = quadMeshOf(app, ts, ts.sp, nfs::s32(args.eax), args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_41a550: trackVerticesOf for the view eax, edx vertices at ebx into ecx, the records from
 * [esp+4] (ret 4). */
nfs::u32 trackVerticesNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    trackVerticesOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, app->getMemory<u32>(ts.sp + 4));
    ts.sp += 4 + 4;
    return args.eax;
}

/* objectVertexList as floats at single precision to nearest (`asFloats`), and
 * once more through the emulated FPU's operations should a value leave a
 * float's range on the way: it writes only what it works out from what it
 * reads.  In any other mode the FPU's way at once. */
void objectVerticesExactly(App* app, bool asFloats, i32 count, u32 in, u32 matrix, u32 position, u32 out)
{
    if (asFloats)
    {
        std::feclearexcept(kOutOfRange);
        objectVertexList(app, FloatMath(), count, in, matrix, position, out);
        if (!std::fetestexcept(kOutOfRange))
            return;
    }
    objectVertexList(app, SingleMath(), count, in, matrix, position, out);
}

/* sub_4bf4c0: objectVerticesOf eax vertices at edx by ebx and ecx into [esp+4] (ret 4); eax what
 * the last vertex's clip code leaves (objectClipLeft), the count for none. */
nfs::u32 objectVerticesNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 out = app->getMemory<u32>(ts.sp + 4);
    objectVerticesOf(app, ts, count, args.edx, args.ebx, args.ecx, out);
    if (count > 0)
        args.eax = objectClipLeft(app, out + u32(count - 1) * 0x20);
    ts.sp += 4 + 4;
    return args.eax;
}

}  // namespace

/* The cores of 0x4c12e0, 0x4c11b0 and 0x4c1e20 as C++ for the menus' code (native_vertices.h): the quad, the
 * triangle and the mesh clipped and drawn; `esp` the frame base the thunk hands them. */
u32 quadClippedAt(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c, u32 d)
{
    return clipQuadOf(app, ts, esp, a, b, c, d);
}

u32 triangleClippedAt(App* app, ThreadState& ts, u32 esp, u32 a, u32 b, u32 c)
{
    return clipTriangleOf(app, ts, esp, a, b, c);
}

u32 quadMeshDrawn(App* app, ThreadState& ts, u32 esp, nfs::s32 count, u32 vertices, u32 quads)
{
    return quadMeshOf(app, ts, esp, count, vertices, quads);
}

/* sub_4bf4c0's work as its thunk does it: objectVerticesExactly of `count` vertices at `in` by
 * `matrix` and `position` into `out`, as floats at single precision to nearest; nothing for a
 * count under 1. */
void objectVerticesOf(App* app, ThreadState& ts, i32 count, u32 in, u32 matrix, u32 position, u32 out)
{
    if (count > 0)
        objectVerticesExactly(app, singleNearest(ts), count, in, matrix, position, out);
}

/* sub_4e03b0's work: rotateVectorList of `count` vectors at `in` by `matrix` into `out` as floats
 * at single precision to nearest, once more through the emulated FPU's operations should a value
 * leave a float's range (runNative's way).  Vectors turned where they are (or with the output
 * across the input) could not be turned again from what they were: one run, the FPU's -- and
 * in any other mode. */
void turnVectors(App* app, ThreadState& ts, i32 count, u32 in, u32 matrix, u32 out)
{
    const u32 span = count > 0 ? u32(count) * 0xc : 0;
    const bool inPlace = span != 0 && out - in + span - 1 < 2 * span - 1;
    if (singleNearest(ts) && !inPlace)
    {
        std::feclearexcept(kOutOfRange);
        rotateVectorList(app, FloatMath(), count, in, matrix, out);
        if (!std::fetestexcept(kOutOfRange))
            return;
    }
    rotateVectorList(app, SingleMath(), count, in, matrix, out);
}

namespace
{

/* sub_4e03b0: turnVectors of eax vectors at edx by ebx into ecx; eax past the vectors read (where
 * they start for none). */
nfs::u32 rotateVectorsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 in = args.edx;
    turnVectors(app, ts, count, in, args.ebx, args.ecx);
    args.eax = count > 0 ? in + u32(count) * 0xc : in;
    ts.sp += 4;
    return args.eax;
}

/* sub_4e0210: dot products, out[i] = v[i] . d.
 *   eax  count; edx  the vectors, 12 bytes each; ebx  d; ecx  out, a float each
 * Called with long lists: a twenty-fifth of a race's game thread (2026-10-03). */
template <typename Math>
void dotProductList(App* app, const Math& math, i32 count, u32 in, u32 d, u32 out)
{
    for (i32 k = 0; k < count; ++k, in += 0xc, out += 4)
    {
        const float* v = &app->getMemory<float>(in);
        const float* w = &app->getMemory<float>(d);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        app->getMemory<float>(out) = float(math.add(math.add(math.mul(v0, w[0]), math.mul(v1, w[1])),
                                                    math.mul(v2, w[2])));
    }
}

}  // namespace

/* sub_4e0210's work: dotProductList of `count` vectors at `in` with `d` into `out` as floats at
 * single precision to nearest, once more through the emulated FPU's operations should a value
 * leave a float's range (runNative's way); in any other mode the FPU's way at once. */
void dotProductsOf(App* app, ThreadState& ts, i32 count, u32 in, u32 d, u32 out)
{
    if (singleNearest(ts))
    {
        std::feclearexcept(kOutOfRange);
        dotProductList(app, FloatMath(), count, in, d, out);
        if (!std::fetestexcept(kOutOfRange))
            return;
    }
    dotProductList(app, SingleMath(), count, in, d, out);
}

namespace
{

/* sub_4e0210: dotProductsOf eax vectors at edx with ebx into ecx; eax at the last vector (the
 * count for none). */
nfs::u32 dotProductsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 in = args.edx;
    dotProductsOf(app, ts, count, in, args.ebx, args.ecx);
    if (count > 0)
        args.eax = in + u32(count - 1) * 0xc;
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* sub_4e0280: c = a b, 3x3 matrices of floats, row by row.
 *   eax  a; edx  b; ebx  c
 * c may be a or b: the original copies whichever it is to its frame first, and
 * leaves eax or edx pointing at the copy.  Here everything is read before
 * anything is written; the arithmetic is the emulated FPU's (SingleMath), so
 * nothing runs twice. */
void multiplyMatrix(App* app, u32 a, u32 b, u32 c)
{
    const SingleMath M;
    float left[9], right[9];
    std::memcpy(left, &app->getMemory<float>(a), sizeof left);
    std::memcpy(right, &app->getMemory<float>(b), sizeof right);
    float product[9];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            product[i * 3 + j] = float(M.add(M.add(M.mul(left[i * 3], right[j]), M.mul(left[i * 3 + 1], right[3 + j])),
                                              M.mul(left[i * 3 + 2], right[6 + j])));
    std::memcpy(&app->getMemory<float>(c), product, sizeof product);
}

/* multiplyMatrix with the eax sub_4e0280 leaves: `a`, or the copy of it when `c` is `a`. */
u32 multiplyMatrixEax(App* app, ThreadState& ts, u32 a, u32 b, u32 c)
{
    multiplyMatrix(app, a, b, c);
    return a == c ? ts.sp - 16 - 0x2c : a;
}

namespace
{

/* sub_4e0280's thunk: multiplyMatrix of eax and edx into ebx (ret).  When c is a, the original
 * works from a copy of it in its frame (push ecx, esi, edi, ebp; sub esp, 0x2c) and leaves eax at
 * the copy. */
nfs::u32 multiplyMatricesNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = multiplyMatrixEax(app, ts, args.eax, args.edx, args.ebx);
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* sub_4e0430: points turned by a 3x3 matrix and moved.
 *   eax  count; edx  the points in; ebx  the matrix; ecx  the move;
 *   [esp+4]  the points out, 12 bytes each (it returns with ret 4)
 * A point's three are read before it is written, so it works in place; the
 * move is added to what was stored, as floats.  SingleMath, as for a
 * multiplied matrix: out may be in. */
void transformPointList(App* app, i32 count, u32 in, u32 matrix, u32 move, u32 out)
{
    const SingleMath M;
    for (i32 k = 0; k < count; ++k)
    {
        const float* v = &app->getMemory<float>(in);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        const float* m = &app->getMemory<float>(matrix);
        const float x = float(M.add(M.add(M.mul(v0, m[0]), M.mul(v1, m[3])), M.mul(v2, m[6])));
        const float y = float(M.add(M.add(M.mul(v0, m[1]), M.mul(v1, m[4])), M.mul(v2, m[7])));
        const float z = float(M.add(M.add(M.mul(v0, m[2]), M.mul(v1, m[5])), M.mul(v2, m[8])));
        float* o = &app->getMemory<float>(out);
        o[0] = x;
        o[1] = y;
        o[2] = z;
        const float* t = &app->getMemory<float>(move);
        const float t0 = t[0], t1 = t[1], t2 = t[2];
        const float x2 = float(M.add(o[0], t0));
        const float y2 = float(M.add(o[1], t1));
        const float z2 = float(M.add(o[2], t2));
        o[0] = x2;
        o[1] = y2;
        o[2] = z2;
        in += 0xc;
        out += 0xc;
    }
}

namespace
{

/* sub_4e0430's thunk: transformPointList of eax points at edx by ebx and ecx into [esp+4]
 * (ret 4); eax past the points out (the count for none). */
nfs::u32 transformPointsNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 out = app->getMemory<u32>(ts.sp + 4);
    transformPointList(app, count, args.edx, args.ebx, args.ecx, out);
    if (count > 0)
        args.eax = out + u32(count) * 0xc;
    ts.sp += 4 + 4;
    return args.eax;
}

/* sub_4c79a0: the frame's polygons as one list, from the depth buckets, its head and tail to
 * `out`; `deep` whether the 2000 far buckets count too.
 * Each bucket is a head and a tail (0x7d575c, 8 bytes each): the 256 near ones
 * from the second on, then, asked, the 2000 below 0x7d9dd4 from the top down,
 * each non-empty one hung on the tail so far (its first dword the next).
 * With [0x7a3a60] clear the list is the empty one sub_4bbe70 makes. */
void gatherBucketsOf(App* app, u32 deep, u32 out)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    nfs::u32 head = 0, tail = 0;
    if (word(0x7a3a60) != 0)
    {
        head = word(0x7d575c);
        tail = word(0x7d5760);
        auto take = [&](nfs::u32 bucket) {
            if (head == 0)
            {
                head = word(bucket);
                tail = word(bucket + 4);
            }
            else if (const nfs::u32 first = word(bucket))
            {
                app->getMemory<u32>(tail) = first;
                tail = word(bucket + 4);
            }
        };
        for (nfs::u32 i = 1; i < 0x100; ++i)
            take(0x7d575c + i * 8);
        if (deep)
            for (nfs::u32 i = 0; i < 0x7d0; ++i)
                take(0x7d9dd4 - i * 8);
    }
    app->getMemory<u32>(out) = head;
    app->getMemory<u32>(out + 4) = tail;
}

/* The thunk: eax whether deep, esi the list; esi left at the copy of it the code makes in its
 * frame (five pushes; lea esi, [ebp-8]). */
nfs::u32 gatherBucketsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    gatherBucketsOf(app, args.eax, args.esi);
    args.esi = ts.sp - 20 - 8;
    ts.sp += 4;
    return args.eax;
}

constexpr u32 kFarthestZ = 0x477fff00;  // 65535.0f, where the nearest z starts

/* sub_49d9d0's work: `count` vertices of an object from `in` into view space
 * and onto the screen at `out` (objectVertexList by `matrix` and `position`),
 * each given `colour` (+0x10), and the bits of the nearest z of them, from
 * 65535 down (a NaN z taken too, as fcomp's "not above or equal" takes it). */
u32 colourObjectNearest(App* app, bool asFloats, i32 count, u32 in, u32 matrix, u32 position, u32 out,
                          u32 colour)
{
    u32 nearest = kFarthestZ;
    if (count > 0)
        objectVerticesExactly(app, asFloats, count, in, matrix, position, out);
    for (i32 k = 0; k < count; ++k)
    {
        const u32 vertex = out + u32(k) * 0x20;
        app->getMemory<u32>(vertex + 0x10) = colour;
        const double z = app->getMemory<float>(vertex + 8);
        if (!(z >= double(asFloat(nearest))))
            nearest = app->getMemory<u32>(vertex + 8);
    }
    return nearest;
}

/* sub_49d9d0: colourObjectNearest of eax vertices at edx by ecx and [esp+4]
 * into ebx, the colour [esp+8] (ret 8); the nearest z onto the x87 stack. */
nfs::u32 objectVerticesNearestNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    args.floatSet = true;
    args.floatResult = double(colouredNearestZ(app, ts, i32(args.eax), args.edx, args.ebx, args.ecx,
                                             app->getMemory<u32>(esp + 4), app->getMemory<u32>(esp + 8)));
    ts.sp = esp + 4 + 8;
    return args.eax;
}

/* idiv as the game's code does it: edx:eax over a signed dword, on arm64,
 * where a zero divisor gives a quotient of 0 and leaves the dividend over. */
void signedDivide(nfs::s64 dividend, nfs::s32 divisor, nfs::u32& quotient, nfs::u32& remainder)
{
    if (divisor == 0)
    {
        quotient = 0;
        remainder = nfs::u32(dividend);
        return;
    }
    quotient = nfs::u32(nfs::u64(dividend / divisor));
    remainder = nfs::u32(dividend % divisor);
}

/* sub_41b300: one piece of track's polygons -- four corners each -- as records
 * for the frame's polygon list, rather than drawn at once (sub_41ae90).
 *   ecx  the vertex records, 0x20 bytes each; edx, ebx  the piece: its
 *   polygon count at 0x571370 + edx*0x5c0 + ebx*4, the polygons, 14 bytes
 *   each, 0x2c further on; [esp+4]  the list's tail; [esp+8]  its head
 * The records go one after another from Render_GetTm's buffer (sub_4bbe40,
 * committed by sub_4bbde0), 0x20 bytes: the next record, kind words at +4 and
 * +6 (bits 4 from the polygon's flag 1, 8 for a near one when [0x7a3a58] & 6
 * and [0x6fbc48] are set, 1 for a texture with bit 2 at +0x28), the four
 * corners at +8 to +0x14, the texture record at +0x18 (animated ones step by
 * the race clock), and with bit 1 the mean depth at +0x1c.  A polygon is
 * dropped when its first corner is past the far plane, when its corners share
 * a clip bit, or when both its triangles face away (not when it is two-sided or
 * a corner is behind the view).  Returns how many records were written.
 * Entered at `esp`; the slots the callees are pointed at in its frame (three
 * pushes, then 0x3c bytes). */
u32 trackRecordsOf(App* app, ThreadState& ts, u32 esp, u32 records, u32 pieceRow, u32 pieceColumn,
                     u32 tailArgument, u32 headArgument)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto half = [app](nfs::u32 address) { return app->getMemory<nfs::u16>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 frame = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    const nfs::u32 base = frame - 0x3c;
    const nfs::u32 piece = pieceRow * 0x5c0 + 0x571370;
    const nfs::u32 count = word(piece + pieceColumn * 4);
    nfs::u32 polygon = word(piece + pieceColumn * 4 + 0x2c);
    const nfs::u32 first = frame - 0x3c;
    const nfs::u32 cursorSlot = frame - 0x10;

    if (!coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, first))
        return 0;
    nfs::u32 cursor = word(first);
    nfs::u32 written = 0;
    const double farthest = floatAt(word(0x5dd830) + 0x30);
    const nfs::u32 textures = word(0x552df4);

    for (nfs::s32 i = 0; i < nfs::s32(count); ++i, polygon += 0xe)
    {
        const nfs::u32 v0 = records + (nfs::u32(nfs::s32(nfs::s16(half(polygon)))) << 5);
        const nfs::u32 v1 = records + (nfs::u32(nfs::s32(word(polygon)) >> 16) << 5);
        const nfs::u32 v2 = records + (nfs::u32(nfs::s32(word(polygon + 2)) >> 16) << 5);
        const nfs::u32 v3 = records + (nfs::u32(nfs::s32(word(polygon + 4)) >> 16) << 5);
        word(cursor + 8) = v0;
        word(cursor + 0xc) = v1;
        word(cursor + 0x10) = v2;
        word(cursor + 0x14) = v3;
        const nfs::u8 flags = byte(polygon + 0xc);
        const double z0 = floatAt(v0 + 8);
        if (z0 > farthest)
            continue;
        const nfs::u32 c0 = byte(v0 + 0x14), c1 = byte(v1 + 0x14), c2 = byte(v2 + 0x14), c3 = byte(v3 + 0x14);
        if (c3 & c0 & c1 & c2)
            continue;
        if (!(flags & 0x10) && !((c0 | c1 | c2 | c3) & 0x10))
        {
            const CrossProducts p = crossProducts(ts, floatAt(v2 + 4), floatAt(v1 + 4), floatAt(v0), floatAt(v1),
                                                  floatAt(v0 + 4), floatAt(v1 + 4), floatAt(v2), floatAt(v1));
            const nfs::u32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
            if (back ^ word(0x554e4c))
            {
                const CrossProducts q = crossProducts(ts, floatAt(v3 + 4), floatAt(v2 + 4), floatAt(v0), floatAt(v2),
                                                      floatAt(v0 + 4), floatAt(v2 + 4), floatAt(v3), floatAt(v2));
                const nfs::u32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                if (backToo ^ word(0x554e4c))
                    continue;
            }
        }

        word(cursor) = cursor + 0x20;
        word(cursor + 0x18) = textures + nfs::u32(nfs::s32(word(polygon + 6)) >> 16) * 47;
        app->getMemory<nfs::u16>(cursor + 6) = 0;
        app->getMemory<nfs::u16>(cursor + 4) = 0;
        if (flags & 1)
            byte(cursor + 6) = nfs::u8(byte(cursor + 6) | 4);
        auto depthSum = [&]() {
            return M.mul(M.add(M.add(M.add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8)), floatAt(v3 + 8)),
                         app->getMemory<double>(nfs::u32(0x536c8c)));
        };
        if ((byte(0x7a3a58) & 6) && gameSettings(app).nearPolygons != 0)
        {
            if (!(depthSum() >= app->getMemory<double>(nfs::u32(0x536c94))))
                byte(cursor + 6) = nfs::u8(byte(cursor + 6) | 8);
        }
        if (flags & 4)
        {
            const nfs::u8 animation = byte(polygon + 0xd);
            nfs::u32 frames, step, unused, phase;
            const nfs::s32 clock = nfs::s32(word(0x7d3684));
            signedDivide(clock, nfs::s32(animation >> 3), frames, unused);
            signedDivide(nfs::s32(frames), nfs::s32(animation & 7), step, phase);
            NFS2_USE(step);
            const nfs::u32 texture = phase + nfs::u32(nfs::s32(word(polygon + 8)) >> 16);
            word(cursor + 0x18) = textures + texture * 47;
        }
        if (byte(word(cursor + 0x18) + 0x28) & 2)
        {
            const nfs::u32 colour = word(0x552e1c);
            word(v0 + 0x10) = colour;
            word(v1 + 0x10) = colour;
            word(v2 + 0x10) = colour;
            word(v3 + 0x10) = colour;
            byte(cursor + 6) = nfs::u8(byte(cursor + 6) | 1);
            app->getMemory<float>(cursor + 0x1c) = float(depthSum());
        }
        cursor += 0x20;
        ++written;
    }

    word(cursorSlot) = word(tailArgument);
    if (written != 0)
    {
        if (!commitArena(app, ts, base, frame, first, written << 5))
            return 0;
        const nfs::u32 tail = word(cursorSlot);
        word(tail != 0 ? tail : headArgument) = word(first);
        if (!coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, cursorSlot))
            return 0;
        word(cursorSlot) = word(cursorSlot) - 0x20;
    }
    word(tailArgument) = word(cursorSlot);
    return written;
}

nfs::u32 trackRecordsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    const u32 written = trackRecordsOf(app, ts, esp, args.ecx, args.edx, args.ebx, app->getMemory<u32>(esp + 4),
                                         app->getMemory<u32>(esp + 8));
    if (ts.terminate)
        return args.eax;
    args.eax = written;
    ts.sp = esp + 4 + 8;
    return args.eax;
}

/* One of the drawing's states kept at `slot`: `value` through THRASH_setstate (state `state`)
 * from the depth `base` when it is not what is set already.  False when the game stopped. */
bool setState(App* app, ThreadState& ts, u32 base, u32 slot, u32 state, u32 value)
{
    if (app->getMemory<u32>(slot) == value)
        return true;
    app->getMemory<u32>(slot) = value;
    return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, ts, base, state, value).has_value();
}

/* The state sub_433bb0 and sub_433a50 set for a polygon by its flags `bits` (+6) and its texture
 * record `record` (+0x18), around the copy of the record's light coordinates to its `corners` at
 * +0x18 unless bit 2: blending (bit 1, state 0x68 through [0x554e58]), then the texture (state 1
 * through [0x554e54]) and state 0x15 from [0x7a3a64] unless bit 4 (through [0x554e5c]).  False
 * when the game stopped. */
bool polygonState(App* app, ThreadState& ts, u32 base, nfs::u8 bits, u32 record, std::initializer_list<u32> corners)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    if (!setState(app, ts, base, 0x554e58, 0x68, bits & 1))
        return false;
    if (record != 0 && !(bits & 2))
    {
        u32 from = record + 8;
        for (const u32 corner : corners)
        {
            app->getMemory<u32>(corner + 0x18) = word(from);
            app->getMemory<u32>(corner + 0x1c) = word(from + 4);
            from += 8;
        }
    }
    return setState(app, ts, base, 0x554e54, 1, record != 0 ? u32(word(record + 4)) : 0)
           && setState(app, ts, base, 0x554e5c, 0x15, (bits & 4) ? nfs::u32(0) : nfs::u32(word(0x7a3a64)));
}

/* sub_433bb0: a list of quads to the drawing, from `quad` while each next
 * one's kind word at +4 is 0.  For each, its state (polygonState) and its
 * four corners (+8 to +0x14) to sub_4c20e0 with bit 0x10 of its flags, else
 * to the quad's clipper sub_4c12e0.  Returns the quad the list stopped at.
 * Entered at `esp`; its frame below six pushes and 0xc bytes. */
u32 quadListOf(App* app, ThreadState& ts, u32 esp, u32 quad)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 base = esp - 24 - 0xc;
    while (true)
    {
        const nfs::u32 a = word(quad + 8), b = word(quad + 0xc), c = word(quad + 0x10), d = word(quad + 0x14);
        const nfs::u8 bits = app->getMemory<nfs::u8>(quad + 6);
        if (!polygonState(app, ts, base, bits, word(quad + 0x18), { a, b, c, d }))
            return 0;
        if (bits & 0x10)
            coreAt<0x4c20e0, render::quadByZDrawn>(app, ts, base, a, b, c, d);
        else
            clipQuadOf(app, ts, base - 4, a, b, c, d);
        if (ts.terminate)
            return 0;
        quad = word(quad);
        if (quad == 0 || app->getMemory<nfs::u16>(quad + 4) != 0)
            return quad;
    }
}

nfs::u32 quadListNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = quadListOf(app, ts, ts.sp, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_433a50: a list of triangles to the drawing, from `triangle` while each
 * next one's kind word at +4 is 1 -- sub_433bb0 for triangles: its state
 * (polygonState), then the corners (+8 to +0x10) to sub_4c1aa0 with bit 0x10
 * of its flags, else to sub_4c11b0.  Returns the triangle the list stopped
 * at.  Entered at `esp`; its frame below six pushes and 8 bytes. */
u32 triangleListOf(App* app, ThreadState& ts, u32 esp, u32 triangle)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 base = esp - 24 - 8;
    while (true)
    {
        const nfs::u32 a = word(triangle + 8), b = word(triangle + 0xc), c = word(triangle + 0x10);
        const nfs::u8 bits = app->getMemory<nfs::u8>(triangle + 6);
        if (!polygonState(app, ts, base, bits, word(triangle + 0x18), { a, b, c }))
            return 0;
        if (bits & 0x10)
            clipTriangleByZOf(app, ts, base - 4, a, b, c);
        else
            clipTriangleOf(app, ts, base - 4, a, b, c);
        if (ts.terminate)
            return 0;
        triangle = word(triangle);
        if (triangle == 0 || app->getMemory<nfs::u16>(triangle + 4) != 1)
            return triangle;
    }
}

nfs::u32 triangleListNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = triangleListOf(app, ts, ts.sp, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* A bucket's head and tail (two dwords): the polygon hung on its end. */
inline void appendToBucket(nfs::App* app, nfs::u32 bucket, nfs::u32 polygon)
{
    if (app->getMemory<nfs::u32>(bucket) != 0)
        app->getMemory<nfs::u32>(app->getMemory<nfs::u32>(bucket + 4)) = polygon;
    else
        app->getMemory<nfs::u32>(bucket) = polygon;
    app->getMemory<nfs::u32>(bucket + 4) = polygon;
}

/* sub_4c7610: the polygons of the list `list` whose texture has bit 2 at
 * +0x28, each put into one of 800 buckets at 0x7db6dc by its depth at +0x1c
 * times [0x54024c], kept in order of that depth, deepest first; `start`
 * (its stack word) passed on to sub_4c7480 when [0x7a3a60] is clear, which
 * does it instead.  Returns what the code leaves in eax: the callee's, or
 * the last bucket's offset or node looked at, the status of its fcomp over
 * the node's low word (fnstsw ax).  Entered at `esp`; six pushes. */
u32 sortedBucketsOf(App* app, ThreadState& ts, u32 esp, u32 list, u32 start)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    if (word(0x7a3a60) == 0)
        return coreAt<0x4c7480, frontend::rende::depthBucketsAdd>(app, ts, esp - 24, list, start).value_or(0);
    nfs::u32 polygon = list;
    nfs::u32 eax = list;
    const double scale = app->getMemory<float>(nfs::u32(0x54024c));
    while (polygon != 0)
    {
        const nfs::u32 next = word(polygon);
        nfs::s32 index =
            fp::toInteger<nfs::s32>(nfs::Float(SingleMath().mul(app->getMemory<float>(polygon + 0x1c), scale)));
        if (index < 0)
            index = 0;
        else if (index > 0x31f)
            index = 0x31f;
        const nfs::u32 bucket = 0x7db6dc + nfs::u32(index) * 8;
        eax = nfs::u32(index) * 8;
        if (word(bucket) == 0)
        {
            app->getMemory<u32>(polygon) = 0;
            app->getMemory<u32>(bucket) = polygon;
            app->getMemory<u32>(bucket + 4) = polygon;
            polygon = next;
            continue;
        }
        const double depth = app->getMemory<float>(polygon + 0x1c);
        nfs::u32 link = bucket;
        while (true)
        {
            const nfs::u32 node = word(link);
            eax = node;
            if (node == 0)
                break;
            const double nodeDepth = app->getMemory<float>(node + 0x1c);
            eax = statusAfterCompare(node, nodeDepth, depth);
            if (!(nodeDepth >= depth))
            {
                app->getMemory<u32>(polygon) = node;
                app->getMemory<u32>(link) = polygon;
                break;
            }
            if (word(node) == 0)
            {
                app->getMemory<u32>(polygon) = 0;
                app->getMemory<u32>(word(bucket + 4)) = polygon;
                app->getMemory<u32>(bucket + 4) = polygon;
                break;
            }
            link = node;
        }
        polygon = next;
    }
    return eax;
}

nfs::u32 sortedBucketsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    const u32 result = sortedBucketsOf(app, ts, esp, args.eax, app->getMemory<u32>(esp + 4));
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp = esp + 4 + 4;
    return args.eax;
}

/* sub_4c7790: the frame's polygons into the depth buckets.
 *   eax  the list (each polygon's first dword the next; its corners at +8 to
 *   +0x14, +4 its kind -- 0 four corners, 1 three -- +0x18 its texture);
 *   [esp+4]  the depth the buckets start from (ret 4)
 * A texture with bit 2 at +0x28 goes to sub_4c7610 alone; an opaque polygon
 * (every corner's alpha 0xff) whose texture has no bit 4 into the near bucket
 * its texture names at +0x29 (0x7d575c); the rest into one of the 2000 far
 * buckets (0x7d5f5c) by its mean depth, ((sum * [0x540254] or [0x540264]) -
 * start + [0x7dcfdc]) * [0x54025c], held to 0..1999.  With [0x7a3a60] clear,
 * sub_4c7480 does it instead.  Entered at `esp`; six pushes, then 8 bytes. */
void depthBucketsOf(App* app, ThreadState& ts, u32 esp, u32 list, u32 start)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto floatAt = [app](nfs::u32 address) -> double { return app->getMemory<float>(address); };
    const nfs::u32 frame = esp - 24;
    if (word(0x7a3a60) == 0)
    {
        coreAt<0x4c7480, frontend::rende::depthBucketsAdd>(app, ts, frame - 4, list, start);
        return;
    }
    nfs::u32 polygon = list;
    while (polygon != 0)
    {
        const nfs::u32 next = word(polygon);
        const nfs::u32 v0 = word(polygon + 8), v1 = word(polygon + 0xc), v2 = word(polygon + 0x10),
                         v3 = word(polygon + 0x14);
        const nfs::u32 texture = word(polygon + 0x18);
        if (texture && (app->getMemory<nfs::u8>(texture + 0x28) & 2))
        {
            // alone, pushed with a start of 0 under the next polygon's slot
            app->getMemory<u32>(polygon) = 0;
            sortedBucketsOf(app, ts, frame - 12, polygon, 0);
            if (ts.terminate)
                return;
            polygon = next;
            continue;
        }
        const nfs::u16 kind = app->getMemory<nfs::u16>(polygon + 4);
        app->getMemory<u32>(polygon) = 0;
        nfs::u32 alpha = word(v0 + 0x10) & word(v1 + 0x10) & word(v2 + 0x10);
        if (kind == 0)
            alpha &= word(v3 + 0x10);
        if ((alpha >> 24) == 0xff && !(texture && (app->getMemory<nfs::u8>(texture + 0x28) & 4)))
        {
            appendToBucket(app, 0x7d575c + (nfs::u32(app->getMemory<nfs::u8>(texture + 0x29)) << 3), polygon);
            polygon = next;
            continue;
        }
        if (kind > 1)
        {
            polygon = next;
            continue;
        }
        double sum = M.add(M.add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8));
        if (kind == 0)
            sum = M.add(sum, floatAt(v3 + 8));
        const double mean = M.mul(sum, app->getMemory<double>(nfs::u32(kind == 0 ? 0x540254 : 0x540264)));
        const double shifted = M.add(M.sub(mean, asFloat(start)), floatAt(0x7dcfdc));
        nfs::s32 index = fp::toInteger<nfs::s32>(nfs::Float(M.mul(shifted, floatAt(0x54025c))));
        if (index < 0)
            index = 0;
        else if (index > 0x7cf)
            index = 0x7cf;
        appendToBucket(app, 0x7d5f5c + (nfs::u32(index) << 3), polygon);
        polygon = next;
    }
}

nfs::u32 depthBucketsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    depthBucketsOf(app, ts, esp, args.eax, app->getMemory<u32>(esp + 4));
    if (ts.terminate)
        return args.eax;
    ts.sp = esp + 4 + 4;
    return args.eax;
}

}  // namespace

/* sub_49d9d0's core: colourObjectNearest's nearest z, which its thunk leaves on the x87 stack. */
float colouredNearestZ(App* app, ThreadState& ts, i32 count, u32 in, u32 out, u32 matrix, u32 position, u32 colour)
{
    return asFloat(colourObjectNearest(app, singleNearest(ts), count, in, matrix, position, out, colour));
}

/* sub_4e01f0's work: (a0 b0 + a1 b1) + a2 b2 of the vectors at a and b, as wide as the x87 holds
 * it. */
double dotProduct(App* app, u32 a, u32 b)
{
    const SingleMath M;
    const float* u = &app->getMemory<float>(a);
    const float* v = &app->getMemory<float>(b);
    return M.add(M.add(M.mul(u[0], v[0]), M.mul(u[1], v[1])), M.mul(u[2], v[2]));
}

namespace
{

/* sub_4e01f0: dotProduct of eax and edx onto the x87 stack. */
nfs::u32 dotPushNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(dotProduct(app, args.eax, args.edx));
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* sub_4972f0's work: the vector at `vector` made of length 1 in place; returns its length as a
 * float, 0 for a zero one (of either sign), which stays as it is. */
float normaliseVector(App* app, u32 vector)
{
    const SingleMath M;
    float* v = &app->getMemory<float>(vector);
    const float x = v[0], y = v[1], z = v[2];
    const float length = float(M.sqrt(M.add(M.add(M.mul(y, y), M.mul(x, x)), M.mul(z, z))));
    if (!(asBits(length) & 0x7fffffff))
        return 0.0f;
    const double inverse = M.div(1.0, length);
    v[0] = float(M.mul(x, inverse));
    v[1] = float(M.mul(y, inverse));
    v[2] = float(M.mul(z, inverse));
    return length;
}

namespace
{

/* sub_4972f0: normaliseVector of eax, the length onto the x87 stack. */
nfs::u32 normaliseNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(normaliseVector(app, args.eax));
    ts.sp += 4;
    return args.eax;
}

/* A point of 16.16 integers on the ground: x and z (y is not measured). */
struct GroundPoint
{
    i32 x, z;
};

GroundPoint groundPointAt(App* app, u32 point)
{
    return {i32(app->getMemory<u32>(point)), i32(app->getMemory<u32>(point + 8))};
}

/* sub_41aab0's work: how far apart two points are on the ground, as the game measures it:
 * (dx >> 12)^2 >> 6 plus (dz >> 12)^2 >> 6, wrapping as its integers do. */
u32 groundDistance(GroundPoint a, GroundPoint b)
{
    const i32 dz = i32(u32(b.z) - u32(a.z)) >> 12;
    const i32 dx = i32(u32(b.x) - u32(a.x)) >> 12;
    const i32 z2 = i32(u32(dz) * u32(dz));
    const i32 x2 = i32(u32(dx) * u32(dx));
    return u32(x2 >> 6) + u32(z2 >> 6);
}

}  // namespace

/* sub_41aab0's work for the points at `a` and `b`: groundDistance. */
u32 groundDistanceAt(App* app, u32 a, u32 b)
{
    return groundDistance(groundPointAt(app, a), groundPointAt(app, b));
}

namespace
{

/* sub_41aab0: groundDistanceAt the points at eax and edx to eax. */
nfs::u32 groundDistanceNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = groundDistanceAt(app, args.eax, args.edx);
    ts.sp += 4;
    return args.eax;
}

// The track's blocks: their count, and where their records (36 bytes, a 16.16 point first) are.
constexpr u32 kBlockCount = 0x5dd940, kBlocks = 0x5dd948, kBlockBytes = 36;
constexpr u32 kToFixed = 0x536d74;  // a double: a float to 16.16

}  // namespace

/* sub_41e0f0's work: the track block nearest the point at `point` (floats, turned to 16.16 by
 * [kToFixed] as fistp rounds), the first of the nearest. */
NearestBlock nearestBlock(App* app, u32 point)
{
    const SingleMath M;
    const double scale = app->getMemory<double>(kToFixed);
    const float* p = &app->getMemory<float>(point);
    i32 fixed[3];
    for (int k = 0; k < 3; ++k)
        fixed[k] = fp::toInteger<i32>(nfs::Float(M.mul(p[k], scale)));
    const GroundPoint here{fixed[0], fixed[2]};
    const i32 count = i32(app->getMemory<u32>(kBlockCount));
    const u32 blocks = app->getMemory<u32>(kBlocks);
    i32 best = 0x7fffffff;
    u32 nearest = 0xffffffff;
    i32 block = 0;
    for (; block < count; ++block)
    {
        const i32 distance = i32(groundDistance(groundPointAt(app, blocks + u32(block) * kBlockBytes), here));
        if (distance < best)
        {
            best = distance;
            nearest = u32(block);
        }
    }
    return {nearest, block, count};
}

namespace
{

/* sub_41e0f0: nearestBlock to the point at eax, to eax. */
nfs::u32 nearestBlockNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = nearestBlock(app, args.eax).block;
    ts.sp += 4;
    return args.eax;
}

// The 800 buckets below the ones sub_4c7610 sorts into: a head and a tail each, the top one first.
constexpr u32 kSortedBucketsTop = 0x7db6d4, kSortedBucketCount = 0x320;

/* sub_4c7320's work: kSortedBucketCount buckets as one list, from the top one down, its head and
 * tail to `out`. */
void gatherSortedBuckets(App* app, u32 out)
{
    auto word = [app](u32 address) { return app->getMemory<u32>(address); };
    u32 head = 0, tail = 0;
    for (u32 k = 0; k < kSortedBucketCount; ++k)
    {
        const u32 bucket = kSortedBucketsTop - k * 8;
        const u32 first = word(bucket);
        if (first == 0)
            continue;
        if (head == 0)
            head = first;
        else
            word(tail) = first;
        tail = word(bucket + 4);
    }
    word(out) = head;
    word(out + 4) = tail;
}

/* sub_4c7320: gatherSortedBuckets to esi; esi left past the two words the code copies from its
 * frame, where it keeps them. */
nfs::u32 gatherSortedNative(App* app, ThreadState& ts, nfs::Args& args)
{
    gatherSortedBuckets(app, args.esi);
    args.esi = ts.sp - 20;  // push ebx, ecx, edx, edi, ebp; lea esi, [ebp-8]; two movsd
    ts.sp += 4;
    return args.eax;
}

// A view's position and matrix for an object's vertices (objectVertexList).
constexpr u32 kViewObjectPosition = 0x38, kViewObjectMatrix = 0x44;

/* sub_41a970's work: `count` vertices of an object from `in` into view space
 * and onto the screen at `out` (objectVertexList with the view's position and
 * matrix), each then given the colour (+0x10) from the list at `colours`. */
void colouredObjectVertices(App* app, bool asFloats, u32 view, u32 count, u32 in, u32 colours, u32 out)
{
    if (i32(count) > 0)
        objectVerticesExactly(app, asFloats, i32(count), in, view + kViewObjectMatrix, view + kViewObjectPosition,
                              out);
    for (u32 left = count; left != 0; --left, colours += 4, out += 0x20)
        app->getMemory<u32>(out + 0x10) = app->getMemory<u32>(colours);
}

/* sub_41a970: colouredVerticesOf the view eax, edx vertices at ebx, the colours at ecx, into
 * [esp+4] (ret 4). */
nfs::u32 objectVerticesColouredNative(App* app, ThreadState& ts, nfs::Args& args)
{
    colouredVerticesOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, app->getMemory<u32>(ts.sp + 4));
    ts.sp += 4 + 4;
    return args.eax;
}

}  // namespace

/* sub_41a970's work as its thunk does it: colouredObjectVertices for the view `view`, `count`
 * vertices at `in`, the colours at `colours`, into `out`, as floats at single precision to
 * nearest. */
void colouredVerticesOf(App* app, ThreadState& ts, u32 view, u32 count, u32 in, u32 colours, u32 out)
{
    colouredObjectVertices(app, singleNearest(ts), view, count, in, colours, out);
}

/* sub_4968a0's work: a point brought onto the ground plane of a frame -- p - o
 * turned into the frame by the transpose of `matrix` (sub_4e0110), its height
 * dropped, turned back and o added -- and the height of the result returned.
 * Nothing is written. */
float groundHeight(App* app, u32 matrix, const float (&o)[3], const float (&p)[3])
{
    const SingleMath M;
    const float* m = &app->getMemory<float>(matrix);
    const float d[3] = { float(M.sub(p[0], o[0])), float(M.sub(p[1], o[1])), float(M.sub(p[2], o[2])) };
    float r[3];
    for (int j = 0; j < 3; ++j)
        r[j] = float(M.add(M.add(M.mul(d[0], m[3 * j]), M.mul(d[1], m[3 * j + 1])), M.mul(d[2], m[3 * j + 2])));
    r[1] = 0.0f;
    float q[3];
    for (int j = 0; j < 3; ++j)
        q[j] = float(M.add(M.add(M.mul(r[0], m[j]), M.mul(r[1], m[3 + j])), M.mul(r[2], m[6 + j])));
    for (int j = 0; j < 3; ++j)
        q[j] = float(M.add(q[j], o[j]));
    return q[1];
}

namespace
{

/* sub_4968a0's thunk: groundHeight by the matrix at eax of o and p, onto the x87 stack (ret
 * 0x18; the d the code leaves in p's place is below the esp it returns with); eax at the second
 * turn's output in its frame. */
nfs::u32 groundHeightNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    const float* stack = &app->getMemory<float>(esp + 4);
    const float o[3] = { stack[0], stack[1], stack[2] };
    const float p[3] = { stack[3], stack[4], stack[5] };
    args.floatSet = true;
    args.floatResult = double(groundHeight(app, args.eax, o, p));
    args.eax = esp - 16 - 0xc;  // lea eax, [eax + 0xc] past the second turn's output, [ebp-0xc]
    ts.sp = esp + 4 + 0x18;
    return args.eax;
}

/* A screen vertex's clip code from its x, y and 1/z at +0, +4, +0xc, compared
 * as integers with the bounds at 0x7d34f8..0x7d350c -- the code sub_41bf30,
 * sub_4cbc90 and others write inline. */
inline nfs::u8 screenCode(nfs::App* app, nfs::u32 vertex)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const nfs::u32 inverse = word(vertex + 0xc);
    if ((inverse & 0x80000000) || nfs::s32(inverse) >= nfs::s32(word(0x7d34f8)))
        return 0x10;
    nfs::u8 code = 0;
    const nfs::u32 y = word(vertex + 4);
    if ((y & 0x80000000) || nfs::s32(y) < nfs::s32(word(0x7d3504)))
        code |= 8;
    else if (nfs::s32(y) > nfs::s32(word(0x7d3500)))
        code |= 4;
    const nfs::u32 x = word(vertex);
    if ((x & 0x80000000) || nfs::s32(x) < nfs::s32(word(0x7d34fc)))
        code |= 1;
    else if (nfs::s32(x) > nfs::s32(word(0x7d350c)))
        code |= 2;
    return code;
}

/* sub_4cbc90: the 600 particles (smoke, spray, sparks: 0x7ddfd0, 128 bytes
 * each) of one view pass as quads in Render_GetTm's buffer.
 *   eax  the view (+0 its pass, 1 the mirror; +4 its player; +0x38 the
 *   camera); edx  its matrix; esi  where the list's head and tail go
 * A particle is skipped when it is not live (+0), when Effects is low
 * ([0x6fbc30] 1) and it is one of the minor ones (+0x7d bit 1), when it is
 * behind the view or past its kind's reach ([0x560bc4] of its kind, 128 bytes
 * at 0x560bbc each, times View Distance's factor at 0x4ca480); one within 3 of
 * the main view's camera may splash on it (sub_4ca500) and die.  The rest:
 * screen position as sub_41bf30 works it out, the colour at +0x54 scaled by
 * the alpha intensity slider when the kind's +0 bit 1 asks for it, the size at
 * +0x24 in pixels (rounded once past 1 away), and a quad -- a turned square
 * (+0x38, +0x3c its cos and sin) or, for a streak (+0x7c), a band from where
 * it was last drawn in this pass (+0x58, per player and pass; the flag byte at
 * +0x78 per player and pass says it was) -- with the kind's flags, the
 * texture at +0x50 and every corner's clip code.  The flag byte is cleared
 * for every particle not drawn this way.  Entered at `esp`; the slot the
 * callees are pointed at in its frame (four pushes, ebp 0x82 under them).  Returns
 * where the code leaves esi. */
u32 particlesOf(App* app, ThreadState& ts, u32 esp, u32 view, u32 matrix, u32 out)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    auto putFloat = [app](nfs::u32 address, double value) { app->getMemory<float>(address) = float(value); };
    const nfs::u32 ebp = esp - 16 - 0x82;  // push ebx, ecx, edi, ebp; mov ebp, esp; sub ebp, 0x82
    const nfs::u32 base = esp - 16 - 0xe4;
    const nfs::u32 slot = ebp + 0x22;      // Render_GetTm's
    nfs::u32 head = 0, tail = 0;           // sub_4bbe70's empty list
    auto finish = [&]() {
        app->getMemory<u32>(out) = head;
        app->getMemory<u32>(out + 4) = tail;
        return ebp - 0x16;  // lea esi, [ebp-0x1e]; two movsd
    };

    const nfs::u32 alphaScale = nfs::u32(fp::toInteger<nfs::s32>(
        nfs::Float(M.mul(gameSettings(app).alphaIntensity, app->getMemory<double>(nfs::u32(0x540448))))));
    const float camera[3] = { floatAt(view + 0x38), floatAt(view + 0x3c), floatAt(view + 0x40) };
    const nfs::u32 player = word(view + 4);

    const auto room = coreAt<0x4bbe40, frontend::rendd::arenaHere>(app, ts, base, slot);
    if (!room)
        return 0;
    if (*room == 0)
        return finish();
    const nfs::u32 first = word(slot);
    nfs::u32 cursor = first;
    nfs::u32 count = 0;
    const double k1 = floatAt(0x56009c), k2 = floatAt(0x5600a0), c1 = floatAt(0x5600a4), c2 = floatAt(0x5600a8);
    const nfs::u32 viewLevel = gameSettings(app).viewDistance;
    for (nfs::u32 i = 0; i < 0x258; ++i)
    {
        const nfs::u32 particle = 0x7ddfd0 + i * 0x80;
        const float position[3] = { floatAt(particle + 0xc), floatAt(particle + 0x10), floatAt(particle + 0x14) };
        float sizeX = float(M.mul(floatAt(particle + 0x24), k1));
        float sizeY = float(M.mul(floatAt(particle + 0x24), k2));
        const nfs::u32 kind = word(particle + 4);
        const nfs::u32 pass = word(view);
        const nfs::u32 flag = particle + 0x78 + player * 2 + (pass == 1 ? 1 : 0);
        if ((gameSettings(app).effectsLevel == 1 && (byte(particle + 0x7d) & 1)) || word(particle) == 0)
        {
            app->getMemory<nfs::u8>(flag) = 0;
            continue;
        }
        const float* m = &app->getMemory<float>(matrix);
        float x = float(M.add(M.add(M.mul(position[0], m[0]), M.mul(position[1], m[3])), M.mul(position[2], m[6])));
        float y = float(M.add(M.add(M.mul(position[0], m[1]), M.mul(position[1], m[4])), M.mul(position[2], m[7])));
        float z = float(M.add(M.add(M.mul(position[0], m[2]), M.mul(position[1], m[5])), M.mul(position[2], m[8])));
        x = float(M.add(x, camera[0]));
        y = float(M.add(y, camera[1]));
        z = float(M.add(z, camera[2]));
        if (0.0 >= z || !(M.mul(floatAt(0x560bc4 + (kind << 7)), floatAt(0x4ca480 + viewLevel * 4)) >= z))
        {
            app->getMemory<nfs::u8>(flag) = 0;
            continue;
        }
        if (!(asBits(z) & 0x7fffffff))
            z = asFloat(0x37800080);
        const float inverse = float(M.div(1.0, z));
        const nfs::u32 centre = ebp - 0x62;
        putFloat(centre, M.add(M.mul(M.mul(k1, inverse), x), c1));
        putFloat(centre + 4, M.add(M.mul(M.mul(k2, inverse), y), c2));
        app->getMemory<u32>(centre + 8) = asBits(z);
        app->getMemory<u32>(centre + 0xc) = asBits(inverse);
        app->getMemory<nfs::u8>(centre + 0x14) = screenCode(app, centre);
        if (nfs::s32(asBits(z)) < 0x40400000 && pass == 0)
        {
            // a particle on the camera: sub_4ca500 may splash it over the view (ret 0x10)
            const u32 on = base - 16;
            app->getMemory<u32>(on) = asBits(x);
            app->getMemory<u32>(on + 4) = asBits(y);
            app->getMemory<u32>(on + 8) = asBits(z);
            app->getMemory<u32>(on + 12) = kind;
            const auto splashed = coreAt<0x4ca500, frontend::sime::dropsOnView>(app, ts, on, player, on);
            if (!splashed)
                return 0;
            if (*splashed != 0)
            {
                app->getMemory<nfs::u8>(flag) = 0;
                app->getMemory<u32>(particle) = 0;  // edi, 0 then
                continue;
            }
        }

        const nfs::u32 corners[4] = { cursor + 0x20, cursor + 0x40, cursor + 0x60, cursor + 0x80 };
        nfs::u32 colour = word(particle + 0x54);
        // [0x7a3a58] & 0x40 forced on (the port's alpha intensity change)
        if (1.0 > gameSettings(app).alphaIntensity && (byte(0x560bbc + (kind << 7)) & 1))
        {
            auto channel = [&](nfs::u32 at) {
                return (nfs::u32(nfs::s32(nfs::u32(byte(particle + at)) * alphaScale) >> 16)) & 0xff;
            };
            colour = channel(0x57) << 24 | channel(0x56) << 16 | channel(0x55) << 8 | channel(0x54);
        }
        if (!(1.0 > z))
        {
            const float scale = float(M.div(1.0, z));
            sizeX = float(fp::toInteger<nfs::s32>(nfs::Float(M.mul(sizeX, scale))));
            sizeY = float(fp::toInteger<nfs::s32>(nfs::Float(M.mul(sizeY, scale))));
        }
        app->getMemory<nfs::u16>(cursor + 4) = 0;
        app->getMemory<u32>(cursor) = cursor + 0xa0;
        app->getMemory<nfs::u16>(cursor + 6) = app->getMemory<nfs::u16>(0x560bbc + (kind << 7));
        app->getMemory<u32>(cursor + 0x18) = word(particle + 0x50);
        for (int c = 0; c < 4; ++c)
            app->getMemory<u32>(cursor + 8 + nfs::u32(c) * 4) = corners[c];
        app->getMemory<u32>(cursor + 0x1c) = asBits(z);
        for (int c = 0; c < 4; ++c)
        {
            app->getMemory<u32>(corners[c] + 8) = asBits(z);
            app->getMemory<u32>(corners[c] + 0xc) = asBits(inverse);
            app->getMemory<u32>(corners[c] + 0x10) = colour;
        }
        const double cx = floatAt(centre), cy = floatAt(centre + 4);
        if (byte(particle + 0x7c) != 0)
        {
            const nfs::u32 previous = particle + 0x58 + (player << 4) + ((pass == 1 ? 1u : 0u) << 3);
            const double limit = app->getMemory<double>(nfs::u32(0x540450));
            auto clamp = [&](double size) {
                if (!(size >= limit))
                    return 0.5;
                return 1.0 >= size ? size : 1.0;  // never wider than a pixel
            };
            const float width = float(clamp(sizeX));
            const float height = float(clamp(sizeY));
            if (byte(flag) != 0)
            {
                const float px = floatAt(previous);
                app->getMemory<u32>(corners[0]) = asBits(px);
                app->getMemory<u32>(corners[1]) = asBits(px);
                app->getMemory<u32>(corners[0] + 4) = word(previous + 4);
                app->getMemory<u32>(corners[1] + 4) = word(previous + 4);
                putFloat(corners[2], M.add(cx, width));
                putFloat(corners[2] + 4, M.add(cy, height));
                putFloat(corners[3], M.sub(cx, width));
                putFloat(corners[3] + 4, M.sub(cy, height));
                const double half = floatAt(0x540458);
                putFloat(previous, M.mul(M.add(cx, floatAt(previous)), half));
                putFloat(previous + 4, M.mul(M.add(cy, floatAt(previous + 4)), half));
            }
            else
            {
                app->getMemory<u32>(corners[0]) = word(centre);
                app->getMemory<u32>(corners[0] + 4) = word(centre + 4);
                putFloat(corners[1], M.add(cx, width));
                putFloat(corners[1] + 4, M.sub(cy, height));
                putFloat(corners[2], M.add(width, cx));
                putFloat(corners[2] + 4, M.add(cy, height));
                app->getMemory<u32>(corners[3]) = word(centre);
                putFloat(corners[3] + 4, M.add(height, cy));
                app->getMemory<u32>(previous) = word(centre);
                app->getMemory<u32>(previous + 4) = word(centre + 4);
            }
            app->getMemory<nfs::u8>(flag) = 1;
        }
        else
        {
            const double a = M.mul(floatAt(particle + 0x3c), sizeY);
            const double b = M.mul(floatAt(particle + 0x38), sizeX);
            putFloat(corners[0], M.sub(cx, a));
            putFloat(corners[0] + 4, M.add(cy, b));
            putFloat(corners[1], M.add(cx, b));
            putFloat(corners[1] + 4, M.add(cy, a));
            putFloat(corners[2], M.add(cx, a));
            putFloat(corners[2] + 4, M.sub(cy, b));
            putFloat(corners[3], M.sub(cx, b));
            putFloat(corners[3] + 4, M.sub(cy, a));
        }
        for (int c = 0; c < 4; ++c)
            app->getMemory<nfs::u8>(corners[c] + 0x14) = screenCode(app, corners[c]);
        cursor += 0xa0;
        ++count;
    }

    if (count == 0)
        return finish();
    const nfs::u32 last = cursor - 0xa0;
    if (!commitArena(app, ts, base, ebp, slot, count * 0xa0))
        return 0;
    tail = last;
    head = word(slot);
    app->getMemory<u32>(last) = 0;
    return finish();
}

/* The thunk: eax the view, edx its matrix, esi where the list goes; esi left past the copy
 * of it the code makes from its frame. */
nfs::u32 particlesNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esi = particlesOf(app, ts, ts.sp, args.eax, args.edx, args.esi);
    if (ts.terminate)
        return args.eax;
    args.esi = esi;
    ts.sp += 4;
    return args.eax;
}

/* sub_4cb750: the drops on the view of one player (0x7f0bd0, 0x3200 bytes a
 * player, 200 drops of 64 bytes), drawn as screen-space quads hung on the
 * overlay list (sub_4bbf80): each corner at +8.. +0x24 scaled to the screen,
 * depth 0 and 1/z 0x3f7fff00, the colour at +0x28 (scaled by the alpha
 * intensity slider when the drop's flags at +0x2c ask; the [0x7a3a58] & 0x40
 * test is forced on, the port's alpha intensity change), the flags and the
 * texture at +0x30.  Nothing under camera modes 8, 0xe and 0x12.
 *   eax  the view (+4 its player)
 * Entered at `esp`; the slot sub_4bbf80 fills in its frame below six pushes. */
void dropsOf(App* app, ThreadState& ts, u32 esp, u32 view)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 frame = esp - 24;
    const nfs::u32 player = word(view + 4);
    const nfs::u32 alphaScale = nfs::u32(fp::toInteger<nfs::s32>(
        nfs::Float(M.mul(gameSettings(app).alphaIntensity, app->getMemory<double>(nfs::u32(0x540440))))));
    // The camera's mode, three times as the original reads it.
    auto cameraMode = [&]() -> nfs::u32 {
        const nfs::u32 first = word(0x5e10b8 + player * 4);
        if (first)
            return word(first);
        const nfs::u32 second = word(0x5e10b0 + player * 4);
        return second ? nfs::u32(word(second)) : 0u;
    };
    for (const nfs::u32 skipped : { 8u, 0xeu, 0x12u })
        if (cameraMode() == skipped)
            return;
    const double k1 = floatAt(0x56009c), k2 = floatAt(0x5600a0), c1 = floatAt(0x5600a4), c2 = floatAt(0x5600a8);
    const nfs::u32 drops = 0x7f0bd0 + player * 0x3200;
    for (nfs::u32 i = 0; i < 0xc8; ++i)
    {
        const nfs::u32 drop = drops + (i << 6);
        if (word(drop) == 0)
            continue;
        // sub_4bbf80 hands edx, the size, on to sub_4bbde0
        const auto room = coreAt<0x4bbf80, frontend::rendd::openListTake>(app, ts, frame - 0x28, frame - 0x28, 0xa0);
        if (!room || *room == 0)
            return;
        const nfs::u32 polygon = word(frame - 0x28);
        nfs::u32 colour = word(drop + 0x28);
        if (1.0 > gameSettings(app).alphaIntensity && (byte(drop + 0x2c) & 1))
        {
            auto channel = [&](nfs::u32 at) {
                return (nfs::u32(nfs::s32(nfs::u32(byte(drop + at)) * alphaScale) >> 16)) & 0xff;
            };
            colour = channel(0x2b) << 24 | channel(0x2a) << 16 | channel(0x29) << 8 | channel(0x28);
        }
        app->getMemory<nfs::u16>(polygon + 4) = 0;
        app->getMemory<nfs::u16>(polygon + 6) = app->getMemory<nfs::u16>(drop + 0x2c);
        for (nfs::u32 c = 0; c < 4; ++c)
            app->getMemory<u32>(polygon + 8 + c * 4) = polygon + 0x20 + c * 0x20;
        app->getMemory<u32>(polygon + 0x18) = word(drop + 0x30);
        for (nfs::u32 c = 0; c < 4; ++c)
        {
            const nfs::u32 corner = polygon + 0x20 + c * 0x20;
            app->getMemory<float>(corner) = float(M.add(M.mul(floatAt(drop + 8 + c * 8), k1), c1));
            app->getMemory<u32>(corner + 8) = 0;
            app->getMemory<u32>(corner + 0xc) = 0x3f7fff00;
            app->getMemory<float>(corner + 4) = float(M.add(M.mul(floatAt(drop + 0xc + c * 8), k2), c2));
            app->getMemory<nfs::u8>(corner + 0x14) = screenCode(app, corner);
            app->getMemory<u32>(corner + 0x10) = colour;
        }
    }
}

nfs::u32 dropsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    dropsOf(app, ts, ts.sp, args.eax);
    if (ts.terminate)
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* sub_4cb2c0: the 600 particles moved on a frame.  Every live one moves by its
 * velocity times [0x540420]; on odd frames (and after sub_4cb220 for each
 * player first) it also ages: its spin turns it, bounced back off the
 * limits [0x540410]/[0x540418] where its kind (0x560b64, 128 bytes) says so
 * and damped; a swirling one (+0x44) swings its velocity round; its size
 * grows with age; every fourth frame its colour fades by its kind's; its
 * velocity is damped and falls by its kind's gravity; it loses a frame of
 * life, dies with its alpha, and one that spawns (+0x7e) hands a copy of
 * itself to sub_4cab60 as it dies.  Returns what the code leaves in eax: the
 * last live particle's position, else what sub_4cb220 or the frame's parity
 * left.  Entered at `esp`; its frame below six pushes and 0x38 bytes. */
u32 moveParticlesOf(App* app, ThreadState& ts, u32 esp)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    auto putFloat = [app](nfs::u32 address, double value) { app->getMemory<float>(address) = float(value); };
    const nfs::u32 frame = esp - 24;  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x38
    const nfs::u32 base = frame - 0x38;
    const nfs::u32 odd = word(0x7d3684) & 1;
    nfs::u32 eax = odd;
    if (odd)
    {
        for (u32 player = 0; player < (gameSettings(app).playMode == 1 ? 2u : 1u); ++player)
        {
            const auto aged = coreAt<0x4cb220, frontend::rende::fadeParticles>(app, ts, base, player);
            if (!aged)
                return 0;
            eax = *aged;
        }
    }
    const double step = app->getMemory<double>(nfs::u32(0x540420));
    auto trig = [&](bool sine, nfs::u32 at) {
        const nfs::Float angle = fp::mul(nfs::Float(floatAt(at)),
                                             nfs::Float(app->getMemory<double>(nfs::u32(sine ? 0x54b6b0 : 0x54b6b8))));
        return double(sine ? fp::sin(angle) : fp::cos(angle));
    };
    for (nfs::u32 i = 0; i < 0x258; ++i)
    {
        const nfs::u32 p = 0x7ddfd0 + i * 0x80;
        const nfs::u32 life = word(p);
        if (life == 0)
            continue;
        const nfs::u32 velocity = p + 0x18;
        const nfs::u32 position = p + 0xc;
        if (odd)
        {
            const nfs::u32 kind = 0x560b64 + (word(p + 4) << 7);
            const double damping = floatAt(kind + 0x38);
            const nfs::s32 lived = nfs::s32(word(p + 8) - life);
            const float age = float(M.div(double(lived), double(nfs::s32(word(p + 8)))));
            if (word(p + 0x44) & 0x7fffffff)
            {
                const double s = trig(true, p + 0x40);
                putFloat(p + 0x18, M.add(M.mul(s, floatAt(p + 0x48)), floatAt(p + 0x18)));
                const double c = M.mul(trig(false, p + 0x40), floatAt(p + 0x48));
                const float swirl = float(M.mul(floatAt(p + 0x4c), damping));
                const float turn = float(M.mul(damping, floatAt(p + 0x44)));
                app->getMemory<float>(p + 0x4c) = swirl;
                app->getMemory<float>(p + 0x44) = turn;
                putFloat(p + 0x20, M.add(c, floatAt(p + 0x20)));
                putFloat(p + 0x48, M.add(double(swirl), floatAt(p + 0x48)));
                putFloat(p + 0x40, M.add(double(turn), floatAt(p + 0x40)));
            }
            const nfs::u32 spinBits = word(p + 0x34);
            if (spinBits & 0x7fffffff)
            {
                if (word(kind + 0x3c) != 0)
                {
                    const double spin = floatAt(p + 0x34);
                    const nfs::u32 size = (0.0 <= spin || spin != spin) ? spinBits : asBits(float(-spin));
                    const double angle = floatAt(p + 0x30);
                    nfs::u32 bounced;
                    if (angle > app->getMemory<double>(nfs::u32(0x540410)))
                        bounced = asBits(-asFloat(size));
                    else
                    {
                        bounced = angle >= app->getMemory<double>(nfs::u32(0x540418)) ? spinBits : size;
                    }
                    word(p + 0x34) = bounced;
                }
                putFloat(p + 0x30, M.add(floatAt(p + 0x34), floatAt(p + 0x30)));
                const double c = trig(false, p + 0x30);
                app->getMemory<float>(p + 0x38) = float(c);
                const double s = trig(true, p + 0x30);
                const float damped = float(M.mul(floatAt(p + 0x34), damping));
                app->getMemory<float>(p + 0x3c) = float(s);
                app->getMemory<float>(p + 0x34) = damped;
            }
            putFloat(p + 0x24, M.add(M.mul(floatAt(p + 0x2c), age), floatAt(p + 0x28)));
            if ((byte(0x7d3684) & 2) && word(kind + 0x50) != 0)
            {
                for (int c = 3; c >= 0; --c)
                {
                    const nfs::s32 fade = fp::toInteger<nfs::s32>(
                        nfs::Float(M.mul(double(byte(kind + 0x50 + nfs::u32(c))), age)));
                    byte(p + 0x54 + nfs::u32(c)) = nfs::u8(byte(kind + 0x4c + nfs::u32(c)) - nfs::u8(fade));
                }
            }
            const double vx = M.mul(floatAt(velocity), damping);
            const double fallen = M.add(floatAt(kind + 0x2c), floatAt(velocity + 4));
            const double vz = M.mul(floatAt(velocity + 8), damping);
            const double vy = M.mul(damping, fallen);
            putFloat(velocity, vx);
            putFloat(velocity + 8, vz);
            putFloat(velocity + 4, vy);
            word(p) = word(p) - 1;
            if (byte(p + 0x57) == 0)
                word(p) = 0;
            if (byte(p + 0x7e) != 0 && word(p) == 0)
            {
                // a copy of the particle on the stack for sub_4cab60 (ret 0x80)
                std::memmove(&app->getMemory<nfs::u8>(base - 0x80), &app->getMemory<nfs::u8>(p), 0x80);
                if (!coreAt<0x4cab60, frontend::sime::particleChildren>(app, ts, base - 0x80, base - 0x80))
                    return 0;
            }
        }
        putFloat(position, M.add(M.mul(floatAt(velocity), step), floatAt(position)));
        putFloat(position + 4, M.add(M.mul(floatAt(velocity + 4), step), floatAt(position + 4)));
        putFloat(position + 8, M.add(M.mul(step, floatAt(velocity + 8)), floatAt(position + 8)));
        eax = position;
    }
    return eax;
}

nfs::u32 moveParticlesNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = moveParticlesOf(app, ts, ts.sp);
    if (ts.terminate)
        return args.eax;
    args.eax = eax;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 scaleColour(nfs::u32 colour, nfs::u32 scale);

/* The projection inline in the game's drawing: x, y, z in view space to the
 * screen record at `record` -- x, y on the screen, z, 1/z -- as sub_41bf30
 * and the rest write it, z of zero first made 0x37800080. */
inline void projectToScreen(nfs::App* app, const SingleMath& M, nfs::u32 record, float x, float y,
                            float& z)
{
    if (!(asBits(z) & 0x7fffffff))
        z = asFloat(0x37800080);
    const float inverse = float(M.div(1.0, z));
    const double k1 = app->getMemory<float>(nfs::u32(0x56009c));
    const double k2 = app->getMemory<float>(nfs::u32(0x5600a0));
    app->getMemory<float>(record) =
        float(M.add(M.mul(M.mul(k1, inverse), x), app->getMemory<float>(nfs::u32(0x5600a4))));
    app->getMemory<float>(record + 4) =
        float(M.add(M.mul(M.mul(k2, inverse), y), app->getMemory<float>(nfs::u32(0x5600a8))));
    app->getMemory<float>(record + 8) = z;
    app->getMemory<float>(record + 0xc) = inverse;
}

/* sub_492370: a light's glow -- a headlight's, a brake light's, a flasher's --
 * as a turned square in the sorted transparent buckets (sub_4c7610).
 *   eax  the view (matrix at +0x44, camera at +0x38); [esp+4..0xc]  the
 *   light; [esp+0x10]  its colour; [esp+0x14]  its size; [esp+0x18]  its
 *   depth (the light's own z when 0); [esp+0x1c]  added to that (ret 0x1c)
 * Nothing for a size of 0 or less, beyond 300 or behind the view, or when the
 * point at the pushed depth is off the screen.  The colour is scaled by the
 * alpha intensity slider below 1 (the [0x7a3a58] & 0x40 test forced on,
 * the port's alpha intensity change); the square, of the size scaled by the
 * light's own 1/z, is turned by (1 + x/z) * [0x53bd00], its texture 0x8b4840.
 * The arguments are written as the original writes them.  Returns what the
 * code leaves in eax: fnstsw's over the view or the light's end for a light
 * left out, the 1/z or x the inline clip code last read for one off the
 * screen, else sub_4c7610's.  Entered at `esp`; its frame below six pushes. */
u32 lightGlowOf(App* app, ThreadState& ts, u32 esp, u32 view)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 ebp = esp - 24;  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp
    word(ebp - 0x2c) = 0;
    const double size = floatAt(ebp + 0x2c);
    if (0.0 >= size)
        return statusAfterCompare(view, 0.0, size);
    const float* m = &app->getMemory<float>(view + 0x44);
    const float p0 = floatAt(ebp + 0x1c), p1 = floatAt(ebp + 0x20), p2 = floatAt(ebp + 0x24);
    float x = float(M.add(M.add(M.mul(p0, m[0]), M.mul(p1, m[3])), M.mul(p2, m[6])));
    float y = float(M.add(M.add(M.mul(p0, m[1]), M.mul(p1, m[4])), M.mul(p2, m[7])));
    float z = float(M.add(M.add(M.mul(p0, m[2]), M.mul(p1, m[5])), M.mul(p2, m[8])));
    x = float(M.add(x, floatAt(view + 0x38)));
    y = float(M.add(y, floatAt(view + 0x3c)));
    z = float(M.add(z, floatAt(view + 0x40)));
    app->getMemory<float>(ebp - 0x40) = x;
    app->getMemory<float>(ebp - 0x3c) = y;
    app->getMemory<float>(ebp - 0x38) = z;
    const nfs::u32 afterTurn = ebp + 0x28;  // lea eax, [eax + 0xc] past the light's three
    if (nfs::s32(asBits(z)) > 0x43960000)  // cmp ecx, 300.0
        return afterTurn;
    if (0.0 >= z)
        return statusAfterCompare(afterTurn, 0.0, z);
    const nfs::u32 record = ebp - 0x60;
    projectToScreen(app, M, record, x, y, z);
    app->getMemory<float>(ebp - 0x38) = z;
    app->getMemory<nfs::u8>(record + 0x14) = screenCode(app, record);
    const nfs::u32 lightInverse = word(record + 0xc);
    word(ebp - 4) = lightInverse;
    if (!(word(ebp + 0x30) & 0x7fffffff))
        word(ebp + 0x30) = word(record + 8);
    const double depth = M.add(floatAt(ebp + 0x30), floatAt(ebp + 0x34));
    app->getMemory<float>(ebp + 0x30) = float(depth);
    app->getMemory<float>(record + 0xc) = float(M.div(1.0, depth));
    word(record + 8) = word(ebp + 0x30);
    const nfs::u8 code = screenCode(app, record);
    app->getMemory<nfs::u8>(record + 0x14) = code;
    if (code != 0)
    {
        // eax as the inline code left it: 1/z behind the near plane, else x
        const nfs::u32 inverse = word(record + 0xc);
        const bool behind = (inverse & 0x80000000) || nfs::s32(inverse) >= nfs::s32(word(0x7d34f8));
        return behind ? inverse : word(record);
    }
    // [0x7a3a58] & 0x40 forced on (the port's alpha intensity change)
    const double alpha = gameSettings(app).alphaIntensity;
    if (1.0 > alpha)
    {
        const nfs::u32 scale = nfs::u32(fp::toInteger<nfs::s32>(
            nfs::Float(M.mul(alpha, app->getMemory<double>(nfs::u32(0x53bcf8))))));
        word(ebp + 0x28) = scaleColour(word(ebp + 0x28), scale);
    }
    word(ebp - 0x50) = word(ebp + 0x28);

    const auto room = commitArena(app, ts, ebp - 0x60, ebp, ebp - 0x2c, 0xa0);
    if (!room)
        return 0;
    const nfs::u32 polygon = word(ebp - 0x2c);
    if (*room != 0)
    {
        const nfs::u32 corners[4] = { polygon + 0x20, polygon + 0x40, polygon + 0x60, polygon + 0x80 };
        const double slope = M.div(x, floatAt(ebp - 0x38));
        const double k1i = M.mul(floatAt(0x56009c), asFloat(lightInverse));
        const double k2i = M.mul(asFloat(lightInverse), floatAt(0x5600a0));
        const float width = float(M.mul(k1i, size));
        const float height = float(M.mul(size, k2i));
        app->getMemory<nfs::u16>(polygon + 4) = 0;
        word(polygon) = 0;
        app->getMemory<nfs::u16>(polygon + 6) = 1;
        word(polygon + 0x18) = 0x8b4840;
        for (nfs::u32 c = 0; c < 4; ++c)
            word(polygon + 8 + c * 4) = corners[c];
        word(polygon + 0x1c) = word(record + 8);
        const float slopeFloat = float(slope);
        for (int c = 3; c >= 0; --c)
            std::memmove(&app->getMemory<nfs::u8>(corners[c]), &app->getMemory<nfs::u8>(record), 0x20);
        const float angle = float(M.mul(M.add(1.0, slopeFloat), app->getMemory<float>(nfs::u32(0x53bd00))));
        word(ebp - 8) = asBits(angle);
        word(ebp - 0x24) = asBits(width);
        word(ebp - 0x20) = asBits(height);
        const nfs::Float turned = fp::mul(nfs::Float(angle), nfs::Float(app->getMemory<double>(nfs::u32(0x54b6b0))));
        const float sw = float(M.mul(double(fp::sin(turned)), width));
        const nfs::Float turnedToo = fp::mul(nfs::Float(angle), nfs::Float(app->getMemory<double>(nfs::u32(0x54b6b8))));
        const double ch = M.mul(double(fp::cos(turnedToo)), height);
        word(ebp - 0x10) = asBits(sw);
        auto move = [&](nfs::u32 corner, double dx, double dy) {
            const double cx = floatAt(corner), cy = floatAt(corner + 4);
            app->getMemory<float>(corner) = float(M.add(cx, dx));
            app->getMemory<float>(corner + 4) = float(M.add(cy, dy));
        };
        // v0: x - sw, y + ch; v1: x + ch, y + sw; v2: x + sw, y - ch; v3: x - ch, y - sw
        {
            const double cx = floatAt(corners[0]), cy = floatAt(corners[0] + 4);
            app->getMemory<float>(corners[0]) = float(M.sub(cx, sw));
            app->getMemory<float>(corners[0] + 4) = float(M.add(cy, ch));
        }
        move(corners[1], ch, sw);
        {
            const double cx = floatAt(corners[2]), cy = floatAt(corners[2] + 4);
            app->getMemory<float>(corners[2]) = float(M.add(cx, sw));
            app->getMemory<float>(corners[2] + 4) = float(M.sub(cy, ch));
        }
        {
            const double cx = floatAt(corners[3]), cy = floatAt(corners[3] + 4);
            app->getMemory<float>(corners[3]) = float(M.sub(cx, ch));
            app->getMemory<float>(corners[3] + 4) = float(M.sub(cy, sw));
        }
        for (int c = 0; c < 4; ++c)
            app->getMemory<nfs::u8>(corners[c] + 0x14) = screenCode(app, corners[c]);
    }
    // pushed with the bucket's start, from the depth of the call before
    return sortedBucketsOf(app, ts, ebp - 0x60 - 8, polygon, word(0x7dcffc));
}

nfs::u32 lightGlowNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    const u32 eax = lightGlowOf(app, ts, esp, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = eax;
    ts.sp = esp + 4 + 0x1c;
    return args.eax;
}

/* st(0) toward zero, then to an integer as fistp stores it (sub_4dfd56). */
inline nfs::s32 truncated(double value)
{
    return fp::toInteger<nfs::s32>(std::trunc(value));
}

/* sub_492980: a headlight's beam, a cone of quads between a ring at the lamp
 * and one out along the beam, in the sorted transparent buckets.
 *   eax  the view (its camera at +8.. and +0x38.., matrix at +0x44);
 *   [esp+4..0xc]  the lamp; [esp+0x10]  the car's matrix; [esp+0x14]  the
 *   colour; [esp+0x18]  the length; [esp+0x1c]  the far ring's radius;
 *   [esp+0x20]  the near one's; [esp+0x24]  the texture's v; [esp+0x28]
 *   added to each quad's depth (ret 0x28)
 * Nothing when Effects is low ([0x6fbc30] 2), for a beam of no length, or
 * from further than [0x53bd04].  The rings have 3 to 12 segments by how far
 * off the beam the camera looks, the colour fades as the camera looks into
 * it and by the alpha intensity slider (the [0x7a3a58] & 0x40 test forced on,
 * the port's alpha intensity change); a quad all off one side of the screen is
 * left out, the rest hung on a list, last first, each keyed by its deepest
 * corner.  Entered at `esp`; the rings and the arena's slot in its frame. */
void headlightBeamOf(App* app, ThreadState& ts, u32 esp, u32 view)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 ebp = esp - 24 - 0x5a;  // six pushes; mov ebp, esp; sub ebp, 0x5a
    const nfs::u32 args = esp + 4;
    const float lamp[3] = { floatAt(args), floatAt(args + 4), floatAt(args + 8) };
    const nfs::u32 carMatrix = word(args + 0xc);
    nfs::u32 colour = word(args + 0x10);
    const nfs::u32 lengthBits = word(args + 0x14);
    const float farRadius = floatAt(args + 0x18);
    const float nearRadius = floatAt(args + 0x1c);
    const float textureV = floatAt(args + 0x20);
    const float depthBias = floatAt(args + 0x24);
    word(ebp + 0x1a) = 0;
    word(ebp - 0x32) = 0;
    if (gameSettings(app).effectsLevel == 2 || !(lengthBits & 0x7fffffff))
        return;
    float d[3] = { float(M.sub(floatAt(view + 8), lamp[0])), float(M.sub(floatAt(view + 0xc), lamp[1])),
                   float(M.sub(floatAt(view + 0x10), lamp[2])) };
    const float distance = float(M.sqrt(M.add(M.add(M.mul(d[1], d[1]), M.mul(d[0], d[0])), M.mul(d[2], d[2]))));
    if (!(asBits(distance) & 0x7fffffff) || distance > double(floatAt(0x53bd04)))
        return;
    {
        const double inverse = M.div(1.0, distance);
        d[0] = float(M.mul(d[0], inverse));
        d[1] = float(M.mul(d[1], inverse));
        d[2] = float(M.mul(inverse, d[2]));
    }
    auto dot = [&](nfs::u32 w) {
        const float* v = &app->getMemory<float>(w);
        return float(M.add(M.add(M.mul(d[0], v[0]), M.mul(d[1], v[1])), M.mul(d[2], v[2])));
    };
    const float c1 = dot(carMatrix + 0x18);
    const float c2 = dot(carMatrix);
    const float c3 = dot(carMatrix + 0xc);
    const double k8 = floatAt(0x53bd08);
    const double segments = M.mul(M.sub(1.0, M.mul(distance, floatAt(0x53bd10))), floatAt(0x53bd14));
    const float start = float(M.mul(k8, M.sub(M.sub(floatAt(0x53bd0c), M.mul(c2, k8)), M.mul(c3, k8))));
    const nfs::s32 raw = truncated(segments);
    const nfs::s32 n = raw > 12 ? 12 : (raw >= 3 ? raw : 3);
    const nfs::s32 count = n + 1;

    // sub_4ea9c0: the half angle of the cone
    double angle;
    {
        angle = M.mul(double(fp::atan(nfs::Float(asFloat(lengthBits)), nfs::Float(nearRadius))),
                       app->getMemory<double>(nfs::u32(0x54b6c0)));
    }
    const float spread = float(M.mul(angle, floatAt(0x53bd18)));
    float width;
    if (!(1.0 < spread || spread != spread) && floatAt(0x53bd1c) > spread)
        width = asFloat(0x3d4ccccd);
    else
        width = 1.0 >= spread ? spread : 1.0f;
    if (!(M.sub(1.0, width) >= c1))
    {
        const double into = M.sub(c1, M.sub(1.0, width));
        const double fade = M.sub(1.0, M.mul(M.div(1.0, width), into));
        const nfs::u32 a = nfs::u32(truncated(M.mul(double((colour >> 24) & 0xff), fade))) & 0xff;
        const nfs::u32 r = nfs::u32(truncated(M.mul(double((colour >> 16) & 0xff), fade))) & 0xff;
        const nfs::u32 b = nfs::u32(truncated(M.mul(double(colour & 0xff), fade))) & 0xff;
        const nfs::u32 g = nfs::u32(truncated(M.mul(fade, double((colour >> 8) & 0xff)))) & 0xff;
        colour = a << 24 | r << 16 | g << 8 | b;
    }
    // [0x7a3a58] & 0x40 forced on (the port's alpha intensity change)
    const double alpha = gameSettings(app).alphaIntensity;
    if (1.0 > alpha)
        colour = scaleColour(colour, nfs::u32(fp::toInteger<nfs::s32>(
                                         nfs::Float(M.mul(alpha, app->getMemory<double>(nfs::u32(0x53bd20)))))));
    const float step = float(M.div(1.0, double(n)));

    // The two rings: points round the beam, to the car, the world, the view, the screen.
    auto ring = [&](nfs::u32 records, float radius, bool lit) {
        const float* m = &app->getMemory<float>(carMatrix);
        const float* v = &app->getMemory<float>(view + 0x44);
        const float camera[3] = { floatAt(view + 0x38), floatAt(view + 0x3c), floatAt(view + 0x40) };
        float a = start;
        for (nfs::s32 i = 0; i < count; ++i)
        {
            const double c = M.mul(double(fp::cos(fp::mul(nfs::Float(a),
                                                                   nfs::Float(app->getMemory<double>(nfs::u32(0x54b6b8)))))),
                                    radius);
            const double s = M.mul(double(fp::sin(fp::mul(nfs::Float(a),
                                                                   nfs::Float(app->getMemory<double>(nfs::u32(0x54b6b0)))))),
                                    radius);
            const float p[3] = { float(c), float(s), lit ? 0.0f : asFloat(lengthBits) };
            a = float(M.add(a, step));
            float q[3];
            for (int j = 0; j < 3; ++j)
                q[j] = float(M.add(M.add(M.mul(p[0], m[j]), M.mul(p[1], m[3 + j])), M.mul(p[2], m[6 + j])));
            for (int j = 0; j < 3; ++j)
                q[j] = float(M.add(q[j], lamp[j]));
            float w[3];
            for (int j = 0; j < 3; ++j)
                w[j] = float(M.add(M.add(M.mul(q[0], v[j]), M.mul(q[1], v[3 + j])), M.mul(q[2], v[6 + j])));
            for (int j = 0; j < 3; ++j)
                w[j] = float(M.add(camera[j], w[j]));
            const nfs::u32 record = records + nfs::u32(i) * 0x20;
            projectToScreen(app, M, record, w[0], w[1], w[2]);
            app->getMemory<nfs::u8>(record + 0x14) = screenCode(app, record);
            word(record + 0x10) = lit ? colour : 0;
            app->getMemory<float>(record + 0x18) = float(M.div(double(i), double(n)));
            app->getMemory<float>(record + 0x1c) = lit ? float(M.sub(1.0, textureV)) : -textureV;
        }
    };
    const nfs::u32 nearRing = ebp - 0x3ba;
    const nfs::u32 farRing = ebp - 0x55a;
    ring(nearRing, nearRadius, false);
    ring(farRing, farRadius, true);

    const nfs::u32 base = ebp + 0x5a - 0x5b4;
    nfs::u32 head = 0;
    for (nfs::s32 j = 0; j < n; ++j)
    {
        const auto room = commitArena(app, ts, base, ebp, ebp - 0x32, 0xa0);
        if (!room || *room == 0)
            return;
        const nfs::u32 polygon = word(ebp - 0x32);
        const nfs::u32 sources[4] = { nearRing + nfs::u32(j + 1) * 0x20, nearRing + nfs::u32(j) * 0x20,
                                        farRing + nfs::u32(j) * 0x20, farRing + nfs::u32(j + 1) * 0x20 };
        const nfs::u8 shared = app->getMemory<nfs::u8>(sources[0] + 0x14) & app->getMemory<nfs::u8>(sources[1] + 0x14)
                                 & app->getMemory<nfs::u8>(sources[2] + 0x14) & app->getMemory<nfs::u8>(sources[3] + 0x14);
        if (shared)
        {
            word(ebp - 0x32) = head;
            continue;
        }
        app->getMemory<nfs::u16>(polygon + 4) = 0;
        word(polygon) = head;
        app->getMemory<nfs::u16>(polygon + 6) = 3;
        word(polygon + 0x18) = 0x8b47e8;
        for (nfs::u32 c = 0; c < 4; ++c)
        {
            word(polygon + 8 + c * 4) = polygon + 0x20 + c * 0x20;
            std::memmove(&app->getMemory<nfs::u8>(polygon + 0x20 + c * 0x20), &app->getMemory<nfs::u8>(sources[c]),
                         0x20);
        }
        const double z0 = floatAt(polygon + 0x28), z1 = floatAt(polygon + 0x48);
        const double z2 = floatAt(polygon + 0x68), z3 = floatAt(polygon + 0x88);
        const double z23 = z2 > z3 ? z2 : z3;
        const double z123 = z1 > z23 ? z1 : z23;
        const double deepest = z0 > z123 ? z0 : z123;
        app->getMemory<float>(polygon + 0x1c) = float(M.add(deepest, depthBias));
        head = polygon;
    }
    // pushed with the bucket's start, from the depth of the calls before
    sortedBucketsOf(app, ts, base - 8, word(ebp - 0x32), word(0x7dcffc));
}

nfs::u32 headlightBeamNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    headlightBeamOf(app, ts, esp, args.eax);
    if (ts.terminate)
        return args.eax;
    ts.sp = esp + 4 + 0x28;
    return args.eax;
}

float viewZoom(nfs::App* app, nfs::u32 view);
void signedDivide(nfs::s64 dividend, nfs::s32 divisor, nfs::u32& quotient, nfs::u32& remainder);

/* sub_4bb5d0: which model a car is drawn with in a view pass.
 *   eax  the view; edx  the car; ebx  where its mean depth goes
 * -1 (not drawn) for a car not to be drawn (+0x89 clear, +0x88 set), for the
 * view's own car under the in-car and bumper cameras or in the mirror, for one
 * whose block is out of view (sub_41e070), and for one whose box -- its eight
 * corners from the half sizes at +0x114, turned by its matrix (+0xc0) and the
 * view's, projected into 0x7a3bf0 -- is wholly off one side of the screen;
 * otherwise 0 to 3 by the mean depth of the corners against the Car Detail
 * row of 0x55fdf4 (times 0.75 in split screen or at night, times the mirror's
 * factor -- nfs3hp::mirrorCarScale -- in the mirror, times the camera's
 * zoom), or -1 past the last.  Entered at `esp`; its frame below four
 * pushes and 0xd8 bytes. */
u32 carDetailOf(App* app, ThreadState& ts, u32 esp, u32 view, u32 car, u32 out)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    const nfs::u32 base = esp - 16 - 0xd8;  // push ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xd8
    const float zoom = viewZoom(app, view);
    if (!byte(car + 0x89) || byte(car + 0x88) != 0)
        return 0xffffffff;
    const auto own = coreAt<0x422ba0, carstate::carOfCamera>(app, ts, base, word(view + 4));
    if (!own)
        return 0;
    if (*own == car)
    {
        const auto mode = coreAt<0x423b00, carstate::modeOfCamera>(app, ts, base, word(view + 4));
        if (!mode)
            return 0;
        if (*mode == 1 || word(view) == 1 || *mode == 2)
            return 0xffffffff;
    }
    const auto shown = coreAt<0x41e070, track::blockDrawn>(app, ts, base, view, nfs::u32(nfs::s32(word(car + 0x1c)) >> 3));
    if (!shown)
        return 0;
    if (*shown == 0)
        return 0xffffffff;
    app->getMemory<u32>(out) = 0;
    const nfs::u32 row = 0x55fdf4 + (gameSettings(app).detailLevel << 4);
    float t[4] = { floatAt(row), floatAt(row + 4), floatAt(row + 8), floatAt(row + 0xc) };
    if (gameSettings(app).playMode == 1 || gameSettings(app).night != 0)
    {
        const double k = app->getMemory<double>(nfs::u32(0x540130));
        t[1] = float(M.mul(t[1], k));
        t[2] = float(M.mul(k, t[2]));
        t[0] = float(M.mul(t[0], k));
    }
    if (word(view) == 1)
    {
        const double k = mirrorCarScale(app->getMemory<double>(nfs::u32(0x540138)));
        for (int i = 0; i < 3; ++i)
            t[i] = float(M.mul(t[i], k));
        t[3] = float(M.mul(k, t[3]));
    }
    if (asBits(zoom) != 0x3f800000)
    {
        for (int i = 0; i < 3; ++i)
            t[i] = float(M.mul(t[i], zoom));
        t[3] = float(M.mul(zoom, t[3]));
    }
    const float e0 = floatAt(car + 0x114), e1 = floatAt(car + 0x118), e2 = floatAt(car + 0x11c);
    const float corners[8][3] = { { -e0, e1, e2 },  { e0, e1, e2 },  { e0, e1, -e2 },  { -e0, e1, -e2 },
                                  { -e0, -e1, e2 }, { e0, -e1, e2 }, { e0, -e1, -e2 }, { -e0, -e1, -e2 } };
    // the car's place in the view (sub_4e0430) and its matrix turned by the view's (sub_4e0280)
    const float* v = &app->getMemory<float>(view + 0x44);
    const float* position = &app->getMemory<float>(car + 0x98);
    float place[3];
    for (int j = 0; j < 3; ++j)
        place[j] = float(M.add(M.add(M.mul(position[0], v[j]), M.mul(position[1], v[3 + j])), M.mul(position[2], v[6 + j])));
    for (int j = 0; j < 3; ++j)
        place[j] = float(M.add(place[j], floatAt(view + 0x38 + nfs::u32(j) * 4)));
    const float* turn = &app->getMemory<float>(car + 0xc0);
    float r[9];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i * 3 + j] = float(M.add(M.add(M.mul(turn[i * 3], v[j]), M.mul(turn[i * 3 + 1], v[3 + j])),
                                        M.mul(turn[i * 3 + 2], v[6 + j])));
    float sum = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        float p[3];
        for (int j = 0; j < 3; ++j)
            p[j] = float(M.add(M.add(M.mul(corners[i][0], r[j]), M.mul(corners[i][1], r[3 + j])),
                                M.mul(corners[i][2], r[6 + j])));
        for (int j = 0; j < 3; ++j)
            p[j] = float(M.add(p[j], place[j]));
        const nfs::u32 record = 0x7a3bf0 + nfs::u32(i) * 0x20;
        projectToScreen(app, M, record, p[0], p[1], p[2]);
        app->getMemory<nfs::u8>(record + 0x14) = screenCode(app, record);
        sum = float(M.add(sum, p[2]));
    }
    const nfs::u32 near = byte(0x7a3c04) & byte(0x7a3c24) & byte(0x7a3c44);
    if ((near & byte(0x7a3c64)) && (byte(0x7a3c84) & byte(0x7a3ca4) & byte(0x7a3cc4) & byte(0x7a3ce4)))
        return 0xffffffff;
    const float mean = float(M.mul(sum, app->getMemory<double>(nfs::u32(0x540140))));
    app->getMemory<u32>(out) = asBits(mean);
    if ((asBits(t[0]) & 0x7fffffff) && !(mean >= t[0]))
        return 0;
    for (int i = 1; i < 4; ++i)
        if (!(mean >= t[i]))
            return nfs::u32(i);
    return 0xffffffff;
}

nfs::u32 carDetailNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 level = carDetailOf(app, ts, ts.sp, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = level;
    ts.sp += 4;
    return args.eax;
}

/* sub_4dbef0: an object's vertices to the screen with their colours.
 *   eax  the matrix; edx  the move; ebx  the count; ecx  the vertices in,
 *   12 bytes each; [esp+4]  the colours; [esp+8]  the screen records out,
 *   0x20 bytes each (ret 8)
 * More than 20 go to sub_4bf4c0 (objectVertices) at once, fewer
 * one by one, turned, moved and projected inline; each record then takes its
 * colour at +0x10.  Returns what the code leaves in eax: past the records
 * out, or for the inline loop past the vertices read (the matrix for none). */
u32 objectVerticesListOf(App* app, ThreadState& ts, u32 matrix, u32 move, u32 count, u32 vertices,
                           u32 colours, u32 out)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    if (nfs::s32(count) > 0x14)
    {
        objectVerticesOf(app, ts, i32(count), vertices, matrix, move, out);
        for (nfs::u32 left = count; left != 0; --left, colours += 4, out += 0x20)
            app->getMemory<u32>(out + 0x10) = word(colours);
        return out;
    }
    nfs::u32 in = vertices;
    const float* m = &app->getMemory<float>(matrix);
    const float* t = &app->getMemory<float>(move);
    for (nfs::u32 left = count; left != 0; --left, colours += 4, out += 0x20, in += 0xc)
    {
        const float* v = &app->getMemory<float>(in);
        float p[3];
        for (int j = 0; j < 3; ++j)
            p[j] = float(M.add(M.add(M.mul(v[0], m[j]), M.mul(v[1], m[3 + j])), M.mul(v[2], m[6 + j])));
        for (int j = 0; j < 3; ++j)
            p[j] = float(M.add(p[j], t[j]));
        projectToScreen(app, M, out, p[0], p[1], p[2]);
        app->getMemory<nfs::u8>(out + 0x14) = screenCode(app, out);
        app->getMemory<u32>(out + 0x10) = word(colours);
    }
    return count != 0 ? in : matrix;
}

nfs::u32 objectVerticesListNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    args.eax = objectVerticesListOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, app->getMemory<u32>(esp + 4),
                                   app->getMemory<u32>(esp + 8));
    ts.sp = esp + 4 + 8;
    return args.eax;
}

/* sub_41b640: the polygons of a piece of an object, four corners each, as
 * records for the frame's polygon list -- as sub_41b300 does for the track,
 * written one after another at `record`, with the corners taken the other way
 * round: `count` polygons at `polygon`, 14 bytes each, their corners among
 * the vertex records at `base`, 0x20 bytes each.  Returns how many records
 * were written. */
u32 objectRecordsOf(App* app, ThreadState& ts, nfs::s32 count, u32 polygon, u32 record, u32 base)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto half = [app](nfs::u32 address) { return app->getMemory<nfs::u16>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    auto floatAt = [app](nfs::u32 address) -> float { return app->getMemory<float>(address); };
    nfs::u32 written = 0;
    const double farthest = floatAt(word(0x5dd830) + 0x30);
    const nfs::u32 textures = word(0x552df4);
    for (nfs::s32 i = 0; i < count; ++i, polygon += 0xe)
    {
        const nfs::u32 v0 = base + (nfs::u32(nfs::s32(word(polygon)) >> 16) << 5);
        const nfs::u32 v1 = base + (nfs::u32(nfs::s32(nfs::s16(half(polygon)))) << 5);
        const nfs::u32 v2 = base + (nfs::u32(nfs::s32(word(polygon + 4)) >> 16) << 5);
        const nfs::u32 v3 = base + (nfs::u32(nfs::s32(word(polygon + 2)) >> 16) << 5);
        const nfs::u8 flags8 = byte(polygon + 0xc);
        const nfs::u32 c0 = byte(v0 + 0x14), c1 = byte(v1 + 0x14), c2 = byte(v2 + 0x14), c3 = byte(v3 + 0x14);
        if (c0 & c1 & c2 & c3)
            continue;
        const double z0 = floatAt(v0 + 8);
        if (z0 > farthest)
            continue;
        if (!(flags8 & 0x10) && !((c0 | c1 | c2 | c3) & 0x10))
        {
            const CrossProducts p = crossProducts(ts, floatAt(v2 + 4), floatAt(v1 + 4), floatAt(v0), floatAt(v1),
                                                  floatAt(v0 + 4), floatAt(v1 + 4), floatAt(v2), floatAt(v1));
            const nfs::u32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
            if (back ^ word(0x554e4c))
            {
                const CrossProducts q = crossProducts(ts, floatAt(v3 + 4), floatAt(v2 + 4), floatAt(v0), floatAt(v2),
                                                      floatAt(v0 + 4), floatAt(v2 + 4), floatAt(v3), floatAt(v2));
                const nfs::u32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                if (backToo ^ word(0x554e4c))
                    continue;
            }
        }
        word(record + 0x10) = v2;
        word(record + 8) = v0;
        word(record + 0xc) = v1;
        word(record + 0x14) = v3;
        word(record) = record + 0x20;
        app->getMemory<nfs::u16>(record + 6) = 0;
        word(record + 0x18) = textures + nfs::u32(nfs::s32(word(polygon + 6)) >> 16) * 47;
        app->getMemory<nfs::u16>(record + 4) = 0;
        if (flags8 & 1)
            byte(record + 6) = nfs::u8(byte(record + 6) | 4);
        auto depthSum = [&]() {
            return M.mul(M.add(M.add(M.add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8)), floatAt(v3 + 8)),
                         app->getMemory<double>(nfs::u32(0x536c9c)));
        };
        if ((byte(0x7a3a58) & 6) && gameSettings(app).nearPolygons != 0)
        {
            if (!(depthSum() >= app->getMemory<double>(nfs::u32(0x536ca4))))
                byte(record + 6) = nfs::u8(byte(record + 6) | 8);
        }
        if (flags8 & 4)
        {
            const nfs::u8 animation = byte(polygon + 0xd);
            nfs::u32 frames, step, unused, phase;
            signedDivide(nfs::s32(word(0x7d3684)), nfs::s32(animation >> 3), frames, unused);
            signedDivide(nfs::s32(frames), nfs::s32(animation & 7), step, phase);
            NFS2_USE(step);
            word(record + 0x18) = textures + (phase + nfs::u32(nfs::s32(word(polygon + 8)) >> 16)) * 47;
        }
        if (byte(word(record + 0x18) + 0x28) & 2)
        {
            const nfs::u32 colour = word(0x552e1c);
            word(v0 + 0x10) = colour;
            word(v1 + 0x10) = colour;
            word(v2 + 0x10) = colour;
            word(v3 + 0x10) = colour;
            byte(record + 6) = nfs::u8(byte(record + 6) | 1);
            app->getMemory<float>(record + 0x1c) = float(depthSum());
        }
        record += 0x20;
        ++written;
    }
    return written;
}

/* The thunk: edx the count, ebx the polygons, ecx the records out, [esp+4] the vertex records
 * (ret 4). */
nfs::u32 objectRecordsNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    args.eax = objectRecordsOf(app, ts, nfs::s32(args.edx), args.ebx, args.ecx, app->getMemory<u32>(esp + 4));
    ts.sp = esp + 4 + 4;
    return args.eax;
}

/* The game's small vector helpers (Watcom-style register calls, used from
 * dozens of places, the physics and the opponents' driving among them): each
 * element as the emulated x87 has it in the precision it is in. */

constexpr u32 kVectorBytes = 0xc;

}  // namespace

/* sub_4e0050's work: `count` vectors from `in` times `scale` into `out`, each read whole before it
 * is written. */
void scaleVectorList(App* app, i32 count, u32 in, double scale, u32 out)
{
    const SingleMath M;
    for (i32 k = 0; k < count; ++k, in += kVectorBytes, out += kVectorBytes)
    {
        const float* v = &app->getMemory<float>(in);
        const float x = v[0], y = v[1], z = v[2];
        float* o = &app->getMemory<float>(out);
        o[0] = float(M.mul(x, scale));
        o[1] = float(M.mul(y, scale));
        o[2] = float(M.mul(z, scale));
    }
}

/* scaleVectorList with the eax sub_4e0050 leaves: `out` past the vectors written. */
u32 scaleVectorsEax(App* app, i32 count, u32 in, float scale, u32 out)
{
    scaleVectorList(app, count, in, scale, out);
    return count > 0 ? out + u32(count) * kVectorBytes : out;
}

namespace
{

/* sub_4e0050: scaleVectorList of eax vectors at edx by the float [esp+4] into [esp+8] (ret 8);
 * eax past the vectors out. */
nfs::u32 scaleVectorsNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 out = app->getMemory<u32>(ts.sp + 8);
    scaleVectorList(app, count, args.edx, app->getMemory<float>(ts.sp + 4), out);
    args.eax = count > 0 ? out + u32(count) * kVectorBytes : out;
    ts.sp += 4 + 8;
    return args.eax;
}

}  // namespace

/* sub_4dffb0 (Add) and sub_4e0000's work: `count` vectors from `in`, each with the one at `by`,
 * into `out`, each read whole before it is written. */
template <bool Add>
void offsetVectorList(App* app, i32 count, u32 in, u32 by, u32 out)
{
    const SingleMath M;
    for (i32 k = 0; k < count; ++k, in += kVectorBytes, out += kVectorBytes)
    {
        const float* a = &app->getMemory<float>(in);
        const float* b = &app->getMemory<float>(by);
        const float a0 = a[0], a1 = a[1], a2 = a[2];
        const float b0 = b[0], b1 = b[1], b2 = b[2];
        float* o = &app->getMemory<float>(out);
        o[0] = float(Add ? M.add(a0, b0) : M.sub(a0, b0));
        o[1] = float(Add ? M.add(a1, b1) : M.sub(a1, b1));
        o[2] = float(Add ? M.add(a2, b2) : M.sub(a2, b2));
    }
}

// Both made here, for the other files' calls (native_vertices.h).
template void offsetVectorList<true>(App* app, i32 count, u32 in, u32 by, u32 out);
template void offsetVectorList<false>(App* app, i32 count, u32 in, u32 by, u32 out);

/* offsetVectorList with the eax sub_4dffb0 and sub_4e0000 leave: `in` past the vectors read. */
template <bool Add>
u32 offsetVectorsEax(App* app, i32 count, u32 in, u32 by, u32 out)
{
    offsetVectorList<Add>(app, count, in, by, out);
    return count > 0 ? in + u32(count) * kVectorBytes : in;
}

template u32 offsetVectorsEax<true>(App* app, i32 count, u32 in, u32 by, u32 out);
template u32 offsetVectorsEax<false>(App* app, i32 count, u32 in, u32 by, u32 out);

namespace
{

/* sub_4dffb0 and sub_4e0000: offsetVectorList of eax vectors at edx with ebx into ecx; eax past
 * the vectors read. */
template <bool Add>
nfs::u32 offsetVectorsNative(App* app, ThreadState& ts, nfs::Args& args)
{
    const i32 count = i32(args.eax);
    const u32 in = args.edx;
    offsetVectorList<Add>(app, count, in, args.ebx, args.ecx);
    args.eax = count > 0 ? in + u32(count) * kVectorBytes : in;
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* sub_4e06a0's work: the length of the vector at `vector`, as wide as the x87 holds it. */
double vectorLength(App* app, u32 vector)
{
    const SingleMath M;
    const float* v = &app->getMemory<float>(vector);
    return M.sqrt(M.add(M.add(M.mul(v[1], v[1]), M.mul(v[0], v[0])), M.mul(v[2], v[2])));
}

namespace
{

/* sub_4e06a0: vectorLength of eax onto the x87 stack. */
nfs::u32 vectorLengthNative(App* app, ThreadState& ts, nfs::Args& args)
{
    args.floatSet = true;
    args.floatResult = double(vectorLength(app, args.eax));
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* sub_4e01c0's work: the vector at `from` divided by its length into `to` (a zero one as the x87
 * divides it), read whole before it is written. */
void makeUnitVector(App* app, u32 from, u32 to)
{
    const SingleMath M;
    const double inverse = M.div(1.0, vectorLength(app, from));
    const float* v = &app->getMemory<float>(from);
    const float x = v[0], y = v[1], z = v[2];
    float* o = &app->getMemory<float>(to);
    o[0] = float(M.mul(x, inverse));
    o[1] = float(M.mul(y, inverse));
    o[2] = float(M.mul(inverse, z));
}

namespace
{

/* sub_4e01c0: makeUnitVector of eax into edx. */
nfs::u32 unitVectorNative(App* app, ThreadState& ts, nfs::Args& args)
{
    makeUnitVector(app, args.eax, args.edx);
    ts.sp += 4;
    return args.eax;
}

/* The view's zoom (sub_4227e0): the camera record's +0x54 for the main view
 * when it is set, else 1. */
float viewZoom(nfs::App* app, nfs::u32 view)
{
    if (app->getMemory<nfs::u32>(view) != 0)
        return 1.0f;
    const nfs::u32 index = app->getMemory<nfs::u32>(view + 4) * 4;
    nfs::u32 camera = app->getMemory<nfs::u32>(0x5e10b8 + index);
    if (!camera)
        camera = app->getMemory<nfs::u32>(0x5e10b0 + index);
    const nfs::u32 zoom = app->getMemory<nfs::u32>(camera + 0x54);
    return (zoom & 0x7fffffff) ? asFloat(zoom) : 1.0f;
}

/* sub_41bf30: whether a box -- four corners -- lies wholly outside the view.
 *   eax  the view; edx  the four corners, 12 bytes each; ebx  a 3x3 matrix;
 *   ecx  a move
 * Each corner is turned and moved (as sub_4e0430 does it), its 1/z and screen
 * position worked out from [0x56009c..a8], and its clip code from the screen
 * bounds at [0x7d34f8..0x7d350c], compared as integers; then x becomes |x|
 * times the zoom, held to 1.  Returns 1 when every corner has |x| beyond its z
 * and the four share a clip bit, else 0.  Only its stack is written. */
u32 boxOutsideOf(App* app, u32 view, u32 corners, u32 matrix, u32 move)
{
    const SingleMath M;
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    const double zoom = viewZoom(app, view);
    const double scale = 1.0 >= zoom ? zoom : 1.0;

    const double k1 = app->getMemory<float>(nfs::u32(0x56009c));
    const double k2 = app->getMemory<float>(nfs::u32(0x5600a0));
    const double c1 = app->getMemory<float>(nfs::u32(0x5600a4));
    const double c2 = app->getMemory<float>(nfs::u32(0x5600a8));
    const nfs::s32 nearest = nfs::s32(word(0x7d34f8));
    const nfs::s32 top = nfs::s32(word(0x7d3504)), bottom = nfs::s32(word(0x7d3500));
    const nfs::s32 left = nfs::s32(word(0x7d34fc)), right = nfs::s32(word(0x7d350c));
    float xs[4], zs[4];
    nfs::u8 codes[4];
    for (int i = 0; i < 4; ++i)
    {
        const float* v = &app->getMemory<float>(corners + nfs::u32(i) * 12);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        const float* m = &app->getMemory<float>(matrix);
        float x = float(M.add(M.add(M.mul(v0, m[0]), M.mul(v1, m[3])), M.mul(v2, m[6])));
        float y = float(M.add(M.add(M.mul(v0, m[1]), M.mul(v1, m[4])), M.mul(v2, m[7])));
        float z = float(M.add(M.add(M.mul(v0, m[2]), M.mul(v1, m[5])), M.mul(v2, m[8])));
        const float* t = &app->getMemory<float>(move);
        x = float(M.add(t[0], x));
        y = float(M.add(t[1], y));
        z = float(M.add(t[2], z));
        if (!(asBits(z) & 0x7fffffff))
            z = asFloat(0x37800080);
        const float inverse = float(M.div(1.0, z));
        const float sx = float(M.add(M.mul(M.mul(k1, inverse), x), c1));
        const float sy = float(M.add(M.mul(M.mul(k2, inverse), y), c2));
        const nfs::u32 inverseBits = asBits(inverse);
        nfs::u8 code = 0x10;
        if (!(inverseBits & 0x80000000) && nfs::s32(inverseBits) < nearest)
        {
            code = 0;
            const nfs::u32 yb = asBits(sy);
            if ((yb & 0x80000000) || nfs::s32(yb) < top)
                code |= 8;
            else if (nfs::s32(yb) > bottom)
                code |= 4;
            const nfs::u32 xb = asBits(sx);
            if ((xb & 0x80000000) || nfs::s32(xb) < left)
                code |= 1;
            else if (nfs::s32(xb) > right)
                code |= 2;
        }
        codes[i] = code;
        const float size = (0.0 <= x || x != x) ? x : -x;
        xs[i] = float(M.mul(size, scale));
        zs[i] = z;
    }

    for (int i = 0; i < 4; ++i)
        if (!(xs[i] > zs[i]))
            return 0;
    return (codes[0] & codes[1] & codes[2] & codes[3]) ? 1 : 0;
}

nfs::u32 boxOutsideNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = boxOutsideOf(app, args.eax, args.edx, args.ebx, args.ecx);
    ts.sp += 4;
    return args.eax;
}

/* The brightness the projected light is scaled by (sub_41a120), applied to a
 * colour as sub_41a3e0 darkens one. */
nfs::u32 scaleColour(nfs::u32 colour, nfs::u32 scale)
{
    const nfs::u32 clamped = nfs::s32(scale) >= 0x10000 ? 0xffff : scale;
    const nfs::u32 level = (clamped >> 8) & 0xff;
    const nfs::u32 redBlue = (colour & 0xff00ff) * level;
    const nfs::u32 alphaGreen = ((colour & 0xff00ff00) >> 8) * level;
    return (alphaGreen & 0xff00ff00) | ((redBlue >> 8) & 0xff00ff);
}

/* sub_41a360 and the sub_41a120 it calls: one triangle of a piece of track for
 * the projected headlight pass.
 *   eax  the slot of Render_GetTm's buffer the polygon goes to; ecx  the
 *   first corner's record (0x1c bytes: +0 and +0xc, +0x10 the texture
 *   coordinates of the light, +4 a clip byte, +8 the vertex, +0x14 colour);
 *   [esp+4], [esp+8]  the other two (ret 8)
 * Nothing when the records share a clip bit or one has bit 0x20, or when the
 * vertices share one; otherwise a polygon of 0xa0 bytes -- kind 1/0x13, the
 * light's texture 0x8b4814, the three vertices copied after it with the
 * records' coordinates and colours, the colours scaled by [0x6fbc38] when
 * that is below 1 (the alpha intensity slider; the game's [0x7a3a58] & 0x40
 * test is forced on by the port's alpha intensity change) -- and the slot moves on past it
 * unless the three colours' alpha is all below 8.  Returns 1 when it moved. */
u32 projectedTriangleOf(App* app, u32 slot, u32 a, u32 b, u32 c)
{
    auto word = [app](nfs::u32 address) { return app->getMemory<nfs::u32>(address); };
    auto byte = [app](nfs::u32 address) { return app->getMemory<nfs::u8>(address); };
    const nfs::u32 records[3] = { a, b, c };
    const nfs::u8 ca = byte(records[0] + 4), cb = byte(records[1] + 4), cc = byte(records[2] + 4);
    if ((ca & cb & cc) || ((ca | cb | cc) & 0x20))
        return 0;

    // sub_41a120, with the records as copied to its stack
    nfs::u32 vertexOf[3], recordWords[3][7];
    for (int i = 0; i < 3; ++i)
    {
        for (int w = 0; w < 7; ++w)
            recordWords[i][w] = word(records[i] + nfs::u32(w) * 4);
        vertexOf[i] = recordWords[i][2];
    }
    const nfs::u32 polygon = word(slot);
    if (byte(vertexOf[1] + 0x14) & byte(vertexOf[0] + 0x14) & byte(vertexOf[2] + 0x14))
        return 0;
    const nfs::u32 copies[3] = { polygon + 0x20, polygon + 0x40, polygon + 0x60 };
    app->getMemory<nfs::u16>(polygon + 4) = 1;
    word(polygon + 0x18) = 0x8b4814;
    app->getMemory<nfs::u16>(polygon + 6) = 0x13;
    word(polygon) = polygon + 0xa0;
    word(polygon + 8) = copies[0];
    word(polygon + 0xc) = copies[1];
    word(polygon + 0x10) = copies[2];
    for (int i = 0; i < 3; ++i)
        std::memmove(&app->getMemory<nfs::u8>(copies[i]), &app->getMemory<nfs::u8>(vertexOf[i]), 0x20);
    // The [0x7a3a58] & 0x40 test is forced on (the port's alpha intensity change).
    const double brightness = gameSettings(app).alphaIntensity;
    if (1.0 > brightness)
    {
        const nfs::u32 scale = nfs::u32(fp::toInteger<nfs::s32>(
            nfs::Float(SingleMath().mul(brightness, app->getMemory<double>(nfs::u32(0x536c6c))))));
        for (int i = 0; i < 3; ++i)
            recordWords[i][5] = scaleColour(recordWords[i][5], scale);
    }
    for (int i = 0; i < 3; ++i)
    {
        word(copies[i] + 0xc) = recordWords[i][0];
        word(copies[i] + 0x18) = recordWords[i][3];
        word(copies[i] + 0x1c) = recordWords[i][4];
        word(copies[i] + 0x10) = recordWords[i][5];
    }
    const nfs::u32 alpha = (word(copies[0] + 0x10) | word(copies[1] + 0x10) | word(copies[2] + 0x10)) >> 24;
    if (alpha < 8)
        return 0;
    word(slot) = polygon + 0xa0;
    return 1;
}

nfs::u32 projectedTriangleNative(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 esp = ts.sp;
    args.eax = projectedTriangleOf(app, args.eax, args.ecx, app->getMemory<u32>(esp + 4), app->getMemory<u32>(esp + 8));
    ts.sp = esp + 4 + 8;
    return args.eax;
}

}  // namespace

/* The functions at the game's addresses: the game's code calls them by name where it called the
 * game's, with no other code behind them. */
nfs::u32 projectedTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41a360
{
    return projectedTriangleNative(app, ts, args);
}

nfs::u32 trackVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41a550
{
    return trackVerticesNative(app, ts, args);
}

nfs::u32 objectVerticesColoured(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41a970
{
    return objectVerticesColouredNative(app, ts, args);
}

nfs::u32 groundDistanceOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41aab0
{
    return groundDistanceNative(app, ts, args);
}

nfs::u32 trackPolygons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41ae90
{
    return trackPolygonsNative(app, ts, args);
}

nfs::u32 trackRecords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41b300
{
    return trackRecordsNative(app, ts, args);
}

nfs::u32 objectRecords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41b640
{
    return objectRecordsNative(app, ts, args);
}

nfs::u32 boxOutside(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41bf30
{
    return boxOutsideNative(app, ts, args);
}

nfs::u32 nearestBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x41e0f0
{
    return nearestBlockNative(app, ts, args);
}

nfs::u32 triangleList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x433a50
{
    return triangleListNative(app, ts, args);
}

nfs::u32 quadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x433bb0
{
    return quadListNative(app, ts, args);
}

nfs::u32 submitPolygons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x434380
{
    return submitPolygonsNative(app, ts, args);
}

nfs::u32 lightGlow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x492370
{
    return lightGlowNative(app, ts, args);
}

nfs::u32 headlightBeam(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x492980
{
    return headlightBeamNative(app, ts, args);
}

nfs::u32 groundHeight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4968a0
{
    return groundHeightNative(app, ts, args);
}

nfs::u32 normalise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4972f0
{
    return normaliseNative(app, ts, args);
}

nfs::u32 objectVerticesNearest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x49d9d0
{
    return objectVerticesNearestNative(app, ts, args);
}

nfs::u32 shadeNormals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x49da30
{
    return shadeNormalsNative(app, ts, args);
}

nfs::u32 envMapCoords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x49db70
{
    return envMapCoordsNative(app, ts, args);
}

nfs::u32 cullPolygons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x49dbf0
{
    return cullPolygonsNative(app, ts, args);
}

nfs::u32 carDetail(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4bb5d0
{
    return carDetailNative(app, ts, args);
}

nfs::u32 objectVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4bf4c0
{
    return objectVerticesNative(app, ts, args);
}

nfs::u32 clipTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c11b0
{
    return clipTriangleNative(app, ts, args);
}

nfs::u32 clipQuad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c12e0
{
    return clipQuadNative(app, ts, args);
}

nfs::u32 clipTriangleByZ(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c1aa0
{
    return clipTriangleByZNative(app, ts, args);
}

nfs::u32 quadMesh(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c1e20
{
    return quadMeshNative(app, ts, args);
}

nfs::u32 clipPolygon(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c2cd0
{
    return clipPolygonNative(app, ts, args);
}

nfs::u32 gatherSorted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c7320
{
    return gatherSortedNative(app, ts, args);
}

nfs::u32 sortedBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c7610
{
    return sortedBucketsNative(app, ts, args);
}

nfs::u32 depthBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c7790
{
    return depthBucketsNative(app, ts, args);
}

nfs::u32 triangleClipped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c)
{
    return clipTriangleOf(app, ts, ts.sp, a, b, c);
}

nfs::u32 quadClipped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c, nfs::u32 d)
{
    return clipQuadOf(app, ts, ts.sp, a, b, c, d);
}

// The one stack word makes the frame one word deeper than the core's esp (the return address's slot) says.
nfs::u32 bucketsSortedInto(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start)
{
    return sortedBucketsOf(app, ts, ts.sp - 4, list, start);
}

void bucketsDepthedInto(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start)
{
    depthBucketsOf(app, ts, ts.sp - 4, list, start);
}

// The seven stack words are written where the code's pushes put them: the core reads them there.
nfs::u32 lightGlowDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 w0, nfs::u32 w1,
                          nfs::u32 w2, nfs::u32 w3, nfs::u32 w4, nfs::u32 w5, nfs::u32 w6)
{
    const nfs::u32 esp = ts.sp - 0x1c;
    const nfs::u32 words[] = {w0, w1, w2, w3, w4, w5, w6};
    for (unsigned k = 0; k < 7; ++k)
        app->getMemory<nfs::u32>(esp + 4 + 4 * k) = words[k];
    return lightGlowOf(app, ts, esp, view);
}

nfs::u32 gatherBuckets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4c79a0
{
    return gatherBucketsNative(app, ts, args);
}

nfs::u32 moveParticles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4cb2c0
{
    return moveParticlesNative(app, ts, args);
}

nfs::u32 drops(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4cb750
{
    return dropsNative(app, ts, args);
}

nfs::u32 particles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4cbc90
{
    return particlesNative(app, ts, args);
}

nfs::u32 objectVerticesList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4dbef0
{
    return objectVerticesListNative(app, ts, args);
}

nfs::u32 addVectors(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4dffb0
{
    return offsetVectorsNative<true>(app, ts, args);
}

nfs::u32 subtractVectors(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e0000
{
    return offsetVectorsNative<false>(app, ts, args);
}

nfs::u32 scaleVectors(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e0050
{
    return scaleVectorsNative(app, ts, args);
}

nfs::u32 unitVector(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e01c0
{
    return unitVectorNative(app, ts, args);
}

nfs::u32 dotPush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e01f0
{
    return dotPushNative(app, ts, args);
}

nfs::u32 dotProducts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e0210
{
    return dotProductsNative(app, ts, args);
}

nfs::u32 multiplyMatrices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e0280
{
    return multiplyMatricesNative(app, ts, args);
}

nfs::u32 rotateVectors(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e03b0
{
    return rotateVectorsNative(app, ts, args);
}

nfs::u32 transformPoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e0430
{
    return transformPointsNative(app, ts, args);
}

nfs::u32 vectorLengthOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)  // 0x4e06a0
{
    return vectorLengthNative(app, ts, args);
}

/* Typed cores of the thunks above for the track's code (guest::direct): the registers and the
 * stack words their thunks read are parameters; esp as the callee finds it is the caller's (its
 * stdcall words' room reserved, the return address's slot), what the cores that lay out a frame
 * on it count from.  The eax each leaves is the result. */
void trackVerticesCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 count, nfs::u32 in,
                       nfs::u32 out, nfs::u32 record)
{
    trackVerticesOf(app, ts, view, count, in, out, record);
}

nfs::u32 objectVerticesListCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 matrix, nfs::u32 move,
                                  nfs::u32 count, nfs::u32 vertices, nfs::u32 colours, nfs::u32 out)
{
    return objectVerticesListOf(app, ts, matrix, move, count, vertices, colours, out);
}

nfs::u32 objectRecordsCore(nfs::App* app, nfs::ThreadState& ts, nfs::s32 count, nfs::u32 polygon,
                             nfs::u32 record, nfs::u32 base)
{
    return objectRecordsOf(app, ts, count, polygon, record, base);
}

nfs::u32 trackRecordsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 records, nfs::u32 pieceRow,
                            nfs::u32 pieceColumn, nfs::u32 tail, nfs::u32 head)
{
    return trackRecordsOf(app, ts, ts.sp, records, pieceRow, pieceColumn, tail, head);
}

nfs::u32 trackPolygonsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 pieceRow,
                             nfs::u32 pieceColumn, nfs::u32 records, nfs::u32 tail, nfs::u32 head)
{
    return trackPolygonsOf(app, ts, ts.sp, view, pieceRow, pieceColumn, records, tail, head);
}

nfs::u32 boxOutsideCore(nfs::App* app, nfs::u32 view, nfs::u32 corners, nfs::u32 matrix,
                          nfs::u32 move)
{
    return boxOutsideOf(app, view, corners, matrix, move);
}

void depthBucketsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start)
{
    depthBucketsOf(app, ts, ts.sp, list, start);
}

void gatherBucketsCore(nfs::App* app, nfs::u32 deep, nfs::u32 out)
{
    gatherBucketsOf(app, deep, out);
}

nfs::u32 clipTriangleCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c)
{
    return clipTriangleOf(app, ts, ts.sp, a, b, c);
}

void clipTriangleByZCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c)
{
    clipTriangleByZOf(app, ts, ts.sp, a, b, c);
}

nfs::u32 clipQuadCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c,
                        nfs::u32 d)
{
    return clipQuadOf(app, ts, ts.sp, a, b, c, d);
}

nfs::u32 submitPolygonsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 first)
{
    return submitPolygonsOf(app, ts, ts.sp, first);
}

}  // namespace vertices

/* sub_4bbd70, where each view pass starts Render_GetTm's buffer over: how much
 * the pass before used, the most since [VIEW] last asked (viewTick in
 * nfs3hp_main.cpp) -- what drawing further would run out of first.  Never
 * takes the call over. */
static nfs::u32 s_arenaPeak = 0;

bool arenaReset(nfs::App* app, nfs::ThreadState& ts)
{
    NFS2_USE(ts);
    const nfs::u32 used = app->getMemory<nfs::u32>(0x55fe38) - app->getMemory<nfs::u32>(0x55fe34);
    if (used > s_arenaPeak && used <= app->getMemory<nfs::u32>(0x7a3d04) + 0x1000)
        s_arenaPeak = used;
    return false;
}

/* sub_40de30 is where an opponent's driver acts on the
 * lane votes sub_409c00 has just added up for it (0x56eea8 on, three
 * directions each, sub_4096b0 among the voters).  NFS_CAR_TRACE writes, for
 * every opponent from 24 to 40 s of the race clock, the car (0x9ac bytes) and
 * the votes (0x56ee80, 0x100 bytes) to ai.bin beside cars.bin.  Each record:
 * "AIVT", the race clock, the car's place in the array at 0x5e1120, 0. */
bool aiVotes(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers)
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_CAR_TRACE");
        return value && *value && *value != '0';
    }();
    if (!on)
        return false;
    const nfs::u32 clock = app->getMemory<nfs::u32>(0x7d3684);
    const nfs::u32 car = registers.eax;
    if (clock < 24 * 64 || clock > 40 * 64 || car < 0x5e1120 || car >= 0x5e1120 + 16 * 0x9ac)
        return false;
    static SDL_IOStream* s_file = nullptr;
    static bool s_failed = false;
    if (!s_file)
    {
        if (s_failed)
            return false;
#ifdef __ANDROID__
        const char* root = SDL_GetAndroidExternalStoragePath();
#else
        const char* root = ".";
#endif
        const std::string path = std::string(root ? root : ".") + "/ai.bin";
        s_file = SDL_IOFromFile(path.c_str(), "wb");
        s_failed = !s_file;
        SDL_Log("[AI] %s %s", s_file ? "writing" : "cannot write", path.c_str());
        if (!s_file)
            return false;
    }
    const nfs::u32 header[4] = { 0x54564941, clock, (car - 0x5e1120) / 0x9ac, 0 };
    SDL_WriteIO(s_file, header, sizeof header);
    SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(car), 0x9ac);
    SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(0x56ee80), 0x100);
    static nfs::u32 s_flushed = 0;
    if (clock / 64 != s_flushed)
    {
        s_flushed = clock / 64;
        SDL_FlushIO(s_file);
    }
    return false;
}

/* NFS_NET_TRACE (extra net_trace) writes what EA's comm
 * library does with a network race into the log ([COMM]), to see where two
 * phones stop on the way into an IPX race.  sub_4f5c10 (the IPX transport
 * opening) turns the library's own event log up ([0x566dec], 0 in the game);
 * sub_512180 is that log: a four-letter tag at eax ("sent", "rsnd", "busy",
 * "hold", "qdat", "recv", "deny"...) with edx, ebx, ecx and two arguments on
 * the stack; sub_51d810 is packet_sendpacket (eax the connection, edx the
 * data, ebx its length); Error (sub_401010, native_game.cpp, which calls
 * commMessageTrace first) is the library's message printer, its format at
 * [esp+4]. */
bool netTraceOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NET_TRACE");
        return value && *value && *value != '0';
    }();
    return on;
}

std::string guestText(nfs::App* app, nfs::u32 address, size_t limit = 80)
{
    std::string text;
    if (address < 0x400000 || address >= 0xa00000 + 0x100000)
        return text;
    for (size_t i = 0; i < limit; ++i)
    {
        const char c = char(app->getMemory<nfs::u8>(address + nfs::u32(i)));
        if (!c)
            break;
        text += (c == '\n' || c == '\r') ? ' ' : c;
    }
    return text;
}

bool commOpenTrace(nfs::App* app, nfs::ThreadState& ts)
{
    NFS2_USE(ts);
    if (netTraceOn())
    {
        app->getMemory<nfs::u32>(0x566dec) = 3;
        nfs::App::traceNewCalls();
        SDL_Log("[COMM] IPX transport opening, the library's event log turned up, new call targets traced");
    }
    return false;
}

bool commEventTrace(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers)
{
    if (!netTraceOn())
        return false;
    const nfs::u32 esp = ts.sp;
    SDL_Log("[COMM] %s edx %08x ebx %08x ecx %08x args %08x %08x", guestText(app, registers.eax, 8).c_str(),
            unsigned(registers.edx), unsigned(registers.ebx), unsigned(registers.ecx), unsigned(app->getMemory<nfs::u32>(esp + 4)),
            unsigned(app->getMemory<nfs::u32>(esp + 8)));
    return false;
}

bool commSendTrace(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers)
{
    if (!netTraceOn())
        return false;
    char bytes[3 * 24 + 1] = "";
    const nfs::u32 length = registers.ebx;
    const int shown = length < 24 ? int(length) : 24;
    for (int i = 0; i < shown; ++i)
        SDL_snprintf(bytes + 3 * i, 4, "%02x ", unsigned(app->getMemory<nfs::u8>(registers.edx + nfs::u32(i))));
    SDL_Log("[COMM] sendpacket conn %08x, %u bytes: %s", unsigned(registers.eax), unsigned(length), bytes);
    return false;
}

void commMessageTrace(nfs::App* app, nfs::ThreadState& ts)
{
    if (!netTraceOn())
        return;
    const nfs::u32 esp = ts.sp;
    SDL_Log("[COMM] message \"%s\" (%s %s line %u) %08x %08x",
            guestText(app, app->getMemory<nfs::u32>(esp + 4), 120).c_str(),
            guestText(app, app->getMemory<nfs::u32>(0x552190)).c_str(),
            guestText(app, app->getMemory<nfs::u32>(0x552194)).c_str(),
            unsigned(app->getMemory<nfs::u32>(0x552198)), unsigned(app->getMemory<nfs::u32>(esp + 8)),
            unsigned(app->getMemory<nfs::u32>(esp + 0xc)));
}

nfs::u32 takeArenaPeak()
{
    const nfs::u32 peak = s_arenaPeak;
    s_arenaPeak = 0;
    return peak;
}

}
