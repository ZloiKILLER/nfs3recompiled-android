#ifndef NFS3HP_HOST_NET_H
#define NFS3HP_HOST_NET_H

#include <x86.h>

#include <cstdint>

namespace win32
{
class WinApplication;
}

/* The game's network on the host (plan step 6): Winsock 1.1 as the game uses it, on the host's
 * sockets (lib/socket.h), and the serial port, which the host has none of.
 *
 * Each function takes the words the game's code pushes for the import it stands for, in its order,
 * typed (an address a pointer into the game's memory, 0 a null one), so the call sites name it in
 * the import's place: guest::thunked<kSocket, &host::socketOpen>(app, cpu, af, type, protocol).
 * What each gives back is what the import gave the game: Winsock's results and error codes (kept
 * per thread, as WSAGetLastError reads them), the socket numbers the game keeps (the application's
 * resource handles), the structures it reads, laid out as Winsock lays them.  One that can wait on
 * the network takes the CPU and lets the guest context go meanwhile. */
namespace nfs3hp
{
namespace host
{

/* Winsock's structures as the game has them in its memory. */
struct NetTimeval
{
    std::int32_t seconds;
    std::int32_t microseconds;
};

constexpr unsigned kNetSetSize = 64;

struct NetSet
{
    std::uint32_t count;
    std::uint32_t sockets[kNetSetSize];
};

/* sockaddr: the family and 14 bytes, sockaddr_in's or sockaddr_ipx's. */
struct NetAddress
{
    std::uint16_t family;
    std::uint8_t data[14];
};

struct NetStartup
{
    std::uint16_t version;
    std::uint16_t highVersion;
    std::uint16_t maxSockets;
    std::uint16_t maxUdpDatagram;
    std::uint32_t vendorInfo;
    char description[257];
    char systemStatus[129];
};

/* WSAStartup, WSACleanup, WSAGetLastError. */
int netStartup(std::uint16_t version, NetStartup* data);
int netCleanup();
int netLastError();

/* socket: the socket's number, or 0xffffffff. */
x86::reg32 socketOpen(win32::WinApplication* app, int family, int type, int protocol);
/* closesocket: waits for the calls still on the socket (the IPX receive thread's) to end. */
int socketClose(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket);
int socketBind(win32::WinApplication* app, x86::reg32 socket, const NetAddress* name, int length);
int socketListen(win32::WinApplication* app, x86::reg32 socket, int backlog);
x86::reg32 socketAccept(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, NetAddress* address,
                        int* length);
int socketConnect(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, const NetAddress* name, int length);
int socketShutdown(win32::WinApplication* app, x86::reg32 socket, int how);
int socketSend(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, const char* data, int size, int flags);
int socketRecv(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, char* data, int size, int flags);
int socketSendTo(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, const char* data, int size, int flags,
                 const NetAddress* to, int length);
int socketRecvFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 socket, char* data, int size, int flags,
                   NetAddress* from, int* length);
/* getsockname. */
int socketName(win32::WinApplication* app, x86::reg32 socket, NetAddress* name, int* length);
/* setsockopt. */
int socketSetOption(win32::WinApplication* app, x86::reg32 socket, int level, int name, const char* value, int size);
/* ioctlsocket: FIONBIO, FIONREAD, SIOCATMARK. */
int socketControl(win32::WinApplication* app, x86::reg32 socket, x86::reg32 command, x86::reg32* argument);
/* select: each set left with the sockets that have its event, and their count. */
int socketSelect(win32::WinApplication* app, x86::CPU& cpu, int count, NetSet* reading, NetSet* writing,
                 NetSet* failing, const NetTimeval* timeout);
/* __WSAFDIsSet. */
int socketIsSet(x86::reg32 socket, NetSet* set);

/* htons and ntohs alike. */
unsigned short byteSwap16(unsigned short value);
/* inet_addr: a dotted address, or 0xffffffff. */
std::uint32_t addressParse(const char* text);
/* inet_ntoa: the address's text in the game's memory, its address. */
x86::reg32 addressText(win32::WinApplication* app, x86::reg32 address);
/* gethostname. */
int hostName(char* name, int length);
/* gethostbyname: a hostent in the game's memory, its address, or 0. */
x86::reg32 hostByName(win32::WinApplication* app, x86::CPU& cpu, const char* name);

/* The serial port (GetCommState, SetCommState, SetCommTimeouts, GetCommProperties, PurgeComm,
 * GetOverlappedResult on a port): the host has none, so each fails (FALSE) and says so in the log
 * once per kind.  The game gets to them only on a port it opened, which it never does here. */
int serialGetState(x86::reg32 handle, x86::reg32 dcb);
int serialSetState(x86::reg32 handle, x86::reg32 dcb);
int serialSetTimeouts(x86::reg32 handle, x86::reg32 timeouts);
int serialGetProperties(x86::reg32 handle, x86::reg32 properties);
int serialPurge(x86::reg32 handle, x86::reg32 flags);
int serialOverlappedResult(x86::reg32 handle, x86::reg32 overlapped, x86::reg32 transferred, x86::reg32 wait);

}  // namespace host
}  // namespace nfs3hp

#endif
