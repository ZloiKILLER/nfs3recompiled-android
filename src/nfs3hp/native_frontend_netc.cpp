#include "native_frontend_netc.h"
#include "native_frontend_neta.h"
#include "native_ea_files.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_netb.h"
#include "native_frontend_wata.h"
#include "native_frontend_wina.h"
#include "os_threads.h"
#include "os_net.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include "os_files.h"
#include "os_system.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 netcGatherPulse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherPulseStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherJoin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherNewGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherSetGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherCloseGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherReopenGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherEnumPlayers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherKillPlayer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherOpenConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcGatherReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcLoadRasApi(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcFrameChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcFrameChecksumGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcFrameBuild(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcFrameSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcFrameSendControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableReceiveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableNotify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableProcess(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcUnreliableClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSequenceWiden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketChecksumGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketAckNumber(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketBuild(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketSendControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketSendNak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketGotAck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketGotNak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketReceiveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketQueueKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketCountDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketResendDue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketNotify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketSubmit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketProcess(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcPacketClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcModemResponse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcModemCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcModemAttention(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialCloseAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialStartIo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialFinishIo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialWaitEvents(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialIsOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialDropInput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialDropOutput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialOpenHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialWriteWhole(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialInCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialOutRoom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialPeek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netcSerialWaitIn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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


/* The last of EA's network library, native whole: the gathering's server side
 * (cmn\netgathr.c), the serial port and its thread (win\comm.c) with the
 * modem's talk over it, and the two packet layers over a stream --
 * cmn\unrely.c and cmn\packet.c, which numbers and resends besides.
 *
 * Everything stays in guest memory, where the rest of the library reads it,
 * and the calling convention is the game's: a transport's functions and a
 * connection's are reached through the tables and pointers they are kept in.
 * Windows' functions are the port's own, called as C++.  A register the original leaves changed is left the
 * same.
 *
 * Plan step 5: each function a typed core in frontend::netc (native_frontend_netc.h) -- its
 * arguments put in the registers the game's function takes them in, for the code that reads them
 * there, then the code as it was, in a frame of its own (fe::Frame: what it calls runs at the
 * stack addresses it did) -- and a thunk under the game's name for the calls by address.  A
 * core of this file calls another as C++ (guest::direct) where nothing after it, nor the
 * function's end, meets a register the call by address would leave otherwise; the typed cores of
 * other files likewise (fe::direct); the rest of the game, the tables' and pointers' calls and
 * those that hand registers back, by address. */
namespace nfs3hp
{

/* The port's trace of what is sent (native_vertices.cpp), before sub_51d810. */
bool commSendTrace(nfs::App* app, nfs::ThreadState& ts, const nfs::Args& registers);

namespace
{

using fe::args;
using fe::Guest;
using fe::Site;
using nfs::u32;

constexpr std::uint8_t kEbx = fe::kRestoreEbx, kEcx = fe::kRestoreEcx, kEdx = fe::kRestoreEdx;
constexpr std::uint8_t kEsi = fe::kRestoreEsi, kEdi = fe::kRestoreEdi, kAll = fe::kRestoreAll;

std::int32_t sign(u32 value)
{
    return std::int32_t(value);
}

u32 byteSwapped(u32 value)
{
    return (value >> 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}

// The game's functions, by address.
constexpr u32 kReportError = 0x401010, kSprintf = 0x4df690, kCreateThread = 0x4df7a0;
constexpr u32 kPollEvent = 0x4dfb40;
constexpr u32 kWaitForAny = 0x4dfbf0, kCloseEvent = 0x4dfc00, kStrstr = 0x4e0820;
constexpr u32 kMonoPrintf = 0x4e08f0, kStrncpy = 0x4e0e30, kAllocate = 0x4e1620;
constexpr u32 kWaitSystemTasks = 0x4e7720, kMoveMemory = 0x4ea4f0;
constexpr u32 kAtoi = 0x4f0430;
constexpr u32 kEndGame = 0x4f4f90, kCheckPlayer = 0x4f5040;
constexpr u32 kOpenConnection = 0x4f5130, kFileReport = 0x501050, kQueueFree = 0x50dde0;
constexpr u32 kQueueAdd = 0x50de50, kQueueAddSorted = 0x50e0a0, kQueueRemove = 0x50e150;
constexpr u32 kQueueFindIndex = 0x50e470, kQueueFindKey = 0x50e500;
constexpr u32 kTrace = 0x512180, kDeliver = 0x5126f0;
constexpr u32 kRequest = 0x513190, kAnswerRequest = 0x5133a0;

// The game's data.
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr u32 kTicksPerSecond = 0x5643d8, kMemoryType = 0x5643f4, kModemLog = 0x564454, kModemAbort = 0x564458;
constexpr u32 kPulseTicks = 0x566dd4, kPulsePeriod = 0x566dd8, kGatherTraceLevel = 0x566de0;
constexpr u32 kTraceLevel = 0x566dec, kConnectionTimeout = 0x566df0, kConnectionNames = 0x566e04;
constexpr u32 kLeaveSerial = 0x5689dc, kRasLibrary = 0x568a34, kRasEnum = 0x568a2c, kRasProjection = 0x568a30;
constexpr u32 kPulseQueue = 0x9f25f0, kClosedConnections = 0x9f260c, kActiveConnections = 0x9f2628;
constexpr u32 kDropFunction = 0x4f50b0, kNeverTimesOut = 1000000000;

// The gathering's messages, as the transport sends them.
constexpr u32 kMessageAlive = 0xea6f6c61, kMessageAdd = 0xea616464, kMessageRequest = 0xea726571;
constexpr u32 kMessageBye = 0xea627965, kMessageGet = 0xea676574, kMessageKick = 0xea6f7574;
constexpr u32 kMessageData = 0xea646174, kMessageRefused = 0xea6e6f21;

/* A network adapter's table of functions (a gathering's connection +8). */
namespace transport
{
constexpr u32 kStartListen = 0x28, kStopListen = 0x2c, kGetAddress = 0x30, kCancel = 0x34, kSend = 0x54, kAck = 0x5c;
}

/* A gathering's connection. */
namespace conn
{
constexpr u32 kInfo = 0x04, kTransport = 0x08, kHandle = 0x0c, kGame = 0x14, kRequest = 0x18, kPlayer = 0x1c;
constexpr u32 kOwnPlayer = 0x30, kRequests = 0x8c, kPulse = 0xa0, kInGame = 0xa4, kQuiet = 0xa8;
constexpr u32 kPlayerDwords = 5, kGameDwords = 13;
}

/* The game a server keeps, its players' slots after it. */
namespace game
{
constexpr u32 kOwner = 0x04, kClosed = 0x08, kAddress = 0x0c, kName = 0x3c, kUser = 0x4c, kMembers = 0x50;
constexpr u32 kInfo = 0x54, kMaxPlayers = 0x84, kSlots = 0x88, kSlotSize = 0x64, kSlotTimer = 0x5c;
constexpr u32 kBaseSize = 0xec;
}

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A typed core called as C++ (fe::direct), the registers left as the call by address leaves them:
 * its first words in edx, ebx and ecx after eax's, where that call loads them and the callee keeps
 * them, and its result in eax.  What the code after it reads there, and the function's end, finds
 * what it found before.  For a core whose first four words go in eax, edx, ebx and ecx, called
 * where nothing after it meets a register its function leaves changed (specs_from_exe.py's
 * clobbers; FRONTEND_STRICT=1 shows none): those calls stay by address. */
template <u32 Address, auto Core, typename... Words>
u32 loaded(nfs::App* app, nfs::ThreadState& ts, Held& h, Words... words)
{
    const u32 all[] = { u32(words)..., 0 };
    Slot* const into[] = { &h.edx, &h.ebx, &h.ecx };
    for (std::size_t i = 1; i < sizeof...(Words) && i < 4; ++i)
        *into[i - 1] = all[i];
    const auto result = fe::direct<Address, Core>(app, ts, u32(words)...);
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, u32>)
        h.eax = result;
    else
        h.eax = *result;
    return h.eax;
}

/* The library's trace of a connection's events, sub_512180. */
u32 trace(Guest& guest, Held& h, u32 text, u32 data, u32 ebx, u32 ecx, u32 raw, u32 id)
{
    return h.eax = callHeld(guest, h, kTrace, args({ text, data, ebx, ecx }, { raw, id }));
}

/* A connection's event with no data. */
u32 traceEvent(Guest& guest, Held& h, u32 text, u32 raw, u32 id)
{
    return trace(guest, h, text, 0, 0, 0, raw, id);
}

/* A fatal message: where it is from, then the message. */
void reportError(Guest& guest, Held& h, u32 file, u32 function, u32 line, u32 message)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    fe::direct<kReportError, ea::error>(guest.app(), guest.ts(), message, guest::Words({}));
}

void reportError(Guest& guest, Held& h, u32 file, u32 function, u32 line, u32 message, u32 argument)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    fe::direct<kReportError, ea::error>(guest.app(), guest.ts(), message, guest::Words{argument});
}

}  // namespace

/* ---------------------------------------------------------------------------
 * cmn\netgathr.c: a server's game, and asking for one
 * ------------------------------------------------------------------------- */

/* sub_5140e0: the server's pulse, per connection: counts the game's advert
 * down, tells each player we are alive every so often and drops one that has
 * not answered in time.  Always 1. */
nfs::u32 frontend::netc::gatherPulse(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    const u32 frame = guest.frame();
    const u32 slotPlace = frame, messagePlace = frame + 4;
    const u32 info = guest.dword(self + conn::kInfo);
    h.edx = (info);
    if (guest.dword(info) != 0)
        return 1;
    if (guest.dword(self + conn::kQuiet) == 0 && guest.dword(self + conn::kHandle) != 0)
    {
        const u32 left = guest.dword(self + conn::kInGame);
        if (left != 0 && guest.dword(self + conn::kGame) == 0)
            guest.setDword(self + conn::kInGame, left - 1);
    }
    const u32 server = guest.dword(self + conn::kGame);
    if (server == 0 || guest.dword(server + game::kClosed) != 0 || guest.dword(server + game::kMembers) == 0)
        return 1;
    guest.setDword(messagePlace, kMessageAlive);
    const u32 pulse = guest.dword(self + conn::kPulse);
    bool tell = false;
    if (sign(pulse) >= 2)
    {
        guest.setDword(self + conn::kPulse, pulse - 1);
    }
    else
    {
        guest.setDword(self + conn::kPulse, guest.dword(kPulsePeriod));
        if (sign(guest.dword(kGatherTraceLevel)) >= 3)
            trace(guest, h, 0x5503bc, messagePlace, 0, 0, 0, 0);
        tell = true;
    }
    const u32 slots = server + game::kSlots;
    guest.setDword(slotPlace, 0);
    for (;;)
    {
        const u32 slot = guest.dword(slotPlace);
        if (guest.dword(server + game::kMaxPlayers) - 1 <= slot)
            return 1;
        const u32 bit = u32(1) << (guest.byte(slotPlace) & 31);
        if ((guest.dword(server + game::kMembers) & bit) != 0)
        {
            const u32 timer = slots + slot * game::kSlotSize + game::kSlotTimer;
            const u32 count = guest.dword(timer);
            guest.setDword(timer, count - 1);
            h.edx = (count);
            if (count == 0)
            {
                if (sign(guest.dword(kGatherTraceLevel)) >= 2)
                    trace(guest, h, 0x5503cc, slotPlace, 0, 0, 1, 0);
                const u32 gone = u32(1) << (guest.byte(slotPlace) & 31);
                guest.setDword(server + game::kMembers, guest.dword(server + game::kMembers) & ~gone);
            }
            else if (tell)
            {
                const u32 table = guest.dword(self + conn::kTransport);
                callHeld(guest, h, guest.dword(table + transport::kSend),
                           args({ guest.dword(self + conn::kHandle), slot * game::kSlotSize + slots,
                                  guest.dword(messagePlace), 0 },
                                { 0 }));
            }
        }
        guest.setDword(slotPlace, guest.dword(slotPlace) + 1);
    }
}

nfs::u32 frontend::netcGatherPulse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5140e0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherPulse(app, ts, self); });
}

/* sub_514260: every gathering's connection pulsed, by the queue of them. */
nfs::u32 frontend::netc::gatherPulseStart(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    return loaded<0x50e8e0, frontend::netb::qiterate>(app, ts, h, kPulseQueue, 0x5140e0, 1);
}

nfs::u32 frontend::netcGatherPulseStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514260", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEdx) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherPulseStart(app, ts); });
}

/* sub_5143d0: a transport's address lookup waited for, the request's record
 * (0x50 bytes copied from the code's table) on the stack while it runs. */
nfs::u32 frontend::netc::gatherRequest(App* app, ThreadState& ts, u32 self, u32 start, u32 wanted, u32 last)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x94);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = start; h.ebx = wanted; h.ecx = last;
    const u32 frame = guest.frame();
    const u32 answer = frame + 0x50;
    copyDwords(guest, frame, 0x512dd0, 0x14);
    // The game's code hands it `wanted` and 0 in ebx and ecx too, which it does not take; there they stay.
    h.ebx = (wanted);
    h.ecx = (0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, ts, h, self + conn::kRequests, answer);
    guest.setDword(frame + 0x10, self);
    guest.setDword(frame + 0x14, start);
    guest.setDword(frame, 0);
    guest.setDword(frame + 0xc, last);
    guest.setDword(self + conn::kRequest, frame);
    // And 0 in edx, which it does not take; there it stays.
    h.edx = (0);
    loaded<0x512fc0, frontend::netb::setResponseTimeout>(app, ts, h, wanted);
    h.ebx = (0);
    for (bool again = true; again;)
    {
        const u32 table = guest.dword(self + conn::kTransport);
        const u32 got = callHeld(guest, h, guest.dword(table + transport::kGetAddress),
                                     args({ guest.dword(self + conn::kHandle), answer, h.ebx, table }));
        h.edx = (got);
        for (;;)
        {
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, h.ebx)) != 0)
            {
                if (sign(callHeld(guest, h, kWaitSystemTasks, args({ 1 }))) < 0)
                {
                    again = false;
                    break;
                }
            }
            else
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
            }
            if (loaded<0x512fd0, frontend::netb::responseTimedOut>(app, ts, h) != 0 || h.ebx == guest.dword(frame + 0x14))
            {
                again = false;
                break;
            }
            if (h.edx == 0)
                break;
        }
    }
    if (h.edx != 0)
    {
        const u32 table = guest.dword(self + conn::kTransport);
        h.ecx = (table);
        callHeld(guest, h, guest.dword(table + transport::kCancel), args({ guest.dword(self + conn::kHandle) }));
    }
    guest.setDword(self + conn::kRequest, 0);
    return h.edx;
}

nfs::u32 frontend::netcGatherRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5143d0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 self = args.eax, start = args.edx, wanted = args.ebx, last = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherRequest(app, ts, self, start, wanted, last); });
}

/* sub_5144b0: joining a player's game: leaves the one we are in unless it is
 * that one, then asks to be added.  What the server answered. */
nfs::u32 frontend::netc::gatherJoin(App* app, ThreadState& ts, u32 self, u32 player, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x114);
    // The arguments where the game's code hands them: the code below reads them there.
    const u32 frame = guest.frame();
    const u32 reply = frame + 0xac;
    copyDwords(guest, reply, 0x512e20, 0x14);
    if (guest.dword(self + conn::kInGame) != 0)
    {
        if (got(fe::direct<0x4f5040, ea::netPlayerChecked>(app, ts, self, self + conn::kPlayer, player)) != 0)
            return 1;
        got(fe::direct<0x4f4f90, ea::netGameEnded>(app, ts, self));
        guest.setDword(self + conn::kInGame, 0);
    }
    copyDwords(guest, frame + 0x28, self + conn::kOwnPlayer, 0x17);
    copyDwords(guest, frame + 4, player, conn::kPlayerDwords);
    copyDwords(guest, self + conn::kPlayer, player, conn::kPlayerDwords);
    if (name != 0)
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), frame + 0x18, name, 0x10);
    else
        guest.setByte(guest.dword(self + conn::kGame) + game::kName, 0);
    fe::direct<0x513190, frontend::netb::sendServerRequest>(app, ts, self, reply, kMessageAdd, frame, 0x88, guest.dword(kTicksPerSecond) * 2);
    return guest.dword(frame + 0xb0);
}

nfs::u32 frontend::netcGatherJoin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5144b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 self = args.eax, player = args.edx, name = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherJoin(app, ts, self, player, name); });
}

/* sub_5145b0: a game's information asked of its server, into `out`: 1 if the
 * server answered. */
nfs::u32 frontend::netc::gatherGameInfo(App* app, ThreadState& ts, u32 self, u32 player, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10c);
    // The arguments where the game's code hands them: the code below reads them there.
    const u32 frame = guest.frame();
    const u32 reply = frame + 0xac;
    copyDwords(guest, reply, 0x512e70, 0x14);
    const bool joined = guest.dword(self + conn::kInGame) != 0;
    if (player == 0 ? !joined : joined)
        return 0;
    if (player != 0)
        copyDwords(guest, self + conn::kPlayer, player, conn::kPlayerDwords);
    fe::direct<0x513190, frontend::netb::sendServerRequest>(app, ts, self, reply, kMessageRequest, frame, 0xac, guest.dword(kTicksPerSecond) * 2);
    copyDwords(guest, out, frame + 0xc8, conn::kGameDwords);
    return guest.dword(frame + 0xb0) != 0 ? 1 : 0;
}

nfs::u32 frontend::netcGatherGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5145b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 self = args.eax, player = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherGameInfo(app, ts, self, player, out); });
}

/* sub_514650: a new game served: its record made, the transport listening.  1 if so. */
nfs::u32 frontend::netc::gatherNewGame(App* app, ThreadState& ts, u32 self, u32 info, u32 name, u32 user)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = info; h.ebx = name; h.ecx = user;
    constexpr u32 kFile = 0x5503dc, kFunction = 0x5503ec;
    const u32 maxPlayers = info + 0x30;
    if (guest.dword(maxPlayers) == 0)
        guest.setDword(maxPlayers, 0x20);
    if (guest.dword(self + conn::kInGame) != 0)
    {
        callHeld(guest, h, kEndGame, args({ self }));
        guest.setDword(self + conn::kInGame, 0);
    }
    if (guest.dword(maxPlayers) < 2)
        reportError(guest, h, kFile, kFunction, 0x2c5, 0x5503f8);
    if (guest.dword(maxPlayers) > 0x20)
        reportError(guest, h, kFile, kFunction, 0x2c7, 0x55042c, 0x20);
    const u32 size = (guest.dword(maxPlayers) - 2) * game::kSlotSize + game::kBaseSize;
    guest.setDword(kErrorFile, kFile);
    guest.setDword(kErrorFunction, kFunction);
    guest.setDword(kErrorLine, 0x2cb);
    const u32 server = callHeld(guest, h, kAllocate, args({ 0x550458, size, guest.dword(kMemoryType), kFile }));
    guest.setDword(self + conn::kGame, server);
    if (server == 0)
        return 0;
    guest.setDword(server + game::kOwner, self);
    copyDwords(guest, guest.dword(self + conn::kGame) + game::kInfo, info, conn::kGameDwords);
    h.ecx = (0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kClosed, 0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kMembers, 0);
    guest.setDword(guest.dword(self + conn::kGame) + game::kUser, user);
    guest.setDword(self + conn::kInGame, guest.dword(kPulseTicks));
    guest.setDword(self + conn::kPulse, guest.dword(kPulsePeriod));
    copyDwords(guest, self + conn::kPlayer, self + conn::kOwnPlayer, conn::kPlayerDwords);
    if (name != 0)
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), guest.dword(self + conn::kGame) + game::kName, name, 0x10);
    else
        guest.setByte(guest.dword(self + conn::kGame) + game::kName, 0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, ts, h, self + conn::kRequests, guest.dword(self + conn::kGame) + game::kAddress);
    const u32 table = guest.dword(self + conn::kTransport);
    h.ebx = (table);
    if (callHeld(guest, h, guest.dword(table + transport::kStartListen),
                   args({ guest.dword(self + conn::kHandle), guest.dword(self + conn::kGame) + game::kAddress })) != 0)
        return 1;
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(self + conn::kGame));
    guest.setDword(self + conn::kGame, 0);
    return 0;
}

nfs::u32 frontend::netcGatherNewGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514650", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 self = args.eax, info = args.edx, name = args.ebx, user = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherNewGame(app, ts, self, info, name, user); });
}

/* sub_514820: the served game's information changed.  1, or 0 with no game. */
nfs::u32 frontend::netc::gatherSetGameInfo(App* app, ThreadState& ts, u32 self, u32 info)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = info;
    const u32 server = guest.dword(h.eax + conn::kGame);
    if (server == 0)
        return 0;
    copyDwords(guest, server + game::kInfo, h.edx, conn::kGameDwords);
    return 1;
}

nfs::u32 frontend::netcGatherSetGameInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514820", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 self = args.eax, info = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherSetGameInfo(app, ts, self, info); });
}

/* sub_514850: the served game no longer listened for; with `discard` its record goes too. */
nfs::u32 frontend::netc::gatherCloseGame(App* app, ThreadState& ts, u32 self, u32 discard)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = discard;
    const u32 server = guest.dword(self + conn::kGame);
    h.ecx = (self);
    h.ebx = (discard);
    h.edx = (server);
    if (server == 0)
        return 0;
    if (guest.dword(server + game::kClosed) == 0)
    {
        const u32 table = guest.dword(self + conn::kTransport);
        callHeld(guest, h, guest.dword(table + transport::kStopListen),
                   args({ guest.dword(self + conn::kHandle), server + game::kAddress }));
        guest.setByte(guest.dword(h.ecx + conn::kGame) + game::kAddress, 0);
        guest.setDword(guest.dword(h.ecx + conn::kGame) + game::kClosed, 1);
    }
    if (h.ebx != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(h.ecx + conn::kGame));
        guest.setDword(h.ecx + conn::kGame, 0);
    }
    return 1;
}

nfs::u32 frontend::netcGatherCloseGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514850", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi) };
    const u32 self = args.eax, discard = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherCloseGame(app, ts, self, discard); });
}

/* sub_5148b0: a closed game listened for again, every slot's time given back. */
nfs::u32 frontend::netc::gatherReopenGame(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    const u32 server = guest.dword(self + conn::kGame);
    if (server == 0 || guest.dword(server + game::kClosed) == 0)
        return 0;
    guest.setDword(server + game::kClosed, 0);
    loaded<0x512f60, frontend::netb::gameGuidText>(app, ts, h, self + conn::kRequests, guest.dword(self + conn::kGame) + game::kAddress);
    const u32 table = guest.dword(self + conn::kTransport);
    const u32 listening = callHeld(guest, h, guest.dword(table + transport::kStartListen),
                                       args({ guest.dword(self + conn::kHandle),
                                              guest.dword(self + conn::kGame) + game::kAddress, h.ebx, table }));
    if (listening == 0)
    {
        guest.setDword(guest.dword(self + conn::kGame) + game::kClosed, 0);
        return 0;
    }
    const u32 ticks = guest.dword(kPulseTicks);
    guest.setDword(self + conn::kPulse, 1);
    const u32 now = guest.dword(self + conn::kGame);
    for (u32 slot = 0; slot < guest.dword(now + game::kMaxPlayers) - 1; ++slot)
        guest.setDword(now + game::kSlots + slot * game::kSlotSize + game::kSlotTimer, ticks);
    return 1;
}

nfs::u32 frontend::netcGatherReopenGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5148b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx | kEsi | kEdi) };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherReopenGame(app, ts, self); });
}

/* sub_514970: leaving: the served game told goodbye and thrown away, or our
 * server told we go.  Always 1. */
nfs::u32 frontend::netc::gatherLeave(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xb8);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    const u32 frame = guest.frame();
    if (guest.dword(self + conn::kInGame) == 0)
        return 1;
    const u32 byeNumber = guest.dword(kLeaveSerial);
    const u32 bye = frame, messagePlace = frame + 0xac;
    guest.setDword(bye, byeNumber);
    guest.setDword(messagePlace, kMessageBye);
    copyDwords(guest, frame + 4, self + conn::kPlayer, conn::kPlayerDwords);
    guest.setDword(kLeaveSerial, byeNumber + 1);
    const u32 server = guest.dword(self + conn::kGame);
    if (server == 0)
    {
        if (sign(guest.dword(kGatherTraceLevel)) >= 2)
            trace(guest, h, 0x55046c, messagePlace, 0, 0, 0, 0);
        const u32 table = guest.dword(self + conn::kTransport);
        callHeld(guest, h, guest.dword(table + transport::kSend),
                   args({ guest.dword(self + conn::kHandle), self + conn::kPlayer, guest.dword(messagePlace), bye },
                        { 0x18 }));
    }
    else
    {
        if (guest.dword(server + game::kClosed) == 0)
        {
            const u32 table = guest.dword(self + conn::kTransport);
            callHeld(guest, h, guest.dword(table + transport::kStopListen),
                       args({ guest.dword(self + conn::kHandle), server + game::kAddress, server, table }));
            guest.setByte(guest.dword(self + conn::kGame) + game::kAddress, 0);
            if (sign(guest.dword(kGatherTraceLevel)) >= 2)
                trace(guest, h, 0x550460, messagePlace, 0, 0, 0, 0);
            for (u32 slot = 0;; ++slot)
            {
                const u32 now = guest.dword(self + conn::kGame);
                const u32 members = guest.dword(now + game::kMembers);
                if (members == 0 || guest.dword(now + game::kMaxPlayers) - 1 <= slot)
                    break;
                const u32 bit = u32(1) << (slot & 31);
                if ((members & bit) == 0)
                    continue;
                guest.setDword(now + game::kMembers, members & ~bit);
                const u32 sendTable = guest.dword(self + conn::kTransport);
                callHeld(guest, h, guest.dword(sendTable + transport::kSend),
                           args({ guest.dword(self + conn::kHandle),
                                  guest.dword(self + conn::kGame) + game::kSlots + slot * game::kSlotSize,
                                  guest.dword(messagePlace), bye },
                                { 0x18 }));
            }
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(self + conn::kGame));
        guest.setDword(self + conn::kGame, 0);
    }
    guest.setDword(self + conn::kInGame, 0);
    guest.setDword(self + conn::kPulse, 0);
    return 1;
}

nfs::u32 frontend::netcGatherLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514970", 0, 0, nullptr, 0, kAll };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherLeave(app, ts, self); });
}

/* sub_514af0: the players of a game, each handed to the caller's function:
 * from our own record if it is our game, asked of the server if not.  How many
 * went by, or whether the last answer came. */
nfs::u32 frontend::netc::gatherEnumPlayers(App* app, ThreadState& ts, u32 self, u32 player, u32 callback)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x120);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = player; h.ebx = callback;
    const u32 frame = guest.frame();
    const u32 reply = frame + 0xac, replyWords = frame + 0xb0;
    u32 wanted;
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
        && callHeld(guest, h, kCheckPlayer, args({ self, self + conn::kPlayer, wanted })) != 0)
    {
        // Our own game: the server's player, then the members.
        if (callHeld(guest, h, callback, args({ self, self + conn::kOwnPlayer, 0, 1 })) == 0)
            return 0;
        u32 went = 1;
        for (u32 slot = 0;; ++slot)
        {
            const u32 server = guest.dword(self + conn::kGame);
            h.edx = (guest.dword(server + game::kMaxPlayers) - 1);
            if (guest.dword(server + game::kMaxPlayers) - 1 <= slot)
                return went;
            const u32 bit = u32(1) << (slot & 31);
            if ((guest.dword(server + game::kMembers) & bit) == 0)
                continue;
            if (callHeld(guest, h, callback, args({ self, server + game::kSlots + slot * game::kSlotSize, slot + 1, 0 })) == 0)
                return went;
            ++went;
        }
    }
    copyDwords(guest, reply, 0x512ec0, 0x14);
    callHeld(guest, h, kRequest, args({ self, reply, kMessageRequest, frame }, { 0xac, guest.dword(kTicksPerSecond) * 2 }));
    if (guest.dword(replyWords) == 0)
        return 0;
    const u32 members = guest.dword(frame + 0xb4);
    guest.setDword(frame + 0xc4, callback);
    guest.setDword(replyWords, 1);
    guest.setDword(frame + 0x18, 0xffffffff);
    copyDwords(guest, frame + 4, self + conn::kPlayer, conn::kPlayerDwords);
    callHeld(guest, h, kRequest, args({ self, reply, kMessageGet, frame }, { 0x1c, guest.dword(kTicksPerSecond) * 2 }));
    u32 answered = guest.dword(replyWords) != 0 ? 1 : 0;
    h.ebx = (0);
    if (members == 0)
        return answered;
    for (u32 index = 0;;)
    {
        h.ebx = (index);
        if (sign(index) >= 0x1f || guest.dword(frame + 0xc4) == 0 || answered == 0)
            return answered;
        if (((u32(1) << (index & 31)) & members) != 0)
        {
            guest.setDword(frame + 0x18, byteSwapped(index));
            callHeld(guest, h, kRequest, args({ self, reply, kMessageGet, frame }, { 0x1c, guest.dword(kTicksPerSecond) * 2 }));
            answered = guest.dword(replyWords) != 0 ? 1 : 0;
            index &= ~(u32(1) << (index & 31));
            h.ebx = (index);
        }
        ++index;
        h.edx = (index);
    }
}

nfs::u32 frontend::netcGatherEnumPlayers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514af0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 self = args.eax, player = args.edx, callback = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherEnumPlayers(app, ts, self, player, callback); });
}

/* sub_514dc0: a player thrown out of our game, or an error if it is us or not our game.  0. */
nfs::u32 frontend::netc::gatherKillPlayer(App* app, ThreadState& ts, u32 self, u32 player)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = player;
    constexpr u32 kFile = 0x5503dc, kFunction = 0x550478;
    const u32 server = guest.dword(self + conn::kGame);
    h.edx = (server);
    if (server == 0)
    {
        reportError(guest, h, kFile, kFunction, 0x39f, 0x5504b8);
        return 0;
    }
    const u32 slot = loaded<0x5132a0, frontend::netb::findPlayerByAddress>(app, ts, h, server, player);
    if (slot == 0)
    {
        if (callHeld(guest, h, kCheckPlayer, args({ self, player, self + conn::kOwnPlayer })) != 0)
            reportError(guest, h, kFile, kFunction, 0x39b, 0x550488);
        return 0;
    }
    const u32 index = u32(sign(slot - (guest.dword(self + conn::kGame) + game::kSlots)) / sign(game::kSlotSize));
    const u32 table = guest.dword(self + conn::kTransport);
    callHeld(guest, h, guest.dword(table + transport::kSend),
               args({ guest.dword(self + conn::kHandle), slot, kMessageKick, 0 }, { 0 }));
    const u32 keep = ~(u32(1) << (index & 31));
    const u32 now = guest.dword(self + conn::kGame);
    guest.setDword(now + game::kMembers, guest.dword(now + game::kMembers) & keep);
    h.edx = (keep);
    return 0;
}

nfs::u32 frontend::netcGatherKillPlayer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514dc0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 self = args.eax, player = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherKillPlayer(app, ts, self, player); });
}

/* sub_514ec0: a packet connection opened over one of the two layers, its times
 * filled in from the clock's rate where it has none. */
nfs::u32 frontend::netc::gatherOpenConnection(App* app, ThreadState& ts, u32 first, u32 connection, u32 wanted,
                                                u32 window)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    constexpr u32 kUnreliableLayer = 0x56ac10, kPacketLayer = 0x56ac28;
    const u32 ticks = guest.dword(kTicksPerSecond);
    const u32 layer = window == 0xffffffff ? kUnreliableLayer : kPacketLayer;
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
    return fe::direct<0x4f5130, frontend::neta::serviceCall>(app, ts, connection, layer, kConnectionNames, wanted, window);
}

nfs::u32 frontend::netcGatherOpenConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514ec0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 first = args.eax, connection = args.edx, wanted = args.ebx, window = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherOpenConnection(app, ts, first, connection, wanted, window); });
}

/* sub_514f70: what came for a request in progress handed to it, and the
 * transport told, unless it is quiet.  1 if it was data. */
nfs::u32 frontend::netc::gatherReceive(App* app, ThreadState& ts, u32 self, u32 from, u32 kind, u32 length,
                                         u32 extraArgument, u32 acceptedArgument)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x68);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = from; h.ebx = kind; h.ecx = length;
    const u32 frame = guest.frame();
    const u32 kindPlace = frame + 0x50;
    const u32 extra = extraArgument, acceptedPlace = acceptedArgument;  // its stack arguments
    guest.setDword(kindPlace, h.ebx);
    copyDwords(guest, frame, 0x512f10, 0x14);
    guest.setDword(frame + 0x10, self);
    if (guest.dword(self + conn::kRequest) == 0)
        guest.setDword(self + conn::kRequest, frame);
    if (sign(guest.dword(kGatherTraceLevel)) >= 5)
        trace(guest, h, 0x5504f8, kindPlace, length, extra, 0, 0);
    callHeld(guest, h, kAnswerRequest, args({ guest.dword(self + conn::kRequest), from, guest.dword(kindPlace), length }, { extra }));
    if (guest.dword(self + conn::kRequest) == frame)
        guest.setDword(self + conn::kRequest, 0);
    if (guest.dword(self + conn::kQuiet) == 0)
    {
        const u32 table = guest.dword(self + conn::kTransport);
        callHeld(guest, h, guest.dword(table + transport::kAck), args({ guest.dword(self + conn::kHandle), 0, kMessageData, 0 }));
    }
    guest.setDword(acceptedPlace, guest.dword(kindPlace) != kMessageRefused ? 1 : 0);
    h.edx = (acceptedPlace);
    return guest.dword(kindPlace) == kMessageData ? 1 : 0;
}

nfs::u32 frontend::netcGatherReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514f70", 8, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 self = args.eax, from = args.edx, kind = args.ebx, length = args.ecx;
    const u32 extraArgument = app->getMemory<u32>(ts.sp + 4);
    const u32 acceptedArgument = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::gatherReceive(app, ts, self, from, kind, length, extraArgument, acceptedArgument); });
}

/* sub_515050: rasapi32.dll and its two functions found, once.  1 if they are there. */
nfs::u32 frontend::netc::loadRasApi(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    if (guest.dword(kRasLibrary) != 0)
        return 1;
    const u32 library = os::modules::load(app, guest::text(app, 0x550504)).value_or(0);
    guest.setDword(kRasLibrary, library);
    if (library == 0)
    {
        guest.setDword(kRasProjection, 0);
        guest.setDword(kRasEnum, 0);
        return 0;
    }
    const u32 enumerate = os::modules::symbol(app, library, guest::text(app, 0x550514));
    guest.setDword(kRasEnum, enumerate);
    const u32 projection = os::modules::symbol(app, guest.dword(kRasLibrary), guest::text(app, 0x550528));
    guest.setDword(kRasProjection, projection);
    return guest.dword(kRasEnum) != 0 && projection != 0 ? 1 : 0;
}

nfs::u32 frontend::netcLoadRasApi(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_515050", 0, 0, nullptr, 0, kAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::loadRasApi(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The two packet layers over a stream
 * ------------------------------------------------------------------------- */

namespace
{

// This file's own, called by address as the game calls them.
constexpr u32 kFrameChecksum = 0x51bc70, kFrameGood = 0x51bca0, kFrameBuild = 0x51bcd0, kFrameSend = 0x51bcf0;
constexpr u32 kFrameControl = 0x51bd50;
constexpr u32 kUnreliableNotify = 0x51c0a0;
constexpr u32 kPacketChecksum = 0x51c7c0, kPacketBuild = 0x51c890;
constexpr u32 kPacketSend = 0x51c8f0, kPacketControl = 0x51ca50;
constexpr u32 kQueueKey = 0x51d000, kCountDown = 0x51d010, kResendDue = 0x51d040, kPacketNotify = 0x51d050;

constexpr u32 kNextUnreliable = 0x56ac0c, kNextPacket = 0x56ac24;

/* A stream connection's functions (what a packet layer runs over). */
namespace stream
{
constexpr u32 kOpen = 0x00, kClose = 0x04, kReady = 0x08, kRoom = 0x0c, kAvailable = 0x10, kRead = 0x14, kWrite = 0x18;
constexpr u32 kMaxFrame = 0x1c, kMinFrame = 0x1e, kReliable = 0x1f;
}

/* A connection as the layers above see it. */
namespace link
{
constexpr u32 kState = 0x00, kLayer = 0x10, kProcess = 0x14, kDrop = 0x18, kTimedOut = 0x1c, kTimeout = 0x24;
constexpr u32 kRetries = 0x2a, kIdleRetries = 0x2c, kResendTicks = 0x2e;
}

/* What both layers begin with. */
namespace layer
{
constexpr u32 kLink = 0x00, kStream = 0x04, kMaxSize = 0x08, kOwned = 0x0c;
}

/* The unreliable layer: frame 0xfe, length, checksum, data. */
namespace plain
{
constexpr u32 kAlive = 0x0d, kId = 0x0e, kLock = 0x10, kBuffer = 0x14, kBufferSize = 0x18, kHeld = 0x1c;
constexpr u32 kIdleLeft = 0x20, kSize = 0x24;
}

/* The reliable layer: the same, numbered and acknowledged. */
namespace rely
{
constexpr u32 kId = 0x0d, kAckSent = 0x0e, kReceived = 0x10, kLock = 0x14, kFirstPacket = 0x18, kBuffer = 0x1c;
constexpr u32 kBufferSize = 0x20, kHeld = 0x24, kBad = 0x28, kIdleLeft = 0x2c, kNextSend = 0x30, kAcked = 0x34;
constexpr u32 kFree = 0x38, kUnacked = 0x54, kNextDeliver = 0x70, kNextFree = 0x74, kReceiving = 0x78;
constexpr u32 kReceivedQueue = 0x94, kSize = 0xb0;
}

/* A packet of the reliable layer, as it waits in a queue, its frame at +0xc. */
namespace packet
{
constexpr u32 kNumber = 0x04, kResend = 0x08, kFrame = 0x0c, kLength = 0x0d, kSequence = 0x0e, kAck = 0x0f;
constexpr u32 kData = 0x12, kFrameHeader = 6;
}

constexpr u32 kFrameStart = 0xfe, kSequenceMask = 0x7f, kFrameClose = 0xff, kFrameControlAck = 0xfb;

std::int32_t traceLevel(const Guest& guest)
{
    return sign(guest.dword(kTraceLevel));
}

/* A frame's checksum worked out and put in its place, big end first. */
u32 stampChecksum(Guest& guest, nfs::ThreadState& ts, u32 frame, u32 length, u32 place)
{
    Held h;
    guest.setByte(frame + place + 1, 0);
    guest.setByte(frame + place, 0);
    const u32 sum = loaded<0x4f4780, frontend::wata::blockChecksum>(guest.app(), ts, h, frame, length);
    const u32 high = u32(sign(sum) >> 8);
    guest.setByte(frame + place + 1, std::uint8_t(sum));
    guest.setByte(frame + place, std::uint8_t(high));
    return high;
}

/* Whether a frame's checksum is what it says, by stamping it again. */
u32 checksumGood(Guest& guest, u32 stamp, u32 frame, u32 length, u32 place)
{
    Held h;
    const std::uint16_t saved = guest.word(frame + place);
    callHeld(guest, h, stamp, args({ frame, length }));
    return saved == guest.word(frame + place) ? 1 : 0;
}

/* What is left of a stream's connection closed for good, once. */
u32 shutdownLayer(Guest& guest, u32 self, u32 idField, u32 shutdownText, u32 disconnectText, u32 closedText,
                    u32 duplicateText)
{
    Held h;
    if (traceLevel(guest) >= 2)
        traceEvent(guest, h, shutdownText, 1, guest.byte(self + idField));
    if (callHeld(guest, h, kQueueRemove, args({ kActiveConnections, guest.dword(self + layer::kLink) })) == 0)
    {
        if (traceLevel(guest) >= 1)
            traceEvent(guest, h, duplicateText, 1, guest.byte(self + idField));
        return h.eax;
    }
    const u32 connection = guest.dword(self + layer::kLink);
    const std::int32_t level = traceLevel(guest);
    h.ebx = (u32(level));
    if (level >= 1)
        traceEvent(guest, h, disconnectText, 1, guest.byte(self + idField));
    guest.setDword(connection + link::kTimeout, 0);
    const u32 ops = guest.dword(self + layer::kStream);
    callHeld(guest, h, guest.dword(ops + stream::kClose), args({ connection, ops }));
    callHeld(guest, h, guest.dword(connection + link::kDrop), args({ connection, connection }));
    callHeld(guest, h, kQueueAdd, args({ kClosedConnections, h.edx }));
    if (traceLevel(guest) >= 2)
        traceEvent(guest, h, closedText, 1, guest.byte(self + idField));
    return h.eax;
}

/* The stream read into the layer's buffer and its frames taken from there, what
 * is left moved down.  1 if any came whole; the frames count them on the frame. */
template <u32 FrameFunction, auto Core>
u32 receiveFrames(Guest& guest, nfs::ThreadState& ts, u32 self, u32 bufferField, u32 sizeField, u32 heldField)
{
    Held h;
    const u32 frame = guest.frame();
    guest.setDword(frame, 0);
    const u32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    guest.setDword(frame + 4, connection);
    guest.setDword(frame + 8, ops);
    if (guest.byte(ops + stream::kReliable) != 0)
        guest.setDword(self + heldField, 0);
    const u32 held = guest.dword(self + heldField), size = guest.dword(self + sizeField);
    if (size > held)
    {
        const u32 got = callHeld(guest, h, guest.dword(ops + stream::kRead),
                                     args({ connection, guest.dword(self + bufferField) + held, size - held, held }));
        guest.setDword(self + heldField, guest.dword(self + heldField) + got);
    }
    u32 left = guest.dword(self + heldField);
    if (left != 0)
    {
        u32 done = 0;
        for (;;)
        {
            const u32 used = loaded<FrameFunction, Core>(guest.app(), ts, h, self, frame,
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
            const u32 buffer = guest.dword(self + bufferField);
            callHeld(guest, h, kMoveMemory, args({ buffer + done, buffer, left }));
            guest.setDword(self + heldField, left);
        }
    }
    return guest.dword(frame) != 0 ? 1 : 0;
}

}  // namespace

/* sub_51bc70: an unreliable frame's checksum stamped. */
nfs::u32 frontend::netc::frameChecksum(App* app, ThreadState& ts, u32 frame, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    return stampChecksum(guest, ts, frame, length, 2);
}

nfs::u32 frontend::netcFrameChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bc70", 0, 0, nullptr, 0, kEcx };
    const u32 frame = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::frameChecksum(app, ts, frame, length); });
}

/* sub_51bca0: whether an unreliable frame's checksum holds. */
nfs::u32 frontend::netc::frameChecksumGood(App* app, ThreadState& ts, u32 frame, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    return checksumGood(guest, kFrameChecksum, frame, length, 2);
}

nfs::u32 frontend::netcFrameChecksumGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bca0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx) };
    const u32 frame = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::frameChecksumGood(app, ts, frame, length); });
}

/* sub_51bcd0: an unreliable frame's start and data. */
nfs::u32 frontend::netc::frameBuild(App* app, ThreadState& ts, u32 out, u32 self, u32 data, u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = out; h.edx = self; h.ebx = data; h.ecx = length;
    guest.setByte(out, kFrameStart);
    guest.setByte(out + 1, std::uint8_t(length));
    return callHeld(guest, h, kMoveMemory, args({ data, out + 4, length }));
}

nfs::u32 frontend::netcFrameBuild(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bcd0", 0, 0, nullptr, 0, kEsi };
    const u32 out = args.eax, self = args.edx, data = args.ebx, length = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::frameBuild(app, ts, out, self, data, length); });
}

/* sub_51bcf0: an unreliable frame written if the stream has room.  1 if all of it went. */
nfs::u32 frontend::netc::frameSend(App* app, ThreadState& ts, u32 out, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = out; h.edx = self;
    const u32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    const u32 size = guest.byte(out + 1) + u32(4);
    if (callHeld(guest, h, guest.dword(ops + stream::kRoom), args({ connection, 0, self, size })) < size)
        return h.edx;
    guest::direct<0x51bc70, frameChecksum>(app, ts, out, size);
    const u32 sent = callHeld(guest, h, guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                  args({ connection, out, size }));
    const u32 whole = sent == size ? 1 : 0;
    h.edx = (whole);
    return whole;
}

nfs::u32 frontend::netcFrameSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bcf0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 out = args.eax, self = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::frameSend(app, ts, out, self); });
}

/* sub_51bd50: a frame of no data, its length the `code`, written if the stream has room. */
nfs::u32 frontend::netc::frameSendControl(App* app, ThreadState& ts, u32 self, u32 code)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = code;
    const u32 frame = guest.frame();
    const u32 connection = guest.dword(self + layer::kLink), ops = guest.dword(self + layer::kStream);
    if (callHeld(guest, h, guest.dword(ops + stream::kRoom), args({ connection, 0, code, self })) < 4)
        return h.edx;
    guest.setByte(frame + 1, std::uint8_t(code));
    guest.setByte(frame, kFrameStart);
    // The game's code hands it 4 in ebx too, which it does not read; the call after loads ebx again.
    guest::direct<0x51bc70, frameChecksum>(app, ts, frame, 4);
    const u32 now = guest.dword(self + layer::kStream);
    const u32 sent = callHeld(guest, h, guest.dword(now + stream::kWrite), args({ connection, frame, 4, now }));
    const u32 whole = sent == 4 ? 1 : 0;
    h.edx = (whole);
    return whole;
}

nfs::u32 frontend::netcFrameSendControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bd50", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 self = args.eax, code = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::frameSendControl(app, ts, self, code); });
}

/* sub_51bdc0: the unreliable layer's connection closed. */
nfs::u32 frontend::netc::unreliableShutdown(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    return shutdownLayer(guest, h.eax, plain::kId, 0x550cb4, 0x550cc4, 0x550cd0, 0x550cdc);
}

nfs::u32 frontend::netcUnreliableShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bdc0", 0, 0, nullptr, 0, kAll };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableShutdown(app, ts, self); });
}

/* sub_51be90: the first frame found in what the stream gave: delivered, or
 * skipped.  What was used of the buffer. */
nfs::u32 frontend::netc::unreliableReceiveFrame(App* app, ThreadState& ts, u32 self, u32 counted, u32 buffer,
                                                  u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = counted; h.ebx = buffer; h.ecx = length;
    const u32 connection = guest.dword(self + layer::kLink);
    const std::uint8_t reliable = guest.byte(guest.dword(self + layer::kStream) + stream::kReliable);
    h.ebx = (length);
    u32 used = 0;
    if (reliable == 0)
    {
        const u32 end = length + buffer;
        h.edx = (end);
        for (u32 at = buffer; at < end && guest.byte(at) != kFrameStart; ++at)
            ++used;
    }
    else
    {
        h.edx = ((h.edx & 0xffffff00) | reliable);
    }
    const u32 frame = used + buffer;
    if (guest.byte(frame) != kFrameStart)
        return length;
    u32 end = used + 4;
    h.ecx = (end);
    if (end > length)
        return used;
    const u32 size = guest.byte(frame + 1);
    h.edx = (size);
    if (size > guest.dword(self + layer::kMaxSize))
        return used + (size == kFrameStart ? 1 : 2);
    end += size;
    h.ecx = (end);
    if (end > length)
        return used;
    const u32 whole = end - used;
    h.ecx = (whole);
    if (callHeld(guest, h, kFrameGood, args({ frame, whole })) == 0)
        return used + 2;
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
        guest.setByte(self + plain::kAlive, 1);
        h.edx = (connection);
    }
    used += whole;
    const u32 kind = guest.byte(frame + 1);
    h.edx = ((h.edx & 0xffff00ff) | (kind << 8));
    if (kind == kFrameClose)
    {
        if (traceLevel(guest) >= 2)
            traceEvent(guest, h, 0x550cec, 1, guest.byte(self + plain::kId));
        guest::direct<0x51bdc0, unreliableShutdown>(app, ts, self);
        return used;
    }
    if (kind == 0)
        return used;
    if (traceLevel(guest) >= 3)
        trace(guest, h, 0x550cfc, 0, frame + 4, kind, 0, guest.byte(self + plain::kId));
    callHeld(guest, h, kDeliver, args({ connection, frame + 4, guest.byte(frame + 1) }));
    return used;
}

nfs::u32 frontend::netcUnreliableReceiveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51be90", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 self = args.eax, counted = args.edx, buffer = args.ebx, length = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableReceiveFrame(app, ts, self, counted, buffer, length); });
}

/* sub_51bfd0: all that came for the unreliable layer. */
nfs::u32 frontend::netc::unreliableReceive(App* app, ThreadState& ts, u32 self)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    return receiveFrames<0x51be90, unreliableReceiveFrame>(guest, ts, self, plain::kBuffer, plain::kBufferSize, plain::kHeld);
}

nfs::u32 frontend::netcUnreliableReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bfd0", 0, 0, nullptr, 0, kAll };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableReceive(app, ts, self); });
}

/* sub_51c0a0: the stream says something came: read while there is. */
nfs::u32 frontend::netc::unreliableNotify(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    const u32 self = guest.dword(connection + link::kLayer);
    const u32 ops = guest.dword(self + layer::kStream);
    h.edx = (guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
    {
        const u32 ebx = h.ebx, ecx = h.ecx;
        traceEvent(guest, h, 0x550d04, 0, guest.byte(self + plain::kId));
        h.ebx = (ebx);
        h.ecx = (ecx);
    }
    if (guest.dword(connection + link::kTimeout) == 0)
        return h.eax;
    for (;;)
    {
        if (callHeld(guest, h, guest.dword(ops + stream::kAvailable), args({ connection })) == 0)
            return 0;
        if (guest::direct<0x51bfd0, unreliableReceive>(app, ts, self) == 0)
            return 0;
    }
}

nfs::u32 frontend::netcUnreliableNotify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c0a0", 0, 0, nullptr, 0, std::uint8_t(kEdx | kEsi | kEdi) };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableNotify(app, ts, connection); });
}

/* sub_51c100: a connection opened on the unreliable layer: the layer's record
 * made unless it has one, the stream asked, and waited for until it stands or
 * has failed.  1 if it stands; with no connection, the record's size. */
nfs::u32 frontend::netc::unreliableOpen(App* app, ThreadState& ts, u32 connection, u32 ops, u32 asked)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x1c);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection; h.edx = ops; h.ebx = asked;
    constexpr u32 kFile = 0x550d10, kFunction = 0x550d20;
    u32 wanted = asked;
    if (sign(asked) < 6)
    {
        wanted = 6;
    }
    else if (sign(asked) <= 0xfa)
    {
        const u32 limit = guest.word(ops + stream::kMaxFrame);
        h.ebx = ((h.ebx & 0xffff0000) | limit);
        if (limit != 0 && asked + 4 > limit)
            reportError(guest, h, kFile, kFunction, 0x12a, 0x550d7c);
    }
    else
    {
        reportError(guest, h, kFile, kFunction, 0x128, 0x550d3c, 0xfa);
        h.ebx = (kFile);
    }
    if (connection != 0 && guest.dword(connection + link::kTimeout) != 0)
    {
        reportError(guest, h, kFile, kFunction, 0x12d, 0x550dcc);
        h.ebx = (kFile);
    }
    const u32 least = guest.byte(ops + stream::kMinFrame);
    if (sign(least) > sign(wanted))
        wanted = least;
    const std::uint8_t reliable = guest.byte(ops + stream::kReliable);
    const u32 bufferSize = reliable != 0 ? 0xfe : 0x1fc;
    h.ecx = (bufferSize);
    h.edx = ((ops & 0xffffff00) | reliable);
    const u32 total = bufferSize + plain::kSize;
    if (connection == 0)
        return total;
    u32 owned = 0;
    u32 self = guest.dword(connection + link::kLayer);
    h.ebx = (self);
    if (self == 0)
    {
        guest.setDword(kErrorFile, kFile);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x145);
        self = callHeld(guest, h, kAllocate, args({ 0x550e04, total, guest.dword(kMemoryType), bufferSize }));
        owned = 1;
        if (self == 0)
            return 0;
    }
    guest.setDword(connection + link::kState, 0);
    guest.setDword(connection + link::kTimeout, 0);
    const u32 oldDrop = guest.dword(connection + link::kDrop);
    guest.setDword(connection + link::kLayer, self);
    guest.setDword(connection + link::kDrop, kDropFunction);
    guest.setDword(self + layer::kLink, connection);
    guest.setDword(self + layer::kMaxSize, wanted);
    guest.setDword(self + plain::kIdleLeft, guest.word(connection + link::kRetries));
    guest.setByte(self + layer::kOwned, std::uint8_t(owned));
    guest.setDword(self + layer::kStream, ops);
    const u32 id = guest.dword(kNextUnreliable);
    guest.setWord(self + plain::kId, std::uint16_t(id));
    guest.setByte(self + plain::kAlive, 0);
    guest.setDword(kNextUnreliable, id + 1);
    const u32 lock = u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    guest.setDword(self + plain::kHeld, 0);
    guest.setDword(self + plain::kLock, lock);
    guest.setDword(self + plain::kBuffer, self + plain::kSize);
    guest.setDword(self + plain::kBufferSize, bufferSize);
    if (guest.dword(kConnectionTimeout) == 0)
    {
        reportError(guest, h, kFile, kFunction, 0x16d, 0x550e0c);
        h.ebx = (kFunction);
    }
    h.ecx = (ops);
    u32 failed = 0;
    if (callHeld(guest, h, guest.dword(ops + stream::kOpen), args({ connection, kUnreliableNotify })) == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, h, 0x550e60, 0, guest.byte(self + plain::kId));
    }
    else
    {
        guest.setDword(connection + link::kTimeout, kNeverTimesOut);
        callHeld(guest, h, kQueueAdd, args({ kActiveConnections, connection }));
        if (traceLevel(guest) >= 1)
            traceEvent(guest, h, 0x550e34, 0, guest.byte(self + plain::kId));
        for (;;)
        {
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) == 0)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
                failed = callHeld(guest, h, guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            else if (sign(callHeld(guest, h, kWaitSystemTasks, args({ 1 }))) < 0)
            {
                failed = 1;
            }
            else
            {
                failed = callHeld(guest, h, guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            if (failed != 0 && traceLevel(guest) >= 2)
                traceEvent(guest, h, 0x550e40, 0, guest.byte(self + plain::kId));
            if (guest.dword(connection + link::kTimeout) != 0)
                guest.setDword(connection + link::kTimeout, kNeverTimesOut);
            if (guest.dword(connection + link::kTimeout) == 0 || guest.byte(self + plain::kAlive) != 0 || failed != 0)
                break;
        }
        if (guest.dword(connection + link::kTimeout) == 0 || failed != 0)
        {
            const u32 timeout = guest.dword(connection + link::kTimeout);
            h.ebx = (timeout);
            if (timeout == 0 && traceLevel(guest) >= 2)
                traceEvent(guest, h, 0x550e50, 0, guest.byte(self + plain::kId));
            guest.setDword(connection + link::kTimeout, 0);
            callHeld(guest, h, guest.dword(ops + stream::kClose), args({ connection, ops }));
        }
    }
    if (guest.dword(connection + link::kTimeout) != 0 && failed == 0)
    {
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
        guest.setDword(connection + link::kDrop, oldDrop);
        if (traceLevel(guest) >= 1)
            traceEvent(guest, h, 0x550e70, 0, guest.byte(self + plain::kId));
        return 1;
    }
    if (traceLevel(guest) >= 1)
        traceEvent(guest, h, 0x550e84, 0, guest.byte(self + plain::kId));
    callHeld(guest, h, kQueueRemove, args({ kActiveConnections, connection }));
    callHeld(guest, h, kQueueRemove, args({ kClosedConnections, connection, owned }));
    if (owned != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 0;
}

nfs::u32 frontend::netcUnreliableOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c100", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 connection = args.eax, ops = args.edx, asked = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableOpen(app, ts, connection, ops, asked); });
}

/* sub_51c510: data sent on the unreliable layer, made a frame on the stack.
 * (The original's test for no buffer is of its stack pointer, never zero.) */
nfs::u32 frontend::netc::unreliableSend(App* app, ThreadState& ts, u32 connection, u32 data, u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x110);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection; h.edx = data; h.ebx = length;
    const u32 frame = guest.frame();
    if (connection == 0 || guest.dword(connection + link::kTimeout) == 0)
        return 0;
    const u32 self = guest.dword(connection + link::kLayer);
    if (length > guest.dword(self + layer::kMaxSize))
    {
        reportError(guest, h, 0x550d10, 0x550e98, 0x1b1, 0x550eb0);
        h.ebx = (0x550e98);
    }
    callHeld(guest, h, kFrameBuild, args({ frame, self, data, length }));
    callHeld(guest, h, kFrameSend, args({ frame, self }));
    return 1;
}

nfs::u32 frontend::netcUnreliableSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c510", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 connection = args.eax, data = args.edx, length = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableSend(app, ts, connection, data, length); });
}

/* sub_51c5c0: the unreliable layer's turn: counts its timeout down, says it is
 * alive when it has been quiet, closes on timing out.  Always 1. */
nfs::u32 frontend::netc::unreliableProcess(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    const u32 self = guest.dword(connection + link::kLayer);
    h.edx = (guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
        traceEvent(guest, h, 0x550ef0, 0, guest.byte(self + plain::kId));
    const u32 left = guest.dword(connection + link::kTimeout) - 1;
    guest.setDword(connection + link::kTimeout, left);
    h.ebx = (left);
    if (left == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, h, 0x550efc, 0, guest.byte(self + plain::kId));
        guest.setDword(connection + link::kTimeout, callHeld(guest, h, guest.dword(connection + link::kTimedOut), args({ connection })));
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        const u32 ops = guest.dword(self + layer::kStream);
        if (callHeld(guest, h, guest.dword(ops + stream::kReady), args({ connection, ops })) != 0)
        {
            const u32 idle = guest.dword(self + plain::kIdleLeft);
            guest.setDword(self + plain::kIdleLeft, idle - 1);
            if (sign(idle) <= 0)
                guest::direct<0x51bd50, frameSendControl>(app, ts, self, 0);
            return 1;
        }
    }
    if (traceLevel(guest) >= 2)
        trace(guest, h, 0x550f08, connection + link::kTimeout, 0, 0, 1, guest.byte(self + plain::kId));
    guest::direct<0x51bdc0, unreliableShutdown>(app, ts, self);
    return 1;
}

nfs::u32 frontend::netcUnreliableProcess(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c5c0", 0, 0, nullptr, 0, kAll };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableProcess(app, ts, connection); });
}

/* sub_51c690: the unreliable layer's connection closed by us: the other end told
 * if it can be, the record gone.  1 if it was there. */
nfs::u32 frontend::netc::unreliableClose(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    if (connection == 0)
        return 0;
    const u32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
        return 0;
    const u32 ops = guest.dword(self + layer::kStream);
    h.ecx = (guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 1)
    {
        const u32 ebx = h.ebx;
        traceEvent(guest, h, 0x550f14, 0, guest.byte(self + plain::kId));
        h.ebx = (ebx);
    }
    if (callHeld(guest, h, kQueueRemove, args({ kActiveConnections, connection })) == 0)
    {
        if (guest.dword(connection + link::kTimeout) != 0)
            callHeld(guest, h, kMonoPrintf, args({ 0 }, { 0x550f24 }));
    }
    else
    {
        if (guest.dword(connection + link::kTimeout) != 0)
        {
            guest.setDword(connection + link::kTimeout, 0);
            u32 tries = 4;
            for (;;)
            {
                --tries;
                h.ecx = (tries);
                if (tries == 0xffffffff
                    || callHeld(guest, h, guest.dword(ops + stream::kReady), args({ connection })) == 0)
                    break;
                // By address: the edx it leaves is handed to the stream's functions after it.
                if (callHeld(guest, h, kFrameControl, args({ self, kFrameClose })) != 0)
                    tries = 0;
                else
                    loaded<0x4f06e0, frontend::wata::timerWait>(app, ts, h, 1);
            }
            loaded<0x4f06e0, frontend::wata::timerWait>(app, ts, h, 3);
            callHeld(guest, h, guest.dword(ops + stream::kClose), args({ connection }));
        }
        fe::direct<0x50de50, netb::qpush>(app, ts, kClosedConnections, connection);
    }
    if (callHeld(guest, h, kQueueRemove, args({ kClosedConnections, connection })) == 0)
        return 0;
    u32(*fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(self + plain::kLock)));
    if (guest.byte(self + layer::kOwned) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 1;
}

nfs::u32 frontend::netcUnreliableClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c690", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx | kEsi | kEdi) };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::unreliableClose(app, ts, connection); });
}

/* sub_51c7a0: a number of few bits widened, near what is expected, going on past it. */
nfs::u32 frontend::netc::sequenceWiden(App* app, ThreadState& ts, u32 expected, u32 low, u32 mask)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    u32 wide = (~mask & expected) | low;
    if (wide < expected)
    {
        wide += mask + 1;
    }
    return wide;
}

nfs::u32 frontend::netcSequenceWiden(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c7a0", 0, 0, nullptr, 0, kEcx };
    const u32 expected = args.eax, low = args.edx, mask = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::sequenceWiden(app, ts, expected, low, mask); });
}

/* sub_51c7c0: a reliable frame's checksum stamped. */
nfs::u32 frontend::netc::packetChecksum(App* app, ThreadState& ts, u32 frame, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    return stampChecksum(guest, ts, frame, length, 4);
}

nfs::u32 frontend::netcPacketChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c7c0", 0, 0, nullptr, 0, kEcx };
    const u32 frame = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetChecksum(app, ts, frame, length); });
}

/* sub_51c7f0: whether a reliable frame's checksum holds. */
nfs::u32 frontend::netc::packetChecksumGood(App* app, ThreadState& ts, u32 frame, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    // The arguments where the game's code hands them: the code below reads them there.
    return checksumGood(guest, kPacketChecksum, frame, length, 4);
}

nfs::u32 frontend::netcPacketChecksumGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c7f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx) };
    const u32 frame = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetChecksumGood(app, ts, frame, length); });
}

/* sub_51c820: the number to acknowledge in what is sent now, the idle time starting over. */
nfs::u32 frontend::netc::packetAckNumber(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    const u32 frame = guest.frame();
    // The game's code hands it `self` in edx too, which it does not take; there it stays.
    u32 number = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, ts, h, self + rely::kReceiving);
    if (number == 0xffffffff)
        number = guest.dword(self + rely::kNextFree);
    --number;
    guest.setDword(frame, number);
    u32 result = guest.word(guest.dword(self + layer::kLink) + link::kRetries);
    guest.setDword(self + rely::kIdleLeft, result);
    if (traceLevel(guest) >= 3)
    {
        const u32 ebx = h.ebx;
        trace(guest, h, 0x550f68, frame, 0, 0, 0, guest.byte(self + rely::kId));
        h.ebx = (ebx);
        result = h.eax;
    }
    return (result & 0xffffff00) | (guest.byte(frame) & kSequenceMask);
}

nfs::u32 frontend::netcPacketAckNumber(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c820", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetAckNumber(app, ts, self); });
}

/* sub_51c890: a packet numbered and its frame's start and data made. */
nfs::u32 frontend::netc::packetBuild(App* app, ThreadState& ts, u32 out, u32 self, u32 data, u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = out; h.edx = self; h.ebx = data; h.ecx = length;
    const u32 queue = self + rely::kUnacked;
    const u32 locked = fe::direct<0x50ea20, frontend::menz::fileLockEnter>(app, ts, queue);
    const u32 number = guest.dword(self + rely::kNextSend);
    guest.setDword(self + rely::kNextSend, number + 1);
    guest.setDword(out + packet::kNumber, number);
    fe::direct<0x50ea40, frontend::menz::fileLockLeave>(app, ts, queue, locked);
    guest.setDword(out + packet::kResend, 0xffffffff);
    guest.setByte(out + packet::kFrame, kFrameStart);
    guest.setByte(out + packet::kSequence, guest.byte(out + packet::kNumber) & kSequenceMask);
    guest.setByte(out + packet::kLength, std::uint8_t(length));
    return callHeld(guest, h, kMoveMemory, args({ data, out + packet::kData, length }));
}

nfs::u32 frontend::netcPacketBuild(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c890", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 out = args.eax, self = args.edx, data = args.ebx, length = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetBuild(app, ts, out, self, data, length); });
}

/* sub_51c8f0: a packet written if the stream has room, its resend time started.
 * 1 if it went. */
nfs::u32 frontend::netc::packetSend(App* app, ThreadState& ts, u32 out, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = out; h.edx = self;
    const u32 sizePlace = guest.frame();
    const u32 connection = guest.dword(self + layer::kLink);
    const u32 room = callHeld(guest, h, guest.dword(guest.dword(self + layer::kStream) + stream::kRoom),
                                  args({ connection, guest.dword(self + layer::kStream), h.ebx, self }));
    const u32 number = out + packet::kNumber;
    const u32 needed = guest.byte(out + packet::kLength) + packet::kFrameHeader;
    h.edx = (needed);
    bool went = false;
    if (room < needed)
    {
        if (sign(guest.dword(out + packet::kResend)) < 0)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, h, 0x550f90, number, 0, 0, 1, guest.byte(self + rely::kId));
        }
        else if (traceLevel(guest) >= 3)
        {
            trace(guest, h, 0x550f98, number, 0, 0, 1, guest.byte(self + rely::kId));
        }
    }
    else
    {
        const u32 ack = guest::direct<0x51c820, packetAckNumber>(app, ts, self);
        const u32 size = guest.byte(out + packet::kLength) + packet::kFrameHeader;
        guest.setDword(sizePlace, size);
        guest.setByte(out + packet::kAck, std::uint8_t(ack));
        const u32 frame = out + packet::kFrame;
        guest::direct<0x51c7c0, packetChecksum>(app, ts, frame, guest.byte(out + packet::kLength) + packet::kFrameHeader);
        const u32 sent = callHeld(guest, h, guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                      args({ connection, frame, guest.dword(sizePlace) }));
        const u32 expected = guest.dword(sizePlace);
        h.edx = (expected);
        if (sent == expected)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, h, sign(guest.dword(out + packet::kResend)) >= 0 ? 0x550f78 : 0x550f70, number,
                      out + packet::kData, guest.byte(out + packet::kLength), 0, guest.byte(self + rely::kId));
            went = true;
        }
        else if (traceLevel(guest) >= 1)
        {
            trace(guest, h, 0x550f80, sizePlace, frame, expected, 1, guest.byte(self + rely::kId));
        }
    }
    if (!went)
    {
        guest.setDword(out + packet::kResend, 1);
        return 0;
    }
    h.edx = (connection);
    guest.setDword(out + packet::kResend, guest.word(connection + link::kResendTicks));
    return 1;
}

nfs::u32 frontend::netcPacketSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51c8f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 out = args.eax, self = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetSend(app, ts, out, self); });
}

/* sub_51ca50: a frame of no data, its number the `code`, written if the stream
 * has room; `control` holds it and its resend time.  1 if it went. */
nfs::u32 frontend::netc::packetSendControl(App* app, ThreadState& ts, u32 control, u32 self, u32 code)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = control; h.edx = self; h.ebx = code;
    const u32 connection = guest.dword(self + layer::kLink);
    h.ecx = (control);
    guest.setDword(control + packet::kResend, 1);
    const u32 ops = guest.dword(self + layer::kStream);
    if (callHeld(guest, h, guest.dword(ops + stream::kRoom), args({ connection, 0 })) < packet::kFrameHeader)
        return h.edx;
    const u32 ack = guest::direct<0x51c820, packetAckNumber>(app, ts, self) & 0xff;
    const u32 frame = control + packet::kFrame;
    guest.setByte(frame, kFrameStart);
    guest.setByte(control + packet::kSequence, std::uint8_t(code));
    guest.setByte(control + packet::kAck, std::uint8_t(ack));
    guest.setByte(control + packet::kLength, 0);
    // The game's code hands it the length in ebx too, which it does not read; the write below reads it there.
    h.ebx = (packet::kFrameHeader);
    guest::direct<0x51c7c0, packetChecksum>(app, ts, frame, packet::kFrameHeader);
    const u32 sent = callHeld(guest, h, guest.dword(guest.dword(self + layer::kStream) + stream::kWrite),
                                  args({ connection, frame }));
    const u32 whole = sent == packet::kFrameHeader ? 1 : 0;
    h.edx = (whole);
    if (whole != 0 && code <= kSequenceMask)
        guest.setDword(control + packet::kResend, guest.word(connection + 0x2c));
    return whole;
}

nfs::u32 frontend::netcPacketSendControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51ca50", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 control = args.eax, self = args.edx, code = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetSendControl(app, ts, control, self, code); });
}

/* sub_51caf0: a packet that came out of order asked for again, when its time has
 * come.  1, or what asking gave. */
nfs::u32 frontend::netc::packetSendNak(App* app, ThreadState& ts, u32 waiting, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = waiting; h.edx = self;
    if (waiting == 0)
        return 1;
    const u32 left = guest.dword(waiting + packet::kResend);
    h.edx = (left - 1);
    guest.setDword(waiting + packet::kResend, left - 1);
    if (sign(left) > 0)
        return 1;
    if (traceLevel(guest) >= 3)
    {
        const u32 ecx = h.ecx;
        trace(guest, h, 0x550fa0, waiting + packet::kNumber, 0, 0, 1, guest.byte(self + rely::kId));
        h.ecx = (ecx);
    }
    return callHeld(guest, h, kPacketControl, args({ guest.dword(self + rely::kFirstPacket), self,
                                             guest.dword(waiting + packet::kNumber) & kSequenceMask }));
}

nfs::u32 frontend::netcPacketSendNak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51caf0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 waiting = args.eax, self = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetSendNak(app, ts, waiting, self); });
}

/* sub_51cb50: what the other end has now: what it acknowledged taken off those waiting. */
nfs::u32 frontend::netc::packetGotAck(App* app, ThreadState& ts, u32 self, u32 ack)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = ack;
    const u32 frame = guest.frame();
    const u32 acked = loaded<0x51c7a0, sequenceWiden>(app, ts, h, guest.dword(self + rely::kAcked), h.edx & 0xff,
                                                         kSequenceMask);
    guest.setDword(frame, acked);
    if (acked < guest.dword(self + rely::kNextSend) && acked > guest.dword(self + rely::kAcked))
    {
        guest.setDword(self + rely::kAcked, acked);
        if (traceLevel(guest) >= 3)
            trace(guest, h, 0x550fa8, frame, 0, 0, 0, guest.byte(self + rely::kId));
    }
    else if (traceLevel(guest) >= 3)
    {
        trace(guest, h, 0x550fb0, frame, 0, 0, 0, guest.byte(self + rely::kId));
    }
    const u32 unacked = self + rely::kUnacked;
    for (;;)
    {
        const u32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, ts, h, unacked);
        if (first > guest.dword(self + rely::kAcked))
            return first;
        const u32 done = loaded<0x50df60, frontend::netb::qpop>(app, ts, h, unacked);
        if (done == 0)
            return 0;
        callHeld(guest, h, kQueueAdd, args({ self + rely::kFree, done }));
    }
}

nfs::u32 frontend::netcPacketGotAck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51cb50", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 self = args.eax, ack = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetGotAck(app, ts, self, ack); });
}

/* sub_51cc00: the other end wants a packet again: its time to resend is now. */
nfs::u32 frontend::netc::packetGotNak(App* app, ThreadState& ts, u32 self, u32 sequence)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = sequence;
    const u32 frame = guest.frame();
    const u32 unacked = self + rely::kUnacked;
    const u32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, ts, h, unacked);
    const u32 number = loaded<0x51c7a0, sequenceWiden>(app, ts, h, first, h.edx & 0xff, kSequenceMask);
    guest.setDword(frame, number);
    const u32 found = callHeld(guest, h, kQueueFindKey, args({ unacked, number }));
    if (found == 0)
        return 0;
    if (traceLevel(guest) >= 3)
        trace(guest, h, 0x550fb8, frame, 0, 0, 1, guest.byte(self + rely::kId));
    guest.setDword(found + packet::kResend, 0);
    return h.eax;
}

nfs::u32 frontend::netcPacketGotNak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51cc00", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 self = args.eax, sequence = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetGotNak(app, ts, self, sequence); });
}

/* sub_51cc80: the reliable layer's connection closed. */
nfs::u32 frontend::netc::packetShutdown(App* app, ThreadState& ts, u32 self)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self;
    return shutdownLayer(guest, h.eax, rely::kId, 0x550fc0, 0x550fd0, 0x550fdc, 0x550fe8);
}

nfs::u32 frontend::netcPacketShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51cc80", 0, 0, nullptr, 0, kAll };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetShutdown(app, ts, self); });
}

/* sub_51cd50: the first frame found in what the stream gave: its acknowledgement
 * seen to and its data kept for its turn, or a resend asked for or a close taken.
 * What was used of the buffer; frames that were not whole counted as bad. */
nfs::u32 frontend::netc::packetReceiveFrame(App* app, ThreadState& ts, u32 self, u32 counted, u32 buffer, u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x1c);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = self; h.edx = counted; h.ebx = buffer; h.ecx = length;
    const u32 connection = guest.dword(self + layer::kLink);
    const std::uint8_t reliable = guest.byte(guest.dword(self + layer::kStream) + stream::kReliable);
    h.ebx = (length);
    u32 used = 0, whole = 0;
    if (reliable == 0)
    {
        const u32 end = length + buffer;
        h.edx = (end);
        for (u32 at = buffer; at < end && guest.byte(at) != kFrameStart; ++at)
            ++used;
    }
    else
    {
        h.edx = ((h.edx & 0xffffff00) | reliable);
    }
    const u32 frame = used + buffer;
    if (guest.byte(frame) != kFrameStart)
        return length;
    const auto finished = [&]() -> u32 {
        h.edx = (whole);
        const u32 bad = guest.dword(self + rely::kBad) + (used > whole ? 1 : 0);
        guest.setDword(self + rely::kBad, bad);
        h.ecx = (bad);
        return used;
    };
    u32 end = used + packet::kFrameHeader;
    if (end > length)
        return finished();
    const u32 size = guest.byte(frame + 1);
    if (size > guest.dword(self + layer::kMaxSize))
    {
        used += size == kFrameStart ? 1 : 2;
        return finished();
    }
    end += size;
    if (end > length)
        return finished();
    const u32 bytes = end - used;
    h.ecx = (bytes);
    if (guest::direct<0x51c7f0, packetChecksumGood>(app, ts, frame, bytes) == 0)
    {
        used += 3;
        return finished();
    }
    if (guest.dword(connection + link::kTimeout) != 0)
        guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
    guest.setDword(self + rely::kReceived, guest.dword(self + rely::kReceived) + 1);
    const u32 ack = guest.byte(frame + 3);
    used += bytes;
    if (ack <= kSequenceMask)
        guest::direct<0x51cb50, packetGotAck>(app, ts, self, ack);
    whole = bytes;
    const u32 dataLength = guest.byte(frame + 1), sequence = guest.byte(frame + 2);
    h.ebx = ((h.ebx & 0xffffff00) | dataLength);
    if (dataLength != 0)
        h.ebx = ((h.ebx & 0xffff00ff) | (sequence << 8));
    if (dataLength != 0 && sequence <= kSequenceMask)
    {
        const u32 number = loaded<0x51c7a0, sequenceWiden>(app, ts, h, guest.dword(self + rely::kNextDeliver), sequence,
                                                              kSequenceMask);
        // The game's code hands it the queue in ebx too, which it does not take; there it stays.
        h.ebx = (self + rely::kReceiving);
        const u32 slot = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, ts, h, self + rely::kReceiving, number);
        guest.setByte(self + rely::kAckSent, 0);
        if (slot != 0)
        {
            callHeld(guest, h, kMoveMemory, args({ frame, slot + packet::kFrame, bytes }));
            loaded<0x50e0a0, frontend::netb::qinsert>(app, ts, h, self + rely::kReceivedQueue, slot);
            guest.setDword(counted, guest.dword(counted) + 1);
            if (traceLevel(guest) >= 3)
                trace(guest, h, 0x550ff8, slot + packet::kNumber, slot + packet::kData, guest.byte(slot + packet::kLength),
                      0, guest.byte(self + rely::kId));
        }
    }
    else if (sequence <= kSequenceMask)
    {
        h.ecx = ((h.ecx & 0xffffff00) | sequence);
        guest::direct<0x51cc00, packetGotNak>(app, ts, self, sequence);
    }
    else if (sequence == kFrameClose)
    {
        h.ecx = ((h.ecx & 0xffffff00) | sequence);
        if (traceLevel(guest) >= 2)
            traceEvent(guest, h, 0x551000, 1, guest.byte(self + rely::kId));
        guest::direct<0x51cc80, packetShutdown>(app, ts, self);
    }
    return finished();
}

nfs::u32 frontend::netcPacketReceiveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51cd50", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 self = args.eax, counted = args.edx, buffer = args.ebx, length = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetReceiveFrame(app, ts, self, counted, buffer, length); });
}

/* sub_51cf30: all that came for the reliable layer. */
nfs::u32 frontend::netc::packetReceive(App* app, ThreadState& ts, u32 self)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x24);
    // The arguments where the game's code hands them: the code below reads them there.
    return receiveFrames<0x51cd50, packetReceiveFrame>(guest, ts, self, rely::kBuffer, rely::kBufferSize, rely::kHeld);
}

nfs::u32 frontend::netcPacketReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51cf30", 0, 0, nullptr, 0, kAll };
    const u32 self = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetReceive(app, ts, self); });
}

/* sub_51d000: what the reliable layer's queues keep packets in the order of. */
nfs::u32 frontend::netc::packetQueueKey(App* app, ThreadState& ts, u32 packet)
{
    Held h;
    Guest guest(app, ts);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = packet;
    return guest.dword(h.eax + packet::kNumber);
}

nfs::u32 frontend::netcPacketQueueKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d000", 0, 0, nullptr, 0, kAll };
    const u32 packet = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetQueueKey(app, ts, packet); });
}

/* sub_51d010: a packet's time to resend counted down.  1 when it is up. */
nfs::u32 frontend::netc::packetCountDown(App* app, ThreadState& ts, u32 waiting)
{
    Guest guest(app, ts);
    // The arguments where the game's code hands them: the code below reads them there.
    const u32 left = guest.dword(waiting + packet::kResend);
    if (sign(left) <= 0)
        return 1;
    guest.setDword(waiting + packet::kResend, left - 1);
    return sign(left - 1) <= 0 ? 1 : 0;
}

nfs::u32 frontend::netcPacketCountDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d010", 0, 0, nullptr, 0, kEcx };
    const u32 waiting = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetCountDown(app, ts, waiting); });
}

/* sub_51d040: a packet sent again if its time is up, else on to the next.  The
 * original goes on to sub_51c8f0 by a jump: what returns from there returns from here. */
nfs::u32 frontend::netc::packetResendDue(App* app, ThreadState& ts, u32 waiting, u32 self)
{
    Guest guest(app, ts);
    // The arguments where the game's code hands them: the code below reads them there.
    // Called as C++, the jump a call: sub_51c8f0's frame 4 bytes lower than the game's.
    if (sign(guest.dword(waiting + packet::kResend)) <= 0)
        return guest::direct<0x51c8f0, packetSend>(app, ts, waiting, self);
    return 1;
}

nfs::u32 frontend::netcPacketResendDue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (sign(app->getMemory<u32>(args.eax + packet::kResend)) <= 0)
    {
        return netcPacketSend(app, ts, args);  // the jump to it
    }
    static Site site = { "sub_51d040", 0, 0, nullptr, 0, kAll };
    const u32 waiting = args.eax, self = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetResendDue(app, ts, waiting, self); });
}

/* sub_51d050: the stream says something came: read while there is, what is next
 * in order delivered, and a resend asked of a gap. */
nfs::u32 frontend::netc::packetNotify(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x34);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    const u32 self = guest.dword(connection + link::kLayer);
    const u32 ops = guest.dword(self + layer::kStream);
    if (traceLevel(guest) >= 5)
        traceEvent(guest, h, 0x551010, 0, guest.byte(self + rely::kId));
    guest.setDword(self + rely::kBad, 0);
    if (guest.dword(connection + link::kTimeout) == 0)
        return connection;
    const u32 received = self + rely::kReceivedQueue, unacked = self + rely::kUnacked;
    const u32 receiving = self + rely::kReceiving;
    for (;;)
    {
        u32 more = callHeld(guest, h, guest.dword(ops + stream::kAvailable), args({ connection, ops }));
        if (more != 0)
            more = guest::direct<0x51cf30, packetReceive>(app, ts, self);
        u32 next = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, ts, h, received, guest.dword(self + rely::kNextDeliver));
        while (next != 0)
        {
            if (traceLevel(guest) >= 3)
                trace(guest, h, 0x55101c, next + packet::kNumber, next + packet::kData, guest.byte(next + packet::kLength),
                      0, guest.byte(self + rely::kId));
            // By address: the ebx and edx it leaves are handed to the calls after it.
            if (callHeld(guest, h, kDeliver, args({ connection, next + packet::kData, guest.byte(next + packet::kLength) })) == 0)
            {
                if (traceLevel(guest) >= 3)
                    trace(guest, h, 0x551024, next + packet::kNumber, 0, 0, 1, guest.byte(self + rely::kId));
                loaded<0x50e0a0, frontend::netb::qinsert>(app, ts, h, received, next);
                break;
            }
            const u32 locked = fe::direct<0x50ea20, frontend::menz::fileLockEnter>(app, ts, unacked);
            const u32 number = guest.dword(self + rely::kNextFree);
            h.ecx = (number + 1);
            guest.setDword(self + rely::kNextFree, number + 1);
            guest.setDword(next + packet::kNumber, number);
            guest.setDword(self + rely::kNextDeliver, guest.dword(self + rely::kNextDeliver) + 1);
            fe::direct<0x50ea40, frontend::menz::fileLockLeave>(app, ts, unacked, locked);
            guest.setDword(next + packet::kResend, 0);
            loaded<0x50e0a0, frontend::netb::qinsert>(app, ts, h, receiving, next);
            next = loaded<0x50e800, frontend::wina::queueTakeByPriority>(app, ts, h, received, guest.dword(self + rely::kNextDeliver));
        }
        if (more == 0)
            break;
    }
    u32 gap = 0;
    if (guest.byte(self + rely::kAckSent) == 0)
    {
        const u32 first = loaded<0x50e2d0, frontend::netb::qlowpriority>(app, ts, h, received);
        if (first != 0xffffffff && first != guest.dword(self + rely::kNextDeliver))
            gap = 1;
        guest.setByte(self + rely::kAckSent, 1);
    }
    if (guest.dword(self + rely::kBad) == 0 && gap == 0)
        return 0;
    if (traceLevel(guest) >= 4)
        traceEvent(guest, h, guest.dword(self + rely::kBad) != 0 ? 0x55102c : 0x551034, 1, guest.byte(self + rely::kId));
    const u32 waiting = callHeld(guest, h, kQueueFindIndex, args({ receiving, 0 }));
    if (waiting == 0)
        return 0;
    return guest::direct<0x51caf0, packetSendNak>(app, ts, waiting, self);
}

nfs::u32 frontend::netcPacketNotify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d050", 0, 0, nullptr, 0, kAll };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetNotify(app, ts, connection); });
}

/* sub_51d270: a connection opened on the reliable layer: the record made unless
 * it has one, its packets made and queued, the stream asked, and waited for until
 * it stands or has failed.  1 if it stands; with no connection, the record's size. */
nfs::u32 frontend::netc::packetOpen(App* app, ThreadState& ts, u32 connection, u32 ops, u32 asked, u32 windowBits)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x38);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection; h.edx = ops; h.ebx = asked; h.ecx = windowBits;
    constexpr u32 kFile = 0x55103c, kFunction = 0x55104c;
    u32 wanted = asked;
    h.ebx = (0);
    if (sign(asked) < 6)
    {
        wanted = 6;
    }
    else if (sign(asked) <= 0xfa)
    {
        const u32 limit = guest.word(ops + stream::kMaxFrame);
        h.ebx = (limit);
        if (limit != 0 && asked + packet::kFrameHeader > limit)
        {
            reportError(guest, h, kFile, kFunction, 0x207, 0x5510a4);
            h.ebx = (kFunction);
        }
    }
    else
    {
        reportError(guest, h, kFile, kFunction, 0x205, 0x551064, 0xfa);
    }
    if (connection != 0 && guest.dword(connection + link::kTimeout) != 0)
        reportError(guest, h, kFile, kFunction, 0x20a, 0x5510f4);
    if (sign(windowBits) < 2)
        windowBits = 2;
    else if (sign(windowBits) > 7)
        windowBits = 7;
    h.ecx = (windowBits);
    h.ebx = (wanted);
    const u32 least = guest.byte(ops + stream::kMinFrame);
    if (sign(least) > sign(wanted))
        wanted = least;
    const u32 half = u32(sign(u32(1) << (windowBits & 31)) / 2);
    const u32 bufferSize = guest.byte(ops + stream::kReliable) == 0 ? 0x200 : 0x100;
    const u32 step = (wanted + 0x15) & 0xfffffffc;
    const u32 total = step * (half + half - 1) + rely::kSize + bufferSize;
    h.edx = (total);
    if (connection == 0)
        return total;
    u32 owned = 0;
    u32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
    {
        guest.setDword(kErrorFile, kFile);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x22b);
        owned = 1;
        self = callHeld(guest, h, kAllocate, args({ 0x55112c, total, guest.dword(kMemoryType), 1 }));
        if (self == 0)
        {
            h.edx = (0);
            return 0;
        }
    }
    u32 place = self + rely::kSize;
    guest.setDword(connection + link::kState, 0);
    guest.setDword(connection + link::kTimeout, 0);
    const u32 oldDrop = guest.dword(connection + link::kDrop);
    guest.setDword(connection + link::kLayer, self);
    guest.setDword(connection + link::kDrop, kDropFunction);
    guest.setDword(self + layer::kLink, connection);
    guest.setDword(self + layer::kMaxSize, wanted);
    guest.setDword(self + rely::kIdleLeft, guest.word(connection + link::kRetries));
    guest.setByte(self + layer::kOwned, std::uint8_t(owned));
    guest.setDword(self + layer::kStream, ops);
    const u32 id = guest.dword(kNextPacket);
    guest.setByte(self + rely::kId, std::uint8_t(id));
    guest.setDword(kNextPacket, id + 1);
    guest.setDword(self + rely::kReceived, 0);
    const u32 lock = u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    guest.setDword(self + rely::kNextSend, 1);
    guest.setDword(self + rely::kAcked, 0);
    guest.setDword(self + rely::kLock, lock);
    loaded<0x50dcd0, frontend::netb::qinit>(app, ts, h, self + rely::kFree, 0, 0);
    loaded<0x50dcd0, frontend::netb::qinit>(app, ts, h, self + rely::kUnacked, kQueueKey, self);
    guest.setDword(self + rely::kNextDeliver, 1);
    guest.setDword(self + rely::kNextFree, 1);
    loaded<0x50dcd0, frontend::netb::qinit>(app, ts, h, self + rely::kReceiving, kQueueKey, self);
    loaded<0x50dcd0, frontend::netb::qinit>(app, ts, h, self + rely::kReceivedQueue, kQueueKey, self);
    for (u32 i = half - 1; i != 0; --i)
    {
        const u32 at = place;
        guest.setDword(at, 0);
        guest.setDword(at + packet::kNumber, 0);
        guest.setDword(at + packet::kResend, 0);
        place += step;
        loaded<0x50de50, frontend::netb::qpush>(app, ts, h, self + rely::kFree, at);
    }
    for (u32 i = half - 1; i != 0; --i)
    {
        const u32 at = place;
        guest.setDword(at, 0);
        place += step;
        const u32 number = guest.dword(self + rely::kNextFree);
        h.ebx = (number + 1);
        guest.setDword(self + rely::kNextFree, number + 1);
        guest.setDword(at + packet::kNumber, number);
        guest.setDword(at + packet::kResend, 0);
        loaded<0x50e0a0, frontend::netb::qinsert>(app, ts, h, self + rely::kReceiving, at);
    }
    guest.setDword(self + rely::kHeld, 0);
    guest.setDword(self + rely::kBuffer, place + step);
    guest.setDword(self + rely::kFirstPacket, place);
    guest.setDword(self + rely::kBufferSize, bufferSize);
    if (guest.dword(kConnectionTimeout) == 0)
    {
        reportError(guest, h, kFile, kFunction, 0x276, 0x551134);
        h.ebx = (kFunction);
    }
    h.ecx = (ops);
    u32 failed = 0;
    if (callHeld(guest, h, guest.dword(ops + stream::kOpen), args({ connection, kPacketNotify })) == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, h, 0x551188, 0, guest.byte(self + rely::kId));
    }
    else
    {
        guest.setDword(connection + link::kTimeout, kNeverTimesOut);
        callHeld(guest, h, kQueueAdd, args({ kActiveConnections, connection }));
        if (traceLevel(guest) >= 1)
            traceEvent(guest, h, 0x55115c, 0, guest.byte(self + rely::kId));
        for (;;)
        {
            if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) == 0)
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
                failed = callHeld(guest, h, guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            else if (sign(callHeld(guest, h, kWaitSystemTasks, args({ 1 }))) < 0)
            {
                failed = 1;
            }
            else
            {
                failed = callHeld(guest, h, guest.dword(connection + link::kProcess), args({ connection })) != 0 ? 1 : 0;
            }
            h.edx = (failed);
            if (failed != 0 && traceLevel(guest) >= 2)
                traceEvent(guest, h, 0x551168, 0, guest.byte(self + rely::kId));
            if (guest.dword(connection + link::kTimeout) != 0)
                guest.setDword(connection + link::kTimeout, kNeverTimesOut);
            if (guest.dword(connection + link::kTimeout) == 0)
                break;
            const u32 received = guest.dword(self + rely::kReceived);
            h.edx = (received);
            if (received != 0 || received != failed)
                break;
        }
        if (guest.dword(connection + link::kTimeout) == 0 || failed != 0)
        {
            if (guest.dword(connection + link::kTimeout) == 0 && traceLevel(guest) >= 2)
                traceEvent(guest, h, 0x551178, 0, guest.byte(self + rely::kId));
            guest.setDword(connection + link::kTimeout, 0);
            callHeld(guest, h, guest.dword(ops + stream::kClose), args({ connection, ops }));
        }
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        h.ebx = (failed);
        if (failed == 0)
        {
            guest.setDword(connection + link::kTimeout, guest.dword(kConnectionTimeout));
            guest.setDword(connection + link::kDrop, oldDrop);
            if (traceLevel(guest) >= 1)
                traceEvent(guest, h, 0x551198, 0, guest.byte(self + rely::kId));
            h.edx = (1);
            return 1;
        }
    }
    if (traceLevel(guest) >= 1)
        traceEvent(guest, h, 0x5511ac, 0, guest.byte(self + rely::kId));
    callHeld(guest, h, kQueueRemove, args({ kActiveConnections, connection }));
    callHeld(guest, h, kQueueRemove, args({ kClosedConnections, connection }));
    if (owned != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    h.edx = (0);
    return 0;
}

nfs::u32 frontend::netcPacketOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d270", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 connection = args.eax, ops = args.edx, asked = args.ebx, windowBits = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetOpen(app, ts, connection, ops, asked, windowBits); });
}

/* sub_51d810: data sent on the reliable layer: a free packet taken, numbered, sent
 * and kept until acknowledged.  The port's trace of it first. */
nfs::u32 frontend::netc::packetSubmit(App* app, ThreadState& ts, u32 connection, u32 data, u32 length)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection; h.edx = data; h.ebx = length;
    constexpr u32 kFile = 0x55103c, kFunction = 0x5511c0;
    if (connection == 0 || guest.dword(connection + link::kTimeout) == 0)
        return 0;
    const u32 self = guest.dword(connection + link::kLayer);
    const u32 free = loaded<0x50df60, frontend::netb::qpop>(app, ts, h, self + rely::kFree);
    if (length > guest.dword(self + layer::kMaxSize))
    {
        reportError(guest, h, kFile, kFunction, 0x2ba, 0x5511d4);
        h.ebx = (kFunction);
    }
    if (free == 0)
    {
        if (traceLevel(guest) >= 2)
            trace(guest, h, 0x55125c, 0, data, length, 1, guest.byte(self + rely::kId));
        return 0;
    }
    callHeld(guest, h, kPacketBuild, args({ free, self, data, length }));
    guest::direct<0x51c8f0, packetSend>(app, ts, free, self);
    callHeld(guest, h, kQueueAddSorted, args({ self + rely::kUnacked, free }));
    if (guest.dword(self + rely::kNextSend) == 0)
    {
        reportError(guest, h, kFile, kFunction, 0x2c6, 0x551208);
        h.ebx = (kFunction);
    }
    return 1;
}

nfs::u32 frontend::netcPacketSubmit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (commSendTrace(app, ts, args))
        return args.eax;
    static Site site = { "sub_51d810", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 connection = args.eax, data = args.edx, length = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetSubmit(app, ts, connection, data, length); });
}

/* sub_51d8f0: the reliable layer's turn: counts its timeout down, resends what is
 * due, says it is alive when it has been quiet, closes on timing out.  Always 1. */
nfs::u32 frontend::netc::packetProcess(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    const u32 self = guest.dword(connection + link::kLayer);
    h.edx = (guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 5)
        traceEvent(guest, h, 0x551268, 0, guest.byte(self + rely::kId));
    const u32 left = guest.dword(connection + link::kTimeout) - 1;
    guest.setDword(connection + link::kTimeout, left);
    h.ebx = (left);
    if (left == 0)
    {
        if (traceLevel(guest) >= 3)
            traceEvent(guest, h, 0x551274, 0, guest.byte(self + rely::kId));
        guest.setDword(connection + link::kTimeout, callHeld(guest, h, guest.dword(connection + link::kTimedOut), args({ connection })));
    }
    if (guest.dword(connection + link::kTimeout) != 0)
    {
        const u32 ops = guest.dword(self + layer::kStream);
        if (callHeld(guest, h, guest.dword(ops + stream::kReady), args({ connection, ops })) != 0)
        {
            loaded<0x50e9b0, frontend::netb::qsafeiterate>(app, ts, h, self + rely::kUnacked, kCountDown, self);
            loaded<0x50e8e0, frontend::netb::qiterate>(app, ts, h, self + rely::kUnacked, kResendDue, self);
            const u32 idle = guest.dword(self + rely::kIdleLeft);
            guest.setDword(self + rely::kIdleLeft, idle - 1);
            if (sign(idle) <= 0)
                guest::direct<0x51ca50, packetSendControl>(app, ts, guest.dword(self + rely::kFirstPacket), self, kFrameControlAck);
            return 1;
        }
    }
    if (traceLevel(guest) >= 2)
        trace(guest, h, 0x551280, connection + link::kTimeout, 0, 0, 1, guest.byte(self + rely::kId));
    guest::direct<0x51cc80, packetShutdown>(app, ts, self);
    return 1;
}

nfs::u32 frontend::netcPacketProcess(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d8f0", 0, 0, nullptr, 0, kAll };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetProcess(app, ts, connection); });
}

/* sub_51d9e0: the reliable layer's connection closed by us: the other end told if
 * it can be, the record gone.  1 if it was there. */
nfs::u32 frontend::netc::packetClose(App* app, ThreadState& ts, u32 connection)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = connection;
    if (connection == 0)
        return 0;
    const u32 self = guest.dword(connection + link::kLayer);
    if (self == 0)
        return 0;
    const u32 ops = guest.dword(self + layer::kStream);
    h.ecx = (guest.dword(kTraceLevel));
    if (traceLevel(guest) >= 1)
        traceEvent(guest, h, 0x55128c, 0, guest.byte(self + rely::kId));
    if (callHeld(guest, h, kQueueRemove, args({ kActiveConnections, connection })) == 0)
    {
        if (guest.dword(connection + link::kTimeout) != 0)
            callHeld(guest, h, kMonoPrintf, args({ 0 }, { 0x55129c }));
    }
    else
    {
        if (guest.dword(connection + link::kTimeout) != 0)
        {
            guest.setDword(connection + link::kTimeout, 0);
            u32 tries = 4;
            for (;;)
            {
                --tries;
                h.ecx = (tries);
                if (tries == 0xffffffff
                    || callHeld(guest, h, guest.dword(ops + stream::kReady), args({ connection })) == 0)
                    break;
                // By address: the ebx and edx it leaves are handed to the stream's functions after it.
                if (callHeld(guest, h, kPacketControl, args({ guest.dword(self + rely::kFirstPacket), self, kFrameClose })) != 0)
                    tries = 0;
                else
                    loaded<0x4f06e0, frontend::wata::timerWait>(app, ts, h, 1);
            }
            loaded<0x4f06e0, frontend::wata::timerWait>(app, ts, h, 3);
            callHeld(guest, h, guest.dword(ops + stream::kClose), args({ connection }));
        }
        fe::direct<0x50de50, netb::qpush>(app, ts, kClosedConnections, connection);
    }
    if (callHeld(guest, h, kQueueRemove, args({ kClosedConnections, connection })) == 0)
        return 0;
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, self + rely::kFree);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, self + rely::kUnacked);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, self + rely::kReceiving);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, self + rely::kReceivedQueue);
    u32(*fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(self + rely::kLock)));
    if (guest.byte(self + layer::kOwned) != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, self);
        guest.setDword(connection + link::kLayer, 0);
    }
    return 1;
}

nfs::u32 frontend::netcPacketClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51d9e0", 0, 0, nullptr, 0, kAll };
    const u32 connection = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::packetClose(app, ts, connection); });
}

/* ---------------------------------------------------------------------------
 * win\comm.c: the serial ports, their thread, and a modem's talk
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kModemWait = 0x56aab8, kModemAttentionText = 0x56aabc, kModemTable = 0x56aac0, kModemRowSize = 12;
constexpr u32 kModemResponseText = 0x56abb4, kModemRows = 0x56abb8, kModemConnected = 0x56abbc;
constexpr u32 kModemEcho = 0x56abb0, kOpenPorts = 0x56abc0, kPortsEvent = 0x56abc4, kPortsLock = 0x56abc8;
constexpr u32 kBaudRates = 0x56abcc, kClockTicks = 0x79c1a0, kClockSeconds = 0x79c1a4;
constexpr u32 kModemEchoBuffer = 0xa0b160, kPorts = 0xa0b1a0, kThreadHandle = 0xa0b1e0;
constexpr u32 kModemFailed = 0xfffffffd, kSourceFile = 0x550b9c;

constexpr u32 kCloseAllPorts = 0x51ab40;
constexpr u32 kSerialThreadEntry = 0x51af10;
constexpr u32 kPortWrite = 0x51b5f0;
constexpr u32 kPortRead = 0x51ba50, kWaitIn = 0x51bbd0;

/* A serial port's record, 0x450 bytes, with its two rings of 0x200. */
namespace serial
{
constexpr u32 kCommand = 0x00, kHandle = 0x04, kReading = 0x08, kInFirst = 0x0c, kInCount = 0x10, kInGot = 0x14;
constexpr u32 kInOverlapped = 0x18, kInEvent = 0x28, kInRing = 0x2c, kWriting = 0x22c, kOutFirst = 0x230;
constexpr u32 kOutCount = 0x234, kOutSent = 0x238, kOutOverlapped = 0x23c, kOutEvent = 0x24c, kOutRing = 0x250;
constexpr u32 kRecordSize = 0x450, kRingSize = 0x200, kRingMask = 0x1ff;
constexpr u32 kCommandWake = 1, kCommandDropInput = 2, kCommandDropOutput = 3, kCommandClose = 4;
}

u32 portRecord(const Guest& guest, u32 number)
{
    return guest.dword(kPorts + number * 4);
}

/* The ports' lock, made the first time it is asked for. */
void ensurePortsLock(Guest& guest, nfs::ThreadState& ts)
{
    if (guest.dword(kPortsLock) == 0)
        guest.setDword(kPortsLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(guest.app(), ts)));
}

void enterPorts(Guest& guest, nfs::ThreadState& ts)
{
    fe::enterLock(guest, ts, guest.dword(kPortsLock));
}

u32 leavePorts(Guest& guest, nfs::ThreadState& ts)
{
    return fe::leaveLock(guest, ts, guest.dword(kPortsLock));
}

/* The modem's patience, a quarter of a second, worked out once. */
void ensureModemWait(Guest& guest)
{
    if (guest.dword(kModemWait) == 0)
        guest.setDword(kModemWait, u32(sign(guest.dword(kTicksPerSecond)) / 4));
}

/* A line for the modem's log file. */
void modemLog(Guest& guest, u32 text)
{
    if (guest.dword(kModemLog) != 0)
        static_cast<void>(*fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.ts(), kModemLog, 1u, text, guest::Words({})));
}

/* Time let pass, as a thread or not.  False if the game's tasks say stop. */
bool modemPause(Guest& guest, nfs::ThreadState& ts, u32 ticks)
{
    Held h;
    if (u32(*fe::direct<0x4df930, ea::isThreadNow>(guest.app(), ts, 0)) == 0)
    {
        fe::direct<0x4df8e0, ea::sleepFor>(guest.app(), ts, ticks);
        return true;
    }
    return sign(callHeld(guest, h, kWaitSystemTasks, args({ ticks }))) >= 0;
}

}  // namespace

/* sub_51a470: a modem's answer read a character at a time until one of the table's
 * words is in it or it ends in a number.  Its number; -1 for none, -3 when told to stop. */
nfs::u32 frontend::netc::modemResponse(App* app, ThreadState& ts, u32 number, u32 timeout)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x54);
    // The arguments where the game's code hands them: the code below reads them there.
    const u32 text = guest.frame();
    if (guest.dword(kModemWait) == 0)
    {
        const u32 ticks = guest.dword(kTicksPerSecond);
        ensureModemWait(guest);
    }
    guest.setDword(kModemResponseText, 0);
    const u32 deadline = guest.dword(kClockSeconds) + timeout;
    u32 firstDigit = 0, last = text - 1, count = 0;
    do
    {
        if (sign(guest.dword(kClockSeconds)) >= sign(deadline) || guest.dword(kModemAbort) != 0)
            break;
        if (loaded<0x51bbd0, serialWaitIn>(app, ts, h, number, 1, text + count, 1) != 0)
        {
            ++last;
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
            for (u32 column = 0; column < 2; ++column)
            {
                for (row = sign(guest.dword(kModemRows)) - 1; row >= 0; --row)
                {
                    const u32 entry = kModemTable + u32(row) * kModemRowSize;
                    if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), text, guest.dword(entry + column * 4)) == 0)
                        continue;
                    guest.setDword(kModemConnected, guest.dword(kModemConnected) | (column > 0 ? 1 : 0));
                    if (guest.dword(kModemLog) != 0)
                    {
                        static_cast<void>(*fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.ts(), kModemLog, 1u, 0x550adcu,
                                                                                guest::Words({ text, guest.dword(entry) })));
                    }
                    if (!modemPause(guest, ts, guest.dword(kModemWait)))
                    {
                        guest.setDword(kModemAbort, 1);
                        return kModemFailed;
                    }
                    const u32 word = guest.dword(entry);
                    guest.setDword(kModemResponseText, word);
// as the code loads it for it, which it keeps
                    return fe::direct<0x4f0430, watcom::clib::atoi>(app, ts, guest.dword(entry + 8));
                }
            }
            if (firstDigit != 0 && !digit && guest.dword(kModemConnected) != 0)
            {
                const std::int32_t said = sign(fe::direct<kAtoi, watcom::clib::atoi>(guest.app(), guest.ts(), firstDigit));
                const std::int32_t code = said >= 10 ? 1 : said >= 0 ? said : -1;
                if (guest.dword(kModemLog) != 0 && code >= 0)
                    static_cast<void>(*fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.ts(), kModemLog, 1u, 0x550adcu,
                        guest::Words({ firstDigit, guest.dword(kModemTable + u32(code) * kModemRowSize) })));
                if (code >= 0)
                    guest.setDword(kModemResponseText, guest.dword(kModemTable + u32(row) * kModemRowSize));
                if (!modemPause(guest, ts, guest.dword(kModemWait)))
                {
                    guest.setDword(kModemAbort, 1);
                    return kModemFailed;
                }
                return u32(code);
            }
        }
        u32 stop = 0;
        if (u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0)
            stop = sign(u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0))) < 0 ? 1 : 0;
        else
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
        guest.setDword(kModemAbort, guest.dword(kModemAbort) | stop);
    } while (sign(count) < 0x29);
    return guest.dword(kModemAbort) != 0 ? kModemFailed : 0xffffffff;
}

nfs::u32 frontend::netcModemResponse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51a470", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 number = args.eax, timeout = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::modemResponse(app, ts, number, timeout); });
}

/* sub_51a7b0: a command said to the modem, with or without its input thrown away
 * first, and its echo waited for.  1 if it came. */
nfs::u32 frontend::netc::modemCommand(App* app, ThreadState& ts, u32 number, u32 command, u32 drop)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number; h.edx = command; h.ebx = drop;
    const u32 length = guest.length(command);
    h.edx = (0);
    guest.setDword(kModemEcho, 0);
    if (sign(length) > 0x31)
        return 0;
    if (drop != 0)
        guest::direct<0x51b1e0, serialDropOutput>(app, ts, number);
    const auto stopped = [&]() -> u32 {
        h.ebx = (1);
        guest.setDword(kModemAbort, 1);
        return 0;
    };
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    const bool thread = u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0;
    const std::int32_t ticks = sign(guest.dword(kTicksPerSecond));
    const u32 tenth = u32(ticks / 10);
    h.edx = (u32(ticks % 10));
    if (!thread)
    {
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, tenth);
    }
    else if (sign(callHeld(guest, h, kWaitSystemTasks, args({ tenth }))) < 0)
    {
        return stopped();
    }
    // The game's code hands it edx as it is and the command in ebx too, which it does not read; the call after
    // loads them again.
    guest::direct<0x51b1c0, serialDropInput>(app, ts, number);
    fe::direct<0x51b5f0, frontend::netc::serialWrite>(app, ts, number, length, command);
    fe::direct<0x51b5f0, frontend::netc::serialWrite>(app, ts, number, 1, 0x550b04);
    const u32 start = guest.dword(kClockSeconds);
    const std::int32_t rate = sign(guest.dword(kTicksPerSecond));
    const u32 deadline = start + u32(rate / 5);
    h.edx = (u32(rate % 5));
    u32 got = 0;
    while (sign(guest.dword(kClockSeconds)) < sign(deadline) && guest.dword(kModemEcho) == 0 && guest.dword(kModemAbort) == 0)
    {
        got += fe::direct<0x51bbd0, frontend::netc::serialWaitIn>(app, ts, number, 1, kModemEchoBuffer + got, 1);
        guest.setByte(got + kModemEchoBuffer, 0);
        guest.setDword(kModemEcho, fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), kModemEchoBuffer, command));
    }
    if (guest.dword(kModemLog) != 0)
    {
        const u32 echo = guest.dword(kModemEcho);
        h.ebx = (echo);
        if (echo != 0)
            static_cast<void>(*fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.ts(), kModemLog, 1u, 0x550b08u, guest::Words({ echo })));
        else
            static_cast<void>(*fe::direct<0x501050, ea::reportedAt>(guest.app(), guest.ts(), kModemLog, 1u, 0x550b1cu, guest::Words({})));
    }
    return guest.dword(kModemEcho) != 0 ? 1 : 0;
}

nfs::u32 frontend::netcModemCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51a7b0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 number = args.eax, command = args.edx, drop = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::modemCommand(app, ts, number, command, drop); });
}

/* sub_51a930: the modem's attention got: the escape, its setup words, then AT and
 * what it answers.  1 if it did. */
nfs::u32 frontend::netc::modemAttention(App* app, ThreadState& ts, u32 number)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number;
    const auto stopped = [&]() -> u32 {
        guest.setDword(kModemAbort, 1);
        return 0;
    };
    const auto oneAndAHalf = [&]() -> u32 {
        return u32(sign(guest.dword(kTicksPerSecond) * 3) / 2);
    };
    modemLog(guest, 0x550b3c);
    ensureModemWait(guest);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    {
        const bool thread = u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0;
        if (!thread)
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, oneAndAHalf());
        else if (sign(callHeld(guest, h, kWaitSystemTasks, args({ oneAndAHalf() }))) < 0)
            return stopped();
    }
    guest::direct<0x51b5f0, serialWrite>(app, ts, number, 3, 0x550b58);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    {
        const bool thread = u32(*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0)) != 0;
        if (!thread)
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, oneAndAHalf());
        else if (sign(callHeld(guest, h, kWaitSystemTasks, args({ oneAndAHalf() }))) < 0)
            return stopped();
    }
    const u32 attention = guest.dword(kModemAttentionText);
    guest::direct<0x51b5f0, serialWrite>(app, ts, number, guest.length(attention), attention);
    if (guest.dword(kModemAbort) != 0)
        return stopped();
    if (!modemPause(guest, ts, guest.dword(kModemWait)))
        return stopped();
    // The game's code hands it the command in edx too, which it does not read; the call after loads edx again.
    guest::direct<0x51b1c0, serialDropInput>(app, ts, number);
    bool fine = guest::direct<0x51a7b0, modemCommand>(app, ts, number, 0x550b5c, 0) != 0;
    if (fine)
        fine = guest::direct<0x51a470, modemResponse>(app, ts, number, guest.dword(kTicksPerSecond) * 2) == 0;
    if (!fine)
    {
        modemLog(guest, 0x550b74);
        return 0;
    }
    modemLog(guest, 0x550b60);
    return 1;
}

nfs::u32 frontend::netcModemAttention(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51a930", 0, 0, nullptr, 0, kAll };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::modemAttention(app, ts, number); });
}

/* sub_51ab40: at the game's end, every port still open closed. */
nfs::u32 frontend::netc::serialCloseAll(App* app, ThreadState& ts, u32 handed)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    u32 left = handed;  // what the calls leave in eax: the last one's
    // The game's code hands it the number past it in edx too, which it does not read.
    for (u32 number = 0; number < 0x10 && guest.dword(kOpenPorts) != 0; ++number)
        left = guest::direct<0x51b200, serialClose>(app, ts, number);
    return left;
}

nfs::u32 frontend::netcSerialCloseAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51ab40", 0, 0, nullptr, 0, kEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialCloseAll(app, ts, handed); });
}

/* sub_51ab70: a command left for a port's thread, which is woken. */
nfs::u32 frontend::netc::serialCommand(App* app, ThreadState& ts, u32 number, u32 command)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    enterPorts(guest, ts);
    const u32 record = portRecord(guest, number);
    if (record != 0)
    {
        guest.setDword(record + serial::kCommand, command);
        fe::direct<0x4dfb30, ea::eventSet>(app, ts, guest.dword(kPortsEvent));
    }
    return leavePorts(guest, ts);
}

nfs::u32 frontend::netcSerialCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51ab70", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi) };
    const u32 number = args.eax, command = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialCommand(app, ts, number, command); });
}

/* sub_51abc0: the port's reads started until its ring is full, then its writes until none is left. */
nfs::u32 frontend::netc::serialStartIo(App* app, ThreadState& ts, u32 record)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    enterPorts(guest, ts);
    while (guest.dword(record + serial::kReading) == 0)
    {
        const u32 count = guest.dword(record + serial::kInCount);
        if (sign(count) >= sign(serial::kRingSize))
            break;
        const u32 at = (guest.dword(record + serial::kInFirst) + count) & serial::kRingMask;
        const u32 first = guest.dword(record + serial::kInFirst);
        const u32 top = sign(at) >= sign(first) ? serial::kRingSize : first;
        for (u32 i = 0; i < 4; ++i)
            guest.setDword(record + serial::kInOverlapped + 4 * i, 0);
        // How many came, into the record (left as it is for a handle that is no file).
        u32 got = guest.dword(record + serial::kInGot);
        const bool read = os::files::read(app, ts, guest.dword(record + serial::kHandle),
                                          guest::memory(app, at + record + serial::kInRing), top - at, got);
        guest.setDword(record + serial::kInGot, got);
        if (!read)
        {
            // A read that did not finish is under way: the port gives no error to say otherwise.
            guest.setDword(record + serial::kReading, 1);
            break;
        }
        guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) + guest.dword(record + serial::kInGot));
    }
    while (guest.dword(record + serial::kWriting) == 0)
    {
        const u32 count = guest.dword(record + serial::kOutCount);
        if (sign(count) <= 0)
            break;
        const u32 first = guest.dword(record + serial::kOutFirst);
        const u32 length = sign(count + first) > sign(serial::kRingSize) ? serial::kRingSize - first : count;
        for (u32 i = 0; i < 4; ++i)
            guest.setDword(record + serial::kOutOverlapped + 4 * i, 0);
        // How many went, into the record (left as it is for a handle that is no file).
        u32 sentNow = guest.dword(record + serial::kOutSent);
        const bool written = os::files::write(app, ts, guest.dword(record + serial::kHandle),
                                              guest::memory(app, first + record + serial::kOutRing), length, sentNow);
        guest.setDword(record + serial::kOutSent, sentNow);
        if (!written)
        {
            // A write that did not finish is under way: the port gives no error to say otherwise.
            guest.setDword(record + serial::kWriting, 1);
            break;
        }
        const u32 sent = guest.dword(record + serial::kOutSent);
        guest.setDword(record + serial::kOutCount, guest.dword(record + serial::kOutCount) - sent);
        guest.setDword(record + serial::kOutFirst, (guest.dword(record + serial::kOutFirst) + sent) & serial::kRingMask);
    }
    return leavePorts(guest, ts);
}

nfs::u32 frontend::netcSerialStartIo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51abc0", 0, 0, nullptr, 0, kAll };
    const u32 record = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialStartIo(app, ts, record); });
}

/* sub_51ad70: the port's reads and writes under way looked at, those done taken in. */
nfs::u32 frontend::netc::serialFinishIo(App* app, ThreadState& ts, u32 record)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = record;
    enterPorts(guest, ts);
    if (guest.dword(record + serial::kReading) != 0 && callHeld(guest, h, kPollEvent, args({ guest.dword(record + serial::kInEvent) })) != 0)
    {
        if (const std::optional<u32> bytes = os::serial::finished(app, guest.dword(record + serial::kHandle), true))
        {
            if (*bytes != 0)
                guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) + *bytes);
            guest.setDword(record + serial::kReading, 0);
        }
        else
        {
            // Still under way only by the error that says so, which the port does not give.
            guest.setDword(record + serial::kReading, 0);
        }
    }
    if (guest.dword(record + serial::kWriting) != 0 && callHeld(guest, h, kPollEvent, args({ guest.dword(record + serial::kOutEvent) })) != 0)
    {
        if (const std::optional<u32> bytes = os::serial::finished(app, guest.dword(record + serial::kHandle), false))
        {
            if (*bytes != 0)
            {
                guest.setDword(record + serial::kOutCount, guest.dword(record + serial::kOutCount) - *bytes);
                guest.setDword(record + serial::kOutFirst, (guest.dword(record + serial::kOutFirst) + *bytes) & serial::kRingMask);
            }
            guest.setDword(record + serial::kWriting, 0);
        }
        else
        {
            // Still under way only by the error that says so, which the port does not give.
            guest.setDword(record + serial::kWriting, 0);
        }
    }
    return leavePorts(guest, ts);
}

nfs::u32 frontend::netcSerialFinishIo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51ad70", 0, 0, nullptr, 0, kAll };
    const u32 record = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialFinishIo(app, ts, record); });
}

/* sub_51aeb0: the thread's wait: for its command event and every port's work under way. */
nfs::u32 frontend::netc::serialWaitEvents(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x90);
    const u32 events = guest.frame();
    u32 count = 1;
    guest.setDword(events, guest.dword(kPortsEvent));
    for (u32 number = 0; number < 0x10; ++number)
    {
        const u32 record = portRecord(guest, number);
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
    return callHeld(guest, h, kWaitForAny, args({ count, events }));
}

nfs::u32 frontend::netcSerialWaitEvents(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51aeb0", 0, 0, nullptr, 0, kAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialWaitEvents(app, ts); });
}

/* sub_51af10: the ports' thread: waits, sees to what is done and to the commands
 * left for each port, starts what can be, while any port is open. */
nfs::u32 frontend::netc::serialThread(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    guest.setDword(kPortsEvent, u32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts)));
    while (guest.dword(kOpenPorts) != 0)
    {
        guest::direct<0x51aeb0, serialWaitEvents>(app, ts);
        for (u32 number = 0; number < 0x10; ++number)
        {
            const u32 record = portRecord(guest, number);
            if (record == 0)
                continue;
            guest::direct<0x51ad70, serialFinishIo>(app, ts, record);
            if (guest.dword(record + serial::kCommand) != 0)
            {
                enterPorts(guest, ts);
                const u32 command = guest.dword(record + serial::kCommand);
                if (command == serial::kCommandDropInput)
                {
                    os::serial::drop(app, guest.dword(record + serial::kHandle), true, false);
                    guest::direct<0x51ad70, serialFinishIo>(app, ts, record);
                    guest.setDword(record + serial::kInFirst, 0);
                    guest.setDword(record + serial::kInCount, 0);
                }
                else if (command == serial::kCommandDropOutput)
                {
                    os::serial::drop(app, guest.dword(record + serial::kHandle), false, true);
                    guest::direct<0x51ad70, serialFinishIo>(app, ts, record);
                    guest.setDword(record + serial::kOutFirst, 0);
                    guest.setDword(record + serial::kOutCount, 0);
                }
                else if (command == serial::kCommandClose)
                {
                    os::serial::drop(app, guest.dword(record + serial::kHandle), true, true);
                    guest::direct<0x51ad70, serialFinishIo>(app, ts, record);
                    os::release(app, guest.dword(record + serial::kHandle));
                    directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.ts(), h, guest.dword(record + serial::kInEvent));
                    directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.ts(), h, guest.dword(record + serial::kOutEvent));
                    guest.setDword(kPorts + number * 4, 0);
                }
                guest.setDword(record + serial::kCommand, 0);
                leavePorts(guest, ts);
            }
            if (portRecord(guest, number) != 0)
                guest::direct<0x51abc0, serialStartIo>(app, ts, record);
        }
    }
    const u32 closed = directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.ts(), h, guest.dword(kPortsEvent));
    guest.setDword(kPortsEvent, 0);
    return closed;
}

nfs::u32 frontend::netcSerialThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51af10", 0, 0, nullptr, 0, kAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialThread(app, ts); });
}

/* sub_51b030: whether a port's number is one that is open. */
nfs::u32 frontend::netc::serialIsOpen(App* app, ThreadState& ts, u32 number)
{
    Guest guest(app, ts);
    // The arguments where the game's code hands them: the code below reads them there.
    const std::int32_t index = sign(number);
    return index >= 0 && index < 0x10 && portRecord(guest, u32(index)) != 0 ? 1 : 0;
}

nfs::u32 frontend::netcSerialIsOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b030", 0, 0, nullptr, 0, kAll };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialIsOpen(app, ts, number); });
}

/* sub_51b1c0: what a port has received thrown away, by its thread. */
nfs::u32 frontend::netc::serialDropInput(App* app, ThreadState& ts, u32 number)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) == 0)
        return 0;
    return guest::direct<0x51ab70, serialCommand>(app, ts, number, serial::kCommandDropInput);
}

nfs::u32 frontend::netcSerialDropInput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b1c0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialDropInput(app, ts, number); });
}

/* sub_51b1e0: what a port has yet to send thrown away, by its thread. */
nfs::u32 frontend::netc::serialDropOutput(App* app, ThreadState& ts, u32 number)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // The arguments where the game's code hands them: the code below reads them there.
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) == 0)
        return 0;
    return guest::direct<0x51ab70, serialCommand>(app, ts, number, serial::kCommandDropOutput);
}

nfs::u32 frontend::netcSerialDropOutput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b1e0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEdx) };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialDropOutput(app, ts, number); });
}

/* sub_51b200: a port closed: its thread told and waited for, its record freed;
 * with the last of them the thread's lock goes too. */
nfs::u32 frontend::netc::serialClose(App* app, ThreadState& ts, u32 number)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number;
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0)
    {
        const u32 record = portRecord(guest, number);
        guest::direct<0x51ab70, serialCommand>(app, ts, number, serial::kCommandClose);
        h.eax = leavePorts(guest, ts);
        while (portRecord(guest, number) != 0)
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
        loaded<0x4e1890, ea::purgeBlock>(app, ts, h, record);
        guest.setDword(kOpenPorts, guest.dword(kOpenPorts) - 1);
    }
    else
    {
        h.eax = leavePorts(guest, ts);
    }
    if (guest.dword(kOpenPorts) != 0)
        return h.eax;
    guest::direct<0x51ab70, serialCommand>(app, ts, number, 0);
    const u32 freed = u32(*fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(kPortsLock)));
    guest.setDword(kPortsLock, 0);
    return freed;
}

nfs::u32 frontend::netcSerialClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b200", 0, 0, nullptr, 0, kAll };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialClose(app, ts, number); });
}

/* sub_51b2f0: a port made of a handle already open, in the number asked or, for -1,
 * the last free; with the first of them the thread starts.  Its number, or -1. */
nfs::u32 frontend::netc::serialOpenHandle(App* app, ThreadState& ts, u32 number, u32 handle)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number; h.edx = handle;
    if (number == 0xffffffff)
    {
        number = 0xf;
        const u32 top = guest.dword(kPorts + 0x3c);
        h.edx = (top);
        if (top != 0)
        {
            u32 at = 0x3c, seen;
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
        loaded<0x4f2a78, frontend::eab::atexitJump>(app, ts, h, kCloseAllPorts);
        ensurePortsLock(guest, ts);
        if (callHeld(guest, h, kCreateThread, args({ kSerialThreadEntry, 0, 2, 0xffffffff }, { kThreadHandle })) == 0)
        {
            u32(*fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(kPortsLock)));
            guest.setDword(kPortsLock, 0);
            guest.setDword(kOpenPorts, guest.dword(kOpenPorts) - 1);
            return 0xffffffff;
        }
    }
    guest.setDword(kErrorFile, kSourceFile);
    guest.setDword(kErrorFunction, 0x550ba8);
    guest.setDword(kErrorLine, 0x248);
    const u32 record = got(fe::direct<0x4e1620, ea::reserved>(app, ts, 0x550bbc, serial::kRecordSize, guest.dword(kMemoryType)));
    guest.setDword(record + serial::kReading, 0);
    guest.setDword(record + serial::kHandle, handle);
    guest.setDword(record + serial::kInEvent, os::events::make(app, true, false));
    guest.setDword(record + serial::kInFirst, 0);
    guest.setDword(record + serial::kInCount, 0);
    guest.setDword(record + serial::kWriting, 0);
    const u32 outEvent = os::events::make(app, true, false);
    guest.setDword(record + serial::kOutFirst, 0);
    guest.setDword(record + serial::kOutEvent, outEvent);
    guest.setDword(record + serial::kOutCount, 0);
    // A read gives up after 25 milliseconds in all, whatever it asked for.
    os::serial::setReadTimeout(app, handle, 0x19);
    guest.setDword(kPorts + number * 4, record);
    // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    h.edx = (1);
    guest::direct<0x51ab70, serialCommand>(app, ts, number, 1);
    return number;
}

nfs::u32 frontend::netcSerialOpenHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b2f0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEsi | kEdi) };
    const u32 number = args.eax, handle = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialOpenHandle(app, ts, number, handle); });
}

/* sub_51b480: a port opened by its number.  Its number, -1 if it would not; open already, nothing. */
nfs::u32 frontend::netc::serialOpen(App* app, ThreadState& ts, u32 number)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x20);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number;
    const u32 name = guest.frame();
    if (sign(number) < 0 || sign(number) >= 0x10)
        reportError(guest, h, kSourceFile, 0x550bc8, 0x269, 0x550bd4, number);
    // The system's error cleared here (the port keeps none): eax 0, as the call leaves it.
    h.eax = 0;
    if (portRecord(guest, number) != 0)
        return h.eax;
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, 0x550bf4, guest::Words{number + 1});
    // COMn: a file of that name, if any, in the data's root; -1 for none.
    const u32 handle = os::files::open(app, guest::text(app, name), os::files::Access::readWrite,
                                       os::files::Create::openExisting)
                           .value_or(0xffffffff);
    if (handle == 0xffffffff)
        return handle;
    return guest::direct<0x51b2f0, serialOpenHandle>(app, ts, number, handle);
}

nfs::u32 frontend::netcSerialOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b480", 0, 0, nullptr, 0, kAll };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialOpen(app, ts, number); });
}

/* sub_51b530: data written to a port, if all of it fits.  What went. */
nfs::u32 frontend::netc::serialWriteWhole(App* app, ThreadState& ts, u32 number, u32 length, u32 data)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    u32 went = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0 && sign(guest::direct<0x51b900, serialOutRoom>(app, ts, number)) >= sign(length))
        went = loaded<0x51b5f0, serialWrite>(app, ts, h, number, length, data);
    leavePorts(guest, ts);
    return went;
}

nfs::u32 frontend::netcSerialWriteWhole(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b530", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 number = args.eax, length = args.edx, data = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialWriteWhole(app, ts, number, length, data); });
}

/* sub_51b5f0: as much data as there is room for put in a port's ring, its thread
 * told.  What went. */
nfs::u32 frontend::netc::serialWrite(App* app, ThreadState& ts, u32 number, u32 length, u32 data)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    u32 first = 0, second = 0;
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0)
    {
        const u32 count = guest.dword(portRecord(guest, number) + serial::kOutCount);
        if (sign(count + length) > sign(serial::kRingSize))
            length = serial::kRingSize - count;
        const u32 record = portRecord(guest, number);
        const u32 at = (guest.dword(record + serial::kOutFirst) + guest.dword(record + serial::kOutCount)) & serial::kRingMask;
        first = length;
        if (sign(at + length) > sign(serial::kRingSize))
        {
            first = serial::kRingSize - at;
            second = length - first;
        }
        if (first != 0)
            completed(fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, portRecord(guest, number) + serial::kOutRing + at, data, first));
        if (second != 0)
            completed(fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, portRecord(guest, number) + serial::kOutRing, data + first, second));
        const u32 now = portRecord(guest, number);
        guest.setDword(now + serial::kOutCount, guest.dword(now + serial::kOutCount) + second + first);
        // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    guest::direct<0x51ab70, serialCommand>(app, ts, number, 1);
    }
    leavePorts(guest, ts);
    return second + first;
}

nfs::u32 frontend::netcSerialWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b5f0", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 number = args.eax, length = args.edx, data = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialWrite(app, ts, number, length, data); });
}

/* sub_51b710: a port's speed, parity, stop bits and word's length.  The speed is
 * the table's for a small number. */
nfs::u32 frontend::netc::serialSet(App* app, ThreadState& ts, u32 number, u32 speed, u32 parity, u32 stopBits,
                                     u32 wordBits)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x28);
    // The arguments where the game's code hands them: the code below reads them there.
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) == 0)
    {
        reportError(guest, h, kSourceFile, 0x550c00, 0x356, 0x550c0c);
    }
    else if (std::optional<os::serial::LineSettings> line = os::serial::settings(app, guest.dword(portRecord(guest, number) + serial::kHandle)))
    {
        // Binary, no flow control (what the code clears of the line's flags), and as asked.
        if (sign(speed) <= 0x10)
            speed = guest.dword(kBaudRates + speed * 4);
        line->speed = speed;
        line->wordBits = std::uint8_t(std::uint8_t(wordBits) + 5);
        line->twoStopBits = stopBits == 1;
        line->parity = parity == 1 ? os::serial::Parity::odd : parity == 3 ? os::serial::Parity::even : os::serial::Parity::none;
        os::serial::setSettings(app, guest.dword(portRecord(guest, number) + serial::kHandle), *line);
    }
    return leavePorts(guest, ts);
}

nfs::u32 frontend::netcSerialSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b710", 4, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 number = args.eax, speed = args.edx, parity = args.ebx, stopBits = args.ecx;
    const u32 wordBits = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialSet(app, ts, number, speed, parity, stopBits, wordBits); });
}

/* sub_51b8a0: how much a port has received. */
nfs::u32 frontend::netc::serialInCount(App* app, ThreadState& ts, u32 number)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    u32 count = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0)
        count = guest.dword(portRecord(guest, number) + serial::kInCount);
    leavePorts(guest, ts);
    return count;
}

nfs::u32 frontend::netcSerialInCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b8a0", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEdx) };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialInCount(app, ts, number); });
}

/* sub_51b900: how much room a port has to send. */
nfs::u32 frontend::netc::serialOutRoom(App* app, ThreadState& ts, u32 number)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    u32 room = 0;
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0)
        room = serial::kRingSize - guest.dword(portRecord(guest, number) + serial::kOutCount);
    leavePorts(guest, ts);
    return room;
}

nfs::u32 frontend::netcSerialOutRoom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b900", 0, 0, nullptr, 0, std::uint8_t(kEbx | kEcx | kEdx) };
    const u32 number = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialOutRoom(app, ts, number); });
}

/* sub_51b970: what a port has received copied out, left in the ring.  How much. */
nfs::u32 frontend::netc::serialPeek(App* app, ThreadState& ts, u32 number, u32 length, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    // The arguments where the game's code hands them: the code below reads them there.
    u32 first = 0, second = 0;
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) != 0)
    {
        const u32 available = guest.dword(portRecord(guest, number) + serial::kInCount);
        if (sign(length) > sign(available))
            length = available;
        const u32 record = portRecord(guest, number);
        const u32 at = guest.dword(record + serial::kInFirst);
        first = length;
        if (sign(at + length) > sign(serial::kRingSize))
        {
            second = length - (serial::kRingSize - at);
            first = length - second;
        }
        if (first != 0)
        {
            const u32 now = portRecord(guest, number);
            completed(fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, out, guest.dword(now + serial::kInFirst) + now + serial::kInRing, first));
        }
        if (second != 0)
            completed(fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, out + first, portRecord(guest, number) + serial::kInRing, second));
    }
    leavePorts(guest, ts);
    return second + first;
}

nfs::u32 frontend::netcSerialPeek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51b970", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 number = args.eax, length = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialPeek(app, ts, number, length, out); });
}

/* sub_51ba50: what a port has received taken out of the ring, its thread told.  How much. */
nfs::u32 frontend::netc::serialRead(App* app, ThreadState& ts, u32 number, u32 length, u32 out)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    ensurePortsLock(guest, ts);
    enterPorts(guest, ts);
    const u32 got = loaded<0x51b970, serialPeek>(app, ts, h, number, length, out);
    if (got != 0)
    {
        const u32 record = portRecord(guest, number);
        guest.setDword(record + serial::kInCount, guest.dword(record + serial::kInCount) - got);
        const u32 now = portRecord(guest, number);
        guest.setDword(now + serial::kInFirst, (got + guest.dword(now + serial::kInFirst)) & serial::kRingMask);
        // edx as the call by address loads it, which sub_51ab70 keeps: the function leaves it.
    guest::direct<0x51ab70, serialCommand>(app, ts, number, 1);
    }
    leavePorts(guest, ts);
    return got;
}

nfs::u32 frontend::netcSerialRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51ba50", 0, 0, nullptr, 0, std::uint8_t(kEcx | kEsi | kEdi) };
    const u32 number = args.eax, length = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialRead(app, ts, number, length, out); });
}

/* sub_51bbd0: what is wanted from a port read once it is there or the time is up.  How much. */
nfs::u32 frontend::netc::serialWaitIn(App* app, ThreadState& ts, u32 number, u32 length, u32 out, u32 timeout)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    // The arguments where the game's code hands them: the code below reads them there.
    h.eax = number; h.edx = length; h.ebx = out; h.ecx = timeout;
    h.ebx = (0);
    if (sign(length) > sign(serial::kRingSize))
        reportError(guest, h, kSourceFile, 0x550c58, 0x471, 0x550c68, serial::kRingSize);
    if (guest::direct<0x51b030, serialIsOpen>(app, ts, number) == 0)
        return 0;
    const u32 deadline = guest.dword(kClockTicks) + timeout;
    h.ecx = (number * 4);
    while (sign(length) > sign(guest.dword(portRecord(guest, number) + serial::kInCount))
           && sign(deadline) > sign(guest.dword(kClockTicks)))
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    const u32 got = callHeld(guest, h, kPortRead, args({ number, length, out }));
    h.ebx = (got);
    return got;
}

nfs::u32 frontend::netcSerialWaitIn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_51bbd0", 0, 0, nullptr, 0, std::uint8_t(kEsi | kEdi) };
    const u32 number = args.eax, length = args.edx, out = args.ebx, timeout = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return netc::serialWaitIn(app, ts, number, length, out, timeout); });
}

}  // namespace nfs3hp
