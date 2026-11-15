#ifndef NFS3HP_WATCOM_HOOKS_H
#define NFS3HP_WATCOM_HOOKS_H

#include "native_guest.h"

#if defined(NFS_GAME_CHECKS) || defined(NFS_FRONTEND_CHECKS)
#define NFS3HP_RUNTIME_THROUGH_HOOKS 1
#else
#include "native_frontend_watc.h"
#endif

/* The Watcom runtime's calls through its hooks -- the pointers in its data it
 * calls its locks, the thread's data, the handles' allocation, the floats'
 * formatter and its exit's steps through -- as C++ calls of the functions
 * they can hold: plan step 4's, for the runtime.
 *
 *     cpu.eax = handle;
 *     guest::runtimeHook<watcom::hooks::kAccessFile>(app, cpu)
 *
 * A hook holds what the runtime's data has there from the start (nfs3.exe's
 * DGROUP) or what its code puts there, and nothing else: no other code of the
 * game writes them.  The locks' hooks are __NullAccessRtn's until the first
 * _beginthread, which points them at the real locks (watc::initThreads,
 * sub_51e964); __GetThreadPtr at the thread list's lookup there too; the
 * floats' formatter and scanner fpNotLoaded's until the start-up's
 * sub_4ff570 points them at the x87's (watb::initFloatFormatters); the thread
 * start's hook __NullAccessRtn's until sub_526420 hooks the signals in.
 * _exit's two hooks hold ea::doNothing, and _matherr's the bare zero the
 * linker gave it (watc::zeroJump): nothing sets them -- _set_matherr
 * (0x52b641) has no caller.
 *
 * The caller sets the registers the hook takes, as for the call through the
 * pointer, and reads what it leaves.  In the game, when the pointer leads to
 * one of those -- the native registered there -- that function's work is
 * done as C++: nothing for __NullAccessRtn and doNothing, the single
 * thread's data for __GetThreadPtr, the typed core of a lock that only names
 * its semaphore and jumps on, __allocPOSIXHandle or __freePOSIXHandle for the
 * jumps to them, and otherwise the registered native itself, called as C++
 * with the registers it takes; every register and eax left as the call
 * through the pointer leaves them.  A pointer to anything else is called as
 * the code calls it.  In tools/game_checks (NFS_GAME_CHECKS) and
 * tools/frontend_checks (NFS_FRONTEND_CHECKS) the call always goes through
 * the pointer, for the checks' stand-ins.
 *
 * Either way the return address's slot below esp while it runs and esp back
 * where it was after, as guest::call; false when the game stopped in it.
 *
 * Hooks with no target to know of stay calls through the pointer at their
 * callers: the default windowing's (0x5677b0 to 0x5677ec) and _exit's user
 * hook (0x5677f0) and early one (0x5677ac), 0 from the start, never set, and
 * called only when they are not 0; the finalizers' and initializers' tables,
 * atexit's, the scope tables' handlers, the comparators and the scanf's and
 * printf's character routines handed in. */
namespace nfs3hp
{

/* The natives a hook can lead to (disassembly/nfs3hp.h). */
namespace watcom
{
void GetThreadPtr(win32::WinApplication* app, x86::CPU& cpu);
void NullAccess(win32::WinApplication* app, x86::CPU& cpu);
void allocPOSIXHandle(win32::WinApplication* app, x86::CPU& cpu);
void freePOSIXHandle(win32::WinApplication* app, x86::CPU& cpu);
}  // namespace watcom
namespace ea
{
void doNothing(win32::WinApplication* app, x86::CPU& cpu);
}  // namespace ea
namespace frontend
{
void watbAllocHandleJump(win32::WinApplication* app, x86::CPU& cpu);
void watbFreeHandleJump(win32::WinApplication* app, x86::CPU& cpu);
void watcFpNotLoaded(win32::WinApplication* app, x86::CPU& cpu);
void watcLockStreams(win32::WinApplication* app, x86::CPU& cpu);
void watcUnlockStreams(win32::WinApplication* app, x86::CPU& cpu);
void watcLockFile(win32::WinApplication* app, x86::CPU& cpu);
void watcUnlockFile(win32::WinApplication* app, x86::CPU& cpu);
void watcAllocHandleHook(win32::WinApplication* app, x86::CPU& cpu);
void watcFreeHandleHook(win32::WinApplication* app, x86::CPU& cpu);
void watcLockHeap(win32::WinApplication* app, x86::CPU& cpu);
void watcUnlockHeap(win32::WinApplication* app, x86::CPU& cpu);
void watcLockHandleTable(win32::WinApplication* app, x86::CPU& cpu);
void watcUnlockHandleTable(win32::WinApplication* app, x86::CPU& cpu);
void watcThreadData(win32::WinApplication* app, x86::CPU& cpu);
void watcEndThreads(win32::WinApplication* app, x86::CPU& cpu);
void watcFormatFloat(win32::WinApplication* app, x86::CPU& cpu);
void watcScanFloat(win32::WinApplication* app, x86::CPU& cpu);
void watcInitThreadSignals(win32::WinApplication* app, x86::CPU& cpu);
void watcZeroJump(win32::WinApplication* app, x86::CPU& cpu);
}  // namespace frontend

namespace watcom
{
namespace hooks
{

constexpr x86::reg32 kGetThreadPtr = 0x567764;   // __GetThreadPtr: the thread's data in eax
constexpr x86::reg32 kAccessFile = 0x567768;     // a file's lock taken, eax the handle
constexpr x86::reg32 kReleaseFile = 0x56776c;    // and given back
constexpr x86::reg32 kAllocHandle = 0x567770;    // __allocPOSIXHandle: eax Windows' HANDLE; the handle
constexpr x86::reg32 kFreeHandle = 0x567774;     // __freePOSIXHandle: eax the handle
constexpr x86::reg32 kAccessIob = 0x567778;      // the streams' lock taken
constexpr x86::reg32 kReleaseIob = 0x56777c;     // and given back
constexpr x86::reg32 kHeapLock = 0x567780;       // the heap's lock taken
constexpr x86::reg32 kHeapUnlock = 0x567788;     // and given back
constexpr x86::reg32 kAccessHandles = 0x567798;  // the handles' table's lock taken
constexpr x86::reg32 kReleaseHandles = 0x56779c; // and given back
constexpr x86::reg32 kExitDone = 0x5677a0;       // __exit's last step: the threads' end
constexpr x86::reg32 kThreadStart = 0x5677a4;    // a thread's start: eax what the initializers left
constexpr x86::reg32 kExitHook = 0x564484;       // _exit's and exit's first
constexpr x86::reg32 kExitHook2 = 0x564488;      // _exit's second
constexpr x86::reg32 kFloatFormat = 0x56ac64;    // __EFG_printf: eax the buffer, edx the arguments, ebx the SPECS
constexpr x86::reg32 kScanFloat = 0x56ac68;      // __EFG_scanf: eax the text, edx where the double goes
constexpr x86::reg32 kMatherr = 0x56b16c;        // _matherr: eax and ebx the struct exception

}  // namespace hooks
}  // namespace watcom

namespace guest
{

/* What a hook can hold: the native registered there and its work as C++,
 * the hook's registers in cpu and esp at the caller's (above). */
template <x86::reg32 Slot>
struct RuntimeHook;

#ifndef NFS3HP_RUNTIME_THROUGH_HOOKS

namespace runtime
{

using App = win32::WinApplication;
using CPU = x86::CPU;

/* `run` as the call runs its callee: the return address's slot below esp,
 * esp back after, as guest::call. */
template <typename Run>
inline bool asCalled(CPU& cpu, Run run)
{
    const x86::reg32 esp = cpu.esp;
    cpu.esp = esp - 4;
    run();
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    return true;
}

/* __NullAccessRtn (0x4ff0f4), doNothing (0x4dfd00): nothing. */
inline bool nothing(App*, CPU&)
{
    return true;
}

/* __GetThreadPtr (0x4ff0ec): the single thread's data. */
inline bool singleThreadData(App* app, CPU& cpu)
{
    cpu.eax = at(app, 0x9f642c);
    return true;
}

/* A hook's target that only names something and jumps on -- a lock's
 * (sub_51e680 to sub_51e6e4) to the lock or its release, _matherr's to its
 * zero: its core, written register by register, which takes the hook's eax
 * if it reads it. */
template <x86::reg32 (*Core)(App*, CPU&)>
inline bool jumpOf(App* app, CPU& cpu)
{
    return asCalled(cpu, [&] { Core(app, cpu); });
}

template <x86::reg32 (*Core)(App*, CPU&, x86::reg32)>
inline bool jumpOfEax(App* app, CPU& cpu)
{
    return asCalled(cpu, [&] { Core(app, cpu, cpu.eax); });
}

/* A native called as C++ with the registers it takes (__allocPOSIXHandle and
 * __freePOSIXHandle for the jumps to them, and the rest of the runtime's
 * that a hook leads to), as guest::call calls one. */
template <Native Function>
inline bool native(App* app, CPU& cpu)
{
    return call(app, cpu, Function);
}

template <Native Registered, bool (*Run)(App*, CPU&)>
struct Target
{
    static bool run(Native method, App* app, CPU& cpu, bool& ok)
    {
        if (method != Registered)
            return false;
        ok = Run(app, cpu);
        return true;
    }
};

template <typename... T>
struct Targets
{
    /* The one the hook leads to run, if it is one of them: true, and `ok`
     * false when the game stopped in it. */
    static bool run(Native method, App* app, CPU& cpu, bool& ok)
    {
        return (T::run(method, app, cpu, ok) || ...);
    }
};

}  // namespace runtime

#define NFS3HP_RUNTIME_HOOK(slot, ...)                                   \
    template <>                                                          \
    struct RuntimeHook<watcom::hooks::slot>                              \
    {                                                                    \
        using targets = runtime::Targets<__VA_ARGS__>;                   \
    }
#define NFS3HP_HOOK_TARGET(registered, run) runtime::Target<&registered, &run>
#define NFS3HP_HOOK_NULL NFS3HP_HOOK_TARGET(watcom::NullAccess, runtime::nothing)

NFS3HP_RUNTIME_HOOK(kGetThreadPtr, NFS3HP_HOOK_TARGET(watcom::GetThreadPtr, runtime::singleThreadData),
                    NFS3HP_HOOK_TARGET(frontend::watcThreadData, runtime::native<frontend::watcThreadData>));
NFS3HP_RUNTIME_HOOK(kAccessFile, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcLockFile, runtime::jumpOfEax<frontend::watc::lockFile>));
NFS3HP_RUNTIME_HOOK(kReleaseFile, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcUnlockFile, runtime::jumpOfEax<frontend::watc::unlockFile>));
NFS3HP_RUNTIME_HOOK(kAllocHandle,
                    NFS3HP_HOOK_TARGET(frontend::watbAllocHandleJump, runtime::native<watcom::allocPOSIXHandle>),
                    NFS3HP_HOOK_TARGET(frontend::watcAllocHandleHook, runtime::native<watcom::allocPOSIXHandle>));
NFS3HP_RUNTIME_HOOK(kFreeHandle,
                    NFS3HP_HOOK_TARGET(frontend::watbFreeHandleJump, runtime::native<watcom::freePOSIXHandle>),
                    NFS3HP_HOOK_TARGET(frontend::watcFreeHandleHook, runtime::native<frontend::watcFreeHandleHook>));
NFS3HP_RUNTIME_HOOK(kAccessIob, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcLockStreams, runtime::jumpOf<frontend::watc::lockStreams>));
NFS3HP_RUNTIME_HOOK(kReleaseIob, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcUnlockStreams, runtime::jumpOf<frontend::watc::unlockStreams>));
NFS3HP_RUNTIME_HOOK(kHeapLock, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcLockHeap, runtime::jumpOf<frontend::watc::lockHeap>));
NFS3HP_RUNTIME_HOOK(kHeapUnlock, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcUnlockHeap, runtime::jumpOf<frontend::watc::unlockHeap>));
NFS3HP_RUNTIME_HOOK(kAccessHandles, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcLockHandleTable, runtime::native<frontend::watcLockHandleTable>));
NFS3HP_RUNTIME_HOOK(kReleaseHandles, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcUnlockHandleTable,
                                       runtime::native<frontend::watcUnlockHandleTable>));
NFS3HP_RUNTIME_HOOK(kExitDone, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcEndThreads, runtime::native<frontend::watcEndThreads>));
NFS3HP_RUNTIME_HOOK(kThreadStart, NFS3HP_HOOK_NULL,
                    NFS3HP_HOOK_TARGET(frontend::watcInitThreadSignals,
                                       runtime::native<frontend::watcInitThreadSignals>));
NFS3HP_RUNTIME_HOOK(kExitHook, NFS3HP_HOOK_TARGET(ea::doNothing, runtime::nothing));
NFS3HP_RUNTIME_HOOK(kExitHook2, NFS3HP_HOOK_TARGET(ea::doNothing, runtime::nothing));
NFS3HP_RUNTIME_HOOK(kFloatFormat,
                    NFS3HP_HOOK_TARGET(frontend::watcFpNotLoaded, runtime::native<frontend::watcFpNotLoaded>),
                    NFS3HP_HOOK_TARGET(frontend::watcFormatFloat, runtime::native<frontend::watcFormatFloat>));
NFS3HP_RUNTIME_HOOK(kScanFloat,
                    NFS3HP_HOOK_TARGET(frontend::watcFpNotLoaded, runtime::native<frontend::watcFpNotLoaded>),
                    NFS3HP_HOOK_TARGET(frontend::watcScanFloat, runtime::native<frontend::watcScanFloat>));
NFS3HP_RUNTIME_HOOK(kMatherr, NFS3HP_HOOK_TARGET(frontend::watcZeroJump, runtime::jumpOf<frontend::watc::zeroJump>));

#undef NFS3HP_HOOK_NULL
#undef NFS3HP_HOOK_TARGET
#undef NFS3HP_RUNTIME_HOOK

#endif

/* The hook at Slot called, the registers it takes already set (above). */
template <x86::reg32 Slot>
inline bool runtimeHook(win32::WinApplication* app, x86::CPU& cpu)
{
#ifndef NFS3HP_RUNTIME_THROUGH_HOOKS
    bool ok = true;
    if (RuntimeHook<Slot>::targets::run(app->methodAt(at(app, Slot)), app, cpu, ok))
        return ok;
#endif
    return call(app, cpu, at(app, Slot));
}

}  // namespace guest
}  // namespace nfs3hp

#endif
