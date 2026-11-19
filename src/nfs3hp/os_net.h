#ifndef NFS3HP_OS_NET_H
#define NFS3HP_OS_NET_H

#include "os_system.h"

#include <lib/socket.h>

#include <cstdint>
#include <optional>
#include <string>

/* The port's network, as the game's code needs it, on src/lib's Socket: TCP connections, and IPX
 * carried on UDP inside the local network.  A socket is held by a handle (os_system.h) the game
 * keeps in its memory.  A call that can wait on the network lets the other threads run while it
 * waits.
 *
 * TCP/IP races are the game's own protocol on a real TCP connection, so a phone plays with the
 * original game and the Modern Patch on a PC alike.
 *
 * IPX, which nothing carries any more, is carried on UDP inside the local network, the way the
 * game finds races there: a host advertises its race with SAP on IPX socket 0x452 (sub_512ad0)
 * and a player looking for races asks every machine on the network (sub_5127d0), then the race
 * runs on datagrams between the machines' own sockets (sub_4f5c10).  An IPX socket is a UDP
 * socket whose port is its IPX socket number (0x452 is UDP 1106; a socket the game leaves to the
 * system gets the port the system gives), an IPX node is the machine's IPv4 address followed by
 * two zero bytes, the network number is 0, and the broadcast node goes to every network's
 * broadcast address.  A datagram carries the game's data and nothing else. */
namespace nfs3hp
{
namespace os
{
namespace net
{

/* Why a call failed. */
using Error = nfs::SocketError;

/* What a call that moves data or counts sockets gives: the count, or -1 and why it failed. */
struct Result
{
    int value = 0;
    Error error = Error::none;

    bool ok() const { return error == Error::none; }
    static Result failed(Error why) { return Result{-1, why}; }
};

/* The number the game keeps for no socket. */
constexpr Handle kNoSocket = 0xffffffff;

/* ---- Addresses ---- */

/* The kinds of address the game's records hold, by the word they start with. */
enum class Family
{
    other,
    inet,
    ipx,
};

/* A machine's place on the network: its IPv4 address in network order and a port in host order.
 * For IPX the address is the node's first four bytes and the port the IPX socket number; a node
 * of all ones is every machine (`everyone`). */
struct Endpoint
{
    Family family = Family::inet;
    std::uint32_t ip = 0;
    std::uint16_t port = 0;
    bool everyone = false;

    static Endpoint inet(std::uint32_t ip, std::uint16_t port) { return Endpoint{Family::inet, ip, port, false}; }
    static Endpoint ipx(std::uint32_t ip, std::uint16_t socket) { return Endpoint{Family::ipx, ip, socket, false}; }
    static Endpoint ipxEveryone(std::uint16_t socket) { return Endpoint{Family::ipx, 0, socket, true}; }
};

/* A port number in the order it goes on the network, and back (the same swap both ways). */
constexpr std::uint16_t networkOrder(std::uint16_t value)
{
    return std::uint16_t((value >> 8) | (value << 8));
}

/* An address as the game keeps it in its memory, compares byte by byte and hands to other
 * machines.  The internet's, 16 bytes: the word 2, the port in network order, the IPv4 address,
 * zeros.  IPX's, 14 bytes: the word 6, the network number, the node's six bytes, the socket in
 * network order. */
constexpr nfs::u32 kInetBytes = 16;
constexpr nfs::u32 kIpxBytes = 14;

/* The address at `at` read as the internet's, or IPX's; its family says what its first word
 * was. */
Endpoint inetAt(App* app, nfs::u32 at);
Endpoint ipxAt(App* app, nfs::u32 at);

/* `endpoint` written at `at`, the whole of the address's bytes. */
void putInet(App* app, nfs::u32 at, const Endpoint& endpoint);
void putIpx(App* app, nfs::u32 at, const Endpoint& endpoint);

/* The address other machines on the local network reach this one at. */
std::uint32_t localAddress();

/* This machine's name; resolve() of it is localAddress(). */
constexpr const char* kHostName = "nfs3hp";

/* A dotted address or a machine's name as an address, waited for on the network; nullopt for
 * none found. */
std::optional<std::uint32_t> resolve(App* app, nfs::ThreadState& ts, const char* name);

/* An address as its dotted text. */
std::string addressText(std::uint32_t ip);

/* ---- The network ---- */

/* The network made ready: false when the host has none.  Each start needs no stop; stop() is
 * there for the game's count of its users and has nothing to let go. */
bool start();
void stop();

/* ---- Sockets ---- */

enum class Kind
{
    tcp,
    ipx,
};

/* A new socket of `kind`, waiting on each call until it is done; nullopt when there can be none.
 * An IPX socket lets broadcasts through: a race is found by broadcasting on the local network. */
std::optional<Handle> open(App* app, Kind kind);

/* `socket` closed, once the calls still waiting on it (the IPX receive thread's) have ended. */
Error close(App* app, nfs::ThreadState& ts, Handle socket);

/* `socket` given its own address: an IPX socket only its socket number (0 for one the system
 * picks), a TCP one its address and port. */
Error bind(App* app, Handle socket, const Endpoint& own);

/* The address `socket` has, as the other machines see it. */
std::optional<Endpoint> localName(App* app, Handle socket);

/* `socket` listening for connections, `backlog` of them waiting at most. */
Error listen(App* app, Handle socket, int backlog);

/* A connection taken from the listening `socket`, waited for if `socket` waits: the new socket,
 * which waits or not as `socket` does, and whom it is with. */
struct Accepted
{
    Handle socket;
    Endpoint peer;
};
std::optional<Accepted> accept(App* app, nfs::ThreadState& ts, Handle socket);

/* A TCP connection to `to` started.  On a socket that does not wait it is under way when this
 * gives wouldBlock (it never says it is made at once); select() then sees it made (writing) or
 * refused (failing).  On one that waits, none once made, the reason once refused. */
Error connect(App* app, nfs::ThreadState& ts, Handle socket, const Endpoint& to);

/* Both ways of a connection shut. */
Error shutdown(App* app, Handle socket);

/* `size` bytes of `data` sent on a connection: how many went. */
Result send(App* app, nfs::ThreadState& ts, Handle socket, const void* data, nfs::u32 size);

/* Up to `size` bytes received from a connection into `data`: how many came, 0 when the other
 * side closed it; `peek` leaves them on the socket. */
Result receive(App* app, nfs::ThreadState& ts, Handle socket, void* data, nfs::u32 size, bool peek);

/* A datagram of `size` bytes sent to `to`; to everyone, it goes to each network this machine is
 * on.  How many went. */
Result sendTo(App* app, nfs::ThreadState& ts, Handle socket, const void* data, nfs::u32 size, const Endpoint& to);

/* A datagram received into `data`, `size` bytes at most, its sender to `from`: how many came. */
Result receiveFrom(App* app, nfs::ThreadState& ts, Handle socket, void* data, nfs::u32 size, Endpoint& from);

/* Whether `socket`'s calls wait until done (false: they fail with wouldBlock instead). */
Error setWaits(App* app, Handle socket, bool waits);

/* How many bytes wait to be received on `socket`. */
Result waiting(App* app, Handle socket);

/* Broadcasts let through `socket`. */
Error setBroadcast(App* app, Handle socket, bool on);

/* A TCP socket's small sends sent at once, not gathered. */
Error setNoDelay(App* app, Handle socket, bool on);

/* Sockets waited on with select(), as the game keeps them in its memory: a count, then the
 * sockets. */
constexpr unsigned kSetSize = 64;
struct SocketSet
{
    std::uint32_t count;
    Handle sockets[kSetSize];
};

/* Whether `socket` is in `set`. */
bool contains(const SocketSet& set, Handle socket);

/* Waits up to `milliseconds` (nullopt: without end, 0: not at all) for any socket of `reading` to
 * have something to receive (or a connection to accept), of `writing` to take more (or a
 * connection under way to be made), of `failing` to have failed (or a connection under way to be
 * refused); any set may be null.  Each set is left with the sockets that have its event; the
 * value counts a socket once for every set it stays in. */
Result select(App* app, nfs::ThreadState& ts, SocketSet* reading, SocketSet* writing, SocketSet* failing,
              std::optional<nfs::u32> milliseconds);

}  // namespace net

/* ---- The serial port ---- */

/* A port is the file COMn the game opens (os_files.h); the host has no serial line behind it, so
 * each of these fails (and asserts): the game gets to them only with such a file there. */
namespace serial
{

enum class Parity
{
    none,
    odd,
    even,
};

struct LineSettings
{
    nfs::u32 speed = 0;
    Parity parity = Parity::none;
    bool twoStopBits = false;
    nfs::u32 wordBits = 8;
};

/* The port's line settings; nullopt when they cannot be read. */
std::optional<LineSettings> settings(App* app, Handle port);

/* The port's line set as `line` says: false when it cannot be. */
bool setSettings(App* app, Handle port, const LineSettings& line);

/* A read on the port given up after `milliseconds` in all: false when it cannot be set. */
bool setReadTimeout(App* app, Handle port, nfs::u32 milliseconds);

/* What the port has received, or has still to send, thrown away. */
bool drop(App* app, Handle port, bool input, bool output);

/* How many bytes a read (`input`) or write under way on the port moved once it is done; nullopt
 * while it is not, or when none is under way. */
std::optional<nfs::u32> finished(App* app, Handle port, bool input);

}  // namespace serial
}  // namespace os
}  // namespace nfs3hp

#endif
