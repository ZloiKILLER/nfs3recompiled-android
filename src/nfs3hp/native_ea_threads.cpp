#include "native_ea_files.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "native_vertices.h"
#include "native_windows.h"
#include "host_threads.h"
#include <winapi/kernel32.h>

#include <cstdint>
#include <cstring>
#include <optional>

/* EA's threads, events, timers and locks: win\threads.c of EA's platform
 * library over Win32 -- and the small neighbours the linker left with it (the
 * rotation matrices, a random fraction, DirectDraw's surface lock flag, the
 * machine's memory).  disassemble_nfs3hp.py binds each address to the
 * function of that name here (EA_THREADS_NATIVES), and the generated code
 * calls it by name.  tools/game_checks holds each against its generated code.
 *
 * Threads: set up the first time any of them is asked for (0x9ef5b8 1), with
 * the main thread's id (0x9ef5b4), a real handle of it duplicated (0x9ef5b0)
 * and a lock made for the game (0x564480).  A thread made by createThread is
 * a record of five dwords the caller keeps: its handle, its stack size, its
 * priority (EA's, -3 to 3), 0, its id; it starts suspended at 0x4df730, which
 * calls the function handed over, its priority set, and is let go.  Where a
 * thread is asked for, 0 is the main thread and -1 the one asking.  The
 * game's main, sound, file and network threads are made so.
 *
 * Events are Win32's, auto-reset or manual; a wait is WaitForMultipleObjectsEx
 * over handles, giving back the handle that was set, or 0.  Times are in ms,
 * or in the game's ticks: 1000 / [0x5643d8] ms each, 100 a second where it is 0.
 *
 * Locks are critical sections in records of 0x20 bytes: the CRITICAL_SECTION's
 * 0x18 bytes, a link, and 'FREE' (0x46524545) on those free.  Free ones are a
 * list at 0x564a64, grown by [0x564a60] records at a time (32 at first; as
 * many as the pages allocated hold after).
 *
 * Each is a typed function under a name of its own, with an adapter under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * the registers and the x87's stack as the game's code does.  Win32 is called
 * through the imports' slots; other files' typed functions (a vector made a
 * unit long, Windows' pages got) as C++ (guest::direct, their headers); the
 * game's other functions -- and those of
 * this file that tools/game_checks stands in for (the main thread asked, a
 * sleep, a lock made and freed, DirectDraw's lock) -- by address, with their
 * arguments in the registers and on the guest's stack as the code hands them
 * over; a core keeps the registers its code pushes and pops around them
 * (Kept), and leaves the rest as the calls leave them, as the code does.  A
 * frame whose address a callee is handed stays on the guest's stack at the
 * code's depth (GuestFrame).  Where the code is a thread's entry, the CPU it
 * is handed is kept to.  The x87's operations of the rotations and the random
 * fraction are fp's, in the game's order, a float kept where the game stores
 * one. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::put;
using x86::reg32;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The threads: set up, the main thread's id, its handle; the lock made for
// the game; the game's ticks a second (0: 100).
constexpr reg32 kThreadsReady = 0x9ef5b8, kMainThreadId = 0x9ef5b4, kMainThread = 0x9ef5b0;
constexpr reg32 kGameLock = 0x564480, kTicksPerSecond = 0x5643d8, kDefaultTicks = 100;
// A thread's record: its handle, stack size, priority, 0, id.
constexpr reg32 kHandle = 0, kStackSize = 4, kPriority = 8, kUnused = 0xc, kThreadId = 0x10;
// The threads as the functions are handed them: the main thread, the one asking.
constexpr reg32 kTheMainThread = 0, kThisThread = 0xffffffff;
// The error code lastErrorText says (never set), and its text.
constexpr reg32 kErrorCode = 0x9ef5bc, kNotImplemented = 0x549900;
// The locks: free ones, how many to a grown pool; a record's link and tag.
constexpr reg32 kFreeLocks = 0x564a64, kLocksToGrow = 0x564a60;
constexpr reg32 kLockLink = 0x18, kLockTag = 0x1c, kFree = 0x46524545, kLockBytes = 0x20, kSectionBytes = 0x18;
// Where an Error is from (Error's file, function and line), and its texts.
constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr reg32 kThreadsFile = 0x549868;
constexpr reg32 kCreateThreadName = 0x549878, kCreateThreadText = 0x549888, kCreateThreadLine = 0x9f;
constexpr reg32 kRemoveThreadName = 0x5498c0, kRemoveThreadText = 0x5498d0, kRemoveThreadLine = 0xdc;
// DirectDraw's surfaces locked: the screen's, another's.
constexpr reg32 kScreenLocked = 0x9f3d0c, kSurfaceLocked = 0x567180;
// The identity matrix, 1/65536 (a double).
constexpr reg32 kIdentity = 0x564498, kOneIn65536 = 0x54b734;
constexpr reg32 kOne = 0x3f800000;
// A matrix: 3 by 3 floats, by rows.
constexpr reg32 kMatrixBytes = 0x24;
// Windows: WaitForMultipleObjectsEx's timeout; for ever; DUPLICATE_SAME_ACCESS;
// CREATE_SUSPENDED; SleepEx alertable; a MEMORYSTATUS's size and dwTotalPhys.
constexpr reg32 kWaitTimeout = 0x102;
constexpr reg32 kInfinite = 0xffffffff;
constexpr reg32 kSameAccess = 2, kCreateSuspended = 4, kAlertable = 1;
constexpr reg32 kMemoryStatusBytes = 0x20, kTotalPhys = 8;
// What is called by address.
constexpr reg32 kEaRandom = 0x4e0d88;
constexpr reg32 kSineCosine = 0x4ea970;
constexpr reg32 kIsThreadAt = 0x4df930;
constexpr reg32 kThreadStart = 0x4df730;
// The imports' slots.
constexpr reg32 kCloseHandle = 0x534488, kCreateEventA = 0x534494, kCreateThread = 0x5344a4;
constexpr reg32 kDeleteCriticalSection = 0x5344ac, kDuplicateHandle = 0x5344b4;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kExitThread = 0x5344c4, kGetCurrentProcess = 0x534504;
constexpr reg32 kGetCurrentThreadId = 0x53450c, kGetCurrentThread = 0x534510, kGetLastError = 0x534534;
constexpr reg32 kGetThreadPriority = 0x534564, kGlobalMemoryStatus = 0x534580;
constexpr reg32 kInitializeCriticalSection = 0x534584, kLeaveCriticalSection = 0x534588;
constexpr reg32 kPulseEvent = 0x5345a4, kResetEvent = 0x5345b8, kResumeThread = 0x5345bc;
constexpr reg32 kSetEvent = 0x5345e8, kSetLastError = 0x5345f4, kSetThreadPriority = 0x5345fc;
constexpr reg32 kSleepEx = 0x534604, kTerminateThread = 0x534608, kWaitForMultipleObjectsEx = 0x534630;

// The registers Kept keeps.
constexpr unsigned kKeepEax = 1, kKeepEbx = 2, kKeepEcx = 4, kKeepEdx = 8;

/* Registers the game's code pushes on entry and pops before its ret, which
 * the calls by address in between may change: as they were when the scope
 * ends. */
class Kept
{
public:
    Kept(CPU& cpu, unsigned which)
        : cpu_(cpu), which_(which), eax_(cpu.eax), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx)
    {
    }
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;
    ~Kept()
    {
        if (which_ & kKeepEax)
            cpu_.eax = eax_;
        if (which_ & kKeepEbx)
            cpu_.ebx = ebx_;
        if (which_ & kKeepEcx)
            cpu_.ecx = ecx_;
        if (which_ & kKeepEdx)
            cpu_.edx = edx_;
    }

private:
    CPU& cpu_;
    const unsigned which_;
    const reg32 eax_, ebx_, ecx_, edx_;
};

/* Room on the guest's stack for what a callee is handed the address of: the
 * code's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after (as
 * native_collide.cpp's). */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    ~GuestFrame() { cpu_.esp = esp_; }
    reg32 operator-(reg32 offset) const { return base_ - offset; }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

/* Win32's function `Function`, its import's slot `Slot`, handed `words` (the
 * first the lowest on the stack): the port's own, called as C++
 * (native_windows.h); in the check through the slot, stdcall.  What it gives
 * back in eax, the other registers as it leaves them.  nullopt when the game
 * stopped in it. */
template <reg32 Slot, auto Function, typename... Words>
std::optional<reg32> windows(App* app, CPU& cpu, Words... words)
{
    return guest::windows<Slot, Function>(app, cpu, words...);
}

/* One of the game's functions at `target`, its register arguments set
 * already: what it gives back in eax.  nullopt when the game stopped. */
std::optional<reg32> game(App* app, CPU& cpu, reg32 target)
{
    if (!guest::call(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

/* Error (0x401010) from win\threads.c's `function` at `line` with `text`:
 * where it is from set first, edx and ecx the file's and the function's
 * names as the code hands them over.  False when the game stopped. */
bool threadsError(App* app, CPU& cpu, reg32 function, reg32 line, reg32 text)
{
    cpu.edx = kThreadsFile;
    cpu.ecx = function;
    put(app, kWhereFile, kThreadsFile);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
    guest::direct<0x401010, ea::error>(app, cpu, text, guest::Words({}));
    return !cpu.terminate;
}

/* `ticks` of the game's in ms: times 1000 (as the code works it out, by
 * shifts and adds, so to 32 bits) over the ticks a second (100 where they are
 * 0), signed. */
reg32 ticksToMs(App* app, reg32 ticks)
{
    reg32 rate = at(app, kTicksPerSecond);
    if (rate == 0)
        rate = kDefaultTicks;
    const std::int64_t ms = sreg32(ticks * 1000);
    return reg32(ms / sreg32(rate));
}

/* Windows' thread priority for EA's -3 to 3 (any other: normal), by the
 * code's table at 0x4dfa10. */
reg32 windowsPriority(reg32 priority)
{
    switch (priority + 3)
    {
    case 0: return reg32(-15);
    case 1: return reg32(-2);
    case 2: return reg32(-1);
    case 4: return 1;
    case 5: return 2;
    case 6: return 15;
    default: return 0;
    }
}

/* EA's priority for Windows' -15, -2, -1, 0, 1, 2, 15; any other 0. */
reg32 eaPriority(reg32 priority)
{
    switch (sreg32(priority))
    {
    case -15: return reg32(-3);
    case -2: return reg32(-2);
    case -1: return reg32(-1);
    case 1: return 1;
    case 2: return 2;
    case 15: return 3;
    default: return 0;
    }
}

/* What a wait gives: the handle that was set, or 0; and what
 * WaitForMultipleObjectsEx gave back. */
struct Wait
{
    reg32 set;
    reg32 result;
};

/* A wait on `count` handles at `handles`, `ms` at most (kInfinite: for ever):
 * the handle whose index came back (signed, under the count), else 0 -- the
 * time up (kWaitTimeout), the wait failed or one abandoned.  edx the handles,
 * as the code hands them over (Windows leaves it as it finds it, now and
 * then); ecx kept. */
std::optional<Wait> waitForHandles(App* app, CPU& cpu, reg32 count, reg32 handles, reg32 ms)
{
    const Kept kept(cpu, kKeepEcx);
    cpu.edx = handles;
    const auto result = windows<kWaitForMultipleObjectsEx, &host::waitMany>(
        app, cpu, count, handles, 0, ms, 0);
    if (!result)
        return std::nullopt;
    const sreg32 index = sreg32(*result);
    return Wait{index >= 0 && index < sreg32(count) ? at(app, handles + *result * 4) : 0, *result};
}

/* The function a thread made by createThread runs, called with eax it.
 * False when the game stopped. */
bool runThread(App* app, CPU& cpu, reg32 function)
{
    cpu.eax = function;
    return guest::call(app, cpu, function);
}

/* The threads set up: the main thread's id kept, a handle of it of its own
 * (DuplicateHandle of GetCurrentThread's into the current process), and the
 * game's lock made (0x4eb2e0, by address): the lock back, as eax.  ecx and
 * edx kept. */
std::optional<reg32> setUpThreads(App* app, CPU& cpu)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    const auto id = windows<kGetCurrentThreadId, &host::currentThreadId>(app, cpu);
    if (!id)
        return std::nullopt;
    put(app, kMainThreadId, *id);
    const auto target = windows<kGetCurrentProcess, &win32::kernel32::GetCurrentProcess>(app, cpu);
    if (!target)
        return std::nullopt;
    const auto thread = windows<kGetCurrentThread, &host::currentThread>(app, cpu);
    if (!thread)
        return std::nullopt;
    const auto source = windows<kGetCurrentProcess, &win32::kernel32::GetCurrentProcess>(app, cpu);
    if (!source)
        return std::nullopt;
    if (!windows<kDuplicateHandle, &host::duplicateThread>(app, cpu, *source, *thread, *target, kMainThread, 0,
                                                                       0, kSameAccess))
        return std::nullopt;
    put(app, kThreadsReady, 1);
    const auto lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
    if (!lock)
        return std::nullopt;
    cpu.eax = *lock;  // as the call by address leaves it
    put(app, kGameLock, *lock);
    return *lock;
}

/* The threads set up if they are not yet, as each of their functions does
 * first, eax 0 handed over.  False when the game stopped. */
bool threadsReady(App* app, CPU& cpu)
{
    if (at(app, kThreadsReady) != 0)
        return true;
    cpu.eax = 0;
    return setUpThreads(app, cpu).has_value();
}

/* The handle of `thread`: the main thread's, this one's (GetCurrentThread),
 * a record's. */
std::optional<reg32> threadHandle(App* app, CPU& cpu, reg32 thread)
{
    if (thread == kTheMainThread)
        return at(app, kMainThread);
    if (thread == kThisThread)
        return windows<kGetCurrentThread, &host::currentThread>(app, cpu);
    return at(app, thread + kHandle);
}

/* `thread` given EA's priority `priority`: what SetThreadPriority gives
 * back.  ecx kept, edx as Windows leaves it. */
std::optional<reg32> setPriority(App* app, CPU& cpu, reg32 thread, reg32 priority)
{
    const Kept kept(cpu, kKeepEcx);
    if (!threadsReady(app, cpu))
        return std::nullopt;
    const auto handle = threadHandle(app, cpu, thread);
    if (!handle)
        return std::nullopt;
    return windows<kSetThreadPriority, &host::setThreadPriority>(app, cpu, *handle,
                                                                            windowsPriority(priority));
}

/* EA's createthread: a thread running `function`, with a stack of
 * `stackBytes` and EA's priority `priority`, into the record at `record` --
 * made suspended (its id into the frame), its priority set, let go, and this
 * thread's time given up.  1 if it was made, else 0.  Made while a DirectDraw
 * surface is locked, an Error says to unlock it first, and it is made all the
 * same.  ecx left the handle and edx as Windows leaves them. */
std::optional<reg32> startThread(App* app, CPU& cpu, reg32 function, reg32 stackBytes, reg32 priority, reg32 record)
{
    // esi, edi and ebp pushed; the thread's id below them.
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 id = frame - 4;
    const bool locked = guest::direct<0x4f9070, ea::surfaceIsLocked>(app, cpu);
    cpu.eax = locked ? 1 : 0;  // as the call by address leaves it
    if (locked)
    {
        cpu.eax = kCreateThreadLine;  // as the code has it, with the file's and the function's names
        if (!threadsError(app, cpu, kCreateThreadName, kCreateThreadLine, kCreateThreadText))
            return std::nullopt;
    }
    if (!threadsReady(app, cpu))
        return std::nullopt;
    const auto handle =
        windows<kCreateThread, &host::createThread>(app, cpu, 0, stackBytes, kThreadStart, function,
                                                               kCreateSuspended, id);
    if (!handle)
        return std::nullopt;
    cpu.ecx = *handle;
    if (*handle != 0)
    {
        put(app, record + kUnused, 0);
        put(app, record + kHandle, *handle);
        put(app, record + kStackSize, stackBytes);
        put(app, record + kPriority, priority);
        put(app, record + kThreadId, at(app, id));
        // The record and the priority in eax and edx, as the code hands them over.
        cpu.eax = record;
        cpu.edx = priority;
        if (!setPriority(app, cpu, record, priority)
            || !windows<kResumeThread, &host::resumeThread>(app, cpu, *handle)
            || !windows<kSleepEx, &host::sleep>(app, cpu, 0, kAlertable))
            return std::nullopt;
    }
    return *handle != 0 ? 1 : 0;
}

}  // namespace

/* EA's removethread: `thread` ended and its handle closed: what CloseHandle
 * gives back.  None is the main thread, which an Error says cannot be removed
 * -- and then the code goes on with `stale`, the handle ebx held as it was
 * called (a bug of the game's, kept).  ebx, ecx and edx kept. */
std::optional<reg32> endThread(App* app, CPU& cpu, reg32 thread, reg32 stale)
{
    const Kept kept(cpu, kKeepEbx | kKeepEcx | kKeepEdx);
    reg32 handle = stale;
    if (thread == kTheMainThread)
    {
        // As deep as the code calls Error, ebx, ecx, edx and esi pushed: Error's own frame goes to its
        // callees.
        const GuestFrame pushed(cpu, 0x10, 0);
        if (!threadsError(app, cpu, kRemoveThreadName, kRemoveThreadLine, kRemoveThreadText))
            return std::nullopt;
    }
    else if (thread == kThisThread)
    {
        const auto current = windows<kGetCurrentThread, &host::currentThread>(app, cpu);
        if (!current)
            return std::nullopt;
        handle = *current;
    }
    else
    {
        handle = at(app, thread + kHandle);
    }
    if (!windows<kTerminateThread, &host::terminateThread>(app, cpu, handle, 0))
        return std::nullopt;
    return windows<kCloseHandle, &win32::kernel32::CloseHandle>(app, cpu, handle);
}

/* `ms` slept (SleepEx, alertable), eax the ms as the code has them there:
 * what SleepEx gives back.  ecx and edx kept. */
std::optional<reg32> sleepFor(App* app, CPU& cpu, reg32 ms)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    cpu.eax = ms;
    return windows<kSleepEx, &host::sleep>(app, cpu, ms, kAlertable);
}

/* Whether this is `thread` (the main thread; any, for this one), by its id.
 * ecx and edx kept. */
std::optional<bool> isThreadNow(App* app, CPU& cpu, reg32 thread)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    const auto id = windows<kGetCurrentThreadId, &host::currentThreadId>(app, cpu);
    if (!id || !threadsReady(app, cpu))
        return std::nullopt;
    if (thread == kTheMainThread)
        return *id == at(app, kMainThreadId);
    if (thread == kThisThread)
        return true;
    return *id == at(app, thread + kThreadId);
}

namespace
{

/* The priority of `thread`, EA's.  ecx and edx kept. */
std::optional<reg32> priorityOf(App* app, CPU& cpu, reg32 thread)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    if (!threadsReady(app, cpu))
        return std::nullopt;
    const auto handle = threadHandle(app, cpu, thread);
    if (!handle)
        return std::nullopt;
    const auto priority = windows<kGetThreadPriority, &host::threadPriority>(app, cpu, *handle);
    if (!priority)
        return std::nullopt;
    return eaPriority(*priority);
}

/* Win32's function `Function` (its import's slot `Slot`) handed `words`: what
 * it gives back.  ecx and edx kept. */
template <reg32 Slot, auto Function, typename... Words>
std::optional<reg32> windowsKept(App* app, CPU& cpu, Words... words)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    return windows<Slot, Function>(app, cpu, words...);
}

}  // namespace

std::optional<reg32> eventSet(App* app, CPU& cpu, reg32 event)
{
    return windowsKept<kSetEvent, &host::setEvent>(app, cpu, event);
}

namespace
{

std::optional<reg32> eventReset(App* app, CPU& cpu, reg32 event)
{
    return windowsKept<kResetEvent, &host::resetEvent>(app, cpu, event);
}

std::optional<reg32> eventPulsed(App* app, CPU& cpu, reg32 event)
{
    return windowsKept<kPulseEvent, &host::pulseEvent>(app, cpu, event);
}

std::optional<reg32> handleClosed(App* app, CPU& cpu, reg32 handle)
{
    return windowsKept<kCloseHandle, &win32::kernel32::CloseHandle>(app, cpu, handle);
}

std::optional<reg32> lastErrorSet(App* app, CPU& cpu, reg32 code)
{
    return windowsKept<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, code);
}

std::optional<reg32> lockEntered(App* app, CPU& cpu, reg32 lock)
{
    return windowsKept<kEnterCriticalSection, &host::enterLock>(app, cpu, lock);
}

std::optional<reg32> lockLeft(App* app, CPU& cpu, reg32 lock)
{
    return windowsKept<kLeaveCriticalSection, &host::leaveLock>(app, cpu, lock);
}

/* An event, not set, unnamed: manual-reset or not.  ecx and edx kept. */
std::optional<reg32> newEvent(App* app, CPU& cpu, reg32 manual)
{
    return windowsKept<kCreateEventA, &host::createEvent>(app, cpu, 0, manual, 0, 0);
}

/* pollEvent's and the event waits' frame: two registers pushed, the event's
 * handle below them, which the wait is handed as its handles. */
constexpr reg32 kEventWaitPushed = 8, kEventWaitFrame = 4;

/* Whether `event` is set within `ms` (taken, if auto-reset).  edx as the wait
 * leaves it. */
std::optional<bool> eventSetWithin(App* app, CPU& cpu, reg32 event, reg32 ms)
{
    const GuestFrame frame(cpu, kEventWaitPushed, kEventWaitFrame);
    const reg32 handles = frame - 4;
    put(app, handles, event);
    const auto wait = waitForHandles(app, cpu, 1, handles, ms);
    if (!wait)
        return std::nullopt;
    return wait->set == at(app, handles);
}

/* Whether `event` is set, without waiting.  edx kept. */
std::optional<bool> eventPolled(App* app, CPU& cpu, reg32 event)
{
    const Kept kept(cpu, kKeepEdx);
    return eventSetWithin(app, cpu, event, 0);
}

}  // namespace

/* `event` waited on for ever: it, or 0 if the wait failed.  edx kept. */
std::optional<reg32> eventWaited(App* app, CPU& cpu, reg32 event)
{
    const Kept kept(cpu, kKeepEdx);
    const GuestFrame frame(cpu, kEventWaitPushed, kEventWaitFrame);
    const reg32 handles = frame - 4;
    put(app, handles, event);
    const auto wait = waitForHandles(app, cpu, 1, handles, kInfinite);
    if (!wait)
        return std::nullopt;
    return wait->set;
}

namespace
{

/* `event` set and its handle closed: what CloseHandle gives back.  ecx and
 * edx kept. */
std::optional<reg32> eventSetAndClosed(App* app, CPU& cpu, reg32 event)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    if (!windows<kSetEvent, &host::setEvent>(app, cpu, event))
        return std::nullopt;
    return windows<kCloseHandle, &win32::kernel32::CloseHandle>(app, cpu, event);
}

/* EA's stub: the error's code (0x9ef5bc's low word, which nothing sets) --
 * with `buffer`, "[%5d] sorry, not implemented" printed into it first.
 * sprintf is handed the stack as the code has it: edx, which the code pushes,
 * just above the arguments (a word sprintf never reads). */
std::optional<reg32> errorText(App* app, CPU& cpu, reg32 buffer)
{
    if (buffer != 0)
    {
        const GuestFrame frame(cpu, 4, 0);
        put(app, frame - 0, cpu.edx);
        guest::direct<0x4df690, watcom::clib::sprintf>(app, cpu, buffer, kNotImplemented,
                                                       guest::Words({at(app, kErrorCode) & 0xffff}));
        if (cpu.terminate)
            return std::nullopt;
    }
    return at(app, kErrorCode) & 0xffff;
}

/* An angle's sine and cosine (0x4ea970) of the angle at `angle` (the game's
 * units) into the floats at `sine` and `cosine`: eax, edx and ebx the
 * addresses, as the code hands them over; ebx and edx kept.  False when the
 * game stopped. */
bool sineCosine(App* app, CPU& cpu, reg32 angle, reg32 sine, reg32 cosine)
{
    const Kept kept(cpu, kKeepEbx | kKeepEdx);
    cpu.eax = angle;
    cpu.edx = sine;
    cpu.ebx = cosine;
    return guest::call(app, cpu, kSineCosine);
}

x86::Float floatAt(App* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

void putFloat(App* app, reg32 address, float value)
{
    app->getMemory<float>(address) = value;
}

/* fld; fchs; fstp: the float of `bits` negated through the x87. */
float negated(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return float(-x86::Float(value));
}

/* A turn's sine and cosine, as bits. */
struct Turn
{
    reg32 sine;
    reg32 cosine;
};

/* The sine and cosine of the angle at `angle` (the rotation's argument), as
 * the rotations work them out: into their frame, ebx, ecx and edx pushed and
 * the sine and the cosine below them. */
std::optional<Turn> turnOf(App* app, CPU& cpu, reg32 angle)
{
    const GuestFrame frame(cpu, 0xc, 8);
    const reg32 sine = frame - 8, cosine = frame - 4;
    if (!sineCosine(app, cpu, angle, sine, cosine))
        return std::nullopt;
    return Turn{at(app, sine), at(app, cosine)};
}

}  // namespace

/* The matrix at `matrix` (3 by 3 floats, by rows) a rotation about x by the
 * angle at `angle`: the cosine's bits back, which the code leaves in eax. */
std::optional<reg32> rotateAboutX(App* app, CPU& cpu, reg32 matrix, reg32 angle)
{
    const auto turn = turnOf(app, cpu, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix, kOne);
    put(app, matrix + 4, 0);
    put(app, matrix + 8, 0);
    put(app, matrix + 0xc, 0);
    put(app, matrix + 0x10, turn->cosine);
    put(app, matrix + 0x18, 0);
    put(app, matrix + 0x14, turn->sine);
    putFloat(app, matrix + 0x1c, negated(turn->sine));
    put(app, matrix + 0x20, turn->cosine);
    return turn->cosine;
}

/* The matrix at `matrix` a rotation about y by the angle at `angle`: the
 * cosine's bits back. */
std::optional<reg32> rotateAboutY(App* app, CPU& cpu, reg32 matrix, reg32 angle)
{
    const auto turn = turnOf(app, cpu, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix + 4, 0);
    put(app, matrix, turn->cosine);
    put(app, matrix + 0xc, 0);
    put(app, matrix + 0x10, kOne);
    put(app, matrix + 0x14, 0);
    putFloat(app, matrix + 8, negated(turn->sine));
    put(app, matrix + 0x1c, 0);
    put(app, matrix + 0x18, turn->sine);
    put(app, matrix + 0x20, turn->cosine);
    return turn->cosine;
}

namespace
{

/* The matrix at `matrix` a rotation about z by the angle at `angle`: the
 * cosine's bits back. */
std::optional<reg32> rotateAboutZ(App* app, CPU& cpu, reg32 matrix, reg32 angle)
{
    const auto turn = turnOf(app, cpu, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix, turn->cosine);
    put(app, matrix + 8, 0);
    put(app, matrix + 4, turn->sine);
    putFloat(app, matrix + 0xc, negated(turn->sine));
    put(app, matrix + 0x14, 0);
    put(app, matrix + 0x18, 0);
    put(app, matrix + 0x1c, 0);
    put(app, matrix + 0x20, kOne);
    put(app, matrix + 0x10, turn->cosine);
    return turn->cosine;
}

/* The matrix at `matrix` a rotation by the angle at `angle` about the axis at
 * `axis` (made a unit long first, 0x4e01c0): with s and c its sine and
 * cosine, t 1 - c, x y z the axis, t x x + c, t x y + s z, t x z - s y;
 * t x y - s z, t y y + c, t y z + s x; t x z + s y, t y z - s x, t z z + c --
 * t x kept as the float the code stores and reloads.  An angle of 0 (either
 * sign) gives the identity (0x564498).  eax as the calls leave it.  False
 * when the game stopped. */
bool rotateAboutAxis(App* app, CPU& cpu, reg32 axis, reg32 angle, reg32 matrix)
{
    if ((at(app, angle) & 0x7fffffff) == 0)
    {
        for (reg32 i = 0; i < kMatrixBytes; i += 4)
            put(app, matrix + i, at(app, kIdentity + i));
        return true;
    }
    // ecx, esi and edi pushed; the axis made a unit long at the bottom, the
    // sine 0xc above it, the cosine at the top.
    const GuestFrame frame(cpu, 0xc, 0x30);
    const reg32 unit = frame - 0x30, sineAt = frame - 0x24, cosineAt = frame - 4;
    if (!sineCosine(app, cpu, angle, sineAt, cosineAt))
        return false;
    // The axis in eax, as the code hands it over: the callee leaves it there, and so does this.
    cpu.eax = axis;
    guest::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, axis, unit);
    if (cpu.terminate)
        return false;
    const x86::Float s = floatAt(app, sineAt), c = floatAt(app, cosineAt);
    const x86::Float x = floatAt(app, unit), y = floatAt(app, unit + 4), z = floatAt(app, unit + 8);
    const x86::Float sx = fp::mul(s, x);
    const x86::Float sy = fp::mul(s, y);
    const x86::Float sz = fp::mul(s, z);
    const x86::Float t = fp::sub(1.0, c);
    const x86::Float tx = fp::mul(t, x);
    const x86::Float ty = fp::mul(t, y);
    const float txKept = float(tx);
    const x86::Float txx = fp::mul(tx, x);
    const x86::Float tz = fp::mul(t, z);
    putFloat(app, matrix, float(fp::add(txx, c)));
    putFloat(app, matrix + 4, float(fp::add(fp::mul(x86::Float(txKept), y), sz)));
    putFloat(app, matrix + 8, float(fp::sub(fp::mul(x86::Float(txKept), z), sy)));
    putFloat(app, matrix + 0xc, float(fp::sub(fp::mul(ty, x), sz)));
    putFloat(app, matrix + 0x10, float(fp::add(fp::mul(ty, y), c)));
    putFloat(app, matrix + 0x14, float(fp::add(fp::mul(ty, z), sx)));
    putFloat(app, matrix + 0x18, float(fp::add(sy, fp::mul(tz, x))));
    putFloat(app, matrix + 0x1c, float(fp::sub(fp::mul(tz, y), sx)));
    putFloat(app, matrix + 0x20, float(fp::add(fp::mul(tz, z), c)));
    return true;
}

}  // namespace

/* A random fraction, 0 to 65535/65536: eaRandom's (0x4e0d88) low word, as a
 * 64-bit integer, times the double 1/65536. */
std::optional<x86::Float> randomFractionOf(App* app, CPU& cpu)
{
    const auto drawn = game(app, cpu, kEaRandom);
    if (!drawn)
        return std::nullopt;
    return fp::mul(x86::Float(*drawn & 0xffff), x86::Float(app->getMemory<double>(kOneIn65536)));
}

namespace
{

/* The free locks grown: [0x564a60] records' bytes of pages (0x501110, handed
 * the size's address in the frame, which it may round up to whole pages),
 * each record tagged free and linked to the one before -- the first to what
 * was free -- the last the list's head, and [0x564a60] as many as the pages
 * held.  None got, the list is left empty.  The pages back. */
std::optional<reg32> growFreeLocks(App* app, CPU& cpu)
{
    // ebx, ecx and esi pushed; the size below them.
    const GuestFrame frame(cpu, 0xc, 4);
    const reg32 size = frame - 4;
    reg32 link = at(app, kFreeLocks);
    put(app, size, at(app, kLocksToGrow) << 5);
    const auto pages = guest::direct<0x501110, ea::allocatePages>(app, cpu, size);
    if (!pages)
        return std::nullopt;
    reg32 count = at(app, kLocksToGrow);
    reg32 head = *pages;
    if (*pages != 0)
    {
        count = at(app, size) >> 5;
        reg32 record = *pages;
        for (sreg32 i = 0; i < sreg32(count); ++i)
        {
            put(app, record + kLockTag, kFree);
            put(app, record + kLockLink, link);
            link = record;
            record += kLockBytes;
        }
        head = link;
    }
    put(app, kLocksToGrow, count);
    put(app, kFreeLocks, head);
    return *pages;
}

}  // namespace

/* A lock made: a free record taken (the list grown if empty), cleared
 * (0x4e070c, eax and edx) and its critical section initialized.  0 if there
 * was none.  ecx and edx kept. */
std::optional<reg32> newLock(App* app, CPU& cpu)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    if (!threadsReady(app, cpu))
        return std::nullopt;
    if (at(app, kFreeLocks) == 0)
    {
        // As deep as the code calls growLocks, ebx pushed and the return address's slot: its frame's
        // address goes to 0x501110.
        const GuestFrame called(cpu, 8, 0);
        if (!growFreeLocks(app, cpu))
            return std::nullopt;
    }
    const reg32 lock = at(app, kFreeLocks);
    if (lock == 0)
        return 0;
    put(app, kFreeLocks, at(app, lock + kLockLink));
    // eax and edx as the call by address loads them (edx kept; eax Windows' after).
    cpu.eax = lock;
    cpu.edx = kSectionBytes;
    guest::direct<0x4e070c, ea::clearBytes>(app, cpu, lock, kSectionBytes);
    if (cpu.terminate
        || !windows<kInitializeCriticalSection, &host::initLock>(app, cpu, lock))
        return std::nullopt;
    return lock;
}

/* `lock` (none: nothing) deleted and its record put back on the free list,
 * tagged free: the list's head before back (0 for none).  ecx and edx kept. */
std::optional<reg32> lockFreed(App* app, CPU& cpu, reg32 lock)
{
    if (lock == 0)
        return 0;
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    if (!windows<kDeleteCriticalSection, &host::deleteLock>(app, cpu, lock))
        return std::nullopt;
    const reg32 head = at(app, kFreeLocks);
    put(app, lock + kLockTag, kFree);
    put(app, kFreeLocks, lock);
    put(app, lock + kLockLink, head);
    return head;
}

/* Whether a DirectDraw surface is locked: the screen's, or another's. */
bool surfaceIsLocked(App* app)
{
    return at(app, kScreenLocked) != 0 || at(app, kSurfaceLocked) != 0;
}

/* The machine's physical memory in bytes: GlobalMemoryStatus's dwTotalPhys,
 * the MEMORYSTATUS in the frame under ecx and edx pushed.  ecx and edx kept. */
std::optional<reg32> physicalBytes(App* app, CPU& cpu)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    const GuestFrame frame(cpu, 8, kMemoryStatusBytes);
    const reg32 status = frame - kMemoryStatusBytes;
    put(app, status, kMemoryStatusBytes);
    if (!windows<kGlobalMemoryStatus, &win32::kernel32::GlobalMemoryStatus>(app, cpu, status))
        return std::nullopt;
    return at(app, status + kTotalPhys);
}

namespace
{

/* The adapter of a core of one register's argument: eax what it gives back,
 * ret. */
void oneArgument(App* app, CPU& cpu, std::optional<reg32> (*core)(App*, CPU&, reg32), reg32 argument)
{
    const auto result = core(app, cpu, argument);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

}  // namespace

void waitForObjects(win32::WinApplication* app, x86::CPU& cpu);
void threadStart(win32::WinApplication* app, x86::CPU& cpu);
void threadsInit(win32::WinApplication* app, x86::CPU& cpu);
void createThread(win32::WinApplication* app, x86::CPU& cpu);
void removeThread(win32::WinApplication* app, x86::CPU& cpu);
void sleepMs(win32::WinApplication* app, x86::CPU& cpu);
void sleepTicks(win32::WinApplication* app, x86::CPU& cpu);
void isThread(win32::WinApplication* app, x86::CPU& cpu);
void threadPriority(win32::WinApplication* app, x86::CPU& cpu);
void setThreadPriority(win32::WinApplication* app, x86::CPU& cpu);
void exitThread(win32::WinApplication* app, x86::CPU& cpu);
void createEvent(win32::WinApplication* app, x86::CPU& cpu);
void postEvent(win32::WinApplication* app, x86::CPU& cpu);
void signalEvent(win32::WinApplication* app, x86::CPU& cpu);
void pollEvent(win32::WinApplication* app, x86::CPU& cpu);
void waitEvent(win32::WinApplication* app, x86::CPU& cpu);
void waitEventTicks(win32::WinApplication* app, x86::CPU& cpu);
void waitForAny(win32::WinApplication* app, x86::CPU& cpu);
void closeEvent(win32::WinApplication* app, x86::CPU& cpu);
void createManualEvent(win32::WinApplication* app, x86::CPU& cpu);
void setEvent(win32::WinApplication* app, x86::CPU& cpu);
void resetEvent(win32::WinApplication* app, x86::CPU& cpu);
void pulseEvent(win32::WinApplication* app, x86::CPU& cpu);
void waitManualEvent(win32::WinApplication* app, x86::CPU& cpu);
void setAndCloseEvent(win32::WinApplication* app, x86::CPU& cpu);
void lastErrorText(win32::WinApplication* app, x86::CPU& cpu);
void setLastError(win32::WinApplication* app, x86::CPU& cpu);
void lastError(win32::WinApplication* app, x86::CPU& cpu);
void doNothing(win32::WinApplication* app, x86::CPU& cpu);
void xRotation(win32::WinApplication* app, x86::CPU& cpu);
void yRotation(win32::WinApplication* app, x86::CPU& cpu);
void zRotation(win32::WinApplication* app, x86::CPU& cpu);
void axisRotation(win32::WinApplication* app, x86::CPU& cpu);
void randomFraction(win32::WinApplication* app, x86::CPU& cpu);
void growLocks(win32::WinApplication* app, x86::CPU& cpu);
void makeLock(win32::WinApplication* app, x86::CPU& cpu);
void enterLock(win32::WinApplication* app, x86::CPU& cpu);
void leaveLock(win32::WinApplication* app, x86::CPU& cpu);
void freeLock(win32::WinApplication* app, x86::CPU& cpu);
void surfaceLocked(win32::WinApplication* app, x86::CPU& cpu);
void physicalMemory(win32::WinApplication* app, x86::CPU& cpu);

/* 0x4df6f0: waitForHandles for eax handles at edx, ebx ms at most (-1 for
 * ever): eax the handle that was set, or 0; ebx what the wait gave back; edx
 * as Windows leaves it. */
void waitForObjects(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto wait = waitForHandles(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!wait)
        return;
    cpu.eax = wait->set;
    cpu.ebx = wait->result;
    cpu.esp += 4;
}

/* 0x4df730: where a thread createThread made starts (a ThreadProc): the
 * function it was handed as its parameter run (runThread); 0 back, ret 4. */
void threadStart(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!runThread(app, cpu, at(app, cpu.esp + 4)))
        return;
    cpu.eax = 0;
    cpu.esp += 4 + 4;
}

/* 0x4df740: setUpThreads, once: eax left the lock (as it was, set up
 * already). */
void threadsInit(win32::WinApplication* app, x86::CPU& cpu)
{
    if (at(app, kThreadsReady) == 0)
    {
        const auto lock = setUpThreads(app, cpu);
        if (!lock)
            return;
        cpu.eax = *lock;
    }
    cpu.esp += 4;
}

/* 0x4df7a0: startThread of function eax, a stack of edx bytes, priority ebx,
 * into the record at [esp + 4]: eax 1 if it was made; ebx left the record;
 * ret 4. */
void createThread(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 record = at(app, cpu.esp + 4);
    const auto made = startThread(app, cpu, cpu.eax, cpu.edx, cpu.ebx, record);
    if (!made)
        return;
    cpu.eax = *made;
    cpu.ebx = record;
    cpu.esp += 4 + 4;
}

/* 0x4df870: endThread of record eax (-1: the one asking), ebx the handle the
 * main thread's case goes on with.  No caller in the game. */
void removeThread(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto result = endThread(app, cpu, cpu.eax, cpu.ebx);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

/* 0x4df8e0: eax ms slept. */
void sleepMs(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, sleepFor, cpu.eax);
}

/* 0x4df8f0: eax of the game's ticks slept.  No caller in the game. */
void sleepTicks(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, sleepFor, ticksToMs(app, cpu.eax));
}

/* 0x4df930: 1 if this is the thread of record eax (0: the main thread; -1:
 * any, so always), else 0. */
void isThread(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto is = isThreadNow(app, cpu, cpu.eax);
    if (!is)
        return;
    cpu.eax = *is ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4df980: the priority of the thread of record eax (0: the main thread;
 * -1: this one), EA's.  No caller in the game. */
void threadPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, priorityOf, cpu.eax);
}

/* 0x4dfa30: setPriority of the thread of record eax (0: the main thread; -1:
 * this one) to EA's priority edx: eax what SetThreadPriority gives back. */
void setThreadPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto result = setPriority(app, cpu, cpu.eax, cpu.edx);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

/* 0x4dfaf0: this thread ended (ExitThread(0)).  No caller in the game. */
void exitThread(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto result = windowsKept<kExitThread, &host::exitThread>(app, cpu, 0);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

/* 0x4dfb00's core: newEvent, auto-reset. */
std::optional<reg32> newAutoEvent(App* app, CPU& cpu)
{
    return newEvent(app, cpu, 0);
}

/* 0x4dfb20's core: eventSet of `event`, handed in edx. */
std::optional<reg32> eventPosted(App* app, CPU& cpu, reg32 event)
{
    return eventSet(app, cpu, event);
}

/* 0x4dfb00: an event, set by one and reset as a wait takes it (auto-reset). */
void createEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, newEvent, 0);
}

/* 0x4dfb20: event edx set; eax (what the callers would post with it) not
 * kept.  It runs on into signalEvent. */
void postEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventSet, cpu.edx);
}

/* 0x4dfb30: event eax set -- a second entry of postEvent's, past its first
 * instruction (and its nops), which the game calls too. */
void signalEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventSet, cpu.eax);
}

/* 0x4dfb40: 1 if event eax is set (taken, if auto-reset), else 0, without
 * waiting. */
void pollEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto set = eventPolled(app, cpu, cpu.eax);
    if (!set)
        return;
    cpu.eax = *set ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4dfb70: event eax waited on for ever: it, or 0 if the wait failed. */
void waitEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventWaited, cpu.eax);
}

/* 0x4dfb90: 1 if event eax is set within edx of the game's ticks, else 0.
 * edx left as the wait leaves it.  No caller in the game. */
void waitEventTicks(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto set = eventSetWithin(app, cpu, cpu.eax, ticksToMs(app, cpu.edx));
    if (!set)
        return;
    cpu.eax = *set ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4dfbf0: eax handles at edx waited on for ever: the one set, or 0. */
void waitForAny(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto wait = waitForHandles(app, cpu, cpu.eax, cpu.edx, kInfinite);
    if (!wait)
        return;
    cpu.eax = wait->set;
    cpu.esp += 4;
}

/* 0x4dfc00: handle eax closed -- an event's, a thread's. */
void closeEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, handleClosed, cpu.eax);
}

/* 0x4dfc10: an event that stays set until it is reset (manual-reset). */
void createManualEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, newEvent, 1);
}

/* 0x4dfc30: event eax set -- the same code as signalEvent's.  No caller in
 * the game. */
void setEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventSet, cpu.eax);
}

/* 0x4dfc40: event eax reset.  No caller in the game. */
void resetEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventReset, cpu.eax);
}

/* 0x4dfc50: event eax pulsed: set, those waiting let go, reset. */
void pulseEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventPulsed, cpu.eax);
}

/* 0x4dfc60: event eax waited on for ever -- waitEvent's code again, for the
 * manual events. */
void waitManualEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventWaited, cpu.eax);
}

/* 0x4dfc80: event eax set, and its handle closed. */
void setAndCloseEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, eventSetAndClosed, cpu.eax);
}

/* 0x4dfca0: errorText into the buffer at eax (none: only the code).  No
 * caller in the game. */
void lastErrorText(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, errorText, cpu.eax);
}

/* 0x4dfce0: Windows' last error set to eax.  No caller in the game. */
void setLastError(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, lastErrorSet, cpu.eax);
}

/* 0x4dfcf0: Windows' last error. */
void lastError(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto result = windowsKept<kGetLastError, &win32::kernel32::GetLastError>(app, cpu);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

/* 0x4dfd00: nothing (a bare ret).  No caller in the game. */
void doNothing(win32::WinApplication*, x86::CPU& cpu)
{
    cpu.esp += 4;
}

/* 0x4dfd80: rotateAboutX of the matrix at eax by the angle at [esp + 4]: eax
 * left the cosine's bits; ret 4. */
void xRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto cosine = rotateAboutX(app, cpu, cpu.eax, cpu.esp + 4);
    if (!cosine)
        return;
    cpu.eax = *cosine;
    cpu.esp += 4 + 4;
}

/* 0x4dfde0: rotateAboutY of the matrix at eax by the angle at [esp + 4]. */
void yRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto cosine = rotateAboutY(app, cpu, cpu.eax, cpu.esp + 4);
    if (!cosine)
        return;
    cpu.eax = *cosine;
    cpu.esp += 4 + 4;
}

/* 0x4dfe40: rotateAboutZ of the matrix at eax by the angle at [esp + 4]. */
void zRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto cosine = rotateAboutZ(app, cpu, cpu.eax, cpu.esp + 4);
    if (!cosine)
        return;
    cpu.eax = *cosine;
    cpu.esp += 4 + 4;
}

/* 0x4dfea0: rotateAboutAxis of the matrix at [esp + 8] by the angle at
 * [esp + 4] about the axis at eax; ret 8.  No caller in the game. */
void axisRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!rotateAboutAxis(app, cpu, cpu.eax, cpu.esp + 4, at(app, cpu.esp + 8)))
        return;
    cpu.esp += 4 + 8;
}

/* 0x4eb230: randomFractionOf onto the x87's stack; eax and edx kept. */
void randomFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    const Kept kept(cpu, kKeepEax | kKeepEdx);
    const auto fraction = randomFractionOf(app, cpu);
    if (!fraction)
        return;
    cpu.fpu.push(*fraction);
    cpu.esp += 4;
}

/* 0x4eb260: growFreeLocks: eax left the pages. */
void growLocks(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto pages = growFreeLocks(app, cpu);
    if (!pages)
        return;
    cpu.eax = *pages;
    cpu.esp += 4;
}

/* 0x4eb2e0: newLock: the lock, or 0. */
void makeLock(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto lock = newLock(app, cpu);
    if (!lock)
        return;
    cpu.eax = *lock;
    cpu.esp += 4;
}

/* 0x4eb340: lock eax taken (EnterCriticalSection).  No caller in the game. */
void enterLock(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, lockEntered, cpu.eax);
}

/* 0x4eb350: lock eax given back (LeaveCriticalSection).  No caller in the
 * game. */
void leaveLock(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, lockLeft, cpu.eax);
}

/* 0x4eb360: lockFreed of lock eax: eax left the list's head before. */
void freeLock(win32::WinApplication* app, x86::CPU& cpu)
{
    oneArgument(app, cpu, lockFreed, cpu.eax);
}

/* 0x4f9070: 1 if a DirectDraw surface is locked (the screen's, 0x9f3d0c, or
 * another's, 0x567180), else 0 -- DirectDraw's, not the threads': what
 * createThread asks. */
void surfaceLocked(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = surfaceIsLocked(app) ? 1 : 0;
    cpu.esp += 4;
}

/* 0x5221f0: physicalBytes -- the file system's, not the threads'. */
void physicalMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto bytes = physicalBytes(app, cpu);
    if (!bytes)
        return;
    cpu.eax = *bytes;
    cpu.esp += 4;
}

/* 0x4dfc00's and 0x4dfcf0's typed cores (native_ea_threads.h): as their adapters run them. */
std::optional<reg32> eventClosed(App* app, CPU& cpu, reg32 event)
{
    return handleClosed(app, cpu, event);
}

std::optional<reg32> lastErrorOf(App* app, CPU& cpu)
{
    return windowsKept<kGetLastError, &win32::kernel32::GetLastError>(app, cpu);
}

}  // namespace ea
}  // namespace nfs3hp
