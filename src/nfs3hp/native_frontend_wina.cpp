#include "native_frontend_wina.h"
#include "native_frontend_pcall.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_raceload.h"
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
// What lib/thread.h holds pointers to, declared here.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include "os_time.h"
#include "os_video.h"
#include "os_window.h"
#include "os_files.h"
#include "os_system.h"
#include "native_graphics.h"
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined (the header's declarations are the thunks'). */
namespace nfs3hp
{
namespace frontend {
nfs::u32 winaStreamCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaRingBetween(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamConsume(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamTakeFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamQueueFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFindFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamUnlinkFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamClassify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamParse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFileOpened(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFileSwitched(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFilled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamNextFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamReadMore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamHeaderSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamSetRule(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamSetLowWater(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamSetBelow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamChannel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamOpenFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamOpenMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamCloseFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFlush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamNextChunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaStreamFreeChunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaChannelBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaChannelState(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaChannelDone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaChannelRingSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaMovieNextFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpAllocate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpSetName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileOpRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileSlotOfName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileCloseWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileSizeAndSeek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFileLockOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCompareHandlers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaHandlerOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaHandlerSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPostMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaSetFocusCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaNoWindowTask(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaMovePointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPushKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaKeyOfMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaKeyboardHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnSetFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnKillFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnQuitAsked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnDestroy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnSysCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnSettingChange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnChar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnKeyDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnKeyUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnMouseDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnMouseUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnPaint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOnEraseBackground(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaWindowProc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCreateWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaWindowThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaOpenWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCloseWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPlaceWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCentreWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaHookOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaHookOn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPointerMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaAlreadyRunning(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaVersionText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaTickCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCheckLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFatalMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCheckMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaDeviceAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaThrashDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaThrashDescription(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaDesktopSetting(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaDirectXVersion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaTableEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaShapeHotspots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaReleaseDriverSlot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaHasFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaVideoClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFreeKeptBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillHalved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillSame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillDoubled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillQuadrupled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFillTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaShiftTo64(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPaletteToSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPaletteSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaPaletteGet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaQueueTakeByPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaGraphicsClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaGraphicsAskClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaGraphicsPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaReleaseVideoBuffers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFreeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaGraphicsIdle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaSetDirectDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaGraphicsLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaDriverMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaDriverEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaCloseDriverOnly(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaSetScreenSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaFreeSurfaceCache(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaNextByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaMapCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaNextCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 winaTextStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax: the first `Taken` of `words` its arguments,
 * its result in eax. */
template <nfs::u32 Address, auto Core, std::size_t... I>
auto directOf(nfs::App* app, nfs::ThreadState& ts, const nfs::u32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, ts, words[I]...);
}

template <nfs::u32 Address, auto Core, std::size_t Taken, typename... Words>
nfs::u32 directKept(nfs::App* app, nfs::ThreadState& ts, Words... words)
{
    const nfs::u32 all[] = { nfs::u32(words)..., 0 };
    const auto result = directOf<Address, Core>(app, ts, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, nfs::u32>)
        return result;
    else
        return *result;
}

}  // namespace
}  // namespace nfs3hp


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
 * All state stays in guest memory.  The files, modules, settings and boxes are the port's own
 * (os_files.h, os_system.h), so are the window, its events, keys and screen (os_window.h) and the
 * display (os_video.h).  What each function leaves in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::wina (native_frontend_wina.h), in a frame of
 * its own where the game's function has one, its stack words handed to it as parameters, and a
 * thunk under the game's name for the calls by address.  A register the code reads that the
 * function took from its caller is a parameter too, put back where the code reads it. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;

constexpr std::uint8_t kBx = fe::kRestoreEbx, kCx = fe::kRestoreEcx, kDx = fe::kRestoreEdx;
constexpr std::uint8_t kSi = fe::kRestoreEsi, kDi = fe::kRestoreEdi;

// The game's messages in its window's table: those the port's events become (wina::dispatchEvent).
constexpr u32 kDestroyMessage = 0x2, kKeyDownMessage = 0x100, kKeyUpMessage = 0x101, kCharMessage = 0x102;

// The game's functions, by address.
constexpr u32 kFileOpened = 0x4cf570, kFileSwitched = 0x4cf5a0, kFilled = 0x4cf5e0;
constexpr u32 kOpWait = 0x4fc600;
constexpr u32 kOpSlotOfName = 0x4fce50;
constexpr u32 kSystemTasks = 0x4e7630;
constexpr u32 kSleep = 0x4df8e0, kStrcmp = 0x4ee310, kStrncpy = 0x4e0e30, kMemset = 0x4e0640;
constexpr u32 kBsearch = 0x4fb300, kQsort = 0x4ee4f8, kError = 0x401010;
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kCloseDriver = 0x524650, kFillDword = 0x4e0721;

// The window record the game has one of, and its globals.
constexpr u32 kWindow = 0x9f38d0;
constexpr u32 kHwnd = 0x9f3d24, kActive = 0x9f3d4c, kFullscreen = 0x9f3d2d, kInstance = 0x9f3d48;
constexpr u32 kMessageLock = 0x561290, kHandlerCount = 0x561268, kHandlers = 0x7f7060, kCompareHandlers = 0x4cc650;
constexpr u32 kKeyLock = 0x7f7478, kKeyHead = 0x7f7474, kKeyTail = 0x7f747c, kKeyRoom = 0x7f7480;
constexpr u32 kKeyQueue = 0x7f6fe0, kKeysDown = 0x564818, kScanTable = 0x561298, kWantRaw = 0x7f7488;
constexpr u32 kLastScan = 0x7f748c, kHookHandle = 0x56127c, kHookWanted = 0x561280, kHookCallback = 0x561288;
constexpr u32 kHookAlt = 0x561284, kFocusCallback = 0x7f7484, kOnResize = 0x567184, kWindowReady = 0x56126c;
constexpr u32 kQuitAsked = 0x561270, kStickyKeys = 0x561274, kFilterKeys = 0x561278, kKeyboardHook = 0x4ccc70;
constexpr u32 kWindowHandle = 0x454, kWindowFullscreen = 0x45d, kWindowX = 0x464, kWindowY = 0x468;
constexpr u32 kGraphicsLock = 0x564480;
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr u32 kFillColour = 0x9f4fac;  // what the row fills write
constexpr u32 kPalette = 0xa0a814;     // the screen's palette, as bytes blue green red

/* The stream's record, then its file records, rules and channels, then the ring. */
namespace stream
{
constexpr u32 kMagic = 0x4d525453;   // 'STRM'
constexpr u32 kLock = 0x04;
constexpr u32 kFiles = 0x08, kFileCount = 0x0c, kRules = 0x10, kRuleCount = 0x14;
constexpr u32 kChannels = 0x18, kChannelCount = 0x1c, kRing = 0x20, kRingEnd = 0x24;
constexpr u32 kState = 0x28;         // 0 idle, 1 reading, 2 waiting for room
constexpr u32 kPriorityIdle = 0x2c, kPriorityBusy = 0x30, kLowWater = 0x34, kBelow = 0x38, kQueued = 0x3c;
constexpr u32 kOldest = 0x40, kParsed = 0x44, kWritten = 0x48;
constexpr u32 kFirst = 0x4c, kCurrent = 0x50, kLast = 0x54, kFree = 0x58;
constexpr u32 kPath = 0x5c, kFile = 0x160, kFilePos = 0x164, kOp = 0x168, kReadSize = 0x16c;
constexpr u32 kHeader = 0x170;
constexpr u32 kBufferSize = 0x2000;  // what one read asks for
constexpr u32 kFreed = 0xfffffffe, kWrap = 0xffffffff;
constexpr u32 kLengthMask = 0xffffff, kChannelMask = 0xff000000;
}

/* A file on the stream's queue: 0x128 bytes. */
namespace file
{
constexpr u32 kSize = 0x128;
constexpr u32 kId = 0x00, kState = 0x04, kPrevious = 0x08, kNext = 0x0c, kInMemory = 0x10, kName = 0x14;
constexpr u32 kData = 0x118, kLength = 0x11c, kEndId = 0x120, kRingPos = 0x124;
constexpr u32 kWaiting = 1, kReading = 2, kAtEnd = 3, kClosing = 4;
}

/* A channel of chunks: 16 bytes. */
namespace channel
{
constexpr u32 kSize = 0x10;
constexpr u32 kStream = 0x00, kNumber = 0x04, kBytes = 0x08, kFirstChunk = 0x0c;
}

/* The file library's slot, one per priority class: 104 bytes in the table at 0x9f4f60. */
constexpr u32 kFileSlots = 0x9f4f60, kFileSlotSize = 104;
constexpr u32 kFileFreeOps = 0x9f4f74, kSlotQueue = 0x1c, kSlotEvent = 0x54, kSlotSerial = 0x60;

/* Another thread's turn with the game, where the game's code offers it. */
void safepoint(nfs::App* app, nfs::ThreadState& ts)
{
    if (app->contextWanted())
        app->yieldContext(ts);
}



/* sub_4cf1c0 on a stream's handle, its two out slots at the bottom of the frame:
 * frame() the stream, frame() + 4 the handle.  0 when it is a stream. */
u32 checkStream(Guest& guest, nfs::ThreadState& ts, u32 handle)
{
    return guest::direct<0x4cf1c0, frontend::wina::streamCheck>(guest.app(), ts, handle, guest.frame(),
                                                                guest.frame() + 4);
}

/* `length` bytes forward, as rep movsd and then rep movsb move them. */
void moveForward(Guest& guest, u32 to, u32 from, u32 length)
{
    const u32 dwords = length >> 2;
    for (u32 i = 0; i < dwords; ++i)
    {
        guest.setDword(to, guest.dword(from));
        to += 4;
        from += 4;
    }
    for (u32 i = 0; i < (length & 3); ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A chunk's length without the channel in its top byte. */
constexpr u32 lengthOf(u32 word)
{
    return word & stream::kLengthMask;
}

constexpr u32 fileSlot(u32 slot)
{
    return slot * kFileSlotSize;
}

/* A failed assertion of EA's: where it was, in the error record, and the message. */
void assertFailed(Guest& guest, u32 sourceFile, u32 function, u32 line, const fe::Arguments& arguments)
{
    guest.setDword(kErrorFile, sourceFile);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    // Its format and words pushed (Error reads no register).
    nfs::ThreadState& ts = guest.ts();
    guest::Words words({});
    for (std::size_t i = 1; i < arguments.stackCount; ++i)
        words.word[words.count++] = arguments.stack[i];
    fe::direct<0x401010, ea::error>(guest.app(), ts, arguments.stack[0], words);
}

/* The lock of the message table, made the first time it is asked for. */
void messageLockMade(Guest& guest, nfs::ThreadState& ts)
{
    const u32 lock = guest.dword(kMessageLock);
    if (lock == 0)
        guest.setDword(kMessageLock, u32(*fe::direct<0x4eb2e0, ea::newLock>(guest.app(), ts)));
}

/* Whether the system's screen saver and its power-off timer are on, kept (kStickyKeys and
 * kFilterKeys hold them) to be turned back on when the window closes, and turned off meanwhile.
 * The port has neither: both are kept as off, and nothing is turned off or, later, on. */
void accessKeysOff(Guest& guest, nfs::ThreadState& ts)
{
    NFS2_USE(ts);
    guest.setDword(kStickyKeys, 0);
    guest.setDword(kFilterKeys, 0);
}

/* The mouse messages' buttons, as the game keeps them, and the place in the message: its window, its
 * keys (wParam) and its place (lParam). */
void pointerMessage(Guest& guest, nfs::ThreadState& ts, u32 window, u32 keys, u32 place, u32 down)
{
    if (window != 0)
    {
        u32 buttons = 0;
        if (keys & 1)
            buttons = 1;
        if (keys & 2)
            buttons |= 2;
        if (keys & 0x10)
            buttons |= 4;
        guest::direct<0x4fb470, frontend::wina::pointerMoved>(guest.app(), ts, place & 0xffff, (place >> 16) & 0xffff,
                                                              buttons, down);
    }
}

}

/* ---------------------------------------------------------------------------
 * The buffered stream
 * ------------------------------------------------------------------------- */

/* sub_4cf1c0: a handle to its stream, both handed out; nonzero when it is none. */
nfs::u32 frontend::wina::streamCheck(App* app, ThreadState& ts, u32 handle, u32 streamOut, u32 handleOut)
{
    Guest guest(app, ts);
    if (handle == 0)
        return 1;
    const u32 s = guest.dword(handle);
    if (guest.dword(s) != stream::kMagic)
        return 1;
    guest.setDword(handleOut, handle);
    guest.setDword(streamOut, s);
    return 0;
}

/* sub_4cf1f0: whether a place of the ring is between two others, the way round. */
nfs::u32 frontend::wina::ringBetween(App*, ThreadState&, u32 from, u32 to, u32 place)
{
    if (from <= to)
        return place >= from && place < to ? 1 : 0;
    return place >= from || place < to ? 1 : 0;
}

/* sub_4cf220: bytes taken off what is queued; below the low water, more is asked for
 * sooner: the read in hand gets the busy priority. */
nfs::u32 frontend::wina::streamConsume(App* app, ThreadState& ts, u32 s, u32 amount)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 before = guest.dword(s + stream::kQueued);
    const u32 after = before - amount;
    const u32 lock = guest.dword(s + stream::kLock);
    guest.setDword(s + stream::kQueued, after);
    leaveLock(guest, ts, lock);
    const u32 low = guest.dword(s + stream::kLowWater);
    if (s32(before) >= s32(low) && s32(after) < s32(low))
    {
        const u32 state = guest.dword(s + stream::kState);
        guest.setDword(s + stream::kBelow, 1);
        if (state == 1)
        {
            return guest::direct<0x4fc9b0, fileOpPriority>(app, ts, guest.dword(s + stream::kOp),
                                                           guest.dword(s + stream::kPriorityBusy));
        }
    }
    return low;
}

/* sub_4cf280: a file record off the free list, its id's serial moved on; 0 when none. */
nfs::u32 frontend::wina::streamTakeFree(App* app, ThreadState& ts, u32 s)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    constexpr u32 kSerial = 0x7f74b0, kSerialStep = 0x100;
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 taken = guest.dword(s + stream::kFree);
    if (taken != 0)
    {
        const u32 serial = guest.dword(kSerial) + kSerialStep;
        guest.setDword(s + stream::kFree, guest.dword(taken + file::kNext));
        guest.setDword(kSerial, serial);
        if (serial == 0)
            guest.setDword(kSerial, kSerialStep);
        guest.setDword(taken + file::kId, (guest.dword(taken + file::kId) & 0xff) | guest.dword(kSerial));
    }
    leaveLock(guest, ts, guest.dword(s + stream::kLock));
    return taken;
}

/* sub_4cf2f0: a file record on the end of the queue. */
nfs::u32 frontend::wina::streamQueueFile(App* app, ThreadState& ts, u32 s, u32 record)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    guest.setDword(record + file::kState, file::kWaiting);
    guest.setDword(record + file::kNext, 0);
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 last = guest.dword(s + stream::kLast);
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
    return leaveLock(guest, ts, guest.dword(s + stream::kLock));
}

/* sub_4cf340: the file record of an id, 0 when it has gone. */
nfs::u32 frontend::wina::streamFindFile(App* app, ThreadState& ts, u32 s, u32 id)
{
    Guest guest(app, ts);
    const u32 index = id & 0xff;
    if (s32(index) < s32(guest.dword(s + stream::kFileCount)))
    {
        const u32 record = index * file::kSize + guest.dword(s + stream::kFiles);
        if (id == guest.dword(record + file::kId) && guest.dword(record + file::kState) != 0)
            return record;
    }
    return 0;
}

/* sub_4cf380: a file record off the queue, onto the free list. */
nfs::u32 frontend::wina::streamUnlinkFile(App* app, ThreadState& ts, u32 s, u32 record)
{
    Guest guest(app, ts);
    const u32 first = guest.dword(s + stream::kFirst);
    if (record == first)
        guest.setDword(s + stream::kFirst, guest.dword(first + file::kNext));
    else
        guest.setDword(guest.dword(record + file::kPrevious) + file::kNext, guest.dword(record + file::kNext));
    const u32 last = guest.dword(s + stream::kLast);
    if (record == last)
        guest.setDword(s + stream::kLast, guest.dword(last + file::kPrevious));
    else
        guest.setDword(guest.dword(record + file::kNext) + file::kPrevious, guest.dword(record + file::kPrevious));
    const u32 current = guest.dword(s + stream::kCurrent);
    if (record == current)
    {
        const u32 next = guest.dword(current + file::kNext);
        guest.setDword(s + stream::kCurrent, next != 0 ? next : guest.dword(current + file::kPrevious));
    }
    guest.setDword(record + file::kState, 0);
    guest.setDword(record + file::kNext, guest.dword(s + stream::kFree));
    guest.setDword(s + stream::kFree, record);
    return s;
}

/* sub_4cf3f0: the channel a chunk's id goes to, by the rules (mask, value, channel); -2 none. */
nfs::u32 frontend::wina::streamClassify(App* app, ThreadState& ts, u32 s, u32 chunk)
{
    Guest guest(app, ts);
    constexpr u32 kRuleSize = 12;
    const u32 id = guest.dword(chunk);
    u32 i = 0;
    for (;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(guest.dword(s + stream::kRuleCount)))
            break;
        const u32 rule = guest.dword(s + stream::kRules) + i * kRuleSize;
        if ((guest.dword(rule) & id) == guest.dword(rule + 4))
        {
            return guest.dword(rule + 8);
        }
    }
    return 0xfffffffe;
}

/* sub_4cf440: the whole chunks read so far given to their channels; 1 when the current
 * file's last chunk has come. */
nfs::u32 frontend::wina::streamParse(App* app, ThreadState& ts, u32 s)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 36);
    const u32 current = guest.dword(s + stream::kCurrent);
    for (;;)
    {
        safepoint(app, ts);
        const u32 chunk = guest.dword(s + stream::kParsed);
        if (s32(guest.dword(s + stream::kWritten) - chunk) < 8)
            return 0;
        const u32 length = guest.dword(chunk + 4);
        if ((length & stream::kChannelMask) != 0)
            return 0;
        if (chunk + length > guest.dword(s + stream::kWritten))
            return 0;
        const u32 number = guest::direct<0x4cf3f0, streamClassify>(app, ts, s, chunk);
        bool closing;
        if (s32(number) < 0)
        {
            enterLock(guest, ts, guest.dword(s + stream::kLock));
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
            enterLock(guest, ts, guest.dword(s + stream::kLock));
            closing = guest.dword(current + file::kState) == file::kClosing;
            if (!closing)
            {
                const u32 record = (number << 4) - channel::kSize + guest.dword(s + stream::kChannels);
                const u32 bytes = guest.dword(record + channel::kBytes) + length;
                guest.setDword(record + channel::kBytes, bytes);
                if (length == bytes)
                    guest.setDword(record + channel::kFirstChunk, chunk);
                guest.setDword(s + stream::kParsed, guest.dword(s + stream::kParsed) + length);
                const u32 before = guest.dword(s + stream::kQueued);
                const u32 after = length + before;
                const u32 low = guest.dword(s + stream::kLowWater);
                guest.setDword(s + stream::kQueued, after);
                if (s32(before) < s32(low) && s32(after) >= s32(low))
                    guest.setDword(s + stream::kBelow, 0);
            }
        }
        leaveLock(guest, ts, guest.dword(s + stream::kLock));
        if (closing)
            return 0;
        if (guest.dword(chunk) == guest.dword(current + file::kEndId))
            return 1;
    }
}

/* sub_4cf570: the file's open has ended: its handle kept, reading begins. */
nfs::u32 frontend::wina::streamFileOpened(App* app, ThreadState& ts, u32 s)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    const u32 handle = fe::direct<0x4fc600, menz::fileOpComplete>(app, ts, guest.dword(s + stream::kOp));
    guest.setDword(s + stream::kFile, handle);
    if (handle == 0)
        return handle;
    return guest::direct<0x4cf810, streamReadMore>(app, ts, s, guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cf5a0: the last file's close has ended: the next one opened. */
nfs::u32 frontend::wina::streamFileSwitched(App* app, ThreadState& ts, u32 s)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    fe::direct<0x4fc600, menz::fileOpComplete>(app, ts, guest.dword(s + stream::kOp));
    // edx is 1 (what the code loads for the call) and the callee keeps it.
    const u32 op = guest::direct<0x4fca60, fileOpOpen>(app, ts, s + stream::kPath, 1,
                                                         guest.dword(s + stream::kPriorityBusy), s);
    guest.setDword(s + stream::kOp, op);
    if (op == 0)
        return op;
    return guest::direct<0x4fc8e0, fileOpCallback>(app, ts, op, kFileOpened);
}

/* sub_4cf5e0: a read has ended, or a memory block was copied: the chunks parsed, and on
 * to more of this file or the next. */
nfs::u32 frontend::wina::streamFilled(App* app, ThreadState& ts, u32 s)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    const u32 current = guest.dword(s + stream::kCurrent);
    u32 got, ended;
    if (guest.dword(current + file::kInMemory) == 1)
    {
        got = guest.dword(s + stream::kReadSize);
        ended = s32(guest.dword(s + stream::kFilePos) + got) >= s32(guest.dword(current + file::kLength)) ? 1 : 0;
    }
    else
    {
        got = fe::direct<0x4fc600, menz::fileOpComplete>(app, ts, guest.dword(s + stream::kOp));
        // edx is s (what the code loads for the call, kept by the callee), ebx as it came: s too.
        ended = s32(got) < s32(guest.dword(s + stream::kReadSize)) ? 1 : s ^ s;  // ebx is the stream, as it came
    }
    const u32 position = guest.dword(s + stream::kFilePos) + got;
    const u32 written = guest.dword(s + stream::kWritten) + got;
    guest.setDword(s + stream::kFilePos, position);
    guest.setDword(s + stream::kWritten, written);
    // The game's code hands it the end in edx, which it keeps and the test below reads.
    const u32 finished = guest::direct<0x4cf440, streamParse>(app, ts, s);
    if (guest.dword(current + file::kState) == file::kClosing)
        return guest::direct<0x4cf6b0, streamNextFile>(app, ts, s, guest.dword(s + stream::kPriorityBusy));
    if (ended == 0 && finished == 0)
        return guest::direct<0x4cf810, streamReadMore>(app, ts, s, guest.dword(s + stream::kPriorityBusy));
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    if (guest.dword(current + file::kState) != file::kClosing)
        guest.setDword(current + file::kState, file::kAtEnd);
    const u32 lock = guest.dword(s + stream::kLock);
    leaveLock(guest, ts, lock);
    return guest::direct<0x4cf6b0, streamNextFile>(app, ts, s, guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cf6b0: on to the next file of the queue: from memory at once, from a file by an
 * open, after a close when another one is open. */
nfs::u32 frontend::wina::streamNextFile(App* app, ThreadState& ts, u32 s, u32 priority)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 32);
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 current = guest.dword(s + stream::kCurrent);
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
        const u32 next = guest.dword(current + file::kNext);
        none = next == 0;
        if (!none)
            guest.setDword(s + stream::kCurrent, next);
    }
    u32 record = 0;
    if (none)
    {
        guest.setDword(s + stream::kState, 0);
    }
    else
    {
        record = guest.dword(s + stream::kCurrent);
        const u32 parsed = guest.dword(s + stream::kParsed);
        guest.setDword(record + file::kState, file::kReading);
        guest.setDword(record + file::kRingPos, parsed);
    }
    const u32 lock = guest.dword(s + stream::kLock);
    fe::leaveLock(guest, ts, lock);
    if (none)
        return s;
    guest.setDword(s + stream::kWritten, guest.dword(s + stream::kParsed));
    if (guest.dword(record + file::kInMemory) == 1)
    {
        guest.setDword(s + stream::kFilePos, 0);
    }
    else
    {
        guest.setDword(s + stream::kFilePos, guest.dword(record + file::kLength));
        const u32 path = s + stream::kPath;
        const u32 name = record + file::kName;
        if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, path) != 0)
        {
            guest.copyString(path, name);
            const u32 open = guest.dword(s + stream::kFile);
            u32 op, callback;
            if (open == 0)
            {
                op = guest::direct<0x4fca60, fileOpOpen>(app, ts, path, 1, priority, s);
                callback = kFileOpened;
            }
            else
            {
                op = guest::direct<0x4fca90, fileOpClose>(app, ts, open, priority, s);
                callback = kFileSwitched;
            }
            guest.setDword(s + stream::kOp, op);
            if (op == 0)
                return op;
            return guest::direct<0x4fc8e0, fileOpCallback>(app, ts, op, callback);
        }
    }
    return guest::direct<0x4cf810, streamReadMore>(app, ts, s, priority);
}

/* sub_4cf810: the next read into the ring: records of files read through let go, the
 * ring wrapped when the end is too near, and 0x2000 bytes asked for or copied. */
nfs::u32 frontend::wina::streamReadMore(App* app, ThreadState& ts, u32 s, u32 priority)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    // The oldest chunk past what has been freed.
    for (;;)
    {
        safepoint(app, ts);
        const u32 oldest = guest.dword(s + stream::kOldest);
        if (guest.dword(s + stream::kParsed) == oldest)
            break;
        const u32 id = guest.dword(oldest);
        if (id == stream::kWrap)
            guest.setDword(s + stream::kOldest, guest.dword(s + stream::kRing));
        else if (id == stream::kFreed)
            guest.setDword(s + stream::kOldest, oldest + guest.dword(oldest + 4));
        else
            break;
    }
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    // The files whose chunks have all been read.
    for (;;)
    {
        safepoint(app, ts);
        const u32 first = guest.dword(s + stream::kFirst);
        const u32 next = guest.dword(first + file::kNext);
        if (next == 0 || guest.dword(next + file::kState) == file::kWaiting)
            break;
        if (guest::direct<0x4cf1f0, ringBetween>(app, ts, guest.dword(s + stream::kOldest), guest.dword(s + stream::kWritten),
                                                 guest.dword(next + file::kRingPos) - 1) != 0)
            break;
        guest::direct<0x4cf380, streamUnlinkFile>(app, ts, s, first);
    }
    leaveLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 oldest = guest.dword(s + stream::kOldest);
    const u32 written = guest.dword(s + stream::kWritten);
    u32 room;
    if (oldest > written)
    {
        room = oldest - written - 1;
    }
    else
    {
        room = guest.dword(s + stream::kRingEnd) - written - 8;
        if (s32(room) < s32(stream::kBufferSize))
        {
            // Too near the end: what is unparsed goes to the start, if it fits.
            const u32 parsed = guest.dword(s + stream::kParsed);
            const u32 partial = written - parsed;
            const u32 ring = guest.dword(s + stream::kRing);
            if (s32(oldest - ring) < s32(partial + 1))
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
    if (s32(room) < s32(stream::kBufferSize))
    {
        guest.setDword(s + stream::kState, 2);
        return s;
    }
    const u32 current = guest.dword(s + stream::kCurrent);
    if (guest.dword(current + file::kInMemory) == 1)
    {
        const u32 length = guest.dword(current + file::kLength);
        if (s32(guest.dword(s + stream::kFilePos) + room) > s32(length))
            room = length - guest.dword(s + stream::kFilePos);
        guest.setDword(s + stream::kReadSize, room);
        const u32 size = guest.dword(s + stream::kReadSize);
        moveForward(guest, guest.dword(s + stream::kWritten), guest.dword(current + file::kData), size);
        guest.setDword(current + file::kData, guest.dword(current + file::kData) + guest.dword(s + stream::kReadSize));
        return guest::direct<0x4cf5e0, streamFilled>(app, ts, s);
    }
    guest.setDword(s + stream::kReadSize, stream::kBufferSize);
    const u32 op = guest::direct<0x4fcab0, fileOpRead>(app, ts, guest.dword(s + stream::kFile),
                                                         guest.dword(s + stream::kFilePos), guest.dword(s + stream::kWritten),
                                                         guest.dword(s + stream::kReadSize), priority, s);
    guest.setDword(s + stream::kOp, op);
    if (op == 0)
        return op;
    return guest::direct<0x4fc8e0, fileOpCallback>(app, ts, op, kFilled);
}

/* sub_4cfa40: the bytes a stream's record and tables take. */
nfs::u32 frontend::wina::streamHeaderSize(App* app, ThreadState& ts, u32 fileCount, u32 ruleCount, u32 channelCount)
{
    Guest guest(app, ts);
    constexpr u32 kRuleSize = 12;
    const u32 files = fileCount * file::kSize + stream::kHeader;
    const u32 channels = channelCount << 4;
    const u32 rules = ruleCount * kRuleSize;
    return rules + files + channels;
}

/* sub_4cfa80: a stream made in the memory given; its first channel, or 0 when it does
 * not fit or asks too much. */
nfs::u32 frontend::wina::streamCreate(App* app, ThreadState& ts, u32 files, u32 rules, u32 channels, u32 s,
                                        u32 memory)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kRuleSize = 12, kLeastRing = 0x6000, kMostFiles = 0x100, kMostRules = 0x10;
    const u32 room = memory - guest::direct<0x4cfa40, streamHeaderSize>(app, ts, files, rules, channels);
    if (s32(room) < s32(kLeastRing))
        return 0;
    if (s32(files) < 2 || s32(files) > s32(kMostFiles))
        return 0;
    if (s32(rules) < 1 || s32(rules) > s32(kMostRules))
        return 0;
    if (s32(channels) < 1 || s32(rules) < s32(channels))
        return 0;

    guest.setDword(s, stream::kMagic);
    const u32 lock = u32(*fe::direct<0x4eb2e0, ea::newLock>(app, ts));
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
    const u32 ring = guest.dword(s + stream::kRing);
    guest.setDword(s + stream::kOldest, ring);
    guest.setDword(s + stream::kParsed, ring);
    guest.setDword(s + stream::kWritten, ring);
    guest.setDword(s + stream::kFree, guest.dword(s + stream::kFiles));
    guest.setDword(s + stream::kRuleCount, rules);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), s + stream::kPath, 0, stream::kFile - stream::kPath);
    guest.setDword(s + stream::kFile, 0);

    // The file records, on the free list.
    u32 record = 0;
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(files))
            break;
        record = guest.dword(s + stream::kFiles) + i * file::kSize;
        guest.setDword(record + file::kId, i);
        guest.setDword(record + file::kState, 0);
        guest.setDword(record + file::kNext, guest.dword(s + stream::kFiles) + (i + 1) * file::kSize);
    }
    guest.setDword(guest.dword(s + stream::kFiles) + (files - 1) * file::kSize + file::kNext, 0);
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(rules))
            break;
        const u32 rule = guest.dword(s + stream::kRules) + i * kRuleSize;
        guest.setDword(rule, 0);
        guest.setDword(rule + 4, 0);
        guest.setDword(rule + 8, 1);
    }
    u32 last = 0;
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(channels))
            break;
        last = guest.dword(s + stream::kChannels) + (i << 4);
        guest.setDword(last + channel::kStream, s);
        guest.setDword(last + channel::kBytes, 0);
        guest.setDword(last + channel::kNumber, i + 1);
    }
    return guest.dword(s + stream::kChannels);
}

/* sub_4cfca0: a rule of a stream that is not reading: mask, value, channel. */
nfs::u32 frontend::wina::streamSetRule(App* app, ThreadState& ts, u32 handle, u32 number, u32 mask, u32 value,
                                         u32 target)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    constexpr u32 kRuleSize = 12;
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0 || s32(number) < 1)
        return bad;
    const u32 s = guest.dword(guest.frame());
    const u32 rules = guest.dword(s + stream::kRuleCount);
    if (s32(number) > s32(rules))
        return s;
    u32 left = s;
    if (number == rules)
    {
        left = mask | value;
        if (left != 0)
            return left;
    }
    if (s32(target) < 1 && target != 0xffffffff && target != 0xfffffffe)
        return left;
    if (s32(target) > s32(guest.dword(s + stream::kChannelCount)))
        return s;
    if (guest.dword(s + stream::kState) != 0)
        return s;
    const u32 rule = guest.dword(s + stream::kRules) + (number - 1) * kRuleSize;
    guest.setDword(rule + 4, value);
    guest.setDword(rule + 8, target);
    guest.setDword(rule, mask);
    return rule;
}

/* sub_4cfd30: a stream closed: flushed, its read waited for, its lock and file gone. */
nfs::u32 frontend::wina::streamClose(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    guest::direct<0x4d0220, streamFlush>(app, ts, handle);
    for (;;)
    {
        safepoint(app, ts);
        if (guest.dword(guest.dword(guest.frame()) + stream::kState) != 1)
            break;
        if (*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0))
            fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    }
    guest.setDword(guest.dword(guest.frame()), 0);
    const u32 s = guest.dword(guest.frame());
    fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(s + stream::kLock));
    return guest::direct<0x4fd060, fileCloseWait>(app, ts, guest.dword(guest.dword(guest.frame()) + stream::kFile), 0x64);
}

/* sub_4cfdd0: the low water mark: crossing it tells the read in hand. */
nfs::u32 frontend::wina::streamSetLowWater(App* app, ThreadState& ts, u32 handle, u32 low)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    const u32 s = guest.dword(guest.frame());
    const u32 before = guest.dword(s + stream::kLowWater);
    guest.setDword(s + stream::kLowWater, low);
    const u32 queued = guest.dword(s + stream::kQueued);
    const u32 wasBelow = s32(queued) < s32(before) ? 1 : 0;
    const u32 isBelow = s32(queued) < s32(low) ? 1 : 0;
    if (wasBelow == isBelow)
        return queued;
    return guest::direct<0x4cfe30, streamSetBelow>(app, ts, handle, isBelow);
}

/* sub_4cfe30: whether the stream is below its low water; when it is, the read gets the
 * busy priority. */
nfs::u32 frontend::wina::streamSetBelow(App* app, ThreadState& ts, u32 handle, u32 below)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    const u32 s = guest.dword(guest.frame());
    guest.setDword(s + stream::kBelow, below);
    if (below == 0 || guest.dword(s + stream::kState) != 1)
        return s;
    return guest::direct<0x4fc9b0, fileOpPriority>(app, ts, guest.dword(s + stream::kOp),
                                                   guest.dword(s + stream::kPriorityBusy));
}

/* sub_4cfe70: a channel's handle, by its number from 1; 0 when there is none. */
nfs::u32 frontend::wina::streamChannel(App* app, ThreadState& ts, u32 handle, u32 number)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    const u32 s = guest.dword(guest.frame());
    if (s32(number) < 1 || s32(number) > s32(guest.dword(s + stream::kChannelCount)))
        return 0;
    const u32 channels = guest.dword(s + stream::kChannels);
    return (number << 4) - channel::kSize + channels;
}

/* sub_4cfef0: a file put on the queue; reading starts if it had stopped.  Its id, 0 when
 * there is no room. */
nfs::u32 frontend::wina::streamOpenFile(App* app, ThreadState& ts, u32 handle, u32 name, u32 length, u32 endId)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    constexpr u32 kMostName = 0x103;
    const u32 frame = guest.frame();
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    const u32 s = guest.dword(frame);
    const u32 record = guest::direct<0x4cf280, streamTakeFree>(app, ts, s);
    if (record == 0)
        return 0;
    guest.setDword(record + file::kInMemory, 0);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), record + file::kName, name, kMostName);
    guest.setDword(record + file::kEndId, endId);
    guest.setDword(record + file::kLength, length);
    guest::direct<0x4cf2f0, streamQueueFile>(app, ts, s, record);
    const u32 lock = guest.dword(s + stream::kLock);
    enterLock(guest, ts, lock);
    const u32 state = guest.dword(s + stream::kState);
    if (state == 0)
        guest.setDword(s + stream::kState, 1);
    const u32 lockAgain = guest.dword(s + stream::kLock);
    leaveLock(guest, ts, lockAgain);
    if (state == 0)
    {
        const u32 priority = guest.dword(s + stream::kBelow) != 0 ? guest.dword(s + stream::kPriorityBusy)
                                                                    : guest.dword(s + stream::kPriorityIdle);
        guest::direct<0x4cf6b0, streamNextFile>(app, ts, s, priority);
    }
    return guest.dword(record + file::kId);
}

/* sub_4cffb0: a block of memory put on the queue; with no length given, it runs to the
 * chunk with the end id.  Its id, 0 when there is no room. */
nfs::u32 frontend::wina::streamOpenMemory(App* app, ThreadState& ts, u32 handle, u32 data, u32 length, u32 endId)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    const u32 frame = guest.frame();
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    const u32 s = guest.dword(frame);
    const u32 record = guest::direct<0x4cf280, streamTakeFree>(app, ts, s);
    if (record == 0)
        return 0;
    if (length == 0)
    {
        u32 chunk = data;
        for (;;)
        {
            safepoint(app, ts);
            if (endId == guest.dword(chunk))
                break;
            const u32 size = guest.dword(chunk + 4);
            length += size;
            chunk += size;
        }
        length += guest.dword(chunk + 4);
    }
    guest.setDword(record + file::kInMemory, 1);
    guest.setDword(record + file::kData, data);
    guest.setDword(record + file::kLength, length);
    guest.setDword(record + file::kEndId, endId);
    guest::direct<0x4cf2f0, streamQueueFile>(app, ts, s, record);
    const u32 lock = guest.dword(s + stream::kLock);
    enterLock(guest, ts, lock);
    const u32 state = guest.dword(s + stream::kState);
    if (state == 0)
        guest.setDword(s + stream::kState, 1);
    leaveLock(guest, ts, guest.dword(s + stream::kLock));
    if (state == 0)
        guest::direct<0x4cf6b0, streamNextFile>(app, ts, s, 0);
    return guest.dword(record + file::kId);
}

/* sub_4d0060: a file of the queue closed: what it has in the ring taken back from every
 * channel, by freeing its chunks or marking them. */
nfs::u32 frontend::wina::streamCloseFile(App* app, ThreadState& ts, u32 handle, u32 id)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 48);
    const u32 frame = guest.frame();
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    const u32 s = guest.dword(frame);
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 record = guest::direct<0x4cf340, streamFindFile>(app, ts, s, id);
    bool nothing;
    u32 from = 0, to = 0, oldest = 0;
    if (record == 0 || guest.dword(record + file::kState) == file::kClosing)
    {
        nothing = true;
    }
    else if (guest.dword(record + file::kState) == file::kWaiting)
    {
        guest::direct<0x4cf380, streamUnlinkFile>(app, ts, s, record);
        nothing = true;
    }
    else
    {
        guest.setDword(record + file::kState, file::kClosing);
        oldest = guest.dword(s + stream::kOldest);
        from = record == guest.dword(s + stream::kFirst) ? oldest : guest.dword(record + file::kRingPos);
        const u32 next = guest.dword(record + file::kNext);
        to = next == 0 || guest.dword(next + file::kState) == file::kWaiting ? guest.dword(s + stream::kParsed)
                                                                             : guest.dword(next + file::kRingPos);
        nothing = false;
    }
    const u32 lock = guest.dword(s + stream::kLock);
    fe::leaveLock(guest, ts, lock);
    if (nothing)
        return s;

    for (u32 index = 0;; ++index)
    {
        safepoint(app, ts);
        if (s32(index) >= s32(guest.dword(s + stream::kChannelCount)))
            return s;
        const u32 reader = guest.dword(s + stream::kChannels) + (index << 4);
        guest.setDword(frame + 4, reader);
        if (s32(guest.dword(reader + channel::kBytes)) <= 0)
            continue;
        if (guest::direct<0x4cf1f0, ringBetween>(app, ts, oldest, from, guest.dword(reader + channel::kFirstChunk)) != 0)
        {
            // The channel's reader is before these chunks: marked, for when it comes.
            const u32 mine = guest.dword(reader + channel::kNumber) << 24;
            u32 chunk = from;
            for (;;)
            {
                safepoint(app, ts);
                if (chunk == to)
                    break;
                if (guest.dword(chunk) == stream::kWrap)
                {
                    chunk = guest.dword(s + stream::kRing);
                    continue;
                }
                const u32 length = lengthOf(guest.dword(chunk + 4));
                if ((guest.dword(chunk + 4) & stream::kChannelMask) == mine)
                {
                    enterLock(guest, ts, guest.dword(s + stream::kLock));
                    guest.setDword(reader + channel::kBytes, guest.dword(reader + channel::kBytes) - length);
                    leaveLock(guest, ts, guest.dword(s + stream::kLock));
                    guest::direct<0x4cf220, streamConsume>(app, ts, s, length);
                    guest.setDword(chunk, stream::kFreed);
                    guest.setDword(chunk + 4, length);
                }
                chunk += length;
            }
        }
        else if (guest::direct<0x4cf1f0, ringBetween>(app, ts, from, to, guest.dword(reader + channel::kFirstChunk)) != 0)
        {
            // Its reader is inside them: read on, past them.
            do
            {
                safepoint(app, ts);
                const u32 chunk = guest::direct<0x4d0300, streamNextChunk>(app, ts, reader);
                guest::direct<0x4d03c0, streamFreeChunk>(app, ts, reader, chunk);
                if (s32(guest.dword(reader + channel::kBytes)) <= 0)
                    break;
            } while (guest::direct<0x4cf1f0, ringBetween>(app, ts, from, to, guest.dword(reader + channel::kFirstChunk)) != 0);
        }
    }
}

/* sub_4d0220: everything off a stream: its files closed, the channels emptied, the ring
 * marked free. */
nfs::u32 frontend::wina::streamFlush(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    const u32 frame = guest.frame();
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    u32 last = guest.dword(guest.dword(frame) + stream::kLast);
    if (last == 0)
        return 0;
    for (;;)
    {
        const u32 state = guest.dword(last + file::kState);
        if (state != file::kWaiting && state != file::kReading)
            break;
        guest::direct<0x4d0060, streamCloseFile>(app, ts, handle, guest.dword(last + file::kId));
        safepoint(app, ts);
        last = guest.dword(guest.dword(frame) + stream::kLast);
    }
    u32 current;
    for (;;)
    {
        safepoint(app, ts);
        const u32 s = guest.dword(frame);
        current = guest.dword(s + stream::kCurrent);
        const u32 first = guest.dword(s + stream::kFirst);
        if (first == current)
            break;
        guest::direct<0x4cf380, streamUnlinkFile>(app, ts, s, first);
    }
    guest.setDword(current + file::kState, file::kClosing);
    const u32 s = guest.dword(frame);
    for (u32 i = 0; s32(i) < s32(guest.dword(s + stream::kChannelCount)); ++i)
    {
        safepoint(app, ts);
        guest.setDword(guest.dword(s + stream::kChannels) + (i << 4) + channel::kBytes, 0);
    }
    guest::direct<0x4cf220, streamConsume>(app, ts, s, guest.dword(s + stream::kQueued));
    u32 chunk = guest.dword(guest.dword(frame) + stream::kOldest);
    for (;;)
    {
        safepoint(app, ts);
        if (chunk == guest.dword(guest.dword(frame) + stream::kParsed))
            break;
        if (guest.dword(chunk) == stream::kWrap)
        {
            chunk = guest.dword(guest.dword(frame) + stream::kRing);
            continue;
        }
        const u32 length = lengthOf(guest.dword(chunk + 4));
        guest.setDword(chunk, stream::kFreed);
        guest.setDword(chunk + 4, length);
        chunk += length;
    }
    if (guest.dword(guest.dword(frame) + stream::kState) == 2)
        guest.setDword(guest.dword(frame) + stream::kState, 0);
    return chunk;
}

/* sub_4d0300: a channel's next chunk, its reader moved on past it; 0 when it has none. */
nfs::u32 frontend::wina::streamNextChunk(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 32);
    const u32 frame = guest.frame();
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    const u32 reader = guest.dword(frame + 4);
    if (guest.dword(reader + channel::kBytes) == 0)
        return 0;
    const u32 chunk = guest.dword(reader + channel::kFirstChunk);
    const u32 length = lengthOf(guest.dword(chunk + 4));
    guest.setDword(chunk + 4, length);
    const u32 s = guest.dword(frame);
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 bytes = guest.dword(reader + channel::kBytes) - length;
    guest.setDword(reader + channel::kBytes, bytes);
    leaveLock(guest, ts, guest.dword(s + stream::kLock));
    if (s32(bytes) > 0)
    {
        const u32 mine = guest.dword(reader + channel::kNumber) << 24;
        u32 at = chunk + length;
        for (;;)
        {
            safepoint(app, ts);
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
nfs::u32 frontend::wina::streamFreeChunk(App* app, ThreadState& ts, u32 handle, u32 chunk)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    const u32 frame = guest.frame();
    const u32 bad = checkStream(guest, ts, handle);
    if (bad != 0)
        return bad;
    const u32 s = guest.dword(frame);
    if (chunk < guest.dword(s + stream::kRing))
        return s;
    const u32 last = guest.dword(s + stream::kRingEnd) - 8;
    if (chunk > last)
        return last;
    if (guest.dword(chunk) == stream::kFreed)
        return last;
    guest.setDword(chunk, stream::kFreed);
    guest::direct<0x4cf220, streamConsume>(app, ts, s, guest.dword(chunk + 4));
    enterLock(guest, ts, guest.dword(s + stream::kLock));
    const u32 state = guest.dword(s + stream::kState);
    if (state == 2)
        guest.setDword(s + stream::kState, 1);
    leaveLock(guest, ts, guest.dword(s + stream::kLock));
    if (state != 2)
        return s;
    const u32 priority = guest.dword(s + stream::kBelow) != 0 ? guest.dword(s + stream::kPriorityBusy)
                                                                : guest.dword(s + stream::kPriorityIdle);
    return guest::direct<0x4cf810, streamReadMore>(app, ts, s, priority);
}

/* sub_4d0460: the bytes a channel has waiting. */
nfs::u32 frontend::wina::channelBytes(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame() + 4) + channel::kBytes);
}

/* sub_4d0490: the state of a channel's stream. */
nfs::u32 frontend::wina::channelState(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame()) + stream::kState);
}

/* sub_4d04c0: whether a channel has come to its end: nothing waiting, nothing reading. */
nfs::u32 frontend::wina::channelDone(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    return guest.dword(guest.dword(guest.frame()) + stream::kState) == 0
                   && guest.dword(guest.dword(guest.frame() + 4) + channel::kBytes) == 0
               ? 1
               : 0;
}

/* sub_4d0500: how big a channel's stream's ring is. */
nfs::u32 frontend::wina::channelRingSize(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    if (checkStream(guest, ts, handle) != 0)
        return 0;
    const u32 s = guest.dword(guest.frame());
    return guest.dword(s + stream::kRingEnd) - guest.dword(s + stream::kRing);
}

/* sub_495b60: the movie's next frame chunk from its channel; what is not one is thrown
 * away.  0 at the channel's end.  It lets other threads run at every try. */
nfs::u32 frontend::wina::movieNextFrame(App* app, ThreadState& ts, u32 reader)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    constexpr u32 kFrameKey = 0x6d44414d, kFrameInter = 0x6544414d, kFrameBlack = 0x6b44414d;
    constexpr u32 kFramesTaken = 0x79f32c;
    for (;;)
    {
        app->unlockContext(ts);
        nfs::Thread::sleep(0);
        app->lockContext(ts);
        safepoint(app, ts);
        const u32 chunk = guest::direct<0x4d0300, streamNextChunk>(app, ts, reader);
        if (chunk != 0)
        {
            const u32 id = guest.dword(chunk);
            if (id == kFrameKey || id == kFrameInter || id == kFrameBlack)
            {
                guest.setDword(kFramesTaken, guest.dword(kFramesTaken) + 1);
                return chunk;
            }
            guest::direct<0x4d03c0, streamFreeChunk>(app, ts, reader, chunk);
        }
        if (guest::direct<0x4d04c0, channelDone>(app, ts, reader) != 0)
            return 0;
    }
}

/* ---------------------------------------------------------------------------
 * EA's file library
 * ------------------------------------------------------------------------- */

/* sub_4fb990: a file operation off the free list, set up; its slot's serial in its id. */
nfs::u32 frontend::wina::fileOpAllocate(App* app, ThreadState& ts, u32 kind, u32 priorityIn, u32 target, u32 slot)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    constexpr u32 kSource = 0x54dcac, kFunction = 0x54dd10;
    const u32 frame = guest.frame();
    guest.setDword(frame, priorityIn);
    if (guest.dword(kFileSlots) == 0)
    {
        assertFailed(guest, kSource, kFunction, 0xc5, args({ 0xc5 }, { 0x54dd20 }));
    }
    const u32 priority = guest.dword(frame);
    if (s32(priority) < 0 || s32(priority) > 0xff)
    {
        assertFailed(guest, kSource, kFunction, 0xc8, args({ kFunction, 0xc8 }, { 0x54dd64, priority }));
    }
    if (s32(slot) < 0 || slot > 0x1f)
    {
        assertFailed(guest, kSource, kFunction, 0xcb, args({ kFunction, 0xcb }, { 0x54dda0, slot }));
    }
    const u32 op = fe::direct<0x50df60, netb::qpop>(app, ts, kFileFreeOps);
    const u32 record = guest.dword(kFileSlots) + fileSlot(slot);
    if (op == 0)
    {
        assertFailed(guest, kSource, kFunction, 0xd3, args({}, { 0x54dddc }));
        return 0;
    }
    if (guest.dword(record) == 0)
        fe::direct<0x4fb860, menz::fileDeviceStart>(app, ts, slot);
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
    u32 token = 0;
    if (guest.dword(record) != 0)
        token = fe::direct<0x50ea20, menz::fileLockEnter>(app, ts, record + kSlotQueue);
    guest.setDword(op + 4, (guest.dword(record + kSlotSerial) << 5) | slot);
    const u32 serial = (guest.dword(record + kSlotSerial) + 1) & 0xffffff;
    guest.setDword(record + kSlotSerial, serial);
    if (serial == 0)
        guest.setDword(record + kSlotSerial, 1);
    if (guest.dword(record) != 0)
        fe::direct<0x50ea40, menz::fileLockLeave>(app, ts, record + kSlotQueue, token);
    return op;
}

/* sub_4fbb60: an operation on its slot's queue, the slot's thread told; its id. */
nfs::u32 frontend::wina::fileOpQueue(App* app, ThreadState& ts, u32 op)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    const u32 record = guest.dword(kFileSlots) + fileSlot(guest.dword(op + 4) & 0x1f);
    if (record == 0)
    {
        ts.sp -= 8;
        assertFailed(guest, 0x54dcac, 0x54de10, 0xfb, args({}, { 0x54de20 }));
        ts.sp += 8;
    }
    fe::direct<0x50e0a0, netb::qinsert>(app, ts, record + kSlotQueue, op);
    fe::direct<0x4dfb30, ea::eventSet>(app, ts, guest.dword(record + kSlotEvent));
    return guest.dword(op + 4);
}

/* sub_4fbbe0: an operation's file name, a copy of it in memory of its own. */
nfs::u32 frontend::wina::fileOpSetName(App* app, ThreadState& ts, u32 op, u32 nameIn)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kNoName = 0x54de60;
    const u32 name = nameIn != 0 ? nameIn : kNoName;
    const u32 size = guest.length(name) + 1;
    guest.setDword(guest.frame(), size);
    guest.setByte(op + 0xc, guest.byte(op + 0xc) | 1);
    const u32 copy = *fe::direct<0x501110, ea::allocatePages>(app, ts, guest.frame());
    guest.setDword(op + 0x2c, copy);
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, copy, name, size);
    return name + size;  // eax past the bytes, as the memmove leaves it
}

/* sub_4fc8e0: what is to be called when an operation ends; at once when it has. */
nfs::u32 frontend::wina::fileOpCallback(App* app, ThreadState& ts, u32 id, u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    const u32 frame = guest.frame();
    const u32 record = guest.dword(kFileSlots) + fileSlot(id & 0x1f);
    const u32 token = guest.dword(record) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, ts, record + kSlotQueue) : 0;
    const u32 op = fe::direct<0x4fbd10, menz::fileOpFind>(app, ts, record, id, frame);
    u32 left = op;  // what the calls leave in eax
    if (op == 0)
    {
        assertFailed(guest, 0x54dcac, 0x54df38, 0x4a6, args({}, { 0x54df48 }));
    }
    else
    {
        guest.setByte(op + 0xc, guest.byte(op + 0xc) | 4);
        const u32 pending = guest.dword(frame);
        if (pending != 0)
        {
            guest.setDword(op + 0x20, callback);
        }
        else
        {
            guest.setDword(op + 0x20, pending);
            guest.setByte(op + 0xc, guest.byte(op + 0xc) | 8);
            const u32 priority = u32(s32(guest.dword(op + 0xd)) >> 24);
            left = fe::pointerCall(guest, callback, args({ guest.dword(op + 4), priority, guest.dword(op + 0x1c) }));
        }
    }
    if (guest.dword(record) != 0)
        return fe::direct<0x50ea40, menz::fileLockLeave>(app, ts, record + kSlotQueue, token);
    return left;
}

/* sub_4fc9b0: an operation still queued given another priority. */
nfs::u32 frontend::wina::fileOpPriority(App* app, ThreadState& ts, u32 id, u32 priority)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 slots = guest.dword(kFileSlots);
    const u32 record = fileSlot(id & 0x1f) + slots;
    if (record == 0)
        return 0;
    // The priority the game's code hands it in ecx is read neither by it nor by the test (sub_4fb820).
    const u32 op = fe::direct<0x50e5a0, watb::queueFetch>(app, ts, record + kSlotQueue, 0x4fb820, id);
    if (op == 0)
        return 0;
    guest.setByte(op + 0x11, std::uint8_t(priority));
    return guest::direct<0x4fbb60, fileOpQueue>(app, ts, op);
}

/* sub_4fca60: an operation to open a file. */
nfs::u32 frontend::wina::fileOpOpen(App* app, ThreadState& ts, u32 name, u32 mode, u32 priority, u32 target)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    // sub_4fce50 keeps every register but eax: the priority and the target are still in edx and ebx.
    const u32 slot = guest::direct<0x4fce50, fileSlotOfName>(app, ts, name);
    const u32 op = guest::direct<0x4fb990, fileOpAllocate>(app, ts, 0, priority, target, slot);
    guest::direct<0x4fbbe0, fileOpSetName>(app, ts, op, name);
    guest.setDword(op + 0x24, mode);
    return guest::direct<0x4fbb60, fileOpQueue>(app, ts, op);
}

/* sub_4fca90: an operation to close a file. */
nfs::u32 frontend::wina::fileOpClose(App* app, ThreadState& ts, u32 handle, u32 priority, u32 target)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 slot = guest::direct<0x4e4320, fileLockOf>(app, ts, handle);
    const u32 op = guest::direct<0x4fb990, fileOpAllocate>(app, ts, 1, priority, target, slot);
    guest.setDword(op + 0x18, handle);
    return guest::direct<0x4fbb60, fileOpQueue>(app, ts, op);
}

/* sub_4fcab0: an operation to read a file. */
nfs::u32 frontend::wina::fileOpRead(App* app, ThreadState& ts, u32 handle, u32 offset, u32 buffer, u32 size,
                                      u32 priority, u32 target)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    const u32 frame = guest.frame();
    guest.setDword(frame, size);
    // The game's code hands it the priority and the target in edx and ebx too, which it keeps there for the
    // next call.
    const u32 slot = guest::direct<0x4e4320, fileLockOf>(app, ts, handle);
    const u32 op = guest::direct<0x4fb990, fileOpAllocate>(app, ts, 2, priority, target, slot);
    guest.setDword(op + 0x18, handle);
    guest.setDword(op + 0x24, offset);
    guest.setDword(op + 0x2c, buffer);
    guest.setDword(op + 0x28, guest.dword(frame));
    return guest::direct<0x4fbb60, fileOpQueue>(app, ts, op);
}

/* sub_4fce50: the slot of a file's name: 0 when it has a '|', else by its drive. */
nfs::u32 frontend::wina::fileSlotOfName(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    for (u32 at = name;; ++at)
    {
        safepoint(app, ts);
        const std::uint8_t c = guest.byte(at);
        if (c == '|')
            return 0;
        if (c == 0)
            break;
    }
    return u32(*guest::direct<0x4e3330, ea::driveNumberOf>(app, ts, name));
}

/* sub_4fd060: a file closed and waited for. */
nfs::u32 frontend::wina::fileCloseWait(App* app, ThreadState& ts, u32 handle, u32 priority)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 op = guest::direct<0x4fca90, fileOpClose>(app, ts, handle, priority, 0);
    if (op == 0)
        return 0;
    // The operation in eax as the close leaves it; then in edx, which sub_4fc780 keeps.
    fe::direct<0x4fc780, menz::fileOpWait>(app, ts, op);
    return fe::direct<0x4fc600, menz::fileOpComplete>(app, ts, op);
}

/* sub_4e4230: a file's size, 0 when the handle is not of an open one or it keeps no
 * system handle; the file pointer put where asked.  Under the table's own lock. */
nfs::u32 frontend::wina::fileSizeAndSeek(App* app, ThreadState& ts, u32 handle, u32 offset)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kTable = 0x5645a0, kCount = 0x56459c, kLocks = 0x9ef760;
    const u32 index = ~handle;
    const u32 table = guest.dword(kTable);
    if (table == 0 || s32(index) < 0 || s32(index) >= s32(guest.dword(kCount)))
        return 0;
    const u32 place = index << 5;
    if (guest.byte(table + place) == 0)
        return 0;
    const u32 lockIndex = guest.byte(table + place + 1);
    enterLock(guest, ts, guest.dword(lockIndex * 4 + kLocks));
    const u32 entry = guest.dword(kTable) + place;
    u32 size = 0;
    if (guest.dword(entry + 0x14) == 0)
    {
        size = os::files::truncate(app, guest.dword(entry + 4)) ? 1 : 0;
        const u32 moved = os::files::seek(app, guest.dword(guest.dword(kTable) + place + 4), s32(offset),
                                          os::files::From::start)
                              .value_or(0xffffffff);
        guest.setDword(guest.dword(kTable) + place + 0x18, moved);
    }
    leaveLock(guest, ts, guest.dword(lockIndex * 4 + kLocks));
    return size;
}

/* sub_4e4320: which lock of the table an open file's handle has; 0 when none. */
nfs::u32 frontend::wina::fileLockOf(App* app, ThreadState& ts, u32 handle)
{
    Guest guest(app, ts);
    const u32 index = ~handle;
    if (guest.dword(0x5645a0) == 0 || s32(index) < 0 || s32(index) >= s32(guest.dword(0x56459c)))
        return 0;
    const u32 entry = guest.dword(0x5645a0) + (index << 5);
    if (guest.byte(entry) == 0)
        return 0;
    return guest.byte(entry + 1);
}

/* ---------------------------------------------------------------------------
 * The window's messages
 * ------------------------------------------------------------------------- */

/* sub_4cc650: two handler rows, by their message: for qsort and bsearch. */
nfs::u32 frontend::wina::compareHandlers(App* app, ThreadState& ts, u32 leftRow, u32 rightRow)
{
    Guest guest(app, ts);
    const u32 right = guest.dword(rightRow);
    const u32 left = guest.dword(leftRow);
    return left - right;
}

/* sub_4cc660: a message's handler, 0 when it has none. */
nfs::u32 frontend::wina::handlerOf(App* app, ThreadState& ts, u32 message)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    const u32 frame = guest.frame();
    messageLockMade(guest, ts);
    const u32 lock = guest.dword(kMessageLock);
    enterLock(guest, ts, lock);
    guest.setDword(frame, message);
    const u32 row = fe::direct<kBsearch, watcom::clib::bsearch>(app, ts, frame, kHandlers, guest.dword(kHandlerCount), 8u, kCompareHandlers);
    const u32 handler = row != 0 ? guest.dword(row + 4) : 0;
    leaveLock(guest, ts, guest.dword(kMessageLock));
    return handler;
}

/* sub_4cc6d0: a message's handler set, changed, or taken away with 0; the rows stay
 * sorted.  1 when done, 0 when the table is full or it had none to take away. */
nfs::u32 frontend::wina::handlerSet(App* app, ThreadState& ts, u32 message, u32 handler)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    constexpr u32 kMostHandlers = 0x80, kPastEnd = 0x7fffffff;
    const u32 frame = guest.frame();
    messageLockMade(guest, ts);
    const u32 lock = guest.dword(kMessageLock);
    enterLock(guest, ts, lock);
    guest.setDword(frame, message);
    const u32 row = fe::direct<kBsearch, watcom::clib::bsearch>(app, ts, frame, kHandlers, guest.dword(kHandlerCount), 8u, kCompareHandlers);
    u32 done = 0;
    if (row != 0)
    {
        if (handler == 0)
        {
            const u32 count = guest.dword(kHandlerCount);
            guest.setDword(row, kPastEnd);
            fe::direct<kQsort, watcom::clib::qsort>(guest.app(), guest.ts(), kHandlers, count, 8, kCompareHandlers);
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
        const u32 count = guest.dword(kHandlerCount);
        if (count < kMostHandlers)
        {
            done = 1;
            guest.setDword(count * 8 + kHandlers, message);
            guest.setDword(count * 8 + kHandlers + 4, handler);
            guest.setDword(kHandlerCount, count + 1);
            fe::direct<kQsort, watcom::clib::qsort>(guest.app(), guest.ts(), kHandlers, count + 1, 8, kCompareHandlers);
        }
    }
    const u32 lockAgain = guest.dword(kMessageLock);
    leaveLock(guest, ts, lockAgain);
    return done;
}

/* sub_4cc7c0: a message to the game's window, sent or posted; 1 when it went. */
nfs::u32 frontend::wina::postMessage(App* app, ThreadState& ts, u32 message, u32 lParam, u32 wParam, u32 send)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    const u32 hwnd = guest.dword(kHwnd);
    if (hwnd == 0)
        return 0;
    /* Sent, the message would be handled before this returns: the port has no sending, so a
     * message to send is dropped -- the window's procedure never sees it -- and said to have gone. */
    if (send == 0)
        os::window::post(hwnd, message, wParam, lParam);
    return 1;
}

/* sub_4cc860: who is told when the window gets or loses the focus. */
nfs::u32 frontend::wina::setFocusCallback(App* app, ThreadState& ts, u32 callback)
{
    Guest guest(app, ts);
    guest.setDword(kFocusCallback, callback);
    return callback;
}

/* sub_4cc870: the system task that leaves the game when there is no window. */
nfs::u32 frontend::wina::noWindowTask(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, ts, 0x4cc870);
    fe::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
    return 0;
}

/* sub_4cc890: the system's cursor put at a place of the window.  The port has no system cursor --
 * the game's own pointer is all there is -- so nothing is moved; 0, as the cursor set after it
 * answers. */
nfs::u32 frontend::wina::movePointer(App* app, ThreadState& ts, u32 window, u32 x, u32 y)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    NFS2_USE(window);
    NFS2_USE(x);
    NFS2_USE(y);
    return 0;
}

/* sub_4cc8e0: a key on the queue, the oldest dropped when it is full. */
nfs::u32 frontend::wina::pushKey(App* app, ThreadState& ts, u32 key, u32 scan, u32 flags)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    guest.setDword(guest.frame(), scan);
    const u32 tail = guest.dword(kKeyTail);
    const u32 extended = (flags & 0xffff00ffu) | ((scan & 0xff) << 8);
    const u32 lock = guest.dword(kKeyLock);
    enterLock(guest, ts, lock);
    guest.setByte(tail * 4 + kKeyQueue + 1, std::uint8_t(extended >> 8));
    guest.setByte(tail * 4 + kKeyQueue, std::uint8_t(extended));
    const u32 room = guest.dword(kKeyRoom);
    std::int64_t next = s32(guest.dword(kKeyTail) + 1);
    u32 quotient = u32(next / s32(room));
    guest.setDword(kKeyTail, u32(next % s32(room)));
    u32 left = guest.dword(kKeyHead);
    const u32 newTail = guest.dword(kKeyTail);
    guest.setWord(tail * 4 + kKeyQueue + 2, std::uint16_t(key));
    if (left == newTail)
    {
        next = s32(guest.dword(kKeyHead) + 1);
        quotient = u32(next / s32(room));
        left = u32(next % s32(room));
        guest.setDword(kKeyHead, left);
    }
    const u32 lockAgain = guest.dword(kKeyLock);
    fe::leaveLock(guest, ts, lockAgain);
    return quotient;
}

/* sub_4ccaa0: the key code of the arrows and the function keys, from a key-down message's
 * virtual key; 0 for anything else.  The game's code's jumps are one table. */
nfs::u32 frontend::wina::keyOfMessage(App*, ThreadState&, u32 message, u32 virtualKey)
{
    constexpr u32 kKeyDown = 0x100, kSystemKeyDown = 0x104;
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
 * keys told to the game instead of the system.  The port sets no system hook (onSetFocus,
 * hookOn), so it is called only by address. */
nfs::u32 frontend::wina::keyboardHook(App* app, ThreadState& ts, u32 code, u32 wParam, u32 lParam)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kVkTab = 9, kVkEscape = 0x1b, kVkControl = 0x11;
    u32 swallowed = 0, modifier = 0, key = 0;  // the callback is told only where key is set
    if (code == 0 && guest.dword(kActive) != 0)
    {
        if (guest.dword(kHookAlt) != 0 && (std::uint8_t(lParam >> 24) & 0x20) != 0)
        {
            modifier = 0x38;
            if (wParam == kVkTab)
            {
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
            if (os::window::keyDown(kVkControl))
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
                modifier = 0;
            }
        }
        if (swallowed != 0 && guest.dword(kHookCallback) != 0)
            swallowed = fe::pointerCall(guest, guest.dword(kHookCallback), args({ modifier, key }));
    }
    if (swallowed != 0)
        return 1;
    // The next hook's answer: the port has no other hook, so 0.
    return 0;
}

/* sub_4ccd60: the window has the focus: hook and callback. */
nfs::u32 frontend::wina::onSetFocus(App* app, ThreadState& ts, u32 window, u32 hwnd, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    if (window != 0)
    {
        if (hwnd == guest.dword(kHwnd))
        {
            const u32 wanted = guest.dword(kHookWanted);
            guest.setDword(kActive, 1);
            /* The keyboard hook (kKeyboardHook) set: the port has no system hook to set -- Alt+Tab
             * and the Windows keys never reach a system -- so its handle stays 0. */
            if (wanted != 0 && guest.dword(kHookHandle) == 0)
                guest.setDword(kHookHandle, 0);
            if (guest.dword(kFocusCallback) != 0)
                fe::pointerCall(guest, guest.dword(kFocusCallback), args({ 1 }));
        }
    }
    return 0;
}

/* sub_4ccdd0: the window has lost the focus: the hook off, the keys let go. */
nfs::u32 frontend::wina::onKillFocus(App* app, ThreadState& ts, u32, u32 hwnd, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    if (hwnd == guest.dword(kHwnd))
    {
        guest.setDword(kActive, 0);
        // The keyboard hook taken off: the port sets none (onSetFocus), its handle is 0.
        if (guest.dword(kFocusCallback) != 0)
            fe::pointerCall(guest, guest.dword(kFocusCallback), args({ 0 }));
        // The system's cursor shown again: the port has none, the game's own pointer is all there is.
        fe::direct<0x4e070c, ea::clearBytes>(guest.app(), guest.ts(), kKeysDown, 0x80);
    }
    return 0;
}

/* sub_4cce40: WM_CLOSE. */
nfs::u32 frontend::wina::onClose(App* app, ThreadState& ts, u32 window, u32, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    if (window != 0)
        os::window::quit(app->getMemory<u32>(ts.sp));
    return 1;
}

/* sub_4cce60: the game's own message to close the window: the thread does it. */
nfs::u32 frontend::wina::onQuitAsked(App* app, ThreadState& ts, u32 window, u32, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    if (window != 0)
        guest.setDword(kQuitAsked, 1);
    return 1;
}

/* sub_4cce80: a message said to be handled, and no more. */
nfs::u32 frontend::wina::onDestroy(App*, ThreadState&, u32, u32, u32, u32, u32, u32)
{
    return 0;
}

/* sub_4cce90: WM_SYSCOMMAND: closing, sizing and the screen saver are nobody's in a
 * fullscreen window. */
nfs::u32 frontend::wina::onSysCommand(App* app, ThreadState& ts, u32 window, u32, u32, u32 wParam, u32, u32)
{
    Guest guest(app, ts);
    const u32 command = wParam;
    if (command == 0xf060 || command == 0xf020 || command == 0xf030 || command == 0xf120)
        return 0;
    return guest.byte(window + kWindowFullscreen);
}

/* sub_4cced0: WM_SETTINGCHANGE: in fullscreen, they are turned off again. */
nfs::u32 frontend::wina::onSettingChange(App* app, ThreadState& ts, u32 window, u32, u32, u32, u32, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    if (window != 0 && guest.byte(window + kWindowFullscreen) != 0)
        accessKeysOff(guest, ts);
    guest.setDword(result, 0);
    return 0;
}

/* sub_4ccf50: the window was moved or sized: its place kept, the resize told. */
nfs::u32 frontend::wina::onMoved(App* app, ThreadState& ts, u32 window, u32 hwnd, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    if (window != 0 && guest.byte(window + kWindowFullscreen) == 0)
    {
        const os::window::Area client = os::window::clientArea(guest.dword(window + kWindowHandle));
        guest.setDword(window + kWindowX, u32(client.left));
        guest.setDword(window + kWindowY, u32(client.top));
        if (guest.dword(kOnResize) != 0)
            fe::pointerCall(guest, guest.dword(kOnResize));
    }
    return 0;
}

/* sub_4ccfc0: WM_CHAR: the character on the key queue, as many times as it repeats. */
nfs::u32 frontend::wina::onChar(App* app, ThreadState& ts, u32 window, u32, u32, u32 wParam, u32 lParam, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    const u32 character = wParam, flags = lParam;
    if (window != 0)
    {
        s32 count = s32(flags & 0xffff);
        const s32 room = s32(guest.dword(kKeyRoom));
        if (count > room)
            count = room;
        for (--count; count != -1; --count)
        {
            safepoint(app, ts);
            guest::direct<0x4cc8e0, pushKey>(app, ts, character, (flags >> 16) & 0x7f, (flags >> 24) & 1);
        }
    }
    return 1;
}

/* sub_4cd020: WM_KEYDOWN: the key down in the table, and on the queue when it is one
 * the game asks for by scan code or an arrow or function key. */
nfs::u32 frontend::wina::onKeyDown(App* app, ThreadState& ts, u32 window, u32, u32 message, u32 wParam, u32 lParam,
                                     u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    const u32 virtualKey = wParam, flags = lParam;
    if (window == 0)
        return 1;
    s32 count = s32(flags & 0xffff);
    const s32 room = s32(guest.dword(kKeyRoom));
    if (count > room)
        count = room;
    guest.setDword(kLastScan, (flags >> 16) & 0x7f);
    const u32 table = guest.dword(kScanTable);
    if (table != 0)
        guest.setDword(kLastScan, guest.byte(table + guest.dword(kLastScan)));
    guest.setByte(guest.dword(kLastScan) + kKeysDown, 1);
    u32 key = 0;
    if (guest.dword(kWantRaw) == 0)
    {
        key = guest::direct<0x4ccaa0, keyOfMessage>(app, ts, message, virtualKey);
        if (key == 0)
            return 0;
    }
    for (--count; count != -1; --count)
    {
        safepoint(app, ts);
        guest::direct<0x4cc8e0, pushKey>(app, ts, key, guest.dword(kLastScan), (flags >> 24) & 1);
    }
    return 1;
}

/* sub_4cd0e0: WM_KEYUP: the key up in the table; the two shifts go together. */
nfs::u32 frontend::wina::onKeyUp(App* app, ThreadState& ts, u32 window, u32, u32, u32, u32 lParam, u32)
{
    Guest guest(app, ts);
    if (window != 0)
    {
        u32 scan = (lParam >> 16) & 0x7f;
        const u32 table = guest.dword(kScanTable);
        if (table != 0)
        {
            scan = guest.byte(table + scan);
        }
        if (scan == 0x2a || scan == 0x36)
        {
            guest.setByte(kKeysDown + 0x2a, 0);
            guest.setByte(kKeysDown + 0x36, 0);
        }
        else
        {
            guest.setByte(scan + kKeysDown, 0);
        }
    }
    return 0;
}

/* sub_4cd130: a mouse button pressed. */
nfs::u32 frontend::wina::onMouseDown(App* app, ThreadState& ts, u32 window, u32, u32, u32 wParam, u32 lParam,
                                       u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    pointerMessage(guest, ts, window, wParam, lParam, 1);
    return 0;
}

/* sub_4cd190: a mouse button let go, or the mouse moved. */
nfs::u32 frontend::wina::onMouseUp(App* app, ThreadState& ts, u32 window, u32, u32, u32 wParam, u32 lParam, u32)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    pointerMessage(guest, ts, window, wParam, lParam, 0);
    return 0;
}

/* sub_4cd1f0: WM_PAINT: painted with nothing, under the graphics' lock.  The port never asks its
 * window to be painted, and its window is always in front: the system's painting is nothing. */
nfs::u32 frontend::wina::onPaint(App* app, ThreadState& ts, u32 window, u32 hwnd, u32, u32, u32, u32 result)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 80);
    NFS2_USE(hwnd);
    if (window != 0 && guest.dword(window + kWindowHandle) != 0)
    {
        enterLock(guest, ts, guest.dword(kGraphicsLock));
        const u32 lock = guest.dword(kGraphicsLock);
        leaveLock(guest, ts, lock);
        if (guest.dword(kOnResize) != 0 && guest.byte(window + kWindowFullscreen) == 0)
            fe::pointerCall(guest, guest.dword(kOnResize));
        guest.setDword(result, 1);
    }
    return 1;
}

/* sub_4cd2a0: WM_ERASEBKGND of the fullscreen window: said to be done. */
nfs::u32 frontend::wina::onEraseBackground(App* app, ThreadState& ts, u32 window, u32 hwnd, u32, u32, u32, u32)
{
    Guest guest(app, ts);
    if (window != 0)
    {
        if (hwnd == guest.dword(window + kWindowHandle) && guest.byte(window + kWindowFullscreen) != 0)
            return 1;
    }
    return 0;
}

/* sub_4cd2d0: the window's procedure: the table's handler, else the system's. */
nfs::u32 frontend::wina::windowProc(App* app, ThreadState& ts, u32 hwnd, u32 message, u32 wParam, u32 lParam)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    const u32 frame = guest.frame();
    guest.setDword(frame, message);
    guest.setDword(frame + 8, 0);
    const u32 row = fe::direct<kBsearch, watcom::clib::bsearch>(app, ts, frame, kHandlers, guest.dword(kHandlerCount), 8u, kCompareHandlers);
    u32 handled = 0;
    if (row != 0)
    {
        handled = fe::pointerCall(guest, guest.dword(row + 4), args({}, { kWindow, hwnd, message, wParam, lParam, frame + 8 }));
    }
    if (handled != 0)
        return guest.dword(frame + 8);
    // What no handler takes is the port's: 0, and the window's destroy lets the window go.
    if (message == kDestroyMessage)
        os::release(app, hwnd);
    return 0;
}

/* The port's event given to the window's procedure as the game's message in its table: a key as
 * its key-down or key-up, a character as its character message, each with its virtual key or
 * character first and then, as the handlers read it, the times it counts in the low half and the
 * scan code above; the window let go as its destroy; the game's own messages as it posted them. */
nfs::u32 frontend::wina::dispatchEvent(App* app, ThreadState& ts, const os::window::Event& event)
{
    using Kind = os::window::Event::Kind;
    u32 message = event.message, first = event.first, second = event.second;
    switch (event.kind)
    {
    case Kind::key:
        message = event.down ? kKeyDownMessage : kKeyUpMessage;
        first = event.key;
        second = event.repeats | (event.scan << 16);
        break;
    case Kind::character:
        message = kCharMessage;
        first = event.character;
        second = event.repeats | (event.scan << 16);
        break;
    case Kind::closed:
        message = kDestroyMessage;
        first = 0;
        second = 0;
        break;
    case Kind::message:
        break;
    case Kind::stop:
        return 0;
    }
    return guest::direct<0x4cd2d0, windowProc>(app, ts, event.window, message, first, second);
}

/* sub_4cd350: the window created, fullscreen or framed; its handle.  With none it stays
 * as it is; the arrows and such of the scan table's three keyboards are set up. */
nfs::u32 frontend::wina::createWindow(App* app, ThreadState& ts, u32 window)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 44);
    constexpr u32 kTitle = 0x5612a4;
    u32 width, height;
    if (guest.byte(window + kWindowFullscreen) != 0)
    {
        guest.setDword(window + kWindowX, 0);
        guest.setDword(window + kWindowY, 0);
        width = u32(os::window::kScreenWidth);
        height = u32(os::window::kScreenHeight);
    }
    else
    {
        /* Framed: where it was, or a new one where the system puts it, its client area the size
         * asked.  The port's window has no frame round its client area to make room for. */
        const bool placed = guest.dword(window + kWindowHandle) != 0;
        guest.setDword(window + kWindowX, placed ? guest.dword(window + kWindowX) : 0xffffe000);
        guest.setDword(window + kWindowY, placed ? guest.dword(window + kWindowY) : 0xffffe000);
        width = guest.dword(window + 0x14);
        height = guest.dword(window + 0x18);
    }
    const u32 created =
        os::window::open(app, guest::text(app, guest.dword(kTitle)), std::int32_t(width), std::int32_t(height));
    if (created != 0)
    {
        // The system's cursor hidden: the port has none, the game's own pointer is all there is.
        guest.setDword(0x564814, 0);
        if (guest.dword(kScanTable) == 0)
        {
            fe::direct<0x4fb710, wata::lockKeySet>(app, ts, 0, 0);
            fe::direct<0x4fb710, wata::lockKeySet>(app, ts, 1, 0);
            fe::direct<0x4fb710, wata::lockKeySet>(app, ts, 2, 0);
        }
    }
    return created;
}

/* sub_4cd4e0: the window's thread: makes the window, tells who waits, pumps its
 * messages until it is gone. */
nfs::u32 frontend::wina::windowThread(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 68);
    const u32 hwnd = guest::direct<0x4cd350, createWindow>(app, ts, kWindow);
    guest.setDword(kWindowReady, u32(*fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts)));
    // The event in eax as the call before leaves it.
    fe::direct<0x4dfb70, ea::eventWaited>(app, ts, guest.dword(kWindowReady));
    directKept<0x4dfc00, ea::eventClosed, 1>(guest.app(), guest.ts(), guest.dword(kWindowReady), 0);
    const std::uint8_t fullscreen = guest.byte(kFullscreen);
    guest.setDword(kHwnd, hwnd);
    const u32 zero = 0;  // edx as the eventClosed call loads it
    guest.setDword(kQuitAsked, zero);
    guest.setDword(kWindowReady, zero);
    // The client area's place on the screen; the window brought to the front and made active is
    // what the port's window always is.
    if (fullscreen == 0)
    {
        const os::window::Area client = os::window::clientArea(hwnd);
        guest.setDword(0x9f3d34, u32(client.left));
        guest.setDword(0x9f3d38, u32(client.top));
    }
    else
    {
        guest.setDword(0x9f3d34, 0);
        guest.setDword(0x9f3d38, 0);
    }
    guest.setDword(kActive, 1);
    for (;;)
    {
        safepoint(app, ts);
        if (hwnd != guest.dword(kHwnd))
            break;
        /* The port's events, as the window's messages were: the characters a key types come from
         * the port as events of their own, so nothing turns keys into them here. */
        const os::window::Event event = os::window::wait(app, ts);
        if (ts.terminate)
            throw fe::Terminated();
        if (event.kind != os::window::Event::Kind::stop)
        {
            enterLock(guest, ts, guest.dword(kMessageLock));
            dispatchEvent(app, ts, event);
            leaveLock(guest, ts, guest.dword(kMessageLock));
        }
        else if (guest.dword(0x56129c) != 0)
        {
            fe::pointerCall(guest, guest.dword(0x56129c));
        }
        else
        {
            // By address, as sub_4f9070 below: the thread's answer is what the loop's last call left in eax.
            fe::direct<0x4e7520, port::stream::scheduleTask>(app, ts, ts.sp - 4, 0x4cc870u, 1, 10u);
        }
        if (guest.dword(kQuitAsked) != 0)
        {
            const u32 surface = guest.dword(kWindow);
            guest.setDword(kQuitAsked, 0);
            if (surface != 0 && guest.dword(0x9f38e0) != 0)
                fe::direct<0x4ecd50, eab::ddrawDisposeDisplay>(app, ts, kWindow);
            os::window::close(guest.dword(kHwnd));
            guest.setDword(kHwnd, 0);
        }
        /* In fullscreen, the system's cursor hidden while the game has the focus, else the window
         * brought back to the front unless it is minimised: the port has no system cursor, and its
         * window is never minimised and always in front, so there is nothing to do. */
    }
    return 0;  // the window thread's exit code, which nothing reads
}

/* sub_4cd6c0: the game's window, opened or changed for a mode: the class and the
 * handlers the first time, its thread each time; its handle. */
nfs::u32 frontend::wina::openWindow(App* app, ThreadState& ts, u32 width, u32 height, u32 isFullscreen)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 84);
    static constexpr struct { u32 message, handler; } kHandlersOfWindow[] = {
        { 7, 0x4ccd60 }, { 8, 0x4ccdd0 }, { 5, 0x4ccf50 }, { 3, 0x4ccf50 }, { 0x10, 0x4cce40 }, { 2, 0x4cce80 },
        { 0xf, 0x4cd1f0 }, { 0x14, 0x4cd2a0 }, { 0x1a, 0x4cced0 }, { 0x112, 0x4cce90 }, { 0x466, 0x4cce60 },
        { 0x102, 0x4ccfc0 }, { 0x100, 0x4cd020 }, { 0x104, 0x4cd020 }, { 0x101, 0x4cd0e0 }, { 0x105, 0x4cd0e0 },
        { 0x201, 0x4cd130 }, { 0x207, 0x4cd130 }, { 0x204, 0x4cd130 }, { 0x203, 0x4cd130 }, { 0x209, 0x4cd130 },
        { 0x206, 0x4cd130 }, { 0x202, 0x4cd190 }, { 0x208, 0x4cd190 }, { 0x205, 0x4cd190 }, { 0x200, 0x4cd190 },
    };
    constexpr u32 kClassMade = 0x7f7490, kClassName = 0x5612a4;
    constexpr u32 kWidth = 0x9f38e4, kHeight = 0x9f38e8, kThreadRecord = 0x7f7460;
    const u32 frame = guest.frame();
    const u32 thread = frame + 0x28;
    if (guest.dword(kMessageLock) == 0)
        guest.setDword(kMessageLock, *fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    if (guest.byte(kClassMade) == 0)
    {
        guest.setDword(kInstance, os::kProgramModule);
        if (guest.dword(kClassName) == 0)
        {
            const u32 names = guest.dword(0x9f38c0);
            guest.setDword(kClassName, names != 0 ? guest.dword(names) : 0x5612ac);
        }
        /* The window's class registered, its procedure 0x4cd2d0: the port keeps no classes -- the
         * window's thread hands the port's events to that procedure itself (dispatchEvent). */
        guest.setDword(kKeyLock, *fe::direct<0x4eb2e0, ea::newLock>(app, ts));
        guest.setDword(kKeyRoom, 0x20);
        guest.setDword(kKeyHead, 0);
        guest.setDword(kKeyTail, 0);
        guest.setByte(kClassMade, 1);
        for (const auto& row : kHandlersOfWindow)
            guest::direct<0x4cc6d0, handlerSet>(app, ts, row.message, row.handler);
    }
    else if (guest.dword(kHwnd) != 0 && isFullscreen != guest.byte(kFullscreen))
    {
        guest::direct<0x4cdbd0, centreWindow>(app, ts, width, height);
        return 1;
    }
    guest.setByte(kFullscreen, std::uint8_t(isFullscreen));
    guest.setDword(kWidth, width);
    guest.setDword(kActive, 1);
    guest.setDword(kHeight, height);
    if (*fe::direct<0x4df7a0, ea::threadStarted>(app, ts, 0x4cd4e0u, 0u, 1u, thread) == 0)
        return 0;
    while (guest.dword(kWindowReady) == 0)
    {
        safepoint(app, ts);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    }
    if (guest.dword(kHwnd) != 0)
        guest::direct<0x4cdaa0, closeWindow>(app, ts);
    // The event stays in edx after the call, as the game's code has it, for the end below when the wait does not
    // run; the thread it hands in eax is not read.
    const u32 ready = guest.dword(kWindowReady);
    fe::direct<0x4dfb20, ea::eventPosted>(app, ts, ready);
    while (guest.dword(kWindowReady) != 0)
    {
        safepoint(app, ts);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
    }
    const std::uint8_t fullscreen = guest.byte(kFullscreen);
    copyDwords(guest, kThreadRecord, thread, 5);
    if (fullscreen != 0)
    {
        accessKeysOff(guest, ts);
    }
    else
    {
        // Brought to the front, as the port's window always is, and centred.
        guest::direct<0x4cdbd0, centreWindow>(app, ts, width, height);
        guest.setDword(kStickyKeys, 0);
        guest.setDword(kFilterKeys, 0);
    }
    return guest.dword(kHwnd);
}

/* sub_4cdaa0: the window closed: the keys the system had put back, and waited for. */
nfs::u32 frontend::wina::closeWindow(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    if (guest.dword(kHwnd) == 0)
        return 0;
    // The screen saver and the power-off timer turned back on if they were: the port has neither
    // (accessKeysOff), so they never were.
    u32 left = guest::direct<0x4cc7c0, postMessage>(app, ts, 0x466, 0, 0, 0);  // what the calls leave in eax
    while (guest.dword(kHwnd) != 0)
    {
        safepoint(app, ts);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
        left = u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0));
    }
    return left;
}

/* sub_4cdb10: the window put at a place with a size of client area; what came kept.  The port's
 * window is the screen: it is neither moved nor sized, and what comes is its client area. */
nfs::u32 frontend::wina::placeWindow(App* app, ThreadState& ts, u32 left, u32 top, u32 width, u32 height)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 28);
    NFS2_USE(left);
    NFS2_USE(top);
    NFS2_USE(width);
    NFS2_USE(height);
    const os::window::Area client = os::window::clientArea(guest.dword(kHwnd));
    guest.setDword(0x9f38e4, u32(client.right - client.left));
    guest.setDword(0x9f3d34, u32(client.left));
    guest.setDword(0x9f3d38, u32(client.top));
    const u32 clientHeight = u32(client.bottom - client.top);
    guest.setDword(0x9f38e8, clientHeight);
    return clientHeight;
}

/* sub_4cdbd0: the window centred on the screen. */
nfs::u32 frontend::wina::centreWindow(App* app, ThreadState& ts, u32 width, u32 height)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    const s32 spareY = s32(u32(os::window::kScreenHeight) - height);
    const u32 top = u32(spareY / 2);
    const s32 spareX = s32(u32(os::window::kScreenWidth) - width);
    const u32 left = u32(spareX / 2);
    return guest::direct<0x4cdb10, placeWindow>(app, ts, left, top, width, height);
}

/* sub_4cdc50: the keyboard hook off, in fullscreen. */
nfs::u32 frontend::wina::hookOff(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    // The port sets no hook (hookOn): its handle is 0, so there is none to take off.
    NFS2_USE(guest);
    return 0;
}

/* sub_4cdc90: the keyboard hook on, with who is told. */
nfs::u32 frontend::wina::hookOn(App* app, ThreadState& ts, u32 alt, u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    guest.setDword(kHookCallback, callback);
    const u32 hook = guest.dword(kHookHandle);
    guest.setDword(kHookAlt, alt);
    if (hook == 0)
    {
        /* The keyboard hook (kKeyboardHook) set: the port has no system hook to set -- Alt+Tab and
         * the Windows keys never reach a system -- so its handle stays 0, and so does the answer. */
        guest.setDword(kHookHandle, 0);
        guest.setDword(kHookWanted, 1);
        return 0;
    }
    return 0;
}

/* sub_4fb470: the mouse: the place held inside the clip, the system's cursor put back
 * when it was outside, the place, buttons and the click kept under the table's lock. */
nfs::u32 frontend::wina::pointerMoved(App* app, ThreadState& ts, u32 x, u32 y, u32 buttons, u32 click)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    constexpr u32 kClipLeft = 0x9f4f4c, kClipTop = 0x9f4f50, kClipRight = 0x9f4f44, kClipBottom = 0x9f4f40;
    constexpr u32 kButtons = 0x9f4f54, kLastX = 0x9f4f3c, kLastY = 0x9f4f38;
    const u32 frame = guest.frame();
    guest.setDword(frame + 4, buttons);
    guest.setDword(frame, click);
    if (guest.dword(0x564368) == 0 || guest.dword(kMessageLock) == 0)
        return x;

    const s32 left = s32(guest.dword(kClipLeft));
    u32 heldX = guest.dword(kClipRight);
    if ((s32(x) > left ? s32(x) : left) < s32(heldX))
        heldX = s32(x) <= left ? u32(left) : x;
    const s32 top = s32(guest.dword(kClipTop));
    const u32 bottom = guest.dword(kClipBottom);
    u32 heldY = u32(s32(y) > top ? s32(y) : top);
    if (s32(heldY) >= s32(bottom))
        heldY = bottom;
    enterLock(guest, ts, guest.dword(kMessageLock));
    guest.setDword(frame + 8, heldY);
    guest.setDword(kButtons, guest.dword(frame + 4));
    if (heldX != x || heldY != y)
        guest::direct<0x4cc890, movePointer>(app, ts, kWindow, heldX, guest.dword(frame + 8));
    const u32 kept = guest.dword(frame + 8);
    if (heldX != guest.dword(kLastX) || kept != guest.dword(kLastY))
    {
        guest.setDword(kLastX, heldX);
        guest.setDword(kLastY, kept);
    }
    const u32 lock = guest.dword(kMessageLock);
    fe::leaveLock(guest, ts, lock);
    u32 result = kept;
    if (guest.dword(frame) != 0 && guest.dword(0x56446c) != 0)
        result = fe::pointerCall(guest, guest.dword(0x56446c), args({ guest.dword(frame + 4) }));
    return result;
}

/* ---------------------------------------------------------------------------
 * Startup checks
 * ------------------------------------------------------------------------- */

/* sub_4a38c0: whether the game's window is up already. */
nfs::u32 frontend::wina::alreadyRunning(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    // A window of the game's class is looked for: the port runs one game, so there is none.
    return 0;
}

/* sub_4a38f0: the file version of the game's exe, its text into the buffer given; 1 when
 * it has one.  The version block (the port's: os::kVersionBytes) is copied into memory of the
 * game's, the value taken from there: the FileVersion string, or for a key the block has no
 * value of, the block itself and the length the frame's word holds. */
nfs::u32 frontend::wina::versionText(App* app, ThreadState& ts, u32 buffer, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x118);
    constexpr u32 kVersionKey = 0x53ce2c;
    const u32 frame = guest.frame();
    const u32 valueLength = frame + 0x108;
    guest.setDword(frame + 0x10c, buffer);
    guest.setDword(frame + 0x114, length);
    guest.setDword(frame + 0x110, 0);
    const u32 memory = fe::direct<0x4f7900, watcom::clib::malloc>(guest.app(), guest.ts(), os::kVersionBytes);
    if (memory == 0)
        return guest.dword(frame + 0x110);
    // The block as the port lays it out: a word, then the version string in the rest.
    guest.setDword(memory, 0x02000000);
    for (u32 i = os::kVersionText; i < os::kVersionBytes; ++i)
        guest.setByte(memory + i, 0);
    guest::putText(app, memory + os::kVersionText, os::fileVersion().c_str());
    u32 value = memory, count = guest.dword(valueLength);
    if (const auto version = os::versionValue(guest::text(app, kVersionKey)))
    {
        value = memory + os::kVersionText;
        count = u32(version->size() + 1);
    }
    if (count != 0)
    {
        guest.setDword(frame + 0x110, 1);
        if (count >= guest.dword(frame + 0x114))
            count = guest.dword(frame + 0x114);
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), guest.dword(frame + 0x10c), value, count);
    }
    fe::direct<0x4f79f0, watcom::clib::free>(guest.app(), guest.ts(), memory);
    return guest.dword(frame + 0x110);
}

/* sub_4b56d0: an import's answer, no more. */
nfs::u32 frontend::wina::tickCount(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    return os::time::ticks(guest.app(), ts);
}

/* sub_4b5860: DirectX 5 and a sound or graphics library there, or the game says so in
 * the player's language and leaves. */
nfs::u32 frontend::wina::checkLibrary(App* app, ThreadState& ts, u32 library, u32 language)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x23c);
    constexpr u32 kNeedsDirectX = 0x4b5580, kNoLibrary = 0x4b5598, kLibraries = 0x55fd68, kTitle = 0x53ec28;
    const u32 frame = guest.frame();
    const u32 text = frame + 0x100, names = frame + 0x200, formats = frame + 0x218;
    guest.setDword(frame + 0x230, library);
    if (s32(guest::direct<0x4b6590, directXVersion>(app, ts)) < 5)
    {
        copyDwords(guest, names, kNeedsDirectX, 6);
        const u32 name = guest.dword(names + language * 4);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), text, name, guest::Words({}));
        os::messageBox(guest::text(app, text), guest::text(app, kTitle), os::Box::plain);
        fe::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
    }
    const u32 place = guest.dword(frame + 0x230) << 2;
    guest.setDword(frame + 0x238, place);
    const u32 module = os::modules::load(app, guest::text(app, guest.dword(place + kLibraries))).value_or(0);
    guest.setDword(frame + 0x234, module);
    if (module == 0)
    {
        copyDwords(guest, formats, kNoLibrary, 6);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), frame, guest.dword(formats + language * 4), guest::Words{guest.dword(guest.dword(frame + 0x238) + kLibraries)});
        os::messageBox(guest::text(app, frame), guest::text(app, kTitle), os::Box::plain);
        fe::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
    }
    os::release(app, guest.dword(frame + 0x234));
    return 1;  // the library let go: 1, as the code is told
}

/* sub_4b5e50: a message of two tables, by the language, and the game leaves. */
nfs::u32 frontend::wina::fatalMessage(App* app, ThreadState& ts, u32 language)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20 + 0x30);
    const u32 frame = guest.frame();
    copyDwords(guest, frame, 0x4b55c8, 6);
    copyDwords(guest, frame + 0x18, 0x4b55e0, 6);
    os::messageBox(guest::text(app, guest.dword(frame + language * 4)),
                   guest::text(app, guest.dword(frame + 0x18 + language * 4)), os::Box::error);
    return fe::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
}

/* sub_4b5f10: enough memory and disk for the game, or the player is asked.  The boxes have one
 * button and no answer (0): the game goes on. */
nfs::u32 frontend::wina::checkMemory(App* app, ThreadState& ts, u32 language)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x60);
    constexpr u32 kEnoughMemory = 0x1000000;
    const u32 frame = guest.frame();
    // The memory free and what more could be had, as the port tells them: all of it, each.
    const u32 total = os::memory::kPhysicalBytes + os::memory::kPhysicalBytes;
    // The hook's words (the memory in megabytes, twice) are dropped: it does nothing.
    guest::direct<0x42d500, raceload::noHook>(app, ts);
    if (total < kEnoughMemory)
    {
        copyDwords(guest, frame + 0x30, 0x4b5628, 6);
        copyDwords(guest, frame + 0x48, 0x4b5640, 6);
        os::messageBox(guest::text(app, guest.dword(frame + 0x30 + language * 4)),
                       guest::text(app, guest.dword(frame + 0x48 + language * 4)), os::Box::error);
    }
    const u32 room = fe::direct<0x4516a0, loada::diskHasRoom>(app, ts, 0x100000);
    if (room != 0)
        return room;
    copyDwords(guest, frame, 0x4b5658, 6);
    copyDwords(guest, frame + 0x18, 0x4b5670, 6);
    // eax the answer the box leaves: none, 0.
    os::messageBox(guest::text(app, guest.dword(frame + language * 4)),
                   guest::text(app, guest.dword(frame + 0x18 + language * 4)), os::Box::error);
    return 0;
}

/* sub_4b5ff0: the answer to the Direct3D device enumeration's question. */
nfs::u32 frontend::wina::deviceAnswer(App*, ThreadState&, u32, u32, u32, u32 question, u32, u32)
{
    return question == 0 || question == 1 ? 0x424d5144 : 1;
}

/* What the game's code is told when a setting, or its key, is not there (the registry's
 * ERROR_FILE_NOT_FOUND as the port has always answered it: 1); 0 when it is. */
constexpr u32 kSettingNotThere = 1, kSettingThere = 0;

/* sub_4b6400: the 3D device's name and its memory from the settings; a name that says
 * so when it has none.  The memory's setting into the frame's word, where the code takes it. */
nfs::u32 frontend::wina::thrashDevice(App* app, ThreadState& ts, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 44);
    constexpr u32 kNoDevice = 0x53ebc4;
    using os::settings::Root;
    const u32 frame = guest.frame();
    const char* key = guest::text(app, 0x53fe50);
    if (const auto device = os::settings::find(app, Root::localMachine, key, guest::text(app, 0x53fe7c)))
    {
        os::settings::copyTo(app, *device, buffer, 0x40);
        const auto memory = os::settings::find(app, Root::localMachine, key, guest::text(app, 0x53fe8c));
        if (!memory)
            return kSettingNotThere;
        os::settings::copyTo(app, *memory, frame + 8, 4);
        guest.setDword(0x55fdbc, guest.dword(frame + 8));
        return guest.dword(frame + 8);
    }
    return fe::direct<kStrncpy, watcom::clib::strncpy>(app, ts, buffer, kNoDevice, guest.length(kNoDevice) - 5);
}

/* sub_4b64c0: the 3D device's description from the settings: kSettingThere, or
 * kSettingNotThere. */
nfs::u32 frontend::wina::thrashDescription(App* app, ThreadState& ts, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    using os::settings::Root;
    const auto description =
        os::settings::find(app, Root::localMachine, guest::text(app, 0x53fe50), guest::text(app, 0x53fe98));
    if (!description)
        return kSettingNotThere;
    os::settings::copyTo(app, *description, buffer, 0x32);
    return kSettingThere;
}

/* sub_4b6520: the desktop's wallpaper setting from the settings. */
nfs::u32 frontend::wina::desktopSetting(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24);
    constexpr u32 kBuffer = 0x7a3ae0;
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kBuffer, 0, 0x100);
    using os::settings::Root;
    if (const auto wallpaper =
            os::settings::find(app, Root::currentUser, guest::text(app, 0x53feb0), guest::text(app, 0x53fef4)))
        os::settings::copyTo(app, *wallpaper, kBuffer, 0x100);
    return kBuffer;
}

/* sub_4b6590: what DirectX there is: ddraw.dll's interfaces asked for one by one: 0 none, 1, 3
 * or 5.  The code asks the system's version first, for none older than NT 5: the port's is newer
 * (it has always told NT 5.1).  The port's display (os_video.h) is all the game asks of
 * DirectDraw, each interface's part of it: 5. */
nfs::u32 frontend::wina::directXVersion(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 24 + 0x110);
    return 5;
}

/* ---------------------------------------------------------------------------
 * What is left of the graphics library, the palette and the runtime
 * ------------------------------------------------------------------------- */

/* sub_4ea665: entry `index` of a table, or 0 past its end. */
nfs::u32 frontend::wina::tableEntry(App* app, ThreadState& ts, u32 table, u32 index, u32 out)
{
    Guest guest(app, ts);
    if (index < guest.dword(table + 8))
    {
        const u32 value = guest.dword(table + index * 8 + 0x10);
        guest.setDword(out, value);
        return value;
    }
    guest.setDword(out, 0);
    return table;
}

/* sub_4f2460: a shape's hot spots, the attachment of type 0x7c in its chain; 0 when none.
 * A shape of a kind not known is said to be wrong. */
nfs::u32 frontend::wina::shapeHotspots(App* app, ThreadState& ts, u32 shape)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    constexpr u32 kShapeKinds = 0x566d54, kHotspots = 0x7c;
    if (shape == 0)
        return 0;
    if (guest.byte((guest.dword(shape) & 0x7f) + kShapeKinds) == 0)
        assertFailed(guest, 0x54cad4, 0x54cae4, 0x31, args({ guest.dword(shape) & 0xff }, { 0x54caf8, guest.dword(shape) & 0xff }));
    for (;;)
    {
        safepoint(app, ts);
        if (shape == 0)
            return 0;
        if ((guest.dword(shape) & 0xff) == kHotspots)
            return shape + 8;
        const s32 step = s32(guest.dword(shape)) >> 8;
        shape = step == 0 ? 0 : shape + u32(step);
    }
}

/* sub_4f87a0: the graphics' screen lock let go and its driver slot back. */
nfs::u32 frontend::wina::releaseDriverSlot(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    const u32 module = guest.dword(0x564a6c), slot = guest.dword(0x564a68);
    if (module != 0)
    {
        os::release(app, module);
    }
    guest.setDword(0x564a6c, 0);
    fe::direct<0x517170, graphics::driverSlotUsed>(app, ts, 0);
    return fe::direct<0x517370, graphics::driverSlotKept>(app, ts, slot);
}

/* sub_4f9120: whether the game has the focus and a window; else sub_4f9088 says. */
nfs::u32 frontend::wina::hasFocus(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (guest.dword(kActive) != 0 && guest.dword(kHwnd) != 0)
        return 1;
    return fe::direct<0x4f9088, eab::lineNoFlag>(app, ts);
}

/* sub_4f9360: the video buffers given back. */
nfs::u32 frontend::wina::videoClose(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    constexpr u32 kOpen = 0x567194, kSurface = 0x567190;
    if (guest.dword(kOpen) == 0)
        return 0;
    guest::direct<0x51a170, freeSurface>(app, ts, guest.dword(kSurface));
    guest.setDword(kSurface, 0);
    const u32 left = guest::direct<0x519140, releaseVideoBuffers>(app, ts);
    guest.setDword(kOpen, 0);
    return left;
}

/* sub_4fb060: a block of memory the race kept, let go. */
nfs::u32 frontend::wina::freeKeptBlock(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 block = guest.dword(0x5650c0);
    if (block == 0)
        return 0;
    const u32 left = u32(*fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block));
    guest.setDword(0x5650c0, 0);
    return left;
}

/* sub_4fd490: a row filled, its length halved. */
nfs::u32 frontend::wina::fillHalved(App* app, ThreadState& ts, u32 to, u32 count)
{
    Guest guest(app, ts);
    return fe::direct<kFillDword, ea::fillPattern>(app, ts, to, u32(s32(count) >> 1), guest.dword(kFillColour));
}

/* sub_4fd4b0: a row filled, the count as it is. */
nfs::u32 frontend::wina::fillSame(App* app, ThreadState& ts, u32 to, u32 count)
{
    Guest guest(app, ts);
    return fe::direct<kFillDword, ea::fillPattern>(app, ts, to, count, guest.dword(kFillColour));
}

/* sub_4fd4c0: a row filled, the count doubled. */
nfs::u32 frontend::wina::fillDoubled(App* app, ThreadState& ts, u32 to, u32 count)
{
    Guest guest(app, ts);
    return fe::direct<kFillDword, ea::fillPattern>(app, ts, to, count + count, guest.dword(kFillColour));
}

/* sub_4fd520: a row filled, the count times four. */
nfs::u32 frontend::wina::fillQuadrupled(App* app, ThreadState& ts, u32 to, u32 count)
{
    Guest guest(app, ts);
    return fe::direct<kFillDword, ea::fillPattern>(app, ts, to, count << 2, guest.dword(kFillColour));
}

/* sub_4fd4e0: 16-bit pixels of the fill colour. */
nfs::u32 frontend::wina::fillWords(App* app, ThreadState& ts, u32 upper, u32 to, u32 count)
{
    Guest guest(app, ts);
    u32 at = to, left = count, colour = 0;
    do
    {
        safepoint(app, ts);
        at += 2;
        colour = guest.word(kFillColour);
        guest.setWord(at - 2, std::uint16_t(colour));
        --left;
    } while (left != 0);
    return (upper & 0xffff0000u) | colour;
}

/* sub_4fd500: 24-bit pixels of the fill colour. */
nfs::u32 frontend::wina::fillTriples(App* app, ThreadState& ts, u32 to, u32 count)
{
    Guest guest(app, ts);
    u32 at = to, left = count, high = 0;
    do
    {
        safepoint(app, ts);
        guest.setWord(at, guest.word(kFillColour));
        high = guest.dword(kFillColour) >> 16;
        at += 3;
        guest.setByte(at - 1, std::uint8_t(high));
        --left;
    } while (left != 0);
    return high;
}

/* sub_503b00: an int shifted left into 64 bits at esi. */
nfs::u32 frontend::wina::shiftTo64(App* app, ThreadState& ts, u32 value, u32 by, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    u32 low, high;
    if (s32(by) >= 0x20)
    {
        low = 0;
        high = value << ((by - 0x20) & 31);
    }
    else if (by == 0)
    {
        low = value;
        high = u32(s32(value) >> 31);
    }
    else
    {
        low = value << (by & 31);
        high = u32(s32(value) >> ((0x20 - by) & 31));
    }
    guest.setDword(out, low);
    guest.setDword(out + 4, high);
    return out;
}

/* sub_503cd0: palette entries given to the window's surface, read back unless it is
 * in fullscreen. */
nfs::u32 frontend::wina::paletteToSurface(App* app, ThreadState& ts, u32 surface, u32 first, u32 count)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 144);
    constexpr u32 kColours = 0x30, kLockDepth = 0x43c, kLowest = 0x430, kHighest = 0x432;
    const u32 frame = guest.frame();
    const u32 window = surface != 0 ? surface : kWindow;
    guest.setDword(frame + 0x7c, first);
    guest.setDword(0x5643e4, guest.dword(0x5643e4) + 1);
    if (guest.dword(0x564364) != 7 || s32(guest.dword(window + 0x1c)) > 8)
        return surface;
    const u32 last = first + count - 1;
    const u32 locked = guest.dword(window + kLockDepth) != 0 ? 1 : 0;
    guest.setDword(frame + 0x78, last);
    guest.setDword(frame + 0x70, locked);
    u32 in = first * 4 + kPalette, out = window + kColours + first * 4;
    u32 leftInEbx = out;
    for (s32 i = 0; i < s32(count); ++i)
    {
        safepoint(app, ts);
        leftInEbx = (u32(guest.byte(in)) << 16) | (u32(guest.byte(in + 1)) << 8) | guest.byte(in + 2);
        guest.setDword(out, leftInEbx);
        in += 4;
        out += 4;
    }
    if ((s32(guest.dword(window + 0x42e)) >> 16) > s32(first))
        guest.setWord(window + kLowest, std::uint16_t(first));
    if ((s32(guest.dword(window + kLowest)) >> 16) < s32(last))
        guest.setWord(window + kHighest, std::uint16_t(last));
    if (locked != 0)
    {
        fe::direct<0x4f46d0, wata::windowSave>(app, ts, frame);
        fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    }
    u32 left = fe::direct<0x4ecf60, eab::ddrawApplyPalette>(app, ts, window);
    if (guest.byte(window + 0x45d) == 0)
    {
        u32 to = first * 4 + kPalette, from = window + kColours + first * 4;
        for (s32 i = 0; i < s32(count); ++i)
        {
            safepoint(app, ts);
            guest.setByte(to + 2, guest.byte(from));
            guest.setByte(to + 1, std::uint8_t(guest.dword(from) >> 8));
            const u32 red = (guest.dword(from) >> 16) & 0xff;
            guest.setByte(to, std::uint8_t(red));
            to += 4;
            from += 4;
        }
        left = to;
    }
    if (guest.dword(frame + 0x70) != 0)
    {
        return fe::direct<0x4f4700, wata::windowRestore>(app, ts, frame);
    }
    return left;
}

/* sub_503e50: palette entries set from bytes red green blue. */
nfs::u32 frontend::wina::paletteSet(App* app, ThreadState& ts, u32 first, u32 count, u32 from)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    fe::direct<0x50425d, rendf::rowRgb24ToArgb>(app, ts, from, first * 4 + kPalette, count);
    return guest::direct<0x503cd0, paletteToSurface>(app, ts, kWindow, first, count);
}

/* sub_503ec0: palette entries as bytes red green blue. */
nfs::u32 frontend::wina::paletteGet(App* app, ThreadState& ts, u32 first, u32 count, u32 to)
{
    Guest guest(app, ts);
    u32 from = first * 4 + kPalette;
    if (s32(count) <= 0)
        return first << 2;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        safepoint(app, ts);
        guest.setByte(to, guest.byte(from + 2));
        guest.setByte(to + 1, guest.byte(from + 1));
        guest.setByte(to + 2, guest.byte(from));
        from += 4;
        to += 3;
    }
    return from;
}

/* sub_50e800: the queue's first entry of a priority taken off it, 0 when it has none. */
nfs::u32 frontend::wina::queueTakeByPriority(App* app, ThreadState& ts, u32 queue, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 20);
    constexpr u32 kSource = 0x54fbb8, kFunction = 0x54fd58;
    constexpr u32 kFirstEntry = 8, kPriorityOf = 0x10, kPriorityArg = 0x14, kQueueLock = 0x18;
    enterLock(guest, ts, guest.dword(queue + kQueueLock));
    if (guest.dword(queue + kQueueLock) == 0)
        assertFailed(guest, kSource, kFunction, 0x3ac, args({ 0x3ac }, { 0x54fbcc, kFunction, queue }));
    u32 entry = guest.dword(queue + kFirstEntry);
    while (entry != 0)
    {
        safepoint(app, ts);
        if (fe::pointerCall(guest, guest.dword(queue + kPriorityOf), args({ entry, guest.dword(queue + kPriorityArg) })) >= wanted)
            break;
        entry = guest.dword(entry);
    }
    u32 taken = 0;
    if (entry != 0 && fe::pointerCall(guest, guest.dword(queue + kPriorityOf), args({ entry, guest.dword(queue + kPriorityArg) })) == wanted)
    {
        if (directKept<0x50dc20, ea::queueUnlinked, 2>(guest.app(), guest.ts(), queue, entry) != 0)
        {
            taken = entry;
        }
        else
        {
            assertFailed(guest, kSource, kFunction, 0x3b5, args({}, { 0x54fd68 }));
        }
    }
    leaveLock(guest, ts, guest.dword(queue + kQueueLock));
    return taken;
}

/* sub_517580: the graphics closed when they are open. */
nfs::u32 frontend::wina::graphicsClose(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    if (guest.dword(0x564364) == 0)
        return 0;
    guest::direct<0x517920, graphicsPalette>(app, ts, 0);
    fe::direct<0x524650, graphics::driverClosed>(app, ts, true);
    const u32 closed = 0;  // the eax it leaves
    guest.setDword(0x564364, 0);
    guest.setDword(0x564418, 0);
    return closed;
}

/* sub_5175b0: the system's request to close, from whichever thread. */
nfs::u32 frontend::wina::graphicsAskClose(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    if (*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0))
    {
        guest::direct<0x517580, graphicsClose>(app, ts);
        return 0;
    }
    const u32 lock = guest.dword(kGraphicsLock);
    if (lock == 0)
        return 0;
    enterLock(guest, ts, lock);
    const u32 hwnd = guest.dword(kHwnd);
    const u32 answer = guest.dword(kGraphicsLock);
    // The window minimised: the port's window is the screen and is not minimised by the game.
    NFS2_USE(hwnd);
    return answer;
}

/* sub_517920: the screen's palette, or a shape's, given to the driver. */
nfs::u32 frontend::wina::graphicsPalette(App* app, ThreadState& ts, u32 surface)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    if (guest.dword(0xa18220) != 0)
        fe::direct<0x5178d0, graphics::frameLockReleased>(app, ts);
    if (guest.dword(0x9f3d0c) != 0)
        fe::direct<0x4ed2a0, eab::ddrawUnlockBitmap>(app, ts, kWindow);
    if (surface != 0)
    {
        const std::uint8_t mode = guest.byte(surface + 0x1e);
        if (mode != 0 && mode != 0xff)
        {
            return *fe::direct<0x517890, graphics::frameBufferLocked>(app, ts, surface, mode);
        }
        const u32 palette = guest.dword(surface + 0x34);
        if (palette != 0)
        {
            // The palette in edx, where the call takes the surface; ebx as the game's code has it (the surface).
            return fe::direct<0x4ed0f0, eab::ddrawLockBitmap>(app, ts, kWindow, palette, surface);
        }
    }
    return 1;
}

/* sub_519140: the video's two buffers given back through the driver's table. */
nfs::u32 frontend::wina::releaseVideoBuffers(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 16);
    constexpr u32 kDriver = 0x56718c, kFreeBuffer = 0x18, kFirst = 0x569058, kSecond = 0x56905c;
    u32 eax = 0;
    const u32 first = guest.dword(kFirst);
    if (first != 0)
    {
        const u32 table = guest.dword(kDriver);
        eax = fe::pointerCall(guest, guest.dword(table + kFreeBuffer), args({ first }));
        guest.setDword(kFirst, 0);
    }
    const u32 second = guest.dword(kSecond);
    if (second != 0)
    {
        ts.sp -= 4;
        const u32 table = guest.dword(kDriver);
        eax = fe::pointerCall(guest, guest.dword(table + kFreeBuffer), args({ second }));
        guest.setDword(kSecond, 0);
        ts.sp += 4;
    }
    guest.setDword(0x569054, 0);
    guest.setDword(0x569060, 0);
    return eax;
}

/* sub_51a170: a surface given back through the driver's table. */
nfs::u32 frontend::wina::freeSurface(App* app, ThreadState& ts, u32 surface)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    guest::direct<0x525600, freeSurfaceCache>(app, ts, surface);
    const u32 table = guest.dword(0x56718c);
    return fe::pointerCall(guest, guest.dword(table + 0x18), args({ surface }));
}

/* sub_5244f0: the system's tasks, the idle task of the graphics. */
nfs::u32 frontend::wina::graphicsIdle(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0));
}

/* sub_524500: the display `object` (os_video.h) taken as the library's own, or with none the
 * library's let go; 0, nothing left of it. */
nfs::u32 frontend::wina::setDirectDraw(App* app, ThreadState& ts, u32 object)
{
    Guest guest(app, ts);
    constexpr u32 kUses = 0x9f3d3c;
    if (object != 0)
    {
        guest.setDword(kWindow, object);
        guest.setDword(kUses, guest.dword(kUses) + 1);
        return 0;
    }
    os::video::release(app, guest.dword(kWindow));
    const u32 uses = guest.dword(kUses) + 1;
    guest.setDword(kWindow, 0);
    guest.setDword(kUses, uses);
    return 0;
}

/* sub_524550: the graphics' lock taken or let go, as the argument says. */
nfs::u32 frontend::wina::graphicsLock(App* app, ThreadState& ts, u32 taking)
{
    Guest guest(app, ts);
    if (taking != 0)
    {
        ts.sp -= 4;
        enterLock(guest, ts, guest.dword(kGraphicsLock));
        ts.sp += 4;
    }
    else
    {
        const u32 lock = guest.dword(kGraphicsLock);
        leaveLock(guest, ts, lock);
    }
    guest.setDword(0x567180, taking);
    return taking;
}

/* sub_5245a0: a message of the driver's: an error, on the monochrome screen, in the log,
 * or to the debugger, by its level. */
nfs::u32 frontend::wina::driverMessage(App* app, ThreadState& ts, u32 level, u32 text)
{
    Guest guest(app, ts);
    switch (level)
    {
    case 0:
        fe::direct<0x401010, ea::error>(app, ts, text, guest::Words({}));
        return level;  // Error leaves eax as it was
    case 1:
        return fe::direct<0x4e08f0, ea::monoPrinted>(app, ts, text, text, guest::Words({})).value_or(0);
    case 2:
        return fe::direct<0x4df6c0, ea::messageOf>(app, ts, text).value_or(0);
    case 3:
        os::debugText(guest::text(app, text));
        return text;  // the debugger's log leaves eax as the code set it
    default:
        return text;
    }
}

/* sub_5245f0: the driver's event: the resize callback told on the second kind. */
nfs::u32 frontend::wina::driverEvent(App* app, ThreadState& ts, u32 kind, u32)
{
    Guest guest(app, ts);
    if (kind == 2 && guest.dword(kOnResize) != 0)
    {
        return fe::pointerCall(guest, guest.dword(kOnResize), args({ kind }));
    }
    return kind;
}

/* sub_524620: the driver closed without its window, for an error. */
nfs::u32 frontend::wina::closeDriverOnly(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    fe::direct<0x524650, graphics::driverClosed>(app, ts, true);
    guest.setDword(0x564418, 0);  // the edx it was called with, which it keeps
    return 0;  // the eax it leaves
}

/* sub_5247c0: the screen's size and depth, with the defaults; the graphics started when
 * they are not. */
nfs::u32 frontend::wina::setScreenSize(App* app, ThreadState& ts, u32 widthIn, u32 heightIn, u32 depthIn)
{
    Guest guest(app, ts);
    const u32 width = widthIn != 0 ? widthIn : 0x280;
    const u32 height = heightIn != 0 ? heightIn : 0x1e0;
    const u32 depth = depthIn != 0 ? depthIn : 0x10;
    guest.setDword(0x56438c, depth);
    const u32 originX = guest.dword(0x56439c);
    guest.setDword(0x564384, width);
    guest.setDword(0x5643a4, width - originX);
    const u32 originY = guest.dword(0x5643a0);
    guest.setDword(0x564388, height);
    const u32 room = height - originY;
    guest.setDword(0x5643a8, room);
    if (guest.dword(0x564364) != 0)
        return originY;
    // originY, room and originX, which the game's code hands it in eax, edx and ebx, are not read.
    return fe::direct<0x509940, watb::initRowTable>(app, ts);
}

/* sub_525600: what a surface keeps on the side, given back. */
nfs::u32 frontend::wina::freeSurfaceCache(App* app, ThreadState& ts, u32 surface)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    if (guest.dword(surface + 0x54) == 0)
        return surface;
    ts.sp -= 4;
    const u32 table = guest.dword(0x56718c);
    const u32 answer = fe::pointerCall(guest, guest.dword(table + 8), args({ guest.dword(surface + 0x54) }));
    ts.sp += 4;
    guest.setDword(surface + 0x54, 0);
    guest.setByte(surface + 0xbc, 1);
    return answer;
}

/* sub_525630: the next byte of a text, its pointer moved. */
nfs::u32 frontend::wina::nextByte(App* app, ThreadState& ts, u32 place)
{
    Guest guest(app, ts);
    const u32 at = guest.dword(place);
    const u32 character = guest.byte(at);
    guest.setDword(place, at + 1);
    return character;
}

/* sub_525650: a character through the table of the printable ones. */
nfs::u32 frontend::wina::mapCharacter(App* app, ThreadState& ts, u32 character)
{
    Guest guest(app, ts);
    if (s32(character) < 0x20 || s32(character) > 0x7f)
        return character;
    return guest.word(character * 2 + 0x56afb8);
}

/* sub_525670: the next character of a text: two bytes with the top bit set, else one
 * through the table. */
nfs::u32 frontend::wina::nextCharacter(App* app, ThreadState& ts, u32 place)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    u32 at = guest.dword(place);
    const u32 first = guest.byte(at);
    ++at;
    if ((first & 0x80) != 0)
    {
        const u32 second = guest.byte(at);
        guest.setDword(place, at + 1);
        return (first << 8) | second;
    }
    const u32 mapped = guest::direct<0x525650, mapCharacter>(app, ts, first);
    guest.setDword(place, at);
    return mapped & 0xffff;
}

/* sub_52e420: the text driver started with the screen's saved two values. */
nfs::u32 frontend::wina::textStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 12);
    return fe::direct<0x52ee00, watc::setClip>(app, ts, 0, 0, guest.dword(0x564ff8), guest.dword(0x564ffc));
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::winaStreamCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf1c0", 0, 0, nullptr, 0, kCx };
    const u32 handle = args.eax, streamOut = args.edx, handleOut = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamCheck(app, ts, handle, streamOut, handleOut); });
}

nfs::u32 frontend::winaRingBetween(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf1f0", 0, 0, nullptr, 0, 0 };
    const u32 from = args.eax, to = args.edx, place = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::ringBetween(app, ts, from, to, place); });
}

nfs::u32 frontend::winaStreamConsume(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf220", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 s = args.eax, amount = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamConsume(app, ts, s, amount); });
}

nfs::u32 frontend::winaStreamTakeFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf280", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 s = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamTakeFree(app, ts, s); });
}

nfs::u32 frontend::winaStreamQueueFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf2f0", 0, 0, nullptr, 0, kBx | kCx | kSi };
    const u32 s = args.eax, record = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamQueueFile(app, ts, s, record); });
}

nfs::u32 frontend::winaStreamFindFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf340", 0, 0, nullptr, 0, kBx | kCx };
    const u32 s = args.eax, id = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFindFile(app, ts, s, id); });
}

nfs::u32 frontend::winaStreamUnlinkFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf380", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 s = args.eax, record = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamUnlinkFile(app, ts, s, record); });
}

nfs::u32 frontend::winaStreamClassify(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf3f0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 s = args.eax, chunk = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamClassify(app, ts, s, chunk); });
}

nfs::u32 frontend::winaStreamParse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf440", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 s = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamParse(app, ts, s); });
}

nfs::u32 frontend::winaStreamFileOpened(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf570", 0, 0, nullptr, 0, 0 };
    const u32 s = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFileOpened(app, ts, s); });
}

nfs::u32 frontend::winaStreamFileSwitched(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf5a0", 0, 0, nullptr, 0, kCx | kSi };
    const u32 s = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFileSwitched(app, ts, s); });
}

nfs::u32 frontend::winaStreamFilled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf5e0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 s = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFilled(app, ts, s); });
}

nfs::u32 frontend::winaStreamNextFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf6b0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 s = args.eax, priority = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamNextFile(app, ts, s, priority); });
}

nfs::u32 frontend::winaStreamReadMore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf810", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 s = args.eax, priority = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamReadMore(app, ts, s, priority); });
}

nfs::u32 frontend::winaStreamHeaderSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfa40", 0, 0, nullptr, 0, kCx | kSi };
    const u32 fileCount = args.eax, ruleCount = args.edx, channelCount = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::streamHeaderSize(app, ts, fileCount, ruleCount, channelCount);
    });
}

nfs::u32 frontend::winaStreamCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfa80", 4, 0, nullptr, 0, kSi | kDi };
    const u32 files = args.eax, rules = args.edx, channels = args.ebx, s = args.ecx;
    const u32 memory = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::streamCreate(app, ts, files, rules, channels, s, memory);
    });
}

nfs::u32 frontend::winaStreamSetRule(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfca0", 4, 0, nullptr, 0, kSi | kDi };
    const u32 handle = args.eax, number = args.edx, mask = args.ebx, value = args.ecx;
    const u32 target = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::streamSetRule(app, ts, handle, number, mask, value, target);
    });
}

nfs::u32 frontend::winaStreamClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfd30", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamClose(app, ts, handle); });
}

nfs::u32 frontend::winaStreamSetLowWater(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfdd0", 0, 0, nullptr, 0, kBx | kCx | kSi };
    const u32 handle = args.eax, low = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamSetLowWater(app, ts, handle, low); });
}

nfs::u32 frontend::winaStreamSetBelow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfe30", 0, 0, nullptr, 0, kBx | kCx };
    const u32 handle = args.eax, below = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamSetBelow(app, ts, handle, below); });
}

nfs::u32 frontend::winaStreamChannel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfe70", 0, 0, nullptr, 0, kBx | kCx };
    const u32 handle = args.eax, number = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamChannel(app, ts, handle, number); });
}

nfs::u32 frontend::winaStreamOpenFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cfef0", 0, 0, nullptr, 0, kSi | kDi };
    const u32 handle = args.eax, name = args.edx, length = args.ebx, endId = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamOpenFile(app, ts, handle, name, length, endId); });
}

nfs::u32 frontend::winaStreamOpenMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cffb0", 0, 0, nullptr, 0, kSi | kDi };
    const u32 handle = args.eax, data = args.edx, length = args.ebx, endId = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamOpenMemory(app, ts, handle, data, length, endId); });
}

nfs::u32 frontend::winaStreamCloseFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0060", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 handle = args.eax, id = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamCloseFile(app, ts, handle, id); });
}

nfs::u32 frontend::winaStreamFlush(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0220", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFlush(app, ts, handle); });
}

nfs::u32 frontend::winaStreamNextChunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0300", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamNextChunk(app, ts, handle); });
}

nfs::u32 frontend::winaStreamFreeChunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d03c0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 handle = args.eax, chunk = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::streamFreeChunk(app, ts, handle, chunk); });
}

nfs::u32 frontend::winaChannelBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0460", 0, 0, nullptr, 0, kBx | kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::channelBytes(app, ts, handle); });
}

nfs::u32 frontend::winaChannelState(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0490", 0, 0, nullptr, 0, kBx | kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::channelState(app, ts, handle); });
}

nfs::u32 frontend::winaChannelDone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d04c0", 0, 0, nullptr, 0, kBx | kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::channelDone(app, ts, handle); });
}

nfs::u32 frontend::winaChannelRingSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d0500", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::channelRingSize(app, ts, handle); });
}

nfs::u32 frontend::winaMovieNextFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_495b60", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 reader = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::movieNextFrame(app, ts, reader); });
}

nfs::u32 frontend::winaFileOpAllocate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fb990", 0, 0, nullptr, 0, kSi | kDi };
    const u32 kind = args.eax, priorityIn = args.edx, target = args.ebx, slot = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::fileOpAllocate(app, ts, kind, priorityIn, target, slot);
    });
}

nfs::u32 frontend::winaFileOpQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fbb60", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 op = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpQueue(app, ts, op); });
}

nfs::u32 frontend::winaFileOpSetName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fbbe0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 op = args.eax, nameIn = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpSetName(app, ts, op, nameIn); });
}

nfs::u32 frontend::winaFileOpCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fc8e0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 id = args.eax, callback = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpCallback(app, ts, id, callback); });
}

nfs::u32 frontend::winaFileOpPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fc9b0", 0, 0, nullptr, 0, kBx | kCx };
    const u32 id = args.eax, priority = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpPriority(app, ts, id, priority); });
}

nfs::u32 frontend::winaFileOpOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fca60", 0, 0, nullptr, 0, kSi | kDi };
    const u32 name = args.eax, mode = args.edx, priority = args.ebx, target = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpOpen(app, ts, name, mode, priority, target); });
}

nfs::u32 frontend::winaFileOpClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fca90", 0, 0, nullptr, 0, kCx | kSi };
    const u32 handle = args.eax, priority = args.edx, target = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileOpClose(app, ts, handle, priority, target); });
}

nfs::u32 frontend::winaFileOpRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fcab0", 8, 0, nullptr, 0, kSi | kDi };
    const u32 handle = args.eax, offset = args.edx, buffer = args.ebx, size = args.ecx;
    const u32 priority = app->getMemory<u32>(ts.sp + 4), target = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::fileOpRead(app, ts, handle, offset, buffer, size, priority, target);
    });
}

nfs::u32 frontend::winaFileSlotOfName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fce50", 0, 0, nullptr, 0, kCx | kDx | kSi };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileSlotOfName(app, ts, name); });
}

nfs::u32 frontend::winaFileCloseWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd060", 0, 0, nullptr, 0, kBx | kCx };
    const u32 handle = args.eax, priority = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileCloseWait(app, ts, handle, priority); });
}

nfs::u32 frontend::winaFileSizeAndSeek(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e4230", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 handle = args.eax, offset = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileSizeAndSeek(app, ts, handle, offset); });
}

nfs::u32 frontend::winaFileLockOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4e4320", 0, 0, nullptr, 0, kDx };
    const u32 handle = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fileLockOf(app, ts, handle); });
}

nfs::u32 frontend::winaCompareHandlers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc650", 0, 0, nullptr, 0, 0 };
    const u32 leftRow = args.eax, rightRow = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::compareHandlers(app, ts, leftRow, rightRow); });
}

nfs::u32 frontend::winaHandlerOf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc660", 4, 0, nullptr, 0, kBx | kSi };
    const u32 message = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::handlerOf(app, ts, message); });
}

nfs::u32 frontend::winaHandlerSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc6d0", 8, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 message = app->getMemory<u32>(ts.sp + 4), handler = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::handlerSet(app, ts, message, handler); });
}

nfs::u32 frontend::winaPostMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc7c0", 0, 0, nullptr, 0, kSi | kDi };
    const u32 message = args.eax, lParam = args.edx, wParam = args.ebx, send = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::postMessage(app, ts, message, lParam, wParam, send); });
}

nfs::u32 frontend::winaSetFocusCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc860", 0, 0, nullptr, 0, 0 };
    const u32 callback = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::setFocusCallback(app, ts, callback); });
}

nfs::u32 frontend::winaNoWindowTask(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc870", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::noWindowTask(app, ts); });
}

nfs::u32 frontend::winaMovePointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc890", 0, 0, nullptr, 0, kCx | kSi };
    const u32 window = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::movePointer(app, ts, window, x, y); });
}

nfs::u32 frontend::winaPushKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cc8e0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 key = args.eax, scan = args.edx, flags = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::pushKey(app, ts, key, scan, flags); });
}

nfs::u32 frontend::winaKeyOfMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccaa0", 0, 0, nullptr, 0, 0 };
    const u32 message = args.eax, virtualKey = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::keyOfMessage(app, ts, message, virtualKey); });
}

nfs::u32 frontend::winaKeyboardHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccc70", 12, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 code = app->getMemory<u32>(ts.sp + 4), wParam = app->getMemory<u32>(ts.sp + 8);
    const u32 lParam = app->getMemory<u32>(ts.sp + 12);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::keyboardHook(app, ts, code, wParam, lParam); });
}

nfs::u32 frontend::winaOnSetFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccd60", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onSetFocus(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnKillFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccdd0", 0x18, 0, nullptr, 0, kBx | kSi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onKillFocus(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cce40", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onClose(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnQuitAsked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cce60", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onQuitAsked(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnDestroy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cce80", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onDestroy(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnSysCommand(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cce90", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onSysCommand(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnSettingChange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cced0", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onSettingChange(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccf50", 0x18, 0, nullptr, 0, kBx };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onMoved(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnChar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ccfc0", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onChar(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnKeyDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd020", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onKeyDown(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnKeyUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd0e0", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onKeyUp(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnMouseDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd130", 0x18, 0, nullptr, 0, kBx | kSi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onMouseDown(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnMouseUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd190", 0x18, 0, nullptr, 0, kBx | kSi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onMouseUp(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnPaint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd1f0", 0x18, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onPaint(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaOnEraseBackground(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd2a0", 0x18, 0, nullptr, 0, 0 };
    const u32 window = app->getMemory<u32>(ts.sp + 4), hwnd = app->getMemory<u32>(ts.sp + 8);
    const u32 message = app->getMemory<u32>(ts.sp + 12), wParam = app->getMemory<u32>(ts.sp + 16);
    const u32 lParam = app->getMemory<u32>(ts.sp + 20), result = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::onEraseBackground(app, ts, window, hwnd, message, wParam, lParam, result);
    });
}

nfs::u32 frontend::winaWindowProc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd2d0", 0x10, 0, nullptr, 0, kBx | kSi | kDi };
    const u32 hwnd = app->getMemory<u32>(ts.sp + 4), message = app->getMemory<u32>(ts.sp + 8);
    const u32 wParam = app->getMemory<u32>(ts.sp + 12), lParam = app->getMemory<u32>(ts.sp + 16);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::windowProc(app, ts, hwnd, message, wParam, lParam); });
}

nfs::u32 frontend::winaCreateWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd350", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 window = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::createWindow(app, ts, window); });
}

nfs::u32 frontend::winaWindowThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd4e0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::windowThread(app, ts); });
}

nfs::u32 frontend::winaOpenWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cd6c0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 width = args.eax, height = args.edx, isFullscreen = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::openWindow(app, ts, width, height, isFullscreen); });
}

nfs::u32 frontend::winaCloseWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdaa0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::closeWindow(app, ts); });
}

nfs::u32 frontend::winaPlaceWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdb10", 0, 0, nullptr, 0, kSi | kDi };
    const u32 left = args.eax, top = args.edx, width = args.ebx, height = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::placeWindow(app, ts, left, top, width, height); });
}

nfs::u32 frontend::winaCentreWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdbd0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 width = args.eax, height = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::centreWindow(app, ts, width, height); });
}

nfs::u32 frontend::winaHookOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdc50", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::hookOff(app, ts); });
}

nfs::u32 frontend::winaHookOn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cdc90", 0, 0, nullptr, 0, kCx };
    const u32 alt = args.eax, callback = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::hookOn(app, ts, alt, callback); });
}

nfs::u32 frontend::winaPointerMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fb470", 0, 0, nullptr, 0, kSi | kDi };
    const u32 x = args.eax, y = args.edx, buttons = args.ebx, click = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::pointerMoved(app, ts, x, y, buttons, click); });
}

nfs::u32 frontend::winaAlreadyRunning(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a38c0", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::alreadyRunning(app, ts); });
}

nfs::u32 frontend::winaVersionText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a38f0", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 buffer = args.eax, length = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::versionText(app, ts, buffer, length); });
}

nfs::u32 frontend::winaTickCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b56d0", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::tickCount(app, ts); });
}

nfs::u32 frontend::winaCheckLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5860", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 library = args.eax, language = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::checkLibrary(app, ts, library, language); });
}

nfs::u32 frontend::winaFatalMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5e50", 0, 0, nullptr, 0, kCx | kDx | kSi | kDi };
    const u32 language = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fatalMessage(app, ts, language); });
}

nfs::u32 frontend::winaCheckMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5f10", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 language = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::checkMemory(app, ts, language); });
}

nfs::u32 frontend::winaDeviceAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5ff0", 0x18, 0, nullptr, 0, 0 };
    const u32 word0 = app->getMemory<u32>(ts.sp + 4), word1 = app->getMemory<u32>(ts.sp + 8);
    const u32 word2 = app->getMemory<u32>(ts.sp + 12), question = app->getMemory<u32>(ts.sp + 16);
    const u32 word4 = app->getMemory<u32>(ts.sp + 20), word5 = app->getMemory<u32>(ts.sp + 24);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wina::deviceAnswer(app, ts, word0, word1, word2, question, word4, word5);
    });
}

nfs::u32 frontend::winaThrashDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6400", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    const u32 buffer = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::thrashDevice(app, ts, buffer); });
}

nfs::u32 frontend::winaThrashDescription(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b64c0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 buffer = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::thrashDescription(app, ts, buffer); });
}

nfs::u32 frontend::winaDesktopSetting(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6520", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::desktopSetting(app, ts); });
}

nfs::u32 frontend::winaDirectXVersion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6590", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi | kDi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::directXVersion(app, ts); });
}

nfs::u32 frontend::winaTableEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea665", 0, 0, nullptr, 0, 0 };
    const u32 table = args.eax, index = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::tableEntry(app, ts, table, index, out); });
}

nfs::u32 frontend::winaShapeHotspots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f2460", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::shapeHotspots(app, ts, shape); });
}

nfs::u32 frontend::winaReleaseDriverSlot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f87a0", 0, 0, nullptr, 0, kBx | kDx | kSi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::releaseDriverSlot(app, ts); });
}

nfs::u32 frontend::winaHasFocus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f9120", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::hasFocus(app, ts); });
}

nfs::u32 frontend::winaVideoClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4f9360", 0, 0, nullptr, 0, kCx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::videoClose(app, ts); });
}

nfs::u32 frontend::winaFreeKeptBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fb060", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::freeKeptBlock(app, ts); });
}

nfs::u32 frontend::winaFillHalved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd490", 0, 0, nullptr, 0, 0 };
    const u32 to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillHalved(app, ts, to, count); });
}

nfs::u32 frontend::winaFillSame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd4b0", 0, 0, nullptr, 0, 0 };
    const u32 to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillSame(app, ts, to, count); });
}

nfs::u32 frontend::winaFillDoubled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd4c0", 0, 0, nullptr, 0, 0 };
    const u32 to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillDoubled(app, ts, to, count); });
}

nfs::u32 frontend::winaFillQuadrupled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd520", 0, 0, nullptr, 0, 0 };
    const u32 to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillQuadrupled(app, ts, to, count); });
}

nfs::u32 frontend::winaFillWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd4e0", 0, 0, nullptr, 0, 0 };
    const u32 upper = args.eax, to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillWords(app, ts, upper, to, count); });
}

nfs::u32 frontend::winaFillTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4fd500", 0, 0, nullptr, 0, 0 };
    const u32 to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::fillTriples(app, ts, to, count); });
}

nfs::u32 frontend::winaShiftTo64(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_503b00", 0, 0, nullptr, 0, kBx | kCx | kDi };
    const u32 value = args.eax, by = args.edx, out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::shiftTo64(app, ts, value, by, out); });
}

nfs::u32 frontend::winaPaletteToSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_503cd0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 surface = args.eax, first = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::paletteToSurface(app, ts, surface, first, count); });
}

nfs::u32 frontend::winaPaletteSet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_503e50", 0, 0, nullptr, 0, kCx | kSi };
    const u32 first = args.eax, count = args.edx, from = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::paletteSet(app, ts, first, count, from); });
}

nfs::u32 frontend::winaPaletteGet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_503ec0", 0, 0, nullptr, 0, kCx | kSi };
    const u32 first = args.eax, count = args.edx, to = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::paletteGet(app, ts, first, count, to); });
}

nfs::u32 frontend::winaQueueTakeByPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_50e800", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 queue = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::queueTakeByPriority(app, ts, queue, wanted); });
}

nfs::u32 frontend::winaGraphicsClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_517580", 0, 0, nullptr, 0, kCx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::graphicsClose(app, ts); });
}

nfs::u32 frontend::winaGraphicsAskClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_5175b0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::graphicsAskClose(app, ts); });
}

nfs::u32 frontend::winaGraphicsPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_517920", 0, 0, nullptr, 0, kBx | kDx | kSi };
    const u32 surface = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::graphicsPalette(app, ts, surface); });
}

nfs::u32 frontend::winaReleaseVideoBuffers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_519140", 0, 0, nullptr, 0, kBx | kCx | kDx | kDi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::releaseVideoBuffers(app, ts); });
}

nfs::u32 frontend::winaFreeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_51a170", 0, 0, nullptr, 0, kCx | kDx };
    const u32 surface = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::freeSurface(app, ts, surface); });
}

nfs::u32 frontend::winaGraphicsIdle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_5244f0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::graphicsIdle(app, ts); });
}

nfs::u32 frontend::winaSetDirectDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_524500", 4, 0, nullptr, 0, 0 };
    const u32 object = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::setDirectDraw(app, ts, object); });
}

nfs::u32 frontend::winaGraphicsLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_524550", 4, 0, nullptr, 0, 0 };
    const u32 taking = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::graphicsLock(app, ts, taking); });
}

nfs::u32 frontend::winaDriverMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_5245a0", 8, 0, nullptr, 0, 0 };
    const u32 level = app->getMemory<u32>(ts.sp + 4), text = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::driverMessage(app, ts, level, text); });
}

nfs::u32 frontend::winaDriverEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_5245f0", 8, 0, nullptr, 0, 0 };
    const u32 kind = app->getMemory<u32>(ts.sp + 4), detail = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::driverEvent(app, ts, kind, detail); });
}

nfs::u32 frontend::winaCloseDriverOnly(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_524620", 0, 0, nullptr, 0, kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::closeDriverOnly(app, ts); });
}

nfs::u32 frontend::winaSetScreenSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_5247c0", 0, 0, nullptr, 0, 0 };
    const u32 widthIn = args.eax, heightIn = args.edx, depthIn = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::setScreenSize(app, ts, widthIn, heightIn, depthIn); });
}

nfs::u32 frontend::winaFreeSurfaceCache(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_525600", 0, 0, nullptr, 0, kCx | kDx };
    const u32 surface = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::freeSurfaceCache(app, ts, surface); });
}

nfs::u32 frontend::winaNextByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_525630", 0, 0, nullptr, 0, kCx | kDx };
    const u32 place = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::nextByte(app, ts, place); });
}

nfs::u32 frontend::winaMapCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_525650", 0, 0, nullptr, 0, 0 };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::mapCharacter(app, ts, character); });
}

nfs::u32 frontend::winaNextCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_525670", 0, 0, nullptr, 0, kCx | kDx };
    const u32 place = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::nextCharacter(app, ts, place); });
}

nfs::u32 frontend::winaTextStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_52e420", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wina::textStart(app, ts); });
}

}
