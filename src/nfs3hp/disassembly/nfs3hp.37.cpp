#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4f0880: native, nfs3hp::frontend::mixA4Bit */

/* sub_4f0890: native, nfs3hp::frontend::mixA16Bit */

/* sub_4f08a0: native, nfs3hp::frontend::mixA24Bit */

/* sub_4f08c0: native, nfs3hp::frontend::mixA32Bit */

/* sub_4f08d0: native, nfs3hp::frontend::mixB4Bit */

/* sub_4f08e0: native, nfs3hp::frontend::mixB16Bit */

/* sub_4f08f0: native, nfs3hp::frontend::mixB24Bit */

/* sub_4f0910: native, nfs3hp::frontend::mixB32Bit */

/* sub_4f0920: native, nfs3hp::frontend::mixC4Bit */

/* sub_4f0930: native, nfs3hp::frontend::mixC16Bit */

/* sub_4f0940: native, nfs3hp::frontend::mixC24Bit */

/* sub_4f0960: native, nfs3hp::frontend::mixC32Bit */

/* sub_4f0970: native, nfs3hp::frontend::mixD4Bit */

/* sub_4f0980: native, nfs3hp::frontend::mixD16Bit */

/* sub_4f0990: native, nfs3hp::frontend::mixD24Bit */

/* sub_4f09b0: native, nfs3hp::frontend::mixD32Bit */

/* sub_4f09c0: native, nfs3hp::frontend::blitDispatch */

/* sub_4f0a10: native, nfs3hp::frontend::blitShapeAtPosition */

/* sub_4f0a30: native, nfs3hp::dead::stub_0x4f0a30 */

/* sub_4f0a50: native, nfs3hp::dead::stub_0x4f0a50 */

/* sub_4f0a70: native, nfs3hp::dead::stub_0x4f0a70 */

/* sub_4f0a90: native, nfs3hp::frontend::surfaceFillAndDraw */

/* sub_4f0ae0: native, nfs3hp::watcom::tolower */

/* sub_4f0af0: native, nfs3hp::frontend::textMeasureGlyph */

/* sub_4f0b60: native, nfs3hp::frontend::textMeasureBox */

/* sub_4f0c90: native, nfs3hp::frontend::textMeasure */

/* sub_4f0cb0: native, nfs3hp::frontend::asyncRequestHasId */

/* sub_4f0cc0: native, nfs3hp::frontend::asyncNoRequest */

/* sub_4f0cd0: native, nfs3hp::frontend::asyncCancelCurrent */

/* sub_4f0d20: native, nfs3hp::frontend::asyncRunCallbacks */

/* sub_4f0da0: native, nfs3hp::frontend::asyncInit */

/* sub_4f0f40: native, nfs3hp::frontend::asyncShutdown */

/* sub_4f1110: native, nfs3hp::dead::stub_0x4f1110 */

/* sub_4f1230: native, nfs3hp::dead::stub_0x4f1230 */

/* sub_4f1240: native, nfs3hp::dead::stub_0x4f1240 */

/* sub_4f1360: native, nfs3hp::dead::stub_0x4f1360 */

/* sub_4f1370: native, nfs3hp::frontend::asyncSetFile */

/* sub_4f14b0: native, nfs3hp::dead::stub_0x4f14b0 */

/* sub_4f14f0: native, nfs3hp::dead::stub_0x4f14f0 */

/* sub_4f1500: native, nfs3hp::frontend::asyncReadAt */

/* sub_4f15d0: native, nfs3hp::frontend::asyncRead */

/* sub_4f15e0: native, nfs3hp::dead::stub_0x4f15e0 */

/* sub_4f15f0: native, nfs3hp::dead::stub_0x4f15f0 */

/* sub_4f1610: native, nfs3hp::dead::stub_0x4f1610 */

/* sub_4f1710: native, nfs3hp::dead::stub_0x4f1710 */

/* sub_4f1790: native, nfs3hp::frontend::asyncReadStatus */

/* sub_4f1820: native, nfs3hp::frontend::asyncThreadMain */

/* sub_4f1c30: native, nfs3hp::watcom::floor */

/* sub_4f1c70: native, nfs3hp::removed::processorChecked */

/* sub_4f1ea0: native, nfs3hp::dead::stub_0x4f1ea0 */

/* sub_4f1ef0: native, nfs3hp::removed::processorSpeed */

/* sub_4f20b0: native, nfs3hp::dead::stub_0x4f20b0 */

/* sub_4f2170: native, nfs3hp::frontend::processorCountLoops */

/* sub_4f21c0: native, nfs3hp::dead::stub_0x4f21c0 */

/* sub_4f22b0: native, nfs3hp::watcom::time */

/* sub_4f22e0: native, nfs3hp::frontend::fileLoadInto */

/* sub_4f2360: native, nfs3hp::dead::stub_0x4f2360 */

/* sub_4f2380: native, nfs3hp::load::fixedSine */

/* sub_4f23f0: native, nfs3hp::load::fixedCosine */

/* sub_4f2460: native, nfs3hp::frontend::winaShapeHotspots */

/* sub_4f24e0: native, nfs3hp::frontend::shapeFindHotspot */

/* sub_4f2560: native, nfs3hp::ea::unpackQuietly */

/* sub_4f2620: native, nfs3hp::ea::unpack */

/* sub_4f26a0: native, nfs3hp::ea::unpackSizeQuietly */

/* sub_4f2710: native, nfs3hp::ea::unpacksize */

/* sub_4f2790: native, nfs3hp::frontend::lowTimerTicks */

/* sub_4f27a0: native, nfs3hp::frontend::lowTimerThread */

/* sub_4f2830: native, nfs3hp::ea::stopLowTimer */

/* sub_4f2890: native, nfs3hp::frontend::lowTimerStart */

/* sub_4f2930: native, nfs3hp::frontend::lowTimerAdd */

/* sub_4f29b0: native, nfs3hp::ea::removeLowTimer */

/* sub_4f2a60: native, nfs3hp::frontend::lowTimerSignal */

/* sub_4f2a78: native, nfs3hp::frontend::atexitJump */

/* sub_4f2a7d: native, nfs3hp::frontend::exitGameNow */

/* sub_4f2ad0: native, nfs3hp::frontend::memCopyAligned */

/* sub_4f2bf0: native, nfs3hp::frontend::memMoveLibrary */

/* sub_4f2c40: native, nfs3hp::frontend::memMoveHalfBytes */

/* sub_4f2c50: native, nfs3hp::frontend::memMoveBytes */

/* sub_4f2c60: native, nfs3hp::frontend::memMoveWords */

/* sub_4f2c70: native, nfs3hp::frontend::memMoveTriples */

/* sub_4f2c90: native, nfs3hp::frontend::memMoveLongs */

/* sub_4f2cb0: native, nfs3hp::frontend::shapeMoveWithSpot */

/* sub_4f2d00: native, nfs3hp::dead::stub_0x4f2d00 */

/* sub_4f2d20: native, nfs3hp::dead::stub_0x4f2d20 */

/* sub_4f2d40: native, nfs3hp::dead::stub_0x4f2d40 */

/* sub_4f2d60: native, nfs3hp::dead::stub_0x4f2d60 */

/* sub_4f2d78: native, nfs3hp::watcom::ceil */

/* sub_4f2da0: native, nfs3hp::dead::stub_0x4f2da0 */

/* sub_4f2e20: native, nfs3hp::ea::loadpackadr */

/* sub_4f2f30: native, nfs3hp::dead::stub_0x4f2f30 */

/* sub_4f2f50: native, nfs3hp::port::SNDSTRM_requeststatus */

/* sub_4f30d0: native, nfs3hp::dead::stub_0x4f30d0 */

/* sub_4f3350: native, nfs3hp::dead::stub_0x4f3350 */

/* sub_4f33b0: native, nfs3hp::dead::stub_0x4f33b0 */

/* sub_4f33e0: native, nfs3hp::dead::stub_0x4f33e0 */

/* sub_4f3420: native, nfs3hp::dead::stub_0x4f3420 */

/* sub_4f34f0: native, nfs3hp::dead::stub_0x4f34f0 */

/* sub_4f3560: native, nfs3hp::dead::stub_0x4f3560 */

/* sub_4f3600: native, nfs3hp::dead::stub_0x4f3600 */

/* sub_4f3a10: native, nfs3hp::port::streamMemory */

}
