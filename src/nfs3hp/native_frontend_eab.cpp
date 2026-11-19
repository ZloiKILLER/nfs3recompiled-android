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
#include "os_time.h"
#include "os_video.h"
#include "os_window.h"
// What lib/thread.h holds pointers to, forward-declared.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include "os_system.h"
#include "os_net.h"
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
nfs::u32 ddrawSetDisplayMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawDisposeDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawApplyPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawLockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawUnlockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapCreateDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawClearClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeFileFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyNibblesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyBytesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyWordsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyTriplesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyLongsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeDrawKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeDrawKeyedAtHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontUse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 paletteNearest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 colourPack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontSetColours(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGlyphSearch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGlyphFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontKerning(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textDrawAtPen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textDrawAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureUpTo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyNextLow16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyNextDispatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keySetHookFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 inputStateOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 inputStateClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileLoadInto(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeFindHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerSignal(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 atexitJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitGameNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memCopyAligned(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveHalfBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveLongs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeMoveWithSpot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpFirstField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpResolve(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpAddressText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSameAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpLocalName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineNoFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncOpenFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFileThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFilesShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFilesInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netLocalAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolDump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolDumpDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockHasCrcTag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockCrcGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockCheckCrc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitConfirm(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockResize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bigFileCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bigFileName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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
 * the typed cores (below); the window, the display and the network are the port's own
 * (os_window.h, os_video.h, os_net.h).  What each function leaves in eax and in the registers it does
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
nfs::u32 ddrawSetDisplayMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawDisposeDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawApplyPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawLockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawUnlockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapCreateDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawClearClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeFileFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyNibblesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyBytesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyWordsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyTriplesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyLongsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeDrawKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeDrawKeyedAtHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontUse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 paletteNearest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 colourPack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontSetColours(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGlyphSearch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGlyphFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontKerning(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textDrawAtPen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textDrawAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureUpTo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowBitmapFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyNextLow16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyNextDispatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keySetHookFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 inputStateOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 inputStateClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileLoadInto(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeFindHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lowTimerSignal(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 atexitJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitGameNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memCopyAligned(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveHalfBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memMoveLongs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeMoveWithSpot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpFirstField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpResolve(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpAddressText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpSameAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tcpLocalName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineNoFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lineShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncOpenFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFileThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFilesShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncFilesInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 netLocalAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolDump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolDumpDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockHasCrcTag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockCrcGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockCheckCrc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitConfirm(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockResize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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
using nfs::u32;
using nfs::s32;

constexpr u32 kDefaultDisplay = 0x9f38d0;
constexpr u32 kDisplayLock = 0x564480;       // its critical section, by value
constexpr u32 kError = 0x401010, kSprintf = 0x4df690, kMessage = 0x4df6c0, kReport = 0x501050;
constexpr u32 kFill = 0x4e070c, kReserve = 0x4e1620, kMoveMemory = 0x4ea4f0;
constexpr u32 kStrncpy = 0x4e0e30, kBlockName = 0x4e15e0;
constexpr u32 kCreateThread = 0x4df7a0;
constexpr u32 kLastError = 0x4dfcf0;
constexpr u32 kEaLog = 0x564ed4, kLineLog = 0x564454;
constexpr u32 kShapeBits = 0x566d54;         // the bits a pixel of each shape type has

// The fields of a display's record.
constexpr u32 kModeCount = 0x08, kModeNow = 0x0c, kModeTable = 0x10, kWidth = 0x14, kHeight = 0x18, kBits = 0x1c;
constexpr u32 kPalette = 0x24, kPaletteEntries = 0x30, kFirstChanged = 0x430, kLastChanged = 0x432;
constexpr u32 kLockedSurface = 0x43c, kLockedBits = 0x440, kPrimary = 0x444, kWindow = 0x454;
constexpr u32 kFullscreen = 0x45d, kOffsetX = 0x464, kOffsetY = 0x468;

// The font in use (sub_4ef520 fills it).
constexpr u32 kPenX = 0x5671f8, kPenY = 0x5671fc, kFontOriginX = 0x567200, kFontOriginY = 0x567204;
constexpr u32 kFontBits = 0x567208, kFontAscent = 0x567210, kFontDescent = 0x567214, kFontHeight = 0x567218;
constexpr u32 kFontColourA = 0x567220, kFontColourB = 0x567224, kFontRamp = 0x567228, kFontGlyphCount = 0x567268;
constexpr u32 kFontPitch = 0x56726c, kFontVersion = 0x567270, kFontGlyphs = 0x567278, kFontPixels = 0x56727c;
constexpr u32 kFontDraw = 0x567288, kFontBefore = 0x56728c, kFontAfter = 0x567290, kFontReader = 0x567294;
constexpr u32 kFontPerChar = 0x567298, kKerningCount = 0x56729c, kKerningTable = 0x5672a0, kFontRampGood = 0x5672a8;
constexpr u32 kPalette32 = 0xa0a814;         // 256 ARGB entries

// Where the error report's stamp goes before a call of sub_401010.
void errorAt(Guest& g, u32 file, u32 function, u32 line)
{
    g.setDword(0x552190, file);
    g.setDword(0x552194, function);
    g.setDword(0x552198, line);
}

void error(Guest& g, u32 text, std::initializer_list<u32> values = {})
{
    fe::direct<0x401010, ea::error>(g.app(), g.ts(), text, guest::Words(values));
}

/* The same by address where what it leaves in eax is handed back, which ea::error does not give. */
u32 errorLeft(Guest& g, u32 text, std::initializer_list<u32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = text;
    for (const u32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return fe::pointerCall(g, kError, arguments);
}

// sub_501050 on DirectDraw's log.
u32 report(Guest& g, u32 level, u32 text, std::initializer_list<u32> values = {})
{
    return *fe::direct<0x501050, ea::reportedAt>(g.app(), g.ts(), kEaLog, level, text, guest::Words(values));
}

/* The palette's colours at `address` in the game's memory, as the display takes them. */
const os::video::Colour* coloursAt(Guest& g, u32 address)
{
    return &g.app()->getMemory<const os::video::Colour>(address);
}


u32 message(Guest& g, u32 text, std::initializer_list<u32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = text;
    for (const u32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return fe::pointerCall(g, kMessage, arguments);
}

// What a loop waiting for another thread does on each pass.
void safepoint(nfs::App* app, nfs::ThreadState& ts)
{
    if (app->contextWanted())
        app->yieldContext(ts);
}

// x / 2 rounded towards zero, as Watcom writes it (the sign taken off, then sar 1).
u32 half(u32 value)
{
    return u32(s32(value) / 2);
}

// A signed byte at an address the original reads as `mov r, [a - 3]; sar r, 0x18`.
u32 signedByteAt(Guest& g, u32 address)
{
    return u32(s32(g.dword(address - 3)) >> 24);
}

// A word read as `mov r, [a - 2]; shr r, 0x10`.
u32 wordAt(Guest& g, u32 address)
{
    return g.dword(address - 2) >> 16;
}

/* A thunk's stack argument `index` (0 the first), above the return address. */
u32 stackWord(nfs::App* app, const nfs::ThreadState& ts, u32 index)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * index);
}

/* fe::Frame for a core whose way out is the game's end: esp left where the call that stopped the
 * game left it, as the game's code leaves it (sub_524bb0's exit), else back as fe::Frame puts it. */
class EndingFrame
{
public:
    EndingFrame(Guest& guest, nfs::ThreadState& ts, u32 bytes) : m_ts(ts), m_esp(ts.sp)
    {
        ts.sp -= bytes;
        guest.setFrame(ts.sp);
    }
    ~EndingFrame()
    {
        if (!m_ts.terminate)
            m_ts.sp = m_esp;
    }
    EndingFrame(const EndingFrame&) = delete;
    EndingFrame& operator=(const EndingFrame&) = delete;

private:
    nfs::ThreadState& m_ts;
    const u32 m_esp;
};

}  // namespace

/* sub_4ec870: a display (eax, 0 the library's own) put in the mode edx of its table, a
 * negative one 640x480x16; a window's client area made that size instead.  1 when the
 * mode came, 0 when none is known (and -1 on DirectDraw's "not exclusive", which the port's display
 * never says). */
nfs::u32 frontend::eab::ddrawSetDisplayMode(App* app, ThreadState& ts, u32 given, u32 mode)
{
    Guest g(app, ts);
    const fe::Frame ownFrame(g, ts, 0x4a0);
    const u32 frame = g.frame();
    report(g, 1, 0x54be18, { mode });
    const u32 display = given != 0 ? given : kDefaultDisplay;
    const os::video::Format now = os::video::mode();
    report(g, 0xe, 0x54be38, { 0, now.width, now.height, now.bits });
    if (g.byte(display + kFullscreen) == 0)
    {
        /* In a window: the client area, which the library centres on the desktop and sizes to the
         * display -- the port's window is the screen, neither moved nor sized, so its client area
         * is what comes. */
        const os::window::Area client = os::window::clientArea(g.dword(display + kWindow));
        g.setDword(display + kWidth, u32(client.right - client.left));
        g.setDword(display + kHeight, u32(client.bottom - client.top));
        g.setDword(display + kOffsetX, u32(client.left));
        g.setDword(display + kOffsetY, u32(client.top));
        g.setDword(display + kBits, now.bits);
        g.setDword(display + kModeNow, mode);
        return 1;
    }
    if (g.dword(display + kModeTable) == 0)
    {
        report(g, 1, 0x54be70);
        return 0;
    }
    u32 wide, tall, bits, refresh;
    if (s32(mode) < 0)
    {
        report(g, 0xe, 0x54bec0);
        wide = 0x280;
        tall = 0x1e0;
        bits = 0x10;
        refresh = 0;
    }
    else
    {
        if (s32(mode) >= s32(g.dword(display + kModeCount)))
            mode = 0;
        const u32 entry = g.dword(display + kModeTable) + mode * 20;
        bits = g.byte(entry + 0x12);
        tall = g.dword(entry + 4);
        wide = g.dword(entry);
        refresh = g.dword(entry + 8);
        report(g, 0xe, 0x54be98, { wide, tall, bits });
    }
    if (g.dword(display + kPalette) != 0)
    {
        fe::direct<kFill, ea::clearBytes>(g.app(), g.ts(), frame, 0x400);
        os::video::setPalette(app, g.dword(display + kPalette), 0, 0x100, coloursAt(g, frame));
        g.setWord(display + kFirstChanged, 0);
        g.setWord(display + kLastChanged, 0xff);
    }
    report(g, 0xf, 0x54beec);
    // The mode is always had (0), whatever its refresh: the port's display is never another's to keep.
    NFS2_USE(refresh);
    os::video::setMode(app, g.dword(display), wide, tall, bits);
    report(g, 0xc, 0x54bf1c, { 0 });
    const os::video::Format got = os::video::mode();
    // What the mode gives past what was asked for is a border round the picture.
    const u32 gotWide = got.width;
    if (wide < gotWide)
    {
        g.setDword(display + kWidth, wide);
        g.setDword(display + kOffsetX, (gotWide - wide) >> 1);
    }
    else
    {
        g.setDword(display + kWidth, gotWide);
    }
    const u32 gotTall = got.height;
    if (tall < gotTall)
    {
        g.setDword(display + kHeight, tall);
        g.setDword(display + kOffsetY, (gotTall - tall) >> 1);
    }
    else
    {
        g.setDword(display + kHeight, gotTall);
    }
    // 1 when the mode has the bits asked for; else it is marked in the table as one that does not work.
    u32 result = 0;
    if (bits == got.bits)
    {
        result = 1;
    }
    else
    {
        const u32 flags = g.dword(display + kModeTable) + mode * 20 + 0xf;
        g.setByte(flags, g.byte(flags) | 0x80);
    }
    g.setDword(display + kBits, got.bits);
    g.setDword(display + kModeNow, mode);
    // The code flushes the registry's classes here: the port's settings are in memory, nothing to flush.
    const u32 bitsGot = g.dword(display + kBits);
    report(g, 2, 0x54bf4c, { wide, tall, bits, g.dword(display + kWidth), g.dword(display + kHeight), bitsGot });
    return result;
}

nfs::u32 frontend::ddrawSetDisplayMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ec870", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 given = args.eax, mode = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::ddrawSetDisplayMode(app, ts, given, mode); });
}

/* sub_4ecd50: a display's surfaces, its DirectDraw object and its table of modes gone. */
nfs::u32 frontend::eab::ddrawDisposeDisplay(App* app, ThreadState& ts, u32 given)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 display = given != 0 ? given : kDefaultDisplay;
    if (g.dword(display) == 0)
        return given;
    report(g, 1, 0x54bf8c);
    fe::enterLock(g, ts, g.dword(kDisplayLock));
    guest::direct<0x4ec0d0, eaa::ddrawRemoveSurfaces>(app, ts, display);
    report(g, 0xa, 0x54bfa8);
    os::video::release(app, g.dword(display));
    report(g, 0xa, 0x54bfc8);
    const u32 modes = g.dword(display + kModeTable);
    if (modes != 0)
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, modes);
    g.setDword(display + kModeTable, 0);
    g.setDword(display + 4, 0);
    g.setDword(display + kPalette, 0);
    g.setDword(display, 0);
    return fe::leaveLock(g, ts, g.dword(kDisplayLock));
}

nfs::u32 frontend::ddrawDisposeDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ecd50", 0, 0, nullptr, 0, kRestoreAll };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::ddrawDisposeDisplay(app, ts, given); });
}

/* sub_4ecf60: the colours changed since the last time told to the display's palette,
 * unless a surface is locked (an error) or the display has no palette. */
nfs::u32 frontend::eab::ddrawApplyPalette(App* app, ThreadState& ts, u32 given)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 display = given != 0 ? given : kDefaultDisplay;
    u32 eax = given;
    if (g.dword(display + kLockedSurface) != 0)
    {
        errorAt(g, 0x54bb1c, 0x54bfd4, 0x37e);
        eax = errorLeft(g, 0x54bfe4);
    }
    if (g.dword(display + kPalette) == 0 || s32(g.dword(display + kBits)) > 8)
        return eax;
    report(g, 1, 0x54c024);
    const u32 first = u32(s32(g.dword(display + kFirstChanged - 2)) >> 16);
    const u32 span = u32(s32(g.dword(display + kLastChanged - 2)) >> 16) - first;
    const u32 count = span + 1;
    u32 result = span;
    if (s32(first) < 0x100 && s32(count) > 0)
    {
        const u32 entries = display + kPaletteEntries + first * 4;
        const u32 palette = g.dword(display + kPalette);
        os::video::setPalette(app, palette, first, count, coloursAt(g, entries));
        const u32 reported = report(g, 1, 0x54c03c, { first, count, 0 });
        const u32 fullscreen = g.byte(display + kFullscreen);
        result = (reported & 0xffff00ff) | (fullscreen << 8);
        // In a window the colours are read back from the palette: the port's gives nothing back (0).
        if (fullscreen == 0)
            result = 0;
    }
    g.setWord(display + kLastChanged, 0xffff);
    g.setWord(display + kFirstChanged, 0x100);
    return result;
}

nfs::u32 frontend::ddrawApplyPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ecf60", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::ddrawApplyPalette(app, ts, given); });
}

/* sub_4ed0f0: the surface edx of a display locked, its bits named in a bitmap (ebx, 0 for
 * none); what was locked before unlocked first.  1, or 0 when it could not be had: a
 * full-screen game out of focus or minimised, or Lock's error. */
nfs::u32 frontend::eab::ddrawLockBitmap(App* app, ThreadState& ts, u32 given, u32 surface, u32 bitmap)
{
    constexpr u32 kLockedNow = 0x5643e8, kLockFailed = 0x564ee8, kLockTime = 0x564eec, kLockHeld = 0x564ee0;
    constexpr u32 kNotNow = 0xeacbad;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x7c);
    os::video::Locked locked{};
    const u32 display = given != 0 ? given : kDefaultDisplay;
    if (surface != 0 && surface == g.dword(display + kLockedSurface))
        return 1;
    if (g.dword(display + kLockedSurface) != 0)
        guest::direct<0x4ed2a0, ddrawUnlockBitmap>(app, ts, display);
    if (surface == 0)
    {
        g.setDword(kLockedNow, 0);
        return 1;
    }
    const u32 lock = g.dword(kDisplayLock);
    fe::enterLock(g, ts, lock);
    // In fullscreen, not while the game has not the focus; the port's window is never minimised.
    bool mayLock = true;
    if (g.byte(display + kFullscreen) != 0 && fe::direct<0x4f9120, wina::hasFocus>(app, ts) == 0)
        mayLock = false;
    const u32 status =
        mayLock ? guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, ts, display, surface, &locked) : kNotNow;
    g.setDword(kLockTime, os::time::ticks(g.app(), ts));
    if (status != 0)
    {
        const u32 failedBefore = g.dword(kLockFailed);
        g.setDword(kLockHeld, u32(-1));
        if (failedBefore != 0)
        {
            report(g, 4, 0x54c064, { status });
            g.setDword(kLockFailed, 0);
        }
        const u32 held = g.dword(kDisplayLock);
        g.setDword(display + kLockedBits, 0);
        g.setDword(display + kLockedSurface, 0);
        fe::leaveLock(g, ts, held);
        g.setDword(kLockedNow, 1);
        return 0;
    }
    if (g.dword(kLockFailed) == 0)
    {
        report(g, 4, 0x54c09c);
        g.setDword(kLockFailed, 1);
    }
    const u32 bits = locked.pixels;
    g.setDword(display + kLockedSurface, surface);
    g.setDword(display + kLockedBits, bits);
    if (bitmap != 0)
    {
        const u32 pitch = u32(locked.format.pitch);
        u32 first = locked.pixels;
        // A window's picture starts where the window is on the primary surface.
        if (g.byte(display + kFullscreen) == 0 && surface == g.dword(display + kPrimary))
        {
            first = g.dword(display + kOffsetY) * pitch + (locked.format.bits >> 3) * g.dword(display + kOffsetX)
                + locked.pixels;
        }
        fe::direct<0x508da0, menz::videoWindowUpdate>(app, ts, bitmap, pitch, first);
    }
    g.setDword(kLockedNow, 0);
    return 1;
}

nfs::u32 frontend::ddrawLockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ed0f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 given = args.eax, surface = args.edx, bitmap = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::ddrawLockBitmap(app, ts, given, surface, bitmap); });
}

/* sub_4ed2a0: the surface locked unlocked, how long it was held noted. */
nfs::u32 frontend::eab::ddrawUnlockBitmap(App* app, ThreadState& ts, u32 given)
{
    constexpr u32 kLockedNow = 0x5643e8;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 display = given != 0 ? given : kDefaultDisplay;
    const u32 surface = g.dword(display + kLockedSurface);
    if (surface == 0)
    {
        g.setDword(kLockedNow, 0);
        return given;
    }
    os::video::unlock(app, ts, surface);
    g.setDword(display + kLockedSurface, 0);
    g.setDword(display + kLockedBits, 0);
    const u32 held = os::time::ticks(g.app(), ts) - g.dword(0x564eec);
    g.setDword(0x564ee0, held);
    if (s32(held) > s32(g.dword(0x564ee4)))
        g.setDword(0x564ee4, held);
    fe::leaveLock(g, ts, g.dword(kDisplayLock));  // leaves eax as it is: held
    g.setDword(kLockedNow, 0);
    return held;
}

nfs::u32 frontend::ddrawUnlockBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ed2a0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::ddrawUnlockBitmap(app, ts, given); });
}

/* sub_4eee30: a bitmap of eax by edx pixels of ebx bits (0 the screen's) in the memory
 * flags of ecx: its header, a shape header at 0x38 and its pixels below; made with the
 * rows when the library can draw.  The stack's argument says to complain on failure; 0
 * on failure. */
nfs::u32 frontend::eab::windowBitmapCreate(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 flags,
                                             u32 complain)
{
    constexpr u32 kFile = 0x54c2b4, kFunction = 0x54c2c4, kBitmapMagic = 0x57494e44;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x34);
    const u32 name = g.frame();
    u32 depth = bits != 0 ? bits : g.dword(0x56438c);
    u32 cellBits = depth == 0xf ? 0x10 : depth;
    if (depth == 0)
    {
        depth = 8;
        cellBits = 8;
    }
    if (s32(width) < 0 || s32(width) > 0x4000 || s32(height) < 0 || s32(height) > 0x2000)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x54);
            error(g, 0x54c2d8, { width, height, depth, 0x4000, 0x2000 });
        }
        return 0;
    }
    if (fe::direct<0x5098d0, watb::shapeTypeForDepth>(app, ts, depth) == 0)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x5f);
            error(g, 0x54c328, { width, height, depth });
        }
        return 0;
    }
    const u32 roundedBits = (width * cellBits + 7) & ~u32(7);
    const u32 wholeBits = u32((s32(roundedBits) / s32(cellBits)) * s32(cellBits));
    const u32 rowBytes = u32(s32(wholeBits) >> 3);
    const u32 size = rowBytes * height + 0x48;
    fe::direct<0x4df690, watcom::clib::sprintf>(app, ts, name, 0x54c37cu, guest::Words({ width, height, depth }));
    errorAt(g, kFile, kFunction, 0x6a);
    const u32 block = *fe::direct<kReserve, ea::reserved>(app, ts, name, size, flags);
    if (block == 0)
    {
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x70);
            g.setDword(0x564400, g.dword(0x564400) + 1);
            const u32 largest = fe::direct<0x4e1580, ea::defaultLargest>(app, ts);
            error(g, 0x54c388, { width, height, depth, size, largest });
        }
        return 0;
    }
    fe::direct<kFill, ea::clearBytes>(g.app(), g.ts(), block, 0x48);
    const u32 header = block + 0x38;
    const u32 type = fe::direct<0x5098d0, watb::shapeTypeForDepth>(app, ts, depth);  // header in edx too, not read
    g.setByte(header, 0);
    const u32 headerWord = g.dword(header) | (type & 0xff);
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
    const u32 canDraw = g.dword(0x568380);
    if (canDraw == 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
        if (complain != 0)
        {
            errorAt(g, kFile, kFunction, 0x8b);
            error(g, 0x54c3d8, { width, height, depth });
        }
        return 0;
    }
    g.setDword(block + 0x28, rowBytes);
    const u32 scanRow = fe::direct<0x509a30, watb::makeRow>(app, ts, 0x10u, rowBytes, 0u, height);
    g.setDword(block + 0x2c, scanRow);
    const u32 pixelRow = fe::direct<0x509a30, watb::makeRow>(app, ts, 0u, cellBits, 3u, width);
    const u32 scanRowKept = g.dword(block + 0x2c);
    g.setDword(block + 0x30, pixelRow);
    if (scanRowKept != 0 && pixelRow != 0)
        return block;
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
    if (complain != 0)
    {
        errorAt(g, kFile, kFunction, 0x99);
        error(g, 0x54c444, { width, height });
    }
    return 0;
}

nfs::u32 frontend::windowBitmapCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eee30", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 width = args.eax, height = args.edx, bits = args.ebx, flags = args.ecx;
    const u32 complain = stackWord(app, ts, 0);
    return runThunk(app, ts, args, site, [&](Guest&) {
        return eab::windowBitmapCreate(app, ts, width, height, bits, flags, complain);
    });
}

/* sub_4ef190: sub_4eee30, complaining as the library's flag at 0x56440c says. */
nfs::u32 frontend::eab::windowBitmapCreateDefault(App* app, ThreadState& ts, u32 width, u32 height, u32 bits,
                                                    u32 flags)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    const u32 complain = g.dword(0x56440c);
    return guest::direct<0x4eee30, windowBitmapCreate>(app, ts, width, height, bits, flags, complain);
}

nfs::u32 frontend::windowBitmapCreateDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef190", 0, 0, nullptr, 0, kRestoreEsi };
    const u32 width = args.eax, height = args.edx, bits = args.ebx, flags = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) {
        return eab::windowBitmapCreateDefault(app, ts, width, height, bits, flags);
    });
}

/* sub_4ef1b0: the clip rectangle filled with the colour in eax. */
nfs::u32 frontend::eab::drawClearClip(App* app, ThreadState& ts, u32 colour)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 left = g.dword(0x565000), top = g.dword(0x565004);
    const u32 wide = g.dword(0x565008) - left, tall = g.dword(0x56500c) - top;
    return fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, left, top, wide, tall, colour);
}

nfs::u32 frontend::drawClearClip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef1b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 colour = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::drawClearClip(app, ts, colour); });
}

/* sub_4ef1e0: the shape named by the four bytes at edx in the shape file eax: last to
 * first, the first entry the fallback.  None: 0, and an error if the flag at 0x56440c
 * says to. */
nfs::u32 frontend::eab::shapeFileFind(App* app, ThreadState& ts, u32 file, u32 nameAt)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 wanted = g.dword(nameAt);
    u32 edxNow = nameAt;
    u32 index = g.dword(file + 8);
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
    const u32 found = fe::direct<0x509c00, watb::findBlockHeader>(app, ts, file, edxNow);
    const u32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), found);
    const u32 shown = blockName != 0 ? blockName : 0x54c47c;
    errorAt(g, 0x54c480, 0x54c490, 0x4f);
    error(g, 0x54c49c, { nameAt, shown });
    return 0;
}

nfs::u32 frontend::shapeFileFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef1e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 file = args.eax, nameAt = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::shapeFileFind(app, ts, file, nameAt); });
}

/* sub_4ef2b0: ebx 4-bit pixels from eax to edx, a zero one not copied. */
nfs::u32 frontend::eab::copyNibblesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 pixels)
{
    Guest g(app, ts);
    const s32 count = s32(pixels);
    s32 i = 0;
    while (i < count)
    {
        const u32 at = u32(i >> 1);
        const std::uint8_t source = g.byte(from + at);
        const u32 pixel = (i & 1) != 0 ? u32(source) >> 4 : u32(source & 0xf);
        if (pixel == 0)
        {
        }
        else
        {
            const std::uint8_t kept = g.byte(to + at);
            const u32 value = (i & 1) != 0 ? (pixel << 4) | (kept & 0xf) : pixel | (kept & 0xf0);
            g.setByte(to + at, std::uint8_t(value));
        }
        ++i;
    }
    return u32(i);
}

nfs::u32 frontend::copyNibblesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef2b0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 from = args.eax, to = args.edx, pixels = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::copyNibblesKeyed(app, ts, from, to, pixels); });
}

/* sub_4ef320: ebx bytes from eax to edx, a 0xff not copied. */
nfs::u32 frontend::eab::copyBytesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    Guest g(app, ts);
    do
    {
        const std::uint8_t pixel = g.byte(from++);
        if (pixel != 0xff)
            g.setByte(to, pixel);
        ++to;
    } while (--count != 0);
    return from;
}

nfs::u32 frontend::copyBytesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef320", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::copyBytesKeyed(app, ts, from, to, count); });
}

/* sub_4ef340: ebx words from eax to edx, a zero one not copied. */
nfs::u32 frontend::eab::copyWordsKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    Guest g(app, ts);
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

nfs::u32 frontend::copyWordsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef340", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::copyWordsKeyed(app, ts, from, to, count); });
}

/* sub_4ef370: ebx 3-byte pixels from eax to edx, an all-zero one not copied. */
nfs::u32 frontend::eab::copyTriplesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    Guest g(app, ts);
    u32 result;
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

nfs::u32 frontend::copyTriplesKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef370", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::copyTriplesKeyed(app, ts, from, to, count); });
}

/* sub_4ef3c0: ebx ARGB pixels from eax to edx, one with no alpha not copied. */
nfs::u32 frontend::eab::copyLongsKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    Guest g(app, ts);
    do
    {
        const u32 pixel = g.dword(from);
        if ((pixel >> 24) != 0)
            g.setDword(to, pixel);
        to += 4;
        from += 4;
    } while (--count != 0);
    return from;
}

nfs::u32 frontend::copyLongsKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef3c0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::copyLongsKeyed(app, ts, from, to, count); });
}

/* sub_4ef3f0: a shape (eax) drawn at edx, ebx with its key colour clear: the palette's
 * entry 0 for 4 bits, 0xff else, emptied for the time; an 8-bit shape on a 16-bit screen
 * through a palette of its own, made in the frame. */
nfs::u32 frontend::eab::shapeDrawKeyed(App* app, ThreadState& ts, u32 shape, u32 x, u32 y)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x414);
    const u32 bits = g.byte((g.dword(shape) & 0x7f) + kShapeBits);
    const u32 key = kPalette32 + (bits == 4 ? 0 : 0xff) * 4;
    const u32 keptColour = g.dword(key);
    g.setDword(0x5650b4, 0);
    g.setDword(key, 0);
    if (bits == 8 && g.byte(0x565010) == 0x10)
    {
        // The game's code hands it y and 0x7b in ebx and ecx too, which it does not read.
        const u32 pointer = fe::direct<0x50a830, watb::clut16aPointer>(app, ts, shape, g.frame());
        g.setDword(0x9f09b0, pointer);
        g.setDword(0x5650bc, 0x7b);
        g.setDword(0x5650b4, 0x50afc0);
    }
    fe::direct<0x50b0b0, watb::moveShape>(app, ts, shape, x, y, 0x56509cu);
    g.setDword(0x5650bc, 0);
    g.setDword(key, keptColour);
    return keptColour;
}

nfs::u32 frontend::shapeDrawKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef3f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 shape = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::shapeDrawKeyed(app, ts, shape, x, y); });
}

/* sub_4ef4c0: sub_4ef3f0 with the shape's own offset taken off the position. */
nfs::u32 frontend::eab::shapeDrawKeyedAtHotspot(App* app, ThreadState& ts, u32 shape, u32 x, u32 top)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    const u32 y = top - u32(s32(g.dword(shape + 8)) >> 16);
    const u32 offsetX = u32(s32(g.dword(shape + 6)) >> 16);
    // The game's code hands it offsetX in ecx too, which it does not read.
    return guest::direct<0x4ef3f0, shapeDrawKeyed>(app, ts, shape, x - offsetX, y);
}

nfs::u32 frontend::shapeDrawKeyedAtHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef4c0", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 shape = args.eax, x = args.edx, top = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::shapeDrawKeyedAtHotspot(app, ts, shape, x, top); });
}

/* sub_4ef520: the font at eax taken into use. */
nfs::u32 frontend::eab::fontUse(App* app, ThreadState& ts, u32 font)
{
    constexpr u32 kFontMagic = 0x464e5400;  // "FNT" in the file's first three bytes
    constexpr u32 kReaderBytes = 0x50b540, kReaderWords = 0x50b580;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    const u32 first = g.dword(font);
    const u32 tag = ((first >> 24) | ((first >> 8) & 0xff00) | ((first << 8) & 0xff0000) | (first << 24)) & 0xffffff00;
    if (tag != kFontMagic)
    {
        errorAt(g, 0x54c4cc, 0x54c4dc, 0x54);
        error(g, 0x54c4e4, { font });
    }
    g.setDword(kFontOriginX, signedByteAt(g, font + 0x10));
    g.setDword(kFontOriginY, signedByteAt(g, font + 0x11));
    g.setDword(kFontAscent, signedByteAt(g, font + 0x12));
    const u32 descent = signedByteAt(g, font + 0x13);
    g.setDword(0x567274, font);
    g.setDword(kFontDescent, descent);
    const u32 height = descent + g.dword(kFontAscent);
    const u32 pixels = g.dword(font + 0x1c) + font;
    g.setDword(kFontVersion, 0x64);
    g.setDword(0x56720c, 1);
    g.setDword(kFontPixels, pixels);
    g.setDword(kFontGlyphCount, g.word(font + 0xa));
    g.setDword(kFontHeight, height);
    g.setDword(0x5672a4, 0);
    g.setDword(kKerningTable, 0);
    g.setDword(0x56721c, height);
    g.setDword(kFontGlyphs, font + g.dword(font + 0x14));
    const u32 pixelBits = g.byte((g.dword(pixels) & 0x7f) + kShapeBits);
    g.setDword(kFontBits, pixelBits);
    const u32 wide = u32(s32(g.dword(pixels + 2)) >> 16);
    const u32 rowBits = (pixelBits * wide + 7) & ~u32(7);
    g.setDword(kFontPitch, u32(s32(rowBits) >> 3));
    // Wide characters when the file says so, or when it has more glyphs than a byte counts.
    const bool wideText = (u32(s32(g.dword(font + 0xc)) >> 16) & 3) == 2
        || s32(g.dword(kFontGlyphCount)) > 0xff || wordAt(g, font + 0x20) > 0xff;
    g.setDword(kFontReader, wideText ? kReaderWords : kReaderBytes);
    g.setDword(kFontColourA, 0);
    g.setDword(kFontColourB, 0);
    g.setDword(kFontRampGood, 0);
    fe::direct<kFill, ea::clearBytes>(g.app(), g.ts(), kFontRamp, 0x40);
    const u32 result = fe::direct<0x50c0d0, watb::initTextDraw>(app, ts);
    g.setDword(kFontPerChar, 0);
    return result;
}

nfs::u32 frontend::fontUse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef520", 0, 0, nullptr, 0, kRestoreAll };
    const u32 font = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fontUse(app, ts, font); });
}

/* sub_4ef6c0: the palette's entry nearest to the colour in edx, of those in use at
 * 0x5650c4. */
nfs::u32 frontend::eab::paletteNearest(App* app, ThreadState& ts, u32 colour)
{
    Guest g(app, ts);
    const s32 red = s32((colour >> 16) & 0xff), green = s32((colour >> 8) & 0xff), blue = s32(colour & 0xff);
    s32 nearest = 0x1869f;
    u32 best = 0;
    for (u32 i = 0; i < 0x100; ++i)
    {
        const u32 entry = kPalette32 + i * 4;
        if (g.byte(0x5650c4 + i) == 0)
            continue;
        const s32 dg = green - s32(g.byte(entry + 1));
        const s32 dr = red - s32(g.byte(entry + 2));
        const s32 db = blue - s32(g.byte(entry));
        const s32 distance = db * db + dg * dg + dr * dr;
        if (distance < nearest)
        {
            best = i;
            nearest = distance;
        }
    }
    return best;
}

nfs::u32 frontend::paletteNearest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef6c0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 colour = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::paletteNearest(app, ts, colour); });
}

/* sub_4ef760: the ARGB colour in eax as the screen's depth (0x565010) stores it. */
nfs::u32 frontend::eab::colourPack(App* app, ThreadState& ts, u32 colour)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const std::uint8_t depth = g.byte(0x565010);
    const u32 blue5 = (colour >> 3) & 0x1f;
    if (depth <= 8)
    {
        const u32 table = g.dword(0x5650c0);
        if (table != 0)
            return g.byte(table + (((colour & 0xf80000) >> 9) + blue5 + ((colour & 0xf800) >> 6)));
        return guest::direct<0x4ef6c0, paletteNearest>(app, ts, colour);  // the palette in eax too, not read
    }
    if (depth == 0xf)
        return colour == 0 ? 0 : ((colour & 0xf80000) >> 9) + blue5 + ((colour & 0xf800) >> 6) + 0x8000;
    if (depth == 0x10)
        return ((colour & 0xf80000) >> 8) + blue5 + ((colour & 0xfc00) >> 5);
    return depth < 0x18 ? 0 : colour;
}

nfs::u32 frontend::colourPack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef760", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 colour = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::colourPack(app, ts, colour); });
}

/* sub_4ef830: the font's two colours (eax, edx): sixteen steps from one to the other for
 * the shades of an anti-aliased glyph, packed for the screen. */
nfs::u32 frontend::eab::fontSetColours(App* app, ThreadState& ts, u32 ink, u32 back)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x48);
    const u32 depth = g.byte(0x565010);
    g.setDword(0x5672a4, 0);
    u32 result = (ink & 0xffff00ff) | (depth << 8);
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
    const u32 inkArgb = fe::direct<0x50c3a0, watb::colourToArgb>(app, ts, ink);
    const u32 backArgb = fe::direct<0x50c3a0, watb::colourToArgb>(app, ts, back);
    const u32 inkRed = (inkArgb >> 16) & 0xff, inkGreen = (inkArgb >> 8) & 0xff, inkBlue = inkArgb & 0xff;
    const u32 backRed = (backArgb >> 16) & 0xff, backGreen = (backArgb >> 8) & 0xff, backBlue = backArgb & 0xff;
    for (u32 step = 0; step < 16; ++step)
    {
        const u32 red = (backRed * (15 - step) + inkRed * step) >> 4;
        const u32 green = (backGreen * (15 - step) + inkGreen * step) >> 4;
        const u32 blue = (backBlue * (15 - step) + inkBlue * step) >> 4;
        const u32 packed =
            guest::direct<0x4ef760, colourPack>(app, ts, (step << 28) | (red << 16) | (green << 8) | blue);
        g.setDword(kFontRamp + step * 4, packed);
    }
    return fe::direct<0x50c180, watb::textPalette>(app, ts);
}

nfs::u32 frontend::fontSetColours(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ef830", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 ink = args.eax, back = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fontSetColours(app, ts, ink, back); });
}

/* sub_4efa10: the record for the character eax among ebx records of ecx bytes at edx,
 * sorted by the word at each one's start; 0 if none. */
nfs::u32 frontend::eab::fontGlyphSearch(App* app, ThreadState& ts, u32 character, u32 table, u32 count, u32 stride)
{
    Guest g(app, ts);
    u32 left = count;
    while (left != 0)
    {
        const u32 probe = table + u32(s32(left) >> 1) * stride;
        const u32 difference = character - wordAt(g, probe);
        if (difference == 0)
            return probe;
        if (s32(difference) > 0)
        {
            --left;
            table = probe + stride;
        }
        left = u32(s32(left) >> 1);
    }
    return 0;
}

nfs::u32 frontend::fontGlyphSearch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efa10", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 character = args.eax, table = args.edx, count = args.ebx, stride = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fontGlyphSearch(app, ts, character, table, count, stride); });
}

/* sub_4efa70: the glyph of the character eax in the font in use: where a table of the
 * printable characters would have it, else searched for. */
nfs::u32 frontend::eab::fontGlyphFind(App* app, ThreadState& ts, u32 character)
{
    constexpr u32 kGlyphBytes = 0xb;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    const u32 version = g.dword(kFontVersion);
    const u32 glyphs = g.dword(kFontGlyphs);
    if (s32(version) < 0x64)
    {
        errorAt(g, 0x54c5ac, 0x54c5bc, 0x64);
        error(g, 0x54c5cc);
    }
    const u32 guess = glyphs + (character - 0x20) * kGlyphBytes;
    if (wordAt(g, guess) == character)
        return guess;
    return guest::direct<0x4efa10, fontGlyphSearch>(app, ts, character, glyphs, g.dword(kFontGlyphCount), kGlyphBytes);
}

nfs::u32 frontend::fontGlyphFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efa70", 0, 0, nullptr, 0, kRestoreAll };
    const u32 character = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fontGlyphFind(app, ts, character); });
}

/* sub_4efb10: the kerning of the pair eax, edx in the font's table. */
nfs::u32 frontend::eab::fontKerning(App* app, ThreadState& ts, u32 character, u32 previous)
{
    Guest g(app, ts);
    const std::uint16_t second = std::uint16_t(character), first = std::uint16_t(previous);
    const u32 count = g.dword(kKerningCount);
    u32 pair = g.dword(kKerningTable);
    u32 i = 0;
    for (; s32(i) < s32(count); ++i, pair += 8)
    {
        if (second == g.word(pair + 2) && first == g.word(pair))
        {
            return signedByteAt(g, pair + 4);
        }
    }
    return 0;
}

nfs::u32 frontend::fontKerning(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efb10", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi };
    const u32 character = args.eax, previous = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fontKerning(app, ts, character, previous); });
}

/* sub_4efb50: the text at eax drawn at the pen, which ends where it began: newlines,
 * tabs on the stops at 0x5651c8, kerning, and a box for what has no glyph if asked. */
nfs::u32 frontend::eab::textDrawAtPen(App* app, ThreadState& ts, u32 text)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x5c);
    const u32 cursor = g.frame();           // what the font's reader moves along
    const u32 offsetX = g.frame() + 4, offsetY = g.frame() + 8;  // what the per-character hook gives
    u32 x = g.dword(kPenX) + g.dword(kFontOriginX);
    u32 y = g.dword(kPenY) + g.dword(kFontOriginY);
    g.setDword(cursor, text);
    const u32 draw = g.dword(kFontDraw), reader = g.dword(kFontReader);
    const u32 perChar = g.dword(kFontPerChar), kerning = g.dword(kKerningTable);
    const u32 lineStart = g.dword(kPenX);
    if (text == 0)
    {
        errorAt(g, 0x54c5ac, 0x54c600, 0x9e);
        error(g, 0x54c608);
    }
    if (g.dword(kFontBefore) != 0)
        fe::pointerCall(g, g.dword(kFontBefore));
    u32 spaceAdvance = 0;
    const u32 space = guest::direct<0x4efa70, fontGlyphFind>(app, ts, 0x20u);
    if (space != 0)
        spaceAdvance = signedByteAt(g, space + 8);
    u32 character = 0;
    u32 index = u32(-1);
    for (;;)
    {
        const u32 previous = character;
        nfs::Args readerLeft;
        character = fe::pointerCall(g, reader, args({ cursor, index + 1 }), readerLeft);
        index = readerLeft.edx;
        if (s32(character) <= 0x20)
        {
            if (character == 0)
                break;
            if (character == 10)
            {
                const u32 line = g.dword(kFontAscent) + g.dword(kFontDescent);
                x = lineStart;
                y += line;
                continue;
            }
            if (g.dword(0x5651c8) == 0 || character != 9)
            {
                x += spaceAdvance;
                continue;
            }
            const s32 stops = s32(g.dword(0x5651c4));
            for (s32 i = 0; i < stops; ++i)
            {
                const u32 stop = g.dword(g.dword(0x5651c8) + u32(i) * 4);
                if (s32(x) < s32(stop))
                {
                    x = stop;
                    break;
                }
            }
            continue;
        }
        const u32 glyph = guest::direct<0x4efa70, fontGlyphFind>(app, ts, character);
        if (glyph == 0)
        {
            if (g.dword(0x5651cc) != 0)
            {
                const u32 tall = half(g.dword(kFontHeight));
                const u32 wide = half(tall);
                // The game's code hands it y in edx too, which it keeps: the box's top, its height tall.
                const u32 colour = guest::direct<0x4ef760, colourPack>(app, ts, 0xffaa0000u);
                fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, x, y, wide - 2, tall, colour);
                x += wide;
            }
            continue;
        }
        const u32 glyphWide = wordAt(g, glyph + 4), glyphTall = wordAt(g, glyph + 6);
        g.setDword(offsetX, 0);
        g.setDword(offsetY, 0);
        if (kerning != 0)
            x += guest::direct<0x4efb10, fontKerning>(app, ts, character, previous);
        const u32 penX = x + signedByteAt(g, glyph + 9);
        const u32 penY = y + signedByteAt(g, glyph + 0xa);
        if (perChar != 0)
            fe::pointerCall(g, perChar, args({ offsetX, offsetY, text, index }));
        fe::pointerCall(g, draw, args({ penX + g.dword(offsetX), penY + g.dword(offsetY), g.dword(kFontPixels), glyphWide },
                          { glyphTall, g.byte(glyph + 2), g.byte(glyph + 3), g.dword(kFontPitch) }));
        x += signedByteAt(g, glyph + 8);
    }
    if (g.dword(kFontAfter) != 0)
        fe::pointerCall(g, g.dword(kFontAfter));
    const u32 originX = g.dword(kFontOriginX);
    const u32 originY = g.dword(kFontOriginY);
    g.setDword(kPenX, x - originX);
    g.setDword(kPenY, y - originY);
    return originY;
}

nfs::u32 frontend::textDrawAtPen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efb50", 0, 0, nullptr, 0, kRestoreAll };
    const u32 text = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::textDrawAtPen(app, ts, text); });
}

/* sub_4efe30: the pen moved to edx, ebx and the text at eax drawn. */
nfs::u32 frontend::eab::textDrawAt(App* app, ThreadState& ts, u32 text, u32 x, u32 y)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    fe::direct<0x4f0850, wata::textSetPen>(app, ts, x, y);
    return guest::direct<0x4efb50, textDrawAtPen>(app, ts, text);
}

nfs::u32 frontend::textDrawAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efe30", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 text = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::textDrawAt(app, ts, text, x, y); });
}

/* sub_4efe50: how wide the first edx characters of the text at eax are. */
nfs::u32 frontend::eab::textMeasureUpTo(App* app, ThreadState& ts, u32 text, u32 limit)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x20);
    const u32 cursor = g.frame();
    g.setDword(cursor, text);
    const u32 kerning = g.dword(kKerningTable);
    u32 width = 0, previous = 0;
    s32 count = 0;
    if (text == 0)
    {
        errorAt(g, 0x54c624, 0x54c634, 0x47);
        error(g, 0x54c640);
    }
    nfs::Args readerLeft;  // what the reader leaves in edx
    while (count < s32(limit))
    {
        const u32 character = fe::pointerCall(g, g.dword(kFontReader), args({ cursor, previous }), readerLeft);
        previous = character;
        if (character == 0)
            break;
        const u32 glyph = guest::direct<0x4efa70, fontGlyphFind>(app, ts, character);
        ++count;
        if (glyph == 0)
            continue;
        width += signedByteAt(g, glyph + 8);
        if (kerning != 0)
            width += guest::direct<0x4efb10, fontKerning>(app, ts, character, readerLeft.edx);  // edx as the reader left it
    }
    return width;
}

nfs::u32 frontend::textMeasureUpTo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efe50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 text = args.eax, limit = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::textMeasureUpTo(app, ts, text, limit); });
}

/* sub_4efef0: how wide a text is, up to 256 characters. */
nfs::u32 frontend::eab::textMeasureAll(App* app, ThreadState& ts, u32 text)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4efe50, textMeasureUpTo>(app, ts, text, 0x100u);
}

nfs::u32 frontend::textMeasureAll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4efef0", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 text = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::textMeasureAll(app, ts, text); });
}

/* sub_4eff00: a bitmap freed: its surface if it was the screen's, its rows, its memory. */
nfs::u32 frontend::eab::windowBitmapFree(App* app, ThreadState& ts, u32 bitmap)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 surface = g.dword(bitmap + 0x34);
    if (surface != 0)
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, ts, kDefaultDisplay, surface);
    const u32 scanRow = g.dword(bitmap + 0x2c);
    if (scanRow != 0)
        fe::direct<0x509b10, watb::removeRow>(app, ts, scanRow);
    const u32 pixelRow = g.dword(bitmap + 0x30);
    if (pixelRow != 0)
        fe::direct<0x509b10, watb::removeRow>(app, ts, pixelRow);
    g.setByte(bitmap + 0x1d, 0);
    g.setByte(bitmap + 0x1c, 0);
    g.setDword(bitmap, 0x57494e6b);
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, bitmap);
}

nfs::u32 frontend::windowBitmapFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eff00", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 bitmap = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::windowBitmapFree(app, ts, bitmap); });
}

/* sub_4f00d0: the next key (sub_4cc970), its low word. */
nfs::u32 frontend::eab::keyNextLow16(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return fe::direct<0x4cc970, glue::nextKey>(app, ts) & 0xffff;
}

nfs::u32 frontend::keyNextLow16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f00d0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::keyNextLow16(app, ts); });
}

/* sub_4f00e0: the next key handed on to sub_50c584. */
nfs::u32 frontend::eab::keyNextDispatch(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return fe::direct<0x50c584, watb::dispatchKey>(app, ts, fe::direct<0x4cc970, glue::nextKey>(app, ts) & 0xffff);
}

nfs::u32 frontend::keyNextDispatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f00e0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::keyNextDispatch(app, ts); });
}

/* sub_4f00f0: the flag at 0x5651d0 set from eax. */
nfs::u32 frontend::eab::keySetHookFlag(App* app, ThreadState& ts, u32 flag)
{
    Guest g(app, ts);
    g.setDword(0x5651d0, flag);
    return flag;
}

nfs::u32 frontend::keySetHookFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f00f0", 0, 0, nullptr, 0, 0 };
    const u32 flag = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::keySetHookFlag(app, ts, flag); });
}

/* sub_4f01f0: the keys' table at 0x564818 cleared and the state marked open, with its
 * lock made the first time. */
nfs::u32 frontend::eab::inputStateOpen(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    if (g.dword(0x561290) == 0)
        g.setDword(0x561290, *fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    g.setDword(0x7f7488, 1);
    fe::direct<kFill, ea::clearBytes>(app, ts, 0x564818u, 0x80u);
    return 0x564818 + 0x80;  // eax past what it cleared, as the fill leaves it
}

nfs::u32 frontend::inputStateOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f01f0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::inputStateOpen(app, ts); });
}

/* sub_4f0230: the state marked closed. */
nfs::u32 frontend::eab::inputStateClosedWith(App* app, ThreadState& ts, u32 eax)
{
    Guest g(app, ts);
    g.setDword(0x7f7488, 0);
    return eax;
}

nfs::u32 frontend::eab::inputStateClose(App* app, ThreadState& ts)
{
    return inputStateClosedWith(app, ts, 0);
}

nfs::u32 frontend::inputStateClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f0230", 0, 0, nullptr, 0, kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::inputStateClosedWith(app, ts, args.eax); });
}

/* sub_4f22e0: the file named at eax read into edx, its size kept at 0x5643f8.  edx, or 0 if
 * empty.  Port: the music stream is no longer topped up before the read (sub_4ff7c0). */
nfs::u32 frontend::eab::fileLoadInto(App* app, ThreadState& ts, u32 name, u32 into)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    const u32 size = g.frame(), handle = g.frame() + 8;
    fe::direct<0x4e28d0, ea::openToRead>(app, ts, name, handle, g.frame() + 4, size);
    if (g.dword(size) == 0)
        return 0;
    fe::direct<0x4e2de0, ea::readChecked>(app, ts, g.dword(handle), into, g.dword(size));
    g.setDword(0x5643f8, g.dword(size));
    fe::direct<0x4e2da0, ea::closeChecked>(app, ts, g.dword(handle));
    return into;
}

nfs::u32 frontend::fileLoadInto(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f22e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 name = args.eax, into = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::fileLoadInto(app, ts, name, into); });
}

/* sub_4f24e0: a shape's record of type 0x6f (its hot spot), past the record's head; 0 if
 * none.  A shape of a type with no pixels is an error. */
nfs::u32 frontend::eab::shapeFindHotspot(App* app, ThreadState& ts, u32 shape)
{
    constexpr u32 kHotspotRecord = 0x6f;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    if (shape == 0)
        return 0;
    if (g.byte((g.dword(shape) & 0x7f) + kShapeBits) == 0)
    {
        errorAt(g, 0x54cad4, 0x54cb28, 0x69);
        error(g, 0x54cb38, { g.dword(shape) & 0xff });
    }
    u32 record = shape;
    while (record != 0)
    {
        if ((g.dword(record) & 0xff) == kHotspotRecord)
            return record + 8;
        const u32 next = u32(s32(g.dword(record)) >> 8);
        record = next == 0 ? 0 : record + next;
    }
    return 0;
}

nfs::u32 frontend::shapeFindHotspot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f24e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 shape = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::shapeFindHotspot(app, ts, shape); });
}

/* sub_4f2790: GetTickCount. */
nfs::u32 frontend::eab::lowTimerTicks(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    return os::time::ticks(g.app(), ts);
}

nfs::u32 frontend::lowTimerTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2790", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lowTimerTicks(app, ts); });
}

namespace
{
constexpr u32 kLowTimerLock = 0x9f0c34, kLowTimerTable = 0x9f0c00, kLowTimerEvent = 0x5652ac;
}

/* sub_4f27a0: the low timer's thread: at each signal of its event, the functions of the
 * table run in turn, until its lock is gone. */
nfs::u32 frontend::eab::lowTimerThread(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    g.setDword(kLowTimerEvent, *fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts));
    if (g.dword(kLowTimerLock) != 0)
    {
        do
        {
            safepoint(app, ts);
            fe::direct<0x4dfb70, ea::eventWaited>(app, ts, g.dword(kLowTimerEvent));
            const u32 lock = g.dword(kLowTimerLock);
            if (lock != 0)
            {
                fe::enterLock(g, ts, lock);
                if (g.dword(kLowTimerTable) != 0)
                {
                    u32 slot = 0;
                    do
                    {
                        const u32 at = slot;
                        slot += 4;
                        fe::pointerCall(g, g.dword(at + kLowTimerTable), args({ at }));
                    } while (slot < 0x20 && g.dword(slot + kLowTimerTable) != 0);
                }
                fe::leaveLock(g, ts, g.dword(kLowTimerLock));
            }
        } while (g.dword(kLowTimerLock) != 0);
    }
    const u32 result = directKept<0x4dfc00, ea::eventClosed, 1>(g.app(), g.ts(), g.dword(kLowTimerEvent), 0);
    g.setDword(kLowTimerEvent, 0);
    return result;
}

nfs::u32 frontend::lowTimerThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f27a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEdi };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lowTimerThread(app, ts); });
}

/* sub_4f2890: the low timer started: its end at exit (0x4f2830), its lock, its thread. */
nfs::u32 frontend::eab::lowTimerStart(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    guest::direct<0x4f2a78, atexitJump>(app, ts, 0x4f2830u);
    const u32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, ts);
    g.setDword(kLowTimerLock, lock);
    fe::enterLock(g, ts, lock);
    const u32 made = *fe::direct<kCreateThread, ea::threadStarted>(app, ts, 0x4f27a0u, 0u, 2u, 0x9f0c20u);
    if (made != 0)
    {
        fe::leaveLock(g, ts, g.dword(kLowTimerLock));  // leaves eax as it is
        return made;
    }
    fe::direct<0x4eb360, ea::lockFreed>(app, ts, g.dword(kLowTimerLock));
    g.setDword(kLowTimerLock, 0);
    errorAt(g, 0x54cbd8, 0x54cbe8, 0x4d);
    return errorLeft(g, 0x54cbf8);
}

nfs::u32 frontend::lowTimerStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2890", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lowTimerStart(app, ts); });
}

/* sub_4f2930: the function eax added to the low timer's table unless it is there; the
 * timer started if it was not. */
nfs::u32 frontend::eab::lowTimerAdd(App* app, ThreadState& ts, u32 function)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    if (g.dword(kLowTimerLock) == 0)
        guest::direct<0x4f2890, lowTimerStart>(app, ts);
    fe::enterLock(g, ts, g.dword(kLowTimerLock));
    s32 empty = -1;
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 present = g.dword(kLowTimerTable + i * 4);
        if (present == 0 && empty < 0)
        {
            empty = s32(i);
            continue;
        }
        if (function == present)
        {
            return fe::leaveLock(g, ts, g.dword(kLowTimerLock));
        }
    }
    if (empty != -1)
        g.setDword(kLowTimerTable + u32(empty) * 4, function);
    return fe::leaveLock(g, ts, g.dword(kLowTimerLock));
}

nfs::u32 frontend::lowTimerAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2930", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 function = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lowTimerAdd(app, ts, function); });
}

/* sub_4f2a60: the low timer's event signalled, if it has one. */
nfs::u32 frontend::eab::lowTimerSignal(App* app, ThreadState& ts, u32 eax)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    const u32 event = g.dword(kLowTimerEvent);
    return event != 0 ? *fe::direct<0x4dfb30, ea::eventSet>(app, ts, event) : eax;
}

nfs::u32 frontend::lowTimerSignal(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2a60", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 eax = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lowTimerSignal(app, ts, eax); });
}

/* sub_4f2a78: the runtime's atexit (a jmp). */
nfs::u32 frontend::eab::atexitJump(App* app, ThreadState& ts, u32 function)
{
    Guest g(app, ts);
    return fe::direct<0x4f7c20, watcom::clib::atexit>(app, ts, function);
}

nfs::u32 frontend::atexitJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2a78", 0, 0, nullptr, 0, 0 };
    const u32 function = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::atexitJump(app, ts, function); });
}

/* sub_4f2a7d: exit(0).  What follows it is the jump table of sub_4f2ad0: it does not
 * come back. */
nfs::u32 frontend::eab::exitGameNow(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return fe::direct<0x4dfd04, watcom::clib::exit>(g.app(), g.ts(), 0);
}

nfs::u32 frontend::exitGameNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2a7d", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::exitGameNow(app, ts); });
}

/* sub_4f2ad0: memcpy of ebx bytes from eax to edx, ecx of them by bytes to align it, a
 * Duff's device on 16 words by its jump table at 0x4f2a90, the last bytes one at a time.
 * The registers are left as the device leaves them: ecx carried the words of the slots
 * in kEcxSlots. */
nfs::u32 frontend::eab::memCopyAligned(App* app, ThreadState& ts, u32 from, u32 to, u32 size, u32 lead)
{
    constexpr u32 kEcxSlots = 0x7de;
    Guest g(app, ts);
    if (lead != 0)
    {
        size -= lead;
        for (u32 i = 0; i < lead; ++i)
            g.setByte(to++, g.byte(from++));
    }
    u32 ecx = u32(s32(size) >> 2);
    const u32 entry = ecx & 0xf;
    u32 blocks = u32(s32(size) >> 6);
    const auto move = [&](u32 slot) {
        const u32 value = g.dword(from);
        from += 4;
        g.setDword(to, value);
        to += 4;
        if (((kEcxSlots >> slot) & 1) != 0)
            ecx = value;
    };
    if (entry != 0)
        for (u32 slot = 16 - entry; slot < 16; ++slot)
            move(slot);
    while (--blocks != u32(-1))
        for (u32 slot = 0; slot < 16; ++slot)
            move(slot);
    const u32 tail = size & 3;
    for (u32 i = 0; i < tail; ++i)
    {
        const std::uint8_t value = g.byte(from + i);
        ecx = ((from + i) & 0xffffff00) | value;
        g.setByte(to + i, value);
    }
    return to;
}

nfs::u32 frontend::memCopyAligned(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2ad0", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 from = args.eax, to = args.edx, size = args.ebx, lead = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memCopyAligned(app, ts, from, to, size, lead); });
}

/* sub_4f2bf0: memmove of ebx bytes from eax to edx: from the end when they overlap or are
 * few, by sub_4f2ad0 else. */
nfs::u32 frontend::eab::memMoveLibrary(App* app, ThreadState& ts, u32 from, u32 to, u32 size)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    if (s32(size) >= 4 && (to <= from || to >= from + size))
        return guest::direct<0x4f2ad0, memCopyAligned>(app, ts, from, to, size, (4 - (from & 3)) & 3);
    for (u32 left = size; left-- != 0;)
        g.setByte(to + left, g.byte(from + left));
    return from;
}

nfs::u32 frontend::memMoveLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2bf0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const u32 from = args.eax, to = args.edx, size = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveLibrary(app, ts, from, to, size); });
}

/* sub_4f2c40 and its like: sub_4f2bf0 from edx to eax (these take the destination
 * first), ebx counting half bytes, bytes, words, three bytes or longs. */
nfs::u32 frontend::eab::memMoveHalfBytes(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, ts, from, to, u32(s32(count) >> 1));
}

nfs::u32 frontend::memMoveHalfBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2c40", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveHalfBytes(app, ts, to, from, count); });
}

nfs::u32 frontend::eab::memMoveBytes(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, ts, from, to, count);
}

nfs::u32 frontend::memMoveBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2c50", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveBytes(app, ts, to, from, count); });
}

nfs::u32 frontend::eab::memMoveWords(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, ts, from, to, count * 2);
}

nfs::u32 frontend::memMoveWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2c60", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveWords(app, ts, to, from, count); });
}

nfs::u32 frontend::eab::memMoveTriples(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, ts, from, to, count * 3);
}

nfs::u32 frontend::memMoveTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2c70", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi };
    const u32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveTriples(app, ts, to, from, count); });
}

nfs::u32 frontend::eab::memMoveLongs(App* app, ThreadState& ts, u32 to, u32 from, u32 count)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4f2bf0, memMoveLibrary>(app, ts, from, to, count << 2);
}

nfs::u32 frontend::memMoveLongs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2c90", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 to = args.eax, from = args.edx, count = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memMoveLongs(app, ts, to, from, count); });
}

/* sub_4f2cb0: a shape moved (eax) to a place in memory (edx, ebx), the place's low twelve
 * bits of each kept in its fields at 0xc and 0xe. */
nfs::u32 frontend::eab::shapeMoveWithSpot(App* app, ThreadState& ts, u32 shape, u32 x, u32 y)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 result = fe::direct<0x50b0b0, watb::moveShape>(app, ts, shape, x, y, 0x5652b0u);
    g.setWord(shape + 0xc, g.word(shape + 0xc) & 0xf000);
    g.setDword(shape + 0xc, g.dword(shape + 0xc) | (x & 0xfff));
    // The code's ebx is stored by its low half alone: the word's high bits.
    const u32 kept = g.word(shape + 0xe) & 0xf000;
    g.setWord(shape + 0xe, std::uint16_t(kept));
    g.setDword(shape + 0xc, g.dword(shape + 0xc) | ((y & 0xfff) << 16));
    return result;
}

nfs::u32 frontend::shapeMoveWithSpot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f2cb0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 shape = args.eax, x = args.edx, y = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::shapeMoveWithSpot(app, ts, shape, x, y); });
}

namespace
{
constexpr u32 kTcpLogLevel = 0x566de0;     // the transport's trace, from 4 on
constexpr u32 kSockPort = 0x00, kSockThread = 0x04, kSockHandle = 0x18, kSockCallback = 0x3c, kSockArgument = 0x40;
constexpr u32 kSockStop = 0x46, kSockRunning = 0x47;
}

/* sub_4f7450: a listening socket on the port eax: its record, its thread (sub_4f68c0)
 * waited for till it runs or gives up.  The record, or 0. */
nfs::u32 frontend::eab::tcpOpen(App* app, ThreadState& ts, u32 port, u32 callback, u32 argument)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x28);
    const u32 portSlot = g.frame() + 0x10;
    g.setDword(portSlot, port);
    errorAt(g, 0x54d2d0, 0x54d358, 0x31a);
    const u32 record = *fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54d368u, 0x48u, g.dword(0x5643f4));
    fe::direct<kFill, ea::clearBytes>(g.app(), g.ts(), record, 0x48);
    g.setDword(record + 0x1c, u32(-1));
    const u32 handle = fe::direct<0x4f6490, netb::tcpListenSocket>(app, ts, g.dword(portSlot));
    g.setDword(record + kSockHandle, handle);
    g.setDword(record + kSockCallback, callback);
    g.setDword(record + kSockArgument, argument);
    const u32 given = g.dword(portSlot);
    if (given != 0)
    {
        g.setDword(record + kSockPort, given);
    }
    else
    {
        // No port asked for: the one the system gave, kept as an address holds it, in network order.
        if (const auto own = os::net::localName(app, g.dword(record + kSockHandle)))
            g.setDword(record + kSockPort, os::net::networkOrder(own->port));
    }
    if (s32(g.dword(kTcpLogLevel)) >= 4)
        fe::direct<0x512180, netb::commEvent>(app, ts, 0x54d370u, portSlot, record + kSockHandle, 4u, 1u, 0u);
    u32 running = 0;
    const u32 socket = g.dword(record + kSockHandle);
    if (socket != os::net::kNoSocket && os::net::listen(app, socket, 5) == os::net::Error::none)
    {
        g.setDword(0x9f37f0, record);
        if (*fe::direct<kCreateThread, ea::threadStarted>(app, ts, 0x4f68c0u, 0u, 2u, record + kSockThread) != 0)
        {
            running = 1;
            for (;;)
            {
                safepoint(app, ts);
                const std::uint8_t state = g.byte(record + kSockRunning);
                if (state != 0 || state != g.byte(record + kSockStop))
                {
                    running = g.byte(record + kSockRunning);
                    break;
                }
                if (!*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0u))
                    fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);
                else
                    fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1u);
            }
        }
    }
    if (running != 0)
        return record;
    if (s32(g.dword(kTcpLogLevel)) >= 4)
        fe::direct<0x512180, netb::commEvent>(app, ts, 0x54d380u, 0u, 0u, 0u, 1u, 0u);
    guest::direct<0x4f75d0, tcpClose>(app, ts, record);
    return 0;
}

nfs::u32 frontend::tcpOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7450", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 port = args.eax, callback = args.edx, argument = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpOpen(app, ts, port, callback, argument); });
}

/* sub_4f75d0: a socket's record closed: its thread told and waited for, the socket, the
 * memory. */
nfs::u32 frontend::eab::tcpClose(App* app, ThreadState& ts, u32 record)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    if (s32(g.dword(kTcpLogLevel)) >= 4)
    {
        fe::direct<0x512180, netb::commEvent>(app, ts, 0x54d390u, 0u, 0u, 0u, 0u, 0u);
    }
    if (g.byte(record + kSockRunning) != 0)
    {
        g.setByte(record + kSockStop, 1);
        do
        {
            safepoint(app, ts);
            if (!*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0u))
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);  // the 0 the call before left in eax
            else
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1u);
        } while (g.byte(record + kSockRunning) != 0);
    }
    const u32 socket = g.dword(record + kSockHandle);
    if (socket != os::net::kNoSocket)
        os::net::close(app, ts, socket);
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, record);
    return 1;
}

nfs::u32 frontend::tcpClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f75d0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 record = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpClose(app, ts, record); });
}

/* sub_4f7660: the first field of a record. */
nfs::u32 frontend::eab::tcpFirstField(App* app, ThreadState& ts, u32 record)
{
    Guest g(app, ts);
    return g.dword(record);
}

nfs::u32 frontend::tcpFirstField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7660", 0, 0, nullptr, 0, 0 };
    const u32 record = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpFirstField(app, ts, record); });
}

/* sub_4f7670: a name (edx) made an address (ecx) on the port ebx, or on the word at eax if
 * that is 0: a dotted number or a host's.  1, or 0 for no name, an unfinished one (the
 * dot at its end) or no such host. */
nfs::u32 frontend::eab::tcpResolve(App* app, ThreadState& ts, u32 portAt, u32 name, u32 port, u32 address)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    if (name == 0)
        return 0;
    const u32 length = g.length(name);
    if (length == 0 || g.byte(name + length - 1) == 0x2e)
        return 0;
    const std::optional<std::uint32_t> number = os::net::resolve(app, ts, guest::text(app, name));
    // The address as the game keeps it (os_net.h), the internet's, in the 20 bytes it has room for.
    fe::direct<kFill, ea::clearBytes>(g.app(), g.ts(), address, 0x14);
    g.setWord(address, 2);
    if (!number)
        return 0;
    g.setDword(address + 4, *number);
    std::uint16_t value = std::uint16_t(port);
    if (port == 0)
    {
        value = g.word(portAt);
    }
    g.setWord(address + 2, os::net::networkOrder(value));
    return 1;
}

nfs::u32 frontend::tcpResolve(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7670", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 portAt = args.eax, name = args.edx, port = args.ebx, address = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpResolve(app, ts, portAt, name, port, address); });
}

/* sub_4f7750: an address (edx) as text at ebx, its port through ecx if asked for. */
nfs::u32 frontend::eab::tcpAddressText(App* app, ThreadState& ts, u32 address, u32 text, u32 portOut)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 number = g.dword(address + 4);
    guest::putText(app, text, os::net::addressText(number).c_str());
    if (portOut != 0)
    {
        g.setDword(portOut, os::net::networkOrder(g.word(address + 2)));
    }
    return 1;
}

nfs::u32 frontend::tcpAddressText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f7750", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 address = args.edx, text = args.ebx, portOut = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpAddressText(app, ts, address, text, portOut); });
}

/* sub_4f77c0: whether the 16 bytes at edx and at ebx are the same. */
nfs::u32 frontend::eab::tcpSameAddress(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest g(app, ts);
    for (u32 i = 0; i < 0x10; ++i)
        if (g.byte(a + i) != g.byte(b + i))
            return 0;
    return 1;
}

nfs::u32 frontend::tcpSameAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f77c0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 a = args.edx, b = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpSameAddress(app, ts, a, b); });
}

/* sub_4f77f0: a socket's own address (getsockname) into edx. */
nfs::u32 frontend::eab::tcpLocalName(App* app, ThreadState& ts, u32 record, u32 out)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const u32 socket = g.dword(record + kSockHandle);
    const std::optional<os::net::Endpoint> own = os::net::localName(app, socket);
    if (!own)
        return 0;
    os::net::putInet(app, out, *own);
    return 1;
}

nfs::u32 frontend::tcpLocalName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f77f0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const u32 record = args.eax, out = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::tcpLocalName(app, ts, record, out); });
}

/* sub_4f9088: the end of sub_4f9070, called on its own: 0. */
nfs::u32 frontend::eab::lineNoFlag(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::lineNoFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9088", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lineNoFlag(app, ts); });
}

namespace
{
constexpr u32 kLineText = 0x9f4dac, kLineLast = 0x56abb4, kLineFormat = 0x5347b0;
constexpr u32 kLineDone = 0x9f4d98, kLineResult = 0x9f4d9c, kLineCall = 0x9f4da4, kLineHandle = 0x9f4da0;

// A line in the modem's log; the text kept as the last one.
u32 lineLog(Guest& g, u32 text)
{
    g.setDword(kLineLast, text);
    return *fe::direct<0x501050, ea::reportedAt>(g.app(), g.ts(), kLineLog, 1u, text, guest::Words({}));
}

/* wsprintfA's formatting: the port formats nothing here (it says so in the log). */
void lineFormat(Guest& g, const Arguments& arguments)
{
    static_cast<void>(g);
    static_cast<void>(arguments);
    os::debugBreak();
}

u32 lineFinish(Guest& g, u32 result, u32 eax)
{
    g.setDword(kLineResult, result);
    g.setDword(kLineDone, 1);
    return eax;  // what the caller held in eax: nothing here changes it
}
}

/* sub_4f9500: TAPI's callback (hDevice, message, instance, three parameters; stdcall): each
 * message told to the log; a call's state answered or dropped, its end the result the
 * connecting screen waits for at 0x9f4d9c, under 0x9f4d98. */
nfs::u32 frontend::eab::lineCallback(App* app, ThreadState& ts, u32 device, u32 what, u32 instance, u32 p1, u32 p2,
                                       u32 p3)
{
    constexpr u32 kReply = 12, kCallInfo = 8, kCallState = 2, kNewCall = 1;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x58);
    static_cast<void>(instance);  // the callback's instance word: not read
    const auto format = [&](u32 text) { lineFormat(g, args({}, { kLineText, text, p1, p2, p3 })); };
    switch (what)
    {
    case kNewCall:
        format(0x54d998);
        return lineLog(g, kLineText);
    case kCallInfo:
        format(0x54d934);
        lineLog(g, kLineText);
        return fe::direct<0x530b4d, watc::lineGetLineDevStatus>(app, ts, g.dword(kLineHandle), g.frame());
    case kReply:
    {
        format(0x54d968);
        const u32 reported = lineLog(g, kLineText);
        return p2 != 0 ? lineFinish(g, 0xf, reported) : reported;
    }
    case kCallState:
    {
        format(0x54d9cc);
        const u32 reported = lineLog(g, kLineText);
        switch (p1)
        {
        case 1:
        {
            const u32 left = fe::direct<0x52f969, watc::lineDeallocateCall>(app, ts, g.dword(kLineCall));
            g.setDword(kLineDone, p1);  // ebx, which the code compared with 1 to come here, and the call keeps
            return left;
        }
        case 2:
            fe::direct<0x52f535, watc::lineAnswer>(app, ts, device, 0u, 0u);
            g.setDword(kLineCall, device);
            return device;
        case 0x40:
            return lineFinish(g, 0xc, reported);
        case 0x100:
        {
            const u32 modem = fe::direct<0x4fa0f0, netb::tapiGetModemHandle>(app, ts);
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

nfs::u32 frontend::lineCallback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f9500", 0x18, 0, nullptr, 0, kRestoreEbx | kRestoreEsi | kRestoreEdi };
    const u32 device = stackWord(app, ts, 0);
    const u32 what = stackWord(app, ts, 1);
    const u32 instance = stackWord(app, ts, 2);
    const u32 p1 = stackWord(app, ts, 3);
    const u32 p2 = stackWord(app, ts, 4);
    const u32 p3 = stackWord(app, ts, 5);
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lineCallback(app, ts, device, what, instance, p1, p2, p3); });
}

/* sub_4f97c0: the modem shut: a second's wait, the call dropped, the line closed, the
 * reading thread or its handle gone, the buffers freed and TAPI told. */
nfs::u32 frontend::eab::lineShutdown(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0x3e8u);
    const u32 call = g.dword(kLineCall);
    if (call != 0)
        fe::direct<0x52f969, watc::lineDeallocateCall>(app, ts, call);
    const u32 line = g.dword(kLineHandle);
    if (line != 0)
    {
        if (fe::direct<0x52f661, watc::lineClose>(app, ts, line) != 0)
            lineLog(g, 0x54da3c);
    }
    const u32 reader = g.dword(0x9f4d84);
    if (reader != 0 && g.dword(reader + 0x38) != u32(-1))
    {
        const u32 thread = g.dword(reader + 0x38);
        fe::direct<0x51b200, netc::serialClose>(app, ts, thread);
        g.setDword(0x9f4d84, 0);
    }
    else
    {
        const u32 handle = g.dword(0x9f4da8);
        if (handle != 0)
        {
            // Its handle let go; the code logs one that would not be, which the port's always are.
            os::release(app, handle);
        }
    }
    const u32 buffer = g.dword(0x9f4d88);
    if (buffer != 0)
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, buffer);
        g.setDword(0x9f4d88, 0);
    }
    if (g.dword(0x9f4d94) != 0)
    {
        for (s32 i = 0; i < s32(g.dword(0x9f4d80)); ++i)
        {
            const u32 entry = g.dword(g.dword(0x9f4d94) + u32(i) * 8 + 4);
            if (entry != 0)
                fe::direct<0x4e1890, ea::purgeBlock>(app, ts, entry);
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, g.dword(0x9f4d94));
        g.setDword(0x9f4d94, 0);
    }
    const u32 result = fe::direct<0x5324d2, watc::lineShutdown>(app, ts, g.dword(0x9f4d74));
    return result != 0 ? lineLog(g, 0x54da70) : result;
}

nfs::u32 frontend::lineShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4f97c0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::lineShutdown(app, ts); });
}

namespace
{
constexpr u32 kFileThreads = 0x9f4f60, kFileThreadIndex = 0x9f4f64, kFileRecords = 0x9f4f68, kFileLock = 0x9f4f6c;
constexpr u32 kFileFree = 0x9f4f74, kFileHandles = 0x9f4f90, kFileStop = 0x5671f0;
constexpr u32 kThreadBytes = 0x68;
constexpr u32 kQueueFree = 0x50dde0;
constexpr u32 kPulse = 0x4dfc50;
constexpr u32 kFreePages = 0x501180, kCloseHandleSlot = 0x4e3fe0;
}

/* sub_4fbc30: a file opened (eax) as edx asks; a name with a '|' is a pack and a member,
 * and is looked for among the open packs, as is a name that did not open when ebx says.
 * The handle in the frame, or 0. */
nfs::u32 frontend::eab::asyncOpenFile(App* app, ThreadState& ts, u32 name, u32 mode, u32 search)
{
    constexpr u32 kSearch = 0x104, kMember = 0x108, kResult = 0x10c, kHandle = 0x110;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x128);
    const u32 pack = g.frame();
    u32 bar = name;
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
        g.setDword(pack + kHandle, *fe::direct<0x4e3620, ea::fileHandleOpened>(app, ts, name, mode, 0u, pack + kResult));
        g.setByte(pack, 0);
    }
    else
    {
        const u32 length = bar - name;
        // By address: the ebx and edx it leaves are left at the end (edx read here).
        fe::direct<kStrncpy, watcom::clib::strncpy>(app, ts, pack, name, length);
        g.setDword(pack + kMember, bar + 1);
        g.setByte(pack + length, 0);
    }
    if (search != 0 && g.dword(pack + kHandle) == 0)
        fe::direct<0x50e9b0, netb::qsafeiterate>(app, ts, kFileHandles, 0x4fb790u, pack + kSearch);
    return g.dword(pack + kResult);
}

nfs::u32 frontend::asyncOpenFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fbc30", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 name = args.eax, mode = args.edx, search = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::asyncOpenFile(app, ts, name, mode, search); });
}

/* sub_4fbda0: a thread of the file requests: a request of the highest priority it may
 * take from its queue, served; put on the queue of what is done, its callback told. */
nfs::u32 frontend::eab::asyncFileThread(App* app, ThreadState& ts)
{
    enum { kOpen, kNothing, kRead, kWrite, kSize, kSeek, kExists, kDelete, kNoError, kMap, kRemove };
    constexpr std::uint8_t kFinished = 1, kFailed = 0xfe;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x20);
    const u32 sizeOut = g.frame();
    const u32 thread = g.dword(kFileThreads) + g.dword(kFileThreadIndex) * kThreadBytes;
    const u32 busyEvent = g.dword(thread + 0x5c);
    g.setDword(thread, 1);
    const u32 firstPulse = *fe::direct<kPulse, ea::eventPulse>(app, ts, busyEvent);
    if (g.dword(kFileStop) != 0)
    {
        g.setDword(thread, 0);
        return firstPulse;
    }
    const u32 queue = thread + 0x1c, done = thread + 0x38;
    const auto failed = [&](u32 request) { return (s32(g.dword(request + 0xc) << 0x1e) >> 0x1f) != 0; };
    const auto noteError = [&](u32 request) { g.setDword(request + 0x14, directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.ts())); };
    const auto serve = [&](u32 request) {
        const u32 operation = g.dword(request + 8);
        switch (operation)
        {
        case kOpen:
        {
            const u32 flags = g.byte(request + 0x24);
            u32 mode = 3, search = 0;
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
            const u32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, ts, g.dword(request + 0x2c), mode, search);
            g.setDword(request + 0x18, handle);
            if (handle != 0)
            {
                g.setByte(request + 0x10, kFinished);
            }
            else
            {
                const u32 code = directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.ts());
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
            if (*fe::direct<0x4e3e60, ea::handleSought>(app, ts, g.dword(request + 0x18), g.dword(request + 0x24)) != 0)
            {
                const u32 moved = operation == kRead
                    ? *fe::direct<0x4e3ae0, ea::bytesRead>(app, ts, g.dword(request + 0x18), g.dword(request + 0x2c),
                                                          g.dword(request + 0x28))
                    : *fe::direct<0x4e3cc0, ea::bytesWritten>(app, ts, g.dword(request + 0x18), g.dword(request + 0x2c),
                                                             g.dword(request + 0x28));
                g.setByte(request + 0x10, kFinished);
                g.setDword(request + 0x28, moved);
            }
            const u32 code = directKept<0x4dfcf0, ea::lastErrorOf, 0>(g.app(), g.ts());
            g.setDword(request + 0x14, code);
            if (code != 0)
                g.setByte(request + 0x10, kFailed);
            break;
        }
        case kExists:
        {
            const u32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, ts, g.dword(request + 0x2c), 5u, 1u);
            g.setDword(request + 0x18, handle);
            if (handle != 0)
            {
                fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, handle);
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
            const u32 got = *fe::direct<0x4e3980, ea::handleInfoOf>(app, ts, g.dword(request + 0x18), 0u, 0u, sizeOut, 0u);
            if (got != 0)
            {
                const u32 size = g.dword(sizeOut);
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
            if (fe::direct<0x4e4230, wina::fileSizeAndSeek>(app, ts, g.dword(request + 0x18), g.dword(request + 0x28)) != 0)
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
            const u32 record = g.dword(request + 0x2c);
            if (record == 0)
                break;
            const u32 handle = guest::direct<0x4fbc30, asyncOpenFile>(app, ts, record + 0x14, 0x805u, 1u);
            g.setDword(record + 0xc, handle);
            if (handle != 0)
            {
                g.setByte(request + 0x10, kFinished);
                g.setDword(record + 0x10, fe::direct<0x4e41f0, ea::handleMemoryOf>(app, ts, g.dword(record + 0xc)));
            }
            else
            {
                g.setByte(request + 0x10, kFailed);
                noteError(request);
            }
            break;
        }
        case kRemove:
            fe::direct<0x50e150, ea::queueRemoved>(app, ts, kFileHandles, g.dword(request + 0x2c), 0u);  // the answer is not read
            g.setByte(request + 0x10, kFinished);
            break;
        case kDelete:
            if (*fe::direct<0x4e3600, ea::fileRemoved>(app, ts, g.dword(request + 0x2c)) != 0)
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
            const u32 unknown = g.dword(g.dword(thread + 0x18) + 8);
            g.setDword(0x552190, 0x54dcac);
            g.setDword(0x552194, 0x54de64);
            error(g, 0x54de78, { unknown });
            break;
        }
        }
    };
    u32 left = 0;  // eax as the last call leaves it, this one's result
    do
    {
        safepoint(app, ts);
        const u32 token = g.dword(thread) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, ts, queue) : 0;
        const u32 next = fe::direct<0x50df60, netb::qpop>(app, ts, queue);
        u32 request = next;
        g.setDword(thread + 0x18, next);
        if (next != 0 && s32(g.byte(next + 0x11)) > s32(g.dword(thread + 0x64)))
        {
            fe::direct<0x50e0a0, netb::qinsert>(app, ts, queue, next);  // put back: in edx in the game's code
            g.setDword(thread + 0x18, 0);
            request = 0;
        }
        if (g.dword(thread) != 0)
            fe::direct<0x50ea40, menz::fileLockLeave>(app, ts, queue, token);
        if (g.dword(thread + 0x18) == 0)
        {
            left = *fe::direct<0x4dfb70, ea::eventWaited>(app, ts, g.dword(thread + 0x54));
            continue;
        }
        if (!failed(request))
            serve(request);
        const u32 token2 = g.dword(thread) != 0 ? fe::direct<0x50ea20, menz::fileLockEnter>(app, ts, queue) : 0;
        fe::direct<0x50de50, netb::qpush>(app, ts, done, request);
        const u32 live = g.dword(thread);
        g.setDword(thread + 0x18, 0);
        if (live != 0)
            fe::direct<0x50ea40, menz::fileLockLeave>(app, ts, queue, token2);
        if (g.dword(request + 0x20) != 0)
        {
            g.setByte(request + 0xc, g.byte(request + 0xc) | 8);
            const u32 status = failed(request) ? u32(-1) : u32(s32(g.dword(request + 0xd)) >> 24);
            fe::pointerCall(g, g.dword(request + 0x20), args({ g.dword(request + 4), status, g.dword(request + 0x1c) }));
        }
        left = *fe::direct<kPulse, ea::eventPulse>(app, ts, g.dword(thread + 0x5c));
    } while (g.dword(kFileStop) == 0);
    g.setDword(thread, 0);
    return left;
}

nfs::u32 frontend::asyncFileThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fbda0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::asyncFileThread(app, ts); });
}

/* sub_4fc170: the file requests ended, at exit: what is open closed, each thread woken
 * and waited for, its events and queues gone, the memory and the lock. */
nfs::u32 frontend::eab::asyncFilesShutdown(App* app, ThreadState& ts, u32 eax)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    if (g.dword(kFileThreads) == 0)
        return eax;
    for (;;)
    {
        const u32 open = fe::direct<0x50df60, netb::qpop>(app, ts, kFileHandles);
        if (open == 0)
            break;
        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, g.dword(open + 0xc));
        directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.ts(), open);
    }
    g.setDword(kFileStop, 1);
    for (u32 at = 0; at != 0xd00; at += kThreadBytes)
    {
        const u32 thread = g.dword(kFileThreads) + at;
        if (g.dword(thread) == 0)
            continue;
        fe::direct<0x4dfb30, ea::eventSet>(app, ts, g.dword(thread + 0x54));
        while (g.dword(thread) != 0)
        {
            safepoint(app, ts);
            fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);
        }
        directKept<0x4dfc00, ea::eventClosed, 1>(g.app(), g.ts(), g.dword(thread + 0x54));
        fe::direct<0x4dfc80, ea::eventSetClosed>(app, ts, g.dword(thread + 0x5c));
        directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.ts(), thread + 0x38);
        directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.ts(), thread + 0x1c);
    }
    directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.ts(), g.dword(kFileRecords));
    directKept<0x501180, ea::pagesFreed, 1>(g.app(), g.ts(), g.dword(kFileThreads));
    directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.ts(), kFileFree);
    directKept<0x50dde0, ea::queueLockFreed, 1>(g.app(), g.ts(), kFileHandles);
    fe::direct<0x4eb360, ea::lockFreed>(app, ts, g.dword(kFileLock));
    g.setDword(kFileThreads, 0);
    return 0;
}

nfs::u32 frontend::asyncFilesShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fc170", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 eax = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::asyncFilesShutdown(app, ts, eax); });
}

/* sub_4fc250: the file requests set up, once: eax handles (0: 0x40), edx records (0:
 * 0x40) on the free queue, room for the threads, the lock, the end at exit. */
nfs::u32 frontend::eab::asyncFilesInit(App* app, ThreadState& ts, u32 handles, u32 records)
{
    constexpr u32 kRecordBytes = 0x30;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const u32 size = g.frame() + 4;
    if (g.dword(kFileThreads) != 0)
        return 1;
    if (records == 0)
        records = 0x40;
    fe::direct<0x4e3430, ea::handlesMade>(app, ts, handles != 0 ? handles : 0x40);
    fe::direct<0x50dcd0, netb::qinit>(app, ts, kFileFree, 0u, 0u);
    fe::direct<0x50dcd0, netb::qinit>(app, ts, kFileHandles, 0u, 0u);
    g.setDword(size, 0xd00);
    g.setDword(kFileThreads, *fe::direct<0x501110, ea::allocatePages>(app, ts, size));
    fe::direct<0x4e070c, ea::clearBytes>(app, ts, g.dword(kFileThreads), g.dword(size));
    g.setDword(size, records * kRecordBytes);
    const u32 memory = *fe::direct<0x501110, ea::allocatePages>(app, ts, size);
    g.setDword(kFileRecords, memory);
    fe::direct<0x4e070c, ea::clearBytes>(app, ts, memory, g.dword(size));
    const u32 bytes = g.dword(size);
    u32 record = g.dword(kFileRecords);
    for (u32 left = bytes / kRecordBytes; left != 0; --left)
    {
        fe::direct<0x50de50, netb::qpush>(app, ts, kFileFree, record);
        record += kRecordBytes;
    }
    g.setDword(kFileLock, *fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    g.setDword(0x9f4f70, u32(-1));
    guest::direct<0x4f2a78, atexitJump>(app, ts, 0x4fc170u);
    return 1;
}

nfs::u32 frontend::asyncFilesInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4fc250", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const u32 handles = args.eax, records = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::asyncFilesInit(app, ts, handles, records); });
}

/* sub_514280: a game on a transport opened: eax the game's record, ecx the argument of
 * the transport's opening; the first one also starts the transport and its timer.  Its
 * address (edx), name and password (the stack's) kept.  0xfa, or 0. */
nfs::u32 frontend::eab::netOpen(App* app, ThreadState& ts, u32 net, u32 address, u32 opening, u32 name,
                                  u32 password)
{
    constexpr u32 kTransportsOpen = 0x5689e4, kNetTimer = 0x514260;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const u32 open = g.dword(kTransportsOpen) + 1;
    g.setDword(kTransportsOpen, open);
    // ecx and edx where the code has them: the table's indirect calls read them.
    nfs::Args registers;
    registers.ecx = opening;
    registers.edx = open;
    if (open == 1)
    {
        const u32 table = g.dword(net + 8);
        if (fe::pointerCall(g, g.dword(table + 0x14), args({ table }), registers) == 0)
            return 0;
        guest::direct<0x4f2930, lowTimerAdd>(app, ts, kNetTimer);
    }
    registers.ecx = opening;
    registers.edx = open;
    const u32 socket = fe::pointerCall(g, g.dword(g.dword(net + 8) + 0x1c), args({ opening, net, 0x4f53f0 }), registers);
    g.setDword(net + 0xc, socket);
    if (socket == 0)
        return 0;
    const u32 table = g.dword(net + 8);
    registers.ecx = table;
    registers.edx = open;
    fe::pointerCall(g, g.dword(table + 0x44), args({ socket, net + 0x30, 0x10 }), registers);
    g.setDword(net + 0x9c, 1);
    fe::direct<kMoveMemory, ea::moveMemory>(app, ts, net + 0x8c, address, 0x10u);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), net + 0x44, name, 0x33);
    fe::direct<kStrncpy, watcom::clib::strncpy>(g.app(), g.ts(), net + 0x78, password, 0x13);
    return 0xfa;
}

nfs::u32 frontend::netOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514280", 8, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 net = args.eax, address = args.edx, opening = args.ecx;
    const u32 name = stackWord(app, ts, 0);
    const u32 password = stackWord(app, ts, 1);
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::netOpen(app, ts, net, address, opening, name, password); });
}

/* sub_514320: a game's transport closed: the game ended, what it was waiting for done;
 * the last one stops the timer and the transport.  What the end said. */
nfs::u32 frontend::eab::netClose(App* app, ThreadState& ts, u32 net)
{
    constexpr u32 kTransportsOpen = 0x5689e4, kNetTimer = 0x514260;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 socket = g.dword(net + 0xc);
    u32 result = 1;
    if (socket == 0)
        return result;
    if (g.dword(net + 0x14) != 0 || g.dword(net + 0xa0) != 0)
        result = *fe::direct<0x4f4f90, ea::netGameEnded>(app, ts, net);
    const u32 waiting = g.dword(net + 0xa8);
    g.setDword(net + 0x9c, 0);
    if (waiting != 0)
    {
        do
        {
            safepoint(app, ts);
            if (!*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0u))
            {
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0u);  // the 0 the call before left in eax
            }
            else
            {
                // Port: the other threads get to run, the transport's among them.
                app->unlockContext(ts);
                nfs::Thread::sleep(0);
                app->lockContext(ts);
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0u);
            }
            safepoint(app, ts);
        } while (g.dword(net + 0xa8) != 0);
    }
    const u32 table = g.dword(net + 8);
    g.setDword(net + 0xc, 0);
    fe::pointerCall(g, g.dword(table + 0x20), args({ socket }));  // ebx as the calls before leave it: the socket
    const u32 open = g.dword(kTransportsOpen) - 1;
    g.setDword(kTransportsOpen, open);
    if (open != 0)
        return result;
    fe::direct<0x4f29b0, ea::removeTimer>(app, ts, kNetTimer);
    const u32 tableNow = g.dword(net + 8);
    return fe::pointerCall(g, g.dword(tableNow + 0x18), args({ tableNow }));
}

nfs::u32 frontend::netClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_514320", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 net = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::netClose(app, ts, net); });
}

/* sub_5150f0: the machine's own address as text at eax: from the network's thunks (the
 * functions at 0x568a2c and 0x568a30, if it found them), else its host name's. */
nfs::u32 frontend::eab::netLocalAddress(App* app, ThreadState& ts, u32 out)
{
    constexpr u32 kEntryBytes = 0x19c, kFile = 0x550540, kFunction = 0x550550, kBlockTag = 0x550568;
    constexpr u32 kEnum = 0x568a2c, kQuery = 0x568a30;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x148);
    const u32 valueSize = g.frame() + 0x100, value = g.frame() + 0x104;
    const u32 bufferSize = g.frame() + 0x128, count = g.frame() + 0x12c;
    g.setByte(out, 0);
    if (fe::direct<0x515050, netc::loadRasApi>(app, ts) != 0)
    {
        g.setDword(count, 0);
        g.setDword(bufferSize, kEntryBytes);
        errorAt(g, kFile, kFunction, 0x72);
        u32 list = *fe::direct<0x4e1620, ea::reserved>(app, ts, kBlockTag, kEntryBytes, g.dword(0x5643f4));
        g.setDword(list, kEntryBytes);
        if (g.dword(kEnum) != 0 && fe::pointerCall(g, g.dword(kEnum), args({}, { list, bufferSize, count })) != 0)
        {
            // Not room enough: again, the size it asked for.
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, list);  // kFile in edx too, which it does not read
            g.setDword(0x552190, kFile);
            g.setDword(0x552198, 0x79);
            g.setDword(0x552194, kFunction);
            list = *fe::direct<0x4e1620, ea::reserved>(app, ts, kBlockTag, g.dword(bufferSize), g.dword(0x5643f4));
            g.setDword(list, kEntryBytes);
            if (fe::pointerCall(g, g.dword(kEnum), args({}, { list, bufferSize, count })) != 0)
                g.setDword(count, 0);
        }
        u32 entry = list;
        for (u32 i = 0; i < g.dword(count); ++i, entry += kEntryBytes)
        {
            g.setDword(valueSize, 0x28);
            g.setDword(bufferSize, 0x28);
            if (g.dword(kQuery) == 0)
                continue;
            if (fe::pointerCall(g, g.dword(kQuery), args({}, { g.dword(entry + 4), 0x8021, valueSize, bufferSize })) == 0
                && g.dword(value) == 0)
            {
                g.copyString(out, g.frame() + 0x108);
                return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, list);
            }
        }
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, list);
    }
    // The machine's own name's address: where the others on the local network reach it.
    const std::optional<std::uint32_t> own = os::net::resolve(app, ts, os::net::kHostName);
    if (!own)
        return 0;
    guest::putText(app, out, os::net::addressText(*own).c_str());
    return 1;
}

nfs::u32 frontend::netLocalAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_5150f0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 out = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::netLocalAddress(app, ts, out); });
}

/* sub_517bf0: a pool's blocks written out: name, size, room, owner, checksum, kind, and
 * the total of what is free. */
nfs::u32 frontend::eab::memPoolDump(App* app, ThreadState& ts, u32 which)
{
    constexpr u32 kPools = 0x9ef5c0, kLine = 0x568fb4, kRow = 0x568ff7;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    const u32 pool = which & 0xf;
    message(g, kLine);
    message(g, kLine);
    message(g, 0x550614);
    u32 total = 0;
    u32 block = g.dword(g.dword(pool * 4 + kPools) + 8);
    while (block != 0)
    {
        const u32 data = block + 0x10;
        const u32 named = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), data);
        const u32 name = named != 0 ? named : 0x550658;
        const std::uint8_t flags = g.byte(block + 3);
        if ((flags & 0x40) != 0)
        {
            const u32 size = g.dword(block + 4);
            total += size;
            message(g, kRow, { 0x55065c, data, size, g.dword(block + 8) - block, g.word(block + 2), 0, 0x550610 });
        }
        else
        {
            const u32 size = g.dword(block + 4);
            u32 checksum = 0;
            if (size != 0 && (flags & 0x80) == 0)
                checksum = fe::direct<0x4f4780, wata::blockChecksum>(app, ts, data, size);
            const u32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, ts, block);
            const u32 label = g.dword(kind * 4 + 0x569020);
            const u32 sizeNow = g.dword(block + 4);
            message(g, kRow, { name, data, sizeNow, g.dword(block + 8) - block, g.word(block + 2), checksum, label });
        }
        const std::uint8_t flagsNow = g.byte(block + 3);
        if ((flagsNow & 4) != 0 && (flagsNow & 0x40) == 0)
        {
            const u32 end = data + g.dword(block + 4);
            const u32 first = g.dword(end + 4), second = g.dword(end + 8);
            if (first != 0)
                message(g, 0x550668, { first, second });
        }
        message(g, 0x550670);
        block = g.dword(block + 8);
    }
    message(g, 0x550674, { total, total });
    return message(g, kLine);
}

nfs::u32 frontend::memPoolDump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_517bf0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 which = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memPoolDump(app, ts, which); });
}

/* sub_517d60: the dump of the library's pool now. */
nfs::u32 frontend::eab::memPoolDumpDefault(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return guest::direct<0x517bf0, memPoolDump>(app, ts, g.dword(0x5643f4));
}

nfs::u32 frontend::memPoolDumpDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_517d60", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::memPoolDumpDefault(app, ts); });
}

namespace
{
u32 byteSwapped(u32 value)
{
    return (value >> 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}
}

/* sub_517e50: whether a block ends with the "CRCF" tag of what was loaded with a
 * checksum. */
nfs::u32 frontend::eab::blockHasCrcTag(App* app, ThreadState& ts, u32 block)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const u32 size = fe::direct<0x4e15d0, ea::askedSize>(app, ts, block);
    if (s32(size) < 0xc)
        return 0;
    return byteSwapped(g.dword(size + block - 0xc)) == byteSwapped(g.dword(0x5506b4)) ? 1 : 0;
}

nfs::u32 frontend::blockHasCrcTag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_517e50", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::blockHasCrcTag(app, ts, block); });
}

/* sub_517eb0: whether a block's checksum holds; one with no tag does. */
nfs::u32 frontend::eab::blockCrcGood(App* app, ThreadState& ts, u32 block)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 size = fe::direct<0x4e15d0, ea::askedSize>(app, ts, block);
    if (guest::direct<0x517e50, blockHasCrcTag>(app, ts, block) == 0)
        return 1;
    const u32 stored = g.dword(block + size - 4);
    return stored == fe::direct<0x4f4780, wata::blockChecksum>(app, ts, block, size - 0xc) ? 1 : 0;
}

nfs::u32 frontend::blockCrcGood(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_517eb0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::blockCrcGood(app, ts, block); });
}

/* sub_517f10: a block checked as edx names it: no tag or a bad sum an error if asked for
 * and 0; a good one has its tail cut off.  The block. */
nfs::u32 frontend::eab::blockCheckCrc(App* app, ThreadState& ts, u32 block, u32 name)
{
    constexpr u32 kFile = 0x5506bc, kFunction = 0x5506cc;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    if (block == 0)
        return 0;
    if (guest::direct<0x517e50, blockHasCrcTag>(app, ts, block) == 0)
    {
        if (g.dword(0x569030) == 0)
            return block;
        const u32 size = fe::direct<0x4e15d0, ea::askedSize>(app, ts, block);
        if (g.dword(0x56440c) != 0)
        {
            errorAt(g, kFile, kFunction, 0x57);
            error(g, 0x5506e0, { name, size });
        }
        return 0;
    }
    if (guest::direct<0x517eb0, blockCrcGood>(app, ts, block) == 0)
    {
        if (g.dword(0x56440c) != 0)
        {
            const u32 size = fe::direct<0x4e15d0, ea::askedSize>(app, ts, block);
            const u32 stored = g.dword(size + block - 4);
            const u32 sum = fe::direct<0x4f4780, wata::blockChecksum>(app, ts, block, size - 0xc);
            errorAt(g, kFile, kFunction, 0x68);
            error(g, 0x550720, { name, size, stored, sum });
        }
        return 0;
    }
    const u32 size = fe::direct<0x4e15d0, ea::askedSize>(app, ts, block);
    guest::direct<0x524df0, blockResize>(app, ts, block, size - g.dword(0x56902c));
    return block;
}

nfs::u32 frontend::blockCheckCrc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_517f10", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 block = args.eax, name = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::blockCheckCrc(app, ts, block, name); });
}

/* sub_524bb0: the question of leaving (Y/N) over the screen, the game left on a yes;
 * the screen let go first if it is held. */
nfs::u32 frontend::eab::exitConfirm(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const EndingFrame frame(g, ts, 0xc);
    if (g.dword(0x564364) != 0)
    {
        if (!*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0u))
        {
            fe::enterLock(g, ts, g.dword(kDisplayLock));
            fe::leaveLock(g, ts, g.dword(kDisplayLock));
        }
        else
        {
            guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
        }
    }
    const u32 y = half(g.dword(0x564388)) - 8;
    const u32 x = half(g.dword(0x564384)) - 0x50;
    const u32 answer = fe::direct<0x524970, menz::debugBoxAsk>(app, ts, 0x551e74u, x, y, 0xa0u, 0x18u, 7u);
    return answer != 0 ? guest::direct<0x4f2a7d, exitGameNow>(app, ts) : answer;
}

nfs::u32 frontend::exitConfirm(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_524bb0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::exitConfirm(app, ts); });
}

/* sub_524df0: a block (eax) made the size edx in place, as far as there is room up to
 * the next: -1 as much as can be; what is past made a free block if it is worth it. */
nfs::u32 frontend::eab::blockResize(App* app, ThreadState& ts, u32 block, u32 size)
{
    constexpr u32 kPools = 0x9ef5c0;
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x24);
    const u32 header = block - 0x10;
    const u32 owner = g.word(header + 2);
    const u32 pool = g.dword((owner & 0xf) * 4 + kPools);
    const u32 lock = g.dword(pool + 0x38);
    if (lock != 0)
        fe::enterLock(g, ts, lock);
    u32 next = g.dword(header + 8);
    if ((g.byte(next + 3) & 0x40) != 0)
    {
        fe::direct<0x4e10e0, ea::freeUnlinked>(app, ts, pool, next);
        next = g.dword(next + 8);
        g.setDword(header + 8, next);
    }
    u32 asked = size;
    if (s32(size) < 8)
    {
        if (size == u32(-1))
        {
            asked = 0x40000000;
        }
        else if (s32(size) >= 0)
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
    const u32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), block, owner);
    const u32 overhead = fe::direct<0x4e11f0, ea::blockOverhead>(app, ts, blockName, owner);
    const u32 granule = g.dword(pool + 0x28) - 1;
    u32 room = ((asked + overhead + 0x10 + granule) & ~granule) - 0x10;
    const u32 span = next - header - 0x10;
    if (s32(room) > s32(span))
    {
        size = span - overhead;
        room = span;
    }
    fe::direct<kMoveMemory, ea::moveMemory>(app, ts, block + size, g.dword(header + 4) + block, overhead);
    g.setDword(header + 4, size);
    if (s32(span - room) > 0x40)
    {
        const u32 rest = block + 0x10 + room;
        fe::direct<0x4e1120, ea::blockMarked>(app, ts, rest, 0u, 0u, 0u, 0u, header, next);
        fe::direct<0x4e1080, ea::freeLinked>(app, ts, pool, rest);
        g.setDword(next + 0xc, rest);
        g.setDword(header + 8, rest);
    }
    const u32 lockNow = g.dword(pool + 0x38);
    if (lockNow != 0)
        fe::leaveLock(g, ts, lockNow);
    return block;
}

nfs::u32 frontend::blockResize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_524df0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 block = args.eax, size = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::blockResize(app, ts, block, size); });
}

/* sub_4ea440: how many files the BIG archive at eax holds, by its kind (sub_4e9f00): kind
 * 1 a big-endian word at +4, kind 2 a big-endian dword at +8; 0 for another. */
nfs::u32 frontend::eab::bigFileCount(App* app, ThreadState& ts, u32 archive)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    const u32 kind = *fe::direct<0x4e9f00, load::bigFileKindOf>(app, ts, archive);
    if (kind == 1)
        return __builtin_bswap32(g.dword(archive + 4)) >> 16;
    if (kind == 2)
        return __builtin_bswap32(g.dword(archive + 8));
    return 0;
}

nfs::u32 frontend::bigFileCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ea440", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const u32 archive = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::bigFileCount(app, ts, archive); });
}

/* sub_4ea4a0: the name of file edx of the BIG archive at eax copied to ebx (sub_4ea290 finds
 * it), or an empty name for none.  eax: the copy, or 0. */
nfs::u32 frontend::eab::bigFileName(App* app, ThreadState& ts, u32 archive, u32 index, u32 out)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    // By address: what it leaves in ebx and edx is what this function leaves there.
    u32 from = *fe::direct<0x4ea290, load::locatedEntry>(app, ts, archive, 0u, index, 0u, 0u);
    if (from == 0)
    {
        g.setByte(out, 0);
        return 0;
    }
    for (u32 to = out;; ++from, ++to)
    {
        const std::uint8_t c = g.byte(from);
        g.setByte(to, c);
        if (c == 0)
            return out;
    }
}

nfs::u32 frontend::bigFileName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ea4a0", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 archive = args.eax, index = args.edx, out = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eab::bigFileName(app, ts, archive, index, out); });
}

}
