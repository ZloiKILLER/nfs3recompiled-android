#ifndef NFS3HP_NATIVE_FRONTEND_CONNECT_H
#define NFS3HP_NATIVE_FRONTEND_CONNECT_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_connect.cpp's typed functions (the network's screens: connect, ipx, and what
 * they call of the network's session), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace connect
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x43dab0: a player's three words as a packet tells them, into its record.
u32 applyPlayerVector(App* app, ThreadState& ts, u32 packet);
// The game's 0x43f1f0: the screen drawn once, while the games are looked for.
u32 listingRedraw(App* app, ThreadState& ts);
// The game's 0x43f220: the games on the network looked for and shown as the rows of `list`.
u32 refreshGames(App* app, ThreadState& ts, u32 list);
// The game's 0x43f320: the list of how the game goes on the network, for `mode`.
u32 openProtocolList(App* app, ThreadState& ts, u32 mode);
// The game's 0x43f4e0: a line to type the game's TCP address in.
u32 openAddressDialog(App* app, ThreadState& ts);
// The game's 0x43f550: "Modem: <its name>", in a buffer of its own.
u32 modemLabel(App* app, ThreadState& ts);
// The game's 0x43f610: "COM port: <n>", in a buffer of its own.
u32 comPortLabel(App* app, ThreadState& ts);
// The game's 0x43f640: the player's name typed for a game to create: the list next.
u32 gameNameAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43f680: a TCP address typed: kept, looked up, the game there joined.
u32 addressAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43f760: TCP/IP's question answered: create (the list), join (an address).
u32 tcpAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43f7e0: the protocol list's answer.
u32 protocolListAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43f960: the serial question's answer: connect, or the next COM port.
u32 serialAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43fa00: the phone number typed: dialled.
u32 phoneAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43faf0: the modem question's answer: dial, wait for a call, or the next modem.
u32 modemAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43fc20: the message that dial-up networking is missing, answered.
u32 raceNetMissingAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43fc50: RaceNet's question answered: dial, or not.
u32 raceNetAnswer(App* app, ThreadState& ts, u32 answer, u32 closes);
// The game's 0x43fd10: RaceNet's button: a question, or a message that dial-up is missing.
u32 raceNetClicked(App* app, ThreadState& ts);
// The game's 0x43fe50: TCP/IP's button: joining only, by an address at once; else a question.
u32 tcpClicked(App* app, ThreadState& ts);
// The game's 0x43fee0: the serial button: its question.
u32 serialClicked(App* app, ThreadState& ts);
// The game's 0x43ff50: the modem button: its question.
u32 modemClicked(App* app, ThreadState& ts);
// The game's 0x440040: ipx's "create game": a line to type the player's name in.
u32 createGameClicked(App* app, ThreadState& ts);
// The game's 0x4400b0: ipx's "refresh": the games looked for again.
u32 refreshClicked(App* app, ThreadState& ts);
// The game's 0x4405c0: connect as it opens (menu, the screen).
u32 connectEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x440730: connect as it closes: the network's mode from the button chosen.
u32 connectExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x440810: ipx's every pass: the games looked for again when it is time.
u32 ipxFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x440890: ipx as it opens: the name's edit box, the buttons' handlers.
u32 ipxEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x451940: the keys waiting thrown away, `held` the key now counted as held.
u32 flushKeys(App* app, ThreadState& ts, u32 held);
// The game's 0x458450: an edit box's text, its length and third argument, if it is shown now.
u32 setEditField(App* app, ThreadState& ts, u32 field, u32 text, u32 third, u32 length);
// The game's 0x49c0e0: the mouse's pointer: -1 back to the one before, -3 as it is, else a shape.
u32 setPointer(App* app, ThreadState& ts, u32 asked);
// The game's 0x49e360: whether the machine has a connection of this kind.
u32 connectionAvailable(App* app, ThreadState& ts, u32 kind);
// The game's 0x49e3d0: the games on the connection of this kind, sorted by name.
u32 listSessions(App* app, ThreadState& ts, u32 lookFor, u32 player, u32 kind, u32 redraw);
// The game's 0x49e5f0: a game created on the connection of this kind, under `name`.
u32 createSession(App* app, ThreadState& ts, u32 name, u32 kind);
// The game's 0x49e800: the game at `endpoint` joined.
u32 joinSession(App* app, ThreadState& ts, u32 endpoint);
// The game's 0x49e9d0: the TCP/IP connection opened on the launcher's port, if it is not open.
u32 useTcp(App* app, ThreadState& ts, u32 player);
// The game's 0x49ea70: a host's name made an address on the launcher's port.
u32 findAddress(App* app, ThreadState& ts, u32 host, u32 address);
// The game's 0x49efb0: a connection's record: its timeouts and the address it goes to.
u32 setUpEndpoint(App* app, ThreadState& ts, u32 address, u32 connection);
// The game's 0x49f030: the list of connections made anew.
u32 enumerateServices(App* app, ThreadState& ts, u32 baud, u32 blocks, u32 slots);
// The game's 0x49f150: the connections looked for, once; `handed` the eax the code hands it, given
// back when they were.
u32 findConnections(App* app, ThreadState& ts, u32 handed);
// The game's 0x49f200: one the comm library has, told by name.
u32 serviceFound(App* app, ThreadState& ts, u32 guid, u32 name);
// The game's 0x49f380: a game found, told by the comm library: put on the list.
u32 sessionFound(App* app, ThreadState& ts, u32 comm, u32 session, u32 address);
// The game's 0x49f470: a peer with a name of its own, in the queue to be let in.
u32 queuePeerByName(App* app, ThreadState& ts, u32 peer);
// The game's 0x49f4b0: a peer at an address, in the queue under a name made up.
u32 queuePeer(App* app, ThreadState& ts, u32 address);
// The game's 0x49f520: the seven kept addresses forgotten.
u32 clearPeerSlots(App* app, ThreadState& ts);
// The game's 0x49f5c0: a peer's address kept, in the first of seven free.
u32 rememberPeer(App* app, ThreadState& ts, u32 address);
// The game's 0x49f650: the queue of peers emptied; `handed` the eax the code hands it, given back.
u32 clearQueue(App* app, ThreadState& ts, u32 handed);
// The game's 0x49f6a0: a peer's name and address, at the queue's tail.
u32 enqueuePeer(App* app, ThreadState& ts, u32 name, u32 address);
// The game's 0x4c3f90: a modem or the cable started, under a message.
u32 startConnection(App* app, ThreadState& ts, u32 service);
// The game's 0x4c41d0: the modems' names, at most 15, in `list`.
u32 listModems(App* app, ThreadState& ts, u32 list);
// The game's 0x4c4230: a modem's or a cable's link set up.
u32 setUpLink(App* app, ThreadState& ts, u32 service);
// The game's 0x4c4310: hello said to the other end, its hello awaited.
u32 shakeHands(App* app, ThreadState& ts);

}  // namespace connect
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
