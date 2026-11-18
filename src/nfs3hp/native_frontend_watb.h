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
 * registers it does not save is not kept.  `cpu` only for their frames on the guest's stack and the
 * calls they make; they throw fe::Terminated when the game stopped in one, as a call by address does. */
namespace nfs3hp
{
namespace frontend
{
namespace watb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4fb820: qfetch's test: whether the element's key at +4 is edx.
reg32 keyMatches(App* app, CPU& cpu, reg32 element, reg32 key);
// The game's 0x4fb82b: the end of sub_4fb820 that sub_4fb850 jumps to: 0, no match.
reg32 noMatch(App* app, CPU& cpu);
// The game's 0x4fb830: an element's sort key: its byte at +0x11 over the upper bits of its dword at +4.
reg32 sortKey(App* app, CPU& cpu, reg32 element);
// The game's 0x4feffc: a hook of the library's that answers 0.
reg32 returnZero(App* app, CPU& cpu);
// The game's 0x4ff570: printf's and scanf's floats, [__EFG_printf] and [__EFG_scanf].
reg32 initFloatFormatters(App* app, CPU& cpu);
// The game's 0x4ff59c: where there is a real x87, its state's save and restore for the signal handlers.
reg32 initChipBugHooks(App* app, CPU& cpu);
// The game's 0x4ff5dc: an x87 looked for, by what fninit leaves in the control word (`previous` eax as it came).
reg32 init8087(App* app, CPU& cpu, reg32 previous);
// The game's 0x4fd1c0: the palette among the records that follow a shape, or 0.
reg32 shapeClut(App* app, CPU& cpu, reg32 shape);
// The game's 0x4fd540: a filled rectangle of `colour`, drawn by movdfl as a shape of the window's type with no pixels.
reg32 drawFilledRect(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour);
// The game's 0x503bc0: the pixel at `x`, `y` of the current window; 0 outside it.  edx left the pixel's place.
reg32 readPixel(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x508b80: a window's record of 0x38 bytes and its two row tables.
reg32 initVideoWindow(App* app, CPU& cpu,
                      reg32 window, reg32 width, reg32 height, reg32 stride, reg32 depth, reg32 pixels, reg32 tag);
// The game's 0x5098d0: the shape type of a window of this many bits a pixel, or 0.
reg32 shapeTypeForDepth(App* app, CPU& cpu, reg32 depth);
// The game's 0x509940: the space made if it is not there, one free run in it.
reg32 initRowTable(App* app, CPU& cpu);
// The game's 0x509a00: the space given back.
reg32 freeRowTable(App* app, CPU& cpu);
// The game's 0x509a30: a table of `count` offsets from `offset`, `step` apart, each shifted right by `shift`.
reg32 makeRow(App* app, CPU& cpu, reg32 offset, reg32 step, reg32 shift, reg32 count);
// The game's 0x509b10: a use of a row table dropped; the last, its run joined to free neighbours.
reg32 removeRow(App* app, CPU& cpu, reg32 table);
// The game's 0x509c00: the memory manager's block that `address` lies in, by its 16 heaps: its pixels, or 0.
reg32 findBlockHeader(App* app, CPU& cpu, reg32 address, reg32 data);
// The game's 0x50a1c0: a span of 24-bit pixels copied (memmove's ebx and edx left).
reg32 copyTriples(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509c70: bytes into nibbles, the even pixel in the low one.
reg32 bytesToNibbles(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509cd0: nibbles into bytes.
reg32 nibblesToBytes(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509d20: 565 into palette indices: by the inverse table where there is one, else the nearest.
reg32 convert565ToIndex(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509df0: 8888 into 1555, nothing under a quarter of alpha, the top bit set on the rest.
reg32 convert8888To1555Keyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509e50: the same with every pixel but 0 opaque.
reg32 convert8888To1555(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509ef0: 8888 into 565, nothing under a quarter of alpha, and what would come out 0 as 1.
reg32 convert8888To565Keyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509f50: 8888 into 565.
reg32 convert8888To565(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509fb0: 8888 into 4444.
reg32 convert8888To4444(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a020: 1555 into 4444, opaque, the key colour nothing.
reg32 convert1555To4444(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a0a0: 8888's colour into 565 in the low half of a dword, the alpha kept.
reg32 convert8888To8565(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a100: palette indices into 888, by the palette at 0x568390.
reg32 indexTo888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a150: 565 into 888.
reg32 convert565To888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a1e0: 8888 into 888.
reg32 convert8888To888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a240: 1555 into 8888, opaque, the key colour nothing.
reg32 convert1555To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a2b0: 565 into 8888, opaque, the key colour nothing.
reg32 convert565To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a330: 4444 into 8888, each nibble doubled.
reg32 convert4444To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a3c0: a dword with a 1555 colour low and the alpha on top, into 8888.
reg32 convert8555To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a440: a dword with a 565 colour low and the alpha on top, into 8888.
reg32 convert8565To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a4c0: 888 into 8888, opaque.
reg32 convert888To8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50afc0: palette indices into 16 bits by the table at [0x9f09b0], index 0xff no pixel.
reg32 spanTo16Keyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 last);
// The game's 0x50b080: a span through two converters, by a buffer in the frame.
reg32 spanTwoSteps(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a570: a pixel format's index in the converters' tables, by its shape type; -1 for none.
reg32 depthIndex(App* app, CPU& cpu, reg32 type);
// The game's 0x50a5d0: a shape's palette as 8888, made at `out` from whatever its record holds.
reg32 clutPointer(App* app, CPU& cpu, reg32 shape, reg32 out);
// The game's 0x50a830: the same with the colours in 565, the alpha kept.
reg32 clut16aPointer(App* app, CPU& cpu, reg32 shape, reg32 out);
// The game's 0x50a960: the converter from a shape's type straight to `target`, or 0 where there is none.
reg32 converterFor(App* app, CPU& cpu, reg32 shape, reg32 target, reg32 keyed);
// The game's 0x50aad0: a new shape of the type `target`, from the shape at `shape`.
reg32 convertShape(App* app, CPU& cpu, reg32 shape, reg32 target, reg32 keyed, reg32 mode);
// The game's 0x50b0b0: a shape drawn at `x`, `y` through the blitters at `blitters`, clipped (movdfl).
reg32 moveShape(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y, reg32 blitters);
// The game's 0x50b540: a string's next byte: `pointerAt` the place of the pointer.
reg32 nextByte(App* app, CPU& cpu, reg32 pointerAt);
// The game's 0x50b560: a character's glyph number by the font's table, for ' ' to 0x7f.
reg32 charWidthEntry(App* app, CPU& cpu, reg32 character);
// The game's 0x50b580: a string's next character: two bytes with the top bit of the first set, else one.
reg32 nextChar(App* app, CPU& cpu, reg32 pointerAt);
// The game's 0x50b5c0: a line of a 4-bit font in alpha.
reg32 textAlpha(App* app, CPU& cpu,
                reg32 x, reg32 y, reg32 font, reg32 xOffset, reg32 firstRow, reg32 width, reg32 height, reg32 stride);
// The game's 0x50ba90: a line of a 4-bit font plotted by sub_523b00 from the font's own colours.
reg32 textPlotted(App* app, CPU& cpu,
                  reg32 x, reg32 y, reg32 font, reg32 xOffset, reg32 firstRow, reg32 width, reg32 height, reg32 stride);
// The game's 0x50bb80: a line of a 1-bit font, a byte of its mask at a time.
reg32 textMono(App* app, CPU& cpu,
               reg32 x, reg32 y, reg32 font, reg32 bitOffset, reg32 firstRow, reg32 width, reg32 height, reg32 stride);
// The game's 0x50bdf0: for each palette index and each of 16 ARGBs, the index nearest the ARGB over it.
reg32 blendTable(App* app, CPU& cpu, reg32 table, reg32 palette);
// The game's 0x50bfd0: the font's 16 ARGBs, a colour faded in over alpha; the blend table for 8 bits.
reg32 fadeTable(App* app, CPU& cpu, reg32 colour);
// The game's 0x50c0d0: the font's tables made and filled, its blitter chosen.
reg32 initTextDraw(App* app, CPU& cpu);
// The game's 0x50c180: the font's colours for its kind: copied where they are given, faded where not.
reg32 textPalette(App* app, CPU& cpu);
// The game's 0x50c1e0: the font's tables given back.
reg32 freeTextDraw(App* app, CPU& cpu);
// The game's 0x50c220: the set bits of `bits`' eight pixels at `to` in `colour`, 8 bits: the next eight's place.
reg32 plotMask8(App* app, CPU& cpu, reg32 bits, reg32 colour, reg32 to);
// The game's 0x50c270: the same at 16 bits.
reg32 plotMask16(App* app, CPU& cpu, reg32 bits, reg32 colour, reg32 to);
// The game's 0x50c2c0: the same at 24 bits.
reg32 plotMask24(App* app, CPU& cpu, reg32 bits, reg32 colour, reg32 to);
// The game's 0x50c350: the same at 32 bits.
reg32 plotMask32(App* app, CPU& cpu, reg32 bits, reg32 colour, reg32 to);
// The game's 0x50c3a0: a colour of the current window's depth as an ARGB.
reg32 colourToArgb(App* app, CPU& cpu, reg32 colour);
// The game's 0x50c584: a key's handler called, at most one at a time, by the code in al or, al 0, the scan code in ah.
reg32 dispatchKey(App* app, CPU& cpu, reg32 code);
// The game's 0x50c5d4: a handler given a number, a free one or its own, and the key `key` set to it.
reg32 registerKey(App* app, CPU& cpu, reg32 key, reg32 handler);
// The game's 0x50d560: a memory copy, 32 bytes at a time through the x87 where it can.
reg32 copyBlock(App* app, CPU& cpu, reg32 from, reg32 to, reg32 left);
// The game's 0x50d6e0: ARGBs over palette indices, each result the nearest index.
reg32 blendSpanIndexed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50d7e0: ARGBs over 1555 pixels.
reg32 blendSpan1555(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50d880: ARGBs over 565 pixels.
reg32 blendSpan565(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50d910: ARGBs over 888 pixels.
reg32 blendSpan888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50d9c0: ARGBs over ARGBs; over nothing, the ARGB as it is.
reg32 blendSpan8888(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50daa0: palette indices over 565 pixels, by a palette of the alpha over 565.
reg32 blendSpanPalette565(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50db30: a shape in alpha by movdfl with the table at 0x5688a8.
reg32 drawAlphaShape(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y);
// The game's 0x50e5a0: under the queue's lock, the first element `test` says yes to, taken off the queue (qfetch).
reg32 queueFetch(App* app, CPU& cpu, reg32 queue, reg32 test, reg32 argument);
// The game's 0x509350: the closed streams' links freed.
reg32 freeClosedStreams(App* app, CPU& cpu);
// The game's 0x515740: a link for each FILE of __iob in use, stderr's flags first.
reg32 initStreams(App* app, CPU& cpu, reg32 previous);
// The game's 0x5157f4: the open streams from the `first`th of __iob closed, on shut down.
reg32 closeStreamsFrom(App* app, CPU& cpu, reg32 first);
// The game's 0x515900: the command line at [0x5677fc] split into argv: counted, copied, split again.
reg32 buildArgv(App* app, CPU& cpu);
// The game's 0x5159e4: the command line at `text` split into words, their starts at `words` if not 0: how many.
reg32 splitCommandLine(App* app, CPU& cpu, reg32 text, reg32 words, reg32 endPlace);
// The game's 0x516bf5: %f's layout of `count` digits at `digits` in `buffer`, the point after `beforePoint` + 1.
reg32 formatF(App* app, CPU& cpu, reg32 cvt, reg32 digits, reg32 count, reg32 beforePoint, reg32 buffer);
// The game's 0x516e13: %e's layout: digits, zeros, the point, more, the letter, the exponent's sign and digits.
reg32 formatE(App* app, CPU& cpu, reg32 cvt, reg32 digits, reg32 count, reg32 exponent, reg32 buffer);

/* Where the game's code jumps to another function, which returns for it (memmove,
 * __allocPOSIXHandle's), these call it and their thunks pop the return address; when the game
 * stopped in it they return at once with cpu.terminate set, and do not throw.  The rest are the
 * game's functions with no frame of their own: ends of others, hooks, __LDcvt's digits. */
// The game's 0x4ff620: a hook that does nothing.
void nothing4ff620(App* app, CPU& cpu);
// The game's 0x50d6d0: a hook that does nothing.
void nothing50d6d0(App* app, CPU& cpu);
// The game's 0x50dbf0: a hook that does nothing.
void nothing50dbf0(App* app, CPU& cpu);
// The game's 0x4ff0f8: __allocPOSIXHandle's jump (0x51e304): `handle` (eax) on to it, its answer.
reg32 allocHandleJump(App* app, CPU& cpu, reg32 handle);
// The game's 0x4ff100: __freePOSIXHandle's jump (0x51e45c): `handle` (eax) on to it, its answer.
reg32 freeHandleJump(App* app, CPU& cpu, reg32 handle);
// The game's 0x509c50: a span of `count` 4-bit pixels from `from` to `to`, by memmove: no result, 0.
reg32 copyNibbles(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509c60: the same of 8-bit pixels.
reg32 copyBytes(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x509eb0: the same of 16-bit pixels.
reg32 copyWords(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a510: the same of 32-bit pixels.
reg32 copyDwords(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x50a543: sub_50a570's end for a 16-bit type: 3.
reg32 depthIndex16(App* app, CPU& cpu);
// The game's 0x50a550: its end for a 32-bit type: 5.
reg32 depthIndex32(App* app, CPU& cpu);
// The game's 0x50a559: its end for a 15-bit type: 2.
reg32 depthIndex15(App* app, CPU& cpu);
// The game's 0x5157e0: __full_io_exit: every stream closed, then sub_509350, which the code jumps to:
// what it leaves in eax.
reg32 closeStreams(App* app, CPU& cpu);
// The game's 0x5167b3: __LDcvt: the long double at `value` into the CVT at `cvt`, its digits laid out
// in `buffer` for %e, %f or %g.
reg32 longDoubleToDigits(App* app, CPU& cpu, reg32 value, reg32 cvt, reg32 buffer);
// The game's 0x516a87: the last eight decimal digits of `value` at `at` (its code leaves ebx past
// them: `at` + 8); eax as the code leaves it.
reg32 eightDigits(App* app, CPU& cpu, reg32 value, reg32 at);
// The game's 0x516a9d: the last four, likewise (`at` + 4).
reg32 fourDigits(App* app, CPU& cpu, reg32 value, reg32 at);
// The game's 0x516ab4: the last two, of ax, likewise (`at` + 2).
reg32 twoDigits(App* app, CPU& cpu, reg32 value, reg32 at);

}  // namespace watb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
