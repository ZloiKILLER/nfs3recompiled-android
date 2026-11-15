#include "native_frontend_watc.h"
#include "native_game.h"
#include "native_watcom.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_wina.h"
#include "native_frontend.h"
#include "native_windows.h"
#include "host_threads.h"
#include <nfs3hp.h>
#include <winapi/kernel32.h>

#include <array>
#include <optional>
#include <utility>

/* The rest of the C runtime, decompiled, with what EA's libraries and the linker put between its
 * modules:
 *
 *   start and end    the handle table, the C_FILE_INFO the parent hands down, the environment,
 *                    setmbcp, the semaphores and thread data the runtime's locks live on;
 *   threads          _beginthread and its start, the list of threads' data;
 *   signals          the x87's and the console's exceptions turned into raise, and the report
 *                    written to stderr when nothing takes them;
 *   numbers          strtol, the float formatter's hook;
 *   TAPI's stubs     the delay-loaded functions of TAPI32.DLL the modem code calls;
 *   drawing          writepixel, the blending putpixel, the 4-bit expansion, shaperowbytes, and
 *                    the debug screen's text, boxes and colours (cmn\monomem.c).
 *
 * The state stays in guest memory.  A Watcom function saves only the registers it says it does,
 * and callers may go on with what is left in the others, so each Site restores what its function
 * pushes, no more, and what a function leaves behind in the others is left as its code leaves it.
 * Windows' functions are the port's own, called as C++ (callImport); in the checks through the
 * import table's slots. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::reg32;
using x86::sreg32;

constexpr std::uint8_t kEbx = fe::kRestoreEbx, kEcx = fe::kRestoreEcx, kEdx = fe::kRestoreEdx;
constexpr std::uint8_t kEsi = fe::kRestoreEsi, kEdi = fe::kRestoreEdi;

fe::Site makeSite(const char* name, reg32 stackBytes, reg32 frameBytes, std::uint8_t restores)
{
    fe::Site site{};
    site.name = name;
    site.stackBytes = stackBytes;
    site.frameBytes = frameBytes;
    site.restores = restores;
    return site;
}

sreg32 s(reg32 value)
{
    return sreg32(value);
}

reg32 sar(reg32 value, unsigned bits)
{
    return reg32(sreg32(value) >> bits);
}

/* A jump to another function: it returns to our caller. */
void tail(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    app->dynamic_call(target, cpu);
}

/* The inline memcpy: dwords, then what is left, upwards. */
void copyUp(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count >> 2; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
    const reg32 done = count & ~reg32(3);
    for (reg32 i = done; i < count; ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

// The import table's slots: Windows' functions, stdcall.
constexpr reg32 kCloseHandle = 0x534488, kCreateEvent = 0x534494, kCreateThread = 0x5344a4;
constexpr reg32 kDeleteCriticalSection = 0x5344ac, kEnterCriticalSection = 0x5344b8, kExitProcess = 0x5344c0;
constexpr reg32 kExitThread = 0x5344c4, kFreeEnvironmentStrings = 0x5344d8, kGetAcp = 0x5344e0, kGetCpInfo = 0x5344e4;
constexpr reg32 kGetCurrentThreadId = 0x53450c, kGetCurrentThread = 0x534510, kGetLastError = 0x534534;
constexpr reg32 kGetOemCp = 0x534550, kGetProcAddress = 0x534558, kInitializeCriticalSection = 0x534584;
constexpr reg32 kLeaveCriticalSection = 0x534588, kLoadLibrary = 0x53458c, kSetConsoleCtrlHandler = 0x5345cc;
constexpr reg32 kSetEnvironmentVariable = 0x5345dc, kSetEvent = 0x5345e8, kSetLastError = 0x5345f4, kSetStdHandle = 0x5345f8;
constexpr reg32 kTlsAlloc = 0x53460c, kTlsFree = 0x534610, kTlsGetValue = 0x534614, kTlsSetValue = 0x534618;
constexpr reg32 kUnhandledExceptionFilter = 0x53461c, kWaitForSingleObject = 0x534634, kWriteFile = 0x534640;

/* A Windows function, its words as the code pushes them: the port's own, called as C++
 * (native_windows.h); in the checks through its import's slot, as a call of the guest's.  Its
 * answer, eax; the game stopped in it ends the stand-in as Guest::call's do. */
template <reg32 Slot, auto Function, typename... Words>
reg32 callImport(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

// The runtime's functions.
constexpr reg32 kMemset = 0x4e0640, kStrncpy = 0x4e0e30, kToupper = 0x4eede0, kNmalloc = 0x4f7900, kNfree = 0x4f79f0;
constexpr reg32 kSetErrno = 0x502880, kFatalError = 0x511d0c, kNrealloc = 0x5185c0, kMbslen = 0x518050;
constexpr reg32 kMemmove = 0x518640, kCalloc = 0x525df0, kVsprintf = 0x4df674;

// EA's library.
constexpr reg32 kError = 0x401010, kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kTextStart = 0x52e420, kBitmapToScreen = 0x4ead90, kRgbToColour = 0x4ef760;

// The runtime's data.
constexpr reg32 kThreadDataGetter = 0x567764;  // the current thread's data
constexpr reg32 kLockThreads = 0x567790, kUnlockThreads = 0x567794, kLockHandles = 0x567798, kUnlockHandles = 0x56779c;
constexpr reg32 kThreadOnStart = 0x5677a4, kThreadOnEnd = 0x5677a8;
constexpr reg32 kTlsIndex = 0x567760, kEnvironmentBlock = 0x567839;
constexpr reg32 kHandleTable = 0x56acbc, kHandleCount = 0x56acc0;
constexpr reg32 kEnviron = 0xa0b154, kEnvironFlags = 0xa0b150;
constexpr reg32 kThreadDataSize = 0x56af70, kThreadList = 0xa18250;
constexpr reg32 kSignalStopped = 0xa0ba94;

// The runtime's locks: a semaphore each, and the one that guards their creation.
constexpr reg32 kLockStreams = 0xa0b320, kLockFileTable = 0xa0b330, kLockFiles = 0xa0b340, kLockHeap = 0xa0b440;
constexpr reg32 kLockThreadList = 0xa0ba60, kLockHandleTable = 0xa0ba70, kLockOfLocks = 0xa0ba50;
constexpr reg32 kStaticSemaphores = 0xa0b450, kStaticSemaphoreCount = 0xa0ba80;
constexpr reg32 kExtraSemaphoreCount = 0xa0ba84, kExtraSemaphores = 0xa0ba88;

// The current window, as the library keeps it.
constexpr reg32 kClipLeft = 0x565000, kClipTop = 0x565004, kClipRight = 0x565008, kClipBottom = 0x56500c;
constexpr reg32 kWindowDepth = 0x565010, kWindowFlag = 0x565012, kWindowPixels = 0x565014;
constexpr reg32 kRowOffsets = 0x565020, kColumnOffsets = 0x565024;
constexpr reg32 kScreenWidth = 0x564ff8, kScreenHeight = 0x564ffc;

/* A semaphore of the runtime's: its lock taken (sub_51e71c's body). */
reg32 lockSemaphore(Guest& guest, x86::CPU& cpu, reg32 semaphore)
{
    const reg32 thread = callImport<kGetCurrentThreadId, &host::currentThreadId>(guest, cpu);
    if (thread != guest.dword(semaphore + 8))
    {
        if (guest.dword(semaphore + 4) == 0)
        {
            guest::direct<0x51e71c, frontend::watc::lock>(guest.app(), cpu, kLockOfLocks);
            if (guest.dword(semaphore + 4) == 0)
            {
                const reg32 made = guest::direct<0x51e544, frontend::watc::newSemaphore>(guest.app(), cpu);
                guest.setDword(semaphore + 4, 1);
                guest.setDword(semaphore, made);
            }
            guest::direct<0x51e780, frontend::watc::unlock>(guest.app(), cpu, kLockOfLocks);
        }
        callImport<kEnterCriticalSection, &host::enterLock>(guest, cpu, guest.dword(semaphore));
        guest.setDword(semaphore + 8, thread);
    }
    guest.setDword(semaphore + 0xc, guest.dword(semaphore + 0xc) + 1);
    return guest.eax();
}

/* And given back (sub_51e780's body). */
reg32 unlockSemaphore(Guest& guest, x86::CPU& cpu, reg32 semaphore)
{
    const reg32 holds = guest.dword(semaphore + 0xc);
    if (holds != 0)
    {
        guest.setDword(semaphore + 0xc, holds - 1);
        if (holds - 1 == 0)
        {
            const reg32 section = guest.dword(semaphore);
            guest.setDword(semaphore + 8, 0);
            return callImport<kLeaveCriticalSection, &host::leaveLock>(guest, cpu, section);
        }
    }
    return semaphore;
}

/* sub_51e940's body: the thread data's TLS index given back. */
reg32 giveBackTlsIndex(Guest& guest, x86::CPU& cpu)
{
    const reg32 index = guest.dword(kTlsIndex);
    if (index != 0xffffffff)
    {
        callImport<kTlsFree, &host::tlsFree>(guest, cpu, index);
        guest.setDword(kTlsIndex, 0xffffffff);
    }
    return guest.eax();
}

}

// ---------------------------------------------------------------------------
// The handle table and the locks
// ---------------------------------------------------------------------------

/* sub_51e110: what a printf that needs floats finds when they are not linked in. */
reg32 frontend::watc::fpNotLoaded(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest.call(kFatalError, args({ 0x551368, 1 }));
}

void frontend::watcFpNotLoaded(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e110", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::fpNotLoaded(app, cpu); });
}

/* sub_51e2a0: setmbcp(0). */
reg32 frontend::watc::setMbcpZero(App* app, CPU& cpu)
{
    cpu.eax = 0;
    tail(app, cpu, 0x525ba0);
    return cpu.eax;
}

void frontend::watcSetMbcpZero(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::setMbcpZero(app, cpu);
}

/* sub_51e388: an OS handle in the table of POSIX handles, the table grown to take it, and for
 * 0, 1 and 2 Windows' standard handle set too. */
reg32 frontend::watc::setOsHandle(App* app, CPU& cpu, reg32 osHandle, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    // The lock's hook keeps edx, which the code reads after it.
    cpu.edx = handle;
    if (s(handle) < 0)
        return osHandle;
    guest.call(guest.dword(kLockHandles));
    const reg32 standard = guest.edx();
    if (standard < 3)
    {
        if (standard == 2)
            guest.leaveEdx(osHandle);
        callImport<kSetStdHandle, &win32::kernel32::SetStdHandle>(guest, cpu, reg32(0) - 10 - standard, osHandle);
    }
    const reg32 size = guest.dword(kHandleCount);
    reg32 table = guest.dword(kHandleTable);
    const reg32 bytes = handle * 4;
    reg32 last = osHandle;
    if (s(handle) < s(size))
    {
        guest.setDword(table + bytes, osHandle);
    }
    else
    {
        table = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.cpu(), table, bytes + 4);
        const reg32 count = guest.dword(kHandleCount);
        if (s(count) < s(handle))
        {
            reg32 at = count * 4;
            do
            {
                guest.setDword(table + at, 0);
                at += 4;
            } while (s(at) < s(bytes));
        }
        guest.setDword(kHandleTable, table);
        guest.setDword(kHandleCount, handle + 1);
        last = table + bytes;
        guest.setDword(last, osHandle);
        guest.leaveEdx(osHandle);
    }
    return guest.call(guest.dword(kUnlockHandles), args({ last }));
}

void frontend::watcSetOsHandle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e388", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const reg32 osHandle = cpu.eax, handle = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setOsHandle(app, cpu, osHandle, handle); });
}

/* sub_51e51c: the table of handles freed. */
reg32 frontend::watc::freeHandleTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const reg32 table = guest.dword(kHandleTable);
    if (table != 0)
    {
        guest.call(kNfree, args({ table }));
        guest.setDword(kHandleTable, 0);
    }
    return guest.eax();
}

void frontend::watcFreeHandleTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e51c", 0, 0, kEbx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::freeHandleTable(app, cpu); });
}

/* sub_51e540, sub_51f270: hooks that do nothing. */
reg32 frontend::watc::nothing51e540(App* app, CPU& cpu)
{
    (void)app;
    return cpu.eax;
}

void frontend::watcNothing51e540(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::nothing51e540(app, cpu);
    cpu.esp += 4;
}

reg32 frontend::watc::nothing51f270(App* app, CPU& cpu)
{
    (void)app;
    return cpu.eax;
}

void frontend::watcNothing51f270(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::nothing51f270(app, cpu);
    cpu.esp += 4;
}

/* sub_51e544: a critical section for a lock: one of the 64 kept, then ones allocated. */
reg32 frontend::watc::newSemaphore(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 used = guest.dword(kStaticSemaphoreCount);
    reg32 section;
    if (s(used) < 0x40)
    {
        section = kStaticSemaphores + used * 0x18;
        guest.setDword(kStaticSemaphoreCount, used + 1);
    }
    else
    {
        section = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.cpu(), 1, 0x18);
        if (section == 0)
            guest.call(kFatalError, args({ 0x55138c, 1 }));
        const reg32 list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.cpu(), guest.dword(kExtraSemaphores), (guest.dword(kExtraSemaphoreCount) + 1) << 2);
        guest.setDword(kExtraSemaphores, list);
        if (list == 0)
            guest.call(kFatalError, args({ 0x5513b0, 1 }));
        const reg32 count = guest.dword(kExtraSemaphoreCount);
        guest.setDword(guest.dword(kExtraSemaphores) + count * 4, section);
        guest.setDword(kExtraSemaphoreCount, count + 1);
    }
    callImport<kInitializeCriticalSection, &host::initLock>(guest, cpu, section);
    return section;
}

void frontend::watcNewSemaphore(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e544", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::newSemaphore(app, cpu); });
}

/* sub_51e5e8: the kept critical sections deleted. */
reg32 frontend::watc::deleteKeptSemaphores(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (s(guest.dword(kStaticSemaphoreCount)) > 0)
    {
        reg32 i = 0, section = kStaticSemaphores;
        do
        {
            ++i;
            callImport<kDeleteCriticalSection, &host::deleteLock>(guest, cpu, section);
            section += 0x18;
        } while (s(i) < s(guest.dword(kStaticSemaphoreCount)));
    }
    return guest.eax();
}

void frontend::watcDeleteKeptSemaphores(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e5e8", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::deleteKeptSemaphores(app, cpu); });
}

/* sub_51e618: the allocated ones deleted and freed, and their list. */
reg32 frontend::watc::deleteExtraSemaphores(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (s(guest.dword(kExtraSemaphoreCount)) > 0)
    {
        reg32 i = 0;
        do
        {
            const reg32 offset = i * 4;
            ++i;
            callImport<kDeleteCriticalSection, &host::deleteLock>(
                guest, cpu, guest.dword(guest.dword(kExtraSemaphores) + offset));
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), guest.dword(guest.dword(kExtraSemaphores) + offset));
        } while (s(i) < s(guest.dword(kExtraSemaphoreCount)));
    }
    const reg32 list = guest.dword(kExtraSemaphores);
    if (list != 0)
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), list);
    return guest.eax();
}

void frontend::watcDeleteExtraSemaphores(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e618", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::deleteExtraSemaphores(app, cpu); });
}

/* sub_51e670: a lock's owner and count cleared. */
reg32 frontend::watc::clearLock(App* app, CPU& cpu, reg32 lock)
{
    Guest guest(app, cpu);
    guest.setDword(lock + 8, 0);
    guest.setDword(lock + 0xc, 0);
    return lock;
}

void frontend::watcClearLock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e670", 0, 0, 0);
    const reg32 lock = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::clearLock(app, cpu, lock); });
}

/* sub_51e680 to sub_51e70c: the runtime's hooks for its locks, each a lock named and sub_51e71c
 * or sub_51e780 on it. */
reg32 frontend::watc::lockStreams(App* app, CPU& cpu)
{
    cpu.eax = kLockStreams;
    tail(app, cpu, 0x51e71c);
    return cpu.eax;
}

void frontend::watcLockStreams(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::lockStreams(app, cpu);
}

reg32 frontend::watc::unlockStreams(App* app, CPU& cpu)
{
    cpu.eax = kLockStreams;
    tail(app, cpu, 0x51e780);
    return cpu.eax;
}

void frontend::watcUnlockStreams(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::unlockStreams(app, cpu);
}

reg32 frontend::watc::lockFile(App* app, CPU& cpu, reg32 file)
{
    cpu.eax = ((file & 0xf) << 4) + kLockFiles;
    tail(app, cpu, 0x51e71c);
    return cpu.eax;
}

void frontend::watcLockFile(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 file = cpu.eax;
    watc::lockFile(app, cpu, file);
}

reg32 frontend::watc::unlockFile(App* app, CPU& cpu, reg32 file)
{
    cpu.eax = ((file & 0xf) << 4) + kLockFiles;
    tail(app, cpu, 0x51e780);
    return cpu.eax;
}

void frontend::watcUnlockFile(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 file = cpu.eax;
    watc::unlockFile(app, cpu, file);
}

/* sub_51e6b8: allocPOSIXHandle. */
reg32 frontend::watc::allocHandleHook(App* app, CPU& cpu, reg32 osHandle)
{
    cpu.eax = osHandle;
    tail(app, cpu, 0x51e304);
    return cpu.eax;
}

void frontend::watcAllocHandleHook(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 osHandle = cpu.eax;
    watc::allocHandleHook(app, cpu, osHandle);
}

/* sub_51e6c0: a file's lock cleared and its POSIX handle freed. */
reg32 frontend::watc::freeHandleHook(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest::direct<0x51e670, clearLock>(app, cpu, ((handle & 0xf) << 4) + kLockFiles);
    return guest.call(0x51e45c, args({ handle }));
}

void frontend::watcFreeHandleHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e6c0", 0, 0, kEdx);
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::freeHandleHook(app, cpu, handle); });
}

reg32 frontend::watc::lockHeap(App* app, CPU& cpu)
{
    cpu.eax = kLockHeap;
    tail(app, cpu, 0x51e71c);
    return cpu.eax;
}

void frontend::watcLockHeap(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::lockHeap(app, cpu);
}

reg32 frontend::watc::unlockHeap(App* app, CPU& cpu)
{
    cpu.eax = kLockHeap;
    tail(app, cpu, 0x51e780);
    return cpu.eax;
}

void frontend::watcUnlockHeap(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::unlockHeap(app, cpu);
}

reg32 frontend::watc::lockFileTable(App* app, CPU& cpu)
{
    cpu.eax = kLockFileTable;
    tail(app, cpu, 0x51e71c);
    return cpu.eax;
}

void frontend::watcLockFileTable(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::lockFileTable(app, cpu);
}

reg32 frontend::watc::unlockFileTable(App* app, CPU& cpu)
{
    cpu.eax = kLockFileTable;
    tail(app, cpu, 0x51e780);
    return cpu.eax;
}

void frontend::watcUnlockFileTable(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::unlockFileTable(app, cpu);
}

reg32 frontend::watc::lockThreadList(App* app, CPU& cpu)
{
    cpu.eax = kLockThreadList;
    tail(app, cpu, 0x51e71c);
    return cpu.eax;
}

void frontend::watcLockThreadList(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::lockThreadList(app, cpu);
}

reg32 frontend::watc::unlockThreadList(App* app, CPU& cpu)
{
    cpu.eax = kLockThreadList;
    tail(app, cpu, 0x51e780);
    return cpu.eax;
}

void frontend::watcUnlockThreadList(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::unlockThreadList(app, cpu);
}

/* sub_51e714: the handle table's lock. */
reg32 frontend::watc::lockHandleTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    return lockSemaphore(guest, cpu, kLockHandleTable);
}

void frontend::watcLockHandleTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e714", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lockHandleTable(app, cpu); });
}

/* sub_51e71c: a lock taken: its critical section made the first time, entered unless this thread
 * has it already, counted. */
reg32 frontend::watc::lock(App* app, CPU& cpu, reg32 lock)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    return lockSemaphore(guest, cpu, lock);
}

void frontend::watcLock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e71c", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const reg32 lock = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lock(app, cpu, lock); });
}

/* sub_51e778: the handle table's lock given back. */
reg32 frontend::watc::unlockHandleTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    return unlockSemaphore(guest, cpu, kLockHandleTable);
}

void frontend::watcUnlockHandleTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e778", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::unlockHandleTable(app, cpu); });
}

/* sub_51e780: a lock given back, its critical section left when the count is out. */
reg32 frontend::watc::unlock(App* app, CPU& cpu, reg32 lock)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    return unlockSemaphore(guest, cpu, lock);
}

void frontend::watcUnlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e780", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const reg32 lock = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::unlock(app, cpu, lock); });
}

// ---------------------------------------------------------------------------
// Threads' data
// ---------------------------------------------------------------------------

/* sub_51e7a8: the current thread's data, made if it has none and regrown if it asks, without
 * changing Windows' last error. */
reg32 frontend::watc::threadData(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    // In esi across what follows, as the code keeps it.
    cpu.esi = callImport<kGetLastError, &win32::kernel32::GetLastError>(guest, cpu);
    reg32 data = callImport<kTlsGetValue, &host::tlsGet>(guest, cpu, guest.dword(kTlsIndex));
    if (data == 0)
        data = guest.call(0x525e10);   // by address: the esi its fatal error's path leaves is read after it
    else if (guest.byte(data + 0x53) != 0)
        data = fe::direct<0x525e4c, resizeThreadData>(app, cpu);
    callImport<kSetLastError, &win32::kernel32::SetLastError>(guest, cpu, cpu.esi);
    return data;
}

void frontend::watcThreadData(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e7a8", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::threadData(app, cpu); });
}

/* sub_51e828: the TLS index taken, past 0, 1 and 2 on Windows' older versions. */
reg32 frontend::watc::allocTlsIndex(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    constexpr reg32 kWinVersion = 0x567841, kWinMajor = 0x56783f;
    reg32 index = guest.dword(kTlsIndex);
    if (index == 0xffffffff)
    {
        index = callImport<kTlsAlloc, &host::tlsAlloc>(guest, cpu);
        if (guest.word(kWinVersion) >= 0x8000 && guest.byte(kWinMajor) < 4)
        {
            while (index != 0xffffffff && index <= 2)
            {
                guest.setDword(kTlsIndex, index);
                index = callImport<kTlsAlloc, &host::tlsAlloc>(guest, cpu);
            }
        }
    }
    guest.setDword(kTlsIndex, index);
    return index != 0xffffffff ? 1 : 0;
}

void frontend::watcAllocTlsIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e828", 0, 0, kEbx | kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::allocTlsIndex(app, cpu); });
}

/* sub_51e888: a thread's data made, put on the list and in its TLS slot. */
reg32 frontend::watc::attachThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    if (guest.dword(kTlsIndex) == 0xffffffff)
        return 0;
    const reg32 data = guest.call(0x51e7ec);
    if (data == 0)
        return 0;
    if (fe::direct<0x525f20, addThread>(app, cpu, guest.dword(data + 0xda), data) == 0)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), data);
        return 0;
    }
    callImport<kTlsSetValue, &host::tlsSet>(guest, cpu, guest.dword(kTlsIndex), data);
    return 1;
}

void frontend::watcAttachThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e888", 0, 0, kEbx | kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::attachThread(app, cpu); });
}

/* sub_51e8dc: this thread's data off the list and out of its slot; with eax its handle closed. */
reg32 frontend::watc::detachThread(App* app, CPU& cpu, reg32 closeIt)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    const reg32 index = guest.dword(kTlsIndex);
    if (index == 0xffffffff)
        return guest.eax();
    const reg32 data = callImport<kTlsGetValue, &host::tlsGet>(guest, cpu, index);
    if (data == 0)
        return 0;
    const reg32 thread = guest.dword(data + 0xde);
    guest::direct<0x525f84, removeThread>(app, cpu, guest.dword(data + 0xda));
    callImport<kTlsSetValue, &host::tlsSet>(guest, cpu, guest.dword(kTlsIndex), 0);
    if (thread != 0 && closeIt != 0)
        callImport<kCloseHandle, &win32::kernel32::CloseHandle>(guest, cpu, thread);
    return guest.eax();
}

void frontend::watcDetachThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e8dc", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const reg32 closeIt = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::detachThread(app, cpu, closeIt); });
}

/* sub_51e934: the last thread's end: its data, then the TLS index. */
reg32 frontend::watc::endThreads(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0);
    // 1 in eax as the game's code hands it, which it gives back with no index; what it gives back, in
    // eax: what the end hands back if there is no index to give.
    cpu.eax = 1;
    cpu.eax = fe::direct<0x51e8dc, detachThread>(app, cpu, 1);
    cpu.esp -= 8;
    return giveBackTlsIndex(guest, cpu);
}

void frontend::watcEndThreads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e934", 0, 0, kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::endThreads(app, cpu); });
}

/* sub_51e940: the TLS index given back. */
reg32 frontend::watc::freeTlsIndex(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    return giveBackTlsIndex(guest, cpu);
}

void frontend::watcFreeTlsIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e940", 0, 0, kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::freeTlsIndex(app, cpu); });
}

/* sub_51e964: the runtime made thread-safe, on the first _beginthread: the hooks pointed at the
 * real locks, and the first thread's data on the list. */
reg32 frontend::watc::initThreads(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    constexpr reg32 kMainThreadData = 0x9f642c;
    guest.setDword(0x567768, 0x51e698);
    guest.setDword(0x56776c, 0x51e6a8);
    guest.setDword(0x567770, 0x51e6b8);
    guest.setDword(0x567774, 0x51e6c0);
    guest.setDword(0x567778, 0x51e680);
    guest.setDword(0x56777c, 0x51e68c);
    guest.setDword(kLockThreads, 0x51e704);
    guest.setDword(kUnlockThreads, 0x51e70c);
    guest.setDword(0x56acc8, 0x51e71c);
    guest.setDword(0x56accc, 0x51e780);
    guest.setDword(0x56acd0, 0x51e670);
    guest.setDword(0x567780, 0x51e6dc);
    guest.setDword(0x567784, 0x51e6f0);
    guest.setDword(0x567788, 0x51e6e4);
    guest.setDword(0x56778c, 0x51e6f8);
    // In edi across the call, as the code keeps it.  By address: the edi it leaves is stored after it
    // (its fatal error's path changes it).
    cpu.edi = 0x51e934;
    const reg32 section = guest.call(0x51e544);
    guest.setDword(kLockOfLocks, section);
    guest.setDword(kLockOfLocks + 4, 1);
    guest.setDword(kLockHandles, 0x51e714);
    guest.setDword(kUnlockHandles, 0x51e778);
    const reg32 data = guest.dword(kMainThreadData);
    guest.setDword(0x5677a0, cpu.edi);
    guest::direct<0x525f20, addThread>(app, cpu, guest.dword(data + 0xda), data);
    callImport<kTlsSetValue, &host::tlsSet>(guest, cpu, guest.dword(kTlsIndex), guest.dword(kMainThreadData));
    guest.setDword(kThreadDataGetter, 0x51e7a8);
    return guest.eax();
}

void frontend::watcInitThreads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51e964", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::initThreads(app, cpu); });
}

/* sub_51ea7c: the runtime's locks taken down at its end. */
reg32 frontend::watc::finiThreads(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const auto clear = [&](reg32 lock) { guest.call(guest.dword(0x56acd0), args({ lock })); };
    clear(kLockStreams);
    for (reg32 lock = kLockFiles; lock != kLockFiles + 0x100; lock += 0x10)
        clear(lock);
    clear(kLockHandleTable);
    fe::direct<0x51e618, deleteExtraSemaphores>(app, cpu);
    fe::direct<0x526060, freeThreadListJump>(app, cpu);
    guest.call(0x5241c0);
    clear(kLockHeap);
    clear(kLockFileTable);
    clear(kLockThreadList);
    clear(kLockOfLocks);
    // What it gives back on in eax: what the last hands back when there is no index to give.
    cpu.eax = fe::direct<0x51e5e8, deleteKeptSemaphores>(app, cpu);
    return fe::direct<0x51e940, freeTlsIndex>(app, cpu);
}

void frontend::watcFiniThreads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51ea7c", 0, 0, kEbx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::finiThreads(app, cpu); });
}

// ---------------------------------------------------------------------------
// Exceptions
// ---------------------------------------------------------------------------

/* sub_51ec90: whether the program has an active window, asked of USER32 if it is loadable. */
reg32 frontend::watc::hasActiveWindow(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    reg32 window = 0;
    const reg32 user32 = callImport<kLoadLibrary, &win32::kernel32::LoadLibraryA>(guest, cpu, 0x5513d4);
    if (user32 != 0)
    {
        const reg32 getActiveWindow = callImport<kGetProcAddress, &win32::kernel32::GetProcAddress>(guest, cpu, user32, 0x5513e0);
        if (getActiveWindow != 0)
            window = guest.call(getActiveWindow);
    }
    return window != 0 ? 1 : 0;
}

void frontend::watcHasActiveWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51ec90", 0, 0, kEbx | kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::hasActiveWindow(app, cpu); });
}

/* sub_51eccc: a line added to the report, the hex digits after its 0x the value's. */
reg32 frontend::watc::reportAppend(App* app, CPU& cpu, reg32 report, reg32 line, reg32 value)
{
    Guest guest(app, cpu);
    constexpr reg32 kHexDigits = 0x56acdc;
    reg32 to = report, from = line;
    if (guest.byte(to) != 0)
    {
        do
            ++to;
        while (guest.byte(to) != 0);
    }
    reg32 lastDigit = 0;
    reg32 digitAt = to + 9;
    for (;;)
    {
        const std::uint8_t c = guest.byte(from);
        guest.setByte(to, c);
        if (c == 0)
            break;
        if (c == '0' && guest.byte(from + 1) == 'x')
            lastDigit = digitAt;
        ++digitAt;
        ++to;
        ++from;
    }
    guest.leaveEdx(from);
    if (lastDigit == 0)
    {
        guest.leaveEbx(0);
        return to;
    }
    reg32 rest = value;
    while (rest != 0)
    {
        const std::uint8_t digit = guest.byte((rest & 0xf) + kHexDigits);
        guest.leaveEdx(digit);
        --lastDigit;
        guest.setByte(lastDigit + 1, digit);
        rest >>= 4;
    }
    guest.leaveEbx(lastDigit);
    return 0;
}

void frontend::watcReportAppend(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51eccc", 0, 0, kEcx | kEsi | kEdi);
    const reg32 value = cpu.ebx;
    const reg32 report = cpu.eax;
    const reg32 line = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::reportAppend(app, cpu, report, line, value); });
}

/* sub_51ed24: the filter that writes the report on stderr, when there is a console to read it. */
reg32 frontend::watc::reportException(App* app, CPU& cpu, reg32 pointers)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x10c);
    const reg32 buffer = guest.frame();
    const reg32 record = guest.dword(pointers), context = guest.dword(pointers + 4);
    cpu.ecx = record;
    if (fe::direct<0x51ec90, hasActiveWindow>(app, cpu) != 0 || guest.call(0x515734) == 0xffffffff)
        return 0;
    guest.setByte(buffer, 0);
    const auto add = [&](reg32 line, reg32 value) { fe::direct<0x51eccc, reportAppend>(app, cpu, buffer, line, value); };
    const reg32 where = guest.dword(record + 0xc);
    const reg32 code = guest.dword(record);
    reg32 line = 0;
    switch (code)
    {
    case 0xc000008d: line = 0x551498; break;  // denormal operand
    case 0xc000008e: line = 0x5514ec; break;  // division by zero
    case 0xc000008f: line = 0x551540; break;  // inexact
    case 0xc0000090: line = 0x551630; break;  // invalid operation
    case 0xc0000091: line = 0x551594; break;  // overflow
    case 0xc0000092:                          // the x87's stack: which way, from its status word
        line = (guest.byte(context + 0x21) & 2) != 0 ? 0x5513f0 : 0x551444;
        break;
    case 0xc0000093: line = 0x5515e0; break;  // underflow
    case 0xc0000094: line = 0x551774; break;  // integer division by zero
    case 0xc0000096: line = 0x5516f8; break;  // privileged instruction
    case 0xc000001d: line = 0x551738; break;  // illegal instruction
    case 0xc00000fd: line = 0x5517b8; break;  // stack overflow
    case 0xc0000005:                          // the instruction, the address and which way
        add(0x551688, where);
        add(0x5516bc, guest.dword(record + 0x18));
        add(guest.dword(record + 0x14) == 0 ? 0x5516e4 : 0x5516ec, 0);
        break;
    default:
        add(0x5517f4, code);
        line = 0x551828;
        break;
    }
    if (line != 0)
        add(line, where);
    const reg32 length = guest.length(buffer);
    cpu.ecx = length;
    const reg32 handles = guest.dword(kHandleTable);
    callImport<kWriteFile, &win32::kernel32::WriteFile>(guest, cpu, guest.dword(handles + 8), buffer, length, buffer + 0x100, 0);
    return 1;
}

void frontend::watcReportException(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51ed24", 4, 0, kEbx | kEdi);
    const reg32 pointers = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::reportException(app, cpu, pointers); });
}

/* sub_51ef08: the runtime's exception handler: the x87's exceptions and the rest raised as
 * signals, or handed to what signal hooked, and otherwise Windows' filter and the exit. */
reg32 frontend::watc::exceptionHandler(App* app, CPU& cpu, reg32 record, reg32 context)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    constexpr reg32 kSignalAsked = 0x56acd4, kSignalRaise = 0x56acd8;
    const reg32 pointers = guest.frame();
    const auto notHandled = [&]() -> reg32 {
        guest.setDword(pointers, record);
        guest.setDword(pointers + 4, context);
        cpu.eax = pointers;
        if (callImport<kUnhandledExceptionFilter, &win32::kernel32::UnhandledExceptionFilter>(guest, cpu, pointers) != 0)
            callImport<kExitProcess, &win32::kernel32::ExitProcess>(guest, cpu, 0xffffffff);
        return 1;
    };
    if ((guest.byte(record + 4) & 6) != 0)
        return 1;
    const reg32 kind = guest.dword(record) + 0x3fffff73;
    reg32 signal = 0;
    if (kind <= 6)
    {
        switch (kind)
        {
        case 0: signal = 0x82; break;
        case 1: signal = 0x83; break;
        case 2: signal = 0x86; break;
        case 4: signal = 0x84; break;
        case 5: signal = (guest.byte(context + 0x21) & 2) != 0 ? 0x8a : 0x8b; break;
        case 6: signal = 0x85; break;
        default:
        {
            // An invalid operation: which instruction, and for a store whether st(0) was empty.
            const reg32 instruction = guest.dword(context + 0x28);
            const std::uint16_t opcode = guest.word(instruction);
            reg32 edx = (cpu.edx & 0xffff0000) | opcode;
            signal = 0x81;
            if (opcode == 0xfad9)
                signal = 0x88;
            else if (opcode == 0xf1d9)
                signal = 0x8e;
            else
            {
                const std::uint8_t first = guest.byte(instruction);
                edx = (edx & 0xffff00ff) | (reg32(first) << 8);
                if (first == 0xdb || first == 0xdf)
                {
                    const std::uint8_t form = guest.byte(instruction + 1) & 0x30;
                    edx = (edx & 0xffffff00) | form;
                    if (form == 0x10)
                        signal = 0x8d;
                }
                if ((guest.byte(instruction) & 1) == 0 && (guest.byte(instruction + 1) & 0x30) == 0x30)
                {
                    const reg32 top = ((guest.dword(context + 0x20) << 2) & 0xffff) >> 13;
                    const reg32 tags = guest.dword(context + 0x24) & 0xffff;
                    cpu.ecx = top * 2;
                    edx = (tags >> (top * 2)) & 1;
                    if (edx == 1)
                        signal = 0x83;
                }
            }
            guest.leaveEdx(edx);
            break;
        }
        }
    }
    if (signal != 0)
    {
        guest.setByte(kSignalStopped, 1);
        fe::direct<0x526070, clearFpExceptions>(app, cpu);
        if (fe::direct<0x526244, raiseFp>(app, cpu, signal) == 0xffffffff || guest.byte(kSignalStopped) == 0)
            return notHandled();
        const reg32 flags = guest.word(context + 0x20);
        guest.setWord(context + 0x20, std::uint16_t(flags & 0x7f00));
        return 0;
    }
    if (guest.dword(kSignalRaise) == 0)
        return notHandled();
    cpu.ecx = 1;
    for (reg32 number = 1; s(number) <= 0xc; ++number)
    {
        const reg32 asked = guest.call(guest.dword(kSignalAsked), args({ number, guest.dword(record) }));
        if (asked == 0)
            continue;
        if (asked == 1 || asked == 2 || asked == 3)
            break;
        guest.setByte(kSignalStopped, std::uint8_t(cpu.ecx));
        guest.call(guest.dword(kSignalRaise), args({ number }));
        if (guest.byte(kSignalStopped) != 0)
            return 0;
    }
    return notHandled();
}

void frontend::watcExceptionHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51ef08", 0, 0, kEbx | kEsi | kEdi);
    const reg32 record = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 context = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::exceptionHandler(app, cpu, record, context); });
}

/* sub_51f140: the float formatter's hook for printf: the double taken off the arguments, its
 * digits from __LDcvt, and the sign. */
reg32 frontend::watc::formatFloat(App* app, CPU& cpu, reg32 out, reg32 arguments, reg32 spec)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x50);
    const reg32 ebp = guest.frame() + 0x4c;
    const auto local = [&](reg32 below) { return ebp - below; };
    const reg32 character = guest.byte(spec + 0x15);
    guest.setDword(local(0x40), character);
    const reg32 upper = guest.byte(spec + 0x15) & 0x5f;
    reg32 precision = guest.dword(spec + 8);
    if (upper == 'G')
    {
        if (precision == 0)
            precision = 1;
        guest.setDword(local(0x44), 4);
        guest.setDword(local(0x48), 1);
        guest.setDword(local(0x40), character - 2);
    }
    else if (upper == 'E')
    {
        guest.setDword(local(0x44), 1);
        guest.setDword(local(0x48), 1);
    }
    else
    {
        guest.setDword(local(0x44), 2);
        guest.setDword(local(0x48), 0);
    }
    if ((guest.byte(spec + 0x1e) & 1) != 0)
        guest.setByte(local(0x44), guest.byte(local(0x44)) | 0x10);
    const reg32 next = guest.dword(arguments) + 8;
    guest.setDword(arguments, next);
    guest.setDword(local(0x14), guest.dword(next - 8));
    guest.setDword(local(0x10), guest.dword(next - 4));
    app->getMemory<x86::IEEEf80>(local(0x20)) = x86::Float(app->getMemory<double>(local(0x14)));
    if (precision == 0xffffffff)
        precision = 6;
    guest.setDword(local(0x4c), precision);
    guest.setDword(local(0x3c), 0);
    guest.leaveEcx(spec);
    fe::direct<0x5167b3, watb::longDoubleToDigits>(app, cpu, local(0x20), local(0x4c), out + 1);
    guest.setDword(spec + 0x28, guest.dword(local(0x30)));
    guest.setDword(spec + 0x2c, guest.dword(local(0x2c)));
    guest.setDword(spec + 0x30, guest.dword(local(0x28)));
    guest.setDword(spec + 0x34, guest.dword(local(0x24)));
    const auto sign = [&](std::uint8_t c) {
        const reg32 at = guest.dword(spec + 0x20);
        guest.setDword(spec + 0x20, at + 1);
        guest.setByte(out + at, c);
    };
    if (s(guest.dword(local(0x38))) < 0)
        sign('-');
    else
    {
        const std::uint8_t flags = guest.byte(spec + 0x1e);
        if ((flags & 4) != 0)
            sign('+');
        else if ((flags & 2) != 0)
            sign(' ');
    }
    guest.leaveEdx(cpu.ds);
    return out;
}

void frontend::watcFormatFloat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51f140", 0, 0, kEcx | kEsi | kEdi);
    const reg32 out = cpu.eax, arguments = cpu.edx, spec = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::formatFloat(app, cpu, out, arguments, spec); });
}

/* sub_51f254: scanf's hook: strtod, the double stored. */
reg32 frontend::watc::scanFloat(App* app, CPU& cpu, reg32 text, reg32 to)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    const reg32 result = guest.call(0x4f82b6, args({ text, 0 }));
    app->getMemory<double>(to) = double(guest.fpu().pop());
    return result;
}

void frontend::watcScanFloat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_51f254", 0, 0, kEbx);
    const reg32 text = cpu.eax, to = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::scanFloat(app, cpu, text, to); });
}

// ---------------------------------------------------------------------------
// Pixels
// ---------------------------------------------------------------------------

/* sub_5236c0: shaperowbytes: a shape's bytes a row, by its type's bits and alignment. */
reg32 frontend::watc::shapeRowBytes(App* app, CPU& cpu, reg32 shape)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    constexpr reg32 kBitsOfType = 0x566d54;
    const reg32 type = guest.dword(shape) & 0xff;
    if (type == 0x7b)
        return sar(guest.dword(shape + 2), 16);
    if ((type & 0x80) != 0)
    {
        const reg32 ebx = cpu.ebx, esi = cpu.esi;
        guest.setDword(kErrorFile, 0x551b0c);
        guest.setDword(kErrorFunction, 0x551b1c);
        guest.setDword(kErrorLine, 0x35);
        fe::direct<0x401010, ea::error>(app, cpu, 0x551b2cu, guest::Words({ type }));
        cpu.ebx = ebx;
        cpu.esi = esi;
    }
    reg32 bits = guest.byte((guest.dword(shape) & 0x7f) + kBitsOfType);
    if (bits == 0xf)
        bits = 0x10;
    const reg32 rowBits = sar(guest.dword(shape + 2), 16) * bits;
    if ((guest.dword(shape) & 0xf0) == 0x40)
        return sar((rowBits + 0xf) & 0xfffffff0, 3);
    return sar((rowBits + 7) & 0xfffffff8, 3);
}

void frontend::watcShapeRowBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5236c0", 0, 0, kEcx | kEdx);
    const reg32 shape = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::shapeRowBytes(app, cpu, shape); });
}

namespace
{

/* A channel under a pixel scaled by a 16.16 inverse alpha, rounded on the bit shifted out. */
reg32 fixedProduct(reg32 value, reg32 scale)
{
    const std::int64_t product = std::int64_t(sreg32(value)) * std::int64_t(sreg32(scale));
    const reg32 low = reg32(product);
    const reg32 high = reg32(std::uint64_t(product) >> 32) << 16;
    return (low >> 16) + high + ((low >> 15) & 1);
}

}

/* sub_523760: a pixel with its alpha: a 16-bit window blended in place at once, any other through
 * readpixel and writepixel. */
reg32 frontend::watc::putPixel(App* app, CPU& cpu, reg32 x, reg32 y, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 40);
    if (s(x) < s(guest.dword(kClipLeft)) || s(x) >= s(guest.dword(kClipRight)) || s(y) < s(guest.dword(kClipTop))
        || s(y) >= s(guest.dword(kClipBottom)))
        return x;
    if (guest.byte(kWindowDepth) == 0x10 && guest.byte(kWindowFlag) == 0)
    {
        const reg32 rows = guest.dword(kRowOffsets);
        const reg32 at = guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(rows + y * 4) + guest.dword(kWindowPixels);
        if (colour < 0x10000000)
        {
            guest.leaveEdx(rows);
            return at;
        }
        reg32 mixed = colour;
        if (colour < 0xfc000000)
        {
            // The pixel's fields spread out and scaled by the inverse alpha, added to the colour.
            const reg32 pixel = guest.word(at);
            const reg32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
            const std::uint64_t product = std::uint64_t(spread) * ((colour ^ 0xffffffff) >> 24);
            const reg32 low = reg32(product), high = reg32(product >> 32) << 16;
            mixed = colour + high + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
        }
        const reg32 packed = ((mixed & 0xf80000) >> 8) + ((mixed >> 3) & 0x1f) + ((mixed & 0xfc00) >> 5);
        guest.setWord(at, std::uint16_t(packed));
        guest.leaveEbx(packed);
        guest.leaveEdx((mixed & 0xfc00) >> 5);
        return (mixed >> 3) & 0x1f;
    }
    const reg32 alpha = colour >> 24;
    if (alpha == 0xff)
    {
        const reg32 converted = fe::direct<0x4ef760, eab::colourPack>(app, cpu, colour);
        return fe::direct<0x523b00, writePixel>(app, cpu, x, y, converted);
    }
    if (alpha == 0)
    {
        guest.leaveEbx(0);
        return x;
    }
    cpu.ebx = alpha;
    const reg32 red = (colour >> 16) & 0xff, green = (colour >> 8) & 0xff, blue = colour & 0xff;
    cpu.ecx = 0x10000 - (alpha << 8);
    const reg32 under = fe::direct<0x50c3a0, watb::colourToArgb>(app, cpu, fe::direct<0x503bc0, watb::readPixel>(app, cpu, x, y));
    const reg32 underAlpha = sar(under, 24) & 0xff, underRed = sar(under, 16) & 0xff;
    const reg32 underGreen = sar(under, 8) & 0xff, underBlue = under & 0xff;
    const reg32 outAlpha = 0xff - reg32(s((0xff - cpu.ebx) * (0xff - underAlpha)) / 0xff);
    const reg32 scale = cpu.ecx;
    const auto channel = [&](reg32 below, reg32 own) {
        const reg32 sum = fixedProduct(below, scale) + own;
        return s(sum) <= 0xff ? sum : reg32(0xff);
    };
    const reg32 outRed = channel(underRed, red);
    const reg32 outGreen = channel(underGreen, green);
    const reg32 outBlue = channel(underBlue, blue);
    const reg32 argb = (outAlpha << 24) | (outRed << 16) | (outGreen << 8) | outBlue;
    const reg32 converted = fe::direct<0x4ef760, eab::colourPack>(app, cpu, argb);
    return fe::direct<0x523b00, writePixel>(app, cpu, x, y, converted);
}

void frontend::watcPutPixel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_523760", 0, 0, kEcx | kEsi | kEdi);
    const reg32 x = cpu.eax, y = cpu.edx, colour = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::putPixel(app, cpu, x, y, colour); });
}

/* sub_5239ec: a 4-bit shape's rows through a blending table onto 8-bit pixels: a pixel's nibble is
 * the table's high byte, what is under it the low. */
reg32 frontend::watc::blend4To8(App* app, CPU& cpu, reg32 arguments)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 base = arguments;
    const reg32 kPairs = base + 12, kRows = base + 16, kFromSkip = base + 20, kToSkip = base + 24;
    const reg32 table = guest.dword(base);
    reg32 eax = cpu.eax;
    if (s(guest.dword(kRows)) <= 0)
        return eax;
    reg32 edx = 0;
    guest.leaveEdx(0);
    reg32 from = guest.dword(base + 4), to = guest.dword(base + 8);
    const reg32 width = guest.dword(kPairs);
    if (s(width) < 1)
    {
        guest.leaveEcx(width);
        return eax;
    }
    const bool odd = (width & 1) != 0;
    guest.setDword(kPairs, width >> 1);
    const auto pixel = [&](reg32 source, reg32 under) {
        const std::uint8_t p = guest.byte(source);
        const reg32 low = (reg32(p >> 4) << 8) | guest.byte(under);
        edx = (reg32(p & 0xf) << 8) | guest.byte(under + 1);
        return reg32(guest.byte(table + low)) | (reg32(guest.byte(table + edx)) << 8);
    };
    if (odd)
    {
        guest.setDword(kFromSkip, guest.dword(kFromSkip) + 1);
        guest.setDword(kToSkip, guest.dword(kToSkip) + 2);
    }
    do
    {
        for (reg32 count = guest.dword(kPairs); count != 0; --count)
        {
            const reg32 second = pixel(from + 1, to + 2);
            const reg32 first = pixel(from, to);
            eax = (second << 16) | first;
            guest.setDword(to, eax);
            from += 2;
            to += 4;
        }
        if (odd)
        {
            const reg32 last = pixel(from, to);
            eax = (eax & 0xffff0000) | last;
            guest.setWord(to, std::uint16_t(last));
        }
        from += guest.dword(kFromSkip);
        to += guest.dword(kToSkip);
        guest.setDword(kRows, guest.dword(kRows) - 1);
    } while (guest.dword(kRows) != 0);
    guest.leaveEcx(0);
    guest.leaveEdx(edx);
    return eax;
}

void frontend::watcBlend4To8(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5239ec", 0, 0, kEbx | kEsi | kEdi);
    const reg32 arguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::blend4To8(app, cpu, arguments); });
}

/* sub_523b00: writepixel: a colour already in the window's format, at its depth. */
reg32 frontend::watc::writePixel(App* app, CPU& cpu, reg32 x, reg32 y, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (s(x) < s(guest.dword(kClipLeft)) || s(x) >= s(guest.dword(kClipRight)) || s(y) < s(guest.dword(kClipTop))
        || s(y) >= s(guest.dword(kClipBottom)))
        return x;
    const reg32 at = guest.dword(guest.dword(kRowOffsets) + y * 4) + guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(kWindowPixels);
    guest.leaveEdx(at);
    const std::uint8_t depth = guest.byte(kWindowDepth);
    const reg32 eax = (guest.dword(kWindowPixels) & 0xffffff00) | depth;
    switch (depth)
    {
    case 0xf:
    case 0x10:
        guest.setWord(at, std::uint16_t(colour));
        return eax;
    case 0x18:
        guest.setWord(at, std::uint16_t(colour));
        guest.leaveEbx(colour >> 16);
        guest.setByte(at + 2, std::uint8_t(colour >> 16));
        return eax;
    case 0x20:
        guest.setDword(at, colour);
        return eax;
    case 8:
        guest.setByte(at, std::uint8_t(colour));
        return eax;
    default:
        break;
    }
    reg32 left = eax;
    if (depth != 4)
    {
        const reg32 edi = cpu.edi;
        guest.setDword(kErrorLine, 0x53);
        guest.setDword(kErrorFile, 0x551b68);
        guest.setDword(kErrorFunction, 0x551b78);
        guest.call(kError, args({ depth }, { 0x551b84, depth }));
        cpu.edi = edi;
        left = cpu.eax;
    }
    // Four bits a pixel, or what a depth that is none falls through to.
    const reg32 where = cpu.edx;
    if ((x & 1) != 0)
    {
        const std::uint8_t high = std::uint8_t(colour << 4);
        const std::uint8_t merged = (guest.byte(where) & 0xf) | high;
        guest.setByte(where, merged);
        guest.leaveEbx((colour & 0xffff00ff) | (reg32(merged) << 8));
        return (left & 0xffffff00) | high;
    }
    const std::uint8_t merged = (guest.byte(where) & 0xf0) | std::uint8_t(colour);
    guest.setByte(where, merged);
    return (left & 0xffffff00) | merged;
}

void frontend::watcWritePixel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_523b00", 0, 0, kEcx | kEsi);
    const reg32 x = cpu.eax, y = cpu.edx;
    const reg32 colour = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::writePixel(app, cpu, x, y, colour); });
}

// ---------------------------------------------------------------------------
// The environment
// ---------------------------------------------------------------------------

/* sub_524f88: environ made from Windows' block, the strings copied, with a byte a string that says
 * whose memory each is in; then the handles' modes from C_FILE_INFO. */
reg32 frontend::watc::initEnviron(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    constexpr reg32 kEnvironCopy = 0xa18230;
    if (guest.dword(kEnviron) != 0)
        return guest.eax();
    const reg32 block = guest.dword(kEnvironmentBlock);
    reg32 strings = 0;
    reg32 end = block;
    if (guest.byte(block) != 0)
    {
        do
        {
            while (guest.byte(end) != 0)
                ++end;
            ++end;
            ++strings;
        } while (guest.byte(end) != 0);
    }
    guest.setDword(kEnviron, 0);
    reg32 bytes = end - block;
    if (bytes == 0)
        bytes = 1;
    const reg32 copy = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), bytes);
    if (copy != 0)
    {
        guest.setDword(kEnvironCopy, copy);
        const reg32 list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), strings * 5 + 4);
        if (list == 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), copy);
        }
        else
        {
            reg32 count = 0, offset = 0, from = block, to = copy;
            while (guest.byte(from) != 0)
            {
                guest.setDword(list + offset, to);
                std::uint8_t c;
                do
                {
                    c = guest.byte(from++);
                    guest.setByte(to++, c);
                } while (c != 0);
                offset += 4;
                ++count;
            }
            guest.setDword(list + offset, 0);
            offset += 4;
            guest.setDword(kEnvironFlags, list + offset);
            guest.setDword(kEnviron, list);
            fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), list + offset, 0, count);
        }
    }
    return guest::direct<0x52bdb0, inheritHandles>(app, cpu);
}

void frontend::watcInitEnviron(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_524f88", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::initEnviron(app, cpu); });
}

/* sub_52506c: environ freed, and Windows' block. */
reg32 frontend::watc::freeEnviron(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    constexpr reg32 kEnvironCopy = 0xa18230;
    guest::direct<0x52bed0, clearEnviron>(app, cpu);
    const reg32 list = guest.dword(kEnviron);
    if (list != 0)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), list);
        guest.setDword(kEnviron, 0);
    }
    const reg32 copy = guest.dword(kEnvironCopy);
    if (copy != 0)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), copy);
        guest.setDword(kEnvironCopy, 0);
    }
    const reg32 block = guest.dword(kEnvironmentBlock);
    if (block != 0)
        callImport<kFreeEnvironmentStrings, &win32::kernel32::FreeEnvironmentStringsA>(guest, cpu, block);
    return guest.eax();
}

void frontend::watcFreeEnviron(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52506c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::freeEnviron(app, cpu); });
}

/* sub_525ba0: setmbcp: the table of lead bytes for a code page. */
reg32 frontend::watc::setMbcp(App* app, CPU& cpu, reg32 asked)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 40);
    constexpr reg32 kLeadBytes = 0xa0b210, kIsMultibyte = 0xa0b200, kCodePage = 0x56b0b8;
    const auto clearTable = [&] { fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), kLeadBytes, 0, 0x101); };
    reg32 page = asked;
    if (asked == 0xffffffff)
        page = callImport<kGetAcp, &win32::kernel32::GetACP>(guest, cpu);
    else if (asked == 0xfffffffe)
        page = callImport<kGetOemCp, &win32::kernel32::GetOEMCP>(guest, cpu);
    else if (asked == 0xfffffffd)
    {
        clearTable();
        guest.setDword(kIsMultibyte, 0);
        guest.setDword(kCodePage, 0);
        return 0;
    }
    else if (asked == 0xfffffffc)
    {
        // Shift-JIS, without asking.
        clearTable();
        for (reg32 c = 0x82; c <= 0xa0; ++c)
            guest.setByte(kLeadBytes + c, 1);
        for (reg32 c = 0xe1; c <= 0xfd; ++c)
            guest.setByte(kLeadBytes + c, 1);
        guest.setDword(kIsMultibyte, 1);
        guest.setDword(kCodePage, 0x3a4);
        return 0;
    }
    if (page == 0)
        page = 1;
    const reg32 info = guest.frame();
    cpu.eax = info;
    if (callImport<kGetCpInfo, &win32::kernel32::GetCPInfo>(guest, cpu, page, info) == 0)
        return 1;
    clearTable();
    guest.setDword(kIsMultibyte, 0);
    if (guest.byte(info + 6) != 0)
        guest.setDword(kIsMultibyte, 1);
    for (reg32 range = 0; guest.byte(info + range + 6) != 0 || guest.byte(info + range + 7) != 0; range += 2)
    {
        for (reg32 c = guest.byte(info + range + 6); s(c) <= s(guest.byte(info + range + 7));)
        {
            ++c;
            guest.setByte(kLeadBytes + c, 1);
        }
    }
    if (page == 1)
    {
        guest.setDword(kCodePage, callImport<kGetOemCp, &win32::kernel32::GetOEMCP>(guest, cpu));
        return 0;
    }
    guest.setDword(kCodePage, page);
    return 0;
}

void frontend::watcSetMbcp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525ba0", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const reg32 asked = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setMbcp(app, cpu, asked); });
}

// ---------------------------------------------------------------------------
// Threads' list
// ---------------------------------------------------------------------------

/* sub_525e10: a thread that has no data yet: made now. */
reg32 frontend::watc::attachThisThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    reg32 data = 0;
    cpu.eax = 0;   // on through it to sub_51e7ec: no block of its own
    if (fe::direct<0x51e888, attachThread>(app, cpu) != 0)
        data = callImport<kTlsGetValue, &host::tlsGet>(guest, cpu, guest.dword(kTlsIndex));
    if (data == 0)
        guest.call(kFatalError, args({ 0x551ef0, 1 }));
    return data;
}

void frontend::watcAttachThisThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525e10", 0, 0, kEbx | kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::attachThisThread(app, cpu); });
}

/* sub_525e4c: the thread's data to the size asked now, moved from the block it did not own. */
reg32 frontend::watc::resizeThreadData(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    guest.call(guest.dword(kLockThreads));
    const reg32 thread = callImport<kGetCurrentThreadId, &host::currentThreadId>(guest, cpu);
    reg32 node = guest.dword(kThreadList);
    while (node != 0 && thread != guest.dword(node + 4))
        node = guest.dword(node);
    reg32 data;
    if (guest.dword(node + 0xc) != 0)
    {
        data = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.cpu(), guest.dword(node + 8), guest.dword(kThreadDataSize));
        if (data == 0)
            guest.call(kFatalError, args({ 0x551f18, 1 }));
    }
    else
    {
        data = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.cpu(), 1, guest.dword(kThreadDataSize));
        if (data == 0)
            guest.call(kFatalError, args({ 0x551f40, 1 }));
        const reg32 old = guest.dword(node + 8);
        copyUp(guest, data, old, guest.dword(old + 0xf0));
        guest.setDword(node + 0xc, 1);
    }
    guest.setDword(node + 8, data);
    const reg32 size = guest.dword(kThreadDataSize);
    guest.setByte(data + 0x52, 1);
    guest.setByte(data + 0x53, 0);
    guest.setDword(data + 0xf0, size);
    callImport<kTlsSetValue, &host::tlsSet>(guest, cpu, guest.dword(kTlsIndex), data);
    guest.call(guest.dword(kUnlockThreads));
    return data;
}

void frontend::watcResizeThreadData(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525e4c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::resizeThreadData(app, cpu); });
}

/* sub_525f20: a thread's data on the list: 1, or 0 if there is no memory or the hook says no. */
reg32 frontend::watc::addThread(App* app, CPU& cpu, reg32 thread, reg32 data)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest.call(guest.dword(kLockThreads));
    reg32 added = 0;
    const reg32 node = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.cpu(), 1, 0x10);
    guest.leaveEdx(node);
    if (node != 0)
    {
        cpu.eax = data;   // on through it to the hook
        if (fe::direct<0x52c824, threadStartHook>(app, cpu) != 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), node);
        }
        else
        {
            guest.setDword(node + 8, data);
            guest.setDword(node + 4, thread);
            guest.setDword(node + 0xc, guest.byte(data + 0x52));
            const reg32 first = guest.dword(kThreadList);
            guest.setDword(kThreadList, node);
            guest.setDword(node, first);
            added = 1;
        }
    }
    guest.call(guest.dword(kUnlockThreads));
    return added;
}

void frontend::watcAddThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525f20", 0, 0, kEbx | kEcx | kEsi);
    const reg32 thread = cpu.eax, data = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::addThread(app, cpu, thread, data); });
}

/* sub_525f84: a thread off the list, its data freed if the list owns it. */
reg32 frontend::watc::removeThread(App* app, CPU& cpu, reg32 thread)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest.call(guest.dword(kLockThreads));
    reg32 link = kThreadList;
    for (reg32 node = guest.dword(kThreadList); node != 0; link = node, node = guest.dword(node))
    {
        if (thread == guest.dword(node + 4))
        {
            if (guest.dword(node + 0xc) != 0)
                fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), guest.dword(node + 8));
            guest.setDword(link, guest.dword(node));
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), node);
            break;
        }
    }
    guest.call(guest.dword(kUnlockThreads));
    return guest.eax();
}

void frontend::watcRemoveThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525f84", 0, 0, kEbx | kEcx | kEdx);
    const reg32 thread = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::removeThread(app, cpu, thread); });
}

/* sub_525ff8: the whole list freed. */
reg32 frontend::watc::freeThreadList(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    reg32 node = guest.dword(kThreadList);
    while (node != 0)
    {
        const reg32 owned = guest.dword(node + 0xc), next = guest.dword(node);
        if (owned != 0)
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), guest.dword(node + 8));
        guest.call(kNfree, args({ node }));
        node = next;
    }
    return guest.eax();
}

void frontend::watcFreeThreadList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_525ff8", 0, 0, kEbx | kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::freeThreadList(app, cpu); });
}

/* sub_526060: the same, by another name. */
reg32 frontend::watc::freeThreadListJump(App* app, CPU& cpu)
{
    tail(app, cpu, 0x525ff8);
    return cpu.eax;
}

void frontend::watcFreeThreadListJump(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::freeThreadListJump(app, cpu);
}

/* sub_526070: the x87's exceptions cleared, which the host has none of. */
reg32 frontend::watc::clearFpExceptions(App* app, CPU& cpu)
{
    (void)app;
    return cpu.eax;
}

void frontend::watcClearFpExceptions(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::clearFpExceptions(app, cpu);
    cpu.esp += 4;
}

// ---------------------------------------------------------------------------
// Signals
// ---------------------------------------------------------------------------

namespace
{

constexpr reg32 kBreakHandlers = 0x56b0bc;  // the handlers of 4 and 7, signals of the whole process

bool processWide(reg32 signal)
{
    return signal == 7 || signal == 4;
}

}

/* sub_526080: signal's handler set, the old one returned. */
reg32 frontend::watc::setSignalHandler(App* app, CPU& cpu, reg32 signal, reg32 handler)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    cpu.edx = handler;
    if (processWide(signal))
    {
        const reg32 at = kBreakHandlers + signal * 8;
        const reg32 old = guest.dword(at);
        guest.setDword(at, guest.edx());
        return old;
    }
    const reg32 old = guest.dword(signal * 8 + guest.call(guest.dword(kThreadDataGetter)) + 0x58);
    const reg32 data = guest.call(guest.dword(kThreadDataGetter));
    guest.setDword(signal * 8 + data + 0x58, guest.edx());
    return old;
}

void frontend::watcSetSignalHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526080", 0, 0, kEbx | kEcx);
    const reg32 signal = cpu.eax;
    const reg32 handler = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setSignalHandler(app, cpu, signal, handler); });
}

/* sub_5260c8: a signal's handler. */
reg32 frontend::watc::signalHandler(App* app, CPU& cpu, reg32 signal)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (processWide(signal))
        return guest.dword(kBreakHandlers + signal * 8);
    return guest.dword(guest.call(guest.dword(kThreadDataGetter)) + signal * 8 + 0x58);
}

void frontend::watcSignalHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5260c8", 0, 0, kEdx);
    const reg32 signal = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::signalHandler(app, cpu, signal); });
}

/* sub_5260ec: the x87's status a signal goes with. */
reg32 frontend::watc::signalMask(App* app, CPU& cpu, reg32 signal)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (processWide(signal))
        return guest.dword(kBreakHandlers + 4 + signal * 8);
    return guest.dword(guest.call(guest.dword(kThreadDataGetter)) + signal * 8 + 0x5c);
}

void frontend::watcSignalMask(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5260ec", 0, 0, kEdx);
    const reg32 signal = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::signalMask(app, cpu, signal); });
}

/* sub_526110: the exception handler's question: the handler of a signal that goes with this code. */
reg32 frontend::watc::signalForCode(App* app, CPU& cpu, reg32 signal, reg32 code)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.edx = code;   // across sub_5260ec, which keeps it
    const reg32 mask = fe::direct<0x5260ec, signalMask>(app, cpu, signal);
    if (code != mask)
        return 0;
    return fe::direct<0x5260c8, signalHandler>(app, cpu, signal);
}

void frontend::watcSignalForCode(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526110", 0, 0, kEbx);
    const reg32 signal = cpu.eax;
    const reg32 code = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::signalForCode(app, cpu, signal, code); });
}

/* sub_52612c: the console control handler: Ctrl-C and Ctrl-Break as signals. */
reg32 frontend::watc::consoleControl(App* app, CPU& cpu, reg32 event)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0);
    reg32 signal;
    if (event == 0)
        signal = 4;
    else if (event == 1)
        signal = 7;
    else
        return 0;
    const reg32 handler = fe::direct<0x5260c8, signalHandler>(app, cpu, signal);
    guest.leaveEdx(handler);   // across sub_526338, which keeps it
    if (handler == 0)
        return 0;
    fe::direct<0x526338, raiseSignal>(app, cpu, signal);
    return handler == 2 || handler == 3 ? 0 : 1;
}

void frontend::watcConsoleControl(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52612c", 4, 0, 0);
    const reg32 event = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::consoleControl(app, cpu, event); });
}

/* sub_526194: whether the console handler is wanted: not when both signals are ignored or default. */
reg32 frontend::watc::consoleWanted(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    const reg32 first = fe::direct<0x5260c8, signalHandler>(app, cpu, 4);
    guest.leaveEdx(first);   // across the second, which keeps it
    const reg32 second = fe::direct<0x5260c8, signalHandler>(app, cpu, 7);
    if ((first == 2 || first == 3) && (second == 2 || second == 3))
        return 0;
    return 1;
}

void frontend::watcConsoleWanted(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526194", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::consoleWanted(app, cpu); });
}

/* sub_5261fc: the console handler taken off: 1 when it is. */
reg32 frontend::watc::consoleUnhook(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    constexpr reg32 kHooked = 0x56b124;
    if (guest.byte(kHooked) != 0
        && callImport<kSetConsoleCtrlHandler, &win32::kernel32::SetConsoleCtrlHandler>(guest, cpu, 0x52612c, 0) != 0)
        guest.setByte(kHooked, 0);
    return guest.byte(kHooked) == 0 ? 1 : 0;
}

void frontend::watcConsoleUnhook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5261fc", 0, 0, kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::consoleUnhook(app, cpu); });
}

/* sub_526244: the floating-point signal raised with its code, the handler reset first. */
reg32 frontend::watc::raiseFp(App* app, CPU& cpu, reg32 code)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 handler = fe::direct<0x5260c8, signalHandler>(app, cpu, 2);
    if (handler == 1 || handler == 2 || handler == 3)
        return 0xffffffff;
    cpu.ecx = handler;   // across sub_526080, which keeps it
    fe::direct<0x526080, setSignalHandler>(app, cpu, 2, 2);
    guest.call(cpu.ecx, args({ 2, code }));
    return 0;
}

void frontend::watcRaiseFp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526244", 0, 0, kEbx | kEcx | kEdx);
    const reg32 code = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::raiseFp(app, cpu, code); });
}

/* sub_526338: raise. */
reg32 frontend::watc::raiseSignal(App* app, CPU& cpu, reg32 signal)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 handler = fe::direct<0x5260c8, signalHandler>(app, cpu, signal);
    cpu.ecx = handler;   // across what follows, which keeps it
    if (signal - 1 > 0xb)
        return 0xffffffff;
    if (signal == 2)
    {
        fe::direct<0x526244, raiseFp>(app, cpu, 0x8c);
        return 0;
    }
    if (signal == 1 && handler == 2)
        fe::direct<0x52c878, abnormalTermination>(app, cpu);
    const reg32 current = cpu.ecx;
    if (current != 1 && current != 2 && current != 3)
    {
        guest.leaveEdx(2);   // the new handler in edx, as the game's code loads it: on to the handler's call
        fe::direct<0x526080, setSignalHandler>(app, cpu, signal, 2);
        guest.call(cpu.ecx, args({ signal }));
    }
    if (fe::direct<0x526194, consoleWanted>(app, cpu) == 0)
        fe::direct<0x5261fc, consoleUnhook>(app, cpu);
    return 0;
}

void frontend::watcRaise(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526338", 0, 0, kEbx | kEcx | kEdx);
    const reg32 signal = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::raiseSignal(app, cpu, signal); });
}

/* sub_5263a8: a new thread's handlers, copied from the process's, and the exception handler's
 * hooks set. */
reg32 frontend::watc::initThreadSignals(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    for (reg32 offset = 8; offset != 0x68; offset += 8)
    {
        const reg32 data = guest.call(guest.dword(kThreadDataGetter));
        guest.setDword(data + offset + 0x58, guest.dword(kBreakHandlers + offset));
        guest.setDword(data + offset + 0x5c, guest.dword(kBreakHandlers + offset + 4));
    }
    guest.setDword(0x56acd4, 0x526110);
    guest.setDword(0x56acd8, 0x526338);
    return cpu.ds;
}

void frontend::watcInitThreadSignals(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5263a8", 0, 0, kEbx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::initThreadSignals(app, cpu); });
}

/* sub_5263f0: a thread's end: the console signals back to default, when it was wanted. */
reg32 frontend::watc::finiThreadSignals(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    // What the calls left in eax goes back.
    reg32 left = fe::direct<0x526194, consoleWanted>(app, cpu);
    if (left != 0)
    {
        fe::direct<0x5261fc, consoleUnhook>(app, cpu);
        fe::direct<0x526080, setSignalHandler>(app, cpu, 4, 2);
        left = fe::direct<0x526080, setSignalHandler>(app, cpu, 7, 2);
    }
    return left;
}

void frontend::watcFiniThreadSignals(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5263f0", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::finiThreadSignals(app, cpu); });
}

/* sub_526420: the two above hooked. */
reg32 frontend::watc::hookThreadSignals(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kThreadOnStart, 0x5263a8);
    guest.setDword(kThreadOnEnd, 0x5263f0);
    return guest.eax();
}

void frontend::watcHookThreadSignals(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_526420", 0, 0, kEbx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::hookThreadSignals(app, cpu); });
}

/* sub_52b523: pow's bad cases: the error reported, or 0 where the answer is 0 with no error; the
 * answer, which its adapter leaves on the x87's stack as the code does. */
double frontend::watc::powError(App* app, CPU& cpu, reg32 arguments)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x1c);
    const reg32 x = arguments, y = arguments + 8;
    const auto doubleAt = [&](reg32 at) { return app->getMemory<double>(at); };
    const std::uint8_t kind = guest.byte(arguments + 0x10);
    reg32 code = 0;
    if (kind <= 1)
    {
        if (kind != 0)
            code = 0x2047;
        else
        {
            const std::uint16_t word = fp::compare(0.0, doubleAt(y));
            if ((word & 0x100) == 0)
                code = (word & 0x4000) != 0 ? 0x4047 : 0x8047;
        }
    }
    else if ((fp::compare(0.0, doubleAt(y)) & 0x100) != 0)
    {
        code = (fp::compare(0.0, doubleAt(x)) & 0x100) == 0 ? 0x1107 : 0x8107;
    }
    double answer = 0.0;
    if (code != 0)
    {
        // The runtime's error report (native_watcom.cpp's) by address: its answer off the x87's stack.
        guest.call(0x5243b1, args({ code, x, y }));
        answer = double(guest.fpu().pop());
    }
    return answer;
}

void frontend::watcPowError(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b523", 0x14, 0, kEbx | kEcx | kEdx);
    const reg32 arguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        const reg32 eax = cpu.eax;  // as it came
        cpu.fpu.push(x86::Float(watc::powError(app, cpu, arguments)));
        return eax;
    });
}

// ---------------------------------------------------------------------------
// The debug screen
// ---------------------------------------------------------------------------

/* sub_52b6c0: a rect's clip, each edge held inside its width and height. */
reg32 frontend::watc::setRectClip(App* app, CPU& cpu, reg32 rect, reg32 leftAsked, reg32 topAsked, reg32 rightAsked, reg32 bottomAsked)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    cpu.edx = leftAsked;
    cpu.ebx = topAsked;
    cpu.ecx = rightAsked;
    const auto held = [](reg32 value, reg32 most) {
        const reg32 positive = s(value) < 0 ? 0 : value;
        return s(positive) > s(most) ? most : positive;
    };
    const reg32 wide = guest.dword(rect + 4), high = guest.dword(rect + 8);
    const reg32 left = held(cpu.edx, wide), right = held(cpu.ecx, wide);
    const reg32 top = held(cpu.ebx, high), bottom = held(bottomAsked, high);
    guest.setDword(rect + 0x14, right);
    guest.setDword(rect + 0x10, top);
    guest.setDword(rect + 0x18, bottom);
    guest.setDword(rect + 0xc, left);
    guest.leaveEdx(left);
    guest.leaveEbx(top);
    guest.leaveEcx(right);
    return rect;
}

void frontend::watcSetRectClip(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b6c0", 4, 0, kEsi | kEdi);
    const reg32 rect = cpu.eax;
    const reg32 leftAsked = cpu.edx, topAsked = cpu.ebx, rightAsked = cpu.ecx;
    const reg32 bottomAsked = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setRectClip(app, cpu, rect, leftAsked, topAsked, rightAsked, bottomAsked); });
}

/* sub_52b720: a box's outline, its corners in any order, as four filled rects. */
reg32 frontend::watc::drawBox(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (s(left) > s(right))
        std::swap(left, right);
    if (s(bottom) < s(top))
        std::swap(top, bottom);
    const reg32 wide = right - left + 1;
    const reg32 high = bottom - top - 1;
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, left, top, wide, 1, colour);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, right, top + 1, 1, high, colour);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, left, top + 1, 1, high, colour);
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, left, bottom, wide, 1, colour);
}

void frontend::watcDrawBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b720", 4, 0, kEsi | kEdi);
    const reg32 left = cpu.eax;
    const reg32 top = cpu.edx;
    const reg32 right = cpu.ebx;
    const reg32 bottom = cpu.ecx;
    const reg32 colour = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::drawBox(app, cpu, left, top, right, bottom, colour); });
}

/* sub_52b860: a line of text centred, the pen put back. */
reg32 frontend::watc::textCentred(App* app, CPU& cpu, reg32 text, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    constexpr reg32 kPenX = 0x5671f8, kPenY = 0x5671fc, kWindowWidth = 0x564384;
    const reg32 width = fe::direct<0x4efef0, eab::textMeasureAll>(app, cpu, text);
    const reg32 room = guest.dword(kWindowWidth) - width;
    const reg32 x = reg32(s(room) / 2);
    const reg32 penX = guest.dword(kPenX), penY = guest.dword(kPenY);
    guest::direct<0x4f0850, wata::textSetPen>(app, cpu, x, y);
    guest.leaveEdx(y);   // as the call by address left it, on to sub_4efb50
    const reg32 result = fe::direct<0x4efb50, eab::textDrawAtPen>(app, cpu, text);
    guest.setDword(kPenX, penX);
    guest.setDword(kPenY, penY);
    return result;
}

void frontend::watcTextCentred(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b860", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const reg32 text = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::textCentred(app, cpu, text, y); });
}

/* sub_52b8b0: a rect's clip to all of it. */
reg32 frontend::watc::clipWholeRect(App* app, CPU& cpu, reg32 rect)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    return fe::direct<0x52b6c0, setRectClip>(app, cpu, rect, 0, 0, guest.dword(rect + 4), guest.dword(rect + 8));
}

void frontend::watcClipWholeRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b8b0", 0, 0, kEbx | kEcx | kEdx);
    const reg32 rect = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::clipWholeRect(app, cpu, rect); });
}

/* sub_52b8d0, sub_52bb20: the memory viewer on the two drivers' tables. */
reg32 frontend::watc::viewerOnWindow(App* app, CPU& cpu)
{
    cpu.eax = 0x56b170;
    tail(app, cpu, 0x497620);
    return cpu.eax;
}

void frontend::watcViewerOnWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::viewerOnWindow(app, cpu);
}

reg32 frontend::watc::viewerOnMono(App* app, CPU& cpu)
{
    cpu.eax = 0x56b198;
    tail(app, cpu, 0x497620);
    return cpu.eax;
}

void frontend::watcViewerOnMono(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::viewerOnMono(app, cpu);
}

/* sub_52b8e0: the debug window's start. */
reg32 frontend::watc::windowStart(App* app, CPU& cpu)
{
    // sub_4ead20 called as C++ (a stop in it thrown, as its core's calls throw), what it gives back
    // in eax on to the jump.
    try
    {
        cpu.eax = guest::direct<0x4ead20, wata::windowApply>(app, cpu);
    }
    catch (const fe::Terminated&)
    {
        return cpu.eax;
    }
    if (cpu.terminate)
        return cpu.eax;
    tail(app, cpu, kTextStart);
    return cpu.eax;
}

void frontend::watcWindowStart(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::windowStart(app, cpu);
}

/* sub_52b8f0: and its end. */
reg32 frontend::watc::windowEnd(App* app, CPU& cpu)
{
    tail(app, cpu, kBitmapToScreen);
    return cpu.eax;
}

void frontend::watcWindowEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::windowEnd(app, cpu);
}

/* sub_52b910: the window driver's sizes: 0 and 1 in characters of 8 pixels, 2 and 3 in pixels. */
reg32 frontend::watc::windowSize(App* app, CPU& cpu, reg32 which)
{
    Guest guest(app, cpu);
    const auto characters = [](reg32 pixels) {
        const reg32 eighth = sar(s(pixels) < 0 ? pixels + 7 : pixels, 3);
        const reg32 sign = sar(eighth, 31);
        return (eighth ^ sign) - sign;
    };
    switch (which)
    {
    case 1: return characters(guest.dword(kScreenHeight)) - 1;
    case 2: return guest.dword(kScreenWidth);
    case 3: return guest.dword(kScreenHeight);
    case 4: return reg32(1);
    default: return characters(guest.dword(kScreenWidth));
    }
}

void frontend::watcWindowSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b910", 0, 0, kEdx);
    const reg32 which = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::windowSize(app, cpu, which); });
}

/* sub_52b970: a line printed at a character cell. */
reg32 frontend::watc::printAtCell(App* app, CPU& cpu, reg32 arguments)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x114);
    const reg32 buffer = guest.frame();
    const reg32 x = guest.dword(arguments) << 3, y = guest.dword(arguments + 4) << 3;
    guest.setDword(buffer + 0x100, arguments + 12);
    fe::direct<kVsprintf, watcom::clib::vsprintf>(guest.app(), guest.cpu(), buffer, guest.dword(arguments + 8), buffer + 0x100);
    const reg32 result = fe::direct<0x52e4e0, textAt>(app, cpu, buffer, x, y);
    guest.setDword(buffer + 0x100, 0);
    return result;
}

void frontend::watcPrintAtCell(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b970", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const reg32 arguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::printAtCell(app, cpu, arguments); });
}

/* sub_52b9d0: the driver's hook that does nothing. */
reg32 frontend::watc::nothing52b9d0(App* app, CPU& cpu)
{
    (void)app;
    return cpu.eax;
}

void frontend::watcNothing52b9d0(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::nothing52b9d0(app, cpu);
    cpu.esp += 4;
}

/* sub_52b9e0: a shape drawn on black. */
reg32 frontend::watc::blitOnBlack(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.ebx = x;
    cpu.ecx = y;
    guest.leaveEdx(cpu.ebx);
    const reg32 black = fe::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000);
    fe::direct<0x4ef1b0, eab::drawClearClip>(app, cpu, black);   // the colour as it came back in eax
    guest.leaveEbx(cpu.ecx);
    cpu.eax = shape;   // the blitter's, with edx and ebx
    return fe::direct<0x4f09c0, wata::blitDispatch>(app, cpu);
}

void frontend::watcBlitOnBlack(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52b9e0", 0, 0, kEsi);
    const reg32 shape = cpu.eax;
    const reg32 x = cpu.ebx;
    const reg32 y = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::blitOnBlack(app, cpu, shape, x, y); });
}

/* sub_52ba00: the text's colours by the debug screen's numbers. */
reg32 frontend::watc::setTextColours(App* app, CPU& cpu, reg32 ink, reg32 paper)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.edx = paper;   // across the first, which keeps it
    const reg32 inkColour = fe::direct<0x52ba70, debugColour>(app, cpu, ink);
    const reg32 paperColour = fe::direct<0x52ba70, debugColour>(app, cpu, paper);
    return fe::direct<0x4ef830, eab::fontSetColours>(app, cpu, inkColour, paperColour);
}

void frontend::watcSetTextColours(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ba00", 0, 0, kEcx);
    const reg32 ink = cpu.eax;
    const reg32 paper = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setTextColours(app, cpu, ink, paper); });
}

/* sub_52ba70: a debug-screen colour, one of the CGA's by its number, in the window's format. */
reg32 frontend::watc::debugColour(App* app, CPU& cpu, reg32 colour)
{
    static constexpr reg32 kColours[19] = {
        0xff00aaaa, 0xff000000, 0xffffffff, 0xff0000aa, 0xff000000, 0xffffffff, 0xff00aaaa, 0xffffffff, 0xff000000, 0xffaa0000,
        0xffffffff, 0xffaa0000, 0xffffff50, 0xffffffff, 0xff0000aa, 0xffaa0000, 0xffffffff, 0xffffffff, 0xffaa0000,
    };
    cpu.eax = colour > 0x12 ? 0xff00aaaa : kColours[colour];
    tail(app, cpu, kRgbToColour);
    return cpu.eax;
}

void frontend::watcDebugColour(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 colour = cpu.eax;
    watc::debugColour(app, cpu, colour);
}

namespace
{

/* sub_52bac0's body: a key waited for, its letter in capitals. */
reg32 keyWaitedFor(Guest& guest, x86::CPU& cpu)
{
    fe::direct<0x4f00f0, frontend::eab::keySetHookFlag>(guest.app(), cpu, 0x4f00e0);
    reg32 key;
    // By address: the ebx and ecx it leaves are left at the end.
    do
        key = guest.call(0x4f0120);
    while (key == 0);
    return s(key) >= 0x61 && s(key) <= 0x7a ? key - 0x20 : key;
}

}

reg32 frontend::watc::windowWaitKey(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return keyWaitedFor(guest, cpu);
}

void frontend::watcWaitKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bac0", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::windowWaitKey(app, cpu); });
}

/* sub_52baf0: a filled box in character cells, in a debug colour. */
reg32 frontend::watc::fillCells(App* app, CPU& cpu, reg32 left, reg32 top, reg32 wide, reg32 high, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.edx = top;
    cpu.ebx = wide;
    cpu.ecx = high;
    // The cell's place and size kept across it, read after it.
    const reg32 shown = fe::direct<0x52ba70, debugColour>(app, cpu, colour);
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, left << 3, cpu.edx << 3, cpu.ebx << 3, cpu.ecx << 3, shown);
}

void frontend::watcFillCells(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52baf0", 4, 0, kEsi);
    const reg32 left = cpu.eax;
    const reg32 top = cpu.edx;
    const reg32 wide = cpu.ebx;
    const reg32 high = cpu.ecx;
    const reg32 colour = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::fillCells(app, cpu, left, top, wide, high, colour); });
}

/* sub_52bb50: the mono driver's sizes. */
reg32 frontend::watc::monoSize(App* app, CPU& cpu, reg32 which)
{
    Guest guest(app, cpu);
    switch (which)
    {
    case 1: return reg32(0x18);
    case 2: return guest.dword(kScreenWidth);
    case 3: return guest.dword(kScreenHeight);
    case 4: return reg32(0);
    default: return reg32(0x50);
    }
}

void frontend::watcMonoSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bb50", 0, 0, 0);
    const reg32 which = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::monoSize(app, cpu, which); });
}

/* sub_52bb80: the mono screen cleared. */
reg32 frontend::watc::monoClearJump(App* app, CPU& cpu)
{
    tail(app, cpu, 0x4e0ca0);
    return cpu.eax;
}

void frontend::watcMonoClearJump(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::monoClearJump(app, cpu);
}

/* sub_52bb90: a shape centred on the cleared screen, on black. */
reg32 frontend::watc::showCentred(App* app, CPU& cpu, reg32 shape)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    fe::direct<0x52e420, wina::textStart>(app, cpu);
    const reg32 shapeWide = sar(guest.dword(shape + 2), 16);
    const reg32 roomX = guest::direct<0x52bb50, monoSize>(app, cpu, 2);
    const reg32 shapeHigh = sar(guest.dword(shape + 4), 16);
    const reg32 roomY = guest::direct<0x52bb50, monoSize>(app, cpu, 3);
    guest.leaveEdx(sar(roomX - shapeWide, 1));
    guest.leaveEbx(sar(roomY - shapeHigh, 1));
    const reg32 black = fe::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000);
    // The colour as it came back in eax; what that gives back in eax on to sub_4e0ca0, which reads it.
    cpu.eax = fe::direct<0x4ef1b0, eab::drawClearClip>(app, cpu, black);
    guest.call(0x4e0ca0);
    cpu.eax = shape;   // the blitter's, with edx and ebx
    fe::direct<0x4f09c0, wata::blitDispatch>(app, cpu);
    return fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
}

void frontend::watcShowCentred(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bb90", 0, 0, kEsi);
    const reg32 shape = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::showCentred(app, cpu, shape); });
}

/* sub_52bbf0: the mono screen's attribute, from a debug colour. */
reg32 frontend::watc::monoAttribute(App* app, CPU& cpu, reg32 colour)
{
    // sub_52bd00 called as C++, what it gives back in eax on to the jump.
    cpu.eax = guest::direct<0x52bd00, monoAttributeOf>(app, cpu, colour);
    if (cpu.terminate)
        return cpu.eax;
    tail(app, cpu, 0x4e0d50);
    return cpu.eax;
}

void frontend::watcMonoAttribute(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 colour = cpu.eax;
    watc::monoAttribute(app, cpu, colour);
}

/* sub_52bc00: a line printed through a window's text at a character cell. */
reg32 frontend::watc::printInWindow(App* app, CPU& cpu, reg32 arguments)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x118);
    const reg32 buffer = guest.frame();
    const reg32 x = guest.dword(arguments) << 3, y = guest.dword(arguments + 4) << 3;
    if (guest.dword(0x567294) == 0x50b580)
    {
        const reg32 edi = cpu.edi;
        guest.setDword(kErrorFile, 0x552050);
        guest.setDword(kErrorFunction, 0x552060);
        guest.setDword(kErrorLine, 0x72);
        fe::direct<kError, ea::error>(guest.app(), guest.cpu(), 0x552074, guest::Words({}));
        cpu.edi = edi;
    }
    guest.setDword(buffer + 0x100, arguments + 12);
    fe::direct<kVsprintf, watcom::clib::vsprintf>(guest.app(), guest.cpu(), buffer, guest.dword(arguments + 8), buffer + 0x100);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    fe::direct<0x52e420, wina::textStart>(app, cpu);
    fe::direct<0x52e4e0, textAt>(app, cpu, buffer, x, y);
    guest.setDword(buffer + 0x100, 0);
    return fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
}

void frontend::watcPrintInWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bc00", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const reg32 arguments = cpu.esp + 4;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::printInWindow(app, cpu, arguments); });
}

/* sub_52bd00: a debug colour's mono attribute. */
reg32 frontend::watc::monoAttributeOf(App* app, CPU& cpu, reg32 colour)
{
    static constexpr std::uint8_t kBright[19] = { 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0 };
    return colour <= 0x12 && kBright[colour] != 0 ? 0x70 : 7;
}

void frontend::watcMonoAttributeOf(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bd00", 0, 0, 0);
    const reg32 colour = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::monoAttributeOf(app, cpu, colour); });
}

/* sub_52bd20: the mono driver's key: as the window's. */
reg32 frontend::watc::monoWaitKey(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return keyWaitedFor(guest, cpu);
}

void frontend::watcMonoWaitKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bd20", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::monoWaitKey(app, cpu); });
}

/* sub_52bd50: a box of spaces on the mono screen. */
reg32 frontend::watc::monoFill(App* app, CPU& cpu, reg32 left, reg32 top, reg32 wide, reg32 high, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    constexpr reg32 kSpace = 0x5520b4;
    // What it reads of ebx after the call, kept by it.
    cpu.ebx = wide;
    cpu.ecx = top;
    const reg32 bottom = high + top;
    // The colour handed in edx too, as the game's code does; what it gives back left in eax.
    guest.leaveEdx(colour);
    cpu.eax = fe::direct<0x52bbf0, monoAttribute>(app, cpu, colour);
    reg32 row = cpu.ecx;
    if (s(bottom) > s(row))
    {
        const reg32 count = cpu.ebx;
        do
        {
            if (s(wide) > 0)
            {
                const reg32 right = left + count;
                cpu.eax = count;
                guest.leaveEbx(right);
                reg32 column = left;
                do
                {
                    guest.call(0x4e0980, args({}, { column, row, kSpace }));
                    ++column;
                    guest.leaveEdx(column);
                } while (s(column) < s(right));
            }
            ++row;
            guest.leaveEcx(row);
        } while (s(row) < s(bottom));
    }
    return guest.eax();
}

void frontend::watcMonoFill(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bd50", 4, 0, kEsi | kEdi);
    const reg32 left = cpu.eax, top = cpu.edx, wide = cpu.ebx, high = cpu.ecx;
    const reg32 colour = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::monoFill(app, cpu, left, top, wide, high, colour); });
}

// ---------------------------------------------------------------------------
// C_FILE_INFO and environ again
// ---------------------------------------------------------------------------

/* sub_52bdb0: the handles a parent hands down in C_FILE_INFO: fields handle:os handle:mode*, then
 * the variable taken away. */
reg32 frontend::watc::inheritHandles(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 40);
    const reg32 buffer = guest.frame();
    reg32 at = fe::direct<0x50ead0, watcom::clib::getenv>(guest.app(), guest.cpu(), 0x5520b8);
    if (at == 0)
        return 0;
    // A field up to its separator as a number; `at` past the separator.
    const auto field = [&](std::uint8_t separator) {
        reg32 stop = at;
        for (;; ++stop)
        {
            const std::uint8_t c = guest.byte(stop);
            if (c == separator)
                break;
            if (c == 0)
            {
                stop = 0;
                break;
            }
        }
        const reg32 length = stop - at;
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.cpu(), buffer, at, length);
        guest.setByte(buffer + length, 0);
        const reg32 value = guest::direct<0x52e660, stringToLongSigned>(app, cpu, buffer, 0, 0x10);
        at = stop + 1;
        return value;
    };
    while (guest.byte(at) != 0)
    {
        const reg32 handle = field(':');
        guest.setDword(buffer + 0xc, field(':'));
        const reg32 mode = field('*');
        guest::direct<0x51e388, setOsHandle>(app, cpu, guest.dword(buffer + 0xc), handle);
        guest.call(0x522e38, args({ handle, mode }));
    }
    return guest::direct<0x52e6d0, putEnvironment>(app, cpu, 0x5520c4);
}

void frontend::watcInheritHandles(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bdb0", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::inheritHandles(app, cpu); });
}

/* sub_52bed0: environ's own strings freed and its list emptied. */
reg32 frontend::watc::clearEnviron(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    reg32 at = guest.dword(kEnviron);
    if (at == 0)
        return 0;
    for (reg32 string = guest.dword(at); string != 0; at += 4, string = guest.dword(at))
    {
        if (guest.dword(kEnvironFlags) == 0)
            continue;
        const reg32 index = sar(at - guest.dword(kEnviron), 2);
        if (guest.byte(index + guest.dword(kEnvironFlags)) != 0)
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), string);
        guest.setDword(at, 0);
    }
    reg32 list;
    if (guest.dword(kEnvironFlags) == 0)
        list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), 5);
    else
        list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.cpu(), guest.dword(kEnviron), 5);
    if (list == 0)
        return 0xffffffff;
    guest.setDword(kEnviron, list);
    guest.setDword(list, 0);
    guest.setDword(kEnvironFlags, list + 4);
    guest.setByte(list + 4, 0);
    return 0;
}

void frontend::watcClearEnviron(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52bed0", 0, 0, kEbx | kEcx | kEdx | kEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::clearEnviron(app, cpu); });
}

/* sub_52c810: 0. */
reg32 frontend::watc::returnZero52c810(App* app, CPU& cpu)
{
    return 0;
}

void frontend::watcReturnZero52c810(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c810", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::returnZero52c810(app, cpu); });
}

/* sub_52c81c, sub_52c824, sub_52ed20: calls through the hooks of the thread start and end, and the
 * math error. */
reg32 frontend::watc::threadEndHook(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.call(guest.dword(0x56b1c4));
}

void frontend::watcThreadEndHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c81c", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::threadEndHook(app, cpu); });
}

reg32 frontend::watc::threadStartHook(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.call(guest.dword(0x56b1c8));
}

void frontend::watcThreadStartHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c824", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::threadStartHook(app, cpu); });
}

reg32 frontend::watc::mathErrorHook(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.call(guest.dword(0x567853));
}

void frontend::watcMathErrorHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ed20", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::mathErrorHook(app, cpu); });
}

/* sub_52c878: abort's report. */
reg32 frontend::watc::abnormalTermination(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest.call(kFatalError, args({ 0x5520d4, 0xff }));
}

void frontend::watcAbnormalTermination(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c878", 0, 0, kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::abnormalTermination(app, cpu); });
}

/* sub_52c88a: a double's 0 in edx:eax. */
reg32 frontend::watc::doubleZero(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.leaveEdx(0);
    return 0;
}

void frontend::watcDoubleZero(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c88a", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::doubleZero(app, cpu); });
}

/* sub_52c89b: the math error, and a double's infinity, in edx:eax, of the sign in what it returns. */
reg32 frontend::watc::doubleInfinityHook(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    cpu.eax = 0x83;   // on through it to the hook
    const reg32 sign = fe::direct<0x52ed20, mathErrorHook>(app, cpu);
    guest.leaveEdx((sign & 0x80000000) | 0x7ff00000);
    return 0;
}

void frontend::watcDoubleInfinityHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c89b", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::doubleInfinityHook(app, cpu); });
}

/* sub_52c8a7: errno's ERANGE, the math error, and a double's infinity of the sign in eax. */
reg32 frontend::watc::doubleInfinity(App* app, CPU& cpu, reg32 sign)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest.call(0x5028a4);
    cpu.eax = 0x84;   // on through it to the hook
    fe::direct<0x52ed20, mathErrorHook>(app, cpu);
    guest.leaveEdx((sign & 0x80000000) | 0x7ff00000);
    return 0;
}

void frontend::watcDoubleInfinity(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52c8a7", 0, 0, 0);
    const reg32 sign = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::doubleInfinity(app, cpu, sign); });
}

/* sub_52e3f8: the driver's hook, 0. */
reg32 frontend::watc::zeroJump(App* app, CPU& cpu)
{
    tail(app, cpu, 0x52edfa);
    return cpu.eax;
}

void frontend::watcZeroJump(win32::WinApplication* app, x86::CPU& cpu)
{
    watc::zeroJump(app, cpu);
}

/* sub_52e4e0: a line of text at a position. */
reg32 frontend::watc::textAt(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.edx = x;
    cpu.ebx = y;
    guest::direct<0x4f0850, wata::textSetPen>(app, cpu, x, y);
    guest.leaveEdx(y);   // as the call by address left it, on to sub_52e440
    return fe::direct<0x52e440, menz::textBackgroundBox>(app, cpu, text);
}

void frontend::watcTextAt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e4e0", 0, 0, kEcx);
    const reg32 text = cpu.eax;
    const reg32 x = cpu.edx;
    const reg32 y = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::textAt(app, cpu, text, x, y); });
}

/* sub_52e500: strtol and strtoul: ecx 1 for the signed. */
reg32 frontend::watc::stringToLong(App* app, CPU& cpu, reg32 text, reg32 end, reg32 radix, reg32 isSigned)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 28);
    cpu.ebx = radix;
    cpu.ecx = isSigned;
    reg32 base = cpu.ebx;
    constexpr reg32 kCharacterClass = 0x564ef0, kOverflowLimits = 0x56b648;
    constexpr std::uint8_t kSpace = 2;
    constexpr reg32 kEinval = 0xd, kErange = 0xe;
    if (end != 0)
        guest.setDword(end, text);
    reg32 at = text;
    while ((guest.byte(kCharacterClass + std::uint8_t(guest.byte(at) + 1)) & kSpace) != 0)
        ++at;
    const std::uint8_t sign = guest.byte(at);
    reg32 ecx = (cpu.ecx & 0xffff00ff) | (reg32(sign) << 8);
    if (sign == '+' || sign == '-')
        ++at;
    const auto hasHexPrefix = [&] {
        if (guest.byte(at) != '0')
            return false;
        const std::uint8_t x = guest.byte(at + 1);
        return x == 'x' || x == 'X';
    };
    if (base == 0)
    {
        if (guest.byte(at) == '0')
            ecx = (ecx & 0xffffff00) | guest.byte(at + 1);
        if (hasHexPrefix())
        {
            base = 0x10;
            at += 2;
        }
        else
            base = guest.byte(at) == '0' ? 8 : 10;
    }
    else if (s(base) < 2 || s(base) > 0x24)
    {
        guest.leaveEcx(ecx);
        guest.leaveEdx(at);
        guest.call(kSetErrno, args({ kEinval }));
        return 0;
    }
    else if (base == 0x10 && hasHexPrefix())
    {
        at += 2;
    }
    const reg32 start = at;
    const reg32 limit = guest.dword(kOverflowLimits + base * 4);
    bool overflow = false;
    reg32 value = 0;
    ecx &= 0xffffff00;
    for (;;)
    {
        const reg32 digit = fe::direct<0x52e670, digitValue>(app, cpu, std::uint8_t(guest.byte(at)));
        if (s(digit) >= s(base))
            break;
        if (value > limit)
            overflow = true;
        const reg32 before = value;
        value = value * base + digit;
        if (value < before)
            overflow = true;
        ++at;
    }
    if (at == start)
        at = text;
    if (end != 0)
        guest.setDword(end, at);
    if (overflow)
        ecx |= 1;
    guest.leaveEcx(ecx);
    guest.leaveEbx(value);
    guest.leaveEdx(at);
    if ((isSigned == 1 && value >= 0x80000000 && (value != 0x80000000 || sign != '-')) || overflow)
    {
        guest.call(kSetErrno, args({ kErange }));
        if (isSigned == 0)
            return 0xffffffff;
        return sign == '-' ? 0x80000000 : 0x7fffffff;
    }
    if (sign == '-')
    {
        value = reg32(0) - value;
        guest.leaveEbx(value);
    }
    return value;
}

void frontend::watcStrtol(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e500", 0, 0, kEsi | kEdi);
    const reg32 text = cpu.eax, end = cpu.edx, isSigned = cpu.ecx;
    const reg32 radix = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::stringToLong(app, cpu, text, end, radix, isSigned); });
}

/* sub_52e660: strtol. */
reg32 frontend::watc::stringToLongSigned(App* app, CPU& cpu, reg32 text, reg32 end, reg32 radix)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    // On to sub_52e500 with them as they came, signed.
    return fe::direct<0x52e500, stringToLong>(app, cpu, text, end, radix, 1);
}

void frontend::watcStrtolSigned(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e660", 0, 0, kEcx);
    const reg32 text = cpu.eax, end = cpu.edx, radix = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::stringToLongSigned(app, cpu, text, end, radix); });
}

/* sub_52e670: a character's value as a digit of any base, 0x25 for none. */
reg32 frontend::watc::digitValue(App* app, CPU& cpu, std::uint8_t c)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (c >= '0' && c <= '9')
        return reg32(c) - 0x30;
    const std::uint8_t lower = std::uint8_t(fe::direct<0x4f0ae0, watcom::clib::tolower>(guest.app(), guest.cpu(), c));
    if (lower >= 'a' && lower <= 'z')
        return reg32(lower) - 0x57;
    return 0x25;
}

void frontend::watcDigitValue(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e670", 0, 0, kEdx);
    const std::uint8_t c = std::uint8_t(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::digitValue(app, cpu, c); });
}

/* sub_52e6d0: putenv: Windows' variable set, then environ, then the wide one if there is one. */
reg32 frontend::watc::putEnvironment(App* app, CPU& cpu, reg32 text)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 48);
    constexpr reg32 kFailed = 0xffffffff;
    constexpr reg32 kWideEnviron = 0xa0b158;
    const reg32 equals = guest::direct<0x52ee60, mbschr>(app, cpu, text, 0x3d);
    if (equals == 0 || equals == text)
        return kFailed;
    const reg32 nameLength = equals - text;
    const reg32 name = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), nameLength + 1);
    if (name == 0)
        return kFailed;
    copyUp(guest, name, text, nameLength);
    guest.setByte(name + nameLength, 0);
    const reg32 from = equals + 1;
    const reg32 valueLength = guest.call(kMbslen, args({ from }));
    reg32 value = 0;
    if (valueLength != 0)
    {
        value = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), valueLength + 1);
        if (value == 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), name);
            return kFailed;
        }
        copyUp(guest, value, from, valueLength);
        guest.setByte(value + valueLength, 0);
    }
    const reg32 set = callImport<kSetEnvironmentVariable, &win32::kernel32::SetEnvironmentVariableA>(guest, cpu, name, value);
    fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), name);
    fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), value);
    if (set == 0)
        return kFailed;
    if (guest::direct<0x52e88c, environSet>(app, cpu, text) != 0)
        return kFailed;
    if (guest.dword(kWideEnviron) == 0)
        return 0;
    const reg32 characters = guest.call(kMbslen, args({ text })) + 1;
    const reg32 wide = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), 2 * characters);
    if (wide == 0)
    {
        guest.call(0x500f8c, args({ 5 }));
        return kFailed;
    }
    if (guest.call(0x518080, args({ wide, text, characters })) == kFailed)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), wide);
        return kFailed;
    }
    return guest.call(0x52c178, args({ wide }));
}

void frontend::watcPutenv(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e6d0", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::putEnvironment(app, cpu, text); });
}

/* sub_52e88c: the string put in environ, or the variable taken out if it has nothing after =. */
reg32 frontend::watc::environSet(App* app, CPU& cpu, reg32 text)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 36);
    constexpr reg32 kFailed = 0xffffffff;
    if (text == 0)
        return kFailed;
    reg32 at = cpu.edx;
    if (guest.byte(text) != 0)
    {
        at = text + 1;
        if (guest.byte(at) != 0)
        {
            while (guest.byte(at) != '=')
            {
                const std::uint8_t next = guest.byte(at + 1);
                ++at;
                if (next == 0)
                    break;
            }
        }
    }
    if (guest.byte(at) == 0)
        return kFailed;
    const bool removing = guest.byte(at + 1) == 0;
    reg32 list = guest.dword(kEnviron);
    reg32 index = 0;
    if (list == 0)
    {
        if (removing)
            return 0;
        list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), 9);
        if (list == 0)
            return kFailed;
        guest.setDword(kEnviron, list);
        guest.setDword(list, 0);
        guest.setDword(list + 4, 0);
        guest.setDword(kEnvironFlags, list + 8);
    }
    else
    {
        const reg32 found = guest::direct<0x52ea3c, environFind>(app, cpu, text, removing ? 1u : 0u);
        if (removing)
            return 0;
        if (s(found) > 0)
        {
            index = found - 1;
        }
        else
        {
            // New: it has -found strings; the list one longer, and the flags after it.
            index = reg32(0) - found;
            const reg32 flagCount = index + 1;
            const reg32 listBytes = index * 4 + 8;
            const reg32 flags = guest.dword(kEnvironFlags);
            if (flags == 0)
            {
                const reg32 grown = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), listBytes + flagCount);
                list = grown;
                if (grown == 0)
                    return kFailed;
                copyUp(guest, grown, guest.dword(kEnviron), index * 4);
                guest.setDword(kEnvironFlags, listBytes + grown);
                fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), listBytes + grown, 0, flagCount);
            }
            else
            {
                list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.cpu(), list, listBytes + flagCount);
                if (list == 0)
                    return kFailed;
                const reg32 moved = listBytes + list;
                guest.call(kMemmove, args({ moved, guest.dword(kEnvironFlags), index }));
                guest.setDword(kEnvironFlags, moved);
            }
            guest.setDword(kEnviron, list);
            guest.setDword(list + index * 4 + 4, 0);
        }
    }
    guest.setDword(list + index * 4, text);
    guest.setByte(guest.dword(kEnvironFlags) + index, 0);
    return 0;
}

void frontend::watcEnvironSet(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52e88c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::environSet(app, cpu, text); });
}

/* sub_52ea3c: a variable looked for in environ, case aside: its index + 1, or with edx set the
 * variable taken out and 0; if not there minus the count of strings. */
reg32 frontend::watc::environFind(App* app, CPU& cpu, reg32 name, reg32 removing)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    reg32 entry = guest.dword(kEnviron);
    for (;; entry += 4)
    {
        reg32 string = guest.dword(entry);
        guest.leaveEdx(string);
        if (string == 0)
            return sar(guest.dword(kEnviron) - entry, 2);
        reg32 wanted = name;
        if (guest.byte(wanted) == 0)
            continue;
        bool same = false;
        for (;;)
        {
            const reg32 upperOwn = fe::direct<kToupper, watcom::clib::toupper>(guest.app(), guest.cpu(), guest.byte(string));
            const reg32 upperWanted = fe::direct<kToupper, watcom::clib::toupper>(guest.app(), guest.cpu(), guest.byte(wanted));
            if (upperOwn != upperWanted)
                break;
            if (guest.byte(string) == '=')
            {
                same = true;
                break;
            }
            ++string;
            const std::uint8_t next = guest.byte(wanted + 1);
            ++wanted;
            if (next == 0)
                break;
        }
        if (!same)
            continue;
        reg32 index = sar(entry - guest.dword(kEnviron), 2);
        if (removing == 0)
        {
            guest.leaveEdx(guest.dword(kEnviron));
            return index + 1;
        }
        const reg32 removed = guest.dword(entry);
        guest.leaveEdx(removed);
        reg32 last = entry;
        if (removed != 0)
        {
            reg32 next;
            do
            {
                guest.setDword(last, guest.dword(last + 4));
                next = guest.dword(last + 4);
                last += 4;
            } while (next != 0);
        }
        const reg32 flags = guest.dword(kEnvironFlags);
        if (flags != 0)
        {
            if (guest.byte(index + flags) != 0)
                fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), removed);
            const reg32 count = sar(last - guest.dword(kEnviron), 2);
            guest.call(kMemmove, args({ last, guest.dword(kEnvironFlags), count }));
            guest.setDword(kEnvironFlags, last);
            for (; s(index) < s(count); ++index)
            {
                const std::uint8_t flag = guest.byte(index + last + 1);
                guest.leaveEdx((guest.edx() & 0xffffff00) | flag);
                guest.setByte(index + last, flag);
            }
        }
        return 0;
    }
}

void frontend::watcEnvironFind(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ea3c", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const reg32 name = cpu.eax, removing = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::environFind(app, cpu, name, removing); });
}

/* sub_52eb70: a thread's first function: its data made, _beginthread told, the function called,
 * the thread's end. */
reg32 frontend::watc::threadStart(App* app, CPU& cpu, reg32 parameters)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 40);
    constexpr reg32 kSingleThread = 0x9f6420;
    const reg32 frame = guest.frame();
    const reg32 function = guest.dword(parameters), argument = guest.dword(parameters + 4);
    const reg32 single = guest.dword(kSingleThread);
    guest.leaveEdx(single);
    guest.setDword(frame + 8, function);
    if (single == 0)
    {
        // The data on the stack, for a thread of the program's.
        cpu.esp -= (guest.dword(kThreadDataSize) + 3) & 0xfffffffc;
        const reg32 data = cpu.esp;
        guest.leaveEcx(data);
        const reg32 size = guest.dword(kThreadDataSize);
        fe::direct<0x4e0640, watcom::clib::memset>(app, cpu, data, 0u, size);
        guest.leaveEbx(size);  // as the code loads it for it, which it keeps
        guest.setDword(data + 0xf0, guest.dword(kThreadDataSize));
        cpu.eax = data;   // on through it to sub_51e7ec: the block to take
        if (fe::direct<0x51e888, attachThread>(app, cpu) == 0)
            return 0;
    }
    const reg32 own = guest.call(guest.dword(kThreadDataGetter)) + 0xde;
    guest.setDword(parameters + 0x10, own);
    callImport<kSetEvent, &host::setEvent>(guest, cpu, guest.dword(parameters + 0xc));
    guest.call(0x51f0d0, args({ frame }));
    guest.call(guest.dword(kThreadOnStart));
    guest.call(guest.dword(frame + 8), args({ argument }));
    return fe::direct<0x52c81c, threadEndHook>(app, cpu);   // the registers on to the hook as they are
}

void frontend::watcThreadStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52eb70", 0, 0, kEbx | kEsi | kEdi);
    const reg32 parameters = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::threadStart(app, cpu, parameters); });
}

/* sub_52ec04: _beginthread: the thread made and waited for until it has its data. */
reg32 frontend::watc::beginThread(App* app, CPU& cpu, reg32 function, reg32 stackSize, reg32 argument)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x4c);
    cpu.ebx = argument;
    const reg32 frame = guest.frame();
    constexpr reg32 kEventName = 0x552134;
    if (guest.dword(kTlsIndex) == 0xffffffff)
    {
        if (fe::direct<0x51e828, allocTlsIndex>(app, cpu) == 0)
            return 0;
        fe::direct<0x51e964, initThreads>(app, cpu);
    }
    guest.setDword(frame + 0x20, function);
    guest.setDword(frame + 0x24, cpu.ebx);
    guest.setDword(frame + 0x28, callImport<kGetCurrentThread, &host::currentThread>(guest, cpu));
    guest.setDword(frame + 0x30, 0);
    copyUp(guest, frame, kEventName, 9);
    const reg32 thread = callImport<kGetCurrentThreadId, &host::currentThreadId>(guest, cpu);
    guest.call(0x51e0ec, args({ thread, frame + 8, 0x10 }));
    cpu.eax = frame;
    const reg32 event = callImport<kCreateEvent, &host::createEvent>(guest, cpu, 0, 0, 0, frame);
    guest.setDword(frame + 0x2c, event);
    cpu.eax = frame + 0x20;
    const reg32 handle = callImport<kCreateThread, &host::createThread>(
        guest, cpu, 0, (stackSize + 0xfff) & 0xfffff000, 0x52eb70, frame + 0x20, 0, frame + 0x34);
    guest.leaveEbx(handle);
    if (handle == 0)
    {
        guest.setDword(frame + 0x34, 0xffffffff);
    }
    else
    {
        callImport<kWaitForSingleObject, &host::waitOne>(guest, cpu, guest.dword(frame + 0x2c), 0xffffffff);
        const reg32 slot = guest.dword(frame + 0x30);
        if (slot != 0)
            guest.setDword(slot, handle);
        else
            callImport<kCloseHandle, &win32::kernel32::CloseHandle>(guest, cpu, handle);
    }
    callImport<kCloseHandle, &win32::kernel32::CloseHandle>(guest, cpu, guest.dword(frame + 0x2c));
    return handle;
}

void frontend::watcBeginThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ec04", 0, 0, kEcx | kEsi | kEdi);
    const reg32 function = cpu.eax, stackSize = cpu.edx;
    const reg32 argument = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::beginThread(app, cpu, function, stackSize, argument); });
}

/* sub_52ecf0: _endthread. */
reg32 frontend::watc::endThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    constexpr reg32 kSingleThread = 0x9f6420;
    guest.call(guest.dword(kThreadOnEnd));
    guest.call(0x51f11c);
    if (guest.dword(kSingleThread) == 0)
    {
        cpu.eax = 1;   // as the game's code hands it
        fe::direct<0x51e8dc, detachThread>(app, cpu, 1);
    }
    callImport<kExitThread, &host::exitThread>(guest, cpu, 0);
    return guest.eax();
}

void frontend::watcEndThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ecf0", 0, 0, kEcx | kEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::endThread(app, cpu); });
}

/* sub_52edfa: 0. */
reg32 frontend::watc::returnZero52edfa(App* app, CPU& cpu)
{
    return 0;
}

void frontend::watcReturnZero52edfa(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52edfa", 0, 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::returnZero52edfa(app, cpu); });
}

/* sub_52ee00: the window's clip, held inside the screen. */
reg32 frontend::watc::setClip(App* app, CPU& cpu, reg32 leftAsked, reg32 topAsked, reg32 rightAsked, reg32 bottomAsked)
{
    Guest guest(app, cpu);
    cpu.eax = leftAsked;
    cpu.edx = topAsked;
    cpu.ebx = rightAsked;
    cpu.ecx = bottomAsked;
    const auto held = [](reg32 value, reg32 most) {
        const reg32 positive = s(value) < 0 ? 0 : value;
        return s(positive) > s(most) ? most : positive;
    };
    const reg32 wide = guest.dword(kScreenWidth), high = guest.dword(kScreenHeight);
    const reg32 left = held(cpu.eax, wide), right = held(cpu.ebx, wide);
    const reg32 top = held(cpu.edx, high), bottom = held(cpu.ecx, high);
    guest.setDword(kClipRight, right);
    guest.setDword(kClipTop, top);
    guest.setDword(kClipBottom, bottom);
    guest.setDword(kClipLeft, left);
    guest.leaveEbx(right);
    guest.leaveEdx(top);
    guest.leaveEcx(bottom);
    return left;
}

void frontend::watcSetClip(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ee00", 0, 0, kEsi);
    const reg32 leftAsked = cpu.eax;
    const reg32 topAsked = cpu.edx;
    const reg32 rightAsked = cpu.ebx;
    const reg32 bottomAsked = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::setClip(app, cpu, leftAsked, topAsked, rightAsked, bottomAsked); });
}

/* sub_52ee60: _mbschr: the multibyte character found, or the end if asked for 0. */
reg32 frontend::watc::mbschr(App* app, CPU& cpu, reg32 text, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    cpu.eax = text;
    reg32 at = cpu.eax;
    constexpr reg32 kMbterm = 0x5250c0;
    const reg32 character = guest.frame();
    guest.call(0x52ede0, args({ wanted, character }));
    guest.leaveEdx(cpu.edx & 0xffffff00);
    const reg32 length = guest.call(0x52b5b0, args({ character }));
    guest.setByte(character + length, std::uint8_t(cpu.edx));
    while (guest.call(kMbterm, args({ at })) == 0 && guest.call(0x52e3a0, args({ at, character })) != 0)
        at = guest.call(0x525100, args({ at }));
    if (guest.call(kMbterm, args({ at })) != 0 && wanted != 0)
        return 0;
    return at;
}

void frontend::watcMbschr(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52ee60", 0, 0, kEbx | kEcx);
    const reg32 wanted = cpu.edx;
    const reg32 text = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::mbschr(app, cpu, text, wanted); });
}

// ---------------------------------------------------------------------------
// TAPI32.DLL, loaded when the modem code first asks
// ---------------------------------------------------------------------------

namespace
{

constexpr reg32 kLineUnavailable = 0x80000049;  // LINEERR_OPERATIONUNAVAIL

/* A stub of TAPI's: the function found the first time, and called with what the stub was. */
template <std::size_t Count>
reg32 tapiStub(win32::WinApplication* app, x86::CPU& cpu, reg32 frameBytes, reg32 slot, reg32 name,
               const std::array<reg32, Count>& words)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, frameBytes);
    if (guest.dword(slot) == 0)
    {
        const reg32 failed = fe::direct<0x52f318, frontend::watc::tapiFind>(app, cpu, name, slot);
        if (failed != 0)
        {
            guest.setDword(slot, 0xffffffff);
            return failed;
        }
    }
    if (guest.dword(slot) == 0xffffffff)
        return kLineUnavailable;
    fe::Arguments arguments = args({ kLineUnavailable });
    for (const reg32 word : words)
        arguments.stack[arguments.stackCount++] = word;
    return guest.call(guest.dword(slot), arguments);
}

}

/* sub_52f318: TAPI32.DLL loaded if it is not, a function looked up into its slot. */
reg32 frontend::watc::tapiFind(App* app, CPU& cpu, reg32 name, reg32 slot)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0);
    constexpr reg32 kTapi = 0x56d12c, kTapiName = 0x56b9d8;
    if (guest.dword(kTapi) == 0)
    {
        const reg32 library = callImport<kLoadLibrary, &win32::kernel32::LoadLibraryA>(guest, cpu, kTapiName);
        guest.setDword(kTapi, library);
        if (library == 0)
            guest.setDword(kTapi, 0xffffffff);
    }
    if (guest.dword(kTapi) == 0xffffffff)
        return kLineUnavailable;
    const reg32 found = callImport<kGetProcAddress, &win32::kernel32::GetProcAddress>(guest, cpu, guest.dword(kTapi), name);
    guest.leaveEcx(slot);
    guest.setDword(slot, found);
    return found == 0 ? kLineUnavailable : 0;
}

void frontend::watcTapiFind(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52f318", 8, 0, 0);
    const reg32 name = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 slot = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::tapiFind(app, cpu, name, slot); });
}

/* sub_52f535: lineAnswer. */
reg32 frontend::watc::lineAnswer(App* app, CPU& cpu, reg32 callHandle, reg32 userUserInfo, reg32 size)
{
    return tapiStub<3>(app, cpu, 0, 0x56d14c, 0x56ba50, { callHandle, userUserInfo, size });
}

void frontend::watcLineAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52f535", 12, 0, 0);
    const reg32 callHandle = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 userUserInfo = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 size = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineAnswer(app, cpu, callHandle, userUserInfo, size); });
}

/* sub_52f661: lineClose. */
reg32 frontend::watc::lineClose(App* app, CPU& cpu, reg32 line)
{
    return tapiStub<1>(app, cpu, 0, 0x56d15c, 0x56ba98, { line });
}

void frontend::watcLineClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52f661", 4, 0, 0);
    const reg32 line = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineClose(app, cpu, line); });
}

/* sub_52f969: lineDeallocateCall. */
reg32 frontend::watc::lineDeallocateCall(App* app, CPU& cpu, reg32 callHandle)
{
    return tapiStub<1>(app, cpu, 0, 0x56d184, 0x56bb68, { callHandle });
}

void frontend::watcLineDeallocateCall(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52f969", 4, 0, 0);
    const reg32 callHandle = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineDeallocateCall(app, cpu, callHandle); });
}

/* sub_52fb2e: lineDrop. */
reg32 frontend::watc::lineDrop(App* app, CPU& cpu, reg32 callHandle, reg32 userUserInfo, reg32 size)
{
    return tapiStub<3>(app, cpu, 0, 0x56d19c, 0x56bbc8, { callHandle, userUserInfo, size });
}

void frontend::watcLineDrop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_52fb2e", 12, 0, 0);
    const reg32 callHandle = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 userUserInfo = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 size = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineDrop(app, cpu, callHandle, userUserInfo, size); });
}

/* sub_530747: lineGetDevCaps. */
reg32 frontend::watc::lineGetDevCaps(App* app, CPU& cpu, reg32 lineApp, reg32 device, reg32 apiVersion, reg32 extVersion, reg32 caps)
{
    return tapiStub<5>(app, cpu, 4, 0x56d238, 0x56bed4, { lineApp, device, apiVersion, extVersion, caps });
}

void frontend::watcLineGetDevCaps(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_530747", 20, 0, 0);
    const reg32 lineApp = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 device = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 apiVersion = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 extVersion = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 caps = app->getMemory<reg32>(cpu.esp + 20);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineGetDevCaps(app, cpu, lineApp, device, apiVersion, extVersion, caps); });
}

/* sub_53083d: lineGetDevConfig. */
reg32 frontend::watc::lineGetDevConfig(App* app, CPU& cpu, reg32 device, reg32 config, reg32 deviceClass)
{
    return tapiStub<3>(app, cpu, 0, 0x56d244, 0x56bf04, { device, config, deviceClass });
}

void frontend::watcLineGetDevConfig(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_53083d", 12, 0, 0);
    const reg32 device = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 config = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 deviceClass = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineGetDevConfig(app, cpu, device, config, deviceClass); });
}

/* sub_530a4e: lineGetID. */
reg32 frontend::watc::lineGetID(App* app, CPU& cpu, reg32 line, reg32 address, reg32 callHandle, reg32 select, reg32 deviceId, reg32 deviceClass)
{
    return tapiStub<6>(app, cpu, 4, 0x56d260, 0x56bf7c, { line, address, callHandle, select, deviceId, deviceClass });
}

void frontend::watcLineGetID(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_530a4e", 24, 0, 0);
    const reg32 line = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 address = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 callHandle = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 select = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 deviceId = app->getMemory<reg32>(cpu.esp + 20);
    const reg32 deviceClass = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineGetID(app, cpu, line, address, callHandle, select, deviceId, deviceClass); });
}

/* sub_530b4d: lineGetLineDevStatus. */
reg32 frontend::watc::lineGetLineDevStatus(App* app, CPU& cpu, reg32 line, reg32 status)
{
    return tapiStub<2>(app, cpu, 0, 0x56d26c, 0x56bfa0, { line, status });
}

void frontend::watcLineGetLineDevStatus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_530b4d", 8, 0, 0);
    const reg32 line = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 status = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineGetLineDevStatus(app, cpu, line, status); });
}

/* sub_5310be: lineInitialize. */
reg32 frontend::watc::lineInitialize(App* app, CPU& cpu, reg32 lineAppOut, reg32 instance, reg32 callback, reg32 appName, reg32 devicesOut)
{
    return tapiStub<5>(app, cpu, 4, 0x56d2b8, 0x56c114, { lineAppOut, instance, callback, appName, devicesOut });
}

void frontend::watcLineInitialize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5310be", 20, 0, 0);
    const reg32 lineAppOut = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 instance = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 callback = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 appName = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 devicesOut = app->getMemory<reg32>(cpu.esp + 20);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineInitialize(app, cpu, lineAppOut, instance, callback, appName, devicesOut); });
}

/* sub_5311c0: lineMakeCall. */
reg32 frontend::watc::lineMakeCall(App* app, CPU& cpu, reg32 line, reg32 callOut, reg32 destination, reg32 country, reg32 callParams)
{
    return tapiStub<5>(app, cpu, 4, 0x56d2c4, 0x56c14c, { line, callOut, destination, country, callParams });
}

void frontend::watcLineMakeCall(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5311c0", 20, 0, 0);
    const reg32 line = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 callOut = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 destination = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 country = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 callParams = app->getMemory<reg32>(cpu.esp + 20);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineMakeCall(app, cpu, line, callOut, destination, country, callParams); });
}

/* sub_53138f: lineNegotiateAPIVersion. */
reg32 frontend::watc::lineNegotiateAPIVersion(App* app, CPU& cpu, reg32 lineApp, reg32 device, reg32 lowVersion, reg32 highVersion, reg32 versionOut, reg32 extensionId)
{
    return tapiStub<6>(app, cpu, 4, 0x56d2dc, 0x56c1b8, { lineApp, device, lowVersion, highVersion, versionOut, extensionId });
}

void frontend::watcLineNegotiateAPIVersion(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_53138f", 24, 0, 0);
    const reg32 lineApp = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 device = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 lowVersion = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 highVersion = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 versionOut = app->getMemory<reg32>(cpu.esp + 20);
    const reg32 extensionId = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineNegotiateAPIVersion(app, cpu, lineApp, device, lowVersion, highVersion, versionOut, extensionId); });
}

/* sub_531439: lineOpen. */
reg32 frontend::watc::lineOpen(App* app, CPU& cpu, reg32 lineApp, reg32 device, reg32 lineOut, reg32 apiVersion, reg32 extVersion, reg32 callbackInstance, reg32 privileges, reg32 mediaModes, reg32 callParams)
{
    return tapiStub<9>(app, cpu, 4, 0x56d2e4, 0x56c1e8, { lineApp, device, lineOut, apiVersion, extVersion, callbackInstance, privileges, mediaModes, callParams });
}

void frontend::watcLineOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_531439", 36, 0, 0);
    const reg32 lineApp = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 device = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 lineOut = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 apiVersion = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 extVersion = app->getMemory<reg32>(cpu.esp + 20);
    const reg32 callbackInstance = app->getMemory<reg32>(cpu.esp + 24);
    const reg32 privileges = app->getMemory<reg32>(cpu.esp + 28);
    const reg32 mediaModes = app->getMemory<reg32>(cpu.esp + 32);
    const reg32 callParams = app->getMemory<reg32>(cpu.esp + 36);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineOpen(app, cpu, lineApp, device, lineOut, apiVersion, extVersion, callbackInstance, privileges, mediaModes, callParams); });
}

/* sub_531f31: lineSetDevConfig. */
reg32 frontend::watc::lineSetDevConfig(App* app, CPU& cpu, reg32 device, reg32 config, reg32 size, reg32 deviceClass)
{
    return tapiStub<4>(app, cpu, 4, 0x56d374, 0x56c4a8, { device, config, size, deviceClass });
}

void frontend::watcLineSetDevConfig(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_531f31", 16, 0, 0);
    const reg32 device = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 config = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 size = app->getMemory<reg32>(cpu.esp + 12);
    const reg32 deviceClass = app->getMemory<reg32>(cpu.esp + 16);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineSetDevConfig(app, cpu, device, config, size, deviceClass); });
}

/* sub_532162: lineSetStatusMessages. */
reg32 frontend::watc::lineSetStatusMessages(App* app, CPU& cpu, reg32 line, reg32 lineStates, reg32 addressStates)
{
    return tapiStub<3>(app, cpu, 0, 0x56d390, 0x56c534, { line, lineStates, addressStates });
}

void frontend::watcLineSetStatusMessages(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_532162", 12, 0, 0);
    const reg32 line = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 lineStates = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 addressStates = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineSetStatusMessages(app, cpu, line, lineStates, addressStates); });
}

/* sub_5324d2: lineShutdown. */
reg32 frontend::watc::lineShutdown(App* app, CPU& cpu, reg32 lineApp)
{
    return tapiStub<1>(app, cpu, 0, 0x56d3bc, 0x56c614, { lineApp });
}

void frontend::watcLineShutdown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5324d2", 4, 0, 0);
    const reg32 lineApp = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watc::lineShutdown(app, cpu, lineApp); });
}

}
