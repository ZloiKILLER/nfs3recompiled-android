#ifndef NFS3HP_PORT_TIMER_H
#define NFS3HP_PORT_TIMER_H

#include "native_guest.h"

/* port_timer.cpp's typed functions that other files call as C++ (guest::direct), and how the game's
 * code calls each by address.  Each is nullopt when the game
 * stopped in it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x48eff0: the timer started at `rate` ticks a second (1 to 10000, else 100); eax as the way out leaves it.
std::optional<nfs::u32> timerStarted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 rate);
// The game's 0x48ef50: the timer stopped; eax as the code leaves it (`unchanged`, its eax, when it ran none).
std::optional<nfs::u32> timerStopped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x48ee30: `function` registered to run with every tick; the place the search ended at.
nfs::u32 timerCallbackAdded(nfs::App* app, nfs::u32 function);
// The game's 0x48ee90: `function` no longer registered; the place the search ended at.
nfs::u32 timerCallbackRemoved(nfs::App* app, nfs::u32 function);
// The game's 0x48ed00: the callbacks cleared; eax as the code leaves it.
nfs::u32 timerCallbacksCleared(nfs::App* app);

}  // namespace port
}  // namespace nfs3hp

#endif
