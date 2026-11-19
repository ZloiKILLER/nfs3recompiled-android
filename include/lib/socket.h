#ifndef LIB_SOCKET_H_
#define LIB_SOCKET_H_

#include <lib/app.h>
#include <atomic>
#include <cstdint>
#include <vector>

namespace nfs
{

/* Why a call on a socket failed. */
enum class SocketError
{
    none,
    interrupted,
    accessDenied,
    fault,
    invalid,
    tooManySockets,
    wouldBlock,             // nothing now; a connection under way says so too
    already,                // a connection already under way
    notSocket,
    destinationRequired,
    messageSize,
    protocolType,
    noProtocolOption,
    protocolNotSupported,
    operationNotSupported,
    familyNotSupported,
    addressInUse,
    addressNotAvailable,
    networkDown,
    networkUnreachable,
    networkReset,
    connectionAborted,
    connectionReset,
    noBuffers,
    isConnected,
    notConnected,
    shutDown,
    timedOut,
    connectionRefused,
    hostDown,
    hostUnreachable,
    systemNotReady,
    hostNotFound,
};

/* One of the game's sockets, on one of the host's (src/nfs3hp/os_net.cpp).
 *
 * The host's socket is always non-blocking.  A call the game makes on a
 * blocking socket waits in poll() instead, so the caller can let the guest lock
 * go meanwhile, and closing the socket wakes a thread still waiting on it -- the
 * game's IPX receive thread sits in its receive until its socket is closed.
 *
 * Addresses are IPv4 in network order, ports in host order.  A call that fails
 * leaves why, kept per thread, for lastError(). */
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
    /* A connection under way fails with wouldBlock, even when the host made
     * it at once: the game counts on that (0x4f6ec6). */
    int connect(uint32_t address, uint16_t port);
    int send(const void* data, int size);
    /* `peek` leaves what it read on the socket. */
    int recv(void* data, int size, bool peek);
    int sendTo(const void* data, int size, uint32_t address, uint16_t port);
    int recvFrom(void* data, int size, uint32_t* address, uint16_t* port);
    int localName(uint32_t* address, uint16_t* port);
    /* Both ways shut. */
    int shutdown();
    /* Bytes waiting, or -1. */
    int pending();
    /* Broadcasts let through, and sends without waiting to gather small ones. */
    int setBroadcast(bool on);
    int setNoDelay(bool on);

    /* The events of `want` (Event bits) each socket has, waiting up to
     * `milliseconds` (0: none, -1: no limit) for any: the number of sockets
     * with one, or -1.  A connection under way is Writable once made and
     * Failed if refused. */
    static int poll(Socket* const* sockets, const int* want, int* have, int count, int milliseconds);

    /* Why a connection under way failed, once poll() has seen it fail. */
    SocketError connectError() const { return m_connectError; }

    /* Closed by the game: every wait on the socket ends, and a new one fails. */
    void close() { m_closed = true; }
    bool closed() const { return m_closed; }

    /* Calls in progress on the socket; closing waits for none before the
     * socket is freed. */
    std::atomic<int> users{0};

    /* The game's choice of not waiting; the host's socket is non-blocking either way. */
    bool nonBlocking = false;

    /* Opened by the game as IPX: a datagram socket whose addresses the game
     * sees as IPX's (src/nfs3hp/os_net.cpp). */
    bool ipx = false;

    /* How many of its calls have gone into the log: the first few of each
     * socket do, which is what shows how a connection got on. */
    unsigned traced = 0;

    static SocketError lastError();
    static void setLastError(SocketError error);

    /* The address other machines on the local network reach this one at, and
     * where a broadcast to all of them goes. */
    static uint32_t localAddress();
    static std::vector<uint32_t> broadcastAddresses();
    /* A host name or a dotted address; waits on the network for a name. */
    static bool resolve(const char* name, uint32_t* address);
    /* A dotted address, or 0xffffffff. */
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
    std::atomic<SocketError> m_connectError{SocketError::none};
    std::atomic<bool> m_closed{false};
};

}

#endif
