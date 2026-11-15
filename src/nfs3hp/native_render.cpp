#include <nfs3hp.h>
#include "game_settings.h"
#include "dynamic_fullscreen.h"
#include "port_thrash.h"
#include "thrash_calls.h"
#include "native_render.h"
#include "native_windows.h"
#include "host_window.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_rendc.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "port_text.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "host_system.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

/* a * b + c fused into one fma rounds once where the x87 rounds twice. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The rest of the drawing, native whole: the functions native_vertices.cpp
 * left generated -- the clipping of a triangle whose corners carry z, the
 * polygon lists, the sky, the lights, the effects' colours, the views, the HUD,
 * the textures and the 2D screens -- in nfs3hp::render, each bound to its
 * function's address by disassemble_nfs3hp.py (RENDER_NATIVES), which writes
 * no code for it: the game's callers call it by name, dynamic_call by address.
 * Since phase 2 there is no hook and no generated code behind them to fall
 * back on.  Each does the original's arithmetic in the original's order
 * through fp:: (FpuMath), so it rounds as the generated code does, at single
 * precision, and converts to integers as fistp does.  Where the original copies with
 * movs they copy forwards: the game never leaves the direction flag set (its
 * one std, in memmove at 0x518640, is undone by the cld before it returns).
 * An integer divide by zero (lightning, sparks) gives what the generated
 * code's idiv gives on arm64: quotient 0, the dividend's low word the
 * remainder.  The port's changes to these functions (the mirror, View
 * Distance, alpha intensity, the HUD's and the cabin's 4:3 sizes, wide-screen
 * HUD pictures, car detail, the loading screen at 4:3) are made here; the
 * tools/apply_*.py scripts leave these functions out (NATIVE_SITES).
 *
 * Each keeps its work in C++ locals (plan step 3); the registers are read at
 * its entry, set for a call out -- the callee's register arguments as its code
 * reads them (entry_regs.py), esi where a callee reads a pointer there -- and
 * left at its exit as the game's callers read them: eax where a caller reads
 * it, every register but the function's own arguments as it came.  Memory
 * comes out as the generated code leaves it, but for the stack below the
 * returned esp.  THRASH's exports are called through guest::driver
 * (thrash_calls.h, plan step 4): the port's driver as C++ where the game's
 * pointer leads to it.  hud (sub_482b00) alone is still written on the
 * registers and the flags (Registers, IntegerFlags, callWithRegisters).
 * tools/game_checks/render holds each against its code made afresh (with the
 * port's changes) on the desktop, in five x87 control words. */
namespace nfs3hp
{

// nfs3hp_main.cpp: a view pass's kind as the main view's where the mirror is drawn in full.
x86::reg32 mirrorAsMain(x86::reg32 kind);
// nfs3hp_main.cpp: View Distance Full kept whole (sub_41d620's tables, split-screen far
// distance) and the mirror at the main view's distances.
x86::reg32 viewDistanceReduced(win32::WinApplication* app, x86::reg32 reduced);
x86::reg32 splitFarDistance(win32::WinApplication* app, x86::reg32 distance);
x86::reg32 fullFarDistance(win32::WinApplication* app, x86::reg32 distance);
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

/* The arithmetic as the generated code does it (fp::).  The operands go in the
 * order the generated code has them, which decides a NaN's payload. */
struct FpuMath
{
    typedef double Value;
    static Value add(Value a, Value b) { return fp::add(a, b); }
    static Value sub(Value a, Value b) { return fp::sub(a, b); }
    static Value mul(Value a, Value b) { return fp::mul(a, b); }
    static Value div(Value a, Value b) { return fp::div(a, b); }
    static Value sqrt(Value a) { return fp::sqrt(a); }
};
typedef FpuMath M;

/* cdq; idiv r32 as the generated code does it: edx:eax, eax sign-extended,
 * divided in 64 bits, the quotient's low word to eax (2^31 for the lowest
 * integer over -1) and the remainder to edx.  A divisor of 0, which faults on
 * the x86, gives what that division gives on arm64, where sdiv by zero is 0
 * and the remainder (msub) the dividend less nothing: eax 0, edx the
 * dividend's low word. */
struct Division
{
    x86::reg32 quotient, remainder;
};
inline Division idiv(x86::reg32 low, x86::reg32 divisor)
{
    const x86::sreg64 dividend = x86::sreg32(low);
    const x86::sreg64 by = x86::sreg32(divisor);
    if (!by)
        return {0, low};
    return {x86::reg32(dividend / by), x86::reg32(dividend % by)};
}

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
    return x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(M::mul(M::mul(width, m.load(k0)), m.load(k1)))));
}

/* The typed natives' words (plan step 5): a core with named parameters that keeps its locals in
 * C++, and an adapter under the game's name that takes the registers and leaves them, the flags
 * and the x87 as the game's code does -- the check compares all of them, and every register at
 * every call out. */
using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;
using x86::reg8;
using i32 = std::int32_t;

reg32 at(App* app, reg32 address) { return app->getMemory<reg32>(address); }
void put(App* app, reg32 address, reg32 value) { app->getMemory<reg32>(address) = value; }
reg8 byteAt(App* app, reg32 address) { return app->getMemory<reg8>(address); }
void putByte(App* app, reg32 address, reg8 value) { app->getMemory<reg8>(address) = value; }

// The driver's abilities (THRASH_about's): bit 0x40 a driver drawing with alpha, 6 others.
constexpr reg32 kDriverAbilities = 0x7a3a58;
constexpr reg8 kAbilityAlpha = 0x40;

/* A colour's channels times the high byte of `scale` (held to 0xffff) in 256ths, as the game's
 * inline pair of muls (darken): each channel's product fits its 16 bits. */
reg32 darkened(reg32 colour, reg32 scale)
{
    const reg32 factor = ((i32(scale) >= 0x10000 ? 0xffffu : scale) >> 8) & 0xff;
    const reg32 redBlue = (colour & 0xff00ff) * factor;
    const reg32 alphaGreen = ((colour & 0xff00ff00) >> 8) * factor;
    return (alphaGreen & 0xff00ff00) | ((redBlue >> 8) & 0xff00ff);
}

/* The caller's registers as an adapter found them, for those the original pushes on entry and
 * pops before its ret: restore() gives back the ones named and returns past the return address
 * and `popped` bytes of arguments. */
class Entry
{
public:
    enum : unsigned { kEbx = 1, kEcx = 2, kEdx = 4, kEsi = 8, kEdi = 16, kEbp = 32 };
    explicit Entry(const CPU& cpu)
        : esp(cpu.esp), saved{cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp}
    {
    }
    void restore(CPU& cpu, unsigned which, reg32 popped = 0) const
    {
        reg32* const registers[] = {&cpu.ebx, &cpu.ecx, &cpu.edx, &cpu.esi, &cpu.edi, &cpu.ebp};
        for (unsigned k = 0; k < 6; ++k)
            if (which & (1u << k))
                *registers[k] = saved[k];
        cpu.esp = esp + 4 + popped;
    }
    const reg32 esp;  // the return address's

private:
    reg32 saved[6];
};

/* A call by address with ebp at the original's frame and esp as deep as its is there; the
 * registers the original has at the call are set by each callee's helper, every other one is as
 * the caller left it.  False when the game stopped. */
bool callIn(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 target)
{
    cpu.ebp = frame;
    cpu.esp = esp;
    return call(app, cpu, target);
}

/* The functions the register code below calls whose results come back on st(0), where they have a
 * typed core (plan step 1): in the game the core as C++, the registers, the flags and esp left as
 * the call by address leaves them and the result handed back; in the checks (NFS_GAME_CHECKS:
 * tools/game_checks/render builds this file alone, and its stand-ins log every call and leave a
 * made-up result on st(0)) the call by address as the code makes it, st(0) taken off.  Each takes
 * what callWithRegisters takes and moves esp on as the call does; nullopt when the game stopped. */
#ifdef NFS_GAME_CHECKS
std::optional<double> stackResultOf(App* app, CPU& cpu, IntegerFlags& flags, Registers& r, reg32 ebp, reg32& esp,
                                    reg32 target)
{
    if (!callWithRegisters(app, cpu, flags, r, ebp, esp, target))
        return std::nullopt;
    esp = cpu.esp;
    return double(cpu.fpu.take());
}
#else
// What callWithRegisters hands a callee: the flags, the registers, ebp and esp.
void handOver(CPU& cpu, IntegerFlags& flags, const Registers& r, reg32 ebp, reg32 esp)
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
}
#endif

// __CHP (0x4dfd56) on `value`, handed it on st(0): watcom::clib::chop, which leaves all else as it was.
std::optional<double> chopValue(App* app, CPU& cpu, IntegerFlags& flags, Registers& r, reg32 ebp, reg32& esp,
                                double value)
{
#ifdef NFS_GAME_CHECKS
    cpu.fpu.push(x86::Float(value));
    return stackResultOf(app, cpu, flags, r, ebp, esp, 0x4dfd56);
#else
    handOver(cpu, flags, r, ebp, esp);
    const double whole = guest::direct<0x4dfd56, watcom::clib::chop>(app, cpu, value);
    flags.load(cpu);
    return whole;
#endif
}

#ifndef NFS_GAME_CHECKS
/* A core of the `words` words the code pushes (stdcall) whose result the code takes off st(0), run as
 * C++ by `core` (handed the first two words): the registers (eax too), the flags and ebp as the call
 * by address leaves them, esp past the words. */
template <typename Core>
std::optional<double> pushedValueOf(App* app, CPU& cpu, IntegerFlags& flags, const Registers& r, reg32 ebp,
                                    reg32& esp, reg32 words, Core core)
{
    handOver(cpu, flags, r, ebp, esp);
    std::optional<double> value;
    try
    {
        value = core(app->getMemory<reg32>(esp), app->getMemory<reg32>(esp + 4));
    }
    catch (const fe::Terminated&)
    {
    }
    if (!value || cpu.terminate)
        return std::nullopt;
    esp += 4 * words;
    flags.load(cpu);
    return value;
}
#endif

// sub_4d1390, the scale pushed: port_text's textScaleSet, which leaves all else (eax too) as it was;
// the scale before it, which the code drops (fstp st(0)).
bool textScalePushed(App* app, CPU& cpu, IntegerFlags& flags, Registers& r, reg32 ebp, reg32& esp)
{
#ifdef NFS_GAME_CHECKS
    return stackResultOf(app, cpu, flags, r, ebp, esp, 0x4d1390).has_value();
#else
    return pushedValueOf(app, cpu, flags, r, ebp, esp, 1, [&](reg32 scale, reg32) {
        return guest::direct<0x4d1390, port::text::textScaleSet>(app, cpu, scale);
    }).has_value();
#endif
}

}

namespace render
{

// step 3, first part
namespace
{

/* A call by address as the original makes it: ebp at its frame, esp as deep as its is there,
 * `registers` into eax, edx, ebx and ecx in turn (the callee's register arguments); after it
 * every register but eax as it was before.  False when the game stopped. */
bool callAt(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 target, std::initializer_list<reg32> registers = {})
{
    const guest::KeptRegisters kept(cpu);
    reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const reg32 value : registers)
        *into[i++] = value;
    return callIn(app, cpu, frame, esp, target);
}

/* callAt for a callee that also takes a pointer in esi (sub_479100, sub_49c470, sub_4c7380,
 * sub_420230, sub_420650, sub_448330: their code reads it), esi as it was after it. */
bool callAtEsi(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 target, reg32 esi,
               std::initializer_list<reg32> registers = {})
{
    const guest::KeptRegisters kept(cpu);
    cpu.esi = esi;
    return callAt(app, cpu, frame, esp, target, registers);
}

/* One of THRASH's exports through the game's pointer to it (guest::driver, thrash_calls.h: the
 * port's driver as C++ where the pointer leads to it, through the pointer in the checks), its words
 * pushed from `esp` as the original pushes them: stdcall, every register but eax kept. */
template <reg32 Slot, auto Fn, typename... W>
std::optional<reg32> driverAt(App* app, CPU& cpu, reg32 esp, W... words)
{
    const guest::KeptRegisters kept(cpu);
    cpu.esp = esp;
    return guest::driver<Slot, Fn>(app, cpu, words...);
}

/* A vertex's clip code from the bits of its 1/z, y and x, as screenCode works it out. */
reg8 clipCode(const Memory& m, reg32 v)
{
    const reg32 w = m.word(v + 0xc);
    if ((w & 0x80000000) || x86::sreg32(w) >= x86::sreg32(m.word(0x7d34f8)))
        return 0x10;
    reg8 code = 0;
    const reg32 y = m.word(v + 4);
    if ((y & 0x80000000) || x86::sreg32(y) < x86::sreg32(m.word(0x7d3504)))
        code |= 8;
    else if (x86::sreg32(y) > x86::sreg32(m.word(0x7d3500)))
        code |= 4;
    const reg32 x = m.word(v);
    if ((x & 0x80000000) || x86::sreg32(x) < x86::sreg32(m.word(0x7d34fc)))
        code |= 1;
    else if (x86::sreg32(x) > x86::sreg32(m.word(0x7d350c)))
        code |= 2;
    return code;
}

/* __CHP (0x4dfd56) on `value` at the original's depth, every register kept: watcom::clib::chop in
 * the game; in the checks (NFS_GAME_CHECKS) the call by address, `value` pushed alone on the x87.
 * nullopt when the game stopped. */
std::optional<double> chopAt(App* app, CPU& cpu, reg32 frame, reg32 esp, double value)
{
#ifdef NFS_GAME_CHECKS
    cpu.fpu.push(x86::Float(value));
    if (!callAt(app, cpu, frame, esp, 0x4dfd56))
        return std::nullopt;
    return double(cpu.fpu.take());
#else
    const guest::KeptRegisters kept(cpu);
    cpu.ebp = frame;
    cpu.esp = esp;
    return guest::direct<0x4dfd56, watcom::clib::chop>(app, cpu, value);
#endif
}

/* The functions whose results come back on st(0), at the original's depth (`esp` past the words
 * the code pushes for them), every register kept: in the game the core as C++, in the checks
 * (NFS_GAME_CHECKS) the call by address with its register arguments, st(0) taken off.  nullopt
 * when the game stopped. */
#ifdef NFS_GAME_CHECKS
std::optional<double> resultAt(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 target,
                               std::initializer_list<reg32> registers)
{
    if (!callAt(app, cpu, frame, esp, target, registers))
        return std::nullopt;
    return double(cpu.fpu.take());
}
#else
template <typename Core>
std::optional<double> resultIn(App* app, CPU& cpu, reg32 frame, reg32 esp, Core core)
{
    const guest::KeptRegisters kept(cpu);
    cpu.ebp = frame;
    cpu.esp = esp;
    std::optional<double> value;
    try
    {
        value = core();
    }
    catch (const fe::Terminated&)
    {
    }
    if (!value || cpu.terminate)
        return std::nullopt;
    return value;
}
#endif

// The pushed word `i` (0 the last pushed) of a call at `esp`.
reg32 pushedAt(App* app, reg32 esp, reg32 i) { return at(app, esp + 4 * i); }

// sub_4e01f0 on the vectors `a` and `b`: vertices::dotProduct.
std::optional<double> dotAt(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 a, reg32 b)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4e01f0, { a, b });
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    });
#endif
}

// sub_4972f0 on the vector `v`: vertices::normaliseVector, its length.
std::optional<double> normalisedAt(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 v)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4972f0, { v });
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return double(guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, v));
    });
#endif
}

// sub_4968a0 on the matrix `matrix` and the six floats pushed (ret 0x18): vertices::groundHeightAt.
std::optional<double> groundHeightAt(App* app, CPU& cpu, reg32 frame, reg32 esp, reg32 matrix)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4968a0, { matrix });
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        const auto pushed = [&](reg32 i) { return asFloat(pushedAt(app, esp, i)); };
        return double(guest::direct<0x4968a0, vertices::groundHeightAt>(app, cpu, matrix, pushed(0), pushed(1),
                                                                         pushed(2), pushed(3), pushed(4), pushed(5)));
    });
#endif
}

// sub_4ea930 and sub_4ea950 on the angle pushed: simd's sineValue and cosineValue.
std::optional<double> sineAt(App* app, CPU& cpu, reg32 frame, reg32 esp)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4ea930, {});
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return guest::direct<0x4ea930, frontend::simd::sineValue>(app, cpu, pushedAt(app, esp, 0));
    });
#endif
}
std::optional<double> cosineAt(App* app, CPU& cpu, reg32 frame, reg32 esp)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4ea950, {});
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return guest::direct<0x4ea950, frontend::simd::cosineValue>(app, cpu, pushedAt(app, esp, 0));
    });
#endif
}

// sub_4ea920 on the float pushed: simc's squareRootOf.
std::optional<double> squareRootAt(App* app, CPU& cpu, reg32 frame, reg32 esp)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4ea920, {});
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return guest::direct<0x4ea920, frontend::simc::squareRootOf>(app, cpu, pushedAt(app, esp, 0));
    });
#endif
}

// sub_4ea9c0 on the two floats pushed (the first last): simd's angleOfFloatsValue.
std::optional<double> angleAt(App* app, CPU& cpu, reg32 frame, reg32 esp)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, 0x4ea9c0, {});
#else
    return resultIn(app, cpu, frame, esp, [&]() -> std::optional<double> {
        return guest::direct<0x4ea9c0, frontend::simd::angleOfFloatsValue>(app, cpu, pushedAt(app, esp, 0),
                                                                            pushedAt(app, esp, 1));
    });
#endif
}

/* sub_49d5c0 (`all`) or sub_49de20, `registers` its arguments and its words pushed: rendc's as the
 * game's code calls it, the nearest depth it leaves on st(0). */
std::optional<double> modelDepthAt(App* app, CPU& cpu, reg32 frame, reg32 esp, bool all,
                                   std::initializer_list<reg32> registers)
{
#ifdef NFS_GAME_CHECKS
    return resultAt(app, cpu, frame, esp, all ? 0x49d5c0 : 0x49de20, registers);
#else
    const guest::KeptRegisters kept(cpu);
    reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const reg32 value : registers)
        *into[i++] = value;
    cpu.ebp = frame;
    cpu.esp = esp - 4;  // the return address's slot, as call() leaves it
    return all ? frontend::rendc::modelTransformAllCalled(app, cpu)
               : frontend::rendc::modelTransformPartCalled(app, cpu);
#endif
}

/* `words` pushed under `esp`, the first pushed first: esp after them. */
reg32 pushWords(App* app, reg32 esp, std::initializer_list<reg32> words)
{
    for (const reg32 word : words)
        put(app, esp -= 4, word);
    return esp;
}

}

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
 * skipped.  Returns the triangles tried, -1 when a pass leaves fewer than
 * three corners. */
namespace
{

reg32 clipByZOf(App* app, CPU& cpu, reg32 corners, reg8 clipCodes, reg32 codes)
{
    const Memory m{app};
    // push ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xf4; sub ebp, 0x82
    const x86::reg32 frameEsp = cpu.esp - 0x104;
    const x86::reg32 frameEbp = cpu.esp - 0x92;

    // The corners copied (rep movsd), each pointer read as its copy starts.
    for (x86::reg32 i = 0; i < 3; ++i)
    {
        const x86::reg32 from = m.word(corners + 4 * i);
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

    x86::reg32 list = 0x5600c8;    // [ebp+0x56]; codes [ebp+0x52]
    x86::reg32 outList = 0x7d2db8; // ecx
    x86::reg32 outCodes = 0x7d0db0; // edx
    x86::reg32 count = 3;          // ebx
    x86::reg32 made = 0;           // [ebp+0x4e]

    auto codeX = [&](x86::reg32 vertex, x86::reg32 code) {
        const double x = m.load(vertex);
        const double left = m.load(0x7d34f4);
        if (!(x >= left))
            m.byte(code, m.byte(code) | 1);
        else
        {
            const double right = m.load(0x7d34ec);
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
            channels[i] = from - x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(product)));
        }
        return (channels[0] << 24) | (channels[1] << 16) | (channels[2] << 8) | channels[3];
    };

    /* One pass: the corners outside `bit` dropped, a vertex made where an edge
     * crosses it.  False when fewer than three are left; the lists move on
     * otherwise. */
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
            return false;
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
                again = 0.0 > t;
            if (again)
                t = double(float(M::div(m.load(a + 8), M::sub(m.load(a + 8), m.load(b + 8)))));
            m.word(vertex + 0x10, blend(a, b, t));
            m.word(vertex + 8, m.word(0x7d34f0));
            m.byte(code, 0x10);
            const double y = m.load(vertex + 4);
            const double bottom = m.load(0x7d34e0);
            if (!(y >= bottom))
                m.byte(code, m.byte(code) | 8);
            else
            {
                const double top = m.load(0x7d34dc);
                if (y > top)
                    m.byte(code, m.byte(code) | 4);
            }
            codeX(vertex, code);
        });
        if (!kept)
            return 0xffffffff;
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
                return 0xffffffff;
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
                return 0xffffffff;
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
        if (0.0 > w)
            m.word(v + 0xc, 0);
        else
        {
            const x86::reg32 bits = m.word(v + 0xc);
            m.word(v + 0xc, x86::sreg32(bits) > 0x3f800000 ? 0x3f800000 : bits);
        }
    }

    // The fan, last triangle first.
    const x86::reg32 first = m.word(list);
    x86::reg32 drawn = 0;
    for (x86::reg32 i = count - 2; i >= 1; --i)
    {
        ++drawn;
        const x86::reg32 b = m.word(list + 4 * i);
        const x86::reg32 c = m.word(list + 4 * (i + 1));
        bool skip = false;
        for (const x86::reg32 vertex : { first, b, c })
            for (const x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
                skip = skip || (m.word(vertex + field) | 0x80000000) == 0xffc00000;
        if (skip)
            continue;
        if (!driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, frameEsp, first, b, c).has_value())
            return 0;
    }
    return drawn;
}

}

// sub_4bf790 by address: the corners in eax, their codes or-ed in bl, the codes in edx.
void clipByZ(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 drawn = clipByZOf(app, cpu, cpu.eax, reg8(cpu.ebx), cpu.edx);
    if (cpu.terminate)
        return;
    cpu.eax = drawn;
    cpu.esp += 4;
}

/* sub_4c20e0: a quad (eax, edx, ebx, ecx: a, b, c, d) whose corners carry z,
 * to the screen -- as sub_4c12e0 is for the clipper of sub_4c2cd0, here for
 * sub_4bf790's.  Nothing when the four share a clip bit (the byte at +0x14);
 * THRASH_drawquad ([0x9ef94c]) when none has one; otherwise triangles a b c
 * and a c d, each skipped when its corners share a bit, clipped by sub_4bf790
 * when one has one, else drawn by THRASH_drawtri ([0x9ef974]).  A corner
 * drawn gets its depth from 1/z (65535 for a z of 0) as sub_4c1aa0's do,
 * +0x14 cleared and 1/w held to 0..1; nothing is drawn of a quad or triangle
 * with an indefinite in a corner's x, y, depth, 1/w, s or t. */
void quadByZ(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0xdc; sub ebp, 0x82
    const x86::reg32 frameEbp = cpu.esp - 0x8e;
    const x86::reg32 frameEsp = cpu.esp - 0xe8;
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx, d = cpu.ecx;

    // A corner as THRASH takes it: its depth, +0x14 cleared, 1/w held to 0..1.
    auto corner = [&](x86::reg32 v) {
        const x86::reg32 z = m.word(v + 8);
        const x86::reg32 inverse = (z & 0x7fffffff) ? asBits(float(M::div(1.0, m.load(v + 8)))) : 0x477fff00;
        m.word(v + 0x14, 0);
        const bool zeroDepth = (inverse & 0x80000000) || x86::sreg32(inverse) > 0x3f800000;
        m.word(v + 8, zeroDepth ? 0 : asBits(float(M::sub(1.0, double(asFloat(inverse))))));
        m.store(v + 8, M::mul(m.load(0x560094), m.load(v + 8)));
        m.store(v + 8, M::add(m.load(0x560098), m.load(v + 8)));
        const double w = m.load(v + 0xc);
        const x86::reg32 bits = m.word(v + 0xc);
        m.word(v + 0xc, 0.0 > w ? 0 : x86::sreg32(bits) > 0x3f800000 ? 0x3f800000 : bits);
    };
    // The fields THRASH takes of each corner: true when one is an indefinite.
    auto anyIndefinite = [&](std::initializer_list<x86::reg32> vertices) {
        for (const x86::reg32 vertex : vertices)
            for (const x86::reg32 field : { 0u, 4u, 8u, 0xcu, 0x18u, 0x1cu })
                if ((m.word(vertex + field) | 0x80000000) == 0xffc00000)
                    return true;
        return false;
    };
    auto leave = [&]() { cpu.esp += 4; };

    m.word(frameEbp + 0x7e, d);
    const x86::reg8 ca = m.byte(a + 0x14), cb = m.byte(b + 0x14), cc = m.byte(c + 0x14), cd = m.byte(d + 0x14);
    m.byte(frameEbp - 0x46, ca);
    m.byte(frameEbp - 0x45, cb);
    m.byte(frameEbp - 0x44, cc);
    m.byte(frameEbp - 0x43, cd);

    const x86::reg8 threeShared = ca & cb & cc;
    if (cd & threeShared)
        return leave();
    const x86::reg8 threeAny = ca | cb | cc;
    if (!(cd | threeAny))
    {
        for (const x86::reg32 v : { a, b, c, d })
            corner(v);
        if (anyIndefinite({ a, b, c, d }))
            return leave();
        if (!driverAt<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, cpu, frameEsp, a, b, c, d).has_value())
            return;
        return leave();
    }

    // Triangle a b c.
    if (!threeShared)
    {
        if (threeAny)
        {
            m.word(frameEbp - 0x5a, a);
            m.word(frameEbp - 0x56, b);
            m.word(frameEbp - 0x52, c);
            if (!callAt(app, cpu, frameEbp, frameEsp, 0x4bf790, { frameEbp - 0x5a, frameEbp - 0x46, threeAny }))
                return;
        }
        else
        {
            for (const x86::reg32 v : { a, b, c })
                corner(v);
            if (!anyIndefinite({ a, b, c })
                && !driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, frameEsp, a, b, c).has_value())
                return;
        }
    }

    // Triangle a c d, the codes read back from the frame.
    const x86::reg8 ta = m.byte(frameEbp - 0x46), tc = m.byte(frameEbp - 0x44), td = m.byte(frameEbp - 0x43);
    if (ta & tc & td)
        return leave();
    const x86::reg8 any2 = ta | tc | td;
    if (any2)
    {
        // The clipper's list and codes: a, c, d.
        m.word(frameEbp - 0x5a, a);
        m.word(frameEbp - 0x52, d);
        m.byte(frameEbp - 0x45, m.byte(frameEbp - 0x44));
        m.byte(frameEbp - 0x44, m.byte(frameEbp - 0x43));
        m.word(frameEbp - 0x56, c);
        if (!callAt(app, cpu, frameEbp, frameEsp, 0x4bf790, { frameEbp - 0x5a, frameEbp - 0x46, any2 }))
            return;
        return leave();
    }
    for (const x86::reg32 v : { a, c, d })
        corner(v);
    if (anyIndefinite({ a, c, d }))
        return leave();
    if (!driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, frameEsp, a, c, d).has_value())
        return;
    leave();
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
 * at (0 at the end of the list) in eax.  sub_4c3ad0 gets the record in eax
 * and ebx, the near bits in edx, and ecx as the code leaves it: the low byte
 * the first corner's near bit, the next the third corner's code, the rest
 * what the last full write of ecx left. */
void triangleRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const x86::reg32 frameEbp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 28;
    x86::reg32 record = cpu.eax;
    // ecx as the code leaves it: sub_4c3ad0 is handed it.
    x86::reg32 ecx = cpu.ecx;

    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, cpu, frameEsp, state, value).has_value();
    };
    auto drawTriangle = [&](x86::reg32 a, x86::reg32 b, x86::reg32 c) {
        return driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, frameEsp, a, b, c).has_value();
    };
    // The corners' s, t from the record at `from`.
    auto texture = [&](x86::reg32 from) {
        for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
        {
            m.word(m.word(record + corner) + 0x18, m.word(record + from));
            m.word(m.word(record + corner) + 0x1c, m.word(record + from + 4));
            from += 8;
        }
    };
    // The texture at `field` set, unless it is the one kept in [0x554e54].
    auto setTexture = [&](x86::reg32 field, x86::reg32 from) {
        const x86::reg32 handle = m.word(record + field);
        if (handle)
            texture(from);
        if (m.word(0x554e54) == handle)
            return true;
        m.word(0x554e54, handle);
        return setState(1, handle);
    };
    // The blending kept in [0x554e58] set to `on`.
    auto setBlending = [&](x86::reg32 on) {
        if (m.word(0x554e58) == on)
            return true;
        m.word(0x554e58, on);
        return setState(0x68, on);
    };
    // The corners off the left (bit 1 of a code) or-ed.
    auto offLeft = [&]() {
        return (m.word(m.word(record + 8) + 0x14) & 1) | (m.word(m.word(record + 0xc) + 0x14) & 1)
               | (m.word(m.word(record + 0x10) + 0x14) & 1);
    };
    // The triangle drawn, state 0x2c2 on around it when a corner is off the left.
    auto draw = [&]() {
        ecx = offLeft();
        if (!ecx)
            return drawTriangle(m.word(record + 8), m.word(record + 0xc), m.word(record + 0x10));
        if (!setState(0x2c2, 1))
            return false;
        ecx = m.word(record + 8);
        return drawTriangle(m.word(record + 8), m.word(record + 0xc), m.word(record + 0x10)) && setState(0x2c2, 0);
    };

    for (;;)
    {
        const x86::reg8 c0 = m.byte(m.word(record + 8) + 0x14), c1 = m.byte(m.word(record + 0xc) + 0x14);
        const x86::reg8 c2 = m.byte(m.word(record + 0x10) + 0x14);
        const x86::reg8 shared = c0 & c1 & c2;
        setByte(ecx, 0, shared);
        setByte(ecx, 1, c2);
        if (!shared)
        {
            setByte(ecx, 0, c0 & 0x10);
            const x86::reg8 nearBits = (c0 | c1 | c2) & 0x10;
            if (nearBits)
            {
                if (!callAt(app, cpu, frameEbp, frameEsp, 0x4c3ad0, { record, nearBits, record, ecx }))
                    return;
            }
            else
            {
                if (m.word(0x554e58) != 0)
                {
                    ecx = 0;
                    m.word(0x554e58, 0);
                    if (!setState(0x68, 0))
                        return;
                }
                if (!setTexture(0x18, 0x20) || !draw())
                    return;
                if (m.byte(record + 7) & 0x40)
                {
                    if (!setBlending(1) || !setTexture(0x50, 0x54) || !draw())
                        return;
                }
            }
        }
        // The next record, while it is a triangle.
        record = m.word(record);
        if (!record || x86::reg16(m.word(record + 4)) != 3)
            break;
    }
    cpu.eax = record;
    cpu.esp += 4;
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
 * else again the same.  Nothing is drawn when fewer than three corners are
 * left, or when the first corner has an indefinite x, y or 1/w. */
void nearTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x3c
    const x86::reg32 frameEbp = cpu.esp - 20;
    const x86::reg32 frameEsp = cpu.esp - 0x50;
    const x86::reg32 record = cpu.eax;
    const x86::reg8 nearBits = x86::reg8(cpu.edx);
    auto leave = [&]() { cpu.esp += 4; };

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
            if (!(y >= bottom))
                m.byte(code, m.byte(code) | 8);
            else
            {
                const double top = m.load(0x7d34dc);
                if (y > top)
                    m.byte(code, m.byte(code) | 4);
            }
            const double x = m.load(v);
            const double left = m.load(0x7d34f4);
            if (!(x >= left))
                m.byte(code, m.byte(code) | 1);
            else
            {
                const double right = m.load(0x7d34ec);
                if (x > right)
                    m.byte(code, m.byte(code) | 2);
            }
            m.word(outList + 4 * kept, v);
            ++made;
            ++kept;
        }
        if (kept < 3)
            return leave();
        list = outList;
        count = kept;
    }

    // The fan, a corner with an indefinite x or y left out.
    const x86::reg32 first = m.word(list);
    for (const x86::reg32 field : { 0u, 4u, 8u })
        if ((m.word(first + field) | 0x80000000) == 0xffc00000)
            return leave();
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
        if (twoPass)
        {
            m.store(fan + 0xc, M::mul(m.load(v + 0x14), inverse));
            m.store(fan + 8, M::mul(inverse, m.load(v + 0x18)));
        }
        at += 4;
        if ((m.word(fan) | 0x80000000) != 0xffc00000 && (m.word(fan + 4) | 0x80000000) != 0xffc00000)
        {
            fan += 0x20;
            ++drawn;
        }
    }

    // The state through sub_431900, then the fan to THRASH_drawtrifan.
    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return callAt(app, cpu, frameEbp, frameEsp, 0x431900, { state, value });
    };
    auto drawFan = [&]() {
        return driverAt<thrash::kThrashDrawTriFan, &port::driver::drawTriFan>(app, cpu,
            frameEsp, drawn - 2, 0x7cdbb0).has_value();
    };
    if (!setState(0x68, 0) || !setState(1, m.word(record + 0x18)) || !drawFan())
        return;
    if (twoPass)
    {
        if (!setState(0x68, 1) || !setState(1, m.word(record + 0x50)))
            return;
        // The second texture's s, t where THRASH reads them.
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(drawn); ++i)
        {
            const x86::reg32 entryAt = i << 5;
            const double t = m.load(entryAt + 0x7cdbb8);
            m.word(entryAt + 0x7cdbc8, m.word(entryAt + 0x7cdbbc));
            m.store(entryAt + 0x7cdbcc, t);
        }
    }
    if (!drawFan())
        return;
    leave();
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
void quadRecords(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 frameEbp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 36;
    x86::reg32 record = cpu.eax;

    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, cpu, frameEsp, state, value).has_value();
    };
    auto drawQuad = [&](x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d) {
        return driverAt<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, cpu, frameEsp, a, b, c, d).has_value();
    };

    for (;;)
    {
        const x86::reg32 a = m.word(record + 8), b = m.word(record + 0xc);
        const x86::reg32 c = m.word(record + 0x10), d = m.word(record + 0x14);
        const x86::reg32 recordFlags = x86::reg32(x86::sreg32(m.word(record + 4)) >> 16);
        const x86::reg32 blending = recordFlags & 1;
        if (m.word(0x554e58) != blending)
        {
            m.word(0x554e58, blending);
            if (!setState(0x68, blending))
                return;
        }

        const x86::reg32 texture = m.word(record + 0x18);
        if (texture)
        {
            if (!(recordFlags & 2))
            {
                // the record's texture read again for each, as the code does
                auto from = [&](x86::reg32 field) { return m.word(record + 0x18) + field; };
                m.word(a + 0x18, m.word(from(8)));
                m.word(a + 0x1c, m.word(from(0xc)));
                m.store(b + 0x18, m.load(from(0x10)));
                m.store(b + 0x1c, m.load(from(0x14)));
                m.word(c + 0x18, m.word(from(0x18)));
                m.word(c + 0x1c, m.word(from(0x1c)));
                m.store(d + 0x18, m.load(from(0x20)));
                m.store(d + 0x1c, m.load(from(0x24)));
            }
            const x86::reg32 handle = m.word(m.word(record + 0x18) + 4);
            if (m.word(0x554e54) != handle)
            {
                m.word(0x554e54, handle);
                if (!setState(1, handle))
                    return;
            }
        }
        else if (m.word(0x554e54) != 0)
        {
            m.word(0x554e54, 0);
            if (!setState(1, 0))
                return;
        }

        if (m.word(record + 0x18) && (m.byte(m.word(record + 0x18) + 0x28) & 8)
            && !callAt(app, cpu, frameEbp, frameEsp, 0x432390, { m.word(a + 0x10) }))
            return;

        if (recordFlags & 8)
        {
            if (!callAt(app, cpu, frameEbp, frameEsp, 0x433060, { a, b, c, d }))
                return;
        }
        else if (!m.word(record + 0x18) || !(m.byte(m.word(record + 0x18) + 0x28) & 0x10))
        {
            if (!callAt(app, cpu, frameEbp, frameEsp, 0x4c1900, { a, b, c, d }))
                return;
        }
        else if ((m.word(a + 0x14) | m.word(b + 0x14) | m.word(c + 0x14) | m.word(d + 0x14)) & 0x10)
        {
            if (!callAt(app, cpu, frameEbp, frameEsp, 0x4c19e0, { a, b, c, d }))
                return;
        }
        else if (!((m.word(a + 0x14) | m.word(b + 0x14) | m.word(c + 0x14) | m.word(d + 0x14)) & 1))
        {
            if (!drawQuad(a, b, c, d))
                return;
        }
        else if (!setState(0x2c2, 1) || !drawQuad(a, b, c, d) || !setState(0x2c2, 0))
            return;

        // The next record, while it is a quad.
        record = m.word(record);
        if (!record || x86::reg16(m.word(record + 4)))
            break;
    }
    cpu.eax = record;
    cpu.esp += 4;
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
void subdivideQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const x86::reg32 entry = cpu.esp;
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x100; sub ebp, 0x6e
    const x86::reg32 ebp = entry - 0x7a;
    const x86::reg32 frameEsp = entry - 0x10c;
    const x86::reg32 depth = cpu.eax;
    const x86::reg32 sa = cpu.edx, sb = cpu.ebx, sc = cpu.ecx, sd = m.word(entry + 4);
    const x86::reg32 a = m.word(entry + 8), b = m.word(entry + 0xc), c = m.word(entry + 0x10), d = m.word(entry + 0x14);
    // eax as the last call leaves it, or as given
    auto leave = [&]() { cpu.esp += 4 + 0x14; };
    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, cpu, frameEsp, state, value).has_value();
    };
    auto drawQuad = [&]() { return driverAt<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, cpu,
        frameEsp, a, b, c, d).has_value(); };

    if (!callAt(app, cpu, ebp, frameEsp, 0x432690, { a, b, c, d }))
        return;
    if (x86::sreg32(cpu.eax) <= x86::sreg32(depth))
    {
        // Small enough: drawn.
        const x86::reg32 codes = m.word(a + 0x14) | m.word(b + 0x14) | m.word(c + 0x14) | m.word(d + 0x14);
        if (codes & 0x10)
        {
            if (!callAt(app, cpu, ebp, frameEsp, 0x4c19e0, { a, b, c, d }))
                return;
        }
        else if (!(codes & 1))
        {
            if (!drawQuad())
                return;
        }
        else if (!setState(0x2c2, 1) || !drawQuad() || !setState(0x2c2, 0))
            return;
        return leave();
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
        m.byte(v + 0x14, clipCode(m, v));
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
    auto quad = [&](x86::reg32 p, x86::reg32 q, x86::reg32 r, x86::reg32 t, x86::reg32 u, x86::reg32 v,
                    x86::reg32 w, x86::reg32 z) {
        if (code(u) & code(v) & code(w) & code(z))
            return true;
        return callAt(app, cpu, ebp, pushWords(app, frameEsp, { z, w, v, u, t }), 0x432720, { depth + 1, p, q, r });
    };
    if (!quad(sa, s1, s5, s2, a, v1, v5, v2) || !quad(s1, sb, s3, s5, v1, b, v3, v5)
        || !quad(s2, s5, s4, sd, v2, v5, v4, d))
        return;
    const x86::reg32 last = code(v4);
    if (code(v5) & code(v3) & code(c) & last)
    {
        cpu.eax = last;
        return leave();
    }
    if (!callAt(app, cpu, ebp, pushWords(app, frameEsp, { v4, c, v3, v5, s4 }), 0x432720, { depth + 1, s5, s3, sc }))
        return;
    leave();
}

/* sub_433060: a quad (eax, edx, ebx, ecx: a, b, c, d) drawn whole or
 * subdivided.  Nothing when its corners share a clip bit.  When sub_432690
 * asks for subdivision, the corners are projected -- x, y less the centre
 * ([0x5600a4], [0x5600a8]) over w (+0xc, 2^-16 for 0) times the scale
 * ([0x56009c], [0x5600a0]), and 1/w -- to the stack and the quad goes to
 * sub_432720 at depth 0; otherwise to sub_4c19e0 when a corner is past the
 * near plane, else to THRASH_drawquad (state 0x2c2 around it when a corner
 * is off the left). */
void quadSubdivided(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    const x86::reg32 ebp = cpu.esp - 12;
    const x86::reg32 frameEsp = cpu.esp - 0x54;
    const x86::reg32 a = cpu.eax, b = cpu.edx, c = cpu.ebx, d = cpu.ecx;
    auto leave = [&]() { cpu.esp += 4; };
    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, cpu, frameEsp, state, value).has_value();
    };
    auto drawQuad = [&]() { return driverAt<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, cpu,
        frameEsp, a, b, c, d).has_value(); };

    if (m.byte(a + 0x14) & m.byte(b + 0x14) & m.byte(c + 0x14) & m.byte(d + 0x14))
        return leave();
    if (!callAt(app, cpu, ebp, frameEsp, 0x432690, { a, b, c, d }))
        return;
    if (!cpu.eax)
    {
        const x86::reg32 codes = m.word(a + 0x14) | m.word(b + 0x14) | m.word(c + 0x14) | m.word(d + 0x14);
        if (codes & 0x10)
        {
            if (!callAt(app, cpu, ebp, frameEsp, 0x4c19e0, { a, b, c, d }))
                return;
        }
        else if (!(codes & 1))
        {
            if (!drawQuad())
                return;
        }
        else if (!setState(0x2c2, 1) || !drawQuad() || !setState(0x2c2, 0))
            return;
        return leave();
    }

    // Each corner projected: x, y, 1/w (a w of 0 taken as 2^-16).
    auto project = [&](x86::reg32 v, x86::reg32 to) {
        const x86::reg32 bits = m.word(v + 0xc);
        const double w = double(asFloat((bits & 0x7fffffff) ? bits : 0x37800080));
        const double x = M::div(M::sub(m.load(v), m.load(0x5600a4)), M::mul(w, m.load(0x56009c)));
        const double inverse = M::div(1.0, w);
        m.store(to, x);
        const double y = M::div(M::sub(m.load(v + 4), m.load(0x5600a8)), M::mul(w, m.load(0x5600a0)));
        m.store(to + 8, inverse);
        m.store(to + 4, y);
    };
    project(a, ebp - 0x48);
    project(b, ebp - 0x3c);
    project(c, ebp - 0x30);
    project(d, ebp - 0x24);
    if (!callAt(app, cpu, ebp, pushWords(app, frameEsp, { d, c, b, a, ebp - 0x24 }), 0x432720,
                { 0, ebp - 0x48, ebp - 0x3c, ebp - 0x30 }))
        return;
    leave();
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
void colouredTriangles(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 36;
    x86::reg32 record = cpu.eax;

    auto setState = [&](x86::reg32 state, x86::reg32 value) {
        return driverAt<thrash::kThrashSetState, &port::driver::setState>(app, cpu, frameEsp, state, value).has_value();
    };
    auto setBlending = [&](x86::reg32 on) {
        if (m.word(0x554e58) == on)
            return true;
        m.word(0x554e58, on);
        return setState(0x68, on);
    };
    /* One pass's colours, s, t and texture: colours from `colours`, the
     * texture's record field at `texture`, s, t from `st` on. */
    auto prepare = [&](x86::reg32 colours, x86::reg32 texture, x86::reg32 st) -> bool {
        for (x86::reg32 i = 0; i < 3; ++i)
            m.word(m.word(record + 8 + 4 * i) + 0x10, m.word(record + colours + 4 * i));
        const x86::reg32 handle = m.word(record + texture);
        if (handle)
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
            {
                m.word(m.word(record + corner) + 0x18, m.word(record + st));
                m.word(m.word(record + corner) + 0x1c, m.word(record + st + 4));
                st += 8;
            }
        const x86::reg32 kept = handle ? m.word(record + texture) : 0;
        if (m.word(0x554e54) == kept)
            return true;
        m.word(0x554e54, kept);
        return setState(1, kept);
    };
    // The triangle to THRASH_drawtri, or to sub_4c11b0 when `clipped`.
    auto draw = [&](bool clipped) {
        const x86::reg32 a = m.word(record + 8), b = m.word(record + 0xc), c = m.word(record + 0x10);
        if (clipped)
            return callAt(app, cpu, ebp, frameEsp, 0x4c11b0, { a, b, c });
        return driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu, frameEsp, a, b, c).has_value();
    };

    for (;;)
    {
        if (m.byte(record + 7) & 0x80)
        {
            if (!setBlending(0) || !prepare(0x40, 0x18, 0x20))
                return;
            bool clipped = false;
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
                clipped = clipped || m.word(m.word(record + corner) + 0x14) != 0;
            if (!clipped)
            {
                // Each corner's depth: 1 - 1/w (0 when 1/w is negative or past 1), scaled.
                for (x86::reg32 i = 0; i < 3; ++i)
                {
                    const x86::reg32 v = m.word(record + 8 + 4 * i);
                    const x86::reg32 w = m.word(v + 0xc);
                    const bool zero = (w & 0x80000000) || x86::sreg32(w) > 0x3f800000;
                    const x86::reg32 depth = zero ? 0 : asBits(float(M::sub(1.0, m.load(v + 0xc))));
                    const x86::reg32 at = m.word(record + 8 + 4 * i);
                    m.word(at + 8, depth);
                    m.store(at + 8, M::mul(m.load(0x560094), m.load(at + 8)));
                    m.store(at + 8, M::add(m.load(0x560098), m.load(at + 8)));
                }
            }
            if (!draw(clipped))
                return;
        }

        if (m.byte(record + 7) & 0x40)
        {
            if (!setBlending(1) || !prepare(0x74, 0x50, 0x54))
                return;
            bool clipped = false;
            for (const x86::reg32 corner : { 8u, 0xcu, 0x10u })
                clipped = clipped || m.byte(m.word(record + corner) + 0x14) != 0;
            if (!draw(clipped))
                return;
        }

        record = m.word(record);
        if (!record || x86::reg16(m.word(record + 4)) != 3)
            break;
    }
    cpu.eax = record;
    cpu.esp += 4;
}

/* sub_47a190: the sky of a view (eax).  Unless [0x55d2a4], sub_479ed0 first;
 * then, unless [0x6fbc2c] is 1 or the view's kind (its first word, the
 * mirror's as the main view's where the port draws it in full) is 1 -- when
 * [0x55d154] gets [0x7d3684] -- the sky by sub_47b850 and, with [0x55d260],
 * sub_4946e0; with [0x55d2a4], sub_479ed0 after, each handed the view. */
void sky(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push edx, ebp; mov ebp, esp
    const x86::reg32 ebp = cpu.esp - 8;
    const x86::reg32 view = cpu.eax;
    auto callTo = [&](x86::reg32 target) { return callAt(app, cpu, ebp, ebp, target, { view }); };

    if (m.word(0x55d2a4) == 0 && !callTo(0x479ed0))
        return;
    // port (apply_mirror_detail): clouds and lightning in the mirror too
    if (gameSettings(app).skyDetail == 1 || mirrorAsMain(m.word(view)) == 1)
        m.word(0x55d154, m.word(0x7d3684));
    else if (!callTo(0x47b850) || (m.word(0x55d260) != 0 && !callTo(0x4946e0)))
        return;
    if (m.word(0x55d2a4) != 0 && !callTo(0x479ed0))
        return;
    cpu.esp += 4;
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
 * sub_470ad0.  Everything goes through those calls; esi is left past the 8
 * bytes it copies. */
void viewSky(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, edi, ebp; mov ebp, esp; sub esp, 0x10
    const x86::reg32 ebp = cpu.esp - 20;
    const x86::reg32 frameEsp = cpu.esp - 36;
    const x86::reg32 to = cpu.esi;
    const x86::reg32 view = cpu.eax;
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers) {
        return callAt(app, cpu, ebp, frameEsp, target, registers);
    };
    auto viewCall = [&](x86::reg32 target) { return callTo(target, { view }); };

    const double detail = M::mul(M::mul(M::sub(1.0, m.load(0x55d268)), m.load(0x55d1c8)), m.loadDouble(0x53b2f8));
    const x86::reg32 level = x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(detail)));
    m.word(frameEsp - 4, level);
    if (!callTo(0x478dc0, {}) || !callTo(0x479080, {}) || !callTo(0x478e10, { view, ebp - 8, ebp - 4 })
        || !callTo(0x470b70, { m.word(ebp - 8), m.word(ebp - 4) }) || !viewCall(0x47a4c0))
        return;
    bool done = cpu.eax != 0;
    // port (apply_mirror_detail): the sky's dome rather than a flat sky in the mirror too
    if (!done && mirrorAsMain(m.word(view)) == 1)
    {
        if (!callTo(0x47a1f0, {}))
            return;
        done = cpu.eax != 0;
        if (!done && !viewCall(0x4790d0))
            return;
    }
    done = done || gameSettings(app).skyDetail == 2;
    if (!done)
    {
        if (!viewCall(0x479680) || !callTo(0x4790d0, { view, level })
            || !callTo(0x470b20, { m.word(ebp - 4), level }) || !viewCall(0x47a190) || !viewCall(0x4799d0)
            || (m.word(0x55d160) != 0 && !viewCall(0x479b90)) || !viewCall(0x4790d0)
            || !callTo(0x470ad0, { m.word(ebp - 4) }))
            return;
    }
    // sub_479100 leaves its 8 bytes at esi
    if (!callAtEsi(app, cpu, ebp, frameEsp, 0x479100, ebp - 0x10))
        return;
    // movsd twice: the result to [esi], esi past what it copied
    m.word(to, m.word(ebp - 0x10));
    m.word(to + 4, m.word(ebp - 0xc));
    cpu.eax = to;
    cpu.esi = ebp - 8;
    cpu.esp += 4;
}

/* sub_47a1f0: the sky as one quad over the whole screen, for a flat sky.  A
 * polygon record of 0xa0 bytes is allocated (sub_4bbf80, [0x6ff9f8] its
 * texture); four vertices on the stack -- the corners of the screen's rect
 * at [0x6fdc40..4c] (integers), 1500 deep, 1/z 0.000667 -- get their clip
 * codes, and go to sub_478cb0 by value with the texture four times.
 * Returns the record allocation's second word ([ebp+0x7a]: 0 unless the
 * allocator wrote it). */
void flatSky(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x8c; sub ebp, 0x82
    const x86::reg32 ebp = cpu.esp - 0x9a;
    const x86::reg32 frameEsp = cpu.esp - 0xa4;
    auto leave = [&]() {
        cpu.eax = m.word(ebp + 0x7a);
        cpu.esp += 4;
    };

    const x86::reg32 texture = m.word(0x6ff9f8);
    m.word(ebp + 0x7a, 0);
    if (!callAt(app, cpu, ebp, frameEsp, 0x4bbf80, { ebp + 0x76, 0xa0 }))
        return;
    if (!cpu.eax)
        return leave();
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
    for (int i = 0; i < 4; ++i)
        m.byte(v[i] + 0x14, clipCode(m, v[i]));
    // push 0, the texture four times, then the vertices by value, v[3] first.
    x86::reg32 esp = pushWords(app, frameEsp, { 0, texture, texture, texture, texture });
    for (int i = 3; i >= 0; --i)
    {
        esp -= 0x20;
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            m.word(esp + k, m.word(v[i] + k));
    }
    if (!callAt(app, cpu, ebp, esp, 0x478cb0, { m.word(ebp + 0x76) }))
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
void skyDome(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x30
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 0x48;
    const x86::reg32 view = cpu.eax;
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp) {
        return callAt(app, cpu, ebp, esp, target, registers);
    };
    // eax is left as the last call leaves it, or as below
    auto leave = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.esp += 4;
    };

    const x86::reg32 dome = m.word(0x55d1f4);
    if (!dome)
        return leave(view);
    const x86::reg32 recordFlags = (dome == 1 ? 3 : 2) | (m.word(0x55d204) ? 0 : 4);
    if (!callTo(0x47b5c0, {}, frameEsp))
        return;
    const double scale = m.load(0x55d208);
    const double across = M::add(M::mul(m.load(view + 8), scale), m.load(0x55d214));
    const double up = M::add(M::mul(m.load(view + 0x10), scale), m.load(0x55d218));
    if (!callTo(0x47b4f0, { view, 0xa9, 0x6ff9fc, 0x6ff170 },
                pushWords(app, frameEsp, { asBits(float(up)), asBits(float(across)), 0x6fdc50 })))
        return;
    if (m.word(0x55d250) != 0 && !callTo(0x47b3a0, {}, frameEsp))
        return;
    // port (apply_alpha_intensity): the glare on every driver
    if (m.word(0x55d1f4) == 1 && 1.0 > double(gameSettings(app).alphaIntensity)
        && !callTo(0x47b7d0, { 0xa9, 0x6fdc50 }, frameEsp))
        return;
    const x86::reg32 first = ebp - 0x28;  // the records' block, as sub_4bbe40 writes it
    if (!callTo(0x4bbe40, { first }, frameEsp))
        return;
    if (!cpu.eax)
        return leave(0);
    x86::reg32 record = m.word(first);
    x86::reg32 cells = 0;
    x86::reg32 shared = 0;
    const x86::reg32 grid = 0x6fdc50;
    for (x86::reg32 column = 0; column < 12; ++column)
        for (x86::reg32 row = 0; row < 12; ++row)
        {
            const x86::reg32 v00 = grid + 0x1a0 * row + (column << 5), v01 = v00 + 0x20;
            const x86::reg32 v10 = v00 + 0x1a0, v11 = v10 + 0x20;
            shared = x86::reg32(m.byte(v00 + 0x14)) & m.byte(v10 + 0x14) & m.byte(v11 + 0x14);
            if (shared & m.byte(v01 + 0x14))
                continue;
            m.word(record + 0x18, 0x55d21c);
            m.word(record + 4, m.word(record + 4) & 0xffff0000);
            m.word(record, record + 0x20);
            app->getMemory<x86::reg16>(record + 6) = x86::reg16(recordFlags);
            m.word(record + 8, v00);
            m.word(record + 0xc, v10);
            m.word(record + 0x10, v11);
            m.word(record + 0x14, v01);
            record += 0x20;
            ++cells;
        }
    if (!cells)
        return leave(shared);
    const x86::reg32 last = record - 0x20;
    if (!callTo(0x4bbde0, { first, cells << 5 }, frameEsp))
        return;
    const x86::reg32 list = m.word(first);
    m.word(last, 0);
    if (!callTo(0x4bbf40, {}, pushWords(app, frameEsp, { last, list })))
        return;
    cpu.esp += 4;
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
void lightning(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x7c
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 0x94;
    const x86::reg32 view = cpu.eax;
    Registers unused{};

    // A point at `p` (3 floats) turned by the view's matrix into `s`.
    auto turn = [&](x86::reg32 p, x86::reg32 s) {
        const x86::reg32 matrix = view + 0x44;
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            const double sum = M::add(M::mul(m.load(p), m.load(matrix + 4 * i)),
                                      M::mul(m.load(p + 4), m.load(matrix + 0xc + 4 * i)));
            m.store(s + 4 * i, M::add(sum, M::mul(m.load(p + 8), m.load(matrix + 0x18 + 4 * i))));
        }
    };

    if (!callAt(app, cpu, ebp, frameEsp, 0x494630, {}))
        return;
    x86::reg32 eax = 0;  // as the loops leave it
    for (x86::reg32 index = 0; x86::sreg32(index) < 0xf0; ++index)
    {
        const x86::reg32 bolt = 0x79c570 + index * 0x30;
        eax = bolt;
        const x86::reg32 life = m.word(bolt);
        if (!life)
            continue;
        x86::reg32 alpha = idiv(life * 510, m.word(0x55e4ac)).quotient;
        if (x86::sreg32(alpha) >= 0xff)
            alpha = 0xff;
        const x86::reg32 colour = m.word(bolt + 4) != 0 ? 0 : (alpha << 24) | 0xe2e2ff;
        // The start.
        turn(bolt + 0x10, ebp - 0x3c);
        projectPoint(m, ebp - 0x3c, ebp - 0x24, ebp - 0x7c, unused);
        m.byte(ebp - 0x68, clipCode(m, ebp - 0x7c));
        for (x86::reg32 i = 0;; ++i)
        {
            eax = i;
            if (x86::sreg32(i) >= x86::sreg32(m.word(bolt + 0x1c)))
                break;
            const x86::reg32 point = m.word(bolt + 0x20 + 4 * i) + 0x10;
            turn(point, ebp - 0x30);
            projectPoint(m, ebp - 0x30, ebp - 0x18, ebp - 0x5c, unused);
            m.byte(ebp - 0x48, clipCode(m, ebp - 0x5c));
            if (m.byte(ebp - 0x68) & m.byte(ebp - 0x48))
                continue;
            if (!callAt(app, cpu, ebp, frameEsp, 0x4bbf80, { ebp - 0x1c, 0xa0 }))
                return;
            if (!cpu.eax)
                continue;
            const x86::reg32 record = m.word(ebp - 0x1c);
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
            m.word(record + 0x30, colour);
            m.word(record + 0x50, colour);
            m.word(record + 0x70, colour);
            m.word(record + 0x90, colour);
            m.store(record + 0x40, M::add(1.0, m.load(record + 0x40)));
            m.store(record + 0x44, M::add(1.0, m.load(record + 0x44)));
            m.store(record + 0x60, M::add(1.0, m.load(record + 0x60)));
            m.store(record + 0x64, M::add(1.0, m.load(record + 0x64)));
            m.byte(record + 0x54, clipCode(m, record + 0x40));
            m.byte(record + 0x74, clipCode(m, record + 0x60));
        }
    }
    cpu.eax = eax;
    cpu.esp += 4;
}


/* sub_482620: the start's countdown, while [0x7d3684] is between 0x120 and
 * 0x220 and [0x552858] (the number) at most 3: sub_475b50's lettering
 * (record 3, [0x55b088]) at the screen's middle (sub_4beb80), the number
 * printed by sub_4df690 ("%d" at 0x53b5d4) or the word of sub_4d1850(0x150)
 * for 0, centred by sub_476fb0's width, turned by sub_4ea9e0's matrix (a
 * quarter of a turn per 64 ticks, or 0.05) and sized by the ticks
 * ([0x53b5dc], [0x53b5e0] once more on [0x6fd3b0] 1, [0x53b5e4] for the
 * word), its height from the screen's height ([0x53b5d8], 40 or 80). */
void countdown(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x6c
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 24 - 0x6c;
    auto leave = [&]() { cpu.esp += 4; };
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp) {
        return callAt(app, cpu, ebp, esp, target, registers);
    };

    // rep movsd: the 9 words at 0x47bb94 to ebp - 0x6c
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x6c + k, m.word(0x47bb94 + k));
    const x86::reg32 ticks = m.word(0x7d3684);
    if (x86::sreg32(ticks) >= 0x220 || x86::sreg32(ticks) <= 0x120 || m.byte(0x552858) > 3)
        return leave();
    m.word(0x55b088, 3);
    if (!callTo(0x4beb80, { ebp - 0x18, ebp - 0x24, ebp - 0x20, ebp - 0x1c }, frameEsp))
        return;
    // Half a width and a height, rounded toward zero (sar edx, 31; sub; sar 1).
    auto half = [](x86::reg32 value) { return x86::reg32(x86::sreg32(value - x86::reg32(x86::sreg32(value) >> 31)) >> 1); };
    const x86::reg32 width = m.word(ebp - 0x20);
    const double heightScale = double(float(M::mul(double(x86::sreg32(width)), m.load(0x53b5d8))));
    const x86::reg32 centre = m.word(ebp - 0x18) + half(width);
    const x86::reg32 lines = gameSettings(app).playMode == 1 ? 0x28 : 0x50;
    const x86::reg32 middle = half(m.word(ebp - 0x1c)) + m.word(ebp - 0x24);
    const double height = M::mul(double(x86::sreg32(lines)), heightScale);
    const double top = M::sub(double(x86::sreg32(middle)), height);
    const x86::reg8 number = m.byte(0x552858);
    const std::optional<double> topWhole = chopAt(app, cpu, ebp, frameEsp, top);
    if (!topWhole)
        return;
    // the top's low half, as sar of the word under it by 16 reads it
    const x86::reg32 y = x86::reg32(x86::sreg32(x86::sreg16(fp::toInteger<x86::sreg32>(*topWhole))));
    // The lettering: its matrix by sub_4ea9e0, then sub_475b50 at the centre less half `text`'s width.
    auto print = [&](x86::reg32 angle, x86::reg32 textWidth, auto&& makeText) {
        const x86::reg16 left = x86::reg16(centre - x86::reg32(x86::sreg32(textWidth) >> 1));
        const x86::reg32 x = x86::reg32(x86::sreg32(x86::sreg16(left)));
        if (!callTo(0x4ea9e0, { ebp - 0x6c }, pushWords(app, frameEsp, { ebp - 0x48, angle })))
            return false;
        const x86::reg32 esp = pushWords(app, frameEsp, { ebp - 0x48, 0, m.word(0x679014), 0x41f00000 });
        const std::optional<x86::reg32> made = makeText(esp);
        if (!made)
            return false;
        return callTo(0x475b50, { *made, x, y }, esp);
    };
    if (number)
    {
        const x86::reg32 t = (0x200 - m.word(0x7d3684)) & 0x3f;
        const double turn = M::mul(double(x86::sreg32(t)), m.load(0x53b5dc));
        m.store(0x679014, turn);
        const double size = M::mul(turn, heightScale);
        const double angle = M::mul(-m.load(0x679014), m.load(0x53b5e0));
        m.store(0x679014, size);
        if (gameSettings(app).playMode == 1)
            m.store(0x679014, M::mul(m.load(0x679014), m.load(0x53b5e0)));
        if (!callTo(0x4df690, {}, pushWords(app, frameEsp, { number, 0x53b5d4, ebp - 0x14 }))
            || !callTo(0x476fb0, { ebp - 0x14 }, frameEsp))
            return;
        auto number = [&](x86::reg32) { return std::optional<x86::reg32>(ebp - 0x14); };
        if (!print(asBits(float(angle)), cpu.eax, number))
            return;
        return leave();
    }
    const x86::reg32 t = (m.word(0x7d3684) - 0x200) & 7;
    const double grown = M::add(M::mul(double(x86::sreg32(t)), m.load(0x53b5dc)), m.load(0x53b5e4));
    m.store(0x679014, M::mul(grown, heightScale));
    if (gameSettings(app).playMode == 1)
        m.store(0x679014, M::mul(m.load(0x679014), m.load(0x53b5e0)));
    if (!callTo(0x4d1850, { 0x150 }, frameEsp) || !callTo(0x476fb0, { cpu.eax }, frameEsp))
        return;
    // the word asked for again, under the lettering's words
    if (!print(0x3d4ccccd, cpu.eax, [&](x86::reg32 esp) -> std::optional<x86::reg32> {
            if (!callTo(0x4d1850, { 0x150 }, esp))
                return std::nullopt;
            return cpu.eax;
        }))
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
void carLights(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x4c
    const x86::reg32 ebp = cpu.esp - 20;
    const x86::reg32 frameEsp = cpu.esp - 20 - 0x4c;
    const x86::reg32 car = cpu.eax, view = cpu.edx;
    auto leave = [&]() { cpu.esp += 4; };
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers,
                      x86::reg32 esp = 0) { return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers); };

    for (x86::reg32 k = 0; k < 0xc; k += 4)
        m.word(ebp - 0x40 + k, m.word(car + 0x910 + k));
    if (!callTo(0x4206f0, { car + 0xc, 1 }))
        return;
    const x86::reg32 matrix = cpu.eax;
    x86::reg32 lightMatrix = matrix;
    // port (apply_mirror_detail): the mirror as the main view
    if (mirrorAsMain(m.word(view)) == 1)
        return leave();
    if (!callTo(0x4e0430, { 1, ebp - 0x40, view + 0x44, view + 0x38 }, pushWords(app, frameEsp, { ebp - 0x28 })))
        return;
    if (x86::sreg32(m.word(ebp - 0x20)) > 0x42c80000)
        return leave();
    if (-m.load(m.word(0x55fe2c) + 0x30) > m.load(ebp - 0x20))
        return leave();
    if (!callTo(0x49c410, {}) || !callTo(0x49d640, {}))
        return;
    // port (apply_mirror_detail): the mirror as the main view
    if (gameSettings(app).night != 0 && mirrorAsMain(m.word(view)) != 1)
    {
        if (!callTo(0x4b97b0, { car, matrix }) || !callTo(0x4b9550, { car, cpu.eax }))
            return;
        lightMatrix = cpu.eax;
    }
    if (!callTo(0x49d750, { gameSettings(app).useSpecular != 0 ? 1u : 0u }) || !callTo(0x49d760, { 0x55d2a8 })
        || !callTo(0x49d8f0, { lightMatrix }) || !callTo(0x49d800, { 0x4f4f4f4f, 0x7f7f7f7f })
        || !callTo(0x4e0000, { 1, view + 8, ebp - 0x40, ebp - 0x4c }) || !callTo(0x49d780, { ebp - 0x4c }))
        return;
    if ((m.byte(0x7a3a58) & 2) && (!callTo(0x49d750, { 0 }) || !callTo(0x49d8f0, { 0xffffffff })))
        return;
    const x86::reg32 camera = m.word(0x55fe2c);
    m.store(ebp - 0x3c, M::add(m.load(ebp - 0x3c), m.load(camera + 0x2c)));
    const std::optional<double> nearest =
        modelDepthAt(app, cpu, ebp, pushWords(app, frameEsp, { m.word(m.word(0x563a90) + 4), view + 0x38 }), true,
                     { camera, car + 0x8ec, ebp - 0x40, view + 0x44 });
    if (!nearest)
        return;
    app->getMemory<float>(ebp - 0x10) = float(*nearest);
    // the objects' two ends to esi's ebp - 0x1c
    if (!callAtEsi(app, cpu, ebp, frameEsp, 0x49c470, ebp - 0x1c))
        return;
    if (m.word(0x7a3a60) == 0
        && !callAtEsi(app, cpu, ebp, pushWords(app, frameEsp, { m.word(ebp - 0x18), m.word(ebp - 0x1c) }), 0x4c7380,
                      ebp - 0x1c))
        return;
    const double nearer = M::sub(m.load(ebp - 0x20), m.load(0x7dcfe0));
    if (!callTo(0x49c420, { view, ebp - 0x1c }, pushWords(app, frameEsp, { asBits(float(nearer)) })))
        return;
    if (gameSettings(app).track != 8)
        return leave();
    const x86::reg32 camera2 = m.word(0x55fe2c);
    if (m.word(camera2 + 0x34) == 0)
        return leave();
    if (!callTo(0x4e0430, { 1, camera2 + 0x38, car + 0x8ec, ebp - 0x40 }, pushWords(app, frameEsp, { ebp - 0x34 })))
        return;
    const std::optional<double> facing = dotAt(app, cpu, ebp, frameEsp, car + 0x904, view + 0x2c);
    if (!facing)
        return;
    const double dot = double(float(*facing));
    if (!(0.0 > dot))
        return leave();
    const double strength = M::mul(-dot, m.loadDouble(0x5400f8));
    if (!callTo(0x492370, { view },
                pushWords(app, frameEsp, { 0, m.word(ebp - 0x10), asBits(float(strength)), 0xffff0000,
                                           m.word(ebp - 0x2c), m.word(ebp - 0x30), m.word(ebp - 0x34) })))
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
void detailedCarLights(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc0; sub ebp, 0x82
    const x86::reg32 ebp = cpu.esp - 20 - 0x82;
    const x86::reg32 frameEsp = cpu.esp - 20 - 0xc0;
    const x86::reg32 car = cpu.eax, view = cpu.edx;
    const x86::reg32 model = m.word(car + 0x8a4);
    auto leave = [&]() { cpu.esp += 4; };
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers,
                      x86::reg32 esp = 0) { return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers); };
    // sub_49de20 for light `which` at `at` of the model, raised by the car's `raise`, its depth not wanted
    auto light = [&](x86::reg32 which, x86::reg32 at, x86::reg32 raise) {
        for (x86::reg32 k = 0; k < 0xc; k += 4)
            m.word(ebp + 0x52 + k, m.word(model + at + k));
        const double raised = M::add(m.load(ebp + 0x56), m.load(car + raise));
        const x86::reg32 esp = pushWords(app, frameEsp, { 0, view + 0x38, view + 0x44, ebp + 0x5e, ebp - 0x3e });
        m.store(ebp + 0x56, raised);
        return modelDepthAt(app, cpu, ebp, esp, false, { model, which, ebp + 0xa, ebp + 0x52 }).has_value();
    };

    if (!callTo(0x4ba810, { car, model }))
        return;
    x86::reg32 matrix = cpu.eax;
    if (!model || x86::sreg32(m.word(model + 0xf8)) <= 0)
        return leave();
    // port (apply_mirror_detail): the mirror as the main view
    if (gameSettings(app).night != 0 && mirrorAsMain(m.word(view)) != 1)
    {
        if (!callTo(0x4b97b0, { car, matrix }) || !callTo(0x4b9550, { car, cpu.eax }))
            return;
        matrix = cpu.eax;
    }
    if (!callTo(0x4ba320, { view, car, matrix }) || !callTo(0x4ba350, { view, car, matrix }))
        return;
    const x86::reg32 lights = cpu.eax;
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x3e + k, m.word(car + 0xc0 + k));
    m.word(ebp + 0x5e, m.word(car + 0x98));
    m.word(ebp + 0x62, m.word(car + 0x9c));
    const double up = M::add(m.load(ebp + 0x62), m.load(0x540124));
    m.word(ebp + 0x66, m.word(car + 0xa0));
    m.store(ebp + 0x62, up);
    if (!callTo(0x4b93c0, { car, ebp - 0x1a, ebp - 0x3e }))
        return;
    const x86::reg32 bodyWords = pushWords(app, frameEsp, { lights, view + 0x38, view + 0x44, ebp + 0x5e, ebp - 0x3e });
    const std::optional<double> bodyDepth =
        modelDepthAt(app, cpu, ebp, bodyWords, false, { model, 0, ebp - 0x1a, model + 0xfc });
    if (!bodyDepth)
        return;
    // kept where the code keeps it, next to the position the callees are handed
    app->getMemory<float>(ebp + 0x6a) = float(*bodyDepth);
    if (m.word(car + 0x660))
    {
        const x86::reg32 esp = pushWords(app, frameEsp, { lights, view + 0x38, view + 0x44, ebp + 0x5e, ebp - 0x3e });
        if (!modelDepthAt(app, cpu, ebp, esp, false, { model, 0xc, ebp - 0x1a, model + 0x18c }))
            return;
    }
    if (!callTo(0x4b9140, { car, ebp + 0xa, 1 }) || !callTo(0x4b90e0, { car, 0 }))
        return;
    if (!(m.byte(car + 0x99c) & 1) && !light(1, 0x108, 0x93c))
        return;
    if (!(m.byte(car + 0x99c) & 2) && !light(2, 0x114, 0x93c))
        return;
    if (!callTo(0x4b9280, { car, ebp + 0xa }) || !callTo(0x4b90e0, { car, 1 }))
        return;
    if (!(m.byte(car + 0x99c) & 8) && !light(3, 0x120, 0x940))
        return;
    if (!(m.byte(car + 0x99c) & 4) && !light(4, 0x12c, 0x940))
        return;
    if (!callTo(0x4e0280, { ebp - 0x1a, ebp - 0x3e, ebp + 0x2e })
        || !callTo(0x4b8740, { view, car, ebp + 0x2e, ebp + 0x5e },
                   pushWords(app, frameEsp, { matrix, m.word(ebp + 0x6a) }))
        || !callTo(0x4b81a0, { car, view }))
        return;
    leave();
}


namespace
{

/* sub_475b50's colours of a face by the eight lights of its look (`look`, at +0x30, 0x40 bytes
 * each: +0 kind, +4 on the face, +8 colour, +0xc on, +0x10 until, +0x18 position, +0x20 range,
 * +0x24 fading by its own +0, ..), summed in its frame (`ebp`) at ebp - 0x5a (red), - 0x4e
 * (green), - 0x72 (blue) from the colours at 0x474d20.., held to 255, to the three corners at
 * 0x55cfac: the face `faceIndex` of the letter, its corners' bytes at `face`, their normals from
 * `normals`, the faces' from `faceNormals`.  Its floats where the code keeps them in the frame.
 * False when the game stopped. */
bool eightLights(App* app, CPU& cpu, reg32 ebp, reg32 esp, reg32 look, reg8 faceIndex, reg32 face, reg32 normals,
                 reg32 faceNormals)
{
    const Memory m{app};
    auto byteAt = [&](reg32 address) { return reg32(m.byte(address)); };
    auto fild = [&](reg32 value) { return double(x86::sreg32(value)); };
    const reg32 sums[] = { ebp - 0x5a, ebp - 0x4e, ebp - 0x72 };  // red, green, blue: one float a corner
    for (reg32 k = 0; k < 0xc; k += 4)
    {
        m.word(ebp - 0x5a + k, m.word(0x474d20 + k));
        m.word(ebp - 0x4e + k, m.word(0x474d2c + k));
        m.word(ebp - 0x72 + k, m.word(0x474d38 + k));
    }
    // The light's colour times `light` added to corner `k`'s sums.
    auto addLight = [&](reg32 k, reg32 light) {
        const double value = m.load(light);
        const double red = M::mul(value, m.load(ebp + 2));
        const double green = M::mul(value, m.load(ebp + 6));
        const double blue = M::mul(value, m.load(ebp + 0xa));
        m.store(sums[0] + 4 * k, M::add(red, m.load(sums[0] + 4 * k)));
        m.store(sums[1] + 4 * k, M::add(green, m.load(sums[1] + 4 * k)));
        m.store(sums[2] + 4 * k, M::add(blue, m.load(sums[2] + 4 * k)));
    };
    if (m.word(look + 0x234))
    {
        for (reg32 l = 0; l < 8; ++l)
        {
            const reg32 light = look + (l << 6);
            if (m.word(light + 0x3c) == 0 || m.word(light + 0x30) != 0)
                continue;
            const reg32 until = m.word(light + 0x40);
            if (until != 0xffffffff && x86::sreg32(until) < x86::sreg32(m.word(0x7d3684)))
            {
                // Its time is up: off, and one fewer.
                m.word(light + 0x3c, 0);
                m.word(look + 0x230, m.word(look + 0x230) - 1);
                m.word(look + 0x234, m.word(look + 0x234) - 1);
                continue;
            }
            const reg32 colour = m.word(light + 0x38);
            m.store(ebp + 2, fild((colour & 0xff0000) >> 16));
            m.store(ebp + 6, fild((colour & 0xff00) >> 8));
            m.store(ebp + 0xa, fild(colour & 0xff));
            if (m.word(light + 0x34))
            {
                // On each corner.
                for (reg32 k = 0; k < 3; ++k)
                {
                    const std::optional<double> dot =
                        dotAt(app, cpu, ebp, esp, normals + byteAt(face + k) * 12, light + 0x58);
                    if (!dot)
                        return false;
                    m.store(ebp + 0x2a, *dot);
                    if (!(0.0 >= m.load(ebp + 0x2a)))
                        addLight(k, ebp + 0x2a);
                }
                continue;
            }
            // On the face: the same to all three corners.
            const std::optional<double> dot =
                dotAt(app, cpu, ebp, esp, faceNormals + reg32(faceIndex) * 12, light + 0x58);
            if (!dot)
                return false;
            m.store(ebp + 0x36, *dot);
            if (0.0 >= m.load(ebp + 0x36))
                continue;
            for (reg32 k = 0; k < 3; ++k)
                addLight(k, ebp + 0x36);
        }
        if (x86::sreg32(m.word(look + 0x234)) > 1)
        {
            // Shared by the lights on.
            m.store(ebp + 0x1a, M::div(1.0, fild(m.word(look + 0x234))));
            for (reg32 k = 0; k < 3; ++k)
            {
                const double share = m.load(ebp + 0x1a);
                const double red = M::mul(m.load(sums[0] + 4 * k), share);
                const double green = M::mul(m.load(sums[1] + 4 * k), share);
                const double blue = M::mul(share, m.load(sums[2] + 4 * k));
                m.store(sums[0] + 4 * k, red);
                m.store(sums[1] + 4 * k, green);
                m.store(sums[2] + 4 * k, blue);
            }
        }
    }

    // `dot` times the spot's colour and `inverse` added to corner `k`'s sums, each held to 255.
    auto addSpot = [&](reg32 k, reg32 dot, reg32 inverse) {
        const reg32 colours[] = { ebp + 0x1e, ebp + 0x32, ebp + 0x2e };
        for (int c = 0; c < 3; ++c)
        {
            const double add = M::mul(M::mul(m.load(dot), m.load(colours[c])), m.load(inverse));
            const reg32 to = sums[c] + 4 * k;
            m.store(to, M::add(add, m.load(to)));
            if (x86::sreg32(m.word(to)) > 0x437f0000)
                m.word(to, 0x437f0000);
        }
    };
    /* The spot's direction to the point at ebp - 0x66, from sub_4e0090's distance: 1 over it to
     * `inverse`, the direction made a unit by sub_4ea920's square root.  False when the game
     * stopped. */
    auto direction = [&](reg32 spot, reg32 inverse) -> bool {
        if (!callAt(app, cpu, ebp, esp, 0x4e0090, { spot + 0x58, ebp - 0x66 }))
            return false;
        const double distance = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        const double over = M::div(1.0, distance);
        m.store(ebp - 0x66, M::sub(m.load(spot + 0x58), m.load(ebp - 0x66)));
        const double dy = M::sub(m.load(ebp - 0x62), m.load(spot + 0x5c));
        m.store(ebp - 0x62, dy);
        const double dy2 = M::mul(dy, m.load(ebp - 0x62));
        const double dx2 = M::mul(m.load(ebp - 0x66), m.load(ebp - 0x66));
        const reg32 pushed = pushWords(app, esp, { asBits(float(M::add(dy2, dx2))) });
        m.store(inverse, over);
        const std::optional<double> root = squareRootAt(app, cpu, ebp, pushed);
        if (!root)
            return false;
        const double unit = M::div(1.0, *root);
        const double dx = M::mul(m.load(ebp - 0x66), unit);
        const double dyUnit = M::mul(unit, m.load(ebp - 0x62));
        m.store(ebp - 0x66, dx);
        m.store(ebp - 0x62, dyUnit);
        return true;
    };
    // The normal at `normal` against the direction at ebp - 0x66, to `to`.
    auto facing = [&](reg32 normal, reg32 to) {
        m.store(to, M::add(M::mul(m.load(normal + 4), m.load(ebp - 0x62)), M::mul(m.load(normal), m.load(ebp - 0x66))));
        return !(0.0 >= m.load(to));
    };
    if (m.word(look + 0x230) - m.word(look + 0x234))
    {
        // The lights of kinds 1 and 2 that are not off by +0x24.
        for (reg32 l = 0; l < 8; ++l)
        {
            const reg32 spot = look + (l << 6);
            if (m.word(spot + 0x3c) == 0)
                continue;
            const reg32 kind = m.word(spot + 0x30);
            if ((kind != 1 && kind != 2) || m.word(spot + 0x54) != 0)
                continue;
            const reg32 fading = m.word(spot + 0x4c);
            if (m.word(spot + 0x30) == 2 && fading)
            {
                // Fading: its colour times what [+0x4c] holds over +0x50.
                m.store(ebp - 0x16, M::div(1.0, m.load(spot + 0x50)));
                m.word(ebp + 0x12, m.word(fading));
                if (0.0 > m.load(ebp + 0x12))
                    continue;
                const bool within = !(m.load(ebp + 0x12) >= m.load(spot + 0x50));
                const reg32 colour = m.word(spot + 0x38);
                if (within)
                {
                    const double red = fild((colour & 0xff0000) >> 16);
                    const double k = M::mul(m.load(ebp + 0x12), m.load(ebp - 0x16));
                    const double redK = M::mul(red, k);
                    const double greenK = M::mul(fild((colour & 0xff00) >> 8), k);
                    const double blueK = M::mul(k, fild(colour & 0xff));
                    m.store(ebp + 0x1e, redK);
                    m.store(ebp + 0x32, greenK);
                    m.store(ebp + 0x2e, blueK);
                }
                else
                {
                    m.store(ebp + 0x1e, fild((colour & 0xff0000) >> 16));
                    m.store(ebp + 0x32, fild((colour & 0xff00) >> 8));
                    m.store(ebp + 0x2e, fild(colour & 0xff));
                }
            }
            else
            {
                const reg32 colour = m.word(spot + 0x38);
                m.store(ebp + 0x1e, fild((colour & 0xff0000) >> 16));
                m.store(ebp + 0x32, fild((colour & 0xff00) >> 8));
                m.store(ebp + 0x2e, fild(colour & 0xff));
            }
            if (m.word(spot + 0x34) == 0)
            {
                // On the face, from its middle on the screen.
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
                m.store(ebp - 0x66, x);
                m.store(ebp - 0x62, y);
                m.store(ebp - 0x5e, w);
                if (!direction(spot, ebp + 0xe))
                    return false;
                if (!facing(faceNormals + reg32(faceIndex) * 12, ebp + 0x26))
                    continue;
                for (reg32 k = 0; k < 3; ++k)
                    addSpot(k, ebp + 0x26, ebp + 0xe);
                continue;
            }
            // On each corner, from where it is on the screen.
            for (reg32 k = 0; k < 3; ++k)
            {
                const reg32 corner = 0x55cf9c + (k << 5);
                const double x = M::mul(m.load(corner), m.load(0x53b090));
                const double y = M::mul(m.load(corner + 4), m.load(0x53b094));
                m.word(ebp - 0x5e, m.word(corner + 8));
                m.store(ebp - 0x66, x);
                m.store(ebp - 0x62, y);
                if (!direction(spot, ebp + 0x16))
                    return false;
                if (facing(normals + byteAt(face + k) * 12, ebp + 0x22))
                    addSpot(k, ebp + 0x22, ebp + 0x16);
            }
        }
    }
    // The sums to the corners' colours, opaque.
    for (reg32 k = 0; k < 3; ++k)
    {
        reg32 channel[3];
        for (int c = 0; c < 3; ++c)
        {
            const std::optional<double> whole = chopAt(app, cpu, ebp, esp, m.load(sums[c] + 4 * k));
            if (!whole)
                return false;
            channel[c] = reg32(fp::toInteger<x86::sreg32>(x86::Float(*whole)));
        }
        m.word(0x55cfac + (k << 5),
               0xff000000 | ((channel[0] << 16) & 0xff0000) | ((channel[1] << 8) & 0xff00) | (channel[2] & 0xff));
    }
    return true;
}

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
void solidText(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x58e4; sub ebp, 0x72
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebp = entry - 16 - 0x72;
    const x86::reg32 frameEsp = entry - 16 - 0x58e4;
    // The letter's turned vertices, then its normals and its faces' normals.
    const x86::reg32 rotated = ebp - 0x5872;
    // The vertices on the screen, 20 bytes each: x, y, depth, s, t.
    const x86::reg32 screen = ebp - 0x2872;
    const x86::reg32 scale = entry + 4, size = entry + 8, texture = entry + 0xc, matrix = entry + 0x10;
    const x86::reg32 corners = 0x55cf9c;  // THRASH's three, 0x20 bytes each: x, y, 1/w, depth, colour, s, t
    x86::reg32 text = cpu.eax;
    x86::reg16 penX = x86::reg16(cpu.edx), penY = x86::reg16(cpu.ebx);
    auto leave = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.esp += 4 + 0x10;
    };
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp = 0) {
        return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers);
    };
    auto fild32 = [&](x86::reg32 value) { return double(x86::sreg32(value)); };
    auto fild16 = [&](x86::reg16 value) { return double(x86::sreg16(value)); };
    auto byteAt = [&](x86::reg32 address) { return x86::reg32(m.byte(address)); };
    // sub_4dfd56 on `value`, then fistp
    std::optional<x86::reg32> truncated;
    auto truncate = [&](double value) {
        const std::optional<double> whole = chopAt(app, cpu, ebp, frameEsp, value);
        if (!whole)
            return false;
        truncated = x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(*whole)));
        return true;
    };
    // The pen moved on by `width`.
    auto advancePen = [&](double width) {
        if (!truncate(M::add(width, fild16(penX))))
            return false;
        penX = x86::reg16(*truncated);
        return true;
    };
    // A float sub_4e0090 leaves on the FPU (simd's vectorDistance, which has no core giving it back).
    auto distanceTo = [&](x86::reg32 position, x86::reg32 point) -> std::optional<double> {
        if (!callTo(0x4e0090, { position, point }))
            return std::nullopt;
        const double value = double(cpu.fpu.st(0));
        cpu.fpu.count -= 1;
        return value;
    };
    // The colour of the channels `red`, `green` and `blue` (0..255) times `light`, chopped, under `top`.
    auto lit = [&](x86::reg32 top, x86::reg32 red, x86::reg32 green, x86::reg32 blue,
                   double light) -> std::optional<x86::reg32> {
        if (!truncate(M::mul(fild32(red), light)))
            return std::nullopt;
        const x86::reg32 r = (*truncated << 16) & 0xff0000;
        const double g = M::mul(fild32(green), light);
        const double b = M::mul(light, fild32(blue));
        if (!truncate(g))
            return std::nullopt;
        const x86::reg32 gr = (*truncated << 8) & 0xff00;
        if (!truncate(b))
            return std::nullopt;
        return (top & 0xff000000) | r | gr | (*truncated & 0xff);
    };

    const x86::reg32 lookIndex = m.word(0x55b088);
    const x86::reg32 look = 0x55b08c + lookIndex * 0x238;
    if (text == 0 || m.word(0x679010) == 0 || m.word(0x55b088) == 0xffffffff)
        return leave(lookIndex * 0x238);
    const x86::reg32 lookTexture = m.word(look + 0x2c);
    if (lookTexture)
    {
        if (!callTo(0x431900, { 1, m.word(lookTexture + 4) }) || !callTo(0x431900, { 0xd, 1 }))
            return;
    }
    else
    {
        m.word(0x55cfb8, 0);
        m.word(0x55cfd8, 0);
        m.word(0x55cfb4, 0);
        m.word(0x55cfd4, 0x3f800000);
        m.word(0x55cff4, 0x3f800000);
        m.word(0x55cff8, 0x3f800000);
        const x86::reg32 handle = m.word(texture);
        if (!callTo(0x431900, { 1, handle ? m.word(handle + 4) : 0 }))
            return;
    }
    if (!truncate(M::add(M::mul(m.load(size), m.load(m.word(0x679010) + 8)), fild16(penY))))
        return;
    penY = x86::reg16(*truncated);

    if (!(m.word(scale) & 0x7fffffff))
    {
        // Flat letters: the colour +0x14, depth 1, 1/w 0.
        const x86::reg32 colour = m.word(look + 0x14);
        m.word(0x55cfac, colour);
        m.word(0x55cfcc, colour);
        m.word(0x55cfec, colour);
        m.word(0x55cffc, m.word(0x55cffc) + 1);
        for (;; ++text)
        {
            if (m.byte(text) == 0)
                return leave(text);
            const x86::reg32 font = m.word(0x679010);
            const x86::reg32 c = byteAt(text);
            double width;
            if (c >= byteAt(font) && x86::sreg32(c) < x86::sreg32(byteAt(font) + byteAt(font + 1)))
            {
                const x86::reg32 part = font + 0x10 + (c << 5);
                for (x86::reg8 faceIndex = 0;
                     faceIndex < m.byte((byteAt(text) << 5) + m.word(0x679010) + 0x10); ++faceIndex)
                {
                    const x86::reg32 face = m.word(part + 0x1c) + x86::reg32(faceIndex) * 3;
                    const x86::reg32 vertices = m.word(part + 0x10);
                    const x86::reg32 v0 = vertices + byteAt(face) * 12;
                    const double letterSize = m.load(size);
                    const double x0 = M::mul(m.load(v0), letterSize);
                    const x86::reg32 v1 = vertices + byteAt(face + 1) * 12;
                    const double y0 = M::mul(m.load(v0 + 4), letterSize);
                    const double x1 = M::mul(m.load(v1), letterSize);
                    const double y1 = M::mul(m.load(v1 + 4), letterSize);
                    const double y = fild16(penY);
                    const double x = double(float(fild16(penX)));
                    m.store(0x55cf9c, M::add(x0, x));
                    m.store(0x55cfa0, M::sub(y, y0));
                    const x86::reg32 v2 = vertices + byteAt(face + 2) * 12;
                    m.store(0x55cfbc, M::add(x1, x));
                    m.store(0x55cfc0, M::sub(y, y1));
                    const double x2 = M::mul(m.load(v2), letterSize);
                    const double y2 = M::mul(letterSize, m.load(v2 + 4));
                    for (const x86::reg32 k : { 0u, 0x20u, 0x40u })
                        m.word(0x55cfa8 + k, 0x3f800000);
                    for (const x86::reg32 k : { 0u, 0x20u, 0x40u })
                        m.word(0x55cfa4 + k, 0);
                    m.store(0x55cfdc, M::add(x2, x));
                    m.store(0x55cfe0, M::sub(y, y2));
                    if (!driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu,
                        frameEsp, 0x55cf9c, 0x55cfbc, 0x55cfdc).has_value())
                        return;
                }
                width = M::mul(m.load(size), m.load(part + 4));
            }
            else
                width = M::mul(m.load(size), m.load(0x53b098));
            if (!advancePen(width))
                return;
        }
    }

    const double inverseScale = double(float(M::div(1.0, m.load(scale))));
    {
        const double depthScale = m.load(scale);
        const double fontScale = m.load(m.word(0x679010) + 4);
        if (depthScale < fontScale || depthScale > fontScale)
        {
            // A new scale: every letter's vertices get it at +8, the font at +4.
            x86::reg32 c = byteAt(m.word(0x679010));
            x86::reg32 font;
            for (;; ++c)
            {
                font = m.word(0x679010);
                if (x86::sreg32(c) >= x86::sreg32(byteAt(font + 1) + byteAt(font)))
                    break;
                const x86::reg32 part = (c << 5) + font + 0x10;
                for (x86::reg32 v = byteAt(part + 2); x86::sreg32(v) < x86::sreg32(byteAt(part + 3)); ++v)
                    m.word(m.word(part + 0x10) + v * 12 + 8, m.word(scale));
            }
            m.word(font + 4, m.word(scale));
        }
    }

    // What the code keeps in the frame from letter to letter.
    x86::reg32 normals = m.word(ebp + 0x3a), faceNormals = m.word(ebp + 0x4a);

    for (;; ++text)
    {
        if (m.byte(text) == 0)
            break;
        const x86::reg32 font = m.word(0x679010);
        const x86::reg32 c = byteAt(text);
        if (c < byteAt(font) || x86::sreg32(c) >= x86::sreg32(byteAt(font) + byteAt(font + 1)))
        {
            if (!advancePen(M::mul(m.load(size), m.load(0x53b098))))
                return;
            continue;
        }
        const x86::reg32 part = font + 0x10 + (c << 5);
        const x86::reg32 turning = m.word(matrix);
        if (!turning)
        {
            // No matrix: the rest drawn flat by a call of its own, this letter first.
            const x86::reg32 esp = pushWords(app, frameEsp, { 0, m.word(texture), m.word(size), 0 });
            if (!callTo(0x475b50, { text, x86::reg32(x86::sreg32(x86::sreg16(penX))),
                                    x86::reg32(x86::sreg32(x86::sreg16(penY))) }, esp))
                return;
            continue;
        }

        // The letter's vertices turned (sub_4e03b0) into the frame's buffer, its normals after them,
        // its faces' normals after those.
        auto turn = [&](x86::reg32 count, x86::reg32 from, x86::reg32 to) {
            return callTo(0x4e03b0, { count, from, turning, to });
        };
        auto turnFaces = [&]() { return turn(byteAt(part + 1), m.word(part + 0x14), faceNormals); };
        const x86::reg32 kind = m.word(look);
        if (kind == 2)
        {
            if (!turn(byteAt(part + 3) * 2, m.word(part + 0x10), rotated))
                return;
            normals = rotated + byteAt(part + 3) * 12;
        }
        else if (kind == 3 && m.word(look + 0x2c) == 0)
        {
            if (!turn(byteAt(part + 3), m.word(part + 0x10), rotated))
                return;
            faceNormals = rotated + byteAt(part + 3) * 12;
            if (!turnFaces())
                return;
        }
        else
        {
            const x86::reg32 kind2 = m.word(look);
            const bool both = kind2 == 4 || (kind2 == 3 && m.word(look + 0x2c) != 0);
            if (!both)
            {
                if (!turn(byteAt(part + 3), m.word(part + 0x10), rotated))
                    return;
            }
            else
            {
                if (!turn(byteAt(part + 3) * 2, m.word(part + 0x10), rotated))
                    return;
                const x86::reg32 n = byteAt(part + 3);
                normals = rotated + n * 12;
                faceNormals = rotated + n * 24;
                if (!turnFaces())
                    return;
            }
        }

        // The vertices onto the screen: x, y, depth, and s, t when textured.
        for (x86::reg32 v = 0; x86::sreg32(v) < x86::sreg32(byteAt(part + 3)); ++v)
        {
            const x86::reg32 from = rotated + v * 12;
            const x86::reg32 to = screen + v * 20;
            const double letterSize = m.load(size);
            const double x = M::mul(m.load(from), letterSize);
            m.store(to, M::add(x, fild16(penX)));
            const double y = M::mul(letterSize, m.load(from + 4));
            m.store(to + 4, M::sub(fild16(penY), y));
            const double scaled =
                M::mul(M::add(M::mul(m.load(from + 8), inverseScale), m.load(0x53b06c)), m.load(0x53b070));
            const double depth = M::sub(1.0, scaled);
            m.store(to + 8, depth);
            if (!(1.0 >= double(float(depth))))
                m.word(to + 8, 0x3f800000);
            if (!(m.load(to + 8) >= m.loadDouble(0x53b078)))
                m.word(to + 8, 0x37800080);
            if (m.word(look + 0x2c) != 0)
            {
                const x86::reg32 normal = normals + v * 12;
                const std::optional<double> angle =
                    angleAt(app, cpu, ebp, pushWords(app, frameEsp, { m.word(normal + 4), m.word(normal + 8) }));
                if (!angle)
                    return;
                m.store(to + 0xc, M::add(*angle, m.load(0x53b080)));
                m.store(to + 0x10, M::mul(m.load(0x53b080), M::add(1.0, m.load(normal))));
            }
        }

        // The faces, three vertex bytes each from [part + 0x1c].
        for (x86::reg8 faceIndex = 0; faceIndex < m.byte(m.word(0x679010) + (byteAt(text) << 5) + 0x11); ++faceIndex)
        {
            const x86::reg32 face = m.word(part + 0x1c) + x86::reg32(faceIndex) * 3;
            for (x86::reg32 k = 0; k < 3; ++k)
            {
                const x86::reg32 from = screen + byteAt(face + k) * 20;
                const x86::reg32 to = corners + (k << 5);
                m.store(to, m.load(from));
                m.store(to + 4, m.load(from + 4));
                m.store(to + 0xc, m.load(from + 8));
                if (m.word(look + 0x2c) != 0)
                {
                    m.word(to + 0x18, m.word(from + 0xc));
                    m.word(to + 0x1c, m.word(from + 0x10));
                }
            }
            {
                // Facing the screen: (y2 - y1)(x0 - x1) above (x2 - x1)(y0 - y1).
                const double a =
                    M::mul(M::sub(m.load(0x55cfdc), m.load(0x55cfbc)), M::sub(m.load(0x55cfa0), m.load(0x55cfc0)));
                const double b =
                    M::mul(M::sub(m.load(0x55cfe0), m.load(0x55cfc0)), M::sub(m.load(0x55cf9c), m.load(0x55cfbc)));
                if (!(b > a))
                    continue;
            }
            // The face's normal, and each corner's.
            auto faceNormal = [&]() { return faceNormals + x86::reg32(faceIndex) * 12; };
            auto cornerNormal = [&](x86::reg32 k) { return normals + byteAt(face + k) * 12; };
            const x86::reg32 colours = m.word(look + 0x14);
            switch (m.word(look))
            {
            case 0:
            {
                // The letter's first faces in +8, the next as many in +0xc, the rest in +0x10.
                const x86::reg8 first = m.byte((byteAt(text) << 5) + m.word(0x679010) + 0x10);
                const bool second = x86::sreg32(x86::reg32(faceIndex)) < x86::sreg32(x86::reg32(first) * 2);
                const x86::reg32 field = faceIndex < first ? 8 : second ? 0xc : 0x10;
                const x86::reg32 colour = m.word(look + field);
                m.word(0x55cfac, colour);
                m.word(0x55cfcc, colour);
                m.word(0x55cfec, colour);
                break;
            }
            case 1:
            {
                // Each corner: +8 nearer than +0x18, +0xc beyond +0x1c, +0x14 at 0, between them
                // +0x14 blended with +0xc or +8 by the depth.
                const x86::reg32 near = m.word(look + 8), far = m.word(look + 0xc);
                for (x86::reg32 k = 0; k < 3; ++k)
                {
                    const x86::reg32 vertex = rotated + byteAt(face + k) * 12;
                    const double z = m.load(vertex + 8);
                    x86::reg32 colour;
                    if (!(z > m.load(look + 0x18)))
                        colour = m.word(look + 8);
                    else if (z >= m.load(look + 0x1c))
                        colour = m.word(look + 0xc);
                    else if (!(m.word(vertex + 8) & 0x7fffffff))
                        colour = m.word(look + 0x14);
                    else
                    {
                        const bool beyond = !(0.0 < z);
                        const x86::reg32 blend = beyond ? near : far;
                        const double t = M::div(m.load(vertex + 8), m.load(look + (beyond ? 0x18 : 0x1c)));
                        const double u = M::sub(1.0, t);
                        const double fogRed = M::mul(fild32((colours >> 16) & 0xff), u);
                        if (!truncate(M::add(fogRed, M::mul(fild32((blend >> 16) & 0xff), t))))
                            return;
                        const x86::reg32 top = (m.word(look + 0x14) & 0xff000000) | ((*truncated << 16) & 0xff0000);
                        const double fogGreen = M::mul(fild32((colours >> 8) & 0xff), u);
                        const double green = M::add(fogGreen, M::mul(fild32((blend >> 8) & 0xff), t));
                        const double fogBlue = M::mul(u, fild32(colours & 0xff));
                        if (!truncate(green))
                            return;
                        const x86::reg32 greenBits = (*truncated << 8) & 0xff00;
                        if (!truncate(M::add(M::mul(t, fild32(blend & 0xff)), fogBlue)))
                            return;
                        colour = top | greenBits | (*truncated & 0xff);
                    }
                    m.word(0x55cfac + (k << 5), colour);
                }
                break;
            }
            case 2:
                // Each corner lit by its normal against the light at +0x20.
                for (x86::reg32 k = 0; k < 3; ++k)
                {
                    const std::optional<double> dot = dotAt(app, cpu, ebp, frameEsp, cornerNormal(k), look + 0x20);
                    if (!dot)
                        return;
                    const double light = 0.0 > double(float(*dot)) ? 0.0 : double(float(*dot));
                    const std::optional<x86::reg32> colour =
                        lit(m.word(look + 0x14), (colours >> 16) & 0xff, (colours >> 8) & 0xff, colours & 0xff, light);
                    if (!colour)
                        return;
                    m.word(0x55cfac + (k << 5), *colour);
                }
                break;
            case 3:
            {
                // The face lit by its normal against the light at +0x20.
                const std::optional<double> dot = dotAt(app, cpu, ebp, frameEsp, faceNormal(), look + 0x20);
                if (!dot)
                    return;
                const double light = 0.0 > double(float(*dot)) ? 0.0 : double(float(*dot));
                const std::optional<x86::reg32> colour =
                    lit(m.word(look + 0x14), (colours >> 16) & 0xff, (colours >> 8) & 0xff, colours & 0xff, light);
                if (!colour)
                    return;
                m.word(0x55cfac, *colour);
                m.word(0x55cfcc, *colour);
                m.word(0x55cfec, *colour);
                break;
            }
            case 4:
                if (!eightLights(app, cpu, ebp, frameEsp, look, faceIndex, face, normals, faceNormals))
                    return;
                break;
            default:
                break;
            }

            // 1/w = 1 - depth at +8, then the triangle.
            const double w0 = M::sub(1.0, m.load(0x55cfa8));
            const double w1 = M::sub(1.0, m.load(0x55cfc8));
            const double w2 = M::sub(1.0, m.load(0x55cfe8));
            m.store(0x55cfa4, w0);
            m.store(0x55cfc4, w1);
            m.store(0x55cfe4, w2);
            if (!driverAt<thrash::kThrashDrawTri, &port::driver::drawTri>(app, cpu,
                frameEsp, 0x55cf9c, 0x55cfbc, 0x55cfdc).has_value())
                return;
        }
        if (!advancePen(M::mul(m.load(size), m.load(part + 4))))
            return;
    }

    if (m.word(look + 0x2c) != 0)
    {
        if (!callTo(0x431900, { 0xd, 0 }))
            return;
        return leave(cpu.eax);
    }
    leave(look);
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
void mediumCarLights(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc8; sub ebp, 0x82
    const x86::reg32 ebp = cpu.esp - 20 - 0x82;
    const x86::reg32 frameEsp = cpu.esp - 20 - 0xc8;
    const x86::reg32 car = cpu.eax, view = cpu.edx;
    const x86::reg32 model = m.word(car + 0x8a4);
    auto leave = [&]() { cpu.esp += 4; };
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers,
                      x86::reg32 esp = 0) { return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers); };
    // sub_49de20 for light `which` at `at` of the model, raised by the car's `raise`, its depth not wanted
    auto light = [&](x86::reg32 which, x86::reg32 at, x86::reg32 raise) {
        for (x86::reg32 k = 0; k < 0xc; k += 4)
            m.word(ebp + 0x4a + k, m.word(model + at + k));
        const double raised = M::add(m.load(ebp + 0x4e), m.load(car + raise));
        const x86::reg32 esp = pushWords(app, frameEsp, { 0, view + 0x38, view + 0x44, ebp + 0x56, ebp - 0x46 });
        m.store(ebp + 0x4e, raised);
        return modelDepthAt(app, cpu, ebp, esp, false, { model, which, ebp + 2, ebp + 0x4a }).has_value();
    };

    if (!callTo(0x4206f0, { car + 0xc, 1 }))
        return;
    x86::reg32 matrix = cpu.eax;
    if (!model || x86::sreg32(m.word(model + 0xf8)) <= 0)
        return leave();
    // port (apply_mirror_detail): the mirror as the main view
    if (gameSettings(app).night != 0 && mirrorAsMain(m.word(view)) != 1)
    {
        if (!callTo(0x4b97b0, { car, matrix }))
            return;
        matrix = cpu.eax;
    }
    if (!callTo(0x4ba320, { view, car, matrix }) || !callTo(0x4ba350, { view, car, matrix }))
        return;
    // the words after the position are kept where the code keeps them
    m.word(ebp + 0x62, cpu.eax);
    for (x86::reg32 k = 0; k < 0x24; k += 4)
        m.word(ebp - 0x46 + k, m.word(car + 0xc0 + k));
    m.word(ebp + 0x56, m.word(car + 0x98));
    m.word(ebp + 0x5a, m.word(car + 0x9c));
    const double up = M::add(m.load(ebp + 0x5a), m.load(0x54012c));
    m.word(ebp + 0x5e, m.word(car + 0xa0));
    m.store(ebp + 0x5a, up);
    if (!callTo(0x4b93c0, { car, ebp - 0x22, ebp - 0x46 }))
        return;
    const x86::reg32 bodyWords =
        pushWords(app, frameEsp, { m.word(ebp + 0x62), view + 0x38, view + 0x44, ebp + 0x56, ebp - 0x46 });
    const std::optional<double> bodyDepth =
        modelDepthAt(app, cpu, ebp, bodyWords, false, { model, 0, ebp - 0x22, model + 0xfc });
    if (!bodyDepth)
        return;
    app->getMemory<float>(ebp + 0x66) = float(*bodyDepth);
    if (!callTo(0x4b9140, { car, ebp + 2, 1 }) || !callTo(0x4b90e0, { car, 0 }) || !light(1, 0x108, 0x93c)
        || !light(2, 0x114, 0x93c) || !callTo(0x4b9280, { car, ebp + 2 }) || !callTo(0x4b90e0, { car, 1 })
        || !light(3, 0x120, 0x940) || !light(4, 0x12c, 0x940))
        return;
    if (!callTo(0x4e0280, { ebp - 0x22, ebp - 0x46, ebp + 0x26 })
        || !callTo(0x4b8740, { view, car, ebp + 0x26, ebp + 0x56 },
                   pushWords(app, frameEsp, { matrix, m.word(ebp + 0x66) }))
        || !callTo(0x4b81a0, { car, view }))
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
void nightColour(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x68
    const x86::reg32 ebp = cpu.esp - 20;
    const x86::reg32 frameEsp = cpu.esp - 20 - 0x68;
    const x86::reg32 car = cpu.eax, colour = cpu.edx;
    // fistp
    auto integer = [](double value) { return x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(value))); };
    // |[from]|: fld, fchs when below 0
    auto magnitude = [&](x86::reg32 from) {
        const double value = m.load(from);
        return 0.0 > value ? -value : value;
    };
    // The point at ebp - 0x50 into the light's space at `to` by sub_4e0430; true when it is lit:
    // its z (`to` + 8) from [light+0x64] to [light+0x6c], x and y within z either way.
    auto inBeam = [&](x86::reg32 light, x86::reg32 to) {
        const double z = m.load(to + 8);
        return z >= m.load(light + 0x64) && !(z > m.load(light + 0x6c)) && !(magnitude(to) > z)
               && !(magnitude(to + 4) > z);
    };
    // The light's colour at `weight` (a float) added to the sums.
    x86::reg32 count = 0, red = 0, green = 0, blue = 0;
    auto add = [&](x86::reg32 light, double weight) {
        red += integer(M::mul(double(x86::sreg32((light >> 16) & 0xff)), weight));
        green += integer(M::mul(double(x86::sreg32((light >> 8) & 0xff)), weight));
        blue += integer(M::mul(double(x86::sreg32(light & 0xff)), weight));
        ++count;
    };

    // The point: the car's position, at ebp - 0x50 as sub_4e0430 takes it.
    for (x86::reg32 k = 0; k < 0xc; k += 4)
        m.word(ebp - 0x50 + k, m.word(car + 0x98 + k));
    for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(m.word(0x5efd9c)); ++i)
    {
        const x86::reg32 other = m.word(i * 4 + 0x5efac8);
        const x86::reg8 state = m.byte(other + 0x200);
        if ((state & 0x40) || !m.byte(other + 0x89) || m.byte(other + 0x88) || other == car)
            continue;
        if ((state & 0x20) && m.word(other + 0x734) != 0)
        {
            const x86::reg32 light = other + 0x734;
            x86::reg32 lightColour = m.word(light + 0x70);
            if (!callAt(app, cpu, ebp, pushWords(app, frameEsp, { ebp - 0x68 }), 0x4e0430,
                        { 1, ebp - 0x50, light + 0x28, light + 0x58 }))
                return;
            if (0.0 > m.load(ebp - 0x60))
            {
                // Behind it: z the other way, the rear colour.
                m.byte(ebp - 0x5d, m.byte(ebp - 0x5d) ^ 0x80);
                lightColour = m.word(light + 0x74);
            }
            if (!inBeam(light, ebp - 0x68))
                continue;
            const double weight = double(float(M::sub(1.0, M::div(m.load(ebp - 0x60), m.load(light + 0x6c)))));
            add(lightColour, weight);
        }
        if (m.word(other + 0x660) == 0)
            continue;
        const x86::reg32 light = other + 0x660;
        if (!callAt(app, cpu, ebp, pushWords(app, frameEsp, { ebp - 0x5c }), 0x4e0430,
                    { 1, ebp - 0x50, other + 0x688, other + 0x6b8 }))
            return;
        if (!inBeam(light, ebp - 0x5c))
            continue;
        const double ahead = double(float(M::sub(1.0, M::div(m.load(ebp - 0x54), m.load(light + 0x6c)))));
        const double across = M::div(double(float(magnitude(ebp - 0x5c))), m.load(ebp - 0x54));
        const x86::reg32 lightColour = m.word(light + 0x70);
        add(lightColour, double(float(M::mul(M::sub(1.0, across), ahead))));
    }
    x86::reg32 result = colour;
    if (count != 0)
    {
        const x86::sreg32 n = x86::sreg32(count);
        // A channel of the colour plus the average, held to 0..255.
        auto channel = [&](x86::reg32 value, x86::reg32 sum) {
            const x86::sreg32 v = x86::sreg32(value + x86::reg32(x86::sreg32(sum) / n));
            return x86::reg32(v < 0 ? 0 : v > 0xff ? 0xff : v);
        };
        result = 0xff000000 | (channel((colour >> 16) & 0xff, red) << 16) | (channel((colour >> 8) & 0xff, green) << 8)
                 | channel(colour & 0xff, blue);
    }
    cpu.eax = result;
    cpu.esp += 4;
}

/* sub_41b9b0: a view's pass (view in eax): the glare colour at 0x552e1c --
 * grey, darkened by the sun's glare [0x6fbc38] when that is below 1 (on
 * every driver, a port) -- then sub_41aaf0 for the view, the cars' sub_42d160
 * for the main view, sub_41cdc0 with the track's record, the four
 * sub_4c7790s, sub_41b8c0, the two sub_4c7940s, sub_4217e0 and sub_494170. */
void viewPass(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, ebp; mov ebp, esp; sub esp, 0x38
    const x86::reg32 ebp = cpu.esp - 20;
    const x86::reg32 frameEsp = cpu.esp - 20 - 0x38;
    const x86::reg32 view = cpu.eax;
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp = 0) {
        return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers);
    };

    m.word(0x552e1c, 0x7f7f7f7f);
    // port (apply_alpha_intensity): the glare on every driver
    const double glare = double(gameSettings(app).alphaIntensity);
    if (1.0 > glare)
    {
        const x86::reg32 scale =
            x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x536cac)))));
        m.word(0x552e1c, darkened(m.word(0x552e1c), scale));
    }
    const x86::reg32 kind = m.word(view);
    if ((kind == 0 && !callTo(0x41aaf0, { view, m.word(view + 4) })) || (kind == 1 && !callTo(0x41aaf0, { view, 1 })))
        return;
    if (m.word(view) == 0 && m.word(view + 4) == 0)
        for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(m.word(0x5efd9c)); ++i)
        {
            const x86::reg32 car = m.word(i * 4 + 0x5efac8);
            if (!(m.byte(car + 0x200) & 0x40) && !m.byte(car + 0x88) && !callTo(0x42d160, { car }))
                return;
        }
    const x86::reg32 track = m.word(m.word(0x5dd830) + 0x10) * 0x5c0 + 0x571370;
    if (!callTo(0x41cdc0, { view, track + 0xb4, ebp - 0x38 }))
        return;
    const x86::reg32 slots[] = { 0x7dcfe8, 0x7dcff4, 0x7dcff0, 0x7dcfe4 };
    const x86::reg32 values[] = { 0x38, 0x30, 0x20, 0x28 };
    for (int k = 0; k < 4; ++k)
        if (!callTo(0x4c7790, { m.word(ebp - values[k]) }, pushWords(app, frameEsp, { m.word(slots[k]) })))
            return;
    if (!callTo(0x41b8c0, { ebp - 0x10 })
        || !callTo(0x4c7940, {}, pushWords(app, frameEsp, { m.word(0x7dcff8), m.word(ebp - 0xc), m.word(ebp - 0x10) }))
        || !callTo(0x4c7940, {}, pushWords(app, frameEsp, { m.word(0x7dcff8), m.word(ebp - 0x14), m.word(ebp - 0x18) }))
        || !callTo(0x4217e0, { m.word(view + 4) }) || !callTo(0x494170, { view, cpu.eax }))
        return;
    cpu.esp += 4;
}

/* sub_434870's core: the polygon lists' drawers for the driver's abilities
 * ([0x7a3a58]): the five pointers at 0x5f0780.. and 0x5f0788. */
void choosePolygonDrawers(win32::WinApplication* app)
{
    const Memory m{app};
    m.word(0x5f0788, 0x433d40);
    const x86::reg8 abilities = m.byte(0x7a3a58);
    if (abilities & 0x40)
    {
        m.word(0x5f0784, 0x4336e0);
        m.word(0x5f0790, 0x4339d0);
        m.word(0x5f0794, 0x434850);
        m.word(0x5f078c, 0x4347b0);
        m.word(0x5f0780, 0x433840);
    }
    else if (abilities & 6)
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
    }
}

/* sub_434870: choosePolygonDrawers; eax as the code leaves it: the abilities in its second
 * byte (mov ah), then the last pointer stored where that is a constant (0x4347b0, 0x433bb0). */
void polygonDrawers(win32::WinApplication* app, x86::CPU& cpu)
{
    choosePolygonDrawers(app);
    const reg8 abilities = byteAt(app, kDriverAbilities);
    if (abilities & 0x40)
        cpu.eax = 0x4347b0;
    else if (!(abilities & 6))
        cpu.eax = 0x433bb0;
    else
        cpu.eax = (cpu.eax & 0xffff00ff) | (reg32(abilities) << 8);
    cpu.esp += 4;
}


namespace
{

// The glare's darkening of the two colours, a double: its integer from the glare by fistp.
constexpr reg32 kGlareToColourScale = 0x53c698;
constexpr reg32 kGlareColours = 0x55e6dc;  // two colours

/* 0x49d800's work: the two colours to kGlareColours, darkened by the sun's glare (gameSettings'
 * alpha intensity, on every driver: a port) times [kGlareToColourScale] when the glare is below 1. */
void setGlareColours(App* app, reg32 first, reg32 second)
{
    const Memory m{app};
    const double glare = double(gameSettings(app).alphaIntensity);
    // fld1; fcomp; sahf; jbe: not below for a NaN, as 1 > NaN is false
    if (1.0 > glare)
    {
        const reg32 scale =
            reg32(fp::toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(kGlareToColourScale)))));
        first = darkened(first, scale);
        second = darkened(second, scale);
    }
    put(app, kGlareColours + 4, second);
    put(app, kGlareColours, first);
}

}

/* sub_49d800: setGlareColours for eax and edx; no caller reads what it leaves in eax. */
void glareColours(win32::WinApplication* app, x86::CPU& cpu)
{
    setGlareColours(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* sub_472d10: the detail settings, the game's highest of the five it chose by
 * the machine's speed, halved on a driver with bit 6 of
 * [0x7a3a58]: the eight words of each of the tables at 0x472168.. to
 * 0x6fbc28 (the view distance, kept full: a port), 0x6fbc2c, 0x6fbc1c,
 * 0x6fbc30, 0x6fbc20, 0x6fbc24, 0x6fbc3c and 0x791b28; the glare at 1 (0.5
 * when sub_4ee310 finds the driver's name at 0x53b01c), the screen width
 * for sub_4bee80, and sub_4483a0, sub_448330's choice of 0x6fbc34. */
void detailSettings(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x174
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 24 - 0x174;
    GameSettings& settings = gameSettings(app);
    // The eight tables of five words (movsd, movsd, movsw), in the frame as the code keeps them.
    const x86::reg32 tables[] = { 0x60, 0x30, 0x54, 0x48, 0x3c, 0xc, 0x24, 0x18 };
    for (x86::reg32 k = 0; k < 8; ++k)
    {
        if (k == 7)
            settings.unknown1868 = 2;
        for (x86::reg32 b = 0; b < 10; b += 2)
            app->getMemory<x86::reg16>(ebp - tables[k] + b) = app->getMemory<x86::reg16>(0x472168 + 10 * k + b);
    }
    // The detail for the fastest machine the game knows of: no phone is slower.
    const x86::reg32 level = (m.byte(0x7a3a58) & 6) ? 2 : 4;
    auto entryOf = [&](x86::reg32 table) {
        return x86::reg32(x86::sreg32(m.word(ebp + level * 2 - table - 2)) >> 16);
    };
    settings.viewDistance = 0;  // port (apply_view_distance): View Distance Full at any CPU speed
    settings.skyDetail = entryOf(0x30);
    settings.detailLevel = entryOf(0x48);
    const x86::reg32 flagFc = entryOf(0x18);
    settings.effectsLevel = entryOf(0x3c);
    const x86::reg32 specular = entryOf(0xc);
    settings.detailFlag104 = 1;
    settings.nearPolygons = 1;
    settings.useSpecular = specular;
    const x86::reg32 lights = entryOf(0x24);
    settings.alphaIntensityBits = 0x3f800000;
    settings.detailFlagFc = flagFc;
    settings.useLights = lights;
    const x86::reg32 abilities = m.word(0x7a3a58);
    settings.wideScreen = 0;
    settings.configVersion = abilities;
    if (abilities & 0x40)
    {
        if (!callAt(app, cpu, ebp, frameEsp, 0x4ee310, { 0x7a3a74, 0x53b01c }))
            return;
        if (!cpu.eax)
            settings.alphaIntensityBits = 0x3f000000;
    }
    x86::reg32 width = 0x280;
    if (m.byte(0x7a3a58) & 6)
    {
        width = 0x140;
        settings.detailFlagFc = 0;
        settings.projectedText = 0;
    }
    if (!callAt(app, cpu, ebp, frameEsp, 0x4bee80, { width }) || !callAt(app, cpu, ebp, frameEsp, 0x4483a0, { cpu.eax })
        || !callAtEsi(app, cpu, ebp, frameEsp, 0x448330, ebp - 0x174))
        return;
    if (m.word(ebp - 0x164))
        settings.projectedText = 1;
    else
    {
        settings.detailFlagFc = 0;
        settings.projectedText = 0;
    }
    const x86::reg32 table = entryOf(0x54);
    m.word(0x791b28, table);
    cpu.eax = table;
    cpu.esp += 4;
}

/* sub_4ddb40: the sparks of a group (eax: the group at +4) -- the
 * [0x8ca278] records of 20 bytes from 0x8c08c0 + group * 400: x, y, a
 * count, a size, an angle -- each with a count drawn as a quad record
 * (sub_4bbf80, 0xa0 bytes, flags 1, texture 0x8ca288) turned by the angle
 * (sub_4ea950, sub_4ea930), its size scaled by the count over [0x8ca27c]
 * when [0x8ca22c] is 1, its grey [0x8ca284] * count / [0x8ca27c] darkened by
 * the sun's glare times [0x5497dc] when that is below 1 (on every driver, a
 * port), depth 0.1, 1/w 0.9, with the corners' clip codes. */
void sparks(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x5c
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 24 - 0x5c;
    const x86::reg32 group = m.word(cpu.eax + 4);
    const x86::reg32 glareScale = x86::reg32(fp::toInteger<x86::sreg32>(
        x86::Float(M::mul(double(gameSettings(app).alphaIntensity), m.loadDouble(0x5497dc)))));
    auto asDouble = [](x86::reg32 bits) { return double(asFloat(bits)); };

    for (x86::reg32 i = 0; x86::sreg32(i) < x86::sreg32(m.word(0x8ca278)); ++i)
    {
        const x86::reg32 spark = i * 20 + group * 400 + 0x8c08c0;
        x86::reg32 sizeBits = m.word(spark + 0xc);
        const x86::reg32 angle = m.word(spark + 0x10);
        const x86::reg32 xBits = m.word(spark), yBits = m.word(spark + 4);
        const x86::reg32 count = m.word(spark + 8);
        if (!count)
            continue;
        const x86::reg32 grey = idiv(m.word(0x8ca284) * count, m.word(0x8ca27c)).quotient;
        if (m.word(0x8ca22c) == 1)
            sizeBits = asBits(float(M::div(M::mul(fild32(m, spark + 8), asDouble(sizeBits)), fild32(m, 0x8ca27c))));
        const std::optional<double> cosine = cosineAt(app, cpu, ebp, pushWords(app, frameEsp, { angle }));
        if (!cosine)
            return;
        const double a = M::mul(*cosine, asDouble(sizeBits));
        const std::optional<double> sine = sineAt(app, cpu, ebp, pushWords(app, frameEsp, { angle }));
        if (!sine)
            return;
        const double b = M::mul(*sine, asDouble(sizeBits));
        // The corners on the screen, each a float.
        float xs[4], ys[4];
        {
            const double x = asDouble(xBits), y = asDouble(yBits), aa = double(float(a));
            const double sx = m.load(0x56009c), sy = m.load(0x5600a0), cx = m.load(0x5600a4), cy = m.load(0x5600a8);
            const double y1 = M::mul(M::add(y, aa), sy);
            const double x1 = M::mul(M::add(x, aa), sx);
            const double y2 = M::mul(M::sub(y, aa), sy);
            const double x2 = M::mul(M::sub(x, aa), sx);
            const double bb = double(float(b));
            const double x3 = M::mul(M::sub(x, bb), sx);
            const double y3 = M::mul(M::add(y, bb), sy);
            ys[1] = float(M::add(y1, cy));
            const double x4 = M::mul(M::add(x, bb), sx);
            const double y4 = M::mul(M::sub(y, bb), sy);
            xs[0] = float(M::add(x1, cx));
            ys[3] = float(M::add(y2, cy));
            xs[2] = float(M::add(x2, cx));
            xs[1] = float(M::add(x3, cx));
            ys[0] = float(M::add(y3, cy));
            xs[3] = float(M::add(x4, cx));
            ys[2] = float(M::add(y4, cy));
        }
        if (!callAt(app, cpu, ebp, frameEsp, 0x4bbf80, { ebp - 0x50, 0xa0 }))
            return;
        if (!cpu.eax)
            break;
        const x86::reg32 record = m.word(ebp - 0x50);
        x86::reg32 colour = (grey << 8) | (grey << 16) | (grey << 24) | grey;
        // port (apply_alpha_intensity): the glare on every driver
        if (1.0 > double(gameSettings(app).alphaIntensity))
            colour = darkened(colour, glareScale);
        app->getMemory<x86::reg16>(record + 4) = 0;
        app->getMemory<x86::reg16>(record + 6) = 1;
        m.word(record + 0x18, 0x8ca288);
        m.word(record + 8, record + 0x20);
        m.word(record + 0xc, record + 0x40);
        m.word(record + 0x10, record + 0x60);
        m.word(record + 0x14, record + 0x80);
        // The corners: (x, y), their clip codes, the grey.
        for (int k = 0; k < 4; ++k)
        {
            const x86::reg32 v = record + 0x20 + 0x20 * k;
            m.word(v + 8, 0x3dcccccd);
            m.word(v + 0xc, 0x3f666666);
            m.word(v, asBits(xs[k]));
            m.word(v + 4, asBits(ys[k]));
            m.byte(v + 0x14, clipCode(m, v));
            m.word(v + 0x10, colour);
        }
    }
    cpu.esp += 4;
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
void glow(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x9c; sub ebp, 0x72
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebp = entry - 24 - 0x72;
    const x86::reg32 frameEsp = entry - 24 - 0x9c;
    const x86::reg32 view = cpu.eax;
    const x86::reg32 vertex = ebp - 0x2a;  // the light on the screen: x, y, z, 1/z, colour, code
    auto leave = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.esp = entry + 4 + 0x10;
    };
    // rep movsd of a 0x20-byte vertex
    auto copyVertex = [&](x86::reg32 to, x86::reg32 from) {
        for (x86::reg32 k = 0; k < 0x20; k += 4)
            m.word(to + k, m.word(from + k));
    };
    // What the code's clip code leaves in eax: 1/z's bits when it is behind, else x's.
    auto codeEax = [&](x86::reg32 v) {
        const x86::reg32 w = m.word(v + 0xc);
        return (w & 0x80000000) || x86::sreg32(w) >= x86::sreg32(m.word(0x7d34f8)) ? w : m.word(v);
    };
    // One of the two quads at [ebp + slot]: the vertex at ebp - 0x2a copied to its corners (`first`
    // and those at [corner1..3]), turned by `angle` with half sizes `across`, `down` (kept at
    // ebp + acrossAt, + downAt; the angle at + angleAt, the sine's share at + sineAt), clip-coded.
    struct Quad
    {
        x86::reg32 slot, corner1, corner2, corner3, angleAt, acrossAt, downAt, sineAt;
    };
    auto quad = [&](const Quad& q, x86::reg32 first, double across, double down, double angle) {
        m.store(q.angleAt, angle);
        copyVertex(m.word(q.corner3), vertex);
        const double turn = M::mul(M::add(1.0, m.load(q.angleAt)), m.load(0x53bcb8));
        copyVertex(m.word(q.corner2), m.word(q.corner3));
        m.store(q.acrossAt, across);
        m.store(q.downAt, down);
        copyVertex(m.word(q.corner1), m.word(q.corner2));
        m.store(q.angleAt, turn);
        const x86::reg32 sineWords = pushWords(app, frameEsp, { m.word(q.angleAt) });
        copyVertex(first, m.word(q.corner1));
        const std::optional<double> sineTaken = sineAt(app, cpu, ebp, sineWords);
        if (!sineTaken)
            return false;
        const double sine = M::mul(*sineTaken, m.load(q.acrossAt));
        const x86::reg32 cosineWords = pushWords(app, frameEsp, { m.word(q.angleAt) });
        m.store(q.sineAt, sine);
        const std::optional<double> cosineTaken = cosineAt(app, cpu, ebp, cosineWords);
        if (!cosineTaken)
            return false;
        const double cosine = M::mul(*cosineTaken, m.load(q.downAt));
        const double s = m.load(q.sineAt);
        {
            const double x = M::sub(m.load(first), s);
            const double y = M::add(m.load(first + 4), cosine);
            const x86::reg32 v = m.word(q.corner1);
            m.store(first, x);
            m.store(first + 4, y);
            const double x1 = M::add(m.load(v), cosine);
            const double y1 = M::add(m.load(v + 4), s);
            m.store(v, x1);
            m.store(v + 4, y1);
        }
        {
            const x86::reg32 v = m.word(q.corner2);
            const double x = M::add(m.load(v), s);
            const double y = M::sub(m.load(v + 4), cosine);
            m.store(v, x);
            m.store(v + 4, y);
        }
        {
            const x86::reg32 v = m.word(q.corner3);
            const double x = M::sub(m.load(v), cosine);
            const double y = M::sub(m.load(v + 4), s);
            m.store(v, x);
            m.store(v + 4, y);
        }
        m.byte(first + 0x14, clipCode(m, first));
        for (x86::reg32 corner : { q.corner1, q.corner2, q.corner3 })
        {
            const x86::reg32 v = m.word(corner);
            m.byte(v + 0x14, clipCode(m, v));
        }
        return true;
    };

    const x86::reg32 kind = m.word(entry + 0x10) & 0xf;
    m.word(ebp + 0x42, 0);
    m.word(ebp + 0x3a, 0);
    if (gameSettings(app).effectsLevel == 2)
        return leave(view);
    const x86::reg32 blink = kind * 12 + 0x55e3e0;
    if (app->getMemory<x86::reg16>(blink) != 0)
    {
        const x86::reg32 phase = m.byte(blink + 3), shift = m.byte(blink + 2);
        const x86::reg32 on = x86::reg32(x86::sreg32(m.word(0x7d3684) + phase) >> (shift & 31));
        if (on & 1)
            return leave(on);
    }
    m.word(ebp + 0x66, m.word(kind * 12 + 0x55e3dc));
    m.word(ebp + 0x62, m.word(kind * 12 + 0x55e3e4));
    // port (apply_alpha_intensity): the glare on every driver
    {
        const double glare = double(gameSettings(app).alphaIntensity);
        if (1.0 > glare)
        {
            const x86::reg32 scale =
                x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(0x53bc98)))));
            m.word(ebp + 0x66, darkened(m.word(ebp + 0x66), scale));
        }
    }
    {
        // Into the view: times [0x53bca0], turned by +0x44, moved by +0x38.
        const double c = m.loadDouble(0x53bca0);
        const double px = M::mul(fild32(m, entry + 4), c);
        const double py = M::mul(fild32(m, entry + 8), c);
        const double pz = M::mul(c, fild32(m, entry + 0xc));
        m.store(ebp - 0xa, px);
        m.store(ebp - 6, py);
        m.store(ebp - 2, pz);
        const x86::reg32 v = ebp - 0xa, mx = view + 0x44;
        double t[3];
        for (int i = 0; i < 3; ++i)
            t[i] = M::add(M::add(M::mul(m.load(v), m.load(mx + 4 * i)), M::mul(m.load(v + 4), m.load(mx + 0xc + 4 * i))),
                          M::mul(m.load(v + 8), m.load(mx + 0x18 + 4 * i)));
        for (int i = 0; i < 3; ++i)
            m.store(ebp + 2 + 4 * i, t[i]);
        for (int i = 0; i < 3; ++i)
            m.store(ebp + 2 + 4 * i, M::add(m.load(ebp + 2 + 4 * i), m.load(view + 0x38 + 4 * i)));
    }
    const x86::reg32 z = m.word(ebp + 0xa);
    if (x86::sreg32(z) > 0x43800000 || x86::sreg32(z) < 0x3f800000)
        return leave(z);
    if (!(z & 0x7fffffff))
        m.word(ebp + 0xa, 0x37800080);
    {
        // The vertex: x, y, z, 1/z.
        m.store(ebp + 0x1e, M::div(1.0, m.load(ebp + 0xa)));
        const x86::reg32 inverse = ebp + 0x1e, p = ebp + 2;
        const double sx = M::mul(m.load(0x56009c), m.load(inverse));
        const double sy = M::mul(M::mul(m.load(0x5600a0), m.load(inverse)), m.load(p + 4));
        m.word(vertex + 0xc, m.word(inverse));
        const double x = M::mul(sx, m.load(p));
        const x86::reg32 zBits = m.word(p + 8);
        m.store(vertex, M::add(x, m.load(0x5600a4)));
        m.word(vertex + 8, zBits);
        m.store(vertex + 4, M::add(sy, m.load(0x5600a8)));
    }
    m.byte(vertex + 0x14, clipCode(m, vertex));
    m.store(vertex + 0xc, M::mul(m.load(vertex + 0xc), m.loadDouble(0x53bca8)));
    m.byte(vertex + 0x14, clipCode(m, vertex));
    if (m.byte(vertex + 0x14))
        return leave(codeEax(vertex));
    {
        // The inner glow: the colour darkened by the depth too.
        const x86::reg32 depth = x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(m.load(ebp + 0xa))));
        m.word(vertex + 0x10, darkened(m.word(ebp + 0x66), (0x100 - depth) << 8));
    }
    if (!callAt(app, cpu, ebp, frameEsp, 0x4bbde0, { ebp + 0x3a, 0xa0 }))
        return;
    if (!cpu.eax)
        return leave(0);
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
        m.word(record + 0x18, 0x8b47bc);
        const double size = m.load(ebp + 0x62);
        const double across = M::mul(se, size);
        m.word(record + 8, record + 0x20);
        const double down = M::mul(size, et);
        m.word(record + 0xc, m.word(ebp + 0x3e));
        m.word(record + 0x10, m.word(ebp + 0x2a));
        m.word(record + 0x14, m.word(ebp + 0x6a));
        m.word(record + 0x1c, m.word(ebp - 0x22));
        if (!quad({ ebp + 0x3a, ebp + 0x3e, ebp + 0x2a, ebp + 0x6a, ebp + 0x36, ebp + 0x22, ebp + 0x26, ebp + 0x2e },
                  record + 0x20, across, down, angle))
            return;
    }
    // The outer glow, linked to the inner one.
    m.word(vertex + 0x10, m.word(ebp + 0x66));
    if (!callAt(app, cpu, ebp, frameEsp, 0x4bbde0, { ebp + 0x42, 0xa0 }))
        return;
    if (!cpu.eax)
        return leave(0);
    {
        const x86::reg32 record = m.word(ebp + 0x42);
        const double sx = M::mul(m.load(0x56009c), m.load(vertex + 0xc));
        const double sy = M::mul(m.load(0x5600a0), m.load(vertex + 0xc));
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
        const double down = M::mul(f, sh);
        m.word(record + 8, record + 0x20);
        m.word(record + 0xc, m.word(ebp + 0x4e));
        m.word(record + 0x10, m.word(ebp + 0x52));
        m.word(record + 0x14, m.word(ebp + 0x6e));
        m.word(record + 0x1c, m.word(ebp - 0x22));
        if (!quad({ ebp + 0x42, ebp + 0x4e, ebp + 0x52, ebp + 0x6e, ebp + 0x5e, ebp + 0x46, ebp + 0x4a, ebp + 0x56 },
                  record + 0x20, across, down, angle))
            return;
    }
    if (!callAt(app, cpu, ebp, pushWords(app, frameEsp, { m.word(0x7dcffc) }), 0x4c7610, { m.word(ebp + 0x42) }))
        return;
    leave(cpu.eax);
}


namespace
{

// The glare's darkening of vertices' colours, a double: its integer from the glare by fistp.
constexpr reg32 kGlareToVertexScale = 0x53b418;
constexpr reg32 kVertexBytes = 0x20, kVertexColour = 0x10;

/* 0x47b7d0's work: the colours of `count` vertices from `vertices` darkened by the sun's glare
 * (gameSettings' alpha intensity) times [kGlareToVertexScale]. */
void glareVertexColours(App* app, reg32 count, reg32 vertices)
{
    const Memory m{app};
    const reg32 scale = reg32(fp::toInteger<x86::sreg32>(
        x86::Float(M::mul(double(gameSettings(app).alphaIntensity), m.loadDouble(kGlareToVertexScale)))));
    for (reg32 left = count; left != 0; --left, vertices += kVertexBytes)
        put(app, vertices + kVertexColour, darkened(at(app, vertices + kVertexColour), scale));
}

// The two buffers cleared for a race's drawing, and the words after them.
constexpr reg32 kZeroMemory = 0x4e070c;  // ZeroMemoryRegisterCall: edx bytes from eax
constexpr reg32 kDepthBuckets = 0x7ddfd0, kDepthBucketsBytes = 0x12c00;
constexpr reg32 kPolygonBuckets = 0x7f0bd0, kPolygonBucketsBytes = 0x6400;
constexpr reg32 kBucketsUsed = 0x7f6fd0;  // two words
// Eight bytes of colours, set for a driver drawing with alpha.
constexpr reg32 kAlphaColours = 0x560f30;

/* kZeroMemory on `bytes` from `address` in a frame at `frame`, esp there.  False when the game
 * stopped. */
bool zeroMemory(App* app, CPU& cpu, reg32 frame, reg32 address, reg32 bytes)
{
    return callAt(app, cpu, frame, frame, kZeroMemory, { address, bytes });
}

/* 0x4cb670's work, from a frame three words below the return address.  False when the game
 * stopped. */
bool clearDrawingBuffers(App* app, CPU& cpu)
{
    const reg32 frame = cpu.esp - 12;  // push ecx, edx, ebp; mov ebp, esp
    if (!zeroMemory(app, cpu, frame, kDepthBuckets, kDepthBucketsBytes)
        || !zeroMemory(app, cpu, frame, kPolygonBuckets, kPolygonBucketsBytes))
        return false;
    if (byteAt(app, kDriverAbilities) & kAbilityAlpha)
    {
        for (reg32 k : {2u, 1u, 0u, 7u, 4u, 3u})
            putByte(app, kAlphaColours + k, 0x7f);
        putByte(app, kAlphaColours + 6, 0);
        putByte(app, kAlphaColours + 5, 0x3f);
    }
    put(app, kBucketsUsed + 4, 0);
    put(app, kBucketsUsed, 0);
    return true;
}

}

/* sub_47b7d0: glareVertexColours for eax vertices from edx; no caller reads its eax. */
void glareVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    glareVertexColours(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* sub_4cb670: clearDrawingBuffers; no caller reads its eax. */
void clearBuffers(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!clearDrawingBuffers(app, cpu))
        return;
    cpu.esp += 4;
}

/* sub_4a4410: the ten bouncing pictures of a 2D screen (not on the drivers
 * the strings at 0x53cf28 and 0x53cf34 name, nor with bit 2 of
 * [0x7a3a58]): placed at random (sub_4a4060) the first time, each moved by
 * its speed (0x7a1ae8, 0x7a1b10) and put back (sub_4a40c0) when it leaves
 * 150..640 x 0..450, drawn by sub_4d7d90 with eleven fading copies trailing
 * behind it (the last picture as a shadow, its y from 225), centred by its
 * scaled width. */
void bouncers(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18
    const x86::reg32 ebp = cpu.esp - 24;
    const x86::reg32 frameEsp = cpu.esp - 24 - 0x18;
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp = 0) {
        return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers);
    };
    // sub_4dfd56 on `value`, then fistp
    std::optional<x86::reg32> truncated;
    auto truncate = [&](double value) {
        const std::optional<double> whole = chopAt(app, cpu, ebp, frameEsp, value);
        if (!whole)
            return false;
        truncated = x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(*whole)));
        return true;
    };
    auto leave = [&]() { cpu.esp += 4; };
    auto half = [](x86::reg32 value) { return x86::reg32(x86::sreg32(value - x86::reg32(x86::sreg32(value) >> 31)) >> 1); };
    auto fild = [](x86::reg32 value) { return double(x86::sreg32(value)); };

    if (m.byte(0x7a3a58) & 0x40)
        for (x86::reg32 name : { 0x53cf28u, 0x53cf34u })
        {
            if (!callTo(0x4e0820, { 0x7a3a74, name }))
                return;
            if (cpu.eax)
                return leave();
        }
    if (m.byte(0x7a3a58) & 2)
        return leave();
    if (m.word(0x55e95c) == 0xffffffff)
        for (x86::reg32 i = 0; i < 10; ++i)
        {
            if (!callTo(0x4a40c0, { i }))
                return;
            if (i < 9)
            {
                if (!callTo(0x4a4060, { 0x96, 0x280 }))
                    return;
                m.word(i * 4 + 0x55e95c, cpu.eax);
                if (!callTo(0x4a4060, { 0, 0x1c2 }))
                    return;
                m.word(i * 4 + 0x7a1ac0, cpu.eax);
            }
        }
    for (x86::reg32 i = 0; i < 10; ++i)
    {
        const x86::reg32 at = i * 4;
        {
            const x86::reg32 x = m.word(at + 0x55e95c) + m.word(at + 0x7a1ae8);
            const x86::reg32 y = m.word(at + 0x7a1ac0) + m.word(at + 0x7a1b10);
            m.word(at + 0x55e95c, x);
            m.word(at + 0x7a1ac0, y);
            if ((x86::sreg32(x) > 0x280 || x86::sreg32(x) < 0x96 || x86::sreg32(y) > 0x1c2 || x86::sreg32(y) < 0)
                && !callTo(0x4a40c0, { i }))
                return;
        }
        x86::reg32 x = m.word(at + 0x55e95c);
        x86::reg32 y = m.word(at + 0x7a1ac0);
        for (x86::reg32 copy = 0; copy < 12; ++copy)
        {
            // Fading with the copy's age; the first copy as it is, the shadow's opaque.
            x86::reg32 alpha = i == 9 ? 0xff : 0x96;
            if (copy)
                alpha = half(x86::reg32(x86::sreg32((12 - copy) * 160 - (12 - copy) * 10) / 12));
            else
            {
                // Centred by half its scaled width.
                const x86::reg32 w = m.word((m.word(at + 0x7a1b38) << 4) + 0x55e8e0);
                const double width = M::mul(fild(half(w)), m.load(at + 0x7a1b60));
                if (!truncate(M::sub(fild(x), width)))
                    return;
                x = *truncated;
            }
            const x86::reg32 colour = (alpha << 24) + 0xffffff;
            const x86::reg32 picture = m.word(0x7a1b94);
            if (x86::sreg32(x) < 0x280 && x86::sreg32(x) > 0x96 && x86::sreg32(y) < 0x1c2 && x86::sreg32(y) > 0)
            {
                const x86::reg32 kind = m.word(at + 0x7a1b38) << 4;
                const x86::reg32 esp =
                    pushWords(app, frameEsp, { colour, m.word(at + 0x7a1b60), m.word(kind + 0x55e8e4),
                                               m.word(kind + 0x55e8e0), m.word(kind + 0x55e8dc) });
                if (!callTo(0x4d7d90, { picture, x, y, 0 }, esp))
                    return;
            }
            if (i == 9)
                y -= half(0xe1 - m.word(at + 0x7a1ac0));
            else
            {
                // The next copy back along the speed, the shadow's y kept.
                const double backX = M::mul(fild(m.word(at + 0x7a1ae8) * 10), m.load(at + 0x7a1b60));
                const double backY = M::mul(fild(m.word(at + 0x7a1b10) * 10), m.load(at + 0x7a1b60));
                const x86::reg32 lift = half(0xe1 - m.word(at + 0x7a1ac0));
                const double nextX = M::sub(fild(x), backX);
                const double nextY = M::sub(fild(y), backY);
                if (!truncate(nextY))
                    return;
                const x86::reg32 newY = *truncated;
                if (!truncate(nextX))
                    return;
                x = *truncated;
                y = newY - lift;
            }
        }
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
void roadLight(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x204; sub ebp, 0x76
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 ebp = entry - 24 - 0x76;
    const x86::reg32 frameEsp = entry - 24 - 0x204;
    const x86::reg32 view = cpu.eax;
    // The arguments, which the code also writes: the point, the colour, the size.
    const x86::reg32 point = entry + 4, colour = entry + 0x10, size = entry + 0x18;
    const x86::reg32 refusal = m.word(entry + 0x14), car = m.word(entry + 0x1c), kept = entry + 0x20;
    const x86::reg32 track = ebp - 0x10e;  // the track position sub_420650 and the rest work on
    auto callTo = [&](x86::reg32 target, std::initializer_list<x86::reg32> registers, x86::reg32 esp = 0) {
        return callAt(app, cpu, ebp, esp ? esp : frameEsp, target, registers);
    };
    auto integer = [](double value) { return x86::reg32(fp::toInteger<x86::sreg32>(x86::Float(value))); };
    auto leave = [&](x86::reg32 eax) {
        cpu.eax = eax;
        cpu.esp = entry + 4 + 0x20;
    };
    // movsd three times
    auto copy3 = [&](x86::reg32 to, x86::reg32 from) {
        for (x86::reg32 k = 0; k < 0xc; k += 4)
            m.word(to + k, m.word(from + k));
    };

    copy3(ebp + 6, 0x491110);
    m.word(ebp + 0x66, 0);
    if (gameSettings(app).effectsLevel == 2 || gameSettings(app).weather == 0)
        return leave(0);
    if (!callTo(0x420bf0, {},
                pushWords(app, frameEsp, { refusal, m.word(point + 8), m.word(point + 4), m.word(point) })))
        return;
    if (cpu.eax || m.word(view) == 1)
        return leave(0);
    {
        // To the track's fixed point and back (sub_420650).
        const double half = M::mul(m.load(size), m.loadDouble(0x53bcc8));
        const double toFixed = m.loadDouble(0x53bcd0);
        const x86::reg32 x = integer(M::mul(m.load(point), toFixed));
        m.store(size, half);
        m.word(ebp + 0x2a, x);
        m.word(ebp + 0x2e, integer(M::mul(m.load(point + 4), toFixed)));
        m.word(ebp + 0x32, integer(M::mul(m.load(point + 8), toFixed)));
    }
    if (!callAtEsi(app, cpu, ebp, frameEsp, 0x420650, track, { ebp + 0x2a, 1, refusal }))
        return;
    {
        const double fromFixed = m.loadDouble(0x53bcd8);
        const double x = M::mul(fild32(m, ebp + 0x2a), fromFixed);
        const double y = M::mul(fild32(m, ebp + 0x2e), fromFixed);
        const double z = M::mul(fromFixed, fild32(m, ebp + 0x32));
        m.store(point, x);
        m.store(point + 4, y);
        const x86::reg32 found = m.word(ebp - 0xf6);
        m.store(point + 8, z);
        if (!found)
            return leave(0);
    }
    if (!callTo(0x4207e0, { track, 1 }))
        return;
    const x86::reg32 place = cpu.eax;
    if (!callTo(0x4dcdd0, {}))
        return;
    if (m.word(((cpu.eax * 9) << 6) + (place & 0x3f) * 9 * 4 + 0x55308c) != 0xa)
        return leave(0);
    {
        // From the view (+8..+0x10), level.
        const double dx = M::sub(m.load(point), m.load(view + 8));
        const double dy = M::sub(m.load(point + 4), m.load(view + 0xc));
        const double dz = M::sub(m.load(point + 8), m.load(view + 0x10));
        m.store(ebp + 0x1e, dx);
        m.store(ebp + 0x22, dy);
        m.store(ebp + 0x26, dz);
        m.word(ebp + 0x22, 0);
    }
    const std::optional<double> length1 = normalisedAt(app, cpu, ebp, frameEsp, ebp + 0x1e);
    if (!length1)
        return;
    m.store(ebp + 0x6e, *length1);
    if (!(m.word(ebp + 0x6e) & 0x7fffffff))
        return leave(0);
    if (car != 0)
    {
        // Shortened by the car's facing (+0x18), reversed unless kept.
        copy3(ebp - 6, car + 0x18);
        if (!m.word(kept))
            for (const x86::reg32 top : { ebp + 1, ebp - 3, ebp + 5 })
                m.byte(top, m.byte(top) ^ 0x80);
        const std::optional<double> dot = dotAt(app, cpu, ebp, frameEsp, ebp - 6, ebp + 0x1e);
        if (!dot)
            return;
        m.store(ebp + 0x6e, M::mul(-*dot, m.load(ebp + 0x6e)));
    }
    if (!callTo(0x4e0160, { ebp + 0x1e, ebp + 6, ebp - 0x12 })
        || !callTo(0x4e0050, { 1, ebp - 0x12 }, pushWords(app, frameEsp, { ebp - 0x12, m.word(size) })))
        return;
    {
        // The length held to [size, 30].
        const double length = M::mul(m.load(ebp + 0x6e), m.loadDouble(0x53bce0));
        const double least = m.load(size);
        const double along = !(length >= least) ? least : length > m.loadDouble(0x53bce8) ? 30.0 : length;
        const x86::reg32 esp = pushWords(app, frameEsp, { ebp + 0x1e, asBits(float(-along)) });
        if (!callTo(0x4e0050, { 1, ebp + 0x1e }, esp))
            return;
    }
    x86::reg32 heights;
    {
        // The four corners: point - across, + across, + across + along, - across + along.
        const x86::reg32 across = ebp - 0x12, along = ebp + 0x1e;
        const x86::reg32 corners[] = { ebp - 0x96, ebp - 0x8a, ebp - 0x7e, ebp - 0x72 };
        for (int c = 0; c < 4; ++c)
        {
            const x86::reg32 p = corners[c];
            copy3(p, point);
            for (x86::reg32 k = 0; k < 0xc; k += 4)
            {
                const double side = c == 0 || c == 3 ? M::sub(m.load(p + k), m.load(across + k))
                                                     : M::add(m.load(p + k), m.load(across + k));
                m.store(p + k, side);
            }
            if (c == 3)
                heights = m.word(ebp - 0xfa);
            if (c >= 2)
                for (x86::reg32 k = 0; k < 0xc; k += 4)
                    m.store(p + k, M::add(m.load(p + k), m.load(along + k)));
        }
    }
    if (heights && m.word(ebp - 0xa2) == 0)
    {
        // The road's heights at the corners.
        if (!callAtEsi(app, cpu, ebp, frameEsp, 0x420230, ebp - 0x1e, { track, ebp - 0x66 }))
            return;
        const double fromFixed = m.loadDouble(0x53bcd8);
        const x86::reg32 sources[] = { 0x66, 0x62, 0x5e, 0x5a, 0x56, 0x52, 0x4e, 0x4a, 0x46, 0xea, 0xe6, 0xe2 };
        double v[12];
        for (int i = 0; i < 12; ++i)
            v[i] = M::mul(fild32(m, ebp - sources[i]), fromFixed);
        for (int i = 0; i < 9; ++i)
            m.store(ebp - 0x42 + 4 * i, v[i]);
        for (int i = 0; i < 3; ++i)
            m.store(ebp + 0x12 + 4 * i, v[9 + i]);
        for (x86::reg32 k = 0; k < 4; ++k)
        {
            const x86::reg32 at = 12 * k;
            const x86::reg32 esp =
                pushWords(app, frameEsp, { m.word(at + ebp - 0x8e), m.word(at + ebp - 0x92), m.word(at + ebp - 0x96),
                                           m.word(ebp + 0x1a), m.word(ebp + 0x16), m.word(ebp + 0x12) });
            const std::optional<double> ground = groundHeightAt(app, cpu, ebp, esp, ebp - 0x42);
            if (!ground)
                return;
            m.store(at + ebp - 0x92, M::add(*ground, m.load(0x53bcf0)));
        }
    }
    if (!callTo(0x4bf4c0, { 4, ebp - 0x96, view + 0x44, view + 0x38 }, pushWords(app, frameEsp, { ebp - 0x18e })))
        return;
    {
        const x86::reg32 c0 = m.byte(ebp - 0x17a), c1 = m.byte(ebp - 0x15a), c2 = m.byte(ebp - 0x13a);
        const x86::reg32 c3 = m.byte(ebp - 0x11a);
        if (c0 & c1 & c2 & c3)
            return leave(0);
        if (!((c0 | c1 | c2 | c3) & 0x10))
        {
            // Facing: (x2 - x1)(y0 - y1) against (y2 - y1)(x0 - x1), twice.
            const x86::reg32 v0 = ebp - 0x18e, v1 = ebp - 0x16e, v2 = ebp - 0x14e, v3 = ebp - 0x12e;
            auto facing = [&](x86::reg32 a, x86::reg32 b, x86::reg32 c) {
                const double p = M::mul(M::sub(m.load(c), m.load(b)), M::sub(m.load(a + 4), m.load(b + 4)));
                const double q = M::mul(M::sub(m.load(c + 4), m.load(b + 4)), M::sub(m.load(a), m.load(b)));
                return ((q > p ? 1u : 0u) ^ m.word(0x554e4c)) != 0;
            };
            if (!facing(v0, v1, v2) || !facing(v0, v2, v3))
                return leave(0);
        }
    }
    // port (apply_alpha_intensity): the glare on every driver
    {
        const double glare = double(gameSettings(app).alphaIntensity);
        if (1.0 > glare)
            m.word(colour, darkened(m.word(colour), integer(M::mul(glare, m.loadDouble(0x53bcd0)))));
    }
    for (const x86::reg32 at : { 0x11eu, 0x13eu, 0x15eu, 0x17eu })
        m.word(ebp - at, m.word(colour));
    if (!callTo(0x4bbde0, { ebp + 0x66, 0xa0 }))
        return;
    if (!cpu.eax)
        return leave(0);
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
    leave(record);
}


// step 3, second part: the views, the effects, the HUD and the textures with their work in C++
// locals; the registers only at their entries and exits and at the calls out.
namespace
{

/* The calls out of a function whose frame is on the guest stack: esp as deep as the original's,
 * the words a callee takes pushed below it, each callee handed its register arguments (eax, edx,
 * ebx, ecx in that order) and nothing else set -- the other registers as the caller has them,
 * which the callees keep.  call() moves esp on as the callee pops; false when the game stopped,
 * the callee's eax in result(). */
class Calls
{
public:
    Calls(App* app, CPU& cpu, reg32 esp) : app(app), cpu(cpu), esp(esp) {}
    void push(reg32 value)
    {
        esp -= 4;
        put(app, esp, value);
    }
    bool call(reg32 target, std::initializer_list<reg32> arguments = {})
    {
        reg32* const registers[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
        unsigned k = 0;
        for (const reg32 a : arguments)
            *registers[k++] = a;
        cpu.esp = esp;
        if (!nfs3hp::call(app, cpu, target))
            return false;
        esp = cpu.esp;
        return true;
    }
    reg32 result() const { return cpu.eax; }
    /* One of THRASH's exports through the game's pointer to it (guest::driver, thrash_calls.h: the
     * port's driver as C++ where the pointer leads to it, through the pointer in the checks), its
     * words already pushed: taken off as its ret takes them. */
    template <reg32 Slot, auto Fn>
    bool driver()
    {
        return driverWith<Slot, Fn>(std::make_index_sequence<thrash::exportAt(Slot).bytes / 4>{});
    }

    App* const app;
    CPU& cpu;
    reg32 esp;

private:
    template <reg32 Slot, auto Fn, std::size_t... I>
    bool driverWith(std::index_sequence<I...>)
    {
        const reg32 words[] = {at(app, esp + 4 * reg32(I))..., 0};
        cpu.esp = esp + 4 * reg32(sizeof...(I));
        if (!guest::driver<Slot, Fn>(app, cpu, words[I]...))
            return false;
        esp = cpu.esp;
        return true;
    }
};

/* fcom a with b as the jump after its sahf reads it: CF a below b, ZF equal, both when unordered.
 * fcomAbove ja's (neither), fcomBelow jb's (CF), fcomBelowOrEqual jbe's (CF or ZF). */
inline bool fcomAbove(double a, double b) { return a > b; }
inline bool fcomBelow(double a, double b) { return !(a >= b); }
inline bool fcomBelowOrEqual(double a, double b) { return !(a > b); }

// A float's bits as fld dword reads them.
inline double loaded(reg32 bits) { return double(asFloat(bits)); }

/* A screen vertex's clip code from the bits of its 1/z (+0xc), y (+4) and x (+0) against the
 * view's limits, as screenCode makes it: 0x10 behind, else 8 or 4 above or below, 1 or 2 left or
 * right. */
reg8 screenCodeOf(App* app, reg32 v)
{
    const i32 inverse = i32(at(app, v + 0xc));
    if (inverse < 0 || inverse >= i32(at(app, 0x7d34f8)))
        return 0x10;
    reg8 code = 0;
    const i32 y = i32(at(app, v + 4)), x = i32(at(app, v));
    if (y < 0 || y < i32(at(app, 0x7d3504)))
        code |= 8;
    else if (y > i32(at(app, 0x7d3500)))
        code |= 4;
    if (x < 0 || x < i32(at(app, 0x7d34fc)))
        code |= 1;
    else if (x > i32(at(app, 0x7d350c)))
        code |= 2;
    return code;
}

// sub_4eb230: ea::randomFractionOf, a fraction on st(0) taken off; nullopt when the game stopped.
std::optional<double> randomFraction(Calls& out)
{
#ifdef NFS_GAME_CHECKS
    if (!out.call(0x4eb230))
        return std::nullopt;
    return double(out.cpu.fpu.take());
#else
    out.cpu.esp = out.esp;
    const std::optional<x86::Float> fraction = guest::direct<0x4eb230, ea::randomFractionOf>(out.app, out.cpu);
    if (!fraction || out.cpu.terminate)
        return std::nullopt;
    return double(*fraction);
#endif
}

/* The effects' particles: their positions (12 bytes each, 9600 bytes a group), their states (+8
 * dead, +9 a streak's last place kept) and the screen points sub_4bf4c0 makes of them (0x20 bytes
 * each). */
constexpr reg32 kParticlePositions = 0x8c56e8, kParticleStates = 0x8c0be8, kParticlePoints = 0x8ba4c0;
constexpr reg32 kParticleGroupBytes = 9600;

/* A particle at `position` put back at `view`: its height the view's (+0xc) plus [0x8ca264], x
 * and z the view's (+8, +0x10) scattered by sub_4eb230's fraction times [0x8ca25c] and [scale],
 * less [0x8ca25c].  False when the game stopped. */
bool respawnParticle(Calls& out, reg32 view, reg32 position, reg32 scale)
{
    const Memory m{out.app};
    m.store(position + 4, M::add(m.load(view + 0xc), m.load(0x8ca264)));
    for (const reg32 field : {8u, 0x10u})
    {
        const std::optional<double> fraction = randomFraction(out);
        if (!fraction)
            return false;
        const double spread = m.load(0x8ca25c);
        const double scattered = M::sub(M::mul(M::mul(*fraction, spread), m.load(scale)), spread);
        m.store(position + (field == 8 ? 0 : 8), M::add(scattered, m.load(view + field)));
    }
    return true;
}

/* A Windows function (`Function`, the port's own: native_windows.h) through its import's slot
 * `Slot`, `words` handed: as guest::windows, the words pushed in the check, where the stand-in
 * leaves them as the game's code then has them.  The check cannot tell the function its slot
 * holds, so takes all four of eax, edx, ebx, ecx at the call: there they are `held`, as the code
 * has them.  Its eax; nullopt when the game stopped. */
template <reg32 Slot, auto Function, typename... Words>
std::optional<reg32> windowsCall(Calls& out, std::initializer_list<reg32> held, Words... words)
{
#ifdef NFS_GAME_CHECKS
    const reg32 pushed[] = {reg32(words)..., 0};
    for (std::size_t i = sizeof...(Words); i-- > 0;)
        out.push(pushed[i]);
    if (!out.call(at(out.app, Slot), held))
        return std::nullopt;
    return out.result();
#else
    (void)held;
    out.cpu.esp = out.esp;
    return guest::windows<Slot, Function>(out.app, out.cpu, words...);
#endif
}

// `count` words from `from` to `to`, forwards, as rep movsd copies them.
void copyWords(App* app, reg32 to, reg32 from, reg32 count)
{
    for (reg32 k = 0; k < count; ++k)
        put(app, to + 4 * k, at(app, from + 4 * k));
}

constexpr reg32 kChop = 0x4dfd56;  // __CHP: st(0) to a whole value toward 0

/* kChop on `value`: watcom::clib::chop, which leaves all else as it was.  In the checks
 * (NFS_GAME_CHECKS) by address, `value` pushed alone -- the code has its other values under it on
 * the x87 stack, which __CHP leaves alone.  nullopt when the game stopped. */
std::optional<x86::Float> chopped(Calls& out, x86::Float value)
{
#ifdef NFS_GAME_CHECKS
    out.cpu.fpu.push(value);
    if (!out.call(kChop))
        return std::nullopt;
    return out.cpu.fpu.take();
#else
    out.cpu.esp = out.esp;
    return x86::Float(guest::direct<kChop, watcom::clib::chop>(out.app, out.cpu, double(value)));
#endif
}

/* sub_4d1390, `scale` pushed: port_text's textScaleSet, which leaves all else (eax too) as it was;
 * the scale before it, which the code drops (fstp st(0)).  False when the game stopped. */
bool setTextScale(Calls& out, reg32 scale)
{
    out.push(scale);
#ifdef NFS_GAME_CHECKS
    if (!out.call(0x4d1390))
        return false;
    out.cpu.fpu.take();
    return true;
#else
    out.cpu.esp = out.esp;
    try
    {
        guest::direct<0x4d1390, port::text::textScaleSet>(out.app, out.cpu, scale);
    }
    catch (const fe::Terminated&)
    {
        return false;
    }
    if (out.cpu.terminate)
        return false;
    out.esp += 4;
    return true;
#endif
}

// fcom a with b as je reads it after sahf: ZF, equal or unordered.
inline bool fcomEqual(double a, double b) { return !(a < b || a > b); }

// fstp dword then fld dword: a value rounded to a float, as a frame's float leaves it.
inline double single(double value) { return double(float(value)); }

// fild of an integer then fstp dword: its bits as a float.
inline reg32 floatOf(reg32 integer) { return asBits(float(double(i32(integer)))); }

// The effects' grey: [0x8ca238] in each channel.
reg32 effectGrey(App* app)
{
    const reg32 grey = at(app, 0x8ca238);
    return grey | (grey << 8) | (grey << 16) | (grey << 24);
}

/* `colour` darkened by the sun's glare below 1 (Alpha Intensity, on every driver: a port,
 * apply_alpha_intensity), the glare times the double at `factor` as fistp rounds it. */
reg32 glareDarkened(App* app, reg32 colour, reg32 factor)
{
    const Memory m{app};
    const double glare = double(gameSettings(app).alphaIntensity);
    if (!fcomAbove(1.0, glare))
        return colour;
    return darkened(colour, reg32(fp::toInteger<x86::sreg32>(x86::Float(M::mul(glare, m.loadDouble(factor))))));
}

}

/* sub_41df10: the next view record (0x54 bytes from 0x5dd0e0, [0x5dd0dc]
 * of them so far) for view kind eax, its screen edx and its owner ebx: the
 * cull distances and fog by the kind -- the main view, a split screen (its
 * cull at 500 as a single view's, a port), the mirror -- each also times
 * [0x536d6c] as floats; the count moved on (the record's number, left in
 * eax, no caller reads). */
void viewRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const reg32 kind = cpu.eax, screen = cpu.edx, owner = cpu.ebx;
    const reg32 record = 0x5dd0e0 + m.word(0x5dd0dc) * 0x54;
    m.word(record + 0x14, 0xffffffff);
    m.word(record + 0x18, 0xffffffff);
    m.word(record + 0x48, 0);
    m.word(record + 8, 1);
    m.word(record + 4, 0);
    m.word(record, screen);
    m.word(record + 0x1c, owner);
    auto plain = [&](reg32 near, reg32 far, reg32 fogNear, reg32 fogFar, reg32 cull, reg32 detail) {
        m.word(record + 0x24, near);
        m.word(record + 0x28, far);
        m.word(record + 0x38, fogNear);
        m.word(record + 0x3c, fogFar);
        m.word(record + 0x48, cull);
        m.word(record + 0x4c, detail);
    };
    // port: at Full the views cull past 500 (nfs3hp_main.cpp's kFullFarDistance)
    const reg32 cullDistance = fullFarDistance(app, 0x1f40000);
    switch (kind)
    {
    case 0:
        plain(cullDistance, cullDistance, 0x6e0000, 0x960000, 0x7d0, 0x2a);
        break;
    case 1:
    case 2:
        // port (apply_split_screen_cull): culls as a single view does
        plain(cullDistance, 0x1b80000, 0x3c0000, 0x5c0000, 0x2bc, 0x18);
        break;
    case 3:
        plain(0x960000, 0x640000, 0x280000, 0x500000, 0xfa, 6);
        break;
    default:
        break;
    }
    if (kind <= 3)
    {
        m.word(record + 0x20, 0xfffb0000);
        const bool split = kind == 1 || kind == 2;
        m.word(m.word(record + 0x1c) + 0x1c, split ? 0x380 : 0);
        m.word(m.word(record + 0x1c) + 0x20, split ? 1 : 0);
    }
    const double scale = m.loadDouble(0x536d6c);
    auto scaled = [&](reg32 field) { return M::mul(fild32(m, record + field), scale); };
    const double a = scaled(0x20), b = scaled(0x24), d = scaled(0x28), e = scaled(0x3c);
    const double f = M::mul(scale, fild32(m, record + 0x38));
    m.store(record + 0x30, b);
    m.store(record + 0x34, d);
    m.store(record + 0x44, e);
    m.store(record + 0x40, f);
    m.store(record + 0x2c, a);
    m.word(0x5dd0dc, m.word(0x5dd0dc) + 1);
    cpu.esp += 4;
}

/* sub_41d960: the views for a race: sub_41de40, the track's setup
 * (sub_41d8a0, sub_40f280, stopping at [0x55e88c]), sub_41dae0, sub_41deb0,
 * the view records (sub_41df10: the main view and the mirror, or the two
 * halves of a split screen with [0x6fd3b0] 1), then sub_41d310 ..
 * sub_41d900. */
void raceViews(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    // push ebx, edx, esi, ebp; mov ebp, esp
    Calls out(app, cpu, entry.esp - 16);
    m.word(0x5dd834, 0x1234);
    if (!out.call(0x41de40))
        return;
    if (m.word(0x55e88c) == 0)
    {
        // sub_41d8a0 pushes eax, the setup's name, for its sprintf
        if (!out.call(0x41d8a0, {0x536d10}) || !out.call(0x40f280, {out.result()}))
            return;
        if (m.word(0x55e88c) == 0)
        {
            if (!out.call(0x41dae0) || !out.call(0x41deb0))
                return;
            const bool split = gameSettings(app).playMode == 1;
            if (!out.call(0x41df10, {split ? 1u : 0u, 0x10, 0x5dd090})
                || !out.call(0x41df10, {split ? 2u : 3u, split ? 0x20u : 0x10u, 0x5dd0b4}))
                return;
            if (!out.call(0x41d310, {0}) || !out.call(0x41d620) || !out.call(0x419b60)
                || !out.call(0x41d8a0, {0x536d14}) || !out.call(0x41dcc0, {out.result()}) || !out.call(0x41dc60)
                || !out.call(0x41a100) || !out.call(0x470b90, {gameSettings(app).track}) || !out.call(0x41d900))
                return;
        }
    }
    entry.restore(cpu, Entry::kEbx | Entry::kEdx | Entry::kEsi | Entry::kEbp);
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
void smoke(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 result = cpu.esi, view = cpu.eax, count = cpu.edx;
    // push ebx, ecx, edi, ebp; mov ebp, esp; sub esp, 0x5c
    const reg32 frame = entry.esp - 16;
    const reg32 records = frame - 0x3c, none = frame - 0x5c;  // handed to sub_4bbe40, sub_4bbe70
    Calls out(app, cpu, frame - 0x5c);
    const reg32 group = m.word(view + 4);
    const reg32 colour = glareDarkened(app, effectGrey(app), 0x5497e4);
    reg32 first = 0, last = 0;
    if (!out.call(0x4bbe40, {records, count * 0xa0}))
        return;
    reg32 quads = 0;
    if (out.result())
    {
        reg32 record = m.word(records);
        out.push(kParticlePoints);
        if (!out.call(0x4bf4c0, {count, kParticlePositions + group * kParticleGroupBytes, view + 0x44, view + 0x38}))
            return;
        for (reg32 i = 0; i32(i) < i32(count); ++i)
        {
            const reg32 particle = group * kParticleGroupBytes + i * 12;
            const reg32 position = kParticlePositions + particle;
            if (m.byte(kParticleStates + particle + 8))
                continue;
            const reg32 v = kParticlePoints + (i << 5);
            if (fcomAbove(m.load(v + 8), m.loadDouble(0x5497ec)) && fcomAbove(0.0, m.load(v + 8)))
            {
                if (!respawnParticle(out, view, position, 0x5497f4))
                    return;
                continue;
            }
            if (!fcomBelow(0.0, m.load(v + 8)) || fcomBelow(fild32(m, 0x8ca24c), m.load(v + 8)) || m.byte(v + 0x14))
                continue;
            if (fcomBelow(m.load(v + 8), m.loadDouble(0x5497fc)))
            {
                out.push(m.word(position + 8));
                out.push(m.word(position + 4));
                out.push(m.word(position));
                if (!out.call(0x4dd930, {view}))
                    return;
                if (out.result())
                {
                    if (!respawnParticle(out, view, position, 0x5497f4))
                        return;
                    continue;
                }
            }
            // The quad, its size less the depth times [0x8ca250], as a float
            const double base = m.load(0x8ca230);
            const double size =
                double(float(M::mul(M::sub(base, M::mul(M::mul(m.load(v + 8), base), m.load(0x8ca250))),
                                    m.loadDouble(0x549804))));
            const double w = fcomBelowOrEqual(1.0, size) ? size : 1.0;
            app->getMemory<x86::reg16>(record + 4) = 0;
            m.word(record + 0x18, 0x8ca200);
            app->getMemory<x86::reg16>(record + 6) = 1;
            m.word(record + 8, record + 0x20);
            m.word(record + 0xc, record + 0x40);
            m.word(record + 0x10, record + 0x60);
            m.word(record + 0x14, record + 0x80);
            m.word(record, record + 0xa0);
            for (int c = 0; c < 4; ++c)
            {
                const reg32 q = record + 0x20 * (c + 1);
                if (c == 0)
                {
                    m.word(q, m.word(v));
                    m.word(q + 4, m.word(v + 4));
                    m.word(q + 8, m.word(v + 8));
                }
                else
                {
                    m.store(q, c == 3 ? m.load(v) : M::add(m.load(v), w));
                    m.store(q + 4, c == 1 ? m.load(v + 4) : M::add(m.load(v + 4), w));
                    m.store(q + 8, m.load(v + 8));
                }
                m.store(q + 0xc, m.load(v + 0xc));
                m.word(q + 0x10, colour);
                m.byte(q + 0x14, screenCodeOf(app, q));
            }
            record += 0xa0;
            ++quads;
        }
        if (quads)
        {
            last = record - 0xa0;
            if (!out.call(0x4bbde0, {records, quads * 5 << 5}))
                return;
            first = m.word(records);
            m.word(last, 0);
        }
    }
    if (!quads)
    {
        cpu.esi = none;  // sub_4bbe70's argument
        if (!out.call(0x4bbe70))
            return;
        first = m.word(none);
        last = m.word(none + 4);
    }
    m.word(result, first);
    m.word(result + 4, last);
    cpu.esi = frame - 0x54;  // as the code's copy of the two words leaves it
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdi | Entry::kEbp);
}

/* sub_4de700: a view's spray streaks (view in eax, its group at +4; edx
 * particles; ebx where the count of live ones goes; the result's two words
 * to esi, which is returned): as sub_4de070, but each particle drawn as a
 * streak from where it was on the screen last time (its state's +0, +4,
 * kept with byte 9) to where it is now, half way between scaled by
 * [0x54982c]; quads of 0x80 bytes (flags 1 at +4 and +6), a respawned
 * particle also handed to sub_414f90, [0x54981c] the scatter and the
 * streak's width. */
void streaks(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 result = cpu.esi, view = cpu.eax, count = cpu.edx, liveCount = cpu.ebx;
    // push ecx, edi, ebp; mov ebp, esp; sub esp, 0x60
    const reg32 frame = entry.esp - 12;
    const reg32 records = frame - 0x38, none = frame - 0x60;  // handed to sub_4bbe40, sub_4bbe70
    Calls out(app, cpu, frame - 0x60);
    const reg32 group = m.word(view + 4);
    reg32 colour = effectGrey(app);
    reg32 first = 0, last = 0, quads = 0;
    m.word(liveCount, 0);
    if (!out.call(0x4bbe40, {records, count << 7}))
        return;
    if (out.result())
    {
        reg32 record = m.word(records);
        colour = glareDarkened(app, colour, 0x54980c);
        out.push(kParticlePoints);
        if (!out.call(0x4bf4c0, {count, kParticlePositions + group * kParticleGroupBytes, view + 0x44, view + 0x38}))
            return;
        reg32 live = 0;
        for (reg32 i = 0; i32(i) < i32(count); ++i)
        {
            const reg32 particle = group * kParticleGroupBytes + i * 12;
            const reg32 position = kParticlePositions + particle, state = kParticleStates + particle;
            if (m.byte(state + 8))
                continue;
            ++live;
            const reg32 v = kParticlePoints + (i << 5);
            // put back, or out of sight: no last place
            auto lost = [&]() { m.byte(state + 9, 0); };
            if (fcomAbove(m.load(v + 8), m.loadDouble(0x549814)) && fcomAbove(0.0, m.load(v + 8)))
            {
                if (!respawnParticle(out, view, position, 0x54981c))
                    return;
                lost();
                continue;
            }
            if (!fcomBelow(0.0, m.load(v + 8)) || fcomBelow(fild32(m, 0x8ca24c), m.load(v + 8)) || m.byte(v + 0x14))
            {
                lost();
                continue;
            }
            if (fcomBelow(m.load(v + 8), m.loadDouble(0x549824)))
            {
                out.push(m.word(position + 8));
                out.push(m.word(position + 4));
                out.push(m.word(position));
                if (!out.call(0x4dd930, {view}))
                    return;
                if (out.result())
                {
                    cpu.edi = m.word(position + 4);  // as the code has it: sub_414f90 reads edi
                    if (!out.call(0x414f90, {group, 0xffffffff}) || !respawnParticle(out, view, position, 0x54981c))
                        return;
                    lost();
                    continue;
                }
            }
            // The streak from the last place (state +0, +4) to this one, half way scaled by [0x54982c].
            const reg32 x = m.word(v), y = m.word(v + 4);
            const double k = m.loadDouble(0x54982c);
            reg32 fromX = x, fromY = y;
            if (m.byte(state + 9))
            {
                fromY = asBits(float(M::mul(M::add(loaded(y), m.load(state + 4)), k)));
                fromX = asBits(float(M::mul(k, M::add(loaded(x), m.load(state)))));
            }
            else
            {
                m.word(state, x);
                m.word(state + 4, y);
            }
            const double toX = M::mul(M::add(loaded(x), m.load(state)), k);
            const double toY = M::mul(k, M::add(loaded(y), m.load(state + 4)));
            m.byte(state + 9, 1);
            m.store(state, toX);
            m.store(state + 4, toY);
            app->getMemory<x86::reg16>(record + 4) = 1;
            m.word(record + 0x18, 0x8ca200);
            app->getMemory<x86::reg16>(record + 6) = 1;
            m.word(record + 8, record + 0x20);
            m.word(record + 0xc, record + 0x40);
            m.word(record + 0x10, record + 0x60);
            m.word(record, record + 0x80);
            // The two ends, then the second again shifted along x by [0x54981c].
            const reg32 ends[][2] = {{fromX, fromY}, {x, y}};
            for (int c = 0; c < 2; ++c)
            {
                const reg32 q = record + 0x20 * (c + 1);
                m.word(q, ends[c][0]);
                m.word(q + 4, ends[c][1]);
                m.word(q + 8, m.word(v + 8));
                m.store(q + 0xc, m.load(v + 0xc));
                m.word(q + 0x10, colour);
                m.byte(q + 0x14, screenCodeOf(app, q));
            }
            for (reg32 k8 = 0; k8 < 0x20; k8 += 4)
                m.word(record + 0x60 + k8, m.word(record + 0x40 + k8));
            m.store(record + 0x60, M::add(m.load(record + 0x60), m.load(0x54981c)));
            m.byte(record + 0x74, screenCodeOf(app, record + 0x60));
            ++quads;
            record += 0x80;
        }
        m.word(liveCount, live);
        if (quads)
        {
            last = record - 0x80;
            if (!out.call(0x4bbde0, {records, quads << 7}))
                return;
            first = m.word(records);
            m.word(last, 0);
        }
    }
    if (!quads)
    {
        cpu.esi = none;  // sub_4bbe70's argument
        if (!out.call(0x4bbe70))
            return;
        first = m.word(none);
        last = m.word(none + 4);
    }
    m.word(result, first);
    m.word(result + 4, last);
    cpu.esi = frame - 0x58;  // as the code's copy of the two words leaves it
    entry.restore(cpu, Entry::kEcx | Entry::kEdi | Entry::kEbp);
}


/* sub_41d620: the views' distances for View Distance ([0x6fbc28], 0 Full):
 * the far, fog-near and fog-far distances of the main view (or both split
 * halves) times the multiplier for the setting from 0x41a8b0 (reduced,
 * [0x6fd4c8]) or 0x41a8bc, in 16.16; at Full the fog to the far distance.
 * Ports: at Full the far distance stays whole and a split view's reaches
 * 500, and the mirror takes the main view's distances.  Then every view's
 * distances as floats times [0x536ce4]. */
void viewDistances(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x18
    const reg32 frame = cpu.esp - 24;
    const i32 setting = i32(gameSettings(app).viewDistance - 1);
    const reg32 row = (setting < 0 ? 0u : reg32(setting)) * 4;
    // port (apply_view_distance): Full leaves the far distance whole
    const reg32 reduced = viewDistanceReduced(app, gameSettings(app).night);
    // the multipliers copied to the frame, read there by the setting (a large one reads past them)
    const reg32 table = reduced ? frame - 0x18 : frame - 0xc;
    copyWords(app, table, reduced ? 0x41a8b0 : 0x41a8bc, 3);
    // 16.16 times 16.16, rounded at bit 15 (imul, shl, shr, adc)
    auto scale = [&](reg32 distance) {
        const std::uint64_t product = std::uint64_t(x86::sreg64(x86::sreg32(distance)) * x86::sreg32(m.word(table + row)));
        const reg32 low = reg32(product), high = reg32(product >> 32) << 16;
        return (low >> 16) + high + ((low >> 15) & 1);
    };
    // eax as the code leaves it, which a caller reads: the fog's far distance, at Full the far
    // distance, then the last view's record number times 21
    reg32 left;
    if (gameSettings(app).playMode == 1)
    {
        // port (apply_view_distance): Full reaches 500 in split screen too
        const reg32 far = scale(reduced ? 0x1b80000 : splitFarDistance(app, 0x1b80000));
        m.word(0x5dd15c, far);
        m.word(0x5dd108, far);
        const reg32 fogNear = scale(0x3c0000);
        m.word(0x5dd16c, fogNear);
        m.word(0x5dd118, fogNear);
        left = scale(0x5c0000);
        m.word(0x5dd170, left);
    }
    else
    {
        // port: Full reaches past 500 (nfs3hp_main.cpp's kFullFarDistance)
        m.word(0x5dd108, scale(reduced ? 0x1f40000 : fullFarDistance(app, 0x1f40000)));
        m.word(0x5dd118, scale(0x6e0000));
        left = scale(0x960000);
    }
    m.word(0x5dd11c, left);
    if (gameSettings(app).viewDistance == 0)
    {
        left = m.word(0x5dd108);
        m.word(0x5dd11c, left);
        m.word(0x5dd118, left);
        if (gameSettings(app).playMode == 1)
        {
            left = m.word(0x5dd15c);
            m.word(0x5dd170, left);
            m.word(0x5dd16c, left);
        }
    }
    // port (apply_mirror_detail): the mirror at the main view's distances
    mirrorFollowsMain(app);
    const double factor = m.loadDouble(0x536ce4);
    for (reg32 view = 0; i32(view) < i32(m.word(0x5dd0dc)); ++view)
    {
        left = view * 21;
        const reg32 record = 0x5dd0e0 + left * 4;
        const double a = M::mul(fild32(m, record + 0x20), factor);
        const double b = M::mul(fild32(m, record + 0x24), factor);
        const double d = M::mul(fild32(m, record + 0x28), factor);
        const double e = M::mul(fild32(m, record + 0x3c), factor);
        const double f = M::mul(factor, fild32(m, record + 0x38));
        m.store(record + 0x2c, a);
        m.store(record + 0x30, b);
        m.store(record + 0x34, d);
        m.store(record + 0x44, e);
        m.store(record + 0x40, f);
    }
    cpu.eax = left;
    cpu.esp += 4;
}

namespace
{

// port (apply_transform_buffer): four times the original's transform buffer.
constexpr reg32 kTransformBufferBytes = 4u * 799744u;
constexpr reg32 kTransformBuffer = 0x53ec28;
constexpr reg32 kAllocate = 0x4e1620;  // the game's allocator: edx bytes for the block eax names, ebx its kind

/* 0x4b56e0's work: the transform buffer's size to `size` and the buffer allocated (kAllocate) from
 * a frame three words below the return address.  The allocator's eax; nullopt when the game
 * stopped. */
std::optional<reg32> allocateTransformBuffer(App* app, CPU& cpu, reg32 size)
{
    Calls out(app, cpu, cpu.esp - 12);  // push ebx, edx, ebp; mov ebp, esp
    put(app, size, kTransformBufferBytes);
    if (!out.call(kAllocate, {kTransformBuffer, kTransformBufferBytes, 0}))
        return std::nullopt;
    return out.result();
}

}

/* sub_4b56e0: allocateTransformBuffer for eax, the allocator's eax left in eax. */
void transformBuffer(win32::WinApplication* app, x86::CPU& cpu)
{
    const Entry entry(cpu);
    const std::optional<reg32> allocated = allocateTransformBuffer(app, cpu, cpu.eax);
    if (!allocated)
        return;
    cpu.eax = *allocated;
    entry.restore(cpu, Entry::kEbx | Entry::kEdx | Entry::kEbp);
}

/* sub_4bef50: a screen mode (eax, at least 1) from the driver's list
 * (THRASH_about's +0x3c count, +0x40 the 40-byte entries): the same as now
 * only refreshes 0x7cdae0 and sub_4bebb0's rectangle; another within the
 * list is set by THRASH_setvideomode (16 bits at most, triple buffered when
 * the entry allows) between clears and flips, then the same refresh, the
 * window and [0x7a3a68] by the buffering. */
void screenMode(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 mode = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 8
    Calls out(app, cpu, entry.esp - 24 - 8);
    auto leave = [&]() { entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp); };
    // sub_4bebb0 with the mode's size, its rectangle copied to 0x7cdac0
    auto refresh = [&]() {
        if (!out.call(0x4bebb0, {0, 0, m.word(0x7cdae0), m.word(0x7cdae4)}))
            return false;
        copyWords(app, 0x7cdac0, 0x7cda60, 8);
        return true;
    };
    // window 2, the 2D state, clear and flip twice
    auto clearTwice = [&]() {
        out.push(2);
        return out.driver<thrash::kThrashWindow, &port::driver::window>() && out.call(0x431900, {3, 0xff000000})
               && out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>()
               && out.driver<thrash::kThrashPageFlip, &port::driver::pageFlip>()
               && out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>()
               && out.driver<thrash::kThrashPageFlip, &port::driver::pageFlip>();
    };

    /* port (Dynamic Fullscreen): the screen measured again as a race opens the
     * mode, so a foldable opened or shut since gets its new shape; a size that
     * changed opens the mode afresh rather than only refreshing it. */
    if (dynamicFullscreenMode(mode) && dynamicFullscreenRefresh(app) && m.word(0x55fec0) == mode)
        m.word(0x55fec0, 0);
    if (!out.driver<thrash::kThrashAbout, &port::driver::about>())
        return;
    const reg32 about = out.result();
    const reg32 wanted = i32(mode) < 1 ? 1 : mode;
    if (wanted == m.word(0x55fec0))
    {
        copyWords(app, 0x7cdae0, wanted * 40 + m.word(about + 0x40), 10);
        if (refresh())
            leave();
        return;
    }
    if (i32(wanted) > i32(m.word(about + 0x3c)))
        return leave();
    // 16 bits at most; triple buffered when the entry allows that depth
    const reg32 allowed = m.word(m.word(about + 0x40) + wanted * 40 + 0x14);
    const reg32 depth = i32(allowed) <= 2 ? allowed : 2;
    const reg32 triple = i32(depth) > i32(m.word(m.word(about + 0x40) + wanted * 40 + 0x18)) ? 0 : 1;
    if (!out.driver<thrash::kThrashFlushWindow, &port::driver::flushWindow>()
        || !out.driver<thrash::kThrashPageFlip, &port::driver::pageFlip>() || !clearTwice())
        return;
    out.push(triple);
    out.push(depth);
    out.push(wanted);
    if (!out.driver<thrash::kThrashSetVideoMode, &port::driver::setVideoMode>())
        return;
    if (out.result())
    {
        const reg32 from = m.word(about + 0x40) + wanted * 40;
        m.word(0x55fec0, wanted);
        copyWords(app, 0x7cdae0, from, 10);
        m.word(0x7a3a68, triple);
        if (!out.call(0x431900, {4, triple}) || !refresh())
            return;
    }
    if (clearTwice())
        leave();
}

/* The listing memory Dynamic Fullscreen needs: 32 MB lists modes up to
 * 8 million pixels, 2600x1080 and more. */
constexpr x86::reg32 kListingMemory = 0x2000000;

/* sub_4b59c0: the driver eax (0..10) started (edx the window's message
 * when it cannot be): its library (0x55fd68) loaded by sub_4f87e0,
 * sub_4f8830, sub_4f8930 (sub_4f2a7d without [0x564364]), the renderer's
 * states set as each driver wants, [0x55fed0], [0x7a3a60], [0x7a3a64],
 * [0x7a3a68], [0x7a3a6c], [0x7a3a70]; the 2D one only on a screen of 8 bits
 * or more, those not built in with a message and sub_4f2a7d. */
void startDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 driver = cpu.eax, message = cpu.edx, kept = cpu.ecx;
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    const reg32 frame = entry.esp - 20;
    Calls out(app, cpu, frame - 0x1c);
    auto setState = [&](reg32 a, reg32 b) {
        out.push(b);
        out.push(a);
        return out.driver<thrash::kThrashSetState, &port::driver::setState>();
    };
    /* sub_4f2a7d, the game's exit, reads no register, but the check takes all four from it (its
     * code runs on into data): eax as the last callee leaves it and the others as the code has
     * them -- ebx the message (1 after sub_4f8b50), edx 0 (sub_4f8b50's), ecx as the caller's; and
     * edi, which its code would read, as the code has it. */
    reg32 edx = 0, ebx = message, ecx = kept, edi = cpu.edi;
    auto quit = [&]() {
        cpu.edi = edi;
        return out.call(0x4f2a7d, {out.result(), edx, ebx, ecx});
    };
    // The driver's library: load, link, start, and quit if that failed.
    auto load = [&](reg32 library) {
        if (!out.call(0x4f87e0, {library}) || !out.call(0x4f8830) || !out.call(0x4f8930))
            return false;
        return m.word(0x564364) != 0 || quit();
    };

    m.word(frame - 4, driver);  // the frame's last word, after the messages' table
    m.word(0x7a3a6c, 0);
    m.word(0x7a3a70, 0);
    if (driver > 0xa)
    {
        if (out.call(0x4b5e50, {message}) && quit())
            entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
        return;
    }
    const reg32 library = m.word(driver * 4 + 0x55fd68);
    switch (driver)
    {
    case 0:
        if (!load(library))
            return;
        m.word(0x7a3a70, 1);
        break;
    case 1:
    case 2:
        if (!load(library) || !setState(0xa, 2) || !setState(3, 0)
            || !out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>() || !setState(4, 0))
            return;
        out.push(0);
        out.push(6);
        m.word(0x7a3a68, 0);
        m.word(0x7a3a60, 0);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>() || !setState(2, 0))
            return;
        m.word(0x7a3a64, 0);
        if (!out.call(0x431c50))
            return;
        m.word(0x55fed0, 0x400000);
        m.word(0x7a3a70, 0);
        break;
    case 3:
    case 4:
    {
        edx = 0x53fd34;
        ebx = 1;
        if (!out.call(0x4f8b50, {0x53fd38, edx, ebx}) || !load(m.word(driver * 4 + 0x55fd68)))
            return;
        for (const reg32 name : {0x53fd48u, 0x53fd54u, 0x53fd60u, 0x53fd6cu, 0x53fd78u, 0x53fd88u, 0x53fd98u, 0x53fda8u})
            if (!out.call(0x4f8b50, {name, 0x53ec28, 1}))
                return;
        if (!setState(0xa, 2) || !out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>())
            return;
        out.push(1);
        out.push(4);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        out.push(0);
        out.push(2);
        m.word(0x7a3a68, 1);
        m.word(0x7a3a60, 1);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        m.word(0x55fed0, 0x800000);
        m.word(0x7a3a70, 1);
        m.word(0x7a3a64, 4);
        break;
    }
    case 5:
        edi = 1;
        m.word(0x7a3a6c, 1);
        if (!load(library) || !setState(0xa, 2)
            || !out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>() || !setState(4, 1))
            return;
        out.push(1);
        out.push(6);
        m.word(0x7a3a68, 1);
        m.word(0x7a3a60, 1);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        out.push(0);
        out.push(2);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        m.word(0x7a3a64, 0);
        m.word(0x55fed0, 0x200000);
        break;
    case 6:
    {
        const std::optional<reg32> desktop = windowsCall<0x534740, &host::desktopWindow>(out, {driver << 2, edx, ebx, ecx});
        if (!desktop)
            return;
        const std::optional<reg32> dc = windowsCall<0x534754, &host::windowDC>(out, {*desktop, edx, ebx, ecx}, *desktop);
        if (!dc)
            return;
        const std::optional<reg32> bits = windowsCall<0x534458, &host::deviceCaps>(out, {*dc, edx, ebx, ecx}, *dc, 0xc);
        if (!bits)
            return;
        if (i32(*bits) < 8)
        {
            // the messages' table copied to the frame, read there by the message
            copyWords(app, frame - 0x1c, 0x4b55b0, 6);
            ecx = 0;
            edi = m.word(frame - 0x1c + message * 4);
            if (!windowsCall<0x534764, &host::messageBox>(out, {*bits, edx, ebx, ecx}, 0,
                                                             m.word(frame - 0x1c + message * 4), 0x53ec28, 0x30)
                || !quit())
                return;
        }
        if (!load(m.word(m.word(frame - 4) * 4 + 0x55fd68)))
            return;
        out.push(m.word(0x55fdbc));
        if (!out.driver<thrash::kThrashSelectDisplay, &port::driver::selectDisplay>())
            return;
        out.push(1);
        out.push(2);
        out.push(1);
        if (!out.driver<thrash::kThrashSetVideoMode, &port::driver::setVideoMode>() || !out.call(0x4b5940)
            || !setState(0xa, 2) || !setState(0x68, 0)
            || !out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>() || !setState(4, 1))
            return;
        out.push(1);
        out.push(6);
        m.word(0x7a3a68, 1);
        m.word(0x7a3a60, 1);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        out.push(0);
        out.push(2);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        m.word(0x7a3a64, 0);
        m.word(0x55fed0, 0x200000);
        break;
    }
    case 7:
    case 8:
        if (!load(library) || !setState(0xa, 2)
            || !out.driver<thrash::kThrashClearWindow, &port::driver::clearWindow>() || !setState(4, 2))
            return;
        out.push(0);
        out.push(2);
        m.word(0x7a3a68, 2);
        m.word(0x7a3a60, 2);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>() || !setState(0xce, 1) || !setState(6, 1))
            return;
        out.push(2);
        out.push(0xc9);
        if (!out.driver<thrash::kThrashSetState, &port::driver::setState>())
            return;
        m.word(0x7a3a64, 0);
        m.word(0x55fed0, 0x200000);
        m.word(0x7a3a70, 1);
        break;
    default:
        if (!windowsCall<0x534764, &host::messageBox>(out, {driver << 2, edx, ebx, ecx}, 0, 0x53fdb4, 0x53ec28, 0x30) || !quit())
            return;
        m.word(0x7a3a70, 1);
        break;
    }
    /* port (Dynamic Fullscreen): the memory sub_4bed90 lists a mode by -- twice
     * its frame, 16-bit, within half of this -- big enough for a race at the
     * screen's own size; nothing else reads it. */
    if (m.word(0x55fed0) < kListingMemory)
        m.word(0x55fed0, kListingMemory);
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_47f650: the frame around a HUD element, eight textured quads at eax
 * (0x80 bytes each, four 0x20-byte corners: x, y at +0, +4, s, t at +0x18,
 * +0x1c) -- the corners and the edges of a 9-slice, the middle left out --
 * from the element's inside, x ebx to [esp+4], y edx to ecx, and a border
 * of hudBorder outside it.  ret 4. */
void hudFrame(win32::WinApplication* app, x86::CPU& cpu)
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
    cpu.eax = m.word(quads + 0x3e4);  // the last corner's y, as the code leaves it: a caller reads eax
    cpu.esp = entry + 8;
}

namespace
{

constexpr reg32 kPlayerCar = 0x422bd0;  // the car of player eax's view, to eax
constexpr reg32 kHudFrame = 0x47f650;   // hudFrame, called by address: the check stands in for it
// The HUD's rectangles in pixels: 23 of 16 bytes (left, top, right, bottom) a player.
constexpr reg32 kHudRects = 0x749758, kHudRectsBytes = 0x170;
// A car's flags: bit 0x20 a cop's (Hot Pursuit), whose HUD has the other layout.
constexpr reg32 kCarFlags = 0x200;
constexpr reg8 kCarCop = 0x20;

/* An element's inside in pixels: its rectangle moved in by a border. */
struct Edges
{
    reg32 left, top, right, bottom;
};

Edges insetBy(App* app, reg32 rect, reg32 border)
{
    return {at(app, rect) + border, at(app, rect + 4) + border, at(app, rect + 8) - border,
            at(app, rect + 0xc) - border};
}

// kPlayerCar for `player`: the car, or nullopt when the game stopped.
std::optional<reg32> playerCar(Calls& out, reg32 player)
{
    if (!out.call(kPlayerCar, {player}))
        return std::nullopt;
    return out.result();
}

/* kHudFrame: the frame's eight quads to `quads` around `inside`, its right edge pushed (ret 4).
 * False when the game stopped. */
bool drawHudFrame(Calls& out, reg32 quads, const Edges& inside)
{
    out.push(inside.right);
    return out.call(kHudFrame, {quads, inside.top, inside.left, inside.bottom});
}

// The HUD layout's records (gameSettings' hudAt): a row by split screen and player, two layouts.
constexpr reg32 kHudRowBytes = 0x348, kHudLayoutBytes = 0x1a4;



/* 0x480910's work: HUD element `element`'s rectangle for `player` in pixels to kHudRects: its
 * layout's fractions (by split screen, the player's car's cop bit) times the width and the
 * height, each made whole toward 0 by __CHP and stored by fistp; then widescreenHudRect (a port,
 * tools/apply_widescreen.py).  False when the game stopped. */
bool placeHudRect(App* app, CPU& cpu, reg32 player, reg32 element, reg32 width, reg32 height)
{
    Calls out(app, cpu, cpu.esp - 12 - 12);  // push esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const reg32 row = player + (gameSettings(app).playMode == 1 ? 1u : 0u);
    const std::optional<reg32> car = playerCar(out, player);
    if (!car)
        return false;
    const reg32 offset = element << 4;
    const reg32 record = row * kHudRowBytes + ((byteAt(app, *car + kCarFlags) & kCarCop) ? kHudLayoutBytes : 0)
                         + offset;
    const HudRect& fractions = gameSettings(app).hudAt(record).rects[0];
    const double across = double(i32(width)), down = double(i32(height));
    const double left = M::mul(across, double(fractions.left));
    const double right = M::mul(across, double(fractions.right));
    const double top = M::mul(down, double(fractions.top));
    const double bottom = M::mul(down, double(fractions.bottom));
    // the four chopped in this order
    std::optional<x86::Float> whole[4];
    const double values[4] = {left, right, top, bottom};
    for (int k = 0; k < 4; ++k)
    {
        whole[k] = chopped(out, x86::Float(values[k]));
        if (!whole[k])
            return false;
    }
    const reg32 slot = offset + player * kHudRectsBytes;
    const reg32 placed[4] = {reg32(fp::toInteger<i32>(*whole[0])), reg32(fp::toInteger<i32>(*whole[1])),
                             reg32(fp::toInteger<i32>(*whole[2])), reg32(fp::toInteger<i32>(*whole[3]))};
    put(app, kHudRects + slot, placed[0]);
    put(app, kHudRects + slot + 8, placed[1]);
    put(app, kHudRects + slot + 4, placed[2]);
    put(app, kHudRects + slot + 0xc, placed[3]);
    // port (apply_widescreen): pictures in the HUD keep their shape on a wide screen
    widescreenHudRect(app, slot);
    return true;
}

}

/* sub_480910: placeHudRect for player eax, element edx, width ebx, height ecx (no caller reads
 * what it leaves in eax). */
void hudRect(win32::WinApplication* app, x86::CPU& cpu)
{
    const Entry entry(cpu);
    if (placeHudRect(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx))
        entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

namespace
{

/* A HUD quad's corners as floats at `quad`: x at +0, +0x60 (left) and +0x20, +0x40 (right), y at
 * +4, +0x24 (top) and +0x44, +0x64 (bottom), each pair in the order the game writes them. */
void hudQuadLeft(App* app, reg32 quad, i32 x)
{
    const Memory m{app};
    m.store(quad + 0x60, double(x));
    m.store(quad, double(x));
}

void hudQuadRight(App* app, reg32 quad, i32 x)
{
    const Memory m{app};
    m.store(quad + 0x40, double(x));
    m.store(quad + 0x20, double(x));
}

void hudQuadTop(App* app, reg32 quad, i32 y)
{
    const Memory m{app};
    m.store(quad + 0x24, double(y));
    m.store(quad + 4, double(y));
}

/* (a + b) / 2 rounded toward 0, as the code's sar, sub, sar does it, wrapping as it does. */
reg32 middle(reg32 a, reg32 b)
{
    const reg32 sum = a + b;
    return reg32(i32(sum - reg32(i32(sum) >> 31)) >> 1);
}

// Element 1, the picture: its frame's quads (0x400 bytes a player) and its quad (0x100).
constexpr reg32 kPictureRect = 0x10, kPictureFrames = 0x791a8c, kPictureQuads = 0x791b38;
constexpr reg32 kPictureBorder = 0x53b518;  // two floats, the border's scale
constexpr reg32 kQuadCentre = 0xc0;          // the centre's x, y after the four corners

/* 0x480fc0's work: HUD element 1 for `player`: its frame a border inside its rectangle, and the
 * picture in it the same inset, with its centre.  Returns the centre's y; nullopt when the game
 * stopped. */
std::optional<reg32> placeHudPicture(App* app, CPU& cpu, reg32 player)
{
    const Memory m{app};
    Calls out(app, cpu, cpu.esp - 24 - 4);  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const reg32 rect = kHudRects + player * kHudRectsBytes + kPictureRect;
    // the border by fistp each time it is used: the same each time
    const Edges inside = insetBy(app, rect, hudBorder(app, cpu, kPictureBorder, kPictureBorder + 4));
    if (!drawHudFrame(out, (player << 10) + at(app, kPictureFrames), inside))
        return std::nullopt;
    // each edge read as it is written: the quad may lie over the rectangle
    const reg32 border = hudBorder(app, cpu, kPictureBorder, kPictureBorder + 4);
    const reg32 quad = at(app, kPictureQuads) + (player << 8);
    hudQuadLeft(app, quad, i32(border + at(app, rect)));
    hudQuadRight(app, quad, i32(at(app, rect + 8) - border));
    hudQuadTop(app, quad, i32(border + at(app, rect + 4)));
    const i32 bottom = i32(at(app, rect + 0xc) - border);
    m.store(quad + 0x64, double(bottom));
    m.store(quad + 0x44, double(bottom));
    m.store(quad + kQuadCentre, double(i32(middle(at(app, rect), at(app, rect + 8)))));
    const reg32 centreY = middle(at(app, rect + 4), at(app, rect + 0xc));
    m.store(quad + kQuadCentre + 4, double(i32(centreY)));
    return centreY;
}

// Element 3, the dial: its frame's quads (0x400 bytes a player) and its quad (0xe0).
constexpr reg32 kDialRect = 0x30, kDialFrames = 0x791b34, kDialQuads = 0x791b2c;
constexpr reg32 kDialBorder = 0x53b520;  // two floats, the border's scale
constexpr reg32 kDialWidthScale = 0x53b528, kDialCentreScale = 0x53b52c;
constexpr reg32 kDialWidths = 0x724760, kDialCentres = 0x724768;
constexpr reg32 kDialForm = 3;  // tachometerForm's dial with a centre

/* What placeHudDial chose: the layout's offset (0 or kHudLayoutBytes) and the tachometer's form. */
struct DialLayout
{
    reg32 layout, form;
};

/* 0x4812e0's work: HUD element 3 for `player`: its frame a border inside its rectangle, the dial's
 * quad laid over the frame's inside corners, its width times [kDialWidthScale] to kDialWidths
 * and, for the layout's dial with a centre, its centre times [kDialCentreScale] to kDialCentres.
 * nullopt when the game stopped. */
std::optional<DialLayout> placeHudDial(App* app, CPU& cpu, reg32 player)
{
    const Memory m{app};
    Calls out(app, cpu, cpu.esp - 24 - 12);  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    const reg32 row = player + (gameSettings(app).playMode == 1 ? 1u : 0u);
    const std::optional<reg32> car = playerCar(out, player);
    if (!car)
        return std::nullopt;
    const reg32 layout = (byteAt(app, *car + kCarFlags) & kCarCop) ? kHudLayoutBytes : 0;
    const reg32 rect = kHudRects + player * kHudRectsBytes + kDialRect;
    const Edges inside = insetBy(app, rect, hudBorder(app, cpu, kDialBorder, kDialBorder + 4));
    if (!drawHudFrame(out, at(app, kDialFrames) + (player << 10), inside))
        return std::nullopt;
    const double border = double(i32(hudBorder(app, cpu, kDialBorder, kDialBorder + 4)));
    const reg32 quad = at(app, kDialQuads) + player * 0xe0;
    // the frame drawn, its inside corners: [kDialFrames] read again each time
    auto frameAt = [&](reg32 offset) { return m.load(at(app, kDialFrames) + (player << 10) + offset); };
    auto place = [&](double value, reg32 first, reg32 second) {
        m.store(quad + first, value);
        m.store(quad + second, value);
    };
    place(M::add(border, frameAt(0)), 0x60, 0);
    place(M::add(border, frameAt(4)), 0x24, 4);
    place(M::sub(frameAt(0x240), border), 0x40, 0x20);
    const double width = M::mul(M::sub(m.load(quad + 0x20), m.load(quad)), m.load(kDialWidthScale));
    place(M::sub(frameAt(0x244), border), 0x64, 0x44);
    const reg32 form = gameSettings(app).hudAt(row * kHudRowBytes + layout).tachometerForm;
    m.store(player * 4 + kDialWidths, width);
    if (form == kDialForm)
    {
        const double scale = m.load(kDialCentreScale);
        m.store(player * 8 + kDialCentres, M::mul(M::add(m.load(quad), m.load(quad + 0x40)), scale));
        m.store(player * 8 + kDialCentres + 4, M::mul(scale, M::add(m.load(quad + 4), m.load(quad + 0x44))));
    }
    return DialLayout{layout, form};
}

// Element 17 (22 for a cop): the panel's frame quads (0x400 bytes a player) and its quad (0x80).
constexpr reg32 kPanelRect = 0x110, kCopPanelRect = 0x160, kPanelFrames = 0x791b40, kPanelQuads = 0x791b58;
constexpr reg32 kPanelBorder = 0x53b58c;  // two floats, the border's scale
constexpr reg32 kPanelText = 0x700230;    // four words cleared

/* 0x481aa0's work: HUD element 17 for `player` (22 for a cop's car): its frame a border inside its
 * rectangle, its quad the same inset, and kPanelText's four words cleared.  False when the game
 * stopped. */
bool placeHudPanel(App* app, CPU& cpu, reg32 player)
{
    const Memory m{app};
    Calls out(app, cpu, cpu.esp - 24 - 4);  // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 4
    const std::optional<reg32> car = playerCar(out, player);
    if (!car)
        return false;
    const reg32 rect = kHudRects + player * kHudRectsBytes
                       + ((byteAt(app, *car + kCarFlags) & kCarCop) ? kCopPanelRect : kPanelRect);
    const Edges inside = insetBy(app, rect, hudBorder(app, cpu, kPanelBorder, kPanelBorder + 4));
    if (!drawHudFrame(out, at(app, kPanelFrames) + (player << 10), inside))
        return false;
    // each edge read as it is written: the quad may lie over the rectangle
    const reg32 border = hudBorder(app, cpu, kPanelBorder, kPanelBorder + 4);
    const reg32 quad = (player << 7) + at(app, kPanelQuads);
    hudQuadLeft(app, quad, i32(border + at(app, rect)));
    hudQuadRight(app, quad, i32(at(app, rect + 8) - border));
    hudQuadTop(app, quad, i32(border + at(app, rect + 4)));
    const i32 bottom = i32(at(app, rect + 0xc) - border);
    put(app, kPanelText + 4, 0);
    put(app, kPanelText + 0xc, 0);
    put(app, kPanelText + 8, 0);
    m.store(quad + 0x64, double(bottom));
    put(app, kPanelText, 0);
    m.store(quad + 0x44, double(bottom));
    return true;
}

}

/* sub_480fc0: placeHudPicture for player eax, the centre's y left in eax. */
void hudPicture(win32::WinApplication* app, x86::CPU& cpu)
{
    const Entry entry(cpu);
    const std::optional<reg32> centreY = placeHudPicture(app, cpu, cpu.eax);
    if (!centreY)
        return;
    cpu.eax = *centreY;
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_4812e0: placeHudDial for player eax: the layout's offset left in eax. */
void hudDial(win32::WinApplication* app, x86::CPU& cpu)
{
    const Entry entry(cpu);
    const std::optional<DialLayout> dial = placeHudDial(app, cpu, cpu.eax);
    if (!dial)
        return;
    cpu.eax = dial->layout;
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_481aa0: placeHudPanel for player eax, eax 0. */
void hudPanel(win32::WinApplication* app, x86::CPU& cpu)
{
    const Entry entry(cpu);
    if (!placeHudPanel(app, cpu, cpu.eax))
        return;
    cpu.eax = 0;
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
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
void hudTable(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 player = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc
    Calls out(app, cpu, entry.esp - 24 - 12);
    auto border = [&]() { return hudBorder(app, cpu, 0x53b5a8, 0x53b5ac); };
    // a value through __CHP and fistp
    auto truncate = [&](double value) -> std::optional<reg32> {
        const std::optional<x86::Float> whole = chopped(out, x86::Float(value));
        if (!whole)
            return std::nullopt;
        return reg32(fp::toInteger<x86::sreg32>(double(*whole)));
    };
    const std::optional<reg32> car = playerCar(out, player);
    if (!car)
        return;
    const bool cops = m.word(*car + kCarFlags) & kCarCop;
    if (!setTextScale(out, m.word(0x791b4c)))
        return;
    const reg32 slot = player * 0x170;
    reg32 top, bottom;
    if (cops)
    {
        // the cop's: a row for each speeder in the table
        reg32 speeders = 0;
        for (reg32 k = 0; i32(k) < i32(m.word(0x5efda0)); ++k)
            if (m.word(k * 4 + 0x725238))
                ++speeders;
        if (!out.call(0x4d14f0, {0x30}))
            return;
        const reg32 above = out.result();
        if (!out.call(0x4d1540, {0x30}))
            return;
        // port (apply_hud_scale): the cop's rows as close as the Modern Patch has them, was [0x53b598] = 20
        const std::optional<reg32> height =
            truncate(M::mul(M::mul(double(i32(out.result() + above)), 12.414), m.loadDouble(0x53b5a0)));
        if (!height)
            return;
        const reg32 rows = *height * speeders;
        if (m.byte(player * 92 + 0x7256e0) & 0x10)
        {
            const reg32 i = border();
            top = m.word(slot + 0x7498a4) - rows - (i + i + 4);
            bottom = m.word(slot + 0x7498a4);
            m.word(player * 4 + 0x724780, top);
        }
        else
        {
            const reg32 i = border();
            top = m.word(slot + 0x74989c);
            m.word(player * 4 + 0x724780, top);
            bottom = i + i + 4 + (rows + top);
        }
    }
    else
    {
        // the racers': [0x7254f8] rows
        // port (apply_hud_scale): 4:3 proportions, was [0x7cdacc]
        const double size = M::mul(M::mul(double(x86::sreg32(hudReferenceHeight(app))), m.loadDouble(0x53b5b0)),
                                   m.loadDouble(0x53b5b8));
        const std::optional<reg32> rowHeight = truncate(size);
        if (!rowHeight || !out.call(0x4d1540, {0x49}))
            return;
        const reg32 rows = (*rowHeight + out.result()) * m.word(0x7254f8);
        if (m.byte(player * 92 + 0x7256c4) & 0x10)
        {
            const reg32 i = border();
            top = m.word(slot + 0x749834) - rows - (i + i + 4);
            bottom = m.word(slot + 0x749834);
            m.word(player * 4 + 0x724780, top);
        }
        else
        {
            const reg32 i = border();
            top = m.word(slot + 0x74982c);
            m.word(player * 4 + 0x724780, top);
            bottom = i + i + 4 + (rows + top);
        }
    }
    m.word(player * 4 + 0x724788, bottom);
    // the frame
    const reg32 rect = kHudRects + slot + (cops ? 0x140 : 0xd0);
    const reg32 width = cops ? 0x7925cc : 0x7925c8;
    const reg32 i = border();
    const Edges inside = {m.word(rect) + i, m.word(player * 4 + 0x724780) + i, m.word(width) + m.word(rect),
                          m.word(player * 4 + 0x724788)};
    if (!drawHudFrame(out, (player << 10) + m.word(0x791b18), inside))
        return;
    const reg32 j = border();
    const reg32 quad = m.word(0x791ad4) + (player << 7);
    auto corner = [&](reg32 value, reg32 a, reg32 b) {
        m.store(quad + a, double(i32(value)));
        m.store(quad + b, double(i32(value)));
    };
    corner(j + m.word(rect), 0x60, 0);
    corner(m.word(width) + m.word(rect), 0x40, 0x20);
    const reg32 topEdge = j + m.word(player * 4 + 0x724780);
    corner(topEdge, 0x24, 4);
    const double bottomEdge = fild32(m, player * 4 + 0x724788);
    m.store(quad + 0x64, bottomEdge);
    m.store(quad + 0x44, bottomEdge);
    // port (apply_hud_editor): the table's rectangle is its panel
    hudTablePanel(app, player, cops);
    cpu.eax = topEdge;  // as the code leaves it: a caller reads eax
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_487210: the rectangle of floats at edx (x0, y0 at +0, +4, x1, y1 at +8,
 * +0xc) kept for player eax at 0x791b60 and, where sub_481f90 has a frame
 * round it, moved in by a border (4:3, a port) at each side; then a right or
 * bottom edge on the screen's last column or row ([0x7cdac8], [0x7cdacc], not
 * ported: these are the screen's) is moved in by one. */
void hudInset(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 player = cpu.eax;
    const reg32 kept = player * 4 + 0x791b60;
    // push ebx, ecx, ebp; mov ebp, esp; sub esp, 4
    Calls out(app, cpu, entry.esp - 12 - 4);
    m.word(kept, cpu.edx);
    if (!out.call(0x481f90, {player}))
        return;
    if (out.result())
    {
        const double border = double(i32(hudBorder(app, cpu, 0x53b690, 0x53b694)));
        // the rectangle's pointer read again for each edge: an edge may lie over it
        m.store(m.word(kept), M::add(border, m.load(m.word(kept))));
        m.store(m.word(kept) + 4, M::add(border, m.load(m.word(kept) + 4)));
        m.store(m.word(kept) + 0xc, M::sub(m.load(m.word(kept) + 0xc), border));
        m.store(m.word(kept) + 8, M::sub(m.load(m.word(kept) + 8), border));
    }
    // fcomp; fnstsw ax; sahf; je: the screen's size against the edge
    reg32 rect = m.word(kept);
    if (fcomEqual(fild32(m, 0x7cdac8), m.load(rect + 0xc)))
        m.store(rect + 0xc, double(i32(m.word(0x7cdac8) - 1)));
    rect = m.word(kept);
    if (fcomEqual(fild32(m, 0x7cdacc), m.load(rect + 8)))
        m.store(rect + 8, double(i32(m.word(0x7cdacc) - 1)));
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEbp);
}

/* sub_48c7c0: the three-light start (and finish) panel: the quads at
 * 0x724790, 0x724810, 0x724890 drawn by THRASH_drawquad ([0x9ef94c]) with the
 * texture state (sub_431900) [0x749a38], under bit 2 of [0x7a3a58] state
 * 0x2bf on round the rest; then the lamp at 0x724d10 lit (0xffdcdb58, unlit
 * 0xff797c93) when [esp+4] is 0, the quads eax and edx (0x80 bytes each from
 * 0x724790) when it is 0 and 2, the lamp at 0x724c90, ebx and ecx when it is
 * 1, 1 and 3.  ret 4. */
void startLights(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 lights[4] = {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx};
    const reg32 lit = m.word(entry.esp + 4);
    // push esi, edi, ebp; mov ebp, esp; sub esp, 8
    Calls out(app, cpu, entry.esp - 12 - 8);
    // push a+0x60, a+0x40, a+0x20, a; call [0x9ef94c]: a quad's four corners
    auto quad = [&](reg32 corners) {
        for (const reg32 k : {0x60u, 0x40u, 0x20u, 0u})
            out.push(corners + k);
        return out.driver<thrash::kThrashDrawQuad, &port::driver::drawQuad>();
    };
    // the four corners' colours
    auto colour = [&](reg32 corners, reg32 value) {
        for (const reg32 k : {0x30u, 0x50u, 0x70u, 0x10u})
            m.word(corners + k, value);
    };
    auto choose = [](bool on) { return on ? 0xffdcdb58u : 0xff797c93u; };
    // the quad of light `index`
    auto light = [&](reg32 index, bool on) {
        colour((index << 7) + 0x724790, choose(on));
        return quad((index << 7) + 0x724790);
    };
    if (!out.call(0x431900, {1, m.word(0x749a38)}))
        return;
    for (const reg32 corners : {0x724790u, 0x724810u, 0x724890u})
        if (!quad(corners))
            return;
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 1}))
        return;
    for (const reg32 at : {0x724d40u, 0x724d60u, 0x724d80u, 0x724d20u})
        m.word(at, choose(lit == 0));
    if (!quad(0x724d10) || !light(lights[0], lit == 0) || !light(lights[1], lit == 2))
        return;
    for (const reg32 at : {0x724cc0u, 0x724ce0u, 0x724d00u, 0x724ca0u})
        m.word(at, choose(lit == 1));
    if (!quad(0x724c90) || !light(lights[2], lit == 1) || !light(lights[3], lit == 3))
        return;
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 0}))
        return;
    // eax as the last callee leaves it: a caller reads eax
    entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp, 4);
}

/* sub_4880b0: the cockpit's needles for the car at eax: the speedometer's
 * (when [0x7922d0]) at the angle [0x7922e0] times |the car's +0x4f4| plus
 * [0x7922f4], the tachometer's (when [0x7922e8]) at the car's +0x590 times
 * [0x7922ec] and [0x53b6ac] plus [0x7922f0]; each a quad (the template at
 * 0x488030, its colour sub_4206f0's, its texture [0x7922d8], [0x7922d4])
 * from the dial's centre ([0x792040], [0x792044] or [0x792068], [0x79206c],
 * scaled to the cabin, a port: cabinRect) along the angle (sub_4ea970), drawn
 * by THRASH_drawquad under the texture state [0x7922e4] (sub_431900). */
void needles(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 car = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xdc; sub ebp, 0x82
    const reg32 ebp = entry.esp - 24 - 0x82;
    Calls out(app, cpu, entry.esp - 24 - 0xdc);
    // the needle's quad, its four corners from the template at 0x488030 (rep movsd), on the frame
    // for THRASH_drawquad; the angle, its sine and cosine there for sub_4ea970
    const reg32 corners = ebp - 0x5a, angle = ebp + 0x6a, sine = ebp + 0x66, cosine = ebp + 0x6e;
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdba0]
    const double k = single(M::mul(double(i32(cabinRect(app, 2))), m.load(0x53b69c)));
    copyWords(app, corners, 0x488030, 0x20);
    if (!out.call(0x4206f0, {car + 0xc, 1}))
        return;
    for (const reg32 at : {0x16u, 0xfffffff6u, 0xffffffd6u, 0xffffffb6u})
        m.word(ebp + at, out.result());
    /* A needle at the angle: sub_4ea970's sine and cosine; the dial's centre and the needle's ends
     * from the ints at `dial` (+0, +4 the centre, +8 its length) and 0x792038 (its width), [tail]
     * the tail's length; its sides half as wide at the tail. */
    auto needle = [&](reg32 dial, reg32 tail) {
        if (!out.call(0x4ea970, {angle, sine, cosine}))
            return false;
        const double D = m.loadDouble(0x53b6a4);
        const double x = M::mul(fild32(m, dial), k);
        const double y = M::mul(fild32(m, dial + 4), k);
        const double length = M::mul(fild32(m, dial + 8), k);
        const double width = M::mul(M::mul(fild32(m, 0x792038), k), D);
        const double back = M::mul(k, m.load(tail));
        // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdb98], [0x7cdb9c]
        const double centreX = single(M::add(x, double(i32(cabinRect(app, 0)))));
        const double headX = M::mul(length, m.load(cosine));
        const double headY = M::mul(length, m.load(sine));
        const double centreY = single(M::add(y, double(i32(cabinRect(app, 1)))));
        const double tailX = M::mul(back, m.load(cosine));
        const double tailY = M::mul(back, m.load(sine));
        const double sideX = M::mul(-m.load(sine), width);
        const double sideY = M::mul(width, m.load(cosine));
        for (const reg32 at : {0x1eu, 0xfffffffeu, 0xffffffdeu, 0xffffffbeu})
            m.word(ebp + at, m.word(0x7922d8));
        for (const reg32 at : {0x22u, 0x2u, 0xffffffe2u, 0xffffffc2u})
            m.word(ebp + at, m.word(0x7922d4));
        const double headEndX = single(M::add(headX, centreX)), headEndY = single(M::add(headY, centreY));
        const double tailEndX = single(M::add(tailX, centreX)), tailEndY = single(M::add(tailY, centreY));
        m.store(corners, M::add(headEndX, single(sideX)));
        m.store(corners + 4, M::add(headEndY, single(sideY)));
        m.store(corners + 0x20, M::sub(headEndX, single(sideX)));
        m.store(corners + 0x24, M::sub(headEndY, single(sideY)));
        const double halfX = single(M::mul(sideX, D)), halfY = single(M::mul(sideY, D));
        m.store(corners + 0x40, M::sub(tailEndX, halfX));
        m.store(corners + 0x44, M::sub(tailEndY, halfY));
        m.store(corners + 0x60, M::add(tailEndX, halfX));
        m.store(corners + 0x64, M::add(tailEndY, halfY));
        if (!out.call(0x431900, {1, m.word(0x7922e4)}))
            return false;
        for (const reg32 at : {0x60u, 0x40u, 0x20u, 0u})
            out.push(corners + at);
        return out.driver<thrash::kThrashDrawQuad, &port::driver::drawQuad>();
    };
    if (m.word(0x7922d0))
    {
        // fldz; fcomp [car+0x4f4]; fnstsw ax; sahf; jae: the speed's size
        const double speed = fcomBelow(0.0, m.load(car + 0x4f4)) ? m.load(car + 0x4f4) : single(-m.load(car + 0x4f4));
        m.store(angle, M::add(M::mul(m.load(0x7922e0), speed), m.load(0x7922f4)));
        if (!needle(0x792040, 0x7922dc))
            return;
    }
    if (m.word(0x7922e8))
    {
        m.store(angle, M::add(M::mul(M::mul(fild32(m, car + 0x590), m.load(0x7922ec)), m.load(0x53b6ac)),
                              m.load(0x7922f0)));
        if (!needle(0x792068, 0x7922f8))
            return;
    }
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_488470: the cockpit's bar gauges for the car at ecx (eax): how full
 * (|speed| times [0x53b6b8], [0x53b6bc] off [0x53b6c4], times the car's
 * +0x51c and [0x53b6cc]; wrapped into 0..1, reversed when [0x6fd4c4]), then
 * each of the [0x792310] bars at 0x792090 (36 bytes: its texture state's
 * record, the texture's height, s, t, x, y, width, height, upright) as a quad
 * of four vertices (colour sub_4206f0's) bent by sub_496bf0, scaled to the
 * cabin and placed in it (a port: cabinRect), clip-coded and drawn by
 * sub_4c12e0 between sub_4bed70's clip rectangles 0x7cdb88 and 0x7cdac0. */
void barGauges(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 car = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc8; sub ebp, 0x82
    const reg32 ebp = entry.esp - 24 - 0x82;
    Calls out(app, cpu, entry.esp - 24 - 0xc8);
    // the quad's four vertices (0x20 bytes each) on the frame, for sub_496bf0 and sub_4c12e0
    const reg32 vertices = ebp - 0x46;
    auto vertex = [&](reg32 c) { return vertices + (c << 5); };
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdba0], [0x7cdba4], [0x7cdb98], [0x7cdb9c]
    const reg32 cabinX = cabinRect(app, 0);
    const double scaleX = single(M::mul(double(i32(cabinRect(app, 2))), m.load(0x53b6b0)));
    const double scaleY = single(M::mul(double(i32(cabinRect(app, 3))), m.load(0x53b6b4)));
    const reg32 cabinY = cabinRect(app, 1);
    // the four vertices: z 2^-16, 1/w 1, the colour, clip code 0
    for (reg32 c = 0; c < 4; ++c)
    {
        if (!out.call(0x4206f0, {car + 0xc, 1}))
            return;
        m.word(vertex(c) + 0x10, out.result());
        m.word(vertex(c) + 0xc, 0x3f7fff00);
        m.word(vertex(c) + 0x14, 0);
        m.word(vertex(c) + 8, 0x37800080);
    }
    // how full: the speed's size
    const double speed = fcomBelow(0.0, m.load(car + 0x4f4)) ? m.load(car + 0x4f4) : single(-m.load(car + 0x4f4));
    const double off = M::sub(m.loadDouble(0x53b6c4), M::mul(M::mul(speed, m.load(0x53b6b8)), m.load(0x53b6bc)));
    double full = single(M::mul(M::mul(fild32(m, car + 0x51c), off), m.load(0x53b6cc)));
    if (fcomAbove(0.0, full))
        full = single(M::add(1.0, full));
    if (gameSettings(app).mirrored)
        full = single(M::sub(1.0, full));
    reg32 index = 0;
    for (; i32(index) < i32(m.word(0x792310)); ++index)
    {
        const reg32 bar = 0x792090 + index * 36;
        const reg32 left = m.word(bar + 0x10) - m.word(0x792300);
        const reg32 top = m.word(bar + 0x14) - m.word(0x79230c);
        /* The bar's rectangle, its width and height the other way round when it lies on its side;
         * the corners' x and y as floats. */
        const bool upright = m.word(bar + 0x20) != 0;
        const reg32 right = floatOf(m.word(bar + (upright ? 0x1c : 0x18)) + left);
        const reg32 near = floatOf(top);
        const reg32 far = floatOf(m.word(bar + (upright ? 0x18 : 0x1c)) + top);
        const reg32 xs[4] = {floatOf(left), right, right, floatOf(left)};
        const reg32 ys[4] = {upright ? far : near, upright ? far : near, upright ? near : far, upright ? near : far};
        // each corner bent by how full (sub_496bf0), then scaled and placed in the cabin
        for (reg32 c = 0; c < 4; ++c)
        {
            const reg32 corner = vertex(c);
            out.push(corner + 4);
            out.push(corner);
            out.push(asBits(float(full)));
            out.push(ys[c]);
            out.push(xs[c]);
            if (!out.call(0x496bf0))
                return;
            const double x = M::mul(M::add(fild32(m, 0x792304), m.load(corner)), scaleX);
            const double y = M::mul(M::add(fild32(m, 0x792308), m.load(corner + 4)), scaleY);
            m.store(corner, M::add(x, double(i32(cabinX))));
            m.store(corner + 4, M::add(y, double(i32(cabinY))));
        }
        // the texture's s and t at the corners (+0x18, +0x1c), over its height
        const double inverse = M::div(1.0, fild32(m, bar + 4));
        const double k0 = m.load(0x53b6d0);
        if (upright)
        {
            const double s1 = M::mul(M::add(double(i32(m.word(bar + 8) + m.word(bar + 0x18))), k0), inverse);
            const double s0 = M::mul(M::add(fild32(m, bar + 8), m.load(0x53b6d4)), inverse);
            const double t0 = M::mul(M::add(fild32(m, bar + 0xc), m.load(0x53b6d8)), inverse);
            m.store(vertex(1) + 0x18, s1);
            m.word(vertex(0) + 0x18, m.word(vertex(1) + 0x18));
            m.store(vertex(3) + 0x18, s0);
            m.word(vertex(2) + 0x18, m.word(vertex(3) + 0x18));
            m.store(vertex(3) + 0x1c, t0);
            m.word(vertex(0) + 0x1c, m.word(vertex(3) + 0x1c));
            m.store(vertex(2) + 0x1c,
                    M::mul(M::add(k0, double(i32(m.word(bar + 0xc) + m.word(bar + 0x1c)))), inverse));
            m.word(vertex(1) + 0x1c, m.word(vertex(2) + 0x1c));
        }
        else
        {
            m.store(vertex(3) + 0x18, M::mul(M::add(fild32(m, bar + 8), m.load(0x53b6d8)), inverse));
            m.word(vertex(0) + 0x18, m.word(vertex(3) + 0x18));
            const double t = fild32(m, bar + 0xc);
            const double s1 = M::mul(M::add(double(i32(m.word(bar + 8) + m.word(bar + 0x18))), k0), inverse);
            const double t0 = M::mul(M::add(t, m.load(0x53b6d4)), inverse);
            m.store(vertex(2) + 0x18, s1);
            m.store(vertex(1) + 0x1c, t0);
            m.word(vertex(1) + 0x18, m.word(vertex(2) + 0x18));
            m.word(vertex(0) + 0x1c, m.word(vertex(1) + 0x1c));
            m.store(vertex(3) + 0x1c,
                    M::mul(inverse, M::add(k0, double(i32(m.word(bar + 0xc) + m.word(bar + 0x1c))))));
            m.word(vertex(2) + 0x1c, m.word(vertex(3) + 0x1c));
        }
        // drawn under the bar's texture state between the cabin's clip rectangle and the screen's
        if (!out.call(0x431900, {1, m.word(m.word(bar) + 4)}) || !out.call(0x4bed70, {0x7cdb88}))
            return;
        for (reg32 c = 0; c < 4; ++c)
            m.byte(vertex(c) + 0x14, screenCodeOf(app, vertex(c)));
        if (!out.call(0x4c12e0, {vertex(0), vertex(1), vertex(2), vertex(3)}) || !out.call(0x4bed70, {0x7cdac0}))
            return;
    }
    cpu.eax = index;  // as the loop leaves it: a caller reads eax
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
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
void cockpit(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xc4; sub ebp, 0x82
    const reg32 ebp = entry.esp - 24 - 0x82;
    Calls out(app, cpu, entry.esp - 24 - 0xc4);
    // eax as the last callee leaves it (or as it came), which a caller reads
    auto leave = [&]() {
        entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
    };
    // The quad on the frame for THRASH_drawquad: its corners from the top right, clockwise.
    const reg32 corner[4] = {ebp - 0x42, ebp - 0x22, ebp - 2, ebp + 0x1e};
    auto drawQuad = [&](reg32 state) {
        if (!out.call(0x431900, {1, state}))
            return false;
        for (const reg32 c : {3, 2, 0, 1})
            out.push(corner[c]);
        return out.driver<thrash::kThrashDrawQuad, &port::driver::drawQuad>();
    };
    // its s and t at the corners (+0x18, +0x1c), from eight ints at `from`
    auto texture = [&](reg32 from) {
        static const unsigned order[4] = {1, 0, 2, 3};
        for (reg32 k = 0; k < 8; ++k)
            m.word(corner[order[k >> 1]] + 0x18 + 4 * (k & 1), m.word(from + 4 * k));
    };
    // x of the two left corners, x of the two right, y of the two top, y of the two bottom
    auto edges = [&](reg32 left, reg32 right, reg32 top, reg32 bottom) {
        m.word(corner[3], left);
        m.word(corner[1], left);
        m.word(corner[2], right);
        m.word(corner[0], right);
        m.word(corner[0] + 4, top);
        m.word(corner[1] + 4, top);
        m.word(corner[3] + 4, bottom);
        m.word(corner[2] + 4, bottom);
    };
    if (!m.word(0x7922fc) || m.word(0x7a3d10))
        return leave();
    if (!out.call(0x423b00, {0}))
        return;
    if (out.result() != 1 || m.word(0x79c4a0) || gameSettings(app).wideScreen)
        return leave();
    // port (apply_cabin): the cabin fitted to 4:3, was [0x7cdb98], [0x7cdb9c], [0x7cdba0], [0x7cdba4]
    const double tiles = m.loadDouble(0x53b76c);
    const double width = single(M::mul(M::mul(double(i32(cabinRect(app, 2))), tiles), m.loadDouble(0x53b774)));
    const double height = single(M::mul(M::mul(tiles, double(i32(cabinRect(app, 3)))), m.loadDouble(0x53b77c)));
    const reg32 left = cabinRect(app, 0), right = cabinRect(app, 2) + left;
    const reg32 top = cabinRect(app, 1), bottom = cabinRect(app, 3) + top;
    if (!out.call(0x422bd0, {0}))
        return;
    const reg32 view = out.result();
    if (!out.call(0x4206f0, {view + 0xc, 1}))
        return;
    if (gameSettings(app).night
        && (!out.call(0x4b97b0, {view, out.result()}) || !out.call(0x4b9550, {view, out.result()})))
        return;
    // the quad's corners: colour, z 2^-16, 1/w 1, +0x14 0
    const reg32 colour = out.result();
    for (const reg32 c : {2, 0, 1, 3})
        m.word(corner[c] + 0x10, colour);
    for (const reg32 c : {3, 2, 0, 1})
        m.word(corner[c] + 0xc, 0x3f7fff00);
    for (const reg32 c : {3, 2, 0, 1})
        m.word(corner[c] + 8, 0x37800080);
    for (const reg32 c : {3, 2, 0, 1})
        m.word(corner[c] + 0x14, 0);
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 3}))
        return;
    // the tiles, a column at a time, each cut at the cabin's right and bottom edges
    reg32 tile = 0;
    double x = single(double(i32(left)));
    while (!fcomBelowOrEqual(double(i32(right)), x))
    {
        double y = single(double(i32(top)));
        double nextX;
        for (;;)
        {
            nextX = single(M::add(x, width));
            if (fcomBelowOrEqual(double(i32(bottom)), y))
                break;
            const double rightEdge = single(double(i32(right)));
            const double toX = fcomBelow(nextX, rightEdge) ? nextX : rightEdge;
            const double nextY = single(M::add(y, height));
            const double bottomEdge = single(double(i32(bottom)));
            const double toY = fcomBelow(nextY, bottomEdge) ? nextY : bottomEdge;
            edges(asBits(float(x)), asBits(float(toX)), asBits(float(y)), asBits(float(toY)));
            texture(tile * 44 + 0x791d78);
            if (!drawQuad(m.word(tile * 44 + 0x791d74)))
                return;
            y = single(M::add(y, height));
            ++tile;
        }
        x = nextX;
    }
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 0}))
        return;
    if (!out.call(0x4880b0, {view}))
        return;
    if (m.word(0x55d6a4))
    {
        // the gear: the view's +0x50b picks the entry (44 bytes: its size, its texture state, s and t)
        const double across = M::mul(double(i32(cabinRect(app, 2))), m.load(0x53b784));
        const double down = M::mul(double(i32(cabinRect(app, 3))), m.load(0x53b788));
        const double gearX = M::mul(m.load(0x55d69c), across);
        const double gearY = M::mul(m.load(0x55d6a0), down);
        const double fromX = single(M::add(gearX, double(i32(left))));
        const reg32 gear = m.byte(view + 0x50b) * 44 + 0x791c10;
        const double toX = single(M::add(M::mul(across, double(app->getMemory<x86::reg16>(gear))), fromX));
        const double fromY = single(M::add(gearY, double(i32(top))));
        const double toY = single(M::add(M::mul(down, double(app->getMemory<x86::reg16>(gear + 2))), fromY));
        edges(asBits(float(fromX)), asBits(float(toX)), asBits(float(fromY)), asBits(float(toY)));
        texture(gear + 8);
        if (!drawQuad(m.word(gear + 4)))
            return;
    }
    if (out.call(0x488470, {view}))
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
void line(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 a = cpu.eax, b = cpu.edx;
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0xac; sub ebp, 0x82
    const reg32 ebp = entry.esp - 20 - 0x82;
    Calls out(app, cpu, entry.esp - 20 - 0xac);
    // eax a, or as THRASH_drawline leaves it: a caller reads eax
    auto leave = [&]() { entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEsi | Entry::kEdi | Entry::kEbp); };
    auto drawLine = [&](reg32 from, reg32 to) {
        out.push(to);
        out.push(from);
        if (out.driver<thrash::kThrashDrawLine, &port::driver::drawLine>())
            leave();
    };
    // 1 - 1/w as a float, or 0 when 1/w is negative or past 1 (as an int compare does it)
    auto nearness = [&](reg32 v) {
        if ((m.byte(v + 0xf) & 0x80) || i32(m.word(v + 0xc)) > 0x3f800000)
            return 0u;
        return asBits(float(M::sub(1.0, m.load(v + 0xc))));
    };
    const reg8 codeA = m.byte(a + 0x14), codeB = m.byte(b + 0x14);
    if (codeA & codeB)
        return leave();
    if (!(codeA | codeB))
    {
        // each depth through its 1/w
        for (const reg32 v : {a, b})
        {
            m.word(v + 8, nearness(v));
            m.store(v + 8, M::mul(m.load(0x560094), m.load(v + 8)));
            m.store(v + 8, M::add(m.load(0x560098), m.load(v + 8)));
        }
        // none drawn with the indefinite in a coordinate
        for (const reg32 v : {a, b})
            for (const reg32 field : {0u, 4u, 8u, 0xcu, 0x18u, 0x1cu})
                if ((m.word(v + field) | 0x80000000) == 0xffc00000)
                    return leave();
        return drawLine(a, b);
    }
    // the copies on the frame: the originals at ebp-0x2a, ebp-0xa, the clipped at ebp+0x16, ebp+0x36
    const reg32 original = ebp - 0x2a, clipped = ebp + 0x16;
    copyWords(app, original, a, 8);
    copyWords(app, original + 0x20, b, 8);
    for (reg32 k = 0; k < 2; ++k)
    {
        const reg32 v = original + (k << 5), other = original + (((k + 1) % 2) << 5), to = clipped + (k << 5);
        copyWords(app, to, v, 8);
        /* Where the line leaves through the plane `along` (0 x, 4 y, 0xc 1/w)
         * = `edge`: that, and the other two (x, y, 1/w) moved as far along. */
        auto cut = [&](reg32 along, reg32 first, reg32 second, reg32 edge) {
            const double t = M::div(M::sub(m.load(v + along), m.load(edge)), M::sub(m.load(v + along), m.load(other + along)));
            const double moveFirst = M::mul(M::sub(m.load(v + first), m.load(other + first)), t);
            const double moveSecond = M::mul(t, M::sub(m.load(v + second), m.load(other + second)));
            m.store(to + along, m.load(edge));
            m.store(to + first, M::sub(m.load(v + first), moveFirst));
            m.store(to + second, M::sub(m.load(v + second), moveSecond));
        };
        const reg8 code = m.byte(v + 0x14);
        if (code & 0x10)
            cut(0xc, 0, 4, 0x7d34f0);
        if (code & 1)
            cut(0, 4, 0xc, 0x7d34f4);
        if (code & 2)
            cut(0, 4, 0xc, 0x7d34ec);
        if (code & 8)
            cut(4, 0, 0xc, 0x7d34e0);
        if (code & 4)
            cut(4, 0, 0xc, 0x7d34dc);
    }
    // codes cleared, depths through 1/w
    m.word(clipped + 0x34, 0);
    m.word(clipped + 0x14, 0);
    const reg32 near0 = nearness(clipped);
    m.word(clipped + 8, near0);
    m.store(clipped + 8, M::add(M::mul(loaded(near0), m.load(0x560094)), m.load(0x560098)));
    const reg32 near1 = nearness(clipped + 0x20);
    m.word(clipped + 0x28, near1);
    m.store(clipped + 0x28, M::mul(loaded(near1), m.load(0x560094)));
    m.store(clipped + 0x28, M::add(m.load(clipped + 0x28), m.load(0x560098)));
    drawLine(clipped, clipped + 0x20);
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
void segmentMeter(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 x = cpu.edx, y = cpu.ebx, width = cpu.ecx;
    const reg32 height = m.word(entry.esp + 4), lit = m.word(entry.esp + 8), corner = m.word(entry.esp + 0xc);
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    Calls out(app, cpu, entry.esp - 12 - 0x1c);
    // THRASH_drawquadmesh(count, vertices, indices)
    auto mesh = [&](reg32 count, reg32 indices) {
        for (const reg32 v : {indices, 0x55d6e4u, count})
            out.push(v);
        return out.driver<thrash::kThrashDrawQuadMesh, &port::driver::drawQuadMesh>();
    };
    auto state = [&](reg32 a, reg32 d) { return out.call(0x431900, {a, d}); };
    // the frame's floats (each fstp dword there, read back by fld dword) and a float's bits copied
    double wide, high, x0, y0, gap, piece, step;
    reg32 word;
    // the frame's corners and edges
    const double f1 = double(i32(width));
    wide = single(f1);
    const double f2 = m.load(0x53b98c);
    const double f3 = M::mul(f1, f2);
    const double f4 = double(i32(height));
    high = single(f4);
    const double f5 = M::mul(f2, f4);
    const double f6 = high;
    const double f7 = M::mul(f6, m.load(0x53b990));
    m.word(0x55e014, corner);
    m.word(0x55dff4, corner);
    const double f8 = double(i32(y));
    const double f9 = double(i32(x));
    x0 = single(f9);
    y0 = single(f8);
    const double f10 = y0;
    const double f11 = x0;
    const double f12 = M::add(f11, wide);
    word = asBits(float(x0));
    m.store(0x55e024, f12);
    const double f13 = f12;
    m.word(0x55e044, word);
    m.word(0x55dfe4, word);
    m.store(0x55e004, f12);
    const double f14 = M::add(f10, high);
    const double f15 = M::add(f3, x0);
    m.store(0x55e124, f13);
    word = asBits(float(y0));
    const double f16 = m.load(0x55e124);
    m.store(0x55e048, f14);
    m.store(0x55e0a4, f15);
    const double f17 = m.load(0x55e048);
    const double f18 = m.load(0x55e0a4);
    const double f19 = f18;
    m.store(0x55e104, f16);
    m.word(0x55e008, word);
    m.word(0x55dfe8, word);
    m.store(0x55e028, f17);
    m.store(0x55e084, f18);
    m.store(0x55e144, f19);
    const double f20 = M::add(f5, y0);
    word = asBits(float(x0));
    m.store(0x55e0e4, f19);
    const double f21 = f20;
    m.word(0x55e0c4, word);
    m.word(0x55e064, word);
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
    m.word(0x55e0e8, m.word(0x55e108));
    m.store(0x55e128, f27);
    // the segments' left and right edges
    for (reg32 k = 0; k < 0x24; k += 4)
    {
        const reg32 at = k << 5;
        const double f28 = high;
        const double f29 = M::mul(f28, m.load(0x53b98c));
        const double f30 = wide;
        const double f31 = M::mul(f30, m.load(0x53b994));
        gap = single(f31);
        const double f32 = M::add(f29, y0);
        const double f33 = f32;
        const double f34 = M::sub(f33, gap);
        const double f35 = M::add(f32, gap);
        m.store(at + 0x55d708, f34);
        m.store(at + 0x55d748, f35);
        m.word(at + 0x55d6e8, m.word(at + 0x55d708));
        m.store(at + 0x55d728, f35);
    }
    // their tops and bottoms
    const double f36 = wide;
    const double f37 = M::mul(f36, m.load(0x53b98c));
    const double f38 = wide;
    const double f39 = M::mul(f38, m.load(0x53b99c));
    const double f40 = wide;
    const double f41 = M::mul(f40, m.load(0x53b9a0));
    const double f42 = x0;
    const double f43 = M::add(f42, gap);
    const double f44 = wide;
    const double f45 = M::mul(f44, m.load(0x53b9a4));
    m.store(0x55d744, f43);
    const double f46 = wide;
    const double f47 = M::mul(f46, m.load(0x53b998));
    m.word(0x55d6e4, m.word(0x55d744));
    const double f48 = x0;
    const double f49 = M::add(f48, wide);
    const double f50 = M::sub(f49, gap);
    const double f51 = M::add(f37, x0);
    piece = single(f50);
    step = single(f47);
    const double f52 = M::add(f43, step);
    const double f53 = piece;
    word = asBits(float(piece));
    const double f54 = f51;
    m.word(0x55d7a4, word);
    m.word(0x55d784, word);
    const double f55 = M::sub(f54, gap);
    const double f56 = M::add(f51, gap);
    m.store(0x55d844, f55);
    m.store(0x55d824, f56);
    m.store(0x55d724, f52);
    const double f57 = m.load(0x55d844);
    const double f58 = M::sub(f53, step);
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
    piece = single(f39);
    const double f66 = M::sub(f62, piece);
    const double f67 = M::sub(f63, piece);
    const double f68 = M::add(f64, piece);
    const double f69 = M::add(f65, piece);
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
    for (reg32 k = 0; k < 0x24; k += 4)
    {
        const reg32 at = k << 5;
        const double f98 = wide;
        const double f99 = M::mul(f98, m.load(0x53b9a8));
        const double f100 = m.load(at + 0x55d6e4);
        const double f101 = m.load(at + 0x55d724);
        const double f102 = m.load(at + 0x55d6e8);
        const double f103 = m.load(at + 0x55d728);
        const double f104 = M::sub(f100, f99);
        const double f105 = M::add(f101, f99);
        const double f106 = M::sub(f102, f99);
        const double f107 = M::add(f99, f103);
        m.store(at + 0x55dbc4, f104);
        m.store(at + 0x55dba4, f105);
        m.store(at + 0x55db88, f106);
        m.store(at + 0x55dbc8, f107);
        const double f108 = m.load(at + 0x55dbc4);
        const double f109 = m.load(at + 0x55dba4);
        const double f110 = m.load(at + 0x55db88);
        const double f111 = m.load(at + 0x55dbc8);
        m.store(at + 0x55db64, f108);
        m.store(at + 0x55db84, f109);
        m.store(at + 0x55db68, f110);
        m.store(at + 0x55dba8, f111);
    }
    // the colours: the back and the frame's
    for (reg32 k = 0; k < 0x24; ++k)
    {
        m.word((k << 5) + 0x55d6f4, 0xbf00ff00);
        m.word((k << 5) + 0x55db74, 0x9f307f30);
    }
    // the lit segments: the frame's edges beside each red, its own corners pink
    if (i32(lit) > 0)
    {
        struct Lit
        {
            reg32 frame[8];
            reg32 own[8];
        };
        static const Lit segments[4] = {
            { { 0x55d834, 0x55d814, 0x55d7f4, 0x55d854 }, { 0x55dcd4, 0x55dcb4, 0x55dc94, 0x55dc74 } },
            { { 0x55d8b4, 0x55d894, 0x55d874, 0x55d954, 0x55d934, 0x55d914, 0x55d8f4, 0x55d8d4 },
              { 0x55dd54, 0x55dd34, 0x55dd14, 0x55dcf4, 0x55ddd4, 0x55ddb4, 0x55dd94, 0x55dd74 } },
            { { 0x55d9b4, 0x55d994, 0x55d974, 0x55da54, 0x55da34, 0x55da14, 0x55d9f4, 0x55d9d4 },
              { 0x55de54, 0x55de34, 0x55de14, 0x55ddf4, 0x55ded4, 0x55deb4, 0x55de94, 0x55de74 } },
            { { 0x55dab4, 0x55da94, 0x55da74, 0x55db54, 0x55db34, 0x55db14, 0x55daf4, 0x55dad4 },
              { 0x55df54, 0x55df34, 0x55df14, 0x55def4, 0x55dfd4, 0x55dfb4, 0x55df94, 0x55df74 } },
        };
        for (reg32 k = 0; k < 4; ++k)
        {
            if (k && i32(lit) <= i32(k))
                break;
            for (const reg32 at : segments[k].frame)
                if (at)
                    m.word(at, 0xffff0000);
            for (const reg32 at : segments[k].own)
                if (at)
                    m.word(at, 0xccff5050);
        }
    }
    if (!state(1, 0) || !mesh(3, 0x55e284))
        return;
    if (!state(1, m.word(0x76da48)) || !mesh(9, 0x55e164))
        return;
    if (i32(lit) > -1)
    {
        if (!state(0x68, 1) || !state(1, m.word(0x8b4844)))
            return;
        if (m.byte(0x7a3a58) & 2)
        {
            if (lit && (!out.call(0x432390, {0xccff5050}) || !mesh(lit + lit - 1, 0x55e214)))
                return;
        }
        else if (!mesh(9, 0x55e1f4))
            return;
        if (!state(0x68, 0))
            return;
    }
    // eax as the last callee leaves it: a caller reads eax
    entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp, 12);
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
void hudBand(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 player = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0xf0; sub ebp, 0x82
    const reg32 ebp = entry.esp - 24 - 0x82;
    Calls out(app, cpu, entry.esp - 24 - 0xf0);
    // eax as the code leaves it, which a caller reads: sub_4808b0's, or the last quad's address
    auto leave = [&]() {
        entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
    };
    // no band: sub_4808b0 for the player
    auto none = [&]() {
        if (out.call(0x4808b0, {player}))
            leave();
    };
    auto lt = [](reg32 a, reg32 b) { return i32(a) < i32(b); };
    auto le = [](reg32 a, reg32 b) { return i32(a) <= i32(b); };
    /* The cuts: x0, x1, y0, y1 of rectangle k, the code's four arrays of seven on the frame, one
     * after the other (a count past seven runs on into the next, as the code's does). */
    auto X0 = [&](reg32 k) { return ebp - 0x6e + 4 * k; };
    auto X1 = [&](reg32 k) { return ebp - 0x36 + 4 * k; };
    auto Y0 = [&](reg32 k) { return ebp - 0x52 + 4 * k; };
    auto Y1 = [&](reg32 k) { return ebp - 0x1a + 4 * k; };
    const reg32 quads = m.word(0x791ac0) + player * 0x380;
    const reg32 row = player + (gameSettings(app).playMode == 1 ? 1u : 0u);
    if (!out.call(0x422bd0, {player}))
        return;
    const reg32 view = out.result();
    const reg32 cop = (m.byte(view + 0x200) & 0x20) ? 1 : 0;
    const reg32 rects = player * 0x170 + kHudRects;
    const reg32 three = rects + 0x30, one = rects + 0x10;  // elements 3 and 1
    if (!(m.byte(view + 0x200) & 4))
        return none();
    // the band's top: the screen's or its lower half's
    reg32 top = 0;
    if (row == 2)
    {
        const reg32 height = m.word(0x7cdacc);
        top = reg32(i32(height - reg32(i32(height) >> 31)) >> 1);
    }
    // its bottom: past every element reaching into it
    reg32 bottom = top;
    for (reg32 element = 0; lt(element, 0x17); ++element)
    {
        if (!out.call(0x47bc20, {element}))
            return;
        if (!out.result())
            continue;
        // 0..9, and the racer's 10..17 or the cop's 18..22
        if (!(lt(element, 10) || (!cop && lt(element, 0x12)) || (cop && !lt(element, 0x12))))
            continue;
        const reg32 reach = (element << 4) + player * 0x170 + 0x749764;
        if (le(m.word(reach), bottom))
            continue;
        const reg32 i = hudBorder(app, cpu, 0x53b500, 0x53b504);
        if (le(m.word(reach), top + m.word(0x7cdb64) + (i + i)))
            bottom = m.word(reach);
    }
    if (bottom == top)
        return none();
    // element 3 in the band (its layout kind 2 or 3, or [0x559264])
    const reg32 layout = row * kHudRowBytes + cop * kHudLayoutBytes;
    const reg32 form3 = gameSettings(app).hudAt(layout).tachometerForm;
    const bool threeIn = (form3 == 2 || form3 == 3 || m.word(0x559264)) && lt(top, m.word(three + 0xc))
                         && !le(bottom, m.word(three + 4));
    // element 1 in the band (kind 4 or 2, or [0x559264])
    const reg32 form1 = gameSettings(app).hudAt(layout).speedometerForm;
    const bool oneIn = (form1 == 4 || form1 == 2 || m.word(0x559264)) && !le(m.word(one + 0xc), top)
                       && lt(m.word(one + 4), bottom);
    // the mirror in the band (the top view, its layout's rear view or [0x559264], sub_422aa0)
    bool mirrorIn = false;
    if (row == 0 && (gameSettings(app).hudAt(cop * kHudLayoutBytes).rearView || m.word(0x559264)))
    {
        if (!out.call(0x422aa0, {0}))
            return;
        if (out.result())
        {
            const reg32 mirror = m.word(0x791a90);
            if (fcomBelow(double(i32(top)), m.load(mirror + 0x244)))
                mirrorIn = fcomAbove(double(i32(bottom)), m.load(mirror + 4));
        }
    }
    // which of them overlap across: 3 and 1, 3 and the mirror, 1 and the mirror
    auto across = [&](reg32 a0, reg32 a1, reg32 b0, reg32 b1) {
        if (!le(a0, b0) && lt(a0, b1))
            return true;
        return !le(a1, b0) && lt(a1, b1);
    };
    // against the mirror's floats: the left edge inside (as a float), then the right
    auto acrossMirror = [&](reg32 element) {
        const double left = single(double(i32(m.word(element))));
        reg32 mirror = m.word(0x791a90);
        if (fcomAbove(left, m.load(mirror)) && fcomBelow(left, m.load(mirror + 0x140)))
            return true;
        const double edge = double(i32(m.word(element + 8)));
        mirror = m.word(0x791a90);
        return fcomAbove(edge, m.load(mirror)) && fcomBelow(single(edge), m.load(mirror + 0x140));
    };
    const bool threeOne = across(m.word(three), m.word(three + 8), m.word(one), m.word(one + 8));
    const bool threeMirror = acrossMirror(three);
    const bool oneMirror = acrossMirror(one);
    if ((threeIn && oneIn && threeOne) || (threeIn && mirrorIn && threeMirror) || (oneIn && mirrorIn && oneMirror))
        return none();
    // the band whole, then cut around element 3
    reg32 count = 1;
    m.word(X0(0), 0);
    m.word(X1(0), m.word(0x7cdac8));
    m.word(Y0(0), top);
    m.word(Y1(0), bottom);
    if (threeIn)
    {
        const reg32 right = m.word(X1(0));
        if (!le(m.word(three), 0) && !le(right, m.word(three + 8)))
        {
            // across the band's middle: left and right of it
            m.word(X1(1), right);
            m.word(X1(0), m.word(three));
            m.word(X0(1), m.word(three + 8));
            m.word(Y0(1), top);
            m.word(Y1(1), bottom);
            count = 2;
        }
        else if (m.word(0x7cdac8) == m.word(three + 8))
            m.word(X1(0), m.word(three));
        else if (!m.word(three))
            m.word(X0(0), m.word(three + 8));
        // below or above it, the rest of the band beside it
        if (!le(bottom, m.word(three + 4)))
        {
            const reg32 k = count;
            if (!le(m.word(three + 4), top))
            {
                m.word(X0(k), m.word(three));
                m.word(X1(k), m.word(three + 8));
                m.word(Y0(k), top);
                ++count;
                m.word(Y1(k), m.word(three + 4));
            }
            else if (lt(m.word(three + 0xc), bottom))
            {
                m.word(X0(k), m.word(three));
                m.word(X1(k), m.word(three + 8));
                m.word(Y0(k), m.word(three + 0xc));
                ++count;
                m.word(Y1(k), bottom);
            }
        }
    }
    /* Cut every rectangle so far around [x0, x1] (element 1's or the mirror's), then the band
     * below or above it: a rectangle it lies inside split in two, one it covers an end of cut
     * short there. */
    auto cutAround = [&](reg32 x0, reg32 x1) {
        for (reg32 j = 0; lt(j, count); ++j)
        {
            if (!le(x0, m.word(X0(j))) && lt(x1, m.word(X1(j))))
            {
                const reg32 k = count;
                m.word(X1(k), m.word(X1(j)));
                m.word(X0(k), x1);
                m.word(Y0(k), top);
                m.word(Y1(k), bottom);
                ++count;
                m.word(X1(j), x0);
                continue;
            }
            if (!le(x0, m.word(X0(j))) && lt(x0, m.word(X1(j))) && le(m.word(X1(j)), x1))
            {
                m.word(X1(j), x0);
                continue;
            }
            if (!le(x1, m.word(X0(j))) && !(!lt(x1, m.word(X1(j))) || !le(x0, m.word(X0(j)))))
                m.word(X0(j), x1);
        }
    };
    if (oneIn)
    {
        // element 1's edges read afresh at each use: a cut may lie over them
        for (reg32 j = 0; lt(j, count); ++j)
        {
            if (!le(m.word(one), m.word(X0(j))) && lt(m.word(one + 8), m.word(X1(j))))
            {
                const reg32 k = count;
                m.word(X1(k), m.word(X1(j)));
                m.word(X0(k), m.word(one + 8));
                m.word(Y0(k), top);
                m.word(Y1(k), bottom);
                ++count;
                m.word(X1(j), m.word(one));
                continue;
            }
            const reg32 x0 = m.word(one);
            if (!le(x0, m.word(X0(j))) && lt(x0, m.word(X1(j))) && le(m.word(X1(j)), m.word(one + 8)))
            {
                m.word(X1(j), m.word(one));
                continue;
            }
            const reg32 x1 = m.word(one + 8);
            if (!le(x1, m.word(X0(j))) && lt(x1, m.word(X1(j))) && le(m.word(one), m.word(X0(j))))
                m.word(X0(j), m.word(one + 8));
        }
        const reg32 k = count;
        if (lt(m.word(one + 4), bottom))
        {
            if (!le(m.word(one + 4), top))
            {
                m.word(X0(k), m.word(one));
                m.word(X1(k), m.word(one + 8));
                m.word(Y0(k), top);
                const reg32 y1 = m.word(one + 4);
                ++count;
                m.word(Y1(k), y1);
            }
            else if (!le(bottom, m.word(one + 0xc)))
            {
                m.word(X0(k), m.word(one));
                m.word(X1(k), m.word(one + 8));
                const reg32 y0 = m.word(one + 0xc);
                ++count;
                m.word(Y0(k), y0);
                m.word(Y1(k), bottom);
            }
        }
    }
    if (mirrorIn)
    {
        // the mirror's pixel rectangle: x0, x1, y0, y1 through __CHP and fistp
        // (all four loaded first, the code's fxch's choosing which is chopped and stored next)
        const reg32 mirror = m.word(0x791a90);
        const double corners[4] = {m.load(mirror), m.load(mirror + 0x140), m.load(mirror + 4), m.load(mirror + 0x244)};
        reg32 whole[4];
        for (int i = 0; i < 4; ++i)
        {
            const std::optional<x86::Float> chop = chopped(out, x86::Float(corners[i]));
            if (!chop)
                return;
            whole[i] = reg32(fp::toInteger<x86::sreg32>(double(*chop)));
        }
        cutAround(whole[0], whole[1]);
        const reg32 k = count;
        if (lt(whole[2], bottom))
        {
            if (!le(whole[2], top))
            {
                m.word(X0(k), whole[0]);
                ++count;
                m.word(X1(k), whole[1]);
                m.word(Y0(k), top);
                m.word(Y1(k), whole[2]);
            }
            else if (!le(bottom, whole[3]))
            {
                m.word(X0(k), whole[0]);
                ++count;
                m.word(X1(k), whole[1]);
                m.word(Y1(k), bottom);
                m.word(Y0(k), whole[3]);
            }
        }
    }
    // a new count: the quads rebuilt
    if (count != m.word(player * 4 + 0x700240))
    {
        if (!out.call(0x4808b0, {player}))
            return;
        m.word(player * 4 + 0x700240, count);
        reg32 at = m.word(0x791a88) + player * 28;
        for (reg32 n = count; n--;)
        {
            out.push(at + 3);
            if (!out.call(0x49bd30, {0x749a38, at, at + 1, at + 2}))
                return;
            at += 4;
        }
    }
    // the quads: corners from the cuts, shaded down the band
    std::optional<reg32> last;
    for (reg32 k = 0; lt(k, m.word(player * 4 + 0x700240)); ++k)
    {
        const reg32 q = (k << 7) + quads;
        const double y0 = fild32(m, Y0(k));
        m.store(q + 0x24, y0);
        m.store(q + 4, y0);
        const double y1 = fild32(m, Y1(k));
        m.store(q + 0x64, y1);
        m.store(q + 0x44, y1);
        const double x0 = fild32(m, X0(k));
        m.store(q + 0x60, x0);
        m.store(q, x0);
        const double x1 = fild32(m, X1(k));
        m.word(q + 0x78, 0x3f7b0000);
        m.word(q + 0x58, 0x3f7b8000);
        m.store(q + 0x40, x1);
        const double s0 = m.load(q + 0x78);
        const double s1 = m.load(q + 0x58);
        m.store(q + 0x20, x1);
        m.store(q + 0x18, s0);
        m.store(q + 0x38, s1);
        if (top == m.word(Y0(k)) && m.word(Y1(k)) == bottom)
        {
            m.word(q + 0x3c, 0x3f440000);
            m.word(q + 0x7c, 0x3f630000);
            const double t1 = m.load(q + 0x7c);
            m.word(q + 0x1c, m.word(q + 0x3c));
            m.store(q + 0x5c, t1);
        }
        else
        {
            const double bandBottom = double(i32(bottom));
            const double span = M::sub(bandBottom, double(i32(top)));
            const double from = fild32(m, Y0(k));
            const double step = M::div(m.load(0x53b508), span);
            const double down = M::mul(from, step);
            const double past = M::mul(bandBottom, step);
            const double base = M::sub(m.load(0x53b50c), past);
            const double t0 = M::add(down, base);
            m.store(q + 0x3c, t0);
            m.store(q + 0x1c, t0);
            const double t1 = M::add(base, M::mul(step, fild32(m, Y1(k))));
            m.store(q + 0x7c, t1);
            m.store(q + 0x5c, t1);
        }
        last = q;
    }
    // (the loop runs at least once: the count is one or more)
    cpu.eax = last ? *last : out.result();
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
void hud(win32::WinApplication* app, x86::CPU& cpu)
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
        const std::optional<double> whole = chopValue(app, cpu, flags, r, ebp, esp, value);
        if (!whole)
            return false;
        m.word(ebp - 0x40, x86::reg32(fp::toInteger<x86::sreg32>(*whole)));
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
    r.edx = gameSettings(app).playMode;
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
        r.eax = gameSettings(app).pursuitOutCount - 1;
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
    flags.compare(gameSettings(app).raceType, 3);
    if (gameSettings(app).raceType == 3)
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
                flags.compare(gameSettings(app).replayMode, 2);
                if (gameSettings(app).replayMode == 2)
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
        flags.compare(gameSettings(app).playMode, 1);
        if (gameSettings(app).playMode == 1)
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
        flags.compare(gameSettings(app).hudAt(r.edx + r.eax).shown, 0);
        if (gameSettings(app).hudAt(r.edx + r.eax).shown)
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
                        flags.compare(gameSettings(app).replayMode, 2);
                        compact = gameSettings(app).replayMode != 2;
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
        r.ebx = gameSettings(app).hudAt(r.edx + r.eax).opponentList;
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
        flags.compare(gameSettings(app).hudAt(r.edx + r.eax).mapStyle, 0);
        if (gameSettings(app).hudAt(r.edx + r.eax).mapStyle && !map())
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
    flags.compare(gameSettings(app).hudAt(r.edx + r.eax).shown, 0);
    if (!gameSettings(app).hudAt(r.edx + r.eax).shown)
        return finish();
    flags.compare(m.word(0x791b1c), 0);
    if (!m.word(0x791b1c))
    {
        flags.compare(gameSettings(app).replayMode, 2);
        if (gameSettings(app).replayMode == 2)
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
            flags.compare(r.eax, gameSettings(app).entryCount);
            if (!lt(r.eax, gameSettings(app).entryCount))
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
    r.ebx = gameSettings(app).hudAt(r.eax).splitTimeForm;
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
            flags.compare(gameSettings(app).hudAt(r.edx + r.eax).splitTimeForm, 3);
            if (gameSettings(app).hudAt(r.edx + r.eax).splitTimeForm == 3)
            {
                r.edx = gameSettings(app).recordsHolder;
                flags.compare(r.edx, m.word(0x72522c));
                blink = !lt(r.edx, m.word(0x72522c));
            }
            if (!blink)
            {
                r.eax = m.word(ebp - 4) * 0x1a4;
                r.edx = m.word(ebp - 0xc) * 0x348;
                flags.compare(gameSettings(app).hudAt(r.edx + r.eax).splitTimeForm, 2);
                if (gameSettings(app).hudAt(r.edx + r.eax).splitTimeForm == 2)
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
    r.eax = gameSettings(app).laps;
    flags.compare(r.ecx, r.eax);
    if (!le(r.ecx, r.eax))
        r.ecx = r.eax;
    r.eax = m.word(0x7d3684) - m.word(r.edi + 0x238);
    flags.compare(r.eax, 0x140);
    bool flash = false;
    if (lt(r.eax, 0x140))
    {
        flags.compare(r.ecx, gameSettings(app).laps);
        flash = r.ecx == gameSettings(app).laps;
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
    if (!textScalePushed(app, cpu, flags, r, ebp, esp))
        return;
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
        r.edx = gameSettings(app).pursuitWinCount;
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
    flags.compare(gameSettings(app).raceOptionCones, 0);
    if (gameSettings(app).raceOptionCones)
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
        r.ecx = gameSettings(app).laps;
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
        r.edx = gameSettings(app).laps;
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
            r.ebx = gameSettings(app).language;
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
            const bool small = gameSettings(app).language != 0;
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
    flags.compare(gameSettings(app).raceType, 3);
    if (gameSettings(app).raceType == 3)
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
    flags.compare(gameSettings(app).raceType, 3);
    if (gameSettings(app).raceType == 3)
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
            r.edx = gameSettings(app).pursuitOutCount;
            push(m.word(0x791b3c));
            if (!callTo(0x489a50))
                return;
        }
    }
    // the tables (layout +0x6fbc58)
    r.ebx = m.word(ebp - 4);
    r.eax = r.ebx * 0x1a4;
    r.edx = m.word(ebp - 0xc) * 0x348;
    flags.compare(gameSettings(app).hudAt(r.edx + r.eax).opponentList, 0);
    if (gameSettings(app).hudAt(r.edx + r.eax).opponentList)
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
    flags.compare(gameSettings(app).hudAt(r.edx + r.eax).mapStyle, 0);
    if (gameSettings(app).hudAt(r.edx + r.eax).mapStyle && !map())
        return;
    // the rev counter by its kind (layout +0x6fbc6c): 1 the digital one, else the dial
    r.eax = m.word(ebp - 4) * 0x1a4;
    r.edx = m.word(ebp - 0xc) * 0x348;
    r.eax += r.edx;
    r.ebx = gameSettings(app).hudAt(r.eax).tachometerForm;
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
            // both through sub_4dfd56: y first, then x (both loaded first)
            const double yValue = m.load(ebp - 0x20), xValue = m.load(ebp - 0x24);
            const std::optional<double> yWhole = chopValue(app, cpu, flags, r, ebp, esp, yValue);
            if (!yWhole)
                return;
            m.word(ebp - 0x40, x86::reg32(fp::toInteger<x86::sreg32>(*yWhole)));
            if (redline)
                r.ebx = m.word(ebp - 0x40);
            else
                r.edx = m.word(ebp - 0x40);
            const std::optional<double> xWhole = chopValue(app, cpu, flags, r, ebp, esp, xValue);
            if (!xWhole)
                return;
            if (redline)
            {
                push(r.ebx);
                m.word(ebp - 0x40, x86::reg32(fp::toInteger<x86::sreg32>(*xWhole)));
                r.eax = m.word(ebp - 0x40);
                push(r.eax);
                push(0xffa01010);
                push(m.word(0x791b54));
                r.eax = r.ecx;
            }
            else
            {
                push(r.edx);
                m.word(ebp - 0x40, x86::reg32(fp::toInteger<x86::sreg32>(*xWhole)));
                r.ecx = m.word(ebp - 0x40);
                push(r.ecx);
                r.ebx = m.word(ebp - 8);
                push(r.ebx);
                push(m.word(0x791b54));
                r.eax = m.word(r.edi + 0x590);
            }
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
    r.ebx = gameSettings(app).hudAt(r.eax).speedometerForm;
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
            r.eax = gameSettings(app).playMode;
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
            flags.compare(gameSettings(app).hudAt(r.edx + r.eax).speedometerForm, 1);
            bool big = gameSettings(app).hudAt(r.edx + r.eax).speedometerForm == 1;
            if (!big)
            {
                r.eax = gameSettings(app).playMode;
                split();
                r.eax = r.eax * 0x348;
                r.edx = m.word(ebp - 4) * 0x1a4;
                flags.compare(gameSettings(app).hudAt(r.edx + r.eax).speedometerForm, 2);
                big = gameSettings(app).hudAt(r.edx + r.eax).speedometerForm == 2;
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
    flags.compare(gameSettings(app).entryAt(r.edx + r.eax * 8).transmission, 0);
    if (!gameSettings(app).entryAt(r.edx + r.eax * 8).transmission)
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
    r.ecx = gameSettings(app).hudAt(r.eax).speedometerForm;
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
        r.eax = gameSettings(app).hudAt(r.ecx + r.eax).splitTimeForm;
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
                    flags.compare(gameSettings(app).recordsHolder, 0);
                    if (gameSettings(app).recordsHolder)
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
                            r.eax = gameSettings(app).recordsHolder;
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
                            r.eax = gameSettings(app).recordsHolder;
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
        flags.compare(gameSettings(app).hudAt(r.edx + r.eax).opponentName, 0);
        if (gameSettings(app).hudAt(r.edx + r.eax).opponentName)
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
            flags.compare(gameSettings(app).hudAt(4 * 0x1a4 + r.eax).shown, 0);
            layersOnly = gameSettings(app).hudAt(4 * 0x1a4 + r.eax).shown != 0;
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
void carTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 car = cpu.eax, shape = cpu.edx;
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x108
    const reg32 frame = entry.esp - 20;
    Calls out(app, cpu, frame - 0x108);
    auto leave = [&]() { entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEsi | Entry::kEdi | Entry::kEbp); };
    const reg32 name = frame - 0x108;  // the model's name, on the frame for the loaders
    auto isPlayer = [&]() { return car == m.word(gameSettings(app).localPlayer * 4 + 0x5efa48); };
    // port (apply_car_detail): every car at the player's texture size
    const reg32 size = isPlayer() ? 256u : (fullCarDetail() ? 256u : 128u);
    if (!out.call(0x439300, {gameSettings(app).entryAt(m.word(car + 0x1f0) * 108).car}))
        return;
    // the model's name, as the inline strcpy copies it
    for (reg32 k = 0;; ++k)
    {
        const reg8 c = m.byte(out.result() + k);
        m.byte(name + k, c);
        if (!c)
            break;
    }
    // a pursuit car with its own palette
    const i32 model = i32(m.word(m.word(car + 0x220) + 0x14));
    if (model >= 0 && model <= 0x31 && !(m.byte(car + 0x200) & 0x20))
    {
        out.push(shape);
        if (!out.call(0x4b7a60, {car, name, size, SETTINGS_ADDRESS(entries) + m.word(car + 0x1f0) * 108 + 0x20}))
            return;
    }
    else
    {
        // another car of the same model: its textures shared
        for (reg32 k = 0; i32(k) < i32(m.word(0x5efd9c)); ++k)
        {
            const reg32 other = m.word(k * 4 + 0x5efac8);
            if (m.word(other + 0x8a4)
                && m.word(m.word(other + 0x220) + 0x14) == m.word(m.word(car + 0x220) + 0x14))
            {
                if (!out.call(0x4b7ba0, {other, name, shape}))
                    return;
                m.word(car + 0x8a4, out.result());
                return leave();
            }
        }
        out.push(shape);
        if (!out.call(0x4b7a60, {car, name, size, 0}))
            return;
    }
    m.word(car + 0x8a4, out.result());
    if (!isPlayer() || m.word(0x55d000) != 1)
        return leave();
    if (!out.call(0x4b7ba0, {car, name, shape}))
        return;
    m.word(0x55fe24, out.result());
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
void loadingScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    // port (apply_loading_screen): the loading screen at 4:3
    loadingScreenFit(app, cpu, true);
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 picture = cpu.eax;
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x184; sub ebp, 0x82
    const reg32 ebp = entry.esp - 24 - 0x82;
    Calls out(app, cpu, entry.esp - 24 - 0x184);
    // sub esp, 8; fstp qword [esp]
    auto pushDouble = [&](double value) {
        out.esp -= 8;
        app->getMemory<double>(out.esp) = value;
    };
    // fstp dword of what a callee returned on st(0), read back by fld dword
    auto popped = [&]() { return single(double(cpu.fpu.take())); };
    // a value through __CHP and fistp; nullopt when the game stopped
    auto truncate = [&](double value) -> std::optional<reg32> {
        const std::optional<x86::Float> whole = chopped(out, x86::Float(value));
        if (!whole)
            return std::nullopt;
        return reg32(fp::toInteger<x86::sreg32>(double(*whole)));
    };
    /* The calls hand on what the code has in eax, edx, ebx and ecx, stale values too: sub_4ef190
     * and sub_4f09c0 pass them on to the functions they call, sub_4ef1b0 pushes eax, and the
     * check compares those of sub_4d4080, sub_4d79b0 and sub_4ead90. */
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 3}))
        return;
    // the window's rectangle (sub_4beb20 of 0x7cdac0) to the frame
    const reg32 rect = ebp + 0x42;
    m.word(0x79f288, 1);
    out.push(rect + 0xc);
    if (!out.call(0x4beb20, {0x7cdac0, rect, rect + 4, rect + 8}))
        return;
    // the picture's scale to the window
    const double across = single(M::mul(fild32(m, rect + 8), m.load(0x53bddc)));
    const double down = single(M::mul(fild32(m, rect + 0xc), m.load(0x53bde0)));
    // the picture into a buffer of 32x32 tiles, 20 rows of 15
    const reg32 rows = 0x14, columns = 0xf;
    if (!out.call(0x431bc0, {m.word(rect + 0xc), 0x20, 0x10, 0x10}) || !out.call(0x4ef190, {0x20, 0x20, 0x10, 0x10}))
        return;
    const reg32 tiles = out.result();
    if (!out.call(0x4eada0, {tiles, 0x20, columns, rows}) || !out.call(0x4ef760, {0xff000000, 0x1e0, columns, rows})
        || !out.call(0x4ef1b0, {out.result(), 0x1e0, columns, rows})
        || !out.call(0x4ef190, {0x280, 0x1e0, 0x10, 0x10}))
        return;
    const reg32 screen = out.result();
    if (!out.call(0x4eada0, {screen, 0x1e0, 0x10, 0x10})
        || !out.call(0x4ef1b0, {m.byte(0x565010) == 8 ? 0xffu : 0u, 0x1e0, 0x10, 0x10})
        || !out.call(0x4f09c0, {picture, 0, 0, 0x10}))
        return;
    reg32 edx = 0;  // as the code leaves it for sub_4ead90
    for (reg32 row = 0; i32(row) < i32(rows); ++row)
        for (reg32 column = 0; i32(column) < i32(columns); ++column)
        {
            const reg32 texture = ebp - 2;  // sub_4f2cb0's on the frame
            if (!out.call(0x4f2cb0, {m.word(tiles + 0x20), row << 5, column << 5, texture}))
                return;
            out.push(0);
            if (!out.call(0x4d4080, {0, m.word(tiles + 0x20), 0x20, texture}))
                return;
            // the tile's corners: floor of its top left, ceil of its bottom right
            const double D = m.loadDouble(0x53bde8);
            pushDouble(M::mul(M::mul(double(i32(row)), D), across));
            if (!out.call(0x4f1c30))
                return;
            const double x = M::mul(double(i32(column)), D);
            pushDouble(M::mul(x, down));
            const double floorY = popped();
            if (!out.call(0x4f1c30))
                return;
            pushDouble(M::mul(across, D));
            const double floorX = popped();
            if (!out.call(0x4f2d78))
                return;
            pushDouble(M::mul(down, D));
            const double ceilY = popped();
            if (!out.call(0x4f2d78))
                return;
            // the last ceil as it returned it, and the three stored before, each through __CHP in
            // the order the code's fxch's take them, then stored by fistp
            const double ceilX = double(cpu.fpu.take());
            out.push(0xffffffff);
            const reg32 eax = m.word(ebp + 2);
            const double corners[4] = {ceilY, floorX, floorY, ceilX};
            reg32 whole[4];
            for (int i = 0; i < 4; ++i)
            {
                const std::optional<reg32> w = truncate(corners[i]);
                if (!w)
                    return;
                whole[i] = *w;
            }
            out.push(whole[3]);
            edx = whole[2];
            if (!out.call(0x4d79b0, {eax, edx, whole[1], whole[0]}))
                return;
        }
    if (!out.call(0x4eff00, {tiles}) || !out.call(0x4ead90, {out.result(), edx}) || !out.call(0x4eff00, {screen}))
        return;
    // the text: its size, "loading" at its place
    if (!setTextScale(out, m.word(0x791b54)))
        return;
    m.word(0x561b34, 0xffffffff);
    m.word(0x561b3c, 0);
    if (!out.call(0x4d1850, {0x18c}))
        return;
    const reg32 text = ebp - 0x102;
    out.push(out.result());
    out.push(0x53bdd8);
    out.push(text);
    if (!out.call(0x4df690))
        return;
    const double textY = M::mul(down, m.load(0x53bdf0));
    const double textX = M::mul(across, m.load(0x53bdf4));
    out.esp += 0xc;
    const std::optional<reg32> y = truncate(textY);
    if (!y)
        return;
    const std::optional<reg32> x = truncate(textX);
    if (!x || !out.call(0x4d1000, {text, *x, *y}))
        return;
    // the progress bar: from the text's right edge, over [0x53bdfc] of the width
    const double scale = M::div(m.load(0x53bdfc), fild32(m, 0x7cdac8));
    m.store(0x79f284, double(cpu.fpu.take()));
    m.store(0x79f280, M::add(M::add(single(textX), m.load(0x79f284)), m.load(0x53bdf8)));
    m.store(0x79f280, M::mul(scale, m.load(0x79f280)));
    if (!out.call(0x4d1340))
        return;
    if ((m.byte(0x7a3a58) & 2) && !out.call(0x431900, {0x2bf, 0}))
        return;
    entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
}

/* sub_4d0a10: the 2D picture at [0x56727c] (+4, +6 its width and height) made
 * texture [0x840184] (44 bytes each from 0x83f6a8: its size twice, the THRASH
 * handle): one over 0x8000 pixels by sub_4d4080 at 256; else decoded
 * (sub_4d37a0, its format to [ebp-0x34]) and copied into a square of 64, 128
 * or 256 by its depth (sub_4d18b0: 8, 16 or 32 bits), cut or padded to it,
 * then THRASH_talloc and THRASH_tupdate. */
void pictureTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    // push ebx, ecx, edx, esi, edi, ebp; mov ebp, esp; sub esp, 0x34
    const reg32 frame = entry.esp - 24;
    Calls out(app, cpu, frame - 0x34);
    auto leave = [&]() {
        entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEdx | Entry::kEsi | Entry::kEdi | Entry::kEbp);
    };
    // the picture's width and height (+4, +6), read afresh each time: a copy may lie over them
    auto width = [&]() { return reg32(i32(m.word(m.word(0x56727c) + 2)) >> 16); };
    auto height = [&]() { return reg32(i32(m.word(m.word(0x56727c) + 4)) >> 16); };
    auto half = [&](reg32 at) { return x86::sreg16(app->getMemory<x86::reg16>(at)); };
    const reg32 picture = m.word(0x56727c);
    if (i32((i32(m.word(picture + 4)) >> 16) * (i32(m.word(picture + 2)) >> 16)) > 0x8000)
    {
        // a big one: sub_4d4080 at 256
        out.push(3);
        if (!out.call(0x4d4080, {0, picture, 0x100, 0x83f6a8 + m.word(0x840184) * 44}))
            return;
        const reg32 texture = m.word(0x840184) * 44;
        app->getMemory<x86::reg16>(texture + 0x83f6aa) = 0x100;
        app->getMemory<x86::reg16>(texture + 0x83f6a8) = 0x100;
        return leave();
    }
    // the format (sub_4d37a0's) and its flag on the frame, 3 and 1 till it sets them
    const reg32 format = frame - 0x34;
    m.word(format, 3);
    m.word(format + 4, 1);
    // its size: 64 or 128 when it fits, else 256
    reg32 size = 0x100;
    if (half(picture + 4) < 0x40 && half(picture + 6) < 0x40)
        size = 0x40;
    else if (half(m.word(0x56727c) + 4) < 0x80 && half(m.word(0x56727c) + 6) < 0x80)
        size = 0x80;
    const reg32 bytes = ((size * size) << 2) + 0x14;
    if (!out.call(kAllocate, {0x540b90, bytes, 0}))
        return;
    const reg32 decoded = out.result();
    if (!out.call(kAllocate, {0x540b90, bytes, 0, 0}))
        return;
    const reg32 square = out.result();
    out.push(format);
    if (!out.call(0x4d37a0, {decoded, m.word(0x56727c), format + 4, 0}) || !out.call(0x4d18b0, {m.word(format)}))
        return;
    /* The rows copied, `bytes` a pixel, from the decoded picture to the square, each min(size,
     * width) pixels, the square or the picture stepped past the rest. */
    auto copy = [&](reg32 pixel) {
        reg32 from = decoded + 0x10, to = square + 0x10;
        for (reg32 row = 0;; ++row)
        {
            const reg32 rows = i32(size) < i32(height()) ? size : height();
            if (i32(rows) <= i32(row))
                break;
            for (reg32 column = 0;; ++column)
            {
                const reg32 columns = i32(size) < i32(width()) ? size : width();
                if (i32(columns) <= i32(column))
                    break;
                if (pixel == 4)
                    m.word(to, m.word(from));
                else if (pixel == 1)
                    m.byte(to, m.byte(from));
                else
                    app->getMemory<x86::reg16>(to) = app->getMemory<x86::reg16>(from);
                from += pixel;
                to += pixel;
            }
            const reg32 across = width();
            if (i32(size) > i32(across))
                to += (size - across) * pixel;
            else if (i32(size) < i32(across))
                from += (across - size) * pixel;
        }
    };
    const reg32 depth = out.result();
    if (depth == 0x10)
        copy(2);
    else if (depth == 8)
        copy(1);
    else if (depth == 0x20)
        copy(4);
    // the texture: its size, THRASH_talloc, THRASH_tupdate
    const reg32 texture = m.word(0x840184) * 44;
    out.push(0);
    app->getMemory<x86::reg16>(texture + 0x83f6aa) = x86::reg16(size);
    out.push(0);
    app->getMemory<x86::reg16>(texture + 0x83f6a8) = x86::reg16(size);
    out.push(m.word(format));
    out.push(size);
    out.push(size);
    if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
        return;
    const reg32 handle = out.result();
    out.push(0);
    out.push(square + 0x10);
    out.push(handle);
    m.word(m.word(0x840184) * 44 + 0x83f6ac, handle);
    if (!out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>() || !out.call(0x4e1890, {decoded})
        || !out.call(0x4e1890, {square}))
        return;
    leave();
}

/* sub_4d3f50: a texture of ecx x ecx from the picture at eax (sub_4d37a0
 * decodes it, edx its palette, its format to [esp+4]): THRASH_talloc of its
 * width and height (the format 2 with flag 4 and the palette [0x563a84]),
 * THRASH_tupdate; the handle to eax, and at ebx the texture's corners inset
 * by half a texel ([0x549438] times its size): s, t at +8..+0x24, +0x28 0.
 * ret 4. */
void squareTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 picture = cpu.eax, palette = cpu.edx, corners = cpu.ebx;
    const reg32 format = entry.esp + 4;  // the caller's word, sub_4d37a0's to write
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x20
    Calls out(app, cpu, entry.esp - 12 - 0x20);
    // port: a picture of 512 a side or more at 512 (port_thrash.h)
    const reg32 side = reg32(port::driver::texture512Side(i32(cpu.ecx), x86::sreg16(app->getMemory<x86::reg16>(picture + 4))));
    if (!out.driver<thrash::kThrashAbout, &port::driver::about>()
        || !out.call(kAllocate, {0x549434, ((side * side) << 2) + 0x14, 0, side}))
        return;
    const reg32 buffer = out.result();
    out.push(format);
    if (!out.call(0x4d37a0, {buffer, picture, palette, side}))
        return;
    const bool paletted = m.word(format) == 2;
    out.push(0);
    out.push(paletted ? 4 : 0);
    out.push(m.word(format));
    out.push(reg32(i32(m.word(buffer + 4)) >> 16));
    out.push(reg32(i32(m.word(buffer + 2)) >> 16));
    if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
        return;
    const reg32 handle = out.result();
    out.push(paletted ? m.word(0x563a84) : 0);
    out.push(buffer + 0x10);
    out.push(handle);
    if (!out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>())
        return;
    // the corners half a texel in, each a float
    const double half = m.load(0x549438);
    const reg32 s0 = asBits(float(M::div(1.0, M::mul(double(x86::sreg16(app->getMemory<x86::reg16>(buffer + 4))), half))));
    const reg32 t0 = asBits(float(M::div(1.0, M::mul(half, double(x86::sreg16(app->getMemory<x86::reg16>(buffer + 6)))))));
    const reg32 s1 = asBits(float(M::sub(1.0, loaded(s0))));
    m.word(corners + 8, s0);
    m.word(corners + 0xc, t0);
    m.word(corners + 0x10, s1);
    const reg32 t1 = asBits(float(M::sub(1.0, loaded(t0))));
    m.word(corners + 0x14, t0);
    m.word(corners + 0x18, s1);
    m.word(corners + 0x1c, t1);
    m.word(corners + 0x20, s0);
    m.word(corners + 0x24, t1);
    m.byte(corners + 0x28, 0);
    if (!out.call(0x4e1890, {buffer}))
        return;
    cpu.eax = handle;  // a caller reads eax
    entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp, 4);
}

/* sub_4d41a0: the paletted picture at eax as a texture into the record at
 * edx (+0, +2 its size, +4 the THRASH handle, +8..+0x24 its corners' s, t,
 * +0x28 flags): resized to a power of two (sub_4d4120, sub_4d1ce0), its
 * 1555 palette (sub_4fd1c0) to ARGB at palette [0x563a98] of 0x8401a0 (400
 * of 1 KB, round robin); then with sub_4f24e0's name in sub_4e0820's list
 * ("0x54943c") expanded to 16 bits (sub_4d3380, format 7), else paletted
 * (format 2 with flag 4). */
void paletteTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 picture = cpu.eax, record = cpu.edx;
    // push ebx, ecx, esi, edi, ebp; mov ebp, esp; sub esp, 0x1c
    const reg32 frame = entry.esp - 20;
    Calls out(app, cpu, frame - 0x1c);
    auto half = [&](reg32 at) { return x86::reg16(app->getMemory<x86::reg16>(at)); };
    auto putHalf = [&](reg32 at, x86::reg16 value) { app->getMemory<x86::reg16>(at) = value; };
    auto leave = [&]() { entry.restore(cpu, Entry::kEbx | Entry::kEcx | Entry::kEsi | Entry::kEdi | Entry::kEbp); };
    // the record: the picture's size, its corners 0..1
    putHalf(record, half(picture + 4));
    const x86::reg16 tall = half(picture + 6);
    m.byte(record + 0x28, 0);
    static const reg32 corners[8] = {0, 0, 0x3f800000, 0, 0x3f800000, 0x3f800000, 0, 0x3f800000};
    for (reg32 k = 0; k < 8; ++k)
        m.word(record + 8 + 4 * k, corners[k]);
    m.byte(record + 0x29, 0);
    putHalf(record + 2, tall);
    if (!out.call(0x4d4120, {reg32(i32(m.word(picture + 4)) >> 16), reg32(i32(m.word(picture + 2)) >> 16)}))
        return;
    // resized to that
    const reg32 size = out.result();
    out.push(frame - 0x14);  // a word on the frame for sub_4d1ce0
    out.push(size);
    out.push(size);
    if (!out.call(0x4d1ce0, {picture + 0x10, picture + 0x10, reg32(i32(m.word(picture + 2)) >> 16),
                             reg32(i32(m.word(picture + 4)) >> 16)}))
        return;
    putHalf(record, x86::reg16(size));
    putHalf(record + 2, x86::reg16(size));
    if (!out.call(0x4fd1c0, {picture}))
        return;
    // the palette: 1555 to ARGB, alpha 0 or 0xff, into the round robin's slot
    const reg32 from = out.result() + 0x10;
    for (reg32 k = 0; k < 0x100; ++k)
    {
        const reg32 alpha = (m.byte(from + k * 2 + 1) & 0x80) ? 0xff : 0;
        const reg32 red = reg32(half(from + k * 2) & 0x7c00) >> 7;
        const reg32 green = reg32(half(from + k * 2) & 0x3e0) >> 2;
        const reg32 blue = half(from + k * 2) & 0x1f;
        m.word((m.word(0x563a98) << 10) + k * 4 + 0x8401a0, (green << 8) | (alpha << 24) | (red << 16) | (blue << 3));
    }
    if (!out.call(0x4f24e0, {picture}))
        return;
    bool expand = out.result() != 0;
    if (expand)
    {
        if (!out.call(0x4e0820, {out.result(), 0x54943c}))
            return;
        expand = out.result() != 0;
    }
    if (expand)
    {
        // expanded to 16 bits through the palette
        const reg32 pixels = reg32(half(record)) * reg32(half(record + 2));
        m.byte(record + 0x28, m.byte(record + 0x28) | 6);
        if (!out.call(kAllocate, {0x549434, pixels * 2, 0x10}))
            return;
        const reg32 buffer = out.result();
        if (!out.call(0x4d3380, {picture, buffer, 0x8401a0 + (m.word(0x563a98) << 10)}))
            return;
        out.push(0);
        out.push(0);
        out.push(7);
        out.push(half(record + 2));
        out.push(half(record));
        if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
            return;
        out.push(0);
        out.push(buffer);
        out.push(out.result());
        m.word(record + 4, out.result());
        if (!out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>() || !out.call(0x4e1890, {buffer}))
            return;
        return leave();
    }
    // paletted
    out.push(0);
    out.push(4);
    out.push(2);
    out.push(half(record + 2));
    const reg8 flags = m.byte(record + 0x28) | 0x10;
    out.push(half(record));
    m.byte(record + 0x28, flags);
    if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
        return;
    m.word(record + 4, out.result());
    out.push((m.word(0x563a98) << 10) + 0x8401a0);
    out.push(picture + 0x10);
    out.push(m.word(record + 4));
    if (!out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>())
        return;
    m.word(0x563a98, m.word(0x563a98) + 1);
    if (i32(m.word(0x563a98)) >= 0x190)
        m.word(0x563a98, 0);
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
void pieceTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 picture = cpu.eax, record = cpu.edx, format = cpu.ebx, x = cpu.ecx;
    const reg32 y = m.word(entry.esp + 4), barArgument = m.word(entry.esp + 8);
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    Calls out(app, cpu, entry.esp - 12 - 0x48);
    // eax 1 when no texture could be had, else 0: a caller reads it
    auto leave = [&](reg32 result) {
        cpu.eax = result;
        entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp, 8);
    };
    // n / 8 toward 0 (sar, shl, sbb, sar)
    auto eighth = [](reg32 n) { return reg32(i32(n + (reg32(i32(n) >> 31) & 7)) >> 3); };
    // a value through __CHP and fistp; nullopt when the game stopped
    auto truncate = [&](double value) -> std::optional<reg32> {
        const std::optional<x86::Float> whole = chopped(out, x86::Float(value));
        if (!whole)
            return std::nullopt;
        return reg32(fp::toInteger<x86::sreg32>(double(*whole)));
    };
    if (!out.call(0x4d18b0, {format}))
        return;
    const reg32 bits = out.result();
    // where the piece starts, its size to 256 (each a float)
    const reg32 wide = reg32(i32(m.word(picture + 2)) >> 16);
    reg32 from = picture + 0x10 + eighth(y * wide * bits);
    from += eighth(x * bits);
    const reg32 pieceWidth = i32(wide - x) > 0x100 ? 0x100 : wide - x;
    const double across = single(double(i32(pieceWidth)));
    const reg32 high = reg32(i32(m.word(picture + 4)) >> 16);
    const reg32 pieceHeight = i32(high - y) > 0x100 ? 0x100 : high - y;
    const double rowBytes = M::mul(M::mul(double(i32(bits)), across), m.load(0x549448));
    const double down = single(double(i32(pieceHeight)));
    const std::optional<reg32> rowSize = truncate(rowBytes);
    if (!rowSize || !out.call(kAllocate, {0x549434, 0x40000, 0, 0}))
        return;
    const reg32 buffer = out.result();
    // the rows: rep movsd, rep movsb, the destination 256 pixels on
    reg32 to = buffer;
    for (reg32 row = 0; fcomBelow(double(i32(row)), down); ++row)
    {
        reg32 source = from, target = to;
        for (reg32 n = *rowSize >> 2; n; --n, target += 4, source += 4)
            m.word(target, m.word(source));
        for (reg32 n = *rowSize & 3; n; --n, ++target, ++source)
            m.byte(target, m.byte(source));
        to += eighth(bits << 8);
        from += eighth((reg32(i32(m.word(picture + 2)) >> 16)) * bits);
    }
    // a piece short of 256 either way: bars in the cabin's strip (the bar record [0x792310]: its
    // texture, s, t, its size)
    const double K = m.load(0x54944c);
    if ((fcomBelow(across, K) || fcomBelow(down, K)) && i32(m.word(0x8b4418)) > i32(m.word(0x8b441c)))
    {
        // the strip's new start (the code's st(0), above the strip)
        double stripStart;
        auto bar = [&](reg32 count) { return count * 36; };
        if (fcomBelow(across, K) && fcomBelow(down, K))
        {
            // narrow and short: beside the strip, as wide as fits
            const reg32 strip = m.word(0x8b441c);
            const reg32 at = bar(m.word(0x792310));
            const std::optional<reg32> width = truncate(across);
            if (!width)
                return;
            m.word(at + 0x792090, record);
            const double rest = single(M::sub(K, across));
            m.word(at + 0x792098, *width);
            m.word(at + 0x7920a0, m.word(0x8b441c));
            const reg32 room = m.word(0x8b4418) - strip;
            m.word(at + 0x79209c, 0);
            m.word(at + 0x7920a4, 0);
            const double roomFloat = single(double(i32(room)));
            const std::optional<reg32> height = truncate(fcomBelow(rest, roomFloat) ? rest : roomFloat);
            if (!height)
                return;
            const reg32 next = bar(m.word(0x792310));
            m.word(next + 0x7920a8, *height);
            m.word(next + 0x7920ac, 0x100);
            m.word(next + 0x792094, 0x100);
            m.word(next + 0x7920b0, 0);
            if (!out.call(0x4d4440, {m.word(0x792310), barArgument, buffer, bits}))
                return;
            const double left = M::sub(K, across);
            const reg32 count = m.word(0x792310) + 1;
            const double stripStart0 = fild32(m, 0x8b441c);
            m.word(0x792310, count);
            stripStart = M::add(left, stripStart0);
        }
        else
        {
            const reg32 at = bar(m.word(0x792310));
            const reg32 strip = m.word(0x8b441c);
            const reg32 room = m.word(0x8b4418) - strip;
            if (fcomBelow(across, K))
            {
                // narrow, tall: its whole height
                const std::optional<reg32> width = truncate(across);
                if (!width)
                    return;
                m.word(at + 0x7920a0, strip);
                const double roomFloat = single(double(i32(room)));
                m.word(at + 0x792090, record);
                m.word(at + 0x79209c, 0);
                m.word(at + 0x792098, *width);
                const double rest = single(M::sub(K, across));
                m.word(at + 0x7920a4, 0);
                const std::optional<reg32> height = truncate(fcomBelow(rest, roomFloat) ? rest : roomFloat);
                if (!height)
                    return;
                const reg32 next = bar(m.word(0x792310));
                m.word(next + 0x7920a8, *height);
                m.word(next + 0x7920ac, 0x100);
                m.word(next + 0x7920b0, 0);
                m.word(next + 0x792094, 0x100);
                if (!out.call(0x4d4440, {m.word(0x792310), barArgument, buffer, bits}))
                    return;
                const double left = M::sub(K, across);
                const reg32 count = m.word(0x792310) + 1;
                const double stripStart0 = fild32(m, 0x8b441c);
                m.word(0x792310, count);
                stripStart = M::add(left, stripStart0);
            }
            else
            {
                // wide, short: lying on its side
                m.word(at + 0x7920a0, strip);
                m.word(at + 0x792090, record);
                const std::optional<reg32> height = truncate(down);
                if (!height)
                    return;
                m.word(at + 0x7920a4, 0);
                m.word(at + 0x792098, 0);
                m.word(at + 0x79209c, *height);
                const reg32 widthLimit = i32(room) > 0x100 ? 0x100 : room;
                const reg32 next = bar(m.word(0x792310));
                const double rest = single(M::sub(K, down));
                const std::optional<reg32> width = truncate(rest);
                if (!width)
                    return;
                m.word(next + 0x7920a8, widthLimit);
                m.word(next + 0x7920b0, 1);
                m.word(next + 0x792094, 0x100);
                m.word(next + 0x7920ac, *width);
                if (!out.call(0x4d4440, {m.word(0x792310), barArgument, buffer, bits}))
                    return;
                const double stripStart0 = fild32(m, 0x8b441c);
                m.word(0x792310, m.word(0x792310) + 1);
                stripStart = M::add(stripStart0, rest);
            }
        }
        // fstp st(1), the strip dropped: the strip's new start, through __CHP
        const std::optional<reg32> start = truncate(stripStart);
        if (!start)
            return;
        m.word(0x8b441c, *start);
    }
    // the texture
    const bool paletted = format == 2;
    out.push(0);
    out.push(paletted ? 4 : 0);
    out.push(format);
    out.push(0x100);
    out.push(0x100);
    if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
        return;
    const reg32 handle = out.result();
    m.word(record + 4, handle);
    if (!handle)
    {
        if (out.call(0x4e1890, {buffer}))
            leave(1);
        return;
    }
    out.push(paletted ? m.word(0x563a84) : 0);
    out.push(buffer);
    out.push(handle);
    if (!out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>())
        return;
    // its corners: half a texel in, out to the piece's size over 256
    const double scale = m.load(0x549450);
    const double s1 = M::mul(across, scale);
    m.word(record + 8, 0x3b000000);
    m.word(record + 0xc, 0x3b000000);
    const double t1 = M::mul(scale, down);
    m.word(record + 0x14, 0x3b000000);
    const double inset = m.load(0x549454);
    m.word(record + 0x20, 0x3b000000);
    const reg32 s = asBits(float(M::add(s1, inset)));
    m.word(record + 0x10, s);
    const reg32 t = asBits(float(M::add(t1, inset)));
    m.word(record + 0x18, s);
    m.word(record + 0x1c, t);
    m.word(record + 0x24, t);
    m.byte(record + 0x28, 4);
    if (out.call(0x4e1890, {buffer}))
        leave(0);
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
void cabinTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    const Memory m{app};
    const Entry entry(cpu);
    const reg32 compressed = cpu.eax, plain = cpu.edx, plainSecond = cpu.ebx, records = cpu.ecx;
    const reg32 format = entry.esp + 4;  // the caller's word, sub_4d37a0's to write
    const reg32 first = m.word(entry.esp + 8);
    // push esi, edi, ebp; mov ebp, esp; sub esp, 0x48
    const reg32 frame = entry.esp - 12;
    Calls out(app, cpu, frame - 0x48);
    // eax 0 when a texture could not be had, else the records used: a caller reads it
    auto leave = [&](reg32 result) {
        cpu.eax = result;
        entry.restore(cpu, Entry::kEsi | Entry::kEdi | Entry::kEbp, 8);
    };
    auto half = [](reg32 v) { return reg32(i32(v) >> 16); };
    auto less = [](reg32 a, reg32 b) { return i32(a) < i32(b); };
    auto least = [&](reg32 a, reg32 b) { return less(a, b) ? a : b; };
    reg32 used = 0;  // the records used
    reg32 fullWidth = 0, fullHeight = 0;  // the square's parts used up across, down
    // the bar [0x792310]'s record: its texture's record, s, t, x, y; its offset
    auto bar = [&](reg32 x, reg32 y) {
        const reg32 at = m.word(0x792310) * 36;
        m.word(at + 0x792090, records + used * 44);
        m.word(at + 0x792098, fullWidth);
        m.word(at + 0x79209c, fullHeight);
        m.word(at + 0x7920a0, x);
        m.word(at + 0x7920a4, y);
        return at;
    };
    reg32 second;
    if (compressed)
    {
        if (!out.call(0x4ef1e0, {compressed, plain}))
            return;
        m.word(first, out.result());
        if (!out.call(0x4ef1e0, {compressed, plainSecond}))
            return;
        second = out.result();
    }
    else
    {
        second = plainSecond;
        m.word(first, plain);
    }
    // the cabin
    m.word(0x792300, half(m.word(second + 6)));
    m.word(0x79230c, half(m.word(second + 8)));
    m.word(0x792304, reg32(i32(m.word(second + 0xc) << 20) >> 20));
    m.word(0x792308, reg32(i32(m.word(second + 0xc) << 4) >> 20));
    // buffers for both pictures, unpacked
    const reg32 shape = m.word(first);
    if (!out.call(kAllocate, {0x549434, ((half(m.word(shape + 2)) * half(m.word(shape + 4))) << 2) + 0x14, 0}))
        return;
    const reg32 pieces = out.result();
    if (!out.call(kAllocate, {0x549434, ((half(m.word(second + 2)) * half(m.word(second + 4))) << 2) + 0x14, 0}))
        return;
    const reg32 strip = out.result();
    const reg32 flag = frame - 0x48;  // sub_4d37a0's on the frame
    out.push(format);
    if (!out.call(0x4d37a0, {pieces, m.word(first), flag, 0}))
        return;
    out.push(format);
    if (!out.call(0x4d37a0, {strip, second, flag, 0}))
        return;
    m.word(0x792310, 0);
    m.word(0x8b441c, 0);
    m.word(0x8b4418, half(m.word(second + 2)));
    m.word(0x8b4414, half(m.word(second + 4)));
    // the pieces of 256 x 256, column by column
    for (reg32 x = 0; less(x, half(m.word(pieces + 2))); x += 0x100)
        for (reg32 y = 0; less(y, half(m.word(pieces + 4))); y += 0x100)
        {
            out.push(strip);
            out.push(y);
            const reg32 record = used * 44 + records;
            ++used;
            if (!out.call(0x4d4580, {pieces, record, m.word(format), x}))
                return;
            if (out.result())
            {
                if (out.call(0x4e1890, {strip}) && out.call(0x4e1890, {pieces}))
                    leave(0);
                return;
            }
        }
    if (!out.call(0x4e1890, {pieces}))
        return;
    // the strip's area, and the least square of 16 << n holding it
    reg32 area = (m.word(0x8b4414) - 0x100) * m.word(0x8b4418);
    if (less(m.word(0x8b441c), m.word(0x8b4418)))
        area += (m.word(0x8b4418) - m.word(0x8b441c)) << 8;
    reg32 side = 0x10;
    while (less(side * side, area))
        side += side;
    if (!out.call(kAllocate, {0x549434, (side * side) << 2, 0}))
        return;
    const reg32 square = out.result();
    // the part 256 high above: bars down each column of the square
    const reg32 step = least(side, m.word(0x8b4418));
    reg32 right;  // [0x8b4418] as the loop's test last read it
    for (reg32 x = 0; right = m.word(0x8b4418), less(x, right); x += step)
    {
        const reg32 tall = least(side, m.word(0x8b4414) - 0x100);
        for (reg32 y = 0x100; less(y, m.word(0x8b4414)); y += tall)
        {
            const reg32 at = bar(x, y);
            m.word(at + 0x7920a8, least(step, m.word(0x8b4418) - x));
            const reg32 count = m.word(0x792310);  // read before the record's last words: they may lie over it
            m.word(at + 0x7920ac, least(tall, m.word(0x8b4414) - y));
            m.word(at + 0x7920b0, 0);
            m.word(at + 0x792094, side);
            if (!out.call(0x4d4440, {count, strip, square, 0x10}))
                return;
            m.word(0x792310, m.word(0x792310) + 1);
            if (less(tall, step))
                fullHeight += tall;
            else
                fullWidth += step;
        }
    }
    // the strip itself, 256 high, in what is left of the square
    if (less(m.word(0x8b441c), right))
    {
        const reg32 wide = least(side - fullWidth, right - m.word(0x8b441c));
        const reg32 high = less(side - fullHeight, 0x100) ? side - fullHeight : 0x100;
        for (reg32 x = m.word(0x8b441c); less(x, m.word(0x8b4418)); x += wide)
        {
            reg32 y = 0;
            do
            {
                const reg32 at = bar(x, y);
                m.word(at + 0x7920a8, least(wide, m.word(0x8b4418) - x));
                const reg32 height = least(high, m.word(0x8b4414) - y);
                const reg32 count = m.word(0x792310);
                m.word(at + 0x7920ac, height);
                m.word(at + 0x792094, side);
                m.word(at + 0x7920b0, 0);
                if (!out.call(0x4d4440, {count, strip, square, 0x10}))
                    return;
                m.word(0x792310, m.word(0x792310) + 1);
                if (less(high, wide))
                    fullHeight += high;
                else
                    fullWidth += wide;
                y += high;
            } while (less(y, 0x100));
        }
    }
    // the square's texture, in the next record
    const reg32 record = records + used * 44;
    const reg32 kind = m.word(format);
    const bool paletted = kind == 2;
    out.push(0);
    out.push(paletted ? 4 : 0);
    out.push(kind);
    out.push(side);
    out.push(side);
    if (!out.driver<thrash::kThrashTAlloc, &port::driver::tAlloc>())
        return;
    const reg32 handle = out.result();
    m.word(record + 4, handle);
    auto drop = [&]() { return out.call(0x4e1890, {square}) && out.call(0x4e1890, {strip}); };
    if (!handle)
    {
        if (drop())
            leave(0);
        return;
    }
    out.push(paletted ? m.word(0x563a84) : 0);
    out.push(square);
    out.push(handle);
    if (out.driver<thrash::kThrashTUpdate, &port::driver::tUpdate>() && drop())
        leave(used);
}

}

}
