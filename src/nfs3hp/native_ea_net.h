#ifndef NFS3HP_NATIVE_EA_NET_H
#define NFS3HP_NATIVE_EA_NET_H

#include "native_guest.h"

#include <type_traits>

/* native_ea_net.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

/* What a call leaves in eax, edx, ebx and ecx -- or what the code hands it
 * in them, arguments or not: a Windows function may leave a
 * register as it was handed. */
struct Regs
{
    nfs::u32 eax, edx, ebx, ecx;
};

// The game's 0x4a60e0: a packet of `length` bytes at `packet` sent to the connection `connection`,
// or with 0 to all; eax, edx and ebx as the code leaves them.
std::optional<Regs> sendPacketTo(nfs::App* app, nfs::ThreadState& ts, nfs::u32 connection,
                                 nfs::u32 packet, nfs::u32 length);
// sendPacketTo for a caller: eax, edx and ebx as the code leaves them, ecx 0 (what the code leaves
// there is its caller's own).
inline std::optional<Regs> packetSent(nfs::App* app, nfs::ThreadState& ts, nfs::u32 connection,
                                      nfs::u32 packet, nfs::u32 length)
{
    const std::optional<Regs> sent = sendPacketTo(app, ts, connection, packet, length);
    if (!sent)
        return std::nullopt;
    return Regs{sent->eax, sent->edx, sent->ebx, 0};
}
// The game's 0x4a6650: the number of the machine whose connection is `connection`, or -1.
nfs::u32 machineOf(nfs::App* app, nfs::u32 connection);
// The game's 0x510110: EA's runs unpacked from `source` to `to`: the length unpacked.
std::optional<nfs::u32> unpackRuns(nfs::App* app, nfs::ThreadState& ts, nfs::u32 source, nfs::u32 to);
// The game's 0x510760: EA's DXT1 image unpacked from `source` to `to`, the blocks too for
// `blocksToo` not 0: eax (the length), edx and ebx as the code leaves them, ecx 0.
Regs unpackDxt(nfs::App* app, nfs::u32 source, nfs::u32 to, nfs::u32 blocksToo);
// The game's 0x510870: EA's byte pairs unpacked from `source` to `to`: eax (the length) and edx as
// the code leaves them, ebx and ecx 0.
Regs unpackPairs(nfs::App* app, nfs::ThreadState& ts, nfs::u32 source, nfs::u32 to);
// The game's 0x49f1c0: the index of the session `id` in the sessions' table, or -1.
nfs::u32 sessionIndexOf(nfs::App* app, nfs::u32 id);
// The game's 0x49ecb0: the connection `connection` dropped: eax as the code leaves it.
std::optional<nfs::u32> connectionDropped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 connection);
// The game's 0x49ecf0: the machine `machine` announced gone to the others (the registers as the
// caller has them pushed, as the code pushes them): eax as the code leaves it.
std::optional<nfs::u32> machineLeftAnnounced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 machine);
// The game's 0x4a6390: the network's state made anew, the second lost flag cleared too for
// `alsoLostToo` not 0: eax as the last clearing leaves it.
std::optional<nfs::u32> networkReset(nfs::App* app, nfs::ThreadState& ts, nfs::u32 alsoLostToo);
// The game's 0x4a65e0: the network closed: eax as the code leaves it.
std::optional<nfs::u32> networkClosed(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4a6690: the machine of the player `player` (its low 16 bits).
nfs::u32 machineOfPlayerNumber(nfs::App* app, nfs::u32 player);
// The game's 0x4f4c50: NET's trace of the call named `name`: eax as the code leaves it.
std::optional<nfs::u32> netCallTraced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// The game's 0x4f5280: the connections restored, `handed` the eax the code hands it: eax as the code
// leaves it.
std::optional<nfs::u32> connectionsRestored(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handed);
// The game's 0x50dc20: the element `element` unlinked from the queue `queue`: eax as the code leaves it.
nfs::u32 queueUnlinked(nfs::App* app, nfs::u32 queue, nfs::u32 element);
// The game's 0x50dde0: the queue `queue`'s lock freed: eax as the code leaves it.
std::optional<nfs::u32> queueLockFreed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 queue);
// The game's 0x50e1d0: how many elements the queue `queue` holds.
std::optional<nfs::u32> queueLengthOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 queue);
// The game's 0x49eed0: the network game ended, `handed` the eax the code hands it: eax as the code
// leaves it.
std::optional<nfs::u32> networkGameEnded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handed);
// The game's 0x4f4f50, NET_shutgame: the NET object `net`'s game shut, `how` handed to its function.
std::optional<nfs::u32> netGameShut(nfs::App* app, nfs::ThreadState& ts, nfs::u32 net, nfs::u32 how);
// The game's 0x4f4f90, NET_endgame: the NET object `net`'s game ended.
std::optional<nfs::u32> netGameEnded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 net);
// The game's 0x4f5040: the NET object `net`'s check asked with its game, `player` and `third` (edx and
// ebx, which the check's function reads).
std::optional<nfs::u32> netPlayerChecked(nfs::App* app, nfs::ThreadState& ts, nfs::u32 net,
                                           nfs::u32 player, nfs::u32 third);
// The game's 0x50e150, qremove: the element taken out of the queue; 1 if it was there, `forNone`
// (the caller's edi) asked for none.
std::optional<nfs::u32> queueRemoved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 queue,
                                       nfs::u32 element, nfs::u32 forNone);
// The game's 0x50e220, qposition: the element's index in the queue, or -1.
std::optional<nfs::u32> queuePositionOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 queue,
                                          nfs::u32 element);
// The game's 0x50e470, qfindindex: the queue's element `index`, or 0.
std::optional<nfs::u32> queueElementAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 queue,
                                         nfs::u32 index);

}  // namespace ea

}  // namespace nfs3hp

#endif
