#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_518e40: native, nfs3hp::graphics::dead_0x518e40 */

/* sub_518eb0: native, nfs3hp::graphics::dead_0x518eb0 */

/* sub_518ee0: native, nfs3hp::graphics::dead_0x518ee0 */

/* sub_518ef0: native, nfs3hp::graphics::dead_0x518ef0 */

/* sub_518f80: native, nfs3hp::frontend::rendFontAlloc */

/* sub_518fc0: native, nfs3hp::frontend::rendFontFree */

/* sub_518fd0: native, nfs3hp::frontend::videoBuffersStart */

/* sub_519080: native, nfs3hp::dead::stub_0x519080 */

/* sub_519140: native, nfs3hp::frontend::winaReleaseVideoBuffers */

/* sub_5191b0: native, nfs3hp::dead::stub_0x5191b0 */

/* sub_519560: native, nfs3hp::dead::stub_0x519560 */

/* sub_5195b0: native, nfs3hp::dead::stub_0x5195b0 */

/* sub_5199b0: native, nfs3hp::frontend::fontFindGlyph */

/* sub_519a10: native, nfs3hp::dead::stub_0x519a10 */

/* sub_519a50: native, nfs3hp::frontend::fontGlyph */

/* sub_519ae0: native, nfs3hp::dead::stub_0x519ae0 */

/* sub_519e30: native, nfs3hp::dead::stub_0x519e30 */

/* sub_519ea0: native, nfs3hp::frontend::fontBitsPerPixel */

/* sub_519f10: native, nfs3hp::frontend::fontCreate */

/* sub_51a170: native, nfs3hp::frontend::winaFreeSurface */

/* sub_51a188: native, nfs3hp::dead::stub_0x51a188 */

/* sub_51a1b8: native, nfs3hp::pieces::stub_0x51a1b8 */

/* sub_51a1ba: native, nfs3hp::watcom::DSQRT */

/* sub_51a1f0: native, nfs3hp::dead::stub_0x51a1f0 */

/* sub_51a200: native, nfs3hp::dead::stub_0x51a200 */

/* sub_51a260: native, nfs3hp::dead::stub_0x51a260 */

/* sub_51a2a0: native, nfs3hp::dead::stub_0x51a2a0 */

/* sub_51a310: native, nfs3hp::dead::stub_0x51a310 */

/* sub_51a3b0: native, nfs3hp::dead::stub_0x51a3b0 */

/* sub_51a3e0: native, nfs3hp::dead::stub_0x51a3e0 */

/* sub_51a400: native, nfs3hp::dead::stub_0x51a400 */

/* sub_51a420: native, nfs3hp::dead::stub_0x51a420 */

/* sub_51a470: native, nfs3hp::frontend::netcModemResponse */

/* sub_51a7b0: native, nfs3hp::frontend::netcModemCommand */

/* sub_51a930: native, nfs3hp::frontend::netcModemAttention */

/* sub_51ab10: native, nfs3hp::dead::stub_0x51ab10 */

/* sub_51ab40: native, nfs3hp::frontend::netcSerialCloseAll */

/* sub_51ab70: native, nfs3hp::frontend::netcSerialCommand */

/* sub_51abc0: native, nfs3hp::frontend::netcSerialStartIo */

/* sub_51ad70: native, nfs3hp::frontend::netcSerialFinishIo */

/* sub_51aeb0: native, nfs3hp::frontend::netcSerialWaitEvents */

/* sub_51af10: native, nfs3hp::frontend::netcSerialThread */

/* sub_51b030: native, nfs3hp::frontend::netcSerialIsOpen */

/* sub_51b060: native, nfs3hp::dead::stub_0x51b060 */

/* sub_51b0d0: native, nfs3hp::dead::stub_0x51b0d0 */

/* sub_51b140: native, nfs3hp::dead::stub_0x51b140 */

/* sub_51b1c0: native, nfs3hp::frontend::netcSerialDropInput */

/* sub_51b1e0: native, nfs3hp::frontend::netcSerialDropOutput */

/* sub_51b200: native, nfs3hp::frontend::netcSerialClose */

/* sub_51b2f0: native, nfs3hp::frontend::netcSerialOpenHandle */

/* sub_51b480: native, nfs3hp::frontend::netcSerialOpen */

/* sub_51b530: native, nfs3hp::frontend::netcSerialWriteWhole */

/* sub_51b5a0: native, nfs3hp::dead::stub_0x51b5a0 */

/* sub_51b5f0: native, nfs3hp::frontend::netcSerialWrite */

/* sub_51b710: native, nfs3hp::frontend::netcSerialSet */

/* sub_51b830: native, nfs3hp::dead::stub_0x51b830 */

/* sub_51b8a0: native, nfs3hp::frontend::netcSerialInCount */

/* sub_51b900: native, nfs3hp::frontend::netcSerialOutRoom */

/* sub_51b970: native, nfs3hp::frontend::netcSerialPeek */

/* sub_51ba50: native, nfs3hp::frontend::netcSerialRead */

/* sub_51bae0: native, nfs3hp::dead::stub_0x51bae0 */

/* sub_51bb60: native, nfs3hp::dead::stub_0x51bb60 */

/* sub_51bbd0: native, nfs3hp::frontend::netcSerialWaitIn */

/* sub_51bc70: native, nfs3hp::frontend::netcFrameChecksum */

/* sub_51bca0: native, nfs3hp::frontend::netcFrameChecksumGood */

/* sub_51bcd0: native, nfs3hp::frontend::netcFrameBuild */

/* sub_51bcf0: native, nfs3hp::frontend::netcFrameSend */

/* sub_51bd50: native, nfs3hp::frontend::netcFrameSendControl */

/* sub_51bdc0: native, nfs3hp::frontend::netcUnreliableShutdown */

/* sub_51be90: native, nfs3hp::frontend::netcUnreliableReceiveFrame */

/* sub_51bfd0: native, nfs3hp::frontend::netcUnreliableReceive */

/* sub_51c0a0: native, nfs3hp::frontend::netcUnreliableNotify */

/* sub_51c100: native, nfs3hp::frontend::netcUnreliableOpen */

/* sub_51c510: native, nfs3hp::frontend::netcUnreliableSend */

/* sub_51c5c0: native, nfs3hp::frontend::netcUnreliableProcess */

/* sub_51c690: native, nfs3hp::frontend::netcUnreliableClose */

/* sub_51c7a0: native, nfs3hp::frontend::netcSequenceWiden */

/* sub_51c7c0: native, nfs3hp::frontend::netcPacketChecksum */

/* sub_51c7f0: native, nfs3hp::frontend::netcPacketChecksumGood */

/* sub_51c820: native, nfs3hp::frontend::netcPacketAckNumber */

/* sub_51c890: native, nfs3hp::frontend::netcPacketBuild */

/* sub_51c8f0: native, nfs3hp::frontend::netcPacketSend */

/* sub_51ca50: native, nfs3hp::frontend::netcPacketSendControl */

/* sub_51caf0: native, nfs3hp::frontend::netcPacketSendNak */

/* sub_51cb50: native, nfs3hp::frontend::netcPacketGotAck */

/* sub_51cc00: native, nfs3hp::frontend::netcPacketGotNak */

/* sub_51cc80: native, nfs3hp::frontend::netcPacketShutdown */

/* sub_51cd50: native, nfs3hp::frontend::netcPacketReceiveFrame */

/* sub_51cf30: native, nfs3hp::frontend::netcPacketReceive */

/* sub_51d000: native, nfs3hp::frontend::netcPacketQueueKey */

/* sub_51d010: native, nfs3hp::frontend::netcPacketCountDown */

/* sub_51d040: native, nfs3hp::frontend::netcPacketResendDue */

/* sub_51d050: native, nfs3hp::frontend::netcPacketNotify */

/* sub_51d270: native, nfs3hp::frontend::netcPacketOpen */

/* sub_51d810: native, nfs3hp::frontend::netcPacketSubmit */

/* sub_51d8f0: native, nfs3hp::frontend::netcPacketProcess */

/* sub_51d9e0: native, nfs3hp::frontend::netcPacketClose */

/* sub_51db20: native, nfs3hp::dead::stub_0x51db20 */

/* sub_51dd50: native, nfs3hp::dead::stub_0x51dd50 */

/* sub_51ddf0: native, nfs3hp::dead::stub_0x51ddf0 */

}
