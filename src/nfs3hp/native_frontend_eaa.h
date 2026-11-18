#ifndef NFS3HP_NATIVE_FRONTEND_EAA_H
#define NFS3HP_NATIVE_FRONTEND_EAA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_eaa.cpp's typed functions (EA's libraries' memory viewer, the text's fonts, udff,
 * a file's size, the bitmap drawn into and its TGA, DirectDraw's display), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
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
void memViewerClose(App* app, CPU& cpu);
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
void memViewerNextClass(App* app, CPU& cpu);
// The game's 0x498e40: a block's flags as letters at `text`.
reg32 memViewerFlagText(App* app, CPU& cpu, reg32 flags, reg32 text);
// The game's 0x498f00: choosing the column to sort by: the arrows move along the header, Up sorts down it and Down
// up it, Escape leaves.
reg32 memViewerSortMode(App* app, CPU& cpu, reg32 blocks);
// The game's 0x499150: qsort's comparison of the block table's entries `a` and `b`, by the sort column (esi and ebx
// compared as they are for columns 5 and 6).
// esi and ebx as the caller (qsort) leaves them: what columns 5 and 6 compare.
reg32 memViewerCompare(App* app, CPU& cpu, reg32 a, reg32 b, reg32 esi, reg32 ebx);
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

}  // namespace guest
}  // namespace nfs3hp

#endif
