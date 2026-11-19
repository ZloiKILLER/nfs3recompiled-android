#include "native_frontend_netb.h"
#include "native_ea_files.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_frontend_eab.h"
#include "native_frontend_neta.h"
#include "native_frontend_netc.h"
#include "native_frontend_wata.h"
#include "native_frontend_watc.h"
#include "os_time.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include "os_system.h"
#include "os_net.h"
#include "os_window.h"
#include "native_frontend_wina.h"
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 commEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCountBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sendGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findPlayerByAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 addNewPlayer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sessionMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qinit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qinitpeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qpush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qbottom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qpop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qpoplast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qinsert(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qlowpriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qfind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qfindpriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qiterate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 qsafeiterate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 layerReceiveLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 layerCallUnpacker(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 layerReceiveFromTop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connInitFilter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 layerReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpFlushQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxOpenWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpOpenWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpListenSocket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSetSocketOptions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpFillSelectSets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpReceivePacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSendPending(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpConnectionDropped(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpServerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpOpenConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSendConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpCloseConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSameAddressParts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpHostGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpEndGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netReturnZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpStartWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpStopWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiSelectModem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiEnumerateModems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiOpenLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiGetCommHandleConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiSetCommConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiGetDevCaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiMakeCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiGetModemHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiHangUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemPoll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemSendReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemReceiveReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 modemName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialPoll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialSendReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialReceiveReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 serialOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxBroadcastOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxScanInterfaces(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxAdvertiseTo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxIsRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxServerSocket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxServerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxStartServer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxStopServer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 gameGuidText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setResponseTimeout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 responseTimedOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netReturnZeroToo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sendServerRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpExceptionSeen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax: the registers left as the call by address
 * leaves them -- `words` in eax, edx, ebx and ecx as that call loads them, the first `Taken` of
 * them the core's arguments -- and its result in eax, which the call by address leaves there. */
/* One register's value, as the game's code holds it at a point of a function: a plain value, and whether
 * the code has put one there. */
struct Slot
{
    nfs::u32 value = 0;
    bool set = false;
    Slot& operator=(nfs::u32 v)
    {
        value = v;
        set = true;
        return *this;
    }
    operator nfs::u32() const { return value; }
    Slot& operator|=(nfs::u32 v) { return *this = value | v; }
    Slot& operator&=(nfs::u32 v) { return *this = value & v; }
    Slot& operator+=(nfs::u32 v) { return *this = value + v; }
    Slot& operator-=(nfs::u32 v) { return *this = value - v; }
    Slot& operator++() { return *this = value + 1; }
    Slot& operator--() { return *this = value - 1; }
};

/* What the game's code holds in eax, edx, ebx and ecx: the values its callees are handed beyond their
 * arguments, and its results. */
struct Held
{
    Slot eax, edx, ebx, ecx;
};

/* Guest::call with what the code holds in the registers its arguments leave out (those it has put
 * something in) going with it; what the callee leaves in the registers is held after, eax the result. */
inline nfs::u32 callHeld(fe::Guest& guest, Held& h, nfs::u32 function, fe::Arguments arguments = fe::Arguments())
{
    Slot* const held[4] = { &h.eax, &h.edx, &h.ebx, &h.ecx };
    std::size_t count = arguments.registerCount;
    for (std::size_t i = count; i < 4; ++i)
        if (held[i]->set)
            count = i + 1;
    for (std::size_t i = arguments.registerCount; i < count; ++i)
        arguments.registers[i] = held[i]->value;
    for (std::size_t i = 0; i < count; ++i)
        *held[i] = arguments.registers[i];
    arguments.registerCount = count;
    nfs::Args left;
    const nfs::u32 result = fe::pointerCall(guest, function, arguments, left);
    // What the callee left in them: the code goes on with it, as with the callee's own value in eax.
    h.eax = result;
    h.edx = left.edx;
    h.ebx = left.ebx;
    h.ecx = left.ecx;
    return result;
}

/* A typed core that can stop with the game: its result, or Terminated as Guest::call throws it. */
inline nfs::u32 got(std::optional<nfs::u32> result)
{
    if (!result)
        throw fe::Terminated();
    return *result;
}

inline void completed(bool finished)
{
    if (!finished)
        throw fe::Terminated();
}

template <nfs::u32 Address, auto Core, std::size_t... I>
auto directOf(nfs::App* app, nfs::ThreadState& ts, const nfs::u32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, ts, words[I]...);
}

template <nfs::u32 Address, auto Core, std::size_t Taken, typename... Words>
nfs::u32 directKept(nfs::App* app, nfs::ThreadState& ts, Held& h, Words... words)
{
    const nfs::u32 all[] = { nfs::u32(words)..., 0 };
    Slot* const into[] = { &h.eax, &h.edx, &h.ebx, &h.ecx };
    for (std::size_t i = 0; i < sizeof...(Words) && i < 4; ++i)
        *into[i] = all[i];
    const auto result = directOf<Address, Core>(app, ts, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, nfs::u32>)
        h.eax = result;
    else
        h.eax = *result;
    return h.eax;
}

}  // namespace
}  // namespace nfs3hp


/* EA's network library, the parts of it left:
 *
 *   TCP          win\tcp.c: the listening thread and a connection's sends and receives
 *                (sub_4f6490 to sub_4f7420);
 *   IPX          win\ipx.c: Winsock opened, the advertising thread (sub_4f63d0, sub_4f7830,
 *                sub_5127d0 to sub_512d70);
 *   TAPI         win\tapipkt.c: the modem's line and the thread that dials (sub_4f9a50 to sub_4fa780);
 *   the queues   cmn\queue.c: 0x1c bytes each, +0 how many, +4 flags (1 changed, 2 its lock
 *                its own), +8 first, +0xc last, +0x10 and +0x14 a priority's function and
 *                argument, +0x18 the lock (sub_50dcd0 to sub_50e9b0);
 *   LAYER        cmn\layer.c: a packet handed up a connection's layers (sub_512470 to sub_5126f0);
 *   sessions     the messages of a session's players (sub_512f60 to sub_5133a0).
 *
 * All their state stays in guest memory.  What they call of the game is called by address;
 * Windows' functions are the port's own, called as C++; the network and the serial port are the
 * port's (os_net.h).  What each function
 * leaves in eax and in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::netb (native_frontend_netb.h), in a frame of
 * its own where the game's function has one (fe::Frame), and a thunk under the game's name for
 * the calls by address.  A core calls the cores of this file and of native_frontend_neta.cpp as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game still
 * by address, and so does a call whose callee leaves in a register what the caller goes on with or
 * leaves at its own exit. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdi;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::Site;
using nfs::u32;
using nfs::s32;

constexpr u32 kTraceLevel = 0x566de0;   // how much sub_512180 is asked to log
constexpr u32 kError = 0x401010;
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr u32 kMemoryGroup = 0x5643f4;  // what the library's allocations are tagged with
constexpr u32 kNetLock = 0x564480;      // the sockets' critical section
constexpr u32 kNow = 0x79c1a4;
constexpr u32 kTicksPerSecond = 0x5643d8;
constexpr u32 kReserve = 0x4e1620;
constexpr u32 kMemmove = 0x4ea4f0, kStrncpy = 0x4e0e30, kStrncmp = 0x4eb390, kMemset = 0x4e0640;
constexpr u32 kCreateThread = 0x4df7a0, kWaitSystemTasks = 0x4e7720;
constexpr u32 kCheckPlayer = 0x4f5040;  // NET_: whether a player's address is this one's

u32 swapped(u32 v)
{
    return (v << 24) | ((v & 0xff00) << 8) | ((v >> 8) & 0xff00) | (v >> 24);
}

void copyDwords(Guest& g, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        g.setDword(to + 4 * i, g.dword(from + 4 * i));
}

/* A number of `bytes` bytes in network order at `at`, read as the library's macro reads it:
 * a whole dword swapped and shifted down, eax the value and ecx the count of the shift. */
u32 bigEndian(Guest& g, nfs::ThreadState& ts, u32 at, u32 bytes)
{
    Held h;
    const u32 shift = 0x20 - bytes * 8;
    h.eax = swapped(g.dword(at)) >> (shift & 31);
    return h.eax;
}

/* A line of the log: its text, the address it is about, the data and how many bytes of it; eax as
 * the log's function leaves it, for the callers that end with what their calls left there. */
u32 logEvent(Guest& g, Held& h, nfs::ThreadState& ts, u32 text, u32 what, u32 data, u32 bytes, u32 stackWord = 0)
{
    return h.eax = guest::direct<0x512180, frontend::netb::commEvent>(g.app(), ts, text, what, data, bytes, stackWord,
                                                                     stackWord);
}

/* The same line by address, where the call after it takes the registers the log's function leaves
 * (a function through a pointer handed what it left in ebx and ecx). */
void logEventLeaving(Guest& g, Held& h, u32 text, u32 what, u32 data, u32 bytes, u32 stackWord = 0)
{
    callHeld(g, h, 0x512180, args({ text, what, data, bytes }, { stackWord, stackWord }));
}

/* The source position the library leaves for sub_401010 before a report. */
void setErrorAt(Guest& g, u32 file, u32 function, u32 line)
{
    g.setDword(kErrorFile, file);
    g.setDword(kErrorFunction, function);
    g.setDword(kErrorLine, line);
}

/* The session messages' four-letter types, sub_5133a0. */
constexpr u32 kMsgAdd = 0xea616464, kMsgBad = 0xea626164, kMsgBye = 0xea627965, kMsgDat = 0xea646174;
constexpr u32 kMsgDec = 0xea646563, kMsgGet = 0xea676574, kMsgNo = 0xea6e6f21, kMsgOla = 0xea6f6c61;
constexpr u32 kMsgOpn = 0xea6f706e, kMsgOut = 0xea6f7574, kMsgPly = 0xea706c79, kMsgReq = 0xea726571;
constexpr u32 kMsgRsp = 0xea727370, kMsgSrv = 0xea737276;

constexpr u32 kPulseTime = 0x566dd4, kAliveTime = 0x566dd8;
constexpr u32 kOwnPlayer = 0x1c;        // in a NET object: this player's address
constexpr u32 kNetSend = 0x54;          // in a NET object's functions: send to an address
constexpr u32 kPlayerRecord = 0x64;     // a player in a game: 100 bytes
constexpr u32 kPlayers = 0x88;          // in a game
constexpr u32 kPlayerMask = 0x50;       // in a game: a bit for each player present

constexpr u32 kSendGameInfo = 0x513030, kFindPlayer = 0x5132a0;

}  // namespace

/* The port's own line in logcat for what sub_512180 logs (native_vertices.cpp). */
bool commEventTrace(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers);

namespace
{

/* The port's line for one of the log's (commEventTrace) reads the call's registers and its two stack
 * words: they are put where the game's call puts them, whichever way sub_512180's core was called. */
void traceEvent(nfs::App* app, nfs::ThreadState& ts, u32 text, u32 what, u32 data, u32 bytes,
                u32 flagged, u32 kind)
{
    const EspKept kept(ts);
    ts.sp -= 12;
    app->getMemory<u32>(ts.sp + 4) = flagged;
    app->getMemory<u32>(ts.sp + 8) = kind;
    nfs::Args registers;
    registers.eax = text, registers.edx = what, registers.ebx = data, registers.ecx = bytes;
    commEventTrace(app, ts, registers);
}

}  // namespace

/* sub_512180: a line in the network's log, a ring of 64-byte records at [0x5689d0]: the time,
 * the first dword of an address, how many bytes of data, the second stack word, flags (1 a
 * text, 2 an address, 4 data, 8 the first stack word set) and data and text after them, the two
 * no more than 0x35 bytes.  Printed too when [0x5689d4] says so. */
u32 frontend::netb::commEvent(App* app, ThreadState& ts, u32 text, u32 what, u32 data, u32 bytes, u32 flagged, u32 kindWord)
{
    Held h;
    h.eax = text; h.edx = what; h.ebx = data; h.ecx = bytes;
    traceEvent(app, ts, text, what, data, bytes, flagged, kindWord);
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1c);
    constexpr u32 kLog = 0x5689d0, kLogLock = 0x5689cc, kPrinted = 0x5689d4, kThreadId = 0x5689d8;
    constexpr u32 kSize = 0xa0b090, kHead = 0xa0b094, kUsed = 0xa0b098;
    const std::uint8_t kind = std::uint8_t(kindWord);
    if (g.dword(kLog) != 0)
    {
        fe::enterLock(g, ts, g.dword(kLogLock));
        const u32 used = g.dword(kUsed), size = g.dword(kSize);
        const u32 slot = u32(s32(g.dword(kHead) + used) % s32(size));
        const u32 record = g.dword(kLog) + (slot << 6);
        if (used != size)
            g.setDword(kUsed, used + 1);
        else
            g.setDword(kHead, u32(s32(g.dword(kHead) + 1) % s32(size)));

        std::uint8_t textLength = 0;
        if (text != 0)
        {
            const u32 length = g.length(text);
            textLength = std::uint8_t(length >= 0x35 ? 0x35 : length);
        }
        std::uint8_t dataLength = 0;
        if (data != 0)
        {
            const s32 room = s32(0x35 - u32(textLength));
            dataLength = std::uint8_t(room > s32(bytes) ? bytes : u32(room));
        }
        g.setByte(record + 8, dataLength);
        std::uint8_t flags = flagged != 0 ? 8 : 0;
        g.setByte(record + 0xa, flags);
        g.setDword(record, g.dword(kNow));
        g.setByte(record + 9, kind);
        u32 to = record + 0xb;
        if (data != 0 && dataLength != 0)
        {
            flags |= 4;
            g.setByte(record + 0xa, flags);
            for (std::uint8_t i = 0; i < dataLength; ++i)
                g.setByte(to++, g.byte(data + i));
        }
        if (text != 0 && textLength != 0)
        {
            flags |= 1;
            g.setByte(record + 0xa, flags);
            for (std::uint8_t i = 0; i < textLength; ++i)
                g.setByte(to++, g.byte(text + i));
            g.setByte(to, 0);
        }
        if (what != 0)
        {
            g.setDword(record + 4, g.dword(what));
            flags |= 2;
            g.setByte(record + 0xa, flags);
            h.ebx = (what & 0xffff00ff) | (u32(flags) << 8);
        }
        else
        {
            g.setDword(record + 4, 0);
            h.ebx = 0;
        }
        if (g.dword(kPrinted) != 0)
        {
            h.ebx = flagged;
            callHeld(g, h, 0x4e08f0, args({ g.byte(kThreadId) },
                                  { 0x55016c, g.dword(record), g.byte(kThreadId), kind,
                                    flagged != 0 ? u32(0x550108) : u32(0x550110),
                                    what != 0 ? g.dword(what) : 0, text != 0 ? text : u32(0x550168), bytes }));
        }
        fe::leaveLock(g, ts, g.dword(kLogLock));
    }
    return g.dword(kLog) != 0 ? 1 : 0;
}

nfs::u32 frontend::commEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512180", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 text = args.eax;
    const u32 what = args.edx;
    const u32 data = args.ebx;
    const u32 bytes = args.ecx;
    const u32 flagged = app->getMemory<u32>(ts.sp + 4);
    const u32 kindWord = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::commEvent(app, ts, text, what, data, bytes, flagged, kindWord); });
}

/* sub_513000: how many bits a word has set. */
u32 frontend::netb::countBits(App* app, ThreadState& ts, u32 value)
{
    u32 count = 0;
    while (value != 0)
    {
        value &= value - 1;
        ++count;
    }
    return count;
}

nfs::u32 frontend::netCountBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_513000", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const u32 value = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::countBits(app, ts, value); });
}

/* sub_513030: this game's description sent to an address: eax the game, edx the address, ebx
 * the id of the request.  With no request, a game that has started says nothing. */
u32 frontend::netb::sendGameInfo(App* app, ThreadState& ts, u32 game, u32 to, u32 id)
{
    Held h;
    h.eax = game; h.edx = to; h.ebx = id;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc4);
    const u32 net = g.dword(game + 4);
    if (id == 0 && g.dword(game + 8) != 0)
        return id;
    const u32 frame = g.frame();
    const u32 reply = frame + 8, typeAt = frame + 0xb4;
    constexpr u32 kReplySize = 0xac;
    const auto shortHigh = [&](u32 at) {
        return std::uint16_t(swapped(u32(s32(g.dword(at)) >> 16)) >> 16);
    };
    g.setDword(typeAt, kMsgDec);
    g.setDword(reply, swapped(id));
    for (u32 i = 0; i < 4; ++i)
        g.setByte(frame + 0x50 + i, g.byte(game + 0x74 + i));
    g.setWord(frame + 0x54, shortHigh(game + 0x76));
    g.setWord(frame + 0x56, shortHigh(game + 0x78));
    g.setDword(frame + 0x58, swapped(g.dword(game + 0x7c)));
    g.setDword(frame + 0x5c, swapped(g.dword(game + 0x80)));
    g.setDword(frame + 0x60, swapped(g.dword(game + 0x84)));
    g.setDword(frame + 0x74, swapped(g.dword(game + 0x50)));
    copyDwords(g, frame + 0xc, net + kOwnPlayer, 5);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), frame + 0x30, game + 0x54, 0x20);
    for (u32 i = 0; i < 8; ++i)
        g.setByte(frame + 0x28 + i, g.byte(net + 0x94 + i));
    g.setDword(frame + 0x20, swapped(g.dword(net + 0x8c)));
    g.setWord(frame + 0x24, std::uint16_t(swapped(g.word(net + 0x90)) >> 16));
    g.setWord(frame + 0x26, std::uint16_t(swapped(g.word(net + 0x92)) >> 16));
    if (s32(g.dword(kTraceLevel)) >= 3)
        logEvent(g, h, ts, 0x550264, typeAt, reply, kReplySize);
    return callHeld(g, h, g.dword(g.dword(net + 8) + kNetSend), args({ g.dword(net + 0xc), to, kMsgDec, reply }, { kReplySize }));
}

nfs::u32 frontend::sendGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_513030", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 game = args.eax, to = args.edx, id = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::sendGameInfo(app, ts, game, to, id); });
}

/* sub_5132a0: the player of a game at an address, his time last heard from set; 0 for none. */
u32 frontend::netb::findPlayerByAddress(App* app, ThreadState& ts, u32 game, u32 address)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    u32 player = game + kPlayers;
    for (u32 i = 0;; ++i, player += kPlayerRecord)
    {
        if (g.dword(game + 0x84) - 1 <= i)
            return 0;
        if ((g.dword(game + kPlayerMask) & (u32(1) << (i & 31))) != 0
            && got(fe::direct<0x4f5040, ea::netPlayerChecked>(app, ts, g.dword(game + 4), address, player)) != 0)
        {
            g.setDword(game + i * kPlayerRecord + 0xe4, g.dword(kPulseTime));
            return player;
        }
    }
}

nfs::u32 frontend::findPlayerByAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5132a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 game = args.eax, address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::findPlayerByAddress(app, ts, game, address); });
}

/* sub_513320: a player's record taken into a game's first empty place; his number, or 0 and a
 * printed complaint when there is none (the host has counted before this). */
u32 frontend::netb::addNewPlayer(App* app, ThreadState& ts, u32 game, u32 record)
{
    Held h;
    h.eax = game; h.edx = record;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    for (u32 i = 0;; ++i)
    {
        h.edx = g.dword(game + 0x84) - 1;
        if (i >= h.edx)
        {
            callHeld(g, h, 0x4e08f0, args({ i }, { 0x55027c }));
            h.edx = 0;
            return 0;
        }
        const u32 bit = u32(1) << (i & 31);
        const u32 mask = g.dword(game + kPlayerMask);
        if ((mask & bit) != 0)
            continue;
        g.setDword(game + kPlayerMask, mask | bit);
        const u32 place = game + i * kPlayerRecord;
        copyDwords(g, place + kPlayers, record, 0x17);
        g.setDword(place + 0xe4, g.dword(kPulseTime));
        g.setDword(place + 0xe8, i);
        h.edx = i;
        return i;
    }
}

nfs::u32 frontend::addNewPlayer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_513320", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 game = args.eax, record = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::addNewPlayer(app, ts, game, record); });
}

/* sub_5133a0: a session message from an address.  eax the session waiting for answers, edx
 * the address, ebx the type, ecx the message, the stack its length.  [+0x10] is the NET
 * object, whose +0x14 is the game this machine hosts: a host answers GET, ADD, REQ and SRV, a
 * client hears RSP, PLY, BAD and DEC; OPN, OLA, DAT, OUT and BYE go with either. */
u32 frontend::netb::sessionMessage(App* app, ThreadState& ts, u32 session, u32 from, u32 type, u32 message, u32 length)
{
    Held h;
    h.eax = session; h.edx = from; h.ebx = type; h.ecx = message;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xdc);
    const u32 frame = g.frame();
    const u32 net = g.dword(session + 0x10);
    const u32 typeAt = frame + 0xac, replyType = frame + 0xb0;
    g.setDword(typeAt, type);
    g.setDword(frame + 0xcc, from);
    g.setDword(frame + 0xb8, message);

    // The reply is built at the frame's bottom: +0 the request's id, +4 an address,
    // +0x18 its verdict, +0x1c a player.
    const auto send = [&](u32 size) {
        const u32 functions = g.dword(net + 8);
        return callHeld(g, h, g.dword(functions + kNetSend), args({ g.dword(net + 0xc), from, g.dword(replyType), frame }, { size }));
    };
    const auto trace = [&](u32 level) { return s32(g.dword(kTraceLevel)) >= s32(level); };
    const auto logMessage = [&](u32 text) { logEvent(g, h, ts, text, typeAt, message, length); };
    const auto checkOwn = [&] { return callHeld(g, h, kCheckPlayer, args({ net, from, net + kOwnPlayer })); };

    switch (type)
    {
    case kMsgNo:
        if (trace(3))
            logEvent(g, h, ts, 0x550380, 0, from, 0x14);
        break;

    case kMsgOla:
    {
        const u32 game = g.dword(net + 0x14);
        h.ecx = game;
        if (game != 0)
        {
            const u32 player = callHeld(g, h, kFindPlayer, args({ game }));
            if (player != 0 && trace(4))
                logEvent(g, h, ts, 0x550388, player + 0x60, 0, 0);
            break;
        }
        if (g.dword(net + 0xa4) == 0 || g.dword(net + 0xa0) == 0)
            break;
        if (checkOwn() == 0)
            break;
        g.setDword(replyType, kMsgOla);
        g.setDword(net + 0xa4, g.dword(kPulseTime));
        callHeld(g, h, g.dword(g.dword(net + 8) + kNetSend),
               args({ g.dword(net + 0xc), net + kOwnPlayer, kMsgOla, h.ecx }, { h.ecx }));
        if (trace(4))
            logEvent(g, h, ts, 0x550398, typeAt, 0, 0);
        break;
    }

    case kMsgReq:
        if (net == 0 || g.dword(net + 0x14) == 0)
            break;
        {
            const u32 wanted = bigEndian(g, ts, message, 4);
            const u32 game = g.dword(net + 0x14);
            callHeld(g, h, kSendGameInfo, args({ game, from, wanted, game }));
        }
        break;

    case kMsgSrv:
    {
        if (net == 0)
            break;
        const u32 game = g.dword(net + 0x14);
        if (game == 0)
            break;
        const u32 differs = fe::direct<kStrncmp, watcom::clib::strncmp>(g.app(), g.ts(), game + 0xc, message, 0x20);
        h.eax = differs;  // what the function gives back when it leaves here: strncmp's answer, as the code has it
        if (differs != 0)
            break;
        callHeld(g, h, kSendGameInfo, args({ g.dword(net + 0x14), from, 0 }));
        break;
    }

    case kMsgOpn:
        if (trace(3))
            logEvent(g, h, ts, 0x550378, 0, from, 0x14);
        if (g.dword(net + 0x10) == 0)
            break;
        {
            const u32 refused = callHeld(g, h, g.dword(net + 0x10), args({ net, from }));
            if (refused != 0)
                break;
            callHeld(g, h, g.dword(g.dword(net + 8) + kNetSend), args({ g.dword(net + 0xc), from, kMsgNo, 0 }, { refused }));
        }
        break;

    case kMsgOut:
    case kMsgBye:
    {
        if (trace(2))
            logMessage(0x5503a8);
        const u32 game = g.dword(net + 0x14);
        h.ecx = game;
        if (game == 0)
        {
            g.setDword(net + 0xa4, 0);
            g.setDword(net + 0xa0, 0);
            break;
        }
        const u32 player = callHeld(g, h, kFindPlayer, args({ game, from }));
        if (player == 0)
            break;
        const u32 hosting = g.dword(net + 0x14);
        const s32 offset = s32(player - (hosting + kPlayers));
        const s32 index = offset / s32(kPlayerRecord);
        h.edx = u32(offset % s32(kPlayerRecord));
        h.ecx = u32(index) & 0xff;
        h.eax = ~(u32(1) << (u32(index) & 31));
        h.ebx = g.dword(hosting + kPlayerMask) & h.eax;
        g.setDword(hosting + kPlayerMask, h.ebx);
        g.setDword(player + 0x5c, 0);
        break;
    }

    case kMsgDat:
    {
        const u32 game = g.dword(net + 0x14);
        if (game != 0)
        {
            callHeld(g, h, kFindPlayer, args({ game }));
            break;
        }
        if (checkOwn() == 0 || g.dword(net + 0xa0) == 0)
            break;
        h.eax = g.dword(kAliveTime);
        g.setDword(net + 0xa0, h.eax);
        break;
    }

    case kMsgGet:
    {
        if (g.dword(net + 0x14) == 0)
            break;
        // Answered with the player asked for, or the host himself for -1.
        g.setDword(replyType, kMsgPly);
        copyDwords(g, frame + 4, message + 4, 5);
        u32 size = 0x78;
        g.setDword(frame, g.dword(message));
        g.setDword(frame + 0x18, g.dword(message + 0x18));
        const u32 which = bigEndian(g, ts, message + 0x18, 4);
        if (which == 0xffffffff)
        {
            copyDwords(g, frame + 0x1c, net + 0x30, 0x17);
        }
        else if ((g.dword(g.dword(net + 0x14) + kPlayerMask) & (u32(1) << (which & 31))) == 0)
        {
            size -= 0x5c;
            g.setDword(replyType, kMsgBad);
        }
        else
        {
            copyDwords(g, frame + 0x1c, g.dword(net + 0x14) + which * kPlayerRecord + kPlayers, 0x17);
        }
        if (trace(3))
            logEvent(g, h, ts, 0x5502dc, replyType, frame, size);
        send(size);
        break;
    }

    case kMsgAdd:
    {
        const u32 game = g.dword(net + 0x14);
        h.edx = game;
        if (game == 0)
            break;
        h.ecx = g.dword(game + 8);
        if (h.ecx != 0)
            break;
        const u32 known = callHeld(g, h, kFindPlayer, args({ game, from, kMsgRsp }));
        g.setDword(replyType, kMsgRsp);
        g.setDword(frame, g.dword(message));

        if (known != 0)
        {
            g.setDword(frame + 0x18, swapped(1));
            const s32 index = s32(known - (g.dword(net + 0x14) + kPlayers)) / s32(kPlayerRecord);
            g.setDword(frame + 0x1c, swapped(u32(index)));
            if (trace(4))
                logEvent(g, h, ts, 0x55033c, replyType, from, 0x14);
        }
        else
        {
            u32 verdict = 0, text = 0;
            const u32 hosting = g.dword(net + 0x14);
            const u32 most = g.dword(hosting + 0x84) - 1;
            h.edx = most;
            if (guest::direct<0x513000, countBits>(app, ts, g.dword(hosting + kPlayerMask)) >= most)
            {
                verdict = 0xfffffffe;
                text = 0x550330;
            }
            else
            {
                const u32 password = g.dword(net + 0x14) + 0x3c;
                if (g.byte(password) != 0 && fe::direct<kStrncmp, watcom::clib::strncmp>(g.app(), g.ts(), password, message + 0x18, 0x10) != 0)
                {
                    verdict = 0xffffffff;
                    text = 0x550324;
                }
                else
                {
                    // A new player: his record is what he sent, with the address he came from.
                    const u32 record = message + 0x28;
                    const u32 slot = guest::direct<0x513320, addNewPlayer>(app, ts, g.dword(net + 0x14), record);
                    copyDwords(g, record, from, 5);
                    const u32 now = g.dword(net + 0x14);
                    if (g.dword(now + 0x4c) != 0
                        && callHeld(g, h, g.dword(now + 0x4c), args({ net, record, slot + 1, 0 })) == 0)
                    {
                        const u32 hostingNow = g.dword(net + 0x14);
                        g.setDword(hostingNow + kPlayerMask, g.dword(hostingNow + kPlayerMask) & ~(u32(1) << (slot & 31)));
                        verdict = 0xfffffffd;
                        text = 0x550318;
                    }
                    else
                    {
                        g.setDword(frame + 0x18, swapped(1));
                        g.setDword(frame + 0x1c, swapped(slot));
                        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, frame + 4, message + 4, 0x14);
                        if (trace(3))
                            logEvent(g, h, ts, 0x55030c, replyType, from, 0x14);
                    }
                }
            }
            if (text != 0)
            {
                g.setDword(frame + 0x18, swapped(verdict));
                if (trace(3))
                    logEvent(g, h, ts, text, replyType, from, 0x14, text == 0x550318 ? 0 : h.ecx);
            }
        }
        const u32 refused = send(0x20);
        if (refused == 0 && trace(3))
            logEvent(g, h, ts, 0x550348, replyType, from, 0x14);
        break;
    }

    case kMsgPly:
    case kMsgBad:
    {
        if (checkOwn() == 0)
        {
            if (trace(3))
                logMessage(0x550300);
            break;
        }
        if (g.dword(net + 0xa0) != 0)
        {
            h.eax = g.dword(kAliveTime);
            g.setDword(net + 0xa0, h.eax);
        }
        const u32 id = bigEndian(g, ts, message, 4);
        if (g.dword(session + 0x18) == 0 || id != g.dword(session))
        {
            if (trace(3))
                logMessage(0x5502f4);
            break;
        }
        if (trace(3))
            logMessage(0x5502e8);
        if (g.dword(typeAt) == kMsgPly)
        {
            const u32 last = bigEndian(g, ts, message + 0x18, 4) == 0xffffffff ? 1 : 0;
            const u32 number = bigEndian(g, ts, message + 0x18, 4);
            const u32 keep = callHeld(g, h, g.dword(session + 0x18),
                                      args({ g.dword(session + 0x10), message + 0x1c, number + 1, last }));
            if (keep == 0)
                g.setDword(session + 0x18, 0);
        }
        g.setDword(session, 0);
        g.setDword(session + 4, 1);
        break;
    }

    case kMsgRsp:
    {
        if (checkOwn() == 0)
        {
            if (trace(3))
                logMessage(0x55036c);
            break;
        }
        if (bigEndian(g, ts, message, 4) != g.dword(session))
        {
            if (trace(3))
                logMessage(0x550360);
            break;
        }
        if (trace(3))
            logMessage(0x550354);
        const u32 verdict = bigEndian(g, ts, message + 0x18, 4);
        g.setDword(session + 4, verdict);
        g.setDword(session, 0);
        if (verdict != 1)
            break;
        copyDwords(g, net + kOwnPlayer, message + 4, 5);
        g.setDword(net + 0xa4, g.dword(kPulseTime));
        h.eax = g.dword(kAliveTime);
        g.setDword(net + 0xa0, h.eax);
        break;
    }

    case kMsgDec:
    {
        // The host's description: wanted by a session looking for games (id 0), or the
        // one asked by id, from the host this machine is with.
        const u32 id = bigEndian(g, ts, message, 4);
        h.edx = id;
        bool wanted = id == 0 && g.dword(session) == 0 && g.dword(session + 0x14) != 0;
        if (!wanted && g.dword(session + 0x14) == 0)
        {
            h.ecx = g.dword(session);
            if (h.ecx != 0 && id == h.ecx && checkOwn() != 0)
                wanted = true;
        }
        h.eax = wanted ? 1 : 0;
        h.ebx = h.eax;
        if (!wanted)
            break;
        if (g.dword(net + 0xa0) != 0 && g.dword(session + 0x14) == 0)
        {
            h.eax = g.dword(kAliveTime);
            g.setDword(net + 0xa0, h.eax);
        }
        g.setByte(session + 0x3c, std::uint8_t(bigEndian(g, ts, message + 0x48, 1)));
        g.setByte(session + 0x3d, std::uint8_t(bigEndian(g, ts, message + 0x49, 1)));
        g.setByte(session + 0x3e, std::uint8_t(bigEndian(g, ts, message + 0x4a, 1)));
        g.setByte(session + 0x3f, std::uint8_t(bigEndian(g, ts, message + 0x4b, 1)));
        g.setWord(session + 0x40, std::uint16_t(bigEndian(g, ts, message + 0x4c, 2)));
        g.setWord(session + 0x42, std::uint16_t(bigEndian(g, ts, message + 0x4e, 2)));
        g.setDword(session + 0x44, bigEndian(g, ts, message + 0x50, 4));
        g.setDword(session + 0x48, bigEndian(g, ts, message + 0x54, 4));
        g.setDword(session + 0x4c, bigEndian(g, ts, message + 0x58, 4));
        // By address: the ebx and edx it leaves may be left at the end.
        callHeld(g, h, kStrncpy, args({ session + 0x1c, message + 0x28, 0x20 }));
        const u32 mask = bigEndian(g, ts, message + 0x6c, 4);
        g.setDword(session + 4, 1);
        h.ecx = g.dword(kTraceLevel);
        g.setDword(session + 8, mask);
        if (trace(3))
            logMessage(0x5502d0);
        if (g.dword(session + 0x14) == 0)
            break;
        h.ecx = from;
        const u32 players = guest::direct<0x513000, countBits>(app, ts, g.dword(message + 0x6c));
        const u32 keep = callHeld(g, h, g.dword(session + 0x14), args({ net, session + 0x1c, players + 1, from }));
        if (keep == 0)
            g.setDword(session + 0x14, 0);
        break;
    }

    default:
        if (trace(3))
            logMessage(0x5503b0);
        break;
    }
    return h.eax;
}

nfs::u32 frontend::sessionMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5133a0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 session = args.eax;
    const u32 from = args.edx;
    const u32 type = args.ebx;
    const u32 message = args.ecx;
    const u32 length = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::sessionMessage(app, ts, session, from, type, message, length); });
}

/* ---------------------------------------------------------------------------
 * cmn\queue.c: the queues
 * ------------------------------------------------------------------------- */
namespace
{

constexpr u32 kQueueFile = 0x54fbb8, kQueueUninitialized = 0x54fbcc;  // "cmn\queue.c", "%s - UNINITIALIZED QUEUE %08x USED.\n"
constexpr u32 kDefaultPriority = 0x50dbf0;
constexpr u32 kQCount = 0, kQFlags = 4, kQFirst = 8, kQLast = 0xc, kQPriority = 0x10, kQPriorityArg = 0x14, kQLock = 0x18;

/* A queue with no lock is reported, and the code goes on with it. */
void reportQueue(Guest& g, Held& h, u32 function, u32 line, u32 queue)
{
    setErrorAt(g, kQueueFile, function, line);
    // What the report leaves in eax, with the eax the code holds at the call.
    h.eax = *fe::direct<kError, ea::errorLeaves>(g.app(), g.ts(), h.eax.value, kQueueUninitialized,
                                                 guest::Words{function, queue});
}

void setChanged(Guest& g, u32 queue)
{
    g.setByte(queue + kQFlags, std::uint8_t(g.byte(queue + kQFlags) | 1));
}

u32 queuePriority(Guest& g, u32 queue, u32 element)
{
    Held h;
    return callHeld(g, h, g.dword(queue + kQPriority), args({ element, g.dword(queue + kQPriorityArg) }));
}

}  // namespace

/* sub_50dcd0: qinit: a queue with a lock of its own, eax it, edx its priority function or 0 for
 * the default, ebx the function's argument. */
u32 frontend::netb::qinit(App* app, ThreadState& ts, u32 queue, u32 priority, u32 argument)
{
    Held h;
    h.edx = priority;
    h.eax = queue;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    const u32 lock = u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    g.setDword(queue + kQCount, 0);
    g.setDword(queue + kQFlags, 2);
    g.setDword(queue + kQFirst, 0);
    g.setDword(queue + kQLast, 0);
    g.setDword(queue + kQLock, lock);
    g.setDword(queue + kQPriority, priority != 0 ? priority : kDefaultPriority);
    h.edx = g.dword(queue + kQLock);
    g.setDword(queue + kQPriorityArg, argument);
    u32 left = lock;  // eax: the lock, or what the report leaves
    if (h.edx == 0)
    {
        reportQueue(g, h, 0x54fbc4, 0x8b, queue);
        left = h.eax;
    }
    return left;
}

nfs::u32 frontend::qinit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50dcd0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, priority = args.edx, argument = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qinit(app, ts, queue, priority, argument); });
}

/* sub_50dd60: qinitpeer: a queue that shares the lock of the queue in ecx. */
u32 frontend::netb::qinitpeer(App* app, ThreadState& ts, u32 queue, u32 priority, u32 argument, u32 peer)
{
    Held h;
    h.eax = queue; h.edx = priority;h.ecx = peer;
    Guest g(app, ts);
    h.ecx = g.dword(peer + kQLock);
    g.setDword(queue + kQCount, 0);
    g.setDword(queue + kQFlags, 0);
    g.setDword(queue + kQFirst, 0);
    g.setDword(queue + kQLast, 0);
    g.setDword(queue + kQLock, h.ecx);
    g.setDword(queue + kQPriority, priority != 0 ? priority : kDefaultPriority);
    h.edx = g.dword(queue + kQLock);
    g.setDword(queue + kQPriorityArg, argument);
    u32 left = queue;  // eax: the queue, or what the report leaves
    if (h.edx == 0)
    {
        reportQueue(g, h, 0x54fbf4, 0xc7, queue);
        left = h.eax;
        h.ecx = kQueueFile;
    }
    return left;
}

nfs::u32 frontend::qinitpeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50dd60", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, priority = args.edx, argument = args.ebx, peer = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qinitpeer(app, ts, queue, priority, argument, peer); });
}

/* sub_50de50: qpush: an element at the head. */
u32 frontend::netb::qpush(App* app, ThreadState& ts, u32 queue, u32 element)
{
    Held h;
    h.eax = queue;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc08, 0x128, queue);
    if (element != 0)
    {
        g.setDword(element, g.dword(queue + kQFirst));
        g.setDword(queue + kQFirst, element);
        g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
        if (g.dword(element) == 0)
            g.setDword(queue + kQLast, element);
        setChanged(g, queue);
    }
    h.eax = fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return h.eax;
}

nfs::u32 frontend::qpush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50de50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, element = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qpush(app, ts, queue, element); });
}

/* sub_50ded0: qbottom: an element at the tail. */
u32 frontend::netb::qbottom(App* app, ThreadState& ts, u32 queue, u32 element)
{
    Held h;
    h.eax = queue;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc10, 0x13c, queue);
    if (element != 0)
    {
        const u32 last = g.dword(queue + kQLast);
        g.setDword(element, 0);
        g.setDword(queue + kQLast, element);
        g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
        if (last != 0)
            g.setDword(last, element);
        else
            g.setDword(queue + kQFirst, element);
        setChanged(g, queue);
    }
    h.eax = fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return h.eax;
}

nfs::u32 frontend::qbottom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50ded0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, element = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qbottom(app, ts, queue, element); });
}

/* sub_50df60: qpop: the element at the head, taken off; 0 for none. */
u32 frontend::netb::qpop(App* app, ThreadState& ts, u32 queue)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc18, 0x17b, queue);
    const u32 first = g.dword(queue + kQFirst);
    if (first != 0)
    {
        if (first == g.dword(queue + kQLast))
        {
            g.setDword(queue + kQLast, 0);
            g.setDword(queue + kQFirst, 0);
        }
        else
        {
            g.setDword(queue + kQFirst, g.dword(first));
        }
        g.setDword(queue + kQCount, g.dword(queue + kQCount) - 1);
        g.setDword(first, 0);
    }
    setChanged(g, queue);
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return first;
}

nfs::u32 frontend::qpop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50df60", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qpop(app, ts, queue); });
}

/* sub_50dff0: qpoplast: the element at the tail, taken off; 0 for none. */
u32 frontend::netb::qpoplast(App* app, ThreadState& ts, u32 queue)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc20, 0x190, queue);
    const u32 last = g.dword(queue + kQLast);
    if (last != 0)
    {
        const u32 first = g.dword(queue + kQFirst);
        g.setDword(queue + kQCount, g.dword(queue + kQCount) - 1);
        if (last == first)
        {
            g.setDword(queue + kQFirst, 0);
            g.setDword(queue + kQLast, 0);
        }
        else
        {
            // The tail walks up from the head to the element before the last.
            g.setDword(queue + kQLast, first);
            for (;;)
            {
                const u32 at = g.dword(queue + kQLast);
                const u32 next = g.dword(at);
                if (next == last)
                {
                    g.setDword(at, 0);
                    break;
                }
                g.setDword(queue + kQLast, next);
            }
        }
        g.setDword(last, 0);
    }
    setChanged(g, queue);
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return last;
}

nfs::u32 frontend::qpoplast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50dff0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qpoplast(app, ts, queue); });
}

/* sub_50e0a0: qinsert: an element in the order of the queue's priority function. */
u32 frontend::netb::qinsert(App* app, ThreadState& ts, u32 queue, u32 element)
{
    Held h;
    h.eax = queue;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (element == 0)
    {
        h.eax = fe::leaveLock(g, ts, g.dword(queue + kQLock));
        return h.eax;
    }
    const u32 priority = queuePriority(g, queue, element);
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc2c, 0x1d8, queue);
    g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
    u32 at = g.dword(queue + kQFirst), before = 0;
    while (at != 0 && queuePriority(g, queue, at) < priority)
    {
        before = at;
        at = g.dword(at);
    }
    g.setDword(element, at);
    if (before != 0)
        g.setDword(before, element);
    else
        g.setDword(queue + kQFirst, element);
    if (at == 0)
        g.setDword(queue + kQLast, element);
    setChanged(g, queue);
    h.eax = fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return h.eax;
}

nfs::u32 frontend::qinsert(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e0a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, element = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qinsert(app, ts, queue, element); });
}

/* sub_50e2d0: qlowpriority: the priority of the element at the head, -1 for none. */
u32 frontend::netb::qlowpriority(App* app, ThreadState& ts, u32 queue)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc50, 0x2b9, queue);
    u32 priority = 0xffffffff;
    if (g.dword(queue + kQFirst) != 0)
        priority = queuePriority(g, queue, g.dword(queue + kQFirst));
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return priority;
}

nfs::u32 frontend::qlowpriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e2d0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qlowpriority(app, ts, queue); });
}

/* sub_50e3f0: qfind: the first element a function says yes to, edx it, ebx its argument. */
u32 frontend::netb::qfind(App* app, ThreadState& ts, u32 queue, u32 match, u32 argument)
{
    Held h;
    h.eax = queue; h.edx = match; h.ebx = argument;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc70, 0x303, queue);
    u32 at = g.dword(queue + kQFirst);
    while (at != 0 && match != 0 && callHeld(g, h, match, args({ at, argument, at })) == 0)
        at = g.dword(at);
    h.ebx = at;
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return at;
}

nfs::u32 frontend::qfind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e3f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, match = args.edx, argument = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qfind(app, ts, queue, match, argument); });
}

/* sub_50e500: qfindpriority: the element of exactly a priority, edx it; 0 for none. */
u32 frontend::netb::qfindpriority(App* app, ThreadState& ts, u32 queue, u32 priority)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fc84, 0x321, queue);
    u32 at = g.dword(queue + kQFirst);
    while (at != 0 && queuePriority(g, queue, at) < priority)
        at = g.dword(at);
    if (at != 0 && queuePriority(g, queue, at) != priority)
        at = 0;
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return at;
}

nfs::u32 frontend::qfindpriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e500", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, priority = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qfindpriority(app, ts, queue, priority); });
}

/* sub_50e8e0: qiterate: a function on each element until the queue changes, outside the lock;
 * -1 when it came to the end with no change, else how many it was called for.  A function that
 * answers 0 counts as a change: the walk stops there. */
u32 frontend::netb::qiterate(App* app, ThreadState& ts, u32 queue, u32 function, u32 argument)
{
    Held h;
    h.eax = queue; h.edx = function; h.ebx = argument;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    const u32 wasChanged = g.dword(queue + kQFlags) & 1;
    g.setByte(queue + kQFlags, std::uint8_t(g.byte(queue + kQFlags) & 0xfe));
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fda4, 0x3f6, queue);
    u32 called = 0;
    for (u32 at = g.dword(queue + kQFirst); at != 0 && (g.byte(queue + kQFlags) & 1) == 0; at = g.dword(at))
    {
        if (callHeld(g, h, function, args({ at, argument })) == 0)
            setChanged(g, queue);
        ++called;
    }
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if ((g.byte(queue + kQFlags) & 1) == 0)
        called = 0xffffffff;
    g.setDword(queue + kQFlags, g.dword(queue + kQFlags) | wasChanged);
    h.ebx = g.dword(queue + kQLock);
    fe::leaveLock(g, ts, h.ebx);
    return called;
}

nfs::u32 frontend::qiterate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e8e0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, function = args.edx, argument = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qiterate(app, ts, queue, function, argument); });
}

/* sub_50e9b0: qsafeiterate: qiterate with the lock held all along. */
u32 frontend::netb::qsafeiterate(App* app, ThreadState& ts, u32 queue, u32 function, u32 argument)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    fe::enterLock(g, ts, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, h, 0x54fdb0, 0x414, queue);
    const u32 called = guest::direct<0x50e8e0, qiterate>(app, ts, queue, function, argument);
    fe::leaveLock(g, ts, g.dword(queue + kQLock));
    return called;
}

nfs::u32 frontend::qsafeiterate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_50e9b0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 queue = args.eax, function = args.edx, argument = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::qsafeiterate(app, ts, queue, function, argument); });
}

/* ---------------------------------------------------------------------------
 * cmn\layer.c: a packet handed up a connection's layers
 * ------------------------------------------------------------------------- */

/* sub_512470: under the last layer: the pieces -- edx of them at ebx, {length, address} --
 * gathered into one block for the connection's own function, eax the connection. */
u32 frontend::netb::layerReceiveLast(App* app, ThreadState& ts, u32 connection, u32 pieceCount, u32 pieces)
{
    Held h;
    h.eax = connection; h.edx = pieceCount; h.ebx = pieces;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1c);
    const s32 count = s32(pieceCount);
    h.ecx = pieces;
    u32 total = 0;
    for (s32 i = 0; i < count; ++i)
        total += g.dword(pieces + 8 * u32(i));
    setErrorAt(g, 0x550194, 0x5501b4, 0x4a);
    const u32 block = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x5501c4, total, 0));
    u32 to = block;
    for (s32 i = 0; i < count; ++i)
    {
        const u32 piece = pieces + 8 * u32(i);
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to, g.dword(piece + 4), g.dword(piece));
        to += g.dword(piece);
    }
    const u32 result = callHeld(g, h, g.dword(connection + 0x20), args({ connection, block, total, connection }));
    h.ecx = result;
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
    return result;
}

nfs::u32 frontend::layerReceiveLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512470", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 connection = args.eax, pieceCount = args.edx, pieces = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::layerReceiveLast(app, ts, connection, pieceCount, pieces); });
}

/* sub_5125b0: to the layer of an index in the queue of layers on the stack, or past the last. */
u32 frontend::netb::layerCallUnpacker(App* app, ThreadState& ts, u32 first, u32 index, u32 second, u32 pieces, u32 layers)
{
    Held h;
    h.eax = first; h.edx = index; h.ebx = second; h.ecx = pieces;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    h.ebx = layers;
    if (s32(index) >= 0 && index < directKept<0x50e1d0, ea::queueLengthOf, 1>(g.app(), g.ts(), h, layers))
    {
        g.setDword(layers + 0x1c, index);
        const u32 layer = callHeld(g, h, 0x50e470, args({ layers }));
        if (layer == 0)
        {
            setErrorAt(g, 0x550194, 0x5501f4, 0x78);
            fe::direct<0x401010, ea::error>(app, ts, 0x5501e0u, guest::Words({}));
        }
        return callHeld(g, h, g.dword(layer + 8), args({ first, g.dword(layer + 0xc), second, pieces }, { layers }));
    }
    // The pieces in ecx too, which it does not read.
    return guest::direct<0x512470, layerReceiveLast>(app, ts, first, second, pieces);
}

nfs::u32 frontend::layerCallUnpacker(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5125b0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 first = args.eax;
    const u32 index = args.edx;
    const u32 second = args.ebx;
    const u32 pieces = args.ecx;
    const u32 layers = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::layerCallUnpacker(app, ts, first, index, second, pieces, layers); });
}

/* sub_512650: from the top of a connection's layers: ecx them, edx the pieces' count, ebx them.
 * The caller's ecx is the argument sub_5125b0 pops. */
u32 frontend::netb::layerReceiveFromTop(App* app, ThreadState& ts, u32 first, u32 count, u32 pieces, u32 layers)
{
    Guest g(app, ts);
    return guest::direct<0x5125b0, layerCallUnpacker>(app, ts, first, 0, count, pieces, layers);
}

nfs::u32 frontend::layerReceiveFromTop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512650", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 first = args.eax, count = args.edx, pieces = args.ebx, layers = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::layerReceiveFromTop(app, ts, first, count, pieces, layers); });
}

/* sub_512660: CONN_initfilter: a connection's queue of layers. */
u32 frontend::netb::connInitFilter(App* app, ThreadState& ts, u32 connection)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    setErrorAt(g, 0x550194, 0x550208, 0x9a);
    const u32 queue = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x5501c4, 0x20, g.dword(kMemoryGroup)));
    g.setDword(connection + 0x30, queue);
    guest::direct<0x50dcd0, qinit>(app, ts, queue, 0, 0);
    return g.dword(connection + 0x30) != 0 ? 1 : 0;
}

nfs::u32 frontend::connInitFilter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512660", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::connInitFilter(app, ts, connection); });
}

/* sub_5126f0: a packet received on a connection, edx it, ebx how long: one piece for the layers. */
u32 frontend::netb::layerReceive(App* app, ThreadState& ts, u32 connection, u32 packet, u32 length)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    const u32 piece = g.frame();
    g.setDword(piece + 4, packet);
    g.setDword(piece, length);
    return guest::direct<0x512650, layerReceiveFromTop>(app, ts, connection, 1, piece, g.dword(connection + 0x30));
}

nfs::u32 frontend::layerReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5126f0", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 connection = args.eax, packet = args.edx, length = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::layerReceive(app, ts, connection, packet, length); });
}

/* ---------------------------------------------------------------------------
 * win\tcp.c: connections over TCP, and Winsock opened for IPX and TCP
 * ------------------------------------------------------------------------- */
namespace
{

using os::net::Error;

// Whether `socket` is in the select set at `set` in the game's memory.
bool isSet(Guest& g, u32 socket, u32 set)
{
    return os::net::contains(g.app()->getMemory<os::net::SocketSet>(set), socket);
}

// A select set in the game's memory, as the port takes it.
os::net::SocketSet* setAt(Guest& g, u32 set)
{
    return &g.app()->getMemory<os::net::SocketSet>(set);
}

// Why a network call failed, as the game's log shows it.
u32 errorNumber(Error error)
{
    return u32(error);
}

// What a send says when the socket cannot take it now, and should be tried again.
bool sendLater(Error error)
{
    return error == Error::wouldBlock || error == Error::noBuffers;
}

constexpr u32 kQRemove = 0x50e150;
constexpr u32 kOpenQueue = 0x9f2628, kFreeQueue = 0x9f260c;  // connections up, connections done with
constexpr u32 kMaxPacket = 0x9f37e8;   // word; the largest datagram the network told of, 0 for none (the port's never tells)
constexpr u32 kListener = 0x9f37f0;    // what the TCP thread listens with
constexpr u32 kNetCount = 0x566de4, kNetNames = 0x9f25d0, kNetTables = 0x9f25b0;

// A TCP connection's fields.
constexpr u32 kConnFunctions = 4, kConnBuffer = 0x10, kConnReady = 0x14, kConnClosing = 0x18, kConnUp = 0x24;
constexpr u32 kConnAddress = 0x34, kConnSocket = 0x48, kConnNet = 0x4c;

/* The errors with which a connection gives up for good, of those a socket that has not yet
 * connected may say; any other leaves it to try on. */
bool connectFailed(Error error)
{
    switch (error)
    {
    case Error::fault: case Error::invalid:
    case Error::familyNotSupported: case Error::addressInUse: case Error::addressNotAvailable:
    case Error::networkDown: case Error::networkUnreachable:
    case Error::isConnected: case Error::connectionRefused: case Error::hostDown: case Error::hostUnreachable:
        return true;
    default:
        return false;
    }
}

/* What sub_4f63d0 and sub_4f7830 share: the network's name and table added to the list. */
void addNetKind(Guest& g, u32 name, u32 table)
{
    const u32 count = g.dword(kNetCount);
    g.setDword(kNetNames + count * 4, name);
    g.setDword(kNetTables + count * 4, table);
    g.setDword(kNetCount, count + 1);
}

}  // namespace

/* sub_4f6350: what is queued for an address flushed: edx a packet whose first 20 bytes are the
 * address, or 0, ebx the rest for sub_4f5ac0 by way of the frame. */
u32 frontend::netb::tcpFlushQueue(App* app, ThreadState& ts, u32 owner, u32 packet, u32 rest)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x24);
    const u32 frame = g.frame();
    g.setDword(frame + 0x1c, rest);
    g.setDword(frame, rest);
    if (packet != 0)
    {
        copyDwords(g, frame + 4, packet, 5);
        g.setDword(frame + 0x18, owner);
    }
    else
    {
        g.setDword(frame + 0x18, 0);
    }
    if (s32(g.dword(kTraceLevel)) >= 4)
        logEvent(g, h, ts, 0x54d2b4, frame + 0x1c, packet, 0x14);
    guest::direct<0x50e9b0, qsafeiterate>(app, ts, owner + 0x8c, 0x4f5ac0, frame);
    fe::direct<0x4dfb30, ea::eventSet>(app, ts, g.dword(owner + 0x1c));
    return 1;
}

nfs::u32 frontend::tcpFlushQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6350", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 owner = args.eax, packet = args.edx, rest = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpFlushQueue(app, ts, owner, packet, rest); });
}

/* sub_4f63d0: IPX's kind of network, once: Winsock tried, and the list told if it will do.
 * eax what to remember; the kind's record, or 0. */
u32 frontend::netb::ipxOpenWinsock(App* app, ThreadState& ts, u32 remember)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1a0);
    constexpr u32 kOpened = 0x566e84, kRemembered = 0x566e24, kRecord = 0x566e28, kTable = 0x5689e8;
    if (g.dword(kNetLock) == 0)
        g.setDword(kNetLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    if (g.dword(kOpened) == 0 && os::net::start())
    {
        const u32 found = guest::direct<0x4f5c10, neta::ipxOpen>(app, ts, 0, 0, 0);
        if (found != 0)
        {
            g.setDword(kOpened, 1);
            guest::direct<0x4f5ea0, neta::ipxClose>(app, ts, found);
            addNetKind(g, kRemembered, kTable);
            if (remember != 0)
                g.setDword(kRemembered, remember);
        }
        os::net::stop();
    }
    return g.dword(kOpened) != 0 ? kRecord : 0;
}

nfs::u32 frontend::ipxOpenWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f63d0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 remember = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxOpenWinsock(app, ts, remember); });
}

/* sub_4f7830: the same for TCP: Windows has a network, Winsock starts and a socket can be made. */
u32 frontend::netb::tcpOpenWinsock(App* app, ThreadState& ts, u32 remember)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    constexpr u32 kOpened = 0x566f48, kRemembered = 0x566ee4, kRecord = 0x566ee8, kTable = 0x566ea0;
    if (g.dword(kNetLock) == 0)
        g.setDword(kNetLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    if (g.dword(kOpened) == 0 && os::window::kNetworkPresent
        && os::net::start())
    {
        if (const std::optional<os::Handle> probe = os::net::open(app, os::net::Kind::tcp))
        {
            addNetKind(g, kRemembered, kTable);
            os::net::close(app, ts, *probe);
            g.setDword(kOpened, 1);
            if (remember != 0)
                g.setDword(kRemembered, remember);
        }
        os::net::stop();
    }
    return g.dword(kOpened) != 0 ? kRecord : 0;
}

nfs::u32 frontend::tcpOpenWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7830", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 remember = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpOpenWinsock(app, ts, remember); });
}

/* sub_4f6490: a TCP socket bound to a port, -1 for none. */
u32 frontend::netb::tcpListenSocket(App* app, ThreadState& ts, u32 port)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x20);
    const std::optional<os::Handle> socket = os::net::open(app, os::net::Kind::tcp);
    if (!socket)
        return os::net::kNoSocket;
    // Any address of this machine's.
    if (os::net::bind(app, *socket, os::net::Endpoint::inet(0, std::uint16_t(port))) == Error::none)
        return *socket;
    os::net::close(app, ts, *socket);
    return os::net::kNoSocket;
}

nfs::u32 frontend::tcpListenSocket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6490", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 port = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpListenSocket(app, ts, port); });
}

/* sub_4f6510: a socket made non-blocking, reusable and without Nagle's delay. */
u32 frontend::netb::tcpSetSocketOptions(App* app, ThreadState& ts, u32 socket)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    os::net::setWaits(app, socket, false);
    // The code asks for the address to be reused too: the port's TCP sockets always are, and a
    // port in use is never shared.
    return os::net::setNoDelay(app, socket, true) == Error::none ? 0 : 0xffffffff;
}

nfs::u32 frontend::tcpSetSocketOptions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6510", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 socket = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpSetSocketOptions(app, ts, socket); });
}

/* sub_4f6570: one connection into select's three sets, for qsafeiterate: edx the frame of
 * sub_4f68c0, its +0 the listener, +4 the read set, +0x108 the write set, +0x20c the except set
 * (a count and 64 sockets each), +0x310 the connections so added. */
u32 frontend::netb::tcpFillSelectSets(App* app, ThreadState& ts, u32 connection, u32 sets)
{
    Held h;
    h.edx = sets;
    Guest g(app, ts);
    constexpr u32 kMost = 0x40;
    if (g.dword(g.dword(connection + kConnFunctions)) != 1 || g.dword(connection + kConnSocket) == 0xffffffff
        || connection == g.dword(g.dword(sets) + 0x38))
        return 1;
    const u32 added = g.dword(sets + 0x310);
    g.setDword(sets + 0x310, added + 1);
    g.setDword(sets + 0x314 + added * 4, connection);
    const u32 reading = g.dword(sets + 4);
    if (reading < kMost)
    {
        g.setDword(sets + 4, reading + 1);
        g.setDword(sets + 8 + reading * 4, g.dword(connection + kConnSocket));
    }
    const u32 excepting = g.dword(sets + 0x20c);
    if (excepting < kMost)
    {
        g.setDword(sets + 0x20c, excepting + 1);
        h.edx = g.dword(connection + kConnSocket);
        g.setDword(sets + 0x210 + excepting * 4, h.edx);
    }
    if (g.word(g.dword(connection + kConnBuffer) + 6) != 0)
    {
        const u32 writing = g.dword(sets + 0x108);
        if (writing < kMost)
        {
            g.setDword(sets + 0x108, writing + 1);
            h.edx = g.dword(connection + kConnSocket);
            g.setDword(sets + 0x10c + writing * 4, h.edx);
        }
    }
    return 1;
}

nfs::u32 frontend::tcpFillSelectSets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6570", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 connection = args.eax, sets = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpFillSelectSets(app, ts, connection, sets); });
}

/* sub_4f6640: a packet read from a connection's socket when it has all come: a length of two
 * bytes in network order, peeked at, then the whole of it peeked at and handed to the layers;
 * taken off the socket when they have used it.  1 when the connection is dead. */
u32 frontend::netb::tcpReceivePacket(App* app, ThreadState& ts, u32 listener, u32 connection)
{
    Held h;
    h.eax = listener; h.edx = connection;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x424);
    const u32 data = g.frame();
    const u32 buffer = g.dword(connection + kConnBuffer);
    const auto receive = [&](u32 bytes, bool peek) {
        return os::net::receive(app, ts, g.dword(connection + kConnSocket), guest::memory(app, data), bytes, peek);
    };
    fe::enterLock(g, ts, g.dword(kNetLock));
    u32 dead = 0;
    const auto readFailed = [&](const os::net::Result& got) {
        if (got.value == 0)
            dead = 1;
        else if (!got.ok() && got.error != Error::wouldBlock)
            dead = 1;
    };
    const os::net::Result peeked = receive(2, true);
    if (peeked.value == 0 || (!peeked.ok() && peeked.error != Error::wouldBlock))
    {
        dead = 1;
    }
    else if (peeked.value >= 2)
    {
        const u32 size = swapped(g.dword(data)) >> 16;
        if (s32(size) > s32(u32(g.word(buffer + 4))))
        {
            dead = 1;
        }
        else
        {
            os::net::Result got = receive(size + 2, true);
            if (u32(got.value) == size + 2)
            {
                if (s32(g.dword(kTraceLevel)) >= 4)
                    logEventLeaving(g, h, 0x54d2c8, 0, connection + kConnAddress, 0x14);
                if (g.dword(listener + 0x40) != 0)
                    callHeld(g, h, g.dword(listener + 0x40),
                           args({ g.dword(connection + kConnNet), connection + kConnAddress, listener }));
                if (guest::direct<0x5126f0, layerReceive>(app, ts, connection, data + 2, size) != 0)
                    got = receive(size + 2, false);
            }
            readFailed(got);
        }
    }
    fe::leaveLock(g, ts, g.dword(kNetLock));
    return dead;
}

nfs::u32 frontend::tcpReceivePacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6640", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 listener = args.eax, connection = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpReceivePacket(app, ts, listener, connection); });
}

/* sub_4f67b0: what is left of a connection's packet sent.  1 when the connection is dead. */
u32 frontend::netb::tcpSendPending(App* app, ThreadState& ts, u32 connection)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    const u32 buffer = g.dword(connection + kConnBuffer);
    if (g.word(buffer + 6) == 0)
        return 0;
    fe::enterLock(g, ts, g.dword(kNetLock));
    const os::net::Result sent = os::net::send(app, ts, g.dword(connection + kConnSocket),
                                               guest::memory(app, g.dword(buffer + 8)), g.word(buffer + 6));
    u32 dead = 0;
    if (!sent.ok())
    {
        if (!sendLater(sent.error))
        {
            dead = 1;
            g.setWord(buffer + 6, 0);
        }
    }
    else if (sent.value > 0)
    {
        g.setWord(buffer + 6, std::uint16_t(g.word(buffer + 6) - u32(sent.value)));
        g.setDword(buffer + 8, g.dword(buffer + 8) + u32(sent.value));
    }
    fe::leaveLock(g, ts, g.dword(kNetLock));
    return dead;
}

nfs::u32 frontend::tcpSendPending(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f67b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 connection = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpSendPending(app, ts, connection); });
}

/* sub_4f6870: a dead connection closed and off the open queue, told to its owner. */
u32 frontend::netb::tcpConnectionDropped(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    h.eax = connection;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    h.ebx = connection;
    os::net::close(app, ts, g.dword(connection + kConnSocket));
    g.setDword(connection + kConnSocket, os::net::kNoSocket);
    const u32 removed = callHeld(g, h, kQRemove, args({ kOpenQueue, connection }));
    if (removed == 0)
        return 0;
    g.setDword(connection + kConnUp, 0);
    callHeld(g, h, g.dword(connection + kConnClosing), args({ connection, connection }));
    return guest::direct<0x50de50, qpush>(app, ts, kFreeQueue, h.edx);
}

nfs::u32 frontend::tcpConnectionDropped(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6870", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpConnectionDropped(app, ts, connection); });
}

/* sub_4f68c0: the TCP thread: selects over the listening socket, a connection going up and the
 * connections open, accepts, receives and sends, until the listener says stop.  Its frame is
 * what sub_4f6570 fills; +0x414 what accept gives. */
u32 frontend::netb::tcpServerThread(App* app, ThreadState& ts)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x448);
    constexpr u32 kStop = 0x46, kRunning = 0x47, kGame = 0x3c, kAccepted = 0x1c, kPeerAddress = 0x20;
    constexpr u32 kAcceptedBy = 0x34, kListenSocket = 0x18, kConnecting = 0x38;
    const u32 frame = g.frame();
    const u32 reading = frame + 4, writing = frame + 0x108, excepting = frame + 0x20c, added = frame + 0x310;
    const u32 peer = frame + 0x414;
    const u32 listener = g.dword(kListener);
    g.setDword(frame, listener);
    g.setByte(listener + kRunning, 1);
    while (g.byte(listener + kStop) == 0)
    {
        // A socket accepted and not yet taken up: closed when it has waited too long, or
        // the game does not want more.
        const u32 game = g.dword(listener + kGame);
        const u32 wants = game != 0 ? g.dword(game + 0x10) : 0;
        const u32 accepted = g.dword(listener + kAccepted);
        if (accepted != 0xffffffff)
        {
            bool stale = true;
            if (wants == 0 || callHeld(g, h, wants, args({ game, listener + kPeerAddress, wants, accepted })) != 0)
                stale = s32(g.dword(kNow)) > s32(g.dword(listener + kAcceptedBy));
            if (stale)
            {
                os::net::close(app, ts, g.dword(listener + kAccepted));
                g.setDword(listener + kAccepted, os::net::kNoSocket);
            }
        }
        g.setDword(reading, 0);
        g.setDword(writing, 0);
        g.setDword(excepting, 0);
        g.setDword(added, 0);
        if (g.dword(listener + kAccepted) == 0xffffffff)
        {
            g.setDword(reading, 1);
            g.setDword(reading + 4, g.dword(listener + kListenSocket));
        }
        else
        {
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
        }
        const u32 going = g.dword(listener + kConnecting);
        if (going != 0)
        {
            const u32 inWrite = g.dword(writing);
            if (inWrite < 0x40)
            {
                g.setDword(writing, inWrite + 1);
                g.setDword(writing + 4 + inWrite * 4, g.dword(going + kConnSocket));
            }
            const u32 inExcept = g.dword(excepting);
            if (inExcept < 0x40)
            {
                g.setDword(excepting, inExcept + 1);
                g.setDword(excepting + 4 + inExcept * 4, g.dword(g.dword(listener + kConnecting) + kConnSocket));
            }
        }
        guest::direct<0x50e9b0, qsafeiterate>(app, ts, kOpenQueue, 0x4f6570, frame);
        fe::enterLock(g, ts, g.dword(kNetLock));
        fe::leaveLock(g, ts, g.dword(kNetLock));
        // A wait of 25 milliseconds at most.
        if (os::net::select(app, ts, setAt(g, reading), setAt(g, writing), setAt(g, excepting), 25).ok())
        {
            if (isSet(g, g.dword(listener + kListenSocket), reading))
            {
                const std::optional<os::net::Accepted> accepted = os::net::accept(app, ts, g.dword(listener + kListenSocket));
                g.setDword(listener + kAccepted, accepted ? accepted->socket : os::net::kNoSocket);
                if (!accepted)
                {
                    g.setByte(listener + kStop, 1);
                }
                else
                {
                    // The peer's address, its first word cleared, kept in the listener.
                    os::net::putInet(app, peer, accepted->peer);
                    g.setWord(peer, 0);
                    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, listener + kPeerAddress, peer, os::net::kInetBytes);
                    guest::direct<0x4f6510, tcpSetSocketOptions>(app, ts, g.dword(listener + kAccepted));
                    g.setDword(listener + kAcceptedBy, g.dword(kNow) + (g.dword(kTicksPerSecond) << 2));
                }
            }
            const u32 connecting = g.dword(listener + kConnecting);
            if (connecting != 0)
            {
                if (isSet(g, g.dword(connecting + kConnSocket), excepting))
                {
                    g.setDword(connecting + kConnUp, 0);
                    g.setDword(listener + kConnecting, 0);
                }
                else if (isSet(g, g.dword(connecting + kConnSocket), writing))
                {
                    g.setDword(connecting + kConnUp, 1);
                    g.setDword(listener + kConnecting, 0);
                }
            }
            for (s32 i = 0; i < s32(g.dword(added)); ++i)
            {
                const u32 at = frame + 0x314 + u32(i) * 4;
                const u32 connection = g.dword(at);
                u32 dead = 0;
                if (g.dword(connection + kConnSocket) != os::net::kNoSocket && connection != connecting)
                {
                    if (isSet(g, g.dword(connection + kConnSocket), reading))
                        dead = guest::direct<0x4f6640, tcpReceivePacket>(app, ts, listener, g.dword(at));
                    if (isSet(g, g.dword(g.dword(at) + kConnSocket), writing))
                        dead |= guest::direct<0x4f67b0, tcpSendPending>(app, ts, g.dword(at));
                    if (isSet(g, g.dword(g.dword(at) + kConnSocket), excepting))
                        dead |= guest::direct<0x4f6860, tcpExceptionSeen>(app, ts);
                }
                if (dead != 0 && g.dword(at) != 0)
                    guest::direct<0x4f6870, tcpConnectionDropped>(app, ts, g.dword(at));
            }
        }
    }
    g.setByte(listener + kRunning, 0);
    return listener;
}

nfs::u32 frontend::tcpServerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f68c0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpServerThread(app, ts); });
}

/* sub_4f6c50: a connection opened over TCP: with no address, by waiting for someone to connect
 * to the thread; with one, by connecting there, the two ends' sockets racing, what is accepted
 * from that address winning.  1 when it is up. */
u32 frontend::netb::tcpOpenConnection(App* app, ThreadState& ts, u32 connection, u32 room)
{
    Held h;
    h.eax = connection; h.ebx = room;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1c);
    constexpr u32 kAccepted = 0x1c, kPeerAddress = 0x20, kConnecting = 0x38;
    if (connection == 0)
        return room + 0xe;
    const u32 frame = g.frame();
    const u32 bufferAt = frame + 4, addressAt = frame + 8, socketAt = frame + 0xc, listenerAt = frame + 0x10;
    const u32 given = g.dword(connection + kConnBuffer);
    u32 buffer = given;
    if (given == 0)
    {
        const u32 size = room + 0xe;
        setErrorAt(g, 0x54d2d0, 0x54d2dc, 0x1ac);
        h.ecx = 0x54d2d0;
        buffer = callHeld(g, h, kReserve, args({ 0x54d2f0, size, g.dword(kMemoryGroup) }));
    }
    g.setDword(bufferAt, buffer);
    g.setDword(buffer, g.dword(connection + kConnBuffer) == 0 ? 1 : 0);
    const u32 maximum = g.word(kMaxPacket);
    g.setWord(buffer + 6, 0);
    g.setDword(buffer + 8, 0);
    g.setWord(buffer + 4, std::uint16_t(maximum != 0 ? maximum - 2 : 0x400));
    const u32 listener = g.dword(g.dword(connection + kConnNet) + 0xc);
    g.setDword(listenerAt, listener);
    g.setDword(connection + kConnUp, 0);
    const std::uint16_t family = g.word(connection + kConnAddress);
    h.ebx = (h.ebx & 0xffff0000) | family;
    g.setDword(connection + kConnBuffer, buffer);
    h.edx = buffer;

    const auto ready = [&] { return callHeld(g, h, g.dword(connection + kConnReady), args({ connection })); };
    const auto oneSecond = [&](u32 ticks) {
        const s32 each = s32(0x3e8) / s32(ticks);
        h.edx = u32(s32(0x3e8) % s32(ticks));
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, u32(each));
    };

    bool waiting = family == 0;  // esi of the code: no address, or given up
    if (waiting)
    {
        h.ebx = 0;
        while (g.dword(listener + kAccepted) == 0xffffffff)
        {
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
            {
                if (s32(u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
            }
            if (ready() != 0)
                break;
            h.ecx = g.dword(kTicksPerSecond);
            oneSecond(h.ecx);
        }
    }

    bool takeAccepted = false;
    if (g.dword(listener + kAccepted) != 0xffffffff)
    {
        takeAccepted = waiting
            || callHeld(g, h, kCheckPlayer, args({ g.dword(connection + kConnNet), connection + kConnAddress, listener + kPeerAddress })) != 0;
    }
    if (takeAccepted)
    {
        fe::enterLock(g, ts, g.dword(kNetLock));
        g.setDword(connection + kConnSocket, g.dword(listener + kAccepted));
        copyDwords(g, connection + kConnAddress, listener + kPeerAddress, 5);
        g.setDword(connection + kConnUp, 1);
        g.setDword(listener + kAccepted, 0xffffffff);
        fe::leaveLock(g, ts, g.dword(kNetLock));
    }
    else if (!waiting)
    {
        const auto giveUp = [&] {
            waiting = true;
            g.setDword(listener + kConnecting, 0);
            g.setDword(connection + kConnUp, 0);
        };
        g.setDword(listener + kConnecting, connection);
        g.setDword(addressAt, connection + kConnAddress);
        while (!waiting)
        {
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
            {
                if (s32(u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0))) < 0)
                {
                    giveUp();
                    if (g.dword(listener + kConnecting) != 0)
                        continue;
                    break;
                }
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
            }
            if (ready() != 0)
            {
                giveUp();
                if (g.dword(listener + kConnecting) != 0)
                    continue;
                break;
            }
            const u32 socket = guest::direct<0x4f6490, tcpListenSocket>(app, ts, 0);
            h.ebx = socket;
            g.setDword(socketAt, socket);
            const u32 until = os::time::ticks(g.app(), ts) + 0x3e8;
            if (socket == 0xffffffff)
            {
                waiting = true;
                if (g.dword(listener + kConnecting) != 0)
                    continue;
                break;
            }
            guest::direct<0x4f6510, tcpSetSocketOptions>(app, ts, socket);
            // The socket waits on nothing: the connection is under way, or refused at once.
            const os::net::Error error = os::net::connect(app, ts, socket, os::net::inetAt(app, g.dword(addressAt)));
            if (error == Error::wouldBlock)
            {
                g.setDword(connection + kConnSocket, socket);
                while (g.dword(listener + kConnecting) != 0)
                {
                    if (until <= os::time::ticks(g.app(), ts))
                        break;
                    bool failed = false;
                    if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
                        failed = s32(u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0))) < 0;
                    else
                        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
                    if (failed || ready() != 0)
                    {
                        giveUp();
                    }
                    else
                    {
                        h.ebx = g.dword(kTicksPerSecond);
                        oneSecond(h.ebx);
                    }
                }
            }
            else if (connectFailed(error))
            {
                giveUp();
            }
            if (g.dword(connection + kConnUp) != 0)
            {
                if (g.dword(listener + kConnecting) != 0)
                    continue;
                break;
            }
            os::net::close(app, ts, g.dword(socketAt));
            if (g.dword(listener + kConnecting) == 0)
                break;
        }
    }

    h.edx = g.dword(connection + kConnUp);
    if (h.edx == 0 && g.dword(g.dword(bufferAt)) != 0)
    {
        g.setDword(connection + kConnBuffer, h.edx);
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, g.dword(bufferAt));
    }
    else if (g.dword(connection + kConnUp) != 0)
    {
        guest::direct<0x50de50, qpush>(app, ts, kOpenQueue, connection);
    }
    return g.dword(connection + kConnUp);
}

nfs::u32 frontend::tcpOpenConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6c50", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 connection = args.eax, room = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpOpenConnection(app, ts, connection, room); });
}

/* sub_4f7030: a packet sent on a connection: its length in network order before it, into the
 * connection's buffer, and what the socket will not take now left there.  0 while a packet
 * before it is still going, or when the socket is not connected yet. */
u32 frontend::netb::tcpSendConnection(App* app, ThreadState& ts, u32 connection, u32 data, u32 length)
{
    Held h;
    h.eax = connection; h.edx = data; h.ebx = length;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    const u32 frame = g.frame();
    const u32 buffer = g.dword(connection + kConnBuffer);
    const u32 room = g.word(buffer + 4);
    if (length > room)
    {
        setErrorAt(g, 0x54d2d0, 0x54d2f8, 0x22e);
        h.edx = (0x54d2d0);  // as the code loads them for it, which Error keeps
        h.ebx = (0x22e);
        fe::direct<0x401010, ea::error>(app, ts, 0x54d30cu, guest::Words({ room }));
    }
    if (g.word(buffer + 6) != 0)
        return 0;
    fe::enterLock(g, ts, g.dword(kNetLock));
    g.setWord(buffer + 0xc, std::uint16_t(swapped(length) >> 16));
    callHeld(g, h, kMemmove, args({ data, buffer + 0xe, length }));
    const u32 whole = length + 2;
    g.setDword(buffer + 8, buffer + 0xc);
    g.setWord(buffer + 6, std::uint16_t(whole));
    const os::net::Result sent = os::net::send(app, ts, g.dword(connection + kConnSocket),
                                               guest::memory(app, g.dword(buffer + 8)), whole & 0xffff);
    if (!sent.ok())
    {
        g.setDword(frame, errorNumber(sent.error));
        if (s32(g.dword(kTraceLevel)) >= 4)
            guest::direct<0x512180, commEvent>(app, ts, 0x54d344, frame, connection + kConnSocket, 4, 1, 0);
        if (sendLater(sent.error))
        {
            g.setWord(buffer + 6, 0);
            fe::leaveLock(g, ts, g.dword(kNetLock));
            return 0;
        }
    }
    else if (sent.value > 0)
    {
        g.setWord(buffer + 6, std::uint16_t(g.word(buffer + 6) - u32(sent.value)));
        g.setDword(buffer + 8, g.dword(buffer + 8) + u32(sent.value));
        h.ebx = g.dword(kTraceLevel);
        if (s32(h.ebx) >= 4)
            logEvent(g, h, ts, 0x54d350, 0, connection + kConnAddress, 0x14);
    }
    fe::leaveLock(g, ts, g.dword(kNetLock));
    return 1;
}

nfs::u32 frontend::tcpSendConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7030", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 connection = args.eax, data = args.edx, length = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpSendConnection(app, ts, connection, data, length); });
}

/* sub_4f71a0: a connection closed for good and its buffer freed if it was its own. */
u32 frontend::netb::tcpCloseConnection(App* app, ThreadState& ts, u32 connection)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    if (got(fe::direct<0x50e150, ea::queueRemoved>(app, ts, kOpenQueue, connection, 0)) != 0)
    {
        os::net::shutdown(app, g.dword(connection + kConnSocket));
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
        os::net::close(app, ts, g.dword(connection + kConnSocket));
        g.setDword(connection + kConnSocket, os::net::kNoSocket);
        g.setDword(connection + kConnUp, 0);
        guest::direct<0x50de50, qpush>(app, ts, kFreeQueue, connection);
    }
    if (got(fe::direct<0x50e150, ea::queueRemoved>(app, ts, kFreeQueue, connection, 0)) == 0)
        return 0;
    const u32 buffer = g.dword(connection + kConnBuffer);
    if (g.dword(buffer) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, buffer);
        g.setDword(connection + kConnBuffer, 0);
    }
    return 1;
}

nfs::u32 frontend::tcpCloseConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f71a0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpCloseConnection(app, ts, connection); });
}

/* sub_4f7220: whether two addresses are the same: a dword, two words and 8 bytes. */
u32 frontend::netb::tcpSameAddressParts(App* app, ThreadState& ts, u32 first, u32 second)
{
    Held h;
    h.edx = second;
    Guest g(app, ts);
    h.edx = g.dword(second);
    if (h.edx != g.dword(first))
        return 0;
    h.edx = (h.edx & 0xffff0000) | g.word(second + 4);
    if (g.word(second + 4) != g.word(first + 4) || g.word(second + 6) != g.word(first + 6))
        return 0;
    for (h.edx = 0; h.edx < 8; ++h.edx)
        if (g.byte(first + 8 + h.edx) != g.byte(second + 8 + h.edx))
            return 0;
    return 1;
}

nfs::u32 frontend::tcpSameAddressParts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7220", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 first = args.eax, second = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpSameAddressParts(app, ts, first, second); });
}

/* sub_4f7280: a game hosted over a network: edx the player's address, ebx what it is opened
 * with, the stack the game's name and the player's. */
u32 frontend::netb::tcpHostGame(App* app, ThreadState& ts, u32 net, u32 player, u32 address, u32 gameName, u32 playerName)
{
    Held h;
    h.eax = net; h.edx = player; h.ebx = address;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    constexpr u32 kOpenResult = 0x566f44;
    if (guest::direct<0x4f7220, tcpSameAddressParts>(app, ts, address, g.dword(net + 8) + 4) == 0)
        return 0;
    h.edx = 8;  // as it leaves edx when the parts agree: the kind's open below takes it
    guest::direct<0x4f5230, neta::servicesReady>(app, ts);
    const u32 opened = callHeld(g, h, g.dword(g.dword(net + 8) + 0x14), args({ g.dword(net + 8) }));
    g.setDword(kOpenResult, opened);
    if (opened == 0)
        return 0;
    const u32 functions = g.dword(net + 8);
    const u32 game = callHeld(g, h, g.dword(functions + 0x1c), args({ h.ecx, net, 0 }));
    g.setDword(net + 0xc, game);
    if (game == 0)
        return 0;
    const u32 same = g.dword(net + 8);
    callHeld(g, h, g.dword(same + 0x44), args({ game, net + 0x30, 0x10, same }));
    g.setDword(net + 0x9c, 1);
    g.setDword(net + 0x14, 0);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, net + 0x8c, player, h.ebx);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), net + 0x44, gameName, 0x33);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), net + 0x78, playerName, 0x13);
    return g.dword(kOpenResult);
}

nfs::u32 frontend::tcpHostGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7280", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 net = args.eax;
    const u32 player = args.edx;
    const u32 address = args.ebx;
    const u32 gameName = app->getMemory<u32>(ts.sp + 4);
    const u32 playerName = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpHostGame(app, ts, net, player, address, gameName, playerName); });
}

/* sub_4f7320: the game of a NET object ended: its players told, its game closed. */
u32 frontend::netb::tcpEndGame(App* app, ThreadState& ts, u32 net)
{
    Held h;
    h.eax = net;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    const u32 game = g.dword(net + 0xc);
    h.edx = net;
    h.ecx = game;
    if (game == 0)
        return 1;
    if (g.dword(net + 0xa0) != 0)
        callHeld(g, h, 0x4f4f90, args({ net }));
    // ebx is saved only from here on, past what sub_4f4f90 leaves in it.
    const u32 ebx = h.ebx;
    const u32 functions = g.dword(net + 8);
    callHeld(g, h, g.dword(functions + 0x20), args({ game, net, functions, game }));
    g.setDword(net + 0x9c, 0);
    const u32 same = g.dword(net + 8);
    g.setDword(net + 0xc, 0);
    const u32 result = callHeld(g, h, g.dword(same + 0x18), args({ same }));
    h.ebx = ebx;
    return result;
}

nfs::u32 frontend::tcpEndGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7320", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const u32 net = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpEndGame(app, ts, net); });
}

/* sub_4f7370: what the kinds' tables hold for a function a kind has no need of. */
u32 frontend::netb::returnZero(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::netReturnZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7370", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::returnZero(app, ts); });
}

/* sub_4f7380: a network over TCP: sub_4f5130 with this kind's table at 0x566e8c. */
u32 frontend::netb::tcpOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 first, u32 second)
{
    Guest g(app, ts);
    if (settings != 0)
        g.setWord(settings + 0x28, 1);
    return guest::direct<0x4f5130, neta::serviceCall>(app, ts, settings, 0x566e8c, 0, first, second);
}

nfs::u32 frontend::tcpOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7380", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 settings = args.edx, first = args.ebx, second = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpOpenNetwork(app, ts, settings, first, second); });
}

/* sub_4f73a0: Winsock started for one more user, a socket's limit read once; the largest packet
 * a connection will carry, 0 when Winsock will not start. */
u32 frontend::netb::tcpStartWinsock(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 8);
    constexpr u32 kUsers = 0x566e88;
    if (g.dword(kUsers) == 0 && !os::net::start())
        return 0;
    if (const std::optional<os::Handle> socket = os::net::open(app, os::net::Kind::tcp))
    {
        fe::direct<0x5150f0, frontend::eab::netLocalAddress>(app, ts, 0xa0b120);
        os::net::close(app, ts, *socket);
    }
    g.setDword(kUsers, g.dword(kUsers) + 1);
    const u32 maximum = g.word(kMaxPacket);
    if (maximum == 0)
        return 0x400;
    const u32 room = maximum - 2;
    return s32(room) <= 0x400 ? room : 0x400;
}

nfs::u32 frontend::tcpStartWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f73a0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpStartWinsock(app, ts); });
}

/* sub_4f7420: one user of Winsock less, stopped with the last; 0 when it was not started or
 * would not stop. */
u32 frontend::netb::tcpStopWinsock(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 8);
    constexpr u32 kUsers = 0x566e88;
    const u32 users = g.dword(kUsers);
    if (users == 0)
        return 0;
    g.setDword(kUsers, users - 1);
    if (users - 1 == 0)
        os::net::stop();
    return 1;
}

nfs::u32 frontend::tcpStopWinsock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7420", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpStopWinsock(app, ts); });
}

/* ---------------------------------------------------------------------------
 * win\tapipkt.c: the modem, through TAPI, and the serial port's table
 * ------------------------------------------------------------------------- */
namespace
{

constexpr u32 kTapiFile = 0x54da88;
constexpr u32 kLineApp = 0x9f4d74, kDeviceCount = 0x9f4d90, kApiVersion = 0x9f4d8c, kDevCaps = 0x9f4d88;
constexpr u32 kModemCount = 0x9f4d80, kModems = 0x9f4d94, kModemSettings = 0x9f4d84, kLine = 0x9f4da0, kCall = 0x9f4da4;
constexpr u32 kModemHandle = 0x9f4da8, kModemState = 0x9f4d9c, kThreadStarting = 0x9f4d98, kThreadId = 0x9f4d70;
constexpr u32 kChosenDevice = 0x5671a8, kModemCallback = 0x5671ac, kSerialCallback = 0x9f4eac;
constexpr u32 kCommConfig = 0x9f4d78, kAnswerMode = 0x9f4d7c, kCommDcb = 0x9f3d60, kCommClass = 0x54db40;
constexpr u32 kFileReport = 0x501050, kReportStream = 0x564454, kReportText = 0x56abb4;
constexpr u32 kApiLow = 0x10004;       // the version of TAPI asked for, low and high
constexpr u32 kStateConnected = 9;

u32 fileReport(Guest& g, u32 text)
{
    g.setDword(kReportText, text);
    const u32 result = *fe::direct<0x501050, ea::reportedAt>(g.app(), g.ts(), kReportStream, 1u, text, guest::Words({}));
    return result;
}

/* The modem's thread told to stop: its loop's flag cleared, and its message queue would get
 * WM_USER+200 (0x4c8) -- the port has no thread's own messages, so the thread sees the
 * flag the next time its wait ends. */
void postQuit(Guest& g, nfs::ThreadState& ts, u32 settings)
{
    NFS2_USE(ts);
    g.setDword(settings + 0x34, 0);
}

/* What sub_4fa550 and sub_4fa780 share: the timeouts a network's settings leave at 0 taken from
 * the ticks of a second, and its kind. */
void fillTimeouts(Guest& g, u32 settings, u32 ticks, s32 second, std::uint16_t kind)
{
    if (settings == 0)
        return;
    if (g.word(settings + 0x2a) == 0)
        g.setWord(settings + 0x2a, std::uint16_t(s32(ticks) / 16));
    if (g.word(settings + 0x2c) == 0)
        g.setWord(settings + 0x2c, std::uint16_t(s32(ticks) / second));
    if (g.word(settings + 0x2e) == 0)
        g.setWord(settings + 0x2e, std::uint16_t(s32(ticks) / second));
    g.setWord(settings + 0x28, kind);
}

/* sub_4fa500 and sub_4fa730: bytes sent on an object's port, a failure's count made positive. */
u32 portSend(Guest& g, nfs::ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    Held h;
    const u32 port = g.dword(object + 0x38);
    // The port in ecx too, which it does not read.
    const u32 result = fe::direct<0x51ba50, frontend::netc::serialRead>(g.app(), ts, port, buffer, length);
    h.edx = s32(result) < 0 ? 0 - result : result;
    return h.edx;
}

/* sub_4fa530 and sub_4fa760: bytes received. */
u32 portReceive(Guest& g, nfs::ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    const u32 port = g.dword(object + 0x38);
    // The port in ecx too, which it does not read.
    return fe::direct<0x51b530, frontend::netc::serialWriteWhole>(g.app(), ts, port, buffer, length);
}

}  // namespace

/* sub_4f9a50: the modem of an index chosen, the list made first if it has not been. */
u32 frontend::netb::tapiSelectModem(App* app, ThreadState& ts, u32 index)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 8);
    if (g.dword(kModems) == 0)
        guest::direct<0x4f9ac0, tapiEnumerateModems>(app, ts);
    if (s32(index) >= s32(g.dword(kModemCount)))
    {
        setErrorAt(g, kTapiFile, 0x54daec, 0x176);
        fe::direct<kError, ea::error>(g.app(), g.ts(), 0x54dafc, guest::Words({}));
        return 0;
    }
    g.setDword(kChosenDevice, g.dword(g.dword(kModems) + index * 8));
    return 1;
}

nfs::u32 frontend::tapiSelectModem(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9a50", 0, 0, nullptr, 0, kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiSelectModem(app, ts, index); });
}

/* sub_4f9ac0: TapiEnumerateModems: the list of the lines that are modems, 20 places of a device
 * and its name; how many.  ebx is saved only where the code saves it, past lineInitialize. */
u32 frontend::netb::tapiEnumerateModems(App* app, ThreadState& ts)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x28);
    constexpr u32 kListBytes = 0xa0, kCapsDatamodem = 0x10;
    g.setDword(kModemCount, 0);
    const u32 window = callHeld(g, h, 0x4eba70);
    // The window's module, for TAPI: the port's windows have none of their own, 0.
    NFS2_USE(window);
    const u32 instance = 0;
    if (fe::direct<0x5310be, frontend::watc::lineInitialize>(app, ts, kLineApp, instance, 0x4f9500, 0, kDeviceCount) != 0)
        return 0;
    if (g.dword(kDeviceCount) == 0)
    {
        // eax as this call leaves it, handed on to the stub, which does not read it.
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0x1f4);
        if (fe::direct<0x5324d2, frontend::watc::lineShutdown>(app, ts, g.dword(kLineApp)) != 0)
            fileReport(g, 0x54da70);
        return 0;
    }
    if (g.dword(kModems) != 0)
    {
        for (u32 at = 0; at != kListBytes; at += 8)
        {
            const u32 name = g.dword(g.dword(kModems) + at + 4);
            if (name != 0)
                fe::direct<0x4e1890, ea::purgeBlock>(app, ts, name);
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, g.dword(kModems));
        g.setDword(kModems, 0);
    }
    const u32 savedEbx = h.ebx;
    setErrorAt(g, kTapiFile, 0x54db14, 0x1a9);
    g.setDword(kModems, u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54db28, kListBytes, g.dword(kMemoryGroup))));
    for (u32 at = 0; at != kListBytes; at += 8)
    {
        g.setDword(g.dword(kModems) + at + 4, 0);
        g.setDword(g.dword(kModems) + at, 0);
    }
    const u32 version = g.frame() + 4;
    for (u32 device = 0; device < g.dword(kDeviceCount); ++device)
    {
        if (fe::direct<0x53138f, frontend::watc::lineNegotiateAPIVersion>(app, ts, g.dword(kLineApp), device, kApiLow,
                                                                          kApiLow, kApiVersion, version) != 0)
            continue;
        if (guest::direct<0x4f9f30, tapiGetDevCaps>(app, ts, device) != 0 || (g.byte(g.dword(kDevCaps) + 0x3c) & kCapsDatamodem) == 0)
            continue;
        const u32 caps = g.dword(kDevCaps);
        setErrorAt(g, kTapiFile, 0x54db14, 0x1c1);
        const u32 name = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54db34, g.dword(caps + 0x20) + 1, g.dword(kMemoryGroup)));
        const u32 place = g.dword(kModems) + (g.dword(kModemCount) << 3);
        g.setDword(place + 4, name);
        const u32 caps2 = g.dword(kDevCaps);
        fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), g.dword(place + 4), g.dword(caps2 + 0x24) + caps2, g.dword(caps2 + 0x20));
        const u32 place2 = g.dword(kModems) + (g.dword(kModemCount) << 3);
        g.setByte(g.dword(place2 + 4) + g.dword(g.dword(kDevCaps) + 0x20), 0);
        g.setDword(place2, device);
        g.setDword(kModemCount, g.dword(kModemCount) + 1);
    }
    fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0x1f4);
    g.dword(kLineApp);  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x5324d2, frontend::watc::lineShutdown>(app, ts, g.dword(kLineApp)) != 0)
        fileReport(g, 0x54da70);
    h.ebx = savedEbx;
    return g.dword(kModemCount);
}

nfs::u32 frontend::tapiEnumerateModems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9ac0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiEnumerateModems(app, ts); });
}

/* sub_4f9d20: the chosen line opened, its comm's settings seen to when asked for. */
u32 frontend::netb::tapiOpenLine(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x24);
    const u32 device = g.dword(kChosenDevice), version = g.frame();
    if (device == 0xffffffff)
        return 0;
    if (fe::direct<0x53138f, frontend::watc::lineNegotiateAPIVersion>(app, ts, g.dword(kLineApp), device, kApiLow,
                                                                      kApiLow, kApiVersion, version) != 0)
        return 0;
    if (guest::direct<0x4f9f30, tapiGetDevCaps>(app, ts, g.dword(kChosenDevice)) != 0)
        return 0;
    const u32 mode = g.dword(kAnswerMode) != 0 ? 4 : 1;
    const u32 app_ = g.dword(kLineApp);
    if (fe::direct<0x531439, frontend::watc::lineOpen>(app, ts, app_, g.dword(kChosenDevice), kLine, g.dword(kApiVersion), 0, 0,
                                                       mode, 0x10, 0) != 0)
        return 0;
    if (g.dword(kCommConfig) != 0)
    {
        guest::direct<0x4f9ea0, tapiSetCommConfig>(app, ts);
        const u32 caps = g.dword(kDevCaps);
        fe::direct<0x532162, frontend::watc::lineSetStatusMessages>(app, ts, g.dword(kLine), g.dword(caps + 0x80), 0);
    }
    return 1;
}

nfs::u32 frontend::tapiOpenLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9d20", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiOpenLine(app, ts); });
}

/* sub_4f9df0: the line's comm port asked for, its DCB read into 0x9f3d60 and the handle closed. */
u32 frontend::netb::tapiGetCommHandleConfig(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1f58);
    constexpr u32 kBytes = 0x1f40;
    const u32 id = g.frame();
    fe::direct<0x4e0640, watcom::clib::memset>(app, ts, id, 0u, kBytes);
// as the code loads it for it, which it keeps
    g.setDword(id, kBytes);
    if (fe::direct<0x530a4e, frontend::watc::lineGetID>(app, ts, g.dword(kLine), 0, 0, 1, id, kCommClass) != 0)
        return 0;
    const u32 handleAt = id + g.dword(id + 0x14);
    const bool read = os::serial::settings(app, g.dword(handleAt)).has_value();
    os::release(app, g.dword(handleAt));
    // The port's ports have no settings to read: the configuration at kCommDcb is never filled.
    if (!read)
        return 0;
    g.setDword(kCommConfig, kCommDcb + 0x3c);
    return 1;
}

nfs::u32 frontend::tapiGetCommHandleConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9df0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiGetCommHandleConfig(app, ts); });
}

/* sub_4f9ea0: the chosen device's configuration read, its low three bits of settings cleared,
 * and set again. */
u32 frontend::netb::tapiSetCommConfig(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1f54);
    constexpr u32 kBytes = 0x1f40;
    const u32 config = g.frame() + 4;
    fe::direct<0x4e0640, watcom::clib::memset>(app, ts, config, 0u, kBytes);
// as the code loads it for it, which it keeps
    g.setDword(config, kBytes);
    if (fe::direct<0x53083d, frontend::watc::lineGetDevConfig>(app, ts, g.dword(kChosenDevice), config, kCommClass) != 0
        || g.dword(config + 4) > g.dword(config))
        return 0;
    const u32 data = config + g.dword(config + 0x14);
    const u32 settings = data + 0x3c + 0x20;
    g.setByte(settings, std::uint8_t(g.byte(settings) & 0xf8));
    const u32 result = fe::direct<0x531f31, frontend::watc::lineSetDevConfig>(app, ts, g.dword(kChosenDevice), data,
                                                                         g.dword(config + 0x10), kCommClass);
    return result != 0 ? 1 : 0;
}

nfs::u32 frontend::tapiSetCommConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9ea0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiSetCommConfig(app, ts); });
}

/* sub_4f9f30: TapiGetDevCaps: a device's capabilities in [0x9f4d88], the block made as large as
 * it says it needs; TAPI's error, or 0. */
u32 frontend::netb::tapiGetDevCaps(App* app, ThreadState& ts, u32 device)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    const auto drop = [&] {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, g.dword(kDevCaps));
        g.setDword(kDevCaps, 0);
    };
    if (g.dword(kDevCaps) != 0)
        drop();
    setErrorAt(g, kTapiFile, 0x54db50, 0x25a);
    u32 size = 0xf0;
    u32 block = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54db60, size, g.dword(kMemoryGroup)));
    g.setDword(kDevCaps, block);
    g.setDword(block, size);
    for (;;)
    {
        const u32 caps = g.dword(kDevCaps);
        const u32 error = fe::direct<0x530747, frontend::watc::lineGetDevCaps>(app, ts, g.dword(kLineApp), device,
                                                                         g.dword(kApiVersion), 0, caps);
        if (error != 0)
        {
            drop();
            return error;
        }
        size = g.dword(g.dword(kDevCaps) + 4);
        if (size <= g.dword(g.dword(kDevCaps)))
            return 0;
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, g.dword(kDevCaps));
        setErrorAt(g, kTapiFile, 0x54db50, 0x26f);
        block = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54db60, size, g.dword(kMemoryGroup)));
        g.setDword(kDevCaps, block);
        g.setDword(block, size);
    }
}

nfs::u32 frontend::tapiGetDevCaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9f30", 0, 0, nullptr, 0 };
    const u32 device = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiGetDevCaps(app, ts, device); });
}

/* sub_4fa030: TapiMakeCall: a number dialled on the open line. */
u32 frontend::netb::tapiMakeCall(App* app, ThreadState& ts, u32 number)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    if (number == 0 || g.dword(kLine) == 0)
        return 0;
    setErrorAt(g, kTapiFile, 0x54db70, 0x287);
    const u32 parameters = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54db80, 0x70, g.dword(kMemoryGroup)));
    fe::direct<kMemset, watcom::clib::memset>(g.app(), g.ts(), parameters, 0, 0x70);
    g.setDword(parameters, 0x70);
    g.setDword(parameters + 4, 1);
    g.setDword(parameters + 0x10, 0x10);
    g.setDword(parameters + 0x2c, 0xbb8);
    const u32 result = fe::direct<0x5311c0, frontend::watc::lineMakeCall>(app, ts, g.dword(kLine), kCall, number, 0, parameters);
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, parameters);
    if (s32(result) < 0)
    {
        g.setDword(kModemState, 0xf);
        return 0;
    }
    return 1;
}

nfs::u32 frontend::tapiMakeCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa030", 0, 0, nullptr, 0 };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiMakeCall(app, ts, number); });
}

/* sub_4fa0f0: TapiGetModemHandle: the comm handle of the call in [0x9f4da8]; 0, or -1. */
u32 frontend::netb::tapiGetModemHandle(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x18);
    setErrorAt(g, kTapiFile, 0x54db94, 0x2aa);
    u32 block = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54dba8, 0x18, g.dword(kMemoryGroup)));
    g.setDword(block, 0x18);
    for (;;)
    {
        if (fe::direct<0x530a4e, frontend::watc::lineGetID>(app, ts, 0, 0, g.dword(kCall), 4, block, kCommClass) != 0)
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
            return 0xffffffff;
        }
        const u32 needed = g.dword(block + 4);
        if (needed <= g.dword(block))
        {
            g.setDword(kModemHandle, g.dword(block + g.dword(block + 0x14)));
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
            return 0;
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
        setErrorAt(g, kTapiFile, 0x54db94, 0x2bd);
        block = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54dba8, needed, g.dword(kMemoryGroup)));
        g.setDword(block, needed);
    }
}

nfs::u32 frontend::tapiGetModemHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa0f0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiGetModemHandle(app, ts); });
}

/* sub_4fa1d0: the call dropped. */
u32 frontend::netb::tapiHangUp(App* app, ThreadState& ts)
{
    return tapiHangUpFrom(app, ts, 0);
}

u32 frontend::netb::tapiHangUpFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    if (g.dword(kCall) != 0)
    {
        constexpr u32 kDropText = 0x54dbb8;
        // The report's text is kept in ebx over lineDrop (the callee keeps it).
        fe::direct<0x52fb2e, frontend::watc::lineDrop>(app, ts, g.dword(kCall), 0, 0);
        h.eax = fileReport(g, kDropText);
    }
    return h.eax;
}

nfs::u32 frontend::tapiHangUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa1d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiHangUpFrom(app, ts, handed); });
}

/* sub_4fa210: the modem's thread: the modem chosen, TAPI set up, the number dialled, and
 * the port's events handed to the window's procedure until told to stop; then the line hung
 * up. */
u32 frontend::netb::tapiThread(App* app, ThreadState& ts)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x20);
    if (guest::direct<0x4f9a50, tapiSelectModem>(app, ts, g.dword(g.dword(kModemSettings) + 0x3c)) == 0)
    {
        g.setDword(kModemState, 0x80000002);
        g.setDword(kModemSettings, 0);
        return 0;
    }
    const u32 state = fe::direct<0x4f9920, frontend::wata::tapiSetup>(
        app, ts, u32(g.dword(g.dword(kModemSettings) + 0x68) == 0 ? 1 : 0));
    g.setDword(kModemState, state);
    if (g.dword(kModemState) == 0)
    {
        g.setDword(kModemState, 2);
        const u32 number = g.dword(g.dword(kModemSettings) + 0x68);
        const u32 up = number == 0 || guest::direct<0x4fa030, tapiMakeCall>(app, ts, number) != 0 ? 1 : 0;
        const u32 edx = h.edx, ecx = h.ecx, ebx = h.ebx;
        ts.sp -= 0xc;  // they are pushed from here on
        g.setDword(g.dword(kModemSettings) + 0x34, up);
        g.setDword(kThreadStarting, 0);
        h.ebx = 0;
        while (g.dword(g.dword(kModemSettings) + 0x34) != 0)
        {
            const os::window::Event event = os::window::wait(app, ts);
            if (ts.terminate)
                throw fe::Terminated();
            if (event.kind == os::window::Event::Kind::stop)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
            }
            else
            {
                frontend::wina::dispatchEvent(app, ts, event);
            }
        }
        guest::direct<0x4fa1d0, tapiHangUp>(app, ts);
        ts.sp += 0xc;
        h.edx = edx, h.ecx = ecx, h.ebx = ebx;
    }
    return fe::direct<0x4f97c0, frontend::eab::lineShutdown>(app, ts);
}

nfs::u32 frontend::tapiThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa210", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tapiThread(app, ts); });
}

/* sub_4fa310: a modem connection opened: the thread started and waited for, then the object's
 * function (+0x14) until it says stop or the thread has done; the port opened on the handle. */
u32 frontend::netb::modemOpen(App* app, ThreadState& ts, u32 settings, u32 callback)
{
    Held h;
    h.eax = settings; h.edx = callback;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    g.setDword(kModemSettings, settings);
    g.setDword(kModemCallback, callback);
    g.setDword(kModemState, 0);
    g.setDword(settings + 0x38, 0xffffffff);
    g.setDword(settings + 0x34, 0);
    g.setDword(kThreadStarting, 1);
    const auto oneSecond = [&] {
        const s32 ticks = s32(g.dword(kTicksPerSecond));
        h.edx = u32(s32(0x3e8) % ticks);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, u32(s32(0x3e8) / ticks));
    };
    if (callHeld(g, h, kCreateThread, args({ 0x4fa210, 0, 1, 0xffffffff }, { 0x9f4d60 })) == 0)
    {
        g.setDword(kModemState, 1);
    }
    else
    {
        while (g.dword(kThreadStarting) != 0)
            oneSecond();
        while (g.dword(kThreadStarting) == 0)
        {
            if (callHeld(g, h, g.dword(settings + 0x14), args({ settings })) != 0)
                break;
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
            {
                if (s32(u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
            }
            oneSecond();
        }
        if (g.dword(kThreadStarting) == 0 || g.dword(kModemState) != kStateConnected)
            postQuit(g, ts, settings);
    }
    if (g.dword(settings + 0x34) != 0 && g.dword(kModemState) == kStateConnected && g.dword(kModemHandle) != 0)
        g.setDword(settings + 0x38, fe::direct<0x51b2f0, frontend::netc::serialOpenHandle>(app, ts, 0xffffffff,
                                                                                            g.dword(kModemHandle)));
    return g.dword(settings + 0x34);
}

nfs::u32 frontend::modemOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa310", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 settings = args.eax, callback = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemOpen(app, ts, settings, callback); });
}

/* sub_4fa450: a modem connection closed: the thread told. */
u32 frontend::netb::modemClose(App* app, ThreadState& ts, u32 settings)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 8);
    postQuit(g, ts, settings);
    return 1;
}

nfs::u32 frontend::modemClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa450", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const u32 settings = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemClose(app, ts, settings); });
}

/* sub_4fa480: a modem connection polled: told to close unless connected; the callback with
 * what has come; whether it is still up. */
u32 frontend::netb::modemPoll(App* app, ThreadState& ts, u32 object)
{
    Held h;
    h.eax = object;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    h.edx = object;
    h.ebx = object;
    if (g.dword(kModemState) != kStateConnected)
    {
        const u32 ecx = h.ecx;
        postQuit(g, ts, object);
        h.ecx = ecx;
    }
    else if (g.dword(kModemCallback) != 0 && guest::direct<0x4fa4f0, modemReceiveReady>(app, ts, object) != 0)
    {
        callHeld(g, h, g.dword(kModemCallback), args({ object }));
    }
    return g.dword(object + 0x34);
}

nfs::u32 frontend::modemPoll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa480", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx | kRestoreEdi };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemPoll(app, ts, object); });
}

/* sub_4fa4e0: whether the modem's port will take more. */
u32 frontend::netb::modemSendReady(App* app, ThreadState& ts, u32 object)
{
    Guest g(app, ts);
    return fe::direct<0x51b900, frontend::netc::serialOutRoom>(app, ts, g.dword(object + 0x38));
}

nfs::u32 frontend::modemSendReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa4e0", 0, 0, nullptr, 0, 0 };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemSendReady(app, ts, object); });
}

/* sub_4fa4f0: whether the modem's port has something. */
u32 frontend::netb::modemReceiveReady(App* app, ThreadState& ts, u32 object)
{
    Guest g(app, ts);
    return fe::direct<0x51b8a0, frontend::netc::serialInCount>(app, ts, g.dword(object + 0x38));
}

nfs::u32 frontend::modemReceiveReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa4f0", 0, 0, nullptr, 0, 0 };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemReceiveReady(app, ts, object); });
}

/* sub_4fa500: bytes sent over the modem. */
u32 frontend::netb::modemSend(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    return portSend(g, ts, object, length, buffer);
}

nfs::u32 frontend::modemSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa500", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 object = args.eax, length = args.edx, buffer = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemSend(app, ts, object, length, buffer); });
}

/* sub_4fa530: bytes received over the modem. */
u32 frontend::netb::modemReceive(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    return portReceive(g, ts, object, length, buffer);
}

nfs::u32 frontend::modemReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa530", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 object = args.eax, length = args.edx, buffer = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemReceive(app, ts, object, length, buffer); });
}

/* sub_4fa550: a network over the modem: sub_4f5130 with this kind's table at 0x5671b0. */
u32 frontend::netb::modemOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 second, u32 mode)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    const u32 ticks = g.dword(kTicksPerSecond);
    fillTimeouts(g, settings, ticks, 4, 6);
    g.setDword(kTicksPerSecond, ticks);
    return guest::direct<0x4f5130, neta::serviceCall>(app, ts, settings,
                                                      mode == 0xffffffff ? u32(0x56ac10) : u32(0x56ac28), 0x5671b0,
                                                      second, mode);
}

nfs::u32 frontend::modemOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa550", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 settings = args.eax, second = args.edx, mode = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemOpenNetwork(app, ts, settings, second, mode); });
}

/* sub_4fa5f0: a modem's name by its index, 0 for none. */
u32 frontend::netb::modemName(App* app, ThreadState& ts, u32 index)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    if (g.dword(kModems) == 0)
        guest::direct<0x4f9ac0, tapiEnumerateModems>(app, ts);
    if (s32(index) < 0 || index >= g.dword(kModemCount))
        return 0;
    return g.dword(g.dword(kModems) + index * 8 + 4);
}

nfs::u32 frontend::modemName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa5f0", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::modemName(app, ts, index); });
}

/* sub_4fa620: a connection over a serial port opened, with the speed and so on in its settings. */
u32 frontend::netb::serialOpen(App* app, ThreadState& ts, u32 settings, u32 callback)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x10);
    constexpr u32 kPortOffset = 0x38, kSerialTrace = 0x566dec, kSerialFlag = 0x564458;
    const auto port = [&] { return g.dword(settings + kPortOffset); };
    g.setDword(kSerialCallback, callback);
    g.setDword(settings + 0x34, 0);
    g.setDword(kSerialFlag, 0);
    fe::direct<0x51b480, frontend::netc::serialOpen>(app, ts, port());
    if (fe::direct<0x51b030, frontend::netc::serialIsOpen>(app, ts, port()) == 0)
    {
        if (s32(g.dword(kSerialTrace)) >= 1)
            guest::direct<0x512180, commEvent>(app, ts, 0x54dbd8, settings + kPortOffset, 0, 0, 1, 0);
        return g.dword(settings + 0x34);
    }
    fe::direct<0x51b710, frontend::netc::serialSet>(app, ts, port(), g.dword(settings + 0x40), g.dword(settings + 0x44),
                                                    g.dword(settings + 0x48), g.dword(settings + 0x4c));
    fe::direct<0x51b1c0, frontend::netc::serialDropInput>(app, ts, port());
    fe::direct<0x51b1e0, frontend::netc::serialDropOutput>(app, ts, port());
    if (fe::direct<0x51a930, frontend::netc::modemAttention>(app, ts, port()) != 0)
    {
        fe::direct<0x51b200, frontend::netc::serialClose>(app, ts, port());
        return 0;
    }
    g.setDword(settings + 0x34, 1);
    return g.dword(settings + 0x34);
}

nfs::u32 frontend::serialOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa620", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 settings = args.eax, callback = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialOpen(app, ts, settings, callback); });
}

/* sub_4fa6d0: a serial connection closed: nothing but that it is. */
u32 frontend::netb::serialClose(App* app, ThreadState& ts, u32 object)
{
    Guest g(app, ts);
    g.setDword(object + 0x34, 0);
    return 1;
}

nfs::u32 frontend::serialClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa6d0", 0, 0, nullptr, 0 };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialClose(app, ts, object); });
}

/* sub_4fa6e0: a serial connection polled: the callback with what has come; whether it is up. */
u32 frontend::netb::serialPoll(App* app, ThreadState& ts, u32 object)
{
    Held h;
    h.eax = object;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    h.edx = object;
    h.ecx = object;
    h.ebx = g.dword(kSerialCallback);
    if (g.dword(kSerialCallback) != 0 && guest::direct<0x4fa720, serialReceiveReady>(app, ts, object) != 0)
        callHeld(g, h, g.dword(kSerialCallback), args({ object }));
    return g.dword(object + 0x34);
}

nfs::u32 frontend::serialPoll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa6e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialPoll(app, ts, object); });
}

/* sub_4fa710: whether the serial port will take more. */
u32 frontend::netb::serialSendReady(App* app, ThreadState& ts, u32 object)
{
    Guest g(app, ts);
    return fe::direct<0x51b900, frontend::netc::serialOutRoom>(app, ts, g.dword(object + 0x38));
}

nfs::u32 frontend::serialSendReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa710", 0, 0, nullptr, 0, 0 };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialSendReady(app, ts, object); });
}

/* sub_4fa720: whether the serial port has something. */
u32 frontend::netb::serialReceiveReady(App* app, ThreadState& ts, u32 object)
{
    Guest g(app, ts);
    return fe::direct<0x51b8a0, frontend::netc::serialInCount>(app, ts, g.dword(object + 0x38));
}

nfs::u32 frontend::serialReceiveReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa720", 0, 0, nullptr, 0, 0 };
    const u32 object = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialReceiveReady(app, ts, object); });
}

/* sub_4fa730: bytes sent over the serial port. */
u32 frontend::netb::serialSend(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    return portSend(g, ts, object, length, buffer);
}

nfs::u32 frontend::serialSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa730", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 object = args.eax, length = args.edx, buffer = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialSend(app, ts, object, length, buffer); });
}

/* sub_4fa760: bytes received over the serial port. */
u32 frontend::netb::serialReceive(App* app, ThreadState& ts, u32 object, u32 length, u32 buffer)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    return portReceive(g, ts, object, length, buffer);
}

nfs::u32 frontend::serialReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa760", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 object = args.eax, length = args.edx, buffer = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialReceive(app, ts, object, length, buffer); });
}

/* sub_4fa780: a network over a serial port: sub_4f5130 with this kind's table at 0x5671d0. */
u32 frontend::netb::serialOpenNetwork(App* app, ThreadState& ts, u32 settings, u32 second, u32 mode)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    const u32 ticks = g.dword(kTicksPerSecond);
    fillTimeouts(g, settings, ticks, 10, 3);
    g.setDword(kTicksPerSecond, ticks);
    return guest::direct<0x4f5130, neta::serviceCall>(app, ts, settings,
                                                      mode == 0xffffffff ? u32(0x56ac10) : u32(0x56ac28), 0x5671d0,
                                                      second, mode);
}

nfs::u32 frontend::serialOpenNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fa780", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 settings = args.eax, second = args.edx, mode = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::serialOpenNetwork(app, ts, settings, second, mode); });
}

/* ---------------------------------------------------------------------------
 * win\ipx: games looked for, advertised by a thread over a TCP/IP socket of IPX's family
 * ------------------------------------------------------------------------- */
namespace
{

constexpr u32 kAdvertRunning = 0xa0b0b8, kAdvertDone = 0xa0b0b4, kAdvertSocket = 0xa0b0bc, kAdvertPacket = 0xa0b0d2;
constexpr u32 kIpxSocket = 0x452, kGameProtocol = 0x8be;

}  // namespace

/* sub_5127d0: a broadcast asking for games sent on a socket: IPX's address, all ones. */
u32 frontend::netb::ipxBroadcastOpen(App* app, ThreadState& ts, u32 socket)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x24);
    const u32 packet = g.frame() + 0x10;
    g.setWord(packet, std::uint16_t(swapped(1) >> 16));
    g.setWord(packet + 2, std::uint16_t(swapped(kGameProtocol) >> 16));
    fe::enterLock(g, ts, g.dword(kNetLock));
    const os::net::Result sent = os::net::sendTo(app, ts, socket, guest::memory(app, packet), 4,
                                                 os::net::Endpoint::ipxEveryone(std::uint16_t(kIpxSocket)));
    fe::leaveLock(g, ts, g.dword(kNetLock));
    return sent.ok() ? 1 : 0;
}

nfs::u32 frontend::ipxBroadcastOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5127d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 socket = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxBroadcastOpen(app, ts, socket); });
}

/* sub_512890: a list of games read from a packet: 64 bytes each after two of a version; those of
 * this game handed to sub_4f5a30.  1 when the packet was such a list. */
u32 frontend::netb::ipxScanInterfaces(App* app, ThreadState& ts, u32 owner, u32 header, u32 data, u32 size)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x14);
    constexpr u32 kEntry = 0x40;
    // The sender's IPX socket, in network order in its address.
    if (os::net::networkOrder(std::uint16_t(kIpxSocket)) != g.word(header + 0xc))
        return 0;
    const u32 version = bigEndian(g, ts, data, 2);
    if (version != 2)
        return 0;
    u32 left = (size - version) >> 6;
    u32 entry = data + version;
    for (;;)
    {
        --left;
        if (left == 0xffffffff)
            return 1;
        if (bigEndian(g, ts, entry, 2) == kGameProtocol)
            guest::direct<0x4f5a30, neta::ipxAnswerName>(app, ts, owner, entry + 0x32, entry + 2);
        entry += kEntry;
    }
}

nfs::u32 frontend::ipxScanInterfaces(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512890", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 owner = args.eax, header = args.edx, data = args.ebx, size = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxScanInterfaces(app, ts, owner, header, data, size); });
}

/* sub_512940: the game's advertisement sent to an address. */
u32 frontend::netb::ipxAdvertiseTo(App* app, ThreadState& ts, u32 to)
{
    Held h;
    h.eax = to;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    fe::enterLock(g, ts, g.dword(kNetLock));
    os::net::sendTo(app, ts, g.dword(kAdvertSocket), guest::memory(app, kAdvertPacket), 0x42, os::net::ipxAt(app, to));
    h.eax = fe::leaveLock(g, ts, g.dword(kNetLock));
    return h.eax;
}

nfs::u32 frontend::ipxAdvertiseTo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512940", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 to = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxAdvertiseTo(app, ts, to); });
}

/* sub_512980: whether a packet asks for games: type 1, this game's protocol or any. */
u32 frontend::netb::ipxIsRequest(App* app, ThreadState& ts, u32 data)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 8);
    h.edx = bigEndian(g, ts, data, 2);
    const u32 protocol = bigEndian(g, ts, data + 2, 2);
    return h.edx == 1 && (protocol == kGameProtocol || protocol == 0xffff) ? 1 : 0;
}

nfs::u32 frontend::ipxIsRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512980", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const u32 data = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxIsRequest(app, ts, data); });
}

/* sub_5129e0: an IPX socket for the advertising thread, broadcast and larger buffers on, bound
 * at the game's port; -1 for none. */
u32 frontend::netb::ipxServerSocket(App* app, ThreadState& ts, u32 address)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1c);
    const std::optional<os::Handle> socket = os::net::open(app, os::net::Kind::ipx);
    if (!socket)
        return os::net::kNoSocket;
    os::net::setBroadcast(app, *socket, true);
    // IPX's own receiving of broadcasts the code asks on: the port's IPX has no such switch.
    if (os::net::bind(app, *socket, os::net::Endpoint::ipx(0, std::uint16_t(kIpxSocket))) == Error::none)
    {
        // Its own address, as the system gave it, to `address`.
        if (const auto own = os::net::localName(app, *socket))
        {
            os::net::putIpx(app, address, *own);
            return *socket;
        }
    }
    os::net::close(app, ts, *socket);
    return os::net::kNoSocket;
}

nfs::u32 frontend::ipxServerSocket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5129e0", 0, 0, nullptr, 0 };
    const u32 address = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxServerSocket(app, ts, address); });
}

/* sub_512ad0: the advertising thread: it answers requests that come, and sends the game to all
 * every minute; one last time, with the game gone, if it ever did. */
u32 frontend::netb::ipxServerThread(App* app, ThreadState& ts)
{
    Held h;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x348);
    const u32 frame = g.frame(), data = frame, from = frame + 0x304;
    const u32 everyone = frame + 0x314;
    // The thread ends with eax as its last call leaves it: each call's result made eax.
    const auto ticks = [&] { return h.eax = fe::direct<0x4f2790, frontend::eab::lowTimerTicks>(app, ts); };
    const auto advertise = [&](u32 to) { h.eax = guest::direct<0x512940, ipxAdvertiseTo>(app, ts, to); };
    u32 next = ticks() + 4;
    g.setWord(everyone, 6);
    const u32 port = os::net::networkOrder(std::uint16_t(kIpxSocket));
    h.eax = port;
    guest::direct<0x4f5900, neta::portToSockaddr>(app, ts, u32(s32(std::int16_t(port))), everyone);
    g.setDword(kAdvertRunning, 1);
    u32 sentAny = 0;
    do
    {
        os::net::SocketSet set{};
        set.count = 1;
        set.sockets[0] = g.dword(kAdvertSocket);
        // The wait the code asks for is the timer's count `next` taken as milliseconds; none
        // once that is not above 0.
        const os::net::Result ready = os::net::select(app, ts, &set, nullptr, nullptr, u32(s32(next) > 0 ? next : 0));
        h.eax = u32(ready.value);
        if (!ready.ok())
        {
            g.setDword(kAdvertRunning, 0);
            break;
        }
        if (ready.value != 0)
        {
            fe::enterLock(g, ts, g.dword(kNetLock));
            os::net::Endpoint sender;
            const os::net::Result got = os::net::receiveFrom(app, ts, g.dword(kAdvertSocket), guest::memory(app, data),
                                                             0x200, sender);
            if (got.ok())
                os::net::putIpx(app, from, sender);
            h.eax = fe::leaveLock(g, ts, g.dword(kNetLock));
            // 4 in edx too, which it does not read.
            if (got.value == 4 && (h.eax = guest::direct<0x512980, ipxIsRequest>(app, ts, data)) != 0)
                advertise(from);
        }
        if (g.dword(kAdvertRunning) != 0 && next < ticks())
        {
            sentAny = 1;
            advertise(everyone);
            next = ticks() + 0xea60;
        }
    } while (g.dword(kAdvertRunning) != 0);
    if (sentAny != 0)
    {
        g.setWord(0xa0b112, std::uint16_t(swapped(0x10) >> 16));
        advertise(everyone);
    }
    g.setDword(kAdvertDone, 1);
    return h.eax;
}

nfs::u32 frontend::ipxServerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512ad0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxServerThread(app, ts); });
}

/* sub_512c80: the game advertised: the socket, the packet of its name and id, and the thread,
 * waited for. */
u32 frontend::netb::ipxStartServer(App* app, ThreadState& ts, u32 owner, u32 id, u32 name)
{
    Held h;
    h.eax = owner; h.edx = id; h.ebx = name;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    h.edx = name;
    if (g.dword(kAdvertRunning) != 0)
        return g.dword(kAdvertRunning);
    const u32 socket = guest::direct<0x5129e0, ipxServerSocket>(app, ts, 0xa0b0c4);
    g.setDword(kAdvertSocket, socket);
    if (socket == 0xffffffff)
        return g.dword(kAdvertRunning);
    g.setDword(0xa0b0c0, owner);
    g.setWord(kAdvertPacket, std::uint16_t(swapped(2) >> 16));
    g.setWord(0xa0b0d4, std::uint16_t(swapped(kGameProtocol) >> 16));
    g.setWord(0xa0b112, std::uint16_t(swapped(1) >> 16));
    fe::direct<0x4e0e30, watcom::clib::strncpy>(app, ts, 0xa0b0d6u, name, 0x30u);
    h.ecx = (0xffffffff);  // as the code loads it for it, which it keeps: the thread's maker reads it
    copyDwords(g, 0xa0b106, id, 3);
    g.setDword(kAdvertDone, 0);
    if (callHeld(g, h, kCreateThread, args({ 0x512ad0, 0, 1 }, { 0xa0b0a0 })) != 0)
    {
        h.edx = 0;
        while (g.dword(kAdvertRunning) == 0 && g.dword(kAdvertDone) == 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
    }
    if (g.dword(kAdvertDone) != 0)
        os::net::close(app, ts, g.dword(kAdvertSocket));
    return g.dword(kAdvertRunning);
}

nfs::u32 frontend::ipxStartServer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512c80", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 owner = args.eax, id = args.edx, name = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxStartServer(app, ts, owner, id, name); });
}

/* sub_512d70: the advertising stopped: the socket closed and the thread waited for. */
u32 frontend::netb::ipxStopServer(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0xc);
    if (g.dword(kAdvertRunning) != 0)
    {
        const u32 socket = g.dword(kAdvertSocket);
        g.setDword(kAdvertRunning, 0);
        os::net::close(app, ts, socket);
        while (g.dword(kAdvertDone) == 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
    }
    return 1;
}

nfs::u32 frontend::ipxStopServer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512d70", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::ipxStopServer(app, ts); });
}

/* ---------------------------------------------------------------------------
 * Sessions
 * ------------------------------------------------------------------------- */

/* sub_512f60: a game's GUID as the text that names its shared memory. */
u32 frontend::netb::gameGuidText(App* app, ThreadState& ts, u32 guid, u32 text)
{
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 4);
    return fe::direct<0x4df690, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x55022c, guest::Words{g.dword(guid), g.word(guid + 4), g.word(guid + 6), g.byte(guid + 8), g.byte(guid + 9), g.byte(guid + 0xa), g.byte(guid + 0xb), g.byte(guid + 0xc), g.byte(guid + 0xd), g.byte(guid + 0xe), g.byte(guid + 0xf)});
}

nfs::u32 frontend::gameGuidText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512f60", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 guid = args.eax, text = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::gameGuidText(app, ts, guid, text); });
}

/* sub_512fc0: when an answer stops being waited for: eax ticks from now. */
u32 frontend::netb::setResponseTimeout(App* app, ThreadState& ts, u32 ticks)
{
    Guest g(app, ts);
    const u32 until = ticks + g.dword(0x79c1a0);
    g.setDword(0x5689e0, until);
    return until;
}

nfs::u32 frontend::setResponseTimeout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512fc0", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 ticks = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::setResponseTimeout(app, ts, ticks); });
}

/* sub_512fd0: whether that time has come. */
u32 frontend::netb::responseTimedOut(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return s32(g.dword(0x79c1a0) - g.dword(0x5689e0)) < 0 ? 0 : 1;
}

nfs::u32 frontend::responseTimedOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512fd0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::responseTimedOut(app, ts); });
}

/* sub_512ff0: what a table holds for a function that always says no. */
u32 frontend::netb::returnZeroToo(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::netReturnZeroToo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_512ff0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::returnZeroToo(app, ts); });
}

/* sub_513190: a request sent to the session's server and waited for, sent again each time round:
 * eax the game, edx the session, ebx the type, ecx the message, its first dword the new id; the
 * stack its length and how long to wait. */
u32 frontend::netb::sendServerRequest(App* app, ThreadState& ts, u32 game, u32 session, u32 type, u32 message, u32 length, u32 patience)
{
    Held h;
    h.eax = game; h.edx = session; h.ebx = type; h.ecx = message;
    Guest g(app, ts);
    const fe::Frame locals(g, ts, 0x1c);
    constexpr u32 kNextId = 0x5689dc, kPatience = 0x566ddc;
    const u32 frame = g.frame();
    g.setDword(frame, type);
    g.setDword(session + 4, 0);
    g.setDword(session + 0x10, game);
    const u32 id = g.dword(kNextId);
    g.setDword(session, id);
    g.setDword(kNextId, id + 1);
    g.setDword(message, swapped(id));
    h.edx = g.dword(kTraceLevel);
    if (s32(h.edx) >= 3)
        logEventLeaving(g, h, 0x550270, frame, message, length);
    const u32 functions = g.dword(game + 8);
    if (g.dword(functions + 0x48) != 0)
        callHeld(g, h, g.dword(functions + 0x48), args({ g.dword(game + 0xc), game + 0x1c }));
    g.setDword(game + 0x18, session);
    guest::direct<0x512fc0, setResponseTimeout>(app, ts, patience);
    h.eax = session;
    if (g.dword(session + 4) == 0)
    {
        u32 delay = 1;
        for (;;)
        {
            const u32 timedOut = guest::direct<0x512fd0, responseTimedOut>(app, ts);
            if (timedOut != 0)
            {
                h.eax = timedOut;  // the code returns its eax
                break;
            }
            const u32 sendTo = g.dword(game + 8);
            if (callHeld(g, h, g.dword(sendTo + kNetSend), args({ g.dword(game + 0xc), game + 0x1c, type, message }, { length })) != 0)
                delay = g.dword(kPatience);
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
            {
                if (s32(callHeld(g, h, kWaitSystemTasks, args({ delay }))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, delay);
            }
            h.eax = session;
            if (g.dword(session + 4) != 0)
                break;
        }
    }
    g.setDword(game + 0x18, 0);
    return h.eax;
}

nfs::u32 frontend::sendServerRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_513190", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 game = args.eax;
    const u32 session = args.edx;
    const u32 type = args.ebx;
    const u32 message = args.ecx;
    const u32 length = app->getMemory<u32>(ts.sp + 4);
    const u32 patience = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::sendServerRequest(app, ts, game, session, type, message, length, patience); });
}

/* sub_4f6860: nothing to do for an exception on a socket; it counts as handled. */
u32 frontend::netb::tcpExceptionSeen(App* app, ThreadState& ts)
{
    return 1;
}

nfs::u32 frontend::tcpExceptionSeen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f6860", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netb::tcpExceptionSeen(app, ts); });
}

}
