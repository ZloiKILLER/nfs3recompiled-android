#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_52d010: native, nfs3hp::snd::dead_0x52d010 */

/* sub_52d110: native, nfs3hp::snd::dead_0x52d110 */

/* sub_52d160: native, nfs3hp::port::packetPlayerOf */

/* sub_52d1a0: native, nfs3hp::mix::xaMono */

/* sub_52d250: native, nfs3hp::mix::xaStereo */

/* sub_52d370: native, nfs3hp::port::codedBits */

/* sub_52d3cc: native, nfs3hp::port::codedSkip */

/* sub_52d428: native, nfs3hp::port::codedPulses */

/* sub_52d5cc: native, nfs3hp::port::codedInterpolate */

/* sub_52d618: native, nfs3hp::port::codedFilter */

/* sub_52d6d8: native, nfs3hp::port::codedSynthesis */

/* sub_52de94: native, nfs3hp::port::codedStart */

/* sub_52df80: native, nfs3hp::port::codedFrame */

/* sub_52e310: native, nfs3hp::watcom::mbccpy */

/* sub_52e350: native, nfs3hp::watcom::mbsupr */

/* sub_52e3a0: native, nfs3hp::watcom::mbccmp */

/* sub_52e3f8: native, nfs3hp::frontend::watcZeroJump */

/* sub_52e400: native, nfs3hp::watcom::get_std_stream */

/* sub_52e420: native, nfs3hp::frontend::winaTextStart */

/* sub_52e440: native, nfs3hp::frontend::textBackgroundBox */

/* sub_52e4e0: native, nfs3hp::frontend::watcTextAt */

/* sub_52e500: native, nfs3hp::frontend::watcStrtol */

/* sub_52e654: native, nfs3hp::dead::stub_0x52e654 */

/* sub_52e660: native, nfs3hp::frontend::watcStrtolSigned */

/* sub_52e670: native, nfs3hp::frontend::watcDigitValue */

/* sub_52e6d0: native, nfs3hp::frontend::watcPutenv */

/* sub_52e88c: native, nfs3hp::frontend::watcEnvironSet */

/* sub_52ea3c: native, nfs3hp::frontend::watcEnvironFind */

/* sub_52eb40: native, nfs3hp::dead::stub_0x52eb40 */

/* sub_52eb70: native, nfs3hp::frontend::watcThreadStart */

/* sub_52ec04: native, nfs3hp::frontend::watcBeginThread */

/* sub_52ecf0: native, nfs3hp::frontend::watcEndThread */

/* sub_52ed20: native, nfs3hp::frontend::watcMathErrorHook */

/* sub_52ed30: native, nfs3hp::watcom::mbsnextc */

/* sub_52ed70: native, nfs3hp::watcom::mbctoupper */

/* sub_52ede0: native, nfs3hp::watcom::mbvtop */

/* sub_52edfa: native, nfs3hp::frontend::watcReturnZero52edfa */

/* sub_52ee00: native, nfs3hp::frontend::watcSetClip */

/* sub_52ee60: native, nfs3hp::frontend::watcMbschr */

/* sub_52eec0: native, nfs3hp::watcom::mbcupper932 */

/* sub_52eee0: native, nfs3hp::watcom::mbclower */

/* sub_52ef30: native, nfs3hp::watcom::mbclegal */

/* sub_52ef70: native, nfs3hp::watcom::mbbtrail */

/* sub_52f318: native, nfs3hp::frontend::watcTapiFind */

/* sub_52f370: native, nfs3hp::dead::stub_0x52f370 */

/* sub_52f3bb: native, nfs3hp::dead::stub_0x52f3bb */

/* sub_52f406: native, nfs3hp::dead::stub_0x52f406 */

/* sub_52f451: native, nfs3hp::dead::stub_0x52f451 */

/* sub_52f49c: native, nfs3hp::dead::stub_0x52f49c */

/* sub_52f4e3: native, nfs3hp::dead::stub_0x52f4e3 */

/* sub_52f535: native, nfs3hp::frontend::watcLineAnswer */

/* sub_52f580: native, nfs3hp::dead::stub_0x52f580 */

/* sub_52f5cb: native, nfs3hp::dead::stub_0x52f5cb */

/* sub_52f616: native, nfs3hp::dead::stub_0x52f616 */

/* sub_52f661: native, nfs3hp::frontend::watcLineClose */

/* sub_52f6a4: native, nfs3hp::dead::stub_0x52f6a4 */

/* sub_52f6f3: native, nfs3hp::dead::stub_0x52f6f3 */

/* sub_52f742: native, nfs3hp::dead::stub_0x52f742 */

/* sub_52f78d: native, nfs3hp::dead::stub_0x52f78d */

/* sub_52f7d8: native, nfs3hp::dead::stub_0x52f7d8 */

/* sub_52f823: native, nfs3hp::dead::stub_0x52f823 */

/* sub_52f878: native, nfs3hp::dead::stub_0x52f878 */

/* sub_52f8cd: native, nfs3hp::dead::stub_0x52f8cd */

/* sub_52f922: native, nfs3hp::dead::stub_0x52f922 */

/* sub_52f969: native, nfs3hp::frontend::watcLineDeallocateCall */

/* sub_52f9ac: native, nfs3hp::dead::stub_0x52f9ac */

/* sub_52f9fe: native, nfs3hp::dead::stub_0x52f9fe */

/* sub_52fa4d: native, nfs3hp::dead::stub_0x52fa4d */

/* sub_52fa98: native, nfs3hp::dead::stub_0x52fa98 */

/* sub_52fae3: native, nfs3hp::dead::stub_0x52fae3 */

/* sub_52fb2e: native, nfs3hp::frontend::watcLineDrop */

/* sub_52fb79: native, nfs3hp::dead::stub_0x52fb79 */

/* sub_52fbd1: native, nfs3hp::dead::stub_0x52fbd1 */

/* sub_52fc29: native, nfs3hp::dead::stub_0x52fc29 */

/* sub_52fc81: native, nfs3hp::dead::stub_0x52fc81 */

/* sub_52fcd9: native, nfs3hp::dead::stub_0x52fcd9 */

/* sub_52fd31: native, nfs3hp::dead::stub_0x52fd31 */

/* sub_52fd89: native, nfs3hp::dead::stub_0x52fd89 */

/* sub_52fdd8: native, nfs3hp::dead::stub_0x52fdd8 */

/* sub_52fe27: native, nfs3hp::dead::stub_0x52fe27 */

/* sub_52fe76: native, nfs3hp::dead::stub_0x52fe76 */

/* sub_52fec8: native, nfs3hp::dead::stub_0x52fec8 */

/* sub_52ff1d: native, nfs3hp::dead::stub_0x52ff1d */

/* sub_52ff72: native, nfs3hp::dead::stub_0x52ff72 */

/* sub_52ffc7: native, nfs3hp::pieces::stub_0x52ffc7 */

/* sub_530019: native, nfs3hp::pieces::stub_0x530019 */

/* sub_53006b: native, nfs3hp::pieces::stub_0x53006b */

/* sub_5300bd: native, nfs3hp::dead::stub_0x5300bd */

/* sub_530108: native, nfs3hp::dead::stub_0x530108 */

/* sub_530153: native, nfs3hp::dead::stub_0x530153 */

/* sub_53019e: native, nfs3hp::dead::stub_0x53019e */

/* sub_5301e9: native, nfs3hp::dead::stub_0x5301e9 */

/* sub_530234: native, nfs3hp::dead::stub_0x530234 */

/* sub_530286: native, nfs3hp::dead::stub_0x530286 */

/* sub_5302d8: native, nfs3hp::dead::stub_0x5302d8 */

/* sub_530323: native, nfs3hp::dead::stub_0x530323 */

/* sub_53036e: native, nfs3hp::dead::stub_0x53036e */

/* sub_5303b9: native, nfs3hp::dead::stub_0x5303b9 */

/* sub_530404: native, nfs3hp::dead::stub_0x530404 */

/* sub_530459: native, nfs3hp::dead::stub_0x530459 */

}
