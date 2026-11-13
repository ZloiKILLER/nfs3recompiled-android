#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_496550: native, nfs3hp::frontend::rendApproxDistance */

/* sub_4965b0: native, nfs3hp::frontend::simQuickDistance */

/* sub_496690: native, nfs3hp::dead::stub_0x496690 */

/* sub_4967e0: native, nfs3hp::frontend::simPlanarLength */

/* sub_4968a0: native, nfs3hp::vertices::groundHeight */

/* sub_4969c0: native, nfs3hp::frontend::simTrackPointOnPlane */

/* sub_496a50: native, nfs3hp::frontend::simFrameOfTriangle */

/* sub_496b10: native, nfs3hp::dead::stub_0x496b10 */

/* sub_496b60: native, nfs3hp::dead::stub_0x496b60 */

/* sub_496bf0: native, nfs3hp::frontend::rendRotatePlane */

/* sub_496c40: native, nfs3hp::dead::stub_0x496c40 */

/* sub_496cb0: native, nfs3hp::dead::stub_0x496cb0 */

/* sub_496df0: native, nfs3hp::dead::stub_0x496df0 */

/* sub_496f20: native, nfs3hp::frontend::rendWithinFlatDistance */

/* sub_496fc0: native, nfs3hp::dead::stub_0x496fc0 */

/* sub_497010: native, nfs3hp::dead::stub_0x497010 */

/* sub_4970c0: native, nfs3hp::frontend::simFixedNormalise */

/* sub_497240: native, nfs3hp::dead::stub_0x497240 */

/* sub_4972f0: native, nfs3hp::vertices::normalise */

/* sub_497360: native, nfs3hp::dead::stub_0x497360 */

/* sub_497370: native, nfs3hp::dead::stub_0x497370 */

/* sub_497380: native, nfs3hp::dead::stub_0x497380 */

/* sub_4973e0: native, nfs3hp::dead::stub_0x4973e0 */

/* sub_497430: native, nfs3hp::frontend::loopFillRootBytes */

/* sub_497490: native, nfs3hp::dead::stub_0x497490 */

/* sub_497500: native, nfs3hp::frontend::mathQuickNormal */

/* sub_497590: native, nfs3hp::frontend::hudVectorLength */

/* sub_497620: native, nfs3hp::frontend::memViewerRun */

/* sub_497ba0: native, nfs3hp::frontend::memViewerOpen */

/* sub_497cb0: native, nfs3hp::frontend::memViewerClose */

/* sub_497cc0: native, nfs3hp::frontend::memViewerFirstDraw */

/* sub_497d20: native, nfs3hp::frontend::memViewerStatsLine */

/* sub_497fe0: native, nfs3hp::frontend::memViewerListBlocks */

/* sub_498090: native, nfs3hp::frontend::memViewerHeader */

/* sub_498370: native, nfs3hp::frontend::memViewerRows */

/* sub_4989a0: native, nfs3hp::frontend::memViewerStatusLine */

/* sub_498a40: native, nfs3hp::frontend::memViewerDescribeBlock */

/* sub_498b70: native, nfs3hp::frontend::memViewerCountShapes */

/* sub_498bf0: native, nfs3hp::frontend::memViewerMenu */

/* sub_498e10: native, nfs3hp::frontend::memViewerNextClass */

/* sub_498e40: native, nfs3hp::frontend::memViewerFlagText */

/* sub_498f00: native, nfs3hp::frontend::memViewerSortMode */

/* sub_499150: native, nfs3hp::frontend::memViewerCompare */

/* sub_4992f0: native, nfs3hp::dead::stub_0x4992f0 */

/* sub_499300: native, nfs3hp::dead::stub_0x499300 */

/* sub_499330: native, nfs3hp::frontend::memPoolTotals */

/* sub_4993b0: native, nfs3hp::frontend::memPoolFirstBlock */

/* sub_4993d0: native, nfs3hp::frontend::memPoolCountBlocks */

/* sub_499400: native, nfs3hp::frontend::memPoolNextBlock */

/* sub_499460: native, nfs3hp::frontend::viewerBlockSize */

/* sub_499480: native, nfs3hp::frontend::memBlockData */

/* sub_499490: native, nfs3hp::frontend::viewerBlockFlags */

/* sub_4994a0: native, nfs3hp::frontend::memBlockKind */

/* sub_499580: native, nfs3hp::frontend::memBlockIsFree */

/* sub_4995b0: native, nfs3hp::frontend::memViewerViewShapes */

/* sub_499b30: native, nfs3hp::frontend::memViewerViewWindow */

/* sub_499d30: native, nfs3hp::frontend::memViewerViewBitmap */

/* sub_499e90: native, nfs3hp::frontend::memShapeBits */

/* sub_499ea0: native, nfs3hp::frontend::memViewerHexDump */

/* sub_499f70: native, nfs3hp::frontend::memViewerHexPage */

/* sub_49a120: native, nfs3hp::frontend::memViewerWriteMap */

/* sub_49a400: native, nfs3hp::frontend::memViewerFreeFileName */

/* sub_49a490: native, nfs3hp::frontend::netMakeLocks */

/* sub_49a4d0: native, nfs3hp::frontend::freeNetworkLocks */

/* sub_49a520: native, nfs3hp::frontend::netLobbyChanged */

/* sub_49a8e0: native, nfs3hp::frontend::netSendMyCar */

/* sub_49aa80: native, nfs3hp::frontend::netSendSettings */

/* sub_49ab60: native, nfs3hp::frontend::allPlayersReady */

/* sub_49abd0: native, nfs3hp::frontend::netAllReady */

/* sub_49acd0: native, nfs3hp::frontend::netPlayersToRace */

/* sub_49ae30: native, nfs3hp::frontend::lobbyDrawPlayers */

/* sub_49b2b0: native, nfs3hp::frontend::netSettingsPacket */

/* sub_49b310: native, nfs3hp::frontend::netPlayerPacket */

/* sub_49b3e0: native, nfs3hp::frontend::netCarFileToHost */

/* sub_49b470: native, nfs3hp::frontend::netCarFileToClient */

/* sub_49b500: native, nfs3hp::frontend::netCarChunkToHost */

/* sub_49b520: native, nfs3hp::frontend::netCarChunkToClient */

/* sub_49b540: native, nfs3hp::load::beginCarpReceive */

/* sub_49b5f0: native, nfs3hp::load::receiveCarpChunk */

/* sub_49b800: native, nfs3hp::load::beginCarpSend */

/* sub_49b9b0: native, nfs3hp::frontend::netSendCarFile */

/* sub_49bab0: native, nfs3hp::frontend::netExchangeCars */

/* sub_49bc80: native, nfs3hp::frontend::hudQueueStart */

/* sub_49bcd0: native, nfs3hp::port::text::takeQuad */

/* sub_49bcf0: native, nfs3hp::dead::stub_0x49bcf0 */

/* sub_49bd10: native, nfs3hp::frontend::hudQueueTakeCorners */

/* sub_49bd30: native, nfs3hp::port::text::queueQuad */

/* sub_49bdc0: native, nfs3hp::port::text::unqueueQuad */

/* sub_49be40: native, nfs3hp::port::text::flushQuads */

/* sub_49bef0: native, nfs3hp::frontend::hudSurfaceReset */

/* sub_49bf40: native, nfs3hp::frontend::hudSurfaceTakeVertices */

/* sub_49bf60: native, nfs3hp::graphics::flushLineMesh */

/* sub_49bfb0: native, nfs3hp::frontend::simOrFlags */

/* sub_49bfc0: native, nfs3hp::frontend::setPointer */

/* sub_49c000: native, nfs3hp::port::text::mouseMenuKey */

/* sub_49c080: native, nfs3hp::frontend::mouseKey */

/* sub_49c0e0: native, nfs3hp::frontend::connectSetPointer */

/* sub_49c140: native, nfs3hp::frontend::menuPointerLoadArt */

/* sub_49c1b0: native, nfs3hp::dead::stub_0x49c1b0 */

/* sub_49c1c0: native, nfs3hp::frontend::menuPointerListLine */

}
