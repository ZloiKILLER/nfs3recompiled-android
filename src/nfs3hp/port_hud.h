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
u32 loadingTextPrepared(App* app, ThreadState& ts, nfs::Args& args);    // 0x484040
u32 floatsSwapped(App* app, ThreadState& ts, nfs::Args& args);          // 0x485c20
u32 fourIntegersAppended(App* app, ThreadState& ts, nfs::Args& args);   // 0x487350
u32 pointTransformed(App* app, ThreadState& ts, nfs::Args& args);       // 0x48cb50
u32 boxDrawn(App* app, ThreadState& ts, nfs::Args& args);               // 0x48cc60
u32 indicatorGeometryUpdated(App* app, ThreadState& ts, nfs::Args& args); // 0x48a220
u32 textAlignedDrawn(App* app, ThreadState& ts, nfs::Args& args);       // 0x4897f0
u32 percentTextDrawn(App* app, ThreadState& ts, nfs::Args& args);       // 0x489d70
u32 listRowsDrawn(App* app, ThreadState& ts, nfs::Args& args);          // 0x489b30
u32 timeTextDrawn(App* app, ThreadState& ts, nfs::Args& args);          // 0x489c40
u32 viewZoomStepped(App* app, ThreadState& ts, nfs::Args& args);        // 0x48ea20
u32 viewPaneStepped(App* app, ThreadState& ts, nfs::Args& args);        // 0x48e790
u32 screenScalesSet(App* app, ThreadState& ts, nfs::Args& args);        // 0x484b70
u32 rectangleQuadSet(App* app, ThreadState& ts, nfs::Args& args);       // 0x4870c0
u32 stackQuadSet(App* app, ThreadState& ts, nfs::Args& args);           // 0x486f70
u32 labelTextSet(App* app, ThreadState& ts, nfs::Args& args);           // 0x485180
// 0x485180's core (labelTextSet): what it leaves in eax, nullopt when the game stopped; esp the frame base.
std::optional<u32> labelTextSetOf(App* app, ThreadState& ts, u32 esp, u32 group, u32 slot, u32 shown, u32 record);
u32 pointClipped(App* app, ThreadState& ts, nfs::Args& args);           // 0x485c30
u32 segmentClippedToRect(App* app, ThreadState& ts, nfs::Args& args);   // 0x485f20
u32 segmentCutIntoPieces(App* app, ThreadState& ts, nfs::Args& args);   // 0x4860b0
u32 strokeClippedDrawn(App* app, ThreadState& ts, nfs::Args& args);     // 0x4873e0
u32 standingsGapMeasured(App* app, ThreadState& ts, nfs::Args& args);   // 0x485a60
// The game's 0x485a60: the gap between the cars `first` (eax) and `second` (edx); eax what it leaves.
u32 standingsGapOf(App* app, u32 first, u32 second);
u32 floatsQuadSet(App* app, ThreadState& ts, nfs::Args& args);          // 0x486df0
u32 hudStarted(App* app, ThreadState& ts, nfs::Args& args);             // 0x484070
u32 rankTextDrawn(App* app, ThreadState& ts, nfs::Args& args);          // 0x48a3a0
u32 labelRowsDrawn(App* app, ThreadState& ts, nfs::Args& args);         // 0x48a660
u32 standingsDrawn(App* app, ThreadState& ts, nfs::Args& args);         // 0x48ae70
u32 columnsStarted(App* app, ThreadState& ts, nfs::Args& args);         // 0x48aa40
u32 mapOutlineStarted(App* app, ThreadState& ts, nfs::Args& args);      // 0x48cdb0
u32 hudViewsSet(App* app, ThreadState& ts, nfs::Args& args);            // 0x48d110
u32 hudPaneSet(App* app, ThreadState& ts, nfs::Args& args);              // 0x4844c0
u32 hudTablesStarted(App* app, ThreadState& ts, nfs::Args& args);        // 0x484c10
u32 indicatorQuadSet(App* app, ThreadState& ts, nfs::Args& args);       // 0x489e00
u32 gaugeQuadClipped(App* app, ThreadState& ts, nfs::Args& args);       // 0x487620
u32 segmentsMet(App* app, ThreadState& ts, nfs::Args& args);            // 0x486340
u32 boxQuadsSet(App* app, ThreadState& ts, nfs::Args& args);            // 0x487a30
u32 markersSet(App* app, ThreadState& ts, nfs::Args& args);             // 0x48b5f0
u32 mapPointsDrawn(App* app, ThreadState& ts, nfs::Args& args);         // 0x48d4c0

/* The typed core the menus call as C++ (guest::direct); nullopt when the game stopped in a call.  Its `esp` is the frame base its
 * thunk hands it: the game's esp on entry, at the return address.  A caller through guest::direct passes
 * ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the thunk hands it, so the core's
 * frame lies where the game's does; by address the call makes it. */

// The game's 0x4844c0: the HUD's pane of group in.eax set up for a frame, in.edx, in.ebx and in.ecx handed on as
// the caller's registers to what it calls; esp the frame base.
Result hudPaneSetOf(App* app, ThreadState& ts, u32 esp, const Args& in);

/* The other thunks' typed cores, called as C++ by the HUD's own code and the menus' (native_frontend_hudr,
 * _hudx, _loopa).  Each takes the registers its callers hand it -- the others are the ts's, as a call by
 * address leaves them -- and the words the call pushes, which it writes above `esp` where the call puts them
 * (the code reads them there, and some write back into them). */
// The game's 0x484040: the loading screen's text set up; `eax` back, as it came; esp the frame base.
Result loadingTextOf(App* app, ThreadState& ts, u32 esp, u32 eax);
// The game's 0x484b70: the HUD's four scales set from the screen's width: what the code leaves in eax; esp the frame base.
Result screenScalesOf(App* app, ThreadState& ts, u32 esp);
// The game's 0x485c20: the float at `b` into `a`, rounded as a float's load and store, and `a`'s old bits into `b`: `a` back; esp the frame base.
Result floatsSwappedOf(App* app, ThreadState& ts, u32 esp, u32 a, u32 b);
// The game's 0x485f20: segment `segment` clipped to rectangle `rect` into `out`, its flags at `flagsAt`; esp the frame base.
Result segmentClippedOf(App* app, ThreadState& ts, u32 esp, u32 segment, u32 rect, u32 out, u32 flagsAt);
// The game's 0x486340: where two segments meet (the decompiled code's, its registers as handed); esp the frame base.
Result segmentsMetOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx, u32 first, u32 second);
// The game's 0x486df0: a quad of a box's floats (its ten words as the call pushes them); esp the frame base.
Result floatsQuadOf(App* app, ThreadState& ts, u32 esp, const guest::Words& words);
// The game's 0x486f70: a quad from the stack's ten words, `ecx` handed on to the vertices' calls; esp the frame base.
Result stackQuadOf(App* app, ThreadState& ts, u32 esp, u32 ecx, const guest::Words& words);
// The game's 0x4870c0: a rectangle's quad (its registers as handed); esp the frame base.
Result rectangleQuadOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx);
// The game's 0x4873e0: group `group`'s stroke (mode `mode`) of segment `segment` in `colour` on `surface`, clipped and drawn: `group` back; esp the frame base.
Result strokeOf(App* app, ThreadState& ts, u32 esp, u32 group, u32 mode, u32 segment, u32 colour, u32 surface);
// The game's 0x487620: a gauge's quad clipped (the decompiled code's, its registers as handed, ten words); esp the frame base.
Result gaugeQuadOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx, const guest::Words& words);
// The game's 0x4897f0: text `text` at scale `scale` in `colour`, aligned by `mode` at `x` (+`dx`), `y`; the caller's x written back where the call pushed it; esp the frame base.
Result textAlignedOf(App* app, ThreadState& ts, u32 esp, u32 text, u32 scale, u32 colour, u32 x, u32 dx, u32 y, u32 mode);
// The game's 0x489b30: a list of `count` rows, `chosen` (1 on) and the last (`total` - 1) in colours of their own, between `top` and `bottom` at `x`; esp the frame base.
Result listRowsOf(App* app, ThreadState& ts, u32 esp, u32 count, u32 chosen, u32 total, u32 scale, u32 colourRow, u32 colourChosen, u32 colourLast, u32 top, u32 bottom, u32 x);
// The game's 0x489c40: `time` (ms) as seconds and tenths with the language's lines, at `x`, `y`; esp the frame base.
Result timeTextOf(App* app, ThreadState& ts, u32 esp, u32 time, u32 colour, u32 colourSecond, u32 x, u32 y, u32 scale);
// The game's 0x489d70: a percentage of the float `value` (its bits) with the language's line (`first`: the first), drawn aligned; esp the frame base.
Result percentTextOf(App* app, ThreadState& ts, u32 esp, u32 value, u32 first, u32 scale, u32 colour, u32 x, u32 top);
// The game's 0x489e00: an indicator's quad set (its registers as handed); esp the frame base.
Result indicatorQuadOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx);
// The game's 0x48a220: indicator `index`'s corners from `angle` (its bits), in `colour`; esp the frame base.
Result indicatorGeometryOf(App* app, ThreadState& ts, u32 esp, u32 index, u32 angle, u32 colour);
// The game's 0x48a3a0: place `place` among `among` with the language's letters; esp the frame base.
Result rankTextOf(App* app, ThreadState& ts, u32 esp, u32 place, u32 among, u32 scaleSuffix, u32 scaleMain, u32 colour, u32 left, u32 right, u32 top, u32 mode);
// The game's 0x48a660: group `group`'s list of labels; esp the frame base.
Result labelRowsOf(App* app, ThreadState& ts, u32 esp, u32 group, u32 scale, u32 left, u32 right, u32 width);
// The game's 0x48ae70: the standings of view `group`; esp the frame base.
Result standingsOf(App* app, ThreadState& ts, u32 esp, u32 scale, u32 left, u32 width, u32 group);
// The game's 0x48b5f0: the markers (the decompiled code's, its registers as handed); esp the frame base.
Result markersOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx);
// The game's 0x48cc60: gauge `gauge`'s box round (x, y), `width` and `height` from it (floats' bits), `edx` handed on to the quad; esp the frame base.
Result boxOf(App* app, ThreadState& ts, u32 esp, u32 gauge, u32 edx, u32 x, u32 y, u32 width, u32 height);
// The game's 0x48cb50: the point at the pushed `x`, `y` moved and turned for gauge `gauge` into `out` (the words changed where the call pushed them): `gauge` back; esp the frame base.
Result pointTransformedOf(App* app, ThreadState& ts, u32 esp, u32 gauge, u32 x, u32 y, u32 out);
// The game's 0x48d110: group `group`'s map views set (`edx`, `ebx`, `ecx` as handed); esp the frame base.
Result hudViewsOf(App* app, ThreadState& ts, u32 esp, u32 group, u32 edx, u32 ebx, u32 ecx);
// The game's 0x48d4c0: the map's points (the decompiled code's, its registers as handed); esp the frame base.
Result mapPointsOf(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ebx, u32 ecx);
// The game's 0x48e790: view `view`'s map pane stepped; esp the frame base.
Result paneStepOf(App* app, ThreadState& ts, u32 esp, u32 view);
// The game's 0x48ea20: view `view`'s zoom stepped (`step` 0 up, 1 down); esp the frame base.
Result zoomStepOf(App* app, ThreadState& ts, u32 esp, u32 view, u32 step);

}  // namespace hud
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
