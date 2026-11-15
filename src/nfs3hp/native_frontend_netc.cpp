#include "native_frontend_netc.h"
#include "native_ea_files.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_netb.h"
#include "native_frontend_wata.h"
#include "native_frontend_wina.h"
#include "native_windows.h"
#include "host_threads.h"
#include "host_net.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include "host_files.h"
#include "host_system.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax (as
 * tools/frontend_checks/specs_from_exe.py reads it): the registers left as the call by address
 * leaves them -- `words` in eax, edx, ebx and ecx as that call loads them, the first `Taken` of
 * them the core's arguments -- and its result in eax, which the call by address leaves there. */
template <x86::reg32 Address, auto Core, std::size_t... I>
auto directOf(win32::WinApplication* app, x86::CPU& cpu, const x86::reg32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, cpu, words[I]...);
}

template <x86::reg32 Address, auto Core, std::size_t Taken, typename... Words>
x86::reg32 directKept(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    const x86::reg32 all[] = { x86::reg32(words)..., 0 };
    x86::reg32* const into[] = { &cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 0; i < sizeof...(Words) && i < 4; ++i)
        *into[i] = all[i];
    const auto result = directOf<Address, Core>(app, cpu, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, x86::reg32>)
        cpu.eax = result;
    else
        cpu.eax = *result;
    return cpu.eax;
}

}  // namespace
}  // namespace nfs3hp


/* The last of EA's network library, native whole: the gathering's server side
 * (cmn\netgathr.c), the serial port and its thread (win\comm.c) with the
 * modem's talk over it, and the two packet layers over a stream --
 * cmn\unrely.c and cmn\packet.c, which numbers and resends besides.
 *
 * Everything stays in guest memory, where the rest of the library reads it,
 * and the calling convention is the game's: a transport's functions and a
 * connection's are reached through the tables and pointers they are kept in.
 * Windows' functions are the port's own, called as C++ (in the checks through
 * the imports' slots).  A register the original leaves changed is left the
 * same.
 *
 * Plan step 5: each function a typed core in frontend::netc (native_frontend_netc.h) -- its
 * arguments put in the registers the game's function takes them in, for the code that reads them
 * there, then the code as it was, in a frame of its own (fe::Frame: what it calls runs at the
 * stack addresses it did) -- and an adapter under the game's name for the calls by address.  A
 * core of this file calls another as C++ (guest::direct) where nothing after it, nor the
 * function's end, meets a register the call by address would leave otherwise; the typed cores of
 * other files likewise (fe::direct); the rest of the game, the tables' and pointers' calls and
 * those that hand registers back, by address. */
namespace nfs3hp
{

/* The port's trace of what is sent (native_vertices.cpp), before sub_51d810. */
bool commSendTrace(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

using fe::args;
using fe::Guest;
using fe::Site;
using x86::reg32;

constexpr std::uint8_t kEbx = fe::kRestoreEbx, kEcx = fe::kRestoreEcx, kEdx = fe::kRestoreEdx;
constexpr std::uint8_t kEsi = fe::kRestoreEsi, kEdi = fe::kRestoreEdi, kAll = fe::kRestoreAll;

std::int32_t sign(reg32 value)
{
    return std::int32_t(value);
}

reg32 byteSwapped(reg32 value)
{
    return (value >> 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}

// The imports' slots.
constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588, kLoadLibrary = 0x53458c, kGetProcAddress = 0x534558;
constexpr reg32 kWriteFile = 0x534640, kReadFile = 0x5345b0, kGetLastError = 0x534534, kOverlappedResult = 0x534554;
constexpr reg32 kPurgeComm = 0x5345a8, kCloseHandle = 0x534488, kClearError = 0x5345f4, kCreateEventApi = 0x534494;
constexpr reg32 kSetTimeouts = 0x5345c8, kCreateFile = 0x534498, kGetCommState = 0x5344f0, kSetCommState = 0x5345c4;

// The game's functions, by address.
constexpr reg32 kReportError = 0x401010, kSprintf = 0x4df690, kCreateThread = 0x4df7a0;
constexpr reg32 kPollEvent = 0x4dfb40;
constexpr reg32 kWaitForAny = 0x4dfbf0, kCloseEvent = 0x4dfc00, kFillZero = 0x4e070c, kStrstr = 0x4e0820;
constexpr reg32 kMonoPrintf = 0x4e08f0, kStrncpy = 0x4e0e30, kAllocate = 0x4e1620;
constexpr reg32 kWaitSystemTasks = 0x4e7720, kMoveMemory = 0x4ea4f0;
constexpr reg32 kAtoi = 0x4f0430;
constexpr reg32 kEndGame = 0x4f4f90, kCheckPlayer = 0x4f5040;
constexpr reg32 kOpenConnection = 0x4f5130, kFileReport = 0x501050, kQueueFree = 0x50dde0;
constexpr reg32 kQueueAdd = 0x50de50, kQueueAddSorted = 0x50e0a0, kQueueRemove = 0x50e150;
constexpr reg32 kQueueFindIndex = 0x50e470, kQueueFindKey = 0x50e500;
constexpr reg32 kTrace = 0x512180, kDeliver = 0x5126f0;
constexpr reg32 kRequest = 0x513190, kAnswerRequest = 0x5133a0;

// The game's data.
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kTicksPerSecond = 0x5643d8, kMemoryType = 0x5643f4, kModemLog = 0x564454, kModemAbort = 0x564458;
constexpr reg32 kPulseTicks = 0x566dd4, kPulsePeriod = 0x566dd8, kGatherTraceLevel = 0x566de0;
constexpr reg32 kTraceLevel = 0x566dec, kConnectionTimeout = 0x566df0, kConnectionNames = 0x566e04;
constexpr reg32 kLeaveSerial = 0x5689dc, kRasLibrary = 0x568a34, kRasEnum = 0x568a2c, kRasProjection = 0x568a30;
constexpr reg32 kPulseQueue = 0x9f25f0, kClosedConnections = 0x9f260c, kActiveConnections = 0x9f2628;
constexpr reg32 kDropFunction = 0x4f50b0, kNeverTimesOut = 1000000000;

// The gathering's messages, as the transport sends them.
constexpr reg32 kMessageAlive = 0xea6f6c61, kMessageAdd = 0xea616464, kMessageRequest = 0xea726571;
constexpr reg32 kMessageBye = 0xea627965, kMessageGet = 0xea676574, kMessageKick = 0xea6f7574;
constexpr reg32 kMessageData = 0xea646174, kMessageRefused = 0xea6e6f21;

/* A network adapter's table of functions (a gathering's connection +8). */
namespace transport
{
constexpr reg32 kStartListen = 0x28, kStopListen = 0x2c, kGetAddress = 0x30, kCancel = 0x34, kSend = 0x54, kAck = 0x5c;
}

/* A gathering's connection. */
namespace conn
{
constexpr reg32 kInfo = 0x04, kTransport = 0x08, kHandle = 0x0c, kGame = 0x14, kRequest = 0x18, kPlayer = 0x1c;
constexpr reg32 kOwnPlayer = 0x30, kRequests = 0x8c, kPulse = 0xa0, kInGame = 0xa4, kQuiet = 0xa8;
constexpr reg32 kPlayerDwords = 5, kGameDwords = 13;
}

/* The game a server keeps, its players' slots after it. */
namespace game
{
constexpr reg32 kOwner = 0x04, kClosed = 0x08, kAddress = 0x0c, kName = 0x3c, kUser = 0x4c, kMembers = 0x50;
constexpr reg32 kInfo = 0x54, kMaxPlayers = 0x84, kSlots = 0x88, kSlotSize = 0x64, kSlotTimer = 0x5c;
constexpr reg32 kBaseSize = 0xec;
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its import's slot.  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 callImport(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A typed core called as C++ (fe::direct), the registers left as the call by address leaves them:
 * its first words in edx, ebx and ecx after eax's, where that call loads them and the callee keeps
 * them, and its result in eax.  What the code after it reads there, and the function's end, finds
 * what it found before.  For a core whose first four words go in eax, edx, ebx and ecx, called
 * where nothing after it meets a register its function leaves changed (specs_from_exe.py's
 * clobbers; FRONTEND_STRICT=1 shows none): those calls stay by address. */
template <reg32 Address, auto Core, typename... Words>
reg32 loaded(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    const reg32 all[] = { reg32(words)..., 0 };
    reg32* const into[] = { &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 1; i < sizeof...(Words) && i < 4; ++i)
        *into[i - 1] = all[i];
    const auto result = fe::direct<Address, Core>(app, cpu, reg32(words)...);
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, reg32>)
        cpu.eax = result;
    else
        cpu.eax = *result;
    return cpu.eax;
}

/* The library's trace of a connection's events, sub_512180. */
void trace(Guest& guest, reg32 text, reg32 data, reg32 ebx, reg32 ecx, reg32 raw, reg32 id)
{
    guest.call(kTrace, args({ text, data, ebx, ecx }, { raw, id }));
}

/* A connection's event with no data. */
void traceEvent(Guest& guest, reg32 text, reg32 raw, reg32 id)
{
    trace(guest, text, 0, 0, 0, raw, id);
}

/* A fatal message: where it is from, then the message. */
void reportError(Guest& guest, reg32 file, reg32 function, reg32 line, reg32 message)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    fe::direct<kReportError, ea::error>(guest.app(), guest.cpu(), message, guest::Words({}));
}

void reportError(Guest& guest, reg32 file, reg32 function, reg32 line, reg32 message, reg32 argument)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    fe::direct<kReportError, ea::error>(guest.app(), guest.cpu(), message, guest::Words{argument});
}

}  // namespace

/* ---------------------------------------------------------------------------
 * cmn\netgathr.c: a server's game, and asking for one
 * ------------------------------------------------------------------------- */

/* sub_5140e0: the server's pulse, per connection: counts the game's advert
 * down, tells each player we are alive every so often and drops one that has
 * not answered in time.  Always 1. */
x86::reg32 frontend::netc::gatherPulse(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    const reg32 frame = guest.frame();
    const reg32 slotPlace = frame, messagePlace = frame + 4;
    const reg32 info = guest.dword(self + conn::kInfo);
    guest.leaveEdx(info);
    if (guest.dword(info) != 0)
        return 1;
    if (guest.dword(self + conn::kQuiet) == 0 && guest.dword(self + conn::kHandle) != 0)
    {
        const reg32 left = guest.dword(self + conn::kInGame);
        if (left != 0 && guest.dword(self + conn::kGame) == 0)
            guest.setDword(self + conn::kInGame, left - 1);
    }
    const reg32 server = guest.dword(self + conn::kGame);
    if (server == 0 || guest.dword(server + game::kClosed) != 0 || guest.dword(server + game::kMembers) == 0)
        return 1;
    guest.setDword(messagePlace, kMessageAlive);
    const reg32 pulse = guest.dword(self + conn::kPulse);
    bool tell = false;
    if (sign(pulse) >= 2)
    {
        guest.setDword(self + conn::kPulse, pulse - 1);
    }
    else
    {
        guest.setDword(self + conn::kPulse, guest.dword(kPulsePeriod));
        if (sign(guest.dword(kGatherTraceLevel)) >= 3)
            trace(guest, 0x5503bc, messagePlace, 0, 0, 0, 0);
        tell = true;
    }
    const reg32 slots = server + game::kSlots;
    guest.setDword(slotPlace, 0);
    for (;;)
    {
        const reg32 slot = guest.dword(slotPlace);
        if (guest.dword(server + game::kMaxPlayers) - 1 <= slot)
            return 1;
        const reg32 bit = reg32(1) << (guest.byte(slotPlace) & 31);
        if ((guest.dword(server + game::kMembers) & bit) != 0)
        {
            const reg32 timer = slots + slot * game::kSlotSize + game::kSlotTimer;
            const reg32 count = guest.dword(timer);
            guest.setDword(timer, count - 1);
            guest.leaveEdx(count);
            if (count == 0)
            {
                if (sign(guest.dword(kGatherTraceLevel)) >= 2)
                    trace(guest, 0x5503cc, slotPlace, 0, 0, 1, 0);
                const reg32 gone = reg32(1) << (guest.byte(slotPlace) & 31);
                guest.setDword(server + game::kMembers, guest.dword(server + game::kMembers) & ~gone);
            }
            else if (tell)
            {
                const reg32 table = guest.dword(self + conn::kTransport);
                guest.call(guest.dword(table + transport::kSend),
                           args({ guest.dword(self + conn::kHandle), slot * game::kSlotSize + slots,
                                  guest.dword(messagePlace), 0 },
                                { 0 }));
            }
        }
        guest.setDword(slotPlace, guest.dword(slotPlace) + 1);
    }
}

void frontend::netcGatherPulse(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5140e0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherPulse(app, cpu, self); });
}

/* sub_514260: every gathering's connection pulsed, by the queue of them. */
x86::reg32 frontend::netc::gatherPulseStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    return loaded<0x50e8e0, frontend::netb::qiterate>(app, cpu, kPulseQueue, 0x5140e0, 1);
}

void frontend::netcGatherPulseStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514260", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEdx) };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherPulseStart(app, cpu); });
}

/* sub_5143d0: a transport's address lookup waited for, the request's record
 * (0x50 bytes copied from the code's table) on the stack while it runs. */
x86::reg32 frontend::netc::gatherRequest(App* app, CPU& cpu, reg32 self, reg32 start, reg32 wanted, reg32 last)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x94);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = start; cpu.ebx = wanted; cpu.ecx = last;
    const reg32 frame = guest.frame();
    const reg32 answer = frame + 0x50;
    copyDwords(guest, frame, 0x512dd0, 0x14);
    // The game's code hands it `wanted` and 0 in ebx and ecx too, which it does not take; there they stay.
    guest.leaveEbx(wanted);
    guest.leaveEcx(0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, cpu, self + conn::kRequests, answer);
    guest.setDword(frame + 0x10, self);
    guest.setDword(frame + 0x14, start);
    guest.setDword(frame, 0);
    guest.setDword(frame + 0xc, last);
    guest.setDword(self + conn::kRequest, frame);
    // And 0 in edx, which it does not take; there it stays.
    guest.leaveEdx(0);
    loaded<0x512fc0, frontend::netb::setResponseTimeout>(app, cpu, wanted);
    guest.leaveEbx(0);
    for (bool again = true; again;)
    {
        const reg32 table = guest.dword(self + conn::kTransport);
        const reg32 got = guest.call(guest.dword(table + transport::kGetAddress),
                                     args({ guest.dword(self + conn::kHandle), answer, cpu.ebx, table }));
        guest.leaveEdx(got);
        for (;;)
        {
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, cpu.ebx)) != 0)
            {
                if (sign(guest.call(kWaitSystemTasks, args({ 1 }))) < 0)
                {
                    again = false;
                    break;
                }
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
            }
            if (loaded<0x512fd0, frontend::netb::responseTimedOut>(app, cpu) != 0 || cpu.ebx == guest.dword(frame + 0x14))
            {
                again = false;
                break;
            }
            if (guest.edx() == 0)
                break;
        }
    }
    if (guest.edx() != 0)
    {
        const reg32 table = guest.dword(self + conn::kTransport);
        guest.leaveEcx(table);
        guest.call(guest.dword(table + transport::kCancel), args({ guest.dword(self + conn::kHandle) }));
    }
    guest.setDword(self + conn::kRequest, 0);
    return guest.edx();
}

void frontend::netcGatherRequest(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5143d0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 self = cpu.eax, start = cpu.edx, wanted = cpu.ebx, last = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherRequest(app, cpu, self, start, wanted, last); });
}

/* sub_5144b0: joining a player's game: leaves the one we are in unless it is
 * that one, then asks to be added.  What the server answered. */
x86::reg32 frontend::netc::gatherJoin(App* app, CPU& cpu, reg32 self, reg32 player, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x114);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = player; cpu.ebx = name;
    const reg32 frame = guest.frame();
    const reg32 reply = frame + 0xac;
    copyDwords(guest, reply, 0x512e20, 0x14);
    if (guest.dword(self + conn::kInGame) != 0)
    {
        if (guest.call(kCheckPlayer, args({ self, self + conn::kPlayer, player })) != 0)
            return 1;
        guest.call(kEndGame, args({ self }));
        guest.setDword(self + conn::kInGame, 0);
    }
    copyDwords(guest, frame + 0x28, self + conn::kOwnPlayer, 0x17);
    copyDwords(guest, frame + 4, player, conn::kPlayerDwords);
    copyDwords(guest, self + conn::kPlayer, player, conn::kPlayerDwords);
    if (name != 0)
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), frame + 0x18, name, 0x10);
    else
        guest.setByte(guest.dword(self + conn::kGame) + game::kName, 0);
    guest.call(kRequest, args({ self, reply, kMessageAdd, frame }, { 0x88, guest.dword(kTicksPerSecond) * 2 }));
    return guest.dword(frame + 0xb0);
}

void frontend::netcGatherJoin(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5144b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, player = cpu.edx, name = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherJoin(app, cpu, self, player, name); });
}

/* sub_5145b0: a game's information asked of its server, into `out`: 1 if the
 * server answered. */
x86::reg32 frontend::netc::gatherGameInfo(App* app, CPU& cpu, reg32 self, reg32 player, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10c);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = player; cpu.ebx = out;
    const reg32 frame = guest.frame();
    const reg32 reply = frame + 0xac;
    copyDwords(guest, reply, 0x512e70, 0x14);
    const bool joined = guest.dword(self + conn::kInGame) != 0;
    if (player == 0 ? !joined : joined)
        return 0;
    if (player != 0)
        copyDwords(guest, self + conn::kPlayer, player, conn::kPlayerDwords);
    guest.call(kRequest, args({ self, reply, kMessageRequest, frame }, { 0xac, guest.dword(kTicksPerSecond) * 2 }));
    copyDwords(guest, out, frame + 0xc8, conn::kGameDwords);
    return guest.dword(frame + 0xb0) != 0 ? 1 : 0;
}

void frontend::netcGatherGameInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5145b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, player = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherGameInfo(app, cpu, self, player, out); });
}

/* sub_514650: a new game served: its record made, the transport listening.  1 if so. */
x86::reg32 frontend::netc::gatherNewGame(App* app, CPU& cpu, reg32 self, reg32 info, reg32 name, reg32 user)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = info; cpu.ebx = name; cpu.ecx = user;
    constexpr reg32 kFile = 0x5503dc, kFunction = 0x5503ec;
    const reg32 maxPlayers = info + 0x30;
    if (guest.dword(maxPlayers) == 0)
        guest.setDword(maxPlayers, 0x20);
    if (guest.dword(self + conn::kInGame) != 0)
    {
        guest.call(kEndGame, args({ self }));
        guest.setDword(self + conn::kInGame, 0);
    }
    if (guest.dword(maxPlayers) < 2)
        reportError(guest, kFile, kFunction, 0x2c5, 0x5503f8);
    if (guest.dword(maxPlayers) > 0x20)
        reportError(guest, kFile, kFunction, 0x2c7, 0x55042c, 0x20);
    const reg32 size = (guest.dword(maxPlayers) - 2) * game::kSlotSize + game::kBaseSize;
    guest.setDword(kErrorFile, kFile);
    guest.setDword(kErrorFunction, kFunction);
    guest.setDword(kErrorLine, 0x2cb);
    const reg32 server = guest.call(kAllocate, args({ 0x550458, size, guest.dword(kMemoryType), kFile }));
    guest.setDword(self + conn::kGame, server);
    if (server == 0)
        return 0;
    guest.setDword(server + game::kOwner, self);
    copyDwords(guest, guest.dword(self + conn::kGame) + game::kInfo, info, conn::kGameDwords);
    guest.leaveEcx(0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kClosed, 0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kMembers, 0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kUser, user);
    guest.setDword(self + conn::kInGame, guest.dword(kPulseTicks));
    guest.setDword(self + conn::kPulse, guest.dword(kPulsePeriod));
    copyDwords(guest, self + conn::kPlayer, self + conn::kOwnPlayer, conn::kPlayerDwords);
    if (name != 0)
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), guest.dword(self + conn::kGame) + game::kName, name, 0x10);
    else
        guest.setByte(guest.dword(self + conn::kGame) + game::kName, 0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, cpu, self + conn::kRequests, guest.dword(self + conn::kGame) + game::kAddress);
    const reg32 table = guest.dword(self + conn::kTransport);
    guest.leaveEbx(table);
    if (guest.call(guest.dword(table + transport::kStartListen),
                   args({ guest.dword(self + conn::kHandle), guest.dword(self + conn::kGame) + game::kAddress })) != 0)
        return 1;
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(self + conn::kGame));
    guest.setDword(self + conn::kGame, 0);
    return 0;
}

void frontend::netcGatherNewGame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514650", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 self = cpu.eax, info = cpu.edx, name = cpu.ebx, user = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherNewGame(app, cpu, self, info, name, user); });
}

/* sub_514820: the served game's information changed.  1, or 0 with no game. */
x86::reg32 frontend::netc::gatherSetGameInfo(App* app, CPU& cpu, reg32 self, reg32 info)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = info;
    const reg32 server = guest.dword(guest.eax() + conn::kGame);
    if (server == 0)
        return 0;
    copyDwords(guest, server + game::kInfo, guest.edx(), conn::kGameDwords);
    return 1;
}

void frontend::netcGatherSetGameInfo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514820", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, info = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherSetGameInfo(app, cpu, self, info); });
}

/* sub_514850: the served game no longer listened for; with `discard` its record goes too. */
x86::reg32 frontend::netc::gatherCloseGame(App* app, CPU& cpu, reg32 self, reg32 discard)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = discard;
    const reg32 server = guest.dword(self + conn::kGame);
    guest.leaveEcx(self);
    guest.leaveEbx(discard);
    guest.leaveEdx(server);
    if (server == 0)
        return 0;
    if (guest.dword(server + game::kClosed) == 0)
    {
        const reg32 table = guest.dword(self + conn::kTransport);
        guest.call(guest.dword(table + transport::kStopListen),
                   args({ guest.dword(self + conn::kHandle), server + game::kAddress }));
        guest.setByte(guest.dword(cpu.ecx + conn::kGame) + game::kAddress, 0);
        guest.setDword(guest.dword(cpu.ecx + conn::kGame) + game::kClosed, 1);
    }
    if (cpu.ebx != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(cpu.ecx + conn::kGame));
        guest.setDword(cpu.ecx + conn::kGame, 0);
    }
    return 1;
}

void frontend::netcGatherCloseGame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514850", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi) };
    const reg32 self = cpu.eax, discard = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherCloseGame(app, cpu, self, discard); });
}

/* sub_5148b0: a closed game listened for again, every slot's time given back. */
x86::reg32 frontend::netc::gatherReopenGame(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    const reg32 server = guest.dword(self + conn::kGame);
    if (server == 0 || guest.dword(server + game::kClosed) == 0)
        return 0;
    guest.setDword(server + game::kClosed, 0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, cpu, self + conn::kRequests, guest.dword(self + conn::kGame) + game::kAddress);
    const reg32 table = guest.dword(self + conn::kTransport);
    const reg32 listening = guest.call(guest.dword(table + transport::kStartListen),
                                       args({ guest.dword(self + conn::kHandle),
                                              guest.dword(self + conn::kGame) + game::kAddress, cpu.ebx, table }));
    if (listening == 0)
    {
        guest.setDword(guest.dword(self + conn::kGame) + game::kClosed, 0);
        return 0;
    }
    const reg32 ticks = guest.dword(kPulseTicks);
    guest.setDword(self + conn::kPulse, 1);
    const reg32 now = guest.dword(self + conn::kGame);
    for (reg32 slot = 0; slot < guest.dword(now + game::kMaxPlayers) - 1; ++slot)
        guest.setDword(now + game::kSlots + slot * game::kSlotSize + game::kSlotTimer, ticks);
    return 1;
}

void frontend::netcGatherReopenGame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_5148b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx | kEsi | kEdi) };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherReopenGame(app, cpu, self); });
}

/* sub_514970: leaving: the served game told goodbye and thrown away, or our
 * server told we go.  Always 1. */
x86::reg32 frontend::netc::gatherLeave(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xb8);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    const reg32 frame = guest.frame();
    if (guest.dword(self + conn::kInGame) == 0)
        return 1;
    const reg32 byeNumber = guest.dword(kLeaveSerial);
    const reg32 bye = frame, messagePlace = frame + 0xac;
    guest.setDword(bye, byeNumber);
    guest.setDword(messagePlace, kMessageBye);
    copyDwords(guest, frame + 4, self + conn::kPlayer, conn::kPlayerDwords);
    guest.setDword(kLeaveSerial, byeNumber + 1);
    const reg32 server = guest.dword(self + conn::kGame);
    if (server == 0)
    {
        if (sign(guest.dword(kGatherTraceLevel)) >= 2)
            trace(guest, 0x55046c, messagePlace, 0, 0, 0, 0);
        const reg32 table = guest.dword(self + conn::kTransport);
        guest.call(guest.dword(table + transport::kSend),
                   args({ guest.dword(self + conn::kHandle), self + conn::kPlayer, guest.dword(messagePlace), bye },
                        { 0x18 }));
    }
    else
    {
        if (guest.dword(server + game::kClosed) == 0)
        {
            const reg32 table = guest.dword(self + conn::kTransport);
            guest.call(guest.dword(table + transport::kStopListen),
                       args({ guest.dword(self + conn::kHandle), server + game::kAddress, server, table }));
            guest.setByte(guest.dword(self + conn::kGame) + game::kAddress, 0);
            if (sign(guest.dword(kGatherTraceLevel)) >= 2)
                trace(guest, 0x550460, messagePlace, 0, 0, 0, 0);
            for (reg32 slot = 0;; ++slot)
            {
                const reg32 now = guest.dword(self + conn::kGame);
                const reg32 members = guest.dword(now + game::kMembers);
                if (members == 0 || guest.dword(now + game::kMaxPlayers) - 1 <= slot)
                    break;
                const reg32 bit = reg32(1) << (slot & 31);
                if ((members & bit) == 0)
                    continue;
                guest.setDword(now + game::kMembers, members & ~bit);
                const reg32 sendTable = guest.dword(self + conn::kTransport);
                guest.call(guest.dword(sendTable + transport::kSend),
                           args({ guest.dword(self + conn::kHandle),
                                  guest.dword(self + conn::kGame) + game::kSlots + slot * game::kSlotSize,
                                  guest.dword(messagePlace), bye },
                                { 0x18 }));
            }
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(self + conn::kGame));
        guest.setDword(self + conn::kGame, 0);
    }
    guest.setDword(self + conn::kInGame, 0);
    guest.setDword(self + conn::kPulse, 0);
    return 1;
}

void frontend::netcGatherLeave(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514970", 0, 0, nullptr, 0, kAll };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherLeave(app, cpu, self); });
}

/* sub_514af0: the players of a game, each handed to the caller's function:
 * from our own record if it is our game, asked of the server if not.  How many
 * went by, or whether the last answer came. */
x86::reg32 frontend::netc::gatherEnumPlayers(App* app, CPU& cpu, reg32 self, reg32 player, reg32 callback)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x120);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = player; cpu.ebx = callback;
    const reg32 frame = guest.frame();
    const reg32 reply = frame + 0xac, replyWords = frame + 0xb0;
    reg32 wanted;
    if (player == 0 && guest.dword(self + conn::kInGame) != 0)
    {
        wanted = self + conn::kPlayer;
    }
    else
    {
        if (player == 0 || guest.dword(self + conn::kInGame) != 0)
            return 0;
        copyDwords(guest, self + conn::kPlayer, player, conn::kPlayerDwords);
        wanted = player;
    }
    if (guest.dword(self + conn::kGame) != 0
        && guest.call(kCheckPlayer, args({ self, self + conn::kPlayer, wanted })) != 0)
    {
        // Our own game: the server's player, then the members.
        if (guest.call(callback, args({ self, self + conn::kOwnPlayer, 0, 1 })) == 0)
            return 0;
        reg32 went = 1;
        for (reg32 slot = 0;; ++slot)
        {
            const reg32 server = guest.dword(self + conn::kGame);
            guest.leaveEdx(guest.dword(server + game::kMaxPlayers) - 1);
            if (guest.dword(server + game::kMaxPlayers) - 1 <= slot)
                return went;
            const reg32 bit = reg32(1) << (slot & 31);
            if ((guest.dword(server + game::kMembers) & bit) == 0)
                continue;
            if (guest.call(callback, args({ self, server + game::kSlots + slot * game::kSlotSize, slot + 1, 0 })) == 0)
                return went;
            ++went;
        }
    }
    copyDwords(guest, reply, 0x512ec0, 0x14);
    guest.call(kRequest, args({ self, reply, kMessageRequest, frame }, { 0xac, guest.dword(kTicksPerSecond) * 2 }));
    if (guest.dword(replyWords) == 0)
        return 0;
    const reg32 members = guest.dword(frame + 0xb4);
    guest.setDword(frame + 0xc4, callback);
    guest.setDword(replyWords, 1);
    guest.setDword(frame + 0x18, 0xffffffff);
    copyDwords(guest, frame + 4, self + conn::kPlayer, conn::kPlayerDwords);
    guest.call(kRequest, args({ self, reply, kMessageGet, frame }, { 0x1c, guest.dword(kTicksPerSecond) * 2 }));
    reg32 answered = guest.dword(replyWords) != 0 ? 1 : 0;
    guest.leaveEbx(0);
    if (members == 0)
        return answered;
    for (reg32 index = 0;;)
    {
        guest.leaveEbx(index);
        if (sign(index) >= 0x1f || guest.dword(frame + 0xc4) == 0 || answered == 0)
            return answered;
        if (((reg32(1) << (index & 31)) & members) != 0)
        {
            guest.setDword(frame + 0x18, byteSwapped(index));
            guest.call(kRequest, args({ self, reply, kMessageGet, frame }, { 0x1c, guest.dword(kTicksPerSecond) * 2 }));
            answered = guest.dword(replyWords) != 0 ? 1 : 0;
            index &= ~(reg32(1) << (index & 31));
            guest.leaveEbx(index);
        }
        ++index;
        guest.leaveEdx(index);
    }
}

void frontend::netcGatherEnumPlayers(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514af0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, player = cpu.edx, callback = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherEnumPlayers(app, cpu, self, player, callback); });
}

/* sub_514dc0: a player thrown out of our game, or an error if it is us or not our game.  0. */
x86::reg32 frontend::netc::gatherKillPlayer(App* app, CPU& cpu, reg32 self, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = player;
    constexpr reg32 kFile = 0x5503dc, kFunction = 0x550478;
    const reg32 server = guest.dword(self + conn::kGame);
    guest.leaveEdx(server);
    if (server == 0)
    {
        reportError(guest, kFile, kFunction, 0x39f, 0x5504b8);
        return 0;
    }
    const reg32 slot = loaded<0x5132a0, frontend::netb::findPlayerByAddress>(app, cpu, server, player);
    if (slot == 0)
    {
        if (guest.call(kCheckPlayer, args({ self, player, self + conn::kOwnPlayer })) != 0)
            reportError(guest, kFile, kFunction, 0x39b, 0x550488);
        return 0;
    }
    const reg32 index = reg32(sign(slot - (guest.dword(self + conn::kGame) + game::kSlots)) / sign(game::kSlotSize));
    const reg32 table = guest.dword(self + conn::kTransport);
    guest.call(guest.dword(table + transport::kSend),
               args({ guest.dword(self + conn::kHandle), slot, kMessageKick, 0 }, { 0 }));
    const reg32 keep = ~(reg32(1) << (index & 31));
    const reg32 now = guest.dword(self + conn::kGame);
    guest.setDword(now + game::kMembers, guest.dword(now + game::kMembers) & keep);
    guest.leaveEdx(keep);
    return 0;
}

void frontend::netcGatherKillPlayer(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514dc0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, player = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherKillPlayer(app, cpu, self, player); });
}

/* sub_514ec0: a packet connection opened over one of the two layers, its times
 * filled in from the clock's rate where it has none. */
x86::reg32 frontend::netc::gatherOpenConnection(App* app, CPU& cpu, reg32 first, reg32 connection, reg32 wanted,
                                                reg32 window)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = first; cpu.edx = connection; cpu.ebx = wanted; cpu.ecx = window;
    constexpr reg32 kUnreliableLayer = 0x56ac10, kPacketLayer = 0x56ac28;
    const reg32 ticks = guest.dword(kTicksPerSecond);
    const reg32 layer = window == 0xffffffff ? kUnreliableLayer : kPacketLayer;
    if (connection != 0)
    {
        if (guest.word(connection + 0x2a) == 0)
            guest.setWord(connection + 0x2a, std::uint16_t(sign(ticks) / 0x1e));
        if (guest.word(connection + 0x2c) == 0)
            guest.setWord(connection + 0x2c, std::uint16_t(sign(ticks) / 0xf));
        if (guest.word(connection + 0x2e) == 0)
            guest.setWord(connection + 0x2e, std::uint16_t(sign(ticks) / 0xf));
        guest.setWord(connection + 0x28, 0);
    }
    guest.setDword(kTicksPerSecond, ticks);
    return guest.call(kOpenConnection, args({ connection, layer, kConnectionNames, wanted }, { window }));
}

void frontend::netcGatherOpenConnection(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514ec0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 first = cpu.eax, connection = cpu.edx, wanted = cpu.ebx, window = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherOpenConnection(app, cpu, first, connection, wanted, window); });
}

/* sub_514f70: what came for a request in progress handed to it, and the
 * transport told, unless it is quiet.  1 if it was data. */
x86::reg32 frontend::netc::gatherReceive(App* app, CPU& cpu, reg32 self, reg32 from, reg32 kind, reg32 length,
                                         reg32 extraArgument, reg32 acceptedArgument)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x68);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = from; cpu.ebx = kind; cpu.ecx = length;
    const reg32 frame = guest.frame();
    const reg32 kindPlace = frame + 0x50;
    const reg32 extra = extraArgument, acceptedPlace = acceptedArgument;  // its stack arguments
    guest.setDword(kindPlace, cpu.ebx);
    copyDwords(guest, frame, 0x512f10, 0x14);
    guest.setDword(frame + 0x10, self);
    if (guest.dword(self + conn::kRequest) == 0)
        guest.setDword(self + conn::kRequest, frame);
    if (sign(guest.dword(kGatherTraceLevel)) >= 5)
        trace(guest, 0x5504f8, kindPlace, length, extra, 0, 0);
    guest.call(kAnswerRequest, args({ guest.dword(self + conn::kRequest), from, guest.dword(kindPlace), length }, { extra }));
    if (guest.dword(self + conn::kRequest) == frame)
        guest.setDword(self + conn::kRequest, 0);
    if (guest.dword(self + conn::kQuiet) == 0)
    {
        const reg32 table = guest.dword(self + conn::kTransport);
        guest.call(guest.dword(table + transport::kAck), args({ guest.dword(self + conn::kHandle), 0, kMessageData, 0 }));
    }
    guest.setDword(acceptedPlace, guest.dword(kindPlace) != kMessageRefused ? 1 : 0);
    guest.leaveEdx(acceptedPlace);
    return guest.dword(kindPlace) == kMessageData ? 1 : 0;
}

void frontend::netcGatherReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_514f70", 8, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 self = cpu.eax, from = cpu.edx, kind = cpu.ebx, length = cpu.ecx;
    const reg32 extraArgument = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 acceptedArgument = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::gatherReceive(app, cpu, self, from, kind, length, extraArgument, acceptedArgument); });
}

/* sub_515050: rasapi32.dll and its two functions found, once.  1 if they are there. */
x86::reg32 frontend::netc::loadRasApi(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    if (guest.dword(kRasLibrary) != 0)
        return 1;
    const reg32 library = callImport<kLoadLibrary, &host::loadLibrary>(guest, cpu, 0x550504);
    guest.setDword(kRasLibrary, library);
    if (library == 0)
    {
        guest.setDword(kRasProjection, 0);
        guest.setDword(kRasEnum, 0);
        return 0;
    }
    const reg32 enumerate = callImport<kGetProcAddress, &host::procAddress>(guest, cpu, library, 0x550514);
    guest.setDword(kRasEnum, enumerate);
    const reg32 projection = callImport<kGetProcAddress, &host::procAddress>(guest, cpu, guest.dword(kRasLibrary), 0x550528);
    guest.setDword(kRasProjection, projection);
    return guest.dword(kRasEnum) != 0 && projection != 0 ? 1 : 0;
}

void frontend::netcLoadRasApi(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_515050", 0, 0, nullptr, 0, kAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::loadRasApi(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The two packet layers over a stream
 * ------------------------------------------------------------------------- */

namespace
{

// This file's own, called by address as the game calls them.
constexpr reg32 kFrameChecksum = 0x51bc70, kFrameGood = 0x51bca0, kFrameBuild = 0x51bcd0, kFrameSend = 0x51bcf0;
constexpr reg32 kFrameControl = 0x51bd50;
constexpr reg32 kUnreliableNotify = 0x51c0a0;
constexpr reg32 kPacketChecksum = 0x51c7c0, kPacketBuild = 0x51c890;
constexpr reg32 kPacketSend = 0x51c8f0, kPacketControl = 0x51ca50;
constexpr reg32 kQueueKey = 0x51d000, kCountDown = 0x51d010, kResendDue = 0x51d040, kPacketNotify = 0x51d050;

constexpr reg32 kNextUnreliable = 0x56ac0c, kNextPacket = 0x56ac24;

/* A stream connection's functions (what a packet layer runs over). */
namespace stream
{
constexpr reg32 kOpen = 0x00, kClose = 0x04, kReady = 0x08, kRoom = 0x0c, kAvailable = 0x10, kRead = 0x14, kWrite = 0x18;
constexpr reg32 kMaxFrame = 0x1c, kMinFrame = 0x1e, kReliable = 0x1f;
}

/* A connection as the layers above see it. */
namespace link
{
constexpr reg32 kState = 0x00, kLayer = 0x10, kProcess = 0x14, kDrop = 0x18, kTimedOut = 0x1c, kTimeout = 0x24;
constexpr reg32 kRetries = 0x2a, kIdleRetries = 0x2c, kResendTicks = 0x2e;
}

/* What both layers begin with. */
namespace layer
{
constexpr reg32 kLink = 0x00, kStream = 0x04, kMaxSize = 0x08, kOwned = 0x0c;
}

/* The unreliable layer: frame 0xfe, length, checksum, data. */
namespace plain
{
constexpr reg32 kAlive = 0x0d, kId = 0x0e, kLock = 0x10, kBuffer = 0x14, kBufferSize = 0x18, kHeld = 0x1c;
constexpr reg32 kIdleLeft = 0x20, kSize = 0x24;
}

/* The reliable layer: the same, numbered and acknowledged. */
namespace rely
{
constexpr reg32 kId = 0x0d, kAckSent = 0x0e, kReceived = 0x10, kLock = 0x14, kFirstPacket = 0x18, kBuffer = 0x1c;
constexpr reg32 kBufferSize = 0x20, kHeld = 0x24, kBad = 0x28, kIdleLeft = 0x2c, kNextSend = 0x30, kAcked = 0x34;
constexpr reg32 kFree = 0x38, kUnacked = 0x54, kNextDeliver = 0x70, kNextFree = 0x74, kReceiving = 0x78;
constexpr reg32 kReceivedQueue = 0x94, kSize = 0xb0;
}

/* A packet of the reliable layer, as it waits in a queue, its frame at +0xc. */
namespace packet
{
constexpr reg32 kNumber = 0x04, kResend = 0x08, kFrame = 0x0c, kLength = 0x0d, kSequence = 0x0e, kAck = 0x0f;
constexpr reg32 kData = 0x12, kFrameHeader = 6;
}

constexpr reg32 kFrameStart = 0xfe, kSequenceMask = 0x7f, kFrameClose = 0xff, kFrameControlAck = 0xfb;

std::int32_t traceLevel(const Guest& guest)
{
    return sign(guest.dword(kTraceLevel));
}

/* A frame's checksum worked out and put in its place, big end first. */
reg32 stampChecksum(Guest& guest, x86::CPU& cpu, reg32 frame, reg32 place)
{
    guest.setByte(frame + place + 1, 0);
    guest.setByte(frame + place, 0);
    const reg32 sum = loaded<0x4f4780, frontend::wata::blockChecksum>(guest.app(), cpu, frame, guest.edx());
    const reg32 high = reg32(sign(sum) >> 8);
    guest.leaveEdx(sum);
    guest.setByte(frame + place + 1, std::uint8_t(sum));
    guest.setByte(frame + place, std::uint8_t(high));
    return high;
}

/* Whether a frame's checksum is what it says, by stamping it again. */
reg32 checksumGood(Guest& guest, reg32 stamp, reg32 place)
{
    const reg32 frame = guest.eax();
    const std::uint16_t saved = guest.word(frame + place);
    guest.call(stamp, args({ frame, guest.edx() }));
    return saved == guest.word(frame + place) ? 1 : 0;
}

/* What is left of a stream's connection closed for good, once. */
reg32 shutdownLayer(Guest& guest, reg32 self, reg32 idField, reg32 shutdownText, reg32 disconnectText, reg32 closedText,
                    reg32 duplicateText)
{
    if (traceLevel(guest) >= 2)
        traceEvent(guest, shutdownText, 1, guest.byte(self + idField));
    if (guest.call(kQueueRemove, args({ kActiveConnections, guest.dword(self + layer::kLink) })) == 0)
    {
        if (traceLevel(guest) >= 1)
            traceEvent(guest, duplicateText, 1, guest.byte(self + idField));
        return guest.eax();
    }
    const reg32 connection = guest.dword(self + layer::kLink);
    const std::int32_t level = traceLevel(guest);
    guest.leaveEbx(reg32(level));
    if (level >= 1)
        traceEvent(guest, disconnectText, 1, guest.byte(self + idField));
    guest.setDword(connection + link::kTimeout, 0);
    const reg32 ops = guest.dword(self + layer::kStream);
    guest.call(guest.dword(ops + stream::kClose), args({ connection, ops }));
    guest.call(guest.dword(connection + link::kDrop), args({ connection, connection }));
    guest.call(kQueueAdd, args({ kClosedConnections, guest.edx() }));
    if (traceLevel(guest) >= 2)
        traceEvent(guest, closedText, 1, guest.byte(self + idField));
    return guest.eax();
}

/* The stream read into the layer's buffer and its frames taken from there, what
 * is left moved down.  1 if any came whole; the frames count them on the frame. */
template <reg32 FrameFunction, auto Core>
reg32 receiveFrames(Guest& guest, x86::CPU& cpu, reg32 bufferField, reg32 sizeField, reg32 heldField)
{
    const reg32 frame = guest.frame();
    const reg32 self = guest.eax();
    guest.setDword(frame, 0);
    const reg32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    guest.setDword(frame + 4, connection);
    guest.setDword(frame + 8, ops);
    if (guest.byte(ops + stream::kReliable) != 0)
        guest.setDword(self + heldField, 0);
    const reg32 held = guest.dword(self + heldField), size = guest.dword(self + sizeField);
    if (size > held)
    {
        const reg32 got = guest.call(guest.dword(ops + stream::kRead),
                                     args({ connection, guest.dword(self + bufferField) + held, size - held, held }));
        guest.setDword(self + heldField, guest.dword(self + heldField) + got);
    }
    reg32 left = guest.dword(self + heldField);
    if (left != 0)
    {
        reg32 done = 0;
        for (;;)
        {
            const reg32 used = loaded<FrameFunction, Core>(guest.app(), cpu, self, frame,
                                                           guest.dword(self + bufferField) + done, left);
            if (guest.byte(ops + stream::kReliable) != 0 || used == 0)
                break;
            left -= used;
            done += used;
            if (guest.dword(connection + link::kTimeout) == 0)
                break;
        }
        if (guest.byte(ops + stream::kReliable) == 0 && done != 0)
        {
            const reg32 buffer = guest.dword(self + bufferField);
            guest.call(kMoveMemory, args({ buffer + done, buffer, left }));
            guest.setDword(self + heldField, left);
        }
    }
    return guest.dword(frame) != 0 ? 1 : 0;
}

}  // namespace

/* sub_51bc70: an unreliable frame's checksum stamped. */
x86::reg32 frontend::netc::frameChecksum(App* app, CPU& cpu, reg32 frame, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = frame; cpu.edx = length;
    return stampChecksum(guest, cpu, guest.eax(), 2);
}

void frontend::netcFrameChecksum(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bc70", 0, 0, nullptr, 0, kEcx };
    const reg32 frame = cpu.eax, length = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::frameChecksum(app, cpu, frame, length); });
}

/* sub_51bca0: whether an unreliable frame's checksum holds. */
x86::reg32 frontend::netc::frameChecksumGood(App* app, CPU& cpu, reg32 frame, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = frame; cpu.edx = length;
    return checksumGood(guest, kFrameChecksum, 2);
}

void frontend::netcFrameChecksumGood(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bca0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx) };
    const reg32 frame = cpu.eax, length = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::frameChecksumGood(app, cpu, frame, length); });
}

/* sub_51bcd0: an unreliable frame's start and data. */
x86::reg32 frontend::netc::frameBuild(App* app, CPU& cpu, reg32 out, reg32 self, reg32 data, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = out; cpu.edx = self; cpu.ebx = data; cpu.ecx = length;
    guest.setByte(out, kFrameStart);
    guest.setByte(out + 1, std::uint8_t(length));
    return guest.call(kMoveMemory, args({ data, out + 4, length }));
}

void frontend::netcFrameBuild(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bcd0", 0, 0, nullptr, 0, kEsi };
    const reg32 out = cpu.eax, self = cpu.edx, data = cpu.ebx, length = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::frameBuild(app, cpu, out, self, data, length); });
}

/* sub_51bcf0: an unreliable frame written if the stream has room.  1 if all of it went. */
x86::reg32 frontend::netc::frameSend(App* app, CPU& cpu, reg32 out, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = out; cpu.edx = self;
    const reg32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    const reg32 size = guest.byte(out + 1) + reg32(4);
    if (guest.call(guest.dword(ops + stream::kRoom), args({ connection, 0, self, size })) < size)
        return guest.edx();
    guest::direct<0x51bc70, frameChecksum>(app, cpu, out, size);
    const reg32 sent = guest.call(guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                  args({ connection, out, size }));
    const reg32 whole = sent == size ? 1 : 0;
    guest.leaveEdx(whole);
    return whole;
}

void frontend::netcFrameSend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bcf0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 out = cpu.eax, self = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::frameSend(app, cpu, out, self); });
}

/* sub_51bd50: a frame of no data, its length the `code`, written if the stream has room. */
x86::reg32 frontend::netc::frameSendControl(App* app, CPU& cpu, reg32 self, reg32 code)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = code;
    const reg32 frame = guest.frame();
    const reg32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    if (guest.call(guest.dword(ops + stream::kRoom), args({ connection, 0, code, self })) < 4)
        return guest.edx();
    guest.setByte(frame + 1, std::uint8_t(code));
    guest.setByte(frame, kFrameStart);
    // The game's code hands it 4 in ebx too, which it does not read; the call after loads ebx again.
    guest::direct<0x51bc70, frameChecksum>(app, cpu, frame, 4);
    const reg32 now = guest.dword(self + layer::kStream);
    const reg32 sent = guest.call(guest.dword(now + stream::kWrite), args({ connection, frame, 4, now }));
    const reg32 whole = sent == 4 ? 1 : 0;
    guest.leaveEdx(whole);
    return whole;
}

void frontend::netcFrameSendControl(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bd50", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, code = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::frameSendControl(app, cpu, self, code); });
}

/* sub_51bdc0: the unreliable layer's connection closed. */
x86::reg32 frontend::netc::unreliableShutdown(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    return shutdownLayer(guest, guest.eax(), plain::kId, 0x550cb4, 0x550cc4, 0x550cd0, 0x550cdc);
}

void frontend::netcUnreliableShutdown(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bdc0", 0, 0, nullptr, 0, kAll };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableShutdown(app, cpu, self); });
}

/* sub_51be90: the first frame found in what the stream gave: delivered, or
 * skipped.  What was used of the buffer. */
x86::reg32 frontend::netc::unreliableReceiveFrame(App* app, CPU& cpu, reg32 self, reg32 counted, reg32 buffer,
                                                  reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = counted; cpu.ebx = buffer; cpu.ecx = length;
    const reg32 connection = guest.dword(self + layer::kLink);
    const std::uint8_t reliable = guest.byte(guest.dword(self + layer::kStream) + stream::kReliable);
    guest.leaveEbx(length);
    reg32 used = 0;
    if (reliable == 0)
    {
        const reg32 end = length + buffer;
        guest.leaveEdx(end);
        for (reg32 at = buffer; at < end && guest.byte(at) != kFrameStart; ++at)
            ++used;
    }
    else
    {
        guest.leaveEdx((guest.edx() & 0xffffff00) | reliable);
    }
    const reg32 frame = used + buffer;
    if (guest.byte(frame) != kFrameStart)
        return length;
    reg32 end = used + 4;
    guest.leaveEcx(end);
    if (end > length)
        return used;
    const reg32 size = guest.byte(frame + 1);
    guest.leaveEdx(size);
    if (size > guest.dword(self + layer::kMaxSize))
        return used + (size == kFrameStart ? 1 : 2);
    end += size;
    guest.leaveEcx(end);
    if (end > length)
        return used;
    const reg32 whole = end - used;
    guest.leaveEcx(whole);
    if (guest.call(kFrameGood, args({ frame, whole })) == 0)
        return used + 2;
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
        guest.setByte(self + plain::kAlive, 1);
        guest.leaveEdx(connection);
    }
    used += whole;
    const reg32 kind = guest.byte(frame + 1);
    guest.leaveEdx((guest.edx() & 0xffff00ff) | (kind << 8));
    if (kind == kFrameClose)
    {
        if (traceLevel(guest) >= 2)
            traceEvent(guest, 0x550cec, 1, guest.byte(self + plain::kId));
        guest::direct<0x51bdc0, unreliableShutdown>(app, cpu, self);
        return used;
    }
    if (kind == 0)
        return used;
    if (traceLevel(guest) >= 3)
        trace(guest, 0x550cfc, 0, frame + 4, kind, 0, guest.byte(self + plain::kId));
    guest.call(kDeliver, args({ connection, frame + 4, guest.byte(frame + 1) }));
    return used;
}

void frontend::netcUnreliableReceiveFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51be90", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 self = cpu.eax, counted = cpu.edx, buffer = cpu.ebx, length = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableReceiveFrame(app, cpu, self, counted, buffer, length); });
}

/* sub_51bfd0: all that came for the unreliable layer. */
x86::reg32 frontend::netc::unreliableReceive(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    return receiveFrames<0x51be90, unreliableReceiveFrame>(guest, cpu, plain::kBuffer, plain::kBufferSize, plain::kHeld);
}

void frontend::netcUnreliableReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bfd0", 0, 0, nullptr, 0, kAll };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableReceive(app, cpu, self); });
}

/* sub_51c0a0: the stream says something came: read while there is. */
x86::reg32 frontend::netc::unreliableNotify(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    const reg32 self = guest.dword(connection + link::kLayer);
    const reg32 ops = guest.dword(self + layer::kStream);
    guest.leaveEdx(guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
    {
        const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
        traceEvent(guest, 0x550d04, 0, guest.byte(self + plain::kId));
        guest.leaveEbx(ebx);
        guest.leaveEcx(ecx);
    }
    if (guest.dword(connection + link::kTimeout) == 0)
        return guest.eax();
    for (;;)
    {
        if (guest.call(guest.dword(ops + stream::kAvailable), args({ connection })) == 0)
            return 0;
        if (guest::direct<0x51bfd0, unreliableReceive>(app, cpu, self) == 0)
            return 0;
    }
}

void frontend::netcUnreliableNotify(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c0a0", 0, 0, nullptr, 0, std::uint8_t(kEdx | kEsi | kEdi) };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableNotify(app, cpu, connection); });
}

/* sub_51c100: a connection opened on the unreliable layer: the layer's record
 * made unless it has one, the stream asked, and waited for until it stands or
 * has failed.  1 if it stands; with no connection, the record's size. */
x86::reg32 frontend::netc::unreliableOpen(App* app, CPU& cpu, reg32 connection, reg32 ops, reg32 asked)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection; cpu.edx = ops; cpu.ebx = asked;
    constexpr reg32 kFile = 0x550d10, kFunction = 0x550d20;
    reg32 wanted = asked;
    if (sign(asked) < 6)
    {
        wanted = 6;
    }
    else if (sign(asked) <= 0xfa)
    {
        const reg32 limit = guest.word(ops + stream::kMaxFrame);
        guest.leaveEbx((cpu.ebx & 0xffff0000) | limit);
        if (limit != 0 && asked + 4 > limit)
            reportError(guest, kFile, kFunction, 0x12a, 0x550d7c);
    }
    else
    {
        reportError(guest, kFile, kFunction, 0x128, 0x550d3c, 0xfa);
        guest.leaveEbx(kFile);
    }
    if (connection != 0 && guest.dword(connection + link::kTimeout) != 0)
    {
        reportError(guest, kFile, kFunction, 0x12d, 0x550dcc);
        guest.leaveEbx(kFile);
    }
    const reg32 least = guest.byte(ops + stream::kMinFrame);
    if (sign(least) > sign(wanted))
        wanted = least;
    const std::uint8_t reliable = guest.byte(ops + stream::kReliable);
    const reg32 bufferSize = reliable != 0 ? 0xfe : 0x1fc;
    guest.leaveEcx(bufferSize);
    guest.leaveEdx((ops & 0xffffff00) | reliable);
    const reg32 total = bufferSize + plain::kSize;
    if (connection == 0)
        return total;
    reg32 owned = 0;
    reg32 self = guest.dword(connection + link::kLayer);
    guest.leaveEbx(self);
    if (self == 0)
    {
        guest.setDword(kErrorFile, kFile);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x145);
        self = guest.call(kAllocate, args({ 0x550e04, total, guest.dword(kMemoryType), bufferSize }));
        owned = 1;
        if (self == 0)
            return 0;
    }
    guest.setDword(connection + link::kState, 0);
    guest.setDword(connection + link::kTimeout, 0);
    const reg32 oldDrop = guest.dword(connection + link::kDrop);
    guest.setDword(connection + link::kLayer, self);
    guest.setDword(connection + link::kDrop, kDropFunction);
    guest.setDword(self + layer::kLink, connection);
    guest.setDword(self + layer::kMaxSize, wanted);
    guest.setDword(self + plain::kIdleLeft, guest.word(connection + link::kRetries));
    guest.setByte(self + layer::kOwned, std::uint8_t(owned));
    guest.setDword(self + layer::kStream, ops);
    const reg32 id = guest.dword(kNextUnreliable);
    guest.setWord(self + plain::kId, std::uint16_t(id));
    guest.setByte(self + plain::kAlive, 0);
    guest.setDword(kNextUnreliable, id + 1);
    const reg32 lock = reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    guest.setDword(self + plain::kHeld, 0);
    guest.setDword(self + plain::kLock, lock);
    guest.setDword(self + plain::kBuffer, self + plain::kSize);
    guest.setDword(self + plain::kBufferSize, bufferSize);
    if (guest.dword(kConnectionTimeout) == 0)
    {
        reportError(guest, kFile, kFunction, 0x16d, 0x550e0c);
        guest.leaveEbx(kFunction);
    }
    guest.leaveEcx(ops);
    reg32 failed = 0;
    if (guest.call(guest.dword(ops + stream::kOpen), args({ connection, kUnreliableNotify })) == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, 0x550e60, 0, guest.byte(self + plain::kId));
    }
    else
    {
        guest.setDword(connection + link::kTimeout, kNeverTimesOut);
        guest.call(kQueueAdd, args({ kActiveConnections, connection }));
        if (traceLevel(guest) >= 1)
            traceEvent(guest, 0x550e34, 0, guest.byte(self + plain::kId));
        for (;;)
        {
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) == 0)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
                failed = guest.call(guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            else if (sign(guest.call(kWaitSystemTasks, args({ 1 }))) < 0)
            {
                failed = 1;
            }
            else
            {
                failed = guest.call(guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            if (failed != 0 && traceLevel(guest) >= 2)
                traceEvent(guest, 0x550e40, 0, guest.byte(self + plain::kId));
            if (guest.dword(connection + link::kTimeout) != 0)
                guest.setDword(connection + link::kTimeout, kNeverTimesOut);
            if (guest.dword(connection + link::kTimeout) == 0 || guest.byte(self + plain::kAlive) != 0 || failed != 0)
                break;
        }
        if (guest.dword(connection + link::kTimeout) == 0 || failed != 0)
        {
            const reg32 timeout = guest.dword(connection + link::kTimeout);
            guest.leaveEbx(timeout);
            if (timeout == 0 && traceLevel(guest) >= 2)
                traceEvent(guest, 0x550e50, 0, guest.byte(self + plain::kId));
            guest.setDword(connection + link::kTimeout, 0);
            guest.call(guest.dword(ops + stream::kClose), args({ connection, ops }));
        }
    }
    if (guest.dword(connection + link::kTimeout) != 0 && failed == 0)
    {
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
        guest.setDword(connection + link::kDrop, oldDrop);
        if (traceLevel(guest) >= 1)
            traceEvent(guest, 0x550e70, 0, guest.byte(self + plain::kId));
        return 1;
    }
    if (traceLevel(guest) >= 1)
        traceEvent(guest, 0x550e84, 0, guest.byte(self + plain::kId));
    guest.call(kQueueRemove, args({ kActiveConnections, connection }));
    guest.call(kQueueRemove, args({ kClosedConnections, connection, owned }));
    if (owned != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 0;
}

void frontend::netcUnreliableOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c100", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 connection = cpu.eax, ops = cpu.edx, asked = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableOpen(app, cpu, connection, ops, asked); });
}

/* sub_51c510: data sent on the unreliable layer, made a frame on the stack.
 * (The original's test for no buffer is of its stack pointer, never zero.) */
x86::reg32 frontend::netc::unreliableSend(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x110);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection; cpu.edx = data; cpu.ebx = length;
    const reg32 frame = guest.frame();
    if (connection == 0 || guest.dword(connection + link::kTimeout) == 0)
        return 0;
    const reg32 self = guest.dword(connection + link::kLayer);
    if (length > guest.dword(self + layer::kMaxSize))
    {
        reportError(guest, 0x550d10, 0x550e98, 0x1b1, 0x550eb0);
        guest.leaveEbx(0x550e98);
    }
    guest.call(kFrameBuild, args({ frame, self, data, length }));
    guest.call(kFrameSend, args({ frame, self }));
    return 1;
}

void frontend::netcUnreliableSend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c510", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 connection = cpu.eax, data = cpu.edx, length = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableSend(app, cpu, connection, data, length); });
}

/* sub_51c5c0: the unreliable layer's turn: counts its timeout down, says it is
 * alive when it has been quiet, closes on timing out.  Always 1. */
x86::reg32 frontend::netc::unreliableProcess(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    const reg32 self = guest.dword(connection + link::kLayer);
    guest.leaveEdx(guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
        traceEvent(guest, 0x550ef0, 0, guest.byte(self + plain::kId));
    const reg32 left = guest.dword(connection + link::kTimeout) - 1;
    guest.setDword(connection + link::kTimeout, left);
    guest.leaveEbx(left);
    if (left == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, 0x550efc, 0, guest.byte(self + plain::kId));
        guest.setDword(connection + link::kTimeout, guest.call(guest.dword(connection + link::kTimedOut), args({ connection })));
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        const reg32 ops = guest.dword(self + layer::kStream);
        if (guest.call(guest.dword(ops + stream::kReady), args({ connection, ops })) != 0)
        {
            const reg32 idle = guest.dword(self + plain::kIdleLeft);
            guest.setDword(self + plain::kIdleLeft, idle - 1);
            if (sign(idle) <= 0)
                guest::direct<0x51bd50, frameSendControl>(app, cpu, self, 0);
            return 1;
        }
    }
    if (traceLevel(guest) >= 2)
        trace(guest, 0x550f08, connection + link::kTimeout, 0, 0, 1, guest.byte(self + plain::kId));
    guest::direct<0x51bdc0, unreliableShutdown>(app, cpu, self);
    return 1;
}

void frontend::netcUnreliableProcess(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c5c0", 0, 0, nullptr, 0, kAll };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableProcess(app, cpu, connection); });
}

/* sub_51c690: the unreliable layer's connection closed by us: the other end told
 * if it can be, the record gone.  1 if it was there. */
x86::reg32 frontend::netc::unreliableClose(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    if (connection == 0)
        return 0;
    const reg32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
        return 0;
    const reg32 ops = guest.dword(self + layer::kStream);
    guest.leaveEcx(guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 1)
    {
        const reg32 ebx = cpu.ebx;
        traceEvent(guest, 0x550f14, 0, guest.byte(self + plain::kId));
        guest.leaveEbx(ebx);
    }
    if (guest.call(kQueueRemove, args({ kActiveConnections, connection })) == 0)
    {
        if (guest.dword(connection + link::kTimeout) != 0)
            guest.call(kMonoPrintf, args({ 0 }, { 0x550f24 }));
    }
    else
    {
        if (guest.dword(connection + link::kTimeout) != 0)
        {
            guest.setDword(connection + link::kTimeout, 0);
            reg32 tries = 4;
            for (;;)
            {
                --tries;
                guest.leaveEcx(tries);
                if (tries == 0xffffffff
                    || guest.call(guest.dword(ops + stream::kReady), args({ connection })) == 0)
                    break;
                // By address: the edx it leaves is handed to the stream's functions after it.
                if (guest.call(kFrameControl, args({ self, kFrameClose })) != 0)
                    tries = 0;
                else
                    loaded<0x4f06e0, frontend::wata::timerWait>(app, cpu, 1);
            }
            loaded<0x4f06e0, frontend::wata::timerWait>(app, cpu, 3);
            guest.call(guest.dword(ops + stream::kClose), args({ connection }));
        }
        fe::direct<0x50de50, netb::qpush>(app, cpu, kClosedConnections, connection);
    }
    if (guest.call(kQueueRemove, args({ kClosedConnections, connection })) == 0)
        return 0;
    reg32(*fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(self + plain::kLock)));
    if (guest.byte(self + layer::kOwned) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 1;
}

void frontend::netcUnreliableClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c690", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx | kEsi | kEdi) };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::unreliableClose(app, cpu, connection); });
}

/* sub_51c7a0: a number of few bits widened, near what is expected, going on past it. */
x86::reg32 frontend::netc::sequenceWiden(App* app, CPU& cpu, reg32 expected, reg32 low, reg32 mask)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = expected; cpu.edx = low; cpu.ebx = mask;
    reg32 wide = (~mask & expected) | low;
    if (wide < expected)
    {
        guest.leaveEbx(mask + 1);
        wide += mask + 1;
    }
    return wide;
}

void frontend::netcSequenceWiden(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c7a0", 0, 0, nullptr, 0, kEcx };
    const reg32 expected = cpu.eax, low = cpu.edx, mask = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::sequenceWiden(app, cpu, expected, low, mask); });
}

/* sub_51c7c0: a reliable frame's checksum stamped. */
x86::reg32 frontend::netc::packetChecksum(App* app, CPU& cpu, reg32 frame, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = frame; cpu.edx = length;
    return stampChecksum(guest, cpu, guest.eax(), 4);
}

void frontend::netcPacketChecksum(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c7c0", 0, 0, nullptr, 0, kEcx };
    const reg32 frame = cpu.eax, length = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetChecksum(app, cpu, frame, length); });
}

/* sub_51c7f0: whether a reliable frame's checksum holds. */
x86::reg32 frontend::netc::packetChecksumGood(App* app, CPU& cpu, reg32 frame, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = frame; cpu.edx = length;
    return checksumGood(guest, kPacketChecksum, 4);
}

void frontend::netcPacketChecksumGood(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c7f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx) };
    const reg32 frame = cpu.eax, length = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetChecksumGood(app, cpu, frame, length); });
}

/* sub_51c820: the number to acknowledge in what is sent now, the idle time starting over. */
x86::reg32 frontend::netc::packetAckNumber(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    const reg32 frame = guest.frame();
    // The game's code hands it `self` in edx too, which it does not take; there it stays.
    guest.leaveEdx(self);
    reg32 number = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, cpu, self + rely::kReceiving);
    if (number == 0xffffffff)
        number = guest.dword(self + rely::kNextFree);
    --number;
    guest.setDword(frame, number);
    reg32 result = guest.word(guest.dword(self + layer::kLink) + link::kRetries);
    guest.setDword(self + rely::kIdleLeft, result);
    if (traceLevel(guest) >= 3)
    {
        const reg32 ebx = cpu.ebx;
        trace(guest, 0x550f68, frame, 0, 0, 0, guest.byte(self + rely::kId));
        guest.leaveEbx(ebx);
        result = guest.eax();
    }
    return (result & 0xffffff00) | (guest.byte(frame) & kSequenceMask);
}

void frontend::netcPacketAckNumber(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c820", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetAckNumber(app, cpu, self); });
}

/* sub_51c890: a packet numbered and its frame's start and data made. */
x86::reg32 frontend::netc::packetBuild(App* app, CPU& cpu, reg32 out, reg32 self, reg32 data, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = out; cpu.edx = self; cpu.ebx = data; cpu.ecx = length;
    const reg32 queue = self + rely::kUnacked;
    const reg32 locked = fe::direct<0x50ea20, frontend::menz::fileLockEnter>(app, cpu, queue);
    const reg32 number = guest.dword(self + rely::kNextSend);
    guest.setDword(self + rely::kNextSend, number + 1);
    guest.setDword(out + packet::kNumber, number);
    fe::direct<0x50ea40, frontend::menz::fileLockLeave>(app, cpu, queue, locked);
    guest.setDword(out + packet::kResend, 0xffffffff);
    guest.setByte(out + packet::kFrame, kFrameStart);
    guest.setByte(out + packet::kSequence, guest.byte(out + packet::kNumber) & kSequenceMask);
    guest.setByte(out + packet::kLength, std::uint8_t(length));
    return guest.call(kMoveMemory, args({ data, out + packet::kData, length }));
}

void frontend::netcPacketBuild(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c890", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 out = cpu.eax, self = cpu.edx, data = cpu.ebx, length = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetBuild(app, cpu, out, self, data, length); });
}

/* sub_51c8f0: a packet written if the stream has room, its resend time started.
 * 1 if it went. */
x86::reg32 frontend::netc::packetSend(App* app, CPU& cpu, reg32 out, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = out; cpu.edx = self;
    const reg32 sizePlace = guest.frame();
    const reg32 connection = guest.dword(self + layer::kLink);
    const reg32 room = guest.call(guest.dword(guest.dword(self + layer::kStream) + stream::kRoom),
                                  args({ connection, guest.dword(self + layer::kStream), cpu.ebx, self }));
    const reg32 number = out + packet::kNumber;
    const reg32 needed = guest.byte(out + packet::kLength) + packet::kFrameHeader;
    guest.leaveEdx(needed);
    bool went = false;
    if (room < needed)
    {
        if (sign(guest.dword(out + packet::kResend)) < 0)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, 0x550f90, number, 0, 0, 1, guest.byte(self + rely::kId));
        }
        else if (traceLevel(guest) >= 3)
        {
            trace(guest, 0x550f98, number, 0, 0, 1, guest.byte(self + rely::kId));
        }
    }
    else
    {
        const reg32 ack = guest::direct<0x51c820, packetAckNumber>(app, cpu, self);
        const reg32 size = guest.byte(out + packet::kLength) + packet::kFrameHeader;
        guest.setDword(sizePlace, size);
        guest.setByte(out + packet::kAck, std::uint8_t(ack));
        const reg32 frame = out + packet::kFrame;
        guest::direct<0x51c7c0, packetChecksum>(app, cpu, frame, guest.byte(out + packet::kLength) + packet::kFrameHeader);
        const reg32 sent = guest.call(guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                      args({ connection, frame, guest.dword(sizePlace) }));
        const reg32 expected = guest.dword(sizePlace);
        guest.leaveEdx(expected);
        if (sent == expected)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, sign(guest.dword(out + packet::kResend)) >= 0 ? 0x550f78 : 0x550f70, number,
                      out + packet::kData, guest.byte(out + packet::kLength), 0, guest.byte(self + rely::kId));
            went = true;
        }
        else if (traceLevel(guest) >= 1)
        {
            trace(guest, 0x550f80, sizePlace, frame, expected, 1, guest.byte(self + rely::kId));
        }
    }
    if (!went)
    {
        guest.setDword(out + packet::kResend, 1);
        return 0;
    }
    guest.leaveEdx(connection);
    guest.setDword(out + packet::kResend, guest.word(connection + link::kResendTicks));
    return 1;
}

void frontend::netcPacketSend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51c8f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 out = cpu.eax, self = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetSend(app, cpu, out, self); });
}

/* sub_51ca50: a frame of no data, its number the `code`, written if the stream
 * has room; `control` holds it and its resend time.  1 if it went. */
x86::reg32 frontend::netc::packetSendControl(App* app, CPU& cpu, reg32 control, reg32 self, reg32 code)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = control; cpu.edx = self; cpu.ebx = code;
    const reg32 connection = guest.dword(self + layer::kLink);
    guest.leaveEcx(control);
    guest.setDword(control + packet::kResend, 1);
    const reg32 ops = guest.dword(self + layer::kStream);
    if (guest.call(guest.dword(ops + stream::kRoom), args({ connection, 0 })) < packet::kFrameHeader)
        return guest.edx();
    const reg32 ack = guest::direct<0x51c820, packetAckNumber>(app, cpu, self) & 0xff;
    const reg32 frame = control + packet::kFrame;
    guest.setByte(frame, kFrameStart);
    guest.setByte(control + packet::kSequence, std::uint8_t(code));
    guest.setByte(control + packet::kAck, std::uint8_t(ack));
    guest.setByte(control + packet::kLength, 0);
    // The game's code hands it the length in ebx too, which it does not read; the write below reads it there.
    guest.leaveEbx(packet::kFrameHeader);
    guest::direct<0x51c7c0, packetChecksum>(app, cpu, frame, packet::kFrameHeader);
    const reg32 sent = guest.call(guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                  args({ connection, frame }));
    const reg32 whole = sent == packet::kFrameHeader ? 1 : 0;
    guest.leaveEdx(whole);
    if (whole != 0 && code <= kSequenceMask)
        guest.setDword(control + packet::kResend, guest.word(connection + 0x2c));
    return whole;
}

void frontend::netcPacketSendControl(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51ca50", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 control = cpu.eax, self = cpu.edx, code = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetSendControl(app, cpu, control, self, code); });
}

/* sub_51caf0: a packet that came out of order asked for again, when its time has
 * come.  1, or what asking gave. */
x86::reg32 frontend::netc::packetSendNak(App* app, CPU& cpu, reg32 waiting, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = waiting; cpu.edx = self;
    if (waiting == 0)
        return 1;
    const reg32 left = guest.dword(waiting + packet::kResend);
    guest.leaveEdx(left - 1);
    guest.setDword(waiting + packet::kResend, left - 1);
    if (sign(left) > 0)
        return 1;
    if (traceLevel(guest) >= 3)
    {
        const reg32 ecx = cpu.ecx;
        trace(guest, 0x550fa0, waiting + packet::kNumber, 0, 0, 1, guest.byte(self + rely::kId));
        guest.leaveEcx(ecx);
    }
    return guest.call(kPacketControl, args({ guest.dword(self + rely::kFirstPacket), self,
                                             guest.dword(waiting + packet::kNumber) & kSequenceMask }));
}

void frontend::netcPacketSendNak(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51caf0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 waiting = cpu.eax, self = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetSendNak(app, cpu, waiting, self); });
}

/* sub_51cb50: what the other end has now: what it acknowledged taken off those waiting. */
x86::reg32 frontend::netc::packetGotAck(App* app, CPU& cpu, reg32 self, reg32 ack)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = ack;
    const reg32 frame = guest.frame();
    const reg32 acked = loaded<0x51c7a0, sequenceWiden>(app, cpu, guest.dword(self + rely::kAcked), guest.edx() & 0xff,
                                                         kSequenceMask);
    guest.setDword(frame, acked);
    if (acked < guest.dword(self + rely::kNextSend) && acked > guest.dword(self + rely::kAcked))
    {
        guest.setDword(self + rely::kAcked, acked);
        if (traceLevel(guest) >= 3)
            trace(guest, 0x550fa8, frame, 0, 0, 0, guest.byte(self + rely::kId));
    }
    else if (traceLevel(guest) >= 3)
    {
        trace(guest, 0x550fb0, frame, 0, 0, 0, guest.byte(self + rely::kId));
    }
    const reg32 unacked = self + rely::kUnacked;
    for (;;)
    {
        const reg32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, cpu, unacked);
        if (first > guest.dword(self + rely::kAcked))
            return first;
        const reg32 done = loaded<0x50df60, frontend::netb::qpop>(app, cpu, unacked);
        if (done == 0)
            return 0;
        guest.call(kQueueAdd, args({ self + rely::kFree, done }));
    }
}

void frontend::netcPacketGotAck(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51cb50", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, ack = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetGotAck(app, cpu, self, ack); });
}

/* sub_51cc00: the other end wants a packet again: its time to resend is now. */
x86::reg32 frontend::netc::packetGotNak(App* app, CPU& cpu, reg32 self, reg32 sequence)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = sequence;
    const reg32 frame = guest.frame();
    const reg32 unacked = self + rely::kUnacked;
    const reg32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, cpu, unacked);
    const reg32 number = loaded<0x51c7a0, sequenceWiden>(app, cpu, first, guest.edx() & 0xff, kSequenceMask);
    guest.setDword(frame, number);
    const reg32 found = guest.call(kQueueFindKey, args({ unacked, number }));
    if (found == 0)
        return 0;
    if (traceLevel(guest) >= 3)
        trace(guest, 0x550fb8, frame, 0, 0, 1, guest.byte(self + rely::kId));
    guest.setDword(found + packet::kResend, 0);
    return guest.eax();
}

void frontend::netcPacketGotNak(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51cc00", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 self = cpu.eax, sequence = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetGotNak(app, cpu, self, sequence); });
}

/* sub_51cc80: the reliable layer's connection closed. */
x86::reg32 frontend::netc::packetShutdown(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    return shutdownLayer(guest, guest.eax(), rely::kId, 0x550fc0, 0x550fd0, 0x550fdc, 0x550fe8);
}

void frontend::netcPacketShutdown(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51cc80", 0, 0, nullptr, 0, kAll };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetShutdown(app, cpu, self); });
}

/* sub_51cd50: the first frame found in what the stream gave: its acknowledgement
 * seen to and its data kept for its turn, or a resend asked for or a close taken.
 * What was used of the buffer; frames that were not whole counted as bad. */
x86::reg32 frontend::netc::packetReceiveFrame(App* app, CPU& cpu, reg32 self, reg32 counted, reg32 buffer, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self; cpu.edx = counted; cpu.ebx = buffer; cpu.ecx = length;
    const reg32 connection = guest.dword(self + layer::kLink);
    const std::uint8_t reliable = guest.byte(guest.dword(self + layer::kStream) + stream::kReliable);
    guest.leaveEbx(length);
    reg32 used = 0, whole = 0;
    if (reliable == 0)
    {
        const reg32 end = length + buffer;
        guest.leaveEdx(end);
        for (reg32 at = buffer; at < end && guest.byte(at) != kFrameStart; ++at)
            ++used;
    }
    else
    {
        guest.leaveEdx((guest.edx() & 0xffffff00) | reliable);
    }
    const reg32 frame = used + buffer;
    if (guest.byte(frame) != kFrameStart)
        return length;
    const auto finished = [&]() -> reg32 {
        guest.leaveEdx(whole);
        const reg32 bad = guest.dword(self + rely::kBad) + (used > whole ? 1 : 0);
        guest.setDword(self + rely::kBad, bad);
        guest.leaveEcx(bad);
        return used;
    };
    reg32 end = used + packet::kFrameHeader;
    if (end > length)
        return finished();
    const reg32 size = guest.byte(frame + 1);
    if (size > guest.dword(self + layer::kMaxSize))
    {
        used += size == kFrameStart ? 1 : 2;
        return finished();
    }
    end += size;
    if (end > length)
        return finished();
    const reg32 bytes = end - used;
    guest.leaveEcx(bytes);
    if (guest::direct<0x51c7f0, packetChecksumGood>(app, cpu, frame, bytes) == 0)
    {
        used += 3;
        return finished();
    }
    if (guest.dword(connection + link::kTimeout) != 0)
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
    guest.setDword(self + rely::kReceived, guest.dword(self + rely::kReceived) + 1);
    const reg32 ack = guest.byte(frame + 3);
    used += bytes;
    if (ack <= kSequenceMask)
        guest::direct<0x51cb50, packetGotAck>(app, cpu, self, ack);
    whole = bytes;
    const reg32 dataLength = guest.byte(frame + 1), sequence = guest.byte(frame + 2);
    guest.leaveEbx((cpu.ebx & 0xffffff00) | dataLength);
    if (dataLength != 0)
        guest.leaveEbx((cpu.ebx & 0xffff00ff) | (sequence << 8));
    if (dataLength != 0 && sequence <= kSequenceMask)
    {
        const reg32 number = loaded<0x51c7a0, sequenceWiden>(app, cpu, guest.dword(self + rely::kNextDeliver), sequence,
                                                              kSequenceMask);
        // The game's code hands it the queue in ebx too, which it does not take; there it stays.
        guest.leaveEbx(self + rely::kReceiving);
        const reg32 slot = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, cpu, self + rely::kReceiving, number);
        guest.setByte(self + rely::kAckSent, 0);
        if (slot != 0)
        {
            guest.call(kMoveMemory, args({ frame, slot + packet::kFrame, bytes }));
            loaded<0x50e0a0, frontend::netb::qinsert>(app, cpu, self + rely::kReceivedQueue, slot);
            guest.setDword(counted, guest.dword(counted) + 1);
            if (traceLevel(guest) >= 3)
                trace(guest, 0x550ff8, slot + packet::kNumber, slot + packet::kData, guest.byte(slot + packet::kLength),
                      0, guest.byte(self + rely::kId));
        }
    }
    else if (sequence <= kSequenceMask)
    {
        guest.leaveEcx((cpu.ecx & 0xffffff00) | sequence);
        guest::direct<0x51cc00, packetGotNak>(app, cpu, self, sequence);
    }
    else if (sequence == kFrameClose)
    {
        guest.leaveEcx((cpu.ecx & 0xffffff00) | sequence);
        if (traceLevel(guest) >= 2)
            traceEvent(guest, 0x551000, 1, guest.byte(self + rely::kId));
        guest::direct<0x51cc80, packetShutdown>(app, cpu, self);
    }
    return finished();
}

void frontend::netcPacketReceiveFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51cd50", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 self = cpu.eax, counted = cpu.edx, buffer = cpu.ebx, length = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetReceiveFrame(app, cpu, self, counted, buffer, length); });
}

/* sub_51cf30: all that came for the reliable layer. */
x86::reg32 frontend::netc::packetReceive(App* app, CPU& cpu, reg32 self)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = self;
    return receiveFrames<0x51cd50, packetReceiveFrame>(guest, cpu, rely::kBuffer, rely::kBufferSize, rely::kHeld);
}

void frontend::netcPacketReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51cf30", 0, 0, nullptr, 0, kAll };
    const reg32 self = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetReceive(app, cpu, self); });
}

/* sub_51d000: what the reliable layer's queues keep packets in the order of. */
x86::reg32 frontend::netc::packetQueueKey(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = packet;
    return guest.dword(guest.eax() + packet::kNumber);
}

void frontend::netcPacketQueueKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d000", 0, 0, nullptr, 0, kAll };
    const reg32 packet = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetQueueKey(app, cpu, packet); });
}

/* sub_51d010: a packet's time to resend counted down.  1 when it is up. */
x86::reg32 frontend::netc::packetCountDown(App* app, CPU& cpu, reg32 waiting)
{
    Guest guest(app, cpu);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = waiting;
    const reg32 left = guest.dword(waiting + packet::kResend);
    guest.leaveEdx(left);
    if (sign(left) <= 0)
        return 1;
    guest.setDword(waiting + packet::kResend, left - 1);
    return sign(left - 1) <= 0 ? 1 : 0;
}

void frontend::netcPacketCountDown(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d010", 0, 0, nullptr, 0, kEcx };
    const reg32 waiting = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetCountDown(app, cpu, waiting); });
}

/* sub_51d040: a packet sent again if its time is up, else on to the next.  The
 * original goes on to sub_51c8f0 by a jump: what returns from there returns from here. */
x86::reg32 frontend::netc::packetResendDue(App* app, CPU& cpu, reg32 waiting, reg32 self)
{
    Guest guest(app, cpu);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = waiting; cpu.edx = self;
    // Called as C++, the jump a call: sub_51c8f0's frame 4 bytes lower than the game's.
    if (sign(guest.dword(waiting + packet::kResend)) <= 0)
        return guest::direct<0x51c8f0, packetSend>(app, cpu, waiting, self);
    return 1;
}

void frontend::netcPacketResendDue(win32::WinApplication* app, x86::CPU& cpu)
{
    if (sign(app->getMemory<reg32>(cpu.eax + packet::kResend)) <= 0)
    {
        app->dynamic_call(kPacketSend, cpu);
        return;
    }
    static Site site = { "sub_51d040", 0, 0, nullptr, 0, kAll };
    const reg32 waiting = cpu.eax, self = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetResendDue(app, cpu, waiting, self); });
}

/* sub_51d050: the stream says something came: read while there is, what is next
 * in order delivered, and a resend asked of a gap. */
x86::reg32 frontend::netc::packetNotify(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x34);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    const reg32 self = guest.dword(connection + link::kLayer);
    const reg32 ops = guest.dword(self + layer::kStream);
    if (traceLevel(guest) >= 5)
        traceEvent(guest, 0x551010, 0, guest.byte(self + rely::kId));
    guest.setDword(self + rely::kBad, 0);
    if (guest.dword(connection + link::kTimeout) == 0)
        return connection;
    const reg32 received = self + rely::kReceivedQueue, unacked = self + rely::kUnacked;
    const reg32 receiving = self + rely::kReceiving;
    for (;;)
    {
        reg32 more = guest.call(guest.dword(ops + stream::kAvailable), args({ connection, ops }));
        if (more != 0)
            more = guest::direct<0x51cf30, packetReceive>(app, cpu, self);
        reg32 next = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, cpu, received, guest.dword(self + rely::kNextDeliver));
        while (next != 0)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, 0x55101c, next + packet::kNumber, next + packet::kData, guest.byte(next + packet::kLength),
                      0, guest.byte(self + rely::kId));
            // By address: the ebx and edx it leaves are handed to the calls after it.
            if (guest.call(kDeliver, args({ connection, next + packet::kData, guest.byte(next + packet::kLength) })) == 0)
            {
                if (traceLevel(guest) >= 3)
                    trace(guest, 0x551024, next + packet::kNumber, 0, 0, 1, guest.byte(self + rely::kId));
                loaded<0x50e0a0, frontend::netb::qinsert>(app, cpu, received, next);
                break;
            }
            const reg32 locked = fe::direct<0x50ea20, frontend::menz::fileLockEnter>(app, cpu, unacked);
            const reg32 number = guest.dword(self + rely::kNextFree);
            guest.leaveEcx(number + 1);
            guest.setDword(self + rely::kNextFree, number + 1);
            guest.setDword(next + packet::kNumber, number);
            guest.setDword(self + rely::kNextDeliver, guest.dword(self + rely::kNextDeliver) + 1);
            fe::direct<0x50ea40, frontend::menz::fileLockLeave>(app, cpu, unacked, locked);
            guest.setDword(next + packet::kResend, 0);
            loaded<0x50e0a0, frontend::netb::qinsert>(app, cpu, receiving, next);
            next = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, cpu, received, guest.dword(self + rely::kNextDeliver));
        }
        if (more == 0)
            break;
    }
    reg32 gap = 0;
    if (guest.byte(self + rely::kAckSent) == 0)
    {
        const reg32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, cpu, received);
        if (first != 0xffffffff && first != guest.dword(self + rely::kNextDeliver))
            gap = 1;
        guest.setByte(self + rely::kAckSent, 1);
    }
    if (guest.dword(self + rely::kBad) == 0 && gap == 0)
        return 0;
    if (traceLevel(guest) >= 4)
        traceEvent(guest, guest.dword(self + rely::kBad) != 0 ? 0x55102c : 0x551034, 1, guest.byte(self + rely::kId));
    const reg32 waiting = guest.call(kQueueFindIndex, args({ receiving, 0 }));
    if (waiting == 0)
        return 0;
    return guest::direct<0x51caf0, packetSendNak>(app, cpu, waiting, self);
}

void frontend::netcPacketNotify(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d050", 0, 0, nullptr, 0, kAll };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetNotify(app, cpu, connection); });
}

/* sub_51d270: a connection opened on the reliable layer: the record made unless
 * it has one, its packets made and queued, the stream asked, and waited for until
 * it stands or has failed.  1 if it stands; with no connection, the record's size. */
x86::reg32 frontend::netc::packetOpen(App* app, CPU& cpu, reg32 connection, reg32 ops, reg32 asked, reg32 windowBits)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x38);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection; cpu.edx = ops; cpu.ebx = asked; cpu.ecx = windowBits;
    constexpr reg32 kFile = 0x55103c, kFunction = 0x55104c;
    reg32 wanted = asked;
    guest.leaveEbx(0);
    if (sign(asked) < 6)
    {
        wanted = 6;
    }
    else if (sign(asked) <= 0xfa)
    {
        const reg32 limit = guest.word(ops + stream::kMaxFrame);
        guest.leaveEbx(limit);
        if (limit != 0 && asked + packet::kFrameHeader > limit)
        {
            reportError(guest, kFile, kFunction, 0x207, 0x5510a4);
            guest.leaveEbx(kFunction);
        }
    }
    else
    {
        reportError(guest, kFile, kFunction, 0x205, 0x551064, 0xfa);
    }
    if (connection != 0 && guest.dword(connection + link::kTimeout) != 0)
        reportError(guest, kFile, kFunction, 0x20a, 0x5510f4);
    if (sign(windowBits) < 2)
        windowBits = 2;
    else if (sign(windowBits) > 7)
        windowBits = 7;
    guest.leaveEcx(windowBits);
    guest.leaveEbx(wanted);
    const reg32 least = guest.byte(ops + stream::kMinFrame);
    if (sign(least) > sign(wanted))
        wanted = least;
    const reg32 half = reg32(sign(reg32(1) << (windowBits & 31)) / 2);
    const reg32 bufferSize = guest.byte(ops + stream::kReliable) == 0 ? 0x200 : 0x100;
    const reg32 step = (wanted + 0x15) & 0xfffffffc;
    const reg32 total = step * (half + half - 1) + rely::kSize + bufferSize;
    guest.leaveEdx(total);
    if (connection == 0)
        return total;
    reg32 owned = 0;
    reg32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
    {
        guest.setDword(kErrorFile, kFile);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x22b);
        owned = 1;
        self = guest.call(kAllocate, args({ 0x55112c, total, guest.dword(kMemoryType), 1 }));
        if (self == 0)
        {
            guest.leaveEdx(0);
            return 0;
        }
    }
    reg32 place = self + rely::kSize;
    guest.setDword(connection + link::kState, 0);
    guest.setDword(connection + link::kTimeout, 0);
    const reg32 oldDrop = guest.dword(connection + link::kDrop);
    guest.setDword(connection + link::kLayer, self);
    guest.setDword(connection + link::kDrop, kDropFunction);
    guest.setDword(self + layer::kLink, connection);
    guest.setDword(self + layer::kMaxSize, wanted);
    guest.setDword(self + rely::kIdleLeft, guest.word(connection + link::kRetries));
    guest.setByte(self + layer::kOwned, std::uint8_t(owned));
    guest.setDword(self + layer::kStream, ops);
    const reg32 id = guest.dword(kNextPacket);
    guest.setByte(self + rely::kId, std::uint8_t(id));
    guest.setDword(kNextPacket, id + 1);
    guest.setDword(self + rely::kReceived, 0);
    const reg32 lock = reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    guest.setDword(self + rely::kNextSend, 1);
    guest.setDword(self + rely::kAcked, 0);
    guest.setDword(self + rely::kLock, lock);
    loaded<0x50dcd0, frontend::netb::qinit>(app, cpu, self + rely::kFree, 0, 0);
    loaded<0x50dcd0, frontend::netb::qinit>(app, cpu, self + rely::kUnacked, kQueueKey, self);
    guest.setDword(self + rely::kNextDeliver, 1);
    guest.setDword(self + rely::kNextFree, 1);
    loaded<0x50dcd0, frontend::netb::qinit>(app, cpu, self + rely::kReceiving, kQueueKey, self);
    loaded<0x50dcd0, frontend::netb::qinit>(app, cpu, self + rely::kReceivedQueue, kQueueKey, self);
    for (reg32 i = half - 1; i != 0; --i)
    {
        const reg32 at = place;
        guest.setDword(at, 0);
        guest.setDword(at + packet::kNumber, 0);
        guest.setDword(at + packet::kResend, 0);
        place += step;
        loaded<0x50de50, frontend::netb::qpush>(app, cpu, self + rely::kFree, at);
    }
    for (reg32 i = half - 1; i != 0; --i)
    {
        const reg32 at = place;
        guest.setDword(at, 0);
        place += step;
        const reg32 number = guest.dword(self + rely::kNextFree);
        guest.leaveEbx(number + 1);
        guest.setDword(self + rely::kNextFree, number + 1);
        guest.setDword(at + packet::kNumber, number);
        guest.setDword(at + packet::kResend, 0);
        loaded<0x50e0a0, frontend::netb::qinsert>(app, cpu, self + rely::kReceiving, at);
    }
    guest.setDword(self + rely::kHeld, 0);
    guest.setDword(self + rely::kBuffer, place + step);
    guest.setDword(self + rely::kFirstPacket, place);
    guest.setDword(self + rely::kBufferSize, bufferSize);
    if (guest.dword(kConnectionTimeout) == 0)
    {
        reportError(guest, kFile, kFunction, 0x276, 0x551134);
        guest.leaveEbx(kFunction);
    }
    guest.leaveEcx(ops);
    reg32 failed = 0;
    if (guest.call(guest.dword(ops + stream::kOpen), args({ connection, kPacketNotify })) == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, 0x551188, 0, guest.byte(self + rely::kId));
    }
    else
    {
        guest.setDword(connection + link::kTimeout, kNeverTimesOut);
        guest.call(kQueueAdd, args({ kActiveConnections, connection }));
        if (traceLevel(guest) >= 1)
            traceEvent(guest, 0x55115c, 0, guest.byte(self + rely::kId));
        for (;;)
        {
            if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) == 0)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
                failed = guest.call(guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            else if (sign(guest.call(kWaitSystemTasks, args({ 1 }))) < 0)
            {
                failed = 1;
            }
            else
            {
                failed = guest.call(guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            guest.leaveEdx(failed);
            if (failed != 0 && traceLevel(guest) >= 2)
                traceEvent(guest, 0x551168, 0, guest.byte(self + rely::kId));
            if (guest.dword(connection + link::kTimeout) != 0)
                guest.setDword(connection + link::kTimeout, kNeverTimesOut);
            if (guest.dword(connection + link::kTimeout) == 0)
                break;
            const reg32 received = guest.dword(self + rely::kReceived);
            guest.leaveEdx(received);
            if (received != 0 || received != failed)
                break;
        }
        if (guest.dword(connection + link::kTimeout) == 0 || failed != 0)
        {
            if (guest.dword(connection + link::kTimeout) == 0 && traceLevel(guest) >= 2)
                traceEvent(guest, 0x551178, 0, guest.byte(self + rely::kId));
            guest.setDword(connection + link::kTimeout, 0);
            guest.call(guest.dword(ops + stream::kClose), args({ connection, ops }));
        }
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        guest.leaveEbx(failed);
        if (failed == 0)
        {
            guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
            guest.setDword(connection + link::kDrop, oldDrop);
            if (traceLevel(guest) >= 1)
                traceEvent(guest, 0x551198, 0, guest.byte(self + rely::kId));
            guest.leaveEdx(1);
            return 1;
        }
    }
    if (traceLevel(guest) >= 1)
        traceEvent(guest, 0x5511ac, 0, guest.byte(self + rely::kId));
    guest.call(kQueueRemove, args({ kActiveConnections, connection }));
    guest.call(kQueueRemove, args({ kClosedConnections, connection }));
    if (owned != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    guest.leaveEdx(0);
    return 0;
}

void frontend::netcPacketOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d270", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 connection = cpu.eax, ops = cpu.edx, asked = cpu.ebx, windowBits = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetOpen(app, cpu, connection, ops, asked, windowBits); });
}

/* sub_51d810: data sent on the reliable layer: a free packet taken, numbered, sent
 * and kept until acknowledged.  The port's trace of it first. */
x86::reg32 frontend::netc::packetSubmit(App* app, CPU& cpu, reg32 connection, reg32 data, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection; cpu.edx = data; cpu.ebx = length;
    constexpr reg32 kFile = 0x55103c, kFunction = 0x5511c0;
    if (connection == 0 || guest.dword(connection + link::kTimeout) == 0)
        return 0;
    const reg32 self = guest.dword(connection + link::kLayer);
    const reg32 free = loaded<0x50df60, frontend::netb::qpop>(app, cpu, self + rely::kFree);
    if (length > guest.dword(self + layer::kMaxSize))
    {
        reportError(guest, kFile, kFunction, 0x2ba, 0x5511d4);
        guest.leaveEbx(kFunction);
    }
    if (free == 0)
    {
        if (traceLevel(guest) >= 2)
            trace(guest, 0x55125c, 0, data, length, 1, guest.byte(self + rely::kId));
        return 0;
    }
    guest.call(kPacketBuild, args({ free, self, data, length }));
    guest::direct<0x51c8f0, packetSend>(app, cpu, free, self);
    guest.call(kQueueAddSorted, args({ self + rely::kUnacked, free }));
    if (guest.dword(self + rely::kNextSend) == 0)
    {
        reportError(guest, kFile, kFunction, 0x2c6, 0x551208);
        guest.leaveEbx(kFunction);
    }
    return 1;
}

void frontend::netcPacketSubmit(win32::WinApplication* app, x86::CPU& cpu)
{
    if (commSendTrace(app, cpu))
        return;
    static Site site = { "sub_51d810", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 connection = cpu.eax, data = cpu.edx, length = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetSubmit(app, cpu, connection, data, length); });
}

/* sub_51d8f0: the reliable layer's turn: counts its timeout down, resends what is
 * due, says it is alive when it has been quiet, closes on timing out.  Always 1. */
x86::reg32 frontend::netc::packetProcess(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    const reg32 self = guest.dword(connection + link::kLayer);
    guest.leaveEdx(guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
        traceEvent(guest, 0x551268, 0, guest.byte(self + rely::kId));
    const reg32 left = guest.dword(connection + link::kTimeout) - 1;
    guest.setDword(connection + link::kTimeout, left);
    guest.leaveEbx(left);
    if (left == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, 0x551274, 0, guest.byte(self + rely::kId));
        guest.setDword(connection + link::kTimeout, guest.call(guest.dword(connection + link::kTimedOut), args({ connection })));
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        const reg32 ops = guest.dword(self + layer::kStream);
        if (guest.call(guest.dword(ops + stream::kReady), args({ connection, ops })) != 0)
        {
            loaded<0x50e9b0, frontend::netb::qsafeiterate>(app, cpu, self + rely::kUnacked, kCountDown, self);
            loaded<0x50e8e0, frontend::netb::qiterate>(app, cpu, self + rely::kUnacked, kResendDue, self);
            const reg32 idle = guest.dword(self + rely::kIdleLeft);
            guest.setDword(self + rely::kIdleLeft, idle - 1);
            if (sign(idle) <= 0)
                guest::direct<0x51ca50, packetSendControl>(app, cpu, guest.dword(self + rely::kFirstPacket), self, kFrameControlAck);
            return 1;
        }
    }
    if (traceLevel(guest) >= 2)
        trace(guest, 0x551280, connection + link::kTimeout, 0, 0, 1, guest.byte(self + rely::kId));
    guest::direct<0x51cc80, packetShutdown>(app, cpu, self);
    return 1;
}

void frontend::netcPacketProcess(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d8f0", 0, 0, nullptr, 0, kAll };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetProcess(app, cpu, connection); });
}

/* sub_51d9e0: the reliable layer's connection closed by us: the other end told if
 * it can be, the record gone.  1 if it was there. */
x86::reg32 frontend::netc::packetClose(App* app, CPU& cpu, reg32 connection)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = connection;
    if (connection == 0)
        return 0;
    const reg32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
        return 0;
    const reg32 ops = guest.dword(self + layer::kStream);
    guest.leaveEcx(guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 1)
        traceEvent(guest, 0x55128c, 0, guest.byte(self + rely::kId));
    if (guest.call(kQueueRemove, args({ kActiveConnections, connection })) == 0)
    {
        if (guest.dword(connection + link::kTimeout) != 0)
            guest.call(kMonoPrintf, args({ 0 }, { 0x55129c }));
    }
    else
    {
        if (guest.dword(connection + link::kTimeout) != 0)
        {
            guest.setDword(connection + link::kTimeout, 0);
            reg32 tries = 4;
            for (;;)
            {
                --tries;
                guest.leaveEcx(tries);
                if (tries == 0xffffffff
                    || guest.call(guest.dword(ops + stream::kReady), args({ connection })) == 0)
                    break;
                // By address: the ebx and edx it leaves are handed to the stream's functions after it.
                if (guest.call(kPacketControl, args({ guest.dword(self + rely::kFirstPacket), self, kFrameClose })) != 0)
                    tries = 0;
                else
                    loaded<0x4f06e0, frontend::wata::timerWait>(app, cpu, 1);
            }
            loaded<0x4f06e0, frontend::wata::timerWait>(app, cpu, 3);
            guest.call(guest.dword(ops + stream::kClose), args({ connection }));
        }
        fe::direct<0x50de50, netb::qpush>(app, cpu, kClosedConnections, connection);
    }
    if (guest.call(kQueueRemove, args({ kClosedConnections, connection })) == 0)
        return 0;
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), self + rely::kFree);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), self + rely::kUnacked);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), self + rely::kReceiving);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), self + rely::kReceivedQueue);
    reg32(*fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(self + rely::kLock)));
    if (guest.byte(self + layer::kOwned) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 1;
}

void frontend::netcPacketClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51d9e0", 0, 0, nullptr, 0, kAll };
    const reg32 connection = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::packetClose(app, cpu, connection); });
}

/* ---------------------------------------------------------------------------
 * win\comm.c: the serial ports, their thread, and a modem's talk
 * ------------------------------------------------------------------------- */

namespace
{

constexpr reg32 kModemWait = 0x56aab8, kModemAttentionText = 0x56aabc, kModemTable = 0x56aac0, kModemRowSize = 12;
constexpr reg32 kModemResponseText = 0x56abb4, kModemRows = 0x56abb8, kModemConnected = 0x56abbc;
constexpr reg32 kModemEcho = 0x56abb0, kOpenPorts = 0x56abc0, kPortsEvent = 0x56abc4, kPortsLock = 0x56abc8;
constexpr reg32 kBaudRates = 0x56abcc, kClockTicks = 0x79c1a0, kClockSeconds = 0x79c1a4;
constexpr reg32 kModemEchoBuffer = 0xa0b160, kPorts = 0xa0b1a0, kThreadHandle = 0xa0b1e0;
constexpr reg32 kErrorIoPending = 0x3e5, kModemFailed = 0xfffffffd, kSourceFile = 0x550b9c;

constexpr reg32 kCloseAllPorts = 0x51ab40;
constexpr reg32 kSerialThreadEntry = 0x51af10;
constexpr reg32 kPortWrite = 0x51b5f0;
constexpr reg32 kPortRead = 0x51ba50, kWaitIn = 0x51bbd0;

/* A serial port's record, 0x450 bytes, with its two rings of 0x200. */
namespace serial
{
constexpr reg32 kCommand = 0x00, kHandle = 0x04, kReading = 0x08, kInFirst = 0x0c, kInCount = 0x10, kInGot = 0x14;
constexpr reg32 kInOverlapped = 0x18, kInEvent = 0x28, kInRing = 0x2c, kWriting = 0x22c, kOutFirst = 0x230;
constexpr reg32 kOutCount = 0x234, kOutSent = 0x238, kOutOverlapped = 0x23c, kOutEvent = 0x24c, kOutRing = 0x250;
constexpr reg32 kRecordSize = 0x450, kRingSize = 0x200, kRingMask = 0x1ff;
constexpr reg32 kCommandWake = 1, kCommandDropInput = 2, kCommandDropOutput = 3, kCommandClose = 4;
}

reg32 portRecord(const Guest& guest, reg32 number)
{
    return guest.dword(kPorts + number * 4);
}

/* The ports' lock, made the first time it is asked for. */
void ensurePortsLock(Guest& guest, x86::CPU& cpu)
{
    if (guest.dword(kPortsLock) == 0)
        guest.setDword(kPortsLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(guest.app(), cpu)));
}

void enterPorts(Guest& guest, x86::CPU& cpu)
{
    callImport<kEnterLock, &host::enterLock>(guest, cpu, guest.dword(kPortsLock));
}

reg32 leavePorts(Guest& guest, x86::CPU& cpu)
{
    return callImport<kLeaveLock, &host::leaveLock>(guest, cpu, guest.dword(kPortsLock));
}

/* The modem's patience, a quarter of a second, worked out once. */
void ensureModemWait(Guest& guest)
{
    if (guest.dword(kModemWait) == 0)
        guest.setDword(kModemWait, reg32(sign(guest.dword(kTicksPerSecond)) / 4));
}

/* A line for the modem's log file. */
void modemLog(Guest& guest, reg32 text)
{
    if (guest.dword(kModemLog) != 0)
        guest.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.cpu(), kModemLog, 1u, text, guest::Words({}));
}

/* Time let pass, as a thread or not.  False if the game's tasks say stop. */
bool modemPause(Guest& guest, x86::CPU& cpu, reg32 ticks)
{
    if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(guest.app(), cpu, 0)) == 0)
    {
        fe::direct<0x4df8e0, ea::sleepFor>(guest.app(), cpu, ticks);
        return true;
    }
    return sign(guest.call(kWaitSystemTasks, args({ ticks }))) >= 0;
}

}  // namespace

/* sub_51a470: a modem's answer read a character at a time until one of the table's
 * words is in it or it ends in a number.  Its number; -1 for none, -3 when told to stop. */
x86::reg32 frontend::netc::modemResponse(App* app, CPU& cpu, reg32 number, reg32 timeout)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x54);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = timeout;
    const reg32 text = guest.frame();
    guest.leaveEdx(0);
    if (guest.dword(kModemWait) == 0)
    {
        const reg32 ticks = guest.dword(kTicksPerSecond);
        guest.leaveEdx(sign(ticks) < 0 ? 0xfffffffc : 0);
        ensureModemWait(guest);
    }
    guest.setDword(kModemResponseText, 0);
    const reg32 deadline = guest.dword(kClockSeconds) + timeout;
    reg32 firstDigit = 0, last = text - 1, count = 0;
    do
    {
        if (sign(guest.dword(kClockSeconds)) >= sign(deadline) || guest.dword(kModemAbort) != 0)
            break;
        if (loaded<0x51bbd0, serialWaitIn>(app, cpu, number, 1, text + count, 1) != 0)
        {
            ++last;
            guest.leaveEdx(last);
            const std::uint8_t got = guest.byte(text + count);
            ++count;
            bool digit = false;
            if (got < 0x20)
                guest.setByte(text + count - 1, 0x5f);
            else if (got >= 0x30 && got <= 0x39)
                digit = true;
            if (firstDigit == 0 && digit)
                firstDigit = last;
            guest.setByte(text + count, 0);
            std::int32_t row = -1;
            for (reg32 column = 0; column < 2; ++column)
            {
                for (row = sign(guest.dword(kModemRows)) - 1; row >= 0; --row)
                {
                    const reg32 entry = kModemTable + reg32(row) * kModemRowSize;
                    if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.cpu(), text, guest.dword(entry + column * 4)) == 0)
                        continue;
                    guest.setDword(kModemConnected, guest.dword(kModemConnected) | (column > 0 ? 1 : 0));
                    if (guest.dword(kModemLog) != 0)
                    {
                        guest.leaveEdx(guest.dword(entry));
                        guest.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.cpu(), kModemLog, 1u, 0x550adcu,
                                                                                guest::Words({ text, guest.dword(entry) }));
                    }
                    if (!modemPause(guest, cpu, guest.dword(kModemWait)))
                    {
                        guest.setDword(kModemAbort, 1);
                        return kModemFailed;
                    }
                    const reg32 word = guest.dword(entry);
                    guest.setDword(kModemResponseText, word);
                    guest.leaveEdx(word);  // as the code loads it for it, which it keeps
                    return fe::direct<0x4f0430, watcom::clib::atoi>(app, cpu, guest.dword(entry + 8));
                }
            }
            if (firstDigit != 0 && !digit && guest.dword(kModemConnected) != 0)
            {
                const std::int32_t said = sign(fe::direct<kAtoi, watcom::clib::atoi>(guest.app(), guest.cpu(), firstDigit));
                const std::int32_t code = said >= 10 ? 1 : said >= 0 ? said : -1;
                guest.leaveEdx(reg32(code));
                if (guest.dword(kModemLog) != 0 && code >= 0)
                    guest.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.cpu(), kModemLog, 1u, 0x550adcu,
                        guest::Words({ firstDigit, guest.dword(kModemTable + reg32(code) * kModemRowSize) }));
                if (code >= 0)
                    guest.setDword(kModemResponseText, guest.dword(kModemTable + reg32(row) * kModemRowSize));
                if (!modemPause(guest, cpu, guest.dword(kModemWait)))
                {
                    guest.setDword(kModemAbort, 1);
                    return kModemFailed;
                }
                return reg32(code);
            }
        }
        reg32 stop = 0;
        if (reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0)
            stop = sign(reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0))) < 0 ? 1 : 0;
        else
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
        guest.setDword(kModemAbort, guest.dword(kModemAbort) | stop);
    } while (sign(count) < 0x29);
    return guest.dword(kModemAbort) != 0 ? kModemFailed : 0xffffffff;
}

void frontend::netcModemResponse(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51a470", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, timeout = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::modemResponse(app, cpu, number, timeout); });
}

/* sub_51a7b0: a command said to the modem, with or without its input thrown away
 * first, and its echo waited for.  1 if it came. */
x86::reg32 frontend::netc::modemCommand(App* app, CPU& cpu, reg32 number, reg32 command, reg32 drop)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = command; cpu.ebx = drop;
    const reg32 length = guest.length(command);
    guest.leaveEdx(0);
    guest.setDword(kModemEcho, 0);
    if (sign(length) > 0x31)
        return 0;
    if (drop != 0)
        guest::direct<0x51b1e0, serialDropOutput>(app, cpu, number);
    const auto stopped = [&]() -> reg32 {
        guest.leaveEbx(1);
        guest.setDword(kModemAbort, 1);
        return 0;
    };
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    const bool thread = reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0;
    const std::int32_t ticks = sign(guest.dword(kTicksPerSecond));
    const reg32 tenth = reg32(ticks / 10);
    guest.leaveEdx(reg32(ticks % 10));
    if (!thread)
    {
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, tenth);
    }
    else if (sign(guest.call(kWaitSystemTasks, args({ tenth }))) < 0)
    {
        return stopped();
    }
    // The game's code hands it edx as it is and the command in ebx too, which it does not read; the call after
    // loads them again.
    guest::direct<0x51b1c0, serialDropInput>(app, cpu, number);
    guest.call(kPortWrite, args({ number, length, command }));
    guest.call(kPortWrite, args({ number, 1, 0x550b04 }));
    const reg32 start = guest.dword(kClockSeconds);
    const std::int32_t rate = sign(guest.dword(kTicksPerSecond));
    const reg32 deadline = start + reg32(rate / 5);
    guest.leaveEdx(reg32(rate % 5));
    reg32 got = 0;
    while (sign(guest.dword(kClockSeconds)) < sign(deadline) && guest.dword(kModemEcho) == 0 && guest.dword(kModemAbort) == 0)
    {
        got += guest.call(kWaitIn, args({ number, 1, kModemEchoBuffer + got, 1 }));
        guest.setByte(got + kModemEchoBuffer, 0);
        guest.setDword(kModemEcho, fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.cpu(), kModemEchoBuffer, command));
    }
    if (guest.dword(kModemLog) != 0)
    {
        const reg32 echo = guest.dword(kModemEcho);
        guest.leaveEbx(echo);
        if (echo != 0)
            guest.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.cpu(), kModemLog, 1u, 0x550b08u, guest::Words({ echo }));
        else
            guest.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.cpu(), kModemLog, 1u, 0x550b1cu, guest::Words({}));
    }
    return guest.dword(kModemEcho) != 0 ? 1 : 0;
}

void frontend::netcModemCommand(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51a7b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, command = cpu.edx, drop = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::modemCommand(app, cpu, number, command, drop); });
}

/* sub_51a930: the modem's attention got: the escape, its setup words, then AT and
 * what it answers.  1 if it did. */
x86::reg32 frontend::netc::modemAttention(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    const auto stopped = [&]() -> reg32 {
        guest.setDword(kModemAbort, 1);
        return 0;
    };
    const auto oneAndAHalf = [&]() -> reg32 {
        return reg32(sign(guest.dword(kTicksPerSecond) * 3) / 2);
    };
    modemLog(guest, 0x550b3c);
    ensureModemWait(guest);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    {
        const bool thread = reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0;
        if (!thread)
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, oneAndAHalf());
        else if (sign(guest.call(kWaitSystemTasks, args({ oneAndAHalf() }))) < 0)
            return stopped();
    }
    guest::direct<0x51b5f0, serialWrite>(app, cpu, number, 3, 0x550b58);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    {
        const bool thread = reg32(*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0)) != 0;
        if (!thread)
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, oneAndAHalf());
        else if (sign(guest.call(kWaitSystemTasks, args({ oneAndAHalf() }))) < 0)
            return stopped();
    }
    const reg32 attention = guest.dword(kModemAttentionText);
    guest::direct<0x51b5f0, serialWrite>(app, cpu, number, guest.length(attention), attention);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    if (!modemPause(guest, cpu, guest.dword(kModemWait)))
        return stopped();
    // The game's code hands it the command in edx too, which it does not read; the call after loads edx again.
    guest::direct<0x51b1c0, serialDropInput>(app, cpu, number);
    bool fine = guest::direct<0x51a7b0, modemCommand>(app, cpu, number, 0x550b5c, 0) != 0;
    if (fine)
        fine = guest::direct<0x51a470, modemResponse>(app, cpu, number, guest.dword(kTicksPerSecond) * 2) == 0;
    if (!fine)
    {
        modemLog(guest, 0x550b74);
        return 0;
    }
    modemLog(guest, 0x550b60);
    return 1;
}

void frontend::netcModemAttention(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51a930", 0, 0, nullptr, 0, kAll };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::modemAttention(app, cpu, number); });
}

/* sub_51ab40: at the game's end, every port still open closed. */
x86::reg32 frontend::netc::serialCloseAll(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    reg32 left = guest.eax();  // what the calls leave in eax: the last one's
    // The game's code hands it the number past it in edx too, which it does not read.
    for (reg32 number = 0; number < 0x10 && guest.dword(kOpenPorts) != 0; ++number)
        left = guest::direct<0x51b200, serialClose>(app, cpu, number);
    return left;
}

void frontend::netcSerialCloseAll(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51ab40", 0, 0, nullptr, 0, kEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialCloseAll(app, cpu); });
}

/* sub_51ab70: a command left for a port's thread, which is woken. */
x86::reg32 frontend::netc::serialCommand(App* app, CPU& cpu, reg32 number, reg32 command)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = command;
    enterPorts(guest, cpu);
    const reg32 record = portRecord(guest, number);
    if (record != 0)
    {
        guest.setDword(record + serial::kCommand, command);
        fe::direct<0x4dfb30, ea::eventSet>(app, cpu, guest.dword(kPortsEvent));
    }
    return leavePorts(guest, cpu);
}

void frontend::netcSerialCommand(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51ab70", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi) };
    const reg32 number = cpu.eax, command = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialCommand(app, cpu, number, command); });
}

/* sub_51abc0: the port's reads started until its ring is full, then its writes until none is left. */
x86::reg32 frontend::netc::serialStartIo(App* app, CPU& cpu, reg32 record)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = record;
    enterPorts(guest, cpu);
    while (guest.dword(record + serial::kReading) == 0)
    {
        const reg32 count = guest.dword(record + serial::kInCount);
        if (sign(count) >= sign(serial::kRingSize))
            break;
        const reg32 at = (guest.dword(record + serial::kInFirst) + count) & serial::kRingMask;
        const reg32 first = guest.dword(record + serial::kInFirst);
        const reg32 top = sign(at) >= sign(first) ? serial::kRingSize : first;
        for (reg32 i = 0; i < 4; ++i)
            guest.setDword(record + serial::kInOverlapped + 4 * i, 0);
        if (callImport<kReadFile, &host::readFile>(guest, cpu, guest.dword(record + serial::kHandle),
                                                              at + record + serial::kInRing, top - at, record + serial::kInGot,
                                                              record + serial::kInOverlapped)
            == 0)
        {
            const reg32 error = callImport<kGetLastError, &host::lastError>(guest, cpu);
            if (error == 0 || error == kErrorIoPending)
                guest.setDword(record + serial::kReading, 1);
            break;
        }
        guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) + guest.dword(record + serial::kInGot));
    }
    while (guest.dword(record + serial::kWriting) == 0)
    {
        const reg32 count = guest.dword(record + serial::kOutCount);
        if (sign(count) <= 0)
            break;
        const reg32 first = guest.dword(record + serial::kOutFirst);
        const reg32 length = sign(count + first) > sign(serial::kRingSize) ? serial::kRingSize - first : count;
        for (reg32 i = 0; i < 4; ++i)
            guest.setDword(record + serial::kOutOverlapped + 4 * i, 0);
        if (callImport<kWriteFile, &host::writeFile>(guest, cpu, guest.dword(record + serial::kHandle),
                                                                first + record + serial::kOutRing, length, record + serial::kOutSent,
                                                                record + serial::kOutOverlapped)
            == 0)
        {
            const reg32 error = callImport<kGetLastError, &host::lastError>(guest, cpu);
            if (error == 0 || error == kErrorIoPending)
                guest.setDword(record + serial::kWriting, 1);
            break;
        }
        const reg32 sent = guest.dword(record + serial::kOutSent);
        guest.setDword(record + serial::kOutCount, guest.dword(record + serial::kOutCount) - sent);
        guest.setDword(record + serial::kOutFirst, (guest.dword(record + serial::kOutFirst) + sent) & serial::kRingMask);
    }
    return leavePorts(guest, cpu);
}

void frontend::netcSerialStartIo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51abc0", 0, 0, nullptr, 0, kAll };
    const reg32 record = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialStartIo(app, cpu, record); });
}

/* sub_51ad70: the port's reads and writes under way looked at, those done taken in. */
x86::reg32 frontend::netc::serialFinishIo(App* app, CPU& cpu, reg32 record)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = record;
    const reg32 got = guest.frame();
    enterPorts(guest, cpu);
    if (guest.dword(record + serial::kReading) != 0 && guest.call(kPollEvent, args({ guest.dword(record + serial::kInEvent) })) != 0)
    {
        if (callImport<kOverlappedResult, &host::serialOverlappedResult>(guest, cpu, guest.dword(record + serial::kHandle), record + serial::kInOverlapped, got, 0) != 0)
        {
            const reg32 bytes = guest.dword(got);
            if (bytes != 0)
                guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) + bytes);
            guest.setDword(record + serial::kReading, 0);
        }
        else
        {
            guest.setDword(record + serial::kReading, callImport<kGetLastError, &host::lastError>(guest, cpu) == kErrorIoPending ? 1 : 0);
        }
    }
    if (guest.dword(record + serial::kWriting) != 0 && guest.call(kPollEvent, args({ guest.dword(record + serial::kOutEvent) })) != 0)
    {
        if (callImport<kOverlappedResult, &host::serialOverlappedResult>(guest, cpu, guest.dword(record + serial::kHandle), record + serial::kOutOverlapped, got, 0) != 0)
        {
            const reg32 bytes = guest.dword(got);
            if (bytes != 0)
            {
                guest.setDword(record + serial::kOutCount, guest.dword(record + serial::kOutCount) - bytes);
                guest.setDword(record + serial::kOutFirst, (guest.dword(record + serial::kOutFirst) + guest.dword(got)) & serial::kRingMask);
            }
            guest.setDword(record + serial::kWriting, 0);
        }
        else
        {
            guest.setDword(record + serial::kWriting, callImport<kGetLastError, &host::lastError>(guest, cpu) == kErrorIoPending ? 1 : 0);
        }
    }
    return leavePorts(guest, cpu);
}

void frontend::netcSerialFinishIo(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51ad70", 0, 0, nullptr, 0, kAll };
    const reg32 record = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialFinishIo(app, cpu, record); });
}

/* sub_51aeb0: the thread's wait: for its command event and every port's work under way. */
x86::reg32 frontend::netc::serialWaitEvents(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x90);
    const reg32 events = guest.frame();
    reg32 count = 1;
    guest.setDword(events, guest.dword(kPortsEvent));
    for (reg32 number = 0; number < 0x10; ++number)
    {
        const reg32 record = portRecord(guest, number);
        if (record == 0)
            continue;
        if (guest.dword(record + serial::kReading) != 0)
        {
            ++count;
            guest.setDword(events + count * 4 - 4, guest.dword(record + serial::kInEvent));
        }
        if (guest.dword(record + serial::kWriting) != 0)
        {
            ++count;
            guest.setDword(events + count * 4 - 4, guest.dword(record + serial::kOutEvent));
        }
    }
    return guest.call(kWaitForAny, args({ count, events }));
}

void frontend::netcSerialWaitEvents(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51aeb0", 0, 0, nullptr, 0, kAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialWaitEvents(app, cpu); });
}

/* sub_51af10: the ports' thread: waits, sees to what is done and to the commands
 * left for each port, starts what can be, while any port is open. */
x86::reg32 frontend::netc::serialThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    guest.setDword(kPortsEvent, reg32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu)));
    while (guest.dword(kOpenPorts) != 0)
    {
        guest::direct<0x51aeb0, serialWaitEvents>(app, cpu);
        for (reg32 number = 0; number < 0x10; ++number)
        {
            const reg32 record = portRecord(guest, number);
            if (record == 0)
                continue;
            guest::direct<0x51ad70, serialFinishIo>(app, cpu, record);
            if (guest.dword(record + serial::kCommand) != 0)
            {
                enterPorts(guest, cpu);
                const reg32 command = guest.dword(record + serial::kCommand);
                if (command == serial::kCommandDropInput)
                {
                    callImport<kPurgeComm, &host::serialPurge>(guest, cpu, guest.dword(record + serial::kHandle), 0xa);
                    guest::direct<0x51ad70, serialFinishIo>(app, cpu, record);
                    guest.setDword(record + serial::kInFirst, 0);
                    guest.setDword(record + serial::kInCount, 0);
                }
                else if (command == serial::kCommandDropOutput)
                {
                    callImport<kPurgeComm, &host::serialPurge>(guest, cpu, guest.dword(record + serial::kHandle), 5);
                    guest::direct<0x51ad70, serialFinishIo>(app, cpu, record);
                    guest.setDword(record + serial::kOutFirst, 0);
                    guest.setDword(record + serial::kOutCount, 0);
                }
                else if (command == serial::kCommandClose)
                {
                    callImport<kPurgeComm, &host::serialPurge>(guest, cpu, guest.dword(record + serial::kHandle), 0xf);
                    guest::direct<0x51ad70, serialFinishIo>(app, cpu, record);
                    callImport<kCloseHandle, &host::closeHandle>(guest, cpu, guest.dword(record + serial::kHandle));
                    directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.cpu(), guest.dword(record + serial::kInEvent));
                    directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.cpu(), guest.dword(record + serial::kOutEvent));
                    guest.setDword(kPorts + number * 4, 0);
                }
                guest.setDword(record + serial::kCommand, 0);
                leavePorts(guest, cpu);
            }
            if (portRecord(guest, number) != 0)
                guest::direct<0x51abc0, serialStartIo>(app, cpu, record);
        }
    }
    const reg32 closed = directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.cpu(), guest.dword(kPortsEvent));
    guest.setDword(kPortsEvent, 0);
    return closed;
}

void frontend::netcSerialThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51af10", 0, 0, nullptr, 0, kAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialThread(app, cpu); });
}

/* sub_51b030: whether a port's number is one that is open. */
x86::reg32 frontend::netc::serialIsOpen(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    const std::int32_t index = sign(number);
    return index >= 0 && index < 0x10 && portRecord(guest, reg32(index)) != 0 ? 1 : 0;
}

void frontend::netcSerialIsOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b030", 0, 0, nullptr, 0, kAll };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialIsOpen(app, cpu, number); });
}

/* sub_51b1c0: what a port has received thrown away, by its thread. */
x86::reg32 frontend::netc::serialDropInput(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) == 0)
        return 0;
    return guest::direct<0x51ab70, serialCommand>(app, cpu, number, serial::kCommandDropInput);
}

void frontend::netcSerialDropInput(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b1c0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialDropInput(app, cpu, number); });
}

/* sub_51b1e0: what a port has yet to send thrown away, by its thread. */
x86::reg32 frontend::netc::serialDropOutput(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) == 0)
        return 0;
    return guest::direct<0x51ab70, serialCommand>(app, cpu, number, serial::kCommandDropOutput);
}

void frontend::netcSerialDropOutput(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b1e0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialDropOutput(app, cpu, number); });
}

/* sub_51b200: a port closed: its thread told and waited for, its record freed;
 * with the last of them the thread's lock goes too. */
x86::reg32 frontend::netc::serialClose(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0)
    {
        const reg32 record = portRecord(guest, number);
        guest::direct<0x51ab70, serialCommand>(app, cpu, number, serial::kCommandClose);
        leavePorts(guest, cpu);
        while (portRecord(guest, number) != 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
        loaded<0x4e1890, ea::purgeBlock>(app, cpu, record);
        guest.setDword(kOpenPorts, guest.dword(kOpenPorts) - 1);
    }
    else
    {
        leavePorts(guest, cpu);
    }
    if (guest.dword(kOpenPorts) != 0)
        return guest.eax();
    guest::direct<0x51ab70, serialCommand>(app, cpu, number, 0);
    const reg32 freed = reg32(*fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(kPortsLock)));
    guest.setDword(kPortsLock, 0);
    return freed;
}

void frontend::netcSerialClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b200", 0, 0, nullptr, 0, kAll };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialClose(app, cpu, number); });
}

/* sub_51b2f0: a port made of a handle already open, in the number asked or, for -1,
 * the last free; with the first of them the thread starts.  Its number, or -1. */
x86::reg32 frontend::netc::serialOpenHandle(App* app, CPU& cpu, reg32 number, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = handle;
    const reg32 timeouts = guest.frame();
    callImport<kClearError, &host::setLastError>(guest, cpu, 0);
    if (number == 0xffffffff)
    {
        number = 0xf;
        const reg32 top = guest.dword(kPorts + 0x3c);
        guest.leaveEdx(top);
        if (top != 0)
        {
            reg32 at = 0x3c, seen;
            do
            {
                if (sign(at) < 0)
                    break;
                seen = guest.dword(kPorts - 4 + at);
                at -= 4;
                --number;
            } while (seen != 0);
        }
        if (sign(number) < 0)
            return 0xffffffff;
    }
    guest.setDword(kOpenPorts, guest.dword(kOpenPorts) + 1);
    if (guest.dword(kOpenPorts) == 1)
    {
        loaded<0x4f2a78, frontend::eab::atexitJump>(app, cpu, kCloseAllPorts);
        ensurePortsLock(guest, cpu);
        if (guest.call(kCreateThread, args({ kSerialThreadEntry, 0, 2, 0xffffffff }, { kThreadHandle })) == 0)
        {
            reg32(*fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(kPortsLock)));
            guest.setDword(kPortsLock, 0);
            guest.setDword(kOpenPorts, guest.dword(kOpenPorts) - 1);
            return 0xffffffff;
        }
    }
    guest.setDword(kErrorFile, kSourceFile);
    guest.setDword(kErrorFunction, 0x550ba8);
    guest.setDword(kErrorLine, 0x248);
    const reg32 record = guest.call(kAllocate, args({ 0x550bbc, serial::kRecordSize, guest.dword(kMemoryType), 0x550ba8 }));
    guest.setDword(record + serial::kReading, 0);
    guest.setDword(record + serial::kHandle, handle);
    guest.setDword(record + serial::kInEvent, callImport<kCreateEventApi, &host::createEvent>(guest, cpu, 0, 1, 0, 0));
    guest.setDword(record + serial::kInFirst, 0);
    guest.setDword(record + serial::kInCount, 0);
    guest.setDword(record + serial::kWriting, 0);
    const reg32 outEvent = callImport<kCreateEventApi, &host::createEvent>(guest, cpu, 0, 1, 0, 0);
    guest.setDword(record + serial::kOutFirst, 0);
    guest.setDword(record + serial::kOutEvent, outEvent);
    guest.setDword(record + serial::kOutCount, 0);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), timeouts, 0x14);
    guest.setDword(timeouts + 8, 0x19);
    callImport<kSetTimeouts, &host::serialSetTimeouts>(guest, cpu, handle, timeouts);
    guest.setDword(kPorts + number * 4, record);
    // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    guest.leaveEdx(1);
    guest::direct<0x51ab70, serialCommand>(app, cpu, number, 1);
    return number;
}

void frontend::netcSerialOpenHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b2f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, handle = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialOpenHandle(app, cpu, number, handle); });
}

/* sub_51b480: a port opened by its number.  Its number, -1 if it would not; open already, nothing. */
x86::reg32 frontend::netc::serialOpen(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x20);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    const reg32 name = guest.frame();
    if (sign(number) < 0 || sign(number) >= 0x10)
        reportError(guest, kSourceFile, 0x550bc8, 0x269, 0x550bd4, number);
    callImport<kClearError, &host::setLastError>(guest, cpu, 0);
    if (portRecord(guest, number) != 0)
        return guest.eax();
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, 0x550bf4, guest::Words{number + 1});
    const reg32 handle = callImport<kCreateFile, &host::createFile>(guest, cpu, name, 0xc0000000, 0, 0, 3, 0x40000080, 0);
    if (handle == 0xffffffff)
        return handle;
    return guest::direct<0x51b2f0, serialOpenHandle>(app, cpu, number, handle);
}

void frontend::netcSerialOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b480", 0, 0, nullptr, 0, kAll };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialOpen(app, cpu, number); });
}

/* sub_51b530: data written to a port, if all of it fits.  What went. */
x86::reg32 frontend::netc::serialWriteWhole(App* app, CPU& cpu, reg32 number, reg32 length, reg32 data)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = length; cpu.ebx = data;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    reg32 went = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0 && sign(guest::direct<0x51b900, serialOutRoom>(app, cpu, number)) >= sign(length))
        went = loaded<0x51b5f0, serialWrite>(app, cpu, number, length, data);
    guest.leaveEbx(went);
    leavePorts(guest, cpu);
    return went;
}

void frontend::netcSerialWriteWhole(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b530", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, length = cpu.edx, data = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialWriteWhole(app, cpu, number, length, data); });
}

/* sub_51b5f0: as much data as there is room for put in a port's ring, its thread
 * told.  What went. */
x86::reg32 frontend::netc::serialWrite(App* app, CPU& cpu, reg32 number, reg32 length, reg32 data)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = length; cpu.ebx = data;
    reg32 first = 0, second = 0;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0)
    {
        const reg32 count = guest.dword(portRecord(guest, number) + serial::kOutCount);
        if (sign(count + length) > sign(serial::kRingSize))
            length = serial::kRingSize - count;
        const reg32 record = portRecord(guest, number);
        const reg32 at = (guest.dword(record + serial::kOutFirst) + guest.dword(record + serial::kOutCount)) & serial::kRingMask;
        first = length;
        if (sign(at + length) > sign(serial::kRingSize))
        {
            first = serial::kRingSize - at;
            second = length - first;
        }
        if (first != 0)
            guest.call(kMoveMemory, args({ data, portRecord(guest, number) + serial::kOutRing + at, first }));
        guest.leaveEbx(second);
        if (second != 0)
            guest.call(kMoveMemory, args({ data + first, portRecord(guest, number) + serial::kOutRing, second }));
        const reg32 now = portRecord(guest, number);
        guest.setDword(now + serial::kOutCount, guest.dword(now + serial::kOutCount) + second + first);
        // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    guest.leaveEdx(1);
    guest::direct<0x51ab70, serialCommand>(app, cpu, number, 1);
    }
    leavePorts(guest, cpu);
    return second + first;
}

void frontend::netcSerialWrite(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b5f0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, length = cpu.edx, data = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialWrite(app, cpu, number, length, data); });
}

/* sub_51b710: a port's speed, parity, stop bits and word's length.  The speed is
 * the table's for a small number. */
x86::reg32 frontend::netc::serialSet(App* app, CPU& cpu, reg32 number, reg32 speed, reg32 parity, reg32 stopBits,
                                     reg32 wordBits)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = speed; cpu.ebx = parity; cpu.ecx = stopBits;
    const reg32 state = guest.frame();
    guest.leaveEbx(stopBits);
    callImport<kClearError, &host::setLastError>(guest, cpu, 0);
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) == 0)
    {
        reportError(guest, kSourceFile, 0x550c00, 0x356, 0x550c0c);
        guest.leaveEbx(0x550c00);
    }
    else if (callImport<kGetCommState, &host::serialGetState>(guest, cpu, guest.dword(portRecord(guest, number) + serial::kHandle), state) != 0)
    {
        guest.setByte(state + 8, guest.byte(state + 8) | 1);
        guest.setWord(state + 8, guest.word(state + 8) & 0xb0fd);
        if (sign(speed) <= 0x10)
            speed = guest.dword(kBaudRates + speed * 4);
        guest.setDword(state + 4, speed);
        guest.setByte(state + 0x12, std::uint8_t(std::uint8_t(wordBits) + 5));
        guest.setByte(state + 0x14, stopBits == 1 ? 2 : 0);
        guest.setByte(state + 0x13, parity == 1 ? 1 : parity == 3 ? 2 : 0);
        callImport<kSetCommState, &host::serialSetState>(guest, cpu, guest.dword(portRecord(guest, number) + serial::kHandle), state);
    }
    return leavePorts(guest, cpu);
}

void frontend::netcSerialSet(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b710", 4, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 number = cpu.eax, speed = cpu.edx, parity = cpu.ebx, stopBits = cpu.ecx;
    const reg32 wordBits = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialSet(app, cpu, number, speed, parity, stopBits, wordBits); });
}

/* sub_51b8a0: how much a port has received. */
x86::reg32 frontend::netc::serialInCount(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    reg32 count = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0)
        count = guest.dword(portRecord(guest, number) + serial::kInCount);
    leavePorts(guest, cpu);
    return count;
}

void frontend::netcSerialInCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b8a0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEdx) };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialInCount(app, cpu, number); });
}

/* sub_51b900: how much room a port has to send. */
x86::reg32 frontend::netc::serialOutRoom(App* app, CPU& cpu, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    reg32 room = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0)
        room = serial::kRingSize - guest.dword(portRecord(guest, number) + serial::kOutCount);
    leavePorts(guest, cpu);
    return room;
}

void frontend::netcSerialOutRoom(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b900", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEdx) };
    const reg32 number = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialOutRoom(app, cpu, number); });
}

/* sub_51b970: what a port has received copied out, left in the ring.  How much. */
x86::reg32 frontend::netc::serialPeek(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = length; cpu.ebx = out;
    reg32 first = 0, second = 0;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) != 0)
    {
        const reg32 available = guest.dword(portRecord(guest, number) + serial::kInCount);
        guest.leaveEbx(available);
        if (sign(length) > sign(available))
            length = available;
        const reg32 record = portRecord(guest, number);
        const reg32 at = guest.dword(record + serial::kInFirst);
        first = length;
        if (sign(at + length) > sign(serial::kRingSize))
        {
            second = length - (serial::kRingSize - at);
            first = length - second;
        }
        if (first != 0)
        {
            const reg32 now = portRecord(guest, number);
            guest.call(kMoveMemory, args({ guest.dword(now + serial::kInFirst) + now + serial::kInRing, out, first }));
        }
        if (second != 0)
            guest.call(kMoveMemory, args({ portRecord(guest, number) + serial::kInRing, out + first, second }));
    }
    leavePorts(guest, cpu);
    return second + first;
}

void frontend::netcSerialPeek(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51b970", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, length = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialPeek(app, cpu, number, length, out); });
}

/* sub_51ba50: what a port has received taken out of the ring, its thread told.  How much. */
x86::reg32 frontend::netc::serialRead(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = length; cpu.ebx = out;
    ensurePortsLock(guest, cpu);
    enterPorts(guest, cpu);
    const reg32 got = loaded<0x51b970, serialPeek>(app, cpu, number, length, out);
    guest.leaveEbx(got);
    if (got != 0)
    {
        const reg32 record = portRecord(guest, number);
        guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) - got);
        const reg32 now = portRecord(guest, number);
        guest.setDword(now + serial::kInFirst, (got + guest.dword(now + serial::kInFirst)) & serial::kRingMask);
        // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    guest.leaveEdx(1);
    guest::direct<0x51ab70, serialCommand>(app, cpu, number, 1);
    }
    leavePorts(guest, cpu);
    return got;
}

void frontend::netcSerialRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51ba50", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const reg32 number = cpu.eax, length = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialRead(app, cpu, number, length, out); });
}

/* sub_51bbd0: what is wanted from a port read once it is there or the time is up.  How much. */
x86::reg32 frontend::netc::serialWaitIn(App* app, CPU& cpu, reg32 number, reg32 length, reg32 out, reg32 timeout)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    cpu.eax = number; cpu.edx = length; cpu.ebx = out; cpu.ecx = timeout;
    guest.leaveEbx(0);
    if (sign(length) > sign(serial::kRingSize))
        reportError(guest, kSourceFile, 0x550c58, 0x471, 0x550c68, serial::kRingSize);
    if (guest::direct<0x51b030, serialIsOpen>(app, cpu, number) == 0)
        return 0;
    const reg32 deadline = guest.dword(kClockTicks) + timeout;
    guest.leaveEcx(number * 4);
    while (sign(length) > sign(guest.dword(portRecord(guest, number) + serial::kInCount))
           && sign(deadline) > sign(guest.dword(kClockTicks)))
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    const reg32 got = guest.call(kPortRead, args({ number, length, out }));
    guest.leaveEbx(got);
    return got;
}

void frontend::netcSerialWaitIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_51bbd0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const reg32 number = cpu.eax, length = cpu.edx, out = cpu.ebx, timeout = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return netc::serialWaitIn(app, cpu, number, length, out, timeout); });
}

}  // namespace nfs3hp
