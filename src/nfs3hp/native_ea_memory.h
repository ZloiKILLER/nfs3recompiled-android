#ifndef NFS3HP_NATIVE_EA_MEMORY_H
#define NFS3HP_NATIVE_EA_MEMORY_H

#include "native_guest.h"

/* native_ea_memory.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

/* What reservememadr's code leaves in ebx and edx: its last callee's, or what it put there. */
struct ReserveLeft
{
    nfs::u32 ebx, edx;
};

// The game's 0x4e15d0: the size asked for of the block whose body is at `body`.
nfs::u32 askedSize(nfs::App* app, nfs::u32 body);
// The game's 0x4e1620, reservememadr: `size` bytes of the class `flags` names, named `name` (or 0):
// the body's address, or 0; `left` what the code leaves in ebx and edx.
std::optional<nfs::u32> reserveBlock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 size,
                                       nfs::u32 flags, ReserveLeft& left);
// reserveBlock for a caller that reads only what the game's code leaves in eax: the body's address,
// or 0.
inline std::optional<nfs::u32> reserved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 size,
                                          nfs::u32 flags)
{
    ReserveLeft left = {};
    return reserveBlock(app, ts, name, size, flags, left);
}
// The game's 0x4e1890, purgememadr: the block at `body` given back to its class: 1 (for none too), or
// 0 for one not in use.
std::optional<nfs::u32> purgeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::u32 body);
// The game's 0x4ea4f0, memmove: `count` bytes from `from` to `to`.
bool moveMemory(nfs::App* app, nfs::ThreadState& ts, nfs::u32 to, nfs::u32 from, nfs::u32 count);
// The game's 0x4e15e0: the name of the block at `body`, past its body and its class's bytes; 0 for
// one not named.
nfs::u32 blockNameOf(nfs::App* app, nfs::u32 body);

}  // namespace ea

namespace ea
{
// The game's 0x4e19d0 (stdcall): `size` bytes of the default class, not named: the body's address, or 0.
std::optional<nfs::u32> defaultReserved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size);
// The game's 0x4e19f0 (stdcall): the block at `body` given back: purgememadr's answer.
std::optional<nfs::u32> defaultPurged(nfs::App* app, nfs::ThreadState& ts, nfs::u32 body);
}  // namespace ea

namespace ea
{
// The game's 0x4e1580: the size of the default class's largest free block, or 0.
nfs::u32 defaultLargest(nfs::App* app);
// The game's 0x4e1080: `block` put into class `cls`'s free list: what the code leaves in eax.
nfs::u32 freeLinked(nfs::App* app, nfs::u32 cls, nfs::u32 block);
// The game's 0x4e10e0: `block` taken out of its free list (`cls` is not looked at): what the code leaves in eax.
nfs::u32 freeUnlinked(nfs::App* app, nfs::u32 cls, nfs::u32 block);
// The game's 0x4e11f0: the bytes a block of the class `flags` names, named `name`, takes past its body.
nfs::u32 blockOverhead(nfs::App* app, nfs::u32 name, nfs::u32 flags);
// The game's 0x4e1120: `block` marked in use: `name`, `size`, the class's bytes `bytesOfClass`, `flags` (the
// first stack word), the blocks `before` and `after` it (the second and third): its bytes.
nfs::u32 blockMarked(nfs::App* app, nfs::u32 block, nfs::u32 name, nfs::u32 size,
                       nfs::u32 bytesOfClass, nfs::u32 flags, nfs::u32 before, nfs::u32 after);
}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
