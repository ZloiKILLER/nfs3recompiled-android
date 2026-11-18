#ifndef NFS3HP_NATIVE_FRONTEND_NETB_H
#define NFS3HP_NATIVE_FRONTEND_NETB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_netb.cpp's typed functions (EA's network library: TCP, IPX, TAPI, the queues,
 * LAYER and the sessions' messages), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace netb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x512180: a line in the network's log, a ring of 64-byte records at [0x5689d0]: the time.
reg32 commEvent(App* app, CPU& cpu, reg32 text, reg32 what, reg32 data, reg32 bytes, reg32 flagged, reg32 kindWord);
// The game's 0x513000: how many bits a word has set.
reg32 countBits(App* app, CPU& cpu, reg32 value);
// The game's 0x513030: this game's description sent to an address: eax the game, edx the address.
reg32 sendGameInfo(App* app, CPU& cpu, reg32 game, reg32 to, reg32 id);
// The game's 0x5132a0: the player of a game at an address, his time last heard from set.
reg32 findPlayerByAddress(App* app, CPU& cpu, reg32 game, reg32 address);
// The game's 0x513320: a player's record taken into a game's first empty place.
reg32 addNewPlayer(App* app, CPU& cpu, reg32 game, reg32 record);
// The game's 0x5133a0: a session message from an address.
reg32 sessionMessage(App* app, CPU& cpu, reg32 session, reg32 from, reg32 type, reg32 message, reg32 length);
// The game's 0x50dcd0: qinit: a queue with a lock of its own, eax it, edx its priority function or 0 for the default.
reg32 qinit(App* app, CPU& cpu, reg32 queue, reg32 priority, reg32 argument);
// The game's 0x50dd60: qinitpeer: a queue that shares the lock of the queue in ecx.
reg32 qinitpeer(App* app, CPU& cpu, reg32 queue, reg32 priority, reg32 argument, reg32 peer);
// The game's 0x50de50: qpush: an element at the head.
reg32 qpush(App* app, CPU& cpu, reg32 queue, reg32 element);
// The game's 0x50ded0: qbottom: an element at the tail.
reg32 qbottom(App* app, CPU& cpu, reg32 queue, reg32 element);
// The game's 0x50df60: qpop: the element at the head, taken off.
reg32 qpop(App* app, CPU& cpu, reg32 queue);
// The game's 0x50dff0: qpoplast: the element at the tail, taken off.
reg32 qpoplast(App* app, CPU& cpu, reg32 queue);
// The game's 0x50e0a0: qinsert: an element in the order of the queue's priority function.
reg32 qinsert(App* app, CPU& cpu, reg32 queue, reg32 element);
// The game's 0x50e2d0: qlowpriority: the priority of the element at the head, -1 for none.
reg32 qlowpriority(App* app, CPU& cpu, reg32 queue);
// The game's 0x50e3f0: qfind: the first element a function says yes to, edx it, ebx its argument.
reg32 qfind(App* app, CPU& cpu, reg32 queue, reg32 match, reg32 argument);
// The game's 0x50e500: qfindpriority: the element of exactly a priority, edx it.
reg32 qfindpriority(App* app, CPU& cpu, reg32 queue, reg32 priority);
// The game's 0x50e8e0: qiterate: a function on each element until the queue changes, outside the lock.
reg32 qiterate(App* app, CPU& cpu, reg32 queue, reg32 function, reg32 argument);
// The game's 0x50e9b0: qsafeiterate: qiterate with the lock held all along.
reg32 qsafeiterate(App* app, CPU& cpu, reg32 queue, reg32 function, reg32 argument);
// The game's 0x512470: under the last layer: the pieces -- edx of them at ebx, {length.
reg32 layerReceiveLast(App* app, CPU& cpu, reg32 connection, reg32 pieceCount, reg32 pieces);
// The game's 0x5125b0: to the layer of an index in the queue of layers on the stack, or past the last.
reg32 layerCallUnpacker(App* app, CPU& cpu, reg32 first, reg32 index, reg32 second, reg32 pieces, reg32 layers);
// The game's 0x512650: from the top of a connection's layers: ecx them, edx the pieces' count, ebx them.
reg32 layerReceiveFromTop(App* app, CPU& cpu, reg32 first, reg32 count, reg32 pieces, reg32 layers);
// The game's 0x512660: CONN_initfilter: a connection's queue of layers.
reg32 connInitFilter(App* app, CPU& cpu, reg32 connection);
// The game's 0x5126f0: a packet received on a connection, edx it, ebx how long: one piece for the layers.
reg32 layerReceive(App* app, CPU& cpu, reg32 connection, reg32 packet, reg32 length);
// The game's 0x4f6350: what is queued for an address flushed: edx a packet whose first 20 bytes are the address, or 0.
reg32 tcpFlushQueue(App* app, CPU& cpu, reg32 owner, reg32 packet, reg32 rest);
// The game's 0x4f63d0: IPX's kind of network, once: Winsock tried, and the list told if it will do.
reg32 ipxOpenWinsock(App* app, CPU& cpu, reg32 remember);
// The game's 0x4f7830: the same for TCP: Windows has a network, Winsock starts and a socket can be made.
reg32 tcpOpenWinsock(App* app, CPU& cpu, reg32 remember);
// The game's 0x4f6490: a TCP socket bound to a port, -1 for none.
reg32 tcpListenSocket(App* app, CPU& cpu, reg32 port);
// The game's 0x4f6510: a socket made non-blocking, reusable and without Nagle's delay.
reg32 tcpSetSocketOptions(App* app, CPU& cpu, reg32 socket);
// The game's 0x4f6570: one connection into select's three sets, for qsafeiterate: edx the frame of sub_4f68c0.
reg32 tcpFillSelectSets(App* app, CPU& cpu, reg32 connection, reg32 sets);
// The game's 0x4f6640: a packet read from a connection's socket when it has all come.
reg32 tcpReceivePacket(App* app, CPU& cpu, reg32 listener, reg32 connection);
// The game's 0x4f67b0: what is left of a connection's packet sent.
reg32 tcpSendPending(App* app, CPU& cpu, reg32 connection);
// The game's 0x4f6870: a dead connection closed and off the open queue, told to its owner.
reg32 tcpConnectionDropped(App* app, CPU& cpu, reg32 connection);
// The game's 0x4f68c0: the TCP thread: selects over the listening socket.
reg32 tcpServerThread(App* app, CPU& cpu);
// The game's 0x4f6c50: a connection opened over TCP: with no address, by waiting for someone to connect to the thread.
reg32 tcpOpenConnection(App* app, CPU& cpu, reg32 connection, reg32 room);
// The game's 0x4f7030: a packet sent on a connection: its length in network order before it.
reg32 tcpSendConnection(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length);
// The game's 0x4f71a0: a connection closed for good and its buffer freed if it was its own.
reg32 tcpCloseConnection(App* app, CPU& cpu, reg32 connection);
// The game's 0x4f7220: whether two addresses are the same: a dword, two words and 8 bytes.
reg32 tcpSameAddressParts(App* app, CPU& cpu, reg32 first, reg32 second);
// The game's 0x4f7280: a game hosted over a network: edx the player's address, ebx what it is opened with.
reg32 tcpHostGame(App* app, CPU& cpu, reg32 net, reg32 player, reg32 address, reg32 gameName, reg32 playerName);
// The game's 0x4f7320: the game of a NET object ended: its players told, its game closed.
reg32 tcpEndGame(App* app, CPU& cpu, reg32 net);
// The game's 0x4f7370: what the kinds' tables hold for a function a kind has no need of.
reg32 returnZero(App* app, CPU& cpu);
// The game's 0x4f7380: a network over TCP: sub_4f5130 with this kind's table at 0x566e8c.
reg32 tcpOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 first, reg32 second);
// The game's 0x4f73a0: Winsock started for one more user, a socket's limit read once.
reg32 tcpStartWinsock(App* app, CPU& cpu);
// The game's 0x4f7420: one user of Winsock less, stopped with the last.
reg32 tcpStopWinsock(App* app, CPU& cpu);
// The game's 0x4f9a50: the modem of an index chosen, the list made first if it has not been.
reg32 tapiSelectModem(App* app, CPU& cpu, reg32 index);
// The game's 0x4f9ac0: TapiEnumerateModems: the list of the lines that are modems, 20 places of a device and its name.
reg32 tapiEnumerateModems(App* app, CPU& cpu);
// The game's 0x4f9d20: the chosen line opened, its comm's settings seen to when asked for.
reg32 tapiOpenLine(App* app, CPU& cpu);
// The game's 0x4f9df0: the line's comm port asked for, its DCB read into 0x9f3d60 and the handle closed.
reg32 tapiGetCommHandleConfig(App* app, CPU& cpu);
// The game's 0x4f9ea0: the chosen device's configuration read, its low three bits of settings cleared, and set again.
reg32 tapiSetCommConfig(App* app, CPU& cpu);
// The game's 0x4f9f30: TapiGetDevCaps: a device's capabilities in [0x9f4d88].
reg32 tapiGetDevCaps(App* app, CPU& cpu, reg32 device);
// The game's 0x4fa030: TapiMakeCall: a number dialled on the open line.
reg32 tapiMakeCall(App* app, CPU& cpu, reg32 number);
// The game's 0x4fa0f0: TapiGetModemHandle: the comm handle of the call in [0x9f4da8].
reg32 tapiGetModemHandle(App* app, CPU& cpu);
// The game's 0x4fa1d0: the call dropped.
reg32 tapiHangUp(App* app, CPU& cpu);
reg32 tapiHangUpFrom(App* app, CPU& cpu, reg32 handed);  // the same, the eax it is entered with
// The game's 0x4fa210: the modem's thread: the modem chosen, TAPI set up, the number dialled.
reg32 tapiThread(App* app, CPU& cpu);
// The game's 0x4fa310: a modem connection opened: the thread started and waited for.
reg32 modemOpen(App* app, CPU& cpu, reg32 settings, reg32 callback);
// The game's 0x4fa450: a modem connection closed: the thread told.
reg32 modemClose(App* app, CPU& cpu, reg32 settings);
// The game's 0x4fa480: a modem connection polled: told to close unless connected.
reg32 modemPoll(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa4e0: whether the modem's port will take more.
reg32 modemSendReady(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa4f0: whether the modem's port has something.
reg32 modemReceiveReady(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa500: bytes sent over the modem.
reg32 modemSend(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer);
// The game's 0x4fa530: bytes received over the modem.
reg32 modemReceive(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer);
// The game's 0x4fa550: a network over the modem: sub_4f5130 with this kind's table at 0x5671b0.
reg32 modemOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 second, reg32 mode);
// The game's 0x4fa5f0: a modem's name by its index, 0 for none.
reg32 modemName(App* app, CPU& cpu, reg32 index);
// The game's 0x4fa620: a connection over a serial port opened, with the speed and so on in its settings.
reg32 serialOpen(App* app, CPU& cpu, reg32 settings, reg32 callback);
// The game's 0x4fa6d0: a serial connection closed: nothing but that it is.
reg32 serialClose(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa6e0: a serial connection polled: the callback with what has come.
reg32 serialPoll(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa710: whether the serial port will take more.
reg32 serialSendReady(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa720: whether the serial port has something.
reg32 serialReceiveReady(App* app, CPU& cpu, reg32 object);
// The game's 0x4fa730: bytes sent over the serial port.
reg32 serialSend(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer);
// The game's 0x4fa760: bytes received over the serial port.
reg32 serialReceive(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer);
// The game's 0x4fa780: a network over a serial port: sub_4f5130 with this kind's table at 0x5671d0.
reg32 serialOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 second, reg32 mode);
// The game's 0x5127d0: a broadcast asking for games sent on a socket: IPX's address, all ones.
reg32 ipxBroadcastOpen(App* app, CPU& cpu, reg32 socket);
// The game's 0x512890: a list of games read from a packet: 64 bytes each after two of a version.
reg32 ipxScanInterfaces(App* app, CPU& cpu, reg32 owner, reg32 header, reg32 data, reg32 size);
// The game's 0x512940: the game's advertisement sent to an address.
reg32 ipxAdvertiseTo(App* app, CPU& cpu, reg32 to);
// The game's 0x512980: whether a packet asks for games: type 1, this game's protocol or any.
reg32 ipxIsRequest(App* app, CPU& cpu, reg32 data);
// The game's 0x5129e0: an IPX socket for the advertising thread, broadcast and larger buffers on.
reg32 ipxServerSocket(App* app, CPU& cpu, reg32 address);
// The game's 0x512ad0: the advertising thread: it answers requests that come, and sends the game to all every minute.
reg32 ipxServerThread(App* app, CPU& cpu);
// The game's 0x512c80: the game advertised: the socket, the packet of its name and id, and the thread, waited for.
reg32 ipxStartServer(App* app, CPU& cpu, reg32 owner, reg32 id, reg32 name);
// The game's 0x512d70: the advertising stopped: the socket closed and the thread waited for.
reg32 ipxStopServer(App* app, CPU& cpu);
// The game's 0x512f60: a game's GUID as the text that names its shared memory.
reg32 gameGuidText(App* app, CPU& cpu, reg32 guid, reg32 text);
// The game's 0x512fc0: when an answer stops being waited for: eax ticks from now.
reg32 setResponseTimeout(App* app, CPU& cpu, reg32 ticks);
// The game's 0x512fd0: whether that time has come.
reg32 responseTimedOut(App* app, CPU& cpu);
// The game's 0x512ff0: what a table holds for a function that always says no.
reg32 returnZeroToo(App* app, CPU& cpu);
// The game's 0x513190: a request sent to the session's server and waited for, sent again each time round: eax the game.
reg32 sendServerRequest(App* app, CPU& cpu, reg32 game, reg32 session, reg32 type, reg32 message, reg32 length, reg32 patience);
// The game's 0x4f6860: nothing to do for an exception on a socket.
reg32 tcpExceptionSeen(App* app, CPU& cpu);

}  // namespace netb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
