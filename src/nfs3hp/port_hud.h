#ifndef NFS3HP_PORT_HUD_H
#define NFS3HP_PORT_HUD_H

#include "port.h"

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

}  // namespace hud
}  // namespace port
}  // namespace nfs3hp

#endif
