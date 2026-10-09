#include <nfs3hp.h>
#include "native_thrash.h"
#include <lib/gliderenderer.h>
#include <lib/memmap.h>
#include <winapi/glide2x.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
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

/* Native stand-ins for the game's hottest vertex loops, entered from the top of
 * the generated functions (tools/apply_native_vertices.py).  They do the
 * original's arithmetic in the original's order, and step aside -- the
 * generated code runs -- unless the FPU rounds to single precision and to
 * nearest, as it does in a race (fldcw at 0x4a3ed3).
 *
 * The x87 at single precision rounds each result to a float's 24-bit
 * significand but keeps its own wide exponent; the emulated FPU does the same
 * with a double (FPU::add, mul, sub, div).  With float inputs, float arithmetic
 * gives the same bits -- a double holds every product and sum of two floats
 * well enough that rounding it again to a float is the correct rounding -- as
 * long as nothing leaves a float's range.  One track vertex does: a product
 * past the largest float, an infinity the x87 never sees (2026-09-25).  So each
 * call runs as floats, and when the host raises overflow, underflow or invalid
 * on the way, runs again through the emulated FPU's own operations.
 *
 * The polygon and car loops (sub_41ae90, sub_49d...) do a few operations per
 * item among much else, call other functions, and could not simply run twice;
 * they round the emulated FPU's way from the start (NearestMath).
 *
 * Memory, registers, flags and the FPU status word come out as the generated
 * code leaves them -- which skips the flags of an instruction nothing reads --
 * and only the stack below the returned esp is not written. */
namespace nfs3hp
{

// Stand-ins further down that the clipper's callers call straight.
bool thrashDrawTri(win32::WinApplication* app, x86::CPU& cpu);
bool clipPolygon(win32::WinApplication* app, x86::CPU& cpu);
bool clipTriangle(win32::WinApplication* app, x86::CPU& cpu);
bool objectVertices(win32::WinApplication* app, x86::CPU& cpu);
bool projectedTriangle(win32::WinApplication* app, x86::CPU& cpu);
bool clipQuad(win32::WinApplication* app, x86::CPU& cpu);
bool clipTriangleByZ(win32::WinApplication* app, x86::CPU& cpu);
bool sortedBuckets(win32::WinApplication* app, x86::CPU& cpu);
// nfs3hp_main.cpp: races at extended precision, as the Modern Patch (tools/apply_race_precision.py).
bool extendedRaces();
// nfs3hp_main.cpp: a view pass's kind as the main view's where the mirror is drawn in full.
x86::reg32 mirrorAsMain(x86::reg32 kind);
double mirrorCarScale(double scale);

namespace
{

inline float asFloat(x86::reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

inline x86::reg32 asBits(float value)
{
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The arithmetic as floats: exact while nothing over- or underflows. */
struct FloatMath
{
    typedef float Value;
    static Value add(Value a, Value b) { return a + b; }
    static Value sub(Value a, Value b) { return a - b; }
    static Value mul(Value a, Value b) { return a * b; }
    static Value div(Value a, Value b) { return a / b; }
    static Value sqrt(Value a) { return std::sqrt(a); }
};

/* The arithmetic as the generated code does it, through the emulated FPU. */
struct FpuMath
{
    typedef double Value;
    x86::FPU& fpu;
    Value add(Value a, Value b) const { return fpu.add(a, b); }
    Value sub(Value a, Value b) const { return fpu.sub(a, b); }
    Value mul(Value a, Value b) const { return fpu.mul(a, b); }
    Value div(Value a, Value b) const { return fpu.div(a, b); }
};

/* The emulated FPU's own rounding (FPU::toSingle) for the one mode the natives
 * run in, single precision to nearest, without looking the mode up: exact
 * whatever the operands -- a double from memory as well -- and with the x87's
 * range, so nothing has to be run again.  For loops whose few operations are
 * not worth two versions. */
struct NearestMath
{
    typedef double Value;
#ifdef WITH_PEDANTIC_FPU
    static Value add(Value a, Value b) { return a + b; }
    static Value sub(Value a, Value b) { return a - b; }
    static Value mul(Value a, Value b) { return a * b; }
    static Value div(Value a, Value b) { return a / b; }
    static Value sqrt(Value a) { return std::sqrt(a); }
#else
    typedef x86::FPU::Rounded Rounded;
    static Value round(double value, Rounded op, double a, double b)
    {
        std::uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        const std::uint64_t exponent = (bits >> 52) & 0x7ff;
        if ((bits & x86::FPU::kSingleDropped) != x86::FPU::kSingleHalf && exponent - 0x381 < 0x47e - 0x381)
            return double(float(value));
        if ((bits << 1) == 0)
            return value;
        return x86::FPU::roundSingleSlow(value, op, a, b, 0);
    }
    static Value add(Value a, Value b) { return round(a + b, Rounded::Add, a, b); }
    static Value sub(Value a, Value b) { return round(a - b, Rounded::Sub, a, b); }
    static Value mul(Value a, Value b) { return round(a * b, Rounded::Mul, a, b); }
    static Value div(Value a, Value b) { return round(a / b, Rounded::Div, a, b); }
    static Value sqrt(Value a) { return round(std::sqrt(a), Rounded::Sqrt, a, 0.0); }
#endif
};

/* Two products of differences as the x87 works them out at single precision,
 *   one = (p - q) * (r - s),  two = (t - u) * (v - w),  all eight floats,
 * for a comparison of the two -- a polygon's facing.  A difference of floats
 * comes out of float arithmetic as the x87 has it, unless it overflows; a
 * product of two floats is exact in a double; and rounding each to 24 bits
 * cannot swap two that lie further apart than 2^-20 of the larger.  So the
 * exact products are returned, which compare as the rounded ones would, and
 * the emulated FPU's rounding is done only for a near tie or an infinity. */
struct CrossProducts
{
    double two;
    double one;
};

inline CrossProducts crossProducts(float p, float q, float r, float s, float t, float u, float v, float w)
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
    typedef NearestMath M;
    return {M::mul(M::sub(t, u), M::sub(v, w)), M::mul(M::sub(p, q), M::sub(r, s))};
}

/* The last fcomp of a loop over doubles, replayed on the FPU at its end. */
struct LastWideCompare
{
    bool any = false;
    double a = 0.0;
    double b = 0.0;
    void operator()(double x, double y)
    {
        any = true;
        a = x;
        b = y;
    }
    /* The status word it leaves, and sahf's flags from it: done once the loop
     * is over, before the flags of what follows the last sahf. */
    void replay(x86::CPU& cpu) const
    {
        if (!any)
            return;
        cpu.fpu.compare(a, b);
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    }
};

/* A call as the generated code makes one: a slot for the return address, then
 * the callee.  False when the game is shutting down, as the generated code
 * then returns at once. */
bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    return !cpu.terminate;
}

/* fcomp leaves only its condition bits behind, so only the last comparison
 * matters: it is replayed on the FPU once the loop is done. */
struct LastCompare
{
    float a = 0.0f;
    float b = 0.0f;
    void operator()(float x, float y)
    {
        a = x;
        b = y;
    }
};

/* The integer flags as the generated code sets them: test and or clear CF and
 * OF, cmp works them out; neither touches PF. */
struct IntegerFlags
{
    bool cf = false;
    bool of = false;
    bool zf = false;
    bool sf = false;

    void logic(x86::reg32 value)
    {
        cf = of = false;
        zf = !value;
        sf = value >> 31;
    }
    void compare(x86::reg32 a, x86::reg32 b)
    {
        const x86::reg32 result = a - b;
        cf = a < b;
        of = ((a >> 31) != (result >> 31)) && ((a >> 31) != (b >> 31));
        zf = !result;
        sf = result >> 31;
    }
    void store(x86::CPU& cpu) const
    {
        cpu.flags.cf = cf;
        cpu.flags.of = of;
        cpu.flags.zf = zf;
        cpu.flags.sf = sf;
    }
};

/* NFS_NATIVES=0: the generated code throughout, to compare speeds; =vertices:
 * only the three vertex loops native, not the polygon and car loops. */
bool nativesOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

bool polygonNativesOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return !value || (SDL_strcmp(value, "0") != 0 && SDL_strcmp(value, "vertices") != 0);
    }();
    return on;
}

/* NFS_NATIVES=no-drawtri: all but THRASH_drawtri. */
bool drawTriNativeOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return polygonNativesOn() && !(value && SDL_strcmp(value, "no-drawtri") == 0);
    }();
    return on;
}

/* NFS_NATIVES=no-clip: all but the clipper, sub_4c2cd0. */
bool clipNativeOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return polygonNativesOn() && !(value && SDL_strcmp(value, "no-clip") == 0);
    }();
    return on;
}

/* NFS_NATIVES=no-rotate: all but sub_4e03b0, which the game also calls outside
 * the drawing (sub_466330 turns a point of a car into the world). */
bool rotateNativeOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return !(value && SDL_strcmp(value, "no-rotate") == 0);
    }();
    return on;
}

inline bool singlePrecision(const x86::CPU& cpu)
{
#if defined(WITH_PEDANTIC_FPU)
    NFS2_USE(cpu);
    return false;
#elif defined(WITH_WIDE_FPU)
    /* The wide experiment never rounds to single precision, but the loops
     * here only draw: vertices, colours and polygons worked out in float
     * change the picture by a last bit at most, and the game's own logic
     * keeps its wide arithmetic. */
    return cpu.fpu.control.rc == 0 && nativesOn();
#else
    return cpu.fpu.control.pc == x86::FPU::s_singlePrecision && cpu.fpu.control.rc == 0 && nativesOn();
#endif
}

/* The stand-ins that only draw -- vertices, colours, polygons, the clipper,
 * the lights -- stay on in a race at extended precision (extendedRaces, as
 * the Modern Patch runs one): what they work out in float reaches the picture
 * and nothing else, a last bit of a pixel at most, while every stand-in the
 * physics and the opponents' driving can reach (the vector helpers, the
 * nearest block, the ground's height, the particles with their share of the
 * random numbers) steps aside for the generated code and its extended
 * arithmetic. */
inline bool drawingPrecision(const x86::CPU& cpu)
{
#if defined(WITH_PEDANTIC_FPU) || defined(WITH_WIDE_FPU)
    return cpu.fpu.control.rc == 0 && nativesOn();
#else
    return cpu.fpu.control.rc == 0 && nativesOn()
        && (cpu.fpu.control.pc == x86::FPU::s_singlePrecision || (cpu.fpu.control.pc == 3 && extendedRaces()));
#endif
}

/* sub_41a3e0: the colour of a track vertex, darkened by how far off the view's
 * axis it lies and by the fog past [0x552e04].  Written to `out`, returned. */
template <typename Math>
x86::reg32 shadeVertex(win32::WinApplication* app, const Math& math, float x, float y, float z,
                       x86::reg32 colour, x86::reg32 out, LastCompare& compared)
{
    const float inverse = float(math.div(1.0f, z));
    compared(0.0f, x);
    const float absX = 0.0f > x ? -x : x;
    compared(0.0f, y);
    const float absY = 0.0f > y ? -y : y;
    const float offX = float(math.mul(absX, inverse));
    const float offY = float(math.mul(inverse, absY));
    float fog = 1.0f;
    const float fogStart = app->getMemory<float>(x86::reg32(0x552e04));
    compared(z, fogStart);
    if (z > fogStart)
        fog = float(math.sub(1.0f, math.mul(math.sub(z, fogStart), app->getMemory<float>(x86::reg32(0x552e10)))));
    // Each clamped to [0, 1] -- the upper bound by the bits, as a signed integer.
    compared(0.0f, offX);
    const float clampX = 0.0f > offX ? 0.0f : (x86::sreg32(asBits(offX)) > 0x3f800000 ? 1.0f : offX);
    compared(0.0f, offY);
    const float clampY = 0.0f > offY ? 0.0f : (x86::sreg32(asBits(offY)) > 0x3f800000 ? 1.0f : offY);
    const double light = double(math.mul(math.mul(math.sub(1.0f, math.mul(math.add(clampX, clampY), 0.5f)), fog),
                                         65536.0f));
    // fistp as the generated code has it (FPU::rndint, round to nearest).
    x86::reg32 level = x86::reg32(x86::sreg32(std::nearbyint(light)));
    if (x86::sreg32(level) >= 0x10000)
        level = 0xffff;
    level = (level >> 8) & 0xff;
    const x86::reg32 redBlue = (colour & 0xff00ff) * level;
    const x86::reg32 alphaGreen = ((colour & 0xff00ff00) >> 8) * level;
    const x86::reg32 result = (alphaGreen & 0xff00ff00) | ((redBlue >> 8) & 0xff00ff);
    app->getMemory<x86::reg32>(out) = result;
    return result;
}

/* The clip code of a point in view space, as sub_41a550 builds it: 0x10 nearer
 * than [0x552e0c], 0x20 past [0x552e08], 8/4 above/below, 2/1 right/left. */
x86::reg8 clipCode(float x, float y, float z, float nearest, float farthest, LastCompare& compared)
{
    x86::reg8 code = 0;
    compared(z, nearest);
    if (!(z >= nearest))
        code |= 0x10;
    compared(z, farthest);
    if (z > farthest)
        code |= 0x20;
    compared(y, z);
    if (y > z)
        code |= 8;
    else
    {
        compared(-z, y);
        if (-z > y)
            code |= 4;
    }
    compared(x, z);
    if (x > z)
        code |= 2;
    else
    {
        compared(-z, x);
        if (-z > x)
            code |= 1;
    }
    return code;
}

/* sub_41a550: track vertices into view space and onto the screen.
 *   eax  the view: 3x3 matrix at +0x28, position at +0x58, colours at +0x70/+0x74
 *   edx  vertex count; ebx  the vertices, 3 floats each
 *   ecx  the output, 0x1c bytes each; [esp+4]  a 0x20-byte record per vertex
 * A vertex behind the view is mirrored into the second view when +0x74 is set. */
template <typename Math>
void trackVerticesWith(win32::WinApplication* app, x86::CPU& cpu, const Math& math)
{
    const x86::reg32 view = cpu.eax;
    const float* m = &app->getMemory<float>(view + 0x28);
    const float* t = &app->getMemory<float>(view + 0x58);
    const x86::reg32 frontColour = app->getMemory<x86::reg32>(view + 0x70);
    const x86::reg32 backColour = app->getMemory<x86::reg32>(view + 0x74);
    const float nearest = app->getMemory<float>(x86::reg32(0x552e0c));
    const float farthest = app->getMemory<float>(x86::reg32(0x552e08));

    x86::reg32 count = cpu.edx;
    x86::reg32 in = cpu.ebx;
    x86::reg32 out = cpu.ecx;
    x86::reg32 record = app->getMemory<x86::reg32>(cpu.esp + 4);
    LastCompare compared;
    bool any = false;
    bool shaded = false;
    x86::reg32 eax = cpu.eax;
    x86::reg32 ebx = cpu.ebx;
    x86::reg32 zRaw = 0;

    while (--count != x86::reg32(-1))
    {
        any = true;
        app->getMemory<x86::reg16>(out + 6) = 0x10;
        app->getMemory<x86::reg32>(out + 0x18) = 0;
        app->getMemory<x86::reg32>(out + 8) = record;
        app->getMemory<x86::reg16>(out + 4) = 0x10;
        app->getMemory<x86::reg32>(out + 0x14) = 0;

        // Rotated, stored, then moved: each a float in memory in between.
        const float* v = &app->getMemory<float>(in);
        const float rx = float(math.add(math.add(math.mul(v[0], m[0]), math.mul(v[1], m[3])), math.mul(v[2], m[6])));
        const float ry = float(math.add(math.add(math.mul(v[0], m[1]), math.mul(v[1], m[4])), math.mul(v[2], m[7])));
        const float rz = float(math.add(math.add(math.mul(v[0], m[2]), math.mul(v[1], m[5])), math.mul(v[2], m[8])));
        const float x = float(math.add(rx, t[0]));
        const float y = float(math.add(ry, t[1]));
        zRaw = asBits(float(math.add(rz, t[2])));
        x86::reg32 zBits = zRaw;
        if ((zBits & 0x7fffffff) == 0)
            zBits = 0x37800080;
        const float z = asFloat(zBits);
        ebx = zBits;
        const float inverse = x86::sreg32(zBits) < 0x3f800000 ? 1.0f : float(math.div(1.0f, z));
        app->getMemory<float>(out + 0xc) = float(math.mul(math.add(math.mul(x, inverse), 1.0f), 0.5f));
        app->getMemory<float>(out + 0x10) = float(math.mul(math.add(math.mul(y, inverse), 1.0f), 0.5f));
        app->getMemory<float>(out) = float(math.div(app->getMemory<float>(record + 0xc), inverse));

        shaded = false;
        compared(0.0f, z);
        if (!(0.0f >= z))
        {
            app->getMemory<x86::reg16>(out + 4) = clipCode(x, y, z, nearest, farthest, compared);
            compared(z, farthest);
            if (!(z >= farthest))
            {
                eax = shadeVertex(app, math, x, y, z, frontColour, out + 0x14, compared);
                shaded = true;
            }
        }
        else if (backColour != 0)
        {
            const float bx = asFloat(asBits(x) ^ 0x80000000);
            const float by = asFloat(asBits(y) ^ 0x80000000);
            const float bz = asFloat(zBits ^ 0x80000000);
            app->getMemory<x86::reg16>(out + 6) = clipCode(bx, by, bz, nearest, farthest, compared);
            compared(bz, farthest);
            if (!(bz >= farthest))
            {
                eax = shadeVertex(app, math, bx, by, bz, backColour, out + 0x18, compared);
                shaded = true;
            }
        }

        record += 0x20;
        out += 0x1c;
        in += 0xc;
    }

    if (any)
    {
        cpu.fpu.compare(double(compared.a), double(compared.b));
        if (!shaded)
            eax = (zRaw & 0xffff0000) | cpu.fpu.status.word;
        // sahf after the last fnstsw
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    }
    cpu.eax = eax;
    cpu.ebx = ebx;
    cpu.ecx = out;
    cpu.edx = count;
    // cmp edx, -1 before the ret
    cpu.flags.cf = 0;
    cpu.flags.of = 0;
    cpu.set_szp(x86::reg32(0));
    cpu.esp += 4 + 4;
}

/* sub_4bf4c0's clip code of one projected vertex, from the bits of its floats
 * compared as signed integers: 0x10 when 1/z is negative or past [0x7d34f8],
 * else 8/4 for y before [0x7d3504]/past [0x7d3500] and 1/2 for x before
 * [0x7d34fc]/past [0x7d350c].  The registers and flags it leaves are the
 * function's own when this is the last vertex. */
struct ObjectClip
{
    x86::reg32 eax, ebx, ecx, edx;
    IntegerFlags flags;
};

ObjectClip objectClip(win32::WinApplication* app, x86::reg32 out)
{
    ObjectClip clip;
    const x86::reg32 inverse = app->getMemory<x86::reg32>(out + 0xc);
    clip.eax = inverse;
    clip.ecx = app->getMemory<x86::reg32>(x86::reg32(0x7d34f8));
    clip.edx = app->getMemory<x86::reg32>(out + 8);
    clip.ebx = 0x10;
    clip.flags.logic(inverse & 0x80000000);
    if (inverse & 0x80000000)
        return clip;
    clip.flags.compare(inverse, clip.ecx);
    if (x86::sreg32(inverse) >= x86::sreg32(clip.ecx))
        return clip;

    x86::reg32 code = 0;
    // One axis: below `low` (or negative) sets `under`, past `high` sets `over`.
    auto axis = [&](x86::reg32 value, x86::reg32 low, x86::reg32 high, x86::reg32 under, x86::reg32 over) {
        clip.flags.logic(value & 0x80000000);
        if (!(value & 0x80000000))
        {
            clip.flags.compare(value, low);
            if (x86::sreg32(value) >= x86::sreg32(low))
            {
                clip.flags.compare(value, high);
                if (x86::sreg32(value) > x86::sreg32(high))
                    clip.flags.logic(code |= over);
                return;
            }
        }
        clip.flags.logic(code |= under);
    };
    axis(app->getMemory<x86::reg32>(out + 4), app->getMemory<x86::reg32>(x86::reg32(0x7d3504)),
         app->getMemory<x86::reg32>(x86::reg32(0x7d3500)), 8, 4);
    clip.eax = app->getMemory<x86::reg32>(out);
    clip.ecx = app->getMemory<x86::reg32>(x86::reg32(0x7d34fc));
    clip.edx = app->getMemory<x86::reg32>(x86::reg32(0x7d350c));
    axis(clip.eax, clip.ecx, clip.edx, 1, 2);
    clip.ebx = code;
    return clip;
}

/* sub_4bf4c0: object vertices into view space and onto the screen.
 *   eax  vertex count; edx  the vertices, 3 floats each
 *   ebx  3x3 matrix; ecx  position; [esp+4]  the output, 0x20 bytes each:
 *   x, y, z, 1/z, and the clip code at +0x14
 * Pipelined as the original is: a vertex is moved while the one before it is
 * projected. */
template <typename Math>
void objectVerticesWith(win32::WinApplication* app, x86::CPU& cpu, const Math& math)
{
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    if (count <= 0)
    {
        IntegerFlags flags;
        flags.logic(cpu.eax);
        flags.store(cpu);
        cpu.esp += 4 + 4;
        return;
    }
    const float* m = &app->getMemory<float>(cpu.ebx);
    const float* t = &app->getMemory<float>(cpu.ecx);
    const float scaleX = app->getMemory<float>(x86::reg32(0x56009c));
    const float scaleY = app->getMemory<float>(x86::reg32(0x5600a0));
    const float centreX = app->getMemory<float>(x86::reg32(0x5600a4));
    const float centreY = app->getMemory<float>(x86::reg32(0x5600a8));
    x86::reg32 in = cpu.edx;
    x86::reg32 out = app->getMemory<x86::reg32>(cpu.esp + 4);

    // Rotated and moved on the x87 stack, stored once; z of exactly 1 made tiny.
    auto move = [&](x86::reg32 from, x86::reg32 to) {
        const float* v = &app->getMemory<float>(from);
        const auto x = math.add(math.add(math.mul(v[0], m[0]), math.mul(v[1], m[3])), math.mul(v[2], m[6]));
        const auto y = math.add(math.add(math.mul(v[0], m[1]), math.mul(v[1], m[4])), math.mul(v[2], m[7]));
        const auto z = math.add(math.add(math.mul(v[0], m[2]), math.mul(v[1], m[5])), math.mul(v[2], m[8]));
        app->getMemory<float>(to) = float(math.add(x, t[0]));
        app->getMemory<float>(to + 4) = float(math.add(y, t[1]));
        app->getMemory<float>(to + 8) = float(math.add(z, t[2]));
        if (app->getMemory<x86::reg32>(to + 8) == 0x3f800000)
            app->getMemory<x86::reg32>(to + 8) = 0x37800080;
        return float(math.div(1.0f, app->getMemory<float>(to + 8)));
    };
    auto project = [&](x86::reg32 at, float inverse) {
        const auto overX = math.mul(scaleX, inverse);
        const auto overY = math.mul(scaleY, inverse);
        const auto y = math.mul(overY, app->getMemory<float>(at + 4));
        const auto x = math.mul(overX, app->getMemory<float>(at));
        app->getMemory<float>(at + 0xc) = inverse;
        app->getMemory<float>(at) = float(math.add(x, centreX));
        app->getMemory<float>(at + 4) = float(math.add(y, centreY));
    };

    float inverse = move(in, out);
    for (x86::sreg32 left = count - 1; left > 0; --left)
    {
        in += 0xc;
        const float next = move(in, out + 0x20);
        project(out, inverse);
        inverse = next;
        app->getMemory<x86::reg8>(out + 0x14) = x86::reg8(objectClip(app, out).ebx);
        out += 0x20;
    }
    project(out, inverse);
    const ObjectClip last = objectClip(app, out);
    app->getMemory<x86::reg8>(out + 0x14) = x86::reg8(last.ebx);
    cpu.eax = last.eax;
    cpu.ebx = last.ebx;
    cpu.ecx = last.ecx;
    cpu.edx = last.edx;
    last.flags.store(cpu);
    cpu.esp += 4 + 4;
}

/* sub_4e03b0: vectors turned by a 3x3 matrix, the row vector on the left.
 *   eax  count; edx  the vectors in; ebx  the matrix; ecx  the vectors out
 * All three of a vector are read before it is written, so it turns in place as
 * well.  Called from a dozen places; a twentieth of a split-screen race. */
template <typename Math>
void rotateVectorsWith(win32::WinApplication* app, x86::CPU& cpu, const Math& math)
{
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    IntegerFlags flags;
    if (count <= 0)
    {
        flags.logic(cpu.eax);   // test esi, esi
        flags.store(cpu);
        cpu.eax = cpu.edx;
        cpu.esp += 4;
        return;
    }
    const x86::reg32 matrix = cpu.ebx;
    x86::reg32 in = cpu.edx;
    x86::reg32 out = cpu.ecx;
    for (x86::sreg32 left = count; left > 0; --left)
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
        in += 0xc;
        out += 0xc;
    }
    cpu.eax = in;
    cpu.ebx = out - 0xc;
    cpu.ecx = out;
    cpu.edx = matrix;
    // dec esi to 0, and the carry of the last add ecx, 0xc, which dec leaves
    flags.compare(0, 0);
    flags.cf = out < 0xc;
    flags.store(cpu);
    cpu.esp += 4;
}

/* sub_49db70: sphere-map texture coordinates of a car's vertices.
 *   eax  count; edx  the normals; ebx  where they go, turned; ecx  the
 *   vertices, 0x20 bytes each; [esp+4]  the matrix
 * The normals are turned first (sub_4e03b0); then for each, with |z| and
 *   s = 1 / sqrt((1 + |z|) * [0x53c6a8]) * [0x53c6b0], a double,
 *   u = x * s + [0x53c6b8] at +0x18 and v = s * y + [0x53c6b8] at +0x1c. */
template <typename Math>
void envMapLoop(win32::WinApplication* app, x86::reg32 count, x86::reg32 in, x86::reg32 out, x86::reg32& ebx,
                LastWideCompare& compared)
{
    typedef typename Math::Value V;
    const V scale = app->getMemory<float>(x86::reg32(0x53c6a8));
    const double wide = app->getMemory<double>(x86::reg32(0x53c6b0));
    const V centre = app->getMemory<float>(x86::reg32(0x53c6b8));
    for (x86::reg32 left = count; left != 0; --left, in += 0xc, out += 0x20)
    {
        const float z = app->getMemory<float>(in + 8);
        compared(0.0, z);
        V absZ = z;
        if (0.0f > z)
            absZ = -z;
        else
            ebx = app->getMemory<x86::reg32>(in + 8);
        const V root = Math::sqrt(Math::mul(Math::add(V(1), absZ), scale));
        // fmul qword: the one operand that is no float, rounded exactly
        const V s = V(NearestMath::mul(double(Math::div(V(1), root)), wide));
        const V x = app->getMemory<float>(in);
        app->getMemory<float>(out + 0x18) = float(Math::add(Math::mul(x, s), centre));
        const V y = app->getMemory<float>(in + 4);
        app->getMemory<float>(out + 0x1c) = float(Math::add(Math::mul(s, y), centre));
    }
}

void envMapCoordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebp = cpu.ebp;
    const x86::reg32 esi = cpu.esi;
    const x86::reg32 edi = cpu.edi;
    const x86::reg32 count = cpu.eax;
    const x86::reg32 turned = cpu.ebx;
    const x86::reg32 vertices = cpu.ecx;
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0xc -- then the call
    cpu.ebp = esp - 12;
    cpu.esp = esp - 24;
    cpu.ebx = app->getMemory<x86::reg32>(esp + 4);
    cpu.ecx = turned;
    if (!call(app, cpu, 0x4e03b0))
        return;

    // As floats, and once more the emulated FPU's way should one leave their
    // range: the loop only writes what it works out from what it reads.
    x86::reg32 ebx = cpu.ebx;
    LastWideCompare compared;
    std::feclearexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
    envMapLoop<FloatMath>(app, count, turned, vertices, ebx, compared);
    if (std::fetestexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID))
    {
        ebx = cpu.ebx;
        compared = LastWideCompare();
        envMapLoop<NearestMath>(app, count, turned, vertices, ebx, compared);
    }
    const x86::reg32 in = turned + count * 0xc;
    const x86::reg32 out = vertices + count * 0x20;

    compared.replay(cpu);
    if (compared.any)
        cpu.eax = (cpu.eax & 0xffff0000) | cpu.fpu.status.word;  // fnstsw ax
    cpu.ebx = ebx;
    cpu.ecx = out;
    cpu.edx = in;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    // cmp esi, -1 at the end
    IntegerFlags flags;
    flags.compare(0, 0);
    flags.store(cpu);
    cpu.esp = esp + 4 + 4;
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
void shadeNormalsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebp = cpu.ebp;
    const x86::reg32 esi = cpu.esi;
    const x86::reg32 edi = cpu.edi;
    const x86::reg32 count = cpu.edx;
    const x86::reg32 turned = cpu.ecx;
    const x86::reg32 dots = app->getMemory<x86::reg32>(cpu.eax) + app->getMemory<x86::reg32>(cpu.eax + 4) * 0x84;
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x14 -- then the two calls
    cpu.ebp = esp - 12;
    cpu.esp = esp - 12 - 0x14;
    cpu.eax = count;
    cpu.edx = cpu.ebx;
    cpu.ebx = app->getMemory<x86::reg32>(esp + 8);
    cpu.ecx = turned;
    if (!call(app, cpu, 0x4e03b0))
        return;
    cpu.eax = count;
    cpu.ebx = 0x55e6a0;
    cpu.ecx = dots;
    cpu.edx = turned;
    if (!call(app, cpu, 0x4e0210))
        return;

    auto global = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const double factor = app->getMemory<double>(x86::reg32(0x53c6a0));
    x86::reg32 eax = cpu.eax;
    x86::reg32 ebx = cpu.ebx;
    x86::reg32 ecx = cpu.ecx;
    x86::reg32 in = dots;
    x86::reg32 out = app->getMemory<x86::reg32>(esp + 4);
    LastWideCompare compared;
    for (x86::reg32 left = count; left != 0; --left)
    {
        const double d = app->getMemory<float>(in);
        compared(0.0, d);
        if (0.0 < d || d != d)
        {
            const x86::reg32 n = x86::reg32(x86::sreg32(std::nearbyint(NearestMath::mul(d, factor))));
            x86::reg32 red = global(0x55e6b8) + ((n * global(0x55e6d0)) >> 16);
            if (red > global(0x55e6c4))
                red = global(0x55e6c4);
            x86::reg32 green = global(0x55e6bc) + ((global(0x55e6d4) * n) >> 16);
            if (green > global(0x55e6c8))
                green = global(0x55e6c8);
            x86::reg32 blue = ((global(0x55e6d8) * n) >> 16) + global(0x55e6c0);
            if (blue > global(0x55e6cc))
                blue = global(0x55e6cc);
            ecx = (red << 16) | global(0x55e6e4) | (green << 8) | blue;
            app->getMemory<x86::reg32>(out + 0x10) = ecx;
            eax = blue;
            ebx = green << 8;
        }
        else
        {
            eax = (global(0x55e6bc) << 8) | global(0x55e6e4) | (global(0x55e6b8) << 16) | global(0x55e6c0);
            app->getMemory<x86::reg32>(out + 0x10) = eax;
            ecx = global(0x55e6c0);
        }
        out += 0x20;
        in += 4;
    }

    compared.replay(cpu);
    cpu.eax = eax;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = in;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    // cmp esi, -1 at the end
    IntegerFlags flags;
    flags.compare(0, 0);
    flags.store(cpu);
    cpu.esp = esp + 4 + 8;
}

/* sub_49dbf0: those of a part's polygons that face the view and are not off
 * the screen, into the draw list.
 *   eax  the car or object: polygons of 0x84 bytes at [eax], the part's first
 *   at [eax+edx*4+0x5fc], how many at [eax+edx*4+0x6fc];  edx  the part
 *   ebx  nonzero to take the vertices' texture coordinates into the polygon
 * A polygon faces away when its corners' cross product on the screen says so,
 * flipped by [0x554e4c]; bit 0 of +7 makes it two-sided.  The draw list runs
 * from [0x7a3d0c] through each polygon's first dword to [0x7a3d08]. */
void cullPolygonsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto reg32At = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto reg8At = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 object = cpu.eax;
    const x86::reg32 flag = cpu.ebx;
    const x86::reg32 slot = object + cpu.edx * 4;
    const x86::reg32 count = reg32At(slot + 0x6fc);
    IntegerFlags flags;
    if (count == 0)
    {
        cpu.eax = slot;
        cpu.edx = 0;
        flags.logic(0);  // test edx, edx
        flags.store(cpu);
        cpu.esp = esp + 4;
        return;
    }

    LastWideCompare compared;
    x86::reg32 eax = 0;
    // 1 when the cross product of a, b and c says the polygon faces the view.
    auto facing = [&](x86::reg32 polygon) -> x86::reg32 {
        const x86::reg32 a = reg32At(polygon + 8);
        const x86::reg32 b = reg32At(polygon + 0xc);
        const x86::reg32 c = reg32At(polygon + 0x10);
        const CrossProducts cross = crossProducts(floatAt(c + 4), floatAt(b + 4), floatAt(a), floatAt(b),
                                                  floatAt(a + 4), floatAt(b + 4), floatAt(c), floatAt(b));
        compared(cross.two, cross.one);
        eax = cross.two > cross.one ? 1 : 0;
        return reg32At(0x554e4c) ^ eax;
    };

    const x86::reg32 polygons = reg32At(object);
    x86::reg32 edx = reg32At(slot + 0x5fc) * 0x84 + polygons;
    x86::reg32 ebx = edx;
    x86::reg32 current;
    if (reg8At(edx + 7) & 1)
    {
        eax = (polygons & 0xffff00ff) | (x86::reg32(reg8At(edx + 7)) << 8);  // mov ah, [edx+7]
        current = 0;
    }
    else
    {
        current = facing(edx);
    }
    // [ebp-0x14]: what the next polygon's test left, or what the stack held
    x86::reg32 next = reg32At(esp - 16 - 0x14);
    x86::sreg32 i = 0;
    for (; i < x86::sreg32(count); ++i)
    {
        if (i < x86::sreg32(count - 1))
        {
            const bool twoSided = reg8At(ebx + 0x8b) & 1;
            ebx += 0x84;
            next = twoSided ? 0 : facing(ebx);
        }
        if (current == 0
            && !(reg8At(reg32At(edx + 8) + 0x14) & reg8At(reg32At(edx + 0xc) + 0x14) & reg8At(reg32At(edx + 0x10) + 0x14)))
        {
            if (reg32At(0x7a3d0c) == 0)
                reg32At(0x7a3d0c) = edx;
            else
                reg32At(reg32At(0x7a3d08)) = edx;
            reg8At(edx + 7) |= 0x80;
            const x86::reg32 texture = reg8At(edx + 6) & 0x3f;
            reg32At(0x7a3d08) = edx;
            reg32At(edx + 0x18) = reg32At(object + texture * 4 + 0x1e04);
            x86::reg32 colour;
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
                auto copy = [&](x86::reg32 to, x86::reg32 vertex, x86::reg32 at) {
                    app->getMemory<float>(to) = float(double(app->getMemory<float>(reg32At(edx + vertex) + at)));
                };
                copy(edx + 0x54, 8, 0x18);
                copy(edx + 0x58, 8, 0x1c);
                copy(edx + 0x5c, 0xc, 0x18);
                copy(edx + 0x60, 0xc, 0x1c);
                copy(edx + 0x64, 0x10, 0x18);
                copy(edx + 0x68, 0x10, 0x1c);
                reg32At(edx + 0x50) = flag;
                const x86::reg32 shade = reg32At((reg8At(edx + 7) & 8) ? 0x55e6e0 : 0x55e6dc);
                reg32At(edx + 0x74) = shade;
                reg32At(edx + 0x78) = shade;
                reg32At(edx + 0x7c) = shade;
            }
        }
        eax = next;
        current = next;
        edx = ebx;
    }

    compared.replay(cpu);
    cpu.eax = eax;
    cpu.ebx = ebx;
    cpu.edx = edx;
    // cmp ecx, edi at the top of the loop
    flags.compare(x86::reg32(i), count);
    flags.store(cpu);
    cpu.esp = esp + 4;
}

/* voodoo2a's sub_a85770, THRASH_drawtri: a triangle of the game's vertices to
 * Glide.  [esp+4], [esp+8], [esp+0xc]  the three vertices: x, y, z, w at +0,
 * the colour's bytes b, g, r, a at +0x10, u and v at +0x18.  Each becomes a
 * GrVertex on its stack --
 *   x, y;  ooz = z * [0xa9235c];  oow = w;  r, g, b, a from the table at
 *   0xa93210;  s = u * (w * [0xa92358]),  t = (w * [0xa92358]) * v
 * -- and the three go to grDrawTriangle ([0xa93710]).  Here they go to the
 * renderer straight from the host's stack.  Flags and the FPU are as they
 * were: the generated code skips every flag this sets. */
/* A float product that is the x87's at single precision too: anything but a
 * product that left a float's range -- an infinity from finite factors, or a
 * subnormal or zero from nonzero ones -- which the x87 keeps with its wider
 * exponent. */
inline bool floatProductExact(float a, float b, float product)
{
    const float size = std::fabs(product);
    if (size >= FLT_MIN && size <= FLT_MAX)
        return true;
    if (std::isinf(product))
        return std::isinf(a) || std::isinf(b);
    if (product == 0)
        return a == 0 || b == 0;
    return product != product;  // NaN, the emulated FPU's as well
}

void thrashDrawTriNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 corners[3] = { app->getMemory<x86::reg32>(esp + 4), app->getMemory<x86::reg32>(esp + 8),
                                    app->getMemory<x86::reg32>(esp + 0xc) };
    thrashTriangle(app, corners[0], corners[1], corners[2]);
    // lea eax, [esp+8] before the call: the first GrVertex; the last colour in ecx
    cpu.eax = esp - 0xb8;
    cpu.ecx = app->getMemory<x86::reg32>(0xa93210 + x86::reg32(app->getMemory<x86::reg8>(corners[2] + 0x10)) * 4);
    cpu.edx = corners[2];
    cpu.esp = esp + 4 + 0xc;
}

/* A vertex's depth for Glide from its 1/z, as sub_434380 (and the clipper)
 * write it over z: 1 - w, or 0 for a negative w or one past 1, then times
 * [0x560094] and plus [0x560098], a float in memory after each step. */
void polygonDepth(win32::WinApplication* app, x86::reg32 vertex, float scale, float offset)
{
    const x86::reg32 w = app->getMemory<x86::reg32>(vertex + 0xc);
    float t = 0.0f;
    if (!(w & 0x80000000) && x86::sreg32(w) <= 0x3f800000)
        t = 1.0f - app->getMemory<float>(vertex + 0xc);  // exact: w is in [0, 1]
    float scaled = scale * t;
    if (!floatProductExact(scale, t, scaled))
        scaled = float(NearestMath::mul(scale, t));
    app->getMemory<float>(vertex + 8) = offset + scaled;  // a float sum only overflows as fstp would
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
 * THRASH_drawtri ([0x9ef974]) -- here straight to the renderer when that is
 * voodoo2a's.  Returns the polygon the list ended on. */
void submitPolygonsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto reg32At = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto reg8At = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 first = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18
    cpu.ebp = esp - 24;
    cpu.esp = esp - 24 - 0x18;

    auto setState = [&](x86::reg32 state, x86::reg32 value) -> bool {
        reg32At(cpu.esp - 4) = value;
        reg32At(cpu.esp - 8) = state;
        cpu.esp -= 8;
        return call(app, cpu, reg32At(0x9ef96c));
    };
    IntegerFlags flags;
    x86::reg32 polygon = first;
    for (int pass = 0; pass < 2; ++pass)
    {
        const x86::reg8 mark = pass == 0 ? 0x80 : 0x40;
        const x86::reg32 blend = pass == 0 ? 0 : 1;
        const x86::reg32 colours = pass == 0 ? 0x40 : 0x74;
        const x86::reg32 texturing = pass == 0 ? 0x18 : 0x50;
        const x86::reg32 coordinates = pass == 0 ? 0x20 : 0x54;
        polygon = first;
        while (true)
        {
            const x86::reg8 marks = reg8At(polygon + 7);
            if (marks & mark)
            {
                if (pass == 1)
                    reg8At(polygon + 7) = x86::reg8(marks & 0xbf);
                if (reg32At(0x554e58) != blend)
                {
                    reg32At(0x554e58) = blend;
                    if (!setState(0x68, blend))
                        return;
                }
                for (int i = 0; i < 3; ++i)
                    reg32At(reg32At(polygon + 8 + 4 * i) + 0x10) = reg32At(polygon + colours + 4 * i);
                const x86::reg32 texture = reg32At(polygon + texturing);
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
                    if (!setState(1, texture))
                        return;
                }
                const x86::reg32 corners[3] = { reg32At(polygon + 8), reg32At(polygon + 0xc), reg32At(polygon + 0x10) };
                if (reg32At(corners[0] + 0x14) != 0 || reg32At(corners[1] + 0x14) != 0
                    || reg32At(corners[2] + 0x14) != 0)
                {
                    cpu.eax = corners[0];
                    cpu.edx = corners[1];
                    cpu.ebx = corners[2];
                    cpu.esi = polygon;
                    cpu.edi = pass == 0 ? first : texture;
                    cpu.esp -= 4;
                    if (!clipTriangle(app, cpu))
                        app->dynamic_call(0x4c11b0, cpu);
                    if (cpu.terminate)
                        return;
                }
                else
                {
                    const float scale = app->getMemory<float>(x86::reg32(0x560094));
                    const float offset = app->getMemory<float>(x86::reg32(0x560098));
                    for (int i = 0; i < 3; ++i)
                        polygonDepth(app, corners[i], scale, offset);
                    const x86::reg32 draw = reg32At(0x9ef974);
                    if (draw == 0xa85770 && drawTriNativeOn() && thrashPlainTriangles(app))
                    {
                        thrashTriangle(app, corners[0], corners[1], corners[2]);
                    }
                    else
                    {
                        reg32At(cpu.esp - 4) = corners[2];
                        reg32At(cpu.esp - 8) = corners[1];
                        reg32At(cpu.esp - 12) = corners[0];
                        cpu.esp -= 12;
                        if (!call(app, cpu, draw))
                            return;
                    }
                }
            }
            polygon = reg32At(polygon);
            if (polygon == 0)
            {
                flags.logic(0);  // test esi, esi
                break;
            }
            const x86::reg16 kind = app->getMemory<x86::reg16>(polygon + 4);
            if (kind != 3)
            {
                // cmp ax, 3
                const x86::reg16 result = x86::reg16(kind - 3);
                flags.cf = kind < 3;
                flags.of = ((kind ^ 3) & (kind ^ result) & 0x8000) != 0;
                flags.zf = false;
                flags.sf = (result & 0x8000) != 0;
                break;
            }
        }
    }

    cpu.eax = polygon;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    flags.store(cpu);
    cpu.esp = esp + 4;
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
 * Only the arena slots the callees are pointed at live in the guest frame;
 * the rest stays here.  The callees' registers, flags and FPU status are
 * carried through as the original carries them. */
void trackPolygonsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto reg32At = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto reg16At = [app](x86::reg32 address) { return app->getMemory<x86::reg16>(address); };
    auto reg8At = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 esi = cpu.esi;
    const x86::reg32 edi = cpu.edi;
    const x86::reg32 ebp0 = cpu.ebp;
    const x86::reg32 ebp = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    const x86::reg32 arena = ebp - 0x48;
    const x86::reg32 cursor = ebp - 0x40;
    const x86::reg32 view = cpu.eax;
    const x86::reg32 handed = cpu.edx;
    const x86::reg32 piece = cpu.ebx * 0x5c0 + 0x571370;
    const x86::reg32 count = reg32At(piece + cpu.ecx * 4);
    x86::reg32 polygon = reg32At(piece + cpu.ecx * 4 + 0x2c);
    const x86::reg32 records = reg32At(esp + 4);

    cpu.ebp = ebp;
    cpu.esp = ebp - 0x48;
    cpu.edx = 0xa0;
    cpu.eax = arena;
    if (!call(app, cpu, 0x4bbe40))
        return;
    reg32At(cursor) = reg32At(arena);
    cpu.ebx = 0;
    cpu.edx = count * 0xa0;
    cpu.eax = ebp - 0x3c;
    if (!call(app, cpu, 0x4bbe40))
        return;
    // test eax, eax: no room in the arena, nothing drawn
    const x86::sreg32 polygons = cpu.eax != 0 ? x86::sreg32(count) : 0;

    x86::reg32 total = 0;
    x86::reg32 ebx = cpu.ebx;
    x86::reg32 ecx = cpu.ecx;
    LastWideCompare compared;
    // A call sees the FPU status and sahf's flags as the original leaves them.
    auto drawTriangle = [&](x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 ediThen) -> bool {
        compared.replay(cpu);
        compared.any = false;
        reg32At(cpu.esp - 4) = c;
        reg32At(cpu.esp - 8) = b;
        cpu.esp -= 8;
        cpu.eax = cursor;
        cpu.ebx = view;
        cpu.ecx = a;
        cpu.edx = handed;
        cpu.esi = a;
        cpu.edi = ediThen;
        cpu.esp -= 4;
        if (!projectedTriangle(app, cpu))
            app->dynamic_call(0x41a360, cpu);
        if (cpu.terminate)
            return false;
        total += cpu.eax;
        return true;
    };
    auto raise = [&](x86::reg32 record, x86::reg32 at, x86::reg8& vertexByte) {
        // cmp; jbe: the record's byte where it is the greater
        const x86::reg32 vertex = reg32At(record + 8);
        const x86::reg8 own = reg8At(record + at);
        vertexByte = reg8At(vertex + at - 4);
        reg8At(vertex + at - 4) = own > vertexByte ? own : vertexByte;
    };

    for (x86::sreg32 i = 0; i < polygons; ++i, polygon += 0xe)
    {
        const x86::reg32 rec0 = records + x86::reg32(x86::sreg32(x86::sreg16(reg16At(polygon)))) * 0x1c;
        const x86::reg32 rec1 = records + x86::reg32(x86::sreg32(reg32At(polygon)) >> 16) * 0x1c;
        const x86::reg32 rec2 = records + x86::reg32(x86::sreg32(reg32At(polygon + 2)) >> 16) * 0x1c;
        const x86::reg32 rec3 = records + x86::reg32(x86::sreg32(reg32At(polygon + 4)) >> 16) * 0x1c;
        const x86::reg8 flags = reg8At(polygon + 0xc);
        const x86::reg32 t0 = reg32At(rec0 + 8);
        ecx = t0;
        const double farthest = floatAt(reg32At(0x5dd830) + 0x30);
        const double z0 = floatAt(t0 + 8);
        compared(z0, farthest);
        if (z0 > farthest)
            continue;

        const x86::reg32 c0 = reg8At(t0 + 0x14);
        const x86::reg32 t1 = reg32At(rec1 + 8);
        const x86::reg32 c1 = reg8At(t1 + 0x14);
        const x86::reg32 t2 = reg32At(rec2 + 8);
        const x86::reg32 c2 = reg8At(t2 + 0x14);
        const x86::reg32 c3 = reg8At(reg32At(rec3 + 8) + 0x14);
        ebx = c2;
        ecx = c0 & c1 & c2;
        if (c3 & ecx)
            continue;
        if (!(flags & 0x10))
        {
            ecx = c0 | c1 | c2 | c3;
            if (!(ecx & 0x10))
            {
                const CrossProducts p = crossProducts(floatAt(t2 + 4), floatAt(t1 + 4), floatAt(t0), floatAt(t1),
                                                      floatAt(t0 + 4), floatAt(t1 + 4), floatAt(t2), floatAt(t1));
                compared(p.two, p.one);
                ecx = t2;
                const x86::reg32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
                if (back ^ reg32At(0x554e4c))
                {
                    const x86::reg32 t3 = reg32At(rec3 + 8);
                    const x86::reg32 t2again = reg32At(rec2 + 8);
                    ebx = t3;
                    ecx = reg32At(rec0 + 8);
                    const CrossProducts q = crossProducts(floatAt(t3 + 4), floatAt(t2again + 4), floatAt(ecx),
                                                          floatAt(t2again), floatAt(ecx + 4), floatAt(t2again + 4),
                                                          floatAt(t3), floatAt(t2again));
                    compared(q.two, q.one);
                    const x86::reg32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                    if (backToo ^ reg32At(0x554e4c))
                        continue;
                }
            }
        }

        bool draw = false;
        if (mirrorAsMain(reg32At(view)) != 1)
        {
            const x86::reg32 texture = x86::reg32(x86::sreg32(reg32At(polygon + 6)) >> 16);
            ecx = reg32At(0x552df4);
            draw = !(reg8At(ecx + texture * 47 + 0x28) & 4) && reg32At(0x6fbc3c) == 1 && reg32At(0x7a3a60) != 0;
        }
        if (draw)
        {
            if (!drawTriangle(rec0, rec1, rec2, rec2) || !drawTriangle(rec0, rec2, rec3, polygon))
                return;
            ebx = cpu.ebx;
            ecx = cpu.ecx;
            continue;
        }

        // Each corner whose record is on the screen raises its vertex's colour.
        x86::reg8 low, high, third;
        if (reg16At(rec0 + 4) == 0)
        {
            raise(rec0, 0x16, low);
            raise(rec0, 0x15, high);
            raise(rec0, 0x14, high);
            ebx = (ebx & 0xffff0000) | (x86::reg32(high) << 8) | reg8At(rec0 + 0x14);
            ecx = reg32At(rec0 + 8);
        }
        if (reg16At(rec1 + 4) == 0)
        {
            raise(rec1, 0x16, low);
            raise(rec1, 0x15, high);
            raise(rec1, 0x14, high);
            ebx = (ebx & 0xffff0000) | (x86::reg32(high) << 8) | reg8At(rec1 + 0x14);
            ecx = reg32At(rec1 + 8);
        }
        if (reg16At(rec2 + 4) == 0)
        {
            raise(rec2, 0x16, low);
            raise(rec2, 0x15, high);
            raise(rec2, 0x14, third);
            ecx = (ecx & 0xffff0000) | (x86::reg32(high) << 8) | low;
        }
        if (reg16At(rec3 + 4) == 0)
        {
            raise(rec3, 0x16, low);
            raise(rec3, 0x15, high);
            raise(rec3, 0x14, third);
            ebx = (ebx & 0xffff0000) | (x86::reg32(high) << 8) | low;
            ecx = (ecx & 0xffffff00) | third;
        }
    }

    // The arena gets what went out, and the list its new end.
    const x86::reg32 tail = reg32At(esp + 8);
    reg32At(cursor) = reg32At(tail);
    IntegerFlags flags;
    if (total != 0)
    {
        compared.replay(cpu);
        compared.any = false;
        cpu.eax = arena;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = total * 0xa0;
        if (!call(app, cpu, 0x4bbde0))
            return;
        ecx = reg32At(cursor);
        reg32At(ecx != 0 ? ecx : reg32At(esp + 12)) = reg32At(arena);
        cpu.eax = cursor;
        cpu.edx = 0xa0;
        if (!call(app, cpu, 0x4bbe40))
            return;
        // sub dword ptr [ebp-0x40], 0xa0, whose flags nothing reads: the
        // generated code leaves sub_4bbe40's.
        reg32At(cursor) = reg32At(cursor) - 0xa0;
        ebx = cpu.ebx;
    }
    else
    {
        compared.replay(cpu);
        flags.logic(0);  // test edx, edx
        flags.store(cpu);
    }
    reg32At(tail) = reg32At(cursor);

    cpu.eax = total;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = tail;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp0;
    cpu.esp = esp + 4 + 0xc;
}

/* An x87 "indefinite" of either sign: a corner the clipper will not draw. */
inline bool indefinite(x86::reg32 bits)
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
 * goes to THRASH_drawtrifan ([0x9ef960]).  Returns -1 in eax and edx when a
 * pass leaves fewer than three corners, 0 otherwise (also when the first
 * corner is an indefinite and nothing is drawn).
 *
 * The arithmetic is the emulated FPU's rounding at single precision
 * (NearestMath), every float stored where the original stores one; the last
 * fcomp and its sahf are replayed on the FPU, and the integer flags set as
 * the last instruction before the return (or the call) leaves them. */
void clipPolygonNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto load = [app](x86::reg32 address) { return double(app->getMemory<float>(address)); };
    auto store = [app](x86::reg32 address, double value) { app->getMemory<float>(address) = float(value); };

    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 corners = cpu.eax;
    x86::reg8 clipCodes = cpu.bl;

    struct LastFcomp
    {
        bool any = false;
        double a = 0.0;
        double b = 0.0;
        void operator()(double x, double y)
        {
            any = true;
            a = x;
            b = y;
        }
    } compared;
    // fcomp, fnstsw ax, sahf: the FPU's condition bits and the flags from them.
    auto replay = [&]() {
        if (!compared.any)
            return;
        cpu.fpu.compare(compared.a, compared.b);
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    auto finish = [&](x86::reg32 result, x86::reg32 ebx) {
        cpu.eax = result;
        cpu.edx = result;
        cpu.ebx = ebx;
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };

    {
        // Every field read before any is written.
        float fields[3][9];
        for (int i = 0; i < 3; ++i)
        {
            const x86::reg32 from = word(corners + 4 * i);
            const double w = load(from + 0xc);
            const x86::reg32 colour = word(from + 0x10);
            fields[i][0] = app->getMemory<float>(from);
            fields[i][1] = app->getMemory<float>(from + 4);
            fields[i][2] = float(w);
            fields[i][3] = float(M::mul(load(from + 0x18), w));
            fields[i][4] = float(M::mul(load(from + 0x1c), w));
            fields[i][5] = float(colour >> 24);
            fields[i][6] = float(colour >> 16 & 0xff);
            fields[i][7] = float(colour >> 8 & 0xff);
            fields[i][8] = float(colour & 0xff);
        }
        std::memcpy(&app->getMemory<x86::reg8>(0x7d3570), fields, sizeof fields);
    }

    x86::reg32 list = 0x5600d4;    // ebx: the corners' pointers, the work records'
    x86::reg32 codes = cpu.edx;    // their clip codes
    x86::reg32 outList = 0x7d0e78; // esi
    x86::reg32 outCodes = 0x7d0db0;
    x86::reg32 count = 3;
    x86::reg32 made = 0;

    const double nearW = load(0x7d34f0);
    const double top = load(0x7d34dc);
    const double bottom = load(0x7d34e0);
    const double left = load(0x7d34f4);
    const double right = load(0x7d34ec);
    auto codeX = [&](x86::reg32 vertex, x86::reg32 code) {
        const double x = load(vertex);
        compared(x, left);
        if (!(x >= left))
            byte(code) |= 1;
        else
        {
            compared(x, right);
            if (x > right)
                byte(code) |= 2;
        }
    };
    auto between = [&](x86::reg32 a, x86::reg32 b, x86::reg32 made, x86::reg32 field, double t) {
        const double from = load(a + field);
        store(made + field, M::sub(from, M::mul(M::sub(from, load(b + field)), t)));
    };

    /* One pass: the corners outside `bit` dropped, a vertex made where an edge
     * crosses it.  False when fewer than three are left; the lists move on. */
    auto pass = [&](x86::reg8 bit, auto&& makeVertex) -> bool {
        x86::reg32 kept = 0;
        x86::reg32 previous = count - 1;
        for (x86::reg32 current = 0; current < count; previous = current++)
        {
            const x86::reg8 previousCode = byte(codes + previous);
            if (!(previousCode & bit))
            {
                word(outList + 4 * kept) = word(list + 4 * previous);
                byte(outCodes + kept) = previousCode;
                ++kept;
            }
            if ((previousCode ^ byte(codes + current)) & bit)
            {
                const x86::reg32 vertex = 0x7d1198 + made * 0x24;
                makeVertex(word(list + 4 * previous), word(list + 4 * current), vertex, outCodes + kept, previousCode);
                word(outList + 4 * kept) = vertex;
                ++made;
                ++kept;
            }
        }
        if (kept < 3)
        {
            replay();
            IntegerFlags flags;
            flags.compare(kept, 3);
            flags.store(cpu);
            finish(0xffffffff, list);
            return false;
        }
        codes = outCodes;
        outCodes += kept;
        list = outList;
        outList += 4 * kept;
        count = kept;
        return true;
    };
    auto orCodes = [&]() {
        clipCodes = 0;
        for (x86::reg32 i = 0; i < count; ++i)
            clipCodes |= byte(codes + i);
    };

    if (clipCodes & 0x10)
    {
        const bool kept = pass(0x10, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code, x86::reg8) {
            const double aW = load(a + 8);
            const double inverse = double(float(M::div(1.0, M::sub(aW, load(b + 8)))));
            double t = double(float(M::mul(M::sub(aW, nearW), inverse)));
            for (x86::reg32 field : { 0u, 4u, 0xcu, 0x10u })
                between(a, b, vertex, field, t);
            if (x86::sreg32(asBits(float(t))) > 0x3f800000)
                t = double(float(M::mul(aW, inverse)));
            else
            {
                compared(0.0, t);
                if (0.0 > t)
                    t = double(float(M::mul(aW, inverse)));
            }
            for (x86::reg32 field : { 0x14u, 0x18u, 0x1cu, 0x20u })
                between(a, b, vertex, field, t);
            word(vertex + 8) = word(0x7d34f0);
            byte(code) = 0x10;
            const double y = load(vertex + 4);
            compared(y, bottom);
            if (!(y >= bottom))
                byte(code) |= 8;
            else
            {
                compared(y, top);
                if (y > top)
                    byte(code) |= 4;
            }
            codeX(vertex, code);
        });
        if (!kept)
            return;
        orCodes();
    }

    if (clipCodes & 0xc)
    {
        for (const x86::reg8 bit : { x86::reg8(4), x86::reg8(8) })
        {
            if (!(clipCodes & bit))
                continue;
            const double edge = bit == 4 ? top : bottom;
            const x86::reg32 edgeBits = word(bit == 4 ? 0x7d34dc : 0x7d34e0);
            const bool kept = pass(bit, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code,
                                            x86::reg8 previousCode) {
                const double aY = load(a + 4);
                const double t = M::div(M::sub(aY, edge), M::sub(aY, load(b + 4)));
                for (x86::reg32 field : { 0u, 8u, 0xcu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u })
                    between(a, b, vertex, field, t);
                word(vertex + 4) = edgeBits;
                byte(code) = x86::reg8((previousCode & 0xf0) | bit);
                codeX(vertex, code);
            });
            if (!kept)
                return;
        }
        orCodes();
    }

    if (clipCodes & 3)
    {
        for (const x86::reg8 bit : { x86::reg8(1), x86::reg8(2) })
        {
            if (!(clipCodes & bit))
                continue;
            const double edge = bit == 1 ? left : right;
            const x86::reg32 edgeBits = word(bit == 1 ? 0x7d34f4 : 0x7d34ec);
            const bool kept = pass(bit, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code,
                                            x86::reg8 previousCode) {
                const double aX = load(a);
                const double t = M::div(M::sub(aX, edge), M::sub(aX, load(b)));
                for (x86::reg32 field : { 4u, 8u, 0xcu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u })
                    between(a, b, vertex, field, t);
                word(vertex) = edgeBits;
                byte(code) = x86::reg8((previousCode & 0xfc) | bit);
            });
            if (!kept)
                return;
        }
    }

    // A first corner with an indefinite x, y or w: nothing drawn.
    {
        const x86::reg32 first = word(list);
        for (x86::reg32 field : { 0u, 4u, 8u })
        {
            if (indefinite(word(first + field)))
            {
                replay();
                IntegerFlags flags;
                flags.compare(word(first + field) | 0x80000000, 0xffc00000);
                flags.store(cpu);
                finish(0, list);
                return;
            }
        }
    }

    const double scale = load(0x560094);
    const double offset = load(0x560098);
    x86::reg32 fan = 0x7cdbb0;
    x86::reg32 drawn = 0;
    x86::reg32 vertex = 0;
    for (x86::reg32 i = 0; i < count; ++i)
    {
        vertex = word(list + 4 * i);
        const double w = load(vertex + 8);
        const double inverse = double(float(M::div(1.0, w)));
        const x86::reg32 wBits = word(vertex + 8);
        float depth = 0.0f;
        if (!(wBits & 0x80000000) && x86::sreg32(wBits) <= 0x3f800000)
            depth = float(M::sub(1.0, w));
        word(fan) = word(vertex);
        word(fan + 4) = word(vertex + 4);
        const float scaled = float(M::mul(scale, depth));
        store(fan + 8, M::add(offset, scaled));
        store(fan + 0xc, w);
        auto channel = [&](x86::reg32 field) {
            return x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(load(vertex + field)))) & 0xff;
        };
        word(fan + 0x10) = channel(0x14) << 24 | channel(0x18) << 16 | channel(0x1c) << 8 | channel(0x20);
        word(fan + 0x14) = 0;
        store(fan + 0x18, M::mul(load(vertex + 0xc), inverse));
        store(fan + 0x1c, M::mul(load(vertex + 0x10), inverse));
        if (!indefinite(word(fan)) && !indefinite(word(fan + 4)) && !indefinite(word(fan + 8))
            && !indefinite(word(fan + 0xc)))
        {
            fan += 0x20;
            ++drawn;
        }
    }

    /* The call as the original makes it: its frame below the four pushes, the
     * registers it has then; dec edx ended the loop (add ecx, 0x20 or the
     * equal cmp before it cleared CF). */
    replay();
    cpu.flags.cf = false;
    cpu.flags.of = false;
    cpu.flags.zf = true;
    cpu.flags.sf = false;
    const x86::reg32 frame = esp - 16 - 0xc8;
    word(frame - 4) = 0x7cdbb0;
    word(frame - 8) = drawn - 2;
    cpu.eax = drawn - 2;
    cpu.ebx = list + 4 * count;
    cpu.ecx = fan;
    cpu.edx = 0;
    cpu.esi = vertex;
    cpu.edi = drawn;
    cpu.ebp = esp - 16 - 0x82;
    cpu.esp = frame - 8;
    if (!call(app, cpu, word(0x9ef960)))
        return;
    finish(0, cpu.ebx);
}

/* A call through one of THRASH's pointers, the arguments pushed: voodoo2a's
 * THRASH_drawtri straight to its stand-in, without the lookup of a guest call. */
bool callThrash(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot)
{
    const x86::reg32 target = app->getMemory<x86::reg32>(slot);
    cpu.esp -= 4;
    if (target == 0xa85770 && thrashDrawTri(app, cpu))
        return !cpu.terminate;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* call 0x4c2cd0, to the native clipper when it takes the call. */
bool callClipper(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp -= 4;
    if (!clipPolygon(app, cpu))
        app->dynamic_call(0x4c2cd0, cpu);
    return !cpu.terminate;
}

/* The flags of an 8-bit and, or or test. */
void logic8(x86::CPU& cpu, x86::reg8 value)
{
    cpu.flags.cf = false;
    cpu.flags.of = false;
    cpu.flags.zf = !value;
    cpu.flags.sf = value >> 7;
}

/* polygonDepth as sub_4c11b0 and sub_4c12e0 write it out inline, and what it
 * leaves: the flags of its test of w's sign and its cmp with 1.0 -- or of the
 * xor that zeroes a register when w is negative or past 1 (`zeroed`) -- and
 * the 1 - w (or 0) it moved through eax. */
struct InlineDepth
{
    bool zeroed;
    x86::reg32 t;
};

InlineDepth inlineDepth(win32::WinApplication* app, x86::reg32 vertex, float scale, float offset,
                        IntegerFlags& flags)
{
    const x86::reg32 w = app->getMemory<x86::reg32>(vertex + 0xc);
    float t = 0.0f;
    bool zeroed = true;
    if (!(w & 0x80000000))
    {
        flags.compare(w, 0x3f800000);
        zeroed = x86::sreg32(w) > 0x3f800000;
        if (!zeroed)
            t = 1.0f - asFloat(w);
    }
    if (zeroed)
        flags.logic(0);
    float scaled = scale * t;
    if (!floatProductExact(scale, t, scaled))
        scaled = float(NearestMath::mul(scale, t));
    app->getMemory<float>(vertex + 8) = offset + scaled;
    return { zeroed, asBits(t) };
}

/* The first indefinite (either sign) among x, y, depth, w, u, v of the
 * vertices, as or eax, 0x80000000 leaves it; 0 if none. */
x86::reg32 firstIndefinite(win32::WinApplication* app, std::initializer_list<x86::reg32> vertices)
{
    for (x86::reg32 vertex : vertices)
        for (x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
        {
            const x86::reg32 value = app->getMemory<x86::reg32>(vertex + field) | 0x80000000;
            if (value == 0xffc00000)
                return value;
        }
    return 0;
}

/* sub_4c11b0: a triangle (eax, edx, ebx) to the screen.  Nothing when its
 * corners share a clip bit (the byte at +0x14); through the clipper
 * (sub_4c2cd0) when one of them has one; otherwise each gets its depth
 * (polygonDepth) and the three go to THRASH_drawtri ([0x9ef974]).  Registers
 * and flags as the generated code leaves them, the callees' included. */
void clipTriangleNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx;
    const x86::reg8 codes[3] = { byte(a + 0x14), byte(b + 0x14), byte(c + 0x14) };
    const x86::reg32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    auto finish = [&]() {
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };
    for (int i = 0; i < 3; ++i)
        byte(frame - 4 + x86::reg32(i)) = codes[i];

    const x86::reg8 shared = codes[0] & codes[1] & codes[2];
    if (shared)
    {
        cpu.eax = (a & 0xffffff00) | shared;
        logic8(cpu, shared);
        finish();
        return;
    }
    const x86::reg8 any = codes[0] | codes[1] | codes[2];
    cpu.ecx = a;
    cpu.ebp = frame;
    if (any)
    {
        word(frame - 0x1c) = a;
        word(frame - 0x18) = b;
        word(frame - 0x14) = c;
        cpu.eax = frame - 0x1c;
        cpu.edx = frame - 4;
        cpu.ebx = any;
        cpu.esp = frame - 0x1c;
        if (!callClipper(app, cpu))
            return;
        finish();
        return;
    }
    const float scale = app->getMemory<float>(x86::reg32(0x560094));
    const float offset = app->getMemory<float>(x86::reg32(0x560098));
    IntegerFlags flags;
    const InlineDepth da = inlineDepth(app, a, scale, offset, flags);
    const InlineDepth db = inlineDepth(app, b, scale, offset, flags);
    const InlineDepth dc = inlineDepth(app, c, scale, offset, flags);
    cpu.eax = dc.t;
    cpu.edx = b;
    cpu.esi = db.zeroed ? 0 : saved[1];
    cpu.edi = da.zeroed ? 0 : saved[2];
    flags.store(cpu);
    word(frame - 0x20) = c;
    word(frame - 0x24) = b;
    word(frame - 0x28) = a;
    cpu.esp = frame - 0x28;
    if (!callThrash(app, cpu, 0x9ef974))
        return;
    finish();
}

/* sub_4c12e0: a quad (eax, edx, ebx, ecx: a, b, c, d) to the screen.  Nothing
 * when the four share a clip bit; THRASH_drawquad ([0x9ef94c]) with their
 * depths when none has one; otherwise triangles a b c and a c d, each skipped
 * when its corners share a bit, clipped when one has one, else drawn with
 * THRASH_drawtri unless a field of a corner is an indefinite.  The registers
 * it does not keep, and the flags, as the generated code leaves them. */
void clipQuadNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx, d = cpu.ecx;
    const x86::reg8 ca = byte(a + 0x14), cb = byte(b + 0x14), cc = byte(c + 0x14), cd = byte(d + 0x14);
    const x86::reg32 frame = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    auto finish = [&](x86::reg32 eax, x86::reg32 ebx, x86::reg32 ecx, x86::reg32 edx) {
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = esp + 4;
    };
    // And after a callee: its registers as they come back.
    auto finishCalled = [&]() { finish(cpu.eax, cpu.ebx, cpu.ecx, cpu.edx); };
    byte(frame - 4) = ca;
    byte(frame - 3) = cb;
    byte(frame - 2) = cc;
    byte(frame - 1) = cd;
    word(frame - 8) = d;

    const x86::reg8 three = ca & cb & cc;
    const x86::reg8 all = three & cd;
    if (all)
    {
        logic8(cpu, all);
        finish((a & 0xffff0000) | x86::reg32(three) << 8 | all, c, d, b);
        return;
    }
    const x86::reg8 first = ca | cb | cc;
    const x86::reg8 any = first | cd;
    x86::reg32 eax = (a & 0xffff0000) | x86::reg32(three) << 8 | first;
    x86::reg32 ebx = (c & 0xffffff00) | any;
    x86::reg32 ecx = d;
    x86::reg32 edx = b;
    const float scale = app->getMemory<float>(x86::reg32(0x560094));
    const float offset = app->getMemory<float>(x86::reg32(0x560098));
    cpu.esi = c;
    cpu.edi = a;
    cpu.ebp = frame;
    IntegerFlags flags;

    if (!any)
    {
        inlineDepth(app, a, scale, offset, flags);
        const InlineDepth db = inlineDepth(app, b, scale, offset, flags);
        inlineDepth(app, c, scale, offset, flags);
        const InlineDepth dd = inlineDepth(app, d, scale, offset, flags);
        flags.store(cpu);
        cpu.eax = dd.t;
        cpu.ebx = d;
        cpu.ecx = db.zeroed ? 0 : d;
        cpu.edx = b;
        word(frame - 0x40) = d;
        word(frame - 0x44) = c;
        word(frame - 0x48) = b;
        word(frame - 0x4c) = a;
        cpu.esp = frame - 0x4c;
        if (!callThrash(app, cpu, 0x9ef94c))
            return;
        finishCalled();
        return;
    }

    // The first triangle, a b c.
    if (!three)
    {
        if (first)
        {
            word(frame - 0x3c) = a;
            word(frame - 0x38) = b;
            word(frame - 0x34) = c;
            cpu.eax = frame - 0x3c;
            cpu.edx = frame - 4;
            cpu.ebx = first;
            cpu.ecx = ecx;
            cpu.esp = frame - 0x3c;
            if (!callClipper(app, cpu))
                return;
        }
        else
        {
            const InlineDepth da = inlineDepth(app, a, scale, offset, flags);
            const InlineDepth db = inlineDepth(app, b, scale, offset, flags);
            inlineDepth(app, c, scale, offset, flags);
            if (da.zeroed)
                ebx = 0;
            if (db.zeroed)
                ecx = 0;
            const x86::reg32 indefinite = firstIndefinite(app, { a, b, c });
            if (indefinite)
            {
                eax = indefinite;
                cpu.eax = eax;
                cpu.ebx = ebx;
                cpu.ecx = ecx;
                cpu.edx = edx;
            }
            else
            {
                cpu.eax = word(c + 0x1c) | 0x80000000;
                flags.compare(cpu.eax, 0xffc00000);
                flags.store(cpu);
                cpu.ebx = ebx;
                cpu.ecx = ecx;
                cpu.edx = edx;
                word(frame - 0x40) = c;
                word(frame - 0x44) = b;
                word(frame - 0x48) = a;
                cpu.esp = frame - 0x48;
                if (!callThrash(app, cpu, 0x9ef974))
                    return;
            }
        }
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
    }

    // The second, a c d.
    const x86::reg8 shared = ca & cc & cd;
    edx = (edx & 0xffffff00) | cd;
    if (shared)
    {
        logic8(cpu, shared);
        finish((eax & 0xffff0000) | x86::reg32(cc) << 8 | shared, ebx, ecx, edx);
        return;
    }
    const x86::reg8 second = ca | cc | cd;
    if (second)
    {
        byte(frame - 3) = cc;
        byte(frame - 2) = cd;
        word(frame - 0x3c) = a;
        word(frame - 0x38) = c;
        word(frame - 0x34) = d;
        cpu.eax = frame - 0x3c;
        cpu.edx = frame - 4;
        cpu.ebx = second;
        cpu.ecx = ecx;
        cpu.esp = frame - 0x3c;
        if (!callClipper(app, cpu))
            return;
        finishCalled();
        return;
    }
    flags = IntegerFlags();
    const InlineDepth da = inlineDepth(app, a, scale, offset, flags);
    inlineDepth(app, c, scale, offset, flags);
    const InlineDepth dd = inlineDepth(app, d, scale, offset, flags);
    if (da.zeroed || dd.zeroed)
        ecx = 0;
    edx = d;
    const x86::reg32 indefinite = firstIndefinite(app, { a, c, d });
    if (indefinite)
    {
        cpu.flags.cf = false;
        cpu.flags.of = false;
        cpu.flags.zf = true;
        cpu.flags.sf = false;
        finish(indefinite, ebx, ecx, edx);
        return;
    }
    cpu.eax = word(d + 0x1c) | 0x80000000;
    flags.compare(cpu.eax, 0xffc00000);
    flags.store(cpu);
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    word(frame - 0x40) = d;
    word(frame - 0x44) = c;
    word(frame - 0x48) = a;
    cpu.esp = frame - 0x48;
    if (!callThrash(app, cpu, 0x9ef974))
        return;
    finishCalled();
}

/* A corner of sub_4c1aa0's triangle, which carries z at +8 rather than a
 * depth: the depth of 1/z written there as polygonDepth works it out (1/z
 * taken as 65535 for a z of 0), the clip code's dword at +0x14 cleared, and
 * 1/w at +0xc held to 0..1.  Returns 1/w as it came, for the last fcomp. */
float zCornerDepth(win32::WinApplication* app, x86::reg32 vertex, float scale, float offset)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 z = word(vertex + 8);
    const float inverse = (z & 0x7fffffff) ? float(NearestMath::div(1.0, asFloat(z))) : asFloat(0x477fff00);
    word(vertex + 0x14) = 0;
    const x86::reg32 bits = asBits(inverse);
    float t = 0.0f;
    if (!(bits & 0x80000000) && x86::sreg32(bits) <= 0x3f800000)
        t = 1.0f - inverse;
    float scaled = scale * t;
    if (!floatProductExact(scale, t, scaled))
        scaled = float(NearestMath::mul(scale, t));
    app->getMemory<float>(vertex + 8) = offset + scaled;
    const x86::reg32 w = word(vertex + 0xc);
    const float oow = asFloat(w);
    if (0.0f > oow)
        word(vertex + 0xc) = 0;
    else if (x86::sreg32(w) > 0x3f800000)
        word(vertex + 0xc) = 0x3f800000;
    return oow;
}

/* sub_4c1aa0: a triangle (eax, edx, ebx) whose corners carry z, to the
 * screen -- as sub_4c11b0, but each corner drawn gets its depth from 1/z
 * (zCornerDepth), and the clipping goes to sub_4bf790.  Nothing when an
 * x, y, depth, 1/w, u or v of a corner is an indefinite.  Registers, flags
 * and the FPU status as the generated code leaves them, the callees'
 * included. */
void clipTriangleByZNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx;
    const x86::reg8 codes[3] = { byte(a + 0x14), byte(b + 0x14), byte(c + 0x14) };
    const x86::reg32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    auto finish = [&]() {
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };
    for (int i = 0; i < 3; ++i)
        byte(frame - 4 + x86::reg32(i)) = codes[i];

    const x86::reg8 shared = codes[0] & codes[1] & codes[2];
    if (shared)
    {
        cpu.eax = (a & 0xffffff00) | shared;
        logic8(cpu, shared);
        finish();
        return;
    }
    const x86::reg8 any = codes[0] | codes[1] | codes[2];
    cpu.ecx = a;
    cpu.ebp = frame;
    if (any)
    {
        word(frame - 0x4c) = a;
        word(frame - 0x48) = b;
        word(frame - 0x44) = c;
        cpu.eax = frame - 0x4c;
        cpu.edx = frame - 4;
        cpu.ebx = any;
        cpu.esp = frame - 0x4c;
        logic8(cpu, any);
        if (!call(app, cpu, 0x4bf790))
            return;
        finish();
        return;
    }
    const float scale = app->getMemory<float>(x86::reg32(0x560094));
    const float offset = app->getMemory<float>(x86::reg32(0x560098));
    zCornerDepth(app, a, scale, offset);
    zCornerDepth(app, b, scale, offset);
    const float last = zCornerDepth(app, c, scale, offset);
    LastWideCompare compared;
    compared(0.0, last);
    compared.replay(cpu);
    IntegerFlags flags;
    const x86::reg32 indefinite = firstIndefinite(app, { a, b, c });
    if (indefinite)
    {
        flags.compare(indefinite, 0xffc00000);
        flags.store(cpu);
        cpu.eax = indefinite;
        finish();
        return;
    }
    cpu.eax = word(c + 0x1c) | 0x80000000;
    flags.compare(cpu.eax, 0xffc00000);
    flags.store(cpu);
    word(frame - 0x50) = c;
    word(frame - 0x54) = b;
    word(frame - 0x58) = a;
    cpu.esp = frame - 0x58;
    if (!callThrash(app, cpu, 0x9ef974))
        return;
    finish();
}

/* sub_4c1e20: a mesh of quads (a car's) to the screen.
 *   eax  the count; edx  the vertices, 0x20 bytes each; ebx  the quads, four
 *   vertex numbers each
 * Nothing when every corner of every quad shares a clip bit; when none has
 * one, every corner gets its depth (polygonDepth, inline) and the mesh goes
 * whole to THRASH_drawquadmesh ([0x9ef930]); otherwise each quad to
 * sub_4c12e0.  The registers it does not keep, and the flags, as the
 * generated code leaves them. */
void quadMeshNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 16;  // push ecx, esi, edi, ebp; mov ebp, esp
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    const x86::reg32 base = cpu.edx;
    const x86::reg32 list = cpu.ebx;
    auto corner = [&](x86::reg32 quad, x86::reg32 i) { return base + (word(quad + i * 4) << 5); };
    auto finish = [&](x86::reg32 eax, x86::reg32 ebx, x86::reg32 edx) {
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.edx = edx;
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };
    // The codes of the first quad, then of the rest: or and and over all.
    x86::reg8 any = 0;
    x86::reg8 all = 0xff;
    x86::sreg32 n = 0;
    do
    {
        const x86::reg32 quad = list + x86::reg32(n) * 0x10;
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            const x86::reg8 code = byte(corner(quad, i) + 0x14);
            any |= code;
            all &= code;
        }
        ++n;
    } while (n < count);
    if (all)
    {
        logic8(cpu, all);
        finish(list + x86::reg32(n) * 0x10, list, x86::reg32(n));
        return;
    }
    cpu.esi = base;
    cpu.ebp = frame;
    IntegerFlags flags;
    if (any)
    {
        x86::sreg32 i = 0;
        x86::reg32 ebx = 0, edx = x86::reg32(n);
        for (; i < count; ++i)
        {
            const x86::reg32 quad = list + x86::reg32(i) * 0x10;
            flags.compare(x86::reg32(i), x86::reg32(count));
            flags.store(cpu);
            cpu.eax = corner(quad, 0);
            cpu.edx = corner(quad, 1);
            cpu.ebx = corner(quad, 2);
            cpu.ecx = corner(quad, 3);
            cpu.edi = quad;
            cpu.esp = frame - 0x30 - 4;
            if (!clipQuad(app, cpu))
                app->dynamic_call(0x4c12e0, cpu);
            if (cpu.terminate)
                return;
            ebx = cpu.ebx;
            edx = cpu.edx;
        }
        flags.compare(x86::reg32(i), x86::reg32(count));
        flags.store(cpu);
        finish(x86::reg32(i), ebx, edx);
        return;
    }
    const float scale = app->getMemory<float>(x86::reg32(0x560094));
    const float offset = app->getMemory<float>(x86::reg32(0x560098));
    x86::sreg32 i = 0;
    for (; i < count; ++i)
    {
        const x86::reg32 quad = list + x86::reg32(i) * 0x10;
        for (x86::reg32 k = 0; k < 4; ++k)
            inlineDepth(app, corner(quad, k), scale, offset, flags);
    }
    flags.compare(x86::reg32(i), x86::reg32(count));  // cmp edi, edx
    flags.store(cpu);
    cpu.eax = list + x86::reg32(i) * 0x10;
    cpu.edi = x86::reg32(i);
    cpu.edx = x86::reg32(count);
    word(frame - 0x34) = list;
    word(frame - 0x38) = base;
    word(frame - 0x3c) = x86::reg32(count);
    cpu.esp = frame - 0x3c;
    if (!callThrash(app, cpu, 0x9ef930))
        return;
    finish(cpu.eax, cpu.ebx, cpu.edx);
}

/* As floats; again through the emulated FPU when the host says a result left a
 * float's range (or came from one that had).  Every output is a function of
 * inputs the call does not write, so the second run simply writes over the
 * first. */
constexpr int kOutOfRange = FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID;

template <void (*Float)(win32::WinApplication*, x86::CPU&, const FloatMath&),
          void (*Exact)(win32::WinApplication*, x86::CPU&, const FpuMath&)>
void runNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::CPU entry = cpu;
    std::feclearexcept(kOutOfRange);
    Float(app, cpu, FloatMath());
    if (std::fetestexcept(kOutOfRange))
    {
        cpu = entry;
        Exact(app, cpu, FpuMath{cpu.fpu});
    }
}

void trackVerticesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    runNative<trackVerticesWith<FloatMath>, trackVerticesWith<FpuMath>>(app, cpu);
}

void objectVerticesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    runNative<objectVerticesWith<FloatMath>, objectVerticesWith<FpuMath>>(app, cpu);
}

void rotateVectorsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    runNative<rotateVectorsWith<FloatMath>, rotateVectorsWith<FpuMath>>(app, cpu);
}

/* sub_4e0210: dot products, out[i] = v[i] . d.
 *   eax  count; edx  the vectors, 12 bytes each; ebx  d; ecx  out, a float each
 * Called with long lists: a twenty-fifth of a race's game thread (2026-10-03). */
template <typename Math>
void dotProductsWith(win32::WinApplication* app, x86::CPU& cpu, const Math& math)
{
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    IntegerFlags flags;
    if (count <= 0)
    {
        flags.logic(cpu.eax);   // test eax, eax
        flags.store(cpu);
        cpu.esp += 4;
        return;
    }
    const x86::reg32 d = cpu.ebx;
    x86::reg32 in = cpu.edx;
    x86::reg32 out = cpu.ecx;
    for (x86::sreg32 left = count; left > 0; --left)
    {
        const float* v = &app->getMemory<float>(in);
        const float* w = &app->getMemory<float>(d);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        app->getMemory<float>(out) = float(math.add(math.add(math.mul(v0, w[0]), math.mul(v1, w[1])),
                                                    math.mul(v2, w[2])));
        in += 0xc;
        out += 4;
    }
    cpu.eax = in - 0xc;
    cpu.edx = d;
    cpu.ecx = out;
    // dec edi to 0, and the carry of the last add esi, 0xc, which dec leaves
    flags.compare(0, 0);
    flags.cf = in < 0xc;
    flags.store(cpu);
    cpu.esp += 4;
}

void dotProductsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    runNative<dotProductsWith<FloatMath>, dotProductsWith<FpuMath>>(app, cpu);
}

/* sub_4e0280: c = a b, 3x3 matrices of floats, row by row.
 *   eax  a; edx  b; ebx  c
 * c may be a or b: the original copies whichever it is to its frame first, and
 * leaves eax or edx pointing at the copy.  Here everything is read before
 * anything is written; the arithmetic is the emulated FPU's (NearestMath), so
 * nothing runs twice. */
void multiplyMatricesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx;
    const x86::reg32 frame = esp - 16 - 0x2c;  // push ecx, esi, edi, ebp; sub esp, 0x2c
    float left[9], right[9];
    std::memcpy(left, &app->getMemory<float>(a), sizeof left);
    std::memcpy(right, &app->getMemory<float>(b), sizeof right);
    float product[9];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            product[i * 3 + j] = float(M::add(M::add(M::mul(left[i * 3], right[j]), M::mul(left[i * 3 + 1], right[3 + j])),
                                              M::mul(left[i * 3 + 2], right[6 + j])));
    std::memcpy(&app->getMemory<float>(c), product, sizeof product);
    // The copy in the frame, where the original makes one.
    if (a == c || b == c)
        std::memcpy(&app->getMemory<float>(frame), a == c ? left : right, sizeof left);
    cpu.eax = a == c ? frame : a;
    cpu.edx = b == c ? frame : b;
    // add esp, 0x2c
    const x86::reg32 sum = frame + 0x2c;
    cpu.flags.cf = sum < frame;
    cpu.flags.of = ((~(frame ^ 0x2c) & (frame ^ sum)) >> 31) != 0;
    cpu.flags.zf = sum == 0;
    cpu.flags.sf = sum >> 31;
    cpu.esp = esp + 4;
}

/* sub_4e0430: points turned by a 3x3 matrix and moved.
 *   eax  count; edx  the points in; ebx  the matrix; ecx  the move;
 *   [esp+4]  the points out, 12 bytes each (it returns with ret 4)
 * A point's three are read before it is written, so it works in place; the
 * move is added to what was stored, as floats.  NearestMath, as for a
 * multiplied matrix: out may be in. */
void transformPointsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 outArgument = app->getMemory<x86::reg32>(esp + 4);
    IntegerFlags flags;
    if (count <= 0)
    {
        flags.logic(cpu.eax);   // test eax, eax
        flags.store(cpu);
        cpu.ecx = outArgument;
        cpu.esp = esp + 4 + 4;
        return;
    }
    const x86::reg32 matrix = cpu.ebx;
    const x86::reg32 move = cpu.ecx;
    x86::reg32 in = cpu.edx;
    x86::reg32 out = outArgument;
    for (x86::sreg32 left = count; left > 0; --left)
    {
        const float* v = &app->getMemory<float>(in);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        const float* m = &app->getMemory<float>(matrix);
        const float x = float(M::add(M::add(M::mul(v0, m[0]), M::mul(v1, m[3])), M::mul(v2, m[6])));
        const float y = float(M::add(M::add(M::mul(v0, m[1]), M::mul(v1, m[4])), M::mul(v2, m[7])));
        const float z = float(M::add(M::add(M::mul(v0, m[2]), M::mul(v1, m[5])), M::mul(v2, m[8])));
        float* o = &app->getMemory<float>(out);
        o[0] = x;
        o[1] = y;
        o[2] = z;
        const float* t = &app->getMemory<float>(move);
        const float t0 = t[0], t1 = t[1], t2 = t[2];
        const float x2 = float(M::add(o[0], t0));
        const float y2 = float(M::add(o[1], t1));
        const float z2 = float(M::add(o[2], t2));
        o[0] = x2;
        o[1] = y2;
        o[2] = z2;
        in += 0xc;
        out += 0xc;
    }
    cpu.eax = out;
    cpu.ebx = out - 0xc;
    cpu.ecx = out;
    cpu.edx = move;
    // dec esi to 0; CF still test eax, eax's
    flags.compare(0, 0);
    flags.store(cpu);
    cpu.esp = esp + 4 + 4;
}

/* sub_4c79a0: the frame's polygons as one list, from the depth buckets.
 *   eax  whether the 2000 far buckets count too; esi  where the list's head
 *   and tail go; returns it in eax
 * Each bucket is a head and a tail (0x7d575c, 8 bytes each): the 256 near ones
 * from the second on, then, asked, the 2000 below 0x7d9dd4 from the top down,
 * each non-empty one hung on the tail so far (its first dword the next).
 * With [0x7a3a60] clear the list comes from sub_4bbe70 instead. */
void gatherBucketsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.edi, cpu.ebp };
    x86::reg32 out = cpu.esi;
    const x86::reg32 deep = cpu.eax;
    const x86::reg32 frame = esp - 20;  // push ebx, ecx, edx, edi, ebp; mov ebp, esp
    x86::reg32 head, tail;
    if (word(0x7a3a60) == 0)
    {
        cpu.ecx = out;
        cpu.ebx = deep;
        cpu.esi = frame - 0x10;
        cpu.ebp = frame;
        cpu.esp = frame - 0x10;
        if (!call(app, cpu, 0x4bbe70))
            return;
        out = cpu.ecx;  // mov edi, ecx: where the callee left it
        head = word(frame - 0x10);
        tail = word(frame - 0xc);
    }
    else
    {
        head = word(0x7d575c);
        tail = word(0x7d5760);
        auto take = [&](x86::reg32 bucket) {
            if (head == 0)
            {
                head = word(bucket);
                tail = word(bucket + 4);
            }
            else if (const x86::reg32 first = word(bucket))
            {
                word(tail) = first;
                tail = word(bucket + 4);
            }
        };
        for (x86::reg32 i = 1; i < 0x100; ++i)
            take(0x7d575c + i * 8);
        IntegerFlags flags;
        if (deep)
        {
            for (x86::reg32 i = 0; i < 0x7d0; ++i)
                take(0x7d9dd4 - i * 8);
            flags.compare(0x7d0, 0x7d0);
        }
        else
        {
            flags.logic(0);  // test ebx, ebx
        }
        flags.store(cpu);
        word(frame - 8) = head;
        word(frame - 4) = tail;
    }
    word(frame - 0x10) = head;
    word(frame - 0xc) = tail;
    word(out) = head;
    word(out + 4) = tail;
    cpu.eax = out;
    cpu.esi = frame - 8;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = esp + 4;
}

/* sub_49d9d0: an object's vertices to view space and the screen (sub_4bf4c0,
 * the stand-in objectVertices), each given the colour [esp+8], and the
 * nearest z of them -- from 65535 down -- returned on the x87 stack.
 *   eax  count; ebx  the vertices out (sub_4bf4c0's argument); ecx  its ebx;
 *   edx  its edx; [esp+4]  its ecx; [esp+8]  the colour (ret 8)
 * The last fcomp and its sahf are replayed, and ax holds the status word as
 * fnstsw left it. */
void objectVerticesNearestNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    const x86::reg32 vertices = cpu.ebx;
    const x86::reg32 colour = app->getMemory<x86::reg32>(esp + 8);
    const x86::reg32 frame = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    app->getMemory<x86::reg32>(frame - 4) = 0x477fff00;  // 65535.0f
    app->getMemory<x86::reg32>(frame - 8) = vertices;    // push edi: sub_4bf4c0's argument
    cpu.ebx = cpu.ecx;
    cpu.ecx = app->getMemory<x86::reg32>(esp + 4);
    cpu.esi = x86::reg32(count);
    cpu.edi = vertices;
    cpu.ebp = frame;
    cpu.esp = frame - 8;
    cpu.esp -= 4;
    if (!objectVertices(app, cpu))
        app->dynamic_call(0x4bf4c0, cpu);
    if (cpu.terminate)
        return;

    x86::reg32 nearest = 0x477fff00;
    x86::reg32 edx = 0;
    x86::reg32 ecx = cpu.ecx;
    x86::reg32 ebx = cpu.ebx;
    bool comparedAny = false;
    double lastA = 0.0, lastB = 0.0;
    for (; x86::sreg32(edx) < count; ++edx)
    {
        ebx = vertices + edx * 0x20;
        ecx = colour;
        app->getMemory<x86::reg32>(ebx + 0x10) = ecx;
        const double z = app->getMemory<float>(ebx + 8);
        const double low = asFloat(nearest);
        comparedAny = true;
        lastA = z;
        lastB = low;
        if (!(z >= low))
        {
            ecx = app->getMemory<x86::reg32>(ebx + 8);
            nearest = ecx;
        }
    }
    app->getMemory<x86::reg32>(frame - 4) = nearest;
    if (comparedAny)
    {
        cpu.fpu.compare(lastA, lastB);
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        cpu.ax = x86::reg16(cpu.fpu.status.word);
        cpu.ebx = ebx;
        cpu.ecx = ecx;
    }
    IntegerFlags flags;
    flags.compare(edx, x86::reg32(count));  // cmp edx, esi
    flags.store(cpu);
    cpu.edx = edx;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(asFloat(nearest));
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = esp + 4 + 8;
}

/* idiv as the generated code does it: edx:eax over a signed dword, on arm64,
 * where a zero divisor gives a quotient of 0 and leaves the dividend over. */
void signedDivide(x86::sreg64 dividend, x86::sreg32 divisor, x86::reg32& quotient, x86::reg32& remainder)
{
    if (divisor == 0)
    {
        quotient = 0;
        remainder = x86::reg32(dividend);
        return;
    }
    quotient = x86::reg32(x86::reg64(dividend / divisor));
    remainder = x86::reg32(dividend % divisor);
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
 * The registers and flags it does not keep are left as the original leaves
 * them; the last fcomp and sahf are replayed before each call and the end. */
void trackRecordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto half = [app](x86::reg32 address) { return app->getMemory<x86::reg16>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 12;  // push esi, edi, ebp; mov ebp, esp
    const x86::reg32 records = cpu.ecx;
    const x86::reg32 piece = cpu.edx * 0x5c0 + 0x571370;
    const x86::reg32 count = word(piece + cpu.ebx * 4);
    x86::reg32 polygon = word(piece + cpu.ebx * 4 + 0x2c);
    const x86::reg32 tailArgument = word(esp + 4);
    const x86::reg32 headArgument = word(esp + 8);
    const x86::reg32 first = frame - 0x3c;
    const x86::reg32 cursorSlot = frame - 0x10;

    cpu.ebp = frame;
    cpu.esp = frame - 0x3c;
    cpu.ecx = polygon;
    cpu.edx = 0x20;
    cpu.eax = first;
    cpu.ebx = 0;
    if (!call(app, cpu, 0x4bbe40))
        return;
    x86::reg32 cursor = word(first);
    x86::reg32 ebx = cpu.ebx;
    polygon = cpu.ecx;  // the loop walks ecx as the call left it
    x86::reg32 written = 0;
    LastWideCompare compared;
    const double farthest = floatAt(word(0x5dd830) + 0x30);
    const x86::reg32 textures = word(0x552df4);

    for (x86::sreg32 i = x86::sreg32(ebx); i < x86::sreg32(count); ++i, polygon += 0xe)
    {
        const x86::reg32 v0 = records + (x86::reg32(x86::sreg32(x86::sreg16(half(polygon)))) << 5);
        const x86::reg32 v1 = records + (x86::reg32(x86::sreg32(word(polygon)) >> 16) << 5);
        const x86::reg32 v2 = records + (x86::reg32(x86::sreg32(word(polygon + 2)) >> 16) << 5);
        const x86::reg32 v3 = records + (x86::reg32(x86::sreg32(word(polygon + 4)) >> 16) << 5);
        word(cursor + 8) = v0;
        word(cursor + 0xc) = v1;
        word(cursor + 0x10) = v2;
        word(cursor + 0x14) = v3;
        ebx = v2;
        const x86::reg8 flags = byte(polygon + 0xc);
        const double z0 = floatAt(v0 + 8);
        compared(z0, farthest);
        if (z0 > farthest)
            continue;
        const x86::reg32 c0 = byte(v0 + 0x14), c1 = byte(v1 + 0x14), c2 = byte(v2 + 0x14), c3 = byte(v3 + 0x14);
        if (c3 & c0 & c1 & c2)
            continue;
        if (!(flags & 0x10) && !((c0 | c1 | c2 | c3) & 0x10))
        {
            const CrossProducts p = crossProducts(floatAt(v2 + 4), floatAt(v1 + 4), floatAt(v0), floatAt(v1),
                                                  floatAt(v0 + 4), floatAt(v1 + 4), floatAt(v2), floatAt(v1));
            compared(p.two, p.one);
            const x86::reg32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
            if (back ^ word(0x554e4c))
            {
                const CrossProducts q = crossProducts(floatAt(v3 + 4), floatAt(v2 + 4), floatAt(v0), floatAt(v2),
                                                      floatAt(v0 + 4), floatAt(v2 + 4), floatAt(v3), floatAt(v2));
                compared(q.two, q.one);
                const x86::reg32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                if (backToo ^ word(0x554e4c))
                    continue;
            }
        }

        word(cursor) = cursor + 0x20;
        word(cursor + 0x18) = textures + x86::reg32(x86::sreg32(word(polygon + 6)) >> 16) * 47;
        app->getMemory<x86::reg16>(cursor + 6) = 0;
        app->getMemory<x86::reg16>(cursor + 4) = 0;
        if (flags & 1)
            byte(cursor + 6) = x86::reg8(byte(cursor + 6) | 4);
        auto depthSum = [&]() {
            return M::mul(M::add(M::add(M::add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8)), floatAt(v3 + 8)),
                          app->getMemory<double>(x86::reg32(0x536c8c)));
        };
        if ((byte(0x7a3a58) & 6) && word(0x6fbc48) != 0)
        {
            const double mean = depthSum();
            const double near = app->getMemory<double>(x86::reg32(0x536c94));
            compared(mean, near);
            if (!(mean >= near))
                byte(cursor + 6) = x86::reg8(byte(cursor + 6) | 8);
        }
        if (flags & 4)
        {
            const x86::reg8 animation = byte(polygon + 0xd);
            x86::reg32 frames, step, unused, phase;
            const x86::sreg32 clock = x86::sreg32(word(0x7d3684));
            signedDivide(clock, x86::sreg32(animation >> 3), frames, unused);
            signedDivide(x86::sreg32(frames), x86::sreg32(animation & 7), step, phase);
            NFS2_USE(step);
            const x86::reg32 texture = phase + x86::reg32(x86::sreg32(word(polygon + 8)) >> 16);
            word(cursor + 0x18) = textures + texture * 47;
        }
        if (byte(word(cursor + 0x18) + 0x28) & 2)
        {
            const x86::reg32 colour = word(0x552e1c);
            word(v0 + 0x10) = colour;
            word(v1 + 0x10) = colour;
            word(v2 + 0x10) = colour;
            word(v3 + 0x10) = colour;
            byte(cursor + 6) = x86::reg8(byte(cursor + 6) | 1);
            app->getMemory<float>(cursor + 0x1c) = float(depthSum());
        }
        cursor += 0x20;
        ++written;
        ebx = written;
    }

    word(cursorSlot) = cursor;
    x86::reg32 ecx = polygon;
    cursor = word(tailArgument);
    word(cursorSlot) = cursor;
    compared.replay(cpu);
    IntegerFlags flags;
    flags.logic(written);  // test eax, eax
    flags.store(cpu);
    if (written != 0)
    {
        cpu.eax = first;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = written << 5;
        if (!call(app, cpu, 0x4bbde0))
            return;
        const x86::reg32 tail = word(cursorSlot);
        word(tail != 0 ? tail : headArgument) = word(first);
        flags.logic(tail);  // test edx, edx
        flags.store(cpu);
        cpu.edx = 0x20;
        cpu.eax = cursorSlot;
        if (!call(app, cpu, 0x4bbe40))
            return;
        word(cursorSlot) = word(cursorSlot) - 0x20;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
    }
    word(tailArgument) = word(cursorSlot);
    cpu.eax = written;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = tailArgument;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = esp + 4 + 8;
}

bool setThrashState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value)
{
    app->getMemory<x86::reg32>(cpu.esp - 4) = value;
    app->getMemory<x86::reg32>(cpu.esp - 8) = state;
    cpu.esp -= 8;
    return call(app, cpu, app->getMemory<x86::reg32>(0x9ef96c));
}

/* sub_433bb0: a list of quads to the drawing, from the one in eax while each
 * next one's kind word at +4 is 0.  For each, by its flags at +6: blending
 * (bit 1, state 0x68 through [0x554e58]), its texture record at +0x18 -- the
 * light coordinates copied to the four corners at +0x18 unless bit 2, then
 * the texture through [0x554e54] (state 1) -- state 0x15 from [0x7a3a64]
 * unless bit 4 (through [0x554e5c]), and the four corners (+8 to +0x14) to
 * sub_4c20e0 with bit 0x10, else to the quad's clipper sub_4c12e0.  Returns
 * the quad the list stopped at.  The registers but eax are kept. */
void quadListNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 24;
    x86::reg32 quad = cpu.eax;
    cpu.ebp = frame;
    cpu.esp = frame - 0xc;
    IntegerFlags flags;
    while (true)
    {
        const x86::reg32 a = word(quad + 8), b = word(quad + 0xc), c = word(quad + 0x10), d = word(quad + 0x14);
        const x86::reg8 bits = app->getMemory<x86::reg8>(quad + 6);
        cpu.esi = quad;
        cpu.edi = b;
        cpu.ebx = d;
        const x86::reg32 blend = bits & 1;
        if (word(0x554e58) != blend)
        {
            word(0x554e58) = blend;
            if (!setThrashState(app, cpu, 0x68, blend))
                return;
        }
        if (word(quad + 0x18) != 0)
        {
            if (!(bits & 2))
            {
                word(a + 0x18) = word(word(quad + 0x18) + 8);
                word(a + 0x1c) = word(word(quad + 0x18) + 0xc);
                word(b + 0x18) = word(word(quad + 0x18) + 0x10);
                word(b + 0x1c) = word(word(quad + 0x18) + 0x14);
                word(c + 0x18) = word(word(quad + 0x18) + 0x18);
                word(c + 0x1c) = word(word(quad + 0x18) + 0x1c);
                word(d + 0x18) = word(word(quad + 0x18) + 0x20);
                word(d + 0x1c) = word(word(quad + 0x18) + 0x24);
            }
            const x86::reg32 texture = word(word(quad + 0x18) + 4);
            if (word(0x554e54) != texture)
            {
                word(0x554e54) = texture;
                if (!setThrashState(app, cpu, 1, texture))
                    return;
            }
        }
        else if (word(0x554e54) != 0)
        {
            word(0x554e54) = 0;
            if (!setThrashState(app, cpu, 1, 0))
                return;
        }
        const x86::reg32 mode = (bits & 4) ? x86::reg32(0) : x86::reg32(word(0x7a3a64));
        if (word(0x554e5c) != mode)
        {
            word(0x554e5c) = mode;
            if (!setThrashState(app, cpu, 0x15, mode))
                return;
        }
        cpu.eax = a;
        cpu.edx = b;
        cpu.ebx = c;
        cpu.ecx = d;
        cpu.esp -= 4;
        if (bits & 0x10)
            app->dynamic_call(0x4c20e0, cpu);
        else if (!clipQuad(app, cpu))
            app->dynamic_call(0x4c12e0, cpu);
        if (cpu.terminate)
            return;
        quad = word(quad);
        if (quad == 0)
        {
            flags.logic(0);  // test esi, esi
            break;
        }
        const x86::reg16 kind = app->getMemory<x86::reg16>(quad + 4);
        if (kind != 0)
        {
            // test ax, ax
            flags.cf = flags.of = false;
            flags.zf = false;
            flags.sf = (kind & 0x8000) != 0;
            break;
        }
    }
    flags.store(cpu);
    cpu.eax = quad;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = esp + 4;
}

/* sub_433a50: a list of triangles to the drawing, from the one in eax while
 * each next one's kind word at +4 is 1 -- sub_433bb0 for triangles: blending
 * (bit 1 of the flags at +6), the texture record at +0x18 (its light
 * coordinates to the three corners at +0x18 unless bit 2) and state 0x15
 * unless bit 4, each through the state it last set; then the corners (+8 to
 * +0x10) to sub_4c1aa0 with bit 0x10, else to sub_4c11b0.  Returns the
 * triangle the list stopped at.  The registers but eax are kept. */
void triangleListNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 24;
    x86::reg32 triangle = cpu.eax;
    cpu.ebp = frame;
    cpu.esp = frame - 8;
    IntegerFlags flags;
    while (true)
    {
        const x86::reg32 a = word(triangle + 8), b = word(triangle + 0xc), c = word(triangle + 0x10);
        const x86::reg8 bits = app->getMemory<x86::reg8>(triangle + 6);
        cpu.esi = triangle;
        cpu.edi = b;
        cpu.ebx = c;
        const x86::reg32 blend = bits & 1;
        if (word(0x554e58) != blend)
        {
            word(0x554e58) = blend;
            if (!setThrashState(app, cpu, 0x68, blend))
                return;
        }
        const x86::reg32 record = word(triangle + 0x18);
        if (record != 0)
        {
            if (!(bits & 2))
            {
                word(a + 0x18) = word(record + 8);
                word(a + 0x1c) = word(record + 0xc);
                word(b + 0x18) = word(record + 0x10);
                word(b + 0x1c) = word(record + 0x14);
                word(c + 0x18) = word(record + 0x18);
                word(c + 0x1c) = word(record + 0x1c);
            }
            const x86::reg32 texture = word(record + 4);
            if (word(0x554e54) != texture)
            {
                word(0x554e54) = texture;
                if (!setThrashState(app, cpu, 1, texture))
                    return;
            }
        }
        else if (word(0x554e54) != 0)
        {
            word(0x554e54) = 0;
            if (!setThrashState(app, cpu, 1, 0))
                return;
        }
        const x86::reg32 mode = (bits & 4) ? x86::reg32(0) : x86::reg32(word(0x7a3a64));
        if (word(0x554e5c) != mode)
        {
            word(0x554e5c) = mode;
            if (!setThrashState(app, cpu, 0x15, mode))
                return;
        }
        cpu.eax = a;
        cpu.edx = b;
        cpu.ebx = c;
        cpu.esp -= 4;
        if (bits & 0x10)
        {
            if (!clipTriangleByZ(app, cpu))
                app->dynamic_call(0x4c1aa0, cpu);
        }
        else if (!clipTriangle(app, cpu))
            app->dynamic_call(0x4c11b0, cpu);
        if (cpu.terminate)
            return;
        triangle = word(triangle);
        if (triangle == 0)
        {
            flags.logic(0);  // test esi, esi
            break;
        }
        const x86::reg16 kind = app->getMemory<x86::reg16>(triangle + 4);
        if (kind != 1)
        {
            // cmp ax, 1
            const x86::reg16 result = x86::reg16(kind - 1);
            flags.cf = kind < 1;
            flags.of = (((kind ^ 1) & (kind ^ result)) >> 15) != 0;
            flags.zf = false;
            flags.sf = (result >> 15) != 0;
            break;
        }
    }
    flags.store(cpu);
    cpu.eax = triangle;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = esp + 4;
}

/* A bucket's head and tail (two dwords): the polygon hung on its end. */
inline void appendToBucket(win32::WinApplication* app, x86::reg32 bucket, x86::reg32 polygon)
{
    if (app->getMemory<x86::reg32>(bucket) != 0)
        app->getMemory<x86::reg32>(app->getMemory<x86::reg32>(bucket + 4)) = polygon;
    else
        app->getMemory<x86::reg32>(bucket) = polygon;
    app->getMemory<x86::reg32>(bucket + 4) = polygon;
}

/* sub_4c7610: polygons whose texture has bit 2 at +0x28, each put into one of
 * 800 buckets at 0x7db6dc by its depth at +0x1c times [0x54024c], kept in
 * order of that depth, deepest first.
 *   eax  the list; [esp+4]  passed on to sub_4c7480 when [0x7a3a60] is clear
 * The last fcomp and sahf are replayed; eax as fnstsw and the rest leave it. */
void sortedBucketsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    if (word(0x7a3a60) == 0)
    {
        cpu.ebp = esp - 24;
        word(esp - 28) = word(esp + 4);
        cpu.edx = cpu.eax;
        cpu.esp = esp - 28;
        if (!call(app, cpu, 0x4c7480))
            return;
    }
    else
    {
        x86::reg32 polygon = cpu.eax;
        x86::reg32 eax = cpu.eax;
        LastWideCompare compared;
        const double scale = app->getMemory<float>(x86::reg32(0x54024c));
        while (polygon != 0)
        {
            const x86::reg32 next = word(polygon);
            x86::sreg32 index = cpu.fpu.toInteger<x86::sreg32>(
                x86::Float(NearestMath::mul(app->getMemory<float>(polygon + 0x1c), scale)));
            if (index < 0)
                index = 0;
            else if (index > 0x31f)
                index = 0x31f;
            const x86::reg32 bucket = 0x7db6dc + x86::reg32(index) * 8;
            eax = x86::reg32(index) * 8;
            if (word(bucket) == 0)
            {
                word(polygon) = 0;
                word(bucket) = polygon;
                word(bucket + 4) = polygon;
                polygon = next;
                continue;
            }
            const double depth = app->getMemory<float>(polygon + 0x1c);
            x86::reg32 link = bucket;
            while (true)
            {
                const x86::reg32 node = word(link);
                eax = node;
                if (node == 0)
                    break;
                const double nodeDepth = app->getMemory<float>(node + 0x1c);
                compared(nodeDepth, depth);
                cpu.fpu.compare(nodeDepth, depth);
                eax = (node & 0xffff0000) | cpu.fpu.status.word;
                if (!(nodeDepth >= depth))
                {
                    word(polygon) = node;
                    word(link) = polygon;
                    break;
                }
                if (word(node) == 0)
                {
                    word(polygon) = 0;
                    word(word(bucket + 4)) = polygon;
                    word(bucket + 4) = polygon;
                    break;
                }
                link = node;
            }
            polygon = next;
        }
        compared.replay(cpu);
        IntegerFlags flags;
        flags.logic(0);  // test edx, edx
        flags.store(cpu);
        cpu.eax = eax;
    }
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = esp + 4 + 4;
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
 * sub_4c7480 does it instead. */
void depthBucketsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto floatAt = [app](x86::reg32 address) -> double { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 24;
    const x86::reg32 start = word(esp + 4);
    if (word(0x7a3a60) == 0)
    {
        word(frame - 8) = start;
        cpu.edx = cpu.eax;
        cpu.ecx = 0;  // mov ecx, [0x7a3a60]
        cpu.ebp = frame;
        cpu.esp = frame - 8;
        if (!call(app, cpu, 0x4c7480))
            return;
    }
    else
    {
        x86::reg32 polygon = cpu.eax;
        x86::reg32 eax = cpu.eax;
        bool called = false;
        while (polygon != 0)
        {
            const x86::reg32 next = word(polygon);
            const x86::reg32 v0 = word(polygon + 8), v1 = word(polygon + 0xc), v2 = word(polygon + 0x10),
                             v3 = word(polygon + 0x14);
            const x86::reg32 texture = word(polygon + 0x18);
            eax = v1;
            if (texture && (app->getMemory<x86::reg8>(texture + 0x28) & 2))
            {
                word(polygon) = 0;
                word(frame - 4) = next;
                word(frame - 8) = 0;
                cpu.eax = polygon;
                cpu.ebx = v2;
                cpu.ecx = v0;
                cpu.edx = polygon;
                cpu.esi = texture;
                cpu.edi = v3;
                cpu.ebp = frame;
                cpu.esp = frame - 8;
                cpu.esp -= 4;
                if (!sortedBuckets(app, cpu))
                    app->dynamic_call(0x4c7610, cpu);
                if (cpu.terminate)
                    return;
                eax = cpu.eax;
                called = true;
                polygon = next;
                continue;
            }
            const x86::reg16 kind = app->getMemory<x86::reg16>(polygon + 4);
            word(polygon) = 0;
            x86::reg32 alpha = word(v0 + 0x10) & word(v1 + 0x10) & word(v2 + 0x10);
            if (kind == 0)
                alpha &= word(v3 + 0x10);
            if ((alpha >> 24) == 0xff
                && !(texture && (app->getMemory<x86::reg8>(texture + 0x28) & 4)))
            {
                eax = x86::reg32(app->getMemory<x86::reg8>(texture + 0x29)) << 3;
                appendToBucket(app, 0x7d575c + eax, polygon);
                polygon = next;
                continue;
            }
            if (kind > 1)
            {
                polygon = next;
                continue;
            }
            double sum = M::add(M::add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8));
            if (kind == 0)
                sum = M::add(sum, floatAt(v3 + 8));
            const double mean = M::mul(sum, app->getMemory<double>(x86::reg32(kind == 0 ? 0x540254 : 0x540264)));
            const double shifted = M::add(M::sub(mean, asFloat(start)), floatAt(0x7dcfdc));
            x86::sreg32 index = cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(shifted, floatAt(0x54025c))));
            if (index < 0)
                index = 0;
            else if (index > 0x7cf)
                index = 0x7cf;
            eax = x86::reg32(index) << 3;
            appendToBucket(app, 0x7d5f5c + eax, polygon);
            polygon = next;
        }
        NFS2_USE(called);
        IntegerFlags flags;
        flags.logic(0);  // test edx, edx
        flags.store(cpu);
        cpu.eax = eax;
    }
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = esp + 4 + 4;
}

/* sub_4dfd56: st(0) rounded toward zero -- the game's float-to-integer step,
 * frndint under a control word of 0x1f.. (extended precision, truncation),
 * the caller's restored after.  Nothing else changes: the registers, the
 * flags and the status word stay as they were. */
void truncateNative(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    const auto rc = cpu.fpu.control.rc;
    cpu.fpu.control.rc = 3;
    cpu.fpu.st(0) = cpu.fpu.rndint(cpu.fpu.st(0));
    cpu.fpu.control.rc = rc;
    cpu.esp += 4;
}

/* sub_4e01f0: the dot product of the vectors at eax and edx, pushed onto the
 * x87 stack: (a0 b0 + a1 b1) + a2 b2. */
void dotPushNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const float* a = &app->getMemory<float>(cpu.eax);
    const float* b = &app->getMemory<float>(cpu.edx);
    const double sum = M::add(M::add(M::mul(a[0], b[0]), M::mul(a[1], b[1])), M::mul(a[2], b[2]));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(sum);
    cpu.esp += 4;
}

/* sub_4972f0: the vector at eax made of length 1 in place, its length (as a
 * float, 0 for a zero vector, which stays) pushed onto the x87 stack. */
void normaliseNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    float* v = &app->getMemory<float>(cpu.eax);
    const float x = v[0], y = v[1], z = v[2];
    const float length = float(M::sqrt(M::add(M::add(M::mul(y, y), M::mul(x, x)), M::mul(z, z))));
    IntegerFlags flags;
    float result = 0.0f;
    if (asBits(length) & 0x7fffffff)
    {
        flags.logic(asBits(length) & 0x7fffffff);  // test ecx, 0x7fffffff
        const double inverse = M::div(1.0, length);
        v[0] = float(M::mul(x, inverse));
        v[1] = float(M::mul(y, inverse));
        v[2] = float(M::mul(z, inverse));
        result = length;
    }
    else
    {
        flags.logic(0);  // xor ebx, ebx
    }
    flags.store(cpu);
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(result);
    cpu.esp += 4;
}

/* sub_41aab0: how far apart two points of 16.16 integers are on the ground,
 * as the game measures it: (dx >> 12)^2 >> 6 plus (dz >> 12)^2 >> 6. */
inline x86::reg32 groundDistance(win32::WinApplication* app, x86::reg32 a, x86::reg32 b)
{
    const x86::sreg32 dz = (x86::sreg32(app->getMemory<x86::reg32>(b + 8)) - x86::sreg32(app->getMemory<x86::reg32>(a + 8))) >> 12;
    const x86::sreg32 dx = (x86::sreg32(app->getMemory<x86::reg32>(b)) - x86::sreg32(app->getMemory<x86::reg32>(a))) >> 12;
    const x86::sreg32 z2 = x86::sreg32(x86::reg32(dz) * x86::reg32(dz));
    const x86::sreg32 x2 = x86::sreg32(x86::reg32(dx) * x86::reg32(dx));
    return x86::reg32((x2 >> 6) + (z2 >> 6));
}

void groundDistanceNative(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = groundDistance(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* sub_41e0f0: the track block nearest the point at eax (floats, turned to
 * 16.16 by [0x536d74] as fistp rounds), of the [0x5dd940] blocks at
 * [0x5dd948], 36 bytes each; -1 when there are none. */
void nearestBlockNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 frame = esp - 20;  // push ebx, ecx, edx, esi, ebp; mov ebp, esp
    const double scale = app->getMemory<double>(x86::reg32(0x536d74));
    const float* p = &app->getMemory<float>(cpu.eax);
    x86::reg32* point = &app->getMemory<x86::reg32>(frame - 0x10);
    for (int i = 0; i < 3; ++i)
        point[i] = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(NearestMath::mul(p[i], scale))));
    const x86::sreg32 count = x86::sreg32(app->getMemory<x86::reg32>(0x5dd940));
    const x86::reg32 blocks = app->getMemory<x86::reg32>(0x5dd948);
    x86::sreg32 best = 0x7fffffff;
    x86::reg32 nearest = 0xffffffff;
    x86::sreg32 i = 0;
    for (; i < count; ++i)
    {
        const x86::sreg32 distance = x86::sreg32(groundDistance(app, blocks + x86::reg32(i) * 36, frame - 0x10));
        if (distance < best)
        {
            best = distance;
            nearest = x86::reg32(i);
        }
    }
    IntegerFlags flags;
    flags.compare(x86::reg32(i), x86::reg32(count));  // cmp ecx, [ebp-4]
    flags.store(cpu);
    cpu.eax = nearest;
    cpu.esp = esp + 4;
}

/* sub_4c7320: 800 buckets (0x7d9ddc to 0x7db6d4, below the ones sub_4c7610
 * sorts into) as one list, from the top one down, into the head and tail at
 * esi; returns esi. */
void gatherSortedNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 out = cpu.esi;
    const x86::reg32 frame = cpu.esp - 20;  // push ebx, ecx, edx, edi, ebp
    x86::reg32 head = 0, tail = 0;
    for (x86::reg32 i = 0; i < 0x320; ++i)
    {
        const x86::reg32 at = 0x7db6d4 - i * 8;
        const x86::reg32 first = word(at);
        if (first == 0)
            continue;
        if (head == 0)
            head = first;
        else
            word(tail) = first;
        tail = word(at + 4);
    }
    word(frame - 8) = head;
    word(frame - 4) = tail;
    word(out) = head;
    word(out + 4) = tail;
    IntegerFlags flags;
    flags.compare(0x320, 0x320);  // cmp ecx, 0x320
    flags.store(cpu);
    cpu.eax = out;
    cpu.esi = frame;  // lea esi, [ebp-8]; two movsd
    cpu.esp += 4;
}

/* sub_41a970: an object's vertices (sub_4bf4c0, the stand-in objectVertices,
 * with the view's matrix at +0x38 and move at +0x44), each then given the
 * colour from the list at ecx.
 *   eax  the view; edx  the count; ebx  the vertices in; ecx  the colours;
 *   [esp+4]  the vertices out (ret 4) */
void objectVerticesColouredNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 12;
    const x86::reg32 count = cpu.edx;
    x86::reg32 colours = cpu.ecx;
    x86::reg32 out = app->getMemory<x86::reg32>(esp + 4);
    app->getMemory<x86::reg32>(frame - 4) = out;  // push ecx
    cpu.esi = count;
    cpu.edi = colours;
    cpu.edx = cpu.ebx;
    cpu.ecx = cpu.eax + 0x38;
    cpu.ebx = cpu.eax + 0x44;
    cpu.eax = count;
    cpu.ebp = frame;
    cpu.esp = frame - 4;
    cpu.esp -= 4;
    if (!objectVertices(app, cpu))
        app->dynamic_call(0x4bf4c0, cpu);
    if (cpu.terminate)
        return;
    x86::reg32 left = count;
    IntegerFlags flags;
    while (true)
    {
        --left;
        flags.compare(left, 0xffffffff);  // cmp esi, -1
        if (left == 0xffffffff)
            break;
        cpu.edx = out;
        cpu.eax = app->getMemory<x86::reg32>(colours);
        colours += 4;
        cpu.ebx = out + 0x20;
        app->getMemory<x86::reg32>(out + 0x10) = cpu.eax;
        out += 0x20;
    }
    flags.store(cpu);
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = esp + 4 + 4;
}

/* sub_4e0721 (Watcom's memset core; sub_4e0716 spreads bl over ebx first,
 * sub_4e070c fills with zeros): edx bytes at eax from the pattern in ebx --
 * up to an 8-byte boundary a byte, a word and a dword at a time, then 32 and 8
 * bytes at a time (as doubles when the pattern's bytes are all one value other
 * than 0xff), then what is left.  eax ends past the bytes; edx and the flags
 * as its last steps leave them. */
void fillCore(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 at = cpu.eax;
    x86::sreg32 left = x86::sreg32(cpu.edx);
    const x86::reg32 pattern = cpu.ebx;
    x86::reg8* memory = &app->getMemory<x86::reg8>(0);
    auto put = [&](x86::reg32 bytes) {
        std::memcpy(memory + at, &pattern, bytes);
        at += bytes;
    };
    IntegerFlags flags;
    if (at & 7)
    {
        if ((at & 1) && left >= 1)
        {
            put(1);
            left -= 1;
        }
        if ((at & 2) && left >= 2)
        {
            put(2);
            left -= 2;
        }
        if ((at & 4) && left >= 4)
        {
            put(4);
            left -= 4;
        }
    }
    flags.compare(x86::reg32(left), 0);  // cmp edx, 0
    if (left >= 0)
    {
        // 32 at a time: sub edx, 0x20 until it goes negative; then 8 at a time
        left -= 0x20;
        if (left >= 0)
        {
            // one fill for the whole run of 32-byte blocks
            const x86::reg32 blocks = x86::reg32(left) / 0x20 + 1;
            x86::reg8* to = memory + at;
            for (x86::reg32 i = 0; i < blocks * 8; ++i)
                std::memcpy(to + i * 4, &pattern, 4);
            at += blocks * 0x20;
            left -= x86::sreg32(blocks * 0x20);
        }
        left += 0x18;
        while (left >= 0)
        {
            put(4);
            put(4);
            left -= 8;
        }
        const x86::reg32 before = x86::reg32(left);
        left += 8;
        // add edx, 8
        const x86::reg32 sum = x86::reg32(left);
        flags.cf = sum < before;
        flags.of = (~(before ^ 8u) & (before ^ sum)) >> 31;
        flags.zf = sum == 0;
        flags.sf = sum >> 31;
        if (left != 0)
        {
            if (left & 4)
                put(4);
            if (left & 2)
                put(2);
            if (left & 1)
                put(1);
            flags.logic(x86::reg32(left) & 1);  // test edx, 1
        }
    }
    flags.store(cpu);
    cpu.eax = at;
    cpu.edx = x86::reg32(left);
}

void fillWordNative(win32::WinApplication* app, x86::CPU& cpu)
{
    fillCore(app, cpu);
    cpu.esp += 4;
}

void fillByteNative(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 ebx = (cpu.ebx & 0xffff0000) | (cpu.ebx & 0xff) << 8 | (cpu.ebx & 0xff);
    ebx |= ebx << 16;
    cpu.ebx = ebx;
    fillCore(app, cpu);
    cpu.esp += 4;
}

void fillZeroNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 ebx = cpu.ebx;
    cpu.ebx = 0;
    fillCore(app, cpu);
    cpu.ebx = ebx;
    cpu.esp += 4;
}

/* sub_4968a0: a point brought onto the ground plane of a frame -- p - o turned
 * into the frame by the transpose of the matrix at eax (sub_4e0110), its height
 * dropped, turned back and o added -- and the height of the result pushed
 * onto the x87 stack.
 *   eax  the matrix; [esp+4..0xc]  o; [esp+0x10..0x18]  p (ret 0x18)
 * Only the stack and the arguments are written; the flags are untouched. */
void groundHeightNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const x86::reg32 esp = cpu.esp;
    const float* m = &app->getMemory<float>(cpu.eax);
    const float* args = &app->getMemory<float>(esp + 4);
    const float o[3] = { args[0], args[1], args[2] };
    const float d[3] = { float(M::sub(args[3], o[0])), float(M::sub(args[4], o[1])), float(M::sub(args[5], o[2])) };
    float r[3];
    for (int j = 0; j < 3; ++j)
        r[j] = float(M::add(M::add(M::mul(d[0], m[3 * j]), M::mul(d[1], m[3 * j + 1])), M::mul(d[2], m[3 * j + 2])));
    r[1] = 0.0f;
    float q[3];
    for (int j = 0; j < 3; ++j)
        q[j] = float(M::add(M::add(M::mul(r[0], m[j]), M::mul(r[1], m[3 + j])), M::mul(r[2], m[6 + j])));
    for (int j = 0; j < 3; ++j)
        q[j] = float(M::add(q[j], o[j]));
    float* written = &app->getMemory<float>(esp + 0x10);
    written[0] = d[0];
    written[1] = d[1];
    written[2] = d[2];
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(q[1]);
    cpu.eax = esp - 16 - 0xc;  // lea eax, [eax + 0xc] past the second turn's output, [ebp-0xc]
    cpu.esp = esp + 4 + 0x18;
}

/* A screen vertex's clip code from its x, y and 1/z at +0, +4, +0xc, compared
 * as integers with the bounds at 0x7d34f8..0x7d350c -- the code sub_41bf30,
 * sub_4cbc90 and others write inline.  `edx` becomes what the inline code
 * leaves in it: the right bound, or unchanged for a vertex behind the near
 * plane. */
inline x86::reg8 screenCode(win32::WinApplication* app, x86::reg32 vertex, x86::reg32& edx, IntegerFlags& flags)
{
    /* The flags its last test, cmp or or leaves (or ebx, 8 sets none). */
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 inverse = word(vertex + 0xc);
    if (inverse & 0x80000000)
    {
        flags.logic(inverse & 0x80000000);
        return 0x10;
    }
    flags.compare(inverse, word(0x7d34f8));
    if (x86::sreg32(inverse) >= x86::sreg32(word(0x7d34f8)))
        return 0x10;
    x86::reg8 code = 0;
    const x86::reg32 y = word(vertex + 4);
    if ((y & 0x80000000) || x86::sreg32(y) < x86::sreg32(word(0x7d3504)))
        code |= 8;
    else if (x86::sreg32(y) > x86::sreg32(word(0x7d3500)))
        code |= 4;
    const x86::reg32 x = word(vertex);
    edx = word(0x7d350c);
    if ((x & 0x80000000) || x86::sreg32(x) < x86::sreg32(word(0x7d34fc)))
    {
        code |= 1;
        flags.logic(code);
    }
    else
    {
        flags.compare(x, edx);
        if (x86::sreg32(x) > x86::sreg32(edx))
        {
            code |= 2;
            flags.logic(code);
        }
    }
    return code;
}

inline x86::reg8 screenCode(win32::WinApplication* app, x86::reg32 vertex, x86::reg32& edx)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 inverse = word(vertex + 0xc);
    if ((inverse & 0x80000000) || x86::sreg32(inverse) >= x86::sreg32(word(0x7d34f8)))
        return 0x10;
    x86::reg8 code = 0;
    const x86::reg32 y = word(vertex + 4);
    if ((y & 0x80000000) || x86::sreg32(y) < x86::sreg32(word(0x7d3504)))
        code |= 8;
    else if (x86::sreg32(y) > x86::sreg32(word(0x7d3500)))
        code |= 4;
    const x86::reg32 x = word(vertex);
    if ((x & 0x80000000) || x86::sreg32(x) < x86::sreg32(word(0x7d34fc)))
        code |= 1;
    else if (x86::sreg32(x) > x86::sreg32(word(0x7d350c)))
        code |= 2;
    edx = word(0x7d350c);
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
 * for every particle not drawn this way.  Returns esi. */
void particlesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    auto putFloat = [app](x86::reg32 address, double value) { app->getMemory<float>(address) = float(value); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edi, cpu.ebp };
    const x86::reg32 ebp = esp - 16 - 0x82;  // push ebx, ecx, edi, ebp; mov ebp, esp; sub ebp, 0x82
    const x86::reg32 view = cpu.eax;
    const x86::reg32 matrix = cpu.edx;
    const x86::reg32 out = cpu.esi;
    LastWideCompare compared;
    auto finish = [&](x86::reg32 edx) {
        word(out) = word(ebp - 0x1e);
        word(out + 4) = word(ebp - 0x1a);
        cpu.eax = out;
        cpu.edx = edx;
        cpu.esi = ebp - 0x16;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };

    // sub_4bbe70 into [ebp-0xe]: an empty list
    word(ebp - 0xe) = 0;
    word(ebp - 0xa) = 0;
    const x86::reg32 alphaScale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(
        x86::Float(M::mul(floatAt(0x6fbc38), app->getMemory<double>(x86::reg32(0x540448))))));
    const float camera[3] = { floatAt(view + 0x38), floatAt(view + 0x3c), floatAt(view + 0x40) };
    const x86::reg32 player = word(view + 4);

    cpu.ebp = ebp;
    cpu.esp = ebp + 0x82 - 0xe4;
    cpu.esi = view + 0x44;  // past the camera's three, copied with movsd
    cpu.edi = ebp - 0x1e;
    cpu.eax = ebp + 0x22;
    cpu.edx = 0x17700;
    if (!call(app, cpu, 0x4bbe40))
        return;
    if (cpu.eax == 0)
    {
        IntegerFlags flags;
        flags.logic(0);  // test eax, eax
        flags.store(cpu);
        word(ebp - 0x1e) = 0;
        word(ebp - 0x1a) = 0;
        finish(cpu.edx);
        return;
    }
    const x86::reg32 first = word(ebp + 0x22);
    x86::reg32 cursor = first;
    x86::reg32 count = 0;
    x86::reg32 edx = 0;
    const double k1 = floatAt(0x56009c), k2 = floatAt(0x5600a0), c1 = floatAt(0x5600a4), c2 = floatAt(0x5600a8);
    const x86::reg32 viewLevel = word(0x6fbc28);
    for (x86::reg32 i = 0; i < 0x258; ++i)
    {
        const x86::reg32 particle = 0x7ddfd0 + i * 0x80;
        const float position[3] = { floatAt(particle + 0xc), floatAt(particle + 0x10), floatAt(particle + 0x14) };
        float sizeX = float(M::mul(floatAt(particle + 0x24), k1));
        float sizeY = float(M::mul(floatAt(particle + 0x24), k2));
        const x86::reg32 kind = word(particle + 4);
        const x86::reg32 pass = word(view);
        const x86::reg32 flag = particle + 0x78 + player * 2 + (pass == 1 ? 1 : 0);
        edx = flag;
        if (word(0x6fbc30) == 1 && (byte(particle + 0x7d) & 1))
        {
            byte(flag) = 0;
            continue;
        }
        if (word(particle) == 0)
        {
            byte(flag) = 0;
            continue;
        }
        const float* m = &app->getMemory<float>(matrix);
        float x = float(M::add(M::add(M::mul(position[0], m[0]), M::mul(position[1], m[3])), M::mul(position[2], m[6])));
        float y = float(M::add(M::add(M::mul(position[0], m[1]), M::mul(position[1], m[4])), M::mul(position[2], m[7])));
        float z = float(M::add(M::add(M::mul(position[0], m[2]), M::mul(position[1], m[5])), M::mul(position[2], m[8])));
        x = float(M::add(x, camera[0]));
        y = float(M::add(y, camera[1]));
        z = float(M::add(z, camera[2]));
        edx = matrix;
        compared(0.0, z);
        if (0.0 >= z)
        {
            byte(flag) = 0;
            continue;
        }
        edx = kind << 7;
        const double reach = M::mul(floatAt(0x560bc4 + (kind << 7)), floatAt(0x4ca480 + viewLevel * 4));
        compared(reach, z);
        if (!(reach >= z))
        {
            byte(flag) = 0;
            continue;
        }
        if (!(asBits(z) & 0x7fffffff))
            z = asFloat(0x37800080);
        const float inverse = float(M::div(1.0, z));
        const x86::reg32 centre = ebp - 0x62;
        putFloat(centre, M::add(M::mul(M::mul(k1, inverse), x), c1));
        putFloat(centre + 4, M::add(M::mul(M::mul(k2, inverse), y), c2));
        word(centre + 8) = asBits(z);
        word(centre + 0xc) = asBits(inverse);
        edx = asBits(z);
        byte(centre + 0x14) = screenCode(app, centre, edx);
        if (x86::sreg32(asBits(z)) < 0x40400000 && pass == 0)
        {
            // a particle on the camera: sub_4ca500 may splash it over the view
            compared.replay(cpu);
            compared.any = false;
            word(cpu.esp - 4) = kind;
            word(cpu.esp - 8) = asBits(z);
            word(cpu.esp - 12) = asBits(y);
            word(cpu.esp - 16) = asBits(x);
            cpu.esp -= 16;
            cpu.eax = player;
            cpu.ebx = asBits(x);
            cpu.ecx = asBits(y);
            cpu.edx = edx;
            cpu.esi = asBits(z);
            cpu.edi = 0;
            IntegerFlags tested;
            tested.logic(0);  // test edi, edi
            tested.store(cpu);
            if (!call(app, cpu, 0x4ca500))
                return;
            edx = cpu.edx;
            if (cpu.eax != 0)
            {
                byte(flag) = 0;
                word(particle) = cpu.edi;
                continue;
            }
        }

        const x86::reg32 corners[4] = { cursor + 0x20, cursor + 0x40, cursor + 0x60, cursor + 0x80 };
        x86::reg32 colour = word(particle + 0x54);
        // [0x7a3a58] & 0x40 forced on (tools/apply_alpha_intensity.py)
        const double alpha = floatAt(0x6fbc38);
        compared(1.0, alpha);
        if (1.0 > alpha && (byte(0x560bbc + (kind << 7)) & 1))
        {
            auto channel = [&](x86::reg32 at) {
                return (x86::reg32(x86::sreg32(x86::reg32(byte(particle + at)) * alphaScale) >> 16)) & 0xff;
            };
            colour = channel(0x57) << 24 | channel(0x56) << 16 | channel(0x55) << 8 | channel(0x54);
        }
        compared(1.0, z);
        if (!(1.0 > z))
        {
            const float scale = float(M::div(1.0, z));
            sizeX = float(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(sizeX, scale))));
            sizeY = float(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(sizeY, scale))));
        }
        app->getMemory<x86::reg16>(cursor + 4) = 0;
        word(cursor) = cursor + 0xa0;
        app->getMemory<x86::reg16>(cursor + 6) = app->getMemory<x86::reg16>(0x560bbc + (kind << 7));
        word(cursor + 0x18) = word(particle + 0x50);
        for (int c = 0; c < 4; ++c)
            word(cursor + 8 + x86::reg32(c) * 4) = corners[c];
        word(cursor + 0x1c) = asBits(z);
        for (int c = 0; c < 4; ++c)
        {
            word(corners[c] + 8) = asBits(z);
            word(corners[c] + 0xc) = asBits(inverse);
            word(corners[c] + 0x10) = colour;
        }
        edx = colour;
        const double cx = floatAt(centre), cy = floatAt(centre + 4);
        if (byte(particle + 0x7c) != 0)
        {
            const x86::reg32 previous = particle + 0x58 + (player << 4) + ((pass == 1 ? 1u : 0u) << 3);
            edx = previous;
            const double limit = app->getMemory<double>(x86::reg32(0x540450));
            auto clamp = [&](double size) {
                compared(size, limit);
                if (!(size >= limit))
                    return 0.5;
                compared(1.0, size);
                return 1.0 >= size ? size : 1.0;  // never wider than a pixel
            };
            const float width = float(clamp(sizeX));
            const float height = float(clamp(sizeY));
            if (byte(flag) != 0)
            {
                const float px = floatAt(previous);
                word(corners[0]) = asBits(px);
                word(corners[1]) = asBits(px);
                word(corners[0] + 4) = word(previous + 4);
                word(corners[1] + 4) = word(previous + 4);
                putFloat(corners[2], M::add(cx, width));
                putFloat(corners[2] + 4, M::add(cy, height));
                putFloat(corners[3], M::sub(cx, width));
                putFloat(corners[3] + 4, M::sub(cy, height));
                const double half = floatAt(0x540458);
                putFloat(previous, M::mul(M::add(cx, floatAt(previous)), half));
                putFloat(previous + 4, M::mul(M::add(cy, floatAt(previous + 4)), half));
            }
            else
            {
                word(corners[0]) = word(centre);
                word(corners[0] + 4) = word(centre + 4);
                putFloat(corners[1], M::add(cx, width));
                putFloat(corners[1] + 4, M::sub(cy, height));
                putFloat(corners[2], M::add(width, cx));
                putFloat(corners[2] + 4, M::add(cy, height));
                word(corners[3]) = word(centre);
                putFloat(corners[3] + 4, M::add(height, cy));
                word(previous) = word(centre);
                word(previous + 4) = word(centre + 4);
            }
            byte(flag) = 1;
        }
        else
        {
            const double a = M::mul(floatAt(particle + 0x3c), sizeY);
            const double b = M::mul(floatAt(particle + 0x38), sizeX);
            putFloat(corners[0], M::sub(cx, a));
            putFloat(corners[0] + 4, M::add(cy, b));
            putFloat(corners[1], M::add(cx, b));
            putFloat(corners[1] + 4, M::add(cy, a));
            putFloat(corners[2], M::add(cx, a));
            putFloat(corners[2] + 4, M::sub(cy, b));
            putFloat(corners[3], M::sub(cx, b));
            putFloat(corners[3] + 4, M::sub(cy, a));
        }
        for (int c = 0; c < 4; ++c)
            byte(corners[c] + 0x14) = screenCode(app, corners[c], edx);
        // the stack copies the original keeps: the sizes it rounded
        putFloat(ebp + 0x4e, sizeX);
        putFloat(ebp + 0x52, sizeY);
        cursor += 0xa0;
        ++count;
    }

    compared.replay(cpu);
    IntegerFlags flags;
    if (count == 0)
    {
        flags.logic(0);  // test eax, eax
        flags.store(cpu);
        word(ebp - 0x1e) = 0;
        word(ebp - 0x1a) = 0;
        finish(edx);
        return;
    }
    const x86::reg32 last = cursor - 0xa0;
    // sub ecx, 0xa0
    flags.compare(cursor, 0xa0);
    flags.store(cpu);
    cpu.eax = ebp + 0x22;
    cpu.edx = count * 0xa0;
    cpu.ecx = last;
    if (!call(app, cpu, 0x4bbde0))
        return;
    word(ebp - 0xa) = last;
    word(ebp - 0xe) = word(ebp + 0x22);
    word(last) = 0;
    word(ebp - 0x1e) = word(ebp - 0xe);
    word(ebp - 0x1a) = word(ebp - 0xa);
    finish(count * 0xa0);
}

/* sub_4cb750: the drops on the view of one player (0x7f0bd0, 0x3200 bytes a
 * player, 200 drops of 64 bytes), drawn as screen-space quads hung on the
 * overlay list (sub_4bbf80): each corner at +8.. +0x24 scaled to the screen,
 * depth 0 and 1/z 0x3f7fff00, the colour at +0x28 (scaled by the alpha
 * intensity slider when the drop's flags at +0x2c ask; the [0x7a3a58] & 0x40
 * test is forced on, tools/apply_alpha_intensity.py), the flags and the
 * texture at +0x30.  Nothing under camera modes 8, 0xe and 0x12.
 *   eax  the view (+4 its player)
 * Only eax is not kept: what its last step left in it. */
void dropsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 24;
    const x86::reg32 view = cpu.eax;
    const x86::reg32 player = word(view + 4);
    auto finish = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = esp + 4;
    };
    const x86::reg32 alphaScale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(
        x86::Float(M::mul(floatAt(0x6fbc38), app->getMemory<double>(x86::reg32(0x540440))))));
    // The camera's mode, three times as the original reads it.
    auto cameraMode = [&]() -> x86::reg32 {
        const x86::reg32 first = word(0x5e10b8 + player * 4);
        if (first)
            return word(first);
        const x86::reg32 second = word(0x5e10b0 + player * 4);
        return second ? x86::reg32(word(second)) : 0u;
    };
    IntegerFlags flags;
    for (const x86::reg32 skipped : { 8u, 0xeu, 0x12u })
    {
        const x86::reg32 mode = cameraMode();
        flags.compare(mode, skipped);
        if (mode == skipped)
        {
            flags.store(cpu);
            finish(mode);
            return;
        }
    }
    LastWideCompare compared;
    const double k1 = floatAt(0x56009c), k2 = floatAt(0x5600a0), c1 = floatAt(0x5600a4), c2 = floatAt(0x5600a8);
    const x86::reg32 drops = 0x7f0bd0 + player * 0x3200;
    x86::reg32 eax = 0;
    cpu.ebp = frame;
    for (x86::reg32 i = 0; i < 0xc8; ++i)
    {
        const x86::reg32 drop = drops + (i << 6);
        eax = i << 6;
        if (word(drop) == 0)
            continue;
        compared.replay(cpu);
        compared.any = false;
        cpu.eax = frame - 0x28;
        cpu.edx = 0xa0;
        cpu.esp = frame - 0x28;
        if (!call(app, cpu, 0x4bbf80))
            return;
        if (cpu.eax == 0)
        {
            flags.logic(0);  // test eax, eax
            flags.store(cpu);
            finish(0);
            return;
        }
        const x86::reg32 polygon = word(frame - 0x28);
        x86::reg32 colour = word(drop + 0x28);
        const double alpha = floatAt(0x6fbc38);
        compared(1.0, alpha);
        if (1.0 > alpha && (byte(drop + 0x2c) & 1))
        {
            auto channel = [&](x86::reg32 at) {
                return (x86::reg32(x86::sreg32(x86::reg32(byte(drop + at)) * alphaScale) >> 16)) & 0xff;
            };
            colour = channel(0x2b) << 24 | channel(0x2a) << 16 | channel(0x29) << 8 | channel(0x28);
        }
        app->getMemory<x86::reg16>(polygon + 4) = 0;
        app->getMemory<x86::reg16>(polygon + 6) = app->getMemory<x86::reg16>(drop + 0x2c);
        for (x86::reg32 c = 0; c < 4; ++c)
            word(polygon + 8 + c * 4) = polygon + 0x20 + c * 0x20;
        word(polygon + 0x18) = word(drop + 0x30);
        x86::reg32 edx = 0;
        for (x86::reg32 c = 0; c < 4; ++c)
        {
            const x86::reg32 corner = polygon + 0x20 + c * 0x20;
            app->getMemory<float>(corner) = float(M::add(M::mul(floatAt(drop + 8 + c * 8), k1), c1));
            word(corner + 8) = 0;
            word(corner + 0xc) = 0x3f7fff00;
            app->getMemory<float>(corner + 4) = float(M::add(M::mul(floatAt(drop + 0xc + c * 8), k2), c2));
            byte(corner + 0x14) = screenCode(app, corner, edx);
            word(corner + 0x10) = colour;
        }
        eax = colour;
    }
    compared.replay(cpu);
    flags.compare(0xc8, 0xc8);  // cmp [ebp-0x1c], 0xc8
    flags.store(cpu);
    finish(eax);
}

/* sub_4cb2c0: the 600 particles moved on a frame.  Every live one moves by its
 * velocity times [0x540420]; on odd frames (and after sub_4cb220 for each
 * player first) it also ages: its spin turns it, bounced back off the
 * limits [0x540410]/[0x540418] where its kind (0x560b64, 128 bytes) says so
 * and damped; a swirling one (+0x44) swings its velocity round; its size
 * grows with age; every fourth frame its colour fades by its kind's; its
 * velocity is damped and falls by its kind's gravity; it loses a frame of
 * life, dies with its alpha, and one that spawns (+0x7e) hands a copy of
 * itself to sub_4cab60 as it dies.  Only eax is not kept. */
void moveParticlesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    auto putFloat = [app](x86::reg32 address, double value) { app->getMemory<float>(address) = float(value); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 frame = esp - 24;
    cpu.ebp = frame;
    cpu.esp = frame - 0x38;
    const x86::reg32 odd = word(0x7d3684) & 1;
    x86::reg32 eax = odd;
    if (odd)
    {
        cpu.eax = 0;
        if (!call(app, cpu, 0x4cb220))
            return;
        eax = cpu.eax;
        if (word(0x6fd3b0) == 1)
        {
            cpu.eax = 1;
            if (!call(app, cpu, 0x4cb220))
                return;
            eax = cpu.eax;
        }
    }
    LastWideCompare compared;
    const double step = app->getMemory<double>(x86::reg32(0x540420));
    auto trig = [&](bool sine, x86::reg32 at) {
        const x86::Float angle = cpu.fpu.mul(x86::Float(floatAt(at)),
                                             x86::Float(app->getMemory<double>(x86::reg32(sine ? 0x54b6b0 : 0x54b6b8))));
        return double(sine ? cpu.fpu.sin(angle) : cpu.fpu.cos(angle));
    };
    for (x86::reg32 i = 0; i < 0x258; ++i)
    {
        const x86::reg32 p = 0x7ddfd0 + i * 0x80;
        const x86::reg32 life = word(p);
        if (life == 0)
            continue;
        const x86::reg32 velocity = p + 0x18;
        const x86::reg32 position = p + 0xc;
        if (odd)
        {
            const x86::reg32 kind = 0x560b64 + (word(p + 4) << 7);
            const double damping = floatAt(kind + 0x38);
            word(frame - 8) = word(p + 8) - life;
            const float age = float(M::div(double(x86::sreg32(word(frame - 8))), double(x86::sreg32(word(p + 8)))));
            if (word(p + 0x44) & 0x7fffffff)
            {
                const double s = trig(true, p + 0x40);
                putFloat(p + 0x18, M::add(M::mul(s, floatAt(p + 0x48)), floatAt(p + 0x18)));
                const double c = M::mul(trig(false, p + 0x40), floatAt(p + 0x48));
                const float swirl = float(M::mul(floatAt(p + 0x4c), damping));
                const float turn = float(M::mul(damping, floatAt(p + 0x44)));
                app->getMemory<float>(p + 0x4c) = swirl;
                app->getMemory<float>(p + 0x44) = turn;
                putFloat(p + 0x20, M::add(c, floatAt(p + 0x20)));
                putFloat(p + 0x48, M::add(double(swirl), floatAt(p + 0x48)));
                putFloat(p + 0x40, M::add(double(turn), floatAt(p + 0x40)));
            }
            const x86::reg32 spinBits = word(p + 0x34);
            if (spinBits & 0x7fffffff)
            {
                if (word(kind + 0x3c) != 0)
                {
                    const double spin = floatAt(p + 0x34);
                    compared(0.0, spin);
                    const x86::reg32 size = (0.0 <= spin || spin != spin) ? spinBits : asBits(float(-spin));
                    const double angle = floatAt(p + 0x30);
                    compared(angle, app->getMemory<double>(x86::reg32(0x540410)));
                    x86::reg32 bounced;
                    if (angle > app->getMemory<double>(x86::reg32(0x540410)))
                        bounced = asBits(-asFloat(size));
                    else
                    {
                        compared(angle, app->getMemory<double>(x86::reg32(0x540418)));
                        bounced = angle >= app->getMemory<double>(x86::reg32(0x540418)) ? spinBits : size;
                    }
                    word(p + 0x34) = bounced;
                }
                putFloat(p + 0x30, M::add(floatAt(p + 0x34), floatAt(p + 0x30)));
                const double c = trig(false, p + 0x30);
                app->getMemory<float>(p + 0x38) = float(c);
                const double s = trig(true, p + 0x30);
                const float damped = float(M::mul(floatAt(p + 0x34), damping));
                app->getMemory<float>(p + 0x3c) = float(s);
                app->getMemory<float>(p + 0x34) = damped;
            }
            putFloat(p + 0x24, M::add(M::mul(floatAt(p + 0x2c), age), floatAt(p + 0x28)));
            if ((byte(0x7d3684) & 2) && word(kind + 0x50) != 0)
            {
                for (int c = 3; c >= 0; --c)
                {
                    const x86::sreg32 fade = cpu.fpu.toInteger<x86::sreg32>(
                        x86::Float(M::mul(double(byte(kind + 0x50 + x86::reg32(c))), age)));
                    byte(p + 0x54 + x86::reg32(c)) = x86::reg8(byte(kind + 0x4c + x86::reg32(c)) - x86::reg8(fade));
                }
            }
            const double vx = M::mul(floatAt(velocity), damping);
            const double fallen = M::add(floatAt(kind + 0x2c), floatAt(velocity + 4));
            const double vz = M::mul(floatAt(velocity + 8), damping);
            const double vy = M::mul(damping, fallen);
            putFloat(velocity, vx);
            putFloat(velocity + 8, vz);
            putFloat(velocity + 4, vy);
            word(p) = word(p) - 1;
            if (byte(p + 0x57) == 0)
                word(p) = 0;
            if (byte(p + 0x7e) != 0 && word(p) == 0)
            {
                // a copy of the particle on the stack for sub_4cab60 (ret 0x80)
                compared.replay(cpu);
                compared.any = false;
                cpu.esp = frame - 0x38 - 0x80;
                std::memmove(&app->getMemory<x86::reg8>(cpu.esp), &app->getMemory<x86::reg8>(p), 0x80);
                cpu.ebx = velocity;
                cpu.edx = p;
                cpu.ecx = 0;
                cpu.esi = p + 0x80;
                cpu.edi = frame - 0x38;
                IntegerFlags dead;
                dead.compare(0, 0);  // cmp dword ptr [edx], 0
                dead.store(cpu);
                if (!call(app, cpu, 0x4cab60))
                    return;
                cpu.esp = frame - 0x38;
            }
        }
        putFloat(position, M::add(M::mul(floatAt(velocity), step), floatAt(position)));
        putFloat(position + 4, M::add(M::mul(floatAt(velocity + 4), step), floatAt(position + 4)));
        putFloat(position + 8, M::add(M::mul(step, floatAt(velocity + 8)), floatAt(position + 8)));
        eax = position;
    }
    compared.replay(cpu);
    IntegerFlags flags;
    flags.compare(0x258, 0x258);  // cmp [ebp-0x1c], 0x258
    flags.store(cpu);
    cpu.eax = eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = esp + 4;
}

x86::reg32 scaleColour(x86::reg32 colour, x86::reg32 scale);

/* The projection inline in the game's drawing: x, y, z in view space to the
 * screen record at `record` -- x, y on the screen, z, 1/z -- as sub_41bf30
 * and the rest write it, z of zero first made 0x37800080. */
inline void projectToScreen(win32::WinApplication* app, x86::reg32 record, float x, float y, float& z)
{
    typedef NearestMath M;
    if (!(asBits(z) & 0x7fffffff))
        z = asFloat(0x37800080);
    const float inverse = float(M::div(1.0, z));
    const double k1 = app->getMemory<float>(x86::reg32(0x56009c));
    const double k2 = app->getMemory<float>(x86::reg32(0x5600a0));
    app->getMemory<float>(record) =
        float(M::add(M::mul(M::mul(k1, inverse), x), app->getMemory<float>(x86::reg32(0x5600a4))));
    app->getMemory<float>(record + 4) =
        float(M::add(M::mul(M::mul(k2, inverse), y), app->getMemory<float>(x86::reg32(0x5600a8))));
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
 * tools/apply_alpha_intensity.py); the square, of the size scaled by the
 * light's own 1/z, is turned by (1 + x/z) * [0x53bd00], its texture 0x8b4840.
 * The arguments are written as the original writes them.  Only eax is not
 * kept. */
void lightGlowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 ebp = esp - 24;
    const x86::reg32 view = cpu.eax;
    LastWideCompare compared;
    auto finish = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = esp + 4 + 0x1c;
    };
    auto statusInto = [&](x86::reg32 eax) {
        // fnstsw ax after the last fcomp
        return (eax & 0xffff0000) | cpu.fpu.status.word;
    };
    word(ebp - 0x2c) = 0;
    const double size = floatAt(ebp + 0x2c);
    compared(0.0, size);
    if (0.0 >= size)
    {
        compared.replay(cpu);
        cpu.flags.of = false;  // xor edx, edx
        finish(statusInto(view));
        return;
    }
    const float* m = &app->getMemory<float>(view + 0x44);
    const float p0 = floatAt(ebp + 0x1c), p1 = floatAt(ebp + 0x20), p2 = floatAt(ebp + 0x24);
    float x = float(M::add(M::add(M::mul(p0, m[0]), M::mul(p1, m[3])), M::mul(p2, m[6])));
    float y = float(M::add(M::add(M::mul(p0, m[1]), M::mul(p1, m[4])), M::mul(p2, m[7])));
    float z = float(M::add(M::add(M::mul(p0, m[2]), M::mul(p1, m[5])), M::mul(p2, m[8])));
    x = float(M::add(x, floatAt(view + 0x38)));
    y = float(M::add(y, floatAt(view + 0x3c)));
    z = float(M::add(z, floatAt(view + 0x40)));
    app->getMemory<float>(ebp - 0x40) = x;
    app->getMemory<float>(ebp - 0x3c) = y;
    app->getMemory<float>(ebp - 0x38) = z;
    const x86::reg32 afterTurn = ebp + 0x28;  // lea eax, [eax + 0xc] past the light's three
    IntegerFlags flags;
    flags.compare(asBits(z), 0x43960000);  // cmp ecx, 300.0
    if (x86::sreg32(asBits(z)) > 0x43960000)
    {
        compared.replay(cpu);
        flags.store(cpu);
        finish(afterTurn);
        return;
    }
    compared(0.0, z);
    if (0.0 >= z)
    {
        compared.replay(cpu);
        flags.store(cpu);
        finish(statusInto(afterTurn));
        return;
    }
    const x86::reg32 record = ebp - 0x60;
    projectToScreen(app, record, x, y, z);
    app->getMemory<float>(ebp - 0x38) = z;
    x86::reg32 edx = 0;
    app->getMemory<x86::reg8>(record + 0x14) = screenCode(app, record, edx);
    const x86::reg32 lightInverse = word(record + 0xc);
    word(ebp - 4) = lightInverse;
    if (!(word(ebp + 0x30) & 0x7fffffff))
        word(ebp + 0x30) = word(record + 8);
    const double depth = M::add(floatAt(ebp + 0x30), floatAt(ebp + 0x34));
    app->getMemory<float>(ebp + 0x30) = float(depth);
    app->getMemory<float>(record + 0xc) = float(M::div(1.0, depth));
    word(record + 8) = word(ebp + 0x30);
    const x86::reg8 code = screenCode(app, record, edx);
    app->getMemory<x86::reg8>(record + 0x14) = code;
    if (code != 0)
    {
        compared.replay(cpu);
        flags.cf = false;  // cmp byte ptr [ebp-0x4c], 0
        flags.of = false;
        flags.zf = false;
        flags.sf = (code & 0x80) != 0;
        flags.store(cpu);
        // eax as the inline code left it: 1/z behind the near plane, else x
        const x86::reg32 inverse = word(record + 0xc);
        const bool behind = (inverse & 0x80000000) || x86::sreg32(inverse) >= x86::sreg32(word(0x7d34f8));
        finish(behind ? inverse : word(record));
        return;
    }
    // [0x7a3a58] & 0x40 forced on (tools/apply_alpha_intensity.py)
    const double alpha = floatAt(0x6fbc38);
    compared(1.0, alpha);
    bool scaled = false;
    x86::reg32 scale = 0;
    if (1.0 > alpha)
    {
        scaled = true;
        scale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(
            x86::Float(M::mul(alpha, app->getMemory<double>(x86::reg32(0x53bcf8))))));
        word(ebp + 0x28) = scaleColour(word(ebp + 0x28), scale);
    }
    word(ebp - 0x50) = word(ebp + 0x28);

    // the forced test's clear OF, sahf's flags, and the scaling's cmp ecx, 0x10000
    compared.replay(cpu);
    compared.any = false;
    cpu.flags.of = false;
    if (scaled)
    {
        flags.compare(scale, 0x10000);
        flags.store(cpu);
    }
    cpu.eax = ebp - 0x2c;
    cpu.edx = 0xa0;
    cpu.ebp = ebp;
    cpu.esp = ebp - 0x60;
    if (!call(app, cpu, 0x4bbde0))
        return;
    const x86::reg32 polygon = word(ebp - 0x2c);
    if (cpu.eax != 0)
    {
        const x86::reg32 corners[4] = { polygon + 0x20, polygon + 0x40, polygon + 0x60, polygon + 0x80 };
        const double slope = M::div(x, floatAt(ebp - 0x38));
        const double k1i = M::mul(floatAt(0x56009c), asFloat(lightInverse));
        const double k2i = M::mul(asFloat(lightInverse), floatAt(0x5600a0));
        const float width = float(M::mul(k1i, size));
        const float height = float(M::mul(size, k2i));
        app->getMemory<x86::reg16>(polygon + 4) = 0;
        word(polygon) = 0;
        app->getMemory<x86::reg16>(polygon + 6) = 1;
        word(polygon + 0x18) = 0x8b4840;
        for (x86::reg32 c = 0; c < 4; ++c)
            word(polygon + 8 + c * 4) = corners[c];
        word(polygon + 0x1c) = word(record + 8);
        const float slopeFloat = float(slope);
        for (int c = 3; c >= 0; --c)
            std::memmove(&app->getMemory<x86::reg8>(corners[c]), &app->getMemory<x86::reg8>(record), 0x20);
        const float angle = float(M::mul(M::add(1.0, slopeFloat), app->getMemory<float>(x86::reg32(0x53bd00))));
        word(ebp - 8) = asBits(angle);
        word(ebp - 0x24) = asBits(width);
        word(ebp - 0x20) = asBits(height);
        const x86::Float turned = cpu.fpu.mul(x86::Float(angle), x86::Float(app->getMemory<double>(x86::reg32(0x54b6b0))));
        const float sw = float(M::mul(double(cpu.fpu.sin(turned)), width));
        const x86::Float turnedToo = cpu.fpu.mul(x86::Float(angle), x86::Float(app->getMemory<double>(x86::reg32(0x54b6b8))));
        const double ch = M::mul(double(cpu.fpu.cos(turnedToo)), height);
        word(ebp - 0x10) = asBits(sw);
        auto move = [&](x86::reg32 corner, double dx, double dy) {
            const double cx = floatAt(corner), cy = floatAt(corner + 4);
            app->getMemory<float>(corner) = float(M::add(cx, dx));
            app->getMemory<float>(corner + 4) = float(M::add(cy, dy));
        };
        // v0: x - sw, y + ch; v1: x + ch, y + sw; v2: x + sw, y - ch; v3: x - ch, y - sw
        {
            const double cx = floatAt(corners[0]), cy = floatAt(corners[0] + 4);
            app->getMemory<float>(corners[0]) = float(M::sub(cx, sw));
            app->getMemory<float>(corners[0] + 4) = float(M::add(cy, ch));
        }
        move(corners[1], ch, sw);
        {
            const double cx = floatAt(corners[2]), cy = floatAt(corners[2] + 4);
            app->getMemory<float>(corners[2]) = float(M::add(cx, sw));
            app->getMemory<float>(corners[2] + 4) = float(M::sub(cy, ch));
        }
        {
            const double cx = floatAt(corners[3]), cy = floatAt(corners[3] + 4);
            app->getMemory<float>(corners[3]) = float(M::sub(cx, ch));
            app->getMemory<float>(corners[3] + 4) = float(M::sub(cy, sw));
        }
        IntegerFlags codeFlags;
        for (int c = 0; c < 4; ++c)
            app->getMemory<x86::reg8>(corners[c] + 0x14) = screenCode(app, corners[c], edx, codeFlags);
        codeFlags.store(cpu);
    }
    else
    {
        flags.logic(0);  // test eax, eax
        flags.store(cpu);
    }
    word(cpu.esp - 4) = word(0x7dcffc);
    cpu.esp -= 4;
    cpu.eax = polygon;
    cpu.esp -= 4;
    if (!sortedBuckets(app, cpu))
        app->dynamic_call(0x4c7610, cpu);
    if (cpu.terminate)
        return;
    finish(cpu.eax);
}

/* st(0) toward zero, then to an integer as fistp stores it (sub_4dfd56). */
inline x86::sreg32 truncated(x86::CPU& cpu, double value)
{
    const auto rc = cpu.fpu.control.rc;
    cpu.fpu.control.rc = 3;
    const x86::Float whole = cpu.fpu.rndint(x86::Float(value));
    cpu.fpu.control.rc = rc;
    return cpu.fpu.toInteger<x86::sreg32>(whole);
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
 * tools/apply_alpha_intensity.py); a quad all off one side of the screen is
 * left out, the rest hung on a list, last first, each keyed by its deepest
 * corner.  Only eax is not kept. */
void headlightBeamNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 ebp = esp - 24 - 0x5a;
    const x86::reg32 view = cpu.eax;
    const x86::reg32 args = esp + 4;
    const float lamp[3] = { floatAt(args), floatAt(args + 4), floatAt(args + 8) };
    const x86::reg32 carMatrix = word(args + 0xc);
    x86::reg32 colour = word(args + 0x10);
    const x86::reg32 lengthBits = word(args + 0x14);
    const float farRadius = floatAt(args + 0x18);
    const float nearRadius = floatAt(args + 0x1c);
    const float textureV = floatAt(args + 0x20);
    const float depthBias = floatAt(args + 0x24);
    LastWideCompare compared;
    IntegerFlags flags;
    auto finish = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = esp + 4 + 0x28;
    };
    word(ebp + 0x1a) = 0;
    word(ebp - 0x32) = 0;
    flags.compare(word(0x6fbc30), 2);
    if (word(0x6fbc30) == 2)
    {
        flags.store(cpu);
        finish(view);
        return;
    }
    flags.logic(lengthBits & 0x7fffffff);
    if (!(lengthBits & 0x7fffffff))
    {
        flags.store(cpu);
        finish(view);
        return;
    }
    float d[3] = { float(M::sub(floatAt(view + 8), lamp[0])), float(M::sub(floatAt(view + 0xc), lamp[1])),
                   float(M::sub(floatAt(view + 0x10), lamp[2])) };
    const float distance = float(M::sqrt(M::add(M::add(M::mul(d[1], d[1]), M::mul(d[0], d[0])), M::mul(d[2], d[2]))));
    flags.logic(asBits(distance) & 0x7fffffff);
    if (!(asBits(distance) & 0x7fffffff))
    {
        flags.store(cpu);
        finish(ebp - 0x46);
        return;
    }
    const double reach = floatAt(0x53bd04);
    compared(distance, reach);
    if (distance > reach)
    {
        compared.replay(cpu);
        cpu.flags.of = false;
        finish(((ebp - 0x46) & 0xffff0000) | cpu.fpu.status.word);
        return;
    }
    {
        const double inverse = M::div(1.0, distance);
        d[0] = float(M::mul(d[0], inverse));
        d[1] = float(M::mul(d[1], inverse));
        d[2] = float(M::mul(inverse, d[2]));
    }
    auto dot = [&](x86::reg32 w) {
        const float* v = &app->getMemory<float>(w);
        return float(M::add(M::add(M::mul(d[0], v[0]), M::mul(d[1], v[1])), M::mul(d[2], v[2])));
    };
    const float c1 = dot(carMatrix + 0x18);
    const float c2 = dot(carMatrix);
    const float c3 = dot(carMatrix + 0xc);
    const double k8 = floatAt(0x53bd08);
    const double segments = M::mul(M::sub(1.0, M::mul(distance, floatAt(0x53bd10))), floatAt(0x53bd14));
    const float start = float(M::mul(k8, M::sub(M::sub(floatAt(0x53bd0c), M::mul(c2, k8)), M::mul(c3, k8))));
    const x86::sreg32 raw = truncated(cpu, segments);
    const x86::sreg32 n = raw > 12 ? 12 : (raw >= 3 ? raw : 3);
    const x86::sreg32 count = n + 1;

    // sub_4ea9c0: the half angle of the cone
    double angle;
    if (app->getMemory<x86::reg8>(0x567858) & 1)
    {
        word(cpu.esp - 4) = lengthBits;
        word(cpu.esp - 8) = asBits(nearRadius);
        cpu.esp -= 8;
        if (!call(app, cpu, 0x4ea9c0))
            return;
        angle = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        cpu.esp = esp;
    }
    else
    {
        angle = M::mul(double(cpu.fpu.atan(x86::Float(asFloat(lengthBits)), x86::Float(nearRadius))),
                       app->getMemory<double>(x86::reg32(0x54b6c0)));
    }
    const float spread = float(M::mul(angle, floatAt(0x53bd18)));
    float width;
    compared(1.0, spread);
    bool small = false;
    if (!(1.0 < spread || spread != spread))
    {
        compared(floatAt(0x53bd1c), spread);
        small = floatAt(0x53bd1c) > spread;
    }
    if (small)
        width = asFloat(0x3d4ccccd);
    else
    {
        compared(1.0, spread);
        width = 1.0 >= spread ? spread : 1.0f;
    }
    compared(M::sub(1.0, width), c1);
    if (!(M::sub(1.0, width) >= c1))
    {
        const double into = M::sub(c1, M::sub(1.0, width));
        const double fade = M::sub(1.0, M::mul(M::div(1.0, width), into));
        const x86::reg32 a = x86::reg32(truncated(cpu, M::mul(double((colour >> 24) & 0xff), fade))) & 0xff;
        const x86::reg32 r = x86::reg32(truncated(cpu, M::mul(double((colour >> 16) & 0xff), fade))) & 0xff;
        const x86::reg32 b = x86::reg32(truncated(cpu, M::mul(double(colour & 0xff), fade))) & 0xff;
        const x86::reg32 g = x86::reg32(truncated(cpu, M::mul(fade, double((colour >> 8) & 0xff)))) & 0xff;
        colour = a << 24 | r << 16 | g << 8 | b;
    }
    // [0x7a3a58] & 0x40 forced on (tools/apply_alpha_intensity.py)
    const double alpha = floatAt(0x6fbc38);
    compared(1.0, alpha);
    if (1.0 > alpha)
        colour = scaleColour(colour, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(
                                         x86::Float(M::mul(alpha, app->getMemory<double>(x86::reg32(0x53bd20)))))));
    const float step = float(M::div(1.0, double(n)));

    // The two rings: points round the beam, to the car, the world, the view, the screen.
    auto ring = [&](x86::reg32 records, float radius, bool lit) {
        const float* m = &app->getMemory<float>(carMatrix);
        const float* v = &app->getMemory<float>(view + 0x44);
        const float camera[3] = { floatAt(view + 0x38), floatAt(view + 0x3c), floatAt(view + 0x40) };
        float a = start;
        for (x86::sreg32 i = 0; i < count; ++i)
        {
            const double c = M::mul(double(cpu.fpu.cos(cpu.fpu.mul(x86::Float(a),
                                                                   x86::Float(app->getMemory<double>(x86::reg32(0x54b6b8)))))),
                                    radius);
            const double s = M::mul(double(cpu.fpu.sin(cpu.fpu.mul(x86::Float(a),
                                                                   x86::Float(app->getMemory<double>(x86::reg32(0x54b6b0)))))),
                                    radius);
            const float p[3] = { float(c), float(s), lit ? 0.0f : asFloat(lengthBits) };
            a = float(M::add(a, step));
            float q[3];
            for (int j = 0; j < 3; ++j)
                q[j] = float(M::add(M::add(M::mul(p[0], m[j]), M::mul(p[1], m[3 + j])), M::mul(p[2], m[6 + j])));
            for (int j = 0; j < 3; ++j)
                q[j] = float(M::add(q[j], lamp[j]));
            float w[3];
            for (int j = 0; j < 3; ++j)
                w[j] = float(M::add(M::add(M::mul(q[0], v[j]), M::mul(q[1], v[3 + j])), M::mul(q[2], v[6 + j])));
            for (int j = 0; j < 3; ++j)
                w[j] = float(M::add(camera[j], w[j]));
            const x86::reg32 record = records + x86::reg32(i) * 0x20;
            projectToScreen(app, record, w[0], w[1], w[2]);
            x86::reg32 edx = 0;
            app->getMemory<x86::reg8>(record + 0x14) = screenCode(app, record, edx);
            word(record + 0x10) = lit ? colour : 0;
            app->getMemory<float>(record + 0x18) = float(M::div(double(i), double(n)));
            app->getMemory<float>(record + 0x1c) = lit ? float(M::sub(1.0, textureV)) : -textureV;
        }
    };
    const x86::reg32 nearRing = ebp - 0x3ba;
    const x86::reg32 farRing = ebp - 0x55a;
    ring(nearRing, nearRadius, false);
    ring(farRing, farRadius, true);

    x86::reg32 head = 0;
    cpu.ebp = ebp;
    for (x86::sreg32 j = 0; j < n; ++j)
    {
        compared.replay(cpu);
        compared.any = false;
        cpu.eax = ebp - 0x32;
        cpu.edx = 0xa0;
        cpu.esp = ebp + 0x5a - 0x5b4;
        if (!call(app, cpu, 0x4bbde0))
            return;
        if (cpu.eax == 0)
        {
            flags.logic(0);  // test eax, eax
            flags.store(cpu);
            finish(0);
            return;
        }
        const x86::reg32 polygon = word(ebp - 0x32);
        const x86::reg32 sources[4] = { nearRing + x86::reg32(j + 1) * 0x20, nearRing + x86::reg32(j) * 0x20,
                                        farRing + x86::reg32(j) * 0x20, farRing + x86::reg32(j + 1) * 0x20 };
        const x86::reg8 shared = app->getMemory<x86::reg8>(sources[0] + 0x14) & app->getMemory<x86::reg8>(sources[1] + 0x14)
                                 & app->getMemory<x86::reg8>(sources[2] + 0x14) & app->getMemory<x86::reg8>(sources[3] + 0x14);
        if (shared)
        {
            word(ebp - 0x32) = head;
            continue;
        }
        app->getMemory<x86::reg16>(polygon + 4) = 0;
        word(polygon) = head;
        app->getMemory<x86::reg16>(polygon + 6) = 3;
        word(polygon + 0x18) = 0x8b47e8;
        for (x86::reg32 c = 0; c < 4; ++c)
        {
            word(polygon + 8 + c * 4) = polygon + 0x20 + c * 0x20;
            std::memmove(&app->getMemory<x86::reg8>(polygon + 0x20 + c * 0x20), &app->getMemory<x86::reg8>(sources[c]),
                         0x20);
        }
        const double z0 = floatAt(polygon + 0x28), z1 = floatAt(polygon + 0x48);
        const double z2 = floatAt(polygon + 0x68), z3 = floatAt(polygon + 0x88);
        auto max23 = [&]() {
            compared(z2, z3);
            return z2 > z3 ? z2 : z3;
        };
        double deepest;
        double b;
        const double a = max23();
        compared(z1, a);
        if (z1 > a)
            b = z1;
        else
            b = max23();
        compared(z0, b);
        if (z0 > b)
            deepest = z0;
        else
        {
            const double a2 = max23();
            compared(z1, a2);
            deepest = z1 > a2 ? z1 : max23();
        }
        app->getMemory<float>(polygon + 0x1c) = float(M::add(deepest, depthBias));
        head = polygon;
    }
    compared.replay(cpu);
    word(cpu.esp - 4) = word(0x7dcffc);
    cpu.esp -= 4;
    cpu.eax = word(ebp - 0x32);
    cpu.esp -= 4;
    if (!sortedBuckets(app, cpu))
        app->dynamic_call(0x4c7610, cpu);
    if (cpu.terminate)
        return;
    finish(cpu.eax);
}

float viewZoom(win32::WinApplication* app, x86::reg32 view);
void signedDivide(x86::sreg64 dividend, x86::sreg32 divisor, x86::reg32& quotient, x86::reg32& remainder);

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
 * zoom), or -1 past the last.  The registers it does not keep, the flags and
 * the FPU status as the original leaves them. */
void carDetailNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 ebp = esp - 16 - 0x82;
    const x86::reg32 view = cpu.eax;
    const x86::reg32 car = cpu.edx;
    const x86::reg32 out = cpu.ebx;
    LastWideCompare compared;
    IntegerFlags flags;
    auto finish = [&](x86::reg32 eax, x86::reg32 ebx, x86::reg32 edx) {
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.edx = edx;
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = esp + 4;
    };
    const float zoom = viewZoom(app, view);
    const x86::reg8 shown = byte(car + 0x89);
    x86::reg32 edx = (car & 0xffffff00) | shown;
    if (!shown)
    {
        logic8(cpu, 0);  // test dl, dl
        finish(0xffffffff, out, edx);
        return;
    }
    if (byte(car + 0x88) != 0)
    {
        // cmp byte ptr [eax+0x88], 0
        cpu.flags.cf = false;
        cpu.flags.of = false;
        cpu.flags.zf = false;
        cpu.flags.sf = (byte(car + 0x88) & 0x80) != 0;
        finish(0xffffffff, out, edx);
        return;
    }
    cpu.ebp = ebp;
    cpu.esp = ebp + 0x82 - 0xd8;
    cpu.ecx = view;
    cpu.ebx = out;
    cpu.eax = word(view + 4);
    cpu.edx = car;
    if (!call(app, cpu, 0x422ba0))
        return;
    if (cpu.eax == car)
    {
        cpu.eax = word(view + 4);
        if (!call(app, cpu, 0x423b00))
            return;
        const x86::reg32 mode = cpu.eax;
        flags.compare(mode, 1);
        bool skip = mode == 1;
        if (!skip)
        {
            flags.compare(word(view), 1);
            skip = word(view) == 1;
        }
        if (!skip)
        {
            flags.compare(mode, 2);
            skip = mode == 2;
        }
        if (skip)
        {
            flags.store(cpu);
            finish(0xffffffff, cpu.ebx, cpu.edx);
            return;
        }
    }
    cpu.edx = x86::reg32(x86::sreg32(word(car + 0x1c)) >> 3);
    cpu.eax = view;
    if (!call(app, cpu, 0x41e070))
        return;
    if (cpu.eax == 0)
    {
        flags.logic(0);  // test eax, eax
        flags.store(cpu);
        finish(0xffffffff, cpu.ebx, cpu.edx);
        return;
    }
    word(out) = 0;
    const x86::reg32 row = 0x55fdf4 + (word(0x6fbc1c) << 4);
    float t[4] = { floatAt(row), floatAt(row + 4), floatAt(row + 8), floatAt(row + 0xc) };
    if (word(0x6fd3b0) == 1 || word(0x6fd4c8) != 0)
    {
        const double k = app->getMemory<double>(x86::reg32(0x540130));
        t[1] = float(M::mul(t[1], k));
        t[2] = float(M::mul(k, t[2]));
        t[0] = float(M::mul(t[0], k));
    }
    if (word(view) == 1)
    {
        const double k = mirrorCarScale(app->getMemory<double>(x86::reg32(0x540138)));
        for (int i = 0; i < 3; ++i)
            t[i] = float(M::mul(t[i], k));
        t[3] = float(M::mul(k, t[3]));
    }
    if (asBits(zoom) != 0x3f800000)
    {
        for (int i = 0; i < 3; ++i)
            t[i] = float(M::mul(t[i], zoom));
        t[3] = float(M::mul(zoom, t[3]));
    }
    const float e0 = floatAt(car + 0x114), e1 = floatAt(car + 0x118), e2 = floatAt(car + 0x11c);
    const float corners[8][3] = { { -e0, e1, e2 },  { e0, e1, e2 },  { e0, e1, -e2 },  { -e0, e1, -e2 },
                                  { -e0, -e1, e2 }, { e0, -e1, e2 }, { e0, -e1, -e2 }, { -e0, -e1, -e2 } };
    // the car's place in the view (sub_4e0430) and its matrix turned by the view's (sub_4e0280)
    const float* v = &app->getMemory<float>(view + 0x44);
    const float* position = &app->getMemory<float>(car + 0x98);
    float place[3];
    for (int j = 0; j < 3; ++j)
        place[j] = float(M::add(M::add(M::mul(position[0], v[j]), M::mul(position[1], v[3 + j])), M::mul(position[2], v[6 + j])));
    for (int j = 0; j < 3; ++j)
        place[j] = float(M::add(place[j], floatAt(view + 0x38 + x86::reg32(j) * 4)));
    const float* turn = &app->getMemory<float>(car + 0xc0);
    float r[9];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i * 3 + j] = float(M::add(M::add(M::mul(turn[i * 3], v[j]), M::mul(turn[i * 3 + 1], v[3 + j])),
                                        M::mul(turn[i * 3 + 2], v[6 + j])));
    float sum = 0.0f;
    x86::reg32 code = 0;
    for (int i = 0; i < 8; ++i)
    {
        float p[3];
        for (int j = 0; j < 3; ++j)
            p[j] = float(M::add(M::add(M::mul(corners[i][0], r[j]), M::mul(corners[i][1], r[3 + j])),
                                M::mul(corners[i][2], r[6 + j])));
        for (int j = 0; j < 3; ++j)
            p[j] = float(M::add(p[j], place[j]));
        const x86::reg32 record = 0x7a3bf0 + x86::reg32(i) * 0x20;
        projectToScreen(app, record, p[0], p[1], p[2]);
        x86::reg32 unused = 0;
        code = screenCode(app, record, unused);
        byte(record + 0x14) = x86::reg8(code);
        sum = float(M::add(sum, p[2]));
    }
    const x86::reg32 near = byte(0x7a3c04) & byte(0x7a3c24) & byte(0x7a3c44);
    flags.logic(near & byte(0x7a3c64));
    if (near & byte(0x7a3c64))
    {
        const x86::reg32 far = byte(0x7a3c84) & byte(0x7a3ca4) & byte(0x7a3cc4);
        flags.logic(far & byte(0x7a3ce4));
        if (far & byte(0x7a3ce4))
        {
            flags.store(cpu);
            finish(0xffffffff, code, far);
            return;
        }
    }
    const float mean = float(M::mul(sum, app->getMemory<double>(x86::reg32(0x540140))));
    word(out) = asBits(mean);
    x86::reg32 level = 0xffffffff;
    bool decided = false;
    bool first = false;
    if (asBits(t[0]) & 0x7fffffff)
    {
        compared(mean, t[0]);
        if (!(mean >= t[0]))
        {
            level = 0;
            decided = true;
            first = true;
        }
    }
    for (int i = 1; i < 4 && !decided; ++i)
    {
        compared(mean, t[i]);
        if (!(mean >= t[i]))
        {
            level = x86::reg32(i);
            decided = true;
        }
    }
    compared.replay(cpu);
    if (first)
    {
        flags.logic(0);  // xor eax, eax
        flags.store(cpu);
    }
    else
    {
        // sahf's, with OF clear from test ecx, 0x7fffffff (or test edx, eax before it)
        cpu.flags.of = false;
    }
    finish(level, code, out);
}

/* sub_4dbef0: an object's vertices to the screen with their colours.
 *   eax  the matrix; edx  the move; ebx  the count; ecx  the vertices in,
 *   12 bytes each; [esp+4]  the colours; [esp+8]  the screen records out,
 *   0x20 bytes each (ret 8)
 * More than 20 go to sub_4bf4c0 (the stand-in objectVertices) at once, fewer
 * one by one, turned, moved and projected inline; each record then takes its
 * colour at +0x10.  esi, edi and ebp are kept, the rest as the original leaves
 * them; the two pointers it moves on in its arguments are left, past the return. */
void objectVerticesListNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 ebp = esp - 12;
    const x86::reg32 matrix = cpu.eax, move = cpu.edx, vertices = cpu.ecx;
    const x86::reg32 count = cpu.ebx;
    x86::reg32 colours = word(esp + 4);
    x86::reg32 out = word(esp + 8);
    IntegerFlags flags;
    auto finish = [&]() {
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = esp + 4 + 8;
    };
    if (x86::sreg32(count) > 0x14)
    {
        word(ebp - 0x24) = out;  // push ecx: sub_4bf4c0's argument
        cpu.eax = count;
        cpu.ebx = matrix;
        cpu.ecx = move;
        cpu.edx = vertices;
        cpu.ebp = ebp;
        cpu.esp = ebp - 0x24;
        cpu.esp -= 4;
        if (!objectVertices(app, cpu))
            app->dynamic_call(0x4bf4c0, cpu);
        if (cpu.terminate)
            return;
        x86::reg32 left = count;
        x86::reg32 eax = cpu.eax, ecx = cpu.ecx;
        while (true)
        {
            --left;
            if (left == 0xffffffff)
                break;
            eax = word(colours);
            colours += 4;
            word(out + 0x10) = eax;
            ecx = out;
            out += 0x20;
            eax = out;
        }
        flags.compare(0xffffffff, 0xffffffff);  // cmp ebx, -1
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = 0xffffffff;
        cpu.ecx = ecx;
        finish();
        return;
    }
    x86::reg32 left = count;
    x86::reg32 eax = matrix, ebx = count, ecx = vertices;
    x86::reg32 in = vertices;
    const float* m = &app->getMemory<float>(matrix);
    const float* t = &app->getMemory<float>(move);
    while (true)
    {
        --left;
        if (left == 0xffffffff)
            break;
        const float* v = &app->getMemory<float>(in);
        float p[3];
        for (int j = 0; j < 3; ++j)
            p[j] = float(M::add(M::add(M::mul(v[0], m[j]), M::mul(v[1], m[3 + j])), M::mul(v[2], m[6 + j])));
        for (int j = 0; j < 3; ++j)
            p[j] = float(M::add(p[j], t[j]));
        projectToScreen(app, out, p[0], p[1], p[2]);
        x86::reg32 unused = 0;
        ebx = screenCode(app, out, unused);
        app->getMemory<x86::reg8>(out + 0x14) = x86::reg8(ebx);
        word(out + 0x10) = word(colours);
        colours += 4;
        ecx = out;
        out += 0x20;
        in += 0xc;
        eax = in;
    }
    flags.compare(0xffffffff, 0xffffffff);  // cmp edx, -1
    flags.store(cpu);
    cpu.eax = eax;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = 0xffffffff;
    finish();
}

/* sub_41b640: the polygons of a piece of an object, four corners each, as
 * records for the frame's polygon list -- as sub_41b300 does for the track,
 * written one after another at ecx, with the corners taken the other way round.
 *   ebx  the polygons, 14 bytes each; edx  their count; ecx  the records out;
 *   [esp+4]  the vertex records, 0x20 bytes each (ret 4)
 * Returns how many records were written.  esi, edi and ebp are kept; the
 * rest, the flags and the FPU status as the original leaves them. */
void objectRecordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto half = [app](x86::reg32 address) { return app->getMemory<x86::reg16>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    auto floatAt = [app](x86::reg32 address) -> float { return app->getMemory<float>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    const x86::reg32 base = word(esp + 4);
    const x86::sreg32 count = x86::sreg32(cpu.edx);
    x86::reg32 polygon = cpu.ebx;
    x86::reg32 record = cpu.ecx;
    x86::reg32 edx = 0;
    x86::reg32 written = 0;
    LastWideCompare compared;
    const double farthest = floatAt(word(0x5dd830) + 0x30);
    const x86::reg32 textures = word(0x552df4);
    x86::sreg32 i = 0;
    for (; i < count; ++i)
    {
        const x86::reg32 v0 = base + (x86::reg32(x86::sreg32(word(polygon)) >> 16) << 5);
        const x86::reg32 v1 = base + (x86::reg32(x86::sreg32(x86::sreg16(half(polygon)))) << 5);
        const x86::reg32 v2 = base + (x86::reg32(x86::sreg32(word(polygon + 4)) >> 16) << 5);
        const x86::reg32 v3 = base + (x86::reg32(x86::sreg32(word(polygon + 2)) >> 16) << 5);
        const x86::reg8 flags8 = byte(polygon + 0xc);
        const x86::reg32 c0 = byte(v0 + 0x14), c1 = byte(v1 + 0x14), c2 = byte(v2 + 0x14), c3 = byte(v3 + 0x14);
        edx = c3;
        if (c0 & c1 & c2 & c3)
        {
            polygon += 0xe;
            continue;
        }
        const double z0 = floatAt(v0 + 8);
        compared(z0, farthest);
        if (z0 > farthest)
        {
            polygon += 0xe;
            continue;
        }
        if (!(flags8 & 0x10) && !((c0 | c1 | c2 | c3) & 0x10))
        {
            const CrossProducts p = crossProducts(floatAt(v2 + 4), floatAt(v1 + 4), floatAt(v0), floatAt(v1),
                                                  floatAt(v0 + 4), floatAt(v1 + 4), floatAt(v2), floatAt(v1));
            compared(p.two, p.one);
            const x86::reg32 back = (p.two < p.one || p.two != p.two || p.one != p.one) ? 1 : 0;
            if (back ^ word(0x554e4c))
            {
                const CrossProducts q = crossProducts(floatAt(v3 + 4), floatAt(v2 + 4), floatAt(v0), floatAt(v2),
                                                      floatAt(v0 + 4), floatAt(v2 + 4), floatAt(v3), floatAt(v2));
                compared(q.two, q.one);
                const x86::reg32 backToo = (q.two < q.one || q.two != q.two || q.one != q.one) ? 1 : 0;
                if (backToo ^ word(0x554e4c))
                {
                    polygon += 0xe;
                    continue;
                }
            }
        }
        word(record + 0x10) = v2;
        word(record + 8) = v0;
        word(record + 0xc) = v1;
        word(record + 0x14) = v3;
        word(record) = record + 0x20;
        x86::reg32 texture = textures + x86::reg32(x86::sreg32(word(polygon + 6)) >> 16) * 47;
        app->getMemory<x86::reg16>(record + 6) = 0;
        word(record + 0x18) = texture;
        edx = (texture & 0xffffff00) | flags8;
        app->getMemory<x86::reg16>(record + 4) = 0;
        if (flags8 & 1)
            byte(record + 6) = x86::reg8(byte(record + 6) | 4);
        auto depthSum = [&]() {
            return M::mul(M::add(M::add(M::add(floatAt(v0 + 8), floatAt(v1 + 8)), floatAt(v2 + 8)), floatAt(v3 + 8)),
                          app->getMemory<double>(x86::reg32(0x536c9c)));
        };
        if ((byte(0x7a3a58) & 6) && word(0x6fbc48) != 0)
        {
            const double mean = depthSum();
            const double near = app->getMemory<double>(x86::reg32(0x536ca4));
            compared(mean, near);
            if (!(mean >= near))
                byte(record + 6) = x86::reg8(byte(record + 6) | 8);
        }
        if (flags8 & 4)
        {
            const x86::reg8 animation = byte(polygon + 0xd);
            x86::reg32 frames, step, unused, phase;
            signedDivide(x86::sreg32(word(0x7d3684)), x86::sreg32(animation >> 3), frames, unused);
            signedDivide(x86::sreg32(frames), x86::sreg32(animation & 7), step, phase);
            NFS2_USE(step);
            texture = textures + (phase + x86::reg32(x86::sreg32(word(polygon + 8)) >> 16)) * 47;
            word(record + 0x18) = texture;
            edx = texture;
        }
        if (byte(word(record + 0x18) + 0x28) & 2)
        {
            const x86::reg32 colour = word(0x552e1c);
            word(v0 + 0x10) = colour;
            word(v1 + 0x10) = colour;
            word(v2 + 0x10) = colour;
            word(v3 + 0x10) = colour;
            byte(record + 6) = x86::reg8(byte(record + 6) | 1);
            app->getMemory<float>(record + 0x1c) = float(depthSum());
            edx = v3;
        }
        polygon += 0xe;
        record += 0x20;
        ++written;
    }
    compared.replay(cpu);
    IntegerFlags flags;
    flags.compare(x86::reg32(i), x86::reg32(count));  // cmp eax, [ebp-0x2c]
    flags.store(cpu);
    cpu.eax = written;
    cpu.ebx = polygon;
    cpu.ecx = record;
    cpu.edx = edx;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = esp + 4 + 4;
}

/* The game's small vector helpers (Watcom-style register calls, used from
 * dozens of places): each element as the x87 at single precision has it. */

/* The flags dec leaves after a loop counted down to zero, CF from the add
 * before it. */
inline void countedDown(x86::CPU& cpu, x86::reg32 added, x86::reg32 before)
{
    IntegerFlags flags;
    flags.compare(0, 0);
    flags.cf = added < before;
    flags.store(cpu);
}

/* sub_4e0050: count vectors at edx times the float [esp+4] into [esp+8]
 * (ret 8). */
void scaleVectorsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const x86::reg32 esp = cpu.esp;
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    const double scale = app->getMemory<float>(esp + 4);
    x86::reg32 out = app->getMemory<x86::reg32>(esp + 8);
    x86::reg32 in = cpu.edx;
    if (count <= 0)
    {
        IntegerFlags flags;
        flags.logic(cpu.eax);
        flags.store(cpu);
        cpu.eax = out;
        cpu.esp = esp + 4 + 8;
        return;
    }
    for (x86::sreg32 i = 0; i < count; ++i)
    {
        const float* v = &app->getMemory<float>(in);
        const float x = v[0], y = v[1], z = v[2];
        float* o = &app->getMemory<float>(out);
        o[0] = float(M::mul(x, scale));
        o[1] = float(M::mul(y, scale));
        o[2] = float(M::mul(z, scale));
        out += 0xc;
        in += 0xc;
    }
    countedDown(cpu, in, in - 0xc);
    cpu.eax = out;
    cpu.edx = in;
    cpu.esp = esp + 4 + 8;
}

/* sub_4dffb0 (add) and sub_4e0000 (subtract): count vectors at edx, each with
 * the one vector at ebx, into ecx. */
template <bool Add>
void offsetVectorsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const x86::sreg32 count = x86::sreg32(cpu.eax);
    x86::reg32 in = cpu.edx;
    const x86::reg32 by = cpu.ebx;
    x86::reg32 out = cpu.ecx;
    if (count <= 0)
    {
        IntegerFlags flags;
        flags.logic(cpu.eax);  // test esi, esi
        flags.store(cpu);
        cpu.eax = in;
        cpu.esp += 4;
        return;
    }
    x86::reg32 last = out;
    for (x86::sreg32 i = 0; i < count; ++i)
    {
        const float* a = &app->getMemory<float>(in);
        const float* b = &app->getMemory<float>(by);
        const float a0 = a[0], a1 = a[1], a2 = a[2];
        const float b0 = b[0], b1 = b[1], b2 = b[2];
        float* o = &app->getMemory<float>(out);
        o[0] = float(Add ? M::add(a0, b0) : M::sub(a0, b0));
        o[1] = float(Add ? M::add(a1, b1) : M::sub(a1, b1));
        o[2] = float(Add ? M::add(a2, b2) : M::sub(a2, b2));
        last = out;
        in += 0xc;
        out += 0xc;
    }
    countedDown(cpu, out, out - 0xc);
    cpu.eax = in;
    cpu.ebx = last;
    cpu.ecx = out;
    cpu.edx = by;
    cpu.esp += 4;
}

/* sub_4e06a0: the length of the vector at eax, pushed onto the x87 stack. */
inline double vectorLength(win32::WinApplication* app, x86::reg32 at)
{
    typedef NearestMath M;
    const float* v = &app->getMemory<float>(at);
    return M::sqrt(M::add(M::add(M::mul(v[1], v[1]), M::mul(v[0], v[0])), M::mul(v[2], v[2])));
}

void vectorLengthNative(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(vectorLength(app, cpu.eax));
    cpu.esp += 4;
}

/* sub_4e01c0: the vector at eax divided by its length into edx (a zero one
 * as the x87 divides it). */
void unitVectorNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    const double inverse = M::div(1.0, vectorLength(app, cpu.eax));
    const float* v = &app->getMemory<float>(cpu.eax);
    const float x = v[0], y = v[1], z = v[2];
    float* o = &app->getMemory<float>(cpu.edx);
    o[0] = float(M::mul(x, inverse));
    o[1] = float(M::mul(y, inverse));
    o[2] = float(M::mul(inverse, z));
    cpu.esp += 4;
}

/* The view's zoom (sub_4227e0): the camera record's +0x54 for the main view
 * when it is set, else 1. */
float viewZoom(win32::WinApplication* app, x86::reg32 view)
{
    if (app->getMemory<x86::reg32>(view) != 0)
        return 1.0f;
    const x86::reg32 index = app->getMemory<x86::reg32>(view + 4) * 4;
    x86::reg32 camera = app->getMemory<x86::reg32>(0x5e10b8 + index);
    if (!camera)
        camera = app->getMemory<x86::reg32>(0x5e10b0 + index);
    const x86::reg32 zoom = app->getMemory<x86::reg32>(camera + 0x54);
    return (zoom & 0x7fffffff) ? asFloat(zoom) : 1.0f;
}

/* sub_41bf30: whether a box -- four corners -- lies wholly outside the view.
 *   eax  the view; edx  the four corners, 12 bytes each; ebx  a 3x3 matrix;
 *   ecx  a move
 * Each corner is turned and moved (as sub_4e0430 does it), its 1/z and screen
 * position worked out from [0x56009c..a8], and its clip code from the screen
 * bounds at [0x7d34f8..0x7d350c], compared as integers; then x becomes |x|
 * times the zoom, held to 1.  Returns 1 when every corner has |x| beyond its z
 * and the four share a clip bit, else 0.  Only its stack is written.  The
 * registers it does not keep, the flags and the FPU status as the original
 * leaves them. */
void boxOutsideNative(win32::WinApplication* app, x86::CPU& cpu)
{
    typedef NearestMath M;
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 corners = cpu.edx;
    const x86::reg32 matrix = cpu.ebx;
    const x86::reg32 move = cpu.ecx;
    LastWideCompare compared;

    const double zoom = viewZoom(app, cpu.eax);
    compared(1.0, zoom);
    const double scale = 1.0 >= zoom ? zoom : 1.0;

    const double k1 = app->getMemory<float>(x86::reg32(0x56009c));
    const double k2 = app->getMemory<float>(x86::reg32(0x5600a0));
    const double c1 = app->getMemory<float>(x86::reg32(0x5600a4));
    const double c2 = app->getMemory<float>(x86::reg32(0x5600a8));
    const x86::sreg32 nearest = x86::sreg32(word(0x7d34f8));
    const x86::sreg32 top = x86::sreg32(word(0x7d3504)), bottom = x86::sreg32(word(0x7d3500));
    const x86::sreg32 left = x86::sreg32(word(0x7d34fc)), right = x86::sreg32(word(0x7d350c));
    float xs[4], zs[4];
    x86::reg8 codes[4];
    x86::reg32 code = 0, edx = 0;
    for (int i = 0; i < 4; ++i)
    {
        const float* v = &app->getMemory<float>(corners + x86::reg32(i) * 12);
        const float v0 = v[0], v1 = v[1], v2 = v[2];
        const float* m = &app->getMemory<float>(matrix);
        float x = float(M::add(M::add(M::mul(v0, m[0]), M::mul(v1, m[3])), M::mul(v2, m[6])));
        float y = float(M::add(M::add(M::mul(v0, m[1]), M::mul(v1, m[4])), M::mul(v2, m[7])));
        float z = float(M::add(M::add(M::mul(v0, m[2]), M::mul(v1, m[5])), M::mul(v2, m[8])));
        const float* t = &app->getMemory<float>(move);
        x = float(M::add(t[0], x));
        y = float(M::add(t[1], y));
        z = float(M::add(t[2], z));
        if (!(asBits(z) & 0x7fffffff))
            z = asFloat(0x37800080);
        const float inverse = float(M::div(1.0, z));
        const float sx = float(M::add(M::mul(M::mul(k1, inverse), x), c1));
        const float sy = float(M::add(M::mul(M::mul(k2, inverse), y), c2));
        const x86::reg32 inverseBits = asBits(inverse);
        if ((inverseBits & 0x80000000) || x86::sreg32(inverseBits) >= nearest)
        {
            code = 0x10;
            edx = asBits(z);
        }
        else
        {
            code = 0;
            const x86::reg32 yb = asBits(sy);
            if ((yb & 0x80000000) || x86::sreg32(yb) < top)
                code |= 8;
            else if (x86::sreg32(yb) > bottom)
                code |= 4;
            const x86::reg32 xb = asBits(sx);
            if ((xb & 0x80000000) || x86::sreg32(xb) < left)
                code |= 1;
            else if (x86::sreg32(xb) > right)
                code |= 2;
            edx = x86::reg32(right);
        }
        codes[i] = x86::reg8(code);
        compared(0.0, x);
        const float size = (0.0 <= x || x != x) ? x : -x;
        xs[i] = float(M::mul(size, scale));
        zs[i] = z;
    }

    x86::reg32 eax = 0, ecx = 4;
    bool beyond = true;
    for (int i = 0; i < 4 && beyond; ++i)
    {
        compared(xs[i], zs[i]);
        beyond = xs[i] > zs[i];
    }
    compared.replay(cpu);
    cpu.flags.of = false;  // the loop's cmp [ebp+0x7e], 4 left it clear; sahf keeps it
    if (beyond)
    {
        const x86::reg32 shared = x86::reg32(codes[0] & codes[1] & codes[2]);
        ecx = codes[3];
        IntegerFlags flags;
        flags.logic(shared & ecx);  // test eax, ecx
        flags.store(cpu);
        eax = (shared & ecx) ? 1 : 0;
    }
    cpu.eax = eax;
    cpu.ebx = code;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp = esp + 4;
}

/* The brightness the projected light is scaled by (sub_41a120), applied to a
 * colour as sub_41a3e0 darkens one. */
x86::reg32 scaleColour(x86::reg32 colour, x86::reg32 scale)
{
    const x86::reg32 clamped = x86::sreg32(scale) >= 0x10000 ? 0xffff : scale;
    const x86::reg32 level = (clamped >> 8) & 0xff;
    const x86::reg32 redBlue = (colour & 0xff00ff) * level;
    const x86::reg32 alphaGreen = ((colour & 0xff00ff00) >> 8) * level;
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
 * test is forced on by tools/apply_alpha_intensity.py) -- and the slot moves on past it
 * unless the three colours' alpha is all below 8.  Returns 1 when it moved.
 * The registers and flags as the two leave them. */
void projectedTriangleNative(win32::WinApplication* app, x86::CPU& cpu)
{
    auto word = [app](x86::reg32 address) { return app->getMemory<x86::reg32>(address); };
    auto byte = [app](x86::reg32 address) { return app->getMemory<x86::reg8>(address); };
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 slot = cpu.eax;
    const x86::reg32 records[3] = { cpu.ecx, word(esp + 4), word(esp + 8) };
    const x86::reg8 ca = byte(records[0] + 4), cb = byte(records[1] + 4), cc = byte(records[2] + 4);
    auto finish = [&]() { cpu.esp = esp + 4 + 8; };

    const x86::reg8 shared = ca & cb & cc;
    if (shared)
    {
        logic8(cpu, shared);
        cpu.eax = 0;
        cpu.ecx = (records[1] & 0xffff0000) | x86::reg32(cc) << 8 | shared;
        finish();
        return;
    }
    const x86::reg8 any = ca | cb | cc;
    if (any & 0x20)
    {
        logic8(cpu, any & 0x20);
        cpu.eax = 0;
        cpu.ecx = (records[1] & 0xffff0000) | x86::reg32(cc) << 8 | any;
        finish();
        return;
    }

    // sub_41a120, with the records as copied to its stack
    cpu.ecx = 0;  // rep movsd; sub_41a120 keeps it
    x86::reg32 vertexOf[3], recordWords[3][7];
    for (int i = 0; i < 3; ++i)
    {
        for (int w = 0; w < 7; ++w)
            recordWords[i][w] = word(records[i] + x86::reg32(w) * 4);
        vertexOf[i] = recordWords[i][2];
    }
    const x86::reg32 polygon = word(slot);
    const x86::reg32 ab = x86::reg32(byte(vertexOf[1] + 0x14)) & x86::reg32(byte(vertexOf[0] + 0x14));
    const x86::reg32 c = byte(vertexOf[2] + 0x14);
    if (ab & c)
    {
        IntegerFlags flags;
        flags.logic(0);  // test edx, ebx; xor eax, eax
        flags.store(cpu);
        cpu.eax = 0;
        cpu.ebx = c;
        cpu.edx = ab;
        finish();
        return;
    }
    const x86::reg32 copies[3] = { polygon + 0x20, polygon + 0x40, polygon + 0x60 };
    app->getMemory<x86::reg16>(polygon + 4) = 1;
    word(polygon + 0x18) = 0x8b4814;
    app->getMemory<x86::reg16>(polygon + 6) = 0x13;
    word(polygon) = polygon + 0xa0;
    word(polygon + 8) = copies[0];
    word(polygon + 0xc) = copies[1];
    word(polygon + 0x10) = copies[2];
    for (int i = 0; i < 3; ++i)
        std::memmove(&app->getMemory<x86::reg8>(copies[i]), &app->getMemory<x86::reg8>(vertexOf[i]), 0x20);
    x86::reg32 ebx = c;
    // The [0x7a3a58] & 0x40 test is forced on (tools/apply_alpha_intensity.py).
    {
        const double brightness = app->getMemory<float>(x86::reg32(0x6fbc38));
        cpu.fpu.compare(1.0, brightness);
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        if (1.0 > brightness)
        {
            const x86::reg32 scale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(
                x86::Float(NearestMath::mul(brightness, app->getMemory<double>(x86::reg32(0x536c6c))))));
            for (int i = 0; i < 3; ++i)
                recordWords[i][5] = scaleColour(recordWords[i][5], scale);
            // the last mul's: ebx the red and blue of the third colour, shifted and masked
            const x86::reg32 clamped = x86::sreg32(scale) >= 0x10000 ? 0xffff : scale;
            const x86::reg32 level = (clamped >> 8) & 0xff;
            ebx = (((word(records[2] + 0x14) & 0xff00ff) * level) >> 8) & 0xff00ff;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        word(copies[i] + 0xc) = recordWords[i][0];
        word(copies[i] + 0x18) = recordWords[i][3];
        word(copies[i] + 0x1c) = recordWords[i][4];
        word(copies[i] + 0x10) = recordWords[i][5];
    }
    const x86::reg32 alpha = (word(copies[0] + 0x10) | word(copies[1] + 0x10) | word(copies[2] + 0x10)) >> 24;
    IntegerFlags flags;
    flags.compare(alpha, 8);
    flags.store(cpu);
    cpu.ebx = ebx;
    if (alpha < 8)
    {
        cpu.eax = 0;
        cpu.edx = copies[2];
    }
    else
    {
        word(slot) = polygon + 0xa0;
        cpu.eax = 1;
        cpu.edx = slot;
    }
    finish();
}

/* NFS_NATIVE_CHECK=1: every call runs twice -- the native loop into a copy, then
 * the generated code for real -- and whatever differs is logged: the output
 * records, the registers, the flags and the FPU status word. */
bool checking()
{
    static const bool check = []() {
        const char* value = SDL_getenv("NFS_NATIVE_CHECK");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return check;
}

/* NFS_NATIVE_CHECK=clip: only the clipper checked (checkClip). */
bool clipChecking()
{
    static const bool check = []() {
        const char* value = SDL_getenv("NFS_NATIVE_CHECK");
        return value && SDL_strcmp(value, "clip") == 0;
    }();
    return check || checking();
}

thread_local bool t_original = false;
/* Whether a check is running the generated code: the thread-local is only
 * looked at while checks are on.  On Android every read of one is a call into
 * the runtime (__emutls_get_address), which a profile of a race had at a
 * percent of the game thread across the stand-ins' entries (2026-10-03). */
inline bool originalRunning()
{
    return clipChecking() && t_original;
}
/* Set while a whole-memory check traces the triangles of both paths: a nested
 * check of THRASH_drawtri would take the trace over. */
thread_local bool t_tracing = false;

struct CheckCounts
{
    unsigned calls = 0;
    unsigned items = 0;
    unsigned mismatches = 0;
};

void report(const char* name, CheckCounts& counts, bool same, x86::reg32 items, const char* what)
{
    counts.calls++;
    counts.items += items;
    if (!same)
    {
        counts.mismatches++;
        if (counts.mismatches <= 20)
            SDL_Log("[NATIVE] %s differs: %s (call %u, %u items)", name, what, counts.calls, unsigned(items));
    }
    if (counts.calls % 20000 == 0)
        SDL_Log("[NATIVE] %s checked: %u calls, %u items, %u differ", name, counts.calls, counts.items, counts.mismatches);
}

/* What differs between the state a native left (`mine`) and the one the
 * generated code left: registers, then flags, then the FPU; empty if nothing. */
void describeState(const x86::CPU& mine, const x86::CPU& cpu, char* what, std::size_t size)
{
    if (mine.eax != cpu.eax || mine.ebx != cpu.ebx || mine.ecx != cpu.ecx || mine.edx != cpu.edx
        || mine.esi != cpu.esi || mine.edi != cpu.edi || mine.ebp != cpu.ebp || mine.esp != cpu.esp)
    {
        SDL_snprintf(what, size, "registers: eax %08x/%08x ebx %08x/%08x ecx %08x/%08x edx %08x/%08x esp %08x/%08x",
                     unsigned(mine.eax), unsigned(cpu.eax), unsigned(mine.ebx), unsigned(cpu.ebx),
                     unsigned(mine.ecx), unsigned(cpu.ecx), unsigned(mine.edx), unsigned(cpu.edx),
                     unsigned(mine.esp), unsigned(cpu.esp));
    }
    else if ((mine.flags.lo & 0xd5) != (cpu.flags.lo & 0xd5) || mine.flags.of != cpu.flags.of)
    {
        SDL_snprintf(what, size, "flags %02x/%02x of %u/%u", unsigned(mine.flags.lo), unsigned(cpu.flags.lo),
                     unsigned(mine.flags.of), unsigned(cpu.flags.of));
    }
    else if (mine.fpu.status.word != cpu.fpu.status.word || mine.fpu.count != cpu.fpu.count)
    {
        SDL_snprintf(what, size, "fpu status %04x/%04x depth %u/%u", unsigned(mine.fpu.status.word),
                     unsigned(cpu.fpu.status.word), unsigned(mine.fpu.count), unsigned(cpu.fpu.count));
    }
    else if (mine.fpu.count > 0
             && std::memcmp(&mine.fpu.regs[mine.fpu.count & 7], &cpu.fpu.regs[cpu.fpu.count & 7],
                            sizeof cpu.fpu.regs[0]) != 0)
    {
        SDL_snprintf(what, size, "st(0) %.17g/%.17g", double(mine.fpu.regs[mine.fpu.count & 7]),
                     double(cpu.fpu.regs[cpu.fpu.count & 7]));
    }
}

/* Runs `native` against the generated function at `address`: the output block of
 * `size` bytes at `out` and the state the caller sees afterwards must agree. */
void checkAgainstOriginal(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address,
                          x86::reg32 out, x86::reg32 size, x86::reg32 items, CheckCounts& counts,
                          void (*native)(win32::WinApplication*, x86::CPU&))
{
    x86::reg8* block = &app->getMemory<x86::reg8>(out);
    std::vector<x86::reg8> before(block, block + size);
    const x86::CPU entry = cpu;
    native(app, cpu);
    const x86::CPU mine = cpu;
    std::vector<x86::reg8> written(block, block + size);
    std::memcpy(block, before.data(), size);
    cpu = entry;
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;

    char what[160] = "";
    if (std::memcmp(block, written.data(), size) != 0)
    {
        x86::reg32 at = 0;
        while (at < size && block[at] == written[at])
            ++at;
        SDL_snprintf(what, sizeof what, "memory at +0x%x (item %u, byte 0x%x): 0x%02x native, 0x%02x original",
                     unsigned(at), unsigned(items ? at / (size / items) : 0), unsigned(items ? at % (size / items) : at),
                     unsigned(written[at]), unsigned(block[at]));
    }
    else
    {
        describeState(mine, cpu, what, sizeof what);
    }
    report(name, counts, what[0] == 0, items, what);
}

/* What differs between the triangles two paths handed to the renderer: their
 * number, or a field THRASH_drawtri sets; empty if nothing. */
void describeTriangles(const std::vector<win32::GrVertex>& mineTraced,
                       const std::vector<win32::GrVertex>& originalTraced, char* what, std::size_t size)
{
    if (mineTraced.size() != originalTraced.size())
    {
        SDL_snprintf(what, size, "vertices: %u native, %u original", unsigned(mineTraced.size()),
                     unsigned(originalTraced.size()));
        return;
    }
    struct Field
    {
        const char* name;
        float win32::GrVertex::*member;
    };
    static const Field fields[] = {
        {"x", &win32::GrVertex::x}, {"y", &win32::GrVertex::y}, {"ooz", &win32::GrVertex::ooz},
        {"r", &win32::GrVertex::r}, {"g", &win32::GrVertex::g}, {"b", &win32::GrVertex::b},
        {"a", &win32::GrVertex::a}, {"oow", &win32::GrVertex::oow},
    };
    for (std::size_t i = 0; i < mineTraced.size() && !what[0]; ++i)
    {
        const win32::GrVertex& m = mineTraced[i];
        const win32::GrVertex& o = originalTraced[i];
        auto differ = [&](const char* field, float a, float b) {
            if (!what[0] && std::memcmp(&a, &b, sizeof a) != 0)
                SDL_snprintf(what, size, "vertex %u %s: %.9g native, %.9g original", unsigned(i), field, double(a),
                             double(b));
        };
        for (const Field& field : fields)
            differ(field.name, m.*field.member, o.*field.member);
        differ("s", m.tmuvtx[0].sow, o.tmuvtx[0].sow);
        differ("t", m.tmuvtx[0].tow, o.tmuvtx[0].tow);
    }
}

/* NFS_NATIVE_CHECK for THRASH_drawtri, whose work goes to the renderer rather
 * than to guest memory: every field it sets of what each path hands over, and
 * the state after.  The triangle is drawn twice. */
void checkTriangles(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address,
                    CheckCounts& counts, void (*native)(win32::WinApplication*, x86::CPU&))
{
    static std::vector<win32::GrVertex> mineTraced;
    static std::vector<win32::GrVertex> originalTraced;
    mineTraced.clear();
    originalTraced.clear();
    const x86::CPU entry = cpu;
    win32::glide2x::traceTriangles(&mineTraced);
    native(app, cpu);
    win32::glide2x::traceTriangles(nullptr);
    const x86::CPU mine = cpu;
    cpu = entry;
    win32::glide2x::traceTriangles(&originalTraced);
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;
    win32::glide2x::traceTriangles(nullptr);

    char what[160] = "";
    describeTriangles(mineTraced, originalTraced, what, sizeof what);
    if (!what[0])
        describeState(mine, cpu, what, sizeof what);
    report(name, counts, what[0] == 0, x86::reg32(mineTraced.size() / 3), what);
}

/* NFS_NATIVE_CHECK=1, or =clip for these alone: the clipper or one of its
 * callers run natively, what they write put back, then the generated code; the
 * clipper's records -- the work vertices, the lists and codes after them, the
 * vertices it made, the fan -- and the game's vertices handed in (`vertices`,
 * whose depths the callers write), the triangles that went to THRASH, and the
 * state after must agree.  The triangles are drawn twice. */
typedef std::vector<std::pair<x86::reg32, x86::reg32>> Ranges;

/* The game's vertex records handed in, 0x20 bytes each. */
Ranges vertexRanges(std::initializer_list<x86::reg32> vertices)
{
    Ranges ranges;
    for (x86::reg32 vertex : vertices)
        ranges.push_back({ vertex, 0x20 });
    return ranges;
}

void checkClip(win32::WinApplication* app, x86::CPU& cpu, CheckCounts& counts, const char* name,
               x86::reg32 address, void (*native)(win32::WinApplication*, x86::CPU&), const Ranges& extra = {})
{
    static Ranges kRanges;
    kRanges.assign({ { 0x7cdbb0, 0x400 }, { 0x7d0db0, 0x8c8 }, { 0x7d3570, 0x6c } });
    kRanges.insert(kRanges.end(), extra.begin(), extra.end());
    static std::vector<x86::reg8> before;
    static std::vector<x86::reg8> written;
    static std::vector<win32::GrVertex> mineTraced;
    static std::vector<win32::GrVertex> originalTraced;
    auto copyOut = [app](std::vector<x86::reg8>& to) {
        to.clear();
        for (const auto& range : kRanges)
        {
            const x86::reg8* bytes = &app->getMemory<x86::reg8>(range.first);
            to.insert(to.end(), bytes, bytes + range.second);
        }
    };
    mineTraced.clear();
    originalTraced.clear();
    copyOut(before);
    const x86::CPU entry = cpu;
    t_tracing = true;
    win32::glide2x::traceTriangles(&mineTraced);
    native(app, cpu);
    win32::glide2x::traceTriangles(nullptr);
    const x86::CPU mine = cpu;
    copyOut(written);
    std::size_t at = 0;
    // Backwards: where two ranges overlap, the first copy is the one from before.
    at = before.size();
    for (std::size_t i = kRanges.size(); i-- > 0;)
    {
        at -= kRanges[i].second;
        std::memcpy(&app->getMemory<x86::reg8>(kRanges[i].first), &before[at], kRanges[i].second);
    }
    cpu = entry;
    win32::glide2x::traceTriangles(&originalTraced);
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;
    win32::glide2x::traceTriangles(nullptr);
    t_tracing = false;

    char what[200] = "";
    at = 0;
    for (std::size_t r = 0; r < kRanges.size(); ++r)
    {
        const auto& range = kRanges[r];
        const x86::reg8* original = &app->getMemory<x86::reg8>(range.first);
        for (x86::reg32 i = 0; i < range.second && !what[0]; ++i)
        {
            if (original[i] != written[at + i])
            {
                // the dword it is in, as integers and floats, and where in its range
                const x86::reg32 aligned = i & ~3u;
                x86::reg32 mineWord = 0, originalWord = 0;
                std::memcpy(&mineWord, &written[at + aligned], std::min<x86::reg32>(4, range.second - aligned));
                std::memcpy(&originalWord, &original[aligned], std::min<x86::reg32>(4, range.second - aligned));
                SDL_snprintf(what, sizeof what,
                             "memory at %08x (range %u +0x%x): %08x/%g native, %08x/%g original",
                             unsigned(range.first + i), unsigned(r), unsigned(aligned), unsigned(mineWord),
                             double(asFloat(mineWord)), unsigned(originalWord), double(asFloat(originalWord)));
            }
        }
        at += range.second;
    }
    if (!what[0])
        describeTriangles(mineTraced, originalTraced, what, sizeof what);
    if (!what[0])
        describeState(mine, cpu, what, sizeof what);
    report(name, counts, what[0] == 0, x86::reg32(mineTraced.size() / 3), what);
}

/* NFS_NATIVE_CHECK for a native whose writes go all over the game's records,
 * and through its callees: the call runs natively, everything is put back,
 * the generated code runs, and all of guest memory is compared -- all but the
 * stack below the esp the call returns with, which the natives do not write.
 * Copying the game's memory three times takes a while, so one call of each
 * function in every kWholeEveryMs is checked and the rest run natively.  A
 * callee's own effects outside guest memory (a triangle drawn) happen twice. */
constexpr Uint64 kWholeEveryMs = 1000;
/* Set while a whole-memory check runs a call: a native it calls that has a
 * check of its own (sub_480fc0's sub_47f650) runs unchecked, as its check would
 * take over the copies the outer one puts memory back from.  The outer check
 * covers what it does. */
thread_local bool t_wholeChecking = false;

void checkWholeMemory(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address,
                      x86::reg32 items, CheckCounts& counts, Uint64& last,
                      void (*native)(win32::WinApplication*, x86::CPU&), bool triangles = false)
{
    static std::vector<win32::GrVertex> mineTraced;
    static std::vector<win32::GrVertex> originalTraced;
    mineTraced.clear();
    originalTraced.clear();
    const Uint64 now = SDL_GetTicks();
    if (t_wholeChecking || now - last < kWholeEveryMs)
    {
        native(app, cpu);
        return;
    }
    last = now;

    static std::vector<std::pair<x86::reg32, x86::reg32>> ranges;
    static std::vector<x86::reg8> before;
    static std::vector<x86::reg8> written;
    win32::MemMap::usedRanges(ranges);
    std::size_t total = 0;
    for (const auto& range : ranges)
        total += range.second;
    if (before.size() != total)
        SDL_Log("[NATIVE] whole-memory checks over %u KB in %u ranges", unsigned(total >> 10), unsigned(ranges.size()));
    before.resize(total);
    written.resize(total);
    auto copyOut = [&](std::vector<x86::reg8>& to) {
        std::size_t at = 0;
        for (const auto& range : ranges)
        {
            std::memcpy(&to[at], &app->getMemory<x86::reg8>(range.first), range.second);
            at += range.second;
        }
    };

    static unsigned timed = 0;
    Uint64 stamps[6];
    stamps[0] = SDL_GetTicksNS();
    copyOut(before);
    stamps[1] = SDL_GetTicksNS();
    const x86::CPU entry = cpu;
    t_tracing = triangles;
    t_wholeChecking = true;
    if (triangles)
        win32::glide2x::traceTriangles(&mineTraced);
    native(app, cpu);
    win32::glide2x::traceTriangles(nullptr);
    const x86::CPU mine = cpu;
    stamps[2] = SDL_GetTicksNS();
    copyOut(written);
    stamps[3] = SDL_GetTicksNS();
    std::size_t at = 0;
    for (const auto& range : ranges)
    {
        std::memcpy(&app->getMemory<x86::reg8>(range.first), &before[at], range.second);
        at += range.second;
    }
    stamps[4] = SDL_GetTicksNS();
    cpu = entry;
    if (triangles)
        win32::glide2x::traceTriangles(&originalTraced);
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;
    win32::glide2x::traceTriangles(nullptr);
    t_tracing = false;
    t_wholeChecking = false;
    stamps[5] = SDL_GetTicksNS();
    if (timed++ < 6)
        SDL_Log("[NATIVE] %s whole-memory check: copy %u ms, native %u us, copy %u ms, restore %u ms, original %u us",
                name, unsigned((stamps[1] - stamps[0]) / 1000000), unsigned((stamps[2] - stamps[1]) / 1000),
                unsigned((stamps[3] - stamps[2]) / 1000000), unsigned((stamps[4] - stamps[3]) / 1000000),
                unsigned((stamps[5] - stamps[4]) / 1000));

    const x86::reg32 stackEnd = cpu.esp;
    const x86::reg32 stackStart = cpu.esp - 0x10000;
    unsigned differing = 0;
    // The first few differing bytes: where, and native/original.
    char where[120] = "";
    std::size_t whereLength = 0;
    at = 0;
    for (const auto& range : ranges)
    {
        const x86::reg8* original = &app->getMemory<x86::reg8>(range.first);
        const x86::reg8* mineBytes = &written[at];
        for (x86::reg32 chunk = 0; chunk < range.second; chunk += 0x1000)
        {
            const x86::reg32 end = std::min<x86::reg32>(chunk + 0x1000, range.second);
            if (std::memcmp(original + chunk, mineBytes + chunk, end - chunk) == 0)
                continue;
            for (x86::reg32 i = chunk; i < end; ++i)
            {
                const x86::reg32 address = range.first + i;
                if (original[i] == mineBytes[i] || (address >= stackStart && address < stackEnd))
                    continue;
                if (differing++ < 4 && whereLength < sizeof where)
                    whereLength += SDL_snprintf(where + whereLength, sizeof where - whereLength, " %08x:%02x/%02x",
                                                unsigned(address), unsigned(mineBytes[i]), unsigned(original[i]));
            }
        }
        at += range.second;
    }

    char what[200] = "";
    if (differing)
    {
        // Whose memory: a block's start and size tell a thread's stack from a heap.
        const auto block = win32::MemMap::blockOf(x86::reg32(SDL_strtoul(where + 1, nullptr, 16)));
        SDL_snprintf(what, sizeof what, "memory: %u bytes, native/original at%s; block %08x+%x", differing, where,
                     unsigned(block.first), unsigned(block.second));
    }
    else
    {
        describeTriangles(mineTraced, originalTraced, what, sizeof what);
        if (!what[0])
            describeState(mine, cpu, what, sizeof what);
    }
    report(name, counts, what[0] == 0, items, what);
    if (counts.calls % 100 == 0)
        SDL_Log("[NATIVE] %s checked whole: %u calls, %u items, %u differ", name, counts.calls, counts.items,
                counts.mismatches);
}

}

/* For the stand-ins in native_thrash.cpp, which keep their own checks. */
bool nativeChecking()
{
    return checking();
}

bool nativesEnabled()
{
    return nativesOn();
}

bool nativeOriginalRunning()
{
    return originalRunning();
}

void nativeCheckWhole(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address,
                      void*& state, void (*native)(win32::WinApplication*, x86::CPU&), bool triangles)
{
    struct State
    {
        CheckCounts counts;
        Uint64 last = 0;
    };
    if (!state)
        state = new State;
    State& checked = *static_cast<State*>(state);
    checkWholeMemory(app, cpu, name, address, 1, checked.counts, checked.last, native, triangles);
}

bool trackVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu))
        return false;
    if (checking() && cpu.edx < 0x10000)
    {
        static CheckCounts counts;
        checkAgainstOriginal(app, cpu, "sub_41a550", 0x41a550, cpu.ecx, cpu.edx * 0x1c, cpu.edx, counts,
                             trackVerticesNative);
        return true;
    }
    trackVerticesNative(app, cpu);
    return true;
}

bool objectVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu))
        return false;
    if (checking())
    {
        static CheckCounts counts;
        const x86::sreg32 count = x86::sreg32(cpu.eax);
        const x86::reg32 items = count > 0 && count < 0x10000 ? x86::reg32(count) : 0;
        checkAgainstOriginal(app, cpu, "sub_4bf4c0", 0x4bf4c0, app->getMemory<x86::reg32>(cpu.esp + 4),
                             items * 0x20, items, counts, objectVerticesNative);
        return true;
    }
    objectVerticesNative(app, cpu);
    return true;
}

bool rotateVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !rotateNativeOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        const x86::sreg32 count = x86::sreg32(cpu.eax);
        const x86::reg32 items = count > 0 && count < 0x10000 ? x86::reg32(count) : 0;
        checkAgainstOriginal(app, cpu, "sub_4e03b0", 0x4e03b0, cpu.ecx, items * 0xc, items, counts,
                             rotateVectorsNative);
        return true;
    }
    rotateVectorsNative(app, cpu);
    return true;
}

bool envMapCoords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        static Uint64 last = 0;
        checkWholeMemory(app, cpu, "sub_49db70", 0x49db70, cpu.eax, counts, last, envMapCoordsNative);
        return true;
    }
    envMapCoordsNative(app, cpu);
    return true;
}

bool shadeNormals(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        static Uint64 last = 0;
        checkWholeMemory(app, cpu, "sub_49da30", 0x49da30, cpu.edx, counts, last, shadeNormalsNative);
        return true;
    }
    shadeNormalsNative(app, cpu);
    return true;
}

bool cullPolygons(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        static Uint64 last = 0;
        const x86::reg32 items = app->getMemory<x86::reg32>(cpu.eax + cpu.edx * 4 + 0x6fc);
        checkWholeMemory(app, cpu, "sub_49dbf0", 0x49dbf0, items, counts, last, cullPolygonsNative);
        return true;
    }
    cullPolygonsNative(app, cpu);
    return true;
}

/* sub_4bbd70, where each view pass starts Render_GetTm's buffer over: how much
 * the pass before used, the most since [VIEW] last asked (viewTick in
 * nfs3hp_main.cpp) -- what drawing further would run out of first.  Never
 * takes the call over. */
static x86::reg32 s_arenaPeak = 0;

bool arenaReset(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    const x86::reg32 used = app->getMemory<x86::reg32>(0x55fe38) - app->getMemory<x86::reg32>(0x55fe34);
    if (used > s_arenaPeak && used <= app->getMemory<x86::reg32>(0x7a3d04) + 0x1000)
        s_arenaPeak = used;
    return false;
}

/* Not a stand-in either: sub_40de30 is where an opponent's driver acts on the
 * lane votes sub_409c00 has just added up for it (0x56eea8 on, three
 * directions each, sub_4096b0 among the voters).  NFS_CAR_TRACE writes, for
 * every opponent from 24 to 40 s of the race clock, the car (0x9ac bytes) and
 * the votes (0x56ee80, 0x100 bytes) to ai.bin beside cars.bin.  Each record:
 * "AIVT", the race clock, the car's place in the array at 0x5e1120, 0. */
bool aiVotes(win32::WinApplication* app, x86::CPU& cpu)
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_CAR_TRACE");
        return value && *value && *value != '0';
    }();
    if (!on)
        return false;
    const x86::reg32 clock = app->getMemory<x86::reg32>(0x7d3684);
    const x86::reg32 car = cpu.eax;
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
    const x86::reg32 header[4] = { 0x54564941, clock, (car - 0x5e1120) / 0x9ac, 0 };
    SDL_WriteIO(s_file, header, sizeof header);
    SDL_WriteIO(s_file, &app->getMemory<x86::reg8>(car), 0x9ac);
    SDL_WriteIO(s_file, &app->getMemory<x86::reg8>(0x56ee80), 0x100);
    static x86::reg32 s_flushed = 0;
    if (clock / 64 != s_flushed)
    {
        s_flushed = clock / 64;
        SDL_FlushIO(s_file);
    }
    return false;
}

/* Not stand-ins: NFS_NET_TRACE (extra net_trace) writes what EA's comm
 * library does with a network race into the log ([COMM]), to see where two
 * phones stop on the way into an IPX race.  sub_4f5c10 (the IPX transport
 * opening) turns the library's own event log up ([0x566dec], 0 in the game);
 * sub_512180 is that log: a four-letter tag at eax ("sent", "rsnd", "busy",
 * "hold", "qdat", "recv", "deny"...) with edx, ebx, ecx and two arguments on
 * the stack; sub_51d810 is packet_sendpacket (eax the connection, edx the
 * data, ebx its length); sub_401010 is the library's message printer, its
 * format at [esp+4]. */
bool netTraceOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NET_TRACE");
        return value && *value && *value != '0';
    }();
    return on;
}

std::string guestText(win32::WinApplication* app, x86::reg32 address, size_t limit = 80)
{
    std::string text;
    if (address < 0x400000 || address >= 0xa00000 + 0x100000)
        return text;
    for (size_t i = 0; i < limit; ++i)
    {
        const char c = char(app->getMemory<x86::reg8>(address + x86::reg32(i)));
        if (!c)
            break;
        text += (c == '\n' || c == '\r') ? ' ' : c;
    }
    return text;
}

bool commOpenTrace(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    if (netTraceOn())
    {
        app->getMemory<x86::reg32>(0x566dec) = 3;
        win32::WinApplication::traceNewCalls();
        SDL_Log("[COMM] IPX transport opening, the library's event log turned up, new call targets traced");
    }
    return false;
}

bool commEventTrace(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!netTraceOn())
        return false;
    const x86::reg32 esp = cpu.esp;
    SDL_Log("[COMM] %s edx %08x ebx %08x ecx %08x args %08x %08x", guestText(app, cpu.eax, 8).c_str(),
            unsigned(cpu.edx), unsigned(cpu.ebx), unsigned(cpu.ecx), unsigned(app->getMemory<x86::reg32>(esp + 4)),
            unsigned(app->getMemory<x86::reg32>(esp + 8)));
    return false;
}

bool commSendTrace(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!netTraceOn())
        return false;
    char bytes[3 * 24 + 1] = "";
    const x86::reg32 length = cpu.ebx;
    const int shown = length < 24 ? int(length) : 24;
    for (int i = 0; i < shown; ++i)
        SDL_snprintf(bytes + 3 * i, 4, "%02x ", unsigned(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(i))));
    SDL_Log("[COMM] sendpacket conn %08x, %u bytes: %s", unsigned(cpu.eax), unsigned(length), bytes);
    return false;
}

bool commMessageTrace(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!netTraceOn())
        return false;
    const x86::reg32 esp = cpu.esp;
    SDL_Log("[COMM] message \"%s\" (%s %s line %u) %08x %08x",
            guestText(app, app->getMemory<x86::reg32>(esp + 4), 120).c_str(),
            guestText(app, app->getMemory<x86::reg32>(0x552190)).c_str(),
            guestText(app, app->getMemory<x86::reg32>(0x552194)).c_str(),
            unsigned(app->getMemory<x86::reg32>(0x552198)), unsigned(app->getMemory<x86::reg32>(esp + 8)),
            unsigned(app->getMemory<x86::reg32>(esp + 0xc)));
    return false;
}

x86::reg32 takeArenaPeak()
{
    const x86::reg32 peak = s_arenaPeak;
    s_arenaPeak = 0;
    return peak;
}

bool thrashDrawTri(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !drawTriNativeOn() || !thrashPlainTriangles(app))
        return false;
    if (checking() && !t_tracing)
    {
        static CheckCounts counts;
        checkTriangles(app, cpu, "sub_a85770", 0xa85770, counts, thrashDrawTriNative);
        return true;
    }
    thrashDrawTriNative(app, cpu);
    return true;
}

bool submitPolygons(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        static Uint64 last = 0;
        checkWholeMemory(app, cpu, "sub_434380", 0x434380, 1, counts, last, submitPolygonsNative, true);
        return true;
    }
    submitPolygonsNative(app, cpu);
    return true;
}

bool clipPolygon(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !clipNativeOn())
        return false;
    if (clipChecking() && !t_tracing && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        checkClip(app, cpu, counts, "sub_4c2cd0", 0x4c2cd0, clipPolygonNative);
        return true;
    }
    clipPolygonNative(app, cpu);
    return true;
}

bool clipTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !clipNativeOn())
        return false;
    if (clipChecking() && !t_tracing && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        checkClip(app, cpu, counts, "sub_4c11b0", 0x4c11b0, clipTriangleNative,
                  vertexRanges({ cpu.eax, cpu.edx, cpu.ebx }));
        return true;
    }
    clipTriangleNative(app, cpu);
    return true;
}

bool clipTriangleByZ(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !clipNativeOn())
        return false;
    if (clipChecking() && !t_tracing && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        checkClip(app, cpu, counts, "sub_4c1aa0", 0x4c1aa0, clipTriangleByZNative,
                  vertexRanges({ cpu.eax, cpu.edx, cpu.ebx }));
        return true;
    }
    clipTriangleByZNative(app, cpu);
    return true;
}

bool quadMesh(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !clipNativeOn())
        return false;
    if (clipChecking() && !t_tracing && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges;
        const x86::sreg32 count = x86::sreg32(cpu.eax);
        for (x86::sreg32 i = 0; i < (count < 1 ? 1 : count) && i < 0x1000; ++i)
            for (x86::reg32 k = 0; k < 4; ++k)
                ranges.push_back({ cpu.edx + (app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(i) * 0x10 + k * 4) << 5),
                                   0x20 });
        checkClip(app, cpu, counts, "sub_4c1e20", 0x4c1e20, quadMeshNative, ranges);
        return true;
    }
    quadMeshNative(app, cpu);
    return true;
}

bool clipQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !clipNativeOn())
        return false;
    if (clipChecking() && !t_tracing && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        checkClip(app, cpu, counts, "sub_4c12e0", 0x4c12e0, clipQuadNative,
                  vertexRanges({ cpu.eax, cpu.edx, cpu.ebx, cpu.ecx }));
        return true;
    }
    clipQuadNative(app, cpu);
    return true;
}

/* The checks below run under NFS_NATIVE_CHECK=clip as well as =1: each call,
 * the native into the output block, then the generated code (checkAgainstOriginal). */
bool targetedCheck()
{
    return clipChecking() && !t_tracing;
}

/* Counts the checks can take as items. */
x86::reg32 checkItems(x86::reg32 count)
{
    return x86::sreg32(count) > 0 && x86::sreg32(count) < 0x10000 ? count : 0;
}

bool dotProducts(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        const x86::reg32 items = checkItems(cpu.eax);
        checkAgainstOriginal(app, cpu, "sub_4e0210", 0x4e0210, cpu.ecx, items * 4, items, counts, dotProductsNative);
        return true;
    }
    dotProductsNative(app, cpu);
    return true;
}

bool multiplyMatrices(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        checkAgainstOriginal(app, cpu, "sub_4e0280", 0x4e0280, cpu.ebx, 0x24, 1, counts, multiplyMatricesNative);
        return true;
    }
    multiplyMatricesNative(app, cpu);
    return true;
}

bool transformPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        const x86::reg32 items = checkItems(cpu.eax);
        checkAgainstOriginal(app, cpu, "sub_4e0430", 0x4e0430, app->getMemory<x86::reg32>(cpu.esp + 4), items * 0xc,
                             items, counts, transformPointsNative);
        return true;
    }
    transformPointsNative(app, cpu);
    return true;
}

/* No arithmetic of the FPU's in it: it takes the call in any precision. */
bool gatherBuckets(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        checkAgainstOriginal(app, cpu, "sub_4c79a0", 0x4c79a0, cpu.esi, 8, 1, counts, gatherBucketsNative);
        return true;
    }
    gatherBucketsNative(app, cpu);
    return true;
}

bool objectVerticesNearest(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        const x86::reg32 items = checkItems(cpu.eax);
        checkAgainstOriginal(app, cpu, "sub_49d9d0", 0x49d9d0, cpu.ebx, items * 0x20, items, counts,
                             objectVerticesNearestNative);
        return true;
    }
    objectVerticesNearestNative(app, cpu);
    return true;
}

bool trackRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        /* What it can write: the allocator's pointers, the buffer past its
         * end, the colours of the piece's corners, and the list's links. */
        const x86::reg32 piece = cpu.edx * 0x5c0 + 0x571370;
        const x86::reg32 count = app->getMemory<x86::reg32>(piece + cpu.ebx * 4);
        x86::reg32 polygon = app->getMemory<x86::reg32>(piece + cpu.ebx * 4 + 0x2c);
        Ranges ranges = { { 0x55fe34, 8 }, { app->getMemory<x86::reg32>(0x55fe38), checkItems(count) * 0x20 + 0x40 } };
        for (x86::reg32 i = 0; i < checkItems(count); ++i, polygon += 0xe)
        {
            ranges.push_back({ cpu.ecx + (x86::reg32(x86::sreg32(x86::sreg16(app->getMemory<x86::reg16>(polygon)))) << 5) + 0x10, 4 });
            for (x86::reg32 at : { 0u, 2u, 4u })
                ranges.push_back({ cpu.ecx + (x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(polygon + at)) >> 16) << 5) + 0x10, 4 });
        }
        const x86::reg32 tailArgument = app->getMemory<x86::reg32>(cpu.esp + 4);
        const x86::reg32 tail = app->getMemory<x86::reg32>(tailArgument);
        ranges.push_back({ tailArgument, 4 });
        ranges.push_back({ tail != 0 ? tail : app->getMemory<x86::reg32>(cpu.esp + 8), 4 });
        checkClip(app, cpu, counts, "sub_41b300", 0x41b300, trackRecordsNative, ranges);
        return true;
    }
    trackRecordsNative(app, cpu);
    return true;
}

bool boxOutside(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck())
    {
        static CheckCounts counts;
        checkAgainstOriginal(app, cpu, "sub_41bf30", 0x41bf30, cpu.edx, 0, 0, counts, boxOutsideNative);
        return true;
    }
    boxOutsideNative(app, cpu);
    return true;
}

bool projectedTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        const x86::reg32 slot = cpu.eax;
        checkClip(app, cpu, counts, "sub_41a360", 0x41a360, projectedTriangleNative,
                  Ranges{ { slot, 4 }, { app->getMemory<x86::reg32>(slot), 0xa0 } });
        return true;
    }
    projectedTriangleNative(app, cpu);
    return true;
}

/* The first dword of every polygon on a list, from `first` while `more` says
 * the next belongs to it: the links these functions rewrite. */
template <typename More>
void listLinks(win32::WinApplication* app, x86::reg32 first, Ranges& ranges, More more)
{
    x86::reg32 polygon = first;
    for (int i = 0; polygon != 0 && i < 0x4000; ++i)
    {
        ranges.push_back({ polygon, 4 });
        more(polygon);
        polygon = app->getMemory<x86::reg32>(polygon);
        if (polygon == 0)
            break;
    }
}

/* Everything already in the buckets a call may hang polygons on: every node
 * of the sorted buckets (0x7db6dc, sub_4c7610 rewrites links in the middle of
 * them) and the tail of every other bucket.  Without these the check put back
 * the call's own list but not the links it changed in the buckets, and the
 * generated code then walked a ring (2026-10-03). */
void bucketRanges(win32::WinApplication* app, Ranges& ranges, bool depthBucketsToo)
{
    for (x86::reg32 i = 0; i < 0x320; ++i)
    {
        x86::reg32 node = app->getMemory<x86::reg32>(0x7db6dc + i * 8);
        for (int n = 0; node != 0 && n < 0x4000; ++n)
        {
            ranges.push_back({ node, 4 });
            node = app->getMemory<x86::reg32>(node);
        }
    }
    if (!depthBucketsToo)
        return;
    for (x86::reg32 i = 0; i < 0x100 + 0x7d0; ++i)
    {
        const x86::reg32 bucket = 0x7d575c + i * 8;
        if (app->getMemory<x86::reg32>(bucket) != 0 && app->getMemory<x86::reg32>(bucket + 4) != 0)
            ranges.push_back({ app->getMemory<x86::reg32>(bucket + 4), 4 });
    }
}

bool triangleList(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x554e54, 12 } };
        x86::reg32 triangle = cpu.eax;
        for (int i = 0; triangle != 0 && i < 0x1000; ++i)
        {
            for (x86::reg32 at : { 8u, 0xcu, 0x10u })
                ranges.push_back({ app->getMemory<x86::reg32>(triangle + at), 0x20 });
            triangle = app->getMemory<x86::reg32>(triangle);
            if (triangle == 0 || app->getMemory<x86::reg16>(triangle + 4) != 1)
                break;
        }
        checkClip(app, cpu, counts, "sub_433a50", 0x433a50, triangleListNative, ranges);
        return true;
    }
    triangleListNative(app, cpu);
    return true;
}

bool quadList(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x554e54, 12 } };
        x86::reg32 quad = cpu.eax;
        for (int i = 0; quad != 0 && i < 0x1000; ++i)
        {
            for (x86::reg32 at : { 8u, 0xcu, 0x10u, 0x14u })
                ranges.push_back({ app->getMemory<x86::reg32>(quad + at), 0x20 });
            quad = app->getMemory<x86::reg32>(quad);
            if (quad == 0 || app->getMemory<x86::reg16>(quad + 4) != 0)
                break;
        }
        checkClip(app, cpu, counts, "sub_433bb0", 0x433bb0, quadListNative, ranges);
        return true;
    }
    quadListNative(app, cpu);
    return true;
}

bool sortedBuckets(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x7db6dc, 0x320 * 8 } };
        listLinks(app, cpu.eax, ranges, [](x86::reg32) {});
        bucketRanges(app, ranges, false);
        checkClip(app, cpu, counts, "sub_4c7610", 0x4c7610, sortedBucketsNative, ranges);
        return true;
    }
    sortedBucketsNative(app, cpu);
    return true;
}

bool depthBuckets(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x7d575c, (0x100 + 0x7d0) * 8 }, { 0x7db6dc, 0x320 * 8 } };
        listLinks(app, cpu.eax, ranges, [](x86::reg32) {});
        bucketRanges(app, ranges, true);
        checkClip(app, cpu, counts, "sub_4c7790", 0x4c7790, depthBucketsNative, ranges);
        return true;
    }
    depthBucketsNative(app, cpu);
    return true;
}

/* Small helpers called from everywhere: under NFS_NATIVE_CHECK=clip one call
 * in 64 is checked against the generated code -- every call of the float to
 * integer step alone ran to millions a race and stopped the game. */
/* What a small stand-in needs of the FPU: nothing (integers only, false),
 * the race's single precision (true), or only to draw (kDrawing). */
const int kDrawing = 2;

template <void (*Native)(win32::WinApplication*, x86::CPU&)>
bool smallNative(win32::WinApplication* app, x86::CPU& cpu, int precise, const char* name, x86::reg32 address,
                 x86::reg32 out, x86::reg32 size, CheckCounts& counts)
{
    if (originalRunning() || !nativesOn() || (precise == 1 && !singlePrecision(cpu))
        || (precise == kDrawing && !drawingPrecision(cpu)))
        return false;
    static unsigned s_calls = 0;
    if (targetedCheck() && (++s_calls & 63) == 0)
    {
        checkAgainstOriginal(app, cpu, name, address, out, size, 0, counts, Native);
        return true;
    }
    Native(app, cpu);
    return true;
}

/* The particles' drawing, the drops on the view, the particles' moving: each
 * checked whole against the generated code under NFS_NATIVE_CHECK=clip. */
Ranges particleRanges(win32::WinApplication* app)
{
    Ranges ranges = { { 0x55fe34, 8 }, { 0x7a3d08, 8 }, { 0x7ddfd0, 0x258 * 0x80 }, { 0x5644e0, 0x18 },
                      { 0x7f0bd0, 0x3200 * 2 },
                      { app->getMemory<x86::reg32>(0x55fe38), 0x258 * 0xa0 + 0x40 } };
    const x86::reg32 tail = app->getMemory<x86::reg32>(0x7a3d08);
    if (tail != 0)
        ranges.push_back({ tail, 4 });
    return ranges;
}

bool particles(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = particleRanges(app);
        ranges.push_back({ cpu.esi, 8 });
        checkClip(app, cpu, counts, "sub_4cbc90", 0x4cbc90, particlesNative, ranges);
        return true;
    }
    particlesNative(app, cpu);
    return true;
}

bool drops(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = particleRanges(app);
        ranges.push_back({ 0x7f0bd0, 0x3200 * 2 });
        checkClip(app, cpu, counts, "sub_4cb750", 0x4cb750, dropsNative, ranges);
        return true;
    }
    dropsNative(app, cpu);
    return true;
}

bool moveParticles(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !singlePrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        // sub_4cb220 and sub_4cab60 write their own; the RNG and the drops too
        Ranges ranges = { { 0x7ddfd0, 0x258 * 0x80 }, { 0x5644e0, 0x18 }, { 0x7f0bd0, 0x3200 * 2 },
                          { 0x7f6fd0, 0x10 } };
        checkClip(app, cpu, counts, "sub_4cb2c0", 0x4cb2c0, moveParticlesNative, ranges);
        return true;
    }
    moveParticlesNative(app, cpu);
    return true;
}

bool lightGlow(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x55fe34, 8 }, { app->getMemory<x86::reg32>(0x55fe38), 0xa0 + 0x40 },
                          { 0x7db6dc, 0x320 * 8 }, { cpu.esp + 0x10, 4 }, { cpu.esp + 0x18, 4 } };
        bucketRanges(app, ranges, false);
        checkClip(app, cpu, counts, "sub_492370", 0x492370, lightGlowNative, ranges);
        return true;
    }
    lightGlowNative(app, cpu);
    return true;
}

bool headlightBeam(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { 0x55fe34, 8 }, { app->getMemory<x86::reg32>(0x55fe38), 12 * 0xa0 + 0x40 },
                          { 0x7db6dc, 0x320 * 8 } };
        bucketRanges(app, ranges, false);
        checkClip(app, cpu, counts, "sub_492980", 0x492980, headlightBeamNative, ranges);
        return true;
    }
    headlightBeamNative(app, cpu);
    return true;
}

bool carDetail(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        checkClip(app, cpu, counts, "sub_4bb5d0", 0x4bb5d0, carDetailNative,
                  Ranges{ { 0x7a3bf0, 0x100 }, { cpu.ebx, 4 } });
        return true;
    }
    carDetailNative(app, cpu);
    return true;
}

bool objectVerticesList(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<objectVerticesListNative>(app, cpu, kDrawing, "sub_4dbef0", 0x4dbef0,
                                                 app->getMemory<x86::reg32>(cpu.esp + 8), checkItems(cpu.ebx) * 0x20,
                                                 counts);
}

bool objectRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        const x86::reg32 count = checkItems(cpu.edx);
        Ranges ranges = { { cpu.ecx, count * 0x20 } };
        const x86::reg32 base = app->getMemory<x86::reg32>(cpu.esp + 4);
        x86::reg32 polygon = cpu.ebx;
        for (x86::reg32 i = 0; i < count; ++i, polygon += 0xe)
        {
            ranges.push_back({ base + (x86::reg32(x86::sreg32(x86::sreg16(app->getMemory<x86::reg16>(polygon)))) << 5) + 0x10, 4 });
            for (x86::reg32 at : { 0u, 2u, 4u })
                ranges.push_back({ base + (x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(polygon + at)) >> 16) << 5) + 0x10, 4 });
        }
        checkClip(app, cpu, counts, "sub_41b640", 0x41b640, objectRecordsNative, ranges);
        return true;
    }
    objectRecordsNative(app, cpu);
    return true;
}

bool scaleVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<scaleVectorsNative>(app, cpu, true, "sub_4e0050", 0x4e0050,
                                           app->getMemory<x86::reg32>(cpu.esp + 8), checkItems(cpu.eax) * 0xc, counts);
}

bool addVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<offsetVectorsNative<true>>(app, cpu, true, "sub_4dffb0", 0x4dffb0, cpu.ecx,
                                                  checkItems(cpu.eax) * 0xc, counts);
}

bool subtractVectors(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<offsetVectorsNative<false>>(app, cpu, true, "sub_4e0000", 0x4e0000, cpu.ecx,
                                                   checkItems(cpu.eax) * 0xc, counts);
}

bool vectorLengthOf(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<vectorLengthNative>(app, cpu, true, "sub_4e06a0", 0x4e06a0, 0, 0, counts);
}

bool unitVector(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<unitVectorNative>(app, cpu, true, "sub_4e01c0", 0x4e01c0, cpu.edx, 12, counts);
}

bool groundHeight(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    // p, written over with p - o as the original does: put back before the generated code runs
    return smallNative<groundHeightNative>(app, cpu, true, "sub_4968a0", 0x4968a0, cpu.esp + 0x10, 12, counts);
}

bool truncateTop(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    if (cpu.fpu.count == 0)
        return false;
    return smallNative<truncateNative>(app, cpu, false, "sub_4dfd56", 0x4dfd56, 0, 0, counts);
}

bool dotPush(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<dotPushNative>(app, cpu, true, "sub_4e01f0", 0x4e01f0, 0, 0, counts);
}

bool normalise(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<normaliseNative>(app, cpu, true, "sub_4972f0", 0x4972f0, cpu.eax, 12, counts);
}

bool groundDistanceOf(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<groundDistanceNative>(app, cpu, false, "sub_41aab0", 0x41aab0, 0, 0, counts);
}

bool nearestBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<nearestBlockNative>(app, cpu, true, "sub_41e0f0", 0x41e0f0, 0, 0, counts);
}

bool objectVerticesColoured(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<objectVerticesColouredNative>(app, cpu, kDrawing, "sub_41a970", 0x41a970,
                                                     app->getMemory<x86::reg32>(cpu.esp + 4),
                                                     checkItems(cpu.edx) * 0x20, counts);
}

/* The bytes a fill may write, for its check. */
x86::reg32 fillSize(x86::reg32 count)
{
    return x86::sreg32(count) > 0 && count < 0x100000 ? count : 0;
}

bool fillWord(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<fillWordNative>(app, cpu, false, "sub_4e0721", 0x4e0721, cpu.eax, fillSize(cpu.edx), counts);
}

bool fillByte(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<fillByteNative>(app, cpu, false, "sub_4e0716", 0x4e0716, cpu.eax, fillSize(cpu.edx), counts);
}

bool fillZero(win32::WinApplication* app, x86::CPU& cpu)
{
    static CheckCounts counts;
    return smallNative<fillZeroNative>(app, cpu, false, "sub_4e070c", 0x4e070c, cpu.eax, fillSize(cpu.edx), counts);
}

bool gatherSorted(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !nativesOn())
        return false;
    if (targetedCheck() && !win32::glide2x::tracingTriangles())
    {
        static CheckCounts counts;
        Ranges ranges = { { cpu.esi, 8 } };
        for (x86::reg32 i = 0; i < 0x320; ++i)
        {
            const x86::reg32 tail = app->getMemory<x86::reg32>(0x7db6d4 - i * 8 + 4);
            if (app->getMemory<x86::reg32>(0x7db6d4 - i * 8) != 0 && tail != 0)
                ranges.push_back({ tail, 4 });
        }
        checkClip(app, cpu, counts, "sub_4c7320", 0x4c7320, gatherSortedNative, ranges);
        return true;
    }
    gatherSortedNative(app, cpu);
    return true;
}

bool trackPolygons(win32::WinApplication* app, x86::CPU& cpu)
{
    if (originalRunning() || !drawingPrecision(cpu) || !polygonNativesOn())
        return false;
    if (checking())
    {
        static CheckCounts counts;
        static Uint64 last = 0;
        const x86::reg32 items = app->getMemory<x86::reg32>(cpu.ebx * 0x5c0 + 0x571370 + cpu.ecx * 4);
        checkWholeMemory(app, cpu, "sub_41ae90", 0x41ae90, items, counts, last, trackPolygonsNative);
        return true;
    }
    trackPolygonsNative(app, cpu);
    return true;
}

}
