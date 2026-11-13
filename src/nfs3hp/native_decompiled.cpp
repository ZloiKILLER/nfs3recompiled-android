#include "game_settings.h"
#include "native_guest.h"

#include <cstdint>

/* nfs3.exe's functions decompiled one at a time from their x86 code, each
 * where its generated code was: disassemble_nfs3hp.py binds the addresses of
 * decompiled.json to the functions of those names here (DECOMPILED_NATIVES),
 * and the generated code calls them by name.
 *
 * tools/game_checks holds each against the game's code: on random registers,
 * a random stack and made-up values in every global it uses (decompiled.json
 * lists them), every register, every byte it touches and the stack come out
 * as the game's code leaves them.  These are the functions that use nothing
 * but registers, the stack and globals at fixed addresses, which that is
 * enough for.
 *
 * Written as their code is, as the other natives: each push and pop to the
 * guest's stack, the registers as the code leaves them, results in eax. */
namespace nfs3hp
{
namespace decompiled
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

}  // namespace

/* The menus' help line (native_frontend_loop.cpp's) and the pointer where the
 * menus have it, on a 640 by 480 screen (native_frontend_screen.cpp's). */
constexpr reg32 kHelpNeedsMove = 0x559238;    // not before the pointer moves on a new screen
constexpr reg32 kHelpLastX = 0x55923c;        // where the pointer was
constexpr reg32 kHelpLastY = 0x559240;
constexpr reg32 kHelpItem = 0x559244;         // the item it is shown for, -1 for none
constexpr reg32 kHelpText = 0x55924c;
constexpr reg32 kMouseX = 0x55e5e4, kMouseY = 0x55e5e8;

/* 0x449a60: the help line cleared for a new screen: no item, no text, and
 * none until the pointer moves from where it is now. */
void resetMenuHelp(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = 0xffffffff;
    cpu.edx = 1;
    cpu.eax = at(app, kMouseX);
    cpu.ebx = 0;
    put(app, kHelpItem, cpu.ecx);
    put(app, kHelpText, cpu.ebx);
    put(app, kHelpLastX, cpu.eax);
    cpu.eax = at(app, kMouseY);
    put(app, kHelpNeedsMove, cpu.edx);
    put(app, kHelpLastY, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* The settings' effects and engine volumes, and a race's own, which its pause
 * screen moves (native_frontend_options.cpp's names). */
constexpr reg32 kRaceEffectsVolume = 0x552920, kRaceEngineVolume = 0x552924;
constexpr reg32 kRaceVolumesToSettings = 0x552909;  // byte: the race's volumes go back to the settings next

/* 0x418840: the effects and engine volumes handed between the settings and
 * the race: with kRaceVolumesToSettings set, the settings' are kept as the
 * race's and silenced (edx 0); with it clear, the race's go back into the
 * settings (dl 1).  The byte is left the other way round. */
void swapRaceVolumes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.dl = byte(app, kRaceVolumesToSettings);
    if (cpu.dl != 0)
    {
        cpu.eax = gameSettings(app).effectsVolume;
        cpu.edx = 0;
        put(app, kRaceEffectsVolume, cpu.eax);
        cpu.eax = gameSettings(app).engineVolume;
        gameSettings(app).effectsVolume = cpu.edx;
        put(app, kRaceEngineVolume, cpu.eax);
        gameSettings(app).engineVolume = cpu.edx;
        cpu.dl = 0;
    }
    else
    {
        cpu.eax = at(app, kRaceEffectsVolume);
        gameSettings(app).effectsVolume = cpu.eax;
        cpu.eax = at(app, kRaceEngineVolume);
        cpu.dl = 1;
        gameSettings(app).engineVolume = cpu.eax;
    }
    putByte(app, kRaceVolumesToSettings, cpu.dl);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* Whether the driver draws on a 3D card: 0 for the software renderers, 1 for
 * the others, as the driver starts (sub_4b59c0, startDriverNative).  With it
 * clear the polygons are sorted by sub_4c7480, not into depth buckets. */
constexpr reg32 kDriverOnCard = 0x7a3a60;

/* The depth offsets, 10 floats from 0x7dcfdc, each handed to the code that
 * draws one kind of thing, or added to or taken from a depth by it:
 *   +0x00  the mean depth a polygon is bucketed by (depthBucketsNative, sub_4c7400, sub_4c7480)
 *   +0x04  taken from the lights' and the shadow's depths (sub_4b81a0, sub_4b8740, sub_4b9bc0)
 *   +0x08, +0x0c, +0x14, +0x18  each to sub_4c7790 in a view's pass (viewPassNative, sub_41c1a0)
 *   +0x10  sub_4c6f00
 *   +0x1c  to sub_4c7940, twice, in a view's pass
 *   +0x20  the lights' glows and the headlights' beams (sub_491190, sub_492370, sub_492980)
 *   +0x24  sub_494170, sub_4cc5c0 */
constexpr reg32 kDepthOffsets = 0x7dcfdc;
constexpr reg32 kFloat4 = 0x40800000, kFloat5Half = 0x40b00000, kFloat6 = 0x40c00000;
constexpr reg32 kFloat9Half = 0x41180000, kFloat10 = 0x41200000;

/* 0x4c7a60: the depth offsets for the driver (kDriverOnCard): on a card most
 * are 0, the far ones 10 and 6; in software, 9.5 nearly throughout. */
void setDepthOffsets(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kDriverOnCard);
    if (cpu.edx == 0)
    {
        cpu.ebx = kFloat5Half;
        cpu.esi = kFloat9Half;
        cpu.ecx = kFloat4;
        put(app, kDepthOffsets + 0x08, cpu.edx);
        put(app, kDepthOffsets + 0x0c, cpu.edx);
        put(app, kDepthOffsets + 0x10, cpu.ebx);
        put(app, kDepthOffsets + 0x04, cpu.esi);
        put(app, kDepthOffsets + 0x24, cpu.esi);
        put(app, kDepthOffsets + 0x20, cpu.esi);
        put(app, kDepthOffsets + 0x18, cpu.esi);
        put(app, kDepthOffsets + 0x14, cpu.ebx);
        put(app, kDepthOffsets + 0x00, cpu.esi);
        put(app, kDepthOffsets + 0x1c, cpu.ecx);
    }
    else
    {
        cpu.edx = kFloat6;
        cpu.eax = kFloat4;
        cpu.ecx = 0;
        put(app, kDepthOffsets + 0x10, cpu.edx);
        put(app, kDepthOffsets + 0x04, cpu.ecx);
        put(app, kDepthOffsets + 0x24, cpu.ecx);
        put(app, kDepthOffsets + 0x20, cpu.ecx);
        put(app, kDepthOffsets + 0x0c, cpu.ecx);
        put(app, kDepthOffsets + 0x08, cpu.ecx);
        put(app, kDepthOffsets + 0x18, cpu.edx);
        cpu.ecx = kFloat10;
        put(app, kDepthOffsets + 0x1c, cpu.eax);
        put(app, kDepthOffsets + 0x14, cpu.ecx);
        put(app, kDepthOffsets + 0x00, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* How the sun lights a car's vertices (sub_49da30, native_vertices.cpp's
 * shadeNormalsNative): the normals dotted with the sun's direction, and each of
 * red, green and blue the base colour plus the dot times the slope, capped.
 * The sun's colour as it was given (sub_49d8f0) makes the cap (twice it, 255
 * at most) and the slope (the cap less the base). */
constexpr reg32 kSunShadingMode = 0x55e69c;     // sub_49d750's; sub_49de20 does otherwise when it is 1
constexpr reg32 kSunDirection = 0x55e6a0;       // three floats, x y z (sub_49d760)
constexpr reg32 kSunRed = 0x55e6ac, kSunGreen = 0x55e6b0, kSunBlue = 0x55e6b4;  // the colour given
constexpr reg32 kSunBaseRed = 0x55e6b8, kSunBaseGreen = 0x55e6bc, kSunBaseBlue = 0x55e6c0;
constexpr reg32 kSunCapRed = 0x55e6c4, kSunCapGreen = 0x55e6c8, kSunCapBlue = 0x55e6cc;
constexpr reg32 kSunSlopeRed = 0x55e6d0, kSunSlopeGreen = 0x55e6d4, kSunSlopeBlue = 0x55e6d8;
// The two colours sub_49d800 sets (darkened by the glare): a polygon's
// without its flag 8, and with it (sub_49de20, into the vertex's +0x74).
constexpr reg32 kSunPlainColour = 0x55e6dc, kSunFlaggedColour = 0x55e6e0;
constexpr reg32 kSunAlpha = 0x55e6e4;           // alpha in the top byte (sub_49d720's eax)
constexpr reg32 kSunAlphaMask = 0x55e6e8;       // sub_49d720's edx in the top byte, over 0xffffff
constexpr reg32 kSunMatrix = 0x55e6ec;          // three rows of three floats
constexpr reg32 kFloatOne = 0x3f800000, kFloatMinusOne = 0xbf800000;

/* 0x49d640: the sun's lighting back to the game's defaults: straight down,
 * a mid grey sun (127) on no base colour, so caps and slopes of 255; the
 * plain colour grey, the flagged one white, alpha opaque; its matrix the
 * identity. */
void resetSunLighting(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0x7f;
    cpu.ecx = 0xffffffff;
    cpu.ebx = 0xff000000;
    cpu.edi = kFloatOne;
    cpu.esi = 0;
    cpu.eax = 0;
    put(app, kSunGreen, cpu.edx);
    put(app, kSunBlue, cpu.edx);
    put(app, kSunRed, cpu.edx);
    put(app, kSunBaseRed, cpu.esi);
    put(app, kSunBaseGreen, cpu.esi);
    put(app, kSunBaseBlue, cpu.esi);
    put(app, kSunFlaggedColour, cpu.ecx);
    put(app, kSunAlpha, cpu.ebx);
    // The matrix: the identity, row by row.
    put(app, kSunMatrix, cpu.edi);
    put(app, kSunMatrix + 0x04, cpu.eax);
    put(app, kSunMatrix + 0x08, cpu.eax);
    put(app, kSunMatrix + 0x0c, cpu.eax);
    put(app, kSunMatrix + 0x10, cpu.edi);
    put(app, kSunMatrix + 0x14, cpu.eax);
    put(app, kSunMatrix + 0x18, cpu.eax);
    put(app, kSunMatrix + 0x1c, cpu.eax);
    put(app, kSunMatrix + 0x20, cpu.edi);
    put(app, kSunShadingMode, cpu.eax);
    put(app, kSunDirection, cpu.eax);
    put(app, kSunDirection + 8, cpu.eax);
    cpu.edx = 0xff;
    cpu.esi = 0x7fffffff;
    put(app, kSunCapRed, cpu.edx);
    put(app, kSunCapGreen, cpu.edx);
    put(app, kSunCapBlue, cpu.edx);
    put(app, kSunSlopeRed, cpu.edx);
    put(app, kSunSlopeGreen, cpu.edx);
    put(app, kSunSlopeBlue, cpu.edx);
    put(app, kSunAlphaMask, cpu.esi);
    cpu.edx = 0x7f7f7f7f;
    cpu.esi = kFloatMinusOne;
    put(app, kSunPlainColour, cpu.edx);
    put(app, kSunDirection + 4, cpu.esi);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* The race's handling tunings, 0x5efda8 to 0x5efde8: dwords, floats but two,
 * each read by the car's physics where named; what each stands for is known
 * only for the last two.
 *   +0x00 1.0      sub_42b7b0, with the car's +0x4f8
 *   +0x04 -0.04    sub_42b7b0
 *   +0x08 0.15     sub_42b7b0, with the car's +0x4f8
 *   +0x0c -0.0035  sub_42b7b0, with the car's +0x4e8
 *   +0x10 1.2      sub_42bcb0
 *   +0x14 3.0      sub_42bcb0
 *   +0x18 700      sub_429260, into the car's +0x8a0 (an integer)
 *   +0x1c 175      sub_429260, into the car's +0x8a0 (an integer)
 *   +0x20 0        sub_428970, as it clears the car's +0x924 (its cornering load)
 *   +0x24 5        sub_428f90, sub_429100: a multiplier (imul)
 *   +0x28 1.5      sub_40bd80, sub_428f90, sub_429100
 *   +0x2c 0.98     read by no address of its own
 *   +0x30 0.87     sub_4a1610, sub_4a2830: compared with
 *   +0x34 0.5      sub_4a1610, sub_4a1ce0: compared with
 *   +0x38 1/240    kSteerScale (native_controls_race.cpp): the steering's
 *   +0x3c 1/160    kSpringScale: the body roll spring's, against the cornering load
 *   +0x40 10.0     read by no address of its own */
constexpr reg32 kHandlingTunings = 0x5efda8;

/* 0x42c590: the race's handling tunings set to the game's own (above). */
void resetHandlingTunings(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0x3f99999a;
    cpu.ecx = 0x40400000;
    cpu.ebx = 0x3f800000;
    cpu.esi = 0xbd23d70a;
    cpu.edi = 0x3e19999a;
    cpu.eax = 0xbb67ff58;
    put(app, kHandlingTunings + 0x10, cpu.edx);
    put(app, kHandlingTunings + 0x14, cpu.ecx);
    put(app, kHandlingTunings + 0x00, cpu.ebx);
    put(app, kHandlingTunings + 0x04, cpu.esi);
    put(app, kHandlingTunings + 0x08, cpu.edi);
    put(app, kHandlingTunings + 0x0c, cpu.eax);
    cpu.edx = 0x2bc;
    cpu.ecx = 0xaf;
    cpu.esi = 5;
    cpu.edi = 0x3fc00000;
    cpu.eax = 0x3f7ae148;
    cpu.ebx = 0;
    put(app, kHandlingTunings + 0x18, cpu.edx);
    put(app, kHandlingTunings + 0x1c, cpu.ecx);
    put(app, kHandlingTunings + 0x20, cpu.ebx);
    put(app, kHandlingTunings + 0x24, cpu.esi);
    put(app, kHandlingTunings + 0x28, cpu.edi);
    put(app, kHandlingTunings + 0x2c, cpu.eax);
    cpu.edx = 0x3f5eb852;
    cpu.ecx = 0x3f000000;
    cpu.ebx = 0x41200000;
    cpu.esi = 0x3b888889;
    cpu.edi = 0x3bcccccd;
    put(app, kHandlingTunings + 0x30, cpu.edx);
    put(app, kHandlingTunings + 0x34, cpu.ecx);
    put(app, kHandlingTunings + 0x40, cpu.ebx);
    put(app, kHandlingTunings + 0x38, cpu.esi);
    put(app, kHandlingTunings + 0x3c, cpu.edi);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace decompiled
}  // namespace nfs3hp
