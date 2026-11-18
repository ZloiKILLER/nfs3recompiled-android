#ifndef NFS3HP_NATIVE_EA_THREADS_H
#define NFS3HP_NATIVE_EA_THREADS_H

#include "native_guest.h"

/* native_ea_threads.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace ea
{

// The game's 0x4df870, EA's removethread: `thread` ended and its handle closed, `stale` the handle
// the main thread's case goes on with: what CloseHandle gives back.
std::optional<x86::reg32> endThread(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 thread, x86::reg32 stale);
// The game's 0x4df8e0: `ms` slept: what SleepEx gives back.
std::optional<x86::reg32> sleepFor(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 ms);
// The game's 0x4df930: whether this is `thread`'s (0 the main thread; -1 any).
std::optional<bool> isThreadNow(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 thread);
// The game's 0x4df740: the threads set up, once: the lock; `eax` (as the code has it) when they are up already.
std::optional<x86::reg32> threadsSetUp(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax);
// The game's 0x4dfb00: an event, auto-reset: its handle.
std::optional<x86::reg32> newAutoEvent(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4dfc10: an event that stays set until it is reset (manual-reset): its handle.
std::optional<x86::reg32> newManualEvent(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4dfb20: event `event` set: what SetEvent gives back.
std::optional<x86::reg32> eventPosted(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4dfb30: event `event` set: what SetEvent gives back.
std::optional<x86::reg32> eventSet(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4dfb70: event `event` waited on for ever: it, or 0 if the wait failed.
std::optional<x86::reg32> eventWaited(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4dfd80: the matrix at `matrix` a rotation about x by the angle at `angle`: the
// cosine's bits.
std::optional<x86::reg32> rotateAboutX(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 angle);
// The game's 0x4dfde0: the matrix at `matrix` a rotation about y by the angle at `angle`: the
// cosine's bits.
std::optional<x86::reg32> rotateAboutY(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 angle);
// The game's 0x4eb230: a random fraction, 0 to 65535/65536.
std::optional<x86::Float> randomFractionOf(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4eb2e0: a lock made: the lock, or 0.
std::optional<x86::reg32> newLock(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4eb360: `lock` deleted and its record put back on the free list: the list's head
// before.
std::optional<x86::reg32> lockFreed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lock);
// The game's 0x4f9070: whether a DirectDraw surface is locked.
bool surfaceIsLocked(win32::WinApplication* app);
// The game's 0x5221f0: the machine's physical memory in bytes.
std::optional<x86::reg32> physicalBytes(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4dfc00: the event `event` closed (CloseHandle): Windows' answer.
std::optional<x86::reg32> eventClosed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4df7a0: a thread running `function`, with a stack of `stackBytes` and EA's priority `priority`,
// into the record at `record` (a stack word): 1 if it was made, else 0.
std::optional<x86::reg32> threadStarted(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 function,
                                        x86::reg32 stackBytes, x86::reg32 priority, x86::reg32 record);
// The game's 0x4dfc50: the event `event` pulsed (PulseEvent): Windows' answer.
std::optional<x86::reg32> eventPulse(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4dfc80: the event `event` set and its handle closed: CloseHandle's answer.
std::optional<x86::reg32> eventSetClosed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event);
// The game's 0x4dfcf0: Windows' last error (GetLastError).
std::optional<x86::reg32> lastErrorOf(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace ea

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
