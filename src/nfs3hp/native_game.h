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
nfs::u32 acquireWindow(nfs::App* app);
// 0x4e070c: `count` bytes at `at` cleared -- none for a count with its top bit set; the eax it leaves (where the fill stopped).
nfs::u32 clearBytes(nfs::App* app, nfs::u32 at, nfs::u32 count);
// 0x4e0716: `count` bytes at `at` of `value`'s low byte -- none for a count with its top bit set.
void fillBytes(nfs::App* app, nfs::u32 at, nfs::u32 count, nfs::u32 value);
// 0x4e0721: `count` bytes at `at` of the four-byte `pattern`; the eax it leaves (where the fill stopped).
nfs::u32 fillPattern(nfs::App* app, nfs::u32 at, nfs::u32 count, nfs::u32 pattern);
// THRASH_opendll, 0x4f8390: the renderer `name` ("voodoo2", or with an "a") loaded and looked up: 1 if it loaded
// and is (or will not say) 3dfx's of version 104, else 0; nullopt when the game stopped.  Called through
// guest::direct only: its buffer lies below the callee's entry esp as the game's does.
std::optional<nfs::u32> thrashOpened(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// Message, 0x4df6c0, cdecl: `format` printed to stdout (no arguments); the eax it leaves (what the print gave).
// Through guest::direct only, as error.
std::optional<nfs::u32> messageOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 format);
// Error, 0x401010, as error, with `eax` the caller has at the call; the eax Error leaves when it returns (the
// game ends in it otherwise: nullopt).
std::optional<nfs::u32> errorLeaves(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax, nfs::u32 format,
                                      guest::Words words);
// Error, 0x401010, cdecl: the game's abort, `format` with `words` -- run as its code ran (Error in
// native_game.cpp), every register but eax kept; ts.terminate set as the game ends.  Called through
// guest::direct only: its frame at the depth of the game's call.
void error(nfs::App* app, nfs::ThreadState& ts, nfs::u32 format, guest::Words words);
// Message, 0x4df6c0, cdecl: printf to stdout, `format` with `words` -- run as its code ran (Message in
// native_game.cpp); called through guest::direct only: its frame at the depth of the game's call.
void message(nfs::App* app, nfs::ThreadState& ts, nfs::u32 format, guest::Words words);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
