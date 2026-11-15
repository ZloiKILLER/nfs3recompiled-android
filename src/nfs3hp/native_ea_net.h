#ifndef NFS3HP_NATIVE_EA_NET_H
#define NFS3HP_NATIVE_EA_NET_H

#include "native_guest.h"

#include <type_traits>

/* native_ea_net.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one.
 * The Huffman unpacker (0x50ed50) is not one: it is still its code, register by register. */
namespace nfs3hp
{
namespace ea
{

/* What a call leaves in eax, edx, ebx and ecx -- or what the code hands it
 * in them, arguments or not: a stand-in, a Windows function may leave a
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

}  // namespace ea

namespace guest
{

// The connection in eax, the packet in edx, its length in ebx; eax, edx and ebx back.
template <>
struct ByAddress<ea::packetSent>
{
    template <x86::reg32 Address>
    static std::optional<ea::Regs> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 connection,
                                        x86::reg32 packet, x86::reg32 length)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = connection;
        cpu.edx = packet;
        cpu.ebx = length;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return ea::Regs{cpu.eax, cpu.edx, cpu.ebx, 0};
    }
};

// The connection in eax; the machine back in eax.
template <>
struct ByAddress<ea::machineOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 connection)
    {
        return watcomEax(app, cpu, Address, {connection}).value_or(0);
    }
};

// The source in eax, where to in edx; the length back in eax.
template <>
struct ByAddress<ea::unpackRuns>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 to)
    {
        return watcomEax(app, cpu, Address, {source, to});
    }
};

// The source in eax, where to in edx, blocksToo in ebx; eax, edx and ebx back (ecx 0, as the core
// gives it).
template <>
struct ByAddress<ea::unpackDxt>
{
    template <x86::reg32 Address>
    static ea::Regs call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 to,
                         x86::reg32 blocksToo)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = source;
        cpu.edx = to;
        cpu.ebx = blocksToo;
        guest::call(app, cpu, Address);
        return ea::Regs{cpu.eax, cpu.edx, cpu.ebx, 0};
    }
};

// The source in eax, where to in edx; eax and edx back (ebx and ecx 0, as the core gives them).
template <>
struct ByAddress<ea::unpackPairs>
{
    template <x86::reg32 Address>
    static ea::Regs call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 to)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = source;
        cpu.edx = to;
        guest::call(app, cpu, Address);
        return ea::Regs{cpu.eax, cpu.edx, 0, 0};
    }
};


/* Those above by address: their words in eax and edx, every other register kept (none of them
 * changes one but eax); eax back. */
template <typename Result, typename... Words>
struct EaNetByAddress
{
    template <x86::reg32 Address>
    static Result call(win32::WinApplication* app, x86::CPU& cpu, Words... words)
    {
        const std::optional<x86::reg32> eax = watcomEax(app, cpu, Address, {x86::reg32(words)...});
        if constexpr (std::is_same_v<Result, x86::reg32>)
            return eax.value_or(0);
        else
            return eax;
    }
};

template <> struct ByAddress<ea::sessionIndexOf> : EaNetByAddress<x86::reg32, x86::reg32> {};
template <> struct ByAddress<ea::connectionDropped> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::machineLeftAnnounced> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::networkReset> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::networkClosed> : EaNetByAddress<std::optional<x86::reg32>> {};
template <> struct ByAddress<ea::machineOfPlayerNumber> : EaNetByAddress<x86::reg32, x86::reg32> {};
template <> struct ByAddress<ea::netCallTraced> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::connectionsRestored> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::queueUnlinked> : EaNetByAddress<x86::reg32, x86::reg32, x86::reg32> {};
template <> struct ByAddress<ea::queueLockFreed> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};
template <> struct ByAddress<ea::queueLengthOf> : EaNetByAddress<std::optional<x86::reg32>, x86::reg32> {};

}  // namespace guest
}  // namespace nfs3hp

#endif
