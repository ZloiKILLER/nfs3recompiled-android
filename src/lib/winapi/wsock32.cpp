#include <winapi/wsock32.h>
#include <x86.h>
#include <lib/socket.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <string>
#include <vector>

/* Winsock 1.1 as the game uses it, on the host's sockets (lib/socket.h).
 *
 * TCP/IP races are the game's own protocol on a real TCP connection, so a phone
 * plays with the original game and the Modern Patch on a PC alike.
 *
 * IPX, which nothing carries any more, is carried on UDP inside the local
 * network, the way the game finds races there: a host advertises its race with
 * SAP on IPX socket 0x452 (sub_512ad0) and a player looking for races asks every
 * machine on the network (sub_5127d0), then the race runs on datagrams between
 * the machines' own sockets (sub_4f5c10).  An IPX socket is a UDP socket whose
 * port is its IPX socket number (0x452 is UDP 1106; a socket the game leaves to
 * the system gets the port the system gives), an IPX node is the machine's IPv4
 * address followed by two zero bytes, the network number is 0, and the
 * broadcast node goes to every network's broadcast address.  A datagram carries
 * the game's data and nothing else. */

namespace win32 { namespace wsock32
{

namespace
{

constexpr int kInterrupted = 10004;
constexpr int kFault = 10014;
constexpr int kInvalid = 10022;
constexpr int kWouldBlock = 10035;
constexpr int kNotSocket = 10038;
constexpr int kOperationNotSupported = 10045;
constexpr int kFamilyNotSupported = 10047;
constexpr int kHostNotFound = 11001;

constexpr x86::reg32 kInvalidSocket = 0xffffffff;
constexpr int kInetFamily = 2;
constexpr int kIpxFamily = 6;
constexpr int kInetLength = 16;
constexpr int kIpxLength = 14;

/* The name gethostname gives; gethostbyname of it is this machine's address on
 * the local network, which the TCP/IP screen shows the player (sub_5150f0). */
const char kHostName[] = "nfs3hp";

int failWith(int error)
{
    Socket::setLastError(error);
    return -1;
}

/* Guest memory for what Winsock hands back by pointer: inet_ntoa's text at +0,
 * gethostbyname's hostent at +64, its lists at +96 and +104, its address at
 * +120 and its name at +256. */
x86::reg32 scratch()
{
    static MemMap* s_scratch = new MemMap(4096);
    return s_scratch->getBlockStart();
}

Socket* find(WinApplication* app, SOCKET s)
{
    Socket* socket = dynamic_cast<Socket*>(app->getResource(s));
    if (!socket || socket->closed())
    {
        Socket::setLastError(kNotSocket);
        return nullptr;
    }
    return socket;
}

/* A call in progress on a socket: closesocket frees it only once there is none. */
class Use
{
public:
    explicit Use(Socket* socket) : m_socket(socket) { if (m_socket) ++m_socket->users; }
    ~Use() { if (m_socket) --m_socket->users; }
    Use(const Use&) = delete;
    Use& operator=(const Use&) = delete;
private:
    Socket* m_socket;
};

/* The guest lock let go while the calling thread waits on the network, so the
 * game's other threads run meanwhile -- as they do on Windows. */
class Unlocked
{
public:
    Unlocked(WinApplication* app, x86::CPU& cpu)
        :   m_app(app)
        ,   m_cpu(cpu)
        ,   m_held(static_cast<Mutex*>(app->getMemory<Mutex*>(cpu.efs + 8)) != nullptr)
    {
        if (m_held)
            m_app->unlockContext(m_cpu);
    }
    ~Unlocked()
    {
        if (m_held)
            m_app->lockContext(m_cpu);
    }
    Unlocked(const Unlocked&) = delete;
    Unlocked& operator=(const Unlocked&) = delete;
private:
    WinApplication* m_app;
    x86::CPU& m_cpu;
    bool m_held;
};

/* Waits for any of `events` on a socket: the events it has, or 0 once the
 * socket is closed under the wait. */
int wait(WinApplication* app, x86::CPU& cpu, Socket* socket, int events)
{
    Unlocked unlocked(app, cpu);
    for (;;)
    {
        if (socket->closed())
        {
            failWith(kInterrupted);
            return 0;
        }
        int have = 0;
        if (Socket::poll(&socket, &events, &have, 1, 50) < 0)
            return 0;
        if (have)
            return have;
    }
}

/* A call on a socket the game left blocking: tried, and if it would block,
 * tried again each time the socket is ready, until it is done or fails. */
template <typename Call>
int blocking(WinApplication* app, x86::CPU& cpu, Socket* socket, int event, Call call)
{
    for (;;)
    {
        const int result = call();
        if (result >= 0 || socket->nonBlocking || Socket::lastError() != kWouldBlock)
            return result;
        if (!wait(app, cpu, socket, event))
            return -1;
    }
}

const uint8_t* bytes(const sockaddr* address)
{
    return reinterpret_cast<const uint8_t*>(address->sa_data);
}

uint8_t* bytes(sockaddr* address)
{
    return reinterpret_cast<uint8_t*>(address->sa_data);
}

/* sockaddr_in: the port and then the address, both in network order. */
bool readInet(const sockaddr* address, int length, uint32_t* ip, uint16_t* port)
{
    if (!address || length < kInetLength)
        return failWith(kFault) == 0;
    if (address->sa_family != kInetFamily)
        return failWith(kFamilyNotSupported) == 0;
    const uint8_t* data = bytes(address);
    *port = uint16_t((data[0] << 8) | data[1]);
    std::memcpy(ip, data + 2, 4);
    return true;
}

void writeInet(sockaddr* address, uint32_t ip, uint16_t port)
{
    std::memset(address, 0, kInetLength);
    address->sa_family = kInetFamily;
    uint8_t* data = bytes(address);
    data[0] = uint8_t(port >> 8);
    data[1] = uint8_t(port);
    std::memcpy(data + 2, &ip, 4);
}

/* sockaddr_ipx: the network number, the node and the socket, all in network
 * order; the node's first four bytes are the machine's IPv4 address. */
struct IpxAddress
{
    uint32_t ip;
    uint16_t socket;
    bool broadcast;
};

bool readIpx(const sockaddr* address, int length, IpxAddress* ipx)
{
    if (!address || length < kIpxLength)
        return failWith(kFault) == 0;
    if (address->sa_family != kIpxFamily)
        return failWith(kFamilyNotSupported) == 0;
    const uint8_t* data = bytes(address);
    ipx->broadcast = true;
    for (int i = 4; i < 10; ++i)
        ipx->broadcast = ipx->broadcast && data[i] == 0xff;
    std::memcpy(&ipx->ip, data + 4, 4);
    ipx->socket = uint16_t((data[10] << 8) | data[11]);
    return true;
}

void writeIpx(sockaddr* address, uint32_t ip, uint16_t socket)
{
    std::memset(address, 0, kIpxLength);
    address->sa_family = kIpxFamily;
    uint8_t* data = bytes(address);
    std::memcpy(data + 4, &ip, 4);
    data[10] = uint8_t(socket >> 8);
    data[11] = uint8_t(socket);
}

const char* dotted(uint32_t ip, char (&text)[16])
{
    SDL_snprintf(text, sizeof(text), "%u.%u.%u.%u", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, ip >> 24);
    return text;
}

/* The first calls of each socket that moved data, into the log: how much was
 * asked, how much went, from or to where, and the first bytes of it -- enough
 * to see how far two machines got with each other.  16 of them, or 400 with
 * NFS_NET_TRACE (extra net_trace): an IPX join takes a few dozen before it
 * settles.  A call that would only have blocked is left out. */
unsigned tracedCalls()
{
    static const unsigned count = []() {
        const char* value = SDL_getenv("NFS_NET_TRACE");
        return value && *value && *value != '0' ? 400u : 16u;
    }();
    return count;
}

void traceData(Socket* socket, const char* what, x86::reg32 handle, const char* data, int asked, int result,
               uint32_t ip = 0, uint16_t port = 0)
{
    if (socket->traced >= tracedCalls() || (result < 0 && Socket::lastError() == kWouldBlock))
        return;
    ++socket->traced;
    char text[16];
    if (result < 0)
    {
        SDL_Log("[NET] %s 0x%x %d: error %d", what, unsigned(handle), asked, Socket::lastError());
        return;
    }
    char where[32] = "";
    if (ip)
        SDL_snprintf(where, sizeof(where), " %s:%u", dotted(ip, text), unsigned(port));
    char bytes[3 * 32 + 1] = "";
    const int shown = result < 32 ? result : 32;
    for (int i = 0; i < shown; ++i)
        SDL_snprintf(bytes + 3 * i, 4, "%02x ", unsigned(uint8_t(data[i])));
    SDL_Log("[NET] %s 0x%x %d: %d%s %s", what, unsigned(handle), asked, result, where, bytes);
}

}

int select(WinApplication* app, x86::CPU& cpu, int nfds,
           fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
           const timeval *timeout)
{
    NFS2_USE(nfds);
    std::vector<Socket*> sockets;
    std::vector<x86::reg32> handles;
    std::vector<int> want;
    auto collect = [&](const fd_set* set, int event) {
        if (!set)
            return true;
        const unsigned listed = set->fd_count < FD_SETSIZE_WSA ? set->fd_count : FD_SETSIZE_WSA;
        for (unsigned i = 0; i < listed; ++i)
        {
            const x86::reg32 handle = set->fd_array[i];
            size_t at = 0;
            while (at < handles.size() && handles[at] != handle)
                ++at;
            if (at == handles.size())
            {
                Socket* socket = find(app, handle);
                if (!socket)
                {
                    static unsigned s_reported = 0;
                    if (++s_reported <= 8)
                        SDL_Log("[NET] select: 0x%x is no socket", unsigned(handle));
                    return false;
                }
                sockets.push_back(socket);
                handles.push_back(handle);
                want.push_back(0);
            }
            want[at] |= event;
        }
        return true;
    };
    if (!collect(readfds, Socket::Readable) || !collect(writefds, Socket::Writable)
        || !collect(exceptfds, Socket::Failed))
        return -1;

    const int count = int(sockets.size());
    std::vector<int> have(sockets.size(), 0);
    /* Into the log each time a thread asks select about a different set of
     * sockets than it last did: what it waits for on each, R, W or E. */
    {
        thread_local std::string t_lastAsked;
        std::string asked;
        for (int i = 0; i < count; ++i)
        {
            char one[32];
            SDL_snprintf(one, sizeof(one), " 0x%x:%s%s%s", unsigned(handles[size_t(i)]),
                         (want[size_t(i)] & Socket::Readable) ? "R" : "", (want[size_t(i)] & Socket::Writable) ? "W" : "",
                         (want[size_t(i)] & Socket::Failed) ? "E" : "");
            asked += one;
        }
        if (asked != t_lastAsked)
        {
            static unsigned s_logged = 0;
            if (++s_logged <= 60)
                SDL_Log("[NET] select on%s", asked.c_str());
            t_lastAsked = asked;
        }
    }
    for (Socket* socket : sockets)
        ++socket->users;
    int ready = 0;
    if (!timeout || timeout->tv_sec > 0 || timeout->tv_usec > 0)
    {
        /* Waited out with the guest lock let go, a little at a time, so a
         * socket closed meanwhile is let go of soon. */
        const Uint64 start = SDL_GetTicks();
        const Uint64 limit = timeout ? Uint64(timeout->tv_sec > 0 ? timeout->tv_sec : 0) * 1000
                                           + Uint64(timeout->tv_usec > 0 ? timeout->tv_usec : 0) / 1000
                                     : ~Uint64(0);
        Unlocked unlocked(app, cpu);
        for (;;)
        {
            const Uint64 spent = SDL_GetTicks() - start;
            const Uint64 left = spent < limit ? limit - spent : 0;
            ready = Socket::poll(sockets.data(), want.data(), have.data(), count, int(left < 20 ? left : 20));
            if (ready != 0 || left == 0)
                break;
            bool closed = false;
            for (Socket* socket : sockets)
                closed = closed || socket->closed();
            if (closed)
                break;
        }
    }
    else
    {
        ready = Socket::poll(sockets.data(), want.data(), have.data(), count, 0);
    }
    for (Socket* socket : sockets)
        --socket->users;
    if (ready < 0)
        return -1;
    if (ready > 0)
    {
        static unsigned s_logged = 0;
        if (++s_logged <= 40)
        {
            std::string got;
            for (int i = 0; i < count; ++i)
            {
                if (!have[size_t(i)])
                    continue;
                char one[32];
                SDL_snprintf(one, sizeof(one), " 0x%x:%s%s%s", unsigned(handles[size_t(i)]),
                             (have[size_t(i)] & Socket::Readable) ? "R" : "", (have[size_t(i)] & Socket::Writable) ? "W" : "",
                             (have[size_t(i)] & Socket::Failed) ? "E" : "");
                got += one;
            }
            SDL_Log("[NET] select ready%s", got.c_str());
        }
    }

    /* Each set keeps the sockets that have its event; the result counts a
     * socket once for every set it stays in. */
    int total = 0;
    auto keep = [&](fd_set* set, int event) {
        if (!set)
            return;
        const unsigned listed = set->fd_count < FD_SETSIZE_WSA ? set->fd_count : FD_SETSIZE_WSA;
        unsigned kept = 0;
        for (unsigned i = 0; i < listed; ++i)
        {
            const x86::reg32 handle = set->fd_array[i];
            for (size_t at = 0; at < handles.size(); ++at)
            {
                if (handles[at] == handle && (have[at] & event))
                {
                    set->fd_array[kept++] = handle;
                    break;
                }
            }
        }
        set->fd_count = kept;
        total += int(kept);
    };
    keep(readfds, Socket::Readable);
    keep(writefds, Socket::Writable);
    keep(exceptfds, Socket::Failed);
    return total;
}

int sendto(WinApplication* app, x86::CPU& cpu, SOCKET s, const char* buf, int len,
           int flags, const sockaddr* to, int tolen)
{
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    Use use(socket);
    if (socket->ipx)
    {
        IpxAddress address;
        if (!readIpx(to, tolen, &address))
            return -1;
        if (address.broadcast)
        {
            /* Every machine of every network this one is on. */
            int sent = -1;
            for (uint32_t ip : Socket::broadcastAddresses())
            {
                sent = socket->sendTo(buf, len, flags, ip, address.socket);
                traceData(socket, "broadcast", s, buf, len, sent, ip, address.socket);
            }
            return sent;
        }
        const int sent = blocking(app, cpu, socket, Socket::Writable, [&]() {
            return socket->sendTo(buf, len, flags, address.ip, address.socket);
        });
        traceData(socket, "sendto", s, buf, len, sent, address.ip, address.socket);
        return sent;
    }
    uint32_t ip = 0;
    uint16_t port = 0;
    if (!readInet(to, tolen, &ip, &port))
        return -1;
    const int sent = blocking(app, cpu, socket, Socket::Writable, [&]() {
        return socket->sendTo(buf, len, flags, ip, port);
    });
    traceData(socket, "sendto", s, buf, len, sent, ip, port);
    return sent;
}

int getsockname(WinApplication* app, x86::CPU& cpu,
                SOCKET s, sockaddr* name, int* namelen)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    const int length = socket->ipx ? kIpxLength : kInetLength;
    if (!name || !namelen || *namelen < length)
        return failWith(kFault);
    uint32_t ip = 0;
    uint16_t port = 0;
    if (socket->localName(&ip, &port) < 0)
        return -1;
    if (socket->ipx)
        writeIpx(name, Socket::localAddress(), port);
    else
        writeInet(name, ip, port);
    *namelen = length;
    return 0;
}

int bind(WinApplication* app, x86::CPU& cpu,
         SOCKET s, const sockaddr* name, int namelen)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    char text[16];
    if (socket->ipx)
    {
        IpxAddress address;
        if (!readIpx(name, namelen, &address))
            return -1;
        const int result = socket->bind(0, address.socket);
        uint16_t port = address.socket;
        socket->localName(nullptr, &port);
        SDL_Log("[NET] IPX socket 0x%04x on UDP %u at %s: %s", unsigned(address.socket), unsigned(port),
                dotted(Socket::localAddress(), text), result < 0 ? "failed" : "bound");
        return result;
    }
    uint32_t ip = 0;
    uint16_t port = 0;
    if (!readInet(name, namelen, &ip, &port))
        return -1;
    const int result = socket->bind(ip, port);
    if (result < 0)
        SDL_Log("[NET] bind %s:%u failed: %d", dotted(ip, text), unsigned(port), Socket::lastError());
    return result;
}

unsigned short htons(WinApplication* app, x86::CPU& cpu, unsigned short hostshort)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return (hostshort >> 8) | (hostshort << 8);
}

unsigned short ntohs(WinApplication* app, x86::CPU& cpu, unsigned short netshort)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return (netshort >> 8) | (netshort << 8);
}

unsigned long inet_addr(WinApplication* app, x86::CPU& cpu, const char *cp)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return Socket::parseAddress(cp);
}

Packed<char> inet_ntoa(WinApplication* app, x86::CPU& cpu, x86::reg32 in)
{
    NFS2_USE(cpu);
    const x86::reg32 at = scratch();
    char text[16];
    std::memcpy(&app->getMemory<char>(at), dotted(in, text), sizeof(text));
    return Packed<char>(at);
}

int gethostname(WinApplication* app, x86::CPU& cpu, char *name, int namelen)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (!name || namelen < int(sizeof(kHostName)))
        return failWith(kFault);
    std::memcpy(name, kHostName, sizeof(kHostName));
    return 0;
}

int setsockopt(WinApplication* app, x86::CPU& cpu,
               SOCKET s, int level, int optname, const char *optval, int optlen)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    if (!optval || optlen < 0)
        return failWith(kFault);
    return socket->setOption(level, optname, optval, optlen);
}

int closesocket(WinApplication* app, x86::CPU& cpu, SOCKET s)
{
    Socket* socket = dynamic_cast<Socket*>(app->getResource(s));
    if (!socket || socket->closed())
        return failWith(kNotSocket);
    socket->close();
    SDL_Log("[NET] closing 0x%x%s", unsigned(s), socket->users.load() > 0 ? ", a thread still waits on it" : "");
    if (socket->users.load() > 0)
    {
        /* A thread still waits on it -- the IPX receive thread, whose socket
         * the game closes to stop it: it wakes within a wait's slice. */
        Unlocked unlocked(app, cpu);
        const Uint64 start = SDL_GetTicks();
        bool said = false;
        while (socket->users.load() > 0)
        {
            SDL_Delay(1);
            if (!said && SDL_GetTicks() - start > 2000)
            {
                SDL_Log("[NET] closing 0x%x: still waited on after two seconds", unsigned(s));
                said = true;
            }
        }
    }
    app->freeResource(s);
    return 0;
}

SOCKET socket(WinApplication* app, x86::CPU& cpu, int af, int type, int protocol)
{
    NFS2_USE(cpu);
    NFS2_USE(protocol);
    Socket::Kind kind = Socket::Stream;
    bool ipx = false;
    if (af == kInetFamily && type == 1)
        kind = Socket::Stream;
    else if (af == kInetFamily && type == 2)
        kind = Socket::Datagram;
    else if (af == kIpxFamily && type == 2)
    {
        kind = Socket::Datagram;
        ipx = true;
    }
    else
        return x86::reg32(failWith(kFamilyNotSupported));
    Socket* result = Socket::open(kind);
    if (!result)
        return kInvalidSocket;
    if (ipx)
    {
        /* A race is found by broadcasting on the local network. */
        const int32_t on = 1;
        result->ipx = true;
        result->setOption(0xffff, 0x20, &on, int(sizeof(on)));
    }
    return app->allocateResource(result);
}

int WSACleanup(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

int WSAStartup(WinApplication* app, x86::CPU& cpu, WORD wVersionRequired, LPWSADATA lpWSAData)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(wVersionRequired);
    if (!lpWSAData)
        return kFault;
    lpWSAData->wVersion = 0x0101;
    lpWSAData->wHighVersion = 0x0101;
    lpWSAData->iMaxSockets = 64;
    lpWSAData->iMaxUdpDg = 1600;
    lpWSAData->lpVendorInfo = Packed<char>();
    lpWSAData->szDescription[0] = 0;
    lpWSAData->szSystemStatus[0] = 0;
    return Socket::startup() ? 0 : 10091; // WSASYSNOTREADY
}

int recvfrom(WinApplication* app, x86::CPU& cpu, SOCKET s, char* buf, int len,
             int flags, sockaddr* from, int* fromlen)
{
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    Use use(socket);
    uint32_t ip = 0;
    uint16_t port = 0;
    const int got = blocking(app, cpu, socket, Socket::Readable, [&]() {
        return socket->recvFrom(buf, len, flags, &ip, &port);
    });
    traceData(socket, "recvfrom", s, buf, len, got, ip, port);
    if (got >= 0 && from && fromlen)
    {
        const int length = socket->ipx ? kIpxLength : kInetLength;
        if (*fromlen >= length)
        {
            if (socket->ipx)
                writeIpx(from, ip, port);
            else
                writeInet(from, ip, port);
            *fromlen = length;
        }
    }
    return got;
}

Packed<hostent> gethostbyname(WinApplication* app, x86::CPU& cpu, const char *name)
{
    uint32_t address = 0;
    bool found = false;
    if (name && SDL_strcasecmp(name, kHostName) == 0)
    {
        address = Socket::localAddress();
        found = true;
    }
    else
    {
        Unlocked unlocked(app, cpu);
        found = Socket::resolve(name, &address);
    }
    if (!found)
    {
        Socket::setLastError(kHostNotFound);
        return Packed<hostent>();
    }
    const x86::reg32 at = scratch();
    const x86::reg32 entry = at + 64;
    const x86::reg32 aliases = at + 96;
    const x86::reg32 addresses = at + 104;
    const x86::reg32 first = at + 120;
    const x86::reg32 hostName = at + 256;
    SDL_strlcpy(&app->getMemory<char>(hostName), name ? name : "", 256);
    app->getMemory<x86::reg32>(entry) = hostName;
    app->getMemory<x86::reg32>(entry + 4) = aliases;
    app->getMemory<x86::reg16>(entry + 8) = kInetFamily;
    app->getMemory<x86::reg16>(entry + 10) = 4;
    app->getMemory<x86::reg32>(entry + 12) = addresses;
    app->getMemory<x86::reg32>(aliases) = 0;
    app->getMemory<x86::reg32>(addresses) = first;
    app->getMemory<x86::reg32>(addresses + 4) = 0;
    app->getMemory<x86::reg32>(first) = address;
    return Packed<hostent>(entry);
}

int ioctlsocket(WinApplication* app, x86::CPU& cpu, SOCKET s,
                x86::sreg32 cmd, x86::reg32* argp)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    if (!argp)
        return failWith(kFault);
    switch (x86::reg32(cmd))
    {
    case 0x8004667e: // FIONBIO
        socket->nonBlocking = *argp != 0;
        return 0;
    case 0x4004667f: // FIONREAD
    {
        const int waiting = socket->pending();
        if (waiting < 0)
            return -1;
        *argp = x86::reg32(waiting);
        return 0;
    }
    case 0x40047307: // SIOCATMARK: no urgent data
        *argp = 1;
        return 0;
    default:
        return failWith(kInvalid);
    }
}

int listen(WinApplication* app, x86::CPU& cpu, SOCKET s, int backlog)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    uint16_t port = 0;
    socket->localName(nullptr, &port);
    const int result = socket->listen(backlog);
    char text[16];
    SDL_Log("[NET] hosting on TCP %u at %s: %s", unsigned(port), dotted(Socket::localAddress(), text),
            result < 0 ? "failed" : "listening");
    return result;
}

int shutdown(WinApplication* app, x86::CPU& cpu, SOCKET s, int how)
{
    NFS2_USE(cpu);
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    return socket->shutdown(how);
}

int connect(WinApplication* app, x86::CPU& cpu, SOCKET s,
            const sockaddr *name, int namelen)
{
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    if (socket->ipx)
        return failWith(kOperationNotSupported);
    Use use(socket);
    uint32_t ip = 0;
    uint16_t port = 0;
    if (!readInet(name, namelen, &ip, &port))
        return -1;
    char text[16];
    SDL_Log("[NET] connecting to %s:%u", dotted(ip, text), unsigned(port));
    const int result = socket->connect(ip, port);
    if (result >= 0 || socket->nonBlocking || Socket::lastError() != kWouldBlock)
        return result;
    /* A blocking connect: until it is made or refused. */
    const int outcome = wait(app, cpu, socket, Socket::Writable | Socket::Failed);
    if (!outcome)
        return -1;
    if (outcome & Socket::Failed)
        return failWith(socket->connectError());
    return 0;
}

SOCKET accept(WinApplication* app, x86::CPU& cpu, SOCKET s,
              sockaddr *addr, int* addrlen)
{
    Socket* listening = find(app, s);
    if (!listening)
        return kInvalidSocket;
    Use use(listening);
    Socket* accepted = nullptr;
    uint32_t ip = 0;
    uint16_t port = 0;
    const int result = blocking(app, cpu, listening, Socket::Readable, [&]() {
        accepted = listening->accept(&ip, &port);
        return accepted ? 0 : -1;
    });
    if (result < 0 || !accepted)
        return kInvalidSocket;
    if (addr && addrlen && *addrlen >= kInetLength)
    {
        writeInet(addr, ip, port);
        *addrlen = kInetLength;
    }
    char text[16];
    SDL_Log("[NET] %s:%u joined", dotted(ip, text), unsigned(port));
    return app->allocateResource(accepted);
}

int __WSAFDIsSet(WinApplication* app, x86::CPU& cpu, SOCKET fd, fd_set* set)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (!set)
        return 0;
    const unsigned count = set->fd_count < FD_SETSIZE_WSA ? set->fd_count : FD_SETSIZE_WSA;
    for (unsigned i = 0; i < count; ++i)
    {
        if (set->fd_array[i] == fd)
            return 1;
    }
    return 0;
}

int send(WinApplication* app, x86::CPU& cpu, SOCKET s, const char* buf, int len, int flags)
{
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    Use use(socket);
    const int sent = blocking(app, cpu, socket, Socket::Writable, [&]() {
        return socket->send(buf, len, flags);
    });
    traceData(socket, "send", s, buf, len, sent);
    return sent;
}

int recv(WinApplication* app, x86::CPU& cpu, SOCKET s, char* buf, int len, int flags)
{
    Socket* socket = find(app, s);
    if (!socket)
        return -1;
    Use use(socket);
    const int got = blocking(app, cpu, socket, Socket::Readable, [&]() {
        return socket->recv(buf, len, flags);
    });
    traceData(socket, (flags & 2) ? "peek" : "recv", s, buf, len, got);
    return got;
}

int WSAGetLastError(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return Socket::lastError();
}

}}
