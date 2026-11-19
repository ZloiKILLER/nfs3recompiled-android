#ifndef NFS3HP_NATIVE_FRONTEND_TAIL_H
#define NFS3HP_NATIVE_FRONTEND_TAIL_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_tail.cpp's typed functions (a tick of the race and its task lists, the controls'
 * queues emptied, the track's named places and lanes, an angle's sine and cosine, the weather),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax.  Some leave edx (or esi) to their callers as the game's
 * do, through the thunk; a C++ caller gets every register but eax back as it had them. */
namespace nfs3hp
{
namespace frontend
{
namespace tail
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4be240: `block` given back to its memory, if there is one.
u32 blockFree(App* app, ThreadState& ts, u32 block);
// The game's 0x4be3a0: each task of `list` that has a function called, with its argument.
u32 runTaskList(App* app, ThreadState& ts, u32 list);
// The game's 0x4c48d0: a tick of the race: the countdown, the task lists, the standings.
u32 raceFrame(App* app, ThreadState& ts);
// The game's 0x4c7b50: the controls' queues zeroed under their lock.
u32 queuesClear(App* app, ThreadState& ts);
// The game's 0x4c8470: the named place nearest `node`, which is in none, to `out`: 1, else 0.
u32 placeNearNode(App* app, ThreadState& ts, u32 node, u32 out);
// The game's 0x4c8550: the named place `node` is in, to `out`: 0, else -1.
u32 placeAtNode(App* app, ThreadState& ts, u32 node, u32 out);
// The game's 0x4c9a80: how far from the road's middle, 16.16, `lane` is at `node`; edx left the near limit.
u32 laneOffset(App* app, ThreadState& ts, u32 node, u32 lane);
// The game's 0x4dcdf0: the weather set up as a race loads.
u32 weatherStart(App* app, ThreadState& ts);
// The game's 0x4dd230: the camera of `view` changed: its player's drops put around it again.
u32 weatherScatter(App* app, ThreadState& ts, u32 view);
// The game's 0x4dd330: `amount` of weather by the time of the race.
u32 weatherFadeInTime(App* app, ThreadState& ts, u32 amount);
// The game's 0x4dd3d0: how many drops fall at `node`.
u32 weatherFallAtNode(App* app, ThreadState& ts, u32 node);
// The game's 0x4dd4c0: of the drops, the share `seen` of them is for `player`, per cent; edx left.
u32 weatherShare(App* app, ThreadState& ts, u32 seen, u32 player);
// The game's 0x4dd510: whether `player`'s view is covered, its drops' places forgotten when it changes.
u32 weatherViewChange(App* app, ThreadState& ts, u32 player);
// The game's 0x4dd5e0: the drops of `view`'s player moved, `wanted` of them put back round it; edx left.
u32 weatherDropsStep(App* app, ThreadState& ts, u32 view, u32 wanted);
// The game's 0x4dece0: the weather's pass over `view`, the list for the caller to `out` (esi); esi left.
u32 weatherOnView(App* app, ThreadState& ts, u32 view, u32 out);
// The game's 0x4eac60: a 10-bit angle's sine to `sineOut` and cosine to `cosineOut`; edx left.
u32 angleParts(App* app, ThreadState& ts, u32 angle, u32 sineOut, u32 cosineOut);

}  // namespace tail
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
