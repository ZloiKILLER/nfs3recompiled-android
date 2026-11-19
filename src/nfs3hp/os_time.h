#ifndef NFS3HP_OS_TIME_H
#define NFS3HP_OS_TIME_H

#include "os_system.h"

#include <optional>

/* The port's clocks and timers, as the game's code needs them, on src/lib's clock and Timer: the
 * game's milliseconds, the local date and time, and one-shot timers that call the game's function
 * back. */
namespace nfs3hp
{
namespace os
{
namespace time
{

/* The game's milliseconds.  They stand still while the app is in the background
 * (nfs::suspendTime), and run faster for a replay checked at speed.  A thread that asks for them
 * at the rate of a loop that only waits on them has the context let go for a millisecond each time,
 * for the others to run. */
nfs::u32 ticks(App* app, nfs::ThreadState& ts);

/* The local date and time.  The port tells no time zone, and no milliseconds (0). */
struct Date
{
    nfs::u32 year;       // in full
    nfs::u32 month;      // 1 to 12
    nfs::u32 dayOfWeek;  // 0 Sunday
    nfs::u32 day;        // of the month, 1 to 31
    nfs::u32 hour;
    nfs::u32 minute;
    nfs::u32 second;
    nfs::u32 millisecond;
};
Date now();

}  // namespace time

namespace timers
{

/* The shortest and the longest delay the port's timers keep to, in milliseconds. */
constexpr nfs::u32 kShortest = 1, kLongest = 1000;

/* A one-shot timer: the game's function at `callback` called once, handed `user`, `delay` ms on,
 * on a thread of the timer's own.  Its handle, or nullopt when the thread that asks is to stop
 * (no timer then). */
std::optional<Handle> start(App* app, nfs::ThreadState& ts, nfs::u32 delay, nfs::u32 callback, nfs::u32 user);

/* The timer `timer` stopped, if it has not gone off, and let go of; none: nothing. */
void stop(App* app, Handle timer);

}  // namespace timers

}  // namespace os
}  // namespace nfs3hp

#endif
