#ifndef NFS3HP_NATIVE_FRONTEND_HUDR_H
#define NFS3HP_NATIVE_FRONTEND_HUDR_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudr.cpp's typed functions (the race HUD's elements drawn and made again, the
 * map's pane, the HUD's text and lines), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax.
 *
 * A text's place, as sub_4897f0 takes it on the stack and the functions over it pass on: its scale,
 * colour, left, right, top and how it is aligned, six words. */
namespace nfs3hp
{
namespace frontend
{
namespace hudr
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45ab30: an element of the HUD drawn for its view, `item` its item on the editor's screen.
reg32 drawElement(App* app, CPU& cpu, reg32 item);
// The game's 0x4853c0: one element of a view's HUD made again for the rectangle it now has.
reg32 rebuildElement(App* app, CPU& cpu, reg32 view, reg32 element);
// The game's 0x48e920: the views' map panes stepped on until a split screen's two agree.
reg32 stepMapPanes(App* app, CPU& cpu);
// The game's 0x4896f0: the letters after a place: "st", "nd", "rd" or "th".
reg32 ordinalLetters(App* app, CPU& cpu, reg32 place);
// The game's 0x489750: a time in 64ths of a second written into `buffer`; edx left 0.
reg32 formatTime(App* app, CPU& cpu, reg32 buffer, reg32 time);
// The game's 0x4898b0: a time at its place.
reg32 drawTime(App* app, CPU& cpu, reg32 time, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top,
               reg32 how);
// The game's 0x489900: a percentage and, below it, a count, at their place.
reg32 drawPercentAndCount(App* app, CPU& cpu, reg32 percent, reg32 count, reg32 scale, reg32 colour, reg32 left,
                          reg32 right, reg32 top, reg32 how);
// The game's 0x4899d0: "Lap" with a count of a total, at its place.
reg32 drawLaps(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right,
               reg32 top, reg32 how);
// The game's 0x489a50: a line of the language's with a count of a total, at its place.
reg32 drawRatio(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right,
                reg32 top, reg32 how);
// The game's 0x48a320: a place among so many, at its place.
reg32 drawPlaceAmong(App* app, CPU& cpu, reg32 place, reg32 among, reg32 scale, reg32 colour, reg32 left,
                     reg32 right, reg32 top, reg32 how);
// The game's 0x48b2a0: a triangle in a colour, filled or as three lines.
reg32 drawTriangle(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 filled,
                   reg32 colour);
// The game's 0x48b4a0: a line in a colour.
reg32 drawLine(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 colour);
// The game's 0x48e770: the HUD's line mesh flushed.
reg32 flushLines(App* app, CPU& cpu);
// The game's 0x48e780: the HUD's line mesh made ready.
reg32 prepareLines(App* app, CPU& cpu);

}  // namespace hudr
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
