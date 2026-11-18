#include "game_settings.h"
#include "native_guest.h"

#include <cstdint>

/* nfs3.exe's functions decompiled one at a time from their x86 code: the
 * game's code calls them by name.
 *
 * These are the functions that use nothing
 * but registers, the stack and globals at fixed addresses (decompiled.json
 * lists the globals).
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
x86::reg32 resetMenuHelp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebx);
    push(app, cpu, args.ecx);
    push(app, cpu, args.edx);
    push(app, cpu, args.ebp);
    args.ebp = cpu.esp;
    args.ecx = 0xffffffff;
    args.edx = 1;
    args.eax = at(app, kMouseX);
    args.ebx = 0;
    put(app, kHelpItem, args.ecx);
    put(app, kHelpText, args.ebx);
    put(app, kHelpLastX, args.eax);
    args.eax = at(app, kMouseY);
    put(app, kHelpNeedsMove, args.edx);
    put(app, kHelpLastY, args.eax);
    args.ebp = pop(app, cpu);
    args.edx = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* The settings' effects and engine volumes, and a race's own, which its pause
 * screen moves (native_frontend_options.cpp's names). */
constexpr reg32 kRaceEffectsVolume = 0x552920, kRaceEngineVolume = 0x552924;
constexpr reg32 kRaceVolumesToSettings = 0x552909;  // byte: the race's volumes go back to the settings next

/* 0x418840: the effects and engine volumes handed between the settings and
 * the race: with kRaceVolumesToSettings set, the settings' are kept as the
 * race's and silenced (edx 0); with it clear, the race's go back into the
 * settings (dl 1).  The byte is left the other way round. */
x86::reg32 swapRaceVolumes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.edx);
    push(app, cpu, args.ebp);
    args.ebp = cpu.esp;
    std::uint8_t dl;  // edx's low byte, while it runs: edx is pushed and popped round it
    dl = byte(app, kRaceVolumesToSettings);
    if (dl != 0)
    {
        args.eax = gameSettings(app).effectsVolume;
        args.edx = 0;
        put(app, kRaceEffectsVolume, args.eax);
        args.eax = gameSettings(app).engineVolume;
        gameSettings(app).effectsVolume = args.edx;
        put(app, kRaceEngineVolume, args.eax);
        gameSettings(app).engineVolume = args.edx;
        dl = 0;
    }
    else
    {
        args.eax = at(app, kRaceEffectsVolume);
        gameSettings(app).effectsVolume = args.eax;
        args.eax = at(app, kRaceEngineVolume);
        dl = 1;
        gameSettings(app).engineVolume = args.eax;
    }
    putByte(app, kRaceVolumesToSettings, dl);
    args.ebp = pop(app, cpu);
    args.edx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
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
void depthOffsetsSet(win32::WinApplication* app)
{
    if (at(app, kDriverOnCard) == 0)
    {
        put(app, kDepthOffsets + 0x08, 0);
        put(app, kDepthOffsets + 0x0c, 0);
        put(app, kDepthOffsets + 0x10, kFloat5Half);
        put(app, kDepthOffsets + 0x04, kFloat9Half);
        put(app, kDepthOffsets + 0x24, kFloat9Half);
        put(app, kDepthOffsets + 0x20, kFloat9Half);
        put(app, kDepthOffsets + 0x18, kFloat9Half);
        put(app, kDepthOffsets + 0x14, kFloat5Half);
        put(app, kDepthOffsets + 0x00, kFloat9Half);
        put(app, kDepthOffsets + 0x1c, kFloat4);
    }
    else
    {
        put(app, kDepthOffsets + 0x10, kFloat6);
        put(app, kDepthOffsets + 0x04, 0);
        put(app, kDepthOffsets + 0x24, 0);
        put(app, kDepthOffsets + 0x20, 0);
        put(app, kDepthOffsets + 0x0c, 0);
        put(app, kDepthOffsets + 0x08, 0);
        put(app, kDepthOffsets + 0x18, kFloat6);
        put(app, kDepthOffsets + 0x1c, kFloat4);
        put(app, kDepthOffsets + 0x14, kFloat10);
        put(app, kDepthOffsets + 0x00, kFloat10);
    }
}

/* 0x4c7a60: depthOffsetsSet; every register but eax kept, which a software driver leaves 4.0's bits (the
 * code pushed its registers below esp and popped them again: nothing written there now). */
x86::reg32 setDepthOffsets(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const bool cardBranch = at(app, kDriverOnCard) == 0;
    depthOffsetsSet(app);
    if (!cardBranch)
        args.eax = kFloat4;
    cpu.esp += 4;
    return args.eax;
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

/* 0x49d640's work: the sun's lighting back to the game's defaults: straight down,
 * a mid grey sun (127) on no base colour, so caps and slopes of 255; the
 * plain colour grey, the flagged one white, alpha opaque; its matrix the
 * identity.  The writes in the order the code makes them. */
void sunLightingReset(win32::WinApplication* app)
{
    put(app, kSunGreen, 0x7f);
    put(app, kSunBlue, 0x7f);
    put(app, kSunRed, 0x7f);
    put(app, kSunBaseRed, 0);
    put(app, kSunBaseGreen, 0);
    put(app, kSunBaseBlue, 0);
    put(app, kSunFlaggedColour, 0xffffffff);
    put(app, kSunAlpha, 0xff000000);
    // The matrix: the identity, row by row.
    put(app, kSunMatrix, kFloatOne);
    put(app, kSunMatrix + 0x04, 0);
    put(app, kSunMatrix + 0x08, 0);
    put(app, kSunMatrix + 0x0c, 0);
    put(app, kSunMatrix + 0x10, kFloatOne);
    put(app, kSunMatrix + 0x14, 0);
    put(app, kSunMatrix + 0x18, 0);
    put(app, kSunMatrix + 0x1c, 0);
    put(app, kSunMatrix + 0x20, kFloatOne);
    put(app, kSunShadingMode, 0);
    put(app, kSunDirection, 0);
    put(app, kSunDirection + 8, 0);
    put(app, kSunCapRed, 0xff);
    put(app, kSunCapGreen, 0xff);
    put(app, kSunCapBlue, 0xff);
    put(app, kSunSlopeRed, 0xff);
    put(app, kSunSlopeGreen, 0xff);
    put(app, kSunSlopeBlue, 0xff);
    put(app, kSunAlphaMask, 0x7fffffff);
    put(app, kSunPlainColour, 0x7f7f7f7f);
    put(app, kSunDirection + 4, kFloatMinusOne);
}

/* 0x49d640: sunLightingReset; the registers the code keeps (ebx, ecx, edx, esi, edi, ebp pushed and popped,
 * their values on the stack below), eax 0. */
x86::reg32 resetSunLighting(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebx);
    push(app, cpu, args.ecx);
    push(app, cpu, args.edx);
    push(app, cpu, args.esi);
    push(app, cpu, args.edi);
    push(app, cpu, args.ebp);
    sunLightingReset(app);
    args.eax = 0;
    args.ebp = pop(app, cpu);
    args.edi = pop(app, cpu);
    args.esi = pop(app, cpu);
    args.edx = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
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
x86::reg32 resetHandlingTunings(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    push(app, cpu, args.ebx);
    push(app, cpu, args.ecx);
    push(app, cpu, args.edx);
    push(app, cpu, args.esi);
    push(app, cpu, args.edi);
    push(app, cpu, args.ebp);
    args.ebp = cpu.esp;
    args.edx = 0x3f99999a;
    args.ecx = 0x40400000;
    args.ebx = 0x3f800000;
    args.esi = 0xbd23d70a;
    args.edi = 0x3e19999a;
    args.eax = 0xbb67ff58;
    put(app, kHandlingTunings + 0x10, args.edx);
    put(app, kHandlingTunings + 0x14, args.ecx);
    put(app, kHandlingTunings + 0x00, args.ebx);
    put(app, kHandlingTunings + 0x04, args.esi);
    put(app, kHandlingTunings + 0x08, args.edi);
    put(app, kHandlingTunings + 0x0c, args.eax);
    args.edx = 0x2bc;
    args.ecx = 0xaf;
    args.esi = 5;
    args.edi = 0x3fc00000;
    args.eax = 0x3f7ae148;
    args.ebx = 0;
    put(app, kHandlingTunings + 0x18, args.edx);
    put(app, kHandlingTunings + 0x1c, args.ecx);
    put(app, kHandlingTunings + 0x20, args.ebx);
    put(app, kHandlingTunings + 0x24, args.esi);
    put(app, kHandlingTunings + 0x28, args.edi);
    put(app, kHandlingTunings + 0x2c, args.eax);
    args.edx = 0x3f5eb852;
    args.ecx = 0x3f000000;
    args.ebx = 0x41200000;
    args.esi = 0x3b888889;
    args.edi = 0x3bcccccd;
    put(app, kHandlingTunings + 0x30, args.edx);
    put(app, kHandlingTunings + 0x34, args.ecx);
    put(app, kHandlingTunings + 0x40, args.ebx);
    put(app, kHandlingTunings + 0x38, args.esi);
    put(app, kHandlingTunings + 0x3c, args.edi);
    args.ebp = pop(app, cpu);
    args.edi = pop(app, cpu);
    args.esi = pop(app, cpu);
    args.edx = pop(app, cpu);
    args.ecx = pop(app, cpu);
    args.ebx = pop(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

}  // namespace decompiled
}  // namespace nfs3hp
