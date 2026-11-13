#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_52a144: native, nfs3hp::port::xaStereoIntroSetup */

/* sub_52a1a0: native, nfs3hp::port::xaStereoStreamDecode */

/* sub_52a354: native, nfs3hp::port::xaStereoStreamSetup */

/* sub_52a3a0: native, nfs3hp::port::xaStereoPacketDecode */

/* sub_52a5b4: native, nfs3hp::port::xaStereoFileSetup */

/* sub_52a610: native, nfs3hp::port::framesPlayedCoded */

/* sub_52a618: native, nfs3hp::port::codedToWords */

/* sub_52a668: native, nfs3hp::port::codedDecode */

/* sub_52a790: native, nfs3hp::port::codedSetup */

/* sub_52a7e0: native, nfs3hp::port::codedStreamDecode */

/* sub_52a970: native, nfs3hp::port::codedStreamSetup */

/* sub_52a9c0: native, nfs3hp::port::codedPacketDecode */

/* sub_52ab70: native, nfs3hp::port::codedPacketSetup */

/* sub_52abd0: native, nfs3hp::snd::dead_0x52abd0 */

/* sub_52abd8: native, nfs3hp::snd::dead_0x52abd8 */

/* sub_52abf8: native, nfs3hp::snd::dead_0x52abf8 */

/* sub_52ad28: native, nfs3hp::snd::dead_0x52ad28 */

/* sub_52ad70: native, nfs3hp::snd::dead_0x52ad70 */

/* sub_52af30: native, nfs3hp::snd::dead_0x52af30 */

/* sub_52af80: native, nfs3hp::snd::dead_0x52af80 */

/* sub_52b160: native, nfs3hp::snd::dead_0x52b160 */

/* sub_52b1c0: native, nfs3hp::port::panLaw */

/* sub_52b230: native, nfs3hp::port::readBigEndian */

/* sub_52b290: native, nfs3hp::watcom::putch */

/* sub_52b300: native, nfs3hp::snd::dead_0x52b300 */

/* sub_52b4d0: native, nfs3hp::snd::dead_0x52b4d0 */

/* sub_52b500: native, nfs3hp::port::setSendLevel */

/* sub_52b523: native, nfs3hp::frontend::watcPowError */

/* sub_52b5b0: native, nfs3hp::watcom::mbclen */

/* sub_52b5e0: native, nfs3hp::watcom::mbcicmp */

/* sub_52b640: native, nfs3hp::watcom::startHook */

/* sub_52b641: native, nfs3hp::dead::stub_0x52b641 */

/* sub_52b647: native, nfs3hp::watcom::printMathError */

/* sub_52b68f: native, nfs3hp::watcom::callMatherr */

/* sub_52b6c0: native, nfs3hp::frontend::watcSetRectClip */

/* sub_52b720: native, nfs3hp::frontend::watcDrawBox */

/* sub_52b7d0: native, nfs3hp::dead::stub_0x52b7d0 */

/* sub_52b860: native, nfs3hp::frontend::watcTextCentred */

/* sub_52b8b0: native, nfs3hp::frontend::watcClipWholeRect */

/* sub_52b8d0: native, nfs3hp::frontend::watcViewerOnWindow */

/* sub_52b8e0: native, nfs3hp::frontend::watcWindowStart */

/* sub_52b8f0: native, nfs3hp::frontend::watcWindowEnd */

/* sub_52b910: native, nfs3hp::frontend::watcWindowSize */

/* sub_52b970: native, nfs3hp::frontend::watcPrintAtCell */

/* sub_52b9d0: native, nfs3hp::frontend::watcNothing52b9d0 */

/* sub_52b9e0: native, nfs3hp::frontend::watcBlitOnBlack */

/* sub_52ba00: native, nfs3hp::frontend::watcSetTextColours */

/* sub_52ba70: native, nfs3hp::frontend::watcDebugColour */

/* sub_52bac0: native, nfs3hp::frontend::watcWaitKey */

/* sub_52baf0: native, nfs3hp::frontend::watcFillCells */

/* sub_52bb20: native, nfs3hp::frontend::watcViewerOnMono */

/* sub_52bb50: native, nfs3hp::frontend::watcMonoSize */

/* sub_52bb80: native, nfs3hp::frontend::watcMonoClearJump */

/* sub_52bb90: native, nfs3hp::frontend::watcShowCentred */

/* sub_52bbf0: native, nfs3hp::frontend::watcMonoAttribute */

/* sub_52bc00: native, nfs3hp::frontend::watcPrintInWindow */

/* sub_52bd00: native, nfs3hp::frontend::watcMonoAttributeOf */

/* sub_52bd20: native, nfs3hp::frontend::watcMonoWaitKey */

/* sub_52bd50: native, nfs3hp::frontend::watcMonoFill */

/* sub_52bdb0: native, nfs3hp::frontend::watcInheritHandles */

/* sub_52bed0: native, nfs3hp::frontend::watcClearEnviron */

/* sub_52bf70: native, nfs3hp::dead::stub_0x52bf70 */

/* sub_52bfe0: native, nfs3hp::dead::stub_0x52bfe0 */

/* sub_52c178: native, nfs3hp::watcom::wputenv */

/* sub_52c300: native, nfs3hp::watcom::wputenvFind */

/* sub_52c410: native, nfs3hp::dead::stub_0x52c410 */

/* sub_52c420: native, nfs3hp::dead::stub_0x52c420 */

/* sub_52c430: native, nfs3hp::dead::stub_0x52c430 */

/* sub_52c580: native, nfs3hp::dead::stub_0x52c580 */

/* sub_52c5d0: native, nfs3hp::dead::stub_0x52c5d0 */

/* sub_52c720: native, nfs3hp::dead::stub_0x52c720 */

/* sub_52c7b0: native, nfs3hp::dead::stub_0x52c7b0 */

/* sub_52c810: native, nfs3hp::frontend::watcReturnZero52c810 */

/* sub_52c814: native, nfs3hp::dead::stub_0x52c814 */

/* sub_52c81c: native, nfs3hp::frontend::watcThreadEndHook */

/* sub_52c824: native, nfs3hp::frontend::watcThreadStartHook */

/* sub_52c82c: native, nfs3hp::dead::stub_0x52c82c */

/* sub_52c870: native, nfs3hp::pieces::stub_0x52c870 */

/* sub_52c878: native, nfs3hp::frontend::watcAbnormalTermination */

/* sub_52c88a: native, nfs3hp::frontend::watcDoubleZero */

/* sub_52c88f: native, nfs3hp::pieces::stub_0x52c88f */

/* sub_52c8a7: native, nfs3hp::frontend::watcDoubleInfinity */

/* sub_52c89b: native, nfs3hp::frontend::watcDoubleInfinityHook */

/* sub_52c8c7: native, nfs3hp::dead::stub_0x52c8c7 */

/* sub_52c8f0: native, nfs3hp::port::pcmPlayed */

/* sub_52c8f4: native, nfs3hp::port::pcmOnceDecode */

/* sub_52c9a0: native, nfs3hp::port::pcmOnceSetup */

/* sub_52c9d0: native, nfs3hp::port::pcmLoopedDecode */

/* sub_52ca48: native, nfs3hp::port::pcmLoopedSetup */

/* sub_52ca80: native, nfs3hp::port::pcmStreamedDecode */

/* sub_52cb74: native, nfs3hp::port::pcmStreamedSetup */

/* sub_52cba0: native, nfs3hp::port::pcmFileDecode */

/* sub_52ccb8: native, nfs3hp::port::pcmFileSetup */

/* sub_52ccf0: native, nfs3hp::snd::dead_0x52ccf0 */

/* sub_52ccf4: native, nfs3hp::snd::dead_0x52ccf4 */

/* sub_52cdd4: native, nfs3hp::snd::dead_0x52cdd4 */

/* sub_52ce20: native, nfs3hp::snd::dead_0x52ce20 */

/* sub_52ceb0: native, nfs3hp::snd::dead_0x52ceb0 */

/* sub_52cef0: native, nfs3hp::snd::dead_0x52cef0 */

/* sub_52cfd8: native, nfs3hp::snd::dead_0x52cfd8 */

}
