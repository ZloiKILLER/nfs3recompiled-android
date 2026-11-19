#ifndef NFS3HP_NATIVE_FRONTEND_INPA_H
#define NFS3HP_NATIVE_FRONTEND_INPA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_inpa.cpp's typed functions (the race's main loop and what hangs off it: a frame's
 * keys, a network race's hold, the set-up, the controls' queues and their locks), which other files
 * call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace inpa
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4c4ac0: the developer's keys while paused: the freeze, single steps and their way.
u32 raceDebugKeys(App* app, ThreadState& ts);
// The game's 0x4c4be0: the pause menu's turn in a frame: back to the race, again, or out of it.
u32 pauseMenuStep(App* app, ThreadState& ts);
// The game's 0x4c4e40: the race, every pass of it, until it is over.
u32 runRaceLoop(App* app, ThreadState& ts);
// The game's 0x4c5520: the network's lock made.
u32 networkLockMake(App* app, ThreadState& ts);
// The game's 0x4c5540: a network race's hold forgotten.
u32 holdClear(App* app, ThreadState& ts);
// The game's 0x4c5560: the network's lock freed, the connection put back when the game is over here.
u32 networkLockFree(App* app, ThreadState& ts);
// The game's 0x4c55a0: a network race's hold (asked for when `mayAsk`): 1 once the race goes on.
u32 networkHold(App* app, ThreadState& ts, u32 mayAsk);
// The game's 0x4c62a0: what a race needs before its first frame; 1 when it is given up.
u32 raceSetUp(App* app, ThreadState& ts);
// The game's 0x4c63c0: the input's locks made, the queue held stopped; not for a replay.
u32 inputLocksMake(App* app, ThreadState& ts);
// The game's 0x4c63f0: the controls' queues back to their start.
u32 inputReset(App* app, ThreadState& ts);
// The game's 0x4c64e0: the input's locks freed, unless the race was given up or is a replay.
u32 inputLocksFree(App* app, ThreadState& ts);
// The game's 0x4c7b30: the queue's lock made, once.
u32 queueLockMake(App* app, ThreadState& ts);
// The game's 0x4c7bb0: the queue's lock freed.
u32 queueLockFree(App* app, ThreadState& ts);

}  // namespace inpa
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
