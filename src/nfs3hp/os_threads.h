#ifndef NFS3HP_OS_THREADS_H
#define NFS3HP_OS_THREADS_H

#include "os_system.h"

#include <optional>

/* The port's threads, locks and events, as the game's code needs them, on src/lib's Thread, Mutex
 * and Event.  The game's threads share one context (the application's, lockContext /
 * unlockContext) and take turns with it: whatever can wait -- a lock another thread holds, an
 * event, a sleep, a thread's end -- lets the context go while it waits and takes it back after, so
 * the thread it waits for can run.  Those take the thread's state, and answer false (or nullopt)
 * when the thread is to stop (ts.terminate, told while it waited): it goes no further.  A thread's
 * entry is the game's function by address, run as src/lib's Thread runs it. */
namespace nfs3hp
{
namespace os
{

/* The milliseconds of a wait that has no end: the game's -1. */
constexpr nfs::u32 kForever = 0xffffffff;

/* ---- Locks ---- */

namespace locks
{

/* The game's locks: each is a record of the game's (its critical section, 0x18 bytes) that holds
 * the port's lock -- a recursive one, in the application's table -- at its start. */

/* A lock made into the record at `section`. */
void make(App* app, nfs::u32 section);

/* The lock in the record at `section` let go of. */
void remove(App* app, nfs::u32 section);

/* The lock at `section` taken: at once when nobody else holds it, else the context let go while it
 * waits.  False when the thread is to stop. */
bool enter(App* app, nfs::ThreadState& ts, nfs::u32 section);

/* The lock at `section` given back, then the context let go and taken back: a thread that waits
 * for the lock gets its turn.  False when the thread is to stop. */
bool leave(App* app, nfs::ThreadState& ts, nfs::u32 section);

/* A lock the game holds by handle, named `name`, taken by the thread that asks when `held`; its
 * handle, or nullopt when the thread is to stop.  Its name is its own: it never finds another's. */
std::optional<Handle> named(App* app, nfs::ThreadState& ts, const char* name, bool held);

}  // namespace locks

/* ---- Threads ---- */

namespace threads
{

/* A thread made: its handle and its id. */
struct Started
{
    Handle handle;
    nfs::u32 id;
};

/* A thread that runs the game's function at `entry`, handed `parameter`, waiting to be resumed
 * when `suspended`.  The port's threads have the stack SDL gives them: `stackBytes`, what the game
 * asks for, is at most 1 MB. */
Started start(App* app, nfs::u32 entry, nfs::u32 parameter, nfs::u32 stackBytes, bool suspended);

/* The thread `thread` made suspended let go to run. */
void resume(App* app, Handle thread);

/* The thread `thread` told to stop and waited for until it has ended.  False when the thread that
 * asks is to stop. */
bool stop(App* app, nfs::ThreadState& ts, Handle thread);

/* The thread that asks ends: it stops (ts.terminate). */
void exit(nfs::ThreadState& ts);

/* A handle of the thread that asks, a new one each time (the game lets each go). */
Handle current(App* app);

/* The id of the thread that asks, as its thread block (fs) has it. */
nfs::u32 currentId(App* app, nfs::ThreadState& ts);

/* A handle of its own of the thread `thread`. */
Handle duplicate(App* app, Handle thread);

/* `ms` slept, the context let go meanwhile.  False when the thread is to stop. */
bool sleep(App* app, nfs::ThreadState& ts, nfs::u32 ms);

/* The port runs every thread at one priority: none is set, each is said to be normal. */

/* Thread-local slots: the port keeps none.  Their one user, the Watcom runtime's threads' data,
 * is never set up (the game's threads are EA's).  Asked for anyway, each says so in the log and
 * answers as a slot that holds nothing: slot 0 taken, 0 held, a value not kept (false). */
nfs::u32 takeSlot();
void giveBackSlot(nfs::u32 slot);
nfs::u32 slotValue(nfs::u32 slot);
bool setSlotValue(nfs::u32 slot, nfs::u32 value);

}  // namespace threads

/* ---- Events ---- */

namespace events
{

/* An event, set or not (`set`).  A wait that finds it set clears it, unless it is `manual`.
 * `name` (none: nullptr) is its own: it never finds another's. */
Handle make(App* app, bool manual, bool set, const char* name = nullptr);

/* The event `event` set: false when `event` is no event. */
bool set(App* app, Handle event);

/* The port never resets an event: the game's code that would is never reached.  Asked anyway, it
 * says so in the log: false. */
bool reset(App* app, Handle event);

/* The event `event` set for the threads that wait on it now, and cleared once they have seen it;
 * with none waiting it stays as it is. */
void pulse(App* app, Handle event);

/* How a wait ends: an event set (`index` its place among those waited on, the first found), the
 * time up with none set, or a handle among them that is no event. */
enum class Ended
{
    set,
    timeUp,
    failed,
};

struct Wait
{
    Ended ended;
    nfs::u32 index;
};

/* The `count` events at `handles` waited on until one is set, `ms` at most (kForever: for ever, 0:
 * a look without waiting); the context let go meanwhile.  nullopt when the thread is to stop. */
std::optional<Wait> waitAny(App* app, nfs::ThreadState& ts, const Handle* handles, nfs::u32 count, nfs::u32 ms);

/* The event `event` waited on, the same way. */
std::optional<Wait> wait(App* app, nfs::ThreadState& ts, Handle event, nfs::u32 ms);

}  // namespace events

}  // namespace os
}  // namespace nfs3hp

#endif
