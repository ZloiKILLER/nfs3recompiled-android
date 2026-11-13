#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
bool widescreenMode(x86::reg32 width, x86::reg32 height);
x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);

/* sub_4be970: native, nfs3hp::frontend::raceViewsResize */

/* sub_4bea40: native, nfs3hp::frontend::raceViewResize */

/* sub_4beb00: native, nfs3hp::frontend::pauseSetRectFirst */

/* sub_4beb20: native, nfs3hp::frontend::pauseReadRect */

/* sub_4beb40: native, nfs3hp::frontend::pauseSetRectSecond */

/* sub_4beb60: native, nfs3hp::dead::stub_0x4beb60 */

/* sub_4beb80: native, nfs3hp::port::text::viewport */

/* sub_4bebb0: native, nfs3hp::frontend::simSetClipRect */

/* sub_4bebe0: native, nfs3hp::graphics::setClip */

/* sub_4becd0: native, nfs3hp::dead::stub_0x4becd0 */

/* sub_4bed20: native, nfs3hp::frontend::viewsUseWithProjection */

/* sub_4bed50: native, nfs3hp::frontend::viewsProjectionOnly */

/* sub_4bed70: native, nfs3hp::frontend::viewsUseClip */

/* sub_4bed90: native, nfs3hp::graphics::listScreenModes */

/* sub_4bee80: native, nfs3hp::graphics::findScreenMode */

/* sub_4bef50: native, nfs3hp::render::screenMode */

/* sub_4bf110: native, nfs3hp::frontend::rendFillReciprocals */

/* sub_4bf160: native, nfs3hp::dead::stub_0x4bf160 */

/* sub_4bf1b0: native, nfs3hp::frontend::rendSetClipBounds */

/* sub_4bf260: native, nfs3hp::frontend::rendSetProjection */

/* sub_4bf370: native, nfs3hp::frontend::rendProjectPoints */

/* sub_4bf4c0: native, nfs3hp::vertices::objectVertices */

/* sub_4bf790: native, nfs3hp::render::clipByZ */

/* sub_4c0b20: native, nfs3hp::render::line */

/* sub_4c0f60: native, nfs3hp::graphics::dead_0x4c0f60 */

/* sub_4c11b0: native, nfs3hp::vertices::clipTriangle */

/* sub_4c12e0: native, nfs3hp::vertices::clipQuad */

/* sub_4c1900: native, nfs3hp::frontend::rendDrawQuadClipped */

/* sub_4c19e0: native, nfs3hp::frontend::rendDrawQuadSplit */

/* sub_4c1aa0: native, nfs3hp::vertices::clipTriangleByZ */

/* sub_4c1e20: native, nfs3hp::vertices::quadMesh */

/* sub_4c20e0: native, nfs3hp::render::quadByZ */

/* sub_4c2cd0: native, nfs3hp::vertices::clipPolygon */

/* sub_4c3ad0: native, nfs3hp::render::nearTriangle */

/* sub_4c3f90: native, nfs3hp::frontend::connectStartConnection */

/* sub_4c4190: native, nfs3hp::frontend::netStatusPacket */

/* sub_4c41d0: native, nfs3hp::frontend::connectListModems */

/* sub_4c4230: native, nfs3hp::frontend::connectSetUpLink */

/* sub_4c4310: native, nfs3hp::frontend::connectShakeHands */

/* sub_4c4480: native, nfs3hp::dead::stub_0x4c4480 */

/* sub_4c4490: native, nfs3hp::graphics::captureScreen */

/* sub_4c45d0: native, nfs3hp::frontend::netClearInputCounts */

/* sub_4c45f0: native, nfs3hp::frontend::simRaceOpen */

/* sub_4c4740: native, nfs3hp::frontend::simRaceRestart */

/* sub_4c4820: native, nfs3hp::frontend::raceEndSaveConfig */

/* sub_4c4830: native, nfs3hp::frontend::raceEndTeardown */

/* sub_4c48d0: native, nfs3hp::frontend::raceFrameRun */

/* sub_4c4ac0: native, nfs3hp::frontend::inpaRaceDebugKeys */

/* sub_4c4be0: native, nfs3hp::frontend::inpaPauseMenuStep */

/* sub_4c4d80: native, nfs3hp::frontend::raceEndStop */

/* sub_4c4de0: native, nfs3hp::frontend::raceEndAsked */

/* sub_4c4e40: native, nfs3hp::frontend::inpaRunRaceLoop */

/* sub_4c5520: native, nfs3hp::frontend::inpaNetworkLockMake */

/* sub_4c5540: native, nfs3hp::frontend::inpaHoldClear */

/* sub_4c5560: native, nfs3hp::frontend::inpaNetworkLockFree */

/* sub_4c55a0: native, nfs3hp::frontend::inpaNetworkHold */

/* sub_4c5720: native, nfs3hp::frontend::netRaceQuitVote */

/* sub_4c5b00: native, nfs3hp::frontend::netQuitPacket */

/* sub_4c5c00: native, nfs3hp::graphics::showWaitMessage */

/* sub_4c5e80: native, nfs3hp::frontend::netRaceSettings */

/* sub_4c6080: native, nfs3hp::controls::appendToPacket */

/* sub_4c60c0: native, nfs3hp::controls::sendOwnInput */

/* sub_4c61e0: native, nfs3hp::controls::receiveInput */

/* sub_4c62a0: native, nfs3hp::frontend::inpaRaceSetUp */

/* sub_4c63c0: native, nfs3hp::frontend::inpaInputLocksMake */

/* sub_4c63f0: native, nfs3hp::frontend::inpaInputReset */

/* sub_4c64e0: native, nfs3hp::frontend::inpaInputLocksFree */

/* sub_4c6530: native, nfs3hp::controls::queueInput */

/* sub_4c66a0: native, nfs3hp::controls::queueRemoteInput */

/* sub_4c67a0: native, nfs3hp::controls::takeQueuedFrame */

/* sub_4c6840: native, nfs3hp::controls::takenControls */

/* sub_4c6870: native, nfs3hp::controls::repeatMissingInputs */

/* sub_4c6970: native, nfs3hp::controls::syncFrame */

/* sub_4c69a0: native, nfs3hp::controls::framesTaken */

/* sub_4c69b0: native, nfs3hp::controls::framesQueued */

/* sub_4c69c0: native, nfs3hp::controls::queueLead */

/* sub_4c6a10: native, nfs3hp::controls::noteLag */

/* sub_4c6a40: native, nfs3hp::controls::setQueueDelays */

/* sub_4c6ae0: native, nfs3hp::controls::queueFull */

/* sub_4c6c70: native, nfs3hp::frontend::effectsSkidSegmentPut */

/* sub_4c6cb0: native, nfs3hp::frontend::rendSkidAddSegment */

/* sub_4c6f00: native, nfs3hp::frontend::rendSkidMarksDraw */

/* sub_4c70f0: native, nfs3hp::frontend::effectsSkidStripsTake */

/* sub_4c7130: native, nfs3hp::frontend::effectsSkidStripsReset */

/* sub_4c7150: native, nfs3hp::frontend::effectsSkidStripsFree */

/* sub_4c7180: native, nfs3hp::frontend::rendPolygonDepths */

/* sub_4c7260: native, nfs3hp::frontend::rendPolygonsIntoBuckets */

/* sub_4c7320: native, nfs3hp::vertices::gatherSorted */

/* sub_4c7380: native, nfs3hp::frontend::rendPolygonsSorted */

/* sub_4c73e0: native, nfs3hp::frontend::rendClearDepthBuckets */

/* sub_4c7400: native, nfs3hp::frontend::rendDepthBucketAppend */

/* sub_4c7480: native, nfs3hp::frontend::rendDepthBucketsAdd */

/* sub_4c7580: native, nfs3hp::frontend::rendDepthBucketsGather */

/* sub_4c75f0: native, nfs3hp::frontend::rendClearFarBuckets */

/* sub_4c7610: native, nfs3hp::vertices::sortedBuckets */

/* sub_4c76e0: native, nfs3hp::frontend::rendFarBucketsGather */

/* sub_4c7750: native, nfs3hp::frontend::rendClearBuckets */

/* sub_4c7790: native, nfs3hp::vertices::depthBuckets */

/* sub_4c7940: native, nfs3hp::frontend::rendDrawListAdd */

/* sub_4c79a0: native, nfs3hp::vertices::gatherBuckets */

}
