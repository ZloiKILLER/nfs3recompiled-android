#ifndef NFS3HP_NATIVE_FRONTEND_LOOPB_H
#define NFS3HP_NATIVE_FRONTEND_LOOPB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loopb.cpp's typed functions (the race's loop around the simulation: laps and
 * standings, the race's end, the replay's record, the views, the pause's frame), which other files
 * call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loopb
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4bc100: the view a player's race HUD is drawn in, by the form.
u32 viewForHud(App* app, ThreadState& ts, u32 form);
// The game's 0x4bc130: the view a player's scene is drawn in, by the form.
u32 viewForScene(App* app, ThreadState& ts, u32 form);
// The game's 0x4bc560: the pointer shown while the mouse moves, hidden 640 ticks after.
u32 pointerIdle(App* app, ThreadState& ts);
// The game's 0x4bc620: the frame ended, told whether it is the first under a pause.
u32 endFrame(App* app, ThreadState& ts, u32 paused);
// The game's 0x4bc650: the race's text and its count drawn.
u32 textAndCount(App* app, ThreadState& ts);
// The game's 0x4bc680: the frame of a paused race, the menu over it if `menu`.
u32 pauseDraw(App* app, ThreadState& ts, u32 menu);
// The game's 0x4bc820: nothing.
u32 nothing(App* app, ThreadState& ts);
// The game's 0x4bc980: the race's drawing started, its arena made.
u32 drawStart(App* app, ThreadState& ts);
// The game's 0x4bc990: the arena let go, the drawing ended.
u32 drawEnd(App* app, ThreadState& ts);
// The game's 0x4bc9a0: the replay's mode back to 0.
u32 replayModeOff(App* app, ThreadState& ts);
// The game's 0x4bc9b0: 32 bytes packed, the length to *length; the packed row's place.
u32 packRow(App* app, ThreadState& ts, u32 row, u32 length);
// The game's 0x4bca80: and back, the bytes read to *length; the row's place.
u32 unpackRow(App* app, ThreadState& ts, u32 packed, u32 length);
// The game's 0x4bcb90: the race's start: the record cleared, or played from the top.
u32 replayStart(App* app, ThreadState& ts);
// The game's 0x4bcd60: the race's end: the replay saved while it was recording.
u32 replayEnd(App* app, ThreadState& ts);
// The game's 0x4bcda0: a block's four rows (128 bytes at `block`, handed by value) packed onto the record.
u32 storeBlock(App* app, ThreadState& ts, u32 block);
// The game's 0x4bcee0: a block's four rows taken off the record into `block` (esi), esi left past the frame's.
u32 takeBlock(App* app, ThreadState& ts, u32 block);
// The game's 0x4bd000: this tick's controls of a player onto his block, 32 of them packed away.
u32 recordControls(App* app, ThreadState& ts, u32 player);
// The game's 0x4bd090: a player's controls for this tick off the record, the television told.
u32 playControls(App* app, ThreadState& ts, u32 player);
// The game's 0x4bd380: a car's task each tick: the ghost of the player's own, the record or its playback.
u32 replayCarTask(App* app, ThreadState& ts, u32 car);
// The game's 0x4bd3f0: the pointer in the middle of a choice of the strip of four.
u32 pointerAtChoice(App* app, ThreadState& ts, u32 choice);
// The game's 0x4bd9d0: the worth an event gives a car, the most kept; edx left the place.
u32 noteEvent(App* app, ThreadState& ts, u32 worth, u32 index);
// The game's 0x4bda00: each car's worth into this moment's row, the most kept.
u32 recordEvents(App* app, ThreadState& ts);
// The game's 0x4be020: the next number of a text at *cursor, the cursor moved on.
u32 readNumber(App* app, ThreadState& ts, u32 cursor);
// The game's 0x4be5e0: all the race's views from the mode's size and the form's mirror.
u32 viewsLayout(App* app, ThreadState& ts, u32 form);
// The game's 0x4bed20: a view's projection (the angle, the last argument) and clip.
u32 useWithProjection(App* app, ThreadState& ts, u32 view, u32 angle, u32 last);
// The game's 0x4bed50: only a view's projection.
u32 projectionOnly(App* app, ThreadState& ts, u32 view, u32 angle, u32 last);
// The game's 0x4bed70: only a view's clip.
u32 useClip(App* app, ThreadState& ts, u32 view);
// The game's 0x4c4820: the config saved.
u32 saveConfig(App* app, ThreadState& ts);
// The game's 0x4c4830: everything the race holds, let go.
u32 teardown(App* app, ThreadState& ts);
// The game's 0x4c4d80: the race told to stop.
u32 stop(App* app, ThreadState& ts);
// The game's 0x4c4de0: whether a player's asked and the race may stop now; it is stopped then.
u32 asked(App* app, ThreadState& ts, u32 key);
// The game's 0x4c9d90: the records at their table cleared.
u32 clearRecords(App* app, ThreadState& ts);
// The game's 0x4cdd00: the standings emptied, then made.
u32 standingsStart(App* app, ThreadState& ts);
// The game's 0x4cdd30: the qsort's comparison of the cars at *a and *b; edx left the first.
u32 compareCars(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4cde00: the places: those still in it sorted, 0 for the others.
u32 standingsUpdate(App* app, ThreadState& ts);
// The game's 0x4cded0: a car's place.
u32 placeOf(App* app, ThreadState& ts, u32 car);
// The game's 0x4cdee0: how many a place is among.
u32 racerCount(App* app, ThreadState& ts);
// The game's 0x4cdef0: the car a place ahead of `place` from 2 on.
u32 carInPlace(App* app, ThreadState& ts, u32 place);
// The game's 0x4cdf10: the performance summary's times of a car.
u32 performanceTimes(App* app, ThreadState& ts, u32 car);
// The game's 0x4ce280: a car's task each tick: its lap's top speed, the lap's end and time, how far on.
u32 carTask(App* app, ThreadState& ts, u32 car);
// The game's 0x4ce5a0: a car the race ended before it was home: its order and made-up times.
u32 estimateRest(App* app, ThreadState& ts, u32 car);
// The game's 0x4ce750: the race over: the states set, those still racing given their order.
u32 closeRace(App* app, ThreadState& ts, u32 state);
// The game's 0x4ce800: a list's cars, but those home or out, to a state; edx left the last.
u32 setStates(App* app, ThreadState& ts, u32 list, u32 count, u32 state);
// The game's 0x4ce840: whether everyone not out is home long enough.
u32 allFinishedKnockout(App* app, ThreadState& ts);
// The game's 0x4ce8e0: whether everyone is home long enough; the short times cleared then.
u32 allFinishedTimed(App* app, ThreadState& ts);
// The game's 0x4ce960: whether knockout's sides are done.
u32 allFinishedSides(App* app, ThreadState& ts);
// The game's 0x4ceae0: each tick: whether the race has ended, then it is closed.
u32 checkRaceEnd(App* app, ThreadState& ts);
// The game's 0x4ceb50: times too short to be true, gone.
u32 clearShortTimes(App* app, ThreadState& ts);
// The game's 0x4d5d10: the drawn flag cleared.
u32 clearDrawnFlag(App* app, ThreadState& ts);
// The game's 0x4dbce0: the projection a view asks for: its place and size, the angle and the last.
u32 setProjection(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 angle, u32 last);

}  // namespace loopb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
