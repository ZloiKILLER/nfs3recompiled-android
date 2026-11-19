#include "native_ea_files.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend_glue.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "port_snd_stream_calls.h"
#include "os_threads.h"
#include <lib/app.h>

#include <initializer_list>
#include <optional>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* EA's formats and keys under the menus: a shape located in its file
 * (cmn\locatshp.c), the keys (win\key.c: a key's character, the key hook's
 * key, inkey, the game's keys flushed), the packed data (cmn\unpack.c,
 * cmn\loadpack.c: unpack, unpacksize, loadpackadr), and between them in the
 * code the secondary timer's end (win\lowtimer.c: a callback removed, the
 * thread stopped).  Each address is bound to the
 * function of that name here (EA_FORMATS_NATIVES), and the game's code
 * calls it by name.
 *
 * A shape file ("SHPI") has its count at +8 and from +0x10 an entry of 8
 * bytes for each shape: its name, four characters, and where the shape is
 * from the file's start.
 *
 * The keys: their state a byte each by scan code at 0x564818 (Ctrl's
 * 0x564835, the Shifts' 0x564842 and 0x56484e, Alt's 0x564850); Ctrl+Esc
 * leaving the game when 0x564810 is set; Caps Lock's state at 0x564814,
 * flipped by its scan code (0x3a).  A scan code's character is in one of
 * five tables of 0x5b bytes -- plain (0x564898), shifted (0x5648f3), with
 * Caps Lock (0x56494e), with Ctrl (0x5649a9), with Alt (0x564a04) -- and
 * one of 0x80 or more is a key of its own: shifted up a byte (0x90 on
 * without their top bit).  The key hook (0x5651d0, set by the window's
 * code) hands back the next key, 0 for none.
 *
 * Packed data starts with its code -- 0x10, 0x18, 0x30, 0x32, 0x34, 0x46 or
 * 0x4a and that plus 1 -- then 0xfb (or 0x32), then its size unpacked in
 * three bytes, big-endian; each code's own unpacker (0x5102a4, 0x510760,
 * 0x50ed50, 0x510870, 0x510110) unpacks it from eax to edx.  A code not known
 * is an Error when the game's errors are on ([0x56440c] for unpack).
 *
 * The secondary timer: a thread (0x4f27a0, its id at 0x5652ac, 0 once it
 * has ended, its record at 0x9f0c20) that calls the eight callbacks of
 * 0x9f0c00 in turn, under the lock at 0x9f0c34 (0 when there is no timer).
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * eax, and the registers the code does not keep, as the code leaves them --
 * what a callee left there flows through (an unpacker's, the timer's last
 * call's).  This file's own are called as C++, at the depth the code calls
 * them from.  Other files' typed functions -- the memory manager's, a file
 * loaded, the surface's check, the threads' sleep, event, lock and thread
 * asked, the DXT1 and byte pairs' unpackers -- are called as C++
 * (guest::direct, their headers), at the code's depth, what an unpacker
 * leaves in edx put there from what it gives back.  The key hook is called
 * through its slot, Windows' functions through their imports' slots; every
 * other function of the game's by address -- Huffman's unpacker, which no
 * header has, and the runs' whose ebx and edx the code takes as it leaves
 * them -- the registers it keeps put back (EspKept), at the code's
 * depth: Error hands vsprintf its arguments' address.  The waits' loops have
 * the game's code's safepoints. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using nfs::u32;
using nfs::u8;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// Where an Error is from: the file, the function, the line.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
// The game's errors on.
constexpr u32 kErrorsFatal = 0x56440c;
// The keys: Ctrl, the left and right Shifts, Alt held; Ctrl+Esc leaving the
// game; Caps Lock on.
constexpr u32 kCtrlHeld = 0x564835, kLeftShiftHeld = 0x564842, kRightShiftHeld = 0x56484e;
constexpr u32 kAltHeld = 0x564850, kCtrlEscapeQuits = 0x564810, kCapsLock = 0x564814;
constexpr u32 kCapsLockKey = 0x3a, kEscapeKey = 1;
// The characters by scan code: plain, shifted, with Caps Lock, Ctrl, Alt.
constexpr u32 kPlainKeys = 0x564898, kShiftedKeys = 0x5648f3, kCapsKeys = 0x56494e;
constexpr u32 kCtrlKeys = 0x5649a9, kAltKeys = 0x564a04;
// A character of 0x80 or more is a key of its own; from 0x90 without its top bit.
constexpr u32 kOwnKey = 0x80, kOwnKeyHigh = 0x90;
// The key hook.
constexpr u32 kKeyHook = 0x5651d0;
// The secondary timer: its callbacks, its lock, its thread's record and id.
constexpr u32 kTimerCallbacks = 0x9f0c00, kLastTimerCallback = 0x9f0c1c, kTimerLock = 0x9f0c34;
constexpr u32 kTimerThread = 0x9f0c20, kTimerThreadId = 0x5652ac;
constexpr u32 kTimerSlots = 8;
// The game's strings: win\key.c's, cmn\unpack.c's, cmn\loadpack.c's.
constexpr u32 kKeyC = 0x54c660, kInkeyName = 0x54c66c, kInkeyLine = 0x1da;
constexpr u32 kUnpackC = 0x54cb68, kUnpackName = 0x54cb78, kUnpackInvalid = 0x54cb80, kUnpackLine = 0x6d;
constexpr u32 kUnpackSizeName = 0x54cba4, kUnpackSizeInvalid = 0x54cbb0, kUnpackSizeLine = 0xc9;
constexpr u32 kLoadpackC = 0x54cc34, kLoadpackName = 0x54cc44, kLoadpackLine = 0xaf, kLoadpackLine2 = 0xb6;
// A shape file: its count, its entries (a name and where the shape is from the file's start).
constexpr u32 kShapeCount = 8, kShapeEntries = 0x10, kShapeEntry = 8, kShapeOffset = 4;
// Packed data: its code, its mark, its size unpacked (three bytes, big-endian).
constexpr u32 kPackMarkAt = 1, kPackSize = 2;
constexpr u8 kPackMark = 0xfb, kOtherPackMark = 0x32;
// The codes an Error is reported for: those that look packed.
constexpr u8 kLeastPackCode = 0x10, kPastPackCodes = 0x80;
// A block's flag: taken from the top of its class.
constexpr u32 kFromTop = 0x10;
// Windows' functions, by their imports' slots.
// The game's functions, by address.
constexpr u32 kExit = 0x4dfd04;
// The unpackers called by address, by code.
constexpr u32 kUnpack10 = 0x5102a4, kUnpack30 = 0x50ed50, kUnpack4a = 0x510110;

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

/* The big-endian number of `bytes` bytes at `address`. */
u32 bigEndian(App* app, u32 address, u32 bytes)
{
    return __builtin_bswap32(at(app, address)) >> ((0x20 - 8 * bytes) & 0x1f);
}

/* Whether the data at `data` is marked packed: 0xfb (or 0x32) past its code. */
bool marked(App* app, u32 data)
{
    return byte(app, data + kPackMarkAt) == kPackMark || byte(app, data + kPackMarkAt) == kOtherPackMark;
}

/* Whether `code` (its low bit aside) has an unpacker. */
bool knownCode(u8 code)
{
    switch (code & 0xfe)
    {
    case 0x10:
    case 0x18:
    case 0x30:
    case 0x32:
    case 0x34:
    case 0x46:
    case 0x4a:
        return true;
    default:
        return false;
    }
}

/* Whether the data at `data` looks packed, for an Error: a code of 0x10 to 0x7f, 0xfb. */
bool looksPacked(App* app, u32 data)
{
    const u8 code = byte(app, data);
    return code >= kLeastPackCode && code < kPastPackCodes && byte(app, data + kPackMarkAt) == kPackMark;
}

/* The Error of a code not known, cmn\unpack.c's `function` at `line` where it is from: `format`,
 * the data's two first bytes (big-endian) its argument; cdecl, at esp as it is, every register but
 * eax kept. */
bool packError(App* app, ThreadState& ts, u32 function, u32 line, u32 format, u32 data)
{
    put(app, kWhereFile, kUnpackC);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
    guest::direct<0x401010, ea::error>(app, ts, format, guest::Words({bigEndian(app, data, 2)}));
    return !ts.terminate;
}

/* A code's unpacker in native_ea_net.cpp without a typed function (0x5102a4, 0x50ed50), by
 * address: `data` unpacked to `to`, ebx 1 at the call (the tables read); its result, or nullopt when
 * the game stopped. */
std::optional<u32> runUnpacker(App* app, ThreadState& ts, u32 target, u32 data, u32 to)
{
    return guest::pointerCall(app, ts, target, {data, to, 1});
}

/* 0x4ef270: the shape named by the four characters at `name` in the shape file at `file` -- the
 * last of that name -- or 0; and the entry its search stopped at (0 in a file of none). */
struct ShapeFound
{
    u32 shape, entry;
};

ShapeFound findShape(App* app, u32 file, u32 name)
{
    u32 count = at(app, file + kShapeCount);
    const u32 key = at(app, name);
    if (count == 0)
        return {0, 0};
    do
        --count;
    while (count != 0 && key != at(app, file + count * kShapeEntry + kShapeEntries));
    const u32 entry = file + count * kShapeEntry;
    if (key != at(app, entry + kShapeEntries))
        return {0, entry};
    return {at(app, entry + kShapeEntries + kShapeOffset) + file, entry};
}

/* 0x4f0010: the character of scan code `code` as the keys held say -- Alt's, Ctrl's, Shift's, Caps
 * Lock's or the plain one; 0x80 to 0x8f shifted up a byte, 0x90 on without their top bit and
 * shifted up a byte.  Caps Lock's own key flips its state, 0 back; Ctrl+Esc leaves the game (exit
 * 0) when 0x564810 says so.  nullopt when the game stopped. */
std::optional<u32> scanCharacter(App* app, ThreadState& ts, u32 code, u32 callerEcx)
{
    if (code == kCapsLockKey)
    {
        put(app, kCapsLock, at(app, kCapsLock) == 0 ? 1 : 0);
        return 0;
    }
    u32 table;
    if (byte(app, kAltHeld) != 0)
        table = kAltKeys;
    else if (byte(app, kCtrlHeld) != 0)
    {
        if (at(app, kCtrlEscapeQuits) != 0 && code == kEscapeKey)
        {
            // exit(0), the game's end, the code then
            // reads the table at what ecx holds there: the caller's (`callerEcx`), as exit leaves
            // it.  ebx and ecx pushed.
            const GuestFrame pushed(ts, 8, 0);
            guest::direct<kExit, watcom::clib::exit>(app, ts, 0u);
            if (ts.terminate)
                return std::nullopt;
            table = callerEcx;
        }
        else
            table = kCtrlKeys;
    }
    else if (byte(app, kLeftShiftHeld) != 0 || byte(app, kRightShiftHeld) != 0)
        table = kShiftedKeys;
    else if (at(app, kCapsLock) != 0)
        table = kCapsKeys;
    else
        table = kPlainKeys;
    u32 character = byte(app, table + code);
    if (character >= kOwnKey)
    {
        if (character >= kOwnKeyHigh)
            character &= 0x7f;
        character <<= 8;
    }
    return character;
}

/* 0x4f0100: the key hook's next key, 0 for none or without a hook (the hook, a pointer in the
 * game's memory, called by it).  nullopt when the game stopped. */
std::optional<u32> hookKey(App* app, ThreadState& ts)
{
    const u32 hook = at(app, kKeyHook);
    if (hook == 0)
        return 0;
    return guest::pointerCall(app, ts, hook, {});
}

}  // namespace

/* 0x4f0120: inkey -- win\key.c's, files not to be touched with a surface locked
 * (checkSurfaceUnlocked) -- then the game's tasks run (0x4e7630) until the key hook gives a key.
 * nullopt when the game stopped. */
std::optional<u32> waitKey(App* app, ThreadState& ts)
{
    put(app, kWhereFile, kKeyC);
    put(app, kWhereFunction, kInkeyName);
    put(app, kWhereLine, kInkeyLine);
    {
        // edx, ecx and ebx pushed: Error hands vsprintf its arguments' address.
        const GuestFrame pushed(ts, 0xc, 0);
        if (!guest::direct<0x501010, ea::surfaceUnlocked>(app, ts, kInkeyName))
            return std::nullopt;
    }
    u32 key;
    do
    {
        if (app->contextWanted())
            app->yieldContext(ts);
        // Its result is nobody's: the key hook (0x4f00e0, the only one set) does not read eax.
        const port::Result done = guest::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
        if (!done)
            return std::nullopt;
        std::optional<u32> next;
        {
            // The call's return address.
            const GuestFrame call(ts, 4, 0);
            next = hookKey(app, ts);
        }
        if (!next)
            return std::nullopt;
        key = *next;
    } while (key == 0);
    return key;
}

namespace
{

/* 0x4f0240: the keys the menus have (0x4cc970) taken until there are none. */
bool flushMenuKeys(App* app, ThreadState& ts)
{
    for (;;)
    {
        if (app->contextWanted())
            app->yieldContext(ts);
        const u32 key = guest::direct<0x4cc970, frontend::glue::nextKey>(app, ts);
        if (ts.terminate)
            return false;
        if (key == 0)
            return true;
    }
}

/* 0x4f2560: the packed data at `data` unpacked to `to` by its code's unpacker; the size unpacked,
 * 0 for a code not known -- without an Error.  nullopt when the game stopped. */
std::optional<u32> unpackTo(App* app, ThreadState& ts, u32 data, u32 to)
{
    if (!marked(app, data))
        return 0;
    // ebx and esi pushed.
    const GuestFrame pushed(ts, 8, 0);
    switch (byte(app, data) & 0xfe)
    {
    case 0x10:
        return runUnpacker(app, ts, kUnpack10, data, to);
    case 0x18:
    {
        // The blocks too.
        const Regs left = guest::direct<0x510760, ea::unpackDxt>(app, ts, data, to, 1u);
        if (ts.terminate)
            return std::nullopt;
        return left.eax;
    }
    case 0x30:
    case 0x32:
    case 0x34:
        return runUnpacker(app, ts, kUnpack30, data, to);
    case 0x46:
    {
        const Regs left = guest::direct<0x510870, ea::unpackPairs>(app, ts, data, to);
        if (ts.terminate)
            return std::nullopt;
        return left.eax;
    }
    case 0x4a:
    {
        // ecx pushed over it.  The size handed in ebx, which the unpacker keeps, is the result.
        const GuestFrame kept(ts, 4, 0);
        const u32 size = bigEndian(app, data + kPackSize, 3);
        if (!guest::direct<kUnpack4a, ea::unpackRuns>(app, ts, data, to))
            return std::nullopt;
        return size;
    }
    default:
        return 0;
    }
}

}  // namespace

/* 0x4f2620: unpack -- unpackTo; a code not known an Error ("unpack - INVALID PACK CODE") when the
 * game's errors are on and the data looks packed.  The size unpacked, or 0; nullopt when the game
 * stopped. */
std::optional<u32> unpackChecked(App* app, ThreadState& ts, u32 data, u32 to)
{
    std::optional<u32> size;
    {
        // ebx and ebp pushed, and the call's return address.
        const GuestFrame call(ts, 0xc, 0);
        size = unpackTo(app, ts, data, to);
    }
    if (!size)
        return std::nullopt;
    if (*size != 0 || at(app, kErrorsFatal) == 0 || !looksPacked(app, data))
        return size;
    // ebx, ebp, edi, esi and ecx pushed.
    const GuestFrame pushed(ts, 0x14, 0);
    if (!packError(app, ts, kUnpackName, kUnpackLine, kUnpackInvalid, data))
        return std::nullopt;
    return size;
}

namespace
{

/* 0x4f26a0: the size the packed data at `data` unpacks to (its three bytes past the code and the
 * mark), 0 for a code not known -- without an Error. */
u32 packedSize(App* app, u32 data)
{
    if (!marked(app, data) || !knownCode(byte(app, data)))
        return 0;
    return bigEndian(app, data + kPackSize, 3);
}

}  // namespace

/* 0x4f2710: unpacksize -- packedSize; a code not known an Error ("unpacksize - INVALID PACK
 * CODE") where the data looks packed.  nullopt when the game stopped. */
std::optional<u32> packedSizeChecked(App* app, ThreadState& ts, u32 data)
{
    const u32 size = packedSize(app, data);
    if (size != 0 || !looksPacked(app, data))
        return size;
    // ebx, edx, edi, esi and ecx pushed.
    const GuestFrame pushed(ts, 0x14, 0);
    if (!packError(app, ts, kUnpackSizeName, kUnpackSizeLine, kUnpackSizeInvalid, data))
        return std::nullopt;
    return 0;
}

namespace
{

/* 0x4f2830: the secondary timer stopped (win\lowtimer.c; its exit function): its lock taken and
 * done with ([0x9f0c34] 0), its thread woken to end (its event set), and -- but on that thread
 * itself -- waited for until it has ended ([0x5652ac] 0).  What the last call leaves in eax --
 * whether this is the timer's thread (1 or 0), or the last sleep's -- `unchanged` without a
 * timer; nullopt when the game stopped. */
std::optional<u32> stopTimer(App* app, ThreadState& ts, u32 unchanged)
{
    const u32 lock = at(app, kTimerLock);
    if (lock == 0)
        return unchanged;
    // ebx, ecx and edx pushed.
    const GuestFrame pushed(ts, 0xc, 0);
    if (!os::locks::enter(app, ts, lock))
        return std::nullopt;
    const u32 thread = at(app, kTimerThreadId);
    put(app, kTimerLock, 0);
    if (!guest::direct<0x4dfb30, ea::eventSet>(app, ts, thread)
        || !os::locks::leave(app, ts, lock)
        || !guest::direct<0x4eb360, ea::lockFreed>(app, ts, lock))
        return std::nullopt;
    const std::optional<bool> onThread = guest::direct<0x4df930, ea::isThreadNow>(app, ts, kTimerThread);
    if (!onThread)
        return std::nullopt;
    u32 left = *onThread ? 1 : 0;
    if (*onThread)
        return left;
    for (;;)
    {
        if (app->contextWanted())
            app->yieldContext(ts);
        if (at(app, kTimerThreadId) == 0)
            return left;
        const std::optional<u32> slept = guest::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);
        if (!slept)
            return std::nullopt;
        left = *slept;
    }
}

}  // namespace

/* 0x4f29b0: the secondary timer's `callback` removed (win\lowtimer.c), those after it moved down a
 * slot, under its lock; the timer stopped (stopTimer) when none is left.  Nothing without a timer.
 * What the code leaves in eax -- the callback without a timer; 0, or the first slot past the first
 * still taken; what stopping the timer left -- or nullopt when the game stopped. */
std::optional<u32> removeTimer(App* app, ThreadState& ts, u32 callback)
{
    const u32 lock = at(app, kTimerLock);
    if (lock == 0)
        return callback;
    // ebx, ecx, edx and esi pushed.
    const GuestFrame pushed(ts, 0x10, 0);
    if (!os::locks::enter(app, ts, lock))
        return std::nullopt;
    u32 slot = 0;
    if (callback != at(app, kTimerCallbacks))
    {
        do
            ++slot;
        while (slot < kTimerSlots && callback != at(app, kTimerCallbacks + slot * 4));
    }
    if (slot < kTimerSlots && callback == at(app, kTimerCallbacks + slot * 4))
    {
        for (u32 i = slot + 1; i < kTimerSlots; ++i)
            put(app, kTimerCallbacks + (i - 1) * 4, at(app, kTimerCallbacks + i * 4));
        put(app, kLastTimerCallback, 0);
    }
    if (!os::locks::leave(app, ts, at(app, kTimerLock)))
        return std::nullopt;
    if (at(app, kTimerCallbacks) != 0)
        return 0;
    for (u32 i = 1; i < kTimerSlots; ++i)
        if (at(app, kTimerCallbacks + i * 4) != 0)
            return i * 4;
    // The call's return address; eax as the code has it there, what the lock's leave left: 0.
    const GuestFrame call(ts, 4, 0);
    return stopTimer(app, ts, 0);
}

namespace
{

/* What loadpackadr leaves: the block (the file as loaded, the block unpacked, or 0), and the block
 * given back last, which the code leaves in edx (0 for none). */
struct Loaded
{
    u32 block, freed;
};

/* 0x4f2e20: loadpackadr -- the file named `name` loaded (loadFile, `flags` its memory's) and, if
 * packed (unpacksize), unpacked: its packed bytes copied to a block of their own (flags `flags`
 * with 0x10 flipped) and the file's freed, a block of the size unpacked (`flags`) unpacked into,
 * the copy freed.  nullopt when the game stopped. */
std::optional<Loaded> loadPacked(App* app, ThreadState& ts, u32 name, u32 flags)
{
    // Five registers kept, then 0x10 bytes.
    const GuestFrame frame(ts, 0x14, 0x10);
    const std::optional<u32> loaded = guest::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, flags);
    if (!loaded)
        return std::nullopt;
    if (*loaded == 0)
        return Loaded{0, 0};
    std::optional<u32> unpacked;
    {
        // The call's return address.
        const GuestFrame call(ts, 4, 0);
        unpacked = packedSizeChecked(app, ts, *loaded);
    }
    if (!unpacked)
        return std::nullopt;
    if (*unpacked == 0)
        return Loaded{*loaded, 0};
    const u32 packedBytes = guest::direct<0x4e15d0, ea::askedSize>(app, ts, *loaded);
    if (ts.terminate)
        return std::nullopt;
    put(app, kWhereLine, kLoadpackLine);
    put(app, kWhereFunction, kLoadpackName);
    put(app, kWhereFile, kLoadpackC);
    const std::optional<u32> copy =
        guest::direct<0x4e1620, ea::reserved>(app, ts, name, packedBytes, flags ^ kFromTop);
    if (!copy)
        return std::nullopt;
    u32 block = 0, toFree = *loaded;
    if (*copy != 0)
    {
        toFree = *copy;
        if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, *copy, *loaded, packedBytes)
            || !guest::direct<0x4e1890, ea::purgeBlock>(app, ts, *loaded))
            return std::nullopt;
        put(app, kWhereFunction, kLoadpackName);
        put(app, kWhereLine, kLoadpackLine2);
        put(app, kWhereFile, kLoadpackC);
        const std::optional<u32> unpackedBlock =
            guest::direct<0x4e1620, ea::reserved>(app, ts, name, *unpacked, flags);
        if (!unpackedBlock)
            return std::nullopt;
        block = *unpackedBlock;
        if (block != 0)
        {
            const GuestFrame call(ts, 4, 0);
            if (!unpackChecked(app, ts, *copy, block))
                return std::nullopt;
        }
    }
    if (toFree == 0)
        return Loaded{block, 0};
    if (!guest::direct<0x4e1890, ea::purgeBlock>(app, ts, toFree))
        return std::nullopt;
    return Loaded{block, toFree};
}

}  // namespace

/* The cores of 0x4ef270, 0x4f0010, 0x4f0100 and 0x4f0240 for the menus' code (native_ea_formats.h). */
u32 shapeLocated(App* app, u32 file, u32 name)
{
    return findShape(app, file, name).shape;
}

std::optional<u32> scanCodeCharacter(App* app, ThreadState& ts, u32 code, u32 callerEcx)
{
    return scanCharacter(app, ts, code, callerEcx);
}

std::optional<u32> hookedKey(App* app, ThreadState& ts)
{
    return hookKey(app, ts);
}

std::optional<u32> menuKeysFlushed(App* app, ThreadState& ts)
{
    if (!flushMenuKeys(app, ts))
        return std::nullopt;
    return 0;
}

nfs::u32 locateShape(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 keyCharacter(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 keyFromHook(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 inkey(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 flushKeys(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 unpackQuietly(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 unpack(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 unpackSizeQuietly(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 unpacksize(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 stopLowTimer(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 removeLowTimer(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadpackadr(App* app, ThreadState& ts, nfs::Args& args);

/* 0x4ef270: findShape in the file at eax of the name at edx: eax the shape, edx left at the entry
 * the search stopped at (as it was in a file of none). */
nfs::u32 locateShape(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 file = args.eax;
    const ShapeFound found = findShape(app, file, args.edx);
    args.eax = found.shape;
    if (at(app, file + kShapeCount) != 0)
        args.edx = found.entry;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f0010: scanCharacter of scan code eax. */
nfs::u32 keyCharacter(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> character;
    {
        const EspKept kept(ts);
        character = scanCharacter(app, ts, args.eax, args.ecx);
    }
    if (!character)
        return args.eax;
    args.eax = *character;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f0100: hookKey. */
nfs::u32 keyFromHook(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> key = hookKey(app, ts);
    if (!key)
        return args.eax;
    args.eax = *key;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f0120: waitKey: eax the key. */
nfs::u32 inkey(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> key = waitKey(app, ts);
    if (!key)
        return args.eax;
    args.eax = *key;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f0240: flushMenuKeys: eax the last key taken, none. */
nfs::u32 flushKeys(App* app, ThreadState& ts, nfs::Args& args)
{
    if (!flushMenuKeys(app, ts))
        return args.eax;
    args.eax = 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f2560: unpackTo of the data at eax to edx: eax the size. */
nfs::u32 unpackQuietly(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> size = unpackTo(app, ts, args.eax, args.edx);
    if (!size)
        return args.eax;
    args.eax = *size;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f2620: unpackChecked of the data at eax to edx: eax and edx the size. */
nfs::u32 unpack(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> size = unpackChecked(app, ts, args.eax, args.edx);
    if (!size)
        return args.eax;
    args.eax = *size;
    args.edx = *size;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f26a0: packedSize of the data at eax. */
nfs::u32 unpackSizeQuietly(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = packedSize(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4f2710: packedSizeChecked of the data at eax. */
nfs::u32 unpacksize(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> size = packedSizeChecked(app, ts, args.eax);
    if (!size)
        return args.eax;
    args.eax = *size;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f2830: stopTimer: eax as its last call left it (the caller's without a timer). */
nfs::u32 stopLowTimer(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> left;
    {
        const EspKept kept(ts);
        left = stopTimer(app, ts, args.eax);
    }
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f29b0: removeTimer of callback eax. */
nfs::u32 removeLowTimer(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<u32> left;
    {
        const EspKept kept(ts);
        left = removeTimer(app, ts, args.eax);
    }
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x4f2e20: loadPacked of the file named eax, flags edx: eax the block; edx the block given back
 * last, or as loading the file left it. */
nfs::u32 loadpackadr(App* app, ThreadState& ts, nfs::Args& args)
{
    std::optional<Loaded> loaded;
    {
        const EspKept kept(ts);
        loaded = loadPacked(app, ts, args.eax, args.edx);
    }
    if (!loaded)
        return args.eax;
    args.eax = loaded->block;
    if (loaded->freed != 0)
        args.edx = loaded->freed;
    ts.sp += 4;
    return args.eax;
}

}  // namespace ea
}  // namespace nfs3hp
