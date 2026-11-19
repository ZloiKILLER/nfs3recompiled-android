#include "os_net.h"

#include <lib/mutex.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <vector>

namespace nfs3hp
{
namespace os
{
namespace net
{

namespace
{

using nfs::Socket;

constexpr nfs::u16 kInetWord = 2;
constexpr nfs::u16 kIpxWord = 6;

Error failWith(Error error)
{
    Socket::setLastError(error);
    return error;
}

/* The socket `handle` holds, or nullptr (notSocket) when it holds none or it is closed. */
Socket* find(App* app, Handle handle)
{
    Socket* socket = dynamic_cast<Socket*>(app->getResource(handle));
    if (!socket || socket->closed())
    {
        Socket::setLastError(Error::notSocket);
        return nullptr;
    }
    return socket;
}

/* A call in progress on a socket: close frees it only once there is none. */
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
 * game's other threads run meanwhile. */
class Unlocked
{
public:
    Unlocked(App* app, nfs::ThreadState& ts)
        :   m_app(app)
        ,   m_ts(ts)
        ,   m_held(static_cast<nfs::Mutex*>(app->getMemory<nfs::Mutex*>(ts.threadBlock + 8)) != nullptr)
    {
        if (m_held)
            m_app->unlockContext(m_ts);
    }
    ~Unlocked()
    {
        if (m_held)
            m_app->lockContext(m_ts);
    }
    Unlocked(const Unlocked&) = delete;
    Unlocked& operator=(const Unlocked&) = delete;
private:
    App* m_app;
    nfs::ThreadState& m_ts;
    bool m_held;
};

/* Waits for any of `events` on a socket: the events it has, or 0 once the
 * socket is closed under the wait. */
int wait(App* app, nfs::ThreadState& ts, Socket* socket, int events)
{
    Unlocked unlocked(app, ts);
    for (;;)
    {
        if (socket->closed())
        {
            failWith(Error::interrupted);
            return 0;
        }
        int have = 0;
        if (Socket::poll(&socket, &events, &have, 1, 50) < 0)
            return 0;
        if (have)
            return have;
    }
}

/* A call on a socket the game left waiting: tried, and if it would block,
 * tried again each time the socket is ready, until it is done or fails. */
template <typename Call>
int blocking(App* app, nfs::ThreadState& ts, Socket* socket, int event, Call call)
{
    for (;;)
    {
        const int result = call();
        if (result >= 0 || socket->nonBlocking || Socket::lastError() != Error::wouldBlock)
            return result;
        if (!wait(app, ts, socket, event))
            return -1;
    }
}

/* A call's count as a Result: -1 and why, when it failed. */
Result resultOf(int value)
{
    return value < 0 ? Result::failed(Socket::lastError()) : Result{value, Error::none};
}

Error errorOf(int value)
{
    return value < 0 ? Socket::lastError() : Error::none;
}

/* The family a socket's addresses have, and fails for any other. */
bool fitting(Socket* socket, const Endpoint& endpoint)
{
    if (endpoint.family == (socket->ipx ? Family::ipx : Family::inet))
        return true;
    failWith(Error::familyNotSupported);
    return false;
}

Family familyOf(nfs::u16 word)
{
    return word == kInetWord ? Family::inet : word == kIpxWord ? Family::ipx : Family::other;
}

const char* dotted(std::uint32_t ip, char (&text)[16])
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

void traceData(Socket* socket, const char* what, Handle handle, const void* bytes, int asked, int result,
               std::uint32_t ip = 0, std::uint16_t port = 0)
{
    if (socket->traced >= tracedCalls() || (result < 0 && Socket::lastError() == Error::wouldBlock))
        return;
    ++socket->traced;
    char text[16];
    if (result < 0)
    {
        SDL_Log("[NET] %s 0x%x %d: error %d", what, unsigned(handle), asked, int(Socket::lastError()));
        return;
    }
    const std::uint8_t* data = static_cast<const std::uint8_t*>(bytes);
    char where[32] = "";
    if (ip)
        SDL_snprintf(where, sizeof(where), " %s:%u", dotted(ip, text), unsigned(port));
    char shownBytes[3 * 32 + 1] = "";
    const int shown = result < 32 ? result : 32;
    for (int i = 0; i < shown; ++i)
        SDL_snprintf(shownBytes + 3 * i, 4, "%02x ", unsigned(data[i]));
    SDL_Log("[NET] %s 0x%x %d: %d%s %s", what, unsigned(handle), asked, result, where, shownBytes);
}

}  // namespace

/* ---- Addresses ---- */

Endpoint inetAt(App* app, nfs::u32 at)
{
    const nfs::u8* data = &app->getMemory<nfs::u8>(at);
    Endpoint endpoint;
    endpoint.family = familyOf(app->getMemory<nfs::u16>(at));
    endpoint.port = std::uint16_t((data[2] << 8) | data[3]);
    std::memcpy(&endpoint.ip, data + 4, 4);
    return endpoint;
}

Endpoint ipxAt(App* app, nfs::u32 at)
{
    const nfs::u8* data = &app->getMemory<nfs::u8>(at);
    Endpoint endpoint;
    endpoint.family = familyOf(app->getMemory<nfs::u16>(at));
    endpoint.everyone = true;
    for (int i = 6; i < 12; ++i)
        endpoint.everyone = endpoint.everyone && data[i] == 0xff;
    std::memcpy(&endpoint.ip, data + 6, 4);
    endpoint.port = std::uint16_t((data[12] << 8) | data[13]);
    return endpoint;
}

void putInet(App* app, nfs::u32 at, const Endpoint& endpoint)
{
    nfs::u8* data = &app->getMemory<nfs::u8>(at);
    std::memset(data, 0, kInetBytes);
    app->getMemory<nfs::u16>(at) = kInetWord;
    data[2] = std::uint8_t(endpoint.port >> 8);
    data[3] = std::uint8_t(endpoint.port);
    std::memcpy(data + 4, &endpoint.ip, 4);
}

void putIpx(App* app, nfs::u32 at, const Endpoint& endpoint)
{
    nfs::u8* data = &app->getMemory<nfs::u8>(at);
    std::memset(data, 0, kIpxBytes);
    app->getMemory<nfs::u16>(at) = kIpxWord;
    std::memcpy(data + 6, &endpoint.ip, 4);
    data[12] = std::uint8_t(endpoint.port >> 8);
    data[13] = std::uint8_t(endpoint.port);
}

std::uint32_t localAddress()
{
    return Socket::localAddress();
}

std::optional<std::uint32_t> resolve(App* app, nfs::ThreadState& ts, const char* name)
{
    /* A dotted address, and this machine's name, without letting the other threads run. */
    const std::uint32_t dotted = Socket::parseAddress(name);
    if (dotted != 0xffffffffu)
        return dotted;
    if (name && SDL_strcasecmp(name, kHostName) == 0)
        return Socket::localAddress();
    std::uint32_t address = 0;
    bool found = false;
    {
        Unlocked unlocked(app, ts);
        found = Socket::resolve(name, &address);
    }
    if (!found)
    {
        Socket::setLastError(Error::hostNotFound);
        return std::nullopt;
    }
    return address;
}

std::string addressText(std::uint32_t ip)
{
    char text[16];
    return dotted(ip, text);
}

/* ---- The network ---- */

bool start()
{
    return Socket::startup();
}

void stop()
{
}

/* ---- Sockets ---- */

std::optional<Handle> open(App* app, Kind kind)
{
    Socket* socket = Socket::open(kind == Kind::tcp ? Socket::Stream : Socket::Datagram);
    if (!socket)
        return std::nullopt;
    if (kind == Kind::ipx)
    {
        socket->ipx = true;
        socket->setBroadcast(true);
    }
    return app->allocateResource(socket);
}

Error close(App* app, nfs::ThreadState& ts, Handle handle)
{
    Socket* socket = dynamic_cast<Socket*>(app->getResource(handle));
    if (!socket || socket->closed())
        return failWith(Error::notSocket);
    socket->close();
    SDL_Log("[NET] closing 0x%x%s", unsigned(handle), socket->users.load() > 0 ? ", a thread still waits on it" : "");
    if (socket->users.load() > 0)
    {
        /* A thread still waits on it -- the IPX receive thread, whose socket
         * the game closes to stop it: it wakes within a wait's slice. */
        Unlocked unlocked(app, ts);
        const Uint64 start = SDL_GetTicks();
        bool said = false;
        while (socket->users.load() > 0)
        {
            SDL_Delay(1);
            if (!said && SDL_GetTicks() - start > 2000)
            {
                SDL_Log("[NET] closing 0x%x: still waited on after two seconds", unsigned(handle));
                said = true;
            }
        }
    }
    app->freeResource(handle);
    return Error::none;
}

Error bind(App* app, Handle handle, const Endpoint& own)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    if (!fitting(socket, own))
        return Error::familyNotSupported;
    char text[16];
    if (socket->ipx)
    {
        const int result = socket->bind(0, own.port);
        std::uint16_t port = own.port;
        socket->localName(nullptr, &port);
        SDL_Log("[NET] IPX socket 0x%04x on UDP %u at %s: %s", unsigned(own.port), unsigned(port),
                dotted(Socket::localAddress(), text), result < 0 ? "failed" : "bound");
        return errorOf(result);
    }
    const int result = socket->bind(own.ip, own.port);
    if (result < 0)
        SDL_Log("[NET] bind %s:%u failed: %d", dotted(own.ip, text), unsigned(own.port), int(Socket::lastError()));
    return errorOf(result);
}

std::optional<Endpoint> localName(App* app, Handle handle)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return std::nullopt;
    std::uint32_t ip = 0;
    std::uint16_t port = 0;
    if (socket->localName(&ip, &port) < 0)
        return std::nullopt;
    if (socket->ipx)
        return Endpoint::ipx(Socket::localAddress(), port);
    return Endpoint::inet(ip, port);
}

Error listen(App* app, Handle handle, int backlog)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    std::uint16_t port = 0;
    socket->localName(nullptr, &port);
    const int result = socket->listen(backlog);
    char text[16];
    SDL_Log("[NET] hosting on TCP %u at %s: %s", unsigned(port), dotted(Socket::localAddress(), text),
            result < 0 ? "failed" : "listening");
    return errorOf(result);
}

std::optional<Accepted> accept(App* app, nfs::ThreadState& ts, Handle handle)
{
    Socket* listening = find(app, handle);
    if (!listening)
        return std::nullopt;
    Use use(listening);
    Socket* accepted = nullptr;
    std::uint32_t ip = 0;
    std::uint16_t port = 0;
    const int result = blocking(app, ts, listening, Socket::Readable, [&]() {
        accepted = listening->accept(&ip, &port);
        return accepted ? 0 : -1;
    });
    if (result < 0 || !accepted)
        return std::nullopt;
    char text[16];
    SDL_Log("[NET] %s:%u joined", dotted(ip, text), unsigned(port));
    return Accepted{app->allocateResource(accepted), Endpoint::inet(ip, port)};
}

Error connect(App* app, nfs::ThreadState& ts, Handle handle, const Endpoint& to)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    if (socket->ipx)
        return failWith(Error::operationNotSupported);
    Use use(socket);
    if (!fitting(socket, to))
        return Error::familyNotSupported;
    char text[16];
    SDL_Log("[NET] connecting to %s:%u", dotted(to.ip, text), unsigned(to.port));
    const int result = socket->connect(to.ip, to.port);
    if (result >= 0 || socket->nonBlocking || Socket::lastError() != Error::wouldBlock)
        return errorOf(result);
    /* A connection on a socket that waits: until it is made or refused. */
    const int outcome = wait(app, ts, socket, Socket::Writable | Socket::Failed);
    if (!outcome)
        return Socket::lastError();
    if (outcome & Socket::Failed)
        return failWith(socket->connectError());
    return Error::none;
}

Error shutdown(App* app, Handle handle)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    return errorOf(socket->shutdown());
}

Result send(App* app, nfs::ThreadState& ts, Handle handle, const void* data, nfs::u32 size)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Result::failed(Error::notSocket);
    Use use(socket);
    const int sent = blocking(app, ts, socket, Socket::Writable, [&]() {
        return socket->send(data, int(size));
    });
    traceData(socket, "send", handle, data, int(size), sent);
    return resultOf(sent);
}

Result receive(App* app, nfs::ThreadState& ts, Handle handle, void* data, nfs::u32 size, bool peek)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Result::failed(Error::notSocket);
    Use use(socket);
    const int got = blocking(app, ts, socket, Socket::Readable, [&]() {
        return socket->recv(data, int(size), peek);
    });
    traceData(socket, peek ? "peek" : "recv", handle, data, int(size), got);
    return resultOf(got);
}

Result sendTo(App* app, nfs::ThreadState& ts, Handle handle, const void* data, nfs::u32 size, const Endpoint& to)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Result::failed(Error::notSocket);
    Use use(socket);
    if (!fitting(socket, to))
        return Result::failed(Error::familyNotSupported);
    if (socket->ipx && to.everyone)
    {
        /* Every machine of every network this one is on. */
        int sent = -1;
        for (std::uint32_t ip : Socket::broadcastAddresses())
        {
            sent = socket->sendTo(data, int(size), ip, to.port);
            traceData(socket, "broadcast", handle, data, int(size), sent, ip, to.port);
        }
        return resultOf(sent);
    }
    const int sent = blocking(app, ts, socket, Socket::Writable, [&]() {
        return socket->sendTo(data, int(size), to.ip, to.port);
    });
    traceData(socket, "sendto", handle, data, int(size), sent, to.ip, to.port);
    return resultOf(sent);
}

Result receiveFrom(App* app, nfs::ThreadState& ts, Handle handle, void* data, nfs::u32 size, Endpoint& from)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Result::failed(Error::notSocket);
    Use use(socket);
    std::uint32_t ip = 0;
    std::uint16_t port = 0;
    const int got = blocking(app, ts, socket, Socket::Readable, [&]() {
        return socket->recvFrom(data, int(size), &ip, &port);
    });
    traceData(socket, "recvfrom", handle, data, int(size), got, ip, port);
    if (got >= 0)
        from = socket->ipx ? Endpoint::ipx(ip, port) : Endpoint::inet(ip, port);
    return resultOf(got);
}

Error setWaits(App* app, Handle handle, bool waits)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    socket->nonBlocking = !waits;
    return Error::none;
}

Result waiting(App* app, Handle handle)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Result::failed(Error::notSocket);
    return resultOf(socket->pending());
}

Error setBroadcast(App* app, Handle handle, bool on)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    return errorOf(socket->setBroadcast(on));
}

Error setNoDelay(App* app, Handle handle, bool on)
{
    Socket* socket = find(app, handle);
    if (!socket)
        return Error::notSocket;
    return errorOf(socket->setNoDelay(on));
}

bool contains(const SocketSet& set, Handle socket)
{
    const unsigned count = set.count < kSetSize ? set.count : kSetSize;
    for (unsigned i = 0; i < count; ++i)
    {
        if (set.sockets[i] == socket)
            return true;
    }
    return false;
}

Result select(App* app, nfs::ThreadState& ts, SocketSet* reading, SocketSet* writing, SocketSet* failing,
              std::optional<nfs::u32> milliseconds)
{
    std::vector<Socket*> sockets;
    std::vector<Handle> handles;
    std::vector<int> want;
    auto collect = [&](const SocketSet* set, int event) {
        if (!set)
            return true;
        const unsigned listed = set->count < kSetSize ? set->count : kSetSize;
        for (unsigned i = 0; i < listed; ++i)
        {
            const Handle handle = set->sockets[i];
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
    if (!collect(reading, Socket::Readable) || !collect(writing, Socket::Writable)
        || !collect(failing, Socket::Failed))
        return Result::failed(Error::notSocket);

    const int listed = int(sockets.size());
    std::vector<int> have(sockets.size(), 0);
    /* Into the log each time a thread asks select about a different set of
     * sockets than it last did: what it waits for on each, R, W or E. */
    {
        thread_local std::string t_lastAsked;
        std::string asked;
        for (int i = 0; i < listed; ++i)
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
    if (!milliseconds || *milliseconds > 0)
    {
        /* Waited out with the guest lock let go, a little at a time, so a
         * socket closed meanwhile is let go of soon. */
        const Uint64 start = SDL_GetTicks();
        const Uint64 limit = milliseconds ? Uint64(*milliseconds) : ~Uint64(0);
        Unlocked unlocked(app, ts);
        for (;;)
        {
            const Uint64 spent = SDL_GetTicks() - start;
            const Uint64 left = spent < limit ? limit - spent : 0;
            ready = Socket::poll(sockets.data(), want.data(), have.data(), listed, int(left < 20 ? left : 20));
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
        ready = Socket::poll(sockets.data(), want.data(), have.data(), listed, 0);
    }
    for (Socket* socket : sockets)
        --socket->users;
    if (ready < 0)
        return Result::failed(Socket::lastError());
    if (ready > 0)
    {
        static unsigned s_logged = 0;
        if (++s_logged <= 40)
        {
            std::string got;
            for (int i = 0; i < listed; ++i)
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

    int total = 0;
    auto keep = [&](SocketSet* set, int event) {
        if (!set)
            return;
        const unsigned inSet = set->count < kSetSize ? set->count : kSetSize;
        unsigned kept = 0;
        for (unsigned i = 0; i < inSet; ++i)
        {
            const Handle handle = set->sockets[i];
            for (size_t at = 0; at < handles.size(); ++at)
            {
                if (handles[at] == handle && (have[at] & event))
                {
                    set->sockets[kept++] = handle;
                    break;
                }
            }
        }
        set->count = kept;
        total += int(kept);
    };
    keep(reading, Socket::Readable);
    keep(writing, Socket::Writable);
    keep(failing, Socket::Failed);
    return Result{total, Error::none};
}

}  // namespace net

/* ---- The serial port ---- */

namespace serial
{

std::optional<LineSettings> settings(App* app, Handle port)
{
    NFS2_USE(app);
    NFS2_USE(port);
    NFS2_ASSERT(false);
    return std::nullopt;
}

bool setSettings(App* app, Handle port, const LineSettings& line)
{
    NFS2_USE(app);
    NFS2_USE(port);
    NFS2_USE(line);
    NFS2_ASSERT(false);
    return false;
}

bool setReadTimeout(App* app, Handle port, nfs::u32 milliseconds)
{
    NFS2_USE(app);
    NFS2_USE(port);
    NFS2_USE(milliseconds);
    NFS2_ASSERT(false);
    return false;
}

bool drop(App* app, Handle port, bool input, bool output)
{
    NFS2_USE(app);
    NFS2_USE(port);
    NFS2_USE(input);
    NFS2_USE(output);
    NFS2_ASSERT(false);
    return false;
}

std::optional<nfs::u32> finished(App* app, Handle port, bool input)
{
    NFS2_USE(app);
    NFS2_USE(port);
    NFS2_USE(input);
    NFS2_ASSERT(false);
    return std::nullopt;
}

}  // namespace serial
}  // namespace os
}  // namespace nfs3hp
