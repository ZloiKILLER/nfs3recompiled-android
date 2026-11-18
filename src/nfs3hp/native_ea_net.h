#ifndef NFS3HP_NATIVE_EA_NET_H
#define NFS3HP_NATIVE_EA_NET_H

#include "native_guest.h"

#include <type_traits>

/* native_ea_net.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

/* What a call leaves in eax, edx, ebx and ecx -- or what the code hands it
 * in them, arguments or not: a Windows function may leave a
 * register as it was handed. */
struct Regs
{
    x86::reg32 eax, edx, ebx, ecx;
};

// The game's 0x4a60e0: a packet of `length` bytes at `packet` sent to the connection `connection`,
// or with 0 to all; eax, edx and ebx as the code leaves them.
std::optional<Regs> sendPacketTo(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 connection,
                                 x86::reg32 packet, x86::reg32 length);
// sendPacketTo for a caller: eax, edx and ebx as the code leaves them, ecx 0 (what the code leaves
// there is its caller's own).
inline std::optional<Regs> packetSent(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 connection,
                                      x86::reg32 packet, x86::reg32 length)
{
    const std::optional<Regs> sent = sendPacketTo(app, cpu, connection, packet, length);
    if (!sent)
        return std::nullopt;
    return Regs{sent->eax, sent->edx, sent->ebx, 0};
}
// The game's 0x4a6650: the number of the machine whose connection is `connection`, or -1.
x86::reg32 machineOf(win32::WinApplication* app, x86::reg32 connection);
// The game's 0x510110: EA's runs unpacked from `source` to `to`: the length unpacked.
std::optional<x86::reg32> unpackRuns(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 to);
// The game's 0x510760: EA's DXT1 image unpacked from `source` to `to`, the blocks too for
// `blocksToo` not 0: eax (the length), edx and ebx as the code leaves them, ecx 0.
Regs unpackDxt(win32::WinApplication* app, x86::reg32 source, x86::reg32 to, x86::reg32 blocksToo);
// The game's 0x510870: EA's byte pairs unpacked from `source` to `to`: eax (the length) and edx as
// the code leaves them, ebx and ecx 0.
Regs unpackPairs(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 to);
// The game's 0x49f1c0: the index of the session `id` in the sessions' table, or -1.
x86::reg32 sessionIndexOf(win32::WinApplication* app, x86::reg32 id);
// The game's 0x49ecb0: the connection `connection` dropped: eax as the code leaves it.
std::optional<x86::reg32> connectionDropped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 connection);
// The game's 0x49ecf0: the machine `machine` announced gone to the others (the registers as the
// caller has them pushed, as the code pushes them): eax as the code leaves it.
std::optional<x86::reg32> machineLeftAnnounced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 machine);
// The game's 0x4a6390: the network's state made anew, the second lost flag cleared too for
// `alsoLostToo` not 0: eax as the last clearing leaves it.
std::optional<x86::reg32> networkReset(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 alsoLostToo);
// The game's 0x4a65e0: the network closed: eax as the code leaves it.
std::optional<x86::reg32> networkClosed(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4a6690: the machine of the player `player` (its low 16 bits).
x86::reg32 machineOfPlayerNumber(win32::WinApplication* app, x86::reg32 player);
// The game's 0x4f4c50: NET's trace of the call named `name`: eax as the code leaves it.
std::optional<x86::reg32> netCallTraced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// The game's 0x4f5280: the connections restored, `handed` the eax the code hands it: eax as the code
// leaves it.
std::optional<x86::reg32> connectionsRestored(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handed);
// The game's 0x50dc20: the element `element` unlinked from the queue `queue`: eax as the code leaves it.
x86::reg32 queueUnlinked(win32::WinApplication* app, x86::reg32 queue, x86::reg32 element);
// The game's 0x50dde0: the queue `queue`'s lock freed: eax as the code leaves it.
std::optional<x86::reg32> queueLockFreed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue);
// The game's 0x50e1d0: how many elements the queue `queue` holds.
std::optional<x86::reg32> queueLengthOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue);
// The game's 0x49eed0: the network game ended, `handed` the eax the code hands it: eax as the code
// leaves it.
std::optional<x86::reg32> networkGameEnded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handed);
// The game's 0x4f4f50, NET_shutgame: the NET object `net`'s game shut, `how` handed to its function.
std::optional<x86::reg32> netGameShut(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 net, x86::reg32 how);
// The game's 0x4f4f90, NET_endgame: the NET object `net`'s game ended.
std::optional<x86::reg32> netGameEnded(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 net);
// The game's 0x4f5040: the NET object `net`'s check asked with its game, `player` and `third` (edx and
// ebx, which the check's function reads).
std::optional<x86::reg32> netPlayerChecked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 net,
                                           x86::reg32 player, x86::reg32 third);
// The game's 0x50e150, qremove: the element taken out of the queue; 1 if it was there, `forNone`
// (the caller's edi) asked for none.
std::optional<x86::reg32> queueRemoved(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue,
                                       x86::reg32 element, x86::reg32 forNone);
// The game's 0x50e220, qposition: the element's index in the queue, or -1.
std::optional<x86::reg32> queuePositionOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue,
                                          x86::reg32 element);
// The game's 0x50e470, qfindindex: the queue's element `index`, or 0.
std::optional<x86::reg32> queueElementAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue,
                                         x86::reg32 index);

}  // namespace ea

}  // namespace nfs3hp

#endif
