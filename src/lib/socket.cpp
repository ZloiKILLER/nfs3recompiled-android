#include <lib/socket.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <SDL3/SDL.h>
#include <cstring>

namespace nfs
{

namespace
{

thread_local SocketError t_lastError = SocketError::none;

#ifdef _WIN32

typedef SOCKET Native;
const Native kNoSocket = INVALID_SOCKET;
constexpr int kSendFlags = 0;
constexpr int kShutBoth = SD_BOTH;
typedef int AddressLength;

/* The system's code for a failure as the layer's. */
SocketError errorOf(int error)
{
    switch (error)
    {
    case WSAEINTR:           return SocketError::interrupted;
    case WSAEACCES:          return SocketError::accessDenied;
    case WSAEFAULT:          return SocketError::fault;
    case WSAEINVAL:          return SocketError::invalid;
    case WSAEMFILE:          return SocketError::tooManySockets;
    case WSAEWOULDBLOCK:
    case WSAEINPROGRESS:     return SocketError::wouldBlock;
    case WSAEALREADY:        return SocketError::already;
    case WSAENOTSOCK:        return SocketError::notSocket;
    case WSAEDESTADDRREQ:    return SocketError::destinationRequired;
    case WSAEMSGSIZE:        return SocketError::messageSize;
    case WSAEPROTOTYPE:      return SocketError::protocolType;
    case WSAENOPROTOOPT:     return SocketError::noProtocolOption;
    case WSAEPROTONOSUPPORT: return SocketError::protocolNotSupported;
    case WSAEOPNOTSUPP:      return SocketError::operationNotSupported;
    case WSAEAFNOSUPPORT:    return SocketError::familyNotSupported;
    case WSAEADDRINUSE:      return SocketError::addressInUse;
    case WSAEADDRNOTAVAIL:   return SocketError::addressNotAvailable;
    case WSAENETUNREACH:     return SocketError::networkUnreachable;
    case WSAENETRESET:       return SocketError::networkReset;
    case WSAECONNABORTED:    return SocketError::connectionAborted;
    case WSAECONNRESET:      return SocketError::connectionReset;
    case WSAENOBUFS:         return SocketError::noBuffers;
    case WSAEISCONN:         return SocketError::isConnected;
    case WSAENOTCONN:        return SocketError::notConnected;
    case WSAESHUTDOWN:       return SocketError::shutDown;
    case WSAETIMEDOUT:       return SocketError::timedOut;
    case WSAECONNREFUSED:    return SocketError::connectionRefused;
    case WSAEHOSTDOWN:       return SocketError::hostDown;
    case WSAEHOSTUNREACH:    return SocketError::hostUnreachable;
    default:                 return SocketError::networkDown;
    }
}

SocketError nativeError()
{
    return errorOf(WSAGetLastError());
}

void closeNative(Native s)
{
    ::closesocket(s);
}

bool makeNonBlocking(Native s)
{
    u_long on = 1;
    return ::ioctlsocket(s, FIONBIO, &on) == 0;
}

#else

typedef int Native;
const Native kNoSocket = -1;
/* A connection the other side dropped must fail the call, not raise SIGPIPE. */
constexpr int kSendFlags = MSG_NOSIGNAL;
constexpr int kShutBoth = SHUT_RDWR;
typedef socklen_t AddressLength;

/* errno as the layer's failure. */
SocketError errorOf(int error)
{
    switch (error)
    {
    case EINTR:           return SocketError::interrupted;
    case EACCES:
    case EPERM:           return SocketError::accessDenied;
    case EFAULT:          return SocketError::fault;
    case EINVAL:          return SocketError::invalid;
    case EMFILE:
    case ENFILE:          return SocketError::tooManySockets;
    case EAGAIN:
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
    case EINPROGRESS:     return SocketError::wouldBlock;
    case EALREADY:        return SocketError::already;
    case EBADF:
    case ENOTSOCK:        return SocketError::notSocket;
    case EDESTADDRREQ:    return SocketError::destinationRequired;
    case EMSGSIZE:        return SocketError::messageSize;
    case EPROTOTYPE:      return SocketError::protocolType;
    case ENOPROTOOPT:     return SocketError::noProtocolOption;
    case EPROTONOSUPPORT: return SocketError::protocolNotSupported;
    case EOPNOTSUPP:      return SocketError::operationNotSupported;
    case EAFNOSUPPORT:    return SocketError::familyNotSupported;
    case EADDRINUSE:      return SocketError::addressInUse;
    case EADDRNOTAVAIL:   return SocketError::addressNotAvailable;
    case ENETDOWN:        return SocketError::networkDown;
    case ENETUNREACH:     return SocketError::networkUnreachable;
    case ENETRESET:       return SocketError::networkReset;
    case ECONNABORTED:    return SocketError::connectionAborted;
    case ECONNRESET:
    case EPIPE:           return SocketError::connectionReset;
    case ENOBUFS:
    case ENOMEM:          return SocketError::noBuffers;
    case EISCONN:         return SocketError::isConnected;
    case ENOTCONN:        return SocketError::notConnected;
    case ESHUTDOWN:       return SocketError::shutDown;
    case ETIMEDOUT:       return SocketError::timedOut;
    case ECONNREFUSED:    return SocketError::connectionRefused;
    case EHOSTDOWN:       return SocketError::hostDown;
    case EHOSTUNREACH:    return SocketError::hostUnreachable;
    default:              return SocketError::networkDown;
    }
}

SocketError nativeError()
{
    return errorOf(errno);
}

void closeNative(Native s)
{
    ::close(s);
}

bool makeNonBlocking(Native s)
{
    const int flags = ::fcntl(s, F_GETFL, 0);
    return flags >= 0 && ::fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
}

#endif

Native native(intptr_t handle)
{
    return Native(handle);
}

sockaddr_in inetAddress(uint32_t address, uint16_t port)
{
    sockaddr_in result;
    std::memset(&result, 0, sizeof(result));
    result.sin_family = AF_INET;
    result.sin_port = htons(port);
    result.sin_addr.s_addr = address;
    return result;
}

void readAddress(const sockaddr_in& from, uint32_t* address, uint16_t* port)
{
    if (address)
        *address = from.sin_addr.s_addr;
    if (port)
        *port = ntohs(from.sin_port);
}

int setInt(Native s, int level, int name, int value)
{
    return ::setsockopt(s, level, name, reinterpret_cast<const char*>(&value), sizeof(value));
}

#ifndef _WIN32
bool isPrivate(uint32_t address)
{
    const uint32_t host = ntohl(address);
    return (host >> 24) == 10 || (host >> 20) == 0xac1 || (host >> 16) == 0xc0a8;
}
#endif

/* The address a packet to the internet would leave from: a datagram socket
 * connected nowhere in particular sends nothing, only picks the route. */
uint32_t routedAddress()
{
    uint32_t result = htonl(INADDR_LOOPBACK);
    const Native s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == kNoSocket)
        return result;
    sockaddr_in to = inetAddress(htonl(0x08080808), 53);
    if (::connect(s, reinterpret_cast<const sockaddr*>(&to), sizeof(to)) == 0)
    {
        sockaddr_in local;
        AddressLength length = sizeof(local);
        if (::getsockname(s, reinterpret_cast<sockaddr*>(&local), &length) == 0 && local.sin_addr.s_addr != 0)
            result = local.sin_addr.s_addr;
    }
    closeNative(s);
    return result;
}

}

bool Socket::startup()
{
#ifdef _WIN32
    static const bool started = []() {
        WSADATA data;
        return ::WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return started;
#else
    return true;
#endif
}

SocketError Socket::lastError()
{
    return t_lastError;
}

void Socket::setLastError(SocketError error)
{
    t_lastError = error;
}

Socket::Socket(Kind kind, intptr_t handle)
    :   m_kind(kind)
    ,   m_handle(handle)
{
}

Socket::~Socket()
{
    closeNative(native(m_handle));
}

int Socket::fail()
{
    t_lastError = nativeError();
    return -1;
}

Socket* Socket::open(Kind kind)
{
    if (!startup())
    {
        t_lastError = SocketError::networkDown;
        return nullptr;
    }
    const Native s = kind == Stream ? ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
                                    : ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == kNoSocket)
    {
        t_lastError = nativeError();
        SDL_Log("[NET] socket(%s) failed: %d", kind == Stream ? "TCP" : "UDP", int(t_lastError));
        return nullptr;
    }
    if (!makeNonBlocking(s))
    {
        t_lastError = nativeError();
        closeNative(s);
        return nullptr;
    }
#ifndef _WIN32
    /* A host that ends a race and hosts again at once finds its port still held
     * by the last connections' TIME_WAIT; Windows lets it bind anyway. */
    if (kind == Stream)
        setInt(s, SOL_SOCKET, SO_REUSEADDR, 1);
#endif
    return new Socket(kind, intptr_t(s));
}

int Socket::bind(uint32_t address, uint16_t port)
{
    sockaddr_in at = inetAddress(address, port);
    if (::bind(native(m_handle), reinterpret_cast<const sockaddr*>(&at), sizeof(at)) != 0)
        return fail();
    return 0;
}

int Socket::listen(int backlog)
{
    if (::listen(native(m_handle), backlog) != 0)
        return fail();
    return 0;
}

Socket* Socket::accept(uint32_t* address, uint16_t* port)
{
    sockaddr_in from;
    AddressLength length = sizeof(from);
    const Native s = ::accept(native(m_handle), reinterpret_cast<sockaddr*>(&from), &length);
    if (s == kNoSocket)
    {
        fail();
        return nullptr;
    }
    makeNonBlocking(s);
    readAddress(from, address, port);
    Socket* result = new Socket(Stream, intptr_t(s));
    /* An accepted socket waits, or not, as the listening one does. */
    result->nonBlocking = nonBlocking;
    return result;
}

int Socket::connect(uint32_t address, uint16_t port)
{
    if (m_connecting)
    {
        t_lastError = SocketError::already;
        return -1;
    }
    sockaddr_in to = inetAddress(address, port);
    if (::connect(native(m_handle), reinterpret_cast<const sockaddr*>(&to), sizeof(to)) != 0)
    {
        fail();
        if (t_lastError != SocketError::wouldBlock)
            return -1;
    }
    m_connecting = true;
    m_connectError = SocketError::none;
    t_lastError = SocketError::wouldBlock;
    return -1;
}

int Socket::send(const void* data, int size)
{
    const int sent = int(::send(native(m_handle), static_cast<const char*>(data), size, kSendFlags));
    return sent < 0 ? fail() : sent;
}

int Socket::recv(void* data, int size, bool peek)
{
    const int got = int(::recv(native(m_handle), static_cast<char*>(data), size, peek ? MSG_PEEK : 0));
    return got < 0 ? fail() : got;
}

int Socket::sendTo(const void* data, int size, uint32_t address, uint16_t port)
{
    sockaddr_in to = inetAddress(address, port);
    const int sent = int(::sendto(native(m_handle), static_cast<const char*>(data), size, kSendFlags,
                                  reinterpret_cast<const sockaddr*>(&to), sizeof(to)));
    return sent < 0 ? fail() : sent;
}

int Socket::recvFrom(void* data, int size, uint32_t* address, uint16_t* port)
{
    sockaddr_in from;
    std::memset(&from, 0, sizeof(from));
    AddressLength length = sizeof(from);
    const int got = int(::recvfrom(native(m_handle), static_cast<char*>(data), size, 0,
                                   reinterpret_cast<sockaddr*>(&from), &length));
    if (got < 0)
        return fail();
    readAddress(from, address, port);
    return got;
}

int Socket::localName(uint32_t* address, uint16_t* port)
{
    sockaddr_in local;
    AddressLength length = sizeof(local);
    if (::getsockname(native(m_handle), reinterpret_cast<sockaddr*>(&local), &length) != 0)
        return fail();
    readAddress(local, address, port);
    return 0;
}

int Socket::shutdown()
{
    if (::shutdown(native(m_handle), kShutBoth) != 0)
        return fail();
    return 0;
}

int Socket::pending()
{
#ifdef _WIN32
    u_long waiting = 0;
    if (::ioctlsocket(native(m_handle), FIONREAD, &waiting) != 0)
        return fail();
#else
    int waiting = 0;
    if (::ioctl(native(m_handle), FIONREAD, &waiting) != 0)
        return fail();
#endif
    return int(waiting);
}

int Socket::setBroadcast(bool on)
{
    return setInt(native(m_handle), SOL_SOCKET, SO_BROADCAST, on) != 0 ? fail() : 0;
}

int Socket::setNoDelay(bool on)
{
    return setInt(native(m_handle), IPPROTO_TCP, TCP_NODELAY, on) != 0 ? fail() : 0;
}

int Socket::poll(Socket* const* sockets, const int* want, int* have, int count, int milliseconds)
{
#ifndef _WIN32
    std::vector<pollfd> fds(size_t(count > 0 ? count : 0));
    for (int i = 0; i < count; ++i)
    {
        const Socket* s = sockets[i];
        short events = 0;
        if (s->m_connecting)
        {
            if (want[i] & (Writable | Failed))
                events |= POLLOUT;
        }
        else
        {
            if (want[i] & Readable)
                events |= POLLIN;
            if (want[i] & Writable)
                events |= POLLOUT;
            if (want[i] & Failed)
                events |= POLLPRI;
        }
        fds[size_t(i)].fd = events ? native(s->m_handle) : -1;
        fds[size_t(i)].events = events;
        fds[size_t(i)].revents = 0;
    }
    const int result = ::poll(fds.data(), nfds_t(count), milliseconds);
    if (result < 0 && errno != EINTR)
    {
        t_lastError = nativeError();
        return -1;
    }
    int ready = 0;
    for (int i = 0; i < count; ++i)
    {
        Socket* s = sockets[i];
        const short events = result > 0 ? fds[size_t(i)].revents : 0;
        have[i] = 0;
        if (fds[size_t(i)].fd < 0)
            continue;
        if (s->m_connecting)
        {
            if (events & (POLLOUT | POLLERR | POLLHUP))
            {
                int error = 0;
                socklen_t length = sizeof(error);
                ::getsockopt(native(s->m_handle), SOL_SOCKET, SO_ERROR, &error, &length);
                s->m_connecting = false;
                s->m_connectError = error ? errorOf(error) : SocketError::none;
                have[i] = want[i] & (error ? Failed : Writable);
                SDL_Log("[NET] connection %s (%d)", error ? "refused or lost" : "made", int(s->m_connectError.load()));
            }
        }
        else
        {
            if (events & (POLLIN | POLLHUP | POLLERR))
                have[i] |= want[i] & Readable;
            if (events & POLLOUT)
                have[i] |= want[i] & Writable;
            if (events & POLLPRI)
                have[i] |= want[i] & Failed;
        }
        if (have[i])
            ++ready;
    }
    return ready;
#else
    fd_set readable, writable, failed;
    FD_ZERO(&readable);
    FD_ZERO(&writable);
    FD_ZERO(&failed);
    bool any = false;
    for (int i = 0; i < count; ++i)
    {
        const Socket* s = sockets[i];
        const Native n = native(s->m_handle);
        if (s->m_connecting)
        {
            if (want[i] & (Writable | Failed))
            {
                FD_SET(n, &writable);
                FD_SET(n, &failed);
                any = true;
            }
            continue;
        }
        if (want[i] & Readable) { FD_SET(n, &readable); any = true; }
        if (want[i] & Writable) { FD_SET(n, &writable); any = true; }
        if (want[i] & Failed)   { FD_SET(n, &failed);   any = true; }
    }
    for (int i = 0; i < count; ++i)
        have[i] = 0;
    if (!any)
    {
        /* Winsock's select refuses three empty sets. */
        if (milliseconds > 0)
            ::Sleep(DWORD(milliseconds));
        return 0;
    }
    timeval limit;
    limit.tv_sec = milliseconds / 1000;
    limit.tv_usec = (milliseconds % 1000) * 1000;
    if (::select(0, &readable, &writable, &failed, milliseconds < 0 ? nullptr : &limit) == SOCKET_ERROR)
    {
        t_lastError = nativeError();
        return -1;
    }
    int ready = 0;
    for (int i = 0; i < count; ++i)
    {
        Socket* s = sockets[i];
        const Native n = native(s->m_handle);
        if (s->m_connecting)
        {
            if (FD_ISSET(n, &writable))
            {
                s->m_connecting = false;
                have[i] = want[i] & Writable;
                SDL_Log("[NET] connection made");
            }
            else if (FD_ISSET(n, &failed))
            {
                int error = 0;
                int length = sizeof(error);
                ::getsockopt(n, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &length);
                s->m_connecting = false;
                s->m_connectError = error ? errorOf(error) : SocketError::networkDown;
                have[i] = want[i] & Failed;
                SDL_Log("[NET] connection refused or lost (%d)", int(s->m_connectError.load()));
            }
        }
        else
        {
            if (FD_ISSET(n, &readable)) have[i] |= want[i] & Readable;
            if (FD_ISSET(n, &writable)) have[i] |= want[i] & Writable;
            if (FD_ISSET(n, &failed))   have[i] |= want[i] & Failed;
        }
        if (have[i])
            ++ready;
    }
    return ready;
#endif
}

uint32_t Socket::localAddress()
{
#ifndef _WIN32
    /* The Wi-Fi's: of the interfaces up, the one a broadcast reaches others
     * from, on a private network, named as Wi-Fi, a hotspot or a cable is --
     * not the mobile network's, which reaches no one nearby. */
    uint32_t best = 0;
    int bestScore = -1;
    ifaddrs* list = nullptr;
    if (::getifaddrs(&list) == 0)
    {
        for (const ifaddrs* a = list; a; a = a->ifa_next)
        {
            if (!a->ifa_addr || a->ifa_addr->sa_family != AF_INET)
                continue;
            if (!(a->ifa_flags & IFF_UP) || (a->ifa_flags & IFF_LOOPBACK))
                continue;
            const uint32_t address = reinterpret_cast<const sockaddr_in*>(a->ifa_addr)->sin_addr.s_addr;
            const char* name = a->ifa_name ? a->ifa_name : "";
            int score = 0;
            if (a->ifa_flags & IFF_BROADCAST)
                score += 4;
            if (isPrivate(address))
                score += 2;
            if (!std::strncmp(name, "wlan", 4) || !std::strncmp(name, "swlan", 5) || !std::strncmp(name, "ap", 2)
                || !std::strncmp(name, "eth", 3) || !std::strncmp(name, "en", 2))
                score += 1;
            if (score > bestScore)
            {
                best = address;
                bestScore = score;
            }
        }
        ::freeifaddrs(list);
    }
    if (best)
        return best;
#endif
    return routedAddress();
}

std::vector<uint32_t> Socket::broadcastAddresses()
{
    std::vector<uint32_t> result;
#ifndef _WIN32
    ifaddrs* list = nullptr;
    if (::getifaddrs(&list) == 0)
    {
        for (const ifaddrs* a = list; a; a = a->ifa_next)
        {
            if (!a->ifa_addr || a->ifa_addr->sa_family != AF_INET || !a->ifa_broadaddr)
                continue;
            if (!(a->ifa_flags & IFF_UP) || !(a->ifa_flags & IFF_BROADCAST) || (a->ifa_flags & IFF_LOOPBACK))
                continue;
            const uint32_t address = reinterpret_cast<const sockaddr_in*>(a->ifa_broadaddr)->sin_addr.s_addr;
            bool known = false;
            for (uint32_t other : result)
                known = known || other == address;
            if (address != 0 && !known)
                result.push_back(address);
        }
        ::freeifaddrs(list);
    }
#endif
    if (result.empty())
        result.push_back(htonl(INADDR_BROADCAST));
    return result;
}

uint32_t Socket::parseAddress(const char* text)
{
    in_addr address;
    if (text && ::inet_pton(AF_INET, text, &address) == 1)
        return address.s_addr;
    return 0xffffffffu;
}

bool Socket::resolve(const char* name, uint32_t* address)
{
    if (!name || !*name || !startup())
        return false;
    in_addr dotted;
    if (::inet_pton(AF_INET, name, &dotted) == 1)
    {
        *address = dotted.s_addr;
        return true;
    }
    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    addrinfo* found = nullptr;
    if (::getaddrinfo(name, nullptr, &hints, &found) != 0 || !found)
        return false;
    *address = reinterpret_cast<const sockaddr_in*>(found->ai_addr)->sin_addr.s_addr;
    ::freeaddrinfo(found);
    return true;
}

}
