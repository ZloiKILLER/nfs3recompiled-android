#include "native_frontend_neta.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_frontend_connect.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_loada.h"
#include "native_frontend_netb.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_load_cardb.h"
#include "os_window.h"
#include "port.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_fields.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include <game_float.h>
#include "os_system.h"
#include "os_net.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 netRaceQuitVote(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netLobbyChanged(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNpsStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netBarrier(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRaceSettings(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendSetup(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netExchangeCars(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netHostLobby(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netHostStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxReceive(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendMyCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPlayerControls(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPlayersToRace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCompareVersions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netInitService(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendCarFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netAllReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netInputPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServicePoll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxSend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxRecvFrom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netQuitPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSetupPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceOpenGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRasLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceCloseGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPlayerPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRasPhoneBook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendRaceResults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendSettings(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netTakeInputs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNpsMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSessionReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCarFileToHost(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCarFileToClient(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netJoinRaceNet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netBarrierPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netGetServices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRasAvailable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNpsStop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSameGuid(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netFindGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNetworkThere(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netClearRing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxAnswerName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNextPending(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netLobbyClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netEndpointOfLink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRaceLoadStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSettingsPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netMarkPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netLobbyOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netKillService(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netClientLobby(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPacketMatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netEndpointLost(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxStartup(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPendingAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netStatusPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServicesReady(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSameNode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netDropGone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRasLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPortToSockaddr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netGonePacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netToSockaddr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netEndpointTimedOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netResultsPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPackControls(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netFromSockaddr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netHostRaceNet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netTakeInputsOnTick(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netDispatchPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendPlayerGone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRasFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netHostGone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSendQueued(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netMakeLocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNpsReceived(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netWinsockClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxOwnAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netGetGames(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netSetGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netNewGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netOpenGame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServicePollCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceRoom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netServiceAsk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxSetName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netRaceMusicStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPendingEmpty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxPort(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netClearInputCounts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netUnpackControls(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPollServices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netGameListed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCarChunkToHost(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCarChunkToClient(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netEndpointHandlers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netVersionPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPlayerLeftPacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netCloseServices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netPollOne(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netThisLink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxForgetName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netAnswerYes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netAnswerNo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netIpxJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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


/* The network, decompiled: what the game says over it and how it gets there.
 *
 *   the session      the host's and the machines' game setup, who is in it, the barrier
 *                    every machine waits at (sub_4a6a10), the game's version compared, the
 *                    cars' files sent to whoever lacks them;
 *   the packets      a function per message kind, in the table sub_4a6440 fills, each
 *                    reading its fields in the sender's order;
 *   a race's inputs  the controls of each machine against the frame they are for;
 *   the services     NET_ and its services' table: Winsock IPX with its own socket, thread
 *                    and queues, RaceNet's NPS through the DLL's own pointers.
 *
 * All of the game's state stays in guest memory.  Every field a packet carries sits where
 * the original puts it, at the size it puts it: two phones stay in step only while their
 * bytes agree.  What each function leaves in eax and in the registers it does not save is
 * kept.  Windows' functions are the port's own, called as C++; the network is the port's
 * (os_net.h).
 *
 * Plan step 5: each function a typed core in frontend::neta (native_frontend_neta.h), in a frame of
 * its own where the game's function has one (fe::Frame), and a thunk under the game's name for
 * the calls by address.  A core calls the cores of this file and of native_frontend_netb.cpp as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game still
 * by address, and so does a call whose callee leaves in a register what the caller goes on with or
 * leaves at its own exit. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

// The port's trace, native_vertices.cpp.
bool commOpenTrace(nfs::App* app, nfs::ThreadState& ts);

namespace
{

using fe::args;
using fe::Guest;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;

using App = nfs::App;
using ThreadState = nfs::ThreadState;

/* ---------------------------------------------------------------------------
 * What these functions share
 * ------------------------------------------------------------------------- */

// The game's clocks.
constexpr u32 kClock = 0x79c1a0, kTicks = 0x79c1a4;

// The session.  The words the original reads as `dword [a - 2] >> 16` are named at a.
constexpr u32 kPacketSetup = 0x7a2214;     // the setup packet, 0x7c bytes: its kind first
constexpr u32 kSetupNames = 0x7a2215;      // 11 bytes a machine
constexpr u32 kSetupLinks = 0x7a226d;      // a byte a machine
constexpr u32 kMachineCount = 0x7a2276;    // int16
constexpr u32 kThisMachine = 0x7a2278;     // int16, then the rest of the setup's machine numbers
constexpr u32 kMachineNumbers = 0x7a2278;  // int16[9]
constexpr u32 kSetupOther = 0x7a228c;
constexpr u32 kSetupDirty = 0x7a2296;      // int16
constexpr u32 kSetupNew = 0x7a2298;        // int16
constexpr u32 kPlayersNew = 0x7a229a;      // int16
constexpr u32 kHostBarrier = 0x7a229c;     // int16: the barrier the host has reached, for a client
constexpr u32 kMachineLinks = 0x7a21f0;    // endpoint*[8]
constexpr u32 kMachineBarriers = 0x7a22ba; // int16[8]: the barrier each machine has reached
constexpr u32 kMachineGone = 0x7a22ca;     // int16[8]
constexpr u32 kIsHost = 0x7a22da;          // int16
constexpr u32 kSessionOver = 0x7a22dc;     // int16
constexpr u32 kSessionFailed = 0x7a22de;   // int16
constexpr u32 kSessionStage = 0x7a22e0;
constexpr u32 kLinkKind = 0x7a22e4;        // 2 for RaceNet
constexpr u32 kEndpoints = 0x7a22e8;       // 0x6c bytes each
constexpr u32 kEndpointSize = 0x6c;
constexpr u32 kEndpointOffered = 0x7a22f8;
constexpr u32 kEndpointUsed = 0x7a230c;
constexpr u32 kEndpointLink = 0x7a231c;
constexpr u32 kEndpointNames = 0x7a25dc;   // 11 bytes each
constexpr u32 kNameSize = 11;

// The locks' handles.
constexpr u32 kBarrierLock = 0x55ecb8, kPlayersLock = 0x55ecb4, kSetupLock = 0x55e5b8;
constexpr u32 kInputLock = 0x560114;

// Imports, by their slots.

// The game's functions, by address.
constexpr u32 kError = 0x401010;
constexpr u32 kClearMessages = 0x42d800, kOpenMessages = 0x42d790;
constexpr u32 kFindCarById = 0x439990;
constexpr u32 kBeginCarSend = 0x49b800;
constexpr u32 kDropMachine = 0x49ecb0, kAnnounceMachineLeft = 0x49ecf0;
constexpr u32 kCloseNetwork = 0x4a65e0, kPlayerMachine = 0x4a6690;
constexpr u32 kCreateThread = 0x4df7a0;
constexpr u32 kMemset = 0x4e0640, kFillZero = 0x4e070c, kStrncpy = 0x4e0e30;
constexpr u32 kRunSystemTasks = 0x4e7630;
constexpr u32 kMemmove = 0x4ea4f0;
constexpr u32 kStrcmp = 0x4ee310, kToUpper = 0x4eede0;

// This group's own, called where they are.
constexpr u32 kNpsDestroyed = 0x4a5e50, kNpsReceived = 0x4a5ef0;
constexpr u32 kIpxThread = 0x4f5b10;

// How much the network's library logs: 4 and above, its events.
constexpr u32 kLogLevel = 0x566de0;
constexpr s32 kEnoughLog = 4;

u32 signedWord(const Guest& guest, u32 address)
{
    return u32(s32(std::int16_t(guest.word(address))));
}

u32 widened(u32 value)
{
    return u32(s32(std::int16_t(value)));
}

/* rep movsd forwards, then rep movsb for the bytes left: the inlined memcpy. */
void copyBytes(Guest& guest, u32 to, u32 from, u32 count)
{
    u32 at = 0;
    for (u32 i = 0; i < count >> 2; ++i, at += 4)
        guest.setDword(to + at, guest.dword(from + at));
    for (u32 i = 0; i < (count & 3); ++i, ++at)
        guest.setByte(to + at, guest.byte(from + at));
}

/* Why a network call failed, as the game's log shows it. */
u32 errorNumber(os::net::Error error)
{
    return u32(error);
}

void enter(Guest& guest, ThreadState& ts, u32 lock)
{
    fe::enterLock(guest, ts, guest.dword(lock));
}

u32 leave(Guest& guest, ThreadState& ts, u32 lock)
{
    return fe::leaveLock(guest, ts, guest.dword(lock));
}

/* sub_4a60e0 called as C++ for a caller whose exit shows what it leaves: the packet sent, edx and
 * ebx left as its code leaves them (ea::Regs). */
u32 sendLeaving(App* app, ThreadState& ts, u32 connection, u32 packet, u32 length)
{
    const std::optional<ea::Regs> sent = fe::direct<0x4a60e0, ea::packetSent>(app, ts, connection, packet, length);
    return sent->eax;
}

/* The record every machine's player has at 0x79f5f0. */
constexpr u32 kPlayerRecords = 0x79f5f0, kPlayerRecordSize = 0x60;

/* The byte fields of the host's settings, as the setup packet holds them, and the
 * settings they are. */
struct SettingField
{
    u32 packed, setting;
};
constexpr SettingField kSettingFields[] = {
    { 0x79f8f1, 0x6fd3b8 }, { 0x79f8f2, 0x6fd4ac }, { 0x79f8f3, 0x6fd504 }, { 0x79f8f4, 0x6fd4b4 },
    { 0x79f8f5, 0x6fd4b0 }, { 0x79f8f6, 0x6fd4d4 }, { 0x79f908, 0x6fd4d0 }, { 0x79f909, 0x6fd4e8 },
    { 0x79f90a, 0x6fd4c4 }, { 0x79f90b, 0x6fd4cc }, { 0x79f90c, 0x6fd4c8 }, { 0x79f90d, 0x6fd4c0 },
};
constexpr u32 kSettingTrackPacked = 0x79f910, kSettingTrack = 0x6fd3b4;
constexpr u32 kSettingLastPacked = 0x79f914, kSettingLast = 0x6fd470;

// The player's own settings.
constexpr u32 kPlayerCar = 0x6fd2bc, kPlayerColour = 0x6fd2d4, kPlayerName = 0x6fd2fc, kPlayerColours = 0x6fd307;
constexpr u32 kPlayerTransmission = 0x6fd508;

// The player's record about to go out, a copy of which is kept in 0x79f5f0.
constexpr u32 kMyRecord = 0x79f918;

// The race's players, in the settings block: 0x6c bytes each.
constexpr u32 kRacePlayers = 0x6fd52c, kRacePlayerSize = 0x6c;
constexpr u32 kRacePlayerCount = 0x6fd514;

// The cars in a race and the flag sub_4c5720 raises on each.
constexpr u32 kCarCount = 0x5efda4, kCars = 0x5efa48, kCarsByPlayer = 0x5efac8;

void markCarsLeaving(Guest& guest)
{
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarCount)); ++i)
        guest.setDword(guest.dword(kCars + i * 4) + 0x2c4, 1);
}

}  // namespace

/* ---------------------------------------------------------------------------
 * The session
 * ------------------------------------------------------------------------- */

/* sub_4c5720, each frame of a network race: whether this machine leaves the race, and how.
 * eax what the race asks: 1 a replay, 2 on, 3 quit, 4 restart, 5 the session is lost.  With
 * fewer than two local players only.  A key held repeats no more than the delay allows; the
 * host looks at what each machine has said and answers with the barrier that matches, and a
 * client follows what the host has said or says what it wants. */
u32 frontend::neta::raceQuitVote(App* app, ThreadState& ts, u32 vote)
{
    Held h;
    h.eax = vote;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x30);
    constexpr u32 kPlayers = 0x6fd3b0, kSplitCount = 0x6fd3a0, kKeyRepeat = 0x7d36dc, kLastKey = 0x7d36e0;
    constexpr u32 kHostQuit = 0x7d36c4, kRaceQuits = 0x56010c;
    constexpr u32 kWaitSeconds = 20;
    constexpr u32 kAgreed = 9, kGoOn = 10, kQuit = 11, kRestart = 0x20;
    const u32 players = guest.dword(kPlayers);
    if (players == 0 || players == 1 || s32(guest.dword(kSplitCount)) > 1)
        return vote;

    const u32 ticks = guest.dword(kTicks);
    // The game's code hands it the ticks in eax, which it does not read.
    u32 key = fe::direct<0x4cc970, frontend::glue::nextKey>(app, ts) & 0xffff;
    if (s32(key) < 0xff)
        key = fe::direct<kToUpper, watcom::clib::toupper>(guest.app(), guest.ts(), key);
    const bool sameKey = key == guest.dword(kLastKey);
    if (key != 0 && (!sameKey || s32(ticks) > s32(guest.dword(kKeyRepeat))))
        guest.setDword(kKeyRepeat, ticks + (sameKey ? 5 : 15));
    else
        guest.setDword(kKeyRepeat, 0);
    guest.setDword(kLastKey, key);

    const auto answerGoOn = [&]() -> u32 {
        guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kGoOn);
        callHeld(guest, h, kClearMessages);
        return 2;
    };
    const auto answerQuit = [&]() -> u32 {
        markCarsLeaving(guest);
        guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kQuit);
        callHeld(guest, h, kClearMessages);
        return 3;
    };
    const auto answerRestart = [&]() -> u32 {
        markCarsLeaving(guest);
        guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kRestart);
        callHeld(guest, h, kClearMessages);
        return 4;
    };

    if (guest.word(kIsHost) != 0)
    {
        // What each machine has said to the host: 11 is that it is going.
        for (u32 machine = 1; s32(machine) < s32(guest.dword(kRacePlayerCount)); ++machine)
        {
            if (guest.dword(kHostQuit) != 0)
            {
                vote = 1;
                break;
            }
            const u32 barrier = kMachineBarriers + machine * 2;
            const u16 said = guest.word(barrier);
            if (said == 0)
                continue;
            const bool going = said == kQuit;
            if (going)
            {
                if (guest.dword(kLinkKind) != 0)
                    directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.ts(), h, machine);
                else
                    vote = 3;
            }
            enter(guest, ts, kBarrierLock);
            if ((guest.word(barrier) == kQuit) == going)
                guest.setWord(barrier, 0);
            leave(guest, ts, kBarrierLock);
        }
        switch (vote)
        {
        case 1:
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kAgreed);
            guest.setDword(kRaceQuits, 0);
            guest.setDword(kHostQuit, 0);
            callHeld(guest, h, kClearMessages);
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, 1);
            return 1;
        case 2:
            return answerGoOn();
        case 3:
            return answerQuit();
        case 4:
            return answerRestart();
        case 5:
            markCarsLeaving(guest);
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kQuit);
            callHeld(guest, h, kClearMessages);
            guest.setWord(kSessionOver, 1);
            return 5;
        default:
            return 0;
        }
    }

    if (vote != 5)
    {
        switch (u32(guest.word(kHostBarrier)))
        {
        case kAgreed:
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, kAgreed);
            guest.setDword(kRaceQuits, 0);
            callHeld(guest, h, kClearMessages);
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, 1);
            return 1;
        case kGoOn:
            return answerGoOn();
        case kQuit:
            return answerQuit();
        case kRestart:
            return answerRestart();
        default:
            break;
        }
    }

    // Message 8 to the host, what is wanted.
    constexpr u32 kQuitMessage = 8, kMessageBytes = 12;
    if (vote == 1)
    {
        const u32 message = guest.frame();
        guest.setByte(message, kQuitMessage);
        guest.setDword(message + 4, vote);
        fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, message, kMessageBytes);
        return 0;
    }
    if (vote == 3 || vote == 4 || vote == 5)
    {
        const u32 message = guest.frame() + 0xc;
        guest.setByte(message, kQuitMessage);
        guest.setDword(message + 4, 3);
        fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, message, kMessageBytes);
        if (guest.dword(kLinkKind) != 0 || vote == 5)
        {
            guest.setWord(kSessionFailed, 1);
            guest.setWord(kSessionOver, 1);
            return vote;
        }
    }
    return 0;
}

nfs::u32 frontend::netRaceQuitVote(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5720", 0, 0, nullptr, 0 };
    const u32 vote = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::raceQuitVote(app, ts, vote); });
}

/* sub_49a520, each pass of a network lobby: 1 when anything has changed that the lobby's
 * screen shows.  This machine's car against the copy the others were last sent, for the
 * host its settings too, what came in; a client takes on the host's settings here. */
u32 frontend::neta::lobbyChanged(App* app, ThreadState& ts)
{
    return lobbyChangedFrom(app, ts, 0);
}

u32 frontend::neta::lobbyChangedFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    constexpr u32 kPlayersChanged = 0x7a22a8;
    const u32 joined = guest::direct<0x49eae0, hostLobby>(app, ts);
    const u32 colours = guest.frame() + 4;
    const u32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, signedWord(guest, kThisMachine)));
    u32 changed = 0;
    if (machine != u32(-1))
    {
        fe::direct<0x472360, port::fields::customColoursRefreshed>(app, ts, ts.sp - 4, h.eax);
        for (u32 i = 0; i < 4; ++i)
            guest.setByte(colours + i, guest.byte(kPlayerColours + i));
        const u32 record = fe::direct<0x439a50, load::recordAt>(app, ts, guest.dword(kPlayerCar));
        const u32 mine = machine * kPlayerRecordSize;
        const bool same = guest.byte(0x79f92c) == guest.dword(kPlayerCar)
            && guest.byte(0x79f92d) == guest.dword(kPlayerColour)
            && guest.byte(0x79f920) == guest.dword(kPlayerTransmission)
            && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), 0x79f921, kPlayerName) == 0
            && joined == 0
            && guest.byte(mine + 0x79f604) == guest.dword(kPlayerCar)
            && guest.byte(mine + 0x79f605) == guest.dword(kPlayerColour)
            && guest.byte(mine + 0x79f5f8) == guest.dword(kPlayerTransmission)
            && guest.dword(colours) == guest.dword(0x79f961)
            && guest.dword(0x79f968) == guest.dword(record + 0x380);
        if (!same)
        {
            guest::direct<0x49a8e0, sendMyCar>(app, ts);
            changed = 1;
        }
    }
    if (guest.word(kIsHost) != 0)
    {
        bool same = true;
        for (const SettingField& field : kSettingFields)
            same = same && guest.byte(field.packed) == guest.dword(field.setting);
        same = same && guest.dword(kSettingTrackPacked) == guest.dword(kSettingTrack)
            && guest.byte(kSettingLastPacked) == guest.dword(kSettingLast) && joined == 0;
        if (!same)
        {
            guest::direct<0x49aa80, sendSettings>(app, ts);
            changed = 1;
        }
    }
    if (guest.word(kPlayersChanged) != 0)
    {
        guest.setWord(kPlayersChanged, 0);
        changed = 1;
    }
    if (guest.word(kPlayersNew) != 0)
    {
        enter(guest, ts, kPlayersLock);
        guest.setWord(kPlayersNew, 0);
        leave(guest, ts, kPlayersLock);
        changed = 1;
    }
    if (guest.word(kIsHost) == 0 && guest.word(kSetupNew) != 0)
    {
        enter(guest, ts, kSetupLock);
        guest.setWord(kSetupNew, 0);
        leave(guest, ts, kSetupLock);
        for (const SettingField& field : kSettingFields)
            guest.setDword(field.setting, guest.byte(field.packed));
        guest.setDword(kSettingTrack, guest.dword(kSettingTrackPacked));
        guest.setDword(kSettingLast, guest.byte(kSettingLastPacked));
        // A car of the machines' own: found again by its id, past the four colours' places.
        constexpr u32 kOpponentCar = 0x6fd4d4;
        if (s32(guest.dword(kOpponentCar)) >= 4)
            guest.setDword(kOpponentCar, fe::direct<0x439990, load::placeById>(app, ts, 0x79f8f8) + 4);
        changed = 1;
    }
    return changed;
}

nfs::u32 frontend::netLobbyChanged(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49a520", 0, 0, nullptr, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::lobbyChangedFrom(app, ts, handed); });
}

/* sub_4f5c10: opens a game's IPX socket and its thread, eax the port, edx a flag and ebx what
 * goes with it.  The state, 0x140a8 bytes, then the free list of 256 packets of 0x140 bytes,
 * the socket with broadcast and big buffers, bound and asked its own address; the thread
 * clears 0x9f3650 when it runs.  0 when anything fails. */
u32 frontend::neta::ipxOpen(App* app, ThreadState& ts, u32 portNumber, u32 flagValue, u32 withFlag)
{
    Held h;
    h.eax = portNumber; h.edx = flagValue; h.ebx = withFlag;
    commOpenTrace(app, ts);
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x38);
    constexpr u32 kThreadStarting = 0x9f3650;
    constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
    constexpr u32 kStateBytes = 0x140a8, kPacketBytes = 0x140, kPacketsBytes = 0x14000;
    const u32 frame = guest.frame();
    const u32 name = frame, port = frame + 0x10;
    guest.setDword(port, portNumber);
    guest.setDword(frame + 0x24, flagValue);
    guest.setDword(frame + 0x20, withFlag);
    guest.setDword(kWhereFile, 0x54d21c);
    guest.setDword(kWhereLine, 0x10a);
    guest.setDword(kWhereFunction, 0x54d228);

    const u32 state = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54d238, kStateBytes, guest.dword(0x5643f4)));
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), state, kStateBytes);
    const u32 freeQueue = state + 0x70;
    guest::direct<0x50dcd0, netb::qinit>(app, ts, freeQueue, 0, 0);
    guest::direct<0x50dcd0, netb::qinit>(app, ts, state + 0x8c, 0, 0);
    const u32 packets = state + 0xa8;
    for (u32 packet = packets; packet != packets + kPacketsBytes; packet += kPacketBytes)
        guest::direct<0x50de50, netb::qpush>(app, ts, freeQueue, packet);
    h.ebx = (freeQueue);  // as the calls by address left it: the queue went in ebx too

    const u32 socket = os::net::open(app, os::net::Kind::ipx).value_or(os::net::kNoSocket);
    guest.setDword(state + 0x20, 1);
    guest.setDword(state + 0x58, socket);
    const u32 flag = guest.dword(frame + 0x24);
    guest.setDword(state + 4, flag);
    if (flag != 0)
        guest.setDword(state, guest.dword(frame + 0x20));
    if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d228, port, state + 0x58, 4, 1, 0);

    u32 thread = 0;
    if (guest.dword(state + 0x58) != os::net::kNoSocket)
    {
        h.ebx = (guest.dword(state + 0x58));
        os::net::setBroadcast(app, guest.dword(state + 0x58), true);
        // IPX's own receiving of broadcasts the code asks off: the port's IPX has no such switch.
        // The socket waits: the receive thread sits in it until a packet comes.
        os::net::setWaits(app, guest.dword(state + 0x58), true);

        const u32 hostPort = guest.word(port);
        h.eax = hostPort;  // as the code has it at the call
        if (os::net::bind(app, guest.dword(state + 0x58), os::net::Endpoint::ipx(0, u16(hostPort)))
            == os::net::Error::none)
        {
            // Its own address, as the system gave it, into the frame for sub_4f5880.
            if (const auto own = os::net::localName(app, guest.dword(state + 0x58)))
            {
                os::net::putIpx(app, name, *own);
                guest::direct<0x4f5880, fromSockaddr>(app, ts, name, state + 0x5c);
                const u32 boundPort = guest.dword(name + 0xc);
                guest.setDword(kThreadStarting, state);
                guest.setWord(state + 0x24, u16(boundPort));
                thread = callHeld(guest, h, kCreateThread, args({ kIpxThread, 0, 2 }, { state + 8 }));
                while (thread != 0 && guest.dword(kThreadStarting) != 0)
                    fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
            }
        }
    }
    if (thread != 0)
        return state;

    if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d240, 0, 0, 0, 1, 0);
    const u32 openSocket = guest.dword(state + 0x58);
    const os::net::Error closed =
        openSocket != os::net::kNoSocket ? os::net::close(app, ts, openSocket) : os::net::Error::none;
    if (closed != os::net::Error::none)
    {
        const u32 level = guest.dword(kLogLevel);
        h.edx = (level);
        guest.setDword(frame + 0x1c, errorNumber(closed));
        if (s32(level) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d24c, frame + 0x1c, 0, 0, 1, 0);
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, state);
    return 0;
}

nfs::u32 frontend::netIpxOpen(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5c10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 portNumber = args.eax, flagValue = args.edx, withFlag = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxOpen(app, ts, portNumber, flagValue, withFlag); });
}

/* sub_4a5b00: RaceNet's NPS opened and a game looked for: eax the player's name or 0, edx the
 * port, ebx the server and ecx its port.  Both calls through the DLL's pointers. */
u32 frontend::neta::npsStart(App* app, ThreadState& ts, u32 name, u32 localPort, u32 server, u32 remotePort)
{
    Held h;
    h.eax = name; h.edx = localPort; h.ebx = server; h.ecx = remotePort;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x7cc);
    constexpr u32 kNpsOpen = 0x7a21e4, kNpsAttach = 0x7a21dc;
    constexpr u32 kNpsState = 0x7a20a0, kNpsName = 0x7a20a4, kNpsSameMachine = 0x7a2104, kNpsAddressBytes = 0x40;
    constexpr u32 kNpsSession = 0x55ecb0, kNpsVersion = 0x7a215c;
    const u32 frame = guest.frame();
    const u32 port = frame + 0x7bc, serverPort = frame + 0x7b8;
    guest.setDword(port, localPort);
    guest.setDword(serverPort, remotePort);
    const u32 loaded = fe::direct<0x4a59c0, frontend::wata::npsEnsureDll>(app, ts, 0);
    if (loaded == 0)
        return loaded;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kNpsState, 0xbc);
    if (name == 0)
        name = 0x53dba4;
    guest.copyString(kNpsName, name);
    const u16 portNumber = guest.word(port);
    guest.setDword(kNpsState, portNumber);
    guest.setByte(kNpsSameMachine, portNumber > 1 ? 0 : 1);

    // What the NPS library is opened with.
    const u32 who = frame + 0x340;
    guest.copyString(who, kNpsName);
    guest.setDword(who + 0x20, guest.dword(kNpsState));
    copyBytes(guest, who + 0x24, kNpsSameMachine, kNpsAddressBytes);
    const u32 callbacks = frame + 0x5a4;
    guest.setDword(callbacks, 0);
    guest.setDword(callbacks + 8, 0);
    guest.setDword(callbacks + 0x10, 0);
    const u32 serverPortValue = guest.dword(serverPort);
    h.edx = (serverPortValue);
    h.ecx = (kNpsReceived);
    guest.setDword(callbacks + 4, kNpsReceived);
    guest.setDword(callbacks + 0xc, kNpsDestroyed);
    const u32 session = callHeld(guest, h, guest.dword(kNpsOpen), args({}, { server, serverPortValue, who, callbacks, 10 }));
    guest.setDword(kNpsSession, session);
    if (s32(session) < 0)
        return 0;

    // What the game's listing says about this game.
    const u32 listing = frame;
    guest.setDword(kNpsVersion, 1);
    guest.setDword(listing, 1);
    guest.copyString(listing + 4, 0x53dbac);
    guest.setDword(listing + 0x24, 0x21);
    guest.copyString(listing + 0x2a, 0x53dbb4);
    guest.setDword(listing + 0x34, 0x29a);
    guest.setDword(listing + 0x38, 0);
    fe::direct<0x4e0640, watcom::clib::memset>(app, ts, listing + 0x3c, 0u, 0x100u);
    h.ebx = (0x100);  // as the code loads it for it, which it keeps
    guest.copyString(listing + 0x3c, 0x53dbc0);
    const u32 version = guest.dword(kNpsVersion);
    h.edx = (version);
    h.ecx = (guest.dword(kNpsSession));
    if (s32(callHeld(guest, h, guest.dword(kNpsAttach), args({}, { guest.dword(kNpsSession), 0x21, version, listing, 0 }))) < 0)
    {
        guest::direct<0x4a5dd0, npsStop>(app, ts);
        return 0;
    }
    return 1;
}

nfs::u32 frontend::netNpsStart(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5b00", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 name = args.eax, localPort = args.edx, server = args.ebx, remotePort = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::npsStart(app, ts, name, localPort, server, remotePort); });
}

/* sub_4a6a10: the barrier, every machine stopped until all have come to the same point.
 * eax the seconds to give up after (0 never, 128 ticks a second), edx which point: the host
 * says so and sees each machine answer, a client sees the host's word and answers it. */
u32 frontend::neta::barrier(App* app, ThreadState& ts, u32 seconds, u32 point)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    constexpr u32 kBarrierPacket = 4, kPacketBytes = 4, kGaveUp = 0xc;
    const u32 frame = guest.frame();
    guest.setDword(frame + 4, point);
    if (guest.dword(kSessionStage) == 0)
        return seconds;
    const u16 over = guest.word(kSessionOver);
    if (over != 0)
        return seconds;
    u32 giveUpAt = seconds << 7;
    if (giveUpAt != 0)
        giveUpAt += guest.dword(kTicks);
    guest.setByte(frame, kBarrierPacket);
    guest.setWord(frame + 2, u16(guest.dword(frame + 4)));

    if (guest.word(kIsHost) != 0)
    {
        sendLeaving(app, ts, 0, frame, kPacketBytes);
        for (u32 machine = 1;; ++machine)
        {
            const u32 count = signedWord(guest, kMachineCount);
            if (s32(machine) >= s32(count))
                return count;
            const u32 barrier = kMachineBarriers + machine * 2;
            const u32 link = kMachineLinks + machine * 4;
            if (guest.dword(guest.dword(link) + 0x24) == 0 || guest.word(kMachineGone + machine * 2) != 0)
            {
                guest.setWord(barrier, 0);
                continue;
            }
            bool wasDropped = false;
            for (;;)
            {
                if (signedWord(guest, barrier) == guest.dword(frame + 4))
                    break;
                const u32 tasks = u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u));
                if (guest.word(kSessionOver) != 0)
                {
                    guest.setWord(barrier, 0);
                    return tasks;
                }
                if (guest.dword(guest.dword(link) + 0x24) == 0)
                {
                    guest.setWord(barrier, 0);
                    wasDropped = true;
                    break;
                }
                if (giveUpAt != 0 && s32(guest.dword(kTicks)) > s32(giveUpAt))
                {
                    guest.setWord(frame + 2, kGaveUp);
                    guest.setWord(barrier, 0);
                    fe::direct<0x4a60e0, ea::packetSent>(app, ts, guest.dword(link), frame, kPacketBytes);
                    wasDropped = true;
                    break;
                }
            }
            if (wasDropped)
                directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.ts(), h, machine);
            enter(guest, ts, kBarrierLock);
            if (signedWord(guest, barrier) == guest.dword(frame + 4))
                guest.setWord(barrier, 0);
            leave(guest, ts, kBarrierLock);
        }
    }

    for (;;)
    {
        const u32 wanted = guest.dword(frame + 4);
        if (signedWord(guest, kHostBarrier) == wanted)
        {
            enter(guest, ts, kBarrierLock);
            if (signedWord(guest, kHostBarrier) == wanted)
                guest.setWord(kHostBarrier, 0);
            leave(guest, ts, kBarrierLock);
            return sendLeaving(app, ts, kEndpoints, frame, kPacketBytes);
        }
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
        if (guest.word(kSessionOver) != 0 || guest.dword(kEndpointUsed) == 0)
        {
            guest.setWord(kSessionFailed, 1);
            guest.setWord(kHostBarrier, 0);
            guest.setWord(kSessionOver, 1);
            return 0;
        }
        if (giveUpAt != 0)
        {
            const u32 ticks = guest.dword(kTicks);
            if (s32(ticks) > s32(giveUpAt))
            {
                guest.setWord(kHostBarrier, 0);
                guest.setWord(kSessionFailed, 1);
                guest.setWord(kSessionOver, 1);
                return ticks;
            }
        }
    }
}

nfs::u32 frontend::netBarrier(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6a10", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 seconds = args.eax, point = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::barrier(app, ts, seconds, point); });
}

/* sub_4c5e80, before a network race: the settings that matter sent from the host in 0xd0 byte
 * packets, kept apart on a client around it for each of the 16 players' car and colour; and
 * the player this machine is. */
u32 frontend::neta::raceSettings(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xfc);
    constexpr u32 kSettings = 0x6fd39c, kSettingsEnd = 0x6fdc2c, kSendFrom = 0x7d36d4, kSettingsPlace = 0x7d36d0;
    constexpr u32 kSettingsPacket = 5, kPacketRoom = 0xd0, kPlayers = 16;
    constexpr u32 kPlayerCarField = 0x14, kPlayerExtraField = 0x68;
    constexpr s32 kMaxCar = 0x31;
    const u32 frame = guest.frame();
    const u32 packet = frame, keptPlace = frame + 0xdc, roomPlace = frame + 0xd4, leftPlace = frame + 0xe0, chunkPlace = frame + 0xd8;
    guest.setDword(kSettingsPlace, 0);
    guest.setDword(kSendFrom, kSettings);
    if (guest.word(kIsHost) == 0)
    {
        const u32 kept = u32(*fe::direct<0x4e1620, ea::reserved>(app, ts, 0x540214, 0x6c0, 0x10));
        guest.setDword(keptPlace, kept);
        for (u32 i = 0; i < kPlayers; ++i)
            copyBytes(guest, kept + i * kRacePlayerSize, kRacePlayers + i * kRacePlayerSize, kRacePlayerSize);
    }
    guest::direct<0x4a6a10, barrier>(app, ts, 0x5a, 3);
    if (guest.word(kIsHost) != 0)
    {
        guest.setByte(packet, kSettingsPacket);
        guest.setDword(roomPlace, kPacketRoom);
        guest.setDword(leftPlace, kSettingsEnd - kSettings);
        while (s32(guest.dword(leftPlace)) > 0)
        {
            const u32 left = guest.dword(leftPlace), room = guest.dword(roomPlace);
            const u32 chunk = s32(left) < s32(room) ? left : room;
            guest.setDword(chunkPlace, chunk);
            copyBytes(guest, packet + 1, guest.dword(kSendFrom), chunk);
            fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, chunk + 1);
            guest.setDword(kSendFrom, guest.dword(kSendFrom) + guest.dword(chunkPlace));
            guest.setDword(leftPlace, guest.dword(leftPlace) - guest.dword(roomPlace));
        }
    }
    guest::direct<0x4a6a10, barrier>(app, ts, 0x1e, 4);
    if (guest.word(kIsHost) == 0)
    {
        const u32 kept = guest.dword(keptPlace);
        for (u32 i = 0; i < kPlayers; ++i)
        {
            const u32 at = i * kRacePlayerSize;
            const u32 car = guest.dword(kept + at + kPlayerCarField);
            if (s32(car) >= 0 && s32(car) <= kMaxCar)
                guest.setDword(kRacePlayers + at + kPlayerCarField, car);
            guest.setDword(kRacePlayers + at + kPlayerExtraField, guest.dword(kept + at + kPlayerExtraField));
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, kept);
    }
    u32 result = signedWord(guest, kThisMachine);
    guest.setDword(0x6fd4f4, result);
    guest.setDword(0x6fd4f8, result);
    if (guest.dword(0x6fd50c) != 0)
    {
        for (u32 i = 0; s32(i) < s32(guest.dword(kRacePlayerCount)); ++i)
        {
            if (i == guest.dword(0x6fd4f4))
                continue;
            result = i * 12;
            guest.setDword(0x6fd548 + i * kRacePlayerSize, 1);
        }
    }
    return result;
}

nfs::u32 frontend::netRaceSettings(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5e80", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::raceSettings(app, ts); });
}

/* sub_4a66c0: the host sends the machines' list, packet 1, to each of them, naming each
 * machine by its number in its own copy; then the settings. */
u32 frontend::neta::sendSetup(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x98);
    constexpr u32 kSetupBytes = 0x7c, kEndpointsTried = 7;
    const u32 packet = guest.frame();
    guest.setWord(kThisMachine, 0);
    guest.setWord(kMachineCount, 1);
    guest.setByte(kPacketSetup, 1);
    guest.setDword(kSetupOther, guest.dword(kLinkKind));
    guest.setWord(kSetupDirty, 0);
    guest.setByte(kSetupLinks, u8(guest::direct<0x4a5af0, thisLink>(app, ts)));
    guest.copyString(kSetupNames, kPlayerName);
    for (u32 i = 0; i < kEndpointsTried; ++i)
    {
        const u32 at = i * kEndpointSize;
        if (guest.dword(kEndpointUsed + at) == 0)
            continue;
        const u32 machine = signedWord(guest, kMachineCount);
        guest.setDword(kMachineLinks + machine * 4, kEndpoints + at);
        guest.copyString(kSetupNames + machine * kNameSize, kEndpointNames + i * kNameSize);
        if (guest.dword(kLinkKind) == 2)
            guest.setByte(signedWord(guest, kMachineCount) + kSetupLinks, guest.byte(kEndpointLink + at));
        guest.setWord(kMachineCount, u16(guest.word(kMachineCount) + 1));
    }
    guest.setWord(0x7a227a, 0);
    guest.setWord(kMachineGone, 0);
    for (u32 i = 1; i < 8; ++i)
    {
        guest.setWord(kMachineGone + i * 2, 0);
        const u32 number = fe::direct<0x4a6650, ea::machineOf>(app, ts, kEndpoints + (i - 1) * kEndpointSize);
        guest.setWord(kMachineNumbers + (i + 1) * 2, u16(number));
    }
    copyBytes(guest, packet, kPacketSetup, kSetupBytes);
    enter(guest, ts, kPlayersLock);
    for (u32 i = 1; s32(signedWord(guest, kMachineCount)) > s32(i); ++i)
    {
        guest.setWord(packet + 0x64, u16(i));
        fe::direct<0x4a60e0, ea::packetSent>(app, ts, guest.dword(kMachineLinks + i * 4), packet, kSetupBytes);
    }
    leave(guest, ts, kPlayersLock);
    return guest::direct<0x49aa80, sendSettings>(app, ts);
}

nfs::u32 frontend::netSendSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a66c0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendSetup(app, ts); });
}

/* sub_49bab0: the cars' files between the machines that need them, a machine at a time with a
 * barrier after each.  First the client's own car to the host when it is one of its own, then
 * the host's copies back out; a client only waits through the second lot. */
u32 frontend::neta::exchangeCars(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1f8);
    constexpr u32 kWaitSeconds = 0x3c, kFirstBarrier = 0xf, kSecondBarrier = 0x17;
    constexpr u32 kPleaseWait = 0x115, kRedraws = 15;
    const u32 frame = guest.frame();
    // Each redraw hands on what the call before left in eax.
    u32 left = fe::direct<0x444be0, frontend::dialogs::messageById>(app, ts, kPleaseWait);
    for (u32 i = 0; i < kRedraws; ++i)
        left = u32(*fe::direct<0x44b170, port::buttons::redrawn>(app, ts, ts.sp - 4,
                                                                   port::Args{ left, h.edx, h.ebx, h.ecx }));
    if (guest.word(kIsHost) != 0)
    {
        for (u32 i = 0; s32(i) < s32(signedWord(guest, kMachineCount)); ++i)
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, i + kFirstBarrier);
    }
    else
    {
        const u32 mine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, signedWord(guest, kThisMachine)));
        const u32 car = u32(*fe::direct<0x439520, load::idOf>(app, ts, guest.dword(kPlayerCar)));
        for (u32 i = 0; s32(i) < s32(signedWord(guest, kMachineCount)); ++i)
        {
            const u32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, i));
            if (machine != u32(-1) && machine == mine
                && fe::direct<0x439530, frontend::loada::carIdIsPlain>(app, ts, car) != 0)
            {
                const u32 header = frame + 0x1a8, body = frame + 0xd4;
                guest.setByte(header, 0xf);
                const u32 file = callHeld(guest, h, kBeginCarSend, args({ header, guest.dword(kPlayerCar) }));
                guest.setByte(body, 0x10);
                guest::direct<0x49b9b0, sendCarFile>(app, ts, body, file, guest.dword(header + 0x18));
            }
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, i + kFirstBarrier);
        }
    }
    u32 count;
    if (guest.word(kIsHost) != 0)
    {
        for (u32 i = 0; (count = signedWord(guest, kMachineCount)), s32(i) < s32(count); ++i)
        {
            const u32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, i));
            if (machine != u32(-1))
            {
                const u32 id = kPlayerRecords + machine * kPlayerRecordSize + 0x19;
                const u32 car = fe::direct<0x439990, load::placeById>(app, ts, id);
                if (car == u32(-1))
                    fe::direct<0x401010, ea::error>(app, ts, 0x53c5b0u, guest::Words({}));
                if (fe::direct<0x439530, frontend::loada::carIdIsPlain>(app, ts, id) != 0)
                {
                    const u32 header = frame + 0x1c4, body = frame;
                    guest.setByte(header, 0x11);
                    const u32 file = callHeld(guest, h, kBeginCarSend, args({ header, car }));
                    guest.setByte(body, 0x12);
                    guest::direct<0x49b9b0, sendCarFile>(app, ts, body, file, guest.dword(header + 0x18));
                }
            }
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, i + kSecondBarrier);
        }
    }
    else
    {
        for (u32 i = 0; (count = signedWord(guest, kMachineCount)), s32(i) < s32(count); ++i)
            guest::direct<0x4a6a10, barrier>(app, ts, kWaitSeconds, i + kSecondBarrier);
    }
    return count;
}

nfs::u32 frontend::netExchangeCars(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bab0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::exchangeCars(app, ts); });
}

/* sub_49eae0, from the host's lobby: the machines that have come and gone since last time.
 * 1 when any did.  A client has its own (sub_4a69c0). */
u32 frontend::neta::hostLobby(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x3c);
    constexpr u32 kJoining = 0x55e714, kFarAway = 0xfffffff, kAcceptCount = 0x7a01e0, kAcceptRoom = 0x7a0390;
    const u32 frame = guest.frame();
    const u32 changedPlace = frame + 0x20, joiner = frame;
    guest.setWord(changedPlace, 0);
    if (guest.dword(kSessionStage) != 3)
        return 0;
    if (guest.word(kIsHost) == 0)
        return guest::direct<0x4a69c0, clientLobby>(app, ts);
    if (guest.dword(kLinkKind) != 2)
    {
        for (u32 i = 0; i < 7; ++i)
        {
            const u32 at = i * kEndpointSize;
            if (guest.dword(kEndpointUsed + at) == 0 && guest.dword(kEndpointOffered + at) != 0)
            {
                directKept<0x49ecb0, ea::connectionDropped, 1>(guest.app(), guest.ts(), h, kEndpoints + at);
                guest.setWord(changedPlace, 1);
            }
        }
    }
    while (guest::direct<0x49f680, pendingEmpty>(app, ts) == 0)
    {
        guest::direct<0x49f730, nextPending>(app, ts, joiner);
        u32 slot = 0;
        for (; slot < 8; ++slot)
        {
            if (guest.dword(kEndpointUsed + slot * kEndpointSize) == 0)
            {
                guest.copyString(kEndpointNames + slot * kNameSize, joiner);
                break;
            }
        }
        guest.setDword(kJoining, guest.dword(kTicks) + 0x280);
        const u32 endpoint = fe::direct<0x49efb0, frontend::connect::setUpEndpoint>(app, ts, joiner + 0xb, slot);
        bool joined = true;
        if (guest.dword(kLinkKind) != 2)
        {
            if (guest::direct<0x4f5850, sendQueued>(app, ts, endpoint, 0xd2, 7, kAcceptRoom) == 0)
            {
                guest.setDword(kAcceptCount, guest.dword(kAcceptCount) - 1);
                joined = false;
            }
        }
        if (joined)
        {
            guest.setWord(kMachineCount, u16(guest.word(kMachineCount) + 1));
            guest.setWord(changedPlace, 1);
            guest::direct<0x4a6ce0, compareVersions>(app, ts, endpoint);
        }
        guest.setDword(kJoining, kFarAway);
    }
    const u16 changed = u16(guest.word(changedPlace) | guest.word(kSetupDirty));
    guest.setWord(changedPlace, changed);
    if (changed != 0)
        guest::direct<0x4a66c0, sendSetup>(app, ts);
    return signedWord(guest, changedPlace);
}

nfs::u32 frontend::netHostLobby(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49eae0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::hostLobby(app, ts); });
}

/* sub_49e080: the host's game started, on RaceNet: the settings the launcher left at 0x7a1fa0,
 * then NPS opened with its server or with EA's own. */
u32 frontend::neta::hostStart(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    constexpr u32 kServerName = 0x7a1fa0;
    guest.setDword(kSessionStage, 3);
    guest.setDword(kLinkKind, 2);
    guest.setDword(0x6fd50c, 1);
    guest.setDword(0x6fd3b0, 4);
    guest.setDword(0x55e71c, 0);
    guest.setByte(kPacketSetup, 1);
    guest.setWord(0x7a227a, 0);
    guest.copyString(kSetupNames, kPlayerName);
    for (u32 i = 2; i <= 8; ++i)
        guest.setWord(kMachineNumbers + i * 2, 0xffff);
    fe::direct<0x49f650, frontend::connect::clearQueue>(app, ts, 0);
    guest::direct<0x4a6500, sessionReset>(app, ts);
    callHeld(guest, h, kOpenMessages);
    guest.setDword(0x55e860, 1);
    u32 started;
    if (guest.length(kServerName) != 0)
    {
        guest.copyString(kPlayerName, 0x7a1fe0);
        guest.setDword(0x6fd3b8, guest.dword(0x7a2000));
        guest.setDword(0x6fd4ac, guest.dword(0x7a2004));
        const u32 opponents = guest.dword(0x7a200c);
        guest.setDword(0x6fd4b4, opponents);
        guest.setDword(0x6fd4b0, opponents == 8 ? 2 : opponents == 4 ? 1 : 0);
        guest.setDword(0x6fd470, guest.dword(0x7a2010));
        guest.setDword(0x6fd4d4, guest.dword(0x7a2014));
        directKept<0x439a10, frontend::loada::playerToListedCar, 1>(guest.app(), guest.ts(), h, guest.dword(0x7a2008), 0, kServerName);
        const u32 port = (h.edx & 0xffff0000u) | guest.word(0x7a1ffc);
        started = guest::direct<0x4a5b00, npsStart>(app, ts, kPlayerName, port, h.ebx, guest.dword(0x7a1fdc));
    }
    else
    {
        started = guest::direct<0x4a5b00, npsStart>(app, ts, kPlayerName, 0, 0x53c6bc, 0x1a0a);
    }
    return started != 0 ? 1 : 0;
}

nfs::u32 frontend::netHostStart(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e080", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::hostStart(app, ts); });
}

/* sub_4f61f0, the IPX service's receive, stack the room in the buffer: eax the state, edx where
 * the sender's address goes, ebx the kind of packet asked for and where the kind found goes, ecx
 * the buffer.  The first in the queue that fits; ones older than 16 ticks are thrown away. */
u32 frontend::neta::ipxReceive(App* app, ThreadState& ts, u32 state, u32 address, u32 kindPlace, u32 buffer, u32 room)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x3c);
    constexpr u32 kPacketsMatch = 0x4f5a80;
    const u32 frame = guest.frame();
    // What the queue's match reads: the kind, the address, the state.
    guest.setDword(frame, guest.dword(kindPlace));
    copyBytes(guest, frame + 4, address, 20);
    guest.setDword(frame + 0x18, state);
    const u32 received = state + 0x8c;
    u32 packet = fe::direct<0x50e5a0, frontend::watb::queueFetch>(app, ts, received, kPacketsMatch, frame);
    const u32 freeQueue = state + 0x70;
    while (packet != 0)
    {
        if (s32(guest.dword(kClock) + 0x10) >= s32(guest.dword(packet + 4)))
        {
            u32 size = guest.dword(packet + 8) - 4;
            if (s32(size) > s32(room))
                size = room;
            else if (s32(size) < 0)
                size = 0;
            copyBytes(guest, address, packet + 0xc, 20);
            guest.setDword(kindPlace, guest.dword(packet + 0x20));
            if (size != 0)
                completed(fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, buffer, packet + 0x28, size));
            if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
                guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d2a8, packet + 0x20, packet + 0xc, 0x14, 0, 0);
            guest::direct<0x50de50, netb::qpush>(app, ts, freeQueue, packet);
            fe::direct<0x4dfb30, ea::eventSet>(app, ts, guest.dword(state + 0x1c));
            return size;
        }
        if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d29c, packet + 0x20, packet + 0xc, 0x14, 0, 0);
        guest::direct<0x50de50, netb::qpush>(app, ts, freeQueue, packet);
        fe::direct<0x4dfb30, ea::eventSet>(app, ts, guest.dword(state + 0x1c));
        // The match and its frame too, as the call before leaves them in edx and ebx.
        packet = fe::direct<0x50e5a0, frontend::watb::queueFetch>(app, ts, received, kPacketsMatch, frame);
    }
    guest.setDword(kindPlace, 0);
    return 0;
}

nfs::u32 frontend::netIpxReceive(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f61f0", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 state = args.eax;
    const u32 address = args.edx;
    const u32 kindPlace = args.ebx;
    const u32 buffer = args.ecx;
    const u32 room = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxReceive(app, ts, state, address, kindPlace, buffer, room); });
}

/* sub_49a8e0: this machine's player sent to the others, packet 3: its name and car by name and
 * id, its colours and what goes with it, 0x60 bytes, a copy kept at its own number. */
u32 frontend::neta::sendMyCar(App* app, ThreadState& ts)
{
    return sendMyCarFrom(app, ts, 0);
}

u32 frontend::neta::sendMyCarFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    guest.setByte(kMyRecord, 3);
    const u32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, signedWord(guest, kThisMachine)));
    guest.setDword(0x79f91c, machine);
    if (machine == u32(-1))
        return machine;
    guest.setByte(0x79f920, guest.byte(kPlayerTransmission));
    guest.copyString(0x79f921, kPlayerName);
    const u32 id = u32(*fe::direct<0x439520, load::idOf>(app, ts, guest.dword(kPlayerCar)));
    copyBytes(guest, 0x79f931, id, 0x10);
    u32 text = u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4, guest.dword(kPlayerCar), std::int32_t(0)));
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), 0x79f941, text, 0xf);
    guest.setByte(0x79f950, 0);
    text = u32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, ts, ts.sp - 4, guest.dword(kPlayerCar), std::int32_t(1)));
    // By address: the ebx and edx it leaves may be left at the end; eax is read after it.
    h.eax = fe::direct<0x4e0e30, watcom::clib::strncpy>(app, ts, 0x79f951, text, 0x10);
    guest.setByte(0x79f960, 0);
    fe::direct<0x472360, port::fields::customColoursRefreshed>(app, ts, ts.sp - 4, h.eax);
    for (u32 i = 0; i < 4; ++i)
        guest.setByte(0x79f961 + i, guest.byte(kPlayerColours + i));
    const u32 record = fe::direct<0x439a50, load::recordAt>(app, ts, guest.dword(kPlayerCar));
    guest.setDword(0x79f968, guest.dword(record + 0x380));
    guest.setByte(0x79f92c, guest.byte(kPlayerCar));
    guest.setByte(0x79f92d, guest.byte(kPlayerColour));
    guest.setByte(0x79f92e, guest.byte(0x6fd2dc));
    guest.setByte(0x79f92f, guest.byte(0x6fd2e0));
    guest.setByte(0x79f930, guest.byte(0x6fd2e4));
    const u32 extras = fe::direct<0x439510, frontend::loada::carExtras>(app, ts, guest.dword(kPlayerCar));
    copyBytes(guest, 0x79f96c, extras, 12);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kPlayerRecords + guest.dword(0x79f91c) * kPlayerRecordSize, kMyRecord, kPlayerRecordSize);
    return fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, kMyRecord, kPlayerRecordSize)->eax;
}

nfs::u32 frontend::netSendMyCar(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49a8e0", 0, 0, nullptr, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendMyCarFrom(app, ts, handed); });
}

/* sub_4c8040: a player's controls for the race's frame: eax where they go, edx the player.  A
 * local player's are in the tables here, a machine's from the ring of 20 frames its packets fill,
 * valid only for the frame asked and counted for how stale they get.  0 when there are none. */
u32 frontend::neta::playerControls(App* app, ThreadState& ts, u32 out, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    constexpr u32 kLocalControls = 0x7dd010, kWantedFrames = 0x7dd2a0, kRingFrames = 0x7dd120, kRingControls = 0x7dd110;
    const u32 cars = guest.dword(kCarCount);
    const u32 machine = player - cars;
    enter(guest, ts, kInputLock);
    if (s32(player) < s32(guest.dword(kCarCount)))
    {
        const u32 local = kLocalControls + player * 12;
        guest.setDword(out + 8, guest.dword(local));
        guest.setWord(out + 4, guest.word(local + 4));
        const u32 car = guest.dword(kCarsByPlayer + player * 4);
        guest.setDword(out + 0x10, 1);
        guest.setWord(out + 0xc, u16(guest.word(car + 8) - guest.word(out + 4)));
        guest.setByte(car + 0x50e, (guest.byte(local + 8) >> 3) & 1);
        if (guest.word(out + 4) == 0)
        {
            guest.setDword(out + 8, 0);
            guest.setWord(out + 0xc, 0);
        }
        leave(guest, ts, kInputLock);
        return 1;
    }
    const u32 wanted = kWantedFrames + machine * 20;
    const u32 frameWanted = guest.dword(wanted);
    guest.setDword(out, frameWanted);
    if (frameWanted == 0)
    {
        leave(guest, ts, kInputLock);
        return 0;
    }
    const s32 inRing = (s32(frameWanted) >> 5) % 20;
    const u32 ring = u32(inRing) * 20;
    if (frameWanted != guest.dword(kRingFrames + ring))
    {
        leave(guest, ts, kInputLock);
        return 0;
    }
    const u32 controls = kRingControls + ring + machine * 2;
    if (guest.word(controls) == 0)
    {
        leave(guest, ts, kInputLock);
        return 0;
    }
    const u16 pressed = guest.word(wanted + 4);
    guest.setWord(out + 4, pressed);
    guest.setWord(out + 0xc, u16(guest.word(controls) - pressed));
    const u32 stale = guest.dword(wanted + 0x10) + 1;
    guest.setDword(wanted + 0x10, stale);
    guest.setDword(out + 0x10, stale);
    leave(guest, ts, kInputLock);
    return 1;
}

nfs::u32 frontend::netPlayerControls(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c8040", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 out = args.eax, player = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::playerControls(app, ts, out, player); });
}

namespace
{

constexpr u32 kFread = 0x4e9200, kFclose = 0x4ee100, kStrrchr = 0x4ee2f0;
constexpr u32 kQueueFindPosition = 0x50e220;
constexpr u32 kSyncFrame = 0x4c6970;
constexpr u32 kExitNet = 0x4f4c10, kNetTrace = 0x4f4c50;

// The services' games, 32 bytes each, and the lock over them.
constexpr u32 kServiceLock = 0x566dfc, kGameCount = 0x566e00, kGames = 0x9f2644;
constexpr u32 kGameTag = 0xea646174;
constexpr u32 kSocketLock = 0x564480;

}  // namespace

/* sub_49acd0, as a network race loads: the cars' files exchanged and every machine at the same
 * point, then each player's record made into the race's players' table. */
u32 frontend::neta::playersToRace(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    guest::direct<0x49bab0, exchangeCars>(app, ts);
    // What the calls leave in eax goes on to the dialog's close at the end, which reads it.
    u32 left = guest::direct<0x4a6a10, barrier>(app, ts, 0x1e, 0x1f);
    for (u32 i = 0; s32(i) < s32(signedWord(guest, kMachineCount)); ++i)
    {
        left = directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, i);
        const u32 machine = widened(left);
        if (machine == u32(-1))
            continue;
        const u32 record = machine * kPlayerRecordSize;
        const u32 from = kPlayerRecords + record;
        guest.setDword(guest.frame(), from);
        const u32 slot = i * kRacePlayerSize;
        guest.copyString(kRacePlayers + slot + 0x34, from + 9);
        left = fe::direct<0x439990, load::placeById>(app, ts, from + 0x19);
        guest.setDword(slot + 0x6fd540, left);
        guest.setDword(slot + 0x6fd548, guest.byte(record + 0x79f605));
        for (u32 c = 0; c < 4; ++c)
            guest.setByte(slot + 0x6fd54c + c, guest.byte(record + 0x79f639 + c));
        guest.setDword(slot + 0x6fd550, guest.byte(record + 0x79f606));
        guest.setDword(slot + 0x6fd554, guest.byte(record + 0x79f607));
        guest.setDword(slot + 0x6fd558, guest.byte(record + 0x79f608));
        copyBytes(guest, slot + 0x6fd588, record + 0x79f644, 12);
    }
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 record = i * kPlayerRecordSize;
        guest.setByte(record + 0x79f5f8, 0);
        guest.setDword(record + 0x79f5f4, 0);
    }
    return u32(*fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, ts, left));
}

nfs::u32 frontend::netPlayersToRace(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49acd0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::playersToRace(app, ts); });
}

/* sub_4a6ce0: the game's version, packet 0: the host sends its own to a machine that joins, a
 * client waits for the host's and stops the game with an error when they differ. */
u32 frontend::neta::compareVersions(App* app, ThreadState& ts, u32 link)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x44);
    constexpr u32 kOurVersion = 0x55e898, kHostVersion = 0x55ed08, kGameQuits = 0x564819;
    constexpr u32 kSourceFile = 0x53dbdc, kVersionsDiffer = 0x53dbfc, kLine = 0x4d1;
    constexpr u32 kBackslash = 0x5c;
    const u32 packet = guest.frame(), version = packet + 1;
    guest.setByte(packet, 0);
    guest.copyString(version, kOurVersion);
    if (guest.word(kIsHost) != 0)
    {
        fe::direct<0x4a60e0, ea::packetSent>(app, ts, link, packet, 0x29);
        return 1;
    }
    while (guest.byte(kHostVersion) == 0)
    {
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        if (guest.byte(kGameQuits) != 0 || guest.word(kSessionOver) != 0)
            return 0;
    }
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), kHostVersion, version) == 0)
    {
        guest.setByte(kHostVersion, 0);
        return 1;
    }
    directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.ts(), h);
    u32 file = kSourceFile;
    if (fe::direct<kStrrchr, watcom::clib::strrchr>(guest.app(), guest.ts(), kSourceFile, kBackslash) != 0)
        file = fe::direct<kStrrchr, watcom::clib::strrchr>(guest.app(), guest.ts(), kSourceFile, kBackslash) + 1;
    fe::direct<kError, ea::error>(guest.app(), guest.ts(), kVersionsDiffer, guest::Words{1, file, kLine, kHostVersion, version});
    return 0;
}

nfs::u32 frontend::netCompareVersions(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6ce0", 0, 0, nullptr, 0 };
    const u32 link = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::compareVersions(app, ts, link); });
}

/* sub_4f4d10, NET_initservice, stack the three that go on: eax the service's record, edx a
 * name, ebx and ecx what the services' open takes.  The first time, NET_'s queue and its exit;
 * then each service in the table is tried until one takes it, and its record goes on the queue. */
u32 frontend::neta::initService(App* app, ThreadState& ts, u32 service, u32 name, u32 first, u32 second, u32 onOne, u32 onTwo, u32 onThree)
{
    Held h;
    h.eax = service; h.edx = name; h.ebx = first; h.ecx = second;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    constexpr u32 kServicesQueue = 0x9f25f0, kServiceCount = 0x566de4, kReady = 0x566de8;
    constexpr u32 kServiceNames = 0x9f25d0, kServiceTables = 0x9f25b0;
    constexpr u32 kRecordBytes = 0xac;
    const u32 frame = guest.frame();
    guest.setDword(frame + 4, name);
    guest.setDword(frame, first);
    guest.setDword(frame + 8, second);
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.ts(), h, 0x54d0d0);
    if (guest.dword(kReady) == 0)
    {
        guest::direct<0x50dcd0, netb::qinit>(app, ts, kServicesQueue, 0, 0);
        fe::direct<0x4f2a78, frontend::eab::atexitJump>(app, ts, kExitNet);
        if (guest.dword(0x566ddc) == 0)
            guest.setDword(0x566ddc, u32((s32(guest.dword(0x5643d8)) + 9) / 10));
        guest.setDword(kReady, 1);
    }
    if (callHeld(guest, h, kQueueFindPosition, args({ kServicesQueue, service })) != u32(-1))
        guest::direct<0x4f4e60, killService>(app, ts, service);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), service, kRecordBytes);
    guest.setDword(frame + 0xc, 0);
    u32 result = 0;
    if (s32(guest.dword(kServiceCount)) > 0)
    {
        u32 at = 0;
        u32 tried;
        do
        {
            if (result != 0)
                break;
            guest.setDword(service + 8, guest.dword(at + kServiceNames));
            guest.setDword(service + 4, guest.dword(at + kServiceTables));
            guest.setDword(service + 0x10, guest.dword(frame + 4));
            const bool wanted = guest.dword(guest.dword(service + 4) + 4) != 0
                || guest::direct<0x4f4bb0, sameGuid>(app, ts, guest.dword(at + kServiceNames) + 4, guest.dword(frame + 8)) != 0;
            if (wanted)
            {
                result = callHeld(guest, h, guest.dword(guest.dword(service + 4) + 8),
                                     args({ service, guest.dword(frame), guest.dword(frame + 8), onOne }, { onTwo, onThree }));
            }
            tried = guest.dword(frame + 0xc) + 1;
            const u32 count = guest.dword(kServiceCount);
            h.edx = (tried);
            h.ebx = (count);
            at += 4;
            guest.setDword(frame + 0xc, tried);
            if (s32(tried) >= s32(count))
                break;
        } while (true);
    }
    h.ecx = (result);
    if (result != 0)
        guest::direct<0x50de50, netb::qpush>(app, ts, kServicesQueue, service);
    return result;
}

nfs::u32 frontend::netInitService(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4d10", 12, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 service = args.eax;
    const u32 name = args.edx;
    const u32 first = args.ebx;
    const u32 second = args.ecx;
    const u32 onOne = app->getMemory<u32>(ts.sp + 4);
    const u32 onTwo = app->getMemory<u32>(ts.sp + 8);
    const u32 onThree = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::initService(app, ts, service, name, first, second, onOne, onTwo, onThree); });
}

/* sub_49b9b0: a car's file sent, eax the packet with its kind first, edx the file, ebx its size:
 * as much as a packet takes, 0xd0 bytes, with the system let run between them; then the file's
 * 300 byte tail the game keeps at 0x79f97c. */
u32 frontend::neta::sendCarFile(App* app, ThreadState& ts, u32 packet, u32 file, u32 left)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x28);
    constexpr u32 kPacketRoom = 0xd0, kTail = 0x79f97c, kTailBytes = 0x12c, kViv = 0x79f978;
    const u32 filePlace = guest.frame();
    guest.setDword(filePlace, file);
    while (s32(left) > 0)
    {
        const u32 chunk = s32(left) < s32(kPacketRoom) ? left : kPacketRoom;
        if (fe::direct<kFread, watcom::clib::fread>(guest.app(), guest.ts(), packet + 1, chunk, 1, file) == 0)
            return u32(-1);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, chunk + 1);
        left -= chunk;
    }
    const u32 viv = guest.dword(kViv);
    if (viv != 0)
        fe::direct<0x43b9b0, ea::vivClosed>(app, ts, viv, filePlace);
    else
        fe::direct<kFclose, watcom::clib::fclose>(guest.app(), guest.ts(), file);
    u32 sent = 0;
    s32 tail = kTailBytes;
    do
    {
        const u32 chunk = tail < s32(kPacketRoom) ? u32(tail) : kPacketRoom;
        copyBytes(guest, packet + 1, kTail + sent, chunk);
        sent += chunk;
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        sendLeaving(app, ts, 0, packet, chunk + 1);
        tail -= s32(chunk);
    } while (tail > 0);
    return 0;
}

nfs::u32 frontend::netSendCarFile(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b9b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.eax, file = args.edx, left = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendCarFile(app, ts, packet, file, left); });
}

/* sub_49abd0, from the lobby: whether the race can start: 1 alone or when every machine's player
 * has its car; the host sends what it last has of everyone and all wait at barrier 1, twice. */
u32 frontend::neta::allReady(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kStartWanted = 0x55e710;
    if (guest.dword(kSessionStage) != 0)
    {
        if (std::int16_t(guest.word(kMachineCount)) < 2)
            return 0;
        if (guest.word(kIsHost) != 0)
        {
            for (u32 i = 0; s32(i) < s32(signedWord(guest, kMachineCount)); ++i)
            {
                const u32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, i));
                if (machine == u32(-1))
                    return 0;
                if (guest.byte(machine * kPlayerRecordSize + 0x79f5f8) == 0)
                    return 0;
            }
            guest.setDword(kStartWanted, 1);
            if (guest::direct<0x49f680, pendingEmpty>(app, ts) == 0 || guest.word(kPlayersNew) != 0)
            {
                guest.setDword(kStartWanted, 0);
                return 0;
            }
        }
        else if (guest.word(kHostBarrier) != 1)
        {
            return 0;
        }
        if (guest.word(kIsHost) != 0)
        {
            guest::direct<0x49ee50, lobbyClose>(app, ts);
            guest::direct<0x49aa80, sendSettings>(app, ts);
        }
        guest::direct<0x49a8e0, sendMyCar>(app, ts);
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kMachineGone, 0x10);
        guest::direct<0x4a6a10, barrier>(app, ts, 0x3c, 1);
        guest::direct<0x4a6a10, barrier>(app, ts, 0x1e, 1);
    }
    return 1;
}

nfs::u32 frontend::netAllReady(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49abd0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::allReady(app, ts); });
}

/* sub_4c7d30, packet 13: a machine's controls for a frame, edx the packet, ebx its size.  The
 * host passes it on to the others unless RaceNet does; the packed controls are unpacked and kept
 * under the lock, and machine 0's packet says which frame everyone's are for. */
u32 frontend::neta::inputPacket(App* app, ThreadState& ts, u32 packet, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    constexpr u32 kInputRows = 0x7dd070, kRowBytes = 20, kWantedFrames = 0x7dd2a0, kPlayers = 0x6fd520;
    const u32 machine = guest.byte(packet + 1);
    const u32 row = machine * kRowBytes;
    const u32 frameDone = guest.dword(kInputRows + row + 0xc);
    if (s32(frameDone) > s32(guest.dword(packet + 4)))
        return frameDone;
    const u32 local = guest.frame();
    guest.setDword(local + 0xc, packet);
    guest.setDword(local + 8, machine);
    if (guest.word(kIsHost) != 0 && guest.dword(kLinkKind) != 2)
    {
        for (u32 i = 1; s32(i) < s32(signedWord(guest, kMachineCount)); ++i)
        {
            if (i != machine)
                fe::direct<0x4a60e0, ea::packetSent>(app, ts, guest.dword(kMachineLinks + i * 4), packet, size);
        }
    }
    guest.setDword(kInputRows + row + 0xc, guest.dword(packet + 4));
    guest::direct<0x4c8350, unpackControls>(app, ts, packet + 8, local);
    const u32 lock = guest.dword(kInputLock);
    enter(guest, ts, kInputLock);
    copyBytes(guest, kInputRows + row, local, 8);
    u8 readyLow = guest.byte(packet + 0xc);
    u32 eax = (packet & 0xffffff00u) | readyLow;
    guest.setByte(kInputRows + row + 8, readyLow);
    guest.setDword(kInputRows + row + 0x10, 1);
    if (machine == 0)
    {
        u32 player = 0;
        for (; s32(player) < s32(guest.dword(kPlayers)); ++player)
        {
            const u32 wanted = kWantedFrames + player * kRowBytes;
            guest.setDword(wanted, guest.dword(packet + 4));
            guest.setWord(wanted + 4, guest.word(packet + player * 2 + 0xe));
            guest.setDword(wanted + 0x10, 0);
            eax = player * 5;
        }
    }
    return fe::leaveLock(guest, ts, guest.dword(kInputLock));
}

nfs::u32 frontend::netInputPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7d30", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::inputPacket(app, ts, packet, size); });
}

/* sub_4f53f0, a service's poll, eax the game, edx where the sender's address is.  What came for
 * a game the service knows of goes to that game's handler; the rest is read off until none are
 * left, each given to the service's own receive. */
u32 frontend::neta::servicePoll(App* app, ThreadState& ts, u32 service, u32 address)
{
    Held h;
    h.edx = address;
    h.eax = service;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xe0);
    constexpr u32 kPacketRoom = 0xac;
    const u32 frame = guest.frame();
    guest.setDword(frame + 0xc8, address);
    h.edx = (guest.dword(kServiceLock));
    if (guest.dword(kServiceLock) != 0)
    {
        enter(guest, ts, kServiceLock);
        const u32 game = guest::direct<0x4f5390, findGame>(app, ts, service, guest.dword(frame + 0xc8));
        if (s32(game) >= 0)
        {
            const u32 entry = guest.dword(kGames + (game << 5));
            const u32 handler = guest.dword(entry + 8);
            h.edx = (handler);
            if (handler != 0)
                callHeld(guest, h, handler, args({ entry, handler }));
        }
        leave(guest, ts, kServiceLock);
        if (s32(game) >= 0)
            return h.eax;
    }
    const u32 buffer = frame, from = frame + 0xac, tag = frame + 0xc0, size = frame + 0xc4;
    u32 received;
    do
    {
        copyBytes(guest, from, guest.dword(frame + 0xc8), 20);
        guest.setDword(tag, kGameTag);
        received = callHeld(guest, h, guest.dword(guest.dword(service + 8) + 0x58),
                              args({ guest.dword(service + 0xc), from, tag, buffer }, { kPacketRoom }));
        if (received != 0 && guest.dword(tag) != 0)
            callHeld(guest, h, guest.dword(guest.dword(service + 4) + 0x40),
                       args({ service, from, guest.dword(tag), buffer }, { received, size }));
    } while (received != 0);
    return 0;
}

nfs::u32 frontend::netServicePoll(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f53f0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 service = args.eax, address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::servicePoll(app, ts, service, address); });
}

/* sub_4f60b0, WINIPXSend, stack the size: eax the state, edx whom to (0 the state's own port),
 * ebx the packet's kind and ecx its bytes.  The kind goes first in network byte order: two
 * machines read it the same way.  1 when all of it went. */
u32 frontend::neta::ipxSend(App* app, ThreadState& ts, u32 state, u32 whom, u32 kind, u32 data, u32 bytes)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x144);
    constexpr s32 kMostBytes = 0x118;
    const u32 frame = guest.frame();
    if (s32(bytes) > kMostBytes)
        bytes = u32(kMostBytes);
    guest.setDword(frame + 0x12c, kind);
    const u32 kindBytes = (kind >> 24) | ((kind >> 8) & 0xff00u) | ((kind << 8) & 0xff0000u) | (kind << 24);
    guest.setDword(frame, kindBytes);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, frame + 4, data, bytes);
    const u32 to = frame + 0x11c;
    if (whom != 0)
        guest::direct<0x4f58c0, toSockaddr>(app, ts, whom, to);
    else
        guest::direct<0x4f5900, portToSockaddr>(app, ts, signedWord(guest, state + 0x24), to);
    enter(guest, ts, kSocketLock);
    const os::net::Result sent = os::net::sendTo(app, ts, guest.dword(state + 0x58), guest::memory(app, frame),
                                                 bytes + 4, os::net::ipxAt(app, to));
    guest.setDword(frame + 0x134, u32(sent.value));
    leave(guest, ts, kSocketLock);
    if (!sent.ok())
    {
        const u32 level = guest.dword(kLogLevel);
        guest.setDword(frame + 0x130, errorNumber(sent.error));
        if (s32(level) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d284, frame + 0x130, state + 0x58, 4, 1, 0);
    }
    else if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
    {
        guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d290, frame + 0x12c, whom, 0x14, 0, 0);
    }
    return bytes + 4 == guest.dword(frame + 0x134) ? 1 : 0;
}

nfs::u32 frontend::netIpxSend(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f60b0", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 state = args.eax;
    const u32 whom = args.edx;
    const u32 kind = args.ebx;
    const u32 data = args.ecx;
    const u32 bytes = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxSend(app, ts, state, whom, kind, data, bytes); });
}

/* sub_4f5950, one packet off the socket: eax the state, edx the packet's room.  The sender's
 * address kept, decrypted if the session has a key; its kind is its first 4 bytes, network byte
 * order.  0 when there is none. */
u32 frontend::neta::ipxRecvFrom(App* app, ThreadState& ts, u32 state, u32 packet)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x24);
    constexpr u32 kRoom = 0x11c;
    const u32 from = guest.frame(), data = packet + 0x24;
    os::net::Endpoint sender;
    const os::net::Result got = os::net::receiveFrom(app, ts, guest.dword(state + 0x58), guest::memory(app, data),
                                                     kRoom, sender);
    guest.setDword(packet + 8, u32(got.value));
    if (got.value <= 0)
        return 0;
    // The sender's address into the frame, where sub_512890 and sub_4f5880 read it.
    os::net::putIpx(app, from, sender);
    if (guest.byte(state + 0x26) != 0
        && guest::direct<0x512890, netb::ipxScanInterfaces>(app, ts, state, from, data, guest.dword(packet + 8)) != 0)
        return 0;
    guest.setDword(packet + 4, guest.dword(kClock));
    const u32 first = guest.dword(data);
    const u32 kind = (first >> 24) | ((first >> 8) & 0xff00u) | ((first << 8) & 0xff0000u) | (first << 24);
    guest.setDword(packet + 0x20, kind);
    if (kind != 0 && s32(guest.dword(packet + 8)) >= 4)
    {
        guest::direct<0x4f5880, fromSockaddr>(app, ts, from, packet + 0xc);
        if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d210, packet + 0x20, packet + 0xc, 0x14, 0, 0);
        return 1;
    }
    guest.setDword(packet + 8, 0);
    guest.setDword(packet + 0x20, 0);
    return 0;
}

nfs::u32 frontend::netIpxRecvFrom(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5950", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 state = args.eax, packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxRecvFrom(app, ts, state, packet); });
}

/* sub_4c5b00, packet 8: what a machine wants of the race, edx the packet, eax whom from.  0 the
 * start: the host fixes the frame and sends it back; 1 and 2 agreeing and going on; 3 leaving. */
u32 frontend::neta::quitPacket(App* app, ThreadState& ts, u32 from, u32 packet)
{
    Held h;
    h.eax = from; h.edx = packet;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kQuitLock = 0x7d36d8, kStartFrame = 0x7d36c8, kStarted = 0x7d36c0, kHostQuit = 0x7d36c4;
    const u32 what = guest.dword(packet + 4);
    switch (what)
    {
    case 0:
    {
        u32 result;
        if (guest.word(kIsHost) != 0)
        {
            h.edx = (guest.dword(kQuitLock));
            enter(guest, ts, kQuitLock);
            if (guest.dword(kStartFrame) == 0)
            {
                const u32 frame = callHeld(guest, h, kSyncFrame);
                guest.setDword(kStartFrame, frame);
                guest.setDword(packet + 8, frame);
                fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, 0xc);
            }
            result = leave(guest, ts, kQuitLock);
        }
        else
        {
            result = guest.dword(packet + 8);
            guest.setDword(kStartFrame, result);
        }
        guest.setDword(kStarted, 1);
        return result;
    }
    case 1:
        if (guest.word(kIsHost) != 0)
            guest.setDword(kHostQuit, 1);
        else
            guest.setWord(kHostBarrier, 9);
        return what;
    case 2:
        guest.setWord(kHostBarrier, 10);
        return what;
    case 3:
        if (guest.word(kIsHost) != 0)
        {
            enter(guest, ts, kBarrierLock);
            const u32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, ts, from));
            guest.setWord(kMachineBarriers + machine * 2, 11);
            h.edx = (guest.dword(kBarrierLock));
            return leave(guest, ts, kBarrierLock);
        }
        guest.setWord(kHostBarrier, 11);
        return what;
    default:
        return what;
    }
}

nfs::u32 frontend::netQuitPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5b00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax, packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::quitPacket(app, ts, from, packet); });
}

/* sub_4f5130, NET_'s entry to a service's call, stack what goes on: eax the record, edx the
 * service's table, ebx and ecx what it takes.  The record's defaults filled in, the low timer
 * set going for the first that wants polling and stopped with the last.  The result of a record
 * the session refuses is ebp as it came: the original never set it. */
u32 frontend::neta::serviceCall(App* app, ThreadState& ts, u32 record, u32 table, u32 first, u32 second, u32 third)
{
    Held h;
    h.eax = record; h.edx = table; h.ebx = first; h.ecx = second;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kServicesUp = 0x566df4, kPolled = 0x566df8, kNothing = 0x4f50b0, kPoll = 0x4f5110;
    h.edx = (first);
    h.ebx = (second);
    h.ecx = (third);
    if (guest.dword(kServicesUp) == 0)
        guest::direct<0x4f5230, servicesReady>(app, ts);
    if (record != 0)
    {
        if (guest.word(record + 0x2a) == 0)
            guest.setWord(record + 0x2a, 2);
        if (guest.word(record + 0x2c) == 0)
            guest.setWord(record + 0x2c, 4);
        if (guest.word(record + 0x2e) == 0)
            guest.setWord(record + 0x2e, 6);
        for (const u32 field : { 0x14u, 0x1cu, 0x18u, 0x20u })
            if (guest.dword(record + field) == 0)
                guest.setDword(record + field, kNothing);
        guest.setDword(record + 4, table);
        if (guest.dword(table + 0xc) != 0)
        {
            if (guest.dword(kPolled) == 0)
                fe::direct<0x4f2930, frontend::eab::lowTimerAdd>(app, ts, kPoll);
            guest.setDword(kPolled, guest.dword(kPolled) + 1);
        }
    }
    u32 result = 0;  // the ebp the code holds here: not a value the port has
    if (record == 0 || guest::direct<0x512660, netb::connInitFilter>(app, ts, record) != 0)
        result = callHeld(guest, h, guest.dword(table + 4), args({ record }));
    if (record != 0 && guest.dword(table + 0xc) != 0 && result == 0)
    {
        const u32 left = guest.dword(kPolled) - 1;
        h.ecx = (left);
        guest.setDword(kPolled, left);
        if (left == 0)
            fe::direct<0x4f29b0, ea::removeTimer>(app, ts, kPoll);
    }
    return result;
}

nfs::u32 frontend::netServiceCall(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5130", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 record = args.eax;
    const u32 table = args.edx;
    const u32 first = args.ebx;
    const u32 second = args.ecx;
    const u32 third = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceCall(app, ts, record, table, first, second, third); });
}

/* sub_4a68d0, packet 1 on a client: the host's list of machines taken on whole, 0x7c bytes.  On
 * RaceNet the machines on other links become endpoints. */
u32 frontend::neta::setupPacket(App* app, ThreadState& ts, u32 packet)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kHadSetup = 0x7a2210, kSetupBytes = 0x7c;
    if (guest.dword(kHadSetup) != 0)
        enter(guest, ts, kPlayersLock);
    for (u32 i = 1; i <= 8; ++i)
        guest.setWord(kMachineGone - 2 + i * 2, 0);
    copyBytes(guest, kPacketSetup, packet, kSetupBytes);
    u32 eax = kSetupBytes, ebx = 1;
    guest.setWord(kSetupDirty, 1);
    if (guest.dword(kLinkKind) == 2)
    {
        u32 slot = 0;
        for (u32 machine = 0; machine < 8; ++machine)
        {
// it keeps the rest of eax as the code has it at the call
            eax = guest::direct<0x4a5af0, thisLink>(app, ts);
            if (u8(eax) == guest.byte(machine + kSetupLinks))
                continue;
            const u32 at = slot * kEndpointSize;
            eax = at;
            ebx = slot * 12;
            u8 link = 0;
            if (s32(machine) < s32(signedWord(guest, kMachineCount)))
            {
                guest.setDword(kEndpointUsed + at, 1);
                link = guest.byte(machine + kSetupLinks);
            }
            else
            {
                guest.setDword(kEndpointUsed + at, 0);
            }
            ebx = (ebx & 0xffffff00u) | link;
            guest.setByte(kEndpointLink + at, link);
            ++slot;
        }
    }
    if (guest.dword(kHadSetup) != 0)
    {
        eax = leave(guest, ts, kPlayersLock);
    }
    guest.setDword(kHadSetup, 1);
    return eax;
}

nfs::u32 frontend::netSetupPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a68d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::setupPacket(app, ts, packet); });
}

/* sub_4f54f0, a service's open of a game: eax the game, edx what goes with it.  The game is told
 * to the service, then listed among the games under the lock; 0 at once for a service down. */
u32 frontend::neta::serviceOpenGame(App* app, ThreadState& ts, u32 game, u32 with)
{
    Held h;
    h.eax = game; h.edx = with;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kOpened = 0xa8;
    const u32 service = guest.dword(game + 0x4c);
    guest.setDword(game + 8, with);
    const u32 session = guest.dword(service + 0xc);
    if (session == 0)
    {
        h.edx = (0);
        guest.setDword(game + 0x48, 0);
        return guest.dword(game + 0x48);
    }
    guest.setDword(guest.frame(), game);
    const u32 table = guest.dword(service + 8);
    h.edx = (table);
    if (guest.dword(table + 0x48) != 0)
        callHeld(guest, h, guest.dword(table + 0x48), args({ guest.dword(service + 0xc), game + 0x34 }));
    callHeld(guest, h, guest.dword(guest.dword(service + 8) + 0x5c), args({ guest.dword(service + 0xc), game + 0x34, kGameTag, 0 }));
    if (guest.dword(kServiceLock) == 0)
        guest.setDword(kServiceLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    enter(guest, ts, kServiceLock);
    const u32 at = kGames + (guest.dword(kGameCount) << 5);
    guest.setDword(at, game);
    guest.setDword(at + 4, service);
    copyBytes(guest, at + 8, game + 0x34, 20);
    guest.setDword(service + kOpened, guest.dword(service + kOpened) + 1);
    guest.setDword(kGameCount, guest.dword(kGameCount) + 1);
    guest.setDword(game + 0x48, 0xffffffff);
    h.edx = (guest.dword(kServiceLock));
    leave(guest, ts, kServiceLock);
    return guest.dword(game + 0x48);
}

nfs::u32 frontend::netServiceOpenGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f54f0", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const u32 game = args.eax, with = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceOpenGame(app, ts, game, with); });
}

/* sub_49fb60: Windows' dial-up networking loaded and its functions found: 1 when they are all
 * there.  The port hides the modem; the game's code is kept. */
u32 frontend::neta::rasLoad(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    constexpr u32 kRasDll = 0x7a05c0;
    if (guest::direct<0x49fb10, rasLoaded>(app, ts) != 0)
        return 1;
    const u32 library = os::modules::load(app, guest::text(app, 0x53c71c)).value_or(0);
    guest.setDword(kRasDll, library);
    if (library == 0)
        return library;
    constexpr struct { u32 name, place; } kFunctions[] = {
        { 0x53c728, 0x7a05c8 }, { 0x53c734, 0x7a05cc }, { 0x53c748, 0x7a05d0 },
        { 0x53c758, 0x7a05d4 }, { 0x53c770, 0x7a05d8 }, { 0x53c77c, 0x7a05dc },
    };
    for (const auto& function : kFunctions)
        guest.setDword(function.place, os::modules::symbol(app, guest.dword(kRasDll), guest::text(app, function.name)));
    if (guest::direct<0x49fb10, rasLoaded>(app, ts) != 0)
        return 1;
    guest::direct<0x49fc50, rasFree>(app, ts);
    return 0;
}

nfs::u32 frontend::netRasLoad(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49fb60", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::rasLoad(app, ts); });
}

/* sub_4f55e0, a service's close of a game: eax the game.  Taken out of the games' list under the
 * lock, the rest moved down over it, and the service told. */
u32 frontend::neta::serviceCloseGame(App* app, ThreadState& ts, u32 game)
{
    Held h;
    h.eax = game;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kOpened = 0xa8;
    const u32 service = guest.dword(game + 0x4c);
    if (guest.dword(service + 0xc) == 0 || guest.dword(service + kOpened) == 0)
    {
        guest.setDword(game + 0x48, 0);
        return 1;
    }
    enter(guest, ts, kServiceLock);
    const u32 end = guest.dword(kGameCount) << 5;
    u32 found = 0;
    if (game != guest.dword(kGames))
    {
        for (u32 at = 0; s32(at) < s32(end);)
        {
            const u32 next = guest.dword(at + kGames + 0x20);
            at += 0x20;
            ++found;
            if (next == game)
                break;
        }
    }
    const u32 last = guest.dword(kGameCount) - 1;
    guest.setDword(service + kOpened, guest.dword(service + kOpened) - 1);
    guest.setDword(kGameCount, last);
    const u32 to = kGames + (found << 5);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to, to + 0x20, (last - found) << 5);
    leave(guest, ts, kServiceLock);
    callHeld(guest, h, guest.dword(guest.dword(service + 8) + 0x5c), args({ guest.dword(service + 0xc), game + 0x34, kGameTag, 1 }));
    guest.setDword(game + 0x48, 0);
    return 1;
}

nfs::u32 frontend::netServiceCloseGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f55e0", 0, 0, nullptr, 0 };
    const u32 game = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceCloseGame(app, ts, game); });
}

/* sub_49b310, packet 3: another machine's player record, 0x60 bytes, taken on unless it is this
 * machine's own; the host passes it on unless RaceNet does. */
u32 frontend::neta::playerPacket(App* app, ThreadState& ts, u32 packet)
{
    return playerPacketFrom(app, ts, packet, 0);
}

u32 frontend::neta::playerPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    if (guest.word(kSessionOver) != 0)
        return h.eax;
    enter(guest, ts, kPlayersLock);
    if (std::int16_t(guest.word(kThisMachine)) >= 0
        && widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.ts(), h, signedWord(guest, kThisMachine))) == guest.dword(packet + 4))
    {
        return leave(guest, ts, kPlayersLock);
    }
    const u32 machine = guest.dword(packet + 4);
    copyBytes(guest, kPlayerRecords + machine * kPlayerRecordSize, packet, kPlayerRecordSize);
    guest.setWord(kPlayersNew, 1);
    const u32 left = leave(guest, ts, kPlayersLock);
    if (guest.word(kIsHost) == 0 || guest.dword(kLinkKind) == 2)
        return left;
    return sendLeaving(app, ts, 0, packet, kPlayerRecordSize);
}

nfs::u32 frontend::netPlayerPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b310", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx;
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::playerPacketFrom(app, ts, packet, handed); });
}

/* sub_49f880: dial-up networking's phone book: eax where its names go, 21 bytes each, or 0.  How
 * many there are, 0 when it cannot be asked. */
u32 frontend::neta::rasPhoneBook(App* app, ThreadState& ts, u32 names)
{
    Held h;
    h.eax = names;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    constexpr u32 kRasEnumEntries = 0x7a05d0, kEntryCount = 0x7a05e4, kEntries = 0x7a05e8, kEntrySize = 0x108;
    constexpr u32 kBufferTooSmall = 0x25b, kErrorMoreData = 8;
    const u32 bytes = guest.frame();
    guest.setDword(bytes, 0x19c0);
    const u32 loaded = guest::direct<0x49fb60, rasLoad>(app, ts);
    if (loaded == 0)
        return 0;
    guest.setDword(kEntryCount, 0);
    guest.setDword(kEntries, kEntrySize);
    const u32 answer = callHeld(guest, h, guest.dword(kRasEnumEntries), args({}, { 0, 0, kEntries, bytes, kEntryCount }));
    if (answer != 0 && answer != kErrorMoreData && answer != kBufferTooSmall)
        return 0;
    if (names != 0)
    {
        for (u32 i = 0; i < guest.dword(kEntryCount); ++i)
        {
            const u32 name = names + i * 21;
            const u32 entry = kEntries + i * kEntrySize + 4;
            guest.setDword(bytes + 4, entry);
            fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), name, entry, 0x14);
            guest.setByte(name + 0x14, 0);
        }
    }
    return guest.dword(kEntryCount);
}

nfs::u32 frontend::netRasPhoneBook(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f880", 0, 0, nullptr, 0 };
    const u32 names = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::rasPhoneBook(app, ts, names); });
}

/* sub_4f5710, a game's read: eax the game, edx the buffer, ebx its room.  Packets off the service
 * until one gives something for the caller; the number of bytes the last gave. */
u32 frontend::neta::serviceRead(App* app, ThreadState& ts, u32 game, u32 buffer, u32 room)
{
    Held h;
    h.eax = game; h.edx = buffer; h.ebx = room;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x38);
    const u32 frame = guest.frame();
    const u32 service = guest.dword(game + 0x4c);
    guest.setDword(frame + 0x20, buffer);
    guest.setDword(frame + 0x1c, room);
    guest.setDword(frame + 0x24, game);
    const u32 tag = frame + 0x18, more = frame + 0x14;
    for (;;)
    {
        guest.setDword(tag, kGameTag);
        copyBytes(guest, frame, game + 0x34, 20);
        const u32 received = callHeld(guest, h, guest.dword(guest.dword(service + 8) + 0x58),
                                          args({ guest.dword(service + 0xc), frame, tag, buffer }, { room }));
        if (received == 0)
            return 0;
        const u32 answer = callHeld(guest, h, guest.dword(guest.dword(service + 4) + 0x40),
                                        args({ service, frame, guest.dword(tag), buffer }, { received, more }));
        const u32 still = guest.dword(more);
        h.ebx = (still);
        h.edx = (answer);
        if (still == 0)
        {
            guest.setDword(game + 0x24, 0);
        }
        else if (answer != 0 && guest.dword(game + 0x48) != 0)
        {
            guest.setDword(game + 0x48, 1);
        }
        if (answer != 0)
            return received;
    }
}

nfs::u32 frontend::netServiceRead(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5710", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 game = args.eax, buffer = args.edx, room = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceRead(app, ts, game, buffer, room); });
}

/* sub_4c7ea0, after a network race: each machine's times sent to the others, packet 12: this
 * machine's car, the host's the AI cars' as well, a barrier either side. */
u32 frontend::neta::sendRaceResults(App* app, ThreadState& ts)
{
    return sendRaceResultsFrom(app, ts, 0);
}

u32 frontend::neta::sendRaceResultsFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xd8);
    constexpr u32 kResults = 0x224, kResultsBytes = 0xc0, kPacketBytes = 0xc4;
    constexpr u32 kThisPlayer = 0x6fd4f4;
    if (guest.dword(0x6fd50c) == 0 || s32(guest.dword(0x6fd3b0)) <= 1)
        return h.eax;
    const u32 packet = guest.frame();
    guest::direct<0x4a6a10, barrier>(app, ts, 0x5a, 0x22);
    guest.setByte(packet, 0xc);
    guest.setByte(packet + 1, guest.byte(kThisPlayer));
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, packet + 4, guest.dword(kCars + guest.dword(kThisPlayer) * 4) + kResults, kResultsBytes);
    fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, kPacketBytes);
    if (guest.word(kIsHost) != 0)
    {
        for (u32 player = guest.dword(kRacePlayerCount); s32(player) < s32(guest.dword(0x6fd510)); ++player)
        {
            guest.setByte(packet + 1, u8(player));
            const u32 car = guest.dword(kCarsByPlayer + player * 4);
            if (car == 0)
                continue;
            fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, packet + 4, car + kResults, kResultsBytes);
            fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
            fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, kPacketBytes);
        }
    }
    return guest::direct<0x4a6a10, barrier>(app, ts, 0x5a, 0x23);
}

nfs::u32 frontend::netSendRaceResults(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7ea0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendRaceResultsFrom(app, ts, handed); });
}

/* sub_49aa80: the host's settings sent to the others, packet 2, 0x28 bytes. */
u32 frontend::neta::sendSettings(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kPacket = 0x79f8f0, kOpponentCar = 0x79f8f6, kOpponentId = 0x79f8f8;
    guest.setByte(kPacket, 2);
    for (const SettingField& field : kSettingFields)
        guest.setByte(field.packed, guest.byte(field.setting));
    guest.setDword(kSettingTrackPacked, guest.dword(kSettingTrack));
    guest.setByte(kSettingLastPacked, guest.byte(kSettingLast));
    const u8 car = guest.byte(kOpponentCar);
    if (car >= 4)
        copyBytes(guest, kOpponentId, u32(*fe::direct<0x439520, load::idOf>(app, ts, u32(car) - 4)), 16);
    return fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, kPacket, 0x28)->eax;
}

nfs::u32 frontend::netSendSettings(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49aa80", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendSettings(app, ts); });
}

/* sub_4c8250, each race tick: what has come in of each machine's controls moved to where the
 * race reads them, eax the tick; a machine far ahead of it wakes the race's controls. */
u32 frontend::neta::takeInputs(App* app, ThreadState& ts, u32 tick)
{
    Held h;
    h.eax = tick;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    constexpr u32 kRows = 0x7dd070, kLocalControls = 0x7dd010, kThisPlayer = 0x6fd4f4, kRaceTick = 0x7d36ac;
    constexpr u32 kWakeControls = 0x42dc40;
    enter(guest, ts, kInputLock);
    for (u32 player = 0; s32(player) < s32(guest.dword(kRacePlayerCount)); ++player)
    {
        if (player == guest.dword(kThisPlayer))
            continue;
        const u32 row = kRows + player * 20;
        if (guest.dword(row + 0x10) == 0)
            continue;
        const u32 itsTick = guest.dword(row + 0xc);
        if (s32(tick - itsTick) < 0 && s32(guest.dword(kRaceTick) + 0x20) < s32(itsTick))
            callHeld(guest, h, kWakeControls);
        const u32 local = kLocalControls + player * 12;
        copyBytes(guest, local, row, 8);
        guest.setByte(local + 8, guest.byte(row + 8));
        guest.setDword(row + 0x10, 0);
    }
    return leave(guest, ts, kInputLock);
}

nfs::u32 frontend::netTakeInputs(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c8250", 0, 0, nullptr, 0 };
    const u32 tick = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::takeInputs(app, ts, tick); });
}

/* sub_4f5ea0: a game's IPX socket closed, eax the state: the thread told to end and waited for,
 * the queues and the state freed. */
u32 frontend::neta::ipxClose(App* app, ThreadState& ts, u32 state)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kQueueFree = 0x50dde0;
    if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d258, 0, 0, 0, 0, 0);
    const u32 event = guest.dword(state + 0x1c);
    guest.setDword(state + 0x20, 0);
    fe::direct<0x4dfb30, ea::eventSet>(app, ts, event);
    const os::net::Error closed = os::net::close(app, ts, guest.dword(state + 0x58));
    if (closed != os::net::Error::none)
    {
        const u32 errorPlace = guest.frame() + 8;
        guest.setDword(errorPlace, errorNumber(closed));
        if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d268, errorPlace, 0, 0, 1, 0);
    }
    while (guest.dword(state + 0x1c) != 0)
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, state + 0x70);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, state + 0x8c);
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, state);
    return 1;
}

nfs::u32 frontend::netIpxClose(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5ea0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 state = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxClose(app, ts, state); });
}

/* sub_4a5e50, the callback NPS calls with what has happened, stack the session, the message and
 * its data: 0x202 a game gone, 0x203 a player: on a client whether it is in, for the host a
 * player to take on.  Every other message is nothing. */
u32 frontend::neta::npsMessage(App* app, ThreadState& ts, u32 /*s0*/, u32 message, u32 data)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    switch (message & 0xffff)
    {
    case 0x202:
        return guest::direct<0x4a5f30, endpointOfLink>(app, ts, guest.dword(data));
    case 0x203:
        if (guest.word(kIsHost) == 0)
        {
            if (guest.byte(data + 0x64) != 1)
                return message;
            guest.setDword(kEndpointUsed, 1);
            const u8 link = guest.byte(data);
            guest.setByte(kEndpointLink, link);
            return (message & 0xffffff00u) | link;
        }
        else
        {
            const u32 mine = guest.dword(0x7a20a0);
            if (mine == guest.dword(data))
                return mine;
            const u32 player = guest.byte(data);
            return guest::direct<0x49e320, pendingAdd>(app, ts, data + 4, player);
        }
    default:
        return message;
    }
}

nfs::u32 frontend::netNpsMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5e50", 0, 0, nullptr, 0, 0 };
    const u32 word1 = app->getMemory<u32>(ts.sp + 4);
    const u32 message = app->getMemory<u32>(ts.sp + 8);
    const u32 data = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::npsMessage(app, ts, word1, message, data); });
}

/* sub_4a6500: the session's state back to nothing: the barriers and who has gone cleared, the
 * machines' records and the setup's memory, every endpoint. */
u32 frontend::neta::sessionReset(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kEndpointsBytes = 0x34c, kMemoryFlags = 0x5643f4;
    guest.setDword(0x566df0, 0x780);
    for (u32 i = 1; i <= 0x13; ++i)
        guest.setWord(0x7a2292 + i * 2, 0);
    for (u32 i = 1; i <= 8; ++i)
    {
        guest.setWord(0x7a22b8 + i * 2, 0);
        guest.setWord(0x7a22c8 + i * 2, 0);
    }
    guest.setDword(0x7a2210, 0);
    // The game's code hands it 8, the endpoints' size, 0x10 and 0 in its four registers, which it does
    // not read.
    guest::direct<0x49a490, makeLocks>(app, ts);
    guest.setDword(kMemoryFlags, 0x10);
    guest::direct<0x4f5230, servicesReady>(app, ts);
    guest.setDword(kMemoryFlags, 0);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kEndpoints, kEndpointsBytes);
    u32 last = 0;
    for (u32 i = 0; i < 7; ++i)
        last = guest::direct<0x4a6410, endpointHandlers>(app, ts, kEndpoints + i * kEndpointSize);
    return last;
}

nfs::u32 frontend::netSessionReset(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6500", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sessionReset(app, ts); });
}

namespace
{

/* sub_49b3e0 and sub_49b470, one for a host and one for a client: a car's file on its way, the
 * car's id, its time and how big it is.  Already here, it is done; otherwise its receiving is
 * begun, on the machine that does not send. */
u32 carFileComing(Guest& guest, u32 handed, u32 packet, bool received)
{
    Held h;
    h.eax = handed;
    constexpr u32 kFileDone = 0x55e5bc, kFileId = 0x79f5e0, kFileTime = 0x79faa8, kBeginCarReceive = 0x49b540;
    h.edx = (0);
    guest.setDword(kFileDone, 0);
    if (!received)
    {
        guest.setDword(kFileDone, 1);
        return h.eax;
    }
    const u32 id = packet + 1;
    h.edx = (id);
    copyBytes(guest, kFileId, id, 0x10);
    guest.setDword(kFileTime, guest.dword(packet + 0x14));
    const u32 car = fe::direct<0x439990, load::placeById>(guest.app(), guest.ts(), id);
    if (car != u32(-1))
    {
        guest.setDword(kFileDone, 1);
        return car;
    }
    return callHeld(guest, h, kBeginCarReceive, args({ kFileDone, 0, guest.dword(packet + 0x18) }));
}

}  // namespace

u32 frontend::neta::carFileToHost(App* app, ThreadState& ts, u32 packet)
{
    return carFileToHostFrom(app, ts, packet, 0);
}

u32 frontend::neta::carFileToHostFrom(App* app, ThreadState& ts, u32 packet, u32 handed)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    return carFileComing(guest, handed, packet, guest.word(kIsHost) != 0);
}

nfs::u32 frontend::netCarFileToHost(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b3e0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx, handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::carFileToHostFrom(app, ts, packet, handed); });
}

u32 frontend::neta::carFileToClient(App* app, ThreadState& ts, u32 packet)
{
    return carFileToClientFrom(app, ts, packet, 0);
}

u32 frontend::neta::carFileToClientFrom(App* app, ThreadState& ts, u32 packet, u32 handed)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    return carFileComing(guest, handed, packet, guest.word(kIsHost) == 0);
}

nfs::u32 frontend::netCarFileToClient(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b470", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx, handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::carFileToClientFrom(app, ts, packet, handed); });
}

/* sub_49e220: joining a RaceNet game: the session reset and NPS opened, the wait for the host's
 * list, then the versions compared.  The message 0xc2 when any of it does not happen. */
u32 frontend::neta::joinRaceNet(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kResetNetwork = 0x4a6390, kFailed = 0xc2;
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), h, 1, 0, 0xfffffffe);
    guest.setWord(kIsHost, 0);
    guest.setWord(kThisMachine, 0xfffe);
    guest.setWord(kMachineCount, 0);
    const u32 started = guest::direct<0x49e080, hostStart>(app, ts);
    if (started == 0)
        return started;
    while (guest.word(kSetupDirty) == 0)
    {
        if (guest::direct<0x4a65b0, hostGone>(app, ts) != 0 || guest.word(kSessionOver) != 0)
        {
            fe::direct<0x444be0, frontend::dialogs::messageById>(app, ts, kFailed);
            guest.setWord(kSessionOver, 1);
            directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.ts(), h);
            return 0;
        }
    }
    guest.setDword(kLinkKind, guest.dword(kSetupOther));
    if (guest::direct<0x4a6ce0, compareVersions>(app, ts, 0) == 0)
    {
        // The game's code hands it 1 in edx too, which it does not read.
        fe::direct<0x444be0, frontend::dialogs::messageById>(app, ts, kFailed);
        guest.setWord(kSessionOver, 1);
        directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.ts(), h);
        return 0;
    }
    return 1;
}

nfs::u32 frontend::netJoinRaceNet(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e220", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::joinRaceNet(app, ts); });
}

/* sub_4a6c40, packet 4: the barrier a machine has come to.  The host notes it against that
 * machine; a client takes it as the host's, and 12 as the host having given up on it. */
u32 frontend::neta::barrierPacket(App* app, ThreadState& ts, u32 from, u32 packet)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kGaveUp = 0xc;
    if (guest.word(kIsHost) != 0)
    {
        enter(guest, ts, kBarrierLock);
        const u32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, ts, from));
        if (s32(machine) >= 0)
        {
            guest.setWord(kMachineBarriers + machine * 2, guest.word(packet + 2));
        }
        return leave(guest, ts, kBarrierLock);
    }
    if (guest.word(packet + 2) == kGaveUp)
    {
        if (guest.word(kSessionOver) == 0)
        {
            guest.setWord(kSessionFailed, 1);
            guest.setWord(kSessionOver, 1);
        }
        return from;
    }
    enter(guest, ts, kBarrierLock);
    guest.setWord(kHostBarrier, guest.word(packet + 2));
    return leave(guest, ts, kBarrierLock);
}

nfs::u32 frontend::netBarrierPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6c40", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax, packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::barrierPacket(app, ts, from, packet); });
}

/* sub_4f4c70, NET_getservices: eax a function called with each service there is, edx and ebx what
 * goes to it, ecx how big its room is.  A service with its own list is asked for it. */
u32 frontend::neta::getServices(App* app, ThreadState& ts, u32 callback, u32 one, u32 two, u32 room)
{
    Held h;
    h.eax = callback; h.edx = one; h.ebx = two; h.ecx = room;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    constexpr u32 kServiceCount = 0x566de4, kServiceNames = 0x9f25d0, kServiceTables = 0x9f25b0;
    constexpr u32 kBigEnough = 0x20;
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.ts(), h, 0x54d0c0, 1);
    u32 going = 1;
    h.ecx = (0);
    h.ebx = (guest.dword(kServiceCount));
    if (s32(guest.dword(kServiceCount)) > 0)
    {
        for (u32 i = 0; going != 0;)
        {
            const u32 table = guest.dword(kServiceTables + i * 4);
            if (guest.dword(table + 4) != 0)
            {
                going = callHeld(guest, h, guest.dword(table + 4), args({ callback, one, two, room }));
            }
            else if (room <= kBigEnough)
            {
                const u32 name = guest.dword(kServiceNames + i * 4);
                going = callHeld(guest, h, callback, args({ name + 4, guest.dword(name), 0xfa, 0 }));
            }
            h.edx = (going);
            ++i;
            const u32 count = guest.dword(kServiceCount);
            h.ecx = (count);
            if (s32(i) >= s32(count))
                break;
        }
    }
    return going;
}

nfs::u32 frontend::netGetServices(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4c70", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 callback = args.eax, one = args.edx, two = args.ebx, room = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::getServices(app, ts, callback, one, two, room); });
}

/* sub_4f5b10, the IPX thread, by its state in 0x9f3650: a packet taken off the free list and read
 * off the socket, onto the received list; with none free it waits for one. */
u32 frontend::neta::ipxThread(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kThreadStarting = 0x9f3650, kCloseEvent = 0x4dfc00;
    const u32 state = guest.dword(kThreadStarting);
    const u32 event = u32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts));
    guest.setDword(kThreadStarting, 0);
    const u32 running = guest.dword(state + 0x20);
    guest.setDword(state + 0x1c, event);
    if (running != 0)
    {
        const u32 freeQueue = state + 0x70, received = state + 0x8c;
        do
        {
            const u32 packet = guest::direct<0x50df60, netb::qpop>(app, ts, freeQueue);
            if (packet == 0)
            {
                fe::direct<0x4dfb70, ea::eventWaited>(app, ts, guest.dword(state + 0x1c));
            }
            else if (guest::direct<0x4f5950, ipxRecvFrom>(app, ts, state, packet) == 0)
            {
                guest::direct<0x50de50, netb::qpush>(app, ts, freeQueue, packet);
            }
            else
            {
                guest::direct<0x50ded0, netb::qbottom>(app, ts, received, packet);
                if (guest.dword(state) != 0)
                    callHeld(guest, h, guest.dword(state), args({ guest.dword(state + 4), packet + 0xc }));
            }
        } while (guest.dword(state + 0x20) != 0);
    }
    const u32 closed = directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.ts(), h, guest.dword(state + 0x1c));
    guest.setDword(state + 0x1c, 0);
    return closed;
}

nfs::u32 frontend::netIpxThread(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5b10", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxThread(app, ts); });
}

/* sub_4f57c0, a game's write: eax the game, edx the bytes, ebx how many.  A game opening says who
 * it is, every few writes, before it writes; ebx when it went, 0 when not. */
u32 frontend::neta::serviceWrite(App* app, ThreadState& ts, u32 game, u32 bytes, u32 size)
{
    Held h;
    h.eax = game; h.edx = bytes; h.ebx = size;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    constexpr u32 kOpenTag = 0xea6f706e;
    const u32 frame = guest.frame();
    guest.setDword(frame, bytes);
    guest.setDword(frame + 8, game);
    guest.setDword(frame + 4, kGameTag);
    if (s32(guest.dword(game + 0x48)) < 0)
    {
        if (s32(guest.dword(game + 0x48)) >= -4)
        {
            guest.setDword(game + 0x48, guest.dword(game + 0x48) - 1);
        }
        else
        {
            guest.setDword(frame + 4, kOpenTag);
            const u32 service = guest.dword(game + 0x4c);
            const u32 table = guest.dword(service + 8);
            if (callHeld(guest, h, guest.dword(table + 0x54), args({ guest.dword(service + 0xc), game + 0x34, kOpenTag, 0 }, { 0 })) != 0)
                guest.setDword(game + 0x48, 0xffffffff);
        }
    }
    const u32 service = guest.dword(guest.dword(frame + 8) + 0x4c);
    const u32 table = guest.dword(service + 8);
    const u32 sent = callHeld(guest, h, guest.dword(table + 0x54),
                                  args({ guest.dword(service + 0xc), guest.dword(frame + 8) + 0x34, guest.dword(frame + 4), guest.dword(frame) }, { size }));
    return sent != 0 ? size : 0;
}

nfs::u32 frontend::netServiceWrite(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f57c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 game = args.eax, bytes = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceWrite(app, ts, game, bytes, size); });
}

/* sub_49f790: whether dial-up networking is there: a network card, or the DLL and a connection
 * of it.  The port hides the modem; the game's code is kept. */
u32 frontend::neta::rasAvailable(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1b0);
    constexpr u32 kRasEnumConnections = 0x7a05cc, kBytes = 0x19c;
    const u32 connections = guest.frame(), bytes = connections + 0x19c, count = connections + 0x1a0;
    guest.setDword(bytes, kBytes);
    guest.setDword(count, 0);
    if (guest::direct<0x49f810, networkThere>(app, ts) != 0)
        return 1;
    const u32 loaded = guest::direct<0x49fb60, rasLoad>(app, ts);
    if (loaded == 0)
        return loaded;
    guest.setDword(connections, kBytes);
    callHeld(guest, h, guest.dword(kRasEnumConnections), args({}, { connections, bytes, count }));
    if (guest.dword(bytes) != 0 || guest.dword(count) != 0)
    {
        guest::direct<0x49fc50, rasFree>(app, ts);
        return 1;
    }
    return 0;
}

nfs::u32 frontend::netRasAvailable(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f790", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::rasAvailable(app, ts); });
}

/* sub_4f6030: whether the socket has anything: eax the state.  The received queue, then Winsock's
 * count of what waits. */
u32 frontend::neta::ipxReady(App* app, ThreadState& ts, u32 state)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kQueueLength = 0x50e1d0;
    const u32 frame = guest.frame();
    if (directKept<0x50e1d0, ea::queueLengthOf, 1>(guest.app(), guest.ts(), h, state + 0x8c) != 0)
        return 1;
    const os::net::Result waiting = os::net::waiting(app, guest.dword(state + 0x58));
    // The answer where the log reads it: 0, or -1 when there was none.
    guest.setDword(frame, waiting.ok() ? 0 : u32(-1));
    if (!waiting.ok())
    {
        if (s32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, ts, 0x54d274, frame, state + 0x58, 4, 1, 0);
        return 0;
    }
    return waiting.value != 0 ? 1 : 0;
}

nfs::u32 frontend::netIpxReady(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f6030", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 state = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxReady(app, ts, state); });
}

/* sub_4a5dd0: NPS let go: what the session has open, through the DLL's own pointers, then the
 * session, whose number goes to -20. */
u32 frontend::neta::npsStop(App* app, ThreadState& ts)
{
    return npsStopFrom(app, ts, 0);
}

u32 frontend::neta::npsStopFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    constexpr u32 kNpsSession = 0x55ecb0, kNpsClose = 0x7a21e0, kNpsEnd = 0x7a21cc, kNpsVersion = 0x7a215c, kNpsOther = 0x7a2160;
    if (s32(guest.dword(kNpsSession)) < 0)
        return h.eax;
    for (const u32 what : { kNpsVersion, kNpsOther })
    {
        const u32 open = guest.dword(what);
        if (open != 0)
        {
            callHeld(guest, h, guest.dword(kNpsClose), args({}, { guest.dword(kNpsSession), open }));
            guest.setDword(what, 0);
        }
    }
    const u32 ended = callHeld(guest, h, guest.dword(kNpsEnd), args({}, { guest.dword(kNpsSession) }));
    guest.setDword(kNpsSession, 0xffffffec);
    return ended;
}

nfs::u32 frontend::netNpsStop(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5dd0", 0, 0, nullptr, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::npsStopFrom(app, ts, handed); });
}

/* sub_4f4bb0: whether two GUIDs, at eax and edx, are the same. */
u32 frontend::neta::sameGuid(App* app, ThreadState& ts, u32 one, u32 other)
{
    Guest guest(app, ts);
    const u32 first = guest.dword(other);
    if (first != guest.dword(one))
        return 0;
    const u16 second = guest.word(other + 4);
    if (second != guest.word(one + 4) || guest.word(other + 6) != guest.word(one + 6))
        return 0;
    for (u32 i = 0; i < 8; ++i)
    {
        if (guest.byte(one + 8 + i) != guest.byte(other + 8 + i))
            return 0;
    }
    return 1;
}

nfs::u32 frontend::netSameGuid(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4bb0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 one = args.eax, other = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sameGuid(app, ts, one, other); });
}

/* sub_4f5390: which of the services' games a packet is for, eax the service, edx the sender's
 * address: -1 for none. */
u32 frontend::neta::findGame(App* app, ThreadState& ts, u32 service, u32 address)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kCheckPlayer = 0x4f5040;
    if (s32(guest.dword(kGameCount)) <= 0)
        return u32(-1);
    for (u32 i = 0; s32(i) < s32(guest.dword(kGameCount)); ++i)
    {
        const u32 at = i << 5;
        if (service == guest.dword(kGames + 4 + at)
            && got(fe::direct<0x4f5040, ea::netPlayerChecked>(app, ts, service, address, guest.dword(kGames + at) + 0x34)) != 0)
            return i;
    }
    return u32(-1);
}

nfs::u32 frontend::netFindGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5390", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 service = args.eax, address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::findGame(app, ts, service, address); });
}

/* sub_49f810: whether the machine has a network: Windows' word for it, or Winsock opening. */
u32 frontend::neta::networkThere(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x19c);
    if (os::window::kNetworkPresent)
        return 1;
    if (!os::net::start())
        return 0;
    const std::optional<os::Handle> socket = os::net::open(app, os::net::Kind::tcp);
    if (!socket)
    {
        os::net::stop();
        return 0;
    }
    os::net::close(app, ts, *socket);
    os::net::stop();
    return 1;
}

nfs::u32 frontend::netNetworkThere(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f810", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::networkThere(app, ts); });
}

/* sub_4c7fd0: a machine's ring of controls emptied, eax the player; for a local one nothing. */
u32 frontend::neta::clearRing(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x18);
    constexpr u32 kRingControls = 0x7dd110;
    if (s32(player) < s32(guest.dword(kCarCount)))
        return player;
    enter(guest, ts, kInputLock);
    for (u32 i = 0; i < 20; ++i)
        guest.setWord(kRingControls + (player - guest.dword(kCarCount)) * 2 + i * 20, 0);
    return leave(guest, ts, kInputLock);
}

nfs::u32 frontend::netClearRing(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7fd0", 0, 0, nullptr, 0 };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::clearRing(app, ts, player); });
}

/* sub_4f5a30, a service's listing asked for and answered: eax the state, edx whom to, ebx the
 * name asked after.  When it is this game's, the name goes back with its terminator. */
u32 frontend::neta::ipxAnswerName(App* app, ThreadState& ts, u32 state, u32 whom, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    constexpr u32 kStrncmp = 0x4eb390, kNameTag = 0xea737276;
    guest.setDword(guest.frame() + 8, whom);
    const u32 differs = fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.ts(), name, state + 0x26, 0x30);
    if (differs != 0)
        return differs;
    return guest::direct<0x4f60b0, ipxSend>(app, ts, state, whom, kNameTag, name, guest.length(name) + 1);
}

nfs::u32 frontend::netIpxAnswerName(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5a30", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 state = args.eax, whom = args.edx, name = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxAnswerName(app, ts, state, whom, name); });
}

/* sub_49f730: the next one waiting to join, off the ring of 6 at 0x7a01f0, 31 bytes each, to eax. */
u32 frontend::neta::nextPending(App* app, ThreadState& ts, u32 to)
{
    Guest guest(app, ts);
    constexpr u32 kRingAt = 0x7a01e4, kRingCount = 0x7a01ec, kRing = 0x7a01f0;
    constexpr s32 kRingSize = 6;
    copyBytes(guest, to, kRing + guest.dword(kRingAt) * 31, 31);
    const s32 next = s32(guest.dword(kRingAt) + 1);
    guest.setDword(kRingCount, guest.dword(kRingCount) - 1);
    guest.setDword(kRingAt, u32(next % kRingSize));
    return u32(next / kRingSize);
}

nfs::u32 frontend::netNextPending(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f730", 0, 0, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 to = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::nextPending(app, ts, to); });
}

/* sub_49ee50, as a host's lobby closes: no one else may join: its game shut, unless RaceNet's. */
u32 frontend::neta::lobbyClose(App* app, ThreadState& ts)
{
    return lobbyCloseFrom(app, ts, 0);
}

u32 frontend::neta::lobbyCloseFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kSessionIndex = 0x49f1c0, kShutGame = 0x4f4f50;
    u32 eax = h.eax;
    if (guest.word(kIsHost) == 0 || guest.word(kSessionOver) != 0 || guest.dword(0x55e860) == 0 || guest.dword(kSessionStage) != 3)
        return eax;
    if (guest.dword(kLinkKind) != 2)
    {
        eax = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), h, guest.dword(0x55e71c)) * 11;
        if (guest.dword(0x55e738 + eax * 4) != 0)
            eax = (h.eax = got(fe::direct<0x4f4f50, ea::netGameShut>(app, ts, 0x7a0390, 0)));
    }
    guest.setDword(0x55e710, 0);
    guest.setDword(0x55e860, 0);
    return eax;
}

nfs::u32 frontend::netLobbyClose(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ee50", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::lobbyCloseFrom(app, ts, handed); });
}

/* sub_4a5f30: the endpoint of a link's number, eax: 0 for none. */
u32 frontend::neta::endpointOfLink(App* app, ThreadState& ts, u32 link)
{
    Guest guest(app, ts);
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 at = i * kEndpointSize;
        if (guest.byte(kEndpointLink + at) == link)
            return kEndpoints + at;
    }
    return 0;
}

nfs::u32 frontend::netEndpointOfLink(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5f30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 link = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::endpointOfLink(app, ts, link); });
}

/* sub_477f60, a race's loading step: its messages cleared, the controls started, and in a network
 * race the settings exchanged, unless asked to stop along the way. */
u32 frontend::neta::raceLoadStep(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kClearCarSlots = 0x42c660, kStartControls = 0x48f4f0;
    guest::direct<0x4c45d0, clearInputCounts>(app, ts);
    callHeld(guest, h, kClearCarSlots);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
    if (u16(fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts)) == 0)
        callHeld(guest, h, kStartControls);
    const u32 stop = fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
    if (u16(stop) != 0)
        return stop;
    const u32 kind = guest.dword(0x6fd3b8);
    const u32 players = guest.dword(0x6fd3b0);
    if (players == 0 || players == 1 || guest.dword(0x6fd3a0) != 0)
        return kind;
    return guest::direct<0x4c5e80, raceSettings>(app, ts);
}

nfs::u32 frontend::netRaceLoadStep(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477f60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::raceLoadStep(app, ts); });
}

/* sub_49b2b0, packet 2: the host's settings, 0x28 bytes, kept for the lobby to take on. */
u32 frontend::neta::settingsPacket(App* app, ThreadState& ts, u32 packet)
{
    return settingsPacketFrom(app, ts, packet, 0);
}

u32 frontend::neta::settingsPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    if (guest.word(kSessionOver) != 0)
        return h.eax;
    enter(guest, ts, kSetupLock);
    copyBytes(guest, 0x79f8f0, packet, 0x28);
    guest.setWord(kSetupNew, 1);
    return leave(guest, ts, kSetupLock);
}

nfs::u32 frontend::netSettingsPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b2b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx;
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::settingsPacketFrom(app, ts, packet, handed); });
}

/* sub_4f5ac0, a walk over the received queue: eax a packet, edx the kind and sender asked after:
 * one that is theirs is marked for throwing away. */
u32 frontend::neta::markPacket(App* app, ThreadState& ts, u32 packet, u32 asked)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    if (guest.dword(packet + 0x20) == guest.dword(asked))
    {
        const u32 service = guest.dword(asked + 0x18);
        bool theirs = service == 0;
        if (!theirs)
        {
// as the call by address leaves it
            theirs = guest::direct<0x4f5fe0, sameNode>(app, ts, packet + 0xc, asked + 4) != 0;
        }
        if (theirs)
            guest.setDword(packet + 4, 0);
    }
    return 1;
}

nfs::u32 frontend::netMarkPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5ac0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.eax, asked = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::markPacket(app, ts, packet, asked); });
}

/* sub_49ede0, as a host's lobby opens: its game made known, unless RaceNet's. */
u32 frontend::neta::lobbyOpen(App* app, ThreadState& ts)
{
    return lobbyOpenFrom(app, ts, 0);
}

u32 frontend::neta::lobbyOpenFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kSessionIndex = 0x49f1c0;
    u32 eax = h.eax;
    if (guest.word(kIsHost) == 0 || guest.word(kSessionOver) != 0 || guest.dword(0x55e860) != 0 || guest.dword(kSessionStage) != 3)
        return eax;
    if (guest.dword(kLinkKind) != 2)
    {
        eax = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), h, guest.dword(0x55e71c)) * 11;
        if (guest.dword(0x55e738 + eax * 4) != 0)
            eax = guest::direct<0x4f4f70, openGame>(app, ts, 0x7a0390);
    }
    guest.setDword(0x55e860, 1);
    return eax;
}

nfs::u32 frontend::netLobbyOpen(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ede0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::lobbyOpenFrom(app, ts, handed); });
}

/* sub_4f4e60, NET_killservice: eax the record, 0 when NET_ does not have it. */
u32 frontend::neta::killService(App* app, ThreadState& ts, u32 service)
{
    Held h;
    h.eax = service;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kServicesQueue = 0x9f25f0, kQueueRemove = 0x50e150;
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.ts(), h, 0x54d0e0);
    if (callHeld(guest, h, kQueueFindPosition, args({ kServicesQueue, service })) == u32(-1))
        return 0;
    const u32 result = callHeld(guest, h, guest.dword(guest.dword(service + 4) + 0xc), args({ service, service }));
    callHeld(guest, h, kQueueRemove, args({ kServicesQueue }));
    return result;
}

nfs::u32 frontend::netKillService(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4e60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 service = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::killService(app, ts, service); });
}

/* sub_4a69c0: a client's lobby: 1 when the host's list has come since last asked. */
u32 frontend::neta::clientLobby(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    if (guest.word(kSetupDirty) == 0)
        return 0;
    enter(guest, ts, kPlayersLock);
    guest.setWord(kSetupDirty, 0);
    leave(guest, ts, kPlayersLock);
    return 1;
}

nfs::u32 frontend::netClientLobby(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a69c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::clientLobby(app, ts); });
}

/* sub_4f5a80, the match of a walk over the received queue: 0 for the packet asked for, not too old. */
u32 frontend::neta::packetMatch(App* app, ThreadState& ts, u32 packet, u32 asked)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    if (s32(guest.dword(kClock) + 0x10) < s32(guest.dword(packet + 4)) || guest.dword(packet + 0x20) != guest.dword(asked))
        return 1;
// as the call by address leaves it
    const u32 same = guest::direct<0x4f5fe0, sameNode>(app, ts, asked + 4, packet + 0xc);
    return same != 0 ? 1 : 0;
}

nfs::u32 frontend::netPacketMatch(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5a80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 packet = args.eax, asked = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::packetMatch(app, ts, packet, asked); });
}

/* sub_4a6470, an endpoint's call when its machine is lost: the host notes which, on a client the
 * session is over. */
u32 frontend::neta::endpointLost(App* app, ThreadState& ts, u32 endpoint)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    guest.setWord(kSetupDirty, 1);
    if (guest.dword(kSessionStage) == 3 && guest.word(kIsHost) != 0)
    {
        const u32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, ts, endpoint));
        if (s32(machine) > -1)
            directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.ts(), h, machine);
        return 0;
    }
    guest.setWord(kSessionFailed, 1);
    guest.setWord(kSessionOver, 1);
    return 0;
}

nfs::u32 frontend::netEndpointLost(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6470", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 endpoint = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::endpointLost(app, ts, endpoint); });
}

/* sub_4f5ba0, IPX's start: Winsock 1.1 opened; the most a datagram may be, 0x2000 unless it says. */
u32 frontend::neta::ipxStartup(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x198);
    const u32 data = guest.frame();
    if (!os::net::start())
        return 0;
    // The port's start tells of no largest datagram: the word the code reads for it is what its
    // frame holds there already.
    const u16 most = guest.word(data + 0x188);
    return most != 0 ? u32(most) : 0x2000;
}

nfs::u32 frontend::netIpxStartup(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5ba0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxStartup(app, ts); });
}

/* sub_49e320: one more waiting to join: eax who, edx its link. */
u32 frontend::neta::pendingAdd(App* app, ThreadState& ts, u32 who, u32 link)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    constexpr u32 kStartWanted = 0x55e710, kAcceptCount = 0x7a01e0;
    guest.setByte(guest.frame(), u8(link));
    if (guest.dword(kStartWanted) != 0)
        return 0;
    const u32 count = guest.dword(kAcceptCount);
    if (s32(count) >= 7)
        return 0;
    guest.setDword(kAcceptCount, count + 1);
    fe::direct<0x49f6a0, frontend::connect::enqueuePeer>(app, ts, who, guest.frame());
    return 1;
}

nfs::u32 frontend::netPendingAdd(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e320", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 who = args.eax, link = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::pendingAdd(app, ts, who, link); });
}

/* sub_4c4190, a packet of what the host's race says, 0x14 bytes. */
u32 frontend::neta::statusPacket(App* app, ThreadState& ts, u32 packet)
{
    Guest guest(app, ts);
    copyBytes(guest, 0x7d3650, packet, 0x14);
    guest.setWord(0x7a22a2, 1);
    return 0x14;
}

nfs::u32 frontend::netStatusPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4190", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::statusPacket(app, ts, packet); });
}

/* sub_4f5230: NET_'s two lists and its exit, once. */
u32 frontend::neta::servicesReady(App* app, ThreadState& ts)
{
    return servicesReadyFrom(app, ts, 0);
}

u32 frontend::neta::servicesReadyFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kServicesUp = 0x566df4, kCloseServices = 0x4f50c0;
    const u32 up = guest.dword(kServicesUp);
    if (up != 0)
        return h.eax;
    guest::direct<0x50dcd0, netb::qinit>(app, ts, 0x9f2628, up, 0);
    guest::direct<0x50dcd0, netb::qinit>(app, ts, 0x9f260c, 0, 0);
    const u32 result = fe::direct<0x4f2a78, frontend::eab::atexitJump>(app, ts, kCloseServices);
    guest.setDword(kServicesUp, 1);
    return result;
}

nfs::u32 frontend::netServicesReady(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5230", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::servicesReadyFrom(app, ts, handed); });
}

/* sub_4f5fe0: whether two addresses, at edx and ebx, have the same 8 bytes from their +4. */
u32 frontend::neta::sameNode(App* app, ThreadState& ts, u32 first, u32 second)
{
    Guest guest(app, ts);
    const u32 one = first + 4, other = second + 4;
    for (u32 i = 0; i < 8; ++i)
        if (guest.byte(one + i) != guest.byte(other + i))
            return 0;
    return 1;
}

nfs::u32 frontend::netSameNode(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5fe0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 first = args.edx, second = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sameNode(app, ts, first, second); });
}

/* sub_49ec70, each frame: the machines the host has marked as going, 2, dropped and marked gone, 1. */
u32 frontend::neta::dropGone(App* app, ThreadState& ts)
{
    return dropGoneFrom(app, ts, 0);
}

u32 frontend::neta::dropGoneFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    for (u32 machine = 1; s32(machine) < s32(guest.dword(kRacePlayerCount)); ++machine)
    {
        if (guest.word(kMachineGone + machine * 2) != 2)
            continue;
        directKept<0x49ecb0, ea::connectionDropped, 1>(guest.app(), guest.ts(), h, guest.dword(kMachineLinks + machine * 4));
        guest.setWord(kMachineGone + machine * 2, 1);
    }
    return h.eax;
}

nfs::u32 frontend::netDropGone(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ec70", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::dropGoneFrom(app, ts, handed); });
}

/* sub_49fb10: whether dial-up networking's DLL and all its functions are there. */
u32 frontend::neta::rasLoaded(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    for (const u32 place : { 0x7a05c0u, 0x7a05c8u, 0x7a05ccu, 0x7a05d0u, 0x7a05d4u, 0x7a05d8u, 0x7a05dcu })
        if (guest.dword(place) == 0)
            return 0;
    return 1;
}

nfs::u32 frontend::netRasLoaded(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49fb10", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::rasLoaded(app, ts); });
}

/* sub_4f5900: a sockaddr_ipx to everyone: eax the port, edx where it goes. */
u32 frontend::neta::portToSockaddr(App* app, ThreadState& ts, u32 port, u32 to)
{
    Held h;
    h.eax = port; h.edx = to;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kFillBytes = 0x4e0716, kAfIpx = 6;
    guest.setDword(guest.frame(), port);
    guest.setWord(to, kAfIpx);
    fe::direct<kFillBytes, ea::fillBytes>(guest.app(), guest.ts(), to + 2, 4, 0);
    fe::direct<kFillBytes, ea::fillBytes>(guest.app(), guest.ts(), to + 6, 6, 0xff);
    return callHeld(guest, h, kMemmove, args({ guest.frame(), to + 0xc, 2 }));
}

nfs::u32 frontend::netPortToSockaddr(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5900", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 port = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::portToSockaddr(app, ts, port, to); });
}

/* sub_49edb0, the packet of which machines have gone: 16 bytes after its two. */
u32 frontend::neta::gonePacket(App* app, ThreadState& ts, u32 packet)
{
    Guest guest(app, ts);
    copyBytes(guest, kMachineGone, packet + 2, 0x10);
    return 0x10;
}

nfs::u32 frontend::netGonePacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49edb0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::gonePacket(app, ts, packet); });
}

/* sub_4f58c0: NET_'s address made a sockaddr_ipx: eax the address, edx where it goes. */
u32 frontend::neta::toSockaddr(App* app, ThreadState& ts, u32 from, u32 to)
{
    Held h;
    h.eax = from; h.edx = to;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kAfIpx = 6;
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to + 2, from, 4);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to + 6, from + 4, 6);
    const u32 result = callHeld(guest, h, kMemmove, args({ from + 0xa, to + 0xc, 2 }));
    guest.setWord(to, kAfIpx);
    return result;
}

nfs::u32 frontend::netToSockaddr(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f58c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 from = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::toSockaddr(app, ts, from, to); });
}

/* sub_4a64c0, an endpoint's time-out: a host waiting on someone joining past their time, or the
 * game stopping. */
u32 frontend::neta::endpointTimedOut(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (guest.word(kIsHost) != 0 && guest.dword(kSessionStage) == 3
        && s32(guest.dword(kTicks)) > s32(guest.dword(0x55e714)))
        return 1;
    return guest.byte(0x564819) != 0 ? 1 : 0;
}

nfs::u32 frontend::netEndpointTimedOut(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a64c0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::endpointTimedOut(app, ts); });
}

/* sub_4c7f90, packet 12: a car's times, 0xc0 bytes into it; the host passes it on. */
u32 frontend::neta::resultsPacket(App* app, ThreadState& ts, u32 packet)
{
    Held h;
    h.edx = packet;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    const u32 car = guest.dword(kCarsByPlayer + guest.byte(packet + 1) * 4);
    const u32 copied = callHeld(guest, h, kMemmove, args({ packet + 4, car + 0x224, 0xc0 }));
    if (guest.word(kIsHost) == 0)
        return copied;
    return sendLeaving(app, ts, 0, packet, 0xc4);
}

nfs::u32 frontend::netResultsPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7f90", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::resultsPacket(app, ts, packet); });
}

/* sub_4c8310, this machine's controls for its packet, eax where they go: its car's steering cut to
 * an integer, then its pressed keys.  Eax is left as it came. */
u32 frontend::neta::packControls(App* app, ThreadState& ts, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    const u32 player = guest.dword(0x6fd4f4);
    // fld dword, then sub_4dfd56 (__CHP) and fistp: cut towards zero.
    const nfs::Float lateral = nfs::Float(guest.floatAt(guest.dword(kCars + player * 4) + 0x7c0));
    const std::int32_t steering =
        fp::toInteger<nfs::s32>(fe::direct<0x4dfd56, watcom::clib::chop>(app, ts, double(lateral)));
    guest.setWord(out, u16(u32(steering)));
    guest.setWord(out + 2, guest.word(guest.dword(kCars + player * 4) + 8));
    return out;
}

nfs::u32 frontend::netPackControls(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c8310", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 out = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::packControls(app, ts, out); });
}

/* sub_4f5880: a sockaddr_ipx made NET_'s address: eax the sockaddr, edx where it goes. */
u32 frontend::neta::fromSockaddr(App* app, ThreadState& ts, u32 from, u32 to)
{
    Held h;
    h.eax = from; h.edx = to;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to, from + 2, 4);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, to + 4, from + 6, 6);
    return callHeld(guest, h, kMemmove, args({ from + 0xc, to + 0xa, 2 }));
}

nfs::u32 frontend::netFromSockaddr(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 from = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::fromSockaddr(app, ts, from, to); });
}

/* sub_49e2e0: hosting a RaceNet game: the session reset and NPS opened. */
u32 frontend::neta::hostRaceNet(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kResetNetwork = 0x4a6390;
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), h, 1, 1);
    guest.setWord(kIsHost, 1);
    guest.setWord(kThisMachine, 0);
    guest.setWord(kMachineCount, 1);
    return guest::direct<0x49e080, hostStart>(app, ts) != 0 ? 1 : 0;
}

nfs::u32 frontend::netHostRaceNet(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e2e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::hostRaceNet(app, ts); });
}

/* sub_4c81c0, in a network race with one local player on odd ticks: what has come in taken. */
u32 frontend::neta::takeInputsOnTick(App* app, ThreadState& ts, u32 tick)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    const u32 players = guest.dword(0x6fd3b0);
    if (s32(guest.dword(0x6fd3a0)) > 1 || players == 1 || players == 0 || guest.dword(0x6fd50c) == 0 || (tick & 1) == 0)
        return tick;
    return guest::direct<0x4c8250, takeInputs>(app, ts, tick);
}

nfs::u32 frontend::netTakeInputsOnTick(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c81c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 tick = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::takeInputsOnTick(app, ts, tick); });
}

/* sub_4f4c10, at the game's exit: every service NET_ has killed, and its queue freed. */
u32 frontend::neta::exitNet(App* app, ThreadState& ts)
{
    return exitNetFrom(app, ts, 0);
}

u32 frontend::neta::exitNetFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kReady = 0x566de8, kServicesQueue = 0x9f25f0, kQueueFirst = 0x50e470, kQueueFree = 0x50dde0;
    if (guest.dword(kReady) == 0)
        return h.eax;
    for (;;)
    {
        const u32 service = (h.eax = got(fe::direct<0x50e470, ea::queueElementAt>(app, ts, kServicesQueue, 0)));
        if (service == 0)
            break;
        guest::direct<0x4f4e60, killService>(app, ts, service);
    }
    const u32 freed = directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, kServicesQueue);
    guest.setDword(kReady, 0);
    return freed;
}

nfs::u32 frontend::netExit(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4c10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::exitNetFrom(app, ts, handed); });
}

/* sub_4a6440, an endpoint's receive: a packet by its first byte, 0 to 18, to its handler in the
 * table at 0x55ecbc, while the session lasts. */
u32 frontend::neta::dispatchPacket(App* app, ThreadState& ts, u32 from, u32 packet, u32 size)
{
    Held h;
    h.eax = from; h.edx = packet; h.ebx = size;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kHandlers = 0x55ecbc, kPacketKinds = 0x13;
    const u8 kind = guest.byte(packet);
    if (guest.word(kSessionOver) == 0 && kind < kPacketKinds)
    {
        // The handler takes what this one was handed, the kind in ecx.
        callHeld(guest, h, guest.dword(kHandlers + kind * 4), args({ from, packet, size, kind }));
    }
    return 1;
}

nfs::u32 frontend::netDispatchPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6440", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 from = args.eax, packet = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::dispatchPacket(app, ts, from, packet, size); });
}

/* sub_4c7e50: packet 13's first two bytes sent, eax's low byte the player. */
u32 frontend::neta::sendPlayerGone(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    const u32 packet = guest.frame();
    guest.setByte(packet, 0xd);
    guest.setByte(packet + 1, u8(player));
    return fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, 2)->eax;
}

nfs::u32 frontend::netSendPlayerGone(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7e50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendPlayerGone(app, ts, player); });
}

/* sub_49fc50: dial-up networking's DLL let go and its functions forgotten. */
u32 frontend::neta::rasFree(App* app, ThreadState& ts)
{
    return rasFreeFrom(app, ts, 0);
}

u32 frontend::neta::rasFreeFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    constexpr u32 kRasDll = 0x7a05c0;
    const u32 library = guest.dword(kRasDll);
    if (library == 0)
        return h.eax;
    os::release(app, library);
    return callHeld(guest, h, kFillZero, args({ kRasDll, 0x24 }));
}

nfs::u32 frontend::netRasFree(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49fc50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::rasFreeFrom(app, ts, handed); });
}

/* sub_4a65b0, while a client waits for the host: the system let run, and whether it has timed
 * out; with the game stopping it stays here until the other thread is through. */
u32 frontend::neta::hostGone(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kGameQuits = 0x564819;
    fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
    const u32 gone = guest::direct<0x4a64c0, endpointTimedOut>(app, ts);
    if (gone != 0)
    {
        while (guest.byte(kGameQuits) != 0)
            if (app->contextWanted())
                app->yieldContext(ts);
    }
    return gone;
}

nfs::u32 frontend::netHostGone(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a65b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::hostGone(app, ts); });
}

/* sub_4f5850: a service's call to send, by its table: eax the game, ecx the service. */
u32 frontend::neta::sendQueued(App* app, ThreadState& ts, u32 game, u32 first, u32 second, u32 service)
{
    Held h;
    h.eax = game; h.edx = first; h.ebx = second; h.ecx = service;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    if (game != 0)
    {
        guest.setDword(game + 0x48, 0);
        guest.setDword(game + 0x4c, service);
    }
    return callHeld(guest, h, guest.dword(guest.dword(service + 4) + 0x38), args({ service, game, first, second }));
}

nfs::u32 frontend::netSendQueued(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5850", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 game = args.eax, first = args.edx, second = args.ebx, service = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::sendQueued(app, ts, game, first, second, service); });
}

/* sub_49a490: the session's three locks made and the players' records emptied. */
u32 frontend::neta::makeLocks(App* app, ThreadState& ts)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    guest.setDword(kSetupLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    guest.setDword(kPlayersLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    guest.setDword(kBarrierLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts)));
    return callHeld(guest, h, kFillZero, args({ kPlayerRecords, 0x300 }));
}

nfs::u32 frontend::netMakeLocks(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49a490", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::makeLocks(app, ts); });
}

/* sub_4a5ef0, NPS's callback with a packet, on the stack: its link, its data, its size: to that
 * link's endpoint's receive. */
u32 frontend::neta::npsReceived(App* app, ThreadState& ts, u32 /*s0*/, u32 /*s1*/, u32 link, u32 /*s3*/, u32 data, u32 size)
{
    Held h;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    h.edx = (data);  // the data went in edx too, and the call by address leaves it there
    const u32 endpoint = guest::direct<0x4a5f30, endpointOfLink>(app, ts, link);
    h.ecx = (endpoint);
    if (endpoint == 0)
        return 0;
    return callHeld(guest, h, guest.dword(endpoint + 0x20), args({ endpoint, data, size & 0xffff }));
}

nfs::u32 frontend::netNpsReceived(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5ef0", 0, 0, nullptr, 0, fe::kRestoreEbx };
    const u32 word1 = app->getMemory<u32>(ts.sp + 4);
    const u32 word2 = app->getMemory<u32>(ts.sp + 8);
    const u32 link = app->getMemory<u32>(ts.sp + 12);
    const u32 word4 = app->getMemory<u32>(ts.sp + 16);
    const u32 data = app->getMemory<u32>(ts.sp + 20);
    const u32 size = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::npsReceived(app, ts, word1, word2, link, word4, data, size); });
}

/* sub_4f5bf0: Winsock closed: 1 when it did. */
u32 frontend::neta::winsockClose(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    os::net::stop();
    return 1;
}

nfs::u32 frontend::netWinsockClose(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5bf0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::winsockClose(app, ts); });
}

/* sub_4f6010: the socket's own address, 20 bytes, to edx. */
u32 frontend::neta::ipxOwnAddress(App* app, ThreadState& ts, u32 state, u32 to)
{
    Guest guest(app, ts);
    copyBytes(guest, to, state + 0x5c, 20);
    return 1;
}

nfs::u32 frontend::netIpxOwnAddress(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f6010", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 state = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxOwnAddress(app, ts, state, to); });
}

namespace
{

/* NET_'s calls, each to the service's table by its place, what it was called with going on (the
 * service in eax, `in` in edx, ebx and ecx as they came). */
u32 callThroughTable(Guest& guest, u32 service, u32 name, u32 place, u32 edx, u32 ebx, u32 ecx)
{
    Held h;
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.ts(), h, name);
    return callHeld(guest, h, guest.dword(guest.dword(service + 4) + place), args({ service, edx, ebx, ecx }));
}

}  // namespace

u32 frontend::neta::getGames(App* app, ThreadState& ts, u32 service, u32 callback, u32 lookFor, u32 byte)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    return callThroughTable(guest, service, 0x54d0f0, 0x10, callback, lookFor, byte);
}

// sub_4f4eb0, NET_getgames
nfs::u32 frontend::netGetGames(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4eb0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 service = args.eax, callback = args.edx, lookFor = args.ebx, byte = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::getGames(app, ts, service, callback, lookFor, byte); });
}

/* The joiner gets the service in ecx too, as the code has it there. */
u32 frontend::neta::setGame(App* app, ThreadState& ts, u32 service, u32 endpoint, u32 byte, u32 /*kept*/)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    return callThroughTable(guest, service, 0x54d100, 0x14, endpoint, byte, service);
}

// sub_4f4ed0, NET_setgame
nfs::u32 frontend::netSetGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4ed0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 service = args.eax, endpoint = args.edx, byte = args.ebx, kept = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::setGame(app, ts, service, endpoint, byte, kept); });
}

u32 frontend::neta::newGame(App* app, ThreadState& ts, u32 service, u32 name, u32 byte, u32 handler)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    return callThroughTable(guest, service, 0x54d11c, 0x1c, name, byte, handler);
}

// sub_4f4f10, NET_newgame
nfs::u32 frontend::netNewGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4f10", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 service = args.eax, name = args.edx, byte = args.ebx, handler = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::newGame(app, ts, service, name, byte, handler); });
}

u32 frontend::neta::openGame(App* app, ThreadState& ts, u32 service)
{
    Held h;
    h.eax = service;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.ts(), h, 0x54d148, service);
    const u32 table = guest.dword(service + 4);
    h.ecx = (table);
    return callHeld(guest, h, guest.dword(table + 0x28), args({ service, service }));
}

// sub_4f4f70, NET_opengame
nfs::u32 frontend::netOpenGame(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f4f70", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 service = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::openGame(app, ts, service); });
}

/* sub_4f5050: a service's poll through its table, when it has one; eax the service's record, the
 * three words in edx, ebx and ecx as they came. */
u32 frontend::neta::servicePollCall(App* app, ThreadState& ts, u32 service, u32 first, u32 second, u32 third)
{
    Held h;
    h.eax = service; h.edx = first; h.ebx = second; h.ecx = third;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 table = guest.dword(service + 8);
    if (guest.dword(table + 0x38) == 0)
        return 0;
    return callHeld(guest, h, guest.dword(table + 0x38), args({ guest.dword(service + 0xc), first, second, third }));
}

nfs::u32 frontend::netServicePollCall(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5050", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const u32 service = args.eax, first = args.edx, second = args.ebx, third = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::servicePollCall(app, ts, service, first, second, third); });
}

/* sub_4f56e0: whether a game's service is up, and what it can take: 0x566e20's word when it is. */
u32 frontend::neta::serviceRoom(App* app, ThreadState& ts, u32 game)
{
    Held h;
    h.eax = game;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 service = guest.dword(game + 0x4c);
    const u32 table = guest.dword(service + 8);
    const u32 up = callHeld(guest, h, guest.dword(table + 0x4c), args({ guest.dword(service + 0xc), table }));
    return up != 0 ? u32(guest.word(0x566e20)) : 0;
}

nfs::u32 frontend::netServiceRoom(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f56e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 game = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceRoom(app, ts, game); });
}

/* sub_4f5700: a game's service asked through its table's +0x50. */
u32 frontend::neta::serviceAsk(App* app, ThreadState& ts, u32 game)
{
    Held h;
    h.eax = game;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 service = guest.dword(game + 0x4c);
    const u32 table = guest.dword(service + 8);
    return callHeld(guest, h, guest.dword(table + 0x50), args({ guest.dword(service + 0xc), table }));
}

nfs::u32 frontend::netServiceAsk(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5700", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 game = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::serviceAsk(app, ts, game); });
}

/* sub_4f5fa0: the game's name kept, 0x30 bytes at most, and given to the socket's key: eax the
 * state, edx the name. */
u32 frontend::neta::ipxSetName(App* app, ThreadState& ts, u32 state, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), state + 0x26, name, 0x30);
    return guest::direct<0x5127d0, netb::ipxBroadcastOpen>(app, ts, guest.dword(state + 0x58));
}

nfs::u32 frontend::netIpxSetName(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5fa0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 state = args.eax, name = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxSetName(app, ts, state, name); });
}

/* sub_477fd0, a race's loading step: its music prepared, outside a replay. */
u32 frontend::neta::raceMusicStep(App* app, ThreadState& ts)
{
    return raceMusicStepFrom(app, ts, 0);
}

u32 frontend::neta::raceMusicStepFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    constexpr u32 kRacePrepareMusic = 0x410030;
    if (guest.dword(0x6fbb54) != 0)
        return h.eax;
    callHeld(guest, h, kRacePrepareMusic, args({ guest.dword(0x6fd4ac) }));
    return fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
}

nfs::u32 frontend::netRaceMusicStep(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477fd0", 0, 0, nullptr, 0, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::raceMusicStepFrom(app, ts, handed); });
}

/* sub_49f680: whether nobody waits to join. */
u32 frontend::neta::pendingEmpty(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(0x7a01ec) != 0 ? 0 : 1;
}

nfs::u32 frontend::netPendingEmpty(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f680", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::pendingEmpty(app, ts); });
}

/* sub_4f5f50: the socket's port number, in the machine's order, out of the state's network order. */
u32 frontend::neta::ipxPort(App* app, ThreadState& ts, u32 state)
{
    Guest guest(app, ts);
    const u32 value = guest.dword(state + 0x24);
    const u32 swapped = (value >> 24) | ((value >> 8) & 0xff00u) | ((value << 8) & 0xff0000u) | (value << 24);
    return swapped >> 16;
}

nfs::u32 frontend::netIpxPort(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5f50", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 state = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxPort(app, ts, state); });
}

/* sub_4c45d0: the controls' two counts cleared. */
u32 frontend::neta::clearInputCounts(App* app, ThreadState& ts)
{
    return clearInputCountsFrom(app, ts, 0);
}

u32 frontend::neta::clearInputCountsFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    guest.setDword(0x7d368c, 0);
    guest.setDword(0x7d3690, 0);
    return h.eax;
}

nfs::u32 frontend::netClearInputCounts(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c45d0", 0, 0, nullptr, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::clearInputCountsFrom(app, ts, handed); });
}

/* sub_4c8350, a packet's controls unpacked, eax the packed, edx the room: the steering as a float,
 * the keys pressed zero extended. */
u32 frontend::neta::unpackControls(App* app, ThreadState& ts, u32 packed, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    guest.setFloat(out, float(nfs::Float(nfs::s32(std::int16_t(guest.word(packed))))));
    const u32 keys = guest.word(packed + 2);
    guest.setDword(out + 4, keys);
    return keys;
}

nfs::u32 frontend::netUnpackControls(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c8350", 0, 0, nullptr, 0 };
    const u32 packed = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::unpackControls(app, ts, packed, out); });
}

/* sub_4f5110, the low timer's poll: NET_'s services walked. */
u32 frontend::neta::pollServices(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    constexpr u32 kPollOne = 0x4f50f0;
    return guest::direct<0x50e8e0, netb::qiterate>(app, ts, 0x9f2628, kPollOne, 0);
}

nfs::u32 frontend::netPollServices(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5110", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::pollServices(app, ts); });
}

/* sub_4f56c0: whether a game is listed and its service still has any. */
u32 frontend::neta::gameListed(App* app, ThreadState& ts, u32 game)
{
    Guest guest(app, ts);
    if (guest.dword(game + 0x48) != 0 && guest.dword(guest.dword(game + 0x4c) + 0x9c) != 0)
        return 1;
    return 0;
}

nfs::u32 frontend::netGameListed(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f56c0", 0, 0, nullptr, 0 };
    const u32 game = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::gameListed(app, ts, game); });
}

/* sub_4f5f70: a key asked of the socket's library: eax the state, edx the room. */
u32 frontend::neta::ipxKey(App* app, ThreadState& ts, u32 state, u32 room)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    return guest::direct<0x512c80, netb::ipxStartServer>(app, ts, state, state + 0x5c, room);
}

nfs::u32 frontend::netIpxKey(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5f70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 state = args.eax, room = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxKey(app, ts, state, room); });
}

namespace
{

/* sub_49b500 and sub_49b520: a chunk of a car's file: past its kind byte, the packet and its size. */
u32 carFileChunk(Guest& guest, u32 packet, u32 size)
{
    Held h;
    constexpr u32 kFileDone = 0x55e5bc, kReceiveChunk = 0x49b5f0;
    return callHeld(guest, h, kReceiveChunk, args({ kFileDone, packet + 1, size - 1 }));
}

}  // namespace

u32 frontend::neta::carChunkToHost(App* app, ThreadState& ts, u32 packet, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return carFileChunk(guest, packet, size);
}

nfs::u32 frontend::netCarChunkToHost(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b500", 0, 0, nullptr, 0, 0 };
    const u32 packet = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::carChunkToHost(app, ts, packet, size); });
}

u32 frontend::neta::carChunkToClient(App* app, ThreadState& ts, u32 packet, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return carFileChunk(guest, packet, size);
}

nfs::u32 frontend::netCarChunkToClient(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49b520", 0, 0, nullptr, 0, 0 };
    const u32 packet = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::carChunkToClient(app, ts, packet, size); });
}

/* sub_4a6410: an endpoint's handlers: lost, receive and time-out. */
u32 frontend::neta::endpointHandlers(App* app, ThreadState& ts, u32 endpoint)
{
    Guest guest(app, ts);
    guest.setDword(endpoint + 0x18, 0x4a6470);
    guest.setDword(endpoint + 0x20, 0x4a6440);
    guest.setDword(endpoint + 0x1c, 0);
    guest.setDword(endpoint + 0x14, 0x4a64c0);
    return endpoint;
}

nfs::u32 frontend::netEndpointHandlers(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6410", 0, 0, nullptr, 0 };
    const u32 endpoint = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::endpointHandlers(app, ts, endpoint); });
}

/* sub_4a6df0, packet 0: the host's version, 0x28 bytes after its kind. */
u32 frontend::neta::versionPacket(App* app, ThreadState& ts, u32 packet)
{
    Held h;
    h.edx = packet;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return callHeld(guest, h, kMemmove, args({ packet + 1, 0x55ed08, 0x28 }));
}

nfs::u32 frontend::netVersionPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6df0", 0, 0, nullptr, 0, 0 };
    const u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::versionPacket(app, ts, packet); });
}

/* sub_4c7e80, the packet that a player has left: its car marked as gone. */
u32 frontend::neta::playerLeftPacket(App* app, ThreadState& ts, u32 packet)
{
    return playerLeftPacketFrom(app, ts, packet, 0);
}

u32 frontend::neta::playerLeftPacketFrom(App* app, ThreadState& ts, u32 packet, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    const u32 car = guest.dword(kCarsByPlayer + guest.byte(packet + 1) * 4);
    guest.setDword(car + 0x2c4, 3);
    return h.eax;
}

nfs::u32 frontend::netPlayerLeftPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7e80", 0, 0, nullptr, 0, 0 };
    const u32 packet = args.edx;
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::playerLeftPacketFrom(app, ts, packet, handed); });
}

/* sub_4f50c0, at the game's exit: the services' connections undone and NET_'s two lists freed. */
u32 frontend::neta::closeServices(App* app, ThreadState& ts)
{
    return closeServicesFrom(app, ts, 0);
}

u32 frontend::neta::closeServicesFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    constexpr u32 kConnRestore = 0x4f5280, kQueueFree = 0x50dde0;
    if (guest.dword(0x566df4) == 0)
        return h.eax;
    directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), h, handed);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, 0x9f2628);
    return directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.ts(), h, 0x9f260c);
}

nfs::u32 frontend::netCloseServices(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f50c0", 0, 0, nullptr, 0, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::closeServicesFrom(app, ts, handed); });
}

/* sub_4f50f0: one of NET_'s services polled when its table has a poll. */
u32 frontend::neta::pollOne(App* app, ThreadState& ts, u32 service)
{
    Held h;
    h.eax = service;
    Guest guest(app, ts);
    const u32 table = guest.dword(service + 4);
    h.edx = (table);
    if (guest.dword(table + 0xc) != 0)
        callHeld(guest, h, guest.dword(table + 0xc));
    return 1;
}

nfs::u32 frontend::netPollOne(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f50f0", 0, 0, nullptr, 0, 0 };
    const u32 service = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::pollOne(app, ts, service); });
}

/* sub_4a5af0: this machine's link, the low byte of the NPS state's first word; the rest of eax
 * as it came. */
u32 frontend::neta::thisLink(App* app, ThreadState& ts)
{
    return thisLinkFrom(app, ts, 0);
}

u32 frontend::neta::thisLinkFrom(App* app, ThreadState& ts, u32 handed)
{
    Held h;
    h.eax = handed;
    Guest guest(app, ts);
    return (h.eax & 0xffffff00u) | guest.byte(0x7a20a0);
}

nfs::u32 frontend::netThisLink(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a5af0", 0, 0, nullptr, 0 };
    const u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::thisLinkFrom(app, ts, handed); });
}

/* sub_4f5fc0: the game's name forgotten. */
u32 frontend::neta::ipxForgetName(App* app, ThreadState& ts, u32 state)
{
    Guest guest(app, ts);
    guest.setByte(state + 0x26, 0);
    return 1;
}

nfs::u32 frontend::netIpxForgetName(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5fc0", 0, 0, nullptr, 0 };
    const u32 state = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxForgetName(app, ts, state); });
}

/* sub_4f5fd0: nothing to do, a service's table's "yes". */
u32 frontend::neta::answerYes(App* app, ThreadState& ts)
{
    return 1;
}

nfs::u32 frontend::netAnswerYes(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5fd0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::answerYes(app, ts); });
}

/* sub_4f50b0: nothing to do, a record's default for what it was not given. */
u32 frontend::neta::answerNo(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::netAnswerNo(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f50b0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::answerNo(app, ts); });
}

/* sub_4f5f90: on to sub_512d70, a service's table's jump. */
u32 frontend::neta::ipxJump(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest::direct<0x512d70, netb::ipxStopServer>(app, ts);
}

nfs::u32 frontend::netIpxJump(App* app, ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f5f90", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return neta::ipxJump(app, ts); });
}

}  // namespace nfs3hp
