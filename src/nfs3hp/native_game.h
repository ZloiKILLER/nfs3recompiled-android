#ifndef NFS3HP_NATIVE_GAME_H
#define NFS3HP_NATIVE_GAME_H

#include "native_guest.h"

/* EA's fill (native_game.cpp) as typed C++, for the game's own code that calls it directly
 * (guest::direct): the two forms whose bytes are all one value, which comes to a memset.  0x4e0721
 * with a pattern of four different bytes stays by address.  ByAddress: as the game's code calls
 * each, the place in eax, the count in edx, the byte in bl. */
namespace nfs3hp
{
namespace ea
{

// 0x4e070c: `count` bytes at `at` cleared -- none for a count with its top bit set.
void clearBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count);
// 0x4e0716: `count` bytes at `at` of `value`'s low byte -- none for a count with its top bit set.
void fillBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count, x86::reg32 value);
// Error, 0x401010, cdecl: the game's abort, `format` with `words` -- run as its code ran (Error in
// native_game.cpp), every register but eax kept; cpu.terminate set as the game ends.  Called through
// guest::direct only: its frame at the depth of the game's call.
void error(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, guest::Words words);

}  // namespace ea

namespace guest
{

/* The registers loaded and left as the game's code leaves them (as native_watcom.h's
 * WatcomByAddress). */
template <>
struct ByAddress<ea::clearBytes>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 at, x86::reg32 count)
    {
        cpu.eax = at;
        cpu.edx = count;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<ea::fillBytes>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 at, x86::reg32 count, x86::reg32 value)
    {
        cpu.eax = at;
        cpu.edx = count;
        cpu.ebx = value;
        guest::call(app, cpu, Address);
    }
};

// The format and the words pushed.
template <>
struct ByAddress<ea::error>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, Words words)
    {
        guest::call(app, cpu, Address, {format}, words);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
