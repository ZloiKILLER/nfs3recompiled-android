#ifndef NFS3HP_NATIVE_FRONTEND_NETB_H
#define NFS3HP_NATIVE_FRONTEND_NETB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_netb.cpp's typed functions (EA's network library: TCP, IPX, TAPI, the queues,
 * LAYER and the sessions' messages), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace netb
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x512180: a line in the network's log, a ring of 64-byte records at [0x5689d0]: the time.
u32 commEvent(App* app, ThreadState& ts, u32 text, u32 what, u32 data, u32 bytes, u32 flagged, u32 kindWord);
// The game's 0x513000: how many bits a word has set.
u32 countBits(App* app, ThreadState& ts, u32 value);
// The game's 0x513030: this game's description sent to an address: eax the game, edx the address.
u32 sendGameInfo(App* app, ThreadState& ts, u32 game, u32 to, u32 id);
// The game's 0x5132a0: the player of a game at an address, his time last heard from set.
u32 findPlayerByAddress(App* app, ThreadState& ts, u32 game, u32 address);
// The game's 0x513320: a player's record taken into a game's first empty place.
u32 addNewPlayer(App* app, ThreadState& ts, u32 game, u32 record);
// The game's 0x5133a0: a session message from an address.
u32 sessionMessage(App* app, ThreadState& ts, u32 session, u32 from, u32 type, u32 message, u32 length);
// The game's 0x50dcd0: qinit: a queue with a lock of its own, eax it, edx its priority function or 0 for the default.
u32 qinit(App* app, ThreadState& ts, u32 queue, u32 priority, u32 argument);
// The game's 0x50dd60: qinitpeer: a queue that shares the lock of the queue in ecx.
u32 qinitpeer(App* app, ThreadState& ts, u32 queue, u32 priority, u32 argument, u32 peer);
// The game's 0x50de50: qpush: an element at the head.
u32 qpush(App* app, ThreadState& ts, u32 queue, u32 element);
// The game's 0x50ded0: qbottom: an element at the tail.
u32 qbottom(App* app, ThreadState& ts, u32 queue, u32 element);
// The game's 0x50df60: qpop: the element at the head, taken off.
u32 qpop(App* app, ThreadState& ts, u32 queue);
// The game's 0x50dff0: qpoplast: the element at the tail, taken off.
u32 qpoplast(App* app, ThreadState& ts, u32 queue);
// The game's 0x50e0a0: qinsert: an element in the order of the queue's priority function.
u32 qinsert(App* app, ThreadState& ts, u32 queue, u32 element);
// The game's 0x50e2d0: qlowpriority: the priority of the element at the head, -1 for none.
u32 qlowpriority(App* app, ThreadState& ts, u32 queue);
// The game's 0x50e3f0: qfind: the first element a function says yes to, edx it, ebx its argument.
u32 qfind(App* app, ThreadState& ts, u32 queue, u32 match, u32 argument);
// The game's 0x50e500: qfindpriority: the element of exactly a priority, edx it.
u32 qfindpriority(App* app, ThreadState& ts, u32 queue, u32 priority);
// The game's 0x50e8e0: qiterate: a function on each element until the queue changes, outside the lock.
u32 qiterate(App* app, ThreadState& ts, u32 queue, u32 function, u32 argument);
// The game's 0x50e9b0: qsafeiterate: qiterate with the lock held all along.
u32 qsafeiterate(App* app, ThreadState& ts, u32 queue, u32 function, u32 argument);
// The game's 0x512470: under the last layer: the pieces -- edx of them at ebx, {length.
u32 layerReceiveLast(App* app, ThreadState& ts, u32 connection, u32 pieceCount, u32 pieces);
// The game's 0x5125b0: to the layer of an index in the queue of layers on the stack, or past the last.
u32 layerCallUnpacker(App* app, ThreadState& ts, u32 first, u32 index, u32 second, u32 pieces, u32 layers);
// The game's 0x512650: from the top of a connection's layers: ecx them, edx the pieces' count, ebx them.
u32 layerReceiveFromTop(App* app, ThreadState& ts, u32 first, u32 count, u32 pieces, u32 layers);
// The game's 0x512660: CONN_initfilter: a connection's queue of layers.
u32 connInitFilter(App* app, ThreadState& ts, u32 connection);
// The game's 0x5126f0: a packet received on a connection, edx it, ebx how long: one piece for the layers.
u32 layerReceive(App* app, ThreadState& ts, u32 connection, u32 packet, u32 length);
// The game's 0x4f6350: what is queued for an address flushed: edx a packet whose first 20 bytes are the address, or 0.
u32 tcpFlushQueue(App* app, ThreadState& ts, u32 owner, u32 packet, u32 rest);
// The game's 0x4f63d0: IPX's kind of network, once: Winsock tried, and the list told if it will do.
u32 ipxOpenWinsock(App* app, ThreadState& ts, u32 remember);
// The game's 0x4f7830: the same for TCP: Windows has a network, Winsock starts and a socket can be made.
u32 tcpOpenWinsock(App* app, ThreadState& ts, u32 remember);
// The game's 0x4f6490: a TCP socket bound to a port, -1 for none.
u32 tcpListenSocket(App* app, ThreadState& ts, u32 port);
// The game's 0x4f6510: a socket made non-blocking, reusable and without Nagle's delay.
u32 tcpSetSocketOptions(App* app, ThreadState& ts, u32 socket);
// The game's 0x4f6570: one connection into select's three sets, for qsafeiterate: edx the frame of sub_4f68c0.
u32 tcpFillSelectSets(App* app, ThreadState& ts, u32 connection, u32 sets);
// The game's 0x4f6640: a packet read from a connection's socket when it has all come.
u32 tcpReceivePacket(App* app, ThreadState& ts, u32 listener, u32 connection);
// The game's 0x4f67b0: what is left of a connection's packet sent.
u32 tcpSendPending(App* app, ThreadState& ts, u32 connection);
// The game's 0x4f6870: a dead connection closed and off the open queue, told to its owner.
u32 tcpConnectionDropped(App* app, ThreadState& ts, u32 connection);
// The game's 0x4f68c0: the TCP thread: selects over the listening socket.
u32 tcpServerThread(App* app, ThreadState& ts);
// The game's 0x4f6c50: a connection opened over TCP: with no address, by waiting for someone to connect to the thread.
u32 tcpOpenConnection(App* app, ThreadState& ts, u32 connection, u32 room);
// The game's 0x4f7030: a packet sent on a connection: its length in network order before it.
u32 tcpSendConnection(App* app, ThreadState& ts, u32 connection, u32 data, u32 length);
// The game's 0x4f71a0: a connection closed for good and its buffer freed if it was its own.
u32 tcpCloseConnection(App* app, ThreadState& ts, u32 connection);
// The game's 0x4f7220: whether two addresses are the same: a dword, two words and 8 bytes.
u32 tcpSameAddressParts(App* app, ThreadState& ts, u32 first, u32 second);
// The game's 0x4f7280: a game hosted over a network: edx the player's address, ebx what it is opened with.
u32 tcpHostGame(App* app, ThreadState& ts, u32 net, u32 player, u32 address, u32 gameName, u32 playerName);
// The game's 0x4f7320: the game of a NET object ended: its players told, its game closed.
u32 tcpEndGame(App* app, ThreadState& ts, u32 net);
// The game's 0x4f7370: what the kinds' tables hold for a function a kind has no need of.
u32 returnZero(App* app, ThreadState& ts);
// The game's 0x4f7380: a network over TCP: sub_4f5130 with this kind's table at 0x566e8c.
u32 tcpOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 first, u32 second);
// The game's 0x4f73a0: Winsock started for one more user, a socket's limit read once.
u32 tcpStartWinsock(App* app, ThreadState& ts);
// The game's 0x4f7420: one user of Winsock less, stopped with the last.
u32 tcpStopWinsock(App* app, ThreadState& ts);
// The game's 0x4f9a50: the modem of an index chosen, the list made first if it has not been.
u32 tapiSelectModem(App* app, ThreadState& ts, u32 index);
// The game's 0x4f9ac0: TapiEnumerateModems: the list of the lines that are modems, 20 places of a device and its name.
u32 tapiEnumerateModems(App* app, ThreadState& ts);
// The game's 0x4f9d20: the chosen line opened, its comm's settings seen to when asked for.
u32 tapiOpenLine(App* app, ThreadState& ts);
// The game's 0x4f9df0: the line's comm port asked for, its DCB read into 0x9f3d60 and the handle closed.
u32 tapiGetCommHandleConfig(App* app, ThreadState& ts);
// The game's 0x4f9ea0: the chosen device's configuration read, its low three bits of settings cleared, and set again.
u32 tapiSetCommConfig(App* app, ThreadState& ts);
// The game's 0x4f9f30: TapiGetDevCaps: a device's capabilities in [0x9f4d88].
u32 tapiGetDevCaps(App* app, ThreadState& ts, u32 device);
// The game's 0x4fa030: TapiMakeCall: a number dialled on the open line.
u32 tapiMakeCall(App* app, ThreadState& ts, u32 number);
// The game's 0x4fa0f0: TapiGetModemHandle: the comm handle of the call in [0x9f4da8].
u32 tapiGetModemHandle(App* app, ThreadState& ts);
// The game's 0x4fa1d0: the call dropped.
u32 tapiHangUp(App* app, ThreadState& ts);
u32 tapiHangUpFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4fa210: the modem's thread: the modem chosen, TAPI set up, the number dialled.
u32 tapiThread(App* app, ThreadState& ts);
// The game's 0x4fa310: a modem connection opened: the thread started and waited for.
u32 modemOpen(App* app, ThreadState& ts, u32 settings, u32 callback);
// The game's 0x4fa450: a modem connection closed: the thread told.
u32 modemClose(App* app, ThreadState& ts, u32 settings);
// The game's 0x4fa480: a modem connection polled: told to close unless connected.
u32 modemPoll(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa4e0: whether the modem's port will take more.
u32 modemSendReady(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa4f0: whether the modem's port has something.
u32 modemReceiveReady(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa500: bytes sent over the modem.
u32 modemSend(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer);
// The game's 0x4fa530: bytes received over the modem.
u32 modemReceive(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer);
// The game's 0x4fa550: a network over the modem: sub_4f5130 with this kind's table at 0x5671b0.
u32 modemOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 second, u32 mode);
// The game's 0x4fa5f0: a modem's name by its index, 0 for none.
u32 modemName(App* app, ThreadState& ts, u32 index);
// The game's 0x4fa620: a connection over a serial port opened, with the speed and so on in its settings.
u32 serialOpen(App* app, ThreadState& ts, u32 settings, u32 callback);
// The game's 0x4fa6d0: a serial connection closed: nothing but that it is.
u32 serialClose(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa6e0: a serial connection polled: the callback with what has come.
u32 serialPoll(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa710: whether the serial port will take more.
u32 serialSendReady(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa720: whether the serial port has something.
u32 serialReceiveReady(App* app, ThreadState& ts, u32 object);
// The game's 0x4fa730: bytes sent over the serial port.
u32 serialSend(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer);
// The game's 0x4fa760: bytes received over the serial port.
u32 serialReceive(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer);
// The game's 0x4fa780: a network over a serial port: sub_4f5130 with this kind's table at 0x5671d0.
u32 serialOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 second, u32 mode);
// The game's 0x5127d0: a broadcast asking for games sent on a socket: IPX's address, all ones.
u32 ipxBroadcastOpen(App* app, ThreadState& ts, u32 socket);
// The game's 0x512890: a list of games read from a packet: 64 bytes each after two of a version.
u32 ipxScanInterfaces(App* app, ThreadState& ts, u32 owner, u32 header, u32 data, u32 size);
// The game's 0x512940: the game's advertisement sent to an address.
u32 ipxAdvertiseTo(App* app, ThreadState& ts, u32 to);
// The game's 0x512980: whether a packet asks for games: type 1, this game's protocol or any.
u32 ipxIsRequest(App* app, ThreadState& ts, u32 data);
// The game's 0x5129e0: an IPX socket for the advertising thread, broadcast and larger buffers on.
u32 ipxServerSocket(App* app, ThreadState& ts, u32 address);
// The game's 0x512ad0: the advertising thread: it answers requests that come, and sends the game to all every minute.
u32 ipxServerThread(App* app, ThreadState& ts);
// The game's 0x512c80: the game advertised: the socket, the packet of its name and id, and the thread, waited for.
u32 ipxStartServer(App* app, ThreadState& ts, u32 owner, u32 id, u32 name);
// The game's 0x512d70: the advertising stopped: the socket closed and the thread waited for.
u32 ipxStopServer(App* app, ThreadState& ts);
// The game's 0x512f60: a game's GUID as the text that names its shared memory.
u32 gameGuidText(App* app, ThreadState& ts, u32 guid, u32 text);
// The game's 0x512fc0: when an answer stops being waited for: eax ticks from now.
u32 setResponseTimeout(App* app, ThreadState& ts, u32 ticks);
// The game's 0x512fd0: whether that time has come.
u32 responseTimedOut(App* app, ThreadState& ts);
// The game's 0x512ff0: what a table holds for a function that always says no.
u32 returnZeroToo(App* app, ThreadState& ts);
// The game's 0x513190: a request sent to the session's server and waited for, sent again each time round: eax the game.
u32 sendServerRequest(App* app, ThreadState& ts, u32 game, u32 session, u32 type, u32 message, u32 length, u32 patience);
// The game's 0x4f6860: nothing to do for an exception on a socket.
u32 tcpExceptionSeen(App* app, ThreadState& ts);

}  // namespace netb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
