#ifndef NFS3HP_NATIVE_EA_THREADS_H
#define NFS3HP_NATIVE_EA_THREADS_H

#include "native_guest.h"

/* native_ea_threads.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4df870, EA's removethread: `thread` ended and its handle closed, `stale` the handle
// the main thread's case goes on with: what CloseHandle gives back.
std::optional<nfs::u32> endThread(nfs::App* app, nfs::ThreadState& ts, nfs::u32 thread, nfs::u32 stale);
// The game's 0x4df8e0: `ms` slept: what SleepEx gives back.
std::optional<nfs::u32> sleepFor(nfs::App* app, nfs::ThreadState& ts, nfs::u32 ms);
// The game's 0x4df930: whether this is `thread`'s (0 the main thread; -1 any).
std::optional<bool> isThreadNow(nfs::App* app, nfs::ThreadState& ts, nfs::u32 thread);
// The game's 0x4df740: the threads set up, once: the lock; `eax` (as the code has it) when they are up already.
std::optional<nfs::u32> threadsSetUp(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x4dfb00: an event, auto-reset: its handle.
std::optional<nfs::u32> newAutoEvent(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4dfc10: an event that stays set until it is reset (manual-reset): its handle.
std::optional<nfs::u32> newManualEvent(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4dfb20: event `event` set: what SetEvent gives back.
std::optional<nfs::u32> eventPosted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4dfb30: event `event` set: what SetEvent gives back.
std::optional<nfs::u32> eventSet(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4dfb70: event `event` waited on for ever: it, or 0 if the wait failed.
std::optional<nfs::u32> eventWaited(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4dfd80: the matrix at `matrix` a rotation about x by the angle at `angle`: the
// cosine's bits.
std::optional<nfs::u32> rotateAboutX(nfs::App* app, nfs::ThreadState& ts, nfs::u32 matrix, nfs::u32 angle);
// The game's 0x4dfde0: the matrix at `matrix` a rotation about y by the angle at `angle`: the
// cosine's bits.
std::optional<nfs::u32> rotateAboutY(nfs::App* app, nfs::ThreadState& ts, nfs::u32 matrix, nfs::u32 angle);
// The game's 0x4eb230: a random fraction, 0 to 65535/65536.
std::optional<nfs::Float> randomFractionOf(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4eb2e0: a lock made: the lock, or 0.
std::optional<nfs::u32> newLock(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4eb360: `lock` deleted and its record put back on the free list: the list's head
// before.
std::optional<nfs::u32> lockFreed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 lock);
// The game's 0x4f9070: whether a DirectDraw surface is locked.
bool surfaceIsLocked(nfs::App* app);
// The game's 0x5221f0: the machine's physical memory in bytes.
std::optional<nfs::u32> physicalBytes(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4dfc00: the event `event` closed (CloseHandle): Windows' answer.
std::optional<nfs::u32> eventClosed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4df7a0: a thread running `function`, with a stack of `stackBytes` and EA's priority `priority`,
// into the record at `record` (a stack word): 1 if it was made, else 0.
std::optional<nfs::u32> threadStarted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 function,
                                        nfs::u32 stackBytes, nfs::u32 priority, nfs::u32 record);
// The game's 0x4dfc50: the event `event` pulsed (PulseEvent): Windows' answer.
std::optional<nfs::u32> eventPulse(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4dfc80: the event `event` set and its handle closed: CloseHandle's answer.
std::optional<nfs::u32> eventSetClosed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 event);
// The game's 0x4dfcf0: Windows' last error (GetLastError).
std::optional<nfs::u32> lastErrorOf(nfs::App* app, nfs::ThreadState& ts);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
