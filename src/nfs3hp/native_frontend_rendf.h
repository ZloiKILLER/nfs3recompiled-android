#ifndef NFS3HP_NATIVE_FRONTEND_RENDF_H
#define NFS3HP_NATIVE_FRONTEND_RENDF_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendf.cpp's typed functions (a row of a shape's pixels from one format to
 * another, the display mode, the font's allocator), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax.
 *
 * The rows also leave in ebx and edx what their assembler does (the count run below zero, the
 * pointer past the destination, what it last read ahead), which convshp's callers go on with: a
 * caller that reads them calls by address. */
namespace nfs3hp
{
namespace frontend
{
namespace rendf
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x5040b8: `count` 16-bit pixels at `source` from 5:6:5 to 1:5:5:5 at `destination`;
// eax past the source.
reg32 row565To1555(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x504133: the same from 1:5:5:5 to 5:6:5.
reg32 row1555To565(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x5041ae: `count` 8-bit pixels to 32 bits through the palette at [0x568390].
reg32 rowPal8To32(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x5041d0: `count` 8-bit pixels to 16 bits through the palette at 0xa0ac14.
reg32 rowPal8To16(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x50425d: `count` palette entries of three bytes at `entries` to ARGB words.
reg32 rowRgb24ToArgb(App* app, CPU& cpu, reg32 entries, reg32 destination, reg32 count);
// The game's 0x5042c4: `count` 16-bit pixels with red and blue changed over.
reg32 rowSwapRedBlue(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x504335: `count` 4-bit pixels to 16 bits, each byte's high nibble first.
reg32 rowPal4To16(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count);
// The game's 0x5089d0: the display mode width x height x depth, through the screen's functions.
reg32 setDisplayMode(App* app, CPU& cpu, reg32 width, reg32 height, reg32 depth);
// The game's 0x518f80 (the font table's +0x14): `bytes` of the font's heap.
reg32 fontAlloc(App* app, CPU& cpu, reg32 bytes);
// The game's 0x518fc0 (the font table's +0x18): the block given back, a jmp to the game's free.
reg32 fontFree(App* app, CPU& cpu, reg32 block);
// The game's 0x525b80: two words of a font's record, `second` at +0x4c and `first` at +0x50.
reg32 fontSetOrigin(App* app, CPU& cpu, reg32 font, reg32 second, reg32 first);

}  // namespace rendf
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
