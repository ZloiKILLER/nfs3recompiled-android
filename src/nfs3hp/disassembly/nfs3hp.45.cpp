#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_50e220: native, nfs3hp::ea::qposition */

/* sub_50e2d0: native, nfs3hp::frontend::qlowpriority */

/* sub_50e360: native, nfs3hp::dead::stub_0x50e360 */

/* sub_50e3f0: native, nfs3hp::frontend::qfind */

/* sub_50e470: native, nfs3hp::ea::qfindindex */

/* sub_50e500: native, nfs3hp::frontend::qfindpriority */

/* sub_50e5a0: native, nfs3hp::frontend::watbQueueFetch */

/* sub_50e670: native, nfs3hp::dead::stub_0x50e670 */

/* sub_50e740: native, nfs3hp::dead::stub_0x50e740 */

/* sub_50e800: native, nfs3hp::frontend::winaQueueTakeByPriority */

/* sub_50e8e0: native, nfs3hp::frontend::qiterate */

/* sub_50e9b0: native, nfs3hp::frontend::qsafeiterate */

/* sub_50ea20: native, nfs3hp::frontend::fileLockEnter */

/* sub_50ea40: native, nfs3hp::frontend::fileLockLeave */

/* sub_50ea4e: native, nfs3hp::watcom::modf */

/* sub_50ea68: native, nfs3hp::dead::stub_0x50ea68 */

/* sub_50ea85: native, nfs3hp::dead::stub_0x50ea85 */

/* sub_50eaa7: native, nfs3hp::dead::stub_0x50eaa7 */

/* sub_50ead0: native, nfs3hp::watcom::getenv */

/* sub_50eb30: native, nfs3hp::watcom::getctime */

/* sub_50eba0: native, nfs3hp::watcom::mktime */

/* sub_50ed50: native, nfs3hp::ea::huffUnpack */

/* sub_510110: native, nfs3hp::ea::runUnpack */

/* sub_5102a4: native, nfs3hp::loader::refpack */

/* sub_5102f0: native, nfs3hp::dead::stub_0x5102f0 */

/* sub_51039e: native, nfs3hp::pieces::stub_0x51039e */

/* sub_510337: native, nfs3hp::pieces::stub_0x510337 */

/* sub_510420: native, nfs3hp::ea::dxtBlock */

/* sub_510740: native, nfs3hp::dead::stub_0x510740 */

/* sub_510760: native, nfs3hp::ea::dxtUnpack */

/* sub_510820: native, nfs3hp::ea::pairExpand */

/* sub_510870: native, nfs3hp::ea::pairUnpack */

/* sub_510a10: native, nfs3hp::port::findRequest */

/* sub_510a50: native, nfs3hp::dead::stub_0x510a50 */

/* sub_510bb3: native, nfs3hp::dead::stub_0x510bb3 */

/* sub_510cbd: native, nfs3hp::dead::stub_0x510cbd */

/* sub_510e48: native, nfs3hp::dead::stub_0x510e48 */

/* sub_510ff8: native, nfs3hp::dead::stub_0x510ff8 */

/* sub_511200: native, nfs3hp::dead::stub_0x511200 */

/* sub_511210: native, nfs3hp::port::playerMemory */

/* sub_511228: native, nfs3hp::port::SNDPKTPLAY_create */

/* sub_5112d0: native, nfs3hp::port::SNDPKTPLAY_start */

/* sub_5114d4: native, nfs3hp::port::SNDPKTPLAY_submit */

/* sub_5115b8: native, nfs3hp::port::SNDPKTPLAY_submitspace */

/* sub_51162c: native, nfs3hp::port::framesOutstanding */

/* sub_511680: native, nfs3hp::port::SNDPKTPLAY_framesoutstanding */

/* sub_5116ac: native, nfs3hp::dead::stub_0x5116ac */

/* sub_511718: native, nfs3hp::port::SNDPKTPLAY_purge */

/* sub_51181c: native, nfs3hp::port::SNDPKTPLAY_stop */

/* sub_5118d0: native, nfs3hp::port::SNDPKTPLAY_destroy */

/* sub_5118fc: native, nfs3hp::port::nextPacket */

/* sub_511998: native, nfs3hp::port::packetFramesPlayed */

/* sub_5119b0: native, nfs3hp::port::patchDefaults */

/* sub_5119d8: native, nfs3hp::port::iSNDpatchtohdr */

/* sub_511bf0: native, nfs3hp::port::iSNDplatformcalcdatarate */

/* sub_511ca0: native, nfs3hp::port::streamChunksAtOnce */

/* sub_511cd0: native, nfs3hp::watcom::fatalWrite */

/* sub_511d0c: native, nfs3hp::watcom::fatal_runtime_error */

/* sub_511d30: native, nfs3hp::dead::stub_0x511d30 */

/* sub_511dc0: native, nfs3hp::dead::stub_0x511dc0 */

/* sub_512180: native, nfs3hp::frontend::commEvent */

/* sub_5123b0: native, nfs3hp::ea::layerSend */

/* sub_512470: native, nfs3hp::frontend::layerReceiveLast */

/* sub_512530: native, nfs3hp::ea::layerCallPacker */

/* sub_5125b0: native, nfs3hp::frontend::layerCallUnpacker */

/* sub_512630: native, nfs3hp::ea::layerPackFromTop */

/* sub_512650: native, nfs3hp::frontend::layerReceiveFromTop */

/* sub_512660: native, nfs3hp::frontend::connInitFilter */

/* sub_5126c0: native, nfs3hp::ea::layerFree */

/* sub_5126f0: native, nfs3hp::frontend::layerReceive */

/* sub_512710: native, nfs3hp::dead::stub_0x512710 */

/* sub_512770: native, nfs3hp::ea::layerSendPacket */

/* sub_512790: native, nfs3hp::dead::stub_0x512790 */

/* sub_5127b0: native, nfs3hp::dead::stub_0x5127b0 */

/* sub_5127d0: native, nfs3hp::frontend::ipxBroadcastOpen */

/* sub_512890: native, nfs3hp::frontend::ipxScanInterfaces */

/* sub_512940: native, nfs3hp::frontend::ipxAdvertiseTo */

/* sub_512980: native, nfs3hp::frontend::ipxIsRequest */

/* sub_5129e0: native, nfs3hp::frontend::ipxServerSocket */

/* sub_512ad0: native, nfs3hp::frontend::ipxServerThread */

/* sub_512c80: native, nfs3hp::frontend::ipxStartServer */

/* sub_512d70: native, nfs3hp::frontend::ipxStopServer */

/* sub_512dd0: native, nfs3hp::pieces::stub_0x512dd0 */

/* sub_512f60: native, nfs3hp::frontend::gameGuidText */

/* sub_512fc0: native, nfs3hp::frontend::setResponseTimeout */

/* sub_512fd0: native, nfs3hp::frontend::responseTimedOut */

/* sub_512ff0: native, nfs3hp::frontend::netReturnZeroToo */

/* sub_513000: native, nfs3hp::frontend::netCountBits */

/* sub_513030: native, nfs3hp::frontend::sendGameInfo */

/* sub_513190: native, nfs3hp::frontend::sendServerRequest */

/* sub_5132a0: native, nfs3hp::frontend::findPlayerByAddress */

/* sub_513320: native, nfs3hp::frontend::addNewPlayer */

/* sub_5133a0: native, nfs3hp::frontend::sessionMessage */

/* sub_5140e0: native, nfs3hp::frontend::netcGatherPulse */

/* sub_514260: native, nfs3hp::frontend::netcGatherPulseStart */

/* sub_514280: native, nfs3hp::frontend::netOpen */

/* sub_514320: native, nfs3hp::frontend::netClose */

/* sub_5143d0: native, nfs3hp::frontend::netcGatherRequest */

/* sub_5144b0: native, nfs3hp::frontend::netcGatherJoin */

/* sub_5145b0: native, nfs3hp::frontend::netcGatherGameInfo */

}
