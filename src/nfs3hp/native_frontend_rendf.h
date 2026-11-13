#ifndef NFS3HP_NATIVE_FRONTEND_RENDF_H
#define NFS3HP_NATIVE_FRONTEND_RENDF_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendf.cpp's typed functions (a row of a shape's pixels from one format to
 * another, the display mode, the font's allocator), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's
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

// The source in eax, the destination in edx, the count in ebx; ebx and edx left as the row leaves them.
template <>
struct ByAddress<frontend::rendf::row565To1555>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::row1555To565>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::rowPal8To32>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::rowPal8To16>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::rowRgb24ToArgb>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 entries, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ entries, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::rowSwapRedBlue>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

template <>
struct ByAddress<frontend::rendf::rowPal4To16>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 source, x86::reg32 destination,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ source, destination, count }));
    }
};

// The width in eax, the height in edx, the depth in ebx.
template <>
struct ByAddress<frontend::rendf::setDisplayMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height,
                           x86::reg32 depth)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ width, height, depth }));
    }
};

// The bytes in eax.
template <>
struct ByAddress<frontend::rendf::fontAlloc>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bytes)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ bytes }));
    }
};

// The block in eax.
template <>
struct ByAddress<frontend::rendf::fontFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

// The font in eax, the second word in edx, the first in ebx.
template <>
struct ByAddress<frontend::rendf::fontSetOrigin>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 font, x86::reg32 second,
                           x86::reg32 first)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ font, second, first }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
