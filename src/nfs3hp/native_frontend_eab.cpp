#include "native_frontend_eab.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_files.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend_eaa.h"
#include "native_frontend_glue.h"
#include "native_frontend_menz.h"
#include "native_frontend_netb.h"
#include "native_frontend_netc.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_watc.h"
#include "native_frontend_wina.h"
#include "native_load_saves.h"
#include "port_snd_stream_calls.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_windows.h"
#include "host_threads.h"
#include "host_video.h"
#include "host_window.h"
// What lib/thread.h holds pointers to, forward-declared.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include "host_system.h"
#include "host_net.h"
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
x86::reg32 ddrawSetDisplayMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawDisposeDisplay(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawApplyPalette(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawLockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawUnlockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapCreate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapCreateDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 drawClearClip(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeFileFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyNibblesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyBytesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyWordsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyTriplesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyLongsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeDrawKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeDrawKeyedAtHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontUse(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 paletteNearest(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 colourPack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontSetColours(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGlyphSearch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGlyphFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontKerning(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textDrawAtPen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textDrawAt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textMeasureUpTo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textMeasureAll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keyNextLow16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keyNextDispatch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keySetHookFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 inputStateOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 inputStateClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileLoadInto(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeFindHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerTicks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerSignal(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 atexitJump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 exitGameNow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memCopyAligned(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveLibrary(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveHalfBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveWords(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveTriples(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveLongs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeMoveWithSpot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpFirstField(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpResolve(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpAddressText(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpSameAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpLocalName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineNoFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineCallback(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncOpenFile(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFileThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFilesShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFilesInit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netLocalAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memPoolDump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memPoolDumpDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockHasCrcTag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockCrcGood(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockCheckCrc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 exitConfirm(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockResize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 bigFileCount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 bigFileName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}  // frontend
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax: the first `Taken` of `words` its arguments,
 * its result in eax. */
template <x86::reg32 Address, auto Core, std::size_t... I>
auto directOf(win32::WinApplication* app, x86::CPU& cpu, const x86::reg32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, cpu, words[I]...);
}

template <x86::reg32 Address, auto Core, std::size_t Taken, typename... Words>
x86::reg32 directKept(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    const x86::reg32 all[] = { x86::reg32(words)..., 0 };
    const auto result = directOf<Address, Core>(app, cpu, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, x86::reg32>)
        return result;
    else
        return *result;
}

}  // namespace
}  // namespace nfs3hp


/* EA's libraries from 0x4ec870 up, what was left of them:
 *
 *   DirectDraw        a display's mode, palette and the surface locked into a bitmap
 *                     (sub_4ec870 to sub_4ed2a0, win\ddraw.c);
 *   bitmaps, shapes   a bitmap in memory made and freed (cmn\createw.c), a shape found in
 *                     its file, a shape's records, the copies that skip what is clear;
 *   fonts             font\textset.c, textcol.c, textfor.c and textpix.c: the font chosen,
 *                     its colours, a glyph found, a text drawn and measured;
 *   the low timer     win\lowtimer.c: a thread that runs up to eight functions;
 *   memory moves      the library's own memcpy and memmove, by bytes and by pixels;
 *   TCP, the modem    win\tcp.c, TAPI's callback and its shutting down;
 *   file requests     win\nfile.c: the threads that serve a queue of requests;
 *   net games         what opens and closes a transport, the machine's own address;
 *   memory blocks     a pool's dump, the checksum at a block's end, a block resized.
 *
 * All their state stays in guest memory.  What they call of the game is called by address but
 * the typed cores (below); Windows' functions (Winsock's through its thunks) and DirectDraw's
 * methods are the port's own, called as C++ (native_windows.h).  What each function leaves in eax and in the registers it does
 * not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::eab (native_frontend_eab.h), in a frame
 * of its own where the game's function has one (fe::Frame; sub_524bb0's leaves esp as the exit
 * it ends in left it), and a thunk under the game's name that reads the arguments from the
 * registers and the stack for the calls by address.  A core whose code reads an argument's
 * register again after a call puts the argument there first.  A core calls this file's and
 * native_frontend_eaa.cpp's as C++ (guest::direct), and the typed cores of EA's libraries, the
 * port's and of native_frontend_menz.cpp, _glue.cpp and the other files with a header (_wata,
 * _watb, _watc, _wina, _netb, _netc: fe::direct) -- but by address: memmove (sub_4ea4f0) and the
 * library's own memmove and memcpy from their callers here (sub_4f2bf0, sub_4f2ad0), whose ebx
 * and edx are handed back; reservememadr from sub_4eee30, readhandle from sub_4f22e0 and
 * locatebigentry from sub_4ea4a0, whose registers are what those leave; movdfl (sub_50b0b0) from
 * sub_4f2cb0 and lineDeallocateCall (sub_52f969) from sub_4f9500, which read the ebx they leave.
 * sub_4f97c0 keeps eax as each call leaves it up to lineShutdown (sub_5324d2), as the game's code
 * hands it there. */
namespace nfs3hp
{

namespace frontend
{
x86::reg32 ddrawSetDisplayMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawDisposeDisplay(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawApplyPalette(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawLockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawUnlockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapCreate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapCreateDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 drawClearClip(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeFileFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyNibblesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyBytesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyWordsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyTriplesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 copyLongsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeDrawKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeDrawKeyedAtHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontUse(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 paletteNearest(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 colourPack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontSetColours(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGlyphSearch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGlyphFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontKerning(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textDrawAtPen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textDrawAt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textMeasureUpTo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textMeasureAll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 windowBitmapFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keyNextLow16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keyNextDispatch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keySetHookFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 inputStateOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 inputStateClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileLoadInto(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeFindHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerTicks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lowTimerSignal(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 atexitJump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 exitGameNow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memCopyAligned(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveLibrary(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveHalfBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveWords(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveTriples(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memMoveLongs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shapeMoveWithSpot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpFirstField(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpResolve(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpAddressText(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpSameAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tcpLocalName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineNoFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineCallback(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lineShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncOpenFile(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFileThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFilesShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 asyncFilesInit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 netLocalAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memPoolDump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memPoolDumpDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockHasCrcTag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockCrcGood(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockCheckCrc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 exitConfirm(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockResize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}

namespace
{

using fe::args;
using fe::Arguments;
using fe::Guest;
using fe::kRestoreAll;
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdi;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::Site;
using x86::reg32;
using x86::sreg32;

constexpr reg32 kDefaultDisplay = 0x9f38d0;
constexpr reg32 kDisplayLock = 0x564480;       // its critical section, by value
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kGetTickCount = 0x534568;
// Winsock, through the thunks that jump to its imports.
constexpr reg32 kGetHostName = 0x533dde, kNtohs = 0x533dea, kInetNtoa = 0x533df0, kGetHostByName = 0x533df6;
constexpr reg32 kInetAddr = 0x533dfc, kListen = 0x533e02, kGetSockName = 0x533e3e, kHtons = 0x533e4a;
constexpr reg32 kCloseSocket = 0x533ea4;
constexpr reg32 kError = 0x401010, kSprintf = 0x4df690, kMessage = 0x4df6c0, kReport = 0x501050;
constexpr reg32 kFill = 0x4e070c, kReserve = 0x4e1620, kMoveMemory = 0x4ea4f0;
constexpr reg32 kStrncpy = 0x4e0e30, kBlockName = 0x4e15e0;
constexpr reg32 kCreateThread = 0x4df7a0;
constexpr reg32 kLastError = 0x4dfcf0;
constexpr reg32 kEaLog = 0x564ed4, kLineLog = 0x564454;
constexpr reg32 kShapeBits = 0x566d54;         // the bits a pixel of each shape type has

// The fields of a display's record.
constexpr reg32 kModeCount = 0x08, kModeNow = 0x0c, kModeTable = 0x10, kWidth = 0x14, kHeight = 0x18, kBits = 0x1c;
constexpr reg32 kPalette = 0x24, kPaletteEntries = 0x30, kFirstChanged = 0x430, kLastChanged = 0x432;
constexpr reg32 kLockedSurface = 0x43c, kLockedBits = 0x440, kPrimary = 0x444, kWindow = 0x454;
constexpr reg32 kFullscreen = 0x45d, kOffsetX = 0x464, kOffsetY = 0x468;

// The font in use (sub_4ef520 fills it).
constexpr reg32 kPenX = 0x5671f8, kPenY = 0x5671fc, kFontOriginX = 0x567200, kFontOriginY = 0x567204;
constexpr reg32 kFontBits = 0x567208, kFontAscent = 0x567210, kFontDescent = 0x567214, kFontHeight = 0x567218;
constexpr reg32 kFontColourA = 0x567220, kFontColourB = 0x567224, kFontRamp = 0x567228, kFontGlyphCount = 0x567268;
constexpr reg32 kFontPitch = 0x56726c, kFontVersion = 0x567270, kFontGlyphs = 0x567278, kFontPixels = 0x56727c;
constexpr reg32 kFontDraw = 0x567288, kFontBefore = 0x56728c, kFontAfter = 0x567290, kFontReader = 0x567294;
constexpr reg32 kFontPerChar = 0x567298, kKerningCount = 0x56729c, kKerningTable = 0x5672a0, kFontRampGood = 0x5672a8;
constexpr reg32 kPalette32 = 0xa0a814;         // 256 ARGB entries

// Where the error report's stamp goes before a call of sub_401010.
void errorAt(Guest& g, reg32 file, reg32 function, reg32 line)
{
    g.setDword(0x552190, file);
    g.setDword(0x552194, function);
    g.setDword(0x552198, line);
}

void error(Guest& g, reg32 text, std::initializer_list<reg32> values = {})
{
    fe::direct<0x401010, ea::error>(g.app(), g.cpu(), text, guest::Words(values));
}

/* The same by address where what it leaves in eax is handed back, which ea::error does not give. */
reg32 errorLeft(Guest& g, reg32 text, std::initializer_list<reg32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = text;
    for (const reg32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return fe::pointerCall(g, kError, arguments);
}

// sub_501050 on DirectDraw's log.
reg32 report(Guest& g, reg32 level, reg32 text, std::initializer_list<reg32> values = {})
{
    return *fe::direct<0x501050, ea::reportedAt>(g.app(), g.cpu(), kEaLog, level, text, guest::Words(values));
}

/* A COM object's method through its table, the object first, stdcall: for a method the host
 * answers only through the table (the palette's GetEntries, host_video.cpp). */
reg32 methodByTable(Guest& g, reg32 object, reg32 slot, std::initializer_list<reg32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = object;
    for (const reg32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return fe::pointerCall(g, g.dword(g.dword(object) + slot), arguments);
}

/* A DirectDraw object's method at `Offset` in its table, its words handed: the host's, called
 * as C++ on the object where the table has that very method there (host_video.h); anywhere
 * else through the table.  Terminated when the game stopped in it, as g.call. */
template <reg32 Offset, auto Method, typename... Words>
reg32 method(Guest& g, x86::CPU& cpu, reg32 object, Words... words)
{
    const std::optional<reg32> eax = guest::ddraw<Offset, Method>(g.app(), cpu, object, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h).
 * Terminated when the game stopped in it, as g.call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* The same for one the code calls through its import thunk (Winsock's). */
template <reg32 Thunk, auto Function, typename... Words>
reg32 thunked(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::thunked<Thunk, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

reg32 message(Guest& g, reg32 text, std::initializer_list<reg32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = text;
    for (const reg32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return fe::pointerCall(g, kMessage, arguments);
}

// What a loop waiting for another thread does on each pass.
void safepoint(win32::WinApplication* app, x86::CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

// x / 2 rounded towards zero, as Watcom writes it (the sign taken off, then sar 1).
reg32 half(reg32 value)
{
    return reg32(sreg32(value) / 2);
}

// A signed byte at an address the original reads as `mov r, [a - 3]; sar r, 0x18`.
reg32 signedByteAt(Guest& g, reg32 address)
{
    return reg32(sreg32(g.dword(address - 3)) >> 24);
}

// A word read as `mov r, [a - 2]; shr r, 0x10`.
reg32 wordAt(Guest& g, reg32 address)
{
    return g.dword(address - 2) >> 16;
}

/* A thunk's stack argument `index` (0 the first), above the return address. */
reg32 stackWord(win32::WinApplication* app, const x86::CPU& cpu, reg32 index)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * index);
}

/* fe::Frame for a core whose way out is the game's end: esp left where the call that stopped the
 * game left it, as the game's code leaves it (sub_524bb0's exit), else back as fe::Frame puts it. */
class EndingFrame
{
public:
    EndingFrame(Guest& guest, x86::CPU& cpu, reg32 bytes) : m_cpu(cpu), m_esp(cpu.esp)
    {
        cpu.esp -= bytes;
        guest.setFrame(cpu.esp);
    }
    ~EndingFrame()
    {
        if (!m_cpu.terminate)
            m_cpu.esp = m_esp;
    }
    EndingFrame(const EndingFrame&) = delete;
    EndingFrame& operator=(const EndingFrame&) = delete;

private:
    x86::CPU& m_cpu;
    const reg32 m_esp;
};

}  // namespace

/* sub_4ec870: a display (eax, 0 the library's own) put in the mode edx of its table, a
 * negative one 640x480x16; a window's client area made that size instead.  1 when the
 * mode came, 0 when none is known, -1 on DirectDraw's "not exclusive". */
x86::reg32 frontend::eab::ddrawSetDisplayMode(App* app, CPU& cpu, reg32 given, reg32 mode)
{
    constexpr reg32 kGetDisplayMode = 0x30, kSetDisplayMode = 0x54, kSetEntries = 0x18;
    constexpr reg32 kDescription = 0x400, kRect = 0x46c;  // a surface description, a RECT in the frame
    constexpr reg32 kNoOffset = 0xffffec78;               // the offsets not yet known
    constexpr reg32 kExclusive = 0xe1;
    Guest g(app, cpu);
    const fe::Frame ownFrame(g, cpu, 0x4a0);
    const reg32 frame = g.frame();
    report(g, 1, 0x54be18, { mode });
    const reg32 display = given != 0 ? given : kDefaultDisplay;
    const reg32 description = frame + kDescription;
    g.setDword(description, 0x6c);
    const reg32 modeError = method<kGetDisplayMode, &host::ddraw::drawGetDisplayMode>(g, cpu, g.dword(display), description);
    const reg32 heightNow = g.dword(description + 8);
    report(g, 0xe, 0x54be38, { modeError & 0xffff, g.dword(description + 0xc), heightNow, g.dword(description + 0x54) });
    if (g.byte(display + kFullscreen) == 0)
    {
        // In a window: the client area, centred on the desktop if it never had a place.
        const reg32 rect = frame + kRect;
        const auto clientOnScreen = [&] {
            windows<0x53473c, &host::clientRect>(g, cpu, g.dword(display + kWindow), rect);
            windows<0x534700, &host::clientToScreen>(g, cpu, g.dword(display + kWindow), rect);
            windows<0x534700, &host::clientToScreen>(g, cpu, g.dword(display + kWindow), rect + 8);
        };
        clientOnScreen();
        if (sreg32(g.dword(display + kOffsetX)) < sreg32(kNoOffset))
        {
            const reg32 screen = windows<0x534750, &host::systemMetrics>(g, cpu, 0);
            g.setDword(rect, half(screen - g.dword(display + kWidth)));
        }
        if (sreg32(g.dword(display + kOffsetY)) < sreg32(kNoOffset))
        {
            const reg32 screen = windows<0x534750, &host::systemMetrics>(g, cpu, 1);
            g.setDword(rect + 4, half(screen - g.dword(display + kHeight)));
        }
        g.setDword(rect + 8, g.dword(rect) + g.dword(display + kWidth));
        g.setDword(rect + 0xc, g.dword(rect + 4) + g.dword(display + kHeight));
        const reg32 style = windows<0x534758, &host::windowLong>(g, cpu, g.dword(display + kWindow), reg32(-0x10));
        windows<0x5346ec, &host::adjustWindowRect>(g, cpu, rect, style, 0);
        const reg32 left = g.dword(rect), top = g.dword(rect + 4);
        const reg32 wideNow = g.dword(rect + 8) - left, tallNow = g.dword(rect + 0xc) - top;
        windows<0x53478c, &host::setWindowPosition>(g, cpu, g.dword(display + kWindow), 0, left, top, wideNow, tallNow, 0x14);
        clientOnScreen();
        const reg32 leftNow = g.dword(rect);
        g.setDword(display + kWidth, g.dword(rect + 8) - leftNow);
        g.setDword(display + kHeight, g.dword(rect + 0xc) - g.dword(rect + 4));
        g.setDword(display + kOffsetX, g.dword(rect));
        g.setDword(display + kOffsetY, g.dword(rect + 4));
        g.setDword(display + kBits, modeError != 0 ? g.dword(0x56438c) : g.dword(description + 0x54));
        g.setDword(display + kModeNow, mode);
        return 1;
    }
    if (g.dword(display + kModeTable) == 0)
    {
        report(g, 1, 0x54be70);
        return 0;
    }
    reg32 wide, tall, bits, refresh;
    if (sreg32(mode) < 0)
    {
        report(g, 0xe, 0x54bec0);
        wide = 0x280;
        tall = 0x1e0;
        bits = 0x10;
        refresh = 0;
    }
    else
    {
        if (sreg32(mode) >= sreg32(g.dword(display + kModeCount)))
            mode = 0;
        const reg32 entry = g.dword(display + kModeTable) + mode * 20;
        bits = g.byte(entry + 0x12);
        tall = g.dword(entry + 4);
        wide = g.dword(entry);
        refresh = g.dword(entry + 8);
        report(g, 0xe, 0x54be98, { wide, tall, bits });
    }
    if (g.dword(display + kPalette) != 0)
    {
        fe::direct<kFill, ea::clearBytes>(g.app(), g.cpu(), frame, 0x400);
        method<kSetEntries, &host::ddraw::paletteSetEntries>(g, cpu, g.dword(display + kPalette), 0, 0, 0x100, frame);
        g.setWord(display + kFirstChanged, 0);
        g.setWord(display + kLastChanged, 0xff);
    }
    report(g, 0xf, 0x54beec);
    const reg32 failure =
        method<kSetDisplayMode, &host::ddraw::drawSetDisplayMode>(g, cpu, g.dword(display), wide, tall, bits, refresh, 0) & 0xffff;
    report(g, 0xc, 0x54bf1c, { failure });
    g.setDword(description, 0x6c);
    method<kGetDisplayMode, &host::ddraw::drawGetDisplayMode>(g, cpu, g.dword(display), description);
    // What the mode gives past what was asked for is a border round the picture.
    const reg32 gotWide = g.dword(description + 0xc);
    if (wide < gotWide)
    {
        g.setDword(display + kWidth, wide);
        g.setDword(display + kOffsetX, (gotWide - wide) >> 1);
    }
    else
    {
        g.setDword(display + kWidth, gotWide);
    }
    const reg32 gotTall = g.dword(description + 8);
    if (tall < gotTall)
    {
        g.setDword(display + kHeight, tall);
        g.setDword(display + kOffsetY, (gotTall - tall) >> 1);
    }
    else
    {
        g.setDword(display + kHeight, gotTall);
    }
    reg32 result = 0;
    if (failure == 0 && bits == g.dword(description + 0x54))
    {
        result = 1;
    }
    else if (failure == kExclusive)
    {
        result = reg32(-1);
    }
    else
    {
        // Marked in the table as a mode that does not work.
        const reg32 flags = g.dword(display + kModeTable) + mode * 20 + 0xf;
        g.setByte(flags, g.byte(flags) | 0x80);
    }
    g.setDword(display + kBits, g.dword(description + 0x54));
    g.setDword(display + kModeNow, mode);
    windows<0x534468, &host::registryFlush>(g, cpu, 0x80000000);
    const reg32 bitsGot = g.dword(display + kBits);
    report(g, 2, 0x54bf4c, { wide, tall, bits, g.dword(display + kWidth), g.dword(display + kHeight), bitsGot });
    return result;
}

x86::reg32 frontend::ddrawSetDisplayMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ec870", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 given = args.eax, mode = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::ddrawSetDisplayMode(app, cpu, given, mode); });
}

/* sub_4ecd50: a display's surfaces, its DirectDraw object and its table of modes gone. */
x86::reg32 frontend::eab::ddrawDisposeDisplay(App* app, CPU& cpu, reg32 given)
{
    constexpr reg32 kRelease = 8;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 display = given != 0 ? given : kDefaultDisplay;
    if (g.dword(display) == 0)
        return given;
    report(g, 1, 0x54bf8c);
    windows<kEnterCriticalSection, &host::enterLock>(g, cpu, g.dword(kDisplayLock));
    guest::direct<0x4ec0d0, eaa::ddrawRemoveSurfaces>(app, cpu, display);
    report(g, 0xa, 0x54bfa8);
    method<kRelease, &host::ddraw::release>(g, cpu, g.dword(display));
    report(g, 0xa, 0x54bfc8);
    const reg32 modes = g.dword(display + kModeTable);
    if (modes != 0)
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, modes);
    g.setDword(display + kModeTable, 0);
    g.setDword(display + 4, 0);
    g.setDword(display + kPalette, 0);
    g.setDword(display, 0);
    return windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
}

x86::reg32 frontend::ddrawDisposeDisplay(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ecd50", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 given = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::ddrawDisposeDisplay(app, cpu, given); });
}

/* sub_4ecf60: the colours changed since the last time told to the display's palette,
 * unless a surface is locked (an error) or the display has no palette. */
x86::reg32 frontend::eab::ddrawApplyPalette(App* app, CPU& cpu, reg32 given)
{
    constexpr reg32 kGetEntries = 0x10, kSetEntries = 0x18;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 display = given != 0 ? given : kDefaultDisplay;
    reg32 eax = given;
    if (g.dword(display + kLockedSurface) != 0)
    {
        errorAt(g, 0x54bb1c, 0x54bfd4, 0x37e);
        eax = errorLeft(g, 0x54bfe4);
    }
    if (g.dword(display + kPalette) == 0 || sreg32(g.dword(display + kBits)) > 8)
        return eax;
    report(g, 1, 0x54c024);
    const reg32 first = reg32(sreg32(g.dword(display + kFirstChanged - 2)) >> 16);
    const reg32 span = reg32(sreg32(g.dword(display + kLastChanged - 2)) >> 16) - first;
    const reg32 count = span + 1;
    reg32 result = span;
    if (sreg32(first) < 0x100 && sreg32(count) > 0)
    {
        const reg32 entries = display + kPaletteEntries + first * 4;
        const reg32 palette = g.dword(display + kPalette);
        const reg32 failure =
            method<kSetEntries, &host::ddraw::paletteSetEntries>(g, cpu, palette, 0, first, count, entries) & 0xffff;
        const reg32 reported = report(g, 1, 0x54c03c, { first, count, failure });
        const reg32 fullscreen = g.byte(display + kFullscreen);
        result = (reported & 0xffff00ff) | (fullscreen << 8);
        if (fullscreen == 0)
            result = methodByTable(g, g.dword(display + kPalette), kGetEntries, { 0, 0, 0x100, entries });
    }
    g.setWord(display + kLastChanged, 0xffff);
    g.setWord(display + kFirstChanged, 0x100);
    return result;
}

x86::reg32 frontend::ddrawApplyPalette(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ecf60", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 given = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::ddrawApplyPalette(app, cpu, given); });
}

/* sub_4ed0f0: the surface edx of a display locked, its bits named in a bitmap (ebx, 0 for
 * none); what was locked before unlocked first.  1, or 0 when it could not be had: a
 * full-screen game out of focus or minimised, or Lock's error. */
x86::reg32 frontend::eab::ddrawLockBitmap(App* app, CPU& cpu, reg32 given, reg32 surface, reg32 bitmap)
{
    constexpr reg32 kLockedNow = 0x5643e8, kLockFailed = 0x564ee8, kLockTime = 0x564eec, kLockHeld = 0x564ee0;
    constexpr reg32 kNotNow = 0xeacbad;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x7c);
    const reg32 description = g.frame();
    const reg32 display = given != 0 ? given : kDefaultDisplay;
    if (surface != 0 && surface == g.dword(display + kLockedSurface))
        return 1;
    if (g.dword(display + kLockedSurface) != 0)
        guest::direct<0x4ed2a0, ddrawUnlockBitmap>(app, cpu, display);
    if (surface == 0)
    {
        g.setDword(kLockedNow, 0);
        return 1;
    }
    const reg32 lock = g.dword(kDisplayLock);
    windows<kEnterCriticalSection, &host::enterLock>(g, cpu, lock);
    bool mayLock = true;
    if (g.byte(display + kFullscreen) != 0)
    {
        if (fe::direct<0x4f9120, wina::hasFocus>(app, cpu) == 0)
            mayLock = false;
        else if (windows<0x53475c, &host::isIconic>(g, cpu, g.dword(display + kWindow)) != 0)
            mayLock = false;
    }
    const reg32 status =
        mayLock ? guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, cpu, display, surface, description) : kNotNow;
    g.setDword(kLockTime, windows<kGetTickCount, &host::ticks>(g, cpu));
    if (status != 0)
    {
        const reg32 failedBefore = g.dword(kLockFailed);
        g.setDword(kLockHeld, reg32(-1));
        if (failedBefore != 0)
        {
            report(g, 4, 0x54c064, { status });
            g.setDword(kLockFailed, 0);
        }
        const reg32 held = g.dword(kDisplayLock);
        g.setDword(display + kLockedBits, 0);
        g.setDword(display + kLockedSurface, 0);
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, held);
        g.setDword(kLockedNow, 1);
        return 0;
    }
    if (g.dword(kLockFailed) == 0)
    {
        report(g, 4, 0x54c09c);
        g.setDword(kLockFailed, 1);
    }
    const reg32 bits = g.dword(description + 0x24);
    g.setDword(display + kLockedSurface, surface);
    g.setDword(display + kLockedBits, bits);
    if (bitmap != 0)
    {
        const reg32 pitch = g.dword(description + 0x10);
        reg32 first = g.dword(description + 0x24);
        // A window's picture starts where the window is on the primary surface.
        if (g.byte(display + kFullscreen) == 0 && surface == g.dword(display + kPrimary))
        {
            first = g.dword(display + kOffsetY) * pitch
                + (g.dword(description + 0x54) >> 3) * g.dword(display + kOffsetX) + g.dword(description + 0x24);
        }
        fe::direct<0x508da0, menz::videoWindowUpdate>(app, cpu, bitmap, pitch, first);
    }
    g.setDword(kLockedNow, 0);
    return 1;
}

x86::reg32 frontend::ddrawLockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ed0f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 given = args.eax, surface = args.edx, bitmap = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::ddrawLockBitmap(app, cpu, given, surface, bitmap); });
}

/* sub_4ed2a0: the surface locked unlocked, how long it was held noted. */
x86::reg32 frontend::eab::ddrawUnlockBitmap(App* app, CPU& cpu, reg32 given)
{
    constexpr reg32 kLockedNow = 0x5643e8, kUnlock = 0x80;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 display = given != 0 ? given : kDefaultDisplay;
    const reg32 surface = g.dword(display + kLockedSurface);
    if (surface == 0)
    {
        g.setDword(kLockedNow, 0);
        return given;
    }
    method<kUnlock, &host::ddraw::surfaceUnlock>(g, cpu, surface, g.dword(display + kLockedBits));
    g.setDword(display + kLockedSurface, 0);
    g.setDword(display + kLockedBits, 0);
    const reg32 held = windows<kGetTickCount, &host::ticks>(g, cpu) - g.dword(0x564eec);
    g.setDword(0x564ee0, held);
    if (sreg32(held) > sreg32(g.dword(0x564ee4)))
        g.setDword(0x564ee4, held);
    windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));  // leaves eax as it is: held
    g.setDword(kLockedNow, 0);
    return held;
}

x86::reg32 frontend::ddrawUnlockBitmap(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ed2a0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 given = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::ddrawUnlockBitmap(app, cpu, given); });
}

/* sub_4eee30: a bitmap of eax by edx pixels of ebx bits (0 the screen's) in the memory
 * flags of ecx: its header, a shape header at 0x38 and its pixels below; made with the
 * rows when the library can draw.  The stack's argument says to complain on failure; 0
 * on failure. */
x86::reg32 frontend::eab::windowBitmapCreate(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 flags,
                                             reg32 complain)
{
    constexpr reg32 kFile = 0x54c2b4, kFunction = 0x54c2c4, kBitmapMagic = 0x57494e44;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x34);
    const reg32 name = g.frame();
    reg32 depth = bits != 0 ? bits : g.dword(0x56438c);
    reg32 cellBits = depth == 0xf ? 0x10 : depth;
    if (depth == 0)
    {
        depth = 8;
        cellBits = 8;
    }
    if (sreg32(width) < 0 || sreg32(width) > 0x4000 || sreg32(height) < 0 || sreg32(height) > 0x2000)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x54);
            error(g, 0x54c2d8, { width, height, depth, 0x4000, 0x2000 });
        }
        return 0;
    }
    if (fe::direct<0x5098d0, watb::shapeTypeForDepth>(app, cpu, depth) == 0)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x5f);
            error(g, 0x54c328, { width, height, depth });
        }
        return 0;
    }
    const reg32 roundedBits = (width * cellBits + 7) & ~reg32(7);
    const reg32 wholeBits = reg32((sreg32(roundedBits) / sreg32(cellBits)) * sreg32(cellBits));
    const reg32 rowBytes = reg32(sreg32(wholeBits) >> 3);
    const reg32 size = rowBytes * height + 0x48;
    fe::direct<0x4df690, watcom::clib::sprintf>(app, cpu, name, 0x54c37cu, guest::Words({ width, height, depth }));
    errorAt(g, kFile, kFunction, 0x6a);
    const reg32 block = *fe::direct<kReserve, ea::reserved>(app, cpu, name, size, flags);
    if (block == 0)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x70);
            g.setDword(0x564400, g.dword(0x564400) + 1);
            const reg32 largest = fe::direct<0x4e1580, ea::defaultLargest>(app, cpu);
            error(g, 0x54c388, { width, height, depth, size, largest });
        }
        return 0;
    }
    fe::direct<kFill, ea::clearBytes>(g.app(), g.cpu(), block, 0x48);
    const reg32 header = block + 0x38;
    const reg32 type = fe::direct<0x5098d0, watb::shapeTypeForDepth>(app, cpu, depth);  // header in edx too, not read
    g.setByte(header, 0);
    const reg32 headerWord = g.dword(header) | (type & 0xff);
    g.setWord(header + 4, width);
    g.setDword(header, headerWord);
    g.setWord(header + 6, height);
    g.setDword(block, kBitmapMagic);
    g.setDword(block + 4, width);
    g.setDword(block + 0x14, width);
    g.setDword(block + 8, height);
    g.setDword(block + 0x18, height);
    g.setByte(block + 0x1c, depth & 0xff);
    g.setDword(block + 0x20, header);
    g.setByte(block + 0x1d, g.dword(header) & 0xff);
    const reg32 canDraw = g.dword(0x568380);
    if (canDraw == 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x8b);
            error(g, 0x54c3d8, { width, height, depth });
        }
        return 0;
    }
    g.setDword(block + 0x28, rowBytes);
    const reg32 scanRow = fe::direct<0x509a30, watb::makeRow>(app, cpu, 0x10u, rowBytes, 0u, height);
    g.setDword(block + 0x2c, scanRow);
    const reg32 pixelRow = fe::direct<0x509a30, watb::makeRow>(app, cpu, 0u, cellBits, 3u, width);
    const reg32 scanRowKept = g.dword(block + 0x2c);
    g.setDword(block + 0x30, pixelRow);
    if (scanRowKept != 0 && pixelRow != 0)
        return block;
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    if (complain != 0)
    {
        errorAt(g, kFile, kFunction, 0x99);
        error(g, 0x54c444, { width, height });
    }
    return 0;
}

x86::reg32 frontend::windowBitmapCreate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4eee30", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 width = args.eax, height = args.edx, bits = args.ebx, flags = args.ecx;
    const reg32 complain = stackWord(app, cpu, 0);
    return runThunk(app, cpu, args, site, [&](Guest&) {
        return eab::windowBitmapCreate(app, cpu, width, height, bits, flags, complain);
    });
}

/* sub_4ef190: sub_4eee30, complaining as the library's flag at 0x56440c says. */
x86::reg32 frontend::eab::windowBitmapCreateDefault(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits,
                                                    reg32 flags)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    const reg32 complain = g.dword(0x56440c);
    return guest::direct<0x4eee30, windowBitmapCreate>(app, cpu, width, height, bits, flags, complain);
}

x86::reg32 frontend::windowBitmapCreateDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef190", 0, 0, nullptr, 0, kRestoreEsi };
    const reg32 width = args.eax, height = args.edx, bits = args.ebx, flags = args.ecx;
    return runThunk(app, cpu, args, site, [&](Guest&) {
        return eab::windowBitmapCreateDefault(app, cpu, width, height, bits, flags);
    });
}

/* sub_4ef1b0: the clip rectangle filled with the colour in eax. */
x86::reg32 frontend::eab::drawClearClip(App* app, CPU& cpu, reg32 colour)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 left = g.dword(0x565000), top = g.dword(0x565004);
    const reg32 wide = g.dword(0x565008) - left, tall = g.dword(0x56500c) - top;
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, left, top, wide, tall, colour);
}

x86::reg32 frontend::drawClearClip(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef1b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 colour = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::drawClearClip(app, cpu, colour); });
}

/* sub_4ef1e0: the shape named by the four bytes at edx in the shape file eax: last to
 * first, the first entry the fallback.  None: 0, and an error if the flag at 0x56440c
 * says to. */
x86::reg32 frontend::eab::shapeFileFind(App* app, CPU& cpu, reg32 file, reg32 nameAt)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 wanted = g.dword(nameAt);
    reg32 edxNow = nameAt;
    reg32 index = g.dword(file + 8);
    if (index != 0)
    {
        do
        {
            --index;
        } while (index != 0 && wanted != g.dword(file + index * 8 + 0x10));
        edxNow = file + index * 8;
        if (wanted == g.dword(file + index * 8 + 0x10))
            return g.dword(file + index * 8 + 0x14) + file;
    }
    if (g.dword(0x56440c) == 0)
        return 0;
    const reg32 found = fe::direct<0x509c00, watb::findBlockHeader>(app, cpu, file, edxNow);
    const reg32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), found);
    const reg32 shown = blockName != 0 ? blockName : 0x54c47c;
    errorAt(g, 0x54c480, 0x54c490, 0x4f);
    error(g, 0x54c49c, { nameAt, shown });
    return 0;
}

x86::reg32 frontend::shapeFileFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef1e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 file = args.eax, nameAt = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::shapeFileFind(app, cpu, file, nameAt); });
}

/* sub_4ef2b0: ebx 4-bit pixels from eax to edx, a zero one not copied. */
x86::reg32 frontend::eab::copyNibblesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 pixels)
{
    Guest g(app, cpu);
    const sreg32 count = sreg32(pixels);
    sreg32 i = 0;
    while (i < count)
    {
        const reg32 at = reg32(i >> 1);
        const std::uint8_t source = g.byte(from + at);
        const reg32 pixel = (i & 1) != 0 ? reg32(source) >> 4 : reg32(source & 0xf);
        if (pixel == 0)
        {
        }
        else
        {
            const std::uint8_t kept = g.byte(to + at);
            const reg32 value = (i & 1) != 0 ? (pixel << 4) | (kept & 0xf) : pixel | (kept & 0xf0);
            g.setByte(to + at, std::uint8_t(value));
        }
        ++i;
    }
    return reg32(i);
}

x86::reg32 frontend::copyNibblesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef2b0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 from = args.eax, to = args.edx, pixels = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::copyNibblesKeyed(app, cpu, from, to, pixels); });
}

/* sub_4ef320: ebx bytes from eax to edx, a 0xff not copied. */
x86::reg32 frontend::eab::copyBytesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    Guest g(app, cpu);
    do
    {
        const std::uint8_t pixel = g.byte(from++);
        if (pixel != 0xff)
            g.setByte(to, pixel);
        ++to;
    } while (--count != 0);
    return from;
}

x86::reg32 frontend::copyBytesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef320", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::copyBytesKeyed(app, cpu, from, to, count); });
}

/* sub_4ef340: ebx words from eax to edx, a zero one not copied. */
x86::reg32 frontend::eab::copyWordsKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    Guest g(app, cpu);
    do
    {
        const std::uint16_t pixel = g.word(from);
        if (pixel != 0)
            g.setWord(to, pixel);
        to += 2;
        from += 2;
    } while (--count != 0);
    return from;
}

x86::reg32 frontend::copyWordsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef340", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::copyWordsKeyed(app, cpu, from, to, count); });
}

/* sub_4ef370: ebx 3-byte pixels from eax to edx, an all-zero one not copied. */
x86::reg32 frontend::eab::copyTriplesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    Guest g(app, cpu);
    reg32 result;
    do
    {
        result = g.dword(from - 1) >> 8;
        if (result != 0)
        {
            g.setByte(to, g.byte(from));
            g.setByte(to + 1, g.byte(from + 1));
            const std::uint8_t last = g.byte(from + 2);
            g.setByte(to + 2, last);
            result = (result & 0xffffff00) | last;
        }
        to += 3;
        from += 3;
    } while (--count != 0);
    return result;
}

x86::reg32 frontend::copyTriplesKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef370", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::copyTriplesKeyed(app, cpu, from, to, count); });
}

/* sub_4ef3c0: ebx ARGB pixels from eax to edx, one with no alpha not copied. */
x86::reg32 frontend::eab::copyLongsKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    Guest g(app, cpu);
    do
    {
        const reg32 pixel = g.dword(from);
        if ((pixel >> 24) != 0)
            g.setDword(to, pixel);
        to += 4;
        from += 4;
    } while (--count != 0);
    return from;
}

x86::reg32 frontend::copyLongsKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef3c0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const reg32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::copyLongsKeyed(app, cpu, from, to, count); });
}

/* sub_4ef3f0: a shape (eax) drawn at edx, ebx with its key colour clear: the palette's
 * entry 0 for 4 bits, 0xff else, emptied for the time; an 8-bit shape on a 16-bit screen
 * through a palette of its own, made in the frame. */
x86::reg32 frontend::eab::shapeDrawKeyed(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x414);
    const reg32 bits = g.byte((g.dword(shape) & 0x7f) + kShapeBits);
    const reg32 key = kPalette32 + (bits == 4 ? 0 : 0xff) * 4;
    const reg32 keptColour = g.dword(key);
    g.setDword(0x5650b4, 0);
    g.setDword(key, 0);
    if (bits == 8 && g.byte(0x565010) == 0x10)
    {
        // The game's code hands it y and 0x7b in ebx and ecx too, which it does not read.
        const reg32 pointer = fe::direct<0x50a830, watb::clut16aPointer>(app, cpu, shape, g.frame());
        g.setDword(0x9f09b0, pointer);
        g.setDword(0x5650bc, 0x7b);
        g.setDword(0x5650b4, 0x50afc0);
    }
    fe::direct<0x50b0b0, watb::moveShape>(app, cpu, shape, x, y, 0x56509cu);
    g.setDword(0x5650bc, 0);
    g.setDword(key, keptColour);
    return keptColour;
}

x86::reg32 frontend::shapeDrawKeyed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef3f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 shape = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::shapeDrawKeyed(app, cpu, shape, x, y); });
}

/* sub_4ef4c0: sub_4ef3f0 with the shape's own offset taken off the position. */
x86::reg32 frontend::eab::shapeDrawKeyedAtHotspot(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 top)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    const reg32 y = top - reg32(sreg32(g.dword(shape + 8)) >> 16);
    const reg32 offsetX = reg32(sreg32(g.dword(shape + 6)) >> 16);
    // The game's code hands it offsetX in ecx too, which it does not read.
    return guest::direct<0x4ef3f0, shapeDrawKeyed>(app, cpu, shape, x - offsetX, y);
}

x86::reg32 frontend::shapeDrawKeyedAtHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef4c0", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 shape = args.eax, x = args.edx, top = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::shapeDrawKeyedAtHotspot(app, cpu, shape, x, top); });
}

/* sub_4ef520: the font at eax taken into use. */
x86::reg32 frontend::eab::fontUse(App* app, CPU& cpu, reg32 font)
{
    constexpr reg32 kFontMagic = 0x464e5400;  // "FNT" in the file's first three bytes
    constexpr reg32 kReaderBytes = 0x50b540, kReaderWords = 0x50b580;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    const reg32 first = g.dword(font);
    const reg32 tag = ((first >> 24) | ((first >> 8) & 0xff00) | ((first << 8) & 0xff0000) | (first << 24)) & 0xffffff00;
    if (tag != kFontMagic)
    {
        errorAt(g, 0x54c4cc, 0x54c4dc, 0x54);
        error(g, 0x54c4e4, { font });
    }
    g.setDword(kFontOriginX, signedByteAt(g, font + 0x10));
    g.setDword(kFontOriginY, signedByteAt(g, font + 0x11));
    g.setDword(kFontAscent, signedByteAt(g, font + 0x12));
    const reg32 descent = signedByteAt(g, font + 0x13);
    g.setDword(0x567274, font);
    g.setDword(kFontDescent, descent);
    const reg32 height = descent + g.dword(kFontAscent);
    const reg32 pixels = g.dword(font + 0x1c) + font;
    g.setDword(kFontVersion, 0x64);
    g.setDword(0x56720c, 1);
    g.setDword(kFontPixels, pixels);
    g.setDword(kFontGlyphCount, g.word(font + 0xa));
    g.setDword(kFontHeight, height);
    g.setDword(0x5672a4, 0);
    g.setDword(kKerningTable, 0);
    g.setDword(0x56721c, height);
    g.setDword(kFontGlyphs, font + g.dword(font + 0x14));
    const reg32 pixelBits = g.byte((g.dword(pixels) & 0x7f) + kShapeBits);
    g.setDword(kFontBits, pixelBits);
    const reg32 wide = reg32(sreg32(g.dword(pixels + 2)) >> 16);
    const reg32 rowBits = (pixelBits * wide + 7) & ~reg32(7);
    g.setDword(kFontPitch, reg32(sreg32(rowBits) >> 3));
    // Wide characters when the file says so, or when it has more glyphs than a byte counts.
    const bool wideText = (reg32(sreg32(g.dword(font + 0xc)) >> 16) & 3) == 2
        || sreg32(g.dword(kFontGlyphCount)) > 0xff || wordAt(g, font + 0x20) > 0xff;
    g.setDword(kFontReader, wideText ? kReaderWords : kReaderBytes);
    g.setDword(kFontColourA, 0);
    g.setDword(kFontColourB, 0);
    g.setDword(kFontRampGood, 0);
    fe::direct<kFill, ea::clearBytes>(g.app(), g.cpu(), kFontRamp, 0x40);
    const reg32 result = fe::direct<0x50c0d0, watb::initTextDraw>(app, cpu);
    g.setDword(kFontPerChar, 0);
    return result;
}

x86::reg32 frontend::fontUse(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef520", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 font = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fontUse(app, cpu, font); });
}

/* sub_4ef6c0: the palette's entry nearest to the colour in edx, of those in use at
 * 0x5650c4. */
x86::reg32 frontend::eab::paletteNearest(App* app, CPU& cpu, reg32 colour)
{
    Guest g(app, cpu);
    const sreg32 red = sreg32((colour >> 16) & 0xff), green = sreg32((colour >> 8) & 0xff), blue = sreg32(colour & 0xff);
    sreg32 nearest = 0x1869f;
    reg32 best = 0;
    for (reg32 i = 0; i < 0x100; ++i)
    {
        const reg32 entry = kPalette32 + i * 4;
        if (g.byte(0x5650c4 + i) == 0)
            continue;
        const sreg32 dg = green - sreg32(g.byte(entry + 1));
        const sreg32 dr = red - sreg32(g.byte(entry + 2));
        const sreg32 db = blue - sreg32(g.byte(entry));
        const sreg32 distance = db * db + dg * dg + dr * dr;
        if (distance < nearest)
        {
            best = i;
            nearest = distance;
        }
    }
    return best;
}

x86::reg32 frontend::paletteNearest(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef6c0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 colour = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::paletteNearest(app, cpu, colour); });
}

/* sub_4ef760: the ARGB colour in eax as the screen's depth (0x565010) stores it. */
x86::reg32 frontend::eab::colourPack(App* app, CPU& cpu, reg32 colour)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const std::uint8_t depth = g.byte(0x565010);
    const reg32 blue5 = (colour >> 3) & 0x1f;
    if (depth <= 8)
    {
        const reg32 table = g.dword(0x5650c0);
        if (table != 0)
            return g.byte(table + (((colour & 0xf80000) >> 9) + blue5 + ((colour & 0xf800) >> 6)));
        return guest::direct<0x4ef6c0, paletteNearest>(app, cpu, colour);  // the palette in eax too, not read
    }
    if (depth == 0xf)
        return colour == 0 ? 0 : ((colour & 0xf80000) >> 9) + blue5 + ((colour & 0xf800) >> 6) + 0x8000;
    if (depth == 0x10)
        return ((colour & 0xf80000) >> 8) + blue5 + ((colour & 0xfc00) >> 5);
    return depth < 0x18 ? 0 : colour;
}

x86::reg32 frontend::colourPack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef760", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 colour = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::colourPack(app, cpu, colour); });
}

/* sub_4ef830: the font's two colours (eax, edx): sixteen steps from one to the other for
 * the shades of an anti-aliased glyph, packed for the screen. */
x86::reg32 frontend::eab::fontSetColours(App* app, CPU& cpu, reg32 ink, reg32 back)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x48);
    const reg32 depth = g.byte(0x565010);
    g.setDword(0x5672a4, 0);
    reg32 result = (ink & 0xffff00ff) | (depth << 8);
    if (depth == 8 && ink >= 0x100)
    {
        errorAt(g, 0x54c534, 0x54c544, 0x3c);
        result = errorLeft(g, 0x54c550, { depth, ink, back });
    }
    if (ink == g.dword(kFontColourA) && back == g.dword(kFontColourB) && g.dword(kFontRampGood) != 0)
        return result;
    g.setDword(kFontColourA, ink);
    g.setDword(kFontColourB, back);
    g.setDword(kFontRampGood, 0);
    const reg32 inkArgb = fe::direct<0x50c3a0, watb::colourToArgb>(app, cpu, ink);
    const reg32 backArgb = fe::direct<0x50c3a0, watb::colourToArgb>(app, cpu, back);
    const reg32 inkRed = (inkArgb >> 16) & 0xff, inkGreen = (inkArgb >> 8) & 0xff, inkBlue = inkArgb & 0xff;
    const reg32 backRed = (backArgb >> 16) & 0xff, backGreen = (backArgb >> 8) & 0xff, backBlue = backArgb & 0xff;
    for (reg32 step = 0; step < 16; ++step)
    {
        const reg32 red = (backRed * (15 - step) + inkRed * step) >> 4;
        const reg32 green = (backGreen * (15 - step) + inkGreen * step) >> 4;
        const reg32 blue = (backBlue * (15 - step) + inkBlue * step) >> 4;
        const reg32 packed =
            guest::direct<0x4ef760, colourPack>(app, cpu, (step << 28) | (red << 16) | (green << 8) | blue);
        g.setDword(kFontRamp + step * 4, packed);
    }
    return fe::direct<0x50c180, watb::textPalette>(app, cpu);
}

x86::reg32 frontend::fontSetColours(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ef830", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 ink = args.eax, back = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fontSetColours(app, cpu, ink, back); });
}

/* sub_4efa10: the record for the character eax among ebx records of ecx bytes at edx,
 * sorted by the word at each one's start; 0 if none. */
x86::reg32 frontend::eab::fontGlyphSearch(App* app, CPU& cpu, reg32 character, reg32 table, reg32 count, reg32 stride)
{
    Guest g(app, cpu);
    reg32 left = count;
    while (left != 0)
    {
        const reg32 probe = table + reg32(sreg32(left) >> 1) * stride;
        const reg32 difference = character - wordAt(g, probe);
        if (difference == 0)
            return probe;
        if (sreg32(difference) > 0)
        {
            --left;
            table = probe + stride;
        }
        left = reg32(sreg32(left) >> 1);
    }
    return 0;
}

x86::reg32 frontend::fontGlyphSearch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efa10", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 character = args.eax, table = args.edx, count = args.ebx, stride = args.ecx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fontGlyphSearch(app, cpu, character, table, count, stride); });
}

/* sub_4efa70: the glyph of the character eax in the font in use: where a table of the
 * printable characters would have it, else searched for. */
x86::reg32 frontend::eab::fontGlyphFind(App* app, CPU& cpu, reg32 character)
{
    constexpr reg32 kGlyphBytes = 0xb;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    const reg32 version = g.dword(kFontVersion);
    const reg32 glyphs = g.dword(kFontGlyphs);
    if (sreg32(version) < 0x64)
    {
        errorAt(g, 0x54c5ac, 0x54c5bc, 0x64);
        error(g, 0x54c5cc);
    }
    const reg32 guess = glyphs + (character - 0x20) * kGlyphBytes;
    if (wordAt(g, guess) == character)
        return guess;
    return guest::direct<0x4efa10, fontGlyphSearch>(app, cpu, character, glyphs, g.dword(kFontGlyphCount), kGlyphBytes);
}

x86::reg32 frontend::fontGlyphFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efa70", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 character = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fontGlyphFind(app, cpu, character); });
}

/* sub_4efb10: the kerning of the pair eax, edx in the font's table. */
x86::reg32 frontend::eab::fontKerning(App* app, CPU& cpu, reg32 character, reg32 previous)
{
    Guest g(app, cpu);
    const std::uint16_t second = std::uint16_t(character), first = std::uint16_t(previous);
    const reg32 count = g.dword(kKerningCount);
    reg32 pair = g.dword(kKerningTable);
    reg32 i = 0;
    for (; sreg32(i) < sreg32(count); ++i, pair += 8)
    {
        if (second == g.word(pair + 2) && first == g.word(pair))
        {
            return signedByteAt(g, pair + 4);
        }
    }
    return 0;
}

x86::reg32 frontend::fontKerning(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efb10", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi };
    const reg32 character = args.eax, previous = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fontKerning(app, cpu, character, previous); });
}

/* sub_4efb50: the text at eax drawn at the pen, which ends where it began: newlines,
 * tabs on the stops at 0x5651c8, kerning, and a box for what has no glyph if asked. */
x86::reg32 frontend::eab::textDrawAtPen(App* app, CPU& cpu, reg32 text)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x5c);
    const reg32 cursor = g.frame();           // what the font's reader moves along
    const reg32 offsetX = g.frame() + 4, offsetY = g.frame() + 8;  // what the per-character hook gives
    reg32 x = g.dword(kPenX) + g.dword(kFontOriginX);
    reg32 y = g.dword(kPenY) + g.dword(kFontOriginY);
    g.setDword(cursor, text);
    const reg32 draw = g.dword(kFontDraw), reader = g.dword(kFontReader);
    const reg32 perChar = g.dword(kFontPerChar), kerning = g.dword(kKerningTable);
    const reg32 lineStart = g.dword(kPenX);
    if (text == 0)
    {
        errorAt(g, 0x54c5ac, 0x54c600, 0x9e);
        error(g, 0x54c608);
    }
    if (g.dword(kFontBefore) != 0)
        fe::pointerCall(g, g.dword(kFontBefore));
    reg32 spaceAdvance = 0;
    const reg32 space = guest::direct<0x4efa70, fontGlyphFind>(app, cpu, 0x20u);
    if (space != 0)
        spaceAdvance = signedByteAt(g, space + 8);
    reg32 character = 0;
    reg32 index = reg32(-1);
    for (;;)
    {
        const reg32 previous = character;
        x86::Args readerLeft;
        character = fe::pointerCall(g, reader, args({ cursor, index + 1 }), readerLeft);
        index = readerLeft.edx;
        if (sreg32(character) <= 0x20)
        {
            if (character == 0)
                break;
            if (character == 10)
            {
                const reg32 line = g.dword(kFontAscent) + g.dword(kFontDescent);
                x = lineStart;
                y += line;
                continue;
            }
            if (g.dword(0x5651c8) == 0 || character != 9)
            {
                x += spaceAdvance;
                continue;
            }
            const sreg32 stops = sreg32(g.dword(0x5651c4));
            for (sreg32 i = 0; i < stops; ++i)
            {
                const reg32 stop = g.dword(g.dword(0x5651c8) + reg32(i) * 4);
                if (sreg32(x) < sreg32(stop))
                {
                    x = stop;
                    break;
                }
            }
            continue;
        }
        const reg32 glyph = guest::direct<0x4efa70, fontGlyphFind>(app, cpu, character);
        if (glyph == 0)
        {
            if (g.dword(0x5651cc) != 0)
            {
                const reg32 tall = half(g.dword(kFontHeight));
                const reg32 wide = half(tall);
                // The game's code hands it y in edx too, which it keeps: the box's top, its height tall.
                const reg32 colour = guest::direct<0x4ef760, colourPack>(app, cpu, 0xffaa0000u);
                fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, x, y, wide - 2, tall, colour);
                x += wide;
            }
            continue;
        }
        const reg32 glyphWide = wordAt(g, glyph + 4), glyphTall = wordAt(g, glyph + 6);
        g.setDword(offsetX, 0);
        g.setDword(offsetY, 0);
        if (kerning != 0)
            x += guest::direct<0x4efb10, fontKerning>(app, cpu, character, previous);
        const reg32 penX = x + signedByteAt(g, glyph + 9);
        const reg32 penY = y + signedByteAt(g, glyph + 0xa);
        if (perChar != 0)
            fe::pointerCall(g, perChar, args({ offsetX, offsetY, text, index }));
        fe::pointerCall(g, draw, args({ penX + g.dword(offsetX), penY + g.dword(offsetY), g.dword(kFontPixels), glyphWide },
                          { glyphTall, g.byte(glyph + 2), g.byte(glyph + 3), g.dword(kFontPitch) }));
        x += signedByteAt(g, glyph + 8);
    }
    if (g.dword(kFontAfter) != 0)
        fe::pointerCall(g, g.dword(kFontAfter));
    const reg32 originX = g.dword(kFontOriginX);
    const reg32 originY = g.dword(kFontOriginY);
    g.setDword(kPenX, x - originX);
    g.setDword(kPenY, y - originY);
    return originY;
}

x86::reg32 frontend::textDrawAtPen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efb50", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 text = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::textDrawAtPen(app, cpu, text); });
}

/* sub_4efe30: the pen moved to edx, ebx and the text at eax drawn. */
x86::reg32 frontend::eab::textDrawAt(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    fe::direct<0x4f0850, wata::textSetPen>(app, cpu, x, y);
    return guest::direct<0x4efb50, textDrawAtPen>(app, cpu, text);
}

x86::reg32 frontend::textDrawAt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efe30", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 text = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::textDrawAt(app, cpu, text, x, y); });
}

/* sub_4efe50: how wide the first edx characters of the text at eax are. */
x86::reg32 frontend::eab::textMeasureUpTo(App* app, CPU& cpu, reg32 text, reg32 limit)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x20);
    const reg32 cursor = g.frame();
    g.setDword(cursor, text);
    const reg32 kerning = g.dword(kKerningTable);
    reg32 width = 0, previous = 0;
    sreg32 count = 0;
    if (text == 0)
    {
        errorAt(g, 0x54c624, 0x54c634, 0x47);
        error(g, 0x54c640);
    }
    x86::Args readerLeft;  // what the reader leaves in edx
    while (count < sreg32(limit))
    {
        const reg32 character = fe::pointerCall(g, g.dword(kFontReader), args({ cursor, previous }), readerLeft);
        previous = character;
        if (character == 0)
            break;
        const reg32 glyph = guest::direct<0x4efa70, fontGlyphFind>(app, cpu, character);
        ++count;
        if (glyph == 0)
            continue;
        width += signedByteAt(g, glyph + 8);
        if (kerning != 0)
            width += guest::direct<0x4efb10, fontKerning>(app, cpu, character, readerLeft.edx);  // edx as the reader left it
    }
    return width;
}

x86::reg32 frontend::textMeasureUpTo(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efe50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 text = args.eax, limit = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::textMeasureUpTo(app, cpu, text, limit); });
}

/* sub_4efef0: how wide a text is, up to 256 characters. */
x86::reg32 frontend::eab::textMeasureAll(App* app, CPU& cpu, reg32 text)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4efe50, textMeasureUpTo>(app, cpu, text, 0x100u);
}

x86::reg32 frontend::textMeasureAll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4efef0", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 text = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::textMeasureAll(app, cpu, text); });
}

/* sub_4eff00: a bitmap freed: its surface if it was the screen's, its rows, its memory. */
x86::reg32 frontend::eab::windowBitmapFree(App* app, CPU& cpu, reg32 bitmap)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 surface = g.dword(bitmap + 0x34);
    if (surface != 0)
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, cpu, kDefaultDisplay, surface);
    const reg32 scanRow = g.dword(bitmap + 0x2c);
    if (scanRow != 0)
        fe::direct<0x509b10, watb::removeRow>(app, cpu, scanRow);
    const reg32 pixelRow = g.dword(bitmap + 0x30);
    if (pixelRow != 0)
        fe::direct<0x509b10, watb::removeRow>(app, cpu, pixelRow);
    g.setByte(bitmap + 0x1d, 0);
    g.setByte(bitmap + 0x1c, 0);
    g.setDword(bitmap, 0x57494e6b);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, bitmap);
}

x86::reg32 frontend::windowBitmapFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4eff00", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 bitmap = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::windowBitmapFree(app, cpu, bitmap); });
}

/* sub_4f00d0: the next key (sub_4cc970), its low word. */
x86::reg32 frontend::eab::keyNextLow16(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return fe::direct<0x4cc970, glue::nextKey>(app, cpu) & 0xffff;
}

x86::reg32 frontend::keyNextLow16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f00d0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::keyNextLow16(app, cpu); });
}

/* sub_4f00e0: the next key handed on to sub_50c584. */
x86::reg32 frontend::eab::keyNextDispatch(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return fe::direct<0x50c584, watb::dispatchKey>(app, cpu, fe::direct<0x4cc970, glue::nextKey>(app, cpu) & 0xffff);
}

x86::reg32 frontend::keyNextDispatch(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f00e0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::keyNextDispatch(app, cpu); });
}

/* sub_4f00f0: the flag at 0x5651d0 set from eax. */
x86::reg32 frontend::eab::keySetHookFlag(App* app, CPU& cpu, reg32 flag)
{
    Guest g(app, cpu);
    g.setDword(0x5651d0, flag);
    return flag;
}

x86::reg32 frontend::keySetHookFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f00f0", 0, 0, nullptr, 0, 0 };
    const reg32 flag = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::keySetHookFlag(app, cpu, flag); });
}

/* sub_4f01f0: the keys' table at 0x564818 cleared and the state marked open, with its
 * lock made the first time. */
x86::reg32 frontend::eab::inputStateOpen(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    if (g.dword(0x561290) == 0)
        g.setDword(0x561290, *fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    g.setDword(0x7f7488, 1);
    fe::direct<kFill, ea::clearBytes>(app, cpu, 0x564818u, 0x80u);
    return 0x564818 + 0x80;  // eax past what it cleared, as the fill leaves it
}

x86::reg32 frontend::inputStateOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f01f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::inputStateOpen(app, cpu); });
}

/* sub_4f0230: the state marked closed. */
x86::reg32 frontend::eab::inputStateClosedWith(App* app, CPU& cpu, reg32 eax)
{
    Guest g(app, cpu);
    g.setDword(0x7f7488, 0);
    return eax;
}

x86::reg32 frontend::eab::inputStateClose(App* app, CPU& cpu)
{
    return inputStateClosedWith(app, cpu, 0);
}

x86::reg32 frontend::inputStateClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f0230", 0, 0, nullptr, 0, kRestoreEdx };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::inputStateClosedWith(app, cpu, args.eax); });
}

/* sub_4f22e0: the file named at eax read into edx, its size kept at 0x5643f8; the sound
 * topped up before (sub_4ff7c0), the more so the bigger the file.  edx, or 0 if empty. */
x86::reg32 frontend::eab::fileLoadInto(App* app, CPU& cpu, reg32 name, reg32 into)
{
    constexpr reg32 kTopUp = 0x4ff7c0;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    const reg32 size = g.frame(), handle = g.frame() + 8;
    fe::pointerCall(g, kTopUp, args({ 0x190, handle, g.frame() + 4, size }));
    fe::direct<0x4e28d0, ea::openToRead>(app, cpu, name, handle, g.frame() + 4, size);  // as sub_4ff7c0 was handed them
    const reg32 bytes = g.dword(size);
    if (bytes == 0)
        return 0;
    // ebx as the first call left it, ecx the divisor (mov ecx, 0x1c2; idiv ecx).
    fe::pointerCall(g, kTopUp, args({ reg32(sreg32(bytes) / 0x1c2) + 0x190, reg32(sreg32(bytes) % 0x1c2), g.frame() + 4, 0x1c2 }));
    fe::direct<0x4e2de0, ea::readChecked>(app, cpu, g.dword(handle), into, g.dword(size));
    g.setDword(0x5643f8, g.dword(size));
    fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, g.dword(handle));
    return into;
}

x86::reg32 frontend::fileLoadInto(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f22e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 name = args.eax, into = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::fileLoadInto(app, cpu, name, into); });
}

/* sub_4f24e0: a shape's record of type 0x6f (its hot spot), past the record's head; 0 if
 * none.  A shape of a type with no pixels is an error. */
x86::reg32 frontend::eab::shapeFindHotspot(App* app, CPU& cpu, reg32 shape)
{
    constexpr reg32 kHotspotRecord = 0x6f;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    if (shape == 0)
        return 0;
    if (g.byte((g.dword(shape) & 0x7f) + kShapeBits) == 0)
    {
        errorAt(g, 0x54cad4, 0x54cb28, 0x69);
        error(g, 0x54cb38, { g.dword(shape) & 0xff });
    }
    reg32 record = shape;
    while (record != 0)
    {
        if ((g.dword(record) & 0xff) == kHotspotRecord)
            return record + 8;
        const reg32 next = reg32(sreg32(g.dword(record)) >> 8);
        record = next == 0 ? 0 : record + next;
    }
    return 0;
}

x86::reg32 frontend::shapeFindHotspot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f24e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 shape = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::shapeFindHotspot(app, cpu, shape); });
}

/* sub_4f2790: GetTickCount. */
x86::reg32 frontend::eab::lowTimerTicks(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    return windows<kGetTickCount, &host::ticks>(g, cpu);
}

x86::reg32 frontend::lowTimerTicks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2790", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lowTimerTicks(app, cpu); });
}

namespace
{
constexpr reg32 kLowTimerLock = 0x9f0c34, kLowTimerTable = 0x9f0c00, kLowTimerEvent = 0x5652ac;
}

/* sub_4f27a0: the low timer's thread: at each signal of its event, the functions of the
 * table run in turn, until its lock is gone. */
x86::reg32 frontend::eab::lowTimerThread(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    g.setDword(kLowTimerEvent, *fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu));
    if (g.dword(kLowTimerLock) != 0)
    {
        do
        {
            safepoint(app, cpu);
            fe::direct<0x4dfb70, ea::eventWaited>(app, cpu, g.dword(kLowTimerEvent));
            const reg32 lock = g.dword(kLowTimerLock);
            if (lock != 0)
            {
                windows<kEnterCriticalSection, &host::enterLock>(g, cpu, lock);
                if (g.dword(kLowTimerTable) != 0)
                {
                    reg32 slot = 0;
                    do
                    {
                        const reg32 at = slot;
                        slot += 4;
                        fe::pointerCall(g, g.dword(at + kLowTimerTable), args({ at }));
                    } while (slot < 0x20 && g.dword(slot + kLowTimerTable) != 0);
                }
                windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kLowTimerLock));
            }
        } while (g.dword(kLowTimerLock) != 0);
    }
    const reg32 result = directKept<0x4dfc00, ea::eventClosed, 1>(g.app(), g.cpu(), g.dword(kLowTimerEvent), 0);
    g.setDword(kLowTimerEvent, 0);
    return result;
}

x86::reg32 frontend::lowTimerThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f27a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lowTimerThread(app, cpu); });
}

/* sub_4f2890: the low timer started: its end at exit (0x4f2830), its lock, its thread. */
x86::reg32 frontend::eab::lowTimerStart(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    guest::direct<0x4f2a78, atexitJump>(app, cpu, 0x4f2830u);
    const reg32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, cpu);
    g.setDword(kLowTimerLock, lock);
    windows<kEnterCriticalSection, &host::enterLock>(g, cpu, lock);
    const reg32 made = *fe::direct<kCreateThread, ea::threadStarted>(app, cpu, 0x4f27a0u, 0u, 2u, 0x9f0c20u);
    if (made != 0)
    {
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kLowTimerLock));  // leaves eax as it is
        return made;
    }
    fe::direct<0x4eb360, ea::lockFreed>(app, cpu, g.dword(kLowTimerLock));
    g.setDword(kLowTimerLock, 0);
    errorAt(g, 0x54cbd8, 0x54cbe8, 0x4d);
    return errorLeft(g, 0x54cbf8);
}

x86::reg32 frontend::lowTimerStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2890", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lowTimerStart(app, cpu); });
}

/* sub_4f2930: the function eax added to the low timer's table unless it is there; the
 * timer started if it was not. */
x86::reg32 frontend::eab::lowTimerAdd(App* app, CPU& cpu, reg32 function)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    if (g.dword(kLowTimerLock) == 0)
        guest::direct<0x4f2890, lowTimerStart>(app, cpu);
    windows<kEnterCriticalSection, &host::enterLock>(g, cpu, g.dword(kLowTimerLock));
    sreg32 empty = -1;
    for (reg32 i = 0; i < 8; ++i)
    {
        const reg32 present = g.dword(kLowTimerTable + i * 4);
        if (present == 0 && empty < 0)
        {
            empty = sreg32(i);
            continue;
        }
        if (function == present)
        {
            return windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kLowTimerLock));
        }
    }
    if (empty != -1)
        g.setDword(kLowTimerTable + reg32(empty) * 4, function);
    return windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kLowTimerLock));
}

x86::reg32 frontend::lowTimerAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2930", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 function = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lowTimerAdd(app, cpu, function); });
}

/* sub_4f2a60: the low timer's event signalled, if it has one. */
x86::reg32 frontend::eab::lowTimerSignal(App* app, CPU& cpu, reg32 eax)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    const reg32 event = g.dword(kLowTimerEvent);
    return event != 0 ? *fe::direct<0x4dfb30, ea::eventSet>(app, cpu, event) : eax;
}

x86::reg32 frontend::lowTimerSignal(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2a60", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 eax = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lowTimerSignal(app, cpu, eax); });
}

/* sub_4f2a78: the runtime's atexit (a jmp). */
x86::reg32 frontend::eab::atexitJump(App* app, CPU& cpu, reg32 function)
{
    Guest g(app, cpu);
    return fe::direct<0x4f7c20, watcom::clib::atexit>(app, cpu, function);
}

x86::reg32 frontend::atexitJump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2a78", 0, 0, nullptr, 0, 0 };
    const reg32 function = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::atexitJump(app, cpu, function); });
}

/* sub_4f2a7d: exit(0).  What follows it is the jump table of sub_4f2ad0: it does not
 * come back. */
x86::reg32 frontend::eab::exitGameNow(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return fe::direct<0x4dfd04, watcom::clib::exit>(g.app(), g.cpu(), 0);
}

x86::reg32 frontend::exitGameNow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2a7d", 0, 0, nullptr, 0, 0 };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::exitGameNow(app, cpu); });
}

/* sub_4f2ad0: memcpy of ebx bytes from eax to edx, ecx of them by bytes to align it, a
 * Duff's device on 16 words by its jump table at 0x4f2a90, the last bytes one at a time.
 * The registers are left as the device leaves them: ecx carried the words of the slots
 * in kEcxSlots. */
x86::reg32 frontend::eab::memCopyAligned(App* app, CPU& cpu, reg32 from, reg32 to, reg32 size, reg32 lead)
{
    constexpr reg32 kEcxSlots = 0x7de;
    Guest g(app, cpu);
    if (lead != 0)
    {
        size -= lead;
        for (reg32 i = 0; i < lead; ++i)
            g.setByte(to++, g.byte(from++));
    }
    reg32 ecx = reg32(sreg32(size) >> 2);
    const reg32 entry = ecx & 0xf;
    reg32 blocks = reg32(sreg32(size) >> 6);
    const auto move = [&](reg32 slot) {
        const reg32 value = g.dword(from);
        from += 4;
        g.setDword(to, value);
        to += 4;
        if (((kEcxSlots >> slot) & 1) != 0)
            ecx = value;
    };
    if (entry != 0)
        for (reg32 slot = 16 - entry; slot < 16; ++slot)
            move(slot);
    while (--blocks != reg32(-1))
        for (reg32 slot = 0; slot < 16; ++slot)
            move(slot);
    const reg32 tail = size & 3;
    for (reg32 i = 0; i < tail; ++i)
    {
        const std::uint8_t value = g.byte(from + i);
        ecx = ((from + i) & 0xffffff00) | value;
        g.setByte(to + i, value);
    }
    return to;
}

x86::reg32 frontend::memCopyAligned(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2ad0", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 from = args.eax, to = args.edx, size = args.ebx, lead = args.ecx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memCopyAligned(app, cpu, from, to, size, lead); });
}

/* sub_4f2bf0: memmove of ebx bytes from eax to edx: from the end when they overlap or are
 * few, by sub_4f2ad0 else. */
x86::reg32 frontend::eab::memMoveLibrary(App* app, CPU& cpu, reg32 from, reg32 to, reg32 size)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    if (sreg32(size) >= 4 && (to <= from || to >= from + size))
        return guest::direct<0x4f2ad0, memCopyAligned>(app, cpu, from, to, size, (4 - (from & 3)) & 3);
    for (reg32 left = size; left-- != 0;)
        g.setByte(to + left, g.byte(from + left));
    return from;
}

x86::reg32 frontend::memMoveLibrary(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2bf0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const reg32 from = args.eax, to = args.edx, size = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveLibrary(app, cpu, from, to, size); });
}

/* sub_4f2c40 and its like: sub_4f2bf0 from edx to eax (these take the destination
 * first), ebx counting half bytes, bytes, words, three bytes or longs. */
x86::reg32 frontend::eab::memMoveHalfBytes(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, cpu, from, to, reg32(sreg32(count) >> 1));
}

x86::reg32 frontend::memMoveHalfBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2c40", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveHalfBytes(app, cpu, to, from, count); });
}

x86::reg32 frontend::eab::memMoveBytes(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, cpu, from, to, count);
}

x86::reg32 frontend::memMoveBytes(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2c50", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveBytes(app, cpu, to, from, count); });
}

x86::reg32 frontend::eab::memMoveWords(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, cpu, from, to, count * 2);
}

x86::reg32 frontend::memMoveWords(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2c60", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveWords(app, cpu, to, from, count); });
}

x86::reg32 frontend::eab::memMoveTriples(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, cpu, from, to, count * 3);
}

x86::reg32 frontend::memMoveTriples(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2c70", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const reg32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveTriples(app, cpu, to, from, count); });
}

x86::reg32 frontend::eab::memMoveLongs(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, cpu, from, to, count << 2);
}

x86::reg32 frontend::memMoveLongs(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2c90", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memMoveLongs(app, cpu, to, from, count); });
}

/* sub_4f2cb0: a shape moved (eax) to a place in memory (edx, ebx), the place's low twelve
 * bits of each kept in its fields at 0xc and 0xe. */
x86::reg32 frontend::eab::shapeMoveWithSpot(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 result = fe::direct<0x50b0b0, watb::moveShape>(app, cpu, shape, x, y, 0x5652b0u);
    g.setWord(shape + 0xc, g.word(shape + 0xc) & 0xf000);
    g.setDword(shape + 0xc, g.dword(shape + 0xc) | (x & 0xfff));
    // The code's ebx is stored by its low half alone: the word's high bits.
    const reg32 kept = g.word(shape + 0xe) & 0xf000;
    g.setWord(shape + 0xe, std::uint16_t(kept));
    g.setDword(shape + 0xc, g.dword(shape + 0xc) | ((y & 0xfff) << 16));
    return result;
}

x86::reg32 frontend::shapeMoveWithSpot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f2cb0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 shape = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::shapeMoveWithSpot(app, cpu, shape, x, y); });
}

namespace
{
constexpr reg32 kTcpLogLevel = 0x566de0;     // the transport's trace, from 4 on
constexpr reg32 kSockPort = 0x00, kSockThread = 0x04, kSockHandle = 0x18, kSockCallback = 0x3c, kSockArgument = 0x40;
constexpr reg32 kSockStop = 0x46, kSockRunning = 0x47;
}

/* sub_4f7450: a listening socket on the port eax: its record, its thread (sub_4f68c0)
 * waited for till it runs or gives up.  The record, or 0. */
x86::reg32 frontend::eab::tcpOpen(App* app, CPU& cpu, reg32 port, reg32 callback, reg32 argument)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x28);
    const reg32 address = g.frame(), length = g.frame() + 0x14, portSlot = g.frame() + 0x10;
    g.setDword(portSlot, port);
    errorAt(g, 0x54d2d0, 0x54d358, 0x31a);
    const reg32 record = *fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54d368u, 0x48u, g.dword(0x5643f4));
    fe::direct<kFill, ea::clearBytes>(g.app(), g.cpu(), record, 0x48);
    g.setDword(record + 0x1c, reg32(-1));
    const reg32 handle = fe::direct<0x4f6490, netb::tcpListenSocket>(app, cpu, g.dword(portSlot));
    g.setDword(record + kSockHandle, handle);
    g.setDword(record + kSockCallback, callback);
    g.setDword(record + kSockArgument, argument);
    const reg32 given = g.dword(portSlot);
    if (given != 0)
    {
        g.setDword(record + kSockPort, given);
    }
    else
    {
        g.setDword(length, 0x10);
        // No port asked for: the one the system gave.
        if (thunked<kGetSockName, &host::socketName>(g, cpu, g.dword(record + kSockHandle), address,
                                                                length) == 0)
            g.setDword(record + kSockPort, g.word(address + 2));
    }
    if (sreg32(g.dword(kTcpLogLevel)) >= 4)
        fe::direct<0x512180, netb::commEvent>(app, cpu, 0x54d370u, portSlot, record + kSockHandle, 4u, 1u, 0u);
    reg32 running = 0;
    const reg32 socket = g.dword(record + kSockHandle);
    if (socket != reg32(-1) && thunked<kListen, &host::socketListen>(g, cpu, socket, 5) == 0)
    {
        g.setDword(0x9f37f0, record);
        if (*fe::direct<kCreateThread, ea::threadStarted>(app, cpu, 0x4f68c0u, 0u, 2u, record + kSockThread) != 0)
        {
            running = 1;
            for (;;)
            {
                safepoint(app, cpu);
                const std::uint8_t state = g.byte(record + kSockRunning);
                if (state != 0 || state != g.byte(record + kSockStop))
                {
                    running = g.byte(record + kSockRunning);
                    break;
                }
                if (!*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u))
                    fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u);
                else
                    fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u);
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1u);
            }
        }
    }
    if (running != 0)
        return record;
    if (sreg32(g.dword(kTcpLogLevel)) >= 4)
        fe::direct<0x512180, netb::commEvent>(app, cpu, 0x54d380u, 0u, 0u, 0u, 1u, 0u);
    guest::direct<0x4f75d0, tcpClose>(app, cpu, record);
    return 0;
}

x86::reg32 frontend::tcpOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f7450", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 port = args.eax, callback = args.edx, argument = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpOpen(app, cpu, port, callback, argument); });
}

/* sub_4f75d0: a socket's record closed: its thread told and waited for, the socket, the
 * memory. */
x86::reg32 frontend::eab::tcpClose(App* app, CPU& cpu, reg32 record)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    if (sreg32(g.dword(kTcpLogLevel)) >= 4)
    {
        fe::direct<0x512180, netb::commEvent>(app, cpu, 0x54d390u, 0u, 0u, 0u, 0u, 0u);
    }
    if (g.byte(record + kSockRunning) != 0)
    {
        g.setByte(record + kSockStop, 1);
        do
        {
            safepoint(app, cpu);
            if (!*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u))
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u);  // the 0 the call before left in eax
            else
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u);
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1u);
        } while (g.byte(record + kSockRunning) != 0);
    }
    const reg32 socket = g.dword(record + kSockHandle);
    if (socket != reg32(-1))
        thunked<kCloseSocket, &host::socketClose>(g, cpu, socket);
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, record);
    return 1;
}

x86::reg32 frontend::tcpClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f75d0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 record = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpClose(app, cpu, record); });
}

/* sub_4f7660: the first field of a record. */
x86::reg32 frontend::eab::tcpFirstField(App* app, CPU& cpu, reg32 record)
{
    Guest g(app, cpu);
    return g.dword(record);
}

x86::reg32 frontend::tcpFirstField(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f7660", 0, 0, nullptr, 0, 0 };
    const reg32 record = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpFirstField(app, cpu, record); });
}

/* sub_4f7670: a name (edx) made an address (ecx) on the port ebx, or on the word at eax if
 * that is 0: a dotted number or a host's.  1, or 0 for no name, an unfinished one (the
 * dot at its end) or no such host. */
x86::reg32 frontend::eab::tcpResolve(App* app, CPU& cpu, reg32 portAt, reg32 name, reg32 port, reg32 address)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    if (name == 0)
        return 0;
    const reg32 length = g.length(name);
    if (length == 0 || g.byte(name + length - 1) == 0x2e)
        return 0;
    const reg32 number = thunked<kInetAddr, &host::addressParse>(g, cpu, name);
    fe::direct<kFill, ea::clearBytes>(g.app(), g.cpu(), address, 0x14);
    g.setWord(address, 2);
    reg32 good = 1;
    if (number != reg32(-1))
    {
        g.setDword(address + 4, number);
    }
    else
    {
        const reg32 host = thunked<kGetHostByName, &host::hostByName>(g, cpu, name);
        if (host == 0)
        {
            good = 0;
        }
        else
        {
            const reg32 size = reg32(sreg32(g.dword(host + 8)) >> 16);
            fe::direct<kMoveMemory, ea::moveMemory>(app, cpu, address + 4, g.dword(g.dword(host + 0xc)), size >= 4 ? 4 : size);
        }
    }
    if (good != 0)
    {
        std::uint16_t value = std::uint16_t(port);
        if (port == 0)
        {
            value = g.word(portAt);
        }
        const reg32 network = thunked<kHtons, &host::byteSwap16>(g, cpu, value);
        g.setWord(address + 2, std::uint16_t(network));
    }
    return good;
}

x86::reg32 frontend::tcpResolve(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f7670", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 portAt = args.eax, name = args.edx, port = args.ebx, address = args.ecx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpResolve(app, cpu, portAt, name, port, address); });
}

/* sub_4f7750: an address (edx) as text at ebx, its port through ecx if asked for. */
x86::reg32 frontend::eab::tcpAddressText(App* app, CPU& cpu, reg32 address, reg32 text, reg32 portOut)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 number = g.dword(address + 4);
    const reg32 dotted = thunked<kInetNtoa, &host::addressText>(g, cpu, number);
    g.copyString(text, dotted);
    if (portOut != 0)
    {
        g.setDword(portOut, thunked<kNtohs, &host::byteSwap16>(g, cpu, g.word(address + 2)) & 0xffff);
    }
    return 1;
}

x86::reg32 frontend::tcpAddressText(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f7750", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 address = args.edx, text = args.ebx, portOut = args.ecx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpAddressText(app, cpu, address, text, portOut); });
}

/* sub_4f77c0: whether the 16 bytes at edx and at ebx are the same. */
x86::reg32 frontend::eab::tcpSameAddress(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest g(app, cpu);
    for (reg32 i = 0; i < 0x10; ++i)
        if (g.byte(a + i) != g.byte(b + i))
            return 0;
    return 1;
}

x86::reg32 frontend::tcpSameAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f77c0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 a = args.edx, b = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpSameAddress(app, cpu, a, b); });
}

/* sub_4f77f0: a socket's own address (getsockname) into edx. */
x86::reg32 frontend::eab::tcpLocalName(App* app, CPU& cpu, reg32 record, reg32 out)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const reg32 length = g.frame();
    g.setDword(length, 0x14);
    const reg32 socket = g.dword(record + kSockHandle);
    return thunked<kGetSockName, &host::socketName>(g, cpu, socket, out, length) != reg32(-1) ? 1 : 0;
}

x86::reg32 frontend::tcpLocalName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f77f0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const reg32 record = args.eax, out = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::tcpLocalName(app, cpu, record, out); });
}

/* sub_4f9088: the end of sub_4f9070, called on its own: 0. */
x86::reg32 frontend::eab::lineNoFlag(App* app, CPU& cpu)
{
    return 0;
}

x86::reg32 frontend::lineNoFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f9088", 0, 0, nullptr, 0, 0 };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lineNoFlag(app, cpu); });
}

namespace
{
constexpr reg32 kLineText = 0x9f4dac, kLineLast = 0x56abb4, kLineFormat = 0x5347b0;
constexpr reg32 kLineDone = 0x9f4d98, kLineResult = 0x9f4d9c, kLineCall = 0x9f4da4, kLineHandle = 0x9f4da0;

// A line in the modem's log; the text kept as the last one.
reg32 lineLog(Guest& g, reg32 text)
{
    g.setDword(kLineLast, text);
    return *fe::direct<0x501050, ea::reportedAt>(g.app(), g.cpu(), kLineLog, 1u, text, guest::Words({}));
}

/* wsprintfA, through its slot (cdecl: the caller's words stay pushed): the port's host code. */
void lineFormat(Guest& g, const Arguments& arguments)
{
    // The port's wsprintfA formats nothing (it asserts), so the words need not be pushed.
    static_cast<void>(arguments);
    nfs3hp::host::formatString(g.app(), g.cpu());
}

reg32 lineFinish(Guest& g, reg32 result, reg32 eax)
{
    g.setDword(kLineResult, result);
    g.setDword(kLineDone, 1);
    return eax;  // what the caller held in eax: nothing here changes it
}
}

/* sub_4f9500: TAPI's callback (hDevice, message, instance, three parameters; stdcall): each
 * message told to the log; a call's state answered or dropped, its end the result the
 * connecting screen waits for at 0x9f4d9c, under 0x9f4d98. */
x86::reg32 frontend::eab::lineCallback(App* app, CPU& cpu, reg32 device, reg32 what, reg32 instance, reg32 p1, reg32 p2,
                                       reg32 p3)
{
    constexpr reg32 kReply = 12, kCallInfo = 8, kCallState = 2, kNewCall = 1;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x58);
    static_cast<void>(instance);  // the callback's instance word: not read
    const auto format = [&](reg32 text) { lineFormat(g, args({}, { kLineText, text, p1, p2, p3 })); };
    switch (what)
    {
    case kNewCall:
        format(0x54d998);
        return lineLog(g, kLineText);
    case kCallInfo:
        format(0x54d934);
        lineLog(g, kLineText);
        return fe::direct<0x530b4d, watc::lineGetLineDevStatus>(app, cpu, g.dword(kLineHandle), g.frame());
    case kReply:
    {
        format(0x54d968);
        const reg32 reported = lineLog(g, kLineText);
        return p2 != 0 ? lineFinish(g, 0xf, reported) : reported;
    }
    case kCallState:
    {
        format(0x54d9cc);
        const reg32 reported = lineLog(g, kLineText);
        switch (p1)
        {
        case 1:
        {
            const reg32 left = fe::direct<0x52f969, watc::lineDeallocateCall>(app, cpu, g.dword(kLineCall));
            g.setDword(kLineDone, p1);  // ebx, which the code compared with 1 to come here, and the call keeps
            return left;
        }
        case 2:
            fe::direct<0x52f535, watc::lineAnswer>(app, cpu, device, 0u, 0u);
            g.setDword(kLineCall, device);
            return device;
        case 0x40:
            return lineFinish(g, 0xc, reported);
        case 0x100:
        {
            const reg32 modem = fe::direct<0x4fa0f0, netb::tapiGetModemHandle>(app, cpu);
            lineFinish(g, 9, modem);
            return modem;
        }
        case 0x4000:
            return lineFinish(g, p2 == 8 ? 0xe : p2 == 0x20 ? 0xc : p2 == 0x40 ? 0xd : 0xb, reported);
        default:
            return reported;
        }
    }
    default:
        lineFormat(g, args({}, { kLineText, 0x54da00, what, p1, p2, p3 }));
        return lineLog(g, kLineText);
    }
}

x86::reg32 frontend::lineCallback(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f9500", 0x18, 0, nullptr, 0, kRestoreEbx | kRestoreEsi | kRestoreEdi };
    const reg32 device = stackWord(app, cpu, 0);
    const reg32 what = stackWord(app, cpu, 1);
    const reg32 instance = stackWord(app, cpu, 2);
    const reg32 p1 = stackWord(app, cpu, 3);
    const reg32 p2 = stackWord(app, cpu, 4);
    const reg32 p3 = stackWord(app, cpu, 5);
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lineCallback(app, cpu, device, what, instance, p1, p2, p3); });
}

/* sub_4f97c0: the modem shut: a second's wait, the call dropped, the line closed, the
 * reading thread or its handle gone, the buffers freed and TAPI told. */
x86::reg32 frontend::eab::lineShutdown(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0x3e8u);
    const reg32 call = g.dword(kLineCall);
    if (call != 0)
        fe::direct<0x52f969, watc::lineDeallocateCall>(app, cpu, call);
    const reg32 line = g.dword(kLineHandle);
    if (line != 0)
    {
        if (fe::direct<0x52f661, watc::lineClose>(app, cpu, line) != 0)
            lineLog(g, 0x54da3c);
    }
    const reg32 reader = g.dword(0x9f4d84);
    if (reader != 0 && g.dword(reader + 0x38) != reg32(-1))
    {
        const reg32 thread = g.dword(reader + 0x38);
        fe::direct<0x51b200, netc::serialClose>(app, cpu, thread);
        g.setDword(0x9f4d84, 0);
    }
    else
    {
        const reg32 handle = g.dword(0x9f4da8);
        if (handle != 0)
        {
            if (windows<0x534488, &host::closeHandle>(g, cpu, handle) == 0)
                lineLog(g, 0x54da50);
        }
    }
    const reg32 buffer = g.dword(0x9f4d88);
    if (buffer != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, buffer);
        g.setDword(0x9f4d88, 0);
    }
    if (g.dword(0x9f4d94) != 0)
    {
        for (sreg32 i = 0; i < sreg32(g.dword(0x9f4d80)); ++i)
        {
            const reg32 entry = g.dword(g.dword(0x9f4d94) + reg32(i) * 8 + 4);
            if (entry != 0)
                fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, entry);
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, g.dword(0x9f4d94));
        g.setDword(0x9f4d94, 0);
    }
    const reg32 result = fe::direct<0x5324d2, watc::lineShutdown>(app, cpu, g.dword(0x9f4d74));
    return result != 0 ? lineLog(g, 0x54da70) : result;
}

x86::reg32 frontend::lineShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4f97c0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::lineShutdown(app, cpu); });
}

namespace
{
constexpr reg32 kFileThreads = 0x9f4f60, kFileThreadIndex = 0x9f4f64, kFileRecords = 0x9f4f68, kFileLock = 0x9f4f6c;
constexpr reg32 kFileFree = 0x9f4f74, kFileHandles = 0x9f4f90, kFileStop = 0x5671f0;
constexpr reg32 kThreadBytes = 0x68;
constexpr reg32 kQueueFree = 0x50dde0;
constexpr reg32 kPulse = 0x4dfc50;
constexpr reg32 kFreePages = 0x501180, kCloseHandleSlot = 0x4e3fe0;
}

/* sub_4fbc30: a file opened (eax) as edx asks; a name with a '|' is a pack and a member,
 * and is looked for among the open packs, as is a name that did not open when ebx says.
 * The handle in the frame, or 0. */
x86::reg32 frontend::eab::asyncOpenFile(App* app, CPU& cpu, reg32 name, reg32 mode, reg32 search)
{
    constexpr reg32 kSearch = 0x104, kMember = 0x108, kResult = 0x10c, kHandle = 0x110;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x128);
    const reg32 pack = g.frame();
    reg32 bar = name;
    while (g.byte(bar) != '|' && g.byte(bar) != 0)
        ++bar;
    if (g.byte(bar) != '|')
        bar = 0;
    g.setDword(pack + kMember, name);
    g.setDword(pack + kSearch, pack);
    g.setDword(pack + kHandle, 0);
    g.setDword(pack + kResult, 0);
    if (bar == 0)
    {
        g.setDword(pack + kHandle, *fe::direct<0x4e3620, ea::fileHandleOpened>(app, cpu, name, mode, 0u, pack + kResult));
        g.setByte(pack, 0);
    }
    else
    {
        const reg32 length = bar - name;
        // By address: the ebx and edx it leaves are left at the end (edx read here).
        fe::direct<kStrncpy, watcom::clib::strncpy>(app, cpu, pack, name, length);
        g.setDword(pack + kMember, bar + 1);
        g.setByte(pack + length, 0);
    }
    if (search != 0 && g.dword(pack + kHandle) == 0)
        fe::direct<0x50e9b0, netb::qsafeiterate>(app, cpu, kFileHandles, 0x4fb790u, pack + kSearch);
    return g.dword(pack + kResult);
}

x86::reg32 frontend::asyncOpenFile(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4fbc30", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 name = args.eax, mode = args.edx, search = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::asyncOpenFile(app, cpu, name, mode, search); });
}

/* sub_4fbda0: a thread of the file requests: a request of the highest priority it may
 * take from its queue, served; put on the queue of what is done, its callback told. */
x86::reg32 frontend::eab::asyncFileThread(App* app, CPU& cpu)
{
    enum { kOpen, kNothing, kRead, kWrite, kSize, kSeek, kExists, kDelete, kNoError, kMap, kRemove };
    constexpr std::uint8_t kFinished = 1, kFailed = 0xfe;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x20);
    const reg32 sizeOut = g.frame();
    const reg32 thread = g.dword(kFileThreads) + g.dword(kFileThreadIndex) * kThreadBytes;
    const reg32 busyEvent = g.dword(thread + 0x5c);
    g.setDword(thread, 1);
    const reg32 firstPulse = *fe::direct<kPulse, ea::eventPulse>(app, cpu, busyEvent);
    if (g.dword(kFileStop) != 0)
    {
        g.setDword(thread, 0);
        return firstPulse;
    }
    const reg32 queue = thread + 0x1c, done = thread + 0x38;
    const auto failed = [&](reg32 request) { return (sreg32(g.dword(request + 0xc) << 0x1e) >> 0x1f) != 0; };
    const auto noteError = [&](reg32 request) { g.setDword(request + 0x14, directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.cpu())); };
    const auto serve = [&](reg32 request) {
        const reg32 operation = g.dword(request + 8);
        switch (operation)
        {
        case kOpen:
        {
            const reg32 flags = g.byte(request + 0x24);
            reg32 mode = 3, search = 0;
            if ((flags & 1) != 0)
            {
                search = 1;
                mode = 5;
            }
            else
            {
                if ((g.byte(request + 0x24) & 2) != 0)
                    mode = 0x13;
                if ((g.byte(request + 0x24) & 4) != 0)
                    mode |= 0x20;
            }
            if ((g.byte(request + 0x24) & 8) != 0)
                mode |= 0x800;
            if ((g.byte(request + 0x24) & 0x10) != 0)
                mode |= 0x100;
            if ((g.byte(request + 0x24) & 0x20) != 0)
                mode |= 0x200;
            const reg32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, cpu, g.dword(request + 0x2c), mode, search);
            g.setDword(request + 0x18, handle);
            if (handle != 0)
            {
                g.setByte(request + 0x10, kFinished);
            }
            else
            {
                const reg32 code = directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.cpu());
                g.setByte(request + 0x10, kFailed);
                g.setDword(request + 0x14, code);
            }
            break;
        }
        case kNothing:
            g.setByte(request + 0x10, kFinished);
            break;
        case kRead:
        case kWrite:
        {
            if (*fe::direct<0x4e3e60, ea::handleSought>(app, cpu, g.dword(request + 0x18), g.dword(request + 0x24)) != 0)
            {
                const reg32 moved = operation == kRead
                    ? *fe::direct<0x4e3ae0, ea::bytesRead>(app, cpu, g.dword(request + 0x18), g.dword(request + 0x2c),
                                                          g.dword(request + 0x28))
                    : *fe::direct<0x4e3cc0, ea::bytesWritten>(app, cpu, g.dword(request + 0x18), g.dword(request + 0x2c),
                                                             g.dword(request + 0x28));
                g.setByte(request + 0x10, kFinished);
                g.setDword(request + 0x28, moved);
            }
            const reg32 code = directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.cpu());
            g.setDword(request + 0x14, code);
            if (code != 0)
                g.setByte(request + 0x10, kFailed);
            break;
        }
        case kExists:
        {
            const reg32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, cpu, g.dword(request + 0x2c), 5u, 1u);
            g.setDword(request + 0x18, handle);
            if (handle != 0)
            {
                fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, handle);
                g.setDword(request + 0x24, 1);
            }
            else
            {
                g.setDword(request + 0x24, 0);
            }
            g.setByte(request + 0x10, kFinished);
            break;
        }
        case kSize:
        {
            const reg32 got = *fe::direct<0x4e3980, ea::handleInfoOf>(app, cpu, g.dword(request + 0x18), 0u, 0u, sizeOut, 0u);
            if (got != 0)
            {
                const reg32 size = g.dword(sizeOut);
                g.setByte(request + 0x10, kFinished);
                g.setDword(request + 0x24, size);
            }
            else
            {
                g.setByte(request + 0x10, kFailed);
                g.setDword(request + 0x24, 0);
                noteError(request);
            }
            break;
        }
        case kSeek:
        {
            if (fe::direct<0x4e4230, wina::fileSizeAndSeek>(app, cpu, g.dword(request + 0x18), g.dword(request + 0x28)) != 0)
            {
                g.setByte(request + 0x10, kFinished);
            }
            else
            {
                g.setByte(request + 0x10, kFailed);
                noteError(request);
            }
            break;
        }
        case kNoError:
            g.setDword(request + 0x14, 0);
            g.setByte(request + 0x10, kFinished);
            break;
        case kMap:
        {
            const reg32 record = g.dword(request + 0x2c);
            if (record == 0)
                break;
            const reg32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, cpu, record + 0x14, 0x805u, 1u);
            g.setDword(record + 0xc, handle);
            if (handle != 0)
            {
                g.setByte(request + 0x10, kFinished);
                g.setDword(record + 0x10, fe::direct<0x4e41f0, ea::handleMemoryOf>(app, cpu, g.dword(record + 0xc)));
            }
            else
            {
                g.setByte(request + 0x10, kFailed);
                noteError(request);
            }
            break;
        }
        case kRemove:
            fe::direct<0x50e150, ea::queueRemoved>(app, cpu, kFileHandles, g.dword(request + 0x2c), 0u);  // the answer is not read
            g.setByte(request + 0x10, kFinished);
            break;
        case kDelete:
            if (*fe::direct<0x4e3600, ea::fileRemoved>(app, cpu, g.dword(request + 0x2c)) != 0)
            {
                g.setByte(request + 0x10, kFinished);
            }
            else
            {
                g.setByte(request + 0x10, kFailed);
                noteError(request);
            }
            break;
        default:
        {
            g.setDword(0x552198, 0x1f7);
            const reg32 unknown = g.dword(g.dword(thread + 0x18) + 8);
            g.setDword(0x552190, 0x54dcac);
            g.setDword(0x552194, 0x54de64);
            error(g, 0x54de78, { unknown });
            break;
        }
        }
    };
    reg32 left = 0;  // eax as the last call leaves it, this one's result
    do
    {
        safepoint(app, cpu);
        const reg32 token = g.dword(thread) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, cpu, queue) : 0;
        const reg32 next = fe::direct<0x50df60, netb::qpop>(app, cpu, queue);
        reg32 request = next;
        g.setDword(thread + 0x18, next);
        if (next != 0 && sreg32(g.byte(next + 0x11)) > sreg32(g.dword(thread + 0x64)))
        {
            fe::direct<0x50e0a0, netb::qinsert>(app, cpu, queue, next);  // put back: in edx in the game's code
            g.setDword(thread + 0x18, 0);
            request = 0;
        }
        if (g.dword(thread) != 0)
            fe::direct<0x50ea40, menz::fileLockLeave>(app, cpu, queue, token);
        if (g.dword(thread + 0x18) == 0)
        {
            left = *fe::direct<0x4dfb70, ea::eventWaited>(app, cpu, g.dword(thread + 0x54));
            continue;
        }
        if (!failed(request))
            serve(request);
        const reg32 token2 = g.dword(thread) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, cpu, queue) : 0;
        fe::direct<0x50de50, netb::qpush>(app, cpu, done, request);
        const reg32 live = g.dword(thread);
        g.setDword(thread + 0x18, 0);
        if (live != 0)
            fe::direct<0x50ea40, menz::fileLockLeave>(app, cpu, queue, token2);
        if (g.dword(request + 0x20) != 0)
        {
            g.setByte(request + 0xc, g.byte(request + 0xc) | 8);
            const reg32 status = failed(request) ? reg32(-1) : reg32(sreg32(g.dword(request + 0xd)) >> 24);
            fe::pointerCall(g, g.dword(request + 0x20), args({ g.dword(request + 4), status, g.dword(request + 0x1c) }));
        }
        left = *fe::direct<kPulse, ea::eventPulse>(app, cpu, g.dword(thread + 0x5c));
    } while (g.dword(kFileStop) == 0);
    g.setDword(thread, 0);
    return left;
}

x86::reg32 frontend::asyncFileThread(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4fbda0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::asyncFileThread(app, cpu); });
}

/* sub_4fc170: the file requests ended, at exit: what is open closed, each thread woken
 * and waited for, its events and queues gone, the memory and the lock. */
x86::reg32 frontend::eab::asyncFilesShutdown(App* app, CPU& cpu, reg32 eax)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    if (g.dword(kFileThreads) == 0)
        return eax;
    for (;;)
    {
        const reg32 open = fe::direct<0x50df60, netb::qpop>(app, cpu, kFileHandles);
        if (open == 0)
            break;
        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, g.dword(open + 0xc));
        directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.cpu(), open);
    }
    g.setDword(kFileStop, 1);
    for (reg32 at = 0; at != 0xd00; at += kThreadBytes)
    {
        const reg32 thread = g.dword(kFileThreads) + at;
        if (g.dword(thread) == 0)
            continue;
        fe::direct<0x4dfb30, ea::eventSet>(app, cpu, g.dword(thread + 0x54));
        while (g.dword(thread) != 0)
        {
            safepoint(app, cpu);
            fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u);
        }
        directKept<0x4dfc00, ea::eventClosed, 1>(g.app(), g.cpu(), g.dword(thread + 0x54));
        fe::direct<0x4dfc80, ea::eventSetClosed>(app, cpu, g.dword(thread + 0x5c));
        directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.cpu(), thread + 0x38);
        directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.cpu(), thread + 0x1c);
    }
    directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.cpu(), g.dword(kFileRecords));
    directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.cpu(), g.dword(kFileThreads));
    directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.cpu(), kFileFree);
    directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.cpu(), kFileHandles);
    fe::direct<0x4eb360, ea::lockFreed>(app, cpu, g.dword(kFileLock));
    g.setDword(kFileThreads, 0);
    return 0;
}

x86::reg32 frontend::asyncFilesShutdown(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4fc170", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 eax = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::asyncFilesShutdown(app, cpu, eax); });
}

/* sub_4fc250: the file requests set up, once: eax handles (0: 0x40), edx records (0:
 * 0x40) on the free queue, room for the threads, the lock, the end at exit. */
x86::reg32 frontend::eab::asyncFilesInit(App* app, CPU& cpu, reg32 handles, reg32 records)
{
    constexpr reg32 kRecordBytes = 0x30;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const reg32 size = g.frame() + 4;
    if (g.dword(kFileThreads) != 0)
        return 1;
    if (records == 0)
        records = 0x40;
    fe::direct<0x4e3430, ea::handlesMade>(app, cpu, handles != 0 ? handles : 0x40);
    fe::direct<0x50dcd0, netb::qinit>(app, cpu, kFileFree, 0u, 0u);
    fe::direct<0x50dcd0, netb::qinit>(app, cpu, kFileHandles, 0u, 0u);
    g.setDword(size, 0xd00);
    g.setDword(kFileThreads, *fe::direct<0x501110, ea::allocatePages>(app, cpu, size));
    fe::direct<0x4e070c, ea::clearBytes>(app, cpu, g.dword(kFileThreads), g.dword(size));
    g.setDword(size, records * kRecordBytes);
    const reg32 memory = *fe::direct<0x501110, ea::allocatePages>(app, cpu, size);
    g.setDword(kFileRecords, memory);
    fe::direct<0x4e070c, ea::clearBytes>(app, cpu, memory, g.dword(size));
    const reg32 bytes = g.dword(size);
    reg32 record = g.dword(kFileRecords);
    for (reg32 left = bytes / kRecordBytes; left != 0; --left)
    {
        fe::direct<0x50de50, netb::qpush>(app, cpu, kFileFree, record);
        record += kRecordBytes;
    }
    g.setDword(kFileLock, *fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    g.setDword(0x9f4f70, reg32(-1));
    guest::direct<0x4f2a78, atexitJump>(app, cpu, 0x4fc170u);
    return 1;
}

x86::reg32 frontend::asyncFilesInit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4fc250", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const reg32 handles = args.eax, records = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::asyncFilesInit(app, cpu, handles, records); });
}

/* sub_514280: a game on a transport opened: eax the game's record, ecx the argument of
 * the transport's opening; the first one also starts the transport and its timer.  Its
 * address (edx), name and password (the stack's) kept.  0xfa, or 0. */
x86::reg32 frontend::eab::netOpen(App* app, CPU& cpu, reg32 net, reg32 address, reg32 opening, reg32 name,
                                  reg32 password)
{
    constexpr reg32 kTransportsOpen = 0x5689e4, kNetTimer = 0x514260;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const reg32 open = g.dword(kTransportsOpen) + 1;
    g.setDword(kTransportsOpen, open);
    // ecx and edx where the code has them: the table's indirect calls read them.
    x86::Args registers;
    registers.ecx = opening;
    registers.edx = open;
    if (open == 1)
    {
        const reg32 table = g.dword(net + 8);
        if (fe::pointerCall(g, g.dword(table + 0x14), args({ table }), registers) == 0)
            return 0;
        guest::direct<0x4f2930, lowTimerAdd>(app, cpu, kNetTimer);
    }
    registers.ecx = opening;
    registers.edx = open;
    const reg32 socket = fe::pointerCall(g, g.dword(g.dword(net + 8) + 0x1c), args({ opening, net, 0x4f53f0 }), registers);
    g.setDword(net + 0xc, socket);
    if (socket == 0)
        return 0;
    const reg32 table = g.dword(net + 8);
    registers.ecx = table;
    registers.edx = open;
    fe::pointerCall(g, g.dword(table + 0x44), args({ socket, net + 0x30, 0x10 }), registers);
    g.setDword(net + 0x9c, 1);
    fe::direct<kMoveMemory, ea::moveMemory>(app, cpu, net + 0x8c, address, 0x10u);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.cpu(), net + 0x44, name, 0x33);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.cpu(), net + 0x78, password, 0x13);
    return 0xfa;
}

x86::reg32 frontend::netOpen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_514280", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 net = args.eax, address = args.edx, opening = args.ecx;
    const reg32 name = stackWord(app, cpu, 0);
    const reg32 password = stackWord(app, cpu, 1);
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::netOpen(app, cpu, net, address, opening, name, password); });
}

/* sub_514320: a game's transport closed: the game ended, what it was waiting for done;
 * the last one stops the timer and the transport.  What the end said. */
x86::reg32 frontend::eab::netClose(App* app, CPU& cpu, reg32 net)
{
    constexpr reg32 kTransportsOpen = 0x5689e4, kNetTimer = 0x514260;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 socket = g.dword(net + 0xc);
    reg32 result = 1;
    if (socket == 0)
        return result;
    if (g.dword(net + 0x14) != 0 || g.dword(net + 0xa0) != 0)
        result = *fe::direct<0x4f4f90, ea::netGameEnded>(app, cpu, net);
    const reg32 waiting = g.dword(net + 0xa8);
    g.setDword(net + 0x9c, 0);
    if (waiting != 0)
    {
        do
        {
            safepoint(app, cpu);
            if (!*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u))
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0u);  // the 0 the call before left in eax
            }
            else
            {
                // Port: the other threads get to run, the transport's among them.
                app->unlockContext(cpu);
                win32::Thread::sleep(0);
                app->lockContext(cpu);
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u);
            }
            safepoint(app, cpu);
        } while (g.dword(net + 0xa8) != 0);
    }
    const reg32 table = g.dword(net + 8);
    g.setDword(net + 0xc, 0);
    fe::pointerCall(g, g.dword(table + 0x20), args({ socket }));  // ebx as the calls before leave it: the socket
    const reg32 open = g.dword(kTransportsOpen) - 1;
    g.setDword(kTransportsOpen, open);
    if (open != 0)
        return result;
    fe::direct<0x4f29b0, ea::removeTimer>(app, cpu, kNetTimer);
    const reg32 tableNow = g.dword(net + 8);
    return fe::pointerCall(g, g.dword(tableNow + 0x18), args({ tableNow }));
}

x86::reg32 frontend::netClose(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_514320", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 net = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::netClose(app, cpu, net); });
}

/* sub_5150f0: the machine's own address as text at eax: from the network's thunks (the
 * functions at 0x568a2c and 0x568a30, if it found them), else its host name's. */
x86::reg32 frontend::eab::netLocalAddress(App* app, CPU& cpu, reg32 out)
{
    constexpr reg32 kEntryBytes = 0x19c, kFile = 0x550540, kFunction = 0x550550, kBlockTag = 0x550568;
    constexpr reg32 kEnum = 0x568a2c, kQuery = 0x568a30;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x148);
    const reg32 host = g.frame();
    const reg32 valueSize = g.frame() + 0x100, value = g.frame() + 0x104;
    const reg32 bufferSize = g.frame() + 0x128, count = g.frame() + 0x12c;
    g.setByte(out, 0);
    if (fe::direct<0x515050, netc::loadRasApi>(app, cpu) != 0)
    {
        g.setDword(count, 0);
        g.setDword(bufferSize, kEntryBytes);
        errorAt(g, kFile, kFunction, 0x72);
        reg32 list = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kBlockTag, kEntryBytes, g.dword(0x5643f4));
        g.setDword(list, kEntryBytes);
        if (g.dword(kEnum) != 0 && fe::pointerCall(g, g.dword(kEnum), args({}, { list, bufferSize, count })) != 0)
        {
            // Not room enough: again, the size it asked for.
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, list);  // kFile in edx too, which it does not read
            g.setDword(0x552190, kFile);
            g.setDword(0x552198, 0x79);
            g.setDword(0x552194, kFunction);
            list = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kBlockTag, g.dword(bufferSize), g.dword(0x5643f4));
            g.setDword(list, kEntryBytes);
            if (fe::pointerCall(g, g.dword(kEnum), args({}, { list, bufferSize, count })) != 0)
                g.setDword(count, 0);
        }
        reg32 entry = list;
        for (reg32 i = 0; i < g.dword(count); ++i, entry += kEntryBytes)
        {
            g.setDword(valueSize, 0x28);
            g.setDword(bufferSize, 0x28);
            if (g.dword(kQuery) == 0)
                continue;
            if (fe::pointerCall(g, g.dword(kQuery), args({}, { g.dword(entry + 4), 0x8021, valueSize, bufferSize })) == 0
                && g.dword(value) == 0)
            {
                g.copyString(out, g.frame() + 0x108);
                return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, list);
            }
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, list);
    }
    reg32 result = thunked<kGetHostName, &host::hostName>(g, cpu, host, 0x100);
    if (result != 0)
        return result;
    result = thunked<kGetHostByName, &host::hostByName>(g, cpu, host);
    if (result == 0 || g.word(result + 8) != 2 || g.word(result + 0xa) != 4)
        return result;
    const reg32 text = thunked<kInetNtoa, &host::addressText>(g, cpu, g.dword(g.dword(g.dword(result + 0xc))));
    g.copyString(out, text);
    return text & 0xffffff00;
}

x86::reg32 frontend::netLocalAddress(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_5150f0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 out = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::netLocalAddress(app, cpu, out); });
}

/* sub_517bf0: a pool's blocks written out: name, size, room, owner, checksum, kind, and
 * the total of what is free. */
x86::reg32 frontend::eab::memPoolDump(App* app, CPU& cpu, reg32 which)
{
    constexpr reg32 kPools = 0x9ef5c0, kLine = 0x568fb4, kRow = 0x568ff7;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    const reg32 pool = which & 0xf;
    message(g, kLine);
    message(g, kLine);
    message(g, 0x550614);
    reg32 total = 0;
    reg32 block = g.dword(g.dword(pool * 4 + kPools) + 8);
    while (block != 0)
    {
        const reg32 data = block + 0x10;
        const reg32 named = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), data);
        const reg32 name = named != 0 ? named : 0x550658;
        const std::uint8_t flags = g.byte(block + 3);
        if ((flags & 0x40) != 0)
        {
            const reg32 size = g.dword(block + 4);
            total += size;
            message(g, kRow, { 0x55065c, data, size, g.dword(block + 8) - block, g.word(block + 2), 0, 0x550610 });
        }
        else
        {
            const reg32 size = g.dword(block + 4);
            reg32 checksum = 0;
            if (size != 0 && (flags & 0x80) == 0)
                checksum = fe::direct<0x4f4780, wata::blockChecksum>(app, cpu, data, size);
            const reg32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, cpu, block);
            const reg32 label = g.dword(kind * 4 + 0x569020);
            const reg32 sizeNow = g.dword(block + 4);
            message(g, kRow, { name, data, sizeNow, g.dword(block + 8) - block, g.word(block + 2), checksum, label });
        }
        const std::uint8_t flagsNow = g.byte(block + 3);
        if ((flagsNow & 4) != 0 && (flagsNow & 0x40) == 0)
        {
            const reg32 end = data + g.dword(block + 4);
            const reg32 first = g.dword(end + 4), second = g.dword(end + 8);
            if (first != 0)
                message(g, 0x550668, { first, second });
        }
        message(g, 0x550670);
        block = g.dword(block + 8);
    }
    message(g, 0x550674, { total, total });
    return message(g, kLine);
}

x86::reg32 frontend::memPoolDump(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_517bf0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 which = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memPoolDump(app, cpu, which); });
}

/* sub_517d60: the dump of the library's pool now. */
x86::reg32 frontend::eab::memPoolDumpDefault(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return guest::direct<0x517bf0, memPoolDump>(app, cpu, g.dword(0x5643f4));
}

x86::reg32 frontend::memPoolDumpDefault(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_517d60", 0, 0, nullptr, 0, 0 };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::memPoolDumpDefault(app, cpu); });
}

namespace
{
reg32 byteSwapped(reg32 value)
{
    return (value >> 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}
}

/* sub_517e50: whether a block ends with the "CRCF" tag of what was loaded with a
 * checksum. */
x86::reg32 frontend::eab::blockHasCrcTag(App* app, CPU& cpu, reg32 block)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const reg32 size = fe::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
    if (sreg32(size) < 0xc)
        return 0;
    return byteSwapped(g.dword(size + block - 0xc)) == byteSwapped(g.dword(0x5506b4)) ? 1 : 0;
}

x86::reg32 frontend::blockHasCrcTag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_517e50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const reg32 block = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::blockHasCrcTag(app, cpu, block); });
}

/* sub_517eb0: whether a block's checksum holds; one with no tag does. */
x86::reg32 frontend::eab::blockCrcGood(App* app, CPU& cpu, reg32 block)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 size = fe::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
    if (guest::direct<0x517e50, blockHasCrcTag>(app, cpu, block) == 0)
        return 1;
    const reg32 stored = g.dword(block + size - 4);
    return stored == fe::direct<0x4f4780, wata::blockChecksum>(app, cpu, block, size - 0xc) ? 1 : 0;
}

x86::reg32 frontend::blockCrcGood(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_517eb0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 block = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::blockCrcGood(app, cpu, block); });
}

/* sub_517f10: a block checked as edx names it: no tag or a bad sum an error if asked for
 * and 0; a good one has its tail cut off.  The block. */
x86::reg32 frontend::eab::blockCheckCrc(App* app, CPU& cpu, reg32 block, reg32 name)
{
    constexpr reg32 kFile = 0x5506bc, kFunction = 0x5506cc;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    if (block == 0)
        return 0;
    if (guest::direct<0x517e50, blockHasCrcTag>(app, cpu, block) == 0)
    {
        if (g.dword(0x569030) == 0)
            return block;
        const reg32 size = fe::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
        if (g.dword(0x56440c) != 0)
        {
            errorAt(g, kFile, kFunction, 0x57);
            error(g, 0x5506e0, { name, size });
        }
        return 0;
    }
    if (guest::direct<0x517eb0, blockCrcGood>(app, cpu, block) == 0)
    {
        if (g.dword(0x56440c) != 0)
        {
            const reg32 size = fe::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
            const reg32 stored = g.dword(size + block - 4);
            const reg32 sum = fe::direct<0x4f4780, wata::blockChecksum>(app, cpu, block, size - 0xc);
            errorAt(g, kFile, kFunction, 0x68);
            error(g, 0x550720, { name, size, stored, sum });
        }
        return 0;
    }
    const reg32 size = fe::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
    guest::direct<0x524df0, blockResize>(app, cpu, block, size - g.dword(0x56902c));
    return block;
}

x86::reg32 frontend::blockCheckCrc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_517f10", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 block = args.eax, name = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::blockCheckCrc(app, cpu, block, name); });
}

/* sub_524bb0: the question of leaving (Y/N) over the screen, the game left on a yes;
 * the screen let go first if it is held. */
x86::reg32 frontend::eab::exitConfirm(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const EndingFrame frame(g, cpu, 0xc);
    if (g.dword(0x564364) != 0)
    {
        if (!*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u))
        {
            windows<kEnterCriticalSection, &host::enterLock>(g, cpu, g.dword(kDisplayLock));
            windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
        }
        else
        {
            guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
        }
    }
    const reg32 y = half(g.dword(0x564388)) - 8;
    const reg32 x = half(g.dword(0x564384)) - 0x50;
    const reg32 answer = fe::direct<0x524970, menz::debugBoxAsk>(app, cpu, 0x551e74u, x, y, 0xa0u, 0x18u, 7u);
    return answer != 0 ? guest::direct<0x4f2a7d, exitGameNow>(app, cpu) : answer;
}

x86::reg32 frontend::exitConfirm(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_524bb0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::exitConfirm(app, cpu); });
}

/* sub_524df0: a block (eax) made the size edx in place, as far as there is room up to
 * the next: -1 as much as can be; what is past made a free block if it is worth it. */
x86::reg32 frontend::eab::blockResize(App* app, CPU& cpu, reg32 block, reg32 size)
{
    constexpr reg32 kPools = 0x9ef5c0;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x24);
    const reg32 header = block - 0x10;
    const reg32 owner = g.word(header + 2);
    const reg32 pool = g.dword((owner & 0xf) * 4 + kPools);
    const reg32 lock = g.dword(pool + 0x38);
    if (lock != 0)
        windows<kEnterCriticalSection, &host::enterLock>(g, cpu, lock);
    reg32 next = g.dword(header + 8);
    if ((g.byte(next + 3) & 0x40) != 0)
    {
        fe::direct<0x4e10e0, ea::freeUnlinked>(app, cpu, pool, next);
        next = g.dword(next + 8);
        g.setDword(header + 8, next);
    }
    reg32 asked = size;
    if (sreg32(size) < 8)
    {
        if (size == reg32(-1))
        {
            asked = 0x40000000;
        }
        else if (sreg32(size) >= 0)
        {
            asked = 8;
        }
        else if (g.dword(0x56440c) != 0)
        {
            g.setDword(0x552190, 0x551ea0);
            g.setDword(0x552194, 0x551eb0);
            g.setDword(0x552198, 0x63);
            error(g, 0x551ec0, { size });
        }
    }
    const reg32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), block, owner);
    const reg32 overhead = fe::direct<0x4e11f0, ea::blockOverhead>(app, cpu, blockName, owner);
    const reg32 granule = g.dword(pool + 0x28) - 1;
    reg32 room = ((asked + overhead + 0x10 + granule) & ~granule) - 0x10;
    const reg32 span = next - header - 0x10;
    if (sreg32(room) > sreg32(span))
    {
        size = span - overhead;
        room = span;
    }
    fe::direct<kMoveMemory, ea::moveMemory>(app, cpu, block + size, g.dword(header + 4) + block, overhead);
    g.setDword(header + 4, size);
    if (sreg32(span - room) > 0x40)
    {
        const reg32 rest = block + 0x10 + room;
        fe::direct<0x4e1120, ea::blockMarked>(app, cpu, rest, 0u, 0u, 0u, 0u, header, next);
        fe::direct<0x4e1080, ea::freeLinked>(app, cpu, pool, rest);
        g.setDword(next + 0xc, rest);
        g.setDword(header + 8, rest);
    }
    const reg32 lockNow = g.dword(pool + 0x38);
    if (lockNow != 0)
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, lockNow);
    return block;
}

x86::reg32 frontend::blockResize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_524df0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 block = args.eax, size = args.edx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::blockResize(app, cpu, block, size); });
}

/* sub_4ea440: how many files the BIG archive at eax holds, by its kind (sub_4e9f00): kind
 * 1 a big-endian word at +4, kind 2 a big-endian dword at +8; 0 for another. */
x86::reg32 frontend::eab::bigFileCount(App* app, CPU& cpu, reg32 archive)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    const reg32 kind = *fe::direct<0x4e9f00, load::bigFileKindOf>(app, cpu, archive);
    if (kind == 1)
        return __builtin_bswap32(g.dword(archive + 4)) >> 16;
    if (kind == 2)
        return __builtin_bswap32(g.dword(archive + 8));
    return 0;
}

x86::reg32 frontend::bigFileCount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ea440", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const reg32 archive = args.eax;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::bigFileCount(app, cpu, archive); });
}

/* sub_4ea4a0: the name of file edx of the BIG archive at eax copied to ebx (sub_4ea290 finds
 * it), or an empty name for none.  eax: the copy, or 0. */
x86::reg32 frontend::eab::bigFileName(App* app, CPU& cpu, reg32 archive, reg32 index, reg32 out)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    // By address: what it leaves in ebx and edx is what this function leaves there.
    reg32 from = *fe::direct<0x4ea290, load::locatedEntry>(app, cpu, archive, 0u, index, 0u, 0u);
    if (from == 0)
    {
        g.setByte(out, 0);
        return 0;
    }
    for (reg32 to = out;; ++from, ++to)
    {
        const std::uint8_t c = g.byte(from);
        g.setByte(to, c);
        if (c == 0)
            return out;
    }
}

x86::reg32 frontend::bigFileName(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site = { "sub_4ea4a0", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 archive = args.eax, index = args.edx, out = args.ebx;
    return runThunk(app, cpu, args, site, [&](Guest&) { return eab::bigFileName(app, cpu, archive, index, out); });
}

}
