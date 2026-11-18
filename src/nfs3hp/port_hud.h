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
 * Each function here is a thunk under the game's name: the game's code calls them by name. */
namespace nfs3hp
{
namespace port
{
namespace hud
{

/* The thunks, under the game's functions' names. */
reg32 loadingTextPrepared(App* app, CPU& cpu, x86::Args& args);    // 0x484040
reg32 floatsSwapped(App* app, CPU& cpu, x86::Args& args);          // 0x485c20
reg32 fourIntegersAppended(App* app, CPU& cpu, x86::Args& args);   // 0x487350
reg32 pointTransformed(App* app, CPU& cpu, x86::Args& args);       // 0x48cb50
reg32 boxDrawn(App* app, CPU& cpu, x86::Args& args);               // 0x48cc60
reg32 indicatorGeometryUpdated(App* app, CPU& cpu, x86::Args& args); // 0x48a220
reg32 textAlignedDrawn(App* app, CPU& cpu, x86::Args& args);       // 0x4897f0
reg32 percentTextDrawn(App* app, CPU& cpu, x86::Args& args);       // 0x489d70
reg32 listRowsDrawn(App* app, CPU& cpu, x86::Args& args);          // 0x489b30
reg32 timeTextDrawn(App* app, CPU& cpu, x86::Args& args);          // 0x489c40
reg32 viewZoomStepped(App* app, CPU& cpu, x86::Args& args);        // 0x48ea20
reg32 viewPaneStepped(App* app, CPU& cpu, x86::Args& args);        // 0x48e790
reg32 screenScalesSet(App* app, CPU& cpu, x86::Args& args);        // 0x484b70
reg32 rectangleQuadSet(App* app, CPU& cpu, x86::Args& args);       // 0x4870c0
reg32 stackQuadSet(App* app, CPU& cpu, x86::Args& args);           // 0x486f70
reg32 labelTextSet(App* app, CPU& cpu, x86::Args& args);           // 0x485180
// 0x485180's core (labelTextSet): what it leaves in eax, nullopt when the game stopped; esp the frame base.
std::optional<reg32> labelTextSetOf(App* app, CPU& cpu, reg32 esp, reg32 group, reg32 slot, reg32 shown, reg32 record);
reg32 pointClipped(App* app, CPU& cpu, x86::Args& args);           // 0x485c30
reg32 segmentClippedToRect(App* app, CPU& cpu, x86::Args& args);   // 0x485f20
reg32 segmentCutIntoPieces(App* app, CPU& cpu, x86::Args& args);   // 0x4860b0
reg32 strokeClippedDrawn(App* app, CPU& cpu, x86::Args& args);     // 0x4873e0
reg32 standingsGapMeasured(App* app, CPU& cpu, x86::Args& args);   // 0x485a60
// The game's 0x485a60: the gap between the cars `first` (eax) and `second` (edx); eax what it leaves.
reg32 standingsGapOf(App* app, reg32 first, reg32 second);
reg32 floatsQuadSet(App* app, CPU& cpu, x86::Args& args);          // 0x486df0
reg32 hudStarted(App* app, CPU& cpu, x86::Args& args);             // 0x484070
reg32 rankTextDrawn(App* app, CPU& cpu, x86::Args& args);          // 0x48a3a0
reg32 labelRowsDrawn(App* app, CPU& cpu, x86::Args& args);         // 0x48a660
reg32 standingsDrawn(App* app, CPU& cpu, x86::Args& args);         // 0x48ae70
reg32 columnsStarted(App* app, CPU& cpu, x86::Args& args);         // 0x48aa40
reg32 mapOutlineStarted(App* app, CPU& cpu, x86::Args& args);      // 0x48cdb0
reg32 hudViewsSet(App* app, CPU& cpu, x86::Args& args);            // 0x48d110
reg32 hudPaneSet(App* app, CPU& cpu, x86::Args& args);              // 0x4844c0
reg32 hudTablesStarted(App* app, CPU& cpu, x86::Args& args);        // 0x484c10
reg32 indicatorQuadSet(App* app, CPU& cpu, x86::Args& args);       // 0x489e00
reg32 gaugeQuadClipped(App* app, CPU& cpu, x86::Args& args);       // 0x487620
reg32 segmentsMet(App* app, CPU& cpu, x86::Args& args);            // 0x486340
reg32 boxQuadsSet(App* app, CPU& cpu, x86::Args& args);            // 0x487a30
reg32 markersSet(App* app, CPU& cpu, x86::Args& args);             // 0x48b5f0
reg32 mapPointsDrawn(App* app, CPU& cpu, x86::Args& args);         // 0x48d4c0

/* The typed core the menus call as C++ (guest::direct); nullopt when the game stopped in a call.  Its `esp` is the frame base its
 * thunk hands it: the game's esp on entry, at the return address.  A caller through guest::direct passes
 * cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the thunk hands it, so the core's
 * frame lies where the game's does; by address the call makes it. */

// The game's 0x4844c0: the HUD's pane of group in.eax set up for a frame, in.edx, in.ebx and in.ecx handed on as
// the caller's registers to what it calls; esp the frame base.
Result hudPaneSetOf(App* app, CPU& cpu, reg32 esp, const Args& in);

/* The other thunks' typed cores, called as C++ by the HUD's own code and the menus' (native_frontend_hudr,
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

}  // namespace hud
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
