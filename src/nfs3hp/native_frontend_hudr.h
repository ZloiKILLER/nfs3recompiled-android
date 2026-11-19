#ifndef NFS3HP_NATIVE_FRONTEND_HUDR_H
#define NFS3HP_NATIVE_FRONTEND_HUDR_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudr.cpp's typed functions (the race HUD's elements drawn and made again, the
 * map's pane, the HUD's text and lines), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
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

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x45ab30: an element of the HUD drawn for its view, `item` its item on the editor's screen.
u32 drawElement(App* app, ThreadState& ts, u32 item);
// The game's 0x4853c0: one element of a view's HUD made again for the rectangle it now has.
u32 rebuildElement(App* app, ThreadState& ts, u32 view, u32 element);
// The game's 0x48e920: the views' map panes stepped on until a split screen's two agree.
u32 stepMapPanes(App* app, ThreadState& ts);
// The game's 0x4896f0: the letters after a place: "st", "nd", "rd" or "th".
u32 ordinalLetters(App* app, ThreadState& ts, u32 place);
// The game's 0x489750: a time in 64ths of a second written into `buffer`; edx left 0.
u32 formatTime(App* app, ThreadState& ts, u32 buffer, u32 time);
// The game's 0x4898b0: a time at its place.
u32 drawTime(App* app, ThreadState& ts, u32 time, u32 scale, u32 colour, u32 left, u32 right, u32 top,
               u32 how);
// The game's 0x489900: a percentage and, below it, a count, at their place.
u32 drawPercentAndCount(App* app, ThreadState& ts, u32 percent, u32 count, u32 scale, u32 colour, u32 left,
                          u32 right, u32 top, u32 how);
// The game's 0x4899d0: "Lap" with a count of a total, at its place.
u32 drawLaps(App* app, ThreadState& ts, u32 done, u32 total, u32 scale, u32 colour, u32 left, u32 right,
               u32 top, u32 how);
// The game's 0x489a50: a line of the language's with a count of a total, at its place.
u32 drawRatio(App* app, ThreadState& ts, u32 done, u32 total, u32 scale, u32 colour, u32 left, u32 right,
                u32 top, u32 how);
// The game's 0x48a320: a place among so many, at its place.
u32 drawPlaceAmong(App* app, ThreadState& ts, u32 place, u32 among, u32 scale, u32 colour, u32 left,
                     u32 right, u32 top, u32 how);
// The game's 0x48b2a0: a triangle in a colour, filled or as three lines.
u32 drawTriangle(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 filled,
                   u32 colour);
// The game's 0x48b4a0: a line in a colour.
u32 drawLine(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 colour);
// The game's 0x48e770: the HUD's line mesh flushed.
u32 flushLines(App* app, ThreadState& ts);
// The game's 0x48e780: the HUD's line mesh made ready.
u32 prepareLines(App* app, ThreadState& ts);

}  // namespace hudr
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
