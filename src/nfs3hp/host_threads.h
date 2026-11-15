#ifndef NFS3HP_HOST_THREADS_H
#define NFS3HP_HOST_THREADS_H

#include <lib/winapp.h>

namespace win32
{
class Mutex;
struct SYSTEMTIME;
}

/* Threads, locks, events and time as the game asks for them (plan step 6): the port's host code on
 * src/lib's Thread, Mutex, Event and Timer, in the place of Windows' kernel32 and winmm.  Each takes
 * the import's words, so a call site is guest::windows<kSlot, &host::fn>(app, cpu, words...), and
 * gives back what the game's code reads of the Windows function: a handle the game stores (in the
 * application's resource table, so CloseHandle still frees it), a count, a code.
 *
 * What can wait -- a lock held by another thread, an event, a sleep, a thread's end -- lets the guest
 * context go while it waits and takes it back after (app->unlockContext / lockContext), so the
 * thread holding what it waits for can run: those take the CPU.  A thread's entry and a timer's
 * callback are the game's functions by address, run on the guest as src/lib's Thread and Timer run
 * them. */
namespace nfs3hp
{
namespace host
{

using App = win32::WinApplication;

/* Critical sections: the game's CRITICAL_SECTION holds the host lock, a recursive win32::Mutex in
 * the resource table. */
void initLock(App* app, win32::Mutex** section);
void deleteLock(App* app, win32::Mutex** section);
/* Taken without letting the context go when nobody else holds it; otherwise the context let go
 * while it waits. */
void enterLock(App* app, x86::CPU& cpu, win32::Mutex** section);
/* Let go, then the context let go and taken back: a thread waiting for the lock gets its turn. */
void leaveLock(App* app, x86::CPU& cpu, win32::Mutex** section);

/* Threads.  The handle: the thread in the resource table. */
x86::reg32 createThread(App* app, x86::reg32 attributes, x86::reg32 stackBytes, x86::reg32 entry,
                        x86::reg32 parameter, x86::reg32 flags, x86::reg32* threadId);
void exitThread(App* app, x86::CPU& cpu, x86::reg32 code);
x86::reg32 resumeThread(App* app, x86::reg32 thread);
x86::reg32 terminateThread(App* app, x86::CPU& cpu, x86::reg32 thread, x86::reg32 code);
int threadPriority(x86::reg32 thread);
x86::reg32 setThreadPriority(x86::reg32 thread, int priority);
/* A handle of the thread that asks (a new one each time, as the game closes it). */
x86::reg32 currentThread(App* app);
/* The thread's id, as its thread block (fs) has it. */
x86::reg32 currentThreadId(App* app, x86::CPU& cpu);
/* DuplicateHandle of a thread: a handle of its own for `source`, in `target`. */
x86::reg32 duplicateThread(App* app, x86::reg32 sourceProcess, x86::reg32 source, x86::reg32 targetProcess,
                           x86::reg32* target, x86::reg32 access, x86::reg32 inherit, x86::reg32 options);

/* Thread-local slots: none in the port (asserted, 0). */
x86::reg32 tlsAlloc();
x86::reg32 tlsFree(x86::reg32 index);
x86::reg32 tlsGet(x86::reg32 index);
x86::reg32 tlsSet(x86::reg32 index, x86::reg32 value);

/* Events and mutexes.  The handle: the object in the resource table. */
x86::reg32 createEvent(App* app, x86::reg32 attributes, x86::reg32 manualReset, x86::reg32 initialState,
                       const char* name);
x86::reg32 setEvent(App* app, x86::reg32 event);
x86::reg32 resetEvent(x86::reg32 event);
x86::reg32 pulseEvent(App* app, x86::reg32 event);
x86::reg32 createMutex(App* app, x86::CPU& cpu, x86::reg32 attributes, x86::reg32 initialOwner, const char* name);
/* WaitForSingleObject / WaitForMultipleObjectsEx of events: what win32::Event::wait answers, as
 * Windows does. */
x86::reg32 waitOne(App* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 ms);
x86::reg32 waitMany(App* app, x86::CPU& cpu, x86::reg32 count, const x86::reg32* handles, x86::reg32 waitAll,
                    x86::reg32 ms, x86::reg32 alertable);
x86::reg32 sleep(App* app, x86::CPU& cpu, x86::reg32 ms, x86::reg32 alertable);

/* Time: the game's milliseconds (they stand still in the background, win32::suspendTime), the
 * local date and time. */
x86::reg32 ticks(App* app, x86::CPU& cpu);
void localTime(win32::SYSTEMTIME* time);
x86::reg32 timeZone(x86::reg32 information);

/* The multimedia timer: a one-shot callback of the game's (`proc`, handed `user`) after `delay` ms;
 * its id the timer in the resource table. */
struct TimerCaps
{
    x86::reg32 periodMin;
    x86::reg32 periodMax;
};
x86::reg32 timerPeriodBegin(x86::reg32 period);
x86::reg32 timerPeriodEnd(x86::reg32 period);
x86::reg32 timerCaps(TimerCaps* caps, x86::reg32 bytes);
x86::reg32 startTimer(App* app, x86::CPU& cpu, x86::reg32 delay, x86::reg32 resolution, x86::reg32 proc,
                      x86::reg32 user, x86::reg32 flags);
x86::reg32 killTimer(App* app, x86::reg32 id);

}  // namespace host
}  // namespace nfs3hp

#endif
