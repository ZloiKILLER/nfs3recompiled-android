#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_5200ec: native, nfs3hp::port::resample16MonoDown */

/* sub_5201d8: native, nfs3hp::port::resample16MonoSame */

/* sub_5201e4: native, nfs3hp::port::resample16MonoUp */

/* sub_5202fc: native, nfs3hp::port::resampleMonoStep */

/* sub_5203a0: native, nfs3hp::port::resampleMonoSetup */

/* sub_5203f0: native, nfs3hp::port::resample16StereoNeededDown */

/* sub_520438: native, nfs3hp::port::resample16StereoNeededSame */

/* sub_520440: native, nfs3hp::port::resample16StereoNeededUp */

/* sub_52046c: native, nfs3hp::port::resample16StereoDown */

/* sub_5205b0: native, nfs3hp::port::resample16StereoSame */

/* sub_5205bc: native, nfs3hp::port::resample16StereoUp */

/* sub_520740: native, nfs3hp::port::resampleStereoStep */

/* sub_5207e4: native, nfs3hp::port::resampleStereoSetup */

/* sub_520830: native, nfs3hp::mix::outputMono8 */

/* sub_520870: native, nfs3hp::mix::outputStereo8 */

/* sub_520920: native, nfs3hp::mix::outputStereo16 */

/* sub_5209a0: native, nfs3hp::mix::clear */

/* sub_520a00: native, nfs3hp::mix::finished */

/* sub_520a10: native, nfs3hp::mix::outputMono16 */

/* sub_520a40: native, nfs3hp::snd::dead_0x520a40 */

/* sub_520a5e: native, nfs3hp::snd::dead_0x520a5e */

/* sub_520aee: native, nfs3hp::snd::dead_0x520aee */

/* sub_520b7e: native, nfs3hp::snd::dead_0x520b7e */

/* sub_520bed: native, nfs3hp::snd::dead_0x520bed */

/* sub_520c69: native, nfs3hp::snd::dead_0x520c69 */

/* sub_520ce6: native, nfs3hp::snd::dead_0x520ce6 */

/* sub_520d6b: native, nfs3hp::snd::dead_0x520d6b */

/* sub_520df1: native, nfs3hp::snd::dead_0x520df1 */

/* sub_520e00: native, nfs3hp::snd::dead_0x520e00 */

/* sub_520e1e: native, nfs3hp::snd::dead_0x520e1e */

/* sub_520eae: native, nfs3hp::snd::dead_0x520eae */

/* sub_520f40: native, nfs3hp::snd::dead_0x520f40 */

/* sub_520fb1: native, nfs3hp::snd::dead_0x520fb1 */

/* sub_52102d: native, nfs3hp::snd::dead_0x52102d */

/* sub_5210ac: native, nfs3hp::snd::dead_0x5210ac */

/* sub_521133: native, nfs3hp::snd::dead_0x521133 */

/* sub_5211bb: native, nfs3hp::snd::dead_0x5211bb */

/* sub_5211c0: native, nfs3hp::snd::dead_0x5211c0 */

/* sub_521200: native, nfs3hp::snd::dead_0x521200 */

/* sub_521215: native, nfs3hp::snd::dead_0x521215 */

/* sub_52125b: native, nfs3hp::snd::dead_0x52125b */

/* sub_521310: native, nfs3hp::snd::dead_0x521310 */

/* sub_521350: native, nfs3hp::snd::dead_0x521350 */

/* sub_521365: native, nfs3hp::snd::dead_0x521365 */

/* sub_5213ab: native, nfs3hp::snd::dead_0x5213ab */

/* sub_521490: native, nfs3hp::snd::dead_0x521490 */

/* sub_5214d8: native, nfs3hp::snd::dead_0x5214d8 */

/* sub_5214e0: native, nfs3hp::snd::dead_0x5214e0 */

/* sub_52150c: native, nfs3hp::snd::dead_0x52150c */

/* sub_521614: native, nfs3hp::snd::dead_0x521614 */

/* sub_521620: native, nfs3hp::snd::dead_0x521620 */

/* sub_52175c: native, nfs3hp::snd::dead_0x52175c */

/* sub_521800: native, nfs3hp::snd::dead_0x521800 */

/* sub_521840: native, nfs3hp::snd::dead_0x521840 */

/* sub_5218c0: native, nfs3hp::snd::dead_0x5218c0 */

/* sub_521908: native, nfs3hp::snd::dead_0x521908 */

/* sub_521910: native, nfs3hp::snd::dead_0x521910 */

/* sub_52193c: native, nfs3hp::snd::dead_0x52193c */

/* sub_521a10: native, nfs3hp::snd::dead_0x521a10 */

/* sub_521a1c: native, nfs3hp::snd::dead_0x521a1c */

/* sub_521b20: native, nfs3hp::snd::dead_0x521b20 */

/* sub_521bc4: native, nfs3hp::snd::dead_0x521bc4 */

/* sub_521c10: native, nfs3hp::snd::dead_0x521c10 */

/* sub_521c90: native, nfs3hp::snd::dead_0x521c90 */

/* sub_521d00: native, nfs3hp::snd::dead_0x521d00 */

/* sub_521d80: native, nfs3hp::port::pcmTablesFirst */

/* sub_521e60: native, nfs3hp::port::pcmTablesSecond */

/* sub_521f30: native, nfs3hp::port::xaFloatTable */

/* sub_521f7c: native, nfs3hp::port::xaIntTable */

/* sub_521fc0: native, nfs3hp::port::xaTables */

/* sub_5220a0: native, nfs3hp::port::lastTables */

/* sub_522100: native, nfs3hp::snd::dead_0x522100 */

/* sub_522130: native, nfs3hp::snd::dead_0x522130 */

/* sub_5221f0: native, nfs3hp::ea::physicalMemory */

/* sub_522220: native, nfs3hp::snd::dead_0x522220 */

/* sub_522250: native, nfs3hp::snd::dead_0x522250 */

/* sub_5222f0: native, nfs3hp::port::panSplit */

/* sub_522320: native, nfs3hp::port::panSurround */

/* sub_5224b0: native, nfs3hp::port::panLawPositive */

/* sub_5224e0: native, nfs3hp::port::randomOffset */

/* sub_522510: native, nfs3hp::port::iSNDplatformplay */

/* sub_5229d0: native, nfs3hp::port::nextTag */

/* sub_522a80: native, nfs3hp::port::iSNDplatformresolve */

/* sub_522d08: native, nfs3hp::port::platformUnresolve */

/* sub_522d80: native, nfs3hp::watcom::fsync */

/* sub_522de0: native, nfs3hp::watcom::GetIOMode */

/* sub_522e38: native, nfs3hp::watcom::SetIOMode */

/* sub_522e60: native, nfs3hp::watcom::getche */

/* sub_522e90: native, nfs3hp::port::pitchFactor */

/* sub_522f10: native, nfs3hp::port::mixStep */

/* sub_522f50: native, nfs3hp::port::setMixStep */

/* sub_522f90: native, nfs3hp::snd::dead_0x522f90 */

/* sub_522fd4: native, nfs3hp::snd::dead_0x522fd4 */

/* sub_5230b8: native, nfs3hp::port::reverbStart */

/* sub_5230dc: native, nfs3hp::snd::dead_0x5230dc */

/* sub_523118: native, nfs3hp::snd::dead_0x523118 */

/* sub_523200: native, nfs3hp::port::reverbLength */

/* sub_52325c: native, nfs3hp::port::reverbInit */

/* sub_523310: native, nfs3hp::port::setVoiceSend */

/* sub_523332: native, nfs3hp::graphics::dead_0x523332 */

}
