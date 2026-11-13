#ifndef NFS3HP_NATIVE_FRONTEND_NETA_H
#define NFS3HP_NATIVE_FRONTEND_NETA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_neta.cpp's typed functions (the network's session, its packets, a race's inputs,
 * NET_'s services and Winsock IPX), which other files call as C++ (guest::direct), and how the game's
 * code calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other
 * functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace neta
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4c5720: each frame of a network race: whether this machine leaves the race, and how.
reg32 raceQuitVote(App* app, CPU& cpu, reg32 vote);
// The game's 0x49a520: each pass of a network lobby: 1 when anything has changed that the lobby's screen shows.
reg32 lobbyChanged(App* app, CPU& cpu);
// The game's 0x4f5c10: opens a game's IPX socket and its thread, eax the port, edx a flag and ebx what goes with it.
reg32 ipxOpen(App* app, CPU& cpu, reg32 portNumber, reg32 flagValue, reg32 withFlag);
// The game's 0x4a5b00: RaceNet's NPS opened and a game looked for: eax the player's name or 0, edx the port.
reg32 npsStart(App* app, CPU& cpu, reg32 name, reg32 localPort, reg32 server, reg32 remotePort);
// The game's 0x4a6a10: the barrier, every machine stopped until all have come to the same point.
reg32 barrier(App* app, CPU& cpu, reg32 seconds, reg32 point);
// The game's 0x4c5e80: before a network race: the settings that matter sent from the host in 0xd0 byte packets.
reg32 raceSettings(App* app, CPU& cpu);
// The game's 0x4a66c0: the host sends the machines' list, packet 1, to each of them.
reg32 sendSetup(App* app, CPU& cpu);
// The game's 0x49bab0: the cars' files between the machines that need them.
reg32 exchangeCars(App* app, CPU& cpu);
// The game's 0x49eae0: from the host's lobby: the machines that have come and gone since last time.
reg32 hostLobby(App* app, CPU& cpu);
// The game's 0x49e080: the host's game started, on RaceNet: the settings the launcher left at 0x7a1fa0.
reg32 hostStart(App* app, CPU& cpu);
// The game's 0x4f61f0: the IPX service's receive, stack the room in the buffer: eax the state.
reg32 ipxReceive(App* app, CPU& cpu, reg32 state, reg32 address, reg32 kindPlace, reg32 buffer, reg32 room);
// The game's 0x49a8e0: this machine's player sent to the others, packet 3: its name and car by name and id.
reg32 sendMyCar(App* app, CPU& cpu);
// The game's 0x4c8040: a player's controls for the race's frame: eax where they go, edx the player.
reg32 playerControls(App* app, CPU& cpu, reg32 out, reg32 player);
// The game's 0x49acd0: as a network race loads: the cars' files exchanged and every machine at the same point.
reg32 playersToRace(App* app, CPU& cpu);
// The game's 0x4a6ce0: the game's version, packet 0: the host sends its own to a machine that joins.
reg32 compareVersions(App* app, CPU& cpu, reg32 link);
// The game's 0x4f4d10: NET_initservice, stack the three that go on: eax the service's record, edx a name.
reg32 initService(App* app, CPU& cpu, reg32 service, reg32 name, reg32 first, reg32 second, reg32 onOne, reg32 onTwo, reg32 onThree);
// The game's 0x49b9b0: a car's file sent, eax the packet with its kind first, edx the file, ebx its size.
reg32 sendCarFile(App* app, CPU& cpu, reg32 packet, reg32 file, reg32 left);
// The game's 0x49abd0: from the lobby: whether the race can start: 1 alone or when every machine's player has its car.
reg32 allReady(App* app, CPU& cpu);
// The game's 0x4c7d30: packet 13: a machine's controls for a frame, edx the packet, ebx its size.
reg32 inputPacket(App* app, CPU& cpu, reg32 packet, reg32 size);
// The game's 0x4f53f0: a service's poll, eax the game, edx where the sender's address is.
reg32 servicePoll(App* app, CPU& cpu, reg32 service, reg32 address);
// The game's 0x4f60b0: WINIPXSend, stack the size: eax the state, edx whom to (0 the state's own port).
reg32 ipxSend(App* app, CPU& cpu, reg32 state, reg32 whom, reg32 kind, reg32 data, reg32 bytes);
// The game's 0x4f5950: one packet off the socket: eax the state, edx the packet's room.
reg32 ipxRecvFrom(App* app, CPU& cpu, reg32 state, reg32 packet);
// The game's 0x4c5b00: packet 8: what a machine wants of the race, edx the packet, eax whom from.
reg32 quitPacket(App* app, CPU& cpu, reg32 from, reg32 packet);
// The game's 0x4f5130: NET_'s entry to a service's call, stack what goes on: eax the record, edx the service's table.
reg32 serviceCall(App* app, CPU& cpu, reg32 record, reg32 table, reg32 first, reg32 second, reg32 third);
// The game's 0x4a68d0: packet 1 on a client: the host's list of machines taken on whole, 0x7c bytes.
reg32 setupPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4f54f0: a service's open of a game: eax the game, edx what goes with it.
reg32 serviceOpenGame(App* app, CPU& cpu, reg32 game, reg32 with);
// The game's 0x49fb60: Windows' dial-up networking loaded and its functions found: 1 when they are all there.
reg32 rasLoad(App* app, CPU& cpu);
// The game's 0x4f55e0: a service's close of a game: eax the game.
reg32 serviceCloseGame(App* app, CPU& cpu, reg32 game);
// The game's 0x49b310: packet 3: another machine's player record, 0x60 bytes, taken on unless it is this machine's own.
reg32 playerPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x49f880: dial-up networking's phone book: eax where its names go, 21 bytes each, or 0.
reg32 rasPhoneBook(App* app, CPU& cpu, reg32 names);
// The game's 0x4f5710: a game's read: eax the game, edx the buffer, ebx its room.
reg32 serviceRead(App* app, CPU& cpu, reg32 game, reg32 buffer, reg32 room);
// The game's 0x4c7ea0: after a network race: each machine's times sent to the others, packet 12: this machine's car.
reg32 sendRaceResults(App* app, CPU& cpu);
// The game's 0x49aa80: the host's settings sent to the others, packet 2, 0x28 bytes.
reg32 sendSettings(App* app, CPU& cpu);
// The game's 0x4c8250: each race tick: what has come in of each machine's controls moved to where the race reads them.
reg32 takeInputs(App* app, CPU& cpu, reg32 tick);
// The game's 0x4f5ea0: a game's IPX socket closed, eax the state: the thread told to end and waited for.
reg32 ipxClose(App* app, CPU& cpu, reg32 state);
// The game's 0x4a5e50: the callback NPS calls with what has happened, stack the session, the message and its data.
reg32 npsMessage(App* app, CPU& cpu, reg32, reg32 message, reg32 data);
// The game's 0x4a6500: the session's state back to nothing: the barriers and who has gone cleared.
reg32 sessionReset(App* app, CPU& cpu);
// The game's 0x49b3e0: 
reg32 carFileToHost(App* app, CPU& cpu, reg32 packet);
// The game's 0x49b470: 
reg32 carFileToClient(App* app, CPU& cpu, reg32 packet);
// The game's 0x49e220: joining a RaceNet game: the session reset and NPS opened, the wait for the host's list.
reg32 joinRaceNet(App* app, CPU& cpu);
// The game's 0x4a6c40: packet 4: the barrier a machine has come to.
reg32 barrierPacket(App* app, CPU& cpu, reg32 from, reg32 packet);
// The game's 0x4f4c70: NET_getservices: eax a function called with each service there is, edx and ebx what goes to it.
reg32 getServices(App* app, CPU& cpu, reg32 callback, reg32 one, reg32 two, reg32 room);
// The game's 0x4f5b10: the IPX thread, by its state in 0x9f3650.
reg32 ipxThread(App* app, CPU& cpu);
// The game's 0x4f57c0: a game's write: eax the game, edx the bytes, ebx how many.
reg32 serviceWrite(App* app, CPU& cpu, reg32 game, reg32 bytes, reg32 size);
// The game's 0x49f790: whether dial-up networking is there: a network card, or the DLL and a connection of it.
reg32 rasAvailable(App* app, CPU& cpu);
// The game's 0x4f6030: whether the socket has anything: eax the state.
reg32 ipxReady(App* app, CPU& cpu, reg32 state);
// The game's 0x4a5dd0: NPS let go: what the session has open, through the DLL's own pointers, then the session.
reg32 npsStop(App* app, CPU& cpu);
// The game's 0x4f4bb0: whether two GUIDs, at eax and edx, are the same.
reg32 sameGuid(App* app, CPU& cpu, reg32 one, reg32 other);
// The game's 0x4f5390: which of the services' games a packet is for, eax the service, edx the sender's address.
reg32 findGame(App* app, CPU& cpu, reg32 service, reg32 address);
// The game's 0x49f810: whether the machine has a network: Windows' word for it, or Winsock opening.
reg32 networkThere(App* app, CPU& cpu);
// The game's 0x4c7fd0: a machine's ring of controls emptied, eax the player.
reg32 clearRing(App* app, CPU& cpu, reg32 player);
// The game's 0x4f5a30: a service's listing asked for and answered: eax the state, edx whom to.
reg32 ipxAnswerName(App* app, CPU& cpu, reg32 state, reg32 whom, reg32 name);
// The game's 0x49f730: the next one waiting to join, off the ring of 6 at 0x7a01f0, 31 bytes each, to eax.
reg32 nextPending(App* app, CPU& cpu, reg32 to);
// The game's 0x49ee50: as a host's lobby closes: no one else may join: its game shut, unless RaceNet's.
reg32 lobbyClose(App* app, CPU& cpu);
// The game's 0x4a5f30: the endpoint of a link's number, eax: 0 for none.
reg32 endpointOfLink(App* app, CPU& cpu, reg32 link);
// The game's 0x477f60: a race's loading step: its messages cleared, the controls started.
reg32 raceLoadStep(App* app, CPU& cpu);
// The game's 0x49b2b0: packet 2: the host's settings, 0x28 bytes, kept for the lobby to take on.
reg32 settingsPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4f5ac0: a walk over the received queue: eax a packet, edx the kind and sender asked after.
reg32 markPacket(App* app, CPU& cpu, reg32 packet, reg32 asked);
// The game's 0x49ede0: as a host's lobby opens: its game made known, unless RaceNet's.
reg32 lobbyOpen(App* app, CPU& cpu);
// The game's 0x4f4e60: NET_killservice: eax the record, 0 when NET_ does not have it.
reg32 killService(App* app, CPU& cpu, reg32 service);
// The game's 0x4a69c0: a client's lobby: 1 when the host's list has come since last asked.
reg32 clientLobby(App* app, CPU& cpu);
// The game's 0x4f5a80: the match of a walk over the received queue: 0 for the packet asked for, not too old.
reg32 packetMatch(App* app, CPU& cpu, reg32 packet, reg32 asked);
// The game's 0x4a6470: an endpoint's call when its machine is lost: the host notes which.
reg32 endpointLost(App* app, CPU& cpu, reg32 endpoint);
// The game's 0x4f5ba0: IPX's start: Winsock 1.1 opened.
reg32 ipxStartup(App* app, CPU& cpu);
// The game's 0x49e320: one more waiting to join: eax who, edx its link.
reg32 pendingAdd(App* app, CPU& cpu, reg32 who, reg32 link);
// The game's 0x4c4190: a packet of what the host's race says, 0x14 bytes.
reg32 statusPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4f5230: NET_'s two lists and its exit, once.
reg32 servicesReady(App* app, CPU& cpu);
// The game's 0x4f5fe0: whether two addresses, at edx and ebx, have the same 8 bytes from their +4.
reg32 sameNode(App* app, CPU& cpu, reg32 first, reg32 second);
// The game's 0x49ec70: each frame: the machines the host has marked as going, 2, dropped and marked gone, 1.
reg32 dropGone(App* app, CPU& cpu);
// The game's 0x49fb10: whether dial-up networking's DLL and all its functions are there.
reg32 rasLoaded(App* app, CPU& cpu);
// The game's 0x4f5900: a sockaddr_ipx to everyone: eax the port, edx where it goes.
reg32 portToSockaddr(App* app, CPU& cpu, reg32 port, reg32 to);
// The game's 0x49edb0: the packet of which machines have gone: 16 bytes after its two.
reg32 gonePacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4f58c0: NET_'s address made a sockaddr_ipx: eax the address, edx where it goes.
reg32 toSockaddr(App* app, CPU& cpu, reg32 from, reg32 to);
// The game's 0x4a64c0: an endpoint's time-out: a host waiting on someone joining past their time, or the game stopping.
reg32 endpointTimedOut(App* app, CPU& cpu);
// The game's 0x4c7f90: packet 12: a car's times, 0xc0 bytes into it.
reg32 resultsPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4c8310: this machine's controls for its packet, eax where they go: its car's steering cut to an integer.
reg32 packControls(App* app, CPU& cpu, reg32 out);
// The game's 0x4f5880: a sockaddr_ipx made NET_'s address: eax the sockaddr, edx where it goes.
reg32 fromSockaddr(App* app, CPU& cpu, reg32 from, reg32 to);
// The game's 0x49e2e0: hosting a RaceNet game: the session reset and NPS opened.
reg32 hostRaceNet(App* app, CPU& cpu);
// The game's 0x4c81c0: in a network race with one local player on odd ticks: what has come in taken.
reg32 takeInputsOnTick(App* app, CPU& cpu, reg32 tick);
// The game's 0x4f4c10: at the game's exit: every service NET_ has killed, and its queue freed.
reg32 exitNet(App* app, CPU& cpu);
// The game's 0x4a6440: an endpoint's receive: a packet by its first byte, 0 to 18.
reg32 dispatchPacket(App* app, CPU& cpu, reg32 from, reg32 packet, reg32 size);
// The game's 0x4c7e50: packet 13's first two bytes sent, eax's low byte the player.
reg32 sendPlayerGone(App* app, CPU& cpu, reg32 player);
// The game's 0x49fc50: dial-up networking's DLL let go and its functions forgotten.
reg32 rasFree(App* app, CPU& cpu);
// The game's 0x4a65b0: while a client waits for the host: the system let run, and whether it has timed out.
reg32 hostGone(App* app, CPU& cpu);
// The game's 0x4f5850: a service's call to send, by its table: eax the game, ecx the service.
reg32 sendQueued(App* app, CPU& cpu, reg32 game, reg32 first, reg32 second, reg32 service);
// The game's 0x49a490: the session's three locks made and the players' records emptied.
reg32 makeLocks(App* app, CPU& cpu);
// The game's 0x4a5ef0: NPS's callback with a packet, on the stack: its link, its data, its size.
reg32 npsReceived(App* app, CPU& cpu, reg32, reg32, reg32 link, reg32, reg32 data, reg32 size);
// The game's 0x4f5bf0: Winsock closed: 1 when it did.
reg32 winsockClose(App* app, CPU& cpu);
// The game's 0x4f6010: the socket's own address, 20 bytes, to edx.
reg32 ipxOwnAddress(App* app, CPU& cpu, reg32 state, reg32 to);
// The game's 0x4f4eb0: NET_getgames.
reg32 getGames(App* app, CPU& cpu, reg32 service);
// The game's 0x4f4ed0: NET_setgame.
reg32 setGame(App* app, CPU& cpu, reg32 service);
// The game's 0x4f4f10: NET_newgame.
reg32 newGame(App* app, CPU& cpu, reg32 service);
// The game's 0x4f4f70: NET_opengame.
reg32 openGame(App* app, CPU& cpu, reg32 service);
// The game's 0x4f5050: a service's poll through its table, when it has one.
reg32 servicePollCall(App* app, CPU& cpu, reg32 service);
// The game's 0x4f56e0: whether a game's service is up, and what it can take: 0x566e20's word when it is.
reg32 serviceRoom(App* app, CPU& cpu, reg32 game);
// The game's 0x4f5700: a game's service asked through its table's +0x50.
reg32 serviceAsk(App* app, CPU& cpu, reg32 game);
// The game's 0x4f5fa0: the game's name kept, 0x30 bytes at most, and given to the socket's key: eax the state.
reg32 ipxSetName(App* app, CPU& cpu, reg32 state, reg32 name);
// The game's 0x477fd0: a race's loading step: its music prepared, outside a replay.
reg32 raceMusicStep(App* app, CPU& cpu);
// The game's 0x49f680: whether nobody waits to join.
reg32 pendingEmpty(App* app, CPU& cpu);
// The game's 0x4f5f50: the socket's port number, in the machine's order, out of the state's network order.
reg32 ipxPort(App* app, CPU& cpu, reg32 state);
// The game's 0x4c45d0: the controls' two counts cleared.
reg32 clearInputCounts(App* app, CPU& cpu);
// The game's 0x4c8350: a packet's controls unpacked, eax the packed, edx the room: the steering as a float.
reg32 unpackControls(App* app, CPU& cpu, reg32 packed, reg32 out);
// The game's 0x4f5110: the low timer's poll: NET_'s services walked.
reg32 pollServices(App* app, CPU& cpu);
// The game's 0x4f56c0: whether a game is listed and its service still has any.
reg32 gameListed(App* app, CPU& cpu, reg32 game);
// The game's 0x4f5f70: a key asked of the socket's library: eax the state, edx the room.
reg32 ipxKey(App* app, CPU& cpu, reg32 state, reg32 room);
// The game's 0x49b500: 
reg32 carChunkToHost(App* app, CPU& cpu, reg32 packet, reg32 size);
// The game's 0x49b520: 
reg32 carChunkToClient(App* app, CPU& cpu, reg32 packet, reg32 size);
// The game's 0x4a6410: an endpoint's handlers: lost, receive and time-out.
reg32 endpointHandlers(App* app, CPU& cpu, reg32 endpoint);
// The game's 0x4a6df0: packet 0: the host's version, 0x28 bytes after its kind.
reg32 versionPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4c7e80: the packet that a player has left: its car marked as gone.
reg32 playerLeftPacket(App* app, CPU& cpu, reg32 packet);
// The game's 0x4f50c0: at the game's exit: the services' connections undone and NET_'s two lists freed.
reg32 closeServices(App* app, CPU& cpu);
// The game's 0x4f50f0: one of NET_'s services polled when its table has a poll.
reg32 pollOne(App* app, CPU& cpu, reg32 service);
// The game's 0x4a5af0: this machine's link, the low byte of the NPS state's first word.
reg32 thisLink(App* app, CPU& cpu);
// The game's 0x4f5fc0: the game's name forgotten.
reg32 ipxForgetName(App* app, CPU& cpu, reg32 state);
// The game's 0x4f5fd0: nothing to do, a service's table's "yes".
reg32 answerYes(App* app, CPU& cpu);
// The game's 0x4f50b0: nothing to do, a record's default for what it was not given.
reg32 answerNo(App* app, CPU& cpu);
// The game's 0x4f5f90: on to sub_512d70, a service's table's jump.
reg32 ipxJump(App* app, CPU& cpu);

}  // namespace neta
}  // namespace frontend

namespace guest
{

// vote in eax.
template <>
struct ByAddress<frontend::neta::raceQuitVote>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vote)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ vote }));
    }
};

template <>
struct ByAddress<frontend::neta::lobbyChanged>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// portNumber in eax, flagValue in edx, withFlag in ebx.
template <>
struct ByAddress<frontend::neta::ipxOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 portNumber, x86::reg32 flagValue, x86::reg32 withFlag)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ portNumber, flagValue, withFlag }));
    }
};

// name in eax, localPort in edx, server in ebx, remotePort in ecx.
template <>
struct ByAddress<frontend::neta::npsStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 localPort, x86::reg32 server, x86::reg32 remotePort)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, localPort, server, remotePort }));
    }
};

// seconds in eax, point in edx.
template <>
struct ByAddress<frontend::neta::barrier>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 seconds, x86::reg32 point)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ seconds, point }));
    }
};

template <>
struct ByAddress<frontend::neta::raceSettings>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::sendSetup>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::exchangeCars>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::hostLobby>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::hostStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// state in eax, address in edx, kindPlace in ebx, buffer in ecx, on the stack room.
template <>
struct ByAddress<frontend::neta::ipxReceive>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 address, x86::reg32 kindPlace, x86::reg32 buffer, x86::reg32 room)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, address, kindPlace, buffer }, { room }));
    }
};

template <>
struct ByAddress<frontend::neta::sendMyCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// out in eax, player in edx.
template <>
struct ByAddress<frontend::neta::playerControls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ out, player }));
    }
};

template <>
struct ByAddress<frontend::neta::playersToRace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// link in eax.
template <>
struct ByAddress<frontend::neta::compareVersions>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 link)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ link }));
    }
};

// service in eax, name in edx, first in ebx, second in ecx, on the stack onOne, onTwo, onThree.
template <>
struct ByAddress<frontend::neta::initService>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service, x86::reg32 name, x86::reg32 first, x86::reg32 second, x86::reg32 onOne, x86::reg32 onTwo, x86::reg32 onThree)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service, name, first, second }, { onOne, onTwo, onThree }));
    }
};

// packet in eax, file in edx, left in ebx.
template <>
struct ByAddress<frontend::neta::sendCarFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 file, x86::reg32 left)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ packet, file, left }));
    }
};

template <>
struct ByAddress<frontend::neta::allReady>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packet in edx, size in ebx.
template <>
struct ByAddress<frontend::neta::inputPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet, size }));
    }
};

// service in eax, address in edx.
template <>
struct ByAddress<frontend::neta::servicePoll>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service, address }));
    }
};

// state in eax, whom in edx, kind in ebx, data in ecx, on the stack bytes.
template <>
struct ByAddress<frontend::neta::ipxSend>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 whom, x86::reg32 kind, x86::reg32 data, x86::reg32 bytes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, whom, kind, data }, { bytes }));
    }
};

// state in eax, packet in edx.
template <>
struct ByAddress<frontend::neta::ipxRecvFrom>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, packet }));
    }
};

// from in eax, packet in edx.
template <>
struct ByAddress<frontend::neta::quitPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, packet }));
    }
};

// record in eax, table in edx, first in ebx, second in ecx, on the stack third.
template <>
struct ByAddress<frontend::neta::serviceCall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record, x86::reg32 table, x86::reg32 first, x86::reg32 second, x86::reg32 third)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ record, table, first, second }, { third }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::setupPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// game in eax, with in edx.
template <>
struct ByAddress<frontend::neta::serviceOpenGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game, x86::reg32 with)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game, with }));
    }
};

template <>
struct ByAddress<frontend::neta::rasLoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// game in eax.
template <>
struct ByAddress<frontend::neta::serviceCloseGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::playerPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// names in eax.
template <>
struct ByAddress<frontend::neta::rasPhoneBook>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 names)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ names }));
    }
};

// game in eax, buffer in edx, room in ebx.
template <>
struct ByAddress<frontend::neta::serviceRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game, x86::reg32 buffer, x86::reg32 room)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game, buffer, room }));
    }
};

template <>
struct ByAddress<frontend::neta::sendRaceResults>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::sendSettings>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// tick in eax.
template <>
struct ByAddress<frontend::neta::takeInputs>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 tick)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ tick }));
    }
};

// state in eax.
template <>
struct ByAddress<frontend::neta::ipxClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state }));
    }
};

// on the stack a word, message, data.
template <>
struct ByAddress<frontend::neta::npsMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word1, x86::reg32 message, x86::reg32 data)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { word1, message, data }));
    }
};

template <>
struct ByAddress<frontend::neta::sessionReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::carFileToHost>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::carFileToClient>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

template <>
struct ByAddress<frontend::neta::joinRaceNet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// from in eax, packet in edx.
template <>
struct ByAddress<frontend::neta::barrierPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, packet }));
    }
};

// callback in eax, one in edx, two in ebx, room in ecx.
template <>
struct ByAddress<frontend::neta::getServices>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 callback, x86::reg32 one, x86::reg32 two, x86::reg32 room)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ callback, one, two, room }));
    }
};

template <>
struct ByAddress<frontend::neta::ipxThread>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// game in eax, bytes in edx, size in ebx.
template <>
struct ByAddress<frontend::neta::serviceWrite>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game, x86::reg32 bytes, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game, bytes, size }));
    }
};

template <>
struct ByAddress<frontend::neta::rasAvailable>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// state in eax.
template <>
struct ByAddress<frontend::neta::ipxReady>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state }));
    }
};

template <>
struct ByAddress<frontend::neta::npsStop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// one in eax, other in edx.
template <>
struct ByAddress<frontend::neta::sameGuid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 one, x86::reg32 other)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ one, other }));
    }
};

// service in eax, address in edx.
template <>
struct ByAddress<frontend::neta::findGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service, address }));
    }
};

template <>
struct ByAddress<frontend::neta::networkThere>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// player in eax.
template <>
struct ByAddress<frontend::neta::clearRing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// state in eax, whom in edx, name in ebx.
template <>
struct ByAddress<frontend::neta::ipxAnswerName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 whom, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, whom, name }));
    }
};

// to in eax.
template <>
struct ByAddress<frontend::neta::nextPending>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ to }));
    }
};

template <>
struct ByAddress<frontend::neta::lobbyClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// link in eax.
template <>
struct ByAddress<frontend::neta::endpointOfLink>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 link)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ link }));
    }
};

template <>
struct ByAddress<frontend::neta::raceLoadStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::settingsPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// packet in eax, asked in edx.
template <>
struct ByAddress<frontend::neta::markPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 asked)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ packet, asked }));
    }
};

template <>
struct ByAddress<frontend::neta::lobbyOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::killService>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

template <>
struct ByAddress<frontend::neta::clientLobby>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packet in eax, asked in edx.
template <>
struct ByAddress<frontend::neta::packetMatch>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 asked)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ packet, asked }));
    }
};

// endpoint in eax.
template <>
struct ByAddress<frontend::neta::endpointLost>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 endpoint)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ endpoint }));
    }
};

template <>
struct ByAddress<frontend::neta::ipxStartup>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// who in eax, link in edx.
template <>
struct ByAddress<frontend::neta::pendingAdd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 who, x86::reg32 link)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ who, link }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::statusPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

template <>
struct ByAddress<frontend::neta::servicesReady>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// first in edx, second in ebx.
template <>
struct ByAddress<frontend::neta::sameNode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 second)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, first, second }));
    }
};

template <>
struct ByAddress<frontend::neta::dropGone>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::rasLoaded>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// port in eax, to in edx.
template <>
struct ByAddress<frontend::neta::portToSockaddr>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 port, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ port, to }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::gonePacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// from in eax, to in edx.
template <>
struct ByAddress<frontend::neta::toSockaddr>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to }));
    }
};

template <>
struct ByAddress<frontend::neta::endpointTimedOut>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::resultsPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// out in eax.
template <>
struct ByAddress<frontend::neta::packControls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ out }));
    }
};

// from in eax, to in edx.
template <>
struct ByAddress<frontend::neta::fromSockaddr>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to }));
    }
};

template <>
struct ByAddress<frontend::neta::hostRaceNet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// tick in eax.
template <>
struct ByAddress<frontend::neta::takeInputsOnTick>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 tick)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ tick }));
    }
};

template <>
struct ByAddress<frontend::neta::exitNet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// from in eax, packet in edx, size in ebx.
template <>
struct ByAddress<frontend::neta::dispatchPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 packet, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, packet, size }));
    }
};

// player in eax.
template <>
struct ByAddress<frontend::neta::sendPlayerGone>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

template <>
struct ByAddress<frontend::neta::rasFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::hostGone>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// game in eax, first in edx, second in ebx, service in ecx.
template <>
struct ByAddress<frontend::neta::sendQueued>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game, x86::reg32 first, x86::reg32 second, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game, first, second, service }));
    }
};

template <>
struct ByAddress<frontend::neta::makeLocks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// on the stack a word, a word, link, a word, data, size.
template <>
struct ByAddress<frontend::neta::npsReceived>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word1, x86::reg32 word2, x86::reg32 link, x86::reg32 word4, x86::reg32 data, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { word1, word2, link, word4, data, size }));
    }
};

template <>
struct ByAddress<frontend::neta::winsockClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// state in eax, to in edx.
template <>
struct ByAddress<frontend::neta::ipxOwnAddress>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, to }));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::getGames>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::setGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::newGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::openGame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::servicePollCall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// game in eax.
template <>
struct ByAddress<frontend::neta::serviceRoom>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game }));
    }
};

// game in eax.
template <>
struct ByAddress<frontend::neta::serviceAsk>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game }));
    }
};

// state in eax, name in edx.
template <>
struct ByAddress<frontend::neta::ipxSetName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, name }));
    }
};

template <>
struct ByAddress<frontend::neta::raceMusicStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::pendingEmpty>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// state in eax.
template <>
struct ByAddress<frontend::neta::ipxPort>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state }));
    }
};

template <>
struct ByAddress<frontend::neta::clearInputCounts>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// packed in eax, out in edx.
template <>
struct ByAddress<frontend::neta::unpackControls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packed, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ packed, out }));
    }
};

template <>
struct ByAddress<frontend::neta::pollServices>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// game in eax.
template <>
struct ByAddress<frontend::neta::gameListed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 game)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ game }));
    }
};

// state in eax, room in edx.
template <>
struct ByAddress<frontend::neta::ipxKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 room)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state, room }));
    }
};

// packet in edx, size in ebx.
template <>
struct ByAddress<frontend::neta::carChunkToHost>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet, size }));
    }
};

// packet in edx, size in ebx.
template <>
struct ByAddress<frontend::neta::carChunkToClient>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet, size }));
    }
};

// endpoint in eax.
template <>
struct ByAddress<frontend::neta::endpointHandlers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 endpoint)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ endpoint }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::versionPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

// packet in edx.
template <>
struct ByAddress<frontend::neta::playerLeftPacket>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

template <>
struct ByAddress<frontend::neta::closeServices>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// service in eax.
template <>
struct ByAddress<frontend::neta::pollOne>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

template <>
struct ByAddress<frontend::neta::thisLink>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// state in eax.
template <>
struct ByAddress<frontend::neta::ipxForgetName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state }));
    }
};

template <>
struct ByAddress<frontend::neta::answerYes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::answerNo>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::neta::ipxJump>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
