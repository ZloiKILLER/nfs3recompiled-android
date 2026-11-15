#ifndef NFS3HP_PORT_HUD_H
#define NFS3HP_PORT_HUD_H

#include "native_guest.h"
#include "port.h"

#include <initializer_list>

/* The race HUD, rewritten as the port's own, without the x87 (port.h says
 * how; port_hud.cpp): its gauges, needles and the track's map, one function
 * at a time.  The arithmetic is the host's doubles rounded to a float where
 * the game stores a float (the game's single-precision race is not followed:
 * only what is drawn sees it), as port_view.h has it.
 *
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_HUD_NATIVES): the generated code calls them by name.  tools/game_checks
 * holds each against the game's code for it (hud.inc). */
namespace nfs3hp
{
namespace port
{
namespace hud
{

/* The adapters, under the game's functions' names. */
void loadingTextPrepared(App* app, CPU& cpu);    // 0x484040
void floatsSwapped(App* app, CPU& cpu);          // 0x485c20
void fourIntegersAppended(App* app, CPU& cpu);   // 0x487350
void pointTransformed(App* app, CPU& cpu);       // 0x48cb50
void boxDrawn(App* app, CPU& cpu);               // 0x48cc60
void indicatorGeometryUpdated(App* app, CPU& cpu); // 0x48a220
void textAlignedDrawn(App* app, CPU& cpu);       // 0x4897f0
void percentTextDrawn(App* app, CPU& cpu);       // 0x489d70
void listRowsDrawn(App* app, CPU& cpu);          // 0x489b30
void timeTextDrawn(App* app, CPU& cpu);          // 0x489c40
void viewZoomStepped(App* app, CPU& cpu);        // 0x48ea20
void viewPaneStepped(App* app, CPU& cpu);        // 0x48e790
void screenScalesSet(App* app, CPU& cpu);        // 0x484b70
void rectangleQuadSet(App* app, CPU& cpu);       // 0x4870c0
void stackQuadSet(App* app, CPU& cpu);           // 0x486f70
void labelTextSet(App* app, CPU& cpu);           // 0x485180
void pointClipped(App* app, CPU& cpu);           // 0x485c30
void segmentClippedToRect(App* app, CPU& cpu);   // 0x485f20
void segmentCutIntoPieces(App* app, CPU& cpu);   // 0x4860b0
void strokeClippedDrawn(App* app, CPU& cpu);     // 0x4873e0
void standingsGapMeasured(App* app, CPU& cpu);   // 0x485a60
void floatsQuadSet(App* app, CPU& cpu);          // 0x486df0
void hudStarted(App* app, CPU& cpu);             // 0x484070
void rankTextDrawn(App* app, CPU& cpu);          // 0x48a3a0
void labelRowsDrawn(App* app, CPU& cpu);         // 0x48a660
void standingsDrawn(App* app, CPU& cpu);         // 0x48ae70
void columnsStarted(App* app, CPU& cpu);         // 0x48aa40
void mapOutlineStarted(App* app, CPU& cpu);      // 0x48cdb0
void hudViewsSet(App* app, CPU& cpu);            // 0x48d110
void hudPaneSet(App* app, CPU& cpu);              // 0x4844c0
void hudTablesStarted(App* app, CPU& cpu);        // 0x484c10
void indicatorQuadSet(App* app, CPU& cpu);       // 0x489e00
void gaugeQuadClipped(App* app, CPU& cpu);       // 0x487620
void segmentsMet(App* app, CPU& cpu);            // 0x486340
void boxQuadsSet(App* app, CPU& cpu);            // 0x487a30
void markersSet(App* app, CPU& cpu);             // 0x48b5f0
void mapPointsDrawn(App* app, CPU& cpu);         // 0x48d4c0

/* The typed core the menus call as C++ (guest::direct), and how the game's code calls it by address (ByAddress
 * below, for the checks' stand-ins); nullopt when the game stopped in a call.  Its `esp` is the frame base its
 * adapter hands it: the game's esp on entry, at the return address.  A caller through guest::direct passes
 * cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the adapter hands it, so the core's
 * frame lies where the game's does; by address the call makes it. */

// The game's 0x4844c0: the HUD's pane of group in.eax set up for a frame, in.edx, in.ebx and in.ecx handed on as
// the caller's registers to what it calls; esp the frame base.
Result hudPaneSetOf(App* app, CPU& cpu, reg32 esp, const Args& in);

/* The other adapters' typed cores, called as C++ by the HUD's own code and the menus' (native_frontend_hudr,
 * _hudx, _loopa).  Each takes the registers its callers hand it -- the others are the cpu's, as a call by
 * address leaves them -- and the words the call pushes, which it writes above `esp` where the call puts them
 * (the code reads them there, and some write back into them). */
// The game's 0x484040: the loading screen's text set up; `eax` back, as it came; esp the frame base.
Result loadingTextOf(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x484b70: the HUD's four scales set from the screen's width: what the code leaves in eax; esp the frame base.
Result screenScalesOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x485c20: the float at `b` into `a`, rounded as a float's load and store, and `a`'s old bits into `b`: `a` back; esp the frame base.
Result floatsSwappedOf(App* app, CPU& cpu, reg32 esp, reg32 a, reg32 b);
// The game's 0x485f20: segment `segment` clipped to rectangle `rect` into `out`, its flags at `flagsAt`; esp the frame base.
Result segmentClippedOf(App* app, CPU& cpu, reg32 esp, reg32 segment, reg32 rect, reg32 out, reg32 flagsAt);
// The game's 0x486340: where two segments meet (the decompiled code's, its registers as handed); esp the frame base.
Result segmentsMetOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx, reg32 first, reg32 second);
// The game's 0x486df0: a quad of a box's floats (its ten words as the call pushes them); esp the frame base.
Result floatsQuadOf(App* app, CPU& cpu, reg32 esp, const guest::Words& words);
// The game's 0x486f70: a quad from the stack's ten words, `ecx` handed on to the vertices' calls; esp the frame base.
Result stackQuadOf(App* app, CPU& cpu, reg32 esp, reg32 ecx, const guest::Words& words);
// The game's 0x4870c0: a rectangle's quad (its registers as handed); esp the frame base.
Result rectangleQuadOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx);
// The game's 0x4873e0: group `group`'s stroke (mode `mode`) of segment `segment` in `colour` on `surface`, clipped and drawn: `group` back; esp the frame base.
Result strokeOf(App* app, CPU& cpu, reg32 esp, reg32 group, reg32 mode, reg32 segment, reg32 colour, reg32 surface);
// The game's 0x487620: a gauge's quad clipped (the decompiled code's, its registers as handed, ten words); esp the frame base.
Result gaugeQuadOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx, const guest::Words& words);
// The game's 0x4897f0: text `text` at scale `scale` in `colour`, aligned by `mode` at `x` (+`dx`), `y`; the caller's x written back where the call pushed it; esp the frame base.
Result textAlignedOf(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 scale, reg32 colour, reg32 x, reg32 dx, reg32 y, reg32 mode);
// The game's 0x489b30: a list of `count` rows, `chosen` (1 on) and the last (`total` - 1) in colours of their own, between `top` and `bottom` at `x`; esp the frame base.
Result listRowsOf(App* app, CPU& cpu, reg32 esp, reg32 count, reg32 chosen, reg32 total, reg32 scale, reg32 colourRow, reg32 colourChosen, reg32 colourLast, reg32 top, reg32 bottom, reg32 x);
// The game's 0x489c40: `time` (ms) as seconds and tenths with the language's lines, at `x`, `y`; esp the frame base.
Result timeTextOf(App* app, CPU& cpu, reg32 esp, reg32 time, reg32 colour, reg32 colourSecond, reg32 x, reg32 y, reg32 scale);
// The game's 0x489d70: a percentage of the float `value` (its bits) with the language's line (`first`: the first), drawn aligned; esp the frame base.
Result percentTextOf(App* app, CPU& cpu, reg32 esp, reg32 value, reg32 first, reg32 scale, reg32 colour, reg32 x, reg32 top);
// The game's 0x489e00: an indicator's quad set (its registers as handed); esp the frame base.
Result indicatorQuadOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx);
// The game's 0x48a220: indicator `index`'s corners from `angle` (its bits), in `colour`; esp the frame base.
Result indicatorGeometryOf(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 angle, reg32 colour);
// The game's 0x48a3a0: place `place` among `among` with the language's letters; esp the frame base.
Result rankTextOf(App* app, CPU& cpu, reg32 esp, reg32 place, reg32 among, reg32 scaleSuffix, reg32 scaleMain, reg32 colour, reg32 left, reg32 right, reg32 top, reg32 mode);
// The game's 0x48a660: group `group`'s list of labels; esp the frame base.
Result labelRowsOf(App* app, CPU& cpu, reg32 esp, reg32 group, reg32 scale, reg32 left, reg32 right, reg32 width);
// The game's 0x48ae70: the standings of view `group`; esp the frame base.
Result standingsOf(App* app, CPU& cpu, reg32 esp, reg32 scale, reg32 left, reg32 width, reg32 group);
// The game's 0x48b5f0: the markers (the decompiled code's, its registers as handed); esp the frame base.
Result markersOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx);
// The game's 0x48cc60: gauge `gauge`'s box round (x, y), `width` and `height` from it (floats' bits), `edx` handed on to the quad; esp the frame base.
Result boxOf(App* app, CPU& cpu, reg32 esp, reg32 gauge, reg32 edx, reg32 x, reg32 y, reg32 width, reg32 height);
// The game's 0x48cb50: the point at the pushed `x`, `y` moved and turned for gauge `gauge` into `out` (the words changed where the call pushed them): `gauge` back; esp the frame base.
Result pointTransformedOf(App* app, CPU& cpu, reg32 esp, reg32 gauge, reg32 x, reg32 y, reg32 out);
// The game's 0x48d110: group `group`'s map views set (`edx`, `ebx`, `ecx` as handed); esp the frame base.
Result hudViewsOf(App* app, CPU& cpu, reg32 esp, reg32 group, reg32 edx, reg32 ebx, reg32 ecx);
// The game's 0x48d4c0: the map's points (the decompiled code's, its registers as handed); esp the frame base.
Result mapPointsOf(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx);
// The game's 0x48e790: view `view`'s map pane stepped; esp the frame base.
Result paneStepOf(App* app, CPU& cpu, reg32 esp, reg32 view);
// The game's 0x48ea20: view `view`'s zoom stepped (`step` 0 up, 1 down); esp the frame base.
Result zoomStepOf(App* app, CPU& cpu, reg32 esp, reg32 view, reg32 step);

/* A HUD core by address, for its ByAddress: `registers` into eax, edx, ebx and ecx in turn (the rest as they
 * are), `words` pushed (the function pops them); eax back, every other register kept. */
inline Result byAddress(App* app, CPU& cpu, reg32 address, std::initializer_list<reg32> registers,
                        const guest::Words& words)
{
    const guest::KeptRegisters kept(cpu);
    reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const reg32 value : registers)
        *into[i++] = value;
    if (!guest::call(app, cpu, address, {}, words))
        return std::nullopt;
    return int32_t(cpu.eax);
}

}  // namespace hud
}  // namespace port

namespace guest
{

// `in` in eax, edx, ebx and ecx; eax back.
template <>
struct ByAddress<port::hud::hudPaneSetOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const port::Args& in)
    {
        return watcomEax(app, cpu, Address, {in.eax, in.edx, in.ebx, in.ecx});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::hud::loadingTextOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return port::hud::byAddress(app, cpu, Address, {eax}, guest::Words{});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::hud::screenScalesOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return port::hud::byAddress(app, cpu, Address, {}, guest::Words{});
    }
};

// `a` in eax, `b` in edx; eax back.
template <>
struct ByAddress<port::hud::floatsSwappedOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 a, x86::reg32 b)
    {
        return port::hud::byAddress(app, cpu, Address, {a, b}, guest::Words{});
    }
};

// `segment` in eax, `rect` in edx, `out` in ebx, `flagsAt` in ecx; eax back.
template <>
struct ByAddress<port::hud::segmentClippedOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 segment, x86::reg32 rect, x86::reg32 out, x86::reg32 flagsAt)
    {
        return port::hud::byAddress(app, cpu, Address, {segment, rect, out, flagsAt}, guest::Words{});
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; `first`, `second` pushed; eax back.
template <>
struct ByAddress<port::hud::segmentsMetOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx, x86::reg32 first, x86::reg32 second)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, guest::Words{first, second});
    }
};

// Its ten words pushed; eax back.
template <>
struct ByAddress<port::hud::floatsQuadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, const guest::Words& words)
    {
        return port::hud::byAddress(app, cpu, Address, {}, words);
    }
};

// Its own in eax, its own in edx, its own in ebx, `ecx` in ecx; its ten words pushed; eax back.
template <>
struct ByAddress<port::hud::stackQuadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 ecx, const guest::Words& words)
    {
        return port::hud::byAddress(app, cpu, Address, {cpu.eax, cpu.edx, cpu.ebx, ecx}, words);
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; eax back.
template <>
struct ByAddress<port::hud::rectangleQuadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, guest::Words{});
    }
};

// `group` in eax, `mode` in edx, `segment` in ebx, `colour` in ecx; `surface` pushed; eax back.
template <>
struct ByAddress<port::hud::strokeOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 group, x86::reg32 mode, x86::reg32 segment, x86::reg32 colour, x86::reg32 surface)
    {
        return port::hud::byAddress(app, cpu, Address, {group, mode, segment, colour}, guest::Words{surface});
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; its ten words pushed; eax back.
template <>
struct ByAddress<port::hud::gaugeQuadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx, const guest::Words& words)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, words);
    }
};

// `text` in eax; `scale`, `colour`, `x`, `dx`, `y`, `mode` pushed; eax back.
template <>
struct ByAddress<port::hud::textAlignedOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text, x86::reg32 scale, x86::reg32 colour, x86::reg32 x, x86::reg32 dx, x86::reg32 y, x86::reg32 mode)
    {
        return port::hud::byAddress(app, cpu, Address, {text}, guest::Words{scale, colour, x, dx, y, mode});
    }
};

// `count` in eax, `chosen` in edx, `total` in ebx; `scale`, `colourRow`, `colourChosen`, `colourLast`, `top`, `bottom`, `x` pushed; eax back.
template <>
struct ByAddress<port::hud::listRowsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 count, x86::reg32 chosen, x86::reg32 total, x86::reg32 scale, x86::reg32 colourRow, x86::reg32 colourChosen, x86::reg32 colourLast, x86::reg32 top, x86::reg32 bottom, x86::reg32 x)
    {
        return port::hud::byAddress(app, cpu, Address, {count, chosen, total}, guest::Words{scale, colourRow, colourChosen, colourLast, top, bottom, x});
    }
};

// `time` in eax; `colour`, `colourSecond`, `x`, `y`, `scale` pushed; eax back.
template <>
struct ByAddress<port::hud::timeTextOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 time, x86::reg32 colour, x86::reg32 colourSecond, x86::reg32 x, x86::reg32 y, x86::reg32 scale)
    {
        return port::hud::byAddress(app, cpu, Address, {time}, guest::Words{colour, colourSecond, x, y, scale});
    }
};

// `value`, `first`, `scale`, `colour`, `x`, `top` pushed; eax back.
template <>
struct ByAddress<port::hud::percentTextOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 value, x86::reg32 first, x86::reg32 scale, x86::reg32 colour, x86::reg32 x, x86::reg32 top)
    {
        return port::hud::byAddress(app, cpu, Address, {}, guest::Words{value, first, scale, colour, x, top});
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; eax back.
template <>
struct ByAddress<port::hud::indicatorQuadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, guest::Words{});
    }
};

// `index` in eax; `angle`, `colour` pushed; eax back.
template <>
struct ByAddress<port::hud::indicatorGeometryOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 index, x86::reg32 angle, x86::reg32 colour)
    {
        return port::hud::byAddress(app, cpu, Address, {index}, guest::Words{angle, colour});
    }
};

// `place` in eax, `among` in edx; `scaleSuffix`, `scaleMain`, `colour`, `left`, `right`, `top`, `mode` pushed; eax back.
template <>
struct ByAddress<port::hud::rankTextOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 place, x86::reg32 among, x86::reg32 scaleSuffix, x86::reg32 scaleMain, x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top, x86::reg32 mode)
    {
        return port::hud::byAddress(app, cpu, Address, {place, among}, guest::Words{scaleSuffix, scaleMain, colour, left, right, top, mode});
    }
};

// `group` in eax; `scale`, `left`, `right`, `width` pushed; eax back.
template <>
struct ByAddress<port::hud::labelRowsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 group, x86::reg32 scale, x86::reg32 left, x86::reg32 right, x86::reg32 width)
    {
        return port::hud::byAddress(app, cpu, Address, {group}, guest::Words{scale, left, right, width});
    }
};

// `scale`, `left`, `width`, `group` pushed; eax back.
template <>
struct ByAddress<port::hud::standingsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 scale, x86::reg32 left, x86::reg32 width, x86::reg32 group)
    {
        return port::hud::byAddress(app, cpu, Address, {}, guest::Words{scale, left, width, group});
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; eax back.
template <>
struct ByAddress<port::hud::markersOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, guest::Words{});
    }
};

// `gauge` in eax, `edx` in edx; `x`, `y`, `width`, `height` pushed; eax back.
template <>
struct ByAddress<port::hud::boxOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 gauge, x86::reg32 edx, x86::reg32 x, x86::reg32 y, x86::reg32 width, x86::reg32 height)
    {
        return port::hud::byAddress(app, cpu, Address, {gauge, edx}, guest::Words{x, y, width, height});
    }
};

// `gauge` in eax; `x`, `y`, `out` pushed; eax back.
template <>
struct ByAddress<port::hud::pointTransformedOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 gauge, x86::reg32 x, x86::reg32 y, x86::reg32 out)
    {
        return port::hud::byAddress(app, cpu, Address, {gauge}, guest::Words{x, y, out});
    }
};

// `group` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; eax back.
template <>
struct ByAddress<port::hud::hudViewsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 group, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx)
    {
        return port::hud::byAddress(app, cpu, Address, {group, edx, ebx, ecx}, guest::Words{});
    }
};

// `eax` in eax, `edx` in edx, `ebx` in ebx, `ecx` in ecx; eax back.
template <>
struct ByAddress<port::hud::mapPointsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx, x86::reg32 ebx, x86::reg32 ecx)
    {
        return port::hud::byAddress(app, cpu, Address, {eax, edx, ebx, ecx}, guest::Words{});
    }
};

// `view` in eax; eax back.
template <>
struct ByAddress<port::hud::paneStepOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 view)
    {
        return port::hud::byAddress(app, cpu, Address, {view}, guest::Words{});
    }
};

// `view` in eax, `step` in edx; eax back.
template <>
struct ByAddress<port::hud::zoomStepOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 view, x86::reg32 step)
    {
        return port::hud::byAddress(app, cpu, Address, {view, step}, guest::Words{});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
