#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "os_threads.h"
#include <lib/app.h>

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* EA's memory manager and its memmove.
 *
 * The manager is EA's cmn\memstd.c (its messages name creatememclass,
 * getfreememclass, reservememadr and purgememadr).  Memory is handed out of
 * up to 16 classes, the class table at 0x9ef5c0 a pointer each; a block's
 * flags (and reservememadr's) name its class in their low nibble, and the
 * default class is 0x5643f4's.  A class is made over one stretch of memory:
 * a block "LOW" at its start whose 0x40 bytes are the class's header, one
 * free block for all between, a block "HIGH" of none at its end.
 *
 * A block: its 0x10 bytes of header, then its body, which the caller gets.
 *   +0  its signature: "MB" in use, "FB" free, "SB" the free list's sentinel
 *   +2  its flags: the class in the low nibble; 0x10 taken from the top of
 *       the class, 0x20 from its largest free block; 0x100 named, 0x200
 *       "BEND" after the body, 0x400 the file and line it was asked for at,
 *       0x800 a word cleared; 0x4000 in the free list
 *   +4  its size -- a free one's from its header up to the next block's
 *   +8  the next block in memory, +0xc the one before
 *   +10 and +14, a free one's: the next and the one before in the free list,
 *       which goes by address from the sentinel round to it again
 * After an allocated body: "BEND", the file and line, a zero word and the
 * class's overhead bytes as the flags say, then the name.
 *
 * A class's header (the LOW block's body): +0 its name, +8 and +0xc where its
 * memory starts and ends, +0x10 the free list's sentinel (its size 0x7fffffff,
 * the list's first and last at +0x20 and +0x24), +0x28 how blocks are
 * aligned, +0x2c how its start was, +0x30 the bytes each block takes past its
 * body, +0x34 its flags (0x100: a block's name counted in), +0x38 its lock
 * (0x4eb2e0's, a critical section) or none, +0x3c what its creator said.
 *
 * The game allocates and frees with reservememadr and purgememadr (the
 * sound's, the files', the menus' all), and asks a block's size and name.
 * clearMemClasses, releasememclass, getfreememclass, memBlockFlags and
 * purgememclass have no callers in the game.
 *
 * Its memmove (0x4ea4f0) copies backwards where the copy overlaps its source
 * from above, and otherwise by the forward copy (0x5033e5, mix::copy).
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the calls through a pointer: it leaves
 * eax, and the registers the code does not keep, as the code leaves them.
 * This file's own are called as C++ (reservememadr and purgememadr too), and
 * so are the locks made and freed (native_ea_threads.h's, guest::direct).
 * Error (0x401010), the game's fill (0x4e070c), sprintf (0x4df690), the
 * forward copy and the critical sections' Windows functions are called by
 * address, their registers kept
 * (EspKept), at the depth the code calls them from: Error hands
 * vsprintf its arguments' address, and creatememclass's names are made in its
 * frame on the guest's stack.  The free list's walks and the copies are host loops
 * over the guest's memory. */
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
using i32 = std::int32_t;

// The class table, and the default class; whether an error is reported.
constexpr u32 kClasses = 0x9ef5c0, kDefaultClass = 0x5643f4, kReportErrors = 0x56440c;
// Where an error is from: the file, the function, the line.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// Where the call is from, if anywhere: its file, its line.
constexpr u32 kCalledFile = 0x55219c, kCalledLine = 0x5521a4;
// The signatures.
constexpr u16 kInUse = 0x424d, kFree = 0x4246, kSentinel = 0x4253;
// memstd.c's strings: its file's name, its functions', its messages.
constexpr u32 kFileName = 0x549944;
constexpr u32 kCreateName = 0x549954, kCreateOutOfRange = 0x549964;
constexpr u32 kLowName = 0x5499ac, kHighName = 0x5499b4;
constexpr u32 kGetFreeName = 0x5499bc, kNoFreeClasses = 0x5499cc;
constexpr u32 kReserveName = 0x549a00, kInvalidSize = 0x549a10, kOutOfMemory = 0x549a6c;
constexpr u32 kPurgeName = 0x549ab8, kNotABlock = 0x549ac4;
// The lines of memstd.c its errors name.
constexpr u32 kCreateLine = 0x11a, kGetFreeLine = 0x1ae, kInvalidSizeLine = 0x2ae, kOutOfMemoryLine = 0x2ed;
constexpr u32 kPurgeLine = 0x333;
// What is called by address.
constexpr u32 kFillZero = 0x4e070c;
constexpr u32 kForwardCopy = 0x5033e5;

// A block's header: its flags (and their high byte), its size, the next and the one before in
// memory; a free one's next and the one before in the free list.  Its body past them.
constexpr u32 kBlockFlags = 2, kBlockFlagsHigh = 3, kBlockSize = 4, kBlockNext = 8, kBlockPrevious = 0xc;
constexpr u32 kListNext = 0x10, kListPrevious = 0x14, kBlockHeader = 0x10;
// A block's flags: the class's nibble; taken from the top, from the largest free block; named,
// "BEND" after it, the file and line, a word cleared; those the class's flags add; LOW's own,
// HIGH's; in the free list (in their high byte).
constexpr u32 kClassNibble = 0xf, kFromTop = 0x10, kFromLargest = 0x20;
constexpr u32 kNamed = 0x100, kBend = 0x200, kWhere = 0x400, kZeroWord = 0x800, kClassFlagsAdded = 0x700;
constexpr u32 kLowFlag = 0x8000, kHighFlags = 0x8010;
constexpr u8 kInFreeList = 0x40;
// A class's header: where its memory starts and ends, the free list's sentinel (a block's header:
// its size, the list's first and last), how blocks are aligned, how its start was, the bytes each
// block takes past its body, its flags (their second byte's 1: names counted in), its lock, what
// its creator said; 0x40 bytes.
constexpr u32 kClassStart = 8, kClassEnd = 0xc, kClassSentinel = 0x10, kClassAlign = 0x28;
constexpr u32 kClassStartAlign = 0x2c, kClassBytes = 0x30, kClassFlags = 0x34, kClassLock = 0x38;
constexpr u32 kClassWord = 0x3c, kClassHeader = 0x40;
constexpr u32 kSentinelSize = 0x7fffffff;
// "BEND", as its four bytes lie in memory.
constexpr u32 kBendMark = 0x444e4542;
// What is left of a free block past what is asked that is split off: more than this.
constexpr i32 kSplitAbove = 0x40;
// The least a block's body is.
constexpr i32 kLeastBody = 8;

u16 half(App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putHalf(App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

/* The header of the class `flags` names. */
u32 classOf(App* app, u32 flags)
{
    return at(app, kClasses + (flags & kClassNibble) * 4);
}

/* The name at `from` copied to `to` up to its zero and the zero, a byte at a time forwards (the
 * code's loop takes two at a time, each in turn). */
void copyName(App* app, u32 to, u32 from)
{
    u8* const memory = &app->getMemory<u8>(0);
    for (u32 i = 0;; ++i)
    {
        const u8 b = memory[u32(from + i)];
        memory[u32(to + i)] = b;
        if (b == 0)
            break;
    }
}

/* The bytes of the name at `name` and its zero (the code's repne scasb). */
u32 nameLength(App* app, u32 name)
{
    return u32(std::strlen(reinterpret_cast<const char*>(&app->getMemory<u8>(name)))) + 1;
}

/* The code's depth on the guest's stack under esp on entry: `pushed` bytes of registers (and a
 * call's return address slot), then `bytes` of its frame; esp under them while it lasts. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp)
    {
        ts.sp = esp_ - pushed - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }

private:
    ThreadState& ts_;
    const u32 esp_;
};

/* Error with `format` and its arguments, memstd.c's `function` at `line` where it is from: cdecl, at
 * esp as it is, every register but eax kept.  False when the game stopped. */
bool error(App* app, ThreadState& ts, u32 function, u32 line, std::initializer_list<u32> arguments)
{
    put(app, kWhereFile, kFileName);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
    guest::Words words({});
    for (const u32* word = arguments.begin() + 1; word < arguments.end(); ++word)
        words.word[words.count++] = *word;
    guest::direct<0x401010, ea::error>(app, ts, *arguments.begin(), words);
    return !ts.terminate;
}

/* `bytes` at `address` cleared by the game's fill (none of the callers' exits takes what it leaves:
 * each sets eax after it). */
bool clear(App* app, ThreadState& ts, u32 address, u32 bytes)
{
    guest::direct<kFillZero, ea::clearBytes>(app, ts, address, bytes);
    return !ts.terminate;
}

/* sprintf of `format` with `name` into `buffer`, at esp as it is. */
bool print(App* app, ThreadState& ts, u32 buffer, u32 format, u32 name)
{
    guest::direct<0x4df690, watcom::clib::sprintf>(app, ts, buffer, format, guest::Words({name}));
    return !ts.terminate;
}

/* The memmove's forward copy (native_mix.cpp's, no typed function: by address): `count` bytes from
 * `from` to `to`, ecx kept on the stack over it as the code keeps it.  It leaves eax and edx past
 * the bytes and ebx 0, which the memmove leaves (its thunk). */
bool forwardCopy(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    const GuestFrame frame(ts, 4, 0);
    return guest::pointerCall(app, ts, kForwardCopy, {to, from, count}).has_value();
}

/* Whether a copy of `count` bytes from `from` to `to` lands on its source from above. */
bool overlapsFromAbove(u32 to, u32 from, u32 count)
{
    return to > from && to < from + count;
}

/* 0x4e1000: the first free block of class `cls` of at least `size` bytes, from the free list's
 * start or its end; 0 if none -- the sentinel. */
u32 firstFreeBlock(App* app, u32 cls, i32 size, bool fromEnd)
{
    const u32 link = fromEnd ? kListPrevious : kListNext;
    u32 block = cls + kClassSentinel;
    do
        block = at(app, block + link);
    while (size > i32(at(app, block + kBlockSize)));
    return half(app, block) == kSentinel ? 0 : block;
}

/* What findLargestFreeBlock finds: the block (0 for none), its size -- for none the size asked
 * less one, 0 at least -- and the sentinel its walk stopped at. */
struct LargestFree
{
    u32 block;
    i32 size;
    u32 stop;
};

/* 0x4e1030: the largest free block of class `cls` of at least `asked` bytes (of any, 0 or less),
 * the free list gone through from its start or its end -- of those as large, the first. */
LargestFree largestFreeBlock(App* app, u32 cls, u32 asked, bool fromEnd)
{
    const u32 link = fromEnd ? kListPrevious : kListNext;
    i32 size = i32(asked - 1);
    if (size < 0)
        size = 0;
    u32 found = 0;
    u32 block = cls + kClassSentinel;
    for (;;)
    {
        block = at(app, block + link);
        const i32 blockSize = i32(at(app, block + kBlockSize));
        if (size >= blockSize)
            continue;
        if (half(app, block) == kSentinel)
            break;
        size = blockSize;
        found = block;
    }
    return {found, size, block};
}

/* A free block's neighbours in the free list. */
struct Neighbours
{
    u32 before, after;
};

/* What linkFreeBlock and unlinkFreeBlock leave in eax: a neighbour of `block` in the list, its
 * second byte the block's flags' high byte. */
u32 listLeft(App* app, u32 neighbour, u32 block)
{
    return (neighbour & 0xffff00ff) | u32(byte(app, block + kBlockFlagsHigh)) << 8;
}

/* 0x4e1080: `block` put into class `cls`'s free list where its address goes -- looked for from the
 * list's end if it is past the middle of the first and last, else from its start -- as "FB", its
 * size up to the next block, in the free list. */
Neighbours linkFree(App* app, u32 cls, u32 block)
{
    const u32 size = at(app, block + kBlockNext) - block - kBlockHeader;
    const u32 sentinel = cls + kClassSentinel;
    const u32 first = at(app, sentinel + kListNext);
    const u32 middle = ((at(app, sentinel + kListPrevious) - first) >> 1) + first;
    u32 before = sentinel, after = sentinel;
    if (block > middle)
    {
        do
            before = at(app, before + kListPrevious);
        while (block < before);
        after = at(app, before + kListNext);
    }
    else
    {
        do
            after = at(app, after + kListNext);
        while (block > after);
        before = at(app, after + kListPrevious);
    }
    put(app, block + kListPrevious, before);
    put(app, block + kBlockSize, size);
    put(app, block + kListNext, after);
    put(app, before + kListNext, block);
    put(app, after + kListPrevious, block);
    const u8 flagsHigh = byte(app, block + kBlockFlagsHigh);
    putHalf(app, block, kFree);
    putByte(app, block + kBlockFlagsHigh, u8(flagsHigh | kInFreeList));
    return {before, after};
}

/* 0x4e10e0: free `block` taken out of its free list, its signature cleared and its place in the
 * list; the block before it in the list. */
u32 unlinkFree(App* app, u32 block)
{
    const u32 before = at(app, block + kListPrevious), after = at(app, block + kListNext);
    put(app, before + kListNext, after);
    put(app, after + kListPrevious, before);
    const u8 flagsHigh = byte(app, block + kBlockFlagsHigh);
    putHalf(app, block, 0);
    putByte(app, block + kBlockFlagsHigh, u8(flagsHigh & ~kInFreeList));
    return before;
}

/* 0x4e1120: `block` marked in use ("MB"): `flags`, `size`, the blocks before and after it; after
 * its body "BEND", the file and line it is asked for at and a zero word as the flags say,
 * `classBytes` of the class's, and `name` if there is one and the flags say so.  The bytes from
 * the block to past all that. */
u32 markBlock(App* app, u32 block, u32 flags, u32 size, u32 classBytes, u32 name, u32 before,
                u32 after)
{
    putHalf(app, block, kInUse);
    putHalf(app, block + kBlockFlags, u16(flags));
    put(app, block + kBlockSize, size);
    put(app, block + kBlockNext, after);
    put(app, block + kBlockPrevious, before);
    u32 end = block + kBlockHeader + size;
    if (flags & kBend)
        put(app, end, kBendMark);
    if (flags & kWhere)
    {
        const u32 file = at(app, kCalledFile);
        if (file == 0)
        {
            put(app, end + 4, at(app, kWhereFile));
            put(app, end + 8, at(app, kWhereLine));
        }
        else
        {
            const u32 line = at(app, kCalledLine);
            put(app, end + 4, file);
            put(app, end + 8, line);
        }
    }
    if (flags & kZeroWord)
        put(app, end + 0xc, 0);
    end += classBytes;
    if (name != 0 && (flags & kNamed))
    {
        copyName(app, end, name);
        end += nameLength(app, end);
    }
    return end - block;
}

/* 0x4e11e0: the bytes a block of the class `flags` names takes past its body. */
u32 classBytes(App* app, u32 flags)
{
    return at(app, classOf(app, flags) + kClassBytes);
}

/* 0x4e11f0: the bytes a block of the class `flags` names, named `name`, takes past its body: the
 * class's, and the name and its zero if the class counts names in. */
u32 blockBytes(App* app, u32 name, u32 flags)
{
    const u32 cls = classOf(app, flags);
    u32 nameBytes = 0;
    if (name != 0 && (byte(app, cls + kClassFlags + 1) & 1))
        nameBytes = nameLength(app, name);
    return at(app, cls + kClassBytes) + nameBytes;
}

/* 0x4e1580, 0x4e15b0: the size of class `cls`'s largest free block, or 0. */
u32 largestFree(App* app, u32 cls)
{
    const u32 block = largestFreeBlock(app, cls, 0, false).block;
    return block != 0 ? at(app, block + kBlockSize) : 0;
}

/* What creatememclass asks on the stack: how blocks are aligned, how the free block's start is, the
 * bytes each block takes past its body; whether blocks have "BEND" after them, the file and line,
 * their names; whether the class has a lock; what its creator said. */
struct ClassAsked
{
    u32 align, startAlign, blockBytes;
    bool bend, where, named, locked;
    u32 word;
};

/* What creatememclass's code leaves in ecx and edx: the HIGH block's mark's -- its name's bytes, or
 * its flags -- and the free block's neighbour before it in the free list. */
struct Made
{
    u32 highMark, before;
};

/* 0x4e1230: class `index` (0 to 15, else an error reported, if errors are, and 0) made, named
 * `name`, over the `bytes` at `start`: its header in a block "<name> LOW" there, a free block from
 * where the start's alignment puts it past that, a block "<name> HIGH" at the end.  The free
 * block's size; nullopt when the game stopped. */
std::optional<u32> createClass(App* app, ThreadState& ts, u32 index, u32 name, u32 start, u32 bytes,
                                 const ClassAsked& asked, Made& made)
{
    // The code's frame: three registers kept, then 0x110 bytes, the names made at its bottom.
    const GuestFrame frame(ts, 0xc, 0x110);
    const u32 names = ts.sp;
    if (s32(index) < 0 || s32(index) >= 0x10)
    {
        if (at(app, kReportErrors) != 0
            && !error(app, ts, kCreateName, kCreateLine, {kCreateOutOfRange, index, 0x10}))
            return std::nullopt;
        return 0;
    }
    u32 flags = index;
    if (asked.bend)
        flags |= kBend;
    if (asked.where)
        flags |= kWhere;
    if (asked.named)
        flags |= kNamed;
    const u32 alignLess = asked.startAlign - 1;
    const u32 freeBlock = ((start + 0x50 + asked.blockBytes + 0x18 + alignLess) & ~alignLess) - kBlockHeader;
    const u32 header = start + kBlockHeader;
    const u32 high = start + bytes - 0x18 - asked.blockBytes;
    if (!print(app, ts, names, kLowName, name))
        return std::nullopt;
    markBlock(app, start, flags | kLowFlag, kClassHeader, asked.blockBytes, names, 0, freeBlock);
    markBlock(app, freeBlock, flags, high - freeBlock - kBlockHeader, asked.blockBytes, 0, start, high);
    if (!print(app, ts, names, kHighName, name))
        return std::nullopt;
    const u32 highBytes = markBlock(app, high, flags | kHighFlags, 0, asked.blockBytes, names, freeBlock, 0);
    // The class's header: in the table, cleared, named.
    put(app, kClasses + (index & kClassNibble) * 4, header);
    if (!clear(app, ts, header, kClassHeader))
        return std::nullopt;
    copyName(app, header, name);
    const u32 sentinel = header + kClassSentinel;
    putHalf(app, sentinel, kSentinel);
    put(app, sentinel + kBlockSize, kSentinelSize);
    put(app, header + kClassStart, start);
    put(app, header + kClassEnd, high);
    put(app, sentinel + kListNext, sentinel);
    put(app, sentinel + kListPrevious, sentinel);
    put(app, header + kClassAlign, asked.align);
    put(app, header + kClassStartAlign, asked.startAlign);
    put(app, header + kClassBytes, asked.blockBytes);
    put(app, header + kClassFlags, flags);
    put(app, header + kClassWord, asked.word);
    put(app, header + kClassLock, 0);
    // The free block into the free list; the lock.
    const Neighbours neighbours = linkFree(app, header, freeBlock);
    if (asked.locked)
    {
        const std::optional<u32> lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
        if (!lock)
            return std::nullopt;
        put(app, header + kClassLock, *lock);
    }
    made.highMark = asked.named ? highBytes - kBlockHeader - asked.blockBytes : flags | kHighFlags;
    made.before = neighbours.before;
    return at(app, freeBlock + kBlockSize);
}

/* 0x4e14b0: class `index`'s lock freed, if it has one, and its header cleared.  It stays in the
 * table. */
bool releaseClass(App* app, ThreadState& ts, u32 index)
{
    const u32 cls = classOf(app, index);
    if (const u32 lock = at(app, cls + kClassLock);
        lock != 0 && !guest::direct<0x4eb360, ea::lockFreed>(app, ts, lock))
        return false;
    return clear(app, ts, cls, kClassHeader);
}

/* 0x4e1500: the first class from 1 on with none in the table (16 if all have, an error reported if
 * errors are); nullopt when the game stopped. */
std::optional<u32> freeClass(App* app, ThreadState& ts)
{
    u32 index = 1;
    if (at(app, kClasses + 4) != 0)
    {
        do
            ++index;
        while (index < 0x10 && at(app, kClasses + index * 4) != 0);
    }
    if (at(app, kReportErrors) != 0 && index >= 0x10)
    {
        // ecx, edx, ebp and edi pushed.
        const GuestFrame frame(ts, 0x10, 0);
        if (!error(app, ts, kGetFreeName, kGetFreeLine, {kNoFreeClasses, 0x10}))
            return std::nullopt;
    }
    return index;
}

}  // namespace

/* 0x4e1620: `size` bytes (8 at least) of the class `flags` names, named `name` (or 0), flags `flags`
 * with the class's 0x700 -- under the class's lock, if it has one: the first free block large
 * enough from the bottom (from the top, flag 0x10; the largest, 0x20), split if more than 0x40
 * bytes are left over -- the rest left free below (above, from the top) -- and marked in use (its
 * size the one asked for, 0x100 kept only for a name).  The body's address, or 0 for none large
 * enough or a size less than 0, each an error reported if errors are; nullopt when the game
 * stopped. */
std::optional<u32> reserveBlock(App* app, ThreadState& ts, u32 name, u32 size, u32 flags, ReserveLeft& left)
{
    // The code's frame: four registers kept, then 0x14 bytes.
    const GuestFrame frame(ts, 0x10, 0x14);
    const u32 cls = classOf(app, flags);
    left = {flags, 0};
    if (const u32 lock = at(app, cls + kClassLock);
        lock != 0 && !os::locks::enter(app, ts, lock))
        return std::nullopt;
    if (s32(size) < 0)
    {
        if (const u32 lock = at(app, cls + kClassLock);
            lock != 0
            && !os::locks::leave(app, ts, lock))
            return std::nullopt;
        if (at(app, kReportErrors) != 0)
        {
            left.ebx = kReserveName;
            if (!error(app, ts, kReserveName, kInvalidSizeLine,
                       {kInvalidSize, name, size, largestFree(app, cls), flags}))
                return std::nullopt;
        }
        return 0;
    }
    // What the block takes, aligned; a block for it.
    const u32 body = s32(size) < kLeastBody ? u32(kLeastBody) : size;
    const u32 alignLess = at(app, cls + kClassAlign) - 1;
    const u32 needed =
        ((blockBytes(app, name, flags) + body + kBlockHeader + alignLess) & ~alignLess) - kBlockHeader;
    const bool fromTop = (flags & kFromTop) != 0;
    u32 block;
    if (flags & kFromLargest)
    {
        const LargestFree largest = largestFreeBlock(app, cls, needed, fromTop);
        block = largest.block;
        left = {fromTop ? at(app, largest.stop + kBlockSize) : 0, u32(largest.size)};
    }
    else
    {
        block = firstFreeBlock(app, cls, i32(needed), fromTop);
        left = {flags & kFromTop, needed};
    }
    u32 result = 0;
    if (block == 0)
    {
        // None large enough.
        if (at(app, kReportErrors) != 0)
        {
            u32 held = at(app, cls + kClassLock);
            if (held != 0
                && !os::locks::leave(app, ts, held))
                return std::nullopt;
            left = {kFileName, name};
            if (!error(app, ts, kReserveName, kOutOfMemoryLine,
                       {kOutOfMemory, name, size, largestFree(app, cls), flags}))
                return std::nullopt;
            return 0;
        }
    }
    else
    {
        unlinkFree(app, block);
        const i32 rest = i32(at(app, block + kBlockSize) - needed);
        if (rest > kSplitAbove)
        {
            if (fromTop)
            {
                // From the top: the rest, aligned down, left free below.
                const u32 low = block;
                block = low + (u32(rest) & ~alignLess);
                put(app, at(app, low + kBlockNext) + kBlockPrevious, block);
                put(app, block + kBlockNext, at(app, low + kBlockNext));
                put(app, block + kBlockPrevious, low);
                markBlock(app, low, 0, 0, 0, 0, at(app, low + kBlockPrevious), block);
                linkFree(app, cls, low);
            }
            else
            {
                // From the bottom: the rest left free above.
                const u32 above = block + kBlockHeader + needed;
                put(app, at(app, block + kBlockNext) + kBlockPrevious, above);
                markBlock(app, above, 0, 0, 0, 0, block, at(app, block + kBlockNext));
                linkFree(app, cls, above);
                put(app, block + kBlockNext, above);
            }
        }
        // The block marked in use.
        u32 marked = flags | (at(app, cls + kClassFlags) & kClassFlagsAdded);
        if (name == 0)
            marked &= ~kNamed;
        const u32 bytes = markBlock(app, block, marked, size, at(app, cls + kClassBytes), name,
                                      at(app, block + kBlockPrevious), at(app, block + kBlockNext));
        left = {(marked & kWhere) ? at(app, kCalledFile) : size, block + bytes};
        result = block + kBlockHeader;
    }
    if (const u32 lock = at(app, cls + kClassLock);
        lock != 0 && !os::locks::leave(app, ts, lock))
        return std::nullopt;
    return result;
}

/* 0x4e1890: the block at `body` given back to its class, under its lock if it has one: merged with
 * the block before it and the one after if they are free, into the free list.  1 -- for none too;
 * 0 for one not in use ("MB"), an error reported, if errors are, with its signature; nullopt when
 * the game stopped. */
std::optional<u32> purgeBlock(App* app, ThreadState& ts, u32 body)
{
    if (body == 0)
        return 1;
    // Six registers kept.
    const GuestFrame frame(ts, 0x18, 0);
    const u32 block = body - kBlockHeader;
    const u32 cls = classOf(app, byte(app, block + kBlockFlags));
    if (half(app, block) != kInUse)
    {
        if (at(app, kReportErrors) != 0
            && !error(app, ts, kPurgeName, kPurgeLine,
                      {kNotABlock, body, block, __builtin_bswap32(at(app, block)) >> 16}))
            return std::nullopt;
        return 0;
    }
    if (const u32 lock = at(app, cls + kClassLock);
        lock != 0 && !os::locks::enter(app, ts, lock))
        return std::nullopt;
    u32 merged = block;
    u32 before = at(app, block + kBlockPrevious);
    u32 next = at(app, block + kBlockNext);
    if (byte(app, before + kBlockFlagsHigh) & kInFreeList)
    {
        // Into the free block before it.
        unlinkFree(app, before);
        merged = before;
        before = at(app, before + kBlockPrevious);
        put(app, merged + kBlockNext, next);
        put(app, before + kBlockNext, merged);
        put(app, next + kBlockPrevious, merged);
    }
    if (byte(app, next + kBlockFlagsHigh) & kInFreeList)
    {
        // The free block after it into it.
        unlinkFree(app, next);
        next = at(app, next + kBlockNext);
        put(app, merged + kBlockPrevious, before);
        put(app, merged + kBlockNext, next);
        put(app, next + kBlockPrevious, merged);
    }
    linkFree(app, cls, merged);
    if (const u32 lock = at(app, cls + kClassLock);
        lock != 0 && !os::locks::leave(app, ts, lock))
        return std::nullopt;
    return 1;
}

namespace
{

/* 0x4e1990: every block of class `index` given back, from the start of its memory on, the next of
 * each read after it is -- its LOW and HIGH blocks too, which are not free blocks to give back.
 * `answer` the last one's, as it is for none; false when the game stopped. */
bool purgeClass(App* app, ThreadState& ts, u32 index, u32& answer)
{
    // edx pushed, and each call's return address.
    const GuestFrame frame(ts, 8, 0);
    for (u32 block = at(app, classOf(app, index) + kClassStart); block != 0; block = at(app, block + kBlockNext))
    {
        const std::optional<u32> given = purgeBlock(app, ts, block + kBlockHeader);
        if (!given)
            return false;
        answer = *given;
    }
    return true;
}

/* 0x4e19d0: `size` bytes of the default class, not named -- what the files' code (0x4eb160) takes
 * a buffer with. */
std::optional<u32> reserveDefault(App* app, ThreadState& ts, u32 size, ReserveLeft& left)
{
    // ebx pushed, and the call's return address.
    const GuestFrame frame(ts, 8, 0);
    return reserveBlock(app, ts, 0, size, at(app, kDefaultClass), left);
}

/* 0x4e19f0: the block at `body` given back, as the files' code (0x4eb160) frees its buffer. */
std::optional<u32> purgeDefault(App* app, ThreadState& ts, u32 body)
{
    // The call's return address.
    const GuestFrame frame(ts, 4, 0);
    return purgeBlock(app, ts, body);
}

}  // namespace

/* 0x4ea4f0: `count` bytes from `from` to `to`: backwards where `to` is within them above `from`
 * (memmove's backwards is the code's byte-at-a-time loop from the end), else by the forward
 * copy. */
bool moveMemory(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    if (!overlapsFromAbove(to, from, count))
        return forwardCopy(app, ts, to, from, count);
    u8* const memory = &app->getMemory<u8>(0);
    std::memmove(memory + to, memory + from, count);
    return true;
}

nfs::u32 findFreeBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 findLargestFreeBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 linkFreeBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 unlinkFreeBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 clearMemClasses(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 markMemBlock(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memClassOverhead(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockOverhead(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 creatememclass(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 releasememclass(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 getfreememclass(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 largestmem(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 largestmemclass(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockSize(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockName(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockFlags(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 reservememadr(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 purgememadr(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 purgememclass(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 reservememStdcall(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 purgememStdcall(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 memmove(App* app, ThreadState& ts, nfs::Args& args);

/* 0x4e1000: firstFreeBlock of class eax, edx bytes, from the end for ebx not 0. */
nfs::u32 findFreeBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = firstFreeBlock(app, args.eax, i32(args.edx), args.ebx != 0);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1030: largestFreeBlock of class eax, edx bytes, from the end for ebx not 0: edx left its
 * size, ebx (from the end) the size of the sentinel the walk stopped at. */
nfs::u32 findLargestFreeBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const bool fromEnd = args.ebx != 0;
    const LargestFree largest = largestFreeBlock(app, args.eax, args.edx, fromEnd);
    args.eax = largest.block;
    args.edx = u32(largest.size);
    if (fromEnd)
        args.ebx = at(app, largest.stop + kBlockSize);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1080: linkFree of block edx into class eax: eax left the block after it in the list, its
 * second byte the block's flags' high byte; edx the one before. */
nfs::u32 linkFreeBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 block = args.edx;
    const Neighbours neighbours = linkFree(app, args.eax, block);
    args.eax = listLeft(app, neighbours.after, block);
    args.edx = neighbours.before;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e10e0: unlinkFree of block edx (eax, its class, not looked at): eax left the block before it
 * in the list, its second byte the block's flags' high byte. */
nfs::u32 unlinkFreeBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 block = args.edx;
    const u32 before = unlinkFree(app, block);
    args.eax = listLeft(app, before, block);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1100: the class table cleared; 1.  No callers in the game. */
nfs::u32 clearMemClasses(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const EspKept kept(ts); !clear(app, ts, kClasses, 0x10 * 4))
        return args.eax;
    args.eax = 1;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1120: markBlock of block eax, flags the first stack word, size ebx, ecx bytes of the
 * class's, name edx, the blocks before and after the second and third: eax its bytes, edx past
 * them, ebx the file the flags have it take (else the size), ecx the name's bytes (else the
 * flags); the three words taken off. */
nfs::u32 markMemBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 block = args.eax, size = args.ebx, bytesOfClass = args.ecx, name = args.edx;
    const u32 flags = at(app, ts.sp + 4), before = at(app, ts.sp + 8), after = at(app, ts.sp + 0xc);
    const u32 bytes = markBlock(app, block, flags, size, bytesOfClass, name, before, after);
    args.eax = bytes;
    args.edx = block + bytes;
    if (flags & kWhere)
        args.ebx = at(app, kCalledFile);
    args.ecx = name != 0 && (flags & kNamed) ? bytes - kBlockHeader - size - bytesOfClass : flags;
    ts.sp += 4 + 0xc;
    return args.eax;
}

/* 0x4e11e0: classBytes of the class eax names. */
nfs::u32 memClassOverhead(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = classBytes(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e11f0: blockBytes of a block of class edx named eax; edx left the class. */
nfs::u32 memBlockOverhead(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 flags = args.edx;
    args.eax = blockBytes(app, args.eax, flags);
    args.edx = classOf(app, flags);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1230: createClass of class eax named edx over the ecx bytes at ebx, the stack's eight words
 * what it asks (ClassAsked): eax the free block's size, ebx the file HIGH's mark takes (else 0),
 * ecx and edx as Made; for a class out of range, edx the error's line if one was reported.  The
 * eight words taken off. */
nfs::u32 creatememclass(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 index = args.eax, name = args.edx, start = args.ebx, bytes = args.ecx;
    const auto word = [&](u32 i) { return at(app, ts.sp + 4 + 4 * i); };
    const ClassAsked asked = {word(0),      word(1),      word(2),      word(3) != 0,
                              word(4) != 0, word(5) != 0, word(6) != 0, word(7)};
    Made made = {};
    std::optional<u32> freeBytes;
    {
        const EspKept kept(ts);
        freeBytes = createClass(app, ts, index, name, start, bytes, asked, made);
    }
    if (!freeBytes)
        return args.eax;
    args.eax = *freeBytes;
    if (s32(index) < 0 || s32(index) >= 0x10)
    {
        if (at(app, kReportErrors) != 0)
            args.edx = kCreateLine;
    }
    else
    {
        args.ebx = asked.where ? at(app, kCalledFile) : 0;
        args.ecx = made.highMark;
        args.edx = made.before;
    }
    ts.sp += 4 + 0x20;
    return args.eax;
}

/* 0x4e14b0: releaseClass of class eax; 1.  No callers in the game. */
nfs::u32 releasememclass(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const EspKept kept(ts); !releaseClass(app, ts, args.eax))
        return args.eax;
    args.eax = 1;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1500: freeClass.  No callers in the game. */
nfs::u32 getfreememclass(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> index;
    {
        const EspKept kept(ts);
        index = freeClass(app, ts);
    }
    if (!index)
        return args.eax;
    args.eax = *index;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1580: largestFree of the default class. */
nfs::u32 largestmem(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = largestFree(app, classOf(app, at(app, kDefaultClass)));
    ts.sp += 4;
    return args.eax;
}

/* 0x4e15b0: largestFree of class eax. */
nfs::u32 largestmemclass(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = largestFree(app, classOf(app, args.eax));
    ts.sp += 4;
    return args.eax;
}

/* 0x4e15d0: the size asked for of the block whose body is at `body`. */
u32 askedSize(App* app, u32 body)
{
    return at(app, body - kBlockHeader + kBlockSize);
}

/* 0x4e15d0: askedSize of the block at eax. */
nfs::u32 memBlockSize(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = askedSize(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e15e0: the name of the block at eax, past its body and its class's bytes; 0 for one not
 * named (flag 0x100). */
u32 blockNameOf(App* app, u32 body)
{
    const u32 block = body - kBlockHeader;
    const u32 flags = half(app, block + kBlockFlags);
    return (flags & kNamed) ? classBytes(app, flags) + body + at(app, block + kBlockSize) : 0;
}

nfs::u32 memBlockName(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = blockNameOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1610: the flags of the block at eax.  No callers in the game. */
nfs::u32 memBlockFlags(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = half(app, args.eax - kBlockHeader + kBlockFlags);
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1620: reserveBlock of edx bytes named eax, flags ebx; ebx and edx as ReserveLeft. */
nfs::u32 reservememadr(App* app, ThreadState& ts, nfs::Args& args)
{
    ReserveLeft left = {};
    std::optional<u32> body;
    {
        const EspKept kept(ts);
        body = reserveBlock(app, ts, args.eax, args.edx, args.ebx, left);
    }
    if (!body)
        return args.eax;
    args.eax = *body;
    args.ebx = left.ebx;
    args.edx = left.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1890: purgeBlock of the body at eax. */
nfs::u32 purgememadr(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> answer;
    {
        const EspKept kept(ts);
        answer = purgeBlock(app, ts, args.eax);
    }
    if (!answer)
        return args.eax;
    args.eax = *answer;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e1990: purgeClass of class eax: eax the last block's answer, or the class's number for none.
 * No callers in the game. */
nfs::u32 purgememclass(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 index = args.eax & kClassNibble;
    u32 answer = index;
    if (const EspKept kept(ts); !purgeClass(app, ts, index, answer))
        return args.eax;
    args.eax = answer;
    ts.sp += 4;
    return args.eax;
}

/* 0x4e19d0: reserveDefault of the stack word's bytes; edx as reservememadr leaves it; stdcall. */
nfs::u32 reservememStdcall(App* app, ThreadState& ts, nfs::Args& args)
{
    ReserveLeft left = {};
    std::optional<u32> body;
    {
        const EspKept kept(ts);
        body = reserveDefault(app, ts, at(app, ts.sp + 4), left);
    }
    if (!body)
        return args.eax;
    args.eax = *body;
    args.edx = left.edx;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4e19f0: purgeDefault of the stack word's body; stdcall. */
nfs::u32 purgememStdcall(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> answer;
    {
        const EspKept kept(ts);
        answer = purgeDefault(app, ts, at(app, ts.sp + 4));
    }
    if (!answer)
        return args.eax;
    args.eax = *answer;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4ea4f0: moveMemory of ebx bytes from eax to edx: backwards, eax and edx left where they were
 * and ebx all ones (the count run past 0); forwards, the three as the forward copy leaves them --
 * eax past the bytes copied to, edx past those copied from, ebx 0. */
nfs::u32 memmove(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    if (!moveMemory(app, ts, to, from, count))
        return args.eax;
    if (overlapsFromAbove(to, from, count))
    {
        args.eax = from;
        args.edx = to;
        args.ebx = 0xffffffff;
    }
    else
    {
        args.eax = to + count;
        args.edx = from + count;
        args.ebx = 0;
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x4e19d0's and 0x4e19f0's typed cores (native_ea_memory.h): as their thunks run them. */
u32 defaultLargest(App* app)
{
    return largestFree(app, classOf(app, at(app, kDefaultClass)));
}

u32 freeLinked(App* app, u32 cls, u32 block)
{
    const Neighbours neighbours = linkFree(app, cls, block);
    return listLeft(app, neighbours.after, block);
}

u32 freeUnlinked(App* app, u32 cls, u32 block)
{
    (void)cls;  // not looked at
    return listLeft(app, unlinkFree(app, block), block);
}

u32 blockOverhead(App* app, u32 name, u32 flags)
{
    return blockBytes(app, name, flags);
}

u32 blockMarked(App* app, u32 block, u32 name, u32 size, u32 bytesOfClass, u32 flags, u32 before,
                  u32 after)
{
    return markBlock(app, block, flags, size, bytesOfClass, name, before, after);
}

std::optional<u32> defaultReserved(App* app, ThreadState& ts, u32 size)
{
    ReserveLeft left = {};
    return reserveDefault(app, ts, size, left);
}

std::optional<u32> defaultPurged(App* app, ThreadState& ts, u32 body)
{
    return purgeDefault(app, ts, body);
}

}  // namespace ea
}  // namespace nfs3hp
