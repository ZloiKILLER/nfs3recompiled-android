#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "os_threads.h"
#include "os_files.h"
#include "os_system.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <initializer_list>
#include <optional>
#include <type_traits>

/* EA's files (win\fileio.c, win\file.c, win\blockio.c): the handles the game
 * opens its files by -- whole, mapped into memory, or a buffer of its own
 * taken as one -- read, written and sought a block at a time under the lock
 * of the drive each is on, their errors reported; the drives, paths and
 * directories they are on; and the memory pages EA's library takes from
 * Windows (win\touchmem.c).  The game's code calls each function here by name.
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
 * Each is a typed function under a name of its own, with a thunk under
 * the game's name for the callers still on registers and for the calls by address: it
 * leaves eax, and the registers the game's code keeps, as that code leaves
 * them.  The registers a function is handed its arguments in are its own
 * (Watcom's convention: the callers count on none of them after).  Other
 * files' typed functions -- the main thread asked, a surface locked, the
 * machine's memory, a copy, a sleep, a lock made and freed -- are called as
 * C++ (guest::direct, their headers), a lock made with eax as the code has it
 * at the call.  Every
 * other function of the game's is called by address, its arguments in
 * registers and on the guest's stack as the code hands them (call, error) --
 * openHandle (0x4e28d0) and virtualAllocPages (0x501110) too where this
 * file's own code calls them by address; the locks through their imports'
 * slots (windows); the files, drives and pages are the port's own
 * (os_files.h, os_system.h).  The port's calls give no error's code: an
 * error reported is 0.  A frame whose address a callee is handed -- a buffer
 * -- or under which Error or fileReport is called stays on the guest's stack
 * at the original's depth (GuestFrame, asCalled). */
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
using nfs::u32;
using nfs::u8;
using nfs::s32;

using App = nfs::App;
using ThreadState = nfs::ThreadState;
// What a function gives back in eax; nullopt when the game stopped in it.
using Answer = std::optional<u32>;

// Where an Error is from: the file, the function, the line.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// A file not opened ends the game, when set.
constexpr u32 kErrorsFatal = 0x56440c;
// Whether the functions called on the main thread are checked for a surface
// locked.
constexpr u32 kSurfaceCheck = 0x564364;
// The archive: its name, its directory read in, how much of it, the function
// that looks a file up in it, the file's length it found.
constexpr u32 kArchiveName = 0x564504, kArchiveDirectory = 0x564584, kArchiveLookup = 0x564588;
constexpr u32 kArchiveFoundLength = 0x56458c, kArchiveDirectoryBytes = 0x564590;
// The reports' level, the table's lock, the handles, the drives' locks.
constexpr u32 kReportLevel = 0x564594, kHandlesLock = 0x564598, kHandleCount = 0x56459c, kHandles = 0x5645a0;
constexpr u32 kDriveLocks = 0x9ef760, kDriveLocksBytes = 0x80;
// The reports' way out: none, OutputDebugString, stdout, the game's log.
constexpr u32 kReportOutput = 0x56789c, kStdout = 0x566f72;
constexpr u32 kToDebugger = 1, kToStdout = 2, kToLog = 3;
// Windows' page, and how much memory there is to touch.
constexpr u32 kPageSize = 0x5678a0, kPhysicalMemory = 0x5678a4;
// The handles' table: a slot's size, the default count and block.
constexpr u32 kSlotShift = 5, kSlotBytes = 0x20, kDefaultHandles = 0x40, kDefaultBlock = 0x4000;
// A slot's fields.
constexpr u32 kInUse = 0, kDrive = 1, kReading = 2, kFile = 4, kHow = 8, kBlock = 0xc, kMapping = 0x10;
constexpr u32 kMemory = 0x14, kPlace = 0x18, kLength = 0x1c;
// How a handle is opened: read (and the others may read), to write made
// anew, mapped; the mapped bit in +9.
constexpr u32 kOpenFlags = 0x205, kWriteFlags = 0x232, kMapFlags = 0x805, kMapped = 0x800;
constexpr u8 kMappedHigh = kMapped >> 8;
// No file, no mapping.
constexpr u32 kNone = 0xffffffff;
// The game's strings: win\fileio.c's, win\file.c's, win\touchmem.c's, win\blockio.c's.
constexpr u32 kFileioC = 0x54a148, kOpenHandleName = 0x54a158, kOpenFailed = 0x54a164, kInfoFailed = 0x54a188;
constexpr u32 kOpenHandleWName = 0x54a1ac, kOpenMapHandleName = 0x54a1b8, kOpenHandleAName = 0x54a1c8;
constexpr u32 kMapInfoFailed = 0x54a1d4, kMapOpenFailed = 0x54a1fc;
constexpr u32 kLibCloseHandleName = 0x54a228, kCloseHandleName = 0x54a238, kReadHandleName = 0x54a244;
constexpr u32 kWriteHandleName = 0x54a250, kSeekHandleName = 0x54a25c;
constexpr u32 kNoPath = 0x54a294, kFileC = 0x54a298;
constexpr u32 kGetDirectoryName = 0x54a2d4, kGetDirectoryFailed = 0x54a2e4, kCreateFileFailed = 0x54a320;
constexpr u32 kReadFileFailed = 0x54a36c;
constexpr u32 kWriteFileFailed = 0x54a38c, kSetFilePointerFailed = 0x54a3ac;
constexpr u32 kSurfaceLocked = 0x54e42c, kTouchmemC = 0x54e454, kTouchMemPagesName = 0x54e464;
constexpr u32 kTouchMemFirst = 0x54e474, kBlockioC = 0x54e49c, kOpenBlockHandleName = 0x54e4ac;
constexpr u32 kBlockOpenFailed = 0x54e4bc, kBlockInfoFailed = 0x54e4e4, kBlockHandleFileName = 0x54e50c;
constexpr u32 kBlockFileUnsupported = 0x54e51c, kReadBlockHandleName = 0x54e548, kBlockReadFailed = 0x54e558;
constexpr u32 kSeekBlockHandleName = 0x54e578, kBlockSeekFailed = 0x54e588;
// The game's functions, by address.
constexpr u32 kMessage = 0x4df6c0;
constexpr u32 kFillZero = 0x4e070c, kFflush = 0x4f7d70;
// The longest path copied as it is.
constexpr u32 kMaxPath = 0x104;

/* The frame of a core whose callees see where it is -- handed its locals' addresses, or Error
 * and fileReport -- kept on the guest's
 * stack at the original's depth: `pushed` bytes of registers below the return address's slot,
 * then `bytes` of locals.  frame - n is n bytes below the registers pushed. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* A core of this file called as the game's code calls it: `handed` bytes of stack arguments and
 * the return address's slot below esp, its own frame below them. */
template <typename Core>
auto asCalled(ThreadState& ts, Core core, u32 handed = 0)
{
    ts.sp -= handed + 4;
    const auto result = core();
    ts.sp += handed + 4;
    return result;
}

/* A call by address: eax, edx, ebx and ecx from `registers`, as many as given, the rest left as
 * they are; `words` pushed (cdecl or stdcall: esp back where it was after).  What it leaves in eax
 * back; nullopt when the game stopped.  Where that is used, eax is given as the code has it before
 * the call. */
Answer call(App* app, ThreadState& ts, u32 target, std::initializer_list<u32> registers,
            std::initializer_list<u32> words = {})
{
    return guest::pointerCall(app, ts, target, registers, words);
}

/* `Core`, the game's `Address`, called as C++ (guest::direct), handed `arguments`.  Its result (the
 * eax of the call by address); nullopt when the game stopped in it. */
template <u32 Address, auto Core, typename... Args>
Answer typed(App* app, ThreadState& ts, Args... arguments)
{
    const auto result = guest::direct<Address, Core>(app, ts, arguments...);
    if (ts.terminate)
        return std::nullopt;
    if constexpr (std::is_same_v<std::decay_t<decltype(result)>, Answer>)
    {
        if (!result)
            return std::nullopt;
        return *result;
    }
    else
        return u32(result);
}

/* Error (0x401010, cdecl) as C++, its format the first of `words` and the rest after it.  What it
 * leaves in eax; nullopt when the game stopped. */
Answer errorWith(App* app, ThreadState& ts, std::initializer_list<u32> words)
{
    guest::Words more({});
    for (const u32* word = words.begin() + 1; word < words.end(); ++word)
        more.word[more.count++] = *word;
    // What Error leaves in eax when it returns (the game ends in it otherwise); the eax the caller has at the
    // call is not known here: 0.
    return guest::direct<0x401010, ea::errorLeaves>(app, ts, 0, *words.begin(), more);
}

/* Where an Error is from, as the code sets it. */
void where(App* app, u32 file, u32 function, u32 line)
{
    put(app, kWhereFile, file);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
}

/* Error (0x401010, cdecl) from `function` of `file` at `line` -- where it is from set first --
 * its format and words handed.  What it leaves in eax; nullopt when the game stopped. */
Answer error(App* app, ThreadState& ts, u32 file, u32 function, u32 line, std::initializer_list<u32> words)
{
    where(app, file, function, line);
    return errorWith(app, ts, words);
}

/* The lock of drive `drive` (1 for A:). */
u32 driveLock(App* app, u32 drive)
{
    return at(app, kDriveLocks + drive * 4);
}

/* The table's slot of index `index`, as the table is now. */
u32 slotAt(App* app, u32 index)
{
    return at(app, kHandles) + (index << kSlotShift);
}

/* The index of handle `handle` (its bits flipped) if the table is made and
 * it is within it -- in use or not. */
std::optional<u32> indexOf(App* app, u32 handle)
{
    const u32 index = ~handle;
    if (at(app, kHandles) == 0 || s32(index) < 0 || s32(index) >= s32(at(app, kHandleCount)))
        return std::nullopt;
    return index;
}

/* The index of handle `handle` if it is open. */
std::optional<u32> openIndexOf(App* app, u32 handle)
{
    const std::optional<u32> index = indexOf(app, handle);
    if (!index || byte(app, slotAt(app, *index) + kInUse) == 0)
        return std::nullopt;
    return index;
}

/* The length of the string at `text`, as strlen. */
u32 lengthOf(App* app, u32 text)
{
    u32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    return length;
}

/* How much of the path at `path` is copied as it is: its ending too, no
 * more than kMaxPath. */
u32 copiedLength(App* app, u32 path)
{
    const u32 length = lengthOf(app, path) + 1;
    return length < kMaxPath ? length : kMaxPath;
}

/* The registers the game's code keeps for its caller -- all but eax and
 * esp -- put back in the thunk's args when the thunk is done. */
class Kept
{
public:
    Kept(ThreadState&, nfs::Args& args)
        : args_(args), ebx_(args.ebx), ecx_(args.ecx), edx_(args.edx), esi_(args.esi), edi_(args.edi), ebp_(args.ebp)
    {
    }
    ~Kept()
    {
        args_.ebx = ebx_, args_.ecx = ecx_, args_.edx = edx_, args_.esi = esi_, args_.edi = edi_, args_.ebp = ebp_;
    }
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;

private:
    nfs::Args& args_;
    u32 ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* A thunk's way out: eax the core's result, the return address popped
 * and `handed` bytes of stack arguments; nothing when the game stopped. */
void returns(ThreadState& ts, nfs::Args& args, const Answer& result, u32 handed = 0)
{
    if (!result)
        return;
    args.eax = *result;
    ts.sp += 4 + handed;
}

/* fileReport (0x501050, cdecl): EA's report of `level` -- none if it is
 * above the threshold at `threshold` (0: always) -- `format` and its words at
 * `arguments` formatted into the frame's 1K (vsprintf, its va_list there)
 * and sent the way [0x56789c] says: OutputDebugString, printed (Message as
 * its format, stdout flushed), or the game's log (trace); anything else
 * nowhere.  What the way leaves in eax back: the threshold's address when
 * the level is above it, the way's number when it goes nowhere. */
Answer report(App* app, ThreadState& ts, u32 threshold, u32 level, u32 format, u32 arguments)
{
    const GuestFrame frame(ts, 0xc, 0x404);
    const u32 text = frame - 0x404, list = frame - 4;
    if (threshold != 0 && s32(level) > s32(at(app, threshold)))
        return threshold;
    put(app, list, arguments);
    if (!typed<0x4df674, watcom::clib::vsprintf>(app, ts, text, format, list))
        return std::nullopt;
    const u32 way = at(app, kReportOutput);
    if (way == kToDebugger)
    {
        // The debugger's log leaves eax as the code has it before the call: the text.
        os::debugText(guest::text(app, text));
        return text;
    }
    if (way == kToStdout)
    {
        if (!guest::direct<0x4df6c0, ea::messageOf>(app, ts, text))
            return std::nullopt;
        const u32 flushed = guest::direct<0x4f7d70, watcom::clib::fflush>(app, ts, kStdout);
        if (ts.terminate)
            return std::nullopt;
        return flushed;
    }
    if (way == kToLog)
        return guest::direct<0x4e08f0, ea::monoPrinted>(app, ts, text, text, guest::Words({}));
    return way;
}

/* An error `code` reported at level 1 with `format`: fileReport called as
 * the code calls it, its four words pushed below esp. */
Answer reportError(App* app, ThreadState& ts, u32 format, u32 code)
{
    const u32 top = ts.sp, words = top - 0x10;
    put(app, words, kReportLevel);
    put(app, words + 4, 1);
    put(app, words + 8, format);
    put(app, words + 0xc, code);
    ts.sp = words;
    const Answer result = asCalled(ts, [&] { return report(app, ts, kReportLevel, 1, format, words + 0xc); });
    ts.sp = top;
    return result;
}

/* The system's error reported with `format`: the port's calls give none, 0. */
Answer reportLastError(App* app, ThreadState& ts, u32 format)
{
    return reportError(app, ts, format, 0);
}

}  // namespace

/* checkSurfaceUnlocked (0x501010): 0 -- after an Error naming function
 * `name`, files not to be touched then -- on the main thread with the check
 * on ([0x564364]) and a surface locked; else 1. */
Answer surfaceUnlocked(App* app, ThreadState& ts, u32 name)
{
    const GuestFrame frame(ts, 8, 0);
    const std::optional<bool> onMain = guest::direct<0x4df930, ea::isThreadNow>(app, ts, 0u);
    if (!onMain)
        return std::nullopt;
    if (!*onMain || at(app, kSurfaceCheck) == 0)
        return 1;
    const bool locked = guest::direct<0x4f9070, ea::surfaceIsLocked>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    if (!locked)
        return 1;
    if (!errorWith(app, ts, {kSurfaceLocked, name}))
        return std::nullopt;
    return 0;
}

namespace
{

/* closeHandleSlot (0x4e3fe0): handle `handle` closed under its drive's
 * lock -- its mapping and view (asked whatever +0x10 holds but -1), its file
 * (but -1) -- and its slot freed.  1, or 0 if it was not open. */
Answer closeSlot(App* app, ThreadState& ts, u32 handle)
{
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!os::locks::enter(app, ts, driveLock(app, drive)))
        return std::nullopt;
    if (at(app, slotAt(app, *index) + kMapping) != kNone)
    {
        os::memory::release(at(app, slotAt(app, *index) + kMemory));
        os::release(app, at(app, slotAt(app, *index) + kMapping));
    }
    const u32 file = at(app, slotAt(app, *index) + kFile);
    if (file != kNone)
        os::release(app, file);
    if (!os::locks::leave(app, ts, driveLock(app, drive)))
        return std::nullopt;
    putByte(app, slotAt(app, *index) + kInUse, 0);
    return 1;
}

/* lockHandleDrive and unlockHandleDrive: handle `handle`'s drive's lock
 * entered or left (`enter`).  1, or 0 if it is not open. */
Answer lockDriveOf(App* app, ThreadState& ts, u32 handle, bool enter)
{
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 lock = driveLock(app, byte(app, slotAt(app, *index) + kDrive));
    const bool done = enter ? os::locks::enter(app, ts, lock) : os::locks::leave(app, ts, lock);
    if (!done)
        return std::nullopt;
    return 1;
}

/* handleMemory (0x4e41f0): where handle `handle` is in memory (mapped, or a
 * buffer taken as a file); 0 if it is not, or not open. */
u32 memoryOf(App* app, u32 handle)
{
    const std::optional<u32> index = indexOf(app, handle);
    if (!index)
        return 0;
    const u32 slot = slotAt(app, *index);
    return byte(app, slot + kInUse) == 0 ? 0 : at(app, slot + kMemory);
}

}  // namespace

/* virtualAllocPages (0x501110): the port's pages for the bytes at `bytes` --
 * rounded up to whole pages there (the page's size asked the first time) --
 * read and written; their address, or 0. */
Answer allocatePages(App* app, ThreadState& ts, u32 bytes)
{
    NFS2_USE(ts);
    if (at(app, kPageSize) == 0)
        put(app, kPageSize, os::memory::kPageSize);
    const u32 page = at(app, kPageSize);
    // cdq; idiv: worked in 64 bits, as the game's code does.
    const std::int64_t dividend = s32(at(app, bytes) + page - 1);
    const u32 rounded = u32(dividend / std::int64_t(s32(page))) * page;
    put(app, bytes, rounded);
    return os::memory::allocate(app, rounded);
}

namespace
{

/* virtualFreePages (0x501180): the pages at `pages` given back; 1. */
Answer freePages(App* app, ThreadState& ts, u32 pages)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    os::memory::release(pages);
    return 1;
}

/* What touchMemoryPages leaves: in eax the pages' count (or what the last
 * call left, or the start, when there is not a page to go over); in ebx the
 * page past the last gone over (or the memory there is); in edx the last
 * count down (or the remainder, or the bytes held to the memory). */
struct Touched
{
    u32 result;
    u32 end;
    u32 left;
};

/* touchMemoryPages (0x5011a0, EA's touchmempages): the pages of `bytes`
 * from `start` -- no more than the memory there is ([0x5678a4], asked of
 * the game the first time) -- gone over to have them in memory; its loops
 * count, but touch nothing.  An Error if the page's size is not known yet
 * (initmemman not called). */
std::optional<Touched> touchPages(App* app, ThreadState& ts, u32 start, u32 bytes)
{
    const GuestFrame frame(ts, 0x10, 4);
    u32 result = start;
    if (at(app, kPageSize) == 0)
    {
        const Answer stopped = error(app, ts, kTouchmemC, kTouchMemPagesName, 0x32, {kTouchMemFirst});
        if (!stopped)
            return std::nullopt;
        result = *stopped;
    }
    if (at(app, kPhysicalMemory) == 0)
    {
        const Answer memory = guest::direct<0x5221f0, ea::physicalBytes>(app, ts);
        if (!memory)
            return std::nullopt;
        put(app, kPhysicalMemory, *memory);
        result = *memory;
    }
    u32 end = at(app, kPhysicalMemory);
    const u32 page = at(app, kPageSize);
    if (s32(bytes) > s32(end))
        bytes = end;
    if (s32(bytes) >= s32(page))
    {
        end = (start + page - 1) & ~(page - 1);
        // cdq; idiv: worked in 64 bits, as the game's code does.
        const std::int64_t dividend = s32(bytes - (end - start) + (page - 1));
        const u32 pages = u32(dividend / std::int64_t(s32(page)));
        bytes = u32(dividend % std::int64_t(s32(page)));
        result = pages;
        for (u32 i = 0; s32(i) < s32(pages); ++i)
        {
            bytes = end;
            if (end >= start)
            {
                u32 count = 0;
                do
                {
                    ++count;
                    bytes -= page;
                } while (s32(count) < 0x20 && bytes >= start);
            }
            end += page;
        }
    }
    put(app, kPageSize, page);
    return Touched{result, end, bytes};
}

/* isSlash (0x4e2eb0): whether `c` is a slash, \ or /. */
bool isSlashChar(u8 c)
{
    return c == '\\' || c == '/';
}

/* From `place` to the first slash, or to the string's last character. */
u32 toSlash(App* app, u32 place)
{
    for (;;)
    {
        if (isSlashChar(byte(app, place)))
            return place;
        const u8 next = byte(app, place + 1);
        ++place;
        if (next == 0)
            return place;
    }
}

/* volumeRoot (0x4e2ed0): the root of `path` to `root`: the path copied (at
 * most 0x104 bytes, memmove) and cut after the share of a \\server\share,
 * or else after its first slash -- or its last character, with none.  Where
 * the root ends: its ending put after it. */
std::optional<u32> rootOf(App* app, ThreadState& ts, u32 path, u32 root)
{
    putByte(app, root, 0);
    if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, root, path, copiedLength(app, path)))
        return std::nullopt;
    // A \\ looked for, a byte on at a time.
    u32 place = root;
    if (byte(app, root) != 0)
    {
        for (;;)
        {
            if (isSlashChar(byte(app, place)) && isSlashChar(byte(app, place + 1)))
                break;
            const u8 next = byte(app, place + 1);
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

/* fullPathName (0x4e2fd0): the full path of `path` to `out` (all of it,
 * whatever `room` says) -- the path in capitals where its volume does not
 * keep their case, and the file's name as the directory has it where the
 * volume keeps no Unicode names -- and 1.  The port has one volume, which
 * keeps both (os::files::kVolume).  The frame: the volume's root. */
Answer fullPath(App* app, ThreadState& ts, u32 path, u32 out, u32 room)
{
    NFS2_USE(room);
    const GuestFrame frame(ts, 0x10, 0x250);
    const u32 root = frame - 0x250 + 0x140;
    const os::files::FullPath full = os::files::fullPath(guest::text(app, path));
    guest::putText(app, out, full.path.c_str());
    if (!rootOf(app, ts, out, root))
        return std::nullopt;
    const os::files::Volume& volume = os::files::kVolume;
    if (!volume.keepsCase)
        os::text::upper(guest::memory(app, out));
    if (!volume.unicodeNames)
    {
        os::files::Found found;
        if (const auto search = os::files::findFirst(app, guest::text(app, path), found))
        {
            os::release(app, *search);
            guest::putText(app, out + full.nameAt, found.name.c_str());
        }
    }
    return 1;
}

/* driveOfPath (0x4e3120): the drive of `path`: its full path (or the
 * directory's, not found) cut to its root at `root`, looked for among
 * Windows' drives; the drive's letter, or that of the first after the last
 * there is, or 0 past Z.  Its buffers Windows' pages (virtualAllocPages, by
 * address, the frame's word their size). */
Answer driveOf(App* app, ThreadState& ts, u32 path, u32 root)
{
    const GuestFrame frame(ts, 0x14, 0xc);
    const u32 bytes = frame - 0xc;
    put(app, bytes, 0x1000);
    Answer buffer = typed<0x501110, ea::allocatePages>(app, ts, bytes);
    if (!buffer)
        return std::nullopt;
    const Answer full = asCalled(ts, [&] { return fullPath(app, ts, path, *buffer, 0xfff); });
    if (!full)
        return std::nullopt;
    if (*full == 0 && !asCalled(ts, [&] { return currentDirectory(app, ts, *buffer); }))
        return std::nullopt;
    if (!rootOf(app, ts, *buffer, root))
        return std::nullopt;
    // The port's drives' list, as long as it counts it (its letters), and their bits.
    const u32 needed = u32(std::strlen(os::files::kDriveList));
    if (s32(needed) > 0xfff)
    {
        if (!freePages(app, ts, *buffer))
            return std::nullopt;
        put(app, bytes, needed + 1);
        buffer = typed<0x501110, ea::allocatePages>(app, ts, bytes);
        if (!buffer)
            return std::nullopt;
    }
    guest::putText(app, *buffer, os::files::kDriveList);
    const u32 drives = os::files::kDriveBits;
    u32 index = 0;
    u32 name = *buffer;
    if (byte(app, name) != 0)
    {
        for (;;)
        {
            if ((drives & (1u << (index & 0x1f))) != 0)
            {
                const Answer differs = typed<0x4e9ec0, watcom::clib::stricmp>(app, ts, name, root);
                if (!differs)
                    return std::nullopt;
                if (*differs == 0)
                    break;
                name += lengthOf(app, name) + 1;
            }
            ++index;
            if (s32(index) >= 0x20 || byte(app, name) == 0)
                break;
        }
    }
    if (!freePages(app, ts, *buffer))
        return std::nullopt;
    return s32(index) >= 0x20 ? 0 : index + 'A';
}

/* driveLetter (0x4e3250): the drive letter of `path` (none: the
 * directory's, "."), from its full path -- in capitals -- if it starts X:;
 * 0 if not, or if the path has a |. */
Answer letterOf(App* app, ThreadState& ts, u32 path)
{
    for (u32 place = path;; ++place)
    {
        const u8 c = byte(app, place);
        if (c == '|')
            return 0;
        if (c == 0)
            break;
    }
    NFS2_USE(ts);
    std::string full = os::files::fullPath(guest::text(app, path != 0 ? path : kNoPath)).path;
    os::text::upper(full.data());
    if (full.size() <= 1 || full[1] != ':')
        return 0;
    return u8(full[0]);
}

/* diskFreeBytes (0x4e32e0): the bytes free on the volume whose root is at
 * `root` -- the clusters free, their sectors, the sectors' bytes: the port's
 * one volume's. */
Answer bytesFree(App* app, ThreadState& ts, u32 root)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    NFS2_USE(root);
    const os::files::Volume& volume = os::files::kVolume;
    return volume.freeClusters * volume.sectorsPerCluster * volume.bytesPerSector;
}

/* driveNumber (0x4e3330): the drive of `path` by its number, 1 for A:
 * (driveLetter); 0 for none. */
Answer numberOf(App* app, ThreadState& ts, u32 path)
{
    const Answer letter = asCalled(ts, [&] { return letterOf(app, ts, path); });
    if (!letter || *letter == 0)
        return letter;
    return *letter - 0x40;
}

/* setDirectory (0x4e3340, EA's setdirectory): the current directory set to
 * `path`; 1 (the port's never fails to be set: the Error the code has for
 * that is not reached). */
Answer changeDirectory(App* app, ThreadState& ts, u32 path)
{
    NFS2_USE(ts);
    os::files::setCurrentDirectory(guest::text(app, path));
    return 1;
}

}  // namespace

/* getDirectory (0x4e33a0, EA's getdirectory): the current directory to
 * `buffer` (0x1ff bytes there): its length, or an Error's answer if it
 * cannot be had or is longer. */
Answer currentDirectory(App* app, ThreadState& ts, u32 buffer)
{
    const GuestFrame frame(ts, 0xc, 0);
    // All of it, however long: the length says whether it fitted.
    const std::string& directory = os::files::currentDirectory();
    guest::putText(app, buffer, directory.c_str());
    const u32 length = u32(directory.size());
    if (length != 0 && length <= 0x1ff)
        return length;
    const GuestFrame saved(ts, 4, 0);  // esi pushed for the Error
    return error(app, ts, kFileC, kGetDirectoryName, 0xe9, {kGetDirectoryFailed, length});
}

namespace
{

/* closeAllHandles (0x4e34c0): the handles' table gone, if there is one:
 * every handle open closed under its lock, its pages given back, the
 * drives' locks and its own freed.  False when the game stopped. */
bool closeTable(App* app, ThreadState& ts)
{
    if (at(app, kHandles) == 0)
        return true;
    if (!os::locks::enter(app, ts, at(app, kHandlesLock)))
        return false;
    if (s32(at(app, kHandleCount)) > 0)
    {
        u32 index = 0;
        do
        {
            if (byte(app, slotAt(app, index) + kInUse) != 0 && !closeSlot(app, ts, ~index))
                return false;
            ++index;
        } while (s32(index) < s32(at(app, kHandleCount)));
    }
    if (!freePages(app, ts, at(app, kHandles)))
        return false;
    put(app, kHandles, 0);
    for (u32 offset = 0; offset != kDriveLocksBytes; offset += 4)
    {
        const u32 lock = at(app, kDriveLocks + offset);
        if (lock != 0)
        {
            if (!guest::direct<0x4eb360, ea::lockFreed>(app, ts, lock))
                return false;
            put(app, kDriveLocks + offset, 0);
        }
    }
    if (!os::locks::leave(app, ts, at(app, kHandlesLock))
        || !guest::direct<0x4eb360, ea::lockFreed>(app, ts, at(app, kHandlesLock)))
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
Answer makeTable(App* app, ThreadState& ts, u32 count, u32 eaxIn)
{
    const GuestFrame frame(ts, 0x14, 4);
    const u32 bytes = frame - 4;
    u32 eax = eaxIn;  // what the code has in eax on entry
    if (at(app, kHandlesLock) == 0)
    {
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
        if (!lock)
            return std::nullopt;
        put(app, kHandlesLock, *lock);
    }
    if (at(app, kHandles) != 0 && !asCalled(ts, [&] { return closeTable(app, ts); }))
        return std::nullopt;
    if (!os::locks::enter(app, ts, at(app, kHandlesLock)))
        return std::nullopt;
    if (count == 0)
        count = kDefaultHandles;
    put(app, bytes, count << kSlotShift);
    const Answer table = typed<0x501110, ea::allocatePages>(app, ts, bytes);
    if (!table)
        return std::nullopt;
    put(app, kHandles, *table);
    put(app, kHandleCount, count);
    if (*table != 0)
    {
        guest::direct<kFillZero, ea::clearBytes>(app, ts, *table, at(app, bytes));
        guest::direct<kFillZero, ea::clearBytes>(app, ts, kDriveLocks, kDriveLocksBytes);
        if (ts.terminate)
            return std::nullopt;
        eax = kDriveLocks + kDriveLocksBytes;  // past what the fill cleared, where it leaves eax
    }
    // LeaveCriticalSection leaves eax as it is: the count the code is called with, or where the fill stopped.
    if (!os::locks::leave(app, ts, at(app, kHandlesLock)))
        return std::nullopt;
    return eax;
}

/* takeHandleSlot (0x4e3570): a slot of the table taken, under its lock (the
 * table made first if there is none): the first not in use, cleared, in
 * use; its index -- the count of handles, none free. */
Answer takeSlot(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0);
    if (at(app, kHandles) == 0 && !asCalled(ts, [&] { return makeTable(app, ts, 0, 0); }))
        return std::nullopt;
    if (!os::locks::enter(app, ts, at(app, kHandlesLock)))
        return std::nullopt;
    const u32 count = at(app, kHandleCount);
    u32 index = 0;
    if (s32(count) > 0)
    {
        const u32 table = at(app, kHandles), end = count << kSlotShift;
        for (u32 offset = 0; byte(app, table + offset + kInUse) != 0;)
        {
            offset += kSlotBytes;
            ++index;
            if (s32(offset) >= s32(end))
                break;
        }
    }
    if (s32(index) < s32(at(app, kHandleCount)))
    {
        guest::direct<kFillZero, ea::clearBytes>(app, ts, slotAt(app, index), kSlotBytes);
        if (ts.terminate)
            return std::nullopt;
        putByte(app, slotAt(app, index) + kInUse, 1);
    }
    if (!os::locks::leave(app, ts, at(app, kHandlesLock)))
        return std::nullopt;
    return index;
}

/* deleteFile (0x4e3600): the file named `name` deleted; 1, or 0 if it
 * cannot be. */
Answer removeFile(App* app, ThreadState& ts, u32 name)
{
    NFS2_USE(ts);
    return os::files::remove(guest::text(app, name)) ? 1 : 0;
}

/* openFileHandle (0x4e3620): the file named `name` opened as `flags` say,
 * in a slot of the table (takeHandleSlot), its handle to [handleOut] (0
 * till then): under its drive's lock -- made the first time -- the file made
 * or opened, its block `block` rounded up to the volume's sectors (none
 * asked: 0x4000), its length; copied into memory if asked to be mapped
 * (0x800, kept only if it is).  1 if opened; else 0 and reported.  How it
 * is opened: +1 read, +2 written, +0x10 made, +0x20 emptied; the sharing
 * (+4, +8) and Windows' ways of buffering (0x100 to 0x400) mean nothing to
 * the port. */
Answer openFile(App* app, ThreadState& ts, u32 name, u32 flags, u32 block, u32 handleOut)
{
    const GuestFrame frame(ts, 0xc, 0x12c);
    const Answer taken = asCalled(ts, [&] { return takeSlot(app, ts); });
    if (!taken)
        return std::nullopt;
    const u32 index = *taken;
    put(app, handleOut, 0);
    if (s32(index) < 0 || s32(index) >= s32(at(app, kHandleCount)))
        return 0;
    const u8 how = u8(flags), windowsHow = u8(flags >> 8);
    using os::files::Access;
    using os::files::Create;
    // Neither read nor written is read.
    const Access access = (how & 3) == 3 ? Access::readWrite : (how & 2) ? Access::write : Access::read;
    Create creation;
    if (how & 0x10)
        creation = (how & 0x20) ? Create::createAlways : Create::createNew;
    else
        creation = (how & 0x20) ? Create::truncateExisting : Create::openExisting;
    put(app, slotAt(app, index) + kHow, flags);
    const Answer drive = asCalled(ts, [&] { return numberOf(app, ts, name); });
    if (!drive)
        return std::nullopt;
    putByte(app, slotAt(app, index) + kDrive, u8(*drive));
    if (driveLock(app, *drive) == 0)
    {
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
        if (!lock)
            return std::nullopt;
        put(app, kDriveLocks + *drive * 4, *lock);
    }
    if (!os::locks::enter(app, ts, driveLock(app, *drive)))
        return std::nullopt;
    const std::optional<os::Handle> file = os::files::open(app, guest::text(app, name), access, creation);
    const u32 slot = slotAt(app, index);
    put(app, slot + kFile, file.value_or(kNone));
    u32 opened = 0;
    if (!file)
    {
        if (!reportLastError(app, ts, kCreateFileFailed))
            return std::nullopt;
        putByte(app, slotAt(app, index) + kInUse, 0);
    }
    else
    {
        const u32 sector = block != 0 ? os::files::kVolume.bytesPerSector : 0;
        if (sector != 0)
            put(app, slot + kBlock, block > sector ? (block + sector - 1) / sector * sector : sector);
        else
            put(app, slotAt(app, index) + kBlock, kDefaultBlock);
        put(app, slotAt(app, index) + kLength, os::files::size(app, *file).value_or(0xffffffff));
        bool mapped = false;
        if (windowsHow & kMappedHigh)
        {
            const u32 mapSlot = slotAt(app, index);
            const os::files::Copy copy = os::files::copyToMemory(app, ts, *file, at(app, mapSlot + kLength));
            put(app, mapSlot + kMapping, copy.holder);
            put(app, slotAt(app, index) + kMemory, copy.memory);
            mapped = at(app, slotAt(app, index) + kMemory) != 0;
        }
        if (!mapped)
        {
            const u32 high = slotAt(app, index) + kHow + 1;
            putByte(app, high, byte(app, high) & u8(~kMappedHigh));
        }
        put(app, handleOut, index);
        put(app, handleOut, ~index);
        opened = 1;
    }
    if (!os::locks::leave(app, ts, driveLock(app, *drive)))
        return std::nullopt;
    return opened;
}

/* handleInfo (0x4e3980): what handle `handle` is: how it was opened to
 * [howOut], its block to [blockOut], its length to [lengthOut], and the
 * bytes free on its drive to [freeOut] (each pointer may be 0) -- the
 * clusters free by the sectors' bytes, not by the cluster's: the port's one
 * volume's.  1 if told, 0 if the handle is not open. */
Answer infoOf(App* app, ThreadState& ts, u32 handle, u32 howOut, u32 blockOut, u32 lengthOut, u32 freeOut)
{
    const GuestFrame frame(ts, 0xc, 0x20);
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!os::locks::enter(app, ts, driveLock(app, drive)))
        return std::nullopt;
    if (howOut != 0)
        put(app, howOut, at(app, slotAt(app, *index) + kHow));
    if (blockOut != 0)
        put(app, blockOut, at(app, slotAt(app, *index) + kBlock));
    if (lengthOut != 0)
        put(app, lengthOut, at(app, slotAt(app, *index) + kLength));
    const u32 told = 1;
    if (freeOut != 0)
        put(app, freeOut, os::files::kVolume.freeClusters * os::files::kVolume.bytesPerSector);
    if (!os::locks::leave(app, ts, driveLock(app, drive)))
        return std::nullopt;
    return told;
}

/* readHandleBytes (0x4e3ae0): `count` bytes of handle `handle` read to
 * `buffer` -- no more than are left of it -- a block at a time, from its
 * memory if it is mapped (memmove), under its drive's lock (+2 set while;
 * cleared from elsewhere, the read stops).  How many were read (0 if it is
 * not open, or reported on an error). */
Answer readBytes(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    const GuestFrame frame(ts, 0x10, 0x1c);
    u32 got = 0;  // how many a block's read took
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 offset = *index << kSlotShift;
    const u32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!os::locks::enter(app, ts, driveLock(app, drive)))
        return std::nullopt;
    u32 slot = at(app, kHandles) + offset;
    if (s32(at(app, slot + kPlace)) > s32(at(app, slot + kLength)))
        put(app, slot + kPlace, at(app, slot + kLength));
    slot = at(app, kHandles) + offset;
    if (s32(at(app, slot + kPlace) + count) > s32(at(app, slot + kLength)))
        count = at(app, slot + kLength) - at(app, slot + kPlace);
    slot = at(app, kHandles) + offset;
    u32 block = at(app, slot + kBlock);
    putByte(app, slot + kReading, 1);
    u32 total = 0;
    while (count != 0)
    {
        if (byte(app, at(app, kHandles) + offset + kReading) == 0)
            break;
        if (block > count)
            block = count;
        slot = at(app, kHandles) + offset;
        const u32 memory = at(app, slot + kMemory);
        if (memory != 0)
        {
            if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, buffer, at(app, slot + kPlace) + memory, block))
                return std::nullopt;
            got = block;
            if (!guest::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u))
                return std::nullopt;
        }
        else if (!os::files::read(app, ts, at(app, slot + kFile), guest::memory(app, buffer), block, got))
        {
            // Not a read under way (the port's never are): reported, nothing read.
            if (!reportError(app, ts, kReadFileFailed, 0))
                return std::nullopt;
            total = 0;
            break;
        }
        slot = at(app, kHandles) + offset;
        const u32 place = at(app, slot + kPlace) + got;
        put(app, slot + kPlace, place);
        total += got;
        buffer += got;
        if (s32(place) > s32(at(app, slot + kLength)))
            put(app, slot + kPlace, at(app, slot + kLength));
        if (block > got)
            break;  // fewer than asked: the end
        count -= got;
    }
    putByte(app, at(app, kHandles) + offset + kReading, 0);
    if (!os::locks::leave(app, ts, driveLock(app, drive)))
        return std::nullopt;
    return total;
}

/* writeHandleBytes (0x4e3cc0): `count` bytes at `buffer` written to handle
 * `handle` a block at a time -- to its memory if it is mapped (memmove), no
 * more than its length there -- under its drive's lock, its length grown as
 * it is written past.  How many were written (0 if it is not open, or
 * reported on an error). */
Answer writeBytes(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    const GuestFrame frame(ts, 0x10, 0x10);
    u32 wrote = 0;  // how many a block's write took
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 offset = *index << kSlotShift;
    const u32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!os::locks::enter(app, ts, driveLock(app, drive)))
        return std::nullopt;
    u32 slot = at(app, kHandles) + offset;
    if (s32(at(app, slot + kPlace)) > s32(at(app, slot + kLength)))
        put(app, slot + kPlace, at(app, slot + kLength));
    slot = at(app, kHandles) + offset;
    u32 block = at(app, slot + kBlock);
    if (at(app, slot + kMemory) != 0 && s32(at(app, slot + kPlace) + count) > s32(at(app, slot + kLength)))
        count = at(app, slot + kLength) - at(app, slot + kPlace);
    u32 total = 0;
    while (count != 0)
    {
        if (block > count)
            block = count;
        slot = at(app, kHandles) + offset;
        const u32 memory = at(app, slot + kMemory);
        if (memory != 0)
        {
            if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, memory + at(app, slot + kPlace), buffer, block))
                return std::nullopt;
            wrote = block;
            if (!guest::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u))
                return std::nullopt;
        }
        else if (!os::files::write(app, ts, at(app, slot + kFile), guest::memory(app, buffer), block, wrote))
        {
            // Not a write under way (the port's never are): reported, nothing written.
            if (!reportError(app, ts, kWriteFileFailed, 0))
                return std::nullopt;
            total = 0;
            break;
        }
        slot = at(app, kHandles) + offset;
        put(app, slot + kPlace, at(app, slot + kPlace) + wrote);
        total += wrote;
        buffer += wrote;
        if (block > wrote)
            count = 0;  // fewer than asked: no more
        else
            count -= wrote;
        slot = at(app, kHandles) + offset;
        if (s32(at(app, slot + kPlace)) > s32(at(app, slot + kLength)))
            put(app, slot + kLength, at(app, slot + kPlace));
    }
    if (!os::locks::leave(app, ts, driveLock(app, drive)))
        return std::nullopt;
    return total;
}

/* seekHandleTo (0x4e3e60): handle `handle` sought to `to`, kept within its
 * length, under its drive's lock -- the file itself too unless it is mapped;
 * then 1.  A mapped one gives back what the frame's word holds (the code's
 * local, never set).  0 if it is not open. */
Answer seekTo(App* app, ThreadState& ts, u32 handle, u32 to)
{
    const GuestFrame frame(ts, 0x10, 4);
    const u32 sought = frame - 4;
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const u32 drive = byte(app, slotAt(app, *index) + kDrive);
    if (!os::locks::enter(app, ts, driveLock(app, drive)))
        return std::nullopt;
    u32 slot = slotAt(app, *index);
    const u32 length = at(app, slot + kLength);
    put(app, slot + kPlace, to);
    if (s32(to) > s32(length))
        put(app, slot + kPlace, length);
    else if (s32(to) < 0)
        put(app, slot + kPlace, 0);
    slot = slotAt(app, *index);
    if (at(app, slot + kMemory) == 0)
    {
        const u32 place = os::files::seek(app, at(app, slot + kFile), s32(at(app, slot + kPlace)), os::files::From::start)
                              .value_or(0xffffffff);
        put(app, sought, 1);
        put(app, slotAt(app, *index) + kPlace, place);
    }
    if (!os::locks::leave(app, ts, driveLock(app, drive)))
        return std::nullopt;
    return at(app, sought);
}

/* handlePosition (0x4e3f30): where handle `handle` is: the file asked
 * unless it is mapped (an error reported, 0); 0 if it is not open. */
Answer placeOf(App* app, ThreadState& ts, u32 handle)
{
    const GuestFrame frame(ts, 0x10, 0);
    const std::optional<u32> index = openIndexOf(app, handle);
    if (!index)
        return 0;
    const GuestFrame kept(ts, 8, 0);  // edi and ecx pushed for the rest
    const u32 slot = slotAt(app, *index);
    if (at(app, slot + kMemory) == 0)
    {
        const std::optional<u32> place = os::files::seek(app, at(app, slot + kFile), 0, os::files::From::current);
        put(app, slotAt(app, *index) + kPlace, place.value_or(kNone));
        if (!place)
        {
            if (!reportLastError(app, ts, kSetFilePointerFailed))
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
Answer openMemory(App* app, ThreadState& ts, u32 memory, u32 length, u32 drive, u32 block, u32 handleOut)
{
    const GuestFrame frame(ts, 0xc, 4);
    const Answer taken = asCalled(ts, [&] { return takeSlot(app, ts); });
    if (!taken)
        return std::nullopt;
    const u32 index = *taken;
    if (s32(index) < 0 || s32(index) >= s32(at(app, kHandleCount)))
        return 0;
    if (driveLock(app, drive) == 0)
    {
        const Answer lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
        if (!lock)
            return std::nullopt;
        put(app, kDriveLocks + drive * 4, *lock);
    }
    u32 slot = slotAt(app, index);
    put(app, slot + kFile, kNone);
    put(app, slot + kHow, kMapped);
    putByte(app, slot + kDrive, u8(drive));
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
Answer closeChecked(App* app, ThreadState& ts, u32 handle)
{
    const GuestFrame frame(ts, 0x10, 0);
    where(app, kFileioC, kLibCloseHandleName, 0x19d);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, kCloseHandleName); }))
        return std::nullopt;
    return closeSlot(app, ts, handle);
}

namespace
{

using Transfer = Answer (*)(App*, ThreadState&, u32, u32, u32);

/* readHandle and writeHandle: `name` of win\fileio.c's at `line` checked
 * for a surface locked, then `count` bytes of handle `handle` read to or
 * written from `buffer`; how many. */
Answer transferChecked(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count, u32 name, u32 line,
                       Transfer transfer)
{
    const GuestFrame frame(ts, 0x10, 0);
    where(app, kFileioC, name, line);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, name); }))
        return std::nullopt;
    return asCalled(ts, [&] { return transfer(app, ts, handle, buffer, count); });
}

}  // namespace

/* seekHandle (0x4e2e60, EA's seekhandle): checked for a surface locked,
 * then handle `handle` sought to `to` (seekHandleTo); where it is then
 * (handlePosition), or 0 if it is not open. */
Answer seekChecked(App* app, ThreadState& ts, u32 handle, u32 to)
{
    const GuestFrame frame(ts, 0x10, 0);
    where(app, kFileioC, kSeekHandleName, 0x219);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, kSeekHandleName); }))
        return std::nullopt;
    const Answer sought = asCalled(ts, [&] { return seekTo(app, ts, handle, to); });
    if (!sought || *sought == 0)
        return sought;
    return asCalled(ts, [&] { return placeOf(app, ts, handle); });
}

/* openHandle (0x4e28d0, EA's openhandle): the file named `name` opened to
 * read (0x205), its handle to [handleOut], its length to [lengthOut],
 * where it starts in it to [startOut] (0; either pointer may be 0: the
 * frame's words then).  Not found, it is looked for in the archive: the
 * archive opened, its directory read in, the file looked up and sought, its
 * place in the archive to [startOut] and its length (as the lookup found
 * it) to [lengthOut].  1 if opened, else 0 -- and with [0x56440c] set, an
 * Error. */
Answer openToRead(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut)
{
    const GuestFrame frame(ts, 0xc, 0x14);
    const u32 ownLength = frame - 0x14, ownStart = ownLength + 4;
    where(app, kFileioC, kOpenHandleName, 0x9b);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, kOpenHandleName); }))
        return std::nullopt;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, lengthOut, 0);
    put(app, startOut, 0);
    const Answer opened = asCalled(ts, [&] { return openFile(app, ts, name, kOpenFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened != 0)
    {
        const Answer told = asCalled(ts, [&] { return infoOf(app, ts, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            return 1;
        if (at(app, kErrorsFatal) != 0 && !error(app, ts, kFileioC, kOpenHandleName, 0xc1, {kInfoFailed, name}))
            return std::nullopt;
        if (!closeSlot(app, ts, at(app, handleOut)))
            return std::nullopt;
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
        return 0;
    }
    if (at(app, kArchiveLookup) != 0 && byte(app, kArchiveName) != 0)
    {
        const Answer archive = asCalled(ts, [&] { return openFile(app, ts, kArchiveName, kOpenFlags, 0, handleOut); });
        if (!archive)
            return std::nullopt;
        if (*archive != 0)
        {
            const Answer read = asCalled(ts, [&] {
                return readBytes(app, ts, at(app, handleOut), at(app, kArchiveDirectory), at(app, kArchiveDirectoryBytes));
            });
            if (!read)
                return std::nullopt;
            put(app, kArchiveDirectoryBytes, *read);
            if (*read != 0)
            {
                const Answer place = call(app, ts, at(app, kArchiveLookup), {at(app, kArchiveDirectory), name});
                if (!place)
                    return std::nullopt;
                put(app, startOut, *place);
                if (*place != 0)
                {
                    const Answer sought = asCalled(ts, [&] { return seekTo(app, ts, at(app, handleOut), *place); });
                    if (!sought)
                        return std::nullopt;
                    if (*sought != 0)
                    {
                        put(app, lengthOut, at(app, at(app, kArchiveFoundLength)));
                        return 1;
                    }
                }
            }
            if (!closeSlot(app, ts, at(app, handleOut)))
                return std::nullopt;
            put(app, handleOut, 0);
        }
    }
    if (at(app, kErrorsFatal) != 0 && !error(app, ts, kFileioC, kOpenHandleName, 0xba, {kOpenFailed, name}))
        return std::nullopt;
    return 0;
}

namespace
{

/* openHandleErrorsAs (0x4e2ab0): openHandle (by address) with [0x56440c]
 * -- an Error if it fails -- as `errorsAs` says for the call. */
Answer openErrorsAs(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut,
                    u32 errorsAs)
{
    const GuestFrame frame(ts, 8, 0);
    const u32 saved = at(app, kErrorsFatal);
    put(app, kErrorsFatal, errorsAs);
    const Answer opened = typed<0x4e28d0, ea::openToRead>(app, ts, name,
                                                           handleOut, startOut, lengthOut);
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
Answer openToWrite(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut)
{
    const GuestFrame frame(ts, 0xc, 0xc);
    const u32 ownLength = frame - 0xc, ownStart = ownLength + 4;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, kWhereFile, kFileioC);
    put(app, kWhereLine, 0xdf);
    put(app, kWhereFunction, kOpenHandleWName);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, kOpenHandleWName); }))
        return std::nullopt;
    const Answer opened = asCalled(ts, [&] { return openFile(app, ts, name, kWriteFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    u32 result = 0;
    if (*opened == 0)
    {
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
    }
    else
    {
        const Answer told = asCalled(ts, [&] { return infoOf(app, ts, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            result = 1;
        else
        {
            if (!closeSlot(app, ts, at(app, handleOut)))
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
Answer openMapped(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut)
{
    const GuestFrame frame(ts, 0xc, 0x14);
    const u32 ownLength = frame - 0x14, ownStart = ownLength + 4;
    where(app, kFileioC, kOpenMapHandleName, 0x141);
    if (!asCalled(ts, [&] { return surfaceUnlocked(app, ts, kOpenHandleAName); }))
        return std::nullopt;
    if (startOut == 0)
        startOut = ownStart;
    if (lengthOut == 0)
        lengthOut = ownLength;
    put(app, lengthOut, 0);
    put(app, startOut, 0);
    const Answer opened = asCalled(ts, [&] { return openFile(app, ts, name, kMapFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened != 0)
    {
        const Answer told = asCalled(ts, [&] { return infoOf(app, ts, at(app, handleOut), 0, 0, lengthOut, 0); }, 4);
        if (!told)
            return std::nullopt;
        if (*told != 0)
            return memoryOf(app, at(app, handleOut));
        if (at(app, kErrorsFatal) != 0
            && !error(app, ts, kFileioC, kOpenMapHandleName, 0x150, {kMapInfoFailed, name}))
            return std::nullopt;
        if (!closeSlot(app, ts, at(app, handleOut)))
            return std::nullopt;
        put(app, handleOut, 0);
        put(app, lengthOut, 0);
        return 0;
    }
    if (at(app, kArchiveLookup) != 0 && byte(app, kArchiveName) != 0)
    {
        const Answer archive = asCalled(ts, [&] { return openFile(app, ts, kArchiveName, kMapFlags, 0, handleOut); });
        if (!archive)
            return std::nullopt;
        if (*archive != 0)
        {
            const u32 memory = memoryOf(app, at(app, handleOut));
            const Answer place = call(app, ts, at(app, kArchiveLookup), {memory, name});
            if (!place)
                return std::nullopt;
            put(app, startOut, *place);
            const u32 mapped = *place + memory;
            put(app, lengthOut, at(app, at(app, kArchiveFoundLength)));
            if (at(app, startOut) != 0 || at(app, kErrorsFatal) == 0)
                return mapped;
            if (!error(app, ts, kFileioC, kOpenMapHandleName, 0x15f, {kMapOpenFailed, name}))
                return std::nullopt;
            return mapped;
        }
    }
    if (at(app, kErrorsFatal) != 0 && !error(app, ts, kFileioC, kOpenMapHandleName, 0x162, {kMapOpenFailed, name}))
        return std::nullopt;
    return 0;
}

/* openBlockHandle (0x501270, EA's openblockhandle): the file named `name`
 * opened to read (0x205), its handle to [handleOut], its block to
 * [blockOut], its length to [lengthOut] and 0 to [startOut].  1 if opened,
 * else 0 -- an Error with [0x56440c] set. */
Answer openBlocks(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut, u32 blockOut)
{
    const GuestFrame frame(ts, 0xc, 4);
    const Answer opened = asCalled(ts, [&] { return openFile(app, ts, name, kOpenFlags, 0, handleOut); });
    if (!opened)
        return std::nullopt;
    if (*opened == 0)
    {
        if (at(app, kErrorsFatal) != 0
            && !error(app, ts, kBlockioC, kOpenBlockHandleName, 0x52, {kBlockOpenFailed, name}))
            return std::nullopt;
        put(app, handleOut, 0);
        return 0;
    }
    const Answer told =
        asCalled(ts, [&] { return infoOf(app, ts, at(app, handleOut), 0, blockOut, lengthOut, 0); }, 4);
    if (!told)
        return std::nullopt;
    if (*told != 0)
    {
        put(app, startOut, 0);
        return 1;
    }
    if (at(app, kErrorsFatal) != 0
        && !error(app, ts, kBlockioC, kOpenBlockHandleName, 0x58, {kBlockInfoFailed, name}))
        return std::nullopt;
    if (!closeSlot(app, ts, at(app, handleOut)))
        return std::nullopt;
    put(app, handleOut, 0);
    return 0;
}

/* blockHandleFile (0x501360, EA's blockhandlefile): not supported -- an
 * Error, and 0. */
Answer noBlockFile(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0xc, 0);
    if (!error(app, ts, kBlockioC, kBlockHandleFileName, 0x66, {kBlockFileUnsupported}))
        return std::nullopt;
    return 0;
}

/* readBlockHandle (0x5013b0, EA's readblockhandle): `blocks` blocks of
 * handle `handle` read to `buffer` (readHandleBytes), its block told into
 * the frame (handleInfo); the bytes read, 0 if the handle tells nothing (an
 * Error with [0x56440c] set). */
Answer readBlocks(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 blocks)
{
    const GuestFrame frame(ts, 0x10, 8);
    const u32 block = frame - 8;
    const Answer told = asCalled(ts, [&] { return infoOf(app, ts, handle, 0, block, 0, 0); }, 4);
    if (!told)
        return std::nullopt;
    if (*told == 0)
    {
        if (at(app, kErrorsFatal) != 0
            && !error(app, ts, kBlockioC, kReadBlockHandleName, 0xd6, {kBlockReadFailed}))
            return std::nullopt;
        return 0;
    }
    return asCalled(ts, [&] { return readBytes(app, ts, handle, buffer, at(app, block) * blocks); });
}

/* seekBlockHandle (0x501450, EA's seekblockhandle): handle `handle` sought
 * to `to` (seekHandleTo) -- an Error if it is not open -- and where it is
 * (handlePosition). */
Answer seekBlocks(App* app, ThreadState& ts, u32 handle, u32 to)
{
    const GuestFrame frame(ts, 4, 0);
    const Answer sought = asCalled(ts, [&] { return seekTo(app, ts, handle, to); });
    if (!sought)
        return std::nullopt;
    if (*sought == 0)
    {
        const GuestFrame saved(ts, 8, 0);  // esi and ebx pushed for the Error
        if (!error(app, ts, kBlockioC, kSeekBlockHandleName, 0x115, {kBlockSeekFailed}))
            return std::nullopt;
    }
    return asCalled(ts, [&] { return placeOf(app, ts, handle); });
}

}  // namespace

/* 0x4e28c0: initHandles with 0 (a jump): the table of the default 0x40.  No
 * caller in the game. */
nfs::u32 initHandlesDefault(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, makeTable(app, ts, 0, args.eax));
    return args.eax;
}

/* 0x4e28d0, EA's openhandle: openToRead of the name eax, [edx] the handle,
 * [ebx] where it starts, [ecx] its length. */
nfs::u32 openHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openToRead(app, ts, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* 0x4e2ab0: openErrorsAs of openHandle's registers and the stack's word;
 * the word taken off. */
nfs::u32 openHandleErrorsAs(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openErrorsAs(app, ts, args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4)), 4);
    return args.eax;
}

/* 0x4e2ae0, EA's openhandlew: openToWrite of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length. */
nfs::u32 openHandleToWrite(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openToWrite(app, ts, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* 0x4e2ba0, EA's openmaphandle: openMapped of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length.  No caller in the game. */
nfs::u32 openMapHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openMapped(app, ts, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* 0x4e2da0, EA's closehandle: closeChecked of handle eax. */
nfs::u32 closeHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, closeChecked(app, ts, args.eax));
    return args.eax;
}

/* readHandle's core (0x4e2de0): transferChecked of readBytes, as
 * win\fileio.c's readhandle at its line. */
Answer readChecked(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    return transferChecked(app, ts, handle, buffer, count, kReadHandleName, 0x1d4, readBytes);
}

/* writeHandle's core (0x4e2e20): transferChecked of writeBytes, as
 * win\fileio.c's writehandle at its line. */
Answer writeChecked(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    return transferChecked(app, ts, handle, buffer, count, kWriteHandleName, 0x1dd, writeBytes);
}

/* 0x4e2de0, EA's readhandle: ebx bytes of handle eax read to edx
 * (readHandleBytes); how many back. */
nfs::u32 readHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, transferChecked(app, ts, args.eax, args.edx, args.ebx, kReadHandleName, 0x1d4, readBytes));
    return args.eax;
}

/* 0x4e2e20, EA's writehandle: ebx bytes at edx written to handle eax
 * (writeHandleBytes); how many back. */
nfs::u32 writeHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, transferChecked(app, ts, args.eax, args.edx, args.ebx, kWriteHandleName, 0x1dd, writeBytes));
    return args.eax;
}

/* 0x4e2e60, EA's seekhandle: seekChecked of handle eax to edx. */
nfs::u32 seekHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, seekChecked(app, ts, args.eax, args.edx));
    return args.eax;
}

/* 0x4e2eb0: 1 if al is a slash, \ or /, else 0. */
nfs::u32 isSlash(App*, ThreadState& ts, nfs::Args& args)
{
    args.eax = isSlashChar(nfs::u8(args.eax)) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e2ebe: isSlash's way out with 0 -- an entry of its own, as handlesReady
 * jumps to it: eax 0. */
nfs::u32 notSlash(App*, ThreadState& ts, nfs::Args& args)
{
    args.eax = 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e2ed0: rootOf of path eax to edx; what it leaves in eax nobody reads
 * (ea_files.inc). */
nfs::u32 volumeRoot(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    if (rootOf(app, ts, args.eax, args.edx))
        ts.sp += 4;
    return args.eax;
}

/* 0x4e2fd0: fullPath of path eax to edx, ebx bytes there.  ecx, esi, edi
 * and ebp put where the code pushes them, above the frame, and taken back
 * from there: a root of 0x104 bytes and more, with no slash, is cut past the
 * frame's locals (volumeRoot), at the 0 of the file's part's address, and
 * its ending put in the low byte of the ebp kept there. */
nfs::u32 fullPathName(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 pushed = ts.sp - 0x10;
    put(app, pushed + 0xc, args.ecx);
    put(app, pushed + 8, args.esi);
    put(app, pushed + 4, args.edi);
    put(app, pushed, args.ebp);
    Answer found;
    {
        const Kept kept(ts, args);
        found = fullPath(app, ts, args.eax, args.edx, args.ebx);
    }
    if (!found)
        return args.eax;
    args.ebp = at(app, pushed);
    args.edi = at(app, pushed + 4);
    args.esi = at(app, pushed + 8);
    args.ecx = at(app, pushed + 0xc);
    returns(ts, args, found);
    return args.eax;
}

/* 0x4e3110: the drives' bits.  No caller in the game. */
nfs::u32 logicalDrives(App* app, ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    const Kept kept(ts, args);
    returns(ts, args, os::files::kDriveBits);
    return args.eax;
}

/* 0x4e3120: driveOf of path eax, its root to edx.  No caller in the game. */
nfs::u32 driveOfPath(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, driveOf(app, ts, args.eax, args.edx));
    return args.eax;
}

/* 0x4e3250: letterOf of path eax. */
nfs::u32 driveLetter(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, letterOf(app, ts, args.eax));
    return args.eax;
}

/* 0x4e32e0: bytesFree of the root eax.  No caller in the game. */
nfs::u32 diskFreeBytes(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, bytesFree(app, ts, args.eax));
    return args.eax;
}

/* 0x4e3330: numberOf of path eax. */
nfs::u32 driveNumber(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, numberOf(app, ts, args.eax));
    return args.eax;
}

/* 0x4e3340, EA's setdirectory: changeDirectory to path eax. */
nfs::u32 setDirectory(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, changeDirectory(app, ts, args.eax));
    return args.eax;
}

/* 0x4e33a0, EA's getdirectory: currentDirectory to eax. */
nfs::u32 getDirectory(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, currentDirectory(app, ts, args.eax));
    return args.eax;
}

/* 0x4e3410: 1 if the handles' table is made, else 0 (notSlash's way out).
 * No caller in the game. */
nfs::u32 handlesReady(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = at(app, kHandles) != 0 ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e3430: makeTable for eax handles. */
nfs::u32 initHandles(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, makeTable(app, ts, args.eax, args.eax));
    return args.eax;
}

/* 0x4e34c0: closeTable; what it leaves in eax nobody reads
 * (ea_files.inc). */
nfs::u32 closeAllHandles(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    if (closeTable(app, ts))
        ts.sp += 4;
    return args.eax;
}

/* 0x4e3570: takeSlot; the slot's index back. */
nfs::u32 takeHandleSlot(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, takeSlot(app, ts));
    return args.eax;
}

/* 0x4e3600: removeFile of the name eax. */
nfs::u32 deleteFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, removeFile(app, ts, args.eax));
    return args.eax;
}

/* 0x4e3620: openFile of the name eax as the flags edx, block ebx, [ecx] the
 * handle. */
nfs::u32 openFileHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openFile(app, ts, args.eax, args.edx, args.ebx, args.ecx));
    return args.eax;
}

/* 0x4e3980: infoOf of handle eax, to [edx], [ebx], [ecx] and the stack's
 * word; the word taken off. */
nfs::u32 handleInfo(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, infoOf(app, ts, args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4)), 4);
    return args.eax;
}

/* 0x4e3ae0: readBytes, ebx bytes of handle eax to edx. */
nfs::u32 readHandleBytes(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, readBytes(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4e3cc0: writeBytes, ebx bytes at edx to handle eax. */
nfs::u32 writeHandleBytes(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, writeBytes(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x4e3e60: seekTo, handle eax to edx. */
nfs::u32 seekHandleTo(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, seekTo(app, ts, args.eax, args.edx));
    return args.eax;
}

/* 0x4e3f30: placeOf handle eax. */
nfs::u32 handlePosition(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, placeOf(app, ts, args.eax));
    return args.eax;
}

/* 0x4e3fe0: closeSlot of handle eax. */
nfs::u32 closeHandleSlot(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, closeSlot(app, ts, args.eax));
    return args.eax;
}

/* 0x4e40a0: handle eax's drive's lock entered; 1, or 0 if it is not
 * open. */
nfs::u32 lockHandleDrive(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, lockDriveOf(app, ts, args.eax, true));
    return args.eax;
}

/* 0x4e40f0: handle eax's drive's lock left; 1, or 0 if it is not open. */
nfs::u32 unlockHandleDrive(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, lockDriveOf(app, ts, args.eax, false));
    return args.eax;
}

/* 0x4e4140: openMemory of edx bytes at eax, drive ebx, block ecx, the
 * handle to the stack's word; the word taken off. */
nfs::u32 openMemoryHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openMemory(app, ts, args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4)), 4);
    return args.eax;
}

std::optional<u32> handleDriveLocked(App* app, ThreadState& ts, u32 handle)
{
    return lockDriveOf(app, ts, handle, true);
}

std::optional<u32> handleDriveUnlocked(App* app, ThreadState& ts, u32 handle)
{
    return lockDriveOf(app, ts, handle, false);
}

std::optional<u32> memoryOpened(App* app, ThreadState& ts, u32 memory, u32 length, u32 drive, u32 block,
                                  u32 handleOut)
{
    return openMemory(app, ts, memory, length, drive, block, handleOut);
}

/* 0x4e41f0: memoryOf handle eax. */
nfs::u32 handleMemory(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = memoryOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x501010: surfaceUnlocked for the function named eax. */
nfs::u32 checkSurfaceUnlocked(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, surfaceUnlocked(app, ts, args.eax));
    return args.eax;
}

/* 0x501050, cdecl (threshold's address, level, format, ...): report. */
nfs::u32 fileReport(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    const u32 words = ts.sp + 4;
    returns(ts, args, report(app, ts, at(app, words), at(app, words + 4), at(app, words + 8), words + 0xc));
    return args.eax;
}

/* 0x501110: allocatePages for the bytes [eax]. */
nfs::u32 virtualAllocPages(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, allocatePages(app, ts, args.eax));
    return args.eax;
}

/* 0x501180: freePages at eax. */
nfs::u32 virtualFreePages(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, freePages(app, ts, args.eax));
    return args.eax;
}

/* 0x5011a0, EA's touchmempages: touchPages of edx bytes from eax; ebx and
 * edx left as its loops leave them. */
nfs::u32 touchMemoryPages(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 start = args.eax, bytes = args.edx;
    std::optional<Touched> touched;
    {
        const Kept kept(ts, args);
        touched = touchPages(app, ts, start, bytes);
    }
    if (!touched)
        return args.eax;
    args.eax = touched->result;
    args.ebx = touched->end;
    args.edx = touched->left;
    ts.sp += 4;
    return args.eax;
}

/* 0x501270, EA's openblockhandle: openBlocks of the name eax, [edx] the
 * handle, [ebx] where it starts, [ecx] its length, the block to the stack's
 * word; the word taken off. */
nfs::u32 openBlockHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, openBlocks(app, ts, args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4)), 4);
    return args.eax;
}

/* 0x501360, EA's blockhandlefile: noBlockFile.  No caller in the game. */
nfs::u32 blockHandleFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, noBlockFile(app, ts));
    return args.eax;
}

/* 0x5013a0, EA's closeblockhandle: closeHandleSlot, jumped to. */
nfs::u32 closeBlockHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    return closeHandleSlot(app, ts, args);
}

/* 0x5013b0, EA's readblockhandle: readBlocks, ebx blocks of handle eax to
 * edx. */
nfs::u32 readBlockHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, readBlocks(app, ts, args.eax, args.edx, args.ebx));
    return args.eax;
}

/* 0x501450, EA's seekblockhandle: seekBlocks of handle eax to edx.  No
 * caller in the game. */
nfs::u32 seekBlockHandle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Kept kept(ts, args);
    returns(ts, args, seekBlocks(app, ts, args.eax, args.edx));
    return args.eax;
}

/* 0x501180's and 0x501050's typed cores (native_ea_files.h): as their thunks run them -- the
 * report's words pushed below esp as its caller pushes them, its thunk run on them. */
std::optional<u32> pagesFreed(App* app, ThreadState& ts, u32 pages)
{
    return freePages(app, ts, pages);
}

std::optional<u32> driveNumberOf(App* app, ThreadState& ts, u32 path)
{
    return numberOf(app, ts, path);
}

std::optional<u32> reportedAt(App* app, ThreadState& ts, u32 threshold, u32 level, u32 format, guest::Words words)
{
    // The words pushed as the caller pushes them, the report's own frame below.
    const u32 top = ts.sp, base = top - 4 * (3 + words.count);
    put(app, base, threshold);
    put(app, base + 4, level);
    put(app, base + 8, format);
    for (unsigned i = 0; i < words.count; ++i)
        put(app, base + 0xc + 4 * i, words.word[i]);
    ts.sp = base;
    const Answer result = asCalled(ts, [&] { return report(app, ts, threshold, level, format, base + 0xc); });
    ts.sp = top;
    return result;
}

/* 0x5011a0's, 0x501270's, 0x4e3fe0's (0x5013a0 jumps to it), 0x5013b0's and 0x501450's typed cores
 * (native_ea_files.h): as their thunks run them. */
std::optional<u32> errorsAsOpened(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut,
                                    u32 lengthOut, u32 errorsAs)
{
    return openErrorsAs(app, ts, name, handleOut, startOut, lengthOut, errorsAs);
}

std::optional<u32> fileHandleOpened(App* app, ThreadState& ts, u32 name, u32 flags, u32 block, u32 handleOut)
{
    return openFile(app, ts, name, flags, block, handleOut);
}

std::optional<u32> handleSought(App* app, ThreadState& ts, u32 handle, u32 to)
{
    return seekTo(app, ts, handle, to);
}

std::optional<u32> bytesRead(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    return readBytes(app, ts, handle, buffer, count);
}

std::optional<u32> bytesWritten(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 count)
{
    return writeBytes(app, ts, handle, buffer, count);
}

std::optional<u32> handleInfoOf(App* app, ThreadState& ts, u32 handle, u32 howOut, u32 blockOut, u32 lengthOut,
                                  u32 freeOut)
{
    return infoOf(app, ts, handle, howOut, blockOut, lengthOut, freeOut);
}

u32 handleMemoryOf(App* app, u32 handle)
{
    return memoryOf(app, handle);
}

std::optional<u32> fileRemoved(App* app, ThreadState& ts, u32 name)
{
    return removeFile(app, ts, name);
}

std::optional<u32> handlesMade(App* app, ThreadState& ts, u32 count)
{
    return makeTable(app, ts, count, count);
}

std::optional<u32> memoryTouched(App* app, ThreadState& ts, u32 start, u32 bytes)
{
    const auto touched = touchPages(app, ts, start, bytes);
    if (!touched)
        return std::nullopt;
    return touched->result;
}

std::optional<u32> blockOpened(App* app, ThreadState& ts, u32 name, u32 handleOut, u32 startOut, u32 lengthOut,
                                 u32 blockOut)
{
    return openBlocks(app, ts, name, handleOut, startOut, lengthOut, blockOut);
}

std::optional<u32> slotClosed(App* app, ThreadState& ts, u32 handle)
{
    return closeSlot(app, ts, handle);
}

std::optional<u32> blocksRead(App* app, ThreadState& ts, u32 handle, u32 buffer, u32 blocks)
{
    return readBlocks(app, ts, handle, buffer, blocks);
}

std::optional<u32> blocksSought(App* app, ThreadState& ts, u32 handle, u32 to)
{
    return seekBlocks(app, ts, handle, to);
}

}  // namespace ea
}  // namespace nfs3hp
