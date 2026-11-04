#include "native_guest.h"
#include "diag_hooks.h"
#include "thrash.h"

#include <cstdint>
#include <cstdio>

/* The game's graphics that were still generated: what drives the THRASH
 * driver itself (voodoo2a, the port's own in port_thrash.cpp) -- its states,
 * the frame and the window, the textures, the screen modes -- one function at
 * a time, each where its generated code was.  disassemble_nfs3hp.py binds the
 * addresses of decompiled.json named in "graphics" to the functions of those
 * names here (DECOMPILED_NATIVES), and the generated code calls them by name.
 *
 * Written as their code is, as the other natives: each push and pop to the
 * guest's stack, each call with the return address's slot, the registers as
 * the code leaves them, results in eax.  What they call of the driver they
 * call through the game's pointers to it (0x9ef900 on, thrash.h's order), of
 * the game's by address.  tools/game_checks holds each against its generated
 * code, with every one of the driver's functions stood in by its name. */
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

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
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

/* THRASH_setstate(state, value), as the code pushes it: value, then state. */
bool setState(win32::WinApplication* app, x86::CPU& cpu, reg32 state, reg32 value)
{
    push(app, cpu, value);
    push(app, cpu, state);
    return byAddress(app, cpu, at(app, kThrashSetState));
}

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

/* 0x431900: state eax set to edx through THRASH_setstate.  The texture, the
 * blending, the fog, the texture's addressing and the depth test are sent
 * only when they change; a new fog density or colour sends the fog's own state
 * again after it, unless that is negative.  Every other state is sent as it
 * comes.  eax: the driver's answer, or 1 for one not sent. */
void setRenderState(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.ebx = 1;
    // A kept state: sent if it changed, the driver's answer in ebx.
    auto kept = [&](reg32 keptAt, reg32 state, reg32 value) {
        if (value == at(app, keptAt))
            return true;
        push(app, cpu, value);
        push(app, cpu, state);
        put(app, keptAt, value);
        if (!byAddress(app, cpu, at(app, kThrashSetState)))
            return false;
        cpu.ebx = cpu.eax;
        return true;
    };
    // The fog's state again after its density or colour, when it is not negative.
    auto fogAgain = [&](reg32 fog) {
        if (sreg32(fog) < 0)
            return true;
        push(app, cpu, fog);
        push(app, cpu, thrash::kStateFog);
        return byAddress(app, cpu, at(app, kThrashSetState));
    };
    const reg32 state = cpu.ecx;
    bool going = true;
    if (state == thrash::kStateTexture)
        going = kept(kKeptTexture, cpu.ebx, cpu.edx);
    else if (state == thrash::kStateDepth)
        going = kept(kKeptDepth, thrash::kStateDepth, cpu.edx);
    else if (state == thrash::kStateTextureAddress)
        going = kept(kKeptTextureAddress, thrash::kStateTextureAddress, cpu.edx);
    else if (state == thrash::kStateFogDensity)
    {
        if (cpu.edx != at(app, kKeptFogDensity))
        {
            push(app, cpu, cpu.edx);
            push(app, cpu, thrash::kStateFogDensity);
            put(app, kKeptFogDensity, cpu.edx);
            if (!byAddress(app, cpu, at(app, kThrashSetState)))
                return;
            cpu.ebx = cpu.eax;
            cpu.eax = at(app, kKeptFog);
            going = fogAgain(cpu.eax);
        }
    }
    else if (state == thrash::kStateFogColour)
    {
        if (cpu.eax != at(app, kKeptFogColour))
        {
            push(app, cpu, cpu.eax);
            push(app, cpu, cpu.ecx);
            put(app, kKeptFogColour, cpu.eax);
            if (!byAddress(app, cpu, at(app, kThrashSetState)))
                return;
            cpu.ecx = at(app, kKeptFog);
            cpu.ebx = cpu.eax;
            going = fogAgain(cpu.ecx);
        }
    }
    else if (state == thrash::kStateFog)
        going = kept(kKeptFog, thrash::kStateFog, cpu.eax);
    else if (state == thrash::kStateBlend)
        going = kept(kKeptBlend, cpu.ecx, cpu.edx);
    else
    {
        push(app, cpu, cpu.eax);
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kThrashSetState)))
            return;
        cpu.ebx = cpu.eax;
    }
    if (!going)
        return;
    cpu.eax = cpu.ebx;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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

/* 0x432390: with the blending on (kKeptBlend 1), the rows for colour eax
 * (0xAARRGGBB) chosen -- each channel times the alpha, >> 11 -- and sent with
 * kSoftwareBlendDraw. */
void setSoftwareBlendColour(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (at(app, kKeptBlend) == 1)
    {
        cpu.ecx = reg32(sreg32(cpu.eax) >> 0x18) & 0xff;
        cpu.eax = (cpu.eax & 0xff) * cpu.ecx;
        cpu.eax = reg32(sreg32(cpu.eax) >> 0xb) << 6;
        cpu.ebx = kBlendTable + cpu.eax;
        cpu.eax = (reg32(sreg32(cpu.edx) >> 0x10) & 0xff) * cpu.ecx;
        put(app, kBlendRowBlue, cpu.ebx);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0xb) << 6;
        cpu.ebx = kBlendTable + cpu.eax;
        cpu.eax = (reg32(sreg32(cpu.edx) >> 8) & 0xff) * cpu.ecx;
        put(app, kBlendRowRed, cpu.ebx);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0xb);
        cpu.ebx = at(app, kScreenBits);
        cpu.eax <<= 5;
        if (cpu.ebx == 0xf)
        {
            cpu.edx = kBlendTable;
            cpu.eax += cpu.eax;
        }
        else
        {
            cpu.edx = kBlendTableGreen;
            cpu.eax <<= 2;
        }
        cpu.edx += cpu.eax;
        put(app, kBlendRowGreen, cpu.edx);
        if (!setState(app, cpu, kStateSoftwareBlend, kSoftwareBlendDraw))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x432370: the software renderers' blending off (kStateSoftwareBlend 0). */
void clearSoftwareBlend(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!setState(app, cpu, kStateSoftwareBlend, 0))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The frame ---- */

/* 0x434a00: a frame ended: the window (THRASH_window 2) but on PowerVR, the
 * picture flushed if edx, flipped if eax, the window again, and on a card
 * that is neither PowerVR nor a software renderer, the window cleared. */
void endFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    auto window = [&]() {
        if (half(app, kDriverFlags) & kDriverPowerVr)
            return true;
        push(app, cpu, 2);
        return byAddress(app, cpu, at(app, kThrashWindow));
    };
    if (!window())
        return;
    if (cpu.ebx != 0 && !byAddress(app, cpu, at(app, kThrashFlushWindow)))
        return;
    if (cpu.esi != 0 && !byAddress(app, cpu, at(app, kThrashPageFlip)))
        return;
    if (!window())
        return;
    if (!(byte(app, kDriverFlags) & kDriverSoftware) && !(half(app, kDriverFlags) & kDriverPowerVr)
        && !byAddress(app, cpu, at(app, kThrashClearWindow)))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434b20: the driver waited for (THRASH_sync 0) and the window taken
 * (THRASH_window 2). */
void syncWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kThrashSync)))
        return;
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434b40: the picture flushed and flipped. */
void flushAndFlip(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, at(app, kThrashFlushWindow)) || !byAddress(app, cpu, at(app, kThrashPageFlip)))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x5244e0: THRASH_idle; eax 0. */
void driverIdle(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    if (!byAddress(app, cpu, at(app, kThrashIdle)))
        return;
    cpu.eax = 0;
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- Clearing and flipping ---- */

constexpr reg32 kDriverOnCard = 0x7a3a60;      // 1 but on the software renderers (sub_4b59c0)
constexpr reg16 kDriverSofttria = 0x02;        // kDriverFlags' bit for softtria

/* 0x4d5c90: the window cleared with the depth test as the driver has it, and
 * the depth test off after. */
void clearWithDepth(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = thrash::kStateDepth;
    cpu.edx = at(app, kDriverOnCard);
    if (!direct(app, cpu, setRenderState) || !byAddress(app, cpu, at(app, kThrashClearWindow)))
        return;
    cpu.eax = thrash::kStateDepth;
    cpu.edx = 0;
    if (!direct(app, cpu, setRenderState))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

constexpr reg32 kDrawingBegun = 0x563b30;      // set by beginDrawing, once
constexpr reg32 kResetTextures = 0x4d5c40;

/* 0x4d5cc0: drawing begun, once: the textures reset first if eax; then each
 * buffer (THRASH_window 2, then 1) cleared if edx. */
void beginDrawing(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.edx;
    if (at(app, kDrawingBegun) == 0)
    {
        put(app, kDrawingBegun, 1);
        if (cpu.eax != 0 && !byAddress(app, cpu, kResetTextures))
            return;
        for (const reg32 buffer : {2u, 1u})
        {
            push(app, cpu, buffer);
            if (!byAddress(app, cpu, at(app, kThrashWindow)))
                return;
            if (cpu.ebx != 0 && !direct(app, cpu, clearWithDepth))
                return;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* Both buffers cleared: the window taken, then twice cleared, flushed and
 * flipped. */
bool clearTwice(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return false;
    for (int i = 0; i < 2; ++i)
        if (!direct(app, cpu, clearWithDepth) || !byAddress(app, cpu, at(app, kThrashFlushWindow))
            || !byAddress(app, cpu, at(app, kThrashPageFlip)))
            return false;
    return true;
}

/* 0x4d5d30: both buffers cleared. */
void clearBothBuffers(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!clearTwice(app, cpu))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4d5d70: both buffers cleared, then the textures reset. */
void clearBothBuffersAndReset(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!clearTwice(app, cpu) || !byAddress(app, cpu, kResetTextures))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4d9120: the window taken (THRASH_window 2) and, but on softtria, cleared. */
void takeWindowAndClear(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    if (!(byte(app, kDriverFlags) & kDriverSofttria) && !direct(app, cpu, clearWithDepth))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

constexpr reg32 kTicks = 0x79c1a4;             // the game's clock (native_frontend_loop.cpp's)
constexpr reg32 kLastFlipTick = 0x56415c;      // the clock at the last flipAndWaitFrames
constexpr reg32 kWaitForTicks = 0x4c9860;

/* 0x4d9140: the picture flushed and flipped, then sub_4c9860 waited in with
 * the clock in eax and edx until it is two ticks past the last flip's, which
 * it is kept as. */
void flipAndWaitFrames(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, at(app, kThrashFlushWindow)) || !byAddress(app, cpu, at(app, kThrashPageFlip)))
        return;
    cpu.eax = at(app, kTicks);
    cpu.edx = cpu.eax;
    for (;;)
    {
        cpu.eax = at(app, kLastFlipTick) + 2;
        if (sreg32(cpu.eax) <= sreg32(cpu.edx))
            break;
        cpu.eax = at(app, kTicks);
        cpu.edx = cpu.eax;
        if (!byAddress(app, cpu, kWaitForTicks))
            return;
    }
    put(app, kLastFlipTick, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- Drawn whole ---- */

/* A rectangle's four corners (0x20 bytes each: x, y as floats, the colour at
 * +0x10), and drawFlatRectangle's. */
constexpr reg32 kRectCorners = 0x5599a0;
constexpr reg32 kCornerBytes = 0x20, kCornerColour = 0x10;

void storeTop(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    app->getMemory<float>(address) = float(cpu.fpu.pop());
}

/* 0x459350: a rectangle drawn flat, untextured: corners (eax, edx) and
 * (ebx, ecx), the colour [esp+4] at each.  Its corners clockwise from the
 * first; eax THRASH_drawquad's answer. */
void drawFlatRectangle(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.ebx;
    cpu.ebx = at(app, cpu.ebp + 0xc);
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 0xc, cpu.esi);
    put(app, cpu.ebp - 8, cpu.edx);
    put(app, cpu.ebp - 4, cpu.ecx);
    for (reg32 corner = 3; corner < 4; --corner)
        put(app, kRectCorners + corner * kCornerBytes + kCornerColour, cpu.ebx);
    auto& fpu = cpu.fpu;
    for (const reg32 local : {0x10u, 0xcu, 8u, 4u})
        fpu.push(x86::Float(sreg32(at(app, cpu.ebp - local))));
    cpu.eax = 1;
    std::swap(fpu.st(0), fpu.st(3));
    storeTop(app, cpu, kRectCorners);                      // the first corner's x: eax
    std::swap(fpu.st(0), fpu.st(1));
    storeTop(app, cpu, kRectCorners + kCornerBytes);       // the second's x: ebx
    storeTop(app, cpu, kRectCorners + 4);                  // the first's y: edx
    storeTop(app, cpu, kRectCorners + 2 * kCornerBytes + 4);  // the third's y: ecx
    cpu.edx = 0;
    for (const reg32 at4 : {kRectCorners + kCornerBytes, kRectCorners, kRectCorners + 4,
                            kRectCorners + 2 * kCornerBytes + 4})
        fpu.push(x86::Float(app->getMemory<float>(at4)));
    std::swap(fpu.st(0), fpu.st(3));
    storeTop(app, cpu, kRectCorners + 2 * kCornerBytes);       // the third's x: ebx
    std::swap(fpu.st(0), fpu.st(1));
    storeTop(app, cpu, kRectCorners + 3 * kCornerBytes);       // the fourth's x: eax
    storeTop(app, cpu, kRectCorners + kCornerBytes + 4);       // the second's y: edx
    storeTop(app, cpu, kRectCorners + 3 * kCornerBytes + 4);   // the fourth's y: ecx
    if (!direct(app, cpu, setRenderState))
        return;
    for (reg32 corner = 3; corner < 4; --corner)
        push(app, cpu, kRectCorners + corner * kCornerBytes);
    if (!byAddress(app, cpu, at(app, kThrashDrawQuad)))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* The picture drawn whole on the screen: its four corners and the mesh that
 * makes them one quad. */
constexpr reg32 kPictureCorners = 0x55e60c, kPictureMesh = 0x55e68c;
constexpr reg32 kPollMessages = 0x4f0120;

/* 0x49c570: a frame of one picture, texture eax, over the whole screen: the
 * window taken and cleared, the picture drawn, flushed, flipped, waited for;
 * then the game's messages seen to (sub_4f0120). */
void showPicture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, kThrashWindow)) || !byAddress(app, cpu, at(app, kThrashClearWindow)))
        return;
    push(app, cpu, cpu.ebx);
    if (!byAddress(app, cpu, at(app, kThrashSetTexture)))
        return;
    push(app, cpu, kPictureMesh);
    push(app, cpu, kPictureCorners);
    push(app, cpu, 1);
    if (!byAddress(app, cpu, at(app, kThrashDrawQuadMesh)) || !byAddress(app, cpu, at(app, kThrashFlushWindow))
        || !byAddress(app, cpu, at(app, kThrashPageFlip)))
        return;
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kThrashSync)) || !byAddress(app, cpu, kPollMessages))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

constexpr reg32 kTextureFormatFor = 0x49c5c0;

/* 0x49c600: a texture allocated (THRASH_talloc): eax and edx as given, its
 * format from sub_49c5c0 (eax). */
void allocTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, kTextureFormatFor))
        return;
    push(app, cpu, 0);
    push(app, cpu, 0);
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ecx);
    if (!byAddress(app, cpu, at(app, kThrashTAlloc)))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* The screen's size, and the rectangle drawn in (native_frontend_screen's
 * kViewport): left, top, width, height. */
constexpr reg32 kScreenWidth = 0x7cda68, kScreenHeight = 0x7cda6c;
constexpr reg32 kClipLeft = 0x7cda70, kClipTop = 0x7cda74, kClipWidth = 0x7cda78, kClipHeight = 0x7cda7c;
constexpr reg32 kSetViewCorners = 0x4bf1b0;    // left, top, right, bottom as floats, then two more

/* 0x4bebe0: the rectangle drawn in set to x eax, y edx, width ebx, height
 * ecx, kept on the screen: THRASH_clip given its corners, sub_4bf1b0 them as
 * floats with 1 and 0.002. */
void setClip(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    if (sreg32(cpu.eax) < 0)
        cpu.eax = 0;
    if (sreg32(cpu.edx) < 0)
        cpu.edx = 0;
    cpu.esi = at(app, kScreenWidth) - 1;
    if (sreg32(cpu.eax) >= sreg32(cpu.esi))
        cpu.eax = cpu.esi;
    put(app, kClipLeft, cpu.eax);
    cpu.eax = at(app, kScreenHeight) - 1;
    if (sreg32(cpu.edx) >= sreg32(cpu.eax))
        cpu.edx = cpu.eax;
    cpu.eax = at(app, kScreenWidth);
    put(app, kClipTop, cpu.edx);
    cpu.eax -= at(app, kClipLeft);
    if (sreg32(cpu.ebx) < sreg32(cpu.eax))
        cpu.eax = cpu.ebx;
    cpu.ebx = at(app, kClipTop);
    put(app, kClipWidth, cpu.eax);
    cpu.eax = at(app, kScreenHeight) - cpu.ebx;
    if (sreg32(cpu.ecx) < sreg32(cpu.eax))
        cpu.eax = cpu.ecx;
    cpu.esi = at(app, kClipTop);
    put(app, kClipHeight, cpu.eax);
    cpu.eax += cpu.esi;
    cpu.edi = at(app, kClipWidth);
    push(app, cpu, cpu.eax);
    cpu.eax = at(app, kClipLeft) + cpu.edi;
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.esi);
    cpu.edx = at(app, kClipLeft);
    push(app, cpu, cpu.edx);
    if (!byAddress(app, cpu, at(app, kThrashClip)))
        return;
    push(app, cpu, 0x3b0355ad);  // 0.002f
    push(app, cpu, 0x3f800000);  // 1.0f
    cpu.ecx = at(app, kClipHeight);
    cpu.ebx = at(app, kClipWidth);
    // Each fild and fstp: a corner as a float -- the bottom, the right
    // (through [ebp-4]), the top and the left.
    auto pushFloat = [&](reg32 address) {
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(sreg32(at(app, address)));
    };
    cpu.eax = at(app, kClipTop) + cpu.ecx;
    put(app, cpu.ebp - 4, cpu.eax);
    pushFloat(cpu.ebp - 4);
    cpu.eax = at(app, kClipLeft) + cpu.ebx;
    put(app, cpu.ebp - 4, cpu.eax);
    pushFloat(cpu.ebp - 4);
    pushFloat(kClipTop);
    pushFloat(kClipLeft);
    if (!byAddress(app, cpu, kSetViewCorners))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* The view and what surrounds it. */
constexpr reg32 kViewWidth = 0x7cdba0;         // the view's rectangle, from 0x7cdb98: x, y, width, height
constexpr reg32 kViewSurroundFrames = 0x7cdba8;  // counted down at each drawViewSurround
constexpr reg32 kWideScreen = 0x6fbc4c;        // Wide Screen in the Graphics menu: the view between bars
constexpr reg32 kGameMode = 0x6fd3b0;          // 1: two players on one screen
constexpr reg32 kSurroundCorners = 0x55fed4, kSurroundMesh = 0x560054;

/* 0x4be8e0: the quads around the view, untextured, without the depth test --
 * two of them in split screen, four otherwise -- when the view is narrower
 * than the screen, or with Wide Screen in a race of one screen's player. */
void drawViewSurround(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kViewSurroundFrames) - 1;
    cpu.eax = at(app, kViewWidth);
    cpu.ecx = at(app, kScreenWidth);
    put(app, kViewSurroundFrames, cpu.edx);
    const bool narrower = sreg32(cpu.eax) < sreg32(cpu.ecx);
    if (narrower || (at(app, kWideScreen) != 0 && at(app, kGameMode) != 1))
    {
        cpu.eax = thrash::kStateTexture;
        cpu.edx = 0;
        if (!direct(app, cpu, setRenderState))
            return;
        cpu.eax = thrash::kStateDepth;
        cpu.edx = 0;
        if (!direct(app, cpu, setRenderState))
            return;
        push(app, cpu, kSurroundMesh);
        push(app, cpu, kSurroundCorners);
        push(app, cpu, at(app, kGameMode) == 1 ? 2 : 4);
        if (!byAddress(app, cpu, at(app, kThrashDrawQuadMesh)))
            return;
        cpu.eax = thrash::kStateDepth;
        cpu.edx = at(app, kDriverOnCard);
        if (!direct(app, cpu, setRenderState))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* The quad of a shape of the menus' art (sub_4d7160 fills its corners). */
constexpr reg32 kShapeCorners = 0x563ab0;      // four of kCornerBytes
constexpr reg32 kStateSoftwareShape = 0x2bf;
constexpr reg32 kDrawPolygon = 0x4c12e0;       // native_render's polygon, the corners in eax, edx, ebx, ecx

/* ---- The software renderer's frame buffer ---- */

constexpr reg32 kFrameLocked = 0x56aff4;       // the frame buffer locked by the game
constexpr reg32 kFrameLock = 0xa18220;         // THRASH_lockwindow's record, or none
constexpr reg32 kFrameBuffers = 0x565020;      // two buffers to free as the lock goes
constexpr reg32 kFreeBuffer = 0x509b10;

/* 0x5248f0: the frame buffer's lock let go of (THRASH_unlockwindow) and its
 * two buffers freed, if it is locked. */
void unlockFrameBuffer(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    cpu.edx = at(app, kFrameLock);
    if (cpu.edx != 0)
    {
        push(app, cpu, cpu.ecx);
        push(app, cpu, cpu.ebx);
        push(app, cpu, cpu.edx);
        if (!byAddress(app, cpu, at(app, kThrashUnlockWindow)))
            return;
        cpu.ebx = 0;
        cpu.eax = at(app, kFrameBuffers);
        put(app, kFrameLock, cpu.ebx);
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
        cpu.eax = at(app, kFrameBuffers + 4);
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
        cpu.ebx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
    }
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x5178d0: the frame buffer let go of -- unlocked straight when the game
 * locked it, by unlockFrameBuffer when the driver did -- and the window taken
 * (THRASH_window 0). */
void releaseFrameLock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    if (at(app, kFrameLocked) != 0)
    {
        push(app, cpu, cpu.esi);
        push(app, cpu, cpu.ebx);
        cpu.ebx = at(app, kFrameLock);
        push(app, cpu, cpu.ebx);
        cpu.esi = 0;
        if (!byAddress(app, cpu, at(app, kThrashUnlockWindow)))
            return;
        put(app, kFrameLock, cpu.esi);
        put(app, kFrameLocked, cpu.esi);
        cpu.ebx = pop(app, cpu);
        cpu.esi = pop(app, cpu);
    }
    else if (at(app, kFrameLock) != 0 && !direct(app, cpu, unlockFrameBuffer))
        return;
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x517990: the frame buffer let go of and the picture flipped. */
void releaseAndFlip(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    if (!direct(app, cpu, releaseFrameLock) || !byAddress(app, cpu, at(app, kThrashPageFlip)))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
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
constexpr reg32 kGetInstance = 0x4eb2e0;
constexpr reg32 kMakeWindow = 0x4cd6c0;        // eax the width, ebx whether a window of its own
constexpr reg32 kDestroyWindow = 0x4cdaa0;
constexpr reg32 kAcquireWindow = 0x4eba70;     // native_game's AcquireWindow
constexpr reg32 kAddTimer = 0x4e7520;          // eax a function, edx its argument, ebx the ticks between
constexpr reg32 kAtExit = 0x4f2a78;            // eax a function, called as the game ends
constexpr reg32 kDriverIdle = 0x5244e0;        // driverIdle
constexpr reg32 kCloseDriverAtExit = 0x524640, kCloseDriverForError = 0x524620;

/* 0x5246a0: the driver opened, unless it is already: the game's window made
 * if it has none (one of its own for eax 0), the driver given the window's
 * mode (eax), the window and the game's callbacks, and THRASH_init; if that
 * fails, the window destroyed again.  Once open, driverIdle on a timer if
 * the driver has THRASH_idle, the driver closed at the game's end and before
 * an Error's box.  eax: whether it is open. */
void openDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    auto done = [&]() {
        cpu.eax = byte(app, kDriverOpen);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.ebx = 1;
    cpu.ecx = at(app, kWantedWidth);
    cpu.edx = at(app, kWantedHeight);
    if (cpu.ecx == 0)
        cpu.ecx = 0x280;
    if (cpu.edx == 0)
        cpu.edx = 0x1e0;
    if (byte(app, kDriverOpen) != 0)
        return done();
    if (at(app, kInstance) == 0)
    {
        if (!byAddress(app, cpu, kGetInstance))
            return;
        put(app, kInstance, cpu.eax);
    }
    if (at(app, kGameWindow) == 0)
    {
        cpu.ebx = cpu.esi == 0 ? 1 : 0;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kMakeWindow))
            return;
        cpu.ebx = cpu.eax;
    }
    if (cpu.ebx == 0)
        return done();
    if (!setState(app, cpu, thrash::kStateWindowMode, cpu.esi) || !byAddress(app, cpu, kAcquireWindow)
        || !setState(app, cpu, thrash::kStateWindow, cpu.eax)
        || !setState(app, cpu, thrash::kStateLambdas, kLambdasRecord)
        || !byAddress(app, cpu, at(app, kThrashInit)))
        return;
    if (cpu.eax == 0)
    {
        if (!byAddress(app, cpu, kDestroyWindow))
            return;
        return done();
    }
    if (at(app, kThrashIdle) != 0)
    {
        cpu.ebx = 1;
        cpu.eax = kDriverIdle;
        cpu.edx = cpu.ebx;
        if (!byAddress(app, cpu, kAddTimer))
            return;
    }
    if (byte(app, kCloseAtExit) == 0)
    {
        cpu.eax = kCloseDriverAtExit;
        cpu.edx = (cpu.edx & ~0xff00u) | 0x100;  // mov dh, 1
        if (!byAddress(app, cpu, kAtExit))
            return;
        putByte(app, kCloseAtExit, reg8(cpu.edx >> 8));
    }
    cpu.edx = kCloseDriverForError;
    cpu.ebx = (cpu.ebx & ~0xffu) | 1;  // mov bl, 1
    put(app, kErrorLock, cpu.edx);
    putByte(app, kDriverOpen, reg8(cpu.ebx));
    done();
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

/* 0x517610: the screen set to width eax, height edx, ebx bits (0 for 640,
 * 480 and 16 bits; -1 for 1024 by 768; 15 bits as 16): the size kept
 * (sub_5247c0); the first time the driver opened -- in a window for bit 0 of
 * [0x564390] -- and what to do at the game's end and before an Error's box
 * set; THRASH_setvideomode given the mode of that width (or of the height,
 * else 1), the buffers of the last mode freed, the screens set up again and
 * what they came to kept, the registry flushed.  eax: 1, or 0 when the
 * driver would not open. */
void setVideoMode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    auto done = [&]() {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    if (at(app, kVideoReady) == 0)
    {
        cpu.edi = 0x5505b0;
        cpu.ebp = 0x5505c0;
        cpu.eax = 0x3b;
        push(app, cpu, 0x5505d0);
        put(app, 0x552190, cpu.edi);  // native_game's kWhere: the file, the function, the line
        put(app, 0x552194, cpu.ebp);
        put(app, 0x552198, cpu.eax);
        if (!byAddress(app, cpu, 0x401010))  // Error
            return;
        cpu.esp += 4;
    }
    if (cpu.ecx == 0)
        cpu.ecx = 0x280;
    else if (cpu.ecx == 0xffffffff)
        cpu.ecx = 0x400;
    if (cpu.esi == 0)
        cpu.esi = 0x1e0;
    else if (cpu.esi == 0xffffffff)
        cpu.esi = 0x300;
    if (cpu.ebx == 0 || cpu.ebx == 0xffffffff || cpu.ebx == 0xf)
        cpu.ebx = 0x10;
    cpu.edx = cpu.esi;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kKeepScreenSize))
        return;
    if (at(app, kModeSet) == 0)
    {
        cpu.eax = byte(app, kWindowedFlags) & 1 ? 0 : 1;
        if (!direct(app, cpu, openDriver))
            return;
        if (cpu.eax == 0)
            return done();
        cpu.eax = kModeAtExit;
        cpu.ebx = kModeForError;
        if (!byAddress(app, cpu, kAtExit))
            return;
        put(app, kErrorLock, cpu.ebx);
    }
    reg32 mode = 1;
    switch (cpu.ecx)
    {
    case 0x140: mode = 4; break;
    case 0x200: mode = 6; break;
    case 0x320: mode = 2; break;
    case 0x3c0: mode = 0xa; break;
    case 0x358: mode = 0xb; break;
    case 0x400: mode = 0xd; break;
    default: mode = cpu.esi == 0x1f6 ? 0x11 : cpu.esi == 0x336 ? 0x12 : 1; break;
    }
    push(app, cpu, 1);
    push(app, cpu, 2);
    push(app, cpu, mode);
    if (!byAddress(app, cpu, at(app, kThrashSetVideoMode)))
        return;
    cpu.esi = at(app, kModeBuffers);
    if (cpu.esi != 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
    }
    cpu.edi = at(app, kModeBuffers + 4);
    if (cpu.edi != 0)
    {
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
    }
    cpu.ebp = 0;
    cpu.edx = at(app, kModeBuffersMore);
    put(app, kModeBuffers, cpu.ebp);
    put(app, kModeBuffers + 4, cpu.ebp);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
    }
    cpu.ecx = at(app, kModeBuffersMore + 4);
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFreeBuffer))
            return;
    }
    cpu.ebx = 0;
    cpu.edi = at(app, kModeHandle);
    put(app, kModeBuffersMore, cpu.ebx);
    put(app, kModeBuffersMore + 4, cpu.ebx);
    if (cpu.edi != 0)
    {
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kLetGo))
            return;
        put(app, kModeHandle, cpu.ebx);
    }
    cpu.eax = 1;
    cpu.ebx = kFirstScreen;
    cpu.edx = 0;
    put(app, kModeSet, cpu.eax);
    if (!byAddress(app, cpu, kSetUpScreen))
        return;
    if (sreg32(at(app, kScreens)) > 1)
    {
        cpu.ebx = kSecondScreen;
        cpu.eax = 2;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kSetUpScreen))
            return;
    }
    put(app, kWantedWidth, at(app, kScreenSetWidth));
    cpu.eax = at(app, kScreenSetHeight);
    put(app, kWantedHeight, cpu.eax);
    cpu.eax = byte(app, kScreenSetBits);
    push(app, cpu, 0x80000000);  // HKEY_CLASSES_ROOT
    put(app, kScreenBits, cpu.eax);
    if (!byAddress(app, cpu, at(app, kRegFlushKey)))
        return;
    cpu.eax = 1;
    done();
}

/* ---- The game's end ---- */

constexpr reg32 kFpuControl = 0x79f3f4;        // the x87's control word as the game runs it
constexpr reg32 kStopSounds = 0x410aa0, kStopMoreSounds = 0x40feb0;  // native_frontend_options' kStopSound2, 4
constexpr reg32 kNothingToEnd = 0x4a6f70;      // an empty function
constexpr reg32 kEndThread = 0x4df870;         // eax a thread's record: TerminateThread, CloseHandle
constexpr reg32 kGameThread = 0x7a3aac;
constexpr reg32 kReleaseWindowThing = 0x4cdc50;
constexpr reg32 kForceFeedbackOff = 0x471fb0;  // native_controls_race's: every device's actuators off
constexpr reg32 kInputStopped = 0x434fd0;      // native_controls_input's: DirectInput stopped

/* 0x4b6250: what the game does as it ends: the x87 reset to its own control
 * word, the sounds stopped, the thread at 0x7a3aac ended, sub_4cdc50's
 * release, the force feedback off, DirectInput stopped and the screen given
 * back to Windows (THRASH_restore). */
void shutDown(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.fpu.init();
    cpu.fpu.setControl(half(app, kFpuControl));
    if (!byAddress(app, cpu, kStopSounds) || !byAddress(app, cpu, kStopMoreSounds)
        || !byAddress(app, cpu, kNothingToEnd))
        return;
    cpu.eax = kGameThread;
    if (!byAddress(app, cpu, kEndThread) || !byAddress(app, cpu, kReleaseWindowThing)
        || !byAddress(app, cpu, kForceFeedbackOff) || !byAddress(app, cpu, kInputStopped)
        || !byAddress(app, cpu, at(app, kThrashRestore)))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The loading screen ---- */

constexpr reg32 kLoadingTextShown = 0x79f288;  // the loading text is up; cleared once "Aborted" is
constexpr reg32 kLoadingTextX = 0x79f280;      // where the loading text ends, a float (native_render)
constexpr reg32 kModeRect = 0x7cdac0;          // the mode's rectangle: x, y, width, height
constexpr reg32 kGetRect = 0x4beb20;           // eax a rectangle into edx, ebx, ecx and [esp+4]
constexpr reg32 kTwoDStates = 0x431aa0;        // the render states for 2D
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

/* 0x494bd0: a loading cut short: when the loading text is up, "... Aborted"
 * drawn after it -- in yellow (0xffe0e000), at row 451 of 480 scaled to the
 * mode -- and the picture flushed and flipped; the loading text then counts
 * as gone. */
void drawLoadingAborted(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x118;
    auto& fpu = cpu.fpu;
    const reg32 text = cpu.ebp - 0x118;
    const reg32 scale = cpu.ebp - 0xc;           // the mode's height over 480, a float
    if (at(app, kLoadingTextShown) != 0)
    {
        cpu.eax = cpu.ebp - 0x10;                // the height
        cpu.ecx = cpu.ebp - 0x14;
        cpu.ebx = cpu.ebp - 0x18;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.ebp - 8;
        cpu.eax = kModeRect;
        if (!byAddress(app, cpu, kGetRect))
            return;
        cpu.eax = at(app, cpu.ebp - 0x10);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.ecx = 0;
        const x86::Float perLine =
            fp::mul(x86::Float(sreg32(at(app, cpu.ebp - 4))), x86::Float(app->getMemory<float>(kHeightPer480)));
        put(app, kLoadingTextShown, cpu.ecx);
        app->getMemory<float>(scale) = float(perLine);
        if (!byAddress(app, cpu, kTwoDStates))
            return;
        push(app, cpu, at(app, kTextScale));
        cpu.ebx = 0xffe0e000;
        if (!byAddress(app, cpu, kSetTextScale))
            return;
        fpu.count -= 1;                          // fstp st(0): the last scale
        cpu.eax = kAbortedLine;
        put(app, kTextColour, cpu.ebx);
        put(app, kTextMeasuring, cpu.ecx);
        if (!byAddress(app, cpu, kLanguageText))
            return;
        push(app, cpu, cpu.eax);
        push(app, cpu, kDotsFormat);
        push(app, cpu, text);
        if (!byAddress(app, cpu, kSprintf))
            return;
        const x86::Float row =
            fp::mul(x86::Float(app->getMemory<float>(scale)), x86::Float(app->getMemory<float>(kAbortedRow)));
        const x86::Float x = x86::Float(app->getMemory<float>(kLoadingTextX));
        cpu.eax = text;
        cpu.esp += 0xc;
        fpu.count += 2;
        fpu.st(0) = x;
        fpu.st(1) = row;
        // Each chopped and stored as an integer: x into edx, the row into ebx.
        if (!byAddress(app, cpu, kChop))
            return;
        put(app, cpu.ebp - 4, reg32(fp::toInteger<sreg32>(fpu.st(0))));
        cpu.edx = at(app, cpu.ebp - 4);
        fpu.count -= 1;
        if (!byAddress(app, cpu, kChop))
            return;
        put(app, cpu.ebp - 4, reg32(fp::toInteger<sreg32>(fpu.st(0))));
        cpu.ebx = at(app, cpu.ebp - 4);
        fpu.count -= 1;
        if (!byAddress(app, cpu, kDrawText))
            return;
        fpu.count -= 1;                          // fstp st(0): the text's width
        if (!byAddress(app, cpu, kFlushText) || !byAddress(app, cpu, at(app, kThrashFlushWindow))
            || !byAddress(app, cpu, at(app, kThrashPageFlip)))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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
constexpr reg32 kSetPointerBounds = 0x4352d0;  // native_controls_input's setPointerBounds
constexpr reg32 kRaceEndExtra = 0x495600;      // for eax nonzero
constexpr reg32 kRaceEnds[] = {0x4bc950, 0x4bcd60, 0x42dd00, 0x478000, 0x4c4830, 0x4782d0, 0x41da60};

}  // namespace graphics

void raceRunning(bool running);  // nfs3hp_main.cpp: the port's mark of a race (tools/apply_race_state.py)

namespace graphics
{

/* ---- What the later sections share ---- */

constexpr reg32 kModeWidth = 0x7cdac8;
constexpr reg32 kTextWidth = 0x4d1480;         // port_text's textWidthWithOverhang: onto the x87's stack
constexpr reg32 kTextHeight = 0x4d1540;        // eax a font: its height
constexpr reg32 kTrace = 0x42d500;             // the game's trace, cdecl

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

/* The menus' quad (drawShapeQuad's kShapeCorners) and the rectangle its
 * corners are kept to. */
constexpr reg32 kShapeDepth = 0x563aa8, kShapeW = 0x563aac;  // each corner's +8 and +0xc, floats
constexpr reg32 kClipFar = 0x7d34f8, kClipTopEdge = 0x7d3504, kClipBottomEdge = 0x7d3500;
constexpr reg32 kClipLeftEdge = 0x7d34fc, kClipRightEdge = 0x7d350c;  // a float's bits, compared as integers

/* A corner's clip code into its +0x14, as the menus' quads work it out: 0x10
 * for a +0xc behind the far edge, else 8 above, 4 below, 1 left of, 2 right
 * of the rectangle -- each a float's bits compared as integers, a negative
 * one counting as short.  The registers as the code leaves them. */
void shapeClipCode(win32::WinApplication* app, x86::CPU& cpu, reg32 corner)
{
    cpu.edi = corner + 0x14;
    cpu.esi = corner;
    cpu.eax = at(app, cpu.esi + 0xc);
    cpu.ecx = at(app, kClipFar);
    if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) >= sreg32(cpu.ecx))
        cpu.ebx = 0x10;
    else
    {
        cpu.eax = at(app, cpu.esi + 4);
        cpu.ebx = 0;
        cpu.ecx = at(app, kClipTopEdge);
        cpu.edx = at(app, kClipBottomEdge);
        if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) < sreg32(cpu.ecx))
            cpu.ebx |= 8;
        else if (sreg32(cpu.eax) > sreg32(cpu.edx))
            cpu.ebx |= 4;
        cpu.eax = at(app, cpu.esi);
        cpu.ecx = at(app, kClipLeftEdge);
        cpu.edx = at(app, kClipRightEdge);
        if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) < sreg32(cpu.ecx))
            cpu.ebx |= 1;
        else if (sreg32(cpu.eax) > sreg32(cpu.edx))
            cpu.ebx |= 2;
    }
    putByte(app, cpu.edi, reg8(cpu.ebx));
}

void shapeClipCodes(win32::WinApplication* app, x86::CPU& cpu)
{
    for (reg32 corner = 0; corner < 4; ++corner)
        shapeClipCode(app, cpu, kShapeCorners + corner * kCornerBytes);
}

/* 0x4a35d0: the race taken down, back to the menus: their screen mode, the
 * menus' view at 90 and scale 1, the pointer kept to 640 by 480, both
 * buffers cleared to black, sub_495600 for eax nonzero, and the race's
 * own ends (sub_4bc950 on).  The port marks the race over first, as
 * tools/apply_race_state.py did in the generated code. */
void takeDownRace(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    if (diag::raceDown)
        diag::raceDown(app, "race taken down");
    raceRunning(false);
    cpu.eax = at(app, kMenuScreenMode);
    cpu.edx = 0;
    cpu.ebx = 0;
    put(app, kInRace, cpu.edx);
    if (!byAddress(app, cpu, kScreenMode))
        return;
    cpu.edx = 0x5a;
    cpu.eax = kMenuView;
    if (!byAddress(app, cpu, kSetViewAngle))
        return;
    cpu.eax = kMenuView;
    cpu.ecx = 0x3f800000;  // 1.0f
    if (!byAddress(app, cpu, kSetViewScale))
        return;
    cpu.ebx = 0;
    cpu.edx = 0;
    put(app, kViewScale, cpu.ecx);
    put(app, kViewOffset, cpu.ebx);
    cpu.eax = 0;
    cpu.ecx = 0x1e0;
    cpu.ebx = 0x280;
    if (!byAddress(app, cpu, kSetPointerBounds))
        return;
    cpu.eax = thrash::kStateClearColour;
    cpu.edx = 0;
    if (!direct(app, cpu, setRenderState))
        return;
    for (const reg32 buffer : {2u, 1u})
    {
        push(app, cpu, buffer);
        if (!byAddress(app, cpu, at(app, kThrashWindow)) || !direct(app, cpu, clearWithDepth))
            return;
    }
    if (cpu.esi != 0 && !byAddress(app, cpu, kRaceEndExtra))
        return;
    for (const reg32 end : kRaceEnds)
    {
        if (!byAddress(app, cpu, end))
            return;
        if (diag::raceDown)
        {
            char where[32];
            std::snprintf(where, sizeof where, "after sub_%x", unsigned(end));
            diag::raceDown(app, where);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The polygon lists' drawers for voodoo2a ---- */

namespace
{

constexpr reg32 kDrawLine = 0x4c0b20;          // native_render's line: eax, edx its ends
constexpr reg32 kDrawQuad = 0x4c12e0;          // native_render's polygon: corners eax, edx, ebx, ecx

/* A kept state sent as the drawers send it: the value pushed from `reg`, set
 * to `value` first, then the state; the value kept at `keptAt`. */
bool sendKept(win32::WinApplication* app, x86::CPU& cpu, reg32& reg, reg32 value, reg32 state, reg32 keptAt)
{
    reg = value;
    push(app, cpu, reg);
    push(app, cpu, state);
    put(app, keptAt, reg);
    return byAddress(app, cpu, at(app, kThrashSetState));
}

}  // namespace

/* 0x4347d0: a line of a polygon list (the record in eax, its two ends at +8
 * and +0xc) drawn untextured, blended for bit 0 of its flags (+6); eax the
 * next record. */
void drawLineRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.edx = at(app, kKeptTexture);
    cpu.edi = at(app, cpu.eax + 0xc);
    cpu.esi = at(app, cpu.eax + 8);
    if (cpu.edx != 0 && !sendKept(app, cpu, cpu.ecx, 0, thrash::kStateTexture, kKeptTexture))
        return;
    if (byte(app, cpu.ebx + 6) & 1)
    {
        if (at(app, kKeptBlend) != 1 && !sendKept(app, cpu, cpu.edx, 1, thrash::kStateBlend, kKeptBlend))
            return;
    }
    else if (at(app, kKeptBlend) != 0 && !sendKept(app, cpu, cpu.eax, 0, thrash::kStateBlend, kKeptBlend))
        return;
    cpu.edx = cpu.edi;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kDrawLine))
        return;
    cpu.eax = at(app, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x433d40: a quad of a polygon list with colours of its own (the record in
 * eax: corners at +8..+0x14, their colours at +0x40.., their texture's
 * coordinates at +0x20.. and the texture at +0x18), drawn unblended through
 * sub_4c12e0; eax the next record. */
void drawColouredQuadRecord(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    if (at(app, kKeptBlend) != 0 && !sendKept(app, cpu, cpu.ecx, 0, thrash::kStateBlend, kKeptBlend))
        return;
    for (reg32 corner = 0; corner < 4; ++corner)
    {
        cpu.edx = at(app, cpu.esi + 8 + 4 * corner);
        cpu.eax = at(app, cpu.esi + 0x40 + 4 * corner);
        put(app, cpu.edx + 0x10, cpu.eax);
    }
    cpu.ebx = at(app, cpu.esi + 0x18);
    if (cpu.ebx != 0)
    {
        for (reg32 corner = 0; corner < 4; ++corner)
            for (reg32 half = 0; half < 2; ++half)
            {
                cpu.eax = at(app, cpu.esi + 8 + 4 * corner);
                cpu.edx = at(app, cpu.esi + 0x20 + 8 * corner + 4 * half);
                put(app, cpu.eax + 0x18 + 4 * half, cpu.edx);
            }
        cpu.eax = at(app, kKeptTexture);
        cpu.edi = at(app, cpu.esi + 0x18);
        if (cpu.eax != cpu.edi && !sendKept(app, cpu, cpu.edi, cpu.edi, thrash::kStateTexture, kKeptTexture))
            return;
    }
    else if (at(app, kKeptTexture) != 0 && !sendKept(app, cpu, cpu.ebx, cpu.ebx, thrash::kStateTexture, kKeptTexture))
        return;
    cpu.ecx = at(app, cpu.esi + 0x14);
    cpu.ebx = at(app, cpu.esi + 0x10);
    cpu.edx = at(app, cpu.esi + 0xc);
    cpu.eax = at(app, cpu.esi + 8);
    if (!byAddress(app, cpu, kDrawQuad))
        return;
    cpu.eax = at(app, cpu.esi);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49bf60: a mesh of lines gathered in the record at eax (a count of ends,
 * the ends at +4, the pairs' indices at +0x8004) drawn untextured
 * (THRASH_drawlinemesh) and the count set back to 0, when there is one. */
void flushLineMesh(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    if (at(app, cpu.eax) != 0)
    {
        cpu.eax = thrash::kStateTexture;
        cpu.edx = 0;
        if (!direct(app, cpu, setRenderState))
            return;
        push(app, cpu, cpu.ebx + 0x8004);
        push(app, cpu, cpu.ebx + 4);
        cpu.eax = at(app, cpu.ebx);
        cpu.edx = reg32(sreg32(cpu.eax) >> 31);
        cpu.eax = reg32(sreg32(cpu.eax - cpu.edx) >> 1);  // the ends' count halved, towards 0
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, at(app, kThrashDrawLineMesh)))
            return;
        put(app, cpu.ebx, 0);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- Textures ---- */

constexpr reg32 kNoTextureFormat = 0x53c5f0;   // the Error's text: the driver takes neither format
constexpr reg32 kBadTextureFormat = 0x53c610;  // the Error's text: a format updateTexture does not know
constexpr reg32 kTextureBufferTag = 0x53c60c;  // what the conversion's buffer is allocated as
constexpr reg32 kAllocate = 0x4e1620;          // eax a tag, edx bytes, ebx 0x10, ecx: the block in eax
constexpr reg32 kFree = 0x4e1890;              // eax the block
constexpr reg32 kError = 0x401010;

/* 0x49c5c0: the format the game's 16-bit textures are handed to the driver
 * in: 3 (1555) when THRASH_about's formats (+0x30) take it, else 7 (4444),
 * with an Error first when they take neither. */
void textureFormatFor(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.eax = at(app, cpu.eax + 0x30);
    if (byte(app, cpu.eax + 0xc) & 1)
        cpu.eax = 3;
    else
    {
        if (!(byte(app, cpu.eax + 0x1c) & 1))
        {
            push(app, cpu, kNoTextureFormat);
            if (!byAddress(app, cpu, kError))
                return;
            cpu.esp += 4;
        }
        cpu.eax = 7;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49c620: texture eax given the pixels at edx, ebx by ecx of them, 1555:
 * straight (THRASH_tupdate) when the driver takes 1555, else turned into 4444
 * in a buffer of their own first -- the transparent ones 0, the others with
 * their colour's top four bits -- or an Error for a format it does not know. */
void updateTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.esi = cpu.eax;
    cpu.edi = cpu.edx;
    if (!direct(app, cpu, textureFormatFor))
        return;
    if (cpu.eax == 3)
    {
        push(app, cpu, 0);
        push(app, cpu, cpu.edx);
        push(app, cpu, cpu.esi);
        if (!byAddress(app, cpu, at(app, kThrashTUpdate)))
            return;
    }
    else if (cpu.eax == 7)
    {
        cpu.ebx *= cpu.ecx;
        cpu.eax = kTextureBufferTag;
        put(app, cpu.ebp - 0xc, cpu.ebx);           // the pixels' count
        cpu.edx = cpu.ebx + cpu.ebx;
        put(app, cpu.ebp - 4, cpu.edi);             // the next one to read
        cpu.ebx = 0x10;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kAllocate))
            return;
        cpu.ebx = cpu.eax;
        cpu.edx = cpu.eax;
        while (sreg32(cpu.ecx) < sreg32(at(app, cpu.ebp - 0xc)))
        {
            cpu.eax = reg32(sreg32(x86::sreg16(half(app, at(app, cpu.ebp - 4)))));
            cpu.edi = reg32(sreg32(cpu.eax) >> 0xf);
            if (cpu.edi == 0)
                putHalf(app, cpu.edx, 0);
            else
            {
                const reg32 red = reg32(sreg32(cpu.eax) >> 3) & 0xff00;
                const reg32 greenBlue = reg32(sreg32(cpu.eax) >> 2) & 0xf0f0;
                put(app, cpu.ebp - 0x10, red);
                put(app, cpu.ebp - 8, greenBlue);
                cpu.eax = reg32(sreg32(cpu.eax) >> 1) & 0xf00f;
                cpu.edi = red | greenBlue | cpu.eax;
                putHalf(app, cpu.edx, reg16(cpu.edi));
            }
            cpu.eax = at(app, cpu.ebp - 4) + 2;
            cpu.edx += 2;
            ++cpu.ecx;
            put(app, cpu.ebp - 4, cpu.eax);
        }
        push(app, cpu, 0);
        push(app, cpu, cpu.ebx);
        push(app, cpu, cpu.esi);
        if (!byAddress(app, cpu, at(app, kThrashTUpdate)))
            return;
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    else
    {
        push(app, cpu, kBadTextureFormat);
        if (!byAddress(app, cpu, kError))
            return;
        cpu.esp += 4;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

constexpr reg32 kMemoryWarnings = 0x4b55f8, kMemoryWarningNames = 0x4b5610;  // six words each
constexpr reg32 kMemoryWarningFormat = 0x53fde0;
constexpr reg32 kStandardError = 0x566f72;     // the runtime's stderr
constexpr reg32 kFprintf = 0x4f4b80;

/* 0x4b5ea0: for driver type eax (6, Direct3D, counts its +0x74 too), a line
 * to stderr when THRASH_about's texture memory (+0x70) is under 2 MB, with
 * the words of the tables at 0x4b55f8 and 0x4b5610 for edx. */
void warnTextureMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x70);
    if (cpu.esi == 6)
        cpu.eax += at(app, cpu.edx + 0x74);
    if (sreg32(cpu.eax) < 0x200000)
    {
        // rep movsd twice: the two tables into the frame.
        for (reg32 i = 0; i < 6; ++i)
        {
            put(app, cpu.ebp - 0x30 + 4 * i, at(app, kMemoryWarnings + 4 * i));
            put(app, cpu.ebp - 0x18 + 4 * i, at(app, kMemoryWarningNames + 4 * i));
        }
        cpu.ecx = 0;
        cpu.esi = at(app, cpu.ebp + cpu.ebx * 4 - 0x30);
        push(app, cpu, cpu.esi);
        push(app, cpu, cpu.eax);
        cpu.edi = at(app, cpu.ebp + cpu.ebx * 4 - 0x18);
        push(app, cpu, cpu.edi);
        push(app, cpu, kMemoryWarningFormat);
        push(app, cpu, kStandardError);
        if (!byAddress(app, cpu, kFprintf))
            return;
        cpu.esp += 0x14;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4be570: screen mode eax's width into [ecx] and height into [esp+4] from
 * THRASH_about's modes (+0x40, 0x28 bytes each), 640 by 480 past the last
 * (+0x3c); [edx] and [ebx] set to 0. */
void modeSize(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.edi = cpu.ebx;
    cpu.ebx = cpu.ecx;
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.eax, 0);
    put(app, cpu.edi, 0);
    if (sreg32(cpu.esi) > sreg32(at(app, cpu.edx + 0x3c)))
    {
        cpu.eax = at(app, cpu.ebp + 0x10);
        put(app, cpu.ebx, 0x280);
        put(app, cpu.eax, 0x1e0);
    }
    else
    {
        cpu.eax = cpu.esi * 5;
        cpu.ecx = at(app, at(app, cpu.edx + 0x40) + cpu.eax * 8);
        put(app, cpu.ebx, cpu.ecx);
        cpu.edx = at(app, cpu.edx + 0x40);
        cpu.eax = at(app, cpu.edx + cpu.eax * 8 + 4);
        cpu.edx = at(app, cpu.ebp + 0x10);
        put(app, cpu.edx, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* ---- Text ---- */

constexpr reg32 kTextQuad = 0x561b44;          // four corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr reg32 kTextBoxColour = 0x561b38;
constexpr reg32 kFontHeights = 0x83f6a0, kFont = 0x840184, kFontScale = 0x84018c;

/* 0x4d1120: a box drawn untextured behind text: at x eax, y edx, [esp+4]
 * wide (a float, times the font's scale) and the font's height (its table at
 * 0x83f6a0 for [0x840184], times the scale) high, in [0x561b38] made opaque. */
void drawTextBox(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    auto& fpu = cpu.fpu;
    auto loadFloat = [&](reg32 address) { return x86::Float(app->getMemory<float>(address)); };
    auto storeFloat = [&](reg32 address, x86::Float value) { app->getMemory<float>(address) = float(value); };
    auto loadInteger = [&](reg32 address) { return x86::Float(sreg32(at(app, address))); };
    cpu.ecx = cpu.eax;
    const x86::Float width = loadFloat(cpu.ebp + 0xc);
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.edx = at(app, kFont);
    const x86::Float scale = loadFloat(kFontScale);
    const x86::Float scaledWidth = fp::mul(width, scale);
    const x86::Float height = fp::mul(scale, loadInteger(kFontHeights + cpu.edx * 4));
    put(app, cpu.ebp - 8, cpu.ecx);
    cpu.eax = at(app, kTextBoxColour) | 0xff000000;
    const x86::Float x = loadInteger(cpu.ebp - 8);
    const x86::Float y = loadInteger(cpu.ebp - 4);
    storeFloat(kTextQuad, x);
    storeFloat(kTextQuad + 4, y);
    cpu.ecx = at(app, kTextQuad);
    storeFloat(kTextQuad + 0x24, y);
    for (reg32 corner = 3; corner < 4; --corner)
        put(app, kTextQuad + corner * kCornerBytes + kCornerColour, cpu.eax);
    const x86::Float top = loadFloat(kTextQuad + 0x24);
    cpu.edx = 0;
    cpu.eax = thrash::kStateTexture;
    put(app, cpu.ebp - 8, cpu.ecx);
    put(app, kTextQuad + 0x60, cpu.ecx);
    const x86::Float right = fp::add(scaledWidth, loadFloat(cpu.ebp - 8));
    const x86::Float bottom = fp::add(height, top);
    storeFloat(kTextQuad + 0x20, right);
    storeFloat(kTextQuad + 0x44, bottom);
    cpu.ecx = at(app, kTextQuad + 0x20);
    put(app, kTextQuad + 0x40, cpu.ecx);
    storeFloat(kTextQuad + 0x64, bottom);
    if (!direct(app, cpu, setRenderState))
        return;
    for (reg32 corner = 3; corner < 4; --corner)
        push(app, cpu, kTextQuad + corner * kCornerBytes);
    if (!byAddress(app, cpu, at(app, kThrashDrawQuad)))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* ---- The menus' art in textures ---- */

constexpr reg32 kShapeBufferTag = 0x5495c4;
constexpr reg32 kShapeToPixels = 0x4d37a0;     // eax the buffer, edx the shape, ebx and [esp+4] where its size goes

/* 0x4d5ed0: texture eax filled from shape edx (ebx by ebx pixels at most):
 * the shape turned into pixels (sub_4d37a0) in a buffer of its own, handed to
 * THRASH_tupdate from its +0x10, and the buffer freed. */
void shapeIntoTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    put(app, cpu.ebp - 8, cpu.eax);
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.edi = cpu.ebx;
    put(app, cpu.ebp - 0x10, cpu.ecx);
    cpu.eax = at(app, cpu.edx) & 0xff;
    cpu.edx = cpu.edi * cpu.edi;
    cpu.ebx = 0x10;
    cpu.edx = (cpu.edx << 2) + 0x14;
    cpu.eax = kShapeBufferTag;
    cpu.ecx = cpu.edi;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.esi = cpu.eax;
    cpu.eax = cpu.ebp - 0x10;
    cpu.ebx = cpu.ebp - 0xc;
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ebp - 4);
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kShapeToPixels))
        return;
    push(app, cpu, 0);
    cpu.eax = cpu.esi + 0x10;
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ebp - 8);
    push(app, cpu, cpu.edx);
    if (!byAddress(app, cpu, at(app, kThrashTUpdate)))
        return;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kFree))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The screen's functions, the frame buffer, the driver closed ---- */

constexpr reg32 kScreenFunctions = 0x567dc4;   // five: at exit, set mode, lock, unlock and flip, set up a screen
constexpr reg32 kLockScreen = 0x517920;

/* 0x4f8830: the screen's functions kept for the driver (0x567dc4 on), an
 * Error first when it has no THRASH_about; eax 1. */
void setScreenFunctions(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ecx = at(app, kThrashAbout);
    cpu.edx = 1;
    if (cpu.ecx == 0)
    {
        push(app, cpu, cpu.edi);
        cpu.ebx = 0x54d714;
        cpu.esi = 0x54d724;
        cpu.edi = 0xe;
        push(app, cpu, 0x54d72c);
        put(app, 0x552190, cpu.ebx);  // native_game's kWhere: the file, the function, the line
        put(app, 0x552194, cpu.esi);
        put(app, 0x552198, cpu.edi);
        if (!byAddress(app, cpu, kError))
            return;
        cpu.esp += 4;
        cpu.edi = pop(app, cpu);
    }
    cpu.eax = kModeAtExit;
    cpu.ecx = kLockScreen;
    cpu.ebx = 0x517990;  // releaseAndFlip
    cpu.esi = kSetUpScreen;
    cpu.ebp = 0x517610;  // setVideoMode
    put(app, kScreenFunctions, cpu.eax);
    put(app, kScreenFunctions + 8, cpu.ecx);
    put(app, kScreenFunctions + 0xc, cpu.ebx);
    put(app, kScreenFunctions + 0x10, cpu.esi);
    put(app, kScreenFunctions + 4, cpu.ebp);
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x517890: the window edx taken and the frame buffer locked
 * (THRASH_lockwindow) for the screen record eax: its pointer (+0x20) and
 * pitch (+0x28) from the lock, which is kept, as is the record.  eax: 1, or
 * 0 when it would not lock. */
void lockFrameBuffer(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ebx = cpu.eax;
    push(app, cpu, cpu.edx);
    if (!byAddress(app, cpu, at(app, kThrashWindow)) || !byAddress(app, cpu, at(app, kThrashLockWindow)))
        return;
    put(app, kFrameLock, cpu.eax);
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.eax);
        put(app, cpu.ebx + 0x20, cpu.edx);
        cpu.eax = at(app, cpu.eax + 4);
        put(app, kFrameLocked, cpu.ebx);
        put(app, cpu.ebx + 0x28, cpu.eax);
        cpu.eax = 1;
    }
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

constexpr reg32 kRemoveTask = 0x4e75e0;        // native_snd_stream's removeSystemTask: eax the task

/* 0x524650: the driver closed, when it is open: its idle task taken off the
 * timer, the screen given back (THRASH_restore), the game's window destroyed
 * too for eax nonzero.  eax 0. */
void closeDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.ebx = cpu.eax;
    cpu.eax = (cpu.eax & ~0xff00u) | reg32(byte(app, kDriverOpen)) << 8;
    if (byte(app, kDriverOpen) != 0)
    {
        if (at(app, kThrashIdle) != 0)
        {
            cpu.eax = kDriverIdle;
            if (!byAddress(app, cpu, kRemoveTask))
                return;
        }
        push(app, cpu, cpu.edx);
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kThrashRestore)))
            return;
        if (cpu.ebx != 0 && !byAddress(app, cpu, kDestroyWindow))
            return;
        cpu.edx &= ~0xff00u;
        putByte(app, kDriverOpen, 0);
        cpu.ecx = pop(app, cpu);
        cpu.edx = pop(app, cpu);
    }
    cpu.eax = 0;
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x524640: closeDriver with the game's window destroyed too: what the game
 * does at its end (openDriver hands it to sub_4f2a78). */
void closeDriverAtExit(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = 1;
    closeDriver(app, cpu);
}

/* ---- The drivers' pointers kept and taken back ---- */

constexpr reg32 kDriverSlot = 0x564a68;        // the slot in use
constexpr reg32 kDriverSlots = 0x564a70;       // eight of 0x8c bytes: the library, then 34 pointers
constexpr reg32 kDriverLibrary = 0x564a6c;
/* Where each word of a slot goes, after the library: the game's pointers to
 * the driver's functions, in the order the slot keeps them. */
constexpr reg32 kSlotPointers[] = {
    kThrashAbout, 0x9ef984, kThrashInit, kThrashRestore, 0x9ef944, kThrashSetVideoMode, kThrashTAlloc,
    kThrashTUpdate, kThrashTReset, kThrashWindow, kThrashClearWindow, kThrashFlushWindow, kThrashPageFlip,
    kThrashIdle, kThrashSync, kThrashClip, kThrashSetState, kThrashSetTexture, kThrashLockWindow,
    kThrashUnlockWindow, kThrashReadRect, 0x9ef914, kThrashDrawQuad, kThrashDrawQuadMesh, kThrashDrawTri,
    kThrashDrawTriMesh, 0x9ef91c, 0x9ef960, 0x9ef928, kThrashDrawLineMesh, 0x9ef908, 0x9ef910, 0x9ef900, 0x9ef904,
};

/* 0x517170: the driver of slot eax (0 for one outside 0..7) taken back: its
 * library and its pointers, the slot kept as the one in use.  eax: the
 * slot's last word. */
void useDriverSlot(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    if (sreg32(cpu.eax) >= 8 || sreg32(cpu.eax) < 0)
        cpu.eax = 0;
    put(app, kDriverSlot, cpu.eax);
    const reg32 slot = kDriverSlots + cpu.eax * 0x8c;
    put(app, kDriverLibrary, at(app, slot));
    for (reg32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, kSlotPointers[i], at(app, slot + 4 + 4 * i));
    cpu.eax = at(app, slot + 0x88);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x517370: the driver in use kept in slot eax (0 for one outside 0..7): its
 * library and its pointers.  eax: the slot times 35. */
void keepDriverSlot(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    if (sreg32(cpu.eax) >= 8 || sreg32(cpu.eax) < 0)
        cpu.eax = 0;
    cpu.eax *= 0x23;
    const reg32 slot = kDriverSlots + cpu.eax * 4;
    put(app, slot, at(app, kDriverLibrary));
    for (reg32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, slot + 4 + 4 * i, at(app, kSlotPointers[i]));
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- A screen set up ---- */

constexpr reg32 kBitmapScreen = 0x4ed820;      // eax its window, edx..: its size and depth
constexpr reg32 kMakeBitmap = 0x508b80;
constexpr reg32 kDepthCode = 0x5098d0;         // al a depth: the screen record's code for it
constexpr reg32 kRowBuffer = 0x509a30;         // eax 0, edx a pitch or depth, ebx, ecx rows: a buffer

/* 0x5179d0: screen eax set up in the record at ebx, edx its window: nothing
 * for 0; for 0xff a bitmap of the window's size (sub_4ed820, sub_508b80);
 * otherwise the driver's buffer eax locked once for its pointer, pitch, size
 * and depth (+0x20, +0x28, +4, +8, +0x1c from the lock's format), then the
 * record filled ("DNIW", its size again, which screen, the depth's code,
 * the window) with two buffers of its own (+0x2c, +0x30). */
void setUpScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 0x18;
    const reg32 frame = cpu.esp;
    auto done = [&]() {
        cpu.esp += 0x18;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, frame + 0x14, cpu.eax);
    cpu.edi = cpu.edx;
    cpu.esi = cpu.ebx;
    if (cpu.eax == 0xff)
    {
        cpu.eax = frame + 0x10;
        cpu.ecx = frame + 8;
        push(app, cpu, cpu.eax);
        cpu.eax = frame + 0xc;
        cpu.ebx = frame + 4;
        push(app, cpu, cpu.eax);
        cpu.edx = frame;
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kBitmapScreen))
            return;
        push(app, cpu, cpu.edi);
        cpu.edi = at(app, frame + 0x10);
        cpu.ebp = at(app, frame + 8);
        push(app, cpu, cpu.edi);
        cpu.ecx = at(app, frame + 0xc);
        cpu.ebx = at(app, frame + 4);
        push(app, cpu, cpu.ebp);
        cpu.edx = at(app, frame);
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kMakeBitmap))
            return;
        return done();
    }
    if (cpu.eax == 0)
        return done();
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, at(app, kThrashWindow)) || !byAddress(app, cpu, at(app, kThrashLockWindow)))
        return;
    cpu.ecx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.eax);
        put(app, cpu.ebx + 0x20, cpu.edx);
        cpu.edx = at(app, cpu.eax + 4);
        put(app, cpu.ebx + 0x28, cpu.edx);
        cpu.edx = at(app, cpu.eax + 0xc);
        put(app, cpu.ebx + 4, cpu.edx);
        cpu.edx = at(app, cpu.eax + 0x10);
        put(app, cpu.ebx + 8, cpu.edx);
        cpu.eax = at(app, cpu.eax + 8) - 2;
        // The lock's format (jmp [eax*4 + 0x5179a0]): its bits per pixel.
        switch (cpu.eax)
        {
        case 0: putByte(app, cpu.ebx + 0x1c, 8); break;
        case 1: case 9: putByte(app, cpu.esi + 0x1c, 0xf); break;
        case 2: putByte(app, cpu.ebx + 0x1c, 0x10); break;
        case 3: putByte(app, cpu.ebx + 0x1c, 0x18); break;
        case 4: putByte(app, cpu.ebx + 0x1c, 0x20); break;
        default: putByte(app, cpu.esi + 0x1c, 0x10); break;
        }
        push(app, cpu, cpu.ecx);
        if (!byAddress(app, cpu, at(app, kThrashUnlockWindow)))
            return;
    }
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    cpu.ebp = byte(app, cpu.esi + 0x1c);
    if (cpu.ebp == 0xf)
        cpu.ebp = 0x10;
    put(app, cpu.esi, 0x57494e44);  // "DNIW"
    put(app, cpu.esi + 0xc, 0);
    put(app, cpu.esi + 0x10, 0);
    put(app, cpu.esi + 0x14, at(app, cpu.esi + 4));
    put(app, cpu.esi + 0x18, at(app, cpu.esi + 8));
    putByte(app, cpu.esi + 0x1e, byte(app, frame + 0x14));
    cpu.eax = byte(app, cpu.esi + 0x1c);
    cpu.ebx = 0;
    if (!byAddress(app, cpu, kDepthCode))
        return;
    cpu.ecx = at(app, cpu.esi + 8);
    cpu.edx = at(app, cpu.esi + 0x28);
    putByte(app, cpu.esi + 0x1d, reg8(cpu.eax));
    cpu.eax = 0;
    put(app, cpu.esi + 0x34, cpu.edi);
    if (!byAddress(app, cpu, kRowBuffer))
        return;
    cpu.ebx = 3;
    cpu.edx = cpu.ebp;
    cpu.ecx = at(app, cpu.esi + 4);
    put(app, cpu.esi + 0x2c, cpu.eax);
    cpu.eax = 0;
    if (!byAddress(app, cpu, kRowBuffer))
        return;
    put(app, cpu.esi + 0x30, cpu.eax);
    done();
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

/* THRASH_about's mode `index`: 0x28 bytes from its +0x40. */
reg32 modeRecord(win32::WinApplication* app, reg32 about, reg32 index)
{
    return index * 0x28 + at(app, about + 0x40);
}

/* 0x4bed90: each of THRASH_about's modes the game can race in handed to the
 * function at eax (eax the mode, edx its width, [esp+4] whether it has three
 * buffers): at least two colour buffers (+0x14), 4:3 -- or what
 * nfs3hp::widescreenMode takes (a port) --, 15 or 16 bits, available and not
 * flagged (+0x10's bits 0 and 1), and two buffers of it within half the
 * driver's memory. */
void listScreenModes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    put(app, cpu.ebp - 0xc, cpu.eax);
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.esi = 1;
    put(app, cpu.ebp - 4, cpu.eax);
    for (;; ++cpu.esi)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if (sreg32(cpu.esi) > sreg32(at(app, cpu.eax + 0x3c)))
            break;
        cpu.ecx = at(app, at(app, cpu.ebp - 4) + 0x40);
        cpu.eax = cpu.esi * 0x28 + cpu.ecx;
        cpu.edx = at(app, cpu.eax + 0x14);
        cpu.edi = at(app, cpu.eax);
        cpu.ebx = at(app, cpu.eax + 4);
        cpu.ecx = at(app, cpu.eax + 8);
        cpu.eax = at(app, cpu.eax + 0x10);
        put(app, cpu.ebp - 8, cpu.eax);
        if (sreg32(cpu.edx) < 2)
            continue;
        cpu.edx = cpu.edi * 3;
        cpu.eax = cpu.ebx * 4;
        if (cpu.edx != cpu.eax && !widescreenMode(cpu.edi, cpu.ebx))
            continue;
        if (cpu.ecx != 0xf && cpu.ecx != 0x10)
            continue;
        const reg8 flags = byte(app, cpu.ebp - 8);
        cpu.eax = (cpu.eax & ~0xff00u) | reg32(flags) << 8;
        if (!(flags & 1) || (flags & 2))
            continue;
        cpu.eax = cpu.edi * cpu.ebx * 2;
        put(app, cpu.ebp - 0x10, cpu.eax);
        cpu.eax = at(app, kModeMemory);
        cpu.edx = reg32(sreg32(cpu.eax) >> 31);
        cpu.eax = reg32(sreg32(cpu.eax - cpu.edx) >> 1);
        if (sreg32(cpu.eax) < sreg32(at(app, cpu.ebp - 0x10)))
            continue;
        cpu.eax = cpu.esi * 0x28;
        put(app, cpu.ebp - 0x10, cpu.eax);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, at(app, cpu.ebp - 4) + 0x40) + cpu.edx;
        cpu.eax = sreg32(at(app, cpu.eax + 0x18)) >= 2 ? 1 : 0;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, at(app, cpu.ebp - 0xc)))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4bee80: the mode the Screen Size list would have for width eax: of those
 * listScreenModes takes (memory aside), the nearest in width, the first of
 * equals; nfs3hp::dynamicSavedMode for Dynamic Fullscreen's marker (a port).
 * eax: the mode, 1 for none. */
void findScreenMode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    put(app, cpu.ebp - 8, cpu.eax);
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.edx = 1;
    cpu.esi = cpu.eax;
    cpu.ebx = 0;
    for (; sreg32(cpu.edx) <= sreg32(at(app, cpu.esi + 0x3c)); ++cpu.edx)
    {
        cpu.eax = cpu.edx * 0x28;
        cpu.ecx = at(app, cpu.esi + 0x40) + cpu.eax;
        cpu.eax = at(app, cpu.ecx);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.edi = at(app, cpu.ecx + 0x14);
        put(app, cpu.ebp - 0x10, cpu.eax);
        if (sreg32(cpu.edi) < 2)
            continue;
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 0x14, cpu.eax);
        cpu.edi = cpu.eax * 3;
        cpu.eax = at(app, cpu.ebp - 0x10) << 2;
        if (cpu.edi != cpu.eax && !widescreenMode(at(app, cpu.ebp - 4), at(app, cpu.ebp - 0x10)))
            continue;
        cpu.eax = at(app, cpu.ecx + 8);
        if (cpu.eax != 0xf && cpu.eax != 0x10)
            continue;
        cpu.eax = cpu.edx * 0x28 + at(app, cpu.esi + 0x40);
        const reg8 flags = byte(app, cpu.eax + 0x10);
        cpu.ecx = (at(app, cpu.esi + 0x40) & ~0xffu) | flags;
        if (!(flags & 1) || (flags & 2))
            continue;
        cpu.eax = at(app, cpu.ebp - 4) - at(app, cpu.ebp - 8);
        if (sreg32(cpu.eax) <= 0)
            cpu.eax = 0 - cpu.eax;
        cpu.ecx = cpu.eax;
        if (cpu.ebx != 0 && sreg32(cpu.eax) >= sreg32(at(app, cpu.ebp - 0xc)))
            continue;
        cpu.ebx = cpu.edx;
        put(app, cpu.ebp - 0xc, cpu.ecx);
    }
    cpu.ebx = dynamicSavedMode(cpu.ebx, at(app, cpu.ebp - 8));
    cpu.eax = cpu.ebx != 0 ? cpu.ebx : 1;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The menus' art in textures ---- */

constexpr reg32 kBadArtFormat = 0x5495c8;
constexpr reg32 kFillZero = 0x4e070c;          // eax where, edx bytes
constexpr reg32 kFillByte = 0x4e0716;          // eax where, edx bytes, bl the byte

/* 0x4d5f40: a texture eax by eax of the driver's format [edx] (3: 1555,
 * 4: 565, 6: 8888; another an Error) allocated (THRASH_talloc), after its
 * 32 by 32 pixels -- clear, or opaque white for ebx 0 -- were made into a
 * shape and run through sub_4d37a0 with the record at edx; eax the texture. */
void makeArtTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    cpu.edi = cpu.edx;
    put(app, cpu.ebp - 0x18, cpu.ebx);
    put(app, cpu.ebp - 0x10, cpu.eax);
    cpu.eax = at(app, cpu.edi);
    cpu.edx = 0;
    if (cpu.eax == 4)
    {
        put(app, cpu.ebp - 4, 0x78);
        cpu.edx = 0x10;
    }
    else if (cpu.eax == 3)
    {
        cpu.ecx = 0x7e;
        cpu.edx = 0x10;
        put(app, cpu.ebp - 4, cpu.ecx);
    }
    else if (cpu.eax == 6)
    {
        cpu.ebx = 0x7d;
        cpu.edx = 0x20;
        put(app, cpu.ebp - 4, cpu.ebx);
    }
    else
    {
        push(app, cpu, kBadArtFormat);
        if (!byAddress(app, cpu, kError))
            return;
        cpu.esp += 4;
    }
    // The bytes of a pixel (edx bits / 8, towards 0) times 32 by 32.
    cpu.eax = reg32(sreg32(cpu.edx + (sreg32(cpu.edx) < 0 ? 7 : 0)) >> 3) << 0xa;
    cpu.edx = sreg32(cpu.edx) < 0 ? 0xfffffff8 : 0;
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax += 0x14;
    cpu.ebx = 0x10;
    put(app, cpu.ebp - 0x14, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = kShapeBufferTag;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = 0x10;
    cpu.edx = 0x1014;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.eax;
    cpu.eax = kShapeBufferTag;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.edx = at(app, cpu.ebp - 0x14);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.edx = at(app, cpu.ebp - 0x18);
    cpu.eax = cpu.ecx + 0x10;
    if (cpu.edx == 0)
    {
        cpu.ebx = 0xff;
        cpu.edx = at(app, cpu.ebp - 0xc);
        if (!byAddress(app, cpu, kFillByte))
            return;
    }
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.ebx = cpu.ebp - 0x18;
    push(app, cpu, cpu.edi);
    cpu.edx = cpu.esi;
    putByte(app, cpu.esi, 0);
    cpu.eax &= 0xff;
    cpu.ecx = at(app, cpu.esi) | cpu.eax;
    putHalf(app, cpu.esi + 4, 0x20);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.esi, cpu.ecx);
    cpu.ecx = 0x20;
    putHalf(app, cpu.esi + 6, 0x20);
    if (!byAddress(app, cpu, kShapeToPixels))
        return;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kFree))
        return;
    cpu.eax = at(app, cpu.ebp - 8);
    if (!byAddress(app, cpu, kFree))
        return;
    push(app, cpu, 0);
    push(app, cpu, 0);
    cpu.ebx = at(app, cpu.edi);
    push(app, cpu, cpu.ebx);
    cpu.esi = at(app, cpu.ebp - 0x10);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.esi);
    if (!byAddress(app, cpu, at(app, kThrashTAlloc)))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- Strings as the game's inlined strcpy and strlen do them ---- */

namespace
{

/* The inlined strcpy: from esi to edi two bytes a turn, al the last byte. */
void copyStringInline(win32::WinApplication* app, x86::CPU& cpu)
{
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            return;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            return;
    }
}

/* repne scasb: es:[edi] on until al, ecx counting down. */
void scanForAl(win32::WinApplication* app, x86::CPU& cpu)
{
    while (cpu.ecx != 0)
    {
        const bool found = byte(app, cpu.ees + cpu.edi) == cpu.al;
        cpu.edi += cpu.flags.df ? reg32(0) - 1 : 1;
        --cpu.ecx;
        if (found)
            break;
    }
}

}  // namespace

/* ---- The screen saved to a file (Alt+P) ---- */

constexpr reg32 kCaptureTrace = 0x5401c8;
constexpr reg32 kCaptureFolder = 0x4b6520;     // the folder pictures go in
constexpr reg32 kCaptureName = 0x5401d8;       // "%s...%d...": the folder, the number
constexpr reg32 kCaptureNumber = 0x5600f8;
constexpr reg32 kFileExists = 0x4e0e60;
constexpr reg32 kPaused = 0x6fd3a0;            // 2: paused
constexpr reg32 kRedrawPaused = 0x4bc680;
constexpr reg32 kNewBitmap = 0x4ef190, kSaveBitmap = 0x4eb160;
constexpr reg32 kModeWidthPixels = 0x7cdae0, kModeHeightPixels = 0x7cdae4, kModeBits = 0x7cdae8;

/* 0x4c4490: the screen saved as a picture -- but in a network race -- under
 * the first number not yet taken in the capture folder: a paused race drawn
 * again first, the front buffer read (THRASH_readrect) into a bitmap of the
 * mode's size, and the bitmap saved and freed. */
void captureScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x300;
    const reg32 folder = cpu.ebp - 0x300, name = cpu.ebp - 0x100;
    auto done = [&]() {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    push(app, cpu, kCaptureTrace);
    if (!byAddress(app, cpu, kTrace))
        return;
    cpu.edx = at(app, kGameMode);
    cpu.esp += 4;
    if (sreg32(cpu.edx) >= 2)
        return done();
    if (!byAddress(app, cpu, kCaptureFolder))
        return;
    cpu.edi = folder;
    cpu.esi = cpu.eax;
    push(app, cpu, cpu.edi);
    copyStringInline(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edi = folder;
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    scanForAl(app, cpu);
    cpu.ecx = ~cpu.ecx - 1;                      // its length
    if (cpu.ecx == 0)
        return done();
    do
    {
        cpu.ecx = at(app, kCaptureNumber) + 1;
        push(app, cpu, cpu.ecx);
        push(app, cpu, folder);
        push(app, cpu, kCaptureName);
        cpu.eax = name;
        push(app, cpu, cpu.eax);
        put(app, kCaptureNumber, cpu.ecx);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0x10;
        cpu.eax = name;
        if (!byAddress(app, cpu, kFileExists))
            return;
    } while (cpu.eax != 0);
    cpu.esi = at(app, kPaused);
    if (cpu.esi == 2)
    {
        put(app, kPaused, cpu.eax);
        if (!byAddress(app, cpu, kRedrawPaused) || !byAddress(app, cpu, at(app, kThrashFlushWindow))
            || !byAddress(app, cpu, at(app, kThrashPageFlip)))
            return;
        put(app, kPaused, cpu.esi);
    }
    cpu.ecx = 0x10;
    cpu.ebx = at(app, kModeBits);
    cpu.edx = at(app, kModeHeightPixels);
    cpu.eax = at(app, kModeWidthPixels);
    if (!byAddress(app, cpu, kNewBitmap))
        return;
    cpu.ebx = cpu.eax;
    if (cpu.eax == 0)
        return done();
    if (!byAddress(app, cpu, 0x4eada0))
        return;
    cpu.eax = 0;
    if (!byAddress(app, cpu, 0x4ef1b0))
        return;
    push(app, cpu, 1);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    cpu.eax = at(app, cpu.ebx + 0x20) + 0x10;
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, kModeHeightPixels);
    push(app, cpu, cpu.edx);
    cpu.ecx = at(app, kModeWidthPixels);
    push(app, cpu, cpu.ecx);
    push(app, cpu, 0);
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kThrashReadRect)))
        return;
    cpu.eax = name;
    if (!byAddress(app, cpu, kSaveBitmap))
        return;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kLetGo))
        return;
    done();
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
constexpr reg32 kRaceFrameEnd = 0x434a90;

/* 0x4c5c00: line 0x115 of the game's text (with its ending) in a box in the
 * middle of the view, but while paused: drawn into both buffers, a flip
 * after each. */
void showWaitMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x64;
    const reg32 ebp = cpu.ebp;
    auto local = [&](reg32 offset) { return at(app, ebp - offset); };
    auto setLocal = [&](reg32 offset, reg32 value) { put(app, ebp - offset, value); };
    auto halved = [](reg32 value) { return reg32(sreg32(value - reg32(sreg32(value) >> 31)) >> 1); };
    auto& fpu = cpu.fpu;
    auto done = [&]() {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = ebp - 0x18;
    cpu.ebx = ebp - 8;
    cpu.edx = ebp - 0x14;
    cpu.eax = ebp - 0xc;
    if (!byAddress(app, cpu, kViewRect))
        return;
    if (sreg32(at(app, kPaused)) > 1)
        return done();
    if (!direct(app, cpu, syncWindow))
        return;
    cpu.eax = kMessageLine;
    cpu.edi = ebp - 0x64;
    if (!byAddress(app, cpu, kLanguageText))
        return;
    cpu.esi = cpu.eax;
    push(app, cpu, cpu.edi);
    copyStringInline(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = kMessageEnding;
    cpu.edi = ebp - 0x64;
    push(app, cpu, cpu.edi);
    cpu.ecx = 0xffffffff;
    cpu.al = 0;
    scanForAl(app, cpu);
    --cpu.edi;
    copyStringInline(app, cpu);
    cpu.edi = pop(app, cpu);
    // The text at the view's height over 480.
    cpu.eax = local(8);
    setLocal(4, cpu.eax);
    const x86::Float scale = fp::mul(fp::mul(x86::Float(sreg32(local(4))), x86::Float(app->getMemory<float>(kMessageScale))),
                                     x86::Float(app->getMemory<float>(kMessageScaleAgain)));
    cpu.esp -= 4;
    app->getMemory<float>(cpu.esp) = float(scale);
    if (!byAddress(app, cpu, kSetTextScale))
        return;
    cpu.eax = ebp - 0x64;
    fpu.count -= 1;                              // fstp st(0): the scale
    if (!byAddress(app, cpu, kTextWidth) || !byAddress(app, cpu, kChop))
        return;
    cpu.eax = kFont87;
    setLocal(0x1c, reg32(fp::toInteger<sreg32>(fpu.st(0))));
    fpu.count -= 1;
    if (!byAddress(app, cpu, kTextHeight))
        return;
    cpu.ecx = cpu.eax;
    cpu.eax = kFont87;
    if (!byAddress(app, cpu, kFontLineHeight))
        return;
    cpu.ecx += cpu.eax;
    // The text's place: the view's middle less half the text.
    cpu.ebx = halved(local(8));
    cpu.esi = cpu.ebx - halved(local(0x1c));
    setLocal(0x10, halved(local(0x18)));
    cpu.edx = reg32(sreg32(cpu.ecx) >> 31);
    cpu.eax = halved(cpu.ecx);
    cpu.edi = local(0x10) - cpu.eax;
    // The box: three quarters of the text's width either side, a line up and down.
    cpu.eax = local(0x1c);
    setLocal(4, cpu.eax);
    cpu.eax = cpu.eax * 3;
    cpu.eax = reg32(sreg32(cpu.eax + (sreg32(cpu.eax) < 0 ? 3 : 0)) >> 2);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebx - cpu.edx;
    setLocal(4, cpu.eax);
    cpu.edx += cpu.ebx;
    app->getMemory<float>(ebp - 0x2c) = float(x86::Float(sreg32(local(4))));
    setLocal(4, cpu.edx);
    app->getMemory<float>(ebp - 0x34) = float(x86::Float(sreg32(local(4))));
    setLocal(0x3c, local(0x2c));
    setLocal(0x24, local(0x34));
    cpu.eax = local(0x10) - cpu.ecx;
    setLocal(4, cpu.eax);
    app->getMemory<float>(ebp - 0x20) = float(x86::Float(sreg32(local(4))));
    setLocal(0x38, local(0x20));
    cpu.eax = local(0x10) + cpu.ecx;
    setLocal(4, cpu.eax);
    app->getMemory<float>(ebp - 0x30) = float(x86::Float(sreg32(local(4))));
    cpu.eax = local(0x30);
    push(app, cpu, 2);
    setLocal(0x28, cpu.eax);
    if (!byAddress(app, cpu, at(app, kThrashWindow)))
        return;
    // A box: its colour and 1, or its border's and 0, and its corners' words.
    auto box = [&](reg32 colour, reg32 filled) {
        push(app, cpu, colour);
        push(app, cpu, filled);
        for (const reg32 offset : {0x28u, 0x2cu, 0x30u, 0x34u, 0x20u, 0x24u, 0x38u, 0x3cu})
            push(app, cpu, local(offset));
        cpu.ecx = local(0x20);
        cpu.ebx = local(0x24);
        cpu.eax = local(0x38);
        cpu.edx = local(0x3c);
        return true;
    };
    for (int pass = 0; pass < 2; ++pass)
    {
        if (!box(kBoxColour, 1) || !byAddress(app, cpu, kDrawBox) || !box(kBorderColour, 0))
            return;
        if (pass == 1)
            cpu.ebx = cpu.edi;
        if (!byAddress(app, cpu, kDrawBox))
            return;
        cpu.eax = ebp - 0x64;
        if (pass == 0)
            cpu.ebx = cpu.edi;
        cpu.edx = cpu.esi;
        if (!byAddress(app, cpu, kDrawText))
            return;
        fpu.count -= 1;                          // fstp st(0): the text's width
        if (!byAddress(app, cpu, kFlushText) || !byAddress(app, cpu, at(app, kThrashFlushWindow))
            || !byAddress(app, cpu, at(app, kThrashPageFlip)))
            return;
    }
    if (!byAddress(app, cpu, kRaceFrameEnd))
        return;
    done();
}

/* ---- The HUD editor's frame ---- */

constexpr reg32 kFrameMesh = 0x559a20;         // twelve corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr reg32 kFrameScaleX = 0x539b94, kFrameScaleY = 0x539b9c, kFrameThickness = 0x539b98;
constexpr reg32 kFrameMarginX = 0x539ba0, kFrameMarginY = 0x539ba4;
constexpr reg32 kFrameAll = 0x559ba0, kFrameClippedBoth = 0x559c40;  // the lines' index lists
constexpr reg32 kFrameClippedLeft = 0x559be0, kFrameClippedRight = 0x559c10;

namespace
{

/* fcomp, fnstsw ax, sahf: the comparison of a with b, ax the status word;
 * true for CF (a below b, or unordered). */
bool belowAx(x86::CPU& cpu, x86::Float a, x86::Float b)
{
    cpu.fpu.compare(a, b);
    cpu.ax = cpu.fpu.status.word;
    return cpu.ah & 1;
}

/* And ZF too (a equal to b). */
bool belowOrEqualAx(x86::CPU& cpu, x86::Float a, x86::Float b)
{
    cpu.fpu.compare(a, b);
    cpu.ax = cpu.fpu.status.word;
    return cpu.ah & 0x41;
}

}  // namespace

/* 0x459440: the HUD editor's frame about element eax (its left at +6 and
 * right at +0x60, top at +8 and bottom at +0x64, made room about by the
 * mode's width), drawn as a mesh of lines in colour edx -- the side that is
 * off the screen left out. */
void hudEditorFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    auto loadFloat = [&](reg32 address) { return x86::Float(app->getMemory<float>(address)); };
    auto storeFloat = [&](reg32 address, x86::Float value) { app->getMemory<float>(address) = float(value); };
    auto word16 = [&](reg32 address) { return x86::Float(x86::sreg16(half(app, address))); };
    auto integer = [&](reg32 address) { return x86::Float(sreg32(at(app, address))); };
    cpu.ecx = cpu.eax;
    const x86::Float width = integer(kModeWidth);
    const x86::Float scaledX = fp::mul(width, loadFloat(kFrameScaleX));
    const x86::Float scaledY = fp::mul(width, loadFloat(kFrameScaleY));
    const x86::Float thickness = loadFloat(kFrameThickness);
    const x86::Float roomX = fp::mul(scaledX, thickness);
    const x86::Float roomY = fp::mul(thickness, scaledY);
    const x86::Float right = integer(cpu.eax + 0x60);
    const x86::Float left = word16(cpu.ecx + 6);
    cpu.eax = 0;
    storeFloat(ebp - 0x14, fp::add(left, loadFloat(kFrameMarginX)));
    storeFloat(ebp - 0x10, fp::add(right, loadFloat(kFrameMarginY)));
    storeFloat(ebp - 0x1c, roomX);
    storeFloat(ebp - 0x18, roomY);
    do
    {
        cpu.ebx = cpu.eax << 5;
        ++cpu.eax;
        put(app, kFrameMesh + kCornerColour + cpu.ebx, cpu.edx);
    } while (sreg32(cpu.eax) < 0xc);
    // The left edge kept on the screen.
    if (belowAx(cpu, 0.0, loadFloat(ebp - 0x14)))
    {
        cpu.eax = at(app, ebp - 0x14);
        put(app, ebp - 8, cpu.eax);
    }
    else
    {
        cpu.edx = 0;
        put(app, ebp - 8, cpu.edx);
    }
    const x86::Float rightEdge = loadFloat(ebp - 0x10);
    const x86::Float frameLeft = word16(cpu.ecx + 6);
    cpu.eax = at(app, ebp - 8);
    const x86::Float frameRight = integer(cpu.ecx + 0x60);
    for (const reg32 corner : {11u, 10u, 1u, 0u})
        put(app, kFrameMesh + corner * kCornerBytes, cpu.eax);
    x86::Float room = loadFloat(ebp - 0x1c);
    storeFloat(kFrameMesh + 0x120, fp::add(frameLeft, room));
    storeFloat(kFrameMesh + 0x100, fp::sub(frameRight, room));
    const x86::Float innerLeft = loadFloat(kFrameMesh + 0x120);
    cpu.eax = at(app, kModeWidth);
    const x86::Float innerRight = loadFloat(kFrameMesh + 0x100);
    --cpu.eax;
    storeFloat(kFrameMesh + 0x40, innerLeft);
    put(app, ebp - 4, cpu.eax);
    storeFloat(kFrameMesh + 0x60, innerRight);
    storeFloat(ebp - 0x20, integer(ebp - 4));
    // The right edge kept on the screen.
    cpu.eax = belowAx(cpu, rightEdge, loadFloat(ebp - 0x20)) ? at(app, ebp - 0x10) : at(app, ebp - 0x20);
    put(app, ebp - 0xc, cpu.eax);
    storeFloat(ebp - 0x24, word16(cpu.ecx + 8));
    storeFloat(ebp - 4, integer(cpu.ecx + 0x64));
    cpu.edx = 0;
    const x86::Float top = loadFloat(ebp - 0x24);
    const x86::Float bottom = loadFloat(ebp - 4);
    for (const reg32 corner : {7u, 6u, 5u, 4u})
        put(app, kFrameMesh + corner * kCornerBytes, cpu.eax);
    cpu.eax = at(app, ebp - 0x24);
    room = loadFloat(ebp - 0x18);
    storeFloat(kFrameMesh + 0xa4, fp::add(top, room));
    storeFloat(kFrameMesh + 0x164, fp::sub(bottom, room));
    const x86::Float innerTop = loadFloat(kFrameMesh + 0xa4);
    const x86::Float innerBottom = loadFloat(kFrameMesh + 0x164);
    for (const reg32 corner : {0x4u, 0x3u, 0x2u, 0x1u})
        put(app, kFrameMesh + corner * kCornerBytes + 4, cpu.eax);
    cpu.eax = at(app, ebp - 4);
    storeFloat(kFrameMesh + 4, innerTop);
    for (const reg32 corner : {0xau, 0x9u, 0x8u, 0x7u})
        put(app, kFrameMesh + corner * kCornerBytes + 4, cpu.eax);
    cpu.eax = thrash::kStateTexture;
    storeFloat(kFrameMesh + 0xc4, innerBottom);
    if (!direct(app, cpu, setRenderState))
        return;
    // Which sides are on the screen: the lines of all, or of those.
    cpu.edx = belowOrEqualAx(cpu, 0.0, loadFloat(ebp - 0x14)) ? 0 : 1;
    cpu.eax = at(app, kModeWidth) - 1;
    put(app, ebp - 4, cpu.eax);
    cpu.eax = belowAx(cpu, integer(ebp - 4), loadFloat(ebp - 0x10)) ? 1 : 0;
    cpu.ecx = cpu.eax;
    reg32 lines, count;
    if (cpu.edx == 0 && cpu.eax == 0)
        lines = kFrameAll, count = 8;
    else if (cpu.edx != 0 && cpu.ecx != 0)
        lines = kFrameClippedBoth, count = 4;
    else
        lines = cpu.edx != 0 ? kFrameClippedLeft : kFrameClippedRight, count = 6;
    push(app, cpu, lines);
    push(app, cpu, kFrameMesh);
    push(app, cpu, count);
    if (!byAddress(app, cpu, at(app, kThrashDrawLineMesh)))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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

/* The inlined strlen of the string at edi (ecx its length, edi past its end). */
void stringLength(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    scanForAl(app, cpu);
    cpu.ecx = ~cpu.ecx - 1;
}

}  // namespace

constexpr reg32 kLoadingRace = 0x6fd4f4, kRaceRecords = 0x6fd540;   // 0x6c bytes a race
constexpr reg32 kLoadingPictures = 0x57126b, kPictureChoices = 0x552938;
constexpr reg32 kLoadingPath = 0x7a3134;
constexpr reg32 kLoadingFlags = 0x6fd4fc, kLoadingCar = 0x6fd4ac;
constexpr reg32 kLoadingScreen = 0x494cb0;     // native_render's loadingScreen: eax the picture

/* 0x494fa0: the loading screen put up as a race loads (but with it turned
 * off): the race's track's picture -- one of its several in turn, or the
 * car's when there is none or [0x6fd4fc] asks for it -- loaded and shown in
 * both buffers; the textures let go of, and the race's x87 precision set. */
void showLoadingScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x114;
    cpu.ebp -= 0x192;
    const reg32 ebp = cpu.ebp;
    const reg32 name = ebp + 0x7e, track = ebp + 0x17e;
    auto done = [&]() {
        cpu.esp = cpu.ebp + 0x192;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (at(app, kLoadingOff) != 0)
        return done();
    menuPrecision(app, cpu);
    cpu.edx = at(app, kLoadingRace);
    cpu.eax = cpu.edx * 0xc;
    cpu.edx = at(app, cpu.eax + cpu.eax * 8 + kRaceRecords);
    cpu.ecx = 0x10;
    cpu.eax = cpu.edx;
    cpu.edi = track;
    if (!byAddress(app, cpu, 0x439520))
        return;
    // Its track's name: rep movsd, rep movsb.
    cpu.esi = cpu.eax;
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    for (cpu.ecx >>= 2; cpu.ecx != 0; --cpu.ecx)
        movsd(app, cpu);
    cpu.cl = cpu.al & 3;
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        putByte(app, cpu.ees + cpu.edi, byte(app, cpu.esi));
        const reg32 step = cpu.flags.df ? reg32(0) - 1 : 1;
        cpu.edi += step;
        cpu.esi += step;
    }
    cpu.edi = pop(app, cpu);
    cpu.eax = track;
    cpu.esi = kLoadingPath;
    if (!byAddress(app, cpu, 0x439660))
        return;
    cpu.ebx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edi = name;
    if (!byAddress(app, cpu, 0x43bcb0))
        return;
    cpu.ecx = cpu.eax;
    push(app, cpu, cpu.edi);
    copyStringInline(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.eax = name;
    if (!byAddress(app, cpu, 0x4529a0))
        return;
    // Which of the track's pictures: the next in turn, by its count (ecx).
    cpu.al = byte(app, kLoadingPictures);
    putByte(app, ebp + 0x18e, cpu.al);
    cpu.eax = byte(app, ebp + 0x18e);
    cpu.edx = byte(app, cpu.eax + kPictureChoices);
    cpu.eax = cpu.edx;
    {
        const x86::sreg64 n = x86::sreg64(sreg32(cpu.eax));
        const sreg32 d = sreg32(cpu.ecx);
        cpu.eax = reg32(n / d);
        cpu.edx = reg32(n % d);
    }
    push(app, cpu, cpu.edx);
    cpu.al = byte(app, ebp + 0x18e);
    push(app, cpu, cpu.ebx);
    ++cpu.al;
    push(app, cpu, 0x53be00);
    cpu.edi = name;
    putByte(app, kLoadingPictures, cpu.al);
    stringLength(app, cpu);
    cpu.eax = name + cpu.ecx;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x10;
    cpu.eax = name;
    if (!byAddress(app, cpu, kFileExists))
        return;
    if (cpu.eax == 0 || (byte(app, kLoadingFlags) & 2))
    {
        // The car's picture instead.
        cpu.esi = kLoadingPath;
        cpu.edi = name;
        push(app, cpu, cpu.edi);
        copyStringInline(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.eax = name;
        if (!byAddress(app, cpu, 0x4529a0))
            return;
        push(app, cpu, 0);
        cpu.ecx = at(app, kLoadingCar);
        push(app, cpu, cpu.ecx);
        push(app, cpu, 0x53be10);
        cpu.edi = name;
        stringLength(app, cpu);
        cpu.eax = name + cpu.ecx;
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0x10;
    }
    cpu.edx = 0x10;
    cpu.eax = name;
    if (!byAddress(app, cpu, 0x4956f0))
        return;
    cpu.edx = 0x53be20;
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, 0x4ef1e0))
        return;
    cpu.edx = cpu.eax;
    for (int buffer = 0; buffer < 2; ++buffer)
    {
        if (!direct(app, cpu, takeWindowAndClear))
            return;
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kLoadingScreen) || !direct(app, cpu, flipAndWaitFrames))
            return;
    }
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    if (!byAddress(app, cpu, at(app, kThrashTReset)))
        return;
    racePrecision(app, cpu);
    done();
}

constexpr reg32 kLoadingBar = 0x55e4c4;        // four corners of kCornerBytes
constexpr reg32 kLoadingAll = 0x53be30;        // 100, a double: the most of [esp+4]
constexpr reg32 kLoadingPerCent = 0x53be38;    // a double
constexpr reg32 kLoadingBarEnd = 0x53be40, kLoadingBarHeight = 0x53be44;
constexpr reg32 kRunSystemTasks = 0x4e7630;    // native_snd_stream's runSystemTasks
constexpr reg32 kBarColours = 0x495190;        // eax the bar's corners

/* 0x495230: the loading bar at [esp+4] per cent (101 past 100) drawn after
 * the loading text, in both buffers, while the loading text is up; the
 * system's tasks run first.  The menus' x87 precision while it draws, the
 * race's after.  stdcall's 4 bytes. */
void drawLoadingProgress(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    auto floatAt = [&](reg32 address) { return x86::Float(app->getMemory<float>(address)); };
    auto setFloat = [&](reg32 address, x86::Float value) { app->getMemory<float>(address) = float(value); };
    cpu.eax = 0;
    if (!byAddress(app, cpu, kRunSystemTasks))
        return;
    if (at(app, kLoadingTextShown) != 0)
    {
        push(app, cpu, 0);
        if (!byAddress(app, cpu, at(app, kThrashSync)))
            return;
        push(app, cpu, 2);
        if (!byAddress(app, cpu, at(app, kThrashWindow)))
            return;
        menuPrecision(app, cpu);
        if (!belowAx(cpu, floatAt(ebp + 0x1c), x86::Float(app->getMemory<double>(kLoadingAll))))
            put(app, ebp + 0x1c, 0x42ca0000);  // 101.0f
        const x86::Float perCent = floatAt(ebp + 0x1c);
        const x86::Float scaled = fp::mul(perCent, x86::Float(app->getMemory<double>(kLoadingPerCent)));
        const x86::Float left = floatAt(kLoadingTextX);
        const x86::Float room = fp::sub(floatAt(kLoadingBarEnd), left);
        setFloat(ebp + 0x1c, scaled);
        const x86::Float width = fp::mul(room, floatAt(ebp + 0x1c));
        const x86::Float height = fp::mul(floatAt(ebp + 0x1c), floatAt(kLoadingBarHeight));
        cpu.ebx = 0x43e08000;                    // 449.0f, the bar's top
        cpu.edi = 0x43ea8000;                    // 469.0f, its bottom
        cpu.ecx = kLoadingBar + 3 * kCornerBytes;
        cpu.edx = kLoadingBar + kCornerBytes;
        cpu.esi = kLoadingBar;
        setFloat(kLoadingBar, left);
        setFloat(kLoadingBar + 3 * kCornerBytes, left);
        put(app, kLoadingBar + 4, cpu.ebx);
        put(app, kLoadingBar + kCornerBytes + 4, cpu.ebx);
        put(app, kLoadingBar + 2 * kCornerBytes + 4, cpu.edi);
        put(app, kLoadingBar + 3 * kCornerBytes + 4, cpu.edi);
        cpu.ebx = kLoadingBar + 2 * kCornerBytes;
        cpu.edi = kLoadingBar + 0x14;
        fpu.count += 3;
        fpu.st(0) = height;
        fpu.st(1) = width;
        fpu.st(2) = left;
        if (!byAddress(app, cpu, kChop))
            return;
        put(app, ebp - 4, reg32(fp::toInteger<sreg32>(fpu.st(0))));
        cpu.eax = at(app, ebp - 4);
        const x86::Float right = fp::add(fpu.st(2), fpu.st(1));
        cpu.eax <<= 0x10;
        setFloat(kLoadingBar + kCornerBytes, right);
        cpu.eax -= 0x1000000;
        const x86::Float rightAgain = floatAt(kLoadingBar + kCornerBytes);
        put(app, kLoadingBar + kCornerBytes + kCornerColour, cpu.eax);
        put(app, kLoadingBar + 2 * kCornerBytes + kCornerColour, cpu.eax);
        cpu.eax = kLoadingBar;
        setFloat(kLoadingBar + 2 * kCornerBytes, rightAgain);
        fpu.count -= 3;
        if (!byAddress(app, cpu, kBarColours))
            return;
        for (reg32 corner = 0; corner < 4; ++corner)
            shapeClipCode(app, cpu, kLoadingBar + corner * kCornerBytes);
        cpu.eax = thrash::kStateTexture;
        cpu.edx = 0;
        if (!direct(app, cpu, setRenderState))
            return;
        auto shaded = [&](reg32 value) {
            if (!(byte(app, kDriverFlags) & kDriverSofttria))
                return true;
            cpu.edx = value;
            cpu.eax = thrash::kStateShade;
            return direct(app, cpu, setRenderState);
        };
        for (int buffer = 0; buffer < 2; ++buffer)
        {
            if (buffer == 1)
            {
                push(app, cpu, 0);
                if (!byAddress(app, cpu, at(app, kThrashSync)))
                    return;
            }
            if (!shaded(1))
                return;
            cpu.ecx = kLoadingBar + 3 * kCornerBytes;
            cpu.ebx = kLoadingBar + 2 * kCornerBytes;
            cpu.edx = kLoadingBar + kCornerBytes;
            cpu.eax = kLoadingBar;
            if (!byAddress(app, cpu, kDrawPolygon) || !shaded(0)
                || !byAddress(app, cpu, at(app, kThrashFlushWindow)) || !byAddress(app, cpu, at(app, kThrashPageFlip)))
                return;
        }
        racePrecision(app, cpu);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
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
        push(app, cpu, 0x30);
        for (; cpu.ecx != 0; --cpu.ecx)
            movsd(app, cpu);
        push(app, cpu, 0x53ce54);
        cpu.esi = local(0x38);
        push(app, cpu, cpu.esi);
        push(app, cpu, cpu.ebx);
        if (!byAddress(app, cpu, at(app, cpu.ecs + kMessageBoxImport)))
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
    push(app, cpu, 2);
    if (!byAddress(app, cpu, at(app, cpu.ecs + kSetErrorModeImport)))
        return;
    cpu.ah = 0;
    putByte(app, ebp - 0x44, cpu.ah);
    cpu.eax = 0x190;                             // the probe's read of [0x190] is skipped (SKIP_INSTRUCTIONS)
    putByte(app, ebp - 4, cpu.al);
    cpu.al = byte(app, ebp - 4);
    putByte(app, ebp - 4, reg8(byte(app, ebp - 4) + 1));
    putByte(app, ebp - 0x44, 0xff);
    cpu.edi = 1;
    push(app, cpu, 2);
    cpu.bl = 1;
    put(app, kDebugOutput, cpu.edi);
    if (!byAddress(app, cpu, at(app, cpu.ecs + kSetErrorModeImport)))
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
        cpu.eax = at(app, 0x6fbb4c);
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
constexpr reg32 kCabinFlags = 0x55eb2e;        // bit 0x20: no cabin
constexpr reg32 kCabinGauges = 0x792030;       // twelve words of eight bytes, from the file
constexpr reg32 kCabinNeedle = 0x792040;       // a needle's pivot, its zero and its full scale: x, y each
constexpr reg32 kCabinOtherNeedle = 0x792068;
constexpr reg32 kCabinDigits = 0x55d6a4, kCabinDigitsX = 0x55d69c, kCabinDigitsY = 0x55d6a0;
constexpr reg32 kNeedleShown = 0x7922d0, kOtherNeedleShown = 0x7922e8;
constexpr reg32 kAtan2 = 0x4ea9c0, kSquareRoot = 0x4ea920;  // a float on the stack each, onto the x87's stack
constexpr reg32 kOneDown = 0x53b754;           // -1.0f
constexpr reg32 kHalfOfIt = 0x53b758;          // 0.5f

namespace
{

/* An angle in turns brought into (0, 1]: one added while it is below 0, and
 * [kOneDown] while its bits are above 1.0's. */
void angleIntoTurn(win32::WinApplication* app, x86::CPU& cpu, reg32 at)
{
    while (!belowOrEqualAx(cpu, 0.0, x86::Float(app->getMemory<float>(at))))
        app->getMemory<float>(at) = float(fp::add(1.0, x86::Float(app->getMemory<float>(at))));
}

void angleNotPastTurn(win32::WinApplication* app, x86::CPU& cpu, reg32 at)
{
    while (sreg32(guest::at(app, at)) > 0x3f800000)
        app->getMemory<float>(at) = float(fp::add(x86::Float(app->getMemory<float>(at)),
                                                      x86::Float(app->getMemory<float>(kOneDown))));
}

}  // namespace

/* 0x488a80: the cabin's instruments loaded for the car (its file, 0x53b6dc
 * with the car's name, chunk 0x53b6ec unpacked): its textures (sub_4d4ae0,
 * 1555 on softtria, 4444 otherwise), the gauges' twelve words, the digits'
 * place if it has them; and for each of its two needles that moves, the
 * turn between its zero and its full scale (atan2 of each against the
 * pivot, both into one turn and the full past the zero), the length of the
 * needle (the mean of the two distances) and where it starts.  Only where
 * the driver's texture memory and sizes take the cabin's art, and not in a
 * network race. */
void loadCabinInstruments(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x144;
    const reg32 ebp = cpu.ebp;
    auto& fpu = cpu.fpu;
    auto local = [&](reg32 offset) { return at(app, ebp - offset); };
    auto setLocal = [&](reg32 offset, reg32 value) { put(app, ebp - offset, value); };
    auto floatAt = [&](reg32 address) { return x86::Float(app->getMemory<float>(address)); };
    auto setFloat = [&](reg32 address, x86::Float value) { app->getMemory<float>(address) = float(value); };
    auto done = [&]() {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    // [ebp-4] set and fild'ed into a float pushed for a call.
    auto pushInteger = [&](reg32 value) {
        setLocal(4, value);
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(x86::Float(sreg32(local(4))));
    };
    if (!byAddress(app, cpu, at(app, kThrashAbout)))
        return;
    cpu.edx = 0;
    put(app, kCabinTextures, cpu.edx);
    cpu.dl = byte(app, kCabinFlags);
    cpu.esi = cpu.eax;
    if ((cpu.dl & 0x20) || at(app, kGameMode) == 1)
        return done();
    cpu.dh = byte(app, kDriverFlags);
    // The texture memory the cabin's art wants: more than 4 MB (2 MB on one
    // TMU), or Direct3D's both TMUs together.
    bool enough = (cpu.dh & 6) || (byte(app, cpu.eax + 0x75) & 0x80);
    if (!enough && (cpu.dh & 0x40))
    {
        cpu.ebx = at(app, cpu.eax + 0x70);
        cpu.ecx = at(app, cpu.eax + 0x74) + cpu.ebx;
        enough = sreg32(cpu.ecx) > 0x400000;
    }
    if (!enough)
        enough = ((byte(app, cpu.eax + 0x74) & 1) && sreg32(at(app, cpu.eax + 0x70)) > 0x200000)
                 || sreg32(at(app, cpu.eax + 0x70)) > 0x400000;
    if (!enough || sreg32(at(app, cpu.eax + 0x10)) > 0x100 || sreg32(at(app, cpu.eax + 0x14)) < 0x100
        || sreg32(at(app, cpu.eax + 0x1c)) > 0x100 || sreg32(at(app, cpu.eax + 0x20)) < 0x100)
        return done();
    // The car's cabin file.
    cpu.eax = 0;
    if (!byAddress(app, cpu, 0x422bd0))
        return;
    cpu.eax = at(app, at(app, cpu.eax + 0x220) + 0x14);
    if (!byAddress(app, cpu, 0x4392b0))
        return;
    push(app, cpu, cpu.eax);
    push(app, cpu, 0x7a28b4);
    push(app, cpu, 0x53b6dc);
    cpu.eax = ebp - 0x144;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x10;
    cpu.eax = ebp - 0x144;
    if (!byAddress(app, cpu, kFileExists))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.edx = 0x10;
    cpu.eax = ebp - 0x144;
    if (!byAddress(app, cpu, 0x4e0fe0))
        return;
    cpu.esi = cpu.eax;
    if (cpu.eax == 0)
        return done();
    cpu.eax = ebp - 0x34;
    cpu.ecx = ebp - 0x38;
    cpu.edx = 0x53b6ec;
    push(app, cpu, cpu.eax);
    cpu.ebx = 0;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4ea040))
        return;
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFree))
            return;
        return done();
    }
    cpu.ecx = local(0x38) + cpu.esi;
    cpu.eax = cpu.ecx;
    cpu.ebx = 0x10;
    if (!byAddress(app, cpu, 0x4f2710))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = 0x53b6f8;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.edi = cpu.eax;
    setLocal(0x14, cpu.eax);
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFree))
            return;
        return done();
    }
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x4f2620))
        return;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kFree))
        return;
    // Its textures, and the gauges' words.
    cpu.eax = ebp - 0x40;
    push(app, cpu, cpu.eax);
    cpu.ecx = 0x791d70;
    cpu.ebx = 0x53b6fc;
    push(app, cpu, (byte(app, kDriverFlags) & kDriverSofttria) ? 3 : 7);
    cpu.edx = 0x53b704;
    cpu.eax = cpu.edi;
    if (!byAddress(app, cpu, 0x4d4ae0))
        return;
    put(app, kCabinTextures, cpu.eax);
    cpu.eax = local(0x40);
    if (!byAddress(app, cpu, 0x4f2460))
        return;
    cpu.ecx = cpu.eax;
    for (cpu.eax = 0; sreg32(cpu.eax) < 0xc;)
    {
        cpu.esi = cpu.eax * 8;
        cpu.edx = cpu.ecx + cpu.esi;
        cpu.edi = cpu.esi + kCabinGauges;
        cpu.esi = cpu.edx;
        ++cpu.eax;
        movsd(app, cpu);
        movsd(app, cpu);
    }
    // The digits, for a cabin with more than two pictures.
    cpu.eax = local(0x14);
    if (!byAddress(app, cpu, 0x4ea654))
        return;
    if (sreg32(cpu.eax) <= 2)
    {
        cpu.eax = 0;
        put(app, kCabinDigits, cpu.eax);
    }
    else
    {
        cpu.edi = 1;
        for (reg32 digit = 0; digit < 8; ++digit)
        {
            push(app, cpu, 3);
            cpu.ecx = 0x791c10 + 0x2c * digit;
            cpu.ebx = 0x20;
            cpu.edx = 0x53b70c + 8 * digit;
            cpu.eax = local(0x14);
            if (digit == 0)
                put(app, kCabinDigits, cpu.edi);
            if (!byAddress(app, cpu, 0x4d4080))
                return;
        }
        cpu.edx = 0x53b70c;
        cpu.eax = local(0x14);
        if (!byAddress(app, cpu, 0x4ef1e0))
            return;
        setLocal(0x40, cpu.eax);
        // Its place: two 12-bit signed numbers in the picture's +0xc.
        cpu.edx = reg32(sreg32(at(app, cpu.eax + 0xc) << 20) >> 20);
        cpu.eax = reg32(sreg32(at(app, cpu.eax + 0xc) << 4) >> 20);
        setLocal(4, cpu.edx);
        const x86::Float x = x86::Float(sreg32(local(4)));
        setLocal(4, cpu.eax);
        setFloat(kCabinDigitsX, x);
        setFloat(kCabinDigitsY, x86::Float(sreg32(local(4))));
    }
    cpu.eax = local(0x14);
    if (!byAddress(app, cpu, kFree))
        return;
    // The needles: each one's turn, length and start, when it moves.
    struct Needle
    {
        reg32 points, shown, slope, start, length;
    };
    auto needle = [&](const Needle& n, bool first) {
        // Whether its zero and its full scale differ.
        if (first)
        {
            cpu.eax = at(app, n.points + 0x18);
            if (cpu.eax != at(app, n.points + 0x20))
                cpu.eax = 1;
            else
            {
                cpu.ecx = at(app, n.points + 0x24);
                cpu.eax = at(app, n.points + 0x1c);
                cpu.eax = cpu.eax != cpu.ecx ? 1 : cpu.eax ^ cpu.ecx;
            }
        }
        else
        {
            cpu.esi = at(app, n.points + 0x20);
            if (cpu.esi != at(app, n.points + 0x18))
                cpu.eax = 1;
            else
            {
                cpu.edi = at(app, n.points + 0x24);
                cpu.eax = at(app, n.points + 0x1c);
                cpu.eax = cpu.eax != cpu.edi ? 1 : cpu.eax ^ cpu.edi;
            }
        }
        put(app, n.shown, cpu.eax);
        if (cpu.eax == 0)
            return true;
        // The first needle's scale: the picture's size by a factor of two kinds.
        const reg32 slope = first ? 0x1c : 0x10, other = first ? 0x24 : 0x28;
        const reg32 low = first ? 0x18 : 0x10, high = first ? 8 : 0xc;
        if (first)
        {
            const x86::Float factor = floatAt(at(app, 0x79204c) != 0 ? 0x53b74c : 0x53b750);
            fpu.count += 1;
            fpu.st(0) = factor;
            const x86::Float scaled = fp::mul(x86::Float(sreg32(at(app, 0x792054))), fpu.st(0));
            const x86::Float scaledOther = fp::mul(fpu.st(0), x86::Float(sreg32(at(app, 0x792050))));
            setFloat(ebp - 0x1c, scaled);
            setFloat(ebp - 0x24, scaledOther);
        }
        else
        {
            const x86::Float a = x86::Float(sreg32(at(app, 0x79207c)));
            const x86::Float b = x86::Float(sreg32(at(app, 0x792078)));
            setFloat(ebp - 0x28, b);
            setFloat(ebp - 0x20, a);
        }
        // atan2 of the zero, then of the full scale, against the pivot.
        cpu.eax = at(app, n.points + 0x18);
        (first ? cpu.esi : cpu.edx) = at(app, n.points);
        cpu.eax -= at(app, n.points);
        pushInteger(cpu.eax);
        cpu.eax = at(app, n.points + 0x1c);
        (first ? cpu.edi : cpu.ecx) = at(app, n.points + 4);
        cpu.eax -= at(app, n.points + 4);
        pushInteger(cpu.eax);
        if (first)
            fpu.count -= 1;                      // fstp st(0): the factor
        if (!byAddress(app, cpu, kAtan2))
            return false;
        cpu.eax = at(app, n.points + 0x20);
        (first ? cpu.edx : cpu.ebx) = at(app, n.points);
        cpu.eax -= at(app, n.points);
        pushInteger(cpu.eax);
        cpu.eax = at(app, n.points + 0x24);
        (first ? cpu.ecx : cpu.esi) = at(app, n.points + 4);
        cpu.eax -= at(app, n.points + 4);
        pushInteger(cpu.eax);
        setFloat(ebp - low, fpu.st(0));
        fpu.count -= 1;
        if (!byAddress(app, cpu, kAtan2))
            return false;
        setFloat(ebp - high, fpu.st(0));
        fpu.count -= 1;
        angleIntoTurn(app, cpu, ebp - low);
        angleIntoTurn(app, cpu, ebp - high);
        angleNotPastTurn(app, cpu, ebp - low);
        angleNotPastTurn(app, cpu, ebp - high);
        // The full scale past the zero, the way the picture's scale runs.
        const reg32 from = first ? 0x24 : 0x28, to = first ? 0x1c : 0x20;
        if (belowAx(cpu, floatAt(ebp - from), floatAt(ebp - to)))
        {
            if (belowAx(cpu, floatAt(ebp - high), floatAt(ebp - low)))
                setFloat(ebp - high, fp::add(1.0, floatAt(ebp - high)));
        }
        else if (belowAx(cpu, floatAt(ebp - low), floatAt(ebp - high)))
            setFloat(ebp - low, fp::add(1.0, floatAt(ebp - low)));
        // The slope, the start, and the two distances.
        const x86::Float highTurn = floatAt(ebp - high);
        const x86::Float turn = fp::sub(highTurn, floatAt(ebp - low));
        const x86::Float scaleTo = floatAt(ebp - to);
        const x86::Float scaleSpan = fp::sub(scaleTo, floatAt(ebp - from));
        const x86::Float perUnit = fp::div(turn, scaleSpan);
        cpu.eax = at(app, n.points + 0x18);
        reg32& pivotX = first ? cpu.edi : cpu.ecx;
        reg32& pivotY = first ? cpu.edx : cpu.ebx;
        pivotX = at(app, n.points);
        cpu.eax -= pivotX;
        pivotY = at(app, n.points + 4);
        setLocal(4, cpu.eax);
        cpu.eax = at(app, n.points + 0x1c);
        const x86::Float dx = x86::Float(sreg32(local(4)));
        cpu.eax -= pivotY;
        setLocal(4, cpu.eax);
        const x86::Float dx2 = fp::mul(dx, dx);
        const x86::Float dy = x86::Float(sreg32(local(4)));
        cpu.eax = at(app, n.points + 0x20);
        const x86::Float dy2 = fp::mul(dy, dy);
        cpu.eax -= pivotX;
        setFloat(n.slope, perUnit);
        const x86::Float offset = fp::mul(scaleTo, floatAt(n.slope));
        setLocal(4, cpu.eax);
        cpu.esp -= 4;
        const x86::Float fullX = x86::Float(sreg32(local(4)));
        cpu.eax = at(app, n.points + 0x24);
        setFloat(ebp - 0x2c, fullX);
        cpu.eax -= pivotY;
        const x86::Float zeroSquared = fp::add(dx2, dy2);
        setLocal(4, cpu.eax);
        app->getMemory<float>(cpu.esp) = float(zeroSquared);
        const x86::Float fullY = x86::Float(sreg32(local(4)));
        setFloat(n.start, fp::sub(highTurn, offset));
        setFloat(ebp - 0x3c, fullY);
        if (!byAddress(app, cpu, kSquareRoot))
            return false;
        const x86::Float fullSquared =
            fp::add(fp::mul(floatAt(ebp - 0x2c), floatAt(ebp - 0x2c)), fp::mul(floatAt(ebp - 0x3c), floatAt(ebp - 0x3c)));
        cpu.esp -= 4;
        app->getMemory<float>(cpu.esp) = float(fullSquared);
        setFloat(ebp - 4, fpu.st(0));
        fpu.count -= 1;
        if (!byAddress(app, cpu, kSquareRoot))
            return false;
        setFloat(n.length, fp::mul(fp::add(fpu.st(0), floatAt(ebp - 4)), floatAt(kHalfOfIt)));
        fpu.count -= 1;
        return true;
    };
    if (!needle({kCabinNeedle, kNeedleShown, 0x7922e0, 0x7922f4, 0x7922dc}, true)
        || !needle({kCabinOtherNeedle, kOtherNeedleShown, 0x7922ec, 0x7922f0, 0x7922f8}, false))
        return;
    if (at(app, kNeedleShown) == 0 && at(app, kOtherNeedleShown) == 0)
        return done();
    // The gauges' first two words, as 8.8: their texture and their place.
    auto towardsZero256 = [](reg32 value) { return reg32(sreg32(value + (sreg32(value) < 0 ? 0xff : 0)) >> 8); };
    cpu.eax = towardsZero256(at(app, kCabinGauges));
    cpu.edx = sreg32(at(app, kCabinGauges)) < 0 ? 0xffffff00 : 0;
    cpu.ecx = cpu.eax * 2;
    cpu.eax = towardsZero256(at(app, kCabinGauges + 4));
    cpu.edx = sreg32(at(app, kCabinGauges + 4)) < 0 ? 0xffffff00 : 0;
    cpu.ecx += cpu.eax;
    cpu.eax = cpu.ecx * 0xb;
    cpu.eax = at(app, 0x791d74 + cpu.eax * 4);
    put(app, 0x7922e4, cpu.eax);
    auto divide256 = [&](reg32 value) {
        const x86::sreg64 n = x86::sreg64(sreg32(value));
        cpu.eax = reg32(n / 0x100);
        cpu.edx = reg32(n % 0x100);
    };
    cpu.ecx = 0x100;
    divide256(at(app, kCabinGauges));
    setLocal(4, cpu.edx);
    divide256(at(app, kCabinGauges + 4));
    const x86::Float unit = floatAt(0x53b75c);
    const x86::Float u = fp::mul(x86::Float(sreg32(local(4))), unit);
    setLocal(4, cpu.edx);
    const x86::Float v = fp::mul(unit, x86::Float(sreg32(local(4))));
    const x86::Float bias = x86::Float(app->getMemory<double>(0x53b764));
    setFloat(0x7922d8, fp::add(u, bias));
    setFloat(0x7922d4, fp::add(bias, v));
    done();
}

/* ---- A message in the HUD ---- */

constexpr reg32 kHudBoxes = 0x724258;          // a player's: 0x280 bytes of quads, 0x20 a corner
constexpr reg32 kHudBoxTexture = 0x749a38, kHudFlashTexture = 0x8b4844;
constexpr reg32 kHudTextScale = 0x791b50;
constexpr reg32 kHalf = 0x53b5e8, kHalfAgain = 0x53b5ec;  // 0.5f each

/* 0x482940: player eax's message box in the HUD, text edx: its three quads
 * (textured with [0x749a38]), the text centred in it in white at the HUD's
 * scale, then the flashing bar over it (one of two quads by the clock's
 * half second, blended, [0x8b4844]). */
void hudMessageBox(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    auto& fpu = cpu.fpu;
    auto loadFloat = [&](reg32 address) { return x86::Float(app->getMemory<float>(address)); };
    auto quad = [&](reg32 first) {
        for (reg32 corner = 3; corner < 4; --corner)
        {
            cpu.eax = first + corner * kCornerBytes;
            push(app, cpu, cpu.eax);
        }
        return byAddress(app, cpu, at(app, kThrashDrawQuad));
    };
    cpu.edi = cpu.edx;
    cpu.esi = kHudBoxes + cpu.eax * 0x280;
    cpu.edx = at(app, kHudBoxTexture);
    cpu.eax = thrash::kStateTexture;
    if (!direct(app, cpu, setRenderState) || !quad(cpu.esi) || !quad(cpu.esi + 0x80) || !quad(cpu.esi + 0x100))
        return;
    push(app, cpu, at(app, kHudTextScale));
    if (!byAddress(app, cpu, kSetTextScale))
        return;
    // fild the mode's width, fstp st(0) the scale, fstp it as a float.
    cpu.edx = 0xffffffff;
    app->getMemory<float>(cpu.ebp - 8) = float(x86::Float(sreg32(at(app, kModeWidth))));
    cpu.eax = cpu.edi;
    put(app, kTextColour, cpu.edx);
    fpu.count -= 1;
    if (!byAddress(app, cpu, kTextWidth))
        return;
    // The text's left: the box's middle less half its width (chopped).
    const x86::Float left = fp::mul(fp::sub(loadFloat(cpu.ebp - 8), fpu.st(0)), loadFloat(kHalf));
    const x86::Float boxHeight = fp::sub(loadFloat(cpu.esi + 0x44), loadFloat(cpu.esi + 4));
    cpu.eax = 0x44;
    app->getMemory<float>(cpu.ebp - 8) = float(boxHeight);
    fpu.st(0) = left;
    if (!byAddress(app, cpu, kChop))
        return;
    put(app, cpu.ebp - 0xc, reg32(fp::toInteger<sreg32>(fpu.st(0))));
    fpu.count -= 1;
    if (!byAddress(app, cpu, kTextHeight))
        return;
    // Its top: the box's top and half of what its height leaves.
    put(app, cpu.ebp - 4, cpu.eax);
    const x86::Float spare = fp::sub(loadFloat(cpu.ebp - 8), x86::Float(sreg32(at(app, cpu.ebp - 4))));
    const x86::Float halfSpare = fp::mul(spare, loadFloat(kHalfAgain));
    cpu.edx = at(app, cpu.ebp - 0xc);
    const x86::Float top = fp::add(halfSpare, loadFloat(cpu.esi + 4));
    cpu.eax = cpu.edi;
    fpu.count += 1;
    fpu.st(0) = top;
    if (!byAddress(app, cpu, kChop))
        return;
    put(app, cpu.ebp - 0x10, reg32(fp::toInteger<sreg32>(fpu.st(0))));
    cpu.ebx = at(app, cpu.ebp - 0x10);
    fpu.count -= 1;
    if (!byAddress(app, cpu, kDrawText))
        return;
    fpu.count -= 1;                              // fstp st(0): the text's width
    if (!byAddress(app, cpu, kFlushText))
        return;
    cpu.edx = 1;
    cpu.eax = thrash::kStateBlend;
    if (!direct(app, cpu, setRenderState))
        return;
    cpu.eax = thrash::kStateTexture;
    cpu.edx = at(app, kHudFlashTexture);
    if (!direct(app, cpu, setRenderState))
        return;
    cpu.eax = at(app, kTicks) & 0x3f;
    const reg32 bar = sreg32(cpu.eax) < 0x20 ? 0x180 : 0x200;
    if (byte(app, kDriverFlags) & kDriverSofttria)
    {
        cpu.eax = at(app, cpu.esi + bar + 0x10);
        if (!direct(app, cpu, setSoftwareBlendColour))
            return;
    }
    if (!quad(cpu.esi + bar))
        return;
    cpu.eax = thrash::kStateBlend;
    cpu.edx = 0;
    if (!direct(app, cpu, setRenderState))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- A race's frame ---- */

constexpr reg32 kRaceView = 0x7a3d14;          // the view drawn
constexpr reg32 kTimes = 0x7a3cf0;             // what each stage took, the last one's into here
constexpr reg32 kKeepTime = 0x4bc030;          // eax the view, edx kTimes
constexpr reg32 kStartTime = 0x4bc000;         // eax 1, esi where the time goes
constexpr reg32 kSoftwareRaceView = 0x7a3a60;  // 0: the software renderers' order
constexpr reg32 kWideScreenFlag = 0x6fbc44, kWideScreenMode = 0x6fd4c8;
constexpr reg32 kRaceHudOff = 0x7d36a0, kRaceViewPaused = 0x7a3d10, kRaceCountdown = 0x7a3d18;
constexpr reg32 kRaceFrameSteps[] = {0x4dece0, 0x4dedf0, 0x4cc600};

/* 0x4bc160: a frame of the race's view, eax which pass (0 the main one, 1
 * the mirror, others neither): the frame's lists set up, the window cleared
 * on the software renderers, the sky (sub_4782e0), the view (sub_41b9b0),
 * the effects, the cars, the smoke and the rest, each stage timed (sub_4bc030
 * with what sub_4bc000 or the stage gave back, through esi as Watcom returns
 * a record); then the HUD for the main pass (sub_482b00) or the mirror's
 * frame (sub_484030), and the countdown. */
void raceFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x50;
    const reg32 ebp = cpu.ebp;
    auto call = [&](reg32 target) { return byAddress(app, cpu, target); };
    auto callWith = [&](reg32 eax, reg32 target) {
        cpu.eax = eax;
        return byAddress(app, cpu, target);
    };
    // The time a stage gave back at esi copied to kTimes and kept.
    auto keep = [&](reg32 from) {
        cpu.esi = from;
        cpu.edx = kTimes;
        cpu.eax = kRaceView;
        movsd(app, cpu);
        movsd(app, cpu);
        return call(kKeepTime);
    };
    // A stage timed by sub_4bc000 after it.
    auto timed = [&](reg32 from) {
        cpu.eax = 1;
        cpu.edi = kTimes;
        return call(kStartTime) && keep(from);
    };
    // The pass's layers (sub_431c40, sub_431c30) for the main pass.
    auto layers = [&](reg32 first, reg32 second) {
        if (cpu.ebx != 0)
            return true;
        return callWith(first, 0x431c40) && callWith(second, 0x431c30);
    };
    cpu.ebx = cpu.eax;
    if (cpu.eax == 0 && !call(0x431c30))
        return;
    if (!call(0x4318c0) || !callWith(1, 0x4c7750) || !call(0x4c75f0) || !call(0x4c73e0))
        return;
    if (cpu.ebx == 0 && (byte(app, kDriverFlags) & kDriverSoftware)
        && !byAddress(app, cpu, at(app, kThrashClearWindow)))
        return;
    if (byte(app, kDriverFlags) & kDriverSofttria)
    {
        cpu.edx = 1;
        cpu.eax = thrash::kStateShade;
        cpu.esi = ebp - 0x50;
        if (!direct(app, cpu, setRenderState))
            return;
        cpu.eax = kRaceView;
        cpu.edi = kTimes;
        if (!call(0x4782e0) || !keep(ebp - 0x50))
            return;
    }
    else
    {
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x48;
        cpu.edi = kTimes;
        if (!call(0x4782e0) || !keep(ebp - 0x48))
            return;
    }
    if (!layers(0, 1))
        return;
    if (byte(app, kDriverFlags) & kDriverSofttria)
    {
        cpu.edx = at(app, kWideScreenFlag);
        if (cpu.edx != 0 || at(app, kWideScreenMode) == 1)
            cpu.edx = 1;
        cpu.eax = thrash::kStateShade;
        if (!direct(app, cpu, setRenderState))
            return;
    }
    reg32 last;
    if (at(app, kSoftwareRaceView) == 0)
    {
        if (!callWith(kRaceView, 0x41b9b0) || !layers(1, 2) || !callWith(kRaceView, 0x4c6f00))
            return;
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x40;
        if (!call(0x4bbba0))
            return;
        cpu.eax = kRaceView;
        cpu.edi = kTimes;
        if (!call(0x4cc5c0) || !call(0x4c7580))
            return;
        last = ebp - 0x40;
    }
    else
    {
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x38;
        if (!call(0x41b9b0) || !timed(ebp - 0x38) || !layers(1, 2))
            return;
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x30;
        if (!call(0x4c6f00) || !timed(ebp - 0x30))
            return;
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x28;
        if (!call(0x4bbba0) || !timed(ebp - 0x28))
            return;
        cpu.eax = kRaceView;
        cpu.esi = ebp - 0x20;
        cpu.edi = kTimes;
        if (!call(0x4cc5c0) || !call(0x4c76e0))
            return;
        last = ebp - 0x20;
    }
    if (!keep(last))
        return;
    for (reg32 step = 0; step < 3; ++step)
    {
        const reg32 from = ebp - 0x18 + 8 * step;
        cpu.eax = kRaceView;
        cpu.esi = from;
        cpu.edi = kTimes;
        if (!call(kRaceFrameSteps[step]) || !keep(from))
            return;
    }
    bool hud = true;
    if (cpu.ebx == 0)
    {
        if (!callWith(kModeRect, kSetViewScale) || !call(kTwoDStates) || !callWith(kRaceView, 0x482b00))
            return;
    }
    else if (cpu.ebx == 1)
    {
        if (!callWith(kRaceView, 0x4bc100) || !call(kTwoDStates) || !call(0x484030))
            return;
    }
    else
        hud = false;
    if (hud && (!call(0x431b40) || !callWith(kRaceView, 0x4bc130)))
        return;
    if (cpu.ebx == 0)
    {
        if (at(app, kRaceHudOff) == 0 && at(app, kRaceViewPaused) == 0
            && !callWith(at(app, kRaceCountdown), 0x482620))
            return;
        if (!callWith(2, 0x431c40))
            return;
    }
    if (byte(app, kDriverFlags) & kDriverSofttria)
    {
        cpu.eax = thrash::kStateShade;
        cpu.edx = 0;
        if (!direct(app, cpu, setRenderState))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* ---- The reciprocals ---- */

constexpr reg32 kReciprocalsMade = 0x564ed0;
constexpr reg32 kReciprocals = 0x9ef990;       // 1024 words: 2^32 / (n * 4) + 1, the first 0
constexpr reg32 kDivide64 = 0x503760;          // native_snd_strm's div64: (low, high, divisor), stdcall

/* 0x4eb3c0: the table of reciprocals made, once: for n from 1 to 1023, the
 * 64-bit 1:0 divided by 4n, plus 1. */
void makeReciprocals(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 8;
    cpu.edx = at(app, kReciprocalsMade);
    if (cpu.edx == 0)
    {
        push(app, cpu, cpu.ebx);
        cpu.ebx = 1;
        put(app, kReciprocals, cpu.edx);
        put(app, cpu.esp + 4, cpu.edx);
        cpu.edx = 4;
        put(app, cpu.esp + 8, cpu.ebx);
        cpu.ebp = at(app, cpu.esp + 4);
        cpu.edi = at(app, cpu.esp + 8);
        cpu.ebx = pop(app, cpu);
        do
        {
            push(app, cpu, cpu.edx);
            push(app, cpu, cpu.edi);
            push(app, cpu, cpu.ebp);
            cpu.edx += 4;
            if (!byAddress(app, cpu, kDivide64))
                return;
            put(app, cpu.edx + kReciprocals - 4, cpu.eax + 1);
            ++cpu.eax;
        } while (cpu.edx != 0x1000);
        put(app, kReciprocalsMade, at(app, kReciprocalsMade) + 1);
    }
    cpu.esp += 8;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

void copyFloat(win32::WinApplication* app, reg32 to, reg32 from)
{
    app->getMemory<float>(to) = float(x86::Float(app->getMemory<float>(from)));
}


}  // namespace graphics
}  // namespace nfs3hp
