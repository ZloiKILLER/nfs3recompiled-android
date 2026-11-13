#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4f3a30: native, nfs3hp::dead::stub_0x4f3a30 */

/* sub_4f3a60: native, nfs3hp::port::SNDSTRM_createqueued */

/* sub_4f3a80: native, nfs3hp::port::SNDSTRM_requestdata */

/* sub_4f3a90: native, nfs3hp::port::SNDSTRM_restore */

/* sub_4f3aa4: native, nfs3hp::port::findStream */

/* sub_4f3ab5: native, nfs3hp::port::streamTaskSkipped */

/* sub_4f3ab8: native, nfs3hp::port::removeRequest */

/* sub_4f3b31: native, nfs3hp::port::framesPlayedDone */

/* sub_4f3b44: native, nfs3hp::port::packetFreed */

/* sub_4f3b60: native, nfs3hp::port::framesPlayed */

/* sub_4f3ba4: native, nfs3hp::port::iSNDstreamparseheader */

/* sub_4f3cc8: native, nfs3hp::port::iSNDstreamparseloop */

/* sub_4f3d00: native, nfs3hp::port::iSNDstreamparsedata */

/* sub_4f3dac: native, nfs3hp::port::iSNDstreamparseend */

/* sub_4f3dc4: native, nfs3hp::port::iSNDstreamparsechunk */

/* sub_4f3e1c: native, nfs3hp::port::streamWaits */

/* sub_4f3ebc: native, nfs3hp::port::iSNDstreamsystemtask */

/* sub_4f3fd0: native, nfs3hp::port::streamCount */

/* sub_4f3fec: native, nfs3hp::port::iSNDstreamcreate */

/* sub_4f4258: native, nfs3hp::port::iSNDstreamrequest */

/* sub_4f43ac: native, nfs3hp::dead::stub_0x4f43ac */

/* sub_4f43c0: native, nfs3hp::port::SNDSTRM_destroy */

/* sub_4f4438: native, nfs3hp::dead::stub_0x4f4438 */

/* sub_4f4440: native, nfs3hp::port::SNDSTRM_purge */

/* sub_4f44e0: native, nfs3hp::frontend::simFixedTranspose */

/* sub_4f4544: native, nfs3hp::frontend::simFixedMatrixTimes */

/* sub_4f45c0: native, nfs3hp::watcom::stackavail */

/* sub_4f45d0: native, nfs3hp::frontend::nothingC */

/* sub_4f45d1: native, nfs3hp::dead::stub_0x4f45d1 */

/* sub_4f45e1: native, nfs3hp::pieces::stub_0x4f45e1 */

/* sub_4f4610: native, nfs3hp::watcom::GRO */

/* sub_4f4630: native, nfs3hp::dead::stub_0x4f4630 */

/* sub_4f4650: native, nfs3hp::frontend::registryAddEntry */

/* sub_4f46b0: native, nfs3hp::frontend::screenPixels */

/* sub_4f46d0: native, nfs3hp::frontend::windowSave */

/* sub_4f4700: native, nfs3hp::frontend::windowRestore */

/* sub_4f4740: native, nfs3hp::frontend::textStateSave */

/* sub_4f4760: native, nfs3hp::frontend::textStateRestore */

/* sub_4f4780: native, nfs3hp::frontend::blockChecksum */

/* sub_4f47c0: native, nfs3hp::frontend::shapeEndsAtBend */

/* sub_4f4810: native, nfs3hp::dead::stub_0x4f4810 */

/* sub_4f48f0: native, nfs3hp::dead::stub_0x4f48f0 */

/* sub_4f4920: native, nfs3hp::frontend::windowCheckField */

/* sub_4f4a80: native, nfs3hp::frontend::windowCheck */

/* sub_4f4b80: native, nfs3hp::watcom::fprintf */

/* sub_4f4bb0: native, nfs3hp::frontend::netSameGuid */

/* sub_4f4c10: native, nfs3hp::frontend::netExit */

/* sub_4f4c50: native, nfs3hp::ea::netTrace */

/* sub_4f4c70: native, nfs3hp::frontend::netGetServices */

/* sub_4f4d10: native, nfs3hp::frontend::netInitService */

/* sub_4f4e60: native, nfs3hp::frontend::netKillService */

/* sub_4f4eb0: native, nfs3hp::frontend::netGetGames */

/* sub_4f4ed0: native, nfs3hp::frontend::netSetGame */

/* sub_4f4ef0: native, nfs3hp::dead::stub_0x4f4ef0 */

/* sub_4f4f10: native, nfs3hp::frontend::netNewGame */

/* sub_4f4f30: native, nfs3hp::pieces::stub_0x4f4f30 */

/* sub_4f4f50: native, nfs3hp::ea::netShutGame */

/* sub_4f4f70: native, nfs3hp::frontend::netOpenGame */

/* sub_4f4f90: native, nfs3hp::ea::netEndGame */

/* sub_4f4fd0: native, nfs3hp::dead::stub_0x4f4fd0 */

/* sub_4f4ff0: native, nfs3hp::ea::netKillPlayer */

/* sub_4f5010: native, nfs3hp::dead::stub_0x4f5010 */

/* sub_4f5040: native, nfs3hp::ea::netCheckPlayer */

/* sub_4f5050: native, nfs3hp::frontend::netServicePollCall */

/* sub_4f5070: native, nfs3hp::dead::stub_0x4f5070 */

/* sub_4f5090: native, nfs3hp::dead::stub_0x4f5090 */

/* sub_4f50b0: native, nfs3hp::frontend::netAnswerNo */

/* sub_4f50c0: native, nfs3hp::frontend::netCloseServices */

/* sub_4f50f0: native, nfs3hp::frontend::netPollOne */

/* sub_4f5110: native, nfs3hp::frontend::netPollServices */

/* sub_4f5130: native, nfs3hp::frontend::netServiceCall */

/* sub_4f5230: native, nfs3hp::frontend::netServicesReady */

/* sub_4f5280: native, nfs3hp::ea::connRestore */

/* sub_4f5310: native, nfs3hp::ea::connSend */

/* sub_4f5330: native, nfs3hp::ea::connClose */

/* sub_4f5390: native, nfs3hp::frontend::netFindGame */

/* sub_4f53f0: native, nfs3hp::frontend::netServicePoll */

/* sub_4f54f0: native, nfs3hp::frontend::netServiceOpenGame */

/* sub_4f55e0: native, nfs3hp::frontend::netServiceCloseGame */

/* sub_4f56c0: native, nfs3hp::frontend::netGameListed */

/* sub_4f56e0: native, nfs3hp::frontend::netServiceRoom */

/* sub_4f5700: native, nfs3hp::frontend::netServiceAsk */

/* sub_4f5710: native, nfs3hp::frontend::netServiceRead */

/* sub_4f57c0: native, nfs3hp::frontend::netServiceWrite */

/* sub_4f5850: native, nfs3hp::frontend::netSendQueued */

/* sub_4f5880: native, nfs3hp::frontend::netFromSockaddr */

/* sub_4f58c0: native, nfs3hp::frontend::netToSockaddr */

/* sub_4f5900: native, nfs3hp::frontend::netPortToSockaddr */

/* sub_4f5950: native, nfs3hp::frontend::netIpxRecvFrom */

/* sub_4f5a30: native, nfs3hp::frontend::netIpxAnswerName */

/* sub_4f5a80: native, nfs3hp::frontend::netPacketMatch */

/* sub_4f5ac0: native, nfs3hp::frontend::netMarkPacket */

/* sub_4f5b10: native, nfs3hp::frontend::netIpxThread */

/* sub_4f5ba0: native, nfs3hp::frontend::netIpxStartup */

/* sub_4f5bf0: native, nfs3hp::frontend::netWinsockClose */

/* sub_4f5c10: native, nfs3hp::frontend::netIpxOpen */

/* sub_4f5ea0: native, nfs3hp::frontend::netIpxClose */

/* sub_4f5f50: native, nfs3hp::frontend::netIpxPort */

/* sub_4f5f70: native, nfs3hp::frontend::netIpxKey */

/* sub_4f5f90: native, nfs3hp::frontend::netIpxJump */

}
