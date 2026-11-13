#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_503f90: native, nfs3hp::dead::stub_0x503f90 */

/* sub_5040b8: native, nfs3hp::frontend::rendPixels565To1555 */

/* sub_504133: native, nfs3hp::frontend::rendPixels1555To565 */

/* sub_5041ae: native, nfs3hp::frontend::rendPixelsPal8To32 */

/* sub_5041d0: native, nfs3hp::frontend::rendPixelsPal8To16 */

/* sub_50425d: native, nfs3hp::frontend::rendPixelsRgb24ToArgb */

/* sub_5042c4: native, nfs3hp::frontend::rendPixelsSwapRedBlue */

/* sub_504335: native, nfs3hp::frontend::rendPixelsPal4To16 */

/* sub_5043f0: native, nfs3hp::graphics::dead_0x5043f0 */

/* sub_5049c0: native, nfs3hp::graphics::dead_0x5049c0 */

/* sub_5049f0: native, nfs3hp::graphics::dead_0x5049f0 */

/* sub_504a30: native, nfs3hp::graphics::dead_0x504a30 */

/* sub_504af0: native, nfs3hp::graphics::dead_0x504af0 */

/* sub_504b70: native, nfs3hp::graphics::dead_0x504b70 */

/* sub_504bf0: native, nfs3hp::graphics::dead_0x504bf0 */

/* sub_504c00: native, nfs3hp::graphics::dead_0x504c00 */

/* sub_504c20: native, nfs3hp::graphics::dead_0x504c20 */

/* sub_504c40: native, nfs3hp::graphics::dead_0x504c40 */

/* sub_504c80: native, nfs3hp::graphics::dead_0x504c80 */

/* sub_504e00: native, nfs3hp::graphics::dead_0x504e00 */

/* sub_504f90: native, nfs3hp::graphics::dead_0x504f90 */

/* sub_5050b0: native, nfs3hp::graphics::dead_0x5050b0 */

/* sub_5050e0: native, nfs3hp::graphics::dead_0x5050e0 */

/* sub_5050f0: native, nfs3hp::graphics::dead_0x5050f0 */

/* sub_5051c0: native, nfs3hp::graphics::dead_0x5051c0 */

/* sub_505250: native, nfs3hp::graphics::dead_0x505250 */

/* sub_505280: native, nfs3hp::graphics::dead_0x505280 */

/* sub_5052a0: native, nfs3hp::graphics::dead_0x5052a0 */

/* sub_505320: native, nfs3hp::graphics::dead_0x505320 */

/* sub_505500: native, nfs3hp::graphics::dead_0x505500 */

/* sub_505570: native, nfs3hp::graphics::dead_0x505570 */

/* sub_505650: native, nfs3hp::graphics::dead_0x505650 */

/* sub_5057d0: native, nfs3hp::graphics::dead_0x5057d0 */

/* sub_505830: native, nfs3hp::graphics::dead_0x505830 */

/* sub_505870: native, nfs3hp::graphics::dead_0x505870 */

/* sub_505920: native, nfs3hp::graphics::dead_0x505920 */

/* sub_505950: native, nfs3hp::graphics::dead_0x505950 */

/* sub_505970: native, nfs3hp::graphics::dead_0x505970 */

/* sub_505980: native, nfs3hp::graphics::dead_0x505980 */

/* sub_5059d0: native, nfs3hp::graphics::dead_0x5059d0 */

/* sub_505a50: native, nfs3hp::graphics::dead_0x505a50 */

/* sub_505a80: native, nfs3hp::graphics::dead_0x505a80 */

/* sub_505b30: native, nfs3hp::graphics::dead_0x505b30 */

/* sub_506230: native, nfs3hp::graphics::dead_0x506230 */

/* sub_506310: native, nfs3hp::graphics::dead_0x506310 */

/* sub_506360: native, nfs3hp::graphics::dead_0x506360 */

/* sub_5063d0: native, nfs3hp::graphics::dead_0x5063d0 */

/* sub_506440: native, nfs3hp::graphics::dead_0x506440 */

/* sub_506690: native, nfs3hp::graphics::dead_0x506690 */

/* sub_506910: native, nfs3hp::graphics::dead_0x506910 */

/* sub_506ac0: native, nfs3hp::graphics::dead_0x506ac0 */

/* sub_506ca0: native, nfs3hp::graphics::dead_0x506ca0 */

/* sub_507240: native, nfs3hp::graphics::dead_0x507240 */

/* sub_5074b0: native, nfs3hp::graphics::dead_0x5074b0 */

/* sub_5075d0: native, nfs3hp::graphics::dead_0x5075d0 */

/* sub_507720: native, nfs3hp::graphics::dead_0x507720 */

/* sub_507900: native, nfs3hp::graphics::dead_0x507900 */

/* sub_5079a0: native, nfs3hp::graphics::dead_0x5079a0 */

/* sub_507a60: native, nfs3hp::graphics::dead_0x507a60 */

/* sub_507b10: native, nfs3hp::graphics::dead_0x507b10 */

/* sub_507bc0: native, nfs3hp::graphics::dead_0x507bc0 */

/* sub_507d00: native, nfs3hp::graphics::dead_0x507d00 */

/* sub_507ed0: native, nfs3hp::graphics::dead_0x507ed0 */

/* sub_507f30: native, nfs3hp::graphics::dead_0x507f30 */

/* sub_508170: native, nfs3hp::graphics::dead_0x508170 */

/* sub_5082f0: native, nfs3hp::graphics::dead_0x5082f0 */

/* sub_508390: native, nfs3hp::graphics::dead_0x508390 */

/* sub_5084e0: native, nfs3hp::graphics::dead_0x5084e0 */

/* sub_508500: native, nfs3hp::dead::stub_0x508500 */

/* sub_508510: native, nfs3hp::graphics::dead_0x508510 */

/* sub_5085a0: native, nfs3hp::dead::stub_0x5085a0 */

/* sub_5085c0: native, nfs3hp::dead::stub_0x5085c0 */

/* sub_508970: native, nfs3hp::graphics::dead_0x508970 */

/* sub_5089d0: native, nfs3hp::frontend::rendSetDisplayMode */

/* sub_508b20: native, nfs3hp::dead::stub_0x508b20 */

/* sub_508b80: native, nfs3hp::frontend::watbInitVideoWindow */

/* sub_508c70: native, nfs3hp::frontend::videoWindowMake */

/* sub_508da0: native, nfs3hp::frontend::videoWindowUpdate */

/* sub_508e70: native, nfs3hp::frontend::videoWindowFree */

/* sub_508ec0: native, nfs3hp::dead::stub_0x508ec0 */

/* sub_508ee0: native, nfs3hp::dead::stub_0x508ee0 */

/* sub_508f00: native, nfs3hp::dead::stub_0x508f00 */

/* sub_508fd0: native, nfs3hp::dead::stub_0x508fd0 */

/* sub_508ff4: native, nfs3hp::watcom::sopen */

/* sub_50901c: native, nfs3hp::watcom::sopen_ */

/* sub_509250: native, nfs3hp::watcom::allocfp */

/* sub_509314: native, nfs3hp::watcom::freefp */

/* sub_509350: native, nfs3hp::frontend::watbFreeClosedStreams */

/* sub_509380: native, nfs3hp::watcom::chktty */

/* sub_5093c0: native, nfs3hp::watcom::getpid */

/* sub_5093d0: native, nfs3hp::dead::stub_0x5093d0 */

/* sub_5094e0: native, nfs3hp::watcom::tmpdir */

/* sub_5095b0: native, nfs3hp::watcom::close */

/* sub_509640: native, nfs3hp::watcom::fixed_GetFileAttributesA */

/* sub_5096c0: native, nfs3hp::watcom::qwrite */

/* sub_5097b0: native, nfs3hp::watcom::fputc */

/* sub_5098d0: native, nfs3hp::frontend::watbShapeTypeForDepth */

/* sub_509940: native, nfs3hp::frontend::watbInitRowTable */

/* sub_509a00: native, nfs3hp::frontend::watbFreeRowTable */

/* sub_509a30: native, nfs3hp::frontend::watbMakeRow */

}
