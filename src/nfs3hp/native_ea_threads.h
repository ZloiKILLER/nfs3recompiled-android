#ifndef NFS3HP_NATIVE_EA_THREADS_H
#define NFS3HP_NATIVE_EA_THREADS_H

#include "native_guest.h"

/* native_ea_threads.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
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
// The game's 0x4dfb00: an event, auto-reset: its handle.
std::optional<x86::reg32> newAutoEvent(win32::WinApplication* app, x86::CPU& cpu);
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
// The game's 0x4dfcf0: Windows' last error (GetLastError).
std::optional<x86::reg32> lastErrorOf(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace ea

namespace guest
{

// The thread in eax, the stale handle in ebx; eax back.
template <>
struct ByAddress<ea::endThread>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 thread,
                                          x86::reg32 stale)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = thread;
        cpu.ebx = stale;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return cpu.eax;
    }
};

// The ms in eax; eax back.
template <>
struct ByAddress<ea::sleepFor>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 ms)
    {
        return watcomEax(app, cpu, Address, {ms});
    }
};

// The thread in eax; eax 1 if so, else 0.
template <>
struct ByAddress<ea::isThreadNow>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 thread)
    {
        const std::optional<x86::reg32> is = watcomEax(app, cpu, Address, {thread});
        if (!is)
            return std::nullopt;
        return *is != 0;
    }
};

// Nothing handed; the event back in eax.
template <>
struct ByAddress<ea::newAutoEvent>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The event in edx (eax as it is); eax back.
template <>
struct ByAddress<ea::eventPosted>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event)
    {
        const KeptRegisters kept(cpu);
        cpu.edx = event;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return cpu.eax;
    }
};

// The event in eax; eax back.
template <>
struct ByAddress<ea::eventSet>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event)
    {
        return watcomEax(app, cpu, Address, {event});
    }
};

// The event in eax; eax back.
template <>
struct ByAddress<ea::eventWaited>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event)
    {
        return watcomEax(app, cpu, Address, {event});
    }
};

// The matrix in eax, the angle pushed (stdcall, 1 word); eax back.
template <>
struct ByAddress<ea::rotateAboutX>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix,
                                          x86::reg32 angle)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = matrix;
        if (!guest::call(app, cpu, Address, {at(app, angle)}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The matrix in eax, the angle pushed (stdcall, 1 word); eax back.
template <>
struct ByAddress<ea::rotateAboutY>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix,
                                          x86::reg32 angle)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = matrix;
        if (!guest::call(app, cpu, Address, {at(app, angle)}))
            return std::nullopt;
        return cpu.eax;
    }
};

// Nothing handed; the fraction left on the x87's stack.
template <>
struct ByAddress<ea::randomFractionOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::Float> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomFloat(app, cpu, Address, {});
    }
};

// Nothing handed; the lock back in eax.
template <>
struct ByAddress<ea::newLock>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The lock in eax; eax back.
template <>
struct ByAddress<ea::lockFreed>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lock)
    {
        return watcomEax(app, cpu, Address, {lock});
    }
};

// Nothing handed; eax 1 if so, else 0.
template <>
struct ByAddress<ea::surfaceIsLocked>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        const std::optional<x86::reg32> locked = watcomEax(app, cpu, Address, {});
        return locked && *locked != 0;
    }
};

// Nothing handed; the bytes back in eax.
template <>
struct ByAddress<ea::physicalBytes>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The event in eax; Windows' answer back in eax, every other register kept.
template <>
struct ByAddress<ea::eventClosed>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 event)
    {
        return watcomEax(app, cpu, Address, {event});
    }
};

// Nothing handed; the error back in eax, every other register kept.
template <>
struct ByAddress<ea::lastErrorOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
