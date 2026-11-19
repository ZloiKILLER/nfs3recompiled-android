#include "native_frontend_watc.h"
#include "native_ea_formats.h"
#include "native_ea_support.h"
#include "native_game.h"
#include "native_watcom.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_wina.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_guest.h"
#include "os_threads.h"
#include <nfs3hp.h>
#include "os_files.h"
#include "os_system.h"

#include <array>
#include <optional>
#include <utility>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 watcFpNotLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetMbcpZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetOsHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcNothing51e540(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcNothing51f270(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcNewSemaphore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDeleteKeptSemaphores(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDeleteExtraSemaphores(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcClearLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAllocHandleHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeHandleHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockFileTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockFileTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLockHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlockHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcUnlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAllocTlsIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAttachThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDetachThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcEndThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeTlsIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcInitThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFiniThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcHasActiveWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcReportAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcReportException(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcExceptionHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFormatFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcScanFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcShapeRowBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcPutPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcBlend4To8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcWritePixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcInitEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetMbcp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAttachThisThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcResizeThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAddThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcRemoveThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFreeThreadListJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcClearFpExceptions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetSignalHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSignalHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSignalMask(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSignalForCode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcConsoleControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcConsoleWanted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcConsoleUnhook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcRaiseFp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcRaise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcInitThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFiniThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcHookThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcPowError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetRectClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDrawBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcTextCentred(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcClipWholeRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcViewerOnWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcViewerOnMono(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcWindowStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcWindowEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcWindowSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcPrintAtCell(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcNothing52b9d0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcBlitOnBlack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetTextColours(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDebugColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcWaitKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcFillCells(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoClearJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcShowCentred(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcPrintInWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoAttributeOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoWaitKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMonoFill(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcInheritHandles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcClearEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcReturnZero52c810(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcThreadEndHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcThreadStartHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMathErrorHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcAbnormalTermination(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDoubleZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDoubleInfinityHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDoubleInfinity(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcZeroJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcTextAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcStrtol(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcStrtolSigned(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcDigitValue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcPutenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcEnvironSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcEnvironFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcThreadStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcBeginThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcEndThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcReturnZero52edfa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcSetClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcMbschr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcTapiFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineDeallocateCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineGetDevCaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineGetDevConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineGetID(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineGetLineDevStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineInitialize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineMakeCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineNegotiateAPIVersion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineSetDevConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineSetStatusMessages(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watcLineShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

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
 * The threads, locks, events and time are the port's own (os_threads.h). */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;

constexpr std::uint8_t kEbx = fe::kRestoreEbx, kEcx = fe::kRestoreEcx, kEdx = fe::kRestoreEdx;
constexpr std::uint8_t kEsi = fe::kRestoreEsi, kEdi = fe::kRestoreEdi;

fe::Site makeSite(const char* name, u32 stackBytes, u32 frameBytes, std::uint8_t restores)
{
    fe::Site site{};
    site.name = name;
    site.stackBytes = stackBytes;
    site.frameBytes = frameBytes;
    site.restores = restores;
    return site;
}

s32 s(u32 value)
{
    return s32(value);
}

u32 sar(u32 value, unsigned bits)
{
    return u32(s32(value) >> bits);
}

/* A function called as C++ (fe::direct) from a core that is written register by register: nothing
 * thrown when the game stopped in it -- ts.terminate set, 0 back -- as such a core's callers have
 * it. */
template <u32 Address, auto Core, typename... Args>
u32 callTo(nfs::App* app, nfs::ThreadState& ts, Args... args)
{
    try
    {
        return u32(fe::direct<Address, Core>(app, ts, args...));
    }
    catch (const fe::Terminated&)
    {
        return 0;
    }
}

/* The function such a core jumps to, called where the jump is: its return address's slot the
 * jump's caller's, so its frame lies where the jump puts it.  The core's thunk hands the result
 * back in eax and takes the return address off, as the function jumped to did. */
template <u32 Address, auto Core, typename... Args>
u32 jumpTo(nfs::App* app, nfs::ThreadState& ts, Args... args)
{
    ts.sp += 4;
    const u32 result = callTo<Address, Core>(app, ts, args...);
    ts.sp -= 4;
    return result;
}

/* The same to a function of another file's with no typed core yet: by address, `arguments` in its
 * registers. */
u32 jumpByAddress(nfs::App* app, nfs::ThreadState& ts, u32 target, const fe::Arguments& arguments)
{
    ts.sp += 4;
    u32 result = 0;
    try
    {
        fe::Guest guest(app, ts);
        result = fe::pointerCall(guest, target, arguments);
    }
    catch (const fe::Terminated&)
    {
    }
    ts.sp -= 4;
    return result;
}

/* The inline memcpy: dwords, then what is left, upwards. */
void copyUp(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count >> 2; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
    const u32 done = count & ~u32(3);
    for (u32 i = done; i < count; ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

// The runtime's functions.
constexpr u32 kMemset = 0x4e0640, kStrncpy = 0x4e0e30, kToupper = 0x4eede0, kNmalloc = 0x4f7900, kNfree = 0x4f79f0;
constexpr u32 kSetErrno = 0x502880, kFatalError = 0x511d0c, kNrealloc = 0x5185c0, kMbslen = 0x518050;
constexpr u32 kMemmove = 0x518640, kCalloc = 0x525df0, kVsprintf = 0x4df674;

// EA's library.
constexpr u32 kError = 0x401010, kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr u32 kTextStart = 0x52e420, kBitmapToScreen = 0x4ead90, kRgbToColour = 0x4ef760;

// The runtime's data.
constexpr u32 kThreadDataGetter = 0x567764;  // the current thread's data
constexpr u32 kLockThreads = 0x567790, kUnlockThreads = 0x567794, kLockHandles = 0x567798, kUnlockHandles = 0x56779c;
constexpr u32 kThreadOnStart = 0x5677a4, kThreadOnEnd = 0x5677a8;
constexpr u32 kTlsIndex = 0x567760, kEnvironmentBlock = 0x567839;
constexpr u32 kHandleTable = 0x56acbc, kHandleCount = 0x56acc0;
constexpr u32 kEnviron = 0xa0b154, kEnvironFlags = 0xa0b150;
constexpr u32 kThreadDataSize = 0x56af70, kThreadList = 0xa18250;
constexpr u32 kSignalStopped = 0xa0ba94;

// The runtime's locks: a semaphore each, and the one that guards their creation.
constexpr u32 kLockStreams = 0xa0b320, kLockFileTable = 0xa0b330, kLockFiles = 0xa0b340, kLockHeap = 0xa0b440;
constexpr u32 kLockThreadList = 0xa0ba60, kLockHandleTable = 0xa0ba70, kLockOfLocks = 0xa0ba50;
constexpr u32 kStaticSemaphores = 0xa0b450, kStaticSemaphoreCount = 0xa0ba80;
constexpr u32 kExtraSemaphoreCount = 0xa0ba84, kExtraSemaphores = 0xa0ba88;

// The current window, as the library keeps it.
constexpr u32 kClipLeft = 0x565000, kClipTop = 0x565004, kClipRight = 0x565008, kClipBottom = 0x56500c;
constexpr u32 kWindowDepth = 0x565010, kWindowFlag = 0x565012, kWindowPixels = 0x565014;
constexpr u32 kRowOffsets = 0x565020, kColumnOffsets = 0x565024;
constexpr u32 kScreenWidth = 0x564ff8, kScreenHeight = 0x564ffc;

/* A semaphore of the runtime's: its lock taken (sub_51e71c's body); what its code leaves in eax, the
 * last answer of Windows'. */
u32 lockSemaphore(Guest& guest, nfs::ThreadState& ts, u32 semaphore)
{
    const u32 thread = os::threads::currentId(guest.app(), ts);
    u32 eax = thread;
    if (thread != guest.dword(semaphore + 8))
    {
        if (guest.dword(semaphore + 4) == 0)
        {
            guest::direct<0x51e71c, frontend::watc::lock>(guest.app(), ts, kLockOfLocks);
            if (guest.dword(semaphore + 4) == 0)
            {
                const u32 made = guest::direct<0x51e544, frontend::watc::newSemaphore>(guest.app(), ts);
                guest.setDword(semaphore + 4, 1);
                guest.setDword(semaphore, made);
            }
            guest::direct<0x51e780, frontend::watc::unlock>(guest.app(), ts, kLockOfLocks);
        }
        eax = fe::enterLock(guest, ts, guest.dword(semaphore));
        guest.setDword(semaphore + 8, thread);
    }
    guest.setDword(semaphore + 0xc, guest.dword(semaphore + 0xc) + 1);
    return eax;
}

/* And given back (sub_51e780's body). */
u32 unlockSemaphore(Guest& guest, nfs::ThreadState& ts, u32 semaphore)
{
    const u32 holds = guest.dword(semaphore + 0xc);
    if (holds != 0)
    {
        guest.setDword(semaphore + 0xc, holds - 1);
        if (holds - 1 == 0)
        {
            const u32 section = guest.dword(semaphore);
            guest.setDword(semaphore + 8, 0);
            return fe::leaveLock(guest, ts, section);
        }
    }
    return semaphore;
}

/* sub_51e940's body: the thread data's TLS index given back; what its code leaves in eax, TlsFree's
 * answer or `eax` as it came. */
u32 giveBackTlsIndex(Guest& guest, nfs::ThreadState& ts, u32 eax)
{
    const u32 index = guest.dword(kTlsIndex);
    if (index != 0xffffffff)
    {
        os::threads::giveBackSlot(index);
        eax = 0;  // what the code is told of it
        guest.setDword(kTlsIndex, 0xffffffff);
    }
    return eax;
}

}

// ---------------------------------------------------------------------------
// The handle table and the locks
// ---------------------------------------------------------------------------

/* sub_51e110: what a printf that needs floats finds when they are not linked in. */
u32 frontend::watc::fpNotLoaded(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x551368u, 1u);
}

nfs::u32 frontend::watcFpNotLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e110", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::fpNotLoaded(app, ts); });
}

/* sub_51e2a0: setmbcp(0). */
u32 frontend::watc::setMbcpZero(App* app, ThreadState& ts)
{
    return jumpTo<0x525ba0, setMbcp>(app, ts, 0u);
}

nfs::u32 frontend::watcSetMbcpZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::setMbcpZero(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_51e388: an OS handle in the table of POSIX handles, the table grown to take it.  For 0, 1
 * and 2 the code sets the system's standard handle too: the port keeps none of its own to set. */
u32 frontend::watc::setOsHandle(App* app, ThreadState& ts, u32 osHandle, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (s(handle) < 0)
        return osHandle;
    fe::pointerCall(guest, guest.dword(kLockHandles), args({ osHandle, handle }));  // its registers as the code has them
    const u32 size = guest.dword(kHandleCount);
    u32 table = guest.dword(kHandleTable);
    const u32 bytes = handle * 4;
    u32 last = osHandle;
    if (s(handle) < s(size))
    {
        guest.setDword(table + bytes, osHandle);
    }
    else
    {
        table = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.ts(), table, bytes + 4);
        const u32 count = guest.dword(kHandleCount);
        if (s(count) < s(handle))
        {
            u32 at = count * 4;
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
    }
    return fe::pointerCall(guest, guest.dword(kUnlockHandles), args({ last }));
}

nfs::u32 frontend::watcSetOsHandle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e388", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const u32 osHandle = args.eax, handle = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setOsHandle(app, ts, osHandle, handle); });
}

/* sub_51e51c: the table of handles freed; what its code leaves in eax, _nfree's or `eax` as it came. */
u32 frontend::watc::freeHandleTable(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 table = guest.dword(kHandleTable);
    if (table != 0)
    {
        eax = fe::direct<kNfree, watcom::clib::free>(app, ts, table);
        guest.setDword(kHandleTable, 0);
    }
    return eax;
}

nfs::u32 frontend::watcFreeHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e51c", 0, 0, kEbx | kEdx);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::freeHandleTable(app, ts, eax); });
}

/* sub_51e540, sub_51f270: hooks that do nothing -- eax as it was, which their thunks leave; 0 back
 * to a caller as C++. */
u32 frontend::watc::nothing51e540(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
    return 0;
}

nfs::u32 frontend::watcNothing51e540(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watc::nothing51e540(app, ts);
    ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::nothing51f270(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
    return 0;
}

nfs::u32 frontend::watcNothing51f270(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watc::nothing51f270(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* sub_51e544: a critical section for a lock: one of the 64 kept, then ones allocated. */
u32 frontend::watc::newSemaphore(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 used = guest.dword(kStaticSemaphoreCount);
    u32 section;
    if (s(used) < 0x40)
    {
        section = kStaticSemaphores + used * 0x18;
        guest.setDword(kStaticSemaphoreCount, used + 1);
    }
    else
    {
        section = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.ts(), 1, 0x18);
        if (section == 0)
            fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x55138cu, 1u);
        const u32 list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.ts(), guest.dword(kExtraSemaphores), (guest.dword(kExtraSemaphoreCount) + 1) << 2);
        guest.setDword(kExtraSemaphores, list);
        if (list == 0)
            fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x5513b0u, 1u);
        const u32 count = guest.dword(kExtraSemaphoreCount);
        guest.setDword(guest.dword(kExtraSemaphores) + count * 4, section);
        guest.setDword(kExtraSemaphoreCount, count + 1);
    }
    os::locks::make(guest.app(), section);
    return section;
}

nfs::u32 frontend::watcNewSemaphore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e544", 0, 0, kEbx | kEcx | kEdx | kEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::newSemaphore(app, ts); });
}

/* sub_51e5e8: the kept critical sections deleted; what its code leaves in eax, the last answer of
 * Windows' or `eax` as it came. */
u32 frontend::watc::deleteKeptSemaphores(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (s(guest.dword(kStaticSemaphoreCount)) > 0)
    {
        u32 i = 0, section = kStaticSemaphores;
        do
        {
            ++i;
            os::locks::remove(guest.app(), section);
            eax = 0;  // what the code is told of it
            section += 0x18;
        } while (s(i) < s(guest.dword(kStaticSemaphoreCount)));
    }
    return eax;
}

nfs::u32 frontend::watcDeleteKeptSemaphores(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e5e8", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::deleteKeptSemaphores(app, ts, eax); });
}

/* sub_51e618: the allocated ones deleted and freed, and their list; what its code leaves in eax,
 * the last call's or `eax` as it came. */
u32 frontend::watc::deleteExtraSemaphores(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (s(guest.dword(kExtraSemaphoreCount)) > 0)
    {
        u32 i = 0;
        do
        {
            const u32 offset = i * 4;
            ++i;
            os::locks::remove(guest.app(), guest.dword(guest.dword(kExtraSemaphores) + offset));
            eax = fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), guest.dword(guest.dword(kExtraSemaphores) + offset));
        } while (s(i) < s(guest.dword(kExtraSemaphoreCount)));
    }
    const u32 list = guest.dword(kExtraSemaphores);
    if (list != 0)
        eax = fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), list);
    return eax;
}

nfs::u32 frontend::watcDeleteExtraSemaphores(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e618", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::deleteExtraSemaphores(app, ts, eax); });
}

/* sub_51e670: a lock's owner and count cleared. */
u32 frontend::watc::clearLock(App* app, ThreadState& ts, u32 lock)
{
    Guest guest(app, ts);
    guest.setDword(lock + 8, 0);
    guest.setDword(lock + 0xc, 0);
    return lock;
}

nfs::u32 frontend::watcClearLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e670", 0, 0, 0);
    const u32 lock = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::clearLock(app, ts, lock); });
}

/* sub_51e680 to sub_51e70c: the runtime's hooks for its locks, each a lock named and sub_51e71c
 * or sub_51e780 on it. */
u32 frontend::watc::lockStreams(App* app, ThreadState& ts)
{
    return jumpTo<0x51e71c, lock>(app, ts, kLockStreams);
}

nfs::u32 frontend::watcLockStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::lockStreams(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::unlockStreams(App* app, ThreadState& ts)
{
    return jumpTo<0x51e780, unlock>(app, ts, kLockStreams);
}

nfs::u32 frontend::watcUnlockStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::unlockStreams(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::lockFile(App* app, ThreadState& ts, u32 file)
{
    return jumpTo<0x51e71c, lock>(app, ts, ((file & 0xf) << 4) + kLockFiles);
}

nfs::u32 frontend::watcLockFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::lockFile(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::unlockFile(App* app, ThreadState& ts, u32 file)
{
    return jumpTo<0x51e780, unlock>(app, ts, ((file & 0xf) << 4) + kLockFiles);
}

nfs::u32 frontend::watcUnlockFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::unlockFile(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_51e6b8: allocPOSIXHandle. */
u32 frontend::watc::allocHandleHook(App* app, ThreadState& ts, u32 osHandle)
{
    return jumpTo<0x51e304, watcom::clib::allocPOSIXHandle>(app, ts, osHandle);
}

nfs::u32 frontend::watcAllocHandleHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::allocHandleHook(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_51e6c0: a file's lock cleared and its POSIX handle freed. */
u32 frontend::watc::freeHandleHook(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest::direct<0x51e670, clearLock>(app, ts, ((handle & 0xf) << 4) + kLockFiles);
    return fe::direct<0x51e45c, watcom::clib::freePOSIXHandle>(app, ts, handle);
}

nfs::u32 frontend::watcFreeHandleHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e6c0", 0, 0, kEdx);
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::freeHandleHook(app, ts, handle); });
}

u32 frontend::watc::lockHeap(App* app, ThreadState& ts)
{
    return jumpTo<0x51e71c, lock>(app, ts, kLockHeap);
}

nfs::u32 frontend::watcLockHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::lockHeap(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::unlockHeap(App* app, ThreadState& ts)
{
    return jumpTo<0x51e780, unlock>(app, ts, kLockHeap);
}

nfs::u32 frontend::watcUnlockHeap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::unlockHeap(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::lockFileTable(App* app, ThreadState& ts)
{
    return jumpTo<0x51e71c, lock>(app, ts, kLockFileTable);
}

nfs::u32 frontend::watcLockFileTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::lockFileTable(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::unlockFileTable(App* app, ThreadState& ts)
{
    return jumpTo<0x51e780, unlock>(app, ts, kLockFileTable);
}

nfs::u32 frontend::watcUnlockFileTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::unlockFileTable(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::lockThreadList(App* app, ThreadState& ts)
{
    return jumpTo<0x51e71c, lock>(app, ts, kLockThreadList);
}

nfs::u32 frontend::watcLockThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::lockThreadList(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::unlockThreadList(App* app, ThreadState& ts)
{
    return jumpTo<0x51e780, unlock>(app, ts, kLockThreadList);
}

nfs::u32 frontend::watcUnlockThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::unlockThreadList(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_51e714: the handle table's lock. */
u32 frontend::watc::lockHandleTable(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return lockSemaphore(guest, ts, kLockHandleTable);
}

nfs::u32 frontend::watcLockHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e714", 0, 0, kEbx | kEcx | kEdx | kEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lockHandleTable(app, ts); });
}

/* sub_51e71c: a lock taken: its critical section made the first time, entered unless this thread
 * has it already, counted. */
u32 frontend::watc::lock(App* app, ThreadState& ts, u32 lock)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    return lockSemaphore(guest, ts, lock);
}

nfs::u32 frontend::watcLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e71c", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 lock = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lock(app, ts, lock); });
}

/* sub_51e778: the handle table's lock given back. */
u32 frontend::watc::unlockHandleTable(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return unlockSemaphore(guest, ts, kLockHandleTable);
}

nfs::u32 frontend::watcUnlockHandleTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e778", 0, 0, kEbx | kEcx | kEdx | kEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::unlockHandleTable(app, ts); });
}

/* sub_51e780: a lock given back, its critical section left when the count is out. */
u32 frontend::watc::unlock(App* app, ThreadState& ts, u32 lock)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return unlockSemaphore(guest, ts, lock);
}

nfs::u32 frontend::watcUnlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e780", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 lock = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::unlock(app, ts, lock); });
}

// ---------------------------------------------------------------------------
// Threads' data
// ---------------------------------------------------------------------------

/* sub_51e7a8: the current thread's data, made if it has none and regrown if it asks, the system's
 * last error kept (the port keeps none). */
u32 frontend::watc::threadData(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    u32 data = os::threads::slotValue(guest.dword(kTlsIndex));
    if (data == 0)
        data = fe::direct<0x525e10, attachThisThread>(app, ts);
    else if (guest.byte(data + 0x53) != 0)
        data = fe::direct<0x525e4c, resizeThreadData>(app, ts);
    return data;
}

nfs::u32 frontend::watcThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e7a8", 0, 0, kEbx | kEcx | kEdx | kEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::threadData(app, ts); });
}

/* sub_51e828: the TLS index taken, past 0, 1 and 2 on Windows' older versions. */
u32 frontend::watc::allocTlsIndex(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    constexpr u32 kWinVersion = 0x567841, kWinMajor = 0x56783f;
    u32 index = guest.dword(kTlsIndex);
    if (index == 0xffffffff)
    {
        index = os::threads::takeSlot();
        if (guest.word(kWinVersion) >= 0x8000 && guest.byte(kWinMajor) < 4)
        {
            while (index != 0xffffffff && index <= 2)
            {
                guest.setDword(kTlsIndex, index);
                index = os::threads::takeSlot();
            }
        }
    }
    guest.setDword(kTlsIndex, index);
    return index != 0xffffffff ? 1 : 0;
}

nfs::u32 frontend::watcAllocTlsIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e828", 0, 0, kEbx | kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::allocTlsIndex(app, ts); });
}

/* sub_51e888: a thread's data made -- from the block `data` (eax, which it hands on to sub_51e7ec),
 * 0 for one of calloc's -- put on the list and in its TLS slot. */
u32 frontend::watc::attachThread(App* app, ThreadState& ts, u32 block)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (guest.dword(kTlsIndex) == 0xffffffff)
        return 0;
    const u32 data = fe::direct<0x51e7ec, watcom::clib::initThreadData>(app, ts, block);
    if (data == 0)
        return 0;
    if (fe::direct<0x525f20, addThread>(app, ts, guest.dword(data + 0xda), data) == 0)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), data);
        return 0;
    }
    os::threads::setSlotValue(guest.dword(kTlsIndex), data);
    return 1;
}

nfs::u32 frontend::watcAttachThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e888", 0, 0, kEbx | kEcx | kEdx);
    const u32 block = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::attachThread(app, ts, block); });
}

/* sub_51e8dc: this thread's data off the list and out of its slot; with eax its handle closed. */
u32 frontend::watc::detachThread(App* app, ThreadState& ts, u32 closeIt)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    const u32 index = guest.dword(kTlsIndex);
    if (index == 0xffffffff)
        return closeIt;  // eax as it came
    const u32 data = os::threads::slotValue(index);
    if (data == 0)
        return 0;
    const u32 thread = guest.dword(data + 0xde);
    guest::direct<0x525f84, removeThread>(app, ts, guest.dword(data + 0xda));
    u32 eax = os::threads::setSlotValue(guest.dword(kTlsIndex), 0) ? 1 : 0;
    if (thread != 0 && closeIt != 0)
    {
        os::release(app, thread);
        eax = 1;  // a handle let go: 1, as the code is told
    }
    return eax;
}

nfs::u32 frontend::watcDetachThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e8dc", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const u32 closeIt = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::detachThread(app, ts, closeIt); });
}

/* sub_51e934: the last thread's end: its data, then the TLS index. */
u32 frontend::watc::endThreads(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0);
    // 1 in eax as the game's code hands it, which it gives back with no index; what it gives back is
    // what the end hands back if there is no index to give.
    const u32 detached = fe::direct<0x51e8dc, detachThread>(app, ts, 1);
    ts.sp -= 8;
    return giveBackTlsIndex(guest, ts, detached);
}

nfs::u32 frontend::watcEndThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e934", 0, 0, kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::endThreads(app, ts); });
}

/* sub_51e940: the TLS index given back; `eax` as it came, which it leaves for none. */
u32 frontend::watc::freeTlsIndex(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return giveBackTlsIndex(guest, ts, eax);
}

nfs::u32 frontend::watcFreeTlsIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e940", 0, 0, kEcx | kEdx);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::freeTlsIndex(app, ts, eax); });
}

/* sub_51e964: the runtime made thread-safe, on the first _beginthread: the hooks pointed at the
 * real locks, and the first thread's data on the list. */
u32 frontend::watc::initThreads(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr u32 kMainThreadData = 0x9f642c;
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
    const u32 section = fe::direct<0x51e544, newSemaphore>(app, ts);
    guest.setDword(kLockOfLocks, section);
    guest.setDword(kLockOfLocks + 4, 1);
    guest.setDword(kLockHandles, 0x51e714);
    guest.setDword(kUnlockHandles, 0x51e778);
    const u32 data = guest.dword(kMainThreadData);
    guest.setDword(0x5677a0, 0x51e934);  // kept in edi across the call, as the code keeps it
    guest::direct<0x525f20, addThread>(app, ts, guest.dword(data + 0xda), data);
    const u32 eax =
        os::threads::setSlotValue(guest.dword(kTlsIndex), guest.dword(kMainThreadData)) ? 1 : 0;
    guest.setDword(kThreadDataGetter, 0x51e7a8);
    return eax;
}

nfs::u32 frontend::watcInitThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51e964", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::initThreads(app, ts); });
}

/* sub_51ea7c: the runtime's locks taken down at its end. */
u32 frontend::watc::finiThreads(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const auto clear = [&](u32 lock) { return fe::pointerCall(guest, guest.dword(0x56acd0), args({ lock })); };
    clear(kLockStreams);
    for (u32 lock = kLockFiles; lock != kLockFiles + 0x100; lock += 0x10)
        clear(lock);
    // What each gives back in eax goes on to the next, which gives it back when it has nothing to do.
    u32 eax = clear(kLockHandleTable);
    eax = fe::direct<0x51e618, deleteExtraSemaphores>(app, ts, eax);
    fe::direct<0x526060, freeThreadListJump>(app, ts, eax);
    fe::direct<0x5241c0, watcom::clib::heapShrink>(app, ts);
    clear(kLockHeap);
    clear(kLockFileTable);
    clear(kLockThreadList);
    eax = clear(kLockOfLocks);
    eax = fe::direct<0x51e5e8, deleteKeptSemaphores>(app, ts, eax);
    return fe::direct<0x51e940, freeTlsIndex>(app, ts, eax);
}

nfs::u32 frontend::watcFiniThreads(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51ea7c", 0, 0, kEbx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::finiThreads(app, ts); });
}

// ---------------------------------------------------------------------------
// Exceptions
// ---------------------------------------------------------------------------

/* sub_51ec90: whether the program has an active window, asked of USER32 if the port has it. */
u32 frontend::watc::hasActiveWindow(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    u32 window = 0;
    const u32 user32 = os::modules::load(app, guest::text(app, 0x5513d4)).value_or(0);
    if (user32 != 0)
    {
        const u32 getActiveWindow = os::modules::symbol(app, user32, guest::text(app, 0x5513e0));
        if (getActiveWindow != 0)
            window = fe::pointerCall(guest, getActiveWindow);
    }
    return window != 0 ? 1 : 0;
}

nfs::u32 frontend::watcHasActiveWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51ec90", 0, 0, kEbx | kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::hasActiveWindow(app, ts); });
}

/* sub_51eccc: a line added to the report, the hex digits after its 0x the value's. */
u32 frontend::watc::reportAppend(App* app, ThreadState& ts, u32 report, u32 line, u32 value)
{
    Guest guest(app, ts);
    constexpr u32 kHexDigits = 0x56acdc;
    u32 to = report, from = line;
    if (guest.byte(to) != 0)
    {
        do
            ++to;
        while (guest.byte(to) != 0);
    }
    u32 lastDigit = 0;
    u32 digitAt = to + 9;
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
    if (lastDigit == 0)
        return to;
    u32 rest = value;
    while (rest != 0)
    {
        const std::uint8_t digit = guest.byte((rest & 0xf) + kHexDigits);
        --lastDigit;
        guest.setByte(lastDigit + 1, digit);
        rest >>= 4;
    }
    return 0;
}

nfs::u32 frontend::watcReportAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51eccc", 0, 0, kEcx | kEsi | kEdi);
    const u32 value = args.ebx;
    const u32 report = args.eax;
    const u32 line = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::reportAppend(app, ts, report, line, value); });
}

/* sub_51ed24: the filter that writes the report on stderr, when there is a console to read it. */
u32 frontend::watc::reportException(App* app, ThreadState& ts, u32 pointers)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x10c);
    const u32 buffer = guest.frame();
    const u32 record = guest.dword(pointers), context = guest.dword(pointers + 4);
    if (fe::direct<0x51ec90, hasActiveWindow>(app, ts) != 0
        || fe::direct<0x515734, watcom::clib::consoleOutput>(app, ts) == 0xffffffff)
        return 0;
    guest.setByte(buffer, 0);
    const auto add = [&](u32 line, u32 value) { fe::direct<0x51eccc, reportAppend>(app, ts, buffer, line, value); };
    const u32 where = guest.dword(record + 0xc);
    const u32 code = guest.dword(record);
    u32 line = 0;
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
    const u32 length = guest.length(buffer);
    const u32 handles = guest.dword(kHandleTable);
    u32 written;
    os::files::write(app, ts, guest.dword(handles + 8), guest::memory(app, buffer), length, written);
    return 1;
}

nfs::u32 frontend::watcReportException(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51ed24", 4, 0, kEbx | kEdi);
    const u32 pointers = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::reportException(app, ts, pointers); });
}

/* sub_51ef08: the runtime's exception handler: the x87's exceptions and the rest raised as
 * signals, or handed to what signal hooked, and otherwise the system's filter and the exit --
 * the port has no filter, so no exit: it goes on. */
u32 frontend::watc::exceptionHandler(App* app, ThreadState& ts, u32 record, u32 context)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    constexpr u32 kSignalAsked = 0x56acd4, kSignalRaise = 0x56acd8;
    const u32 pointers = guest.frame();
    const auto notHandled = [&]() -> u32 {
        guest.setDword(pointers, record);
        guest.setDword(pointers + 4, context);
        os::debugBreak();
        return 1;
    };
    if ((guest.byte(record + 4) & 6) != 0)
        return 1;
    const u32 kind = guest.dword(record) + 0x3fffff73;
    u32 signal = 0;
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
            const u32 instruction = guest.dword(context + 0x28);
            const std::uint16_t opcode = guest.word(instruction);
            u32 edx = opcode;  // dx: the code's working of it (the rest of edx is none of its)
            signal = 0x81;
            if (opcode == 0xfad9)
                signal = 0x88;
            else if (opcode == 0xf1d9)
                signal = 0x8e;
            else
            {
                const std::uint8_t first = guest.byte(instruction);
                edx = (edx & 0xffff00ff) | (u32(first) << 8);
                if (first == 0xdb || first == 0xdf)
                {
                    const std::uint8_t form = guest.byte(instruction + 1) & 0x30;
                    edx = (edx & 0xffffff00) | form;
                    if (form == 0x10)
                        signal = 0x8d;
                }
                if ((guest.byte(instruction) & 1) == 0 && (guest.byte(instruction + 1) & 0x30) == 0x30)
                {
                    const u32 top = ((guest.dword(context + 0x20) << 2) & 0xffff) >> 13;
                    const u32 tags = guest.dword(context + 0x24) & 0xffff;
                    edx = (tags >> (top * 2)) & 1;
                    if (edx == 1)
                        signal = 0x83;
                }
            }
            break;
        }
        }
    }
    if (signal != 0)
    {
        guest.setByte(kSignalStopped, 1);
        fe::direct<0x526070, clearFpExceptions>(app, ts);
        if (fe::direct<0x526244, raiseFp>(app, ts, signal) == 0xffffffff || guest.byte(kSignalStopped) == 0)
            return notHandled();
        const u32 flags = guest.word(context + 0x20);
        guest.setWord(context + 0x20, std::uint16_t(flags & 0x7f00));
        return 0;
    }
    if (guest.dword(kSignalRaise) == 0)
        return notHandled();
    for (u32 number = 1; s(number) <= 0xc; ++number)
    {
        const u32 asked = fe::pointerCall(guest, guest.dword(kSignalAsked), args({ number, guest.dword(record) }));
        if (asked == 0)
            continue;
        if (asked == 1 || asked == 2 || asked == 3)
            break;
        guest.setByte(kSignalStopped, 1);  // cl, 1 across the calls, which keep it
        fe::pointerCall(guest, guest.dword(kSignalRaise), args({ number }));
        if (guest.byte(kSignalStopped) != 0)
            return 0;
    }
    return notHandled();
}

nfs::u32 frontend::watcExceptionHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51ef08", 0, 0, kEbx | kEsi | kEdi);
    const u32 record = app->getMemory<u32>(ts.sp + 4);
    const u32 context = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::exceptionHandler(app, ts, record, context); });
}

/* sub_51f140: the float formatter's hook for printf: the double taken off the arguments, its
 * digits from __LDcvt, and the sign. */
u32 frontend::watc::formatFloat(App* app, ThreadState& ts, u32 out, u32 arguments, u32 spec)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x50);
    const u32 ebp = guest.frame() + 0x4c;
    const auto local = [&](u32 below) { return ebp - below; };
    const u32 character = guest.byte(spec + 0x15);
    guest.setDword(local(0x40), character);
    const u32 upper = guest.byte(spec + 0x15) & 0x5f;
    u32 precision = guest.dword(spec + 8);
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
    const u32 next = guest.dword(arguments) + 8;
    guest.setDword(arguments, next);
    guest.setDword(local(0x14), guest.dword(next - 8));
    guest.setDword(local(0x10), guest.dword(next - 4));
    app->getMemory<nfs::Float80>(local(0x20)) = nfs::Float(app->getMemory<double>(local(0x14)));
    if (precision == 0xffffffff)
        precision = 6;
    guest.setDword(local(0x4c), precision);
    guest.setDword(local(0x3c), 0);
    fe::direct<0x5167b3, watb::longDoubleToDigits>(app, ts, local(0x20), local(0x4c), out + 1);
    guest.setDword(spec + 0x28, guest.dword(local(0x30)));
    guest.setDword(spec + 0x2c, guest.dword(local(0x2c)));
    guest.setDword(spec + 0x30, guest.dword(local(0x28)));
    guest.setDword(spec + 0x34, guest.dword(local(0x24)));
    const auto sign = [&](std::uint8_t c) {
        const u32 at = guest.dword(spec + 0x20);
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
    return out;
}

nfs::u32 frontend::watcFormatFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51f140", 0, 0, kEcx | kEsi | kEdi);
    const u32 out = args.eax, arguments = args.edx, spec = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::formatFloat(app, ts, out, arguments, spec); });
}

/* sub_51f254: scanf's hook: strtod, the double stored. */
u32 frontend::watc::scanFloat(App* app, ThreadState& ts, u32 text, u32 to)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    u32 result = 0;
    app->getMemory<double>(to) = fe::direct<0x4f82b6, watcom::clib::strtod>(app, ts, text, 0u, &result);
    return result;
}

nfs::u32 frontend::watcScanFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_51f254", 0, 0, kEbx);
    const u32 text = args.eax, to = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::scanFloat(app, ts, text, to); });
}

// ---------------------------------------------------------------------------
// Pixels
// ---------------------------------------------------------------------------

/* sub_5236c0: shaperowbytes: a shape's bytes a row, by its type's bits and alignment. */
u32 frontend::watc::shapeRowBytes(App* app, ThreadState& ts, u32 shape)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr u32 kBitsOfType = 0x566d54;
    const u32 type = guest.dword(shape) & 0xff;
    if (type == 0x7b)
        return sar(guest.dword(shape + 2), 16);
    if ((type & 0x80) != 0)
    {
        guest.setDword(kErrorFile, 0x551b0c);
        guest.setDword(kErrorFunction, 0x551b1c);
        guest.setDword(kErrorLine, 0x35);
        fe::direct<0x401010, ea::error>(app, ts, 0x551b2cu, guest::Words({ type }));
    }
    u32 bits = guest.byte((guest.dword(shape) & 0x7f) + kBitsOfType);
    if (bits == 0xf)
        bits = 0x10;
    const u32 rowBits = sar(guest.dword(shape + 2), 16) * bits;
    if ((guest.dword(shape) & 0xf0) == 0x40)
        return sar((rowBits + 0xf) & 0xfffffff0, 3);
    return sar((rowBits + 7) & 0xfffffff8, 3);
}

nfs::u32 frontend::watcShapeRowBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5236c0", 0, 0, kEcx | kEdx);
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::shapeRowBytes(app, ts, shape); });
}

namespace
{

/* A channel under a pixel scaled by a 16.16 inverse alpha, rounded on the bit shifted out. */
u32 fixedProduct(u32 value, u32 scale)
{
    const std::int64_t product = std::int64_t(s32(value)) * std::int64_t(s32(scale));
    const u32 low = u32(product);
    const u32 high = u32(std::uint64_t(product) >> 32) << 16;
    return (low >> 16) + high + ((low >> 15) & 1);
}

}

/* sub_523760: a pixel with its alpha: a 16-bit window blended in place at once, any other through
 * readpixel and writepixel. */
u32 frontend::watc::putPixel(App* app, ThreadState& ts, u32 x, u32 y, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 40);
    if (s(x) < s(guest.dword(kClipLeft)) || s(x) >= s(guest.dword(kClipRight)) || s(y) < s(guest.dword(kClipTop))
        || s(y) >= s(guest.dword(kClipBottom)))
        return x;
    if (guest.byte(kWindowDepth) == 0x10 && guest.byte(kWindowFlag) == 0)
    {
        const u32 rows = guest.dword(kRowOffsets);
        const u32 at = guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(rows + y * 4) + guest.dword(kWindowPixels);
        if (colour < 0x10000000)
            return at;
        u32 mixed = colour;
        if (colour < 0xfc000000)
        {
            // The pixel's fields spread out and scaled by the inverse alpha, added to the colour.
            const u32 pixel = guest.word(at);
            const u32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
            const std::uint64_t product = std::uint64_t(spread) * ((colour ^ 0xffffffff) >> 24);
            const u32 low = u32(product), high = u32(product >> 32) << 16;
            mixed = colour + high + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
        }
        const u32 packed = ((mixed & 0xf80000) >> 8) + ((mixed >> 3) & 0x1f) + ((mixed & 0xfc00) >> 5);
        guest.setWord(at, std::uint16_t(packed));
        return (mixed >> 3) & 0x1f;
    }
    const u32 alpha = colour >> 24;
    if (alpha == 0xff)
    {
        const u32 converted = fe::direct<0x4ef760, eab::colourPack>(app, ts, colour);
        return fe::direct<0x523b00, writePixel>(app, ts, x, y, converted);
    }
    if (alpha == 0)
        return x;
    const u32 red = (colour >> 16) & 0xff, green = (colour >> 8) & 0xff, blue = colour & 0xff;
    const u32 scale = 0x10000 - (alpha << 8);  // in ecx across the calls, which keep it
    const u32 under = fe::direct<0x50c3a0, watb::colourToArgb>(app, ts, fe::direct<0x503bc0, watb::readPixel>(app, ts, x, y));
    const u32 underAlpha = sar(under, 24) & 0xff, underRed = sar(under, 16) & 0xff;
    const u32 underGreen = sar(under, 8) & 0xff, underBlue = under & 0xff;
    const u32 outAlpha = 0xff - u32(s((0xff - alpha) * (0xff - underAlpha)) / 0xff);
    const auto channel = [&](u32 below, u32 own) {
        const u32 sum = fixedProduct(below, scale) + own;
        return s(sum) <= 0xff ? sum : u32(0xff);
    };
    const u32 outRed = channel(underRed, red);
    const u32 outGreen = channel(underGreen, green);
    const u32 outBlue = channel(underBlue, blue);
    const u32 argb = (outAlpha << 24) | (outRed << 16) | (outGreen << 8) | outBlue;
    const u32 converted = fe::direct<0x4ef760, eab::colourPack>(app, ts, argb);
    return fe::direct<0x523b00, writePixel>(app, ts, x, y, converted);
}

nfs::u32 frontend::watcPutPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_523760", 0, 0, kEcx | kEsi | kEdi);
    const u32 x = args.eax, y = args.edx, colour = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::putPixel(app, ts, x, y, colour); });
}

/* sub_5239ec: a 4-bit shape's rows through a blending table onto 8-bit pixels: a pixel's nibble is
 * the table's high byte, what is under it the low.  What its code leaves in eax: the last pixels
 * written, or `eax` as it came. */
u32 frontend::watc::blend4To8(App* app, ThreadState& ts, u32 arguments, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 base = arguments;
    const u32 kPairs = base + 12, kRows = base + 16, kFromSkip = base + 20, kToSkip = base + 24;
    const u32 table = guest.dword(base);
    if (s(guest.dword(kRows)) <= 0)
        return eax;
    u32 edx = 0;
    u32 from = guest.dword(base + 4), to = guest.dword(base + 8);
    const u32 width = guest.dword(kPairs);
    if (s(width) < 1)
        return eax;
    const bool odd = (width & 1) != 0;
    guest.setDword(kPairs, width >> 1);
    const auto pixel = [&](u32 source, u32 under) {
        const std::uint8_t p = guest.byte(source);
        const u32 low = (u32(p >> 4) << 8) | guest.byte(under);
        edx = (u32(p & 0xf) << 8) | guest.byte(under + 1);
        return u32(guest.byte(table + low)) | (u32(guest.byte(table + edx)) << 8);
    };
    if (odd)
    {
        guest.setDword(kFromSkip, guest.dword(kFromSkip) + 1);
        guest.setDword(kToSkip, guest.dword(kToSkip) + 2);
    }
    do
    {
        for (u32 count = guest.dword(kPairs); count != 0; --count)
        {
            const u32 second = pixel(from + 1, to + 2);
            const u32 first = pixel(from, to);
            eax = (second << 16) | first;
            guest.setDword(to, eax);
            from += 2;
            to += 4;
        }
        if (odd)
        {
            const u32 last = pixel(from, to);
            eax = (eax & 0xffff0000) | last;
            guest.setWord(to, std::uint16_t(last));
        }
        from += guest.dword(kFromSkip);
        to += guest.dword(kToSkip);
        guest.setDword(kRows, guest.dword(kRows) - 1);
    } while (guest.dword(kRows) != 0);
    (void)edx;
    return eax;
}

nfs::u32 frontend::watcBlend4To8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5239ec", 0, 0, kEbx | kEsi | kEdi);
    const u32 arguments = ts.sp + 4;
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::blend4To8(app, ts, arguments, eax); });
}

/* sub_523b00: writepixel: a colour already in the window's format, at its depth. */
u32 frontend::watc::writePixel(App* app, ThreadState& ts, u32 x, u32 y, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    if (s(x) < s(guest.dword(kClipLeft)) || s(x) >= s(guest.dword(kClipRight)) || s(y) < s(guest.dword(kClipTop))
        || s(y) >= s(guest.dword(kClipBottom)))
        return x;
    const u32 at = guest.dword(guest.dword(kRowOffsets) + y * 4) + guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(kWindowPixels);
    const std::uint8_t depth = guest.byte(kWindowDepth);
    const u32 eax = (guest.dword(kWindowPixels) & 0xffffff00) | depth;
    switch (depth)
    {
    case 0xf:
    case 0x10:
        guest.setWord(at, std::uint16_t(colour));
        return eax;
    case 0x18:
        guest.setWord(at, std::uint16_t(colour));
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
    u32 left = eax;
    if (depth != 4)
    {
        guest.setDword(kErrorLine, 0x53);
        guest.setDword(kErrorFile, 0x551b68);
        guest.setDword(kErrorFunction, 0x551b78);
        // Error's eax read after it (where it comes back).
        left = *fe::direct<kError, ea::errorLeaves>(app, ts, depth, 0x551b84u, guest::Words({ depth }));
    }
    // Four bits a pixel, or what a depth that is none falls through to.
    const u32 where = at;
    if ((x & 1) != 0)
    {
        const std::uint8_t high = std::uint8_t(colour << 4);
        const std::uint8_t merged = (guest.byte(where) & 0xf) | high;
        guest.setByte(where, merged);
        return (left & 0xffffff00) | high;
    }
    const std::uint8_t merged = (guest.byte(where) & 0xf0) | std::uint8_t(colour);
    guest.setByte(where, merged);
    return (left & 0xffffff00) | merged;
}

nfs::u32 frontend::watcWritePixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_523b00", 0, 0, kEcx | kEsi);
    const u32 x = args.eax, y = args.edx;
    const u32 colour = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::writePixel(app, ts, x, y, colour); });
}

// ---------------------------------------------------------------------------
// The environment
// ---------------------------------------------------------------------------

/* sub_524f88: environ made from Windows' block, the strings copied, with a byte a string that says
 * whose memory each is in; then the handles' modes from C_FILE_INFO.  `eax` as it came, which it
 * leaves when environ is made already. */
u32 frontend::watc::initEnviron(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr u32 kEnvironCopy = 0xa18230;
    if (guest.dword(kEnviron) != 0)
        return eax;
    const u32 block = guest.dword(kEnvironmentBlock);
    u32 strings = 0;
    u32 end = block;
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
    u32 bytes = end - block;
    if (bytes == 0)
        bytes = 1;
    const u32 copy = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), bytes);
    if (copy != 0)
    {
        guest.setDword(kEnvironCopy, copy);
        const u32 list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), strings * 5 + 4);
        if (list == 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), copy);
        }
        else
        {
            u32 count = 0, offset = 0, from = block, to = copy;
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
            fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), list + offset, 0, count);
        }
    }
    return guest::direct<0x52bdb0, inheritHandles>(app, ts);
}

nfs::u32 frontend::watcInitEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_524f88", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::initEnviron(app, ts, eax); });
}

/* sub_52506c: environ freed, and Windows' block. */
u32 frontend::watc::freeEnviron(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    constexpr u32 kEnvironCopy = 0xa18230;
    // What each call gives back in eax, the last of them what this one leaves there.
    u32 eax = guest::direct<0x52bed0, clearEnviron>(app, ts);
    const u32 list = guest.dword(kEnviron);
    if (list != 0)
    {
        eax = fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), list);
        guest.setDword(kEnviron, 0);
    }
    const u32 copy = guest.dword(kEnvironCopy);
    if (copy != 0)
    {
        eax = fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), copy);
        guest.setDword(kEnvironCopy, 0);
    }
    // The environment's block is the port's, kept: 1, as the code is told it is let go.
    if (guest.dword(kEnvironmentBlock) != 0)
        eax = 1;
    return eax;
}

nfs::u32 frontend::watcFreeEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52506c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::freeEnviron(app, ts); });
}

/* sub_525ba0: setmbcp: the table of lead bytes for a code page.  The port's text has one code
 * page, a byte a character (os::text::kCodePage): no lead bytes in any page asked for, and no ANSI
 * page of its own (0, the OEM one taken for it). */
u32 frontend::watc::setMbcp(App* app, ThreadState& ts, u32 asked)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 40);
    constexpr u32 kLeadBytes = 0xa0b210, kIsMultibyte = 0xa0b200, kCodePage = 0x56b0b8;
    const auto clearTable = [&] { fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kLeadBytes, 0, 0x101); };
    u32 page = asked;
    if (asked == 0xffffffff)
        page = 0;
    else if (asked == 0xfffffffe)
        page = os::text::kCodePage;
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
        for (u32 c = 0x82; c <= 0xa0; ++c)
            guest.setByte(kLeadBytes + c, 1);
        for (u32 c = 0xe1; c <= 0xfd; ++c)
            guest.setByte(kLeadBytes + c, 1);
        guest.setDword(kIsMultibyte, 1);
        guest.setDword(kCodePage, 0x3a4);
        return 0;
    }
    if (page == 0)
        page = 1;
    clearTable();
    guest.setDword(kIsMultibyte, 0);
    if (page == 1)
    {
        guest.setDword(kCodePage, os::text::kCodePage);
        return 0;
    }
    guest.setDword(kCodePage, page);
    return 0;
}

nfs::u32 frontend::watcSetMbcp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525ba0", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 asked = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setMbcp(app, ts, asked); });
}

// ---------------------------------------------------------------------------
// Threads' list
// ---------------------------------------------------------------------------

/* sub_525e10: a thread that has no data yet: made now. */
u32 frontend::watc::attachThisThread(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    u32 data = 0;
    // 0 on through it to sub_51e7ec: no block of its own.
    if (fe::direct<0x51e888, attachThread>(app, ts, 0u) != 0)
        data = os::threads::slotValue(guest.dword(kTlsIndex));
    if (data == 0)
        fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x551ef0u, 1u);
    return data;
}

nfs::u32 frontend::watcAttachThisThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525e10", 0, 0, kEbx | kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::attachThisThread(app, ts); });
}

/* sub_525e4c: the thread's data to the size asked now, moved from the block it did not own. */
u32 frontend::watc::resizeThreadData(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    fe::pointerCall(guest, guest.dword(kLockThreads));
    const u32 thread = os::threads::currentId(guest.app(), ts);
    u32 node = guest.dword(kThreadList);
    while (node != 0 && thread != guest.dword(node + 4))
        node = guest.dword(node);
    u32 data;
    if (guest.dword(node + 0xc) != 0)
    {
        data = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.ts(), guest.dword(node + 8), guest.dword(kThreadDataSize));
        if (data == 0)
            fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x551f18u, 1u);
    }
    else
    {
        data = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.ts(), 1, guest.dword(kThreadDataSize));
        if (data == 0)
            fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x551f40u, 1u);
        const u32 old = guest.dword(node + 8);
        copyUp(guest, data, old, guest.dword(old + 0xf0));
        guest.setDword(node + 0xc, 1);
    }
    guest.setDword(node + 8, data);
    const u32 size = guest.dword(kThreadDataSize);
    guest.setByte(data + 0x52, 1);
    guest.setByte(data + 0x53, 0);
    guest.setDword(data + 0xf0, size);
    os::threads::setSlotValue(guest.dword(kTlsIndex), data);
    fe::pointerCall(guest, guest.dword(kUnlockThreads));
    return data;
}

nfs::u32 frontend::watcResizeThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525e4c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::resizeThreadData(app, ts); });
}

/* sub_525f20: a thread's data on the list: 1, or 0 if there is no memory or the hook says no. */
u32 frontend::watc::addThread(App* app, ThreadState& ts, u32 thread, u32 data)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    fe::pointerCall(guest, guest.dword(kLockThreads));
    u32 added = 0;
    const u32 node = fe::direct<kCalloc, watcom::clib::calloc>(guest.app(), guest.ts(), 1, 0x10);
    if (node != 0)
    {
        // The data on through it to the hook, in eax.
        if (fe::direct<0x52c824, threadStartHook>(app, ts, data) != 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), node);
        }
        else
        {
            guest.setDword(node + 8, data);
            guest.setDword(node + 4, thread);
            guest.setDword(node + 0xc, guest.byte(data + 0x52));
            const u32 first = guest.dword(kThreadList);
            guest.setDword(kThreadList, node);
            guest.setDword(node, first);
            added = 1;
        }
    }
    fe::pointerCall(guest, guest.dword(kUnlockThreads));
    return added;
}

nfs::u32 frontend::watcAddThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525f20", 0, 0, kEbx | kEcx | kEsi);
    const u32 thread = args.eax, data = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::addThread(app, ts, thread, data); });
}

/* sub_525f84: a thread off the list, its data freed if the list owns it. */
u32 frontend::watc::removeThread(App* app, ThreadState& ts, u32 thread)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    fe::pointerCall(guest, guest.dword(kLockThreads));
    u32 link = kThreadList;
    for (u32 node = guest.dword(kThreadList); node != 0; link = node, node = guest.dword(node))
    {
        if (thread == guest.dword(node + 4))
        {
            if (guest.dword(node + 0xc) != 0)
                fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), guest.dword(node + 8));
            guest.setDword(link, guest.dword(node));
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), node);
            break;
        }
    }
    return fe::pointerCall(guest, guest.dword(kUnlockThreads));
}

nfs::u32 frontend::watcRemoveThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525f84", 0, 0, kEbx | kEcx | kEdx);
    const u32 thread = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::removeThread(app, ts, thread); });
}

/* sub_525ff8: the whole list freed; what its code leaves in eax, _nfree's last or `eax` as it came. */
u32 frontend::watc::freeThreadList(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    u32 node = guest.dword(kThreadList);
    while (node != 0)
    {
        const u32 owned = guest.dword(node + 0xc), next = guest.dword(node);
        if (owned != 0)
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), guest.dword(node + 8));
        eax = fe::direct<kNfree, watcom::clib::free>(app, ts, node);
        node = next;
    }
    return eax;
}

nfs::u32 frontend::watcFreeThreadList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_525ff8", 0, 0, kEbx | kEcx | kEdx);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::freeThreadList(app, ts, eax); });
}

/* sub_526060: the same, by another name: `eax` on to it. */
u32 frontend::watc::freeThreadListJump(App* app, ThreadState& ts, u32 eax)
{
    return jumpTo<0x525ff8, freeThreadList>(app, ts, eax);
}

nfs::u32 frontend::watcFreeThreadListJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::freeThreadListJump(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_526070: the x87's exceptions cleared, which the host has none of -- eax as it was, which its
 * thunk leaves; 0 back to a caller as C++. */
u32 frontend::watc::clearFpExceptions(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
    return 0;
}

nfs::u32 frontend::watcClearFpExceptions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watc::clearFpExceptions(app, ts);
    ts.sp += 4;
    return args.eax;
}

// ---------------------------------------------------------------------------
// Signals
// ---------------------------------------------------------------------------

namespace
{

constexpr u32 kBreakHandlers = 0x56b0bc;  // the handlers of 4 and 7, signals of the whole process

bool processWide(u32 signal)
{
    return signal == 7 || signal == 4;
}

}

/* sub_526080: signal's handler set, the old one returned. */
u32 frontend::watc::setSignalHandler(App* app, ThreadState& ts, u32 signal, u32 handler)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (processWide(signal))
    {
        const u32 at = kBreakHandlers + signal * 8;
        const u32 old = guest.dword(at);
        guest.setDword(at, handler);
        return old;
    }
    const u32 old = guest.dword(signal * 8 + fe::pointerCall(guest, guest.dword(kThreadDataGetter)) + 0x58);
    const u32 data = fe::pointerCall(guest, guest.dword(kThreadDataGetter));
    guest.setDword(signal * 8 + data + 0x58, handler);
    return old;
}

nfs::u32 frontend::watcSetSignalHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526080", 0, 0, kEbx | kEcx);
    const u32 signal = args.eax;
    const u32 handler = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setSignalHandler(app, ts, signal, handler); });
}

/* sub_5260c8: a signal's handler. */
u32 frontend::watc::signalHandler(App* app, ThreadState& ts, u32 signal)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (processWide(signal))
        return guest.dword(kBreakHandlers + signal * 8);
    return guest.dword(fe::pointerCall(guest, guest.dword(kThreadDataGetter)) + signal * 8 + 0x58);
}

nfs::u32 frontend::watcSignalHandler(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5260c8", 0, 0, kEdx);
    const u32 signal = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::signalHandler(app, ts, signal); });
}

/* sub_5260ec: the x87's status a signal goes with. */
u32 frontend::watc::signalMask(App* app, ThreadState& ts, u32 signal)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (processWide(signal))
        return guest.dword(kBreakHandlers + 4 + signal * 8);
    return guest.dword(fe::pointerCall(guest, guest.dword(kThreadDataGetter)) + signal * 8 + 0x5c);
}

nfs::u32 frontend::watcSignalMask(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5260ec", 0, 0, kEdx);
    const u32 signal = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::signalMask(app, ts, signal); });
}

/* sub_526110: the exception handler's question: the handler of a signal that goes with this code. */
u32 frontend::watc::signalForCode(App* app, ThreadState& ts, u32 signal, u32 code)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 mask = fe::direct<0x5260ec, signalMask>(app, ts, signal);
    if (code != mask)
        return 0;
    return fe::direct<0x5260c8, signalHandler>(app, ts, signal);
}

nfs::u32 frontend::watcSignalForCode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526110", 0, 0, kEbx);
    const u32 signal = args.eax;
    const u32 code = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::signalForCode(app, ts, signal, code); });
}

/* sub_52612c: the console control handler: Ctrl-C and Ctrl-Break as signals. */
u32 frontend::watc::consoleControl(App* app, ThreadState& ts, u32 event)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0);
    u32 signal;
    if (event == 0)
        signal = 4;
    else if (event == 1)
        signal = 7;
    else
        return 0;
    const u32 handler = fe::direct<0x5260c8, signalHandler>(app, ts, signal);
    if (handler == 0)
        return 0;
    fe::direct<0x526338, raiseSignal>(app, ts, signal);
    return handler == 2 || handler == 3 ? 0 : 1;
}

nfs::u32 frontend::watcConsoleControl(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52612c", 4, 0, 0);
    const u32 event = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::consoleControl(app, ts, event); });
}

/* sub_526194: whether the console handler is wanted: not when both signals are ignored or default. */
u32 frontend::watc::consoleWanted(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 first = fe::direct<0x5260c8, signalHandler>(app, ts, 4);
    const u32 second = fe::direct<0x5260c8, signalHandler>(app, ts, 7);
    if ((first == 2 || first == 3) && (second == 2 || second == 3))
        return 0;
    return 1;
}

nfs::u32 frontend::watcConsoleWanted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526194", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::consoleWanted(app, ts); });
}

/* sub_5261fc: the console handler taken off: 1 when it is. */
u32 frontend::watc::consoleUnhook(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    constexpr u32 kHooked = 0x56b124;
    // The port has no console's handler to take off: the hook stays as it is.
    return guest.byte(kHooked) == 0 ? 1 : 0;
}

nfs::u32 frontend::watcConsoleUnhook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5261fc", 0, 0, kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::consoleUnhook(app, ts); });
}

/* sub_526244: the floating-point signal raised with its code, the handler reset first. */
u32 frontend::watc::raiseFp(App* app, ThreadState& ts, u32 code)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 handler = fe::direct<0x5260c8, signalHandler>(app, ts, 2);
    if (handler == 1 || handler == 2 || handler == 3)
        return 0xffffffff;
    fe::direct<0x526080, setSignalHandler>(app, ts, 2, 2);
    fe::pointerCall(guest, handler, args({ 2, code }));  // the handler, through its pointer: the signal, the code
    return 0;
}

nfs::u32 frontend::watcRaiseFp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526244", 0, 0, kEbx | kEcx | kEdx);
    const u32 code = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::raiseFp(app, ts, code); });
}

/* sub_526338: raise. */
u32 frontend::watc::raiseSignal(App* app, ThreadState& ts, u32 signal)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 handler = fe::direct<0x5260c8, signalHandler>(app, ts, signal);
    if (signal - 1 > 0xb)
        return 0xffffffff;
    if (signal == 2)
    {
        fe::direct<0x526244, raiseFp>(app, ts, 0x8c);
        return 0;
    }
    if (signal == 1 && handler == 2)
        fe::direct<0x52c878, abnormalTermination>(app, ts);
    if (handler != 1 && handler != 2 && handler != 3)
    {
        fe::direct<0x526080, setSignalHandler>(app, ts, signal, 2);
        // The handler, through its pointer: the signal, and in edx the new handler, as the game's
        // code loads it for sub_526080.
        fe::pointerCall(guest, handler, args({ signal, 2 }));
    }
    if (fe::direct<0x526194, consoleWanted>(app, ts) == 0)
        fe::direct<0x5261fc, consoleUnhook>(app, ts);
    return 0;
}

nfs::u32 frontend::watcRaise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526338", 0, 0, kEbx | kEcx | kEdx);
    const u32 signal = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::raiseSignal(app, ts, signal); });
}

/* sub_5263a8: a new thread's handlers, copied from the process's, and the exception handler's
 * hooks set; `ds`, which its code leaves in eax. */
u32 frontend::watc::initThreadSignals(App* app, ThreadState& ts, u32 ds)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    for (u32 offset = 8; offset != 0x68; offset += 8)
    {
        const u32 data = fe::pointerCall(guest, guest.dword(kThreadDataGetter));
        guest.setDword(data + offset + 0x58, guest.dword(kBreakHandlers + offset));
        guest.setDword(data + offset + 0x5c, guest.dword(kBreakHandlers + offset + 4));
    }
    guest.setDword(0x56acd4, 0x526110);
    guest.setDword(0x56acd8, 0x526338);
    return ds;
}

nfs::u32 frontend::watcInitThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5263a8", 0, 0, kEbx | kEdx | kEsi | kEdi);
    const u32 ds = 0;  // the flat model's data selector, as the port's thread starts with it
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::initThreadSignals(app, ts, ds); });
}

/* sub_5263f0: a thread's end: the console signals back to default, when it was wanted. */
u32 frontend::watc::finiThreadSignals(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    // What the calls left in eax goes back.
    u32 left = fe::direct<0x526194, consoleWanted>(app, ts);
    if (left != 0)
    {
        fe::direct<0x5261fc, consoleUnhook>(app, ts);
        fe::direct<0x526080, setSignalHandler>(app, ts, 4, 2);
        left = fe::direct<0x526080, setSignalHandler>(app, ts, 7, 2);
    }
    return left;
}

nfs::u32 frontend::watcFiniThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5263f0", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::finiThreadSignals(app, ts); });
}

/* sub_526420: the two above hooked; `eax` as it came, which it leaves. */
u32 frontend::watc::hookThreadSignals(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    guest.setDword(kThreadOnStart, 0x5263a8);
    guest.setDword(kThreadOnEnd, 0x5263f0);
    return eax;
}

nfs::u32 frontend::watcHookThreadSignals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_526420", 0, 0, kEbx | kEdx);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::hookThreadSignals(app, ts, eax); });
}

/* sub_52b523: pow's bad cases: the error reported, or 0 where the answer is 0 with no error; the
 * answer, which its thunk leaves on the x87's stack as the code does. */
double frontend::watc::powError(App* app, ThreadState& ts, u32 arguments)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x1c);
    const u32 x = arguments, y = arguments + 8;
    const auto doubleAt = [&](u32 at) { return app->getMemory<double>(at); };
    const std::uint8_t kind = guest.byte(arguments + 0x10);
    u32 code = 0;
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
        // The runtime's error report (native_watcom.cpp's): its answer, which its code leaves on the
        // x87's stack.
        answer = fe::direct<0x5243b1, watcom::clib::reportError>(app, ts, code, x, y);
    return answer;
}

nfs::u32 frontend::watcPowError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b523", 0x14, 0, kEbx | kEcx | kEdx);
    const u32 arguments = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 eax = args.eax;  // as it came
        args.floatSet = true;
        args.floatResult = nfs::Float(watc::powError(app, ts, arguments));
        return eax;
    });
}

// ---------------------------------------------------------------------------
// The debug screen
// ---------------------------------------------------------------------------

/* sub_52b6c0: a rect's clip, each edge held inside its width and height. */
u32 frontend::watc::setRectClip(App* app, ThreadState& ts, u32 rect, u32 leftAsked, u32 topAsked, u32 rightAsked, u32 bottomAsked)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const auto held = [](u32 value, u32 most) {
        const u32 positive = s(value) < 0 ? 0 : value;
        return s(positive) > s(most) ? most : positive;
    };
    const u32 wide = guest.dword(rect + 4), high = guest.dword(rect + 8);
    const u32 left = held(leftAsked, wide), right = held(rightAsked, wide);
    const u32 top = held(topAsked, high), bottom = held(bottomAsked, high);
    guest.setDword(rect + 0x14, right);
    guest.setDword(rect + 0x10, top);
    guest.setDword(rect + 0x18, bottom);
    guest.setDword(rect + 0xc, left);
    return rect;
}

nfs::u32 frontend::watcSetRectClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b6c0", 4, 0, kEsi | kEdi);
    const u32 rect = args.eax;
    const u32 leftAsked = args.edx, topAsked = args.ebx, rightAsked = args.ecx;
    const u32 bottomAsked = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setRectClip(app, ts, rect, leftAsked, topAsked, rightAsked, bottomAsked); });
}

/* sub_52b720: a box's outline, its corners in any order, as four filled rects. */
u32 frontend::watc::drawBox(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (s(left) > s(right))
        std::swap(left, right);
    if (s(bottom) < s(top))
        std::swap(top, bottom);
    const u32 wide = right - left + 1;
    const u32 high = bottom - top - 1;
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, left, top, wide, 1, colour);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, right, top + 1, 1, high, colour);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, left, top + 1, 1, high, colour);
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, left, bottom, wide, 1, colour);
}

nfs::u32 frontend::watcDrawBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b720", 4, 0, kEsi | kEdi);
    const u32 left = args.eax;
    const u32 top = args.edx;
    const u32 right = args.ebx;
    const u32 bottom = args.ecx;
    const u32 colour = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::drawBox(app, ts, left, top, right, bottom, colour); });
}

/* sub_52b860: a line of text centred, the pen put back. */
u32 frontend::watc::textCentred(App* app, ThreadState& ts, u32 text, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr u32 kPenX = 0x5671f8, kPenY = 0x5671fc, kWindowWidth = 0x564384;
    const u32 width = fe::direct<0x4efef0, eab::textMeasureAll>(app, ts, text);
    const u32 room = guest.dword(kWindowWidth) - width;
    const u32 x = u32(s(room) / 2);
    const u32 penX = guest.dword(kPenX), penY = guest.dword(kPenY);
    guest::direct<0x4f0850, wata::textSetPen>(app, ts, x, y);
    const u32 result = fe::direct<0x4efb50, eab::textDrawAtPen>(app, ts, text);
    guest.setDword(kPenX, penX);
    guest.setDword(kPenY, penY);
    return result;
}

nfs::u32 frontend::watcTextCentred(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b860", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const u32 text = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::textCentred(app, ts, text, y); });
}

/* sub_52b8b0: a rect's clip to all of it. */
u32 frontend::watc::clipWholeRect(App* app, ThreadState& ts, u32 rect)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return fe::direct<0x52b6c0, setRectClip>(app, ts, rect, 0, 0, guest.dword(rect + 4), guest.dword(rect + 8));
}

nfs::u32 frontend::watcClipWholeRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b8b0", 0, 0, kEbx | kEcx | kEdx);
    const u32 rect = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::clipWholeRect(app, ts, rect); });
}

/* sub_52b8d0, sub_52bb20: the memory viewer on the two drivers' tables. */
u32 frontend::watc::viewerOnWindow(App* app, ThreadState& ts)
{
    return jumpTo<0x497620, eaa::memViewerRun>(app, ts, 0x56b170u);
}

nfs::u32 frontend::watcViewerOnWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::viewerOnWindow(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

u32 frontend::watc::viewerOnMono(App* app, ThreadState& ts)
{
    return jumpTo<0x497620, eaa::memViewerRun>(app, ts, 0x56b198u);
}

nfs::u32 frontend::watcViewerOnMono(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::viewerOnMono(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52b8e0: the debug window's start: sub_4ead20, then the jump to the text's start. */
u32 frontend::watc::windowStart(App* app, ThreadState& ts)
{
    callTo<0x4ead20, wata::windowApply>(app, ts);
    if (ts.terminate)
        return 0;
    return jumpTo<kTextStart, wina::textStart>(app, ts);
}

nfs::u32 frontend::watcWindowStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::windowStart(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52b8f0: and its end. */
u32 frontend::watc::windowEnd(App* app, ThreadState& ts)
{
    return jumpTo<kBitmapToScreen, eaa::bitmapDrawToScreen>(app, ts);
}

nfs::u32 frontend::watcWindowEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::windowEnd(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52b910: the window driver's sizes: 0 and 1 in characters of 8 pixels, 2 and 3 in pixels. */
u32 frontend::watc::windowSize(App* app, ThreadState& ts, u32 which)
{
    Guest guest(app, ts);
    const auto characters = [](u32 pixels) {
        const u32 eighth = sar(s(pixels) < 0 ? pixels + 7 : pixels, 3);
        const u32 sign = sar(eighth, 31);
        return (eighth ^ sign) - sign;
    };
    switch (which)
    {
    case 1: return characters(guest.dword(kScreenHeight)) - 1;
    case 2: return guest.dword(kScreenWidth);
    case 3: return guest.dword(kScreenHeight);
    case 4: return u32(1);
    default: return characters(guest.dword(kScreenWidth));
    }
}

nfs::u32 frontend::watcWindowSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b910", 0, 0, kEdx);
    const u32 which = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::windowSize(app, ts, which); });
}

/* sub_52b970: a line printed at a character cell. */
u32 frontend::watc::printAtCell(App* app, ThreadState& ts, u32 arguments)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x114);
    const u32 buffer = guest.frame();
    const u32 x = guest.dword(arguments) << 3, y = guest.dword(arguments + 4) << 3;
    guest.setDword(buffer + 0x100, arguments + 12);
    fe::direct<kVsprintf, watcom::clib::vsprintf>(guest.app(), guest.ts(), buffer, guest.dword(arguments + 8), buffer + 0x100);
    const u32 result = fe::direct<0x52e4e0, textAt>(app, ts, buffer, x, y);
    guest.setDword(buffer + 0x100, 0);
    return result;
}

nfs::u32 frontend::watcPrintAtCell(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b970", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 arguments = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::printAtCell(app, ts, arguments); });
}

/* sub_52b9d0: the driver's hook that does nothing -- eax as it was, which its thunk leaves; 0 back
 * to a caller as C++. */
u32 frontend::watc::nothing52b9d0(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
    return 0;
}

nfs::u32 frontend::watcNothing52b9d0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watc::nothing52b9d0(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* sub_52b9e0: a shape drawn on black. */
u32 frontend::watc::blitOnBlack(App* app, ThreadState& ts, u32 shape, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 black = fe::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000);
    fe::direct<0x4ef1b0, eab::drawClearClip>(app, ts, black);   // the colour as it came back in eax
    return fe::direct<0x4f09c0, wata::blitDispatchAt>(app, ts, shape, x, y);
}

nfs::u32 frontend::watcBlitOnBlack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52b9e0", 0, 0, kEsi);
    const u32 shape = args.eax;
    const u32 x = args.ebx;
    const u32 y = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::blitOnBlack(app, ts, shape, x, y); });
}

/* sub_52ba00: the text's colours by the debug screen's numbers. */
u32 frontend::watc::setTextColours(App* app, ThreadState& ts, u32 ink, u32 paper)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 inkColour = fe::direct<0x52ba70, debugColour>(app, ts, ink);
    const u32 paperColour = fe::direct<0x52ba70, debugColour>(app, ts, paper);
    return fe::direct<0x4ef830, eab::fontSetColours>(app, ts, inkColour, paperColour);
}

nfs::u32 frontend::watcSetTextColours(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ba00", 0, 0, kEcx);
    const u32 ink = args.eax;
    const u32 paper = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setTextColours(app, ts, ink, paper); });
}

/* sub_52ba70: a debug-screen colour, one of the CGA's by its number, in the window's format. */
u32 frontend::watc::debugColour(App* app, ThreadState& ts, u32 colour)
{
    static constexpr u32 kColours[19] = {
        0xff00aaaa, 0xff000000, 0xffffffff, 0xff0000aa, 0xff000000, 0xffffffff, 0xff00aaaa, 0xffffffff, 0xff000000, 0xffaa0000,
        0xffffffff, 0xffaa0000, 0xffffff50, 0xffffffff, 0xff0000aa, 0xffaa0000, 0xffffffff, 0xffffffff, 0xffaa0000,
    };
    return jumpTo<kRgbToColour, eab::colourPack>(app, ts, colour > 0x12 ? 0xff00aaaa : kColours[colour]);
}

nfs::u32 frontend::watcDebugColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::debugColour(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

namespace
{

/* sub_52bac0's body: a key waited for, its letter in capitals. */
u32 keyWaitedFor(Guest& guest, nfs::ThreadState& ts)
{
    fe::direct<0x4f00f0, frontend::eab::keySetHookFlag>(guest.app(), ts, 0x4f00e0);
    u32 key;
    do
        key = fe::direct<0x4f0120, ea::waitKey>(guest.app(), ts).value_or(0);
    while (key == 0);
    return s(key) >= 0x61 && s(key) <= 0x7a ? key - 0x20 : key;
}

}

u32 frontend::watc::windowWaitKey(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return keyWaitedFor(guest, ts);
}

nfs::u32 frontend::watcWaitKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bac0", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::windowWaitKey(app, ts); });
}

/* sub_52baf0: a filled box in character cells, in a debug colour. */
u32 frontend::watc::fillCells(App* app, ThreadState& ts, u32 left, u32 top, u32 wide, u32 high, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 shown = fe::direct<0x52ba70, debugColour>(app, ts, colour);
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, left << 3, top << 3, wide << 3, high << 3, shown);
}

nfs::u32 frontend::watcFillCells(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52baf0", 4, 0, kEsi);
    const u32 left = args.eax;
    const u32 top = args.edx;
    const u32 wide = args.ebx;
    const u32 high = args.ecx;
    const u32 colour = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::fillCells(app, ts, left, top, wide, high, colour); });
}

/* sub_52bb50: the mono driver's sizes. */
u32 frontend::watc::monoSize(App* app, ThreadState& ts, u32 which)
{
    Guest guest(app, ts);
    switch (which)
    {
    case 1: return u32(0x18);
    case 2: return guest.dword(kScreenWidth);
    case 3: return guest.dword(kScreenHeight);
    case 4: return u32(0);
    default: return u32(0x50);
    }
}

nfs::u32 frontend::watcMonoSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bb50", 0, 0, 0);
    const u32 which = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::monoSize(app, ts, which); });
}

/* sub_52bb80: the mono screen cleared, by sub_4e0ca0 -- `eax` on to it, as it came, which it reads
 * (by address: native_ea_support.cpp has no typed core of it). */
u32 frontend::watc::monoClearJump(App* app, ThreadState& ts, u32 eax)
{
    return jumpByAddress(app, ts, 0x4e0ca0, args({ eax }));
}

nfs::u32 frontend::watcMonoClearJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::monoClearJump(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52bb90: a shape centred on the cleared screen, on black. */
u32 frontend::watc::showCentred(App* app, ThreadState& ts, u32 shape)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    fe::direct<0x52e420, wina::textStart>(app, ts);
    const u32 shapeWide = sar(guest.dword(shape + 2), 16);
    const u32 roomX = guest::direct<0x52bb50, monoSize>(app, ts, 2);
    const u32 shapeHigh = sar(guest.dword(shape + 4), 16);
    const u32 roomY = guest::direct<0x52bb50, monoSize>(app, ts, 3);
    const u32 x = sar(roomX - shapeWide, 1), y = sar(roomY - shapeHigh, 1);
    const u32 black = fe::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000);
    // The colour as it came back in eax; what that gives back in eax on to sub_4e0ca0, which reads it.
    const u32 cleared = fe::direct<0x4ef1b0, eab::drawClearClip>(app, ts, black);
    fe::direct<0x4e0ca0, ea::monoCleared>(app, ts, cleared);
    fe::direct<0x4f09c0, wata::blitDispatchAt>(app, ts, shape, x, y);
    return fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
}

nfs::u32 frontend::watcShowCentred(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bb90", 0, 0, kEsi);
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::showCentred(app, ts, shape); });
}

/* sub_52bbf0: the mono screen's attribute, from a debug colour. */
u32 frontend::watc::monoAttribute(App* app, ThreadState& ts, u32 colour)
{
    // sub_52bd00 called as C++, what it gives back in eax on to the jump to sub_4e0d50 (by address:
    // native_ea_support.cpp has no typed core of it).
    const u32 attribute = guest::direct<0x52bd00, monoAttributeOf>(app, ts, colour);
    if (ts.terminate)
        return 0;
    return jumpByAddress(app, ts, 0x4e0d50, args({ attribute }));
}

nfs::u32 frontend::watcMonoAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::monoAttribute(app, ts, args.eax);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52bc00: a line printed through a window's text at a character cell. */
u32 frontend::watc::printInWindow(App* app, ThreadState& ts, u32 arguments)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x118);
    const u32 buffer = guest.frame();
    const u32 x = guest.dword(arguments) << 3, y = guest.dword(arguments + 4) << 3;
    if (guest.dword(0x567294) == 0x50b580)
    {
        guest.setDword(kErrorFile, 0x552050);
        guest.setDword(kErrorFunction, 0x552060);
        guest.setDword(kErrorLine, 0x72);
        fe::direct<kError, ea::error>(guest.app(), guest.ts(), 0x552074, guest::Words({}));
    }
    guest.setDword(buffer + 0x100, arguments + 12);
    fe::direct<kVsprintf, watcom::clib::vsprintf>(guest.app(), guest.ts(), buffer, guest.dword(arguments + 8), buffer + 0x100);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    fe::direct<0x52e420, wina::textStart>(app, ts);
    fe::direct<0x52e4e0, textAt>(app, ts, buffer, x, y);
    guest.setDword(buffer + 0x100, 0);
    return fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
}

nfs::u32 frontend::watcPrintInWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bc00", 0, 0, kEbx | kEcx | kEdx | kEsi);
    const u32 arguments = ts.sp + 4;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::printInWindow(app, ts, arguments); });
}

/* sub_52bd00: a debug colour's mono attribute. */
u32 frontend::watc::monoAttributeOf(App* app, ThreadState& ts, u32 colour)
{
    static constexpr std::uint8_t kBright[19] = { 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0 };
    return colour <= 0x12 && kBright[colour] != 0 ? 0x70 : 7;
}

nfs::u32 frontend::watcMonoAttributeOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bd00", 0, 0, 0);
    const u32 colour = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::monoAttributeOf(app, ts, colour); });
}

/* sub_52bd20: the mono driver's key: as the window's. */
u32 frontend::watc::monoWaitKey(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return keyWaitedFor(guest, ts);
}

nfs::u32 frontend::watcMonoWaitKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bd20", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::monoWaitKey(app, ts); });
}

/* sub_52bd50: a box of spaces on the mono screen. */
u32 frontend::watc::monoFill(App* app, ThreadState& ts, u32 left, u32 top, u32 wide, u32 high, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr u32 kSpace = 0x5520b4;
    const u32 bottom = high + top;
    // What each call gives back in eax: on to the next, in eax as the code has it, and what this one
    // leaves there.
    u32 eax = fe::direct<0x52bbf0, monoAttribute>(app, ts, colour);
    u32 row = top;
    if (s(bottom) > s(row))
    {
        const u32 count = wide;
        do
        {
            if (s(wide) > 0)
            {
                const u32 right = left + count;
                eax = count;
                u32 column = left;
                do
                {
                    eax = fe::direct<0x4e0980, ea::monoPrintedAt>(app, ts, eax, column, row, kSpace, guest::Words({})).value_or(0);
                    ++column;
                } while (s(column) < s(right));
            }
            ++row;
        } while (s(row) < s(bottom));
    }
    return eax;
}

nfs::u32 frontend::watcMonoFill(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bd50", 4, 0, kEsi | kEdi);
    const u32 left = args.eax, top = args.edx, wide = args.ebx, high = args.ecx;
    const u32 colour = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::monoFill(app, ts, left, top, wide, high, colour); });
}

// ---------------------------------------------------------------------------
// C_FILE_INFO and environ again
// ---------------------------------------------------------------------------

/* sub_52bdb0: the handles a parent hands down in C_FILE_INFO: fields handle:os handle:mode*, then
 * the variable taken away. */
u32 frontend::watc::inheritHandles(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 40);
    const u32 buffer = guest.frame();
    u32 at = fe::direct<0x50ead0, watcom::clib::getenv>(guest.app(), guest.ts(), 0x5520b8);
    if (at == 0)
        return 0;
    // A field up to its separator as a number; `at` past the separator.
    const auto field = [&](std::uint8_t separator) {
        u32 stop = at;
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
        const u32 length = stop - at;
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), buffer, at, length);
        guest.setByte(buffer + length, 0);
        const u32 value = guest::direct<0x52e660, stringToLongSigned>(app, ts, buffer, 0, 0x10);
        at = stop + 1;
        return value;
    };
    while (guest.byte(at) != 0)
    {
        const u32 handle = field(':');
        guest.setDword(buffer + 0xc, field(':'));
        const u32 mode = field('*');
        guest::direct<0x51e388, setOsHandle>(app, ts, guest.dword(buffer + 0xc), handle);
        fe::direct<0x522e38, watcom::clib::setIOMode>(app, ts, handle, mode);
    }
    return guest::direct<0x52e6d0, putEnvironment>(app, ts, 0x5520c4);
}

nfs::u32 frontend::watcInheritHandles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bdb0", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::inheritHandles(app, ts); });
}

/* sub_52bed0: environ's own strings freed and its list emptied. */
u32 frontend::watc::clearEnviron(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    u32 at = guest.dword(kEnviron);
    if (at == 0)
        return 0;
    for (u32 string = guest.dword(at); string != 0; at += 4, string = guest.dword(at))
    {
        if (guest.dword(kEnvironFlags) == 0)
            continue;
        const u32 index = sar(at - guest.dword(kEnviron), 2);
        if (guest.byte(index + guest.dword(kEnvironFlags)) != 0)
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), string);
        guest.setDword(at, 0);
    }
    u32 list;
    if (guest.dword(kEnvironFlags) == 0)
        list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), 5);
    else
        list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.ts(), guest.dword(kEnviron), 5);
    if (list == 0)
        return 0xffffffff;
    guest.setDword(kEnviron, list);
    guest.setDword(list, 0);
    guest.setDword(kEnvironFlags, list + 4);
    guest.setByte(list + 4, 0);
    return 0;
}

nfs::u32 frontend::watcClearEnviron(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52bed0", 0, 0, kEbx | kEcx | kEdx | kEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::clearEnviron(app, ts); });
}

/* sub_52c810: 0. */
u32 frontend::watc::returnZero52c810(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::watcReturnZero52c810(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c810", 0, 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::returnZero52c810(app, ts); });
}

/* sub_52c81c, sub_52c824, sub_52ed20: calls through the hooks of the thread start and end, and the
 * math error. */
/* Each hook is handed eax as its caller has it, on through these. */
u32 frontend::watc::threadEndHook(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    return fe::pointerCall(guest, guest.dword(0x56b1c4), args({ eax }));
}

nfs::u32 frontend::watcThreadEndHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c81c", 0, 0, 0);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::threadEndHook(app, ts, eax); });
}

u32 frontend::watc::threadStartHook(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    return fe::pointerCall(guest, guest.dword(0x56b1c8), args({ eax }));
}

nfs::u32 frontend::watcThreadStartHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c824", 0, 0, 0);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::threadStartHook(app, ts, eax); });
}

u32 frontend::watc::mathErrorHook(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    return fe::pointerCall(guest, guest.dword(0x567853), args({ eax }));
}

nfs::u32 frontend::watcMathErrorHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ed20", 0, 0, 0);
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::mathErrorHook(app, ts, eax); });
}

/* sub_52c878: abort's report. */
u32 frontend::watc::abnormalTermination(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<kFatalError, watcom::clib::fatalRuntimeError>(app, ts, 0x5520d4u, 0xffu);
}

nfs::u32 frontend::watcAbnormalTermination(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c878", 0, 0, kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::abnormalTermination(app, ts); });
}

/* sub_52c88a: a double's 0, in edx:eax: high and low dwords. */
std::uint64_t frontend::watc::doubleZero(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
    return 0;
}

/* The thunks of these: the double's low dword in eax, its high one left in edx. */
nfs::u32 frontend::watcDoubleZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c88a", 0, 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const std::uint64_t d = watc::doubleZero(app, ts);
        args.edx = u32(d >> 32);
        return u32(d);
    });
}

/* sub_52c89b: the math error (0x83 handed to its hook), and a double's infinity, in edx:eax, of the
 * sign in what that gives back. */
std::uint64_t frontend::watc::doubleInfinityHook(App* app, ThreadState& ts)
{
    const u32 sign = fe::direct<0x52ed20, mathErrorHook>(app, ts, 0x83u);
    return std::uint64_t((sign & 0x80000000) | 0x7ff00000) << 32;
}

nfs::u32 frontend::watcDoubleInfinityHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c89b", 0, 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const std::uint64_t d = watc::doubleInfinityHook(app, ts);
        args.edx = u32(d >> 32);
        return u32(d);
    });
}

/* sub_52c8a7: errno's ERANGE, the math error (0x84 handed to its hook), and a double's infinity of
 * the sign in eax, in edx:eax. */
std::uint64_t frontend::watc::doubleInfinity(App* app, ThreadState& ts, u32 sign)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x5028a4, watcom::clib::setERANGE>(app, ts);
    fe::direct<0x52ed20, mathErrorHook>(app, ts, 0x84u);
    return std::uint64_t((sign & 0x80000000) | 0x7ff00000) << 32;
}

nfs::u32 frontend::watcDoubleInfinity(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52c8a7", 0, 0, 0);
    const u32 sign = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const std::uint64_t d = watc::doubleInfinity(app, ts, sign);
        args.edx = u32(d >> 32);
        return u32(d);
    });
}

/* sub_52e3f8: the driver's hook, 0. */
u32 frontend::watc::zeroJump(App* app, ThreadState& ts)
{
    return jumpTo<0x52edfa, returnZero52edfa>(app, ts);
}

nfs::u32 frontend::watcZeroJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watc::zeroJump(app, ts);
    if (!ts.terminate)
        args.eax = result, ts.sp += 4;
    return args.eax;
}

/* sub_52e4e0: a line of text at a position. */
u32 frontend::watc::textAt(App* app, ThreadState& ts, u32 text, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest::direct<0x4f0850, wata::textSetPen>(app, ts, x, y);
    return fe::direct<0x52e440, menz::textBackgroundBox>(app, ts, text);
}

nfs::u32 frontend::watcTextAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e4e0", 0, 0, kEcx);
    const u32 text = args.eax;
    const u32 x = args.edx;
    const u32 y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::textAt(app, ts, text, x, y); });
}

/* sub_52e500: strtol and strtoul: ecx 1 for the signed. */
u32 frontend::watc::stringToLong(App* app, ThreadState& ts, u32 text, u32 end, u32 radix, u32 isSigned)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    u32 base = radix;
    constexpr u32 kCharacterClass = 0x564ef0, kOverflowLimits = 0x56b648;
    constexpr std::uint8_t kSpace = 2;
    constexpr u32 kEinval = 0xd, kErange = 0xe;
    if (end != 0)
        guest.setDword(end, text);
    u32 at = text;
    while ((guest.byte(kCharacterClass + std::uint8_t(guest.byte(at) + 1)) & kSpace) != 0)
        ++at;
    const std::uint8_t sign = guest.byte(at);
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
        fe::direct<kSetErrno, watcom::clib::setErrno>(app, ts, kEinval);
        return 0;
    }
    else if (base == 0x10 && hasHexPrefix())
    {
        at += 2;
    }
    const u32 start = at;
    const u32 limit = guest.dword(kOverflowLimits + base * 4);
    bool overflow = false;
    u32 value = 0;
    for (;;)
    {
        const u32 digit = fe::direct<0x52e670, digitValue>(app, ts, std::uint8_t(guest.byte(at)));
        if (s(digit) >= s(base))
            break;
        if (value > limit)
            overflow = true;
        const u32 before = value;
        value = value * base + digit;
        if (value < before)
            overflow = true;
        ++at;
    }
    if (at == start)
        at = text;
    if (end != 0)
        guest.setDword(end, at);
    if ((isSigned == 1 && value >= 0x80000000 && (value != 0x80000000 || sign != '-')) || overflow)
    {
        fe::direct<kSetErrno, watcom::clib::setErrno>(app, ts, kErange);
        if (isSigned == 0)
            return 0xffffffff;
        return sign == '-' ? 0x80000000 : 0x7fffffff;
    }
    if (sign == '-')
        value = u32(0) - value;
    return value;
}

nfs::u32 frontend::watcStrtol(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e500", 0, 0, kEsi | kEdi);
    const u32 text = args.eax, end = args.edx, isSigned = args.ecx;
    const u32 radix = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::stringToLong(app, ts, text, end, radix, isSigned); });
}

/* sub_52e660: strtol. */
u32 frontend::watc::stringToLongSigned(App* app, ThreadState& ts, u32 text, u32 end, u32 radix)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    // On to sub_52e500 with them as they came, signed.
    return fe::direct<0x52e500, stringToLong>(app, ts, text, end, radix, 1);
}

nfs::u32 frontend::watcStrtolSigned(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e660", 0, 0, kEcx);
    const u32 text = args.eax, end = args.edx, radix = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::stringToLongSigned(app, ts, text, end, radix); });
}

/* sub_52e670: a character's value as a digit of any base, 0x25 for none. */
u32 frontend::watc::digitValue(App* app, ThreadState& ts, std::uint8_t c)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (c >= '0' && c <= '9')
        return u32(c) - 0x30;
    const std::uint8_t lower = std::uint8_t(fe::direct<0x4f0ae0, watcom::clib::tolower>(guest.app(), guest.ts(), c));
    if (lower >= 'a' && lower <= 'z')
        return u32(lower) - 0x57;
    return 0x25;
}

nfs::u32 frontend::watcDigitValue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e670", 0, 0, kEdx);
    const std::uint8_t c = std::uint8_t(args.eax);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::digitValue(app, ts, c); });
}

/* sub_52e6d0: putenv: Windows' variable set, then environ, then the wide one if there is one. */
u32 frontend::watc::putEnvironment(App* app, ThreadState& ts, u32 text)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 48);
    constexpr u32 kFailed = 0xffffffff;
    constexpr u32 kWideEnviron = 0xa0b158;
    const u32 equals = guest::direct<0x52ee60, mbschr>(app, ts, text, 0x3d);
    if (equals == 0 || equals == text)
        return kFailed;
    const u32 nameLength = equals - text;
    const u32 name = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), nameLength + 1);
    if (name == 0)
        return kFailed;
    copyUp(guest, name, text, nameLength);
    guest.setByte(name + nameLength, 0);
    const u32 from = equals + 1;
    const u32 valueLength = fe::direct<kMbslen, watcom::clib::mbslen>(app, ts, from);
    u32 value = 0;
    if (valueLength != 0)
    {
        value = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), valueLength + 1);
        if (value == 0)
        {
            fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), name);
            return kFailed;
        }
        copyUp(guest, value, from, valueLength);
        guest.setByte(value + valueLength, 0);
    }
    // The port keeps no environment of its own: setting it there fails.
    const u32 set = 0;
    fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), name);
    fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), value);
    if (set == 0)
        return kFailed;
    // The text has its '=' past its start, so edx, which sub_52e88c reads for an empty one, is not
    // its.
    if (guest::direct<0x52e88c, environSet>(app, ts, text, 0u) != 0)
        return kFailed;
    if (guest.dword(kWideEnviron) == 0)
        return 0;
    const u32 characters = fe::direct<kMbslen, watcom::clib::mbslen>(app, ts, text) + 1;
    const u32 wide = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), 2 * characters);
    if (wide == 0)
    {
        fe::direct<0x500f8c, watcom::clib::setErrnoDos>(app, ts, 5u);
        return kFailed;
    }
    if (fe::direct<0x518080, watcom::clib::mbstowcs>(app, ts, wide, text, characters) == kFailed)
    {
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), wide);
        return kFailed;
    }
    return fe::direct<0x52c178, watcom::clib::wputenv>(app, ts, wide);
}

nfs::u32 frontend::watcPutenv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e6d0", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::putEnvironment(app, ts, text); });
}

/* sub_52e88c: the string put in environ, or the variable taken out if it has nothing after =.  For
 * an empty text its code looks for the '=' at `edx`, as the caller has it. */
u32 frontend::watc::environSet(App* app, ThreadState& ts, u32 text, u32 edx)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 36);
    constexpr u32 kFailed = 0xffffffff;
    if (text == 0)
        return kFailed;
    u32 at = edx;
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
    u32 list = guest.dword(kEnviron);
    u32 index = 0;
    if (list == 0)
    {
        if (removing)
            return 0;
        list = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), 9);
        if (list == 0)
            return kFailed;
        guest.setDword(kEnviron, list);
        guest.setDword(list, 0);
        guest.setDword(list + 4, 0);
        guest.setDword(kEnvironFlags, list + 8);
    }
    else
    {
        const u32 found = guest::direct<0x52ea3c, environFind>(app, ts, text, removing ? 1u : 0u);
        if (removing)
            return 0;
        if (s(found) > 0)
        {
            index = found - 1;
        }
        else
        {
            // New: it has -found strings; the list one longer, and the flags after it.
            index = u32(0) - found;
            const u32 flagCount = index + 1;
            const u32 listBytes = index * 4 + 8;
            const u32 flags = guest.dword(kEnvironFlags);
            if (flags == 0)
            {
                const u32 grown = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), listBytes + flagCount);
                list = grown;
                if (grown == 0)
                    return kFailed;
                copyUp(guest, grown, guest.dword(kEnviron), index * 4);
                guest.setDword(kEnvironFlags, listBytes + grown);
                fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), listBytes + grown, 0, flagCount);
            }
            else
            {
                list = fe::direct<kNrealloc, watcom::clib::realloc>(guest.app(), guest.ts(), list, listBytes + flagCount);
                if (list == 0)
                    return kFailed;
                const u32 moved = listBytes + list;
                fe::direct<kMemmove, watcom::clib::memmove>(app, ts, moved, guest.dword(kEnvironFlags), index);
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

nfs::u32 frontend::watcEnvironSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52e88c", 0, 0, kEbx | kEcx | kEdx | kEsi | kEdi);
    const u32 text = args.eax, edx = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::environSet(app, ts, text, edx); });
}

/* sub_52ea3c: a variable looked for in environ, case aside: its index + 1, or with edx set the
 * variable taken out and 0; if not there minus the count of strings. */
u32 frontend::watc::environFind(App* app, ThreadState& ts, u32 name, u32 removing)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    u32 entry = guest.dword(kEnviron);
    for (;; entry += 4)
    {
        u32 string = guest.dword(entry);
        if (string == 0)
            return sar(guest.dword(kEnviron) - entry, 2);
        u32 wanted = name;
        if (guest.byte(wanted) == 0)
            continue;
        bool same = false;
        for (;;)
        {
            const u32 upperOwn = fe::direct<kToupper, watcom::clib::toupper>(guest.app(), guest.ts(), guest.byte(string));
            const u32 upperWanted = fe::direct<kToupper, watcom::clib::toupper>(guest.app(), guest.ts(), guest.byte(wanted));
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
        u32 index = sar(entry - guest.dword(kEnviron), 2);
        if (removing == 0)
            return index + 1;
        const u32 removed = guest.dword(entry);
        u32 last = entry;
        if (removed != 0)
        {
            u32 next;
            do
            {
                guest.setDword(last, guest.dword(last + 4));
                next = guest.dword(last + 4);
                last += 4;
            } while (next != 0);
        }
        const u32 flags = guest.dword(kEnvironFlags);
        if (flags != 0)
        {
            if (guest.byte(index + flags) != 0)
                fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), removed);
            const u32 count = sar(last - guest.dword(kEnviron), 2);
            fe::direct<kMemmove, watcom::clib::memmove>(app, ts, last, guest.dword(kEnvironFlags), count);
            guest.setDword(kEnvironFlags, last);
            for (; s(index) < s(count); ++index)
                guest.setByte(index + last, guest.byte(index + last + 1));
        }
        return 0;
    }
}

nfs::u32 frontend::watcEnvironFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ea3c", 0, 0, kEbx | kEcx | kEsi | kEdi);
    const u32 name = args.eax, removing = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::environFind(app, ts, name, removing); });
}

/* sub_52eb70: a thread's first function: its data made, _beginthread told, the function called,
 * the thread's end. */
u32 frontend::watc::threadStart(App* app, ThreadState& ts, u32 parameters)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 40);
    constexpr u32 kSingleThread = 0x9f6420;
    const u32 frame = guest.frame();
    const u32 function = guest.dword(parameters), argument = guest.dword(parameters + 4);
    const u32 single = guest.dword(kSingleThread);
    guest.setDword(frame + 8, function);
    if (single == 0)
    {
        // The data on the stack, for a thread of the program's.
        ts.sp -= (guest.dword(kThreadDataSize) + 3) & 0xfffffffc;
        const u32 data = ts.sp;
        const u32 size = guest.dword(kThreadDataSize);
        fe::direct<0x4e0640, watcom::clib::memset>(app, ts, data, 0u, size);
        guest.setDword(data + 0xf0, guest.dword(kThreadDataSize));
        // On through it to sub_51e7ec: the block to take.
        if (fe::direct<0x51e888, attachThread>(app, ts, data) == 0)
            return 0;
    }
    const u32 own = fe::pointerCall(guest, guest.dword(kThreadDataGetter)) + 0xde;
    guest.setDword(parameters + 0x10, own);
    os::events::set(guest.app(), guest.dword(parameters + 0xc));
    fe::direct<0x51f0d0, watcom::clib::initExceptionHandling>(app, ts, frame);
    fe::pointerCall(guest, guest.dword(kThreadOnStart));
    const u32 result = fe::pointerCall(guest, guest.dword(frame + 8), args({ argument }));
    return fe::direct<0x52c81c, threadEndHook>(app, ts, result);   // its eax on to the hook
}

nfs::u32 frontend::watcThreadStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52eb70", 0, 0, kEbx | kEsi | kEdi);
    const u32 parameters = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::threadStart(app, ts, parameters); });
}

/* sub_52ec04: _beginthread: the thread made and waited for until it has its data. */
u32 frontend::watc::beginThread(App* app, ThreadState& ts, u32 function, u32 stackSize, u32 argument)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x4c);
    const u32 frame = guest.frame();
    constexpr u32 kEventName = 0x552134;
    if (guest.dword(kTlsIndex) == 0xffffffff)
    {
        if (fe::direct<0x51e828, allocTlsIndex>(app, ts) == 0)
            return 0;
        fe::direct<0x51e964, initThreads>(app, ts);
    }
    guest.setDword(frame + 0x20, function);
    guest.setDword(frame + 0x24, argument);
    guest.setDword(frame + 0x28, os::threads::current(guest.app()));
    guest.setDword(frame + 0x30, 0);
    copyUp(guest, frame, kEventName, 9);
    const u32 thread = os::threads::currentId(guest.app(), ts);
    fe::direct<0x51e0ec, watcom::clib::itoa>(app, ts, thread, frame + 8, 0x10u);
    const u32 event = os::events::make(app, false, false, &app->getMemory<const char>(frame));
    guest.setDword(frame + 0x2c, event);
    const os::threads::Started started =
        os::threads::start(app, 0x52eb70, frame + 0x20, (stackSize + 0xfff) & 0xfffff000, false);
    guest.setDword(frame + 0x34, started.id);
    const u32 handle = started.handle;
    if (handle == 0)
    {
        guest.setDword(frame + 0x34, 0xffffffff);
    }
    else
    {
        if (!os::events::wait(app, ts, guest.dword(frame + 0x2c), os::kForever))
            throw fe::Terminated();
        const u32 slot = guest.dword(frame + 0x30);
        if (slot != 0)
            guest.setDword(slot, handle);
        else
            os::release(app, handle);
    }
    os::release(app, guest.dword(frame + 0x2c));
    return handle;
}

nfs::u32 frontend::watcBeginThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ec04", 0, 0, kEcx | kEsi | kEdi);
    const u32 function = args.eax, stackSize = args.edx;
    const u32 argument = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::beginThread(app, ts, function, stackSize, argument); });
}

/* sub_52ecf0: _endthread. */
u32 frontend::watc::endThread(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    constexpr u32 kSingleThread = 0x9f6420;
    const u32 ended = fe::pointerCall(guest, guest.dword(kThreadOnEnd));
    fe::direct<0x51f11c, watcom::clib::doneExceptionHandler>(app, ts, ended);
    if (guest.dword(kSingleThread) == 0)
        fe::direct<0x51e8dc, detachThread>(app, ts, 1);  // 1 in eax, as the game's code hands it
    os::threads::exit(ts);
    throw fe::Terminated();
}

nfs::u32 frontend::watcEndThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ecf0", 0, 0, kEcx | kEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::endThread(app, ts); });
}

/* sub_52edfa: 0. */
u32 frontend::watc::returnZero52edfa(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::watcReturnZero52edfa(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52edfa", 0, 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::returnZero52edfa(app, ts); });
}

/* sub_52ee00: the window's clip, held inside the screen. */
u32 frontend::watc::setClip(App* app, ThreadState& ts, u32 leftAsked, u32 topAsked, u32 rightAsked, u32 bottomAsked)
{
    Guest guest(app, ts);
    const auto held = [](u32 value, u32 most) {
        const u32 positive = s(value) < 0 ? 0 : value;
        return s(positive) > s(most) ? most : positive;
    };
    const u32 wide = guest.dword(kScreenWidth), high = guest.dword(kScreenHeight);
    const u32 left = held(leftAsked, wide), right = held(rightAsked, wide);
    const u32 top = held(topAsked, high), bottom = held(bottomAsked, high);
    guest.setDword(kClipRight, right);
    guest.setDword(kClipTop, top);
    guest.setDword(kClipBottom, bottom);
    guest.setDword(kClipLeft, left);
    return left;
}

nfs::u32 frontend::watcSetClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ee00", 0, 0, kEsi);
    const u32 leftAsked = args.eax;
    const u32 topAsked = args.edx;
    const u32 rightAsked = args.ebx;
    const u32 bottomAsked = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::setClip(app, ts, leftAsked, topAsked, rightAsked, bottomAsked); });
}

/* sub_52ee60: _mbschr: the multibyte character found, or the end if asked for 0. */
u32 frontend::watc::mbschr(App* app, ThreadState& ts, u32 text, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    u32 at = text;
    constexpr u32 kMbterm = 0x5250c0;
    const u32 character = guest.frame();
    fe::direct<0x52ede0, watcom::clib::mbvtop>(app, ts, wanted, character);
    const u32 length = fe::direct<0x52b5b0, watcom::clib::mbclen>(app, ts, character);
    guest.setByte(character + length, 0);  // dl, which the code clears
    while (fe::direct<kMbterm, watcom::clib::mbterm>(app, ts, at) == 0
           && fe::direct<0x52e3a0, watcom::clib::mbccmp>(app, ts, at, character) != 0)
        at = fe::direct<0x525100, watcom::clib::mbsinc>(app, ts, at);
    if (fe::direct<kMbterm, watcom::clib::mbterm>(app, ts, at) != 0 && wanted != 0)
        return 0;
    return at;
}

nfs::u32 frontend::watcMbschr(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52ee60", 0, 0, kEbx | kEcx);
    const u32 wanted = args.edx;
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::mbschr(app, ts, text, wanted); });
}

// ---------------------------------------------------------------------------
// TAPI32.DLL, loaded when the modem code first asks
// ---------------------------------------------------------------------------

namespace
{

constexpr u32 kLineUnavailable = 0x80000049;  // LINEERR_OPERATIONUNAVAIL

/* A stub of TAPI's: the function found the first time, and called with what the stub was. */
template <std::size_t Count>
u32 tapiStub(nfs::App* app, nfs::ThreadState& ts, u32 frameBytes, u32 slot, u32 name,
               const std::array<u32, Count>& words)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, frameBytes);
    if (guest.dword(slot) == 0)
    {
        const u32 failed = fe::direct<0x52f318, frontend::watc::tapiFind>(app, ts, name, slot);
        if (failed != 0)
        {
            guest.setDword(slot, 0xffffffff);
            return failed;
        }
    }
    if (guest.dword(slot) == 0xffffffff)
        return kLineUnavailable;
    fe::Arguments arguments = args({ kLineUnavailable });
    for (const u32 word : words)
        arguments.stack[arguments.stackCount++] = word;
    return fe::pointerCall(guest, guest.dword(slot), arguments);
}

}

/* sub_52f318: TAPI32.DLL loaded if it is not, a function looked up into its slot. */
u32 frontend::watc::tapiFind(App* app, ThreadState& ts, u32 name, u32 slot)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0);
    constexpr u32 kTapi = 0x56d12c, kTapiName = 0x56b9d8;
    if (guest.dword(kTapi) == 0)
    {
        const u32 library = os::modules::load(app, guest::text(app, kTapiName)).value_or(0);
        guest.setDword(kTapi, library);
        if (library == 0)
            guest.setDword(kTapi, 0xffffffff);
    }
    if (guest.dword(kTapi) == 0xffffffff)
        return kLineUnavailable;
    const u32 found = os::modules::symbol(app, guest.dword(kTapi), guest::text(app, name));
    guest.setDword(slot, found);
    return found == 0 ? kLineUnavailable : 0;
}

nfs::u32 frontend::watcTapiFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52f318", 8, 0, 0);
    const u32 name = app->getMemory<u32>(ts.sp + 4);
    const u32 slot = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::tapiFind(app, ts, name, slot); });
}

/* sub_52f535: lineAnswer. */
u32 frontend::watc::lineAnswer(App* app, ThreadState& ts, u32 callHandle, u32 userUserInfo, u32 size)
{
    return tapiStub<3>(app, ts, 0, 0x56d14c, 0x56ba50, { callHandle, userUserInfo, size });
}

nfs::u32 frontend::watcLineAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52f535", 12, 0, 0);
    const u32 callHandle = app->getMemory<u32>(ts.sp + 4);
    const u32 userUserInfo = app->getMemory<u32>(ts.sp + 8);
    const u32 size = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineAnswer(app, ts, callHandle, userUserInfo, size); });
}

/* sub_52f661: lineClose. */
u32 frontend::watc::lineClose(App* app, ThreadState& ts, u32 line)
{
    return tapiStub<1>(app, ts, 0, 0x56d15c, 0x56ba98, { line });
}

nfs::u32 frontend::watcLineClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52f661", 4, 0, 0);
    const u32 line = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineClose(app, ts, line); });
}

/* sub_52f969: lineDeallocateCall. */
u32 frontend::watc::lineDeallocateCall(App* app, ThreadState& ts, u32 callHandle)
{
    return tapiStub<1>(app, ts, 0, 0x56d184, 0x56bb68, { callHandle });
}

nfs::u32 frontend::watcLineDeallocateCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52f969", 4, 0, 0);
    const u32 callHandle = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineDeallocateCall(app, ts, callHandle); });
}

/* sub_52fb2e: lineDrop. */
u32 frontend::watc::lineDrop(App* app, ThreadState& ts, u32 callHandle, u32 userUserInfo, u32 size)
{
    return tapiStub<3>(app, ts, 0, 0x56d19c, 0x56bbc8, { callHandle, userUserInfo, size });
}

nfs::u32 frontend::watcLineDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_52fb2e", 12, 0, 0);
    const u32 callHandle = app->getMemory<u32>(ts.sp + 4);
    const u32 userUserInfo = app->getMemory<u32>(ts.sp + 8);
    const u32 size = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineDrop(app, ts, callHandle, userUserInfo, size); });
}

/* sub_530747: lineGetDevCaps. */
u32 frontend::watc::lineGetDevCaps(App* app, ThreadState& ts, u32 lineApp, u32 device, u32 apiVersion, u32 extVersion, u32 caps)
{
    return tapiStub<5>(app, ts, 4, 0x56d238, 0x56bed4, { lineApp, device, apiVersion, extVersion, caps });
}

nfs::u32 frontend::watcLineGetDevCaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_530747", 20, 0, 0);
    const u32 lineApp = app->getMemory<u32>(ts.sp + 4);
    const u32 device = app->getMemory<u32>(ts.sp + 8);
    const u32 apiVersion = app->getMemory<u32>(ts.sp + 12);
    const u32 extVersion = app->getMemory<u32>(ts.sp + 16);
    const u32 caps = app->getMemory<u32>(ts.sp + 20);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineGetDevCaps(app, ts, lineApp, device, apiVersion, extVersion, caps); });
}

/* sub_53083d: lineGetDevConfig. */
u32 frontend::watc::lineGetDevConfig(App* app, ThreadState& ts, u32 device, u32 config, u32 deviceClass)
{
    return tapiStub<3>(app, ts, 0, 0x56d244, 0x56bf04, { device, config, deviceClass });
}

nfs::u32 frontend::watcLineGetDevConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_53083d", 12, 0, 0);
    const u32 device = app->getMemory<u32>(ts.sp + 4);
    const u32 config = app->getMemory<u32>(ts.sp + 8);
    const u32 deviceClass = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineGetDevConfig(app, ts, device, config, deviceClass); });
}

/* sub_530a4e: lineGetID. */
u32 frontend::watc::lineGetID(App* app, ThreadState& ts, u32 line, u32 address, u32 callHandle, u32 select, u32 deviceId, u32 deviceClass)
{
    return tapiStub<6>(app, ts, 4, 0x56d260, 0x56bf7c, { line, address, callHandle, select, deviceId, deviceClass });
}

nfs::u32 frontend::watcLineGetID(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_530a4e", 24, 0, 0);
    const u32 line = app->getMemory<u32>(ts.sp + 4);
    const u32 address = app->getMemory<u32>(ts.sp + 8);
    const u32 callHandle = app->getMemory<u32>(ts.sp + 12);
    const u32 select = app->getMemory<u32>(ts.sp + 16);
    const u32 deviceId = app->getMemory<u32>(ts.sp + 20);
    const u32 deviceClass = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineGetID(app, ts, line, address, callHandle, select, deviceId, deviceClass); });
}

/* sub_530b4d: lineGetLineDevStatus. */
u32 frontend::watc::lineGetLineDevStatus(App* app, ThreadState& ts, u32 line, u32 status)
{
    return tapiStub<2>(app, ts, 0, 0x56d26c, 0x56bfa0, { line, status });
}

nfs::u32 frontend::watcLineGetLineDevStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_530b4d", 8, 0, 0);
    const u32 line = app->getMemory<u32>(ts.sp + 4);
    const u32 status = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineGetLineDevStatus(app, ts, line, status); });
}

/* sub_5310be: lineInitialize. */
u32 frontend::watc::lineInitialize(App* app, ThreadState& ts, u32 lineAppOut, u32 instance, u32 callback, u32 appName, u32 devicesOut)
{
    return tapiStub<5>(app, ts, 4, 0x56d2b8, 0x56c114, { lineAppOut, instance, callback, appName, devicesOut });
}

nfs::u32 frontend::watcLineInitialize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5310be", 20, 0, 0);
    const u32 lineAppOut = app->getMemory<u32>(ts.sp + 4);
    const u32 instance = app->getMemory<u32>(ts.sp + 8);
    const u32 callback = app->getMemory<u32>(ts.sp + 12);
    const u32 appName = app->getMemory<u32>(ts.sp + 16);
    const u32 devicesOut = app->getMemory<u32>(ts.sp + 20);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineInitialize(app, ts, lineAppOut, instance, callback, appName, devicesOut); });
}

/* sub_5311c0: lineMakeCall. */
u32 frontend::watc::lineMakeCall(App* app, ThreadState& ts, u32 line, u32 callOut, u32 destination, u32 country, u32 callParams)
{
    return tapiStub<5>(app, ts, 4, 0x56d2c4, 0x56c14c, { line, callOut, destination, country, callParams });
}

nfs::u32 frontend::watcLineMakeCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5311c0", 20, 0, 0);
    const u32 line = app->getMemory<u32>(ts.sp + 4);
    const u32 callOut = app->getMemory<u32>(ts.sp + 8);
    const u32 destination = app->getMemory<u32>(ts.sp + 12);
    const u32 country = app->getMemory<u32>(ts.sp + 16);
    const u32 callParams = app->getMemory<u32>(ts.sp + 20);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineMakeCall(app, ts, line, callOut, destination, country, callParams); });
}

/* sub_53138f: lineNegotiateAPIVersion. */
u32 frontend::watc::lineNegotiateAPIVersion(App* app, ThreadState& ts, u32 lineApp, u32 device, u32 lowVersion, u32 highVersion, u32 versionOut, u32 extensionId)
{
    return tapiStub<6>(app, ts, 4, 0x56d2dc, 0x56c1b8, { lineApp, device, lowVersion, highVersion, versionOut, extensionId });
}

nfs::u32 frontend::watcLineNegotiateAPIVersion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_53138f", 24, 0, 0);
    const u32 lineApp = app->getMemory<u32>(ts.sp + 4);
    const u32 device = app->getMemory<u32>(ts.sp + 8);
    const u32 lowVersion = app->getMemory<u32>(ts.sp + 12);
    const u32 highVersion = app->getMemory<u32>(ts.sp + 16);
    const u32 versionOut = app->getMemory<u32>(ts.sp + 20);
    const u32 extensionId = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineNegotiateAPIVersion(app, ts, lineApp, device, lowVersion, highVersion, versionOut, extensionId); });
}

/* sub_531439: lineOpen. */
u32 frontend::watc::lineOpen(App* app, ThreadState& ts, u32 lineApp, u32 device, u32 lineOut, u32 apiVersion, u32 extVersion, u32 callbackInstance, u32 privileges, u32 mediaModes, u32 callParams)
{
    return tapiStub<9>(app, ts, 4, 0x56d2e4, 0x56c1e8, { lineApp, device, lineOut, apiVersion, extVersion, callbackInstance, privileges, mediaModes, callParams });
}

nfs::u32 frontend::watcLineOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_531439", 36, 0, 0);
    const u32 lineApp = app->getMemory<u32>(ts.sp + 4);
    const u32 device = app->getMemory<u32>(ts.sp + 8);
    const u32 lineOut = app->getMemory<u32>(ts.sp + 12);
    const u32 apiVersion = app->getMemory<u32>(ts.sp + 16);
    const u32 extVersion = app->getMemory<u32>(ts.sp + 20);
    const u32 callbackInstance = app->getMemory<u32>(ts.sp + 24);
    const u32 privileges = app->getMemory<u32>(ts.sp + 28);
    const u32 mediaModes = app->getMemory<u32>(ts.sp + 32);
    const u32 callParams = app->getMemory<u32>(ts.sp + 36);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineOpen(app, ts, lineApp, device, lineOut, apiVersion, extVersion, callbackInstance, privileges, mediaModes, callParams); });
}

/* sub_531f31: lineSetDevConfig. */
u32 frontend::watc::lineSetDevConfig(App* app, ThreadState& ts, u32 device, u32 config, u32 size, u32 deviceClass)
{
    return tapiStub<4>(app, ts, 4, 0x56d374, 0x56c4a8, { device, config, size, deviceClass });
}

nfs::u32 frontend::watcLineSetDevConfig(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_531f31", 16, 0, 0);
    const u32 device = app->getMemory<u32>(ts.sp + 4);
    const u32 config = app->getMemory<u32>(ts.sp + 8);
    const u32 size = app->getMemory<u32>(ts.sp + 12);
    const u32 deviceClass = app->getMemory<u32>(ts.sp + 16);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineSetDevConfig(app, ts, device, config, size, deviceClass); });
}

/* sub_532162: lineSetStatusMessages. */
u32 frontend::watc::lineSetStatusMessages(App* app, ThreadState& ts, u32 line, u32 lineStates, u32 addressStates)
{
    return tapiStub<3>(app, ts, 0, 0x56d390, 0x56c534, { line, lineStates, addressStates });
}

nfs::u32 frontend::watcLineSetStatusMessages(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_532162", 12, 0, 0);
    const u32 line = app->getMemory<u32>(ts.sp + 4);
    const u32 lineStates = app->getMemory<u32>(ts.sp + 8);
    const u32 addressStates = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineSetStatusMessages(app, ts, line, lineStates, addressStates); });
}

/* sub_5324d2: lineShutdown. */
u32 frontend::watc::lineShutdown(App* app, ThreadState& ts, u32 lineApp)
{
    return tapiStub<1>(app, ts, 0, 0x56d3bc, 0x56c614, { lineApp });
}

nfs::u32 frontend::watcLineShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5324d2", 4, 0, 0);
    const u32 lineApp = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watc::lineShutdown(app, ts, lineApp); });
}

}
