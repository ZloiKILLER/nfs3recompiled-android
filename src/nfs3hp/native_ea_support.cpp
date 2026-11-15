#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "native_windows.h"
#include "host_threads.h"
#include <lib/winapp.h>
#include <winapi/types.h>

#include <optional>

/* EA's trace and log, files loaded whole into memory, and .viv archives.  disassemble_nfs3hp.py binds each
 * address to the function of that name here (EA_SUPPORT_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code.
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
 * Each is a typed function under a name of its own, with an adapter under the
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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The trace's: its level, its lock; the monitor's page, the cursor, the
// window's top row, the attribute, the tab's width.
constexpr reg32 kTraceLevel = 0x5644c8, kTraceLock = 0x5644c4;
constexpr reg32 kMonoPage = 0x5644d0, kMonoCursor = 0x5644d4;
constexpr reg32 kMonoTop = 0x5644d8, kMonoAttribute = 0x5644d9, kMonoTab = 0x5644da;
constexpr reg32 kRowBytes = 0xa0, kPageBytes = 0xfa0, kLastRow = 0xf00;
// The page's rows and columns; the level the trace writes from; the page's
// place (0xb0000, 0x8000 a page).
constexpr sreg32 kRows = 0x19, kColumns = 0x50, kTraceWrites = 2;
constexpr reg32 kMonoMemory = 0xb0000, kMonoPageShift = 0xf;
// The text's controls: backspace, tab, newline, the window's top, the page
// cleared (8 to 0xc, by the code's table at 0x4e0a48); the attribute's
// escape; the first byte written as it is.
constexpr reg8 kBackspace = 8, kLastControl = 4, kAttributeEscape = 0x0e, kFirstPrinted = 0x20;
// EA's random numbers' six words (snd::eaRandom draws from them).
constexpr reg32 kRandom = 0x5644e0;
// The loads': whether a missing file is complained of; the last file's size;
// the load's hook; what the load's block size is handed.
constexpr reg32 kComplain = 0x56440c, kLoadedSize = 0x5643f8;
constexpr reg32 kLoadHook = 0x567868, kLoadSomething = 0x567860;
constexpr reg32 kWhere = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr reg32 kLoadFile = 0x549928, kLoadFunction = 0x549938, kLoadLine = 0x4b;
// The stream's top-up before a load, and the bytes of the file each more of it is.
constexpr reg32 kTopUpFirst = 0x1f4;
constexpr sreg32 kTopUpBytes = 0x10e;
// The archive's: fopen's mode "rb", the block's name ""; the header's words
// (its size at 0xc, the files' count at 8, the entries from 0x10, each its
// offset, size and name); the block's slack and its allocation's flags;
// fseek's SEEK_SET; the name's length looked for.
constexpr reg32 kReadBinary = 0x5376e4, kNoName = 0x537690;
constexpr reg32 kVivCount = 8, kVivHeaderSize = 0xc, kVivEntries = 0x10, kVivEntryName = 8;
constexpr reg32 kVivSlack = 0x30, kVivBlockFlags = 0x10, kSeekSet = 0, kVivNameBytes = 0x1d;
// What is called by address.
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kFillZero = 0x4e070c;
constexpr reg32 kOpen = 0x4e28d0, kRead = 0x4e2de0;
constexpr reg32 kTopUp = 0x4ff7c0, kTopUpEnd = 0x4ff7e0, kLoadBlockSize = 0x4ff820, kLoadFlags = 0x4ff830;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kLoadWhole = 0x4e0ec0;

// The registers Kept keeps.
constexpr unsigned kKeepEbx = 1, kKeepEcx = 2, kKeepEdx = 4;

/* Registers the game's code pushes on entry and pops before its ret, which
 * the calls by address in between may change: as they were when the scope
 * ends. */
class Kept
{
public:
    Kept(CPU& cpu, unsigned which) : cpu_(cpu), which_(which), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx) {}
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;
    ~Kept()
    {
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
    const reg32 ebx_, ecx_, edx_;
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

/* One of the game's functions at `target`, handed `arguments` in Watcom's
 * registers, eax, edx, ebx and ecx in that order (the others as they are):
 * what it gives back in eax.  nullopt when the game stopped in it. */
std::optional<reg32> watcom(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> arguments)
{
    reg32* const registers[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    reg32* const* into = registers;
    for (const reg32 argument : arguments)
        **into++ = argument;
    if (!guest::call(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

/* `Core`, the game's `Address`, called as C++ (guest::direct), `arguments` first loaded into
 * Watcom's registers, eax, edx, ebx and ecx, as the call by address loads them -- the runtime's
 * functions keep them, so the code after finds them as it did -- and its result left in eax.
 * Its eax; nullopt when the game stopped in it. */
template <reg32 Address, auto Core, typename... Args>
std::optional<reg32> loaded(App* app, CPU& cpu, Args... arguments)
{
    guest::loadWatcomArguments(cpu, {reg32(arguments)...});
    const auto result = guest::direct<Address, Core>(app, cpu, reg32(arguments)...);
    if (cpu.terminate)
        return std::nullopt;
    cpu.eax = reg32(result);
    return cpu.eax;
}

/* Win32's function `Function`, handed `handle`: the port's own, called as
 * C++ (native_windows.h); in the check through its import's slot `Slot`,
 * stdcall.  What it gives back in eax.  nullopt when the game stopped in it. */
template <reg32 Slot, auto Function>
std::optional<reg32> windows(App* app, CPU& cpu, reg32 handle)
{
    return guest::windows<Slot, Function>(app, cpu, handle);
}

/* The trace's lock, made the first time it is wanted (native_ea_threads.h's
 * newLock), eax left it as the code leaves it: the calls after hand eax over
 * as it is.  False when the game stopped. */
bool traceLockMade(App* app, CPU& cpu)
{
    if (at(app, kTraceLock) != 0)
        return true;
    const std::optional<reg32> lock = guest::direct<0x4eb2e0, ea::newLock>(app, cpu);
    if (!lock)
        return false;
    put(app, kTraceLock, *lock);
    cpu.eax = *lock;
    return true;
}

/* The trace's lock taken or given back (EnterCriticalSection,
 * LeaveCriticalSection): what Windows gives back. */
template <reg32 Slot, auto Function>
std::optional<reg32> traceLock(App* app, CPU& cpu)
{
    return windows<Slot, Function>(app, cpu, at(app, kTraceLock));
}

/* A word of an archive's, big-endian, as the host has it. */
reg32 bigEndian(reg32 word)
{
    return (word & 0xff) << 24 | (word & 0xff00) << 8 | (word & 0xff0000) >> 8 | (word & 0xff000000) >> 24;
}

}  // namespace

/* The .viv archive named `name` opened, its FILE to [`file`]: its header's
 * size (big-endian at 0xc) read into the frame (under ebx, ecx, esi, edi and
 * ebp pushed), a block of it and 0x30 more allocated and cleared, the header
 * read into it whole and its first word, the files' count and the header's
 * size byte-swapped.  The block, or 0 -- the file closed and [`file`] 0 when
 * there is no block.  ebx and ecx kept. */
std::optional<reg32> vivOpened(App* app, CPU& cpu, reg32 name, reg32 file)
{
    const GuestFrame frame(cpu, 0x14, 4);
    const reg32 sizeAt = frame - 4;
    const Kept kept(cpu, kKeepEbx | kKeepEcx);
    const auto stream = loaded<0x4edfe8, watcom::clib::fopen>(app, cpu, name, kReadBinary);
    if (!stream)
        return std::nullopt;
    put(app, file, *stream);
    if (*stream == 0)
        return 0;
    if (!loaded<0x4e9088, watcom::clib::fseek>(app, cpu, *stream, kVivHeaderSize, kSeekSet)
        || !loaded<0x4e9200, watcom::clib::fread>(app, cpu, sizeAt, 4u, 1u, at(app, file)))
        return std::nullopt;
    const reg32 bytes = bigEndian(at(app, sizeAt)) + kVivSlack;
    const auto block = watcom(app, cpu, kAllocate, {kNoName, bytes, kVivBlockFlags});
    if (!block)
        return std::nullopt;
    if (*block == 0)
    {
        if (!loaded<0x4ee100, watcom::clib::fclose>(app, cpu, at(app, file)))
            return std::nullopt;
        put(app, file, 0);
        return 0;
    }
    if (!loaded<0x4e0640, watcom::clib::memset>(app, cpu, *block, 0u, bytes)
        || !loaded<0x4e9088, watcom::clib::fseek>(app, cpu, at(app, file), 0u, kSeekSet)
        || !loaded<0x4e9200, watcom::clib::fread>(app, cpu, *block, bytes, 1u, at(app, file)))
        return std::nullopt;
    put(app, *block, bigEndian(at(app, *block)));
    put(app, *block + kVivHeaderSize, bigEndian(at(app, *block + kVivHeaderSize)));
    put(app, *block + kVivCount, bigEndian(at(app, *block + kVivCount)));
    return *block;
}

/* The archive's block `block` freed and its FILE at [`file`] closed, [`file`]
 * 0: what fclose gives back. */
std::optional<reg32> vivClosed(App* app, CPU& cpu, reg32 block, reg32 file)
{
    if (!watcom(app, cpu, kFree, {block}))
        return std::nullopt;
    const auto closed = loaded<0x4ee100, watcom::clib::fclose>(app, cpu, at(app, file));
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
std::optional<VivSearch> vivEntry(App* app, CPU& cpu, reg32 header, reg32 wanted, reg32 offsetTo, reg32 sizeTo)
{
    reg32 entry = header + kVivEntries;
    for (reg32 index = 0; index < at(app, header + kVivCount); ++index)
    {
        const reg32 name = entry + kVivEntryName;
        const auto differs = loaded<0x4e9ec0, watcom::clib::stricmp>(app, cpu, wanted, name);
        if (!differs)
            return std::nullopt;
        if (*differs == 0)
        {
            const reg32 sizeWord = at(app, entry + 4);
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
 * archive's header `header` (vivEntry).  ebx kept. */
std::optional<VivSearch> vivFile(App* app, CPU& cpu, reg32 header, reg32 path, reg32 offsetTo, reg32 sizeTo)
{
    const GuestFrame frame(cpu, 0xc, 0x24);
    const reg32 name = frame - 0x24;
    const Kept kept(cpu, kKeepEbx);
    const auto slash = loaded<0x4ee2f0, watcom::clib::strrchr>(app, cpu, path, reg32('\\'));
    if (!slash || !loaded<0x4e0e30, watcom::clib::strncpy>(app, cpu, name, *slash != 0 ? *slash + 1 : path, kVivNameBytes))
        return std::nullopt;
    return vivEntry(app, cpu, header, name, offsetTo, sizeTo);
}

namespace
{

/* What a look-up leaves in the registers, as the code does: ecx the entry;
 * found, eax the size word's top byte and edx the size; else eax the header
 * (edx as _stricmp left it). */
void vivSearchLeft(CPU& cpu, reg32 header, const VivSearch& search)
{
    cpu.ecx = search.entry;
    if (search.found)
    {
        cpu.eax = search.sizeWord >> 24;
        cpu.edx = bigEndian(search.sizeWord);
    }
    else
    {
        cpu.eax = header;
    }
    cpu.esp += 4;
}

/* The window's rows below its top moved up one (native_ea_memory.h's moveMemory),
 * the last cleared (0x4e070c), the cursor at its start -- once the cursor is
 * past the page.  `left` what the code leaves in eax: the clearing's.  ebx,
 * ecx and edx kept.  False when the game stopped. */
bool scrolled(App* app, CPU& cpu, reg32& left)
{
    if (sreg32(at(app, kMonoCursor)) < sreg32(kPageBytes))
        return true;
    const Kept kept(cpu, kKeepEbx | kKeepEcx | kKeepEdx);
    const reg32 top = byte(app, kMonoTop);
    const reg32 page = at(app, kMonoPage);
    if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, page + top * kRowBytes, page + (top + 1) * kRowBytes,
                                                 (kRows - 1 - top) * kRowBytes)
        || !watcom(app, cpu, kFillZero, {at(app, kMonoPage) + kLastRow, kRowBytes}))
        return false;
    put(app, kMonoCursor, kLastRow);
    left = cpu.eax;
    return true;
}

/* The monitor's page cleared, a dword at a time, under the trace's lock,
 * while the trace's level is 2 or more.  The cursor stays.  `left` what the
 * code leaves in eax, handed over as it is (the lock's calls give it back as
 * they find it, now and then): what LeaveCriticalSection gives back, eax the
 * clearing's end before it.  ecx and edx kept.  False when the game stopped. */
bool pageCleared(App* app, CPU& cpu, reg32& left)
{
    if (sreg32(at(app, kTraceLevel)) < kTraceWrites)
        return true;
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    cpu.eax = left;
    if (!traceLockMade(app, cpu) || !traceLock<kEnterCriticalSection, &host::enterLock>(app, cpu))
        return false;
    for (reg32 offset = 0; offset < kPageBytes; offset += 4)
        put(app, at(app, kMonoPage) + offset, 0);
    cpu.eax = kPageBytes;
    const auto given = traceLock<kLeaveCriticalSection, &host::leaveLock>(app, cpu);
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
 * What the code leaves in eax back (`left`, the check holds it to that): each
 * cell's word summed in eax out of what eax and edx held before, their upper
 * halves with it (`carried`, edx's), and each control's own. */
std::optional<reg32> monoText(App* app, CPU& cpu, reg32 text)
{
    if (sreg32(at(app, kTraceLevel)) < kTraceWrites)
        return text;
    reg32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    reg32 left = 0;
    reg32 carried = text;
    bool attributeNext = false;
    for (reg32 i = 0; sreg32(i) < sreg32(length); ++i)
    {
        const reg8 c = byte(app, text + i);
        if (attributeNext)
        {
            left = c;
            attributeNext = false;
            putByte(app, kMonoAttribute, c);
            continue;
        }
        attributeNext = c == kAttributeEscape;
        const reg32 cursor = at(app, kMonoCursor);
        if (c >= kFirstPrinted)
        {
            if (!scrolled(app, cpu, left))
                return std::nullopt;
            const reg32 next = at(app, kMonoCursor) + 2;
            left = ((((left & 0xff0000) << 8) | reg32(byte(app, kMonoAttribute)) << 8) + ((carried & 0xffff0000) | c));
            carried = at(app, kMonoPage);
            put(app, kMonoCursor, next);
            app->getMemory<reg16>(carried + next - 2) = reg16(left);
            continue;
        }
        const reg8 control = reg8(c - kBackspace);
        if (control > kLastControl)
        {
            left = (left & 0xffffff00) | control;
            continue;
        }
        left = control;
        switch (control)
        {
        case 0:  // backspace
            if (sreg32(cursor) >= 2)
                put(app, kMonoCursor, cursor - 2);
            break;
        case 1:  // tab: the column on to the next multiple of the tab's width
        {
            const reg32 tab = byte(app, kMonoTab);
            if (tab <= 1)
                break;
            const sreg32 rowStart = sreg32(cursor) / sreg32(kRowBytes) * sreg32(kRowBytes);
            const sreg32 column = (sreg32(cursor) - rowStart) / 2;
            left = reg32((column / sreg32(tab) + 1) * sreg32(tab) * 2);
            carried = reg32(column % sreg32(tab));
            put(app, kMonoCursor, left + reg32(rowStart));
            break;
        }
        case 2:  // newline: the next row's start, the window scrolled at the bottom
            left = reg32(sreg32(cursor) / sreg32(kRowBytes));
            carried = reg32(sreg32(cursor) % sreg32(kRowBytes));
            put(app, kMonoCursor, (left + 1) * kRowBytes);
            if (!scrolled(app, cpu, left))
                return std::nullopt;
            break;
        case 3:  // the window's top
            put(app, kMonoCursor, byte(app, kMonoTop) * kRowBytes);
            break;
        case 4:  // the page cleared, the window's top
            if (!pageCleared(app, cpu, left))
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
 * eax handed to LeaveCriticalSection as monoText leaves it.  ebx, ecx and
 * edx kept.  False when the game stopped. */
bool tracePrinted(App* app, CPU& cpu, reg32 format, reg32 arguments)
{
    const GuestFrame frame(cpu, 0, 0x104);
    const reg32 list = frame - 4, buffer = frame - 0x104;
    const Kept kept(cpu, kKeepEbx | kKeepEcx | kKeepEdx);
    if (!traceLockMade(app, cpu) || !traceLock<kEnterCriticalSection, &host::enterLock>(app, cpu))
        return false;
    put(app, list, arguments);
    if (!loaded<0x4df674, watcom::clib::vsprintf>(app, cpu, buffer, format, list))
        return false;
    const auto left = monoText(app, cpu, buffer);
    if (!left)
        return false;
    cpu.eax = *left;
    return traceLock<kLeaveCriticalSection, &host::leaveLock>(app, cpu).has_value();
}

/* The cursor moved to `column` and `row`, if they are on the page. */
void cursorTo(App* app, reg32 column, reg32 row)
{
    if (sreg32(column) >= 0 && sreg32(column) < kColumns && sreg32(row) >= 0 && sreg32(row) < kRows)
        put(app, kMonoCursor, (row * reg32(kColumns) + column) * 2);
}

/* The scrolling window's top row `row` (0 to 24, else as it was), under the
 * trace's lock: what LeaveCriticalSection gives back.  ecx and edx kept. */
std::optional<reg32> windowTopSet(App* app, CPU& cpu, reg32 row)
{
    const Kept kept(cpu, kKeepEcx | kKeepEdx);
    if (!traceLockMade(app, cpu) || !traceLock<kEnterCriticalSection, &host::enterLock>(app, cpu))
        return std::nullopt;
    if (sreg32(row) >= 0 && sreg32(row) < kRows)
        putByte(app, kMonoTop, reg8(row));
    return traceLock<kLeaveCriticalSection, &host::leaveLock>(app, cpu);
}

/* The monitor's page `page`: 0xb0000 + page * 0x8000, kept and back. */
reg32 monitorPage(App* app, reg32 page)
{
    const reg32 address = (page << kMonoPageShift) + kMonoMemory;
    put(app, kMonoPage, address);
    return address;
}

}  // namespace

/* EA's random numbers seeded from `seed`: their six words, each the one
 * before plus a constant.  The last back. */
reg32 randomSeeded(App* app, reg32 seed)
{
    static const reg32 steps[] = {0xf22d0e56, 0x96041893, 0x3df3b646, 0x40dde76d, 0x97327ae1, 0xd1a9fbe7};
    for (reg32 i = 0; i < 6; ++i)
    {
        seed += steps[i];
        put(app, kRandom + 4 * i, seed);
    }
    return seed;
}

/* Whether the file named `name` is there: opened (openToRead, its handle,
 * where and size into the frame under ebx, ecx, edx and esi pushed) with the
 * complaint at a missing one off, closed again (closeChecked).  ebx, ecx and
 * edx kept. */
std::optional<bool> fileThere(App* app, CPU& cpu, reg32 name)
{
    const GuestFrame frame(cpu, 0x10, 0xc);
    const reg32 handleAt = frame - 0xc, whereAt = frame - 8, sizeAt = frame - 4;
    const Kept kept(cpu, kKeepEbx | kKeepEcx | kKeepEdx);
    const reg32 complain = at(app, kComplain);
    put(app, kComplain, 0);
    if (!guest::direct<0x4e28d0, ea::openToRead>(app, cpu, name, handleAt, whereAt, sizeAt))
        return std::nullopt;
    const reg32 handle = at(app, handleAt);
    if (handle != 0)
    {
        cpu.ebx = 1;  // as the code has it, to say the file is there
        if (!guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle))
            return std::nullopt;
    }
    put(app, kComplain, complain);
    return handle != 0;
}

/* loadfileadr: the file named `name` loaded whole into a block allocated
 * with `flags` -- the sound's stream topped up before (0x4ff7c0), the more so
 * the bigger the file; its size kept at 0x5643f8.  The load's hook
 * (0x567868), if set, is handed the block, the name and the flags, and what
 * it gives back is the block (none: the block freed; edx left it).  The
 * block, or 0.  The open's handle, where and size are in the frame, under
 * ebx, ecx, esi, edi and ebp pushed, their addresses in the registers before
 * the first top-up already.  ebx and ecx kept; edx as the calls leave it --
 * the open's, the block's, the read's and the free's by address for that. */
std::optional<reg32> loadWhole(App* app, CPU& cpu, reg32 name, reg32 flags)
{
    const GuestFrame frame(cpu, 0x14, 0xc);
    const reg32 sizeAt = frame - 0xc, whereAt = frame - 8, handleAt = frame - 4;
    const Kept kept(cpu, kKeepEbx | kKeepEcx);
    cpu.edx = handleAt;
    cpu.ebx = whereAt;
    cpu.ecx = sizeAt;
    if (!watcom(app, cpu, kTopUp, {kTopUpFirst}))
        return std::nullopt;
    const auto opened = watcom(app, cpu, kOpen, {name, handleAt, whereAt, sizeAt});
    if (!opened)
        return std::nullopt;
    if (*opened == 0 || at(app, sizeAt) == 0)
    {
        // Not there, or empty (edx the size the code read): closed if it was opened.
        if (*opened != 0)
            cpu.edx = 0;
        const reg32 handle = at(app, handleAt);
        if (handle != 0 && !guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle))
            return std::nullopt;
        return 0;
    }
    const sreg32 size = sreg32(at(app, sizeAt));
    const auto loadFlags = watcom(app, cpu, kLoadFlags, {at(app, handleAt)});
    if (!loadFlags)
        return std::nullopt;
    const auto blockSize = watcom(app, cpu, kLoadBlockSize, {at(app, kLoadSomething), *loadFlags, kLoadFile});
    if (!blockSize || !watcom(app, cpu, kTopUp, {*blockSize + reg32(size / kTopUpBytes)}))
        return std::nullopt;
    const reg32 bytes = at(app, sizeAt);
    put(app, kWhere, kLoadFile);
    put(app, kWhereLine, kLoadLine);
    put(app, kWhereFunction, kLoadFunction);
    const auto block = watcom(app, cpu, kAllocate, {name, bytes, flags});
    if (!block)
        return std::nullopt;
    if (*block != 0)
    {
        if (!watcom(app, cpu, kRead, {at(app, handleAt), *block, at(app, sizeAt)}))
            return std::nullopt;
        put(app, kLoadedSize, at(app, sizeAt));
    }
    if (!guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, at(app, handleAt))
        || !watcom(app, cpu, kTopUpEnd, {}))
        return std::nullopt;
    if (at(app, kLoadHook) == 0 || *block == 0)
        return *block;
    const auto other = watcom(app, cpu, at(app, kLoadHook), {*block, name, flags});
    if (!other)
        return std::nullopt;
    cpu.edx = *other;
    if (*other == 0 && !watcom(app, cpu, kFree, {*block}))
        return std::nullopt;
    return *other;
}

/* loadWhole (by address) with the complaint at a missing file off: ebx 0 and
 * ecx the complaint as the code hands them over.  ebx and ecx kept. */
std::optional<reg32> loadQuietly(App* app, CPU& cpu, reg32 name, reg32 flags)
{
    const Kept kept(cpu, kKeepEbx | kKeepEcx);
    const reg32 complain = at(app, kComplain);
    put(app, kComplain, 0);
    const auto block = watcom(app, cpu, kLoadWhole, {name, flags, 0, complain});
    if (!block)
        return std::nullopt;
    put(app, kComplain, complain);
    return *block;
}

void gameTrace(win32::WinApplication* app, x86::CPU& cpu);
void openViv(win32::WinApplication* app, x86::CPU& cpu);
void closeViv(win32::WinApplication* app, x86::CPU& cpu);
void findInViv(win32::WinApplication* app, x86::CPU& cpu);
void findVivEntry(win32::WinApplication* app, x86::CPU& cpu);
void monoPrintf(win32::WinApplication* app, x86::CPU& cpu);
void monoPrintfAt(win32::WinApplication* app, x86::CPU& cpu);
void monoWrite(win32::WinApplication* app, x86::CPU& cpu);
void monoScroll(win32::WinApplication* app, x86::CPU& cpu);
void monoClear(win32::WinApplication* app, x86::CPU& cpu);
void monoSetTop(win32::WinApplication* app, x86::CPU& cpu);
void monoSetAttribute(win32::WinApplication* app, x86::CPU& cpu);
void monoSetTab(win32::WinApplication* app, x86::CPU& cpu);
void monoPage(win32::WinApplication* app, x86::CPU& cpu);
void monoNoCursor(win32::WinApplication* app, x86::CPU& cpu);
void seedRandom(win32::WinApplication* app, x86::CPU& cpu);
void fileExists(win32::WinApplication* app, x86::CPU& cpu);
void loadFile(win32::WinApplication* app, x86::CPU& cpu);
void loadFileQuietly(win32::WinApplication* app, x86::CPU& cpu);

/* 0x42d510's core: the game's own trace, compiled out: nothing. */
void traceOff(App*)
{
}

/* 0x42d510: the game's own trace (a level, a channel, a format and its
 * arguments on the stack), compiled out: nothing (its frame only). */
void gameTrace(win32::WinApplication*, x86::CPU& cpu)
{
    cpu.esp += 4;
}

/* 0x43b830: vivOpened of the archive named eax, its FILE to [edx]: the
 * block, or 0; edx left the files' count where there is a block. */
void openViv(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto block = vivOpened(app, cpu, cpu.eax, cpu.edx);
    if (!block)
        return;
    if (*block != 0)
        cpu.edx = at(app, *block + kVivCount);
    cpu.eax = *block;
    cpu.esp += 4;
}

/* 0x43b9b0: vivClosed of block eax and the FILE at [edx]. */
void closeViv(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto closed = vivClosed(app, cpu, cpu.eax, cpu.edx);
    if (!closed)
        return;
    cpu.eax = *closed;
    cpu.esp += 4;
}

/* 0x43b9d0: vivFile of the name at edx in the header eax: its offset to
 * [ebx], its size to [ecx]. */
void findInViv(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 header = cpu.eax;
    const auto search = vivFile(app, cpu, header, cpu.edx, cpu.ebx, cpu.ecx);
    if (!search)
        return;
    vivSearchLeft(cpu, header, *search);
}

/* 0x4679f0: vivEntry of the name at edx in the header eax: its offset to
 * [ebx], its size to [ecx]. */
void findVivEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 header = cpu.eax;
    const auto search = vivEntry(app, cpu, header, cpu.edx, cpu.ebx, cpu.ecx);
    if (!search)
        return;
    vivSearchLeft(cpu, header, *search);
}

/* 0x4e08f0: EA's trace, printf's way (the format and its arguments on the
 * stack): tracePrinted, while the trace's level is 2 or more.  eax as LeaveCriticalSection gives it back (as it was, with
 * nothing written). */
void monoPrintf(win32::WinApplication* app, x86::CPU& cpu)
{
    if (sreg32(at(app, kTraceLevel)) >= kTraceWrites
        && !tracePrinted(app, cpu, at(app, cpu.esp + 4), cpu.esp + 8))
        return;
    cpu.esp += 4;
}

/* 0x4e0980: monoPrintf at column [esp + 4] and row [esp + 8] (the cursor
 * moved there, once the lock is taken, if they are on the page), the format
 * after them; the frame under ebx, esi and ebp pushed.  eax as the lock's
 * calls leave it, the lock handed in it. */
void monoPrintfAt(win32::WinApplication* app, x86::CPU& cpu)
{
    if (sreg32(at(app, kTraceLevel)) < kTraceWrites)
    {
        cpu.esp += 4;
        return;
    }
    const reg32 column = at(app, cpu.esp + 4), row = at(app, cpu.esp + 8);
    const reg32 format = at(app, cpu.esp + 0xc), arguments = cpu.esp + 0x10;
    // The cursor is moved after the lock is taken: tracePrinted's own steps,
    // with the move between them.
    {
        const GuestFrame frame(cpu, 0xc, 0x104);
        const reg32 list = frame - 4, buffer = frame - 0x104;
        const Kept kept(cpu, kKeepEbx | kKeepEcx | kKeepEdx);
        if (!traceLockMade(app, cpu)
            || !traceLock<kEnterCriticalSection, &host::enterLock>(app, cpu))
            return;
        cursorTo(app, column, row);
        put(app, list, arguments);
        if (!loaded<0x4df674, watcom::clib::vsprintf>(app, cpu, buffer, format, list) || !monoText(app, cpu, buffer))
            return;
        cpu.eax = at(app, kTraceLock);
        if (!traceLock<kLeaveCriticalSection, &host::leaveLock>(app, cpu))
            return;
    }
    cpu.esp += 4;
}

/* 0x4e0a60: monoText of the text at eax: eax as the code leaves it. */
void monoWrite(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto left = monoText(app, cpu, cpu.eax);
    if (!left)
        return;
    cpu.eax = *left;
    cpu.esp += 4;
}

/* 0x4e0c10: the window scrolled if the cursor is past the page (scrolled):
 * eax as the clearing leaves it, else as it was. */
void monoScroll(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 left = cpu.eax;
    if (!scrolled(app, cpu, left))
        return;
    cpu.eax = left;
    cpu.esp += 4;
}

/* 0x4e0ca0: pageCleared: eax as LeaveCriticalSection gives it back (as it
 * was, with nothing cleared). */
void monoClear(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 left = cpu.eax;
    if (!pageCleared(app, cpu, left))
        return;
    cpu.eax = left;
    cpu.esp += 4;
}

/* 0x4e0d00: windowTopSet to eax.  No caller in the game. */
void monoSetTop(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto given = windowTopSet(app, cpu, cpu.eax);
    if (!given)
        return;
    cpu.eax = *given;
    cpu.esp += 4;
}

/* 0x4e0d50: the attribute the monitor's text is written in, al. */
void monoSetAttribute(win32::WinApplication* app, x86::CPU& cpu)
{
    putByte(app, kMonoAttribute, cpu.al);
    cpu.esp += 4;
}

/* 0x4e0d60: the tab's width, al, if eax is 1 or more.  No caller. */
void monoSetTab(win32::WinApplication* app, x86::CPU& cpu)
{
    if (sreg32(cpu.eax) >= 1)
        putByte(app, kMonoTab, cpu.al);
    cpu.esp += 4;
}

/* 0x4e0d70: monitorPage eax: eax left the page's address.  No caller. */
void monoPage(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = monitorPage(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x4e0d80: the bytes at eax and edx 0 -- a cursor's column and row asked of
 * a monitor that has none, it seems.  No caller: dead. */
void monoNoCursor(win32::WinApplication* app, x86::CPU& cpu)
{
    putByte(app, cpu.eax, 0);
    putByte(app, cpu.edx, 0);
    cpu.esp += 4;
}

/* 0x4e0dee: randomSeeded from eax (snd::eaRandom, 0x4e0d88, just before it,
 * draws from them): eax left the last word.  Not the trace's: it only follows
 * it in the library. */
void seedRandom(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = randomSeeded(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x4e0e60: fileThere of the file named eax: 1, or 0. */
void fileExists(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto there = fileThere(app, cpu, cpu.eax);
    if (!there)
        return;
    cpu.eax = *there ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4e0ec0 (loadfileadr, cmn\loadfile.c): loadWhole of the file named eax,
 * flags edx: the block, or 0. */
void loadFile(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto block = loadWhole(app, cpu, cpu.eax, cpu.edx);
    if (!block)
        return;
    cpu.eax = *block;
    cpu.esp += 4;
}

/* 0x4e0fe0: loadQuietly of the file named eax, flags edx. */
void loadFileQuietly(win32::WinApplication* app, x86::CPU& cpu)
{
    const auto block = loadQuietly(app, cpu, cpu.eax, cpu.edx);
    if (!block)
        return;
    cpu.eax = *block;
    cpu.esp += 4;
}

}  // namespace ea
}  // namespace nfs3hp
