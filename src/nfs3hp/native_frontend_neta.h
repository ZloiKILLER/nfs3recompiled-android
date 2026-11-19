#ifndef NFS3HP_NATIVE_FRONTEND_NETA_H
#define NFS3HP_NATIVE_FRONTEND_NETA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_neta.cpp's typed functions (the network's session, its packets, a race's inputs,
 * NET_'s services and Winsock IPX), which other files call as C++ (guest::direct), and how the game's
 * code calls each by address.  Each calls the game's other
 * functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace neta
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4c5720: each frame of a network race: whether this machine leaves the race, and how.
u32 raceQuitVote(App* app, ThreadState& ts, u32 vote);
// The game's 0x49a520: each pass of a network lobby: 1 when anything has changed that the lobby's screen shows.
u32 lobbyChanged(App* app, ThreadState& ts);
u32 lobbyChangedFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f5c10: opens a game's IPX socket and its thread, eax the port, edx a flag and ebx what goes with it.
u32 ipxOpen(App* app, ThreadState& ts, u32 portNumber, u32 flagValue, u32 withFlag);
// The game's 0x4a5b00: RaceNet's NPS opened and a game looked for: eax the player's name or 0, edx the port.
u32 npsStart(App* app, ThreadState& ts, u32 name, u32 localPort, u32 server, u32 remotePort);
// The game's 0x4a6a10: the barrier, every machine stopped until all have come to the same point.
u32 barrier(App* app, ThreadState& ts, u32 seconds, u32 point);
// The game's 0x4c5e80: before a network race: the settings that matter sent from the host in 0xd0 byte packets.
u32 raceSettings(App* app, ThreadState& ts);
// The game's 0x4a66c0: the host sends the machines' list, packet 1, to each of them.
u32 sendSetup(App* app, ThreadState& ts);
// The game's 0x49bab0: the cars' files between the machines that need them.
u32 exchangeCars(App* app, ThreadState& ts);
// The game's 0x49eae0: from the host's lobby: the machines that have come and gone since last time.
u32 hostLobby(App* app, ThreadState& ts);
// The game's 0x49e080: the host's game started, on RaceNet: the settings the launcher left at 0x7a1fa0.
u32 hostStart(App* app, ThreadState& ts);
// The game's 0x4f61f0: the IPX service's receive, stack the room in the buffer: eax the state.
u32 ipxReceive(App* app, ThreadState& ts, u32 state, u32 address, u32 kindPlace, u32 buffer, u32 room);
// The game's 0x49a8e0: this machine's player sent to the others, packet 3: its name and car by name and id.
u32 sendMyCar(App* app, ThreadState& ts);
u32 sendMyCarFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4c8040: a player's controls for the race's frame: eax where they go, edx the player.
u32 playerControls(App* app, ThreadState& ts, u32 out, u32 player);
// The game's 0x49acd0: as a network race loads: the cars' files exchanged and every machine at the same point.
u32 playersToRace(App* app, ThreadState& ts);
// The game's 0x4a6ce0: the game's version, packet 0: the host sends its own to a machine that joins.
u32 compareVersions(App* app, ThreadState& ts, u32 link);
// The game's 0x4f4d10: NET_initservice, stack the three that go on: eax the service's record, edx a name.
u32 initService(App* app, ThreadState& ts, u32 service, u32 name, u32 first, u32 second, u32 onOne, u32 onTwo, u32 onThree);
// The game's 0x49b9b0: a car's file sent, eax the packet with its kind first, edx the file, ebx its size.
u32 sendCarFile(App* app, ThreadState& ts, u32 packet, u32 file, u32 left);
// The game's 0x49abd0: from the lobby: whether the race can start: 1 alone or when every machine's player has its car.
u32 allReady(App* app, ThreadState& ts);
// The game's 0x4c7d30: packet 13: a machine's controls for a frame, edx the packet, ebx its size.
u32 inputPacket(App* app, ThreadState& ts, u32 packet, u32 size);
// The game's 0x4f53f0: a service's poll, eax the game, edx where the sender's address is.
u32 servicePoll(App* app, ThreadState& ts, u32 service, u32 address);
// The game's 0x4f60b0: WINIPXSend, stack the size: eax the state, edx whom to (0 the state's own port).
u32 ipxSend(App* app, ThreadState& ts, u32 state, u32 whom, u32 kind, u32 data, u32 bytes);
// The game's 0x4f5950: one packet off the socket: eax the state, edx the packet's room.
u32 ipxRecvFrom(App* app, ThreadState& ts, u32 state, u32 packet);
// The game's 0x4c5b00: packet 8: what a machine wants of the race, edx the packet, eax whom from.
u32 quitPacket(App* app, ThreadState& ts, u32 from, u32 packet);
// The game's 0x4f5130: NET_'s entry to a service's call, stack what goes on: eax the record, edx the service's table.
u32 serviceCall(App* app, ThreadState& ts, u32 record, u32 table, u32 first, u32 second, u32 third);
// The game's 0x4a68d0: packet 1 on a client: the host's list of machines taken on whole, 0x7c bytes.
u32 setupPacket(App* app, ThreadState& ts, u32 packet);
// The game's 0x4f54f0: a service's open of a game: eax the game, edx what goes with it.
u32 serviceOpenGame(App* app, ThreadState& ts, u32 game, u32 with);
// The game's 0x49fb60: Windows' dial-up networking loaded and its functions found: 1 when they are all there.
u32 rasLoad(App* app, ThreadState& ts);
// The game's 0x4f55e0: a service's close of a game: eax the game.
u32 serviceCloseGame(App* app, ThreadState& ts, u32 game);
// The game's 0x49b310: packet 3: another machine's player record, 0x60 bytes, taken on unless it is this machine's own.
u32 playerPacket(App* app, ThreadState& ts, u32 packet);
u32 playerPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed);  // the same, the eax it is entered with
// The game's 0x49f880: dial-up networking's phone book: eax where its names go, 21 bytes each, or 0.
u32 rasPhoneBook(App* app, ThreadState& ts, u32 names);
// The game's 0x4f5710: a game's read: eax the game, edx the buffer, ebx its room.
u32 serviceRead(App* app, ThreadState& ts, u32 game, u32 buffer, u32 room);
// The game's 0x4c7ea0: after a network race: each machine's times sent to the others, packet 12: this machine's car.
u32 sendRaceResults(App* app, ThreadState& ts);
u32 sendRaceResultsFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x49aa80: the host's settings sent to the others, packet 2, 0x28 bytes.
u32 sendSettings(App* app, ThreadState& ts);
// The game's 0x4c8250: each race tick: what has come in of each machine's controls moved to where the race reads them.
u32 takeInputs(App* app, ThreadState& ts, u32 tick);
// The game's 0x4f5ea0: a game's IPX socket closed, eax the state: the thread told to end and waited for.
u32 ipxClose(App* app, ThreadState& ts, u32 state);
// The game's 0x4a5e50: the callback NPS calls with what has happened, stack the session, the message and its data.
u32 npsMessage(App* app, ThreadState& ts, u32, u32 message, u32 data);
// The game's 0x4a6500: the session's state back to nothing: the barriers and who has gone cleared.
u32 sessionReset(App* app, ThreadState& ts);
// The game's 0x49b3e0: 
u32 carFileToHost(App* app, ThreadState& ts, u32 packet);
u32 carFileToHostFrom(App* app, ThreadState& ts, u32 packet, u32 handed);  // the same, the eax it is entered with
// The game's 0x49b470: 
u32 carFileToClient(App* app, ThreadState& ts, u32 packet);
u32 carFileToClientFrom(App* app, ThreadState& ts, u32 packet, u32 handed);  // the same, the eax it is entered with
// The game's 0x49e220: joining a RaceNet game: the session reset and NPS opened, the wait for the host's list.
u32 joinRaceNet(App* app, ThreadState& ts);
// The game's 0x4a6c40: packet 4: the barrier a machine has come to.
u32 barrierPacket(App* app, ThreadState& ts, u32 from, u32 packet);
// The game's 0x4f4c70: NET_getservices: eax a function called with each service there is, edx and ebx what goes to it.
u32 getServices(App* app, ThreadState& ts, u32 callback, u32 one, u32 two, u32 room);
// The game's 0x4f5b10: the IPX thread, by its state in 0x9f3650.
u32 ipxThread(App* app, ThreadState& ts);
// The game's 0x4f57c0: a game's write: eax the game, edx the bytes, ebx how many.
u32 serviceWrite(App* app, ThreadState& ts, u32 game, u32 bytes, u32 size);
// The game's 0x49f790: whether dial-up networking is there: a network card, or the DLL and a connection of it.
u32 rasAvailable(App* app, ThreadState& ts);
// The game's 0x4f6030: whether the socket has anything: eax the state.
u32 ipxReady(App* app, ThreadState& ts, u32 state);
// The game's 0x4a5dd0: NPS let go: what the session has open, through the DLL's own pointers, then the session.
u32 npsStop(App* app, ThreadState& ts);
u32 npsStopFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f4bb0: whether two GUIDs, at eax and edx, are the same.
u32 sameGuid(App* app, ThreadState& ts, u32 one, u32 other);
// The game's 0x4f5390: which of the services' games a packet is for, eax the service, edx the sender's address.
u32 findGame(App* app, ThreadState& ts, u32 service, u32 address);
// The game's 0x49f810: whether the machine has a network: Windows' word for it, or Winsock opening.
u32 networkThere(App* app, ThreadState& ts);
// The game's 0x4c7fd0: a machine's ring of controls emptied, eax the player.
u32 clearRing(App* app, ThreadState& ts, u32 player);
// The game's 0x4f5a30: a service's listing asked for and answered: eax the state, edx whom to.
u32 ipxAnswerName(App* app, ThreadState& ts, u32 state, u32 whom, u32 name);
// The game's 0x49f730: the next one waiting to join, off the ring of 6 at 0x7a01f0, 31 bytes each, to eax.
u32 nextPending(App* app, ThreadState& ts, u32 to);
// The game's 0x49ee50: as a host's lobby closes: no one else may join: its game shut, unless RaceNet's.
u32 lobbyClose(App* app, ThreadState& ts);
u32 lobbyCloseFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4a5f30: the endpoint of a link's number, eax: 0 for none.
u32 endpointOfLink(App* app, ThreadState& ts, u32 link);
// The game's 0x477f60: a race's loading step: its messages cleared, the controls started.
u32 raceLoadStep(App* app, ThreadState& ts);
// The game's 0x49b2b0: packet 2: the host's settings, 0x28 bytes, kept for the lobby to take on.
u32 settingsPacket(App* app, ThreadState& ts, u32 packet);
u32 settingsPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f5ac0: a walk over the received queue: eax a packet, edx the kind and sender asked after.
u32 markPacket(App* app, ThreadState& ts, u32 packet, u32 asked);
// The game's 0x49ede0: as a host's lobby opens: its game made known, unless RaceNet's.
u32 lobbyOpen(App* app, ThreadState& ts);
u32 lobbyOpenFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f4e60: NET_killservice: eax the record, 0 when NET_ does not have it.
u32 killService(App* app, ThreadState& ts, u32 service);
// The game's 0x4a69c0: a client's lobby: 1 when the host's list has come since last asked.
u32 clientLobby(App* app, ThreadState& ts);
// The game's 0x4f5a80: the match of a walk over the received queue: 0 for the packet asked for, not too old.
u32 packetMatch(App* app, ThreadState& ts, u32 packet, u32 asked);
// The game's 0x4a6470: an endpoint's call when its machine is lost: the host notes which.
u32 endpointLost(App* app, ThreadState& ts, u32 endpoint);
// The game's 0x4f5ba0: IPX's start: Winsock 1.1 opened.
u32 ipxStartup(App* app, ThreadState& ts);
// The game's 0x49e320: one more waiting to join: eax who, edx its link.
u32 pendingAdd(App* app, ThreadState& ts, u32 who, u32 link);
// The game's 0x4c4190: a packet of what the host's race says, 0x14 bytes.
u32 statusPacket(App* app, ThreadState& ts, u32 packet);
// The game's 0x4f5230: NET_'s two lists and its exit, once.
u32 servicesReady(App* app, ThreadState& ts);
u32 servicesReadyFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f5fe0: whether two addresses, at edx and ebx, have the same 8 bytes from their +4.
u32 sameNode(App* app, ThreadState& ts, u32 first, u32 second);
// The game's 0x49ec70: each frame: the machines the host has marked as going, 2, dropped and marked gone, 1.
u32 dropGone(App* app, ThreadState& ts);
u32 dropGoneFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x49fb10: whether dial-up networking's DLL and all its functions are there.
u32 rasLoaded(App* app, ThreadState& ts);
// The game's 0x4f5900: a sockaddr_ipx to everyone: eax the port, edx where it goes.
u32 portToSockaddr(App* app, ThreadState& ts, u32 port, u32 to);
// The game's 0x49edb0: the packet of which machines have gone: 16 bytes after its two.
u32 gonePacket(App* app, ThreadState& ts, u32 packet);
// The game's 0x4f58c0: NET_'s address made a sockaddr_ipx: eax the address, edx where it goes.
u32 toSockaddr(App* app, ThreadState& ts, u32 from, u32 to);
// The game's 0x4a64c0: an endpoint's time-out: a host waiting on someone joining past their time, or the game stopping.
u32 endpointTimedOut(App* app, ThreadState& ts);
// The game's 0x4c7f90: packet 12: a car's times, 0xc0 bytes into it.
u32 resultsPacket(App* app, ThreadState& ts, u32 packet);
// The game's 0x4c8310: this machine's controls for its packet, eax where they go: its car's steering cut to an integer.
u32 packControls(App* app, ThreadState& ts, u32 out);
// The game's 0x4f5880: a sockaddr_ipx made NET_'s address: eax the sockaddr, edx where it goes.
u32 fromSockaddr(App* app, ThreadState& ts, u32 from, u32 to);
// The game's 0x49e2e0: hosting a RaceNet game: the session reset and NPS opened.
u32 hostRaceNet(App* app, ThreadState& ts);
// The game's 0x4c81c0: in a network race with one local player on odd ticks: what has come in taken.
u32 takeInputsOnTick(App* app, ThreadState& ts, u32 tick);
// The game's 0x4f4c10: at the game's exit: every service NET_ has killed, and its queue freed.
u32 exitNet(App* app, ThreadState& ts);
u32 exitNetFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4a6440: an endpoint's receive: a packet by its first byte, 0 to 18.
u32 dispatchPacket(App* app, ThreadState& ts, u32 from, u32 packet, u32 size);
// The game's 0x4c7e50: packet 13's first two bytes sent, eax's low byte the player.
u32 sendPlayerGone(App* app, ThreadState& ts, u32 player);
// The game's 0x49fc50: dial-up networking's DLL let go and its functions forgotten.
u32 rasFree(App* app, ThreadState& ts);
u32 rasFreeFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4a65b0: while a client waits for the host: the system let run, and whether it has timed out.
u32 hostGone(App* app, ThreadState& ts);
// The game's 0x4f5850: a service's call to send, by its table: eax the game, ecx the service.
u32 sendQueued(App* app, ThreadState& ts, u32 game, u32 first, u32 second, u32 service);
// The game's 0x49a490: the session's three locks made and the players' records emptied.
u32 makeLocks(App* app, ThreadState& ts);
// The game's 0x4a5ef0: NPS's callback with a packet, on the stack: its link, its data, its size.
u32 npsReceived(App* app, ThreadState& ts, u32, u32, u32 link, u32, u32 data, u32 size);
// The game's 0x4f5bf0: Winsock closed: 1 when it did.
u32 winsockClose(App* app, ThreadState& ts);
// The game's 0x4f6010: the socket's own address, 20 bytes, to edx.
u32 ipxOwnAddress(App* app, ThreadState& ts, u32 state, u32 to);
// The game's 0x4f4eb0: NET_getgames: the service's lister asked with `callback`, `lookFor` and `byte`.
u32 getGames(App* app, ThreadState& ts, u32 service, u32 callback, u32 lookFor, u32 byte);
// The game's 0x4f4ed0: NET_setgame: the service's joiner asked with `endpoint` and `byte`.
u32 setGame(App* app, ThreadState& ts, u32 service, u32 endpoint, u32 byte, u32 kept);  // `kept`: the ecx the caller has, which it keeps
// The game's 0x4f4f10: NET_newgame: the service's maker asked with `name`, `byte` and `handler`.
u32 newGame(App* app, ThreadState& ts, u32 service, u32 name, u32 byte, u32 handler);
// The game's 0x4f4f70: NET_opengame.
u32 openGame(App* app, ThreadState& ts, u32 service);
// The game's 0x4f5050: a service's poll through its table, when it has one.
u32 servicePollCall(App* app, ThreadState& ts, u32 service, u32 first, u32 second, u32 third);
// The game's 0x4f56e0: whether a game's service is up, and what it can take: 0x566e20's word when it is.
u32 serviceRoom(App* app, ThreadState& ts, u32 game);
// The game's 0x4f5700: a game's service asked through its table's +0x50.
u32 serviceAsk(App* app, ThreadState& ts, u32 game);
// The game's 0x4f5fa0: the game's name kept, 0x30 bytes at most, and given to the socket's key: eax the state.
u32 ipxSetName(App* app, ThreadState& ts, u32 state, u32 name);
// The game's 0x477fd0: a race's loading step: its music prepared, outside a replay.
u32 raceMusicStep(App* app, ThreadState& ts);
u32 raceMusicStepFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x49f680: whether nobody waits to join.
u32 pendingEmpty(App* app, ThreadState& ts);
// The game's 0x4f5f50: the socket's port number, in the machine's order, out of the state's network order.
u32 ipxPort(App* app, ThreadState& ts, u32 state);
// The game's 0x4c45d0: the controls' two counts cleared.
u32 clearInputCounts(App* app, ThreadState& ts);
u32 clearInputCountsFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4c8350: a packet's controls unpacked, eax the packed, edx the room: the steering as a float.
u32 unpackControls(App* app, ThreadState& ts, u32 packed, u32 out);
// The game's 0x4f5110: the low timer's poll: NET_'s services walked.
u32 pollServices(App* app, ThreadState& ts);
// The game's 0x4f56c0: whether a game is listed and its service still has any.
u32 gameListed(App* app, ThreadState& ts, u32 game);
// The game's 0x4f5f70: a key asked of the socket's library: eax the state, edx the room.
u32 ipxKey(App* app, ThreadState& ts, u32 state, u32 room);
// The game's 0x49b500: 
u32 carChunkToHost(App* app, ThreadState& ts, u32 packet, u32 size);
// The game's 0x49b520: 
u32 carChunkToClient(App* app, ThreadState& ts, u32 packet, u32 size);
// The game's 0x4a6410: an endpoint's handlers: lost, receive and time-out.
u32 endpointHandlers(App* app, ThreadState& ts, u32 endpoint);
// The game's 0x4a6df0: packet 0: the host's version, 0x28 bytes after its kind.
u32 versionPacket(App* app, ThreadState& ts, u32 packet);
// The game's 0x4c7e80: the packet that a player has left: its car marked as gone.
u32 playerLeftPacket(App* app, ThreadState& ts, u32 packet);
u32 playerLeftPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f50c0: at the game's exit: the services' connections undone and NET_'s two lists freed.
u32 closeServices(App* app, ThreadState& ts);
u32 closeServicesFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f50f0: one of NET_'s services polled when its table has a poll.
u32 pollOne(App* app, ThreadState& ts, u32 service);
// The game's 0x4a5af0: this machine's link, the low byte of the NPS state's first word.
u32 thisLink(App* app, ThreadState& ts);
u32 thisLinkFrom(App* app, ThreadState& ts, u32 handed);  // the same, the eax it is entered with
// The game's 0x4f5fc0: the game's name forgotten.
u32 ipxForgetName(App* app, ThreadState& ts, u32 state);
// The game's 0x4f5fd0: nothing to do, a service's table's "yes".
u32 answerYes(App* app, ThreadState& ts);
// The game's 0x4f50b0: nothing to do, a record's default for what it was not given.
u32 answerNo(App* app, ThreadState& ts);
// The game's 0x4f5f90: on to sub_512d70, a service's table's jump.
u32 ipxJump(App* app, ThreadState& ts);

}  // namespace neta
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
