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
#include "native_frontend_connect.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_loada.h"
#include "native_frontend_netb.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_load_cardb.h"
#include "native_windows.h"
#include "host_threads.h"
#include "host_window.h"
#include "port.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_fields.h"
#include "port_snd_stream_calls.h"
#include <nfs3hp.h>
#include <fpu.h>
#include "host_system.h"
#include "host_net.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

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
 * kept.  Windows' functions, Winsock's through the import table's thunks, are the port's own,
 * called as C++ (in the checks through the thunks and the import table's slots).
 *
 * Plan step 5: each function a typed core in frontend::neta (native_frontend_neta.h), in a frame of
 * its own where the game's function has one (fe::Frame), and an adapter under the game's name for
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
bool commOpenTrace(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

using fe::args;
using fe::Guest;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

using App = win32::WinApplication;
using CPU = x86::CPU;

/* ---------------------------------------------------------------------------
 * What these functions share
 * ------------------------------------------------------------------------- */

// The game's clocks.
constexpr reg32 kClock = 0x79c1a0, kTicks = 0x79c1a4;

// The session.  The words the original reads as `dword [a - 2] >> 16` are named at a.
constexpr reg32 kPacketSetup = 0x7a2214;     // the setup packet, 0x7c bytes: its kind first
constexpr reg32 kSetupNames = 0x7a2215;      // 11 bytes a machine
constexpr reg32 kSetupLinks = 0x7a226d;      // a byte a machine
constexpr reg32 kMachineCount = 0x7a2276;    // int16
constexpr reg32 kThisMachine = 0x7a2278;     // int16, then the rest of the setup's machine numbers
constexpr reg32 kMachineNumbers = 0x7a2278;  // int16[9]
constexpr reg32 kSetupOther = 0x7a228c;
constexpr reg32 kSetupDirty = 0x7a2296;      // int16
constexpr reg32 kSetupNew = 0x7a2298;        // int16
constexpr reg32 kPlayersNew = 0x7a229a;      // int16
constexpr reg32 kHostBarrier = 0x7a229c;     // int16: the barrier the host has reached, for a client
constexpr reg32 kMachineLinks = 0x7a21f0;    // endpoint*[8]
constexpr reg32 kMachineBarriers = 0x7a22ba; // int16[8]: the barrier each machine has reached
constexpr reg32 kMachineGone = 0x7a22ca;     // int16[8]
constexpr reg32 kIsHost = 0x7a22da;          // int16
constexpr reg32 kSessionOver = 0x7a22dc;     // int16
constexpr reg32 kSessionFailed = 0x7a22de;   // int16
constexpr reg32 kSessionStage = 0x7a22e0;
constexpr reg32 kLinkKind = 0x7a22e4;        // 2 for RaceNet
constexpr reg32 kEndpoints = 0x7a22e8;       // 0x6c bytes each
constexpr reg32 kEndpointSize = 0x6c;
constexpr reg32 kEndpointOffered = 0x7a22f8;
constexpr reg32 kEndpointUsed = 0x7a230c;
constexpr reg32 kEndpointLink = 0x7a231c;
constexpr reg32 kEndpointNames = 0x7a25dc;   // 11 bytes each
constexpr reg32 kNameSize = 11;

// The locks' handles.
constexpr reg32 kBarrierLock = 0x55ecb8, kPlayersLock = 0x55ecb4, kSetupLock = 0x55e5b8;
constexpr reg32 kInputLock = 0x560114;

// Imports, by their slots.
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;

// The game's functions, by address.
constexpr reg32 kError = 0x401010;
constexpr reg32 kClearMessages = 0x42d800, kOpenMessages = 0x42d790;
constexpr reg32 kFindCarById = 0x439990, kPlayerToListedCar = 0x439a10;
constexpr reg32 kBeginCarSend = 0x49b800;
constexpr reg32 kDropMachine = 0x49ecb0, kAnnounceMachineLeft = 0x49ecf0;
constexpr reg32 kCloseNetwork = 0x4a65e0, kPlayerMachine = 0x4a6690;
constexpr reg32 kCreateThread = 0x4df7a0;
constexpr reg32 kMemset = 0x4e0640, kFillZero = 0x4e070c, kStrncpy = 0x4e0e30;
constexpr reg32 kRunSystemTasks = 0x4e7630;
constexpr reg32 kMemmove = 0x4ea4f0;
constexpr reg32 kStrcmp = 0x4ee310, kToUpper = 0x4eede0;

// This group's own, called where they are.
constexpr reg32 kNpsDestroyed = 0x4a5e50, kNpsReceived = 0x4a5ef0;
constexpr reg32 kIpxThread = 0x4f5b10;

// Winsock, through the thunks that jump to its imports.
constexpr reg32 kWsaGetLastError = 0x533e38, kGetSockName = 0x533e3e, kBind = 0x533e44;
constexpr reg32 kHtons = 0x533e4a, kIoctlSocket = 0x533e50, kSetSockOpt = 0x533e56;
constexpr reg32 kCloseSocket = 0x533ea4, kSocket = 0x533eaa, kWsaStartup = 0x533eb0, kWsaCleanup = 0x533e9e;

// How much the network's library logs: 4 and above, its events.
constexpr reg32 kLogLevel = 0x566de0;
constexpr sreg32 kEnoughLog = 4;

reg32 signedWord(const Guest& guest, reg32 address)
{
    return reg32(sreg32(std::int16_t(guest.word(address))));
}

reg32 widened(reg32 value)
{
    return reg32(sreg32(std::int16_t(value)));
}

/* rep movsd forwards, then rep movsb for the bytes left: the inlined memcpy. */
void copyBytes(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    reg32 at = 0;
    for (reg32 i = 0; i < count >> 2; ++i, at += 4)
        guest.setDword(to + at, guest.dword(from + at));
    for (reg32 i = 0; i < (count & 3); ++i, ++at)
        guest.setByte(to + at, guest.byte(from + at));
}

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its import's slot.  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& guest, CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* The same for one the code calls through its import thunk (Winsock's): in the check by the
 * thunk's address. */
template <reg32 Thunk, auto Function, typename... Words>
reg32 thunked(Guest& guest, CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::thunked<Thunk, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

void enter(Guest& guest, CPU& cpu, reg32 lock)
{
    windows<kEnterCriticalSection, &host::enterLock>(guest, cpu, guest.dword(lock));
}

reg32 leave(Guest& guest, CPU& cpu, reg32 lock)
{
    return windows<kLeaveCriticalSection, &host::leaveLock>(guest, cpu, guest.dword(lock));
}

/* sub_4a60e0 called as C++ for a caller whose exit shows what it leaves: the packet sent, edx and
 * ebx left as its code leaves them (ea::Regs). */
reg32 sendLeaving(App* app, CPU& cpu, reg32 connection, reg32 packet, reg32 length)
{
    const std::optional<ea::Regs> sent = fe::direct<0x4a60e0, ea::packetSent>(app, cpu, connection, packet, length);
    cpu.edx = sent->edx;
    cpu.ebx = sent->ebx;
    return sent->eax;
}

/* The record every machine's player has at 0x79f5f0. */
constexpr reg32 kPlayerRecords = 0x79f5f0, kPlayerRecordSize = 0x60;

/* The byte fields of the host's settings, as the setup packet holds them, and the
 * settings they are. */
struct SettingField
{
    reg32 packed, setting;
};
constexpr SettingField kSettingFields[] = {
    { 0x79f8f1, 0x6fd3b8 }, { 0x79f8f2, 0x6fd4ac }, { 0x79f8f3, 0x6fd504 }, { 0x79f8f4, 0x6fd4b4 },
    { 0x79f8f5, 0x6fd4b0 }, { 0x79f8f6, 0x6fd4d4 }, { 0x79f908, 0x6fd4d0 }, { 0x79f909, 0x6fd4e8 },
    { 0x79f90a, 0x6fd4c4 }, { 0x79f90b, 0x6fd4cc }, { 0x79f90c, 0x6fd4c8 }, { 0x79f90d, 0x6fd4c0 },
};
constexpr reg32 kSettingTrackPacked = 0x79f910, kSettingTrack = 0x6fd3b4;
constexpr reg32 kSettingLastPacked = 0x79f914, kSettingLast = 0x6fd470;

// The player's own settings.
constexpr reg32 kPlayerCar = 0x6fd2bc, kPlayerColour = 0x6fd2d4, kPlayerName = 0x6fd2fc, kPlayerColours = 0x6fd307;
constexpr reg32 kPlayerTransmission = 0x6fd508;

// The player's record about to go out, a copy of which is kept in 0x79f5f0.
constexpr reg32 kMyRecord = 0x79f918;

// The race's players, in the settings block: 0x6c bytes each.
constexpr reg32 kRacePlayers = 0x6fd52c, kRacePlayerSize = 0x6c;
constexpr reg32 kRacePlayerCount = 0x6fd514;

// The cars in a race and the flag sub_4c5720 raises on each.
constexpr reg32 kCarCount = 0x5efda4, kCars = 0x5efa48, kCarsByPlayer = 0x5efac8;

void markCarsLeaving(Guest& guest)
{
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
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
reg32 frontend::neta::raceQuitVote(App* app, CPU& cpu, reg32 vote)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x30);
    constexpr reg32 kPlayers = 0x6fd3b0, kSplitCount = 0x6fd3a0, kKeyRepeat = 0x7d36dc, kLastKey = 0x7d36e0;
    constexpr reg32 kHostQuit = 0x7d36c4, kRaceQuits = 0x56010c;
    constexpr reg32 kWaitSeconds = 20;
    constexpr reg32 kAgreed = 9, kGoOn = 10, kQuit = 11, kRestart = 0x20;
    const reg32 players = guest.dword(kPlayers);
    if (players == 0 || players == 1 || sreg32(guest.dword(kSplitCount)) > 1)
        return vote;

    const reg32 ticks = guest.dword(kTicks);
    // The game's code hands it the ticks in eax, which it does not read.
    reg32 key = fe::direct<0x4cc970, frontend::glue::nextKey>(app, cpu) & 0xffff;
    if (sreg32(key) < 0xff)
        key = fe::direct<kToUpper, watcom::clib::toupper>(guest.app(), guest.cpu(), key);
    const bool sameKey = key == guest.dword(kLastKey);
    if (key != 0 && (!sameKey || sreg32(ticks) > sreg32(guest.dword(kKeyRepeat))))
        guest.setDword(kKeyRepeat, ticks + (sameKey ? 5 : 15));
    else
        guest.setDword(kKeyRepeat, 0);
    guest.setDword(kLastKey, key);

    const auto answerGoOn = [&]() -> reg32 {
        guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kGoOn);
        guest.call(kClearMessages);
        return 2;
    };
    const auto answerQuit = [&]() -> reg32 {
        markCarsLeaving(guest);
        guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kQuit);
        guest.call(kClearMessages);
        return 3;
    };
    const auto answerRestart = [&]() -> reg32 {
        markCarsLeaving(guest);
        guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kRestart);
        guest.call(kClearMessages);
        return 4;
    };

    if (guest.word(kIsHost) != 0)
    {
        // What each machine has said to the host: 11 is that it is going.
        for (reg32 machine = 1; sreg32(machine) < sreg32(guest.dword(kRacePlayerCount)); ++machine)
        {
            if (guest.dword(kHostQuit) != 0)
            {
                vote = 1;
                break;
            }
            const reg32 barrier = kMachineBarriers + machine * 2;
            const reg16 said = guest.word(barrier);
            if (said == 0)
                continue;
            const bool going = said == kQuit;
            if (going)
            {
                if (guest.dword(kLinkKind) != 0)
                    directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.cpu(), machine);
                else
                    vote = 3;
            }
            enter(guest, cpu, kBarrierLock);
            if ((guest.word(barrier) == kQuit) == going)
                guest.setWord(barrier, 0);
            leave(guest, cpu, kBarrierLock);
        }
        switch (vote)
        {
        case 1:
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kAgreed);
            guest.setDword(kRaceQuits, 0);
            guest.setDword(kHostQuit, 0);
            guest.call(kClearMessages);
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, 1);
            return 1;
        case 2:
            return answerGoOn();
        case 3:
            return answerQuit();
        case 4:
            return answerRestart();
        case 5:
            markCarsLeaving(guest);
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kQuit);
            guest.call(kClearMessages);
            guest.setWord(kSessionOver, 1);
            return 5;
        default:
            return 0;
        }
    }

    if (vote != 5)
    {
        switch (reg32(guest.word(kHostBarrier)))
        {
        case kAgreed:
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, kAgreed);
            guest.setDword(kRaceQuits, 0);
            guest.call(kClearMessages);
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, 1);
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
    constexpr reg32 kQuitMessage = 8, kMessageBytes = 12;
    if (vote == 1)
    {
        const reg32 message = guest.frame();
        guest.setByte(message, kQuitMessage);
        guest.setDword(message + 4, vote);
        fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, message, kMessageBytes);
        return 0;
    }
    if (vote == 3 || vote == 4 || vote == 5)
    {
        const reg32 message = guest.frame() + 0xc;
        guest.setByte(message, kQuitMessage);
        guest.setDword(message + 4, 3);
        fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, message, kMessageBytes);
        if (guest.dword(kLinkKind) != 0 || vote == 5)
        {
            guest.setWord(kSessionFailed, 1);
            guest.setWord(kSessionOver, 1);
            return vote;
        }
    }
    return 0;
}

void frontend::netRaceQuitVote(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c5720", 0, 0, nullptr, 0 };
    const reg32 vote = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::raceQuitVote(app, cpu, vote); });
}

/* sub_49a520, each pass of a network lobby: 1 when anything has changed that the lobby's
 * screen shows.  This machine's car against the copy the others were last sent, for the
 * host its settings too, what came in; a client takes on the host's settings here. */
reg32 frontend::neta::lobbyChanged(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    constexpr reg32 kPlayersChanged = 0x7a22a8;
    const reg32 joined = guest::direct<0x49eae0, hostLobby>(app, cpu);
    const reg32 colours = guest.frame() + 4;
    const reg32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), signedWord(guest, kThisMachine)));
    reg32 changed = 0;
    if (machine != reg32(-1))
    {
        fe::direct<0x472360, port::fields::customColoursRefreshed>(app, cpu, cpu.esp - 4, cpu.eax);
        for (reg32 i = 0; i < 4; ++i)
            guest.setByte(colours + i, guest.byte(kPlayerColours + i));
        const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, guest.dword(kPlayerCar));
        const reg32 mine = machine * kPlayerRecordSize;
        const bool same = guest.byte(0x79f92c) == guest.dword(kPlayerCar)
            && guest.byte(0x79f92d) == guest.dword(kPlayerColour)
            && guest.byte(0x79f920) == guest.dword(kPlayerTransmission)
            && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), 0x79f921, kPlayerName) == 0
            && joined == 0
            && guest.byte(mine + 0x79f604) == guest.dword(kPlayerCar)
            && guest.byte(mine + 0x79f605) == guest.dword(kPlayerColour)
            && guest.byte(mine + 0x79f5f8) == guest.dword(kPlayerTransmission)
            && guest.dword(colours) == guest.dword(0x79f961)
            && guest.dword(0x79f968) == guest.dword(record + 0x380);
        if (!same)
        {
            guest::direct<0x49a8e0, sendMyCar>(app, cpu);
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
            guest::direct<0x49aa80, sendSettings>(app, cpu);
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
        enter(guest, cpu, kPlayersLock);
        guest.setWord(kPlayersNew, 0);
        leave(guest, cpu, kPlayersLock);
        changed = 1;
    }
    if (guest.word(kIsHost) == 0 && guest.word(kSetupNew) != 0)
    {
        enter(guest, cpu, kSetupLock);
        guest.setWord(kSetupNew, 0);
        leave(guest, cpu, kSetupLock);
        for (const SettingField& field : kSettingFields)
            guest.setDword(field.setting, guest.byte(field.packed));
        guest.setDword(kSettingTrack, guest.dword(kSettingTrackPacked));
        guest.setDword(kSettingLast, guest.byte(kSettingLastPacked));
        // A car of the machines' own: found again by its id, past the four colours' places.
        constexpr reg32 kOpponentCar = 0x6fd4d4;
        if (sreg32(guest.dword(kOpponentCar)) >= 4)
            guest.setDword(kOpponentCar, fe::direct<0x439990, load::placeById>(app, cpu, 0x79f8f8) + 4);
        changed = 1;
    }
    return changed;
}

void frontend::netLobbyChanged(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49a520", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::lobbyChanged(app, cpu); });
}

/* sub_4f5c10: opens a game's IPX socket and its thread, eax the port, edx a flag and ebx what
 * goes with it.  The state, 0x140a8 bytes, then the free list of 256 packets of 0x140 bytes,
 * the socket with broadcast and big buffers, bound and asked its own address; the thread
 * clears 0x9f3650 when it runs.  0 when anything fails. */
reg32 frontend::neta::ipxOpen(App* app, CPU& cpu, reg32 portNumber, reg32 flagValue, reg32 withFlag)
{
    commOpenTrace(app, cpu);
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x38);
    constexpr reg32 kThreadStarting = 0x9f3650;
    constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
    constexpr reg32 kStateBytes = 0x140a8, kPacketBytes = 0x140, kPacketsBytes = 0x14000;
    constexpr reg32 kAfIpx = 6, kSockDgram = 2, kNsProtoIpx = 1000, kSolSocket = 0xffff;
    constexpr reg32 kSoBroadcast = 0x20, kIpxReceiveBroadcast = 0x400f, kFionBio = 0x8004667e, kSockaddrBytes = 0xe;
    const reg32 frame = guest.frame();
    const reg32 option = frame + 0x14, name = frame, port = frame + 0x10, nameBytes = frame + 0x18;
    guest.setDword(port, portNumber);
    guest.setDword(frame + 0x24, flagValue);
    guest.setDword(frame + 0x20, withFlag);
    guest.setDword(kWhereFile, 0x54d21c);
    guest.setDword(kWhereLine, 0x10a);
    guest.setDword(kWhereFunction, 0x54d228);

    const reg32 state = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54d238, kStateBytes, guest.dword(0x5643f4)));
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), state, kStateBytes);
    const reg32 freeQueue = state + 0x70;
    guest::direct<0x50dcd0, netb::qinit>(app, cpu, freeQueue, 0, 0);
    guest::direct<0x50dcd0, netb::qinit>(app, cpu, state + 0x8c, 0, 0);
    const reg32 packets = state + 0xa8;
    for (reg32 packet = packets; packet != packets + kPacketsBytes; packet += kPacketBytes)
        guest::direct<0x50de50, netb::qpush>(app, cpu, freeQueue, packet);
    guest.leaveEbx(freeQueue);  // as the calls by address left it: the queue went in ebx too

    const reg32 socket = thunked<kSocket, &host::socketOpen>(guest, cpu, kAfIpx, kSockDgram, kNsProtoIpx);
    guest.setDword(state + 0x20, 1);
    guest.setDword(state + 0x58, socket);
    const reg32 flag = guest.dword(frame + 0x24);
    guest.setDword(state + 4, flag);
    if (flag != 0)
        guest.setDword(state, guest.dword(frame + 0x20));
    if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d228, port, state + 0x58, 4, 1, 0);

    reg32 thread = 0;
    if (guest.dword(state + 0x58) != reg32(-1))
    {
        guest.setDword(option, 1);
        guest.leaveEbx(guest.dword(state + 0x58));
        thunked<kSetSockOpt, &host::socketSetOption>(guest, cpu, guest.dword(state + 0x58), kSolSocket,
                                                          kSoBroadcast, option, 4);
        guest.setDword(option, 0);
        thunked<kSetSockOpt, &host::socketSetOption>(guest, cpu, guest.dword(state + 0x58), kSolSocket,
                                                          kIpxReceiveBroadcast, option, 4);
        thunked<kSetSockOpt, &host::socketSetOption>(guest, cpu, guest.dword(state + 0x58), kNsProtoIpx,
                                                          kIpxReceiveBroadcast, option, 4);
        guest.setDword(option, 0);
        guest.leaveEbx(guest.dword(state + 0x58));
        thunked<kIoctlSocket, &host::socketControl>(guest, cpu, guest.dword(state + 0x58), kFionBio, option);

        guest.call(kFillZero, args({ name, kSockaddrBytes }));
        const reg32 hostPort = guest.word(port);
        guest.setWord(name, kAfIpx);
        cpu.eax = hostPort;  // as the code has it at the call
        guest.setWord(name + 0xc, reg16(thunked<kHtons, &host::byteSwap16>(guest, cpu, hostPort)));
        if (thunked<kBind, &host::socketBind>(guest, cpu, guest.dword(state + 0x58), name, kSockaddrBytes) == 0)
        {
            guest.setDword(nameBytes, kSockaddrBytes);
            if (thunked<kGetSockName, &host::socketName>(guest, cpu, guest.dword(state + 0x58), name,
                                                                    nameBytes) == 0
                && guest.dword(nameBytes) == kSockaddrBytes)
            {
                guest::direct<0x4f5880, fromSockaddr>(app, cpu, name, state + 0x5c);
                const reg32 boundPort = guest.dword(name + 0xc);
                guest.setDword(kThreadStarting, state);
                guest.setWord(state + 0x24, reg16(boundPort));
                thread = guest.call(kCreateThread, args({ kIpxThread, 0, 2 }, { state + 8 }));
                while (thread != 0 && guest.dword(kThreadStarting) != 0)
                    fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
            }
        }
    }
    if (thread != 0)
        return state;

    if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d240, 0, 0, 0, 1, 0);
    const reg32 openSocket = guest.dword(state + 0x58);
    if (openSocket != reg32(-1)
        && thunked<kCloseSocket, &host::socketClose>(guest, cpu, openSocket) == reg32(-1))
    {
        const reg32 error = thunked<kWsaGetLastError, &host::netLastError>(guest, cpu);
        const reg32 level = guest.dword(kLogLevel);
        guest.leaveEdx(level);
        guest.setDword(frame + 0x1c, error);
        if (sreg32(level) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d24c, frame + 0x1c, 0, 0, 1, 0);
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, state);
    return 0;
}

void frontend::netIpxOpen(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5c10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 portNumber = cpu.eax, flagValue = cpu.edx, withFlag = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxOpen(app, cpu, portNumber, flagValue, withFlag); });
}

/* sub_4a5b00: RaceNet's NPS opened and a game looked for: eax the player's name or 0, edx the
 * port, ebx the server and ecx its port.  Both calls through the DLL's pointers. */
reg32 frontend::neta::npsStart(App* app, CPU& cpu, reg32 name, reg32 localPort, reg32 server, reg32 remotePort)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x7cc);
    constexpr reg32 kNpsOpen = 0x7a21e4, kNpsAttach = 0x7a21dc;
    constexpr reg32 kNpsState = 0x7a20a0, kNpsName = 0x7a20a4, kNpsSameMachine = 0x7a2104, kNpsAddressBytes = 0x40;
    constexpr reg32 kNpsSession = 0x55ecb0, kNpsVersion = 0x7a215c;
    const reg32 frame = guest.frame();
    const reg32 port = frame + 0x7bc, serverPort = frame + 0x7b8;
    guest.setDword(port, localPort);
    guest.setDword(serverPort, remotePort);
    const reg32 loaded = fe::direct<0x4a59c0, frontend::wata::npsEnsureDll>(app, cpu, 0);
    if (loaded == 0)
        return loaded;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kNpsState, 0xbc);
    if (name == 0)
        name = 0x53dba4;
    guest.copyString(kNpsName, name);
    const reg16 portNumber = guest.word(port);
    guest.setDword(kNpsState, portNumber);
    guest.setByte(kNpsSameMachine, portNumber > 1 ? 0 : 1);

    // What the NPS library is opened with.
    const reg32 who = frame + 0x340;
    guest.copyString(who, kNpsName);
    guest.setDword(who + 0x20, guest.dword(kNpsState));
    copyBytes(guest, who + 0x24, kNpsSameMachine, kNpsAddressBytes);
    const reg32 callbacks = frame + 0x5a4;
    guest.setDword(callbacks, 0);
    guest.setDword(callbacks + 8, 0);
    guest.setDword(callbacks + 0x10, 0);
    const reg32 serverPortValue = guest.dword(serverPort);
    guest.leaveEdx(serverPortValue);
    guest.leaveEcx(kNpsReceived);
    guest.setDword(callbacks + 4, kNpsReceived);
    guest.setDword(callbacks + 0xc, kNpsDestroyed);
    const reg32 session = guest.call(guest.dword(kNpsOpen), args({}, { server, serverPortValue, who, callbacks, 10 }));
    guest.setDword(kNpsSession, session);
    if (sreg32(session) < 0)
        return 0;

    // What the game's listing says about this game.
    const reg32 listing = frame;
    guest.setDword(kNpsVersion, 1);
    guest.setDword(listing, 1);
    guest.copyString(listing + 4, 0x53dbac);
    guest.setDword(listing + 0x24, 0x21);
    guest.copyString(listing + 0x2a, 0x53dbb4);
    guest.setDword(listing + 0x34, 0x29a);
    guest.setDword(listing + 0x38, 0);
    fe::direct<0x4e0640, watcom::clib::memset>(app, cpu, listing + 0x3c, 0u, 0x100u);
    guest.leaveEbx(0x100);  // as the code loads it for it, which it keeps
    guest.copyString(listing + 0x3c, 0x53dbc0);
    const reg32 version = guest.dword(kNpsVersion);
    guest.leaveEdx(version);
    guest.leaveEcx(guest.dword(kNpsSession));
    if (sreg32(guest.call(guest.dword(kNpsAttach), args({}, { guest.dword(kNpsSession), 0x21, version, listing, 0 }))) < 0)
    {
        guest::direct<0x4a5dd0, npsStop>(app, cpu);
        return 0;
    }
    return 1;
}

void frontend::netNpsStart(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5b00", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 name = cpu.eax, localPort = cpu.edx, server = cpu.ebx, remotePort = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::npsStart(app, cpu, name, localPort, server, remotePort); });
}

/* sub_4a6a10: the barrier, every machine stopped until all have come to the same point.
 * eax the seconds to give up after (0 never, 128 ticks a second), edx which point: the host
 * says so and sees each machine answer, a client sees the host's word and answers it. */
reg32 frontend::neta::barrier(App* app, CPU& cpu, reg32 seconds, reg32 point)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    constexpr reg32 kBarrierPacket = 4, kPacketBytes = 4, kGaveUp = 0xc;
    const reg32 frame = guest.frame();
    guest.setDword(frame + 4, point);
    if (guest.dword(kSessionStage) == 0)
        return seconds;
    const reg16 over = guest.word(kSessionOver);
    guest.leaveEdx((point & 0xffff0000u) | over);
    if (over != 0)
        return seconds;
    reg32 giveUpAt = seconds << 7;
    if (giveUpAt != 0)
        giveUpAt += guest.dword(kTicks);
    guest.setByte(frame, kBarrierPacket);
    guest.setWord(frame + 2, reg16(guest.dword(frame + 4)));

    if (guest.word(kIsHost) != 0)
    {
        sendLeaving(app, cpu, 0, frame, kPacketBytes);
        for (reg32 machine = 1;; ++machine)
        {
            const reg32 count = signedWord(guest, kMachineCount);
            if (sreg32(machine) >= sreg32(count))
                return count;
            const reg32 barrier = kMachineBarriers + machine * 2;
            const reg32 link = kMachineLinks + machine * 4;
            if (guest.dword(guest.dword(link) + 0x24) == 0 || guest.word(kMachineGone + machine * 2) != 0)
            {
                guest.setWord(barrier, 0);
                continue;
            }
            bool wasDropped = false;
            for (;;)
            {
                guest.leaveEdx(machine * 2);
                if (signedWord(guest, barrier) == guest.dword(frame + 4))
                    break;
                const reg32 tasks = reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u));
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
                if (giveUpAt != 0 && sreg32(guest.dword(kTicks)) > sreg32(giveUpAt))
                {
                    guest.setWord(frame + 2, kGaveUp);
                    guest.setWord(barrier, 0);
                    fe::direct<0x4a60e0, ea::packetSent>(app, cpu, guest.dword(link), frame, kPacketBytes);
                    wasDropped = true;
                    break;
                }
            }
            if (wasDropped)
                directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.cpu(), machine);
            enter(guest, cpu, kBarrierLock);
            guest.leaveEdx(machine * 2);
            if (signedWord(guest, barrier) == guest.dword(frame + 4))
                guest.setWord(barrier, 0);
            leave(guest, cpu, kBarrierLock);
        }
    }

    for (;;)
    {
        const reg32 wanted = guest.dword(frame + 4);
        if (signedWord(guest, kHostBarrier) == wanted)
        {
            enter(guest, cpu, kBarrierLock);
            if (signedWord(guest, kHostBarrier) == wanted)
                guest.setWord(kHostBarrier, 0);
            leave(guest, cpu, kBarrierLock);
            return sendLeaving(app, cpu, kEndpoints, frame, kPacketBytes);
        }
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u);
        if (guest.word(kSessionOver) != 0 || guest.dword(kEndpointUsed) == 0)
        {
            guest.setWord(kSessionFailed, 1);
            guest.setWord(kHostBarrier, 0);
            guest.setWord(kSessionOver, 1);
            return 0;
        }
        if (giveUpAt != 0)
        {
            const reg32 ticks = guest.dword(kTicks);
            if (sreg32(ticks) > sreg32(giveUpAt))
            {
                guest.leaveEdx(1);
                guest.setWord(kHostBarrier, 0);
                guest.setWord(kSessionFailed, 1);
                guest.setWord(kSessionOver, 1);
                return ticks;
            }
        }
    }
}

void frontend::netBarrier(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6a10", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 seconds = cpu.eax, point = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::barrier(app, cpu, seconds, point); });
}

/* sub_4c5e80, before a network race: the settings that matter sent from the host in 0xd0 byte
 * packets, kept apart on a client around it for each of the 16 players' car and colour; and
 * the player this machine is. */
reg32 frontend::neta::raceSettings(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xfc);
    constexpr reg32 kSettings = 0x6fd39c, kSettingsEnd = 0x6fdc2c, kSendFrom = 0x7d36d4, kSettingsPlace = 0x7d36d0;
    constexpr reg32 kSettingsPacket = 5, kPacketRoom = 0xd0, kPlayers = 16;
    constexpr reg32 kPlayerCarField = 0x14, kPlayerExtraField = 0x68;
    constexpr sreg32 kMaxCar = 0x31;
    const reg32 frame = guest.frame();
    const reg32 packet = frame, keptPlace = frame + 0xdc, roomPlace = frame + 0xd4, leftPlace = frame + 0xe0, chunkPlace = frame + 0xd8;
    guest.setDword(kSettingsPlace, 0);
    guest.setDword(kSendFrom, kSettings);
    if (guest.word(kIsHost) == 0)
    {
        const reg32 kept = reg32(*fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x540214, 0x6c0, 0x10));
        guest.setDword(keptPlace, kept);
        for (reg32 i = 0; i < kPlayers; ++i)
            copyBytes(guest, kept + i * kRacePlayerSize, kRacePlayers + i * kRacePlayerSize, kRacePlayerSize);
    }
    guest::direct<0x4a6a10, barrier>(app, cpu, 0x5a, 3);
    if (guest.word(kIsHost) != 0)
    {
        guest.setByte(packet, kSettingsPacket);
        guest.setDword(roomPlace, kPacketRoom);
        guest.setDword(leftPlace, kSettingsEnd - kSettings);
        while (sreg32(guest.dword(leftPlace)) > 0)
        {
            const reg32 left = guest.dword(leftPlace), room = guest.dword(roomPlace);
            const reg32 chunk = sreg32(left) < sreg32(room) ? left : room;
            guest.setDword(chunkPlace, chunk);
            copyBytes(guest, packet + 1, guest.dword(kSendFrom), chunk);
            fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, chunk + 1);
            guest.setDword(kSendFrom, guest.dword(kSendFrom) + guest.dword(chunkPlace));
            guest.setDword(leftPlace, guest.dword(leftPlace) - guest.dword(roomPlace));
        }
    }
    guest::direct<0x4a6a10, barrier>(app, cpu, 0x1e, 4);
    if (guest.word(kIsHost) == 0)
    {
        const reg32 kept = guest.dword(keptPlace);
        for (reg32 i = 0; i < kPlayers; ++i)
        {
            const reg32 at = i * kRacePlayerSize;
            const reg32 car = guest.dword(kept + at + kPlayerCarField);
            if (sreg32(car) >= 0 && sreg32(car) <= kMaxCar)
                guest.setDword(kRacePlayers + at + kPlayerCarField, car);
            guest.setDword(kRacePlayers + at + kPlayerExtraField, guest.dword(kept + at + kPlayerExtraField));
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, kept);
    }
    reg32 result = signedWord(guest, kThisMachine);
    guest.setDword(0x6fd4f4, result);
    guest.setDword(0x6fd4f8, result);
    if (guest.dword(0x6fd50c) != 0)
    {
        for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kRacePlayerCount)); ++i)
        {
            if (i == guest.dword(0x6fd4f4))
                continue;
            result = i * 12;
            guest.setDword(0x6fd548 + i * kRacePlayerSize, 1);
        }
    }
    return result;
}

void frontend::netRaceSettings(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c5e80", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::raceSettings(app, cpu); });
}

/* sub_4a66c0: the host sends the machines' list, packet 1, to each of them, naming each
 * machine by its number in its own copy; then the settings. */
reg32 frontend::neta::sendSetup(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x98);
    constexpr reg32 kSetupBytes = 0x7c, kEndpointsTried = 7;
    const reg32 packet = guest.frame();
    guest.setWord(kThisMachine, 0);
    guest.setWord(kMachineCount, 1);
    guest.setByte(kPacketSetup, 1);
    guest.setDword(kSetupOther, guest.dword(kLinkKind));
    guest.setWord(kSetupDirty, 0);
    guest.setByte(kSetupLinks, reg8(guest::direct<0x4a5af0, thisLink>(app, cpu)));
    guest.copyString(kSetupNames, kPlayerName);
    for (reg32 i = 0; i < kEndpointsTried; ++i)
    {
        const reg32 at = i * kEndpointSize;
        if (guest.dword(kEndpointUsed + at) == 0)
            continue;
        const reg32 machine = signedWord(guest, kMachineCount);
        guest.setDword(kMachineLinks + machine * 4, kEndpoints + at);
        guest.copyString(kSetupNames + machine * kNameSize, kEndpointNames + i * kNameSize);
        if (guest.dword(kLinkKind) == 2)
            guest.setByte(signedWord(guest, kMachineCount) + kSetupLinks, guest.byte(kEndpointLink + at));
        guest.setWord(kMachineCount, reg16(guest.word(kMachineCount) + 1));
    }
    guest.setWord(0x7a227a, 0);
    guest.setWord(kMachineGone, 0);
    for (reg32 i = 1; i < 8; ++i)
    {
        guest.setWord(kMachineGone + i * 2, 0);
        const reg32 number = fe::direct<0x4a6650, ea::machineOf>(app, cpu, kEndpoints + (i - 1) * kEndpointSize);
        guest.setWord(kMachineNumbers + (i + 1) * 2, reg16(number));
    }
    copyBytes(guest, packet, kPacketSetup, kSetupBytes);
    enter(guest, cpu, kPlayersLock);
    for (reg32 i = 1; sreg32(signedWord(guest, kMachineCount)) > sreg32(i); ++i)
    {
        guest.setWord(packet + 0x64, reg16(i));
        fe::direct<0x4a60e0, ea::packetSent>(app, cpu, guest.dword(kMachineLinks + i * 4), packet, kSetupBytes);
    }
    leave(guest, cpu, kPlayersLock);
    return guest::direct<0x49aa80, sendSettings>(app, cpu);
}

void frontend::netSendSetup(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a66c0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendSetup(app, cpu); });
}

/* sub_49bab0: the cars' files between the machines that need them, a machine at a time with a
 * barrier after each.  First the client's own car to the host when it is one of its own, then
 * the host's copies back out; a client only waits through the second lot. */
reg32 frontend::neta::exchangeCars(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1f8);
    constexpr reg32 kWaitSeconds = 0x3c, kFirstBarrier = 0xf, kSecondBarrier = 0x17;
    constexpr reg32 kPleaseWait = 0x115, kRedraws = 15;
    const reg32 frame = guest.frame();
    // Each redraw hands on what the call before left in eax.
    reg32 left = fe::direct<0x444be0, frontend::dialogs::messageById>(app, cpu, kPleaseWait);
    for (reg32 i = 0; i < kRedraws; ++i)
        left = reg32(*fe::direct<0x44b170, port::buttons::redrawn>(app, cpu, cpu.esp - 4,
                                                                   port::Args{ left, cpu.edx, cpu.ebx, cpu.ecx }));
    if (guest.word(kIsHost) != 0)
    {
        for (reg32 i = 0; sreg32(i) < sreg32(signedWord(guest, kMachineCount)); ++i)
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, i + kFirstBarrier);
    }
    else
    {
        const reg32 mine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), signedWord(guest, kThisMachine)));
        const reg32 car = reg32(*fe::direct<0x439520, load::idOf>(app, cpu, guest.dword(kPlayerCar)));
        for (reg32 i = 0; sreg32(i) < sreg32(signedWord(guest, kMachineCount)); ++i)
        {
            const reg32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), i));
            if (machine != reg32(-1) && machine == mine
                && fe::direct<0x439530, frontend::loada::carIdIsPlain>(app, cpu, car) != 0)
            {
                const reg32 header = frame + 0x1a8, body = frame + 0xd4;
                guest.setByte(header, 0xf);
                const reg32 file = guest.call(kBeginCarSend, args({ header, guest.dword(kPlayerCar) }));
                guest.setByte(body, 0x10);
                guest::direct<0x49b9b0, sendCarFile>(app, cpu, body, file, guest.dword(header + 0x18));
            }
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, i + kFirstBarrier);
        }
    }
    reg32 count;
    if (guest.word(kIsHost) != 0)
    {
        for (reg32 i = 0; (count = signedWord(guest, kMachineCount)), sreg32(i) < sreg32(count); ++i)
        {
            const reg32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), i));
            if (machine != reg32(-1))
            {
                const reg32 id = kPlayerRecords + machine * kPlayerRecordSize + 0x19;
                const reg32 car = fe::direct<0x439990, load::placeById>(app, cpu, id);
                if (car == reg32(-1))
                    fe::direct<0x401010, ea::error>(app, cpu, 0x53c5b0u, guest::Words({}));
                if (fe::direct<0x439530, frontend::loada::carIdIsPlain>(app, cpu, id) != 0)
                {
                    const reg32 header = frame + 0x1c4, body = frame;
                    guest.setByte(header, 0x11);
                    const reg32 file = guest.call(kBeginCarSend, args({ header, car }));
                    guest.setByte(body, 0x12);
                    guest::direct<0x49b9b0, sendCarFile>(app, cpu, body, file, guest.dword(header + 0x18));
                }
            }
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, i + kSecondBarrier);
        }
    }
    else
    {
        for (reg32 i = 0; (count = signedWord(guest, kMachineCount)), sreg32(i) < sreg32(count); ++i)
            guest::direct<0x4a6a10, barrier>(app, cpu, kWaitSeconds, i + kSecondBarrier);
    }
    return count;
}

void frontend::netExchangeCars(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49bab0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::exchangeCars(app, cpu); });
}

/* sub_49eae0, from the host's lobby: the machines that have come and gone since last time.
 * 1 when any did.  A client has its own (sub_4a69c0). */
reg32 frontend::neta::hostLobby(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x3c);
    constexpr reg32 kJoining = 0x55e714, kFarAway = 0xfffffff, kAcceptCount = 0x7a01e0, kAcceptRoom = 0x7a0390;
    const reg32 frame = guest.frame();
    const reg32 changedPlace = frame + 0x20, joiner = frame;
    guest.setWord(changedPlace, 0);
    if (guest.dword(kSessionStage) != 3)
        return 0;
    if (guest.word(kIsHost) == 0)
        return guest::direct<0x4a69c0, clientLobby>(app, cpu);
    if (guest.dword(kLinkKind) != 2)
    {
        for (reg32 i = 0; i < 7; ++i)
        {
            const reg32 at = i * kEndpointSize;
            if (guest.dword(kEndpointUsed + at) == 0 && guest.dword(kEndpointOffered + at) != 0)
            {
                guest.leaveEcx(1);
                directKept<0x49ecb0, ea::connectionDropped, 1>(guest.app(), guest.cpu(), kEndpoints + at);
                guest.setWord(changedPlace, 1);
            }
        }
    }
    while (guest::direct<0x49f680, pendingEmpty>(app, cpu) == 0)
    {
        guest::direct<0x49f730, nextPending>(app, cpu, joiner);
        reg32 slot = 0;
        for (; slot < 8; ++slot)
        {
            if (guest.dword(kEndpointUsed + slot * kEndpointSize) == 0)
            {
                guest.copyString(kEndpointNames + slot * kNameSize, joiner);
                break;
            }
        }
        guest.setDword(kJoining, guest.dword(kTicks) + 0x280);
        const reg32 endpoint = fe::direct<0x49efb0, frontend::connect::setUpEndpoint>(app, cpu, joiner + 0xb, slot);
        bool joined = true;
        if (guest.dword(kLinkKind) != 2)
        {
            if (guest::direct<0x4f5850, sendQueued>(app, cpu, endpoint, 0xd2, 7, kAcceptRoom) == 0)
            {
                guest.setDword(kAcceptCount, guest.dword(kAcceptCount) - 1);
                joined = false;
            }
        }
        if (joined)
        {
            guest.setWord(kMachineCount, reg16(guest.word(kMachineCount) + 1));
            guest.setWord(changedPlace, 1);
            guest::direct<0x4a6ce0, compareVersions>(app, cpu, endpoint);
        }
        guest.setDword(kJoining, kFarAway);
    }
    const reg16 changed = reg16(guest.word(changedPlace) | guest.word(kSetupDirty));
    guest.setWord(changedPlace, changed);
    if (changed != 0)
        guest::direct<0x4a66c0, sendSetup>(app, cpu);
    return signedWord(guest, changedPlace);
}

void frontend::netHostLobby(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49eae0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::hostLobby(app, cpu); });
}

/* sub_49e080: the host's game started, on RaceNet: the settings the launcher left at 0x7a1fa0,
 * then NPS opened with its server or with EA's own. */
reg32 frontend::neta::hostStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    constexpr reg32 kServerName = 0x7a1fa0;
    guest.setDword(kSessionStage, 3);
    guest.setDword(kLinkKind, 2);
    guest.setDword(0x6fd50c, 1);
    guest.setDword(0x6fd3b0, 4);
    guest.setDword(0x55e71c, 0);
    guest.setByte(kPacketSetup, 1);
    guest.setWord(0x7a227a, 0);
    guest.copyString(kSetupNames, kPlayerName);
    for (reg32 i = 2; i <= 8; ++i)
        guest.setWord(kMachineNumbers + i * 2, 0xffff);
    fe::direct<0x49f650, frontend::connect::clearQueue>(app, cpu);
    guest::direct<0x4a6500, sessionReset>(app, cpu);
    guest.call(kOpenMessages);
    guest.setDword(0x55e860, 1);
    reg32 started;
    if (guest.length(kServerName) != 0)
    {
        guest.copyString(kPlayerName, 0x7a1fe0);
        guest.setDword(0x6fd3b8, guest.dword(0x7a2000));
        guest.setDword(0x6fd4ac, guest.dword(0x7a2004));
        const reg32 opponents = guest.dword(0x7a200c);
        guest.setDword(0x6fd4b4, opponents);
        guest.setDword(0x6fd4b0, opponents == 8 ? 2 : opponents == 4 ? 1 : 0);
        guest.setDword(0x6fd470, guest.dword(0x7a2010));
        guest.setDword(0x6fd4d4, guest.dword(0x7a2014));
        guest.call(kPlayerToListedCar, args({ guest.dword(0x7a2008), 0, kServerName }));
        const reg32 port = (guest.edx() & 0xffff0000u) | guest.word(0x7a1ffc);
        started = guest::direct<0x4a5b00, npsStart>(app, cpu, kPlayerName, port, cpu.ebx, guest.dword(0x7a1fdc));
    }
    else
    {
        started = guest::direct<0x4a5b00, npsStart>(app, cpu, kPlayerName, 0, 0x53c6bc, 0x1a0a);
    }
    return started != 0 ? 1 : 0;
}

void frontend::netHostStart(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49e080", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::hostStart(app, cpu); });
}

/* sub_4f61f0, the IPX service's receive, stack the room in the buffer: eax the state, edx where
 * the sender's address goes, ebx the kind of packet asked for and where the kind found goes, ecx
 * the buffer.  The first in the queue that fits; ones older than 16 ticks are thrown away. */
reg32 frontend::neta::ipxReceive(App* app, CPU& cpu, reg32 state, reg32 address, reg32 kindPlace, reg32 buffer, reg32 room)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x3c);
    constexpr reg32 kPacketsMatch = 0x4f5a80;
    const reg32 frame = guest.frame();
    // What the queue's match reads: the kind, the address, the state.
    guest.setDword(frame, guest.dword(kindPlace));
    copyBytes(guest, frame + 4, address, 20);
    guest.setDword(frame + 0x18, state);
    const reg32 received = state + 0x8c;
    reg32 packet = fe::direct<0x50e5a0, frontend::watb::queueFetch>(app, cpu, received, kPacketsMatch, frame);
    const reg32 freeQueue = state + 0x70;
    while (packet != 0)
    {
        if (sreg32(guest.dword(kClock) + 0x10) >= sreg32(guest.dword(packet + 4)))
        {
            reg32 size = guest.dword(packet + 8) - 4;
            guest.leaveEcx(room);
            if (sreg32(size) > sreg32(room))
                size = room;
            else if (sreg32(size) < 0)
                size = 0;
            copyBytes(guest, address, packet + 0xc, 20);
            guest.leaveEdx(kindPlace);
            guest.setDword(kindPlace, guest.dword(packet + 0x20));
            if (size != 0)
                guest.call(kMemmove, args({ packet + 0x28, buffer, size }));
            if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
                guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d2a8, packet + 0x20, packet + 0xc, 0x14, 0, 0);
            guest::direct<0x50de50, netb::qpush>(app, cpu, freeQueue, packet);
            fe::direct<0x4dfb30, ea::eventSet>(app, cpu, guest.dword(state + 0x1c));
            return size;
        }
        if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d29c, packet + 0x20, packet + 0xc, 0x14, 0, 0);
        guest::direct<0x50de50, netb::qpush>(app, cpu, freeQueue, packet);
        fe::direct<0x4dfb30, ea::eventSet>(app, cpu, guest.dword(state + 0x1c));
        // The match and its frame too, as the call before leaves them in edx and ebx.
        packet = fe::direct<0x50e5a0, frontend::watb::queueFetch>(app, cpu, received, kPacketsMatch, frame);
    }
    guest.setDword(kindPlace, 0);
    return 0;
}

void frontend::netIpxReceive(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f61f0", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 state = cpu.eax;
    const reg32 address = cpu.edx;
    const reg32 kindPlace = cpu.ebx;
    const reg32 buffer = cpu.ecx;
    const reg32 room = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxReceive(app, cpu, state, address, kindPlace, buffer, room); });
}

/* sub_49a8e0: this machine's player sent to the others, packet 3: its name and car by name and
 * id, its colours and what goes with it, 0x60 bytes, a copy kept at its own number. */
reg32 frontend::neta::sendMyCar(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    guest.setByte(kMyRecord, 3);
    const reg32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), signedWord(guest, kThisMachine)));
    guest.setDword(0x79f91c, machine);
    if (machine == reg32(-1))
        return machine;
    guest.setByte(0x79f920, guest.byte(kPlayerTransmission));
    guest.copyString(0x79f921, kPlayerName);
    const reg32 id = reg32(*fe::direct<0x439520, load::idOf>(app, cpu, guest.dword(kPlayerCar)));
    copyBytes(guest, 0x79f931, id, 0x10);
    reg32 text = reg32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, cpu, cpu.esp - 4, guest.dword(kPlayerCar), std::int32_t(0)));
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), 0x79f941, text, 0xf);
    guest.setByte(0x79f950, 0);
    text = reg32(*fe::direct<0x439ab0, port::fields::carTextOf>(app, cpu, cpu.esp - 4, guest.dword(kPlayerCar), std::int32_t(1)));
    // By address: the ebx and edx it leaves may be left at the end; eax is read after it.
    guest.call(kStrncpy, args({ 0x79f951, text, 0x10 }));
    guest.setByte(0x79f960, 0);
    fe::direct<0x472360, port::fields::customColoursRefreshed>(app, cpu, cpu.esp - 4, cpu.eax);
    for (reg32 i = 0; i < 4; ++i)
        guest.setByte(0x79f961 + i, guest.byte(kPlayerColours + i));
    const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, guest.dword(kPlayerCar));
    guest.setDword(0x79f968, guest.dword(record + 0x380));
    guest.setByte(0x79f92c, guest.byte(kPlayerCar));
    guest.setByte(0x79f92d, guest.byte(kPlayerColour));
    guest.setByte(0x79f92e, guest.byte(0x6fd2dc));
    guest.setByte(0x79f92f, guest.byte(0x6fd2e0));
    guest.setByte(0x79f930, guest.byte(0x6fd2e4));
    const reg32 extras = fe::direct<0x439510, frontend::loada::carExtras>(app, cpu, guest.dword(kPlayerCar));
    copyBytes(guest, 0x79f96c, extras, 12);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, kPlayerRecords + guest.dword(0x79f91c) * kPlayerRecordSize, kMyRecord, kPlayerRecordSize);
    return fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, kMyRecord, kPlayerRecordSize)->eax;
}

void frontend::netSendMyCar(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49a8e0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendMyCar(app, cpu); });
}

/* sub_4c8040: a player's controls for the race's frame: eax where they go, edx the player.  A
 * local player's are in the tables here, a machine's from the ring of 20 frames its packets fill,
 * valid only for the frame asked and counted for how stale they get.  0 when there are none. */
reg32 frontend::neta::playerControls(App* app, CPU& cpu, reg32 out, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    constexpr reg32 kLocalControls = 0x7dd010, kWantedFrames = 0x7dd2a0, kRingFrames = 0x7dd120, kRingControls = 0x7dd110;
    const reg32 cars = guest.dword(kCarCount);
    const reg32 machine = player - cars;
    guest.leaveEdx(cars);
    enter(guest, cpu, kInputLock);
    if (sreg32(player) < sreg32(guest.dword(kCarCount)))
    {
        const reg32 local = kLocalControls + player * 12;
        guest.setDword(out + 8, guest.dword(local));
        guest.setWord(out + 4, guest.word(local + 4));
        const reg32 car = guest.dword(kCarsByPlayer + player * 4);
        guest.setDword(out + 0x10, 1);
        guest.setWord(out + 0xc, reg16(guest.word(car + 8) - guest.word(out + 4)));
        guest.setByte(car + 0x50e, (guest.byte(local + 8) >> 3) & 1);
        guest.leaveEdx(car);
        if (guest.word(out + 4) == 0)
        {
            guest.setDword(out + 8, 0);
            guest.setWord(out + 0xc, 0);
        }
        leave(guest, cpu, kInputLock);
        return 1;
    }
    const reg32 wanted = kWantedFrames + machine * 20;
    const reg32 frameWanted = guest.dword(wanted);
    guest.setDword(out, frameWanted);
    if (frameWanted == 0)
    {
        guest.leaveEdx(guest.dword(kInputLock));
        leave(guest, cpu, kInputLock);
        return 0;
    }
    const sreg32 inRing = (sreg32(frameWanted) >> 5) % 20;
    const reg32 ring = reg32(inRing) * 20;
    if (frameWanted != guest.dword(kRingFrames + ring))
    {
        guest.leaveEdx(reg32(inRing));
        leave(guest, cpu, kInputLock);
        return 0;
    }
    const reg32 controls = kRingControls + ring + machine * 2;
    guest.leaveEdx(machine * 2);
    if (guest.word(controls) == 0)
    {
        leave(guest, cpu, kInputLock);
        return 0;
    }
    const reg16 pressed = guest.word(wanted + 4);
    guest.leaveEdx((machine * 2 & 0xffff0000u) | pressed);
    guest.setWord(out + 4, pressed);
    guest.setWord(out + 0xc, reg16(guest.word(controls) - pressed));
    const reg32 stale = guest.dword(wanted + 0x10) + 1;
    guest.setDword(wanted + 0x10, stale);
    guest.setDword(out + 0x10, stale);
    leave(guest, cpu, kInputLock);
    return 1;
}

void frontend::netPlayerControls(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c8040", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 out = cpu.eax, player = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::playerControls(app, cpu, out, player); });
}

namespace
{

constexpr reg32 kFread = 0x4e9200, kFclose = 0x4ee100, kStrrchr = 0x4ee2f0;
constexpr reg32 kQueueFindPosition = 0x50e220;
constexpr reg32 kSyncFrame = 0x4c6970;
constexpr reg32 kExitNet = 0x4f4c10, kNetTrace = 0x4f4c50;
constexpr reg32 kSendTo = 0x533e32, kRecvFrom = 0x533e5c;

// The services' games, 32 bytes each, and the lock over them.
constexpr reg32 kServiceLock = 0x566dfc, kGameCount = 0x566e00, kGames = 0x9f2644;
constexpr reg32 kGameTag = 0xea646174;
constexpr reg32 kSocketLock = 0x564480;

}  // namespace

/* sub_49acd0, as a network race loads: the cars' files exchanged and every machine at the same
 * point, then each player's record made into the race's players' table. */
reg32 frontend::neta::playersToRace(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    guest::direct<0x49bab0, exchangeCars>(app, cpu);
    // What the calls leave in eax goes on to the dialog's close at the end, which reads it.
    reg32 left = guest::direct<0x4a6a10, barrier>(app, cpu, 0x1e, 0x1f);
    for (reg32 i = 0; sreg32(i) < sreg32(signedWord(guest, kMachineCount)); ++i)
    {
        left = directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), i);
        const reg32 machine = widened(left);
        if (machine == reg32(-1))
            continue;
        const reg32 record = machine * kPlayerRecordSize;
        const reg32 from = kPlayerRecords + record;
        guest.setDword(guest.frame(), from);
        const reg32 slot = i * kRacePlayerSize;
        guest.copyString(kRacePlayers + slot + 0x34, from + 9);
        left = fe::direct<0x439990, load::placeById>(app, cpu, from + 0x19);
        guest.setDword(slot + 0x6fd540, left);
        guest.setDword(slot + 0x6fd548, guest.byte(record + 0x79f605));
        for (reg32 c = 0; c < 4; ++c)
            guest.setByte(slot + 0x6fd54c + c, guest.byte(record + 0x79f639 + c));
        guest.setDword(slot + 0x6fd550, guest.byte(record + 0x79f606));
        guest.setDword(slot + 0x6fd554, guest.byte(record + 0x79f607));
        guest.setDword(slot + 0x6fd558, guest.byte(record + 0x79f608));
        copyBytes(guest, slot + 0x6fd588, record + 0x79f644, 12);
    }
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 record = i * kPlayerRecordSize;
        guest.setByte(record + 0x79f5f8, 0);
        guest.setDword(record + 0x79f5f4, 0);
    }
    return reg32(*fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, cpu, left));
}

void frontend::netPlayersToRace(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49acd0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::playersToRace(app, cpu); });
}

/* sub_4a6ce0: the game's version, packet 0: the host sends its own to a machine that joins, a
 * client waits for the host's and stops the game with an error when they differ. */
reg32 frontend::neta::compareVersions(App* app, CPU& cpu, reg32 link)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x44);
    constexpr reg32 kOurVersion = 0x55e898, kHostVersion = 0x55ed08, kGameQuits = 0x564819;
    constexpr reg32 kSourceFile = 0x53dbdc, kVersionsDiffer = 0x53dbfc, kLine = 0x4d1;
    constexpr reg32 kBackslash = 0x5c;
    const reg32 packet = guest.frame(), version = packet + 1;
    guest.setByte(packet, 0);
    guest.copyString(version, kOurVersion);
    if (guest.word(kIsHost) != 0)
    {
        fe::direct<0x4a60e0, ea::packetSent>(app, cpu, link, packet, 0x29);
        return 1;
    }
    while (guest.byte(kHostVersion) == 0)
    {
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        if (guest.byte(kGameQuits) != 0 || guest.word(kSessionOver) != 0)
            return 0;
    }
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), kHostVersion, version) == 0)
    {
        guest.setByte(kHostVersion, 0);
        return 1;
    }
    directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.cpu());
    reg32 file = kSourceFile;
    if (fe::direct<kStrrchr, watcom::clib::strrchr>(guest.app(), guest.cpu(), kSourceFile, kBackslash) != 0)
        file = fe::direct<kStrrchr, watcom::clib::strrchr>(guest.app(), guest.cpu(), kSourceFile, kBackslash) + 1;
    fe::direct<kError, ea::error>(guest.app(), guest.cpu(), kVersionsDiffer, guest::Words{1, file, kLine, kHostVersion, version});
    return 0;
}

void frontend::netCompareVersions(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6ce0", 0, 0, nullptr, 0 };
    const reg32 link = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::compareVersions(app, cpu, link); });
}

/* sub_4f4d10, NET_initservice, stack the three that go on: eax the service's record, edx a
 * name, ebx and ecx what the services' open takes.  The first time, NET_'s queue and its exit;
 * then each service in the table is tried until one takes it, and its record goes on the queue. */
reg32 frontend::neta::initService(App* app, CPU& cpu, reg32 service, reg32 name, reg32 first, reg32 second, reg32 onOne, reg32 onTwo, reg32 onThree)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    constexpr reg32 kServicesQueue = 0x9f25f0, kServiceCount = 0x566de4, kReady = 0x566de8;
    constexpr reg32 kServiceNames = 0x9f25d0, kServiceTables = 0x9f25b0;
    constexpr reg32 kRecordBytes = 0xac;
    const reg32 frame = guest.frame();
    guest.setDword(frame + 4, name);
    guest.setDword(frame, first);
    guest.setDword(frame + 8, second);
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.cpu(), 0x54d0d0);
    if (guest.dword(kReady) == 0)
    {
        guest::direct<0x50dcd0, netb::qinit>(app, cpu, kServicesQueue, 0, 0);
        fe::direct<0x4f2a78, frontend::eab::atexitJump>(app, cpu, kExitNet);
        if (guest.dword(0x566ddc) == 0)
            guest.setDword(0x566ddc, reg32((sreg32(guest.dword(0x5643d8)) + 9) / 10));
        guest.setDword(kReady, 1);
    }
    if (guest.call(kQueueFindPosition, args({ kServicesQueue, service })) != reg32(-1))
        guest::direct<0x4f4e60, killService>(app, cpu, service);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), service, kRecordBytes);
    guest.setDword(frame + 0xc, 0);
    reg32 result = 0;
    if (sreg32(guest.dword(kServiceCount)) > 0)
    {
        reg32 at = 0;
        reg32 tried;
        do
        {
            if (result != 0)
                break;
            guest.setDword(service + 8, guest.dword(at + kServiceNames));
            guest.setDword(service + 4, guest.dword(at + kServiceTables));
            guest.setDword(service + 0x10, guest.dword(frame + 4));
            const bool wanted = guest.dword(guest.dword(service + 4) + 4) != 0
                || guest::direct<0x4f4bb0, sameGuid>(app, cpu, guest.dword(at + kServiceNames) + 4, guest.dword(frame + 8)) != 0;
            if (wanted)
            {
                result = guest.call(guest.dword(guest.dword(service + 4) + 8),
                                     args({ service, guest.dword(frame), guest.dword(frame + 8), onOne }, { onTwo, onThree }));
            }
            tried = guest.dword(frame + 0xc) + 1;
            const reg32 count = guest.dword(kServiceCount);
            guest.leaveEdx(tried);
            guest.leaveEbx(count);
            at += 4;
            guest.setDword(frame + 0xc, tried);
            if (sreg32(tried) >= sreg32(count))
                break;
        } while (true);
    }
    guest.leaveEcx(result);
    if (result != 0)
        guest::direct<0x50de50, netb::qpush>(app, cpu, kServicesQueue, service);
    return result;
}

void frontend::netInitService(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f4d10", 12, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 service = cpu.eax;
    const reg32 name = cpu.edx;
    const reg32 first = cpu.ebx;
    const reg32 second = cpu.ecx;
    const reg32 onOne = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 onTwo = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 onThree = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::initService(app, cpu, service, name, first, second, onOne, onTwo, onThree); });
}

/* sub_49b9b0: a car's file sent, eax the packet with its kind first, edx the file, ebx its size:
 * as much as a packet takes, 0xd0 bytes, with the system let run between them; then the file's
 * 300 byte tail the game keeps at 0x79f97c. */
reg32 frontend::neta::sendCarFile(App* app, CPU& cpu, reg32 packet, reg32 file, reg32 left)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x28);
    constexpr reg32 kPacketRoom = 0xd0, kTail = 0x79f97c, kTailBytes = 0x12c, kViv = 0x79f978;
    const reg32 filePlace = guest.frame();
    guest.setDword(filePlace, file);
    while (sreg32(left) > 0)
    {
        const reg32 chunk = sreg32(left) < sreg32(kPacketRoom) ? left : kPacketRoom;
        if (fe::direct<kFread, watcom::clib::fread>(guest.app(), guest.cpu(), packet + 1, chunk, 1, file) == 0)
            return reg32(-1);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, chunk + 1);
        left -= chunk;
    }
    const reg32 viv = guest.dword(kViv);
    if (viv != 0)
        fe::direct<0x43b9b0, ea::vivClosed>(app, cpu, viv, filePlace);
    else
        fe::direct<kFclose, watcom::clib::fclose>(guest.app(), guest.cpu(), file);
    reg32 sent = 0;
    sreg32 tail = kTailBytes;
    do
    {
        const reg32 chunk = tail < sreg32(kPacketRoom) ? reg32(tail) : kPacketRoom;
        copyBytes(guest, packet + 1, kTail + sent, chunk);
        sent += chunk;
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        sendLeaving(app, cpu, 0, packet, chunk + 1);
        tail -= sreg32(chunk);
    } while (tail > 0);
    return 0;
}

void frontend::netSendCarFile(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b9b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.eax, file = cpu.edx, left = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendCarFile(app, cpu, packet, file, left); });
}

/* sub_49abd0, from the lobby: whether the race can start: 1 alone or when every machine's player
 * has its car; the host sends what it last has of everyone and all wait at barrier 1, twice. */
reg32 frontend::neta::allReady(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kStartWanted = 0x55e710;
    if (guest.dword(kSessionStage) != 0)
    {
        if (std::int16_t(guest.word(kMachineCount)) < 2)
            return 0;
        if (guest.word(kIsHost) != 0)
        {
            for (reg32 i = 0; sreg32(i) < sreg32(signedWord(guest, kMachineCount)); ++i)
            {
                const reg32 machine = widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), i));
                if (machine == reg32(-1))
                    return 0;
                if (guest.byte(machine * kPlayerRecordSize + 0x79f5f8) == 0)
                    return 0;
            }
            guest.setDword(kStartWanted, 1);
            if (guest::direct<0x49f680, pendingEmpty>(app, cpu) == 0 || guest.word(kPlayersNew) != 0)
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
            guest::direct<0x49ee50, lobbyClose>(app, cpu);
            guest::direct<0x49aa80, sendSettings>(app, cpu);
        }
        guest::direct<0x49a8e0, sendMyCar>(app, cpu);
        fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kMachineGone, 0x10);
        guest::direct<0x4a6a10, barrier>(app, cpu, 0x3c, 1);
        guest::direct<0x4a6a10, barrier>(app, cpu, 0x1e, 1);
    }
    return 1;
}

void frontend::netAllReady(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49abd0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::allReady(app, cpu); });
}

/* sub_4c7d30, packet 13: a machine's controls for a frame, edx the packet, ebx its size.  The
 * host passes it on to the others unless RaceNet does; the packed controls are unpacked and kept
 * under the lock, and machine 0's packet says which frame everyone's are for. */
reg32 frontend::neta::inputPacket(App* app, CPU& cpu, reg32 packet, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    constexpr reg32 kInputRows = 0x7dd070, kRowBytes = 20, kWantedFrames = 0x7dd2a0, kPlayers = 0x6fd520;
    const reg32 machine = guest.byte(packet + 1);
    const reg32 row = machine * kRowBytes;
    const reg32 frameDone = guest.dword(kInputRows + row + 0xc);
    guest.leaveEdx(machine);
    if (sreg32(frameDone) > sreg32(guest.dword(packet + 4)))
        return frameDone;
    const reg32 local = guest.frame();
    guest.setDword(local + 0xc, packet);
    guest.setDword(local + 8, machine);
    if (guest.word(kIsHost) != 0 && guest.dword(kLinkKind) != 2)
    {
        for (reg32 i = 1; sreg32(i) < sreg32(signedWord(guest, kMachineCount)); ++i)
        {
            if (i != machine)
                fe::direct<0x4a60e0, ea::packetSent>(app, cpu, guest.dword(kMachineLinks + i * 4), packet, size);
        }
    }
    guest.leaveEbx(row);
    guest.setDword(kInputRows + row + 0xc, guest.dword(packet + 4));
    guest::direct<0x4c8350, unpackControls>(app, cpu, packet + 8, local);
    const reg32 lock = guest.dword(kInputLock);
    guest.leaveEdx(lock);
    enter(guest, cpu, kInputLock);
    copyBytes(guest, kInputRows + row, local, 8);
    reg8 readyLow = guest.byte(packet + 0xc);
    reg32 eax = (packet & 0xffffff00u) | readyLow;
    guest.setByte(kInputRows + row + 8, readyLow);
    guest.setDword(kInputRows + row + 0x10, 1);
    if (machine == 0)
    {
        reg32 player = 0;
        for (; sreg32(player) < sreg32(guest.dword(kPlayers)); ++player)
        {
            const reg32 wanted = kWantedFrames + player * kRowBytes;
            guest.setDword(wanted, guest.dword(packet + 4));
            guest.setWord(wanted + 4, guest.word(packet + player * 2 + 0xe));
            guest.setDword(wanted + 0x10, 0);
            eax = player * 5;
        }
        guest.leaveEdx(player);
    }
    guest.leaveEbx(guest.dword(kInputLock));
    cpu.eax = eax;
    return windows<kLeaveCriticalSection, &host::leaveLock>(guest, cpu, guest.dword(kInputLock));
}

void frontend::netInputPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7d30", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::inputPacket(app, cpu, packet, size); });
}

/* sub_4f53f0, a service's poll, eax the game, edx where the sender's address is.  What came for
 * a game the service knows of goes to that game's handler; the rest is read off until none are
 * left, each given to the service's own receive. */
reg32 frontend::neta::servicePoll(App* app, CPU& cpu, reg32 service, reg32 address)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xe0);
    constexpr reg32 kPacketRoom = 0xac;
    const reg32 frame = guest.frame();
    guest.setDword(frame + 0xc8, address);
    guest.leaveEdx(guest.dword(kServiceLock));
    if (guest.dword(kServiceLock) != 0)
    {
        enter(guest, cpu, kServiceLock);
        const reg32 game = guest::direct<0x4f5390, findGame>(app, cpu, service, guest.dword(frame + 0xc8));
        if (sreg32(game) >= 0)
        {
            const reg32 entry = guest.dword(kGames + (game << 5));
            const reg32 handler = guest.dword(entry + 8);
            guest.leaveEdx(handler);
            if (handler != 0)
                guest.call(handler, args({ entry, handler }));
        }
        leave(guest, cpu, kServiceLock);
        if (sreg32(game) >= 0)
            return guest.eax();
    }
    const reg32 buffer = frame, from = frame + 0xac, tag = frame + 0xc0, size = frame + 0xc4;
    reg32 received;
    do
    {
        copyBytes(guest, from, guest.dword(frame + 0xc8), 20);
        guest.setDword(tag, kGameTag);
        received = guest.call(guest.dword(guest.dword(service + 8) + 0x58),
                              args({ guest.dword(service + 0xc), from, tag, buffer }, { kPacketRoom }));
        if (received != 0 && guest.dword(tag) != 0)
            guest.call(guest.dword(guest.dword(service + 4) + 0x40),
                       args({ service, from, guest.dword(tag), buffer }, { received, size }));
    } while (received != 0);
    return 0;
}

void frontend::netServicePoll(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f53f0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 service = cpu.eax, address = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::servicePoll(app, cpu, service, address); });
}

/* sub_4f60b0, WINIPXSend, stack the size: eax the state, edx whom to (0 the state's own port),
 * ebx the packet's kind and ecx its bytes.  The kind goes first in network byte order: two
 * machines read it the same way.  1 when all of it went. */
reg32 frontend::neta::ipxSend(App* app, CPU& cpu, reg32 state, reg32 whom, reg32 kind, reg32 data, reg32 bytes)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x144);
    constexpr sreg32 kMostBytes = 0x118;
    constexpr reg32 kSockaddrBytes = 0xe;
    const reg32 frame = guest.frame();
    if (sreg32(bytes) > kMostBytes)
        bytes = reg32(kMostBytes);
    guest.setDword(frame + 0x12c, kind);
    const reg32 kindBytes = (kind >> 24) | ((kind >> 8) & 0xff00u) | ((kind << 8) & 0xff0000u) | (kind << 24);
    guest.setDword(frame, kindBytes);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, frame + 4, data, bytes);
    const reg32 to = frame + 0x11c;
    if (whom != 0)
        guest::direct<0x4f58c0, toSockaddr>(app, cpu, whom, to);
    else
        guest::direct<0x4f5900, portToSockaddr>(app, cpu, signedWord(guest, state + 0x24), to);
    enter(guest, cpu, kSocketLock);
    const reg32 sent = thunked<kSendTo, &host::socketSendTo>(guest, cpu, guest.dword(state + 0x58), frame,
                                                                 bytes + 4, 0, to, kSockaddrBytes);
    guest.setDword(frame + 0x134, sent);
    leave(guest, cpu, kSocketLock);
    if (sent == reg32(-1))
    {
        const reg32 error = thunked<kWsaGetLastError, &host::netLastError>(guest, cpu);
        const reg32 level = guest.dword(kLogLevel);
        guest.leaveEcx(level);
        guest.setDword(frame + 0x130, error);
        if (sreg32(level) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d284, frame + 0x130, state + 0x58, 4, 1, 0);
    }
    else if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
    {
        guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d290, frame + 0x12c, whom, 0x14, 0, 0);
    }
    guest.leaveEbx(guest.dword(frame + 0x134));
    return bytes + 4 == guest.dword(frame + 0x134) ? 1 : 0;
}

void frontend::netIpxSend(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f60b0", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 state = cpu.eax;
    const reg32 whom = cpu.edx;
    const reg32 kind = cpu.ebx;
    const reg32 data = cpu.ecx;
    const reg32 bytes = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxSend(app, cpu, state, whom, kind, data, bytes); });
}

/* sub_4f5950, one packet off the socket: eax the state, edx the packet's room.  The sender's
 * address kept, decrypted if the session has a key; its kind is its first 4 bytes, network byte
 * order.  0 when there is none. */
reg32 frontend::neta::ipxRecvFrom(App* app, CPU& cpu, reg32 state, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x24);
    constexpr reg32 kSockaddrBytes = 0xe, kRoom = 0x11c;
    const reg32 from = guest.frame(), fromBytes = from + 0x10, data = packet + 0x24;
    guest.setDword(fromBytes, kSockaddrBytes);
    const reg32 got = thunked<kRecvFrom, &host::socketRecvFrom>(guest, cpu, guest.dword(state + 0x58), data,
                                                                    kRoom, 0, from, fromBytes);
    guest.setDword(packet + 8, got);
    if (sreg32(got) <= 0)
        return 0;
    if (guest.byte(state + 0x26) != 0
        && guest::direct<0x512890, netb::ipxScanInterfaces>(app, cpu, state, from, data, guest.dword(packet + 8)) != 0)
        return 0;
    guest.setDword(packet + 4, guest.dword(kClock));
    const reg32 first = guest.dword(data);
    const reg32 kind = (first >> 24) | ((first >> 8) & 0xff00u) | ((first << 8) & 0xff0000u) | (first << 24);
    guest.setDword(packet + 0x20, kind);
    if (kind != 0 && sreg32(guest.dword(packet + 8)) >= 4 && guest.dword(fromBytes) == kSockaddrBytes)
    {
        guest::direct<0x4f5880, fromSockaddr>(app, cpu, from, packet + 0xc);
        if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d210, packet + 0x20, packet + 0xc, 0x14, 0, 0);
        return 1;
    }
    guest.setDword(packet + 8, 0);
    guest.setDword(packet + 0x20, 0);
    return 0;
}

void frontend::netIpxRecvFrom(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5950", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 state = cpu.eax, packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxRecvFrom(app, cpu, state, packet); });
}

/* sub_4c5b00, packet 8: what a machine wants of the race, edx the packet, eax whom from.  0 the
 * start: the host fixes the frame and sends it back; 1 and 2 agreeing and going on; 3 leaving. */
reg32 frontend::neta::quitPacket(App* app, CPU& cpu, reg32 from, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kQuitLock = 0x7d36d8, kStartFrame = 0x7d36c8, kStarted = 0x7d36c0, kHostQuit = 0x7d36c4;
    const reg32 what = guest.dword(packet + 4);
    switch (what)
    {
    case 0:
    {
        reg32 result;
        if (guest.word(kIsHost) != 0)
        {
            guest.leaveEdx(guest.dword(kQuitLock));
            enter(guest, cpu, kQuitLock);
            if (guest.dword(kStartFrame) == 0)
            {
                const reg32 frame = guest.call(kSyncFrame);
                guest.setDword(kStartFrame, frame);
                guest.setDword(packet + 8, frame);
                fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, 0xc);
            }
            guest.leaveEbx(guest.dword(kQuitLock));
            result = leave(guest, cpu, kQuitLock);
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
            enter(guest, cpu, kBarrierLock);
            const reg32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, cpu, from));
            guest.setWord(kMachineBarriers + machine * 2, 11);
            guest.leaveEdx(guest.dword(kBarrierLock));
            return leave(guest, cpu, kBarrierLock);
        }
        guest.setWord(kHostBarrier, 11);
        return what;
    default:
        return what;
    }
}

void frontend::netQuitPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c5b00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax, packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::quitPacket(app, cpu, from, packet); });
}

/* sub_4f5130, NET_'s entry to a service's call, stack what goes on: eax the record, edx the
 * service's table, ebx and ecx what it takes.  The record's defaults filled in, the low timer
 * set going for the first that wants polling and stopped with the last.  The result of a record
 * the session refuses is ebp as it came: the original never set it. */
reg32 frontend::neta::serviceCall(App* app, CPU& cpu, reg32 record, reg32 table, reg32 first, reg32 second, reg32 third)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kServicesUp = 0x566df4, kPolled = 0x566df8, kNothing = 0x4f50b0, kPoll = 0x4f5110;
    guest.leaveEdx(first);
    guest.leaveEbx(second);
    guest.leaveEcx(third);
    if (guest.dword(kServicesUp) == 0)
        guest::direct<0x4f5230, servicesReady>(app, cpu);
    if (record != 0)
    {
        if (guest.word(record + 0x2a) == 0)
            guest.setWord(record + 0x2a, 2);
        if (guest.word(record + 0x2c) == 0)
            guest.setWord(record + 0x2c, 4);
        if (guest.word(record + 0x2e) == 0)
            guest.setWord(record + 0x2e, 6);
        for (const reg32 field : { 0x14u, 0x1cu, 0x18u, 0x20u })
            if (guest.dword(record + field) == 0)
                guest.setDword(record + field, kNothing);
        guest.setDword(record + 4, table);
        if (guest.dword(table + 0xc) != 0)
        {
            if (guest.dword(kPolled) == 0)
                fe::direct<0x4f2930, frontend::eab::lowTimerAdd>(app, cpu, kPoll);
            guest.setDword(kPolled, guest.dword(kPolled) + 1);
        }
    }
    reg32 result = cpu.ebp;
    if (record == 0 || guest::direct<0x512660, netb::connInitFilter>(app, cpu, record) != 0)
        result = guest.call(guest.dword(table + 4), args({ record }));
    if (record != 0 && guest.dword(table + 0xc) != 0 && result == 0)
    {
        const reg32 left = guest.dword(kPolled) - 1;
        guest.leaveEcx(left);
        guest.setDword(kPolled, left);
        if (left == 0)
            fe::direct<0x4f29b0, ea::removeTimer>(app, cpu, kPoll);
    }
    return result;
}

void frontend::netServiceCall(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5130", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 record = cpu.eax;
    const reg32 table = cpu.edx;
    const reg32 first = cpu.ebx;
    const reg32 second = cpu.ecx;
    const reg32 third = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceCall(app, cpu, record, table, first, second, third); });
}

/* sub_4a68d0, packet 1 on a client: the host's list of machines taken on whole, 0x7c bytes.  On
 * RaceNet the machines on other links become endpoints. */
reg32 frontend::neta::setupPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kHadSetup = 0x7a2210, kSetupBytes = 0x7c;
    if (guest.dword(kHadSetup) != 0)
        enter(guest, cpu, kPlayersLock);
    for (reg32 i = 1; i <= 8; ++i)
        guest.setWord(kMachineGone - 2 + i * 2, 0);
    guest.leaveEdx(0);
    copyBytes(guest, kPacketSetup, packet, kSetupBytes);
    reg32 eax = kSetupBytes, ebx = 1;
    guest.setWord(kSetupDirty, 1);
    if (guest.dword(kLinkKind) == 2)
    {
        reg32 slot = 0;
        for (reg32 machine = 0; machine < 8; ++machine)
        {
            guest.leaveEbx(ebx);
            cpu.eax = eax;  // it keeps the rest of eax as the code has it at the call
            eax = guest::direct<0x4a5af0, thisLink>(app, cpu);
            if (reg8(eax) == guest.byte(machine + kSetupLinks))
                continue;
            const reg32 at = slot * kEndpointSize;
            eax = at;
            ebx = slot * 12;
            reg8 link = 0;
            if (sreg32(machine) < sreg32(signedWord(guest, kMachineCount)))
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
        guest.leaveEdx(8);
    }
    guest.leaveEbx(ebx);
    if (guest.dword(kHadSetup) != 0)
    {
        guest.leaveEdx(guest.dword(kPlayersLock));
        eax = leave(guest, cpu, kPlayersLock);
    }
    guest.setDword(kHadSetup, 1);
    return eax;
}

void frontend::netSetupPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a68d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::setupPacket(app, cpu, packet); });
}

/* sub_4f54f0, a service's open of a game: eax the game, edx what goes with it.  The game is told
 * to the service, then listed among the games under the lock; 0 at once for a service down. */
reg32 frontend::neta::serviceOpenGame(App* app, CPU& cpu, reg32 game, reg32 with)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kOpened = 0xa8;
    const reg32 service = guest.dword(game + 0x4c);
    guest.setDword(game + 8, with);
    const reg32 session = guest.dword(service + 0xc);
    if (session == 0)
    {
        guest.leaveEdx(0);
        guest.setDword(game + 0x48, 0);
        return guest.dword(game + 0x48);
    }
    guest.setDword(guest.frame(), game);
    const reg32 table = guest.dword(service + 8);
    guest.leaveEdx(table);
    if (guest.dword(table + 0x48) != 0)
        guest.call(guest.dword(table + 0x48), args({ guest.dword(service + 0xc), game + 0x34 }));
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edi = cpu.edi;
    guest.call(guest.dword(guest.dword(service + 8) + 0x5c), args({ guest.dword(service + 0xc), game + 0x34, kGameTag, 0 }));
    if (guest.dword(kServiceLock) == 0)
        guest.setDword(kServiceLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    enter(guest, cpu, kServiceLock);
    const reg32 at = kGames + (guest.dword(kGameCount) << 5);
    guest.setDword(at, game);
    guest.setDword(at + 4, service);
    copyBytes(guest, at + 8, game + 0x34, 20);
    guest.setDword(service + kOpened, guest.dword(service + kOpened) + 1);
    guest.setDword(kGameCount, guest.dword(kGameCount) + 1);
    guest.setDword(game + 0x48, 0xffffffff);
    guest.leaveEdx(guest.dword(kServiceLock));
    leave(guest, cpu, kServiceLock);
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edi = edi;
    return guest.dword(game + 0x48);
}

void frontend::netServiceOpenGame(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f54f0", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const reg32 game = cpu.eax, with = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceOpenGame(app, cpu, game, with); });
}

/* sub_49fb60: Windows' dial-up networking loaded and its functions found: 1 when they are all
 * there.  The port hides the modem; the game's code is kept. */
reg32 frontend::neta::rasLoad(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    constexpr reg32 kLoadLibraryA = 0x53458c, kGetProcAddress = 0x534558, kRasDll = 0x7a05c0;
    if (guest::direct<0x49fb10, rasLoaded>(app, cpu) != 0)
        return 1;
    const reg32 library = windows<kLoadLibraryA, &host::loadLibrary>(guest, cpu, 0x53c71c);
    guest.setDword(kRasDll, library);
    if (library == 0)
        return library;
    constexpr struct { reg32 name, place; } kFunctions[] = {
        { 0x53c728, 0x7a05c8 }, { 0x53c734, 0x7a05cc }, { 0x53c748, 0x7a05d0 },
        { 0x53c758, 0x7a05d4 }, { 0x53c770, 0x7a05d8 }, { 0x53c77c, 0x7a05dc },
    };
    for (const auto& function : kFunctions)
        guest.setDword(function.place, windows<kGetProcAddress, &host::procAddress>(guest, cpu, guest.dword(kRasDll), function.name));
    if (guest::direct<0x49fb10, rasLoaded>(app, cpu) != 0)
        return 1;
    guest::direct<0x49fc50, rasFree>(app, cpu);
    return 0;
}

void frontend::netRasLoad(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49fb60", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::rasLoad(app, cpu); });
}

/* sub_4f55e0, a service's close of a game: eax the game.  Taken out of the games' list under the
 * lock, the rest moved down over it, and the service told. */
reg32 frontend::neta::serviceCloseGame(App* app, CPU& cpu, reg32 game)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kOpened = 0xa8;
    const reg32 service = guest.dword(game + 0x4c);
    if (guest.dword(service + 0xc) == 0 || guest.dword(service + kOpened) == 0)
    {
        guest.setDword(game + 0x48, 0);
        return 1;
    }
    enter(guest, cpu, kServiceLock);
    const reg32 end = guest.dword(kGameCount) << 5;
    reg32 found = 0;
    if (game != guest.dword(kGames))
    {
        for (reg32 at = 0; sreg32(at) < sreg32(end);)
        {
            const reg32 next = guest.dword(at + kGames + 0x20);
            at += 0x20;
            ++found;
            if (next == game)
                break;
        }
    }
    const reg32 last = guest.dword(kGameCount) - 1;
    guest.setDword(service + kOpened, guest.dword(service + kOpened) - 1);
    guest.setDword(kGameCount, last);
    const reg32 to = kGames + (found << 5);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, to + 0x20, (last - found) << 5);
    leave(guest, cpu, kServiceLock);
    guest.call(guest.dword(guest.dword(service + 8) + 0x5c), args({ guest.dword(service + 0xc), game + 0x34, kGameTag, 1 }));
    guest.setDword(game + 0x48, 0);
    return 1;
}

void frontend::netServiceCloseGame(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f55e0", 0, 0, nullptr, 0 };
    const reg32 game = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceCloseGame(app, cpu, game); });
}

/* sub_49b310, packet 3: another machine's player record, 0x60 bytes, taken on unless it is this
 * machine's own; the host passes it on unless RaceNet does. */
reg32 frontend::neta::playerPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    if (guest.word(kSessionOver) != 0)
        return guest.eax();
    guest.leaveEdx(guest.dword(kPlayersLock));
    enter(guest, cpu, kPlayersLock);
    if (std::int16_t(guest.word(kThisMachine)) >= 0
        && widened(directKept<0x4a6690, ea::machineOfPlayerNumber, 1>(guest.app(), guest.cpu(), signedWord(guest, kThisMachine))) == guest.dword(packet + 4))
    {
        guest.leaveEbx(guest.dword(kPlayersLock));
        return leave(guest, cpu, kPlayersLock);
    }
    const reg32 machine = guest.dword(packet + 4);
    guest.leaveEdx(machine);
    copyBytes(guest, kPlayerRecords + machine * kPlayerRecordSize, packet, kPlayerRecordSize);
    guest.setWord(kPlayersNew, 1);
    const reg32 left = leave(guest, cpu, kPlayersLock);
    if (guest.word(kIsHost) == 0 || guest.dword(kLinkKind) == 2)
        return left;
    return sendLeaving(app, cpu, 0, packet, kPlayerRecordSize);
}

void frontend::netPlayerPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b310", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::playerPacket(app, cpu, packet); });
}

/* sub_49f880: dial-up networking's phone book: eax where its names go, 21 bytes each, or 0.  How
 * many there are, 0 when it cannot be asked. */
reg32 frontend::neta::rasPhoneBook(App* app, CPU& cpu, reg32 names)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    constexpr reg32 kRasEnumEntries = 0x7a05d0, kEntryCount = 0x7a05e4, kEntries = 0x7a05e8, kEntrySize = 0x108;
    constexpr reg32 kBufferTooSmall = 0x25b, kErrorMoreData = 8;
    const reg32 bytes = guest.frame();
    guest.setDword(bytes, 0x19c0);
    const reg32 loaded = guest::direct<0x49fb60, rasLoad>(app, cpu);
    if (loaded == 0)
        return 0;
    guest.setDword(kEntryCount, 0);
    guest.setDword(kEntries, kEntrySize);
    const reg32 answer = guest.call(guest.dword(kRasEnumEntries), args({}, { 0, 0, kEntries, bytes, kEntryCount }));
    if (answer != 0 && answer != kErrorMoreData && answer != kBufferTooSmall)
        return 0;
    if (names != 0)
    {
        for (reg32 i = 0; i < guest.dword(kEntryCount); ++i)
        {
            const reg32 name = names + i * 21;
            const reg32 entry = kEntries + i * kEntrySize + 4;
            guest.setDword(bytes + 4, entry);
            fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), name, entry, 0x14);
            guest.setByte(name + 0x14, 0);
        }
    }
    return guest.dword(kEntryCount);
}

void frontend::netRasPhoneBook(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49f880", 0, 0, nullptr, 0 };
    const reg32 names = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::rasPhoneBook(app, cpu, names); });
}

/* sub_4f5710, a game's read: eax the game, edx the buffer, ebx its room.  Packets off the service
 * until one gives something for the caller; the number of bytes the last gave. */
reg32 frontend::neta::serviceRead(App* app, CPU& cpu, reg32 game, reg32 buffer, reg32 room)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x38);
    const reg32 frame = guest.frame();
    const reg32 service = guest.dword(game + 0x4c);
    guest.setDword(frame + 0x20, buffer);
    guest.setDword(frame + 0x1c, room);
    guest.setDword(frame + 0x24, game);
    const reg32 tag = frame + 0x18, more = frame + 0x14;
    for (;;)
    {
        guest.setDword(tag, kGameTag);
        copyBytes(guest, frame, game + 0x34, 20);
        const reg32 received = guest.call(guest.dword(guest.dword(service + 8) + 0x58),
                                          args({ guest.dword(service + 0xc), frame, tag, buffer }, { room }));
        if (received == 0)
            return 0;
        const reg32 answer = guest.call(guest.dword(guest.dword(service + 4) + 0x40),
                                        args({ service, frame, guest.dword(tag), buffer }, { received, more }));
        const reg32 still = guest.dword(more);
        guest.leaveEbx(still);
        guest.leaveEdx(answer);
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

void frontend::netServiceRead(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5710", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 game = cpu.eax, buffer = cpu.edx, room = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceRead(app, cpu, game, buffer, room); });
}

/* sub_4c7ea0, after a network race: each machine's times sent to the others, packet 12: this
 * machine's car, the host's the AI cars' as well, a barrier either side. */
reg32 frontend::neta::sendRaceResults(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xd8);
    constexpr reg32 kResults = 0x224, kResultsBytes = 0xc0, kPacketBytes = 0xc4;
    constexpr reg32 kThisPlayer = 0x6fd4f4;
    if (guest.dword(0x6fd50c) == 0 || sreg32(guest.dword(0x6fd3b0)) <= 1)
        return guest.eax();
    const reg32 packet = guest.frame();
    guest::direct<0x4a6a10, barrier>(app, cpu, 0x5a, 0x22);
    guest.setByte(packet, 0xc);
    guest.setByte(packet + 1, guest.byte(kThisPlayer));
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, packet + 4, guest.dword(kCars + guest.dword(kThisPlayer) * 4) + kResults, kResultsBytes);
    fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, kPacketBytes);
    if (guest.word(kIsHost) != 0)
    {
        for (reg32 player = guest.dword(kRacePlayerCount); sreg32(player) < sreg32(guest.dword(0x6fd510)); ++player)
        {
            guest.setByte(packet + 1, reg8(player));
            const reg32 car = guest.dword(kCarsByPlayer + player * 4);
            if (car == 0)
                continue;
            fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, packet + 4, car + kResults, kResultsBytes);
            fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
            fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, kPacketBytes);
        }
    }
    return guest::direct<0x4a6a10, barrier>(app, cpu, 0x5a, 0x23);
}

void frontend::netSendRaceResults(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7ea0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendRaceResults(app, cpu); });
}

/* sub_49aa80: the host's settings sent to the others, packet 2, 0x28 bytes. */
reg32 frontend::neta::sendSettings(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kPacket = 0x79f8f0, kOpponentCar = 0x79f8f6, kOpponentId = 0x79f8f8;
    guest.setByte(kPacket, 2);
    for (const SettingField& field : kSettingFields)
        guest.setByte(field.packed, guest.byte(field.setting));
    guest.setDword(kSettingTrackPacked, guest.dword(kSettingTrack));
    guest.setByte(kSettingLastPacked, guest.byte(kSettingLast));
    const reg8 car = guest.byte(kOpponentCar);
    if (car >= 4)
        copyBytes(guest, kOpponentId, reg32(*fe::direct<0x439520, load::idOf>(app, cpu, reg32(car) - 4)), 16);
    return fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, kPacket, 0x28)->eax;
}

void frontend::netSendSettings(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49aa80", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendSettings(app, cpu); });
}

/* sub_4c8250, each race tick: what has come in of each machine's controls moved to where the
 * race reads them, eax the tick; a machine far ahead of it wakes the race's controls. */
reg32 frontend::neta::takeInputs(App* app, CPU& cpu, reg32 tick)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    constexpr reg32 kRows = 0x7dd070, kLocalControls = 0x7dd010, kThisPlayer = 0x6fd4f4, kRaceTick = 0x7d36ac;
    constexpr reg32 kWakeControls = 0x42dc40;
    enter(guest, cpu, kInputLock);
    for (reg32 player = 0; sreg32(player) < sreg32(guest.dword(kRacePlayerCount)); ++player)
    {
        if (player == guest.dword(kThisPlayer))
            continue;
        const reg32 row = kRows + player * 20;
        if (guest.dword(row + 0x10) == 0)
            continue;
        const reg32 itsTick = guest.dword(row + 0xc);
        if (sreg32(tick - itsTick) < 0 && sreg32(guest.dword(kRaceTick) + 0x20) < sreg32(itsTick))
            guest.call(kWakeControls);
        const reg32 local = kLocalControls + player * 12;
        copyBytes(guest, local, row, 8);
        guest.setByte(local + 8, guest.byte(row + 8));
        guest.setDword(row + 0x10, 0);
    }
    return leave(guest, cpu, kInputLock);
}

void frontend::netTakeInputs(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c8250", 0, 0, nullptr, 0 };
    const reg32 tick = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::takeInputs(app, cpu, tick); });
}

/* sub_4f5ea0: a game's IPX socket closed, eax the state: the thread told to end and waited for,
 * the queues and the state freed. */
reg32 frontend::neta::ipxClose(App* app, CPU& cpu, reg32 state)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kQueueFree = 0x50dde0;
    if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
        guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d258, 0, 0, 0, 0, 0);
    const reg32 event = guest.dword(state + 0x1c);
    guest.setDword(state + 0x20, 0);
    fe::direct<0x4dfb30, ea::eventSet>(app, cpu, event);
    if (thunked<kCloseSocket, &host::socketClose>(guest, cpu, guest.dword(state + 0x58)) == reg32(-1))
    {
        const reg32 error = thunked<kWsaGetLastError, &host::netLastError>(guest, cpu);
        const reg32 errorPlace = guest.frame() + 8;
        guest.setDword(errorPlace, error);
        if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d268, errorPlace, 0, 0, 1, 0);
    }
    while (guest.dword(state + 0x1c) != 0)
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), state + 0x70);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), state + 0x8c);
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, state);
    return 1;
}

void frontend::netIpxClose(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5ea0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 state = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxClose(app, cpu, state); });
}

/* sub_4a5e50, the callback NPS calls with what has happened, stack the session, the message and
 * its data: 0x202 a game gone, 0x203 a player: on a client whether it is in, for the host a
 * player to take on.  Every other message is nothing. */
reg32 frontend::neta::npsMessage(App* app, CPU& cpu, reg32 /*s0*/, reg32 message, reg32 data)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    guest.leaveEdx(data);
    switch (message & 0xffff)
    {
    case 0x202:
        return guest::direct<0x4a5f30, endpointOfLink>(app, cpu, guest.dword(data));
    case 0x203:
        if (guest.word(kIsHost) == 0)
        {
            if (guest.byte(data + 0x64) != 1)
                return message;
            guest.setDword(kEndpointUsed, 1);
            const reg8 link = guest.byte(data);
            guest.setByte(kEndpointLink, link);
            return (message & 0xffffff00u) | link;
        }
        else
        {
            const reg32 mine = guest.dword(0x7a20a0);
            if (mine == guest.dword(data))
                return mine;
            const reg32 player = guest.byte(data);
            guest.leaveEcx(player);
            return guest::direct<0x49e320, pendingAdd>(app, cpu, data + 4, player);
        }
    default:
        return message;
    }
}

void frontend::netNpsMessage(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5e50", 0, 0, nullptr, 0, 0 };
    const reg32 word1 = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 data = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::npsMessage(app, cpu, word1, message, data); });
}

/* sub_4a6500: the session's state back to nothing: the barriers and who has gone cleared, the
 * machines' records and the setup's memory, every endpoint. */
reg32 frontend::neta::sessionReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kEndpointsBytes = 0x34c, kMemoryFlags = 0x5643f4;
    guest.setDword(0x566df0, 0x780);
    for (reg32 i = 1; i <= 0x13; ++i)
        guest.setWord(0x7a2292 + i * 2, 0);
    for (reg32 i = 1; i <= 8; ++i)
    {
        guest.setWord(0x7a22b8 + i * 2, 0);
        guest.setWord(0x7a22c8 + i * 2, 0);
    }
    guest.setDword(0x7a2210, 0);
    // The game's code hands it 8, the endpoints' size, 0x10 and 0 in its four registers, which it does
    // not read.
    guest::direct<0x49a490, makeLocks>(app, cpu);
    guest.setDword(kMemoryFlags, 0x10);
    guest::direct<0x4f5230, servicesReady>(app, cpu);
    guest.setDword(kMemoryFlags, 0);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kEndpoints, kEndpointsBytes);
    reg32 last = 0;
    for (reg32 i = 0; i < 7; ++i)
        last = guest::direct<0x4a6410, endpointHandlers>(app, cpu, kEndpoints + i * kEndpointSize);
    return last;
}

void frontend::netSessionReset(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6500", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sessionReset(app, cpu); });
}

namespace
{

/* sub_49b3e0 and sub_49b470, one for a host and one for a client: a car's file on its way, the
 * car's id, its time and how big it is.  Already here, it is done; otherwise its receiving is
 * begun, on the machine that does not send. */
reg32 carFileComing(Guest& guest, reg32 packet, bool received)
{
    constexpr reg32 kFileDone = 0x55e5bc, kFileId = 0x79f5e0, kFileTime = 0x79faa8, kBeginCarReceive = 0x49b540;
    guest.leaveEdx(0);
    guest.setDword(kFileDone, 0);
    if (!received)
    {
        guest.setDword(kFileDone, 1);
        return guest.eax();
    }
    const reg32 id = packet + 1;
    guest.leaveEdx(id);
    copyBytes(guest, kFileId, id, 0x10);
    guest.setDword(kFileTime, guest.dword(packet + 0x14));
    const reg32 car = fe::direct<0x439990, load::placeById>(guest.app(), guest.cpu(), id);
    if (car != reg32(-1))
    {
        guest.setDword(kFileDone, 1);
        return car;
    }
    return guest.call(kBeginCarReceive, args({ kFileDone, 0, guest.dword(packet + 0x18) }));
}

}  // namespace

reg32 frontend::neta::carFileToHost(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    return carFileComing(guest, packet, guest.word(kIsHost) != 0);
}

void frontend::netCarFileToHost(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b3e0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::carFileToHost(app, cpu, packet); });
}

reg32 frontend::neta::carFileToClient(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    return carFileComing(guest, packet, guest.word(kIsHost) == 0);
}

void frontend::netCarFileToClient(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b470", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::carFileToClient(app, cpu, packet); });
}

/* sub_49e220: joining a RaceNet game: the session reset and NPS opened, the wait for the host's
 * list, then the versions compared.  The message 0xc2 when any of it does not happen. */
reg32 frontend::neta::joinRaceNet(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kResetNetwork = 0x4a6390, kFailed = 0xc2;
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.cpu(), 1, 0, 0xfffffffe);
    guest.setWord(kIsHost, 0);
    guest.setWord(kThisMachine, 0xfffe);
    guest.setWord(kMachineCount, 0);
    const reg32 started = guest::direct<0x49e080, hostStart>(app, cpu);
    if (started == 0)
        return started;
    while (guest.word(kSetupDirty) == 0)
    {
        if (guest::direct<0x4a65b0, hostGone>(app, cpu) != 0 || guest.word(kSessionOver) != 0)
        {
            fe::direct<0x444be0, frontend::dialogs::messageById>(app, cpu, kFailed);
            guest.setWord(kSessionOver, 1);
            directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.cpu());
            return 0;
        }
    }
    guest.setDword(kLinkKind, guest.dword(kSetupOther));
    if (guest::direct<0x4a6ce0, compareVersions>(app, cpu, 0) == 0)
    {
        // The game's code hands it 1 in edx too, which it does not read.
        fe::direct<0x444be0, frontend::dialogs::messageById>(app, cpu, kFailed);
        guest.setWord(kSessionOver, 1);
        directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.cpu());
        return 0;
    }
    return 1;
}

void frontend::netJoinRaceNet(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49e220", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::joinRaceNet(app, cpu); });
}

/* sub_4a6c40, packet 4: the barrier a machine has come to.  The host notes it against that
 * machine; a client takes it as the host's, and 12 as the host having given up on it. */
reg32 frontend::neta::barrierPacket(App* app, CPU& cpu, reg32 from, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kGaveUp = 0xc;
    if (guest.word(kIsHost) != 0)
    {
        guest.leaveEdx(guest.dword(kBarrierLock));
        enter(guest, cpu, kBarrierLock);
        const reg32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, cpu, from));
        if (sreg32(machine) >= 0)
        {
            guest.leaveEdx(machine * 2);
            guest.leaveEbx(guest.dword(kBarrierLock));
            guest.setWord(kMachineBarriers + machine * 2, guest.word(packet + 2));
        }
        return leave(guest, cpu, kBarrierLock);
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
    enter(guest, cpu, kBarrierLock);
    guest.setWord(kHostBarrier, guest.word(packet + 2));
    return leave(guest, cpu, kBarrierLock);
}

void frontend::netBarrierPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6c40", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax, packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::barrierPacket(app, cpu, from, packet); });
}

/* sub_4f4c70, NET_getservices: eax a function called with each service there is, edx and ebx what
 * goes to it, ecx how big its room is.  A service with its own list is asked for it. */
reg32 frontend::neta::getServices(App* app, CPU& cpu, reg32 callback, reg32 one, reg32 two, reg32 room)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    constexpr reg32 kServiceCount = 0x566de4, kServiceNames = 0x9f25d0, kServiceTables = 0x9f25b0;
    constexpr reg32 kBigEnough = 0x20;
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.cpu(), 0x54d0c0, 1);
    reg32 going = 1;
    guest.leaveEcx(0);
    guest.leaveEbx(guest.dword(kServiceCount));
    if (sreg32(guest.dword(kServiceCount)) > 0)
    {
        for (reg32 i = 0; going != 0;)
        {
            const reg32 table = guest.dword(kServiceTables + i * 4);
            if (guest.dword(table + 4) != 0)
            {
                going = guest.call(guest.dword(table + 4), args({ callback, one, two, room }));
            }
            else if (room <= kBigEnough)
            {
                const reg32 name = guest.dword(kServiceNames + i * 4);
                going = guest.call(callback, args({ name + 4, guest.dword(name), 0xfa, 0 }));
            }
            guest.leaveEdx(going);
            ++i;
            const reg32 count = guest.dword(kServiceCount);
            guest.leaveEcx(count);
            if (sreg32(i) >= sreg32(count))
                break;
        }
    }
    return going;
}

void frontend::netGetServices(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f4c70", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 callback = cpu.eax, one = cpu.edx, two = cpu.ebx, room = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::getServices(app, cpu, callback, one, two, room); });
}

/* sub_4f5b10, the IPX thread, by its state in 0x9f3650: a packet taken off the free list and read
 * off the socket, onto the received list; with none free it waits for one. */
reg32 frontend::neta::ipxThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kThreadStarting = 0x9f3650, kCloseEvent = 0x4dfc00;
    const reg32 state = guest.dword(kThreadStarting);
    const reg32 event = reg32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu));
    guest.setDword(kThreadStarting, 0);
    const reg32 running = guest.dword(state + 0x20);
    guest.setDword(state + 0x1c, event);
    if (running != 0)
    {
        const reg32 freeQueue = state + 0x70, received = state + 0x8c;
        do
        {
            const reg32 packet = guest::direct<0x50df60, netb::qpop>(app, cpu, freeQueue);
            if (packet == 0)
            {
                fe::direct<0x4dfb70, ea::eventWaited>(app, cpu, guest.dword(state + 0x1c));
            }
            else if (guest::direct<0x4f5950, ipxRecvFrom>(app, cpu, state, packet) == 0)
            {
                guest::direct<0x50de50, netb::qpush>(app, cpu, freeQueue, packet);
            }
            else
            {
                guest::direct<0x50ded0, netb::qbottom>(app, cpu, received, packet);
                if (guest.dword(state) != 0)
                    guest.call(guest.dword(state), args({ guest.dword(state + 4), packet + 0xc }));
            }
        } while (guest.dword(state + 0x20) != 0);
    }
    const reg32 closed = directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.cpu(), guest.dword(state + 0x1c));
    guest.setDword(state + 0x1c, 0);
    return closed;
}

void frontend::netIpxThread(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5b10", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxThread(app, cpu); });
}

/* sub_4f57c0, a game's write: eax the game, edx the bytes, ebx how many.  A game opening says who
 * it is, every few writes, before it writes; ebx when it went, 0 when not. */
reg32 frontend::neta::serviceWrite(App* app, CPU& cpu, reg32 game, reg32 bytes, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    constexpr reg32 kOpenTag = 0xea6f706e;
    const reg32 frame = guest.frame();
    guest.setDword(frame, bytes);
    guest.setDword(frame + 8, game);
    guest.setDword(frame + 4, kGameTag);
    if (sreg32(guest.dword(game + 0x48)) < 0)
    {
        if (sreg32(guest.dword(game + 0x48)) >= -4)
        {
            guest.setDword(game + 0x48, guest.dword(game + 0x48) - 1);
        }
        else
        {
            guest.setDword(frame + 4, kOpenTag);
            const reg32 service = guest.dword(game + 0x4c);
            const reg32 table = guest.dword(service + 8);
            if (guest.call(guest.dword(table + 0x54), args({ guest.dword(service + 0xc), game + 0x34, kOpenTag, 0 }, { 0 })) != 0)
                guest.setDword(game + 0x48, 0xffffffff);
        }
    }
    const reg32 service = guest.dword(guest.dword(frame + 8) + 0x4c);
    const reg32 table = guest.dword(service + 8);
    const reg32 sent = guest.call(guest.dword(table + 0x54),
                                  args({ guest.dword(service + 0xc), guest.dword(frame + 8) + 0x34, guest.dword(frame + 4), guest.dword(frame) }, { size }));
    return sent != 0 ? size : 0;
}

void frontend::netServiceWrite(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f57c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 game = cpu.eax, bytes = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceWrite(app, cpu, game, bytes, size); });
}

/* sub_49f790: whether dial-up networking is there: a network card, or the DLL and a connection
 * of it.  The port hides the modem; the game's code is kept. */
reg32 frontend::neta::rasAvailable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1b0);
    constexpr reg32 kRasEnumConnections = 0x7a05cc, kBytes = 0x19c;
    const reg32 connections = guest.frame(), bytes = connections + 0x19c, count = connections + 0x1a0;
    guest.setDword(bytes, kBytes);
    guest.setDword(count, 0);
    if (guest::direct<0x49f810, networkThere>(app, cpu) != 0)
        return 1;
    const reg32 loaded = guest::direct<0x49fb60, rasLoad>(app, cpu);
    if (loaded == 0)
        return loaded;
    guest.setDword(connections, kBytes);
    guest.call(guest.dword(kRasEnumConnections), args({}, { connections, bytes, count }));
    if (guest.dword(bytes) != 0 || guest.dword(count) != 0)
    {
        guest::direct<0x49fc50, rasFree>(app, cpu);
        return 1;
    }
    return 0;
}

void frontend::netRasAvailable(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49f790", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::rasAvailable(app, cpu); });
}

/* sub_4f6030: whether the socket has anything: eax the state.  The received queue, then Winsock's
 * count of what waits. */
reg32 frontend::neta::ipxReady(App* app, CPU& cpu, reg32 state)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kQueueLength = 0x50e1d0, kFionRead = 0x4004667f;
    const reg32 frame = guest.frame();
    if (directKept<0x50e1d0, ea::queueLengthOf, 1>(guest.app(), guest.cpu(), state + 0x8c) != 0)
        return 1;
    const reg32 answer = thunked<kIoctlSocket, &host::socketControl>(guest, cpu, guest.dword(state + 0x58),
                                                                             kFionRead, frame + 4);
    guest.setDword(frame, answer);
    if (answer == reg32(-1))
    {
        if (sreg32(guest.dword(kLogLevel)) >= kEnoughLog)
            guest::direct<0x512180, netb::commEvent>(app, cpu, 0x54d274, frame, state + 0x58, 4, 1, 0);
        return 0;
    }
    return guest.dword(frame + 4) != 0 ? 1 : 0;
}

void frontend::netIpxReady(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f6030", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 state = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxReady(app, cpu, state); });
}

/* sub_4a5dd0: NPS let go: what the session has open, through the DLL's own pointers, then the
 * session, whose number goes to -20. */
reg32 frontend::neta::npsStop(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    constexpr reg32 kNpsSession = 0x55ecb0, kNpsClose = 0x7a21e0, kNpsEnd = 0x7a21cc, kNpsVersion = 0x7a215c, kNpsOther = 0x7a2160;
    if (sreg32(guest.dword(kNpsSession)) < 0)
        return guest.eax();
    for (const reg32 what : { kNpsVersion, kNpsOther })
    {
        const reg32 open = guest.dword(what);
        if (open != 0)
        {
            guest.call(guest.dword(kNpsClose), args({}, { guest.dword(kNpsSession), open }));
            guest.setDword(what, 0);
        }
    }
    const reg32 ended = guest.call(guest.dword(kNpsEnd), args({}, { guest.dword(kNpsSession) }));
    guest.setDword(kNpsSession, 0xffffffec);
    return ended;
}

void frontend::netNpsStop(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5dd0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::npsStop(app, cpu); });
}

/* sub_4f4bb0: whether two GUIDs, at eax and edx, are the same. */
reg32 frontend::neta::sameGuid(App* app, CPU& cpu, reg32 one, reg32 other)
{
    Guest guest(app, cpu);
    const reg32 first = guest.dword(other);
    guest.leaveEdx(first);
    if (first != guest.dword(one))
        return 0;
    const reg16 second = guest.word(other + 4);
    guest.leaveEdx((first & 0xffff0000u) | second);
    if (second != guest.word(one + 4) || guest.word(other + 6) != guest.word(one + 6))
        return 0;
    for (reg32 i = 0; i < 8; ++i)
    {
        guest.leaveEdx(i);
        if (guest.byte(one + 8 + i) != guest.byte(other + 8 + i))
            return 0;
    }
    guest.leaveEdx(8);
    return 1;
}

void frontend::netSameGuid(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f4bb0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 one = cpu.eax, other = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sameGuid(app, cpu, one, other); });
}

/* sub_4f5390: which of the services' games a packet is for, eax the service, edx the sender's
 * address: -1 for none. */
reg32 frontend::neta::findGame(App* app, CPU& cpu, reg32 service, reg32 address)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kCheckPlayer = 0x4f5040;
    guest.leaveEdx(guest.dword(kGameCount));
    if (sreg32(guest.dword(kGameCount)) <= 0)
        return reg32(-1);
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kGameCount)); ++i)
    {
        const reg32 at = i << 5;
        if (service == guest.dword(kGames + 4 + at)
            && guest.call(kCheckPlayer, args({ service, address, guest.dword(kGames + at) + 0x34 })) != 0)
            return i;
    }
    return reg32(-1);
}

void frontend::netFindGame(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5390", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 service = cpu.eax, address = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::findGame(app, cpu, service, address); });
}

/* sub_49f810: whether the machine has a network: Windows' word for it, or Winsock opening. */
reg32 frontend::neta::networkThere(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x19c);
    constexpr reg32 kGetSystemMetrics = 0x534750, kSmNetwork = 0x3f;
    if ((windows<kGetSystemMetrics, &host::systemMetrics>(guest, cpu, kSmNetwork) & 1) != 0)
        return 1;
    if (thunked<kWsaStartup, &host::netStartup>(guest, cpu, 0x101, guest.frame()) != 0)
        return 0;
    const reg32 socket = thunked<kSocket, &host::socketOpen>(guest, cpu, 2, 1, 0);
    if (socket == reg32(-1))
    {
        thunked<kWsaCleanup, &host::netCleanup>(guest, cpu);
        return 0;
    }
    thunked<kCloseSocket, &host::socketClose>(guest, cpu, socket);
    thunked<kWsaCleanup, &host::netCleanup>(guest, cpu);
    return 1;
}

void frontend::netNetworkThere(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49f810", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::networkThere(app, cpu); });
}

/* sub_4c7fd0: a machine's ring of controls emptied, eax the player; for a local one nothing. */
reg32 frontend::neta::clearRing(App* app, CPU& cpu, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x18);
    constexpr reg32 kRingControls = 0x7dd110;
    if (sreg32(player) < sreg32(guest.dword(kCarCount)))
        return player;
    enter(guest, cpu, kInputLock);
    for (reg32 i = 0; i < 20; ++i)
        guest.setWord(kRingControls + (player - guest.dword(kCarCount)) * 2 + i * 20, 0);
    return leave(guest, cpu, kInputLock);
}

void frontend::netClearRing(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7fd0", 0, 0, nullptr, 0 };
    const reg32 player = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::clearRing(app, cpu, player); });
}

/* sub_4f5a30, a service's listing asked for and answered: eax the state, edx whom to, ebx the
 * name asked after.  When it is this game's, the name goes back with its terminator. */
reg32 frontend::neta::ipxAnswerName(App* app, CPU& cpu, reg32 state, reg32 whom, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    constexpr reg32 kStrncmp = 0x4eb390, kNameTag = 0xea737276;
    guest.setDword(guest.frame() + 8, whom);
    const reg32 differs = fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.cpu(), name, state + 0x26, 0x30);
    if (differs != 0)
        return differs;
    return guest::direct<0x4f60b0, ipxSend>(app, cpu, state, whom, kNameTag, name, guest.length(name) + 1);
}

void frontend::netIpxAnswerName(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5a30", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 state = cpu.eax, whom = cpu.edx, name = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxAnswerName(app, cpu, state, whom, name); });
}

/* sub_49f730: the next one waiting to join, off the ring of 6 at 0x7a01f0, 31 bytes each, to eax. */
reg32 frontend::neta::nextPending(App* app, CPU& cpu, reg32 to)
{
    Guest guest(app, cpu);
    constexpr reg32 kRingAt = 0x7a01e4, kRingCount = 0x7a01ec, kRing = 0x7a01f0;
    constexpr sreg32 kRingSize = 6;
    copyBytes(guest, to, kRing + guest.dword(kRingAt) * 31, 31);
    const sreg32 next = sreg32(guest.dword(kRingAt) + 1);
    guest.setDword(kRingCount, guest.dword(kRingCount) - 1);
    guest.setDword(kRingAt, reg32(next % kRingSize));
    return reg32(next / kRingSize);
}

void frontend::netNextPending(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49f730", 0, 0, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 to = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::nextPending(app, cpu, to); });
}

/* sub_49ee50, as a host's lobby closes: no one else may join: its game shut, unless RaceNet's. */
reg32 frontend::neta::lobbyClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kSessionIndex = 0x49f1c0, kShutGame = 0x4f4f50;
    reg32 eax = guest.eax();
    if (guest.word(kIsHost) == 0 || guest.word(kSessionOver) != 0 || guest.dword(0x55e860) == 0 || guest.dword(kSessionStage) != 3)
        return eax;
    if (guest.dword(kLinkKind) != 2)
    {
        eax = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.cpu(), guest.dword(0x55e71c)) * 11;
        if (guest.dword(0x55e738 + eax * 4) != 0)
            eax = guest.call(kShutGame, args({ 0x7a0390, 0 }));
    }
    guest.setDword(0x55e710, 0);
    guest.setDword(0x55e860, 0);
    return eax;
}

void frontend::netLobbyClose(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49ee50", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::lobbyClose(app, cpu); });
}

/* sub_4a5f30: the endpoint of a link's number, eax: 0 for none. */
reg32 frontend::neta::endpointOfLink(App* app, CPU& cpu, reg32 link)
{
    Guest guest(app, cpu);
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 at = i * kEndpointSize;
        if (guest.byte(kEndpointLink + at) == link)
            return kEndpoints + at;
    }
    return 0;
}

void frontend::netEndpointOfLink(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5f30", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 link = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::endpointOfLink(app, cpu, link); });
}

/* sub_477f60, a race's loading step: its messages cleared, the controls started, and in a network
 * race the settings exchanged, unless asked to stop along the way. */
reg32 frontend::neta::raceLoadStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kClearCarSlots = 0x42c660, kStartControls = 0x48f4f0;
    guest::direct<0x4c45d0, clearInputCounts>(app, cpu);
    guest.call(kClearCarSlots);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    if (reg16(fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu)) == 0)
        guest.call(kStartControls);
    const reg32 stop = fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    if (reg16(stop) != 0)
        return stop;
    const reg32 kind = guest.dword(0x6fd3b8);
    const reg32 players = guest.dword(0x6fd3b0);
    guest.leaveEdx(players);
    if (players == 0 || players == 1 || guest.dword(0x6fd3a0) != 0)
        return kind;
    return guest::direct<0x4c5e80, raceSettings>(app, cpu);
}

void frontend::netRaceLoadStep(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_477f60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::raceLoadStep(app, cpu); });
}

/* sub_49b2b0, packet 2: the host's settings, 0x28 bytes, kept for the lobby to take on. */
reg32 frontend::neta::settingsPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    if (guest.word(kSessionOver) != 0)
        return guest.eax();
    guest.leaveEdx(guest.dword(kSetupLock));
    enter(guest, cpu, kSetupLock);
    copyBytes(guest, 0x79f8f0, packet, 0x28);
    guest.leaveEbx(1);
    guest.setWord(kSetupNew, 1);
    return leave(guest, cpu, kSetupLock);
}

void frontend::netSettingsPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b2b0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::settingsPacket(app, cpu, packet); });
}

/* sub_4f5ac0, a walk over the received queue: eax a packet, edx the kind and sender asked after:
 * one that is theirs is marked for throwing away. */
reg32 frontend::neta::markPacket(App* app, CPU& cpu, reg32 packet, reg32 asked)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    if (guest.dword(packet + 0x20) == guest.dword(asked))
    {
        const reg32 service = guest.dword(asked + 0x18);
        bool theirs = service == 0;
        if (!theirs)
        {
            guest.leaveEdx(packet + 0xc);  // as the call by address leaves it
            theirs = guest::direct<0x4f5fe0, sameNode>(app, cpu, packet + 0xc, asked + 4) != 0;
        }
        if (theirs)
            guest.setDword(packet + 4, 0);
    }
    return 1;
}

void frontend::netMarkPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5ac0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.eax, asked = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::markPacket(app, cpu, packet, asked); });
}

/* sub_49ede0, as a host's lobby opens: its game made known, unless RaceNet's. */
reg32 frontend::neta::lobbyOpen(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kSessionIndex = 0x49f1c0;
    reg32 eax = guest.eax();
    if (guest.word(kIsHost) == 0 || guest.word(kSessionOver) != 0 || guest.dword(0x55e860) != 0 || guest.dword(kSessionStage) != 3)
        return eax;
    if (guest.dword(kLinkKind) != 2)
    {
        eax = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.cpu(), guest.dword(0x55e71c)) * 11;
        if (guest.dword(0x55e738 + eax * 4) != 0)
            eax = guest::direct<0x4f4f70, openGame>(app, cpu, 0x7a0390);
    }
    guest.setDword(0x55e860, 1);
    return eax;
}

void frontend::netLobbyOpen(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49ede0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::lobbyOpen(app, cpu); });
}

/* sub_4f4e60, NET_killservice: eax the record, 0 when NET_ does not have it. */
reg32 frontend::neta::killService(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kServicesQueue = 0x9f25f0, kQueueRemove = 0x50e150;
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.cpu(), 0x54d0e0);
    if (guest.call(kQueueFindPosition, args({ kServicesQueue, service })) == reg32(-1))
        return 0;
    const reg32 result = guest.call(guest.dword(guest.dword(service + 4) + 0xc), args({ service, service }));
    guest.call(kQueueRemove, args({ kServicesQueue }));
    return result;
}

void frontend::netKillService(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f4e60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::killService(app, cpu, service); });
}

/* sub_4a69c0: a client's lobby: 1 when the host's list has come since last asked. */
reg32 frontend::neta::clientLobby(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    if (guest.word(kSetupDirty) == 0)
        return 0;
    enter(guest, cpu, kPlayersLock);
    guest.setWord(kSetupDirty, 0);
    leave(guest, cpu, kPlayersLock);
    return 1;
}

void frontend::netClientLobby(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a69c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::clientLobby(app, cpu); });
}

/* sub_4f5a80, the match of a walk over the received queue: 0 for the packet asked for, not too old. */
reg32 frontend::neta::packetMatch(App* app, CPU& cpu, reg32 packet, reg32 asked)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    if (sreg32(guest.dword(kClock) + 0x10) < sreg32(guest.dword(packet + 4)) || guest.dword(packet + 0x20) != guest.dword(asked))
        return 1;
    guest.leaveEdx(asked + 4);  // as the call by address leaves it
    const reg32 same = guest::direct<0x4f5fe0, sameNode>(app, cpu, asked + 4, packet + 0xc);
    return same != 0 ? 1 : 0;
}

void frontend::netPacketMatch(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5a80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 packet = cpu.eax, asked = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::packetMatch(app, cpu, packet, asked); });
}

/* sub_4a6470, an endpoint's call when its machine is lost: the host notes which, on a client the
 * session is over. */
reg32 frontend::neta::endpointLost(App* app, CPU& cpu, reg32 endpoint)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    guest.setWord(kSetupDirty, 1);
    if (guest.dword(kSessionStage) == 3 && guest.word(kIsHost) != 0)
    {
        const reg32 machine = widened(fe::direct<0x4a6650, ea::machineOf>(app, cpu, endpoint));
        if (sreg32(machine) > -1)
            directKept<0x49ecf0, ea::machineLeftAnnounced, 1>(guest.app(), guest.cpu(), machine);
        return 0;
    }
    guest.setWord(kSessionFailed, 1);
    guest.setWord(kSessionOver, 1);
    return 0;
}

void frontend::netEndpointLost(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6470", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 endpoint = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::endpointLost(app, cpu, endpoint); });
}

/* sub_4f5ba0, IPX's start: Winsock 1.1 opened; the most a datagram may be, 0x2000 unless it says. */
reg32 frontend::neta::ipxStartup(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x198);
    const reg32 data = guest.frame();
    if (thunked<kWsaStartup, &host::netStartup>(guest, cpu, 0x101, data) != 0)
        return 0;
    const reg16 most = guest.word(data + 0x188);
    return most != 0 ? reg32(most) : 0x2000;
}

void frontend::netIpxStartup(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5ba0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxStartup(app, cpu); });
}

/* sub_49e320: one more waiting to join: eax who, edx its link. */
reg32 frontend::neta::pendingAdd(App* app, CPU& cpu, reg32 who, reg32 link)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    constexpr reg32 kStartWanted = 0x55e710, kAcceptCount = 0x7a01e0;
    guest.setByte(guest.frame(), reg8(link));
    if (guest.dword(kStartWanted) != 0)
        return 0;
    const reg32 count = guest.dword(kAcceptCount);
    if (sreg32(count) >= 7)
        return 0;
    guest.setDword(kAcceptCount, count + 1);
    fe::direct<0x49f6a0, frontend::connect::enqueuePeer>(app, cpu, who, guest.frame());
    return 1;
}

void frontend::netPendingAdd(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49e320", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 who = cpu.eax, link = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::pendingAdd(app, cpu, who, link); });
}

/* sub_4c4190, a packet of what the host's race says, 0x14 bytes. */
reg32 frontend::neta::statusPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    copyBytes(guest, 0x7d3650, packet, 0x14);
    guest.leaveEdx(1);
    guest.setWord(0x7a22a2, 1);
    return 0x14;
}

void frontend::netStatusPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c4190", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::statusPacket(app, cpu, packet); });
}

/* sub_4f5230: NET_'s two lists and its exit, once. */
reg32 frontend::neta::servicesReady(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kServicesUp = 0x566df4, kCloseServices = 0x4f50c0;
    const reg32 up = guest.dword(kServicesUp);
    if (up != 0)
        return guest.eax();
    guest::direct<0x50dcd0, netb::qinit>(app, cpu, 0x9f2628, up, 0);
    guest::direct<0x50dcd0, netb::qinit>(app, cpu, 0x9f260c, 0, 0);
    const reg32 result = fe::direct<0x4f2a78, frontend::eab::atexitJump>(app, cpu, kCloseServices);
    guest.setDword(kServicesUp, 1);
    return result;
}

void frontend::netServicesReady(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5230", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::servicesReady(app, cpu); });
}

/* sub_4f5fe0: whether two addresses, at edx and ebx, have the same 8 bytes from their +4. */
reg32 frontend::neta::sameNode(App* app, CPU& cpu, reg32 first, reg32 second)
{
    Guest guest(app, cpu);
    const reg32 one = first + 4, other = second + 4;
    for (reg32 i = 0; i < 8; ++i)
        if (guest.byte(one + i) != guest.byte(other + i))
            return 0;
    return 1;
}

void frontend::netSameNode(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5fe0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 first = cpu.edx, second = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sameNode(app, cpu, first, second); });
}

/* sub_49ec70, each frame: the machines the host has marked as going, 2, dropped and marked gone, 1. */
reg32 frontend::neta::dropGone(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    for (reg32 machine = 1; sreg32(machine) < sreg32(guest.dword(kRacePlayerCount)); ++machine)
    {
        if (guest.word(kMachineGone + machine * 2) != 2)
            continue;
        directKept<0x49ecb0, ea::connectionDropped, 1>(guest.app(), guest.cpu(), guest.dword(kMachineLinks + machine * 4));
        guest.setWord(kMachineGone + machine * 2, 1);
    }
    return guest.eax();
}

void frontend::netDropGone(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49ec70", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::dropGone(app, cpu); });
}

/* sub_49fb10: whether dial-up networking's DLL and all its functions are there. */
reg32 frontend::neta::rasLoaded(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    for (const reg32 place : { 0x7a05c0u, 0x7a05c8u, 0x7a05ccu, 0x7a05d0u, 0x7a05d4u, 0x7a05d8u, 0x7a05dcu })
        if (guest.dword(place) == 0)
            return 0;
    return 1;
}

void frontend::netRasLoaded(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49fb10", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::rasLoaded(app, cpu); });
}

/* sub_4f5900: a sockaddr_ipx to everyone: eax the port, edx where it goes. */
reg32 frontend::neta::portToSockaddr(App* app, CPU& cpu, reg32 port, reg32 to)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kFillBytes = 0x4e0716, kAfIpx = 6;
    guest.setDword(guest.frame(), port);
    guest.setWord(to, kAfIpx);
    fe::direct<kFillBytes, ea::fillBytes>(guest.app(), guest.cpu(), to + 2, 4, 0);
    fe::direct<kFillBytes, ea::fillBytes>(guest.app(), guest.cpu(), to + 6, 6, 0xff);
    return guest.call(kMemmove, args({ guest.frame(), to + 0xc, 2 }));
}

void frontend::netPortToSockaddr(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5900", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 port = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::portToSockaddr(app, cpu, port, to); });
}

/* sub_49edb0, the packet of which machines have gone: 16 bytes after its two. */
reg32 frontend::neta::gonePacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    copyBytes(guest, kMachineGone, packet + 2, 0x10);
    return 0x10;
}

void frontend::netGonePacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49edb0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::gonePacket(app, cpu, packet); });
}

/* sub_4f58c0: NET_'s address made a sockaddr_ipx: eax the address, edx where it goes. */
reg32 frontend::neta::toSockaddr(App* app, CPU& cpu, reg32 from, reg32 to)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kAfIpx = 6;
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to + 2, from, 4);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to + 6, from + 4, 6);
    const reg32 result = guest.call(kMemmove, args({ from + 0xa, to + 0xc, 2 }));
    guest.setWord(to, kAfIpx);
    return result;
}

void frontend::netToSockaddr(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f58c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 from = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::toSockaddr(app, cpu, from, to); });
}

/* sub_4a64c0, an endpoint's time-out: a host waiting on someone joining past their time, or the
 * game stopping. */
reg32 frontend::neta::endpointTimedOut(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (guest.word(kIsHost) != 0 && guest.dword(kSessionStage) == 3
        && sreg32(guest.dword(kTicks)) > sreg32(guest.dword(0x55e714)))
        return 1;
    return guest.byte(0x564819) != 0 ? 1 : 0;
}

void frontend::netEndpointTimedOut(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a64c0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::endpointTimedOut(app, cpu); });
}

/* sub_4c7f90, packet 12: a car's times, 0xc0 bytes into it; the host passes it on. */
reg32 frontend::neta::resultsPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    const reg32 car = guest.dword(kCarsByPlayer + guest.byte(packet + 1) * 4);
    const reg32 copied = guest.call(kMemmove, args({ packet + 4, car + 0x224, 0xc0 }));
    if (guest.word(kIsHost) == 0)
        return copied;
    return sendLeaving(app, cpu, 0, packet, 0xc4);
}

void frontend::netResultsPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7f90", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::resultsPacket(app, cpu, packet); });
}

/* sub_4c8310, this machine's controls for its packet, eax where they go: its car's steering cut to
 * an integer, then its pressed keys.  Eax is left as it came. */
reg32 frontend::neta::packControls(App* app, CPU& cpu, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    const reg32 player = guest.dword(0x6fd4f4);
    guest.leaveEdx(player);
    fe::X87 x87(guest);
    const std::int32_t steering = x87.truncated(x87.load(guest.dword(kCars + player * 4) + 0x7c0));
    guest.setWord(out, reg16(reg32(steering)));
    guest.setWord(out + 2, guest.word(guest.dword(kCars + player * 4) + 8));
    return out;
}

void frontend::netPackControls(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c8310", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 out = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::packControls(app, cpu, out); });
}

/* sub_4f5880: a sockaddr_ipx made NET_'s address: eax the sockaddr, edx where it goes. */
reg32 frontend::neta::fromSockaddr(App* app, CPU& cpu, reg32 from, reg32 to)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, from + 2, 4);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to + 4, from + 6, 6);
    return guest.call(kMemmove, args({ from + 0xc, to + 0xa, 2 }));
}

void frontend::netFromSockaddr(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 from = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::fromSockaddr(app, cpu, from, to); });
}

/* sub_49e2e0: hosting a RaceNet game: the session reset and NPS opened. */
reg32 frontend::neta::hostRaceNet(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kResetNetwork = 0x4a6390;
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.cpu(), 1, 1);
    guest.leaveEbx(0);
    guest.setWord(kIsHost, 1);
    guest.setWord(kThisMachine, 0);
    guest.setWord(kMachineCount, 1);
    return guest::direct<0x49e080, hostStart>(app, cpu) != 0 ? 1 : 0;
}

void frontend::netHostRaceNet(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49e2e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::hostRaceNet(app, cpu); });
}

/* sub_4c81c0, in a network race with one local player on odd ticks: what has come in taken. */
reg32 frontend::neta::takeInputsOnTick(App* app, CPU& cpu, reg32 tick)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    const reg32 players = guest.dword(0x6fd3b0);
    if (sreg32(guest.dword(0x6fd3a0)) > 1 || players == 1 || players == 0 || guest.dword(0x6fd50c) == 0 || (tick & 1) == 0)
        return tick;
    return guest::direct<0x4c8250, takeInputs>(app, cpu, tick);
}

void frontend::netTakeInputsOnTick(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c81c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 tick = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::takeInputsOnTick(app, cpu, tick); });
}

/* sub_4f4c10, at the game's exit: every service NET_ has killed, and its queue freed. */
reg32 frontend::neta::exitNet(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kReady = 0x566de8, kServicesQueue = 0x9f25f0, kQueueFirst = 0x50e470, kQueueFree = 0x50dde0;
    if (guest.dword(kReady) == 0)
        return guest.eax();
    for (;;)
    {
        const reg32 service = guest.call(kQueueFirst, args({ kServicesQueue, 0 }));
        if (service == 0)
            break;
        guest::direct<0x4f4e60, killService>(app, cpu, service);
    }
    const reg32 freed = directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), kServicesQueue);
    guest.setDword(kReady, 0);
    return freed;
}

void frontend::netExit(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f4c10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::exitNet(app, cpu); });
}

/* sub_4a6440, an endpoint's receive: a packet by its first byte, 0 to 18, to its handler in the
 * table at 0x55ecbc, while the session lasts. */
reg32 frontend::neta::dispatchPacket(App* app, CPU& cpu, reg32 from, reg32 packet, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kHandlers = 0x55ecbc, kPacketKinds = 0x13;
    const reg8 kind = guest.byte(packet);
    if (guest.word(kSessionOver) == 0 && kind < kPacketKinds)
    {
        // The handler takes what this one was handed, the kind in ecx.
        guest.call(guest.dword(kHandlers + kind * 4), args({ from, packet, size, kind }));
    }
    return 1;
}

void frontend::netDispatchPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6440", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 from = cpu.eax, packet = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::dispatchPacket(app, cpu, from, packet, size); });
}

/* sub_4c7e50: packet 13's first two bytes sent, eax's low byte the player. */
reg32 frontend::neta::sendPlayerGone(App* app, CPU& cpu, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    const reg32 packet = guest.frame();
    guest.setByte(packet, 0xd);
    guest.setByte(packet + 1, reg8(player));
    return fe::direct<0x4a60e0, ea::packetSent>(app, cpu, 0, packet, 2)->eax;
}

void frontend::netSendPlayerGone(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7e50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 player = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendPlayerGone(app, cpu, player); });
}

/* sub_49fc50: dial-up networking's DLL let go and its functions forgotten. */
reg32 frontend::neta::rasFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    constexpr reg32 kFreeLibrary = 0x5344dc, kRasDll = 0x7a05c0;
    const reg32 library = guest.dword(kRasDll);
    if (library == 0)
        return guest.eax();
    windows<kFreeLibrary, &host::freeLibrary>(guest, cpu, library);
    return guest.call(kFillZero, args({ kRasDll, 0x24 }));
}

void frontend::netRasFree(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49fc50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::rasFree(app, cpu); });
}

/* sub_4a65b0, while a client waits for the host: the system let run, and whether it has timed
 * out; with the game stopping it stays here until the other thread is through. */
reg32 frontend::neta::hostGone(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kGameQuits = 0x564819;
    fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
    const reg32 gone = guest::direct<0x4a64c0, endpointTimedOut>(app, cpu);
    if (gone != 0)
    {
        while (guest.byte(kGameQuits) != 0)
            if (app->contextWanted())
                app->yieldContext(cpu);
    }
    return gone;
}

void frontend::netHostGone(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a65b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::hostGone(app, cpu); });
}

/* sub_4f5850: a service's call to send, by its table: eax the game, ecx the service. */
reg32 frontend::neta::sendQueued(App* app, CPU& cpu, reg32 game, reg32 first, reg32 second, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    if (game != 0)
    {
        guest.setDword(game + 0x48, 0);
        guest.setDword(game + 0x4c, service);
    }
    return guest.call(guest.dword(guest.dword(service + 4) + 0x38), args({ service, game, first, second }));
}

void frontend::netSendQueued(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5850", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 game = cpu.eax, first = cpu.edx, second = cpu.ebx, service = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::sendQueued(app, cpu, game, first, second, service); });
}

/* sub_49a490: the session's three locks made and the players' records emptied. */
reg32 frontend::neta::makeLocks(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    guest.setDword(kSetupLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    guest.setDword(kPlayersLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    guest.setDword(kBarrierLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu)));
    return guest.call(kFillZero, args({ kPlayerRecords, 0x300 }));
}

void frontend::netMakeLocks(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49a490", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::makeLocks(app, cpu); });
}

/* sub_4a5ef0, NPS's callback with a packet, on the stack: its link, its data, its size: to that
 * link's endpoint's receive. */
reg32 frontend::neta::npsReceived(App* app, CPU& cpu, reg32 /*s0*/, reg32 /*s1*/, reg32 link, reg32 /*s3*/, reg32 data, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    guest.leaveEdx(data);  // the data went in edx too, and the call by address leaves it there
    const reg32 endpoint = guest::direct<0x4a5f30, endpointOfLink>(app, cpu, link);
    guest.leaveEcx(endpoint);
    if (endpoint == 0)
        return 0;
    return guest.call(guest.dword(endpoint + 0x20), args({ endpoint, data, size & 0xffff }));
}

void frontend::netNpsReceived(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5ef0", 0, 0, nullptr, 0, fe::kRestoreEbx };
    const reg32 word1 = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 word2 = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 link = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 word4 = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 data = app->getMemory<reg32>(cpu.esp + 20);
    const reg32 size = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::npsReceived(app, cpu, word1, word2, link, word4, data, size); });
}

/* sub_4f5bf0: Winsock closed: 1 when it did. */
reg32 frontend::neta::winsockClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return thunked<kWsaCleanup, &host::netCleanup>(guest, cpu) == 0 ? 1 : 0;
}

void frontend::netWinsockClose(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5bf0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::winsockClose(app, cpu); });
}

/* sub_4f6010: the socket's own address, 20 bytes, to edx. */
reg32 frontend::neta::ipxOwnAddress(App* app, CPU& cpu, reg32 state, reg32 to)
{
    Guest guest(app, cpu);
    copyBytes(guest, to, state + 0x5c, 20);
    return 1;
}

void frontend::netIpxOwnAddress(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f6010", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 state = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxOwnAddress(app, cpu, state, to); });
}

namespace
{

/* NET_'s calls, each to the service's table by its place, what it was called with going on. */
reg32 callThroughTable(Guest& guest, reg32 service, reg32 name, reg32 place)
{
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.cpu(), name);
    return guest.call(guest.dword(guest.dword(service + 4) + place), args({ service }));
}

}  // namespace

reg32 frontend::neta::getGames(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return callThroughTable(guest, service, 0x54d0f0, 0x10);
}

void frontend::netGetGames(App* app, CPU& cpu)  // sub_4f4eb0, NET_getgames
{
    static fe::Site site{ "sub_4f4eb0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::getGames(app, cpu, service); });
}

reg32 frontend::neta::setGame(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    guest.leaveEcx(service);
    return callThroughTable(guest, service, 0x54d100, 0x14);
}

void frontend::netSetGame(App* app, CPU& cpu)  // sub_4f4ed0, NET_setgame
{
    static fe::Site site{ "sub_4f4ed0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::setGame(app, cpu, service); });
}

reg32 frontend::neta::newGame(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return callThroughTable(guest, service, 0x54d11c, 0x1c);
}

void frontend::netNewGame(App* app, CPU& cpu)  // sub_4f4f10, NET_newgame
{
    static fe::Site site{ "sub_4f4f10", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::newGame(app, cpu, service); });
}

reg32 frontend::neta::openGame(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    directKept<0x4f4c50, ea::netCallTraced, 1>(guest.app(), guest.cpu(), 0x54d148, service);
    const reg32 table = guest.dword(service + 4);
    guest.leaveEcx(table);
    return guest.call(guest.dword(table + 0x28), args({ service, service }));
}

void frontend::netOpenGame(App* app, CPU& cpu)  // sub_4f4f70, NET_opengame
{
    static fe::Site site{ "sub_4f4f70", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::openGame(app, cpu, service); });
}

/* sub_4f5050: a service's poll through its table, when it has one; eax the service's record. */
reg32 frontend::neta::servicePollCall(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 table = guest.dword(service + 8);
    if (guest.dword(table + 0x38) == 0)
        return 0;
    return guest.call(guest.dword(table + 0x38), args({ guest.dword(service + 0xc) }));
}

void frontend::netServicePollCall(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5050", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::servicePollCall(app, cpu, service); });
}

/* sub_4f56e0: whether a game's service is up, and what it can take: 0x566e20's word when it is. */
reg32 frontend::neta::serviceRoom(App* app, CPU& cpu, reg32 game)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 service = guest.dword(game + 0x4c);
    const reg32 table = guest.dword(service + 8);
    const reg32 up = guest.call(guest.dword(table + 0x4c), args({ guest.dword(service + 0xc), table }));
    return up != 0 ? reg32(guest.word(0x566e20)) : 0;
}

void frontend::netServiceRoom(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f56e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 game = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceRoom(app, cpu, game); });
}

/* sub_4f5700: a game's service asked through its table's +0x50. */
reg32 frontend::neta::serviceAsk(App* app, CPU& cpu, reg32 game)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 service = guest.dword(game + 0x4c);
    const reg32 table = guest.dword(service + 8);
    return guest.call(guest.dword(table + 0x50), args({ guest.dword(service + 0xc), table }));
}

void frontend::netServiceAsk(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5700", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 game = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::serviceAsk(app, cpu, game); });
}

/* sub_4f5fa0: the game's name kept, 0x30 bytes at most, and given to the socket's key: eax the
 * state, edx the name. */
reg32 frontend::neta::ipxSetName(App* app, CPU& cpu, reg32 state, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), state + 0x26, name, 0x30);
    return guest::direct<0x5127d0, netb::ipxBroadcastOpen>(app, cpu, guest.dword(state + 0x58));
}

void frontend::netIpxSetName(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5fa0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 state = cpu.eax, name = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxSetName(app, cpu, state, name); });
}

/* sub_477fd0, a race's loading step: its music prepared, outside a replay. */
reg32 frontend::neta::raceMusicStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    constexpr reg32 kRacePrepareMusic = 0x410030;
    if (guest.dword(0x6fbb54) != 0)
        return guest.eax();
    guest.call(kRacePrepareMusic, args({ guest.dword(0x6fd4ac) }));
    return fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
}

void frontend::netRaceMusicStep(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_477fd0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::raceMusicStep(app, cpu); });
}

/* sub_49f680: whether nobody waits to join. */
reg32 frontend::neta::pendingEmpty(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(0x7a01ec) != 0 ? 0 : 1;
}

void frontend::netPendingEmpty(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49f680", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::pendingEmpty(app, cpu); });
}

/* sub_4f5f50: the socket's port number, in the machine's order, out of the state's network order. */
reg32 frontend::neta::ipxPort(App* app, CPU& cpu, reg32 state)
{
    Guest guest(app, cpu);
    const reg32 value = guest.dword(state + 0x24);
    const reg32 swapped = (value >> 24) | ((value >> 8) & 0xff00u) | ((value << 8) & 0xff0000u) | (value << 24);
    return swapped >> 16;
}

void frontend::netIpxPort(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5f50", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 state = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxPort(app, cpu, state); });
}

/* sub_4c45d0: the controls' two counts cleared. */
reg32 frontend::neta::clearInputCounts(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x7d368c, 0);
    guest.setDword(0x7d3690, 0);
    return guest.eax();
}

void frontend::netClearInputCounts(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c45d0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::clearInputCounts(app, cpu); });
}

/* sub_4c8350, a packet's controls unpacked, eax the packed, edx the room: the steering as a float,
 * the keys pressed zero extended. */
reg32 frontend::neta::unpackControls(App* app, CPU& cpu, reg32 packed, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    guest.setFloat(out, float(fe::X87::integer(std::int16_t(guest.word(packed)))));
    const reg32 keys = guest.word(packed + 2);
    guest.setDword(out + 4, keys);
    return keys;
}

void frontend::netUnpackControls(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c8350", 0, 0, nullptr, 0 };
    const reg32 packed = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::unpackControls(app, cpu, packed, out); });
}

/* sub_4f5110, the low timer's poll: NET_'s services walked. */
reg32 frontend::neta::pollServices(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kPollOne = 0x4f50f0;
    return guest::direct<0x50e8e0, netb::qiterate>(app, cpu, 0x9f2628, kPollOne, 0);
}

void frontend::netPollServices(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5110", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::pollServices(app, cpu); });
}

/* sub_4f56c0: whether a game is listed and its service still has any. */
reg32 frontend::neta::gameListed(App* app, CPU& cpu, reg32 game)
{
    Guest guest(app, cpu);
    if (guest.dword(game + 0x48) != 0 && guest.dword(guest.dword(game + 0x4c) + 0x9c) != 0)
        return 1;
    return 0;
}

void frontend::netGameListed(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f56c0", 0, 0, nullptr, 0 };
    const reg32 game = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::gameListed(app, cpu, game); });
}

/* sub_4f5f70: a key asked of the socket's library: eax the state, edx the room. */
reg32 frontend::neta::ipxKey(App* app, CPU& cpu, reg32 state, reg32 room)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return guest::direct<0x512c80, netb::ipxStartServer>(app, cpu, state, state + 0x5c, room);
}

void frontend::netIpxKey(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5f70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 state = cpu.eax, room = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxKey(app, cpu, state, room); });
}

namespace
{

/* sub_49b500 and sub_49b520: a chunk of a car's file: past its kind byte, the packet and its size. */
reg32 carFileChunk(Guest& guest, reg32 packet, reg32 size)
{
    constexpr reg32 kFileDone = 0x55e5bc, kReceiveChunk = 0x49b5f0;
    return guest.call(kReceiveChunk, args({ kFileDone, packet + 1, size - 1 }));
}

}  // namespace

reg32 frontend::neta::carChunkToHost(App* app, CPU& cpu, reg32 packet, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return carFileChunk(guest, packet, size);
}

void frontend::netCarChunkToHost(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b500", 0, 0, nullptr, 0, 0 };
    const reg32 packet = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::carChunkToHost(app, cpu, packet, size); });
}

reg32 frontend::neta::carChunkToClient(App* app, CPU& cpu, reg32 packet, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return carFileChunk(guest, packet, size);
}

void frontend::netCarChunkToClient(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_49b520", 0, 0, nullptr, 0, 0 };
    const reg32 packet = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::carChunkToClient(app, cpu, packet, size); });
}

/* sub_4a6410: an endpoint's handlers: lost, receive and time-out. */
reg32 frontend::neta::endpointHandlers(App* app, CPU& cpu, reg32 endpoint)
{
    Guest guest(app, cpu);
    guest.setDword(endpoint + 0x18, 0x4a6470);
    guest.setDword(endpoint + 0x20, 0x4a6440);
    guest.setDword(endpoint + 0x1c, 0);
    guest.setDword(endpoint + 0x14, 0x4a64c0);
    return endpoint;
}

void frontend::netEndpointHandlers(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6410", 0, 0, nullptr, 0 };
    const reg32 endpoint = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::endpointHandlers(app, cpu, endpoint); });
}

/* sub_4a6df0, packet 0: the host's version, 0x28 bytes after its kind. */
reg32 frontend::neta::versionPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return guest.call(kMemmove, args({ packet + 1, 0x55ed08, 0x28 }));
}

void frontend::netVersionPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a6df0", 0, 0, nullptr, 0, 0 };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::versionPacket(app, cpu, packet); });
}

/* sub_4c7e80, the packet that a player has left: its car marked as gone. */
reg32 frontend::neta::playerLeftPacket(App* app, CPU& cpu, reg32 packet)
{
    Guest guest(app, cpu);
    const reg32 car = guest.dword(kCarsByPlayer + guest.byte(packet + 1) * 4);
    guest.leaveEdx(car);
    guest.setDword(car + 0x2c4, 3);
    return guest.eax();
}

void frontend::netPlayerLeftPacket(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4c7e80", 0, 0, nullptr, 0, 0 };
    const reg32 packet = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::playerLeftPacket(app, cpu, packet); });
}

/* sub_4f50c0, at the game's exit: the services' connections undone and NET_'s two lists freed. */
reg32 frontend::neta::closeServices(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    constexpr reg32 kConnRestore = 0x4f5280, kQueueFree = 0x50dde0;
    if (guest.dword(0x566df4) == 0)
        return guest.eax();
    directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.cpu(), guest.cpu().eax);
    directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), 0x9f2628);
    return directKept<0x50dde0, ea::queueLockFreed, 1>(guest.app(), guest.cpu(), 0x9f260c);
}

void frontend::netCloseServices(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f50c0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::closeServices(app, cpu); });
}

/* sub_4f50f0: one of NET_'s services polled when its table has a poll. */
reg32 frontend::neta::pollOne(App* app, CPU& cpu, reg32 service)
{
    Guest guest(app, cpu);
    const reg32 table = guest.dword(service + 4);
    guest.leaveEdx(table);
    if (guest.dword(table + 0xc) != 0)
        guest.call(guest.dword(table + 0xc));
    return 1;
}

void frontend::netPollOne(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f50f0", 0, 0, nullptr, 0, 0 };
    const reg32 service = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::pollOne(app, cpu, service); });
}

/* sub_4a5af0: this machine's link, the low byte of the NPS state's first word; the rest of eax
 * as it came. */
reg32 frontend::neta::thisLink(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return (guest.eax() & 0xffffff00u) | guest.byte(0x7a20a0);
}

void frontend::netThisLink(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4a5af0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::thisLink(app, cpu); });
}

/* sub_4f5fc0: the game's name forgotten. */
reg32 frontend::neta::ipxForgetName(App* app, CPU& cpu, reg32 state)
{
    Guest guest(app, cpu);
    guest.setByte(state + 0x26, 0);
    return 1;
}

void frontend::netIpxForgetName(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5fc0", 0, 0, nullptr, 0 };
    const reg32 state = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxForgetName(app, cpu, state); });
}

/* sub_4f5fd0: nothing to do, a service's table's "yes". */
reg32 frontend::neta::answerYes(App* app, CPU& cpu)
{
    return 1;
}

void frontend::netAnswerYes(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5fd0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::answerYes(app, cpu); });
}

/* sub_4f50b0: nothing to do, a record's default for what it was not given. */
reg32 frontend::neta::answerNo(App* app, CPU& cpu)
{
    return 0;
}

void frontend::netAnswerNo(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f50b0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::answerNo(app, cpu); });
}

/* sub_4f5f90: on to sub_512d70, a service's table's jump. */
reg32 frontend::neta::ipxJump(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest::direct<0x512d70, netb::ipxStopServer>(app, cpu);
}

void frontend::netIpxJump(App* app, CPU& cpu)
{
    static fe::Site site{ "sub_4f5f90", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return neta::ipxJump(app, cpu); });
}

}  // namespace nfs3hp
