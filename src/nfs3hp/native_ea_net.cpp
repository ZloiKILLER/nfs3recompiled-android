#include "game_settings.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_threads.h"
#include "native_frontend_glue.h"
#include "native_frontend_netb.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_raceload.h"
#include "native_watcom.h"
#include "port_snd_stream_calls.h"
#include "os_threads.h"
#include <lib/app.h>

#include <optional>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* EA's network layer under the menus: packets sent, NET and CONN, the queues, LAYER.  The game's code calls each by the name it has here.
 *
 * The game's side (0x49ecb0 to 0x4a6690): the network's state is at
 * 0x7a2274 to 0x7a25b4 -- the machines in the game (their count in
 * 0x7a2274's high word, their connections from 0x7a21f0 on, machine 0 this
 * one), the player each machine is (8 words at 0x7a227a), what each player's
 * machine is doing (8 words at 0x7a22ca), the network's kind (0x7a22e4: 1 a
 * host's network, 2 two machines over a modem or cable, whose packets go
 * through the link's function at 0x7a21b8), this machine the host
 * (0x7a22da), the connection lost (0x7a22dc, 0x7a22de), the game under way
 * (0x7a22e0 3), and 7 connections of 0x6c bytes from 0x7a22e8 -- the host's
 * to a client, or a client's (the first) to its host.  A packet goes to one
 * machine's connection, or to all of them, tried again 9 times (a sleep a
 * little longer each time) before the connection counts as lost; a machine
 * lost while the game is under way is told to the others (packet 0xe).  The
 * game's NET object is at 0x7a0390 (its functions at [+4], a player's check
 * at [+8], its game at [+0xc]); the game's sessions, 0x2c bytes each, at
 * 0x55e734 ([0x55e718] of them, the current one's id at 0x55e71c).
 *
 * EA's side: NET_ (0x4f4f50 to 0x4f5040) traces its call when [0x566de0]
 * says so, and calls the NET object's function; CONN (cmn\connect.c) closes
 * and restores the connections, kept in the queues at 0x9f260c and 0x9f2628
 * (0x9f25f0: the players NET_endgame looks for); a connection's +4 is its
 * functions (+8 send, +0xc whether it counts, +0x10 close), +0x10 it open,
 * +0x24 it up, +0x30 the queue of its layers.  The queues (cmn\queue.c) are
 * 0x1c bytes: +0 how many, +4 flags (1 changed, 2 its lock its own), +8 the
 * first element, +0xc the last, +0x10 and +0x14 a priority's function and
 * its argument, +0x18 the lock (a critical section; 0 an uninitialized
 * queue, reported); an element's first dword is the next.  LAYER
 * (cmn\layer.c) hands a packet -- pieces of {length, address} -- down a
 * connection's layers from the top, each layer's packer (+4, its data +0xc)
 * calling the next; under the last the pieces are gathered into one block
 * ('FILTER') and sent by the connection's own send.
 *
 * Not network code, but in the range: EA's unpackers that the game's
 * unpacker (0x4f2560) picks by a file's first two bytes -- Huffman with
 * runs (0x30fb to 0x35fb, the 0x32fb and 0x34fb kinds delta coded after),
 * runs of bytes and nibbles (0x4afb), DXT1 blocks to 16-bit pixels (0x18fb)
 * and byte pairs (0x46fb, 0x47fb), the last with its tables in its own frame
 * and their addresses at 0xa0b030 to 0xa0b040.
 *
 * The network's functions are typed: a core of its own name, and the
 * thunk under the game's name, for the dispatch and the callers still on
 * registers, leaving each register its callers read as the code leaves it
 * (none it was handed an argument in) -- what a Windows function or a
 * callee left in one the code does not restore too, which the cores hand
 * back.  A callee is handed its own argument registers, and the others only
 * where what it leaves in them is read after.  The frames a callee is handed the address of stay on
 * the guest's stack at the code's depth: the packets announceMachineLeft and
 * closeNetwork send, with the registers the code pushed above them (the
 * send's check reads on past the packet), connSend's piece, sendOverLink's
 * address.  Windows' functions are called through their imports' slots; the
 * connections', NET's, the link's and the layers' functions through the
 * pointers the game keeps.  Other files' typed functions whose answer is all
 * the code takes of them -- a sleep, a block reserved, a block freed for its
 * eax, a copy, a lock freed, a timer's callback removed -- are called as C++
 * (guest::direct, their headers); every other function of the game's by
 * address -- a block freed whose edx and ebx are left, the messages closed
 * whose registers are handed on, and the packet sent (0x4a60e0) and a
 * machine's number (0x4a6650) too, which other natives' checks stand in for.
 *
 * The unpackers are typed too, the byte pairs' tables in their frame at the
 * code's depth (their addresses are kept at 0xa0b034 to 0xa0b040), and
 * Huffman's (0x50ed50) tables in its frame at the code's depth as well:
 * read again with no tables asked for, and a run's count writes over one. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// Where an Error is from: the file, the function, the line.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// The game's network: its sessions -- how many, the current one's id, the
// first one's id and its flag (0x2c bytes each, 11 dwords) -- the game
// ending, its lock; the link's id.
constexpr u32 kSessionCount = 0x55e718, kSessionId = 0x55e71c, kSessionIds = 0x55e734, kSessionFlags = 0x55e738;
constexpr u32 kSessionBytes = 0x2c, kSessionWords = 11;
constexpr u32 kEnding = 0x55e860, kEndingLock = 0x55e864, kLinkId = 0x55ecb0;
// The NET object; the connections left (7 of them), the links open.
constexpr u32 kNet = 0x7a0390, kPendingPlayers = 0x7a02e8, kLinksOpen = 0x7a01e0;
// The NET object's functions, its check (a function at its +0x40), its
// game; of its functions the game shut, ended, a player killed.
constexpr u32 kNetFunctions = 4, kNetCheck = 8, kNetGame = 0xc, kCheckSlot = 0x40;
constexpr u32 kShutGameSlot = 0x24, kEndGameSlot = 0x2c, kKillPlayerSlot = 0x34;
// The link's: its broadcast's buffer, its port, its function; the kinds of
// address it is handed, a machine's or all.
constexpr u32 kLinkBroadcast = 0x7a20a0, kLinkPort = 0x7a215c, kLinkSend = 0x7a21b8;
constexpr u32 kLinkToOne = 0xb0, kLinkToAll = 0xc0;
// The machines' connections, their count (the high word), the players' machines.
constexpr u32 kMachines = 0x7a21f0, kMachineCount = 0x7a2274, kMachinePlayers = 0x7a227a;
constexpr u32 kSetUp = 0x7a2276, kInputNotSent = 0x7a2290, kStateBlock = 0x7a2294, kStateChanged = 0x7a2296;
constexpr u32 kPlayerStates = 0x7a22ba, kMachineStates = 0x7a22ca;
constexpr u32 kStateBlockBytes = 0x26, kStatesBytes = 0x10;
constexpr u32 kNetworkHost = 0x7a22da, kConnectionLost = 0x7a22dc, kConnectionLostToo = 0x7a22de;
constexpr u32 kGameState = 0x7a22e0, kNetworkKind = 0x7a22e4, kConnections = 0x7a22e8;
// The network's kinds: a host's, two machines' over the link; the game
// under way; a machine gone, in its state.
constexpr u32 kHostNetwork = 1, kLinked = 2, kUnderWay = 3;
constexpr u16 kMachineGone = 2;
// A connection: its functions (+8 its send, +0xc whether it counts, +0x10
// its close), it open, it up, its layers' queue, its player (the byte the
// link addresses it by); 0x6c bytes each of the 7.
constexpr u32 kConnectionFunctions = 4, kConnectionOpen = 0x10, kConnectionUp = 0x24, kConnectionLayers = 0x30;
constexpr u32 kConnectionPlayer = 0x34, kConnectionBytes = 0x6c, kConnectionCount = 7;
constexpr u32 kSendSlot = 8, kCountsSlot = 0xc, kCloseSlot = 0x10;
// A packet tried again 9 times before the connection counts as lost.
constexpr u32 kTries = 9;
// The packets: a machine left (0xe, the machines' states after it, 0x12
// bytes), the network closed (4, its word 0xc after a byte, 4 bytes), the
// input (6).
constexpr u8 kMachineLeftPacket = 0xe, kClosedPacket = 4, kInputPacket = 6;
constexpr u32 kMachineLeftBytes = 0x12, kClosedBytes = 4;
constexpr u16 kClosedWord = 0xc;
// EA's: the trace's level; the connections restored; the open connections.
constexpr u32 kTraceLevel = 0x566de0, kRestoring = 0x566df4, kOpenCount = 0x566df8;
// The queues: the players, the connections, those closing.
constexpr u32 kPlayersQueue = 0x9f25f0, kConnectionsQueue = 0x9f260c, kClosingQueue = 0x9f2628;
// A queue: its count (+0), flags, first and last, a priority's function
// and its argument, its lock, the layer last handed on (a queue of layers).
constexpr u32 kQueueFlags = 4, kQueueFirst = 8, kQueueLast = 0xc, kQueuePriority = 0x10;
constexpr u32 kQueuePriorityArgument = 0x14, kQueueLock = 0x18, kQueueLayer = 0x1c;
constexpr u8 kQueueChanged = 1, kOwnLock = 2;
// A layer: its packer, its data.
constexpr u32 kLayerPacker = 4, kLayerData = 0xc;
// The byte pairs' tables and where they write.
constexpr u32 kPairOut = 0xa0b030, kPairLeft = 0xa0b034, kPairIn = 0xa0b038, kPairRight = 0xa0b03c;
constexpr u32 kPairKind = 0xa0b040;
// The game's strings: NET's, CONN's, the queues', LAYER's; the Errors' lines.
constexpr u32 kShutGameName = 0x54d138, kEndGameName = 0x54d158, kKillPlayerName = 0x54d174;
constexpr u32 kConnectC = 0x54d194, kRestoreName = 0x54d1a4, kNotRemoved = 0x54d1b4;
constexpr u32 kQueueC = 0x54fbb8, kUninitialized = 0x54fbcc, kQfreeName = 0x54fc00, kQremoveName = 0x54fc34;
constexpr u32 kQlengthName = 0x54fc3c, kQpositionName = 0x54fc44, kQfindindexName = 0x54fc78;
constexpr u32 kLayerC = 0x550194, kLayerSendName = 0x5501a0, kFilterName = 0x5501ac;
constexpr u32 kCallPackerName = 0x5501cc, kFindFailed = 0x5501e0;
constexpr u32 kRestoreLine = 0xdb, kQfreeLine = 0xf0, kQremoveLine = 0x21c, kQlengthLine = 0x245;
constexpr u32 kQpositionLine = 0x271, kQfindindexLine = 0x30f, kLayerSendLine = 0x2d, kCallPackerLine = 0x63;
// The game's functions, by address.
constexpr u32 kCloseMessages = 0x42d7f0, kFreeNetworkLocks = 0x49a4d0;
constexpr u32 kFillZero = 0x4e070c;
constexpr u32 kFree = 0x4e1890, kNetMessage = 0x512180;
constexpr u32 kConnectionTask = 0x4f5110;

u16 word16(App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putWord16(App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

/* `count` bytes from `from` to `to`, as rep movsd and rep movsb copy them: a
 * dword at a time, then the bytes left. */
void copyMemory(App* app, u32 to, u32 from, u32 count)
{
    for (u32 n = count >> 2; n != 0; --n, to += 4, from += 4)
        put(app, to, at(app, from));
    for (u32 n = count & 3; n != 0; --n, ++to, ++from)
        putByte(app, to, byte(app, from));
}

/* The registers a function pushes on entry, in the order it pushes them. */
struct Saved
{
    u32 ebx, ecx, edx, esi, edi, ebp;
};

/* A call by address with the four registers `in` -- what a callee that
 * leaves them as handed, or one whose registers are handed on, is given as
 * the code has them -- and `words` pushed (the first at the top), taken off
 * after it; every register but eax as it was before after it, esp too
 * (guest::EspKept).  What the callee left in the four; nullopt when
 * the game stopped. */
std::optional<Regs> callWith(App* app, ThreadState& ts, u32 target, Regs in, std::initializer_list<u32> words = {})
{
    nfs::Args registers;
    registers.eax = in.eax;
    registers.edx = in.edx;
    registers.ebx = in.ebx;
    registers.ecx = in.ecx;
    const std::optional<u32> eax = guest::pointerCall(app, ts, target, registers, words);
    if (!eax)
        return std::nullopt;
    return Regs{*eax, registers.edx, registers.ebx, registers.ecx};
}

/* The lock `lock` entered or left (os::locks): `eax`, as the code has it, where what it leaves
 * there is read (the lock's functions leave it as it is; 0 where it is not read).  nullopt when the
 * game stopped. */
std::optional<u32> lockEnter(App* app, ThreadState& ts, u32 lock, std::optional<u32> eax = std::nullopt)
{
    if (!os::locks::enter(app, ts, lock))
        return std::nullopt;
    return eax.value_or(0);
}

std::optional<u32> lockLeave(App* app, ThreadState& ts, u32 lock, std::optional<u32> eax = std::nullopt)
{
    if (!os::locks::leave(app, ts, lock))
        return std::nullopt;
    return eax.value_or(0);
}

/* An Error from `file`'s `function` at `line` (set where Error reads them,
 * in that order), its words on the stack, the format first; `eax` as the
 * code hands it over.  It keeps every register but eax: eax as it leaves
 * it. */
std::optional<u32> reportError(App* app, ThreadState& ts, u32 file, u32 function, u32 line, u32 eax,
                                 std::initializer_list<u32> words)
{
    put(app, kWhereFile, file);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
    guest::Words more({});
    for (const u32* word = words.begin() + 1; word < words.end(); ++word)
        more.word[more.count++] = *word;
    guest::direct<0x401010, ea::error>(app, ts, *words.begin(), more);
    if (ts.terminate)
        return std::nullopt;
    return eax;  // Error keeps eax as the code handed it over
}

/* The queue `queue` uninitialized, as the queues report one in `function`
 * at `line`: its address and the function's name; eax the queue. */
std::optional<u32> reportUninitialized(App* app, ThreadState& ts, u32 queue, u32 function, u32 line)
{
    return reportError(app, ts, kQueueC, function, line, queue, {kUninitialized, function, queue});
}

/* The guest's stack lowered `bytes` while it lives, as the code's pushes and
 * frame lower it: a frame whose address a callee is handed lies where the
 * code has it. */
class Lowered
{
public:
    Lowered(ThreadState& ts, u32 bytes) : ts_(ts), esp_(ts.sp) { ts_.sp -= bytes; }
    ~Lowered() { ts_.sp = esp_; }
    Lowered(const Lowered&) = delete;
    Lowered& operator=(const Lowered&) = delete;

private:
    ThreadState& ts_;
    const u32 esp_;
};

/* What `call` gives, called with the guest's stack `bytes` lower: at the
 * depth the code calls it, under what it pushed and its return address --
 * Error's text is formatted from its arguments' address, which its
 * formatting's check logs. */
template <typename Call>
auto below(ThreadState& ts, u32 bytes, Call call)
{
    const Lowered lowered(ts, bytes);
    return call();
}

/* The registers the code pushes on entry, written below `esp` as its pushes
 * write them: what a callee handed the frame under them reads past its end. */
void pushRegisters(App* app, u32 esp, std::initializer_list<u32> registers)
{
    for (const u32 value : registers)
    {
        esp -= 4;
        put(app, esp, value);
    }
}

/* The thunks' end: eax the answer, the return address taken off. */
void answer(ThreadState& ts, nfs::Args& args, u32 result)
{
    args.eax = result;
    ts.sp += 4;
}

}  // namespace

nfs::u32 dropMachine(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 announceMachineLeft(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 endNetworkGame(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 sessionIndex(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 forgetPendingPlayer(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 sendOverLink(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 sendPacket(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 resetNetwork(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 closeNetwork(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 machineNumber(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 playerMachine(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 netTrace(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 netShutGame(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 netEndGame(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 netKillPlayer(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 netCheckPlayer(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 connRestore(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 connSend(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 connClose(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qnth(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qunlink(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qfree(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qremove(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qlength(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qposition(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 qfindindex(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 huffUnpack(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 runUnpack(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 dxtBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 dxtUnpack(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 pairExpand(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 pairUnpack(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 layerSend(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 layerCallPacker(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 layerPackFromTop(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 layerFree(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 layerSendPacket(App* app, ThreadState& ts, nfs::Args& args);

namespace
{

/* The connection `index` of the 7. */
constexpr u32 connectionAt(u32 index)
{
    return kConnections + index * kConnectionBytes;
}

/* The sleep before try `tries` + 1 of a packet, in milliseconds. */
constexpr u32 retryWait(u32 tries)
{
    return tries * 50 + 10;
}

/* 0x49f1c0: the index of the session whose id is `id`, of [0x55e718], or -1. */
u32 findSession(App* app, u32 id)
{
    for (u32 session = 0; s32(session) < s32(at(app, kSessionCount)); ++session)
        if (at(app, kSessionIds + session * kSessionBytes) == id)
            return session;
    return 0xffffffff;
}

/* Session `session`'s flag; -1's is the dword before the first's id. */
u32 sessionFlag(App* app, u32 session)
{
    return at(app, kSessionFlags + session * kSessionBytes);
}

}  // namespace

/* 0x4a6650: the number of the machine whose connection is `connection` -- 1
 * up to the machines' count (0x7a2274's high word, signed) -- or -1; -1 for
 * none. */
u32 machineOf(App* app, u32 connection)
{
    if (connection == 0)
        return 0xffffffff;
    for (u32 machine = 1;; ++machine)
    {
        if (s32(machine) >= s32(at(app, kMachineCount)) >> 16)
            return 0xffffffff;
        if (at(app, kMachines + machine * 4) == connection)
            return machine;
    }
}

namespace
{

/* 0x4a6690: the machine whose player is `player`, of the 8 (0x7a227a), or -1. */
u32 machineOfPlayer(App* app, u16 player)
{
    for (u32 machine = 0; machine < 8; ++machine)
        if (word16(app, kMachinePlayers + machine * 2) == player)
            return machine;
    return 0xffffffff;
}

/* 0x4f4c50: NET's trace of the call named `name`, when [0x566de0] is 1 or
 * more: the network's message (0x512180, stdcall) with its registers name,
 * 0, 0, 0 and 0, 0 on the stack.  eax as the code leaves it: the message's,
 * or the name untraced. */
std::optional<u32> traceNet(App* app, ThreadState& ts, u32 name)
{
    if (s32(at(app, kTraceLevel)) < 1)
        return name;
    const u32 left = guest::direct<0x512180, frontend::netb::commEvent>(app, ts, name, 0u, 0u, 0u, 0u, 0u);
    if (ts.terminate)
        return std::nullopt;
    return left;
}

/* The NET object `net`'s function at `slot` of its functions, traced as
 * `name` first, asked with `argument` (edx); what it gives back. */
std::optional<u32> callNet(App* app, ThreadState& ts, u32 net, u32 name, u32 slot, u32 argument)
{
    if (!traceNet(app, ts, name))
        return std::nullopt;
    const u32 functions = at(app, net + kNetFunctions);
    return guest::pointerCall(app, ts, at(app, functions + slot), {net, argument});
}

/* 0x4f5040: whether the player `player` is the NET object `net`'s: its
 * check function (the object's) asked with the object's game.  No trace. */
std::optional<u32> checkNetPlayer(App* app, ThreadState& ts, u32 net, u32 player)
{
    const u32 check = at(app, net + kNetCheck);
    return guest::pointerCall(app, ts, at(app, check + kCheckSlot), {at(app, net + kNetGame), player});
}

/* The same with ebx `third`, which the object's check function reads (two addresses compared). */
std::optional<u32> checkNetPlayerWith(App* app, ThreadState& ts, u32 net, u32 player, u32 third)
{
    const u32 check = at(app, net + kNetCheck);
    return guest::pointerCall(app, ts, at(app, check + kCheckSlot), {at(app, net + kNetGame), player, third});
}

/* 0x50dc00: the element `n` + 1 past `element` -- its next, and on -- or 0
 * past the last. */
u32 nth(App* app, u32 element, u32 n)
{
    element = at(app, element);
    if (element != 0)
    {
        for (;;)
        {
            --n;
            if (n == 0xffffffff)
                break;
            element = at(app, element);
            if (element == 0)
                break;
        }
    }
    return element;
}

/* 0x50dc20: the element `element` taken out of the queue `queue`, its lock
 * not taken (qremove's): the count one fewer, the first or the last moved
 * past it; taken out, its next 0 and the queue's flag 1 (changed) set.  1 if
 * it was there, else 0. */
u32 unlink(App* app, u32 queue, u32 element)
{
    u32 found = 0;
    const u32 count = element != 0 ? at(app, queue) : 0;
    if (count != 0)
    {
        const u32 first = at(app, queue + kQueueFirst);
        if (element == first)
        {
            found = 1;
            const u32 last = at(app, queue + kQueueLast);
            put(app, queue, count - 1);
            if (first == last)
            {
                put(app, queue + kQueueFirst, 0);
                put(app, queue + kQueueLast, at(app, queue + kQueueFirst));
            }
            else
            {
                put(app, queue + kQueueFirst, at(app, first));
            }
        }
        else
        {
            u32 before = first;
            if (at(app, first) != 0)
            {
                while (element != at(app, before))
                {
                    before = at(app, before);
                    if (at(app, before) == 0)
                        break;
                }
            }
            const u32 next = at(app, before);
            if (next != 0 && element == next)
            {
                found = 1;
                put(app, queue, at(app, queue) - 1);
                put(app, before, at(app, next));
                if (element == at(app, queue + kQueueLast))
                    put(app, queue + kQueueLast, before);
            }
        }
    }
    if (found != 0)
    {
        put(app, element, 0);
        putByte(app, queue + kQueueFlags, byte(app, queue + kQueueFlags) | kQueueChanged);
    }
    return found;
}

/* 0x50dde0, EA's qfree: the queue's lock let go -- freed (0x4eb360) if it is
 * its own -- and forgotten; an Error for a queue uninitialized.  eax as the
 * code leaves it: the queue, or what Error or the lock's freeing left. */
std::optional<u32> freeQueueLock(App* app, ThreadState& ts, u32 queue)
{
    u32 left = queue;
    if (at(app, queue + kQueueLock) == 0)
    {
        const auto reported = below(ts, 0x10, [&] {
            return reportUninitialized(app, ts, queue, kQfreeName, kQfreeLine);
        });
        if (!reported)
            return std::nullopt;
        left = *reported;
    }
    if ((byte(app, queue + kQueueFlags) & kOwnLock) != 0)
    {
        const auto freed = guest::direct<0x4eb360, ea::lockFreed>(app, ts, at(app, queue + kQueueLock));
        if (!freed)
            return std::nullopt;
        left = *freed;
    }
    put(app, queue + kQueueLock, 0);
    return left;
}

/* 0x50e1d0, EA's qlength: how many elements the queue holds; an Error for a
 * queue uninitialized. */
std::optional<u32> queueLength(App* app, ThreadState& ts, u32 queue)
{
    if (at(app, queue + kQueueLock) == 0 && !below(ts, 0x10, [&] {
            return reportUninitialized(app, ts, queue, kQlengthName, kQlengthLine);
        }))
        return std::nullopt;
    return at(app, queue);
}

/* 0x50e150, EA's qremove: the element taken out of the queue under its lock
 * (unlink); 1 if it was there.  Asked for none, what eax gives back is
 * `forNone`, edi as the caller had it. */
std::optional<u32> removeFromQueue(App* app, ThreadState& ts, u32 queue, u32 element, u32 forNone)
{
    if (!lockEnter(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    if (at(app, queue + kQueueLock) == 0 && !below(ts, 0x14, [&] {
            return reportError(app, ts, kQueueC, kQremoveName, kQremoveLine, kQremoveName,
                               {kUninitialized, kQremoveName, queue});
        }))
        return std::nullopt;
    const u32 result = element != 0 ? unlink(app, queue, element) : forNone;
    if (!lockLeave(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    return result;
}

/* 0x50e220, EA's qposition: under the queue's lock, the index of the
 * element, or -1.  A queue with a priority's function is looked through only
 * as far as the elements whose priority is not above the element's. */
std::optional<u32> queuePosition(App* app, ThreadState& ts, u32 queue, u32 element)
{
    if (!lockEnter(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    if (at(app, queue + kQueueLock) == 0 && !below(ts, 0x18, [&] {
            return reportError(app, ts, kQueueC, kQpositionName, kQpositionLine, kQpositionName,
                               {kUninitialized, kQpositionName, queue});
        }))
        return std::nullopt;
    u32 index = 0;
    bool found = false;
    if (element != 0)
    {
        const u32 priority = at(app, queue + kQueuePriority);
        u32 current = at(app, queue + kQueueFirst);
        // The element's own priority, none without the function.
        u32 elementsPriority = 0;
        if (priority != 0)
        {
            const auto own = guest::pointerCall(app, ts, priority, {element, at(app, queue + kQueuePriorityArgument)});
            if (!own)
                return std::nullopt;
            elementsPriority = *own;
        }
        while (current != 0 && current != element)
        {
            if (priority != 0)
            {
                const auto its = guest::pointerCall(app, ts, priority, {current, at(app, queue + kQueuePriorityArgument)});
                if (!its)
                    return std::nullopt;
                if (*its > elementsPriority)
                    break;
            }
            current = at(app, current);
            ++index;
        }
        found = current == element;
    }
    if (!found)
        index = 0xffffffff;
    if (!lockLeave(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    return index;
}

/* 0x50e470, EA's qfindindex: under the queue's lock, its element `index`
 * (nth past the first), or 0. */
std::optional<u32> findInQueue(App* app, ThreadState& ts, u32 queue, u32 index)
{
    if (!lockEnter(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    if (at(app, queue + kQueueLock) == 0 && !below(ts, 0x14, [&] {
            return reportError(app, ts, kQueueC, kQfindindexName, kQfindindexLine, kQfindindexLine,
                               {kUninitialized, kQfindindexName, queue});
        }))
        return std::nullopt;
    u32 found = at(app, queue + kQueueFirst);
    if (found != 0 && index != 0)
        found = nth(app, at(app, queue + kQueueFirst), index - 1);
    if (!lockLeave(app, ts, at(app, queue + kQueueLock)))
        return std::nullopt;
    return found;
}

/* 0x5123b0, EA's LAYER_send: the `count` pieces at `pieces` ({length,
 * address} each) gathered into one block ('FILTER', the game's allocation)
 * and sent by the connection's own send (the block, its length); the block
 * freed, and what the send gave back.  edx and ebx as the free left them. */
std::optional<Regs> gatherAndSend(App* app, ThreadState& ts, u32 connection, u32 count, u32 pieces)
{
    u32 total = 0;
    if (s32(count) > 0)
    {
        u32 piece = pieces;
        for (u32 i = 0; s32(i) < s32(count); ++i, piece += 8)
            total += at(app, piece);
    }
    put(app, kWhereFile, kLayerC);
    put(app, kWhereFunction, kLayerSendName);
    put(app, kWhereLine, kLayerSendLine);
    const auto block = guest::direct<0x4e1620, ea::reserved>(app, ts, kFilterName, total, 0u);
    if (!block)
        return std::nullopt;
    if (s32(count) > 0)
    {
        u32 to = *block, piece = pieces;
        for (u32 i = 0; s32(i) < s32(count); ++i, piece += 8)
        {
            if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, to, at(app, piece + 4), at(app, piece)))
                return std::nullopt;
            to += at(app, piece);
        }
    }
    const u32 functions = at(app, connection + kConnectionFunctions);
    const auto sent = callWith(app, ts, at(app, functions + kSendSlot), {connection, *block, total, functions});
    if (!sent)
        return std::nullopt;
    // The free keeps every register but eax: edx and ebx stay what the send left.
    const auto freed = guest::direct<kFree, ea::purgeBlock>(app, ts, *block);
    if (!freed)
        return std::nullopt;
    return Regs{sent->eax, sent->edx, sent->ebx, pieces};
}

/* 0x512530, EA's LAYER_callpacker: the packet -- `count` pieces at `pieces`
 * -- on the connection handed to layer `layer` of the queue `layers` (its
 * index noted at the queue's +0x1c): its packer (stdcall, with its data, the
 * queue on the stack) -- an Error, and the packer read from an element 0,
 * when qfindindex finds none.  Past the layers, sent (gatherAndSend).  The
 * four registers as the packer or the send leaves them. */
std::optional<Regs> callPacker(App* app, ThreadState& ts, u32 connection, u32 layer, u32 count, u32 pieces,
                               u32 layers)
{
    bool down = false;
    if (s32(layer) >= 0)
    {
        const auto length = below(ts, 0x10, [&] { return queueLength(app, ts, layers); });
        if (!length)
            return std::nullopt;
        down = layer < *length;
    }
    if (!down)
        return gatherAndSend(app, ts, connection, count, pieces);
    put(app, layers + kQueueLayer, layer);
    const auto found = below(ts, 0x10, [&] { return findInQueue(app, ts, layers, layer); });
    if (!found)
        return std::nullopt;
    const u32 element = *found;
    if (element == 0 && !below(ts, 0xc, [&] {
            return reportError(app, ts, kLayerC, kCallPackerName, kCallPackerLine, kCallPackerName, {kFindFailed});
        }))
        return std::nullopt;
    return callWith(app, ts, at(app, element + kLayerPacker), {connection, at(app, element + kLayerData), count, pieces},
                    {layers});
}

/* 0x512630: the packet -- `count` pieces at `pieces` -- on the connection
 * handed to the top layer of the queue `layers` (callPacker, its last). */
std::optional<Regs> packFromTop(App* app, ThreadState& ts, u32 connection, u32 count, u32 pieces, u32 layers)
{
    const auto length = below(ts, 0xc, [&] { return queueLength(app, ts, layers); });
    if (!length)
        return std::nullopt;
    return below(ts, 0xc, [&] { return callPacker(app, ts, connection, *length - 1, count, pieces, layers); });
}

/* 0x512770: the packet -- `count` pieces at `pieces` -- sent on the
 * connection if it is one and up, down its layers (packFromTop); else 0. */
std::optional<Regs> sendDownLayers(App* app, ThreadState& ts, u32 connection, u32 count, u32 pieces)
{
    if (connection == 0 || at(app, connection + kConnectionUp) == 0)
        return Regs{0, count, pieces, 0};
    const u32 layers = at(app, connection + kConnectionLayers);
    return below(ts, 8, [&] { return packFromTop(app, ts, connection, count, pieces, layers); });
}

/* 0x4f5310: a packet of `length` bytes at `packet` sent on the connection
 * through its layers (sendDownLayers, one piece).  ts.sp where the call
 * leaves it: the piece in the 8 bytes below it, handed on. */
std::optional<Regs> sendOnConnection(App* app, ThreadState& ts, u32 connection, u32 packet, u32 length)
{
    const Lowered frame(ts, 8);
    const u32 piece = ts.sp;
    put(app, piece + 4, packet);
    put(app, piece, length);
    return below(ts, 4, [&] { return sendDownLayers(app, ts, connection, 1, piece); });
}

/* 0x5126c0: the connection's queue of layers, if any, let go (qfree), freed
 * and forgotten.  eax as the code leaves it: the free's, or the connection. */
std::optional<u32> freeLayers(App* app, ThreadState& ts, u32 connection)
{
    const u32 layers = at(app, connection + kConnectionLayers);
    if (layers == 0)
        return connection;
    if (!below(ts, 0xc, [&] { return freeQueueLock(app, ts, layers); }))
        return std::nullopt;
    const auto freed = guest::direct<0x4e1890, ea::purgeBlock>(app, ts, at(app, connection + kConnectionLayers));
    if (!freed)
        return std::nullopt;
    put(app, connection + kConnectionLayers, 0);
    return *freed;
}

/* 0x4f5330: the connection closed, if it is one and open: its close;
 * closed, one that counts makes one open connection fewer, the last taking
 * the connections' task (0x4f5110) off (0x4f29b0); its layers freed.  What
 * the close gave back, or 0. */
std::optional<u32> closeConnection(App* app, ThreadState& ts, u32 connection)
{
    if (connection == 0 || at(app, connection + kConnectionOpen) == 0)
        return 0;
    const u32 functions = at(app, connection + kConnectionFunctions);
    const auto closed = guest::pointerCall(app, ts, at(app, functions + kCloseSlot), {connection});
    if (!closed)
        return std::nullopt;
    if (*closed != 0 && at(app, at(app, connection + kConnectionFunctions) + kCountsSlot) != 0)
    {
        const u32 open = at(app, kOpenCount);
        if (open != 0)
        {
            put(app, kOpenCount, open - 1);
            if (open - 1 == 0 && !guest::direct<0x4f29b0, ea::removeTimer>(app, ts, kConnectionTask))
                return std::nullopt;
        }
    }
    if (!below(ts, 0x14, [&] { return freeLayers(app, ts, connection); }))
        return std::nullopt;
    return closed;
}

/* 0x4f5280, EA's CONN_restore: when connections are restored, each closing
 * connection closed as long as there is a first, then each connection:
 * closed and, if its close left it in the list, removed -- an Error when it
 * was there to remove ("NOT REMOVED FROM LIST BY CLOSE").  eax as the code
 * leaves it: 0, or `handed` (eax as handed over) when nothing is restored. */
std::optional<u32> restoreConnections(App* app, ThreadState& ts, u32 handed)
{
    if (at(app, kRestoring) == 0)
        return handed;
    for (;;)
    {
        const auto closing = below(ts, 0x1c, [&] { return findInQueue(app, ts, kClosingQueue, 0); });
        if (!closing)
            return std::nullopt;
        if (*closing == 0)
            break;
        if (!below(ts, 0x1c, [&] { return closeConnection(app, ts, *closing); }))
            return std::nullopt;
    }
    for (;;)
    {
        const auto found = below(ts, 0x1c, [&] { return findInQueue(app, ts, kConnectionsQueue, 0); });
        if (!found)
            return std::nullopt;
        const u32 connection = *found;
        if (connection == 0)
            return 0;
        if (!below(ts, 0x1c, [&] { return closeConnection(app, ts, connection); }))
            return std::nullopt;
        const auto removed =
            below(ts, 0x1c, [&] { return removeFromQueue(app, ts, kConnectionsQueue, connection, kConnectC); });
        if (!removed)
            return std::nullopt;
        if (*removed != 0 && !below(ts, 0x18, [&] {
                return reportError(app, ts, kConnectC, kRestoreName, kRestoreLine, kRestoreLine,
                                   {kNotRemoved, connection});
            }))
            return std::nullopt;
    }
}

/* 0x4f4f90, EA's NET_endgame: the NET object's game ended (its function
 * +0x2c) -- 1 without it when the object is not among the players' queue's. */
std::optional<u32> endNetGame(App* app, ThreadState& ts, u32 net)
{
    if (!traceNet(app, ts, kEndGameName))
        return std::nullopt;
    const auto position = below(ts, 0xc, [&] { return queuePosition(app, ts, kPlayersQueue, net); });
    if (!position)
        return std::nullopt;
    if (*position == 0xffffffff)
        return 1;
    const u32 functions = at(app, net + kNetFunctions);
    return guest::pointerCall(app, ts, at(app, functions + kEndGameSlot), {net, functions});
}

/* 0x49ecb0: the connection dropped, if it is one and open: closed, its
 * player (its +0x34) killed in the NET game, the machines' states changed
 * and the links open one fewer.  eax as the code leaves it: what killing
 * gave back, or the connection. */
std::optional<u32> dropConnection(App* app, ThreadState& ts, u32 connection)
{
    if (connection == 0 || at(app, connection + kConnectionOpen) == 0)
        return connection;
    if (!below(ts, 0x10, [&] { return closeConnection(app, ts, connection); }))
        return std::nullopt;
    const auto killed = callNet(app, ts, kNet, kKillPlayerName, kKillPlayerSlot, connection + kConnectionPlayer);
    if (!killed)
        return std::nullopt;
    const u32 links = at(app, kLinksOpen);
    putWord16(app, kStateChanged, 1);
    put(app, kLinksOpen, links - 1);
    return killed;
}

/* 0x49f550: the player `player` forgotten among the 7 pending (0x7a02e8)
 * unless the current session's flag is set: each pending one cleared when
 * the NET object's check says the player is its -- the player asked about,
 * not the one pending, the game's code as it is.  eax as the code leaves
 * it: 0 once a check was made, else the session's dword index. */
std::optional<u32> forgetPending(App* app, ThreadState& ts, u32 player)
{
    const u32 session = findSession(app, at(app, kSessionId));
    u32 left = session * kSessionWords;
    if (sessionFlag(app, session) != 0)
        return left;
    for (u32 i = 0; i < kConnectionCount; ++i)
    {
        if (at(app, kPendingPlayers + i * 4) == 0)
            continue;
        const auto its = checkNetPlayer(app, ts, kNet, player);
        if (!its)
            return std::nullopt;
        left = 0;
        if (*its != 0)
            put(app, kPendingPlayers + i * 4, 0);
    }
    return left;
}

/* 0x49ecf0: machine `machine` gone: its pending player forgotten (its
 * connection's +0x34); then, unless the game is ending and for machines 0
 * to 7, under the ending's lock, a machine whose state was 0 marked gone and
 * the others told: packet 0xe, the 8 machines' states after it, 0x12 bytes,
 * sent to all.  ts.sp where the call leaves it, and the registers the code
 * pushes there `saved`: the packet lies under them, and the send's check
 * reads 9 words of it.  Its second byte and the two after it are what the
 * frame held. */
std::optional<u32> announceLeft(App* app, ThreadState& ts, u32 machine, const Saved& saved)
{
    pushRegisters(app, ts.sp, {saved.ebx, saved.ecx, saved.edx, saved.esi, saved.edi, saved.ebp});
    const Lowered frame(ts, 6 * 4 + 0x14);
    const u32 packet = ts.sp;
    const auto forgotten = forgetPending(app, ts, at(app, kMachines + machine * 4) + kConnectionPlayer);
    if (!forgotten)
        return std::nullopt;
    if (at(app, kEnding) != 0 || s32(machine) > 7 || s32(machine) < 0)
        return forgotten;
    const u32 lock = at(app, kEndingLock);
    if (!lockEnter(app, ts, lock))
        return std::nullopt;
    const u32 state = kMachineStates + machine * 2;
    if (word16(app, state) != 0)
        // Its eax, or the state's offset the code has there.
        return lockLeave(app, ts, at(app, kEndingLock), machine * 2);
    putWord16(app, state, kMachineGone);
    if (!lockLeave(app, ts, at(app, kEndingLock)))
        return std::nullopt;
    putByte(app, packet, kMachineLeftPacket);
    copyMemory(app, packet + 2, kMachineStates, kStatesBytes);
    putWord16(app, kStateChanged, 1);
    const auto sent = guest::direct<0x4a60e0, ea::packetSent>(app, ts, 0u, packet, kMachineLeftBytes);
    if (!sent)
        return std::nullopt;
    return sent->eax;
}

/* 0x49eed0: the network game ended, if under way on a host's network: the
 * host's open connections dropped; with the game ending and the session's
 * flag, the NET game shut (1); with the flag, the NET game ended; the game
 * no longer ending.  eax as the code leaves it, `handed` (eax as handed
 * over) when the game is not under way or the network is the link. */
std::optional<u32> endNetwork(App* app, ThreadState& ts, u32 handed)
{
    u32 left = handed;
    if (at(app, kGameState) != kUnderWay)
        return left;
    if (at(app, kNetworkKind) != kLinked)
    {
        if (word16(app, kNetworkHost) != 0)
        {
            for (u32 i = 0; i < kConnectionCount; ++i)
                if (at(app, connectionAt(i) + kConnectionOpen) != 0
                    && !below(ts, 0x10, [&] { return dropConnection(app, ts, connectionAt(i)); }))
                    return std::nullopt;
            if (at(app, kEnding) != 0 && sessionFlag(app, findSession(app, at(app, kSessionId))) != 0
                && !callNet(app, ts, kNet, kShutGameName, kShutGameSlot, 1))
                return std::nullopt;
        }
        const u32 session = findSession(app, at(app, kSessionId));
        left = session * kSessionWords;
        if (sessionFlag(app, session) != 0)
        {
            const auto ended = below(ts, 0x10, [&] { return endNetGame(app, ts, kNet); });
            if (!ended)
                return std::nullopt;
            left = *ended;
        }
    }
    put(app, kEnding, 0);
    return left;
}

/* 0x4a5d10: a packet of `length` bytes at `packet` sent over the two
 * machines' link: to machine `machine` -- its +0x34 byte the address, 0xb0
 * -- or with 0 to all, 0xc0 and the broadcast's buffer; the link's function
 * called (cdecl, 7 words).  ts.sp where the call leaves it: the machine's
 * address in the dword under the 3 registers the code pushes, handed on.
 * eax and edx as the link left them, ebx the length or the link's port. */
std::optional<Regs> sendLinked(App* app, ThreadState& ts, u32 machine, u32 packet, u32 length)
{
    const Lowered frame(ts, 3 * 4 + 4);
    const u32 address = ts.sp;
    std::optional<Regs> sent;
    if (machine == 0)
    {
        const u32 port = at(app, kLinkPort), id = at(app, kLinkId);
        sent = callWith(app, ts, at(app, kLinkSend), {0, port, length, id},
                        {id, port, kLinkToAll, kLinkBroadcast, 1, packet, length});
    }
    else
    {
        put(app, address, byte(app, machine + kConnectionPlayer));
        const u32 port = at(app, kLinkPort), id = at(app, kLinkId);
        sent = callWith(app, ts, at(app, kLinkSend), {address, packet, port, machine},
                        {id, port, kLinkToOne, address, 1, packet, length});
    }
    if (!sent)
        return std::nullopt;
    return Regs{sent->eax, sent->edx, sent->ebx, 0};
}

/* The connection counts as lost: edx 1 as the code leaves it. */
Regs connectionLost(App* app, Regs left)
{
    putWord16(app, kConnectionLostToo, 1);
    putWord16(app, kConnectionLost, 1);
    left.edx = 1;
    return left;
}

}  // namespace

/* 0x4a60e0: a packet of `length` bytes at `packet` sent to the connection
 * `connection`, or with 0 to all; nothing once the connection is lost.  Over
 * the link (the network's kind 2) by sendLinked.  A client's (or anyone's
 * to all on a network of kind 0) goes to its host's connection: sent,
 * [0x7a2290] is 0; not sent, the input's packet (6) on a host's network
 * leaves [0x7a2290] 1, anything else is tried 9 times more, a sleep before
 * each, and then the connection counts as lost.  A host's to one machine is
 * tried as long as its connection is up, to all to each of the 7 up; a
 * machine still not reached is announced gone while the game is under way
 * (by its number, 0x4a6650), and otherwise the connection counts as lost.
 * ts.sp where the call leaves it: connSend, sendOverLink and
 * announceMachineLeft are called under the 4 registers it pushes and its
 * 0xc bytes of frame, where their frames' addresses are as the code has
 * them.  eax, edx and ebx as the code leaves them. */
std::optional<Regs> sendPacketTo(App* app, ThreadState& ts, u32 connection, u32 packet, u32 length)
{
    const u32 frameBase = ts.sp - 4 * 4;  // ebp, as the code has it and announceMachineLeft pushes it
    const Lowered frame(ts, 4 * 4 + 0xc + 4);
    Regs left{connection, packet, length, 0};
    if (word16(app, kConnectionLost) != 0)
        return left;
    const u32 kind = at(app, kNetworkKind);
    if (kind == kLinked)
        return sendLinked(app, ts, connection, packet, length);
    // The sleep before try `tries` + 1.
    const auto sleep = [&](u32 tries) {
        return guest::direct<0x4df8e0, ea::sleepFor>(app, ts, retryWait(tries)).has_value();
    };
    // The machine of the connection handed (eax) announced gone; `counter`
    // the code's edi, which announceMachineLeft pushes.
    const auto announce = [&](Regs handed, u32 counter) -> std::optional<Regs> {
        // machineNumber keeps every register but eax: the rest as handed.
        const u32 number = guest::direct<0x4a6650, ea::machineOf>(app, ts, handed.eax);
        const u32 machine = u32(s32(nfs::s16(u16(number))));
        const Saved pushed{handed.ebx, packet, handed.edx, length, counter, frameBase};
        const auto told = announceLeft(app, ts, machine, pushed);
        if (!told)
            return std::nullopt;
        return Regs{*told, handed.edx, handed.ebx, packet};
    };
    if (word16(app, kNetworkHost) == 0 || kind == 0)
    {
        left.edx = kind;
        if (at(app, kConnections + kConnectionUp) == 0)
            return left;
        auto sent = sendOnConnection(app, ts, kConnections, packet, length);
        if (!sent)
            return std::nullopt;
        if (sent->eax != 0)
        {
            put(app, kInputNotSent, 0);
            return sent;
        }
        left = *sent;
        left.eax = u32(byte(app, packet)) << 8;
        if (byte(app, packet) == kInputPacket)
        {
            left.eax = at(app, kNetworkKind);
            if (left.eax == kHostNetwork)
            {
                put(app, kInputNotSent, left.eax);
                return left;
            }
        }
        for (u32 tries = 0; at(app, kConnections + kConnectionUp) != 0 && word16(app, kConnectionLost) == 0;)
        {
            if (!sleep(tries))
                return std::nullopt;
            sent = sendOnConnection(app, ts, kConnections, packet, length);
            if (!sent)
                return std::nullopt;
            left = *sent;
            if (sent->eax != 0)
            {
                put(app, kInputNotSent, 0);
                return left;
            }
            if (s32(++tries) >= s32(kTries))
                break;
        }
        return connectionLost(app, left);
    }
    if (connection != 0)
    {
        left.edx = kind;
        if (at(app, connection + kConnectionUp) == 0)
            return left;
        auto sent = sendOnConnection(app, ts, connection, packet, length);
        if (!sent)
            return std::nullopt;
        left = *sent;
        if (sent->eax != 0)
            return left;
        for (u32 tries = 0; at(app, connection + kConnectionUp) != 0 && word16(app, kConnectionLost) == 0;)
        {
            if (!sleep(tries))
                return std::nullopt;
            sent = sendOnConnection(app, ts, connection, packet, length);
            if (!sent)
                return std::nullopt;
            left = *sent;
            if (sent->eax != 0)
                return left;
            if (s32(++tries) >= s32(kTries))
                break;
        }
        if (at(app, kGameState) != kUnderWay)
            return connectionLost(app, left);
        return announce({connection, left.edx, left.ebx, packet}, connection);
    }
    // To all: each of the 7 up.  The code's flag at its frame's -0xc, which
    // says not to announce or lose, is set 0 and never changed.
    for (u32 index = 0;;)
    {
        const u32 to = connectionAt(index);
        left.eax = index * kConnectionBytes;
        left.edx = index * 12;
        if (at(app, to + kConnectionUp) != 0)
        {
            auto sent = sendOnConnection(app, ts, to, packet, length);
            if (!sent)
                return std::nullopt;
            left = *sent;
            if (sent->eax == 0)
            {
                bool reached = false;
                for (u32 tries = 0;;)
                {
                    left = {index * 12, index, index * kConnectionBytes, packet};
                    if (at(app, to + kConnectionUp) == 0 || word16(app, kConnectionLost) != 0)
                        break;
                    if (!sleep(tries))
                        return std::nullopt;
                    sent = sendOnConnection(app, ts, to, packet, length);
                    if (!sent)
                        return std::nullopt;
                    left = *sent;
                    reached = sent->eax != 0;
                    if (reached || s32(++tries) >= s32(kTries))
                        break;
                }
                if (!reached)
                {
                    if (at(app, kGameState) == kUnderWay)
                    {
                        const auto told = announce({to, index * 12, left.ebx, packet}, index);
                        if (!told)
                            return std::nullopt;
                        left = *told;
                    }
                    else
                    {
                        left = connectionLost(app, left);
                    }
                }
            }
        }
        if (nfs::s16(u16(++index)) >= 7)
            return left;
    }
}

namespace
{

/* 0x4a6390: the network's state made anew: set up, the state's block, the
 * players' and the machines' states cleared (0x4e070c), this machine the
 * host, the connection not lost, no game under way; with `alsoLostToo`, the
 * second lost flag cleared too.  eax as the last clearing left it. */
std::optional<u32> resetNetworkState(App* app, ThreadState& ts, u32 alsoLostToo)
{
    putWord16(app, kSetUp, 1);
    guest::direct<0x4e070c, ea::clearBytes>(app, ts, kStateBlock, kStateBlockBytes);
    guest::direct<0x4e070c, ea::clearBytes>(app, ts, kPlayerStates, kStatesBytes);
    // By address: what it leaves in eax is this one's, which ea::clearBytes does not give back.
    const u32 cleared = guest::direct<0x4e070c, ea::clearBytes>(app, ts, kMachineStates, kStatesBytes);
    if (ts.terminate)
        return std::nullopt;
    putWord16(app, kNetworkHost, 1);
    putWord16(app, kConnectionLost, 0);
    put(app, kGameState, 0);
    if (alsoLostToo != 0)
        putWord16(app, kConnectionLostToo, 0);
    return cleared;
}

/* 0x4a65e0: the network closed: the game's timers stopped; a host tells the
 * others (packet 4, 4 bytes); the connections restored; the timers stopped
 * again; the network game ended, the race's messages closed, the network's
 * locks freed (0x49a4d0); the network's state made anew, and only this
 * machine's player queued no longer (0x6fd50c, the 0 resetNetwork keeps in
 * edx).  The packet: 4, the byte the frame held there (never written, a
 * game bug kept), the word 0xc.  ts.sp where the call leaves it: the
 * packet under the 3 registers the code pushes (written: the send's check
 * reads 9 words of it).  eax, and ecx as the callees left it. */
std::optional<Regs> closeNetworkAt(App* app, ThreadState& ts, u32 ecx)
{
    // The three registers the code pushes are not written (only the frame's depth is kept).
    const Lowered frame(ts, 3 * 4 + 4);
    const u32 packet = ts.sp;
    // The game's timers run (0x4e7630) with 0: their eax.  ebp at the code's frame (above the
    // packet), which the code's call leaves the timers to hand their tasks.
    const auto timersRun = [&]() -> std::optional<u32> {
        const EspKept kept(ts);
        const port::HeldEbp held(ts, packet + 4);
        const port::Result ran = guest::direct<0x4e7630, port::stream::runDueTasks>(app, ts, packet - 4, 0u);
        if (!ran)
            return std::nullopt;
        return u32(*ran);
    };
    const auto stopped = timersRun();
    if (!stopped)
        return std::nullopt;
    u32 left = *stopped;
    if (word16(app, kNetworkHost) != 0)
    {
        putByte(app, packet, kClosedPacket);
        putWord16(app, packet + 2, kClosedWord);
        const auto sent = guest::direct<0x4a60e0, ea::packetSent>(app, ts, 0u, packet, kClosedBytes);
        if (!sent)
            return std::nullopt;
        left = sent->eax;
    }
    const auto restored = below(ts, 4, [&] { return restoreConnections(app, ts, left); });
    if (!restored)
        return std::nullopt;
    const auto again = timersRun();
    if (!again)
        return std::nullopt;
    const auto ended = below(ts, 4, [&] { return endNetwork(app, ts, *again); });
    if (!ended)
        return std::nullopt;
    // Both keep every register but eax: ecx stays what the caller had.
    const auto closed = guest::direct<kCloseMessages, raceload::messagesClosedAs>(app, ts, *ended);
    if (!closed)
        return std::nullopt;
    guest::direct<kFreeNetworkLocks, frontend::glue::freeNetworkLocks>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    const auto reset = resetNetworkState(app, ts, 0);
    if (!reset)
        return std::nullopt;
    gameSettings(app).networkGame = 0;
    return Regs{*reset, 0, 0, ecx};
}

}  // namespace

/* 0x49ecb0: dropConnection of eax. */
nfs::u32 dropMachine(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto dropped = dropConnection(app, ts, args.eax))
        answer(ts, args, *dropped);
    return args.eax;
}

/* 0x49ecf0: announceLeft of machine eax. */
nfs::u32 announceMachineLeft(App* app, ThreadState& ts, nfs::Args& args)
{
    const Saved saved{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    if (const auto told = announceLeft(app, ts, args.eax, saved))
        answer(ts, args, *told);
    return args.eax;
}

/* 0x49eed0: endNetwork. */
nfs::u32 endNetworkGame(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto ended = endNetwork(app, ts, args.eax))
        answer(ts, args, *ended);
    return args.eax;
}

/* 0x49f1c0: findSession of id eax. */
nfs::u32 sessionIndex(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, findSession(app, args.eax));
    return args.eax;
}

/* 0x49f550: forgetPending of the player eax. */
nfs::u32 forgetPendingPlayer(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto forgotten = forgetPending(app, ts, args.eax))
        answer(ts, args, *forgotten);
    return args.eax;
}

/* 0x4a5d10: sendLinked to machine eax of edx, ebx long; edx and ebx as it
 * leaves them. */
nfs::u32 sendOverLink(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = sendLinked(app, ts, args.eax, args.edx, args.ebx))
    {
        args.edx = sent->edx;
        args.ebx = sent->ebx;
        answer(ts, args, sent->eax);
    }
    return args.eax;
}

/* 0x4a60e0: sendPacketTo the connection eax of edx, ebx long. */
nfs::u32 sendPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = sendPacketTo(app, ts, args.eax, args.edx, args.ebx))
        answer(ts, args, sent->eax);
    return args.eax;
}

/* 0x4a6390: resetNetworkState, the second lost flag too for eax nonzero. */
nfs::u32 resetNetwork(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto reset = resetNetworkState(app, ts, args.eax))
        answer(ts, args, *reset);
    return args.eax;
}

/* 0x4a65e0: closeNetworkAt; ecx as it leaves it. */
nfs::u32 closeNetwork(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto closed = closeNetworkAt(app, ts, args.ecx))
    {
        args.ecx = closed->ecx;
        answer(ts, args, closed->eax);
    }
    return args.eax;
}

/* 0x4a6650: machineOf the connection eax. */
nfs::u32 machineNumber(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, machineOf(app, args.eax));
    return args.eax;
}

/* 0x4a6690: machineOfPlayer of ax. */
nfs::u32 playerMachine(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, machineOfPlayer(app, nfs::u16(args.eax)));
    return args.eax;
}

/* 0x4f4c50: traceNet of the name eax. */
nfs::u32 netTrace(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto traced = traceNet(app, ts, args.eax))
        answer(ts, args, *traced);
    return args.eax;
}

/* 0x4f4f50, EA's NET_shutgame: the NET object eax's game shut, edx how
 * (its function +0x24). */
nfs::u32 netShutGame(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto shut = callNet(app, ts, args.eax, kShutGameName, kShutGameSlot, args.edx))
        answer(ts, args, *shut);
    return args.eax;
}

/* 0x4f4f90: endNetGame of the NET object eax. */
nfs::u32 netEndGame(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto ended = endNetGame(app, ts, args.eax))
        answer(ts, args, *ended);
    return args.eax;
}

/* 0x4f4ff0, EA's NET_killplayer: the player edx killed in the NET object
 * eax's game (its function +0x34). */
nfs::u32 netKillPlayer(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto killed = callNet(app, ts, args.eax, kKillPlayerName, kKillPlayerSlot, args.edx))
        answer(ts, args, *killed);
    return args.eax;
}

/* 0x4f5040: checkNetPlayer of the NET object eax, the player edx. */
nfs::u32 netCheckPlayer(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto its = checkNetPlayer(app, ts, args.eax, args.edx))
        answer(ts, args, *its);
    return args.eax;
}

/* 0x4f5280: restoreConnections. */
nfs::u32 connRestore(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto restored = restoreConnections(app, ts, args.eax))
        answer(ts, args, *restored);
    return args.eax;
}

/* 0x4f5310: sendOnConnection eax of edx, ebx long. */
nfs::u32 connSend(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = sendOnConnection(app, ts, args.eax, args.edx, args.ebx))
        answer(ts, args, sent->eax);
    return args.eax;
}

/* 0x4f5330: closeConnection of eax. */
nfs::u32 connClose(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto closed = closeConnection(app, ts, args.eax))
        answer(ts, args, *closed);
    return args.eax;
}

/* 0x50dc00: nth of eax, edx. */
nfs::u32 qnth(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, nth(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50dc20: unlink the element edx from the queue eax. */
nfs::u32 qunlink(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, unlink(app, args.eax, args.edx));
    return args.eax;
}

/* 0x50dde0: freeQueueLock of the queue eax. */
nfs::u32 qfree(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto freed = freeQueueLock(app, ts, args.eax))
        answer(ts, args, *freed);
    return args.eax;
}

/* 0x50e150: removeFromQueue of the element edx from the queue eax. */
nfs::u32 qremove(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto removed = removeFromQueue(app, ts, args.eax, args.edx, args.edi))
    {
        answer(ts, args, *removed);
    }
    return args.eax;
}

/* 0x50e1d0: queueLength of the queue eax. */
nfs::u32 qlength(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto length = queueLength(app, ts, args.eax))
        answer(ts, args, *length);
    return args.eax;
}

/* 0x50e220: queuePosition of the element edx in the queue eax. */
nfs::u32 qposition(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto position = queuePosition(app, ts, args.eax, args.edx))
    {
        answer(ts, args, *position);
    }
    return args.eax;
}

/* 0x50e470: findInQueue of element edx of the queue eax. */
nfs::u32 qfindindex(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto found = findInQueue(app, ts, args.eax, args.edx))
    {
        answer(ts, args, *found);
    }
    return args.eax;
}

/* 0x5123b0: gatherAndSend of the edx pieces at ebx on the connection eax. */
nfs::u32 layerSend(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = gatherAndSend(app, ts, args.eax, args.edx, args.ebx))
        answer(ts, args, sent->eax);
    return args.eax;
}

/* 0x512530: callPacker of the ebx pieces at ecx on the connection eax to
 * layer edx of the queue on the stack, taken off (stdcall). */
nfs::u32 layerCallPacker(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto packed = callPacker(app, ts, args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4)))
    {
        answer(ts, args, packed->eax);
        ts.sp += 4;
    }
    return args.eax;
}

/* 0x512630: packFromTop of the edx pieces at ebx on the connection eax, the
 * queue of layers ecx. */
nfs::u32 layerPackFromTop(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto packed = packFromTop(app, ts, args.eax, args.edx, args.ebx, args.ecx))
        answer(ts, args, packed->eax);
    return args.eax;
}

/* 0x5126c0: freeLayers of the connection eax. */
nfs::u32 layerFree(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto freed = freeLayers(app, ts, args.eax))
        answer(ts, args, *freed);
    return args.eax;
}

/* 0x512770: sendDownLayers of the edx pieces at ebx on the connection eax;
 * edx and ebx as it leaves them. */
nfs::u32 layerSendPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = sendDownLayers(app, ts, args.eax, args.edx, args.ebx))
    {
        args.edx = sent->edx;
        args.ebx = sent->ebx;
        answer(ts, args, sent->eax);
    }
    return args.eax;
}

namespace
{

/* The `n` bytes at `address`, big-endian: the dword read there, its bytes
 * swapped and shifted down, as the code reads them. */
u32 bigEndian(App* app, u32 address, u32 n)
{
    const u32 value = at(app, address);
    const u32 swapped = (value >> 24) | (value >> 8 & 0xff00) | (value << 8 & 0xff0000) | (value << 24);
    return swapped >> ((0x20 - n * 8) & 31);
}

}  // namespace

/* 0x510110: EA's runs (0x4afb; 0x4b31 with a 16-bit length) unpacked from
 * `source` to `to`; the length unpacked, as the header says.  Each code
 * byte: its low 6 bits a count, its top 2 what of: 0 bytes copied, 1 a byte
 * given filled, 2 the last byte filled again (0 before any), 3 a byte given
 * and then nibbles, two to a byte, each under its high nibble.  The fills by
 * the game's memset (0x4e0640) -- the run's table (0x510100) goes to them
 * in this order. */
std::optional<u32> unpackRuns(App* app, ThreadState& ts, u32 source, u32 to)
{
    constexpr u32 kShortLength = 0x4b31;
    u8 fill = 0;
    u32 total, in;
    if (bigEndian(app, source, 2) == kShortLength)
    {
        total = bigEndian(app, source + 2, 2);
        in = source + 4;
    }
    else
    {
        total = bigEndian(app, source + 2, 3);
        in = source + 5;
    }
    u32 left = total;
    if (s32(total) <= 0)
        return total;
    do
    {
        const u8 code = byte(app, in);
        u32 count = code & 0x3f;
        ++in;
        switch (code >> 6)
        {
        case 0:
            copyMemory(app, to, in, count);
            in += count;
            left -= count;
            to += count;
            break;
        case 1:
            fill = byte(app, in);
            ++in;
            [[fallthrough]];
        case 2:
            guest::direct<0x4e0640, watcom::clib::memset>(app, ts, to, fill, count);
            left -= count;
            to += count;
            break;
        default:
        {
            const u8 given = byte(app, in);
            ++in;
            putByte(app, to, given);
            ++to;
            const u8 high = given & 0xf0;
            left -= count;
            for (--count; s32(count) > 0;)
            {
                const u8 nibbles = byte(app, in);
                --count;
                putByte(app, to, u8(nibbles >> 4 | high));
                ++to;
                ++in;
                if (s32(count) > 0)
                {
                    --count;
                    putByte(app, to, u8((nibbles & 0xf) | high));
                    ++to;
                }
            }
            break;
        }
        }
    } while (s32(left) > 0);
    return total;
}

namespace
{

/* 0x510420: a DXT1 block at `block` unpacked to 4 rows of 4 16-bit pixels
 * at `to`, `pitch` bytes apart: its two colours (5-6-5), and between them
 * two thirds and a third of the way (each channel times 0xaaaa and 0x5556,
 * 16 bits up) -- or, with the first not above the second, half way and 0.
 * The last row's last colour. */
u32 unpackBlock(App* app, u32 to, u32 pitch, u32 block)
{
    constexpr u32 kRed = 0xf800, kGreen = 0x7e0, kBlue = 0x1f;
    constexpr u32 kTwoThirds = 0xaaaa, kOneThird = 0x5556;
    u16 colours[4];
    const u32 first = word16(app, block), second = word16(app, block + 2);
    colours[0] = u16(first);
    colours[1] = u16(second);
    const u32 red0 = first & kRed, green0 = first & kGreen, blue0 = first & kBlue;
    const u32 red1 = second & kRed, green1 = second & kGreen, blue1 = second & kBlue;
    if (first > second)
    {
        colours[2] = u16(((red0 * kTwoThirds + red1 * kOneThird) >> 16 & kRed)
                           | ((green0 * kTwoThirds + green1 * kOneThird) >> 16 & kGreen)
                           | ((blue0 * kTwoThirds + blue1 * kOneThird) >> 16 & kBlue));
        colours[3] = u16(((red1 * kTwoThirds + red0 * kOneThird) >> 16 & kRed)
                           | ((green1 * kTwoThirds + green0 * kOneThird) >> 16 & kGreen)
                           | ((blue1 * kTwoThirds + blue0 * kOneThird) >> 16 & kBlue));
    }
    else
    {
        colours[2] = u16((((red0 << 15) + (red1 << 15)) >> 16 & kRed)
                           | (((green0 << 15) + (green1 << 15)) >> 16 & kGreen)
                           | (((blue1 << 15) + (blue0 << 15)) >> 16 & kBlue));
        colours[3] = 0;
    }
    for (u32 row = 0; row < 4; ++row)
    {
        const u8 codes = byte(app, block + 4 + row);
        const u32 line = to + row * pitch;
        for (u32 pixel = 0; pixel < 4; ++pixel)
            putWord16(app, line + pixel * 2, colours[codes >> (pixel * 2) & 3]);
    }
    return colours[byte(app, block + 7) >> 6];
}

}  // namespace

/* 0x510760: EA's DXT1 image (0x18fb) unpacked from `source` to `to`: its
 * length unpacked (3 bytes, big-endian, at +2) and its width (2 bytes at
 * +6), the blocks after them a row of blocks at a time (unpackBlock).  With
 * `blocksToo` 0 only the length; the length either way.  eax, edx and ebx
 * as the code leaves them (edx `to` as handed over, without the blocks). */
Regs unpackDxt(App* app, u32 source, u32 to, u32 blocksToo)
{
    const u32 total = bigEndian(app, source + 2, 3);
    const u32 width = bigEndian(app, source + 6, 2);
    if (blocksToo == 0)
        return {total, to, 0, 0};
    if (s32(total) <= 0)
        return {total, 0, blocksToo, 0};
    // A row of pixels' bytes, a row of blocks' (4 rows), the 3 rows after one.
    const u32 pitch = width * 2, blockRow = width * 8, rowsAfter = width * 6;
    u32 block = source + 8, done = 0;
    do
    {
        if (s32(width) > 0)
        {
            u32 x = 0;
            do
            {
                unpackBlock(app, to, pitch, block);
                x += 4;
                to += 8;
                block += 8;
            } while (s32(x) < s32(width));
        }
        done += blockRow;
        to += rowsAfter;
    } while (s32(done) < s32(total));
    return {total, total, done, 0};
}

namespace
{

/* 0x510820: the byte `code` (its low byte) written out ([0xa0b030] on), or
 * -- its kind ([0xa0b040]) not 0 -- the pair it stands for: its first byte
 * expanded ([0xa0b034]) and then its second ([0xa0b03c]).  eax as the code
 * leaves it: `code`, or the last pair's second byte. */
u32 expandPair(App* app, u32 code)
{
    while (byte(app, at(app, kPairKind) + (code & 0xff)) != 0)
    {
        const u32 pair = code & 0xff;
        expandPair(app, byte(app, at(app, kPairLeft) + pair));
        code = byte(app, at(app, kPairRight) + pair);
    }
    const u32 out = at(app, kPairOut);
    putByte(app, out, u8(code));
    put(app, kPairOut, out + 1);
    return code;
}

}  // namespace

/* 0x510870: EA's byte pairs (0x46fb; 0x47fb with 3 bytes more) unpacked
 * from `source` to `to`; the length unpacked, as the header says (0 for
 * none).  After the length: the escape, the pairs' count and each pair's
 * byte and its two (into tables in the frame, their addresses at 0xa0b034
 * to 0xa0b040); then bytes -- one of no pair as it is, a pair's expanded
 * (expandPair), the escape and the byte after it as that byte, the escape
 * and 0 the end.  Where it read to is left at 0xa0b038.  ts.sp where the
 * call leaves it: the tables under the 2 registers the code pushes, where
 * the code has them.  eax, and edx as the code leaves it. */
Regs unpackPairs(App* app, ThreadState& ts, u32 source, u32 to)
{
    constexpr u32 kPairsMore = 0x47fb;
    constexpr u8 kNone = 0, kEscape = 1, kPair = 0xff;
    const Lowered frame(ts, 2 * 4 + 0x304);
    const u32 lefts = ts.sp, rights = lefts + 0x100, kinds = lefts + 0x200;
    put(app, kPairOut, to);
    put(app, kPairKind, kinds);
    put(app, kPairLeft, lefts);
    put(app, kPairRight, rights);
    if (source == 0)
    {
        put(app, kPairIn, 0);
        return {0, rights, 0, 0};
    }
    u32 in = source + 2;
    if ((u32(byte(app, source)) << 8 | byte(app, source + 1)) == kPairsMore)
        in += 3;
    const u32 total = u32(byte(app, in)) << 16 | u32(byte(app, in + 1)) << 8 | byte(app, in + 2);
    for (u32 i = 0; i < 0x100; ++i)
        putByte(app, at(app, kPairKind) + i, kNone);
    const u8 escape = byte(app, in + 3);
    putByte(app, kinds + escape, kEscape);
    const u32 pairs = byte(app, in + 4);
    in += 5;
    for (u32 i = 0; s32(i) < s32(pairs); ++i, in += 3)
    {
        const u8 pair = byte(app, in);
        putByte(app, at(app, kPairLeft) + pair, byte(app, in + 1));
        putByte(app, at(app, kPairRight) + pair, byte(app, in + 2));
        putByte(app, at(app, kPairKind) + pair, kPair);
    }
    put(app, kPairIn, in);
    for (;;)
    {
        in = at(app, kPairIn);
        const u8 c = byte(app, in);
        const u8 kind = byte(app, at(app, kPairKind) + c);
        ++in;
        if (kind == kNone)
        {
            const u32 out = at(app, kPairOut);
            put(app, kPairIn, in);
            putByte(app, out, c);
            put(app, kPairOut, out + 1);
            continue;
        }
        put(app, kPairIn, in);
        if (nfs::s8(kind) < 0)
        {
            expandPair(app, byte(app, at(app, kPairLeft) + c));
            expandPair(app, byte(app, at(app, kPairRight) + c));
            continue;
        }
        const u8 escaped = byte(app, in);
        ++in;
        if (escaped == 0)
            break;
        const u32 out = at(app, kPairOut);
        put(app, kPairIn, in);
        putByte(app, out, escaped);
        put(app, kPairOut, out + 1);
    }
    put(app, kPairIn, in);
    return {total, 0, 0, 0};
}

/* 0x510110: unpackRuns from eax to edx. */
nfs::u32 runUnpack(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto total = unpackRuns(app, ts, args.eax, args.edx))
        answer(ts, args, *total);
    return args.eax;
}

/* 0x510420: unpackBlock at ebx to eax, edx bytes a row. */
nfs::u32 dxtBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, unpackBlock(app, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x510760: unpackDxt from eax to edx, the blocks too for ebx not 0. */
nfs::u32 dxtUnpack(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, unpackDxt(app, args.eax, args.edx, args.ebx).eax);
    return args.eax;
}

/* 0x510820: expandPair of al. */
nfs::u32 pairExpand(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, expandPair(app, args.eax));
    return args.eax;
}

/* 0x510870: unpackPairs from eax to edx. */
nfs::u32 pairUnpack(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, unpackPairs(app, ts, args.eax, args.edx).eax);
    return args.eax;
}

namespace
{

/* EA's Huffman's bits, read from the top: `bits` those not yet taken,
 * left-justified, `left` how many past 16 of them (signed, below 0 once
 * they run short), `word` the bytes read so far, the last at the bottom,
 * `source` the next two.  `bits` is always `word` shifted up 16 - `left`. */
struct HuffBits
{
    App* app;
    u32 source, word, bits, left;

    /* Two bytes more, once `left` is below 0. */
    void refill()
    {
        word = word << 8 | byte(app, source);
        word = word << 8 | byte(app, source + 1);
        source += 2;
        bits = word << ((0 - left) & 31);
        left += 16;
    }

    /* The top `n` bits taken (shifts as the code's, by `n` & 31). */
    u32 take(u32 n)
    {
        const u32 value = bits >> ((0x20 - n) & 31);
        bits <<= n & 31;
        left -= n;
        if (s32(left) < 0)
            refill();
        return value;
    }

    /* A count: 1 and 2 bits as 4 to 7; else 2 + n bits after n 0s and a 1
     * (3 + n where the next 16 bits are all 0), and 1 above them; less 4.
     * `kept` not 0 (a run's): where the n bits are read from written there,
     * as the code keeps it before reading them. */
    u32 count(u32 kept = 0)
    {
        if (s32(bits) < 0)
            return take(3) - 4;
        u32 n = 2;
        if (bits >> 16 == 0)
            while (++n, take(1) == 0)
                ;
        else
        {
            do
            {
                bits += bits;
                ++n;
            } while (s32(bits) >= 0);
            left -= n - 1;
            bits += bits;
            if (s32(left) < 0)
                refill();
        }
        if (kept != 0)
            put(app, kept, source + 2);
        u32 value;
        if (s32(n) <= 0x10)
            value = take(n);
        else
        {
            const u32 high = take(n - 0x10);
            value = high << 16 | take(0x10);
        }
        return value + (1u << (n & 31)) - 4;
    }
};

/* The frame of 0x50ed50 at `frame`, at the code's depth: the symbols taken,
 * the first byte's lookup -- its symbol and its length (0x40 a longer code,
 * 0x60 the clue's) -- the symbols in their codes' order; by length (+ 4 *
 * length) the codes' counts, their limits (16 bits, left-justified), their
 * first's offset in the symbols; the clue's length, the clue.  The 16-bit
 * offset (+0x4c0) is where a run's long count keeps where it reads. */
constexpr u32 kHuffUsed = 0, kHuffLookupSymbols = 0x100, kHuffLookupLengths = 0x200, kHuffSymbols = 0x300;
constexpr u32 kHuffCounts = 0x400, kHuffLimits = 0x440, kHuffOffsets = 0x480, kHuffRunSource = 0x4c0;
constexpr u32 kHuffClueLength = 0x4d0, kHuffClue = 0x540, kHuffFrame = 0x554;
// A lookup's length for the clue's code.
constexpr u8 kHuffClueCode = 0x60;
// The kinds delta coded after: the sums, the sums of the sums.
constexpr u32 kHuffSums = 0x32fb, kHuffSumsOfSums = 0x34fb;

/* The tables of the frame `f` read: the clue, the counts of each length from
 * 1 on (their limits and offsets with them) until a count fills the code
 * space, the symbols, and the first byte's lookup. */
void readHuffTables(App* app, HuffBits& in, u32 f)
{
    putByte(app, f + kHuffClue, u8(in.take(8)));
    u32 base = 0, total = 0, length = 1, shift = 0xf;
    for (;; ++length, --shift)
    {
        base += base;
        put(app, f + kHuffOffsets + length * 4, base - total);
        const u32 count = in.count();
        base += count;
        total += count;
        put(app, f + kHuffCounts + length * 4, count);
        const u32 limit = count == 0 ? 0 : base << (shift & 31) & 0xffff;
        put(app, f + kHuffLimits + length * 4, limit);
        if (count != 0 && limit == 0)
            break;
    }
    const u32 longest = length;
    put(app, f + kHuffLimits + longest * 4, 0xffffffff);
    for (u32 i = 0; i < 0x100; i += 4)
        put(app, f + kHuffUsed + i, 0);
    u8 symbol = 0xff;
    for (u32 index = 0; s32(index) < s32(total); ++index)
    {
        // How many not yet taken to skip, past the one before.
        u32 skip = in.count() + 1;
        do
        {
            ++symbol;
            if (byte(app, f + kHuffUsed + symbol) == 0)
                --skip;
        } while (skip != 0);
        putByte(app, f + kHuffUsed + symbol, 1);
        putByte(app, f + kHuffSymbols + index, symbol);
    }
    for (u32 i = 0; i < 0x100; i += 4)
        put(app, f + kHuffLookupLengths + i, 0x40404040);
    u32 symbolAt = f + kHuffSymbols, lookupSymbol = f + kHuffLookupSymbols, lookupLength = f + kHuffLookupLengths;
    for (u32 bitsLength = 1, entriesShift = 7; s32(bitsLength) <= s32(longest); ++bitsLength, --entriesShift)
    {
        u32 count = at(app, f + kHuffCounts + bitsLength * 4);
        if (s32(bitsLength) >= 9)
            break;
        const u32 entries = 1u << (entriesShift & 31);
        while (count-- != 0)
        {
            const u8 symbol = byte(app, symbolAt);
            ++symbolAt;
            u8 entryLength = u8(bitsLength);
            if (byte(app, f + kHuffClue) == symbol)
            {
                put(app, f + kHuffClueLength, bitsLength);
                entryLength = kHuffClueCode;
            }
            for (u32 entry = 0; s32(entry) < s32(entries); ++entry)
            {
                putByte(app, lookupSymbol++, symbol);
                putByte(app, lookupLength++, entryLength);
            }
        }
    }
}

}  // namespace

/* 0x50ed50: EA's Huffman with runs (0x30fb, 0x32fb and 0x34fb -- each with
 * the packed length too when odd) unpacked from `source` to `to`, its
 * tables read first when `withTables` is not 0 (else those its frame `f`
 * holds); the length unpacked, as the header says (0 for no source).  The
 * bits are read from the top, 16 at a time (HuffBits): the clue byte; how
 * many codes of each length, from 1 bit until they fill the code space; the
 * symbols, each as how many not yet taken to skip; then codes -- up to 8
 * bits looked up by their first byte, longer ones by the limits of each
 * length -- each a byte, or the clue's code: then a count, a run of the byte
 * before that long, or with 0 a bit: 1 the end, 0 a byte as it is.  0x32fb's
 * bytes are the sums of those unpacked, 0x34fb's the sums of their sums.  A
 * 16-bit code's offset (the frame's +0x4c0) is where a run's long count
 * keeps where it reads: kept, the game's own clash.  The tables stay in the
 * frame at the code's depth (with `withTables` 0 they are what it holds
 * there); a byte is written before the bits after a short code are read
 * further, a longer code's after, as the code goes. */
u32 unpackHuffman(App* app, u32 f, u32 source, u32 to, u32 withTables)
{
    if (source == 0)
        return 0;
    const u32 start = to;
    HuffBits in{app, source + 4, bigEndian(app, source, 4), 0, 0};
    in.bits = in.word << 16;
    u32 type = in.word >> 16;
    if ((type & 0x100) != 0)
    {
        // The packed length skipped.
        in.word = bigEndian(app, source + 4, 4);
        in.source = source + 8;
        in.left = 8;
        in.bits = in.word << 8;
    }
    type &= ~u32(0x100);
    const u32 high = in.take(8);
    const u32 length = in.take(0x10) | high << 16;
    if (withTables != 0)
        readHuffTables(app, in, f);
    for (;;)
    {
        const u8 first = u8(in.bits >> 24);
        const u32 bitsLength = byte(app, f + kHuffLookupLengths + first);
        if (s32(in.left - bitsLength) >= -0x10)
        {
            // A short code: its byte, then the bits after it.
            putByte(app, to, byte(app, f + kHuffLookupSymbols + first));
            ++to;
            in.bits <<= bitsLength & 31;
            in.left -= bitsLength;
            if (s32(in.left) < 0)
                in.refill();
            continue;
        }
        u32 n;
        if (bitsLength == kHuffClueCode)
            n = at(app, f + kHuffClueLength);
        else
        {
            const u32 top = in.bits >> 16;
            n = 8;
            do
                ++n;
            while (top >= at(app, f + kHuffLimits + n * 4));
        }
        const u32 code = in.take(n);
        const u8 symbol = byte(app, f + kHuffSymbols + (code - at(app, f + kHuffOffsets + n * 4)));
        if (symbol != byte(app, f + kHuffClue))
        {
            putByte(app, to, symbol);
            ++to;
            continue;
        }
        const u32 run = in.count(f + kHuffRunSource);
        if (run != 0)
        {
            const u8 again = byte(app, to - 1);
            const u32 end = to + run;
            do
                putByte(app, to++, again);
            while (to < end);
            continue;
        }
        if (in.take(1) != 0)
            break;
        putByte(app, to, u8(in.take(8)));
        ++to;
    }
    // The sums.
    const u32 end = start + length;
    u32 sum = 0, sumOfSums = 0;
    if (type == kHuffSums)
        for (u32 at = start; at < end; ++at)
        {
            sum += byte(app, at);
            putByte(app, at, u8(sum));
        }
    else if (type == kHuffSumsOfSums)
        for (u32 at = start; at < end; ++at)
        {
            sum += byte(app, at);
            sumOfSums += sum;
            putByte(app, at, u8(sumOfSums));
        }
    return length;
}

/* 0x50ed50: unpackHuffman from eax to edx, the tables read for ebx not 0;
 * the frame under the 4 registers the code pushes. */
nfs::u32 huffUnpack(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(ts, args, unpackHuffman(app, ts.sp - kHuffFrame, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* ---- The typed cores of native_ea_net.h's list for the game's other code (guest::direct): the
 * functions above as their thunks run them. ---- */

u32 sessionIndexOf(App* app, u32 id)
{
    return findSession(app, id);
}

std::optional<u32> connectionDropped(App* app, ThreadState& ts, u32 connection)
{
    return dropConnection(app, ts, connection);
}

std::optional<u32> machineLeftAnnounced(App* app, ThreadState& ts, u32 machine)
{
    const Saved saved{};  // the pushed registers' values are dropped
    return announceLeft(app, ts, machine, saved);
}

std::optional<u32> networkReset(App* app, ThreadState& ts, u32 alsoLostToo)
{
    return resetNetworkState(app, ts, alsoLostToo);
}

std::optional<u32> networkClosed(App* app, ThreadState& ts)
{
    const auto closed = closeNetworkAt(app, ts, 0);
    if (!closed)
        return std::nullopt;
    return closed->eax;
}

u32 machineOfPlayerNumber(App* app, u32 player)
{
    return machineOfPlayer(app, u16(player));
}

std::optional<u32> netCallTraced(App* app, ThreadState& ts, u32 name)
{
    return traceNet(app, ts, name);
}

std::optional<u32> connectionsRestored(App* app, ThreadState& ts, u32 handed)
{
    return restoreConnections(app, ts, handed);
}

u32 queueUnlinked(App* app, u32 queue, u32 element)
{
    return unlink(app, queue, element);
}

std::optional<u32> queueLockFreed(App* app, ThreadState& ts, u32 queue)
{
    return freeQueueLock(app, ts, queue);
}

std::optional<u32> queueLengthOf(App* app, ThreadState& ts, u32 queue)
{
    return queueLength(app, ts, queue);
}

std::optional<u32> networkGameEnded(App* app, ThreadState& ts, u32 handed)
{
    return endNetwork(app, ts, handed);
}

std::optional<u32> netGameEnded(App* app, ThreadState& ts, u32 net)
{
    return endNetGame(app, ts, net);
}

std::optional<u32> netGameShut(App* app, ThreadState& ts, u32 net, u32 how)
{
    return callNet(app, ts, net, kShutGameName, kShutGameSlot, how);
}

std::optional<u32> netPlayerChecked(App* app, ThreadState& ts, u32 net, u32 player, u32 third)
{
    return checkNetPlayerWith(app, ts, net, player, third);
}

std::optional<u32> queueRemoved(App* app, ThreadState& ts, u32 queue, u32 element, u32 forNone)
{
    return removeFromQueue(app, ts, queue, element, forNone);
}

std::optional<u32> queuePositionOf(App* app, ThreadState& ts, u32 queue, u32 element)
{
    return queuePosition(app, ts, queue, element);
}

std::optional<u32> queueElementAt(App* app, ThreadState& ts, u32 queue, u32 index)
{
    return findInQueue(app, ts, queue, index);
}

}  // namespace ea
}  // namespace nfs3hp
