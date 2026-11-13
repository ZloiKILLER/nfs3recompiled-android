#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4f5fa0: native, nfs3hp::frontend::netIpxSetName */

/* sub_4f5fc0: native, nfs3hp::frontend::netIpxForgetName */

/* sub_4f5fd0: native, nfs3hp::frontend::netAnswerYes */

/* sub_4f5fe0: native, nfs3hp::frontend::netSameNode */

/* sub_4f6010: native, nfs3hp::frontend::netIpxOwnAddress */

/* sub_4f6030: native, nfs3hp::frontend::netIpxReady */

/* sub_4f60b0: native, nfs3hp::frontend::netIpxSend */

/* sub_4f61f0: native, nfs3hp::frontend::netIpxReceive */

/* sub_4f6350: native, nfs3hp::frontend::tcpFlushQueue */

/* sub_4f63d0: native, nfs3hp::frontend::ipxOpenWinsock */

/* sub_4f6490: native, nfs3hp::frontend::tcpListenSocket */

/* sub_4f6510: native, nfs3hp::frontend::tcpSetSocketOptions */

/* sub_4f6570: native, nfs3hp::frontend::tcpFillSelectSets */

/* sub_4f6640: native, nfs3hp::frontend::tcpReceivePacket */

/* sub_4f67b0: native, nfs3hp::frontend::tcpSendPending */

/* sub_4f6860: native, nfs3hp::frontend::tcpExceptionSeen */

/* sub_4f6870: native, nfs3hp::frontend::tcpConnectionDropped */

/* sub_4f68c0: native, nfs3hp::frontend::tcpServerThread */

/* sub_4f6c50: native, nfs3hp::frontend::tcpOpenConnection */

/* sub_4f7030: native, nfs3hp::frontend::tcpSendConnection */

/* sub_4f71a0: native, nfs3hp::frontend::tcpCloseConnection */

/* sub_4f7220: native, nfs3hp::frontend::tcpSameAddressParts */

/* sub_4f7280: native, nfs3hp::frontend::tcpHostGame */

/* sub_4f7320: native, nfs3hp::frontend::tcpEndGame */

/* sub_4f7370: native, nfs3hp::frontend::netReturnZero */

/* sub_4f7380: native, nfs3hp::frontend::tcpOpenNetwork */

/* sub_4f73a0: native, nfs3hp::frontend::tcpStartWinsock */

/* sub_4f7420: native, nfs3hp::frontend::tcpStopWinsock */

/* sub_4f7450: native, nfs3hp::frontend::tcpOpen */

/* sub_4f75d0: native, nfs3hp::frontend::tcpClose */

/* sub_4f7660: native, nfs3hp::frontend::tcpFirstField */

/* sub_4f7670: native, nfs3hp::frontend::tcpResolve */

/* sub_4f7750: native, nfs3hp::frontend::tcpAddressText */

/* sub_4f77c0: native, nfs3hp::frontend::tcpSameAddress */

/* sub_4f77f0: native, nfs3hp::frontend::tcpLocalName */

/* sub_4f7830: native, nfs3hp::frontend::tcpOpenWinsock */

/* sub_4f7900: native, nfs3hp::watcom::nmalloc */

/* sub_4f79f0: native, nfs3hp::watcom::nfree */

/* sub_4f7af8: native, nfs3hp::watcom::TryInit */

/* sub_4f7b1a: native, nfs3hp::dead::stub_0x4f7b1a */

/* sub_4f7b2a: native, nfs3hp::watcom::TryHandler */

/* sub_4f7b61: native, nfs3hp::watcom::TryScope */

/* sub_4f7b7d: native, nfs3hp::watcom::Except */

/* sub_4f7b8c: native, nfs3hp::watcom::TryDone */

/* sub_4f7b87: native, nfs3hp::watcom::TryOuter */

/* sub_4f7ba7: native, nfs3hp::watcom::global_unwind */

/* sub_4f7bc3: native, nfs3hp::watcom::local_unwind */

/* sub_4f7c09: native, nfs3hp::dead::stub_0x4f7c09 */

/* sub_4f7c20: native, nfs3hp::watcom::atexit */

/* sub_4f7c48: native, nfs3hp::frontend::exitFunctionsRun */

/* sub_4f7c90: native, nfs3hp::watcom::control87 */

/* sub_4f7ce0: native, nfs3hp::watcom::kbhit */

/* sub_4f7d70: native, nfs3hp::watcom::fflush */

/* sub_4f7d80: native, nfs3hp::watcom::fputs */

/* sub_4f7e20: native, nfs3hp::watcom::do_getch */

/* sub_4f7f24: native, nfs3hp::watcom::getch */

/* sub_4f7fa0: native, nfs3hp::watcom::cstart_ */

/* sub_4f8020: native, nfs3hp::watcom::strupr */

/* sub_4f8040: native, nfs3hp::watcom::localtime_ */

/* sub_4f8094: native, nfs3hp::watcom::localtime */

/* sub_4f80ac: native, nfs3hp::dead::stub_0x4f80ac */

/* sub_4f80b4: native, nfs3hp::watcom::IF_DEXP */

/* sub_4f80e5: native, nfs3hp::pieces::stub_0x4f80e5 */

/* sub_4f80b6: native, nfs3hp::pieces::stub_0x4f80b6 */

/* sub_4f8116: native, nfs3hp::watcom::exp */

/* sub_4f8122: native, nfs3hp::watcom::Strtold */

/* sub_4f82b6: native, nfs3hp::watcom::strtod */

/* sub_4f8390: native, nfs3hp::THRASH_opendll */

/* sub_4f87a0: native, nfs3hp::frontend::winaReleaseDriverSlot */

/* sub_4f87e0: native, nfs3hp::frontend::thrashOpenByName */

/* sub_4f8830: native, nfs3hp::graphics::setScreenFunctions */

/* sub_4f88c0: native, nfs3hp::dead::stub_0x4f88c0 */

/* sub_4f8930: native, nfs3hp::frontend::graphicsStart */

/* sub_4f8b50: native, nfs3hp::watcom::setenv */

/* sub_4f8ce8: native, nfs3hp::watcom::setenv_ */

/* sub_4f8f38: native, nfs3hp::watcom::findenv */

/* sub_4f9060: native, nfs3hp::frontend::ddrawNoteChange */

/* sub_4f9070: native, nfs3hp::ea::surfaceLocked */

/* sub_4f9088: native, nfs3hp::frontend::lineNoFlag */

/* sub_4f9090: native, nfs3hp::dead::stub_0x4f9090 */

/* sub_4f90a0: native, nfs3hp::dead::stub_0x4f90a0 */

/* sub_4f90d0: native, nfs3hp::dead::stub_0x4f90d0 */

/* sub_4f90e0: native, nfs3hp::frontend::windowReshow */

/* sub_4f9120: native, nfs3hp::frontend::winaHasFocus */

/* sub_4f9140: native, nfs3hp::dead::stub_0x4f9140 */

/* sub_4f9170: native, nfs3hp::dead::stub_0x4f9170 */

/* sub_4f9190: native, nfs3hp::dead::stub_0x4f9190 */

/* sub_4f91b0: native, nfs3hp::dead::stub_0x4f91b0 */

/* sub_4f91d0: native, nfs3hp::frontend::singleInstanceCheck */

/* sub_4f9260: native, nfs3hp::frontend::memClassNoPurge */

/* sub_4f9270: native, nfs3hp::frontend::memClassCreateRam */

/* sub_4f92b0: native, nfs3hp::frontend::heapStart */

/* sub_4f92d0: native, nfs3hp::frontend::heapStartDefault */

/* sub_4f92e0: native, nfs3hp::dead::stub_0x4f92e0 */

/* sub_4f92f0: native, nfs3hp::dead::stub_0x4f92f0 */

/* sub_4f9310: native, nfs3hp::dead::stub_0x4f9310 */

/* sub_4f9320: native, nfs3hp::frontend::fontsStart */

/* sub_4f9360: native, nfs3hp::frontend::winaVideoClose */

/* sub_4f9390: native, nfs3hp::frontend::pathInDirectory */

/* sub_4f93f0: native, nfs3hp::frontend::driveNumberOfPath */

}
