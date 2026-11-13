#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4fcb00: native, nfs3hp::dead::stub_0x4fcb00 */

/* sub_4fcb50: native, nfs3hp::dead::stub_0x4fcb50 */

/* sub_4fcb70: native, nfs3hp::dead::stub_0x4fcb70 */

/* sub_4fcba0: native, nfs3hp::dead::stub_0x4fcba0 */

/* sub_4fcbc0: native, nfs3hp::dead::stub_0x4fcbc0 */

/* sub_4fcc90: native, nfs3hp::dead::stub_0x4fcc90 */

/* sub_4fcce0: native, nfs3hp::dead::stub_0x4fcce0 */

/* sub_4fce20: native, nfs3hp::dead::stub_0x4fce20 */

/* sub_4fce30: native, nfs3hp::dead::stub_0x4fce30 */

/* sub_4fce40: native, nfs3hp::dead::stub_0x4fce40 */

/* sub_4fce50: native, nfs3hp::frontend::winaFileSlotOfName */

/* sub_4fce90: native, nfs3hp::dead::stub_0x4fce90 */

/* sub_4fcea0: native, nfs3hp::dead::stub_0x4fcea0 */

/* sub_4fceb0: native, nfs3hp::dead::stub_0x4fceb0 */

/* sub_4fcf40: native, nfs3hp::dead::stub_0x4fcf40 */

/* sub_4fcfe0: native, nfs3hp::dead::stub_0x4fcfe0 */

/* sub_4fd020: native, nfs3hp::dead::stub_0x4fd020 */

/* sub_4fd040: native, nfs3hp::dead::stub_0x4fd040 */

/* sub_4fd060: native, nfs3hp::frontend::winaFileCloseWait */

/* sub_4fd090: native, nfs3hp::dead::stub_0x4fd090 */

/* sub_4fd0c0: native, nfs3hp::dead::stub_0x4fd0c0 */

/* sub_4fd0f0: native, nfs3hp::dead::stub_0x4fd0f0 */

/* sub_4fd130: native, nfs3hp::dead::stub_0x4fd130 */

/* sub_4fd160: native, nfs3hp::dead::stub_0x4fd160 */

/* sub_4fd190: native, nfs3hp::dead::stub_0x4fd190 */

/* sub_4fd1c0: native, nfs3hp::frontend::watbShapeClut */

/* sub_4fd280: native, nfs3hp::frontend::projectionCentre */

/* sub_4fd2b0: native, nfs3hp::frontend::projectionView */

/* sub_4fd3f0: native, nfs3hp::frontend::projectionViewDegrees */

/* sub_4fd420: native, nfs3hp::dead::stub_0x4fd420 */

/* sub_4fd490: native, nfs3hp::frontend::winaFillHalved */

/* sub_4fd4b0: native, nfs3hp::frontend::winaFillSame */

/* sub_4fd4c0: native, nfs3hp::frontend::winaFillDoubled */

/* sub_4fd4e0: native, nfs3hp::frontend::winaFillWords */

/* sub_4fd500: native, nfs3hp::frontend::winaFillTriples */

/* sub_4fd520: native, nfs3hp::frontend::winaFillQuadrupled */

/* sub_4fd540: native, nfs3hp::frontend::watbDrawFilledRect */

/* sub_4fd61c: native, nfs3hp::dead::stub_0x4fd61c */

/* sub_4fd6ef: native, nfs3hp::dead::stub_0x4fd6ef */

/* sub_4fd7b6: native, nfs3hp::dead::stub_0x4fd7b6 */

/* sub_4fd892: native, nfs3hp::dead::stub_0x4fd892 */

/* sub_4fd8ee: native, nfs3hp::dead::stub_0x4fd8ee */

/* sub_4fd98c: native, nfs3hp::dead::stub_0x4fd98c */

/* sub_4fda20: native, nfs3hp::dead::stub_0x4fda20 */

/* sub_4fdc90: native, nfs3hp::dead::stub_0x4fdc90 */

/* sub_4fe1d0: native, nfs3hp::watcom::prtf */

/* sub_4fe578: native, nfs3hp::watcom::prtfSpec */

/* sub_4fe6cc: native, nfs3hp::watcom::prtfFlags */

/* sub_4fe734: native, nfs3hp::watcom::prtfLength */

/* sub_4fe758: native, nfs3hp::watcom::prtfWideLength */

/* sub_4fe7c4: native, nfs3hp::watcom::prtfHex */

/* sub_4fe81e: native, nfs3hp::watcom::prtfHexEnd */

/* sub_4fe828: native, nfs3hp::watcom::prtfFixed */

/* sub_4fe924: native, nfs3hp::watcom::prtfFloat */

/* sub_4fe92c: native, nfs3hp::watcom::prtfZeros */

/* sub_4fe970: native, nfs3hp::watcom::prtfWideOut */

/* sub_4fe9d4: native, nfs3hp::watcom::prtfConvert */

/* sub_4fefdc: native, nfs3hp::watcom::prtfUpper */

/* sub_4feffc: native, nfs3hp::frontend::watbReturnZero */

/* sub_4ff000: native, nfs3hp::watcom::file_putc */

/* sub_4ff014: native, nfs3hp::watcom::vfprintf */

/* sub_4ff0e0: native, nfs3hp::dead::stub_0x4ff0e0 */

/* sub_4ff0ec: native, nfs3hp::watcom::GetThreadPtr */

/* sub_4ff0f4: native, nfs3hp::watcom::NullAccess */

/* sub_4ff0f8: native, nfs3hp::frontend::watbAllocHandleJump */

/* sub_4ff100: native, nfs3hp::frontend::watbFreeHandleJump */

/* sub_4ff108: native, nfs3hp::watcom::NTInit */

/* sub_4ff338: native, nfs3hp::watcom::NTMain */

/* sub_4ff38c: native, nfs3hp::watcom::exit_ */

/* sub_4ff3d0: native, nfs3hp::watcom::freeEnvironment */

/* sub_4ff410: native, nfs3hp::watcom::callRoutine */

/* sub_4ff41c: native, nfs3hp::watcom::InitRtns */

/* sub_4ff46c: native, nfs3hp::watcom::FiniRtns */

/* sub_4ff4c0: native, nfs3hp::watcom::STOSB */

/* sub_4ff4f2: native, nfs3hp::pieces::stub_0x4ff4f2 */

/* sub_4ff4f7: native, nfs3hp::watcom::STOSD */

/* sub_4ff570: native, nfs3hp::frontend::watbInitFloatFormatters */

/* sub_4ff590: native, nfs3hp::pieces::stub_0x4ff590 */

/* sub_4ff598: native, nfs3hp::pieces::stub_0x4ff598 */

/* sub_4ff59c: native, nfs3hp::frontend::watbInitChipBugHooks */

/* sub_4ff5d0: native, nfs3hp::dead::stub_0x4ff5d0 */

/* sub_4ff5dc: native, nfs3hp::frontend::watbInit8087 */

/* sub_4ff620: native, nfs3hp::frontend::watbNothing4ff620 */

/* sub_4ff624: native, nfs3hp::dead::stub_0x4ff624 */

/* sub_4ff660: native, nfs3hp::port::setTopupFunction */

/* sub_4ff670: native, nfs3hp::port::topUpStream */

/* sub_4ff700: native, nfs3hp::port::waitTopupMain */

/* sub_4ff76e: native, nfs3hp::port::waitTopupEvent */

/* sub_4ff780: native, nfs3hp::port::topupDone */

/* sub_4ff7b0: native, nfs3hp::port::toppedUp */

/* sub_4ff7c0: native, nfs3hp::port::topUpBeforeLoad */

/* sub_4ff7f0: native, nfs3hp::port::waitTopupStream */

/* sub_4ff7e0: native, nfs3hp::port::topupReturn */

/* sub_4ff80f: native, nfs3hp::port::waitTopupDone */

/* sub_4ff820: native, nfs3hp::port::loadBlockSize */

/* sub_4ff830: native, nfs3hp::port::loadFlags */

/* sub_4ff840: native, nfs3hp::port::voicePicked */

/* sub_4ff86c: native, nfs3hp::port::iSNDallocchan */

/* sub_4ffb64: native, nfs3hp::port::iSNDfreechan */

/* sub_4ffc9c: native, nfs3hp::port::voiceOf */

}
