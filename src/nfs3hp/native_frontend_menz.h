#ifndef NFS3HP_NATIVE_FRONTEND_MENZ_H
#define NFS3HP_NATIVE_FRONTEND_MENZ_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menz.cpp's typed functions (EA's libraries' small change and the C runtime's
 * arithmetic: DirectDraw, start-up, the file device, video windows, fonts, palettes, the debug
 * keys, Watcom's doubles), which other files call as C++ (guest::direct), and how the game's code
 * calls each by address.  Each calls the game's other
 * functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menz
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4ed330: the display's surface given up and locked again.
reg32 ddrawRelockSurface(App* app, CPU& cpu, reg32 display);
// The game's 0x4ed480: makesurface: a DirectDraw surface, locked once to see that it can be.
reg32 ddrawMakeSurface(App* app, CPU& cpu, reg32 display, reg32 width, reg32 height, reg32 pages, reg32 backBuffers,
                       reg32 formatWanted, reg32 capsAsked, reg32 palette);
// The game's 0x4ed730: a surface released, unlocked first if the display holds it locked.
reg32 ddrawFreeSurface(App* app, CPU& cpu, reg32 display, reg32 surface);
// The game's 0x4ed770: the surface attached to one as its back buffer.
reg32 ddrawBackSurface(App* app, CPU& cpu, reg32 surface);
// The game's 0x4ed820: a surface's size, depth, pitch and memory, each where its pointer says.
reg32 ddrawSurfaceSize(App* app, CPU& cpu, reg32 surface, reg32 widthOut, reg32 heightOut, reg32 bitsOut,
                       reg32 pitchOut, reg32 memoryOut);
// The game's 0x4edbc0: EnumDisplayModes' callback: the mode noted and reported.
reg32 ddrawEnumModeCallback(App* app, CPU& cpu, reg32 desc, reg32 list);
// The game's 0x4f9060: one more change to what the display shows, counted.
reg32 ddrawNoteChange(App* app, CPU& cpu);
// The game's 0x4f91d0: the mutex by the window class's name; the other instance brought up.
reg32 singleInstanceCheck(App* app, CPU& cpu, reg32 name);
// The game's 0x4f9260: the RAM class's purge callback: nothing can be purged.
reg32 memClassNoPurge(App* app, CPU& cpu);
// The game's 0x4f9270: the memory class RAM.
reg32 memClassCreateRam(App* app, CPU& cpu, reg32 base, reg32 size);
// The game's 0x517b30: the text library's state cleared, once, and its end to run at exit.
reg32 eaTextStart(App* app, CPU& cpu);
// The game's 0x517b70: the text library's end.
reg32 eaTextEnd(App* app, CPU& cpu);
// The game's 0x517ba0: 1 and the NEC PC-98's scan codes when the keyboard is one.
reg32 keyboardIsPc98(App* app, CPU& cpu);
// The game's 0x518040: the load's hook set to sub_517f10.
reg32 eaSetLoadHook(App* app, CPU& cpu);
// The game's 0x518fd0: the video buffers' table, once, and the list of their numbers.
reg32 videoBuffersStart(App* app, CPU& cpu, reg32 count);
// The game's 0x517d70: key 0x17: the display's flag flipped and the mode set again.
reg32 debugKeyDisplayFlag(App* app, CPU& cpu);
// The game's 0x517da0: key 0x16: the screen saved as snap.tga, and a banner that says so.
reg32 debugKeyScreenshot(App* app, CPU& cpu);
// The game's 0x517de0: the debug keys bound, Ctrl+Escape set to leave.
reg32 debugKeysBind(App* app, CPU& cpu);
// The game's 0x524930: what is on the screen copied into the bitmap.
reg32 debugBoxCopyScreen(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y);
// The game's 0x524950: the bitmap back on the screen.
reg32 debugBoxPutBack(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y);
// The game's 0x524970: a box of text, what it covers kept and put back; the answer.
reg32 debugBoxAsk(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 width, reg32 height, reg32 mask);
// The game's 0x524c30: a line of text along the foot of the screen for a time.
reg32 debugBanner(App* app, CPU& cpu, reg32 text, reg32 time);
// The game's 0x524d90: key 0x10: pause, the box in the middle of the screen until a key.
reg32 debugKeyPause(App* app, CPU& cpu);
// The game's 0x524dd0: key 4: the memory viewer's first way in.
reg32 debugKeyViewerFirst(App* app, CPU& cpu);
// The game's 0x524de0: key 5: the memory viewer's second way in.
reg32 debugKeyViewerSecond(App* app, CPU& cpu);
// The game's 0x52e440: text with a filled box behind it.
reg32 textBackgroundBox(App* app, CPU& cpu, reg32 text);
// The game's 0x4fb860: FILE_startdevice: the device's thread, if it has none.
reg32 fileDeviceStart(App* app, CPU& cpu, reg32 index);
// The game's 0x4fbd10: an operation by its id on a device: under way, queued or finished.
reg32 fileOpFind(App* app, CPU& cpu, reg32 device, reg32 id, reg32 where);
// The game's 0x4fc340: an operation's state.
reg32 fileOpStatus(App* app, CPU& cpu, reg32 id);
// The game's 0x4fc600: FILE_completeop: a finished operation taken off its device's list.
reg32 fileOpComplete(App* app, CPU& cpu, reg32 id);
// The game's 0x4fc780: waits until an operation has finished; its state.
reg32 fileOpWait(App* app, CPU& cpu, reg32 id);
// The game's 0x50ea20: the critical section of a lock entered; 0, the token.
reg32 fileLockEnter(App* app, CPU& cpu, reg32 lock);
// The game's 0x50ea40: and left (the token is not read).
reg32 fileLockLeave(App* app, CPU& cpu, reg32 lock, reg32 token);
// The game's 0x508c70: makevideowindow: a window of a size, in a fixed record or its own.
reg32 videoWindowMake(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 passedOn, reg32 where,
                      reg32 passedOn2, reg32 passedOn3);
// The game's 0x508da0: updatevideowindow: a new row table when the row size changed.
reg32 videoWindowUpdate(App* app, CPU& cpu, reg32 window, reg32 rowBytes, reg32 bits);
// The game's 0x508e70: a window's row tables freed, and its record unless it is a fixed one.
reg32 videoWindowFree(App* app, CPU& cpu, reg32 window);
// The game's 0x5199b0: the glyph of a character found by halves in a sorted table.
reg32 fontFindGlyph(App* app, CPU& cpu, reg32 character, reg32 table, reg32 count, reg32 size);
// The game's 0x519a50: FONT_getcharacter: the glyph of a character in a font.
reg32 fontGlyph(App* app, CPU& cpu, reg32 font, reg32 character);
// The game's 0x519ea0: how many bits a pixel a shape's type gives a font's glyph.
reg32 fontBitsPerPixel(App* app, CPU& cpu, reg32 shape);
// The game's 0x519f10: FONT_create: a font's record for a .FNT in memory.
reg32 fontCreate(App* app, CPU& cpu, reg32 data);
// The game's 0x5257c0: a font's property set.
reg32 fontSetProperty(App* app, CPU& cpu, reg32 font, reg32 property, reg32 value);
// The game's 0x5259d0: a font's property read back; ecx for one it has not.
reg32 fontGetProperty(App* app, CPU& cpu, reg32 font, reg32 property, reg32 ecx);
// The game's 0x51de60: colours of 3 bytes of 6 bits to 4 bytes of 8.
reg32 paletteFrom6Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to);
// The game's 0x51df60: colours of 3 bytes to 4.
reg32 paletteFrom8Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to);
// The game's 0x52448c: what a long double is: zero, a number, NaN, infinity or denormal.
reg32 longDoubleKind(App* app, CPU& cpu, reg32 value);
// The game's 0x52643c: Watcom's __FDD: edx:eax divided by ecx:ebx.  `top` is what the caller's x87 holds in st(0):
// the careful division (the Pentium's FDIV bug, which no machine here has) keeps it in memory.
reg32 doubleDivide(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh, double top);
// The game's 0x526642: Watcom's __FDS: edx:eax less ecx:ebx.
reg32 doubleSubtract(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x5267ff: Watcom's __FDM: edx:eax times ecx:ebx.
reg32 doubleMultiply(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x526674: the x87's way's end for sub_5267ff: `product` the answer (st(0), its thunk takes it off).
reg32 doubleProductFromX87(App* app, CPU& cpu, double product);
// The game's 0x52699c: edx:eax against ecx:ebx: -1, 0 or 1.
reg32 doubleCompare(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x51f2a0: the runtime's start-up test of the Pentium's FDIV bug; `top` what the caller's x87 holds in st(0).
reg32 fdivBugCheck(App* app, CPU& cpu, reg32 eax, double top);
// The game's 0x52700c: `top` (st(0)) divided by a double on the stack, safe from the bug, `below` (st(1)) kept
// for it; the quotient, which its code leaves in st(0) (eax as it came).
double safeDivide(App* app, CPU& cpu, double top, double below, reg32 divisorLow, reg32 divisorHigh);
// The game's 0x526f87: `divisor` (st(0)) and `dividend` (st(1)) put in memory for the careful division; its
// answer, which its code leaves in their place.
double safeDivideCall(App* app, CPU& cpu, double dividend, double divisor);

}  // namespace menz
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
