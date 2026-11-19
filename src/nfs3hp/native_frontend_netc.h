#ifndef NFS3HP_NATIVE_FRONTEND_NETC_H
#define NFS3HP_NATIVE_FRONTEND_NETC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_netc.cpp's typed functions (the last of EA's network library: the gathering's
 * server side, the serial ports and their thread with the modem's talk, the two packet layers over
 * a stream), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each takes its arguments into the registers the
 * game's function takes them in and runs as the code does, calling the game's other functions by
 * address on `ts`; it throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace netc
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x5140e0: the server's pulse for a gathering's connection: adverts, alives, drops; 1.
u32 gatherPulse(App* app, ThreadState& ts, u32 self);
// The game's 0x514260: every gathering's connection pulsed, by the queue of them.
u32 gatherPulseStart(App* app, ThreadState& ts);
// The game's 0x5143d0: a transport's address lookup waited for, its request's record on the stack.
u32 gatherRequest(App* app, ThreadState& ts, u32 self, u32 start, u32 wanted, u32 last);
// The game's 0x5144b0: a player's game joined; what the server answered.
u32 gatherJoin(App* app, ThreadState& ts, u32 self, u32 player, u32 name);
// The game's 0x5145b0: a game's information asked of its server into `out`; 1 if it answered.
u32 gatherGameInfo(App* app, ThreadState& ts, u32 self, u32 player, u32 out);
// The game's 0x514650: a new game served: its record made, the transport listening; 1 if so.
u32 gatherNewGame(App* app, ThreadState& ts, u32 self, u32 info, u32 name, u32 user);
// The game's 0x514820: the served game's information changed; 1, or 0 with no game.
u32 gatherSetGameInfo(App* app, ThreadState& ts, u32 self, u32 info);
// The game's 0x514850: the served game no longer listened for, its record gone with `discard`.
u32 gatherCloseGame(App* app, ThreadState& ts, u32 self, u32 discard);
// The game's 0x5148b0: a closed game listened for again.
u32 gatherReopenGame(App* app, ThreadState& ts, u32 self);
// The game's 0x514970: leaving: the served game told goodbye and thrown away, or our server told; 1.
u32 gatherLeave(App* app, ThreadState& ts, u32 self);
// The game's 0x514af0: the players of a game, each handed to `callback`.
u32 gatherEnumPlayers(App* app, ThreadState& ts, u32 self, u32 player, u32 callback);
// The game's 0x514dc0: a player thrown out of our game; 0.
u32 gatherKillPlayer(App* app, ThreadState& ts, u32 self, u32 player);
// The game's 0x514ec0: a packet connection opened over one of the two layers (`first` in eax not read).
u32 gatherOpenConnection(App* app, ThreadState& ts, u32 first, u32 connection, u32 wanted, u32 window);
// The game's 0x514f70: what came for a request in progress handed to it; 1 if it was data.
u32 gatherReceive(App* app, ThreadState& ts, u32 self, u32 from, u32 kind, u32 length, u32 extraArgument,
                    u32 acceptedArgument);
// The game's 0x515050: rasapi32.dll and its two functions found, once; 1 if they are there.
u32 loadRasApi(App* app, ThreadState& ts);
// The game's 0x51a470: a modem's answer read until a word of the table or a number; its number.
u32 modemResponse(App* app, ThreadState& ts, u32 number, u32 timeout);
// The game's 0x51a7b0: a command said to the modem, its echo waited for; 1 if it came.
u32 modemCommand(App* app, ThreadState& ts, u32 number, u32 command, u32 drop);
// The game's 0x51a930: the modem's attention got; 1 if it did.
u32 modemAttention(App* app, ThreadState& ts, u32 number);
// The game's 0x51ab40: at the game's end, every port still open closed.
// `handed` the eax the code hands it, given back when no port was open.
u32 serialCloseAll(App* app, ThreadState& ts, u32 handed);
// The game's 0x51ab70: a command left for a port's thread, which is woken.
u32 serialCommand(App* app, ThreadState& ts, u32 number, u32 command);
// The game's 0x51abc0: the port's reads started until its ring is full, then its writes.
u32 serialStartIo(App* app, ThreadState& ts, u32 record);
// The game's 0x51ad70: the port's reads and writes under way looked at, those done taken in.
u32 serialFinishIo(App* app, ThreadState& ts, u32 record);
// The game's 0x51aeb0: the ports' thread's wait for its command event and every port's work.
u32 serialWaitEvents(App* app, ThreadState& ts);
// The game's 0x51af10: the ports' thread, while any port is open.
u32 serialThread(App* app, ThreadState& ts);
// The game's 0x51b030: whether a port's number is one that is open.
u32 serialIsOpen(App* app, ThreadState& ts, u32 number);
// The game's 0x51b1c0: what a port has received thrown away, by its thread.
u32 serialDropInput(App* app, ThreadState& ts, u32 number);
// The game's 0x51b1e0: what a port has yet to send thrown away, by its thread.
u32 serialDropOutput(App* app, ThreadState& ts, u32 number);
// The game's 0x51b200: a port closed; with the last of them the thread's lock goes too.
u32 serialClose(App* app, ThreadState& ts, u32 number);
// The game's 0x51b2f0: a port made of a handle already open; its number, or -1.
u32 serialOpenHandle(App* app, ThreadState& ts, u32 number, u32 handle);
// The game's 0x51b480: a port opened by its number; its number, -1 if it would not.
u32 serialOpen(App* app, ThreadState& ts, u32 number);
// The game's 0x51b530: data written to a port, if all of it fits; what went.
u32 serialWriteWhole(App* app, ThreadState& ts, u32 number, u32 length, u32 data);
// The game's 0x51b5f0: as much data as there is room for put in a port's ring; what went.
u32 serialWrite(App* app, ThreadState& ts, u32 number, u32 length, u32 data);
// The game's 0x51b710: a port's speed, parity, stop bits and word's length.
u32 serialSet(App* app, ThreadState& ts, u32 number, u32 speed, u32 parity, u32 stopBits, u32 wordBits);
// The game's 0x51b8a0: how much a port has received.
u32 serialInCount(App* app, ThreadState& ts, u32 number);
// The game's 0x51b900: how much room a port has to send.
u32 serialOutRoom(App* app, ThreadState& ts, u32 number);
// The game's 0x51b970: what a port has received copied out, left in the ring; how much.
u32 serialPeek(App* app, ThreadState& ts, u32 number, u32 length, u32 out);
// The game's 0x51ba50: what a port has received taken out of the ring; how much.
u32 serialRead(App* app, ThreadState& ts, u32 number, u32 length, u32 out);
// The game's 0x51bbd0: what is wanted from a port read once it is there or the time is up.
u32 serialWaitIn(App* app, ThreadState& ts, u32 number, u32 length, u32 out, u32 timeout);
// The game's 0x51bc70: an unreliable frame's checksum stamped.
u32 frameChecksum(App* app, ThreadState& ts, u32 frame, u32 length);
// The game's 0x51bca0: whether an unreliable frame's checksum holds.
u32 frameChecksumGood(App* app, ThreadState& ts, u32 frame, u32 length);
// The game's 0x51bcd0: an unreliable frame's start and data.
u32 frameBuild(App* app, ThreadState& ts, u32 out, u32 self, u32 data, u32 length);
// The game's 0x51bcf0: an unreliable frame written if the stream has room; 1 if all of it went.
u32 frameSend(App* app, ThreadState& ts, u32 out, u32 self);
// The game's 0x51bd50: a frame of no data, its length the `code`, written if the stream has room.
u32 frameSendControl(App* app, ThreadState& ts, u32 self, u32 code);
// The game's 0x51bdc0: the unreliable layer's connection closed.
u32 unreliableShutdown(App* app, ThreadState& ts, u32 self);
// The game's 0x51be90: the first frame in what the stream gave, delivered or skipped; what was used.
u32 unreliableReceiveFrame(App* app, ThreadState& ts, u32 self, u32 counted, u32 buffer, u32 length);
// The game's 0x51bfd0: all that came for the unreliable layer.
u32 unreliableReceive(App* app, ThreadState& ts, u32 self);
// The game's 0x51c0a0: the stream says something came: read while there is.
u32 unreliableNotify(App* app, ThreadState& ts, u32 connection);
// The game's 0x51c100: a connection opened on the unreliable layer; 1 if it stands.
u32 unreliableOpen(App* app, ThreadState& ts, u32 connection, u32 ops, u32 asked);
// The game's 0x51c510: data sent on the unreliable layer, made a frame on the stack.
u32 unreliableSend(App* app, ThreadState& ts, u32 connection, u32 data, u32 length);
// The game's 0x51c5c0: the unreliable layer's turn; 1.
u32 unreliableProcess(App* app, ThreadState& ts, u32 connection);
// The game's 0x51c690: the unreliable layer's connection closed by us; 1 if it was there.
u32 unreliableClose(App* app, ThreadState& ts, u32 connection);
// The game's 0x51c7a0: a number of few bits widened, near what is expected.
u32 sequenceWiden(App* app, ThreadState& ts, u32 expected, u32 low, u32 mask);
// The game's 0x51c7c0: a reliable frame's checksum stamped.
u32 packetChecksum(App* app, ThreadState& ts, u32 frame, u32 length);
// The game's 0x51c7f0: whether a reliable frame's checksum holds.
u32 packetChecksumGood(App* app, ThreadState& ts, u32 frame, u32 length);
// The game's 0x51c820: the number to acknowledge in what is sent now.
u32 packetAckNumber(App* app, ThreadState& ts, u32 self);
// The game's 0x51c890: a packet numbered and its frame's start and data made.
u32 packetBuild(App* app, ThreadState& ts, u32 out, u32 self, u32 data, u32 length);
// The game's 0x51c8f0: a packet written if the stream has room, its resend time started; 1 if it went.
u32 packetSend(App* app, ThreadState& ts, u32 out, u32 self);
// The game's 0x51ca50: a frame of no data, its number the `code`; 1 if it went.
u32 packetSendControl(App* app, ThreadState& ts, u32 control, u32 self, u32 code);
// The game's 0x51caf0: a packet that came out of order asked for again, when its time has come.
u32 packetSendNak(App* app, ThreadState& ts, u32 waiting, u32 self);
// The game's 0x51cb50: what the other end acknowledged taken off those waiting.
u32 packetGotAck(App* app, ThreadState& ts, u32 self, u32 ack);
// The game's 0x51cc00: the other end wants a packet again: its time to resend is now.
u32 packetGotNak(App* app, ThreadState& ts, u32 self, u32 sequence);
// The game's 0x51cc80: the reliable layer's connection closed.
u32 packetShutdown(App* app, ThreadState& ts, u32 self);
// The game's 0x51cd50: the first frame in what the stream gave, seen to; what was used.
u32 packetReceiveFrame(App* app, ThreadState& ts, u32 self, u32 counted, u32 buffer, u32 length);
// The game's 0x51cf30: all that came for the reliable layer.
u32 packetReceive(App* app, ThreadState& ts, u32 self);
// The game's 0x51d000: what the reliable layer's queues keep packets in the order of.
u32 packetQueueKey(App* app, ThreadState& ts, u32 packet);
// The game's 0x51d010: a packet's time to resend counted down; 1 when it is up.
u32 packetCountDown(App* app, ThreadState& ts, u32 waiting);
// The game's 0x51d040: a packet sent again if its time is up, else 1.
u32 packetResendDue(App* app, ThreadState& ts, u32 waiting, u32 self);
// The game's 0x51d050: the stream says something came: read, delivered in order, a gap asked for.
u32 packetNotify(App* app, ThreadState& ts, u32 connection);
// The game's 0x51d270: a connection opened on the reliable layer; 1 if it stands.
u32 packetOpen(App* app, ThreadState& ts, u32 connection, u32 ops, u32 asked, u32 windowBits);
// The game's 0x51d810: data sent on the reliable layer and kept until acknowledged.
u32 packetSubmit(App* app, ThreadState& ts, u32 connection, u32 data, u32 length);
// The game's 0x51d8f0: the reliable layer's turn; 1.
u32 packetProcess(App* app, ThreadState& ts, u32 connection);
// The game's 0x51d9e0: the reliable layer's connection closed by us; 1 if it was there.
u32 packetClose(App* app, ThreadState& ts, u32 connection);

}  // namespace netc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
