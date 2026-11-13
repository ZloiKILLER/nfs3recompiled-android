#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_51de60: native, nfs3hp::frontend::paletteFrom6Bits */

/* sub_51def0: native, nfs3hp::dead::stub_0x51def0 */

/* sub_51df60: native, nfs3hp::frontend::paletteFrom8Bits */

/* sub_51dfb0: native, nfs3hp::dead::stub_0x51dfb0 */

/* sub_51e020: native, nfs3hp::dead::stub_0x51e020 */

/* sub_51e060: native, nfs3hp::watcom::wctomb_ */

/* sub_51e0a0: native, nfs3hp::watcom::utoa */

/* sub_51e0ec: native, nfs3hp::watcom::itoa */

/* sub_51e110: native, nfs3hp::frontend::watcFpNotLoaded */

/* sub_51e130: native, nfs3hp::watcom::ulltoa */

/* sub_51e1c8: native, nfs3hp::dead::stub_0x51e1c8 */

/* sub_51e220: native, nfs3hp::watcom::prtfUtoa */

/* sub_51e26c: native, nfs3hp::dead::stub_0x51e26c */

/* sub_51e290: native, nfs3hp::dead::stub_0x51e290 */

/* sub_51e2a0: native, nfs3hp::frontend::watcSetMbcpZero */

/* sub_51e2b0: native, nfs3hp::watcom::handlesFull */

/* sub_51e304: native, nfs3hp::watcom::allocPOSIXHandle */

/* sub_51e388: native, nfs3hp::frontend::watcSetOsHandle */

/* sub_51e45c: native, nfs3hp::watcom::freePOSIXHandle */

/* sub_51e490: native, nfs3hp::watcom::initPOSIXHandles */

/* sub_51e4f0: native, nfs3hp::watcom::NTGetFakeHandle */

/* sub_51e51c: native, nfs3hp::frontend::watcFreeHandleTable */

/* sub_51e540: native, nfs3hp::frontend::watcNothing51e540 */

/* sub_51e544: native, nfs3hp::frontend::watcNewSemaphore */

/* sub_51e5e8: native, nfs3hp::frontend::watcDeleteKeptSemaphores */

/* sub_51e618: native, nfs3hp::frontend::watcDeleteExtraSemaphores */

/* sub_51e670: native, nfs3hp::frontend::watcClearLock */

/* sub_51e680: native, nfs3hp::frontend::watcLockStreams */

/* sub_51e68c: native, nfs3hp::frontend::watcUnlockStreams */

/* sub_51e698: native, nfs3hp::frontend::watcLockFile */

/* sub_51e6a8: native, nfs3hp::frontend::watcUnlockFile */

/* sub_51e6b8: native, nfs3hp::frontend::watcAllocHandleHook */

/* sub_51e6c0: native, nfs3hp::frontend::watcFreeHandleHook */

/* sub_51e6dc: native, nfs3hp::frontend::watcLockHeap */

/* sub_51e6e4: native, nfs3hp::frontend::watcUnlockHeap */

/* sub_51e6f0: native, nfs3hp::frontend::watcLockFileTable */

/* sub_51e6f8: native, nfs3hp::frontend::watcUnlockFileTable */

/* sub_51e704: native, nfs3hp::frontend::watcLockThreadList */

/* sub_51e70c: native, nfs3hp::frontend::watcUnlockThreadList */

/* sub_51e714: native, nfs3hp::frontend::watcLockHandleTable */

/* sub_51e71c: native, nfs3hp::frontend::watcLock */

/* sub_51e778: native, nfs3hp::frontend::watcUnlockHandleTable */

/* sub_51e780: native, nfs3hp::frontend::watcUnlock */

/* sub_51e7a8: native, nfs3hp::frontend::watcThreadData */

/* sub_51e7ec: native, nfs3hp::watcom::initThreadData */

/* sub_51e828: native, nfs3hp::frontend::watcAllocTlsIndex */

/* sub_51e888: native, nfs3hp::frontend::watcAttachThread */

/* sub_51e8dc: native, nfs3hp::frontend::watcDetachThread */

/* sub_51e934: native, nfs3hp::frontend::watcEndThreads */

/* sub_51e940: native, nfs3hp::frontend::watcFreeTlsIndex */

/* sub_51e964: native, nfs3hp::frontend::watcInitThreads */

/* sub_51ea7c: native, nfs3hp::frontend::watcFiniThreads */

/* sub_51eb00: native, nfs3hp::watcom::moduleWideName */

/* sub_51eba0: native, nfs3hp::watcom::copyCommandLine */

/* sub_51ebf0: native, nfs3hp::watcom::copyWideCommandLine */

/* sub_51ec20: native, nfs3hp::watcom::stackLimits */

/* sub_51ec90: native, nfs3hp::frontend::watcHasActiveWindow */

/* sub_51eccc: native, nfs3hp::frontend::watcReportAppend */

/* sub_51ed24: native, nfs3hp::frontend::watcReportException */

/* sub_51ef08: native, nfs3hp::frontend::watcExceptionHandler */

/* sub_51f0d0: native, nfs3hp::watcom::InitExceptionHandling */

/* sub_51f11c: native, nfs3hp::watcom::DoneExceptionHandler */

/* sub_51f140: native, nfs3hp::frontend::watcFormatFloat */

/* sub_51f254: native, nfs3hp::frontend::watcScanFloat */

/* sub_51f270: native, nfs3hp::frontend::watcNothing51f270 */

/* sub_51f272: native, nfs3hp::pieces::stub_0x51f272 */

/* sub_51f2a0: native, nfs3hp::frontend::fdivBugCheck */

/* sub_51f320: native, nfs3hp::pieces::stub_0x51f320 */

/* sub_51f520: native, nfs3hp::port::panSides */

/* sub_51f580: native, nfs3hp::port::volumeMonoPair */

/* sub_51f5a4: native, nfs3hp::port::volumeMonoLevels */

/* sub_51f5dd: native, nfs3hp::mix::addMono8 */

/* sub_51f680: native, nfs3hp::port::volumeStereoPair */

/* sub_51f6a4: native, nfs3hp::port::volumeStereoLevels */

/* sub_51f6c7: native, nfs3hp::mix::addStereo8 */

/* sub_51f750: native, nfs3hp::port::volumeMonoSetup */

/* sub_51f774: native, nfs3hp::port::volumeMonoLevelsMmx */

/* sub_51f7ad: native, nfs3hp::mix::addMono16 */

/* sub_51f850: native, nfs3hp::port::volumeStereoSetup */

/* sub_51f874: native, nfs3hp::port::volumeStereoLevelsMmx */

/* sub_51f897: native, nfs3hp::mix::addStereo16 */

/* sub_51f930: native, nfs3hp::port::resample8MonoNeededDown */

/* sub_51f978: native, nfs3hp::port::resample8MonoNeededSame */

/* sub_51f980: native, nfs3hp::port::resample8MonoNeededUp */

/* sub_51f9ac: native, nfs3hp::port::resample8MonoDown */

/* sub_51fa80: native, nfs3hp::port::resample8MonoSame */

/* sub_51fa8c: native, nfs3hp::port::resample8MonoUp */

/* sub_51fb94: native, nfs3hp::port::resample8MonoStep */

/* sub_51fc38: native, nfs3hp::port::resample8MonoSetup */

/* sub_51fc80: native, nfs3hp::port::resample8StereoNeededDown */

/* sub_51fcc8: native, nfs3hp::port::resample8StereoNeededSame */

/* sub_51fcd0: native, nfs3hp::port::resample8StereoNeededUp */

/* sub_51fcfc: native, nfs3hp::port::resample8StereoDown */

/* sub_51fe24: native, nfs3hp::port::resample8StereoSame */

/* sub_51fe30: native, nfs3hp::port::resample8StereoUp */

/* sub_51ff88: native, nfs3hp::port::resample8StereoStep */

/* sub_52002c: native, nfs3hp::port::resample8StereoSetup */

/* sub_520070: native, nfs3hp::port::resample16MonoNeededDown */

/* sub_5200b8: native, nfs3hp::port::resample16MonoNeededSame */

/* sub_5200c0: native, nfs3hp::port::resample16MonoNeededUp */

}
