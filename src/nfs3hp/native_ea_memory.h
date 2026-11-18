#ifndef NFS3HP_NATIVE_EA_MEMORY_H
#define NFS3HP_NATIVE_EA_MEMORY_H

#include "native_guest.h"

/* native_ea_memory.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
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
// The game's 0x4e15e0: the name of the block at `body`, past its body and its class's bytes; 0 for
// one not named.
x86::reg32 blockNameOf(win32::WinApplication* app, x86::reg32 body);

}  // namespace ea

namespace ea
{
// The game's 0x4e19d0 (stdcall): `size` bytes of the default class, not named: the body's address, or 0.
std::optional<x86::reg32> defaultReserved(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size);
// The game's 0x4e19f0 (stdcall): the block at `body` given back: purgememadr's answer.
std::optional<x86::reg32> defaultPurged(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 body);
}  // namespace ea

namespace ea
{
// The game's 0x4e1580: the size of the default class's largest free block, or 0.
x86::reg32 defaultLargest(win32::WinApplication* app);
// The game's 0x4e1080: `block` put into class `cls`'s free list: what the code leaves in eax.
x86::reg32 freeLinked(win32::WinApplication* app, x86::reg32 cls, x86::reg32 block);
// The game's 0x4e10e0: `block` taken out of its free list (`cls` is not looked at): what the code leaves in eax.
x86::reg32 freeUnlinked(win32::WinApplication* app, x86::reg32 cls, x86::reg32 block);
// The game's 0x4e11f0: the bytes a block of the class `flags` names, named `name`, takes past its body.
x86::reg32 blockOverhead(win32::WinApplication* app, x86::reg32 name, x86::reg32 flags);
// The game's 0x4e1120: `block` marked in use: `name`, `size`, the class's bytes `bytesOfClass`, `flags` (the
// first stack word), the blocks `before` and `after` it (the second and third): its bytes.
x86::reg32 blockMarked(win32::WinApplication* app, x86::reg32 block, x86::reg32 name, x86::reg32 size,
                       x86::reg32 bytesOfClass, x86::reg32 flags, x86::reg32 before, x86::reg32 after);
}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
