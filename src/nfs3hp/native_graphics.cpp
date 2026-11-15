#include "game_settings.h"
#include "native_carstate.h"
#include "native_collide.h"
#include "native_controls_input.h"
#include "native_controls_race.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_draw.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_graphics.h"
#include "native_load_cardb.h"
#include "native_load_saves.h"
#include "native_raceload.h"
#include "native_track.h"
#include "native_watcom.h"
#include "native_windows.h"
#include "port_snd_stream_calls.h"
#include "port_text.h"
#include "diag_hooks.h"
#include "thrash.h"
#include <winapi/advapi32.h>
#include <winapi/kernel32.h>
#include <winapi/user32.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>

/* The game's graphics that were still generated: what drives the THRASH
 * driver itself (voodoo2a, the port's own in port_thrash.cpp) -- its states,
 * the frame and the window, the textures, the screen modes -- one function at
 * a time, each where its generated code was.  disassemble_nfs3hp.py binds the
 * addresses of decompiled.json named in "graphics" to the functions of those
 * names here (DECOMPILED_NATIVES), and the generated code calls them by name.
 *
 * Each is a typed function of its own (a core), with an adapter under the
 * game's name for the callers still on registers and the dispatch: it takes
 * the registers, calls the core and leaves eax, the registers the game's code
 * pushes and pops (Kept) and the x87 as the code does.  The other typed
 * files' functions their headers declare the cores call as C++
 * (guest::direct), the registers the code has around such a call as it has
 * them; the game's other functions they call by address, through a small
 * helper each with the registers the code has at that call (the check's
 * stand-ins log them, and a void one leaves eax as it was); the driver's
 * through the game's pointers to it (0x9ef900 on, thrash.h's order,
 * thrash()).  Locals a callee is handed the
 * address of stay on the guest's stack as deep as the code has them
 * (GuestFrame), and so does a call that reaches the Error, which hands
 * vsprintf an address on the stack.  gameMain alone is still written as its
 * code is, each push and pop to the guest's stack.
 * tools/game_checks holds each against its generated code, with every one of
 * the driver's functions stood in by its name. */
namespace nfs3hp
{
namespace graphics
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

// The game's pointers to the driver's functions (THRASH_opendll fills them).
constexpr reg32 kThrashDrawLineMesh = 0x9ef90c, kThrashTReset = 0x9ef918, kThrashReadRect = 0x9ef920;
constexpr reg32 kThrashDrawTriMesh = 0x9ef924, kThrashClip = 0x9ef92c, kThrashDrawQuadMesh = 0x9ef930;
constexpr reg32 kThrashSetTexture = 0x9ef934, kThrashWindow = 0x9ef938, kThrashIdle = 0x9ef93c;
constexpr reg32 kThrashPageFlip = 0x9ef940, kThrashSync = 0x9ef948, kThrashDrawQuad = 0x9ef94c;
constexpr reg32 kThrashFlushWindow = 0x9ef950, kThrashLockWindow = 0x9ef954, kThrashSetVideoMode = 0x9ef958;
constexpr reg32 kThrashUnlockWindow = 0x9ef95c, kThrashTUpdate = 0x9ef964, kThrashTAlloc = 0x9ef968;
constexpr reg32 kThrashSetState = 0x9ef96c, kThrashClearWindow = 0x9ef970, kThrashDrawTri = 0x9ef974;
constexpr reg32 kThrashInit = 0x9ef978, kThrashRestore = 0x9ef97c, kThrashAbout = 0x9ef980;

using App = win32::WinApplication;
using CPU = x86::CPU;

/* One of the driver's functions, through the game's pointer to it: stdcall, its arguments on the
 * guest's stack.  What it leaves in eax -- a void one leaves eax as the caller had it, so eax at the
 * call is what the game's code has there -- or nullopt when the game stopped in it. */
std::optional<reg32> thrash(App* app, CPU& cpu, reg32 pointer, std::initializer_list<reg32> arguments = {})
{
    if (!guest::call(app, cpu, at(app, pointer), arguments))
        return std::nullopt;
    return cpu.eax;
}

/* THRASH_setstate(state, value). */
std::optional<reg32> setState(App* app, CPU& cpu, reg32 state, reg32 value)
{
    return thrash(app, cpu, kThrashSetState, {state, value});
}

/* Locals the game's code hands a callee the address of: kept on the guest's stack as deep as the
 * code has them (`pushed` below esp on entry, `bytes` of them), esp under them while it lives. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu(cpu), esp(cpu.esp), base(cpu.esp - pushed)
    {
        cpu.esp = base - bytes;
    }
    ~GuestFrame() { cpu.esp = esp; }
    reg32 operator-(reg32 offset) const { return base - offset; }

private:
    CPU& cpu;
    const reg32 esp;
    const reg32 base;
};

constexpr reg32 kError = 0x401010;

/* sub_401010, the game's Error, by address: its text on the stack (cdecl).  By address: Error's code
 * hands what it calls pointers into its frame, which lies 4 bytes lower with the text pushed. */
bool error(App* app, CPU& cpu, reg32 text)
{
    return guest::call(app, cpu, kError, {text});
}

// Which of ebx, ecx, edx, esi, edi a function of the game's pushes on entry and pops before it
// returns.
constexpr unsigned kEbx = 1, kEcx = 2, kEdx = 4, kEsi = 8, kEdi = 0x10;

/* Those registers as the caller had them, which an adapter gives back: whatever the driver or the
 * callees left in them, the game's code's caller sees them unchanged. */
struct Kept
{
    Kept(const CPU& cpu, unsigned which)
        : which(which), ebx(cpu.ebx), ecx(cpu.ecx), edx(cpu.edx), esi(cpu.esi), edi(cpu.edi)
    {
    }
    void back(CPU& cpu) const
    {
        if (which & kEbx)
            cpu.ebx = ebx;
        if (which & kEcx)
            cpu.ecx = ecx;
        if (which & kEdx)
            cpu.edx = edx;
        if (which & kEsi)
            cpu.esi = esi;
        if (which & kEdi)
            cpu.edi = edi;
    }
    unsigned which;
    reg32 ebx, ecx, edx, esi, edi;
};

// The driver's flags (sub_4b6020): 1 << its type, a word.  0x06 the software
// renderers, 0x180 PowerVR's.
constexpr reg32 kDriverFlags = 0x7a3a58;
constexpr reg16 kDriverSoftware = 0x06, kDriverPowerVr = 0x180;

}  // namespace

/* ---- The render states, each sent to the driver only as it changes ---- */

// The last value sent of each state that is kept (native_render.cpp's
// [0x554e54] and [0x554e58] among them).
constexpr reg32 kKeptTexture = 0x554e54;        // thrash::kStateTexture
constexpr reg32 kKeptBlend = 0x554e58;          // thrash::kStateBlend
constexpr reg32 kKeptFog = 0x554e5c;            // thrash::kStateFog; negative: never sent again
constexpr reg32 kKeptTextureAddress = 0x554e60; // thrash::kStateTextureAddress
constexpr reg32 kKeptDepth = 0x554e64;          // thrash::kStateDepth
constexpr reg32 kKeptFogDensity = 0x554e68;     // thrash::kStateFogDensity
constexpr reg32 kKeptFogColour = 0x554e6c;      // thrash::kStateFogColour

/* 0x431900's core: render state `state` set to `value` through THRASH_setstate.  The texture, the
 * blending, the fog, the texture's addressing and the depth test are sent only when they change; a
 * new fog density or colour sends the fog's own state again after it, unless that is negative.
 * Every other state is sent as it comes.  The driver's answer (the first's, of two), or 1 for one
 * not sent.  eax at each call as the game's code has it: the value, the fog for the fog again after
 * a density, the first answer after a colour. */
std::optional<reg32> renderState(App* app, CPU& cpu, reg32 state, reg32 value)
{
    cpu.eax = value;
    // A kept state: sent if it changed.
    auto kept = [&](reg32 keptAt) -> std::optional<reg32> {
        if (value == at(app, keptAt))
            return 1;
        put(app, keptAt, value);
        return setState(app, cpu, state, value);
    };
    // A fog density or colour, kept, then the fog's own state again when it is not negative.
    auto keptThenFog = [&](reg32 keptAt) -> std::optional<reg32> {
        if (value == at(app, keptAt))
            return 1;
        put(app, keptAt, value);
        const std::optional<reg32> answer = setState(app, cpu, state, value);
        if (!answer)
            return std::nullopt;
        const reg32 fog = at(app, kKeptFog);
        if (sreg32(fog) < 0)
            return answer;
        if (state == thrash::kStateFogDensity)
            cpu.eax = fog;
        if (!setState(app, cpu, thrash::kStateFog, fog))
            return std::nullopt;
        return answer;
    };
    switch (state)
    {
    case thrash::kStateTexture:
        return kept(kKeptTexture);
    case thrash::kStateDepth:
        return kept(kKeptDepth);
    case thrash::kStateTextureAddress:
        return kept(kKeptTextureAddress);
    case thrash::kStateFogDensity:
        return keptThenFog(kKeptFogDensity);
    case thrash::kStateFogColour:
        return keptThenFog(kKeptFogColour);
    case thrash::kStateFog:
        return kept(kKeptFog);
    case thrash::kStateBlend:
        return kept(kKeptBlend);
    default:
        return setState(app, cpu, state, value);
    }
}

/* renderState as the game's code calls 0x431900: the value in edx, eax its answer after, ebx and
 * ecx kept.  False when the game stopped. */
bool sendState(App* app, CPU& cpu, reg32 state, reg32 value)
{
    const Kept kept(cpu, kEbx | kEcx);
    cpu.edx = value;
    const std::optional<reg32> answer = renderState(app, cpu, state, value);
    if (!answer)
        return false;
    cpu.eax = *answer;
    kept.back(cpu);
    return true;
}

/* 0x431900: renderState for state eax, value edx. */
void setRenderState(App* app, CPU& cpu)
{
    if (sendState(app, cpu, cpu.eax, cpu.edx))
        cpu.esp += 4;
}

/* The software renderers' blending (softtria's): the rows of the colour
 * tables a blended colour is drawn with, sent as a state of their own with the
 * function that blends by them. */
constexpr reg32 kStateSoftwareBlend = 0x2bd;
constexpr reg32 kSoftwareBlendDraw = 0x431d50;     // what the state hands the renderer
constexpr reg32 kBlendTable = 0x5f1798;            // 64 bytes a row
constexpr reg32 kBlendTableGreen = 0x5f0798;       // the green rows of a 5:6:5 screen, twice as long
constexpr reg32 kBlendRowBlue = 0x5f1f98, kBlendRowRed = 0x5f1f9c, kBlendRowGreen = 0x5f1fa4;  // the rows chosen
constexpr reg32 kScreenBits = 0x56438c;            // 15 for a 5:5:5 screen

/* A blended colour's row of the blending tables: the channel times the alpha, by 2048, a row a
 * step. */
reg32 blendStep(reg32 channel, reg32 alpha)
{
    return channel * alpha >> 11;  // at most 255 * 255: the code's sar is a shift
}

/* 0x432390's core: with the blending on (kKeptBlend 1), the rows for colour 0xAARRGGBB chosen and
 * sent with kSoftwareBlendDraw.  eax at the call as the code has it: the green row's offset. */
bool softwareBlendColour(App* app, CPU& cpu, reg32 colour)
{
    if (at(app, kKeptBlend) != 1)
        return true;
    const reg32 alpha = colour >> 24;
    put(app, kBlendRowBlue, kBlendTable + (blendStep(colour & 0xff, alpha) << 6));
    put(app, kBlendRowRed, kBlendTable + (blendStep(colour >> 16 & 0xff, alpha) << 6));
    const reg32 green = blendStep(colour >> 8 & 0xff, alpha) << 5;
    const reg32 offset = at(app, kScreenBits) == 0xf ? green * 2 : green << 2;
    put(app, kBlendRowGreen, (at(app, kScreenBits) == 0xf ? kBlendTable : kBlendTableGreen) + offset);
    cpu.eax = offset;
    return setState(app, cpu, kStateSoftwareBlend, kSoftwareBlendDraw).has_value();
}

/* 0x432390: softwareBlendColour for colour eax. */
void setSoftwareBlendColour(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!softwareBlendColour(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x432370: the software renderers' blending off (kStateSoftwareBlend 0). */
void clearSoftwareBlend(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!setState(app, cpu, kStateSoftwareBlend, 0))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The frame ---- */

/* 0x434a00's core: a frame ended: the window taken (THRASH_window 2) but on PowerVR, the picture
 * flushed if `flush`, flipped if `flip`, the window again, and on a card that is neither PowerVR nor
 * a software renderer, the window cleared.  eax as the driver leaves it. */
bool frameEnd(App* app, CPU& cpu, bool flip, bool flush)
{
    const bool powerVr = half(app, kDriverFlags) & kDriverPowerVr;
    auto window = [&]() { return powerVr || thrash(app, cpu, kThrashWindow, {2}); };
    if (!window() || (flush && !thrash(app, cpu, kThrashFlushWindow)) || (flip && !thrash(app, cpu, kThrashPageFlip))
        || !window())
        return false;
    if (!(byte(app, kDriverFlags) & kDriverSoftware) && !powerVr)
        return thrash(app, cpu, kThrashClearWindow).has_value();
    return true;
}

/* 0x434a00: frameEnd, flipped if eax, flushed if edx. */
void endFrame(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    if (!frameEnd(app, cpu, cpu.eax != 0, cpu.edx != 0))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x434b20: the driver waited for (THRASH_sync 0) and the window taken (THRASH_window 2). */
void syncWindow(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!thrash(app, cpu, kThrashSync, {0}) || !thrash(app, cpu, kThrashWindow, {2}))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* The picture flushed and flipped (THRASH_flushwindow, THRASH_pageflip). */
bool flushFlip(App* app, CPU& cpu)
{
    return thrash(app, cpu, kThrashFlushWindow) && thrash(app, cpu, kThrashPageFlip);
}

/* 0x434b40: flushFlip. */
void flushAndFlip(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!flushFlip(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x5244e0: THRASH_idle; eax 0. */
void driverIdle(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!thrash(app, cpu, kThrashIdle))
        return;
    cpu.eax = 0;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- Clearing and flipping ---- */

constexpr reg32 kDriverOnCard = 0x7a3a60;      // 1 but on the software renderers (sub_4b59c0)
constexpr reg16 kDriverSofttria = 0x02;        // kDriverFlags' bit for softtria

/* 0x4d5c90's core: the window cleared with the depth test as the driver has it, and the depth test
 * off after; ecx and edx kept, as the game's code keeps them for its callers.  eax: the last
 * state's answer. */
bool clearDepth(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!sendState(app, cpu, thrash::kStateDepth, at(app, kDriverOnCard)) || !thrash(app, cpu, kThrashClearWindow)
        || !sendState(app, cpu, thrash::kStateDepth, 0))
        return false;
    kept.back(cpu);
    return true;
}

/* 0x4d5c90: clearDepth. */
void clearWithDepth(App* app, CPU& cpu)
{
    if (clearDepth(app, cpu))
        cpu.esp += 4;
}

constexpr reg32 kDrawingBegun = 0x563b30;      // set by beginDrawing, once
constexpr reg32 kResetTextures = 0x4d5c40;

/* sub_4d5c40, every texture let go of (port_shapes), by address: eax, edx, ebx and ecx as the
 * game's code has them there, which its stand-in logs. */
bool resetTextures(App* app, CPU& cpu)
{
    return byAddress(app, cpu, kResetTextures);
}

/* 0x4d5cc0's core: drawing begun, once: the textures reset first if `reset` (in eax, `clear` in edx
 * and ebx at that call, as the code has them); then each buffer (THRASH_window 2, then 1) cleared
 * if `clear`. */
bool drawingBegun(App* app, CPU& cpu, reg32 reset, reg32 clear)
{
    if (at(app, kDrawingBegun) != 0)
        return true;
    put(app, kDrawingBegun, 1);
    cpu.ebx = clear;
    if (reset != 0 && !resetTextures(app, cpu))
        return false;
    for (const reg32 buffer : {2u, 1u})
        if (!thrash(app, cpu, kThrashWindow, {buffer}) || (clear != 0 && !clearDepth(app, cpu)))
            return false;
    return true;
}

/* 0x4d5cc0: drawingBegun, reset eax, clear edx. */
void beginDrawing(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    if (!drawingBegun(app, cpu, cpu.eax, cpu.edx))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* Both buffers cleared: the window taken, then twice cleared, flushed and flipped. */
bool clearTwice(App* app, CPU& cpu)
{
    if (!thrash(app, cpu, kThrashWindow, {2}))
        return false;
    for (int i = 0; i < 2; ++i)
        if (!clearDepth(app, cpu) || !flushFlip(app, cpu))
            return false;
    return true;
}

/* 0x4d5d30: both buffers cleared. */
void clearBothBuffers(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!clearTwice(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x4d5d70: both buffers cleared, then the textures reset. */
void clearBothBuffersAndReset(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!clearTwice(app, cpu) || !resetTextures(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x4d9120's core: the window taken (THRASH_window 2) and, but on softtria, cleared; ecx and edx
 * kept, as the code keeps them. */
bool windowTakenAndCleared(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!thrash(app, cpu, kThrashWindow, {2})
        || (!(byte(app, kDriverFlags) & kDriverSofttria) && !clearDepth(app, cpu)))
        return false;
    kept.back(cpu);
    return true;
}

/* 0x4d9120: windowTakenAndCleared. */
void takeWindowAndClear(App* app, CPU& cpu)
{
    if (windowTakenAndCleared(app, cpu))
        cpu.esp += 4;
}

constexpr reg32 kTicks = 0x79c1a4;             // the game's clock (native_frontend_loop.cpp's)
constexpr reg32 kLastFlipTick = 0x56415c;      // the clock at the last flipAndWaitFrames
constexpr reg32 kWaitForTicks = 0x4c9860;

/* sub_4c9860, a wait for the clock, by address: the clock in eax and edx. */
bool waitForTicks(App* app, CPU& cpu, reg32 ticks)
{
    cpu.eax = ticks;
    cpu.edx = ticks;
    return byAddress(app, cpu, kWaitForTicks);
}

/* 0x4d9140's core: the picture flushed and flipped, then waited (sub_4c9860) until the clock is two
 * ticks past the last flip's, which it is kept as.  The tick waited for, as eax has it, or nullopt
 * when the game stopped. */
std::optional<reg32> flipAndWait(App* app, CPU& cpu)
{
    if (!flushFlip(app, cpu))
        return std::nullopt;
    reg32 now = at(app, kTicks);
    reg32 until = at(app, kLastFlipTick) + 2;
    while (sreg32(until) > sreg32(now))
    {
        now = at(app, kTicks);
        if (!waitForTicks(app, cpu, now))
            return std::nullopt;
        until = at(app, kLastFlipTick) + 2;
    }
    put(app, kLastFlipTick, now);
    return until;
}

/* 0x4d9140: flipAndWait. */
void flipAndWaitFrames(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    const std::optional<reg32> until = flipAndWait(app, cpu);
    if (!until)
        return;
    cpu.eax = *until;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- Drawn whole ---- */

/* A rectangle's four corners (0x20 bytes each: x, y as floats, the colour at
 * +0x10), and drawFlatRectangle's. */
constexpr reg32 kRectCorners = 0x5599a0;
constexpr reg32 kCornerBytes = 0x20, kCornerColour = 0x10;

void putFloat(App* app, reg32 address, float value)
{
    app->getMemory<float>(address) = value;
}

float floatAt(App* app, reg32 address)
{
    return app->getMemory<float>(address);
}

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* 0x459350's core: a rectangle drawn flat, untextured, between corners (left, top) and (right,
 * bottom), `colour` at each: its corners clockwise from the first, the texture off, THRASH_drawquad.
 * eax as the driver leaves it. */
bool flatRectangle(App* app, CPU& cpu, sreg32 left, sreg32 top, sreg32 right, sreg32 bottom, reg32 colour)
{
    const auto corner = [](reg32 n) { return kRectCorners + n * kCornerBytes; };
    for (reg32 n = 3; n < 4; --n)
        put(app, corner(n) + kCornerColour, colour);
    // Each fild and fstp: the integer as a float, rounded as C++ rounds it.
    putFloat(app, corner(0), float(left));
    putFloat(app, corner(1), float(right));
    putFloat(app, corner(0) + 4, float(top));
    putFloat(app, corner(2) + 4, float(bottom));
    putFloat(app, corner(2), floatAt(app, corner(1)));
    putFloat(app, corner(3), floatAt(app, corner(0)));
    putFloat(app, corner(1) + 4, floatAt(app, corner(0) + 4));
    putFloat(app, corner(3) + 4, floatAt(app, corner(2) + 4));
    return sendState(app, cpu, thrash::kStateTexture, 0)
        && thrash(app, cpu, kThrashDrawQuad, {corner(0), corner(1), corner(2), corner(3)});
}

/* 0x459350: flatRectangle from (eax, edx) to (ebx, ecx), the colour [esp+4], which ebx is left. */
void drawFlatRectangle(App* app, CPU& cpu)
{
    const reg32 colour = at(app, cpu.esp + 4);
    if (!flatRectangle(app, cpu, sreg32(cpu.eax), sreg32(cpu.edx), sreg32(cpu.ebx), sreg32(cpu.ecx), colour))
        return;
    cpu.ebx = colour;
    cpu.esp += 4 + 4;
}

/* The picture drawn whole on the screen: its four corners and the mesh that
 * makes them one quad. */
constexpr reg32 kPictureCorners = 0x55e60c, kPictureMesh = 0x55e68c;

/* 0x49c570's core: a frame of one picture, `texture`, over the whole screen: the window taken and
 * cleared, the picture drawn, flushed, flipped, waited for; then the game's messages seen to
 * (ea::waitKey, 0x4f0120). */
bool picture(App* app, CPU& cpu, reg32 texture)
{
    return thrash(app, cpu, kThrashWindow, {2}) && thrash(app, cpu, kThrashClearWindow)
        && thrash(app, cpu, kThrashSetTexture, {texture})
        && thrash(app, cpu, kThrashDrawQuadMesh, {1, kPictureCorners, kPictureMesh}) && flushFlip(app, cpu)
        && thrash(app, cpu, kThrashSync, {0}) && guest::direct<0x4f0120, ea::waitKey>(app, cpu).has_value();
}

/* 0x49c570: picture, texture eax. */
void showPicture(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!picture(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

std::optional<reg32> textureFormat(App* app, CPU& cpu);

/* 0x49c600's core: a texture allocated (THRASH_talloc) of `width` and `height` -- eax and edx as
 * the code hands them on -- in the format textureFormat gives.  eax as the driver leaves it. */
bool textureAllocated(App* app, CPU& cpu, reg32 width, reg32 height)
{
    std::optional<reg32> format;
    {
        const GuestFrame deep(cpu, 0xc, 0);  // the code's two pushes and the call's slot, for the Error's
        format = textureFormat(app, cpu);
    }
    return format && thrash(app, cpu, kThrashTAlloc, {width, height, *format, 0, 0});
}

/* 0x49c600: textureAllocated, eax by edx. */
void allocTexture(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!textureAllocated(app, cpu, cpu.eax, cpu.edx))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* The screen's size, and the rectangle drawn in (native_frontend_screen's
 * kViewport): left, top, width, height. */
constexpr reg32 kScreenWidth = 0x7cda68, kScreenHeight = 0x7cda6c;
constexpr reg32 kClipLeft = 0x7cda70, kClipTop = 0x7cda74, kClipWidth = 0x7cda78, kClipHeight = 0x7cda7c;
constexpr reg32 kSetViewCorners = 0x4bf1b0;    // left, top, right, bottom as floats, then two more
constexpr reg32 kViewNear = 0x3f800000, kViewDepthScale = 0x3b0355ad;  // setViewCorners' two more: 1, about 0.002

/* 0x4bebe0's core: the rectangle drawn in set to x, y, `wide` by `high`, kept on the screen:
 * THRASH_clip given its corners, sub_4bf1b0 them as floats with 1 and 0.002.  The registers at that
 * call as the code has them: eax the right, ebx the width, ecx the height. */
bool clipSet(App* app, CPU& cpu, sreg32 x, sreg32 y, sreg32 wide, sreg32 high)
{
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    const sreg32 lastX = sreg32(at(app, kScreenWidth) - 1), lastY = sreg32(at(app, kScreenHeight) - 1);
    put(app, kClipLeft, reg32(x >= lastX ? lastX : x));
    put(app, kClipTop, reg32(y >= lastY ? lastY : y));
    const sreg32 widthLeft = sreg32(at(app, kScreenWidth) - at(app, kClipLeft));
    put(app, kClipWidth, reg32(wide < widthLeft ? wide : widthLeft));
    const sreg32 heightLeft = sreg32(at(app, kScreenHeight) - at(app, kClipTop));
    put(app, kClipHeight, reg32(high < heightLeft ? high : heightLeft));
    const reg32 left = at(app, kClipLeft), top = at(app, kClipTop);
    cpu.edx = left;
    if (!thrash(app, cpu, kThrashClip,
                {left, top, left + at(app, kClipWidth), at(app, kClipHeight) + top}))
        return false;
    const reg32 width = at(app, kClipWidth), height = at(app, kClipHeight);
    const reg32 right = at(app, kClipLeft) + width, bottom = at(app, kClipTop) + height;
    cpu.eax = right;
    cpu.ebx = width;
    cpu.ecx = height;
    // Each fild and fstp: the integer as a float, rounded as C++ rounds it.
    return guest::call(app, cpu, kSetViewCorners,
                       {bitsOf(float(sreg32(at(app, kClipLeft)))), bitsOf(float(sreg32(at(app, kClipTop)))),
                        bitsOf(float(sreg32(right))), bitsOf(float(sreg32(bottom))), kViewNear, kViewDepthScale});
}

/* 0x4bebe0: clipSet to x eax, y edx, width ebx, height ecx. */
void setClip(App* app, CPU& cpu)
{
    if (clipSet(app, cpu, sreg32(cpu.eax), sreg32(cpu.edx), sreg32(cpu.ebx), sreg32(cpu.ecx)))
        cpu.esp += 4;
}

/* The view and what surrounds it. */
constexpr reg32 kViewWidth = 0x7cdba0;         // the view's rectangle, from 0x7cdb98: x, y, width, height
constexpr reg32 kViewSurroundFrames = 0x7cdba8;  // counted down at each drawViewSurround
constexpr reg32 kSurroundCorners = 0x55fed4, kSurroundMesh = 0x560054;

/* 0x4be8e0's core: the quads around the view, untextured, without the depth test -- two of them in
 * split screen, four otherwise -- when the view is narrower than the screen, or with Wide Screen in
 * a race of one screen's player.  The last state's answer, or the view's width when nothing is
 * drawn: what eax is left. */
std::optional<reg32> viewSurround(App* app, CPU& cpu)
{
    put(app, kViewSurroundFrames, at(app, kViewSurroundFrames) - 1);
    const reg32 viewWidth = at(app, kViewWidth);
    const bool narrower = sreg32(viewWidth) < sreg32(at(app, kScreenWidth));
    if (!narrower && (gameSettings(app).wideScreen == 0 || gameSettings(app).playMode == 1))
        return viewWidth;
    if (!sendState(app, cpu, thrash::kStateTexture, 0) || !sendState(app, cpu, thrash::kStateDepth, 0)
        || !thrash(app, cpu, kThrashDrawQuadMesh,
                   {gameSettings(app).playMode == 1 ? 2u : 4u, kSurroundCorners, kSurroundMesh})
        || !sendState(app, cpu, thrash::kStateDepth, at(app, kDriverOnCard)))
        return std::nullopt;
    return cpu.eax;
}

/* 0x4be8e0: viewSurround. */
void drawViewSurround(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    const std::optional<reg32> left = viewSurround(app, cpu);
    if (!left)
        return;
    cpu.eax = *left;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The software renderer's frame buffer ---- */

constexpr reg32 kFrameLocked = 0x56aff4;       // the frame buffer locked by the game
constexpr reg32 kFrameLock = 0xa18220;         // THRASH_lockwindow's record, or none
constexpr reg32 kFrameBuffers = 0x565020;      // two buffers to free as the lock goes
constexpr reg32 kFreeBuffer = 0x509b10;

/* sub_509b10, a block freed, by address: the block in eax. */
bool freeBlock(App* app, CPU& cpu, reg32 block)
{
    cpu.eax = block;
    return byAddress(app, cpu, kFreeBuffer);
}

/* 0x5248f0's core: the frame buffer's lock let go of (THRASH_unlockwindow) and its two buffers
 * freed, if it is locked; ebx, ecx and edx kept, as the code keeps them.  eax as the last call
 * leaves it. */
bool frameBufferUnlocked(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const reg32 lock = at(app, kFrameLock);
    if (lock != 0)
    {
        if (!thrash(app, cpu, kThrashUnlockWindow, {lock}))
            return false;
        put(app, kFrameLock, 0);
        if (!freeBlock(app, cpu, at(app, kFrameBuffers)) || !freeBlock(app, cpu, at(app, kFrameBuffers + 4)))
            return false;
    }
    kept.back(cpu);
    return true;
}

/* 0x5248f0: frameBufferUnlocked. */
void unlockFrameBuffer(App* app, CPU& cpu)
{
    if (frameBufferUnlocked(app, cpu))
        cpu.esp += 4;
}

/* 0x5178d0's core: the frame buffer let go of -- unlocked straight when the game locked it, by
 * frameBufferUnlocked when the driver did -- and the window taken (THRASH_window 0); ecx and edx
 * kept, as the code keeps them. */
bool frameLockReleased(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (at(app, kFrameLocked) != 0)
    {
        if (!thrash(app, cpu, kThrashUnlockWindow, {at(app, kFrameLock)}))
            return false;
        put(app, kFrameLock, 0);
        put(app, kFrameLocked, 0);
    }
    else if (at(app, kFrameLock) != 0 && !frameBufferUnlocked(app, cpu))
        return false;
    if (!thrash(app, cpu, kThrashWindow, {0}))
        return false;
    kept.back(cpu);
    return true;
}

/* 0x5178d0: frameLockReleased. */
void releaseFrameLock(App* app, CPU& cpu)
{
    if (frameLockReleased(app, cpu))
        cpu.esp += 4;
}

/* 0x517990: the frame buffer let go of and the picture flipped. */
void releaseAndFlip(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!frameLockReleased(app, cpu) || !thrash(app, cpu, kThrashPageFlip))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The driver opened ---- */

constexpr reg32 kDriverOpen = 0x56aff0;        // a byte: THRASH_init done, until the driver is closed
constexpr reg32 kCloseAtExit = 0x56aff1;       // a byte: closeDriverAtExit handed to sub_4f2a78 once
constexpr reg32 kWantedWidth = 0x564384, kWantedHeight = 0x564388;  // the screen mode's, 0 for 640 by 480
constexpr reg32 kInstance = 0x564480;          // sub_4eb2e0's, the first time
constexpr reg32 kGameWindow = 0x9f3d24;        // native_game's kWindow
constexpr reg32 kLambdasRecord = 0x56afd0;     // thrash::kStateLambdas' record
constexpr reg32 kErrorLock = 0x564418;         // native_game's: called before an Error's box
constexpr reg32 kMakeWindow = 0x4cd6c0;        // eax the width, ebx whether a window of its own
constexpr reg32 kDestroyWindow = 0x4cdaa0;
constexpr reg32 kAcquireWindow = 0x4eba70;     // native_game's AcquireWindow
constexpr reg32 kAddTimer = 0x4e7520;          // eax a function, edx its argument, ebx the ticks between
constexpr reg32 kAtExit = 0x4f2a78;            // eax a function, called as the game ends
constexpr reg32 kDriverIdle = 0x5244e0;        // driverIdle
constexpr reg32 kCloseDriverAtExit = 0x524640, kCloseDriverForError = 0x524620;

/* A function of the game's by address, the registers as the caller set them: its eax, or nullopt
 * when the game stopped in it. */
std::optional<reg32> game(App* app, CPU& cpu, reg32 target)
{
    if (!byAddress(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

/* sub_4cd6c0, the game's window made, by address: eax the width, edx the height, ebx whether a
 * window of its own.  Its answer, 0 for none. */
std::optional<reg32> makeWindow(App* app, CPU& cpu, reg32 width, reg32 height, bool own)
{
    cpu.eax = width;
    cpu.edx = height;
    cpu.ebx = own ? 1 : 0;
    return game(app, cpu, kMakeWindow);
}

/* port::addSystemTask (0x4e7520), by address: eax the function, edx its argument, ebx the ticks
 * between. */
bool addTask(App* app, CPU& cpu, reg32 function, reg32 argument, reg32 ticks)
{
    cpu.eax = function;
    cpu.edx = argument;
    cpu.ebx = ticks;
    return byAddress(app, cpu, kAddTimer);
}

/* sub_4f2a78, `function` run as the game ends, by address (eax). */
bool atExit(App* app, CPU& cpu, reg32 function)
{
    cpu.eax = function;
    return byAddress(app, cpu, kAtExit);
}

constexpr reg32 kDefaultWidth = 640, kDefaultHeight = 480;

/* 0x5246a0's core: the driver opened, unless it is already: the game's window made if it has none
 * (one of its own for `windowMode` 0), the driver given the window's mode, the window and the
 * game's callbacks, and THRASH_init; if that fails, the window destroyed again.  Once open,
 * driverIdle on a timer if the driver has THRASH_idle, the driver closed at the game's end and
 * before an Error's box.  Whether it is open -- kDriverOpen's byte, as the code hands it back --
 * or nullopt when the game stopped. */
std::optional<reg32> driverOpened(App* app, CPU& cpu, reg32 windowMode)
{
    const reg32 width = at(app, kWantedWidth) == 0 ? kDefaultWidth : at(app, kWantedWidth);
    const reg32 height = at(app, kWantedHeight) == 0 ? kDefaultHeight : at(app, kWantedHeight);
    const auto open = [&]() { return reg32(byte(app, kDriverOpen)); };
    if (open() != 0)
        return open();
    if (at(app, kInstance) == 0)
    {
        const std::optional<reg32> instance = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
        if (!instance)
            return std::nullopt;
        put(app, kInstance, *instance);
    }
    if (at(app, kGameWindow) == 0)
    {
        const std::optional<reg32> made = makeWindow(app, cpu, width, height, windowMode == 0);
        if (!made)
            return std::nullopt;
        if (*made == 0)
            return open();
    }
    if (!setState(app, cpu, thrash::kStateWindowMode, windowMode))
        return std::nullopt;
    const std::optional<reg32> window = game(app, cpu, kAcquireWindow);
    if (!window || !setState(app, cpu, thrash::kStateWindow, *window)
        || !setState(app, cpu, thrash::kStateLambdas, kLambdasRecord))
        return std::nullopt;
    const std::optional<reg32> initialised = thrash(app, cpu, kThrashInit);
    if (!initialised)
        return std::nullopt;
    if (*initialised == 0)
    {
        if (!byAddress(app, cpu, kDestroyWindow))
            return std::nullopt;
        return open();
    }
    if (at(app, kThrashIdle) != 0 && !addTask(app, cpu, kDriverIdle, 1, 1))
        return std::nullopt;
    if (byte(app, kCloseAtExit) == 0)
    {
        if (!atExit(app, cpu, kCloseDriverAtExit))
            return std::nullopt;
        putByte(app, kCloseAtExit, 1);
    }
    put(app, kErrorLock, kCloseDriverForError);
    putByte(app, kDriverOpen, 1);
    return open();
}

/* 0x5246a0: driverOpened, the window's mode eax; eax whether it is open. */
void openDriver(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const std::optional<reg32> open = driverOpened(app, cpu, cpu.eax);
    if (!open)
        return;
    cpu.eax = *open;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The screen's mode ---- */

constexpr reg32 kVideoReady = 0x5644fc;        // set up for modes; without it an Error first
constexpr reg32 kModeSet = 0x564364;           // a mode set, the driver open
constexpr reg32 kWindowedFlags = 0x564390;     // bit 0: the driver opened in a window
constexpr reg32 kModeBuffers = 0x565058;       // two, and two more at 0x565090: sub_509b10's to free
constexpr reg32 kModeBuffersMore = 0x565090;
constexpr reg32 kModeHandle = 0x5643e0;        // sub_4eff00's to let go of
constexpr reg32 kScreens = 0x564394;           // how many screens: past 1 the second set up too
constexpr reg32 kFirstScreen = 0x56502c, kSecondScreen = 0x565064;
constexpr reg32 kScreenSetWidth = 0x565030, kScreenSetHeight = 0x565034, kScreenSetBits = 0x565048;
constexpr reg32 kKeepScreenSize = 0x5247c0;    // eax, edx, ebx: width, height, bits (0 for 640, 480, 16)
constexpr reg32 kLetGo = 0x4eff00;
constexpr reg32 kSetUpScreen = 0x5179d0;       // eax which, ebx its record
constexpr reg32 kRegFlushKey = 0x534468;       // the import
constexpr reg32 kModeAtExit = 0x517580, kModeForError = 0x5175b0;

// native_game's kWhere: the file, the function and the line an Error names.
constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr reg32 kNotReadyFile = 0x5505b0, kNotReadyFunction = 0x5505c0, kNotReadyLine = 0x3b;
constexpr reg32 kNotReady = 0x5505d0;          // the Error's text: modes asked for before they are set up
constexpr reg32 kHkeyClassesRoot = 0x80000000;

/* The driver's mode for a screen `width` (or `height`) wide: THRASH_setvideomode's numbers. */
reg32 driverMode(reg32 width, reg32 height)
{
    switch (width)
    {
    case 0x140: return 4;
    case 0x200: return 6;
    case 0x320: return 2;
    case 0x3c0: return 0xa;
    case 0x358: return 0xb;
    case 0x400: return 0xd;
    default: return height == 0x1f6 ? 0x11 : height == 0x336 ? 0x12 : 1;
    }
}

bool screenSetUp(App* app, CPU& cpu, reg32 which, reg32 window, reg32 record);

/* sub_4eff00, something let go of, by address: eax it. */
bool letGo(App* app, CPU& cpu, reg32 handle)
{
    cpu.eax = handle;
    return byAddress(app, cpu, kLetGo);
}

/* 0x517610's core: the screen set to `width` by `height`, `bits` deep (0 for 640, 480 and 16 bits;
 * -1 for 1024 by 768; 15 bits as 16): the size kept (sub_5247c0); the first time the driver opened
 * -- in a window for bit 0 of [0x564390] -- and what to do at the game's end and before an Error's
 * box set; THRASH_setvideomode given the mode of that width (or of the height, else 1), the
 * buffers of the last mode freed, the screens set up again and what they came to kept, the registry
 * flushed.  1, or 0 when the driver would not open; the registers at each call as the code has
 * them (ebx and edx are left as the last call leaves them). */
std::optional<reg32> videoModeSet(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits)
{
    if (at(app, kVideoReady) == 0)
    {
        put(app, kWhereFile, kNotReadyFile);
        put(app, kWhereFunction, kNotReadyFunction);
        put(app, kWhereLine, kNotReadyLine);
        const GuestFrame deep(cpu, 0x10, 0);  // under the code's four pushes: the Error's vsprintf sees the stack
        if (!error(app, cpu, kNotReady))
            return std::nullopt;
    }
    width = width == 0 ? kDefaultWidth : width == 0xffffffff ? 0x400 : width;
    height = height == 0 ? kDefaultHeight : height == 0xffffffff ? 0x300 : height;
    bits = bits == 0 || bits == 0xffffffff || bits == 0xf ? 0x10 : bits;
    cpu.eax = width;
    cpu.edx = height;
    cpu.ebx = bits;
    if (!byAddress(app, cpu, kKeepScreenSize))
        return std::nullopt;
    if (at(app, kModeSet) == 0)
    {
        const Kept kept(cpu, kEbx | kEcx | kEdx);  // as openDriver keeps them
        const std::optional<reg32> open = driverOpened(app, cpu, byte(app, kWindowedFlags) & 1 ? 0 : 1);
        if (!open)
            return std::nullopt;
        kept.back(cpu);
        if (*open == 0)
            return 0;
        cpu.ebx = kModeForError;
        if (!atExit(app, cpu, kModeAtExit))
            return std::nullopt;
        put(app, kErrorLock, kModeForError);
    }
    if (!thrash(app, cpu, kThrashSetVideoMode, {driverMode(width, height), 2, 1}))
        return std::nullopt;
    for (const reg32 buffer : {kModeBuffers, kModeBuffers + 4})
        if (at(app, buffer) != 0 && !freeBlock(app, cpu, at(app, buffer)))
            return std::nullopt;
    const reg32 more = at(app, kModeBuffersMore);
    put(app, kModeBuffers, 0);
    put(app, kModeBuffers + 4, 0);
    if (more != 0 && !freeBlock(app, cpu, more))
        return std::nullopt;
    if (at(app, kModeBuffersMore + 4) != 0 && !freeBlock(app, cpu, at(app, kModeBuffersMore + 4)))
        return std::nullopt;
    const reg32 handle = at(app, kModeHandle);
    put(app, kModeBuffersMore, 0);
    put(app, kModeBuffersMore + 4, 0);
    if (handle != 0)
    {
        if (!letGo(app, cpu, handle))
            return std::nullopt;
        put(app, kModeHandle, 0);
    }
    put(app, kModeSet, 1);
    const auto screen = [&](reg32 which, reg32 record) {
        cpu.eax = which;
        cpu.edx = 0;
        cpu.ebx = record;
        return screenSetUp(app, cpu, which, 0, record);
    };
    if (!screen(1, kFirstScreen) || (sreg32(at(app, kScreens)) > 1 && !screen(2, kSecondScreen)))
        return std::nullopt;
    put(app, kWantedWidth, at(app, kScreenSetWidth));
    put(app, kWantedHeight, at(app, kScreenSetHeight));
    cpu.eax = byte(app, kScreenSetBits);
    put(app, kScreenBits, cpu.eax);
    if (!guest::windows<kRegFlushKey, &win32::advapi32::RegFlushKey>(app, cpu, kHkeyClassesRoot))
        return std::nullopt;
    return 1;
}

/* 0x517610: videoModeSet, eax by edx, ebx bits. */
void setVideoMode(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    const std::optional<reg32> set = videoModeSet(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!set)
        return;
    cpu.eax = *set;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The game's end ---- */

constexpr reg32 kFpuControl = 0x79f3f4;        // the x87's control word as the game runs it
constexpr reg32 kStopSounds = 0x410aa0, kStopMoreSounds = 0x40feb0;  // native_frontend_options' kStopSound2, 4
constexpr reg32 kNothingToEnd = 0x4a6f70;      // an empty function
constexpr reg32 kGameThread = 0x7a3aac;
constexpr reg32 kReleaseWindowThing = 0x4cdc50;

/* ea::endThread (0x4df870), a thread ended (TerminateThread, CloseHandle): its record in eax, the
 * stale handle the code's ebx; eax as it leaves it. */
bool endThread(App* app, CPU& cpu, reg32 thread)
{
    const std::optional<reg32> closed = guest::direct<0x4df870, ea::endThread>(app, cpu, thread, cpu.ebx);
    if (!closed)
        return false;
    cpu.eax = *closed;
    return true;
}

/* native_controls_race's every device's actuators off (0x471fb0), and native_controls_input's
 * DirectInput stopped (0x434fd0): eax as the first leaves it, then as the code has it. */
bool inputLetGo(App* app, CPU& cpu)
{
    const std::optional<reg32> off = guest::direct<0x471fb0, controls::actuatorsOff>(app, cpu, cpu.eax);
    if (!off)
        return false;
    cpu.eax = *off;
    return guest::direct<0x434fd0, controls::stopInput>(app, cpu);
}

/* 0x4b6250's core: what the game does as it ends: the x87 reset to its own control word, the
 * sounds stopped, the thread at 0x7a3aac ended, sub_4cdc50's release, the force feedback off,
 * DirectInput stopped and the screen given back to Windows (THRASH_restore).  eax as the last
 * call leaves it. */
bool gameEnded(App* app, CPU& cpu)
{
    cpu.fpu.init();
    cpu.fpu.setControl(half(app, kFpuControl));
    return byAddress(app, cpu, kStopSounds) && byAddress(app, cpu, kStopMoreSounds)
        && byAddress(app, cpu, kNothingToEnd) && endThread(app, cpu, kGameThread)
        && byAddress(app, cpu, kReleaseWindowThing) && inputLetGo(app, cpu) && thrash(app, cpu, kThrashRestore);
}

/* 0x4b6250: gameEnded. */
void shutDown(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    if (!gameEnded(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The loading screen ---- */

constexpr reg32 kLoadingTextShown = 0x79f288;  // the loading text is up; cleared once "Aborted" is
constexpr reg32 kLoadingTextX = 0x79f280;      // where the loading text ends, a float (native_render)
constexpr reg32 kModeRect = 0x7cdac0;          // the mode's rectangle: x, y, width, height
constexpr reg32 kGetRect = 0x4beb20;           // eax a rectangle into edx, ebx, ecx and [esp+4]
constexpr reg32 kTextScale = 0x791b54;
constexpr reg32 kSetTextScale = 0x4d1390;      // [esp+4] the scale, the last one onto the x87's stack
constexpr reg32 kTextColour = 0x561b34, kTextMeasuring = 0x561b3c;  // port_text's
constexpr reg32 kLanguageText = 0x4d1850;      // port_text's languageText
constexpr reg32 kAbortedLine = 0xeb;           // "Aborted"
constexpr reg32 kDotsFormat = 0x53bdc8;        // "... %s"
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kHeightPer480 = 0x53bdd0;      // 1/480, a float
constexpr reg32 kAbortedRow = 0x53bdd4;        // 451, a float: the row on a 480 line screen
constexpr reg32 kChop = 0x4dfd56;              // Watcom's __CHP: st(0) towards zero
constexpr reg32 kDrawText = 0x4d1000;          // port_text's drawText, its width onto the x87's stack
constexpr reg32 kFlushText = 0x4d1340;

/* sub_4beb20, a rectangle copied out, by address: eax the rectangle, its x, y, width and height into
 * [edx], [ebx], [ecx] and [[esp+4]] -- that one pushed, and esp left as the callee leaves it, as the
 * code leaves it. */
bool copyRect(App* app, CPU& cpu, reg32 rect, reg32 x, reg32 y, reg32 width, reg32 height)
{
    cpu.eax = rect;
    cpu.edx = x;
    cpu.ebx = y;
    cpu.ecx = width;
    push(app, cpu, height);
    return byAddress(app, cpu, kGetRect);
}

/* Watcom's __CHP (0x4dfd56) on `value`, as C++ (watcom::clib::chop): chopped towards 0 and stored as
 * an integer (fistp).  nullopt when the game stopped. */
std::optional<sreg32> chop(App* app, CPU& cpu, x86::Float value)
{
    const double whole = guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value));
    if (cpu.terminate)
        return std::nullopt;
    return fp::toInteger<sreg32>(whole);
}

/* port_text's text scale set (0x4d1390, stdcall: port::text::textScaleSet): the last scale, which
 * the code drops (fstp st(0)). */
bool setTextScale(App* app, CPU& cpu, reg32 scale)
{
    guest::direct<kSetTextScale, port::text::textScaleSet>(app, cpu, scale);
    return !cpu.terminate;
}

/* port_text's languageText (0x4d1850: port::text::lineText): line eax of the game's text, edx handed
 * on as the code has it. */
std::optional<reg32> languageText(App* app, CPU& cpu, reg32 line)
{
    cpu.eax = line;
    const port::Result text =
        guest::direct<kLanguageText, port::text::lineText>(app, cpu, cpu.esp - 4, sreg32(line), cpu.edx);
    if (!text || cpu.terminate)
        return std::nullopt;
    cpu.eax = reg32(*text);
    return cpu.eax;
}

/* port_text's drawText (0x4d1000: port::text::textDrawn): text eax at x edx, row ebx, as the code
 * has them; its width, which the code drops (fstp st(0)), and eax as it leaves it. */
bool drawText(App* app, CPU& cpu, reg32 text, reg32 x, reg32 row)
{
    cpu.eax = text;
    cpu.edx = x;
    cpu.ebx = row;
    const port::text::WidthResult drawn =
        guest::direct<kDrawText, port::text::textDrawn>(app, cpu, cpu.esp - 4, text, sreg32(x), sreg32(row));
    if (!drawn || cpu.terminate)
        return false;
    cpu.eax = drawn->eax;
    return true;
}

constexpr reg32 kAbortedColour = 0xffe0e000;   // yellow

/* 0x494bd0's core: a loading cut short: when the loading text is up, "... Aborted" drawn after it
 * -- in yellow, at row 451 of 480 scaled to the mode -- and the picture flushed and flipped; the
 * loading text then counts as gone.  The rectangle's and the text's buffers on the guest's stack, as
 * deep as the code has them; the registers at the calls as it has them (ecx 0 from the text's
 * measuring on).  eax as the last call leaves it. */
bool loadingAborted(App* app, CPU& cpu)
{
    if (at(app, kLoadingTextShown) == 0)
        return true;
    const GuestFrame frame(cpu, 0x10, 0x118);
    const reg32 text = frame - 0x118;
    if (!copyRect(app, cpu, kModeRect, frame - 8, frame - 0x18, frame - 0x14, frame - 0x10))
        return false;
    const reg32 height = at(app, frame - 0x10);
    const float scale = float(fp::mul(x86::Float(sreg32(height)), x86::Float(floatAt(app, kHeightPer480))));
    put(app, kLoadingTextShown, 0);
    cpu.eax = height;
    cpu.ecx = 0;
    if (!guest::direct<0x431aa0, collide::textStates>(app, cpu))
        return false;
    cpu.ebx = kAbortedColour;
    if (!setTextScale(app, cpu, at(app, kTextScale)))
        return false;
    put(app, kTextColour, kAbortedColour);
    put(app, kTextMeasuring, 0);
    const std::optional<reg32> aborted = languageText(app, cpu, kAbortedLine);
    if (!aborted)
        return false;
    cpu.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, text, kDotsFormat, guest::Words{*aborted});
    if (cpu.terminate)
        return false;
    const x86::Float row = fp::mul(x86::Float(scale), x86::Float(floatAt(app, kAbortedRow)));
    cpu.eax = text;
    const std::optional<sreg32> x = chop(app, cpu, x86::Float(floatAt(app, kLoadingTextX)));
    if (!x)
        return false;
    cpu.edx = reg32(*x);
    const std::optional<sreg32> y = chop(app, cpu, row);
    return y && drawText(app, cpu, text, reg32(*x), reg32(*y)) && byAddress(app, cpu, kFlushText)
        && flushFlip(app, cpu);
}

/* 0x494bd0: loadingAborted. */
void drawLoadingAborted(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!loadingAborted(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- A race taken down ---- */

constexpr reg32 kMenuScreenMode = 0x55fec8;    // the screen mode the menus run in
constexpr reg32 kInRace = 0x559234;            // native_frontend_screen's: the menus are a race's pause screens
constexpr reg32 kScreenMode = 0x4bef50;        // native_render's screenMode: eax the mode
constexpr reg32 kMenuView = 0x7cdb68;
constexpr reg32 kSetViewAngle = 0x4bed50;      // eax a view, edx 90
constexpr reg32 kSetViewScale = 0x4bed70;      // eax a view, ecx 1.0f
constexpr reg32 kViewScale = 0x560094, kViewOffset = 0x560098;
constexpr reg32 kRaceEndExtra = 0x495600;      // for eax nonzero
// The race's own ends by address: before raceload's controls' tick stopped (0x42dd00), and after it
// up to the track let go of (0x41da60).
constexpr reg32 kRaceEndsFirst[] = {0x4bc950, 0x4bcd60}, kRaceEndsThen[] = {0x478000, 0x4c4830, 0x4782d0};

}  // namespace graphics

void raceRunning(bool running);  // nfs3hp_main.cpp: the port's mark of a race (tools/apply_race_state.py)

namespace graphics
{

/* ---- What the later sections share ---- */

constexpr reg32 kModeWidth = 0x7cdac8;
constexpr reg32 kTextWidth = 0x4d1480;         // port_text's textWidthWithOverhang: onto the x87's stack
constexpr reg32 kTextHeight = 0x4d1540;        // eax a font: its height
constexpr reg32 kTraceWord = 4;                // raceload::noHook's (the game's trace): a word pushed

/* raceload::noHook (0x42d500, the game's trace), its word `word` pushed as the code pushes it (it
 * reads none).  False when the game stopped. */
bool traced(App* app, CPU& cpu, reg32 word)
{
    const GuestFrame pushed(cpu, 0, kTraceWord);
    put(app, pushed - kTraceWord, word);
    guest::direct<0x42d500, raceload::noHook>(app, cpu);
    return !cpu.terminate;
}

namespace
{

/* movsd: a dword from [esi] to es:[edi], both stepped by the direction flag. */
void movsd(win32::WinApplication* app, x86::CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
    cpu.edi += step;
    cpu.esi += step;
}

}  // namespace

/* The loading bar's quad: its corners' clip codes into their +0x14, as the menus' quads work them
 * out: 0x10 for a +0xc negative or behind the far edge, else 8 above, 4 below, 1 left of, 2 right of the
 * rectangle -- each a float's bits compared as integers, a negative one counting as short. */
constexpr reg32 kClipFar = 0x7d34f8, kClipTopEdge = 0x7d3504, kClipBottomEdge = 0x7d3500;
constexpr reg32 kClipLeftEdge = 0x7d34fc, kClipRightEdge = 0x7d350c;  // a float's bits, compared as integers
constexpr reg32 kCornerClipCode = 0x14;

/* Whether a float's bits are short of `edge`'s, compared as integers: a negative one always is. */
bool shortOf(reg32 bits, reg32 edge)
{
    return (bits & 0x80000000) || sreg32(bits) < sreg32(edge);
}

void putClipCode(App* app, reg32 corner)
{
    const reg32 w = at(app, corner + 0xc);
    reg8 code = 0;
    if ((w & 0x80000000) || sreg32(w) >= sreg32(at(app, kClipFar)))
        code = 0x10;
    else
    {
        const reg32 y = at(app, corner + 4);
        if (shortOf(y, at(app, kClipTopEdge)))
            code |= 8;
        else if (sreg32(y) > sreg32(at(app, kClipBottomEdge)))
            code |= 4;
        const reg32 x = at(app, corner);
        if (shortOf(x, at(app, kClipLeftEdge)))
            code |= 1;
        else if (sreg32(x) > sreg32(at(app, kClipRightEdge)))
            code |= 2;
    }
    putByte(app, corner + kCornerClipCode, code);
}

constexpr reg32 kMenuViewAngle = 90;
constexpr reg32 kMenuViewScale = 0x3f800000;   // 1.0f
constexpr reg32 kPointerWidth = 640, kPointerHeight = 480;

/* 0x4a35d0's core: the race taken down, back to the menus: their screen mode, the menus' view at
 * 90 and scale 1, the pointer kept to 640 by 480, both buffers cleared to black, sub_495600 for
 * `extra` nonzero, and the race's own ends (sub_4bc950 on).  The port marks the race over first,
 * as tools/apply_race_state.py did in the generated code.  The registers at each call as the code
 * has them; eax as the last end leaves it. */
bool raceTakenDown(App* app, CPU& cpu, reg32 extra)
{
    if (diag::raceDown)
        diag::raceDown(app, "race taken down");
    raceRunning(false);
    put(app, kInRace, 0);
    cpu.eax = at(app, kMenuScreenMode);
    cpu.edx = 0;
    cpu.ebx = 0;
    if (!byAddress(app, cpu, kScreenMode))
        return false;
    cpu.eax = kMenuView;
    cpu.edx = kMenuViewAngle;
    if (!byAddress(app, cpu, kSetViewAngle))
        return false;
    cpu.eax = kMenuView;
    cpu.ecx = kMenuViewScale;
    if (!byAddress(app, cpu, kSetViewScale))
        return false;
    put(app, kViewScale, kMenuViewScale);
    put(app, kViewOffset, 0);
    cpu.eax = 0;
    cpu.edx = 0;
    cpu.ebx = kPointerWidth;
    cpu.ecx = kPointerHeight;
    if (!guest::direct<0x4352d0, controls::boundPointer>(app, cpu, reg32(0), reg32(0), kPointerWidth, kPointerHeight)
        || !sendState(app, cpu, thrash::kStateClearColour, 0))
        return false;
    for (const reg32 buffer : {2u, 1u})
        if (!thrash(app, cpu, kThrashWindow, {buffer}) || !clearDepth(app, cpu))
            return false;
    if (extra != 0 && !byAddress(app, cpu, kRaceEndExtra))
        return false;
    const auto after = [&](reg32 end) {
        if (diag::raceDown)
        {
            char where[32];
            std::snprintf(where, sizeof where, "after sub_%x", unsigned(end));
            diag::raceDown(app, where);
        }
    };
    for (const reg32 end : kRaceEndsFirst)
    {
        if (!byAddress(app, cpu, end))
            return false;
        after(end);
    }
    if (!guest::direct<0x42dd00, raceload::stopTick>(app, cpu))
        return false;
    after(0x42dd00);
    for (const reg32 end : kRaceEndsThen)
    {
        if (!byAddress(app, cpu, end))
            return false;
        after(end);
    }
    const std::optional<reg32> left = guest::direct<0x41da60, track::letGoOfTrack>(app, cpu);
    if (!left)
        return false;
    cpu.eax = *left;
    after(0x41da60);
    return true;
}

/* 0x4a35d0: raceTakenDown, sub_495600 for eax nonzero. */
void takeDownRace(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!raceTakenDown(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The polygon lists' drawers for voodoo2a ---- */

constexpr reg32 kDrawLine = 0x4c0b20;          // native_render's line: eax, edx its ends
constexpr reg32 kDrawQuad = 0x4c12e0;          // native_render's polygon: corners eax, edx, ebx, ecx

// A polygon list's record: +0 the next, +6 its flags (bit 0 blended), +8.. its corners (vertices),
// +0x18 its texture, +0x20.. the corners' texture coordinates (two floats each), +0x40.. their
// colours.  A vertex's +0x10 its colour, +0x18 and +0x1c its texture coordinates.
constexpr reg32 kRecordNext = 0, kRecordFlags = 6, kRecordCorners = 8, kRecordTexture = 0x18;
constexpr reg32 kRecordCoordinates = 0x20, kRecordColours = 0x40;
constexpr reg32 kVertexColour = 0x10, kVertexCoordinates = 0x18;

/* A kept state sent as the drawers send it: kept at `keptAt`, then THRASH_setstate. */
bool sendKept(App* app, CPU& cpu, reg32 keptAt, reg32 state, reg32 value)
{
    put(app, keptAt, value);
    return setState(app, cpu, state, value).has_value();
}

/* native_render's line (0x4c0b20), by address: its ends in eax and edx. */
bool drawLine(App* app, CPU& cpu, reg32 from, reg32 to)
{
    cpu.eax = from;
    cpu.edx = to;
    return byAddress(app, cpu, kDrawLine);
}

/* 0x4347d0's core: a line of a polygon list (its two ends the record's first two corners) drawn
 * untextured, blended for bit 0 of its flags. */
bool lineRecord(App* app, CPU& cpu, reg32 record)
{
    const reg32 to = at(app, record + kRecordCorners + 4), from = at(app, record + kRecordCorners);
    if (at(app, kKeptTexture) != 0 && !sendKept(app, cpu, kKeptTexture, thrash::kStateTexture, 0))
        return false;
    const reg32 blend = byte(app, record + kRecordFlags) & 1;
    if (at(app, kKeptBlend) != blend && !sendKept(app, cpu, kKeptBlend, thrash::kStateBlend, blend))
        return false;
    return drawLine(app, cpu, from, to);
}

/* 0x4347d0: lineRecord for the record in eax; eax the next record. */
void drawLineRecord(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const reg32 record = cpu.eax;
    if (!lineRecord(app, cpu, record))
        return;
    cpu.eax = at(app, record + kRecordNext);
    kept.back(cpu);
    cpu.esp += 4;
}

/* native_render's polygon (0x4c12e0), by address: its four corners in eax, edx, ebx, ecx. */
bool drawQuad(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third, reg32 fourth)
{
    cpu.eax = first;
    cpu.edx = second;
    cpu.ebx = third;
    cpu.ecx = fourth;
    return byAddress(app, cpu, kDrawQuad);
}

/* 0x433d40's core: a quad of a polygon list with colours of its own, drawn unblended: each
 * corner's colour, and with a texture its coordinates, copied to its vertex, the texture sent when
 * it changed (or none when there is none), then native_render's polygon.  Each field read where the
 * code reads it: a vertex may be the record's own memory. */
bool colouredQuadRecord(App* app, CPU& cpu, reg32 record)
{
    if (at(app, kKeptBlend) != 0 && !sendKept(app, cpu, kKeptBlend, thrash::kStateBlend, 0))
        return false;
    const auto vertex = [&](reg32 corner) { return at(app, record + kRecordCorners + 4 * corner); };
    for (reg32 corner = 0; corner < 4; ++corner)
    {
        const reg32 to = vertex(corner);
        put(app, to + kVertexColour, at(app, record + kRecordColours + 4 * corner));
    }
    if (at(app, record + kRecordTexture) != 0)
    {
        for (reg32 corner = 0; corner < 4; ++corner)
            for (reg32 half = 0; half < 2; ++half)
            {
                const reg32 to = vertex(corner);
                put(app, to + kVertexCoordinates + 4 * half,
                    at(app, record + kRecordCoordinates + 8 * corner + 4 * half));
            }
        const reg32 kept = at(app, kKeptTexture), texture = at(app, record + kRecordTexture);
        if (kept != texture && !sendKept(app, cpu, kKeptTexture, thrash::kStateTexture, texture))
            return false;
    }
    else if (at(app, kKeptTexture) != 0 && !sendKept(app, cpu, kKeptTexture, thrash::kStateTexture, 0))
        return false;
    const reg32 fourth = vertex(3), third = vertex(2), second = vertex(1), first = vertex(0);
    return drawQuad(app, cpu, first, second, third, fourth);
}

/* 0x433d40: colouredQuadRecord for the record in eax; eax the next record. */
void drawColouredQuadRecord(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const reg32 record = cpu.eax;
    if (!colouredQuadRecord(app, cpu, record))
        return;
    cpu.eax = at(app, record + kRecordNext);
    kept.back(cpu);
    cpu.esp += 4;
}

// A mesh of lines gathered: a count of ends, the ends from +4, the pairs' indices from +0x8004.
constexpr reg32 kMeshEnds = 4, kMeshPairs = 0x8004;

/* 0x49bf60's core: the mesh of lines gathered at `mesh` drawn untextured (THRASH_drawlinemesh) and
 * its count set back to 0, when there is one.  eax at the driver's call as the code has it: the
 * lines' count. */
bool lineMeshFlushed(App* app, CPU& cpu, reg32 mesh)
{
    if (at(app, mesh) == 0)
        return true;
    if (!sendState(app, cpu, thrash::kStateTexture, 0))
        return false;
    const reg32 lines = reg32(sreg32(at(app, mesh)) / 2);  // the code's sar of it less its sign: towards 0
    cpu.eax = lines;
    if (!thrash(app, cpu, kThrashDrawLineMesh, {lines, mesh + kMeshEnds, mesh + kMeshPairs}))
        return false;
    put(app, mesh, 0);
    return true;
}

/* 0x49bf60: lineMeshFlushed for the mesh at eax. */
void flushLineMesh(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!lineMeshFlushed(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- Textures ---- */

constexpr reg32 kNoTextureFormat = 0x53c5f0;   // the Error's text: the driver takes neither format
constexpr reg32 kTextureBufferTag = 0x53c60c;  // what the conversion's buffer is allocated as

// THRASH_about's record: +0x30 the texture formats it takes, each with its flags (bit 0: taken).
constexpr reg32 kAboutFormats = 0x30, kFormat1555Flags = 0xc, kFormat4444Flags = 0x1c;
constexpr reg32 kFormat1555 = 3, kFormat4444 = 7;

/* 0x49c5c0's core: the format the game's 16-bit textures are handed to the driver in: 3 (1555)
 * when THRASH_about's formats take it, else 7 (4444), with an Error first when they take neither
 * (eax the formats there, as the code has it).  ecx and edx kept and eax the format, as the code
 * leaves them. */
std::optional<reg32> textureFormat(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx | kEdx);
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return std::nullopt;
    const reg32 formats = at(app, *about + kAboutFormats);
    if (byte(app, formats + kFormat1555Flags) & 1)
    {
        cpu.eax = kFormat1555;
        kept.back(cpu);
        return kFormat1555;
    }
    cpu.eax = formats;
    if (!(byte(app, formats + kFormat4444Flags) & 1))
    {
        // As deep as the code calls the Error, under its three pushes: it hands vsprintf an
        // address on the stack.
        const GuestFrame deep(cpu, 0xc, 0);
        if (!error(app, cpu, kNoTextureFormat))
            return std::nullopt;
    }
    cpu.eax = kFormat4444;
    kept.back(cpu);
    return kFormat4444;
}

/* 0x49c5c0: textureFormat. */
void textureFormatFor(App* app, CPU& cpu)
{
    if (textureFormat(app, cpu))
        cpu.esp += 4;
}

/* The runtime's allocator (ea::reservememadr, 0x4e1620): eax the tag, edx the bytes, ebx 0x10 --
 * which it keeps, the code's after it.  The block. */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 tag, reg32 bytes)
{
    cpu.edx = bytes;
    cpu.ebx = 0x10;
    return guest::direct<0x4e1620, ea::reserved>(app, cpu, tag, bytes, reg32(0x10));
}

/* The runtime's free (ea::purgememadr, 0x4e1890): the block in eax. */
bool release(App* app, CPU& cpu, reg32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block).has_value();
}

/* A 1555 pixel as 4444: 0 for a transparent one (bit 15 clear), else each colour's top four bits
 * and the alpha's.  The code's shifts are of the pixel sign-extended. */
reg16 pixel4444(sreg32 pixel)
{
    if ((pixel >> 15) == 0)
        return 0;
    return reg16(reg32(pixel >> 3 & 0xff00) | reg32(pixel >> 2 & 0xf0f0) | reg32(pixel >> 1 & 0xf00f));
}

/* 0x49c620's core: `texture` given the `width` by `height` 1555 pixels at `pixels`: straight
 * (THRASH_tupdate) when the driver takes 1555, else turned into 4444 in a block of their own first.
 * textureFormat gives one of the two only: the code's Error for another format is never reached.
 * ebx, ecx and edx at the driver's call as the conversion leaves them; eax as the last call
 * leaves it. */
bool textureUpdated(App* app, CPU& cpu, reg32 texture, reg32 pixels, reg32 width, reg32 height)
{
    std::optional<reg32> format;
    {
        const GuestFrame deep(cpu, 0x20, 0);  // the code's pushes, its frame and the call's slot, for the Error's
        format = textureFormat(app, cpu);
    }
    if (!format)
        return false;
    if (*format == kFormat1555)
        return thrash(app, cpu, kThrashTUpdate, {texture, pixels, 0}).has_value();
    const reg32 count = width * height;
    const std::optional<reg32> block = allocate(app, cpu, kTextureBufferTag, count + count);
    if (!block)
        return false;
    reg32 done = 0;
    for (; sreg32(done) < sreg32(count); ++done)
        putHalf(app, *block + 2 * done, pixel4444(x86::sreg16(half(app, pixels + 2 * done))));
    cpu.ebx = *block;
    cpu.ecx = done;
    cpu.edx = *block + 2 * done;
    return thrash(app, cpu, kThrashTUpdate, {texture, *block, 0}) && release(app, cpu, *block);
}

/* 0x49c620: textureUpdated, texture eax, pixels edx, ebx by ecx of them. */
void updateTexture(App* app, CPU& cpu)
{
    if (textureUpdated(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx))
        cpu.esp += 4;
}

constexpr reg32 kMemoryWarnings = 0x4b55f8, kMemoryWarningNames = 0x4b5610;  // six words each
constexpr reg32 kMemoryWarningFormat = 0x53fde0;
constexpr reg32 kStandardError = 0x566f72;     // the runtime's stderr
constexpr reg32 kFprintf = 0x4f4b80;
constexpr reg32 kDriverDirect3D = 6;
// THRASH_about's record: +0x70 the texture memory, +0x74 Direct3D's more.
constexpr reg32 kAboutTextureMemory = 0x70, kAboutMoreMemory = 0x74;
constexpr sreg32 kEnoughTextureMemory = 0x200000;

/* 0x4b5ea0's core: for driver type `type` (Direct3D counts THRASH_about's more memory too), a line
 * to stderr when the texture memory is under 2 MB, with the words of the tables at 0x4b55f8 and
 * 0x4b5610 for `index` -- copied into the frame first and read there, as deep as the code has them
 * (an index past them reads what is there).  edx the driver's record, as the code leaves it.  What
 * eax is left: the memory, or fprintf's answer. */
std::optional<reg32> textureMemoryWarned(App* app, CPU& cpu, reg32 type, reg32 index)
{
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return std::nullopt;
    cpu.edx = *about;
    reg32 memory = at(app, *about + kAboutTextureMemory);
    if (type == kDriverDirect3D)
        memory += at(app, *about + kAboutMoreMemory);
    if (sreg32(memory) >= kEnoughTextureMemory)
        return memory;
    const GuestFrame frame(cpu, 0x14, 0x30);
    for (reg32 i = 0; i < 6; ++i)
    {
        put(app, frame - 0x30 + 4 * i, at(app, kMemoryWarnings + 4 * i));
        put(app, frame - 0x18 + 4 * i, at(app, kMemoryWarningNames + 4 * i));
    }
    const reg32 warning = at(app, (frame - 0x30) + index * 4), name = at(app, (frame - 0x18) + index * 4);
    cpu.eax = memory;
    const reg32 printed = guest::direct<kFprintf, watcom::clib::fprintf>(app, cpu, kStandardError, kMemoryWarningFormat,
                                                                         guest::Words{name, memory, warning});
    if (cpu.terminate)
        return std::nullopt;
    cpu.eax = printed;
    return printed;
}

/* 0x4b5ea0: textureMemoryWarned for driver type eax, index edx. */
void warnTextureMemory(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    const std::optional<reg32> left = textureMemoryWarned(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = *left;
    kept.back(cpu);
    cpu.esp += 4;
}

// THRASH_about's record: +0x3c the last mode, +0x40 the modes (0x28 bytes each: width, height...).
constexpr reg32 kAboutLastMode = 0x3c, kAboutModes = 0x40, kModeBytes = 0x28;

/* What modeSizeOf found: THRASH_about's record, whether the mode is one of its, and its size. */
struct ModeSize
{
    reg32 about;
    bool listed;
    reg32 width, height;
};

/* 0x4be570's core: screen mode `mode`'s width into [widthAt] and height into [heightAt] from
 * THRASH_about's modes, 640 by 480 past the last; [zeroAt] and [alsoZeroAt] set to 0 first. */
std::optional<ModeSize> modeSizeOf(App* app, CPU& cpu, reg32 mode, reg32 zeroAt, reg32 alsoZeroAt, reg32 widthAt,
                                   reg32 heightAt)
{
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return std::nullopt;
    put(app, zeroAt, 0);
    put(app, alsoZeroAt, 0);
    if (sreg32(mode) > sreg32(at(app, *about + kAboutLastMode)))
    {
        put(app, widthAt, kDefaultWidth);
        put(app, heightAt, kDefaultHeight);
        return ModeSize{*about, false, kDefaultWidth, kDefaultHeight};
    }
    const reg32 width = at(app, at(app, *about + kAboutModes) + mode * kModeBytes);
    put(app, widthAt, width);
    const reg32 height = at(app, at(app, *about + kAboutModes) + mode * kModeBytes + 4);
    put(app, heightAt, height);
    return ModeSize{*about, true, width, height};
}

/* 0x4be570: modeSizeOf for mode eax, the zeros at edx and ebx, the width into [ecx], the height into
 * [esp+4]; the registers as the code leaves them. */
void modeSize(App* app, CPU& cpu)
{
    const reg32 widthAt = cpu.ecx, heightAt = at(app, cpu.esp + 4);
    const std::optional<ModeSize> size = modeSizeOf(app, cpu, cpu.eax, cpu.edx, cpu.ebx, widthAt, heightAt);
    if (!size)
        return;
    cpu.ebx = widthAt;
    if (size->listed)
    {
        cpu.eax = size->height;
        cpu.ecx = size->width;
        cpu.edx = heightAt;
    }
    else
    {
        cpu.eax = heightAt;
        cpu.edx = size->about;
    }
    cpu.esp += 4 + 4;
}

/* ---- Text ---- */

constexpr reg32 kTextQuad = 0x561b44;          // four corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr reg32 kTextBoxColour = 0x561b38;
constexpr reg32 kFontHeights = 0x83f6a0, kFont = 0x840184, kFontScale = 0x84018c;

/* A float's bits as the float. */
float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* 0x4d1120's core: a box drawn untextured behind text: at x, y, `width` wide (times the font's
 * scale) and the font's height (its table at 0x83f6a0 for [0x840184], times the scale) high, in
 * [0x561b38] made opaque.  The x87's operations as the code does them, each float stored where it
 * stores one. */
bool textBox(App* app, CPU& cpu, sreg32 x, sreg32 y, float width)
{
    const reg32 font = at(app, kFont);
    const x86::Float scale = x86::Float(floatAt(app, kFontScale));
    const x86::Float scaledWidth = fp::mul(x86::Float(width), scale);
    const x86::Float height = fp::mul(scale, x86::Float(sreg32(at(app, kFontHeights + font * 4))));
    const reg32 colour = at(app, kTextBoxColour) | 0xff000000;
    putFloat(app, kTextQuad, float(x86::Float(x)));
    putFloat(app, kTextQuad + 4, float(x86::Float(y)));
    const reg32 left = at(app, kTextQuad);
    putFloat(app, kTextQuad + 0x24, float(x86::Float(y)));
    for (reg32 corner = 3; corner < 4; --corner)
        put(app, kTextQuad + corner * kCornerBytes + kCornerColour, colour);
    const x86::Float top = x86::Float(floatAt(app, kTextQuad + 0x24));
    put(app, kTextQuad + 0x60, left);
    const x86::Float right = fp::add(scaledWidth, x86::Float(floatOf(left)));
    const x86::Float bottom = fp::add(height, top);
    putFloat(app, kTextQuad + 0x20, float(right));
    putFloat(app, kTextQuad + 0x44, float(bottom));
    put(app, kTextQuad + 0x40, at(app, kTextQuad + 0x20));
    putFloat(app, kTextQuad + 0x64, float(bottom));
    const auto corner = [](reg32 n) { return kTextQuad + n * kCornerBytes; };
    return sendState(app, cpu, thrash::kStateTexture, 0)
        && thrash(app, cpu, kThrashDrawQuad, {corner(0), corner(1), corner(2), corner(3)});
}

/* 0x4d1120: textBox at x eax, y edx, [esp+4] wide. */
void drawTextBox(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!textBox(app, cpu, sreg32(cpu.eax), sreg32(cpu.edx), floatAt(app, cpu.esp + 4)))
        return;
    kept.back(cpu);
    cpu.esp += 4 + 4;
}

/* ---- The menus' art in textures ---- */

constexpr reg32 kShapeBufferTag = 0x5495c4;
constexpr reg32 kShapeToPixels = 0x4d37a0;     // eax the buffer, edx the shape, ebx and [esp+4] where its size goes

/* port::shapes' shape into pixels (0x4d37a0), by address: eax the buffer, edx the shape, ebx and
 * [esp+4] where its size goes (ecx as the code has it, which the stand-in logs); esp left as the
 * callee leaves it, as the code leaves it. */
bool shapeToPixels(App* app, CPU& cpu, reg32 buffer, reg32 shape, reg32 widthAt, reg32 heightAt)
{
    cpu.eax = buffer;
    cpu.edx = shape;
    cpu.ebx = widthAt;
    push(app, cpu, heightAt);
    return byAddress(app, cpu, kShapeToPixels);
}

/* 0x4d5ed0's core: `texture` filled from `shape` (`size` by `size` pixels at most): the shape turned
 * into pixels (sub_4d37a0) in a block of its own -- its size into the frame, as deep as the code has
 * it, the height's slot holding `limit` first -- handed to THRASH_tupdate from its +0x10, and the
 * block freed.  ebx left the width's slot, as the code leaves it. */
bool shapeIntoTextureOf(App* app, CPU& cpu, reg32 texture, reg32 shape, reg32 size, reg32 limit)
{
    const GuestFrame frame(cpu, 0xc, 0x10);
    const reg32 heightAt = frame - 0x10, widthAt = frame - 0xc;
    put(app, heightAt, limit);
    cpu.ecx = size;
    const std::optional<reg32> block = allocate(app, cpu, kShapeBufferTag, ((size * size) << 2) + 0x14);
    if (!block || !shapeToPixels(app, cpu, *block, shape, widthAt, heightAt))
        return false;
    cpu.eax = *block + 0x10;
    cpu.edx = texture;
    return thrash(app, cpu, kThrashTUpdate, {texture, *block + 0x10, 0}) && release(app, cpu, *block);
}

/* 0x4d5ed0: shapeIntoTextureOf, texture eax, shape edx, ebx by ebx at most, ecx into the height's
 * slot. */
void shapeIntoTexture(App* app, CPU& cpu)
{
    if (shapeIntoTextureOf(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx))
        cpu.esp += 4;
}

/* ---- The screen's functions, the frame buffer, the driver closed ---- */

constexpr reg32 kScreenFunctions = 0x567dc4;   // five: at exit, set mode, lock, unlock and flip, set up a screen
constexpr reg32 kLockScreen = 0x517920;
constexpr reg32 kReleaseAndFlip = 0x517990, kSetVideoMode = 0x517610;
constexpr reg32 kNoAboutFile = 0x54d714, kNoAboutFunction = 0x54d724, kNoAboutLine = 0xe;
constexpr reg32 kNoAbout = 0x54d72c;           // the Error's text: the driver has no THRASH_about

/* 0x4f8830's core: the screen's functions kept for the driver (0x567dc4 on), an Error first when it
 * has no THRASH_about -- as deep as the code calls it, under its six pushes: the Error hands
 * vsprintf an address on the stack. */
bool screenFunctionsSet(App* app, CPU& cpu)
{
    if (at(app, kThrashAbout) == 0)
    {
        put(app, kWhereFile, kNoAboutFile);
        put(app, kWhereFunction, kNoAboutFunction);
        put(app, kWhereLine, kNoAboutLine);
        const GuestFrame deep(cpu, 0x18, 0);
        if (!error(app, cpu, kNoAbout))
            return false;
    }
    put(app, kScreenFunctions, kModeAtExit);
    put(app, kScreenFunctions + 8, kLockScreen);
    put(app, kScreenFunctions + 0xc, kReleaseAndFlip);
    put(app, kScreenFunctions + 0x10, kSetUpScreen);
    put(app, kScreenFunctions + 4, kSetVideoMode);
    return true;
}

/* 0x4f8830: screenFunctionsSet; eax 1. */
void setScreenFunctions(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!screenFunctionsSet(app, cpu))
        return;
    cpu.eax = 1;
    kept.back(cpu);
    cpu.esp += 4;
}

// A screen's record: +4 its width, +8 its height, +0x1c its bits per pixel, +0x20 its pixels,
// +0x28 its pitch.  THRASH_lockwindow's record: +0 the pixels, +4 the pitch, +8 the format, +0xc
// the width, +0x10 the height.
constexpr reg32 kScreenWidthAt = 4, kScreenHeightAt = 8, kScreenBitsAt = 0x1c, kScreenPixels = 0x20,
                kScreenPitch = 0x28;
constexpr reg32 kLockPixels = 0, kLockPitch = 4, kLockFormat = 8, kLockWidth = 0xc, kLockHeight = 0x10;

/* 0x517890's core: window `window` taken and the frame buffer locked (THRASH_lockwindow) for the
 * screen record `screen`: its pixels and pitch from the lock, which is kept, as is the record (edx
 * the pixels, as the code leaves it).  1, or 0 when it would not lock. */
std::optional<reg32> frameBufferLocked(App* app, CPU& cpu, reg32 screen, reg32 window)
{
    if (!thrash(app, cpu, kThrashWindow, {window}))
        return std::nullopt;
    const std::optional<reg32> lock = thrash(app, cpu, kThrashLockWindow);
    if (!lock)
        return std::nullopt;
    put(app, kFrameLock, *lock);
    if (*lock == 0)
        return 0;
    cpu.edx = at(app, *lock + kLockPixels);
    put(app, screen + kScreenPixels, cpu.edx);
    const reg32 pitch = at(app, *lock + kLockPitch);
    put(app, kFrameLocked, screen);
    put(app, screen + kScreenPitch, pitch);
    return 1;
}

/* 0x517890: frameBufferLocked for the record eax, window edx. */
void lockFrameBuffer(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    const std::optional<reg32> locked = frameBufferLocked(app, cpu, cpu.eax, cpu.edx);
    if (!locked)
        return;
    cpu.eax = *locked;
    kept.back(cpu);
    cpu.esp += 4;
}

constexpr reg32 kRemoveTask = 0x4e75e0;        // native_snd_stream's removeSystemTask: eax the task

/* port::removeSystemTask (0x4e75e0), by address: the task's function in eax. */
bool removeTask(App* app, CPU& cpu, reg32 function)
{
    cpu.eax = guest::direct<kRemoveTask, port::stream::unscheduleTask>(app, cpu, function);
    return !cpu.terminate;
}

/* 0x524650's core: the driver closed, when it is open: its idle task taken off the timer, the
 * screen given back (THRASH_restore), the game's window destroyed too for `destroyWindow`; ecx and
 * edx kept from then on, as the code keeps them. */
bool driverClosed(App* app, CPU& cpu, bool destroyWindow)
{
    if (byte(app, kDriverOpen) == 0)
        return true;
    if (at(app, kThrashIdle) != 0 && !removeTask(app, cpu, kDriverIdle))
        return false;
    const Kept kept(cpu, kEcx | kEdx);
    if (!thrash(app, cpu, kThrashRestore) || (destroyWindow && !byAddress(app, cpu, kDestroyWindow)))
        return false;
    putByte(app, kDriverOpen, 0);
    kept.back(cpu);
    return true;
}

/* 0x524650: driverClosed, the window destroyed for eax nonzero; eax 0. */
void closeDriver(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx);
    if (!driverClosed(app, cpu, cpu.eax != 0))
        return;
    cpu.eax = 0;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x524640: closeDriver with the game's window destroyed too: what the game does at its end
 * (openDriver hands it to sub_4f2a78).  The code runs on into closeDriver with eax 1. */
void closeDriverAtExit(App* app, CPU& cpu)
{
    cpu.eax = 1;
    closeDriver(app, cpu);
}

/* ---- The drivers' pointers kept and taken back ---- */

constexpr reg32 kDriverSlot = 0x564a68;        // the slot in use
constexpr reg32 kDriverSlots = 0x564a70;       // eight of 0x8c bytes: the library, then 34 pointers
constexpr reg32 kDriverLibrary = 0x564a6c;
constexpr reg32 kSlotBytes = 0x8c, kSlotWords = 0x23, kSlotCount = 8;
/* Where each word of a slot goes, after the library: the game's pointers to
 * the driver's functions, in the order the slot keeps them. */
constexpr reg32 kSlotPointers[] = {
    kThrashAbout, 0x9ef984, kThrashInit, kThrashRestore, 0x9ef944, kThrashSetVideoMode, kThrashTAlloc,
    kThrashTUpdate, kThrashTReset, kThrashWindow, kThrashClearWindow, kThrashFlushWindow, kThrashPageFlip,
    kThrashIdle, kThrashSync, kThrashClip, kThrashSetState, kThrashSetTexture, kThrashLockWindow,
    kThrashUnlockWindow, kThrashReadRect, 0x9ef914, kThrashDrawQuad, kThrashDrawQuadMesh, kThrashDrawTri,
    kThrashDrawTriMesh, 0x9ef91c, 0x9ef960, 0x9ef928, kThrashDrawLineMesh, 0x9ef908, 0x9ef910, 0x9ef900, 0x9ef904,
};

/* A slot's index: 0 for one outside 0..7. */
reg32 slotIndex(reg32 slot)
{
    return sreg32(slot) >= sreg32(kSlotCount) || sreg32(slot) < 0 ? 0 : slot;
}

/* 0x517170's core: the driver of slot `slot` taken back: its library and its pointers, the slot
 * kept as the one in use.  The slot's last word, which the code leaves in eax. */
reg32 driverSlotUsed(App* app, reg32 slot)
{
    const reg32 index = slotIndex(slot);
    put(app, kDriverSlot, index);
    const reg32 words = kDriverSlots + index * kSlotBytes;
    put(app, kDriverLibrary, at(app, words));
    for (reg32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, kSlotPointers[i], at(app, words + 4 + 4 * i));
    return at(app, words + kSlotBytes - 4);
}

/* 0x517170: driverSlotUsed, slot eax. */
void useDriverSlot(App* app, CPU& cpu)
{
    cpu.eax = driverSlotUsed(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x517370's core: the driver in use kept in slot `slot`: its library and its pointers.  The slot's
 * first word's index, which the code leaves in eax. */
reg32 driverSlotKept(App* app, reg32 slot)
{
    const reg32 first = slotIndex(slot) * kSlotWords;
    const reg32 words = kDriverSlots + first * 4;
    put(app, words, at(app, kDriverLibrary));
    for (reg32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, words + 4 + 4 * i, at(app, kSlotPointers[i]));
    return first;
}

/* 0x517370: driverSlotKept, slot eax. */
void keepDriverSlot(App* app, CPU& cpu)
{
    cpu.eax = driverSlotKept(app, cpu.eax);
    cpu.esp += 4;
}

/* ---- A screen set up ---- */

constexpr reg32 kBitmapScreen = 0x4ed820;      // eax its window, edx..: its size and depth
constexpr reg32 kMakeBitmap = 0x508b80;
constexpr reg32 kDepthCode = 0x5098d0;         // al a depth: the screen record's code for it
constexpr reg32 kRowBuffer = 0x509a30;         // eax 0, edx a pitch or depth, ebx, ecx rows: a buffer
constexpr reg32 kBitmapScreenWhich = 0xff;
constexpr reg32 kScreenMagic = 0x57494e44;     // "DNIW"
// A screen's record, more of it: +0xc and +0x10 cleared, +0x14 and +0x18 its size again, +0x1d the
// depth's code, +0x1e which screen, +0x2c and +0x30 its two buffers, +0x34 its window.
constexpr reg32 kScreenCleared = 0xc, kScreenSizeAgain = 0x14, kScreenDepthCode = 0x1d, kScreenWhich = 0x1e,
                kScreenRows = 0x2c, kScreenMoreRows = 0x30, kScreenWindow = 0x34;

/* sub_509a30, a buffer of `rows` rows, by address: eax 0, edx a pitch or depth, ebx a kind, ecx the
 * rows.  The buffer. */
std::optional<reg32> rowBuffer(App* app, CPU& cpu, reg32 pitch, reg32 kind, reg32 rows)
{
    cpu.eax = 0;
    cpu.edx = pitch;
    cpu.ebx = kind;
    cpu.ecx = rows;
    return game(app, cpu, kRowBuffer);
}

/* 0x5179d0's core: screen `which` set up in `record`, `window` its window: nothing for 0; for 0xff
 * a bitmap of the window's size (sub_4ed820 into the frame, as deep as the code has it, then
 * sub_508b80); otherwise the driver's buffer `which` locked once for its pixels, pitch, size and
 * depth, then the record filled ("DNIW", its size again, which screen, the depth's code, the
 * window) with two buffers of its own.  ecx kept, as the code keeps it; the other registers as its
 * last call has them. */
bool screenSetUp(App* app, CPU& cpu, reg32 which, reg32 window, reg32 record)
{
    const Kept kept(cpu, kEcx);
    if (which == kBitmapScreenWhich)
    {
        // sub_4ed820's five numbers, in the frame: [edx], [ebx], [ecx] and the two pushed.
        const GuestFrame frame(cpu, 0x10, 0x18);
        const reg32 first = frame - 0x18, second = frame - 0x14, third = frame - 0x10, fourth = frame - 0xc,
                    fifth = frame - 8;
        cpu.eax = window;
        cpu.edx = first;
        cpu.ebx = second;
        cpu.ecx = third;
        if (!guest::call(app, cpu, kBitmapScreen, {fourth, fifth}))
            return false;
        cpu.eax = record;
        cpu.edx = at(app, first);
        cpu.ebx = at(app, second);
        cpu.ecx = at(app, fourth);
        if (!guest::call(app, cpu, kMakeBitmap, {at(app, third), at(app, fifth), window}))
            return false;
        kept.back(cpu);
        return true;
    }
    if (which == 0)
        return true;
    if (!thrash(app, cpu, kThrashWindow, {which}))
        return false;
    const std::optional<reg32> lock = thrash(app, cpu, kThrashLockWindow);
    if (!lock)
        return false;
    if (*lock != 0)
    {
        put(app, record + kScreenPixels, at(app, *lock + kLockPixels));
        put(app, record + kScreenPitch, at(app, *lock + kLockPitch));
        put(app, record + kScreenWidthAt, at(app, *lock + kLockWidth));
        put(app, record + kScreenHeightAt, at(app, *lock + kLockHeight));
        // The lock's format (the code's jump table at 0x5179a0): its bits per pixel.
        reg8 bits = 0x10;
        switch (at(app, *lock + kLockFormat) - 2)
        {
        case 0: bits = 8; break;
        case 1: case 9: bits = 0xf; break;
        case 2: bits = 0x10; break;
        case 3: bits = 0x18; break;
        case 4: bits = 0x20; break;
        default: break;
        }
        putByte(app, record + kScreenBitsAt, bits);
        if (!thrash(app, cpu, kThrashUnlockWindow, {*lock}))
            return false;
    }
    if (!thrash(app, cpu, kThrashWindow, {0}))
        return false;
    const reg32 depth = byte(app, record + kScreenBitsAt) == 0xf ? 0x10 : byte(app, record + kScreenBitsAt);
    put(app, record, kScreenMagic);
    put(app, record + kScreenCleared, 0);
    put(app, record + kScreenCleared + 4, 0);
    put(app, record + kScreenSizeAgain, at(app, record + kScreenWidthAt));
    put(app, record + kScreenSizeAgain + 4, at(app, record + kScreenHeightAt));
    putByte(app, record + kScreenWhich, reg8(which));
    cpu.eax = byte(app, record + kScreenBitsAt);
    cpu.ebx = 0;
    const std::optional<reg32> code = game(app, cpu, kDepthCode);
    if (!code)
        return false;
    const reg32 height = at(app, record + kScreenHeightAt), pitch = at(app, record + kScreenPitch);
    putByte(app, record + kScreenDepthCode, reg8(*code));
    put(app, record + kScreenWindow, window);
    const std::optional<reg32> rows = rowBuffer(app, cpu, pitch, 0, height);
    if (!rows)
        return false;
    put(app, record + kScreenRows, *rows);
    const std::optional<reg32> moreRows = rowBuffer(app, cpu, depth, 3, at(app, record + kScreenWidthAt));
    if (!moreRows)
        return false;
    put(app, record + kScreenMoreRows, *moreRows);
    kept.back(cpu);
    return true;
}

/* 0x5179d0: screenSetUp, screen eax in the record at ebx, its window edx. */
void setUpScreen(App* app, CPU& cpu)
{
    if (screenSetUp(app, cpu, cpu.eax, cpu.edx, cpu.ebx))
        cpu.esp += 4;
}

}  // namespace graphics

// nfs3hp_main.cpp (tools/apply_widescreen.py): a 16:9 size taken with the
// 4:3 ones, and Dynamic Fullscreen's marker answered with its mode.
bool widescreenMode(x86::reg32 width, x86::reg32 height);
x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);

namespace graphics
{

/* ---- The Screen Size list ---- */

constexpr reg32 kModeMemory = 0x55fed0;        // the driver's memory for a mode (8 MB on a Voodoo)
// A mode of THRASH_about's (kModeBytes from its +0x40): +0 its width, +4 its height, +8 its bits,
// +0x10 its flags (bit 0 available, bit 1 barred), +0x14 its colour buffers, +0x18 its buffers.
constexpr reg32 kModeWidthAt = 0, kModeHeightAt = 4, kModeBitsAt = 8, kModeFlags = 0x10, kModeColourBuffers = 0x14,
                kModeBuffersAt = 0x18;

/* THRASH_about's mode `index`. */
reg32 modeRecord(App* app, reg32 about, reg32 index)
{
    return index * kModeBytes + at(app, about + kAboutModes);
}

/* Whether a mode's width and height are 4:3, or what nfs3hp::widescreenMode takes (a port). */
bool raceShape(reg32 width, reg32 height)
{
    return width * 3 == height << 2 || widescreenMode(width, height);
}

/* Whether a mode's flags say it is available and not barred. */
bool modeOpen(reg8 flags)
{
    return (flags & 1) && !(flags & 2);
}

/* 0x4bed90's core: each of THRASH_about's modes the game can race in handed to `handler` (eax the
 * mode, edx its width, [esp+4] whether it has three buffers; ebx and ecx its height and bits, as the
 * code has them; esp as the handler leaves it): at least two colour buffers, 4:3 or a port's wide
 * one, 15 or 16 bits, available and not barred, and two buffers of it within half the driver's
 * memory.  As deep as the code calls it.  eax THRASH_about's record at the end, as the code leaves
 * it. */
bool screenModesListed(App* app, CPU& cpu, reg32 handler)
{
    const GuestFrame frame(cpu, 0x18, 0x10);
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return false;
    for (reg32 mode = 1;; ++mode)
    {
        cpu.eax = *about;
        if (sreg32(mode) > sreg32(at(app, *about + kAboutLastMode)))
            return true;
        const reg32 record = modeRecord(app, *about, mode);
        const reg32 buffers = at(app, record + kModeColourBuffers), width = at(app, record + kModeWidthAt),
                    height = at(app, record + kModeHeightAt), bits = at(app, record + kModeBitsAt);
        const reg8 flags = reg8(at(app, record + kModeFlags));
        if (sreg32(buffers) < 2 || !raceShape(width, height) || (bits != 0xf && bits != 0x10) || !modeOpen(flags))
            continue;
        if (sreg32(at(app, kModeMemory)) / 2 < sreg32(width * height * 2))
            continue;
        const reg32 triple = sreg32(at(app, modeRecord(app, *about, mode) + kModeBuffersAt)) >= 2 ? 1 : 0;
        cpu.eax = mode;
        cpu.edx = width;
        cpu.ebx = height;
        cpu.ecx = bits;
        push(app, cpu, triple);
        if (!byAddress(app, cpu, handler))
            return false;
    }
}

/* 0x4bed90: screenModesListed, the handler at eax. */
void listScreenModes(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!screenModesListed(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* 0x4bee80's core: the mode the Screen Size list would have for `width`: of those
 * screenModesListed takes (memory aside), the nearest in width, the first of equals;
 * nfs3hp::dynamicSavedMode for Dynamic Fullscreen's marker (a port).  The mode, 1 for none. */
std::optional<reg32> screenModeFor(App* app, CPU& cpu, reg32 width)
{
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return std::nullopt;
    reg32 found = 0, nearest = 0;
    for (reg32 mode = 1; sreg32(mode) <= sreg32(at(app, *about + kAboutLastMode)); ++mode)
    {
        const reg32 record = modeRecord(app, *about, mode);
        const reg32 modeWidth = at(app, record + kModeWidthAt), modeHeight = at(app, record + kModeHeightAt);
        if (sreg32(at(app, record + kModeColourBuffers)) < 2 || !raceShape(modeWidth, modeHeight))
            continue;
        const reg32 bits = at(app, record + kModeBitsAt);
        if ((bits != 0xf && bits != 0x10) || !modeOpen(byte(app, modeRecord(app, *about, mode) + kModeFlags)))
            continue;
        reg32 distance = modeWidth - width;
        if (sreg32(distance) <= 0)
            distance = 0 - distance;
        if (found != 0 && sreg32(distance) >= sreg32(nearest))
            continue;
        found = mode;
        nearest = distance;
    }
    found = dynamicSavedMode(found, width);
    return found != 0 ? found : 1;
}

/* 0x4bee80: screenModeFor width eax. */
void findScreenMode(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    const std::optional<reg32> mode = screenModeFor(app, cpu, cpu.eax);
    if (!mode)
        return;
    cpu.eax = *mode;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The menus' art in textures ---- */

constexpr reg32 kBadArtFormat = 0x5495c8;
constexpr reg32 kFillZero = 0x4e070c;          // eax where, edx bytes
constexpr reg32 kFillByte = 0x4e0716;          // eax where, edx bytes, bl the byte
constexpr reg32 kArtSide = 0x20;               // the art's pixels a side
constexpr reg32 kArtShapeBytes = 0x1014;       // what the pixels are made into

/* 0x4d5f40's core: a texture `size` by `size` of the driver's format [formatAt] (3: 1555, 4: 565,
 * 6: 8888; another an Error) allocated (THRASH_talloc), after its 32 by 32 pixels -- clear, or opaque
 * white for `clear` 0 -- were made into a shape and run through sub_4d37a0 with the record at
 * `formatAt`.  The shape's kind and the clear flag live in the frame, as deep as the code has it:
 * sub_4d37a0 is handed the flag's slot, and another format leaves the kind as the stack has it.
 * The registers at each call as the code has them; eax as THRASH_talloc leaves it. */
bool artTexture(App* app, CPU& cpu, reg32 size, reg32 formatAt, reg32 clear)
{
    const GuestFrame frame(cpu, 0x10, 0x18);
    const reg32 clearAt = frame - 0x18, kindAt = frame - 4;
    put(app, clearAt, clear);
    reg32 bits = 0;
    switch (at(app, formatAt))
    {
    case 4: put(app, kindAt, 0x78); bits = 0x10; break;
    case 3: put(app, kindAt, 0x7e); bits = 0x10; break;
    case 6: put(app, kindAt, 0x7d); bits = 0x20; break;
    default:
        if (!error(app, cpu, kBadArtFormat))
            return false;
        break;
    }
    const reg32 pixelBytes = reg32(sreg32(bits) / 8) << 0xa;
    const reg32 shapeBytes = pixelBytes + 0x14;
    const std::optional<reg32> shape = allocate(app, cpu, kShapeBufferTag, shapeBytes);
    if (!shape)
        return false;
    cpu.ecx = *shape;
    const std::optional<reg32> pixels = allocate(app, cpu, kShapeBufferTag, kArtShapeBytes);
    if (!pixels)
        return false;
    cpu.eax = *shape;
    cpu.edx = shapeBytes;
    if (!byAddress(app, cpu, kFillZero))
        return false;
    cpu.eax = *shape + 0x10;
    if (at(app, clearAt) == 0)
    {
        cpu.ebx = 0xff;
        cpu.edx = pixelBytes;
        if (!byAddress(app, cpu, kFillByte))
            return false;
    }
    const reg32 kind = at(app, kindAt) & 0xff;
    putByte(app, *shape, 0);
    putHalf(app, *shape + 4, kArtSide);
    put(app, *shape, at(app, *shape) | kind);
    putHalf(app, *shape + 6, kArtSide);
    cpu.ecx = kArtSide;
    if (!shapeToPixels(app, cpu, *pixels, *shape, clearAt, formatAt) || !release(app, cpu, *shape)
        || !release(app, cpu, *pixels))
        return false;
    cpu.ebx = at(app, formatAt);
    return thrash(app, cpu, kThrashTAlloc, {size, size, cpu.ebx, 0, 0}).has_value();
}

/* 0x4d5f40: artTexture, eax by eax, the format at edx, clear for ebx nonzero. */
void makeArtTexture(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEcx);
    if (!artTexture(app, cpu, cpu.eax, cpu.edx, cpu.ebx))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The screen saved to a file (Alt+P) ---- */

constexpr reg32 kCaptureTrace = 0x5401c8;
constexpr reg32 kCaptureFolder = 0x4b6520;     // the folder pictures go in
constexpr reg32 kCaptureName = 0x5401d8;       // "%s...%d...": the folder, the number
constexpr reg32 kCaptureNumber = 0x5600f8;
constexpr reg32 kRedrawPaused = 0x4bc680;
constexpr reg32 kNewBitmap = 0x4ef190, kSaveBitmap = 0x4eb160;
constexpr reg32 kModeWidthPixels = 0x7cdae0, kModeHeightPixels = 0x7cdae4, kModeBits = 0x7cdae8;

/* A string copied byte by byte, its 0 too, as the game's inlined strcpy copies it. */
void copyString(App* app, reg32 to, reg32 from)
{
    for (reg32 i = 0;; ++i)
    {
        const reg8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            return;
    }
}

/* A string's length, as the game's inlined strlen (repne scasb) counts it. */
reg32 stringLengthAt(App* app, reg32 from)
{
    reg32 length = 0;
    while (byte(app, from + length) != 0)
        ++length;
    return length;
}

constexpr reg32 kBitmapPrepare = 0x4eada0;     // eax the new bitmap
constexpr reg32 kBitmapSelect = 0x4ef1b0;      // eax 0

/* 0x4c4490's core: the screen saved as a picture -- but in a network race -- under the first
 * number not yet taken in the capture folder: a paused race drawn again first, the front buffer
 * read (THRASH_readrect) into a bitmap of the mode's size, and the bitmap saved and freed.  The
 * folder and the name in the frame, as deep as the code has them; the registers at each call as the
 * code has them; eax as the last call leaves it. */
bool screenCaptured(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0x300);
    const reg32 folder = frame - 0x300, name = frame - 0x100;
    if (!traced(app, cpu, kCaptureTrace))
        return false;
    cpu.edx = gameSettings(app).playMode;
    if (sreg32(cpu.edx) >= 2)
        return true;
    const std::optional<reg32> captures = game(app, cpu, kCaptureFolder);
    if (!captures)
        return false;
    copyString(app, folder, *captures);
    cpu.eax = 0;  // the inlined strlen's al
    if (stringLengthAt(app, folder) == 0)
        return true;
    for (;;)
    {
        const reg32 number = at(app, kCaptureNumber) + 1;
        put(app, kCaptureNumber, number);
        cpu.eax = name;
        cpu.ecx = number;
        cpu.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, kCaptureName,
                                                                 guest::Words{folder, number});
        if (cpu.terminate)
            return false;
        const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
        if (!exists)
            return false;
        if (!*exists)
            break;
    }
    const reg32 replay = gameSettings(app).replayMode;
    if (replay == 2)
    {
        gameSettings(app).replayMode = 0;
        if (!byAddress(app, cpu, kRedrawPaused) || !flushFlip(app, cpu))
            return false;
        gameSettings(app).replayMode = replay;
    }
    cpu.eax = at(app, kModeWidthPixels);
    cpu.edx = at(app, kModeHeightPixels);
    cpu.ebx = at(app, kModeBits);
    cpu.ecx = 0x10;
    const std::optional<reg32> bitmap = game(app, cpu, kNewBitmap);
    if (!bitmap)
        return false;
    cpu.ebx = *bitmap;
    if (*bitmap == 0)
        return true;
    if (!byAddress(app, cpu, kBitmapPrepare))
        return false;
    cpu.eax = 0;
    if (!byAddress(app, cpu, kBitmapSelect) || !thrash(app, cpu, kThrashWindow, {1}))
        return false;
    const reg32 pixels = at(app, *bitmap + 0x20) + 0x10;
    cpu.eax = pixels;
    cpu.edx = at(app, kModeHeightPixels);
    cpu.ecx = at(app, kModeWidthPixels);
    if (!thrash(app, cpu, kThrashReadRect, {0, 0, cpu.ecx, cpu.edx, pixels}))
        return false;
    cpu.eax = name;
    if (!byAddress(app, cpu, kSaveBitmap))
        return false;
    return letGo(app, cpu, *bitmap);
}

/* 0x4c4490: screenCaptured. */
void captureScreen(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!screenCaptured(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- A message across the screen ---- */

constexpr reg32 kViewRect = 0x4beb80;          // eax, edx, ebx, ecx: where the view's x, y, width, height go
constexpr reg32 kMessageLine = 0x115;
constexpr reg32 kMessageEnding = 0x540208;
constexpr reg32 kMessageScale = 0x54020c, kMessageScaleAgain = 0x540210;
constexpr reg32 kFont87 = 0x57;
constexpr reg32 kFontLineHeight = 0x4d14f0;
constexpr reg32 kDrawBox = 0x47d1d0;           // eight floats and two words: a box with its border
constexpr reg32 kBoxColour = 0x7f000000, kBorderColour = 0xff488cfd;

/* port_text's textWidthWithOverhang (0x4d1480), by address: text eax (edx as the code has it); the
 * width it leaves on the x87's stack, taken off.  nullopt when the game stopped. */
std::optional<x86::Float> textWidth(App* app, CPU& cpu, reg32 text)
{
    cpu.eax = text;
    const port::text::WidthResult width =
        guest::direct<kTextWidth, port::text::textMeasuredWithOverhang>(app, cpu, cpu.esp - 4, text);
    if (!width || cpu.terminate)
        return std::nullopt;
    cpu.eax = width->eax;
    return x86::Float(width->value);
}

/* sub_47d1d0, a box with its border, by address: its four corners (x, y as floats' bits), whether
 * filled, its colour; stdcall.  eax, edx, ebx and ecx as the code's pushes leave them. */
bool drawBox(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 filled, reg32 colour)
{
    cpu.eax = top;
    cpu.edx = left;
    cpu.ebx = right;
    cpu.ecx = top;
    return guest::call(app, cpu, kDrawBox, {left, top, right, top, right, bottom, left, bottom, filled, colour});
}

/* 0x4c5c00's core: line 0x115 of the game's text (with its ending) in a box in the middle of the
 * view, but while paused: drawn into both buffers, a flip after each; then the race's frame ended
 * (eax as that leaves it).  The view's rectangle and the text in the frame, as deep as the code has
 * them; the x87's operations as the code does them; the registers at each call as the code has
 * them. */
bool waitMessage(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0x64);
    const reg32 text = frame - 0x64, viewWidthAt = frame - 8, viewHeightAt = frame - 0x18;
    cpu.eax = frame - 0xc;
    cpu.edx = frame - 0x14;
    cpu.ebx = viewWidthAt;
    cpu.ecx = viewHeightAt;
    if (!byAddress(app, cpu, kViewRect))
        return false;
    if (sreg32(gameSettings(app).replayMode) > 1)
        return true;
    const Kept synced(cpu, kEcx | kEdx);  // as syncWindow keeps them
    if (!thrash(app, cpu, kThrashSync, {0}) || !thrash(app, cpu, kThrashWindow, {2}))
        return false;
    synced.back(cpu);
    const std::optional<reg32> line = languageText(app, cpu, kMessageLine);
    if (!line)
        return false;
    copyString(app, text, *line);
    copyString(app, text + stringLengthAt(app, text), kMessageEnding);
    // The text at the view's height over 480.
    const x86::Float scale =
        fp::mul(fp::mul(x86::Float(sreg32(at(app, viewWidthAt))), x86::Float(floatAt(app, kMessageScale))),
                x86::Float(floatAt(app, kMessageScaleAgain)));
    if (!setTextScale(app, cpu, bitsOf(float(scale))))
        return false;
    const std::optional<x86::Float> width = textWidth(app, cpu, text);
    if (!width)
        return false;
    const std::optional<sreg32> textWide = chop(app, cpu, *width);
    if (!textWide)
        return false;
    cpu.eax = kFont87;
    const std::optional<reg32> fontHeight = game(app, cpu, kTextHeight);
    if (!fontHeight)
        return false;
    cpu.ecx = *fontHeight;
    cpu.eax = kFont87;
    const std::optional<reg32> lineHeight = game(app, cpu, kFontLineHeight);
    if (!lineHeight)
        return false;
    // Integers as the code works them out: wrapping, halved and quartered towards 0.
    const auto halved = [](reg32 value) { return reg32(sreg32(value) / 2); };
    const auto asFloat = [](reg32 value) { return bitsOf(float(x86::Float(sreg32(value)))); };
    const reg32 high = *fontHeight + *lineHeight;
    // The text's place: the view's middle less half the text.
    const reg32 middle = halved(at(app, viewWidthAt)), middleY = halved(at(app, viewHeightAt));
    const reg32 textLeft = middle - halved(reg32(*textWide)), textTop = middleY - halved(high);
    // The box: three quarters of the text's width either side, a line up and down.
    const reg32 reach = reg32(sreg32(reg32(*textWide) * 3) / 4);
    const reg32 left = asFloat(middle - reach), right = asFloat(reach + middle);
    const reg32 top = asFloat(middleY - high), bottom = asFloat(middleY + high);
    if (!thrash(app, cpu, kThrashWindow, {2}))
        return false;
    for (int pass = 0; pass < 2; ++pass)
        if (!drawBox(app, cpu, left, top, right, bottom, 1, kBoxColour)
            || !drawBox(app, cpu, left, top, right, bottom, 0, kBorderColour)
            || !drawText(app, cpu, text, textLeft, textTop) || !byAddress(app, cpu, kFlushText)
            || !flushFlip(app, cpu))
            return false;
    return guest::direct<0x434a90, collide::fogColour>(app, cpu);
}

/* 0x4c5c00: waitMessage. */
void showWaitMessage(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!waitMessage(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The HUD editor's frame ---- */

constexpr reg32 kFrameMesh = 0x559a20;         // twelve corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr reg32 kFrameScaleX = 0x539b94, kFrameScaleY = 0x539b9c, kFrameThickness = 0x539b98;
constexpr reg32 kFrameMarginX = 0x539ba0, kFrameMarginY = 0x539ba4;
constexpr reg32 kFrameAll = 0x559ba0, kFrameClippedBoth = 0x559c40;  // the lines' index lists
constexpr reg32 kFrameClippedLeft = 0x559be0, kFrameClippedRight = 0x559c10;

/* fcom and fnstsw's C0, as the code tests it: a below b, or either NaN. */
bool belowOrUnordered(x86::Float a, x86::Float b)
{
    return !(a >= b);
}

// A HUD element: +6 its left and +8 its top (words), +0x60 its right and +0x64 its bottom.
constexpr reg32 kElementLeft = 6, kElementTop = 8, kElementRight = 0x60, kElementBottom = 0x64;

/* 0x459440's core: the HUD editor's frame about `element` (made room about by the mode's width),
 * drawn as a mesh of lines in `colour` -- the side that is off the screen left out.  The x87's
 * operations as the code does them, each float stored where it stores one; its compares (fcom,
 * fnstsw) as float compares that give what C0 and C3 give, NaN too.  eax and edx at the driver's
 * call as the code has them: whether the right side is off, whether the left is. */
bool editorFrame(App* app, CPU& cpu, reg32 element, reg32 colour)
{
    const auto loadFloat = [&](reg32 address) { return x86::Float(floatAt(app, address)); };
    const auto word16 = [&](reg32 address) { return x86::Float(x86::sreg16(half(app, address))); };
    const auto integer = [&](reg32 address) { return x86::Float(sreg32(at(app, address))); };
    const auto corner = [](reg32 n) { return kFrameMesh + n * kCornerBytes; };
    const x86::Float width = integer(kModeWidth);
    const x86::Float scaledX = fp::mul(width, loadFloat(kFrameScaleX));
    const x86::Float scaledY = fp::mul(width, loadFloat(kFrameScaleY));
    const x86::Float thickness = loadFloat(kFrameThickness);
    const float roomX = float(fp::mul(scaledX, thickness));
    const float roomY = float(fp::mul(thickness, scaledY));
    const x86::Float right = integer(element + kElementRight);
    const x86::Float left = word16(element + kElementLeft);
    const float leftEdge = float(fp::add(left, loadFloat(kFrameMarginX)));
    const float rightEdge = float(fp::add(right, loadFloat(kFrameMarginY)));
    for (reg32 n = 0; n < 0xc; ++n)
        put(app, corner(n) + kCornerColour, colour);
    // The left edge kept on the screen.
    const reg32 leftX = belowOrUnordered(0.0, x86::Float(leftEdge)) ? bitsOf(leftEdge) : 0;
    const x86::Float frameLeft = word16(element + kElementLeft);
    const x86::Float frameRight = integer(element + kElementRight);
    for (const reg32 n : {11u, 10u, 1u, 0u})
        put(app, corner(n), leftX);
    putFloat(app, corner(9), float(fp::add(frameLeft, x86::Float(roomX))));
    putFloat(app, corner(8), float(fp::sub(frameRight, x86::Float(roomX))));
    const x86::Float innerLeft = loadFloat(corner(9)), innerRight = loadFloat(corner(8));
    putFloat(app, corner(2), float(innerLeft));
    putFloat(app, corner(3), float(innerRight));
    // The right edge kept on the screen.
    const float lastX = float(x86::Float(sreg32(at(app, kModeWidth) - 1)));
    const reg32 rightX =
        belowOrUnordered(x86::Float(rightEdge), x86::Float(lastX)) ? bitsOf(rightEdge) : bitsOf(lastX);
    const float top = float(word16(element + kElementTop));
    const float bottom = float(integer(element + kElementBottom));
    for (const reg32 n : {7u, 6u, 5u, 4u})
        put(app, corner(n), rightX);
    putFloat(app, corner(5) + 4, float(fp::add(x86::Float(top), x86::Float(roomY))));
    putFloat(app, corner(11) + 4, float(fp::sub(x86::Float(bottom), x86::Float(roomY))));
    const x86::Float innerTop = loadFloat(corner(5) + 4), innerBottom = loadFloat(corner(11) + 4);
    for (const reg32 n : {4u, 3u, 2u, 1u})
        put(app, corner(n) + 4, bitsOf(top));
    putFloat(app, corner(0) + 4, float(innerTop));
    for (const reg32 n : {10u, 9u, 8u, 7u})
        put(app, corner(n) + 4, bitsOf(bottom));
    putFloat(app, corner(6) + 4, float(innerBottom));
    if (!sendState(app, cpu, thrash::kStateTexture, 0))
        return false;
    // Which sides are off the screen: the lines of all, or of those on it.
    const bool leftOff = x86::Float(0.0) > x86::Float(leftEdge);  // C0 and C3 clear: above, not NaN
    const bool rightOff =
        belowOrUnordered(x86::Float(sreg32(at(app, kModeWidth) - 1)), x86::Float(rightEdge));
    reg32 lines = kFrameAll, count = 8;
    if (leftOff && rightOff)
        lines = kFrameClippedBoth, count = 4;
    else if (leftOff || rightOff)
        lines = leftOff ? kFrameClippedLeft : kFrameClippedRight, count = 6;
    cpu.eax = rightOff ? 1 : 0;
    cpu.edx = leftOff ? 1 : 0;
    return thrash(app, cpu, kThrashDrawLineMesh, {count, kFrameMesh, lines}).has_value();
}

/* 0x459440: editorFrame about element eax, colour edx. */
void hudEditorFrame(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    if (!editorFrame(app, cpu, cpu.eax, cpu.edx))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

}  // namespace graphics


namespace graphics
{

/* ---- Loading a race ---- */

constexpr reg32 kMenuControl = 0x79f3f4, kRaceControl = 0x79f3ec;  // the x87's words, the menus' and a race's
constexpr reg32 kLoadingOff = 0x55e88c;

namespace
{

/* fninit and fldcw [0x79f3f4]: the menus' precision. */
void menuPrecision(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.fpu.init();
    cpu.fpu.setControl(half(app, kMenuControl));
}

/* fninit, fnstcw [0x79f3ec], the word kept as the menus' and its precision
 * cleared, fldcw: the race's.  eax as the code leaves it. */
void racePrecision(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.fpu.init();
    putHalf(app, kRaceControl, cpu.fpu.control.word);
    cpu.eax = at(app, kRaceControl);
    put(app, kMenuControl, cpu.eax);
    cpu.eax &= 0xfffff0ff;
    put(app, kRaceControl, cpu.eax);
    cpu.fpu.setControl(half(app, kRaceControl));
}

}  // namespace

constexpr reg32 kLoadingPictures = 0x57126b, kPictureChoices = 0x552938;
constexpr reg32 kLoadingPath = 0x7a3134;
constexpr reg32 kLoadingScreen = 0x494cb0;     // native_render's loadingScreen: eax the picture

constexpr reg32 kPictureCount = 0x43bcb0;      // eax a car: how many pictures its track has
constexpr reg32 kAppendBackslash = 0x4529a0;   // port::menus' path ended with a backslash: eax the path
constexpr reg32 kLoadQfs = 0x4956f0;           // frontend::loadQfs: eax the name, edx 0x10
constexpr reg32 kBitmapShape = 0x4ef1e0;       // eax a .qfs, edx a shape's name
constexpr reg32 kTrackPictureName = 0x53be00, kCarPictureName = 0x53be10, kLoadingShapeName = 0x53be20;
constexpr reg32 kTrackNameBytes = 0x10;
// The race's entries: a player's 0x6c bytes (its car among them).
constexpr reg32 kEntryBytes = 0x6c;

/* port::menus' backslash after a path (0x4529a0), by address: the path in eax. */
bool appendBackslash(App* app, CPU& cpu, reg32 path)
{
    cpu.eax = path;
    return byAddress(app, cpu, kAppendBackslash);
}

/* sprintf onto the end of the string at `text`, as the code finds it (its inlined strlen). */
bool appendFormatted(App* app, CPU& cpu, reg32 text, reg32 format, reg32 first, reg32 second)
{
    cpu.eax = text + stringLengthAt(app, text);
    cpu.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, cpu.eax, format, guest::Words{first, second});
    return !cpu.terminate;
}

/* 0x494fa0's core: the loading screen put up as a race loads (but with it turned off): the race's
 * track's picture -- one of its several in turn, or the car's when there is none or [0x6fd4fc] asks
 * for it -- loaded and shown in both buffers; the textures let go of, and the race's x87 precision
 * set (eax as that leaves it).  The track's name and the picture's in the frame, as deep as the
 * code has them; the registers at each call as the code has them. */
bool loadingScreenShown(App* app, CPU& cpu)
{
    if (at(app, kLoadingOff) != 0)
        return true;
    const GuestFrame frame(cpu, 0x18, 0x114);
    const reg32 name = frame - 0x114, track = frame - 0x14;
    menuPrecision(app, cpu);
    const reg32 car = gameSettings(app).entryAt(gameSettings(app).localPlayer * kEntryBytes).car;
    cpu.edx = car;  // as the code has it on, the car's track's name (load::idOf) keeping it
    const std::optional<reg32> trackName = guest::direct<0x439520, load::idOf>(app, cpu, car);
    if (!trackName)
        return false;
    // Its track's name: rep movsd, rep movsb, ecx counted down.
    for (reg32 i = 0; i < kTrackNameBytes; i += 4)
        put(app, track + i, at(app, *trackName + i));
    cpu.ecx = 0;
    const reg32 id = guest::direct<0x439660, load::numberOf>(app, cpu, track);
    if (cpu.terminate)
        return false;
    cpu.ebx = id;
    cpu.eax = car;
    const std::optional<reg32> pictures = game(app, cpu, kPictureCount);
    if (!pictures)
        return false;
    cpu.ecx = *pictures;
    copyString(app, name, kLoadingPath);
    if (!appendBackslash(app, cpu, name))
        return false;
    // Which of the track's pictures: the next in turn, by their count.
    const reg8 turn = byte(app, kLoadingPictures);
    const sreg32 choice = byte(app, kPictureChoices + turn);
    const reg32 which = reg32(sreg32(choice % sreg32(*pictures)));
    putByte(app, kLoadingPictures, reg8(turn + 1));
    if (!appendFormatted(app, cpu, name, kTrackPictureName, id, which))
        return false;
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
    if (!exists)
        return false;
    if (!*exists || (std::uint8_t(gameSettings(app).raceFlags) & 2))
    {
        // The car's picture instead.
        copyString(app, name, kLoadingPath);
        if (!appendBackslash(app, cpu, name))
            return false;
        cpu.ecx = gameSettings(app).track;
        if (!appendFormatted(app, cpu, name, kCarPictureName, cpu.ecx, 0))
            return false;
    }
    // Its flags in edx, which loadQfs takes from there.
    cpu.edx = 0x10;
    cpu.eax = name;
    const reg32 loaded = guest::direct<kLoadQfs, frontend::draw::loadQfs>(app, cpu, name);
    if (cpu.terminate)
        return false;
    cpu.eax = loaded;
    const std::optional<reg32> qfs = loaded;
    cpu.edx = kLoadingShapeName;
    cpu.ecx = *qfs;
    const std::optional<reg32> shape = game(app, cpu, kBitmapShape);
    if (!shape)
        return false;
    cpu.edx = *shape;
    for (int buffer = 0; buffer < 2; ++buffer)
    {
        if (!windowTakenAndCleared(app, cpu))
            return false;
        cpu.eax = *shape;
        if (!byAddress(app, cpu, kLoadingScreen))
            return false;
        const Kept kept(cpu, kEcx | kEdx);  // as flipAndWaitFrames keeps them
        const std::optional<reg32> until = flipAndWait(app, cpu);
        if (!until)
            return false;
        cpu.eax = *until;
        kept.back(cpu);
    }
    if (*qfs != 0 && !release(app, cpu, *qfs))
        return false;
    if (!thrash(app, cpu, kThrashTReset))
        return false;
    racePrecision(app, cpu);
    return true;
}

/* 0x494fa0: loadingScreenShown. */
void showLoadingScreen(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!loadingScreenShown(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

constexpr reg32 kLoadingBar = 0x55e4c4;        // four corners of kCornerBytes
constexpr reg32 kLoadingAll = 0x53be30;        // 100, a double: the most of [esp+4]
constexpr reg32 kLoadingPerCent = 0x53be38;    // a double
constexpr reg32 kLoadingBarEnd = 0x53be40, kLoadingBarHeight = 0x53be44;
constexpr reg32 kRunSystemTasks = 0x4e7630;    // native_snd_stream's runSystemTasks
constexpr reg32 kBarColours = 0x495190;        // eax the bar's corners

constexpr reg32 kBarTop = 0x43e08000, kBarBottom = 0x43ea8000;  // 449.0f and 469.0f, on a 480 line screen
constexpr reg32 kBarFull = 0x42ca0000;          // 101.0f

/* native_render's polygon (0x4c12e0) for the loading bar, by address: its corners in eax, edx,
 * ebx, ecx. */
bool drawBar(App* app, CPU& cpu)
{
    return drawQuad(app, cpu, kLoadingBar, kLoadingBar + kCornerBytes, kLoadingBar + 2 * kCornerBytes,
                    kLoadingBar + 3 * kCornerBytes);
}

/* 0x495230's core: the loading bar at `perCentAt` per cent (101 past 100, written back there, then
 * the bar's share) drawn after the loading text, in both buffers, while the loading text is up; the
 * system's tasks run first.  The menus' x87 precision while it draws, the race's after (eax as
 * that leaves it).  The x87's operations as the code does them, each float stored where it stores
 * one; its compare (fcom, fnstsw) a float compare that gives what C0 gives, NaN too. */
bool loadingProgress(App* app, CPU& cpu, reg32 perCentAt)
{
    cpu.eax = 0;
    if (!byAddress(app, cpu, kRunSystemTasks))
        return false;
    if (at(app, kLoadingTextShown) == 0)
        return true;
    if (!thrash(app, cpu, kThrashSync, {0}) || !thrash(app, cpu, kThrashWindow, {2}))
        return false;
    menuPrecision(app, cpu);
    if (!belowOrUnordered(x86::Float(floatAt(app, perCentAt)), x86::Float(app->getMemory<double>(kLoadingAll))))
        put(app, perCentAt, kBarFull);
    const x86::Float scaled =
        fp::mul(x86::Float(floatAt(app, perCentAt)), x86::Float(app->getMemory<double>(kLoadingPerCent)));
    const x86::Float left = x86::Float(floatAt(app, kLoadingTextX));
    const x86::Float room = fp::sub(x86::Float(floatAt(app, kLoadingBarEnd)), left);
    putFloat(app, perCentAt, float(scaled));
    const x86::Float width = fp::mul(room, x86::Float(floatAt(app, perCentAt)));
    const x86::Float height =
        fp::mul(x86::Float(floatAt(app, perCentAt)), x86::Float(floatAt(app, kLoadingBarHeight)));
    const auto corner = [](reg32 n) { return kLoadingBar + n * kCornerBytes; };
    putFloat(app, corner(0), float(left));
    putFloat(app, corner(3), float(left));
    put(app, corner(0) + 4, kBarTop);
    put(app, corner(1) + 4, kBarTop);
    put(app, corner(2) + 4, kBarBottom);
    put(app, corner(3) + 4, kBarBottom);
    const std::optional<sreg32> shade = chop(app, cpu, height);
    if (!shade)
        return false;
    putFloat(app, corner(1), float(fp::add(left, width)));
    const reg32 colour = (reg32(*shade) << 0x10) - 0x1000000;
    const x86::Float right = x86::Float(floatAt(app, corner(1)));
    put(app, corner(1) + kCornerColour, colour);
    put(app, corner(2) + kCornerColour, colour);
    putFloat(app, corner(2), float(right));
    // sub_495190 with the bar's corners in eax, edx, ebx and ecx, as the code has them.
    cpu.eax = corner(0);
    cpu.edx = corner(1);
    cpu.ebx = corner(2);
    cpu.ecx = corner(3);
    if (!byAddress(app, cpu, kBarColours))
        return false;
    for (reg32 n = 0; n < 4; ++n)
        putClipCode(app, corner(n));
    if (!sendState(app, cpu, thrash::kStateTexture, 0))
        return false;
    // Softtria's shading on for the bar, and off again.
    const auto shaded = [&](reg32 value) {
        return !(byte(app, kDriverFlags) & kDriverSofttria) || sendState(app, cpu, thrash::kStateShade, value);
    };
    for (int buffer = 0; buffer < 2; ++buffer)
        if ((buffer == 1 && !thrash(app, cpu, kThrashSync, {0})) || !shaded(1) || !drawBar(app, cpu) || !shaded(0)
            || !flushFlip(app, cpu))
            return false;
    racePrecision(app, cpu);
    return true;
}

/* 0x495230: loadingProgress at [esp+4] per cent; stdcall's 4 bytes. */
void drawLoadingProgress(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    if (!loadingProgress(app, cpu, cpu.esp + 4))
        return;
    kept.back(cpu);
    cpu.esp += 4 + 4;
}

/* ---- The game's main ---- */

constexpr reg32 kTryInit = 0x4f7af8, kAtExitList = 0x4f7c20, kControl87 = 0x4f7c90;
constexpr reg32 kKbhit = 0x4f7ce0, kGetch = 0x4f7f24, kFflush = 0x4f7d70, kMemset = 0x4e0640;
constexpr reg32 kMessageBoxImport = 0x534764, kSetErrorModeImport = 0x5345e4;
constexpr reg32 kStartFlags = 0x55eb2e;        // bit 0x20 and 0x10: two ways of starting straight into a race
constexpr reg32 kDebugOutput = 0x5643fc, kErrorWhere = 0x564408;  // native_game's
constexpr reg32 kRaceState = 0x4a3690;         // eax, edx: the race's state machine (apply_race_state's sub_4a3400 on)

/* 0x4a39f0: the game's main, eax and edx the command line's: its exception
 * frame (Watcom's __TryInit; the handlers at 0x4a3afd and 0x4a3f7d are not
 * reached on the port), the checks before it starts (a box if another copy
 * runs), the settings' start-up ways (bits of [0x55eb2e], [0x55eb2f]: the
 * x87's exceptions unmasked), the menus and races in turn until the game
 * quits -- each menu at the menus' x87 precision, each race at its own --,
 * the screen cleared and the sounds stopped, and a key waited for if one is
 * pending; eax the frame's old link. */
void gameMain(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x54;
    const reg32 ebp = cpu.ebp;
    auto local = [&](reg32 offset) { return at(app, ebp - offset); };
    auto setLocal = [&](reg32 offset, reg32 value) { put(app, ebp - offset, value); };
    auto call = [&](reg32 target) { return byAddress(app, cpu, target); };
    auto word = [&](reg32 address, reg32 value) { putHalf(app, address, reg16(value)); };
    // The exception frame's link put back, and out.
    auto leave = [&]() {
        cpu.eax = local(0x54);
        put(app, cpu.efs, cpu.eax);
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    setLocal(8, cpu.eax);
    setLocal(0x1c, cpu.edx);
    cpu.eax = ebp - 0x54;
    cpu.edx = 0x55e8cd;
    if (!call(kTryInit))
        return;
    setLocal(0x48, cpu.edx);
    setLocal(0x20, 0xb);
    cpu.ebx = 0;
    setLocal(0xc, cpu.ebx);
    cpu.eax = 0x4a4020;
    if (!call(kAtExitList) || !call(0x4a38c0))
        return;
    if (cpu.eax != 0)
    {
        // Another copy running: a box, and out.
        cpu.ecx = 6;
        cpu.edi = ebp - 0x38;
        cpu.esi = 0x4a3050;
        for (; cpu.ecx != 0; --cpu.ecx)
            movsd(app, cpu);
        cpu.esi = local(0x38);
        if (!guest::windows<kMessageBoxImport, &win32::user32::MessageBoxA>(app, cpu, cpu.ebx, cpu.esi, 0x53ce54, 0x30))
            return;
        return leave();
    }
    cpu.edx = 0x1f;
    cpu.eax = 0x55e898;
    if (!call(0x4a38f0))
        return;
    if (cpu.eax == 0)
    {
        if (!call(0x4b5e50))
            return;
        return leave();
    }
    push(app, cpu, 0x55e8ba);
    push(app, cpu, 0x55e8b8);
    cpu.eax = ebp - 0x10;
    push(app, cpu, cpu.eax);
    push(app, cpu, 0x53ce68);
    push(app, cpu, 0x55e898);
    if (!call(0x4f0548))                         // sscanf
        return;
    cpu.esp += 0x14;
    if (cpu.eax == 0)
    {
        if (!call(0x4b5e50))
            return;
        return leave();
    }
    cpu.edx = local(0x1c);
    cpu.eax = local(8);
    if (!call(0x4a5080))
        return;
    if (cpu.eax == 0)
        return leave();
    if (!guest::windows<kSetErrorModeImport, &win32::kernel32::SetErrorMode>(app, cpu, 2))
        return;
    cpu.ah = 0;
    putByte(app, ebp - 0x44, cpu.ah);
    cpu.eax = 0x190;                             // the probe's read of [0x190] is skipped (SKIP_INSTRUCTIONS)
    putByte(app, ebp - 4, cpu.al);
    cpu.al = byte(app, ebp - 4);
    putByte(app, ebp - 4, reg8(byte(app, ebp - 4) + 1));
    putByte(app, ebp - 0x44, 0xff);
    cpu.edi = 1;
    cpu.bl = 1;
    put(app, kDebugOutput, cpu.edi);
    if (!guest::windows<kSetErrorModeImport, &win32::kernel32::SetErrorMode>(app, cpu, 2))
        return;
    putByte(app, ebp - 0x44, cpu.bl);
    // The two ways of starting straight into a race: the menus' settings.
    if (byte(app, kStartFlags) & 0x20)
    {
        cpu.ebx = 0x12;
        cpu.eax = 0x556382;
        cpu.edx = 0;
        cpu.cl = 0x32;
        if (!call(kMemset))
            return;
        cpu.edi = 0;
        cpu.esi = 0;
        cpu.edx = 1;
        cpu.al = 0x34;
        putByte(app, 0x55b041, cpu.cl);
        word(0x5562b8, cpu.edi);
        word(0x556220, cpu.esi);
        word(0x556222, cpu.edi);
        word(0x556264, cpu.esi);
        cpu.ebx = 0;
        cpu.ch = 0x33;
        word(0x556388, cpu.edx);
        word(0x55637c, cpu.ebx);
        putByte(app, 0x55b043, cpu.al);
        cpu.ah = 0x38;
        cpu.dl = 0x3b;
        cpu.bl = 0x40;
        putByte(app, 0x55b042, cpu.ch);
        cpu.ecx = 1;
        putByte(app, 0x55b044, cpu.ah);
        putByte(app, 0x55b045, cpu.dl);
        putByte(app, 0x55b047, cpu.bl);
        putByte(app, 0x55b048, cpu.ah);
        cpu.dh = 0x3f;
        for (const reg32 at16 : {0x5562b4u, 0x5562b6u})
            word(at16, cpu.ecx);
        cpu.ebx = cpu.ecx;
        for (const reg32 at16 : {0x55628au, 0x556224u, 0x556208u, 0x556262u})
            word(at16, cpu.ecx);
        cpu.eax = 0;
        putByte(app, 0x55b046, cpu.dh);
        put(app, 0x554f50, cpu.eax);
        cpu.bl ^= cpu.cl;
        cpu.eax = 0;
        word(0x55620a, cpu.ebx);
        for (const reg32 at16 : {0x556286u, 0x556288u, 0x55621eu})
            word(at16, cpu.eax);
    }
    if (byte(app, kStartFlags) & 0x10)
    {
        cpu.ebx = 0x12;
        cpu.eax = 0x556382;
        cpu.edi = 1;
        cpu.edx = 0;
        cpu.ch = 0x32;
        if (!call(kMemset))
            return;
        cpu.esi = 0;
        cpu.dl = 0x38;
        cpu.bl = 0x3f;
        word(0x556388, cpu.edi);
        putByte(app, 0x55b041, cpu.ch);
        word(0x5562b4, cpu.edi);
        cpu.eax = 0;
        word(0x556286, cpu.esi);
        word(0x55637c, cpu.eax);
        putByte(app, 0x55b044, cpu.dl);
        putByte(app, 0x55b046, cpu.bl);
        cpu.edi = 0;
        cpu.al = 0x33;
        cpu.dh = 0x3b;
        cpu.bh = 0x40;
        word(0x556288, cpu.edi);
        putByte(app, 0x55b042, cpu.al);
        putByte(app, 0x55b045, cpu.dh);
        putByte(app, 0x55b047, cpu.bh);
        putByte(app, 0x55b048, cpu.dl);
        cpu.ah = 0x34;
        cpu.ebx = 0;
        cpu.edx = 0;
        putByte(app, 0x55b043, cpu.ah);
        word(0x5562b6, cpu.ebx);
        word(0x5562b8, cpu.ebx);
        put(app, 0x554f50, cpu.edx);
        word(0x556220, cpu.ebx);
        word(0x556222, cpu.ebx);
        cpu.eax = 1;
        word(0x556264, cpu.ebx);
        word(0x55628a, cpu.eax);
        cpu.edx = cpu.eax;
        for (const reg32 at16 : {0x55621eu, 0x556224u, 0x556208u})
            word(at16, cpu.eax);
        cpu.al = 0;
        word(0x556262, cpu.edx);
        word(0x55620a, cpu.eax);
    }
    // The x87's exceptions unmasked, for a debugging start.
    if (byte(app, kStartFlags + 1) & 2)
        for (const reg32 mask : {4u, 2u, 1u, 8u, 0x10u})
        {
            cpu.edx = mask;
            cpu.eax = 0;
            if (!call(kControl87))
                return;
        }
    if (!call(0x4a6e10))
        return;
    setLocal(0x14, cpu.eax);
    cpu.ecx = 0;
    put(app, kErrorWhere, cpu.ecx);
    if (local(0x14) == 0)
    {
        cpu.eax = 0;
        if (!call(0x4b5e50))
            return;
    }
    if (!call(0x4b6320) || !call(0x4a3390))
        return;
    if (byte(app, kStartFlags) & 0x20)
    {
        cpu.eax = 1;
        cpu.ecx = 0;
        cpu.esi = 0;
        cpu.edi = 0;
        cpu.edx = 0;
        cpu.ebx = 0;
        word(0x554fac, cpu.ecx);
        word(0x55500e, cpu.esi);
        word(0x55513c, cpu.edi);
        word(0x55519a, cpu.eax);
        word(0x55519c, cpu.edx);
        word(0x55519e, cpu.ebx);
        word(0x5551a0, cpu.ecx);
        word(0x5551a2, cpu.eax);
        word(0x5551a4, cpu.eax);
        cpu.eax = gameSettings(app).language;
        cpu.esi = 0x49bfb0;
        if (!call(0x4d1750))
            return;
        put(app, 0x56446c, cpu.esi);
        if (!call(0x472d10))
            return;
    }
    if (byte(app, kStartFlags) & 0x10)
    {
        cpu.ecx = 1;
        cpu.eax = 0;
        cpu.esi = 0;
        cpu.edi = 0;
        cpu.edx = 0;
        for (const reg32 at16 : {0x554faeu, 0x555010u, 0x55513cu})
            word(at16, cpu.eax);
        word(0x55519a, cpu.ecx);
        word(0x55519c, cpu.esi);
        word(0x5551a0, cpu.eax);
        word(0x5551a2, cpu.eax);
        cpu.eax = 0;
        word(0x55519e, cpu.edi);
        if (!call(kRaceState))
            return;
    }
    else
        // The menus and races in turn, by what each says comes next, until 10.
        while (local(0x20) != 0xa)
        {
            cpu.eax = 0xd;
            if (!call(0x451940))
                return;
            cpu.eax = local(0x20) - 6;
            if (cpu.eax > 6)
                continue;
            switch (cpu.eax)
            {
            case 0:
            case 1:
                cpu.edx = local(0x20) == 7 ? 1 : 0;
                cpu.eax = local(0xc);
                if (!call(kRaceState))
                    return;
                setLocal(0x20, cpu.eax);
                setLocal(0xc, 2);
                break;
            case 5:
                menuPrecision(app, cpu);
                if (!call(0x4730d0))
                    return;
                setLocal(0x20, cpu.eax);
                racePrecision(app, cpu);
                cpu.eax = 0;
                setLocal(0xc, cpu.eax);
                break;
            case 6:
                menuPrecision(app, cpu);
                if (!call(0x4732b0))
                    return;
                setLocal(0x20, cpu.eax);
                cpu.edx = 1;
                racePrecision(app, cpu);
                setLocal(0xc, cpu.edx);
                break;
            default:
                break;
            }
        }
    // The game quits: the screen cleared black and flipped, the sounds stopped.
    if (!call(0x43bcf0) || !call(0x4d1880))
        return;
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, kThrashWindow)) || !setState(app, cpu, thrash::kStateClearColour, 0)
        || !byAddress(app, cpu, at(app, kThrashClearWindow)) || !byAddress(app, cpu, at(app, kThrashPageFlip))
        || !call(0x4f0f40) || !call(0x40fed0) || !call(0x439b70))
        return;
    setLocal(0x18, cpu.eax);
    cpu.dl = 0xff;
    if (!call(0x4a33a0))
        return;
    putByte(app, ebp - 0x44, cpu.dl);
    // A key pending: each read, until none.
    if (!call(kKbhit))
        return;
    if (cpu.eax != 0)
    {
        bool reading = true;
        while (reading)
        {
            if (!call(kGetch))
                return;
            if (cpu.eax != 0)
            {
                if (!call(kKbhit))
                    return;
                reading = cpu.eax != 0;
                continue;
            }
            if (!call(kGetch) || !call(kKbhit))
                return;
            reading = cpu.eax != 0;
        }
    }
    cpu.eax = kStandardError;
    if (!call(kFflush) || !call(0x4f2a7d))
        return;
    leave();
}

/* ---- The cabin's instruments ---- */

constexpr reg32 kCabinTextures = 0x7922fc;     // sub_4d4ae0's: the cabin's textures, 0 for none
constexpr reg32 kCabinFlags = 0x55eb2e;        // a byte
constexpr reg8 kNoCabin = 0x20;                // kCabinFlags': no cabin view
constexpr reg32 kDriverDirect3d = 0x40;        // kDriverFlags' bit for Direct3D

// THRASH_about's record as the cabin reads it: +0x70 the texture memory's bytes; +0x74 a word read
// both as flags (bit 0: 2 MB will do, bit 15: any will do) and, on Direct3D, as more bytes to add;
// the texture widths it takes from +0x10 to +0x14, the heights from +0x1c to +0x20.
constexpr reg32 kAboutTextureBytes = 0x70, kAboutMoreBytes = 0x74, kAboutAnyWillDo = 0x75;
constexpr reg32 kAboutWidestLeast = 0x10, kAboutWidestMost = 0x14;
constexpr reg32 kAboutHighestLeast = 0x1c, kAboutHighestMost = 0x20;
constexpr sreg32 kCabinArtSide = 0x100;        // the dash's art: 256 by 256
constexpr sreg32 kCabinBytes = 0x400000, kCabinBytesWithFlag = 0x200000;

// The car's dash: "%s%s\car.viv" of the cars' folder and the car's name, its chunk "dash.qfs".
constexpr reg32 kCarModel = 0x220, kCarModelNumber = 0x14;
constexpr reg32 kCabinArchiveName = 0x53b6dc, kCarsFolder = 0x7a28b4;
constexpr reg32 kLoadAlignment = 0x10;
constexpr reg32 kDashChunk = 0x53b6ec;
constexpr reg32 kDashTag = 0x53b6f8;           // what the dash's block is allocated as ("")
constexpr reg32 kUnpackAlignment = 0x10;

// The dash's shapes: its textures (sub_4d4ae0, from "0000" to "0001", records of 0x2c from
// 0x791d70, +4 the texture), and the gear's eight pictures (sub_4d4080: "geaR", "geaN", "gea1" to
// "gea6", eight bytes apart, records of 0x2c from 0x791c10).
constexpr reg32 kShapeTextures = 0x4d4ae0;     // eax the shapes, edx and ebx two names, ecx the records; the format and an address on the stack
constexpr reg32 kDashFirstShape = 0x53b704, kDashLastShape = 0x53b6fc;
constexpr reg32 kDashTextures = 0x791d70;
constexpr reg32 kShapeRecordBytes = 0x2c, kShapeRecordTexture = 4;
constexpr reg32 kRecordData = 0x4f2460;        // eax a record: what it holds
constexpr reg32 kShapeCount = 0x4ea654;        // eax the shapes: how many
constexpr reg32 kShapeTexture = 0x4d4080;      // eax the shapes, edx a name, ebx 0x20, ecx the record; 3 on the stack
constexpr reg32 kGearShapeNames = 0x53b70c, kGearShapeNameBytes = 8;
constexpr reg32 kGearTextures = 0x791c10;
constexpr reg32 kGearPictures = 8, kGearTextureSide = 0x20, kGearTextureKind = 3;
constexpr reg32 kFindShape = 0x4ef1e0;         // eax the shapes, edx a name: the shape
constexpr reg32 kShapePlace = 0xc;             // a shape's place: two 12-bit signed numbers
constexpr reg32 kCabinGear = 0x55d6a4;         // whether the gear's pictures are loaded
constexpr reg32 kCabinGearX = 0x55d69c, kCabinGearY = 0x55d6a0;

// The gauges' twelve words of eight bytes, from the dash: the first the gauges' art's place (x and y
// as 8.8 in the dash's textures), then the two needles.
constexpr reg32 kCabinGauges = 0x792030, kCabinGaugesBytes = 0x60;
constexpr reg32 kGaugesTexture = 0x7922e4;     // the dash's texture they are in
constexpr reg32 kGaugesU = 0x7922d8, kGaugesV = 0x7922d4;
constexpr reg32 kTexel = 0x53b75c;             // 1/256, a float
constexpr reg32 kHalfTexel = 0x53b764;         // 1/512, a double

// A needle's words: its pivot, its scale's two values (+0xc: the speedometer's in km/h, else mph)
// and the dial's points they are at, x and y each.
constexpr reg32 kNeedlePivotX = 0, kNeedlePivotY = 4, kNeedleKmh = 0xc;
constexpr reg32 kNeedleLow = 0x10, kNeedleHigh = 0x14;
constexpr reg32 kNeedleLowX = 0x18, kNeedleLowY = 0x1c, kNeedleHighX = 0x20, kNeedleHighY = 0x24;
constexpr reg32 kCabinNeedle = 0x792040;       // the speedometer's
constexpr reg32 kCabinOtherNeedle = 0x792068;  // the other's, its scale as it is
constexpr reg32 kKmhToMetres = 0x53b74c, kMphToMetres = 0x53b750;  // 1/3.6 and 0.44704, floats
constexpr reg32 kNeedleShown = 0x7922d0, kOtherNeedleShown = 0x7922e8;
constexpr reg32 kAtan2 = 0x4ea9c0, kSquareRoot = 0x4ea920;  // a float on the stack each, onto the x87's stack
constexpr reg32 kOneDown = 0x53b754;           // -1.0f
constexpr reg32 kHalfOfIt = 0x53b758;          // 0.5f

namespace
{

// Where a needle's results go: turns a unit, its turn at 0 and its length.
struct NeedleSet
{
    reg32 slope, start, length;
};
constexpr NeedleSet kNeedleSet = {0x7922e0, 0x7922f4, 0x7922dc};
constexpr NeedleSet kOtherNeedleSet = {0x7922ec, 0x7922f0, 0x7922f8};

/* carstate::targetOfCamera (0x422bd0) of player 0: the player's car record. */
std::optional<reg32> playerCar(App* app, CPU& cpu)
{
    const reg32 car = guest::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, reg32(0));
    if (cpu.terminate)
        return std::nullopt;
    return car;
}

/* load::folderOf (0x4392b0): the name of car `number`'s folder. */
std::optional<reg32> carFolderName(App* app, CPU& cpu, reg32 number)
{
    return guest::direct<0x4392b0, load::folderOf>(app, cpu, number);
}

/* ea::loadQuietly (0x4e0fe0): the file `name` loaded into a block of its own, 0 for none (edx
 * the alignment, which it keeps: the code's after it). */
std::optional<reg32> loadedQuietly(App* app, CPU& cpu, reg32 name)
{
    cpu.edx = kLoadAlignment;
    return guest::direct<0x4e0fe0, ea::loadQuietly>(app, cpu, name, kLoadAlignment);
}

/* load::findEntry (0x4ea040): chunk `chunk` (its name) found in the archive at `archive`, its offset
 * written at `offsetAt` and another word at `otherAt`; nonzero when found.  ebx and ecx as it
 * leaves them, edx the chunk as the code hands it. */
std::optional<reg32> chunkFound(App* app, CPU& cpu, reg32 archive, reg32 chunk, reg32 offsetAt, reg32 otherAt)
{
    cpu.edx = chunk;
    const std::optional<load::BigEntry> found =
        guest::direct<0x4ea040, load::findEntry>(app, cpu, archive, chunk, reg32(0), offsetAt, otherAt);
    if (!found)
        return std::nullopt;
    cpu.ebx = found->entry;
    cpu.ecx = found->last;
    return found->name;
}

/* ea::packedSizeChecked (0x4f2710): the packed chunk at `chunk`'s bytes unpacked (ecx the chunk
 * too, as the code has it there and on to the unpacking; ebx 0x10). */
std::optional<reg32> unpackedBytes(App* app, CPU& cpu, reg32 chunk)
{
    cpu.ecx = chunk;
    cpu.ebx = kUnpackAlignment;
    return guest::direct<0x4f2710, ea::packedSizeChecked>(app, cpu, chunk);
}

/* ea::unpackChecked (0x4f2620): the packed chunk at `chunk` unpacked into `block` (edx it, as the
 * code hands it). */
bool unpacked(App* app, CPU& cpu, reg32 chunk, reg32 block)
{
    cpu.edx = block;
    return guest::direct<0x4f2620, ea::unpackChecked>(app, cpu, chunk, block).has_value();
}

/* sub_4d4ae0 by address: the dash's textures from its `shapes`, in `format`, into kDashTextures'
 * records; a record written at `recordAt`.  The textures, 0 for none. */
std::optional<reg32> dashTextures(App* app, CPU& cpu, reg32 shapes, reg32 format, reg32 recordAt)
{
    cpu.ecx = kDashTextures;
    cpu.ebx = kDashLastShape;
    cpu.edx = kDashFirstShape;
    cpu.eax = shapes;
    if (!guest::call(app, cpu, kShapeTextures, {format, recordAt}))
        return std::nullopt;
    return cpu.eax;
}

/* sub_4f2460 by address: what the record at `record` holds. */
std::optional<reg32> recordData(App* app, CPU& cpu, reg32 record)
{
    cpu.eax = record;
    return game(app, cpu, kRecordData);
}

/* sub_4ea654 by address: how many shapes `shapes` has. */
std::optional<reg32> shapeCount(App* app, CPU& cpu, reg32 shapes)
{
    cpu.eax = shapes;
    return game(app, cpu, kShapeCount);
}

/* sub_4d4080 by address: the shape named at `name` of `shapes` made a texture into `record`. */
bool shapeTexture(App* app, CPU& cpu, reg32 shapes, reg32 name, reg32 record)
{
    cpu.ecx = record;
    cpu.ebx = kGearTextureSide;
    cpu.edx = name;
    cpu.eax = shapes;
    return guest::call(app, cpu, kShapeTexture, {kGearTextureKind});
}

/* sub_4ef1e0 by address: the shape named at `name` of `shapes`. */
std::optional<reg32> shapeNamed(App* app, CPU& cpu, reg32 shapes, reg32 name)
{
    cpu.edx = name;
    cpu.eax = shapes;
    return game(app, cpu, kFindShape);
}

/* sub_4ea9c0 (stdcall: simd's angleOfFloatsValue, which leaves every register as it was): the angle
 * of (x, y) in turns, as the float the code stores it as.  In the checks (NFS_GAME_CHECKS, which do
 * not build the menus' files) the call by address, its result taken off the x87's stack. */
std::optional<float> angleOf(App* app, CPU& cpu, float y, float x)
{
#ifdef NFS_GAME_CHECKS
    if (!guest::call(app, cpu, kAtan2, {bitsOf(y), bitsOf(x)}))
        return std::nullopt;
    const float angle = float(cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    return angle;
#else
    const reg32 eax = cpu.eax;
    double angle = 0.0;
    try
    {
        angle = guest::direct<kAtan2, frontend::simd::angleOfFloatsValue>(app, cpu, bitsOf(y), bitsOf(x));
    }
    catch (const fe::Terminated&)
    {
    }
    if (cpu.terminate)
        return std::nullopt;
    cpu.eax = eax;
    return float(angle);
#endif
}

/* sub_4ea920 (stdcall: simc's squareRootOf, which leaves every register as it was): the square root
 * of `value` as it is.  In the checks (NFS_GAME_CHECKS) the call by address, its result taken off the
 * x87's stack. */
std::optional<x86::Float> rootOf(App* app, CPU& cpu, float value)
{
#ifdef NFS_GAME_CHECKS
    if (!guest::call(app, cpu, kSquareRoot, {bitsOf(value)}))
        return std::nullopt;
    const x86::Float root = cpu.fpu.st(0);
    cpu.fpu.count -= 1;
    return root;
#else
    const double root = guest::direct<kSquareRoot, frontend::simc::squareRootOf>(app, cpu, bitsOf(value));
    if (cpu.terminate)
        return std::nullopt;
    return x86::Float(root);
#endif
}

/* An integer as the float fild and fstp make of it. */
float floatOfInteger(sreg32 value)
{
    return float(x86::Float(value));
}

/* Whether the driver's texture memory and sizes take the dash's art: more than 4 MB (2 MB, or any,
 * by +0x74's flags; Direct3D's two words together first), and 256 within the widths and the heights
 * it takes.  On Direct3D's way ebx and ecx as the code leaves them: the two words, and their sum. */
bool cabinArtTaken(App* app, CPU& cpu, reg32 about, reg8 driver)
{
    bool enough = (driver & kDriverSoftware) || (byte(app, about + kAboutAnyWillDo) & 0x80);
    if (!enough && (driver & kDriverDirect3d))
    {
        cpu.ebx = at(app, about + kAboutTextureBytes);
        cpu.ecx = at(app, about + kAboutMoreBytes) + cpu.ebx;
        enough = sreg32(cpu.ecx) > kCabinBytes;
    }
    if (!enough)
        enough = ((byte(app, about + kAboutMoreBytes) & 1) && sreg32(at(app, about + kAboutTextureBytes)) > kCabinBytesWithFlag)
                 || sreg32(at(app, about + kAboutTextureBytes)) > kCabinBytes;
    return enough && sreg32(at(app, about + kAboutWidestLeast)) <= kCabinArtSide
           && sreg32(at(app, about + kAboutWidestMost)) >= kCabinArtSide
           && sreg32(at(app, about + kAboutHighestLeast)) <= kCabinArtSide
           && sreg32(at(app, about + kAboutHighestMost)) >= kCabinArtSide;
}

/* The gear's pictures, for a dash of more than two shapes: the eight textures (kCabinGear set
 * before the first), and the place of the first (its shape's 12-bit x and y) as floats; else
 * kCabinGear 0. */
bool gearLoaded(App* app, CPU& cpu, reg32 shapes)
{
    const std::optional<reg32> count = shapeCount(app, cpu, shapes);
    if (!count)
        return false;
    if (sreg32(*count) <= 2)
    {
        put(app, kCabinGear, 0);
        return true;
    }
    for (reg32 gear = 0; gear < kGearPictures; ++gear)
    {
        if (gear == 0)
            put(app, kCabinGear, 1);
        if (!shapeTexture(app, cpu, shapes, kGearShapeNames + kGearShapeNameBytes * gear,
                          kGearTextures + kShapeRecordBytes * gear))
            return false;
    }
    const std::optional<reg32> shape = shapeNamed(app, cpu, shapes, kGearShapeNames);
    if (!shape)
        return false;
    const reg32 place = at(app, *shape + kShapePlace);
    putFloat(app, kCabinGearX, floatOfInteger(sreg32(place << 20) >> 20));
    putFloat(app, kCabinGearY, floatOfInteger(sreg32(place << 4) >> 20));
    return true;
}

/* Whether a needle moves: its two points differ.  1 or 0. */
reg32 needleShown(App* app, reg32 needle)
{
    return at(app, needle + kNeedleLowX) != at(app, needle + kNeedleHighX)
                   || at(app, needle + kNeedleLowY) != at(app, needle + kNeedleHighY)
               ? 1
               : 0;
}

/* One added to an angle in turns while it is below 0: fldz and fcom's "above", which a C++ compare
 * gives on every value, NaN (never) and -0 (equal) too. */
float upFromBelow(float turn)
{
    while (0.0f > turn)
        turn = float(fp::add(1.0, turn));
    return turn;
}

/* [kOneDown] added while its bits are above 1.0's as integers -- the code's compare, kept: it loops
 * on a NaN or an infinity of the plus sign as the code does. */
float downFromAbove(App* app, float turn)
{
    while (sreg32(bitsOf(turn)) > 0x3f800000)
        turn = float(fp::add(turn, x86::Float(floatAt(app, kOneDown))));
    return turn;
}

/* The pivot to one of a needle's points, x or y: the integers' difference. */
sreg32 fromPivot(App* app, reg32 needle, reg32 point, reg32 pivot)
{
    return sreg32(at(app, needle + point) - at(app, needle + pivot));
}

struct NeedleTurns
{
    float low, high;
};

/* A needle's angles at its two points (atan2 of each against the pivot, the low's first), each
 * brought into [0, 1]. */
std::optional<NeedleTurns> needleTurns(App* app, CPU& cpu, reg32 needle)
{
    const float lowX = floatOfInteger(fromPivot(app, needle, kNeedleLowX, kNeedlePivotX));
    const float lowY = floatOfInteger(fromPivot(app, needle, kNeedleLowY, kNeedlePivotY));
    const std::optional<float> low = angleOf(app, cpu, lowY, lowX);
    if (!low)
        return std::nullopt;
    const float highX = floatOfInteger(fromPivot(app, needle, kNeedleHighX, kNeedlePivotX));
    const float highY = floatOfInteger(fromPivot(app, needle, kNeedleHighY, kNeedlePivotY));
    const std::optional<float> high = angleOf(app, cpu, highY, highX);
    if (!high)
        return std::nullopt;
    NeedleTurns turns = {upFromBelow(*low), upFromBelow(*high)};
    turns.low = downFromAbove(app, turns.low);
    turns.high = downFromAbove(app, turns.high);
    return turns;
}

/* A needle set from its scale's values `low` and `high` and its turns there: the high's turn put
 * past the low's the way the scale runs (fcom's CF: below, or either NaN), then the turns a unit,
 * the turn at 0 and its length, the mean of the pivot's distances to its two points.  The x87's
 * operations as the code does them, each float stored where it stores one. */
bool needleSet(App* app, CPU& cpu, reg32 needle, NeedleTurns turns, float low, float high, const NeedleSet& set)
{
    if (belowOrUnordered(low, high))
    {
        if (belowOrUnordered(turns.high, turns.low))
            turns.high = float(fp::add(1.0, turns.high));
    }
    else if (belowOrUnordered(turns.low, turns.high))
        turns.low = float(fp::add(1.0, turns.low));
    const x86::Float turn = fp::sub(turns.high, turns.low);
    const x86::Float span = fp::sub(high, low);
    const x86::Float perUnit = fp::div(turn, span);
    const x86::Float lowX = x86::Float(fromPivot(app, needle, kNeedleLowX, kNeedlePivotX));
    const x86::Float lowXSquared = fp::mul(lowX, lowX);
    const x86::Float lowY = x86::Float(fromPivot(app, needle, kNeedleLowY, kNeedlePivotY));
    const x86::Float lowYSquared = fp::mul(lowY, lowY);
    const float slope = float(perUnit);
    putFloat(app, set.slope, slope);
    const x86::Float atHigh = fp::mul(high, slope);
    const float highX = floatOfInteger(fromPivot(app, needle, kNeedleHighX, kNeedlePivotX));
    const float lowSquared = float(fp::add(lowXSquared, lowYSquared));
    const float highY = floatOfInteger(fromPivot(app, needle, kNeedleHighY, kNeedlePivotY));
    putFloat(app, set.start, float(fp::sub(turns.high, atHigh)));
    const std::optional<x86::Float> lowLength = rootOf(app, cpu, lowSquared);
    if (!lowLength)
        return false;
    const float highSquared = float(fp::add(fp::mul(highX, highX), fp::mul(highY, highY)));
    const float lowLengthStored = float(*lowLength);
    const std::optional<x86::Float> highLength = rootOf(app, cpu, highSquared);
    if (!highLength)
        return false;
    putFloat(app, set.length,
             float(fp::mul(fp::add(*highLength, lowLengthStored), x86::Float(floatAt(app, kHalfOfIt)))));
    return true;
}

/* The two needles, each when it moves: the speedometer's, its scale in km/h or mph made metres a
 * second (before its turns), and the other's, its scale as it is (after). */
bool needlesSet(App* app, CPU& cpu)
{
    const reg32 shown = needleShown(app, kCabinNeedle);
    put(app, kNeedleShown, shown);
    if (shown != 0)
    {
        const x86::Float toMetres =
            floatAt(app, at(app, kCabinNeedle + kNeedleKmh) != 0 ? kKmhToMetres : kMphToMetres);
        const float high = float(fp::mul(x86::Float(sreg32(at(app, kCabinNeedle + kNeedleHigh))), toMetres));
        const float low = float(fp::mul(toMetres, x86::Float(sreg32(at(app, kCabinNeedle + kNeedleLow)))));
        const std::optional<NeedleTurns> turns = needleTurns(app, cpu, kCabinNeedle);
        if (!turns || !needleSet(app, cpu, kCabinNeedle, *turns, low, high, kNeedleSet))
            return false;
    }
    const reg32 otherShown = needleShown(app, kCabinOtherNeedle);
    put(app, kOtherNeedleShown, otherShown);
    if (otherShown != 0)
    {
        const std::optional<NeedleTurns> turns = needleTurns(app, cpu, kCabinOtherNeedle);
        if (!turns)
            return false;
        const float high = floatOfInteger(sreg32(at(app, kCabinOtherNeedle + kNeedleHigh)));
        const float low = floatOfInteger(sreg32(at(app, kCabinOtherNeedle + kNeedleLow)));
        if (!needleSet(app, cpu, kCabinOtherNeedle, *turns, low, high, kOtherNeedleSet))
            return false;
    }
    return true;
}

/* The gauges' art: its texture and its place in it (the first two words, 8.8: the whole and the
 * fraction as the compiler's division by 256 makes them, as idiv's, toward zero), half a texel in.
 * The y's whole, what the code leaves in eax. */
reg32 gaugesPlaced(App* app)
{
    const sreg32 x = sreg32(at(app, kCabinGauges)), y = sreg32(at(app, kCabinGauges + 4));
    const reg32 texture = reg32(x / 0x100) * 2 + reg32(y / 0x100);
    put(app, kGaugesTexture, at(app, kDashTextures + kShapeRecordTexture + kShapeRecordBytes * texture));
    const x86::Float texel = floatAt(app, kTexel);
    const x86::Float u = fp::mul(x86::Float(x % 0x100), texel);
    const x86::Float v = fp::mul(texel, x86::Float(y % 0x100));
    const x86::Float halfTexel = x86::Float(app->getMemory<double>(kHalfTexel));
    putFloat(app, kGaugesU, float(fp::add(u, halfTexel)));
    putFloat(app, kGaugesV, float(fp::add(halfTexel, v)));
    return reg32(y / 0x100);
}

}  // namespace

/* 0x488a80's core: the cabin's instruments loaded for the car -- but with no cabin, in a network
 * race, or where the driver's texture memory and sizes do not take its art: the dash's chunk
 * unpacked from the car's archive, its textures (1555 on softtria, 4444 otherwise), the gauges'
 * twelve words, the gear's pictures if it has them; each needle that moves set; and the gauges'
 * art placed when either does.  The archive's name and the two words sub_4ea040 and sub_4d4ae0
 * write in the frame, as deep as the code has it; the registers at each call as the code has them.
 * eax as the code leaves it: THRASH_about's, the last call's, 0 with neither needle, else the
 * gauges' y's whole. */
bool cabinInstrumentsLoaded(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0x144);
    const reg32 archiveName = frame - 0x144, otherAt = frame - 0x34, offsetAt = frame - 0x38;
    const reg32 recordAt = frame - 0x40;
    const std::optional<reg32> about = thrash(app, cpu, kThrashAbout);
    if (!about)
        return false;
    put(app, kCabinTextures, 0);
    const reg8 cabin = byte(app, kCabinFlags);
    if ((cabin & kNoCabin) || gameSettings(app).playMode == 1)
        return true;
    const reg8 driver = byte(app, kDriverFlags);
    cpu.edx = cabin | reg32(driver) << 8;  // the two bytes, as the code has edx at the next calls
    if (!cabinArtTaken(app, cpu, *about, driver))
        return true;
    const std::optional<reg32> car = playerCar(app, cpu);
    if (!car)
        return false;
    const std::optional<reg32> carName = carFolderName(app, cpu, at(app, at(app, *car + kCarModel) + kCarModelNumber));
    if (!carName)
        return false;
    cpu.eax = archiveName;
    cpu.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, archiveName, kCabinArchiveName,
                                                             guest::Words{kCarsFolder, *carName});
    if (cpu.terminate)
        return false;
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, archiveName);
    if (!exists)
        return false;
    if (!*exists)
        return true;
    const std::optional<reg32> archive = loadedQuietly(app, cpu, archiveName);
    if (!archive)
        return false;
    if (*archive == 0)
        return true;
    const std::optional<reg32> found = chunkFound(app, cpu, *archive, kDashChunk, offsetAt, otherAt);
    if (!found)
        return false;
    if (*found == 0)
        return release(app, cpu, *archive);
    const reg32 chunk = at(app, offsetAt) + *archive;
    const std::optional<reg32> bytes = unpackedBytes(app, cpu, chunk);
    if (!bytes)
        return false;
    const std::optional<reg32> shapes = allocate(app, cpu, kDashTag, *bytes);
    if (!shapes)
        return false;
    if (*shapes == 0)
        return release(app, cpu, *archive);
    if (!unpacked(app, cpu, chunk, *shapes) || !release(app, cpu, *archive))
        return false;
    // Its textures, and the gauges' words.
    const reg32 format = (byte(app, kDriverFlags) & kDriverSofttria) ? kFormat1555 : kFormat4444;
    const std::optional<reg32> textures = dashTextures(app, cpu, *shapes, format, recordAt);
    if (!textures)
        return false;
    put(app, kCabinTextures, *textures);
    const std::optional<reg32> gauges = recordData(app, cpu, at(app, recordAt));
    if (!gauges)
        return false;
    // The code's movsd's, two a gauge, the direction flag clear as Watcom's code has it.
    for (reg32 offset = 0; offset < kCabinGaugesBytes; offset += 4)
        put(app, kCabinGauges + offset, at(app, *gauges + offset));
    cpu.ecx = *gauges;  // as the copy leaves them at the next call
    cpu.edx = *gauges + kCabinGaugesBytes - 8;
    if (!gearLoaded(app, cpu, *shapes) || !release(app, cpu, *shapes) || !needlesSet(app, cpu))
        return false;
    if (at(app, kNeedleShown) == 0 && at(app, kOtherNeedleShown) == 0)
    {
        cpu.eax = 0;  // the other needle's flag
        return true;
    }
    cpu.eax = gaugesPlaced(app);
    return true;
}

/* 0x488a80: cabinInstrumentsLoaded; every register but eax as the caller had it. */
void loadCabinInstruments(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx | kEsi | kEdi);
    if (!cabinInstrumentsLoaded(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- A message in the HUD ---- */

constexpr reg32 kHudBoxes = 0x724258;          // a player's: 0x280 bytes of quads, 0x20 a corner
constexpr reg32 kHudBoxTexture = 0x749a38, kHudFlashTexture = 0x8b4844;
constexpr reg32 kHudTextScale = 0x791b50;
constexpr reg32 kHalf = 0x53b5e8, kHalfAgain = 0x53b5ec;  // 0.5f each

/* setSoftwareBlendColour as the code calls it: colour eax, ebx, ecx and edx kept. */
bool blendColourKept(App* app, CPU& cpu, reg32 colour)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx);
    cpu.eax = colour;
    if (!softwareBlendColour(app, cpu, colour))
        return false;
    kept.back(cpu);
    return true;
}

// A player's HUD box: three quads, then the flashing bar's two (kCornerBytes a corner, four a quad).
constexpr reg32 kHudBoxBytes = 0x280, kHudQuadBytes = 0x80, kHudBarFirst = 0x180, kHudBarSecond = 0x200;
constexpr reg32 kHudTextFont = 0x44;
constexpr reg32 kWhite = 0xffffffff;

/* 0x482940's core: player `player`'s message box in the HUD, `text` in it: its three quads (textured
 * with [0x749a38]), the text centred in it in white at the HUD's scale, then the flashing bar over
 * it (one of two quads by the clock's half second, blended, [0x8b4844]).  The x87's operations as
 * the code does them, each float stored where it stores one; the registers at each call as the code
 * has them.  eax as the last state's answer leaves it. */
bool messageBox(App* app, CPU& cpu, reg32 player, reg32 text)
{
    const reg32 box = kHudBoxes + player * kHudBoxBytes;
    const auto quad = [&](reg32 first) {
        cpu.eax = first;
        return thrash(app, cpu, kThrashDrawQuad,
                      {first, first + kCornerBytes, first + 2 * kCornerBytes, first + 3 * kCornerBytes})
            .has_value();
    };
    if (!sendState(app, cpu, thrash::kStateTexture, at(app, kHudBoxTexture)) || !quad(box)
        || !quad(box + kHudQuadBytes) || !quad(box + 2 * kHudQuadBytes)
        || !setTextScale(app, cpu, at(app, kHudTextScale)))
        return false;
    const float modeWidth = float(x86::Float(sreg32(at(app, kModeWidth))));
    put(app, kTextColour, kWhite);
    cpu.edx = kWhite;
    const std::optional<x86::Float> width = textWidth(app, cpu, text);
    if (!width)
        return false;
    // The text's left: the box's middle less half its width (chopped).
    const x86::Float left = fp::mul(fp::sub(x86::Float(modeWidth), *width), x86::Float(floatAt(app, kHalf)));
    const float boxHeight =
        float(fp::sub(x86::Float(floatAt(app, box + 0x44)), x86::Float(floatAt(app, box + 4))));
    cpu.eax = kHudTextFont;
    const std::optional<sreg32> x = chop(app, cpu, left);
    if (!x)
        return false;
    const std::optional<reg32> textHeight = game(app, cpu, kTextHeight);
    if (!textHeight)
        return false;
    // Its top: the box's top and half of what its height leaves.
    const x86::Float spare = fp::sub(x86::Float(boxHeight), x86::Float(sreg32(*textHeight)));
    const x86::Float top =
        fp::add(fp::mul(spare, x86::Float(floatAt(app, kHalfAgain))), x86::Float(floatAt(app, box + 4)));
    cpu.edx = reg32(*x);
    cpu.eax = text;
    const std::optional<sreg32> y = chop(app, cpu, top);
    if (!y || !drawText(app, cpu, text, reg32(*x), reg32(*y)) || !byAddress(app, cpu, kFlushText)
        || !sendState(app, cpu, thrash::kStateBlend, 1)
        || !sendState(app, cpu, thrash::kStateTexture, at(app, kHudFlashTexture)))
        return false;
    const reg32 bar = box + (sreg32(at(app, kTicks) & 0x3f) < 0x20 ? kHudBarFirst : kHudBarSecond);
    if ((byte(app, kDriverFlags) & kDriverSofttria) && !blendColourKept(app, cpu, at(app, bar + kCornerColour)))
        return false;
    return quad(bar) && sendState(app, cpu, thrash::kStateBlend, 0);
}

/* 0x482940: messageBox for player eax, text edx. */
void hudMessageBox(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx);
    if (!messageBox(app, cpu, cpu.eax, cpu.edx))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- A race's frame ---- */

constexpr reg32 kRaceView = 0x7a3d14;          // the view drawn
constexpr reg32 kTimes = 0x7a3cf0;             // what each stage took, the last one's into here
constexpr reg32 kKeepTime = 0x4bc030;          // eax the view, edx kTimes
constexpr reg32 kStartTime = 0x4bc000;         // eax 1, esi where the time goes
constexpr reg32 kSoftwareRaceView = 0x7a3a60;  // 0: the software renderers' order
constexpr reg32 kRaceHudOff = 0x7d36a0, kRaceViewPaused = 0x7a3d10, kRaceCountdown = 0x7a3d18;
constexpr reg32 kRaceFrameSteps[] = {0x4dece0, 0x4dedf0, 0x4cc600};

/* 0x4bc160's core: a frame of the race's view for `pass` (0 the main one, 1 the mirror, others
 * neither): the frame's lists set up, the window cleared on the software renderers, the sky
 * (sub_4782e0), the view (sub_41b9b0), the effects, the cars, the smoke and the rest, each stage
 * timed (sub_4bc030 with what sub_4bc000 or the stage gave back); then the HUD for the main pass
 * (sub_482b00) or the mirror's frame (sub_484030), and the countdown.  A stage gives its time back
 * as Watcom returns a record: into the frame at esi, as deep as the code has it.  The registers at
 * each call as the code has them (ebx the pass throughout, edi and esi as its copies leave them);
 * eax as the last call leaves it. */
bool raceFrameDrawn(App* app, CPU& cpu, reg32 pass)
{
    const GuestFrame frame(cpu, 0x18, 0x50);
    cpu.ebx = pass;
    const auto call = [&](reg32 target) { return byAddress(app, cpu, target); };
    const auto callWith = [&](reg32 value, reg32 target) {
        cpu.eax = value;
        return byAddress(app, cpu, target);
    };
    // A stage of the view's, its time given back at `record`.
    const auto stage = [&](reg32 target, reg32 record) {
        cpu.eax = kRaceView;
        cpu.esi = record;
        return call(target);
    };
    // The time a stage gave back copied to kTimes (two movsd) and kept.
    const auto keep = [&](reg32 record) {
        cpu.edx = kTimes;
        cpu.eax = kRaceView;
        put(app, kTimes, at(app, record));
        put(app, kTimes + 4, at(app, record + 4));
        cpu.esi = record + 8;
        cpu.edi = kTimes + 8;
        return call(kKeepTime);
    };
    // A stage timed by sub_4bc000 after it.
    const auto timed = [&](reg32 record) {
        cpu.eax = 1;
        cpu.edi = kTimes;
        return call(kStartTime) && keep(record);
    };
    // The main pass's layers (native_collide's, which do nothing), eax `layer` as the code has it.
    const auto firstLayer = [&](reg32 layer) {
        cpu.eax = layer;
        guest::direct<0x431c40, collide::firstLayerNothing>(app, cpu, layer);
        return !cpu.terminate;
    };
    const auto secondLayer = [&](reg32 layer) {
        cpu.eax = layer;
        guest::direct<0x431c30, collide::secondLayerNothing>(app, cpu, layer);
        return !cpu.terminate;
    };
    const auto layers = [&](reg32 first, reg32 second) {
        return pass != 0 || (firstLayer(first) && secondLayer(second));
    };
    const auto softtria = [&]() { return (byte(app, kDriverFlags) & kDriverSofttria) != 0; };
    if (pass == 0 && !secondLayer(cpu.eax))
        return false;
    guest::direct<0x4318c0, collide::forgetStates>(app, cpu);
    if (cpu.terminate || !callWith(1, 0x4c7750) || !call(0x4c75f0) || !call(0x4c73e0))
        return false;
    if (pass == 0 && (byte(app, kDriverFlags) & kDriverSoftware) && !thrash(app, cpu, kThrashClearWindow))
        return false;
    const reg32 sky = softtria() ? frame - 0x50 : frame - 0x48;
    if (softtria())
    {
        cpu.esi = sky;
        if (!sendState(app, cpu, thrash::kStateShade, 1))
            return false;
    }
    cpu.edi = kTimes;
    if (!stage(0x4782e0, sky) || !keep(sky) || !layers(0, 1))
        return false;
    if (softtria())
    {
        const reg32 detail = gameSettings(app).detailFlag104;
        if (!sendState(app, cpu, thrash::kStateShade, detail != 0 || gameSettings(app).night == 1 ? 1 : detail))
            return false;
    }
    reg32 last;
    if (at(app, kSoftwareRaceView) == 0)
    {
        if (!callWith(kRaceView, 0x41b9b0) || !layers(1, 2) || !callWith(kRaceView, 0x4c6f00)
            || !stage(0x4bbba0, frame - 0x40))
            return false;
        cpu.edi = kTimes;
        if (!callWith(kRaceView, 0x4cc5c0) || !call(0x4c7580))
            return false;
        last = frame - 0x40;
    }
    else
    {
        if (!stage(0x41b9b0, frame - 0x38) || !timed(frame - 0x38) || !layers(1, 2)
            || !stage(0x4c6f00, frame - 0x30) || !timed(frame - 0x30) || !stage(0x4bbba0, frame - 0x28)
            || !timed(frame - 0x28))
            return false;
        cpu.edi = kTimes;
        if (!stage(0x4cc5c0, frame - 0x20) || !call(0x4c76e0))
            return false;
        last = frame - 0x20;
    }
    if (!keep(last))
        return false;
    for (reg32 step = 0; step < 3; ++step)
    {
        const reg32 record = frame - 0x18 + 8 * step;
        cpu.edi = kTimes;
        if (!stage(kRaceFrameSteps[step], record) || !keep(record))
            return false;
    }
    bool hud = true;
    if (pass == 0)
    {
        if (!callWith(kModeRect, kSetViewScale) || !guest::direct<0x431aa0, collide::textStates>(app, cpu)
            || !callWith(kRaceView, 0x482b00))
            return false;
    }
    else if (pass == 1)
    {
        if (!callWith(kRaceView, 0x4bc100) || !guest::direct<0x431aa0, collide::textStates>(app, cpu)
            || !call(0x484030))
            return false;
    }
    else
        hud = false;
    if (hud && (!guest::direct<0x431b40, collide::hudStates>(app, cpu) || !callWith(kRaceView, 0x4bc130)))
        return false;
    if (pass == 0)
    {
        if (at(app, kRaceHudOff) == 0 && at(app, kRaceViewPaused) == 0
            && !callWith(at(app, kRaceCountdown), 0x482620))
            return false;
        if (!firstLayer(2))
            return false;
    }
    return !softtria() || sendState(app, cpu, thrash::kStateShade, 0);
}

/* 0x4bc160: raceFrameDrawn for pass eax. */
void raceFrame(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEbx | kEcx | kEdx | kEsi | kEdi);
    if (!raceFrameDrawn(app, cpu, cpu.eax))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

/* ---- The reciprocals ---- */

constexpr reg32 kReciprocalsMade = 0x564ed0;
constexpr reg32 kReciprocals = 0x9ef990;       // 1024 words: 2^32 / (n * 4) + 1, the first 0
constexpr reg32 kDivide64 = 0x503760;          // native_snd_strm's div64: (low, high, divisor), stdcall

/* port::div64 (0x503760), by address: (low, high, divisor) on the stack, stdcall.  The quotient. */
std::optional<reg32> divide64(App* app, CPU& cpu, reg32 low, reg32 high, reg32 divisor)
{
    if (!guest::call(app, cpu, kDivide64, {low, high, divisor}))
        return std::nullopt;
    return cpu.eax;
}

/* 0x4eb3c0's core: the table of reciprocals made, once: for n from 1 to 1023, the 64-bit 1:0
 * divided by 4n, plus 1.  The registers at each division as the code has them: edx its count, a
 * step on, eax the last entry. */
bool reciprocalsMade(App* app, CPU& cpu)
{
    if (at(app, kReciprocalsMade) != 0)
        return true;
    put(app, kReciprocals, 0);
    for (reg32 divisor = 4; divisor != 0x1000; divisor += 4)
    {
        cpu.edx = divisor + 4;
        const std::optional<reg32> quotient = divide64(app, cpu, 0, 1, divisor);
        if (!quotient)
            return false;
        put(app, kReciprocals + divisor, *quotient + 1);
        cpu.eax = *quotient + 1;
    }
    put(app, kReciprocalsMade, at(app, kReciprocalsMade) + 1);
    return true;
}

/* 0x4eb3c0: reciprocalsMade. */
void makeReciprocals(App* app, CPU& cpu)
{
    const Kept kept(cpu, kEdx);
    if (!reciprocalsMade(app, cpu))
        return;
    kept.back(cpu);
    cpu.esp += 4;
}

}  // namespace graphics
}  // namespace nfs3hp
