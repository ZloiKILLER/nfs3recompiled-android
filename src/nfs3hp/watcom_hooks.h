#ifndef NFS3HP_WATCOM_HOOKS_H
#define NFS3HP_WATCOM_HOOKS_H

#include "native_guest.h"

/* The Watcom runtime's calls through its hooks -- the pointers in its data it calls its locks, the thread's
 * data, the handles' allocation, the floats' formatter and its exit's steps through.  Each is a call through
 * the pointer (guest::pointerCall, native_watcom.cpp's hook<Slot>): the callee's thunk, whatever the pointer
 * holds.  The caller sets the registers the hook takes and reads what it leaves.
 *
 * A hook holds what the runtime's data has there from the start (nfs3.exe's DGROUP) or what its code puts
 * there, and nothing else: no other code of the game writes them.  The locks' hooks are __NullAccessRtn's
 * until the first _beginthread, which points them at the real locks (watc::initThreads, sub_51e964);
 * __GetThreadPtr at the thread list's lookup there too; the floats' formatter and scanner fpNotLoaded's until
 * the start-up's sub_4ff570 points them at the x87's (watb::initFloatFormatters); the thread start's hook
 * __NullAccessRtn's until sub_526420 hooks the signals in.  _exit's two hooks hold ea::doNothing, and
 * _matherr's the bare zero the linker gave it (watc::zeroJump): nothing sets them -- _set_matherr (0x52b641)
 * has no caller.
 *
 * Hooks with no target to know of stay calls through the pointer at their callers: the default windowing's
 * (0x5677b0 to 0x5677ec) and _exit's user hook (0x5677f0) and early one (0x5677ac), 0 from the start, never
 * set, and called only when they are not 0; the finalizers' and initializers' tables, atexit's, the scope
 * tables' handlers, the comparators and the scanf's and printf's character routines handed in. */
namespace nfs3hp
{

namespace watcom
{
namespace hooks
{

constexpr nfs::u32 kGetThreadPtr = 0x567764;   // __GetThreadPtr: the thread's data in eax
constexpr nfs::u32 kAccessFile = 0x567768;     // a file's lock taken, eax the handle
constexpr nfs::u32 kReleaseFile = 0x56776c;    // and given back
constexpr nfs::u32 kAllocHandle = 0x567770;    // __allocPOSIXHandle: eax Windows' HANDLE; the handle
constexpr nfs::u32 kFreeHandle = 0x567774;     // __freePOSIXHandle: eax the handle
constexpr nfs::u32 kAccessIob = 0x567778;      // the streams' lock taken
constexpr nfs::u32 kReleaseIob = 0x56777c;     // and given back
constexpr nfs::u32 kHeapLock = 0x567780;       // the heap's lock taken
constexpr nfs::u32 kHeapUnlock = 0x567788;     // and given back
constexpr nfs::u32 kAccessHandles = 0x567798;  // the handles' table's lock taken
constexpr nfs::u32 kReleaseHandles = 0x56779c; // and given back
constexpr nfs::u32 kExitDone = 0x5677a0;       // __exit's last step: the threads' end
constexpr nfs::u32 kThreadStart = 0x5677a4;    // a thread's start: eax what the initializers left
constexpr nfs::u32 kExitHook = 0x564484;       // _exit's and exit's first
constexpr nfs::u32 kExitHook2 = 0x564488;      // _exit's second
constexpr nfs::u32 kFloatFormat = 0x56ac64;    // __EFG_printf: eax the buffer, edx the arguments, ebx the SPECS
constexpr nfs::u32 kScanFloat = 0x56ac68;      // __EFG_scanf: eax the text, edx where the double goes
constexpr nfs::u32 kMatherr = 0x56b16c;        // _matherr: eax and ebx the struct exception

}  // namespace hooks
}  // namespace watcom

}  // namespace nfs3hp

#endif
