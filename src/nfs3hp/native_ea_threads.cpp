#include "native_guest.h"

#include <cstdint>

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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack as the code moves it.
 * Win32 is called through the imports' slots; the game's other functions, and
 * those of this file that tools/game_checks stands in for (the main thread
 * asked, a sleep, a lock made and freed, DirectDraw's lock), by address. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using x86::reg32;
using x86::sreg32;

// The threads: set up, the main thread's id, its handle; the lock made for
// the game; the game's ticks a second (0: 100).
constexpr reg32 kThreadsReady = 0x9ef5b8, kMainThreadId = 0x9ef5b4, kMainThread = 0x9ef5b0;
constexpr reg32 kGameLock = 0x564480, kTicksPerSecond = 0x5643d8, kDefaultTicks = 100;
// A thread's record: its handle, stack size, priority, 0, id.
constexpr reg32 kHandle = 0, kStackSize = 4, kPriority = 8, kUnused = 0xc, kThreadId = 0x10;
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
// The identity matrix, 1/65536.
constexpr reg32 kIdentity = 0x564498, kOneIn65536 = 0x54b734;
constexpr reg32 kOne = 0x3f800000;
// Windows: WaitForMultipleObjectsEx's timeout.
constexpr reg32 kWaitTimeout = 0x102;
constexpr reg32 kInfinite = 0xffffffff;
// What is called by address.
constexpr reg32 kError = 0x401010, kSprintf = 0x4df690, kFillZero = 0x4e070c, kEaRandom = 0x4e0d88;
constexpr reg32 kSineCosine = 0x4ea970, kNormalized = 0x4e01c0, kAllocatePages = 0x501110;
constexpr reg32 kIsThreadAt = 0x4df930, kLockMadeAt = 0x4eb2e0, kSurfaceLockedAt = 0x4f9070;
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

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* call dword ptr [slot]: an import. */
bool import(win32::WinApplication* app, x86::CPU& cpu, reg32 slot)
{
    return byAddress(app, cpu, at(app, slot));
}

/* push ecx; push edx; push eax; call [slot]; pop edx; pop ecx; ret: Win32's
 * function of one handle, eax, as it gives back. */
void ofHandle(win32::WinApplication* app, x86::CPU& cpu, reg32 slot)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, slot))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* push ecx; push edx; push 0; push 0; push `manual`; push 0; call
 * CreateEventA; pop edx; pop ecx; ret: an event, not set, unnamed. */
void newEvent(win32::WinApplication* app, x86::CPU& cpu, reg32 manual)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, 0);
    push(app, cpu, 0);
    push(app, cpu, manual);
    push(app, cpu, 0);
    if (!import(app, cpu, kCreateEventA))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* Signed division of edx:eax by `divisor`, as idiv. */
void idiv(x86::CPU& cpu, reg32 divisor)
{
    const std::int64_t dividend = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
    cpu.eax = reg32(dividend / sreg32(divisor));
    cpu.edx = reg32(dividend % sreg32(divisor));
}

/* ecx the game's ticks a second (100 for 0), eax `ticks` in ms as the code
 * works it out -- times 1000 by shifts and adds -- edx the remainder. */
void ticksToMs(win32::WinApplication* app, x86::CPU& cpu, reg32 ticks)
{
    cpu.ecx = at(app, kTicksPerSecond);
    if (cpu.ecx == 0)
        cpu.ecx = kDefaultTicks;
    cpu.eax = ticks << 2;
    cpu.eax -= ticks;
    cpu.eax <<= 3;
    cpu.eax += ticks;
    cpu.eax <<= 3;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.edx += cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    idiv(cpu, cpu.ecx);
}

/* Error (0x401010) from win\threads.c's `function` at `line`, its text
 * pushed already; the text taken off after. */
bool threadsError(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!byAddress(app, cpu, kError))
        return false;
    cpu.esp += 4;
    return true;
}

x86::Float loadFloat(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

void storeFloat(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* fxch st(i). */
void exchange(x86::CPU& cpu, int i)
{
    std::swap(cpu.fpu.st(0), cpu.fpu.st(i));
}

/* fstp: st(0) stored as a float, popped. */
void storePop(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    storeFloat(app, address, cpu.fpu.pop());
}

/* The rotation matrices' start: push ebx, ecx, edx; 8 bytes of locals; the
 * sine and cosine of the angle at [esp + 4] into them (0x4ea970: edx the
 * sine's, ebx the cosine's); ecx the matrix, eax's. */
bool rotationStart(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 8;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.esp + 4;
    cpu.edx = cpu.esp;
    cpu.eax = cpu.esp + 0x18;
    return byAddress(app, cpu, kSineCosine);
}

/* Their end: the locals off, pop edx, ecx, ebx; ret 4. */
void rotationEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.esp += 8;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 4;
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

/* 0x4df6f0: a wait on eax handles at edx, ebx ms at most (-1 for ever): the
 * handle that was set, or 0 -- the time up, or the wait failed.  ebx left
 * what the wait gave back. */
void waitForObjects(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    push(app, cpu, 0);
    push(app, cpu, cpu.ebx);
    push(app, cpu, 0);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kWaitForMultipleObjectsEx))
        return;
    cpu.ebx = cpu.eax;
    if (sreg32(cpu.eax) >= 0 && sreg32(cpu.eax) < sreg32(cpu.edi))
        cpu.eax = at(app, cpu.esi + cpu.eax * 4);
    else
        cpu.eax = 0;  // cmp ebx, 0x102 (kWaitTimeout) before it: nothing reads it
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df730: where a thread createThread made starts (a ThreadProc): the
 * function it was handed as its parameter called, with eax it; 0 back. */
void threadStart(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = at(app, cpu.esp + 4);
    if (!byAddress(app, cpu, cpu.eax))
        return;
    cpu.eax = 0;
    cpu.esp += 4 + 4;
}

/* 0x4df740: the threads set up, once: the main thread's id and a handle of
 * it of its own (DuplicateHandle of GetCurrentThread's), and the game's lock
 * made (0x564480).  eax left the lock. */
void threadsInit(win32::WinApplication* app, x86::CPU& cpu)
{
    if (at(app, kThreadsReady) != 0)
    {
        cpu.esp += 4;
        return;
    }
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ecx);
    if (!import(app, cpu, kGetCurrentThreadId))
        return;
    push(app, cpu, 2);  // DUPLICATE_SAME_ACCESS
    push(app, cpu, 0);
    push(app, cpu, 0);
    push(app, cpu, kMainThread);
    put(app, kMainThreadId, cpu.eax);
    if (!import(app, cpu, kGetCurrentProcess))
        return;
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kGetCurrentThread))
        return;
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kGetCurrentProcess))
        return;
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kDuplicateHandle))
        return;
    put(app, kThreadsReady, 1);
    if (!byAddress(app, cpu, kLockMadeAt))
        return;
    put(app, kGameLock, cpu.eax);
    cpu.ecx = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df7a0: EA's createthread: a thread running function eax, with a stack
 * of edx bytes and priority ebx (EA's, -3 to 3), into the record at [esp +
 * 4] -- made suspended, its priority set, let go, and this thread's time
 * given up.  1 if it was made, else 0.  Made while a DirectDraw surface is
 * locked, an Error says to unlock it first, and it is made all the same.
 * ebx left the record; ecx and edx what Windows leaves. */
void createThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.esp -= 4;
    cpu.ebp = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.esi = cpu.ebx;
    cpu.ebx = at(app, cpu.esp + 0x14);
    if (!byAddress(app, cpu, kSurfaceLockedAt))
        return;
    if (cpu.eax != 0)
    {
        cpu.edx = kThreadsFile;
        cpu.ecx = kCreateThreadName;
        cpu.eax = kCreateThreadLine;
        push(app, cpu, kCreateThreadText);
        put(app, kWhereFile, cpu.edx);
        put(app, kWhereFunction, cpu.ecx);
        put(app, kWhereLine, cpu.eax);
        if (!threadsError(app, cpu))
            return;
    }
    if (at(app, kThreadsReady) == 0)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, threadsInit))
            return;
    }
    cpu.eax = cpu.esp;
    push(app, cpu, cpu.eax);
    push(app, cpu, 4);  // CREATE_SUSPENDED
    push(app, cpu, cpu.ebp);
    push(app, cpu, kThreadStart);
    push(app, cpu, cpu.edi);
    push(app, cpu, 0);
    if (!import(app, cpu, kCreateThread))
        return;
    cpu.ecx = cpu.eax;
    cpu.ebp = cpu.eax;
    if (cpu.eax != 0)
    {
        put(app, cpu.ebx + kUnused, 0);
        put(app, cpu.ebx + kHandle, cpu.eax);
        put(app, cpu.ebx + kStackSize, cpu.edi);
        put(app, cpu.ebx + kPriority, cpu.esi);
        cpu.eax = at(app, cpu.esp);
        cpu.edx = cpu.esi;
        put(app, cpu.ebx + kThreadId, cpu.eax);
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, setThreadPriority))
            return;
        push(app, cpu, cpu.ecx);
        if (!import(app, cpu, kResumeThread))
            return;
        push(app, cpu, 1);
        push(app, cpu, 0);
        if (!import(app, cpu, kSleepEx))
            return;
    }
    cpu.eax = cpu.ebp != 0 ? 1 : 0;
    cpu.esp += 4;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x4df870: EA's removethread: the thread of record eax ended and its handle
 * closed (-1: the one asking).  None is the main thread, which an Error says
 * cannot be removed -- and then the code goes on with the handle ebx held as
 * it was called (a bug of the game's, kept).  No caller in the game. */
void removeThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    if (cpu.eax == 0)
    {
        push(app, cpu, cpu.esi);
        cpu.edx = kThreadsFile;
        cpu.ecx = kRemoveThreadName;
        cpu.esi = kRemoveThreadLine;
        push(app, cpu, kRemoveThreadText);
        put(app, kWhereFile, cpu.edx);
        put(app, kWhereFunction, cpu.ecx);
        put(app, kWhereLine, cpu.esi);
        if (!threadsError(app, cpu))
            return;
        cpu.esi = pop(app, cpu);
    }
    else if (cpu.eax == 0xffffffff)
    {
        if (!import(app, cpu, kGetCurrentThread))
            return;
        cpu.ebx = cpu.eax;
    }
    else
    {
        cpu.ebx = at(app, cpu.eax + kHandle);
    }
    push(app, cpu, 0);
    push(app, cpu, cpu.ebx);
    if (!import(app, cpu, kTerminateThread))
        return;
    push(app, cpu, cpu.ebx);
    if (!import(app, cpu, kCloseHandle))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df8e0: eax ms slept (SleepEx, alertable). */
void sleepMs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, 1);
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kSleepEx))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df8f0: eax of the game's ticks slept (ms: eax * 1000 / [0x5643d8], 100
 * a second where it is 0).  No caller in the game. */
void sleepTicks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    ticksToMs(app, cpu, cpu.eax);
    push(app, cpu, 1);
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kSleepEx))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df930: 1 if this is the thread of record eax (0: the main thread; -1:
 * any, so always), else 0 -- by its id. */
void isThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.ebx = cpu.eax;
    if (!import(app, cpu, kGetCurrentThreadId))
        return;
    cpu.ecx = at(app, kThreadsReady);
    cpu.edx = cpu.eax;
    if (cpu.ecx == 0)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, threadsInit))
            return;
    }
    if (cpu.ebx == 0)
        cpu.eax = cpu.edx == at(app, kMainThreadId) ? 1 : 0;
    else if (cpu.ebx == 0xffffffff)
        cpu.eax = 1;
    else
        cpu.eax = cpu.edx == at(app, cpu.ebx + kThreadId) ? 1 : 0;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4df980: the priority of the thread of record eax (0: the main thread;
 * -1: this one), EA's: Windows' -15, -2, -1, 0, 1, 2, 15 as -3 to 3, any
 * other as 0.  No caller in the game. */
void threadPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.edx = cpu.eax;
    if (at(app, kThreadsReady) == 0)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, threadsInit))
            return;
    }
    if (cpu.edx == 0)
    {
        cpu.eax = at(app, kMainThread);
    }
    else if (cpu.edx == 0xffffffff)
    {
        if (!import(app, cpu, kGetCurrentThread))
            return;
    }
    else
    {
        cpu.eax = at(app, cpu.edx + kHandle);
    }
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kGetThreadPriority))
        return;
    switch (sreg32(cpu.eax))
    {
    case -15: cpu.eax = reg32(-3); break;
    case -2: cpu.eax = reg32(-2); break;
    case -1: cpu.eax = reg32(-1); break;
    case 1: cpu.eax = 1; break;
    case 2: cpu.eax = 2; break;
    case 15: cpu.eax = 3; break;
    default: cpu.eax = 0; break;
    }
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfa30: the thread of record eax (0: the main thread; -1: this one) given
 * EA's priority edx, -3 to 3 (Windows' -15, -2, -1, 0, 1, 2, 15; any other
 * 0) -- by the code's table at 0x4dfa10.  eax left what SetThreadPriority
 * gives back. */
void setThreadPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    if (at(app, kThreadsReady) == 0)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, threadsInit))
            return;
    }
    if (cpu.ecx == 0)
    {
        cpu.ecx = at(app, kMainThread);
    }
    else if (cpu.ecx == 0xffffffff)
    {
        if (!import(app, cpu, kGetCurrentThread))
            return;
        cpu.ecx = cpu.eax;
    }
    else
    {
        cpu.ecx = at(app, cpu.ecx + kHandle);
    }
    cpu.eax = cpu.ebx + 3;
    // The table's targets: 0x4dfa58, 0x4dfad4, 0x4dfac3, 0x4dfab5, 0x4dfaae,
    // 0x4dfaa7, 0x4dfaa0; past it (ja), 0x4dfab5.
    switch (cpu.eax)
    {
    case 0: cpu.eax = reg32(-15); break;
    case 1: cpu.eax = reg32(-2); break;
    case 2: cpu.eax = reg32(-1); break;
    case 4: cpu.eax = 1; break;
    case 5: cpu.eax = 2; break;
    case 6: cpu.eax = 15; break;
    default: cpu.eax = 0; break;
    }
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.ecx);
    if (!import(app, cpu, kSetThreadPriority))
        return;
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfaf0: this thread ended (ExitThread(0)).  No caller in the game. */
void exitThread(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, 0);
    if (!import(app, cpu, kExitThread))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfb00: an event, set by one and reset as a wait takes it (auto-reset). */
void createEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    newEvent(app, cpu, 0);
}

/* 0x4dfb20: event edx set; eax (what the callers would post with it) not
 * kept.  It runs on into signalEvent. */
void postEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = cpu.edx;
    signalEvent(app, cpu);
}

/* 0x4dfb30: event eax set -- a second entry of postEvent's, past its first
 * instruction (and its nops), which the game calls too. */
void signalEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kSetEvent);
}

/* 0x4dfb40: 1 if event eax is set (taken, if auto-reset), else 0, without
 * waiting. */
void pollEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 4;
    put(app, cpu.esp, cpu.eax);
    cpu.edx = cpu.esp;
    cpu.eax = 1;
    cpu.ebx = 0;
    if (!direct(app, cpu, waitForObjects))
        return;
    cpu.eax = cpu.eax == at(app, cpu.esp) ? 1 : 0;
    cpu.esp += 4;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* waitEvent and waitManualEvent: event eax waited on for ever; it, or 0 if
 * the wait failed. */
void waitForever(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 4;
    put(app, cpu.esp, cpu.eax);
    cpu.ebx = kInfinite;
    cpu.edx = cpu.esp;
    cpu.eax = 1;
    if (!direct(app, cpu, waitForObjects))
        return;
    cpu.esp += 4;
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x4dfb70: event eax waited on for ever: it, or 0 if the wait failed. */
void waitEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    waitForever(app, cpu);
}

/* 0x4dfb90: 1 if event eax is set within edx of the game's ticks, else 0.
 * edx left as the wait leaves it.  No caller in the game. */
void waitEventTicks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    cpu.esp -= 4;
    put(app, cpu.esp, cpu.eax);
    ticksToMs(app, cpu, cpu.edx);
    cpu.edx = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.eax = 1;
    if (!direct(app, cpu, waitForObjects))
        return;
    cpu.eax = cpu.eax == at(app, cpu.esp) ? 1 : 0;
    cpu.esp += 4;
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfbf0: eax handles at edx waited on for ever: the one set, or 0. */
void waitForAny(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.ebx = kInfinite;
    if (!direct(app, cpu, waitForObjects))
        return;
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfc00: handle eax closed -- an event's, a thread's. */
void closeEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kCloseHandle);
}

/* 0x4dfc10: an event that stays set until it is reset (manual-reset). */
void createManualEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    newEvent(app, cpu, 1);
}

/* 0x4dfc30: event eax set -- the same code as signalEvent's.  No caller in
 * the game. */
void setEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kSetEvent);
}

/* 0x4dfc40: event eax reset.  No caller in the game. */
void resetEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kResetEvent);
}

/* 0x4dfc50: event eax pulsed: set, those waiting let go, reset. */
void pulseEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kPulseEvent);
}

/* 0x4dfc60: event eax waited on for ever -- waitEvent's code again, for the
 * manual events. */
void waitManualEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    waitForever(app, cpu);
}

/* 0x4dfc80: event eax set, and its handle closed. */
void setAndCloseEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.ebx = cpu.eax;
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kSetEvent))
        return;
    push(app, cpu, cpu.ebx);
    if (!import(app, cpu, kCloseHandle))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfca0: a stub of EA's: the error's code (0x9ef5bc's low word, which
 * nothing sets) back, and with eax a buffer "[%5d] sorry, not implemented"
 * printed into it.  No caller in the game. */
void lastErrorText(win32::WinApplication* app, x86::CPU& cpu)
{
    if (cpu.eax == 0)
    {
        cpu.eax = at(app, kErrorCode) & 0xffff;
        cpu.esp += 4;
        return;
    }
    push(app, cpu, cpu.edx);
    cpu.edx = at(app, kErrorCode) & 0xffff;
    push(app, cpu, cpu.edx);
    push(app, cpu, kNotImplemented);
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.edx = pop(app, cpu);
    cpu.eax = at(app, kErrorCode) & 0xffff;
    cpu.esp += 4;
}

/* 0x4dfce0: Windows' last error set to eax.  No caller in the game. */
void setLastError(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kSetLastError);
}

/* 0x4dfcf0: Windows' last error. */
void lastError(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    if (!import(app, cpu, kGetLastError))
        return;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dfd00: nothing (a bare ret).  No caller in the game. */
void doNothing(win32::WinApplication*, x86::CPU& cpu)
{
    cpu.esp += 4;
}

/* 0x4dfd80: the matrix at eax (3 by 3 floats, by rows) a rotation about x by
 * the angle at [esp + 4] (in the game's units, as 0x4ea970 takes them).  eax
 * left the cosine's bits. */
void xRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!rotationStart(app, cpu))
        return;
    put(app, cpu.ecx, kOne);
    put(app, cpu.ecx + 4, 0);
    put(app, cpu.ecx + 8, 0);
    put(app, cpu.ecx + 0xc, 0);
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx + 0x10, cpu.eax);
    cpu.eax = at(app, cpu.esp);
    put(app, cpu.ecx + 0x18, 0);
    put(app, cpu.ecx + 0x14, cpu.eax);
    cpu.fpu.push(loadFloat(app, cpu.esp));
    cpu.fpu.st(0) = -cpu.fpu.st(0);
    storePop(app, cpu, cpu.ecx + 0x1c);
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx + 0x20, cpu.eax);
    rotationEnd(app, cpu);
}

/* 0x4dfde0: the matrix at eax a rotation about y by the angle at [esp + 4].
 * eax left the cosine's bits. */
void yRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!rotationStart(app, cpu))
        return;
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx + 4, 0);
    put(app, cpu.ecx, cpu.eax);
    cpu.fpu.push(loadFloat(app, cpu.esp));
    put(app, cpu.ecx + 0xc, 0);
    put(app, cpu.ecx + 0x10, kOne);
    put(app, cpu.ecx + 0x14, 0);
    cpu.fpu.st(0) = -cpu.fpu.st(0);
    storePop(app, cpu, cpu.ecx + 8);
    cpu.eax = at(app, cpu.esp);
    put(app, cpu.ecx + 0x1c, 0);
    put(app, cpu.ecx + 0x18, cpu.eax);
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx + 0x20, cpu.eax);
    rotationEnd(app, cpu);
}

/* 0x4dfe40: the matrix at eax a rotation about z by the angle at [esp + 4].
 * eax left the cosine's bits. */
void zRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!rotationStart(app, cpu))
        return;
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx, cpu.eax);
    cpu.eax = at(app, cpu.esp);
    put(app, cpu.ecx + 8, 0);
    put(app, cpu.ecx + 4, cpu.eax);
    cpu.fpu.push(loadFloat(app, cpu.esp));
    cpu.fpu.st(0) = -cpu.fpu.st(0);
    storePop(app, cpu, cpu.ecx + 0xc);
    cpu.eax = at(app, cpu.esp + 4);
    put(app, cpu.ecx + 0x14, 0);
    put(app, cpu.ecx + 0x18, 0);
    put(app, cpu.ecx + 0x1c, 0);
    put(app, cpu.ecx + 0x20, kOne);
    put(app, cpu.ecx + 0x10, cpu.eax);
    rotationEnd(app, cpu);
}

/* 0x4dfea0: the matrix at [esp + 8] a rotation by the angle at [esp + 4]
 * about the axis at eax (normalized first, 0x4e01c0): with s and c its sine
 * and cosine, t 1 - c, x y z the axis, t x x + c, t x y + s z, t x z - s y;
 * t x y - s z, t y y + c, t y z + s x; t x z + s y, t y z - s x, t z z + c.
 * An angle of 0 (either sign) gives the identity (0x564498).  ret 8.  No
 * caller in the game. */
void axisRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    cpu.esp -= 0x30;
    cpu.edi = at(app, cpu.esp + 0x44);
    cpu.ecx = cpu.eax;
    if ((at(app, cpu.esp + 0x40) & 0x7fffffff) == 0)
    {
        // rep movsd: the identity's 9 dwords.
        cpu.ecx = 9;
        cpu.esi = kIdentity;
        for (; cpu.ecx != 0; --cpu.ecx)
        {
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.edi += 4;
            cpu.esi += 4;
        }
        cpu.esp += 0x30;
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4 + 8;
        return;
    }
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebx);
    cpu.ebx = cpu.esp + 0x34;
    cpu.edx = cpu.esp + 0x14;
    cpu.eax = cpu.esp + 0x48;
    if (!byAddress(app, cpu, kSineCosine))
        return;
    cpu.edx = cpu.esp + 8;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kNormalized))
        return;
    // The frame: the axis at esp + 8, + 0xc, + 0x10; the sine at esp + 0x14,
    // t x at esp + 0x1c, the cosine at esp + 0x34.
    const reg32 x = cpu.esp + 8, y = cpu.esp + 0xc, z = cpu.esp + 0x10;
    const reg32 sine = cpu.esp + 0x14, tx = cpu.esp + 0x1c, cosine = cpu.esp + 0x34;
    const auto timesTop = [&](reg32 at) { fpu.st(0) = fp::mul(fpu.st(0), loadFloat(app, at)); };
    fpu.push(loadFloat(app, sine));
    timesTop(x);
    fpu.push(loadFloat(app, sine));
    timesTop(y);
    fpu.push(loadFloat(app, sine));
    timesTop(z);
    fpu.push(1.0);
    fpu.st(0) = fp::sub(fpu.st(0), loadFloat(app, cosine));
    fpu.push(fpu.st(0));
    timesTop(x);
    fpu.push(fpu.st(1));
    timesTop(y);
    exchange(cpu, 1);
    storeFloat(app, tx, fpu.st(0));                          // fst
    timesTop(x);
    exchange(cpu, 2);
    timesTop(z);
    exchange(cpu, 2);
    fpu.st(0) = fp::add(fpu.st(0), loadFloat(app, cosine));
    fpu.push(loadFloat(app, tx));
    exchange(cpu, 1);
    storePop(app, cpu, cpu.edi);
    timesTop(y);
    fpu.st(0) = fp::add(fpu.st(0), fpu.st(3));               // fadd st(3)
    storePop(app, cpu, cpu.edi + 4);
    fpu.push(loadFloat(app, tx));
    timesTop(z);
    fpu.st(0) = fp::sub(fpu.st(0), fpu.st(4));               // fsub st(4)
    fpu.push(fpu.st(1));
    exchange(cpu, 1);
    storePop(app, cpu, cpu.edi + 8);
    timesTop(x);
    fpu.st(3) = fp::sub(fpu.st(0), fpu.st(3));               // fsubrp st(3)
    fpu.pop();
    fpu.push(fpu.st(0));
    exchange(cpu, 3);
    storePop(app, cpu, cpu.edi + 0xc);
    exchange(cpu, 2);
    timesTop(y);
    fpu.st(0) = fp::add(fpu.st(0), loadFloat(app, cosine));
    storePop(app, cpu, cpu.edi + 0x10);
    exchange(cpu, 1);
    timesTop(z);
    fpu.st(0) = fp::add(fpu.st(0), fpu.st(3));               // fadd st(3)
    fpu.push(fpu.st(1));
    exchange(cpu, 1);
    storePop(app, cpu, cpu.edi + 0x14);
    timesTop(x);
    fpu.st(2) = fp::add(fpu.st(2), fpu.st(0));               // faddp st(2)
    fpu.pop();
    fpu.push(fpu.st(0));
    exchange(cpu, 2);
    storePop(app, cpu, cpu.edi + 0x18);
    exchange(cpu, 1);
    timesTop(y);
    fpu.st(2) = fp::sub(fpu.st(0), fpu.st(2));               // fsubrp st(2)
    fpu.pop();
    exchange(cpu, 1);
    storePop(app, cpu, cpu.edi + 0x1c);
    timesTop(z);
    fpu.st(0) = fp::add(fpu.st(0), loadFloat(app, cosine));
    storePop(app, cpu, cpu.edi + 0x20);
    cpu.ebx = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 0x30;
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x4eb230: a random fraction, 0 to 65535/65536 (eaRandom's low word over
 * 65536), on the x87's stack. */
void randomFraction(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.edx);
    cpu.esp -= 8;
    if (!byAddress(app, cpu, kEaRandom))
        return;
    cpu.edx = 0;
    cpu.eax &= 0xffff;
    put(app, cpu.esp + 4, cpu.edx);
    put(app, cpu.esp, cpu.eax);
    cpu.fpu.push(x86::Float(x86::sreg64(app->getMemory<x86::reg64>(cpu.esp))));
    cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), x86::Float(app->getMemory<double>(kOneIn65536)));
    cpu.esp += 8;
    cpu.edx = pop(app, cpu);
    cpu.eax = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4eb260: the free locks grown: [0x564a60] records' bytes of pages
 * (0x501110, which rounds them up to whole pages), each record tagged free
 * and linked to the one before -- the first to what was free -- the last the
 * list's head, and [0x564a60] as many as the pages held.  None got, the list
 * is left empty.  eax left the pages. */
void growLocks(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.esp -= 4;
    cpu.ebx = at(app, kFreeLocks);
    cpu.esi = at(app, kLocksToGrow);
    cpu.eax = cpu.esi << 5;
    put(app, cpu.esp, cpu.eax);
    cpu.eax = cpu.esp;
    cpu.ecx = cpu.ebx;
    if (!byAddress(app, cpu, kAllocatePages))
        return;
    cpu.esi = at(app, kLocksToGrow);
    cpu.ebx = cpu.eax;
    if (cpu.eax != 0)
    {
        push(app, cpu, cpu.edx);
        cpu.esi = at(app, cpu.esp + 4) >> 5;
        cpu.edx = 0;
        if (sreg32(cpu.esi) > 0)
        {
            do
            {
                put(app, cpu.ebx + kLockTag, kFree);
                ++cpu.edx;
                put(app, cpu.ebx + kLockLink, cpu.ecx);
                cpu.ecx = cpu.ebx;
                cpu.ebx += kLockBytes;
            } while (sreg32(cpu.edx) < sreg32(cpu.esi));
        }
        cpu.ebx = cpu.ecx;
        cpu.edx = pop(app, cpu);
    }
    put(app, kLocksToGrow, cpu.esi);
    put(app, kFreeLocks, cpu.ebx);
    cpu.esp += 4;
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4eb2e0: a lock made: a free record taken (the list grown if empty),
 * cleared and its critical section initialized.  0 if there was none. */
void makeLock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    if (at(app, kThreadsReady) == 0)
    {
        cpu.eax = 0;
        if (!direct(app, cpu, threadsInit))
            return;
    }
    if (at(app, kFreeLocks) == 0 && !direct(app, cpu, growLocks))
        return;
    cpu.ebx = at(app, kFreeLocks);
    if (cpu.ebx == 0)
    {
        cpu.eax = 0;
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
        return;
    }
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ecx);
    cpu.eax = at(app, cpu.ebx + kLockLink);
    cpu.edx = kSectionBytes;
    put(app, kFreeLocks, cpu.eax);
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kFillZero))
        return;
    push(app, cpu, cpu.ebx);
    if (!import(app, cpu, kInitializeCriticalSection))
        return;
    cpu.eax = cpu.ebx;
    cpu.ecx = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4eb340: lock eax taken (EnterCriticalSection).  No caller in the game. */
void enterLock(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kEnterCriticalSection);
}

/* 0x4eb350: lock eax given back (LeaveCriticalSection).  No caller in the
 * game. */
void leaveLock(win32::WinApplication* app, x86::CPU& cpu)
{
    ofHandle(app, cpu, kLeaveCriticalSection);
}

/* 0x4eb360: lock eax (none: nothing) deleted and its record put back on the
 * free list, tagged free.  eax left the list's head before. */
void freeLock(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    cpu.ebx = cpu.eax;
    if (cpu.eax == 0)
    {
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
        return;
    }
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.eax);
    if (!import(app, cpu, kDeleteCriticalSection))
        return;
    cpu.eax = at(app, kFreeLocks);
    put(app, cpu.ebx + kLockTag, kFree);
    put(app, kFreeLocks, cpu.ebx);
    put(app, cpu.ebx + kLockLink, cpu.eax);
    cpu.ecx = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4f9070: 1 if a DirectDraw surface is locked (the screen's, 0x9f3d0c, or
 * another's, 0x567180), else 0 -- DirectDraw's, not the threads': what
 * createThread asks. */
void surfaceLocked(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = at(app, kScreenLocked) != 0 || at(app, kSurfaceLocked) != 0 ? 1 : 0;
    cpu.esp += 4;
}

/* 0x5221f0: the machine's physical memory in bytes (GlobalMemoryStatus's
 * dwTotalPhys) -- the file system's, not the threads'. */
void physicalMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    cpu.esp -= 0x20;
    cpu.eax = cpu.esp;
    cpu.edx = 0x20;
    push(app, cpu, cpu.eax);
    put(app, cpu.esp + 4, cpu.edx);
    if (!import(app, cpu, kGlobalMemoryStatus))
        return;
    cpu.eax = at(app, cpu.esp + 8);
    cpu.esp += 0x20;
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace ea
}  // namespace nfs3hp
