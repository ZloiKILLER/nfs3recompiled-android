#include "native_frontend_netb.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_neta.h"
#include "native_frontend_netc.h"
#include "native_frontend_wata.h"
#include "native_frontend_watc.h"
#include "native_windows.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include <winapi/kernel32.h>
#include <winapi/user32.h>
#include <winapi/wsock32.h>
#include <cstdint>
#include <initializer_list>
#include <optional>

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
 * Windows' functions, Winsock's through the import table's thunks, are the port's own, called
 * as C++ (in the checks through the thunks and the import table's slots).  What each function
 * leaves in eax and in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::netb (native_frontend_netb.h), in a frame of
 * its own where the game's function has one (fe::Frame), and an adapter under the game's name for
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
using fe::runWhole;
using fe::Site;
using x86::reg32;
using x86::sreg32;

constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588;
constexpr reg32 kTraceLevel = 0x566de0;   // how much sub_512180 is asked to log
constexpr reg32 kError = 0x401010;
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kMemoryGroup = 0x5643f4;  // what the library's allocations are tagged with
constexpr reg32 kNetLock = 0x564480;      // the sockets' critical section
constexpr reg32 kNow = 0x79c1a4;
constexpr reg32 kTicksPerSecond = 0x5643d8;
constexpr reg32 kReserve = 0x4e1620;
constexpr reg32 kMemmove = 0x4ea4f0, kStrncpy = 0x4e0e30, kStrncmp = 0x4eb390, kMemset = 0x4e0640;
constexpr reg32 kCreateThread = 0x4df7a0, kWaitSystemTasks = 0x4e7720;
constexpr reg32 kCheckPlayer = 0x4f5040;  // NET_: whether a player's address is this one's

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its import's slot.  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* The same for one the code calls through its import thunk (Winsock's): in the check by the
 * thunk's address. */
template <reg32 Thunk, auto Function, typename... Words>
reg32 thunked(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::thunked<Thunk, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

void enterLock(Guest& g, x86::CPU& cpu, reg32 lock) { windows<kEnterLock, &win32::kernel32::EnterCriticalSection>(g, cpu, lock); }
void leaveLock(Guest& g, x86::CPU& cpu, reg32 lock) { windows<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(g, cpu, lock); }

reg32 swapped(reg32 v)
{
    return (v << 24) | ((v & 0xff00) << 8) | ((v >> 8) & 0xff00) | (v >> 24);
}

void copyDwords(Guest& g, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        g.setDword(to + 4 * i, g.dword(from + 4 * i));
}

/* A number of `bytes` bytes in network order at `at`, read as the library's macro reads it:
 * a whole dword swapped and shifted down, eax the value and ecx the count of the shift. */
reg32 bigEndian(Guest& g, x86::CPU& cpu, reg32 at, reg32 bytes)
{
    const reg32 shift = 0x20 - bytes * 8;
    cpu.ecx = shift;
    cpu.eax = swapped(g.dword(at)) >> (shift & 31);
    return cpu.eax;
}

/* A line of the log: its text, the address it is about, the data and how many bytes of it; eax as
 * the log's function leaves it, for the callers that end with what their calls left there. */
void logEvent(Guest& g, x86::CPU& cpu, reg32 text, reg32 what, reg32 data, reg32 bytes, reg32 stackWord = 0)
{
    cpu.eax = guest::direct<0x512180, frontend::netb::commEvent>(g.app(), cpu, text, what, data, bytes, stackWord,
                                                               stackWord);
}

/* The same line by address, where the call after it takes the registers the log's function leaves
 * (a function through a pointer handed what it left in ebx and ecx). */
void logEventLeaving(Guest& g, reg32 text, reg32 what, reg32 data, reg32 bytes, reg32 stackWord = 0)
{
    g.call(0x512180, args({ text, what, data, bytes }, { stackWord, stackWord }));
}

/* The source position the library leaves for sub_401010 before a report. */
void setErrorAt(Guest& g, reg32 file, reg32 function, reg32 line)
{
    g.setDword(kErrorFile, file);
    g.setDword(kErrorFunction, function);
    g.setDword(kErrorLine, line);
}

/* The session messages' four-letter types, sub_5133a0. */
constexpr reg32 kMsgAdd = 0xea616464, kMsgBad = 0xea626164, kMsgBye = 0xea627965, kMsgDat = 0xea646174;
constexpr reg32 kMsgDec = 0xea646563, kMsgGet = 0xea676574, kMsgNo = 0xea6e6f21, kMsgOla = 0xea6f6c61;
constexpr reg32 kMsgOpn = 0xea6f706e, kMsgOut = 0xea6f7574, kMsgPly = 0xea706c79, kMsgReq = 0xea726571;
constexpr reg32 kMsgRsp = 0xea727370, kMsgSrv = 0xea737276;

constexpr reg32 kPulseTime = 0x566dd4, kAliveTime = 0x566dd8;
constexpr reg32 kOwnPlayer = 0x1c;        // in a NET object: this player's address
constexpr reg32 kNetSend = 0x54;          // in a NET object's functions: send to an address
constexpr reg32 kPlayerRecord = 0x64;     // a player in a game: 100 bytes
constexpr reg32 kPlayers = 0x88;          // in a game
constexpr reg32 kPlayerMask = 0x50;       // in a game: a bit for each player present

constexpr reg32 kSendGameInfo = 0x513030, kFindPlayer = 0x5132a0;

}  // namespace

/* The port's own line in logcat for what sub_512180 logs (native_vertices.cpp). */
bool commEventTrace(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

/* The port's line for one of the log's (commEventTrace) reads the call's registers and its two stack
 * words: they are put where the game's call puts them, whichever way sub_512180's core was called. */
void traceEvent(win32::WinApplication* app, x86::CPU& cpu, reg32 text, reg32 what, reg32 data, reg32 bytes,
                reg32 flagged, reg32 kind)
{
    const guest::KeptRegisters kept(cpu);
    cpu.eax = text;
    cpu.edx = what;
    cpu.ebx = data;
    cpu.ecx = bytes;
    cpu.esp -= 12;
    app->getMemory<reg32>(cpu.esp + 4) = flagged;
    app->getMemory<reg32>(cpu.esp + 8) = kind;
    commEventTrace(app, cpu);
}

}  // namespace

/* sub_512180: a line in the network's log, a ring of 64-byte records at [0x5689d0]: the time,
 * the first dword of an address, how many bytes of data, the second stack word, flags (1 a
 * text, 2 an address, 4 data, 8 the first stack word set) and data and text after them, the two
 * no more than 0x35 bytes.  Printed too when [0x5689d4] says so. */
reg32 frontend::netb::commEvent(App* app, CPU& cpu, reg32 text, reg32 what, reg32 data, reg32 bytes, reg32 flagged, reg32 kindWord)
{
    traceEvent(app, cpu, text, what, data, bytes, flagged, kindWord);
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1c);
    constexpr reg32 kLog = 0x5689d0, kLogLock = 0x5689cc, kPrinted = 0x5689d4, kThreadId = 0x5689d8;
    constexpr reg32 kSize = 0xa0b090, kHead = 0xa0b094, kUsed = 0xa0b098;
    const std::uint8_t kind = std::uint8_t(kindWord);
    if (g.dword(kLog) != 0)
    {
        enterLock(g, cpu, g.dword(kLogLock));
        const reg32 used = g.dword(kUsed), size = g.dword(kSize);
        const reg32 slot = reg32(sreg32(g.dword(kHead) + used) % sreg32(size));
        const reg32 record = g.dword(kLog) + (slot << 6);
        if (used != size)
            g.setDword(kUsed, used + 1);
        else
            g.setDword(kHead, reg32(sreg32(g.dword(kHead) + 1) % sreg32(size)));

        std::uint8_t textLength = 0;
        if (text != 0)
        {
            const reg32 length = g.length(text);
            textLength = std::uint8_t(length >= 0x35 ? 0x35 : length);
        }
        std::uint8_t dataLength = 0;
        if (data != 0)
        {
            const sreg32 room = sreg32(0x35 - reg32(textLength));
            dataLength = std::uint8_t(room > sreg32(bytes) ? bytes : reg32(room));
        }
        g.setByte(record + 8, dataLength);
        std::uint8_t flags = flagged != 0 ? 8 : 0;
        g.setByte(record + 0xa, flags);
        g.setDword(record, g.dword(kNow));
        g.setByte(record + 9, kind);
        reg32 to = record + 0xb;
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
            cpu.ebx = (what & 0xffff00ff) | (reg32(flags) << 8);
        }
        else
        {
            g.setDword(record + 4, 0);
            cpu.ebx = 0;
        }
        if (g.dword(kPrinted) != 0)
        {
            cpu.ebx = flagged;
            g.call(0x4e08f0, args({ g.byte(kThreadId) },
                                  { 0x55016c, g.dword(record), g.byte(kThreadId), kind,
                                    flagged != 0 ? reg32(0x550108) : reg32(0x550110),
                                    what != 0 ? g.dword(what) : 0, text != 0 ? text : reg32(0x550168), bytes }));
        }
        leaveLock(g, cpu, g.dword(kLogLock));
    }
    return g.dword(kLog) != 0 ? 1 : 0;
}

void frontend::commEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512180", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 text = cpu.eax;
    const reg32 what = cpu.edx;
    const reg32 data = cpu.ebx;
    const reg32 bytes = cpu.ecx;
    const reg32 flagged = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 kindWord = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::commEvent(app, cpu, text, what, data, bytes, flagged, kindWord); });
}

/* sub_513000: how many bits a word has set. */
reg32 frontend::netb::countBits(App* app, CPU& cpu, reg32 value)
{
    reg32 count = 0;
    while (value != 0)
    {
        value &= value - 1;
        ++count;
    }
    return count;
}

void frontend::netCountBits(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_513000", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const reg32 value = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::countBits(app, cpu, value); });
}

/* sub_513030: this game's description sent to an address: eax the game, edx the address, ebx
 * the id of the request.  With no request, a game that has started says nothing. */
reg32 frontend::netb::sendGameInfo(App* app, CPU& cpu, reg32 game, reg32 to, reg32 id)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc4);
    const reg32 net = g.dword(game + 4);
    if (id == 0 && g.dword(game + 8) != 0)
        return id;
    const reg32 frame = g.frame();
    const reg32 reply = frame + 8, typeAt = frame + 0xb4;
    constexpr reg32 kReplySize = 0xac;
    const auto shortHigh = [&](reg32 at) {
        return std::uint16_t(swapped(reg32(sreg32(g.dword(at)) >> 16)) >> 16);
    };
    g.setDword(typeAt, kMsgDec);
    g.setDword(reply, swapped(id));
    for (reg32 i = 0; i < 4; ++i)
        g.setByte(frame + 0x50 + i, g.byte(game + 0x74 + i));
    g.setWord(frame + 0x54, shortHigh(game + 0x76));
    g.setWord(frame + 0x56, shortHigh(game + 0x78));
    g.setDword(frame + 0x58, swapped(g.dword(game + 0x7c)));
    g.setDword(frame + 0x5c, swapped(g.dword(game + 0x80)));
    g.setDword(frame + 0x60, swapped(g.dword(game + 0x84)));
    g.setDword(frame + 0x74, swapped(g.dword(game + 0x50)));
    copyDwords(g, frame + 0xc, net + kOwnPlayer, 5);
    g.call(kStrncpy, args({ frame + 0x30, game + 0x54, 0x20 }));
    for (reg32 i = 0; i < 8; ++i)
        g.setByte(frame + 0x28 + i, g.byte(net + 0x94 + i));
    g.setDword(frame + 0x20, swapped(g.dword(net + 0x8c)));
    g.setWord(frame + 0x24, std::uint16_t(swapped(g.word(net + 0x90)) >> 16));
    g.setWord(frame + 0x26, std::uint16_t(swapped(g.word(net + 0x92)) >> 16));
    if (sreg32(g.dword(kTraceLevel)) >= 3)
        logEvent(g, cpu, 0x550264, typeAt, reply, kReplySize);
    return g.call(g.dword(g.dword(net + 8) + kNetSend), args({ g.dword(net + 0xc), to, kMsgDec, reply }, { kReplySize }));
}

void frontend::sendGameInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_513030", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 game = cpu.eax, to = cpu.edx, id = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::sendGameInfo(app, cpu, game, to, id); });
}

/* sub_5132a0: the player of a game at an address, his time last heard from set; 0 for none. */
reg32 frontend::netb::findPlayerByAddress(App* app, CPU& cpu, reg32 game, reg32 address)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    reg32 player = game + kPlayers;
    for (reg32 i = 0;; ++i, player += kPlayerRecord)
    {
        cpu.edx = i;
        if (g.dword(game + 0x84) - 1 <= i)
            return 0;
        if ((g.dword(game + kPlayerMask) & (reg32(1) << (i & 31))) != 0
            && g.call(kCheckPlayer, args({ g.dword(game + 4), address, player })) != 0)
        {
            g.setDword(game + i * kPlayerRecord + 0xe4, g.dword(kPulseTime));
            return player;
        }
    }
}

void frontend::findPlayerByAddress(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5132a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 game = cpu.eax, address = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::findPlayerByAddress(app, cpu, game, address); });
}

/* sub_513320: a player's record taken into a game's first empty place; his number, or 0 and a
 * printed complaint when there is none (the host has counted before this). */
reg32 frontend::netb::addNewPlayer(App* app, CPU& cpu, reg32 game, reg32 record)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    for (reg32 i = 0;; ++i)
    {
        cpu.edx = g.dword(game + 0x84) - 1;
        if (i >= cpu.edx)
        {
            g.call(0x4e08f0, args({ i }, { 0x55027c }));
            cpu.edx = 0;
            return 0;
        }
        const reg32 bit = reg32(1) << (i & 31);
        const reg32 mask = g.dword(game + kPlayerMask);
        if ((mask & bit) != 0)
            continue;
        g.setDword(game + kPlayerMask, mask | bit);
        const reg32 place = game + i * kPlayerRecord;
        copyDwords(g, place + kPlayers, record, 0x17);
        g.setDword(place + 0xe4, g.dword(kPulseTime));
        g.setDword(place + 0xe8, i);
        cpu.edx = i;
        return i;
    }
}

void frontend::addNewPlayer(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_513320", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 game = cpu.eax, record = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::addNewPlayer(app, cpu, game, record); });
}

/* sub_5133a0: a session message from an address.  eax the session waiting for answers, edx
 * the address, ebx the type, ecx the message, the stack its length.  [+0x10] is the NET
 * object, whose +0x14 is the game this machine hosts: a host answers GET, ADD, REQ and SRV, a
 * client hears RSP, PLY, BAD and DEC; OPN, OLA, DAT, OUT and BYE go with either. */
reg32 frontend::netb::sessionMessage(App* app, CPU& cpu, reg32 session, reg32 from, reg32 type, reg32 message, reg32 length)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xdc);
    const reg32 frame = g.frame();
    const reg32 net = g.dword(session + 0x10);
    const reg32 typeAt = frame + 0xac, replyType = frame + 0xb0;
    g.setDword(typeAt, type);
    g.setDword(frame + 0xcc, from);
    g.setDword(frame + 0xb8, message);

    // The reply is built at the frame's bottom: +0 the request's id, +4 an address,
    // +0x18 its verdict, +0x1c a player.
    const auto send = [&](reg32 size) {
        const reg32 functions = g.dword(net + 8);
        return g.call(g.dword(functions + kNetSend), args({ g.dword(net + 0xc), from, g.dword(replyType), frame }, { size }));
    };
    const auto trace = [&](reg32 level) { return sreg32(g.dword(kTraceLevel)) >= sreg32(level); };
    const auto logMessage = [&](reg32 text) { logEvent(g, cpu, text, typeAt, message, length); };
    const auto checkOwn = [&] { return g.call(kCheckPlayer, args({ net, from, net + kOwnPlayer })); };

    switch (type)
    {
    case kMsgNo:
        if (trace(3))
            logEvent(g, cpu, 0x550380, 0, from, 0x14);
        break;

    case kMsgOla:
    {
        const reg32 game = g.dword(net + 0x14);
        cpu.ecx = game;
        if (game != 0)
        {
            const reg32 player = g.call(kFindPlayer, args({ game }));
            if (player != 0 && trace(4))
                logEvent(g, cpu, 0x550388, player + 0x60, 0, 0);
            break;
        }
        if (g.dword(net + 0xa4) == 0 || g.dword(net + 0xa0) == 0)
            break;
        if (checkOwn() == 0)
            break;
        g.setDword(replyType, kMsgOla);
        g.setDword(net + 0xa4, g.dword(kPulseTime));
        g.call(g.dword(g.dword(net + 8) + kNetSend),
               args({ g.dword(net + 0xc), net + kOwnPlayer, kMsgOla, cpu.ecx }, { cpu.ecx }));
        if (trace(4))
            logEvent(g, cpu, 0x550398, typeAt, 0, 0);
        break;
    }

    case kMsgReq:
        if (net == 0 || g.dword(net + 0x14) == 0)
            break;
        {
            const reg32 wanted = bigEndian(g, cpu, message, 4);
            const reg32 game = g.dword(net + 0x14);
            g.call(kSendGameInfo, args({ game, from, wanted, game }));
        }
        break;

    case kMsgSrv:
    {
        if (net == 0)
            break;
        const reg32 game = g.dword(net + 0x14);
        if (game == 0)
            break;
        if (g.call(kStrncmp, args({ game + 0xc, message, 0x20 })) != 0)
            break;
        g.call(kSendGameInfo, args({ g.dword(net + 0x14), from, 0 }));
        break;
    }

    case kMsgOpn:
        if (trace(3))
            logEvent(g, cpu, 0x550378, 0, from, 0x14);
        if (g.dword(net + 0x10) == 0)
            break;
        {
            const reg32 refused = g.call(g.dword(net + 0x10), args({ net, from }));
            if (refused != 0)
                break;
            g.call(g.dword(g.dword(net + 8) + kNetSend), args({ g.dword(net + 0xc), from, kMsgNo, 0 }, { refused }));
        }
        break;

    case kMsgOut:
    case kMsgBye:
    {
        if (trace(2))
            logMessage(0x5503a8);
        const reg32 game = g.dword(net + 0x14);
        cpu.ecx = game;
        if (game == 0)
        {
            g.setDword(net + 0xa4, 0);
            g.setDword(net + 0xa0, 0);
            break;
        }
        const reg32 player = g.call(kFindPlayer, args({ game, from }));
        if (player == 0)
            break;
        const reg32 hosting = g.dword(net + 0x14);
        const sreg32 offset = sreg32(player - (hosting + kPlayers));
        const sreg32 index = offset / sreg32(kPlayerRecord);
        cpu.edx = reg32(offset % sreg32(kPlayerRecord));
        cpu.ecx = reg32(index) & 0xff;
        cpu.eax = ~(reg32(1) << (reg32(index) & 31));
        cpu.ebx = g.dword(hosting + kPlayerMask) & cpu.eax;
        g.setDword(hosting + kPlayerMask, cpu.ebx);
        g.setDword(player + 0x5c, 0);
        break;
    }

    case kMsgDat:
    {
        const reg32 game = g.dword(net + 0x14);
        if (game != 0)
        {
            g.call(kFindPlayer, args({ game }));
            break;
        }
        if (checkOwn() == 0 || g.dword(net + 0xa0) == 0)
            break;
        cpu.eax = g.dword(kAliveTime);
        g.setDword(net + 0xa0, cpu.eax);
        break;
    }

    case kMsgGet:
    {
        if (g.dword(net + 0x14) == 0)
            break;
        // Answered with the player asked for, or the host himself for -1.
        g.setDword(replyType, kMsgPly);
        copyDwords(g, frame + 4, message + 4, 5);
        reg32 size = 0x78;
        g.setDword(frame, g.dword(message));
        g.setDword(frame + 0x18, g.dword(message + 0x18));
        const reg32 which = bigEndian(g, cpu, message + 0x18, 4);
        if (which == 0xffffffff)
        {
            copyDwords(g, frame + 0x1c, net + 0x30, 0x17);
        }
        else if ((g.dword(g.dword(net + 0x14) + kPlayerMask) & (reg32(1) << (which & 31))) == 0)
        {
            size -= 0x5c;
            g.setDword(replyType, kMsgBad);
        }
        else
        {
            copyDwords(g, frame + 0x1c, g.dword(net + 0x14) + which * kPlayerRecord + kPlayers, 0x17);
        }
        if (trace(3))
            logEvent(g, cpu, 0x5502dc, replyType, frame, size);
        send(size);
        break;
    }

    case kMsgAdd:
    {
        const reg32 game = g.dword(net + 0x14);
        cpu.edx = game;
        if (game == 0)
            break;
        cpu.ecx = g.dword(game + 8);
        if (cpu.ecx != 0)
            break;
        const reg32 known = g.call(kFindPlayer, args({ game, from, kMsgRsp }));
        g.setDword(replyType, kMsgRsp);
        g.setDword(frame, g.dword(message));

        if (known != 0)
        {
            g.setDword(frame + 0x18, swapped(1));
            const sreg32 index = sreg32(known - (g.dword(net + 0x14) + kPlayers)) / sreg32(kPlayerRecord);
            g.setDword(frame + 0x1c, swapped(reg32(index)));
            if (trace(4))
                logEvent(g, cpu, 0x55033c, replyType, from, 0x14);
        }
        else
        {
            reg32 verdict = 0, text = 0;
            const reg32 hosting = g.dword(net + 0x14);
            const reg32 most = g.dword(hosting + 0x84) - 1;
            cpu.edx = most;
            if (guest::direct<0x513000, countBits>(app, cpu, g.dword(hosting + kPlayerMask)) >= most)
            {
                verdict = 0xfffffffe;
                text = 0x550330;
            }
            else
            {
                const reg32 password = g.dword(net + 0x14) + 0x3c;
                if (g.byte(password) != 0 && g.call(kStrncmp, args({ password, message + 0x18, 0x10 })) != 0)
                {
                    verdict = 0xffffffff;
                    text = 0x550324;
                }
                else
                {
                    // A new player: his record is what he sent, with the address he came from.
                    const reg32 record = message + 0x28;
                    const reg32 slot = guest::direct<0x513320, addNewPlayer>(app, cpu, g.dword(net + 0x14), record);
                    copyDwords(g, record, from, 5);
                    const reg32 now = g.dword(net + 0x14);
                    if (g.dword(now + 0x4c) != 0
                        && g.call(g.dword(now + 0x4c), args({ net, record, slot + 1, 0 })) == 0)
                    {
                        const reg32 hostingNow = g.dword(net + 0x14);
                        g.setDword(hostingNow + kPlayerMask, g.dword(hostingNow + kPlayerMask) & ~(reg32(1) << (slot & 31)));
                        verdict = 0xfffffffd;
                        text = 0x550318;
                    }
                    else
                    {
                        g.setDword(frame + 0x18, swapped(1));
                        g.setDword(frame + 0x1c, swapped(slot));
                        fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, frame + 4, message + 4, 0x14);
                        if (trace(3))
                            logEvent(g, cpu, 0x55030c, replyType, from, 0x14);
                    }
                }
            }
            if (text != 0)
            {
                g.setDword(frame + 0x18, swapped(verdict));
                if (trace(3))
                    logEvent(g, cpu, text, replyType, from, 0x14, text == 0x550318 ? 0 : cpu.ecx);
            }
        }
        const reg32 refused = send(0x20);
        if (refused == 0 && trace(3))
            logEvent(g, cpu, 0x550348, replyType, from, 0x14);
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
            cpu.eax = g.dword(kAliveTime);
            g.setDword(net + 0xa0, cpu.eax);
        }
        const reg32 id = bigEndian(g, cpu, message, 4);
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
            const reg32 last = bigEndian(g, cpu, message + 0x18, 4) == 0xffffffff ? 1 : 0;
            const reg32 number = bigEndian(g, cpu, message + 0x18, 4);
            const reg32 keep = g.call(g.dword(session + 0x18),
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
        if (bigEndian(g, cpu, message, 4) != g.dword(session))
        {
            if (trace(3))
                logMessage(0x550360);
            break;
        }
        if (trace(3))
            logMessage(0x550354);
        const reg32 verdict = bigEndian(g, cpu, message + 0x18, 4);
        g.setDword(session + 4, verdict);
        g.setDword(session, 0);
        if (verdict != 1)
            break;
        copyDwords(g, net + kOwnPlayer, message + 4, 5);
        g.setDword(net + 0xa4, g.dword(kPulseTime));
        cpu.eax = g.dword(kAliveTime);
        g.setDword(net + 0xa0, cpu.eax);
        break;
    }

    case kMsgDec:
    {
        // The host's description: wanted by a session looking for games (id 0), or the
        // one asked by id, from the host this machine is with.
        const reg32 id = bigEndian(g, cpu, message, 4);
        cpu.edx = id;
        bool wanted = id == 0 && g.dword(session) == 0 && g.dword(session + 0x14) != 0;
        if (!wanted && g.dword(session + 0x14) == 0)
        {
            cpu.ecx = g.dword(session);
            if (cpu.ecx != 0 && id == cpu.ecx && checkOwn() != 0)
                wanted = true;
        }
        cpu.eax = wanted ? 1 : 0;
        cpu.ebx = cpu.eax;
        if (!wanted)
            break;
        if (g.dword(net + 0xa0) != 0 && g.dword(session + 0x14) == 0)
        {
            cpu.eax = g.dword(kAliveTime);
            g.setDword(net + 0xa0, cpu.eax);
        }
        g.setByte(session + 0x3c, std::uint8_t(bigEndian(g, cpu, message + 0x48, 1)));
        g.setByte(session + 0x3d, std::uint8_t(bigEndian(g, cpu, message + 0x49, 1)));
        g.setByte(session + 0x3e, std::uint8_t(bigEndian(g, cpu, message + 0x4a, 1)));
        g.setByte(session + 0x3f, std::uint8_t(bigEndian(g, cpu, message + 0x4b, 1)));
        g.setWord(session + 0x40, std::uint16_t(bigEndian(g, cpu, message + 0x4c, 2)));
        g.setWord(session + 0x42, std::uint16_t(bigEndian(g, cpu, message + 0x4e, 2)));
        g.setDword(session + 0x44, bigEndian(g, cpu, message + 0x50, 4));
        g.setDword(session + 0x48, bigEndian(g, cpu, message + 0x54, 4));
        g.setDword(session + 0x4c, bigEndian(g, cpu, message + 0x58, 4));
        g.call(kStrncpy, args({ session + 0x1c, message + 0x28, 0x20 }));
        const reg32 mask = bigEndian(g, cpu, message + 0x6c, 4);
        g.setDword(session + 4, 1);
        cpu.ecx = g.dword(kTraceLevel);
        g.setDword(session + 8, mask);
        if (trace(3))
            logMessage(0x5502d0);
        if (g.dword(session + 0x14) == 0)
            break;
        cpu.ecx = from;
        const reg32 players = guest::direct<0x513000, countBits>(app, cpu, g.dword(message + 0x6c));
        const reg32 keep = g.call(g.dword(session + 0x14), args({ net, session + 0x1c, players + 1, from }));
        if (keep == 0)
            g.setDword(session + 0x14, 0);
        break;
    }

    default:
        if (trace(3))
            logMessage(0x5503b0);
        break;
    }
    return cpu.eax;
}

void frontend::sessionMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5133a0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 session = cpu.eax;
    const reg32 from = cpu.edx;
    const reg32 type = cpu.ebx;
    const reg32 message = cpu.ecx;
    const reg32 length = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::sessionMessage(app, cpu, session, from, type, message, length); });
}

/* ---------------------------------------------------------------------------
 * cmn\queue.c: the queues
 * ------------------------------------------------------------------------- */
namespace
{

constexpr reg32 kQueueFile = 0x54fbb8, kQueueUninitialized = 0x54fbcc;  // "cmn\queue.c", "%s - UNINITIALIZED QUEUE %08x USED.\n"
constexpr reg32 kDefaultPriority = 0x50dbf0;
constexpr reg32 kQCount = 0, kQFlags = 4, kQFirst = 8, kQLast = 0xc, kQPriority = 0x10, kQPriorityArg = 0x14, kQLock = 0x18;

/* A queue with no lock is reported, and the code goes on with it. */
void reportQueue(Guest& g, reg32 function, reg32 line, reg32 queue)
{
    setErrorAt(g, kQueueFile, function, line);
    g.call(kError, args({}, { kQueueUninitialized, function, queue }));
}

void setChanged(Guest& g, reg32 queue)
{
    g.setByte(queue + kQFlags, std::uint8_t(g.byte(queue + kQFlags) | 1));
}

reg32 queuePriority(Guest& g, reg32 queue, reg32 element)
{
    return g.call(g.dword(queue + kQPriority), args({ element, g.dword(queue + kQPriorityArg) }));
}

}  // namespace

/* sub_50dcd0: qinit: a queue with a lock of its own, eax it, edx its priority function or 0 for
 * the default, ebx the function's argument. */
reg32 frontend::netb::qinit(App* app, CPU& cpu, reg32 queue, reg32 priority, reg32 argument)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    cpu.ecx = queue;
    const reg32 lock = reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    g.setDword(queue + kQCount, 0);
    g.setDword(queue + kQFlags, 2);
    g.setDword(queue + kQFirst, 0);
    g.setDword(queue + kQLast, 0);
    g.setDword(queue + kQLock, lock);
    g.setDword(queue + kQPriority, priority != 0 ? priority : kDefaultPriority);
    cpu.edx = g.dword(queue + kQLock);
    g.setDword(queue + kQPriorityArg, argument);
    reg32 left = lock;  // eax: the lock, or what the report leaves
    if (cpu.edx == 0)
    {
        reportQueue(g, 0x54fbc4, 0x8b, queue);
        left = cpu.eax;
        cpu.ebx = kQueueFile;
    }
    return left;
}

void frontend::qinit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50dcd0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, priority = cpu.edx, argument = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qinit(app, cpu, queue, priority, argument); });
}

/* sub_50dd60: qinitpeer: a queue that shares the lock of the queue in ecx. */
reg32 frontend::netb::qinitpeer(App* app, CPU& cpu, reg32 queue, reg32 priority, reg32 argument, reg32 peer)
{
    Guest g(app, cpu);
    cpu.ecx = g.dword(peer + kQLock);
    g.setDword(queue + kQCount, 0);
    g.setDword(queue + kQFlags, 0);
    g.setDword(queue + kQFirst, 0);
    g.setDword(queue + kQLast, 0);
    g.setDword(queue + kQLock, cpu.ecx);
    g.setDword(queue + kQPriority, priority != 0 ? priority : kDefaultPriority);
    cpu.edx = g.dword(queue + kQLock);
    g.setDword(queue + kQPriorityArg, argument);
    reg32 left = queue;  // eax: the queue, or what the report leaves
    if (cpu.edx == 0)
    {
        reportQueue(g, 0x54fbf4, 0xc7, queue);
        left = cpu.eax;
        cpu.ebx = 0x54fbf4;
        cpu.ecx = kQueueFile;
    }
    return left;
}

void frontend::qinitpeer(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50dd60", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, priority = cpu.edx, argument = cpu.ebx, peer = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qinitpeer(app, cpu, queue, priority, argument, peer); });
}

/* sub_50de50: qpush: an element at the head. */
reg32 frontend::netb::qpush(App* app, CPU& cpu, reg32 queue, reg32 element)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc08, 0x128, queue);
    if (element != 0)
    {
        g.setDword(element, g.dword(queue + kQFirst));
        g.setDword(queue + kQFirst, element);
        g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
        if (g.dword(element) == 0)
            g.setDword(queue + kQLast, element);
        setChanged(g, queue);
    }
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return cpu.eax;
}

void frontend::qpush(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50de50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, element = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qpush(app, cpu, queue, element); });
}

/* sub_50ded0: qbottom: an element at the tail. */
reg32 frontend::netb::qbottom(App* app, CPU& cpu, reg32 queue, reg32 element)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc10, 0x13c, queue);
    if (element != 0)
    {
        const reg32 last = g.dword(queue + kQLast);
        g.setDword(element, 0);
        g.setDword(queue + kQLast, element);
        g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
        if (last != 0)
            g.setDword(last, element);
        else
            g.setDword(queue + kQFirst, element);
        setChanged(g, queue);
    }
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return cpu.eax;
}

void frontend::qbottom(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50ded0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, element = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qbottom(app, cpu, queue, element); });
}

/* sub_50df60: qpop: the element at the head, taken off; 0 for none. */
reg32 frontend::netb::qpop(App* app, CPU& cpu, reg32 queue)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc18, 0x17b, queue);
    const reg32 first = g.dword(queue + kQFirst);
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
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return first;
}

void frontend::qpop(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50df60", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qpop(app, cpu, queue); });
}

/* sub_50dff0: qpoplast: the element at the tail, taken off; 0 for none. */
reg32 frontend::netb::qpoplast(App* app, CPU& cpu, reg32 queue)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc20, 0x190, queue);
    const reg32 last = g.dword(queue + kQLast);
    if (last != 0)
    {
        const reg32 first = g.dword(queue + kQFirst);
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
                const reg32 at = g.dword(queue + kQLast);
                const reg32 next = g.dword(at);
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
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return last;
}

void frontend::qpoplast(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50dff0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qpoplast(app, cpu, queue); });
}

/* sub_50e0a0: qinsert: an element in the order of the queue's priority function. */
reg32 frontend::netb::qinsert(App* app, CPU& cpu, reg32 queue, reg32 element)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (element == 0)
    {
        leaveLock(g, cpu, g.dword(queue + kQLock));
        return cpu.eax;
    }
    const reg32 priority = queuePriority(g, queue, element);
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc2c, 0x1d8, queue);
    g.setDword(queue + kQCount, g.dword(queue + kQCount) + 1);
    reg32 at = g.dword(queue + kQFirst), before = 0;
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
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return cpu.eax;
}

void frontend::qinsert(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e0a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, element = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qinsert(app, cpu, queue, element); });
}

/* sub_50e2d0: qlowpriority: the priority of the element at the head, -1 for none. */
reg32 frontend::netb::qlowpriority(App* app, CPU& cpu, reg32 queue)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc50, 0x2b9, queue);
    reg32 priority = 0xffffffff;
    if (g.dword(queue + kQFirst) != 0)
        priority = queuePriority(g, queue, g.dword(queue + kQFirst));
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return priority;
}

void frontend::qlowpriority(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e2d0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qlowpriority(app, cpu, queue); });
}

/* sub_50e3f0: qfind: the first element a function says yes to, edx it, ebx its argument. */
reg32 frontend::netb::qfind(App* app, CPU& cpu, reg32 queue, reg32 match, reg32 argument)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc70, 0x303, queue);
    reg32 at = g.dword(queue + kQFirst);
    while (at != 0 && match != 0 && g.call(match, args({ at, argument, at })) == 0)
        at = g.dword(at);
    cpu.ebx = at;
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return at;
}

void frontend::qfind(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e3f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, match = cpu.edx, argument = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qfind(app, cpu, queue, match, argument); });
}

/* sub_50e500: qfindpriority: the element of exactly a priority, edx it; 0 for none. */
reg32 frontend::netb::qfindpriority(App* app, CPU& cpu, reg32 queue, reg32 priority)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fc84, 0x321, queue);
    reg32 at = g.dword(queue + kQFirst);
    while (at != 0 && queuePriority(g, queue, at) < priority)
        at = g.dword(at);
    if (at != 0 && queuePriority(g, queue, at) != priority)
        at = 0;
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return at;
}

void frontend::qfindpriority(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e500", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, priority = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qfindpriority(app, cpu, queue, priority); });
}

/* sub_50e8e0: qiterate: a function on each element until the queue changes, outside the lock;
 * -1 when it came to the end with no change, else how many it was called for.  A function that
 * answers 0 counts as a change: the walk stops there. */
reg32 frontend::netb::qiterate(App* app, CPU& cpu, reg32 queue, reg32 function, reg32 argument)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    enterLock(g, cpu, g.dword(queue + kQLock));
    const reg32 wasChanged = g.dword(queue + kQFlags) & 1;
    g.setByte(queue + kQFlags, std::uint8_t(g.byte(queue + kQFlags) & 0xfe));
    leaveLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fda4, 0x3f6, queue);
    reg32 called = 0;
    for (reg32 at = g.dword(queue + kQFirst); at != 0 && (g.byte(queue + kQFlags) & 1) == 0; at = g.dword(at))
    {
        if (g.call(function, args({ at, argument })) == 0)
            setChanged(g, queue);
        ++called;
    }
    enterLock(g, cpu, g.dword(queue + kQLock));
    if ((g.byte(queue + kQFlags) & 1) == 0)
        called = 0xffffffff;
    g.setDword(queue + kQFlags, g.dword(queue + kQFlags) | wasChanged);
    cpu.ebx = g.dword(queue + kQLock);
    leaveLock(g, cpu, cpu.ebx);
    return called;
}

void frontend::qiterate(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e8e0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, function = cpu.edx, argument = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qiterate(app, cpu, queue, function, argument); });
}

/* sub_50e9b0: qsafeiterate: qiterate with the lock held all along. */
reg32 frontend::netb::qsafeiterate(App* app, CPU& cpu, reg32 queue, reg32 function, reg32 argument)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    enterLock(g, cpu, g.dword(queue + kQLock));
    if (g.dword(queue + kQLock) == 0)
        reportQueue(g, 0x54fdb0, 0x414, queue);
    const reg32 called = guest::direct<0x50e8e0, qiterate>(app, cpu, queue, function, argument);
    cpu.ebx = called;
    leaveLock(g, cpu, g.dword(queue + kQLock));
    return called;
}

void frontend::qsafeiterate(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_50e9b0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 queue = cpu.eax, function = cpu.edx, argument = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::qsafeiterate(app, cpu, queue, function, argument); });
}

/* ---------------------------------------------------------------------------
 * cmn\layer.c: a packet handed up a connection's layers
 * ------------------------------------------------------------------------- */

/* sub_512470: under the last layer: the pieces -- edx of them at ebx, {length, address} --
 * gathered into one block for the connection's own function, eax the connection. */
reg32 frontend::netb::layerReceiveLast(App* app, CPU& cpu, reg32 connection, reg32 pieceCount, reg32 pieces)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1c);
    const sreg32 count = sreg32(pieceCount);
    cpu.ecx = pieces;
    reg32 total = 0;
    for (sreg32 i = 0; i < count; ++i)
        total += g.dword(pieces + 8 * reg32(i));
    setErrorAt(g, 0x550194, 0x5501b4, 0x4a);
    const reg32 block = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x5501c4, total, 0));
    reg32 to = block;
    for (sreg32 i = 0; i < count; ++i)
    {
        const reg32 piece = pieces + 8 * reg32(i);
        fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, g.dword(piece + 4), g.dword(piece));
        to += g.dword(piece);
    }
    const reg32 result = g.call(g.dword(connection + 0x20), args({ connection, block, total, connection }));
    cpu.ecx = result;
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    return result;
}

void frontend::layerReceiveLast(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512470", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.eax, pieceCount = cpu.edx, pieces = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::layerReceiveLast(app, cpu, connection, pieceCount, pieces); });
}

/* sub_5125b0: to the layer of an index in the queue of layers on the stack, or past the last. */
reg32 frontend::netb::layerCallUnpacker(App* app, CPU& cpu, reg32 first, reg32 index, reg32 second, reg32 pieces, reg32 layers)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    cpu.ebx = layers;
    if (sreg32(index) >= 0 && index < g.call(0x50e1d0, args({ layers })))
    {
        g.setDword(layers + 0x1c, index);
        const reg32 layer = g.call(0x50e470, args({ layers }));
        if (layer == 0)
        {
            setErrorAt(g, 0x550194, 0x5501f4, 0x78);
            g.call(kError, args({ 0x5501f4, 0x78 }, { 0x5501e0 }));
        }
        return g.call(g.dword(layer + 8), args({ first, g.dword(layer + 0xc), second, pieces }, { layers }));
    }
    // The pieces in ecx too, which it does not read.
    return guest::direct<0x512470, layerReceiveLast>(app, cpu, first, second, pieces);
}

void frontend::layerCallUnpacker(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5125b0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 first = cpu.eax;
    const reg32 index = cpu.edx;
    const reg32 second = cpu.ebx;
    const reg32 pieces = cpu.ecx;
    const reg32 layers = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::layerCallUnpacker(app, cpu, first, index, second, pieces, layers); });
}

/* sub_512650: from the top of a connection's layers: ecx them, edx the pieces' count, ebx them.
 * The caller's ecx is the argument sub_5125b0 pops. */
reg32 frontend::netb::layerReceiveFromTop(App* app, CPU& cpu, reg32 first, reg32 count, reg32 pieces, reg32 layers)
{
    Guest g(app, cpu);
    return guest::direct<0x5125b0, layerCallUnpacker>(app, cpu, first, 0, count, pieces, layers);
}

void frontend::layerReceiveFromTop(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512650", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 first = cpu.eax, count = cpu.edx, pieces = cpu.ebx, layers = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::layerReceiveFromTop(app, cpu, first, count, pieces, layers); });
}

/* sub_512660: CONN_initfilter: a connection's queue of layers. */
reg32 frontend::netb::connInitFilter(App* app, CPU& cpu, reg32 connection)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    setErrorAt(g, 0x550194, 0x550208, 0x9a);
    cpu.ecx = connection;
    const reg32 queue = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x5501c4, 0x20, g.dword(kMemoryGroup)));
    g.setDword(connection + 0x30, queue);
    guest::direct<0x50dcd0, qinit>(app, cpu, queue, 0, 0);
    return g.dword(connection + 0x30) != 0 ? 1 : 0;
}

void frontend::connInitFilter(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512660", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::connInitFilter(app, cpu, connection); });
}

/* sub_5126f0: a packet received on a connection, edx it, ebx how long: one piece for the layers. */
reg32 frontend::netb::layerReceive(App* app, CPU& cpu, reg32 connection, reg32 packet, reg32 length)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    const reg32 piece = g.frame();
    g.setDword(piece + 4, packet);
    g.setDword(piece, length);
    return guest::direct<0x512650, layerReceiveFromTop>(app, cpu, connection, 1, piece, g.dword(connection + 0x30));
}

void frontend::layerReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5126f0", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 connection = cpu.eax, packet = cpu.edx, length = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::layerReceive(app, cpu, connection, packet, length); });
}

/* ---------------------------------------------------------------------------
 * win\tcp.c: connections over TCP, and Winsock opened for IPX and TCP
 * ------------------------------------------------------------------------- */
namespace
{

// Winsock, through the import thunks.
constexpr reg32 kWsaStartup = 0x533eb0, kWsaCleanup = 0x533e9e, kWsaLastError = 0x533e38, kSocket = 0x533eaa;
constexpr reg32 kCloseSocket = 0x533ea4, kBind = 0x533e44, kConnect = 0x533e0e, kSend = 0x533e26, kRecv = 0x533e2c;
constexpr reg32 kShutdown = 0x533e08, kAccept = 0x533e14, kFdIsSet = 0x533e1a, kSelect = 0x533e20;
constexpr reg32 kHtons = 0x533e4a, kIoctlSocket = 0x533e50, kSetSockOpt = 0x533e56, kSendTo = 0x533e32;
constexpr reg32 kRecvFrom = 0x533e5c, kGetSockName = 0x533e3e;

// The two the code calls most: WSAGetLastError, and __WSAFDIsSet (FD_ISSET).
reg32 lastError(Guest& g, x86::CPU& cpu)
{
    return thunked<kWsaLastError, &win32::wsock32::WSAGetLastError>(g, cpu);
}

reg32 isSet(Guest& g, x86::CPU& cpu, reg32 socket, reg32 set)
{
    return thunked<kFdIsSet, &win32::wsock32::__WSAFDIsSet>(g, cpu, socket, set);
}
constexpr reg32 kGetTickCount = 0x534568;
constexpr reg32 kWouldBlock = 0x2733, kInProgress = 0x2747;  // WSAEWOULDBLOCK and what send gives while connecting

constexpr reg32 kQRemove = 0x50e150;
constexpr reg32 kOpenQueue = 0x9f2628, kFreeQueue = 0x9f260c;  // connections up, connections done with
constexpr reg32 kMaxPacket = 0x9f37e8;   // word; what the game's data allows, 0 for none set
constexpr reg32 kListener = 0x9f37f0;    // what the TCP thread listens with
constexpr reg32 kNetCount = 0x566de4, kNetNames = 0x9f25d0, kNetTables = 0x9f25b0;

// A TCP connection's fields.
constexpr reg32 kConnFunctions = 4, kConnBuffer = 0x10, kConnReady = 0x14, kConnClosing = 0x18, kConnUp = 0x24;
constexpr reg32 kConnAddress = 0x34, kConnSocket = 0x48, kConnNet = 0x4c;

/* Winsock's error when connect gives up for good, of those a socket that has not yet connected
 * may say; any other leaves it to try on. */
bool connectFailed(reg32 error)
{
    switch (error)
    {
    case 0x271e: case 0x2726:
    case 0x273f: case 0x2740: case 0x2741: case 0x2742: case 0x2743:
    case 0x2748: case 0x274d: case 0x2750: case 0x2751:
        return true;
    default:
        return false;
    }
}

/* What sub_4f63d0 and sub_4f7830 share: the network's name and table added to the list. */
void addNetKind(Guest& g, reg32 name, reg32 table)
{
    const reg32 count = g.dword(kNetCount);
    g.setDword(kNetNames + count * 4, name);
    g.setDword(kNetTables + count * 4, table);
    g.setDword(kNetCount, count + 1);
}

}  // namespace

/* sub_4f6350: what is queued for an address flushed: edx a packet whose first 20 bytes are the
 * address, or 0, ebx the rest for sub_4f5ac0 by way of the frame. */
reg32 frontend::netb::tcpFlushQueue(App* app, CPU& cpu, reg32 owner, reg32 packet, reg32 rest)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x24);
    const reg32 frame = g.frame();
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
    if (sreg32(g.dword(kTraceLevel)) >= 4)
        logEvent(g, cpu, 0x54d2b4, frame + 0x1c, packet, 0x14);
    guest::direct<0x50e9b0, qsafeiterate>(app, cpu, owner + 0x8c, 0x4f5ac0, frame);
    fe::direct<0x4dfb30, ea::eventSet>(app, cpu, g.dword(owner + 0x1c));
    return 1;
}

void frontend::tcpFlushQueue(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6350", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 owner = cpu.eax, packet = cpu.edx, rest = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpFlushQueue(app, cpu, owner, packet, rest); });
}

/* sub_4f63d0: IPX's kind of network, once: Winsock tried, and the list told if it will do.
 * eax what to remember; the kind's record, or 0. */
reg32 frontend::netb::ipxOpenWinsock(App* app, CPU& cpu, reg32 remember)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1a0);
    constexpr reg32 kOpened = 0x566e84, kRemembered = 0x566e24, kRecord = 0x566e28, kTable = 0x5689e8;
    if (g.dword(kNetLock) == 0)
        g.setDword(kNetLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    if (g.dword(kOpened) == 0 && thunked<kWsaStartup, &win32::wsock32::WSAStartup>(g, cpu, 0x101, g.frame()) == 0)
    {
        const reg32 found = guest::direct<0x4f5c10, neta::ipxOpen>(app, cpu, 0, 0, 0);
        if (found != 0)
        {
            g.setDword(kOpened, 1);
            guest::direct<0x4f5ea0, neta::ipxClose>(app, cpu, found);
            addNetKind(g, kRemembered, kTable);
            if (remember != 0)
                g.setDword(kRemembered, remember);
        }
        thunked<kWsaCleanup, &win32::wsock32::WSACleanup>(g, cpu);
    }
    return g.dword(kOpened) != 0 ? kRecord : 0;
}

void frontend::ipxOpenWinsock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f63d0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 remember = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxOpenWinsock(app, cpu, remember); });
}

/* sub_4f7830: the same for TCP: Windows has a network, Winsock starts and a socket can be made. */
reg32 frontend::netb::tcpOpenWinsock(App* app, CPU& cpu, reg32 remember)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    constexpr reg32 kOpened = 0x566f48, kRemembered = 0x566ee4, kRecord = 0x566ee8, kTable = 0x566ea0;
    constexpr reg32 kGetSystemMetrics = 0x534750, kSmNetwork = 0x3f;
    if (g.dword(kNetLock) == 0)
        g.setDword(kNetLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    if (g.dword(kOpened) == 0 && (windows<kGetSystemMetrics, &win32::user32::GetSystemMetrics>(g, cpu, kSmNetwork) & 1) != 0
        && thunked<kWsaStartup, &win32::wsock32::WSAStartup>(g, cpu, 0x101, 0x9f3660) == 0)
    {
        const reg32 probe = thunked<kSocket, &win32::wsock32::socket>(g, cpu, 2, 1, 0);
        if (probe != 0xffffffff)
        {
            addNetKind(g, kRemembered, kTable);
            cpu.edx = g.dword(kNetCount) - 1;
            thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, probe);
            g.setDword(kOpened, 1);
            if (remember != 0)
                g.setDword(kRemembered, remember);
        }
        thunked<kWsaCleanup, &win32::wsock32::WSACleanup>(g, cpu);
    }
    return g.dword(kOpened) != 0 ? kRecord : 0;
}

void frontend::tcpOpenWinsock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7830", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 remember = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpOpenWinsock(app, cpu, remember); });
}

/* sub_4f6490: a TCP socket bound to a port, -1 for none. */
reg32 frontend::netb::tcpListenSocket(App* app, CPU& cpu, reg32 port)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x20);
    const reg32 address = g.frame();
    const reg32 socket = thunked<kSocket, &win32::wsock32::socket>(g, cpu, 2, 1, 0);
    if (socket == 0xffffffff)
        return 0xffffffff;
    g.call(0x4e070c, args({ address, 0x10 }));
    g.setWord(address, 2);
    cpu.eax = port & 0xffff;  // eax and edx as the code has them at the call
    cpu.edx = 2;
    const reg32 network = thunked<kHtons, &win32::wsock32::htons>(g, cpu, port & 0xffff);
    g.setWord(address + 2, std::uint16_t(network));
    if (thunked<kBind, &win32::wsock32::bind>(g, cpu, socket, address, 0x10) == 0)
        return socket;
    thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, socket);
    return 0xffffffff;
}

void frontend::tcpListenSocket(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6490", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 port = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpListenSocket(app, cpu, port); });
}

/* sub_4f6510: a socket made non-blocking, reusable and without Nagle's delay. */
reg32 frontend::netb::tcpSetSocketOptions(App* app, CPU& cpu, reg32 socket)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    const reg32 on = g.frame();
    constexpr reg32 kFionbio = 0x8004667e, kSolSocket = 0xffff, kSoReuseAddr = 8, kIpProtoTcp = 6, kTcpNoDelay = 1;
    g.setDword(on, 1);
    thunked<kIoctlSocket, &win32::wsock32::ioctlsocket>(g, cpu, socket, kFionbio, on);
    g.setDword(on, 1);
    thunked<kSetSockOpt, &win32::wsock32::setsockopt>(g, cpu, socket, kSolSocket, kSoReuseAddr, on, 4);
    g.setDword(on, 1);
    return thunked<kSetSockOpt, &win32::wsock32::setsockopt>(g, cpu, socket, kIpProtoTcp, kTcpNoDelay, on, 4);
}

void frontend::tcpSetSocketOptions(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6510", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 socket = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpSetSocketOptions(app, cpu, socket); });
}

/* sub_4f6570: one connection into select's three sets, for qsafeiterate: edx the frame of
 * sub_4f68c0, its +0 the listener, +4 the read set, +0x108 the write set, +0x20c the except set
 * (a count and 64 sockets each), +0x310 the connections so added. */
reg32 frontend::netb::tcpFillSelectSets(App* app, CPU& cpu, reg32 connection, reg32 sets)
{
    Guest g(app, cpu);
    constexpr reg32 kMost = 0x40;
    if (g.dword(g.dword(connection + kConnFunctions)) != 1 || g.dword(connection + kConnSocket) == 0xffffffff
        || connection == g.dword(g.dword(sets) + 0x38))
        return 1;
    const reg32 added = g.dword(sets + 0x310);
    g.setDword(sets + 0x310, added + 1);
    g.setDword(sets + 0x314 + added * 4, connection);
    const reg32 reading = g.dword(sets + 4);
    if (reading < kMost)
    {
        g.setDword(sets + 4, reading + 1);
        g.setDword(sets + 8 + reading * 4, g.dword(connection + kConnSocket));
    }
    const reg32 excepting = g.dword(sets + 0x20c);
    if (excepting < kMost)
    {
        g.setDword(sets + 0x20c, excepting + 1);
        cpu.edx = g.dword(connection + kConnSocket);
        g.setDword(sets + 0x210 + excepting * 4, cpu.edx);
    }
    if (g.word(g.dword(connection + kConnBuffer) + 6) != 0)
    {
        const reg32 writing = g.dword(sets + 0x108);
        if (writing < kMost)
        {
            g.setDword(sets + 0x108, writing + 1);
            cpu.edx = g.dword(connection + kConnSocket);
            g.setDword(sets + 0x10c + writing * 4, cpu.edx);
        }
    }
    return 1;
}

void frontend::tcpFillSelectSets(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6570", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.eax, sets = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpFillSelectSets(app, cpu, connection, sets); });
}

/* sub_4f6640: a packet read from a connection's socket when it has all come: a length of two
 * bytes in network order, peeked at, then the whole of it peeked at and handed to the layers;
 * taken off the socket when they have used it.  1 when the connection is dead. */
reg32 frontend::netb::tcpReceivePacket(App* app, CPU& cpu, reg32 listener, reg32 connection)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x424);
    constexpr reg32 kMsgPeek = 2;
    const reg32 data = g.frame();
    const reg32 buffer = g.dword(connection + kConnBuffer);
    const auto receive = [&](reg32 bytes, reg32 flags) {
        return thunked<kRecv, &win32::wsock32::recv>(g, cpu, g.dword(connection + kConnSocket), data, bytes, flags);
    };
    enterLock(g, cpu, g.dword(kNetLock));
    reg32 dead = 0;
    const auto readFailed = [&](reg32 got) {
        if (got == 0)
            dead = 1;
        else if (got == 0xffffffff && lastError(g, cpu) != kWouldBlock)
            dead = 1;
    };
    const reg32 peeked = receive(2, kMsgPeek);
    if (peeked == 0 || (peeked == 0xffffffff && lastError(g, cpu) != kWouldBlock))
    {
        dead = 1;
    }
    else if (sreg32(peeked) >= 2)
    {
        const reg32 size = swapped(g.dword(data)) >> 16;
        if (sreg32(size) > sreg32(reg32(g.word(buffer + 4))))
        {
            dead = 1;
        }
        else
        {
            reg32 got = receive(size + 2, kMsgPeek);
            if (got == size + 2)
            {
                if (sreg32(g.dword(kTraceLevel)) >= 4)
                    logEventLeaving(g, 0x54d2c8, 0, connection + kConnAddress, 0x14);
                if (g.dword(listener + 0x40) != 0)
                    g.call(g.dword(listener + 0x40),
                           args({ g.dword(connection + kConnNet), connection + kConnAddress, listener }));
                if (guest::direct<0x5126f0, layerReceive>(app, cpu, connection, data + 2, size) != 0)
                    got = receive(size + 2, 0);
            }
            readFailed(got);
        }
    }
    leaveLock(g, cpu, g.dword(kNetLock));
    return dead;
}

void frontend::tcpReceivePacket(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6640", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 listener = cpu.eax, connection = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpReceivePacket(app, cpu, listener, connection); });
}

/* sub_4f67b0: what is left of a connection's packet sent.  1 when the connection is dead. */
reg32 frontend::netb::tcpSendPending(App* app, CPU& cpu, reg32 connection)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    const reg32 buffer = g.dword(connection + kConnBuffer);
    cpu.edx = (connection & 0xffff0000) | g.word(buffer + 6);
    if (g.word(buffer + 6) == 0)
        return 0;
    enterLock(g, cpu, g.dword(kNetLock));
    const reg32 sent = thunked<kSend, &win32::wsock32::send>(g, cpu, g.dword(connection + kConnSocket),
                                                             g.dword(buffer + 8), g.word(buffer + 6), 0);
    reg32 dead = 0;
    if (sent == 0xffffffff)
    {
        const reg32 error = lastError(g, cpu);
        if (error != kInProgress && error != kWouldBlock)
        {
            dead = 1;
            g.setWord(buffer + 6, 0);
        }
    }
    else if (sreg32(sent) > 0)
    {
        g.setWord(buffer + 6, std::uint16_t(g.word(buffer + 6) - sent));
        g.setDword(buffer + 8, g.dword(buffer + 8) + sent);
    }
    leaveLock(g, cpu, g.dword(kNetLock));
    return dead;
}

void frontend::tcpSendPending(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f67b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpSendPending(app, cpu, connection); });
}

/* sub_4f6870: a dead connection closed and off the open queue, told to its owner. */
reg32 frontend::netb::tcpConnectionDropped(App* app, CPU& cpu, reg32 connection)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    cpu.ebx = connection;
    thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, g.dword(connection + kConnSocket));
    g.setDword(connection + kConnSocket, 0xffffffff);
    const reg32 removed = g.call(kQRemove, args({ kOpenQueue, connection }));
    if (removed == 0)
        return 0;
    g.setDword(connection + kConnUp, 0);
    g.call(g.dword(connection + kConnClosing), args({ connection, connection }));
    return guest::direct<0x50de50, qpush>(app, cpu, kFreeQueue, cpu.edx);
}

void frontend::tcpConnectionDropped(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6870", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpConnectionDropped(app, cpu, connection); });
}

/* sub_4f68c0: the TCP thread: selects over the listening socket, a connection going up and the
 * connections open, accepts, receives and sends, until the listener says stop.  Its frame is
 * what sub_4f6570 fills; +0x414 what accept gives. */
reg32 frontend::netb::tcpServerThread(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x448);
    constexpr reg32 kStop = 0x46, kRunning = 0x47, kGame = 0x3c, kAccepted = 0x1c, kPeerAddress = 0x20;
    constexpr reg32 kAcceptedBy = 0x34, kListenSocket = 0x18, kConnecting = 0x38;
    const reg32 frame = g.frame();
    const reg32 reading = frame + 4, writing = frame + 0x108, excepting = frame + 0x20c, added = frame + 0x310;
    const reg32 wait = frame + 0x424, peer = frame + 0x414, peerLength = frame + 0x42c;
    const reg32 listener = g.dword(kListener);
    g.setDword(frame, listener);
    g.setByte(listener + kRunning, 1);
    while (g.byte(listener + kStop) == 0)
    {
        // A socket accepted and not yet taken up: closed when it has waited too long, or
        // the game does not want more.
        const reg32 game = g.dword(listener + kGame);
        const reg32 wants = game != 0 ? g.dword(game + 0x10) : 0;
        const reg32 accepted = g.dword(listener + kAccepted);
        if (accepted != 0xffffffff)
        {
            bool stale = true;
            if (wants == 0 || g.call(wants, args({ game, listener + kPeerAddress, wants, accepted })) != 0)
                stale = sreg32(g.dword(kNow)) > sreg32(g.dword(listener + kAcceptedBy));
            if (stale)
            {
                thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, g.dword(listener + kAccepted));
                g.setDword(listener + kAccepted, 0xffffffff);
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
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
        }
        const reg32 going = g.dword(listener + kConnecting);
        if (going != 0)
        {
            const reg32 inWrite = g.dword(writing);
            if (inWrite < 0x40)
            {
                g.setDword(writing, inWrite + 1);
                g.setDword(writing + 4 + inWrite * 4, g.dword(going + kConnSocket));
            }
            const reg32 inExcept = g.dword(excepting);
            if (inExcept < 0x40)
            {
                g.setDword(excepting, inExcept + 1);
                g.setDword(excepting + 4 + inExcept * 4, g.dword(g.dword(listener + kConnecting) + kConnSocket));
            }
        }
        guest::direct<0x50e9b0, qsafeiterate>(app, cpu, kOpenQueue, 0x4f6570, frame);
        enterLock(g, cpu, g.dword(kNetLock));
        leaveLock(g, cpu, g.dword(kNetLock));
        g.setDword(wait, 0);
        g.setDword(wait + 4, 0x61a8);
        if (thunked<kSelect, &win32::wsock32::select>(g, cpu, 0, reading, writing, excepting, wait) != 0xffffffff)
        {
            if (isSet(g, cpu, g.dword(listener + kListenSocket), reading) != 0)
            {
                g.setDword(peerLength, 0x10);
                const reg32 socket = thunked<kAccept, &win32::wsock32::accept>(g, cpu, g.dword(listener + kListenSocket),
                                                                               peer, peerLength);
                g.setDword(listener + kAccepted, socket);
                if (socket == 0xffffffff)
                {
                    g.setByte(listener + kStop, 1);
                }
                else
                {
                    g.setWord(peer, 0);
                    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, listener + kPeerAddress, peer, g.dword(peerLength));
                    guest::direct<0x4f6510, tcpSetSocketOptions>(app, cpu, g.dword(listener + kAccepted));
                    g.setDword(listener + kAcceptedBy, g.dword(kNow) + (g.dword(kTicksPerSecond) << 2));
                }
            }
            const reg32 connecting = g.dword(listener + kConnecting);
            if (connecting != 0)
            {
                if (isSet(g, cpu, g.dword(connecting + kConnSocket), excepting) != 0)
                {
                    g.setDword(connecting + kConnUp, 0);
                    g.setDword(listener + kConnecting, 0);
                }
                else if (isSet(g, cpu, g.dword(connecting + kConnSocket), writing) != 0)
                {
                    g.setDword(connecting + kConnUp, 1);
                    g.setDword(listener + kConnecting, 0);
                }
            }
            for (sreg32 i = 0; i < sreg32(g.dword(added)); ++i)
            {
                const reg32 at = frame + 0x314 + reg32(i) * 4;
                const reg32 connection = g.dword(at);
                reg32 dead = 0;
                if (g.dword(connection + kConnSocket) != 0xffffffff && connection != connecting)
                {
                    if (isSet(g, cpu, g.dword(connection + kConnSocket), reading) != 0)
                        dead = guest::direct<0x4f6640, tcpReceivePacket>(app, cpu, listener, g.dword(at));
                    if (isSet(g, cpu, g.dword(g.dword(at) + kConnSocket), writing) != 0)
                        dead |= guest::direct<0x4f67b0, tcpSendPending>(app, cpu, g.dword(at));
                    if (isSet(g, cpu, g.dword(g.dword(at) + kConnSocket), excepting) != 0)
                        dead |= guest::direct<0x4f6860, tcpExceptionSeen>(app, cpu);
                }
                if (dead != 0 && g.dword(at) != 0)
                    guest::direct<0x4f6870, tcpConnectionDropped>(app, cpu, g.dword(at));
            }
        }
    }
    g.setByte(listener + kRunning, 0);
    return listener;
}

void frontend::tcpServerThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f68c0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpServerThread(app, cpu); });
}

/* sub_4f6c50: a connection opened over TCP: with no address, by waiting for someone to connect
 * to the thread; with one, by connecting there, the two ends' sockets racing, what is accepted
 * from that address winning.  1 when it is up. */
reg32 frontend::netb::tcpOpenConnection(App* app, CPU& cpu, reg32 connection, reg32 room)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1c);
    constexpr reg32 kAccepted = 0x1c, kPeerAddress = 0x20, kConnecting = 0x38;
    if (connection == 0)
        return room + 0xe;
    const reg32 frame = g.frame();
    const reg32 bufferAt = frame + 4, addressAt = frame + 8, socketAt = frame + 0xc, listenerAt = frame + 0x10;
    const reg32 given = g.dword(connection + kConnBuffer);
    reg32 buffer = given;
    if (given == 0)
    {
        const reg32 size = room + 0xe;
        setErrorAt(g, 0x54d2d0, 0x54d2dc, 0x1ac);
        cpu.ecx = 0x54d2d0;
        buffer = g.call(kReserve, args({ 0x54d2f0, size, g.dword(kMemoryGroup) }));
    }
    g.setDword(bufferAt, buffer);
    g.setDword(buffer, g.dword(connection + kConnBuffer) == 0 ? 1 : 0);
    const reg32 maximum = g.word(kMaxPacket);
    g.setWord(buffer + 6, 0);
    g.setDword(buffer + 8, 0);
    g.setWord(buffer + 4, std::uint16_t(maximum != 0 ? maximum - 2 : 0x400));
    const reg32 listener = g.dword(g.dword(connection + kConnNet) + 0xc);
    g.setDword(listenerAt, listener);
    g.setDword(connection + kConnUp, 0);
    const std::uint16_t family = g.word(connection + kConnAddress);
    cpu.ebx = (cpu.ebx & 0xffff0000) | family;
    g.setDword(connection + kConnBuffer, buffer);
    cpu.edx = buffer;

    const auto ready = [&] { return g.call(g.dword(connection + kConnReady), args({ connection })); };
    const auto oneSecond = [&](reg32 ticks) {
        const sreg32 each = sreg32(0x3e8) / sreg32(ticks);
        cpu.edx = reg32(sreg32(0x3e8) % sreg32(ticks));
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, reg32(each));
    };

    bool waiting = family == 0;  // esi of the code: no address, or given up
    if (waiting)
    {
        cpu.ebx = 0;
        while (g.dword(listener + kAccepted) == 0xffffffff)
        {
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
            {
                if (sreg32(reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
            }
            if (ready() != 0)
                break;
            cpu.ecx = g.dword(kTicksPerSecond);
            oneSecond(cpu.ecx);
        }
    }

    bool takeAccepted = false;
    if (g.dword(listener + kAccepted) != 0xffffffff)
    {
        takeAccepted = waiting
            || g.call(kCheckPlayer, args({ g.dword(connection + kConnNet), connection + kConnAddress, listener + kPeerAddress })) != 0;
    }
    if (takeAccepted)
    {
        enterLock(g, cpu, g.dword(kNetLock));
        g.setDword(connection + kConnSocket, g.dword(listener + kAccepted));
        copyDwords(g, connection + kConnAddress, listener + kPeerAddress, 5);
        g.setDword(connection + kConnUp, 1);
        g.setDword(listener + kAccepted, 0xffffffff);
        leaveLock(g, cpu, g.dword(kNetLock));
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
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
            {
                if (sreg32(reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0))) < 0)
                {
                    giveUp();
                    if (g.dword(listener + kConnecting) != 0)
                        continue;
                    break;
                }
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
            }
            if (ready() != 0)
            {
                giveUp();
                if (g.dword(listener + kConnecting) != 0)
                    continue;
                break;
            }
            const reg32 socket = guest::direct<0x4f6490, tcpListenSocket>(app, cpu, 0);
            cpu.ebx = socket;
            g.setDword(socketAt, socket);
            const reg32 until = windows<kGetTickCount, &win32::kernel32::GetTickCount>(g, cpu) + 0x3e8;
            if (socket == 0xffffffff)
            {
                waiting = true;
                if (g.dword(listener + kConnecting) != 0)
                    continue;
                break;
            }
            guest::direct<0x4f6510, tcpSetSocketOptions>(app, cpu, socket);
            thunked<kConnect, &win32::wsock32::connect>(g, cpu, socket, g.dword(addressAt), 0x10);
            const reg32 error = lastError(g, cpu);
            if (error == kWouldBlock)
            {
                g.setDword(connection + kConnSocket, socket);
                while (g.dword(listener + kConnecting) != 0)
                {
                    if (until <= windows<kGetTickCount, &win32::kernel32::GetTickCount>(g, cpu))
                        break;
                    bool failed = false;
                    if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
                        failed = sreg32(reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0))) < 0;
                    else
                        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
                    if (failed || ready() != 0)
                    {
                        giveUp();
                    }
                    else
                    {
                        cpu.ebx = g.dword(kTicksPerSecond);
                        oneSecond(cpu.ebx);
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
            thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, g.dword(socketAt));
            if (g.dword(listener + kConnecting) == 0)
                break;
        }
    }

    cpu.edx = g.dword(connection + kConnUp);
    if (cpu.edx == 0 && g.dword(g.dword(bufferAt)) != 0)
    {
        g.setDword(connection + kConnBuffer, cpu.edx);
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, g.dword(bufferAt));
    }
    else if (g.dword(connection + kConnUp) != 0)
    {
        guest::direct<0x50de50, qpush>(app, cpu, kOpenQueue, connection);
    }
    return g.dword(connection + kConnUp);
}

void frontend::tcpOpenConnection(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6c50", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.eax, room = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpOpenConnection(app, cpu, connection, room); });
}

/* sub_4f7030: a packet sent on a connection: its length in network order before it, into the
 * connection's buffer, and what the socket will not take now left there.  0 while a packet
 * before it is still going, or when the socket is not connected yet. */
reg32 frontend::netb::tcpSendConnection(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    const reg32 frame = g.frame();
    const reg32 buffer = g.dword(connection + kConnBuffer);
    const reg32 room = g.word(buffer + 4);
    if (length > room)
    {
        setErrorAt(g, 0x54d2d0, 0x54d2f8, 0x22e);
        g.call(kError, args({ room, 0x54d2d0, 0x22e }, { 0x54d30c, room }));
    }
    if (g.word(buffer + 6) != 0)
        return 0;
    enterLock(g, cpu, g.dword(kNetLock));
    g.setWord(buffer + 0xc, std::uint16_t(swapped(length) >> 16));
    g.call(kMemmove, args({ data, buffer + 0xe, length }));
    const reg32 whole = length + 2;
    g.setDword(buffer + 8, buffer + 0xc);
    g.setWord(buffer + 6, std::uint16_t(whole));
    const reg32 sent = thunked<kSend, &win32::wsock32::send>(g, cpu, g.dword(connection + kConnSocket),
                                                             g.dword(buffer + 8), whole & 0xffff, 0);
    if (sent == 0xffffffff)
    {
        const reg32 error = lastError(g, cpu);
        g.setDword(frame, error);
        if (sreg32(g.dword(kTraceLevel)) >= 4)
            guest::direct<0x512180, commEvent>(app, cpu, 0x54d344, frame, connection + kConnSocket, 4, 1, 0);
        if (error == kInProgress || error == kWouldBlock)
        {
            g.setWord(buffer + 6, 0);
            leaveLock(g, cpu, g.dword(kNetLock));
            return 0;
        }
    }
    else if (sreg32(sent) > 0)
    {
        g.setWord(buffer + 6, std::uint16_t(g.word(buffer + 6) - sent));
        g.setDword(buffer + 8, g.dword(buffer + 8) + sent);
        cpu.ebx = g.dword(kTraceLevel);
        if (sreg32(cpu.ebx) >= 4)
            logEvent(g, cpu, 0x54d350, 0, connection + kConnAddress, 0x14);
    }
    leaveLock(g, cpu, g.dword(kNetLock));
    return 1;
}

void frontend::tcpSendConnection(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7030", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.eax, data = cpu.edx, length = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpSendConnection(app, cpu, connection, data, length); });
}

/* sub_4f71a0: a connection closed for good and its buffer freed if it was its own. */
reg32 frontend::netb::tcpCloseConnection(App* app, CPU& cpu, reg32 connection)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    if (g.call(kQRemove, args({ kOpenQueue, connection })) != 0)
    {
        thunked<kShutdown, &win32::wsock32::shutdown>(g, cpu, g.dword(connection + kConnSocket), 2);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
        thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, g.dword(connection + kConnSocket));
        g.setDword(connection + kConnSocket, 0xffffffff);
        g.setDword(connection + kConnUp, 0);
        guest::direct<0x50de50, qpush>(app, cpu, kFreeQueue, connection);
    }
    if (g.call(kQRemove, args({ kFreeQueue, connection })) == 0)
        return 0;
    const reg32 buffer = g.dword(connection + kConnBuffer);
    if (g.dword(buffer) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, buffer);
        g.setDword(connection + kConnBuffer, 0);
    }
    return 1;
}

void frontend::tcpCloseConnection(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f71a0", 0, 0, nullptr, 0,
                         kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpCloseConnection(app, cpu, connection); });
}

/* sub_4f7220: whether two addresses are the same: a dword, two words and 8 bytes. */
reg32 frontend::netb::tcpSameAddressParts(App* app, CPU& cpu, reg32 first, reg32 second)
{
    Guest g(app, cpu);
    cpu.edx = g.dword(second);
    if (cpu.edx != g.dword(first))
        return 0;
    cpu.edx = (cpu.edx & 0xffff0000) | g.word(second + 4);
    if (g.word(second + 4) != g.word(first + 4) || g.word(second + 6) != g.word(first + 6))
        return 0;
    for (cpu.edx = 0; cpu.edx < 8; ++cpu.edx)
        if (g.byte(first + 8 + cpu.edx) != g.byte(second + 8 + cpu.edx))
            return 0;
    return 1;
}

void frontend::tcpSameAddressParts(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7220", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 first = cpu.eax, second = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpSameAddressParts(app, cpu, first, second); });
}

/* sub_4f7280: a game hosted over a network: edx the player's address, ebx what it is opened
 * with, the stack the game's name and the player's. */
reg32 frontend::netb::tcpHostGame(App* app, CPU& cpu, reg32 net, reg32 player, reg32 address, reg32 gameName, reg32 playerName)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    constexpr reg32 kOpenResult = 0x566f44;
    if (guest::direct<0x4f7220, tcpSameAddressParts>(app, cpu, address, g.dword(net + 8) + 4) == 0)
        return 0;
    cpu.edx = 8;  // as it leaves edx when the parts agree: the kind's open below takes it
    guest::direct<0x4f5230, neta::servicesReady>(app, cpu);
    const reg32 opened = g.call(g.dword(g.dword(net + 8) + 0x14), args({ g.dword(net + 8) }));
    g.setDword(kOpenResult, opened);
    if (opened == 0)
        return 0;
    const reg32 functions = g.dword(net + 8);
    const reg32 game = g.call(g.dword(functions + 0x1c), args({ cpu.ecx, net, 0 }));
    g.setDword(net + 0xc, game);
    if (game == 0)
        return 0;
    const reg32 same = g.dword(net + 8);
    g.call(g.dword(same + 0x44), args({ game, net + 0x30, 0x10, same }));
    g.setDword(net + 0x9c, 1);
    g.setDword(net + 0x14, 0);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, net + 0x8c, player, cpu.ebx);
    g.call(kStrncpy, args({ net + 0x44, gameName, 0x33 }));
    g.call(kStrncpy, args({ net + 0x78, playerName, 0x13 }));
    return g.dword(kOpenResult);
}

void frontend::tcpHostGame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7280", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 net = cpu.eax;
    const reg32 player = cpu.edx;
    const reg32 address = cpu.ebx;
    const reg32 gameName = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 playerName = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpHostGame(app, cpu, net, player, address, gameName, playerName); });
}

/* sub_4f7320: the game of a NET object ended: its players told, its game closed. */
reg32 frontend::netb::tcpEndGame(App* app, CPU& cpu, reg32 net)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    const reg32 game = g.dword(net + 0xc);
    cpu.edx = net;
    cpu.ecx = game;
    if (game == 0)
        return 1;
    if (g.dword(net + 0xa0) != 0)
        g.call(0x4f4f90, args({ net }));
    // ebx is saved only from here on, past what sub_4f4f90 leaves in it.
    const reg32 ebx = cpu.ebx;
    const reg32 functions = g.dword(net + 8);
    g.call(g.dword(functions + 0x20), args({ game, net, functions, game }));
    g.setDword(net + 0x9c, 0);
    const reg32 same = g.dword(net + 8);
    g.setDword(net + 0xc, 0);
    const reg32 result = g.call(g.dword(same + 0x18), args({ same }));
    cpu.ebx = ebx;
    return result;
}

void frontend::tcpEndGame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7320", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const reg32 net = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpEndGame(app, cpu, net); });
}

/* sub_4f7370: what the kinds' tables hold for a function a kind has no need of. */
reg32 frontend::netb::returnZero(App* app, CPU& cpu)
{
    return 0;
}

void frontend::netReturnZero(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7370", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::returnZero(app, cpu); });
}

/* sub_4f7380: a network over TCP: sub_4f5130 with this kind's table at 0x566e8c. */
reg32 frontend::netb::tcpOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 first, reg32 second)
{
    Guest g(app, cpu);
    if (settings != 0)
        g.setWord(settings + 0x28, 1);
    return guest::direct<0x4f5130, neta::serviceCall>(app, cpu, settings, 0x566e8c, 0, first, second);
}

void frontend::tcpOpenNetwork(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7380", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 settings = cpu.edx, first = cpu.ebx, second = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpOpenNetwork(app, cpu, settings, first, second); });
}

/* sub_4f73a0: Winsock started for one more user, a socket's limit read once; the largest packet
 * a connection will carry, 0 when Winsock will not start. */
reg32 frontend::netb::tcpStartWinsock(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 8);
    constexpr reg32 kUsers = 0x566e88;
    if (g.dword(kUsers) == 0 && thunked<kWsaStartup, &win32::wsock32::WSAStartup>(g, cpu, 0x101, 0x9f3660) != 0)
        return 0;
    const reg32 socket = thunked<kSocket, &win32::wsock32::socket>(g, cpu, 2, 1, 0);
    if (socket != 0xffffffff)
    {
        cpu.edx = socket;
        fe::direct<0x5150f0, frontend::eab::netLocalAddress>(app, cpu, 0xa0b120);
        thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, socket);
    }
    g.setDword(kUsers, g.dword(kUsers) + 1);
    const reg32 maximum = g.word(kMaxPacket);
    if (maximum == 0)
        return 0x400;
    const reg32 room = maximum - 2;
    return sreg32(room) <= 0x400 ? room : 0x400;
}

void frontend::tcpStartWinsock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f73a0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpStartWinsock(app, cpu); });
}

/* sub_4f7420: one user of Winsock less, stopped with the last; 0 when it was not started or
 * would not stop. */
reg32 frontend::netb::tcpStopWinsock(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 8);
    constexpr reg32 kUsers = 0x566e88;
    const reg32 users = g.dword(kUsers);
    if (users == 0)
        return 0;
    g.setDword(kUsers, users - 1);
    if (users - 1 == 0 && thunked<kWsaCleanup, &win32::wsock32::WSACleanup>(g, cpu) != 0)
        return 0;
    return 1;
}

void frontend::tcpStopWinsock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f7420", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpStopWinsock(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * win\tapipkt.c: the modem, through TAPI, and the serial port's table
 * ------------------------------------------------------------------------- */
namespace
{

constexpr reg32 kTapiFile = 0x54da88;
constexpr reg32 kLineApp = 0x9f4d74, kDeviceCount = 0x9f4d90, kApiVersion = 0x9f4d8c, kDevCaps = 0x9f4d88;
constexpr reg32 kModemCount = 0x9f4d80, kModems = 0x9f4d94, kModemSettings = 0x9f4d84, kLine = 0x9f4da0, kCall = 0x9f4da4;
constexpr reg32 kModemHandle = 0x9f4da8, kModemState = 0x9f4d9c, kThreadStarting = 0x9f4d98, kThreadId = 0x9f4d70;
constexpr reg32 kChosenDevice = 0x5671a8, kModemCallback = 0x5671ac, kSerialCallback = 0x9f4eac;
constexpr reg32 kCommConfig = 0x9f4d78, kAnswerMode = 0x9f4d7c, kCommDcb = 0x9f3d60, kCommClass = 0x54db40;
constexpr reg32 kFileReport = 0x501050, kReportStream = 0x564454, kReportText = 0x56abb4;
constexpr reg32 kPostThreadMessage = 0x534770, kConnectionQuit = 0x4c8;
constexpr reg32 kApiLow = 0x10004;       // the version of TAPI asked for, low and high
constexpr reg32 kStateConnected = 9;

void fileReport(Guest& g, reg32 text)
{
    g.setDword(kReportText, text);
    g.call(kFileReport, args({}, { kReportStream, 1, text }));
}

/* The modem's thread told to stop: its message queue gets WM_USER+200. */
void postQuit(Guest& g, x86::CPU& cpu, reg32 settings)
{
    g.setDword(settings + 0x34, 0);
    windows<kPostThreadMessage, &win32::user32::PostThreadMessageA>(g, cpu, g.dword(kThreadId), kConnectionQuit, 0, 0);
}

/* What sub_4fa550 and sub_4fa780 share: the timeouts a network's settings leave at 0 taken from
 * the ticks of a second, and its kind. */
void fillTimeouts(Guest& g, reg32 settings, reg32 ticks, sreg32 second, std::uint16_t kind)
{
    if (settings == 0)
        return;
    if (g.word(settings + 0x2a) == 0)
        g.setWord(settings + 0x2a, std::uint16_t(sreg32(ticks) / 16));
    if (g.word(settings + 0x2c) == 0)
        g.setWord(settings + 0x2c, std::uint16_t(sreg32(ticks) / second));
    if (g.word(settings + 0x2e) == 0)
        g.setWord(settings + 0x2e, std::uint16_t(sreg32(ticks) / second));
    g.setWord(settings + 0x28, kind);
}

/* sub_4fa500 and sub_4fa730: bytes sent on an object's port, a failure's count made positive. */
reg32 portSend(Guest& g, x86::CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    const reg32 port = g.dword(object + 0x38);
    // The port in ecx too, which it does not read.
    const reg32 result = fe::direct<0x51ba50, frontend::netc::serialRead>(g.app(), cpu, port, buffer, length);
    cpu.edx = sreg32(result) < 0 ? 0 - result : result;
    return cpu.edx;
}

/* sub_4fa530 and sub_4fa760: bytes received. */
reg32 portReceive(Guest& g, x86::CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    const reg32 port = g.dword(object + 0x38);
    // The port in ecx too, which it does not read.
    return fe::direct<0x51b530, frontend::netc::serialWriteWhole>(g.app(), cpu, port, buffer, length);
}

}  // namespace

/* sub_4f9a50: the modem of an index chosen, the list made first if it has not been. */
reg32 frontend::netb::tapiSelectModem(App* app, CPU& cpu, reg32 index)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 8);
    if (g.dword(kModems) == 0)
        guest::direct<0x4f9ac0, tapiEnumerateModems>(app, cpu);
    if (sreg32(index) >= sreg32(g.dword(kModemCount)))
    {
        setErrorAt(g, kTapiFile, 0x54daec, 0x176);
        g.call(kError, args({}, { 0x54dafc }));
        return 0;
    }
    g.setDword(kChosenDevice, g.dword(g.dword(kModems) + index * 8));
    return 1;
}

void frontend::tapiSelectModem(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9a50", 0, 0, nullptr, 0, kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const reg32 index = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiSelectModem(app, cpu, index); });
}

/* sub_4f9ac0: TapiEnumerateModems: the list of the lines that are modems, 20 places of a device
 * and its name; how many.  ebx is saved only where the code saves it, past lineInitialize. */
reg32 frontend::netb::tapiEnumerateModems(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x28);
    constexpr reg32 kListBytes = 0xa0, kGetWindowLong = 0x534758, kCapsDatamodem = 0x10;
    g.setDword(kModemCount, 0);
    const reg32 window = g.call(0x4eba70);
    const reg32 instance = windows<kGetWindowLong, &win32::user32::GetWindowLongA>(g, cpu, window, 0xfffffffa);
    cpu.eax = instance;  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x5310be, frontend::watc::lineInitialize>(app, cpu, kLineApp, instance, 0x4f9500, 0, kDeviceCount) != 0)
        return 0;
    if (g.dword(kDeviceCount) == 0)
    {
        // eax as this call leaves it, handed on to the stub, which does not read it.
        cpu.eax = reg32(*fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0x1f4));
        if (fe::direct<0x5324d2, frontend::watc::lineShutdown>(app, cpu, g.dword(kLineApp)) != 0)
            fileReport(g, 0x54da70);
        return 0;
    }
    if (g.dword(kModems) != 0)
    {
        for (reg32 at = 0; at != kListBytes; at += 8)
        {
            const reg32 name = g.dword(g.dword(kModems) + at + 4);
            if (name != 0)
                fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, name);
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, g.dword(kModems));
        g.setDword(kModems, 0);
    }
    const reg32 savedEbx = cpu.ebx;
    setErrorAt(g, kTapiFile, 0x54db14, 0x1a9);
    g.setDword(kModems, reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54db28, kListBytes, g.dword(kMemoryGroup))));
    for (reg32 at = 0; at != kListBytes; at += 8)
    {
        g.setDword(g.dword(kModems) + at + 4, 0);
        g.setDword(g.dword(kModems) + at, 0);
    }
    const reg32 version = g.frame() + 4;
    for (reg32 device = 0; device < g.dword(kDeviceCount); ++device)
    {
        cpu.eax = version;  // eax as the code has it at the call, which the stub does not read
        if (fe::direct<0x53138f, frontend::watc::lineNegotiateAPIVersion>(app, cpu, g.dword(kLineApp), device, kApiLow,
                                                                          kApiLow, kApiVersion, version) != 0)
            continue;
        if (guest::direct<0x4f9f30, tapiGetDevCaps>(app, cpu, device) != 0 || (g.byte(g.dword(kDevCaps) + 0x3c) & kCapsDatamodem) == 0)
            continue;
        const reg32 caps = g.dword(kDevCaps);
        setErrorAt(g, kTapiFile, 0x54db14, 0x1c1);
        const reg32 name = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54db34, g.dword(caps + 0x20) + 1, g.dword(kMemoryGroup)));
        const reg32 place = g.dword(kModems) + (g.dword(kModemCount) << 3);
        g.setDword(place + 4, name);
        const reg32 caps2 = g.dword(kDevCaps);
        g.call(kStrncpy, args({ g.dword(place + 4), g.dword(caps2 + 0x24) + caps2, g.dword(caps2 + 0x20) }));
        const reg32 place2 = g.dword(kModems) + (g.dword(kModemCount) << 3);
        g.setByte(g.dword(place2 + 4) + g.dword(g.dword(kDevCaps) + 0x20), 0);
        g.setDword(place2, device);
        g.setDword(kModemCount, g.dword(kModemCount) + 1);
    }
    fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0x1f4);
    cpu.eax = g.dword(kLineApp);  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x5324d2, frontend::watc::lineShutdown>(app, cpu, g.dword(kLineApp)) != 0)
        fileReport(g, 0x54da70);
    cpu.ebx = savedEbx;
    return g.dword(kModemCount);
}

void frontend::tapiEnumerateModems(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9ac0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiEnumerateModems(app, cpu); });
}

/* sub_4f9d20: the chosen line opened, its comm's settings seen to when asked for. */
reg32 frontend::netb::tapiOpenLine(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x24);
    const reg32 device = g.dword(kChosenDevice), version = g.frame();
    if (device == 0xffffffff)
        return 0;
    cpu.eax = version;  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x53138f, frontend::watc::lineNegotiateAPIVersion>(app, cpu, g.dword(kLineApp), device, kApiLow,
                                                                      kApiLow, kApiVersion, version) != 0)
        return 0;
    if (guest::direct<0x4f9f30, tapiGetDevCaps>(app, cpu, g.dword(kChosenDevice)) != 0)
        return 0;
    const reg32 mode = g.dword(kAnswerMode) != 0 ? 4 : 1;
    const reg32 app_ = g.dword(kLineApp);
    cpu.eax = app_;  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x531439, frontend::watc::lineOpen>(app, cpu, app_, g.dword(kChosenDevice), kLine, g.dword(kApiVersion), 0, 0,
                                                       mode, 0x10, 0) != 0)
        return 0;
    if (g.dword(kCommConfig) != 0)
    {
        guest::direct<0x4f9ea0, tapiSetCommConfig>(app, cpu);
        const reg32 caps = g.dword(kDevCaps);
        cpu.eax = caps;  // eax as the code has it at the call, which the stub does not read
        fe::direct<0x532162, frontend::watc::lineSetStatusMessages>(app, cpu, g.dword(kLine), g.dword(caps + 0x80), 0);
    }
    return 1;
}

void frontend::tapiOpenLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9d20", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiOpenLine(app, cpu); });
}

/* sub_4f9df0: the line's comm port asked for, its DCB read into 0x9f3d60 and the handle closed. */
reg32 frontend::netb::tapiGetCommHandleConfig(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1f58);
    constexpr reg32 kBytes = 0x1f40, kGetCommConfig = 0x5344ec, kCloseHandle = 0x534488;
    const reg32 id = g.frame();
    g.call(kMemset, args({ id, 0, kBytes }));
    g.setDword(id, kBytes);
    cpu.eax = id;  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x530a4e, frontend::watc::lineGetID>(app, cpu, g.dword(kLine), 0, 0, 1, id, kCommClass) != 0)
        return 0;
    const reg32 handleAt = id + g.dword(id + 0x14);
    g.setWord(kCommDcb, 0x1000);
    g.setDword(kCommDcb + 0x34, 0xe73cf52e);
    const reg32 read = windows<kGetCommConfig, &win32::kernel32::GetCommProperties>(g, cpu, g.dword(handleAt), kCommDcb);
    windows<kCloseHandle, &win32::kernel32::CloseHandle>(g, cpu, g.dword(handleAt));
    if (read == 0)
        return 0;
    g.setDword(kCommConfig, kCommDcb + 0x3c);
    return 1;
}

void frontend::tapiGetCommHandleConfig(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9df0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiGetCommHandleConfig(app, cpu); });
}

/* sub_4f9ea0: the chosen device's configuration read, its low three bits of settings cleared,
 * and set again. */
reg32 frontend::netb::tapiSetCommConfig(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1f54);
    constexpr reg32 kBytes = 0x1f40;
    const reg32 config = g.frame() + 4;
    g.call(kMemset, args({ config, 0, kBytes }));
    g.setDword(config, kBytes);
    cpu.eax = config;  // eax as the code has it at the call, which the stub does not read
    if (fe::direct<0x53083d, frontend::watc::lineGetDevConfig>(app, cpu, g.dword(kChosenDevice), config, kCommClass) != 0
        || g.dword(config + 4) > g.dword(config))
        return 0;
    const reg32 data = config + g.dword(config + 0x14);
    const reg32 settings = data + 0x3c + 0x20;
    g.setByte(settings, std::uint8_t(g.byte(settings) & 0xf8));
    cpu.eax = data;  // eax as the code has it at the call, which the stub does not read
    const reg32 result = fe::direct<0x531f31, frontend::watc::lineSetDevConfig>(app, cpu, g.dword(kChosenDevice), data,
                                                                         g.dword(config + 0x10), kCommClass);
    return result != 0 ? 1 : 0;
}

void frontend::tapiSetCommConfig(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9ea0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiSetCommConfig(app, cpu); });
}

/* sub_4f9f30: TapiGetDevCaps: a device's capabilities in [0x9f4d88], the block made as large as
 * it says it needs; TAPI's error, or 0. */
reg32 frontend::netb::tapiGetDevCaps(App* app, CPU& cpu, reg32 device)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    const auto drop = [&] {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, g.dword(kDevCaps));
        g.setDword(kDevCaps, 0);
    };
    if (g.dword(kDevCaps) != 0)
        drop();
    setErrorAt(g, kTapiFile, 0x54db50, 0x25a);
    reg32 size = 0xf0;
    reg32 block = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54db60, size, g.dword(kMemoryGroup)));
    g.setDword(kDevCaps, block);
    g.setDword(block, size);
    for (;;)
    {
        const reg32 caps = g.dword(kDevCaps);
        cpu.eax = caps;  // eax as the code has it at the call, which the stub does not read
        const reg32 error = fe::direct<0x530747, frontend::watc::lineGetDevCaps>(app, cpu, g.dword(kLineApp), device,
                                                                         g.dword(kApiVersion), 0, caps);
        if (error != 0)
        {
            drop();
            return error;
        }
        size = g.dword(g.dword(kDevCaps) + 4);
        if (size <= g.dword(g.dword(kDevCaps)))
            return 0;
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, g.dword(kDevCaps));
        setErrorAt(g, kTapiFile, 0x54db50, 0x26f);
        block = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54db60, size, g.dword(kMemoryGroup)));
        g.setDword(kDevCaps, block);
        g.setDword(block, size);
    }
}

void frontend::tapiGetDevCaps(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f9f30", 0, 0, nullptr, 0 };
    const reg32 device = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiGetDevCaps(app, cpu, device); });
}

/* sub_4fa030: TapiMakeCall: a number dialled on the open line. */
reg32 frontend::netb::tapiMakeCall(App* app, CPU& cpu, reg32 number)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    if (number == 0 || g.dword(kLine) == 0)
        return 0;
    setErrorAt(g, kTapiFile, 0x54db70, 0x287);
    const reg32 parameters = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54db80, 0x70, g.dword(kMemoryGroup)));
    g.call(kMemset, args({ parameters, 0, 0x70 }));
    g.setDword(parameters, 0x70);
    g.setDword(parameters + 4, 1);
    g.setDword(parameters + 0x10, 0x10);
    g.setDword(parameters + 0x2c, 0xbb8);
    const reg32 result = fe::direct<0x5311c0, frontend::watc::lineMakeCall>(app, cpu, g.dword(kLine), kCall, number, 0, parameters);
    cpu.ecx = result;
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, parameters);
    if (sreg32(result) < 0)
    {
        g.setDword(kModemState, 0xf);
        return 0;
    }
    return 1;
}

void frontend::tapiMakeCall(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa030", 0, 0, nullptr, 0 };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiMakeCall(app, cpu, number); });
}

/* sub_4fa0f0: TapiGetModemHandle: the comm handle of the call in [0x9f4da8]; 0, or -1. */
reg32 frontend::netb::tapiGetModemHandle(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x18);
    setErrorAt(g, kTapiFile, 0x54db94, 0x2aa);
    reg32 block = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54dba8, 0x18, g.dword(kMemoryGroup)));
    g.setDword(block, 0x18);
    for (;;)
    {
        cpu.eax = block;  // eax as the code has it at the call, which the stub does not read
        if (fe::direct<0x530a4e, frontend::watc::lineGetID>(app, cpu, 0, 0, g.dword(kCall), 4, block, kCommClass) != 0)
        {
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
            return 0xffffffff;
        }
        const reg32 needed = g.dword(block + 4);
        if (needed <= g.dword(block))
        {
            g.setDword(kModemHandle, g.dword(block + g.dword(block + 0x14)));
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
            return 0;
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
        setErrorAt(g, kTapiFile, 0x54db94, 0x2bd);
        block = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54dba8, needed, g.dword(kMemoryGroup)));
        g.setDword(block, needed);
    }
}

void frontend::tapiGetModemHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa0f0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiGetModemHandle(app, cpu); });
}

/* sub_4fa1d0: the call dropped. */
reg32 frontend::netb::tapiHangUp(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    if (g.dword(kCall) != 0)
    {
        constexpr reg32 kDropText = 0x54dbb8;
        cpu.ebx = kDropText;  // the report's text, kept in ebx over lineDrop
        fe::direct<0x52fb2e, frontend::watc::lineDrop>(app, cpu, g.dword(kCall), 0, 0);
        fileReport(g, kDropText);
    }
    return cpu.eax;
}

void frontend::tapiHangUp(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa1d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiHangUp(app, cpu); });
}

/* sub_4fa210: the modem's thread: the modem chosen, TAPI set up, the number dialled, and
 * Windows' messages pumped until told to stop; then the line hung up. */
reg32 frontend::netb::tapiThread(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x20);
    constexpr reg32 kGetMessage = 0x53474c, kTranslateMessage = 0x5347a4, kDispatchMessage = 0x53472c;
    const reg32 message = g.frame();
    if (guest::direct<0x4f9a50, tapiSelectModem>(app, cpu, g.dword(g.dword(kModemSettings) + 0x3c)) == 0)
    {
        g.setDword(kModemState, 0x80000002);
        g.setDword(kModemSettings, 0);
        return 0;
    }
    const reg32 state = fe::direct<0x4f9920, frontend::wata::tapiSetup>(
        app, cpu, reg32(g.dword(g.dword(kModemSettings) + 0x68) == 0 ? 1 : 0));
    g.setDword(kModemState, state);
    if (g.dword(kModemState) == 0)
    {
        g.setDword(kModemState, 2);
        const reg32 number = g.dword(g.dword(kModemSettings) + 0x68);
        const reg32 up = number == 0 || guest::direct<0x4fa030, tapiMakeCall>(app, cpu, number) != 0 ? 1 : 0;
        const reg32 edx = cpu.edx, ecx = cpu.ecx, ebx = cpu.ebx;
        cpu.esp -= 0xc;  // they are pushed from here on
        g.setDword(g.dword(kModemSettings) + 0x34, up);
        g.setDword(kThreadStarting, 0);
        cpu.ebx = 0;
        while (g.dword(g.dword(kModemSettings) + 0x34) != 0)
        {
            if (windows<kGetMessage, &win32::user32::GetMessageA>(g, cpu, message, 0, 0, 0) == 0)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
            }
            else
            {
                windows<kTranslateMessage, &win32::user32::TranslateMessage>(g, cpu, message);
                windows<kDispatchMessage, &win32::user32::DispatchMessageA>(g, cpu, message);
            }
        }
        guest::direct<0x4fa1d0, tapiHangUp>(app, cpu);
        cpu.esp += 0xc;
        cpu.edx = edx, cpu.ecx = ecx, cpu.ebx = ebx;
    }
    return fe::direct<0x4f97c0, frontend::eab::lineShutdown>(app, cpu);
}

void frontend::tapiThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa210", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tapiThread(app, cpu); });
}

/* sub_4fa310: a modem connection opened: the thread started and waited for, then the object's
 * function (+0x14) until it says stop or the thread has done; the port opened on the handle. */
reg32 frontend::netb::modemOpen(App* app, CPU& cpu, reg32 settings, reg32 callback)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    g.setDword(kModemSettings, settings);
    g.setDword(kModemCallback, callback);
    g.setDword(kModemState, 0);
    g.setDword(settings + 0x38, 0xffffffff);
    g.setDword(settings + 0x34, 0);
    g.setDword(kThreadStarting, 1);
    const auto oneSecond = [&] {
        const sreg32 ticks = sreg32(g.dword(kTicksPerSecond));
        cpu.edx = reg32(sreg32(0x3e8) % ticks);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, reg32(sreg32(0x3e8) / ticks));
    };
    if (g.call(kCreateThread, args({ 0x4fa210, 0, 1, 0xffffffff }, { 0x9f4d60 })) == 0)
    {
        g.setDword(kModemState, 1);
    }
    else
    {
        while (g.dword(kThreadStarting) != 0)
            oneSecond();
        while (g.dword(kThreadStarting) == 0)
        {
            if (g.call(g.dword(settings + 0x14), args({ settings })) != 0)
                break;
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
            {
                if (sreg32(reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
            }
            oneSecond();
        }
        if (g.dword(kThreadStarting) == 0 || g.dword(kModemState) != kStateConnected)
            postQuit(g, cpu, settings);
    }
    if (g.dword(settings + 0x34) != 0 && g.dword(kModemState) == kStateConnected && g.dword(kModemHandle) != 0)
        g.setDword(settings + 0x38, fe::direct<0x51b2f0, frontend::netc::serialOpenHandle>(app, cpu, 0xffffffff,
                                                                                            g.dword(kModemHandle)));
    return g.dword(settings + 0x34);
}

void frontend::modemOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa310", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 settings = cpu.eax, callback = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemOpen(app, cpu, settings, callback); });
}

/* sub_4fa450: a modem connection closed: the thread told. */
reg32 frontend::netb::modemClose(App* app, CPU& cpu, reg32 settings)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 8);
    postQuit(g, cpu, settings);
    return 1;
}

void frontend::modemClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa450", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const reg32 settings = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemClose(app, cpu, settings); });
}

/* sub_4fa480: a modem connection polled: told to close unless connected; the callback with
 * what has come; whether it is still up. */
reg32 frontend::netb::modemPoll(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    cpu.edx = object;
    cpu.ebx = object;
    if (g.dword(kModemState) != kStateConnected)
    {
        const reg32 ecx = cpu.ecx;
        postQuit(g, cpu, object);
        cpu.ecx = ecx;
    }
    else if (g.dword(kModemCallback) != 0 && guest::direct<0x4fa4f0, modemReceiveReady>(app, cpu, object) != 0)
    {
        g.call(g.dword(kModemCallback), args({ object }));
    }
    return g.dword(object + 0x34);
}

void frontend::modemPoll(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa480", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx | kRestoreEdi };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemPoll(app, cpu, object); });
}

/* sub_4fa4e0: whether the modem's port will take more. */
reg32 frontend::netb::modemSendReady(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    return fe::direct<0x51b900, frontend::netc::serialOutRoom>(app, cpu, g.dword(object + 0x38));
}

void frontend::modemSendReady(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa4e0", 0, 0, nullptr, 0, 0 };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemSendReady(app, cpu, object); });
}

/* sub_4fa4f0: whether the modem's port has something. */
reg32 frontend::netb::modemReceiveReady(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    return fe::direct<0x51b8a0, frontend::netc::serialInCount>(app, cpu, g.dword(object + 0x38));
}

void frontend::modemReceiveReady(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa4f0", 0, 0, nullptr, 0, 0 };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemReceiveReady(app, cpu, object); });
}

/* sub_4fa500: bytes sent over the modem. */
reg32 frontend::netb::modemSend(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    return portSend(g, cpu, object, length, buffer);
}

void frontend::modemSend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa500", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 object = cpu.eax, length = cpu.edx, buffer = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemSend(app, cpu, object, length, buffer); });
}

/* sub_4fa530: bytes received over the modem. */
reg32 frontend::netb::modemReceive(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    return portReceive(g, cpu, object, length, buffer);
}

void frontend::modemReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa530", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 object = cpu.eax, length = cpu.edx, buffer = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemReceive(app, cpu, object, length, buffer); });
}

/* sub_4fa550: a network over the modem: sub_4f5130 with this kind's table at 0x5671b0. */
reg32 frontend::netb::modemOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 second, reg32 mode)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    const reg32 ticks = g.dword(kTicksPerSecond);
    fillTimeouts(g, settings, ticks, 4, 6);
    g.setDword(kTicksPerSecond, ticks);
    return guest::direct<0x4f5130, neta::serviceCall>(app, cpu, settings,
                                                      mode == 0xffffffff ? reg32(0x56ac10) : reg32(0x56ac28), 0x5671b0,
                                                      second, mode);
}

void frontend::modemOpenNetwork(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa550", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 settings = cpu.eax, second = cpu.edx, mode = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemOpenNetwork(app, cpu, settings, second, mode); });
}

/* sub_4fa5f0: a modem's name by its index, 0 for none. */
reg32 frontend::netb::modemName(App* app, CPU& cpu, reg32 index)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    if (g.dword(kModems) == 0)
        guest::direct<0x4f9ac0, tapiEnumerateModems>(app, cpu);
    if (sreg32(index) < 0 || index >= g.dword(kModemCount))
        return 0;
    return g.dword(g.dword(kModems) + index * 8 + 4);
}

void frontend::modemName(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa5f0", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 index = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::modemName(app, cpu, index); });
}

/* sub_4fa620: a connection over a serial port opened, with the speed and so on in its settings. */
reg32 frontend::netb::serialOpen(App* app, CPU& cpu, reg32 settings, reg32 callback)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x10);
    constexpr reg32 kPortOffset = 0x38, kSerialTrace = 0x566dec, kSerialFlag = 0x564458;
    const auto port = [&] { return g.dword(settings + kPortOffset); };
    g.setDword(kSerialCallback, callback);
    g.setDword(settings + 0x34, 0);
    cpu.edx = 0;
    g.setDword(kSerialFlag, 0);
    fe::direct<0x51b480, frontend::netc::serialOpen>(app, cpu, port());
    if (fe::direct<0x51b030, frontend::netc::serialIsOpen>(app, cpu, port()) == 0)
    {
        if (sreg32(g.dword(kSerialTrace)) >= 1)
            guest::direct<0x512180, commEvent>(app, cpu, 0x54dbd8, settings + kPortOffset, 0, 0, 1, 0);
        return g.dword(settings + 0x34);
    }
    fe::direct<0x51b710, frontend::netc::serialSet>(app, cpu, port(), g.dword(settings + 0x40), g.dword(settings + 0x44),
                                                    g.dword(settings + 0x48), g.dword(settings + 0x4c));
    fe::direct<0x51b1c0, frontend::netc::serialDropInput>(app, cpu, port());
    fe::direct<0x51b1e0, frontend::netc::serialDropOutput>(app, cpu, port());
    if (fe::direct<0x51a930, frontend::netc::modemAttention>(app, cpu, port()) != 0)
    {
        fe::direct<0x51b200, frontend::netc::serialClose>(app, cpu, port());
        return 0;
    }
    g.setDword(settings + 0x34, 1);
    return g.dword(settings + 0x34);
}

void frontend::serialOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa620", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 settings = cpu.eax, callback = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialOpen(app, cpu, settings, callback); });
}

/* sub_4fa6d0: a serial connection closed: nothing but that it is. */
reg32 frontend::netb::serialClose(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    g.setDword(object + 0x34, 0);
    return 1;
}

void frontend::serialClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa6d0", 0, 0, nullptr, 0 };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialClose(app, cpu, object); });
}

/* sub_4fa6e0: a serial connection polled: the callback with what has come; whether it is up. */
reg32 frontend::netb::serialPoll(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    cpu.edx = object;
    cpu.ecx = object;
    cpu.ebx = g.dword(kSerialCallback);
    if (g.dword(kSerialCallback) != 0 && guest::direct<0x4fa720, serialReceiveReady>(app, cpu, object) != 0)
        g.call(g.dword(kSerialCallback), args({ object }));
    return g.dword(object + 0x34);
}

void frontend::serialPoll(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa6e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialPoll(app, cpu, object); });
}

/* sub_4fa710: whether the serial port will take more. */
reg32 frontend::netb::serialSendReady(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    return fe::direct<0x51b900, frontend::netc::serialOutRoom>(app, cpu, g.dword(object + 0x38));
}

void frontend::serialSendReady(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa710", 0, 0, nullptr, 0, 0 };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialSendReady(app, cpu, object); });
}

/* sub_4fa720: whether the serial port has something. */
reg32 frontend::netb::serialReceiveReady(App* app, CPU& cpu, reg32 object)
{
    Guest g(app, cpu);
    return fe::direct<0x51b8a0, frontend::netc::serialInCount>(app, cpu, g.dword(object + 0x38));
}

void frontend::serialReceiveReady(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa720", 0, 0, nullptr, 0, 0 };
    const reg32 object = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialReceiveReady(app, cpu, object); });
}

/* sub_4fa730: bytes sent over the serial port. */
reg32 frontend::netb::serialSend(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    return portSend(g, cpu, object, length, buffer);
}

void frontend::serialSend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa730", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 object = cpu.eax, length = cpu.edx, buffer = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialSend(app, cpu, object, length, buffer); });
}

/* sub_4fa760: bytes received over the serial port. */
reg32 frontend::netb::serialReceive(App* app, CPU& cpu, reg32 object, reg32 length, reg32 buffer)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    return portReceive(g, cpu, object, length, buffer);
}

void frontend::serialReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa760", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 object = cpu.eax, length = cpu.edx, buffer = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialReceive(app, cpu, object, length, buffer); });
}

/* sub_4fa780: a network over a serial port: sub_4f5130 with this kind's table at 0x5671d0. */
reg32 frontend::netb::serialOpenNetwork(App* app, CPU& cpu, reg32 settings, reg32 second, reg32 mode)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    const reg32 ticks = g.dword(kTicksPerSecond);
    fillTimeouts(g, settings, ticks, 10, 3);
    g.setDword(kTicksPerSecond, ticks);
    return guest::direct<0x4f5130, neta::serviceCall>(app, cpu, settings,
                                                      mode == 0xffffffff ? reg32(0x56ac10) : reg32(0x56ac28), 0x5671d0,
                                                      second, mode);
}

void frontend::serialOpenNetwork(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4fa780", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 settings = cpu.eax, second = cpu.edx, mode = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::serialOpenNetwork(app, cpu, settings, second, mode); });
}

/* ---------------------------------------------------------------------------
 * win\ipx: games looked for, advertised by a thread over a TCP/IP socket of IPX's family
 * ------------------------------------------------------------------------- */
namespace
{

constexpr reg32 kAdvertRunning = 0xa0b0b8, kAdvertDone = 0xa0b0b4, kAdvertSocket = 0xa0b0bc, kAdvertPacket = 0xa0b0d2;
constexpr reg32 kIpxSocket = 0x452, kGameProtocol = 0x8be;

}  // namespace

/* sub_5127d0: a broadcast asking for games sent on a socket: IPX's address, all ones. */
reg32 frontend::netb::ipxBroadcastOpen(App* app, CPU& cpu, reg32 socket)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x24);
    const reg32 address = g.frame(), packet = address + 0x10;
    g.setWord(address, 6);
    cpu.edx = 6;  // edx and ebx as the code has them at the call
    cpu.ebx = 0xff;
    const reg32 port = thunked<kHtons, &win32::wsock32::htons>(g, cpu, kIpxSocket);
    g.setWord(address + 0xc, std::uint16_t(port));
    g.call(0x4e0716, args({ address + 2, 4, 0xff }));
    g.call(0x4e0716, args({ address + 6, 6, 0xff }));
    g.setWord(packet, std::uint16_t(swapped(1) >> 16));
    g.setWord(packet + 2, std::uint16_t(swapped(kGameProtocol) >> 16));
    enterLock(g, cpu, g.dword(kNetLock));
    const reg32 sent = thunked<kSendTo, &win32::wsock32::sendto>(g, cpu, socket, packet, 4, 0, address, 0xe);
    leaveLock(g, cpu, g.dword(kNetLock));
    return sent != 0xffffffff ? 1 : 0;
}

void frontend::ipxBroadcastOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5127d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 socket = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxBroadcastOpen(app, cpu, socket); });
}

/* sub_512890: a list of games read from a packet: 64 bytes each after two of a version; those of
 * this game handed to sub_4f5a30.  1 when the packet was such a list. */
reg32 frontend::netb::ipxScanInterfaces(App* app, CPU& cpu, reg32 owner, reg32 header, reg32 data, reg32 size)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x14);
    constexpr reg32 kEntry = 0x40;
    const reg32 port = thunked<kHtons, &win32::wsock32::htons>(g, cpu, kIpxSocket);
    if (std::uint16_t(port) != g.word(header + 0xc))
        return 0;
    const reg32 version = bigEndian(g, cpu, data, 2);
    if (version != 2)
        return 0;
    reg32 left = (size - version) >> 6;
    reg32 entry = data + version;
    for (;;)
    {
        --left;
        cpu.edx = left;
        if (left == 0xffffffff)
            return 1;
        if (bigEndian(g, cpu, entry, 2) == kGameProtocol)
            guest::direct<0x4f5a30, neta::ipxAnswerName>(app, cpu, owner, entry + 0x32, entry + 2);
        entry += kEntry;
    }
}

void frontend::ipxScanInterfaces(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512890", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 owner = cpu.eax, header = cpu.edx, data = cpu.ebx, size = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxScanInterfaces(app, cpu, owner, header, data, size); });
}

/* sub_512940: the game's advertisement sent to an address. */
reg32 frontend::netb::ipxAdvertiseTo(App* app, CPU& cpu, reg32 to)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    enterLock(g, cpu, g.dword(kNetLock));
    thunked<kSendTo, &win32::wsock32::sendto>(g, cpu, g.dword(kAdvertSocket), kAdvertPacket, 0x42, 0, to, 0xe);
    leaveLock(g, cpu, g.dword(kNetLock));
    return cpu.eax;
}

void frontend::ipxAdvertiseTo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512940", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 to = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxAdvertiseTo(app, cpu, to); });
}

/* sub_512980: whether a packet asks for games: type 1, this game's protocol or any. */
reg32 frontend::netb::ipxIsRequest(App* app, CPU& cpu, reg32 data)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 8);
    cpu.edx = bigEndian(g, cpu, data, 2);
    const reg32 protocol = bigEndian(g, cpu, data + 2, 2);
    return cpu.edx == 1 && (protocol == kGameProtocol || protocol == 0xffff) ? 1 : 0;
}

void frontend::ipxIsRequest(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512980", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const reg32 data = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxIsRequest(app, cpu, data); });
}

/* sub_5129e0: an IPX socket for the advertising thread, broadcast and larger buffers on, bound
 * at the game's port; -1 for none. */
reg32 frontend::netb::ipxServerSocket(App* app, CPU& cpu, reg32 address)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1c);
    constexpr reg32 kSoBroadcast = 0x20, kSolSocket = 0xffff, kNsProto = 0x3e8, kOption = 0x400f, kAddressBytes = 0xe;
    const reg32 on = g.frame(), length = on + 4;
    const reg32 socket = thunked<kSocket, &win32::wsock32::socket>(g, cpu, 6, 2, kNsProto);
    if (socket == 0xffffffff)
        return 0xffffffff;
    g.setDword(on, 1);
    thunked<kSetSockOpt, &win32::wsock32::setsockopt>(g, cpu, socket, kSolSocket, kSoBroadcast, on, 4);
    thunked<kSetSockOpt, &win32::wsock32::setsockopt>(g, cpu, socket, kSolSocket, kOption, on, 4);
    thunked<kSetSockOpt, &win32::wsock32::setsockopt>(g, cpu, socket, kNsProto, kOption, on, 4);
    g.call(0x4e070c, args({ address, kAddressBytes }));
    g.setWord(address, 6);
    const reg32 port = thunked<kHtons, &win32::wsock32::htons>(g, cpu, kIpxSocket);
    g.setWord(address + 0xc, std::uint16_t(port));
    if (thunked<kBind, &win32::wsock32::bind>(g, cpu, socket, address, kAddressBytes) == 0)
    {
        g.setDword(length, kAddressBytes);
        if (thunked<kGetSockName, &win32::wsock32::getsockname>(g, cpu, socket, address, length) == 0
            && g.dword(length) == kAddressBytes)
            return socket;
    }
    thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, socket);
    return 0xffffffff;
}

void frontend::ipxServerSocket(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5129e0", 0, 0, nullptr, 0 };
    const reg32 address = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxServerSocket(app, cpu, address); });
}

/* sub_512ad0: the advertising thread: it answers requests that come, and sends the game to all
 * every minute; one last time, with the game gone, if it ever did. */
reg32 frontend::netb::ipxServerThread(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x348);
    const reg32 frame = g.frame(), data = frame, set = frame + 0x200, from = frame + 0x304;
    const reg32 everyone = frame + 0x314, wait = frame + 0x324, fromLength = frame + 0x32c;
    // The thread ends with eax as its last call leaves it: each call's result made eax.
    const auto ticks = [&] { return cpu.eax = fe::direct<0x4f2790, frontend::eab::lowTimerTicks>(app, cpu); };
    const auto advertise = [&](reg32 to) { cpu.eax = guest::direct<0x512940, ipxAdvertiseTo>(app, cpu, to); };
    reg32 next = ticks() + 4;
    g.setWord(everyone, 6);
    const reg32 port = thunked<kHtons, &win32::wsock32::htons>(g, cpu, kIpxSocket);
    guest::direct<0x4f5900, neta::portToSockaddr>(app, cpu, reg32(sreg32(std::int16_t(port))), everyone);
    g.setDword(kAdvertRunning, 1);
    reg32 sentAny = 0;
    do
    {
        g.setDword(set + 4, g.dword(kAdvertSocket));
        g.setDword(wait, reg32(sreg32(next) / 1000));
        g.setDword(wait + 4, reg32((sreg32(next) % 1000) * 1000));
        g.setDword(set, 1);
        const reg32 ready = thunked<kSelect, &win32::wsock32::select>(g, cpu, 0, set, 0, 0, wait);
        if (ready == 0xffffffff)
        {
            cpu.ebx = 0;
            g.setDword(kAdvertRunning, 0);
            break;
        }
        if (ready != 0)
        {
            g.setDword(fromLength, 0x10);
            enterLock(g, cpu, g.dword(kNetLock));
            const reg32 got = thunked<kRecvFrom, &win32::wsock32::recvfrom>(g, cpu, g.dword(kAdvertSocket), data,
                                                                            0x200, 0, from, fromLength);
            leaveLock(g, cpu, g.dword(kNetLock));
            // 4 in edx too, which it does not read.
            if (got == 4 && (cpu.eax = guest::direct<0x512980, ipxIsRequest>(app, cpu, data)) != 0)
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
    return cpu.eax;
}

void frontend::ipxServerThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512ad0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxServerThread(app, cpu); });
}

/* sub_512c80: the game advertised: the socket, the packet of its name and id, and the thread,
 * waited for. */
reg32 frontend::netb::ipxStartServer(App* app, CPU& cpu, reg32 owner, reg32 id, reg32 name)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    cpu.edx = name;
    if (g.dword(kAdvertRunning) != 0)
        return g.dword(kAdvertRunning);
    const reg32 socket = guest::direct<0x5129e0, ipxServerSocket>(app, cpu, 0xa0b0c4);
    g.setDword(kAdvertSocket, socket);
    if (socket == 0xffffffff)
        return g.dword(kAdvertRunning);
    g.setDword(0xa0b0c0, owner);
    g.setWord(kAdvertPacket, std::uint16_t(swapped(2) >> 16));
    g.setWord(0xa0b0d4, std::uint16_t(swapped(kGameProtocol) >> 16));
    g.setWord(0xa0b112, std::uint16_t(swapped(1) >> 16));
    g.call(kStrncpy, args({ 0xa0b0d6, name, 0x30, 0xffffffff }));
    copyDwords(g, 0xa0b106, id, 3);
    g.setDword(kAdvertDone, 0);
    if (g.call(kCreateThread, args({ 0x512ad0, 0, 1 }, { 0xa0b0a0 })) != 0)
    {
        cpu.edx = 0;
        while (g.dword(kAdvertRunning) == 0 && g.dword(kAdvertDone) == 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
    }
    if (g.dword(kAdvertDone) != 0)
        thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, g.dword(kAdvertSocket));
    return g.dword(kAdvertRunning);
}

void frontend::ipxStartServer(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512c80", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 owner = cpu.eax, id = cpu.edx, name = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxStartServer(app, cpu, owner, id, name); });
}

/* sub_512d70: the advertising stopped: the socket closed and the thread waited for. */
reg32 frontend::netb::ipxStopServer(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0xc);
    if (g.dword(kAdvertRunning) != 0)
    {
        const reg32 socket = g.dword(kAdvertSocket);
        g.setDword(kAdvertRunning, 0);
        thunked<kCloseSocket, &win32::wsock32::closesocket>(g, cpu, socket);
        while (g.dword(kAdvertDone) == 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
    }
    return 1;
}

void frontend::ipxStopServer(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512d70", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::ipxStopServer(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * Sessions
 * ------------------------------------------------------------------------- */

/* sub_512f60: a game's GUID as the text that names its shared memory. */
reg32 frontend::netb::gameGuidText(App* app, CPU& cpu, reg32 guid, reg32 text)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 4);
    return g.call(0x4df690, args({}, { text, 0x55022c, g.dword(guid), g.word(guid + 4), g.word(guid + 6),
                                       g.byte(guid + 8), g.byte(guid + 9), g.byte(guid + 0xa), g.byte(guid + 0xb),
                                       g.byte(guid + 0xc), g.byte(guid + 0xd), g.byte(guid + 0xe), g.byte(guid + 0xf) }));
}

void frontend::gameGuidText(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512f60", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 guid = cpu.eax, text = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::gameGuidText(app, cpu, guid, text); });
}

/* sub_512fc0: when an answer stops being waited for: eax ticks from now. */
reg32 frontend::netb::setResponseTimeout(App* app, CPU& cpu, reg32 ticks)
{
    Guest g(app, cpu);
    const reg32 until = ticks + g.dword(0x79c1a0);
    g.setDword(0x5689e0, until);
    return until;
}

void frontend::setResponseTimeout(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512fc0", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 ticks = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::setResponseTimeout(app, cpu, ticks); });
}

/* sub_512fd0: whether that time has come. */
reg32 frontend::netb::responseTimedOut(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return sreg32(g.dword(0x79c1a0) - g.dword(0x5689e0)) < 0 ? 0 : 1;
}

void frontend::responseTimedOut(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512fd0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::responseTimedOut(app, cpu); });
}

/* sub_512ff0: what a table holds for a function that always says no. */
reg32 frontend::netb::returnZeroToo(App* app, CPU& cpu)
{
    return 0;
}

void frontend::netReturnZeroToo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_512ff0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::returnZeroToo(app, cpu); });
}

/* sub_513190: a request sent to the session's server and waited for, sent again each time round:
 * eax the game, edx the session, ebx the type, ecx the message, its first dword the new id; the
 * stack its length and how long to wait. */
reg32 frontend::netb::sendServerRequest(App* app, CPU& cpu, reg32 game, reg32 session, reg32 type, reg32 message, reg32 length, reg32 patience)
{
    Guest g(app, cpu);
    const fe::Frame locals(g, cpu, 0x1c);
    constexpr reg32 kNextId = 0x5689dc, kPatience = 0x566ddc;
    const reg32 frame = g.frame();
    g.setDword(frame, type);
    g.setDword(session + 4, 0);
    g.setDword(session + 0x10, game);
    const reg32 id = g.dword(kNextId);
    g.setDword(session, id);
    g.setDword(kNextId, id + 1);
    g.setDword(message, swapped(id));
    cpu.edx = g.dword(kTraceLevel);
    if (sreg32(cpu.edx) >= 3)
        logEventLeaving(g, 0x550270, frame, message, length);
    const reg32 functions = g.dword(game + 8);
    if (g.dword(functions + 0x48) != 0)
        g.call(g.dword(functions + 0x48), args({ g.dword(game + 0xc), game + 0x1c }));
    g.setDword(game + 0x18, session);
    guest::direct<0x512fc0, setResponseTimeout>(app, cpu, patience);
    cpu.eax = session;
    if (g.dword(session + 4) == 0)
    {
        reg32 delay = 1;
        for (;;)
        {
            const reg32 timedOut = guest::direct<0x512fd0, responseTimedOut>(app, cpu);
            if (timedOut != 0)
            {
                cpu.eax = timedOut;  // the code returns its eax
                break;
            }
            const reg32 sendTo = g.dword(game + 8);
            if (g.call(g.dword(sendTo + kNetSend), args({ g.dword(game + 0xc), game + 0x1c, type, message }, { length })) != 0)
                delay = g.dword(kPatience);
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
            {
                if (sreg32(g.call(kWaitSystemTasks, args({ delay }))) < 0)
                    break;
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, delay);
            }
            cpu.eax = session;
            if (g.dword(session + 4) != 0)
                break;
        }
    }
    g.setDword(game + 0x18, 0);
    return cpu.eax;
}

void frontend::sendServerRequest(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_513190", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 game = cpu.eax;
    const reg32 session = cpu.edx;
    const reg32 type = cpu.ebx;
    const reg32 message = cpu.ecx;
    const reg32 length = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 patience = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::sendServerRequest(app, cpu, game, session, type, message, length, patience); });
}

/* sub_4f6860: nothing to do for an exception on a socket; it counts as handled. */
reg32 frontend::netb::tcpExceptionSeen(App* app, CPU& cpu)
{
    return 1;
}

void frontend::tcpExceptionSeen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4f6860", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netb::tcpExceptionSeen(app, cpu); });
}

}
