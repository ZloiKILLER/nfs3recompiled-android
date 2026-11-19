#ifndef NFS3HP_NATIVE_FRONTEND_WATB_H
#define NFS3HP_NATIVE_FRONTEND_WATB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_watb.cpp's typed functions (EA's graphics library between the C runtime's modules:
 * shapes, their palettes and converters, the windows' row tables, the font's blitters, the queues;
 * and the runtime's streams, argv and __LDcvt), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.
 *
 * Each takes its arguments as parameters, in the order of the registers the game's function takes
 * them in, its stack words after them, and hands back its result; what the game's code leaves in the
 * registers it does not save is not kept.  `ts` only for their frames on the guest's stack and the
 * calls they make; they throw fe::Terminated when the game stopped in one, as a call by address does. */
namespace nfs3hp
{
namespace frontend
{
namespace watb
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4fb820: qfetch's test: whether the element's key at +4 is edx.
u32 keyMatches(App* app, ThreadState& ts, u32 element, u32 key);
// The game's 0x4fb82b: the end of sub_4fb820 that sub_4fb850 jumps to: 0, no match.
u32 noMatch(App* app, ThreadState& ts);
// The game's 0x4fb830: an element's sort key: its byte at +0x11 over the upper bits of its dword at +4.
u32 sortKey(App* app, ThreadState& ts, u32 element);
// The game's 0x4feffc: a hook of the library's that answers 0.
u32 returnZero(App* app, ThreadState& ts);
// The game's 0x4ff570: printf's and scanf's floats, [__EFG_printf] and [__EFG_scanf].
u32 initFloatFormatters(App* app, ThreadState& ts);
// The game's 0x4ff5dc: the runtime's x87 started: fninit, and eax 0 (no x87 noted, as on the port there is none).
u32 init8087(App* app, ThreadState& ts);
// The game's 0x4fd1c0: the palette among the records that follow a shape, or 0.
u32 shapeClut(App* app, ThreadState& ts, u32 shape);
// The game's 0x4fd540: a filled rectangle of `colour`, drawn by movdfl as a shape of the window's type with no pixels.
u32 drawFilledRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 colour);
// The game's 0x503bc0: the pixel at `x`, `y` of the current window; 0 outside it.  edx left the pixel's place.
u32 readPixel(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x508b80: a window's record of 0x38 bytes and its two row tables.
u32 initVideoWindow(App* app, ThreadState& ts,
                      u32 window, u32 width, u32 height, u32 stride, u32 depth, u32 pixels, u32 tag);
// The game's 0x5098d0: the shape type of a window of this many bits a pixel, or 0.
u32 shapeTypeForDepth(App* app, ThreadState& ts, u32 depth);
// The game's 0x509940: the space made if it is not there, one free run in it.
u32 initRowTable(App* app, ThreadState& ts);
// The game's 0x509a00: the space given back.
u32 freeRowTable(App* app, ThreadState& ts);
// The game's 0x509a30: a table of `count` offsets from `offset`, `step` apart, each shifted right by `shift`.
u32 makeRow(App* app, ThreadState& ts, u32 offset, u32 step, u32 shift, u32 count);
// The game's 0x509b10: a use of a row table dropped; the last, its run joined to free neighbours.
u32 removeRow(App* app, ThreadState& ts, u32 table);
// The game's 0x509c00: the memory manager's block that `address` lies in, by its 16 heaps: its pixels, or 0.
u32 findBlockHeader(App* app, ThreadState& ts, u32 address, u32 data);
// The game's 0x50a1c0: a span of 24-bit pixels copied (memmove's ebx and edx left).
u32 copyTriples(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509c70: bytes into nibbles, the even pixel in the low one.
u32 bytesToNibbles(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509cd0: nibbles into bytes.
u32 nibblesToBytes(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509d20: 565 into palette indices: by the inverse table where there is one, else the nearest.
u32 convert565ToIndex(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509df0: 8888 into 1555, nothing under a quarter of alpha, the top bit set on the rest.
u32 convert8888To1555Keyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509e50: the same with every pixel but 0 opaque.
u32 convert8888To1555(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509ef0: 8888 into 565, nothing under a quarter of alpha, and what would come out 0 as 1.
u32 convert8888To565Keyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509f50: 8888 into 565.
u32 convert8888To565(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509fb0: 8888 into 4444.
u32 convert8888To4444(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a020: 1555 into 4444, opaque, the key colour nothing.
u32 convert1555To4444(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a0a0: 8888's colour into 565 in the low half of a dword, the alpha kept.
u32 convert8888To8565(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a100: palette indices into 888, by the palette at 0x568390.
u32 indexTo888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a150: 565 into 888.
u32 convert565To888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a1e0: 8888 into 888.
u32 convert8888To888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a240: 1555 into 8888, opaque, the key colour nothing.
u32 convert1555To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a2b0: 565 into 8888, opaque, the key colour nothing.
u32 convert565To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a330: 4444 into 8888, each nibble doubled.
u32 convert4444To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a3c0: a dword with a 1555 colour low and the alpha on top, into 8888.
u32 convert8555To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a440: a dword with a 565 colour low and the alpha on top, into 8888.
u32 convert8565To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a4c0: 888 into 8888, opaque.
u32 convert888To8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50afc0: palette indices into 16 bits by the table at [0x9f09b0], index 0xff no pixel.
u32 spanTo16Keyed(App* app, ThreadState& ts, u32 from, u32 to, u32 last);
// The game's 0x50b080: a span through two converters, by a buffer in the frame.
u32 spanTwoSteps(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a570: a pixel format's index in the converters' tables, by its shape type; -1 for none.
u32 depthIndex(App* app, ThreadState& ts, u32 type);
// The game's 0x50a5d0: a shape's palette as 8888, made at `out` from whatever its record holds.
u32 clutPointer(App* app, ThreadState& ts, u32 shape, u32 out);
// The game's 0x50a830: the same with the colours in 565, the alpha kept.
u32 clut16aPointer(App* app, ThreadState& ts, u32 shape, u32 out);
// The game's 0x50a960: the converter from a shape's type straight to `target`, or 0 where there is none.
u32 converterFor(App* app, ThreadState& ts, u32 shape, u32 target, u32 keyed);
// The game's 0x50aad0: a new shape of the type `target`, from the shape at `shape`.
u32 convertShape(App* app, ThreadState& ts, u32 shape, u32 target, u32 keyed, u32 mode);
// The game's 0x50b0b0: a shape drawn at `x`, `y` through the blitters at `blitters`, clipped (movdfl).
u32 moveShape(App* app, ThreadState& ts, u32 shape, u32 x, u32 y, u32 blitters);
// The game's 0x50b540: a string's next byte: `pointerAt` the place of the pointer.
u32 nextByte(App* app, ThreadState& ts, u32 pointerAt);
// The game's 0x50b560: a character's glyph number by the font's table, for ' ' to 0x7f.
u32 charWidthEntry(App* app, ThreadState& ts, u32 character);
// The game's 0x50b580: a string's next character: two bytes with the top bit of the first set, else one.
u32 nextChar(App* app, ThreadState& ts, u32 pointerAt);
// The game's 0x50b5c0: a line of a 4-bit font in alpha.
u32 textAlpha(App* app, ThreadState& ts,
                u32 x, u32 y, u32 font, u32 xOffset, u32 firstRow, u32 width, u32 height, u32 stride);
// The game's 0x50ba90: a line of a 4-bit font plotted by sub_523b00 from the font's own colours.
u32 textPlotted(App* app, ThreadState& ts,
                  u32 x, u32 y, u32 font, u32 xOffset, u32 firstRow, u32 width, u32 height, u32 stride);
// The game's 0x50bb80: a line of a 1-bit font, a byte of its mask at a time.
u32 textMono(App* app, ThreadState& ts,
               u32 x, u32 y, u32 font, u32 bitOffset, u32 firstRow, u32 width, u32 height, u32 stride);
// The game's 0x50bdf0: for each palette index and each of 16 ARGBs, the index nearest the ARGB over it.
u32 blendTable(App* app, ThreadState& ts, u32 table, u32 palette);
// The game's 0x50bfd0: the font's 16 ARGBs, a colour faded in over alpha; the blend table for 8 bits.
u32 fadeTable(App* app, ThreadState& ts, u32 colour);
// The game's 0x50c0d0: the font's tables made and filled, its blitter chosen.
u32 initTextDraw(App* app, ThreadState& ts);
// The game's 0x50c180: the font's colours for its kind: copied where they are given, faded where not.
u32 textPalette(App* app, ThreadState& ts);
// The game's 0x50c1e0: the font's tables given back.
u32 freeTextDraw(App* app, ThreadState& ts);
// The game's 0x50c220: the set bits of `bits`' eight pixels at `to` in `colour`, 8 bits: the next eight's place.
u32 plotMask8(App* app, ThreadState& ts, u32 bits, u32 colour, u32 to);
// The game's 0x50c270: the same at 16 bits.
u32 plotMask16(App* app, ThreadState& ts, u32 bits, u32 colour, u32 to);
// The game's 0x50c2c0: the same at 24 bits.
u32 plotMask24(App* app, ThreadState& ts, u32 bits, u32 colour, u32 to);
// The game's 0x50c350: the same at 32 bits.
u32 plotMask32(App* app, ThreadState& ts, u32 bits, u32 colour, u32 to);
// The game's 0x50c3a0: a colour of the current window's depth as an ARGB.
u32 colourToArgb(App* app, ThreadState& ts, u32 colour);
// The game's 0x50c584: a key's handler called, at most one at a time, by the code in al or, al 0, the scan code in ah.
u32 dispatchKey(App* app, ThreadState& ts, u32 code);
// The game's 0x50c5d4: a handler given a number, a free one or its own, and the key `key` set to it.
u32 registerKey(App* app, ThreadState& ts, u32 key, u32 handler);
// The game's 0x50d6e0: ARGBs over palette indices, each result the nearest index.
u32 blendSpanIndexed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50d7e0: ARGBs over 1555 pixels.
u32 blendSpan1555(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50d880: ARGBs over 565 pixels.
u32 blendSpan565(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50d910: ARGBs over 888 pixels.
u32 blendSpan888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50d9c0: ARGBs over ARGBs; over nothing, the ARGB as it is.
u32 blendSpan8888(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50daa0: palette indices over 565 pixels, by a palette of the alpha over 565.
u32 blendSpanPalette565(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50db30: a shape in alpha by movdfl with the table at 0x5688a8.
u32 drawAlphaShape(App* app, ThreadState& ts, u32 shape, u32 x, u32 y);
// The game's 0x50e5a0: under the queue's lock, the first element `test` says yes to, taken off the queue (qfetch).
u32 queueFetch(App* app, ThreadState& ts, u32 queue, u32 test, u32 argument);
// The game's 0x509350: the closed streams' links freed.
u32 freeClosedStreams(App* app, ThreadState& ts);
// The game's 0x515740: a link for each FILE of __iob in use, stderr's flags first.
u32 initStreams(App* app, ThreadState& ts, u32 previous);
// The game's 0x5157f4: the open streams from the `first`th of __iob closed, on shut down.
u32 closeStreamsFrom(App* app, ThreadState& ts, u32 first);
// The game's 0x515900: the command line at [0x5677fc] split into argv: counted, copied, split again.
u32 buildArgv(App* app, ThreadState& ts);
// The game's 0x5159e4: the command line at `text` split into words, their starts at `words` if not 0: how many.
u32 splitCommandLine(App* app, ThreadState& ts, u32 text, u32 words, u32 endPlace);
// The game's 0x516bf5: %f's layout of `count` digits at `digits` in `buffer`, the point after `beforePoint` + 1.
u32 formatF(App* app, ThreadState& ts, u32 cvt, u32 digits, u32 count, u32 beforePoint, u32 buffer);
// The game's 0x516e13: %e's layout: digits, zeros, the point, more, the letter, the exponent's sign and digits.
u32 formatE(App* app, ThreadState& ts, u32 cvt, u32 digits, u32 count, u32 exponent, u32 buffer);

/* Where the game's code jumps to another function, which returns for it (memmove,
 * __allocPOSIXHandle's), these call it and their thunks pop the return address; when the game
 * stopped in it they return at once with ts.terminate set, and do not throw.  The rest are the
 * game's functions with no frame of their own: ends of others, hooks, __LDcvt's digits. */
// The game's 0x4ff620: a hook that does nothing.
void nothing4ff620(App* app, ThreadState& ts);
// The game's 0x50d6d0: a hook that does nothing.
void nothing50d6d0(App* app, ThreadState& ts);
// The game's 0x50dbf0: a hook that does nothing.
void nothing50dbf0(App* app, ThreadState& ts);
// The game's 0x4ff0f8: __allocPOSIXHandle's jump (0x51e304): `handle` (eax) on to it, its answer.
u32 allocHandleJump(App* app, ThreadState& ts, u32 handle);
// The game's 0x4ff100: __freePOSIXHandle's jump (0x51e45c): `handle` (eax) on to it, its answer.
u32 freeHandleJump(App* app, ThreadState& ts, u32 handle);
// The game's 0x509c50: a span of `count` 4-bit pixels from `from` to `to`, by memmove: no result, 0.
u32 copyNibbles(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509c60: the same of 8-bit pixels.
u32 copyBytes(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x509eb0: the same of 16-bit pixels.
u32 copyWords(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a510: the same of 32-bit pixels.
u32 copyDwords(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x50a543: sub_50a570's end for a 16-bit type: 3.
u32 depthIndex16(App* app, ThreadState& ts);
// The game's 0x50a550: its end for a 32-bit type: 5.
u32 depthIndex32(App* app, ThreadState& ts);
// The game's 0x50a559: its end for a 15-bit type: 2.
u32 depthIndex15(App* app, ThreadState& ts);
// The game's 0x5157e0: __full_io_exit: every stream closed, then sub_509350, which the code jumps to:
// what it leaves in eax.
u32 closeStreams(App* app, ThreadState& ts);
// The game's 0x5167b3: __LDcvt: the long double at `value` into the CVT at `cvt`, its digits laid out
// in `buffer` for %e, %f or %g.
u32 longDoubleToDigits(App* app, ThreadState& ts, u32 value, u32 cvt, u32 buffer);
// The game's 0x516a87: the last eight decimal digits of `value` at `at` (its code leaves ebx past
// them: `at` + 8); eax as the code leaves it.
u32 eightDigits(App* app, ThreadState& ts, u32 value, u32 at);
// The game's 0x516a9d: the last four, likewise (`at` + 4).
u32 fourDigits(App* app, ThreadState& ts, u32 value, u32 at);
// The game's 0x516ab4: the last two, of ax, likewise (`at` + 2).
u32 twoDigits(App* app, ThreadState& ts, u32 value, u32 at);

}  // namespace watb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
