#include "native_frontend_wina.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_loada.h"
#include "native_frontend_menz.h"
#include "native_frontend_netb.h"
#include "native_frontend_rendf.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_watc.h"
#include "port_snd_stream_calls.h"
// What lib/thread.h holds pointers to and the checks' stand-in for SDL does not declare.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include "native_windows.h"
#include <winapi/advapi32.h>
#include <winapi/ddraw/idirectdraw2.h>
#include <winapi/gdi32.h>
#include <winapi/kernel32.h>
#include <winapi/user32.h>
#include <winapi/version.h>
#include <cstdint>
#include <optional>

/* The layers under the movie player and over Windows, decompiled:
 *
 *   the buffered stream  sub_4cf1c0..sub_4d0500: a ring of chunks filled from a queue of
 *                        files or memory blocks, what a movie reads (the magic 'STRM');
 *   EA's file library    sub_4fb990..sub_4fd060: operations queued per priority slot;
 *   the window           sub_4cc650..sub_4cdc90: the message table, its handlers, the
 *                        thread that pumps the messages, the key queue;
 *   startup checks       sub_4a38c0, sub_4b5860, sub_4b5e50, sub_4b5f10, sub_4b6400..4b6590;
 *   and what is left of the graphics library, the palette, the shapes and the C runtime.
 *
 * All state stays in guest memory.  Windows' functions are the port's own, called as C++
 * (callImport, callThunk); in the checks through their imports' slots and thunks.  So are
 * DirectDraw's methods (callMethod); in the checks through the object's table.
 * What each function leaves in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::wina (native_frontend_wina.h), in a frame of
 * its own where the game's function has one, its stack words handed to it as parameters, and an
 * adapter under the game's name for the calls by address.  A register the code reads that the
 * function took from its caller is a parameter too, put back where the code reads it. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using win32::com::IUnknown;
using win32::ddraw::IDirectDraw2;
using win32::ddraw::IDirectDrawSurface;
using x86::reg32;
using x86::sreg32;

constexpr std::uint8_t kBx = fe::kRestoreEbx, kCx = fe::kRestoreEcx, kDx = fe::kRestoreEdx;
constexpr std::uint8_t kSi = fe::kRestoreEsi, kDi = fe::kRestoreEdi;

// Imports, by their slot in the table.
constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588;
constexpr reg32 kFreeLibrary = 0x5344dc, kLoadLibrary = 0x53458c, kGetProcAddress = 0x534558;
constexpr reg32 kLastError = 0x534534, kMessageBox = 0x534764, kClientToScreen = 0x534700;
constexpr reg32 kClientRect = 0x53473c, kForeground = 0x534788, kScreenMetric = 0x534750;
constexpr reg32 kShowCursor = 0x534794, kSystemParameters = 0x5347a0, kUnhook = 0x5347a8;
constexpr reg32 kSetHook = 0x534790, kRegOpen = 0x53446c, kRegQuery = 0x534470;
constexpr reg32 kAdjustRect = 0x5346ec, kSetCursor = 0x534780;
// version.dll's, by their import thunk (native_import_thunks.inc).
constexpr reg32 kVersionInfoSize = 0x533e98, kVersionInfo = 0x533e92, kQueryVersionValue = 0x533e8c;

// DirectDraw's methods, by their offset in an object's table (src/lib/winapi/ddraw).
constexpr reg32 kQueryInterface = 0x00, kRelease = 0x08, kCreateSurface = 0x18, kSetCooperativeLevel = 0x50;

// The game's functions, by address.
constexpr reg32 kFileOpened = 0x4cf570, kFileSwitched = 0x4cf5a0, kFilled = 0x4cf5e0;
constexpr reg32 kOpWait = 0x4fc600;
constexpr reg32 kOpSlotOfName = 0x4fce50;
constexpr reg32 kSystemTasks = 0x4e7630;
constexpr reg32 kSleep = 0x4df8e0, kStrcmp = 0x4ee310, kStrncpy = 0x4e0e30, kMemset = 0x4e0640;
constexpr reg32 kBsearch = 0x4fb300, kQsort = 0x4ee4f8, kError = 0x401010;
constexpr reg32 kSprintf = 0x4df690;
constexpr reg32 kCloseDriver = 0x524650, kFillDword = 0x4e0721;

// The window record the game has one of, and its globals.
constexpr reg32 kWindow = 0x9f38d0;
constexpr reg32 kHwnd = 0x9f3d24, kActive = 0x9f3d4c, kFullscreen = 0x9f3d2d, kInstance = 0x9f3d48;
constexpr reg32 kMessageLock = 0x561290, kHandlerCount = 0x561268, kHandlers = 0x7f7060, kCompareHandlers = 0x4cc650;
constexpr reg32 kKeyLock = 0x7f7478, kKeyHead = 0x7f7474, kKeyTail = 0x7f747c, kKeyRoom = 0x7f7480;
constexpr reg32 kKeyQueue = 0x7f6fe0, kKeysDown = 0x564818, kScanTable = 0x561298, kWantRaw = 0x7f7488;
constexpr reg32 kLastScan = 0x7f748c, kHookHandle = 0x56127c, kHookWanted = 0x561280, kHookCallback = 0x561288;
constexpr reg32 kHookAlt = 0x561284, kFocusCallback = 0x7f7484, kOnResize = 0x567184, kWindowReady = 0x56126c;
constexpr reg32 kQuitAsked = 0x561270, kStickyKeys = 0x561274, kFilterKeys = 0x561278, kKeyboardHook = 0x4ccc70;
constexpr reg32 kWindowHandle = 0x454, kWindowFullscreen = 0x45d, kWindowX = 0x464, kWindowY = 0x468;
constexpr reg32 kGraphicsLock = 0x564480;
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kFillColour = 0x9f4fac;  // what the row fills write
constexpr reg32 kPalette = 0xa0a814;     // the screen's palette, as bytes blue green red

/* The stream's record, then its file records, rules and channels, then the ring. */
namespace stream
{
constexpr reg32 kMagic = 0x4d525453;   // 'STRM'
constexpr reg32 kLock = 0x04;
constexpr reg32 kFiles = 0x08, kFileCount = 0x0c, kRules = 0x10, kRuleCount = 0x14;
constexpr reg32 kChannels = 0x18, kChannelCount = 0x1c, kRing = 0x20, kRingEnd = 0x24;
constexpr reg32 kState = 0x28;         // 0 idle, 1 reading, 2 waiting for room
constexpr reg32 kPriorityIdle = 0x2c, kPriorityBusy = 0x30, kLowWater = 0x34, kBelow = 0x38, kQueued = 0x3c;
constexpr reg32 kOldest = 0x40, kParsed = 0x44, kWritten = 0x48;
constexpr reg32 kFirst = 0x4c, kCurrent = 0x50, kLast = 0x54, kFree = 0x58;
constexpr reg32 kPath = 0x5c, kFile = 0x160, kFilePos = 0x164, kOp = 0x168, kReadSize = 0x16c;
constexpr reg32 kHeader = 0x170;
constexpr reg32 kBufferSize = 0x2000;  // what one read asks for
constexpr reg32 kFreed = 0xfffffffe, kWrap = 0xffffffff;
constexpr reg32 kLengthMask = 0xffffff, kChannelMask = 0xff000000;
}

/* A file on the stream's queue: 0x128 bytes. */
namespace file
{
constexpr reg32 kSize = 0x128;
constexpr reg32 kId = 0x00, kState = 0x04, kPrevious = 0x08, kNext = 0x0c, kInMemory = 0x10, kName = 0x14;
constexpr reg32 kData = 0x118, kLength = 0x11c, kEndId = 0x120, kRingPos = 0x124;
constexpr reg32 kWaiting = 1, kReading = 2, kAtEnd = 3, kClosing = 4;
}

/* A channel of chunks: 16 bytes. */
namespace channel
{
constexpr reg32 kSize = 0x10;
constexpr reg32 kStream = 0x00, kNumber = 0x04, kBytes = 0x08, kFirstChunk = 0x0c;
}

/* The file library's slot, one per priority class: 104 bytes in the table at 0x9f4f60. */
constexpr reg32 kFileSlots = 0x9f4f60, kFileSlotSize = 104;
constexpr reg32 kFileFreeOps = 0x9f4f74, kSlotQueue = 0x1c, kSlotEvent = 0x54, kSlotSerial = 0x60;

/* Another thread's turn with the game, where the generated code offers it. */
void safepoint(win32::WinApplication* app, x86::CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

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

/* A Windows function the code calls through its import thunk: callImport, but in the checks by
 * the thunk's address (native_windows.h, guest::thunked). */
template <reg32 Thunk, auto Function, typename... Words>
reg32 callThunk(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::thunked<Thunk, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A method of a DirectDraw object, by its offset in the object's table, the object first on the
 * stack: the port's method called as C++ where the table has it there, else, and in the checks,
 * through the table (native_windows.h, guest::method). */
template <reg32 Offset, auto Method, typename... Words>
reg32 callMethod(Guest& guest, x86::CPU& cpu, reg32 object, Words... words)
{
    const std::optional<reg32> eax = guest::method<Offset, Method>(guest.app(), cpu, object, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

void enterLock(Guest& guest, x86::CPU& cpu, reg32 lock)
{
    callImport<kEnterLock, &win32::kernel32::EnterCriticalSection>(guest, cpu, lock);
}

void leaveLock(Guest& guest, x86::CPU& cpu, reg32 lock)
{
    callImport<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(guest, cpu, lock);
}

/* sub_4cf1c0 on a stream's handle, its two out slots at the bottom of the frame:
 * frame() the stream, frame() + 4 the handle.  0 when it is a stream. */
reg32 checkStream(Guest& guest, x86::CPU& cpu, reg32 handle)
{
    guest.leaveEdx(guest.frame());
    guest.leaveEbx(guest.frame() + 4);
    return guest::direct<0x4cf1c0, frontend::wina::streamCheck>(guest.app(), cpu, handle, guest.frame(),
                                                                guest.frame() + 4);
}

/* `length` bytes forward, as rep movsd and then rep movsb move them. */
void moveForward(Guest& guest, reg32 to, reg32 from, reg32 length)
{
    const reg32 dwords = length >> 2;
    for (reg32 i = 0; i < dwords; ++i)
    {
        guest.setDword(to, guest.dword(from));
        to += 4;
        from += 4;
    }
    for (reg32 i = 0; i < (length & 3); ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A chunk's length without the channel in its top byte. */
constexpr reg32 lengthOf(reg32 word)
{
    return word & stream::kLengthMask;
}

constexpr reg32 fileSlot(reg32 slot)
{
    return slot * kFileSlotSize;
}

/* A failed assertion of EA's: where it was, in the error record, and the message. */
void assertFailed(Guest& guest, reg32 sourceFile, reg32 function, reg32 line, const fe::Arguments& arguments)
{
    guest.setDword(kErrorFile, sourceFile);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    guest.call(kError, arguments);
}

/* The lock of the message table, made the first time it is asked for. */
void messageLockMade(Guest& guest, x86::CPU& cpu)
{
    const reg32 lock = guest.dword(kMessageLock);
    guest.leaveEdx(lock);
    if (lock == 0)
        guest.setDword(kMessageLock, reg32(*fe::direct<0x4eb2e0, ea::newLock>(guest.app(), cpu)));
}

/* The system's sticky and filter keys read, and turned off for what they were on. */
void accessKeysOff(Guest& guest, x86::CPU& cpu)
{
    callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x10, 0, kStickyKeys, 0);
    callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x54, 0, kFilterKeys, 0);
    if (guest.dword(kStickyKeys) != 0)
        callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x11, 0, 0, 2);
    if (guest.dword(kFilterKeys) != 0)
        callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x56, 0, 0, 2);
}

/* The mouse messages' buttons, as the game keeps them, and the place in the message: its window, its
 * keys (wParam) and its place (lParam). */
void pointerMessage(Guest& guest, x86::CPU& cpu, reg32 window, reg32 keys, reg32 place, reg32 down)
{
    guest.leaveEdx(keys);
    guest.leaveEcx(place);
    if (window != 0)
    {
        reg32 buttons = 0;
        if (keys & 1)
            buttons = 1;
        if (keys & 2)
            buttons |= 2;
        if (keys & 0x10)
            buttons |= 4;
        cpu.esi = place & 0xffff;
        guest::direct<0x4fb470, frontend::wina::pointerMoved>(guest.app(), cpu, place & 0xffff, (place >> 16) & 0xffff,
                                                              buttons, down);
    }
}

}

/* ---------------------------------------------------------------------------
 * The buffered stream
 * ------------------------------------------------------------------------- */

/* sub_4cf1c0: a handle to its stream, both handed out; nonzero when it is none. */
x86::reg32 frontend::wina::streamCheck(App* app, CPU& cpu, reg32 handle, reg32 streamOut, reg32 handleOut)
{
    Guest guest(app, cpu);
    if (handle == 0)
        return 1;
    const reg32 s = guest.dword(handle);
    if (guest.dword(s) != stream::kMagic)
        return 1;
    guest.setDword(handleOut, handle);
    guest.setDword(streamOut, s);
    return 0;
}

/* sub_4cf1f0: whether a place of the ring is between two others, the way round. */
x86::reg32 frontend::wina::ringBetween(App*, CPU&, reg32 from, reg32 to, reg32 place)
{
    if (from <= to)
        return place >= from && place < to ? 1 : 0;
    return place >= from || place < to ? 1 : 0;
}

/* sub_4cf220: bytes taken off what is queued; below the low water, more is asked for
 * sooner: the read in hand gets the busy priority. */
x86::reg32 frontend::wina::streamConsume(App* app, CPU& cpu, reg32 s, reg32 amount)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    guest.leaveEdx(guest.dword(s + stream::kLock));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 before = guest.dword(s + stream::kQueued);
    const reg32 after = before - amount;
    const reg32 lock = guest.dword(s + stream::kLock);
    guest.setDword(s + stream::kQueued, after);
    leaveLock(guest, cpu, lock);
    const reg32 low = guest.dword(s + stream::kLowWater);
    if (sreg32(before) >= sreg32(low) && sreg32(after) < sreg32(low))
    {
        const reg32 state = guest.dword(s + stream::kState);
        guest.setDword(s + stream::kBelow, 1);
        if (state == 1)
        {
            return guest::direct<0x4fc9b0, fileOpPriority>(app, cpu, guest.dword(s + stream::kOp),
                                                           guest.dword(s + stream::kPriorityBusy));
        }
    }
    return low;
}

/* sub_4cf280: a file record off the free list, its id's serial moved on; 0 when none. */
x86::reg32 frontend::wina::streamTakeFree(App* app, CPU& cpu, reg32 s)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    constexpr reg32 kSerial = 0x7f74b0, kSerialStep = 0x100;
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 taken = guest.dword(s + stream::kFree);
    if (taken != 0)
    {
        const reg32 serial = guest.dword(kSerial) + kSerialStep;
        guest.setDword(s + stream::kFree, guest.dword(taken + file::kNext));
        guest.setDword(kSerial, serial);
        if (serial == 0)
            guest.setDword(kSerial, kSerialStep);
        guest.setDword(taken + file::kId, (guest.dword(taken + file::kId) & 0xff) | guest.dword(kSerial));
    }
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    return taken;
}

/* sub_4cf2f0: a file record on the end of the queue. */
x86::reg32 frontend::wina::streamQueueFile(App* app, CPU& cpu, reg32 s, reg32 record)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    guest.setDword(record + file::kState, file::kWaiting);
    guest.setDword(record + file::kNext, 0);
    guest.leaveEdx(guest.dword(s + stream::kLock));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 last = guest.dword(s + stream::kLast);
    guest.setDword(record + file::kPrevious, last);
    if (last == 0)
    {
        guest.setDword(s + stream::kFirst, record);
        guest.setDword(s + stream::kCurrent, record);
    }
    else
    {
        guest.setDword(last + file::kNext, record);
    }
    guest.setDword(s + stream::kLast, record);
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    return guest.eax();
}

/* sub_4cf340: the file record of an id, 0 when it has gone. */
x86::reg32 frontend::wina::streamFindFile(App* app, CPU& cpu, reg32 s, reg32 id)
{
    Guest guest(app, cpu);
    const reg32 index = id & 0xff;
    if (sreg32(index) < sreg32(guest.dword(s + stream::kFileCount)))
    {
        const reg32 record = index * file::kSize + guest.dword(s + stream::kFiles);
        if (id == guest.dword(record + file::kId) && guest.dword(record + file::kState) != 0)
            return record;
    }
    return 0;
}

/* sub_4cf380: a file record off the queue, onto the free list. */
x86::reg32 frontend::wina::streamUnlinkFile(App* app, CPU& cpu, reg32 s, reg32 record)
{
    Guest guest(app, cpu);
    const reg32 first = guest.dword(s + stream::kFirst);
    if (record == first)
        guest.setDword(s + stream::kFirst, guest.dword(first + file::kNext));
    else
        guest.setDword(guest.dword(record + file::kPrevious) + file::kNext, guest.dword(record + file::kNext));
    const reg32 last = guest.dword(s + stream::kLast);
    if (record == last)
        guest.setDword(s + stream::kLast, guest.dword(last + file::kPrevious));
    else
        guest.setDword(guest.dword(record + file::kNext) + file::kPrevious, guest.dword(record + file::kPrevious));
    const reg32 current = guest.dword(s + stream::kCurrent);
    if (record == current)
    {
        const reg32 next = guest.dword(current + file::kNext);
        guest.setDword(s + stream::kCurrent, next != 0 ? next : guest.dword(current + file::kPrevious));
    }
    guest.setDword(record + file::kState, 0);
    guest.setDword(record + file::kNext, guest.dword(s + stream::kFree));
    guest.setDword(s + stream::kFree, record);
    return s;
}

/* sub_4cf3f0: the channel a chunk's id goes to, by the rules (mask, value, channel); -2 none. */
x86::reg32 frontend::wina::streamClassify(App* app, CPU& cpu, reg32 s, reg32 chunk)
{
    Guest guest(app, cpu);
    constexpr reg32 kRuleSize = 12;
    const reg32 id = guest.dword(chunk);
    reg32 i = 0;
    for (;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(guest.dword(s + stream::kRuleCount)))
            break;
        const reg32 rule = guest.dword(s + stream::kRules) + i * kRuleSize;
        if ((guest.dword(rule) & id) == guest.dword(rule + 4))
        {
            guest.leaveEdx(i);
            return guest.dword(rule + 8);
        }
    }
    guest.leaveEdx(i);
    return 0xfffffffe;
}

/* sub_4cf440: the whole chunks read so far given to their channels; 1 when the current
 * file's last chunk has come. */
x86::reg32 frontend::wina::streamParse(App* app, CPU& cpu, reg32 s)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 36);
    const reg32 current = guest.dword(s + stream::kCurrent);
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 chunk = guest.dword(s + stream::kParsed);
        if (sreg32(guest.dword(s + stream::kWritten) - chunk) < 8)
            return 0;
        const reg32 length = guest.dword(chunk + 4);
        if ((length & stream::kChannelMask) != 0)
            return 0;
        if (chunk + length > guest.dword(s + stream::kWritten))
            return 0;
        const reg32 number = guest::direct<0x4cf3f0, streamClassify>(app, cpu, s, chunk);
        bool closing;
        if (sreg32(number) < 0)
        {
            enterLock(guest, cpu, guest.dword(s + stream::kLock));
            closing = guest.dword(current + file::kState) == file::kClosing;
            if (!closing)
            {
                guest.setDword(chunk, stream::kFreed);
                guest.setDword(s + stream::kParsed, guest.dword(s + stream::kParsed) + length);
            }
        }
        else
        {
            guest.setDword(chunk + 4, guest.dword(chunk + 4) | (number << 24));
            enterLock(guest, cpu, guest.dword(s + stream::kLock));
            closing = guest.dword(current + file::kState) == file::kClosing;
            if (!closing)
            {
                const reg32 record = (number << 4) - channel::kSize + guest.dword(s + stream::kChannels);
                const reg32 bytes = guest.dword(record + channel::kBytes) + length;
                guest.setDword(record + channel::kBytes, bytes);
                if (length == bytes)
                    guest.setDword(record + channel::kFirstChunk, chunk);
                guest.setDword(s + stream::kParsed, guest.dword(s + stream::kParsed) + length);
                const reg32 before = guest.dword(s + stream::kQueued);
                const reg32 after = length + before;
                const reg32 low = guest.dword(s + stream::kLowWater);
                guest.setDword(s + stream::kQueued, after);
                if (sreg32(before) < sreg32(low) && sreg32(after) >= sreg32(low))
                    guest.setDword(s + stream::kBelow, 0);
            }
        }
        leaveLock(guest, cpu, guest.dword(s + stream::kLock));
        if (closing)
            return 0;
        if (guest.dword(chunk) == guest.dword(current + file::kEndId))
            return 1;
    }
}

/* sub_4cf570: the file's open has ended: its handle kept, reading begins. */
x86::reg32 frontend::wina::streamFileOpened(App* app, CPU& cpu, reg32 s)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    const reg32 handle = fe::direct<0x4fc600, menz::fileOpComplete>(app, cpu, guest.dword(s + stream::kOp));
    guest.setDword(s + stream::kFile, handle);
    if (handle == 0)
        return handle;
    return guest::direct<0x4cf810, streamReadMore>(app, cpu, s, guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cf5a0: the last file's close has ended: the next one opened. */
x86::reg32 frontend::wina::streamFileSwitched(App* app, CPU& cpu, reg32 s)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    guest.call(kOpWait, args({ guest.dword(s + stream::kOp), 1 }));
    const reg32 op = guest::direct<0x4fca60, fileOpOpen>(app, cpu, s + stream::kPath, guest.edx(),
                                                         guest.dword(s + stream::kPriorityBusy), s);
    guest.setDword(s + stream::kOp, op);
    if (op == 0)
        return op;
    return guest::direct<0x4fc8e0, fileOpCallback>(app, cpu, op, kFileOpened);
}

/* sub_4cf5e0: a read has ended, or a memory block was copied: the chunks parsed, and on
 * to more of this file or the next. */
x86::reg32 frontend::wina::streamFilled(App* app, CPU& cpu, reg32 s)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    const reg32 current = guest.dword(s + stream::kCurrent);
    reg32 got, ended;
    if (guest.dword(current + file::kInMemory) == 1)
    {
        got = guest.dword(s + stream::kReadSize);
        ended = sreg32(guest.dword(s + stream::kFilePos) + got) >= sreg32(guest.dword(current + file::kLength)) ? 1 : 0;
    }
    else
    {
        got = guest.call(kOpWait, args({ guest.dword(s + stream::kOp), s }));
        ended = sreg32(got) < sreg32(guest.dword(s + stream::kReadSize)) ? 1 : cpu.edx ^ cpu.ebx;
    }
    const reg32 position = guest.dword(s + stream::kFilePos) + got;
    const reg32 written = guest.dword(s + stream::kWritten) + got;
    guest.setDword(s + stream::kFilePos, position);
    guest.setDword(s + stream::kWritten, written);
    // The game's code hands it the end in edx, which it keeps and the test below reads.
    guest.leaveEdx(ended);
    const reg32 finished = guest::direct<0x4cf440, streamParse>(app, cpu, s);
    if (guest.dword(current + file::kState) == file::kClosing)
        return guest::direct<0x4cf6b0, streamNextFile>(app, cpu, s, guest.dword(s + stream::kPriorityBusy));
    if (ended == 0 && finished == 0)
        return guest::direct<0x4cf810, streamReadMore>(app, cpu, s, guest.dword(s + stream::kPriorityBusy));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    if (guest.dword(current + file::kState) != file::kClosing)
        guest.setDword(current + file::kState, file::kAtEnd);
    const reg32 lock = guest.dword(s + stream::kLock);
    guest.leaveEdx(lock);
    leaveLock(guest, cpu, lock);
    return guest::direct<0x4cf6b0, streamNextFile>(app, cpu, s, guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cf6b0: on to the next file of the queue: from memory at once, from a file by an
 * open, after a close when another one is open. */
x86::reg32 frontend::wina::streamNextFile(App* app, CPU& cpu, reg32 s, reg32 priority)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 32);
    guest.leaveEdx(guest.dword(s + stream::kLock));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 current = guest.dword(s + stream::kCurrent);
    bool none;
    if (current == 0)
    {
        none = true;
    }
    else if (guest.dword(current + file::kState) == file::kWaiting)
    {
        none = false;
    }
    else
    {
        const reg32 next = guest.dword(current + file::kNext);
        none = next == 0;
        if (!none)
            guest.setDword(s + stream::kCurrent, next);
    }
    reg32 record = 0;
    if (none)
    {
        guest.setDword(s + stream::kState, 0);
    }
    else
    {
        record = guest.dword(s + stream::kCurrent);
        const reg32 parsed = guest.dword(s + stream::kParsed);
        guest.setDword(record + file::kState, file::kReading);
        guest.setDword(record + file::kRingPos, parsed);
    }
    const reg32 lock = guest.dword(s + stream::kLock);
    cpu.eax = s;
    guest.leaveEdx(lock);
    callImport<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(guest, cpu, lock);
    if (none)
        return guest.eax();
    guest.setDword(s + stream::kWritten, guest.dword(s + stream::kParsed));
    if (guest.dword(record + file::kInMemory) == 1)
    {
        guest.setDword(s + stream::kFilePos, 0);
    }
    else
    {
        guest.setDword(s + stream::kFilePos, guest.dword(record + file::kLength));
        const reg32 path = s + stream::kPath;
        const reg32 name = record + file::kName;
        if (guest.call(kStrcmp, args({ name, path })) != 0)
        {
            guest.copyString(path, name);
            const reg32 open = guest.dword(s + stream::kFile);
            reg32 op, callback;
            if (open == 0)
            {
                op = guest::direct<0x4fca60, fileOpOpen>(app, cpu, path, 1, priority, s);
                callback = kFileOpened;
            }
            else
            {
                op = guest::direct<0x4fca90, fileOpClose>(app, cpu, open, priority, s);
                callback = kFileSwitched;
            }
            guest.leaveEdx(s);
            guest.setDword(s + stream::kOp, op);
            if (op == 0)
                return op;
            return guest::direct<0x4fc8e0, fileOpCallback>(app, cpu, op, callback);
        }
    }
    return guest::direct<0x4cf810, streamReadMore>(app, cpu, s, priority);
}

/* sub_4cf810: the next read into the ring: records of files read through let go, the
 * ring wrapped when the end is too near, and 0x2000 bytes asked for or copied. */
x86::reg32 frontend::wina::streamReadMore(App* app, CPU& cpu, reg32 s, reg32 priority)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
    // The oldest chunk past what has been freed.
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 oldest = guest.dword(s + stream::kOldest);
        if (guest.dword(s + stream::kParsed) == oldest)
            break;
        const reg32 id = guest.dword(oldest);
        if (id == stream::kWrap)
            guest.setDword(s + stream::kOldest, guest.dword(s + stream::kRing));
        else if (id == stream::kFreed)
            guest.setDword(s + stream::kOldest, oldest + guest.dword(oldest + 4));
        else
            break;
    }
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    // The files whose chunks have all been read.
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 first = guest.dword(s + stream::kFirst);
        const reg32 next = guest.dword(first + file::kNext);
        if (next == 0 || guest.dword(next + file::kState) == file::kWaiting)
            break;
        if (guest::direct<0x4cf1f0, ringBetween>(app, cpu, guest.dword(s + stream::kOldest), guest.dword(s + stream::kWritten),
                                                 guest.dword(next + file::kRingPos) - 1) != 0)
            break;
        guest::direct<0x4cf380, streamUnlinkFile>(app, cpu, s, first);
    }
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    guest.leaveEdx(s);
    const reg32 oldest = guest.dword(s + stream::kOldest);
    const reg32 written = guest.dword(s + stream::kWritten);
    reg32 room;
    if (oldest > written)
    {
        room = oldest - written - 1;
    }
    else
    {
        room = guest.dword(s + stream::kRingEnd) - written - 8;
        if (sreg32(room) < sreg32(stream::kBufferSize))
        {
            // Too near the end: what is unparsed goes to the start, if it fits.
            const reg32 parsed = guest.dword(s + stream::kParsed);
            const reg32 partial = written - parsed;
            guest.leaveEdx(partial);
            const reg32 ring = guest.dword(s + stream::kRing);
            if (sreg32(oldest - ring) < sreg32(partial + 1))
            {
                guest.setDword(s + stream::kState, 2);
                return s;
            }
            moveForward(guest, ring, parsed, partial);
            guest.setDword(parsed, stream::kWrap);
            guest.setDword(parsed + 4, 8);
            guest.setDword(s + stream::kParsed, guest.dword(s + stream::kRing));
            guest.setDword(s + stream::kWritten, guest.dword(s + stream::kRing) + partial);
            room = guest.dword(s + stream::kOldest) - guest.dword(s + stream::kWritten) - 1;
        }
    }
    if (sreg32(room) < sreg32(stream::kBufferSize))
    {
        guest.setDword(s + stream::kState, 2);
        return s;
    }
    const reg32 current = guest.dword(s + stream::kCurrent);
    guest.leaveEdx(current);
    if (guest.dword(current + file::kInMemory) == 1)
    {
        const reg32 length = guest.dword(current + file::kLength);
        if (sreg32(guest.dword(s + stream::kFilePos) + room) > sreg32(length))
            room = length - guest.dword(s + stream::kFilePos);
        guest.setDword(s + stream::kReadSize, room);
        const reg32 size = guest.dword(s + stream::kReadSize);
        moveForward(guest, guest.dword(s + stream::kWritten), guest.dword(current + file::kData), size);
        guest.setDword(current + file::kData, guest.dword(current + file::kData) + guest.dword(s + stream::kReadSize));
        // The game's code hands it 0 in eax and edx too.
        cpu.eax = 0;
        guest.leaveEdx(0);
        return guest::direct<0x4cf5e0, streamFilled>(app, cpu, s);
    }
    guest.setDword(s + stream::kReadSize, stream::kBufferSize);
    const reg32 op = guest::direct<0x4fcab0, fileOpRead>(app, cpu, guest.dword(s + stream::kFile),
                                                         guest.dword(s + stream::kFilePos), guest.dword(s + stream::kWritten),
                                                         guest.dword(s + stream::kReadSize), priority, s);
    guest.leaveEdx(s);
    guest.setDword(s + stream::kOp, op);
    if (op == 0)
        return op;
    return guest::direct<0x4fc8e0, fileOpCallback>(app, cpu, op, kFilled);
}

/* sub_4cfa40: the bytes a stream's record and tables take. */
x86::reg32 frontend::wina::streamHeaderSize(App* app, CPU& cpu, reg32 fileCount, reg32 ruleCount, reg32 channelCount)
{
    Guest guest(app, cpu);
    constexpr reg32 kRuleSize = 12;
    const reg32 files = fileCount * file::kSize + stream::kHeader;
    const reg32 channels = channelCount << 4;
    const reg32 rules = ruleCount * kRuleSize;
    guest.leaveEdx(files);
    guest.leaveEbx(channels);
    return rules + files + channels;
}

/* sub_4cfa80: a stream made in the memory given; its first channel, or 0 when it does
 * not fit or asks too much. */
x86::reg32 frontend::wina::streamCreate(App* app, CPU& cpu, reg32 files, reg32 rules, reg32 channels, reg32 s,
                                        reg32 memory)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kRuleSize = 12, kLeastRing = 0x6000, kMostFiles = 0x100, kMostRules = 0x10;
    const reg32 room = memory - guest::direct<0x4cfa40, streamHeaderSize>(app, cpu, files, rules, channels);
    guest.leaveEdx(room);
    if (sreg32(room) < sreg32(kLeastRing))
        return 0;
    if (sreg32(files) < 2 || sreg32(files) > sreg32(kMostFiles))
        return 0;
    if (sreg32(rules) < 1 || sreg32(rules) > sreg32(kMostRules))
        return 0;
    guest.leaveEdx(channels);
    if (sreg32(channels) < 1 || sreg32(rules) < sreg32(channels))
        return 0;

    guest.setDword(s, stream::kMagic);
    const reg32 lock = reg32(*fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    guest.setDword(s + stream::kState, 0);
    guest.setDword(s + stream::kLock, lock);
    guest.setDword(s + stream::kFiles, s + stream::kHeader);
    guest.setDword(s + stream::kPriorityIdle, 0x96);
    guest.setDword(s + stream::kPriorityBusy, 0x32);
    guest.setDword(s + stream::kLowWater, 0);
    guest.setDword(s + stream::kBelow, 0);
    guest.setDword(s + stream::kQueued, 0);
    guest.setDword(s + stream::kRules, guest.dword(s + stream::kFiles) + files * file::kSize);
    guest.setDword(s + stream::kFirst, 0);
    guest.setDword(s + stream::kCurrent, 0);
    guest.setDword(s + stream::kLast, 0);
    guest.setDword(s + stream::kChannels, guest.dword(s + stream::kRules) + rules * kRuleSize);
    guest.setDword(s + stream::kChannelCount, channels);
    guest.setDword(s + stream::kFileCount, files);
    guest.setDword(s + stream::kRing, guest.dword(s + stream::kChannels) + (channels << 4));
    guest.setDword(s + stream::kRingEnd, memory + s);
    const reg32 ring = guest.dword(s + stream::kRing);
    guest.setDword(s + stream::kOldest, ring);
    guest.setDword(s + stream::kParsed, ring);
    guest.setDword(s + stream::kWritten, ring);
    guest.setDword(s + stream::kFree, guest.dword(s + stream::kFiles));
    guest.setDword(s + stream::kRuleCount, rules);
    guest.call(kMemset, args({ s + stream::kPath, 0, stream::kFile - stream::kPath }));
    guest.setDword(s + stream::kFile, 0);

    // The file records, on the free list.
    reg32 record = 0;
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(files))
            break;
        record = guest.dword(s + stream::kFiles) + i * file::kSize;
        guest.setDword(record + file::kId, i);
        guest.setDword(record + file::kState, 0);
        guest.setDword(record + file::kNext, guest.dword(s + stream::kFiles) + (i + 1) * file::kSize);
    }
    guest.setDword(guest.dword(s + stream::kFiles) + (files - 1) * file::kSize + file::kNext, 0);
    guest.leaveEbx(record);
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(rules))
            break;
        const reg32 rule = guest.dword(s + stream::kRules) + i * kRuleSize;
        guest.setDword(rule, 0);
        guest.setDword(rule + 4, 0);
        guest.setDword(rule + 8, 1);
    }
    reg32 last = 0;
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(channels))
            break;
        last = guest.dword(s + stream::kChannels) + (i << 4);
        guest.setDword(last + channel::kStream, s);
        guest.setDword(last + channel::kBytes, 0);
        guest.setDword(last + channel::kNumber, i + 1);
    }
    guest.leaveEdx(last);
    guest.leaveEcx(channels);
    return guest.dword(s + stream::kChannels);
}

/* sub_4cfca0: a rule of a stream that is not reading: mask, value, channel. */
x86::reg32 frontend::wina::streamSetRule(App* app, CPU& cpu, reg32 handle, reg32 number, reg32 mask, reg32 value,
                                         reg32 target)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    constexpr reg32 kRuleSize = 12;
    guest.leaveEcx(target);
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0 || sreg32(number) < 1)
        return bad;
    const reg32 s = guest.dword(guest.frame());
    const reg32 rules = guest.dword(s + stream::kRuleCount);
    guest.leaveEdx(rules);
    if (sreg32(number) > sreg32(rules))
        return s;
    reg32 left = s;
    if (number == rules)
    {
        left = mask | value;
        if (left != 0)
            return left;
    }
    if (sreg32(target) < 1 && target != 0xffffffff && target != 0xfffffffe)
        return left;
    if (sreg32(target) > sreg32(guest.dword(s + stream::kChannelCount)))
        return s;
    if (guest.dword(s + stream::kState) != 0)
        return s;
    const reg32 rule = guest.dword(s + stream::kRules) + (number - 1) * kRuleSize;
    guest.setDword(rule + 4, value);
    guest.leaveEdx(mask);
    guest.setDword(rule + 8, target);
    guest.setDword(rule, mask);
    return rule;
}

/* sub_4cfd30: a stream closed: flushed, its read waited for, its lock and file gone. */
x86::reg32 frontend::wina::streamClose(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    guest::direct<0x4d0220, streamFlush>(app, cpu, handle);
    for (;;)
    {
        safepoint(app, cpu);
        if (guest.dword(guest.dword(guest.frame()) + stream::kState) != 1)
            break;
        if (*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0))
            fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    }
    guest.setDword(guest.dword(guest.frame()), 0);
    const reg32 s = guest.dword(guest.frame());
    fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(s + stream::kLock));
    return guest::direct<0x4fd060, fileCloseWait>(app, cpu, guest.dword(guest.dword(guest.frame()) + stream::kFile), 0x64);
}

/* sub_4cfdd0: the low water mark: crossing it tells the read in hand. */
x86::reg32 frontend::wina::streamSetLowWater(App* app, CPU& cpu, reg32 handle, reg32 low)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    const reg32 s = guest.dword(guest.frame());
    const reg32 before = guest.dword(s + stream::kLowWater);
    guest.setDword(s + stream::kLowWater, low);
    const reg32 queued = guest.dword(s + stream::kQueued);
    const reg32 wasBelow = sreg32(queued) < sreg32(before) ? 1 : 0;
    const reg32 isBelow = sreg32(queued) < sreg32(low) ? 1 : 0;
    guest.leaveEdx(isBelow);
    if (wasBelow == isBelow)
        return queued;
    return guest::direct<0x4cfe30, streamSetBelow>(app, cpu, handle, isBelow);
}

/* sub_4cfe30: whether the stream is below its low water; when it is, the read gets the
 * busy priority. */
x86::reg32 frontend::wina::streamSetBelow(App* app, CPU& cpu, reg32 handle, reg32 below)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    const reg32 s = guest.dword(guest.frame());
    guest.setDword(s + stream::kBelow, below);
    if (below == 0 || guest.dword(s + stream::kState) != 1)
        return s;
    return guest::direct<0x4fc9b0, fileOpPriority>(app, cpu, guest.dword(s + stream::kOp),
                                                   guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cfe70: a channel's handle, by its number from 1; 0 when there is none. */
x86::reg32 frontend::wina::streamChannel(App* app, CPU& cpu, reg32 handle, reg32 number)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    const reg32 s = guest.dword(guest.frame());
    if (sreg32(number) < 1 || sreg32(number) > sreg32(guest.dword(s + stream::kChannelCount)))
        return 0;
    const reg32 channels = guest.dword(s + stream::kChannels);
    guest.leaveEdx(channels);
    return (number << 4) - channel::kSize + channels;
}

/* sub_4cfef0: a file put on the queue; reading starts if it had stopped.  Its id, 0 when
 * there is no room. */
x86::reg32 frontend::wina::streamOpenFile(App* app, CPU& cpu, reg32 handle, reg32 name, reg32 length, reg32 endId)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
    constexpr reg32 kMostName = 0x103;
    const reg32 frame = guest.frame();
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    const reg32 s = guest.dword(frame);
    const reg32 record = guest::direct<0x4cf280, streamTakeFree>(app, cpu, s);
    guest.leaveEcx(record);
    if (record == 0)
        return 0;
    guest.setDword(record + file::kInMemory, 0);
    guest.call(kStrncpy, args({ record + file::kName, name, kMostName }));
    guest.setDword(record + file::kEndId, endId);
    guest.setDword(record + file::kLength, length);
    guest::direct<0x4cf2f0, streamQueueFile>(app, cpu, s, record);
    const reg32 lock = guest.dword(s + stream::kLock);
    guest.leaveEdx(lock);
    enterLock(guest, cpu, lock);
    const reg32 state = guest.dword(s + stream::kState);
    guest.leaveEbx(state);
    if (state == 0)
        guest.setDword(s + stream::kState, 1);
    const reg32 lockAgain = guest.dword(s + stream::kLock);
    guest.leaveEcx(lockAgain);
    leaveLock(guest, cpu, lockAgain);
    if (state == 0)
    {
        const reg32 priority = guest.dword(s + stream::kBelow) != 0 ? guest.dword(s + stream::kPriorityBusy)
                                                                    : guest.dword(s + stream::kPriorityIdle);
        guest::direct<0x4cf6b0, streamNextFile>(app, cpu, s, priority);
    }
    return guest.dword(record + file::kId);
}

/* sub_4cffb0: a block of memory put on the queue; with no length given, it runs to the
 * chunk with the end id.  Its id, 0 when there is no room. */
x86::reg32 frontend::wina::streamOpenMemory(App* app, CPU& cpu, reg32 handle, reg32 data, reg32 length, reg32 endId)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    const reg32 frame = guest.frame();
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    const reg32 s = guest.dword(frame);
    const reg32 record = guest::direct<0x4cf280, streamTakeFree>(app, cpu, s);
    guest.leaveEbx(record);
    if (record == 0)
        return 0;
    if (length == 0)
    {
        reg32 chunk = data;
        for (;;)
        {
            safepoint(app, cpu);
            if (endId == guest.dword(chunk))
                break;
            const reg32 size = guest.dword(chunk + 4);
            length += size;
            chunk += size;
        }
        length += guest.dword(chunk + 4);
    }
    guest.setDword(record + file::kInMemory, 1);
    guest.setDword(record + file::kData, data);
    guest.setDword(record + file::kLength, length);
    guest.setDword(record + file::kEndId, endId);
    guest::direct<0x4cf2f0, streamQueueFile>(app, cpu, s, record);
    const reg32 lock = guest.dword(s + stream::kLock);
    guest.leaveEcx(lock);
    enterLock(guest, cpu, lock);
    const reg32 state = guest.dword(s + stream::kState);
    if (state == 0)
        guest.setDword(s + stream::kState, 1);
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    if (state == 0)
        guest::direct<0x4cf6b0, streamNextFile>(app, cpu, s, 0);
    return guest.dword(record + file::kId);
}

/* sub_4d0060: a file of the queue closed: what it has in the ring taken back from every
 * channel, by freeing its chunks or marking them. */
x86::reg32 frontend::wina::streamCloseFile(App* app, CPU& cpu, reg32 handle, reg32 id)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 48);
    const reg32 frame = guest.frame();
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    const reg32 s = guest.dword(frame);
    guest.leaveEdx(guest.dword(s + stream::kLock));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 record = guest::direct<0x4cf340, streamFindFile>(app, cpu, s, id);
    bool nothing;
    reg32 from = 0, to = 0, oldest = 0;
    if (record == 0 || guest.dword(record + file::kState) == file::kClosing)
    {
        nothing = true;
    }
    else if (guest.dword(record + file::kState) == file::kWaiting)
    {
        guest::direct<0x4cf380, streamUnlinkFile>(app, cpu, s, record);
        nothing = true;
    }
    else
    {
        guest.setDword(record + file::kState, file::kClosing);
        oldest = guest.dword(s + stream::kOldest);
        from = record == guest.dword(s + stream::kFirst) ? oldest : guest.dword(record + file::kRingPos);
        const reg32 next = guest.dword(record + file::kNext);
        to = next == 0 || guest.dword(next + file::kState) == file::kWaiting ? guest.dword(s + stream::kParsed)
                                                                             : guest.dword(next + file::kRingPos);
        nothing = false;
    }
    const reg32 lock = guest.dword(s + stream::kLock);
    guest.leaveEdx(lock);
    cpu.eax = s;
    callImport<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(guest, cpu, lock);
    if (nothing)
        return guest.eax();

    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        guest.leaveEdx(index);
        if (sreg32(index) >= sreg32(guest.dword(s + stream::kChannelCount)))
            return s;
        const reg32 reader = guest.dword(s + stream::kChannels) + (index << 4);
        guest.setDword(frame + 4, reader);
        if (sreg32(guest.dword(reader + channel::kBytes)) <= 0)
            continue;
        if (guest::direct<0x4cf1f0, ringBetween>(app, cpu, oldest, from, guest.dword(reader + channel::kFirstChunk)) != 0)
        {
            // The channel's reader is before these chunks: marked, for when it comes.
            const reg32 mine = guest.dword(reader + channel::kNumber) << 24;
            reg32 chunk = from;
            for (;;)
            {
                safepoint(app, cpu);
                if (chunk == to)
                    break;
                if (guest.dword(chunk) == stream::kWrap)
                {
                    chunk = guest.dword(s + stream::kRing);
                    continue;
                }
                const reg32 length = lengthOf(guest.dword(chunk + 4));
                if ((guest.dword(chunk + 4) & stream::kChannelMask) == mine)
                {
                    enterLock(guest, cpu, guest.dword(s + stream::kLock));
                    guest.setDword(reader + channel::kBytes, guest.dword(reader + channel::kBytes) - length);
                    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
                    guest::direct<0x4cf220, streamConsume>(app, cpu, s, length);
                    guest.setDword(chunk, stream::kFreed);
                    guest.setDword(chunk + 4, length);
                }
                chunk += length;
            }
        }
        else if (guest::direct<0x4cf1f0, ringBetween>(app, cpu, from, to, guest.dword(reader + channel::kFirstChunk)) != 0)
        {
            // Its reader is inside them: read on, past them.
            do
            {
                safepoint(app, cpu);
                const reg32 chunk = guest::direct<0x4d0300, streamNextChunk>(app, cpu, reader);
                guest::direct<0x4d03c0, streamFreeChunk>(app, cpu, reader, chunk);
                if (sreg32(guest.dword(reader + channel::kBytes)) <= 0)
                    break;
            } while (guest::direct<0x4cf1f0, ringBetween>(app, cpu, from, to, guest.dword(reader + channel::kFirstChunk)) != 0);
        }
    }
}

/* sub_4d0220: everything off a stream: its files closed, the channels emptied, the ring
 * marked free. */
x86::reg32 frontend::wina::streamFlush(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
    const reg32 frame = guest.frame();
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    reg32 last = guest.dword(guest.dword(frame) + stream::kLast);
    if (last == 0)
        return 0;
    for (;;)
    {
        const reg32 state = guest.dword(last + file::kState);
        if (state != file::kWaiting && state != file::kReading)
            break;
        guest::direct<0x4d0060, streamCloseFile>(app, cpu, handle, guest.dword(last + file::kId));
        safepoint(app, cpu);
        last = guest.dword(guest.dword(frame) + stream::kLast);
    }
    reg32 current;
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 s = guest.dword(frame);
        current = guest.dword(s + stream::kCurrent);
        const reg32 first = guest.dword(s + stream::kFirst);
        if (first == current)
            break;
        guest::direct<0x4cf380, streamUnlinkFile>(app, cpu, s, first);
    }
    guest.setDword(current + file::kState, file::kClosing);
    const reg32 s = guest.dword(frame);
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(s + stream::kChannelCount)); ++i)
    {
        safepoint(app, cpu);
        guest.setDword(guest.dword(s + stream::kChannels) + (i << 4) + channel::kBytes, 0);
    }
    guest::direct<0x4cf220, streamConsume>(app, cpu, s, guest.dword(s + stream::kQueued));
    reg32 chunk = guest.dword(guest.dword(frame) + stream::kOldest);
    for (;;)
    {
        safepoint(app, cpu);
        if (chunk == guest.dword(guest.dword(frame) + stream::kParsed))
            break;
        if (guest.dword(chunk) == stream::kWrap)
        {
            chunk = guest.dword(guest.dword(frame) + stream::kRing);
            continue;
        }
        const reg32 length = lengthOf(guest.dword(chunk + 4));
        guest.setDword(chunk, stream::kFreed);
        guest.setDword(chunk + 4, length);
        chunk += length;
    }
    if (guest.dword(guest.dword(frame) + stream::kState) == 2)
        guest.setDword(guest.dword(frame) + stream::kState, 0);
    return chunk;
}

/* sub_4d0300: a channel's next chunk, its reader moved on past it; 0 when it has none. */
x86::reg32 frontend::wina::streamNextChunk(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 32);
    const reg32 frame = guest.frame();
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    const reg32 reader = guest.dword(frame + 4);
    if (guest.dword(reader + channel::kBytes) == 0)
        return 0;
    const reg32 chunk = guest.dword(reader + channel::kFirstChunk);
    const reg32 length = lengthOf(guest.dword(chunk + 4));
    guest.setDword(chunk + 4, length);
    const reg32 s = guest.dword(frame);
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 bytes = guest.dword(reader + channel::kBytes) - length;
    guest.setDword(reader + channel::kBytes, bytes);
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    if (sreg32(bytes) > 0)
    {
        const reg32 mine = guest.dword(reader + channel::kNumber) << 24;
        reg32 at = chunk + length;
        for (;;)
        {
            safepoint(app, cpu);
            if ((guest.dword(at + 4) & stream::kChannelMask) == mine)
                break;
            if (guest.dword(at) == stream::kWrap)
                at = guest.dword(s + stream::kRing);
            else
                at += lengthOf(guest.dword(at + 4));
        }
        guest.setDword(reader + channel::kFirstChunk, at);
    }
    return chunk;
}

/* sub_4d03c0: a chunk read and done with: if reading waited for room, on it goes. */
x86::reg32 frontend::wina::streamFreeChunk(App* app, CPU& cpu, reg32 handle, reg32 chunk)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
    const reg32 frame = guest.frame();
    const reg32 bad = checkStream(guest, cpu, handle);
    if (bad != 0)
        return bad;
    const reg32 s = guest.dword(frame);
    if (chunk < guest.dword(s + stream::kRing))
        return s;
    const reg32 last = guest.dword(s + stream::kRingEnd) - 8;
    if (chunk > last)
        return last;
    if (guest.dword(chunk) == stream::kFreed)
        return last;
    guest.setDword(chunk, stream::kFreed);
    guest::direct<0x4cf220, streamConsume>(app, cpu, s, guest.dword(chunk + 4));
    enterLock(guest, cpu, guest.dword(s + stream::kLock));
    const reg32 state = guest.dword(s + stream::kState);
    if (state == 2)
        guest.setDword(s + stream::kState, 1);
    leaveLock(guest, cpu, guest.dword(s + stream::kLock));
    if (state != 2)
        return s;
    const reg32 priority = guest.dword(s + stream::kBelow) != 0 ? guest.dword(s + stream::kPriorityBusy)
                                                                : guest.dword(s + stream::kPriorityIdle);
    return guest::direct<0x4cf810, streamReadMore>(app, cpu, s, priority);
}

/* sub_4d0460: the bytes a channel has waiting. */
x86::reg32 frontend::wina::channelBytes(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame() + 4) + channel::kBytes);
}

/* sub_4d0490: the state of a channel's stream. */
x86::reg32 frontend::wina::channelState(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame()) + stream::kState);
}

/* sub_4d04c0: whether a channel has come to its end: nothing waiting, nothing reading. */
x86::reg32 frontend::wina::channelDone(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame()) + stream::kState) == 0
                   && guest.dword(guest.dword(guest.frame() + 4) + channel::kBytes) == 0
               ? 1
               : 0;
}

/* sub_4d0500: how big a channel's stream's ring is. */
x86::reg32 frontend::wina::channelRingSize(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    if (checkStream(guest, cpu, handle) != 0)
        return 0;
    const reg32 s = guest.dword(guest.frame());
    return guest.dword(s + stream::kRingEnd) - guest.dword(s + stream::kRing);
}

/* sub_495b60: the movie's next frame chunk from its channel; what is not one is thrown
 * away.  0 at the channel's end.  It lets other threads run at every try. */
x86::reg32 frontend::wina::movieNextFrame(App* app, CPU& cpu, reg32 reader)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    constexpr reg32 kFrameKey = 0x6d44414d, kFrameInter = 0x6544414d, kFrameBlack = 0x6b44414d;
    constexpr reg32 kFramesTaken = 0x79f32c;
    for (;;)
    {
        app->unlockContext(cpu);
        win32::Thread::sleep(0);
        app->lockContext(cpu);
        safepoint(app, cpu);
        const reg32 chunk = guest::direct<0x4d0300, streamNextChunk>(app, cpu, reader);
        if (chunk != 0)
        {
            const reg32 id = guest.dword(chunk);
            if (id == kFrameKey || id == kFrameInter || id == kFrameBlack)
            {
                guest.setDword(kFramesTaken, guest.dword(kFramesTaken) + 1);
                return chunk;
            }
            guest::direct<0x4d03c0, streamFreeChunk>(app, cpu, reader, chunk);
        }
        if (guest::direct<0x4d04c0, channelDone>(app, cpu, reader) != 0)
            return 0;
    }
}

/* ---------------------------------------------------------------------------
 * EA's file library
 * ------------------------------------------------------------------------- */

/* sub_4fb990: a file operation off the free list, set up; its slot's serial in its id. */
x86::reg32 frontend::wina::fileOpAllocate(App* app, CPU& cpu, reg32 kind, reg32 priorityIn, reg32 target, reg32 slot)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    constexpr reg32 kSource = 0x54dcac, kFunction = 0x54dd10;
    const reg32 frame = guest.frame();
    guest.setDword(frame, priorityIn);
    guest.leaveEbx(slot);
    if (guest.dword(kFileSlots) == 0)
    {
        guest.leaveEcx(kSource);
        cpu.esi = kFunction;
        assertFailed(guest, kSource, kFunction, 0xc5, args({ 0xc5 }, { 0x54dd20 }));
    }
    const reg32 priority = guest.dword(frame);
    if (sreg32(priority) < 0 || sreg32(priority) > 0xff)
    {
        cpu.esi = kSource;
        guest.leaveEcx(priority);
        assertFailed(guest, kSource, kFunction, 0xc8, args({ kFunction, 0xc8 }, { 0x54dd64, priority }));
    }
    if (sreg32(slot) < 0 || slot > 0x1f)
    {
        cpu.esi = kSource;
        assertFailed(guest, kSource, kFunction, 0xcb, args({ kFunction, 0xcb }, { 0x54dda0, slot }));
    }
    const reg32 op = fe::direct<0x50df60, netb::qpop>(app, cpu, kFileFreeOps);
    guest.leaveEcx(op);
    const reg32 record = guest.dword(kFileSlots) + fileSlot(slot);
    guest.leaveEdx(record);
    if (op == 0)
    {
        guest.leaveEdx(kSource);
        guest.leaveEcx(kFunction);
        guest.leaveEbx(0xd3);
        assertFailed(guest, kSource, kFunction, 0xd3, args({}, { 0x54dddc }));
        return 0;
    }
    if (guest.dword(record) == 0)
        fe::direct<0x4fb860, menz::fileDeviceStart>(app, cpu, slot);
    guest.setByte(op + 0x10, 0);
    guest.setDword(op + 0x14, 0);
    guest.setDword(op + 0x18, 0);
    guest.setDword(op + 0x20, 0);
    guest.setDword(op + 0x24, 0);
    guest.setDword(op + 0x28, 0);
    guest.setDword(op + 0x2c, 0);
    guest.setDword(op + 0x1c, target);
    guest.setByte(op + 0x11, guest.byte(frame));
    const std::uint8_t flags = guest.byte(op + 0xc) & 0xf0;
    guest.setDword(op + 8, kind);
    guest.setByte(op + 0xc, flags);
    reg32 token = 0;
    if (guest.dword(record) != 0)
        token = fe::direct<0x50ea20, menz::fileLockEnter>(app, cpu, record + kSlotQueue);
    guest.setDword(op + 4, (guest.dword(record + kSlotSerial) << 5) | slot);
    const reg32 serial = (guest.dword(record + kSlotSerial) + 1) & 0xffffff;
    guest.setDword(record + kSlotSerial, serial);
    if (serial == 0)
        guest.setDword(record + kSlotSerial, 1);
    if (guest.dword(record) != 0)
        fe::direct<0x50ea40, menz::fileLockLeave>(app, cpu, record + kSlotQueue, token);
    return op;
}

/* sub_4fbb60: an operation on its slot's queue, the slot's thread told; its id. */
x86::reg32 frontend::wina::fileOpQueue(App* app, CPU& cpu, reg32 op)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    const reg32 record = guest.dword(kFileSlots) + fileSlot(guest.dword(op + 4) & 0x1f);
    if (record == 0)
    {
        const reg32 esi = cpu.esi, edi = cpu.edi;
        cpu.esp -= 8;
        cpu.esi = 0x54de10;
        cpu.edi = 0xfb;
        guest.leaveEdx(0x54dcac);
        assertFailed(guest, 0x54dcac, 0x54de10, 0xfb, args({}, { 0x54de20 }));
        cpu.esp += 8;
        cpu.esi = esi;
        cpu.edi = edi;
    }
    fe::direct<0x50e0a0, netb::qinsert>(app, cpu, record + kSlotQueue, op);
    fe::direct<0x4dfb30, ea::eventSet>(app, cpu, guest.dword(record + kSlotEvent));
    return guest.dword(op + 4);
}

/* sub_4fbbe0: an operation's file name, a copy of it in memory of its own. */
x86::reg32 frontend::wina::fileOpSetName(App* app, CPU& cpu, reg32 op, reg32 nameIn)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kNoName = 0x54de60;
    const reg32 name = nameIn != 0 ? nameIn : kNoName;
    const reg32 size = guest.length(name) + 1;
    guest.leaveEdx(size);
    guest.setDword(guest.frame(), size);
    guest.setByte(op + 0xc, guest.byte(op + 0xc) | 1);
    const reg32 copy = guest.call(0x501110, args({ guest.frame() }));
    guest.setDword(op + 0x2c, copy);
    guest.call(0x4ea4f0, args({ name, copy, guest.edx() }));
    return guest.eax();
}

/* sub_4fc8e0: what is to be called when an operation ends; at once when it has. */
x86::reg32 frontend::wina::fileOpCallback(App* app, CPU& cpu, reg32 id, reg32 callback)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    const reg32 frame = guest.frame();
    const reg32 record = guest.dword(kFileSlots) + fileSlot(id & 0x1f);
    const reg32 token = guest.dword(record) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, cpu, record + kSlotQueue) : 0;
    const reg32 op = fe::direct<0x4fbd10, menz::fileOpFind>(app, cpu, record, id, frame);
    reg32 left = op;  // what the calls leave in eax
    if (op == 0)
    {
        assertFailed(guest, 0x54dcac, 0x54df38, 0x4a6, args({}, { 0x54df48 }));
        left = guest.eax();
    }
    else
    {
        guest.setByte(op + 0xc, guest.byte(op + 0xc) | 4);
        const reg32 pending = guest.dword(frame);
        guest.leaveEdx(pending);
        if (pending != 0)
        {
            guest.setDword(op + 0x20, callback);
        }
        else
        {
            guest.setDword(op + 0x20, pending);
            guest.setByte(op + 0xc, guest.byte(op + 0xc) | 8);
            const reg32 priority = reg32(sreg32(guest.dword(op + 0xd)) >> 24);
            left = guest.call(callback, args({ guest.dword(op + 4), priority, guest.dword(op + 0x1c) }));
        }
    }
    if (guest.dword(record) != 0)
        return fe::direct<0x50ea40, menz::fileLockLeave>(app, cpu, record + kSlotQueue, token);
    return left;
}

/* sub_4fc9b0: an operation still queued given another priority. */
x86::reg32 frontend::wina::fileOpPriority(App* app, CPU& cpu, reg32 id, reg32 priority)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 slots = guest.dword(kFileSlots);
    guest.leaveEdx(slots);
    const reg32 record = fileSlot(id & 0x1f) + slots;
    if (record == 0)
        return 0;
    // The priority the game's code hands it in ecx is read neither by it nor by the test (sub_4fb820).
    const reg32 op = fe::direct<0x50e5a0, watb::queueFetch>(app, cpu, record + kSlotQueue, 0x4fb820, id);
    if (op == 0)
        return 0;
    guest.setByte(op + 0x11, std::uint8_t(priority));
    return guest::direct<0x4fbb60, fileOpQueue>(app, cpu, op);
}

/* sub_4fca60: an operation to open a file. */
x86::reg32 frontend::wina::fileOpOpen(App* app, CPU& cpu, reg32 name, reg32 mode, reg32 priority, reg32 target)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 slot = guest.call(kOpSlotOfName, args({ name, priority, target }));
    const reg32 op = guest::direct<0x4fb990, fileOpAllocate>(app, cpu, 0, cpu.edx, cpu.ebx, slot);
    guest.leaveEcx(op);
    guest::direct<0x4fbbe0, fileOpSetName>(app, cpu, op, name);
    guest.setDword(op + 0x24, mode);
    return guest::direct<0x4fbb60, fileOpQueue>(app, cpu, op);
}

/* sub_4fca90: an operation to close a file. */
x86::reg32 frontend::wina::fileOpClose(App* app, CPU& cpu, reg32 handle, reg32 priority, reg32 target)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 slot = guest::direct<0x4e4320, fileLockOf>(app, cpu, handle);
    const reg32 op = guest::direct<0x4fb990, fileOpAllocate>(app, cpu, 1, priority, target, slot);
    guest.setDword(op + 0x18, handle);
    return guest::direct<0x4fbb60, fileOpQueue>(app, cpu, op);
}

/* sub_4fcab0: an operation to read a file. */
x86::reg32 frontend::wina::fileOpRead(App* app, CPU& cpu, reg32 handle, reg32 offset, reg32 buffer, reg32 size,
                                      reg32 priority, reg32 target)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    const reg32 frame = guest.frame();
    guest.setDword(frame, size);
    // The game's code hands it the priority and the target in edx and ebx too, which it keeps there for the
    // next call.
    const reg32 slot = guest::direct<0x4e4320, fileLockOf>(app, cpu, handle);
    const reg32 op = guest::direct<0x4fb990, fileOpAllocate>(app, cpu, 2, priority, target, slot);
    guest.setDword(op + 0x18, handle);
    guest.setDword(op + 0x24, offset);
    guest.leaveEdx(guest.dword(frame));
    guest.setDword(op + 0x2c, buffer);
    guest.setDword(op + 0x28, guest.dword(frame));
    return guest::direct<0x4fbb60, fileOpQueue>(app, cpu, op);
}

/* sub_4fce50: the slot of a file's name: 0 when it has a '|', else by its drive. */
x86::reg32 frontend::wina::fileSlotOfName(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    for (reg32 at = name;; ++at)
    {
        safepoint(app, cpu);
        const std::uint8_t c = guest.byte(at);
        if (c == '|')
            return 0;
        if (c == 0)
            break;
    }
    guest.call(0x4e3330, args({ name }));
    return guest.eax();
}

/* sub_4fd060: a file closed and waited for. */
x86::reg32 frontend::wina::fileCloseWait(App* app, CPU& cpu, reg32 handle, reg32 priority)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 op = guest::direct<0x4fca90, fileOpClose>(app, cpu, handle, priority, 0);
    guest.leaveEdx(op);
    if (op == 0)
        return 0;
    // The operation in eax as the close leaves it; then in edx, which sub_4fc780 keeps.
    fe::direct<0x4fc780, menz::fileOpWait>(app, cpu, op);
    return fe::direct<0x4fc600, menz::fileOpComplete>(app, cpu, op);
}

/* sub_4e4230: a file's size, 0 when the handle is not of an open one or it keeps no
 * system handle; the file pointer put where asked.  Under the table's own lock. */
x86::reg32 frontend::wina::fileSizeAndSeek(App* app, CPU& cpu, reg32 handle, reg32 offset)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kTable = 0x5645a0, kCount = 0x56459c, kLocks = 0x9ef760;
    const reg32 index = ~handle;
    const reg32 table = guest.dword(kTable);
    guest.leaveEdx(table);
    if (table == 0 || sreg32(index) < 0 || sreg32(index) >= sreg32(guest.dword(kCount)))
        return 0;
    const reg32 place = index << 5;
    if (guest.byte(table + place) == 0)
        return 0;
    const reg32 lockIndex = guest.byte(table + place + 1);
    enterLock(guest, cpu, guest.dword(lockIndex * 4 + kLocks));
    callImport<0x5345f4, &win32::kernel32::SetLastError>(guest, cpu, 0);
    const reg32 entry = guest.dword(kTable) + place;
    reg32 size = 0;
    if (guest.dword(entry + 0x14) == 0)
    {
        size = callImport<0x5345d8, &win32::kernel32::SetEndOfFile>(guest, cpu, guest.dword(entry + 4));
        const reg32 moved = callImport<0x5345f0, &win32::kernel32::SetFilePointer>(
            guest, cpu, guest.dword(guest.dword(kTable) + place + 4), offset, 0, 0);
        guest.setDword(guest.dword(kTable) + place + 0x18, moved);
    }
    leaveLock(guest, cpu, guest.dword(lockIndex * 4 + kLocks));
    return size;
}

/* sub_4e4320: which lock of the table an open file's handle has; 0 when none. */
x86::reg32 frontend::wina::fileLockOf(App* app, CPU& cpu, reg32 handle)
{
    Guest guest(app, cpu);
    const reg32 index = ~handle;
    if (guest.dword(0x5645a0) == 0 || sreg32(index) < 0 || sreg32(index) >= sreg32(guest.dword(0x56459c)))
        return 0;
    const reg32 entry = guest.dword(0x5645a0) + (index << 5);
    if (guest.byte(entry) == 0)
        return 0;
    return guest.byte(entry + 1);
}

/* ---------------------------------------------------------------------------
 * The window's messages
 * ------------------------------------------------------------------------- */

/* sub_4cc650: two handler rows, by their message: for qsort and bsearch. */
x86::reg32 frontend::wina::compareHandlers(App* app, CPU& cpu, reg32 leftRow, reg32 rightRow)
{
    Guest guest(app, cpu);
    const reg32 right = guest.dword(rightRow);
    const reg32 left = guest.dword(leftRow);
    return left - right;
}

/* sub_4cc660: a message's handler, 0 when it has none. */
x86::reg32 frontend::wina::handlerOf(App* app, CPU& cpu, reg32 message)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    const reg32 frame = guest.frame();
    messageLockMade(guest, cpu);
    const reg32 lock = guest.dword(kMessageLock);
    guest.leaveEcx(lock);
    enterLock(guest, cpu, lock);
    guest.setDword(frame, message);
    const reg32 row = guest.call(kBsearch, args({ frame, kHandlers, guest.dword(kHandlerCount), 8 }, { kCompareHandlers }));
    const reg32 handler = row != 0 ? guest.dword(row + 4) : 0;
    leaveLock(guest, cpu, guest.dword(kMessageLock));
    return handler;
}

/* sub_4cc6d0: a message's handler set, changed, or taken away with 0; the rows stay
 * sorted.  1 when done, 0 when the table is full or it had none to take away. */
x86::reg32 frontend::wina::handlerSet(App* app, CPU& cpu, reg32 message, reg32 handler)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    constexpr reg32 kMostHandlers = 0x80, kPastEnd = 0x7fffffff;
    const reg32 frame = guest.frame();
    messageLockMade(guest, cpu);
    const reg32 lock = guest.dword(kMessageLock);
    guest.leaveEcx(lock);
    enterLock(guest, cpu, lock);
    guest.setDword(frame, message);
    const reg32 row = guest.call(kBsearch, args({ frame, kHandlers, guest.dword(kHandlerCount), 8 }, { kCompareHandlers }));
    reg32 done = 0;
    if (row != 0)
    {
        if (handler == 0)
        {
            const reg32 count = guest.dword(kHandlerCount);
            guest.setDword(row, kPastEnd);
            guest.call(kQsort, args({ kHandlers, count, 8, kCompareHandlers }));
            guest.setDword(kHandlerCount, guest.dword(kHandlerCount) - 1);
        }
        else
        {
            guest.setDword(row + 4, handler);
        }
        done = 1;
    }
    else if (handler != 0)
    {
        const reg32 count = guest.dword(kHandlerCount);
        if (count < kMostHandlers)
        {
            done = 1;
            guest.setDword(count * 8 + kHandlers, message);
            guest.setDword(count * 8 + kHandlers + 4, handler);
            guest.setDword(kHandlerCount, count + 1);
            guest.call(kQsort, args({ kHandlers, count + 1, 8, kCompareHandlers }));
        }
    }
    const reg32 lockAgain = guest.dword(kMessageLock);
    guest.leaveEcx(lockAgain);
    leaveLock(guest, cpu, lockAgain);
    return done;
}

/* sub_4cc7c0: a message to the game's window, sent or posted; 1 when it went. */
x86::reg32 frontend::wina::postMessage(App* app, CPU& cpu, reg32 message, reg32 lParam, reg32 wParam, reg32 send)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    const reg32 hwnd = guest.dword(kHwnd);
    guest.leaveEbx(0);
    if (hwnd == 0)
        return 0;
    reg32 failed = 0;
    if (send != 0)
    {
        failed = callImport<0x534778, &win32::user32::SendNotifyMessageA>(guest, cpu, hwnd, message, wParam, lParam);
    }
    else if (callImport<0x534768, &win32::user32::PostMessageA>(guest, cpu, hwnd, message, wParam, lParam) == 0)
    {
        failed = callImport<kLastError, &win32::kernel32::GetLastError>(guest, cpu);
    }
    guest.leaveEbx(failed);
    return failed == 0 ? 1 : 0;
}

/* sub_4cc860: who is told when the window gets or loses the focus. */
x86::reg32 frontend::wina::setFocusCallback(App* app, CPU& cpu, reg32 callback)
{
    Guest guest(app, cpu);
    guest.setDword(kFocusCallback, callback);
    return callback;
}

/* sub_4cc870: the system task that leaves the game when there is no window. */
x86::reg32 frontend::wina::noWindowTask(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, cpu, 0x4cc870);
    fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    return 0;
}

/* sub_4cc890: the system's cursor put at a place of the window. */
x86::reg32 frontend::wina::movePointer(App* app, CPU& cpu, reg32 window, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    const reg32 frame = guest.frame();
    guest.setDword(frame, x);
    guest.setDword(frame + 4, y);
    const reg32 hwnd = guest.dword(window + kWindowHandle);
    guest.leaveEdx(hwnd);
    if (callImport<kClientToScreen, &win32::user32::ClientToScreen>(guest, cpu, hwnd, frame) != 0)
    {
        const reg32 screenY = guest.dword(frame + 4), screenX = guest.dword(frame);
        guest.leaveEcx(screenY);
        guest.leaveEbx(screenX);
        callImport<0x534784, &win32::user32::SetCursorPos>(guest, cpu, screenX, screenY);
    }
    else
    {
        callImport<0x534784, &win32::user32::SetCursorPos>(guest, cpu, x, y);
    }
    callImport<kSetCursor, &win32::user32::SetCursor>(guest, cpu, 0);
    return guest.eax();
}

/* sub_4cc8e0: a key on the queue, the oldest dropped when it is full. */
x86::reg32 frontend::wina::pushKey(App* app, CPU& cpu, reg32 key, reg32 scan, reg32 flags)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    guest.setDword(guest.frame(), scan);
    const reg32 tail = guest.dword(kKeyTail);
    const reg32 extended = (flags & 0xffff00ffu) | ((scan & 0xff) << 8);
    guest.leaveEbx(extended);
    const reg32 lock = guest.dword(kKeyLock);
    guest.leaveEdx(lock);
    enterLock(guest, cpu, lock);
    guest.setByte(tail * 4 + kKeyQueue + 1, std::uint8_t(extended >> 8));
    guest.setByte(tail * 4 + kKeyQueue, std::uint8_t(extended));
    const reg32 room = guest.dword(kKeyRoom);
    std::int64_t next = sreg32(guest.dword(kKeyTail) + 1);
    reg32 quotient = reg32(next / sreg32(room));
    guest.setDword(kKeyTail, reg32(next % sreg32(room)));
    reg32 left = guest.dword(kKeyHead);
    const reg32 newTail = guest.dword(kKeyTail);
    guest.setWord(tail * 4 + kKeyQueue + 2, std::uint16_t(key));
    if (left == newTail)
    {
        next = sreg32(guest.dword(kKeyHead) + 1);
        quotient = reg32(next / sreg32(room));
        left = reg32(next % sreg32(room));
        guest.setDword(kKeyHead, left);
    }
    guest.leaveEdx(left);
    const reg32 lockAgain = guest.dword(kKeyLock);
    guest.leaveEbx(lockAgain);
    cpu.eax = quotient;
    callImport<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(guest, cpu, lockAgain);
    return guest.eax();
}

/* sub_4ccaa0: the key code of the arrows and the function keys, from a key-down message's
 * virtual key; 0 for anything else.  The generated code's jumps are one table. */
x86::reg32 frontend::wina::keyOfMessage(App*, CPU&, reg32 message, reg32 virtualKey)
{
    constexpr reg32 kKeyDown = 0x100, kSystemKeyDown = 0x104;
    if (message != kSystemKeyDown && message != kKeyDown)
        return 0;
    switch (virtualKey)
    {
    case 0x21: case 0x69: return 0x4900;          // page up
    case 0x22: case 0x63: return 0x5100;          // page down
    case 0x23: case 0x61: return 0x4f00;          // end
    case 0x24: case 0x67: return 0x4700;          // home
    case 0x25: case 0x64: return 0x4b00;          // left
    case 0x26: case 0x68: return 0x4800;          // up
    case 0x27: case 0x66: return 0x4d00;          // right
    case 0x28: case 0x62: return 0x5000;          // down
    case 0x2d: case 0x60: return 0x5200;          // insert
    case 0x2e: return 0x5300;                     // delete
    case 0x65: return 0x4c00;                     // the keypad's middle
    case 0x70: return 0x3b00;                     // F1..F12
    case 0x71: return 0x3c00;
    case 0x72: return 0x3d00;
    case 0x73: return 0x3e00;
    case 0x74: return 0x3f00;
    case 0x75: return 0x4000;
    case 0x76: return 0x4100;
    case 0x77: return 0x4200;
    case 0x78: return 0x4300;
    case 0x79: return 0x4400;
    case 0x7a: return 0x8700;
    case 0x7b: return 0x8800;
    default: return 0;
    }
}

/* sub_4ccc70: the low-level keyboard hook: Alt+Tab, Alt+Esc, Ctrl+Esc and the Windows
 * keys told to the game instead of the system. */
x86::reg32 frontend::wina::keyboardHook(App* app, CPU& cpu, reg32 code, reg32 wParam, reg32 lParam)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kVkTab = 9, kVkEscape = 0x1b, kVkControl = 0x11;
    guest.leaveEdx(code);
    reg32 swallowed = 0, modifier = 0, key = cpu.edi;
    if (code == 0 && guest.dword(kActive) != 0)
    {
        if (guest.dword(kHookAlt) != 0 && (std::uint8_t(lParam >> 24) & 0x20) != 0)
        {
            modifier = 0x38;
            if (wParam == kVkTab)
            {
                guest.leaveEcx(1);
                guest.setDword(0x559448, 1);
                key = 0xf;
                swallowed = 1;
            }
            else if (wParam == kVkEscape)
            {
                key = 1;
                swallowed = 1;
            }
        }
        else
        {
            const reg32 state = callImport<0x534738, &win32::user32::GetAsyncKeyState>(guest, cpu, kVkControl);
            if (std::int16_t(state) < 0)
            {
                modifier = 0x1d;
                if (wParam == kVkEscape)
                {
                    key = 1;
                    swallowed = 1;
                }
            }
            else if (wParam == 0x5b || wParam == 0x5c || wParam == 0x5d || wParam == 0x2c)
            {
                swallowed = 1;
                key = wParam;
                guest.leaveEdx(0);
                modifier = 0;
            }
        }
        if (swallowed != 0 && guest.dword(kHookCallback) != 0)
            swallowed = guest.call(guest.dword(kHookCallback), args({ modifier, key }));
    }
    if (swallowed != 0)
        return 1;
    return callImport<0x5346f4, &win32::user32::CallNextHookEx>(guest, cpu, guest.dword(kHookHandle), code, wParam, lParam);
}

/* sub_4ccd60: the window has the focus: hook and callback. */
x86::reg32 frontend::wina::onSetFocus(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    if (window != 0)
    {
        guest.leaveEcx(hwnd);
        if (hwnd == guest.dword(kHwnd))
        {
            const reg32 wanted = guest.dword(kHookWanted);
            guest.setDword(kActive, 1);
            if (wanted != 0 && guest.dword(kHookHandle) == 0)
                guest.setDword(kHookHandle,
                               callImport<kSetHook, &win32::user32::SetWindowsHookExA>(guest, cpu, 2, kKeyboardHook, 0, 0));
            if (guest.dword(kFocusCallback) != 0)
                guest.call(guest.dword(kFocusCallback), args({ 1 }));
        }
    }
    return 0;
}

/* sub_4ccdd0: the window has lost the focus: the hook off, the keys let go. */
x86::reg32 frontend::wina::onKillFocus(App* app, CPU& cpu, reg32, reg32 hwnd, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    guest.leaveEdx(hwnd);
    if (hwnd == guest.dword(kHwnd))
    {
        guest.leaveEcx(0);
        const reg32 wanted = guest.dword(kHookWanted);
        guest.setDword(kActive, 0);
        if (wanted != 0)
        {
            const reg32 hook = guest.dword(kHookHandle);
            if (hook != 0)
            {
                callImport<kUnhook, &win32::user32::UnhookWindowsHookEx>(guest, cpu, hook);
                guest.setDword(kHookHandle, 0);
            }
        }
        if (guest.dword(kFocusCallback) != 0)
            guest.call(guest.dword(kFocusCallback), args({ 0 }));
        callImport<kShowCursor, &win32::user32::ShowCursor>(guest, cpu, 1);
        guest.call(0x4e070c, args({ kKeysDown, 0x80 }));
    }
    return 0;
}

/* sub_4cce40: WM_CLOSE. */
x86::reg32 frontend::wina::onClose(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    if (window != 0)
        callImport<0x53476c, &win32::user32::PostQuitMessage>(guest, cpu, 0);
    return 1;
}

/* sub_4cce60: the game's own message to close the window: the thread does it. */
x86::reg32 frontend::wina::onQuitAsked(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    if (window != 0)
        guest.setDword(kQuitAsked, 1);
    return 1;
}

/* sub_4cce80: a message said to be handled, and no more. */
x86::reg32 frontend::wina::onDestroy(App*, CPU&, reg32, reg32, reg32, reg32, reg32, reg32)
{
    return 0;
}

/* sub_4cce90: WM_SYSCOMMAND: closing, sizing and the screen saver are nobody's in a
 * fullscreen window. */
x86::reg32 frontend::wina::onSysCommand(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32 wParam, reg32, reg32)
{
    Guest guest(app, cpu);
    const reg32 command = wParam;
    if (command == 0xf060 || command == 0xf020 || command == 0xf030 || command == 0xf120)
        return 0;
    guest.leaveEdx(window);
    return guest.byte(window + kWindowFullscreen);
}

/* sub_4cced0: WM_SETTINGCHANGE: in fullscreen, they are turned off again. */
x86::reg32 frontend::wina::onSettingChange(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32, reg32, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    if (window != 0 && guest.byte(window + kWindowFullscreen) != 0)
        accessKeysOff(guest, cpu);
    guest.setDword(result, 0);
    return 0;
}

/* sub_4ccf50: the window was moved or sized: its place kept, the resize told. */
x86::reg32 frontend::wina::onMoved(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    const reg32 frame = guest.frame();
    if (window != 0 && guest.byte(window + kWindowFullscreen) == 0)
    {
        const reg32 hwnd = guest.dword(window + kWindowHandle);
        guest.leaveEdx(hwnd);
        callImport<kClientRect, &win32::user32::GetClientRect>(guest, cpu, hwnd, frame);
        const reg32 hwndAgain = guest.dword(window + kWindowHandle);
        guest.leaveEcx(hwndAgain);
        callImport<kClientToScreen, &win32::user32::ClientToScreen>(guest, cpu, hwndAgain, frame);
        guest.setDword(window + kWindowX, guest.dword(frame));
        guest.setDword(window + kWindowY, guest.dword(frame + 4));
        if (guest.dword(kOnResize) != 0)
            guest.call(guest.dword(kOnResize));
    }
    return 0;
}

/* sub_4ccfc0: WM_CHAR: the character on the key queue, as many times as it repeats. */
x86::reg32 frontend::wina::onChar(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32 wParam, reg32 lParam, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    const reg32 character = wParam, flags = lParam;
    if (window != 0)
    {
        sreg32 count = sreg32(flags & 0xffff);
        const sreg32 room = sreg32(guest.dword(kKeyRoom));
        if (count > room)
            count = room;
        for (--count; count != -1; --count)
        {
            safepoint(app, cpu);
            guest.leaveEcx(reg32(count));
            guest::direct<0x4cc8e0, pushKey>(app, cpu, character, (flags >> 16) & 0x7f, (flags >> 24) & 1);
        }
        guest.leaveEcx(0xffffffff);
    }
    return 1;
}

/* sub_4cd020: WM_KEYDOWN: the key down in the table, and on the queue when it is one
 * the game asks for by scan code or an arrow or function key. */
x86::reg32 frontend::wina::onKeyDown(App* app, CPU& cpu, reg32 window, reg32, reg32 message, reg32 wParam, reg32 lParam,
                                     reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    const reg32 virtualKey = wParam, flags = lParam;
    guest.leaveEdx(virtualKey);
    if (window == 0)
        return 1;
    sreg32 count = sreg32(flags & 0xffff);
    const sreg32 room = sreg32(guest.dword(kKeyRoom));
    if (count > room)
        count = room;
    guest.leaveEcx(reg32(count));
    guest.setDword(kLastScan, (flags >> 16) & 0x7f);
    const reg32 table = guest.dword(kScanTable);
    if (table != 0)
        guest.setDword(kLastScan, guest.byte(table + guest.dword(kLastScan)));
    guest.setByte(guest.dword(kLastScan) + kKeysDown, 1);
    reg32 key = 0;
    if (guest.dword(kWantRaw) == 0)
    {
        key = guest::direct<0x4ccaa0, keyOfMessage>(app, cpu, message, virtualKey);
        if (key == 0)
            return 0;
    }
    for (--count; count != -1; --count)
    {
        safepoint(app, cpu);
        guest.leaveEcx(reg32(count));
        guest::direct<0x4cc8e0, pushKey>(app, cpu, key, guest.dword(kLastScan), (flags >> 24) & 1);
    }
    guest.leaveEcx(0xffffffff);
    return 1;
}

/* sub_4cd0e0: WM_KEYUP: the key up in the table; the two shifts go together. */
x86::reg32 frontend::wina::onKeyUp(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32, reg32 lParam, reg32)
{
    Guest guest(app, cpu);
    if (window != 0)
    {
        reg32 scan = (lParam >> 16) & 0x7f;
        const reg32 table = guest.dword(kScanTable);
        guest.leaveEcx(table);
        if (table != 0)
        {
            guest.leaveEdx(table);
            scan = guest.byte(table + scan);
        }
        if (scan == 0x2a || scan == 0x36)
        {
            guest.leaveEdx(cpu.edx & 0xffff00ffu);
            guest.setByte(kKeysDown + 0x2a, 0);
            guest.setByte(kKeysDown + 0x36, 0);
        }
        else
        {
            guest.leaveEdx(cpu.edx & 0xffffff00u);
            guest.setByte(scan + kKeysDown, 0);
        }
    }
    return 0;
}

/* sub_4cd130: a mouse button pressed. */
x86::reg32 frontend::wina::onMouseDown(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32 wParam, reg32 lParam,
                                       reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    pointerMessage(guest, cpu, window, wParam, lParam, 1);
    return 0;
}

/* sub_4cd190: a mouse button let go, or the mouse moved. */
x86::reg32 frontend::wina::onMouseUp(App* app, CPU& cpu, reg32 window, reg32, reg32, reg32 wParam, reg32 lParam, reg32)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    pointerMessage(guest, cpu, window, wParam, lParam, 0);
    return 0;
}

/* sub_4cd1f0: WM_PAINT: painted with nothing, under the graphics' lock. */
x86::reg32 frontend::wina::onPaint(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32, reg32, reg32, reg32 result)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 80);
    const reg32 frame = guest.frame();
    const reg32 other = hwnd;
    if (window != 0 && guest.dword(window + kWindowHandle) != 0)
    {
        if (guest.byte(window + kWindowFullscreen) != 0)
        {
            guest.leaveEcx(other);
            callImport<kForeground, &win32::user32::SetForegroundWindow>(guest, cpu, other);
        }
        enterLock(guest, cpu, guest.dword(kGraphicsLock));
        callImport<0x5346f0, &win32::user32::BeginPaint>(guest, cpu, guest.dword(window + kWindowHandle), frame);
        callImport<0x534730, &win32::user32::EndPaint>(guest, cpu, guest.dword(window + kWindowHandle), frame);
        const reg32 lock = guest.dword(kGraphicsLock);
        guest.leaveEdx(lock);
        leaveLock(guest, cpu, lock);
        if (guest.dword(kOnResize) != 0 && guest.byte(window + kWindowFullscreen) == 0)
            guest.call(guest.dword(kOnResize));
        guest.setDword(result, 1);
    }
    return 1;
}

/* sub_4cd2a0: WM_ERASEBKGND of the fullscreen window: said to be done. */
x86::reg32 frontend::wina::onEraseBackground(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32, reg32, reg32, reg32)
{
    Guest guest(app, cpu);
    if (window != 0)
    {
        guest.leaveEdx(hwnd);
        if (hwnd == guest.dword(window + kWindowHandle) && guest.byte(window + kWindowFullscreen) != 0)
            return 1;
    }
    return 0;
}

/* sub_4cd2d0: the window's procedure: the table's handler, else the system's. */
x86::reg32 frontend::wina::windowProc(App* app, CPU& cpu, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
    const reg32 frame = guest.frame();
    guest.setDword(frame, message);
    guest.setDword(frame + 8, 0);
    const reg32 row = guest.call(kBsearch, args({ frame, kHandlers, guest.dword(kHandlerCount), 8 }, { kCompareHandlers }));
    guest.leaveEdx(row);
    reg32 handled = 0;
    if (row != 0)
    {
        guest.leaveEcx(lParam);
        handled = guest.call(guest.dword(row + 4), args({}, { kWindow, hwnd, message, wParam, lParam, frame + 8 }));
    }
    if (handled != 0)
        return guest.dword(frame + 8);
    guest.leaveEdx(wParam);
    guest.leaveEcx(hwnd);
    cpu.eax = lParam;
    return callImport<0x534724, &win32::user32::DefWindowProcA>(guest, cpu, hwnd, message, wParam, lParam);
}

/* sub_4cd350: the window created, fullscreen or framed; its handle.  With none it stays
 * as it is; the arrows and such of the scan table's three keyboards are set up. */
x86::reg32 frontend::wina::createWindow(App* app, CPU& cpu, reg32 window)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 44);
    const reg32 frame = guest.frame();
    reg32 style = guest.dword(0x56128c);
    reg32 extended, width, height;
    if (guest.byte(window + kWindowFullscreen) != 0)
    {
        guest.setDword(window + kWindowX, 0);
        extended = 8;
        guest.setDword(window + kWindowY, 0);
        width = callImport<kScreenMetric, &win32::user32::GetSystemMetrics>(guest, cpu, 0);
        style |= 0x80000000;
        height = callImport<kScreenMetric, &win32::user32::GetSystemMetrics>(guest, cpu, 1);
    }
    else
    {
        extended = 0x40000;
        style |= 0xca0000;
        if ((guest.byte(0x564390) & 0x20) != 0)
        {
            style |= 0x50000;
        }
        else
        {
            extended = 0x40100;
            style |= 0x800000;
        }
        if (guest.dword(window + kWindowHandle) != 0)
        {
            guest.setDword(frame + 4, guest.dword(window + kWindowY));
            guest.setDword(frame, guest.dword(window + kWindowX));
        }
        else
        {
            guest.setDword(frame, 0xffffe000);
            guest.setDword(frame + 4, 0xffffe000);
        }
        guest.setDword(frame + 12, guest.dword(frame + 4) + guest.dword(window + 0x18));
        guest.setDword(frame + 8, guest.dword(frame) + guest.dword(window + 0x14));
        callImport<kAdjustRect, &win32::user32::AdjustWindowRect>(guest, cpu, frame, style, 0);
        guest.setDword(window + kWindowX, guest.dword(frame));
        guest.setDword(window + kWindowY, guest.dword(frame + 4));
        width = guest.dword(frame + 8) - guest.dword(frame);
        height = guest.dword(frame + 12) - guest.dword(frame + 4);
    }
    const reg32 created = callImport<0x534704, &win32::user32::CreateWindowExA>(
        guest, cpu, extended, guest.dword(0x5612a0), guest.dword(0x5612a4), style, guest.dword(window + kWindowX),
        guest.dword(window + kWindowY), width, height, 0, 0, guest.dword(kInstance), 0);
    if (created != 0)
    {
        callImport<kSetCursor, &win32::user32::SetCursor>(guest, cpu, 0);
        if (guest.byte(window + kWindowFullscreen) != 0)
            callImport<kShowCursor, &win32::user32::ShowCursor>(guest, cpu, 0);
        guest.setDword(0x564814, 0);
        if (guest.dword(kScanTable) == 0)
        {
            fe::direct<0x4fb710, wata::lockKeySet>(app, cpu, 0, 0);
            fe::direct<0x4fb710, wata::lockKeySet>(app, cpu, 1, 0);
            fe::direct<0x4fb710, wata::lockKeySet>(app, cpu, 2, 0);
        }
    }
    return created;
}

/* sub_4cd4e0: the window's thread: makes the window, tells who waits, pumps its
 * messages until it is gone. */
x86::reg32 frontend::wina::windowThread(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 68);
    const reg32 frame = guest.frame();
    const reg32 message = frame, rect = frame + 0x1c;
    const reg32 hwnd = guest::direct<0x4cd350, createWindow>(app, cpu, kWindow);
    guest.setDword(kWindowReady, reg32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu)));
    // The event in eax as the call before leaves it.
    fe::direct<0x4dfb70, ea::eventWaited>(app, cpu, guest.dword(kWindowReady));
    guest.call(0x4dfc00, args({ guest.dword(kWindowReady), 0 }));
    const std::uint8_t fullscreen = guest.byte(kFullscreen);
    guest.setDword(kHwnd, hwnd);
    const reg32 zero = cpu.edx;
    guest.setDword(kQuitAsked, zero);
    guest.setDword(kWindowReady, zero);
    if (fullscreen == 0)
    {
        callImport<kClientRect, &win32::user32::GetClientRect>(guest, cpu, hwnd, rect);
        callImport<kClientToScreen, &win32::user32::ClientToScreen>(guest, cpu, hwnd, rect);
        guest.setDword(0x9f3d34, guest.dword(rect));
        guest.setDword(0x9f3d38, guest.dword(rect + 4));
    }
    else
    {
        callImport<kForeground, &win32::user32::SetForegroundWindow>(guest, cpu, hwnd);
        guest.setDword(0x9f3d34, 0);
        guest.setDword(0x9f3d38, 0);
    }
    callImport<0x53477c, &win32::user32::SetActiveWindow>(guest, cpu, hwnd);
    guest.setDword(kActive, 1);
    for (;;)
    {
        safepoint(app, cpu);
        if (hwnd != guest.dword(kHwnd))
            break;
        if (callImport<0x53474c, &win32::user32::GetMessageA>(guest, cpu, message, 0, 0, 0) != 0)
        {
            enterLock(guest, cpu, guest.dword(kMessageLock));
            if (guest.dword(kWantRaw) == 0
                && guest::direct<0x4ccaa0, keyOfMessage>(app, cpu, guest.dword(message + 4), guest.dword(message + 8)) == 0)
                callImport<0x5347a4, &win32::user32::TranslateMessage>(guest, cpu, message);
            callImport<0x53472c, &win32::user32::DispatchMessageA>(guest, cpu, message);
            leaveLock(guest, cpu, guest.dword(kMessageLock));
        }
        else if (guest.dword(0x56129c) != 0)
        {
            guest.call(guest.dword(0x56129c));
        }
        else
        {
            // By address, as sub_4f9070 below: the thread's answer is what the loop's last call left in eax.
            guest.call(0x4e7520, args({ 0x4cc870, 1, 10 }));
        }
        if (guest.dword(kQuitAsked) != 0)
        {
            const reg32 surface = guest.dword(kWindow);
            guest.setDword(kQuitAsked, 0);
            if (surface != 0 && guest.dword(0x9f38e0) != 0)
                fe::direct<0x4ecd50, eab::ddrawDisposeDisplay>(app, cpu, kWindow);
            callImport<0x534728, &win32::user32::DestroyWindow>(guest, cpu, guest.dword(kHwnd));
            guest.setDword(kHwnd, 0);
        }
        if (guest.byte(kFullscreen) != 0)
        {
            if (guest::direct<0x4f9120, hasFocus>(app, cpu) != 0)
            {
                if (guest.call(0x4f9070) == 0)
                    callImport<kShowCursor, &win32::user32::ShowCursor>(guest, cpu, 0);
            }
            else if (callImport<0x53475c, &win32::user32::IsIconic>(guest, cpu, guest.dword(kHwnd)) == 0)
            {
                callImport<kForeground, &win32::user32::SetForegroundWindow>(guest, cpu, guest.dword(kHwnd));
            }
        }
    }
    return guest.eax();
}

/* sub_4cd6c0: the game's window, opened or changed for a mode: the class and the
 * handlers the first time, its thread each time; its handle. */
x86::reg32 frontend::wina::openWindow(App* app, CPU& cpu, reg32 width, reg32 height, reg32 isFullscreen)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 84);
    cpu.ebx = isFullscreen;  // as the game's code has it here, for the reads below
    static constexpr struct { reg32 message, handler; } kHandlersOfWindow[] = {
        { 7, 0x4ccd60 }, { 8, 0x4ccdd0 }, { 5, 0x4ccf50 }, { 3, 0x4ccf50 }, { 0x10, 0x4cce40 }, { 2, 0x4cce80 },
        { 0xf, 0x4cd1f0 }, { 0x14, 0x4cd2a0 }, { 0x1a, 0x4cced0 }, { 0x112, 0x4cce90 }, { 0x466, 0x4cce60 },
        { 0x102, 0x4ccfc0 }, { 0x100, 0x4cd020 }, { 0x104, 0x4cd020 }, { 0x101, 0x4cd0e0 }, { 0x105, 0x4cd0e0 },
        { 0x201, 0x4cd130 }, { 0x207, 0x4cd130 }, { 0x204, 0x4cd130 }, { 0x203, 0x4cd130 }, { 0x209, 0x4cd130 },
        { 0x206, 0x4cd130 }, { 0x202, 0x4cd190 }, { 0x208, 0x4cd190 }, { 0x205, 0x4cd190 }, { 0x200, 0x4cd190 },
    };
    constexpr reg32 kClassMade = 0x7f7490, kClassName = 0x5612a4, kWindowName = 0x5612a0;
    constexpr reg32 kWidth = 0x9f38e4, kHeight = 0x9f38e8, kThreadRecord = 0x7f7460;
    const reg32 frame = guest.frame();
    const reg32 thread = frame + 0x28;
    if (guest.dword(kMessageLock) == 0)
        guest.setDword(kMessageLock, *fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    if (guest.byte(kClassMade) == 0)
    {
        guest.setDword(kInstance, callImport<0x53454c, &win32::kernel32::GetModuleHandleA>(guest, cpu, 0));
        if (guest.dword(kClassName) == 0)
        {
            const reg32 names = guest.dword(0x9f38c0);
            guest.setDword(kClassName, names != 0 ? guest.dword(names) : 0x5612ac);
        }
        guest.setDword(frame, 0xb);
        guest.setDword(frame + 4, 0x4cd2d0);
        const reg32 instance = guest.dword(kInstance);
        guest.setDword(frame + 8, 0);
        guest.setDword(frame + 12, 0);
        guest.setDword(frame + 16, instance);
        const reg32 icon = callImport<0x534760, &win32::user32::LoadIconA>(guest, cpu, instance, guest.dword(0x5612a8));
        guest.leaveEdx(0);
        guest.setDword(frame + 20, icon);
        guest.setDword(frame + 24, 0);
        guest.setDword(frame + 28, callImport<0x53445c, &win32::gdi32::GetStockObject>(guest, cpu, 4));
        guest.setDword(frame + 32, guest.dword(kClassName));
        guest.setDword(frame + 36, guest.dword(kWindowName));
        if ((callImport<0x534774, &win32::user32::RegisterClassA>(guest, cpu, frame) & 0xffff) == 0)
        {
            const reg32 code = callImport<kLastError, &win32::kernel32::GetLastError>(guest, cpu);
            guest.call(kError, args({}, { 0x540468, code }));
            return 0;
        }
        guest.setDword(kKeyLock, *fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
        guest.leaveEdx((cpu.edx & 0xffffff00u) | 1);
        guest.setDword(kKeyRoom, 0x20);
        guest.setDword(kKeyHead, 0);
        guest.setDword(kKeyTail, 0);
        guest.setByte(kClassMade, 1);
        for (const auto& row : kHandlersOfWindow)
            guest::direct<0x4cc6d0, handlerSet>(app, cpu, row.message, row.handler);
    }
    else if (guest.dword(kHwnd) != 0 && cpu.ebx != guest.byte(kFullscreen))
    {
        guest::direct<0x4cdbd0, centreWindow>(app, cpu, width, height);
        return 1;
    }
    guest.setByte(kFullscreen, std::uint8_t(cpu.ebx));
    guest.setDword(kWidth, width);
    guest.setDword(kActive, 1);
    guest.setDword(kHeight, height);
    if (guest.call(0x4df7a0, args({ 0x4cd4e0, 0, 1, 0xffffffff }, { thread })) == 0)
        return 0;
    while (guest.dword(kWindowReady) == 0)
    {
        safepoint(app, cpu);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    }
    if (guest.dword(kHwnd) != 0)
        guest::direct<0x4cdaa0, closeWindow>(app, cpu);
    // The event stays in edx after the call, as the game's code has it, for the end below when the wait does not
    // run; the thread it hands in eax is not read.
    const reg32 ready = guest.dword(kWindowReady);
    guest.leaveEdx(ready);
    fe::direct<0x4dfb20, ea::eventPosted>(app, cpu, ready);
    while (guest.dword(kWindowReady) != 0)
    {
        safepoint(app, cpu);
        guest.call(kSleep, args({ 0 }));
        guest.call(kSystemTasks, args({ 0 }));
    }
    const std::uint8_t fullscreen = guest.byte(kFullscreen);
    guest.leaveEdx((cpu.edx & 0xffff00ffu) | (reg32(fullscreen) << 8));
    copyDwords(guest, kThreadRecord, thread, 5);
    if (fullscreen != 0)
    {
        accessKeysOff(guest, cpu);
    }
    else
    {
        const reg32 hwnd = guest.dword(kHwnd);
        guest.leaveEdx(hwnd);
        callImport<kForeground, &win32::user32::SetForegroundWindow>(guest, cpu, hwnd);
        guest.leaveEcx(0);
        guest::direct<0x4cdbd0, centreWindow>(app, cpu, width, height);
        guest.setDword(kStickyKeys, 0);
        guest.setDword(kFilterKeys, 0);
    }
    return guest.dword(kHwnd);
}

/* sub_4cdaa0: the window closed: the keys the system had put back, and waited for. */
x86::reg32 frontend::wina::closeWindow(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    if (guest.dword(kHwnd) == 0)
        return guest.eax();
    const reg32 sticky = guest.dword(kStickyKeys);
    if (sticky != 0)
        callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x11, sticky, 0, 2);
    const reg32 filter = guest.dword(kFilterKeys);
    if (filter != 0)
        callImport<kSystemParameters, &win32::user32::SystemParametersInfoA>(guest, cpu, 0x56, filter, 0, 2);
    reg32 left = guest::direct<0x4cc7c0, postMessage>(app, cpu, 0x466, 0, 0, 0);  // what the calls leave in eax
    while (guest.dword(kHwnd) != 0)
    {
        safepoint(app, cpu);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
        left = reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0));
    }
    return left;
}

/* sub_4cdb10: the window put at a place with a size of client area; what came kept. */
x86::reg32 frontend::wina::placeWindow(App* app, CPU& cpu, reg32 left, reg32 top, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 28);
            const reg32 frame = guest.frame();
            guest.setDword(frame, left);
            guest.setDword(frame + 4, top);
            guest.setDword(frame + 12, top + height);
            const reg32 hwnd = guest.dword(kHwnd);
            guest.leaveEdx(hwnd);
            guest.setDword(frame + 8, left + width);
            // AdjustWindowRect's last argument is pushed first, under GetWindowLong's.
            cpu.esp -= 4;
            guest.setDword(cpu.esp, 0);
            cpu.eax = left + width;
            const reg32 style = callImport<0x534758, &win32::user32::GetWindowLongA>(guest, cpu, hwnd, 0xfffffff0);
    #ifdef NFS_GAME_CHECKS
            // Through the slot: the two words pushed now, over the one pushed first.
            callImport<kAdjustRect, &win32::user32::AdjustWindowRect>(guest, cpu, frame, style);
    #else
            callImport<kAdjustRect, &win32::user32::AdjustWindowRect>(guest, cpu, frame, style, guest.dword(cpu.esp));
    #endif
            cpu.esp += 4;
            const reg32 newTop = guest.dword(frame + 4), newLeft = guest.dword(frame);
            guest.leaveEcx(newTop);
            guest.leaveEbx(newLeft);
            callImport<0x53478c, &win32::user32::SetWindowPos>(guest, cpu, guest.dword(kHwnd), 0, newLeft, newTop,
                                                               guest.dword(frame + 8) - newLeft, guest.dword(frame + 12) - newTop, 0x14);
            const reg32 hwnd2 = guest.dword(kHwnd);
            guest.leaveEdx(hwnd2);
            callImport<kClientRect, &win32::user32::GetClientRect>(guest, cpu, hwnd2, frame);
            const reg32 hwnd3 = guest.dword(kHwnd);
            guest.leaveEcx(hwnd3);
            callImport<kClientToScreen, &win32::user32::ClientToScreen>(guest, cpu, hwnd3, frame);
            const reg32 hwnd4 = guest.dword(kHwnd);
            guest.leaveEbx(hwnd4);
            callImport<kClientToScreen, &win32::user32::ClientToScreen>(guest, cpu, hwnd4, frame + 8);
            const reg32 clientLeft = guest.dword(frame), clientTop = guest.dword(frame + 4);
            guest.setDword(0x9f38e4, guest.dword(frame + 8) - clientLeft);
            guest.setDword(0x9f3d34, clientLeft);
            guest.setDword(0x9f3d38, clientTop);
            const reg32 clientHeight = guest.dword(frame + 12) - clientTop;
            guest.setDword(0x9f38e8, clientHeight);
            return clientHeight;
}

/* sub_4cdbd0: the window centred on the screen. */
x86::reg32 frontend::wina::centreWindow(App* app, CPU& cpu, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    const sreg32 spareY = sreg32(callImport<kScreenMetric, &win32::user32::GetSystemMetrics>(guest, cpu, 1) - height);
    const reg32 top = reg32(spareY / 2);
    const sreg32 spareX = sreg32(callImport<kScreenMetric, &win32::user32::GetSystemMetrics>(guest, cpu, 0) - width);
    const reg32 left = reg32(spareX / 2);
    return guest::direct<0x4cdb10, placeWindow>(app, cpu, left, top, width, height);
}

/* sub_4cdc50: the keyboard hook off, in fullscreen. */
x86::reg32 frontend::wina::hookOff(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    if (guest.byte(kFullscreen) != 0)
    {
        const reg32 hook = guest.dword(kHookHandle);
        if (hook != 0)
        {
            callImport<kUnhook, &win32::user32::UnhookWindowsHookEx>(guest, cpu, hook);
            guest.setDword(kHookHandle, 0);
            guest.setDword(kHookWanted, 0);
        }
    }
    return guest.eax();
}

/* sub_4cdc90: the keyboard hook on, with who is told. */
x86::reg32 frontend::wina::hookOn(App* app, CPU& cpu, reg32 alt, reg32 callback)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    guest.setDword(kHookCallback, callback);
    const reg32 hook = guest.dword(kHookHandle);
    guest.leaveEdx(hook);
    guest.setDword(kHookAlt, alt);
    if (hook == 0)
    {
        const reg32 made = callImport<kSetHook, &win32::user32::SetWindowsHookExA>(guest, cpu, 2, kKeyboardHook, 0, 0);
        guest.setDword(kHookHandle, made);
        guest.setDword(kHookWanted, 1);
    }
    return guest.eax();
}

/* sub_4fb470: the mouse: the place held inside the clip, the system's cursor put back
 * when it was outside, the place, buttons and the click kept under the table's lock. */
x86::reg32 frontend::wina::pointerMoved(App* app, CPU& cpu, reg32 x, reg32 y, reg32 buttons, reg32 click)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    constexpr reg32 kClipLeft = 0x9f4f4c, kClipTop = 0x9f4f50, kClipRight = 0x9f4f44, kClipBottom = 0x9f4f40;
    constexpr reg32 kButtons = 0x9f4f54, kLastX = 0x9f4f3c, kLastY = 0x9f4f38;
    const reg32 frame = guest.frame();
    guest.setDword(frame + 4, buttons);
    guest.setDword(frame, click);
    if (guest.dword(0x564368) == 0 || guest.dword(kMessageLock) == 0)
        return x;

    const sreg32 left = sreg32(guest.dword(kClipLeft));
    reg32 heldX = guest.dword(kClipRight);
    if ((sreg32(x) > left ? sreg32(x) : left) < sreg32(heldX))
        heldX = sreg32(x) <= left ? reg32(left) : x;
    const sreg32 top = sreg32(guest.dword(kClipTop));
    const reg32 bottom = guest.dword(kClipBottom);
    reg32 heldY = reg32(sreg32(y) > top ? sreg32(y) : top);
    if (sreg32(heldY) >= sreg32(bottom))
        heldY = bottom;
    guest.leaveEdx(reg32(top));
    guest.leaveEcx(bottom);
    guest.leaveEbx(heldY);
    enterLock(guest, cpu, guest.dword(kMessageLock));
    guest.setDword(frame + 8, heldY);
    guest.setDword(kButtons, guest.dword(frame + 4));
    if (heldX != x || heldY != y)
        guest::direct<0x4cc890, movePointer>(app, cpu, kWindow, heldX, guest.dword(frame + 8));
    const reg32 kept = guest.dword(frame + 8);
    if (heldX != guest.dword(kLastX) || kept != guest.dword(kLastY))
    {
        guest.setDword(kLastX, heldX);
        guest.setDword(kLastY, kept);
    }
    const reg32 lock = guest.dword(kMessageLock);
    guest.leaveEbx(lock);
    cpu.eax = kept;
    callImport<kLeaveLock, &win32::kernel32::LeaveCriticalSection>(guest, cpu, lock);
    if (guest.dword(frame) != 0 && guest.dword(0x56446c) != 0)
        guest.call(guest.dword(0x56446c), args({ guest.dword(frame + 4) }));
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * Startup checks
 * ------------------------------------------------------------------------- */

/* sub_4a38c0: whether the game's window is up already. */
x86::reg32 frontend::wina::alreadyRunning(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    return callImport<0x534734, &win32::user32::FindWindowA>(guest, cpu, guest.dword(0x5612a0), 0) != 0 ? 1 : 0;
}

/* sub_4a38f0: the file version of the game's exe, its text into the buffer given; 1 when
 * it has one. */
x86::reg32 frontend::wina::versionText(App* app, CPU& cpu, reg32 buffer, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x118);
    constexpr reg32 kDefaultPath = 0x4a2f50, kVersionKey = 0x53ce2c;
    const reg32 frame = guest.frame();
    const reg32 path = frame, handle = frame + 0x100, value = frame + 0x104, valueLength = frame + 0x108;
    guest.setDword(frame + 0x10c, buffer);
    guest.setDword(frame + 0x114, length);
    copyDwords(guest, path, kDefaultPath, 0x40);
    guest.leaveEdx(0);
    guest.setDword(frame + 0x110, 0);
    reg32 size = callThunk<kVersionInfoSize, &win32::version::GetFileVersionInfoSizeA>(guest, cpu, path, handle);
    guest.leaveEdx(size);
    if (size == 0)
    {
        callImport<0x534544, &win32::kernel32::GetModuleFileNameA>(guest, cpu, size, path, 0xff);
        size = callThunk<kVersionInfoSize, &win32::version::GetFileVersionInfoSizeA>(guest, cpu, path, handle);
        guest.leaveEdx(size);
        if (size == 0)
            return 0;
    }
    const reg32 memory = guest.call(0x4f7900, args({ size }));
    if (memory == 0)
        return guest.dword(frame + 0x110);
    if (callThunk<kVersionInfo, &win32::version::GetFileVersionInfoA>(guest, cpu, path, guest.dword(handle), cpu.edx,
                                                                     memory) != 0
        && callThunk<kQueryVersionValue, &win32::version::VerQueryValueA>(guest, cpu, memory, kVersionKey, value,
                                                                         valueLength) != 0)
    {
        reg32 count = guest.dword(valueLength);
        if (count != 0)
        {
            guest.setDword(frame + 0x110, 1);
            if (count >= guest.dword(frame + 0x114))
                count = guest.dword(frame + 0x114);
            guest.call(kStrncpy, args({ guest.dword(frame + 0x10c), guest.dword(value), count }));
        }
    }
    guest.call(0x4f79f0, args({ memory }));
    return guest.dword(frame + 0x110);
}

/* sub_4b56d0: an import's answer, no more. */
x86::reg32 frontend::wina::tickCount(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    return callImport<0x534568, &win32::kernel32::GetTickCount>(guest, cpu);
}

/* sub_4b5860: DirectX 5 and a sound or graphics library there, or the game says so in
 * the player's language and leaves. */
x86::reg32 frontend::wina::checkLibrary(App* app, CPU& cpu, reg32 library, reg32 language)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x23c);
    cpu.edx = language;  // as the game's code has it here, for the reads below
    constexpr reg32 kNeedsDirectX = 0x4b5580, kNoLibrary = 0x4b5598, kLibraries = 0x55fd68, kTitle = 0x53ec28;
    const reg32 frame = guest.frame();
    const reg32 text = frame + 0x100, names = frame + 0x200, formats = frame + 0x218;
    guest.setDword(frame + 0x230, library);
    guest.leaveEbx(language);
    if (sreg32(guest::direct<0x4b6590, directXVersion>(app, cpu)) < 5)
    {
        copyDwords(guest, names, kNeedsDirectX, 6);
        const reg32 name = guest.dword(names + cpu.edx * 4);
        guest.leaveEdx(name);
        guest.call(kSprintf, args({}, { text, name }));
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, text, kTitle, 0x30);
        fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    }
    const reg32 place = guest.dword(frame + 0x230) << 2;
    guest.setDword(frame + 0x238, place);
    const reg32 module = callImport<kLoadLibrary, &win32::kernel32::LoadLibraryA>(guest, cpu, guest.dword(place + kLibraries));
    guest.setDword(frame + 0x234, module);
    if (module == 0)
    {
        copyDwords(guest, formats, kNoLibrary, 6);
        guest.call(kSprintf, args({}, { frame, guest.dword(formats + language * 4), guest.dword(guest.dword(frame + 0x238) + kLibraries) }));
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, frame, kTitle, 0x30);
        fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    }
    callImport<kFreeLibrary, &win32::kernel32::FreeLibrary>(guest, cpu, guest.dword(frame + 0x234));
    return guest.eax();
}

/* sub_4b5e50: a message of two tables, by the language, and the game leaves. */
x86::reg32 frontend::wina::fatalMessage(App* app, CPU& cpu, reg32 language)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20 + 0x30);
    const reg32 frame = guest.frame();
    copyDwords(guest, frame, 0x4b55c8, 6);
    copyDwords(guest, frame + 0x18, 0x4b55e0, 6);
    callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, guest.dword(frame + language * 4),
                                                         guest.dword(frame + 0x18 + language * 4), 0x31);
    return fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
}

/* sub_4b5f10: enough memory and disk for the game, or the player is asked. */
x86::reg32 frontend::wina::checkMemory(App* app, CPU& cpu, reg32 language)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x60);
    constexpr reg32 kStatus = 0x7a3ac0, kEnoughMemory = 0x1000000;
    const reg32 frame = guest.frame();
    guest.setDword(kStatus, 0x20);
    callImport<0x534580, &win32::kernel32::GlobalMemoryStatus>(guest, cpu, kStatus);
    const reg32 total = guest.dword(0x7a3acc) + guest.dword(0x7a3ad4);
    guest.leaveEdx(total);
    guest.call(0x42d500, args({ total >> 20 }, { 0x53fdf0, total >> 20 }));
    if (cpu.edx < kEnoughMemory)
    {
        copyDwords(guest, frame + 0x30, 0x4b5628, 6);
        copyDwords(guest, frame + 0x48, 0x4b5640, 6);
        if (callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, guest.dword(frame + 0x30 + language * 4),
                                                                 guest.dword(frame + 0x48 + language * 4), 0x31) == 2)
            fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    }
    const reg32 room = fe::direct<0x4516a0, loada::diskHasRoom>(app, cpu, 0x100000);
    if (room != 0)
        return room;
    copyDwords(guest, frame, 0x4b5658, 6);
    copyDwords(guest, frame + 0x18, 0x4b5670, 6);
    // eax the answer the box leaves, or what exit's call does.
    if (callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, guest.dword(frame + language * 4),
                                                             guest.dword(frame + 0x18 + language * 4), 0x31) == 2)
        return fe::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    return guest.eax();
}

/* sub_4b5ff0: the answer to the Direct3D device enumeration's question. */
x86::reg32 frontend::wina::deviceAnswer(App*, CPU&, reg32, reg32, reg32, reg32 question, reg32, reg32)
{
    return question == 0 || question == 1 ? 0x424d5144 : 1;
}

/* sub_4b6400: the 3D device's name and its memory from the registry; a name that says
 * so when it has none. */
x86::reg32 frontend::wina::thrashDevice(App* app, CPU& cpu, reg32 buffer)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 44);
    constexpr reg32 kNoDevice = 0x53ebc4;
    const reg32 frame = guest.frame();
    guest.setDword(frame, 0x40);
    guest.setDword(frame + 12, 4);
    if (callImport<kRegOpen, &win32::advapi32::RegOpenKeyExA>(guest, cpu, 0x80000002, 0x53fe50, 0, 0xf003f, frame + 16) == 0
        && callImport<kRegQuery, &win32::advapi32::RegQueryValueExA>(guest, cpu, guest.dword(frame + 16), 0x53fe7c, 0, 0,
                                                                     buffer, frame) == 0)
    {
        const reg32 key = guest.dword(frame + 16);
        guest.setDword(frame + 4, 4);
        if (callImport<kRegQuery, &win32::advapi32::RegQueryValueExA>(guest, cpu, key, 0x53fe8c, 0, frame + 12, frame + 8,
                                                                      frame + 4) != 0)
            return guest.eax();
        const reg32 memory = guest.dword(frame + 8);
        guest.setDword(0x55fdbc, memory);
        return memory;
    }
    guest.call(kStrncpy, args({ buffer, kNoDevice, guest.length(kNoDevice) - 5 }));
    return guest.eax();
}

/* sub_4b64c0: the 3D device's description from the registry. */
x86::reg32 frontend::wina::thrashDescription(App* app, CPU& cpu, reg32 buffer)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    const reg32 frame = guest.frame();
    guest.setDword(frame, 0x32);
    if (callImport<kRegOpen, &win32::advapi32::RegOpenKeyExA>(guest, cpu, 0x80000002, 0x53fe50, 0, 0xf003f, frame + 4) == 0)
        callImport<kRegQuery, &win32::advapi32::RegQueryValueExA>(guest, cpu, guest.dword(frame + 4), 0x53fe98, 0, 0, buffer, frame);
    return guest.eax();
}

/* sub_4b6520: the desktop's wallpaper setting from the registry. */
x86::reg32 frontend::wina::desktopSetting(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24);
    constexpr reg32 kBuffer = 0x7a3ae0;
    const reg32 frame = guest.frame();
    guest.setDword(frame, 0x100);
    guest.call(kMemset, args({ kBuffer, 0, 0x100 }));
    if (callImport<kRegOpen, &win32::advapi32::RegOpenKeyExA>(guest, cpu, 0x80000001, 0x53feb0, 0, 0xf003f, frame + 4) == 0)
        callImport<kRegQuery, &win32::advapi32::RegQueryValueExA>(guest, cpu, guest.dword(frame + 4), 0x53fef4, 0, 0, kBuffer, frame);
    return kBuffer;
}

/* sub_4b6590: what DirectX there is: the system's version, then ddraw.dll's interfaces
 * asked for one by one: 0 none, 1, 3 or 5. */
x86::reg32 frontend::wina::directXVersion(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x110);
    constexpr reg32 kDirectDraw2 = 0x56b928, kSurface3 = 0x56b938, kPlatformNt = 2;
    const reg32 frame = guest.frame();
    const reg32 version = frame, description = frame + 0x94;
    const reg32 directDraw = frame + 0x104, second = frame + 0x100, surface = frame + 0x10c, third = frame + 0x108;
    guest.setDword(directDraw, 0);
    guest.setDword(second, 0);
    guest.setDword(surface, 0);
    guest.setDword(third, 0);
    guest.setDword(version, 0x94);
    const reg32 got = callImport<0x534570, &win32::kernel32::GetVersionExA>(guest, cpu, version);
    if (got == 0)
        return 0;
    if (guest.dword(version + 0x10) == kPlatformNt && guest.dword(version + 4) < 5)
        return 0;
    const reg32 module = callImport<kLoadLibrary, &win32::kernel32::LoadLibraryA>(guest, cpu, 0x53fefc);
    if (module == 0)
        return 0;
    const reg32 create = callImport<kGetProcAddress, &win32::kernel32::GetProcAddress>(guest, cpu, module, 0x53ff08);
    const auto freeLibrary = [&](reg32 answer) {
        callImport<kFreeLibrary, &win32::kernel32::FreeLibrary>(guest, cpu, module);
        return answer;
    };
    if (create == 0)
        return freeLibrary(0);
    if (sreg32(guest.call(create, args({}, { 0, directDraw, 0 }))) < 0)
        return freeLibrary(0);
    if (sreg32(callMethod<kQueryInterface, &IDirectDraw2::QueryInterface>(guest, cpu, guest.dword(directDraw), kDirectDraw2,
                                                                          second)) < 0)
    {
        callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(directDraw));
        return freeLibrary(1);
    }
    cpu.ebx = 0x6c;
    callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(second));
    guest.call(kMemset, args({ description, 0, cpu.ebx, 0x200 }));
    const reg32 object = guest.dword(directDraw);
    guest.setDword(frame + 0xfc, cpu.ecx);
    guest.setDword(description, 0x6c);
    guest.setDword(description + 4, 1);
    if (sreg32(callMethod<kSetCooperativeLevel, &IDirectDraw2::SetCooperativeLevel>(guest, cpu, object, 0, 8)) < 0)
    {
        callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(directDraw));
        return freeLibrary(0);
    }
    if (sreg32(callMethod<kCreateSurface, &IDirectDraw2::CreateSurface>(guest, cpu, guest.dword(directDraw), description,
                                                                        surface, 0)) < 0)
    {
        callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(directDraw));
        return freeLibrary(0);
    }
    if (sreg32(callMethod<kQueryInterface, &IDirectDrawSurface::QueryInterface>(guest, cpu, guest.dword(surface), kSurface3,
                                                                                third)) < 0)
    {
        callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(directDraw));
        return freeLibrary(3);
    }
    callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(surface));
    callMethod<kRelease, &IUnknown::Release>(guest, cpu, guest.dword(directDraw));
    return freeLibrary(5);
}

/* ---------------------------------------------------------------------------
 * What is left of the graphics library, the palette and the runtime
 * ------------------------------------------------------------------------- */

/* sub_4ea665: entry `index` of a table, or 0 past its end. */
x86::reg32 frontend::wina::tableEntry(App* app, CPU& cpu, reg32 table, reg32 index, reg32 out)
{
    Guest guest(app, cpu);
    if (index < guest.dword(table + 8))
    {
        const reg32 value = guest.dword(table + index * 8 + 0x10);
        guest.setDword(out, value);
        return value;
    }
    guest.setDword(out, 0);
    return table;
}

/* sub_4f2460: a shape's hot spots, the attachment of type 0x7c in its chain; 0 when none.
 * A shape of a kind not known is said to be wrong. */
x86::reg32 frontend::wina::shapeHotspots(App* app, CPU& cpu, reg32 shape)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    constexpr reg32 kShapeKinds = 0x566d54, kHotspots = 0x7c;
    if (shape == 0)
        return 0;
    if (guest.byte((guest.dword(shape) & 0x7f) + kShapeKinds) == 0)
        assertFailed(guest, 0x54cad4, 0x54cae4, 0x31, args({ guest.dword(shape) & 0xff }, { 0x54caf8, guest.dword(shape) & 0xff }));
    for (;;)
    {
        safepoint(app, cpu);
        if (shape == 0)
            return 0;
        if ((guest.dword(shape) & 0xff) == kHotspots)
            return shape + 8;
        const sreg32 step = sreg32(guest.dword(shape)) >> 8;
        shape = step == 0 ? 0 : shape + reg32(step);
    }
}

/* sub_4f87a0: the graphics' screen lock let go and its driver slot back. */
x86::reg32 frontend::wina::releaseDriverSlot(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    const reg32 module = guest.dword(0x564a6c), slot = guest.dword(0x564a68);
    guest.leaveEdx(module);
    guest.leaveEbx(slot);
    if (module != 0)
    {
        const reg32 ecx = cpu.ecx;
        cpu.esp -= 4;
        callImport<kFreeLibrary, &win32::kernel32::FreeLibrary>(guest, cpu, module);
        cpu.esp += 4;
        guest.leaveEcx(ecx);
    }
    cpu.esi = 0;
    guest.setDword(0x564a6c, 0);
    guest.call(0x517170, args({ 0 }));
    guest.call(0x517370, args({ slot }));
    return guest.eax();
}

/* sub_4f9120: whether the game has the focus and a window; else sub_4f9088 says. */
x86::reg32 frontend::wina::hasFocus(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (guest.dword(kActive) != 0 && guest.dword(kHwnd) != 0)
        return 1;
    return fe::direct<0x4f9088, eab::lineNoFlag>(app, cpu);
}

/* sub_4f9360: the video buffers given back. */
x86::reg32 frontend::wina::videoClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    constexpr reg32 kOpen = 0x567194, kSurface = 0x567190;
    if (guest.dword(kOpen) == 0)
        return guest.eax();
    guest.leaveEcx(0);
    guest::direct<0x51a170, freeSurface>(app, cpu, guest.dword(kSurface));
    guest.setDword(kSurface, cpu.ecx);
    const reg32 left = guest::direct<0x519140, releaseVideoBuffers>(app, cpu);
    guest.setDword(kOpen, cpu.ecx);
    return left;
}

/* sub_4fb060: a block of memory the race kept, let go. */
x86::reg32 frontend::wina::freeKeptBlock(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 block = guest.dword(0x5650c0);
    if (block == 0)
        return guest.eax();
    guest.leaveEcx(0);
    guest.leaveEdx(block);
    const reg32 left = reg32(*fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block));
    guest.setDword(0x5650c0, cpu.ecx);
    return left;
}

/* sub_4fd490: a row filled, its length halved. */
x86::reg32 frontend::wina::fillHalved(App* app, CPU& cpu, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    guest.call(kFillDword, args({ to, reg32(sreg32(count) >> 1), guest.dword(kFillColour) }));
    return guest.eax();
}

/* sub_4fd4b0: a row filled, the count as it is. */
x86::reg32 frontend::wina::fillSame(App* app, CPU& cpu, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    guest.call(kFillDword, args({ to, count, guest.dword(kFillColour) }));
    return guest.eax();
}

/* sub_4fd4c0: a row filled, the count doubled. */
x86::reg32 frontend::wina::fillDoubled(App* app, CPU& cpu, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    guest.call(kFillDword, args({ to, count + count, guest.dword(kFillColour) }));
    return guest.eax();
}

/* sub_4fd520: a row filled, the count times four. */
x86::reg32 frontend::wina::fillQuadrupled(App* app, CPU& cpu, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    guest.call(kFillDword, args({ to, count << 2, guest.dword(kFillColour) }));
    return guest.eax();
}

/* sub_4fd4e0: 16-bit pixels of the fill colour. */
x86::reg32 frontend::wina::fillWords(App* app, CPU& cpu, reg32 upper, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    reg32 at = to, left = count, colour = 0;
    do
    {
        safepoint(app, cpu);
        at += 2;
        colour = guest.word(kFillColour);
        guest.setWord(at - 2, std::uint16_t(colour));
        --left;
    } while (left != 0);
    guest.leaveEdx(at);
    guest.leaveEbx(left);
    return (upper & 0xffff0000u) | colour;
}

/* sub_4fd500: 24-bit pixels of the fill colour. */
x86::reg32 frontend::wina::fillTriples(App* app, CPU& cpu, reg32 to, reg32 count)
{
    Guest guest(app, cpu);
    reg32 at = to, left = count, high = 0;
    do
    {
        safepoint(app, cpu);
        guest.setWord(at, guest.word(kFillColour));
        high = guest.dword(kFillColour) >> 16;
        at += 3;
        guest.setByte(at - 1, std::uint8_t(high));
        --left;
    } while (left != 0);
    guest.leaveEdx(at);
    guest.leaveEbx(left);
    return high;
}

/* sub_503b00: an int shifted left into 64 bits at esi. */
x86::reg32 frontend::wina::shiftTo64(App* app, CPU& cpu, reg32 value, reg32 by, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    reg32 low, high;
    if (sreg32(by) >= 0x20)
    {
        guest.leaveEdx(by - 0x20);
        low = 0;
        high = value << ((by - 0x20) & 31);
    }
    else if (by == 0)
    {
        low = value;
        high = reg32(sreg32(value) >> 31);
    }
    else
    {
        low = value << (by & 31);
        high = reg32(sreg32(value) >> ((0x20 - by) & 31));
    }
    guest.setDword(out, low);
    guest.setDword(out + 4, high);
    guest.setEsi(guest.frame() + 8);
    return out;
}

/* sub_503cd0: palette entries given to the window's surface, read back unless it is
 * in fullscreen. */
x86::reg32 frontend::wina::paletteToSurface(App* app, CPU& cpu, reg32 surface, reg32 first, reg32 count)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 144);
    constexpr reg32 kColours = 0x30, kLockDepth = 0x43c, kLowest = 0x430, kHighest = 0x432;
    const reg32 frame = guest.frame();
    const reg32 window = surface != 0 ? surface : kWindow;
    guest.setDword(frame + 0x7c, first);
    guest.setDword(0x5643e4, guest.dword(0x5643e4) + 1);
    if (guest.dword(0x564364) != 7 || sreg32(guest.dword(window + 0x1c)) > 8)
        return surface;
    const reg32 last = first + count - 1;
    const reg32 locked = guest.dword(window + kLockDepth) != 0 ? 1 : 0;
    guest.setDword(frame + 0x78, last);
    guest.setDword(frame + 0x70, locked);
    reg32 in = first * 4 + kPalette, out = window + kColours + first * 4;
    reg32 leftInEbx = out;
    for (sreg32 i = 0; i < sreg32(count); ++i)
    {
        safepoint(app, cpu);
        leftInEbx = (reg32(guest.byte(in)) << 16) | (reg32(guest.byte(in + 1)) << 8) | guest.byte(in + 2);
        guest.setDword(out, leftInEbx);
        in += 4;
        out += 4;
    }
    if ((sreg32(guest.dword(window + 0x42e)) >> 16) > sreg32(first))
        guest.setWord(window + kLowest, std::uint16_t(first));
    if ((sreg32(guest.dword(window + kLowest)) >> 16) < sreg32(last))
        guest.setWord(window + kHighest, std::uint16_t(last));
    guest.leaveEdx(first);
    guest.leaveEbx(leftInEbx);
    if (locked != 0)
    {
        fe::direct<0x4f46d0, wata::windowSave>(app, cpu, frame);
        fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    }
    reg32 left = fe::direct<0x4ecf60, eab::ddrawApplyPalette>(app, cpu, window);
    if (guest.byte(window + 0x45d) == 0)
    {
        reg32 to = first * 4 + kPalette, from = window + kColours + first * 4;
        guest.leaveEdx(from);
        for (sreg32 i = 0; i < sreg32(count); ++i)
        {
            safepoint(app, cpu);
            guest.setByte(to + 2, guest.byte(from));
            guest.setByte(to + 1, std::uint8_t(guest.dword(from) >> 8));
            const reg32 red = (guest.dword(from) >> 16) & 0xff;
            guest.leaveEbx(red);
            guest.setByte(to, std::uint8_t(red));
            to += 4;
            from += 4;
        }
        guest.leaveEdx(from);
        left = to;
    }
    if (guest.dword(frame + 0x70) != 0)
    {
        return fe::direct<0x4f4700, wata::windowRestore>(app, cpu, frame);
    }
    return left;
}

/* sub_503e50: palette entries set from bytes red green blue. */
x86::reg32 frontend::wina::paletteSet(App* app, CPU& cpu, reg32 first, reg32 count, reg32 from)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    fe::direct<0x50425d, rendf::rowRgb24ToArgb>(app, cpu, from, first * 4 + kPalette, count);
    return guest::direct<0x503cd0, paletteToSurface>(app, cpu, kWindow, first, count);
}

/* sub_503ec0: palette entries as bytes red green blue. */
x86::reg32 frontend::wina::paletteGet(App* app, CPU& cpu, reg32 first, reg32 count, reg32 to)
{
    Guest guest(app, cpu);
    reg32 from = first * 4 + kPalette;
    guest.leaveEdx(0);
    if (sreg32(count) <= 0)
        return first << 2;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        safepoint(app, cpu);
        guest.setByte(to, guest.byte(from + 2));
        guest.setByte(to + 1, guest.byte(from + 1));
        guest.setByte(to + 2, guest.byte(from));
        from += 4;
        to += 3;
    }
    guest.leaveEdx(count);
    guest.leaveEbx(to);
    return from;
}

/* sub_50e800: the queue's first entry of a priority taken off it, 0 when it has none. */
x86::reg32 frontend::wina::queueTakeByPriority(App* app, CPU& cpu, reg32 queue, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 20);
    constexpr reg32 kSource = 0x54fbb8, kFunction = 0x54fd58;
    constexpr reg32 kFirstEntry = 8, kPriorityOf = 0x10, kPriorityArg = 0x14, kQueueLock = 0x18;
    guest.leaveEdx(guest.dword(queue + kQueueLock));
    enterLock(guest, cpu, guest.dword(queue + kQueueLock));
    if (guest.dword(queue + kQueueLock) == 0)
        assertFailed(guest, kSource, kFunction, 0x3ac, args({ 0x3ac }, { 0x54fbcc, kFunction, queue }));
    reg32 entry = guest.dword(queue + kFirstEntry);
    while (entry != 0)
    {
        safepoint(app, cpu);
        if (guest.call(guest.dword(queue + kPriorityOf), args({ entry, guest.dword(queue + kPriorityArg) })) >= wanted)
            break;
        entry = guest.dword(entry);
    }
    reg32 taken = 0;
    if (entry != 0 && guest.call(guest.dword(queue + kPriorityOf), args({ entry, guest.dword(queue + kPriorityArg) })) == wanted)
    {
        if (guest.call(0x50dc20, args({ queue, entry })) != 0)
        {
            taken = entry;
        }
        else
        {
            guest.leaveEdx(kSource);
            assertFailed(guest, kSource, kFunction, 0x3b5, args({}, { 0x54fd68 }));
        }
    }
    leaveLock(guest, cpu, guest.dword(queue + kQueueLock));
    return taken;
}

/* sub_517580: the graphics closed when they are open. */
x86::reg32 frontend::wina::graphicsClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    if (guest.dword(0x564364) == 0)
        return guest.eax();
    guest::direct<0x517920, graphicsPalette>(app, cpu, 0);
    guest.leaveEcx(0);
    guest.call(kCloseDriver, args({ 1 }));
    guest.setDword(0x564364, cpu.ecx);
    guest.setDword(0x564418, cpu.ecx);
    return guest.eax();
}

/* sub_5175b0: the system's request to close, from whichever thread. */
x86::reg32 frontend::wina::graphicsAskClose(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    if (*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0))
    {
        guest.leaveEbx(0);
        guest::direct<0x517580, graphicsClose>(app, cpu);
        return 0;
    }
    const reg32 lock = guest.dword(kGraphicsLock);
    guest.leaveEdx(lock);
    if (lock == 0)
        return 0;
    enterLock(guest, cpu, lock);
    const reg32 hwnd = guest.dword(kHwnd);
    const reg32 answer = guest.dword(kGraphicsLock);
    if (hwnd != 0)
        callImport<0x534798, &win32::user32::ShowWindow>(guest, cpu, hwnd, 6);
    return answer;
}

/* sub_517920: the screen's palette, or a shape's, given to the driver. */
x86::reg32 frontend::wina::graphicsPalette(App* app, CPU& cpu, reg32 surface)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    guest.leaveEbx(surface);
    if (guest.dword(0xa18220) != 0)
        guest.call(0x5178d0, args({ surface }));
    if (guest.dword(0x9f3d0c) != 0)
        fe::direct<0x4ed2a0, eab::ddrawUnlockBitmap>(app, cpu, kWindow);
    if (surface != 0)
    {
        const std::uint8_t mode = guest.byte(surface + 0x1e);
        if (mode != 0 && mode != 0xff)
        {
            guest.call(0x517890, args({ surface, mode }));
            return guest.eax();
        }
        const reg32 palette = guest.dword(surface + 0x34);
        if (palette != 0)
        {
            // The palette in edx, where the call takes the surface; ebx as the game's code has it (the surface).
            return fe::direct<0x4ed0f0, eab::ddrawLockBitmap>(app, cpu, kWindow, palette, cpu.ebx);
        }
    }
    return 1;
}

/* sub_519140: the video's two buffers given back through the driver's table. */
x86::reg32 frontend::wina::releaseVideoBuffers(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 16);
    constexpr reg32 kDriver = 0x56718c, kFreeBuffer = 0x18, kFirst = 0x569058, kSecond = 0x56905c;
    const reg32 first = guest.dword(kFirst);
    if (first != 0)
    {
        const reg32 table = guest.dword(kDriver);
        guest.leaveEdx(table);
        guest.leaveEcx(0);
        guest.call(guest.dword(table + kFreeBuffer), args({ first }));
        guest.setDword(kFirst, cpu.ecx);
    }
    const reg32 second = guest.dword(kSecond);
    guest.leaveEbx(second);
    if (second != 0)
    {
        const reg32 esi = cpu.esi;
        cpu.esp -= 4;
        const reg32 table = guest.dword(kDriver);
        guest.leaveEdx(table);
        cpu.esi = 0;
        guest.call(guest.dword(table + kFreeBuffer), args({ second }));
        guest.setDword(kSecond, cpu.esi);
        cpu.esp += 4;
        cpu.esi = esi;
    }
    guest.setDword(0x569054, 0);
    guest.setDword(0x569060, 0);
    return guest.eax();
}

/* sub_51a170: a surface given back through the driver's table. */
x86::reg32 frontend::wina::freeSurface(App* app, CPU& cpu, reg32 surface)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    guest.leaveEdx(surface);
    guest::direct<0x525600, freeSurfaceCache>(app, cpu, surface);
    const reg32 table = guest.dword(0x56718c);
    guest.leaveEcx(table);
    guest.call(guest.dword(table + 0x18), args({ cpu.edx }));
    return guest.eax();
}

/* sub_5244f0: the system's tasks, the idle task of the graphics. */
x86::reg32 frontend::wina::graphicsIdle(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0));
}

/* sub_524500: the surface's interface asked for, or the object let go. */
x86::reg32 frontend::wina::setDirectDraw(App* app, CPU& cpu, reg32 object)
{
    Guest guest(app, cpu);
    constexpr reg32 kUses = 0x9f3d3c;
    guest.leaveEdx(object);
    if (object != 0)
    {
        cpu.eax = guest.dword(object);
        const reg32 answer = callMethod<kQueryInterface, &IDirectDraw2::QueryInterface>(guest, cpu, object, 0x56b928, kWindow);
        if (answer == 0)
            guest.setDword(kUses, guest.dword(kUses) + 1);
        return answer;
    }
    const reg32 own = guest.dword(kWindow);
    const reg32 table = guest.dword(own);
    guest.leaveEdx(table);
    cpu.eax = own;
    callMethod<kRelease, &IUnknown::Release>(guest, cpu, own);
    const reg32 uses = guest.dword(kUses) + 1;
    guest.leaveEcx(uses);
    guest.leaveEdx(0);
    guest.setDword(kWindow, 0);
    guest.setDword(kUses, uses);
    return guest.eax();
}

/* sub_524550: the graphics' lock taken or let go, as the argument says. */
x86::reg32 frontend::wina::graphicsLock(App* app, CPU& cpu, reg32 taking)
{
    Guest guest(app, cpu);
    if (taking != 0)
    {
        cpu.esp -= 4;
        enterLock(guest, cpu, guest.dword(kGraphicsLock));
        cpu.esp += 4;
    }
    else
    {
        const reg32 lock = guest.dword(kGraphicsLock);
        guest.leaveEcx(lock);
        leaveLock(guest, cpu, lock);
    }
    guest.setDword(0x567180, taking);
    return taking;
}

/* sub_5245a0: a message of the driver's: an error, on the monochrome screen, in the log,
 * or to the debugger, by its level. */
x86::reg32 frontend::wina::driverMessage(App* app, CPU& cpu, reg32 level, reg32 text)
{
    Guest guest(app, cpu);
    guest.leaveEdx(level);
    switch (level)
    {
    case 0:
        guest.call(kError, args({ text }, { text }));
        break;
    case 1:
        guest.call(0x4e08f0, args({ text }, { text }));
        break;
    case 2:
        guest.call(0x4df6c0, args({ text }, { text }));
        break;
    case 3:
        cpu.eax = text;
        callImport<0x53459c, &win32::kernel32::OutputDebugStringA>(guest, cpu, text);
        break;
    default:
        return text;
    }
    return guest.eax();
}

/* sub_5245f0: the driver's event: the resize callback told on the second kind. */
x86::reg32 frontend::wina::driverEvent(App* app, CPU& cpu, reg32 kind, reg32)
{
    Guest guest(app, cpu);
    if (kind == 2 && guest.dword(kOnResize) != 0)
    {
        guest.call(guest.dword(kOnResize), args({ kind }));
        return guest.eax();
    }
    return kind;
}

/* sub_524620: the driver closed without its window, for an error. */
x86::reg32 frontend::wina::closeDriverOnly(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    guest.call(kCloseDriver, args({ 1, 0 }));
    guest.setDword(0x564418, cpu.edx);
    return guest.eax();
}

/* sub_5247c0: the screen's size and depth, with the defaults; the graphics started when
 * they are not. */
x86::reg32 frontend::wina::setScreenSize(App* app, CPU& cpu, reg32 widthIn, reg32 heightIn, reg32 depthIn)
{
    Guest guest(app, cpu);
    const reg32 width = widthIn != 0 ? widthIn : 0x280;
    const reg32 height = heightIn != 0 ? heightIn : 0x1e0;
    const reg32 depth = depthIn != 0 ? depthIn : 0x10;
    guest.setDword(0x56438c, depth);
    const reg32 originX = guest.dword(0x56439c);
    guest.setDword(0x564384, width);
    guest.setDword(0x5643a4, width - originX);
    const reg32 originY = guest.dword(0x5643a0);
    guest.setDword(0x564388, height);
    const reg32 room = height - originY;
    guest.setDword(0x5643a8, room);
    guest.leaveEdx(room);
    guest.leaveEbx(originX);
    if (guest.dword(0x564364) != 0)
        return originY;
    // originY, room and originX, which the game's code hands it in eax, edx and ebx, are not read.
    return fe::direct<0x509940, watb::initRowTable>(app, cpu);
}

/* sub_525600: what a surface keeps on the side, given back. */
x86::reg32 frontend::wina::freeSurfaceCache(App* app, CPU& cpu, reg32 surface)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    guest.leaveEdx(surface);
    if (guest.dword(surface + 0x54) == 0)
        return surface;
    cpu.esp -= 4;
    const reg32 table = guest.dword(0x56718c);
    guest.leaveEcx(table);
    const reg32 answer = guest.call(guest.dword(table + 8), args({ guest.dword(surface + 0x54) }));
    cpu.esp += 4;
    guest.setDword(surface + 0x54, 0);
    guest.setByte(surface + 0xbc, 1);
    return answer;
}

/* sub_525630: the next byte of a text, its pointer moved. */
x86::reg32 frontend::wina::nextByte(App* app, CPU& cpu, reg32 place)
{
    Guest guest(app, cpu);
    const reg32 at = guest.dword(place);
    const reg32 character = guest.byte(at);
    guest.setDword(place, at + 1);
    return character;
}

/* sub_525650: a character through the table of the printable ones. */
x86::reg32 frontend::wina::mapCharacter(App* app, CPU& cpu, reg32 character)
{
    Guest guest(app, cpu);
    if (sreg32(character) < 0x20 || sreg32(character) > 0x7f)
        return character;
    return guest.word(character * 2 + 0x56afb8);
}

/* sub_525670: the next character of a text: two bytes with the top bit set, else one
 * through the table. */
x86::reg32 frontend::wina::nextCharacter(App* app, CPU& cpu, reg32 place)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    reg32 at = guest.dword(place);
    const reg32 first = guest.byte(at);
    ++at;
    if ((first & 0x80) != 0)
    {
        const reg32 second = guest.byte(at);
        guest.setDword(place, at + 1);
        return (first << 8) | second;
    }
    const reg32 mapped = guest::direct<0x525650, mapCharacter>(app, cpu, first);
    guest.setDword(place, at);
    return mapped & 0xffff;
}

/* sub_52e420: the text driver started with the screen's saved two values. */
x86::reg32 frontend::wina::textStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 12);
    return fe::direct<0x52ee00, watc::setClip>(app, cpu, 0, 0, guest.dword(0x564ff8), guest.dword(0x564ffc));
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

void frontend::winaStreamCheck(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf1c0", 0, 0, nullptr, 0, kCx };
    const reg32 handle = cpu.eax, streamOut = cpu.edx, handleOut = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamCheck(app, cpu, handle, streamOut, handleOut); });
}

void frontend::winaRingBetween(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf1f0", 0, 0, nullptr, 0, 0 };
    const reg32 from = cpu.eax, to = cpu.edx, place = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::ringBetween(app, cpu, from, to, place); });
}

void frontend::winaStreamConsume(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf220", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 s = cpu.eax, amount = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamConsume(app, cpu, s, amount); });
}

void frontend::winaStreamTakeFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf280", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 s = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamTakeFree(app, cpu, s); });
}

void frontend::winaStreamQueueFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf2f0", 0, 0, nullptr, 0, kBx | kCx | kSi };
    const reg32 s = cpu.eax, record = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamQueueFile(app, cpu, s, record); });
}

void frontend::winaStreamFindFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf340", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 s = cpu.eax, id = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFindFile(app, cpu, s, id); });
}

void frontend::winaStreamUnlinkFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf380", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 s = cpu.eax, record = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamUnlinkFile(app, cpu, s, record); });
}

void frontend::winaStreamClassify(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf3f0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 s = cpu.eax, chunk = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamClassify(app, cpu, s, chunk); });
}

void frontend::winaStreamParse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf440", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 s = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamParse(app, cpu, s); });
}

void frontend::winaStreamFileOpened(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf570", 0, 0, nullptr, 0, 0 };
    const reg32 s = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFileOpened(app, cpu, s); });
}

void frontend::winaStreamFileSwitched(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf5a0", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 s = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFileSwitched(app, cpu, s); });
}

void frontend::winaStreamFilled(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf5e0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 s = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFilled(app, cpu, s); });
}

void frontend::winaStreamNextFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf6b0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 s = cpu.eax, priority = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamNextFile(app, cpu, s, priority); });
}

void frontend::winaStreamReadMore(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf810", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 s = cpu.eax, priority = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamReadMore(app, cpu, s, priority); });
}

void frontend::winaStreamHeaderSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfa40", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 fileCount = cpu.eax, ruleCount = cpu.edx, channelCount = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::streamHeaderSize(app, cpu, fileCount, ruleCount, channelCount);
    });
}

void frontend::winaStreamCreate(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfa80", 4, 0, nullptr, 0, kSi | kDi };
    const reg32 files = cpu.eax, rules = cpu.edx, channels = cpu.ebx, s = cpu.ecx;
    const reg32 memory = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::streamCreate(app, cpu, files, rules, channels, s, memory);
    });
}

void frontend::winaStreamSetRule(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfca0", 4, 0, nullptr, 0, kSi | kDi };
    const reg32 handle = cpu.eax, number = cpu.edx, mask = cpu.ebx, value = cpu.ecx;
    const reg32 target = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::streamSetRule(app, cpu, handle, number, mask, value, target);
    });
}

void frontend::winaStreamClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfd30", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamClose(app, cpu, handle); });
}

void frontend::winaStreamSetLowWater(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfdd0", 0, 0, nullptr, 0, kBx | kCx | kSi };
    const reg32 handle = cpu.eax, low = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamSetLowWater(app, cpu, handle, low); });
}

void frontend::winaStreamSetBelow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfe30", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 handle = cpu.eax, below = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamSetBelow(app, cpu, handle, below); });
}

void frontend::winaStreamChannel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfe70", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 handle = cpu.eax, number = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamChannel(app, cpu, handle, number); });
}

void frontend::winaStreamOpenFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cfef0", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 handle = cpu.eax, name = cpu.edx, length = cpu.ebx, endId = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamOpenFile(app, cpu, handle, name, length, endId); });
}

void frontend::winaStreamOpenMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cffb0", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 handle = cpu.eax, data = cpu.edx, length = cpu.ebx, endId = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamOpenMemory(app, cpu, handle, data, length, endId); });
}

void frontend::winaStreamCloseFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0060", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 handle = cpu.eax, id = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamCloseFile(app, cpu, handle, id); });
}

void frontend::winaStreamFlush(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0220", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFlush(app, cpu, handle); });
}

void frontend::winaStreamNextChunk(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0300", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamNextChunk(app, cpu, handle); });
}

void frontend::winaStreamFreeChunk(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d03c0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 handle = cpu.eax, chunk = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::streamFreeChunk(app, cpu, handle, chunk); });
}

void frontend::winaChannelBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0460", 0, 0, nullptr, 0, kBx | kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::channelBytes(app, cpu, handle); });
}

void frontend::winaChannelState(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0490", 0, 0, nullptr, 0, kBx | kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::channelState(app, cpu, handle); });
}

void frontend::winaChannelDone(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d04c0", 0, 0, nullptr, 0, kBx | kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::channelDone(app, cpu, handle); });
}

void frontend::winaChannelRingSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d0500", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::channelRingSize(app, cpu, handle); });
}

void frontend::winaMovieNextFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_495b60", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 reader = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::movieNextFrame(app, cpu, reader); });
}

void frontend::winaFileOpAllocate(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fb990", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 kind = cpu.eax, priorityIn = cpu.edx, target = cpu.ebx, slot = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::fileOpAllocate(app, cpu, kind, priorityIn, target, slot);
    });
}

void frontend::winaFileOpQueue(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fbb60", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 op = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpQueue(app, cpu, op); });
}

void frontend::winaFileOpSetName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fbbe0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 op = cpu.eax, nameIn = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpSetName(app, cpu, op, nameIn); });
}

void frontend::winaFileOpCallback(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fc8e0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 id = cpu.eax, callback = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpCallback(app, cpu, id, callback); });
}

void frontend::winaFileOpPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fc9b0", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 id = cpu.eax, priority = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpPriority(app, cpu, id, priority); });
}

void frontend::winaFileOpOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fca60", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 name = cpu.eax, mode = cpu.edx, priority = cpu.ebx, target = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpOpen(app, cpu, name, mode, priority, target); });
}

void frontend::winaFileOpClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fca90", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 handle = cpu.eax, priority = cpu.edx, target = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileOpClose(app, cpu, handle, priority, target); });
}

void frontend::winaFileOpRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fcab0", 8, 0, nullptr, 0, kSi | kDi };
    const reg32 handle = cpu.eax, offset = cpu.edx, buffer = cpu.ebx, size = cpu.ecx;
    const reg32 priority = app->getMemory<reg32>(cpu.esp + 4), target = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::fileOpRead(app, cpu, handle, offset, buffer, size, priority, target);
    });
}

void frontend::winaFileSlotOfName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fce50", 0, 0, nullptr, 0, kCx | kDx | kSi };
    const reg32 name = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileSlotOfName(app, cpu, name); });
}

void frontend::winaFileCloseWait(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd060", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 handle = cpu.eax, priority = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileCloseWait(app, cpu, handle, priority); });
}

void frontend::winaFileSizeAndSeek(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e4230", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 handle = cpu.eax, offset = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileSizeAndSeek(app, cpu, handle, offset); });
}

void frontend::winaFileLockOf(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4e4320", 0, 0, nullptr, 0, kDx };
    const reg32 handle = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fileLockOf(app, cpu, handle); });
}

void frontend::winaCompareHandlers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc650", 0, 0, nullptr, 0, 0 };
    const reg32 leftRow = cpu.eax, rightRow = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::compareHandlers(app, cpu, leftRow, rightRow); });
}

void frontend::winaHandlerOf(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc660", 4, 0, nullptr, 0, kBx | kSi };
    const reg32 message = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::handlerOf(app, cpu, message); });
}

void frontend::winaHandlerSet(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc6d0", 8, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 message = app->getMemory<reg32>(cpu.esp + 4), handler = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::handlerSet(app, cpu, message, handler); });
}

void frontend::winaPostMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc7c0", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 message = cpu.eax, lParam = cpu.edx, wParam = cpu.ebx, send = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::postMessage(app, cpu, message, lParam, wParam, send); });
}

void frontend::winaSetFocusCallback(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc860", 0, 0, nullptr, 0, 0 };
    const reg32 callback = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::setFocusCallback(app, cpu, callback); });
}

void frontend::winaNoWindowTask(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc870", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::noWindowTask(app, cpu); });
}

void frontend::winaMovePointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc890", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 window = cpu.eax, x = cpu.edx, y = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::movePointer(app, cpu, window, x, y); });
}

void frontend::winaPushKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cc8e0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 key = cpu.eax, scan = cpu.edx, flags = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::pushKey(app, cpu, key, scan, flags); });
}

void frontend::winaKeyOfMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccaa0", 0, 0, nullptr, 0, 0 };
    const reg32 message = cpu.eax, virtualKey = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::keyOfMessage(app, cpu, message, virtualKey); });
}

void frontend::winaKeyboardHook(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccc70", 12, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 code = app->getMemory<reg32>(cpu.esp + 4), wParam = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 12);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::keyboardHook(app, cpu, code, wParam, lParam); });
}

void frontend::winaOnSetFocus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccd60", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onSetFocus(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnKillFocus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccdd0", 0x18, 0, nullptr, 0, kBx | kSi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onKillFocus(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cce40", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onClose(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnQuitAsked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cce60", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onQuitAsked(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnDestroy(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cce80", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onDestroy(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnSysCommand(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cce90", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onSysCommand(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnSettingChange(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cced0", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onSettingChange(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccf50", 0x18, 0, nullptr, 0, kBx };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onMoved(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnChar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ccfc0", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onChar(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnKeyDown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd020", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onKeyDown(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnKeyUp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd0e0", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onKeyUp(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnMouseDown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd130", 0x18, 0, nullptr, 0, kBx | kSi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onMouseDown(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnMouseUp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd190", 0x18, 0, nullptr, 0, kBx | kSi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onMouseUp(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnPaint(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd1f0", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onPaint(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaOnEraseBackground(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd2a0", 0x18, 0, nullptr, 0, 0 };
    const reg32 window = app->getMemory<reg32>(cpu.esp + 4), hwnd = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 message = app->getMemory<reg32>(cpu.esp + 12), wParam = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 lParam = app->getMemory<reg32>(cpu.esp + 20), result = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::onEraseBackground(app, cpu, window, hwnd, message, wParam, lParam, result);
    });
}

void frontend::winaWindowProc(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd2d0", 0x10, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 hwnd = app->getMemory<reg32>(cpu.esp + 4), message = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 wParam = app->getMemory<reg32>(cpu.esp + 12), lParam = app->getMemory<reg32>(cpu.esp + 16);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::windowProc(app, cpu, hwnd, message, wParam, lParam); });
}

void frontend::winaCreateWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd350", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 window = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::createWindow(app, cpu, window); });
}

void frontend::winaWindowThread(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd4e0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::windowThread(app, cpu); });
}

void frontend::winaOpenWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cd6c0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 width = cpu.eax, height = cpu.edx, isFullscreen = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::openWindow(app, cpu, width, height, isFullscreen); });
}

void frontend::winaCloseWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cdaa0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::closeWindow(app, cpu); });
}

void frontend::winaPlaceWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cdb10", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 left = cpu.eax, top = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::placeWindow(app, cpu, left, top, width, height); });
}

void frontend::winaCentreWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cdbd0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 width = cpu.eax, height = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::centreWindow(app, cpu, width, height); });
}

void frontend::winaHookOff(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cdc50", 0, 0, nullptr, 0, kBx | kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::hookOff(app, cpu); });
}

void frontend::winaHookOn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cdc90", 0, 0, nullptr, 0, kCx };
    const reg32 alt = cpu.eax, callback = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::hookOn(app, cpu, alt, callback); });
}

void frontend::winaPointerMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fb470", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 x = cpu.eax, y = cpu.edx, buttons = cpu.ebx, click = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::pointerMoved(app, cpu, x, y, buttons, click); });
}

void frontend::winaAlreadyRunning(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a38c0", 0, 0, nullptr, 0, kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::alreadyRunning(app, cpu); });
}

void frontend::winaVersionText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a38f0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 buffer = cpu.eax, length = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::versionText(app, cpu, buffer, length); });
}

void frontend::winaTickCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b56d0", 0, 0, nullptr, 0, kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::tickCount(app, cpu); });
}

void frontend::winaCheckLibrary(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5860", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 library = cpu.eax, language = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::checkLibrary(app, cpu, library, language); });
}

void frontend::winaFatalMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5e50", 0, 0, nullptr, 0, kCx | kDx | kSi | kDi };
    const reg32 language = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fatalMessage(app, cpu, language); });
}

void frontend::winaCheckMemory(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5f10", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 language = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::checkMemory(app, cpu, language); });
}

void frontend::winaDeviceAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5ff0", 0x18, 0, nullptr, 0, 0 };
    const reg32 word0 = app->getMemory<reg32>(cpu.esp + 4), word1 = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 word2 = app->getMemory<reg32>(cpu.esp + 12), question = app->getMemory<reg32>(cpu.esp + 16);
    const reg32 word4 = app->getMemory<reg32>(cpu.esp + 20), word5 = app->getMemory<reg32>(cpu.esp + 24);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wina::deviceAnswer(app, cpu, word0, word1, word2, question, word4, word5);
    });
}

void frontend::winaThrashDevice(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6400", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const reg32 buffer = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::thrashDevice(app, cpu, buffer); });
}

void frontend::winaThrashDescription(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b64c0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 buffer = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::thrashDescription(app, cpu, buffer); });
}

void frontend::winaDesktopSetting(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6520", 0, 0, nullptr, 0, kBx | kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::desktopSetting(app, cpu); });
}

void frontend::winaDirectXVersion(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6590", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::directXVersion(app, cpu); });
}

void frontend::winaTableEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea665", 0, 0, nullptr, 0, 0 };
    const reg32 table = cpu.eax, index = cpu.edx, out = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::tableEntry(app, cpu, table, index, out); });
}

void frontend::winaShapeHotspots(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f2460", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 shape = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::shapeHotspots(app, cpu, shape); });
}

void frontend::winaReleaseDriverSlot(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f87a0", 0, 0, nullptr, 0, kBx | kDx | kSi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::releaseDriverSlot(app, cpu); });
}

void frontend::winaHasFocus(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f9120", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::hasFocus(app, cpu); });
}

void frontend::winaVideoClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4f9360", 0, 0, nullptr, 0, kCx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::videoClose(app, cpu); });
}

void frontend::winaFreeKeptBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fb060", 0, 0, nullptr, 0, kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::freeKeptBlock(app, cpu); });
}

void frontend::winaFillHalved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd490", 0, 0, nullptr, 0, 0 };
    const reg32 to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillHalved(app, cpu, to, count); });
}

void frontend::winaFillSame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd4b0", 0, 0, nullptr, 0, 0 };
    const reg32 to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillSame(app, cpu, to, count); });
}

void frontend::winaFillDoubled(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd4c0", 0, 0, nullptr, 0, 0 };
    const reg32 to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillDoubled(app, cpu, to, count); });
}

void frontend::winaFillQuadrupled(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd520", 0, 0, nullptr, 0, 0 };
    const reg32 to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillQuadrupled(app, cpu, to, count); });
}

void frontend::winaFillWords(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd4e0", 0, 0, nullptr, 0, 0 };
    const reg32 upper = cpu.eax, to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillWords(app, cpu, upper, to, count); });
}

void frontend::winaFillTriples(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4fd500", 0, 0, nullptr, 0, 0 };
    const reg32 to = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::fillTriples(app, cpu, to, count); });
}

void frontend::winaShiftTo64(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_503b00", 0, 0, nullptr, 0, kBx | kCx | kDi };
    const reg32 value = cpu.eax, by = cpu.edx, out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::shiftTo64(app, cpu, value, by, out); });
}

void frontend::winaPaletteToSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_503cd0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 surface = cpu.eax, first = cpu.edx, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::paletteToSurface(app, cpu, surface, first, count); });
}

void frontend::winaPaletteSet(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_503e50", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 first = cpu.eax, count = cpu.edx, from = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::paletteSet(app, cpu, first, count, from); });
}

void frontend::winaPaletteGet(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_503ec0", 0, 0, nullptr, 0, kCx | kSi };
    const reg32 first = cpu.eax, count = cpu.edx, to = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::paletteGet(app, cpu, first, count, to); });
}

void frontend::winaQueueTakeByPriority(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_50e800", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 queue = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::queueTakeByPriority(app, cpu, queue, wanted); });
}

void frontend::winaGraphicsClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_517580", 0, 0, nullptr, 0, kCx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::graphicsClose(app, cpu); });
}

void frontend::winaGraphicsAskClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_5175b0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::graphicsAskClose(app, cpu); });
}

void frontend::winaGraphicsPalette(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_517920", 0, 0, nullptr, 0, kBx | kDx | kSi };
    const reg32 surface = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::graphicsPalette(app, cpu, surface); });
}

void frontend::winaReleaseVideoBuffers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_519140", 0, 0, nullptr, 0, kBx | kCx | kDx | kDi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::releaseVideoBuffers(app, cpu); });
}

void frontend::winaFreeSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_51a170", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 surface = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::freeSurface(app, cpu, surface); });
}

void frontend::winaGraphicsIdle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_5244f0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::graphicsIdle(app, cpu); });
}

void frontend::winaSetDirectDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_524500", 4, 0, nullptr, 0, 0 };
    const reg32 object = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::setDirectDraw(app, cpu, object); });
}

void frontend::winaGraphicsLock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_524550", 4, 0, nullptr, 0, 0 };
    const reg32 taking = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::graphicsLock(app, cpu, taking); });
}

void frontend::winaDriverMessage(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_5245a0", 8, 0, nullptr, 0, 0 };
    const reg32 level = app->getMemory<reg32>(cpu.esp + 4), text = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::driverMessage(app, cpu, level, text); });
}

void frontend::winaDriverEvent(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_5245f0", 8, 0, nullptr, 0, 0 };
    const reg32 kind = app->getMemory<reg32>(cpu.esp + 4), detail = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::driverEvent(app, cpu, kind, detail); });
}

void frontend::winaCloseDriverOnly(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_524620", 0, 0, nullptr, 0, kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::closeDriverOnly(app, cpu); });
}

void frontend::winaSetScreenSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_5247c0", 0, 0, nullptr, 0, 0 };
    const reg32 widthIn = cpu.eax, heightIn = cpu.edx, depthIn = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::setScreenSize(app, cpu, widthIn, heightIn, depthIn); });
}

void frontend::winaFreeSurfaceCache(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_525600", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 surface = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::freeSurfaceCache(app, cpu, surface); });
}

void frontend::winaNextByte(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_525630", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 place = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::nextByte(app, cpu, place); });
}

void frontend::winaMapCharacter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_525650", 0, 0, nullptr, 0, 0 };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::mapCharacter(app, cpu, character); });
}

void frontend::winaNextCharacter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_525670", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 place = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::nextCharacter(app, cpu, place); });
}

void frontend::winaTextStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_52e420", 0, 0, nullptr, 0, kBx | kCx | kDx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wina::textStart(app, cpu); });
}

}
