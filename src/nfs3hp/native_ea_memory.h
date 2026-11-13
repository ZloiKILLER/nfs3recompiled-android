#ifndef NFS3HP_NATIVE_EA_MEMORY_H
#define NFS3HP_NATIVE_EA_MEMORY_H

#include "native_guest.h"

/* native_ea_memory.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

/* What reservememadr's code leaves in ebx and edx: its last callee's, or what it put there. */
struct ReserveLeft
{
    x86::reg32 ebx, edx;
};

// The game's 0x4e15d0: the size asked for of the block whose body is at `body`.
x86::reg32 askedSize(win32::WinApplication* app, x86::reg32 body);
// The game's 0x4e1620, reservememadr: `size` bytes of the class `flags` names, named `name` (or 0):
// the body's address, or 0; `left` what the code leaves in ebx and edx.
std::optional<x86::reg32> reserveBlock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 size,
                                       x86::reg32 flags, ReserveLeft& left);
// reserveBlock for a caller that reads only what the game's code leaves in eax: the body's address,
// or 0.
inline std::optional<x86::reg32> reserved(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 size,
                                          x86::reg32 flags)
{
    ReserveLeft left = {};
    return reserveBlock(app, cpu, name, size, flags, left);
}
// The game's 0x4e1890, purgememadr: the block at `body` given back to its class: 1 (for none too), or
// 0 for one not in use.
std::optional<x86::reg32> purgeBlock(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 body);
// The game's 0x4ea4f0, memmove: `count` bytes from `from` to `to`.
bool moveMemory(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 from, x86::reg32 count);

}  // namespace ea

namespace guest
{

// The body in eax; the size back in eax.
template <>
struct ByAddress<ea::askedSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 body)
    {
        return watcomEax(app, cpu, Address, {body}).value_or(0);
    }
};

// The name in eax, the size in edx, the flags in ebx; the body back in eax.
template <>
struct ByAddress<ea::reserved>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 size,
                                          x86::reg32 flags)
    {
        return watcomEax(app, cpu, Address, {name, size, flags});
    }
};

// The body in eax; eax back.
template <>
struct ByAddress<ea::purgeBlock>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 body)
    {
        return watcomEax(app, cpu, Address, {body});
    }
};

// From in eax, to in edx, the count in ebx.
template <>
struct ByAddress<ea::moveMemory>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 from, x86::reg32 count)
    {
        return watcom(app, cpu, Address, {from, to, count});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
