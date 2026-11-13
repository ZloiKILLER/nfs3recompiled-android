#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_guest.h"
#include "native_windows.h"
#include <winapi/kernel32.h>
#include <winapi/user32.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>

/* EA's files (win\fileio.c, win\file.c, win\blockio.c): the handles the game
 * opens its files by -- whole, mapped into memory, or a buffer of its own
 * taken as one -- read, written and sought a block at a time under the lock
 * of the drive each is on, their errors reported; the drives, paths and
 * directories they are on; and the memory pages EA's library takes from
 * Windows (win\touchmem.c).  disassemble_nfs3hp.py binds each address to the
 * function of that name here (EA_FILES_NATIVES), and the generated code calls
 * it by name.  tools/game_checks holds each against its generated code.
 *
 * The handles are a table (its address at 0x5645a0, [0x56459c] of them, 0x40
 * unless the game asks for more) of 0x20 bytes each: +0 in use, +1 its
 * drive (1 for A:, the lock of 0x9ef760 by it), +2 a read under way, +4 the
 * Windows file, +8 how it was opened (1 read, 2 write, 4 and 8 the others'
 * sharing, 0x10 and 0x20 how it is created, 0x100 to 0x400 Windows' flags for
 * it, 0x800 mapped), +0xc the block it is read and written by, +0x10 its
 * mapping, +0x14 where it is mapped (or the buffer taken as a file), +0x18
 * where it is, +0x1c how long.  A handle given out is the slot's index, its
 * bits flipped.  The table is taken under its own lock (0x564598); a handle
 * is read, written and sought under its drive's.  A file not found may be in
 * the game's archive ([0x564504], its directory read into [0x564584], looked
 * up by [0x564588]): the archive opened instead, at the file's place in it.
 *
 * Its errors go to fileReport (0x501050), which prints a report as its
 * level and the game's way of reporting ([0x56789c]) say; a file not opened
 * while [0x56440c] is set ends the game (Error).
 *
 * Each is a typed function under a name of its own, with an adapter under
 * the game's name for the callers still on registers and the dispatch: it
 * leaves eax, and the registers the game's code keeps, as that code leaves
 * them.  The registers a function is handed its arguments in are its own
 * (Watcom's convention: the callers count on none of them after), and the
 * checks compare them only where the code keeps them (ea_files.inc).  Other
 * files' typed functions -- the main thread asked, a surface locked, the
 * machine's memory, a copy, a sleep, a lock made and freed -- are called as
 * C++ (guest::direct, their headers), a lock made with eax as the code has it
 * at the call, which the checks' stand-in for it now and then leaves.  Every
 * other function of the game's is called by address, its arguments in
 * registers and on the guest's stack as the code hands them (call, error);
 * Windows' functions through their imports' slots (windows) -- openHandle
 * (0x4e28d0) and virtualAllocPages (0x501110) too where this file's own code
 * calls them by address, as the checks stand in for them for other files'
 * natives.  A frame whose address a callee is handed -- a buffer, a count
 * read or written, a disk's numbers -- or under which Error or fileReport is
 * called (the checks see their va_list's and buffer's addresses) stays on the
 * guest's stack at the original's depth (GuestFrame, asCalled). */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

using App = win32::WinApplication;
using CPU = x86::CPU;
// What a function gives back in eax; nullopt when the game stopped in it.
using Answer = std::optional<reg32>;

// Where an Error is from: the file, the function, the line.
constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// A file not opened ends the game, when set.
constexpr reg32 kErrorsFatal = 0x56440c;
// Whether the functions called on the main thread are checked for a surface
// locked.
constexpr reg32 kSurfaceCheck = 0x564364;
// The archive: its name, its directory read in, how much of it, the function
// that looks a file up in it, the file's length it found.
constexpr reg32 kArchiveName = 0x564504, kArchiveDirectory = 0x564584, kArchiveLookup = 0x564588;
constexpr reg32 kArchiveFoundLength = 0x56458c, kArchiveDirectoryBytes = 0x564590;
// The reports' level, the table's lock, the handles, the drives' locks.
constexpr reg32 kReportLevel = 0x564594, kHandlesLock = 0x564598, kHandleCount = 0x56459c, kHandles = 0x5645a0;
constexpr reg32 kDriveLocks = 0x9ef760, kDriveLocksBytes = 0x80;
// The reports' way out: none, OutputDebugString, stdout, the game's log.
constexpr reg32 kReportOutput = 0x56789c, kStdout = 0x566f72;
constexpr reg32 kToDebugger = 1, kToStdout = 2, kToLog = 3;
// Windows' page, and how much memory there is to touch.
constexpr reg32 kPageSize = 0x5678a0, kPhysicalMemory = 0x5678a4;
// The handles' table: a slot's size, the default count and block.
constexpr reg32 kSlotShift = 5, kSlotBytes = 0x20, kDefaultHandles = 0x40, kDefaultBlock = 0x4000;
// A slot's fields.
constexpr reg32 kInUse = 0, kDrive = 1, kReading = 2, kFile = 4, kHow = 8, kBlock = 0xc, kMapping = 0x10;
constexpr reg32 kMemory = 0x14, kPlace = 0x18, kLength = 0x1c;
// How a handle is opened: read (and the others may read), to write made
// anew, mapped; the mapped bit in +9.  A sector when the disk tells none.
constexpr reg32 kOpenFlags = 0x205, kWriteFlags = 0x232, kMapFlags = 0x805, kMapped = 0x800;
constexpr reg8 kMappedHigh = kMapped >> 8;
constexpr reg32 kNoDiskSector = 0x800;
// No file, no mapping.
constexpr reg32 kNone = 0xffffffff;
// The game's strings: win\fileio.c's, win\file.c's, win\touchmem.c's, win\blockio.c's.
constexpr reg32 kFileioC = 0x54a148, kOpenHandleName = 0x54a158, kOpenFailed = 0x54a164, kInfoFailed = 0x54a188;
constexpr reg32 kOpenHandleWName = 0x54a1ac, kOpenMapHandleName = 0x54a1b8, kOpenHandleAName = 0x54a1c8;
constexpr reg32 kMapInfoFailed = 0x54a1d4, kMapOpenFailed = 0x54a1fc;
constexpr reg32 kLibCloseHandleName = 0x54a228, kCloseHandleName = 0x54a238, kReadHandleName = 0x54a244;
constexpr reg32 kWriteHandleName = 0x54a250, kSeekHandleName = 0x54a25c, kVolumeFailed = 0x54a268;
constexpr reg32 kNoPath = 0x54a294, kFileC = 0x54a298, kSetDirectoryName = 0x54a2a4, kSetDirectoryFailed = 0x54a2b4;
constexpr reg32 kGetDirectoryName = 0x54a2d4, kGetDirectoryFailed = 0x54a2e4, kCreateFileFailed = 0x54a320;
constexpr reg32 kDriveRoot = 0x54a340, kDiskFreeFailed = 0x54a344, kReadFileFailed = 0x54a36c;
constexpr reg32 kWriteFileFailed = 0x54a38c, kSetFilePointerFailed = 0x54a3ac;
constexpr reg32 kSurfaceLocked = 0x54e42c, kTouchmemC = 0x54e454, kTouchMemPagesName = 0x54e464;
constexpr reg32 kTouchMemFirst = 0x54e474, kBlockioC = 0x54e49c, kOpenBlockHandleName = 0x54e4ac;
constexpr reg32 kBlockOpenFailed = 0x54e4bc, kBlockInfoFailed = 0x54e4e4, kBlockHandleFileName = 0x54e50c;
constexpr reg32 kBlockFileUnsupported = 0x54e51c, kReadBlockHandleName = 0x54e548, kBlockReadFailed = 0x54e558;
constexpr reg32 kSeekBlockHandleName = 0x54e578, kBlockSeekFailed = 0x54e588;
// Windows' functions, by their imports' slots.
constexpr reg32 kCloseHandle = 0x534488, kCreateFileA = 0x534498, kCreateFileMappingA = 0x53449c;
constexpr reg32 kDeleteFileA = 0x5344b0, kEnterCriticalSection = 0x5344b8, kFindClose = 0x5344c8;
constexpr reg32 kFindFirstFileA = 0x5344cc, kGetCurrentDirectoryA = 0x534500, kGetDiskFreeSpaceA = 0x534514;
constexpr reg32 kGetFileSize = 0x534524, kGetFullPathNameA = 0x534530, kGetLastError = 0x534534;
constexpr reg32 kGetLogicalDriveStringsA = 0x53453c, kGetLogicalDrives = 0x534540, kGetSystemInfo = 0x534560;
constexpr reg32 kGetVolumeInformationA = 0x534578, kLeaveCriticalSection = 0x534588, kMapViewOfFile = 0x534590;
constexpr reg32 kOutputDebugStringA = 0x53459c, kReadFile = 0x5345b0, kSetCurrentDirectoryA = 0x5345d4;
constexpr reg32 kSetFilePointer = 0x5345f0, kSetLastError = 0x5345f4, kUnmapViewOfFile = 0x534620;
constexpr reg32 kVirtualAlloc = 0x534624, kVirtualFree = 0x534628, kWriteFile = 0x534640, kCharUpperA = 0x5346f8;
constexpr reg32 kIoPending = 0x3e5;
// Windows' constants handed.
constexpr reg32 kGenericRead = 0x80000000, kGenericWrite = 0x40000000, kShareRead = 1, kShareWrite = 2;
constexpr reg32 kCreateNew = 1, kCreateAlways = 2, kOpenExisting = 3, kTruncateExisting = 5;
constexpr reg32 kAttributeNormal = 0x80, kNoBuffering = 0x20000000, kSequentialScan = 0x8000000;
constexpr reg32 kWriteThrough = 0x80000000, kMapWrite = 2, kMapRead = 4;
constexpr reg32 kCommitReadWrite = 0x8000004, kCommitReadOnly = 0x8000002, kFileCurrent = 1;
constexpr reg32 kMemCommitReserve = 0x3000, kPageReadWrite = 4, kMemRelease = 0x8000;
constexpr reg32 kCaseSensitive = 2, kUnicodeOnDisk = 4, kFoundName = 0x2c, kNotFound = 0xffffffff;
// The game's functions, by address.
constexpr reg32 kError = 0x401010, kVsprintf = 0x4df674, kMessage = 0x4df6c0;
constexpr reg32 kFillZero = 0x4e070c, kTrace = 0x4e08f0, kStricmp = 0x4e9ec0, kFflush = 0x4f7d70;
constexpr reg32 kOpenHandle = 0x4e28d0, kVirtualAllocPages = 0x501110;
// The longest path copied as it is.
constexpr reg32 kMaxPath = 0x104;

/* The frame of a core whose callees see where it is -- handed its locals' addresses, or Error
 * and fileReport, whose va_list's and buffer's addresses the checks see -- kept on the guest's
 * stack at the original's depth: `pushed` bytes of registers below the return address's slot,
 * then `bytes` of locals.  frame - n is n bytes below the registers pushed. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    reg32 operator-(reg32 offset) const { return base_ - offset; }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

/* A core of this file called as the game's code calls it: `handed` bytes of stack arguments and
 * the return address's slot below esp, its own frame below them. */
template <typename Core>
auto asCalled(CPU& cpu, Core core, reg32 handed = 0)
{
    cpu.esp -= handed + 4;
    const auto result = core();
    cpu.esp += handed + 4;
    return result;
}

/* A call by address: eax, edx, ebx and ecx from `registers`, as many as given, the rest left as
 * they are; `words` pushed (cdecl or stdcall: esp back where it was after).  What it leaves in eax
 * back; nullopt when the game stopped.  Where that is used, eax is given as the code has it before
 * the call: the checks' stand-in of a function that answers nothing of its own (a lock made,
 * GetLogicalDrives, OutputDebugString) now and then leaves it as it was. */
Answer call(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> registers,
            std::initializer_list<reg32> words = {})
{
    reg32* const slots[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    std::size_t i = 0;
    for (const reg32 value : registers)
        *slots[i++] = value;
    if (!guest::call(app, cpu, target, words))
        return std::nullopt;
    return cpu.eax;
}

/* A Windows function, its words handed: the port's own, called as C++ (native_windows.h); in the
 * check through its import's slot. */
template <reg32 Slot, auto Function, typename... Words>
Answer windows(App* app, CPU& cpu, Words... words)
{
    return guest::windows<Slot, Function>(app, cpu, words...);
}

/* Where an Error is from, as the code sets it. */
void where(App* app, reg32 file, reg32 function, reg32 line)
{
    put(app, kWhereFile, file);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
}

/* Error (0x401010, cdecl) from `function` of `file` at `line` -- where it is from set first --
 * its format and words handed.  What it leaves in eax; nullopt when the game stopped. */
Answer error(App* app, CPU& cpu, reg32 file, reg32 function, reg32 line, std::initializer_list<reg32> words)
{
    where(app, file, function, line);
    return call(app, cpu, kError, {}, words);
}

/* The lock of drive `drive` (1 for A:). */
reg32 driveLock(App* app, reg32 drive)
{
    return at(app, kDriveLocks + drive * 4);
}

/* The table's slot of index `index`, as the table is now. */
reg32 slotAt(App* app, reg32 index)
{
    return at(app, kHandles) + (index << kSlotShift);
}

/* The index of handle `handle` (its bits flipped) if the table is made and
 * it is within it -- in use or not. */
std::optional<reg32> indexOf(App* app, reg32 handle)
{
    const reg32 index = ~handle;
    if (at(app, kHandles) == 0 || sreg32(index) < 0 || sreg32(index) >= sreg32(at(app, kHandleCount)))
        return std::nullopt;
    return index;
}

/* The index of handle `handle` if it is open. */
std::optional<reg32> openIndexOf(App* app, reg32 handle)
{
    const std::optional<reg32> index = indexOf(app, handle);
    if (!index || byte(app, slotAt(app, *index) + kInUse) == 0)
        return std::nullopt;
    return index;
}

/* The length of the string at `text`, as strlen. */
reg32 lengthOf(App* app, reg32 text)
{
    reg32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    return length;
}

/* The string at `from` copied to `to`, its ending too. */
void copyText(App* app, reg32 to, reg32 from)
{
    for (reg32 i = 0;; ++i)
    {
        const reg8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            return;
    }
}

/* How much of the path at `path` is copied as it is: its ending too, no
 * more than kMaxPath. */
reg32 copiedLength(App* app, reg32 path)
{
    const reg32 length = lengthOf(app, path) + 1;
    return length < kMaxPath ? length : kMaxPath;
}

/* The registers the game's code keeps for its caller -- all but eax and
 * esp -- put back when the adapter is done. */
class Kept
{
public:
    explicit Kept(CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp)
    {
    }
    ~Kept()
    {
        cpu_.ebx = ebx_;
        cpu_.ecx = ecx_;
        cpu_.edx = edx_;
        cpu_.esi = esi_;
        cpu_.edi = edi_;
        cpu_.ebp = ebp_;
    }
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;

private:
    CPU& cpu_;
    const reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* An adapter's way out: eax the core's result, the return address popped
 * and `handed` bytes of stack arguments; nothing when the game stopped. */
void returns(CPU& cpu, const Answer& result, reg32 handed = 0)
{
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4 + handed;
}

/* fileReport (0x501050, cdecl): EA's report of `level` -- none if it is
 * above the threshold at `threshold` (0: always) -- `format` and its words at
 * `arguments` formatted into the frame's 1K (vsprintf, its va_list there)
 * and sent the way [0x56789c] says: OutputDebugString, printed (Message as
 * its format, stdout flushed), or the game's log (trace); anything else
 * nowhere.  What the way leaves in eax back: the threshold's address when
 * the level is above it, the way's number when it goes nowhere. */
Answer report(App* app, CPU& cpu, reg32 threshold, reg32 level, reg32 format, reg32 arguments)
{
    const GuestFrame frame(cpu, 0xc, 0x404);
    const reg32 text = frame - 0x404, list = frame - 4;
    if (threshold != 0 && sreg32(level) > sreg32(at(app, threshold)))
        return threshold;
    put(app, list, arguments);
    if (!call(app, cpu, kVsprintf, {text, format, list, 0}))
        return std::nullopt;
    const reg32 way = at(app, kReportOutput);
    if (way == kToDebugger)
        return call(app, cpu, at(app, kOutputDebugStringA), {text}, {text});
    if (way == kToStdout)
    {
        if (!call(app, cpu, kMessage, {text}, {text}))
            return std::nullopt;
        return call(app, cpu, kFflush, {kStdout});
    }
    if (way == kToLog)
        return call(app, cpu, kTrace, {text}, {text});
    return way;
}

/* A Windows error `code` reported at level 1 with `format`: fileReport
 * called as the code calls it, its four words pushed below esp. */
Answer reportError(App* app, CPU& cpu, reg32 format, reg32 code)
{
    const reg32 top = cpu.esp, words = top - 0x10;
    put(app, words, kReportLevel);
    put(app, words + 4, 1);
    put(app, words + 8, format);
    put(app, words + 0xc, code);
    cpu.esp = words;
    const Answer result = asCalled(cpu, [&] { return report(app, cpu, kReportLevel, 1, format, words + 0xc); });
    cpu.esp = top;
    return result;
}

/* GetLastError's error reported with `format`. */
Answer reportLastError(App* app, CPU& cpu, reg32 format)
{
    const Answer code = windows<kGetLastError, &win32::kernel32::GetLastError>(app, cpu);
    if (!code)
        return std::nullopt;
    return reportError(app, cpu, format, *code & 0xffff);
}

}  // namespace

/* checkSurfaceUnlocked (0x501010): 0 -- after an Error naming function
 * `name`, files not to be touched then -- on the main thread with the check
 * on ([0x564364]) and a surface locked; else 1. */
Answer surfaceUnlocked(App* app, CPU& cpu, reg32 name)
{
    const GuestFrame frame(cpu, 8, 0);
    const std::optional<bool> onMain = guest::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u);
    if (!onMain)
        return std::nullopt;
    if (!*onMain || at(app, kSurfaceCheck) == 0)
        return 1;
    const bool locked = guest::direct<0x4f9070, ea::surfaceIsLocked>(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    if (!locked)
        return 1;
    if (!call(app, cpu, kError, {reg32(locked), 0}, {kSurfaceLocked, name}))
        return std::nullopt;
    return 0;
}

namespace
{

/* closeHandleSlot (0x4e3fe0): handle `handle` closed under its drive's
 * lock -- its mapping and view (asked whatever +0x10 holds but -1), its file
 * (but -1) -- and its slot freed.  1, or 0 if it was not open. */
Answer closeSlot(App* app, CPU& cpu, reg32 handle)
{
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, drive)) || !windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    if (at(app, slotAt(app, *index) + kMapping) != kNone)
    {
        if (!windows<kUnmapViewOfFile, &win32::kernel32::UnmapViewOfFile>(app, cpu, at(app, slotAt(app, *index) + kMemory))
            || !windows<kCloseHandle, &win32::kernel32::CloseHandle>(app, cpu, at(app, slotAt(app, *index) + kMapping)))
            return std::nullopt;
    }
    const reg32 file = at(app, slotAt(app, *index) + kFile);
    if (file != kNone && !windows<kCloseHandle, &win32::kernel32::CloseHandle>(app, cpu, file))
        return std::nullopt;
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, drive)))
        return std::nullopt;
    putByte(app, slotAt(app, *index) + kInUse, 0);
    return 1;
}

/* lockHandleDrive and unlockHandleDrive: handle `handle`'s drive's lock
 * entered or left (EnterCriticalSection's or LeaveCriticalSection's slot).
 * 1, or 0 if it is not open. */
Answer lockDriveOf(App* app, CPU& cpu, reg32 handle, reg32 slot)
{
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 lock = driveLock(app, byte(app, slotAt(app, *index) + kDrive));
    const Answer done = slot == kEnterCriticalSection
                            ? windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, lock)
                            : windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, lock);
    if (!done)
        return std::nullopt;
    return 1;
}

/* handleMemory (0x4e41f0): where handle `handle` is in memory (mapped, or a
 * buffer taken as a file); 0 if it is not, or not open. */
reg32 memoryOf(App* app, reg32 handle)
{
    const std::optional<reg32> index = indexOf(app, handle);
    if (!index)
        return 0;
    const reg32 slot = slotAt(app, *index);
    return byte(app, slot + kInUse) == 0 ? 0 : at(app, slot + kMemory);
}

}  // namespace

/* virtualAllocPages (0x501110): Windows' pages for the bytes at `bytes` --
 * rounded up to whole pages there (the page's size asked of Windows the
 * first time, into the frame's SYSTEM_INFO) -- committed, read and written;
 * their address, or 0. */
Answer allocatePages(App* app, CPU& cpu, reg32 bytes)
{
    const GuestFrame frame(cpu, 0x14, 0x24);
    const reg32 info = frame - 0x24;
    if (at(app, kPageSize) == 0)
    {
        if (!windows<kGetSystemInfo, &win32::kernel32::GetSystemInfo>(app, cpu, info))
            return std::nullopt;
        put(app, kPageSize, at(app, info + 4));
    }
    const reg32 page = at(app, kPageSize);
    // cdq; idiv: worked in 64 bits, as the generated code does.
    const std::int64_t dividend = sreg32(at(app, bytes) + page - 1);
    const reg32 rounded = reg32(dividend / std::int64_t(sreg32(page))) * page;
    put(app, bytes, rounded);
    return windows<kVirtualAlloc, &win32::kernel32::VirtualAlloc>(app, cpu, 0, rounded, kMemCommitReserve, kPageReadWrite);
}

namespace
{

/* virtualFreePages (0x501180): Windows' pages at `pages` given back
 * (MEM_RELEASE); VirtualFree's answer. */
Answer freePages(App* app, CPU& cpu, reg32 pages)
{
    return windows<kVirtualFree, &win32::kernel32::VirtualFree>(app, cpu, pages, 0, kMemRelease);
}

/* What touchMemoryPages leaves: in eax the pages' count (or what the last
 * call left, or the start, when there is not a page to go over); in ebx the
 * page past the last gone over (or the memory there is); in edx the last
 * count down (or the remainder, or the bytes held to the memory). */
struct Touched
{
    reg32 result;
    reg32 end;
    reg32 left;
};

/* touchMemoryPages (0x5011a0, EA's touchmempages): the pages of `bytes`
 * from `start` -- no more than the memory there is ([0x5678a4], asked of
 * the game the first time) -- gone over to have them in memory; its loops
 * count, but touch nothing.  An Error if the page's size is not known yet
 * (initmemman not called). */
std::optional<Touched> touchPages(App* app, CPU& cpu, reg32 start, reg32 bytes)
{
    const GuestFrame frame(cpu, 0x10, 4);
    reg32 result = start;
    if (at(app, kPageSize) == 0)
    {
        const Answer stopped = error(app, cpu, kTouchmemC, kTouchMemPagesName, 0x32, {kTouchMemFirst});
        if (!stopped)
            return std::nullopt;
        result = *stopped;
    }
    if (at(app, kPhysicalMemory) == 0)
    {
        const Answer memory = guest::direct<0x5221f0, ea::physicalBytes>(app, cpu);
        if (!memory)
            return std::nullopt;
        put(app, kPhysicalMemory, *memory);
        result = *memory;
    }
    reg32 end = at(app, kPhysicalMemory);
    const reg32 page = at(app, kPageSize);
    if (sreg32(bytes) > sreg32(end))
        bytes = end;
    if (sreg32(bytes) >= sreg32(page))
    {
        end = (start + page - 1) & ~(page - 1);
        // cdq; idiv: worked in 64 bits, as the generated code does.
        const std::int64_t dividend = sreg32(bytes - (end - start) + (page - 1));
        const reg32 pages = reg32(dividend / std::int64_t(sreg32(page)));
        bytes = reg32(dividend % std::int64_t(sreg32(page)));
        result = pages;
        for (reg32 i = 0; sreg32(i) < sreg32(pages); ++i)
        {
            bytes = end;
            if (end >= start)
            {
                reg32 count = 0;
                do
                {
                    ++count;
                    bytes -= page;
                } while (sreg32(count) < 0x20 && bytes >= start);
            }
            end += page;
        }
    }
    put(app, kPageSize, page);
    return Touched{result, end, bytes};
}

/* isSlash (0x4e2eb0): whether `c` is a slash, \ or /. */
bool isSlashChar(reg8 c)
{
    return c == '\\' || c == '/';
}

/* From `place` to the first slash, or to the string's last character. */
reg32 toSlash(App* app, reg32 place)
{
    for (;;)
    {
        if (isSlashChar(byte(app, place)))
            return place;
        const reg8 next = byte(app, place + 1);
        ++place;
        if (next == 0)
            return place;
    }
}

/* volumeRoot (0x4e2ed0): the root of `path` to `root`: the path copied (at
 * most 0x104 bytes, memmove) and cut after the share of a \\server\share,
 * or else after its first slash -- or its last character, with none.  Where
 * the root ends: its ending put after it. */
std::optional<reg32> rootOf(App* app, CPU& cpu, reg32 path, reg32 root)
{
    putByte(app, root, 0);
    if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, root, path, copiedLength(app, path)))
        return std::nullopt;
    // A \\ looked for, a byte on at a time.
    reg32 place = root;
    if (byte(app, root) != 0)
    {
        for (;;)
        {
            if (isSlashChar(byte(app, place)) && isSlashChar(byte(app, place + 1)))
                break;
            const reg8 next = byte(app, place + 1);
            ++place;
            if (next == 0)
                break;
        }
    }
    if (byte(app, place) == 0)
    {
        // None: cut after the first slash.
        place = root;
        if (byte(app, root) != 0)
            place = toSlash(app, root);
        putByte(app, place + 1, 0);
        return place;
    }
    // The server's name passed over, then the share's.
    place += 2;
    if (byte(app, place) != 0)
        place = toSlash(app, place);
    if (byte(app, place) != 0)
        ++place;
    while (byte(app, place) != 0 && !isSlashChar(byte(app, place)))
        ++place;
    putByte(app, place + 1, 0);
    return place;
}

/* fullPathName (0x4e2fd0): the full path of `path` to `out` (`room` bytes
 * there): 1 if it was found -- the path in capitals where its volume does
 * not keep their case, and the file's name as the directory has it where the
 * volume keeps no Unicode names -- else 0, the path copied as it is (at most
 * 0x104 bytes); 0 too, reported, if its volume tells nothing.  The frame:
 * the file found, the volume's root, its flags, its longest name, where the
 * file's name is in `out`. */
Answer fullPath(App* app, CPU& cpu, reg32 path, reg32 out, reg32 room)
{
    const GuestFrame frame(cpu, 0x10, 0x250);
    const reg32 found = frame - 0x250, root = found + 0x140, flags = found + 0x244;
    const reg32 longest = found + 0x248, namePart = found + 0x24c;
    const Answer length = windows<kGetFullPathNameA, &win32::kernel32::GetFullPathNameA>(app, cpu, path, room, out, namePart);
    if (!length)
        return std::nullopt;
    if (*length == 0)
    {
        if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, out, path, copiedLength(app, path)))
            return std::nullopt;
        return 0;
    }
    if (!rootOf(app, cpu, out, root))
        return std::nullopt;
    const Answer told = windows<kGetVolumeInformationA, &win32::kernel32::GetVolumeInformationA>(app, cpu, root, 0, 0, 0, longest, flags, 0, 0);
    if (!told)
        return std::nullopt;
    if (*told == 0)
    {
        if (!reportLastError(app, cpu, kVolumeFailed))
            return std::nullopt;
        return 0;
    }
    if ((byte(app, flags) & kCaseSensitive) == 0 && !windows<kCharUpperA, &win32::user32::CharUpperA>(app, cpu, out))
        return std::nullopt;
    if ((byte(app, flags) & kUnicodeOnDisk) == 0)
    {
        const Answer find = windows<kFindFirstFileA, &win32::kernel32::FindFirstFileA>(app, cpu, path, found);
        if (!find)
            return std::nullopt;
        if (*find != kNotFound)
        {
            if (!windows<kFindClose, &win32::kernel32::FindClose>(app, cpu, *find))
                return std::nullopt;
            copyText(app, at(app, namePart), found + kFoundName);
        }
    }
    return 1;
}

/* driveOfPath (0x4e3120): the drive of `path`: its full path (or the
 * directory's, not found) cut to its root at `root`, looked for among
 * Windows' drives; the drive's letter, or that of the first after the last
 * there is, or 0 past Z.  Its buffers Windows' pages (virtualAllocPages, by
 * address, the frame's word their size). */
Answer driveOf(App* app, CPU& cpu, reg32 path, reg32 root)
{
    const GuestFrame frame(cpu, 0x14, 0xc);
    const reg32 bytes = frame - 0xc;
    put(app, bytes, 0x1000);
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    Answer buffer = call(app, cpu, kVirtualAllocPages, {bytes});
    if (!buffer)
        return std::nullopt;
    const Answer full = asCalled(cpu, [&] { return fullPath(app, cpu, path, *buffer, 0xfff); });
    if (!full)
        return std::nullopt;
    if (*full == 0 && !asCalled(cpu, [&] { return currentDirectory(app, cpu, *buffer); }))
        return std::nullopt;
    if (!rootOf(app, cpu, *buffer, root))
        return std::nullopt;
    const Answer needed = windows<kGetLogicalDriveStringsA, &win32::kernel32::GetLogicalDriveStringsA>(app, cpu, 0, *buffer);
    if (!needed)
        return std::nullopt;
    if (sreg32(*needed) > 0xfff)
    {
        if (!freePages(app, cpu, *buffer))
            return std::nullopt;
        put(app, bytes, *needed + 1);
        buffer = call(app, cpu, kVirtualAllocPages, {bytes});
        if (!buffer)
            return std::nullopt;
    }
    if (!windows<kGetLogicalDriveStringsA, &win32::kernel32::GetLogicalDriveStringsA>(app, cpu, *needed, *buffer))
        return std::nullopt;
    const Answer drives = call(app, cpu, at(app, kGetLogicalDrives), {0});
    if (!drives)
        return std::nullopt;
    reg32 index = 0;
    reg32 name = *buffer;
    if (byte(app, name) != 0)
    {
        for (;;)
        {
            if ((*drives & (1u << (index & 0x1f))) != 0)
            {
                const Answer differs = call(app, cpu, kStricmp, {name, root});
                if (!differs)
                    return std::nullopt;
                if (*differs == 0)
                    break;
                name += lengthOf(app, name) + 1;
            }
            ++index;
            if (sreg32(index) >= 0x20 || byte(app, name) == 0)
                break;
        }
    }
    if (!freePages(app, cpu, *buffer))
        return std::nullopt;
    return sreg32(index) >= 0x20 ? 0 : index + 'A';
}

/* driveLetter (0x4e3250): the drive letter of `path` (none: the
 * directory's, "."), from its full path -- in capitals, in the frame -- if
 * it starts X:; 0 if not, or if the path has a |. */
Answer letterOf(App* app, CPU& cpu, reg32 path)
{
    for (reg32 place = path;; ++place)
    {
        const reg8 c = byte(app, place);
        if (c == '|')
            return 0;
        if (c == 0)
            break;
    }
    const GuestFrame frame(cpu, 0xc, 0x10c);
    const reg32 full = frame - 0x10c, namePart = full + 0x108;
    const Answer length = windows<kGetFullPathNameA, &win32::kernel32::GetFullPathNameA>(app, cpu, path != 0 ? path : kNoPath, 0x105, full, namePart);
    if (!length || !windows<kCharUpperA, &win32::user32::CharUpperA>(app, cpu, full))
        return std::nullopt;
    if (sreg32(*length) <= 1 || byte(app, full + 1) != ':')
        return 0;
    return byte(app, full);
}

/* diskFreeBytes (0x4e32e0): the bytes free on the volume whose root is at
 * `root` -- the clusters free, their sectors, the sectors' bytes, told into
 * the frame -- or 0. */
Answer bytesFree(App* app, CPU& cpu, reg32 root)
{
    const GuestFrame frame(cpu, 0xc, 0x10);
    const reg32 sectors = frame - 0x10, sectorBytes = sectors + 4, free = sectors + 8, clusters = sectors + 0xc;
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    const Answer told = windows<kGetDiskFreeSpaceA, &win32::kernel32::GetDiskFreeSpaceA>(app, cpu, root, sectors, sectorBytes, free, clusters);
    if (!told || *told == 0)
        return told;
    return at(app, free) * at(app, sectors) * at(app, sectorBytes);
}

/* driveNumber (0x4e3330): the drive of `path` by its number, 1 for A:
 * (driveLetter); 0 for none. */
Answer numberOf(App* app, CPU& cpu, reg32 path)
{
    const Answer letter = asCalled(cpu, [&] { return letterOf(app, cpu, path); });
    if (!letter || *letter == 0)
        return letter;
    return *letter - 0x40;
}

/* setDirectory (0x4e3340, EA's setdirectory): the current directory set to
 * `path`: SetCurrentDirectory's answer, or an Error's if it cannot be. */
Answer changeDirectory(App* app, CPU& cpu, reg32 path)
{
    const GuestFrame frame(cpu, 0xc, 0);
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    const Answer changed = windows<kSetCurrentDirectoryA, &win32::kernel32::SetCurrentDirectoryA>(app, cpu, path);
    if (!changed || *changed != 0)
        return changed;
    const GuestFrame saved(cpu, 4, 0);  // esi pushed for the Error
    return error(app, cpu, kFileC, kSetDirectoryName, 0xe0, {kSetDirectoryFailed, path});
}

}  // namespace

/* getDirectory (0x4e33a0, EA's getdirectory): the current directory to
 * `buffer` (0x1ff bytes there): its length, or an Error's answer if it
 * cannot be had or is longer. */
Answer currentDirectory(App* app, CPU& cpu, reg32 buffer)
{
    const GuestFrame frame(cpu, 0xc, 0);
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    const Answer length = windows<kGetCurrentDirectoryA, &win32::kernel32::GetCurrentDirectoryA>(app, cpu, 0x1ff, buffer);
    if (!length || (*length != 0 && *length <= 0x1ff))
        return length;
    const GuestFrame saved(cpu, 4, 0);  // esi pushed for the Error
    return error(app, cpu, kFileC, kGetDirectoryName, 0xe9, {kGetDirectoryFailed, *length});
}

namespace
{

/* closeAllHandles (0x4e34c0): the handles' table gone, if there is one:
 * every handle open closed under its lock, its pages given back, the
 * drives' locks and its own freed.  False when the game stopped. */
bool closeTable(App* app, CPU& cpu)
{
    if (at(app, kHandles) == 0)
        return true;
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, at(app, kHandlesLock)))
        return false;
    if (sreg32(at(app, kHandleCount)) > 0)
    {
        reg32 index = 0;
        do
        {
            if (byte(app, slotAt(app, index) + kInUse) != 0 && !closeSlot(app, cpu, ~index))
                return false;
            ++index;
        } while (sreg32(index) < sreg32(at(app, kHandleCount)));
    }
    if (!freePages(app, cpu, at(app, kHandles)))
        return false;
    put(app, kHandles, 0);
    for (reg32 offset = 0; offset != kDriveLocksBytes; offset += 4)
    {
        const reg32 lock = at(app, kDriveLocks + offset);
        if (lock != 0)
        {
            if (!guest::direct<0x4eb360, ea::lockFreed>(app, cpu, lock))
                return false;
            put(app, kDriveLocks + offset, 0);
        }
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, at(app, kHandlesLock))
        || !guest::direct<0x4eb360, ea::lockFreed>(app, cpu, at(app, kHandlesLock)))
        return false;
    put(app, kHandlesLock, 0);
    return true;
}

/* initHandles (0x4e3430): the handles' table made anew for `count`
 * handles (0: 0x40): its lock made the first time, the table there closed
 * (closeAllHandles -- which frees the lock, left 0 for the rest), Windows'
 * pages taken for it under the lock (virtualAllocPages, by address, the
 * frame's word their size), it and the drives' locks cleared.  What
 * LeaveCriticalSection leaves in eax back. */
Answer makeTable(App* app, CPU& cpu, reg32 count)
{
    const GuestFrame frame(cpu, 0x14, 4);
    const reg32 bytes = frame - 4;
    if (at(app, kHandlesLock) == 0)
    {
        cpu.eax = count;  // as the code has it at the call
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
        if (!lock)
            return std::nullopt;
        put(app, kHandlesLock, *lock);
    }
    if (at(app, kHandles) != 0 && !asCalled(cpu, [&] { return closeTable(app, cpu); }))
        return std::nullopt;
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, at(app, kHandlesLock)))
        return std::nullopt;
    if (count == 0)
        count = kDefaultHandles;
    put(app, bytes, count << kSlotShift);
    const Answer table = call(app, cpu, kVirtualAllocPages, {bytes});
    if (!table)
        return std::nullopt;
    put(app, kHandles, *table);
    put(app, kHandleCount, count);
    if (*table != 0)
    {
        if (!call(app, cpu, kFillZero, {*table, at(app, bytes)})
            || !call(app, cpu, kFillZero, {kDriveLocks, kDriveLocksBytes}))
            return std::nullopt;
    }
    return windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, at(app, kHandlesLock));
}

/* takeHandleSlot (0x4e3570): a slot of the table taken, under its lock (the
 * table made first if there is none): the first not in use, cleared, in
 * use; its index -- the count of handles, none free. */
Answer takeSlot(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0);
    if (at(app, kHandles) == 0 && !asCalled(cpu, [&] { return makeTable(app, cpu, 0); }))
        return std::nullopt;
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, at(app, kHandlesLock)))
        return std::nullopt;
    const reg32 count = at(app, kHandleCount);
    reg32 index = 0;
    if (sreg32(count) > 0)
    {
        const reg32 table = at(app, kHandles), end = count << kSlotShift;
        for (reg32 offset = 0; byte(app, table + offset + kInUse) != 0;)
        {
            offset += kSlotBytes;
            ++index;
            if (sreg32(offset) >= sreg32(end))
                break;
        }
    }
    if (sreg32(index) < sreg32(at(app, kHandleCount)))
    {
        if (!call(app, cpu, kFillZero, {slotAt(app, index), kSlotBytes}))
            return std::nullopt;
        putByte(app, slotAt(app, index) + kInUse, 1);
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, at(app, kHandlesLock)))
        return std::nullopt;
    return index;
}

/* deleteFile (0x4e3600): the file named `name` deleted; DeleteFile's
 * answer. */
Answer removeFile(App* app, CPU& cpu, reg32 name)
{
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    return windows<kDeleteFileA, &win32::kernel32::DeleteFileA>(app, cpu, name);
}

/* openFileHandle (0x4e3620): the file named `name` opened as `flags` say,
 * in a slot of the table (takeHandleSlot), its handle to [handleOut] (0
 * till then): under its drive's lock -- made the first time -- the file made
 * or opened, its block `block` rounded up to the volume's sectors (none
 * asked, or no sector told: 0x4000; no volume told: 0x800 bytes a sector),
 * its length; mapped into memory if asked (0x800, kept only if it is).  1
 * if opened; else 0 and reported.  The frame: GetDiskFreeSpace's root --
 * never set, what the stack's words hold -- and its four numbers. */
Answer openFile(App* app, CPU& cpu, reg32 name, reg32 flags, reg32 block, reg32 handleOut)
{
    const GuestFrame frame(cpu, 0xc, 0x12c);
    const reg32 root = frame - 0x12c, sectors = root + 0x104, sectorBytes = root + 0x108;
    const reg32 free = root + 0x10c, clusters = root + 0x110;
    const Answer taken = asCalled(cpu, [&] { return takeSlot(app, cpu); });
    if (!taken)
        return std::nullopt;
    const reg32 index = *taken;
    put(app, handleOut, 0);
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    if (sreg32(index) < 0 || sreg32(index) >= sreg32(at(app, kHandleCount)))
        return 0;
    const reg8 how = reg8(flags), windowsHow = reg8(flags >> 8);
    reg32 access = 0, share = 0;
    if (how & 1)
        access = kGenericRead;
    if (how & 2)
        access |= kGenericWrite;
    if (how & 4)
        share |= kShareRead;
    if (how & 8)
        share |= kShareWrite;
    reg32 creation;
    if (how & 0x10)
        creation = (how & 0x20) ? kCreateAlways : kCreateNew;
    else
        creation = (how & 0x20) ? kTruncateExisting : kOpenExisting;
    reg32 attributes = kAttributeNormal;
    if (windowsHow & 1)
        attributes |= kNoBuffering;
    if (windowsHow & 2)
        attributes |= kSequentialScan;
    if (windowsHow & 4)
        attributes |= kWriteThrough;
    put(app, slotAt(app, index) + kHow, flags);
    const Answer drive = asCalled(cpu, [&] { return numberOf(app, cpu, name); });
    if (!drive)
        return std::nullopt;
    putByte(app, slotAt(app, index) + kDrive, reg8(*drive));
    if (driveLock(app, *drive) == 0)
    {
        cpu.eax = 0;  // as the code has it at the call
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
        if (!lock)
            return std::nullopt;
        put(app, kDriveLocks + *drive * 4, *lock);
    }
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, *drive)))
        return std::nullopt;
    const Answer told = windows<kGetDiskFreeSpaceA, &win32::kernel32::GetDiskFreeSpaceA>(app, cpu, root, sectors, sectorBytes, free, clusters);
    if (!told)
        return std::nullopt;
    if (*told == 0)
        put(app, sectorBytes, kNoDiskSector);
    const Answer file = windows<kCreateFileA, &win32::kernel32::CreateFileA>(app, cpu, name, access, share, 0, creation, attributes, 0);
    if (!file)
        return std::nullopt;
    const reg32 slot = slotAt(app, index);
    put(app, slot + kFile, *file);
    reg32 opened = 0;
    if (*file == kNone)
    {
        if (!reportLastError(app, cpu, kCreateFileFailed))
            return std::nullopt;
        putByte(app, slotAt(app, index) + kInUse, 0);
    }
    else
    {
        const reg32 sector = block != 0 ? at(app, sectorBytes) : 0;
        if (sector != 0)
            put(app, slot + kBlock, block > sector ? (block + sector - 1) / sector * sector : sector);
        else
            put(app, slotAt(app, index) + kBlock, kDefaultBlock);
        const Answer length = windows<kGetFileSize, &win32::kernel32::GetFileSize>(app, cpu, at(app, slotAt(app, index) + kFile), 0);
        if (!length)
            return std::nullopt;
        put(app, slotAt(app, index) + kLength, *length);
        bool mapped = false;
        if (windowsHow & kMappedHigh)
        {
            const bool write = (how & 2) != 0;
            const Answer mapping = windows<kCreateFileMappingA, &win32::kernel32::CreateFileMappingA>(app, cpu, at(app, slotAt(app, index) + kFile), 0, write ? kCommitReadWrite : kCommitReadOnly, 0, 0, 0);
            if (!mapping)
                return std::nullopt;
            const reg32 mapSlot = slotAt(app, index);
            put(app, mapSlot + kMapping, *mapping);
            if (*mapping != 0)
            {
                const Answer view = windows<kMapViewOfFile, &win32::kernel32::MapViewOfFile>(app, cpu, *mapping, write ? kMapWrite : kMapRead, 0, 0, at(app, mapSlot + kLength));
                if (!view)
                    return std::nullopt;
                put(app, slotAt(app, index) + kMemory, *view);
            }
            mapped = at(app, slotAt(app, index) + kMemory) != 0;
        }
        if (!mapped)
        {
            const reg32 high = slotAt(app, index) + kHow + 1;
            putByte(app, high, byte(app, high) & reg8(~kMappedHigh));
        }
        put(app, handleOut, index);
        put(app, handleOut, ~index);
        opened = 1;
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, *drive)))
        return std::nullopt;
    return opened;
}

/* handleInfo (0x4e3980): what handle `handle` is: how it was opened to
 * [howOut], its block to [blockOut], its length to [lengthOut], and the
 * bytes free on its drive to [freeOut] (each pointer may be 0) -- the
 * clusters free by the sectors' bytes, not by the cluster's, told into the
 * frame for its root " :\" made the drive's: 0 back, reported, if the drive
 * tells nothing.  1 if told, 0 if the handle is not open. */
Answer infoOf(App* app, CPU& cpu, reg32 handle, reg32 howOut, reg32 blockOut, reg32 lengthOut, reg32 freeOut)
{
    const GuestFrame frame(cpu, 0xc, 0x20);
    const reg32 sectors = frame - 0x20, sectorBytes = sectors + 4, free = sectors + 8, clusters = sectors + 0xc;
    const reg32 root = sectors + 0x10;
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, drive)) || !windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    if (howOut != 0)
        put(app, howOut, at(app, slotAt(app, *index) + kHow));
    if (blockOut != 0)
        put(app, blockOut, at(app, slotAt(app, *index) + kBlock));
    if (lengthOut != 0)
        put(app, lengthOut, at(app, slotAt(app, *index) + kLength));
    reg32 told = 1;
    if (freeOut != 0)
    {
        copyText(app, root, kDriveRoot);
        putByte(app, root, reg8(drive + 0x40));
        const Answer got = windows<kGetDiskFreeSpaceA, &win32::kernel32::GetDiskFreeSpaceA>(app, cpu, root, sectors, sectorBytes, free, clusters);
        if (!got)
            return std::nullopt;
        if (*got != 0)
            put(app, freeOut, at(app, free) * at(app, sectorBytes));
        else
        {
            if (!reportLastError(app, cpu, kDiskFreeFailed))
                return std::nullopt;
            told = 0;
        }
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, drive)))
        return std::nullopt;
    return told;
}

/* readHandleBytes (0x4e3ae0): `count` bytes of handle `handle` read to
 * `buffer` -- no more than are left of it -- a block at a time, from its
 * memory if it is mapped (memmove), under its drive's lock (+2 set while;
 * cleared from elsewhere, the read stops); a read pending waited for.  How
 * many were read (0 if it is not open, or reported on an error).  The
 * frame's word: how many ReadFile read. */
Answer readBytes(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 count)
{
    const GuestFrame frame(cpu, 0x10, 0x1c);
    const reg32 got = frame - 0x1c + 4;
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 offset = *index << kSlotShift;
    const reg32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, drive)) || !windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    reg32 slot = at(app, kHandles) + offset;
    if (sreg32(at(app, slot + kPlace)) > sreg32(at(app, slot + kLength)))
        put(app, slot + kPlace, at(app, slot + kLength));
    slot = at(app, kHandles) + offset;
    if (sreg32(at(app, slot + kPlace) + count) > sreg32(at(app, slot + kLength)))
        count = at(app, slot + kLength) - at(app, slot + kPlace);
    slot = at(app, kHandles) + offset;
    reg32 block = at(app, slot + kBlock);
    putByte(app, slot + kReading, 1);
    reg32 total = 0;
    while (count != 0)
    {
        if (byte(app, at(app, kHandles) + offset + kReading) == 0)
            break;
        if (block > count)
            block = count;
        slot = at(app, kHandles) + offset;
        const reg32 memory = at(app, slot + kMemory);
        if (memory != 0)
        {
            if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, buffer, at(app, slot + kPlace) + memory, block))
                return std::nullopt;
            put(app, got, block);
            if (!guest::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u))
                return std::nullopt;
        }
        else
        {
            const Answer read = windows<kReadFile, &win32::kernel32::ReadFile>(app, cpu, at(app, slot + kFile), buffer, block, got, 0);
            if (!read)
                return std::nullopt;
            if (*read == 0)
            {
                const Answer code = windows<kGetLastError, &win32::kernel32::GetLastError>(app, cpu);
                if (!code)
                    return std::nullopt;
                if (*code != kIoPending)
                {
                    if (!reportError(app, cpu, kReadFileFailed, *code & 0xffff))
                        return std::nullopt;
                    total = 0;
                    break;
                }
                if (!guest::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1u))
                    return std::nullopt;
                continue;
            }
        }
        slot = at(app, kHandles) + offset;
        const reg32 place = at(app, slot + kPlace) + at(app, got);
        put(app, slot + kPlace, place);
        total += at(app, got);
        buffer += at(app, got);
        if (sreg32(place) > sreg32(at(app, slot + kLength)))
            put(app, slot + kPlace, at(app, slot + kLength));
        if (block > at(app, got))
            break;  // fewer than asked: the end
        count -= at(app, got);
    }
    putByte(app, at(app, kHandles) + offset + kReading, 0);
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, drive)))
        return std::nullopt;
    return total;
}

/* writeHandleBytes (0x4e3cc0): `count` bytes at `buffer` written to handle
 * `handle` a block at a time -- to its memory if it is mapped (memmove), no
 * more than its length there -- under its drive's lock, its length grown as
 * it is written past; a write pending waited for.  How many were written (0
 * if it is not open, or reported on an error).  The frame's word: how many
 * WriteFile wrote. */
Answer writeBytes(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 count)
{
    const GuestFrame frame(cpu, 0x10, 0x10);
    const reg32 wrote = frame - 0x10;
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 offset = *index << kSlotShift;
    const reg32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, drive)) || !windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    reg32 slot = at(app, kHandles) + offset;
    if (sreg32(at(app, slot + kPlace)) > sreg32(at(app, slot + kLength)))
        put(app, slot + kPlace, at(app, slot + kLength));
    slot = at(app, kHandles) + offset;
    reg32 block = at(app, slot + kBlock);
    if (at(app, slot + kMemory) != 0 && sreg32(at(app, slot + kPlace) + count) > sreg32(at(app, slot + kLength)))
        count = at(app, slot + kLength) - at(app, slot + kPlace);
    reg32 total = 0;
    while (count != 0)
    {
        if (block > count)
            block = count;
        slot = at(app, kHandles) + offset;
        const reg32 memory = at(app, slot + kMemory);
        if (memory != 0)
        {
            if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, memory + at(app, slot + kPlace), buffer, block))
                return std::nullopt;
            put(app, wrote, block);
            if (!guest::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u))
                return std::nullopt;
        }
        else
        {
            const Answer written = windows<kWriteFile, &win32::kernel32::WriteFile>(app, cpu, at(app, slot + kFile), buffer, block, wrote, 0);
            if (!written)
                return std::nullopt;
            if (*written == 0)
            {
                const Answer code = windows<kGetLastError, &win32::kernel32::GetLastError>(app, cpu);
                if (!code)
                    return std::nullopt;
                if (*code != kIoPending)
                {
                    if (!reportError(app, cpu, kWriteFileFailed, *code & 0xffff))
                        return std::nullopt;
                    total = 0;
                    break;
                }
                if (!guest::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1u))
                    return std::nullopt;
                continue;
            }
        }
        slot = at(app, kHandles) + offset;
        put(app, slot + kPlace, at(app, slot + kPlace) + at(app, wrote));
        total += at(app, wrote);
        buffer += at(app, wrote);
        if (block > at(app, wrote))
            count = 0;  // fewer than asked: no more
        else
            count -= at(app, wrote);
        slot = at(app, kHandles) + offset;
        if (sreg32(at(app, slot + kPlace)) > sreg32(at(app, slot + kLength)))
            put(app, slot + kLength, at(app, slot + kPlace));
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, drive)))
        return std::nullopt;
    return total;
}

/* seekHandleTo (0x4e3e60): handle `handle` sought to `to`, kept within its
 * length, under its drive's lock -- the file itself too unless it is mapped;
 * then 1.  A mapped one gives back what the frame's word holds (the code's
 * local, never set).  0 if it is not open. */
Answer seekTo(App* app, CPU& cpu, reg32 handle, reg32 to)
{
    const GuestFrame frame(cpu, 0x10, 4);
    const reg32 sought = frame - 4;
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const reg32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!windows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, driveLock(app, drive)) || !windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    reg32 slot = slotAt(app, *index);
    const reg32 length = at(app, slot + kLength);
    put(app, slot + kPlace, to);
    if (sreg32(to) > sreg32(length))
        put(app, slot + kPlace, length);
    else if (sreg32(to) < 0)
        put(app, slot + kPlace, 0);
    slot = slotAt(app, *index);
    if (at(app, slot + kMemory) == 0)
    {
        const Answer place = windows<kSetFilePointer, &win32::kernel32::SetFilePointer>(app, cpu, at(app, slot + kFile), at(app, slot + kPlace), 0, 0);
        if (!place)
            return std::nullopt;
        put(app, sought, 1);
        put(app, slotAt(app, *index) + kPlace, *place);
    }
    if (!windows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, driveLock(app, drive)))
        return std::nullopt;
    return at(app, sought);
}

/* handlePosition (0x4e3f30): where handle `handle` is: the file asked
 * unless it is mapped (an error reported, 0); 0 if it is not open. */
Answer placeOf(App* app, CPU& cpu, reg32 handle)
{
    const GuestFrame frame(cpu, 0x10, 0);
    const std::optional<reg32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const GuestFrame kept(cpu, 8, 0);  // edi and ecx pushed for the rest
    if (!windows<kSetLastError, &win32::kernel32::SetLastError>(app, cpu, 0))
        return std::nullopt;
    const reg32 slot = slotAt(app, *index);
    if (at(app, slot + kMemory) == 0)
    {
        const Answer place = windows<kSetFilePointer, &win32::kernel32::SetFilePointer>(app, cpu, at(app, slot + kFile), 0, 0, kFileCurrent);
        if (!place)
            return std::nullopt;
        put(app, slotAt(app, *index) + kPlace, *place);
        if (*place == kNone)
        {
            if (!reportLastError(app, cpu, kSetFilePointerFailed))
                return std::nullopt;
            put(app, slotAt(app, *index) + kPlace, 0);
        }
    }
    return at(app, slotAt(app, *index) + kPlace);
}

/* openMemoryHandle (0x4e4140): `length` bytes at `memory` taken as a file,
 * mapped (0x800) on drive `drive` -- its lock made the first time -- read
 * and written by blocks of `block` (0: 0x4000), no file nor mapping (-1);
 * its handle to [handleOut].  1 if a slot was free, else 0. */
Answer openMemory(App* app, CPU& cpu, reg32 memory, reg32 length, reg32 drive, reg32 block, reg32 handleOut)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const Answer taken = asCalled(cpu, [&] { return takeSlot(app, cpu); });
    if (!taken)
        return std::nullopt;
    const reg32 index = *taken;
    if (sreg32(index) < 0 || sreg32(index) >= sreg32(at(app, kHandleCount)))
        return 0;
    if (driveLock(app, drive) == 0)
    {
        cpu.eax = index;  // as the code has it at the call
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
        if (!lock)
            return std::nullopt;
        put(app, kDriveLocks + drive * 4, *lock);
    }
    reg32 slot = slotAt(app, index);
    put(app, slot + kFile, kNone);
    put(app, slot + kHow, kMapped);
    putByte(app, slot + kDrive, reg8(drive));
    if (block == 0)
        block = kDefaultBlock;
    slot = slotAt(app, index);
    put(app, slot + kMapping, kNone);
    put(app, slot + kPlace, 0);
    put(app, slot + kBlock, block);
    put(app, slot + kMemory, memory);
    put(app, slot + kLength, length);
    put(app, handleOut, index);
    put(app, handleOut, ~index);
    return 1;
}

}  // namespace

/* closeHandle (0x4e2da0, EA's closehandle): checked for a surface locked,
 * then handle `handle` closed (closeHandleSlot); 1 if it was open. */
Answer closeChecked(App* app, CPU& cpu, reg32 handle)
{
    const GuestFrame frame(cpu, 0x10, 0);
    where(app, kFileioC, kLibCloseHandleName, 0x19d);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, kCloseHandleName); }))
        return std::nullopt;
    return closeSlot(app, cpu, handle);
}

namespace
{

using Transfer = Answer (*)(App*, CPU&, reg32, reg32, reg32);

/* readHandle and writeHandle: `name` of win\fileio.c's at `line` checked
 * for a surface locked, then `count` bytes of handle `handle` read to or
 * written from `buffer`; how many. */
Answer transferChecked(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 count, reg32 name, reg32 line,
                       Transfer transfer)
{
    const GuestFrame frame(cpu, 0x10, 0);
    where(app, kFileioC, name, line);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, name); }))
        return std::nullopt;
    return asCalled(cpu, [&] { return transfer(app, cpu, handle, buffer, count); });
}

}  // namespace

/* seekHandle (0x4e2e60, EA's seekhandle): checked for a surface locked,
 * then handle `handle` sought to `to` (seekHandleTo); where it is then
 * (handlePosition), or 0 if it is not open. */
Answer seekChecked(App* app, CPU& cpu, reg32 handle, reg32 to)
{
    const GuestFrame frame(cpu, 0x10, 0);
    where(app, kFileioC, kSeekHandleName, 0x219);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, kSeekHandleName); }))
        return std::nullopt;
    const Answer sought = asCalled(cpu, [&] { return seekTo(app, cpu, handle, to); });
    if (!sought || *sought == 0)
        return sought;
    return asCalled(cpu, [&] { return placeOf(app, cpu, handle); });
}

/* openHandle (0x4e28d0, EA's openhandle): the file named `name` opened to
 * read (0x205), its handle to [handleOut], its length to [lengthOut],
 * where it starts in it to [startOut] (0; either pointer may be 0: the
 * frame's words then).  Not found, it is looked for in the archive: the
 * archive opened, its directory read in, the file looked up and sought, its
 * place in the archive to [startOut] and its length (as the lookup found
 * it) to [lengthOut].  1 if opened, else 0 -- and with [0x56440c] set, an
 * Error. */
Answer openToRead(App* app, CPU& cpu, reg32 name, reg32 handleOut, reg32 startOut, reg32 lengthOut)
{
    const GuestFrame frame(cpu, 0xc, 0x14);
    const reg32 ownLength = frame - 0x14, ownStart = ownLength + 4;
    where(app, kFileioC, kOpenHandleName, 0x9b);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, kOpenHandleName); }))
        return std::nullopt;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, lengthOut, 0);
    put(app, startOut, 0);
    const Answer opened = asCalled(cpu, [&] { return openFile(app, cpu, name, kOpenFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened != 0)
    {
        const Answer told = asCalled(cpu, [&] { return infoOf(app, cpu, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            return 1;
        if (at(app, kErrorsFatal) != 0 && !error(app, cpu, kFileioC, kOpenHandleName, 0xc1, {kInfoFailed, name}))
            return std::nullopt;
        if (!closeSlot(app, cpu, at(app, handleOut)))
            return std::nullopt;
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
        return 0;
    }
    if (at(app, kArchiveLookup) != 0 && byte(app, kArchiveName) != 0)
    {
        const Answer archive = asCalled(cpu, [&] { return openFile(app, cpu, kArchiveName, kOpenFlags, 0, handleOut); });
        if (!archive)
            return std::nullopt;
        if (*archive != 0)
        {
            const Answer read = asCalled(cpu, [&] {
                return readBytes(app, cpu, at(app, handleOut), at(app, kArchiveDirectory), at(app, kArchiveDirectoryBytes));
            });
            if (!read)
                return std::nullopt;
            put(app, kArchiveDirectoryBytes, *read);
            if (*read != 0)
            {
                const Answer place = call(app, cpu, at(app, kArchiveLookup), {at(app, kArchiveDirectory), name});
                if (!place)
                    return std::nullopt;
                put(app, startOut, *place);
                if (*place != 0)
                {
                    const Answer sought = asCalled(cpu, [&] { return seekTo(app, cpu, at(app, handleOut), *place); });
                    if (!sought)
                        return std::nullopt;
                    if (*sought != 0)
                    {
                        put(app, lengthOut, at(app, at(app, kArchiveFoundLength)));
                        return 1;
                    }
                }
            }
            if (!closeSlot(app, cpu, at(app, handleOut)))
                return std::nullopt;
            put(app, handleOut, 0);
        }
    }
    if (at(app, kErrorsFatal) != 0 && !error(app, cpu, kFileioC, kOpenHandleName, 0xba, {kOpenFailed, name}))
        return std::nullopt;
    return 0;
}

namespace
{

/* openHandleErrorsAs (0x4e2ab0): openHandle (by address) with [0x56440c]
 * -- an Error if it fails -- as `errorsAs` says for the call. */
Answer openErrorsAs(App* app, CPU& cpu, reg32 name, reg32 handleOut, reg32 startOut, reg32 lengthOut,
                    reg32 errorsAs)
{
    const GuestFrame frame(cpu, 8, 0);
    const reg32 saved = at(app, kErrorsFatal);
    put(app, kErrorsFatal, errorsAs);
    const Answer opened = call(app, cpu, kOpenHandle, {name, handleOut, startOut, lengthOut});
    if (!opened)
        return std::nullopt;
    put(app, kErrorsFatal, saved);
    return opened;
}

}  // namespace

/* openHandleToWrite (0x4e2ae0, EA's openhandlew): the file named `name`
 * opened to write -- made anew (0x232) -- its handle to [handleOut], its
 * length to [lengthOut] and 0 to [startOut] (either pointer may be 0: the
 * frame's words then).  1 if opened, else 0; a file opened that tells
 * nothing of itself closed again. */
Answer openToWrite(App* app, CPU& cpu, reg32 name, reg32 handleOut, reg32 startOut, reg32 lengthOut)
{
    const GuestFrame frame(cpu, 0xc, 0xc);
    const reg32 ownLength = frame - 0xc, ownStart = ownLength + 4;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, kWhereFile, kFileioC);
    put(app, kWhereLine, 0xdf);
    put(app, kWhereFunction, kOpenHandleWName);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, kOpenHandleWName); }))
        return std::nullopt;
    const Answer opened = asCalled(cpu, [&] { return openFile(app, cpu, name, kWriteFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    reg32 result = 0;
    if (*opened == 0)
    {
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
    }
    else
    {
        const Answer told = asCalled(cpu, [&] { return infoOf(app, cpu, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            result = 1;
        else
        {
            if (!closeSlot(app, cpu, at(app, handleOut)))
                return std::nullopt;
            put(app, handleOut, 0);
            put(app, lengthOut, 0);
        }
    }
    put(app, startOut, 0);
    return result;
}

namespace
{

/* openMapHandle (0x4e2ba0, EA's openmaphandle): the file named `name`
 * opened mapped into memory (0x805), its handle to [handleOut], its length
 * to [lengthOut], 0 to [startOut] (either pointer may be 0: the frame's
 * words then); where it is mapped, or 0.  Not found, it is looked for in the
 * archive as openHandle does: the archive's memory and the file's place in
 * it. */
Answer openMapped(App* app, CPU& cpu, reg32 name, reg32 handleOut, reg32 startOut, reg32 lengthOut)
{
    const GuestFrame frame(cpu, 0xc, 0x14);
    const reg32 ownLength = frame - 0x14, ownStart = ownLength + 4;
    where(app, kFileioC, kOpenMapHandleName, 0x141);
    if (!asCalled(cpu, [&] { return surfaceUnlocked(app, cpu, kOpenHandleAName); }))
        return std::nullopt;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, lengthOut, 0);
    put(app, startOut, 0);
    const Answer opened = asCalled(cpu, [&] { return openFile(app, cpu, name, kMapFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened != 0)
    {
        const Answer told = asCalled(cpu, [&] { return infoOf(app, cpu, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            return memoryOf(app, at(app, handleOut));
        if (at(app, kErrorsFatal) != 0
            && !error(app, cpu, kFileioC, kOpenMapHandleName, 0x150, {kMapInfoFailed, name}))
            return std::nullopt;
        if (!closeSlot(app, cpu, at(app, handleOut)))
            return std::nullopt;
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
        return 0;
    }
    if (at(app, kArchiveLookup) != 0 && byte(app, kArchiveName) != 0)
    {
        const Answer archive = asCalled(cpu, [&] { return openFile(app, cpu, kArchiveName, kMapFlags, 0, handleOut); });
        if (!archive)
            return std::nullopt;
        if (*archive != 0)
        {
            const reg32 memory = memoryOf(app, at(app, handleOut));
            const Answer place = call(app, cpu, at(app, kArchiveLookup), {memory, name});
            if (!place)
                return std::nullopt;
            put(app, startOut, *place);
            const reg32 mapped = *place + memory;
            put(app, lengthOut, at(app, at(app, kArchiveFoundLength)));
            if (at(app, startOut) != 0 || at(app, kErrorsFatal) == 0)
                return mapped;
            if (!error(app, cpu, kFileioC, kOpenMapHandleName, 0x15f, {kMapOpenFailed, name}))
                return std::nullopt;
            return mapped;
        }
    }
    if (at(app, kErrorsFatal) != 0 && !error(app, cpu, kFileioC, kOpenMapHandleName, 0x162, {kMapOpenFailed, name}))
        return std::nullopt;
    return 0;
}

/* openBlockHandle (0x501270, EA's openblockhandle): the file named `name`
 * opened to read (0x205), its handle to [handleOut], its block to
 * [blockOut], its length to [lengthOut] and 0 to [startOut].  1 if opened,
 * else 0 -- an Error with [0x56440c] set. */
Answer openBlocks(App* app, CPU& cpu, reg32 name, reg32 handleOut, reg32 startOut, reg32 lengthOut, reg32 blockOut)
{
    const GuestFrame frame(cpu, 0xc, 4);
    const Answer opened = asCalled(cpu, [&] { return openFile(app, cpu, name, kOpenFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened == 0)
    {
        if (at(app, kErrorsFatal) != 0
            && !error(app, cpu, kBlockioC, kOpenBlockHandleName, 0x52, {kBlockOpenFailed, name}))
            return std::nullopt;
        put(app, handleOut, 0);
        return 0;
    }
    const Answer told =
        asCalled(cpu, [&] { return infoOf(app, cpu, at(app, handleOut), 0, blockOut, lengthOut, 0); }, 4);
    if (!told)
        return std::nullopt;
    if (*told != 0)
    {
        put(app, startOut, 0);
        return 1;
    }
    if (at(app, kErrorsFatal) != 0
        && !error(app, cpu, kBlockioC, kOpenBlockHandleName, 0x58, {kBlockInfoFailed, name}))
        return std::nullopt;
    if (!closeSlot(app, cpu, at(app, handleOut)))
        return std::nullopt;
    put(app, handleOut, 0);
    return 0;
}

/* blockHandleFile (0x501360, EA's blockhandlefile): not supported -- an
 * Error, and 0. */
Answer noBlockFile(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0xc, 0);
    if (!error(app, cpu, kBlockioC, kBlockHandleFileName, 0x66, {kBlockFileUnsupported}))
        return std::nullopt;
    return 0;
}

/* readBlockHandle (0x5013b0, EA's readblockhandle): `blocks` blocks of
 * handle `handle` read to `buffer` (readHandleBytes), its block told into
 * the frame (handleInfo); the bytes read, 0 if the handle tells nothing (an
 * Error with [0x56440c] set). */
Answer readBlocks(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 blocks)
{
    const GuestFrame frame(cpu, 0x10, 8);
    const reg32 block = frame - 8;
    const Answer told = asCalled(cpu, [&] { return infoOf(app, cpu, handle, 0, block, 0, 0); }, 4);
    if (!told)
        return std::nullopt;
    if (*told == 0)
    {
        if (at(app, kErrorsFatal) != 0
            && !error(app, cpu, kBlockioC, kReadBlockHandleName, 0xd6, {kBlockReadFailed}))
            return std::nullopt;
        return 0;
    }
    return asCalled(cpu, [&] { return readBytes(app, cpu, handle, buffer, at(app, block) * blocks); });
}

/* seekBlockHandle (0x501450, EA's seekblockhandle): handle `handle` sought
 * to `to` (seekHandleTo) -- an Error if it is not open -- and where it is
 * (handlePosition). */
Answer seekBlocks(App* app, CPU& cpu, reg32 handle, reg32 to)
{
    const GuestFrame frame(cpu, 4, 0);
    const Answer sought = asCalled(cpu, [&] { return seekTo(app, cpu, handle, to); });
    if (!sought)
        return std::nullopt;
    if (*sought == 0)
    {
        const GuestFrame saved(cpu, 8, 0);  // esi and ebx pushed for the Error
        if (!error(app, cpu, kBlockioC, kSeekBlockHandleName, 0x115, {kBlockSeekFailed}))
            return std::nullopt;
    }
    return asCalled(cpu, [&] { return placeOf(app, cpu, handle); });
}

}  // namespace

/* 0x4e28c0: initHandles with 0 (a jump): the table of the default 0x40.  No
 * caller in the game. */
void initHandlesDefault(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, makeTable(app, cpu, 0));
}

/* 0x4e28d0, EA's openhandle: openToRead of the name eax, [edx] the handle,
 * [ebx] where it starts, [ecx] its length. */
void openHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openToRead(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx));
}

/* 0x4e2ab0: openErrorsAs of openHandle's registers and the stack's word;
 * the word taken off. */
void openHandleErrorsAs(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openErrorsAs(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4)), 4);
}

/* 0x4e2ae0, EA's openhandlew: openToWrite of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length. */
void openHandleToWrite(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openToWrite(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx));
}

/* 0x4e2ba0, EA's openmaphandle: openMapped of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length.  No caller in the game. */
void openMapHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openMapped(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx));
}

/* 0x4e2da0, EA's closehandle: closeChecked of handle eax. */
void closeHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, closeChecked(app, cpu, cpu.eax));
}

/* readHandle's core (0x4e2de0): transferChecked of readBytes, as
 * win\fileio.c's readhandle at its line. */
Answer readChecked(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 count)
{
    return transferChecked(app, cpu, handle, buffer, count, kReadHandleName, 0x1d4, readBytes);
}

/* writeHandle's core (0x4e2e20): transferChecked of writeBytes, as
 * win\fileio.c's writehandle at its line. */
Answer writeChecked(App* app, CPU& cpu, reg32 handle, reg32 buffer, reg32 count)
{
    return transferChecked(app, cpu, handle, buffer, count, kWriteHandleName, 0x1dd, writeBytes);
}

/* 0x4e2de0, EA's readhandle: ebx bytes of handle eax read to edx
 * (readHandleBytes); how many back. */
void readHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, transferChecked(app, cpu, cpu.eax, cpu.edx, cpu.ebx, kReadHandleName, 0x1d4, readBytes));
}

/* 0x4e2e20, EA's writehandle: ebx bytes at edx written to handle eax
 * (writeHandleBytes); how many back. */
void writeHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, transferChecked(app, cpu, cpu.eax, cpu.edx, cpu.ebx, kWriteHandleName, 0x1dd, writeBytes));
}

/* 0x4e2e60, EA's seekhandle: seekChecked of handle eax to edx. */
void seekHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, seekChecked(app, cpu, cpu.eax, cpu.edx));
}

/* 0x4e2eb0: 1 if al is a slash, \ or /, else 0. */
void isSlash(App*, CPU& cpu)
{
    cpu.eax = isSlashChar(cpu.al) ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4e2ebe: isSlash's way out with 0 -- an entry of its own, as handlesReady
 * jumps to it: eax 0. */
void notSlash(App*, CPU& cpu)
{
    cpu.eax = 0;
    cpu.esp += 4;
}

/* 0x4e2ed0: rootOf of path eax to edx; what it leaves in eax nobody reads
 * (ea_files.inc). */
void volumeRoot(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    if (rootOf(app, cpu, cpu.eax, cpu.edx))
        cpu.esp += 4;
}

/* 0x4e2fd0: fullPath of path eax to edx, ebx bytes there.  ecx, esi, edi
 * and ebp put where the code pushes them, above the frame, and taken back
 * from there: a root of 0x104 bytes and more, with no slash, is cut past the
 * frame's locals (volumeRoot), at the 0 of the file's part's address, and
 * its ending put in the low byte of the ebp kept there. */
void fullPathName(App* app, CPU& cpu)
{
    const reg32 pushed = cpu.esp - 0x10;
    put(app, pushed + 0xc, cpu.ecx);
    put(app, pushed + 8, cpu.esi);
    put(app, pushed + 4, cpu.edi);
    put(app, pushed, cpu.ebp);
    Answer found;
    {
        const Kept kept(cpu);
        found = fullPath(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    }
    if (!found)
        return;
    cpu.ebp = at(app, pushed);
    cpu.edi = at(app, pushed + 4);
    cpu.esi = at(app, pushed + 8);
    cpu.ecx = at(app, pushed + 0xc);
    returns(cpu, found);
}

/* 0x4e3110: GetLogicalDrives' bits.  No caller in the game. */
void logicalDrives(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, windows<kGetLogicalDrives, &win32::kernel32::GetLogicalDrives>(app, cpu));
}

/* 0x4e3120: driveOf of path eax, its root to edx.  No caller in the game. */
void driveOfPath(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, driveOf(app, cpu, cpu.eax, cpu.edx));
}

/* 0x4e3250: letterOf of path eax. */
void driveLetter(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, letterOf(app, cpu, cpu.eax));
}

/* 0x4e32e0: bytesFree of the root eax.  No caller in the game. */
void diskFreeBytes(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, bytesFree(app, cpu, cpu.eax));
}

/* 0x4e3330: numberOf of path eax. */
void driveNumber(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, numberOf(app, cpu, cpu.eax));
}

/* 0x4e3340, EA's setdirectory: changeDirectory to path eax. */
void setDirectory(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, changeDirectory(app, cpu, cpu.eax));
}

/* 0x4e33a0, EA's getdirectory: currentDirectory to eax. */
void getDirectory(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, currentDirectory(app, cpu, cpu.eax));
}

/* 0x4e3410: 1 if the handles' table is made, else 0 (notSlash's way out).
 * No caller in the game. */
void handlesReady(App* app, CPU& cpu)
{
    cpu.eax = at(app, kHandles) != 0 ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4e3430: makeTable for eax handles. */
void initHandles(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, makeTable(app, cpu, cpu.eax));
}

/* 0x4e34c0: closeTable; what it leaves in eax nobody reads
 * (ea_files.inc). */
void closeAllHandles(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    if (closeTable(app, cpu))
        cpu.esp += 4;
}

/* 0x4e3570: takeSlot; the slot's index back. */
void takeHandleSlot(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, takeSlot(app, cpu));
}

/* 0x4e3600: removeFile of the name eax. */
void deleteFile(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, removeFile(app, cpu, cpu.eax));
}

/* 0x4e3620: openFile of the name eax as the flags edx, block ebx, [ecx] the
 * handle. */
void openFileHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openFile(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx));
}

/* 0x4e3980: infoOf of handle eax, to [edx], [ebx], [ecx] and the stack's
 * word; the word taken off. */
void handleInfo(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, infoOf(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4)), 4);
}

/* 0x4e3ae0: readBytes, ebx bytes of handle eax to edx. */
void readHandleBytes(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, readBytes(app, cpu, cpu.eax, cpu.edx, cpu.ebx));
}

/* 0x4e3cc0: writeBytes, ebx bytes at edx to handle eax. */
void writeHandleBytes(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, writeBytes(app, cpu, cpu.eax, cpu.edx, cpu.ebx));
}

/* 0x4e3e60: seekTo, handle eax to edx. */
void seekHandleTo(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, seekTo(app, cpu, cpu.eax, cpu.edx));
}

/* 0x4e3f30: placeOf handle eax. */
void handlePosition(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, placeOf(app, cpu, cpu.eax));
}

/* 0x4e3fe0: closeSlot of handle eax. */
void closeHandleSlot(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, closeSlot(app, cpu, cpu.eax));
}

/* 0x4e40a0: handle eax's drive's lock entered; 1, or 0 if it is not
 * open. */
void lockHandleDrive(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, lockDriveOf(app, cpu, cpu.eax, kEnterCriticalSection));
}

/* 0x4e40f0: handle eax's drive's lock left; 1, or 0 if it is not open. */
void unlockHandleDrive(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, lockDriveOf(app, cpu, cpu.eax, kLeaveCriticalSection));
}

/* 0x4e4140: openMemory of edx bytes at eax, drive ebx, block ecx, the
 * handle to the stack's word; the word taken off. */
void openMemoryHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openMemory(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4)), 4);
}

/* 0x4e41f0: memoryOf handle eax. */
void handleMemory(App* app, CPU& cpu)
{
    cpu.eax = memoryOf(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x501010: surfaceUnlocked for the function named eax. */
void checkSurfaceUnlocked(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, surfaceUnlocked(app, cpu, cpu.eax));
}

/* 0x501050, cdecl (threshold's address, level, format, ...): report. */
void fileReport(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const reg32 words = cpu.esp + 4;
    returns(cpu, report(app, cpu, at(app, words), at(app, words + 4), at(app, words + 8), words + 0xc));
}

/* 0x501110: allocatePages for the bytes [eax]. */
void virtualAllocPages(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, allocatePages(app, cpu, cpu.eax));
}

/* 0x501180: freePages at eax. */
void virtualFreePages(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, freePages(app, cpu, cpu.eax));
}

/* 0x5011a0, EA's touchmempages: touchPages of edx bytes from eax; ebx and
 * edx left as its loops leave them. */
void touchMemoryPages(App* app, CPU& cpu)
{
    const reg32 start = cpu.eax, bytes = cpu.edx;
    std::optional<Touched> touched;
    {
        const Kept kept(cpu);
        touched = touchPages(app, cpu, start, bytes);
    }
    if (!touched)
        return;
    cpu.eax = touched->result;
    cpu.ebx = touched->end;
    cpu.edx = touched->left;
    cpu.esp += 4;
}

/* 0x501270, EA's openblockhandle: openBlocks of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length, the block to the stack's
 * word; the word taken off. */
void openBlockHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, openBlocks(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4)), 4);
}

/* 0x501360, EA's blockhandlefile: noBlockFile.  No caller in the game. */
void blockHandleFile(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, noBlockFile(app, cpu));
}

/* 0x5013a0, EA's closeblockhandle: closeHandleSlot, jumped to. */
void closeBlockHandle(App* app, CPU& cpu)
{
    closeHandleSlot(app, cpu);
}

/* 0x5013b0, EA's readblockhandle: readBlocks, ebx blocks of handle eax to
 * edx. */
void readBlockHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, readBlocks(app, cpu, cpu.eax, cpu.edx, cpu.ebx));
}

/* 0x501450, EA's seekblockhandle: seekBlocks of handle eax to edx.  No
 * caller in the game. */
void seekBlockHandle(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    returns(cpu, seekBlocks(app, cpu, cpu.eax, cpu.edx));
}

}  // namespace ea
}  // namespace nfs3hp
