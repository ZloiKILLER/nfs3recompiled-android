#ifndef LIB_SOCKET_H_
#define LIB_SOCKET_H_

#include <lib/winapp.h>
#include <atomic>
#include <cstdint>
#include <vector>

namespace win32
{

/* One of the game's sockets, on one of the host's (src/lib/winapi/wsock32.cpp).
 *
 * The host's socket is always non-blocking.  A Winsock call the game makes on a
 * blocking socket waits in wait() instead, so the caller can let the guest lock
 * go meanwhile, and closing the socket wakes a thread still waiting on it -- the
 * game's IPX receive thread sits in recvfrom until its socket is closed.
 *
 * Addresses are IPv4 in network order, ports in host order.  Errors are
 * Winsock's codes, kept per thread as WSAGetLastError keeps them. */
class Socket : public GenericResource
{
public:
    enum Kind { Stream, Datagram };
    enum Event { Readable = 1, Writable = 2, Failed = 4 };

    /* A new socket, or nullptr with the error set. */
    static Socket* open(Kind kind);
    ~Socket();

    Kind kind() const { return m_kind; }

    int bind(uint32_t address, uint16_t port);
    int listen(int backlog);
    Socket* accept(uint32_t* address, uint16_t* port);
    /* A connection under way reads as WSAEWOULDBLOCK, even when the host made
     * it at once: Winsock never does, and the game counts on it (0x4f6ec6). */
    int connect(uint32_t address, uint16_t port);
    int send(const void* data, int size, int flags);
    int recv(void* data, int size, int flags);
    int sendTo(const void* data, int size, int flags, uint32_t address, uint16_t port);
    int recvFrom(void* data, int size, int flags, uint32_t* address, uint16_t* port);
    int localName(uint32_t* address, uint16_t* port);
    int shutdown(int how);
    /* FIONREAD: bytes waiting, or -1. */
    int pending();
    /* setsockopt with Winsock's level and name; what the host has no use for
     * is taken and dropped. */
    int setOption(int level, int name, const void* value, int size);

    /* The events of `want` (Event bits) each socket has, waiting up to
     * `milliseconds` (0: none, -1: no limit) for any: the number of sockets
     * with one, or -1.  A connection under way is Writable once made and
     * Failed if refused, as Winsock's select has it. */
    static int poll(Socket* const* sockets, const int* want, int* have, int count, int milliseconds);

    /* Why a connection under way failed, once poll() has seen it fail. */
    int connectError() const { return m_connectError; }

    /* closesocket: every wait on the socket ends, and a new one fails. */
    void close() { m_closed = true; }
    bool closed() const { return m_closed; }

    /* Calls in progress on the socket; closesocket waits for none before the
     * socket is freed. */
    std::atomic<int> users{0};

    /* FIONBIO, the game's choice; the host's socket is non-blocking either way. */
    bool nonBlocking = false;

    /* Opened by the game as IPX: a datagram socket whose addresses Winsock's
     * callers see as IPX's (src/lib/winapi/wsock32.cpp). */
    bool ipx = false;

    /* How many of its calls have gone into the log: the first few of each
     * socket do, which is what shows how a connection got on. */
    unsigned traced = 0;

    static int lastError();
    static void setLastError(int error);

    /* The address other machines on the local network reach this one at, and
     * where a broadcast to all of them goes. */
    static uint32_t localAddress();
    static std::vector<uint32_t> broadcastAddresses();
    /* A host name or a dotted address; waits on the network for a name. */
    static bool resolve(const char* name, uint32_t* address);
    /* inet_addr: a dotted address, or 0xffffffff. */
    static uint32_t parseAddress(const char* text);

    static bool startup();

private:
    Socket(Kind kind, intptr_t handle);
    int fail();

private:
    Kind m_kind;
    intptr_t m_handle;
    /* Set by the thread that connects, settled by the one that selects. */
    std::atomic<bool> m_connecting{false};
    std::atomic<int> m_connectError{0};
    std::atomic<bool> m_closed{false};
};

}

#endif
