#ifndef NFS3HP_NATIVE_GAME_H
#define NFS3HP_NATIVE_GAME_H

#include "native_guest.h"

/* EA's fill (native_game.cpp) as typed C++, for the game's own code that calls it directly
 * (guest::direct): the two forms whose bytes are all one value, which comes to a memset.  0x4e0721
 * with a pattern of four different bytes stays by address.  By address: as the game's code calls
 * each, the place in eax, the count in edx, the byte in bl. */
namespace nfs3hp
{
namespace ea
{

// 0x4eba70 AcquireWindow: the game's window.
x86::reg32 acquireWindow(win32::WinApplication* app);
// 0x4e070c: `count` bytes at `at` cleared -- none for a count with its top bit set; the eax it leaves (where the fill stopped).
x86::reg32 clearBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count);
// 0x4e0716: `count` bytes at `at` of `value`'s low byte -- none for a count with its top bit set.
void fillBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count, x86::reg32 value);
// 0x4e0721: `count` bytes at `at` of the four-byte `pattern`; the eax it leaves (where the fill stopped).
x86::reg32 fillPattern(win32::WinApplication* app, x86::reg32 at, x86::reg32 count, x86::reg32 pattern);
// THRASH_opendll, 0x4f8390: the renderer `name` ("voodoo2", or with an "a") loaded and looked up: 1 if it loaded
// and is (or will not say) 3dfx's of version 104, else 0; nullopt when the game stopped.  Called through
// guest::direct only: its buffer lies below the callee's entry esp as the game's does.
std::optional<x86::reg32> thrashOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// Message, 0x4df6c0, cdecl: `format` printed to stdout (no arguments); the eax it leaves (what the print gave).
// Through guest::direct only, as error.
std::optional<x86::reg32> messageOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format);
// Error, 0x401010, as error, with `eax` the caller has at the call; the eax Error leaves when it returns (the
// game ends in it otherwise: nullopt).
std::optional<x86::reg32> errorLeaves(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32 format,
                                      guest::Words words);
// Error, 0x401010, cdecl: the game's abort, `format` with `words` -- run as its code ran (Error in
// native_game.cpp), every register but eax kept; cpu.terminate set as the game ends.  Called through
// guest::direct only: its frame at the depth of the game's call.
void error(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, guest::Words words);
// Message, 0x4df6c0, cdecl: printf to stdout, `format` with `words` -- run as its code ran (Message in
// native_game.cpp); called through guest::direct only: its frame at the depth of the game's call.
void message(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, guest::Words words);

}  // namespace ea

namespace removed
{

// 0x4f1c70: the processor "checked": the mixing's MMX set on, no timestamp counter.
void checkProcessor(win32::WinApplication* app);
// 0x4f1ef0: the processor's speed "timed": nothing.
void timeProcessor(win32::WinApplication* app);

}  // namespace removed

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
