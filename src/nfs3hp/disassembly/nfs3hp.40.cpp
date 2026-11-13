#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4f9410: native, nfs3hp::frontend::driveIsCd */

/* sub_4f9470: native, nfs3hp::dead::stub_0x4f9470 */

/* sub_4f9482: native, nfs3hp::watcom::IF_DACOS */

/* sub_4f94bf: native, nfs3hp::dead::stub_0x4f94bf */

/* sub_4f9500: native, nfs3hp::frontend::lineCallback */

/* sub_4f97c0: native, nfs3hp::frontend::lineShutdown */

/* sub_4f9920: native, nfs3hp::frontend::tapiSetup */

/* sub_4f99f0: native, nfs3hp::frontend::tapiInitialize */

/* sub_4f9a50: native, nfs3hp::frontend::tapiSelectModem */

/* sub_4f9ac0: native, nfs3hp::frontend::tapiEnumerateModems */

/* sub_4f9d20: native, nfs3hp::frontend::tapiOpenLine */

/* sub_4f9df0: native, nfs3hp::frontend::tapiGetCommHandleConfig */

/* sub_4f9ea0: native, nfs3hp::frontend::tapiSetCommConfig */

/* sub_4f9f30: native, nfs3hp::frontend::tapiGetDevCaps */

/* sub_4fa030: native, nfs3hp::frontend::tapiMakeCall */

/* sub_4fa0f0: native, nfs3hp::frontend::tapiGetModemHandle */

/* sub_4fa1d0: native, nfs3hp::frontend::tapiHangUp */

/* sub_4fa210: native, nfs3hp::frontend::tapiThread */

/* sub_4fa310: native, nfs3hp::frontend::modemOpen */

/* sub_4fa450: native, nfs3hp::frontend::modemClose */

/* sub_4fa480: native, nfs3hp::frontend::modemPoll */

/* sub_4fa4e0: native, nfs3hp::frontend::modemSendReady */

/* sub_4fa4f0: native, nfs3hp::frontend::modemReceiveReady */

/* sub_4fa500: native, nfs3hp::frontend::modemSend */

/* sub_4fa530: native, nfs3hp::frontend::modemReceive */

/* sub_4fa550: native, nfs3hp::frontend::modemOpenNetwork */

/* sub_4fa5f0: native, nfs3hp::frontend::modemName */

/* sub_4fa620: native, nfs3hp::frontend::serialOpen */

/* sub_4fa6d0: native, nfs3hp::frontend::serialClose */

/* sub_4fa6e0: native, nfs3hp::frontend::serialPoll */

/* sub_4fa710: native, nfs3hp::frontend::serialSendReady */

/* sub_4fa720: native, nfs3hp::frontend::serialReceiveReady */

/* sub_4fa730: native, nfs3hp::frontend::serialSend */

/* sub_4fa760: native, nfs3hp::frontend::serialReceive */

/* sub_4fa780: native, nfs3hp::frontend::serialOpenNetwork */

/* sub_4fa830: native, nfs3hp::dead::stub_0x4fa830 */

/* sub_4faa40: native, nfs3hp::dead::stub_0x4faa40 */

/* sub_4fabe0: native, nfs3hp::dead::stub_0x4fabe0 */

/* sub_4faea0: native, nfs3hp::dead::stub_0x4faea0 */

/* sub_4fb030: native, nfs3hp::dead::stub_0x4fb030 */

/* sub_4fb060: native, nfs3hp::frontend::winaFreeKeptBlock */

/* sub_4fb080: native, nfs3hp::dead::stub_0x4fb080 */

/* sub_4fb140: native, nfs3hp::dead::stub_0x4fb140 */

/* sub_4fb170: native, nfs3hp::dead::stub_0x4fb170 */

/* sub_4fb1a0: native, nfs3hp::dead::stub_0x4fb1a0 */

/* sub_4fb1e0: native, nfs3hp::port::SNDbankpatpresent */

/* sub_4fb300: native, nfs3hp::watcom::bsearch */

/* sub_4fb380: native, nfs3hp::frontend::screenTakeSize */

/* sub_4fb3b0: native, nfs3hp::frontend::screenLockCountUp */

/* sub_4fb3c0: native, nfs3hp::frontend::screenLockCountDown */

/* sub_4fb3d0: native, nfs3hp::frontend::screenInstallDriver */

/* sub_4fb440: native, nfs3hp::pieces::stub_0x4fb440 */

/* sub_4fb536: native, nfs3hp::pieces::stub_0x4fb536 */

/* sub_4fb527: native, nfs3hp::pieces::stub_0x4fb527 */

/* sub_4fb51b: native, nfs3hp::pieces::stub_0x4fb51b */

/* sub_4fb50b: native, nfs3hp::pieces::stub_0x4fb50b */

/* sub_4fb4ea: native, nfs3hp::pieces::stub_0x4fb4ea */

/* sub_4fb4d0: native, nfs3hp::pieces::stub_0x4fb4d0 */

/* sub_4fb4c0: native, nfs3hp::pieces::stub_0x4fb4c0 */

/* sub_4fb470: native, nfs3hp::frontend::winaPointerMoved */

/* sub_4fb560: native, nfs3hp::dead::stub_0x4fb560 */

/* sub_4fb590: native, nfs3hp::dead::stub_0x4fb590 */

/* sub_4fb5a0: native, nfs3hp::dead::stub_0x4fb5a0 */

/* sub_4fb6a0: native, nfs3hp::pieces::stub_0x4fb6a0 */

/* sub_4fb6a7: native, nfs3hp::pieces::stub_0x4fb6a7 */

/* sub_4fb6ae: native, nfs3hp::pieces::stub_0x4fb6ae */

/* sub_4fb6b5: native, nfs3hp::pieces::stub_0x4fb6b5 */

/* sub_4fb6c2: native, nfs3hp::pieces::stub_0x4fb6c2 */

/* sub_4fb6d7: native, nfs3hp::pieces::stub_0x4fb6d7 */

/* sub_4fb6f0: native, nfs3hp::dead::stub_0x4fb6f0 */

/* sub_4fb700: native, nfs3hp::frontend::numLockOff */

/* sub_4fb710: native, nfs3hp::frontend::lockKeySet */

/* sub_4fb790: native, nfs3hp::frontend::bigFileOpenEntry */

/* sub_4fb820: native, nfs3hp::frontend::watbKeyMatches */

/* sub_4fb82b: native, nfs3hp::frontend::watbNoMatch */

/* sub_4fb830: native, nfs3hp::frontend::watbSortKey */

/* sub_4fb850: native, nfs3hp::dead::stub_0x4fb850 */

/* sub_4fb860: native, nfs3hp::frontend::fileDeviceStart */

/* sub_4fb990: native, nfs3hp::frontend::winaFileOpAllocate */

/* sub_4fbb60: native, nfs3hp::frontend::winaFileOpQueue */

/* sub_4fbbe0: native, nfs3hp::frontend::winaFileOpSetName */

/* sub_4fbc30: native, nfs3hp::frontend::asyncOpenFile */

/* sub_4fbd10: native, nfs3hp::frontend::fileOpFind */

/* sub_4fbda0: native, nfs3hp::frontend::asyncFileThread */

/* sub_4fc170: native, nfs3hp::frontend::asyncFilesShutdown */

/* sub_4fc250: native, nfs3hp::frontend::asyncFilesInit */

/* sub_4fc340: native, nfs3hp::frontend::fileOpStatus */

/* sub_4fc400: native, nfs3hp::dead::stub_0x4fc400 */

/* sub_4fc480: native, nfs3hp::dead::stub_0x4fc480 */

/* sub_4fc510: native, nfs3hp::dead::stub_0x4fc510 */

/* sub_4fc600: native, nfs3hp::frontend::fileOpComplete */

/* sub_4fc780: native, nfs3hp::frontend::fileOpWait */

/* sub_4fc860: native, nfs3hp::dead::stub_0x4fc860 */

/* sub_4fc8e0: native, nfs3hp::frontend::winaFileOpCallback */

/* sub_4fc9b0: native, nfs3hp::frontend::winaFileOpPriority */

/* sub_4fca00: native, nfs3hp::dead::stub_0x4fca00 */

/* sub_4fca30: native, nfs3hp::dead::stub_0x4fca30 */

/* sub_4fca60: native, nfs3hp::frontend::winaFileOpOpen */

/* sub_4fca90: native, nfs3hp::frontend::winaFileOpClose */

/* sub_4fcab0: native, nfs3hp::frontend::winaFileOpRead */

}
