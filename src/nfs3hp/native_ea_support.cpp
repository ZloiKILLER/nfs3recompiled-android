#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "os_threads.h"
#include <lib/app.h>

#include <optional>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* EA's trace and log, files loaded whole into memory, and .viv archives.  Each
 * address is bound to the function of that name here (EA_SUPPORT_NATIVES), and the
 * game's code calls it by name.
 *
 * The trace: EA's monochrome monitor, text written straight into a
 * second display's text page of 80 by 25 cells of two bytes (the character,
 * its attribute) -- the page at 0x5644d0 (0xb0000 + 0x8000 a page, as
 * monoPage sets it), the cursor's byte offset into it at 0x5644d4, the top
 * row of the scrolling window at 0x5644d8, the attribute written at 0x5644d9,
 * the tab's width at 0x5644da.  It only writes while the trace's level
 * (0x5644c8) is 2 or more, under a lock of its own (0x5644c4, made as it is
 * first wanted, 0x4eb2e0).  The text understands backspace, tab, newline
 * (scrolling the window at the bottom), 0x0b (to the window's top) and 0x0c
 * (the page cleared), and 0x0e, the attribute in the byte after it.  The
 * game's own trace (0x42d510) was compiled out: an empty function.
 *
 * Files loaded whole (cmn\loadfile.c, loadfileadr): a file of the game's
 * (0x4e28d0 opens it, by name, in an archive or not, its size and handle) read
 * into a block allocated for it, the sound's stream topped up first; the
 * load's hook (0x567868), if one is set, may make another block of it.
 * Whether a file is there asks the same open, its complaint at a missing file
 * (0x56440c) off.
 *
 * A .viv archive ("BIGF", big-endian): its header -- 'BIGF', its size, the
 * files' count at 8, the header's size at 0xc -- and after it, from 0x10, each
 * file's offset and size and its name, ended.  openViv reads the header whole
 * into a block (byte-swapping the first, count and size words), findInViv
 * looks a file's name, without its path, up in it.
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * the registers as the game's code does.  Other files' typed functions
 * whose callers take nothing of them but their answer -- the trace's lock
 * made, the window's rows moved, a file opened and closed to see it is there,
 * a loaded file closed -- are called as C++ (guest::direct, their headers).
 * Every other function of the game's, and Windows' through its imports, is
 * called by address, with its arguments in the registers (Watcom's eax, edx,
 * ebx, ecx) and on the guest's stack as the code hands them over -- among
 * them the archive's block reserved and freed, and a loaded file's open,
 * block, read and free, whose edx (and ebx) the code leaves as they leave
 * them, which their headers do not give back; a core keeps the registers its
 * code pushes and pops around them (Kept), and leaves the rest as the calls
 * leave them, as the code does.  A frame whose address a callee is handed stays on
 * the guest's stack at the code's depth (GuestFrame). */
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
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The trace's: its level, its lock; the monitor's page, the cursor, the
// window's top row, the attribute, the tab's width.
constexpr u32 kTraceLevel = 0x5644c8, kTraceLock = 0x5644c4;
constexpr u32 kMonoPage = 0x5644d0, kMonoCursor = 0x5644d4;
constexpr u32 kMonoTop = 0x5644d8, kMonoAttribute = 0x5644d9, kMonoTab = 0x5644da;
constexpr u32 kRowBytes = 0xa0, kPageBytes = 0xfa0, kLastRow = 0xf00;
// The page's rows and columns; the level the trace writes from; the page's
// place (0xb0000, 0x8000 a page).
constexpr s32 kRows = 0x19, kColumns = 0x50, kTraceWrites = 2;
constexpr u32 kMonoMemory = 0xb0000, kMonoPageShift = 0xf;
// The text's controls: backspace, tab, newline, the window's top, the page
// cleared (8 to 0xc, by the code's table at 0x4e0a48); the attribute's
// escape; the first byte written as it is.
constexpr u8 kBackspace = 8, kLastControl = 4, kAttributeEscape = 0x0e, kFirstPrinted = 0x20;
// EA's random numbers' six words (snd::eaRandom draws from them).
constexpr u32 kRandom = 0x5644e0;
// The loads': whether a missing file is complained of; the last file's size;
// the load's hook.
constexpr u32 kComplain = 0x56440c, kLoadedSize = 0x5643f8;
constexpr u32 kLoadHook = 0x567868;
constexpr u32 kWhere = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr u32 kLoadFile = 0x549928, kLoadFunction = 0x549938, kLoadLine = 0x4b;
// The archive's: fopen's mode "rb", the block's name ""; the header's words
// (its size at 0xc, the files' count at 8, the entries from 0x10, each its
// offset, size and name); the block's slack and its allocation's flags;
// fseek's SEEK_SET; the name's length looked for.
constexpr u32 kReadBinary = 0x5376e4, kNoName = 0x537690;
constexpr u32 kVivCount = 8, kVivHeaderSize = 0xc, kVivEntries = 0x10, kVivEntryName = 8;
constexpr u32 kVivSlack = 0x30, kVivBlockFlags = 0x10, kSeekSet = 0, kVivNameBytes = 0x1d;
// What is called by address.
constexpr u32 kFillZero = 0x4e070c;
constexpr u32 kOpen = 0x4e28d0, kRead = 0x4e2de0;
constexpr u32 kAllocate = 0x4e1620, kFree = 0x4e1890, kLoadWhole = 0x4e0ec0;

/* Room on the guest's stack for what a callee is handed the address of: the
 * code's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after (as
 * native_collide.cpp's). */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    ~GuestFrame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* `Core`, the game's `Address`, called as C++ (guest::direct): its result;
 * nullopt when the game stopped in it. */
template <u32 Address, auto Core, typename... Args>
std::optional<u32> loaded(App* app, ThreadState& ts, Args... arguments)
{
    const auto result = guest::direct<Address, Core>(app, ts, u32(arguments)...);
    if (ts.terminate)
        return std::nullopt;
    return u32(result);
}

/* Where the game's fill (0x4e070c) leaves eax, `count` bytes at `at`
 * cleared: past them -- but for a last odd one, which it ends on -- after
 * its steps up to an 8-byte boundary; where it was for a count below 0. */
u32 fillEnd(u32 at, s32 count)
{
    if (at & 7)
    {
        if ((at & 1) && count >= 1)
        {
            at += 1;
            count -= 1;
        }
        if ((at & 2) && count >= 2)
        {
            at += 2;
            count -= 2;
        }
        if ((at & 4) && count >= 4)
        {
            at += 4;
            count -= 4;
        }
    }
    if (count < 0)
        return at;
    return at + u32(count) - (u32(count) & 1);
}

/* The trace's lock, made the first time it is wanted (native_ea_threads.h's
 * newLock): what the code has in eax after, `eax` handed (the lock, if made).
 * nullopt when the game stopped. */
std::optional<u32> traceLockMade(App* app, ThreadState& ts, u32 eax)
{
    if (at(app, kTraceLock) != 0)
        return eax;
    const std::optional<u32> lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
    if (!lock)
        return std::nullopt;
    put(app, kTraceLock, *lock);
    return *lock;
}

/* The trace's lock taken or given back (os::locks), handed `eax`: what it leaves in eax -- the
 * lock's functions leave it as they are handed it.  nullopt when the game stopped. */
std::optional<u32> traceEnter(App* app, ThreadState& ts, u32 eax)
{
    if (!os::locks::enter(app, ts, at(app, kTraceLock)))
        return std::nullopt;
    return eax;
}

std::optional<u32> traceLeave(App* app, ThreadState& ts, u32 eax)
{
    if (!os::locks::leave(app, ts, at(app, kTraceLock)))
        return std::nullopt;
    return eax;
}

/* A word of an archive's, big-endian, as the host has it. */
u32 bigEndian(u32 word)
{
    return (word & 0xff) << 24 | (word & 0xff00) << 8 | (word & 0xff0000) >> 8 | (word & 0xff000000) >> 24;
}

}  // namespace

/* The .viv archive named `name` opened, its FILE to [`file`]: its header's
 * size (big-endian at 0xc) read into the frame (under ebx, ecx, esi, edi and
 * ebp pushed), a block of it and 0x30 more allocated and cleared, the header
 * read into it whole and its first word, the files' count and the header's
 * size byte-swapped.  The block, or 0 -- the file closed and [`file`] 0 when
 * there is no block. */
std::optional<u32> vivOpened(App* app, ThreadState& ts, u32 name, u32 file)
{
    const GuestFrame frame(ts, 0x14, 4);
    const u32 sizeAt = frame - 4;
    const auto stream = loaded<0x4edfe8, watcom::clib::fopen>(app, ts, name, kReadBinary);
    if (!stream)
        return std::nullopt;
    put(app, file, *stream);
    if (*stream == 0)
        return 0;
    if (!loaded<0x4e9088, watcom::clib::fseek>(app, ts, *stream, kVivHeaderSize, kSeekSet)
        || !loaded<0x4e9200, watcom::clib::fread>(app, ts, sizeAt, 4u, 1u, at(app, file)))
        return std::nullopt;
    const u32 bytes = bigEndian(at(app, sizeAt)) + kVivSlack;
    const auto block = guest::direct<kAllocate, ea::reserved>(app, ts, kNoName, bytes, kVivBlockFlags);
    if (!block)
        return std::nullopt;
    if (*block == 0)
    {
        if (!loaded<0x4ee100, watcom::clib::fclose>(app, ts, at(app, file)))
            return std::nullopt;
        put(app, file, 0);
        return 0;
    }
    if (!loaded<0x4e0640, watcom::clib::memset>(app, ts, *block, 0u, bytes)
        || !loaded<0x4e9088, watcom::clib::fseek>(app, ts, at(app, file), 0u, kSeekSet)
        || !loaded<0x4e9200, watcom::clib::fread>(app, ts, *block, bytes, 1u, at(app, file)))
        return std::nullopt;
    put(app, *block, bigEndian(at(app, *block)));
    put(app, *block + kVivHeaderSize, bigEndian(at(app, *block + kVivHeaderSize)));
    put(app, *block + kVivCount, bigEndian(at(app, *block + kVivCount)));
    return *block;
}

/* The archive's block `block` freed and its FILE at [`file`] closed, [`file`]
 * 0: what fclose gives back. */
std::optional<u32> vivClosed(App* app, ThreadState& ts, u32 block, u32 file)
{
    if (!guest::direct<kFree, ea::purgeBlock>(app, ts, block))
        return std::nullopt;
    const auto closed = loaded<0x4ee100, watcom::clib::fclose>(app, ts, at(app, file));
    if (!closed)
        return std::nullopt;
    put(app, file, 0);
    return *closed;
}

namespace
{

/* The file named `wanted` (case aside, _stricmp) looked for among the
 * entries of the archive's header `header`.  Found, its offset byte-swapped
 * to [`offsetTo`] and its size to [`sizeTo`]; else both 0. */
std::optional<VivSearch> vivEntry(App* app, ThreadState& ts, u32 header, u32 wanted, u32 offsetTo, u32 sizeTo)
{
    u32 entry = header + kVivEntries;
    for (u32 index = 0; index < at(app, header + kVivCount); ++index)
    {
        const u32 name = entry + kVivEntryName;
        const auto differs = loaded<0x4e9ec0, watcom::clib::stricmp>(app, ts, wanted, name);
        if (!differs)
            return std::nullopt;
        if (*differs == 0)
        {
            const u32 sizeWord = at(app, entry + 4);
            put(app, offsetTo, bigEndian(at(app, entry)));
            put(app, sizeTo, bigEndian(sizeWord));
            return VivSearch{entry, true, sizeWord};
        }
        entry = name;
        while (byte(app, entry) != 0)
            ++entry;
        ++entry;
    }
    put(app, offsetTo, 0);
    put(app, sizeTo, 0);
    return VivSearch{entry, false, 0};
}

}  // namespace

/* The file named at `path` -- after its last backslash, 0x1d bytes of it,
 * copied into the frame (under esi, edi and ebp pushed) -- looked up in the
 * archive's header `header` (vivEntry). */
std::optional<VivSearch> vivFile(App* app, ThreadState& ts, u32 header, u32 path, u32 offsetTo, u32 sizeTo)
{
    const GuestFrame frame(ts, 0xc, 0x24);
    const u32 name = frame - 0x24;
    const auto slash = loaded<0x4ee2f0, watcom::clib::strrchr>(app, ts, path, u32('\\'));
    if (!slash || !loaded<0x4e0e30, watcom::clib::strncpy>(app, ts, name, *slash != 0 ? *slash + 1 : path, kVivNameBytes))
        return std::nullopt;
    return vivEntry(app, ts, header, name, offsetTo, sizeTo);
}

namespace
{

/* The window's rows below its top moved up one (native_ea_memory.h's moveMemory),
 * the last cleared (0x4e070c), the cursor at its start -- once the cursor is
 * past the page.  `left` what the code leaves in eax: the clearing's.  False
 * when the game stopped. */
bool scrolled(App* app, ThreadState& ts, u32& left)
{
    if (s32(at(app, kMonoCursor)) < s32(kPageBytes))
        return true;
    const u32 top = byte(app, kMonoTop);
    const u32 page = at(app, kMonoPage);
    if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, page + top * kRowBytes, page + (top + 1) * kRowBytes,
                                                 (kRows - 1 - top) * kRowBytes))
        return false;
    const u32 last = at(app, kMonoPage) + kLastRow;
    guest::direct<kFillZero, ea::clearBytes>(app, ts, last, kRowBytes);
    if (ts.terminate)
        return false;
    put(app, kMonoCursor, kLastRow);
    left = fillEnd(last, s32(kRowBytes));
    return true;
}

/* The monitor's page cleared, a dword at a time, under the trace's lock,
 * while the trace's level is 2 or more.  The cursor stays.  `left` what the
 * code leaves in eax, handed over as it is (the lock's calls give it back as
 * they find it, now and then): what the lock's leave gives back, eax the
 * clearing's end before it.  False when the game stopped. */
bool pageCleared(App* app, ThreadState& ts, u32& left)
{
    if (s32(at(app, kTraceLevel)) < kTraceWrites)
        return true;
    const std::optional<u32> made = traceLockMade(app, ts, left);
    if (!made || !traceEnter(app, ts, *made))
        return false;
    for (u32 offset = 0; offset < kPageBytes; offset += 4)
        put(app, at(app, kMonoPage) + offset, 0);
    const auto given = traceLeave(app, ts, kPageBytes);
    if (!given)
        return false;
    left = *given;
    return true;
}

/* The text at `text` written at the monitor's cursor, while the trace's
 * level is 2 or more: each byte from 0x20 a cell in the attribute (the window
 * scrolled first if the cursor is past the page); 8 a cell back, 9 on to the
 * next tab, 0xa the next row's start (scrolled), 0xb the window's top, 0xc
 * the page cleared and the window's top; 0x0e the next byte the attribute.
 * What the code leaves in eax back (`left`): each
 * cell's word summed in eax out of what eax and edx held before, their upper
 * halves with it (`carried`, edx's), and each control's own. */
std::optional<u32> monoText(App* app, ThreadState& ts, u32 text)
{
    if (s32(at(app, kTraceLevel)) < kTraceWrites)
        return text;
    u32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    u32 left = 0;
    u32 carried = text;
    bool attributeNext = false;
    for (u32 i = 0; s32(i) < s32(length); ++i)
    {
        const u8 c = byte(app, text + i);
        if (attributeNext)
        {
            left = c;
            attributeNext = false;
            putByte(app, kMonoAttribute, c);
            continue;
        }
        attributeNext = c == kAttributeEscape;
        const u32 cursor = at(app, kMonoCursor);
        if (c >= kFirstPrinted)
        {
            if (!scrolled(app, ts, left))
                return std::nullopt;
            const u32 next = at(app, kMonoCursor) + 2;
            left = ((((left & 0xff0000) << 8) | u32(byte(app, kMonoAttribute)) << 8) + ((carried & 0xffff0000) | c));
            carried = at(app, kMonoPage);
            put(app, kMonoCursor, next);
            app->getMemory<u16>(carried + next - 2) = u16(left);
            continue;
        }
        const u8 control = u8(c - kBackspace);
        if (control > kLastControl)
        {
            left = (left & 0xffffff00) | control;
            continue;
        }
        left = control;
        switch (control)
        {
        case 0:  // backspace
            if (s32(cursor) >= 2)
                put(app, kMonoCursor, cursor - 2);
            break;
        case 1:  // tab: the column on to the next multiple of the tab's width
        {
            const u32 tab = byte(app, kMonoTab);
            if (tab <= 1)
                break;
            const s32 rowStart = s32(cursor) / s32(kRowBytes) * s32(kRowBytes);
            const s32 column = (s32(cursor) - rowStart) / 2;
            left = u32((column / s32(tab) + 1) * s32(tab) * 2);
            carried = u32(column % s32(tab));
            put(app, kMonoCursor, left + u32(rowStart));
            break;
        }
        case 2:  // newline: the next row's start, the window scrolled at the bottom
            left = u32(s32(cursor) / s32(kRowBytes));
            carried = u32(s32(cursor) % s32(kRowBytes));
            put(app, kMonoCursor, (left + 1) * kRowBytes);
            if (!scrolled(app, ts, left))
                return std::nullopt;
            break;
        case 3:  // the window's top
            put(app, kMonoCursor, byte(app, kMonoTop) * kRowBytes);
            break;
        case 4:  // the page cleared, the window's top
            if (!pageCleared(app, ts, left))
                return std::nullopt;
            put(app, kMonoCursor, byte(app, kMonoTop) * kRowBytes);
            break;
        }
    }
    return left;
}

/* EA's trace, printf's way, the format at `format` and its arguments at
 * `arguments`, written to the monitor (monoText) under the trace's lock --
 * formatted (vsprintf: eax the buffer, edx the format, ebx the va_list's
 * address) into the 0x100 bytes of the frame, the va_list's slot above it.
 * `eax` what the code has in eax on entry.  What the lock's leave leaves
 * in eax, handed what monoText leaves; nullopt when the game stopped. */
std::optional<u32> tracePrinted(App* app, ThreadState& ts, u32 eax, u32 format, u32 arguments)
{
    const GuestFrame frame(ts, 0, 0x104);
    const u32 list = frame - 4, buffer = frame - 0x104;
    const std::optional<u32> made = traceLockMade(app, ts, eax);
    if (!made || !traceEnter(app, ts, *made))
        return std::nullopt;
    put(app, list, arguments);
    if (!loaded<0x4df674, watcom::clib::vsprintf>(app, ts, buffer, format, list))
        return std::nullopt;
    const auto left = monoText(app, ts, buffer);
    if (!left)
        return std::nullopt;
    return traceLeave(app, ts, *left);
}

/* The cursor moved to `column` and `row`, if they are on the page. */
void cursorTo(App* app, u32 column, u32 row)
{
    if (s32(column) >= 0 && s32(column) < kColumns && s32(row) >= 0 && s32(row) < kRows)
        put(app, kMonoCursor, (row * u32(kColumns) + column) * 2);
}

/* 0x4e0980's: tracePrinted at `column` and `row` (the cursor moved there,
 * once the lock is taken, if they are on the page), the format's arguments at
 * `arguments`; the frame under ebx, esi and ebp pushed (and the return
 * address's slot at esp).  What the code leaves in eax: `eax` as it is handed
 * while the trace's level is under 2, else what the lock's leave leaves,
 * the lock handed in it.  nullopt when the game stopped. */
std::optional<u32> printedAt(App* app, ThreadState& ts, u32 eax, u32 column, u32 row, u32 format,
                               u32 arguments)
{
    if (s32(at(app, kTraceLevel)) < kTraceWrites)
        return eax;
    // The cursor is moved after the lock is taken: tracePrinted's own steps,
    // with the move between them.
    const GuestFrame frame(ts, 0xc, 0x104);
    const u32 list = frame - 4, buffer = frame - 0x104;
    const std::optional<u32> made = traceLockMade(app, ts, eax);
    if (!made || !traceEnter(app, ts, *made))
        return std::nullopt;
    cursorTo(app, column, row);
    put(app, list, arguments);
    if (!loaded<0x4df674, watcom::clib::vsprintf>(app, ts, buffer, format, list) || !monoText(app, ts, buffer))
        return std::nullopt;
    return traceLeave(app, ts, at(app, kTraceLock));
}

/* The scrolling window's top row `row` (0 to 24, else as it was), under the
 * trace's lock: what the lock's leave gives back, handed what
 * the lock's enter did (the row, or the lock if it was made). */
std::optional<u32> windowTopSet(App* app, ThreadState& ts, u32 row)
{
    const std::optional<u32> made = traceLockMade(app, ts, row);
    if (!made)
        return std::nullopt;
    const std::optional<u32> entered = traceEnter(app, ts, *made);
    if (!entered)
        return std::nullopt;
    if (s32(row) >= 0 && s32(row) < kRows)
        putByte(app, kMonoTop, u8(row));
    return traceLeave(app, ts, *entered);
}

/* The monitor's page `page`: 0xb0000 + page * 0x8000, kept and back. */
u32 monitorPage(App* app, u32 page)
{
    const u32 address = (page << kMonoPageShift) + kMonoMemory;
    put(app, kMonoPage, address);
    return address;
}

}  // namespace

/* 0x4e0980: printedAt, its column, row, format and the format's arguments
 * (`words`) where the game's call pushes them: called as C++ (guest::direct),
 * the return address's slot already under esp, they go above a slot of their
 * own under them, as the game's call has them. */
std::optional<u32> monoPrintedAt(App* app, ThreadState& ts, u32 eax, u32 column, u32 row, u32 format,
                                   guest::Words words)
{
    const u32 esp = ts.sp;
    const u32 slot = esp + 4 - 4 * (4 + words.count);
    put(app, slot + 4, column);
    put(app, slot + 8, row);
    put(app, slot + 0xc, format);
    for (unsigned i = 0; i < words.count; ++i)
        put(app, slot + 0x10 + 4 * i, words.word[i]);
    ts.sp = slot;
    const std::optional<u32> left = printedAt(app, ts, eax, column, row, format, slot + 0x10);
    ts.sp = esp;
    return left;
}

/* 0x4e08f0: EA's trace, printf's way, `format` and its arguments (`words`) where the game's call pushes them
 * (as monoPrintedAt has them): tracePrinted while the trace's level is 2 or more, else `eax` as handed. */
std::optional<u32> monoPrinted(App* app, ThreadState& ts, u32 eax, u32 format, guest::Words words)
{
    const u32 esp = ts.sp;
    const u32 slot = esp + 4 - 4 * (2 + words.count);
    put(app, slot + 4, format);
    for (unsigned i = 0; i < words.count; ++i)
        put(app, slot + 8 + 4 * i, words.word[i]);
    ts.sp = slot;
    std::optional<u32> left = eax;
    if (s32(at(app, kTraceLevel)) >= kTraceWrites)
    {
        const EspKept kept(ts);
        left = tracePrinted(app, ts, eax, format, slot + 8);
    }
    ts.sp = esp;
    return left;
}

/* 0x4e0ca0: pageCleared, `eax` handed: what the code leaves in eax. */
std::optional<u32> monoCleared(App* app, ThreadState& ts, u32 eax)
{
    u32 left = eax;
    if (!pageCleared(app, ts, left))
        return std::nullopt;
    return left;
}

/* 0x4e0d50: the attribute the monitor's text is written in, `attribute`'s
 * low byte; eax left as handed. */
u32 monoAttributeSet(App* app, u32 attribute)
{
    putByte(app, kMonoAttribute, u8(attribute));
    return attribute;
}

/* EA's random numbers seeded from `seed`: their six words, each the one
 * before plus a constant.  The last back. */
u32 randomSeeded(App* app, u32 seed)
{
    static const u32 steps[] = {0xf22d0e56, 0x96041893, 0x3df3b646, 0x40dde76d, 0x97327ae1, 0xd1a9fbe7};
    for (u32 i = 0; i < 6; ++i)
    {
        seed += steps[i];
        put(app, kRandom + 4 * i, seed);
    }
    return seed;
}

/* Whether the file named `name` is there: opened (openToRead, its handle,
 * where and size into the frame under ebx, ecx, edx and esi pushed) with the
 * complaint at a missing one off, closed again (closeChecked). */
std::optional<bool> fileThere(App* app, ThreadState& ts, u32 name)
{
    const GuestFrame frame(ts, 0x10, 0xc);
    const u32 handleAt = frame - 0xc, whereAt = frame - 8, sizeAt = frame - 4;
    const u32 complain = at(app, kComplain);
    put(app, kComplain, 0);
    if (!guest::direct<0x4e28d0, ea::openToRead>(app, ts, name, handleAt, whereAt, sizeAt))
        return std::nullopt;
    const u32 handle = at(app, handleAt);
    if (handle != 0 && !guest::direct<0x4e2da0, ea::closeChecked>(app, ts, handle))
        return std::nullopt;
    put(app, kComplain, complain);
    return handle != 0;
}

/* loadfileadr: the file named `name` loaded whole into a block allocated
 * with `flags`; its size kept at 0x5643f8.  The load's hook (0x567868), if
 * set, is handed the block, the name and the flags, and what it gives back is
 * the block (none: the block freed).  The block, or 0.  The open's handle,
 * where and size are in the frame, under ebx, ecx, esi, edi and ebp pushed.
 * Port: the music stream is no longer topped up before and after the load
 * (0x4ff7c0, 0x4ff7e0), which kept one CD drive from seeking under the music;
 * the stream reads on between the load's blocks. */
std::optional<u32> loadWhole(App* app, ThreadState& ts, u32 name, u32 flags)
{
    const GuestFrame frame(ts, 0x14, 0xc);
    const u32 sizeAt = frame - 0xc, whereAt = frame - 8, handleAt = frame - 4;
    const auto opened = guest::direct<kOpen, ea::openToRead>(app, ts, name, handleAt, whereAt, sizeAt);
    if (!opened)
        return std::nullopt;
    if (*opened == 0 || at(app, sizeAt) == 0)
    {
        // Not there, or empty: closed if it was opened.
        const u32 handle = at(app, handleAt);
        if (handle != 0 && !guest::direct<0x4e2da0, ea::closeChecked>(app, ts, handle))
            return std::nullopt;
        return 0;
    }
    const u32 bytes = at(app, sizeAt);
    put(app, kWhere, kLoadFile);
    put(app, kWhereLine, kLoadLine);
    put(app, kWhereFunction, kLoadFunction);
    const auto block = guest::direct<kAllocate, ea::reserved>(app, ts, name, bytes, flags);
    if (!block)
        return std::nullopt;
    if (*block != 0)
    {
        if (!guest::direct<kRead, ea::readChecked>(app, ts, at(app, handleAt), *block, at(app, sizeAt)))
            return std::nullopt;
        put(app, kLoadedSize, at(app, sizeAt));
    }
    if (!guest::direct<0x4e2da0, ea::closeChecked>(app, ts, at(app, handleAt)))
        return std::nullopt;
    if (at(app, kLoadHook) == 0 || *block == 0)
        return *block;
    // The hook, a pointer in the game's memory: the block, the name and the flags in eax, edx and ebx.
    const auto other = guest::pointerCall(app, ts, at(app, kLoadHook), {*block, name, flags});
    if (!other)
        return std::nullopt;
    if (*other == 0 && !guest::direct<kFree, ea::purgeBlock>(app, ts, *block))
        return std::nullopt;
    return *other;
}

/* loadWhole with the complaint at a missing file off. */
std::optional<u32> loadQuietly(App* app, ThreadState& ts, u32 name, u32 flags)
{
    const u32 complain = at(app, kComplain);
    put(app, kComplain, 0);
    const auto block = guest::direct<kLoadWhole, ea::loadWhole>(app, ts, name, flags);
    if (!block)
        return std::nullopt;
    put(app, kComplain, complain);
    return *block;
}

nfs::u32 gameTrace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 openViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 closeViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findInViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findVivEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoPrintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoPrintfAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoScroll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoSetTop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoSetAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoSetTab(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 monoNoCursor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 seedRandom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 loadFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 loadFileQuietly(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

/* 0x42d510's core: the game's own trace, compiled out: nothing. */
void traceOff(App*)
{
}

/* 0x42d510: the game's own trace (a level, a channel, a format and its
 * arguments on the stack), compiled out: nothing (its frame only). */
nfs::u32 gameTrace(nfs::App*, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

/* 0x43b830: vivOpened of the archive named eax, its FILE to [edx]: the
 * block, or 0; edx left the files' count where there is a block. */
nfs::u32 openViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> block;
    {
        const EspKept kept(ts);
        block = vivOpened(app, ts, args.eax, args.edx);
    }
    if (!block)
        return args.eax;
    if (*block != 0)
        args.edx = at(app, *block + kVivCount);
    args.eax = *block;
    ts.sp += 4;
    return args.eax;
}

/* 0x43b9b0: vivClosed of block eax and the FILE at [edx]. */
nfs::u32 closeViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> closed;
    {
        const EspKept kept(ts);
        closed = vivClosed(app, ts, args.eax, args.edx);
    }
    if (!closed)
        return args.eax;
    args.eax = *closed;
    ts.sp += 4;
    return args.eax;
}

/* What a look-up leaves in the registers, as the code does: ecx the entry;
 * found, eax the size word's top byte and edx the size; else eax the header
 * (edx as _stricmp left it). */

/* 0x43b9d0: vivFile of the name at edx in the header eax: its offset to
 * [ebx], its size to [ecx]. */
nfs::u32 findInViv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 header = args.eax;
    std::optional<VivSearch> search;
    {
        const EspKept kept(ts);
        search = vivFile(app, ts, header, args.edx, args.ebx, args.ecx);
    }
    if (!search)
        return args.eax;
    args.ecx = search->entry;
    if (search->found)
    {
        args.eax = search->sizeWord >> 24;
        args.edx = bigEndian(search->sizeWord);
    }
    else
    {
        args.eax = header;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x4679f0: vivEntry of the name at edx in the header eax: its offset to
 * [ebx], its size to [ecx]. */
nfs::u32 findVivEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 header = args.eax;
    std::optional<VivSearch> search;
    {
        const EspKept kept(ts);
        search = vivEntry(app, ts, header, args.edx, args.ebx, args.ecx);
    }
    if (!search)
        return args.eax;
    args.ecx = search->entry;
    if (search->found)
    {
        args.eax = search->sizeWord >> 24;
        args.edx = bigEndian(search->sizeWord);
    }
    else
    {
        args.eax = header;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x4e08f0: EA's trace, printf's way (the format and its arguments on the
 * stack): tracePrinted, while the trace's level is 2 or more.  eax as the lock's leave gives it back (as it was, with
 * nothing written). */
nfs::u32 monoPrintf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (s32(at(app, kTraceLevel)) >= kTraceWrites)
    {
        std::optional<u32> given;
        {
            const EspKept kept(ts);
            given = tracePrinted(app, ts, args.eax, at(app, ts.sp + 4), ts.sp + 8);
        }
        if (!given)
            return args.eax;
        args.eax = *given;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0980: printedAt of column [esp + 4] and row [esp + 8], the format
 * after them. */
nfs::u32 monoPrintfAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> left;
    {
        const EspKept kept(ts);
        left = printedAt(app, ts, args.eax, at(app, ts.sp + 4), at(app, ts.sp + 8), at(app, ts.sp + 0xc),
                         ts.sp + 0x10);
    }
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0a60: monoText of the text at eax: eax as the code leaves it. */
nfs::u32 monoWrite(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> left;
    {
        const EspKept kept(ts);
        left = monoText(app, ts, args.eax);
    }
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0c10: the window scrolled if the cursor is past the page (scrolled):
 * eax as the clearing leaves it, else as it was. */
nfs::u32 monoScroll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 left = args.eax;
    {
        const EspKept kept(ts);
        if (!scrolled(app, ts, left))
            return args.eax;
    }
    args.eax = left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0ca0: monoCleared: eax as the lock's leave gives it back (as it
 * was, with nothing cleared). */
nfs::u32 monoClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> left;
    {
        const EspKept kept(ts);
        left = monoCleared(app, ts, args.eax);
    }
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0d00: windowTopSet to eax.  No caller in the game. */
nfs::u32 monoSetTop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> given;
    {
        const EspKept kept(ts);
        given = windowTopSet(app, ts, args.eax);
    }
    if (!given)
        return args.eax;
    args.eax = *given;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0d50: monoAttributeSet, al; eax as it was. */
nfs::u32 monoSetAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = monoAttributeSet(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0d60: the tab's width, al, if eax is 1 or more.  No caller. */
nfs::u32 monoSetTab(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (s32(args.eax) >= 1)
        putByte(app, kMonoTab, nfs::u8(args.eax));
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0d70: monitorPage eax: eax left the page's address.  No caller. */
nfs::u32 monoPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = monitorPage(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0d80: the bytes at eax and edx 0 -- a cursor's column and row asked of
 * a monitor that has none, it seems.  No caller: dead. */
nfs::u32 monoNoCursor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    putByte(app, args.eax, 0);
    putByte(app, args.edx, 0);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0dee: randomSeeded from eax (snd::eaRandom, 0x4e0d88, just before it,
 * draws from them): eax left the last word.  Not the trace's: it only follows
 * it in the library. */
nfs::u32 seedRandom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = randomSeeded(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0e60: fileThere of the file named eax: 1, or 0. */
nfs::u32 fileExists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<bool> there;
    {
        const EspKept kept(ts);
        there = fileThere(app, ts, args.eax);
    }
    if (!there)
        return args.eax;
    args.eax = *there ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0ec0 (loadfileadr, cmn\loadfile.c): loadWhole of the file named eax,
 * flags edx: the block, or 0. */
nfs::u32 loadFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> block;
    {
        const EspKept kept(ts);
        block = loadWhole(app, ts, args.eax, args.edx);
    }
    if (!block)
        return args.eax;
    args.eax = *block;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e0fe0: loadQuietly of the file named eax, flags edx. */
nfs::u32 loadFileQuietly(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> block;
    {
        const EspKept kept(ts);
        block = loadQuietly(app, ts, args.eax, args.edx);
    }
    if (!block)
        return args.eax;
    args.eax = *block;
    ts.sp += 4;
    return args.eax;
}

}  // namespace ea
}  // namespace nfs3hp
