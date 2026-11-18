#ifndef NFS3HP_NATIVE_FRONTEND_NETC_H
#define NFS3HP_NATIVE_FRONTEND_NETC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_netc.cpp's typed functions (the last of EA's network library: the gathering's
 * server side, the serial ports and their thread with the modem's talk, the two packet layers over
 * a stream), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each takes its arguments into the registers the
 * game's function takes them in and runs as the code does, calling the game's other functions by
 * address on `cpu`; it throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace netc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x5140e0: the server's pulse for a gathering's connection: adverts, alives, drops; 1.
reg32 gatherPulse(App* app, CPU& cpu, reg32 self);
// The game's 0x514260: every gathering's connection pulsed, by the queue of them.
reg32 gatherPulseStart(App* app, CPU& cpu);
// The game's 0x5143d0: a transport's address lookup waited for, its request's record on the stack.
reg32 gatherRequest(App* app, CPU& cpu, reg32 self, reg32 start, reg32 wanted, reg32 last);
// The game's 0x5144b0: a player's game joined; what the server answered.
reg32 gatherJoin(App* app, CPU& cpu, reg32 self, reg32 player, reg32 name);
// The game's 0x5145b0: a game's information asked of its server into `out`; 1 if it answered.
reg32 gatherGameInfo(App* app, CPU& cpu, reg32 self, reg32 player, reg32 out);
// The game's 0x514650: a new game served: its record made, the transport listening; 1 if so.
reg32 gatherNewGame(App* app, CPU& cpu, reg32 self, reg32 info, reg32 name, reg32 user);
// The game's 0x514820: the served game's information changed; 1, or 0 with no game.
reg32 gatherSetGameInfo(App* app, CPU& cpu, reg32 self, reg32 info);
// The game's 0x514850: the served game no longer listened for, its record gone with `discard`.
reg32 gatherCloseGame(App* app, CPU& cpu, reg32 self, reg32 discard);
// The game's 0x5148b0: a closed game listened for again.
reg32 gatherReopenGame(App* app, CPU& cpu, reg32 self);
// The game's 0x514970: leaving: the served game told goodbye and thrown away, or our server told; 1.
reg32 gatherLeave(App* app, CPU& cpu, reg32 self);
// The game's 0x514af0: the players of a game, each handed to `callback`.
reg32 gatherEnumPlayers(App* app, CPU& cpu, reg32 self, reg32 player, reg32 callback);
// The game's 0x514dc0: a player thrown out of our game; 0.
reg32 gatherKillPlayer(App* app, CPU& cpu, reg32 self, reg32 player);
// The game's 0x514ec0: a packet connection opened over one of the two layers (`first` in eax not read).
reg32 gatherOpenConnection(App* app, CPU& cpu, reg32 first, reg32 connection, reg32 wanted, reg32 window);
// The game's 0x514f70: what came for a request in progress handed to it; 1 if it was data.
reg32 gatherReceive(App* app, CPU& cpu, reg32 self, reg32 from, reg32 kind, reg32 length, reg32 extraArgument,
                    reg32 acceptedArgument);
// The game's 0x515050: rasapi32.dll and its two functions found, once; 1 if they are there.
reg32 loadRasApi(App* app, CPU& cpu);
// The game's 0x51a470: a modem's answer read until a word of the table or a number; its number.
reg32 modemResponse(App* app, CPU& cpu, reg32 number, reg32 timeout);
// The game's 0x51a7b0: a command said to the modem, its echo waited for; 1 if it came.
reg32 modemCommand(App* app, CPU& cpu, reg32 number, reg32 command, reg32 drop);
// The game's 0x51a930: the modem's attention got; 1 if it did.
reg32 modemAttention(App* app, CPU& cpu, reg32 number);
// The game's 0x51ab40: at the game's end, every port still open closed.
// `handed` the eax the code hands it, given back when no port was open.
reg32 serialCloseAll(App* app, CPU& cpu, reg32 handed);
// The game's 0x51ab70: a command left for a port's thread, which is woken.
reg32 serialCommand(App* app, CPU& cpu, reg32 number, reg32 command);
// The game's 0x51abc0: the port's reads started until its ring is full, then its writes.
reg32 serialStartIo(App* app, CPU& cpu, reg32 record);
// The game's 0x51ad70: the port's reads and writes under way looked at, those done taken in.
reg32 serialFinishIo(App* app, CPU& cpu, reg32 record);
// The game's 0x51aeb0: the ports' thread's wait for its command event and every port's work.
reg32 serialWaitEvents(App* app, CPU& cpu);
// The game's 0x51af10: the ports' thread, while any port is open.
reg32 serialThread(App* app, CPU& cpu);
// The game's 0x51b030: whether a port's number is one that is open.
reg32 serialIsOpen(App* app, CPU& cpu, reg32 number);
// The game's 0x51b1c0: what a port has received thrown away, by its thread.
reg32 serialDropInput(App* app, CPU& cpu, reg32 number);
// The game's 0x51b1e0: what a port has yet to send thrown away, by its thread.
reg32 serialDropOutput(App* app, CPU& cpu, reg32 number);
// The game's 0x51b200: a port closed; with the last of them the thread's lock goes too.
reg32 serialClose(App* app, CPU& cpu, reg32 number);
// The game's 0x51b2f0: a port made of a handle already open; its number, or -1.
reg32 serialOpenHandle(App* app, CPU& cpu, reg32 number, reg32 handle);
// The game's 0x51b480: a port opened by its number; its number, -1 if it would not.
reg32 serialOpen(App* app, CPU& cpu, reg32 number);
// The game's 0x51b530: data written to a port, if all of it fits; what went.
reg32 serialWriteWhole(App* app, CPU& cpu, reg32 number, reg32 length, reg32 data);
// The game's 0x51b5f0: as much data as there is room for put in a port's ring; what went.
reg32 serialWrite(App* app, CPU& cpu, reg32 number, reg32 length, reg32 data);
// The game's 0x51b710: a port's speed, parity, stop bits and word's length.
reg32 serialSet(App* app, CPU& cpu, reg32 number, reg32 speed, reg32 parity, reg32 stopBits, reg32 wordBits);
// The game's 0x51b8a0: how much a port has received.
reg32 serialInCount(App* app, CPU& cpu, reg32 number);
// The game's 0x51b900: how much room a port has to send.
reg32 serialOutRoom(App* app, CPU& cpu, reg32 number);
// The game's 0x51b970: what a port has received copied out, left in the ring; how much.
reg32 serialPeek(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out);
// The game's 0x51ba50: what a port has received taken out of the ring; how much.
reg32 serialRead(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out);
// The game's 0x51bbd0: what is wanted from a port read once it is there or the time is up.
reg32 serialWaitIn(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out, reg32 timeout);
// The game's 0x51bc70: an unreliable frame's checksum stamped.
reg32 frameChecksum(App* app, CPU& cpu, reg32 frame, reg32 length);
// The game's 0x51bca0: whether an unreliable frame's checksum holds.
reg32 frameChecksumGood(App* app, CPU& cpu, reg32 frame, reg32 length);
// The game's 0x51bcd0: an unreliable frame's start and data.
reg32 frameBuild(App* app, CPU& cpu, reg32 out, reg32 self, reg32 data, reg32 length);
// The game's 0x51bcf0: an unreliable frame written if the stream has room; 1 if all of it went.
reg32 frameSend(App* app, CPU& cpu, reg32 out, reg32 self);
// The game's 0x51bd50: a frame of no data, its length the `code`, written if the stream has room.
reg32 frameSendControl(App* app, CPU& cpu, reg32 self, reg32 code);
// The game's 0x51bdc0: the unreliable layer's connection closed.
reg32 unreliableShutdown(App* app, CPU& cpu, reg32 self);
// The game's 0x51be90: the first frame in what the stream gave, delivered or skipped; what was used.
reg32 unreliableReceiveFrame(App* app, CPU& cpu, reg32 self, reg32 counted, reg32 buffer, reg32 length);
// The game's 0x51bfd0: all that came for the unreliable layer.
reg32 unreliableReceive(App* app, CPU& cpu, reg32 self);
// The game's 0x51c0a0: the stream says something came: read while there is.
reg32 unreliableNotify(App* app, CPU& cpu, reg32 connection);
// The game's 0x51c100: a connection opened on the unreliable layer; 1 if it stands.
reg32 unreliableOpen(App* app, CPU& cpu, reg32 connection, reg32 ops, reg32 asked);
// The game's 0x51c510: data sent on the unreliable layer, made a frame on the stack.
reg32 unreliableSend(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length);
// The game's 0x51c5c0: the unreliable layer's turn; 1.
reg32 unreliableProcess(App* app, CPU& cpu, reg32 connection);
// The game's 0x51c690: the unreliable layer's connection closed by us; 1 if it was there.
reg32 unreliableClose(App* app, CPU& cpu, reg32 connection);
// The game's 0x51c7a0: a number of few bits widened, near what is expected.
reg32 sequenceWiden(App* app, CPU& cpu, reg32 expected, reg32 low, reg32 mask);
// The game's 0x51c7c0: a reliable frame's checksum stamped.
reg32 packetChecksum(App* app, CPU& cpu, reg32 frame, reg32 length);
// The game's 0x51c7f0: whether a reliable frame's checksum holds.
reg32 packetChecksumGood(App* app, CPU& cpu, reg32 frame, reg32 length);
// The game's 0x51c820: the number to acknowledge in what is sent now.
reg32 packetAckNumber(App* app, CPU& cpu, reg32 self);
// The game's 0x51c890: a packet numbered and its frame's start and data made.
reg32 packetBuild(App* app, CPU& cpu, reg32 out, reg32 self, reg32 data, reg32 length);
// The game's 0x51c8f0: a packet written if the stream has room, its resend time started; 1 if it went.
reg32 packetSend(App* app, CPU& cpu, reg32 out, reg32 self);
// The game's 0x51ca50: a frame of no data, its number the `code`; 1 if it went.
reg32 packetSendControl(App* app, CPU& cpu, reg32 control, reg32 self, reg32 code);
// The game's 0x51caf0: a packet that came out of order asked for again, when its time has come.
reg32 packetSendNak(App* app, CPU& cpu, reg32 waiting, reg32 self);
// The game's 0x51cb50: what the other end acknowledged taken off those waiting.
reg32 packetGotAck(App* app, CPU& cpu, reg32 self, reg32 ack);
// The game's 0x51cc00: the other end wants a packet again: its time to resend is now.
reg32 packetGotNak(App* app, CPU& cpu, reg32 self, reg32 sequence);
// The game's 0x51cc80: the reliable layer's connection closed.
reg32 packetShutdown(App* app, CPU& cpu, reg32 self);
// The game's 0x51cd50: the first frame in what the stream gave, seen to; what was used.
reg32 packetReceiveFrame(App* app, CPU& cpu, reg32 self, reg32 counted, reg32 buffer, reg32 length);
// The game's 0x51cf30: all that came for the reliable layer.
reg32 packetReceive(App* app, CPU& cpu, reg32 self);
// The game's 0x51d000: what the reliable layer's queues keep packets in the order of.
reg32 packetQueueKey(App* app, CPU& cpu, reg32 packet);
// The game's 0x51d010: a packet's time to resend counted down; 1 when it is up.
reg32 packetCountDown(App* app, CPU& cpu, reg32 waiting);
// The game's 0x51d040: a packet sent again if its time is up, else 1.
reg32 packetResendDue(App* app, CPU& cpu, reg32 waiting, reg32 self);
// The game's 0x51d050: the stream says something came: read, delivered in order, a gap asked for.
reg32 packetNotify(App* app, CPU& cpu, reg32 connection);
// The game's 0x51d270: a connection opened on the reliable layer; 1 if it stands.
reg32 packetOpen(App* app, CPU& cpu, reg32 connection, reg32 ops, reg32 asked, reg32 windowBits);
// The game's 0x51d810: data sent on the reliable layer and kept until acknowledged.
reg32 packetSubmit(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length);
// The game's 0x51d8f0: the reliable layer's turn; 1.
reg32 packetProcess(App* app, CPU& cpu, reg32 connection);
// The game's 0x51d9e0: the reliable layer's connection closed by us; 1 if it was there.
reg32 packetClose(App* app, CPU& cpu, reg32 connection);

}  // namespace netc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
