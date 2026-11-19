#ifndef NFS3HP_NATIVE_FRONTEND_EAA_H
#define NFS3HP_NATIVE_FRONTEND_EAA_H

#include "native_frontend.h"
#include "native_guest.h"
#include "os_video.h"

/* native_frontend_eaa.cpp's typed functions (EA's libraries' memory viewer, the text's fonts, udff,
 * a file's size, the bitmap drawn into and its TGA, DirectDraw's display), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace eaa
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x497620: the viewer on the console of functions `table`, until Escape.
u32 memViewerRun(App* app, ThreadState& ts, u32 table);
// The game's 0x497ba0: the viewer's state, and the screen's size, afresh.
u32 memViewerOpen(App* app, ThreadState& ts);
// The game's 0x497cb0: nothing, at the viewer's end: eax as it was.
void memViewerClose(App* app, ThreadState& ts);
// The game's 0x497cc0: the whole screen, first: the numbers, the list, the header, its rows.
u32 memViewerFirstDraw(App* app, ThreadState& ts, u32 blocks);
// The game's 0x497d20: the numbers at the top (the pool's total, used, free, largest free), as many lines as they
// take.
u32 memViewerStatsLine(App* app, ThreadState& ts, u32 total, u32 used, u32 available, u32 largest);
// The game's 0x497fe0: `blocks` of the pool into the table, without the free ones in pools of odd number: how many
// it holds.
u32 memViewerListBlocks(App* app, ThreadState& ts, u32 blocks);
// The game's 0x498090: the columns' header, and the list's size under it.
u32 memViewerHeader(App* app, ThreadState& ts);
// The game's 0x498370: the list's rows: the names' column, then a column at a time; only the rows that changed,
// unless all are wanted (0x79f5b4).
u32 memViewerRows(App* app, ThreadState& ts);
// The game's 0x4989a0: the line at the bottom, of the block `index`.
u32 memViewerStatusLine(App* app, ThreadState& ts, u32 index);
// The game's 0x498a40: what the status line says of the block `index` of kind `kind` (sub_4994a0), to `text`.
u32 memViewerDescribeBlock(App* app, ThreadState& ts, u32 index, u32 kind, u32 text);
// The game's 0x498b70: the entries of a pack counted by what they are: how many to [countAt], no pictures to
// [others], packed ones to [packed], the rest to [unpacked].
u32 memViewerCountShapes(App* app, ThreadState& ts, u32 pack, u32 countAt, u32 others, u32 packed, u32 unpacked);
// The game's 0x498bf0: the menu along the bottom of what to do with the block, by arrows and Enter.
u32 memViewerMenu(App* app, ThreadState& ts);
// The game's 0x498e10: the next pool that exists; eax as it was.
void memViewerNextClass(App* app, ThreadState& ts);
// The game's 0x498e40: a block's flags as letters at `text`.
u32 memViewerFlagText(App* app, ThreadState& ts, u32 flags, u32 text);
// The game's 0x498f00: choosing the column to sort by: the arrows move along the header, Up sorts down it and Down
// up it, Escape leaves.
u32 memViewerSortMode(App* app, ThreadState& ts, u32 blocks);
// The game's 0x499150: qsort's comparison of the block table's entries `a` and `b`, by the sort column (esi and ebx
// compared as they are for columns 5 and 6).
// esi and ebx as the caller (qsort) leaves them: what columns 5 and 6 compare.
u32 memViewerCompare(App* app, ThreadState& ts, u32 a, u32 b, u32 esi, u32 ebx);
// The game's 0x499330: the pool's total, used, free and largest free bytes to what the four point at.
u32 memPoolTotals(App* app, ThreadState& ts, u32 totalAt, u32 usedAt, u32 freeAt, u32 largestAt);
// The game's 0x4993b0: the first block of the pool shown.
u32 memPoolFirstBlock(App* app, ThreadState& ts);
// The game's 0x4993d0: how many blocks of the two kinds the pool has.
u32 memPoolCountBlocks(App* app, ThreadState& ts);
// The game's 0x499400: the next block, or the first if the list starts again: it, its size and flags to what the
// first three point at, 0 to [zeroAt]; its size.
u32 memPoolNextBlock(App* app, ThreadState& ts, u32 blockAt, u32 sizeAt, u32 flagsAt, u32 zeroAt);
// The game's 0x499460: a block's size, the selected one's for none.
u32 viewerBlockSize(App* app, ThreadState& ts, u32 block);
// The game's 0x499480: a block's data.
u32 memBlockData(App* app, ThreadState& ts, u32 block);
// The game's 0x499490: a block's flags.
u32 viewerBlockFlags(App* app, ThreadState& ts, u32 block);
// The game's 0x4994a0: what the listed block `index` is: 0 free, 1 the system's, 2 unknown, 3 a pack of shapes, 4
// or 5 unknown high or low.
u32 memBlockKind(App* app, ThreadState& ts, u32 index);
// The game's 0x499580: 1 if a block is free space.
u32 memBlockIsFree(App* app, ThreadState& ts, u32 block);
// The game's 0x4995b0: VIEW: the shapes of a pack one at a time, Left and Right, until Escape; or a bitmap, or a
// window.
u32 memViewerViewShapes(App* app, ThreadState& ts);
// The game's 0x499b30: VIEW of a window: its bitmap and the numbers of it, or why not.
u32 memViewerViewWindow(App* app, ThreadState& ts);
// The game's 0x499d30: VIEW of a bitmap, if the block is the size of one: its bits a pixel back, 0 if it has none.
u32 memViewerViewBitmap(App* app, ThreadState& ts);
// The game's 0x499e90: a shape's bits a pixel.
u32 memShapeBits(App* app, ThreadState& ts, u32 shape);
// The game's 0x499ea0: HEX DUMP of the block, by arrows and pages, until any other key.
u32 memViewerHexDump(App* app, ThreadState& ts);
// The game's 0x499f70: a screenful of the dump from `base`, `perRow` bytes a row.
u32 memViewerHexPage(App* app, ThreadState& ts, u32 base, u32 perRow);
// The game's 0x49a120: P: the pool's numbers and blocks into a file of a free name.
u32 memViewerWriteMap(App* app, ThreadState& ts);
// The game's 0x49a400: the file's name free: its ?? become the first two digits no file has, or xx if there is
// none.
u32 memViewerFreeFileName(App* app, ThreadState& ts, u32 name);
// The game's 0x4d0d50: the text's fonts: their files by the screen's width, the widths and places of their glyphs
// into the tables at 0x83f4e0 and on, and each made a texture.
u32 textLoadFonts(App* app, ThreadState& ts);
// The game's 0x4dc0e0: a udff on the file `name` or the memory at `memory`, one and not both, `complain` whether
// what goes wrong is an error: its record of 12 bytes, or 0.
u32 udffOpen(App* app, ThreadState& ts, u32 name, u32 memory, u32 complain);
// The game's 0x4dc1b0: a udff closed: its file, if it has one, and its record.
u32 udffClose(App* app, ThreadState& ts, u32 record);
// The game's 0x4dc1e0: the next word of a udff.
u32 udffReadWord(App* app, ThreadState& ts, u32 record);
// The game's 0x4e4360: the size of the file `name`, opened as `mode` says.
u32 fileSizeOpened(App* app, ThreadState& ts, u32 name, u32 mode);
// The game's 0x4e43a0: the size of the file `name`.
u32 fileSize(App* app, ThreadState& ts, u32 name);
// The game's 0x4ead90: drawing goes to the screen.
u32 bitmapDrawToScreen(App* app, ThreadState& ts);
// The game's 0x4eada0: drawing goes into a bitmap.
u32 bitmapDrawInto(App* app, ThreadState& ts, u32 bitmap);
// The game's 0x4eaeb0: `width` by `rows` pixels from `left`, `y` of what is drawn into, read at their width to
// `out`.
u32 bitmapReadPixels(App* app, ThreadState& ts, u32 left, u32 y, u32 width, u32 rows, u32 out);
// The game's 0x4eaf60: the size of a TGA of what is drawn into.
u32 bitmapTgaSize(App* app, ThreadState& ts);
// The game's 0x4eafc0: a TGA of what is drawn into, at `tga`: its header, its palette, its pixels.
u32 bitmapWriteTga(App* app, ThreadState& ts, u32 tga);
// The game's 0x4eb160: what is drawn into saved as a TGA to the file `name`: the save's answer, 0 without the
// memory for it.
u32 bitmapSaveTga(App* app, ThreadState& ts, u32 name);
// The game's 0x4eb1c0: a key's state, by its number.
u32 keyStateByte(App* app, ThreadState& ts, u32 key);
// The game's 0x4eb430: a display (os_video.h) for the driver `driver`: the display, and how many have been tried,
// told to the log.
u32 ddrawCreate(App* app, ThreadState& ts, u32 driver);
// The game's 0x4eb490: the primary surface of `display`, as `wanted` asks, else whatever will do.
u32 ddrawMakePrimary(App* app, ThreadState& ts, u32 display, u32 wanted);
// The game's 0x4eb5e0: `surface` of `display` locked, where its pixels are and their layout in `locked`: the error, 0.
u32 ddrawLockSurface(App* app, ThreadState& ts, u32 display, u32 surface, os::video::Locked* locked);
// The game's 0x4eb880: the bits set in `value`.
u32 countBits(App* app, ThreadState& ts, u32 value);
// The game's 0x4eb8a0: the mode `mode` into the display's list of modes at `index`, if there is one; with `current` it
// is the mode now, too.
u32 ddrawNoteMode(App* app, ThreadState& ts, u32 display, u32 index, const os::video::Format& mode, u32 current);
// The game's 0x4ebaf0: DirectDraw, the driver chosen or the first or the emulation, its second interface, and what
// its capabilities mean to the library.
u32 ddrawInit(App* app, ThreadState& ts, u32 given);
// The game's 0x4ebe50: the display taken: its cooperative level, the list of its modes, a clipper if it is in a
// window.
u32 ddrawAccessDisplay(App* app, ThreadState& ts, u32 given);
// The game's 0x4ec0d0: a display's surfaces and their windows gone.
u32 ddrawRemoveSurfaces(App* app, ThreadState& ts, u32 display);
// The game's 0x4ec1a0: the display configured as its record says: what was made gone, the palette, the primary
// surface, the back buffer and the windows on them.
u32 ddrawConfigDisplay(App* app, ThreadState& ts, u32 given);

}  // namespace eaa
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
