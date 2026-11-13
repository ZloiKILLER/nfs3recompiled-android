#ifndef NFS3HP_NATIVE_FRONTEND_EAA_H
#define NFS3HP_NATIVE_FRONTEND_EAA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_eaa.cpp's typed functions (EA's libraries' memory viewer, the text's fonts, udff,
 * a file's size, the bitmap drawn into and its TGA, DirectDraw's display), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace eaa
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x497620: the viewer on the console of functions `table`, until Escape.
reg32 memViewerRun(App* app, CPU& cpu, reg32 table);
// The game's 0x497ba0: the viewer's state, and the screen's size, afresh.
reg32 memViewerOpen(App* app, CPU& cpu);
// The game's 0x497cb0: nothing, at the viewer's end: eax as it was.
reg32 memViewerClose(App* app, CPU& cpu);
// The game's 0x497cc0: the whole screen, first: the numbers, the list, the header, its rows.
reg32 memViewerFirstDraw(App* app, CPU& cpu, reg32 blocks);
// The game's 0x497d20: the numbers at the top (the pool's total, used, free, largest free), as many lines as they
// take.
reg32 memViewerStatsLine(App* app, CPU& cpu, reg32 total, reg32 used, reg32 available, reg32 largest);
// The game's 0x497fe0: `blocks` of the pool into the table, without the free ones in pools of odd number: how many
// it holds.
reg32 memViewerListBlocks(App* app, CPU& cpu, reg32 blocks);
// The game's 0x498090: the columns' header, and the list's size under it.
reg32 memViewerHeader(App* app, CPU& cpu);
// The game's 0x498370: the list's rows: the names' column, then a column at a time; only the rows that changed,
// unless all are wanted (0x79f5b4).
reg32 memViewerRows(App* app, CPU& cpu);
// The game's 0x4989a0: the line at the bottom, of the block `index`.
reg32 memViewerStatusLine(App* app, CPU& cpu, reg32 index);
// The game's 0x498a40: what the status line says of the block `index` of kind `kind` (sub_4994a0), to `text`.
reg32 memViewerDescribeBlock(App* app, CPU& cpu, reg32 index, reg32 kind, reg32 text);
// The game's 0x498b70: the entries of a pack counted by what they are: how many to [countAt], no pictures to
// [others], packed ones to [packed], the rest to [unpacked].
reg32 memViewerCountShapes(App* app, CPU& cpu, reg32 pack, reg32 countAt, reg32 others, reg32 packed, reg32 unpacked);
// The game's 0x498bf0: the menu along the bottom of what to do with the block, by arrows and Enter.
reg32 memViewerMenu(App* app, CPU& cpu);
// The game's 0x498e10: the next pool that exists; eax as it was.
reg32 memViewerNextClass(App* app, CPU& cpu);
// The game's 0x498e40: a block's flags as letters at `text`.
reg32 memViewerFlagText(App* app, CPU& cpu, reg32 flags, reg32 text);
// The game's 0x498f00: choosing the column to sort by: the arrows move along the header, Up sorts down it and Down
// up it, Escape leaves.
reg32 memViewerSortMode(App* app, CPU& cpu, reg32 blocks);
// The game's 0x499150: qsort's comparison of the block table's entries `a` and `b`, by the sort column (esi and ebx
// compared as they are for columns 5 and 6).
reg32 memViewerCompare(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x499330: the pool's total, used, free and largest free bytes to what the four point at.
reg32 memPoolTotals(App* app, CPU& cpu, reg32 totalAt, reg32 usedAt, reg32 freeAt, reg32 largestAt);
// The game's 0x4993b0: the first block of the pool shown.
reg32 memPoolFirstBlock(App* app, CPU& cpu);
// The game's 0x4993d0: how many blocks of the two kinds the pool has.
reg32 memPoolCountBlocks(App* app, CPU& cpu);
// The game's 0x499400: the next block, or the first if the list starts again: it, its size and flags to what the
// first three point at, 0 to [zeroAt]; its size.
reg32 memPoolNextBlock(App* app, CPU& cpu, reg32 blockAt, reg32 sizeAt, reg32 flagsAt, reg32 zeroAt);
// The game's 0x499460: a block's size, the selected one's for none.
reg32 viewerBlockSize(App* app, CPU& cpu, reg32 block);
// The game's 0x499480: a block's data.
reg32 memBlockData(App* app, CPU& cpu, reg32 block);
// The game's 0x499490: a block's flags.
reg32 viewerBlockFlags(App* app, CPU& cpu, reg32 block);
// The game's 0x4994a0: what the listed block `index` is: 0 free, 1 the system's, 2 unknown, 3 a pack of shapes, 4
// or 5 unknown high or low.
reg32 memBlockKind(App* app, CPU& cpu, reg32 index);
// The game's 0x499580: 1 if a block is free space.
reg32 memBlockIsFree(App* app, CPU& cpu, reg32 block);
// The game's 0x4995b0: VIEW: the shapes of a pack one at a time, Left and Right, until Escape; or a bitmap, or a
// window.
reg32 memViewerViewShapes(App* app, CPU& cpu);
// The game's 0x499b30: VIEW of a window: its bitmap and the numbers of it, or why not.
reg32 memViewerViewWindow(App* app, CPU& cpu);
// The game's 0x499d30: VIEW of a bitmap, if the block is the size of one: its bits a pixel back, 0 if it has none.
reg32 memViewerViewBitmap(App* app, CPU& cpu);
// The game's 0x499e90: a shape's bits a pixel.
reg32 memShapeBits(App* app, CPU& cpu, reg32 shape);
// The game's 0x499ea0: HEX DUMP of the block, by arrows and pages, until any other key.
reg32 memViewerHexDump(App* app, CPU& cpu);
// The game's 0x499f70: a screenful of the dump from `base`, `perRow` bytes a row.
reg32 memViewerHexPage(App* app, CPU& cpu, reg32 base, reg32 perRow);
// The game's 0x49a120: P: the pool's numbers and blocks into a file of a free name.
reg32 memViewerWriteMap(App* app, CPU& cpu);
// The game's 0x49a400: the file's name free: its ?? become the first two digits no file has, or xx if there is
// none.
reg32 memViewerFreeFileName(App* app, CPU& cpu, reg32 name);
// The game's 0x4d0d50: the text's fonts: their files by the screen's width, the widths and places of their glyphs
// into the tables at 0x83f4e0 and on, and each made a texture.
reg32 textLoadFonts(App* app, CPU& cpu);
// The game's 0x4dc0e0: a udff on the file `name` or the memory at `memory`, one and not both, `complain` whether
// what goes wrong is an error: its record of 12 bytes, or 0.
reg32 udffOpen(App* app, CPU& cpu, reg32 name, reg32 memory, reg32 complain);
// The game's 0x4dc1b0: a udff closed: its file, if it has one, and its record.
reg32 udffClose(App* app, CPU& cpu, reg32 record);
// The game's 0x4dc1e0: the next word of a udff.
reg32 udffReadWord(App* app, CPU& cpu, reg32 record);
// The game's 0x4e4360: the size of the file `name`, opened as `mode` says.
reg32 fileSizeOpened(App* app, CPU& cpu, reg32 name, reg32 mode);
// The game's 0x4e43a0: the size of the file `name`.
reg32 fileSize(App* app, CPU& cpu, reg32 name);
// The game's 0x4ead90: drawing goes to the screen.
reg32 bitmapDrawToScreen(App* app, CPU& cpu);
// The game's 0x4eada0: drawing goes into a bitmap.
reg32 bitmapDrawInto(App* app, CPU& cpu, reg32 bitmap);
// The game's 0x4eaeb0: `width` by `rows` pixels from `left`, `y` of what is drawn into, read at their width to
// `out`.
reg32 bitmapReadPixels(App* app, CPU& cpu, reg32 left, reg32 y, reg32 width, reg32 rows, reg32 out);
// The game's 0x4eaf60: the size of a TGA of what is drawn into.
reg32 bitmapTgaSize(App* app, CPU& cpu);
// The game's 0x4eafc0: a TGA of what is drawn into, at `tga`: its header, its palette, its pixels.
reg32 bitmapWriteTga(App* app, CPU& cpu, reg32 tga);
// The game's 0x4eb160: what is drawn into saved as a TGA to the file `name`: the save's answer, 0 without the
// memory for it.
reg32 bitmapSaveTga(App* app, CPU& cpu, reg32 name);
// The game's 0x4eb1c0: a key's state, by its number.
reg32 keyStateByte(App* app, CPU& cpu, reg32 key);
// The game's 0x4eb430: DirectDrawCreate for the driver `driver`: the object, and how many have been tried, told to
// the log.
reg32 ddrawCreate(App* app, CPU& cpu, reg32 driver);
// The game's 0x4eb490: the primary surface of `display`, as `wanted` asks, else whatever will do.
reg32 ddrawMakePrimary(App* app, CPU& cpu, reg32 display, reg32 wanted);
// The game's 0x4eb5e0: `surface` of `display` locked into `description`, restored when lost: the error.
reg32 ddrawLockSurface(App* app, CPU& cpu, reg32 display, reg32 surface, reg32 description);
// The game's 0x4eb880: the bits set in `value`.
reg32 countBits(App* app, CPU& cpu, reg32 value);
// The game's 0x4eb8a0: a surface's `description` into the display's list of modes at `index`, if there is one; with
// `current` it is the mode now, too.
reg32 ddrawNoteMode(App* app, CPU& cpu, reg32 display, reg32 index, reg32 description, reg32 current);
// The game's 0x4ebaf0: DirectDraw, the driver chosen or the first or the emulation, its second interface, and what
// its capabilities mean to the library.
reg32 ddrawInit(App* app, CPU& cpu, reg32 given);
// The game's 0x4ebe50: the display taken: its cooperative level, the list of its modes, a clipper if it is in a
// window.
reg32 ddrawAccessDisplay(App* app, CPU& cpu, reg32 given);
// The game's 0x4ec0d0: a display's surfaces and their windows gone.
reg32 ddrawRemoveSurfaces(App* app, CPU& cpu, reg32 display);
// The game's 0x4ec1a0: the display configured as its record says: what was made gone, the palette, the primary
// surface, the back buffer and the windows on them.
reg32 ddrawConfigDisplay(App* app, CPU& cpu, reg32 given);

}  // namespace eaa
}  // namespace frontend

namespace guest
{

// The table in eax.
template <>
struct ByAddress<frontend::eaa::memViewerRun>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 table)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ table }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The blocks in eax.
template <>
struct ByAddress<frontend::eaa::memViewerFirstDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 blocks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ blocks }));
    }
};

// The total in eax, the used in edx, the available in ebx, the largest in ecx.
template <>
struct ByAddress<frontend::eaa::memViewerStatsLine>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 total, x86::reg32 used,
                           x86::reg32 available, x86::reg32 largest)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ total, used, available, largest }));
    }
};

// The blocks in eax.
template <>
struct ByAddress<frontend::eaa::memViewerListBlocks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 blocks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ blocks }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerHeader>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerRows>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The index in eax.
template <>
struct ByAddress<frontend::eaa::memViewerStatusLine>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// The index in eax, the kind in edx, the text in ebx.
template <>
struct ByAddress<frontend::eaa::memViewerDescribeBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index, x86::reg32 kind,
                           x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index, kind, text }));
    }
};

// The pack in eax, the countAt in edx, the others in ebx, the packed in ecx, the unpacked on the stack.
template <>
struct ByAddress<frontend::eaa::memViewerCountShapes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pack, x86::reg32 countAt,
                           x86::reg32 others, x86::reg32 packed, x86::reg32 unpacked)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pack, countAt, others, packed }, { unpacked }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerMenu>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerNextClass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The flags in eax, the text in edx.
template <>
struct ByAddress<frontend::eaa::memViewerFlagText>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 flags, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ flags, text }));
    }
};

// The blocks in eax.
template <>
struct ByAddress<frontend::eaa::memViewerSortMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 blocks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ blocks }));
    }
};

// The a in eax, the b in edx.
template <>
struct ByAddress<frontend::eaa::memViewerCompare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// The totalAt in eax, the usedAt in edx, the freeAt in ebx, the largestAt in ecx.
template <>
struct ByAddress<frontend::eaa::memPoolTotals>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 totalAt, x86::reg32 usedAt,
                           x86::reg32 freeAt, x86::reg32 largestAt)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ totalAt, usedAt, freeAt, largestAt }));
    }
};

template <>
struct ByAddress<frontend::eaa::memPoolFirstBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memPoolCountBlocks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The blockAt in edx, the sizeAt in ebx, the flagsAt in ecx, the zeroAt on the stack.
template <>
struct ByAddress<frontend::eaa::memPoolNextBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 blockAt, x86::reg32 sizeAt,
                           x86::reg32 flagsAt, x86::reg32 zeroAt)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, blockAt, sizeAt, flagsAt }, { zeroAt }));
    }
};

// The block in eax.
template <>
struct ByAddress<frontend::eaa::viewerBlockSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

// The block in eax.
template <>
struct ByAddress<frontend::eaa::memBlockData>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

// The block in eax.
template <>
struct ByAddress<frontend::eaa::viewerBlockFlags>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

// The index in eax.
template <>
struct ByAddress<frontend::eaa::memBlockKind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// The block in eax.
template <>
struct ByAddress<frontend::eaa::memBlockIsFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerViewShapes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerViewWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerViewBitmap>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The shape in eax.
template <>
struct ByAddress<frontend::eaa::memShapeBits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerHexDump>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The base in eax, the perRow in edx.
template <>
struct ByAddress<frontend::eaa::memViewerHexPage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 base, x86::reg32 perRow)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ base, perRow }));
    }
};

template <>
struct ByAddress<frontend::eaa::memViewerWriteMap>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The name in eax.
template <>
struct ByAddress<frontend::eaa::memViewerFreeFileName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

template <>
struct ByAddress<frontend::eaa::textLoadFonts>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The name in eax, the memory in edx, the complain in ebx.
template <>
struct ByAddress<frontend::eaa::udffOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 memory,
                           x86::reg32 complain)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, memory, complain }));
    }
};

// The record in eax.
template <>
struct ByAddress<frontend::eaa::udffClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ record }));
    }
};

// The record in eax.
template <>
struct ByAddress<frontend::eaa::udffReadWord>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ record }));
    }
};

// The name in eax, the mode in edx.
template <>
struct ByAddress<frontend::eaa::fileSizeOpened>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 mode)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, mode }));
    }
};

// The name in eax.
template <>
struct ByAddress<frontend::eaa::fileSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

template <>
struct ByAddress<frontend::eaa::bitmapDrawToScreen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The bitmap in eax.
template <>
struct ByAddress<frontend::eaa::bitmapDrawInto>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bitmap)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ bitmap }));
    }
};

// The left in eax, the y in edx, the width in ebx, the rows in ecx, the out on the stack.
template <>
struct ByAddress<frontend::eaa::bitmapReadPixels>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 left, x86::reg32 y, x86::reg32 width,
                           x86::reg32 rows, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ left, y, width, rows }, { out }));
    }
};

template <>
struct ByAddress<frontend::eaa::bitmapTgaSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The tga in eax.
template <>
struct ByAddress<frontend::eaa::bitmapWriteTga>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 tga)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ tga }));
    }
};

// The name in eax.
template <>
struct ByAddress<frontend::eaa::bitmapSaveTga>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

// The key in eax.
template <>
struct ByAddress<frontend::eaa::keyStateByte>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key }));
    }
};

// The driver in edx.
template <>
struct ByAddress<frontend::eaa::ddrawCreate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 driver)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, driver }));
    }
};

// The display in eax, the wanted in edx.
template <>
struct ByAddress<frontend::eaa::ddrawMakePrimary>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display, wanted }));
    }
};

// The display in eax, the surface in edx, the description in ebx.
template <>
struct ByAddress<frontend::eaa::ddrawLockSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display, x86::reg32 surface,
                           x86::reg32 description)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display, surface, description }));
    }
};

// The value in eax.
template <>
struct ByAddress<frontend::eaa::countBits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ value }));
    }
};

// The display in eax, the index in edx, the description in ebx, the current in ecx.
template <>
struct ByAddress<frontend::eaa::ddrawNoteMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display, x86::reg32 index,
                           x86::reg32 description, x86::reg32 current)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display, index, description, current }));
    }
};

// The given in eax.
template <>
struct ByAddress<frontend::eaa::ddrawInit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 given)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ given }));
    }
};

// The given in eax.
template <>
struct ByAddress<frontend::eaa::ddrawAccessDisplay>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 given)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ given }));
    }
};

// The display in eax.
template <>
struct ByAddress<frontend::eaa::ddrawRemoveSurfaces>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display }));
    }
};

// The given in eax.
template <>
struct ByAddress<frontend::eaa::ddrawConfigDisplay>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 given)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ given }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
