#ifndef NFS3HP_NATIVE_FRONTEND_MENZ_H
#define NFS3HP_NATIVE_FRONTEND_MENZ_H

#include "native_frontend.h"
#include "native_guest.h"
#include "os_video.h"

/* native_frontend_menz.cpp's typed functions (EA's libraries' small change and the C runtime's
 * arithmetic: DirectDraw, start-up, the file device, video windows, fonts, palettes, the debug
 * keys, Watcom's doubles), which other files call as C++ (guest::direct), and how the game's code
 * calls each by address.  Each calls the game's other
 * functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menz
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4ed330: the display's surface given up and locked again.
u32 ddrawRelockSurface(App* app, ThreadState& ts, u32 display);
// The game's 0x4ed480: makesurface: a DirectDraw surface, locked once to see that it can be.
u32 ddrawMakeSurface(App* app, ThreadState& ts, u32 display, u32 width, u32 height, u32 pages, u32 backBuffers,
                       u32 formatWanted, u32 capsAsked, u32 palette);
// The game's 0x4ed730: a surface released, unlocked first if the display holds it locked.
u32 ddrawFreeSurface(App* app, ThreadState& ts, u32 display, u32 surface);
// The game's 0x4ed770: the surface attached to one as its back buffer.
u32 ddrawBackSurface(App* app, ThreadState& ts, u32 surface);
// The game's 0x4ed820: a surface's size, depth, pitch and memory, each where its pointer says.
u32 ddrawSurfaceSize(App* app, ThreadState& ts, u32 surface, u32 widthOut, u32 heightOut, u32 bitsOut,
                       u32 pitchOut, u32 memoryOut);
// The game's 0x4edbc0: the display's modes' callback: the mode noted in `list` and reported; 0 ends the modes.
u32 ddrawEnumModeCallback(App* app, ThreadState& ts, const os::video::Format& mode, u32 list);
// The game's 0x4f9060: one more change to what the display shows, counted.
u32 ddrawNoteChange(App* app, ThreadState& ts);
// The game's 0x4f91d0: the mutex by the window class's name; the other instance brought up.
u32 singleInstanceCheck(App* app, ThreadState& ts, u32 name);
// The game's 0x4f9260: the RAM class's purge callback: nothing can be purged.
u32 memClassNoPurge(App* app, ThreadState& ts);
// The game's 0x4f9270: the memory class RAM.
u32 memClassCreateRam(App* app, ThreadState& ts, u32 base, u32 size);
// The game's 0x517b30: the text library's state cleared, once, and its end to run at exit.
u32 eaTextStart(App* app, ThreadState& ts);
// The game's 0x517b70: the text library's end.
u32 eaTextEnd(App* app, ThreadState& ts);
// The game's 0x517ba0: 1 and the NEC PC-98's scan codes when the keyboard is one.
u32 keyboardIsPc98(App* app, ThreadState& ts);
// The game's 0x518040: the load's hook set to sub_517f10.
u32 eaSetLoadHook(App* app, ThreadState& ts);
// The game's 0x518fd0: the video buffers' table, once, and the list of their numbers.
u32 videoBuffersStart(App* app, ThreadState& ts, u32 count);
// The game's 0x517d70: key 0x17: the display's flag flipped and the mode set again.
u32 debugKeyDisplayFlag(App* app, ThreadState& ts);
// The game's 0x517da0: key 0x16: the screen saved as snap.tga, and a banner that says so.
u32 debugKeyScreenshot(App* app, ThreadState& ts);
// The game's 0x517de0: the debug keys bound, Ctrl+Escape set to leave.
u32 debugKeysBind(App* app, ThreadState& ts);
// The game's 0x524930: what is on the screen copied into the bitmap.
u32 debugBoxCopyScreen(App* app, ThreadState& ts, u32 bitmap, u32 x, u32 y);
// The game's 0x524950: the bitmap back on the screen.
u32 debugBoxPutBack(App* app, ThreadState& ts, u32 bitmap, u32 x, u32 y);
// The game's 0x524970: a box of text, what it covers kept and put back; the answer.
u32 debugBoxAsk(App* app, ThreadState& ts, u32 text, u32 x, u32 y, u32 width, u32 height, u32 mask);
// The game's 0x524c30: a line of text along the foot of the screen for a time.
u32 debugBanner(App* app, ThreadState& ts, u32 text, u32 time);
// The game's 0x524d90: key 0x10: pause, the box in the middle of the screen until a key.
u32 debugKeyPause(App* app, ThreadState& ts);
// The game's 0x524dd0: key 4: the memory viewer's first way in.
u32 debugKeyViewerFirst(App* app, ThreadState& ts);
// The game's 0x524de0: key 5: the memory viewer's second way in.
u32 debugKeyViewerSecond(App* app, ThreadState& ts);
// The game's 0x52e440: text with a filled box behind it.
u32 textBackgroundBox(App* app, ThreadState& ts, u32 text);
// The game's 0x4fb860: FILE_startdevice: the device's thread, if it has none.
u32 fileDeviceStart(App* app, ThreadState& ts, u32 index);
// The game's 0x4fbd10: an operation by its id on a device: under way, queued or finished.
u32 fileOpFind(App* app, ThreadState& ts, u32 device, u32 id, u32 where);
// The game's 0x4fc340: an operation's state.
u32 fileOpStatus(App* app, ThreadState& ts, u32 id);
// The game's 0x4fc600: FILE_completeop: a finished operation taken off its device's list.
u32 fileOpComplete(App* app, ThreadState& ts, u32 id);
// The game's 0x4fc780: waits until an operation has finished; its state.
u32 fileOpWait(App* app, ThreadState& ts, u32 id);
// The game's 0x50ea20: the critical section of a lock entered; 0, the token.
u32 fileLockEnter(App* app, ThreadState& ts, u32 lock);
// The game's 0x50ea40: and left (the token is not read).
u32 fileLockLeave(App* app, ThreadState& ts, u32 lock, u32 token);
// The game's 0x508c70: makevideowindow: a window of a size, in a fixed record or its own.
u32 videoWindowMake(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 passedOn, u32 where,
                      u32 passedOn2, u32 passedOn3);
// The game's 0x508da0: updatevideowindow: a new row table when the row size changed.
u32 videoWindowUpdate(App* app, ThreadState& ts, u32 window, u32 rowBytes, u32 bits);
// The game's 0x508e70: a window's row tables freed, and its record unless it is a fixed one.
u32 videoWindowFree(App* app, ThreadState& ts, u32 window);
// The game's 0x5199b0: the glyph of a character found by halves in a sorted table.
u32 fontFindGlyph(App* app, ThreadState& ts, u32 character, u32 table, u32 count, u32 size);
// The game's 0x519a50: FONT_getcharacter: the glyph of a character in a font.
u32 fontGlyph(App* app, ThreadState& ts, u32 font, u32 character);
// The game's 0x519ea0: how many bits a pixel a shape's type gives a font's glyph.
u32 fontBitsPerPixel(App* app, ThreadState& ts, u32 shape);
// The game's 0x519f10: FONT_create: a font's record for a .FNT in memory.
u32 fontCreate(App* app, ThreadState& ts, u32 data);
// The game's 0x5257c0: a font's property set.
u32 fontSetProperty(App* app, ThreadState& ts, u32 font, u32 property, u32 value);
// The game's 0x5259d0: a font's property read back; ecx for one it has not.
u32 fontGetProperty(App* app, ThreadState& ts, u32 font, u32 property, u32 ecx);
// The game's 0x51de60: colours of 3 bytes of 6 bits to 4 bytes of 8.
u32 paletteFrom6Bits(App* app, ThreadState& ts, u32 count, u32 from, u32 to);
// The game's 0x51df60: colours of 3 bytes to 4.
u32 paletteFrom8Bits(App* app, ThreadState& ts, u32 count, u32 from, u32 to);
// The game's 0x52448c: what a long double is: zero, a number, NaN, infinity or denormal.
u32 longDoubleKind(App* app, ThreadState& ts, u32 value);
// The game's 0x51f2a0: the runtime's start-up test of the Pentium's FDIV bug: nothing on the port, eax as it came.
u32 fdivBugCheck(App* app, ThreadState& ts, u32 eax);

}  // namespace menz
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
