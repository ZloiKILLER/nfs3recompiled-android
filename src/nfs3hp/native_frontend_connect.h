#ifndef NFS3HP_NATIVE_FRONTEND_CONNECT_H
#define NFS3HP_NATIVE_FRONTEND_CONNECT_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_connect.cpp's typed functions (the network's screens: connect, ipx, and what
 * they call of the network's session), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace connect
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x43dab0: a player's three words as a packet tells them, into its record.
reg32 applyPlayerVector(App* app, CPU& cpu, reg32 packet);
// The game's 0x43f1f0: the screen drawn once, while the games are looked for.
reg32 listingRedraw(App* app, CPU& cpu);
// The game's 0x43f220: the games on the network looked for and shown as the rows of `list`.
reg32 refreshGames(App* app, CPU& cpu, reg32 list);
// The game's 0x43f320: the list of how the game goes on the network, for `mode`.
reg32 openProtocolList(App* app, CPU& cpu, reg32 mode);
// The game's 0x43f4e0: a line to type the game's TCP address in.
reg32 openAddressDialog(App* app, CPU& cpu);
// The game's 0x43f550: "Modem: <its name>", in a buffer of its own.
reg32 modemLabel(App* app, CPU& cpu);
// The game's 0x43f610: "COM port: <n>", in a buffer of its own.
reg32 comPortLabel(App* app, CPU& cpu);
// The game's 0x43f640: the player's name typed for a game to create: the list next.
reg32 gameNameAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43f680: a TCP address typed: kept, looked up, the game there joined.
reg32 addressAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43f760: TCP/IP's question answered: create (the list), join (an address).
reg32 tcpAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43f7e0: the protocol list's answer.
reg32 protocolListAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43f960: the serial question's answer: connect, or the next COM port.
reg32 serialAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43fa00: the phone number typed: dialled.
reg32 phoneAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43faf0: the modem question's answer: dial, wait for a call, or the next modem.
reg32 modemAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43fc20: the message that dial-up networking is missing, answered.
reg32 raceNetMissingAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43fc50: RaceNet's question answered: dial, or not.
reg32 raceNetAnswer(App* app, CPU& cpu, reg32 answer, reg32 closes);
// The game's 0x43fd10: RaceNet's button: a question, or a message that dial-up is missing.
reg32 raceNetClicked(App* app, CPU& cpu);
// The game's 0x43fe50: TCP/IP's button: joining only, by an address at once; else a question.
reg32 tcpClicked(App* app, CPU& cpu);
// The game's 0x43fee0: the serial button: its question.
reg32 serialClicked(App* app, CPU& cpu);
// The game's 0x43ff50: the modem button: its question.
reg32 modemClicked(App* app, CPU& cpu);
// The game's 0x440040: ipx's "create game": a line to type the player's name in.
reg32 createGameClicked(App* app, CPU& cpu);
// The game's 0x4400b0: ipx's "refresh": the games looked for again.
reg32 refreshClicked(App* app, CPU& cpu);
// The game's 0x4405c0: connect as it opens (menu, the screen).
reg32 connectEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x440730: connect as it closes: the network's mode from the button chosen.
reg32 connectExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x440810: ipx's every pass: the games looked for again when it is time.
reg32 ipxFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x440890: ipx as it opens: the name's edit box, the buttons' handlers.
reg32 ipxEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x451940: the keys waiting thrown away, `held` the key now counted as held.
reg32 flushKeys(App* app, CPU& cpu, reg32 held);
// The game's 0x458450: an edit box's text, its length and third argument, if it is shown now.
reg32 setEditField(App* app, CPU& cpu, reg32 field, reg32 text, reg32 third, reg32 length);
// The game's 0x49c0e0: the mouse's pointer: -1 back to the one before, -3 as it is, else a shape.
reg32 setPointer(App* app, CPU& cpu, reg32 asked);
// The game's 0x49e360: whether the machine has a connection of this kind.
reg32 connectionAvailable(App* app, CPU& cpu, reg32 kind);
// The game's 0x49e3d0: the games on the connection of this kind, sorted by name.
reg32 listSessions(App* app, CPU& cpu, reg32 lookFor, reg32 player, reg32 kind, reg32 redraw);
// The game's 0x49e5f0: a game created on the connection of this kind, under `name`.
reg32 createSession(App* app, CPU& cpu, reg32 name, reg32 kind);
// The game's 0x49e800: the game at `endpoint` joined.
reg32 joinSession(App* app, CPU& cpu, reg32 endpoint);
// The game's 0x49e9d0: the TCP/IP connection opened on the launcher's port, if it is not open.
reg32 useTcp(App* app, CPU& cpu, reg32 player);
// The game's 0x49ea70: a host's name made an address on the launcher's port.
reg32 findAddress(App* app, CPU& cpu, reg32 host, reg32 address);
// The game's 0x49efb0: a connection's record: its timeouts and the address it goes to.
reg32 setUpEndpoint(App* app, CPU& cpu, reg32 address, reg32 connection);
// The game's 0x49f030: the list of connections made anew.
reg32 enumerateServices(App* app, CPU& cpu, reg32 baud, reg32 blocks, reg32 slots);
// The game's 0x49f150: the connections looked for, once.
reg32 findConnections(App* app, CPU& cpu);
// The game's 0x49f200: one the comm library has, told by name.
reg32 serviceFound(App* app, CPU& cpu, reg32 guid, reg32 name);
// The game's 0x49f380: a game found, told by the comm library: put on the list.
reg32 sessionFound(App* app, CPU& cpu, reg32 comm, reg32 session, reg32 address);
// The game's 0x49f470: a peer with a name of its own, in the queue to be let in.
reg32 queuePeerByName(App* app, CPU& cpu, reg32 peer);
// The game's 0x49f4b0: a peer at an address, in the queue under a name made up.
reg32 queuePeer(App* app, CPU& cpu, reg32 address);
// The game's 0x49f520: the seven kept addresses forgotten.
reg32 clearPeerSlots(App* app, CPU& cpu);
// The game's 0x49f5c0: a peer's address kept, in the first of seven free.
reg32 rememberPeer(App* app, CPU& cpu, reg32 address);
// The game's 0x49f650: the queue of peers emptied.
reg32 clearQueue(App* app, CPU& cpu);
// The game's 0x49f6a0: a peer's name and address, at the queue's tail.
reg32 enqueuePeer(App* app, CPU& cpu, reg32 name, reg32 address);
// The game's 0x4c3f90: a modem or the cable started, under a message.
reg32 startConnection(App* app, CPU& cpu, reg32 service);
// The game's 0x4c41d0: the modems' names, at most 15, in `list`.
reg32 listModems(App* app, CPU& cpu, reg32 list);
// The game's 0x4c4230: a modem's or a cable's link set up.
reg32 setUpLink(App* app, CPU& cpu, reg32 service);
// The game's 0x4c4310: hello said to the other end, its hello awaited.
reg32 shakeHands(App* app, CPU& cpu);

}  // namespace connect
}  // namespace frontend

namespace guest
{

// The packet in edx, eax as it is.
template <>
struct ByAddress<frontend::connect::applyPlayerVector>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packet)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, packet }));
    }
};

template <>
struct ByAddress<frontend::connect::listingRedraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The list in eax.
template <>
struct ByAddress<frontend::connect::refreshGames>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list }));
    }
};

// The mode in eax.
template <>
struct ByAddress<frontend::connect::openProtocolList>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ mode }));
    }
};

template <>
struct ByAddress<frontend::connect::openAddressDialog>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::modemLabel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::comPortLabel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::gameNameAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::addressAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::tcpAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::protocolListAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::serialAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::phoneAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::modemAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::raceNetMissingAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

// The answer in eax, the closes in edx.
template <>
struct ByAddress<frontend::connect::raceNetAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 closes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, closes }));
    }
};

template <>
struct ByAddress<frontend::connect::raceNetClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::tcpClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::serialClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::modemClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::createGameClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::connect::refreshClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::connect::connectEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::connect::connectExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::connect::ipxFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::connect::ipxEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The held in eax.
template <>
struct ByAddress<frontend::connect::flushKeys>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 held)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ held }));
    }
};

// The field in eax, the text in edx, the third in ebx, the length in ecx.
template <>
struct ByAddress<frontend::connect::setEditField>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 field, x86::reg32 text,
                           x86::reg32 third, x86::reg32 length)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ field, text, third, length }));
    }
};

// The asked in eax.
template <>
struct ByAddress<frontend::connect::setPointer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 asked)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ asked }));
    }
};

// The kind in eax.
template <>
struct ByAddress<frontend::connect::connectionAvailable>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind }));
    }
};

// The lookFor in eax, the player in edx, the kind in ebx, the redraw in ecx.
template <>
struct ByAddress<frontend::connect::listSessions>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lookFor, x86::reg32 player,
                           x86::reg32 kind, x86::reg32 redraw)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ lookFor, player, kind, redraw }));
    }
};

// The name in eax, the kind in edx.
template <>
struct ByAddress<frontend::connect::createSession>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 kind)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, kind }));
    }
};

// The endpoint in eax.
template <>
struct ByAddress<frontend::connect::joinSession>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 endpoint)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ endpoint }));
    }
};

// The player in eax.
template <>
struct ByAddress<frontend::connect::useTcp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// The host in eax, the address in edx.
template <>
struct ByAddress<frontend::connect::findAddress>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 host, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ host, address }));
    }
};

// The address in eax, the connection in edx.
template <>
struct ByAddress<frontend::connect::setUpEndpoint>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address, x86::reg32 connection)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ address, connection }));
    }
};

// The baud in eax, the blocks in edx, the slots in ebx.
template <>
struct ByAddress<frontend::connect::enumerateServices>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 baud, x86::reg32 blocks,
                           x86::reg32 slots)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ baud, blocks, slots }));
    }
};

template <>
struct ByAddress<frontend::connect::findConnections>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The guid in eax, the name in edx.
template <>
struct ByAddress<frontend::connect::serviceFound>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 guid, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ guid, name }));
    }
};

// The comm in eax, the session in edx, the address in ecx, ebx as it is.
template <>
struct ByAddress<frontend::connect::sessionFound>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 comm, x86::reg32 session,
                           x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ comm, session, cpu.ebx, address }));
    }
};

// The peer in edx, eax as it is.
template <>
struct ByAddress<frontend::connect::queuePeerByName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 peer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, peer }));
    }
};

// The address in edx, eax as it is.
template <>
struct ByAddress<frontend::connect::queuePeer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, address }));
    }
};

template <>
struct ByAddress<frontend::connect::clearPeerSlots>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The address in eax.
template <>
struct ByAddress<frontend::connect::rememberPeer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ address }));
    }
};

template <>
struct ByAddress<frontend::connect::clearQueue>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The name in eax, the address in edx.
template <>
struct ByAddress<frontend::connect::enqueuePeer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 address)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, address }));
    }
};

// The service in eax.
template <>
struct ByAddress<frontend::connect::startConnection>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

// The list in eax.
template <>
struct ByAddress<frontend::connect::listModems>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list }));
    }
};

// The service in eax.
template <>
struct ByAddress<frontend::connect::setUpLink>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 service)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ service }));
    }
};

template <>
struct ByAddress<frontend::connect::shakeHands>
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
