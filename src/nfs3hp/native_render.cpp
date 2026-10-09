#include <nfs3hp.h>
#include "native_thrash.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstring>

/* a * b + c fused into one fma rounds once where the x87 rounds twice. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* Native stand-ins for the rest of the drawing: the functions native_vertices.cpp
 * left generated -- the clipping of a triangle whose corners carry z, the
 * polygon lists, the sky, the lights, the effects' colours, the views, the HUD,
 * the textures and the 2D screens -- entered from the top of the generated
 * functions (tools/apply_native_render.py) the same way.  Each does the
 * original's arithmetic in the original's order, rounded as the emulated FPU
 * rounds at single precision to nearest (NearestMath), and steps aside -- the
 * generated code runs -- in any other FPU mode, under NFS_NATIVES=0 or
 * =vertices, under NFS_NATIVES=no-render, and for the functions listed in
 * NFS_RENDER_OFF (their addresses, e.g. NFS_RENDER_OFF=4bf790,47a880).
 *
 * Memory, the registers, the flags the generated code leaves (CF, ZF, SF, OF,
 * and PF and AF as its last sahf left them) and the FPU status word come out as
 * the generated code leaves them; only the stack below the returned esp is not
 * written.  The functions they call -- THRASH's, and the game's own -- are
 * called with the registers and the stack the generated code has at the call.
 * NFS_NATIVE_CHECK=1 runs each against the generated code over all of guest
 * memory once a second (nativeCheckWhole in native_vertices.cpp). */
namespace nfs3hp
{

// nfs3hp_main.cpp: a view pass's kind as the main view's where the mirror is drawn in full.
x86::reg32 mirrorAsMain(x86::reg32 kind);
// nfs3hp_main.cpp: View Distance Full kept whole (sub_41d620's tables, split-screen far
// distance) and the mirror at the main view's distances.
x86::reg32 viewDistanceReduced(win32::WinApplication* app, x86::reg32 reduced);
x86::reg32 splitFarDistance(win32::WinApplication* app, x86::reg32 distance);
void mirrorFollowsMain(win32::WinApplication* app);
// nfs3hp_main.cpp: the HUD's sizes at 4:3 proportions, and its pictures' shape on a wide screen.
x86::reg32 hudReferenceWidth(win32::WinApplication* app);
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);
x86::reg32 hudReferenceHeight(win32::WinApplication* app);
void hudTablePanel(win32::WinApplication* app, x86::reg32 player, bool otherLayout);
// nfs3hp_main.cpp: the cabin's rectangle fitted to 4:3 (x, y, width, height).
x86::reg32 cabinRect(win32::WinApplication* app, x86::reg32 field);
// nfs3hp_main.cpp: NFS_CAR_DETAIL_FULL, and the loading screen at 4:3 (ports).
bool fullCarDetail();
void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on);
// nfs3hp_main.cpp: a tap asking to skip the movie playing (a port).
bool movieSkipRequested();

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

/* The emulated FPU's own rounding (FPU::toSingle) for the one mode the natives
 * run in, single precision to nearest, as native_vertices.cpp has it: exact
 * whatever the operands, with the x87's range.  The operands go in the order
 * the generated code hands them to the FPU, which decides a NaN's payload. */
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
typedef NearestMath M;

/* fcomp leaves only its condition bits, and sahf turns them into the flags:
 * only the last comparison matters, replayed on the FPU where the generated
 * code's last sahf would be. */
struct LastCompare
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
    void replay(x86::CPU& cpu) const
    {
        if (!any)
            return;
        cpu.fpu.compare(a, b);
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    }
    // A call in between: what the callee leaves stands instead.
    void forget() { any = false; }
};

/* A byte of a register as mov r8 writes it: k 0 the low byte, 1 the next. */
inline void setByte(x86::reg32& r, unsigned k, x86::reg32 value)
{
    r = (r & ~(0xffu << (8 * k))) | ((value & 0xff) << (8 * k));
}

/* The integer flags of the generated code's cmp, test, and, or, inc and dec:
 * CF, ZF, SF and OF; PF and AF are only ever set by sahf.  Stored only when
 * one of them was set since the last store, so a callee's flags stand until
 * an instruction after the call sets them again. */
struct IntegerFlags
{
    bool cf = false;
    bool of = false;
    bool zf = false;
    bool sf = false;
    bool set = false;

    void logic(x86::reg32 value)
    {
        cf = of = false;
        zf = !value;
        sf = value >> 31;
        set = true;
    }
    void logic8(x86::reg8 value)
    {
        cf = of = false;
        zf = !value;
        sf = value >> 7;
        set = true;
    }
    void compare(x86::reg32 a, x86::reg32 b)
    {
        const x86::reg32 result = a - b;
        cf = a < b;
        of = ((a >> 31) != (result >> 31)) && ((a >> 31) != (b >> 31));
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    void logic16(x86::reg16 value)
    {
        cf = of = false;
        zf = !value;
        sf = value >> 15;
        set = true;
    }
    void compare16(x86::reg16 a, x86::reg16 b)
    {
        const x86::reg16 result = x86::reg16(a - b);
        cf = a < b;
        of = ((a >> 15) != (result >> 15)) && ((a >> 15) != (b >> 15));
        zf = !result;
        sf = result >> 15;
        set = true;
    }
    void compare8(x86::reg8 a, x86::reg8 b)
    {
        const x86::reg8 result = x86::reg8(a - b);
        cf = a < b;
        of = ((a >> 7) != (result >> 7)) && ((a >> 7) != (b >> 7));
        zf = !result;
        sf = result >> 7;
        set = true;
    }
    void add(x86::reg32 a, x86::reg32 b)
    {
        const x86::reg32 result = a + b;
        cf = result < a;
        of = ((a >> 31) == (b >> 31)) && ((a >> 31) != (result >> 31));
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    // inc and dec leave CF as it was
    void inc(x86::reg32 before)
    {
        const x86::reg32 result = before + 1;
        of = !(before >> 31) && (result >> 31);
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    void dec(x86::reg32 before)
    {
        const x86::reg32 result = before - 1;
        of = (before >> 31) && !(result >> 31);
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    // shl by 2..31: OF as it was
    void shl(x86::reg32 value, unsigned count)
    {
        cf = (value >> (32 - count)) & 1;
        const x86::reg32 result = value << count;
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    // inc of a byte register: CF as it was
    void inc8(x86::reg8 before)
    {
        const x86::reg8 result = x86::reg8(before + 1);
        of = before == 0x7f;
        zf = !result;
        sf = result >> 7;
        set = true;
    }
    // sar by 2..31: CF the last bit out, OF as it was
    void sar(x86::reg32 value, unsigned count)
    {
        cf = (x86::sreg32(value) >> (count - 1)) & 1;
        const x86::reg32 result = x86::reg32(x86::sreg32(value) >> count);
        zf = !result;
        sf = result >> 31;
        set = true;
    }
    // The flags a callee left, for an inc or dec that keeps CF.
    void load(const x86::CPU& cpu)
    {
        cf = cpu.flags.cf;
        of = cpu.flags.of;
        zf = cpu.flags.zf;
        sf = cpu.flags.sf;
        set = false;
    }
    void store(x86::CPU& cpu)
    {
        if (!set)
            return;
        cpu.flags.cf = cf;
        cpu.flags.of = of;
        cpu.flags.zf = zf;
        cpu.flags.sf = sf;
        set = false;
    }
};

/* A call as the generated code makes one: a slot for the return address, then
 * the callee.  False when the game is shutting down, as the generated code
 * then returns at once. */
bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    cpu.esp -= 4;
    cpu.ip = address;
    app->dynamic_call(address, cpu);
    return !cpu.terminate;
}

/* Guest memory as the natives read and write it. */
struct Memory
{
    win32::WinApplication* app;
    x86::reg32 word(x86::reg32 address) const { return app->getMemory<x86::reg32>(address); }
    void word(x86::reg32 address, x86::reg32 value) const { app->getMemory<x86::reg32>(address) = value; }
    x86::reg8 byte(x86::reg32 address) const { return app->getMemory<x86::reg8>(address); }
    void byte(x86::reg32 address, x86::reg8 value) const { app->getMemory<x86::reg8>(address) = value; }
    // fld dword: a float as the x87 holds it
    double load(x86::reg32 address) const { return double(app->getMemory<float>(address)); }
    // fstp dword
    void store(x86::reg32 address, double value) const { app->getMemory<float>(address) = float(value); }
    // fld qword
    double loadDouble(x86::reg32 address) const { return app->getMemory<double>(address); }
};

// fild dword
inline double fild32(const Memory& m, x86::reg32 address)
{
    return double(x86::sreg32(m.word(address)));
}

inline bool singlePrecision(const x86::CPU& cpu)
{
#if defined(WITH_PEDANTIC_FPU)
    NFS2_USE(cpu);
    return false;
#elif defined(WITH_WIDE_FPU)
    return cpu.fpu.control.rc == 0 && nativesEnabled();
#else
    return cpu.fpu.control.pc == x86::FPU::s_singlePrecision && cpu.fpu.control.rc == 0 && nativesEnabled();
#endif
}

/* NFS_NATIVES=0 or =vertices: none of these; =no-render: none of these but
 * the others; NFS_RENDER_OFF: the functions at the addresses it lists. */
bool renderNativeOn(x86::reg32 address)
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVES");
        return !value
               || (SDL_strcmp(value, "0") != 0 && SDL_strcmp(value, "vertices") != 0
                   && SDL_strcmp(value, "no-render") != 0);
    }();
    if (!on)
        return false;
    static const char* off = SDL_getenv("NFS_RENDER_OFF");
    if (!off || !*off)
        return true;
    char name[16];
    SDL_snprintf(name, sizeof name, "%x", unsigned(address));
    const std::size_t length = SDL_strlen(name);
    for (const char* at = SDL_strstr(off, name); at; at = SDL_strstr(at + 1, name))
    {
        const bool starts = at == off || at[-1] == ',' || at[-1] == ' ' || at[-1] == 'x' || at[-1] == '_';
        const bool ends = at[length] == 0 || at[length] == ',' || at[length] == ' ';
        if (starts && ends)
            return false;
    }
    return true;
}

/* The registers the generated code works the screen with: they come out of
 * these helpers as its copies of them leave them. */
struct Registers
{
    x86::reg32 eax, ebx, ecx, edx, esi, edi;
};

/* A point in view space (x, y, z floats at `s`) onto the screen as a vertex
 * at `v`: z of 0 taken as 2^-16 (written back), 1/z at `inverse` and at +0xc,
 * x, y scaled ([0x56009c], [0x5600a0]) and centred ([0x5600a4], [0x5600a8]),
 * z's bits at +8.  As sub_432720, sub_47a1f0 and sub_4946e0 write it inline;
 * leaves edx with z's bits. */
void projectPoint(const Memory& m, x86::reg32 s, x86::reg32 inverse, x86::reg32 v, Registers& r)
{
    if (!(m.word(s + 8) & 0x7fffffff))
        m.word(s + 8, 0x37800080);
    m.store(inverse, M::div(1.0, m.load(s + 8)));
    const double x = M::mul(m.load(0x56009c), m.load(inverse));
    const double y = M::mul(m.load(0x5600a0), m.load(inverse));
    m.word(v + 0xc, m.word(inverse));
    const double screenY = M::mul(y, m.load(s + 4));
    const double screenX = M::mul(x, m.load(s));
    r.edx = m.word(s + 8);
    m.store(v, M::add(screenX, m.load(0x5600a4)));
    m.word(v + 8, r.edx);
    m.store(v + 4, M::add(screenY, m.load(0x5600a8)));
}

/* A vertex's clip code from the bits of its 1/z, y and x, to its byte at
 * `code`, as those functions write it inline (esi the vertex, edi the byte). */
void screenCode(const Memory& m, x86::reg32 v, x86::reg32 code, Registers& r)
{
    r.esi = v;
    r.edi = code;
    r.eax = m.word(v + 0xc);
    r.ecx = m.word(0x7d34f8);
    if ((r.eax & 0x80000000) || x86::sreg32(r.eax) >= x86::sreg32(r.ecx))
        r.ebx = 0x10;
    else
    {
        r.eax = m.word(v + 4);
        r.ebx = 0;
        r.ecx = m.word(0x7d3504);
        r.edx = m.word(0x7d3500);
        if ((r.eax & 0x80000000) || x86::sreg32(r.eax) < x86::sreg32(r.ecx))
            r.ebx |= 8;
        else if (x86::sreg32(r.eax) > x86::sreg32(r.edx))
            r.ebx |= 4;
        r.eax = m.word(v);
        r.ecx = m.word(0x7d34fc);
        r.edx = m.word(0x7d350c);
        if ((r.eax & 0x80000000) || x86::sreg32(r.eax) < x86::sreg32(r.ecx))
            r.ebx |= 1;
        else if (x86::sreg32(r.eax) > x86::sreg32(r.edx))
            r.ebx |= 2;
    }
    m.byte(code, x86::reg8(r.ebx));
}

/* screenCode, setting the flags as its tests, compares and ors do: a call
 * or an exit can follow it. */
void screenCodeFlags(const Memory& m, x86::reg32 v, x86::reg32 code, Registers& r, IntegerFlags& flags)
{
    r.esi = v;
    r.edi = code;
    r.eax = m.word(v + 0xc);
    r.ecx = m.word(0x7d34f8);
    flags.logic(r.eax & 0x80000000);
    bool behind = r.eax & 0x80000000;
    if (!behind)
    {
        flags.compare(r.eax, r.ecx);
        behind = x86::sreg32(r.eax) >= x86::sreg32(r.ecx);
    }
    if (behind)
        r.ebx = 0x10;
    else
    {
        r.eax = m.word(v + 4);
        r.ebx = 0;
        r.ecx = m.word(0x7d3504);
        r.edx = m.word(0x7d3500);
        flags.logic(r.eax & 0x80000000);
        bool above = r.eax & 0x80000000;
        if (!above)
        {
            flags.compare(r.eax, r.ecx);
            above = x86::sreg32(r.eax) < x86::sreg32(r.ecx);
        }
        if (above)
            r.ebx |= 8;
        else
        {
            flags.compare(r.eax, r.edx);
            if (x86::sreg32(r.eax) > x86::sreg32(r.edx))
            {
                r.ebx |= 4;
                flags.logic(r.ebx);
            }
        }
        r.eax = m.word(v);
        r.ecx = m.word(0x7d34fc);
        r.edx = m.word(0x7d350c);
        flags.logic(r.eax & 0x80000000);
        bool left = r.eax & 0x80000000;
        if (!left)
        {
            flags.compare(r.eax, r.ecx);
            left = x86::sreg32(r.eax) < x86::sreg32(r.ecx);
        }
        if (left)
        {
            r.ebx |= 1;
            flags.logic(r.ebx);
        }
        else
        {
            flags.compare(r.eax, r.edx);
            if (x86::sreg32(r.eax) > x86::sreg32(r.edx))
            {
                r.ebx |= 2;
                flags.logic(r.ebx);
            }
        }
    }
    m.byte(code, x86::reg8(r.ebx));
}

/* A colour's channels scaled by `scale` (held below 0x10000, its high byte
 * the factor in 256ths), as the game's inline pair of muls: the result in
 * eax, ebx, ecx and edx as they leave them. */
void darken(IntegerFlags& flags, Registers& r, x86::reg32 colour, x86::reg32 scale)
{
    r.ecx = scale;
    r.eax = colour;
    flags.compare(r.ecx, 0x10000);
    if (x86::sreg32(r.ecx) >= 0x10000)
        r.ecx = 0xffff;
    r.ecx = (r.ecx >> 8) & 0xff;
    r.ebx = (r.eax & 0xff00ff00) >> 8;
    r.eax &= 0xff00ff;
    std::uint64_t product = std::uint64_t(r.eax) * r.ecx;
    r.eax = r.ebx;
    r.ebx = x86::reg32(product);
    product = std::uint64_t(r.eax) * r.ecx;
    r.eax = x86::reg32(product);
    r.edx = x86::reg32(product >> 32);
    r.ebx = (r.ebx >> 8) & 0xff00ff;
    r.eax = (r.eax & 0xff00ff00) | r.ebx;
}

/* fld1; fld dword [glare]; fstp qword; fcomp qword; fnstsw ax; sahf: 1
 * against the sun's glare, the flags set as sahf sets them (OF as it was).
 * True when 1 is above it. */
bool glareBelowOne(x86::CPU& cpu, IntegerFlags& flags, Registers& r, double glare)
{
    flags.store(cpu);
    cpu.fpu.compare(x86::Float(1.0), x86::Float(glare));
    r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
    cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    return !cpu.flags.cf && !cpu.flags.zf;
}

/* A call with the registers held in `r`, which get what the callee leaves. */
bool callWithRegisters(win32::WinApplication* app, x86::CPU& cpu, IntegerFlags& flags, Registers& r, x86::reg32 ebp,
                       x86::reg32 esp, x86::reg32 target)
{
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = r.edx;
    cpu.esi = r.esi;
    cpu.edi = r.edi;
    cpu.ebp = ebp;
    cpu.esp = esp;
    if (!call(app, cpu, target))
        return false;
    r = { cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    flags.load(cpu);
    return true;
}

/* The HUD's border width in pixels: the screen's width times [k0] and [k1]
 * (4/640), as fistp rounds it.  The width is the 4:3 one hudReferenceWidth
 * gives (a port, tools/apply_hud_scale.py, in place of [0x7cdac8]). */
x86::reg32 hudBorder(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 k0, x86::reg32 k1)
{
    const Memory m{app};
    const double width = double(x86::sreg32(hudReferenceWidth(app)));
    return x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(M::mul(width, m.load(k0)), m.load(k1)))));
}

}

namespace render
{

/* sub_4bf790: sub_4c1aa0's clipper, a triangle whose corners carry z, cut to
 * the view and drawn as a fan of THRASH_drawtri.
 *   eax  its three corners (pointers to the game's 0x20-byte records: x, y at
 *        +0, +4, z at +8, 1/w at +0xc, colour at +0x10, s, t at +0x18, +0x1c);
 *   bl   their clip codes or-ed together; edx  the three codes
 * The corners are copied to 0x7d3510, 0x7d3530, 0x7d3550 and, through the
 * pointers at 0x5600c8..d0, get s and t times 1/w and 1/z in place of z.  Then
 * Sutherland-Hodgman, each pass only when a code has its bit: the near plane
 * (0x10, 1/z against [0x7d34f0]), the top and the bottom (4, 8: y against
 * [0x7d34dc], [0x7d34e0], both on the codes from before the top), the left and
 * the right (1, 2: x against [0x7d34f4], [0x7d34ec]).  Each pass lists its
 * polygon's pointers after the last one's (from 0x7d2db8) and its codes
 * (from 0x7d0db0); the vertices it makes are records at 0x7cf4b0, each field
 * a - (a - b) * t, the colour's channels as integers, t re-derived for the
 * colour at the near plane when it fell outside 0..1.  The polygon left gets
 * s, t divided by 1/w, its depth from 1/z, 1/w held to 0..1, and goes to
 * THRASH_drawtri ([0x9ef974]) as a fan from its first corner, last triangle
 * first, a triangle with an indefinite in a corner's x, y, depth, 1/w, s or t
 * skipped.  Returns the triangles tried in eax and ebx, the list in edx; -1
 * in eax and ebx when a pass leaves fewer than three corners. */
void clipByZNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xf4; sub ebp, 0x82
    const x86::reg32 frameEsp = entry - 0x104;
    const x86::reg32 frameEbp = entry - 0x92;
    x86::reg8 clipCodes = cpu.bl;
    LastCompare compared;

    auto leave = [&](x86::reg32 result, x86::reg32 edx) {
        cpu.eax = result;
        cpu.ebx = result;
        cpu.edx = edx;
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = entry + 4;
    };

    // The corners copied (rep movsd), each pointer read as its copy starts.
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        const x86::reg32 from = m.word(cpu.eax + 4 * i);
        const x86::reg32 to = 0x7d3510 + 0x20 * i;
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            m.word(to + k, m.word(from + k));
    }
    for (const x86::reg32 slot : { 0x5600c8u, 0x5600ccu, 0x5600d0u })
    {
        const x86::reg32 p = m.word(slot);
        const double s = M::mul(m.load(p + 0xc), m.load(p + 0x18));
        const double t = M::mul(m.load(p + 0xc), m.load(p + 0x1c));
        m.store(p + 0x18, s);
        m.store(p + 0x1c, t);
    }
    {
        const x86::reg32 p0 = m.word(0x5600c8);
        m.store(p0 + 8, M::div(1.0, m.load(p0 + 8)));
        const x86::reg32 p1 = m.word(0x5600cc);
        m.store(p1 + 8, M::div(1.0, m.load(p1 + 8)));
        const x86::reg32 p2 = m.word(0x5600d0);
        const double z = M::div(1.0, m.load(p2 + 8));
        m.word(0x7d3524, 0);
        m.word(0x7d3544, 0);
        m.store(p2 + 8, z);
        m.word(0x7d3564, 0);
    }

    x86::reg32 list = 0x5600c8;    // [ebp+0x56]
    x86::reg32 codes = cpu.edx;    // [ebp+0x52]
    x86::reg32 outList = 0x7d2db8; // ecx
    x86::reg32 outCodes = 0x7d0db0; // edx
    x86::reg32 count = 3;          // ebx
    x86::reg32 made = 0;           // [ebp+0x4e]

    auto codeX = [&](x86::reg32 vertex, x86::reg32 code) {
        const double x = m.load(vertex);
        const double left = m.load(0x7d34f4);
        compared(x, left);
        if (!(x >= left))
            m.byte(code, m.byte(code) | 1);
        else
        {
            const double right = m.load(0x7d34ec);
            compared(x, right);
            if (x > right)
                m.byte(code, m.byte(code) | 2);
        }
    };
    // a - (a - b) * t; t*(a - b) is kept on the x87 for all but the last field
    auto between = [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 field, double t) {
        const double from = m.load(a + field);
        m.store(vertex + field, M::sub(from, M::mul(M::sub(from, m.load(b + field)), t)));
    };
    // ... and the last, t, whose difference goes through a float on the stack
    auto betweenStored = [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 field, double t) {
        const double difference = double(float(M::sub(m.load(a + field), m.load(b + field))));
        m.store(vertex + field, M::sub(m.load(a + field), M::mul(difference, t)));
    };
    // The colour's channels, each a - fistp((a - b) * t), packed again.
    auto blend = [&](x86::reg32 a, x86::reg32 b, double t) {
        const x86::reg32 ca = m.word(a + 0x10);
        const x86::reg32 cb = m.word(b + 0x10);
        x86::reg32 channels[4];
        for (int i = 0; i < 4; ++i)
        {
            const unsigned shift = 24 - 8 * i;
            const x86::reg32 from = (ca >> shift) & 0xff;
            const x86::reg32 to = (cb >> shift) & 0xff;
            const double product = M::mul(double(x86::sreg32(from - to)), t);
            channels[i] = from - x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(product)));
        }
        return (channels[0] << 24) | (channels[1] << 16) | (channels[2] << 8) | channels[3];
    };

    /* One pass: the corners outside `bit` dropped, a vertex made where an edge
     * crosses it.  False -- and the function has returned -- when fewer than
     * three are left; the lists move on otherwise. */
    auto pass = [&](x86::reg8 bit, auto&& makeVertex) -> bool {
        x86::reg32 kept = 0;
        x86::reg32 previous = count - 1;
        for (x86::reg32 current = 0; current < count; previous = current++)
        {
            const x86::reg8 previousCode = m.byte(codes + previous);
            if (!(previousCode & bit))
            {
                m.word(outList + 4 * kept, m.word(list + 4 * previous));
                m.byte(outCodes + kept, previousCode);
                ++kept;
            }
            if ((previousCode ^ m.byte(codes + current)) & bit)
            {
                const x86::reg32 vertex = 0x7cf4b0 + made * 0x20;
                makeVertex(m.word(list + 4 * previous), m.word(list + 4 * current), vertex, outCodes + kept,
                           m.byte(codes + previous));
                ++made;
                m.word(outList + 4 * kept, vertex);
                ++kept;
            }
        }
        if (kept < 3)
        {
            compared.replay(cpu);
            IntegerFlags flags;
            flags.compare(kept, 3);
            flags.store(cpu);
            leave(0xffffffff, outCodes);
            return false;
        }
        list = outList;
        codes = outCodes;
        outList += 4 * kept;
        outCodes += kept;
        count = kept;
        return true;
    };
    auto orCodes = [&]() {
        clipCodes = 0;
        for (x86::reg32 i = count; i-- > 0;)
            clipCodes |= m.byte(codes + i);
    };

    if (clipCodes & 0x10)
    {
        const bool kept = pass(0x10, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code,
                                         x86::reg8) {
            const double aZ = m.load(a + 8);
            double t = double(float(M::div(M::sub(aZ, m.load(0x7d34f0)), M::sub(aZ, m.load(b + 8)))));
            for (const x86::reg32 field : { 0u, 4u, 0xcu, 0x18u })
                between(a, b, vertex, field, t);
            betweenStored(a, b, vertex, 0x1c, t);
            bool again = x86::sreg32(asBits(float(t))) > 0x3f800000;
            if (!again)
            {
                compared(0.0, t);
                again = 0.0 > t;
            }
            if (again)
                t = double(float(M::div(m.load(a + 8), M::sub(m.load(a + 8), m.load(b + 8)))));
            m.word(vertex + 0x10, blend(a, b, t));
            m.word(vertex + 8, m.word(0x7d34f0));
            m.byte(code, 0x10);
            const double y = m.load(vertex + 4);
            const double bottom = m.load(0x7d34e0);
            compared(y, bottom);
            if (!(y >= bottom))
                m.byte(code, m.byte(code) | 8);
            else
            {
                const double top = m.load(0x7d34dc);
                compared(y, top);
                if (y > top)
                    m.byte(code, m.byte(code) | 4);
            }
            codeX(vertex, code);
        });
        if (!kept)
            return;
        orCodes();
    }

    if (clipCodes & 0xc)
    {
        const x86::reg8 vertical = clipCodes;
        for (const x86::reg8 bit : { x86::reg8(4), x86::reg8(8) })
        {
            if (!(vertical & bit))
                continue;
            const x86::reg32 edge = bit == 4 ? 0x7d34dc : 0x7d34e0;
            const bool kept = pass(bit, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code,
                                            x86::reg8 previousCode) {
                const double aY = m.load(a + 4);
                const double t = double(float(M::div(M::sub(aY, m.load(edge)), M::sub(aY, m.load(b + 4)))));
                for (const x86::reg32 field : { 0u, 8u, 0xcu, 0x18u })
                    between(a, b, vertex, field, t);
                betweenStored(a, b, vertex, 0x1c, t);
                m.word(vertex + 0x10, blend(a, b, t));
                m.word(vertex + 4, m.word(edge));
                m.byte(code, x86::reg8((previousCode & 0xf0) | bit));
                codeX(vertex, code);
            });
            if (!kept)
                return;
        }
        orCodes();
    }

    if (clipCodes & 3)
    {
        const x86::reg8 horizontal = clipCodes;
        for (const x86::reg8 bit : { x86::reg8(1), x86::reg8(2) })
        {
            if (!(horizontal & bit))
                continue;
            const x86::reg32 edge = bit == 1 ? 0x7d34f4 : 0x7d34ec;
            const bool kept = pass(bit, [&](x86::reg32 a, x86::reg32 b, x86::reg32 vertex, x86::reg32 code,
                                            x86::reg8 previousCode) {
                const double aX = m.load(a);
                const double t = double(float(M::div(M::sub(aX, m.load(edge)), M::sub(aX, m.load(b)))));
                for (const x86::reg32 field : { 4u, 8u, 0xcu, 0x18u })
                    between(a, b, vertex, field, t);
                betweenStored(a, b, vertex, 0x1c, t);
                m.word(vertex + 0x10, blend(a, b, t));
                m.word(vertex, m.word(edge));
                m.byte(code, x86::reg8((previousCode & 0xfc) | bit));
            });
            if (!kept)
                return;
        }
    }

    // The corners left, as THRASH takes them.
    for (x86::reg32 i = 0; i < count; ++i)
    {
        const x86::reg32 v = m.word(list + 4 * i);
        const double s = m.load(v + 0x18);
        const double inverse = M::div(1.0, m.load(v + 0xc));
        m.store(v + 0x18, M::mul(s, inverse));
        m.store(v + 0x1c, M::mul(inverse, m.load(v + 0x1c)));
        m.word(v + 0x14, 0);
        x86::reg32 depth = 0;
        if (!(m.byte(v + 0xf) & 0x80) && x86::sreg32(m.word(v + 8)) <= 0x3f800000)
            depth = asBits(float(M::sub(1.0, m.load(v + 8))));
        m.word(v + 8, depth);
        m.store(v + 8, M::mul(m.load(0x560094), m.load(v + 8)));
        m.store(v + 8, M::add(m.load(0x560098), m.load(v + 8)));
        const double w = m.load(v + 0xc);
        compared(0.0, w);
        if (0.0 > w)
            m.word(v + 0xc, 0);
        else
        {
            const x86::reg32 bits = m.word(v + 0xc);
            m.word(v + 0xc, x86::sreg32(bits) > 0x3f800000 ? 0x3f800000 : bits);
        }
    }

    // The fan, last triangle first.
    compared.replay(cpu);
    const x86::reg32 first = m.word(list);
    x86::reg32 drawn = 0;
    for (x86::reg32 i = count - 2; i >= 1; --i)
    {
        ++drawn;
        const x86::reg32 b = m.word(list + 4 * i);
        const x86::reg32 c = m.word(list + 4 * (i + 1));
        bool skip = false;
        x86::reg32 last = 0;
        for (const x86::reg32 vertex : { first, b, c })
        {
            for (const x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
            {
                last = m.word(vertex + field) | 0x80000000;
                if (last == 0xffc00000)
                {
                    skip = true;
                    break;
                }
            }
            if (skip)
                break;
        }
        if (skip)
            continue;
        IntegerFlags flags;
        flags.compare(last, 0xffc00000);
        flags.store(cpu);
        m.word(frameEsp - 4, c);
        m.word(frameEsp - 8, b);
        m.word(frameEsp - 12, first);
        cpu.eax = c;
        cpu.esi = b;
        cpu.edi = first;
        cpu.ebx = i;
        cpu.ecx = drawn;
        cpu.edx = last;
        cpu.ebp = frameEbp;
        cpu.esp = frameEsp - 12;
        if (!call(app, cpu, m.word(0x9ef974)))
            return;
    }
    // test ebx, ebx with ebx 0 ended the loop
    cpu.flags.cf = false;
    cpu.flags.of = false;
    cpu.flags.zf = true;
    cpu.flags.sf = false;
    leave(drawn, list);
}

/* sub_4c20e0: a quad (eax, edx, ebx, ecx: a, b, c, d) whose corners carry z,
 * to the screen -- as sub_4c12e0 is for the clipper of sub_4c2cd0, here for
 * sub_4bf790's.  Nothing when the four share a clip bit (the byte at +0x14);
 * THRASH_drawquad ([0x9ef94c]) when none has one; otherwise triangles a b c
 * and a c d, each skipped when its corners share a bit, clipped by sub_4bf790
 * when one has one, else drawn by THRASH_drawtri ([0x9ef974]).  A corner
 * drawn gets its depth from 1/z (65535 for a z of 0) as sub_4c1aa0's do,
 * +0x14 cleared and 1/w held to 0..1; nothing is drawn of a quad or triangle
 * with an indefinite in a corner's x, y, depth, 1/w, s or t.
 *
 * Every corner is written out inline in the original, each copy with its own
 * scratch registers; eax, ebx, ecx and edx come out as each path leaves them,
 * the callees' included, and so do the flags and the FPU status. */
void quadByZNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0xdc; sub ebp, 0x82
    const x86::reg32 frameEbp = entry - 0x8e;
    const x86::reg32 frameEsp = entry - 0xe8;
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx, d = cpu.ecx;
    x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    x86::reg32 esi = c, edi = a;
    LastCompare compared;
    IntegerFlags flags;

    auto leave = [&]() {
        compared.replay(cpu);
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4;
    };
    // After a last call: its registers, flags and FPU, ours put back.
    auto leaveAfterCall = [&]() {
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4;
    };
    auto callWith = [&](x86::reg32 target, x86::reg32 esp) -> bool {
        compared.replay(cpu);
        compared.forget();
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = frameEbp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        return true;
    };

    /* A corner as every copy of it does it; what differs between the copies
     * is which registers are left with what. */
    struct Corner
    {
        x86::reg32 z;      // z's bits
        x86::reg32 high;   // the top byte of 1/z
        bool zeroDepth;    // 1/z negative or past 1: depth 0
        bool negativeW;    // 1/w below 0: held to 0
        x86::reg32 w;      // 1/w's bits as they came
        x86::reg32 heldW;  // ... and as written back
    };
    auto corner = [&](x86::reg32 v) {
        Corner k;
        k.z = m.word(v + 8);
        const x86::reg32 inverse = (k.z & 0x7fffffff) ? asBits(float(M::div(1.0, m.load(v + 8)))) : 0x477fff00;
        k.high = inverse >> 24;
        m.word(v + 0x14, 0);
        k.zeroDepth = (inverse & 0x80000000) || x86::sreg32(inverse) > 0x3f800000;
        m.word(v + 8, k.zeroDepth ? 0 : asBits(float(M::sub(1.0, double(asFloat(inverse))))));
        m.store(v + 8, M::mul(m.load(0x560094), m.load(v + 8)));
        m.store(v + 8, M::add(m.load(0x560098), m.load(v + 8)));
        const double w = m.load(v + 0xc);
        compared(0.0, w);
        k.negativeW = 0.0 > w;
        k.w = m.word(v + 0xc);
        if (k.negativeW)
        {
            k.heldW = 0;
            flags.logic(0);  // xor r, r
        }
        else
        {
            flags.compare(k.w, 0x3f800000);
            k.heldW = x86::sreg32(k.w) > 0x3f800000 ? 0x3f800000 : k.w;
        }
        m.word(v + 0xc, k.heldW);
        return k;
    };
    /* The fields THRASH takes of each corner, an indefinite in any of them
     * (or eax, 0x80000000; cmp eax, 0xffc00000).  True when one is. */
    auto anyIndefinite = [&](std::initializer_list<x86::reg32> vertices) {
        for (const x86::reg32 vertex : vertices)
            for (const x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
            {
                eax = m.word(vertex + field) | 0x80000000;
                flags.compare(eax, 0xffc00000);
                if (eax == 0xffc00000)
                    return true;
            }
        return false;
    };

    m.word(frameEbp + 0x7e, d);
    const x86::reg8 ca = m.byte(a + 0x14), cb = m.byte(b + 0x14), cc = m.byte(c + 0x14), cd = m.byte(d + 0x14);
    m.byte(frameEbp - 0x46, ca);
    m.byte(frameEbp - 0x45, cb);
    m.byte(frameEbp - 0x44, cc);
    m.byte(frameEbp - 0x43, cd);

    // al, ah, bl = the codes of a, b, c; and al, ah; and bl, al; mov al, d's; and al, bl
    const x86::reg8 threeShared = ca & cb & cc;
    const x86::reg8 shared = cd & threeShared;
    eax = (a & 0xffff0000) | (x86::reg32(cb) << 8) | shared;
    ebx = (c & 0xffffff00) | threeShared;
    flags.logic8(shared);
    if (shared)
    {
        leave();
        return;
    }
    // mov cl, c's; or al, ah; or al, cl; mov ah, d's; or ah, al
    ecx = (ecx & 0xffffff00) | cc;
    const x86::reg8 threeAny = ca | cb | cc;
    const x86::reg8 any = cd | threeAny;
    eax = (eax & 0xffff0000) | (x86::reg32(any) << 8) | threeAny;
    flags.logic8(any);

    if (!any)
    {
        Corner k = corner(a);
        ecx = k.z;
        setByte(ecx, 1, k.high);
        if (k.zeroDepth)
            ebx = 0;
        if (!k.negativeW)
            ecx = k.w;
        eax = k.heldW;

        k = corner(b);
        if (k.negativeW)
            ecx = 0;
        else
            ebx = k.w;
        eax = k.heldW;

        k = corner(c);
        ecx = k.z;
        if (k.zeroDepth)
            ebx = 0;
        if (!k.negativeW)
            ecx = k.w;
        eax = k.heldW;

        k = corner(d);
        ecx = k.z;
        ebx = d;
        if (!k.negativeW)
            ecx = k.w;
        eax = k.heldW;

        if (anyIndefinite({ a, b, c, d }))
        {
            leave();
            return;
        }
        m.word(frameEsp - 4, ebx);
        m.word(frameEsp - 8, esi);
        m.word(frameEsp - 12, edx);
        m.word(frameEsp - 16, edi);
        if (!callWith(m.word(0x9ef94c), frameEsp - 16))
            return;
        leaveAfterCall();
        return;
    }

    // Triangle a b c.
    flags.logic8(threeShared);
    if (!threeShared)
    {
        flags.logic8(threeAny);
        if (threeAny)
        {
            m.word(frameEbp - 0x5a, edi);
            m.word(frameEbp - 0x56, edx);
            ebx = threeAny;
            edx = frameEbp - 0x46;
            eax = frameEbp - 0x5a;
            m.word(frameEbp - 0x52, esi);
            if (!callWith(0x4bf790, frameEsp))
                return;
        }
        else
        {
            Corner k = corner(edi);
            ecx = k.z;
            setByte(ebx, 1, k.high);
            if (k.zeroDepth)
                ebx = 0;
            if (!k.negativeW)
                ecx = k.w;
            eax = k.heldW;

            k = corner(edx);
            setByte(ecx, 0, k.high);
            if (k.negativeW)
                ecx = 0;
            else
                ebx = k.w;
            eax = k.heldW;

            k = corner(esi);
            ecx = k.z;
            setByte(ecx, 1, k.high);
            if (k.zeroDepth)
                ebx = 0;
            if (!k.negativeW)
                ecx = k.w;
            eax = k.heldW;

            if (!anyIndefinite({ edi, edx, esi }))
            {
                m.word(frameEsp - 4, esi);
                m.word(frameEsp - 8, edx);
                m.word(frameEsp - 12, edi);
                if (!callWith(m.word(0x9ef974), frameEsp - 12))
                    return;
            }
        }
    }

    // Triangle a c d, the codes read back from the frame.
    const x86::reg8 ta = m.byte(frameEbp - 0x46), tc = m.byte(frameEbp - 0x44), td = m.byte(frameEbp - 0x43);
    edx = (edx & 0xffffff00) | td;
    const x86::reg8 shared2 = ta & tc & td;
    eax = (eax & 0xffff0000) | (x86::reg32(tc) << 8) | shared2;
    flags.logic8(shared2);
    if (shared2)
    {
        leave();
        return;
    }
    const x86::reg8 any2 = ta | tc | td;
    eax = (eax & 0xffff0000) | (x86::reg32(any2) << 8) | x86::reg8(ta | tc);
    flags.logic8(any2);
    if (any2)
    {
        // The clipper's list and codes: a, c, d.
        edx = d;
        m.word(frameEbp - 0x5a, edi);
        m.word(frameEbp - 0x52, edx);
        m.byte(frameEbp - 0x45, m.byte(frameEbp - 0x44));
        m.byte(frameEbp - 0x44, m.byte(frameEbp - 0x43));
        ebx = any2;
        edx = frameEbp - 0x46;
        eax = frameEbp - 0x5a;
        m.word(frameEbp - 0x56, esi);
        if (!callWith(0x4bf790, frameEsp))
            return;
        leaveAfterCall();
        return;
    }

    Corner k = corner(edi);
    setByte(ebx, 0, k.high);
    if (k.zeroDepth)
        ebx = 0;
    if (!k.negativeW)
        edx = k.w;
    eax = k.heldW;

    k = corner(esi);
    ebx = k.z;
    setByte(ebx, 1, k.high);
    if (k.zeroDepth)
        ecx = 0;
    if (k.negativeW)
        ebx = 0;
    eax = k.heldW;

    k = corner(d);
    ecx = k.z;
    setByte(ecx, 0, k.high);
    if (k.zeroDepth)
        ecx = 0;
    edx = d;
    if (k.negativeW)
        ebx = 0;
    eax = k.heldW;

    if (anyIndefinite({ edi, esi, edx }))
    {
        leave();
        return;
    }
    m.word(frameEsp - 4, edx);
    m.word(frameEsp - 8, esi);
    m.word(frameEsp - 12, edi);
    if (!callWith(m.word(0x9ef974), frameEsp - 12))
        return;
    leaveAfterCall();
}

/* sub_434120: the triangles of a polygon list to THRASH, from the record in
 * eax for as long as the next one is a triangle too (type 3, the word at +4).
 * A record: the next at +0, bit 0x40 of +7 to draw it twice, its corners at
 * +8, +0xc, +0x10 (the game's vertices, their clip codes at +0x14); the
 * texture at +0x18 and the corners' s, t at +0x20..+0x34, and for the second
 * pass at +0x50 and +0x54..+0x68.  A triangle whose corners share a clip bit
 * is skipped; one with a corner past the near plane (0x10) goes to sub_4c3ad0.
 * The rest are drawn by THRASH_drawtri with their s, t written to the corners,
 * the texture set (THRASH_setstate 1, the last one kept in [0x554e54]) and the
 * blending (state 0x68, kept in [0x554e58]) off, state 0x2c2 on around the
 * call when a corner is off the left (code bit 1); then, when asked for, again
 * with the second texture and the blending on.  Returns the record it stopped
 * at (0 at the end of the list) in eax.
 *
 * No x87 here: the registers go through as the original moves them, since
 * sub_4c3ad0 gets them. */
void triangleRecordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 frameEbp = entry - 24;
    const x86::reg32 frameEsp = entry - 28;
    x86::reg32 eax = cpu.eax, ebx = cpu.eax, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, edi = cpu.edi;
    IntegerFlags flags;

    auto callWith = [&](x86::reg32 target, std::initializer_list<x86::reg32> pushed) -> bool {
        x86::reg32 esp = frameEsp;
        for (const x86::reg32 value : pushed)
        {
            esp -= 4;
            m.word(esp, value);
        }
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = frameEbp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        return true;
    };
    auto setState = [&](x86::reg32 state, x86::reg32 value) { return callWith(m.word(0x9ef96c), { value, state }); };
    // The corners' s, t from the record at `from`.
    auto texture = [&](x86::reg32 from) {
        for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
        {
            eax = m.word(ebx + corner);
            edx = m.word(ebx + from);
            m.word(eax + 0x18, edx);
            eax = m.word(ebx + corner);
            edx = m.word(ebx + from + 4);
            m.word(eax + 0x1c, edx);
            from += 8;
        }
    };

    for (;;)
    {
        esi = m.word(ebx + 8);
        edx = m.word(ebx + 0xc);
        x86::reg8 cl = m.byte(esi + 0x14) & m.byte(edx + 0x14);
        eax = m.word(ebx + 0x10);
        const x86::reg8 ch = m.byte(eax + 0x14);
        cl &= ch;
        setByte(ecx, 0, cl);
        setByte(ecx, 1, ch);
        flags.logic8(cl);
        if (!cl)
        {
            cl = m.byte(esi + 0x14) & 0x10;
            setByte(ecx, 0, cl);
            const x86::reg8 dl = (m.byte(edx + 0x14) & 0x10) | cl;
            setByte(edx, 0, dl);
            const x86::reg8 al = (ch & 0x10) | dl;
            setByte(eax, 0, al);
            flags.logic8(al);
            if (al)
            {
                edx = al;
                eax = ebx;
                flags.store(cpu);
                cpu.eax = eax;
                cpu.ebx = ebx;
                cpu.ecx = ecx;
                cpu.edx = edx;
                cpu.esi = esi;
                cpu.edi = edi;
                cpu.ebp = frameEbp;
                cpu.esp = frameEsp;
                if (!call(app, cpu, 0x4c3ad0))
                    return;
                eax = cpu.eax;
                ebx = cpu.ebx;
                ecx = cpu.ecx;
                edx = cpu.edx;
                esi = cpu.esi;
                edi = cpu.edi;
            }
            else
            {
                flags.compare(m.word(0x554e58), 0);
                if (m.word(0x554e58) != 0)
                {
                    ecx = 0;
                    m.word(0x554e58, ecx);
                    if (!setState(0x68, ecx))
                        return;
                }
                esi = m.word(ebx + 0x18);
                flags.logic(esi);
                if (esi)
                {
                    texture(0x20);
                    eax = m.word(0x554e54);
                    edi = m.word(ebx + 0x18);
                    flags.compare(eax, edi);
                    if (eax != edi)
                    {
                        m.word(0x554e54, edi);
                        if (!setState(1, edi))
                            return;
                    }
                }
                else
                {
                    flags.compare(m.word(0x554e54), 0);
                    if (m.word(0x554e54) != 0)
                    {
                        m.word(0x554e54, esi);
                        if (!setState(1, esi))
                            return;
                    }
                }
                edx = m.word(ebx + 8);
                esi = m.word(ebx + 0xc);
                eax = m.word(edx + 0x14) & 1;
                ecx = (m.word(esi + 0x14) & 1) | eax;
                eax = m.word(ebx + 0x10);
                edi = m.word(eax + 0x14) & 1;
                ecx |= edi;
                flags.logic(ecx);
                if (!ecx)
                {
                    if (!callWith(m.word(0x9ef974), { eax, esi, edx }))
                        return;
                }
                else
                {
                    if (!setState(0x2c2, 1))
                        return;
                    eax = m.word(ebx + 0x10);
                    edx = m.word(ebx + 0xc);
                    ecx = m.word(ebx + 8);
                    if (!callWith(m.word(0x9ef974), { eax, edx, ecx }))
                        return;
                    if (!setState(0x2c2, 0))
                        return;
                }

                flags.logic8(m.byte(ebx + 7) & 0x40);
                if (m.byte(ebx + 7) & 0x40)
                {
                    flags.compare(m.word(0x554e58), 1);
                    if (m.word(0x554e58) != 1)
                    {
                        edi = 1;
                        m.word(0x554e58, edi);
                        if (!setState(0x68, edi))
                            return;
                    }
                    eax = m.word(ebx + 0x50);
                    flags.logic(eax);
                    if (eax)
                    {
                        texture(0x54);
                        eax = m.word(0x554e54);
                        edx = m.word(ebx + 0x50);
                        flags.compare(eax, edx);
                        if (eax != edx)
                        {
                            m.word(0x554e54, edx);
                            if (!setState(1, edx))
                                return;
                        }
                    }
                    else
                    {
                        flags.compare(m.word(0x554e54), 0);
                        if (m.word(0x554e54) != 0)
                        {
                            m.word(0x554e54, eax);
                            if (!setState(1, eax))
                                return;
                        }
                    }
                    eax = m.word(ebx + 8);
                    esi = m.word(ebx + 0xc);
                    ecx = (m.word(eax + 0x14) & 1) | (m.word(esi + 0x14) & 1);
                    edx = m.word(ebx + 0x10);
                    m.word(frameEbp - 4, ecx);
                    edi = ecx;
                    ecx = (m.word(edx + 0x14) & 1) | edi;
                    flags.logic(ecx);
                    if (!ecx)
                    {
                        if (!callWith(m.word(0x9ef974), { edx, esi, eax }))
                            return;
                    }
                    else
                    {
                        if (!setState(0x2c2, 1))
                            return;
                        eax = m.word(ebx + 0x10);
                        edx = m.word(ebx + 0xc);
                        ecx = m.word(ebx + 8);
                        if (!callWith(m.word(0x9ef974), { eax, edx, ecx }))
                            return;
                        if (!setState(0x2c2, 0))
                            return;
                    }
                }
            }
        }
        // The next record, while it is a triangle.
        ebx = m.word(ebx);
        flags.logic(ebx);
        if (!ebx)
            break;
        eax = (eax & 0xffff0000) | (m.word(ebx + 4) & 0xffff);
        flags.compare16(x86::reg16(eax), 3);
        if (x86::reg16(eax) != 3)
            break;
    }
    flags.store(cpu);
    cpu.eax = ebx;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_4c3ad0: a triangle of sub_434120's polygon list with a corner past the
 * near plane, clipped to it and drawn as a fan.
 *   eax  the record (sub_434120's); dl  its corners' near bits or-ed
 * The corners go to work records at 0x7d35dc, 0x7d3600, 0x7d3624 (listed at
 * 0x5600e0): x, y, 1/w, then s, t times 1/w for the record's texture (+0x20
 * on) and for its second (+0x54 on).  With 0x10 in dl, the corners nearer
 * than [0x7d34f0] are cut away as sub_4bf790 cuts them -- t from 1/(a - b)
 * times (a - near), the second texture's s, t only for a record drawn twice,
 * the made vertices at 0x7d1198 (0x24 bytes), their pointers and codes at
 * 0x7d0e78 and 0x7d0db0.  What is left goes to Glide vertices at 0x7cdbb0 --
 * x, y, s/w, t/w, white, and the second texture's at +0xc, +8 -- a corner
 * with an indefinite x or y left out, and to THRASH_drawtrifan twice: with
 * the record's texture and the blending off, then with its second texture and
 * the blending on for a record drawn twice (sub_431900 setting the state),
 * else again the same.  Returns -1 in eax when fewer than three corners are
 * left, 0 otherwise (also when the first corner is an indefinite and nothing
 * is drawn). */
void nearTriangleNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x3c
    const x86::reg32 frameEbp = entry - 20;
    const x86::reg32 frameEsp = entry - 0x50;
    const x86::reg32 record = cpu.eax;
    const x86::reg8 nearBits = x86::reg8(cpu.edx);
    LastCompare compared;
    IntegerFlags flags;
    flags.load(cpu);

    auto leave = [&](x86::reg32 eax, x86::reg32 edx) {
        compared.replay(cpu);
        flags.store(cpu);
        cpu.eax = eax;
        cpu.edx = edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };

    const x86::reg32 twoPass = m.word(record + 0x50) && (m.byte(record + 7) & 0x40) ? 1 : 0;
    // The corners' codes on the stack, the corners to the work records.
    const x86::reg32 codes = frameEbp - 4;
    {
        const x86::reg32 corners[3] = { m.word(record + 8), m.word(record + 0xc), m.word(record + 0x10) };
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            const x86::reg32 v = corners[i];
            const x86::reg32 to = 0x7d35dc + 0x24 * i;
            const x86::reg32 st = record + 0x20 + 8 * i;
            m.byte(codes + i, m.byte(v + 0x14));
            const double s = M::mul(m.load(st), m.load(v + 0xc));
            const double t = M::mul(m.load(st + 4), m.load(v + 0xc));
            const double s2 = M::mul(m.load(st + 0x34), m.load(v + 0xc));
            const double t2 = M::mul(m.load(st + 0x38), m.load(v + 0xc));
            m.store(to, m.load(v));
            m.store(to + 4, m.load(v + 4));
            m.store(to + 8, m.load(v + 0xc));
            m.store(to + 0xc, s);
            m.store(to + 0x10, t);
            m.store(to + 0x14, s2);
            m.store(to + 0x18, t2);
        }
    }

    x86::reg32 list = 0x5600e0;
    x86::reg32 count = 3;
    flags.logic8(nearBits & 0x10);
    if (nearBits & 0x10)
    {
        const x86::reg32 outList = 0x7d0e78, outCodes = 0x7d0db0;
        x86::reg32 kept = 0;
        x86::reg32 made = 0;
        x86::reg32 previous = 2;
        for (x86::reg32 current = 0; current < 3; previous = current++)
        {
            const x86::reg8 previousCode = m.byte(codes + previous);
            if (!(previousCode & 0x10))
            {
                m.word(outList + 4 * kept, m.word(list + 4 * previous));
                m.byte(outCodes + kept, previousCode);
                ++kept;
            }
            if (!((previousCode ^ m.byte(codes + current)) & 0x10))
                continue;
            const x86::reg32 a = m.word(list + 4 * previous);
            const x86::reg32 b = m.word(list + 4 * current);
            const x86::reg32 v = 0x7d1198 + made * 0x24;
            const double aW = m.load(a + 8);
            const double difference = M::sub(aW, m.load(b + 8));
            const double past = M::sub(aW, m.load(0x7d34f0));
            const double t = double(float(M::mul(M::div(1.0, difference), past)));
            for (const x86::reg32 field : { 0u, 4u, 0xcu, 0x10u })
            {
                const double from = m.load(a + field);
                m.store(v + field, M::sub(from, M::mul(M::sub(from, m.load(b + field)), t)));
            }
            if (twoPass)
            {
                const double s = m.load(a + 0x14);
                m.store(v + 0x14, M::sub(s, M::mul(M::sub(s, m.load(b + 0x14)), t)));
                const double tt = m.load(a + 0x18);
                m.store(v + 0x18, M::sub(tt, M::mul(t, M::sub(tt, m.load(b + 0x18)))));
            }
            m.store(v + 8, m.load(0x7d34f0));
            const x86::reg32 code = outCodes + kept;
            m.byte(code, 0x10);
            const double y = m.load(v + 4);
            const double bottom = m.load(0x7d34e0);
            compared(y, bottom);
            if (!(y >= bottom))
                m.byte(code, m.byte(code) | 8);
            else
            {
                const double top = m.load(0x7d34dc);
                compared(y, top);
                if (y > top)
                    m.byte(code, m.byte(code) | 4);
            }
            const double x = m.load(v);
            const double left = m.load(0x7d34f4);
            compared(x, left);
            if (!(x >= left))
                m.byte(code, m.byte(code) | 1);
            else
            {
                const double right = m.load(0x7d34ec);
                compared(x, right);
                if (x > right)
                    m.byte(code, m.byte(code) | 2);
            }
            m.word(outList + 4 * kept, v);
            ++made;
            ++kept;
        }
        flags.compare(kept, 3);
        if (kept < 3)
        {
            leave(0xffffffff, 3);
            return;
        }
        list = outList;
        count = kept;
        flags.logic(0);  // the dec ecx loop's last test
    }

    // The fan, a corner with an indefinite x or y left out.
    flags.logic(count);
    const x86::reg32 first = m.word(list);
    for (const x86::reg32 field : { 0u, 4u, 8u })
    {
        const x86::reg32 bits = m.word(first + field) | 0x80000000;
        flags.compare(bits, 0xffc00000);
        if (bits == 0xffc00000)
        {
            leave(0, field == 8 ? bits : first);
            return;
        }
    }
    x86::reg32 fan = 0x7cdbb0;
    x86::reg32 drawn = 0;
    x86::reg32 at = list;
    for (x86::reg32 left = count; left; --left)
    {
        const x86::reg32 v = m.word(at);
        const double inverse = double(float(M::div(1.0, m.load(v + 8))));
        m.store(fan, m.load(v));
        m.word(fan + 4, m.word(v + 4));
        m.store(fan + 0x18, M::mul(m.load(v + 0xc), inverse));
        const double t = m.load(v + 0x10);
        m.word(fan + 0x14, 0);
        m.word(fan + 0x10, 0xffffffff);
        m.store(fan + 0x1c, M::mul(t, inverse));
        flags.logic(twoPass);
        if (twoPass)
        {
            m.store(fan + 0xc, M::mul(m.load(v + 0x14), inverse));
            m.store(fan + 8, M::mul(inverse, m.load(v + 0x18)));
        }
        at += 4;
        const x86::reg32 x = m.word(fan) | 0x80000000;
        flags.compare(x, 0xffc00000);
        if (x != 0xffc00000)
        {
            const x86::reg32 y = m.word(fan + 4) | 0x80000000;
            flags.compare(y, 0xffc00000);
            if (y != 0xffc00000)
            {
                flags.add(fan, 0x20);
                fan += 0x20;
                flags.inc(drawn);
                ++drawn;
            }
        }
        flags.dec(left);
    }

    // The state through sub_431900, then the fan to THRASH_drawtrifan.
    compared.replay(cpu);
    flags.store(cpu);
    cpu.eax = 0x68;
    cpu.ebx = record;
    cpu.ecx = at;
    cpu.edx = 0;
    cpu.esi = 0;
    cpu.edi = drawn;
    cpu.ebp = frameEbp;
    cpu.esp = frameEsp;
    if (!call(app, cpu, 0x431900))
        return;
    cpu.eax = 1;
    cpu.edx = m.word(cpu.ebx + 0x18);
    if (!call(app, cpu, 0x431900))
        return;
    auto drawFan = [&]() {
        m.word(frameEsp - 4, 0x7cdbb0);
        cpu.eax = cpu.edi - 2;
        m.word(frameEsp - 8, cpu.eax);
        cpu.esp = frameEsp - 8;
        return call(app, cpu, m.word(0x9ef960));
    };
    cpu.esi = twoPass;
    if (!drawFan())
        return;
    flags.logic(cpu.esi);
    flags.store(cpu);
    if (cpu.esi)
    {
        cpu.edx = 1;
        cpu.eax = 0x68;
        if (!call(app, cpu, 0x431900))
            return;
        cpu.eax = 1;
        cpu.edx = m.word(cpu.ebx + 0x50);
        if (!call(app, cpu, 0x431900))
            return;
        // The second texture's s, t where THRASH reads them.
        IntegerFlags loop;
        loop.load(cpu);
        x86::reg32 i = 0;
        for (;;)
        {
            loop.compare(i, cpu.edi);
            if (x86::sreg32(i) >= x86::sreg32(cpu.edi))
                break;
            const x86::reg32 entryAt = i << 5;
            loop.shl(i, 5);
            cpu.edx = entryAt;
            loop.inc(i);
            ++i;
            const double t = m.load(entryAt + 0x7cdbb8);
            cpu.ecx = m.word(entryAt + 0x7cdbbc);
            m.word(entryAt + 0x7cdbc8, cpu.ecx);
            m.store(entryAt + 0x7cdbcc, t);
        }
        loop.store(cpu);
    }
    if (!drawFan())
        return;
    cpu.eax = 0;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.esi = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_4332b0: the quads of a polygon list to THRASH, from the record in eax
 * for as long as the next one is a quad too (type 0, the word at +4).  A
 * record: the next at +0, its flags in the word at +6, its corners at +8,
 * +0xc, +0x10, +0x14 (a, b, c, d: the game's vertices, clip codes at +0x14),
 * its texture's record at +0x18 (handle at +4, the corners' s, t at +8..+0x24,
 * flags at +0x28).  Flag 1 turns the blending (state 0x68, kept in
 * [0x554e58]) on, flag 2 keeps the corners' s, t as they are; the texture is
 * set (state 1, kept in [0x554e54]), and a texture with 8 in its flags gets
 * sub_432390 with a's colour.  Then the quad goes to sub_433060 (flag 8), to
 * sub_4c1900 (a texture without 0x10 in its flags, or none), to sub_4c19e0
 * (a corner past the near plane), or to THRASH_drawquad -- state 0x2c2 on
 * around it when a corner is off the left.  Those take the corners in eax,
 * edx, ebx, ecx.  Returns the record it stopped at (0 at the end) in eax. */
void quadRecordsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 frameEbp = entry - 24;
    const x86::reg32 frameEsp = entry - 36;
    x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.eax, edi = cpu.edi;
    IntegerFlags flags;

    auto callWith = [&](x86::reg32 target, std::initializer_list<x86::reg32> pushed) -> bool {
        x86::reg32 esp = frameEsp;
        for (const x86::reg32 value : pushed)
        {
            esp -= 4;
            m.word(esp, value);
        }
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = frameEbp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        return true;
    };
    auto setState = [&](x86::reg32 state, x86::reg32 value) { return callWith(m.word(0x9ef96c), { value, state }); };

    for (;;)
    {
        const x86::reg32 b = m.word(esi + 0xc);
        const x86::reg32 d = m.word(esi + 0x14);
        m.word(frameEbp - 8, b);
        m.word(frameEbp - 4, d);
        const x86::reg32 recordFlags = x86::reg32(x86::sreg32(m.word(esi + 4)) >> 16);
        m.word(frameEbp - 0xc, recordFlags);
        edi = m.word(esi + 8);
        ebx = m.word(esi + 0x10);
        eax = (recordFlags & 0xffff00ff) | ((recordFlags & 0xff) << 8);
        flags.logic8(x86::reg8(recordFlags & 1));
        if (recordFlags & 1)
        {
            flags.compare(m.word(0x554e58), 1);
            if (m.word(0x554e58) != 1)
            {
                ecx = 1;
                m.word(0x554e58, ecx);
                if (!setState(0x68, ecx))
                    return;
            }
        }
        else
        {
            flags.compare(m.word(0x554e58), 0);
            if (m.word(0x554e58) != 0)
            {
                edx = 0;
                m.word(0x554e58, edx);
                if (!setState(0x68, edx))
                    return;
            }
        }

        ecx = m.word(esi + 0x18);
        flags.logic(ecx);
        if (ecx)
        {
            flags.logic8(x86::reg8(recordFlags & 2));
            if (!(recordFlags & 2))
            {
                eax = ecx;
                edx = m.word(eax + 8);
                m.word(edi + 0x18, edx);
                eax = m.word(esi + 0x18);
                edx = m.word(eax + 0xc);
                m.word(edi + 0x1c, edx);
                eax = m.word(esi + 0x18);
                m.store(b + 0x18, m.load(eax + 0x10));
                eax = m.word(esi + 0x18);
                m.store(b + 0x1c, m.load(eax + 0x14));
                eax = m.word(esi + 0x18);
                edx = m.word(eax + 0x18);
                m.word(ebx + 0x18, edx);
                eax = m.word(esi + 0x18);
                edx = m.word(eax + 0x1c);
                m.word(ebx + 0x1c, edx);
                eax = m.word(esi + 0x18);
                m.store(d + 0x18, m.load(eax + 0x20));
                eax = m.word(esi + 0x18);
                m.store(d + 0x1c, m.load(eax + 0x24));
                eax = d;
            }
            eax = m.word(esi + 0x18);
            edx = m.word(0x554e54);
            ecx = m.word(eax + 4);
            flags.compare(edx, ecx);
            if (edx != ecx)
            {
                m.word(0x554e54, ecx);
                if (!setState(1, ecx))
                    return;
            }
        }
        else
        {
            flags.compare(m.word(0x554e54), 0);
            if (m.word(0x554e54) != 0)
            {
                eax = 0;
                m.word(0x554e54, eax);
                if (!setState(1, eax))
                    return;
            }
        }

        edx = m.word(esi + 0x18);
        flags.logic(edx);
        if (edx)
        {
            flags.logic8(m.byte(edx + 0x28) & 8);
            if (m.byte(edx + 0x28) & 8)
            {
                eax = m.word(edi + 0x10);
                if (!callWith(0x432390, {}))
                    return;
            }
        }

        const x86::reg8 flagsByte = m.byte(frameEbp - 0xc);
        flags.logic8(flagsByte & 8);
        bool drawn = false;
        if (flagsByte & 8)
        {
            ecx = m.word(frameEbp - 4);
            edx = m.word(frameEbp - 8);
            eax = edi;
            if (!callWith(0x433060, {}))
                return;
            drawn = true;
        }
        if (!drawn)
        {
            ecx = m.word(esi + 0x18);
            flags.logic(ecx);
            bool clip = !ecx;
            if (ecx)
            {
                flags.logic8(m.byte(ecx + 0x28) & 0x10);
                clip = !(m.byte(ecx + 0x28) & 0x10);
            }
            if (clip)
            {
                ecx = m.word(frameEbp - 4);
                edx = m.word(frameEbp - 8);
                eax = edi;
                if (!callWith(0x4c1900, {}))
                    return;
            }
            else
            {
                const x86::reg32 bv = m.word(frameEbp - 8), dv = m.word(frameEbp - 4);
                eax = (m.word(bv + 0x14) & 0x10) | (m.word(edi + 0x14) & 0x10);
                edx = (m.word(ebx + 0x14) & 0x10) | eax;
                eax = (m.word(dv + 0x14) & 0x10) | edx;
                flags.logic(eax);
                if (eax)
                {
                    ecx = dv;
                    edx = bv;
                    eax = edi;
                    if (!callWith(0x4c19e0, {}))
                        return;
                }
                else
                {
                    edx = (m.word(edi + 0x14) & 1) | (m.word(bv + 0x14) & 1);
                    eax = (m.word(ebx + 0x14) & 1) | edx;
                    edx = m.word(dv + 0x14) & 1;
                    eax |= edx;
                    flags.logic(eax);
                    if (!eax)
                    {
                        ecx = dv;
                        const x86::reg32 c = ebx;
                        ebx = bv;
                        if (!callWith(m.word(0x9ef94c), { ecx, c, ebx, edi }))
                            return;
                    }
                    else
                    {
                        if (!setState(0x2c2, 1))
                            return;
                        eax = m.word(frameEbp - 4);
                        edx = m.word(frameEbp - 8);
                        if (!callWith(m.word(0x9ef94c), { eax, ebx, edx, edi }))
                            return;
                        if (!setState(0x2c2, 0))
                            return;
                    }
                }
            }
        }

        // The next record, while it is a quad.
        esi = m.word(esi);
        flags.logic(esi);
        if (!esi)
            break;
        eax = (eax & 0xffff0000) | (m.word(esi + 4) & 0xffff);
        flags.logic16(x86::reg16(eax));
        if (x86::reg16(eax))
            break;
    }
    flags.store(cpu);
    cpu.eax = esi;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_432720: a quad subdivided until it is small enough on the screen.
 *   eax  the depth so far; edx, ebx, ecx, [esp+4]  its corners a, b, c, d
 *   projected (x and y as the projection divides them, then 1/w: three
 *   floats each); [esp+8..+0x14]  the corners themselves (the game's
 *   vertices, 0x20 bytes)
 * When sub_432690 asks for no deeper subdivision than eax, the quad goes to
 * sub_4c19e0 (a corner past the near plane) or THRASH_drawquad (state 0x2c2
 * on around it when a corner is off the left).  Otherwise the midpoints of
 * a b, a d, b c, d c and a c are made -- in the projected space, halved,
 * 1/w of 0 taken as 2^-16, then back to the screen as vertices on the
 * stack with their s, t and colour the corners' averages and a clip code of
 * their own -- and the four quads around the middle go to this function
 * again, each unless its corners share a clip bit.  stdcall: ret 0x14. */
void subdivideQuadNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x100; sub ebp, 0x6e
    const x86::reg32 ebp = entry - 0x7a;
    const x86::reg32 frameEsp = entry - 0x10c;
    x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, edi = cpu.edi;
    IntegerFlags flags;

    m.word(ebp + 0x5e, eax);
    m.word(ebp + 0x6a, edx);
    m.word(ebp + 0x62, ebx);
    m.word(ebp + 0x66, ecx);
    const x86::reg32 sa = edx, sb = ebx, sc = ecx, sd = m.word(entry + 4);
    const x86::reg32 a = m.word(entry + 8), b = m.word(entry + 0xc), c = m.word(entry + 0x10), d = m.word(entry + 0x14);

    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4 + 0x14;
    };
    auto leaveAfterCall = [&]() {
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4 + 0x14;
    };
    auto callWith = [&](x86::reg32 target, std::initializer_list<x86::reg32> pushed) -> bool {
        x86::reg32 esp = frameEsp;
        for (const x86::reg32 value : pushed)
        {
            esp -= 4;
            m.word(esp, value);
        }
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        flags.load(cpu);
        return true;
    };

    eax = a;
    edx = b;
    ebx = c;
    ecx = d;
    if (!callWith(0x432690, {}))
        return;
    const x86::reg32 depth = m.word(ebp + 0x5e);
    flags.compare(eax, depth);
    if (x86::sreg32(eax) <= x86::sreg32(depth))
    {
        // Small enough: drawn.
        edx = (m.word(a + 0x14) & 0x10) | (m.word(b + 0x14) & 0x10);
        edx |= m.word(c + 0x14) & 0x10;
        eax = (m.word(d + 0x14) & 0x10) | edx;
        flags.logic(eax);
        if (eax)
        {
            ecx = d;
            ebx = c;
            edx = b;
            eax = a;
            if (!callWith(0x4c19e0, {}))
                return;
            leaveAfterCall();
            return;
        }
        edx = (m.word(b + 0x14) & 1) | (m.word(a + 0x14) & 1);
        eax = (m.word(c + 0x14) & 1) | edx;
        edx = m.word(d + 0x14) & 1;
        eax |= edx;
        flags.logic(eax);
        if (!eax)
        {
            ecx = d;
            ebx = c;
            esi = b;
            edi = a;
            if (!callWith(m.word(0x9ef94c), { ecx, ebx, esi, edi }))
                return;
            leaveAfterCall();
            return;
        }
        if (!callWith(m.word(0x9ef96c), { 1, 0x2c2 }))
            return;
        eax = d;
        edx = c;
        ecx = b;
        ebx = a;
        if (!callWith(m.word(0x9ef94c), { eax, edx, ecx, ebx }))
            return;
        if (!callWith(m.word(0x9ef96c), { 0, 0x2c2 }))
            return;
        leaveAfterCall();
        return;
    }

    /* A midpoint: projected at `s` from the projected corners p, q, as a
     * vertex at `v` from the corners u, w; 1/w's reciprocal kept at `inverse`. */
    const double half = m.loadDouble(0x537484);
    auto midpoint = [&](x86::reg32 p, x86::reg32 q, x86::reg32 u, x86::reg32 w, x86::reg32 s, x86::reg32 v,
                        x86::reg32 inverse) {
        m.store(s, M::mul(M::add(m.load(p), m.load(q)), half));
        m.store(s + 4, M::mul(M::add(m.load(p + 4), m.load(q + 4)), half));
        m.store(s + 8, M::mul(M::add(m.load(p + 8), m.load(q + 8)), half));
        m.store(v + 0x18, M::mul(M::add(m.load(u + 0x18), m.load(w + 0x18)), half));
        m.store(v + 0x1c, M::mul(half, M::add(m.load(u + 0x1c), m.load(w + 0x1c))));
        m.word(v + 0x10, ((m.word(u + 0x10) >> 1) & 0x7f7f7f7f) + ((m.word(w + 0x10) >> 1) & 0x7f7f7f7f));
        if (!(m.word(s + 8) & 0x7fffffff))
            m.word(s + 8, 0x37800080);
        m.store(inverse, M::div(1.0, m.load(s + 8)));
        const double x = M::mul(m.load(0x56009c), m.load(inverse));
        const double y = M::mul(m.load(0x5600a0), m.load(inverse));
        m.word(v + 0xc, m.word(inverse));
        const double screenY = M::mul(y, m.load(s + 4));
        const double screenX = M::mul(x, m.load(s));
        const x86::reg32 inverseZ = m.word(s + 8);
        m.store(v, M::add(screenX, m.load(0x5600a4)));
        m.word(v + 8, inverseZ);
        m.store(v + 4, M::add(screenY, m.load(0x5600a8)));
        // The clip code, from the bits.
        esi = v;
        edi = v + 0x14;
        eax = m.word(v + 0xc);
        ecx = m.word(0x7d34f8);
        if ((eax & 0x80000000) || x86::sreg32(eax) >= x86::sreg32(ecx))
        {
            edx = inverseZ;
            ebx = 0x10;
        }
        else
        {
            eax = m.word(v + 4);
            ebx = 0;
            ecx = m.word(0x7d3504);
            edx = m.word(0x7d3500);
            if ((eax & 0x80000000) || x86::sreg32(eax) < x86::sreg32(ecx))
                ebx |= 8;
            else if (x86::sreg32(eax) > x86::sreg32(edx))
                ebx |= 4;
            eax = m.word(v);
            ecx = m.word(0x7d34fc);
            edx = m.word(0x7d350c);
            if ((eax & 0x80000000) || x86::sreg32(eax) < x86::sreg32(ecx))
                ebx |= 1;
            else if (x86::sreg32(eax) > x86::sreg32(edx))
                ebx |= 2;
        }
        m.byte(v + 0x14, x86::reg8(ebx));
    };
    const x86::reg32 s1 = ebp + 0xe, s2 = ebp + 0x1a, s3 = ebp + 0x26, s4 = ebp + 0x32, s5 = ebp + 0x3e;
    const x86::reg32 v1 = ebp - 0x92, v2 = ebp - 0x72, v3 = ebp - 0x52, v4 = ebp - 0x32, v5 = ebp - 0x12;
    midpoint(sa, sb, a, b, s1, v1, ebp + 0x4a);
    midpoint(sa, sd, a, d, s2, v2, ebp + 0x4e);
    midpoint(sb, sc, b, c, s3, v3, ebp + 0x52);
    midpoint(sd, sc, d, c, s4, v4, ebp + 0x56);
    midpoint(sa, sc, a, c, s5, v5, ebp + 0x5a);
    auto code = [&](x86::reg32 vertex) { return x86::reg32(m.byte(vertex + 0x14)); };

    // The four quads around the middle, each unless its corners share a bit.
    edx = code(a) & code(v1);
    ecx = depth + 1;
    edx &= code(v5);
    m.word(ebp + 0x5e, ecx);
    eax = code(v2);
    flags.logic(edx & eax);
    if (!(edx & eax))
    {
        ebx = a;
        ecx = s5;
        edx = sa;
        const x86::reg32 cornerA = ebx;
        ebx = s1;
        eax = m.word(ebp + 0x5e);
        if (!callWith(0x432720, { v2, v5, v1, cornerA, s2 }))
            return;
    }
    edx = code(b) & code(v1);
    eax = code(v3) & edx;
    edx = code(v5);
    flags.logic(eax & edx);
    if (!(eax & edx))
    {
        esi = b;
        ecx = s3;
        ebx = sb;
        edx = s1;
        eax = m.word(ebp + 0x5e);
        if (!callWith(0x432720, { v5, v3, esi, v1, s5 }))
            return;
    }
    edx = code(v5) & code(v2);
    ecx = d;
    edx &= code(v4);
    eax = code(d);
    flags.logic(edx & eax);
    if (!(edx & eax))
    {
        ebx = s5;
        edx = s2;
        const x86::reg32 cornerD = ecx;
        ecx = s4;
        eax = m.word(ebp + 0x5e);
        if (!callWith(0x432720, { cornerD, v4, v5, v2, sd }))
            return;
    }
    edx = code(v3) & code(v5);
    ecx = c;
    edx &= code(c);
    eax = code(v4);
    flags.logic(edx & eax);
    if (edx & eax)
    {
        leave();
        return;
    }
    {
        const x86::reg32 cornerC = ecx;
        ebx = s3;
        edx = s5;
        ecx = m.word(ebp + 0x66);
        eax = m.word(ebp + 0x5e);
        if (!callWith(0x432720, { v4, cornerC, v3, v5, s4 }))
            return;
    }
    leaveAfterCall();
}

/* sub_433060: a quad (eax, edx, ebx, ecx: a, b, c, d) drawn whole or
 * subdivided.  Nothing when its corners share a clip bit.  When sub_432690
 * asks for subdivision, the corners are projected -- x, y less the centre
 * ([0x5600a4], [0x5600a8]) over w (+0xc, 2^-16 for 0) times the scale
 * ([0x56009c], [0x5600a0]), and 1/w -- to the stack and the quad goes to
 * sub_432720 at depth 0; otherwise to sub_4c19e0 when a corner is past the
 * near plane, else to THRASH_drawquad (state 0x2c2 around it when a corner
 * is off the left).  eax, ebx, ecx and edx come out as the path leaves them. */
void quadSubdividedNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    const x86::reg32 ebp = entry - 12;
    const x86::reg32 frameEsp = entry - 0x54;
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx, d = cpu.ecx;
    x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = b, edi = d;
    IntegerFlags flags;

    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4;
    };
    auto leaveAfterCall = [&]() {
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 4;
    };
    auto callWith = [&](x86::reg32 target, std::initializer_list<x86::reg32> pushed) -> bool {
        x86::reg32 esp = frameEsp;
        for (const x86::reg32 value : pushed)
        {
            esp -= 4;
            m.word(esp, value);
        }
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        flags.load(cpu);
        return true;
    };

    m.word(ebp - 4, a);
    m.word(ebp - 8, c);
    edx = x86::reg32(m.byte(a + 0x14)) & m.byte(b + 0x14) & m.byte(c + 0x14);
    eax = m.byte(d + 0x14);
    flags.logic(edx & eax);
    if (edx & eax)
    {
        leave();
        return;
    }
    eax = a;
    edx = esi;
    if (!callWith(0x432690, {}))
        return;
    flags.logic(eax);
    if (!eax)
    {
        const x86::reg32 qa = m.word(ebp - 4), qc = m.word(ebp - 8);
        eax = (m.word(esi + 0x14) & 0x10) | (m.word(qa + 0x14) & 0x10);
        eax |= m.word(qc + 0x14) & 0x10;
        edx = m.word(edi + 0x14) & 0x10;
        eax |= edx;
        flags.logic(eax);
        if (eax)
        {
            ebx = qc;
            eax = qa;
            ecx = edi;
            edx = esi;
            if (!callWith(0x4c19e0, {}))
                return;
            leaveAfterCall();
            return;
        }
        edx = (m.word(esi + 0x14) & 1) | (m.word(qa + 0x14) & 1);
        eax = (m.word(qc + 0x14) & 1) | edx;
        edx = m.word(edi + 0x14) & 1;
        eax |= edx;
        flags.logic(eax);
        if (!eax)
        {
            edx = qc;
            ecx = qa;
            if (!callWith(m.word(0x9ef94c), { edi, edx, esi, ecx }))
                return;
            leaveAfterCall();
            return;
        }
        if (!callWith(m.word(0x9ef96c), { 1, 0x2c2 }))
            return;
        ebx = m.word(ebp - 8);
        const x86::reg32 cornerB = esi;
        esi = m.word(ebp - 4);
        if (!callWith(m.word(0x9ef94c), { edi, ebx, cornerB, esi }))
            return;
        if (!callWith(m.word(0x9ef96c), { 0, 0x2c2 }))
            return;
        leaveAfterCall();
        return;
    }

    // Each corner projected: x, y, 1/w.
    auto project = [&](x86::reg32 v, x86::reg32 wAt, x86::reg32 to) {
        eax = m.word(v + 0xc);
        m.word(wAt, eax);
        flags.logic(eax & 0x7fffffff);
        if (!(eax & 0x7fffffff))
            m.word(wAt, 0x37800080);
        const double w = m.load(wAt);
        const double x = M::div(M::sub(m.load(v), m.load(0x5600a4)), M::mul(w, m.load(0x56009c)));
        const double inverse = M::div(1.0, w);
        m.store(to, x);
        const double y = M::div(M::sub(m.load(v + 4), m.load(0x5600a8)), M::mul(w, m.load(0x5600a0)));
        m.store(to + 8, inverse);
        m.store(to + 4, y);
    };
    project(m.word(ebp - 4), ebp - 0x10, ebp - 0x48);
    project(esi, ebp - 0xc, ebp - 0x3c);
    project(m.word(ebp - 8), ebp - 0x14, ebp - 0x30);
    project(edi, ebp - 0x18, ebp - 0x24);
    edx = m.word(ebp - 8);
    ecx = m.word(ebp - 4);
    ebx = ebp - 0x3c;
    eax = 0;
    const x86::reg32 cornerA = ecx;
    edx = ebp - 0x48;
    ecx = ebp - 0x30;
    if (!callWith(0x432720, { edi, m.word(ebp - 8), esi, cornerA, ebp - 0x24 }))
        return;
    leaveAfterCall();
}

/* sub_433e30: the triangles of a polygon list that carry colours of their
 * own, from the record in eax while the next is a triangle too (type 3).  A
 * record: the next at +0, its flags at +7, its corners at +8, +0xc, +0x10;
 * for the first pass (flag 0x80) their colours at +0x40.., the texture at
 * +0x18 and the corners' s, t at +0x20..+0x34; for the second (flag 0x40)
 * the colours at +0x74.., the texture at +0x50 and s, t at +0x54..+0x68.
 * Each pass writes the colours and s, t to the corners and sets the texture
 * (state 1, kept in [0x554e54]) and the blending (state 0x68, kept in
 * [0x554e58]: off for the first pass, on for the second).  A triangle with
 * no clip code goes to THRASH_drawtri -- in the first pass with each
 * corner's depth worked out from its 1/w as sub_4c11b0 does it -- the rest
 * to sub_4c11b0.  Returns the record it stopped at (0 at the end) in eax. */
void colouredTrianglesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 ebp = entry - 24;
    const x86::reg32 frameEsp = entry - 36;
    x86::reg32 eax = cpu.eax, ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.eax, edi = cpu.edi;
    IntegerFlags flags;

    auto callWith = [&](x86::reg32 target, std::initializer_list<x86::reg32> pushed) -> bool {
        x86::reg32 esp = frameEsp;
        for (const x86::reg32 value : pushed)
        {
            esp -= 4;
            m.word(esp, value);
        }
        flags.store(cpu);
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        if (!call(app, cpu, target))
            return false;
        eax = cpu.eax;
        ebx = cpu.ebx;
        ecx = cpu.ecx;
        edx = cpu.edx;
        esi = cpu.esi;
        edi = cpu.edi;
        flags.load(cpu);
        return true;
    };
    auto setState = [&](x86::reg32 state, x86::reg32 value) { return callWith(m.word(0x9ef96c), { value, state }); };
    /* One pass's colours, s, t and texture: colours from `colours`, the
     * texture's record field at `texture`, s, t from `st` on. */
    auto prepare = [&](x86::reg32 colours, x86::reg32 texture, x86::reg32 st) -> bool {
        edx = m.word(esi + 8);
        eax = m.word(esi + colours);
        m.word(edx + 0x10, eax);
        edx = m.word(esi + 0xc);
        eax = m.word(esi + colours + 4);
        m.word(edx + 0x10, eax);
        edx = m.word(esi + 0x10);
        eax = m.word(esi + colours + 8);
        m.word(edx + 0x10, eax);
        ebx = m.word(esi + texture);
        flags.logic(ebx);
        if (ebx)
        {
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
            {
                eax = m.word(esi + corner);
                edx = m.word(esi + st);
                m.word(eax + 0x18, edx);
                eax = m.word(esi + corner);
                edx = m.word(esi + st + 4);
                m.word(eax + 0x1c, edx);
                st += 8;
            }
            eax = m.word(0x554e54);
            edi = m.word(esi + texture);
            flags.compare(eax, edi);
            if (eax != edi)
            {
                m.word(0x554e54, edi);
                if (!setState(1, edi))
                    return false;
            }
        }
        else
        {
            flags.compare(m.word(0x554e54), 0);
            if (m.word(0x554e54) != 0)
            {
                m.word(0x554e54, ebx);
                if (!setState(1, ebx))
                    return false;
            }
        }
        return true;
    };

    for (;;)
    {
        flags.logic8(m.byte(esi + 7) & 0x80);
        if (m.byte(esi + 7) & 0x80)
        {
            flags.compare(m.word(0x554e58), 0);
            if (m.word(0x554e58) != 0)
            {
                ecx = 0;
                m.word(0x554e58, ecx);
                if (!setState(0x68, ecx))
                    return;
            }
            if (!prepare(0x40, 0x18, 0x20))
                return;
            bool clipped = false;
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
            {
                eax = m.word(esi + corner);
                flags.compare(m.word(eax + 0x14), 0);
                if (m.word(eax + 0x14) != 0)
                {
                    clipped = true;
                    break;
                }
            }
            if (!clipped)
            {
                // Each corner's depth: 1 - 1/w (0 when 1/w is negative or past 1), scaled.
                const x86::reg32 local[3] = { ebp - 0xc, ebp - 4, ebp - 8 };
                for (x86::reg32 i = 0; i < 3; ++i)
                {
                    eax = m.word(esi + 8 + 4 * i);
                    const x86::reg32 w = m.word(eax + 0xc);
                    flags.logic8(x86::reg8(w >> 24) & 0x80);
                    bool zero = (w & 0x80000000) != 0;
                    if (!zero)
                    {
                        flags.compare(w, 0x3f800000);
                        zero = x86::sreg32(w) > 0x3f800000;
                    }
                    if (zero)
                    {
                        flags.logic(0);
                        if (i == 0)
                            ebx = 0;
                        else if (i == 1)
                            eax = 0;
                        else
                            ecx = 0;
                        m.word(local[i], 0);
                    }
                    else
                        m.store(local[i], M::sub(1.0, m.load(eax + 0xc)));
                    eax = m.word(esi + 8 + 4 * i);
                    edx = m.word(local[i]);
                    m.word(eax + 8, edx);
                    m.store(eax + 8, M::mul(m.load(0x560094), m.load(eax + 8)));
                    m.store(eax + 8, M::add(m.load(0x560098), m.load(eax + 8)));
                }
                ebx = m.word(esi + 0x10);
                edi = m.word(esi + 0xc);
                eax = m.word(esi + 8);
                if (!callWith(m.word(0x9ef974), { ebx, edi, eax }))
                    return;
            }
            else
            {
                ebx = m.word(esi + 0x10);
                edx = m.word(esi + 0xc);
                eax = m.word(esi + 8);
                if (!callWith(0x4c11b0, {}))
                    return;
            }
        }

        flags.logic8(m.byte(esi + 7) & 0x40);
        if (m.byte(esi + 7) & 0x40)
        {
            flags.compare(m.word(0x554e58), 1);
            if (m.word(0x554e58) != 1)
            {
                ecx = 1;
                m.word(0x554e58, ecx);
                if (!setState(0x68, ecx))
                    return;
            }
            // The second pass's colours go through edx and eax the other way round.
            eax = m.word(esi + 8);
            edx = m.word(esi + 0x74);
            m.word(eax + 0x10, edx);
            edx = m.word(esi + 0xc);
            eax = m.word(esi + 0x78);
            m.word(edx + 0x10, eax);
            edx = m.word(esi + 0x10);
            eax = m.word(esi + 0x7c);
            m.word(edx + 0x10, eax);
            ebx = m.word(esi + 0x50);
            flags.logic(ebx);
            if (ebx)
            {
                x86::reg32 st = 0x54;
                for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
                {
                    eax = m.word(esi + corner);
                    edx = m.word(esi + st);
                    m.word(eax + 0x18, edx);
                    eax = m.word(esi + corner);
                    edx = m.word(esi + st + 4);
                    m.word(eax + 0x1c, edx);
                    st += 8;
                }
                eax = m.word(0x554e54);
                edi = m.word(esi + 0x50);
                flags.compare(eax, edi);
                if (eax != edi)
                {
                    m.word(0x554e54, edi);
                    if (!setState(1, edi))
                        return;
                }
            }
            else
            {
                flags.compare(m.word(0x554e54), 0);
                if (m.word(0x554e54) != 0)
                {
                    m.word(0x554e54, ebx);
                    if (!setState(1, ebx))
                        return;
                }
            }
            bool clipped = false;
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
            {
                eax = m.word(esi + corner);
                const x86::reg8 code = m.byte(eax + 0x14);
                flags.logic8(code);  // cmp byte, 0
                if (code)
                {
                    clipped = true;
                    break;
                }
            }
            ebx = m.word(esi + 0x10);
            if (!clipped)
            {
                edi = m.word(esi + 0xc);
                eax = m.word(esi + 8);
                if (!callWith(m.word(0x9ef974), { ebx, edi, eax }))
                    return;
            }
            else
            {
                edx = m.word(esi + 0xc);
                eax = m.word(esi + 8);
                if (!callWith(0x4c11b0, {}))
                    return;
            }
        }

        esi = m.word(esi);
        flags.logic(esi);
        if (!esi)
            break;
        eax = (eax & 0xffff0000) | (m.word(esi + 4) & 0xffff);
        flags.compare16(x86::reg16(eax), 3);
        if (x86::reg16(eax) != 3)
            break;
    }
    flags.store(cpu);
    cpu.eax = esi;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_47a190: the sky of a view (eax).  Unless [0x55d2a4], sub_479ed0 first;
 * then, unless [0x6fbc2c] is 1 or the view's kind (its first word, the
 * mirror's as the main view's where the port draws it in full) is 1 -- when
 * [0x55d154] gets [0x7d3684] -- the sky by sub_47b850 and, with [0x55d260],
 * sub_4946e0; with [0x55d2a4], sub_479ed0 after.  eax and the registers the
 * callees leave come out as the generated code leaves them. */
void skyNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.edx, cpu.ebp };
    // push edx, ebp; mov ebp, esp
    const x86::reg32 ebp = entry - 8;
    const x86::reg32 view = cpu.eax;
    IntegerFlags flags;
    cpu.edx = view;
    auto callTo = [&](x86::reg32 target) {
        flags.store(cpu);
        cpu.ebp = ebp;
        cpu.esp = ebp;
        if (!call(app, cpu, target))
            return false;
        flags.load(cpu);
        return true;
    };

    flags.compare(m.word(0x55d2a4), 0);
    if (m.word(0x55d2a4) == 0 && !callTo(0x479ed0))
        return;
    flags.compare(m.word(0x6fbc2c), 1);
    bool plain = m.word(0x6fbc2c) == 1;
    if (!plain)
    {
        // port (apply_mirror_detail): clouds and lightning in the mirror too
        const x86::reg32 kind = mirrorAsMain(m.word(cpu.edx));
        flags.compare(kind, 1);
        plain = kind == 1;
    }
    if (plain)
    {
        cpu.eax = m.word(0x7d3684);
        m.word(0x55d154, cpu.eax);
    }
    else
    {
        cpu.eax = cpu.edx;
        if (!callTo(0x47b850))
            return;
        flags.compare(m.word(0x55d260), 0);
        if (m.word(0x55d260) != 0)
        {
            cpu.eax = cpu.edx;
            if (!callTo(0x4946e0))
                return;
        }
    }
    flags.compare(m.word(0x55d2a4), 0);
    if (m.word(0x55d2a4) != 0)
    {
        cpu.eax = cpu.edx;
        if (!callTo(0x479ed0))
            return;
    }
    flags.store(cpu);
    cpu.edx = saved[0];
    cpu.ebp = saved[1];
    cpu.esp = entry + 4;
}

/* sub_47a880: a view's sky, from the pass in eax; the 8 bytes sub_479100
 * leaves at its frame's -0x10 go to esi's address, returned in eax.  The
 * level of detail is (1 - [0x55d268]) * [0x55d1c8] * [0x53b2f8] rounded; then
 * sub_478dc0 (two outputs on the stack), sub_479080, sub_478e10, sub_470b70,
 * and unless sub_47a4c0 says the view has none: for a view of kind 1 (the
 * mirror's as the main view's where the port draws it in full) the dome by
 * sub_47a1f0 or else sub_4790d0, and unless [0x6fbc2c] is 2 the rest --
 * sub_479680, sub_4790d0 at that level, sub_470b20, the sky's parts
 * (sub_47a190), sub_4799d0, sub_479b90 with [0x55d160], sub_4790d0,
 * sub_470ad0.  Everything goes through those calls; the registers as the
 * generated code moves them. */
void viewSkyNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, edi, ebp; mov ebp, esp; sub esp, 0x10
    const x86::reg32 ebp = entry - 20;
    const x86::reg32 frameEsp = entry - 36;
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        flags.store(cpu);
        cpu.ebp = ebp;
        cpu.esp = frameEsp;
        return call(app, cpu, target);
    };

    cpu.ecx = cpu.esi;
    cpu.esi = cpu.eax;
    const double detail = M::mul(M::mul(M::sub(1.0, m.load(0x55d268)), m.load(0x55d1c8)), m.loadDouble(0x53b2f8));
    cpu.ebx = ebp - 4;
    const x86::reg32 level = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(detail)));
    m.word(frameEsp - 4, level);
    cpu.edi = level;
    cpu.eax = cpu.esi;
    cpu.edx = ebp - 8;
    if (!callTo(0x478dc0) || !callTo(0x479080))
        return;
    cpu.eax = cpu.esi;
    if (!callTo(0x478e10))
        return;
    cpu.edx = m.word(ebp - 4);
    cpu.eax = m.word(ebp - 8);
    if (!callTo(0x470b70))
        return;
    cpu.eax = cpu.esi;
    if (!callTo(0x47a4c0))
        return;
    flags.logic(cpu.eax);
    bool done = cpu.eax != 0;
    if (!done)
    {
        // port (apply_mirror_detail): the sky's dome rather than a flat sky in the mirror too
        const x86::reg32 kind = mirrorAsMain(m.word(cpu.esi));
        flags.compare(kind, 1);
        if (kind == 1)
        {
            cpu.eax = cpu.esi;
            if (!callTo(0x47a1f0))
                return;
            flags.logic(cpu.eax);
            done = cpu.eax != 0;
            if (!done)
            {
                cpu.eax = cpu.esi;
                if (!callTo(0x4790d0))
                    return;
            }
        }
    }
    if (!done)
    {
        flags.compare(m.word(0x6fbc2c), 2);
        done = m.word(0x6fbc2c) == 2;
    }
    if (!done)
    {
        cpu.eax = cpu.esi;
        if (!callTo(0x479680))
            return;
        cpu.eax = cpu.esi;
        cpu.edx = cpu.edi;
        if (!callTo(0x4790d0))
            return;
        cpu.eax = m.word(ebp - 4);
        if (!callTo(0x470b20))
            return;
        cpu.eax = cpu.esi;
        if (!callTo(0x47a190))
            return;
        cpu.eax = cpu.esi;
        if (!callTo(0x4799d0))
            return;
        flags.compare(m.word(0x55d160), 0);
        if (m.word(0x55d160) != 0)
        {
            cpu.eax = cpu.esi;
            if (!callTo(0x479b90))
                return;
        }
        cpu.eax = cpu.esi;
        if (!callTo(0x4790d0))
            return;
        cpu.eax = m.word(ebp - 4);
    }
    cpu.esi = ebp - 0x10;
    if (!done && !callTo(0x470ad0))
        return;
    if (!callTo(0x479100))
        return;
    // movsd twice: the result to [ecx]
    cpu.esi = ebp - 0x10;
    cpu.edi = cpu.ecx;
    for (int i = 0; i < 2; ++i)
    {
        m.word(cpu.edi, m.word(cpu.esi));
        const x86::reg32 step = cpu.flags.df ? x86::reg32(-4) : 4;
        cpu.edi += step;
        cpu.esi += step;
    }
    cpu.eax = cpu.ecx;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_47a1f0: the sky as one quad over the whole screen, for a flat sky.  A
 * polygon record of 0xa0 bytes is allocated (sub_4bbf80, [0x6ff9f8] its
 * texture); four vertices on the stack -- the corners of the screen's rect
 * at [0x6fdc40..4c] (integers), 1500 deep, 1/z 0.000667 -- get their clip
 * codes, and go to sub_478cb0 by value with the texture four times.
 * Returns the record allocation's second word ([ebp+0x7a]: 0 unless the
 * allocator wrote it). */
void flatSkyNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x8c; sub ebp, 0x82
    const x86::reg32 ebp = entry - 0x9a;
    const x86::reg32 frameEsp = entry - 0xa4;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto leave = [&]() {
        cpu.eax = m.word(ebp + 0x7a);
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };

    r.edx = 0;
    r.eax = m.word(0x6ff9f8);
    m.word(ebp + 0x7a, r.edx);
    m.word(ebp + 0x7e, r.eax);
    r.edx = 0xa0;
    r.eax = ebp + 0x76;
    if (!callWithRegisters(app, cpu, flags, r, ebp, frameEsp, 0x4bbf80))
        return;
    flags.logic(r.eax);
    if (!r.eax)
    {
        flags.store(cpu);
        leave();
        return;
    }
    // The four corners: (B, C), (A, C), (A, D), (B, D) of the rect A, B, C, D.
    const float a = float(x86::sreg32(m.word(0x6fdc40))), b = float(x86::sreg32(m.word(0x6fdc44)));
    const float c = float(x86::sreg32(m.word(0x6fdc48))), d = float(x86::sreg32(m.word(0x6fdc4c)));
    const x86::reg32 v[4] = { ebp - 0xa, ebp + 0x16, ebp + 0x36, ebp + 0x56 };
    const float xy[4][2] = { { b, c }, { a, c }, { a, d }, { b, d } };
    for (int i = 0; i < 4; ++i)
    {
        app->getMemory<float>(v[i]) = xy[i][0];
        app->getMemory<float>(v[i] + 4) = xy[i][1];
        m.word(v[i] + 8, 0x44bb8000);
        m.word(v[i] + 0xc, 0x3a2ec33e);
    }
    r.ecx = 0x44bb8000;
    r.ebx = 0x3a2ec33e;
    for (int i = 0; i < 4; ++i)
        screenCode(m, v[i], v[i] + 0x14, r);
    // push 0, the texture four times, then the vertices by value, v[3] first.
    x86::reg32 esp = frameEsp;
    esp -= 4;
    m.word(esp, 0);
    r.esi = m.word(ebp + 0x7e);
    for (int i = 0; i < 4; ++i)
    {
        esp -= 4;
        m.word(esp, r.esi);
    }
    for (int i = 3; i >= 0; --i)
    {
        esp -= 0x20;
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            m.word(esp + k, m.word(v[i] + k));
        r.esi = v[i] + 0x20;
        r.edi = esp + 0x20;
    }
    r.ecx = 0;
    r.eax = m.word(ebp + 0x76);
    if (!callWithRegisters(app, cpu, flags, r, ebp, esp, 0x478cb0))
        return;
    leave();
}

/* sub_47b850: the sky's dome, from the view in eax, when [0x55d1f4] asks for
 * one: sub_47b5c0 makes the 13 x 13 vertices at 0x6fdc50 (0x1a0 a row),
 * sub_47b4f0 projects them from the view's position scaled and moved
 * ([0x55d208], [0x55d214], [0x55d218]), sub_47b3a0 with [0x55d250], the
 * sun's glare by sub_47b7d0 when the sky is the textured one and [0x6fbc38]
 * is below 1 (the port lets it through on this driver too); then a list of
 * quad records (allocated by sub_4bbe40, 0x20 each) for the cells whose four
 * corners share no clip bit, flags 2 or 3 (4 more without [0x55d204]), the
 * texture record 0x55d21c, closed by sub_4bbde0 and drawn by sub_4bbf40. */
void skyDomeNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x30
    const x86::reg32 ebp = entry - 24;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.eax, cpu.edi };
    IntegerFlags flags;
    LastCompare compared;
    x86::reg32 esp = entry - 0x48;
    auto leave = [&]() {
        compared.replay(cpu);
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    // A call; the callee decides where esp is after it.
    auto callTo = [&](x86::reg32 target) {
        compared.replay(cpu);
        compared.forget();
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };

    r.edx = m.word(0x55d1f4);
    flags.logic(r.edx);
    if (!r.edx)
    {
        leave();
        return;
    }
    flags.compare(r.edx, 1);
    r.edx = r.edx == 1 ? 3 : 2;
    r.ebx = m.word(0x55d204);
    m.word(ebp - 0x14, r.edx);
    flags.logic(r.ebx);
    if (!r.ebx)
    {
        r.edx |= 4;
        m.word(ebp - 0x14, r.edx);
    }
    if (!callTo(0x47b5c0))
        return;
    const double scale = m.load(0x55d208);
    const double across = M::add(M::mul(m.load(r.esi + 8), scale), m.load(0x55d214));
    const double up = M::add(M::mul(m.load(r.esi + 0x10), scale), m.load(0x55d218));
    esp -= 4;
    r.ecx = 0x6ff170;
    m.store(esp, up);
    m.store(ebp - 0x24, across);
    esp -= 4;
    m.word(esp, m.word(ebp - 0x24));
    r.ebx = 0x6ff9fc;
    esp -= 4;
    m.word(esp, 0x6fdc50);
    r.edx = 0xa9;
    r.eax = r.esi;
    if (!callTo(0x47b4f0))
        return;
    flags.compare(m.word(0x55d250), 0);
    if (m.word(0x55d250) != 0 && !callTo(0x47b3a0))
        return;
    flags.compare(m.word(0x55d1f4), 1);
    if (m.word(0x55d1f4) == 1)
    {
        // port (apply_alpha_intensity): test byte [0x7a3a58], 0x40 as if set
        flags.logic8(0x40);
        flags.store(cpu);  // sahf below takes CF, ZF, SF over; OF stays
        const double glare = m.load(0x6fbc38);
        compared(1.0, glare);
        if (1.0 > glare)
        {
            r.edx = 0x6fdc50;
            r.eax = 0xa9;
            if (!callTo(0x47b7d0))
                return;
        }
    }
    r.edx = 0x1520;
    r.eax = ebp - 0x28;
    if (!callTo(0x4bbe40))
        return;
    flags.logic(r.eax);
    if (!r.eax)
    {
        leave();
        return;
    }
    r.ecx = m.word(ebp - 0x28);
    r.ebx = 0;
    r.esi = 0;
    r.edx = 0;
    flags.logic(0);
    const x86::reg32 grid = 0x6fdc50;
    for (;;)
    {
        // The cell at row edx, column esi.
        const x86::reg32 column = r.esi << 5;
        m.word(ebp - 0x20, column);
        const x86::reg32 v00 = grid + 0x1a0 * r.edx + column, v01 = v00 + 0x20;
        const x86::reg32 v10 = grid + 0x1a0 * (r.edx + 1) + column, v11 = v10 + 0x20;
        m.word(ebp - 8, v00);
        m.word(ebp - 0x1c, column + 0x20);
        m.word(ebp - 0xc, v01);
        m.word(ebp - 4, v10);
        m.word(ebp - 0x10, v11);
        const x86::reg32 shared3 = x86::reg32(m.byte(v00 + 0x14)) & m.byte(v10 + 0x14) & m.byte(v11 + 0x14);
        m.word(ebp - 0x1c, shared3);
        m.word(ebp - 0x18, m.byte(v01 + 0x14));
        r.eax = shared3;
        r.edi = m.byte(v01 + 0x14);
        flags.logic(r.eax & r.edi);
        if (!(r.eax & r.edi))
        {
            const x86::reg32 record = r.ecx;
            r.eax = record + 0x20;
            r.ecx = record + 0x20;
            m.word(record + 0x18, 0x55d21c);
            m.word(record + 4, m.word(record + 4) & 0xffff0000);
            m.word(record, r.eax);
            r.eax = m.word(ebp - 0x14);
            app->getMemory<x86::reg16>(record + 6) = x86::reg16(r.eax);
            r.eax = v00;
            m.word(record + 8, r.eax);
            r.eax = v10;
            m.word(record + 0xc, r.eax);
            r.eax = v11;
            m.word(record + 0x10, r.eax);
            r.eax = v01;
            ++r.ebx;
            m.word(record + 0x14, r.eax);
        }
        ++r.edx;
        flags.compare(r.edx, 0xc);
        if (x86::sreg32(r.edx) < 0xc)
            continue;
        ++r.esi;
        flags.compare(r.esi, 0xc);
        if (x86::sreg32(r.esi) >= 0xc)
            break;
        r.edx = 0;
        flags.logic(0);
    }
    flags.logic(r.ebx);
    if (!r.ebx)
    {
        leave();
        return;
    }
    r.edx = r.ebx << 5;
    r.eax = ebp - 0x28;
    r.ecx -= 0x20;
    if (!callTo(0x4bbde0))
        return;
    esp -= 4;
    m.word(esp, r.ecx);
    r.eax = m.word(ebp - 0x28);
    m.word(ebp - 0x2c, r.ecx);
    esp -= 4;
    m.word(esp, r.eax);
    m.word(ebp - 0x30, r.eax);
    m.word(r.ecx, 0);
    if (!callTo(0x4bbf40))
        return;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_4946e0: the lightning, from the view in eax.  Of the 240 bolts at
 * 0x79c570 (0x30 each), each one alive (its first word) gets a colour --
 * pale blue at alpha min(255, life * 510 / [0x55e4ac]), black for a bolt
 * with +4 set -- and its start (+0x10) and each of its points (pointers at
 * +0x20 on, count at +0x1c, the point at +0x10 of each) turned by the view's
 * matrix (+0x44) and projected with their clip codes; every segment whose
 * ends share no clip bit becomes a quad record (sub_4bbf80, 0xa0 bytes,
 * flags 5) of the start twice and the point twice, the second of each moved
 * a pixel right and down. */
void lightningNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x7c
    const x86::reg32 ebp = entry - 24;
    const x86::reg32 frameEsp = entry - 0x94;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;

    // A point at `p` (3 floats) turned by the view's matrix into `s`.
    auto turn = [&](x86::reg32 p, x86::reg32 s) {
        const x86::reg32 matrix = m.word(ebp - 0x10) + 0x44;
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            const double sum = M::add(M::mul(m.load(p), m.load(matrix + 4 * i)),
                                      M::mul(m.load(p + 4), m.load(matrix + 0xc + 4 * i)));
            m.store(s + 4 * i, M::add(sum, M::mul(m.load(p + 8), m.load(matrix + 0x18 + 4 * i))));
        }
    };

    m.word(ebp - 0x10, r.eax);
    r.edx = 0;
    if (!callWithRegisters(app, cpu, flags, r, ebp, frameEsp, 0x494630))
        return;
    m.word(ebp - 0xc, r.edx);
    for (bool first = true;; first = false)
    {
        if (!first)
        {
            m.word(ebp - 0xc, m.word(ebp - 0xc) + 1);
            flags.compare(m.word(ebp - 0xc), 0xf0);
            if (x86::sreg32(m.word(ebp - 0xc)) >= 0xf0)
                break;
        }
        r.eax = m.word(ebp - 0xc);
        r.esi = r.eax * 0x30;
        r.eax = 0x79c570 + r.esi;
        m.word(ebp - 0x20, r.eax);
        r.ebx = m.word(r.eax);
        flags.logic(r.ebx);
        if (!r.ebx)
            continue;
        const x86::reg32 bolt = r.eax;
        r.eax = r.ebx * 510;
        r.esi = m.word(0x55e4ac);
        {
            const x86::sreg64 dividend = x86::sreg32(r.eax);
            r.edx = x86::reg32(x86::sreg32(dividend % x86::sreg32(r.esi)));
            r.eax = x86::reg32(x86::sreg32(dividend / x86::sreg32(r.esi)));
        }
        flags.compare(r.eax, 0xff);
        if (x86::sreg32(r.eax) >= 0xff)
            r.eax = 0xff;
        r.eax = (r.eax << 24) | 0xe2e2ff;
        m.word(ebp - 4, r.eax);
        flags.compare(m.word(bolt + 4), 0);
        if (m.word(bolt + 4) != 0)
        {
            r.eax = 0;
            m.word(ebp - 4, 0);
        }
        // The start.
        turn(bolt + 0x10, ebp - 0x3c);
        r.ebx = ebp - 0x3c;
        r.edx = m.word(ebp - 0x34);
        flags.logic(r.edx & 0x7fffffff);
        r.ecx = ebp - 0x24;
        r.eax = ebp - 0x3c;
        projectPoint(m, ebp - 0x3c, ebp - 0x24, ebp - 0x7c, r);
        r.ebx = ebp - 0x7c;
        screenCode(m, ebp - 0x7c, ebp - 0x68, r);
        r.ebx = 0;
        m.word(ebp - 8, 0);
        for (;;)
        {
            r.edx = m.word(ebp - 0x20);
            r.eax = m.word(ebp - 8);
            flags.compare(r.eax, m.word(r.edx + 0x1c));
            if (x86::sreg32(r.eax) >= x86::sreg32(m.word(r.edx + 0x1c)))
                break;
            const x86::reg32 point = m.word(r.edx + 0x20 + 4 * r.eax) + 0x10;
            turn(point, ebp - 0x30);
            r.eax = m.word(ebp - 0x28);
            flags.logic(r.eax & 0x7fffffff);
            projectPoint(m, ebp - 0x30, ebp - 0x18, ebp - 0x5c, r);
            screenCode(m, ebp - 0x5c, ebp - 0x48, r);
            r.edx = m.byte(ebp - 0x68);
            r.eax = m.byte(ebp - 0x48);
            flags.logic(r.edx & r.eax);
            bool made = false;
            if (!(r.edx & r.eax))
            {
                r.edx = 0xa0;
                r.eax = ebp - 0x1c;
                if (!callWithRegisters(app, cpu, flags, r, ebp, frameEsp, 0x4bbf80))
                    return;
                flags.logic(r.eax);
                made = r.eax != 0;
            }
            if (made)
            {
                const x86::reg32 record = m.word(ebp - 0x1c);
                m.word(ebp - 0x14, record + 0x60);
                app->getMemory<x86::reg16>(record + 4) = 0;
                m.word(record, 0);
                app->getMemory<x86::reg16>(record + 6) = 5;
                m.word(record + 0x18, 0);
                m.word(record + 8, record + 0x20);
                m.word(record + 0xc, record + 0x40);
                m.word(record + 0x10, record + 0x60);
                m.word(record + 0x14, record + 0x80);
                auto copy = [&](x86::reg32 to, x86::reg32 from) {
                    for (x86::reg32 k = 0; k < 0x20; k += 4)
                        m.word(to + k, m.word(from + k));
                };
                copy(record + 0x40, ebp - 0x7c);
                copy(record + 0x20, record + 0x40);
                copy(record + 0x80, ebp - 0x5c);
                copy(record + 0x60, record + 0x80);
                const x86::reg32 colour = m.word(ebp - 4);
                m.word(record + 0x30, colour);
                m.word(record + 0x50, colour);
                m.word(record + 0x70, colour);
                m.word(record + 0x90, colour);
                m.store(record + 0x40, M::add(1.0, m.load(record + 0x40)));
                m.store(record + 0x44, M::add(1.0, m.load(record + 0x44)));
                m.store(record + 0x60, M::add(1.0, m.load(record + 0x60)));
                m.store(record + 0x64, M::add(1.0, m.load(record + 0x64)));
                r.eax = record + 0x40;
                r.ebx = record + 0x60;
                screenCode(m, record + 0x40, record + 0x54, r);
                screenCode(m, record + 0x60, record + 0x74, r);
            }
            const x86::reg32 next = m.word(ebp - 8) + 1;
            flags.inc(m.word(ebp - 8));
            m.word(ebp - 8, next);
        }
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}


/* sub_482620: the start's countdown, while [0x7d3684] is between 0x120 and
 * 0x220 and [0x552858] (the number) at most 3: sub_475b50's lettering
 * (record 3, [0x55b088]) at the screen's middle (sub_4beb80), the number
 * printed by sub_4df690 ("%d" at 0x53b5d4) or the word of sub_4d1850(0x150)
 * for 0, centred by sub_476fb0's width, turned by sub_4ea9e0's matrix (a
 * quarter of a turn per 64 ticks, or 0.05) and sized by the ticks
 * ([0x53b5dc], [0x53b5e0] once more on [0x6fd3b0] 1, [0x53b5e4] for the
 * word), its height from the screen's height ([0x53b5d8], 40 or 80). */
void countdownNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x6c
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = entry - 24 - 0x6c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };

    // rep movsd: the 9 words at 0x47bb94 to ebp - 0x6c
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x6c + k, m.word(0x47bb94 + k));
    r.ecx = 0;
    r.edi = ebp - 0x48;
    r.esi = 0x47bbb8;
    r.edx = m.word(0x7d3684);
    flags.compare(r.edx, 0x220);
    if (x86::sreg32(r.edx) >= 0x220)
    {
        leave();
        return;
    }
    flags.compare(r.edx, 0x120);
    if (x86::sreg32(r.edx) <= 0x120)
    {
        leave();
        return;
    }
    flags.compare8(m.byte(0x552858), 3);
    if (m.byte(0x552858) > 3)
    {
        leave();
        return;
    }
    m.word(0x55b088, 3);
    r.eax = ebp - 0x18;
    r.ebx = ebp - 0x20;
    r.ecx = ebp - 0x1c;
    r.edx = ebp - 0x24;
    if (!callTo(0x4beb80))
        return;
    // Half a width and a height, rounded toward zero (sar edx, 31; sub; sar 1).
    auto half = [](x86::reg32 value) { return x86::reg32(x86::sreg32(value - x86::reg32(x86::sreg32(value) >> 31)) >> 1); };
    r.eax = m.word(ebp - 0x20);
    r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
    m.word(ebp - 4, r.eax);
    r.eax = half(r.eax);
    m.store(ebp - 8, M::mul(double(x86::sreg32(m.word(ebp - 4))), m.load(0x53b5d8)));
    r.ebx = m.word(ebp - 0x18) + r.eax;
    r.esi = m.word(0x6fd3b0);
    flags.compare(r.esi, 1);
    r.eax = r.esi == 1 ? 0x28 : 0x50;
    m.word(ebp - 4, r.eax);
    r.eax = m.word(ebp - 0x1c);
    r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
    r.eax = half(r.eax);
    r.edi = m.word(ebp - 0x24);
    const double height = M::mul(double(x86::sreg32(m.word(ebp - 4))), m.load(ebp - 8));
    r.eax += r.edi;
    m.word(ebp - 4, r.eax);
    const double top = M::sub(double(x86::sreg32(m.word(ebp - 4))), height);
    setByte(r.edx, 0, m.byte(0x552858));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(top);
    if (!callTo(0x4dfd56))
        return;
    m.word(ebp - 0x10, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
    flags.logic8(x86::reg8(r.edx));
    cpu.fpu.count -= 1;
    if (x86::reg8(r.edx))
    {
        r.eax = (0x200 - m.word(0x7d3684)) & 0x3f;
        m.word(ebp - 4, r.eax);
        const double turn = M::mul(double(x86::sreg32(m.word(ebp - 4))), m.load(0x53b5dc));
        m.store(0x679014, turn);
        const double size = M::mul(turn, m.load(ebp - 8));
        const double angle = M::mul(-m.load(0x679014), m.load(0x53b5e0));
        r.ecx = m.word(0x6fd3b0);
        m.store(0x679014, size);
        m.store(ebp - 0xc, angle);
        flags.compare(r.ecx, 1);
        if (r.ecx == 1)
            m.store(0x679014, M::mul(m.load(0x679014), m.load(0x53b5e0)));
        r.eax = m.byte(0x552858);
        push(r.eax);
        push(0x53b5d4);
        r.eax = ebp - 0x14;
        push(r.eax);
        if (!callTo(0x4df690))
            return;
        esp += 0xc;
        r.eax = ebp - 0x14;
        r.edx = r.ebx;
        if (!callTo(0x476fb0))
            return;
        r.eax = x86::reg32(x86::sreg32(r.eax) >> 1);
        r.edx -= r.eax;
        r.eax = ebp - 0x48;
        push(r.eax);
        r.eax = ebp - 0x6c;
        push(m.word(ebp - 0xc));
        if (!callTo(0x4ea9e0))
            return;
        r.eax = ebp - 0x48;
        r.ebx = m.word(ebp - 0x12);
        push(r.eax);
        r.ebx = x86::reg32(x86::sreg32(r.ebx) >> 16);
        push(0);
        r.edx = x86::reg32(x86::sreg16(x86::reg16(r.edx)));
        push(m.word(0x679014));
        r.eax = ebp - 0x14;
        push(0x41f00000);
        if (!callTo(0x475b50))
            return;
        leave();
        return;
    }
    r.eax = (m.word(0x7d3684) - 0x200) & 7;
    m.word(ebp - 4, r.eax);
    const double grown = M::add(M::mul(double(x86::sreg32(m.word(ebp - 4))), m.load(0x53b5dc)), m.load(0x53b5e4));
    const double size = M::mul(grown, m.load(ebp - 8));
    r.esi = m.word(0x6fd3b0);
    m.store(0x679014, size);
    flags.compare(r.esi, 1);
    if (r.esi == 1)
        m.store(0x679014, M::mul(m.load(0x679014), m.load(0x53b5e0)));
    r.eax = 0x150;
    if (!callTo(0x4d1850) || !callTo(0x476fb0))
        return;
    r.edx = r.ebx;
    r.eax = x86::reg32(x86::sreg32(r.eax) >> 1);
    r.edx -= r.eax;
    r.eax = ebp - 0x48;
    push(r.eax);
    r.eax = ebp - 0x6c;
    push(0x3d4ccccd);
    if (!callTo(0x4ea9e0))
        return;
    r.eax = ebp - 0x48;
    push(r.eax);
    r.ebx = m.word(ebp - 0x12);
    push(0);
    r.ebx = x86::reg32(x86::sreg32(r.ebx) >> 16);
    push(m.word(0x679014));
    r.eax = 0x150;
    push(0x41f00000);
    r.edx = x86::reg32(x86::sreg16(x86::reg16(r.edx)));
    if (!callTo(0x4d1850) || !callTo(0x475b50))
        return;
    leave();
}

/* sub_4b9bc0: a car's lights (car in eax, view in edx; the mirror counts as
 * the main view, a port): its position (+0x910) and matrix (sub_4206f0),
 * nothing when the view is 1 or the car is behind or more than 100 ahead
 * (sub_4e0430 into ebp - 0x28, z against 100 and -[view+0x30]); then the
 * lights' state (sub_49c410, sub_49d640, the night set by sub_4b97b0 and
 * sub_4b9550 when [0x6fd4c8]), sub_49d750 .. sub_49d780, the glare
 * (sub_49d5c0 with the position raised by [view+0x2c]), the shadow
 * (sub_4c7380 without [0x7a3a60]), sub_49c420, and with [0x6fd4ac] 8 and
 * [view+0x34] the headlights' beam on the road (sub_492370) when the car
 * faces the camera (sub_4e01f0 below 0), its strength the dot times
 * [0x5400f8]. */
void carLightsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x4c
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = entry - 20 - 0x4c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // fcomp, fnstsw ax, sahf
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };

    m.word(ebp - 8, r.eax);
    m.word(ebp - 4, r.edx);
    for (x86::reg32 k = 0; k < 0xc; k += 4)
        m.word(ebp - 0x40 + k, m.word(r.eax + 0x910 + k));
    r.edi = ebp - 0x34;
    r.esi = r.eax + 0x91c;
    r.edx = 1;
    r.eax += 0xc;
    if (!callTo(0x4206f0))
        return;
    r.esi = r.eax;
    r.edi = r.eax;
    r.eax = m.word(ebp - 4);
    // port (apply_mirror_detail): the mirror as the main view
    flags.compare(mirrorAsMain(m.word(r.eax)), 1);
    if (mirrorAsMain(m.word(r.eax)) == 1)
    {
        leave();
        return;
    }
    r.eax = ebp - 0x28;
    r.ecx = m.word(ebp - 4) + 0x38;
    r.ebx = m.word(ebp - 4) + 0x44;
    r.edx = ebp - 0x40;
    push(r.eax);
    r.eax = 1;
    if (!callTo(0x4e0430))
        return;
    flags.compare(m.word(ebp - 0x20), 0x42c80000);
    if (x86::sreg32(m.word(ebp - 0x20)) > 0x42c80000)
    {
        leave();
        return;
    }
    r.eax = m.word(0x55fe2c);
    {
        const double behind = -m.load(r.eax + 0x30);
        const double z = m.load(ebp - 0x20);
        compareToFlags(behind, z);
        if (behind > z)
        {
            leave();
            return;
        }
    }
    if (!callTo(0x49c410) || !callTo(0x49d640))
        return;
    flags.compare(m.word(0x6fd4c8), 0);
    if (m.word(0x6fd4c8) != 0)
    {
        r.eax = m.word(ebp - 4);
        // port (apply_mirror_detail): the mirror as the main view
        flags.compare(mirrorAsMain(m.word(r.eax)), 1);
        if (mirrorAsMain(m.word(r.eax)) != 1)
        {
            r.eax = m.word(ebp - 8);
            r.edx = r.esi;
            if (!callTo(0x4b97b0))
                return;
            r.edx = r.eax;
            r.eax = m.word(ebp - 8);
            if (!callTo(0x4b9550))
                return;
            r.edi = r.eax;
        }
    }
    flags.compare(m.word(0x6fbc20), 0);
    r.eax = m.word(0x6fbc20) != 0 ? 1 : 0;
    if (!callTo(0x49d750))
        return;
    r.eax = 0x55d2a8;
    r.edx = 0x7f7f7f7f;
    if (!callTo(0x49d760))
        return;
    r.eax = r.edi;
    r.ecx = ebp - 0x4c;
    if (!callTo(0x49d8f0))
        return;
    r.eax = 0x4f4f4f4f;
    r.ebx = ebp - 0x40;
    if (!callTo(0x49d800))
        return;
    r.edx = m.word(ebp - 4) + 8;
    r.eax = 1;
    if (!callTo(0x4e0000))
        return;
    r.eax = ebp - 0x4c;
    if (!callTo(0x49d780))
        return;
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.eax = 0;
        if (!callTo(0x49d750))
            return;
        r.eax = 0xffffffff;
        if (!callTo(0x49d8f0))
            return;
    }
    r.eax = m.word(0x55fe2c);
    const double raised = M::add(m.load(ebp - 0x3c), m.load(r.eax + 0x2c));
    r.ecx = m.word(ebp - 4);
    r.edx = m.word(0x563a90);
    m.store(ebp - 0x3c, raised);
    r.ebx = m.word(r.edx + 4);
    r.edx = m.word(ebp - 4);
    push(r.ebx);
    r.edx += 0x38;
    r.ecx += 0x44;
    push(r.edx);
    r.edx = m.word(ebp - 8) + 0x8ec;
    r.ebx = ebp - 0x40;
    r.esi = ebp - 0x1c;
    if (!callTo(0x49d5c0))
        return;
    app->getMemory<float>(ebp - 0x10) = float(cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    if (!callTo(0x49c470))
        return;
    flags.compare(m.word(0x7a3a60), 0);
    if (m.word(0x7a3a60) == 0)
    {
        r.edi = m.word(ebp - 0x18);
        push(r.edi);
        r.eax = m.word(ebp - 0x1c);
        push(r.eax);
        r.esi = ebp - 0x1c;
        if (!callTo(0x4c7380))
            return;
    }
    const double nearer = M::sub(m.load(ebp - 0x20), m.load(0x7dcfe0));
    esp -= 4;
    r.edx = ebp - 0x1c;
    r.eax = m.word(ebp - 4);
    m.store(esp, nearer);
    if (!callTo(0x49c420))
        return;
    flags.compare(m.word(0x6fd4ac), 8);
    if (m.word(0x6fd4ac) != 8)
    {
        leave();
        return;
    }
    r.edx = m.word(0x55fe2c);
    flags.compare(m.word(r.edx + 0x34), 0);
    if (m.word(r.edx + 0x34) == 0)
    {
        leave();
        return;
    }
    r.eax = ebp - 0x34;
    r.ebx = m.word(ebp - 8);
    r.ecx = ebp - 0x40;
    r.edx += 0x38;
    push(r.eax);
    r.ebx += 0x8ec;
    r.eax = 1;
    if (!callTo(0x4e0430))
        return;
    r.edx = m.word(ebp - 4) + 0x2c;
    r.eax = m.word(ebp - 8);
    flags.add(r.eax, 0x904);
    r.eax += 0x904;
    if (!callTo(0x4e01f0))
        return;
    app->getMemory<float>(ebp - 0xc) = float(cpu.fpu.st(0));
    compareToFlags(0.0, m.load(ebp - 0xc));
    cpu.fpu.count -= 1;
    if (!(0.0 > m.load(ebp - 0xc)))
    {
        leave();
        return;
    }
    const double strength = M::mul(-m.load(ebp - 0xc), m.loadDouble(0x5400f8));
    push(0);
    push(m.word(ebp - 0x10));
    m.store(ebp - 0x14, strength);
    push(m.word(ebp - 0x14));
    push(0xffff0000);
    r.ebx = m.word(ebp - 0x2c);
    push(r.ebx);
    r.esi = m.word(ebp - 0x30);
    push(r.esi);
    r.edi = m.word(ebp - 0x34);
    push(r.edi);
    r.eax = m.word(ebp - 4);
    if (!callTo(0x492370))
        return;
    leave();
}

/* sub_4bae00: a detailed car's lights (car in eax, view in edx; the mirror
 * counts as the main view, a port): its model ([car+0x8a4]) and matrix
 * (sub_4ba810), nothing without lights in the model ([+0xf8]); the night set
 * (sub_4b97b0, sub_4b9550 when [0x6fd4c8]), sub_4ba320, sub_4ba350, the
 * model's position ([car+0x98..0xa0], y raised by [0x540124]) and matrix
 * (+0xc0) for sub_4b93c0; then sub_49de20 draws the headlights (+0xfc, its
 * value kept), the extra ones (+0x18c, with [car+0x660]), and -- between
 * sub_4b9140, sub_4b90e0, sub_4b9280 -- the four brake and indicator lights
 * (+0x108, +0x114, +0x120, +0x12c, raised by [car+0x93c] or [car+0x940])
 * not masked in [car+0x99c]; sub_4e0280, sub_4b8740 and sub_4b81a0 last. */
void detailedCarLightsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc0; sub ebp, 0x82
    const x86::reg32 ebp = entry - 20 - 0x82;
    x86::reg32 esp = entry - 20 - 0xc0;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // sub_49de20 for light `which` (edx) at `ecx`, its value left on the FPU
    auto light = [&](x86::reg32 which, x86::reg32 at, x86::reg32 raise) {
        r.eax = m.word(ebp + 0x76);
        r.edi = ebp + 0x52;
        r.esi = r.eax + at;
        for (x86::reg32 k = 0; k < 0xc; k += 4)
            m.word(ebp + 0x52 + k, m.word(r.esi + k));
        r.esi += 0xc;
        r.edi += 0xc;
        r.eax = m.word(ebp + 0x7e);
        const double raised = M::add(m.load(ebp + 0x56), m.load(r.eax + raise));
        r.eax = m.word(ebp + 0x7a);
        push(0);
        r.eax += 0x38;
        push(r.eax);
        r.eax = m.word(ebp + 0x7a);
        r.eax += 0x44;
        r.ecx = ebp + 0x52;
        push(r.eax);
        r.eax = ebp + 0x5e;
        r.ebx = ebp + 0xa;
        push(r.eax);
        r.eax = ebp - 0x3e;
        r.edx = which;
        push(r.eax);
        r.eax = m.word(ebp + 0x76);
        m.store(ebp + 0x56, raised);
        if (!callTo(0x49de20))
            return false;
        cpu.fpu.count -= 1;
        return true;
    };

    m.word(ebp + 0x7e, r.eax);
    m.word(ebp + 0x7a, r.edx);
    r.eax = m.word(r.eax + 0x8a4);
    m.word(ebp + 0x76, r.eax);
    r.eax = m.word(ebp + 0x7e);
    r.edx = m.word(ebp + 0x76);
    if (!callTo(0x4ba810))
        return;
    m.word(ebp + 0x72, r.eax);
    flags.logic(r.edx);
    if (!r.edx)
    {
        leave();
        return;
    }
    flags.compare(m.word(r.edx + 0xf8), 0);
    if (x86::sreg32(m.word(r.edx + 0xf8)) <= 0)
    {
        leave();
        return;
    }
    flags.compare(m.word(0x6fd4c8), 0);
    if (m.word(0x6fd4c8) != 0)
    {
        r.eax = m.word(ebp + 0x7a);
        // port (apply_mirror_detail): the mirror as the main view
        flags.compare(mirrorAsMain(m.word(r.eax)), 1);
        if (mirrorAsMain(m.word(r.eax)) != 1)
        {
            r.edx = m.word(ebp + 0x72);
            r.eax = m.word(ebp + 0x7e);
            if (!callTo(0x4b97b0))
                return;
            r.edx = r.eax;
            r.eax = m.word(ebp + 0x7e);
            if (!callTo(0x4b9550))
                return;
            m.word(ebp + 0x72, r.eax);
        }
    }
    r.ebx = m.word(ebp + 0x72);
    r.edx = m.word(ebp + 0x7e);
    r.eax = m.word(ebp + 0x7a);
    if (!callTo(0x4ba320))
        return;
    r.ebx = m.word(ebp + 0x72);
    r.edx = m.word(ebp + 0x7e);
    r.eax = m.word(ebp + 0x7a);
    if (!callTo(0x4ba350))
        return;
    m.word(ebp + 0x6e, r.eax);
    r.eax = m.word(ebp + 0x7e);
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x3e + k, m.word(r.eax + 0xc0 + k));
    r.ecx = 0;
    r.edi = ebp - 0x3e + 0x24;
    r.esi = r.eax + 0xc0 + 0x24;
    r.eax = m.word(r.eax + 0x98);
    r.ebx = ebp - 0x3e;
    m.word(ebp + 0x5e, r.eax);
    r.eax = m.word(ebp + 0x7e);
    r.edx = ebp - 0x1a;
    r.edi = m.word(ebp + 0x6e);
    r.eax = m.word(r.eax + 0x9c);
    r.esi = m.word(ebp + 0x7a);
    m.word(ebp + 0x62, r.eax);
    r.eax = m.word(ebp + 0x7e);
    const double up = M::add(m.load(ebp + 0x62), m.load(0x540124));
    r.eax = m.word(r.eax + 0xa0);
    m.word(ebp + 0x66, r.eax);
    r.eax = m.word(ebp + 0x7e);
    m.store(ebp + 0x62, up);
    if (!callTo(0x4b93c0))
        return;
    push(r.edi);
    r.esi += 0x38;
    r.edi = m.word(ebp + 0x7a);
    push(r.esi);
    r.edi += 0x44;
    r.ecx = m.word(ebp + 0x76);
    push(r.edi);
    r.eax = ebp + 0x5e;
    r.ecx += 0xfc;
    push(r.eax);
    r.eax = ebp - 0x3e;
    r.ebx = ebp - 0x1a;
    push(r.eax);
    r.edx = 0;
    r.eax = m.word(ebp + 0x76);
    if (!callTo(0x49de20))
        return;
    r.eax = m.word(ebp + 0x7e);
    r.edx = m.word(r.eax + 0x660);
    app->getMemory<float>(ebp + 0x6a) = float(cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    flags.logic(r.edx);
    if (r.edx)
    {
        r.ecx = m.word(ebp + 0x6e);
        push(r.ecx);
        push(r.esi);
        r.eax = ebp + 0x5e;
        push(r.edi);
        r.ebx = ebp - 0x1a;
        r.edx = 0xc;
        push(r.eax);
        r.eax = ebp - 0x3e;
        r.ecx = m.word(ebp + 0x76);
        push(r.eax);
        r.ecx += 0x18c;
        r.eax = m.word(ebp + 0x76);
        if (!callTo(0x49de20))
            return;
        cpu.fpu.count -= 1;
    }
    r.ebx = 1;
    r.edx = ebp + 0xa;
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b9140))
        return;
    r.eax = m.word(ebp + 0x7e);
    r.edx = 0;
    if (!callTo(0x4b90e0))
        return;
    r.eax = m.word(ebp + 0x7e);
    flags.logic8(m.byte(r.eax + 0x99c) & 1);
    if (!(m.byte(r.eax + 0x99c) & 1) && !light(1, 0x108, 0x93c))
        return;
    r.eax = m.word(ebp + 0x7e);
    flags.logic8(m.byte(r.eax + 0x99c) & 2);
    if (!(m.byte(r.eax + 0x99c) & 2) && !light(2, 0x114, 0x93c))
        return;
    r.edx = ebp + 0xa;
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b9280))
        return;
    r.edx = 1;
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b90e0))
        return;
    r.eax = m.word(ebp + 0x7e);
    flags.logic8(m.byte(r.eax + 0x99c) & 8);
    if (!(m.byte(r.eax + 0x99c) & 8) && !light(3, 0x120, 0x940))
        return;
    r.eax = m.word(ebp + 0x7e);
    flags.logic8(m.byte(r.eax + 0x99c) & 4);
    if (!(m.byte(r.eax + 0x99c) & 4) && !light(4, 0x12c, 0x940))
        return;
    r.ebx = ebp + 0x2e;
    r.edx = ebp - 0x3e;
    r.eax = ebp - 0x1a;
    r.ecx = ebp + 0x5e;
    if (!callTo(0x4e0280))
        return;
    r.ebx = m.word(ebp + 0x72);
    r.edx = m.word(ebp + 0x7e);
    push(r.ebx);
    r.eax = m.word(ebp + 0x7a);
    r.ebx = ebp + 0x2e;
    push(m.word(ebp + 0x6a));
    if (!callTo(0x4b8740))
        return;
    r.edx = m.word(ebp + 0x7a);
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b81a0))
        return;
    leave();
}


/* sub_475b50: a line of lettering in solid letters -- the countdown's number,
 * the race's messages -- stdcall:
 *   eax  the text (bytes, 0 ending it), dx, bx  where it starts on the screen
 *   +4   the depth's scale (0: flat letters), +8  the size, +0xc  a texture
 *        record or 0, +0x10  a matrix (0: each letter drawn alone, flat)
 * Its look is record [0x55b088] of the 0x238-byte records at 0x55b08c (kind,
 * colours at +8, +0xc, +0x10, +0x14, fog distances +0x18, +0x1c, a texture
 * at +0x2c, eight lights of 0x40 bytes from +0x30 and their counts at +0x230,
 * +0x234); the letters' models are the font at [0x679010] (first letter at
 * +0, count at +1, a 0x20-byte record a letter from +0x10: vertices, faces,
 * normals, advance).  Each letter's vertices are turned by sub_4e03b0 into
 * a buffer in the frame, put on the screen with a depth from 1 - (z / scale
 * + 2) * 0.1 held to [[0x53b078], 1], s and t from sub_4ea9c0's angle of the
 * normal, and its faces that face the screen drawn by THRASH_drawtri from
 * 0x55cf9c, coloured as the kind asks: by the face's place in the letter, by
 * fog with the depth, by the light on each corner or on the face, or by the
 * eight lights (sub_4e01f0, sub_4e0090, sub_4ea920; a light whose time
 * [0x7d3684] has passed is dropped).  The pen moves on by the letter's
 * advance (20 for a byte no letter has) times the size.  Without a scale the
 * letters are drawn flat, depth 1. */
void solidTextNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x58e4; sub ebp, 0x72
    const x86::reg32 ebp = entry - 16 - 0x72;
    x86::reg32 esp = entry - 16 - 0x58e4;
    const x86::reg32 rotated = ebp - 0x5872;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto fild32 = [&](x86::reg32 address) { return double(x86::sreg32(m.word(address))); };
    auto fild16 = [&](x86::reg32 address) { return double(x86::sreg16(app->getMemory<x86::reg16>(address))); };
    // fcom..., fnstsw ax, sahf
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    // sub_4dfd56 on st(0), then fistp to `at`
    auto truncate = [&](double value, x86::reg32 at) {
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(value);
        if (!callTo(0x4dfd56))
            return false;
        m.word(at, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
        cpu.fpu.count -= 1;
        return true;
    };
    // A float sub_4e01f0 or the like leaves on the FPU.
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    auto byteAt = [&](x86::reg32 address) { return x86::reg32(m.byte(address)); };
    auto exit = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = r.ebx;
        cpu.edx = r.edx;
        cpu.ecx = saved[0];
        cpu.esi = saved[1];
        cpu.edi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = entry + 4 + 0x10;
    };
    const x86::reg32 scale = ebp + 0x86, size = ebp + 0x8a, texture = ebp + 0x8e, matrix = ebp + 0x92;

    m.word(ebp + 0x4e, r.eax);
    app->getMemory<x86::reg16>(ebp + 0x6a) = x86::reg16(r.edx);
    app->getMemory<x86::reg16>(ebp + 0x66) = x86::reg16(r.ebx);
    r.eax = rotated;
    r.edx = m.word(0x55b088);
    m.word(ebp - 2, r.eax);
    r.eax = r.edx * 0x238;
    r.edx = 0x55b08c + r.eax;
    m.word(ebp + 0x56, r.edx);
    const x86::reg32 look = r.edx;
    flags.compare(m.word(ebp + 0x4e), 0);
    if (m.word(ebp + 0x4e) == 0)
        return exit();
    flags.compare(m.word(0x679010), 0);
    if (m.word(0x679010) == 0)
        return exit();
    flags.compare(m.word(0x55b088), 0xffffffff);
    if (m.word(0x55b088) == 0xffffffff)
        return exit();
    r.eax = look;
    r.esi = m.word(r.eax + 0x2c);
    flags.logic(r.esi);
    if (r.esi)
    {
        r.eax = 1;
        r.edx = m.word(r.esi + 4);
        if (!callTo(0x431900))
            return;
        r.edx = 1;
        r.eax = 0xd;
    }
    else
    {
        r.edx = 0x3f800000;
        r.edi = m.word(texture);
        m.word(0x55cfb8, r.esi);
        m.word(0x55cfd8, r.esi);
        m.word(0x55cfb4, r.esi);
        m.word(0x55cfd4, r.edx);
        m.word(0x55cff4, r.edx);
        m.word(0x55cff8, r.edx);
        flags.logic(r.edi);
        r.eax = 1;
        r.edx = r.edi ? m.word(r.edi + 4) : 0;
    }
    if (!callTo(0x431900))
        return;
    r.edx = m.word(0x679010);
    {
        const double height = M::mul(m.load(size), m.load(r.edx + 8));
        r.eax = m.word(ebp + 0x66);
        m.word(ebp + 0x6e, r.eax);
        if (!truncate(M::add(height, fild16(ebp + 0x6e)), ebp + 0x5e))
            return;
    }
    r.eax = m.word(ebp + 0x5e);
    r.ecx = m.word(scale);
    app->getMemory<x86::reg16>(ebp + 0x66) = x86::reg16(r.eax);
    flags.logic(r.ecx & 0x7fffffff);
    if (!(r.ecx & 0x7fffffff))
        goto flat;
    {
        const double depthScale = m.load(scale);
        m.store(ebp - 0x26, M::div(1.0, depthScale));
        compareToFlags(depthScale, m.load(r.edx + 4));
    }
    if (!cpu.flags.zf)
    {
        // A new scale: every letter's vertices get it at +8, the font at +4.
        r.edx = byteAt(r.edx);
        for (;;)
        {
            r.eax = m.word(0x679010);
            r.ebx = byteAt(r.eax + 1);
            r.ecx = byteAt(r.eax);
            r.ebx += r.ecx;
            flags.compare(r.edx, r.ebx);
            if (x86::sreg32(r.edx) >= x86::sreg32(r.ebx))
                break;
            r.ecx = (r.edx << 5) + r.eax + 0x10;
            r.ebx = byteAt(r.ecx + 2);
            for (;;)
            {
                r.eax = byteAt(r.ecx + 3);
                flags.compare(r.ebx, r.eax);
                if (x86::sreg32(r.ebx) >= x86::sreg32(r.eax))
                    break;
                r.esi = r.ebx * 12;
                r.eax = m.word(r.ecx + 0x10);
                flags.add(r.eax, r.esi);
                r.eax += r.esi;
                r.esi = m.word(scale);
                flags.inc(r.ebx);
                ++r.ebx;
                m.word(r.eax + 8, r.esi);
            }
            flags.inc(r.edx);
            ++r.edx;
        }
        r.edx = m.word(scale);
        m.word(r.eax + 4, r.edx);
    }

letter:
    r.eax = m.word(ebp + 0x4e);
    flags.compare8(m.byte(r.eax), 0);
    if (m.byte(r.eax) == 0)
        goto done;
    r.edx = m.word(ebp + 0x4e);
    r.eax = m.word(0x679010);
    setByte(r.edx, 0, m.byte(r.edx));
    flags.compare8(x86::reg8(r.edx), m.byte(r.eax));
    if (x86::reg8(r.edx) < m.byte(r.eax))
        goto unknown;
    r.ebx = byteAt(r.eax);
    r.edx = byteAt(r.eax + 1);
    r.ecx = m.word(ebp + 0x4e);
    r.ebx += r.edx;
    r.edx = byteAt(r.ecx);
    flags.compare(r.edx, r.ebx);
    if (x86::sreg32(r.edx) >= x86::sreg32(r.ebx))
        goto unknown;
    r.eax += 0x10;
    r.edx <<= 5;
    r.eax += r.edx;
    r.ebx = m.word(matrix);
    m.word(ebp + 0x52, r.eax);
    flags.logic(r.ebx);
    if (!r.ebx)
        goto alone;
    {
        // The letter's vertices turned (sub_4e03b0) into the frame's buffer,
        // its normals after them ([ebp + 0x3a]), its faces' normals after
        // those ([ebp + 0x4a]).
        const x86::reg32 part = r.eax;
        r.eax = look;
        r.esi = m.word(r.eax);
        flags.compare(r.esi, 2);
        if (r.esi == 2)
        {
            r.ecx = rotated;
            r.edx = m.word(part + 0x10);
            r.eax = byteAt(part + 3) * 2;
            r.ebx = m.word(matrix);
            if (!callTo(0x4e03b0))
                return;
            r.eax = m.word(ebp + 0x52);
            r.edx = byteAt(r.eax + 3);
            r.eax = r.edx * 3;
            r.edx = rotated;
            r.eax <<= 2;
            flags.add(r.edx, r.eax);
            r.edx += r.eax;
            m.word(ebp + 0x3a, r.edx);
            goto project;
        }
        flags.compare(r.esi, 3);
        bool both = false;
        if (r.esi == 3)
        {
            flags.compare(m.word(r.eax + 0x2c), 0);
            if (m.word(r.eax + 0x2c) == 0)
            {
                r.ecx = rotated;
                r.edx = m.word(part + 0x10);
                r.eax = byteAt(part + 3);
                r.ebx = m.word(matrix);
                if (!callTo(0x4e03b0))
                    return;
                r.eax = m.word(ebp + 0x52);
                r.edx = byteAt(r.eax + 3);
                r.edx = rotated + r.edx * 12;
                r.eax = r.edx - rotated;
                r.ebx = m.word(ebp + 0x52);
                m.word(ebp + 0x4a, r.edx);
                r.eax = 0;
                flags.logic(0);
                r.edx = m.word(ebp + 0x52);
                r.ecx = m.word(ebp + 0x4a);
                r.eax = byteAt(r.ebx + 1);
                r.ebx = m.word(matrix);
                r.edx = m.word(r.edx + 0x14);
                goto turnFaces;
            }
        }
        r.eax = look;
        r.ecx = m.word(r.eax);
        flags.compare(r.ecx, 4);
        if (r.ecx == 4)
            both = true;
        else
        {
            flags.compare(r.ecx, 3);
            if (r.ecx == 3)
            {
                flags.compare(m.word(r.eax + 0x2c), 0);
                both = m.word(r.eax + 0x2c) != 0;
            }
        }
        r.ecx = rotated;
        r.edx = m.word(part + 0x10);
        r.eax = byteAt(part + 3);
        r.ebx = m.word(matrix);
        if (both)
        {
            r.eax *= 2;
            if (!callTo(0x4e03b0))
                return;
            r.eax = m.word(ebp + 0x52);
            r.ebx = byteAt(r.eax + 3);
            r.edx = rotated + r.ebx * 12;
            r.eax = r.ebx * 24;
            m.word(ebp + 0x3a, r.edx);
            r.edx = rotated + r.eax;
            r.ebx = m.word(ebp + 0x52);
            m.word(ebp + 0x4a, r.edx);
            r.eax = 0;
            flags.logic(0);
            r.edx = m.word(ebp + 0x52);
            r.ecx = m.word(ebp + 0x4a);
            r.eax = byteAt(r.ebx + 1);
            r.ebx = m.word(matrix);
            r.edx = m.word(r.edx + 0x14);
        }
    }
turnFaces:
    if (!callTo(0x4e03b0))
        return;
project:
    // The vertices onto the screen: x, y, depth, and s, t when textured, 20
    // bytes each from ebp - 0x2872.
    r.edx = 0;
    for (;;)
    {
        r.ebx = m.word(ebp + 0x52);
        r.eax = byteAt(r.ebx + 3);
        flags.compare(r.edx, r.eax);
        if (x86::sreg32(r.edx) >= x86::sreg32(r.eax))
            break;
        r.ecx = m.word(ebp - 2) + r.edx * 12;
        {
            const double letterSize = m.load(size);
            const double x = M::mul(m.load(r.ecx), letterSize);
            r.eax = m.word(ebp + 0x6a);
            m.word(ebp + 0x6e, r.eax);
            r.eax = r.edx * 4;
            const double screenX = M::add(x, fild16(ebp + 0x6e));
            flags.shl(r.edx * 5, 2);
            r.ebx = r.edx * 20;
            r.eax = m.word(ebp + 0x66);
            m.store(r.ebx + ebp - 0x2872, screenX);
            const double y = M::mul(letterSize, m.load(r.ecx + 4));
            m.word(ebp + 0x6e, r.eax);
            m.store(r.ebx + ebp - 0x286e, M::sub(fild16(ebp + 0x6e), y));
            const double scaled = M::mul(M::add(M::mul(m.load(r.ecx + 8), m.load(ebp - 0x26)), m.load(0x53b06c)),
                                         m.load(0x53b070));
            const double depth = M::sub(1.0, scaled);
            m.store(r.ebx + ebp - 0x286a, depth);
            compareToFlags(1.0, double(float(depth)));
            if (cpu.flags.cf)
                m.word(r.ebx + ebp - 0x286a, 0x3f800000);
        }
        r.eax = r.edx * 4;
        flags.shl(r.edx * 5, 2);
        r.ebx = r.edx * 20;
        compareToFlags(m.load(r.ebx + ebp - 0x286a), m.loadDouble(0x53b078));
        if (cpu.flags.cf)
            m.word(r.ebx + ebp - 0x286a, 0x37800080);
        r.eax = look;
        flags.compare(m.word(r.eax + 0x2c), 0);
        if (m.word(r.eax + 0x2c) != 0)
        {
            r.eax = r.edx * 12;
            r.ebx = m.word(ebp + 0x3a) + r.eax;
            push(m.word(r.ebx + 4));
            push(m.word(r.ebx + 8));
            if (!callTo(0x4ea9c0))
                return;
            const double angle = result();
            r.eax = r.edx * 4;
            const double s = M::add(angle, m.load(0x53b080));
            flags.add(r.eax, r.edx);
            r.eax += r.edx;
            m.store(ebp + r.eax * 4 - 0x2866, s);
            m.store(ebp + r.eax * 4 - 0x2862, M::mul(m.load(0x53b080), M::add(1.0, m.load(r.ebx))));
        }
        flags.inc(r.edx);
        ++r.edx;
    }
    // The faces, three vertex bytes each from [part + 0x1c].
    setByte(r.ebx, 0, 0);
face:
    r.edx = m.word(ebp + 0x4e);
    r.eax = byteAt(r.edx) << 5;
    r.edx = m.word(0x679010);
    flags.compare8(x86::reg8(r.ebx), m.byte(r.edx + r.eax + 0x11));
    if (x86::reg8(r.ebx) >= m.byte(r.edx + r.eax + 0x11))
        goto advance;
    r.edx = x86::reg8(r.ebx);
    r.eax = r.edx * 3;
    r.edx = m.word(m.word(ebp + 0x52) + 0x1c) + r.eax;
    r.ecx = 0;
    flags.logic(0);
    m.word(ebp + 0x3e, r.edx);
    for (;; ++r.ecx)
    {
        if (r.ecx)
        {
            flags.inc(r.ecx - 1);
            flags.compare(r.ecx, 3);
            if (x86::sreg32(r.ecx) >= 3)
                break;
        }
        r.edx = m.word(ebp + 0x3e) + r.ecx;
        const x86::reg32 corner = byteAt(r.edx);
        r.esi = r.ecx << 5;
        m.store(r.esi + 0x55cf9c, m.load(ebp + corner * 20 - 0x2872));
        m.word(ebp + 0x5a, corner);
        r.edi = corner;
        m.store(r.esi + 0x55cfa0, m.load(ebp + corner * 20 - 0x286e));
        r.eax = look;
        m.store(r.esi + 0x55cfa8, m.load(ebp + corner * 20 - 0x286a));
        flags.compare(m.word(r.eax + 0x2c), 0);
        if (m.word(r.eax + 0x2c) != 0)
        {
            r.eax = corner;
            m.word(ebp + 0x5a, r.eax);
            r.edi = m.word(ebp + corner * 20 - 0x2866);
            m.word(r.esi + 0x55cfb4, r.edi);
            r.edx = corner;
            flags.add(corner * 4, corner);
            r.eax = corner * 5;
            r.edx = m.word(ebp + corner * 20 - 0x2862);
            m.word(r.esi + 0x55cfb8, r.edx);
        }
    }
    {
        // Facing the screen: (x2 - x1)(y0 - y1) above (y2 - y1)(x0 - x1).
        const double a = M::mul(M::sub(m.load(0x55cfdc), m.load(0x55cfbc)), M::sub(m.load(0x55cfa0), m.load(0x55cfc0)));
        const double b = M::mul(M::sub(m.load(0x55cfe0), m.load(0x55cfc0)), M::sub(m.load(0x55cf9c), m.load(0x55cfbc)));
        compareToFlags(b, a);
        if (cpu.flags.cf || cpu.flags.zf)
            goto nextFace;
    }
    r.eax = m.word(look);
    flags.compare(r.eax, 4);
    switch (r.eax)
    {
    case 0: goto byPlace;
    case 1: goto byFog;
    case 2: goto byCorner;
    case 3: goto byFace;
    case 4: goto byLights;
    default: goto draw;
    }

byPlace:
    // The letter's first faces in +8, the next as many in +0xc, the rest in +0x10.
    {
        r.edx = m.word(ebp + 0x4e);
        r.eax = byteAt(r.edx);
        r.edx = m.word(0x679010);
        r.eax = (r.eax << 5) + r.edx;
        setByte(r.ecx, 0, m.byte(r.eax + 0x10));
        flags.compare8(x86::reg8(r.ebx), x86::reg8(r.ecx));
        x86::reg32 colour;
        if (x86::reg8(r.ebx) < x86::reg8(r.ecx))
            colour = 8;
        else
        {
            r.eax = (x86::reg32(x86::reg8(r.ecx))) * 2;
            r.edx = x86::reg8(r.ebx);
            flags.compare(r.edx, r.eax);
            colour = x86::sreg32(r.edx) < x86::sreg32(r.eax) ? 0xc : 0x10;
        }
        r.eax = m.word(look + colour);
        m.word(0x55cfac, r.eax);
        m.word(0x55cfcc, r.eax);
        m.word(0x55cfec, r.eax);
    }
    goto draw;

byFog:
    // Each corner: +8 nearer than +0x18, +0xc beyond +0x1c, +0x14 at 0,
    // between them +0x14 blended with +0xc or +8 by the depth.
    r.eax = m.word(look + 0x14);
    m.word(ebp - 0xa, (r.eax & 0xff0000) >> 16);
    m.word(ebp - 6, (r.eax & 0xff00) >> 8);
    r.edi = m.word(look + 0x14);
    r.eax = m.word(look + 8);
    m.word(ebp - 0x1e, (r.eax & 0xff0000) >> 16);
    m.word(ebp - 0x2a, (r.eax & 0xff00) >> 8);
    m.word(ebp - 0x2e, r.eax & 0xff);
    r.eax = m.word(look + 0xc);
    m.word(ebp - 0xe, (r.eax & 0xff0000) >> 16);
    m.word(ebp - 0x36, (r.eax & 0xff00) >> 8);
    r.esi = 0;
    r.eax &= 0xff;
    r.edi &= 0xff;
    flags.logic(r.edi);
    m.word(ebp - 0x22, r.eax);
    for (;;)
    {
        r.edx = byteAt(m.word(ebp + 0x3e) + r.esi);
        r.eax = r.edx * 3;
        r.edx = m.word(ebp - 2) + r.eax * 4;
        flags.shl(r.esi, 5);
        r.ecx = r.esi << 5;
        r.eax = look;
        compareToFlags(m.load(r.edx + 8), m.load(r.eax + 0x18));
        if (cpu.flags.cf || cpu.flags.zf)
            r.eax = m.word(look + 8);
        else
        {
            r.eax = look;
            compareToFlags(m.load(r.edx + 8), m.load(r.eax + 0x1c));
            if (!cpu.flags.cf)
                r.eax = m.word(look + 0xc);
            else
            {
                flags.logic(m.word(r.edx + 8) & 0x7fffffff);
                if (!(m.word(r.edx + 8) & 0x7fffffff))
                    r.eax = m.word(look + 0x14);
                else
                {
                    compareToFlags(0.0, m.load(r.edx + 8));
                    const bool beyond = !cpu.flags.cf;
                    const x86::reg32 far = beyond ? 0x18 : 0x1c;
                    const x86::reg32 red = beyond ? ebp - 0x1e : ebp - 0xe;
                    const x86::reg32 green = beyond ? ebp - 0x2a : ebp - 0x36;
                    const x86::reg32 blue = beyond ? ebp - 0x2e : ebp - 0x22;
                    r.eax = look;
                    const double t = M::div(m.load(r.edx + 8), m.load(r.eax + far));
                    r.eax = m.word(ebp - 0xa);
                    m.word(ebp + 0x5a, r.eax);
                    const double u = M::sub(1.0, t);
                    const double fogRed = M::mul(fild32(ebp + 0x5a), u);
                    r.eax = m.word(red);
                    m.word(ebp + 0x5a, r.eax);
                    if (!truncate(M::add(fogRed, M::mul(fild32(ebp + 0x5a), t)), ebp - 0x3a))
                        return;
                    r.eax = (m.word(ebp - 0x3a) << 16) & 0xff0000;
                    if (beyond)
                        m.word(ebp - 0x3a, r.eax);
                    else
                        m.word(ebp + 0x5a, r.eax);
                    r.eax = m.word(look + 0x14) & 0xff000000;
                    r.edx = m.word(beyond ? ebp - 0x3a : ebp + 0x5a);
                    r.eax |= r.edx;
                    m.word(ebp - 0x3a, r.eax);
                    r.eax = m.word(ebp - 6);
                    m.word(ebp + 0x5a, r.eax);
                    const double fogGreen = M::mul(fild32(ebp + 0x5a), u);
                    r.eax = m.word(green);
                    m.word(ebp + 0x5e, r.eax);
                    m.word(ebp + 0x5a, r.edi);
                    const double sumGreen = M::add(fogGreen, M::mul(fild32(ebp + 0x5e), t));
                    const double fogBlue = M::mul(u, fild32(ebp + 0x5a));
                    if (!truncate(sumGreen, ebp + 0x5a))
                        return;
                    r.eax = m.word(ebp + 0x5a) << 8;
                    if (beyond)
                    {
                        r.edx = m.word(ebp - 0x3a);
                        r.eax &= 0xff00;
                        r.eax |= r.edx;
                        r.edx = m.word(blue);
                        m.word(ebp + 0x5a, r.edx);
                    }
                    else
                    {
                        r.edx = m.word(ebp - 0x3a);
                        r.eax &= 0xff00;
                        r.edx |= r.eax;
                        r.eax = m.word(blue);
                        m.word(ebp + 0x5a, r.eax);
                    }
                    if (!truncate(M::add(M::mul(t, fild32(ebp + 0x5a)), fogBlue), ebp - 0x3a))
                        return;
                    if (beyond)
                        r.edx = m.word(ebp - 0x3a) & 0xff;
                    else
                    {
                        r.eax = m.word(ebp - 0x3a) & 0xff;
                        flags.logic(r.eax);
                    }
                    r.eax |= r.edx;
                }
            }
        }
        m.word(r.ecx + 0x55cfac, r.eax);
        flags.inc(r.esi);
        ++r.esi;
        flags.compare(r.esi, 3);
        if (x86::sreg32(r.esi) >= 3)
            break;
    }
    goto draw;

byCorner:
    // Each corner lit by its normal against the light at +0x20.
    r.eax = m.word(look + 0x14);
    m.word(ebp - 0x32, (r.eax & 0xff0000) >> 16);
    m.word(ebp - 0x1a, (r.eax & 0xff00) >> 8);
    r.edi = m.word(look + 0x14);
    r.ecx = 0;
    r.edi &= 0xff;
    flags.logic(r.edi);
    for (;;)
    {
        r.edx = look + 0x20;
        r.eax = byteAt(r.ecx + m.word(ebp + 0x3e));
        r.esi = r.eax;
        r.eax = r.eax * 12;
        r.esi = m.word(ebp + 0x3a);
        flags.add(r.eax, r.esi);
        r.eax += r.esi;
        if (!callTo(0x4e01f0))
            return;
        m.store(ebp + 0x42, result());
        compareToFlags(0.0, m.load(ebp + 0x42));
        if (!(cpu.flags.cf || cpu.flags.zf))
        {
            r.eax = 0;
            m.word(ebp + 0x42, 0);
        }
        r.eax = m.word(ebp - 0x32);
        m.word(ebp + 0x5a, r.eax);
        const double light = m.load(ebp + 0x42);
        r.edx = look;
        if (!truncate(M::mul(fild32(ebp + 0x5a), light), ebp - 0x3a))
            return;
        r.eax = m.word(ebp - 0x3a);
        r.edx = m.word(r.edx + 0x14);
        r.eax <<= 16;
        r.edx &= 0xff000000;
        r.eax &= 0xff0000;
        r.edx |= r.eax;
        r.eax = m.word(ebp - 0x1a);
        m.word(ebp + 0x5e, r.eax);
        m.word(ebp + 0x5a, r.edi);
        const double green = M::mul(fild32(ebp + 0x5e), light);
        const double blue = M::mul(light, fild32(ebp + 0x5a));
        if (!truncate(green, ebp - 0x3a))
            return;
        r.eax = m.word(ebp - 0x3a) << 8;
        if (!truncate(blue, ebp - 0x3a))
            return;
        r.eax &= 0xff00;
        r.edx |= r.eax;
        r.eax = m.word(ebp - 0x3a) & 0xff;
        r.eax |= r.edx;
        r.edx = r.ecx;
        flags.shl(r.edx, 5);
        r.edx <<= 5;
        flags.inc(r.ecx);
        ++r.ecx;
        m.word(r.edx + 0x55cfac, r.eax);
        flags.compare(r.ecx, 3);
        if (x86::sreg32(r.ecx) >= 3)
            break;
    }
    goto draw;

byFace:
    // The face lit by its normal against the light at +0x20.
    {
        r.eax = m.word(look + 0x14);
        r.edi = look;
        r.eax &= 0xff00;
        r.ecx = x86::reg8(r.ebx);
        r.eax >>= 8;
        m.word(ebp - 0x12, r.eax);
        r.eax = look;
        r.edi = m.word(r.edi + 0x14);
        r.edx = r.eax + 0x20;
        r.esi = m.word(r.eax + 0x14);
        r.eax = r.ecx * 3;
        r.ecx = m.word(ebp + 0x4a);
        r.edi = (r.edi & 0xff0000) >> 16;
        r.eax = r.eax * 4 + r.ecx;
        r.esi &= 0xff;
        flags.logic(r.esi);
        if (!callTo(0x4e01f0))
            return;
        m.store(ebp + 0x46, result());
        compareToFlags(0.0, m.load(ebp + 0x46));
        if (!(cpu.flags.cf || cpu.flags.zf))
        {
            r.eax = 0;
            m.word(ebp + 0x46, 0);
        }
        m.word(ebp + 0x5a, r.edi);
        const double light = m.load(ebp + 0x46);
        r.edx = look;
        if (!truncate(M::mul(fild32(ebp + 0x5a), light), ebp - 0x3a))
            return;
        r.eax = m.word(ebp - 0x3a);
        r.edx = m.word(r.edx + 0x14);
        r.eax <<= 16;
        r.edx &= 0xff000000;
        r.eax &= 0xff0000;
        r.eax |= r.edx;
        r.edx = m.word(ebp - 0x12);
        m.word(ebp + 0x5e, r.edx);
        m.word(ebp + 0x5a, r.esi);
        const double green = M::mul(fild32(ebp + 0x5e), light);
        const double blue = M::mul(light, fild32(ebp + 0x5a));
        if (!truncate(green, ebp - 0x3a))
            return;
        r.edx = m.word(ebp - 0x3a) << 8;
        if (!truncate(blue, ebp - 0x3a))
            return;
        r.edx &= 0xff00;
        r.edx |= r.eax;
        r.eax = m.word(ebp - 0x3a) & 0xff;
        r.eax |= r.edx;
        flags.logic(r.eax);
        m.word(0x55cfac, r.eax);
        m.word(0x55cfcc, r.eax);
        m.word(0x55cfec, r.eax);
    }
    goto draw;

byLights:
    // The eight lights at +0x30 (0x40 bytes each: +0 kind, +4 on the face,
    // +8 colour, +0xc on, +0x10 until, +0x18 position, +0x20 range, +0x24
    // fading by its own +0, ..), summed into ebp - 0x5a (red), - 0x4e
    // (green), - 0x72 (blue) from the colours at 0x474d20.., held to 255.
    for (x86::reg32 k = 0; k < 0xc; k += 4)
    {
        m.word(ebp - 0x5a + k, m.word(0x474d20 + k));
        m.word(ebp - 0x4e + k, m.word(0x474d2c + k));
        m.word(ebp - 0x72 + k, m.word(0x474d38 + k));
    }
    r.edi = ebp - 0x66;
    r.esi = 0x474d44;
    r.eax = look;
    r.edx = m.word(r.eax + 0x234);
    flags.logic(r.edx);
    if (r.edx)
    {
        r.ecx = 0;
        flags.logic(0);
        for (;;)
        {
            r.eax = (r.ecx << 6) + look;
            flags.compare(m.word(r.eax + 0x3c), 0);
            if (m.word(r.eax + 0x3c) == 0)
                goto nextLight;
            r.edx = m.word(r.eax + 0x30);
            flags.logic(r.edx);
            if (r.edx)
                goto nextLight;
            r.esi = m.word(r.eax + 0x40);
            flags.compare(r.esi, 0xffffffff);
            if (r.esi != 0xffffffff)
            {
                flags.compare(r.esi, m.word(0x7d3684));
                if (x86::sreg32(r.esi) < x86::sreg32(m.word(0x7d3684)))
                {
                    // Its time is up: off, and one fewer.
                    m.word(r.eax + 0x3c, r.edx);
                    r.eax = look;
                    r.esi = m.word(r.eax + 0x230) - 1;
                    r.edi = m.word(r.eax + 0x234) - 1;
                    m.word(r.eax + 0x230, r.esi);
                    m.word(r.eax + 0x234, r.edi);
                    goto nextLight;
                }
            }
            {
                r.edx = r.ecx << 6;
                r.eax = look;
                const x86::reg32 colour = m.word(r.edx + r.eax + 0x38);
                m.word(ebp + 0x5a, (colour & 0xff0000) >> 16);
                r.esi = (colour & 0xff00) >> 8;
                m.store(ebp + 2, fild32(ebp + 0x5a));
                m.word(ebp + 0x5a, r.esi);
                r.esi = colour & 0xff;
                m.store(ebp + 6, fild32(ebp + 0x5a));
                m.word(ebp + 0x5a, r.esi);
                m.store(ebp + 0xa, fild32(ebp + 0x5a));
                r.esi = m.word(r.edx + r.eax + 0x34);
                flags.logic(r.esi);
            }
            if (r.esi)
            {
                // On each corner.
                r.esi = 0;
                flags.logic(0);
                for (;;)
                {
                    r.eax = (r.ecx << 6) + look + 0x30;
                    r.edi = r.eax + 0x28;
                    r.eax = byteAt(r.esi + m.word(ebp + 0x3e)) * 12;
                    r.edx = m.word(ebp + 0x3a);
                    flags.add(r.eax, r.edx);
                    r.eax += r.edx;
                    r.edx = r.edi;
                    if (!callTo(0x4e01f0))
                        return;
                    m.store(ebp + 0x2a, result());
                    compareToFlags(0.0, m.load(ebp + 0x2a));
                    if (cpu.flags.cf)
                    {
                        const double light = m.load(ebp + 0x2a);
                        const double red = M::mul(light, m.load(ebp + 2));
                        const double green = M::mul(light, m.load(ebp + 6));
                        const double blue = M::mul(light, m.load(ebp + 0xa));
                        m.store(ebp + r.esi * 4 - 0x5a, M::add(red, m.load(ebp + r.esi * 4 - 0x5a)));
                        m.store(ebp + r.esi * 4 - 0x4e, M::add(green, m.load(ebp + r.esi * 4 - 0x4e)));
                        m.store(ebp + r.esi * 4 - 0x72, M::add(blue, m.load(ebp + r.esi * 4 - 0x72)));
                    }
                    flags.inc(r.esi);
                    ++r.esi;
                    flags.compare(r.esi, 3);
                    if (x86::sreg32(r.esi) >= 3)
                        break;
                }
                goto nextLight;
            }
            {
                // On the face: the same to all three corners.
                r.eax += 0x30;
                r.esi = x86::reg8(r.ebx);
                r.edx += r.eax;
                r.eax = r.esi * 12;
                r.edi = m.word(ebp + 0x4a);
                r.edx += 0x28;
                flags.add(r.eax, r.edi);
                r.eax += r.edi;
                if (!callTo(0x4e01f0))
                    return;
                m.store(ebp + 0x36, result());
                compareToFlags(0.0, m.load(ebp + 0x36));
                if (!cpu.flags.cf)
                    goto nextLight;
                r.edx = 0;
                flags.logic(0);
                for (;;)
                {
                    const double light = m.load(ebp + 0x36);
                    const double red = M::mul(light, m.load(ebp + 2));
                    const double green = M::mul(light, m.load(ebp + 6));
                    flags.inc(r.edx);
                    ++r.edx;
                    const double blue = M::mul(light, m.load(ebp + 0xa));
                    m.store(ebp + r.edx * 4 - 0x5e, M::add(red, m.load(ebp + r.edx * 4 - 0x5e)));
                    m.store(ebp + r.edx * 4 - 0x52, M::add(green, m.load(ebp + r.edx * 4 - 0x52)));
                    m.store(ebp + r.edx * 4 - 0x76, M::add(blue, m.load(ebp + r.edx * 4 - 0x76)));
                    flags.compare(r.edx, 3);
                    if (x86::sreg32(r.edx) >= 3)
                        break;
                }
            }
        nextLight:
            flags.inc(r.ecx);
            ++r.ecx;
            flags.compare(r.ecx, 8);
            if (x86::sreg32(r.ecx) >= 8)
                break;
        }
        r.eax = look;
        flags.compare(m.word(r.eax + 0x234), 1);
        if (x86::sreg32(m.word(r.eax + 0x234)) > 1)
        {
            // Shared by the lights on.
            m.store(ebp + 0x1a, M::div(1.0, fild32(r.eax + 0x234)));
            r.edx = 0;
            flags.logic(0);
            for (;;)
            {
                const double share = m.load(ebp + 0x1a);
                const double red = M::mul(m.load(ebp + r.edx * 4 - 0x5a), share);
                const double green = M::mul(m.load(ebp + r.edx * 4 - 0x4e), share);
                const double blueValue = m.load(ebp + r.edx * 4 - 0x72);
                flags.inc(r.edx);
                ++r.edx;
                const double blue = M::mul(share, blueValue);
                m.store(ebp + r.edx * 4 - 0x5e, red);
                m.store(ebp + r.edx * 4 - 0x52, green);
                m.store(ebp + r.edx * 4 - 0x76, blue);
                flags.compare(r.edx, 3);
                if (x86::sreg32(r.edx) >= 3)
                    break;
            }
        }
    }
    r.edx = look;
    r.eax = look;
    r.ecx = m.word(r.edx + 0x234);
    r.eax = m.word(r.eax + 0x230);
    flags.compare(r.eax, r.ecx);
    r.eax -= r.ecx;
    if (r.eax)
    {
        // The lights of kinds 1 and 2 that are not off by +0x24.
        r.esi = 0;
        flags.logic(0);
        for (;;)
        {
            r.eax = (r.esi << 6) + look;
            flags.compare(m.word(r.eax + 0x3c), 0);
            if (m.word(r.eax + 0x3c) == 0)
                goto nextSpot;
            r.ecx = m.word(r.eax + 0x30);
            flags.compare(r.ecx, 1);
            if (r.ecx != 1)
            {
                flags.compare(r.ecx, 2);
                if (r.ecx != 2)
                    goto nextSpot;
            }
            r.edx = (r.esi << 6) + look;
            flags.compare(m.word(r.edx + 0x54), 0);
            if (m.word(r.edx + 0x54) != 0)
                goto nextSpot;
            {
                flags.compare(m.word(r.edx + 0x30), 2);
                bool plain = m.word(r.edx + 0x30) != 2;
                if (!plain)
                {
                    r.eax = m.word(r.edx + 0x4c);
                    flags.logic(r.eax);
                    plain = !r.eax;
                }
                if (!plain)
                {
                    // Fading: its colour times what [+0x4c] holds over +0x50.
                    m.store(ebp - 0x16, M::div(1.0, m.load(r.edx + 0x50)));
                    r.eax = m.word(r.eax);
                    m.word(ebp + 0x12, r.eax);
                    compareToFlags(0.0, m.load(ebp + 0x12));
                    if (!cpu.flags.cf && !cpu.flags.zf)
                        goto nextSpot;
                    compareToFlags(m.load(ebp + 0x12), m.load(r.edx + 0x50));
                    const x86::reg32 colour = m.word(r.edx + 0x38);
                    if (cpu.flags.cf)
                    {
                        m.word(ebp + 0x5a, (colour & 0xff0000) >> 16);
                        const double red = fild32(ebp + 0x5a);
                        const double k = M::mul(m.load(ebp + 0x12), m.load(ebp - 0x16));
                        m.word(ebp + 0x5a, (colour & 0xff00) >> 8);
                        const double redK = M::mul(red, k);
                        const double greenK = M::mul(fild32(ebp + 0x5a), k);
                        r.eax = colour & 0xff;
                        flags.logic(r.eax);
                        m.word(ebp + 0x5a, r.eax);
                        const double blueK = M::mul(k, fild32(ebp + 0x5a));
                        m.store(ebp + 0x1e, redK);
                        m.store(ebp + 0x32, greenK);
                        m.store(ebp + 0x2e, blueK);
                    }
                    else
                    {
                        m.word(ebp + 0x5a, (colour & 0xff0000) >> 16);
                        const double red = fild32(ebp + 0x5a);
                        m.word(ebp + 0x5a, (colour & 0xff00) >> 8);
                        m.store(ebp + 0x1e, red);
                        r.eax = colour & 0xff;
                        flags.logic(r.eax);
                        const double green = fild32(ebp + 0x5a);
                        m.word(ebp + 0x5a, r.eax);
                        m.store(ebp + 0x32, green);
                        m.store(ebp + 0x2e, fild32(ebp + 0x5a));
                    }
                }
                else
                {
                    r.eax = (r.esi << 6) + look;
                    const x86::reg32 colour = m.word(r.eax + 0x38);
                    m.word(ebp + 0x5a, (colour & 0xff0000) >> 16);
                    m.store(ebp + 0x1e, fild32(ebp + 0x5a));
                    r.edx = (colour & 0xff00) >> 8;
                    r.eax = colour & 0xff;
                    m.word(ebp + 0x5a, r.edx);
                    m.word(ebp + 0x5e, r.eax);
                    const double blue = fild32(ebp + 0x5e);
                    m.store(ebp + 0x32, fild32(ebp + 0x5a));
                    m.store(ebp + 0x2e, blue);
                }
            }
            r.eax = r.esi << 6;
            r.ecx = look + r.eax;
            flags.compare(m.word(r.ecx + 0x34), 0);
            if (m.word(r.ecx + 0x34) == 0)
            {
                // On the face, from its middle on the screen.
                {
                    double x = m.load(0x55cf9c);
                    double y = m.load(0x55cfa0);
                    x = M::add(x, m.load(0x55cfbc));
                    y = M::add(y, m.load(0x55cfc0));
                    x = M::add(x, m.load(0x55cfdc));
                    y = M::add(y, m.load(0x55cfe0));
                    x = M::mul(x, m.load(0x53b084));
                    y = M::mul(y, m.load(0x53b088));
                    double w = M::add(m.load(0x55cfa4), m.load(0x55cfc4));
                    w = M::add(w, m.load(0x55cfe4));
                    w = M::mul(w, m.load(0x53b08c));
                    r.edx = look + 0x30;
                    r.eax += r.edx;
                    r.edx = ebp - 0x66;
                    r.eax += 0x28;
                    m.store(ebp - 0x66, x);
                    m.store(ebp - 0x62, y);
                    m.store(ebp - 0x5e, w);
                }
                if (!callTo(0x4e0090))
                    return;
                {
                    const double inverse = M::div(1.0, result());
                    const double dx = M::sub(m.load(r.ecx + 0x58), m.load(ebp - 0x66));
                    m.store(ebp - 0x66, dx);
                    const double dy = M::sub(m.load(ebp - 0x62), m.load(r.ecx + 0x5c));
                    m.store(ebp - 0x62, dy);
                    const double dy2 = M::mul(dy, m.load(ebp - 0x62));
                    const double dx2 = M::mul(m.load(ebp - 0x66), m.load(ebp - 0x66));
                    esp -= 4;
                    r.edx = x86::reg8(r.ebx);
                    flags.logic(0);
                    m.store(esp, M::add(dy2, dx2));
                    m.store(ebp + 0xe, inverse);
                }
                if (!callTo(0x4ea920))
                    return;
                {
                    const double distance = result();
                    const double inverse = M::div(1.0, distance);
                    const double dx = M::mul(m.load(ebp - 0x66), inverse);
                    r.eax = r.edx * 3;
                    r.edx = m.word(ebp + 0x4a);
                    const double dy = M::mul(inverse, m.load(ebp - 0x62));
                    flags.add(r.eax * 4, r.edx);
                    r.eax = r.eax * 4 + r.edx;
                    m.store(ebp - 0x66, dx);
                    m.store(ebp - 0x62, dy);
                    const double dot = M::add(M::mul(m.load(r.eax + 4), m.load(ebp - 0x62)),
                                              M::mul(m.load(r.eax), m.load(ebp - 0x66)));
                    m.store(ebp + 0x26, dot);
                    compareToFlags(0.0, m.load(ebp + 0x26));
                    if (!cpu.flags.cf)
                        goto nextSpot;
                }
                r.eax = 0;
                flags.logic(0);
                for (;;)
                {
                    const x86::reg32 channels[] = { 0x5a, 0x4e, 0x72 };
                    const x86::reg32 colours[] = { 0x1e, 0x32, 0x2e };
                    for (int c = 0; c < 3; ++c)
                    {
                        const double add = M::mul(M::mul(m.load(ebp + 0x26), m.load(ebp + colours[c])), m.load(ebp + 0xe));
                        r.edx = r.eax * 4;
                        const x86::reg32 at = r.edx + ebp - channels[c];
                        m.store(at, M::add(add, m.load(at)));
                        flags.compare(m.word(at), 0x437f0000);
                        if (x86::sreg32(m.word(at)) > 0x437f0000)
                            m.word(at, 0x437f0000);
                    }
                    flags.inc(r.eax);
                    ++r.eax;
                    flags.compare(r.eax, 3);
                    if (x86::sreg32(r.eax) >= 3)
                        break;
                }
                goto nextSpot;
            }
            // On each corner, from where it is on the screen.
            r.ecx = 0;
            flags.logic(0);
            for (;;)
            {
                r.eax = r.ecx << 5;
                {
                    const double x = M::mul(m.load(r.eax + 0x55cf9c), m.load(0x53b090));
                    const double y = M::mul(m.load(r.eax + 0x55cfa0), m.load(0x53b094));
                    r.eax = m.word(r.eax + 0x55cfa4);
                    r.edi = r.esi << 6;
                    m.word(ebp - 0x5e, r.eax);
                    r.eax = look + 0x30 + r.edi + 0x28;
                    r.edx = ebp - 0x66;
                    m.store(ebp - 0x66, x);
                    m.store(ebp - 0x62, y);
                }
                if (!callTo(0x4e0090))
                    return;
                {
                    const double inverse = M::div(1.0, result());
                    r.eax = look;
                    const double dx = M::sub(m.load(r.edi + r.eax + 0x58), m.load(ebp - 0x66));
                    m.store(ebp - 0x66, dx);
                    const double dy = M::sub(m.load(ebp - 0x62), m.load(r.edi + r.eax + 0x5c));
                    m.store(ebp - 0x62, dy);
                    const double dy2 = M::mul(dy, m.load(ebp - 0x62));
                    const double dx2 = M::mul(m.load(ebp - 0x66), m.load(ebp - 0x66));
                    esp -= 4;
                    m.store(esp, M::add(dy2, dx2));
                    m.store(ebp + 0x16, inverse);
                }
                if (!callTo(0x4ea920))
                    return;
                {
                    const double distance = result();
                    const double inverse = M::div(1.0, distance);
                    const double dx = M::mul(m.load(ebp - 0x66), inverse);
                    const double dy = M::mul(inverse, m.load(ebp - 0x62));
                    r.edx = m.word(ebp + 0x3e);
                    m.store(ebp - 0x66, dx);
                    m.store(ebp - 0x62, dy);
                    setByte(r.edx, 0, m.byte(r.ecx + r.edx));
                    r.edx &= 0xff;
                    r.eax = r.edx * 12;
                    r.edx = m.word(ebp + 0x3a);
                    flags.add(r.eax, r.edx);
                    r.eax += r.edx;
                    const double dot = M::add(M::mul(m.load(r.eax + 4), m.load(ebp - 0x62)),
                                              M::mul(m.load(r.eax), m.load(ebp - 0x66)));
                    m.store(ebp + 0x22, dot);
                    compareToFlags(0.0, m.load(ebp + 0x22));
                    if (cpu.flags.cf)
                    {
                        const x86::reg32 channels[] = { 0x5a, 0x4e, 0x72 };
                        const x86::reg32 colours[] = { 0x1e, 0x32, 0x2e };
                        for (int c = 0; c < 3; ++c)
                        {
                            const double add = M::mul(M::mul(m.load(ebp + 0x22), m.load(ebp + colours[c])), m.load(ebp + 0x16));
                            r.eax = r.ecx * 4;
                            const x86::reg32 at = r.eax + ebp - channels[c];
                            m.store(at, M::add(add, m.load(at)));
                            flags.compare(m.word(at), 0x437f0000);
                            if (x86::sreg32(m.word(at)) > 0x437f0000)
                                m.word(at, 0x437f0000);
                        }
                    }
                }
                flags.inc(r.ecx);
                ++r.ecx;
                flags.compare(r.ecx, 3);
                if (x86::sreg32(r.ecx) >= 3)
                    break;
            }
        nextSpot:
            flags.inc(r.esi);
            ++r.esi;
            flags.compare(r.esi, 8);
            if (x86::sreg32(r.esi) >= 8)
                break;
        }
    }
    // The sums to the corners' colours, opaque.
    r.eax = 0;
    flags.logic(0);
    for (;;)
    {
        const double red = m.load(ebp + r.eax * 4 - 0x5a);
        const double green = m.load(ebp + r.eax * 4 - 0x4e);
        const double blue = m.load(ebp + r.eax * 4 - 0x72);
        if (!truncate(red, ebp - 0x3a) || !truncate(green, ebp + 0x5e) || !truncate(blue, ebp + 0x5a))
            return;
        r.esi = m.word(ebp - 0x3a);
        r.ecx = m.word(ebp + 0x5e);
        r.edx = m.word(ebp + 0x5a);
        r.esi = ((r.esi << 16) & 0xff0000) | 0xff000000;
        r.ecx = (r.ecx << 8) & 0xff00;
        r.edx &= 0xff;
        r.ecx |= r.esi;
        r.ecx |= r.edx;
        r.edx = r.eax;
        flags.shl(r.edx, 5);
        r.edx <<= 5;
        flags.inc(r.eax);
        ++r.eax;
        m.word(r.edx + 0x55cfac, r.ecx);
        flags.compare(r.eax, 3);
        if (x86::sreg32(r.eax) >= 3)
            break;
    }

draw:
    {
        // 1/w = 1 - depth at +8, then the triangle.
        const double w0 = M::sub(1.0, m.load(0x55cfa8));
        const double w1 = M::sub(1.0, m.load(0x55cfc8));
        const double w2 = M::sub(1.0, m.load(0x55cfe8));
        push(0x55cfdc);
        push(0x55cfbc);
        push(0x55cf9c);
        m.store(0x55cfa4, w0);
        m.store(0x55cfc4, w1);
        m.store(0x55cfe4, w2);
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = r.ebx;
        cpu.ecx = r.ecx;
        cpu.edx = r.edx;
        cpu.esi = r.esi;
        cpu.edi = r.edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        if (!call(app, cpu, m.word(0x9ef974)))
            return;
        r = { cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
        flags.load(cpu);
        esp = cpu.esp;
    }
nextFace:
    flags.inc8(x86::reg8(r.ebx));
    setByte(r.ebx, 0, r.ebx + 1);
    goto face;

advance:
    {
        r.eax = m.word(ebp + 0x52);
        const double width = M::mul(m.load(size), m.load(r.eax + 4));
        r.eax = m.word(ebp + 0x6a);
        m.word(ebp + 0x6e, r.eax);
        if (!truncate(M::add(width, fild16(ebp + 0x6e)), ebp + 0x5e))
            return;
        r.eax = m.word(ebp + 0x5e);
        app->getMemory<x86::reg16>(ebp + 0x6a) = x86::reg16(r.eax);
    }
    goto nextLetter;

alone:
    // No matrix: the rest drawn flat by a call of its own, this letter first.
    r.ecx = m.word(texture);
    r.edx = m.word(ebp + 0x68);
    push(r.ebx);
    r.eax = m.word(ebp + 0x4e);
    r.ebx = m.word(ebp + 0x64);
    push(r.ecx);
    r.edx = x86::reg32(x86::sreg32(r.edx) >> 16);
    push(m.word(size));
    flags.sar(r.ebx, 16);
    r.ebx = x86::reg32(x86::sreg32(r.ebx) >> 16);
    push(0);
    if (!callTo(0x475b50))
        return;
    goto nextLetter;

unknown:
    {
        const double width = M::mul(m.load(size), m.load(0x53b098));
        r.eax = m.word(ebp + 0x6a);
        m.word(ebp + 0x62, r.eax);
        if (!truncate(M::add(width, fild16(ebp + 0x62)), ebp + 0x5e))
            return;
        r.eax = m.word(ebp + 0x5e);
        app->getMemory<x86::reg16>(ebp + 0x6a) = x86::reg16(r.eax);
    }
nextLetter:
    flags.inc(m.word(ebp + 0x4e));
    m.word(ebp + 0x4e, m.word(ebp + 0x4e) + 1);
    goto letter;

done:
    r.eax = look;
    flags.compare(m.word(r.eax + 0x2c), 0);
    if (m.word(r.eax + 0x2c) != 0)
    {
        r.eax = 0xd;
        r.edx = 0;
        if (!callTo(0x431900))
            return;
    }
    return exit();

flat:
    // Flat letters: the colour +0x14, depth 1, 1/w 0.
    r.eax = m.word(look + 0x14);
    m.word(0x55cfac, r.eax);
    m.word(0x55cfcc, r.eax);
    m.word(0x55cfec, r.eax);
    r.eax = m.word(0x55cffc);
    r.edx = r.eax + 1;
    m.word(0x55cffc, r.edx);
    for (;;)
    {
        r.eax = m.word(ebp + 0x4e);
        flags.compare8(m.byte(r.eax), 0);
        if (m.byte(r.eax) == 0)
            return exit();
        r.edx = m.word(ebp + 0x4e);
        r.eax = m.word(0x679010);
        setByte(r.edx, 0, m.byte(r.edx));
        flags.compare8(x86::reg8(r.edx), m.byte(r.eax));
        double width;
        bool known = false;
        if (x86::reg8(r.edx) >= m.byte(r.eax))
        {
            r.edx = byteAt(r.eax);
            r.ebx = byteAt(r.eax + 1);
            r.ecx = m.word(ebp + 0x4e);
            r.ebx += r.edx;
            r.edx = byteAt(r.ecx);
            flags.compare(r.edx, r.ebx);
            known = x86::sreg32(r.edx) < x86::sreg32(r.ebx);
        }
        if (known)
        {
            r.eax += 0x10;
            r.edx <<= 5;
            setByte(r.ebx, 0, 0);
            r.esi = r.eax + r.edx;
            for (;;)
            {
                r.edx = m.word(ebp + 0x4e);
                r.eax = byteAt(r.edx);
                r.edx = r.eax << 5;
                r.eax = m.word(0x679010);
                flags.compare8(x86::reg8(r.ebx), m.byte(r.edx + r.eax + 0x10));
                if (x86::reg8(r.ebx) >= m.byte(r.edx + r.eax + 0x10))
                    break;
                r.edx = x86::reg8(r.ebx) * 3;
                r.eax = m.word(r.esi + 0x1c);
                r.ecx = r.eax + r.edx;
                r.edx = byteAt(r.ecx);
                r.eax = r.edx * 12;
                r.edx = m.word(r.esi + 0x10);
                r.edi = r.edx + r.eax;
                const double letterSize = m.load(size);
                const double x0 = M::mul(m.load(r.edi), letterSize);
                r.eax = m.word(ebp + 0x6a);
                m.word(ebp + 0x62, r.eax);
                r.eax = m.word(ebp + 0x66);
                m.word(ebp + 0x6e, r.eax);
                r.eax = byteAt(r.ecx + 1);
                m.word(ebp + 0x5a, r.eax);
                const double y0 = M::mul(m.load(r.edi + 4), letterSize);
                r.edi = m.word(ebp + 0x5a);
                r.eax = r.eax * 3;
                const double x1 = M::mul(m.load(r.edx + r.eax * 4), letterSize);
                const double y1 = M::mul(m.load(r.edx + r.eax * 4 + 4), letterSize);
                const double y = fild16(ebp + 0x6e);
                m.store(ebp + 0x5e, fild16(ebp + 0x62));
                m.store(0x55cf9c, M::add(x0, m.load(ebp + 0x5e)));
                m.store(0x55cfa0, M::sub(y, y0));
                setByte(r.ecx, 0, m.byte(r.ecx + 2));
                m.store(0x55cfbc, M::add(x1, m.load(ebp + 0x5e)));
                m.store(0x55cfc0, M::sub(y, y1));
                r.ecx &= 0xff;
                r.eax = r.ecx * 3;
                const double x2 = M::mul(m.load(r.edx + r.eax * 4), letterSize);
                const double y2 = M::mul(letterSize, m.load(r.edx + r.eax * 4 + 4));
                push(0x55cfdc);
                r.edi = 0x3f800000;
                flags.inc8(x86::reg8(r.ebx));
                setByte(r.ebx, 0, r.ebx + 1);
                push(0x55cfbc);
                m.word(0x55cfa8, r.edi);
                m.word(0x55cfc8, r.edi);
                m.word(0x55cfe8, r.edi);
                r.eax = 0;
                flags.logic(0);
                push(0x55cf9c);
                m.word(0x55cfa4, 0);
                m.word(0x55cfc4, 0);
                m.word(0x55cfe4, 0);
                m.store(0x55cfdc, M::add(x2, m.load(ebp + 0x5e)));
                m.store(0x55cfe0, M::sub(y, y2));
                flags.store(cpu);
                cpu.eax = r.eax;
                cpu.ebx = r.ebx;
                cpu.ecx = r.ecx;
                cpu.edx = r.edx;
                cpu.esi = r.esi;
                cpu.edi = r.edi;
                cpu.ebp = ebp;
                cpu.esp = esp;
                if (!call(app, cpu, m.word(0x9ef974)))
                    return;
                r = { cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
                flags.load(cpu);
                esp = cpu.esp;
            }
            width = M::mul(m.load(size), m.load(r.esi + 4));
            r.eax = m.word(ebp + 0x6a);
        }
        else
        {
            width = M::mul(m.load(size), m.load(0x53b098));
            r.eax = m.word(ebp + 0x6a);
        }
        m.word(ebp + 0x6e, r.eax);
        if (!truncate(M::add(width, fild16(ebp + 0x6e)), ebp + 0x5e))
            return;
        r.eax = m.word(ebp + 0x5e);
        app->getMemory<x86::reg16>(ebp + 0x6a) = x86::reg16(r.eax);
        flags.inc(m.word(ebp + 0x4e));
        m.word(ebp + 0x4e, m.word(ebp + 0x4e) + 1);
    }
}


/* sub_4bb200: a medium car's lights (car in eax, view in edx; the mirror
 * counts as the main view, a port), as sub_4bae00 does a detailed car's:
 * its model ([car+0x8a4]) and matrix (sub_4206f0), nothing without lights in
 * the model ([+0xf8]); the night set (sub_4b97b0 when [0x6fd4c8]),
 * sub_4ba320, sub_4ba350, the position (y raised by [0x54012c]) for
 * sub_4b93c0; the headlights by sub_49de20 (their value kept), then between
 * sub_4b9140, sub_4b90e0, sub_4b9280 the four lights at +0x108, +0x114,
 * +0x120, +0x12c raised by [car+0x93c] or [car+0x940]; sub_4e0280,
 * sub_4b8740 and sub_4b81a0 last. */
void mediumCarLightsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc8; sub ebp, 0x82
    const x86::reg32 ebp = entry - 20 - 0x82;
    x86::reg32 esp = entry - 20 - 0xc8;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // movsd three times
    auto copy3 = [&]() {
        for (int k = 0; k < 3; ++k)
        {
            m.word(r.edi, m.word(r.esi));
            r.edi += 4;
            r.esi += 4;
        }
    };
    // sub_49de20, its value dropped (fstp st(0))
    auto light = [&]() {
        if (!callTo(0x49de20))
            return false;
        cpu.fpu.count -= 1;
        return true;
    };

    m.word(ebp + 0x7e, r.eax);
    m.word(ebp + 0x76, r.edx);
    r.eax = m.word(r.eax + 0x8a4);
    m.word(ebp + 0x7a, r.eax);
    r.eax = m.word(ebp + 0x7e) + 0xc;
    r.edx = 1;
    if (!callTo(0x4206f0))
        return;
    r.edx = m.word(ebp + 0x7a);
    m.word(ebp + 0x6e, r.eax);
    flags.logic(r.edx);
    if (!r.edx)
        return leave();
    flags.compare(m.word(r.edx + 0xf8), 0);
    if (x86::sreg32(m.word(r.edx + 0xf8)) <= 0)
        return leave();
    flags.compare(m.word(0x6fd4c8), 0);
    if (m.word(0x6fd4c8) != 0)
    {
        r.eax = m.word(ebp + 0x76);
        // port (apply_mirror_detail): the mirror as the main view
        flags.compare(mirrorAsMain(m.word(r.eax)), 1);
        if (mirrorAsMain(m.word(r.eax)) != 1)
        {
            r.edx = m.word(ebp + 0x6e);
            r.eax = m.word(ebp + 0x7e);
            if (!callTo(0x4b97b0))
                return;
            m.word(ebp + 0x6e, r.eax);
        }
    }
    r.ebx = m.word(ebp + 0x6e);
    r.edx = m.word(ebp + 0x7e);
    r.eax = m.word(ebp + 0x76);
    if (!callTo(0x4ba320))
        return;
    r.ebx = m.word(ebp + 0x6e);
    r.edx = m.word(ebp + 0x7e);
    r.eax = m.word(ebp + 0x76);
    if (!callTo(0x4ba350))
        return;
    m.word(ebp + 0x62, r.eax);
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x46 + k, m.word(m.word(ebp + 0x7e) + 0xc0 + k));
    r.ecx = 0;
    r.edi = ebp - 0x46 + 0x24;
    r.esi = m.word(ebp + 0x7e) + 0xc0 + 0x24;
    m.word(ebp + 0x56, m.word(m.word(ebp + 0x7e) + 0x98));
    r.ebx = ebp - 0x46;
    r.edx = ebp - 0x22;
    m.word(ebp + 0x5a, m.word(m.word(ebp + 0x7e) + 0x9c));
    {
        const double up = M::add(m.load(ebp + 0x5a), m.load(0x54012c));
        r.eax = m.word(m.word(ebp + 0x7e) + 0xa0);
        m.word(ebp + 0x5e, r.eax);
        r.eax = m.word(ebp + 0x7e);
        m.store(ebp + 0x5a, up);
    }
    if (!callTo(0x4b93c0))
        return;
    r.eax = m.word(ebp + 0x76);
    r.edi = m.word(ebp + 0x62);
    r.eax += 0x38;
    push(r.edi);
    m.word(ebp + 0x6a, r.eax);
    push(r.eax);
    r.eax = m.word(ebp + 0x76);
    r.ecx = m.word(ebp + 0x7a);
    r.eax += 0x44;
    r.ecx += 0xfc;
    m.word(ebp + 0x72, r.eax);
    push(r.eax);
    r.eax = ebp + 0x56;
    r.ebx = ebp - 0x22;
    push(r.eax);
    r.eax = ebp - 0x46;
    r.edx = 0;
    push(r.eax);
    r.eax = m.word(ebp + 0x7a);
    r.edi = ebp + 0x4a;
    if (!callTo(0x49de20))
        return;
    app->getMemory<float>(ebp + 0x66) = float(cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    r.ebx = 1;
    r.edx = ebp + 2;
    r.eax = m.word(ebp + 0x7e);
    r.ecx = m.word(ebp + 0x6a);
    if (!callTo(0x4b9140))
        return;
    r.eax = m.word(ebp + 0x7e);
    r.edx = 0;
    r.ebx = m.word(ebp + 0x72);
    if (!callTo(0x4b90e0))
        return;
    // light 1
    r.eax = m.word(ebp + 0x7a);
    push(0);
    r.edx = 1;
    r.esi = r.eax + 0x108;
    push(r.ecx);
    r.eax = m.word(ebp + 0x7e);
    copy3();
    push(r.ebx);
    {
        const double raised = M::add(m.load(ebp + 0x4e), m.load(m.word(ebp + 0x7e) + 0x93c));
        r.eax = ebp + 0x56;
        r.ecx = ebp + 0x4a;
        push(r.eax);
        r.eax = ebp - 0x46;
        r.ebx = ebp + 2;
        push(r.eax);
        r.eax = m.word(ebp + 0x7a);
        m.store(ebp + 0x4e, raised);
    }
    if (!light())
        return;
    // light 2
    r.eax = m.word(ebp + 0x7a);
    r.edi = ebp + 0x4a;
    r.esi = r.eax + 0x114;
    push(0);
    r.ecx = ebp + 0x4a;
    copy3();
    r.eax = m.word(ebp + 0x7e);
    {
        const double raised = M::add(m.load(ebp + 0x4e), m.load(m.word(ebp + 0x7e) + 0x93c));
        r.esi = m.word(ebp + 0x6a);
        r.edi = m.word(ebp + 0x72);
        push(r.esi);
        push(r.edi);
        r.eax = ebp + 0x56;
        r.ebx = ebp + 2;
        push(r.eax);
        r.eax = ebp - 0x46;
        r.edx = 2;
        push(r.eax);
        r.eax = m.word(ebp + 0x7a);
        m.store(ebp + 0x4e, raised);
    }
    if (!light())
        return;
    r.edx = ebp + 2;
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b9280))
        return;
    r.edx = 1;
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b90e0))
        return;
    // light 3
    r.eax = m.word(ebp + 0x7a);
    r.edi = ebp + 0x4a;
    r.esi = r.eax + 0x120;
    push(0);
    copy3();
    r.eax = m.word(ebp + 0x7e);
    {
        const double raised = M::add(m.load(ebp + 0x4e), m.load(m.word(ebp + 0x7e) + 0x940));
        r.eax = m.word(ebp + 0x6a);
        r.ecx = ebp + 0x4a;
        push(r.eax);
        r.edx = m.word(ebp + 0x72);
        r.ebx = ebp + 2;
        push(r.edx);
        r.eax = ebp + 0x56;
        r.edi = ebp + 0x4a;
        push(r.eax);
        r.eax = ebp - 0x46;
        r.edx = 3;
        push(r.eax);
        r.eax = m.word(ebp + 0x7a);
        m.store(ebp + 0x4e, raised);
    }
    if (!light())
        return;
    // light 4
    r.eax = m.word(ebp + 0x7a);
    push(0);
    r.ecx = m.word(ebp + 0x6a);
    r.ebx = m.word(ebp + 0x72);
    r.edx = 4;
    r.esi = r.eax + 0x12c;
    push(r.ecx);
    r.eax = m.word(ebp + 0x7e);
    copy3();
    push(r.ebx);
    {
        const double raised = M::add(m.load(ebp + 0x4e), m.load(m.word(ebp + 0x7e) + 0x940));
        r.eax = ebp + 0x56;
        r.ecx = ebp + 0x4a;
        push(r.eax);
        r.eax = ebp - 0x46;
        r.ebx = ebp + 2;
        push(r.eax);
        r.eax = m.word(ebp + 0x7a);
        m.store(ebp + 0x4e, raised);
    }
    if (!light())
        return;
    r.ebx = ebp + 0x26;
    r.edx = ebp - 0x46;
    r.eax = ebp - 0x22;
    r.esi = m.word(ebp + 0x6e);
    r.ecx = ebp + 0x56;
    if (!callTo(0x4e0280))
        return;
    r.ebx = ebp + 0x26;
    push(r.esi);
    r.edx = m.word(ebp + 0x7e);
    r.eax = m.word(ebp + 0x76);
    push(m.word(ebp + 0x66));
    if (!callTo(0x4b8740))
        return;
    r.edx = m.word(ebp + 0x76);
    r.eax = m.word(ebp + 0x7e);
    if (!callTo(0x4b81a0))
        return;
    leave();
}

/* sub_4b97b0: a car's colour at night (car in eax, its colour in edx; the
 * colour back in eax): the other cars' headlights (+0x734, with bit 0x20 of
 * +0x200) and lights (+0x660) that reach it -- turned to it by sub_4e0430,
 * ahead and within their range (+0x64 .. +0x6c), inside their cone -- add
 * their colours (+0x70, +0x74 when behind) weighted by nearness (and by
 * nearness to the beam's axis for the lights); the average is added to the
 * colour's channels, each held to 0..255, alpha opaque. */
void nightColourNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x68
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = entry - 20 - 0x68;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // fcomp, fnstsw ax, sahf
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    // fistp to the stack and pop eax
    auto integer = [&](double value) {
        const x86::reg32 bits = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(value)));
        m.word(esp - 4, bits);
        return bits;
    };
    // |[from]| to `to`: fld, fchs, fstp when below 0, else mov
    auto magnitude = [&](x86::reg32 from, x86::reg32 to) {
        compareToFlags(0.0, m.load(from));
        if (cpu.flags.cf || cpu.flags.zf)
        {
            r.eax = m.word(from);
            m.word(to, r.eax);
        }
        else
            m.store(to, -m.load(from));
    };
    auto above = [&]() { return !cpu.flags.cf && !cpu.flags.zf; };

    const x86::reg32 car = r.eax;
    m.word(ebp - 0x24, r.eax);
    m.word(ebp - 0x20, r.edx);
    m.word(ebp - 0x50, m.word(car + 0x98));
    r.edi = 0;
    m.word(ebp - 0x14, 0);
    m.word(ebp - 0x4c, m.word(car + 0x9c));
    m.word(ebp - 0x10, 0);
    m.word(ebp - 8, 0);
    r.eax = m.word(car + 0xa0);
    m.word(ebp - 0x28, 0);
    m.word(ebp - 0x48, r.eax);
    for (;; flags.inc(m.word(ebp - 0x28)), m.word(ebp - 0x28, m.word(ebp - 0x28) + 1))
    {
        r.eax = m.word(ebp - 0x28);
        flags.compare(r.eax, m.word(0x5efd9c));
        if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x5efd9c)))
            break;
        r.esi = m.word(r.eax * 4 + 0x5efac8);
        setByte(r.eax, 1, m.byte(r.esi + 0x200));
        const x86::reg8 state = x86::reg8(r.eax >> 8);
        flags.logic8(state & 0x40);
        if (state & 0x40)
            continue;
        flags.compare8(m.byte(r.esi + 0x89), 0);
        if (!m.byte(r.esi + 0x89))
            continue;
        flags.compare8(m.byte(r.esi + 0x88), 0);
        if (m.byte(r.esi + 0x88))
            continue;
        flags.compare(r.esi, m.word(ebp - 0x24));
        if (r.esi == m.word(ebp - 0x24))
            continue;
        flags.logic8(state & 0x20);
        bool headlights = state & 0x20;
        if (headlights)
        {
            flags.compare(m.word(r.esi + 0x734), 0);
            headlights = m.word(r.esi + 0x734) != 0;
        }
        if (headlights)
        {
            r.eax = r.esi + 0x734;
            m.word(ebp - 0xc, r.eax);
            r.edx = ebp - 0x50;
            r.ecx = m.word(ebp - 0xc);
            r.eax = m.word(r.eax + 0x70);
            r.ebx = m.word(ebp - 0xc);
            m.word(ebp - 0x2c, r.eax);
            r.eax = ebp - 0x68;
            r.ecx += 0x58;
            push(r.eax);
            flags.add(r.ebx, 0x28);
            r.ebx += 0x28;
            r.eax = 1;
            if (!callTo(0x4e0430))
                return;
            compareToFlags(0.0, m.load(ebp - 0x60));
            if (above())
            {
                // Behind it: z the other way, the rear colour.
                setByte(r.ebx, 1, m.byte(ebp - 0x5d) ^ 0x80);
                flags.logic8(x86::reg8(r.ebx >> 8));
                r.eax = m.word(ebp - 0xc);
                m.byte(ebp - 0x5d, x86::reg8(r.ebx >> 8));
                r.eax = m.word(r.eax + 0x74);
                m.word(ebp - 0x2c, r.eax);
            }
            r.eax = m.word(ebp - 0xc);
            compareToFlags(m.load(ebp - 0x60), m.load(r.eax + 0x64));
            if (cpu.flags.cf)
                continue;
            r.eax = m.word(ebp - 0xc);
            compareToFlags(m.load(ebp - 0x60), m.load(r.eax + 0x6c));
            if (above())
                continue;
            magnitude(ebp - 0x68, ebp - 0x40);
            compareToFlags(m.load(ebp - 0x40), m.load(ebp - 0x60));
            if (above())
                continue;
            magnitude(ebp - 0x64, ebp - 0x34);
            compareToFlags(m.load(ebp - 0x34), m.load(ebp - 0x60));
            if (above())
                continue;
            r.eax = m.word(ebp - 0xc);
            const double q = M::div(m.load(ebp - 0x60), m.load(r.eax + 0x6c));
            const x86::reg32 colour = m.word(ebp - 0x2c);
            r.edx = (colour >> 16) & 0xff;
            m.word(ebp - 4, r.edx);
            m.store(ebp - 0x44, M::sub(1.0, q));
            const x86::reg32 red = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x44)));
            r.ecx = (colour >> 8) & 0xff;
            r.ebx = colour & 0xff;
            m.word(ebp - 4, r.ecx);
            const x86::reg32 green = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x44)));
            m.word(ebp - 4, r.ebx);
            r.ebx = m.word(ebp - 8) + 1;
            r.edi += red;
            const x86::reg32 blue = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x44)));
            r.edx = m.word(ebp - 0x14) + green;
            r.ecx = m.word(ebp - 0x10) + blue;
            r.eax = blue;
            m.word(ebp - 8, r.ebx);
            m.word(ebp - 0x14, r.edx);
            m.word(ebp - 0x10, r.ecx);
        }
        flags.compare(m.word(r.esi + 0x660), 0);
        if (m.word(r.esi + 0x660) == 0)
            continue;
        r.eax = r.esi + 0x660;
        r.edx = ebp - 0x50;
        m.word(ebp - 0x18, r.eax);
        r.eax = ebp - 0x5c;
        r.ecx = r.esi + 0x6b8;
        push(r.eax);
        r.ebx = r.esi + 0x688;
        r.eax = 1;
        if (!callTo(0x4e0430))
            return;
        r.eax = m.word(ebp - 0x18);
        compareToFlags(m.load(ebp - 0x54), m.load(r.eax + 0x64));
        if (cpu.flags.cf)
            continue;
        r.eax = m.word(ebp - 0x18);
        compareToFlags(m.load(ebp - 0x54), m.load(r.eax + 0x6c));
        if (above())
            continue;
        magnitude(ebp - 0x5c, ebp - 0x38);
        compareToFlags(m.load(ebp - 0x38), m.load(ebp - 0x54));
        if (above())
            continue;
        magnitude(ebp - 0x58, ebp - 0x30);
        compareToFlags(m.load(ebp - 0x30), m.load(ebp - 0x54));
        if (above())
            continue;
        r.eax = m.word(ebp - 0x18);
        m.store(ebp - 0x1c, M::sub(1.0, M::div(m.load(ebp - 0x54), m.load(r.eax + 0x6c))));
        magnitude(ebp - 0x5c, ebp - 0x3c);
        {
            const double across = M::div(m.load(ebp - 0x3c), m.load(ebp - 0x54));
            const x86::reg32 colour = m.word(m.word(ebp - 0x18) + 0x70);
            r.edx = (colour >> 16) & 0xff;
            const double weight = M::mul(M::sub(1.0, across), m.load(ebp - 0x1c));
            m.word(ebp - 4, r.edx);
            m.store(ebp - 0x1c, weight);
            const x86::reg32 red = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x1c)));
            r.esi = (colour >> 8) & 0xff;
            r.eax = m.word(ebp - 0x18);
            r.ebx = colour & 0xff;
            m.word(ebp - 4, r.esi);
            const x86::reg32 green = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x1c)));
            m.word(ebp - 4, r.ebx);
            r.esi = m.word(ebp - 8) + 1;
            r.ebx = m.word(ebp - 0x10);
            m.word(ebp - 8, r.esi);
            r.edi += red;
            r.edx = green;
            const x86::reg32 blue = integer(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x1c)));
            r.ecx = m.word(ebp - 0x14) + green;
            r.eax = blue;
            flags.add(r.ebx, r.eax);
            r.ebx += r.eax;
            m.word(ebp - 0x14, r.ecx);
            m.word(ebp - 0x10, r.ebx);
        }
    }
    flags.compare(m.word(ebp - 8), 0);
    if (m.word(ebp - 8) != 0)
    {
        const x86::sreg32 count = x86::sreg32(m.word(ebp - 8));
        r.ecx = x86::reg32(count);
        r.edi = x86::reg32(x86::sreg32(r.edi) / count);
        const x86::sreg32 green = x86::sreg32(m.word(ebp - 0x14));
        m.word(ebp - 0x14, x86::reg32(green / count));
        const x86::sreg32 blue = x86::sreg32(m.word(ebp - 0x10));
        r.edx = x86::reg32(blue % count);
        m.word(ebp - 0x10, x86::reg32(blue / count));
        // A channel of the colour plus the average, held to 0..255.
        auto channel = [&](x86::reg32 value) {
            flags.logic(value);
            if (x86::sreg32(value) < 0)
            {
                flags.logic(0);
                return x86::reg32(0);
            }
            flags.compare(value, 0xff);
            return x86::sreg32(value) > 0xff ? x86::reg32(0xff) : value;
        };
        const x86::reg32 colour = m.word(ebp - 0x20);
        r.edx = channel(((colour >> 16) & 0xff) + r.edi);
        r.edi = m.word(ebp - 0x14);
        r.ebx = channel(((colour >> 8) & 0xff) + r.edi);
        r.ecx = m.word(ebp - 0x10);
        r.eax = channel((colour & 0xff) + r.ecx);
        r.ecx = ((r.edx & 0xff) << 16) | 0xff000000;
        r.edx = ((r.ebx & 0xff) << 8) | r.ecx | (r.eax & 0xff);
        m.word(ebp - 0x20, r.edx);
    }
    flags.store(cpu);
    cpu.eax = m.word(ebp - 0x20);
    cpu.edx = r.edx;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.esi = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_41b9b0: a view's pass (view in eax): the glare colour at 0x552e1c --
 * grey, darkened by the sun's glare [0x6fbc38] when that is below 1 (on
 * every driver, a port) -- then sub_41aaf0 for the view, the cars' sub_42d160
 * for the main view, sub_41cdc0 with the track's record, the four
 * sub_4c7790s, sub_41b8c0, the two sub_4c7940s, sub_4217e0 and sub_494170. */
void viewPassNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp };
    // push ebx, ecx, edx, esi, ebp; mov ebp, esp; sub esp, 0x38
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = entry - 20 - 0x38;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };

    r.esi = r.eax;
    r.edx = 0x7f7f7f7f;
    setByte(r.eax, 1, m.byte(0x7a3a58));
    m.word(0x552e1c, r.edx);
    // port (apply_alpha_intensity): test ah, 0x40 as if set
    flags.logic8(0x40);
    flags.store(cpu);  // sahf below takes CF, ZF, SF over; OF stays
    {
        const double glare = m.load(0x6fbc38);
        app->getMemory<double>(ebp - 8) = glare;
        cpu.fpu.compare(x86::Float(1.0), x86::Float(glare));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        if (cpu.flags.cf || cpu.flags.zf)
            goto cars;
        const x86::reg32 scale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x536cac)))));
        m.word(esp - 4, scale);
        r.edx = m.word(0x552e1c);
        r.ecx = scale;
        r.eax = r.edx;
        flags.compare(r.ecx, 0x10000);
        if (x86::sreg32(r.ecx) >= 0x10000)
            r.ecx = 0xffff;
        r.ecx = (r.ecx >> 8) & 0xff;
        r.ebx = (r.eax & 0xff00ff00) >> 8;
        r.eax &= 0xff00ff;
        std::uint64_t product = std::uint64_t(r.eax) * r.ecx;
        r.eax = r.ebx;
        r.ebx = x86::reg32(product);
        product = std::uint64_t(r.eax) * r.ecx;
        r.eax = x86::reg32(product);
        r.edx = x86::reg32(product >> 32);
        r.ebx = (r.ebx >> 8) & 0xff00ff;
        r.eax = (r.eax & 0xff00ff00) | r.ebx;
        m.word(0x552e1c, r.eax);
    }
cars:
    r.eax = m.word(r.esi);
    flags.logic(r.eax);
    if (r.eax == 0)
    {
        r.eax = r.esi;
        r.edx = m.word(r.esi + 4);
        if (!callTo(0x41aaf0))
            return;
    }
    else
    {
        flags.compare(r.eax, 1);
        if (r.eax == 1)
        {
            r.edx = r.eax;
            r.eax = r.esi;
            if (!callTo(0x41aaf0))
                return;
        }
    }
    flags.compare(m.word(r.esi), 0);
    if (m.word(r.esi) == 0)
    {
        flags.compare(m.word(r.esi + 4), 0);
        if (m.word(r.esi + 4) == 0)
        {
            for (r.edx = 0;; )
            {
                flags.compare(r.edx, m.word(0x5efd9c));
                if (x86::sreg32(r.edx) >= x86::sreg32(m.word(0x5efd9c)))
                    break;
                r.eax = m.word(r.edx * 4 + 0x5efac8);
                setByte(r.ebx, 0, m.byte(r.eax + 0x200));
                flags.logic8(x86::reg8(r.ebx) & 0x40);
                if (!(r.ebx & 0x40))
                {
                    flags.compare8(m.byte(r.eax + 0x88), 0);
                    if (!m.byte(r.eax + 0x88) && !callTo(0x42d160))
                        return;
                }
                flags.inc(r.edx);
                ++r.edx;
            }
        }
    }
    r.edx = m.word(m.word(0x5dd830) + 0x10);
    r.eax = r.edx * 0x5c0 + 0x571370;
    r.edx = r.eax + 0xb4;
    r.ebx = ebp - 0x38;
    r.eax = r.esi;
    if (!callTo(0x41cdc0))
        return;
    const x86::reg32 slots[] = { 0x7dcfe8, 0x7dcff4, 0x7dcff0, 0x7dcfe4 };
    const x86::reg32 values[] = { 0x38, 0x30, 0x20, 0x28 };
    for (int k = 0; k < 4; ++k)
    {
        r.eax = m.word(ebp - values[k]);
        push(m.word(slots[k]));
        if (!callTo(0x4c7790))
            return;
    }
    r.eax = ebp - 0x10;
    if (!callTo(0x41b8c0))
        return;
    r.eax = m.word(ebp - 0xc);
    push(m.word(0x7dcff8));
    push(r.eax);
    r.edx = m.word(ebp - 0x10);
    push(r.edx);
    if (!callTo(0x4c7940))
        return;
    r.ecx = m.word(ebp - 0x14);
    push(m.word(0x7dcff8));
    push(r.ecx);
    r.ebx = m.word(ebp - 0x18);
    push(r.ebx);
    if (!callTo(0x4c7940))
        return;
    r.eax = m.word(r.esi + 4);
    if (!callTo(0x4217e0))
        return;
    r.edx = r.eax;
    r.eax = r.esi;
    if (!callTo(0x494170))
        return;
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.edi = r.edi;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_434870: the polygon lists' drawers for the driver's abilities
 * ([0x7a3a58]): the five pointers at 0x5f0780.. and 0x5f0788. */
void polygonDrawersNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    IntegerFlags flags;
    m.word(0x5f0788, 0x433d40);
    setByte(cpu.eax, 1, m.byte(0x7a3a58));
    const x86::reg8 abilities = x86::reg8(cpu.eax >> 8);
    flags.logic8(abilities & 0x40);
    if (abilities & 0x40)
    {
        m.word(0x5f0784, 0x4336e0);
        m.word(0x5f0790, 0x4339d0);
        m.word(0x5f0794, 0x434850);
        m.word(0x5f078c, 0x4347b0);
        m.word(0x5f0780, 0x433840);
        cpu.eax = 0x4347b0;
    }
    else
    {
        flags.logic8(abilities & 6);
        if (abilities & 6)
        {
            m.word(0x5f0784, 0x433530);
            m.word(0x5f0790, 0x433660);
            m.word(0x5f0794, 0x434850);
            m.word(0x5f078c, 0x434120);
            m.word(0x5f0780, 0x4332b0);
        }
        else
        {
            m.word(0x5f0784, 0x433a50);
            m.word(0x5f0790, 0x4347d0);
            m.word(0x5f0794, 0x434850);
            m.word(0x5f078c, 0x4347b0);
            m.word(0x5f0780, 0x433bb0);
            cpu.eax = 0x433bb0;
        }
    }
    flags.store(cpu);
    cpu.esp += 4;
}


/* sub_49d800: the two colours in eax and edx to 0x55e6dc and 0x55e6e0,
 * darkened by the sun's glare [0x6fbc38] times [0x53c698] when that is below
 * 1 (on every driver, a port). */
void glareColoursNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 8
    const x86::reg32 ebp = entry - 20;
    const x86::reg32 esp = entry - 20 - 8;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    r.edi = r.eax;
    r.esi = r.edx;
    // port (apply_alpha_intensity): test byte [0x7a3a58], 0x40 as if set
    flags.logic8(0x40);
    const double glare = m.load(0x6fbc38);
    app->getMemory<double>(ebp - 8) = glare;
    if (glareBelowOne(cpu, flags, r, glare))
    {
        const x86::reg32 first = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x53c698)))));
        m.word(esp - 4, first);
        darken(flags, r, r.edi, first);
        const x86::reg32 second =
            x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(m.load(0x6fbc38), m.loadDouble(0x53c698)))));
        r.edi = r.eax;
        m.word(esp - 4, second);
        darken(flags, r, r.esi, second);
        r.esi = r.eax;
    }
    m.word(0x55e6e0, r.esi);
    m.word(0x55e6dc, r.edi);
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.edx = r.edx;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.esi = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_472d10: the detail settings for the machine's speed (sub_4f1ef0 in
 * MHz: up to 166, 200, 233, 266 or above), halved on a driver with bit 6 of
 * [0x7a3a58]: the eight words of each of the tables at 0x472168.. to
 * 0x6fbc28 (the view distance, kept full: a port), 0x6fbc2c, 0x6fbc1c,
 * 0x6fbc30, 0x6fbc20, 0x6fbc24, 0x6fbc3c and 0x791b28; the glare at 1 (0.5
 * when sub_4ee310 finds the driver's name at 0x53b01c), the screen width
 * for sub_4bee80, and sub_4483a0, sub_448330's choice of 0x6fbc34. */
void detailSettingsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x174
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = entry - 24 - 0x174;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    // The eight tables of five words (movsd, movsd, movsw).
    const x86::reg32 tables[] = { 0x60, 0x30, 0x54, 0x48, 0x3c, 0xc, 0x24, 0x18 };
    for (x86::reg32 k = 0; k < 8; ++k)
    {
        if (k == 7)
        {
            r.edx = 2;
            m.word(0x6fd3a8, r.edx);
        }
        for (x86::reg32 b = 0; b < 10; b += 2)
            app->getMemory<x86::reg16>(ebp - tables[k] + b) = app->getMemory<x86::reg16>(0x472168 + 10 * k + b);
    }
    r.edi = ebp - 0x18 + 10;
    r.esi = 0x472168 + 80;
    if (!callTo(0x4f1c70))
        return;
    r.ebx = m.word(0x5643ac);
    r.ecx = 4;
    flags.logic(r.ebx);
    bool measure = r.ebx != 0;
    if (!measure)
    {
        flags.compare(m.word(0x5643b0), 0);
        measure = m.word(0x5643b0) != 0;
    }
    if (measure)
    {
        if (!callTo(0x4f1ef0))
            return;
        r.esi = 1000000;
        const x86::sreg32 ticks = x86::sreg32(r.eax);
        r.edx = x86::reg32(ticks % 1000000);
        r.eax = x86::reg32(ticks / 1000000);
        const x86::sreg32 mhz = x86::sreg32(r.eax);
        flags.compare(r.eax, 0x10a);
        if (mhz <= 0x10a)
            r.ecx = 3;
        flags.compare(r.eax, 0xe9);
        if (mhz <= 0xe9)
            r.ecx = 2;
        flags.compare(r.eax, 0xc8);
        if (mhz <= 0xc8)
            r.ecx = 1;
        flags.compare(r.eax, 0xa6);
        if (mhz <= 0xa6)
            r.ecx = 0;
    }
    flags.logic8(m.byte(0x7a3a58) & 6);
    if (m.byte(0x7a3a58) & 6)
    {
        r.edx = x86::reg32(x86::sreg32(r.ecx) >> 31);
        r.eax = x86::reg32(x86::sreg32(r.ecx - r.edx) >> 1);
        r.ecx = r.eax;
    }
    auto entryOf = [&](x86::reg32 table) { return x86::reg32(x86::sreg32(m.word(ebp + r.ecx * 2 - table - 2)) >> 16); };
    r.edx = entryOf(0x60);
    m.word(0x6fbc28, 0);  // port (apply_view_distance): View Distance Full at any CPU speed
    r.edx = entryOf(0x30);
    m.word(0x6fbc2c, r.edx);
    r.edx = entryOf(0x48);
    r.edi = 1;
    r.ebx = 0x3f800000;
    m.word(0x6fbc1c, r.edx);
    r.edx = entryOf(0x3c);
    r.eax = entryOf(0x18);
    m.word(0x6fbc30, r.edx);
    r.edx = entryOf(0xc);
    m.word(0x6fbc44, r.edi);
    m.word(0x6fbc48, r.edi);
    m.word(0x6fbc20, r.edx);
    r.edx = entryOf(0x24);
    m.word(0x6fbc38, r.ebx);
    m.word(0x6fbc3c, r.eax);
    m.word(0x6fbc24, r.edx);
    r.edx = 0;
    r.eax = m.word(0x7a3a58);
    m.word(0x6fbc4c, r.edx);
    setByte(r.edx, 0, m.byte(0x7a3a58));
    m.word(0x6fbc10, r.eax);
    flags.logic8(x86::reg8(r.edx) & 0x40);
    if (r.edx & 0x40)
    {
        r.edx = 0x53b01c;
        r.eax = 0x7a3a74;
        if (!callTo(0x4ee310))
            return;
        flags.logic(r.eax);
        if (!r.eax)
            m.word(0x6fbc38, 0x3f000000);
    }
    flags.logic8(m.byte(0x7a3a58) & 6);
    if (m.byte(0x7a3a58) & 6)
    {
        r.edi = 0;
        flags.logic(0);
        r.eax = 0x140;
        m.word(0x6fbc3c, r.edi);
        m.word(0x6fbc34, r.edi);
    }
    else
        r.eax = 0x280;
    if (!callTo(0x4bee80) || !callTo(0x4483a0))
        return;
    r.esi = ebp - 0x174;
    if (!callTo(0x448330))
        return;
    r.edx = m.word(ebp - 0x164);
    flags.logic(r.edx);
    if (r.edx)
        m.word(0x6fbc34, 1);
    else
    {
        m.word(0x6fbc3c, r.edx);
        m.word(0x6fbc34, r.edx);
    }
    r.eax = entryOf(0x54);
    m.word(0x791b28, r.eax);
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_4ddb40: the sparks of a group (eax: the group at +4) -- the
 * [0x8ca278] records of 20 bytes from 0x8c08c0 + group * 400: x, y, a
 * count, a size, an angle -- each with a count drawn as a quad record
 * (sub_4bbf80, 0xa0 bytes, flags 1, texture 0x8ca288) turned by the angle
 * (sub_4ea950, sub_4ea930), its size scaled by the count over [0x8ca27c]
 * when [0x8ca22c] is 1, its grey [0x8ca284] * count / [0x8ca27c] darkened by
 * the sun's glare times [0x5497dc] when that is below 1 (on every driver, a
 * port), depth 0.1, 1/w 0.9, with the corners' clip codes. */
void sparksNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x5c
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = entry - 24 - 0x5c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };

    {
        const double scaled = M::mul(m.load(0x6fbc38), m.loadDouble(0x5497dc));
        r.eax = m.word(r.eax + 4);
        r.edx = 0;
        m.word(ebp - 0x20, r.eax);
        r.eax = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(scaled)));
        m.word(esp - 4, r.eax);
        m.word(ebp - 0x18, r.edx);
        m.word(ebp - 0x44, r.eax);
    }
    for (;; flags.inc(m.word(ebp - 0x18)), m.word(ebp - 0x18, m.word(ebp - 0x18) + 1))
    {
        r.eax = m.word(ebp - 0x18);
        flags.compare(r.eax, m.word(0x8ca278));
        if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x8ca278)))
            break;
        r.ebx = m.word(ebp - 0x20);
        r.edi = m.word(ebp - 0x18);
        r.eax = r.ebx * 400;
        r.ecx = r.edi * 20 + r.eax;
        const x86::reg32 spark = r.ecx + 0x8c08c0;
        r.edi = ebp - 0x5c;
        m.word(ebp - 0x1c, m.word(spark + 0xc));
        r.eax = m.word(spark + 0x10);
        m.word(ebp - 0x5c, m.word(spark));
        m.word(ebp - 0x58, m.word(spark + 4));
        r.edi = ebp - 0x54;
        r.esi = spark + 8;
        m.word(ebp - 0x24, r.eax);
        r.eax = m.word(spark + 8);
        flags.logic(r.eax);
        if (!r.eax)
            continue;
        r.edx = m.word(0x8ca284) * r.eax;
        r.esi = m.word(0x8ca27c);
        {
            const x86::sreg32 dividend = x86::sreg32(r.edx);
            r.eax = x86::reg32(dividend / x86::sreg32(r.esi));
            r.edx = x86::reg32(dividend % x86::sreg32(r.esi));
        }
        r.edi = m.word(0x8ca22c);
        r.ebx = r.eax;
        flags.compare(r.edi, 1);
        if (r.edi == 1)
            m.store(ebp - 0x1c, M::div(M::mul(fild32(m, spark + 8), m.load(ebp - 0x1c)), fild32(m, 0x8ca27c)));
        push(m.word(ebp - 0x24));
        if (!callTo(0x4ea950))
            return;
        const double a = M::mul(result(), m.load(ebp - 0x1c));
        push(m.word(ebp - 0x24));
        m.store(ebp - 0x4c, a);
        if (!callTo(0x4ea930))
            return;
        const double b = M::mul(result(), m.load(ebp - 0x1c));
        {
            const double x = m.load(ebp - 0x5c), y = m.load(ebp - 0x58), aa = m.load(ebp - 0x4c);
            const double sx = m.load(0x56009c), sy = m.load(0x5600a0), cx = m.load(0x5600a4), cy = m.load(0x5600a8);
            const double y1 = M::mul(M::add(y, aa), sy);
            const double x1 = M::mul(M::add(x, aa), sx);
            const double y2 = M::mul(M::sub(y, aa), sy);
            const double x2 = M::mul(M::sub(x, aa), sx);
            m.store(ebp - 0x54, b);
            const double bb = m.load(ebp - 0x54);
            const double x3 = M::mul(M::sub(x, bb), sx);
            const double y3 = M::mul(M::add(y, bb), sy);
            m.store(ebp - 0x48, M::add(y1, cy));
            const double x4 = M::mul(M::add(m.load(ebp - 0x5c), bb), sx);
            const double y4 = M::mul(M::sub(m.load(ebp - 0x58), bb), sy);
            m.store(ebp - 0x3c, M::add(x1, cx));
            m.store(ebp - 0x30, M::add(y2, cy));
            m.store(ebp - 0x28, M::add(x2, cx));
            m.store(ebp - 0x40, M::add(x3, cx));
            m.store(ebp - 0x38, M::add(y3, cy));
            m.store(ebp - 0x34, M::add(x4, cx));
            m.store(ebp - 0x2c, M::add(y4, cy));
        }
        r.edx = 0xa0;
        r.eax = ebp - 0x50;
        if (!callTo(0x4bbf80))
            return;
        flags.logic(r.eax);
        if (!r.eax)
            break;
        const x86::reg32 record = m.word(ebp - 0x50);
        m.word(ebp - 4, record + 0x20);
        m.word(ebp - 0x10, record + 0x40);
        m.word(ebp - 0xc, record + 0x60);
        r.eax = record + 0x80;
        r.edx = r.ebx;
        m.word(ebp - 8, r.eax);
        r.edx = (r.ebx << 16) | (r.ebx << 24);
        r.eax = (r.ebx << 8) | r.edx | r.ebx;
        m.word(ebp - 0x14, r.eax);
        setByte(r.eax, 1, m.byte(0x7a3a58));
        // port (apply_alpha_intensity): test ah, 0x40 as if set
        flags.logic8(0x40);
        if (glareBelowOne(cpu, flags, r, m.load(0x6fbc38)))
        {
            darken(flags, r, m.word(ebp - 0x14), m.word(ebp - 0x44));
            m.word(ebp - 0x14, r.eax);
        }
        r.eax = m.word(ebp - 0x50);
        app->getMemory<x86::reg16>(r.eax + 4) = 0;
        app->getMemory<x86::reg16>(r.eax + 6) = 1;
        m.word(r.eax + 0x18, 0x8ca288);
        m.word(r.eax + 8, m.word(ebp - 4));
        m.word(r.eax + 0xc, m.word(ebp - 0x10));
        m.word(r.eax + 0x10, m.word(ebp - 0xc));
        r.edx = m.word(ebp - 8);
        m.word(r.eax + 0x14, r.edx);
        // The corners: (x, y) from the frame, their clip codes, the grey.
        const x86::reg32 corners[] = { 4, 0x10, 0xc, 8 };
        const x86::reg32 xs[] = { 0x3c, 0x40, 0x28, 0x34 };
        const x86::reg32 ys[] = { 0x38, 0x48, 0x2c, 0x30 };
        for (int k = 0; k < 4; ++k)
        {
            r.edx = m.word(ebp - corners[k]);
            m.word(r.edx + 8, 0x3dcccccd);
            r.eax = m.word(ebp - xs[k]);
            m.word(r.edx + 0xc, 0x3f666666);
            m.word(r.edx, r.eax);
            r.eax = m.word(ebp - ys[k]);
            m.word(r.edx + 4, r.eax);
            screenCodeFlags(m, r.edx, r.edx + 0x14, r, flags);
            r.edx = m.word(ebp - corners[k]);
            r.eax = m.word(ebp - 0x14);
            m.word(r.edx + 0x10, r.eax);
        }
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_491190: a light's glow, stdcall (view in eax; +4, +8, +0xc the light's
 * position as integers, +0x10 its kind): unless the detail [0x6fbc30] is 2
 * or the kind's blinking (0x55e3e0, 12 bytes a kind: phase, shift) has it
 * off, the position times [0x53bca0] turned by the view's matrix (+0x44)
 * and moved (+0x38), between 1 and 256 deep, projected and clip-coded twice
 * (1/z then 1/z times [0x53bca8]), and on the screen: two quad records
 * (sub_4bbde0, 0xa0 bytes, flags 1) of the kind's colour (darkened by the
 * sun's glare when that is below 1, on every driver as a port), the inner
 * one also by the depth, each a square of the kind's size turned by an angle
 * from x / z (sub_4ea930, sub_4ea950), textures 0x8b47bc and 0x8b4840; the
 * second linked to the first and handed to sub_4c7610. */
void glowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x9c; sub ebp, 0x72
    const x86::reg32 ebp = entry - 24 - 0x72;
    x86::reg32 esp = entry - 24 - 0x9c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4 + 0x10;
    };
    // rep movsd of a 0x20-byte vertex
    auto copyVertex = [&](x86::reg32 to, x86::reg32 from) {
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            m.word(to + k, m.word(from + k));
        r.ecx = 0;
        r.edi = to + 0x20;
        r.esi = from + 0x20;
    };
    // One of the two quads at [ebp + slot]: record set up, the vertex at
    // ebp - 0x2a copied to its corners, turned by `angle` with half sizes
    // `across`, `down` (kept at ebp + acrossAt, + downAt; the angle at
    // + angleAt, the sine's share at + sineAt), clip-coded.
    struct Quad
    {
        x86::reg32 slot, corner1, corner2, corner3, angleAt, acrossAt, downAt, sineAt;
    };
    auto quad = [&](const Quad& q, double across, double down, double angle) {
        r.ecx = 8;
        m.store(q.angleAt, angle);
        copyVertex(m.word(q.corner3), ebp - 0x2a);
        const double turn = M::mul(M::add(1.0, m.load(q.angleAt)), m.load(0x53bcb8));
        r.ecx = 8;
        copyVertex(m.word(q.corner2), m.word(q.corner3));
        m.store(q.acrossAt, across);
        r.ecx = 8;
        m.store(q.downAt, down);
        copyVertex(m.word(q.corner1), m.word(q.corner2));
        m.store(q.angleAt, turn);
        r.ecx = 8;
        push(m.word(q.angleAt));
        copyVertex(r.edx, m.word(q.corner1));
        if (!callTo(0x4ea930))
            return false;
        const double sine = M::mul(result(), m.load(q.acrossAt));
        push(m.word(q.angleAt));
        m.store(q.sineAt, sine);
        if (!callTo(0x4ea950))
            return false;
        const double cosine = M::mul(result(), m.load(q.downAt));
        const double s = m.load(q.sineAt);
        {
            const double x = M::sub(m.load(r.edx), s);
            const double y = M::add(m.load(r.edx + 4), cosine);
            r.eax = m.word(q.corner1);
            m.store(r.edx, x);
            m.store(r.edx + 4, y);
        }
        {
            const x86::reg32 v = r.eax;
            const double x = M::add(m.load(v), cosine);
            const double y = M::add(m.load(v + 4), s);
            m.store(v, x);
            m.store(v + 4, y);
        }
        {
            r.eax = m.word(q.corner2);
            r.edi = r.edx + 0x14;
            const double x = M::add(m.load(r.eax), s);
            const double y = M::sub(m.load(r.eax + 4), cosine);
            m.store(r.eax, x);
            m.store(r.eax + 4, y);
        }
        {
            r.eax = m.word(q.corner3);
            r.esi = r.edx;
            const double x = M::sub(m.load(r.eax), cosine);
            const double y = M::sub(m.load(r.eax + 4), s);
            m.store(r.eax, x);
            m.store(r.eax + 4, y);
        }
        const x86::reg32 first = r.edx;
        screenCodeFlags(m, first, first + 0x14, r, flags);
        for (x86::reg32 corner : { q.corner1, q.corner2, q.corner3 })
        {
            const x86::reg32 v = m.word(corner);
            screenCodeFlags(m, v, v + 0x14, r, flags);
        }
        return true;
    };

    r.edx = m.word(ebp + 0x9a);
    r.esi = r.eax;
    const x86::reg32 view = r.esi;
    r.ecx = 0;
    r.edi = m.word(0x6fbc30);
    m.word(ebp + 0x42, 0);
    m.word(ebp + 0x3a, 0);
    flags.compare(r.edi, 2);
    if (r.edi == 2)
        return leave();
    r.edx &= 0xf;
    r.eax = r.edx * 12;
    flags.compare16(app->getMemory<x86::reg16>(r.eax + 0x55e3e0), 0);
    if (app->getMemory<x86::reg16>(r.eax + 0x55e3e0) != 0)
    {
        r.ebx = m.word(0x7d3684);
        setByte(r.ecx, 0, m.byte(r.eax + 0x55e3e3));
        setByte(r.eax, 0, m.byte(r.eax + 0x55e3e2));
        r.ebx += r.ecx;
        r.eax &= 0xff;
        setByte(r.ecx, 0, r.eax);
        r.eax = x86::reg32(x86::sreg32(r.ebx) >> (r.ecx & 31));
        flags.logic8(x86::reg8(r.eax) & 1);
        if (r.eax & 1)
            return leave();
    }
    r.eax = r.edx * 3;
    r.edx = m.word(r.eax * 4 + 0x55e3e4);
    r.eax = m.word(r.eax * 4 + 0x55e3dc);
    m.word(ebp + 0x66, r.eax);
    setByte(r.eax, 1, m.byte(0x7a3a58));
    m.word(ebp + 0x62, r.edx);
    // port (apply_alpha_intensity): test ah, 0x40 as if set
    flags.logic8(0x40);
    {
        const double glare = m.load(0x6fbc38);
        app->getMemory<double>(ebp + 0xe) = glare;
        if (glareBelowOne(cpu, flags, r, glare))
        {
            const x86::reg32 scale =
                x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x53bc98)))));
            m.word(esp - 4, scale);
            darken(flags, r, m.word(ebp + 0x66), scale);
            m.word(ebp + 0x66, r.eax);
        }
    }
    {
        // Into the view: times [0x53bca0], turned by +0x44, moved by +0x38.
        const double c = m.loadDouble(0x53bca0);
        const double px = M::mul(fild32(m, ebp + 0x8e), c);
        const double py = M::mul(fild32(m, ebp + 0x92), c);
        const double pz = M::mul(c, fild32(m, ebp + 0x96));
        r.ebx = ebp + 2;
        r.eax = ebp - 0xa;
        r.edx = view + 0x44;
        m.store(ebp - 0xa, px);
        m.store(ebp - 6, py);
        m.store(ebp - 2, pz);
        const x86::reg32 v = ebp - 0xa, mx = r.edx;
        double t[3];
        for (int i = 0; i < 3; ++i)
            t[i] = M::add(M::add(M::mul(m.load(v), m.load(mx + 4 * i)), M::mul(m.load(v + 4), m.load(mx + 0xc + 4 * i))),
                          M::mul(m.load(v + 8), m.load(mx + 0x18 + 4 * i)));
        r.eax = v + 0xc;
        for (int i = 0; i < 3; ++i)
            m.store(ebp + 2 + 4 * i, t[i]);
        for (int i = 0; i < 3; ++i)
            m.store(ebp + 2 + 4 * i, M::add(m.load(ebp + 2 + 4 * i), m.load(view + 0x38 + 4 * i)));
    }
    r.eax = m.word(ebp + 0xa);
    flags.compare(r.eax, 0x43800000);
    if (x86::sreg32(r.eax) > 0x43800000)
        return leave();
    flags.compare(r.eax, 0x3f800000);
    if (x86::sreg32(r.eax) < 0x3f800000)
        return leave();
    flags.logic(r.eax & 0x7fffffff);
    if (!(r.eax & 0x7fffffff))
        m.word(ebp + 0xa, 0x37800080);
    {
        // The vertex at ebp - 0x2a: x, y, z, 1/z; its code at ebp - 0x16.
        m.store(ebp + 0x1e, M::div(1.0, m.load(ebp + 0xa)));
        r.ecx = ebp + 0x1e;
        r.ebx = ebp - 0x2a;
        r.eax = ebp + 2;
        const double sx = M::mul(m.load(0x56009c), m.load(r.ecx));
        const double sy = M::mul(M::mul(m.load(0x5600a0), m.load(r.ecx)), m.load(r.eax + 4));
        r.edx = m.word(r.ecx);
        m.word(r.ebx + 0xc, r.edx);
        const double x = M::mul(sx, m.load(r.eax));
        r.edx = m.word(r.eax + 8);
        m.store(r.ebx, M::add(x, m.load(0x5600a4)));
        m.word(r.ebx + 8, r.edx);
        m.store(r.ebx + 4, M::add(sy, m.load(0x5600a8)));
    }
    screenCodeFlags(m, ebp - 0x2a, ebp - 0x16, r, flags);
    m.store(ebp - 0x1e, M::mul(m.load(ebp - 0x1e), m.loadDouble(0x53bca8)));
    screenCodeFlags(m, ebp - 0x2a, ebp - 0x16, r, flags);
    setByte(r.edx, 0, m.byte(ebp - 0x16));
    flags.logic8(x86::reg8(r.edx));
    if (x86::reg8(r.edx))
        return leave();
    {
        // The inner glow: the colour darkened by the depth too.
        r.ecx = 0x100;
        const x86::reg32 depth = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(m.load(ebp + 0xa))));
        m.word(esp - 4, depth);
        r.ecx = (r.ecx - depth) << 8;
        darken(flags, r, m.word(ebp + 0x66), r.ecx);
        r.edx = 0xa0;
        m.word(ebp - 0x1a, r.eax);
        r.eax = ebp + 0x3a;
    }
    if (!callTo(0x4bbde0))
        return;
    flags.logic(r.eax);
    if (!r.eax)
        return leave();
    {
        const x86::reg32 record = m.word(ebp + 0x3a);
        m.word(ebp + 0x3e, record + 0x40);
        m.word(ebp + 0x2a, record + 0x60);
        m.word(ebp + 0x6a, record + 0x80);
        const double angle = M::div(m.load(ebp + 2), m.load(ebp + 0xa));
        app->getMemory<x86::reg16>(record + 4) = 0;
        const double e = m.loadDouble(0x53bcb0);
        const double se = M::mul(m.load(0x56009c), e);
        m.word(record, 0);
        const double et = M::mul(e, m.load(0x5600a0));
        app->getMemory<x86::reg16>(record + 6) = 1;
        r.edx = record + 0x20;
        m.word(record + 0x18, 0x8b47bc);
        const double size = m.load(ebp + 0x62);
        const double across = M::mul(se, size);
        m.word(record + 8, r.edx);
        const double down = M::mul(size, et);
        m.word(record + 0xc, m.word(ebp + 0x3e));
        m.word(record + 0x10, m.word(ebp + 0x2a));
        m.word(record + 0x14, m.word(ebp + 0x6a));
        m.word(record + 0x1c, m.word(ebp - 0x22));
        r.eax = m.word(ebp - 0x22);
        if (!quad({ ebp + 0x3a, ebp + 0x3e, ebp + 0x2a, ebp + 0x6a, ebp + 0x36, ebp + 0x22, ebp + 0x26, ebp + 0x2e },
                  across, down, angle))
            return;
    }
    {
        // The outer glow, linked to the inner one.
        r.eax = m.word(ebp + 0x66);
        r.edx = 0xa0;
        m.word(ebp - 0x1a, r.eax);
        r.eax = ebp + 0x42;
    }
    if (!callTo(0x4bbde0))
        return;
    flags.logic(r.eax);
    if (!r.eax)
        return leave();
    {
        const x86::reg32 record = m.word(ebp + 0x42);
        const double sx = M::mul(m.load(0x56009c), m.load(ebp - 0x1e));
        const double sy = M::mul(m.load(0x5600a0), m.load(ebp - 0x1e));
        const double size = m.load(ebp + 0x62);
        const double sw = M::mul(sx, size);
        m.word(ebp + 0x4e, record + 0x40);
        m.word(ebp + 0x52, record + 0x60);
        m.word(ebp + 0x6e, record + 0x80);
        const double angle = M::div(m.load(ebp + 2), m.load(ebp + 0xa));
        app->getMemory<x86::reg16>(record + 4) = 0;
        m.word(record, m.word(ebp + 0x3a));
        const double sh = M::mul(size, sy);
        app->getMemory<x86::reg16>(record + 6) = 1;
        const double f = m.loadDouble(0x53bcc0);
        const double across = M::mul(sw, f);
        m.word(record + 0x18, 0x8b4840);
        r.edx = record + 0x20;
        const double down = M::mul(f, sh);
        m.word(record + 8, r.edx);
        m.word(record + 0xc, m.word(ebp + 0x4e));
        m.word(record + 0x10, m.word(ebp + 0x52));
        m.word(record + 0x14, m.word(ebp + 0x6e));
        m.word(record + 0x1c, m.word(ebp - 0x22));
        r.eax = m.word(ebp - 0x22);
        if (!quad({ ebp + 0x42, ebp + 0x4e, ebp + 0x52, ebp + 0x6e, ebp + 0x5e, ebp + 0x46, ebp + 0x4a, ebp + 0x56 },
                  across, down, angle))
            return;
    }
    r.eax = m.word(ebp + 0x42);
    push(m.word(0x7dcffc));
    if (!callTo(0x4c7610))
        return;
    leave();
}


/* sub_47b7d0: the colours (+0x10) of eax vertices from edx (0x20 bytes
 * each) darkened by the sun's glare [0x6fbc38] times [0x53b418]. */
void glareVerticesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 ebp = entry - 20;
    const x86::reg32 esp = entry - 20 - 4;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    r.edi = r.eax;
    r.esi = r.edx;
    r.eax = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(m.load(0x6fbc38), m.loadDouble(0x53b418)))));
    m.word(esp - 4, r.eax);
    m.word(ebp - 4, r.eax);
    for (;;)
    {
        flags.compare(r.edi - 1, 0xffffffff);
        --r.edi;
        if (r.edi == 0xffffffff)
            break;
        const x86::reg32 colour = m.word(r.esi + 0x10);
        r.esi += 0x20;
        darken(flags, r, colour, m.word(ebp - 4));
        flags.logic(r.eax);
        m.word(r.esi - 0x10, r.eax);
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.edx = r.edx;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.esi = saved[2];
    cpu.edi = saved[3];
    cpu.ebp = saved[4];
    cpu.esp = entry + 4;
}

/* sub_4cb670: sub_4e070c on 0x12c00 bytes at 0x7ddfd0 and 0x6400 at
 * 0x7f0bd0, the eight bytes at 0x560f30 for a driver with bit 0x40 of
 * [0x7a3a58], 0x7f6fd0 and 0x7f6fd4 cleared. */
void clearBuffersNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.edx, cpu.ebp };
    // push ecx, edx, ebp; mov ebp, esp
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    r.edx = 0x12c00;
    r.eax = 0x7ddfd0;
    if (!callWithRegisters(app, cpu, flags, r, ebp, esp, 0x4e070c))
        return;
    esp = cpu.esp;
    r.edx = 0x6400;
    r.eax = 0x7f0bd0;
    if (!callWithRegisters(app, cpu, flags, r, ebp, esp, 0x4e070c))
        return;
    flags.logic8(m.byte(0x7a3a58) & 0x40);
    if (m.byte(0x7a3a58) & 0x40)
    {
        setByte(r.edx, 0, 0x7f);
        setByte(r.eax, 0, 0x3f);
        setByte(r.ecx, 1, 0);
        for (x86::reg32 a : { 0x560f32u, 0x560f31u, 0x560f30u, 0x560f37u, 0x560f34u, 0x560f33u })
            m.byte(a, 0x7f);
        m.byte(0x560f36, 0);
        m.byte(0x560f35, 0x3f);
    }
    m.word(0x7f6fd4, 0);
    m.word(0x7f6fd0, 0);
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.esi = r.esi;
    cpu.edi = r.edi;
    cpu.ecx = saved[0];
    cpu.edx = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 4;
}

/* sub_4a4410: the ten bouncing pictures of a 2D screen (not on the drivers
 * the strings at 0x53cf28 and 0x53cf34 name, nor with bit 2 of
 * [0x7a3a58]): placed at random (sub_4a4060) the first time, each moved by
 * its speed (0x7a1ae8, 0x7a1b10) and put back (sub_4a40c0) when it leaves
 * 150..640 x 0..450, drawn by sub_4d7d90 with eleven fading copies trailing
 * behind it (the last picture as a shadow, its y from 225), centred by its
 * scaled width. */
void bouncersNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = entry - 24 - 0x18;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto truncate = [&](double value, x86::reg32 at) {
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(value);
        if (!callTo(0x4dfd56))
            return false;
        m.word(at, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
        cpu.fpu.count -= 1;
        return true;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    auto half = [](x86::reg32 value) { return x86::reg32(x86::sreg32(value - x86::reg32(x86::sreg32(value) >> 31)) >> 1); };

    flags.logic8(m.byte(0x7a3a58) & 0x40);
    if (m.byte(0x7a3a58) & 0x40)
    {
        for (x86::reg32 name : { 0x53cf28u, 0x53cf34u })
        {
            r.edx = name;
            r.eax = 0x7a3a74;
            if (!callTo(0x4e0820))
                return;
            flags.logic(r.eax);
            if (r.eax)
                return leave();
        }
    }
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
        return leave();
    flags.compare(m.word(0x55e95c), 0xffffffff);
    if (m.word(0x55e95c) == 0xffffffff)
    {
        r.esi = 0;
        flags.logic(0);
        for (;;)
        {
            r.eax = r.esi;
            if (!callTo(0x4a40c0))
                return;
            flags.compare(r.esi, 9);
            if (x86::sreg32(r.esi) < 9)
            {
                r.edx = 0x280;
                r.eax = 0x96;
                if (!callTo(0x4a4060))
                    return;
                r.edx = 0x1c2;
                m.word(r.esi * 4 + 0x55e95c, r.eax);
                r.eax = 0;
                flags.logic(0);
                if (!callTo(0x4a4060))
                    return;
                m.word(r.esi * 4 + 0x7a1ac0, r.eax);
            }
            flags.inc(r.esi);
            ++r.esi;
            flags.compare(r.esi, 10);
            if (x86::sreg32(r.esi) >= 10)
                break;
        }
    }
    r.esi = 0;
    flags.logic(0);
    for (;;)
    {
        {
            const x86::reg32 i = r.esi * 4;
            r.eax = i;
            r.edx = m.word(i + 0x7a1ae8);
            r.ecx = m.word(i + 0x55e95c) + r.edx;
            r.ebx = m.word(i + 0x7a1ac0) + m.word(i + 0x7a1b10);
            m.word(i + 0x55e95c, r.ecx);
            m.word(i + 0x7a1ac0, r.ebx);
            flags.compare(r.ecx, 0x280);
            bool out = x86::sreg32(r.ecx) > 0x280;
            if (!out)
            {
                flags.compare(r.ecx, 0x96);
                out = x86::sreg32(r.ecx) < 0x96;
            }
            if (!out)
            {
                flags.compare(r.ebx, 0x1c2);
                out = x86::sreg32(r.ebx) > 0x1c2;
                if (!out)
                {
                    flags.logic(r.ebx);
                    out = x86::sreg32(r.ebx) < 0;
                }
            }
            if (out)
            {
                r.eax = r.esi;
                if (!callTo(0x4a40c0))
                    return;
            }
        }
        m.word(ebp - 0x10, m.word(r.esi * 4 + 0x55e95c));
        r.eax = m.word(r.esi * 4 + 0x7a1ac0);
        r.edi = 0;
        flags.logic(0);
        m.word(ebp - 0xc, r.eax);
        r.ecx = 0x96;
        for (;;)
        {
            if (r.edi)
            {
                // Fading with the copy's age.
                r.edx = 12 - r.edi;
                r.eax = r.edx * 160 - r.edx * 10;
                r.ecx = 12;
                const x86::sreg32 dividend = x86::sreg32(r.eax);
                r.eax = x86::reg32(dividend / 12);
                r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
                r.eax = half(r.eax);
                r.ecx = r.eax;
            }
            flags.compare(r.esi, 9);
            if (r.esi == 9)
            {
                flags.logic(r.edi);
                if (!r.edi)
                    r.ecx = 0xff;
            }
            flags.logic(r.edi);
            if (!r.edi)
            {
                // Centred by half its scaled width.
                r.edx = m.word(r.esi * 4 + 0x7a1b38) << 4;
                r.eax = m.word(r.edx + 0x55e8e0);
                r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
                r.eax = half(r.eax);
                m.word(ebp - 8, r.eax);
                const double width = M::mul(fild32(m, ebp - 8), m.load(r.esi * 4 + 0x7a1b60));
                r.eax = m.word(ebp - 0x10);
                m.word(ebp - 8, r.eax);
                if (!truncate(M::sub(fild32(m, ebp - 8), width), ebp - 0x10))
                    return;
            }
            r.edx = m.word(0x7a1b94);
            r.eax = r.ecx;
            m.word(ebp - 0x14, r.edx);
            r.eax <<= 24;
            r.edx = m.word(ebp - 0x10);
            r.eax += 0xffffff;
            bool shown = false;
            flags.compare(r.edx, 0x280);
            if (x86::sreg32(r.edx) < 0x280)
            {
                flags.compare(r.edx, 0x96);
                if (x86::sreg32(r.edx) > 0x96)
                {
                    r.ebx = m.word(ebp - 0xc);
                    flags.compare(r.ebx, 0x1c2);
                    if (x86::sreg32(r.ebx) < 0x1c2)
                    {
                        flags.logic(r.ebx);
                        shown = x86::sreg32(r.ebx) > 0;
                    }
                }
            }
            if (shown)
            {
                push(r.eax);
                r.eax = m.word(r.esi * 4 + 0x7a1b38) << 4;
                r.ecx = m.word(r.eax + 0x55e8e4);
                push(m.word(r.esi * 4 + 0x7a1b60));
                push(r.ecx);
                r.ebx = m.word(r.eax + 0x55e8e0);
                r.edx = m.word(r.eax + 0x55e8dc);
                push(r.ebx);
                r.eax = m.word(ebp - 0x14);
                r.ecx = 0;
                push(r.edx);
                r.ebx = m.word(ebp - 0xc);
                r.edx = m.word(ebp - 0x10);
                if (!callTo(0x4d7d90))
                    return;
            }
            r.edx = r.esi * 4;
            flags.compare(r.esi, 9);
            if (r.esi == 9)
            {
                r.ecx = m.word(r.edx + 0x7a1ac0);
                r.eax = 0xe1 - r.ecx;
                r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
                r.eax = half(r.eax);
                flags.compare(m.word(ebp - 0xc), r.eax);
                m.word(ebp - 0xc, m.word(ebp - 0xc) - r.eax);
            }
            else
            {
                // The next copy back along the speed, the shadow's y kept.
                r.ecx = m.word(r.edx + 0x7a1ae8);
                m.word(ebp - 8, r.ecx * 10);
                const double backX = M::mul(fild32(m, ebp - 8), m.load(r.edx + 0x7a1b60));
                r.eax = m.word(ebp - 0x10);
                r.ecx = m.word(r.edx + 0x7a1b10);
                m.word(ebp - 8, r.eax);
                m.word(ebp - 4, r.ecx * 10);
                const double backY = M::mul(fild32(m, ebp - 4), m.load(r.edx + 0x7a1b60));
                r.eax = m.word(ebp - 0xc);
                r.ecx = m.word(r.edx + 0x7a1ac0);
                m.word(ebp - 4, r.eax);
                r.eax = 0xe1 - r.ecx;
                r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
                r.eax = half(r.eax);
                const double x = M::sub(fild32(m, ebp - 8), backX);
                const double y = M::sub(fild32(m, ebp - 4), backY);
                if (!truncate(y, ebp - 0xc))
                    return;
                r.ebx = m.word(ebp - 0xc);
                if (!truncate(x, ebp - 0x10))
                    return;
                r.ebx -= r.eax;
                m.word(ebp - 0xc, r.ebx);
            }
            flags.inc(r.edi);
            ++r.edi;
            flags.compare(r.edi, 12);
            if (x86::sreg32(r.edi) >= 12)
                break;
            flags.logic(r.edi);
        }
        flags.inc(r.esi);
        ++r.esi;
        flags.compare(r.esi, 10);
        if (x86::sreg32(r.esi) >= 10)
            break;
    }
    leave();
}

/* sub_491bc0: a headlight's patch on the road, stdcall (view in eax; +4..+0xc
 * where it falls, +0x10 its colour, +0x14 for sub_420bf0, +0x18 its size,
 * +0x1c the car or 0, +0x20 its direction kept): unless the detail
 * [0x6fbc30] is 2, [0x6fd4cc] is clear, sub_420bf0 refuses it or the view is
 * 1, the point snapped to the track (sub_420650, sub_4207e0, sub_4dcdd0; only
 * on surface 10), its distance from the view (sub_4972f0) shortened by the
 * car's facing, a square of the size held to [size, 30] across the road's
 * normal (sub_4e0160, sub_4e0050), its corners raised to the road by
 * sub_4968a0 when the track has heights there; projected (sub_4bf4c0) and,
 * unless all off one side or (in front of the near plane) facing the wrong
 * way for [0x554e4c], a quad record (sub_4bbde0, 0xa0 bytes, flags 1,
 * texture 0x8b486c) of the colour, darkened by the sun's glare (on every
 * driver, a port).  Returns the record, or 0. */
void roadLightNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x204; sub ebp, 0x76
    const x86::reg32 ebp = entry - 24 - 0x76;
    x86::reg32 esp = entry - 24 - 0x204;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    auto integer = [&](double value) {
        const x86::reg32 bits = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(value)));
        m.word(esp - 4, bits);
        return bits;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4 + 0x20;
    };
    auto none = [&]() {
        r.eax = 0;
        leave();
    };
    // fcom..., fnstsw ax, sahf
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    // movsd three times
    auto copy3 = [&](x86::reg32 to, x86::reg32 from) {
        for (x86::reg32 k = 0; k < 0xc; k += 4)
            m.word(to + k, m.word(from + k));
        r.edi = to + 0xc;
        r.esi = from + 0xc;
    };

    r.ebx = m.word(ebp + 0xa2);
    r.ecx = r.eax;
    copy3(ebp + 6, 0x491110);
    r.edx = 0;
    r.esi = m.word(0x6fbc30);
    m.word(ebp + 0x66, 0);
    flags.compare(r.esi, 2);
    if (r.esi == 2)
        return none();
    flags.compare(m.word(0x6fd4cc), 0);
    if (m.word(0x6fd4cc) == 0)
        return none();
    push(r.ebx);
    r.eax = m.word(ebp + 0x9a);
    push(r.eax);
    r.edx = m.word(ebp + 0x96);
    push(r.edx);
    r.esi = m.word(ebp + 0x92);
    push(r.esi);
    if (!callTo(0x420bf0))
        return;
    flags.logic(r.eax);
    if (r.eax)
        return none();
    flags.compare(m.word(r.ecx), 1);
    if (m.word(r.ecx) == 1)
        return leave();
    {
        // To the track's fixed point and back (sub_420650).
        const double size = m.load(ebp + 0xa6);
        const double half = M::mul(size, m.loadDouble(0x53bcc8));
        const double toFixed = m.loadDouble(0x53bcd0);
        const x86::reg32 x = integer(M::mul(m.load(ebp + 0x92), toFixed));
        m.store(ebp + 0xa6, half);
        r.eax = x;
        m.word(ebp + 0x2a, r.eax);
        r.eax = integer(M::mul(m.load(ebp + 0x96), toFixed));
        r.edx = 1;
        r.esi = ebp - 0x10e;
        m.word(ebp + 0x2e, r.eax);
        r.eax = integer(M::mul(m.load(ebp + 0x9a), toFixed));
        m.word(ebp + 0x32, r.eax);
        r.eax = ebp + 0x2a;
    }
    if (!callTo(0x420650))
        return;
    {
        const double fromFixed = m.loadDouble(0x53bcd8);
        r.eax = m.word(ebp + 0x2a);
        m.word(ebp + 0x72, r.eax);
        const double x = M::mul(fild32(m, ebp + 0x72), fromFixed);
        r.eax = m.word(ebp + 0x2e);
        m.word(ebp + 0x72, r.eax);
        r.eax = m.word(ebp + 0x32);
        const double y = M::mul(fild32(m, ebp + 0x72), fromFixed);
        m.word(ebp + 0x72, r.eax);
        const double z = M::mul(fromFixed, fild32(m, ebp + 0x72));
        m.store(ebp + 0x92, x);
        m.store(ebp + 0x96, y);
        r.eax = m.word(ebp - 0xf6);
        m.store(ebp + 0x9a, z);
    }
    flags.logic(r.eax);
    if (!r.eax)
        return leave();
    r.edx = 1;
    r.eax = ebp - 0x10e;
    if (!callTo(0x4207e0))
        return;
    r.esi = r.eax;
    if (!callTo(0x4dcdd0))
        return;
    r.ebx = (r.eax * 9) << 6;
    r.esi &= 0x3f;
    r.eax = r.esi * 9;
    flags.compare(m.word(r.ebx + r.eax * 4 + 0x55308c), 0xa);
    if (m.word(r.ebx + r.eax * 4 + 0x55308c) != 0xa)
        return none();
    {
        // From the view (+8..+0x10), level.
        const double dx = M::sub(m.load(ebp + 0x92), m.load(r.ecx + 8));
        const double dy = M::sub(m.load(ebp + 0x96), m.load(r.ecx + 0xc));
        const double dz = M::sub(m.load(ebp + 0x9a), m.load(r.ecx + 0x10));
        m.store(ebp + 0x1e, dx);
        r.eax = ebp + 0x1e;
        m.store(ebp + 0x22, dy);
        r.ebx = 0;
        m.store(ebp + 0x26, dz);
        m.word(ebp + 0x22, 0);
    }
    if (!callTo(0x4972f0))
        return;
    m.store(ebp + 0x6e, result());
    flags.logic(m.word(ebp + 0x6e) & 0x7fffffff);
    if (!(m.word(ebp + 0x6e) & 0x7fffffff))
        return none();
    flags.compare(m.word(ebp + 0xaa), 0);
    if (m.word(ebp + 0xaa) != 0)
    {
        // Shortened by the car's facing (+0x18), reversed unless kept.
        r.eax = m.word(ebp + 0xaa);
        copy3(ebp - 6, r.eax + 0x18);
        r.eax = m.word(ebp + 0xae);
        flags.logic(r.eax);
        if (!r.eax)
        {
            setByte(r.edx, 0, m.byte(ebp + 1) ^ 0x80);
            setByte(r.eax, 1, m.byte(ebp - 3) ^ 0x80);
            m.byte(ebp + 1, x86::reg8(r.edx));
            setByte(r.edx, 1, m.byte(ebp + 5) ^ 0x80);
            flags.logic8(x86::reg8(r.edx >> 8));
            m.byte(ebp - 3, x86::reg8(r.eax >> 8));
            m.byte(ebp + 5, x86::reg8(r.edx >> 8));
        }
        r.edx = ebp + 0x1e;
        r.eax = ebp - 6;
        if (!callTo(0x4e01f0))
            return;
        m.store(ebp + 0x6e, M::mul(-result(), m.load(ebp + 0x6e)));
    }
    r.ebx = ebp - 0x12;
    r.edx = ebp + 6;
    r.eax = ebp + 0x1e;
    if (!callTo(0x4e0160))
        return;
    r.eax = ebp - 0x12;
    push(r.eax);
    r.edx = ebp - 0x12;
    r.eax = 1;
    push(m.word(ebp + 0xa6));
    if (!callTo(0x4e0050))
        return;
    {
        // The length held to [size, 30].
        const double length = M::mul(m.load(ebp + 0x6e), m.loadDouble(0x53bce0));
        const double size = m.load(ebp + 0xa6);
        app->getMemory<double>(ebp + 0x56) = size;
        app->getMemory<double>(ebp + 0x36) = length;
        compareToFlags(length, size);
        if (cpu.flags.cf)
        {
            r.eax = m.word(ebp + 0x56);
            m.word(ebp + 0x3e, r.eax);
            r.eax = m.word(ebp + 0x5a);
        }
        else
        {
            compareToFlags(app->getMemory<double>(ebp + 0x36), m.loadDouble(0x53bce8));
            if (!cpu.flags.cf && !cpu.flags.zf)
            {
                r.edx = 0;
                flags.logic(0);
                r.ebx = 0x403e0000;
                m.word(ebp + 0x5e, r.edx);
                m.word(ebp + 0x62, r.ebx);
            }
            else
            {
                r.eax = m.word(ebp + 0x36);
                m.word(ebp + 0x5e, r.eax);
                r.eax = m.word(ebp + 0x3a);
                m.word(ebp + 0x62, r.eax);
            }
            r.eax = m.word(ebp + 0x5e);
            m.word(ebp + 0x3e, r.eax);
            r.eax = m.word(ebp + 0x62);
        }
        m.word(ebp + 0x42, r.eax);
        const double along = app->getMemory<double>(ebp + 0x3e);
        r.eax = ebp + 0x1e;
        r.edx = ebp + 0x1e;
        r.edi = ebp - 0x96;
        r.esi = ebp + 0x92;
        push(r.eax);
        esp -= 4;
        r.eax = 1;
        m.store(esp, -along);
    }
    if (!callTo(0x4e0050))
        return;
    {
        // The four corners: point - across, + across, + across + along,
        // - across + along.
        const x86::reg32 across = ebp - 0x12, along = ebp + 0x1e;
        const x86::reg32 corners[] = { ebp - 0x96, ebp - 0x8a, ebp - 0x7e, ebp - 0x72 };
        for (int c = 0; c < 4; ++c)
        {
            // The first copy goes where the callee left edi and esi.
            if (c == 0)
                copy3(r.edi, r.esi);
            else
                copy3(corners[c], ebp + 0x92);
            const x86::reg32 p = corners[c];
            for (x86::reg32 k = 0; k < 0xc; k += 4)
            {
                const double side = c == 0 || c == 3 ? M::sub(m.load(p + k), m.load(across + k))
                                                     : M::add(m.load(p + k), m.load(across + k));
                m.store(p + k, side);
            }
            if (c >= 2)
            {
                if (c == 3)
                    r.esi = m.word(ebp - 0xfa);
                for (x86::reg32 k = 0; k < 0xc; k += 4)
                    m.store(p + k, M::add(m.load(p + k), m.load(along + k)));
            }
            if (c < 3)
            {
                r.edi = corners[c] + 0xc;
                r.esi = ebp + 0x9e;
            }
        }
        r.edi = ebp - 0x72 + 0xc;
    }
    flags.logic(r.esi);
    if (r.esi)
    {
        flags.compare(m.word(ebp - 0xa2), 0);
        if (m.word(ebp - 0xa2) == 0)
        {
            // The road's heights at the corners.
            r.edx = ebp - 0x66;
            r.eax = ebp - 0x10e;
            r.esi = ebp - 0x1e;
            if (!callTo(0x420230))
                return;
            const double fromFixed = m.loadDouble(0x53bcd8);
            const x86::reg32 sources[] = { 0x66, 0x62, 0x5e, 0x5a, 0x56, 0x52, 0x4e, 0x4a, 0x46, 0xea, 0xe6, 0xe2 };
            double v[12];
            for (int i = 0; i < 12; ++i)
            {
                r.eax = m.word(ebp - sources[i]);
                m.word(ebp + 0x72, r.eax);
                v[i] = M::mul(fild32(m, ebp + 0x72), fromFixed);
            }
            r.ebx = 0;
            flags.logic(0);
            for (int i = 0; i < 9; ++i)
                m.store(ebp - 0x42 + 4 * i, v[i]);
            for (int i = 0; i < 3; ++i)
                m.store(ebp + 0x12 + 4 * i, v[9 + i]);
            for (;;)
            {
                r.eax = r.ebx * 4;
                flags.compare(r.eax, r.ebx);
                r.eax -= r.ebx;
                r.edx = r.eax * 4;
                r.eax = m.word(r.edx + ebp - 0x8e);
                push(r.eax);
                r.esi = m.word(r.edx + ebp - 0x92);
                push(r.esi);
                r.edi = m.word(r.edx + ebp - 0x96);
                push(r.edi);
                r.eax = m.word(ebp + 0x1a);
                push(r.eax);
                r.esi = m.word(ebp + 0x16);
                push(r.esi);
                r.edi = m.word(ebp + 0x12);
                push(r.edi);
                r.eax = ebp - 0x42;
                if (!callTo(0x4968a0))
                    return;
                const double height = M::add(result(), m.load(0x53bcf0));
                flags.inc(r.ebx);
                ++r.ebx;
                m.store(r.edx + ebp - 0x92, height);
                flags.compare(r.ebx, 4);
                if (x86::sreg32(r.ebx) >= 4)
                    break;
            }
        }
    }
    r.eax = ebp - 0x18e;
    push(r.eax);
    r.eax = r.ecx + 0x38;
    r.ebx = r.ecx + 0x44;
    r.edx = ebp - 0x96;
    r.ecx = r.eax;
    r.eax = 4;
    if (!callTo(0x4bf4c0))
        return;
    {
        r.edx = m.byte(ebp - 0x17a);
        r.eax = m.byte(ebp - 0x15a);
        r.esi = r.edx & r.eax;
        r.ecx = m.byte(ebp - 0x13a);
        r.esi &= r.ecx;
        r.ebx = m.byte(ebp - 0x11a);
        flags.logic(r.esi & r.ebx);
        if (r.esi & r.ebx)
            return none();
        r.eax |= r.edx | r.ecx | r.ebx;
        flags.logic8(x86::reg8(r.eax) & 0x10);
        if (!(r.eax & 0x10))
        {
            // Facing: (x2 - x1)(y0 - y1) against (y2 - y1)(x0 - x1), twice.
            const x86::reg32 v0 = ebp - 0x18e, v1 = ebp - 0x16e, v2 = ebp - 0x14e, v3 = ebp - 0x12e;
            auto facing = [&](x86::reg32 a, x86::reg32 b, x86::reg32 c) {
                const double p = M::mul(M::sub(m.load(c), m.load(b)), M::sub(m.load(a + 4), m.load(b + 4)));
                const double q = M::mul(M::sub(m.load(c + 4), m.load(b + 4)), M::sub(m.load(a), m.load(b)));
                compareToFlags(q, p);
                r.eax = cpu.flags.cf || cpu.flags.zf ? 0 : 1;
                r.eax ^= m.word(0x554e4c);
                flags.logic(r.eax);
                return r.eax != 0;
            };
            if (!facing(v0, v1, v2))
                return none();
            if (!facing(v0, v2, v3))
                return none();
        }
    }
    // port (apply_alpha_intensity): test byte [0x7a3a58], 0x40 as if set
    flags.logic8(0x40);
    {
        const double glare = m.load(0x6fbc38);
        app->getMemory<double>(ebp + 0x4e) = glare;
        if (glareBelowOne(cpu, flags, r, glare))
        {
            const x86::reg32 scale = integer(M::mul(glare, m.loadDouble(0x53bcd0)));
            darken(flags, r, m.word(ebp + 0x9e), scale);
            m.word(ebp + 0x9e, r.eax);
        }
    }
    r.eax = m.word(ebp + 0x9e);
    r.edx = 0xa0;
    m.word(ebp - 0x11e, r.eax);
    m.word(ebp - 0x13e, r.eax);
    m.word(ebp - 0x15e, r.eax);
    m.word(ebp - 0x17e, r.eax);
    r.eax = ebp + 0x66;
    if (!callTo(0x4bbde0))
        return;
    flags.logic(r.eax);
    if (!r.eax)
        return none();
    {
        const x86::reg32 record = m.word(ebp + 0x66);
        app->getMemory<x86::reg16>(record + 4) = 0;
        m.word(record, 0);
        app->getMemory<x86::reg16>(record + 6) = 1;
        m.word(record + 0x18, 0x8b486c);
        m.word(record + 8, record + 0x20);
        m.word(record + 0xc, record + 0x40);
        m.word(record + 0x10, record + 0x60);
        m.word(record + 0x14, record + 0x80);
        m.word(record + 0x1c, m.word(ebp + 0x6e));
        for (x86::reg32 k = 0; k < 0x80; k += 4)
            m.word(record + 0x20 + k, m.word(ebp - 0x18e + k));
        r.ecx = 0;
        r.edi = record + 0xa0;
        r.esi = ebp - 0x10e;
        r.ebx = record + 0x40;
        r.edx = record + 0x80;
        r.eax = record;
    }
    leave();
}


/* sub_41df10: the next view record (0x54 bytes from 0x5dd0e0, [0x5dd0dc]
 * of them so far) for view kind eax, its screen edx and its owner ebx: the
 * cull distances and fog by the kind -- the main view, a split screen (its
 * cull at 500 as a single view's, a port), the mirror -- each also times
 * [0x536d6c] as floats.  Returns the record's number. */
void viewRecordNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    IntegerFlags flags;
    const x86::reg32 kind = cpu.eax, screen = cpu.edx;
    const x86::reg32 record = 0x5dd0e0 + m.word(0x5dd0dc) * 0x54;
    m.word(record + 0x14, 0xffffffff);
    m.word(record + 0x18, 0xffffffff);
    m.word(record + 0x48, 0);
    m.word(record + 8, 1);
    m.word(record + 4, 0);
    m.word(record, screen);
    m.word(record + 0x1c, cpu.ebx);
    flags.compare(kind, 3);
    x86::reg32 edx = cpu.edx;
    auto plain = [&](x86::reg32 near, x86::reg32 far, x86::reg32 fogNear, x86::reg32 fogFar, x86::reg32 cull,
                     x86::reg32 detail) {
        m.word(record + 0x24, near);
        m.word(record + 0x28, far);
        m.word(record + 0x38, fogNear);
        m.word(record + 0x3c, fogFar);
        m.word(record + 0x48, cull);
        m.word(record + 0x4c, detail);
    };
    switch (kind)
    {
    case 0:
        plain(0x1f40000, 0x1f40000, 0x6e0000, 0x960000, 0x7d0, 0x2a);
        break;
    case 1:
    case 2:
        // port (apply_split_screen_cull): culls at 500, as a single view does
        plain(0x1f40000, 0x1b80000, 0x3c0000, 0x5c0000, 0x2bc, 0x18);
        break;
    case 3:
        plain(0x960000, 0x640000, 0x280000, 0x500000, 0xfa, 6);
        break;
    default:
        break;
    }
    if (kind <= 3)
    {
        edx = m.word(record + 0x1c);
        m.word(record + 0x20, 0xfffb0000);
        const bool split = kind == 1 || kind == 2;
        m.word(edx + 0x1c, split ? 0x380 : 0);
        edx = m.word(record + 0x1c);
        m.word(edx + 0x20, split ? 1 : 0);
    }
    const double scale = m.loadDouble(0x536d6c);
    auto scaled = [&](x86::reg32 field) { return M::mul(fild32(m, record + field), scale); };
    const double a = scaled(0x20), b = scaled(0x24), d = scaled(0x28), e = scaled(0x3c);
    const double f = M::mul(scale, fild32(m, record + 0x38));
    m.store(record + 0x30, b);
    m.store(record + 0x34, d);
    m.store(record + 0x44, e);
    m.store(record + 0x40, f);
    m.store(record + 0x2c, a);
    cpu.eax = m.word(0x5dd0dc);
    cpu.edx = cpu.eax + 1;
    m.word(0x5dd0dc, cpu.edx);
    flags.store(cpu);
    cpu.esp += 4;
}

/* sub_41d960: the views for a race: sub_41de40, the track's setup
 * (sub_41d8a0, sub_40f280, stopping at [0x55e88c]), sub_41dae0, sub_41deb0,
 * the view records (sub_41df10: the main view and the mirror, or the two
 * halves of a split screen with [0x6fd3b0] 1), then sub_41d310 ..
 * sub_41d900. */
void raceViewsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.edx, cpu.esi, cpu.ebp };
    // push ebx, edx, esi, ebp; mov ebp, esp
    const x86::reg32 ebp = entry - 16;
    x86::reg32 esp = ebp;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ecx = r.ecx;
        cpu.edi = r.edi;
        cpu.ebx = saved[0];
        cpu.edx = saved[1];
        cpu.esi = saved[2];
        cpu.ebp = saved[3];
        cpu.esp = entry + 4;
    };
    m.word(0x5dd834, 0x1234);
    if (!callTo(0x41de40))
        return;
    flags.compare(m.word(0x55e88c), 0);
    if (m.word(0x55e88c) != 0)
        return leave();
    r.eax = 0x536d10;
    if (!callTo(0x41d8a0) || !callTo(0x40f280))
        return;
    flags.compare(m.word(0x55e88c), 0);
    if (m.word(0x55e88c) != 0)
        return leave();
    if (!callTo(0x41dae0) || !callTo(0x41deb0))
        return;
    r.esi = m.word(0x6fd3b0);
    flags.compare(r.esi, 1);
    const bool split = r.esi == 1;
    r.ebx = 0x5dd090;
    r.edx = 0x10;
    r.eax = split ? r.esi : 0;
    if (!callTo(0x41df10))
        return;
    r.ebx = 0x5dd0b4;
    r.edx = split ? 0x20 : 0x10;
    r.eax = split ? 2 : 3;
    if (!callTo(0x41df10))
        return;
    r.eax = 0;
    if (!callTo(0x41d310) || !callTo(0x41d620) || !callTo(0x419b60))
        return;
    r.eax = 0x536d14;
    if (!callTo(0x41d8a0) || !callTo(0x41dcc0) || !callTo(0x41dc60) || !callTo(0x41a100))
        return;
    r.eax = m.word(0x6fd4ac);
    if (!callTo(0x470b90) || !callTo(0x41d900))
        return;
    leave();
}


/* sub_4de070: a view's smoke (view in eax, its group at +4; edx particles;
 * the result's two words -- first and last record -- to esi, which is
 * returned): the group's positions (0x8c56e8 + group * 9600, 12 bytes each)
 * projected by sub_4bf4c0 to 0x8ba4c0; each live one (byte 8 of its state at
 * 0x8c0be8.. clear) behind the camera or past [0x8ca24c] is put back at the
 * view (sub_4eb230 scattering it), one too near ([0x5497fc]) when
 * sub_4dd930 says so; the rest become quads (0xa0 bytes each, flags 1,
 * texture 0x8ca200) of the grey [0x8ca238] darkened by the sun's glare (on
 * every driver, a port), their size [0x8ca230] less depth times [0x8ca250]
 * times [0x549804], at most 1, closed by sub_4bbde0 (or sub_4bbe70 for
 * none). */
void smokeNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edi, cpu.ebp };
    // push ebx, ecx, edi, ebp; mov ebp, esp; sub esp, 0x5c
    const x86::reg32 ebp = entry - 16;
    x86::reg32 esp = entry - 16 - 0x5c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    // Back to the view, scattered; `at` re-read after each call.
    auto respawn = [&](auto at) {
        const x86::reg32 view = m.word(ebp - 0x1c);
        r.eax = view;
        m.store(at() + 4, M::add(m.load(view + 0xc), m.load(0x8ca264)));
        for (x86::reg32 field : { 8u, 0x10u })
        {
            if (!callTo(0x4eb230))
                return false;
            const double spread = m.load(0x8ca25c);
            const double scattered = M::sub(M::mul(M::mul(result(), spread), m.load(0x5497f4)), spread);
            r.eax = m.word(ebp - 0x1c);
            m.store(at() + (field == 8 ? 0 : 8), M::add(scattered, m.load(r.eax + field)));
        }
        return true;
    };

    m.word(ebp - 0x38, r.esi);
    m.word(ebp - 0x1c, r.eax);
    m.word(ebp - 4, r.edx);
    r.edx = 0;
    r.eax = m.word(r.eax + 4);
    m.word(ebp - 0x30, 0);
    m.word(ebp - 0x34, r.eax);
    {
        const x86::reg32 grey = m.word(0x8ca238);
        r.edx = grey | (grey << 16) | (grey << 24) | (grey << 8);
        r.eax = (grey << 16) | (grey << 24) | (grey << 8);
    }
    setByte(r.eax, 1, m.byte(0x7a3a58));
    m.word(ebp - 0x24, r.edx);
    // port (apply_alpha_intensity): test ah, 0x40 as if set
    flags.logic8(0x40);
    {
        const double glare = m.load(0x6fbc38);
        app->getMemory<double>(ebp - 0x54) = glare;
        if (glareBelowOne(cpu, flags, r, glare))
        {
            const x86::reg32 scale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x5497e4)))));
            m.word(esp - 4, scale);
            darken(flags, r, r.edx, scale);
            m.word(ebp - 0x24, r.eax);
        }
    }
    r.edx = m.word(ebp - 4) * 5 << 5;
    r.eax = ebp - 0x3c;
    if (!callTo(0x4bbe40))
        return;
    flags.logic(r.eax);
    if (r.eax)
    {
        r.eax = m.word(ebp - 0x3c);
        push(0x8ba4c0);
        r.edi = m.word(ebp - 0x34);
        m.word(ebp - 0x18, r.eax);
        r.ecx = m.word(ebp - 0x1c) + 0x38;
        r.ebx = m.word(ebp - 0x1c) + 0x44;
        r.edi = r.edi * 640;
        r.edx = 0x8c56e8 + r.edi * 15;
        r.eax = m.word(ebp - 4);
        if (!callTo(0x4bf4c0))
            return;
        r.ecx = 0;
        m.word(ebp - 0x20, 0);
        for (;; flags.inc(m.word(ebp - 0x20)), m.word(ebp - 0x20, m.word(ebp - 0x20) + 1))
        {
            r.eax = m.word(ebp - 0x20);
            flags.compare(r.eax, m.word(ebp - 4));
            if (x86::sreg32(r.eax) >= x86::sreg32(m.word(ebp - 4)))
                break;
            r.edi = m.word(ebp - 0x34);
            r.eax = r.edi * 640;
            r.esi = r.eax * 15;
            r.edx = m.word(ebp - 0x20);
            r.eax = r.edx * 3;
            r.ecx = 0x8c0be8 + r.esi;
            r.edx = r.eax * 4;
            r.eax = r.ecx + r.edx;
            setByte(r.ebx, 0, m.byte(r.eax + 8));
            flags.logic8(x86::reg8(r.ebx));
            if (x86::reg8(r.ebx))
                continue;
            r.eax = m.word(ebp - 0x20) << 5;
            flags.add(0x8ba4c0, r.eax);
            r.ecx = 0x8ba4c0 + r.eax;
            const x86::reg32 v = r.ecx;
            m.word(ebp - 0x14, v);
            compareToFlags(m.load(v + 8), m.loadDouble(0x5497ec));
            bool behind = !cpu.flags.cf && !cpu.flags.zf;
            if (behind)
            {
                compareToFlags(0.0, m.load(v + 8));
                behind = !cpu.flags.cf && !cpu.flags.zf;
            }
            if (behind)
            {
                if (!respawn([&]() { return r.edx + r.esi + 0x8c56e8; }))
                    return;
                continue;
            }
            r.eax = m.word(ebp - 0x14);
            compareToFlags(0.0, m.load(r.eax + 8));
            if (!cpu.flags.cf)
                continue;
            r.eax = m.word(ebp - 0x14);
            compareToFlags(fild32(m, 0x8ca24c), m.load(r.eax + 8));
            if (cpu.flags.cf)
                continue;
            r.eax = m.word(ebp - 0x14);
            flags.compare8(m.byte(r.eax + 0x14), 0);
            if (m.byte(r.eax + 0x14))
                continue;
            compareToFlags(m.load(r.eax + 8), m.loadDouble(0x5497fc));
            bool spawned = false;
            if (cpu.flags.cf)
            {
                r.edi = m.word(ebp - 0x34);
                r.eax = r.edi * 640;
                r.esi = r.eax * 15;
                r.edx = m.word(ebp - 0x20);
                r.eax = r.edx * 12;
                r.esi += r.eax;
                r.edi = m.word(r.esi + 0x8c56f0);
                push(r.edi);
                r.eax = m.word(r.esi + 0x8c56ec);
                push(r.eax);
                r.edx = m.word(r.esi + 0x8c56e8);
                push(r.edx);
                r.eax = m.word(ebp - 0x1c);
                if (!callTo(0x4dd930))
                    return;
                flags.logic(r.eax);
                if (r.eax)
                {
                    if (!respawn([&]() { return r.esi + 0x8c56e8; }))
                        return;
                    spawned = true;
                }
            }
            if (spawned)
                continue;
            {
                // The quad, size w.
                r.eax = m.word(ebp - 0x14);
                const double base = m.load(0x8ca230);
                const double size = M::sub(base, M::mul(M::mul(m.load(r.eax + 8), base), m.load(0x8ca250)));
                m.store(ebp - 0x2c, M::mul(size, m.loadDouble(0x549804)));
                compareToFlags(1.0, m.load(ebp - 0x2c));
                if (cpu.flags.cf || cpu.flags.zf)
                {
                    r.eax = m.word(ebp - 0x2c);
                    m.word(ebp - 0x28, r.eax);
                }
                else
                    m.word(ebp - 0x28, 0x3f800000);
            }
            {
                const x86::reg32 record = m.word(ebp - 0x18);
                m.word(ebp - 0x10, record + 0x40);
                m.word(ebp - 0xc, record + 0x60);
                m.word(ebp - 8, record + 0x80);
                app->getMemory<x86::reg16>(record + 4) = 0;
                r.esi = record + 0x20;
                m.word(record + 0x18, 0x8ca200);
                app->getMemory<x86::reg16>(record + 6) = 1;
                m.word(record + 8, r.esi);
                m.word(record + 0xc, record + 0x40);
                m.word(record + 0x10, record + 0x60);
                m.word(record + 0x14, record + 0x80);
                m.word(record, record + 0xa0);
                const x86::reg32 v = m.word(ebp - 0x14);
                const double w = m.load(ebp - 0x28);
                const x86::reg32 corners[] = { record + 0x20, record + 0x40, record + 0x60, record + 0x80 };
                for (int c = 0; c < 4; ++c)
                {
                    const x86::reg32 q = corners[c];
                    if (c == 0)
                    {
                        m.word(q, m.word(v));
                        m.word(q + 4, m.word(v + 4));
                        m.word(q + 8, m.word(v + 8));
                    }
                    else
                    {
                        if (c == 3)
                            m.store(q, m.load(v));
                        else
                            m.store(q, M::add(m.load(v), w));
                        if (c == 1)
                            m.store(q + 4, m.load(v + 4));
                        else
                            m.store(q + 4, M::add(m.load(v + 4), w));
                        m.store(q + 8, m.load(v + 8));
                    }
                    m.store(q + 0xc, m.load(v + 0xc));
                    m.word(q + 0x10, m.word(ebp - 0x24));
                    r.eax = m.word(ebp - 0x24);
                    r.edx = q;
                    screenCodeFlags(m, q, q + 0x14, r, flags);
                }
                r.esi = m.word(ebp - 0x18);
                r.ebx = m.word(ebp - 0x30);
                flags.add(r.esi, 0xa0);
                r.esi += 0xa0;
                flags.inc(r.ebx);
                ++r.ebx;
                m.word(ebp - 0x18, r.esi);
                m.word(ebp - 0x30, r.ebx);
            }
        }
        r.eax = m.word(ebp - 0x30);
        flags.logic(r.eax);
        if (r.eax)
        {
            r.esi = r.eax;
            r.edx = m.word(ebp - 0x18) - 0xa0;
            flags.shl(r.eax * 5, 5);
            r.eax = r.eax * 5 << 5;
            m.word(ebp - 0x18, r.edx);
            r.edx = r.eax;
            r.eax = ebp - 0x3c;
            if (!callTo(0x4bbde0))
                return;
            r.eax = m.word(ebp - 0x3c);
            m.word(ebp - 0x4c, r.eax);
            r.eax = m.word(ebp - 0x18);
            m.word(ebp - 0x48, r.eax);
            m.word(r.eax, 0);
            m.word(ebp - 0x5c, m.word(ebp - 0x4c));
            m.word(ebp - 0x58, m.word(ebp - 0x48));
            r.edi = ebp - 0x54;
            r.esi = ebp - 0x44;
            goto done;
        }
    }
    r.esi = ebp - 0x5c;
    if (!callTo(0x4bbe70))
        return;
done:
    r.edi = m.word(ebp - 0x38);
    m.word(r.edi, m.word(ebp - 0x5c));
    m.word(r.edi + 4, m.word(ebp - 0x58));
    r.edi += 8;
    r.esi = ebp - 0x54;
    r.eax = m.word(ebp - 0x38);
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.edx = r.edx;
    cpu.esi = r.esi;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edi = saved[2];
    cpu.ebp = saved[3];
    cpu.esp = entry + 4;
}

/* sub_4de700: a view's spray streaks (view in eax, its group at +4; edx
 * particles; ebx where the count of live ones goes; the result's two words
 * to esi, which is returned): as sub_4de070, but each particle drawn as a
 * streak from where it was on the screen last time (its state's +0, +4,
 * kept with byte 9) to where it is now, half way between scaled by
 * [0x54982c]; quads of 0x80 bytes (flags 1 at +4 and +6), a respawned
 * particle also handed to sub_414f90, [0x54981c] the scatter and the
 * streak's width. */
void streaksNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.edi, cpu.ebp };
    // push ecx, edi, ebp; mov ebp, esp; sub esp, 0x60
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = entry - 12 - 0x60;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto result = [&]() {
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    auto compareToFlags = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
    };
    auto respawn = [&](auto at) {
        const x86::reg32 view = m.word(ebp - 0xc);
        r.eax = view;
        m.store(at() + 4, M::add(m.load(view + 0xc), m.load(0x8ca264)));
        for (x86::reg32 field : { 8u, 0x10u })
        {
            if (!callTo(0x4eb230))
                return false;
            const double spread = m.load(0x8ca25c);
            const double scattered = M::sub(M::mul(M::mul(result(), spread), m.load(0x54981c)), spread);
            r.eax = m.word(ebp - 0xc);
            m.store(at() + (field == 8 ? 0 : 8), M::add(scattered, m.load(r.eax + field)));
        }
        return true;
    };

    m.word(ebp - 0x28, r.esi);
    m.word(ebp - 0xc, r.eax);
    m.word(ebp - 0x20, r.edx);
    m.word(ebp - 0x2c, r.ebx);
    r.eax = m.word(r.eax + 4);
    m.word(ebp - 0x1c, 0);
    m.word(ebp - 0x24, 0);
    m.word(ebp - 0x18, r.eax);
    {
        const x86::reg32 grey = m.word(0x8ca238);
        r.eax = (grey << 8) | (grey << 16) | (grey << 24);
        r.edx = r.eax | grey;
    }
    r.ecx = 0;
    m.word(ebp - 0x14, r.edx);
    r.edx = m.word(ebp - 0x20) << 7;
    r.eax = ebp - 0x38;
    m.word(r.ebx, 0);
    if (!callTo(0x4bbe40))
        return;
    flags.logic(r.eax);
    if (r.eax)
    {
        r.eax = m.word(ebp - 0x38);
        m.word(ebp - 4, r.eax);
        // port (apply_alpha_intensity): test byte [0x7a3a58], 0x40 as if set
        flags.logic8(0x40);
        const double glare = m.load(0x6fbc38);
        app->getMemory<double>(ebp - 0x50) = glare;
        if (glareBelowOne(cpu, flags, r, glare))
        {
            const x86::reg32 scale = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x54980c)))));
            m.word(esp - 4, scale);
            darken(flags, r, m.word(ebp - 0x14), scale);
            m.word(ebp - 0x14, r.eax);
        }
        r.eax = m.word(ebp - 0x18);
        r.edi = r.eax * 640;
        push(0x8ba4c0);
        r.ecx = m.word(ebp - 0xc) + 0x38;
        r.ebx = m.word(ebp - 0xc) + 0x44;
        r.edx = r.edi * 15 + 0x8c56e8;
        r.eax = m.word(ebp - 0x20);
        if (!callTo(0x4bf4c0))
            return;
        r.ebx = 0;
        m.word(ebp - 0x10, 0);
        for (;; flags.inc(m.word(ebp - 0x10)), m.word(ebp - 0x10, m.word(ebp - 0x10) + 1))
        {
            r.eax = m.word(ebp - 0x10);
            flags.compare(r.eax, m.word(ebp - 0x20));
            if (x86::sreg32(r.eax) >= x86::sreg32(m.word(ebp - 0x20)))
                break;
            r.edi = m.word(ebp - 0x18) * 640;
            r.esi = r.edi * 15;
            r.edi = r.eax * 12;
            r.ebx = 0x8c0be8 + r.esi + r.edi;
            setByte(r.edx, 0, m.byte(r.ebx + 8));
            flags.logic8(x86::reg8(r.edx));
            if (x86::reg8(r.edx))
                continue;
            r.edx = m.word(ebp - 0x24) + 1;
            r.eax <<= 5;
            m.word(ebp - 0x24, r.edx);
            flags.add(0x8ba4c0, r.eax);
            r.edx = 0x8ba4c0 + r.eax;
            const x86::reg32 v = r.edx;
            m.word(ebp - 8, v);
            compareToFlags(m.load(v + 8), m.loadDouble(0x549814));
            bool behind = !cpu.flags.cf && !cpu.flags.zf;
            if (behind)
            {
                compareToFlags(0.0, m.load(v + 8));
                behind = !cpu.flags.cf && !cpu.flags.zf;
            }
            if (behind)
            {
                if (!respawn([&]() { return r.edi + r.esi + 0x8c56e8; }))
                    return;
                m.byte(r.ebx + 9, 0);
                continue;
            }
            r.eax = m.word(ebp - 8);
            compareToFlags(0.0, m.load(r.eax + 8));
            bool lost = !cpu.flags.cf;
            if (!lost)
            {
                r.eax = m.word(ebp - 8);
                compareToFlags(fild32(m, 0x8ca24c), m.load(r.eax + 8));
                lost = cpu.flags.cf;
            }
            if (lost)
            {
                m.byte(r.ebx + 9, 0);
                continue;
            }
            r.eax = m.word(ebp - 8);
            flags.compare8(m.byte(r.eax + 0x14), 0);
            if (m.byte(r.eax + 0x14))
            {
                m.byte(r.ebx + 9, 0);
                continue;
            }
            compareToFlags(m.load(r.eax + 8), m.loadDouble(0x549824));
            if (cpu.flags.cf)
            {
                r.eax = m.word(ebp - 0x10);
                r.edi = m.word(ebp - 0x18) * 640;
                r.esi = r.eax * 12;
                r.edx = r.edi * 15;
                r.esi += r.edx;
                r.ecx = m.word(r.esi + 0x8c56f0);
                push(r.ecx);
                r.edi = m.word(r.esi + 0x8c56ec);
                push(r.edi);
                r.eax = m.word(r.esi + 0x8c56e8);
                push(r.eax);
                r.eax = m.word(ebp - 0xc);
                if (!callTo(0x4dd930))
                    return;
                flags.logic(r.eax);
                if (r.eax)
                {
                    r.edx = 0xffffffff;
                    r.eax = m.word(ebp - 0x18);
                    if (!callTo(0x414f90))
                        return;
                    if (!respawn([&]() { return r.esi + 0x8c56e8; }))
                        return;
                    m.byte(r.ebx + 9, 0);
                    continue;
                }
            }
            {
                // The streak from the last place (state +0, +4) to this one.
                r.eax = m.word(ebp - 8);
                m.word(ebp - 0x58, m.word(r.eax));
                m.word(ebp - 0x54, m.word(r.eax + 4));
                setByte(r.ecx, 0, m.byte(r.ebx + 9));
                flags.logic8(x86::reg8(r.ecx));
                const double k = m.loadDouble(0x54982c);
                if (x86::reg8(r.ecx))
                {
                    const double y = M::mul(M::add(m.load(ebp - 0x54), m.load(r.ebx + 4)), k);
                    const double x = M::mul(k, M::add(m.load(ebp - 0x58), m.load(r.ebx)));
                    m.store(ebp - 0x3c, y);
                    m.store(ebp - 0x40, x);
                }
                else
                {
                    m.word(ebp - 0x40, m.word(ebp - 0x58));
                    m.word(ebp - 0x3c, m.word(ebp - 0x54));
                    m.word(r.ebx, m.word(ebp - 0x58));
                    m.word(r.ebx + 4, m.word(ebp - 0x54));
                    r.edi = r.ebx + 8;
                    r.esi = ebp - 0x50;
                }
                const double x = M::mul(M::add(m.load(ebp - 0x58), m.load(r.ebx)), k);
                const double y = M::mul(k, M::add(m.load(ebp - 0x54), m.load(r.ebx + 4)));
                const x86::reg32 record = m.word(ebp - 4);
                m.word(ebp - 0x34, record + 0x40);
                m.byte(r.ebx + 9, 1);
                m.word(ebp - 0x30, record + 0x60);
                m.store(r.ebx, x);
                m.store(r.ebx + 4, y);
                app->getMemory<x86::reg16>(record + 4) = 1;
                r.esi = record + 0x20;
                m.word(record + 0x18, 0x8ca200);
                app->getMemory<x86::reg16>(record + 6) = 1;
                m.word(record + 8, r.esi);
                m.word(record + 0xc, record + 0x40);
                m.word(record + 0x10, record + 0x60);
                m.word(record, record + 0x80);
                const x86::reg32 v = m.word(ebp - 8);
                // The two ends, then the second shifted along x.
                const x86::reg32 ends[] = { record + 0x20, record + 0x40 };
                const x86::reg32 xs[] = { ebp - 0x40, ebp - 0x58 };
                for (int c = 0; c < 2; ++c)
                {
                    const x86::reg32 q = ends[c];
                    m.word(q, m.word(xs[c]));
                    m.word(q + 4, m.word(xs[c] + 4));
                    r.ecx = m.word(v + 8);
                    m.word(q + 8, r.ecx);
                    m.store(q + 0xc, m.load(v + 0xc));
                    m.word(q + 0x10, m.word(ebp - 0x14));
                    r.eax = m.word(ebp - 0x14);
                    r.edx = q;
                    screenCodeFlags(m, q, q + 0x14, r, flags);
                }
                for (x86::reg32 k8 = 0; k8 < 0x20; k8 += 4)
                    m.word(record + 0x60 + k8, m.word(record + 0x40 + k8));
                r.ecx = 0;
                r.eax = record + 0x60;
                m.store(r.eax, M::add(m.load(r.eax), m.load(0x54981c)));
                screenCodeFlags(m, r.eax, r.eax + 0x14, r, flags);
                r.edx = m.word(ebp - 0x1c) + 1;
                r.ecx = m.word(ebp - 4);
                flags.add(r.ecx, 0x80);
                r.ecx += 0x80;
                m.word(ebp - 0x1c, r.edx);
                m.word(ebp - 4, r.ecx);
            }
        }
        r.edx = m.word(ebp - 0x2c);
        r.eax = m.word(ebp - 0x24);
        r.esi = m.word(ebp - 0x1c);
        m.word(r.edx, r.eax);
        flags.logic(r.esi);
        if (r.esi)
        {
            r.edi = m.word(ebp - 4);
            r.eax = ebp - 0x38;
            r.edx = r.esi << 7;
            r.esi = ebp - 0x48;
            flags.compare(r.edi, 0x80);
            r.edi -= 0x80;
            if (!callTo(0x4bbde0))
                return;
            r.eax = m.word(ebp - 0x38);
            m.word(ebp - 4, r.edi);
            m.word(ebp - 0x44, r.edi);
            m.word(r.edi, 0);
            m.word(ebp - 0x48, r.eax);
            m.word(ebp - 0x60, m.word(r.esi));
            m.word(ebp - 0x5c, m.word(r.esi + 4));
            r.edi = ebp - 0x58;
            r.esi += 8;
            goto done;
        }
    }
    r.esi = ebp - 0x60;
    if (!callTo(0x4bbe70))
        return;
done:
    r.edi = m.word(ebp - 0x28);
    m.word(r.edi, m.word(ebp - 0x60));
    m.word(r.edi + 4, m.word(ebp - 0x5c));
    r.esi = ebp - 0x58;
    r.eax = m.word(ebp - 0x28);
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.edx = r.edx;
    cpu.esi = r.esi;
    cpu.ecx = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 4;
}


/* sub_41d620: the views' distances for View Distance ([0x6fbc28], 0 Full):
 * the far, fog-near and fog-far distances of the main view (or both split
 * halves) times the multiplier for the setting from 0x41a8b0 (reduced,
 * [0x6fd4c8]) or 0x41a8bc, in 16.16; at Full the fog to the far distance.
 * Ports: at Full the far distance stays whole and a split view's reaches
 * 500, and the mirror takes the main view's distances.  Then every view's
 * distances as floats times [0x536ce4]. */
void viewDistancesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18
    const x86::reg32 ebp = entry - 24;
    IntegerFlags flags;
    x86::reg32 eax = m.word(0x6fbc28) - 1;
    flags.logic(eax);
    if (x86::sreg32(eax) < 0)
        eax = 0;
    // port (apply_view_distance): Full leaves the far distance whole
    const x86::reg32 reduced = viewDistanceReduced(app, m.word(0x6fd4c8));
    const x86::reg32 row = eax * 4;
    flags.logic(reduced);
    const x86::reg32 table = reduced ? ebp - 0x18 : ebp - 0xc;
    for (x86::reg32 k = 0; k < 0xc; k += 4)
        m.word(table + k, m.word((reduced ? 0x41a8b0 : 0x41a8bc) + k));
    // 16.16 times 16.16, rounded at bit 15 (imul, shl, shr, adc)
    auto scale = [&](x86::reg32 distance, bool last) {
        const std::uint64_t product = std::uint64_t(x86::sreg64(x86::sreg32(distance)) * x86::sreg32(m.word(table + row)));
        const x86::reg32 low = x86::reg32(product), high = x86::reg32(product >> 32) << 16;
        const bool carry = (low >> 15) & 1;
        const x86::reg32 shifted = low >> 16;
        const x86::reg32 result = shifted + high + carry;
        if (last)
        {
            flags.of = ((shifted >> 31) == (high >> 31)) && ((shifted >> 31) != (result >> 31));
            flags.cf = result < shifted || (carry && result == shifted);
            flags.zf = !result;
            flags.sf = result >> 31;
            flags.set = true;
        }
        return result;
    };
    const x86::reg32 split = m.word(0x6fd3b0);
    flags.compare(split, 1);
    if (split == 1)
    {
        // port (apply_view_distance): Full reaches 500 in split screen too
        const x86::reg32 far = scale(reduced ? 0x1b80000 : splitFarDistance(app, 0x1b80000), false);
        m.word(0x5dd15c, far);
        m.word(0x5dd108, far);
        const x86::reg32 fogNear = scale(0x3c0000, false);
        m.word(0x5dd16c, fogNear);
        m.word(0x5dd118, fogNear);
        eax = scale(0x5c0000, true);
        m.word(0x5dd170, eax);
    }
    else
    {
        m.word(0x5dd108, scale(0x1f40000, false));
        m.word(0x5dd118, scale(0x6e0000, false));
        eax = scale(0x960000, true);
    }
    m.word(0x5dd11c, eax);
    flags.compare(m.word(0x6fbc28), 0);
    if (m.word(0x6fbc28) == 0)
    {
        eax = m.word(0x5dd108);
        m.word(0x5dd11c, eax);
        m.word(0x5dd118, eax);
        flags.compare(m.word(0x6fd3b0), 1);
        if (m.word(0x6fd3b0) == 1)
        {
            eax = m.word(0x5dd15c);
            m.word(0x5dd170, eax);
            m.word(0x5dd16c, eax);
        }
    }
    // port (apply_mirror_detail): the mirror at the main view's distances
    mirrorFollowsMain(app);
    const double factor = m.loadDouble(0x536ce4);
    for (x86::reg32 view = 0;; )
    {
        flags.compare(view, m.word(0x5dd0dc));
        if (x86::sreg32(view) >= x86::sreg32(m.word(0x5dd0dc)))
            break;
        flags.add(view * 20, view);
        eax = view * 21;
        const x86::reg32 record = 0x5dd0e0 + eax * 4;
        const double a = M::mul(fild32(m, record + 0x20), factor);
        const double b = M::mul(fild32(m, record + 0x24), factor);
        const double d = M::mul(fild32(m, record + 0x28), factor);
        const double e = M::mul(fild32(m, record + 0x3c), factor);
        const double f = M::mul(factor, fild32(m, record + 0x38));
        flags.inc(view);
        ++view;
        m.store(record + 0x2c, a);
        m.store(record + 0x30, b);
        m.store(record + 0x34, d);
        m.store(record + 0x44, e);
        m.store(record + 0x40, f);
    }
    flags.store(cpu);
    cpu.eax = eax;
    cpu.esp = entry + 4;
}

/* sub_4b56e0: the transform buffer's size (four times the original's, a
 * port) to [eax] and edx, sub_4e1620 for 0x53ec28. */
void transformBufferNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.edx, cpu.ebp };
    // push ebx, edx, ebp; mov ebp, esp
    const x86::reg32 ebp = entry - 12;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    // port (apply_transform_buffer): four times the transform buffer
    r.edx = 4u * 799744u;
    m.word(r.eax, 4u * 799744u);
    r.ebx = 0;
    r.eax = 0x53ec28;
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp, 0x4e1620))
        return;
    cpu.eax = r.eax;
    cpu.ecx = r.ecx;
    cpu.esi = r.esi;
    cpu.edi = r.edi;
    cpu.ebx = saved[0];
    cpu.edx = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 4;
}

/* sub_4bef50: a screen mode (eax, at least 1) from the driver's list
 * (THRASH_about's +0x3c count, +0x40 the 40-byte entries): the same as now
 * only refreshes 0x7cdae0 and sub_4bebb0's rectangle; another within the
 * list is set by THRASH_setvideomode (16 bits at most, triple buffered when
 * the entry allows) between clears and flips, then the same refresh, the
 * window and [0x7a3a68] by the buffering. */
void screenModeNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 8
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = entry - 24 - 8;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto copyWords = [&](x86::reg32 count) {
        for (x86::reg32 k = 0; k < count; ++k)
            m.word(r.edi + 4 * k, m.word(r.esi + 4 * k));
        r.edi += 4 * count;
        r.esi += 4 * count;
        r.ecx = 0;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    // sub_4bebb0 with the mode's size, its rectangle copied to 0x7cdac0
    auto refresh = [&](bool clear) {
        if (!clear)
        {
            r.eax = 0;
            flags.logic(0);
        }
        r.ecx = m.word(0x7cdae4);
        r.ebx = m.word(0x7cdae0);
        r.edi = 0x7cdac0;
        if (clear)
        {
            r.edx = 0;
            r.eax = 0;
        }
        r.esi = 0x7cda60;
        if (!callTo(0x4bebb0))
            return false;
        r.ecx = 8;
        copyWords(8);
        return true;
    };
    // window 2, the 2D state, clear and flip twice
    auto clearTwice = [&]() {
        push(2);
        if (!callTo(m.word(0x9ef938)))
            return false;
        r.edx = 0xff000000;
        r.eax = 3;
        return callTo(0x431900) && callTo(m.word(0x9ef970)) && callTo(m.word(0x9ef940)) && callTo(m.word(0x9ef970))
               && callTo(m.word(0x9ef940));
    };

    r.ebx = r.eax;
    if (!callTo(m.word(0x9ef980)))
        return;
    r.esi = r.eax;
    flags.compare(r.ebx, 1);
    if (x86::sreg32(r.ebx) < 1)
        r.ebx = 1;
    r.eax = r.ebx * 40;
    r.edx = m.word(0x55fec0);
    flags.compare(r.ebx, r.edx);
    if (r.ebx == r.edx)
    {
        r.ecx = 10;
        r.edx = m.word(r.esi + 0x40);
        r.edi = 0x7cdae0;
        r.esi = r.eax + r.edx;
        r.edx = 0;
        copyWords(10);
        if (!refresh(false))
            return;
        return leave();
    }
    r.edx = m.word(r.esi + 0x3c);
    flags.compare(r.ebx, 1);
    if (x86::sreg32(r.ebx) < 1)
        return leave();
    flags.compare(r.ebx, r.edx);
    if (x86::sreg32(r.ebx) > x86::sreg32(r.edx))
        return leave();
    r.edx = m.word(r.esi + 0x40) + r.eax;
    r.ecx = m.word(r.edx + 0x14);
    flags.compare(r.ecx, 2);
    r.edx = x86::sreg32(r.ecx) <= 2 ? r.ecx : 2;
    r.eax = r.ebx * 5;
    r.ecx = m.word(r.esi + 0x40);
    r.edi = m.word(r.ecx + r.eax * 8 + 0x18);
    m.word(ebp - 8, r.edx);
    flags.compare(r.edx, r.edi);
    if (x86::sreg32(r.edx) > x86::sreg32(r.edi))
    {
        r.edx = 0;
        m.word(ebp - 4, 0);
    }
    else
        m.word(ebp - 4, 1);
    if (!callTo(m.word(0x9ef950)) || !callTo(m.word(0x9ef940)) || !clearTwice())
        return;
    r.ecx = m.word(ebp - 4);
    push(r.ecx);
    r.edi = m.word(ebp - 8);
    push(r.edi);
    push(r.ebx);
    if (!callTo(m.word(0x9ef958)))
        return;
    flags.logic(r.eax);
    if (r.eax)
    {
        r.eax = r.ebx * 5;
        r.ecx = 10;
        r.esi = m.word(r.esi + 0x40);
        r.eax <<= 3;
        r.edi = 0x7cdae0;
        r.esi += r.eax;
        m.word(0x55fec0, r.ebx);
        r.eax = m.word(ebp - 4);
        copyWords(10);
        flags.logic(r.eax);
        if (r.eax)
        {
            r.edx = 1;
            r.eax = 4;
            m.word(0x7a3a68, r.edx);
        }
        else
        {
            m.word(0x7a3a68, r.eax);
            r.edx = 0;
            r.eax = 4;
        }
        if (!callTo(0x431900) || !refresh(true))
            return;
    }
    if (!clearTwice())
        return;
    leave();
}

/* sub_4b59c0: the driver eax (0..10) started (edx the window's message
 * when it cannot be): its library (0x55fd68) loaded by sub_4f87e0,
 * sub_4f8830, sub_4f8930 (sub_4f2a7d without [0x564364]), the renderer's
 * states set as each driver wants, [0x55fed0], [0x7a3a60], [0x7a3a64],
 * [0x7a3a68], [0x7a3a6c], [0x7a3a70]; the 2D one only on a screen of 8 bits
 * or more, those not built in with a message and sub_4f2a7d. */
void startDriverNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = entry - 20 - 0x1c;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto setState = [&](x86::reg32 a, x86::reg32 b) {
        push(b);
        push(a);
        return callTo(m.word(0x9ef96c));
    };
    auto import = [&](x86::reg32 slot) { return callTo(m.word(cpu.ecs + slot)); };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    // The driver's library: load, link, start, and leave if that failed.
    auto load = [&](x86::reg32 library) {
        r.eax = library;
        if (!callTo(0x4f87e0) || !callTo(0x4f8830) || !callTo(0x4f8930))
            return false;
        flags.compare(m.word(0x564364), 0);
        return m.word(0x564364) != 0 || callTo(0x4f2a7d);
    };

    r.ebx = r.edx;
    r.edx = 0;
    m.word(ebp - 4, r.eax);
    m.word(0x7a3a6c, 0);
    m.word(0x7a3a70, 0);
    flags.compare(r.eax, 0xa);
    if (r.eax > 0xa)
    {
        r.eax = r.ebx;
        if (!callTo(0x4b5e50) || !callTo(0x4f2a7d))
            return;
        return leave();
    }
    const x86::reg32 driver = r.eax;
    flags.shl(r.eax, 2);
    r.eax <<= 2;
    const x86::reg32 library = m.word(driver * 4 + 0x55fd68);
    switch (driver)
    {
    case 0:
        if (!load(library))
            return;
        m.word(0x7a3a70, 1);
        break;
    case 1:
    case 2:
        if (!load(library) || !setState(0xa, 2) || !setState(3, 0) || !callTo(m.word(0x9ef970)) || !setState(4, 0))
            return;
        r.eax = 0;
        push(r.eax);
        push(6);
        m.word(0x7a3a68, 0);
        m.word(0x7a3a60, 0);
        if (!callTo(m.word(0x9ef96c)) || !setState(2, 0))
            return;
        r.ecx = 0;
        r.ebx = 0x400000;
        m.word(0x7a3a64, r.ecx);
        if (!callTo(0x431c50))
            return;
        m.word(0x55fed0, r.ebx);
        m.word(0x7a3a70, r.ecx);
        break;
    case 3:
    case 4:
    {
        r.ebx = 1;
        r.edx = 0x53fd34;
        r.eax = 0x53fd38;
        if (!callTo(0x4f8b50) || !load(m.word(m.word(ebp - 4) * 4 + 0x55fd68)))
            return;
        for (x86::reg32 name : { 0x53fd48u, 0x53fd54u, 0x53fd60u, 0x53fd6cu, 0x53fd78u, 0x53fd88u, 0x53fd98u, 0x53fda8u })
        {
            r.ebx = 1;
            r.edx = 0x53ec28;
            r.eax = name;
            if (!callTo(0x4f8b50))
                return;
        }
        if (!setState(0xa, 2) || !callTo(m.word(0x9ef970)))
            return;
        push(1);
        push(4);
        r.ebx = 0x800000;
        if (!callTo(m.word(0x9ef96c)))
            return;
        push(0);
        r.esi = 1;
        r.eax = 1;
        push(2);
        m.word(0x7a3a68, r.eax);
        m.word(0x7a3a60, r.eax);
        if (!callTo(m.word(0x9ef96c)))
            return;
        r.ecx = 4;
        m.word(0x55fed0, r.ebx);
        m.word(0x7a3a70, r.esi);
        m.word(0x7a3a64, r.ecx);
        break;
    }
    case 5:
        r.edi = 1;
        m.word(0x7a3a6c, r.edi);
        if (!load(m.word(r.eax + 0x55fd68)) || !setState(0xa, 2) || !callTo(m.word(0x9ef970)) || !setState(4, 1))
            return;
        r.edx = 1;
        push(r.edx);
        push(6);
        m.word(0x7a3a68, r.edx);
        m.word(0x7a3a60, r.edx);
        if (!callTo(m.word(0x9ef96c)))
            return;
        push(0);
        push(2);
        r.esi = 0x200000;
        r.ebx = 0;
        if (!callTo(m.word(0x9ef96c)))
            return;
        m.word(0x7a3a64, r.ebx);
        m.word(0x55fed0, r.esi);
        break;
    case 6:
        if (!import(0x534740))
            return;
        push(r.eax);
        if (!import(0x534754))
            return;
        push(0xc);
        push(r.eax);
        if (!import(0x534458))
            return;
        flags.compare(r.eax, 8);
        if (x86::sreg32(r.eax) < 8)
        {
            for (x86::reg32 k = 0; k < 0x18; k += 4)
                m.word(ebp - 0x1c + k, m.word(0x4b55b0 + k));
            r.ecx = 0;
            r.edi = ebp - 4;
            r.esi = 0x4b55c8;
            push(0x30);
            push(0x53ec28);
            r.edi = m.word(ebp + r.ebx * 4 - 0x1c);
            push(r.edi);
            push(0);
            if (!import(0x534764) || !callTo(0x4f2a7d))
                return;
        }
        if (!load(m.word(m.word(ebp - 4) * 4 + 0x55fd68)))
            return;
        r.edx = m.word(0x55fdbc);
        push(r.edx);
        if (!callTo(m.word(0x9ef944)))
            return;
        push(1);
        push(2);
        push(1);
        if (!callTo(m.word(0x9ef958)) || !callTo(0x4b5940) || !setState(0xa, 2) || !setState(0x68, 0)
            || !callTo(m.word(0x9ef970)) || !setState(4, 1))
            return;
        r.ecx = 1;
        push(r.ecx);
        push(6);
        m.word(0x7a3a68, r.ecx);
        m.word(0x7a3a60, r.ecx);
        if (!callTo(m.word(0x9ef96c)))
            return;
        push(0);
        push(2);
        r.edi = 0x200000;
        r.esi = 0;
        if (!callTo(m.word(0x9ef96c)))
            return;
        m.word(0x7a3a64, r.esi);
        m.word(0x55fed0, r.edi);
        break;
    case 7:
    case 8:
        if (!load(library) || !setState(0xa, 2) || !callTo(m.word(0x9ef970)) || !setState(4, 2))
            return;
        push(0);
        r.edx = 2;
        push(r.edx);
        m.word(0x7a3a68, r.edx);
        m.word(0x7a3a60, r.edx);
        if (!callTo(m.word(0x9ef96c)) || !setState(0xce, 1) || !setState(6, 1))
            return;
        push(2);
        r.esi = 0x200000;
        push(0xc9);
        r.edi = 1;
        r.ebx = 0;
        if (!callTo(m.word(0x9ef96c)))
            return;
        m.word(0x7a3a64, r.ebx);
        m.word(0x55fed0, r.esi);
        m.word(0x7a3a70, r.edi);
        break;
    default:
        push(0x30);
        push(0x53ec28);
        push(0x53fdb4);
        push(0);
        if (!import(0x534764) || !callTo(0x4f2a7d))
            return;
        m.word(0x7a3a70, 1);
        break;
    }
    leave();
}

/* sub_47f650: the frame around a HUD element, eight textured quads at eax
 * (0x80 bytes each, four 0x20-byte corners: x, y at +0, +4, s, t at +0x18,
 * +0x1c) -- the corners and the edges of a 9-slice, the middle left out --
 * from the element's inside, x ebx to [esp+4], y edx to ecx, and a border
 * of hudBorder outside it.  ret 4. */
void hudFrameNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 quads = cpu.eax;
    const x86::reg32 border = hudBorder(app, cpu, 0x53b4f8, 0x53b4fc);
    const x86::reg32 left = cpu.ebx, right = m.word(entry + 4), top = cpu.edx, bottom = cpu.ecx;
    const x86::reg32 x[4] = { left - border, left, right, right + border };
    const x86::reg32 y[4] = { top - border, top, bottom, bottom + border };
    struct Quad
    {
        unsigned x0, x1, y0, y1;  // indices into x and y
        x86::reg32 s0, s1, t0, t1;
    };
    static const Quad frame[8] = {
        { 0, 1, 0, 1, 0x3f298000, 0x3f2c8000, 0x3f3a8000, 0x3f3d8000 },  // top left
        { 1, 2, 0, 1, 0x3f318000, 0x3f328000, 0x3f3e8000, 0x3f418000 },  // top
        { 2, 3, 0, 1, 0x3f2d8000, 0x3f308000, 0x3f3a8000, 0x3f3d8000 },  // top right
        { 2, 3, 1, 2, 0x3f318000, 0x3f348000, 0x3f3a8000, 0x3f3b8000 },  // right
        { 2, 3, 2, 3, 0x3f2d8000, 0x3f308000, 0x3f3e8000, 0x3f418000 },  // bottom right
        { 1, 2, 2, 3, 0x3f318000, 0x3f328000, 0x3f3e8000, 0x3f418000 },  // bottom
        { 0, 1, 2, 3, 0x3f298000, 0x3f2c8000, 0x3f3e8000, 0x3f418000 },  // bottom left
        { 0, 1, 1, 2, 0x3f318000, 0x3f348000, 0x3f3a8000, 0x3f3b8000 },  // left
    };
    for (unsigned k = 0; k < 8; ++k)
    {
        const Quad& q = frame[k];
        const x86::reg32 at = quads + k * 0x80;
        // its corners clockwise from the top left
        const unsigned xs[4] = { q.x0, q.x1, q.x1, q.x0 }, ys[4] = { q.y0, q.y0, q.y1, q.y1 };
        const x86::reg32 ss[4] = { q.s0, q.s1, q.s1, q.s0 }, ts[4] = { q.t0, q.t0, q.t1, q.t1 };
        for (unsigned c = 0; c < 4; ++c)
        {
            const x86::reg32 corner = at + c * 0x20;
            m.store(corner, double(x86::sreg32(x[xs[c]])));
            m.store(corner + 4, double(x86::sreg32(y[ys[c]])));
            m.word(corner + 0x18, ss[c]);
            m.word(corner + 0x1c, ts[c]);
        }
    }
    cpu.eax = m.word(quads + 0x3e4);
    cpu.ebx = left - border;
    cpu.edx = left - border;
    cpu.esp = entry + 8;
}

/* sub_480910: HUD element edx's rectangle for player eax, in pixels to the
 * table at 0x749758 (23 four-int rectangles a player): its layout's fractions
 * (0x6fbc74.., by split screen, the player's view (sub_422bd0) and bit 0x20
 * of its +0x200) times the width ebx and the height ecx, each truncated by
 * sub_4dfd56 and stored by fistp; then widescreenHudRect (a port,
 * tools/apply_widescreen.py). */
void hudRectNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 ebp = entry - 12;
    const x86::reg32 esp = ebp - 12;
    IntegerFlags flags;
    const x86::reg32 split = m.word(0x6fd3b0);
    flags.compare(split, 1);
    Registers r{ cpu.eax, cpu.ecx, cpu.eax + (split == 1 ? 1u : 0u), cpu.edx, cpu.eax, cpu.ebx };
    if (!callWithRegisters(app, cpu, flags, r, ebp, esp, 0x422bd0))
        return;
    m.word(ebp - 8, r.ecx);
    const x86::reg8 mirrored = m.byte(r.eax + 0x200) & 0x20;
    flags.logic8(mirrored);
    const x86::reg32 edx = r.edx << 4;
    const x86::reg32 ecx = r.ecx * 0x348 + (mirrored ? 0x1a4u : 0u) + edx;
    const double width = double(x86::sreg32(r.edi)), height = double(x86::sreg32(r.ebx));
    const double a = M::mul(width, m.load(ecx + 0x6fbc78));
    const double b = M::mul(width, m.load(ecx + 0x6fbc80));
    const double c = M::mul(height, m.load(ecx + 0x6fbc74));
    const double d = M::mul(height, m.load(ecx + 0x6fbc7c));
    m.word(ebp - 4, r.ebx);
    r.eax = r.esi * 4 - r.esi;
    r.ecx = ecx;
    r.edx = edx;
    cpu.fpu.count += 4;
    cpu.fpu.st(0) = x86::Float(a);
    cpu.fpu.st(1) = x86::Float(c);
    cpu.fpu.st(2) = x86::Float(b);
    cpu.fpu.st(3) = x86::Float(d);
    auto fxch = [&](int i) {
        const x86::Float top = cpu.fpu.st(0);
        cpu.fpu.st(0) = cpu.fpu.st(i);
        cpu.fpu.st(i) = top;
    };
    auto truncate = [&]() { return callWithRegisters(app, cpu, flags, r, ebp, esp, 0x4dfd56); };
    if (!truncate())
        return;
    r.eax <<= 3;
    fxch(2);
    if (!truncate())
        return;
    fxch(1);
    if (!truncate())
        return;
    r.eax -= r.esi;
    fxch(3);
    if (!truncate())
        return;
    r.eax <<= 4;
    // fxch st(2), then four fistp: st(2), st(1), st(3), st(0) as they are now
    const x86::reg32 x0 = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(2)));
    const x86::reg32 x1 = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(1)));
    const x86::reg32 y0 = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(3)));
    const x86::reg32 y1 = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0)));
    const x86::reg32 slot = r.edx + r.eax;
    m.word(slot + 0x749758, x0);
    m.word(slot + 0x749760, x1);
    m.word(slot + 0x74975c, y0);
    m.word(slot + 0x749764, y1);
    m.word(ebp - 4, y1);
    // port (apply_widescreen): pictures in the HUD keep their shape on a wide screen
    widescreenHudRect(app, slot);
    cpu.fpu.count -= 4;
    flags.store(cpu);
    cpu.eax = y1;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = slot;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 4;
}

/* sub_480fc0: HUD element 1 for player eax, from its rectangle at 0x749768:
 * its frame (sub_47f650 into 0x791a8c's 0x400 bytes a player) a border inside
 * the rectangle, and the picture in it at 0x791b38 (0x100 bytes a player):
 * the corners of its quad, the same inset, and its centre at +0xc0. */
void hudPictureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 ebp = entry - 24;
    IntegerFlags flags;
    auto border = [&]() { return hudBorder(app, cpu, 0x53b518, 0x53b51c); };
    const x86::reg32 player = cpu.eax;
    const x86::reg32 rect = player * 0x170 + 0x749758 + 0x10;
    // the border, by fistp each time it is used: the same each time
    const x86::reg32 i = border();
    const x86::reg32 right = m.word(rect + 8) - i;
    const x86::reg32 bottom = m.word(rect + 0xc) - i;
    const x86::reg32 left = m.word(rect) + i;
    const x86::reg32 top = m.word(rect + 4) + i;
    m.word(ebp - 4, top);
    // push edx: the frame's right edge, sub_47f650's argument
    m.word(ebp - 8, right);
    Registers r{ (player << 10) + m.word(0x791a8c), left, bottom, top, rect, player };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 8, 0x47f650))
        return;
    const x86::reg32 j = border();
    const x86::reg32 quad = m.word(0x791b38) + (r.edi << 8);
    auto corner = [&](x86::reg32 value, x86::reg32 a, x86::reg32 b) {
        m.word(ebp - 4, value);
        m.store(quad + a, double(x86::sreg32(value)));
        m.store(quad + b, double(x86::sreg32(value)));
    };
    corner(j + m.word(r.esi), 0x60, 0);
    corner(m.word(r.esi + 8) - j, 0x40, 0x20);
    corner(j + m.word(r.esi + 4), 0x24, 4);
    corner(m.word(r.esi + 0xc) - j, 0x64, 0x44);
    // (a + b) / 2 rounded toward 0, as cdq-free sar, sub, sar does it
    auto middle = [](x86::reg32 a, x86::reg32 b) {
        const x86::reg32 sum = a + b;
        return x86::reg32(x86::sreg32(sum - x86::reg32(x86::sreg32(sum) >> 31)) >> 1);
    };
    x86::reg32 eax = middle(m.word(r.esi), m.word(r.esi + 8));
    m.word(ebp - 4, eax);
    m.store(quad + 0xc0, double(x86::sreg32(eax)));
    eax = middle(m.word(r.esi + 4), m.word(r.esi + 0xc));
    m.word(ebp - 4, eax);
    m.store(quad + 0xc4, double(x86::sreg32(eax)));
    flags.store(cpu);
    cpu.eax = eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_4812e0: HUD element 3 for player eax, from its rectangle at 0x749788:
 * its frame (sub_47f650 into 0x791b34's 0x400 bytes a player) a border inside
 * it, and the picture at 0x791b2c (0xe0 bytes a player) laid over the frame's
 * inside corners; its height times [0x53b528] to 0x724760, and, where the
 * layout's kind ([0x6fbc6c], by split screen and bit 0x20 of the player's
 * view, sub_422bd0) is 3, its centre times [0x53b52c] to 0x724768. */
void hudDialNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 ebp = entry - 24;
    IntegerFlags flags;
    auto border = [&]() { return hudBorder(app, cpu, 0x53b520, 0x53b524); };
    const x86::reg32 split = m.word(0x6fd3b0);
    flags.compare(split, 1);
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.eax + (split == 1 ? 1u : 0u), cpu.eax, cpu.edi };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 12, 0x422bd0))
        return;
    m.word(ebp - 0xc, r.edx);
    const x86::reg8 mirrored = m.byte(r.eax + 0x200) & 0x20;
    flags.logic8(mirrored);
    if (!mirrored)
        r.ebx = 0;
    m.word(ebp - 8, mirrored ? 1 : 0);
    // the border, by fistp each time it is used: the same each time
    const x86::reg32 slot = r.esi * 0x170;
    const x86::reg32 i = border();
    const x86::reg32 right = m.word(slot + 0x749790) - i;
    // push ecx: the frame's right edge, sub_47f650's argument
    m.word(ebp - 16, right);
    const x86::reg32 bottom = m.word(slot + 0x749794) - i;
    const x86::reg32 left = m.word(slot + 0x749788) + i;
    const x86::reg32 top = i + m.word(slot + 0x74978c);
    const x86::reg32 frame = m.word(0x791b34) + (r.esi << 10);
    m.word(ebp - 4, frame);
    r = { frame, left, bottom, top, r.esi, r.esi << 10 };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 16, 0x47f650))
        return;
    const x86::reg32 j = border();
    const x86::reg32 picture = m.word(0x791b2c) + r.esi * 0xe0;
    auto place = [&](double value, x86::reg32 a, x86::reg32 b) {
        m.store(picture + a, value);
        m.store(picture + b, value);
    };
    // the frame sub_47f650 drew, its inside corners: [0x791b34] read again each time
    m.word(ebp - 4, j);
    place(M::add(double(x86::sreg32(j)), m.load(m.word(0x791b34) + r.edi)), 0x60, 0);
    place(M::add(double(x86::sreg32(j)), m.load(m.word(0x791b34) + r.edi + 4)), 0x24, 4);
    place(M::sub(m.load(m.word(0x791b34) + r.edi + 0x240), double(x86::sreg32(j))), 0x40, 0x20);
    const x86::reg32 inside = m.word(0x791b34) + r.edi;
    const x86::reg32 row = m.word(ebp - 0xc) * 0x348, column = m.word(ebp - 8) * 0x1a4;
    const double across = M::mul(M::sub(m.load(picture + 0x20), m.load(picture)), m.load(0x53b528));
    const double down = M::sub(m.load(inside + 0x244), double(x86::sreg32(j)));
    place(down, 0x64, 0x44);
    const x86::reg32 kind = m.word(row + column + 0x6fbc6c);
    m.store(r.esi * 4 + 0x724760, across);
    flags.compare(kind, 3);
    if (kind == 3)
    {
        const double scale = m.load(0x53b52c);
        m.store(r.esi * 8 + 0x724768, M::mul(M::add(m.load(picture), m.load(picture + 0x40)), scale));
        m.store(r.esi * 8 + 0x72476c, M::mul(scale, M::add(m.load(picture + 4), m.load(picture + 0x44))));
    }
    flags.store(cpu);
    cpu.eax = column;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_481aa0: HUD element 17 for player eax (22 where bit 0x20 of the
 * player's view, sub_422bd0, is set): its frame (sub_47f650 into 0x791b40's
 * 0x400 bytes a player) a border inside its rectangle, the quad at 0x791b58
 * (0x80 bytes a player) the same inset, and 0x700230..3c cleared. */
void hudPanelNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 ebp = entry - 24;
    IntegerFlags flags;
    auto border = [&]() { return hudBorder(app, cpu, 0x53b58c, 0x53b590); };
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.eax };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 4, 0x422bd0))
        return;
    const x86::reg32 player = r.edi;
    const x86::reg8 mirrored = m.byte(r.eax + 0x200) & 0x20;
    flags.logic8(mirrored);
    x86::reg32 rect = 0x749758 + player * 0x170;
    if (!mirrored)
        flags.add(rect, 0x110);
    rect += mirrored ? 0x160 : 0x110;
    // the border, by fistp each time it is used: the same each time
    const x86::reg32 i = border();
    const x86::reg32 right = m.word(rect + 8) - i;
    // push edx: the frame's right edge, sub_47f650's argument
    m.word(ebp - 8, right);
    const x86::reg32 bottom = m.word(rect + 0xc) - i;
    const x86::reg32 left = m.word(rect) + i;
    const x86::reg32 top = m.word(rect + 4) + i;
    m.word(ebp - 4, player << 10);
    r = { m.word(0x791b40) + (player << 10), left, bottom, top, rect, player };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 8, 0x47f650))
        return;
    const x86::reg32 j = border();
    const x86::reg32 quad = (r.edi << 7) + m.word(0x791b58);
    auto corner = [&](x86::reg32 value, x86::reg32 a, x86::reg32 b) {
        m.word(ebp - 4, value);
        m.store(quad + a, double(x86::sreg32(value)));
        m.store(quad + b, double(x86::sreg32(value)));
    };
    corner(j + m.word(r.esi), 0x60, 0);
    corner(m.word(r.esi + 8) - j, 0x40, 0x20);
    corner(j + m.word(r.esi + 4), 0x24, 4);
    const x86::reg32 last = m.word(r.esi + 0xc) - j;
    m.word(ebp - 4, last);
    m.word(0x700234, 0);
    m.word(0x70023c, 0);
    m.word(0x700238, 0);
    m.store(quad + 0x64, double(x86::sreg32(last)));
    m.word(0x700230, 0);
    m.store(quad + 0x44, double(x86::sreg32(last)));
    flags.store(cpu);
    cpu.eax = 0;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_481c50: the table of player eax -- the racers' standings, or with bit
 * 0x20 of its view (sub_422bd0) the cop's speeders -- as a panel as tall as
 * its rows (text size [0x791b4c], sub_4d1390): the cop's a row (sub_4d14f0,
 * sub_4d1540 at size 0x30, times 12.414/13 -- a port, the Modern Patch's
 * compact rows) for each of the [0x5efda0] entries at 0x725238 in use, the
 * racers' [0x7254f8] rows of the height (4:3, a port) times [0x53b5b0] and
 * [0x53b5b8] plus sub_4d1540's at 0x49; up from the box's bottom or down from
 * its top by bit 0x10 of the player's flags, to 0x724780 and 0x724788.  Then
 * its frame (sub_47f650 into 0x791b18) as wide as [0x7925c8] ([0x7925cc] for
 * the cop's), the quad at 0x791ad4 inside it, and hudTablePanel (a port). */
void hudTableNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = ebp - 12;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.eax, cpu.edi };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto border = [&]() { return hudBorder(app, cpu, 0x53b5a8, 0x53b5ac); };
    // a value through sub_4dfd56 and fistp
    auto truncate = [&](double value, x86::reg32& out) {
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(value);
        if (!callTo(0x4dfd56))
            return false;
        out = x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0)));
        cpu.fpu.count -= 1;
        return true;
    };
    if (!callTo(0x422bd0))
        return;
    r.eax = m.word(r.eax + 0x200) & 0x20;
    push(m.word(0x791b4c));
    m.word(ebp - 8, r.eax);
    if (!callTo(0x4d1390))
        return;
    r.edx = m.word(ebp - 8);
    cpu.fpu.count -= 1;  // fstp st(0)
    flags.logic(r.edx);
    if (r.edx)
    {
        // the cop's: a row for each speeder in the table
        r.edi = 0;
        for (r.eax = 0;; ++r.eax)
        {
            flags.compare(r.eax, m.word(0x5efda0));
            if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x5efda0)))
                break;
            flags.compare(m.word(r.eax * 4 + 0x725238), 0);
            if (m.word(r.eax * 4 + 0x725238))
            {
                flags.inc(r.edi);
                ++r.edi;
            }
            flags.inc(r.eax);
        }
        r.eax = 0x30;
        if (!callTo(0x4d14f0))
            return;
        r.edx = r.eax;
        r.eax = 0x30;
        if (!callTo(0x4d1540))
            return;
        r.eax += r.edx;
        m.word(ebp - 4, r.eax);
        // port (apply_hud_scale): the cop's rows as close as the Modern Patch has them, was [0x53b598] = 20
        const double row = M::mul(M::mul(double(x86::sreg32(r.eax)), 12.414), m.loadDouble(0x53b5a0));
        x86::reg32 height;
        if (!truncate(row, height))
            return;
        m.word(ebp - 0xc, height);
        r.ecx = height * r.edi;
        r.ebx = r.esi * 92;
        r.edx = r.esi * 0x170;
        setByte(r.eax, 1, m.byte(r.ebx + 0x7256e0));
        r.edi = r.esi * 4;
        flags.logic8(m.byte(r.ebx + 0x7256e0) & 0x10);
        if (m.byte(r.ebx + 0x7256e0) & 0x10)
        {
            const x86::reg32 i = border();
            r.ecx = m.word(r.edx + 0x7498a4) - r.ecx;
            r.eax = i + i + 4;
            flags.compare(r.ecx, r.eax);
            r.ecx -= r.eax;
            r.eax = m.word(r.edx + 0x7498a4);
            m.word(r.edi + 0x724780, r.ecx);
        }
        else
        {
            const x86::reg32 i = border();
            r.eax = m.word(r.edx + 0x74989c);
            r.ecx += r.eax;
            m.word(r.edi + 0x724780, r.eax);
            flags.add(i + i + 4, r.ecx);
            r.eax = i + i + 4 + r.ecx;
        }
        m.word(r.edi + 0x724788, r.eax);
    }
    else
    {
        // the racers': [0x7254f8] rows
        // port (apply_hud_scale): 4:3 proportions, was [0x7cdacc]
        const double size = M::mul(M::mul(double(x86::sreg32(hudReferenceHeight(app))), m.loadDouble(0x53b5b0)),
                                   m.loadDouble(0x53b5b8));
        r.edx = r.esi;
        r.ebx = r.esi * 3;
        r.eax = 0x49;
        x86::reg32 rowHeight;
        if (!truncate(size, rowHeight))
            return;
        r.ebx <<= 3;
        m.word(ebp - 4, rowHeight);
        if (!callTo(0x4d1540))
            return;
        r.ebx -= r.edx;
        r.ecx = (m.word(ebp - 4) + r.eax) * m.word(0x7254f8);
        r.edx = r.esi;
        r.edi = r.esi * 0x170;
        r.ebx <<= 2;
        setByte(r.eax, 0, m.byte(r.ebx + 0x7256c4));
        r.edx = r.esi * 4;
        flags.logic8(m.byte(r.ebx + 0x7256c4) & 0x10);
        if (m.byte(r.ebx + 0x7256c4) & 0x10)
        {
            const x86::reg32 i = border();
            r.ecx = m.word(r.edi + 0x749834) - r.ecx;
            r.eax = i + i + 4;
            flags.compare(r.ecx, r.eax);
            r.ecx -= r.eax;
            r.eax = m.word(r.edi + 0x749834);
            m.word(r.edx + 0x724780, r.ecx);
        }
        else
        {
            const x86::reg32 i = border();
            r.eax = m.word(r.edi + 0x74982c);
            r.ecx += r.eax;
            m.word(r.edx + 0x724780, r.eax);
            flags.add(i + i + 4, r.ecx);
            r.eax = i + i + 4 + r.ecx;
        }
        m.word(r.edx + 0x724788, r.eax);
    }
    // the frame
    const x86::reg32 player = r.esi;
    const bool cops = m.word(ebp - 8) != 0;
    x86::reg32 rect = 0x749758 + player * 0x170;
    flags.compare(m.word(ebp - 8), 0);
    if (!cops)
        flags.add(rect, 0xd0);
    rect += cops ? 0x140 : 0xd0;
    flags.compare(m.word(ebp - 8), 0);
    const x86::reg32 i = border();
    // push eax: the frame's right edge, sub_47f650's argument
    push(m.word(cops ? 0x7925cc : 0x7925c8) + m.word(rect));
    const x86::reg32 bottom = m.word(player * 4 + 0x724788);
    const x86::reg32 left = m.word(rect) + i;
    const x86::reg32 top = m.word(player * 4 + 0x724780) + i;
    m.word(ebp - 4, left);
    r = { (player << 10) + m.word(0x791b18), left, bottom, top, player, rect };
    if (!callTo(0x47f650))
        return;
    const x86::reg32 j = border();
    const x86::reg32 quad = m.word(0x791ad4) + (r.esi << 7);
    auto corner = [&](x86::reg32 value, x86::reg32 a, x86::reg32 b) {
        m.word(ebp - 4, value);
        m.store(quad + a, double(x86::sreg32(value)));
        m.store(quad + b, double(x86::sreg32(value)));
    };
    corner(j + m.word(r.edi), 0x60, 0);
    const x86::reg32 otherLayout = m.word(ebp - 8);
    flags.logic(otherLayout);
    corner(m.word(otherLayout ? 0x7925cc : 0x7925c8) + m.word(r.edi), 0x40, 0x20);
    const x86::reg32 eax = j + m.word(r.esi * 4 + 0x724780);
    corner(eax, 0x24, 4);
    const double bottomEdge = fild32(m, r.esi * 4 + 0x724788);
    m.store(quad + 0x64, bottomEdge);
    m.store(quad + 0x44, bottomEdge);
    // port (apply_hud_editor): the table's rectangle is its panel
    hudTablePanel(app, r.esi, m.word(ebp - 8) != 0);
    flags.store(cpu);
    cpu.eax = eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_487210: the rectangle of floats at edx (x0, y0 at +0, +4, x1, y1 at +8,
 * +0xc) kept for player eax at 0x791b60 and, where sub_481f90 has a frame
 * round it, moved in by a border (4:3, a port) at each side; then a right or
 * bottom edge on the screen's last column or row ([0x7cdac8], [0x7cdacc], not
 * ported: these are the screen's) is moved in by one. */
void hudInsetNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.ebp };
    // push ebx, ecx, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 ebp = entry - 12;
    IntegerFlags flags;
    const x86::reg32 player = cpu.eax;
    m.word(player * 4 + 0x791b60, cpu.edx);
    Registers r{ player, cpu.ebx, player, player * 4, cpu.esi, cpu.edi };
    if (!callWithRegisters(app, cpu, flags, r, ebp, ebp - 4, 0x481f90))
        return;
    flags.logic(r.eax);
    if (r.eax)
    {
        auto border = [&]() { return hudBorder(app, cpu, 0x53b690, 0x53b694); };
        const x86::reg32 i = border();
        const x86::reg32 at = r.edx + 0x791b60;
        m.word(ebp - 4, i);
        m.store(m.word(at), M::add(double(x86::sreg32(i)), m.load(m.word(at))));
        x86::reg32 rect = m.word(at);
        m.store(rect + 4, M::add(double(x86::sreg32(i)), m.load(rect + 4)));
        rect = m.word(at);
        m.store(rect + 0xc, M::sub(m.load(rect + 0xc), double(x86::sreg32(i))));
        rect = m.word(at);
        flags.compare(ebp - 4, 4);  // sub esp, 4
        m.store(rect + 8, M::sub(m.load(rect + 8), double(x86::sreg32(i))));
        r.eax = i;
    }
    // fcomp; fnstsw ax; sahf: the screen's size against the edge
    auto atEdge = [&](x86::reg32 size, x86::reg32 edge) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(fild32(m, size)), x86::Float(m.load(edge)));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
        return cpu.flags.zf;
    };
    x86::reg32 rect = m.word(r.ecx * 4 + 0x791b60);
    if (atEdge(0x7cdac8, rect + 0xc))
    {
        r.eax = m.word(0x7cdac8);
        flags.dec(r.eax);
        --r.eax;
        m.word(ebp - 4, r.eax);
        m.store(rect + 0xc, double(x86::sreg32(r.eax)));
    }
    rect = m.word(r.ecx * 4 + 0x791b60);
    if (atEdge(0x7cdacc, rect + 8))
    {
        r.eax = m.word(0x7cdacc) - 1;
        m.word(ebp - 4, r.eax);
        m.store(rect + 8, double(x86::sreg32(r.eax)));
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.edx = rect;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 4;
}

/* sub_48c7c0: the three-light start (and finish) panel: the quads at
 * 0x724790, 0x724810, 0x724890 drawn by THRASH_drawquad ([0x9ef94c]) with the
 * texture state (sub_431900) [0x749a38], under bit 2 of [0x7a3a58] state
 * 0x2bf on round the rest; then the lamp at 0x724d10 lit (0xffdcdb58, unlit
 * 0xff797c93) when [esp+4] is 0, the quads eax and edx (0x80 bytes each from
 * 0x724790) when it is 0 and 2, the lamp at 0x724c90, ebx and ecx when it is
 * 1, 1 and 3.  ret 4. */
void startLightsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 8
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 8;
    m.word(ebp - 8, cpu.eax);
    m.word(ebp - 4, cpu.ecx);
    const x86::reg32 lit = m.word(entry + 4);
    Registers r{ 1, lit, cpu.ecx, m.word(0x749a38), cpu.edx, cpu.ebx };
    IntegerFlags flags;
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    // push a+0x60, a+0x40, a+0x20, a; call [0x9ef94c]: a quad's four corners
    auto quad = [&](x86::reg32 corners) {
        for (x86::reg32 k : { 0x60u, 0x40u, 0x20u, 0u })
        {
            esp -= 4;
            m.word(esp, corners + k);
        }
        return callTo(m.word(0x9ef94c));
    };
    // the four corners' colours
    auto colour = [&](x86::reg32 corners, x86::reg32 value) {
        for (x86::reg32 k : { 0x30u, 0x50u, 0x70u, 0x10u })
            m.word(corners + k, value);
    };
    auto choose = [](bool on) { return on ? 0xffdcdb58u : 0xff797c93u; };
    // the quad of light eax: [ebp - 8], esi, edi, [ebp - 4]
    auto light = [&](x86::reg32 index, bool on) {
        r.eax = (index << 7) + 0x724790;
        colour(r.eax, choose(on));
        r.eax = (index << 7) + 0x724790;
        r.edx = r.eax + 0x20;
        return quad(r.eax);
    };
    if (!callTo(0x431900))
        return;
    for (x86::reg32 corners : { 0x724790u, 0x724810u, 0x724890u })
    {
        r.eax = corners + 0x20;
        if (!quad(corners))
            return;
    }
    setByte(r.eax, 1, m.byte(0x7a3a58));
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.edx = 1;
        r.eax = 0x2bf;
        if (!callTo(0x431900))
            return;
    }
    flags.logic(r.ebx);
    if (r.ebx == 0)
        r.edx = choose(true);
    else
        r.ecx = choose(false);
    for (x86::reg32 at : { 0x724d40u, 0x724d60u, 0x724d80u, 0x724d20u })
        m.word(at, choose(r.ebx == 0));
    r.eax = 0x724d10 + 0x20;
    if (!quad(0x724d10))
        return;
    flags.logic(r.ebx);
    if (!light(m.word(ebp - 8), r.ebx == 0))
        return;
    flags.compare(r.ebx, 2);
    if (!light(r.esi, r.ebx == 2))
        return;
    flags.compare(r.ebx, 1);
    r.esi = choose(r.ebx == 1);
    for (x86::reg32 at : { 0x724cc0u, 0x724ce0u, 0x724d00u, 0x724ca0u })
        m.word(at, r.esi);
    r.eax = 0x724c90 + 0x20;
    if (!quad(0x724c90))
        return;
    flags.compare(r.ebx, 1);
    if (!light(r.edi, r.ebx == 1))
        return;
    flags.compare(r.ebx, 3);
    if (!light(m.word(ebp - 4), r.ebx == 3))
        return;
    setByte(r.edx, 0, m.byte(0x7a3a58));
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.eax = 0x2bf;
        r.edx = 0;
        if (!callTo(0x431900))
            return;
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = r.edx;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 8;
}

/* sub_4880b0: the cockpit's needles for the car at eax: the speedometer's
 * (when [0x7922d0]) at the angle [0x7922e0] times |the car's +0x4f4| plus
 * [0x7922f4], the tachometer's (when [0x7922e8]) at the car's +0x590 times
 * [0x7922ec] and [0x53b6ac] plus [0x7922f0]; each a quad (the template at
 * 0x488030, its colour sub_4206f0's, its texture [0x7922d8], [0x7922d4])
 * from the dial's centre ([0x792040], [0x792044] or [0x792068], [0x79206c],
 * scaled to the cabin, a port: cabinRect) along the angle (sub_4ea970), drawn
 * by THRASH_drawquad under the texture state [0x7922e4] (sub_431900). */
void needlesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xdc; sub ebp, 0x82
    const x86::reg32 ebp = entry - 24 - 0x82;
    x86::reg32 esp = entry - 24 - 0xdc;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    const x86::reg32 car = r.eax;
    m.word(ebp + 0x76, car);
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdba0]
    m.store(ebp + 0x7a, M::mul(double(x86::sreg32(cabinRect(app, 2))), m.load(0x53b69c)));
    // rep movsd: the quad's four corners from the template
    for (x86::reg32 k = 0; k < 0x80; k += 4)
        m.word(ebp - 0x5a + k, m.word(0x488030 + k));
    r.ecx = 0;
    r.edi = ebp - 0x5a + 0x80;
    r.esi = 0x488030 + 0x80;
    r.edx = 1;
    r.eax = car + 0xc;
    if (!callTo(0x4206f0))
        return;
    for (x86::reg32 at : { 0x16u, 0xfffffff6u, 0xffffffd6u, 0xffffffb6u })
        m.word(ebp + at, r.eax);
    /* A needle at the angle in [ebp+0x6a]: sub_4ea970's sine and cosine to
     * [ebp+0x66], [ebp+0x6e]; the dial's centre and the needle's ends from the
     * ints at `dial` (+0, +4 the centre, +8 its length) and 0x792038 (its
     * width), [length] the tail's; the width's half kept as a double at
     * [ebp+spare]. */
    auto needle = [&](x86::reg32 dial, x86::reg32 tail, x86::reg32 spare) {
        r.ebx = ebp + 0x6e;
        r.edx = ebp + 0x66;
        r.eax = ebp + 0x6a;
        if (!callTo(0x4ea970))
            return false;
        const double D = m.loadDouble(0x53b6a4);
        const double k = m.load(ebp + 0x7a);
        const double x = M::mul(fild32(m, dial), k);
        const double y = M::mul(fild32(m, dial + 4), k);
        const double length = M::mul(fild32(m, dial + 8), k);
        const double width = M::mul(M::mul(fild32(m, 0x792038), k), D);
        const double back = M::mul(k, m.load(tail));
        // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdb98], [0x7cdb9c]
        m.store(ebp + 0x26, M::add(x, double(x86::sreg32(cabinRect(app, 0)))));
        const double headX = M::mul(length, m.load(ebp + 0x6e));
        const double headY = M::mul(length, m.load(ebp + 0x66));
        m.store(ebp + 0x2a, M::add(y, double(x86::sreg32(cabinRect(app, 1)))));
        const double tailX = M::mul(back, m.load(ebp + 0x6e));
        const double tailY = M::mul(back, m.load(ebp + 0x66));
        const double sideX = M::mul(-m.load(ebp + 0x66), width);
        const double sideY = M::mul(width, m.load(ebp + 0x6e));
        m.store(ebp + 0x32, sideX);
        const double halfX = M::mul(sideX, D);
        r.eax = m.word(0x7922d8);
        m.store(ebp + 0x36, sideY);
        const double halfY = M::mul(sideY, D);
        for (x86::reg32 at : { 0x1eu, 0xfffffffeu, 0xffffffdeu, 0xffffffbeu })
            m.word(ebp + at, r.eax);
        r.eax = m.word(0x7922d4);
        r.edx = m.word(0x7922e4);
        for (x86::reg32 at : { 0x22u, 0x2u, 0xffffffe2u, 0xffffffc2u })
            m.word(ebp + at, r.eax);
        r.eax = 1;
        m.store(ebp + 0x3e, M::add(headX, m.load(ebp + 0x26)));
        m.store(ebp + 0x42, M::add(headY, m.load(ebp + 0x2a)));
        m.store(ebp + 0x4a, M::add(tailX, m.load(ebp + 0x26)));
        m.store(ebp + 0x4e, M::add(tailY, m.load(ebp + 0x2a)));
        m.store(ebp - 0x5a, M::add(m.load(ebp + 0x3e), m.load(ebp + 0x32)));
        m.store(ebp - 0x56, M::add(m.load(ebp + 0x42), m.load(ebp + 0x36)));
        m.store(ebp - 0x3a, M::sub(m.load(ebp + 0x3e), m.load(ebp + 0x32)));
        m.store(ebp - 0x36, M::sub(m.load(ebp + 0x42), m.load(ebp + 0x36)));
        app->getMemory<double>(ebp + spare) = halfX;
        m.store(ebp + 0x32, m.loadDouble(ebp + spare));
        m.store(ebp + 0x36, halfY);
        m.store(ebp - 0x1a, M::sub(m.load(ebp + 0x4a), m.load(ebp + 0x32)));
        m.store(ebp - 0x16, M::sub(m.load(ebp + 0x4e), m.load(ebp + 0x36)));
        m.store(ebp + 6, M::add(m.load(ebp + 0x4a), m.load(ebp + 0x32)));
        m.store(ebp + 0xa, M::add(m.load(ebp + 0x4e), m.load(ebp + 0x36)));
        if (!callTo(0x431900))
            return false;
        for (x86::reg32 at : { 6u, 0xffffffe6u, 0xffffffc6u, 0xffffffa6u })
        {
            r.eax = ebp + at;
            esp -= 4;
            m.word(esp, r.eax);
        }
        return callTo(m.word(0x9ef94c));
    };
    r.edx = m.word(0x7922d0);
    flags.logic(r.edx);
    if (r.edx)
    {
        // fldz; fcomp [car+0x4f4]; fnstsw ax; sahf; jae: the speed's size
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(0.0), x86::Float(m.load(car + 0x4f4)));
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
        r.eax = m.word(ebp + 0x76);
        if (cpu.flags.cf)
        {
            r.eax = m.word(r.eax + 0x4f4);
            m.word(ebp + 0x72, r.eax);
        }
        else
            m.store(ebp + 0x72, -m.load(r.eax + 0x4f4));
        m.store(ebp + 0x6a, M::add(M::mul(m.load(0x7922e0), m.load(ebp + 0x72)), m.load(0x7922f4)));
        if (!needle(0x792040, 0x7922dc, 0x56))
            return;
    }
    flags.compare(m.word(0x7922e8), 0);
    if (m.word(0x7922e8))
    {
        r.eax = m.word(ebp + 0x76);
        m.store(ebp + 0x6a, M::add(M::mul(M::mul(fild32(m, r.eax + 0x590), m.load(0x7922ec)), m.load(0x53b6ac)),
                                   m.load(0x7922f0)));
        if (!needle(0x792068, 0x7922f8, 0x5e))
            return;
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_488470: the cockpit's bar gauges for the car at ecx (eax): how full
 * (|speed| times [0x53b6b8], [0x53b6bc] off [0x53b6c4], times the car's
 * +0x51c and [0x53b6cc]; wrapped into 0..1, reversed when [0x6fd4c4]), then
 * each of the [0x792310] bars at 0x792090 (36 bytes: its texture state's
 * record, the texture's height, s, t, x, y, width, height, upright) as a quad
 * of four vertices (colour sub_4206f0's) bent by sub_496bf0, scaled to the
 * cabin and placed in it (a port: cabinRect), clip-coded and drawn by
 * sub_4c12e0 between sub_4bed70's clip rectangles 0x7cdb88 and 0x7cdac0. */
void barGaugesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc8; sub ebp, 0x82
    const x86::reg32 ebp = entry - 24 - 0x82;
    x86::reg32 esp = entry - 24 - 0xc8;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.eax, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // fcomp; fnstsw ax; sahf
    auto sahfCompare = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
    };
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdba0], [0x7cdba4], [0x7cdb98], [0x7cdb9c]
    m.word(ebp + 0x5e, cabinRect(app, 0));
    m.store(ebp + 0x6e, M::mul(double(x86::sreg32(cabinRect(app, 2))), m.load(0x53b6b0)));
    m.store(ebp + 0x66, M::mul(double(x86::sreg32(cabinRect(app, 3))), m.load(0x53b6b4)));
    m.word(ebp + 0x62, cabinRect(app, 1));
    r.eax = cabinRect(app, 1);
    r.ebx = 0;
    flags.logic(0);
    // the four vertices: z 2^-16, 1/w 1, the colour, clip code 0
    for (;;)
    {
        r.edx = 1;
        r.eax = r.ecx + 0xc;
        if (!callTo(0x4206f0))
            return;
        r.edx = r.eax;
        r.eax = r.ebx << 5;
        r.esi = 0x3f7fff00;
        m.word(r.eax + ebp - 0x36, r.edx);
        r.edi = 0;
        flags.logic(0);
        m.word(r.eax + ebp - 0x3a, r.esi);
        m.word(r.eax + ebp - 0x32, r.edi);
        r.edx = 0x37800080;
        flags.inc(r.ebx);
        ++r.ebx;
        m.word(r.eax + ebp - 0x3e, r.edx);
        flags.compare(r.ebx, 4);
        if (x86::sreg32(r.ebx) >= 4)
            break;
    }
    // how full: the speed's size
    sahfCompare(0.0, m.load(r.ecx + 0x4f4));
    if (cpu.flags.cf)
    {
        r.eax = m.word(r.ecx + 0x4f4);
        m.word(ebp + 0x6a, r.eax);
    }
    else
        m.store(ebp + 0x6a, -m.load(r.ecx + 0x4f4));
    const double speed = M::mul(M::mul(m.load(ebp + 0x6a), m.load(0x53b6b8)), m.load(0x53b6bc));
    const double off = M::sub(m.loadDouble(0x53b6c4), speed);
    m.store(ebp + 0x72, M::mul(M::mul(fild32(m, r.ecx + 0x51c), off), m.load(0x53b6cc)));
    sahfCompare(0.0, m.load(ebp + 0x72));
    if (!cpu.flags.cf && !cpu.flags.zf)
        m.store(ebp + 0x72, M::add(1.0, m.load(ebp + 0x72)));
    flags.compare(m.word(0x6fd4c4), 0);
    if (m.word(0x6fd4c4))
        m.store(ebp + 0x72, M::sub(1.0, m.load(ebp + 0x72)));
    r.edx = 0;
    m.word(ebp + 0x76, 0);
    for (;;)
    {
        r.eax = m.word(ebp + 0x76);
        flags.compare(r.eax, m.word(0x792310));
        if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x792310)))
            break;
        r.edx = r.eax * 36;
        const x86::reg32 bar = 0x792090 + r.edx;
        r.ecx = m.word(bar + 0x10) - m.word(0x792300);
        r.esi = m.word(0x79230c);
        r.ebx = m.word(bar + 0x14) - r.esi;
        r.edi = m.word(bar + 0x20);
        flags.logic(r.edi);
        const bool upright = r.edi != 0;
        /* The corners' x and y at [ebp+0x4a..] and [ebp+0x3a..] (as floats,
         * stored and copied): the bar's rectangle, its width and height the
         * other way round when it lies on its side. */
        m.word(ebp + 0x7a, r.ecx);
        m.store(ebp + 0x56, double(x86::sreg32(r.ecx)));
        m.word(ebp + 0x4a, m.word(ebp + 0x56));
        if (upright)
        {
            r.eax = m.word(bar + 0x1c) + r.ecx;
            m.word(ebp + 0x7e, r.ebx);
            m.word(ebp + 0x7a, r.eax);
            m.store(ebp + 0x52, double(x86::sreg32(r.eax)));
            m.store(ebp + 0x46, double(x86::sreg32(r.ebx)));
            m.word(ebp + 0x4e, m.word(ebp + 0x52));
            m.word(ebp + 0x42, m.word(ebp + 0x46));
            r.eax = m.word(bar + 0x18) + r.ebx;
            m.word(ebp + 0x7a, r.eax);
            m.store(ebp + 0x3e, double(x86::sreg32(r.eax)));
            r.eax = m.word(ebp + 0x3e);
            m.word(ebp + 0x3a, r.eax);
        }
        else
        {
            r.eax = m.word(bar + 0x18) + r.ecx;
            m.word(ebp + 0x7a, r.ebx);
            m.word(ebp + 0x7e, r.eax);
            m.store(ebp + 0x52, double(x86::sreg32(r.eax)));
            m.store(ebp + 0x3e, double(x86::sreg32(r.ebx)));
            m.word(ebp + 0x4e, m.word(ebp + 0x52));
            m.word(ebp + 0x3a, m.word(ebp + 0x3e));
            r.eax = m.word(bar + 0x1c) + r.ebx;
            m.word(ebp + 0x7a, r.eax);
            m.store(ebp + 0x46, double(x86::sreg32(r.eax)));
            r.eax = m.word(ebp + 0x46);
            m.word(ebp + 0x42, r.eax);
        }
        r.ecx = 0;
        flags.logic(0);
        // each corner bent by how full (sub_496bf0), then scaled and placed in the cabin
        for (;;)
        {
            r.edx = r.ecx << 5;
            flags.add(ebp - 0x46, r.edx);
            r.eax = ebp - 0x46 + r.edx;
            r.ebx = r.eax + 4;
            push(r.ebx);
            push(r.eax);
            push(m.word(ebp + 0x72));
            push(m.word(ebp + 0x3a + r.ecx * 4));
            push(m.word(ebp + 0x4a + r.ecx * 4));
            if (!callTo(0x496bf0))
                return;
            const x86::reg32 corner = r.edx + ebp - 0x46;
            const double x = M::mul(M::add(fild32(m, 0x792304), m.load(corner)), m.load(ebp + 0x6e));
            const double y = M::mul(M::add(fild32(m, 0x792308), m.load(corner + 4)), m.load(ebp + 0x66));
            m.word(ebp + 0x7a, m.word(ebp + 0x5e));
            r.eax = m.word(ebp + 0x62);
            flags.inc(r.ecx);
            ++r.ecx;
            m.word(ebp + 0x7e, r.eax);
            m.store(corner, M::add(x, fild32(m, ebp + 0x7a)));
            m.store(corner + 4, M::add(y, fild32(m, ebp + 0x7e)));
            flags.compare(r.ecx, 4);
            if (x86::sreg32(r.ecx) >= 4)
                break;
        }
        // the texture's s and t at the corners, over its height
        r.eax = m.word(ebp + 0x76);
        r.edx = r.eax * 9;
        const x86::reg32 b = r.edx * 4 + 0x792090;
        const double inverse = M::div(1.0, fild32(m, b + 4));
        const double k0 = m.load(0x53b6d0);
        if (upright)
        {
            r.ecx = m.word(b + 0x18);
            r.eax = m.word(b + 8) + r.ecx;
            m.word(ebp + 0x7a, r.eax);
            const double s1 = M::mul(M::add(fild32(m, ebp + 0x7a), k0), inverse);
            const double s0 = M::mul(M::add(fild32(m, b + 8), m.load(0x53b6d4)), inverse);
            const double t0 = M::mul(M::add(fild32(m, b + 0xc), m.load(0x53b6d8)), inverse);
            m.store(ebp - 0xe, s1);
            m.word(ebp - 0x2e, m.word(ebp - 0xe));
            m.store(ebp + 0x32, s0);
            m.word(ebp + 0x12, m.word(ebp + 0x32));
            m.store(ebp + 0x36, t0);
            r.ebx = m.word(b + 0x1c);
            m.word(ebp - 0x2a, m.word(ebp + 0x36));
            flags.add(m.word(b + 0xc), r.ebx);
            r.eax = m.word(b + 0xc) + r.ebx;
            m.word(ebp + 0x7a, r.eax);
            m.store(ebp + 0x16, M::mul(M::add(k0, fild32(m, ebp + 0x7a)), inverse));
            r.eax = m.word(ebp + 0x16);
            m.word(ebp - 0xa, r.eax);
        }
        else
        {
            m.store(ebp + 0x32, M::mul(M::add(fild32(m, b + 8), m.load(0x53b6d8)), inverse));
            r.esi = m.word(b + 0x18);
            m.word(ebp - 0x2e, m.word(ebp + 0x32));
            r.eax = m.word(b + 8) + r.esi;
            m.word(ebp + 0x7e, r.eax);
            const double t = fild32(m, b + 0xc);
            const double s1 = M::mul(M::add(fild32(m, ebp + 0x7e), k0), inverse);
            const double t0 = M::mul(M::add(t, m.load(0x53b6d4)), inverse);
            m.store(ebp + 0x12, s1);
            m.store(ebp - 0xa, t0);
            m.word(ebp - 0xe, m.word(ebp + 0x12));
            r.edi = m.word(b + 0x1c);
            m.word(ebp - 0x2a, m.word(ebp - 0xa));
            r.eax = m.word(b + 0xc) + r.edi;
            m.word(ebp + 0x7e, r.eax);
            m.store(ebp + 0x36, M::mul(inverse, M::add(k0, fild32(m, ebp + 0x7e))));
            r.eax = m.word(ebp + 0x36);
            m.word(ebp + 0x16, r.eax);
        }
        // drawn under the bar's texture state between the cabin's clip rectangle and the screen's
        r.edx = m.word(ebp + 0x76);
        r.eax = m.word(r.edx * 36 + 0x792090);
        r.edx = m.word(r.eax + 4);
        r.eax = 1;
        r.edi = ebp - 0x32;
        if (!callTo(0x431900))
            return;
        r.eax = 0x7cdb88;
        r.esi = ebp - 0x46;
        if (!callTo(0x4bed70))
            return;
        screenCodeFlags(m, r.esi, r.edi, r, flags);
        screenCodeFlags(m, ebp - 0x26, ebp - 0x12, r, flags);
        screenCodeFlags(m, ebp - 6, ebp + 0xe, r, flags);
        screenCodeFlags(m, ebp + 0x1a, ebp + 0x2e, r, flags);
        r.ecx = ebp + 0x1a;
        r.ebx = ebp - 6;
        r.edx = ebp - 0x26;
        r.eax = ebp - 0x46;
        if (!callTo(0x4c12e0))
            return;
        r.eax = 0x7cdac0;
        if (!callTo(0x4bed70))
            return;
        flags.inc(m.word(ebp + 0x76));
        m.word(ebp + 0x76, m.word(ebp + 0x76) + 1);
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_489240: the cockpit ([0x7922fc], not [0x7a3d10], sub_423b00(0) 1, not
 * [0x79c4a0], [0x6fbc4c]): the cabin (a port: cabinRect) tiled by the
 * textures at 0x791d74 (44 bytes each: the texture state's, then s and t at
 * the four corners) in steps of its size times [0x53b76c] and [0x53b774] /
 * [0x53b77c], each tile a quad cut at the cabin's right and bottom edges, in
 * the colour sub_4206f0 gives the player's view (sub_422bd0; at night
 * sub_4b97b0's through sub_4b9550), under state 0x2bf 3 with bit 2 of
 * [0x7a3a58]; then the needles (sub_4880b0), the gear (when [0x55d6a4]: the
 * view's +0x50b picks 0x791c10's 44-byte entry, a size and the texture) at
 * [0x55d69c], [0x55d6a0] of the cabin, and the bar gauges (sub_488470). */
void cockpitNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc4; sub ebp, 0x82
    const x86::reg32 ebp = entry - 24 - 0x82;
    x86::reg32 esp = entry - 24 - 0xc4;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    // fcomp; fnstsw ax; sahf
    auto sahfCompare = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
    };
    // the quad at ebp-0x42 .. ebp+0x1e: its texture state, then THRASH_drawquad
    auto drawQuad = [&](x86::reg32 state) {
        r.edx = state;
        r.eax = 1;
        if (!callTo(0x431900))
            return false;
        for (x86::reg32 at : { 0x1eu, 0xfffffffeu, 0xffffffbeu, 0xffffffdeu })
        {
            r.eax = ebp + at;
            esp -= 4;
            m.word(esp, r.eax);
        }
        return callTo(m.word(0x9ef94c));
    };
    // its s and t at the corners, from eight ints at `from`
    auto texture = [&](x86::reg32 from) {
        const x86::reg32 to[] = { 0xfffffff6u, 0xfffffffau, 0xffffffd6u, 0xffffffdau, 0x16u, 0x1au, 0x36u, 0x3au };
        for (x86::reg32 k = 0; k < 8; ++k)
            m.word(ebp + to[k], m.word(from + 4 * k));
    };
    flags.compare(m.word(0x7922fc), 0);
    if (!m.word(0x7922fc))
        return leave();
    r.ecx = m.word(0x7a3d10);
    flags.logic(r.ecx);
    if (r.ecx)
        return leave();
    r.eax = 0;
    if (!callTo(0x423b00))
        return;
    flags.compare(r.eax, 1);
    if (r.eax != 1)
        return leave();
    flags.compare(m.word(0x79c4a0), 0);
    if (m.word(0x79c4a0))
        return leave();
    flags.compare(m.word(0x6fbc4c), 0);
    if (m.word(0x6fbc4c))
        return leave();
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdb98], [0x7cdb9c], [0x7cdba0], [0x7cdba4]
    const double tiles = m.loadDouble(0x53b76c);
    const double width = M::mul(M::mul(double(x86::sreg32(cabinRect(app, 2))), tiles), m.loadDouble(0x53b774));
    const double height = M::mul(M::mul(tiles, double(x86::sreg32(cabinRect(app, 3)))), m.loadDouble(0x53b77c));
    r.edi = cabinRect(app, 0);
    m.word(ebp + 0x4e, cabinRect(app, 2) + r.edi);
    r.edx = 1;
    r.esi = cabinRect(app, 1);
    m.word(ebp + 0x62, r.ecx);
    m.store(ebp + 0x46, width);
    m.word(ebp + 0x5e, cabinRect(app, 3) + r.esi);
    r.eax = 0;
    m.store(ebp + 0x52, height);
    if (!callTo(0x422bd0))
        return;
    r.ecx = r.eax;
    r.ebx = r.eax;
    r.eax += 0xc;
    if (!callTo(0x4206f0))
        return;
    r.edx = r.eax;
    flags.compare(m.word(0x6fd4c8), 0);
    if (m.word(0x6fd4c8))
    {
        r.eax = r.ecx;
        if (!callTo(0x4b97b0))
            return;
        r.edx = r.eax;
        r.eax = r.ecx;
        if (!callTo(0x4b9550))
            return;
    }
    // the quad's corners: colour, z 2^-16, 1/w 1, +0x14 0
    r.edx = 0x3f7fff00;
    for (x86::reg32 at : { 0xeu, 0xffffffceu, 0xffffffeeu, 0x2eu })
        m.word(ebp + at, r.eax);
    for (x86::reg32 at : { 0x2au, 0xau, 0xffffffcau, 0xffffffeau })
        m.word(ebp + at, r.edx);
    for (x86::reg32 at : { 0x26u, 0x6u, 0xffffffc6u, 0xffffffe6u })
        m.word(ebp + at, 0x37800080);
    r.ecx = 0;
    r.eax = 0;
    m.word(ebp + 0x32, 0);
    m.word(ebp + 0x12, 0);
    r.edx = 0;
    setByte(r.eax, 1, m.byte(0x7a3a58));
    m.word(ebp - 0x2e, 0);
    m.word(ebp - 0xe, 0);
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.edx = 3;
        r.eax = 0x2bf;
        if (!callTo(0x431900))
            return;
    }
    // the tiles, a column at a time
    m.word(ebp + 0x7a, r.edi);
    m.store(ebp + 0x72, fild32(m, ebp + 0x7a));
    for (;;)
    {
        r.eax = m.word(ebp + 0x4e);
        m.word(ebp + 0x7a, r.eax);
        sahfCompare(fild32(m, ebp + 0x7a), m.load(ebp + 0x72));
        if (cpu.flags.cf || cpu.flags.zf)
            break;
        m.word(ebp + 0x7a, r.esi);
        m.store(ebp + 0x76, fild32(m, ebp + 0x7a));
        for (;;)
        {
            r.eax = m.word(ebp + 0x5e);
            m.word(ebp + 0x7a, r.eax);
            const double bottom = fild32(m, ebp + 0x7a);
            m.store(ebp + 0x6a, M::add(m.load(ebp + 0x72), m.load(ebp + 0x46)));
            sahfCompare(bottom, m.load(ebp + 0x76));
            if (cpu.flags.cf || cpu.flags.zf)
                break;
            r.eax = m.word(ebp + 0x72);
            m.word(ebp + 0x1e, r.eax);
            m.word(ebp - 0x22, r.eax);
            // the right edge: the tile's or the cabin's
            r.eax = m.word(ebp + 0x4e);
            m.word(ebp + 0x7a, r.eax);
            m.store(ebp + 0x56, fild32(m, ebp + 0x7a));
            sahfCompare(m.load(ebp + 0x6a), m.load(ebp + 0x56));
            r.eax = m.word(ebp + (cpu.flags.cf ? 0x6a : 0x56));
            m.word(ebp + 0x66, r.eax);
            m.word(ebp - 2, r.eax);
            m.word(ebp - 0x42, r.eax);
            r.eax = m.word(ebp + 0x76);
            const double next = M::add(m.load(ebp + 0x76), m.load(ebp + 0x52));
            m.word(ebp - 0x3e, r.eax);
            m.word(ebp - 0x1e, r.eax);
            r.eax = m.word(ebp + 0x5e);
            m.store(ebp + 0x4a, next);
            m.word(ebp + 0x7a, r.eax);
            // the bottom edge: the tile's or the cabin's
            m.store(ebp + 0x5a, fild32(m, ebp + 0x7a));
            sahfCompare(m.load(ebp + 0x4a), m.load(ebp + 0x5a));
            r.eax = m.word(ebp + (cpu.flags.cf ? 0x4a : 0x5a));
            m.word(ebp + 0x6e, r.eax);
            r.edx = m.word(ebp + 0x62);
            m.word(ebp + 0x22, r.eax);
            m.word(ebp + 2, r.eax);
            flags.compare(r.edx * 12, r.edx);
            r.edx = r.edx * 44;
            texture(r.edx + 0x791d78);
            if (!drawQuad(m.word(r.edx + 0x791d74)))
                return;
            r.eax = m.word(ebp + 0x62);
            const double down = M::add(m.load(ebp + 0x76), m.load(ebp + 0x52));
            flags.inc(r.eax);
            ++r.eax;
            m.store(ebp + 0x76, down);
            m.word(ebp + 0x62, r.eax);
        }
        r.eax = m.word(ebp + 0x6a);
        m.word(ebp + 0x72, r.eax);
    }
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.eax = 0x2bf;
        r.edx = 0;
        if (!callTo(0x431900))
            return;
    }
    r.eax = r.ebx;
    if (!callTo(0x4880b0))
        return;
    flags.compare(m.word(0x55d6a4), 0);
    if (m.word(0x55d6a4))
    {
        // the gear
        const double across = M::mul(double(x86::sreg32(cabinRect(app, 2))), m.load(0x53b784));
        const double down = M::mul(double(x86::sreg32(cabinRect(app, 3))), m.load(0x53b788));
        const double x = M::mul(m.load(0x55d69c), across);
        const double y = M::mul(m.load(0x55d6a0), down);
        m.word(ebp + 0x7a, r.edi);
        r.edx = 0;
        m.store(ebp + 0x1e, M::add(x, fild32(m, ebp + 0x7a)));
        setByte(r.edx, 0, m.byte(r.ebx + 0x50b));
        r.eax = m.word(ebp + 0x1e);
        m.word(ebp - 0x22, r.eax);
        r.eax = r.edx;
        r.edx = r.edx * 11;
        const x86::reg32 gear = r.edx * 4 + 0x791c10;
        r.eax = app->getMemory<x86::reg16>(gear);
        m.word(ebp + 0x7a, r.eax);
        m.word(ebp + 0x7e, r.esi);
        const double right = M::add(M::mul(across, fild32(m, ebp + 0x7a)), m.load(ebp + 0x1e));
        const double top = M::add(y, fild32(m, ebp + 0x7e));
        m.store(ebp - 2, right);
        r.eax = m.word(ebp - 2);
        m.store(ebp - 0x3e, top);
        m.word(ebp - 0x42, r.eax);
        r.eax = m.word(ebp - 0x3e);
        m.word(ebp - 0x1e, r.eax);
        r.eax = app->getMemory<x86::reg16>(gear + 2);
        m.word(ebp + 0x7e, r.eax);
        m.store(ebp + 0x22, M::add(M::mul(down, fild32(m, ebp + 0x7e)), m.load(ebp - 0x3e)));
        r.eax = m.word(ebp + 0x22);
        m.word(ebp + 2, r.eax);
        texture(gear + 8);
        if (!drawQuad(m.word(gear + 4)))
            return;
    }
    r.eax = r.ebx;
    if (!callTo(0x488470))
        return;
    leave();
}

/* sub_4c0b20: a line between the vertices at eax and edx (0x20 bytes: x, y,
 * z, 1/w, ..., clip code at +0x14) by THRASH_drawline ([0x9ef928]): none when
 * both are out past the same edge; as they are when neither is out, their
 * depth 1 - 1/w (0 past 1 or below 0) times [0x560094] plus [0x560098], and
 * not at all when any coordinate is the indefinite NaN; otherwise copies of
 * them, each cut where it leaves the view by every plane its code has (near
 * [0x7d34f0] on 1/w, then x [0x7d34f4], [0x7d34ec], then y [0x7d34e0],
 * [0x7d34dc]) against the other's original, codes cleared, depths as above. */
void lineNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xac; sub ebp, 0x82
    const x86::reg32 ebp = entry - 20 - 0x82;
    x86::reg32 esp = entry - 20 - 0xac;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    auto drawLine = [&](x86::reg32 a, x86::reg32 b) {
        esp -= 4;
        m.word(esp, b);
        esp -= 4;
        m.word(esp, a);
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, m.word(0x9ef928)))
            return false;
        leave();
        return true;
    };
    // 1 - 1/w, or 0 when 1/w is negative or past 1 (as an int compare does it)
    auto nearness = [&](x86::reg32 v, x86::reg32 at, x86::reg32& zeroed) {
        flags.logic8(m.byte(v + 0xf) & 0x80);
        bool zero = m.byte(v + 0xf) & 0x80;
        if (!zero)
        {
            flags.compare(m.word(v + 0xc), 0x3f800000);
            zero = x86::sreg32(m.word(v + 0xc)) > 0x3f800000;
        }
        if (zero)
        {
            zeroed = 0;
            flags.logic(0);
            m.word(at, 0);
        }
        else
            m.store(at, M::sub(1.0, m.load(v + 0xc)));
    };
    const x86::reg32 a = r.eax, b = r.edx;
    const x86::reg8 codeA = m.byte(a + 0x14), codeB = m.byte(b + 0x14);
    setByte(r.ebx, 0, codeA | codeB);
    setByte(r.ebx, 1, codeB);
    setByte(r.ecx, 0, codeA & codeB);
    flags.logic8(codeA & codeB);
    if (codeA & codeB)
        return leave();
    flags.logic8(codeA | codeB);
    if (!(codeA | codeB))
    {
        // each depth through its 1/w, a's with ebx, b's with edi
        auto depth = [&](x86::reg32 v, x86::reg32 at, x86::reg32& zeroed) {
            nearness(v, at, zeroed);
            r.ecx = m.word(at);
            m.word(v + 8, r.ecx);
            m.store(v + 8, M::mul(m.load(0x560094), m.load(v + 8)));
            m.store(v + 8, M::add(m.load(0x560098), m.load(v + 8)));
        };
        depth(a, ebp + 0x5e, r.ebx);
        depth(b, ebp + 0x62, r.edi);
        for (x86::reg32 v : { a, b })
            for (x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
            {
                r.ecx = m.word(v + field) | 0x80000000;
                flags.compare(r.ecx, 0xffc00000);
                if (r.ecx == 0xffc00000)
                    return leave();
            }
        drawLine(a, b);
        return;
    }
    // the copies: the originals at ebp-0x2a, ebp-0xa, the clipped at ebp+0x16, ebp+0x36
    for (x86::reg32 k = 0; k < 0x20; k += 4)
        m.word(ebp - 0x2a + k, m.word(a + k));
    r.eax = 0;
    flags.logic(0);
    m.word(ebp + 0x7e, 0);
    for (x86::reg32 k = 0; k < 0x20; k += 4)
        m.word(ebp - 0xa + k, m.word(b + k));
    r.ecx = 0;
    r.edi = ebp - 0xa + 0x20;
    r.esi = b + 0x20;
    for (;;)
    {
        const x86::reg32 k = m.word(ebp + 0x7e);
        m.word(ebp + 0x7a, k << 5);
        const x86::reg32 v = ebp - 0x2a + (k << 5);
        const x86::reg32 other = ebp - 0x2a + (((k + 1) % 2) << 5);
        const x86::reg32 out = ebp + 0x16 + (k << 5);
        r.ebx = v;
        r.eax = other;
        r.edx = k << 5;
        for (x86::reg32 j = 0; j < 0x20; j += 4)
            m.word(out + j, m.word(v + j));
        r.ecx = 0;
        r.edi = out + 0x20;
        r.esi = v + 0x20;
        /* Where the line leaves through the plane `along` (0 x, 4 y, 0xc 1/w)
         * = `edge`: that, and the other two (x, y, 1/w) moved as far along. */
        auto cut = [&](x86::reg32 along, x86::reg32 first, x86::reg32 second, x86::reg32 edge) {
            const double t = M::div(M::sub(m.load(v + along), m.load(edge)), M::sub(m.load(v + along), m.load(other + along)));
            const double moveFirst = M::mul(M::sub(m.load(v + first), m.load(other + first)), t);
            const double moveSecond = M::mul(t, M::sub(m.load(v + second), m.load(other + second)));
            m.store(out + along, m.load(edge));
            m.store(out + first, M::sub(m.load(v + first), moveFirst));
            m.store(out + second, M::sub(m.load(v + second), moveSecond));
        };
        const x86::reg8 code = m.byte(v + 0x14);
        flags.logic8(code & 0x10);
        if (code & 0x10)
            cut(0xc, 0, 4, 0x7d34f0);
        flags.logic8(code & 1);
        if (code & 1)
            cut(0, 4, 0xc, 0x7d34f4);
        flags.logic8(code & 2);
        if (code & 2)
            cut(0, 4, 0xc, 0x7d34ec);
        flags.logic8(code & 8);
        if (code & 8)
            cut(4, 0, 0xc, 0x7d34e0);
        flags.logic8(code & 4);
        if (code & 4)
        {
            cut(4, 0, 0xc, 0x7d34dc);
            r.eax = k << 5;
            flags.shl(k, 5);
        }
        flags.inc(k);
        m.word(ebp + 0x7e, k + 1);
        flags.compare(k + 1, 2);
        if (x86::sreg32(k + 1) >= 2)
            break;
    }
    r.ebx = 0;
    flags.logic(0);
    setByte(r.ecx, 1, m.byte(ebp + 0x25));
    m.word(ebp + 0x4a, 0);
    m.word(ebp + 0x2a, 0);
    nearness(ebp + 0x16, ebp + 0x76, r.eax);
    r.eax = m.word(ebp + 0x76);
    m.word(ebp + 0x1e, r.eax);
    m.store(ebp + 0x1e, M::add(M::mul(m.load(ebp + 0x76), m.load(0x560094)), m.load(0x560098)));
    setByte(r.eax, 0, m.byte(ebp + 0x45));
    nearness(ebp + 0x36, ebp + 0x72, r.ecx);
    r.eax = m.word(ebp + 0x72);
    m.word(ebp + 0x3e, r.eax);
    m.store(ebp + 0x3e, M::mul(m.load(ebp + 0x72), m.load(0x560094)));
    m.store(ebp + 0x3e, M::add(m.load(ebp + 0x3e), m.load(0x560098)));
    r.eax = ebp + 0x16;
    drawLine(ebp + 0x16, ebp + 0x36);
}

/* sub_48c0e0: a meter of segments at x edx, y ebx, ecx wide and [esp+4]
 * high, as quad meshes over the 84 vertices at 0x55d6e4 (0x20 bytes each:
 * x, y, ..., colour at +0x10): a frame of nine (the index lists at 0x55e164,
 * 0x55e1f4, drawn by THRASH_drawquadmesh under texture state [0x76da48]),
 * three for its back (0x55e284), and four segments spaced by [0x53b98c],
 * [0x53b994], [0x53b998].., the first [esp+8] of them lit (0xffff0000,
 * 0xccff5050; with bit 2 of [0x7a3a58] drawn flat in that colour,
 * sub_432390, by the list at 0x55e214); [esp+0xc] the colour of two corners.
 * The x87 work follows the generated code's order (sym/ssa2native.py).
 * ret 0xc. */
void segmentMeterNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 0x1c;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    // THRASH_drawquadmesh(count, vertices, indices)
    auto mesh = [&](x86::reg32 count, x86::reg32 indices) {
        for (x86::reg32 v : { indices, 0x55d6e4u, count })
        {
            esp -= 4;
            m.word(esp, v);
        }
        return callTo(m.word(0x9ef930));
    };
    auto state = [&](x86::reg32 eax, x86::reg32 edx) {
        r.eax = eax;
        r.edx = edx;
        return callTo(0x431900);
    };
    // the frame's corners and edges
        r.esi = m.word(ebp + 0x18);
        r.eax = r.ebx;
        r.ebx = m.word(ebp + 0x14);
        m.word(ebp - 0x4, r.ecx);
        m.word(ebp - 0x8, r.eax);
        const double f1 = fild32(m, ebp - 0x4);
        r.eax = m.word(ebp + 0x10);
        m.store(ebp - 0x10, f1);
        m.word(ebp - 0x4, r.eax);
        const double f2 = m.load(0x53b98c);
        const double f3 = M::mul(f1, f2);
        const double f4 = fild32(m, ebp - 0x4);
        m.store(ebp - 0xc, f4);
        const double f5 = M::mul(f2, f4);
        const double f6 = m.load(ebp - 0xc);
        const double f7 = M::mul(f6, m.load(0x53b990));
        m.word(ebp - 0x4, r.edx);
        m.word(0x55e014, r.esi);
        m.word(0x55dff4, r.esi);
        const double f8 = fild32(m, ebp - 0x8);
        const double f9 = fild32(m, ebp - 0x4);
        m.store(ebp - 0x14, f9);
        m.store(ebp - 0x1c, f8);
        r.edx = 0;
        flags.logic(0);
        const double f10 = m.load(ebp - 0x1c);
        const double f11 = m.load(ebp - 0x14);
        const double f12 = M::add(f11, m.load(ebp - 0x10));
        r.eax = m.word(ebp - 0x14);
        m.store(0x55e024, f12);
        const double f13 = f12;
        m.word(0x55e044, r.eax);
        m.word(0x55dfe4, r.eax);
        m.store(0x55e004, f12);
        const double f14 = M::add(f10, m.load(ebp - 0xc));
        const double f15 = M::add(f3, m.load(ebp - 0x14));
        m.store(0x55e124, f13);
        r.eax = m.word(ebp - 0x1c);
        const double f16 = m.load(0x55e124);
        m.store(0x55e048, f14);
        m.store(0x55e0a4, f15);
        const double f17 = m.load(0x55e048);
        const double f18 = m.load(0x55e0a4);
        const double f19 = f18;
        m.store(0x55e104, f16);
        m.word(0x55e008, r.eax);
        m.word(0x55dfe8, r.eax);
        m.store(0x55e028, f17);
        m.store(0x55e084, f18);
        m.store(0x55e144, f19);
        const double f20 = M::add(f5, m.load(ebp - 0x1c));
        r.eax = m.word(ebp - 0x14);
        m.store(0x55e0e4, f19);
        const double f21 = f20;
        m.word(0x55e0c4, r.eax);
        m.word(0x55e064, r.eax);
        const double f22 = M::sub(f21, f7);
        const double f23 = M::add(f7, f20);
        m.store(0x55e088, f22);
        m.store(0x55e0c8, f23);
        const double f24 = m.load(0x55e088);
        const double f25 = m.load(0x55e0c8);
        const double f26 = m.load(0x55e088);
        const double f27 = m.load(0x55e0c8);
        m.store(0x55e068, f24);
        m.store(0x55e0a8, f25);
        m.store(0x55e108, f26);
        m.store(0x55e148, f27);
        r.eax = m.word(0x55e108);
        m.word(0x55e0e8, r.eax);
        m.store(0x55e128, f27);
    // the segments' left and right edges
    for (;;)
    {
        const double f28 = m.load(ebp - 0xc);
        const double f29 = M::mul(f28, m.load(0x53b98c));
        const double f30 = m.load(ebp - 0x10);
        r.eax = r.edx;
        const double f31 = M::mul(f30, m.load(0x53b994));
        r.eax <<= 5;
        flags.add(r.edx, 4);
        r.edx += 4;
        m.store(ebp - 0x18, f31);
        const double f32 = M::add(f29, m.load(ebp - 0x1c));
        const double f33 = f32;
        const double f34 = M::sub(f33, m.load(ebp - 0x18));
        const double f35 = M::add(f32, m.load(ebp - 0x18));
        m.store(r.eax + 0x55d708, f34);
        m.store(r.eax + 0x55d748, f35);
        r.ecx = m.word(r.eax + 0x55d708);
        m.word(r.eax + 0x55d6e8, r.ecx);
        m.store(r.eax + 0x55d728, f35);
        flags.compare(r.edx, 0x24);
        if (x86::sreg32(r.edx) >= 0x24)
            break;
    }
    // their tops and bottoms
        const double f36 = m.load(ebp - 0x10);
        const double f37 = M::mul(f36, m.load(0x53b98c));
        const double f38 = m.load(ebp - 0x10);
        const double f39 = M::mul(f38, m.load(0x53b99c));
        const double f40 = m.load(ebp - 0x10);
        const double f41 = M::mul(f40, m.load(0x53b9a0));
        const double f42 = m.load(ebp - 0x14);
        const double f43 = M::add(f42, m.load(ebp - 0x18));
        const double f44 = m.load(ebp - 0x10);
        const double f45 = M::mul(f44, m.load(0x53b9a4));
        m.store(0x55d744, f43);
        const double f46 = m.load(ebp - 0x10);
        const double f47 = M::mul(f46, m.load(0x53b998));
        r.eax = m.word(0x55d744);
        m.word(0x55d6e4, r.eax);
        const double f48 = m.load(ebp - 0x14);
        const double f49 = M::add(f48, m.load(ebp - 0x10));
        r.edx = 0;
        flags.logic(0);
        const double f50 = M::sub(f49, m.load(ebp - 0x18));
        const double f51 = M::add(f37, m.load(ebp - 0x14));
        m.store(ebp - 0x8, f50);
        m.store(ebp - 0x4, f47);
        const double f52 = M::add(f43, m.load(ebp - 0x4));
        const double f53 = m.load(ebp - 0x8);
        r.eax = m.word(ebp - 0x8);
        const double f54 = f51;
        m.word(0x55d7a4, r.eax);
        m.word(0x55d784, r.eax);
        const double f55 = M::sub(f54, m.load(ebp - 0x18));
        const double f56 = M::add(f51, m.load(ebp - 0x18));
        m.store(0x55d844, f55);
        m.store(0x55d824, f56);
        m.store(0x55d724, f52);
        const double f57 = m.load(0x55d844);
        const double f58 = M::sub(f53, m.load(ebp - 0x4));
        const double f59 = m.load(0x55d824);
        m.store(0x55d7c4, f58);
        m.store(0x55d7e4, f57);
        m.store(0x55d804, f59);
        const double f60 = m.load(0x55d724);
        const double f61 = m.load(0x55d7c4);
        const double f62 = m.load(0x55d7e4);
        m.store(0x55d704, f60);
        const double f63 = m.load(0x55d804);
        m.store(0x55d764, f61);
        const double f64 = m.load(0x55d7e4);
        const double f65 = m.load(0x55d804);
        m.store(ebp - 0x8, f39);
        const double f66 = M::sub(f62, m.load(ebp - 0x8));
        const double f67 = M::sub(f63, m.load(ebp - 0x8));
        const double f68 = M::add(f64, m.load(ebp - 0x8));
        const double f69 = M::add(f65, m.load(ebp - 0x8));
        m.store(0x55d8c4, f66);
        m.store(0x55d8a4, f67);
        m.store(0x55d944, f68);
        m.store(0x55d924, f69);
        const double f70 = m.load(0x55d8c4);
        const double f71 = m.load(0x55d8a4);
        const double f72 = m.load(0x55d944);
        const double f73 = m.load(0x55d924);
        m.store(0x55d864, f70);
        m.store(0x55d884, f71);
        m.store(0x55d8e4, f72);
        m.store(0x55d904, f73);
        const double f74 = m.load(0x55d864);
        const double f75 = m.load(0x55d884);
        const double f76 = m.load(0x55d8e4);
        const double f77 = m.load(0x55d904);
        const double f78 = M::sub(f74, f41);
        const double f79 = M::sub(f75, f41);
        const double f80 = M::add(f76, f41);
        const double f81 = M::add(f41, f77);
        m.store(0x55d9c4, f78);
        m.store(0x55d9a4, f79);
        m.store(0x55da44, f80);
        m.store(0x55da24, f81);
        const double f82 = m.load(0x55d9c4);
        const double f83 = m.load(0x55d9a4);
        const double f84 = m.load(0x55da44);
        const double f85 = m.load(0x55da24);
        m.store(0x55d964, f82);
        m.store(0x55d984, f83);
        m.store(0x55d9e4, f84);
        m.store(0x55da04, f85);
        const double f86 = m.load(0x55d964);
        const double f87 = m.load(0x55d984);
        const double f88 = m.load(0x55d9e4);
        const double f89 = m.load(0x55da04);
        const double f90 = M::sub(f86, f45);
        const double f91 = M::sub(f87, f45);
        const double f92 = M::add(f88, f45);
        const double f93 = M::add(f45, f89);
        m.store(0x55dac4, f90);
        m.store(0x55daa4, f91);
        m.store(0x55db44, f92);
        m.store(0x55db24, f93);
        const double f94 = m.load(0x55dac4);
        const double f95 = m.load(0x55daa4);
        const double f96 = m.load(0x55db44);
        const double f97 = m.load(0x55db24);
        m.store(0x55da64, f94);
        m.store(0x55da84, f95);
        m.store(0x55dae4, f96);
        m.store(0x55db04, f97);
    for (;;)
    {
        const double f98 = m.load(ebp - 0x10);
        r.eax = r.edx;
        const double f99 = M::mul(f98, m.load(0x53b9a8));
        r.eax <<= 5;
        flags.add(r.edx, 4);
        r.edx += 4;
        const double f100 = m.load(r.eax + 0x55d6e4);
        const double f101 = m.load(r.eax + 0x55d724);
        const double f102 = m.load(r.eax + 0x55d6e8);
        const double f103 = m.load(r.eax + 0x55d728);
        const double f104 = M::sub(f100, f99);
        const double f105 = M::add(f101, f99);
        const double f106 = M::sub(f102, f99);
        const double f107 = M::add(f99, f103);
        m.store(r.eax + 0x55dbc4, f104);
        m.store(r.eax + 0x55dba4, f105);
        m.store(r.eax + 0x55db88, f106);
        m.store(r.eax + 0x55dbc8, f107);
        const double f108 = m.load(r.eax + 0x55dbc4);
        const double f109 = m.load(r.eax + 0x55dba4);
        const double f110 = m.load(r.eax + 0x55db88);
        const double f111 = m.load(r.eax + 0x55dbc8);
        m.store(r.eax + 0x55db64, f108);
        m.store(r.eax + 0x55db84, f109);
        m.store(r.eax + 0x55db68, f110);
        m.store(r.eax + 0x55dba8, f111);
        flags.compare(r.edx, 0x24);
        if (x86::sreg32(r.edx) >= 0x24)
            break;
    }
    // the colours: the back and the frame's
    r.eax = 0;
    flags.logic(0);
    for (;;)
    {
        r.edx = r.eax;
        r.ecx = 0xbf00ff00;
        flags.shl(r.edx, 5);
        r.edx <<= 5;
        r.esi = 0x9f307f30;
        m.word(r.edx + 0x55d6f4, r.ecx);
        flags.inc(r.eax);
        ++r.eax;
        m.word(r.edx + 0x55db74, r.esi);
        flags.compare(r.eax, 0x24);
        if (x86::sreg32(r.eax) >= 0x24)
            break;
    }
    // the lit segments: the frame's edges beside each red, its own corners pink
    flags.logic(r.ebx);
    if (x86::sreg32(r.ebx) > 0)
    {
        r.edi = 0xffff0000;
        r.esi = 0xccff5050;
        struct Lit
        {
            x86::reg32 frame[8];
            x86::reg32 own[8];
        };
        static const Lit lit[4] = {
            { { 0x55d834, 0x55d814, 0x55d7f4, 0x55d854 }, { 0x55dcd4, 0x55dcb4, 0x55dc94, 0x55dc74 } },
            { { 0x55d8b4, 0x55d894, 0x55d874, 0x55d954, 0x55d934, 0x55d914, 0x55d8f4, 0x55d8d4 },
              { 0x55dd54, 0x55dd34, 0x55dd14, 0x55dcf4, 0x55ddd4, 0x55ddb4, 0x55dd94, 0x55dd74 } },
            { { 0x55d9b4, 0x55d994, 0x55d974, 0x55da54, 0x55da34, 0x55da14, 0x55d9f4, 0x55d9d4 },
              { 0x55de54, 0x55de34, 0x55de14, 0x55ddf4, 0x55ded4, 0x55deb4, 0x55de94, 0x55de74 } },
            { { 0x55dab4, 0x55da94, 0x55da74, 0x55db54, 0x55db34, 0x55db14, 0x55daf4, 0x55dad4 },
              { 0x55df54, 0x55df34, 0x55df14, 0x55def4, 0x55dfd4, 0x55dfb4, 0x55df94, 0x55df74 } },
        };
        for (x86::reg32 k = 0; k < 4; ++k)
        {
            if (k)
            {
                flags.compare(r.ebx, k);
                if (x86::sreg32(r.ebx) <= x86::sreg32(k))
                    break;
            }
            for (x86::reg32 at : lit[k].frame)
                if (at)
                    m.word(at, r.edi);
            for (x86::reg32 at : lit[k].own)
                if (at)
                    m.word(at, r.esi);
        }
    }
    if (!state(1, 0) || !mesh(3, 0x55e284))
        return;
    if (!state(1, m.word(0x76da48)) || !mesh(9, 0x55e164))
        return;
    flags.compare(r.ebx, 0xffffffff);
    if (x86::sreg32(r.ebx) > -1)
    {
        if (!state(0x68, 1) || !state(1, m.word(0x8b4844)))
            return;
        flags.logic8(m.byte(0x7a3a58) & 2);
        if (m.byte(0x7a3a58) & 2)
        {
            flags.logic(r.ebx);
            if (r.ebx)
            {
                r.eax = 0xccff5050;
                if (!callTo(0x432390))
                    return;
                flags.dec(r.ebx + r.ebx);
                r.eax = r.ebx + r.ebx - 1;
                if (!mesh(r.eax, 0x55e214))
                    return;
            }
        }
        else if (!mesh(9, 0x55e1f4))
            return;
        if (!state(0x68, 0))
            return;
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = r.edx;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 16;
}

/* sub_4800a0: the dark band behind player eax's HUD in split screen (with
 * bit 4 of the player's view, sub_422bd0; otherwise sub_4808b0 takes it
 * away): from the top of its half down past every element on the screen
 * (sub_47bc20; 0..9 and the racer's or the cop's 10..22) that reaches into it
 * within a gap (4:3, a port) of twice the border plus [0x7cdb64]; cut into
 * rectangles around element 3, element 1 (when their layouts are kinds 2, 3
 * and 4, 2, or [0x559264]) and the rear-view mirror (sub_422aa0, its pixel
 * rectangle at [0x791a90], truncated by sub_4dfd56), none at all when two of
 * them overlap.  A new count of rectangles at 0x700240 rebuilds them
 * (sub_4808b0, sub_49bd30 from [0x791a88]); each is a quad at [0x791ac0]
 * (0x380 bytes a player) shaded down the band by [0x53b508], [0x53b50c]. */
void hudBandNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xf0; sub ebp, 0x82
    const x86::reg32 ebp = entry - 24 - 0x82;
    x86::reg32 esp = entry - 24 - 0xf0;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.eax };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    // no band: sub_4808b0 for the player
    auto none = [&]() {
        r.eax = r.edi;
        if (callTo(0x4808b0))
            leave();
    };
    // fcomp; fnstsw ax; sahf
    auto sahfCompare = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
    };
    auto lt = [](x86::reg32 a, x86::reg32 b) { return x86::sreg32(a) < x86::sreg32(b); };
    // the cuts: x0, x1, y0, y1 of rectangle k
    auto X0 = [&](x86::reg32 k4) { return k4 + ebp - 0x6e; };
    auto X1 = [&](x86::reg32 k4) { return k4 + ebp - 0x36; };
    auto Y0 = [&](x86::reg32 k4) { return k4 + ebp - 0x52; };
    auto Y1 = [&](x86::reg32 k4) { return k4 + ebp - 0x1a; };
    const x86::reg32 player = r.edi;
    m.word(ebp + 0x3e, m.word(0x791ac0) + player * 0x380);
    r.edx = m.word(0x6fd3b0);
    flags.compare(r.edx, 1);
    r.eax = r.edx == 1 ? 1 : 0;
    r.edx = player + r.eax;
    r.eax = player;
    if (!callTo(0x422bd0))
        return;
    m.word(ebp + 0x56, r.edx);
    setByte(r.edx, 0, m.byte(r.eax + 0x200));
    m.word(ebp + 0x12, r.eax);
    flags.logic8(m.byte(r.eax + 0x200) & 0x20);
    r.eax = (m.byte(r.eax + 0x200) & 0x20) ? 1 : 0;
    m.word(ebp + 0x5a, r.eax);
    r.eax = r.edi * 0x170 + 0x749758;
    r.ebx = r.eax + 0x30;  // element 3
    r.ecx = r.eax + 0x10;  // element 1
    r.eax = m.word(ebp + 0x12);
    setByte(r.edx, 1, m.byte(r.eax + 0x200));
    r.esi = 1;
    flags.logic8(m.byte(r.eax + 0x200) & 4);
    if (!(m.byte(r.eax + 0x200) & 4))
        return none();
    // the band's top: the screen's or its lower half's
    flags.compare(m.word(ebp + 0x56), 2);
    if (m.word(ebp + 0x56) == 2)
    {
        r.eax = m.word(0x7cdacc);
        r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
        r.eax -= r.edx;
        flags.sar(r.eax, 1);
        flags.of = false;
        r.eax = x86::reg32(x86::sreg32(r.eax) >> 1);
    }
    else
        r.eax = 0;
    m.word(ebp + 0x76, r.eax);
    r.edx = 0;
    flags.logic(0);
    m.word(ebp + 0x72, r.eax);
    m.word(ebp + 0x6e, 0);
    // its bottom: past every element reaching into it
    for (;;)
    {
        r.eax = m.word(ebp + 0x6e);
        if (!callTo(0x47bc20))
            return;
        flags.logic(r.eax);
        if (r.eax)
        {
            r.edx = m.word(ebp + 0x6e);
            bool measure;
            flags.compare(r.edx, 10);
            if (lt(r.edx, 10))
                measure = true;
            else
            {
                bool cop = false;
                flags.compare(m.word(ebp + 0x5a), 0);
                if (!m.word(ebp + 0x5a))
                {
                    flags.compare(r.edx, 10);
                    flags.compare(r.edx, 0x12);
                    if (lt(r.edx, 0x12))
                        cop = true;
                }
                if (cop)
                    measure = true;
                else
                {
                    flags.compare(m.word(ebp + 0x5a), 0);
                    if (!m.word(ebp + 0x5a))
                        measure = false;
                    else
                    {
                        r.eax = m.word(ebp + 0x6e);
                        flags.compare(r.eax, 10);
                        measure = true;
                        if (!lt(r.eax, 10))
                        {
                            flags.compare(r.eax, 0x12);
                            measure = !lt(r.eax, 0x12);
                        }
                    }
                }
            }
            if (measure)
            {
                r.eax = r.edi * 0x170;
                r.edx = (m.word(ebp + 0x6e) << 4) + r.eax;
                r.eax = m.word(r.edx + 0x749764);
                flags.compare(r.eax, m.word(ebp + 0x76));
                if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(ebp + 0x76))))
                {
                    const x86::reg32 i = hudBorder(app, cpu, 0x53b500, 0x53b504);
                    r.eax = m.word(ebp + 0x72) + m.word(0x7cdb64);
                    m.word(ebp + 0x7e, r.eax);
                    m.word(ebp + 0x7a, i + i);
                    r.eax = m.word(ebp + 0x7e) + m.word(ebp + 0x7a);
                    m.word(ebp + 0x7a, r.eax);
                    r.eax = m.word(r.edx + 0x749764);
                    flags.compare(r.eax, m.word(ebp + 0x7a));
                    if (x86::sreg32(r.eax) <= x86::sreg32(m.word(ebp + 0x7a)))
                        m.word(ebp + 0x76, r.eax);
                }
            }
        }
        flags.inc(m.word(ebp + 0x6e));
        m.word(ebp + 0x6e, m.word(ebp + 0x6e) + 1);
        flags.compare(m.word(ebp + 0x6e), 0x17);
        if (!lt(m.word(ebp + 0x6e), 0x17))
            break;
    }
    r.eax = m.word(ebp + 0x76);
    flags.compare(r.eax, m.word(ebp + 0x72));
    if (r.eax == m.word(ebp + 0x72))
        return none();
    // element 3 in the band (its layout kind 2 or 3, or [0x559264])
    const x86::reg32 kinds = m.word(ebp + 0x56) * 0x348 + m.word(ebp + 0x5a) * 0x1a4;
    r.eax = kinds;
    r.edx = m.word(r.eax + 0x6fbc6c);
    bool counts;
    flags.compare(r.edx, 2);
    counts = r.edx == 2;
    if (!counts)
    {
        flags.compare(r.edx, 3);
        counts = r.edx == 3;
    }
    if (!counts)
    {
        flags.compare(m.word(0x559264), 0);
        counts = m.word(0x559264) != 0;
    }
    bool three = false;
    if (counts)
    {
        r.edx = m.word(ebp + 0x72);
        flags.compare(r.edx, m.word(r.ebx + 0xc));
        if (lt(r.edx, m.word(r.ebx + 0xc)))
        {
            r.edx = m.word(ebp + 0x76);
            flags.compare(r.edx, m.word(r.ebx + 4));
            three = !(x86::sreg32(r.edx) <= x86::sreg32(m.word(r.ebx + 4)));
        }
    }
    r.edx = three ? 1 : 0;
    // element 1 in the band (kind 4 or 2, or [0x559264])
    m.word(ebp + 0x1a, m.word(ebp + 0x56) * 0x348);
    m.word(ebp + 0x16, m.word(ebp + 0x5a) * 0x1a4);
    r.eax = m.word(ebp + 0x1a) + m.word(ebp + 0x16);
    flags.compare(m.word(r.eax + 0x6fbc68), 4);
    counts = m.word(r.eax + 0x6fbc68) == 4;
    if (!counts)
    {
        flags.compare(m.word(r.eax + 0x6fbc68), 2);
        counts = m.word(r.eax + 0x6fbc68) == 2;
    }
    if (!counts)
    {
        flags.compare(m.word(0x559264), 0);
        counts = m.word(0x559264) != 0;
    }
    bool one = false;
    if (counts)
    {
        r.eax = m.word(r.ecx + 0xc);
        flags.compare(r.eax, m.word(ebp + 0x72));
        if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(ebp + 0x72))))
        {
            r.eax = m.word(r.ecx + 4);
            flags.compare(r.eax, m.word(ebp + 0x76));
            one = lt(r.eax, m.word(ebp + 0x76));
        }
    }
    r.eax = one ? 1 : 0;
    m.word(ebp + 0x52, r.eax);
    // the mirror in the band (the top view, its layout's +0x6fbc5c or [0x559264], sub_422aa0)
    bool mirror = false;
    flags.compare(m.word(ebp + 0x56), 0);
    if (!m.word(ebp + 0x56))
    {
        r.eax = m.word(ebp + 0x5a) * 0x1a4;
        flags.compare(m.word(r.eax + 0x6fbc5c), 0);
        counts = m.word(r.eax + 0x6fbc5c) != 0;
        if (!counts)
        {
            flags.compare(m.word(0x559264), 0);
            counts = m.word(0x559264) != 0;
        }
        if (counts)
        {
            r.eax = 0;
            if (!callTo(0x422aa0))
                return;
            flags.logic(r.eax);
            if (r.eax)
            {
                r.eax = m.word(0x791a90);
                m.word(ebp + 0x26, r.eax);
                r.eax = m.word(ebp + 0x72);
                m.word(ebp + 0x7a, r.eax);
                r.eax = m.word(ebp + 0x26);
                sahfCompare(fild32(m, ebp + 0x7a), m.load(m.word(ebp + 0x26) + 0x244));
                if (cpu.flags.cf)
                {
                    r.eax = m.word(ebp + 0x76);
                    m.word(ebp + 0x7a, r.eax);
                    r.eax = m.word(ebp + 0x26);
                    sahfCompare(fild32(m, ebp + 0x7a), m.load(m.word(ebp + 0x26) + 4));
                    mirror = !cpu.flags.cf && !cpu.flags.zf;
                }
            }
        }
    }
    r.eax = mirror ? 1 : 0;
    m.word(ebp + 0x4e, r.eax);
    // which of them overlap across: 3 and 1, 3 and the mirror, 1 and the mirror
    auto across = [&](x86::reg32 a0, x86::reg32 a1, x86::reg32 b0, x86::reg32 b1) {
        flags.compare(a0, b0);
        if (!(x86::sreg32(a0) <= x86::sreg32(b0)))
        {
            flags.compare(a0, b1);
            if (lt(a0, b1))
                return true;
        }
        flags.compare(a1, b0);
        if (x86::sreg32(a1) <= x86::sreg32(b0))
            return false;
        flags.compare(a1, b1);
        return lt(a1, b1);
    };
    r.eax = m.word(r.ebx);
    const bool threeOne = across(m.word(r.ebx), m.word(r.ebx + 8), m.word(r.ecx), m.word(r.ecx + 8));
    r.eax = m.word(r.ebx + 8);
    if (!threeOne)
        flags.logic(0);
    r.eax = threeOne ? 1 : 0;
    // against the mirror's floats: the left edge inside, then the right
    auto acrossMirror = [&](x86::reg32 element, x86::reg32 left, x86::reg32 right, x86::reg32 keepLeft,
                            x86::reg32 keepRight, bool flagZero) {
        m.store(ebp + left, fild32(m, element));
        r.eax = m.word(0x791a90);
        m.word(ebp + keepLeft, r.eax);
        sahfCompare(m.load(ebp + left), m.load(r.eax));
        if (!cpu.flags.cf && !cpu.flags.zf)
        {
            r.eax = m.word(ebp + keepLeft);
            sahfCompare(m.load(ebp + left), m.load(r.eax + 0x140));
            if (cpu.flags.cf)
                return true;
        }
        const double edge = fild32(m, element + 8);
        r.eax = m.word(0x791a90);
        m.store(ebp + right, edge);
        m.word(ebp + keepRight, r.eax);
        sahfCompare(edge, m.load(r.eax));
        if (!cpu.flags.cf && !cpu.flags.zf)
        {
            r.eax = m.word(ebp + keepRight);
            sahfCompare(m.load(ebp + right), m.load(r.eax + 0x140));
            if (cpu.flags.cf)
                return true;
        }
        if (flagZero)
            flags.logic(0);
        return false;
    };
    {
        const x86::reg32 overlap = r.eax;
        const bool threeMirror = acrossMirror(r.ebx, 0x2a, 0x2e, 0x22, 0x46, true);
        m.word(ebp + 0xa, overlap);
        r.eax = threeMirror ? 1 : 0;
    }
    {
        const x86::reg32 overlap = r.eax;
        const bool oneMirror = acrossMirror(r.ecx, 0x36, 0x3a, 0x32, 0x42, false);
        m.word(ebp + 0xe, overlap);
        r.eax = oneMirror ? 1 : 0;
    }
    flags.logic(r.edx);
    if (r.edx)
    {
        flags.compare(m.word(ebp + 0x52), 0);
        if (m.word(ebp + 0x52))
        {
            flags.compare(m.word(ebp + 0xa), 0);
            if (m.word(ebp + 0xa))
                return none();
        }
    }
    flags.logic(r.edx);
    if (r.edx)
    {
        flags.compare(m.word(ebp + 0x4e), 0);
        if (m.word(ebp + 0x4e))
        {
            flags.compare(m.word(ebp + 0xe), 0);
            if (m.word(ebp + 0xe))
                return none();
        }
    }
    flags.compare(m.word(ebp + 0x52), 0);
    if (m.word(ebp + 0x52))
    {
        flags.compare(m.word(ebp + 0x4e), 0);
        if (m.word(ebp + 0x4e))
        {
            flags.logic(r.eax);
            if (r.eax)
                return none();
        }
    }
    // the band whole, then cut around element 3
    r.eax = 0;
    m.word(X0(0), 0);
    m.word(X1(0), m.word(0x7cdac8));
    m.word(Y0(0), m.word(ebp + 0x72));
    r.eax = m.word(ebp + 0x76);
    m.word(Y1(0), r.eax);
    flags.logic(r.edx);
    if (r.edx)
    {
        bool split = false;
        flags.compare(m.word(r.ebx), 0);
        if (!(x86::sreg32(m.word(r.ebx)) <= 0))
        {
            r.edx = m.word(X1(0));
            flags.compare(r.edx, m.word(r.ebx + 8));
            if (!(x86::sreg32(r.edx) <= x86::sreg32(m.word(r.ebx + 8))))
            {
                // across the band's middle: left and right of it
                m.word(X1(4), r.edx);
                r.eax = m.word(r.ebx);
                m.word(X1(0), r.eax);
                r.eax = m.word(r.ebx + 8);
                m.word(X0(4), r.eax);
                r.eax = m.word(ebp + 0x72);
                m.word(Y0(4), r.eax);
                r.eax = m.word(ebp + 0x76);
                r.esi = 2;
                m.word(Y1(4), r.eax);
                split = true;
            }
        }
        if (!split)
        {
            r.edx = m.word(0x7cdac8);
            flags.compare(r.edx, m.word(r.ebx + 8));
            if (r.edx == m.word(r.ebx + 8))
            {
                r.eax = m.word(r.ebx);
                m.word(X1(0), r.eax);
            }
            else
            {
                flags.compare(m.word(r.ebx), 0);
                if (!m.word(r.ebx))
                {
                    r.eax = m.word(r.ebx + 8);
                    m.word(X0(0), r.eax);
                }
            }
        }
        // below or above it, the rest of the band beside it
        r.edx = m.word(ebp + 0x76);
        flags.compare(r.edx, m.word(r.ebx + 4));
        if (!(x86::sreg32(r.edx) <= x86::sreg32(m.word(r.ebx + 4))))
        {
            r.edx = m.word(r.ebx + 4);
            r.eax = r.esi * 4;
            flags.compare(r.edx, m.word(ebp + 0x72));
            bool add = true;
            if (!(x86::sreg32(r.edx) <= x86::sreg32(m.word(ebp + 0x72))))
            {
                m.word(X0(r.eax), m.word(r.ebx));
                m.word(X1(r.eax), m.word(r.ebx + 8));
                m.word(Y0(r.eax), m.word(ebp + 0x72));
                r.edx = m.word(r.ebx + 4);
            }
            else
            {
                r.edx = m.word(r.ebx + 0xc);
                flags.compare(r.edx, m.word(ebp + 0x76));
                if (!lt(r.edx, m.word(ebp + 0x76)))
                    add = false;
                else
                {
                    m.word(X0(r.eax), m.word(r.ebx));
                    m.word(X1(r.eax), m.word(r.ebx + 8));
                    m.word(Y0(r.eax), m.word(r.ebx + 0xc));
                    r.edx = m.word(ebp + 0x76);
                }
            }
            if (add)
            {
                ++r.esi;
                m.word(Y1(r.eax), r.edx);
            }
        }
    }
    /* Cut every rectangle so far around [x0, x1] (element 1's or the
     * mirror's), the counter at `counter` (a local or ebx), then the band
     * below or above it: as the generated code does it for each. */
    flags.compare(m.word(ebp + 0x52), 0);
    if (m.word(ebp + 0x52))
    {
        r.eax = 0;
        m.word(ebp + 0x62, 0);
        for (;;)
        {
            r.edx = m.word(ebp + 0x62);
            r.eax = r.esi * 4;
            flags.compare(r.esi, r.edx);
            if (x86::sreg32(r.esi) <= x86::sreg32(r.edx))
                break;
            r.ebx = m.word(r.ecx);
            r.edx <<= 2;
            flags.compare(r.ebx, m.word(X0(r.edx)));
            if (!(x86::sreg32(r.ebx) <= x86::sreg32(m.word(X0(r.edx)))))
            {
                r.ebx = m.word(r.ecx + 8);
                flags.compare(r.ebx, m.word(X1(r.edx)));
                if (lt(r.ebx, m.word(X1(r.edx))))
                {
                    // inside it: split in two
                    r.ebx = m.word(X1(r.edx));
                    m.word(X1(r.eax), r.ebx);
                    r.ebx = m.word(r.ecx + 8);
                    m.word(X0(r.eax), r.ebx);
                    r.ebx = m.word(ebp + 0x72);
                    m.word(Y0(r.eax), r.ebx);
                    r.ebx = m.word(ebp + 0x76);
                    m.word(Y1(r.eax), r.ebx);
                    r.eax = m.word(r.ecx);
                    flags.inc(r.esi);
                    ++r.esi;
                    m.word(X1(r.edx), r.eax);
                    flags.inc(m.word(ebp + 0x62));
                    m.word(ebp + 0x62, m.word(ebp + 0x62) + 1);
                    continue;
                }
            }
            r.eax = m.word(ebp + 0x62) << 2;
            r.edx = m.word(r.ecx);
            flags.compare(r.edx, m.word(X0(r.eax)));
            if (!(x86::sreg32(r.edx) <= x86::sreg32(m.word(X0(r.eax)))))
            {
                r.ebx = m.word(X1(r.eax));
                flags.compare(r.edx, r.ebx);
                if (lt(r.edx, r.ebx))
                {
                    flags.compare(r.ebx, m.word(r.ecx + 8));
                    if (x86::sreg32(r.ebx) <= x86::sreg32(m.word(r.ecx + 8)))
                    {
                        // its right end: cut short
                        r.edx = m.word(r.ecx);
                        m.word(X1(r.eax), r.edx);
                        flags.inc(m.word(ebp + 0x62));
                        m.word(ebp + 0x62, m.word(ebp + 0x62) + 1);
                        continue;
                    }
                }
            }
            r.eax = m.word(ebp + 0x62) << 2;
            r.edx = m.word(r.ecx + 8);
            flags.compare(r.edx, m.word(X0(r.eax)));
            if (!(x86::sreg32(r.edx) <= x86::sreg32(m.word(X0(r.eax)))))
            {
                flags.compare(r.edx, m.word(X1(r.eax)));
                if (!lt(r.edx, m.word(X1(r.eax))))
                {
                    flags.inc(m.word(ebp + 0x62));
                    m.word(ebp + 0x62, m.word(ebp + 0x62) + 1);
                    continue;
                }
                r.ebx = m.word(X0(r.eax));
                flags.compare(r.ebx, m.word(r.ecx));
                if (lt(r.ebx, m.word(r.ecx)))
                {
                    flags.inc(m.word(ebp + 0x62));
                    m.word(ebp + 0x62, m.word(ebp + 0x62) + 1);
                    continue;
                }
                // its left end: cut short
                r.edx = m.word(r.ecx + 8);
                m.word(X0(r.eax), r.edx);
            }
            flags.inc(m.word(ebp + 0x62));
            m.word(ebp + 0x62, m.word(ebp + 0x62) + 1);
        }
        r.ebx = m.word(ebp + 0x76);
        r.edx = m.word(r.ecx + 4);
        flags.compare(r.edx, r.ebx);
        if (lt(r.edx, r.ebx))
        {
            r.ebx = m.word(ebp + 0x72);
            flags.compare(r.edx, r.ebx);
            if (!(x86::sreg32(r.edx) <= x86::sreg32(r.ebx)))
            {
                m.word(X0(r.eax), m.word(r.ecx));
                m.word(X1(r.eax), m.word(r.ecx + 8));
                m.word(Y0(r.eax), r.ebx);
                r.edx = m.word(r.ecx + 4);
                flags.inc(r.esi);
                ++r.esi;
                m.word(Y1(r.eax), r.edx);
            }
            else
            {
                r.ebx = m.word(ebp + 0x76);
                flags.compare(r.ebx, m.word(r.ecx + 0xc));
                if (!(x86::sreg32(r.ebx) <= x86::sreg32(m.word(r.ecx + 0xc))))
                {
                    m.word(X0(r.eax), m.word(r.ecx));
                    m.word(X1(r.eax), m.word(r.ecx + 8));
                    r.edx = m.word(r.ecx + 0xc);
                    ++r.esi;
                    m.word(Y0(r.eax), r.edx);
                    m.word(Y1(r.eax), r.ebx);
                }
            }
        }
    }
    flags.compare(m.word(ebp + 0x4e), 0);
    if (m.word(ebp + 0x4e))
    {
        // the mirror's pixel rectangle: x0, x1, y0, y1 through sub_4dfd56
        r.eax = m.word(0x791a90);
        r.ebx = 0;
        cpu.fpu.count += 4;
        cpu.fpu.st(3) = x86::Float(m.load(r.eax));
        cpu.fpu.st(2) = x86::Float(m.load(r.eax + 0x140));
        cpu.fpu.st(1) = x86::Float(m.load(r.eax + 4));
        cpu.fpu.st(0) = x86::Float(m.load(r.eax + 0x244));
        auto fxch = [&](int i) {
            const x86::Float top = cpu.fpu.st(0);
            cpu.fpu.st(0) = cpu.fpu.st(i);
            cpu.fpu.st(i) = top;
        };
        for (int i : { 3, 2, 1, 3 })
        {
            fxch(i);
            if (!callTo(0x4dfd56))
                return;
        }
        fxch(2);
        auto fistp = [&](x86::reg32 at) {
            m.word(at, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
            cpu.fpu.count -= 1;
        };
        fistp(ebp + 0x66);
        fistp(ebp + 0x6a);
        fxch(1);
        fistp(ebp + 0x4a);
        fistp(ebp + 0x1e);
        const x86::reg32 x0 = ebp + 0x66, x1 = ebp + 0x6a;
        for (;;)
        {
            r.ecx = r.esi * 4;
            flags.compare(r.ebx, r.esi);
            if (!lt(r.ebx, r.esi))
                break;
            r.edx = r.ebx * 4;
            r.eax = m.word(x0);
            flags.compare(r.eax, m.word(X0(r.edx)));
            if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(X0(r.edx)))))
            {
                r.eax = m.word(x1);
                flags.compare(r.eax, m.word(X1(r.edx)));
                if (lt(r.eax, m.word(X1(r.edx))))
                {
                    m.word(X1(r.ecx), m.word(X1(r.edx)));
                    m.word(X0(r.ecx), m.word(x1));
                    m.word(Y0(r.ecx), m.word(ebp + 0x72));
                    m.word(Y1(r.ecx), m.word(ebp + 0x76));
                    r.eax = m.word(x0);
                    flags.inc(r.esi);
                    ++r.esi;
                    m.word(X1(r.edx), r.eax);
                    flags.inc(r.ebx);
                    ++r.ebx;
                    continue;
                }
            }
            r.edx = r.ebx * 4;
            r.eax = m.word(x0);
            flags.compare(r.eax, m.word(X0(r.edx)));
            if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(X0(r.edx)))))
            {
                r.ecx = m.word(X1(r.edx));
                flags.compare(r.eax, r.ecx);
                if (lt(r.eax, r.ecx))
                {
                    flags.compare(r.ecx, m.word(x1));
                    if (x86::sreg32(r.ecx) <= x86::sreg32(m.word(x1)))
                    {
                        r.eax = m.word(x0);
                        m.word(X1(r.edx), r.eax);
                        flags.inc(r.ebx);
                        ++r.ebx;
                        continue;
                    }
                }
            }
            r.edx = r.ebx * 4;
            r.eax = m.word(x1);
            flags.compare(r.eax, m.word(X0(r.edx)));
            if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(X0(r.edx)))))
            {
                flags.compare(r.eax, m.word(X1(r.edx)));
                if (!lt(r.eax, m.word(X1(r.edx))))
                {
                    flags.inc(r.ebx);
                    ++r.ebx;
                    continue;
                }
                r.eax = m.word(x0);
                flags.compare(r.eax, m.word(X0(r.edx)));
                if (!(x86::sreg32(r.eax) <= x86::sreg32(m.word(X0(r.edx)))))
                {
                    flags.inc(r.ebx);
                    ++r.ebx;
                    continue;
                }
                r.eax = m.word(x1);
                m.word(X0(r.edx), r.eax);
            }
            flags.inc(r.ebx);
            ++r.ebx;
        }
        r.eax = m.word(ebp + 0x4a);
        r.ebx = m.word(ebp + 0x76);
        flags.compare(r.eax, r.ebx);
        if (lt(r.eax, r.ebx))
        {
            r.edx = m.word(ebp + 0x72);
            flags.compare(r.eax, r.edx);
            if (!(x86::sreg32(r.eax) <= x86::sreg32(r.edx)))
            {
                m.word(X0(r.ecx), m.word(x0));
                r.eax = m.word(x1);
                flags.inc(r.esi);
                ++r.esi;
                m.word(X1(r.ecx), r.eax);
                r.eax = m.word(ebp + 0x4a);
                m.word(Y0(r.ecx), r.edx);
                m.word(Y1(r.ecx), r.eax);
            }
            else
            {
                flags.compare(r.ebx, m.word(ebp + 0x1e));
                if (!(x86::sreg32(r.ebx) <= x86::sreg32(m.word(ebp + 0x1e))))
                {
                    m.word(X0(r.ecx), m.word(x0));
                    r.eax = m.word(x1);
                    ++r.esi;
                    m.word(X1(r.ecx), r.eax);
                    r.eax = m.word(ebp + 0x1e);
                    m.word(Y1(r.ecx), r.ebx);
                    m.word(Y0(r.ecx), r.eax);
                }
            }
        }
    }
    // a new count: the quads rebuilt
    r.edx = r.edi * 4;
    flags.compare(r.esi, m.word(r.edx + 0x700240));
    if (r.esi != m.word(r.edx + 0x700240))
    {
        r.eax = r.edi;
        if (!callTo(0x4808b0))
            return;
        r.eax = r.edi * 4;
        m.word(r.edx + 0x700240, r.esi);
        r.edx = r.eax;
        r.eax = r.eax * 7;
        r.edx = m.word(0x791a88) + r.eax;
        m.word(ebp + 0x5e, r.edx);
        for (;;)
        {
            --r.esi;
            flags.compare(r.esi, 0xffffffff);
            if (r.esi == 0xffffffff)
                break;
            const x86::reg32 at = m.word(ebp + 0x5e);
            r.eax = at + 3;
            r.ecx = at + 2;
            r.ebx = at + 1;
            r.edx = at;
            esp -= 4;
            m.word(esp, r.eax);
            r.eax = 0x749a38;
            if (!callTo(0x49bd30))
                return;
            flags.add(m.word(ebp + 0x5e), 4);
            m.word(ebp + 0x5e, m.word(ebp + 0x5e) + 4);
        }
    }
    // the quads: corners from the cuts, shaded down the band
    for (r.ecx = 0;; )
    {
        flags.compare(r.ecx, m.word(r.edi * 4 + 0x700240));
        if (!lt(r.ecx, m.word(r.edi * 4 + 0x700240)))
            break;
        r.ebx = r.ecx * 4;
        r.esi = m.word(ebp + 0x3e);
        r.eax = (r.ecx << 7) + r.esi;
        const double top = fild32(m, Y0(r.ebx));
        m.store(r.eax + 0x24, top);
        m.store(r.eax + 4, top);
        const double bottom = fild32(m, Y1(r.ebx));
        m.store(r.eax + 0x64, bottom);
        m.store(r.eax + 0x44, bottom);
        const double left = fild32(m, X0(r.ebx));
        m.store(r.eax + 0x60, left);
        m.store(r.eax, left);
        const double right = fild32(m, X1(r.ebx));
        m.word(r.eax + 0x78, 0x3f7b0000);
        m.word(r.eax + 0x58, 0x3f7b8000);
        r.esi = m.word(ebp + 0x72);
        m.store(r.eax + 0x40, right);
        const double s0 = m.load(r.eax + 0x78);
        const double s1 = m.load(r.eax + 0x58);
        m.store(r.eax + 0x20, right);
        m.store(r.eax + 0x18, s0);
        m.store(r.eax + 0x38, s1);
        bool whole = false;
        flags.compare(r.esi, m.word(Y0(r.ebx)));
        if (r.esi == m.word(Y0(r.ebx)))
        {
            r.edx = m.word(Y1(r.ebx));
            flags.compare(r.edx, m.word(ebp + 0x76));
            whole = r.edx == m.word(ebp + 0x76);
        }
        if (whole)
        {
            m.word(r.eax + 0x3c, 0x3f440000);
            m.word(r.eax + 0x7c, 0x3f630000);
            const double t1 = m.load(r.eax + 0x7c);
            r.edx = m.word(r.eax + 0x3c);
            m.word(r.eax + 0x1c, r.edx);
            m.store(r.eax + 0x5c, t1);
        }
        else
        {
            r.eax = m.word(ebp + 0x76);
            m.word(ebp + 0x7a, r.eax);
            r.eax = m.word(ebp + 0x72);
            m.word(ebp + 0x7e, r.eax);
            const double bandBottom = fild32(m, ebp + 0x7a);
            const double span = M::sub(bandBottom, fild32(m, ebp + 0x7e));
            const double y0 = fild32(m, ebp - 0x52 + r.ecx * 4);
            const double step = M::div(m.load(0x53b508), span);
            const double down = M::mul(y0, step);
            const double past = M::mul(bandBottom, step);
            r.eax = r.ecx << 7;
            r.esi = m.word(ebp + 0x3e);
            const double base = M::sub(m.load(0x53b50c), past);
            const double t0 = M::add(down, base);
            flags.add(r.eax, r.esi);
            r.eax += r.esi;
            m.store(r.eax + 0x3c, t0);
            m.store(r.eax + 0x1c, t0);
            const double t1 = M::add(base, M::mul(step, fild32(m, ebp - 0x1a + r.ecx * 4)));
            m.store(r.eax + 0x7c, t1);
            m.store(r.eax + 0x5c, t1);
        }
        flags.inc(r.ecx);
        ++r.ecx;
    }
    leave();
}

/* sub_482b00: the HUD of the player in the record at eax (+4 the player): the
 * frame's whole sequence -- sub_4beb80's clip rectangle, the cockpit, the
 * wrong-way warning, the standings or the speeders' table, the map, the band
 * behind the HUD in split screen, the lap and race clocks, the position, the
 * laps, the gauges by the layout's kinds (the jump table at 0x482ae8), the
 * speedometer and gear, the pursuit's messages, the rear-view mirror's frame
 * -- each as the generated code calls it, with its registers and arguments;
 * nothing at all while the player's car is in the pit (+0x9a4) or before
 * frame 0x140 or for a driver of kind 0x10.  The registers every call is
 * handed are the ones the generated code leaves, stale ones too. */
void hudNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x5c
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = ebp - 0x5c;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.eax, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    // a value through sub_4dfd56 and fistp [ebp-0x40]
    auto truncate = [&](double value) {
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(value);
        if (!callTo(0x4dfd56))
            return false;
        m.word(ebp - 0x40, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
        cpu.fpu.count -= 1;
        return true;
    };
    // fcomp; fnstsw ax; sahf
    auto sahfCompare = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
    };
    auto lt = [](x86::reg32 a, x86::reg32 b) { return x86::sreg32(a) < x86::sreg32(b); };
    auto le = [](x86::reg32 a, x86::reg32 b) { return x86::sreg32(a) <= x86::sreg32(b); };
    // the player at [esi+4]
    auto player = [&]() { return m.word(r.esi + 4); };
    r.eax = player();
    if (!callTo(0x422bd0))
        return;
    r.edx = r.eax;
    r.edi = r.eax;  // the player's car
    r.eax = 1;
    if (!callTo(0x422bd0))
        return;
    m.word(ebp - 0x34, r.eax);
    flags.logic8(m.byte(r.edx + 0x200) & 0x20);
    r.eax = (m.byte(r.edx + 0x200) & 0x20) ? 1 : 0;
    m.word(ebp - 4, r.eax);  // the cop's
    r.eax = m.word(ebp - 0x34);
    flags.logic8(m.byte(r.eax + 0x200) & 0x20);
    r.edx = (m.byte(r.eax + 0x200) & 0x20) ? 1 : 0;
    m.word(ebp - 0x3c, r.edx);
    r.edx = m.word(0x6fd3b0);
    flags.compare(r.edx, 1);
    r.eax = r.edx == 1 ? 1 : 0;
    r.edx = player() + r.eax;
    r.eax = player() << 2;
    r.ecx = m.word(r.eax + 0x5e10b8);
    m.word(ebp - 0xc, r.edx);  // the layout's row
    flags.logic(r.ecx);
    if (r.ecx)
        r.eax = m.word(r.ecx);
    else
    {
        r.edx = m.word(r.eax + 0x5e10b0);
        flags.logic(r.edx);
        r.eax = r.edx ? m.word(r.edx) : 0;
    }
    flags.compare(r.eax, 0x10);
    if (r.eax == 0x10)
        return leave();
    flags.compare(m.word(0x7d3684), 0x140);
    if (lt(m.word(0x7d3684), 0x140))
        return leave();
    r.ebx = m.word(0x7a3d10);
    flags.logic(r.ebx);
    if (!r.ebx)
    {
        flags.compare(m.word(0x559264), 0);
        if (m.word(0x559264))
        {
            r.eax = 0xffffffff;
            r.edx = 0;
            m.word(0x559264, r.ebx);
            if (!callTo(0x482030))
                return;
            r.edx = 1;
            r.eax = 0xffffffff;
            r.ebx = 0;
            if (!callTo(0x482030))
                return;
            r.eax = 0;
            if (!callTo(0x4844c0))
                return;
        }
    }
    flags.compare(m.word(r.edi + 0x9a4), 0);
    if (m.word(r.edi + 0x9a4))
    {
        // in the pit: its message only, for the racer with [0x7a3d10] clear
        flags.compare(m.word(ebp - 4), 0);
        if (m.word(ebp - 4))
            return leave();
        flags.compare(m.word(0x7a3d10), 0);
        if (m.word(0x7a3d10))
            return leave();
        r.eax = m.word(0x6fd4bc) - 1;
        r.edx = m.word(r.edi + 0x2c8);
        flags.compare(r.eax, r.edx);
        r.eax = le(r.eax, r.edx) ? 0x1a7 : 0x1a6;
        if (!callTo(0x4d1850))
            return;
        r.ecx = player();
        r.edx = r.eax;
        r.eax = r.ecx;
        if (!callTo(0x482940))
            return;
        return leave();
    }
    r.ecx = ebp - 0x58;
    r.ebx = ebp - 0x50;
    r.edx = ebp - 0x54;
    r.eax = ebp - 0x5c;
    if (!callTo(0x4beb80))
        return;
    // the cop's arrests in a pursuit (kind 7) for game mode 3
    flags.compare(m.word(0x6fd3b8), 3);
    if (m.word(0x6fd3b8) == 3)
    {
        flags.compare(m.word(ebp - 4), 0);
        if (m.word(ebp - 4))
        {
            for (r.edx = 0;; )
            {
                flags.compare(r.edx, m.word(0x5efda0));
                if (!lt(r.edx, m.word(0x5efda0)))
                    break;
                flags.compare(m.word(r.edx * 4 + 0x725238), 0);
                if (m.word(r.edx * 4 + 0x725238))
                {
                    r.ecx = player();
                    r.ebx = r.ecx * 4 + r.ecx;
                    r.eax = r.edx * 4 + r.edx;
                    r.ebx <<= 6;
                    flags.compare(m.word(r.ebx + r.eax * 8 + 0x725278), 7);
                    if (m.word(r.ebx + r.eax * 8 + 0x725278) == 7)
                    {
                        r.ebx = m.word(ebp - 0x50);
                        r.eax = r.esi;
                        if (!callTo(0x47d580))
                            return;
                        break;
                    }
                }
                flags.inc(r.edx);
                ++r.edx;
            }
        }
    }
    if (!callTo(0x489240))
        return;
    // r.eax = player * 23, as lea, sub, shl 3, sub leave it (eax from edx)
    auto times23 = [](x86::reg32 v) { return ((v * 4 - v) << 3) - v; };
    // the frame's end: the mirror's rectangle (sub_482880) in split screen, sub_4d1340
    auto finish = [&]() {
        flags.compare(m.word(0x7a3d10), 0);
        if (!m.word(0x7a3d10))
        {
            flags.compare(m.word(ebp - 0xc), 1);
            if (m.word(ebp - 0xc) != 1)
            {
                flags.compare(m.word(0x6fd3a0), 2);
                if (m.word(0x6fd3a0) == 2)
                {
                    flags.compare(m.word(0x55fe3c), 0);
                    if (m.word(0x55fe3c) && !callTo(0x482880))
                        return;
                }
            }
        }
        flags.compare(m.word(ebp - 0xc), 1);
        if (m.word(ebp - 0xc) != 1 && !callTo(0x4d1340))
            return;
        leave();
    };
    // the speeders' table (the cop's): sub_481c50, then its rows (sub_48a660)
    auto copTable = [&]() {
        r.eax = player();
        if (!callTo(0x481c50))
            return false;
        r.eax = player();
        r.edx = m.word(r.eax * 4 + 0x724780);
        push(r.edx);
        r.edx = r.eax;
        r.eax = times23(r.eax);
        flags.shl(r.eax, 4);
        r.eax <<= 4;
        r.ecx = m.word(r.eax + 0x7498a0);
        push(r.ecx);
        r.ebx = m.word(r.eax + 0x749898);
        push(r.ebx);
        r.eax = r.edx;
        push(m.word(0x791b4c));
        return callTo(0x48a660);
    };
    // the map and the band behind the HUD (layout +0x6fbc70)
    auto map = [&]() {
        r.edx = r.edi;
        r.eax = player();
        if (!callTo(0x48d4c0))
            return false;
        r.eax = player();
        if (!callTo(0x481f90))
            return false;
        flags.logic(r.eax);
        if (!r.eax)
            return true;
        flags.compare(m.word(0x6fd3b0), 1);
        if (m.word(0x6fd3b0) == 1)
            return true;
        r.eax = (player() << 5) + m.word(0x791b48);
        r.edx = m.word(0x791b48);
        if (!callTo(0x47fbe0))
            return false;
        r.eax = player();
        if (!callTo(0x4808b0))
            return false;
        r.eax = 0x749a38;
        if (!callTo(0x49be40))
            return false;
        r.eax = player();
        if (!callTo(0x4800a0))
            return false;
        r.eax = player();
        r.ecx = m.word(0x791b48);
        r.eax = (r.eax << 5) + r.ecx;
        return callTo(0x47fcb0);
    };
    // the 2D layers drawn: 0x725748, 0x76da48 (under state 0x2bf with bit 2 of [0x7a3a58]), 0x749a38
    auto layers = [&](bool mirror) {
        r.eax = 0x725748;
        if (!callTo(0x49be40) || !callTo(0x48e770))
            return false;
        flags.logic8(m.byte(0x7a3a58) & 2);
        if (m.byte(0x7a3a58) & 2)
        {
            r.edx = 1;
            r.eax = 0x2bf;
            if (!callTo(0x431900))
                return false;
        }
        r.eax = 0x76da48;
        if (!callTo(0x49be40))
            return false;
        flags.logic8(m.byte(0x7a3a58) & 2);
        if (m.byte(0x7a3a58) & 2)
        {
            if (mirror)
            {
                r.eax = player();
                if (!callTo(0x482190))
                    return false;
            }
            r.eax = 0x2bf;
            r.edx = 0;
            flags.logic(0);
            if (!callTo(0x431900))
                return false;
        }
        r.eax = 0x749a38;
        return callTo(0x49be40) && callTo(0x4d1340);
    };
    // the wrong-way warning: +0x998 in 0x40..0xbf, blinking past 0x94
    flags.compare(m.word(0x7a3d10), 0);
    if (!m.word(0x7a3d10))
    {
        flags.compare(m.word(ebp - 4), 0);
        if (!m.word(ebp - 4))
        {
            r.edx = m.word(r.edi + 0x998);
            flags.compare(r.edx, 0x40);
            if (!lt(r.edx, 0x40))
            {
                flags.compare(r.edx, 0xc0);
                if (lt(r.edx, 0xc0))
                {
                    flags.compare(r.edx, 0x94);
                    bool red = false;
                    if (lt(r.edx, 0x94))
                    {
                        r.eax = r.edx & 0x1f;
                        flags.compare(r.eax, 0x10);
                        red = lt(r.eax, 0x10);
                    }
                    r.ecx = red ? 0x9fff0000 : 0x9ffafa00;
                    r.edx = player();
                    r.eax = times23(r.edx) << 4;
                    push(4);
                    r.edx = m.word(r.eax + 0x74984c);
                    push(r.edx);
                    r.ebx = m.word(r.eax + 0x749850);
                    push(r.ebx);
                    r.edx = m.word(r.eax + 0x749848);
                    push(r.edx);
                    push(r.ecx);
                    r.eax = 0x151;
                    push(m.word(0x791b3c));
                    if (!callTo(0x4d1850) || !callTo(0x4897f0))
                        return;
                }
            }
        }
    }
    flags.compare(m.word(0x7a3d10), 0);
    bool compact = false;
    if (!m.word(0x7a3d10))
    {
        r.edx = m.word(ebp - 4) * 0x1a4;
        r.eax = m.word(ebp - 0xc) * 0x348;
        flags.compare(m.word(r.edx + r.eax + 0x6fbc50), 0);
        if (m.word(r.edx + r.eax + 0x6fbc50))
        {
            flags.logic8(m.byte(r.edi + 0x200) & 4);
            if (!(m.byte(r.edi + 0x200) & 4))
            {
                flags.logic8(m.byte(0x55eb30) & 1);
                if (!(m.byte(0x55eb30) & 1))
                {
                    flags.compare(m.word(0x791b1c), 0);
                    compact = true;
                    if (!m.word(0x791b1c))
                    {
                        flags.compare(m.word(0x6fd3a0), 2);
                        compact = m.word(0x6fd3a0) != 2;
                    }
                }
            }
        }
    }
    if (compact)
    {
        // without the full HUD (bit 4 of the car's +0x200, bit 0 of [0x55eb30]): the tables, the map, the layers
        r.eax = player();
        r.edx = m.word(0x791acc) + (r.eax << 2);
        r.eax = r.edx + 3;
        push(r.eax);
        r.ecx = r.edx + 2;
        r.eax = 0x749a38;
        r.ebx = r.edx + 1;
        if (!callTo(0x49bdc0))
            return;
        r.edx = m.word(ebp - 0xc) * 0x348;
        r.eax = player();
        r.ecx = m.word(0x791ac8);
        r.eax = (r.eax << 5) + r.ecx;
        r.ecx = m.word(ebp - 4);
        if (!callTo(0x47fcb0))
            return;
        r.eax = r.ecx * 0x1a4;
        r.ebx = m.word(r.edx + r.eax + 0x6fbc58);
        flags.logic(r.ebx);
        if (r.ebx)
        {
            flags.logic(r.ecx);
            if (r.ecx)
            {
                if (!copTable())
                    return;
            }
            else
            {
                flags.compare(m.word(0x5efda0), 1);
                if (!le(m.word(0x5efda0), 1))
                {
                    r.edx = r.edi;
                    r.eax = player();
                    if (!callTo(0x47ee80))
                        return;
                    r.eax = player();
                    if (!callTo(0x481c50))
                        return;
                    r.edx = player();
                    r.eax = (r.edx * 4 - r.edx) << 3;
                    r.ecx = m.word(r.edx * 4 + 0x724780);
                    r.eax -= r.edx;
                    push(r.edx);
                    r.eax <<= 4;
                    push(r.ecx);
                    r.eax = m.word(r.eax + 0x749828);
                    push(r.eax);
                    push(m.word(0x791b4c));
                    if (!callTo(0x48ae70))
                        return;
                }
            }
        }
        r.eax = m.word(ebp - 4) * 0x1a4;
        r.edx = m.word(ebp - 0xc) * 0x348;
        flags.compare(m.word(r.edx + r.eax + 0x6fbc70), 0);
        if (m.word(r.edx + r.eax + 0x6fbc70) && !map())
            return;
        if (!layers(false))
            return;
        return finish();
    }
    // the full HUD
    flags.compare(m.word(0x7a3d10), 0);
    if (m.word(0x7a3d10))
        return finish();
    r.edx = m.word(ebp - 0xc) * 0x348;
    r.eax = m.word(ebp - 4) * 0x1a4;
    flags.compare(m.word(r.edx + r.eax + 0x6fbc50), 0);
    if (!m.word(r.edx + r.eax + 0x6fbc50))
        return finish();
    flags.compare(m.word(0x791b1c), 0);
    if (!m.word(0x791b1c))
    {
        flags.compare(m.word(0x6fd3a0), 2);
        if (m.word(0x6fd3a0) == 2)
            return finish();
    }
    flags.logic8(m.byte(r.edi + 0x200) & 4);
    if (!(m.byte(r.edi + 0x200) & 4))
    {
        flags.logic8(m.byte(0x55eb30) & 1);
        if (!(m.byte(0x55eb30) & 1))
            return finish();
    }
    r.ecx = 0x7fffffff;
    r.ebx = m.word(ebp - 4);
    r.eax = player();
    r.edx = m.word(0x791acc);
    r.eax <<= 2;
    m.word(ebp - 0x14, r.ecx);  // the best lap
    r.edx += r.eax;
    r.eax = r.edi;
    m.word(ebp - 0x18, r.edx);
    if (!callTo(0x4cded0))
        return;
    m.word(0x725234, r.eax);
    // the car the HUD follows: none for the cop, else the leader or the one ahead
    flags.logic(r.ebx);
    if (r.ebx)
    {
        r.eax = 0;
        flags.logic(0);
        m.word(0x725210, r.eax);
    }
    else
    {
        r.edx = player();
        flags.compare(m.word(r.edx * 4 + 0x79c4a0), 0);
        bool call = true;
        if (m.word(r.edx * 4 + 0x79c4a0))
        {
            flags.compare(r.eax, m.word(0x6fd510));
            if (!lt(r.eax, m.word(0x6fd510)))
            {
                r.eax = 0;
                call = false;
            }
            else
            {
                flags.add(r.eax, 2);
                r.eax += 2;
            }
        }
        if (call && !callTo(0x4cdef0))
            return;
        m.word(0x725210, r.eax);
        flags.compare(r.edi, m.word(0x725210));
        if (r.edi == m.word(0x725210))
        {
            r.ecx = 0;
            m.word(0x725210, r.ecx);
        }
    }
    // the best lap among the car's laps, for lap layouts 2 and 3
    r.eax = m.word(ebp - 4) * 0x1a4;
    r.edx = m.word(ebp - 0xc) * 0x348;
    r.eax += r.edx;
    r.ebx = m.word(r.eax + 0x6fbc64);
    flags.compare(r.ebx, 2);
    bool laps = r.ebx == 2;
    if (!laps)
    {
        flags.compare(r.ebx, 3);
        laps = r.ebx == 3;
    }
    if (laps)
    {
        for (r.eax = 0;; )
        {
            flags.compare(r.eax, m.word(r.edi + 0x234));
            if (!lt(r.eax, m.word(r.edi + 0x234)))
                break;
            r.edx = r.eax * 4 + r.edi;
            r.ebx = m.word(ebp - 0x14);
            r.ecx = m.word(r.edx + 0x240);
            flags.compare(r.ecx, r.ebx);
            if (lt(r.ecx, r.ebx))
                m.word(ebp - 0x14, r.ecx);
            flags.inc(r.eax);
            ++r.eax;
        }
    }
    // the lap clock: [0x72522c], and [0x725218] its blinking
    r.eax = m.word(0x7d3684);
    flags.compare(r.eax, 0x200);
    if (lt(r.eax, 0x200))
    {
        r.edx = 0;
        flags.logic(0);
        m.word(0x725218, r.edx);
        m.word(0x72522c, r.edx);
    }
    else
    {
        bool lastLap = false;
        flags.compare(m.word(r.edi + 0x234), 0);
        if (m.word(r.edi + 0x234))
        {
            r.eax -= m.word(r.edi + 0x238);
            flags.compare(r.eax, 0xc0);
            if (lt(r.eax, 0xc0))
                lastLap = true;
            else
            {
                flags.logic8(m.byte(r.edi + 0x200) & 1);
                if (m.byte(r.edi + 0x200) & 1)
                {
                    flags.compare(m.word(r.edi + 0x2c4), 3);
                    lastLap = m.word(r.edi + 0x2c4) == 3;
                }
            }
        }
        if (lastLap)
        {
            r.edx = m.word(ebp - 0xc) * 0x348;
            r.eax = m.word(r.edi + 0x234);
            r.eax = m.word(r.edi + r.eax * 4 + 0x23c);
            r.ecx = m.word(ebp - 4);
            m.word(0x72522c, r.eax);
            r.eax = r.ecx * 0x1a4;
            bool blink = false;
            flags.compare(m.word(r.edx + r.eax + 0x6fbc64), 3);
            if (m.word(r.edx + r.eax + 0x6fbc64) == 3)
            {
                r.edx = m.word(0x6fd2b4);
                flags.compare(r.edx, m.word(0x72522c));
                blink = !lt(r.edx, m.word(0x72522c));
            }
            if (!blink)
            {
                r.eax = m.word(ebp - 4) * 0x1a4;
                r.edx = m.word(ebp - 0xc) * 0x348;
                flags.compare(m.word(r.edx + r.eax + 0x6fbc64), 2);
                if (m.word(r.edx + r.eax + 0x6fbc64) == 2)
                {
                    r.ebx = m.word(ebp - 0x14);
                    flags.compare(r.ebx, m.word(0x72522c));
                    blink = !lt(r.ebx, m.word(0x72522c));
                }
            }
            if (blink)
                m.word(0x725218, 1);
            else
            {
                r.edx = 0;
                flags.logic(0);
                m.word(0x725218, r.edx);
            }
        }
        else
        {
            r.eax = m.word(0x7d3684);
            r.ecx = m.word(r.edi + 0x238);
            r.ebx = 0;
            r.eax -= r.ecx;
            m.word(0x725218, r.ebx);
            m.word(0x72522c, r.eax);
        }
    }
    // the last lap's flash: [0x72521c]
    r.ecx = m.word(r.edi + 0x234) + 1;
    r.eax = m.word(0x6fd4b4);
    flags.compare(r.ecx, r.eax);
    if (!le(r.ecx, r.eax))
        r.ecx = r.eax;
    r.eax = m.word(0x7d3684) - m.word(r.edi + 0x238);
    flags.compare(r.eax, 0x140);
    bool flash = false;
    if (lt(r.eax, 0x140))
    {
        flags.compare(r.ecx, m.word(0x6fd4b4));
        flash = r.ecx == m.word(0x6fd4b4);
    }
    if (flash)
        m.word(0x72521c, 1);
    else
    {
        r.edx = 0;
        m.word(0x72521c, r.edx);
    }
    // the text colour [ebp-8]: grey, or blinking red and white for the wrong way
    flags.compare(m.word(ebp - 4), 0);
    if (m.word(ebp - 4))
        m.word(ebp - 8, 0xffa0a0a0);
    else
    {
        r.edx = m.word(r.edi + 0x998);
        flags.compare(r.edx, 0x94);
        if (lt(r.edx, 0x94))
            m.word(ebp - 8, 0xffa0a0a0);
        else
        {
            r.eax = r.edx & 0x1f;
            flags.compare(r.eax, 0x10);
            m.word(ebp - 8, lt(r.eax, 0x10) ? 0x9fff0000 : 0x9ffafa00);
        }
    }
    r.eax = r.edi;
    if (!callTo(0x47f090))
        return;
    push(m.word(0x791b3c));
    r.ebx = m.word(ebp - 4);
    if (!callTo(0x4d1390))
        return;
    cpu.fpu.count -= 1;  // fstp st(0)
    flags.logic(r.ebx);
    if (r.ebx)
    {
        // the cop's arrests (element 8)
        r.edx = player();
        r.eax = times23(r.edx);
        r.edx = r.eax;
        r.eax = player();
        m.word(ebp - 0x40, r.eax);
        r.ebx = m.word(ebp - 0x40);
        r.eax = times23(r.eax);
        r.ebx = m.word(r.eax * 4 + 0x7256dc);
        r.edx <<= 4;
        push(r.ebx);
        r.eax = m.word(r.edx + 0x74988c);
        push(r.eax);
        r.eax = m.word(r.edx + 0x749890);
        push(r.eax);
        r.ebx = m.word(r.edx + 0x749888);
        push(r.ebx);
        r.eax = m.word(ebp - 8);
        push(r.eax);
        r.edx = m.word(0x6fd4b8);
        r.eax = m.word(r.edi + 0x2cc);
        push(m.word(0x791b3c));
        if (!callTo(0x489a50))
            return;
    }
    // the lap clock (element 5)
    r.edx = player();
    r.eax = times23(r.edx);
    r.edx = m.word(r.eax * 4 + 0x7256a4);
    push(r.edx);
    r.edx = player();
    r.eax = times23(r.edx) << 4;
    r.ebx = m.word(r.eax + 0x7497ac);
    push(r.ebx);
    r.edx = m.word(r.eax + 0x7497b0);
    push(r.edx);
    r.ebx = m.word(r.eax + 0x7497a8);
    push(r.ebx);
    r.eax = m.word(ebp - 8);
    push(r.eax);
    r.eax = m.word(0x72522c);
    push(m.word(0x791b3c));
    if (!callTo(0x4898b0))
        return;
    // the percentage done (element 7) after frame 0x200
    flags.compare(m.word(0x6fd498), 0);
    if (m.word(0x6fd498))
    {
        flags.compare(m.word(0x7d3684), 0x200);
        if (!le(m.word(0x7d3684), 0x200))
        {
            r.ebx = m.word(r.edi + 0x2dc);
            r.eax = m.word(r.edi + 0x2e0);
            flags.add(r.ebx, r.eax);
            r.ebx += r.eax;
            if (r.ebx)
            {
                r.edx = m.word(r.edi + 0x2dc);
                r.eax = (r.edx * 4 - r.edx) << 3;
                r.edx = (r.edx + r.eax) << 2;
                r.eax = r.edx;
                const x86::sreg64 n = x86::sreg64(x86::sreg32(r.edx));
                r.eax = x86::reg32(n / x86::sreg32(r.ebx));
                r.edx = x86::reg32(n % x86::sreg32(r.ebx));
                r.ebx = r.eax;
            }
            else
                r.ebx = 100;
            r.eax = player();
            m.word(ebp - 0x40, r.eax);
            r.edx = m.word(ebp - 0x40);
            r.eax = times23(r.eax);
            r.edx = m.word(r.eax * 4 + 0x7256ac);
            r.eax = player();
            m.word(ebp - 0x40, r.eax);
            push(r.edx);
            r.edx = m.word(ebp - 0x40);
            r.eax = times23(r.eax) << 4;
            r.edx = m.word(r.eax + 0x7497cc);
            push(r.edx);
            r.edx = m.word(r.eax + 0x7497d0);
            push(r.edx);
            r.edx = m.word(r.eax + 0x7497c8);
            push(r.edx);
            r.eax = m.word(ebp - 8);
            push(r.eax);
            r.edx = m.word(r.edi + 0x2d8);
            r.eax = r.ebx;
            push(m.word(0x791b3c));
            if (!callTo(0x489900))
                return;
        }
    }
    // the laps (element 4): the cop's by the most any car has done
    flags.compare(m.word(ebp - 4), 0);
    if (m.word(ebp - 4))
    {
        r.ecx = m.word(0x6fd4b4);
        for (r.eax = 0;; )
        {
            flags.compare(r.eax, m.word(0x5efda0));
            if (!lt(r.eax, m.word(0x5efda0)))
                break;
            r.edx = m.word(r.eax * 4 + 0x5efc48);
            r.ebx = m.word(r.edx + 0x234) + 1;
            m.word(ebp - 0x30, r.ebx);
            flags.compare(r.ecx, r.ebx);
            if (!le(r.ecx, r.ebx))
            {
                flags.compare(m.word(r.edx + 0x2c4), 4);
                if (m.word(r.edx + 0x2c4) != 4)
                    r.ecx = m.word(ebp - 0x30);
            }
            flags.inc(r.eax);
            ++r.eax;
        }
    }
    r.edx = player();
    r.eax = times23(r.edx) << 4;
    m.store(ebp - 0x38, fild32(m, r.eax + 0x74979c));
    r.eax = m.word(0x72521c);
    flags.logic(r.eax);
    bool hideLaps = false;
    if (r.eax)
    {
        flags.logic8(m.byte(0x7d3684) & 0x10);
        hideLaps = m.byte(0x7d3684) & 0x10;
    }
    if (!hideLaps)
    {
        r.edx = player();
        r.eax = times23(r.edx);
        r.edx = m.word(r.eax * 4 + 0x7256a0);
        push(r.edx);
        r.edx = player();
        r.eax = r.edx * 4;
        r.eax -= r.edx;
        if (!truncate(m.load(ebp - 0x38)))
            return;
        r.eax <<= 3;
        r.eax -= r.edx;
        r.ebx = m.word(ebp - 0x40);
        r.eax <<= 4;
        push(r.ebx);
        r.edx = m.word(r.eax + 0x7497a0);
        push(r.edx);
        r.ebx = m.word(r.eax + 0x749798);
        push(r.ebx);
        r.eax = m.word(ebp - 8);
        push(r.eax);
        r.edx = m.word(0x6fd4b4);
        r.eax = r.ecx;
        push(m.word(0x791b3c));
        if (!callTo(0x4899d0))
            return;
    }
    // the position (element 14), for the racer with [0x725234]
    flags.compare(m.word(ebp - 4), 0);
    if (!m.word(ebp - 4))
    {
        flags.compare(m.word(0x725234), 0);
        if (m.word(0x725234))
        {
            r.edx = player();
            r.eax = times23(r.edx);
            r.edx = r.eax;
            r.ebx = m.word(0x6fbb4c);
            r.edx <<= 4;
            flags.logic(r.ebx);
            r.ecx = player();
            r.eax = (r.ecx * 4 - r.ecx) << 3;
            if (r.ebx)
                flags.compare(r.eax, r.ecx);
            r.eax -= r.ecx;
            r.ecx = m.word(r.eax * 4 + 0x7256b8);
            push(r.ecx);
            r.ebx = m.word(r.edx + 0x7497fc);
            push(r.ebx);
            r.eax = m.word(r.edx + 0x749800);
            push(r.eax);
            r.ecx = m.word(r.edx + 0x7497f8);
            push(r.ecx);
            r.ebx = m.word(ebp - 8);
            push(r.ebx);
            push(m.word(0x791b3c));
            const bool small = m.word(0x6fbb4c) != 0;
            if (!small)
                push(m.word(0x791b54));
            if (!callTo(0x4cdee0))
                return;
            r.ecx = m.word(0x725234);
            r.edx = r.eax;
            r.eax = r.ecx;
            if (!callTo(small ? 0x48a320 : 0x48a3a0))
                return;
        }
    }
    // the race-mode meter (game mode 3, the racer): sub_48c0e0 lit by the place (sub_4013b0)
    flags.compare(m.word(0x6fd3b8), 3);
    if (m.word(0x6fd3b8) == 3)
    {
        flags.compare(m.word(ebp - 4), 0);
        if (!m.word(ebp - 4))
        {
            r.eax = r.edi;
            if (!callTo(0x4013b0))
                return;
            r.edx = r.eax - 0xf;
            r.ecx = 10;
            r.eax = r.edx;
            {
                const x86::sreg64 n = x86::sreg64(x86::sreg32(r.edx));
                r.eax = x86::reg32(n / 10);
                r.edx = x86::reg32(n % 10);
            }
            r.ecx = 4 - r.eax;
            flags.logic(r.ecx);
            if (x86::sreg32(r.ecx) < 0)
            {
                r.ecx = 0;
                flags.logic(0);
            }
            else
            {
                flags.compare(r.ecx, 4);
                if (!le(r.ecx, 4))
                    r.ecx = 4;
            }
            bool off;
            flags.compare(m.word(0x7d3684), 0x200);
            if (lt(m.word(0x7d3684), 0x200))
                off = true;
            else
            {
                off = false;
                flags.logic8(m.byte(r.edi + 0x200) & 1);
                if (m.byte(r.edi + 0x200) & 1)
                {
                    flags.compare(m.word(r.edi + 0x2c4), 3);
                    off = m.word(r.edi + 0x2c4) == 3;
                }
                if (!off)
                {
                    flags.logic8(m.byte(r.edi + 0x200) & 1);
                    if (m.byte(r.edi + 0x200) & 1)
                    {
                        flags.compare(m.word(r.edi + 0x2c4), 4);
                        off = m.word(r.edi + 0x2c4) == 4;
                    }
                }
            }
            if (off)
                r.ecx = 0xffffffff;
            else
            {
                flags.compare(r.ecx, 0xffffffff);
                if (r.ecx != 0xffffffff)
                {
                    r.eax = r.ecx;
                    r.edx = player();
                    if (!callTo(0x418990))
                        return;
                }
            }
            r.edx = player();
            r.eax = times23(r.edx);
            push(0xff151f1f);
            r.eax <<= 4;
            push(r.ecx);
            r.edx = m.word(r.eax + 0x749864) - m.word(r.eax + 0x74985c);
            r.ebx = m.word(r.eax + 0x749858);
            push(r.edx);
            r.ecx = m.word(r.eax + 0x749860);
            r.edx = m.word(r.eax + 0x749858);
            r.ecx -= r.ebx;
            r.ebx = m.word(r.eax + 0x74985c);
            r.eax = player();
            if (!callTo(0x48c0e0))
                return;
        }
    }
    // the opponents' count (element 2 of the racer's, game mode 3)
    flags.compare(m.word(0x6fd3b8), 3);
    if (m.word(0x6fd3b8) == 3)
    {
        flags.compare(m.word(ebp - 4), 0);
        if (!m.word(ebp - 4))
        {
            r.edx = player();
            r.eax = times23(r.edx);
            r.ecx = player();
            r.edx = r.eax;
            r.eax = times23(r.ecx);
            r.ecx = m.word(r.eax * 4 + 0x7256c8);
            r.edx <<= 4;
            push(r.ecx);
            r.eax = m.word(r.edx + 0x74983c);
            push(r.eax);
            r.ebx = m.word(r.edx + 0x749840);
            push(r.ebx);
            r.eax = m.word(r.edx + 0x749838);
            push(r.eax);
            r.edx = m.word(ebp - 8);
            push(r.edx);
            r.eax = m.word(r.edi + 0x2c8);
            r.edx = m.word(0x6fd4bc);
            push(m.word(0x791b3c));
            if (!callTo(0x489a50))
                return;
        }
    }
    // the tables (layout +0x6fbc58)
    r.ebx = m.word(ebp - 4);
    r.eax = r.ebx * 0x1a4;
    r.edx = m.word(ebp - 0xc) * 0x348;
    flags.compare(m.word(r.edx + r.eax + 0x6fbc58), 0);
    if (m.word(r.edx + r.eax + 0x6fbc58))
    {
        flags.logic(r.ebx);
        if (r.ebx)
        {
            if (!copTable())
                return;
        }
        else
        {
            flags.compare(m.word(0x5efda0), 1);
            if (!le(m.word(0x5efda0), 1))
            {
                r.edx = r.edi;
                r.eax = player();
                if (!callTo(0x47ee80))
                    return;
                r.eax = player();
                if (!callTo(0x481c50))
                    return;
                r.edx = player();
                r.eax = (r.edx * 4 - r.edx) << 3;
                r.ecx = player();
                r.eax -= r.edx;
                push(r.ecx);
                r.edx = r.eax;
                r.ebx = m.word(r.ecx * 4 + 0x724780);
                r.edx <<= 4;
                push(r.ebx);
                r.eax = m.word(r.edx + 0x749828);
                push(r.eax);
                push(m.word(0x791b4c));
                if (!callTo(0x48ae70))
                    return;
            }
        }
    }
    flags.compare(m.word(ebp - 4), 0);
    if (m.word(ebp - 4))
    {
        r.edx = r.edi;
        r.eax = player();
        if (!callTo(0x47d480))
            return;
    }
    r.eax = m.word(ebp - 0xc) * 0x348;
    r.edx = m.word(ebp - 4) * 0x1a4;
    flags.compare(m.word(r.edx + r.eax + 0x6fbc70), 0);
    if (m.word(r.edx + r.eax + 0x6fbc70) && !map())
        return;
    // the rev counter by its kind (layout +0x6fbc6c): 1 the digital one, else the dial
    r.eax = m.word(ebp - 4) * 0x1a4;
    r.edx = m.word(ebp - 0xc) * 0x348;
    r.eax += r.edx;
    r.ebx = m.word(r.eax + 0x6fbc6c);
    flags.logic(r.ebx);
    if (r.ebx)
    {
        flags.compare(r.ebx, 1);
        if (r.ebx == 1)
        {
            r.edx = player();
            r.eax = times23(r.edx) << 4;
            const double x = fild32(m, r.eax + 0x749780);
            const double y = fild32(m, r.eax + 0x74977c);
            r.eax = m.word(r.edi + 0x528);
            r.edx = m.word(r.eax + 0x1c4);
            r.eax = r.edx * 4;
            r.edx += r.eax;
            r.ecx = 6;
            {
                const x86::sreg64 n = x86::sreg64(x86::sreg32(r.edx));
                r.eax = x86::reg32(n / 6);
                r.edx = x86::reg32(n % 6);
            }
            m.store(ebp - 0x20, y);
            r.ecx = m.word(r.edi + 0x590);
            m.store(ebp - 0x24, x);
            flags.compare(r.eax, r.ecx);
            const bool redline = lt(r.eax, r.ecx);
            // both through sub_4dfd56: y on top first, then x
            cpu.fpu.count += 2;
            cpu.fpu.st(0) = x86::Float(m.load(ebp - 0x20));
            cpu.fpu.st(1) = x86::Float(m.load(ebp - 0x24));
            if (!callTo(0x4dfd56))
                return;
            m.word(ebp - 0x40, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
            cpu.fpu.count -= 1;
            if (redline)
                r.ebx = m.word(ebp - 0x40);
            else
                r.edx = m.word(ebp - 0x40);
            if (!callTo(0x4dfd56))
                return;
            if (redline)
            {
                push(r.ebx);
                m.word(ebp - 0x40, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                r.eax = m.word(ebp - 0x40);
                push(r.eax);
                push(0xffa01010);
                push(m.word(0x791b54));
                r.eax = r.ecx;
            }
            else
            {
                push(r.edx);
                m.word(ebp - 0x40, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                r.ecx = m.word(ebp - 0x40);
                push(r.ecx);
                r.ebx = m.word(ebp - 8);
                push(r.ebx);
                push(m.word(0x791b54));
                r.eax = m.word(r.edi + 0x590);
            }
            cpu.fpu.count -= 1;
            push(m.word(0x791b50));
            if (!callTo(0x489c40))
                return;
        }
        else
        {
            r.eax = m.word(r.edi + 0x528);
            r.ecx = 0xffffffff;
            r.edx = m.word(r.edi + 0x590);
            r.ebx = m.word(r.eax + 0x1c4);
            r.eax = player();
            if (!callTo(0x489e00))
                return;
        }
    }
    // the speedometer by its kind (layout +0x6fbc68): 4 and 2 digital, 3 and 1 the dial
    r.eax = m.word(ebp - 0xc) * 0x348;
    r.edx = m.word(ebp - 4) * 0x1a4;
    r.eax += r.edx;
    r.ebx = m.word(r.eax + 0x6fbc68);
    // |speed| to [ebp+at]: fldz; fcomp; jae, fchs
    auto speed = [&](x86::reg32 at) {
        sahfCompare(0.0, m.load(r.edi + 0x4f4));
        if (cpu.flags.cf)
        {
            r.eax = m.word(r.edi + 0x4f4);
            m.word(ebp + at, r.eax);
        }
        else
            m.store(ebp + at, -m.load(r.edi + 0x4f4));
    };
    flags.compare(r.ebx, 4);
    bool digital = r.ebx == 4;
    if (!digital)
    {
        flags.compare(r.ebx, 2);
        digital = r.ebx == 2;
    }
    if (digital)
    {
        push(0xffffffff);
        speed(0xffffffd4);
        r.eax = player();
        push(m.word(ebp - 0x2c));
        if (!callTo(0x48a220))
            return;
    }
    else
    {
        flags.compare(r.ebx, 3);
        bool dial = r.ebx == 3;
        if (!dial)
        {
            flags.compare(r.ebx, 1);
            dial = r.ebx == 1;
        }
        if (dial)
        {
            r.edx = player();
            r.eax = times23(r.edx) << 4;
            r.edx = m.word(r.eax + 0x74975c);
            push(r.edx);
            r.ecx = m.word(r.eax + 0x749760);
            push(r.ecx);
            r.ebx = m.word(ebp - 8);
            push(r.ebx);
            r.eax = m.word(0x6fd3b0);
            push(m.word(0x791b50));
            auto split = [&]() {
                flags.compare(r.eax, 1);
                if (r.eax != 1)
                    r.eax = 0;
                r.eax += player();
            };
            split();
            r.edx = r.eax * 0x348;
            r.eax = m.word(ebp - 4) * 0x1a4;
            flags.compare(m.word(r.edx + r.eax + 0x6fbc68), 1);
            bool big = m.word(r.edx + r.eax + 0x6fbc68) == 1;
            if (!big)
            {
                r.eax = m.word(0x6fd3b0);
                split();
                r.eax = r.eax * 0x348;
                r.edx = m.word(ebp - 4) * 0x1a4;
                flags.compare(m.word(r.edx + r.eax + 0x6fbc68), 2);
                big = m.word(r.edx + r.eax + 0x6fbc68) == 2;
            }
            if (big)
                r.eax = 1;
            else
            {
                r.eax = 0;
                flags.logic(0);
            }
            push(r.eax);
            speed(0xffffffd8);
            push(m.word(ebp - 0x28));
            if (!callTo(0x489d70))
                return;
        }
    }
    // the gear (element 3's kinds): sub_4a8bb9's, red with the limiter on +0x1f0
    r.edx = m.word(r.edi + 0x1f0);
    r.eax = (r.edx * 4 - r.edx) << 2;
    r.edx = r.eax;
    flags.compare(m.word(r.edx + r.eax * 8 + 0x6fd548), 0);
    if (!m.word(r.edx + r.eax * 8 + 0x6fd548))
    {
        r.eax = r.edi;
        if (!callTo(0x4a8bb9))
            return;
        setByte(r.ecx, 0, m.byte(0x7d3684));
        r.ebx = r.eax;
        flags.logic8(m.byte(0x7d3684) & 0x10);
        r.edx = (m.byte(0x7d3684) & 0x10) ? 0x9fff0000 : 0x9ffafa00;
    }
    else
    {
        r.edx ^= r.eax;
        r.ebx = 0;
    }
    r.eax = m.word(ebp - 0xc) * 0x348;
    r.ecx = m.word(ebp - 4) * 0x1a4;
    r.eax += r.ecx;
    r.ecx = m.word(r.eax + 0x6fbc68);
    flags.compare(r.ecx, 4);
    bool gear = r.ecx == 4;
    if (!gear)
    {
        flags.compare(r.ecx, 2);
        gear = r.ecx == 2;
    }
    bool drawGear = true;
    if (gear)
    {
        r.eax = player();
        m.word(ebp - 0x40, r.eax);
        r.ecx = m.word(ebp - 0x40);
        r.eax = times23(r.eax);
        flags.shl(r.eax, 4);
        r.eax <<= 4;
        r.ecx = m.word(r.eax + 0x749774);
        push(r.ecx);
        r.ecx = m.word(r.eax + 0x749770);
        push(r.ecx);
        r.ecx = m.word(r.eax + 0x749768);
        push(r.ecx);
    }
    else
    {
        flags.compare(r.ecx, 3);
        bool dial = r.ecx == 3;
        if (!dial)
        {
            flags.compare(r.ecx, 1);
            dial = r.ecx == 1;
        }
        if (dial)
        {
            // in the dial's corner: its right edge less [0x53b5f0] of its width
            r.eax = player();
            m.word(ebp - 0x40, r.eax);
            r.ecx = m.word(ebp - 0x40);
            r.eax = times23(r.eax) << 4;
            const double x0 = fild32(m, r.eax + 0x749758);
            const double x1 = fild32(m, r.eax + 0x749760);
            r.ecx = m.word(r.eax + 0x749764);
            const double inset = M::mul(M::sub(x1, x0), m.loadDouble(0x53b5f0));
            push(r.ecx);
            r.eax = m.word(r.eax + 0x749760);
            push(r.eax);
            if (!truncate(M::sub(x1, inset)))
                return;
            r.eax = m.word(ebp - 0x40);
            push(r.eax);
        }
        else
            drawGear = false;
    }
    if (drawGear)
    {
        push(r.edx);
        r.eax = m.word(r.edi + 0x528);
        push(0xffa0a0a0);
        r.eax = m.word(r.eax + 8);
        r.edx = 0;
        push(0xff707070);
        r.eax -= 2;
        setByte(r.edx, 0, m.byte(r.edi + 0x50b));
        push(m.word(0x791b54));
        if (!callTo(0x489b30))
            return;
    }
    // the clocks for the racer, by the lap clock's kind (layout +0x6fbc64, the jump table at 0x482ae8)
    flags.compare(m.word(ebp - 4), 0);
    if (!m.word(ebp - 4))
    {
        r.ecx = m.word(ebp - 0xc) * 0x348;
        r.edx = player();
        r.eax = times23(r.edx);
        r.ebx = m.word(ebp - 4);
        r.edx = r.eax;
        r.eax = r.ebx * 0x1a4;
        r.edx <<= 4;
        r.eax = m.word(r.ecx + r.eax + 0x6fbc64);
        m.store(ebp - 0x10, fild32(m, r.edx + 0x74980c));
        --r.eax;
        flags.compare(r.eax, 4);
        if (r.eax <= 4)
        {
            const x86::reg32 target = m.word(0x482ae8 + r.eax * 4);
            if (target == 0x483ab8)
            {
                // the race clock: the leader's time, or the best lap
                bool shown = true;
                flags.compare(m.word(0x725218), 0);
                if (m.word(0x725218))
                {
                    flags.logic8(m.byte(0x7d3684) & 0x10);
                    shown = !(m.byte(0x7d3684) & 0x10);
                }
                if (shown)
                {
                    flags.compare(m.word(0x6fd2b4), 0);
                    if (m.word(0x6fd2b4))
                    {
                        flags.compare(m.word(r.edi + 0x234), 0);
                        if (m.word(r.edi + 0x234))
                        {
                            r.edx = player();
                            r.eax = times23(r.edx);
                            r.ebx = m.word(r.eax * 4 + 0x7256bc);
                            if (!truncate(m.load(ebp - 0x10)))
                                return;
                            push(r.ebx);
                            r.eax = m.word(ebp - 0x40);
                            push(r.eax);
                            r.eax = times23(r.edx) << 4;
                            r.edx = m.word(r.eax + 0x749810);
                            push(r.edx);
                            r.ecx = m.word(r.eax + 0x749808);
                            push(r.ecx);
                            r.ebx = m.word(ebp - 8);
                            push(r.ebx);
                            r.eax = m.word(0x6fd2b4);
                            r.edx = m.word(ebp - 0x14);
                            push(m.word(0x791b3c));
                            flags.compare(r.eax, r.edx);
                            if (!lt(r.eax, r.edx))
                                r.eax = r.edx;
                        }
                        else
                        {
                            r.edx = player();
                            r.eax = times23(r.edx);
                            r.ecx = m.word(r.eax * 4 + 0x7256bc);
                            r.eax = r.edx * 4;
                            if (!truncate(m.load(ebp - 0x10)))
                                return;
                            r.eax -= r.edx;
                            r.eax <<= 3;
                            r.ebx = m.word(ebp - 0x40);
                            r.eax -= r.edx;
                            push(r.ecx);
                            flags.shl(r.eax, 4);
                            r.eax <<= 4;
                            push(r.ebx);
                            r.edx = m.word(r.eax + 0x749810);
                            push(r.edx);
                            r.ecx = m.word(r.eax + 0x749808);
                            push(r.ecx);
                            r.ebx = m.word(ebp - 8);
                            push(r.ebx);
                            r.eax = m.word(0x6fd2b4);
                            push(m.word(0x791b3c));
                        }
                        if (!callTo(0x4898b0))
                            return;
                    }
                    else
                    {
                        flags.compare(m.word(r.edi + 0x234), 0);
                        if (m.word(r.edi + 0x234))
                        {
                            r.edx = player();
                            r.eax = times23(r.edx);
                            r.edx = m.word(r.eax * 4 + 0x7256bc);
                            push(r.edx);
                            r.edx = player();
                            r.eax = r.edx * 4;
                            r.eax -= r.edx;
                            if (!truncate(m.load(ebp - 0x10)))
                                return;
                            r.eax <<= 3;
                            r.eax -= r.edx;
                            r.ecx = m.word(ebp - 0x40);
                            flags.shl(r.eax, 4);
                            r.eax <<= 4;
                            push(r.ecx);
                            r.ebx = m.word(r.eax + 0x749810);
                            push(r.ebx);
                            r.edx = m.word(r.eax + 0x749808);
                            push(r.edx);
                            r.ecx = m.word(ebp - 8);
                            push(r.ecx);
                            r.eax = m.word(ebp - 0x14);
                            push(m.word(0x791b3c));
                            if (!callTo(0x4898b0))
                                return;
                        }
                    }
                }
            }
            else if (target == 0x483c13)
            {
                // the gap to the cop or to the racer ahead (sub_485a60), red when ahead
                flags.compare(m.word(0x5efda0), 1);
                if (!le(m.word(0x5efda0), 1))
                {
                    r.eax = m.word(r.edi + 0x2bc);
                    flags.compare(r.eax, 1);
                    if (r.eax == 1)
                    {
                        for (r.eax = 0;; )
                        {
                            flags.compare(r.eax, m.word(0x5efda0));
                            if (!lt(r.eax, m.word(0x5efda0)))
                                break;
                            r.edx = m.word(r.eax * 4 + 0x5efc48);
                            flags.compare(m.word(r.edx + 0x2bc), 2);
                            if (m.word(r.edx + 0x2bc) == 2)
                            {
                                r.eax = r.edi;
                                if (!callTo(0x485a60))
                                    return;
                                r.ebx = r.eax;
                                m.word(ebp - 0x1c, r.eax);
                                flags.compare(0, r.ebx);
                                r.ebx = 0 - r.ebx;
                                m.word(ebp - 0x1c, r.ebx);
                                break;
                            }
                            flags.inc(r.eax);
                            ++r.eax;
                        }
                    }
                    else
                    {
                        for (r.eax = 0;; )
                        {
                            flags.compare(r.eax, m.word(0x5efda0));
                            if (!lt(r.eax, m.word(0x5efda0)))
                                break;
                            r.ecx = m.word(r.eax * 4 + 0x5efc48);
                            flags.compare(m.word(r.ecx + 0x2bc), 1);
                            if (m.word(r.ecx + 0x2bc) == 1)
                            {
                                r.edx = r.edi;
                                r.eax = r.ecx;
                                if (!callTo(0x485a60))
                                    return;
                                m.word(ebp - 0x1c, r.eax);
                                break;
                            }
                            flags.inc(r.eax);
                            ++r.eax;
                        }
                    }
                    flags.compare(m.word(ebp - 0x1c), 0);
                    r.ecx = le(m.word(ebp - 0x1c), 0) ? 0xff008c00 : 0xffff0000;
                    r.edx = player();
                    r.eax = times23(r.edx);
                    r.edx = m.word(r.eax * 4 + 0x7256bc);
                    push(r.edx);
                    r.edx = player();
                    r.eax = r.edx * 4;
                    r.eax -= r.edx;
                    if (!truncate(m.load(ebp - 0x10)))
                        return;
                    r.eax <<= 3;
                    r.eax -= r.edx;
                    r.ebx = m.word(ebp - 0x40);
                    flags.shl(r.eax, 4);
                    r.eax <<= 4;
                    push(r.ebx);
                    r.edx = m.word(r.eax + 0x749810);
                    push(r.edx);
                    r.ebx = m.word(r.eax + 0x749808);
                    push(r.ebx);
                    push(r.ecx);
                    r.eax = m.word(ebp - 0x1c);
                    push(m.word(0x791b3c));
                    if (!callTo(0x4898b0))
                        return;
                }
            }
            else if (target == 0x483cf6)
            {
                // the best lap
                flags.compare(m.word(r.edi + 0x234), 0);
                if (m.word(r.edi + 0x234))
                {
                    bool shown = true;
                    flags.compare(m.word(0x725218), 0);
                    if (m.word(0x725218))
                    {
                        flags.logic8(m.byte(0x7d3684) & 0x10);
                        shown = !(m.byte(0x7d3684) & 0x10);
                    }
                    if (shown)
                    {
                        r.edx = player();
                        r.eax = times23(r.edx);
                        r.ecx = m.word(r.eax * 4 + 0x7256bc);
                        r.eax = r.edx * 4;
                        if (!truncate(m.load(ebp - 0x10)))
                            return;
                        r.eax -= r.edx;
                        r.eax <<= 3;
                        r.ebx = m.word(ebp - 0x40);
                        r.eax -= r.edx;
                        push(r.ecx);
                        flags.shl(r.eax, 4);
                        r.eax <<= 4;
                        push(r.ebx);
                        r.edx = m.word(r.eax + 0x749810);
                        push(r.edx);
                        r.ecx = m.word(r.eax + 0x749808);
                        push(r.ecx);
                        r.ebx = m.word(ebp - 8);
                        push(r.ebx);
                        r.eax = m.word(ebp - 0x14);
                        push(m.word(0x791b3c));
                        if (!callTo(0x4898b0))
                            return;
                    }
                }
            }
            else if (target == 0x483d77 || target == 0x483dd8)
            {
                // the last lap's time, or the race's from frame 0x200
                const bool last = target == 0x483d77;
                bool shown = true;
                if (last)
                {
                    flags.compare(m.word(r.edi + 0x234), 0);
                    shown = m.word(r.edi + 0x234) != 0;
                }
                if (shown)
                {
                    r.ecx = player();
                    r.eax = (r.ecx * 4 - r.ecx) << 3;
                    if (last)
                        flags.compare(r.eax, r.ecx);
                    r.eax -= r.ecx;
                    r.ecx = m.word(r.eax * 4 + 0x7256bc);
                    if (!truncate(m.load(ebp - 0x10)))
                        return;
                    push(r.ecx);
                    r.ebx = m.word(ebp - 0x40);
                    push(r.ebx);
                    r.eax = m.word(r.edx + 0x749810);
                    push(r.eax);
                    r.ecx = m.word(r.edx + 0x749808);
                    if (last)
                    {
                        push(r.ecx);
                        r.ebx = m.word(ebp - 8);
                        r.eax = m.word(r.edi + 0x234);
                        push(r.ebx);
                        r.eax = m.word(r.edi + r.eax * 4 + 0x23c);
                    }
                    else
                    {
                        r.ebx = m.word(0x7d3684);
                        push(r.ecx);
                        flags.compare(r.ebx, 0x200);
                        r.eax = le(r.ebx, 0x200) ? 0xffff0000 : m.word(ebp - 8);
                        push(r.eax);
                        r.eax = m.word(0x7d3684) - 0x200;
                    }
                    push(m.word(0x791b3c));
                    if (!callTo(0x4898b0))
                        return;
                }
            }
        }
    }
    // the pursuit's message (layout +0x6fbc54) about the followed car's +0x220
    flags.compare(m.word(0x725210), 0);
    if (m.word(0x725210))
    {
        r.eax = m.word(ebp - 0xc) * 0x348;
        r.edx = m.word(ebp - 4) * 0x1a4;
        flags.compare(m.word(r.edx + r.eax + 0x6fbc54), 0);
        if (m.word(r.edx + r.eax + 0x6fbc54))
        {
            r.eax = m.word(0x725210);
            r.edx = m.word(r.eax + 0x220);
            flags.logic(r.edx);
            if (r.edx)
            {
                flags.add(r.edx, 0x34);
                r.edx += 0x34;
                if (r.edx)
                {
                    r.ecx = player();
                    r.eax = times23(r.ecx);
                    r.ecx = m.word(r.eax * 4 + 0x7256c0);
                    push(r.ecx);
                    r.ecx = player();
                    r.eax = times23(r.ecx) << 4;
                    r.ebx = m.word(r.eax + 0x74981c);
                    push(r.ebx);
                    r.ecx = m.word(r.eax + 0x749820);
                    push(r.ecx);
                    r.ebx = m.word(r.eax + 0x749818);
                    push(r.ebx);
                    r.eax = m.word(ebp - 8);
                    push(r.eax);
                    r.eax = r.edx;
                    push(m.word(0x791b54));
                    if (!callTo(0x4897f0))
                        return;
                }
            }
        }
    }
    // the HUD's quads: sub_49bd30 for the cop on a track without +0x7dd850, else sub_49bdc0
    r.ecx = m.word(ebp - 0x18) + 2;
    r.edx = m.word(ebp - 0x18) + 3;
    r.ebx = m.word(ebp - 0x18) + 1;
    r.eax = m.word(ebp - 4);
    flags.logic(r.eax);
    x86::reg32 quads = 0x49bdc0;
    if (r.eax)
    {
        r.eax = m.word(r.edi + 0x714);
        m.word(ebp - 0x40, r.eax);
        r.edi = m.word(ebp - 0x40);
        r.eax = ((r.eax << 2) + r.edi) << 4;
        flags.compare(m.word(r.eax + 0x7dd850), 0);
        if (!m.word(r.eax + 0x7dd850))
            quads = 0x49bd30;
    }
    push(r.edx);
    r.eax = 0x749a38;
    r.edx = m.word(ebp - 0x18);
    if (!callTo(quads))
        return;
    // the mirror's view: the band rebuilt (sub_4800a0) when it comes or goes
    r.eax = 0;
    r.edx = m.word(ebp - 0xc);
    if (!callTo(0x422aa0))
        return;
    flags.logic(r.edx);
    if (!r.edx)
    {
        flags.compare(r.eax, m.word(0x55d444));
        if (r.eax != m.word(0x55d444))
        {
            m.word(0x55d444, r.eax);
            r.eax = player();
            if (!callTo(0x4800a0))
                return;
        }
    }
    // the layers, or in the top half of split screen with the other car's HUD off only the mirror's frame
    r.ebx = m.word(ebp - 0xc);
    bool layersOnly = false;
    flags.compare(r.ebx, 1);
    if (r.ebx == 1)
    {
        r.eax = m.word(ebp - 0x34);
        flags.compare(m.word(r.eax + 0x9a4), 0);
        if (!m.word(r.eax + 0x9a4))
        {
            r.eax = m.word(ebp - 0x3c) * 0x1a4;
            flags.compare(m.word(r.eax + 0x6fc2e0), 0);
            layersOnly = m.word(r.eax + 0x6fc2e0) != 0;
        }
    }
    if (!layersOnly)
    {
        if (!layers(true))
            return;
    }
    else
    {
        flags.logic8(m.byte(0x7a3a58) & 2);
        if (m.byte(0x7a3a58) & 2)
        {
            r.eax = 0x2bf;
            r.edx = r.ebx;
            if (!callTo(0x431900))
                return;
            r.eax = player();
            if (!callTo(0x482190))
                return;
            r.edx = 0;
            r.eax = 0x2bf;
            if (!callTo(0x431900))
                return;
        }
    }
    finish();
}

/* sub_4b7d30: the textures of the car at eax (edx the 3D shape's): the name
 * sub_439300 gives its model (0x6fd540, 108 bytes a model) copied to the
 * stack; another car of the same model's (+0x220's +0x14) textures shared
 * through sub_4b7ba0, else loaded by sub_4b7a60 -- a pursuit car (model
 * 0..0x31, not the cop) with the model's own palette at 0x6fd54c -- at 256
 * for the player's car and 128 for the rest (256 for all with
 * NFS_CAR_DETAIL_FULL, a port); the player's shared to [0x55fe24] as well
 * when [0x55d000] is 1. */
void carTexturesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x108
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = ebp - 0x108;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    const x86::reg32 car = r.eax;
    const x86::reg32 name = ebp - 0x108;
    m.word(ebp - 4, car);
    m.word(ebp - 8, r.edx);
    auto isPlayer = [&]() {
        r.eax = m.word(0x6fd4f4);
        r.edx = car;
        flags.compare(r.edx, m.word(r.eax * 4 + 0x5efa48));
        return r.edx == m.word(r.eax * 4 + 0x5efa48);
    };
    // port (apply_car_detail): every car at the player's texture size
    r.ebx = isPlayer() ? 256u : (fullCarDetail() ? 256u : 128u);
    r.edx = m.word(car + 0x1f0) * 12;
    r.eax = m.word(r.edx + r.edx * 8 + 0x6fd540);
    r.edi = name;
    if (!callTo(0x439300))
        return;
    // the model's name, two bytes a step as the inline strcpy copies it
    r.esi = r.eax;
    esp -= 4;
    m.word(esp, r.edi);
    for (;;)
    {
        setByte(r.eax, 0, m.byte(r.esi));
        m.byte(r.edi, x86::reg8(r.eax));
        flags.compare8(x86::reg8(r.eax), 0);
        if (!x86::reg8(r.eax))
            break;
        setByte(r.eax, 0, m.byte(r.esi + 1));
        r.esi += 2;
        m.byte(r.edi + 1, x86::reg8(r.eax));
        r.edi += 2;
        flags.compare8(x86::reg8(r.eax), 0);
        if (!x86::reg8(r.eax))
            break;
    }
    r.edi = m.word(esp);
    esp += 4;
    // a pursuit car with its own palette
    r.eax = m.word(car + 0x220);
    r.esi = m.word(r.eax + 0x14);
    flags.logic(r.esi);
    bool pursuit = x86::sreg32(r.esi) >= 0;
    if (pursuit)
    {
        flags.compare(r.esi, 0x31);
        pursuit = x86::sreg32(r.esi) <= 0x31;
    }
    if (pursuit)
    {
        r.eax = car;
        flags.logic8(m.byte(r.eax + 0x200) & 0x20);
        pursuit = !(m.byte(r.eax + 0x200) & 0x20);
    }
    if (pursuit)
    {
        r.edx = m.word(car + 0x1f0);
        r.eax = m.word(ebp - 8);
        esp -= 4;
        m.word(esp, r.eax);
        r.eax = r.edx * 12;
        r.edx = r.eax;
        r.eax = (r.eax << 3) + r.edx;
        flags.add(r.eax, 0x6fd52c);
        r.eax += 0x6fd52c;
        r.ecx = r.eax + 0x20;
        r.edx = name;
        r.eax = car;
    }
    else
    {
        // another car of the same model: its textures shared
        for (r.edx = 0;; )
        {
            flags.compare(r.edx, m.word(0x5efd9c));
            if (x86::sreg32(r.edx) >= x86::sreg32(m.word(0x5efd9c)))
                break;
            r.eax = m.word(r.edx * 4 + 0x5efac8);
            r.esi = m.word(r.eax + 0x8a4);
            flags.logic(r.esi);
            if (r.esi)
            {
                r.edi = car;
                r.ecx = m.word(r.eax + 0x220);
                r.edi = m.word(r.edi + 0x220);
                r.ecx = m.word(r.ecx + 0x14);
                flags.compare(r.ecx, m.word(r.edi + 0x14));
                if (r.ecx == m.word(r.edi + 0x14))
                {
                    r.ebx = m.word(ebp - 8);
                    r.edx = name;
                    if (!callTo(0x4b7ba0))
                        return;
                    r.edx = car;
                    m.word(r.edx + 0x8a4, r.eax);
                    return leave();
                }
            }
            flags.inc(r.edx);
            ++r.edx;
        }
        r.edi = m.word(ebp - 8);
        r.edx = name;
        esp -= 4;
        m.word(esp, r.edi);
        r.eax = car;
        r.ecx = 0;
    }
    if (!callTo(0x4b7a60))
        return;
    r.edx = car;
    m.word(r.edx + 0x8a4, r.eax);
    if (!isPlayer())
        return leave();
    flags.compare(m.word(0x55d000), 1);
    if (m.word(0x55d000) != 1)
        return leave();
    r.ebx = m.word(ebp - 8);
    r.edx = name;
    r.eax = car;
    if (!callTo(0x4b7ba0))
        return;
    m.word(0x55fe24, r.eax);
    leave();
}

/* sub_494cb0: the loading screen: the picture at eax (sub_4f09c0 into an
 * off-screen buffer, sub_4ef190/sub_4ead90/sub_4ef1b0 around it) drawn as 32x32
 * tiles (sub_4f2cb0's texture, sub_4d4080), each from floor to ceil
 * (sub_4f1c30, sub_4f2d78) of its corners scaled to the window (sub_4beb20's
 * size of 0x7cdac0 times [0x53bddc], [0x53bde0], [0x53bde8]) by sub_4d79b0;
 * then the "loading" text (string 0x18c) at its place and [0x79f280],
 * [0x79f284] set from its width (sub_4d1000) for the progress bar.  The
 * whole picture is fitted to 4:3 first (a port: loadingScreenFit). */
void loadingScreenNative(win32::WinApplication* app, x86::CPU& cpu)
{
    // port (apply_loading_screen): the loading screen at 4:3
    loadingScreenFit(app, cpu, true);
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x184; sub ebp, 0x82
    const x86::reg32 ebp = entry - 24 - 0x82;
    x86::reg32 esp = entry - 24 - 0x184;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.eax, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    // sub esp, 8; fstp qword [esp]
    auto pushDouble = [&](double value) {
        esp -= 8;
        app->getMemory<double>(esp) = value;
    };
    auto fxch = [&](int i) {
        const x86::Float top = cpu.fpu.st(0);
        cpu.fpu.st(0) = cpu.fpu.st(i);
        cpu.fpu.st(i) = top;
    };
    // fstp dword: the value a callee returned
    auto popTo = [&](x86::reg32 at) {
        m.store(at, double(cpu.fpu.st(0)));
        cpu.fpu.count -= 1;
    };
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.edx = 3;
        r.eax = 0x2bf;
        if (!callTo(0x431900))
            return;
    }
    r.edx = 1;
    r.eax = ebp + 0x4e;
    r.ecx = ebp + 0x4a;
    r.ebx = ebp + 0x46;
    m.word(0x79f288, r.edx);
    push(r.eax);
    r.edx = ebp + 0x42;
    r.eax = 0x7cdac0;
    if (!callTo(0x4beb20))
        return;
    // the picture's scale to the window
    r.eax = m.word(ebp + 0x4a);
    m.word(ebp + 0x7e, r.eax);
    r.eax = m.word(ebp + 0x4e);
    const double across = M::mul(fild32(m, ebp + 0x7e), m.load(0x53bddc));
    m.word(ebp + 0x7e, r.eax);
    const double down = M::mul(fild32(m, ebp + 0x7e), m.load(0x53bde0));
    r.ecx = 0x10;
    r.edx = 0x20;
    r.ebx = r.ecx;
    m.store(ebp + 0x72, across);
    m.store(ebp + 0x76, down);
    // the picture into a buffer of 32x32 tiles
    if (!callTo(0x431bc0))
        return;
    r.eax = r.edx;
    if (!callTo(0x4ef190))
        return;
    r.ecx = 0x14;
    r.ebx = 0xf;
    m.word(ebp + 0x7a, r.eax);
    if (!callTo(0x4eada0))
        return;
    r.eax = 0xff000000;
    r.edx = 0x1e0;
    if (!callTo(0x4ef760) || !callTo(0x4ef1b0))
        return;
    m.word(ebp + 0x6a, r.ecx);  // rows
    m.word(ebp + 0x66, r.ebx);  // columns
    r.ecx = 0x10;
    r.eax = 0x280;
    r.ebx = r.ecx;
    if (!callTo(0x4ef190))
        return;
    m.word(ebp + 0x6e, r.eax);
    if (!callTo(0x4eada0))
        return;
    flags.compare8(m.byte(0x565010), 8);
    r.eax = m.byte(0x565010) == 8 ? 0xff : 0;
    if (!callTo(0x4ef1b0))
        return;
    r.eax = r.esi;
    r.ebx = 0;
    r.edx = 0;
    if (!callTo(0x4f09c0))
        return;
    for (r.edi = 0;; )
    {
        flags.compare(r.edi, m.word(ebp + 0x6a));
        if (x86::sreg32(r.edi) >= x86::sreg32(m.word(ebp + 0x6a)))
            break;
        for (r.esi = 0;; )
        {
            flags.compare(r.esi, m.word(ebp + 0x66));
            if (x86::sreg32(r.esi) >= x86::sreg32(m.word(ebp + 0x66)))
                break;
            r.eax = m.word(ebp + 0x7a);
            r.ebx = r.esi << 5;
            r.edx = r.edi << 5;
            r.eax = m.word(r.eax + 0x20);
            r.ecx = ebp - 2;
            if (!callTo(0x4f2cb0))
                return;
            r.eax = m.word(ebp + 0x7a);
            push(0);
            r.ebx = 0x20;
            r.edx = m.word(r.eax + 0x20);
            r.eax = 0;
            m.word(ebp + 0x7e, r.edi);
            if (!callTo(0x4d4080))
                return;
            // the tile's corners: floor of its top left, ceil of its bottom right
            const double D = m.loadDouble(0x53bde8);
            const double y = M::mul(fild32(m, ebp + 0x7e), D);
            app->getMemory<double>(ebp + 0x2a) = m.load(ebp + 0x72);
            pushDouble(M::mul(y, m.loadDouble(ebp + 0x2a)));
            m.word(ebp + 0x7e, r.esi);
            if (!callTo(0x4f1c30))
                return;
            const double x = M::mul(fild32(m, ebp + 0x7e), D);
            app->getMemory<double>(ebp + 0x32) = m.load(ebp + 0x76);
            pushDouble(M::mul(x, m.loadDouble(ebp + 0x32)));
            popTo(ebp + 0x52);
            if (!callTo(0x4f1c30))
                return;
            pushDouble(M::mul(m.loadDouble(ebp + 0x2a), D));
            popTo(ebp + 0x56);
            if (!callTo(0x4f2d78))
                return;
            const double last = M::mul(m.loadDouble(ebp + 0x32), D);
            flags.compare(esp, 8);
            pushDouble(last);
            popTo(ebp + 0x3a);
            if (!callTo(0x4f2d78))
                return;
            push(0xffffffff);
            r.eax = m.word(ebp + 2);
            cpu.fpu.count += 3;
            cpu.fpu.st(0) = x86::Float(m.load(ebp + 0x3a));
            cpu.fpu.st(1) = x86::Float(m.load(ebp + 0x56));
            cpu.fpu.st(2) = x86::Float(m.load(ebp + 0x52));
            if (!callTo(0x4dfd56))
                return;
            fxch(1);
            if (!callTo(0x4dfd56))
                return;
            fxch(2);
            if (!callTo(0x4dfd56))
                return;
            fxch(3);
            if (!callTo(0x4dfd56))
                return;
            // fxch, fistp four times: st(1), st(2), st(3), st(0) as they are now
            m.word(ebp + 0x5e, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(1))));
            m.word(ebp + 0x7e, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(2))));
            m.word(ebp + 0x62, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(3))));
            m.word(ebp + 0x5a, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
            cpu.fpu.count -= 4;
            r.ecx = m.word(ebp + 0x5e);
            r.edx = m.word(ebp + 0x5a);
            r.ebx = m.word(ebp + 0x7e);
            push(r.edx);
            r.edx = m.word(ebp + 0x62);
            flags.inc(r.esi);
            ++r.esi;
            if (!callTo(0x4d79b0))
                return;
        }
        flags.inc(r.edi);
        ++r.edi;
    }
    r.eax = m.word(ebp + 0x7a);
    if (!callTo(0x4eff00) || !callTo(0x4ead90))
        return;
    r.eax = m.word(ebp + 0x6e);
    r.ecx = 0xffffffff;
    if (!callTo(0x4eff00))
        return;
    // the text: its size, "loading" at its place
    push(m.word(0x791b54));
    r.ebx = 0;
    if (!callTo(0x4d1390))
        return;
    cpu.fpu.count -= 1;  // fstp st(0)
    r.eax = 0x18c;
    m.word(0x561b34, r.ecx);
    m.word(0x561b3c, r.ebx);
    if (!callTo(0x4d1850))
        return;
    push(r.eax);
    push(0x53bdd8);
    r.eax = ebp - 0x102;
    push(r.eax);
    if (!callTo(0x4df690))
        return;
    const double textY = M::mul(m.load(ebp + 0x76), m.load(0x53bdf0));
    const double textX = M::mul(m.load(ebp + 0x72), m.load(0x53bdf4));
    r.eax = ebp - 0x102;
    esp += 0xc;
    cpu.fpu.count += 2;
    cpu.fpu.st(0) = x86::Float(textY);
    cpu.fpu.st(1) = x86::Float(textX);
    if (!callTo(0x4dfd56))
        return;
    m.store(ebp + 0x62, double(cpu.fpu.st(1)));
    m.word(ebp + 0x7e, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
    cpu.fpu.count -= 1;
    if (!callTo(0x4dfd56))
        return;
    r.ebx = m.word(ebp + 0x7e);
    m.word(ebp + 0x7e, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
    cpu.fpu.count -= 1;
    r.edx = m.word(ebp + 0x7e);
    if (!callTo(0x4d1000))
        return;
    // the progress bar: from the text's right edge, over [0x53bdfc] of the width
    const double scale = M::div(m.load(0x53bdfc), fild32(m, 0x7cdac8));
    const double start = m.load(ebp + 0x62);
    m.store(0x79f284, double(cpu.fpu.st(0)));
    cpu.fpu.count -= 1;
    m.store(0x79f280, M::add(M::add(start, m.load(0x79f284)), m.load(0x53bdf8)));
    m.store(0x79f280, M::mul(scale, m.load(0x79f280)));
    if (!callTo(0x4d1340))
        return;
    flags.logic8(m.byte(0x7a3a58) & 2);
    if (m.byte(0x7a3a58) & 2)
    {
        r.eax = 0x2bf;
        r.edx = 0;
        if (!callTo(0x431900))
            return;
    }
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = saved[0];
    cpu.ecx = saved[1];
    cpu.edx = saved[2];
    cpu.esi = saved[3];
    cpu.edi = saved[4];
    cpu.ebp = saved[5];
    cpu.esp = entry + 4;
}

/* sub_495bc0: a movie (the file named at eax) played: its MAD chunks
 * (sub_495b60 the next of them, "MADk" a key frame and "MADe" one to skip
 * when late) decoded by 16x16 blocks (sub_4f3560, sub_4f3600) into the
 * buffers sub_495890 hands out, shown by sub_4958c0 at their time ([0x79f310]
 * a frame in 16.16) against the clock (sub_495a50), centred on the screen or,
 * with ecx, at ebx, ecx; the sound streamed (sub_4f3a10..sub_4f3a80,
 * sub_495a10, sub_4f2f50 to its end) and [edx] set when the player presses
 * to skip (sub_4df340 2).  ret 4. */
void moviePlayerNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x68
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 0x68;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.eax };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    const x86::reg32 madk = 0x6b44414d, made = 0x6544414d;
    // the generated code's safepoints, at each of its loops' heads
    auto safepoint = [&]() {
        if (app->contextWanted())
            app->yieldContext(cpu);
    };
    auto half = [](x86::reg32 v) {  // (v - (v >> 31)) >> 1, as sar, sub, sar does it
        return x86::reg32(x86::sreg32(v - x86::reg32(x86::sreg32(v) >> 31)) >> 1);
    };
    m.word(ebp - 0x2c, r.edx);
    m.word(ebp - 0x3c, r.ebx);
    m.word(ebp - 0x30, r.ecx);
    // the file's buffer and stream
    r.edx = 0x100000;
    r.eax = 0x53be88;
    r.ebx = m.word(0x5643f4);
    if (!callTo(0x4e1620))
        return;
    push(0x100000);
    r.ebx = 2;
    m.word(ebp - 0x40, r.eax);
    r.ecx = r.eax;
    r.edx = r.ebx;
    r.eax = r.ebx;
    if (!callTo(0x4cfa80))
        return;
    r.edx = 2;
    r.ecx = 0x4353;
    r.ebx = 0xffff;
    r.esi = r.eax;
    m.word(ebp - 8, r.eax);
    if (!callTo(0x4cfe70))
        return;
    push(2);
    r.edx = 1;
    m.word(ebp - 0x24, r.eax);
    r.eax = r.esi;
    if (!callTo(0x4cfca0))
        return;
    r.edx = r.edi;
    r.eax = r.esi;
    r.ecx = 0;
    r.ebx = 0;
    if (!callTo(0x4cfef0))
        return;
    m.word(ebp - 0x34, r.eax);
    r.eax = r.esi;
    if (!callTo(0x495b60))
        return;
    r.esi = r.eax;
    flags.logic(r.eax);
    if (!r.eax)
    {
        push(0x53be8c);
        if (!callTo(0x401010))
            return;
        esp += 4;
    }
    // the first chunk's header: frame time, width, height
    r.edx = 1;
    r.eax = m.word(r.esi + 0xc);
    r.edi = m.word(ebp - 0x30);
    m.word(0x79f310, r.eax);
    r.eax = x86::reg32(x86::sreg32(m.word(r.esi + 0xe)) >> 16);
    r.ecx = 0;
    m.word(0x79f32c, r.edx);
    m.word(0x79f324, r.eax);
    r.eax = x86::reg32(x86::sreg32(m.word(r.esi + 0x10)) >> 16);
    m.word(0x79f314, r.ecx);
    m.word(0x79f334, r.ecx);
    m.word(0x79f31c, r.eax);
    flags.logic(r.edi);
    if (!r.edi)
    {
        // centred
        r.eax = m.word(0x79f324);
        r.edx = m.word(0x564384) - r.eax;
        r.eax = r.edx;
        r.edx = x86::reg32(x86::sreg32(r.edx) >> 31);
        r.eax = half(r.eax);
        r.ecx = m.word(0x79f31c);
        r.edx = m.word(0x564388) - r.ecx;
        r.edi = r.eax;
        r.eax = r.edx;
        r.edx = x86::reg32(x86::sreg32(r.edx) >> 31);
        r.eax -= r.edx;
        flags.sar(r.eax, 1);
        flags.of = false;
        r.eax = x86::reg32(x86::sreg32(r.eax) >> 1);
    }
    else
    {
        r.eax = m.word(0x564384);
        r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
        r.eax = half(r.eax);
        r.edi = r.eax;
        r.eax = m.word(0x564388);
        r.edx = x86::reg32(x86::sreg32(r.eax) >> 31);
        r.eax = half(r.eax);
        r.ebx = m.word(0x79f324);
        r.edx = m.word(0x79f31c);
        r.edi -= r.ebx;
        r.eax -= r.edx;
    }
    r.ecx = m.word(ebp + 0x10);
    push(r.ecx);
    r.ebx = m.word(ebp - 0x30);
    push(r.ebx);
    r.edx = m.word(ebp - 0x3c);
    push(r.edx);
    r.ecx = m.word(0x79f31c);
    r.ebx = m.word(0x79f324);
    push(6);
    r.edx = r.eax;
    r.eax = r.edi;
    if (!callTo(0x4957a0) || !callTo(0x495890))
        return;
    r.ecx = 0;
    r.edi = 0;
    m.word(ebp - 4, r.ecx);
    r.ecx = r.eax;
    // a chunk decoded into the buffer at `buffer`, by 16x16 blocks
    auto decode = [&](x86::reg32 row, x86::reg32 column, bool second) {
        r.edx = m.word(r.esi);
        r.eax = r.esi + 0x18;
        flags.compare(second ? r.edx : r.ebx, madk);
        const bool key = (second ? r.edx : r.ebx) == madk;
        r.ebx = 0;
        if (key)
        {
            r.edx = 0;
            flags.logic(0);
        }
        else
            r.edx = 1;
        setByte(r.ebx, 0, m.byte(r.esi + 0x15));
        if (!callTo(0x4f3560))
            return false;
        if (second)
            r.ebx = 0;
        else
            r.eax = 0;
        m.word(ebp + row, 0);
        if (second)
            m.word(ebp - 0x28, 0);
        for (;;)
        {
            safepoint();
            r.eax = m.word(ebp + row);
            flags.compare(r.eax, m.word(0x79f31c));
            if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x79f31c)))
                break;
            r.ebx = 0;
            m.word(ebp + column, 0);
            for (;;)
            {
                safepoint();
                r.eax = m.word(ebp + column);
                flags.compare(r.eax, m.word(0x79f324));
                if (x86::sreg32(r.eax) >= x86::sreg32(m.word(0x79f324)))
                    break;
                r.ebx = m.word(0x79f324);
                r.edx = m.word(ebp + row) * r.ebx + r.eax;
                r.eax = half(r.edx);
                r.edx = x86::reg32(x86::sreg32(r.edx) >> 31);
                r.eax <<= 2;
                r.edx = (second ? m.word(ebp - 0x1c) : r.ecx) + r.eax;
                m.word(ebp - 0x48, r.edx);
                r.eax += m.word(ebp - 0x20);
                r.edx = m.word(ebp - 0x48);
                if (!callTo(0x4f3600))
                    return false;
                flags.add(m.word(ebp + column), 0x10);
                m.word(ebp + column, m.word(ebp + column) + 0x10);
            }
            if (second)
            {
                // the clock kept running once a row
                flags.compare(m.word(ebp - 0x28), 0);
                if (!m.word(ebp - 0x28))
                {
                    if (!callTo(0x495a50) || !callTo(0x495920))
                        return false;
                    m.word(ebp - 0x28, r.eax);
                }
            }
            flags.add(m.word(ebp + row), 0x10);
            m.word(ebp + row, m.word(ebp + row) + 0x10);
        }
        return true;
    };
    // the frames up front, as fast as they come
    for (;;)
    {
        safepoint();
        flags.logic(r.esi);
        if (!r.esi)
            break;
        flags.logic(r.ecx);
        if (!r.ecx)
            break;
        r.ebx = m.word(r.esi);
        if (!decode(0xffffffec, 0xfffffff0, false))
            return;
        flags.compare(m.word(r.esi), made);
        if (m.word(r.esi) != made)
            m.word(ebp - 0x20, r.ecx);
        r.eax = m.word(ebp - 8);
        r.edx = r.esi;
        if (!callTo(0x4d03c0))
            return;
        r.eax = m.word(ebp - 4);
        if (!callTo(0x4958c0))
            return;
        r.ecx = m.word(0x79f310);
        r.edi += r.ecx;
        r.eax = x86::reg32(x86::sreg32(r.edi) >> 16);
        r.ebx = m.word(ebp - 4);
        r.edi &= 0xffff;
        flags.add(r.ebx, r.eax);
        r.ebx += r.eax;
        if (!callTo(0x495890))
            return;
        r.ecx = r.eax;
        r.eax = m.word(ebp - 8);
        m.word(ebp - 4, r.ebx);
        if (!callTo(0x495b60))
            return;
        r.esi = r.eax;
    }
    // the sound
    r.eax = ebp - 0x68;
    r.edx = 0x1e;
    if (!callTo(0x4e9b20))
        return;
    r.eax = 1;
    if (!callTo(0x4f3a10))
        return;
    r.ebx = m.word(0x5643f4);
    r.ecx = r.eax;
    r.edx = r.eax;
    r.eax = 0x53be88;
    if (!callTo(0x4e1620))
        return;
    r.ebx = 1;
    push(r.ecx);
    r.edx = ebp - 0x68;
    m.word(ebp - 0x38, r.eax);
    push(r.eax);
    r.ecx = 0x1e;
    r.eax = m.word(ebp - 0x24);
    if (!callTo(0x4f3a60))
        return;
    r.ecx = r.eax;
    m.word(ebp - 0x44, r.eax);
    r.eax = m.word(ebp - 0x24);
    if (!callTo(0x4d0460))
        return;
    flags.logic(r.eax);
    if (!r.eax)
        r.ecx = 0xffffffff;
    else
    {
        r.ebx = m.word(ebp - 0x34);
        r.eax = r.ecx;
        r.edx = 0;
        if (!callTo(0x4f3a80))
            return;
        r.ecx = r.eax;
    }
    r.eax = r.ecx;
    if (!callTo(0x495a10))
        return;
    r.eax = 0;
    m.word(0x79f330, 0);
    // the movie in time
    for (;;)
    {
        if (movieSkipRequested())
        {
            // the player's own abort (event type 2, Escape), and out
            m.word(m.word(ebp - 0x2c), 1);
            goto cleanup;
        }
        safepoint();
        flags.logic(r.esi);
        if (!r.esi)
            break;
        flags.compare(m.word(0x79f330), 0);
        if (m.word(0x79f330))
            break;
        r.eax = 0;
        if (!callTo(0x4e7630) || !callTo(0x4df340))
            return;
        flags.compare(r.eax, 2);
        if (r.eax == 2)
        {
            r.eax = m.word(ebp - 0x2c);
            m.word(r.eax, 1);
        }
        if (!callTo(0x495a50))
            return;
        r.edx = m.word(r.esi);
        r.ebx = r.eax;
        flags.compare(r.edx, made);
        if (r.edx == made)
        {
            r.edx = m.word(ebp - 0x2c);
            flags.compare(m.word(r.edx), 0);
            bool skip = true;
            if (!m.word(r.edx))
            {
                r.edx = m.word(ebp - 4) - r.eax;
                flags.compare(r.edx, 0x85);
                skip = x86::sreg32(r.edx) < 0x85;
            }
            if (skip)
            {
                // a skippable frame not decoded: its time on
                r.eax = m.word(0x79f334) + 1;
                r.edx = m.word(0x79f310);
                r.edi += r.edx;
                m.word(0x79f334, r.eax);
                r.eax = r.edi;
                r.edx = m.word(ebp - 4);
                r.eax = x86::reg32(x86::sreg32(r.eax) >> 16);
                r.edx += r.eax;
                r.eax = m.word(ebp - 8);
                m.word(ebp - 4, r.edx);
                r.edx = r.esi;
                if (!callTo(0x4d03c0))
                    return;
                r.eax = m.word(ebp - 8);
                r.edi &= 0xffff;
                if (!callTo(0x495b60))
                    return;
                r.esi = r.eax;
                flags.logic(r.eax);
                if (!r.eax)
                    break;
            }
        }
        r.eax = m.word(ebp - 4) - r.ebx;
        flags.compare(r.eax, 0x43);
        if (x86::sreg32(r.eax) < 0x43)
        {
            // behind: chunks dropped up to the next key frame
            for (;;)
            {
                safepoint();
                flags.compare(m.word(r.esi), made);
                if (m.word(r.esi) != made)
                {
                    flags.inc(m.word(0x79f314));
                    m.word(0x79f314, m.word(0x79f314) + 1);
                }
                else
                    m.word(0x79f334, m.word(0x79f334) + 1);
                r.ebx = m.word(0x79f310);
                r.edi += r.ebx;
                r.eax = r.edi;
                r.edx = m.word(ebp - 4);
                r.eax = x86::reg32(x86::sreg32(r.eax) >> 16);
                r.edx += r.eax;
                r.eax = m.word(ebp - 8);
                m.word(ebp - 4, r.edx);
                r.edx = r.esi;
                if (!callTo(0x4d03c0))
                    return;
                r.eax = m.word(ebp - 8);
                r.edi &= 0xffff;
                if (!callTo(0x495b60))
                    return;
                r.esi = r.eax;
                flags.logic(r.eax);
                if (!r.eax)
                    break;
                flags.compare(m.word(r.eax), madk);
                if (m.word(r.eax) == madk)
                    break;
            }
            flags.logic(r.esi);
            if (!r.esi)
                break;
        }
        // a buffer to decode into, the clock kept running meanwhile
        for (;;)
        {
            safepoint();
            if (!callTo(0x495890))
                return;
            m.word(ebp - 0x1c, r.eax);
            flags.compare(m.word(ebp - 0x1c), 0);
            if (m.word(ebp - 0x1c))
                break;
            if (!callTo(0x495a50) || !callTo(0x495920))
                return;
        }
        if (!decode(0xfffffff4, 0xffffffe8, true))
            return;
        flags.compare(m.word(r.esi), made);
        if (m.word(r.esi) != made)
            m.word(ebp - 0x20, m.word(ebp - 0x1c));
        r.eax = m.word(ebp - 8);
        r.edx = r.esi;
        if (!callTo(0x4d03c0))
            return;
        r.eax = m.word(ebp - 4);
        if (!callTo(0x4958c0))
            return;
        flags.logic(r.ecx);
        if (x86::sreg32(r.ecx) < 0)
        {
            r.eax = m.word(ebp - 0x24);
            if (!callTo(0x495b30))
                return;
        }
        r.esi = m.word(0x79f310);
        r.edi += r.esi;
        r.eax = r.edi;
        r.edx = m.word(ebp - 4);
        r.eax = x86::reg32(x86::sreg32(r.eax) >> 16);
        r.edi &= 0xffff;
        flags.add(r.edx, r.eax);
        r.edx += r.eax;
        r.eax = m.word(ebp - 8);
        m.word(ebp - 4, r.edx);
        if (!callTo(0x495b60))
            return;
        r.esi = r.eax;
        if (!callTo(0x495b10))
            return;
    }
    // the last frame's time out, then the sound to its end
    for (;;)
    {
        safepoint();
        if (!callTo(0x495a50))
            return;
        flags.compare(r.eax, m.word(ebp - 4));
        if (x86::sreg32(r.eax) >= x86::sreg32(m.word(ebp - 4)))
            break;
        if (!callTo(0x495920))
            return;
        r.eax = 0;
        flags.logic(0);
        if (!callTo(0x4e7630))
            return;
    }
    flags.logic(r.ecx);
    if (x86::sreg32(r.ecx) >= 0)
    {
        for (;;)
        {
            safepoint();
            r.edx = ebp - 0x58;
            r.eax = r.ecx;
            if (!callTo(0x4f2f50))
                return;
            r.eax = 0;
            if (!callTo(0x4e7630) || !callTo(0x495b10))
                return;
            flags.compare(m.word(ebp - 0x58), 3);
            if (m.word(ebp - 0x58) == 3)
                break;
            flags.compare(m.word(0x79f330), 0);
            if (m.word(0x79f330))
                break;
        }
    }
cleanup:
    r.eax = m.word(ebp - 0x44);
    if (!callTo(0x4f43c0))
        return;
    r.eax = m.word(ebp - 8);
    if (!callTo(0x4cfd30))
        return;
    r.eax = m.word(ebp - 0x40);
    if (!callTo(0x4e1890))
        return;
    r.eax = m.word(ebp - 0x38);
    if (!callTo(0x4e1890) || !callTo(0x495860))
        return;
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = r.edx;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 8;
}

/* sub_4d0a10: the 2D picture at [0x56727c] (+4, +6 its width and height) made
 * texture [0x840184] (44 bytes each from 0x83f6a8: its size twice, the THRASH
 * handle): one over 0x8000 pixels by sub_4d4080 at 256; else decoded
 * (sub_4d37a0, its format to [ebp-0x34]) and copied into a square of 64, 128
 * or 256 by its depth (sub_4d18b0: 8, 16 or 32 bits), cut or padded to it,
 * then THRASH_talloc and THRASH_tupdate. */
void pictureTextureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x34
    const x86::reg32 ebp = entry - 24;
    x86::reg32 esp = ebp - 0x34;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.edx = saved[2];
        cpu.esi = saved[3];
        cpu.edi = saved[4];
        cpu.ebp = saved[5];
        cpu.esp = entry + 4;
    };
    auto width = [&]() { return x86::reg32(x86::sreg32(m.word(m.word(0x56727c) + 2)) >> 16); };
    auto height = [&]() { return x86::reg32(x86::sreg32(m.word(m.word(0x56727c) + 4)) >> 16); };
    r.edx = m.word(0x56727c);
    r.ecx = x86::reg32(x86::sreg32(m.word(r.edx + 2)) >> 16);
    r.eax = x86::reg32(x86::sreg32(m.word(r.edx + 4)) >> 16) * r.ecx;
    flags.compare(r.eax, 0x8000);
    if (x86::sreg32(r.eax) > 0x8000)
    {
        // a big one: sub_4d4080 at 256
        r.esi = m.word(0x840184);
        push(3);
        r.ecx = 0x83f6a8 + r.esi * 44;
        r.ebx = 0x100;
        r.eax = 0;
        if (!callTo(0x4d4080))
            return;
        r.edx = m.word(0x840184);
        flags.compare(r.edx * 12, r.edx);
        r.eax = r.edx * 11;
        r.edx = 0x100;
        app->getMemory<x86::reg16>(r.eax * 4 + 0x83f6aa) = 0x100;
        app->getMemory<x86::reg16>(r.eax * 4 + 0x83f6a8) = 0x100;
        return leave();
    }
    r.ecx = 3;
    r.ebx = 1;
    m.word(ebp - 0x34, r.ecx);
    m.word(ebp - 0x30, r.ebx);
    // its size: 64 or 128 when it fits, else 256
    r.esi = 0x100;
    r.ebx = (r.ebx & 0xffff0000) | app->getMemory<x86::reg16>(r.edx + 4);
    flags.compare16(x86::reg16(r.ebx), 0x40);
    bool small = false;
    if (x86::sreg16(x86::reg16(r.ebx)) < 0x40)
    {
        flags.compare16(app->getMemory<x86::reg16>(r.edx + 6), 0x40);
        small = x86::sreg16(app->getMemory<x86::reg16>(r.edx + 6)) < 0x40;
    }
    if (small)
        r.esi = 0x40;
    else
    {
        r.eax = m.word(0x56727c);
        flags.compare16(app->getMemory<x86::reg16>(r.eax + 4), 0x80);
        if (x86::sreg16(app->getMemory<x86::reg16>(r.eax + 4)) < 0x80)
        {
            flags.compare16(app->getMemory<x86::reg16>(r.eax + 6), 0x80);
            if (x86::sreg16(app->getMemory<x86::reg16>(r.eax + 6)) < 0x80)
                r.esi = 0x80;
        }
    }
    r.eax = (r.esi * r.esi) << 2;
    r.edi = r.eax + 0x14;
    r.ebx = 0;
    r.eax = 0x540b90;
    r.edx = r.edi;
    if (!callTo(0x4e1620))
        return;
    m.word(ebp - 0x28, r.eax);
    m.word(ebp - 0x2c, r.eax);
    r.edx = r.edi;
    r.ebx = 0;
    r.eax = 0x540b90;
    r.ecx = 0;
    if (!callTo(0x4e1620))
        return;
    r.edi = r.eax;
    r.eax = ebp - 0x34;
    r.ebx = ebp - 0x30;
    push(r.eax);
    r.edx = m.word(0x56727c);
    r.eax = m.word(ebp - 0x28);
    if (!callTo(0x4d37a0))
        return;
    r.eax = m.word(ebp - 0x34);
    if (!callTo(0x4d18b0))
        return;
    /* The rows copied, `size` bytes a pixel: from `from` to `to` (registers
     * as the generated code has them), each min(size, width) pixels, the
     * source or the destination stepped past the rest; `rows`, `columns`
     * its counters' slots. */
    auto copy = [&](x86::reg32& from, x86::reg32& to, x86::reg32 size, x86::reg32 rows, x86::reg32 columns) {
        m.word(ebp + rows, 0);
        r.ecx = 0;
        for (;;)
        {
            r.ecx = height();
            flags.compare(r.esi, r.ecx);
            if (x86::sreg32(r.esi) < x86::sreg32(r.ecx))
                r.ecx = r.esi;
            flags.compare(r.ecx, m.word(ebp + rows));
            if (x86::sreg32(r.ecx) <= x86::sreg32(m.word(ebp + rows)))
                break;
            r.ecx = 0;
            for (;;)
            {
                m.word(ebp + columns, r.ecx);
                r.ecx = width();
                flags.compare(r.esi, r.ecx);
                if (x86::sreg32(r.esi) < x86::sreg32(r.ecx))
                    r.ecx = r.esi;
                if (size == 4)
                {
                    r.ebx = m.word(ebp + columns);
                    flags.compare(r.ecx, r.ebx);
                    if (x86::sreg32(r.ecx) <= x86::sreg32(r.ebx))
                        break;
                    r.ecx = m.word(from);
                    from += 4;
                    m.word(to, r.ecx);
                    r.ecx = r.ebx + 1;
                    to += 4;
                    continue;
                }
                flags.compare(r.ecx, m.word(ebp + columns));
                if (x86::sreg32(r.ecx) <= x86::sreg32(m.word(ebp + columns)))
                    break;
                r.ecx = m.word(ebp + columns);
                if (size == 1)
                {
                    ++to;
                    setByte(r.ebx, 0, m.byte(from));
                    ++from;
                    ++r.ecx;
                    m.byte(to - 1, x86::reg8(r.ebx));
                }
                else
                {
                    to += 2;
                    r.ebx = (r.ebx & 0xffff0000) | app->getMemory<x86::reg16>(from);
                    from += 2;
                    ++r.ecx;
                    app->getMemory<x86::reg16>(to - 2) = x86::reg16(r.ebx);
                }
            }
            r.ecx = width();
            m.word(ebp + (size == 1 ? 0xfffffff4u : size == 2 ? 0xfffffffcu : 0xfffffff8u), r.ecx);
            flags.compare(r.esi, r.ecx);
            if (x86::sreg32(r.esi) > x86::sreg32(r.ecx))
            {
                r.ebx = r.ecx;
                r.ecx = (r.esi - r.ebx) * size;
                to += r.ecx;
            }
            else if (x86::sreg32(r.esi) < x86::sreg32(r.ecx))
            {
                r.ecx = (r.ecx - r.esi) * size;
                from += r.ecx;
            }
            m.word(ebp + rows, m.word(ebp + rows) + 1);
        }
    };
    flags.compare(r.eax, 0x10);
    if (r.eax == 0x10)
    {
        r.eax = m.word(ebp - 0x2c) + 0x10;
        r.edx = r.edi + 0x10;
        r.ecx = 0;
        m.word(ebp - 0x14, 0);
        copy(r.eax, r.edx, 2, 0xffffffec, 0xffffffe0);
    }
    else if (r.eax < 0x10)
    {
        flags.compare(r.eax, 8);
        if (r.eax == 8)
        {
            r.edx = m.word(ebp - 0x28) + 0x10;
            r.eax = r.edi + 0x10;
            copy(r.edx, r.eax, 1, 0xffffffe8, 0xffffffe4);
        }
    }
    else
    {
        flags.compare(r.eax, 0x20);
        if (r.eax == 0x20)
        {
            r.edx = m.word(ebp - 0x2c) + 0x10;
            r.eax = r.edi + 0x10;
            copy(r.edx, r.eax, 4, 0xfffffff0, 0xffffffdc);
        }
    }
    // the texture: its size, THRASH_talloc, THRASH_tupdate
    r.edx = m.word(0x840184);
    r.eax = r.edx * 11;
    push(0);
    app->getMemory<x86::reg16>(r.eax * 4 + 0x83f6aa) = x86::reg16(r.esi);
    push(0);
    app->getMemory<x86::reg16>(r.eax * 4 + 0x83f6a8) = x86::reg16(r.esi);
    r.eax = m.word(ebp - 0x34);
    push(r.eax);
    push(r.esi);
    push(r.esi);
    if (!callTo(m.word(0x9ef968)))
        return;
    r.edx = m.word(0x840184);
    r.ecx = r.eax;
    r.eax = r.edx * 12;
    push(0);
    r.eax -= r.edx;
    r.edx = r.edi + 0x10;
    push(r.edx);
    r.edx = r.ecx;
    push(r.edx);
    m.word(r.eax * 4 + 0x83f6ac, r.ecx);
    if (!callTo(m.word(0x9ef964)))
        return;
    r.eax = m.word(ebp - 0x2c);
    if (!callTo(0x4e1890))
        return;
    r.eax = r.edi;
    if (!callTo(0x4e1890))
        return;
    leave();
}

/* sub_4d3f50: a texture of ecx x ecx from the picture at eax (sub_4d37a0
 * decodes it, edx its palette, its format to [esp+4]): THRASH_talloc of its
 * width and height (the format 2 with flag 4 and the palette [0x563a84]),
 * THRASH_tupdate; the handle to eax, and at ebx the texture's corners inset
 * by half a texel ([0x549438] times its size): s, t at +8..+0x24, +0x28 0.
 * ret 4. */
void squareTextureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x20
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 0x20;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.ebx, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    m.word(ebp - 0x14, r.eax);
    m.word(ebp - 0x10, r.edx);
    m.word(ebp - 4, r.ecx);
    if (!callTo(m.word(0x9ef980)))
        return;
    flags.logic(r.eax);  // test eax, eax: its flags left for sub_4e1620
    r.eax = (m.word(ebp - 4) * m.word(ebp - 4)) << 2;
    r.edx = r.eax + 0x14;
    r.ebx = 0;
    r.eax = 0x549434;
    r.ecx = m.word(ebp - 4);
    if (!callTo(0x4e1620))
        return;
    r.edi = r.eax;
    m.word(ebp - 8, r.eax);
    r.eax = ebp + 0x10;
    r.ebx = m.word(ebp - 0x10);
    push(r.eax);
    r.edx = m.word(ebp - 0x14);
    r.eax = r.edi;
    if (!callTo(0x4d37a0))
        return;
    r.eax = r.edi + 0x10;
    r.ecx = m.word(ebp + 0x10);
    m.word(ebp - 0xc, r.eax);
    flags.compare(r.ecx, 2);
    const bool paletted = r.ecx == 2;
    push(0);
    push(paletted ? 4 : 0);
    push(r.ecx);
    push(x86::reg32(x86::sreg32(m.word(r.edi + 4)) >> 16));
    r.eax = m.word(r.edi + 2);
    if (paletted)
        flags.sar(r.eax, 16);
    r.eax = x86::reg32(x86::sreg32(r.eax) >> 16);
    push(r.eax);
    if (!callTo(m.word(0x9ef968)))
        return;
    if (paletted)
    {
        r.edi = m.word(0x563a84);
        push(r.edi);
        r.edx = m.word(ebp - 0xc);
        push(r.edx);
    }
    else
    {
        push(0);
        r.edi = m.word(ebp - 0xc);
        push(r.edi);
    }
    push(r.eax);
    r.ebx = r.eax;
    if (!callTo(m.word(0x9ef964)))
        return;
    // the corners half a texel in
    r.eax = m.word(ebp - 8);
    const double half = m.load(0x549438);
    const double s = M::div(1.0, M::mul(double(x86::sreg16(app->getMemory<x86::reg16>(r.eax + 4))), half));
    const double t = M::div(1.0, M::mul(half, double(x86::sreg16(app->getMemory<x86::reg16>(r.eax + 6)))));
    m.store(ebp - 0x20, s);
    m.store(ebp - 0x1c, t);
    r.eax = m.word(ebp - 0x20);
    const double s1 = M::sub(1.0, m.load(ebp - 0x20));
    m.word(r.esi + 8, r.eax);
    r.eax = m.word(ebp - 0x1c);
    m.store(ebp - 0x18, s1);
    m.word(r.esi + 0xc, r.eax);
    r.eax = m.word(ebp - 0x18);
    m.word(r.esi + 0x10, r.eax);
    r.eax = m.word(ebp - 0x1c);
    const double t1 = M::sub(1.0, m.load(ebp - 0x1c));
    m.word(r.esi + 0x14, r.eax);
    r.eax = m.word(ebp - 0x18);
    m.store(ebp - 0x18, t1);
    m.word(r.esi + 0x18, r.eax);
    r.eax = m.word(ebp - 0x18);
    m.word(r.esi + 0x1c, r.eax);
    r.eax = m.word(ebp - 0x20);
    m.word(r.esi + 0x20, r.eax);
    r.eax = m.word(ebp - 0x18);
    m.word(r.esi + 0x24, r.eax);
    r.eax = m.word(ebp - 8);
    m.byte(r.esi + 0x28, 0);
    if (!callTo(0x4e1890))
        return;
    r.eax = r.ebx;
    flags.store(cpu);
    cpu.eax = r.eax;
    cpu.ebx = r.ebx;
    cpu.ecx = r.ecx;
    cpu.edx = r.edx;
    cpu.esi = saved[0];
    cpu.edi = saved[1];
    cpu.ebp = saved[2];
    cpu.esp = entry + 8;
}

/* sub_4d41a0: the paletted picture at eax as a texture into the record at
 * edx (+0, +2 its size, +4 the THRASH handle, +8..+0x24 its corners' s, t,
 * +0x28 flags): resized to a power of two (sub_4d4120, sub_4d1ce0), its
 * 1555 palette (sub_4fd1c0) to ARGB at palette [0x563a98] of 0x8401a0 (400
 * of 1 KB, round robin); then with sub_4f24e0's name in sub_4e0820's list
 * ("0x54943c") expanded to 16 bits (sub_4d3380, format 7), else paletted
 * (format 2 with flag 4). */
void paletteTextureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp };
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    const x86::reg32 ebp = entry - 20;
    x86::reg32 esp = ebp - 0x1c;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.edx, cpu.eax };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto word16 = [&](x86::reg32 at) { return app->getMemory<x86::reg16>(at); };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.edx = r.edx;
        cpu.ebx = saved[0];
        cpu.ecx = saved[1];
        cpu.esi = saved[2];
        cpu.edi = saved[3];
        cpu.ebp = saved[4];
        cpu.esp = entry + 4;
    };
    // the record: the picture's size, its corners 0..1
    r.eax = (r.eax & 0xffff0000) | word16(r.edi + 4);
    app->getMemory<x86::reg16>(r.edx) = x86::reg16(r.eax);
    r.eax = (r.eax & 0xffff0000) | word16(r.edi + 6);
    m.byte(r.edx + 0x28, 0);
    static const x86::reg32 corners[8] = { 0, 0, 0x3f800000, 0, 0x3f800000, 0x3f800000, 0, 0x3f800000 };
    for (x86::reg32 k = 0; k < 8; ++k)
        m.word(r.edx + 8 + 4 * k, corners[k]);
    m.byte(r.edx + 0x29, 0);
    app->getMemory<x86::reg16>(r.edx + 2) = x86::reg16(r.eax);
    r.edx = x86::reg32(x86::sreg32(m.word(r.edi + 2)) >> 16);
    r.eax = x86::reg32(x86::sreg32(m.word(r.edi + 4)) >> 16);
    if (!callTo(0x4d4120))
        return;
    // resized to that
    r.edx = ebp - 0x14;
    m.word(ebp - 0x10, r.eax);
    push(r.edx);
    r.ecx = m.word(r.edi + 4);
    r.ebx = m.word(r.edi + 2);
    push(r.eax);
    r.ecx = x86::reg32(x86::sreg32(r.ecx) >> 16);
    push(r.eax);
    r.eax = r.edi + 0x10;
    r.ebx = x86::reg32(x86::sreg32(r.ebx) >> 16);
    r.edx = r.eax;
    if (!callTo(0x4d1ce0))
        return;
    r.eax = m.word(ebp - 0x10);
    app->getMemory<x86::reg16>(r.esi) = x86::reg16(r.eax);
    app->getMemory<x86::reg16>(r.esi + 2) = x86::reg16(r.eax);
    r.eax = r.edi;
    if (!callTo(0x4fd1c0))
        return;
    m.word(ebp - 8, r.eax + 0x10);
    // the palette: 1555 to ARGB, alpha 0 or 0xff
    r.eax = 0;
    flags.logic(0);
    for (;;)
    {
        r.ecx = m.word(ebp - 8);
        r.edx = r.eax * 2 + r.ecx;
        flags.logic8(m.byte(r.edx + 1) & 0x80);
        r.ebx = (m.byte(r.edx + 1) & 0x80) ? 0xff : 0;
        r.ecx = m.word(ebp - 8);
        r.edx = r.eax * 2 + r.ecx;
        r.ecx = (r.ecx & 0xffff0000) | (word16(r.edx) & 0x7c00);
        m.word(ebp - 4, r.ecx);
        r.ecx = x86::reg32(x86::sreg32(x86::reg32(word16(ebp - 4))) >> 7);
        m.word(ebp - 0x1c, r.ecx);
        r.ecx = word16(r.edx) & 0x3e0;
        m.word(ebp - 4, r.ecx);
        r.edx = (r.edx & 0xffff0000) | (word16(r.edx) & 0x1f);
        r.ecx = x86::reg32(x86::sreg32(x86::reg32(word16(ebp - 4))) >> 2);
        m.word(ebp - 0x18, r.ecx);
        r.ecx = m.word(ebp - 0x1c);
        r.ebx = (r.ebx << 24) | (r.ecx << 16);
        r.ecx = m.word(ebp - 0x18);
        r.edx &= 0xffff;
        r.ecx = (r.ecx << 8) | r.ebx | (r.edx << 3);
        r.edx = (m.word(0x563a98) << 10) + r.eax * 4;
        r.ebx = r.eax * 4;
        ++r.eax;
        m.word(r.edx + 0x8401a0, r.ecx);
        flags.compare(r.eax, 0x100);
        if (x86::sreg32(r.eax) >= 0x100)
            break;
    }
    r.eax = r.edi;
    if (!callTo(0x4f24e0))
        return;
    flags.logic(r.eax);
    bool expand = r.eax != 0;
    if (expand)
    {
        r.edx = 0x54943c;
        if (!callTo(0x4e0820))
            return;
        flags.logic(r.eax);
        expand = r.eax != 0;
    }
    if (expand)
    {
        // expanded to 16 bits through the palette
        r.edx = word16(r.esi + 2);
        r.eax = word16(r.esi) * r.edx;
        m.byte(r.esi + 0x28, m.byte(r.esi + 0x28) | 6);
        r.edx = r.eax * 2;
        r.ebx = 0x10;
        r.eax = 0x549434;
        if (!callTo(0x4e1620))
            return;
        m.word(ebp - 0xc, r.eax);
        r.eax = m.word(0x563a98) << 10;
        r.ebx = 0x8401a0 + r.eax;
        r.edx = m.word(ebp - 0xc);
        r.eax = r.edi;
        if (!callTo(0x4d3380))
            return;
        push(0);
        push(0);
        push(7);
        r.eax = word16(r.esi + 2);
        push(r.eax);
        flags.logic(0);
        r.eax = word16(r.esi);
        push(r.eax);
        if (!callTo(m.word(0x9ef968)))
            return;
        push(0);
        r.edi = m.word(ebp - 0xc);
        push(r.edi);
        push(r.eax);
        m.word(r.esi + 4, r.eax);
        if (!callTo(m.word(0x9ef964)))
            return;
        r.eax = r.edi;
        if (!callTo(0x4e1890))
            return;
        return leave();
    }
    // paletted
    push(0);
    push(4);
    push(2);
    r.eax = word16(r.esi + 2);
    push(r.eax);
    setByte(r.ecx, 0, m.byte(r.esi + 0x28) | 0x10);
    r.eax = word16(r.esi);
    push(r.eax);
    m.byte(r.esi + 0x28, x86::reg8(r.ecx));
    if (!callTo(m.word(0x9ef968)))
        return;
    m.word(r.esi + 4, r.eax);
    r.eax = (m.word(0x563a98) << 10) + 0x8401a0;
    push(r.eax);
    r.eax = r.edi + 0x10;
    push(r.eax);
    r.edx = m.word(r.esi + 4);
    push(r.edx);
    if (!callTo(m.word(0x9ef964)))
        return;
    r.ecx = m.word(0x563a98) + 1;
    m.word(0x563a98, r.ecx);
    flags.compare(r.ecx, 0x190);
    if (x86::sreg32(r.ecx) >= 0x190)
    {
        r.esi = 0;
        m.word(0x563a98, 0);
    }
    leave();
}

/* sub_4d4580: a piece of the picture at eax -- from x ecx, y [esp+4], at
 * most 256 pixels each way, ebx its format (sub_4d18b0 its bits) -- as a
 * texture of 256 x 256 for the record at edx: the rows copied into a new
 * buffer 256 pixels wide; a piece narrower or shorter than 256 is also laid
 * out as bars at 0x792090 ([0x792310] of them, 36 bytes each, sub_4d4440 with
 * [esp+8]) where the cabin's strip has room ([0x8b441c] up to [0x8b4418]);
 * then THRASH_talloc (format 2 with the palette [0x563a84]), THRASH_tupdate
 * and the record's corners (+8..+0x24 by [0x549450], [0x549454], +0x28 4).
 * eax 1 when no texture could be had, else 0.  ret 8. */
void pieceTextureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 0x48;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = r.ebx;
        cpu.ecx = r.ecx;
        cpu.edx = r.edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 12;
    };
    // fcomp; fnstsw ax; sahf
    auto sahfCompare = [&](double a, double b) {
        flags.store(cpu);
        cpu.fpu.compare(x86::Float(a), x86::Float(b));
        r.eax = (r.eax & 0xffff0000) | cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | ((cpu.fpu.status.word >> 8) & 0xd7));
        flags.load(cpu);
    };
    // n / 8 toward 0 as sar, shl (its flags), sbb, sar leave it; the flags shl's
    auto eighth = [&](x86::reg32 n) {
        const x86::reg32 sign = x86::reg32(x86::sreg32(n) >> 31);
        flags.shl(sign, 3);
        const x86::reg32 carry = (sign >> 29) & 1;
        return x86::reg32(x86::sreg32(n - (sign << 3) - carry) >> 3);
    };
    m.word(ebp - 0x20, r.eax);
    m.word(ebp - 0xc, r.edx);
    m.word(ebp - 0x24, r.ebx);
    r.eax = r.ebx;
    if (!callTo(0x4d18b0))
        return;
    r.ebx = r.eax;
    m.word(ebp - 0x1c, r.eax);
    // where the piece starts
    r.eax = m.word(ebp - 0x20);
    r.esi = x86::reg32(x86::sreg32(m.word(r.eax + 2)) >> 16);
    r.edx = m.word(ebp + 0x10) * r.esi * r.ebx;
    r.eax = eighth(r.edx);
    r.edx = m.word(ebp - 0x20) + 0x10 + r.eax;
    m.word(ebp - 8, r.edx);
    r.edx = r.ecx * r.ebx;
    r.eax = eighth(r.edx);
    r.edx = m.word(ebp - 8) + r.eax;
    r.eax = r.esi - r.ecx;
    m.word(ebp - 0x28, r.edx);
    flags.compare(r.eax, 0x100);
    if (x86::sreg32(r.eax) > 0x100)
        r.eax = 0x100;
    m.word(ebp - 8, r.eax);
    // its size, to 256
    r.eax = m.word(ebp - 0x20);
    r.eax = x86::reg32(x86::sreg32(m.word(r.eax + 4)) >> 16);
    r.edx = m.word(ebp + 0x10);
    const double across = fild32(m, ebp - 8);
    r.eax -= r.edx;
    m.store(ebp - 0x10, across);
    flags.compare(r.eax, 0x100);
    if (x86::sreg32(r.eax) > 0x100)
        r.eax = 0x100;
    m.word(ebp - 8, r.eax);
    r.eax = m.word(ebp - 0x1c);
    m.word(ebp - 4, r.eax);
    const double rowBytes = M::mul(M::mul(fild32(m, ebp - 4), m.load(ebp - 0x10)), m.load(0x549448));
    r.edx = 0x40000;
    r.ebx = 0;
    r.ecx = 0;
    flags.logic(0);
    m.store(ebp - 0x18, fild32(m, ebp - 8));
    m.word(ebp - 0x2c, r.ecx);
    r.eax = 0x549434;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(rowBytes);
    if (!callTo(0x4dfd56))
        return;
    m.word(ebp - 0x48, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
    cpu.fpu.count -= 1;
    if (!callTo(0x4e1620))
        return;
    m.word(ebp - 0x14, r.eax);
    r.ebx = r.eax;
    // the rows: rep movsd, rep movsb, the destination 256 pixels on
    for (;;)
    {
        r.eax = m.word(ebp - 0x2c);
        m.word(ebp - 4, r.eax);
        sahfCompare(fild32(m, ebp - 4), m.load(ebp - 0x18));
        if (!cpu.flags.cf)
            break;
        r.ecx = m.word(ebp - 0x48);
        r.esi = m.word(ebp - 0x28);
        r.edx = m.word(ebp - 0x1c) << 8;
        r.edi = r.ebx;
        esp -= 4;
        m.word(esp, r.edi);
        r.eax = r.ecx;
        r.ecx >>= 2;
        for (; r.ecx; --r.ecx, r.edi += 4, r.esi += 4)
            m.word(r.edi, m.word(r.esi));
        setByte(r.ecx, 0, x86::reg8(r.eax) & 3);
        flags.logic8(x86::reg8(r.eax) & 3);
        for (; r.ecx; --r.ecx, ++r.edi, ++r.esi)
            m.byte(r.edi, m.byte(r.esi));
        r.edi = m.word(esp);
        esp += 4;
        r.eax = eighth(r.edx);
        r.edx = m.word(ebp - 0x20);
        r.edx = x86::reg32(x86::sreg32(m.word(r.edx + 2)) >> 16);
        r.esi = m.word(ebp - 0x1c);
        r.edx *= r.esi;
        r.ebx += r.eax;
        r.eax = eighth(r.edx);
        r.edx = x86::reg32(x86::sreg32(r.edx) >> 31) << 3;
        r.edi = m.word(ebp - 0x28);
        flags.add(r.edi, r.eax);
        r.edi += r.eax;
        r.eax = m.word(ebp - 0x2c);
        flags.inc(r.eax);
        ++r.eax;
        m.word(ebp - 0x28, r.edi);
        m.word(ebp - 0x2c, r.eax);
    }
    // a piece short of 256 either way: bars in the cabin's strip
    const double K = m.load(0x54944c);
    bool bars = false;
    sahfCompare(m.load(ebp - 0x10), K);
    if (cpu.flags.cf)
        bars = true;
    else
    {
        sahfCompare(m.load(ebp - 0x18), K);
        bars = cpu.flags.cf;
    }
    if (bars)
    {
        r.edx = m.word(0x8b4418);
        flags.compare(r.edx, m.word(0x8b441c));
        bars = x86::sreg32(r.edx) > x86::sreg32(m.word(0x8b441c));
    }
    if (bars)
    {
        // the record of bar [0x792310]: its texture, s, t, its size
        sahfCompare(m.load(ebp - 0x10), K);
        bool narrow = cpu.flags.cf;
        if (narrow)
        {
            sahfCompare(m.load(ebp - 0x18), K);
            narrow = cpu.flags.cf;
        }
        if (narrow)
        {
            // narrow and short: beside the strip, as wide as fits
            const double width = m.load(ebp - 0x10);
            r.edx = m.word(0x792310);
            r.esi = m.word(0x8b441c);
            cpu.fpu.count += 3;
            cpu.fpu.st(0) = x86::Float(width);
            cpu.fpu.st(1) = x86::Float(K);
            cpu.fpu.st(2) = x86::Float(width);
            if (!callTo(0x4dfd56))
                return;
            r.eax = r.edx * 8;
            r.ecx = 0;
            r.edx += r.eax;
            r.eax = m.word(ebp - 0xc);
            r.edx <<= 2;
            m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
            m.word(r.edx + 0x792090, r.eax);
            r.eax = m.word(ebp - 8);
            const double rest = M::sub(double(cpu.fpu.st(1)), double(cpu.fpu.st(2)));
            cpu.fpu.count -= 3;
            m.word(r.edx + 0x792098, r.eax);
            r.eax = m.word(0x8b441c);
            m.store(ebp - 0x44, rest);
            m.word(r.edx + 0x7920a0, r.eax);
            r.eax = m.word(0x8b4418);
            flags.compare(r.eax, r.esi);
            r.eax -= r.esi;
            m.word(r.edx + 0x79209c, r.ecx);
            m.word(ebp - 4, r.eax);
            m.word(r.edx + 0x7920a4, r.ecx);
            m.store(ebp - 0x38, fild32(m, ebp - 4));
            sahfCompare(m.load(ebp - 0x44), m.load(ebp - 0x38));
            r.eax = m.word(ebp + (cpu.flags.cf ? 0xffffffbcu : 0xffffffc8u));
            m.word(ebp - 0x30, r.eax);
            r.edx = m.word(0x792310);
            cpu.fpu.count += 1;
            cpu.fpu.st(0) = x86::Float(m.load(ebp - 0x30));
            if (!callTo(0x4dfd56))
                return;
            r.eax = r.edx * 8;
            m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
            cpu.fpu.count -= 1;
            r.edx = (r.edx + r.eax) << 2;
            r.eax = m.word(ebp - 8);
            r.edi = 0x100;
            m.word(r.edx + 0x7920a8, r.eax);
            m.word(r.edx + 0x7920ac, r.edi);
            r.ecx = m.word(ebp - 0x1c);
            m.word(r.edx + 0x792094, r.edi);
            r.eax = 0;
            flags.logic(0);
            r.ebx = m.word(ebp - 0x14);
            m.word(r.edx + 0x7920b0, r.eax);
            r.edx = m.word(ebp + 0x14);
            r.eax = m.word(0x792310);
            if (!callTo(0x4d4440))
                return;
            const double left = M::sub(K, m.load(ebp - 0x10));
            r.ebx = m.word(0x792310);
            flags.inc(r.ebx);
            ++r.ebx;
            const double strip = fild32(m, 0x8b441c);
            m.word(0x792310, r.ebx);
            cpu.fpu.count += 2;
            cpu.fpu.st(0) = x86::Float(M::add(left, strip));
            cpu.fpu.st(1) = x86::Float(strip);
        }
        else
        {
            const double width = m.load(ebp - 0x10);
            r.eax = m.word(0x792310);
            r.ecx = m.word(0x8b4418);
            r.edx = r.eax * 9;
            r.esi = m.word(0x8b441c);
            r.ecx -= r.esi;
            flags.shl(r.edx, 2);
            r.edx <<= 2;
            sahfCompare(width, K);
            if (cpu.flags.cf)
            {
                // narrow, tall: its whole height
                r.eax = m.word(ebp - 0xc);
                cpu.fpu.count += 3;
                cpu.fpu.st(0) = x86::Float(width);
                cpu.fpu.st(1) = x86::Float(K);
                cpu.fpu.st(2) = x86::Float(width);
                if (!callTo(0x4dfd56))
                    return;
                m.word(ebp - 4, r.ecx);
                r.edi = 0;
                m.word(r.edx + 0x7920a0, r.esi);
                const double room = fild32(m, ebp - 4);
                m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                m.word(r.edx + 0x792090, r.eax);
                m.store(ebp - 0x40, room);
                m.word(r.edx + 0x79209c, r.edi);
                r.eax = m.word(ebp - 8);
                const double rest = M::sub(double(cpu.fpu.st(1)), double(cpu.fpu.st(2)));
                cpu.fpu.count -= 3;
                m.word(r.edx + 0x792098, r.eax);
                r.eax = r.esi;
                m.store(ebp - 0x34, rest);
                r.eax = 0;
                flags.logic(0);
                m.word(r.edx + 0x7920a4, r.eax);
                sahfCompare(m.load(ebp - 0x34), m.load(ebp - 0x40));
                r.eax = m.word(ebp + (cpu.flags.cf ? 0xffffffccu : 0xffffffc0u));
                m.word(ebp - 0x3c, r.eax);
                r.edx = m.word(0x792310);
                cpu.fpu.count += 1;
                cpu.fpu.st(0) = x86::Float(m.load(ebp - 0x3c));
                if (!callTo(0x4dfd56))
                    return;
                r.eax = r.edx * 8;
                m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                cpu.fpu.count -= 1;
                r.edx = (r.edx + r.eax) << 2;
                r.eax = m.word(ebp - 8);
                r.ecx = 0x100;
                m.word(r.edx + 0x7920a8, r.eax);
                r.ebx = 0;
                flags.logic(0);
                m.word(r.edx + 0x7920ac, r.ecx);
                m.word(r.edx + 0x7920b0, r.ebx);
                m.word(r.edx + 0x792094, r.ecx);
                r.ecx = m.word(ebp - 0x1c);
                r.ebx = m.word(ebp - 0x14);
                r.edx = m.word(ebp + 0x14);
                r.eax = m.word(0x792310);
                if (!callTo(0x4d4440))
                    return;
                const double left = M::sub(K, m.load(ebp - 0x10));
                r.edi = m.word(0x792310);
                flags.inc(r.edi);
                ++r.edi;
                const double strip = fild32(m, 0x8b441c);
                m.word(0x792310, r.edi);
                cpu.fpu.count += 2;
                cpu.fpu.st(0) = x86::Float(M::add(left, strip));
                cpu.fpu.st(1) = x86::Float(strip);
            }
            else
            {
                // wide, short: lying on its side
                r.eax = m.word(ebp - 0xc);
                m.word(r.edx + 0x7920a0, r.esi);
                r.ebx = 0;
                m.word(r.edx + 0x792090, r.eax);
                cpu.fpu.count += 1;
                cpu.fpu.st(0) = x86::Float(m.load(ebp - 0x18));
                if (!callTo(0x4dfd56))
                    return;
                m.word(r.edx + 0x7920a4, r.ebx);
                r.eax = 0;
                m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                cpu.fpu.count -= 1;
                m.word(r.edx + 0x792098, r.eax);
                r.eax = m.word(ebp - 8);
                m.word(r.edx + 0x79209c, r.eax);
                flags.compare(r.ecx, 0x100);
                if (x86::sreg32(r.ecx) > 0x100)
                    r.ecx = 0x100;
                r.esi = 1;
                r.edi = 0x100;
                r.ebx = m.word(ebp - 0x14);
                r.edx = m.word(0x792310);
                r.eax = r.edx * 8;
                m.store(ebp - 4, M::sub(K, m.load(ebp - 0x18)));
                r.edx = (r.edx + r.eax) << 2;
                cpu.fpu.count += 1;
                cpu.fpu.st(0) = x86::Float(m.load(ebp - 4));
                if (!callTo(0x4dfd56))
                    return;
                m.word(r.edx + 0x7920a8, r.ecx);
                m.word(ebp - 8, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
                cpu.fpu.count -= 1;
                m.word(r.edx + 0x7920b0, r.esi);
                r.eax = m.word(ebp - 8);
                m.word(r.edx + 0x792094, r.edi);
                r.ecx = m.word(ebp - 0x1c);
                m.word(r.edx + 0x7920ac, r.eax);
                r.edx = m.word(ebp + 0x14);
                r.eax = m.word(0x792310);
                if (!callTo(0x4d4440))
                    return;
                const double strip = fild32(m, 0x8b441c);
                r.eax = m.word(0x792310) + r.esi;
                const double next = M::add(strip, m.load(ebp - 4));
                m.word(0x792310, r.eax);
                cpu.fpu.count += 2;
                cpu.fpu.st(0) = x86::Float(next);
                cpu.fpu.st(1) = x86::Float(strip);
            }
        }
        // fstp st(1): the strip's new start, through sub_4dfd56
        const x86::Float top = cpu.fpu.st(0);
        cpu.fpu.count -= 1;
        cpu.fpu.st(0) = top;
        if (!callTo(0x4dfd56))
            return;
        m.word(0x8b441c, x86::reg32(cpu.fpu.toInteger<x86::sreg32>(cpu.fpu.st(0))));
        cpu.fpu.count -= 1;
    }
    // the texture
    r.edx = m.word(ebp - 0x24);
    flags.compare(r.edx, 2);
    const bool paletted = r.edx == 2;
    push(0);
    push(paletted ? 4 : 0);
    push(r.edx);
    push(0x100);
    push(0x100);
    if (!callTo(m.word(0x9ef968)))
        return;
    r.edx = m.word(ebp - 0xc);
    m.word(r.edx + 4, r.eax);
    flags.logic(r.eax);
    if (!r.eax)
    {
        r.eax = m.word(ebp - 0x14);
        if (!callTo(0x4e1890))
            return;
        r.eax = 1;
        return leave();
    }
    if (paletted)
    {
        r.esi = m.word(0x563a84);
        push(r.esi);
        r.edi = m.word(ebp - 0x14);
        push(r.edi);
    }
    else
    {
        push(0);
        r.esi = m.word(ebp - 0x14);
        push(r.esi);
    }
    push(r.eax);
    if (!callTo(m.word(0x9ef964)))
        return;
    // its corners: half a texel in, out to the piece's size over 256
    const double scale = m.load(0x549450);
    const double s1 = M::mul(m.load(ebp - 0x10), scale);
    r.eax = m.word(ebp - 0xc);
    const double h = m.load(ebp - 0x18);
    m.word(r.eax + 8, 0x3b000000);
    r.edx = m.word(ebp - 0xc);
    m.word(r.eax + 0xc, 0x3b000000);
    const double t1 = M::mul(scale, h);
    m.word(r.edx + 0x14, 0x3b000000);
    const double inset = m.load(0x549454);
    m.word(r.edx + 0x20, 0x3b000000);
    m.store(ebp - 8, M::add(s1, inset));
    r.eax = m.word(ebp - 8);
    m.word(r.edx + 0x10, r.eax);
    m.store(ebp - 8, M::add(t1, inset));
    m.word(r.edx + 0x18, r.eax);
    r.eax = m.word(ebp - 8);
    m.word(r.edx + 0x1c, r.eax);
    m.word(r.edx + 0x24, r.eax);
    r.eax = m.word(ebp - 0x14);
    m.byte(r.edx + 0x28, 4);
    if (!callTo(0x4e1890))
        return;
    r.eax = 0;
    leave();
}

/* sub_4d4ae0: a picture as cabin textures -- eax a compressed picture (or 0
 * for edx, uncompressed, with ebx), ecx the records (44 bytes each) and
 * [esp+4] its format, [esp+8] where its piece of 256 x 256 goes.  The picture
 * at edx/ebx (sub_4ef1e0 unpacks both) gives the cabin's size ([0x792300..
 * 0x79230c]); pieces of 256 x 256 become textures through sub_4d4580, and the
 * rest of the cabin's strip ([0x8b441c] up to [0x8b4418], [0x8b4414] high) is
 * packed, as bars at 0x792090 (sub_4d4440), into one square texture of the
 * least power of two that holds it: THRASH_talloc, THRASH_tupdate.  eax the
 * number of records used, 0 when a texture could not be had.  ret 8. */
void cabinTexturesNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.esi, cpu.edi, cpu.ebp };
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    const x86::reg32 ebp = entry - 12;
    x86::reg32 esp = ebp - 0x48;
    IntegerFlags flags;
    Registers r{ cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi };
    auto callTo = [&](x86::reg32 target) {
        if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
            return false;
        esp = cpu.esp;
        return true;
    };
    auto push = [&](x86::reg32 value) {
        esp -= 4;
        m.word(esp, value);
    };
    auto leave = [&]() {
        flags.store(cpu);
        cpu.eax = r.eax;
        cpu.ebx = r.ebx;
        cpu.ecx = r.ecx;
        cpu.edx = r.edx;
        cpu.esi = saved[0];
        cpu.edi = saved[1];
        cpu.ebp = saved[2];
        cpu.esp = entry + 12;
    };
    auto half = [](x86::reg32 v) { return x86::reg32(x86::sreg32(v) >> 16); };
    auto less = [](x86::reg32 a, x86::reg32 b) { return x86::sreg32(a) < x86::sreg32(b); };
    // the bar [0x792310]'s record: its texture's record, s, t, x, y
    auto bar = [&](x86::reg32 x, x86::reg32 y) {
        const x86::reg32 record = m.word(ebp - 0x3c) + m.word(ebp - 0x10) * 44;
        const x86::reg32 at = m.word(0x792310) * 36;
        m.word(at + 0x792090, record);
        m.word(at + 0x792098, m.word(ebp - 4));
        m.word(at + 0x79209c, m.word(ebp - 0xc));
        m.word(at + 0x7920a0, x);
        m.word(at + 0x7920a4, y);
        r.eax = at / 4;
        r.ecx = record;
    };
    auto drop = [&]() {
        r.eax = m.word(ebp - 8);
        if (!callTo(0x4e1890))
            return false;
        r.eax = r.edi;
        return callTo(0x4e1890);
    };
    r.esi = r.eax;
    m.word(ebp - 0x3c, r.ecx);
    r.ecx = m.word(ebp + 0x14);
    r.edi = 0;
    m.word(ebp - 0x10, r.edi);
    flags.logic(r.eax);
    if (r.eax)
    {
        if (!callTo(0x4ef1e0))
            return;
        r.edx = r.ebx;
        m.word(r.ecx, r.eax);
        r.eax = r.esi;
        if (!callTo(0x4ef1e0))
            return;
        r.esi = r.eax;
    }
    else
    {
        r.esi = r.ebx;
        m.word(r.ecx, r.edx);
    }
    // the cabin
    m.word(0x792300, half(m.word(r.esi + 6)));
    m.word(0x79230c, half(m.word(r.esi + 8)));
    m.word(0x792304, x86::reg32(x86::sreg32(m.word(r.esi + 0xc) << 20) >> 20));
    r.eax = x86::reg32(x86::sreg32(m.word(r.esi + 0xc) << 4) >> 20);
    m.word(0x792308, r.eax);
    // buffers for both pictures, unpacked
    r.eax = m.word(r.ecx);
    r.edx = half(m.word(r.eax + 2)) * half(m.word(r.eax + 4));
    r.ebx = 0;
    r.edx = (r.edx << 2) + 0x14;
    r.eax = 0x549434;
    if (!callTo(0x4e1620))
        return;
    m.word(ebp - 0x44, r.eax);
    m.word(ebp - 0x34, r.eax);
    r.edx = half(m.word(r.esi + 2)) * half(m.word(r.esi + 4));
    r.ebx = 0;
    r.edx = (r.edx << 2) + 0x14;
    r.eax = 0x549434;
    if (!callTo(0x4e1620))
        return;
    r.ebx = ebp - 0x48;
    r.edi = r.eax;
    r.eax = ebp + 0x10;
    r.edx = m.word(r.ecx);
    push(r.eax);
    r.ecx = 0;
    r.eax = m.word(ebp - 0x44);
    if (!callTo(0x4d37a0))
        return;
    r.eax = ebp + 0x10;
    r.ebx = ebp - 0x48;
    r.edx = r.esi;
    push(r.eax);
    r.ecx = 0;
    r.eax = r.edi;
    if (!callTo(0x4d37a0))
        return;
    r.eax = 0;
    m.word(0x792310, r.eax);
    m.word(0x8b441c, r.eax);
    m.word(0x8b4418, half(m.word(r.esi + 2)));
    r.eax = half(m.word(r.esi + 4));
    r.ecx = 0;
    m.word(ebp - 0x38, r.ecx);
    m.word(0x8b4414, r.eax);
    // the pieces of 256 x 256, column by column
    for (;;)
    {
        r.eax = half(m.word(m.word(ebp - 0x34) + 2));
        r.ebx = m.word(ebp - 0x38);
        flags.compare(r.eax, r.ebx);
        if (!less(r.ebx, r.eax))
            break;
        r.esi = 0;
        m.word(ebp - 0x40, r.esi);
        for (;;)
        {
            r.eax = half(m.word(m.word(ebp - 0x34) + 4));
            r.edx = m.word(ebp - 0x40);
            flags.compare(r.eax, r.edx);
            if (!less(r.edx, r.eax))
                break;
            push(r.edi);
            r.eax = m.word(ebp - 0x10);
            push(r.edx);
            r.esi = m.word(ebp - 0x3c);
            r.ecx = m.word(ebp - 0x38);
            r.ebx = r.eax + 1;
            r.edx = r.eax * 44 + r.esi;
            r.eax = m.word(ebp - 0x34);
            m.word(ebp - 0x10, r.ebx);
            r.ebx = m.word(ebp + 0x10);
            if (!callTo(0x4d4580))
                return;
            flags.logic(r.eax);
            if (r.eax)
            {
                r.eax = r.edi;
                if (!callTo(0x4e1890))
                    return;
                r.eax = m.word(ebp - 0x34);
                if (!callTo(0x4e1890))
                    return;
                r.eax = 0;
                flags.logic(0);
                return leave();
            }
            flags.add(m.word(ebp - 0x40), 0x100);
            m.word(ebp - 0x40, m.word(ebp - 0x40) + 0x100);
        }
        flags.add(m.word(ebp - 0x38), 0x100);
        m.word(ebp - 0x38, m.word(ebp - 0x38) + 0x100);
    }
    r.eax = m.word(ebp - 0x34);
    if (!callTo(0x4e1890))
        return;
    // the strip's area, and the least square of 16 << n holding it
    r.edx = m.word(0x8b4414);
    r.ecx = m.word(0x8b4418);
    r.edx = (r.edx - 0x100) * r.ecx;
    r.esi = 0x10;
    r.ebx = 0;
    r.eax = m.word(0x8b441c);
    m.word(ebp - 4, r.ebx);
    m.word(ebp - 0xc, r.ebx);
    flags.compare(r.eax, r.ecx);
    if (less(r.eax, r.ecx))
    {
        r.ebx = m.word(0x8b441c);
        r.eax = (r.ecx - r.ebx) << 8;
        r.edx += r.eax;
    }
    for (;;)
    {
        r.eax = r.esi * r.esi;
        flags.compare(r.eax, r.edx);
        if (!less(r.eax, r.edx))
            break;
        flags.add(r.esi, r.esi);
        r.esi += r.esi;
    }
    flags.compare(r.esi, 0x100);  // its flags left for sub_4e1620
    r.edx = (r.esi * r.esi) << 2;
    r.eax = 0x549434;
    r.ebx = 0;
    if (!callTo(0x4e1620))
        return;
    m.word(ebp - 8, r.eax);
    r.eax = 0;
    r.edx = m.word(0x8b4418);
    m.word(ebp - 0x18, r.eax);
    flags.compare(r.esi, r.edx);
    r.eax = less(r.esi, r.edx) ? r.esi : r.edx;
    m.word(ebp - 0x14, r.eax);
    // the part 256 high above: bars down each column of the square
    for (;;)
    {
        r.eax = m.word(ebp - 0x18);
        r.ecx = m.word(0x8b4418);
        flags.compare(r.eax, r.ecx);
        if (!less(r.eax, r.ecx))
            break;
        r.ebx = 0x100;
        r.eax = m.word(0x8b4414) - r.ebx;
        m.word(ebp - 0x28, r.ebx);
        flags.compare(r.esi, r.eax);
        if (less(r.esi, r.eax))
            r.eax = r.esi;
        m.word(ebp - 0x24, r.eax);
        for (;;)
        {
            r.eax = m.word(ebp - 0x28);
            flags.compare(r.eax, m.word(0x8b4414));
            if (!less(r.eax, m.word(0x8b4414)))
                break;
            bar(m.word(ebp - 0x18), m.word(ebp - 0x28));
            const x86::reg32 at = r.eax * 4;
            r.ecx = m.word(ebp - 0x18);
            r.edx = m.word(0x8b4418) - r.ecx;
            r.ebx = m.word(ebp - 0x14);
            flags.compare(r.edx, r.ebx);
            if (less(r.ebx, r.edx))
                r.edx = r.ebx;
            r.ecx = m.word(0x792310);
            m.word(at + 0x7920a8, r.edx);
            r.eax = m.word(ebp - 0x28);
            r.edx = m.word(0x8b4414) - r.eax;
            r.ecx = m.word(ebp - 0x24);
            flags.compare(r.edx, r.ecx);
            if (less(r.ecx, r.edx))
                r.edx = r.ecx;
            r.ecx = m.word(0x792310);
            r.ebx = 0;
            m.word(at + 0x7920ac, r.edx);
            m.word(at + 0x7920b0, r.ebx);
            m.word(at + 0x792094, r.esi);
            r.ebx = m.word(ebp - 8);
            r.eax = r.ecx;
            r.edx = r.edi;
            r.ecx = 0x10;
            if (!callTo(0x4d4440))
                return;
            r.eax = m.word(0x792310) + 1;
            r.edx = m.word(ebp - 0x14);
            m.word(0x792310, r.eax);
            r.eax = m.word(ebp - 0x24);
            flags.compare(r.eax, r.edx);
            if (less(r.eax, r.edx))
            {
                flags.add(m.word(ebp - 0xc), r.eax);
                m.word(ebp - 0xc, m.word(ebp - 0xc) + r.eax);
            }
            else
                m.word(ebp - 4, m.word(ebp - 4) + r.edx);
            r.eax = m.word(ebp - 0x24);
            flags.add(m.word(ebp - 0x28), r.eax);
            m.word(ebp - 0x28, m.word(ebp - 0x28) + r.eax);
        }
        r.eax = m.word(ebp - 0x14);
        flags.add(m.word(ebp - 0x18), r.eax);
        m.word(ebp - 0x18, m.word(ebp - 0x18) + r.eax);
    }
    // the strip itself, 256 high, in what is left of the square
    flags.compare(r.ecx, m.word(0x8b441c));
    if (less(m.word(0x8b441c), r.ecx))
    {
        r.eax = m.word(ebp - 4);
        r.edx = r.esi - r.eax;
        r.eax = r.ecx - m.word(0x8b441c);
        flags.compare(r.edx, r.eax);
        if (less(r.edx, r.eax))
            r.eax = r.edx;
        r.ebx = m.word(ebp - 0xc);
        m.word(ebp - 0x30, r.eax);
        r.eax = r.esi - r.ebx;
        flags.compare(r.eax, 0x100);
        if (!less(r.eax, 0x100))
            r.eax = 0x100;
        m.word(ebp - 0x2c, r.eax);
        r.eax = m.word(0x8b441c);
        m.word(ebp - 0x20, r.eax);
        for (;;)
        {
            r.eax = m.word(ebp - 0x20);
            flags.compare(r.eax, m.word(0x8b4418));
            if (!less(r.eax, m.word(0x8b4418)))
                break;
            r.ecx = 0;
            flags.logic(0);
            m.word(ebp - 0x1c, r.ecx);
            do
            {
                bar(m.word(ebp - 0x20), m.word(ebp - 0x1c));
                const x86::reg32 at = r.eax * 4;
                r.edx = m.word(ebp - 0x20);
                r.eax = m.word(0x8b4418) - r.edx;
                r.ecx = m.word(ebp - 0x30);
                flags.compare(r.eax, r.ecx);
                if (!less(r.ecx, r.eax))
                    r.ecx = r.eax;
                r.edx = m.word(0x792310);
                r.eax = at / 4;
                r.ebx = m.word(ebp - 0x1c);
                r.edx = m.word(0x8b4414);
                m.word(at + 0x7920a8, r.ecx);
                r.eax = m.word(ebp - 0x2c);
                r.edx -= r.ebx;
                flags.compare(r.edx, r.eax);
                if (less(r.eax, r.edx))
                    r.edx = r.eax;
                r.ecx = m.word(0x792310);
                m.word(at + 0x7920ac, r.edx);
                m.word(at + 0x792094, r.esi);
                r.edx = 0;
                r.ebx = m.word(ebp - 8);
                m.word(at + 0x7920b0, r.edx);
                r.eax = r.ecx;
                r.edx = r.edi;
                r.ecx = 0x10;
                if (!callTo(0x4d4440))
                    return;
                r.ecx = m.word(0x792310) + 1;
                r.eax = m.word(ebp - 0x2c);
                r.ebx = m.word(ebp - 0x30);
                m.word(0x792310, r.ecx);
                flags.compare(r.eax, r.ebx);
                if (less(r.eax, r.ebx))
                {
                    flags.add(m.word(ebp - 0xc), r.eax);
                    m.word(ebp - 0xc, m.word(ebp - 0xc) + r.eax);
                }
                else
                    m.word(ebp - 4, m.word(ebp - 4) + r.ebx);
                r.eax = m.word(ebp - 0x2c);
                flags.add(m.word(ebp - 0x1c), r.eax);
                m.word(ebp - 0x1c, m.word(ebp - 0x1c) + r.eax);
                flags.compare(m.word(ebp - 0x1c), 0x100);
            } while (less(m.word(ebp - 0x1c), 0x100));
            r.eax = m.word(ebp - 0x30);
            flags.add(m.word(ebp - 0x20), r.eax);
            m.word(ebp - 0x20, m.word(ebp - 0x20) + r.eax);
        }
    }
    // the square's texture, in the next record
    r.edx = m.word(ebp - 0x10);
    r.eax = r.edx * 44;
    r.ebx = m.word(ebp - 0x3c) + r.eax;
    r.ecx = m.word(ebp + 0x10);
    flags.compare(r.ecx, 2);
    const bool paletted = r.ecx == 2;
    push(0);
    push(paletted ? 4 : 0);
    push(r.ecx);
    push(r.esi);
    push(r.esi);
    if (!callTo(m.word(0x9ef968)))
        return;
    m.word(r.ebx + 4, r.eax);
    flags.logic(r.eax);
    if (!r.eax)
    {
        if (!drop())
            return;
        r.eax = 0;
        if (paletted)  // the other path's xor leaves the flags as they were
            flags.logic(0);
        return leave();
    }
    if (paletted)
    {
        r.ecx = m.word(0x563a84);
        push(r.ecx);
    }
    else
        push(0);
    r.esi = m.word(ebp - 8);
    push(r.esi);
    push(r.eax);
    if (!callTo(m.word(0x9ef964)))
        return;
    if (!drop())
        return;
    r.eax = m.word(ebp - 0x10);
    leave();
}

}

bool clipByZ(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4bf790))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4bf790", 0x4bf790, state, render::clipByZNative, true);
        return true;
    }
    render::clipByZNative(app, cpu);
    return true;
}

bool quadByZ(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4c20e0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4c20e0", 0x4c20e0, state, render::quadByZNative, true);
        return true;
    }
    render::quadByZNative(app, cpu);
    return true;
}

bool triangleRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x434120))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_434120", 0x434120, state, render::triangleRecordsNative, true);
        return true;
    }
    render::triangleRecordsNative(app, cpu);
    return true;
}

bool nearTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4c3ad0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4c3ad0", 0x4c3ad0, state, render::nearTriangleNative, true);
        return true;
    }
    render::nearTriangleNative(app, cpu);
    return true;
}

bool quadRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4332b0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4332b0", 0x4332b0, state, render::quadRecordsNative, true);
        return true;
    }
    render::quadRecordsNative(app, cpu);
    return true;
}

bool subdivideQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x432720))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_432720", 0x432720, state, render::subdivideQuadNative, true);
        return true;
    }
    render::subdivideQuadNative(app, cpu);
    return true;
}

bool quadSubdivided(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x433060))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_433060", 0x433060, state, render::quadSubdividedNative, true);
        return true;
    }
    render::quadSubdividedNative(app, cpu);
    return true;
}

bool colouredTriangles(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x433e30))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_433e30", 0x433e30, state, render::colouredTrianglesNative, true);
        return true;
    }
    render::colouredTrianglesNative(app, cpu);
    return true;
}

bool sky(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x47a190))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47a190", 0x47a190, state, render::skyNative, true);
        return true;
    }
    render::skyNative(app, cpu);
    return true;
}

bool viewSky(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x47a880))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47a880", 0x47a880, state, render::viewSkyNative, true);
        return true;
    }
    render::viewSkyNative(app, cpu);
    return true;
}

bool flatSky(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x47a1f0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47a1f0", 0x47a1f0, state, render::flatSkyNative, true);
        return true;
    }
    render::flatSkyNative(app, cpu);
    return true;
}

bool skyDome(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x47b850))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47b850", 0x47b850, state, render::skyDomeNative, true);
        return true;
    }
    render::skyDomeNative(app, cpu);
    return true;
}

bool lightning(win32::WinApplication* app, x86::CPU& cpu)
{
    // A divisor of 0 would fault in the original: left to the generated code.
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4946e0)
        || !app->getMemory<x86::reg32>(0x55e4ac))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4946e0", 0x4946e0, state, render::lightningNative, true);
        return true;
    }
    render::lightningNative(app, cpu);
    return true;
}

bool countdown(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x482620))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_482620", 0x482620, state, render::countdownNative, true);
        return true;
    }
    render::countdownNative(app, cpu);
    return true;
}

bool carLights(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4b9bc0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4b9bc0", 0x4b9bc0, state, render::carLightsNative, true);
        return true;
    }
    render::carLightsNative(app, cpu);
    return true;
}

bool detailedCarLights(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4bae00))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4bae00", 0x4bae00, state, render::detailedCarLightsNative, true);
        return true;
    }
    render::detailedCarLightsNative(app, cpu);
    return true;
}

bool solidText(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x475b50))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_475b50", 0x475b50, state, render::solidTextNative, true);
        return true;
    }
    render::solidTextNative(app, cpu);
    return true;
}

bool mediumCarLights(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4bb200))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4bb200", 0x4bb200, state, render::mediumCarLightsNative, true);
        return true;
    }
    render::mediumCarLightsNative(app, cpu);
    return true;
}

bool nightColour(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4b97b0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4b97b0", 0x4b97b0, state, render::nightColourNative, true);
        return true;
    }
    render::nightColourNative(app, cpu);
    return true;
}

bool viewPass(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x41b9b0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_41b9b0", 0x41b9b0, state, render::viewPassNative, true);
        return true;
    }
    render::viewPassNative(app, cpu);
    return true;
}

bool polygonDrawers(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x434870))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_434870", 0x434870, state, render::polygonDrawersNative, true);
        return true;
    }
    render::polygonDrawersNative(app, cpu);
    return true;
}

bool glareColours(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x49d800))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_49d800", 0x49d800, state, render::glareColoursNative, true);
        return true;
    }
    render::glareColoursNative(app, cpu);
    return true;
}

bool detailSettings(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || cpu.flags.df || !renderNativeOn(0x472d10))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_472d10", 0x472d10, state, render::detailSettingsNative, true);
        return true;
    }
    render::detailSettingsNative(app, cpu);
    return true;
}

bool sparks(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4ddb40)
        // A divisor of 0 or -1 would fault in the original: left to the generated code.
        || app->getMemory<x86::reg32>(0x8ca27c) == 0 || app->getMemory<x86::reg32>(0x8ca27c) == 0xffffffff)
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4ddb40", 0x4ddb40, state, render::sparksNative, true);
        return true;
    }
    render::sparksNative(app, cpu);
    return true;
}

bool glow(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x491190))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_491190", 0x491190, state, render::glowNative, true);
        return true;
    }
    render::glowNative(app, cpu);
    return true;
}

bool glareVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x47b7d0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47b7d0", 0x47b7d0, state, render::glareVerticesNative, true);
        return true;
    }
    render::glareVerticesNative(app, cpu);
    return true;
}

bool clearBuffers(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4cb670))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4cb670", 0x4cb670, state, render::clearBuffersNative, true);
        return true;
    }
    render::clearBuffersNative(app, cpu);
    return true;
}

bool bouncers(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4a4410))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4a4410", 0x4a4410, state, render::bouncersNative, true);
        return true;
    }
    render::bouncersNative(app, cpu);
    return true;
}

bool roadLight(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x491bc0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_491bc0", 0x491bc0, state, render::roadLightNative, true);
        return true;
    }
    render::roadLightNative(app, cpu);
    return true;
}

bool viewRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x41df10))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_41df10", 0x41df10, state, render::viewRecordNative, true);
        return true;
    }
    render::viewRecordNative(app, cpu);
    return true;
}

bool raceViews(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x41d960))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_41d960", 0x41d960, state, render::raceViewsNative, true);
        return true;
    }
    render::raceViewsNative(app, cpu);
    return true;
}

bool smoke(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4de070))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4de070", 0x4de070, state, render::smokeNative, true);
        return true;
    }
    render::smokeNative(app, cpu);
    return true;
}

bool streaks(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4de700))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4de700", 0x4de700, state, render::streaksNative, true);
        return true;
    }
    render::streaksNative(app, cpu);
    return true;
}

bool viewDistances(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x41d620))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_41d620", 0x41d620, state, render::viewDistancesNative, true);
        return true;
    }
    render::viewDistancesNative(app, cpu);
    return true;
}

bool transformBuffer(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4b56e0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4b56e0", 0x4b56e0, state, render::transformBufferNative, true);
        return true;
    }
    render::transformBufferNative(app, cpu);
    return true;
}

bool screenMode(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || cpu.flags.df || !renderNativeOn(0x4bef50))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4bef50", 0x4bef50, state, render::screenModeNative, true);
        return true;
    }
    render::screenModeNative(app, cpu);
    return true;
}

bool startDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || cpu.flags.df || !renderNativeOn(0x4b59c0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4b59c0", 0x4b59c0, state, render::startDriverNative, true);
        return true;
    }
    render::startDriverNative(app, cpu);
    return true;
}

bool hudFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x47f650))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_47f650", 0x47f650, state, render::hudFrameNative, true);
        return true;
    }
    render::hudFrameNative(app, cpu);
    return true;
}

bool hudRect(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x480910))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_480910", 0x480910, state, render::hudRectNative, true);
        return true;
    }
    render::hudRectNative(app, cpu);
    return true;
}

bool hudPicture(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x480fc0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_480fc0", 0x480fc0, state, render::hudPictureNative, true);
        return true;
    }
    render::hudPictureNative(app, cpu);
    return true;
}

bool hudDial(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4812e0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4812e0", 0x4812e0, state, render::hudDialNative, true);
        return true;
    }
    render::hudDialNative(app, cpu);
    return true;
}


bool hudPanel(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x481aa0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_481aa0", 0x481aa0, state, render::hudPanelNative, true);
        return true;
    }
    render::hudPanelNative(app, cpu);
    return true;
}

bool hudTable(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x481c50))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_481c50", 0x481c50, state, render::hudTableNative, true);
        return true;
    }
    render::hudTableNative(app, cpu);
    return true;
}

bool hudInset(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x487210))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_487210", 0x487210, state, render::hudInsetNative, true);
        return true;
    }
    render::hudInsetNative(app, cpu);
    return true;
}


bool startLights(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x48c7c0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_48c7c0", 0x48c7c0, state, render::startLightsNative, true);
        return true;
    }
    render::startLightsNative(app, cpu);
    return true;
}

bool needles(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4880b0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4880b0", 0x4880b0, state, render::needlesNative, true);
        return true;
    }
    render::needlesNative(app, cpu);
    return true;
}

bool barGauges(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x488470))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_488470", 0x488470, state, render::barGaugesNative, true);
        return true;
    }
    render::barGaugesNative(app, cpu);
    return true;
}

bool cockpit(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x489240))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_489240", 0x489240, state, render::cockpitNative, true);
        return true;
    }
    render::cockpitNative(app, cpu);
    return true;
}


bool line(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4c0b20))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4c0b20", 0x4c0b20, state, render::lineNative, true);
        return true;
    }
    render::lineNative(app, cpu);
    return true;
}

bool segmentMeter(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x48c0e0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_48c0e0", 0x48c0e0, state, render::segmentMeterNative, true);
        return true;
    }
    render::segmentMeterNative(app, cpu);
    return true;
}


bool hudBand(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4800a0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4800a0", 0x4800a0, state, render::hudBandNative, true);
        return true;
    }
    render::hudBandNative(app, cpu);
    return true;
}


bool hud(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x482b00))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_482b00", 0x482b00, state, render::hudNative, true);
        return true;
    }
    render::hudNative(app, cpu);
    return true;
}


bool carTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4b7d30))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4b7d30", 0x4b7d30, state, render::carTexturesNative, true);
        return true;
    }
    render::carTexturesNative(app, cpu);
    return true;
}


bool loadingScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x494cb0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_494cb0", 0x494cb0, state, render::loadingScreenNative, true);
        return true;
    }
    render::loadingScreenNative(app, cpu);
    return true;
}


bool moviePlayer(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x495bc0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_495bc0", 0x495bc0, state, render::moviePlayerNative, true);
        return true;
    }
    render::moviePlayerNative(app, cpu);
    return true;
}


bool pictureTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4d0a10))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4d0a10", 0x4d0a10, state, render::pictureTextureNative, true);
        return true;
    }
    render::pictureTextureNative(app, cpu);
    return true;
}


bool squareTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || !renderNativeOn(0x4d3f50))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4d3f50", 0x4d3f50, state, render::squareTextureNative, true);
        return true;
    }
    render::squareTextureNative(app, cpu);
    return true;
}


bool paletteTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4d41a0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4d41a0", 0x4d41a0, state, render::paletteTextureNative, true);
        return true;
    }
    render::paletteTextureNative(app, cpu);
    return true;
}


bool pieceTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !singlePrecision(cpu) || cpu.flags.df || !renderNativeOn(0x4d4580))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4d4580", 0x4d4580, state, render::pieceTextureNative, true);
        return true;
    }
    render::pieceTextureNative(app, cpu);
    return true;
}


bool cabinTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    if (nativeOriginalRunning() || !nativesEnabled() || !renderNativeOn(0x4d4ae0))
        return false;
    if (nativeChecking())
    {
        static void* state = nullptr;
        nativeCheckWhole(app, cpu, "sub_4d4ae0", 0x4d4ae0, state, render::cabinTexturesNative, true);
        return true;
    }
    render::cabinTexturesNative(app, cpu);
    return true;
}

}
