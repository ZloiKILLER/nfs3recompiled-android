#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);

/* sub_47fcb0: native, nfs3hp::frontend::rendUnqueueEightQuads */

/* sub_47fd80: native, nfs3hp::frontend::rendFitFrameToScreen */

/* sub_4800a0: native, nfs3hp::render::hudBand */

/* sub_4808b0: native, nfs3hp::frontend::hudUnqueueViewQuads */

/* sub_480910: native, nfs3hp::render::hudRect */

/* sub_480a00: native, nfs3hp::frontend::hudSetAlignments */

/* sub_480fc0: native, nfs3hp::render::hudPicture */

/* sub_481170: native, nfs3hp::frontend::hudPlaceDialArt */

/* sub_481290: native, nfs3hp::frontend::hudLayMarkersFrame */

/* sub_4812e0: native, nfs3hp::render::hudDial */

/* sub_481510: native, nfs3hp::frontend::hudLayPanel */

/* sub_481570: native, nfs3hp::frontend::hudLayNothing */

/* sub_4815d0: native, nfs3hp::frontend::hudLayReplayBar */

/* sub_4818b0: native, nfs3hp::frontend::hudLayGrid */

/* sub_481aa0: native, nfs3hp::render::hudPanel */

/* sub_481c50: native, nfs3hp::render::hudTable */

/* sub_481f90: native, nfs3hp::frontend::hudMapPaneIsSmall */

/* sub_482030: native, nfs3hp::frontend::hudSetGridHighlight */

/* sub_482190: native, nfs3hp::frontend::hudDrawGridHighlight */

/* sub_482610: native, nfs3hp::dead::stub_0x482610 */

/* sub_482620: native, nfs3hp::render::countdown */

/* sub_482880: native, nfs3hp::frontend::hudDrawReplayBar */

/* sub_482940: native, nfs3hp::graphics::hudMessageBox */

/* sub_482b00: native, nfs3hp::render::hud */

/* sub_484030: native, nfs3hp::frontend::hudFlushFrameQueue */

/* sub_484040: native, nfs3hp::port::hud::loadingTextPrepared */

/* sub_484070: native, nfs3hp::port::hud::hudStarted */

/* sub_484220: native, nfs3hp::frontend::hudClearRebuildFlag */

/* sub_484230: native, nfs3hp::dead::stub_0x484230 */

/* sub_484260: native, nfs3hp::frontend::hudToggleShown */

/* sub_484330: native, nfs3hp::pieces::stub_0x484330 */

/* sub_484390: native, nfs3hp::dead::stub_0x484390 */

/* sub_4843f0: native, nfs3hp::dead::stub_0x4843f0 */

/* sub_484450: native, nfs3hp::frontend::hudToggleRearView */

/* sub_4844c0: native, nfs3hp::port::hud::hudPaneSet */

/* sub_484b70: native, nfs3hp::port::hud::screenScalesSet */

/* sub_484c10: native, nfs3hp::port::hud::hudTablesStarted */

/* sub_485180: native, nfs3hp::port::hud::labelTextSet */

/* sub_4853c0: native, nfs3hp::frontend::hudRebuildElement */

/* sub_485640: native, nfs3hp::frontend::hudRebuildView */

/* sub_485680: native, nfs3hp::frontend::hudEditLoadDefaults */

/* sub_485880: native, nfs3hp::frontend::simHudDefaults */

/* sub_485a60: native, nfs3hp::port::hud::standingsGapMeasured */

/* sub_485c20: native, nfs3hp::port::hud::floatsSwapped */

/* sub_485c30: native, nfs3hp::port::hud::pointClipped */

/* sub_485f20: native, nfs3hp::port::hud::segmentClippedToRect */

/* sub_4860b0: native, nfs3hp::port::hud::segmentCutIntoPieces */

/* sub_486340: native, nfs3hp::port::hud::segmentsMet */

/* sub_486df0: native, nfs3hp::port::hud::floatsQuadSet */

/* sub_486f70: native, nfs3hp::port::hud::stackQuadSet */

/* sub_4870c0: native, nfs3hp::port::hud::rectangleQuadSet */

/* sub_487210: native, nfs3hp::render::hudInset */

/* sub_487340: native, nfs3hp::frontend::hudGroupClearCount */

/* sub_487350: native, nfs3hp::port::hud::fourIntegersAppended */

/* sub_4873c0: native, nfs3hp::frontend::hudGroupDropLast */

/* sub_4873e0: native, nfs3hp::port::hud::strokeClippedDrawn */

/* sub_487620: native, nfs3hp::port::hud::gaugeQuadClipped */

/* sub_487a30: native, nfs3hp::port::hud::boxQuadsSet */

/* sub_4880b0: native, nfs3hp::render::needles */

/* sub_488470: native, nfs3hp::render::barGauges */

/* sub_488a80: native, nfs3hp::graphics::loadCabinInstruments */

/* sub_489240: native, nfs3hp::render::cockpit */

/* sub_4896f0: native, nfs3hp::frontend::hudOrdinalLetters */

/* sub_489750: native, nfs3hp::frontend::hudFormatTime */

/* sub_4897f0: native, nfs3hp::port::hud::textAlignedDrawn */

/* sub_4898b0: native, nfs3hp::frontend::hudDrawTime */

/* sub_489900: native, nfs3hp::frontend::hudDrawPercentAndCount */

/* sub_4899d0: native, nfs3hp::frontend::hudDrawLaps */

/* sub_489a50: native, nfs3hp::frontend::hudDrawRatio */

/* sub_489ab0: native, nfs3hp::dead::stub_0x489ab0 */

/* sub_489b30: native, nfs3hp::port::hud::listRowsDrawn */

/* sub_489c40: native, nfs3hp::port::hud::timeTextDrawn */

/* sub_489d70: native, nfs3hp::port::hud::percentTextDrawn */

/* sub_489e00: native, nfs3hp::port::hud::indicatorQuadSet */

/* sub_48a220: native, nfs3hp::port::hud::indicatorGeometryUpdated */

/* sub_48a320: native, nfs3hp::frontend::hudDrawPlaceAmong */

/* sub_48a3a0: native, nfs3hp::port::hud::rankTextDrawn */

/* sub_48a660: native, nfs3hp::port::hud::labelRowsDrawn */

/* sub_48aa40: native, nfs3hp::port::hud::columnsStarted */

/* sub_48ae70: native, nfs3hp::port::hud::standingsDrawn */

/* sub_48b2a0: native, nfs3hp::frontend::hudDrawTriangle */

/* sub_48b4a0: native, nfs3hp::frontend::hudDrawLine */

/* sub_48b5f0: native, nfs3hp::port::hud::markersSet */

/* sub_48c0e0: native, nfs3hp::render::segmentMeter */

/* sub_48c7c0: native, nfs3hp::render::startLights */

/* sub_48cae0: native, nfs3hp::dead::stub_0x48cae0 */

/* sub_48caf0: native, nfs3hp::dead::stub_0x48caf0 */

/* sub_48cb50: native, nfs3hp::port::hud::pointTransformed */

/* sub_48cc60: native, nfs3hp::port::hud::boxDrawn */

/* sub_48ccf0: native, nfs3hp::frontend::hudRefreshPane */

/* sub_48cdb0: native, nfs3hp::port::hud::mapOutlineStarted */

/* sub_48d110: native, nfs3hp::port::hud::hudViewsSet */

/* sub_48d4c0: native, nfs3hp::port::hud::mapPointsDrawn */

/* sub_48e770: native, nfs3hp::frontend::hudFlushLines */

/* sub_48e780: native, nfs3hp::frontend::hudPrepareLines */

/* sub_48e790: native, nfs3hp::port::hud::viewPaneStepped */

/* sub_48e920: native, nfs3hp::frontend::hudStepMapPanes */

/* sub_48ea20: native, nfs3hp::port::hud::viewZoomStepped */

/* sub_48eb60: native, nfs3hp::frontend::hudStepZoom */

/* sub_48eb90: native, nfs3hp::port::timerThread */

}
