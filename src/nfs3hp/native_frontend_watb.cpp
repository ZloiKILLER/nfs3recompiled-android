#include "native_frontend_watb.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_rendf.h"
#include "native_frontend_wata.h"
#include "native_frontend_watc.h"
#include <nfs3hp.h>
#include <lib/app.h>

#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 watbKeyMatches(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNoMatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbSortKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbReturnZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNothing4ff620(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNothing50d6d0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNothing50dbf0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbAllocHandleJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFreeHandleJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitFloatFormatters(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitChipBugHooks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInit8087(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbShapeClut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDrawFilledRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbReadPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitVideoWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbShapeTypeForDepth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitRowTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFreeRowTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbMakeRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbRemoveRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFindBlockHeader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyNibbles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyDwords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBytesToNibbles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNibblesToBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb565ToIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To1555Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To1555(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To565Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To4444(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb1555To4444(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To8565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbIndexTo888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb565To888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8888To888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb1555To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb565To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb4444To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8555To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb8565To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watb888To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbSpanTo16Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbSpanTwoSteps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDepthIndex16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDepthIndex32(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDepthIndex15(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDepthIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbClutPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbClut16aPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbConverterFor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbConvertShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbMoveShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNextByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCharWidthEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbNextChar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbTextAlpha(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbTextPlotted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbTextMono(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFadeTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitTextDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbTextPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFreeTextDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbPlotMask8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbPlotMask16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbPlotMask24(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbPlotMask32(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbColourToArgb(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDispatchKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbRegisterKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCopyBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpanIndexed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpan1555(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpan565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpan888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpan8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBlendSpanPalette565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDrawAlphaShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbQueueFetch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFreeClosedStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbInitStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCloseStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbCloseStreamsFrom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbBuildArgv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbSplitCommandLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbLongDoubleToDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbDigitsLoop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbAfterEightDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbRoundDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbEndLongDouble(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbEightDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFourDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbTwoDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFormatF(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbEndFormat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 watbFormatE(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

/* What lies between the C runtime's modules from 0x4fb800 to 0x517200, decompiled.  The
 * rough map called it the runtime; most of it is EA's graphics library:
 *
 *   shapes     getshapeclut, getclutptr, getclut16aptr, convertshapetype and movdfl
 *              (win\shpclut.c, win\convshp.c, win\movdfl.c): a shape's palette, its
 *              pixels turned into the window's format, drawn clipped; the spans'
 *              converters, one for each pair of formats, reached through tables;
 *   windows    initvideowindow, readpixel and the row tables (win\vidwin.c, cmn\row.c);
 *   text       font\textpc.c: the three blitters, what each depth plots, the tables
 *              the font's alpha goes through;
 *   queues     qfetch (cmn\queue.c);
 *
 * and these of the runtime: the streams' start and end, the command line split into
 * argv, and __LDcvt (0x5167b3), a long double's digits for %e, %f and %g.
 *
 * The state stays in guest memory.
 *
 * Plan step 7: each function a typed core in frontend::watb (native_frontend_watb.h) with its
 * arguments as parameters and its result as its return value, in a frame of its own on the guest's
 * stack where it hands the frame's address on (fe::Frame), and a thunk under the game's name for
 * the calls by address.  What the game's code leaves in the registers it does not save is no
 * longer kept: no caller reads it.  The cores call one another and the typed cores of other files
 * as C++ (guest::direct, fe::direct); by address only what is called through a pointer (the span
 * converters' tables, the blitters, the queues' tests, the keys' handlers), what has no core yet,
 * and Error where its eax is handed back.  The entries into the middle of __LDcvt and %f's layout
 * (sub_5169ef, sub_516acc, sub_516ae2, sub_516be2, sub_516dfa) have no core: their thunks take
 * their function's frame and registers into the same typed parts the core runs. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

constexpr std::uint8_t kLeavesEdx = fe::kRestoreAll & ~fe::kRestoreEdx;
constexpr std::uint8_t kLeavesEbxEdx = fe::kRestoreAll & ~(fe::kRestoreEbx | fe::kRestoreEdx);
constexpr std::uint8_t kLeavesEbxEcxEdx = fe::kRestoreAll & ~(fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx);

fe::Site makeSite(const char* name, u32 stackBytes, u32 frameBytes, std::uint8_t restores = fe::kRestoreAll)
{
    fe::Site site{};
    site.name = name;
    site.stackBytes = stackBytes;
    site.frameBytes = frameBytes;
    site.restores = restores;
    return site;
}

u32 sar(u32 value, unsigned bits)
{
    return u32(s32(value) >> (bits & 31));
}

bool positive(u32 value)
{
    return s32(value) > 0;
}

// ---------------------------------------------------------------------------
// The library's data
// ---------------------------------------------------------------------------

// The library's error report: its printf to the log, and where it is told it was called from.
constexpr u32 kLog = 0x401010;
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;

constexpr u32 kMemmove = 0x4ea4f0;
constexpr u32 kMemoryPool = 0x5643f4;
constexpr u32 kFillZero = 0x4e070c;

// The current window.
constexpr u32 kClipLeft = 0x565000, kClipTop = 0x565004, kClipRight = 0x565008, kClipBottom = 0x56500c;
constexpr u32 kWindowDepth = 0x565010;      // byte, bits a pixel
constexpr u32 kWindowType = 0x565011;       // byte, a shape type
constexpr u32 kWindowLocked = 0x565012;     // byte
constexpr u32 kWindowPixels = 0x565014;
constexpr u32 kWindowStride = 0x56501c;
constexpr u32 kRowOffsets = 0x565020, kColumnOffsets = 0x565024;
constexpr u32 kWindowPolled = 0x565028;

constexpr u32 kDepthOfType = 0x566d54;      // bytes, by a type's low 7 bits
constexpr u32 kInverseTable = 0x5650c0;     // a 15-bit colour's nearest index, or 0

// A shape: its first dword the type in the low byte and the distance to the next record
// above it, then words: width at +4, height at +6, and its pixels from +0x10.
constexpr u32 kShapePixels = 0x10;
constexpr u32 kShapeWidth = 2, kShapeHeight = 4;   // dwords whose upper half is it

u32 shapeDepth(Guest& guest, u32 shape)
{
    return guest.byte(kDepthOfType + (guest.dword(shape) & 0x7f));
}

u32 roundedDepth(u32 depth)
{
    return (depth + 1) & ~u32(1);
}

/* Error as C++, where it is told it was called from first: its format and words as the code pushes
 * them.  The game ends in it (fe::Terminated): what a function would go on to hand back after it is
 * no result. */
void reportError(Guest& guest, u32 file, u32 function, u32 line, const fe::Arguments& arguments)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    guest::Words words({});
    for (std::size_t i = 1; i < arguments.stackCount; ++i)
        words.word[words.count++] = arguments.stack[i];
    fe::direct<kLog, ea::error>(guest.app(), guest.ts(), arguments.stack[0], words);
}

/* rep movsd and rep movsb, as Watcom inlines memcpy. */
void copyRep(Guest& guest, u32 to, u32 from, u32 bytes)
{
    u32 dwords = bytes >> 2;
    while (dwords--)
    {
        guest.setDword(to, guest.dword(from));
        to += 4;
        from += 4;
    }
    u32 rest = bytes & 3;
    while (rest--)
        guest.setByte(to++, guest.byte(from++));
}

// ---------------------------------------------------------------------------
// The blends of an ARGB over a pixel, which two blitters and the spans share
// ---------------------------------------------------------------------------

/* What a blend leaves in the registers its code uses: the callers go on with them. */
struct Scratch
{
    u32 eax, edx;
};

/* A 1555 pixel under an ARGB of less than full alpha: the ARGB with what shows through
 * added. */
u32 blendOver1555(u32 pixel, u32 argb)
{
    const u32 spread = ((pixel & 0x7c00) << 17) + ((pixel & 0x3e0) << 9) + (pixel & 0x1f);
    const u32 inverse = (argb >> 24) ^ 0xff;
    const std::uint64_t product = std::uint64_t(spread) * inverse;
    const u32 low = u32(product), high = u32(product >> 32);
    return argb + (high << 16) + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
}

/* The same over a 565 pixel. */
u32 blendOver565(u32 pixel, u32 argb)
{
    const u32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
    const u32 inverse = (~argb) >> 24;
    const std::uint64_t product = std::uint64_t(spread) * inverse;
    const u32 low = u32(product), high = u32(product >> 32);
    return argb + (high << 16) + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
}

u32 packTo1555(u32 argb, Scratch& left)
{
    left.eax = (argb >> 3) & 0x1f;
    left.edx = (argb & 0xf800) >> 6;
    return ((argb & 0xf80000) >> 9) + left.eax + left.edx;
}

u32 packTo565(u32 argb, Scratch& left)
{
    left.eax = (argb >> 3) & 0x1f;
    left.edx = (argb & 0xfc00) >> 5;
    return ((argb & 0xf80000) >> 8) + left.eax + left.edx;
}

constexpr u32 kOpaqueFrom = 0xfc000000;     // this much alpha and more replaces the pixel
constexpr u32 kVisibleFrom = 0x10000000;    // less than this much leaves it

}

// ---------------------------------------------------------------------------
// Small callbacks
// ---------------------------------------------------------------------------

/* sub_4fb820: qfetch's test: whether the element's key at +4 is edx. */
nfs::u32 frontend::watb::keyMatches(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    return argEdx == guest.dword(argEax + 4) ? 1 : 0;
}

nfs::u32 frontend::watbKeyMatches(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4fb820", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::keyMatches(app, ts, args.eax, args.edx); });
}

/* sub_4fb82b: the end of sub_4fb820, which sub_4fb850 jumps to: no match. */
nfs::u32 frontend::watb::noMatch(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::watbNoMatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4fb82b", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::noMatch(app, ts); });
}

/* sub_4fb830: an element's sort key: its byte at +0x11 over the upper bits of its dword at +4. */
nfs::u32 frontend::watb::sortKey(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const u32 record = argEax;
    return (u32(guest.byte(record + 0x11)) << 24) | (sar(guest.dword(record + 4), 5) & 0xffffff);
}

nfs::u32 frontend::watbSortKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4fb830", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::sortKey(app, ts, args.eax); });
}

/* sub_4feffc: a hook of the library's that answers 0. */
nfs::u32 frontend::watb::returnZero(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::watbReturnZero(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4feffc", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::returnZero(app, ts); });
}

/* sub_4ff620, sub_50d6d0, sub_50dbf0: hooks that do nothing. */
void frontend::watb::nothing4ff620(App*, ThreadState&)
{
}

nfs::u32 frontend::watbNothing4ff620(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watb::nothing4ff620(app, ts);
    ts.sp += 4;
    return args.eax;
}

void frontend::watb::nothing50d6d0(App*, ThreadState&)
{
}

nfs::u32 frontend::watbNothing50d6d0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watb::nothing50d6d0(app, ts);
    ts.sp += 4;
    return args.eax;
}

void frontend::watb::nothing50dbf0(App*, ThreadState&)
{
}

nfs::u32 frontend::watbNothing50dbf0(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    watb::nothing50dbf0(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* sub_4ff0f8, sub_4ff100: __allocPOSIXHandle's and __freePOSIXHandle's jumps, which
 * 0x567770 and 0x567774 point at: the handle (eax, the one register either reads) on to the
 * function, its answer back.  They have no typed core yet: by address.  Their thunks pop the
 * return address the function would have returned through. */
nfs::u32 frontend::watb::allocHandleJump(App* app, ThreadState& ts, u32 handle)
{
    return guest::pointerCall(app, ts, 0x51e304, { handle }).value_or(0);
}

nfs::u32 frontend::watbAllocHandleJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::allocHandleJump(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::freeHandleJump(App* app, ThreadState& ts, u32 handle)
{
    return guest::pointerCall(app, ts, 0x51e45c, { handle }).value_or(0);
}

nfs::u32 frontend::watbFreeHandleJump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::freeHandleJump(app, ts, args.eax);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

// ---------------------------------------------------------------------------
// The runtime's start-up hooks
// ---------------------------------------------------------------------------

/* sub_4ff570: printf's and scanf's floats, [__EFG_printf] and [__EFG_scanf].  No result: the
 * game's code leaves eax as it came, which its thunk does. */
nfs::u32 frontend::watb::initFloatFormatters(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x56ac64, 0x51f140);
    guest.setDword(0x56ac68, 0x51f254);
    return 0;
}

nfs::u32 frontend::watbInitFloatFormatters(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4ff570", 0, 0);
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        watb::initFloatFormatters(app, ts);
        return previous;
    });
}

namespace
{
constexpr u32 k8087 = 0x5644c0, kReal87 = 0x5644c1, kNo87Found = 0x567834;
}

/* sub_4ff59c: where there is a real x87, its state's save and restore for the signal
 * handlers.  Its call of sub_51f272, the test for the Pentium's FDIV bug, is gone in the
 * port, which leaves the word it was handed in eax. */
nfs::u32 frontend::watb::initChipBugHooks(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (guest.byte(kReal87) != 0)
    {
        guest.setDword(0x56acf0, 0x4ff590);
        guest.setDword(0x56acf4, 0x4ff598);
    }
    return guest.word(0x56acf8);
}

nfs::u32 frontend::watbInitChipBugHooks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4ff59c", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::initChipBugHooks(app, ts); });
}

/* sub_4ff5dc: an x87 looked for, by what fninit leaves in the control word: the port's x87 starts
 * with 0x7f (nfs::FloatEnv::init), whose high byte is no x87's.  The fninit itself, which empties the x87's
 * stack, its thunk does. */
nfs::u32 frontend::watb::init8087(App* app, ThreadState& ts, u32 argEax)
{
    constexpr u32 kFninitControl = 0x7f;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const std::uint8_t found = guest.byte(k8087);
    if (found != 0)
        return (argEax & 0xffff00ff) | (u32(found) << 8);
    guest.setByte(kReal87, 0);
    const u32 word = kFninitControl;
    const std::uint8_t high = std::uint8_t(word >> 8);
    u32 result = (word & 0xffffff00) | high;
    std::uint8_t present = 0;
    if (high == 3)
    {
        result = guest::direct<0x4ff59c, initChipBugHooks>(app, ts);
        present = std::uint8_t(result);
    }
    if (guest.byte(kNo87Found) == 0)
    {
        guest.setByte(k8087, present);
        guest.setByte(kReal87, present);
    }
    return result;
}

nfs::u32 frontend::watbInit8087(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4ff5dc", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        if (guest::byte(app, k8087) == 0)
            ts.fenv.init();
        return watb::init8087(app, ts, args.eax);
    });
}

// ---------------------------------------------------------------------------
// A shape's palette and a pixel of the window
// ---------------------------------------------------------------------------

/* sub_4fd1c0, getshapeclut: the palette among the records that follow a shape, or 0. */
nfs::u32 frontend::watb::shapeClut(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    u32 record = argEax;
    if (record == 0)
        return 0;
    if (shapeDepth(guest, record) == 0)
        reportError(guest, 0x54e04c, 0x54e05c, 0x46, args({}, { 0x54e06c, guest.dword(record) & 0xff }));
    while (record != 0)
    {
        const u32 type = guest.dword(record) & 0xff;
        if (type == 0x22 || type == 0x24 || type == 0x23 || type == 0x29 || type == 0x2a || type == 0x2c
            || type == 0x2d || type == 0x2e)
            return record;
        const u32 next = sar(guest.dword(record), 8);
        record = next != 0 ? record + next : 0;
    }
    return 0;
}

nfs::u32 frontend::watbShapeClut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4fd1c0", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::shapeClut(app, ts, args.eax); });
}

/* sub_4fd540: a filled rectangle, drawn by movdfl as a shape of the window's type with
 * no pixels: the colour goes in 0x9f4fac, repeated to fill a dword, for the fillers in
 * the table at 0x5676b4.  The shape's header is built in the frame; what movdfl would take
 * for its pixels lies above it there, where the code saves its registers, and the fillers
 * read none of it. */
nfs::u32 frontend::watb::drawFilledRect(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                          u32 colourWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    const u32 frame = guest.frame();
    const u32 x = argEax, y = argEdx;
    u32 colour = colourWord;
    const std::uint8_t depth = guest.byte(kWindowDepth);
    if (depth == 4)
    {
        colour &= 0xf;
        colour |= colour << 4;
    }
    else if (depth == 8)
    {
        colour &= 0xff;
        colour |= colour << 8;
        colour |= colour << 16;
    }
    else if (depth == 0xf || depth == 0x10)
    {
        colour &= 0xffff;
        colour |= colour << 16;
    }
    guest.setDword(0x9f4fac, colour);
    guest.setDword(0x5676cc, 0);
    if (guest.byte(kWindowDepth) == 0x10 && guest.dword(0x5676b0) != 0)
        guest.setDword(0x5676cc, 0x4fd4e0);
    guest.setWord(frame + 4, std::uint16_t(argEbx));
    guest.setWord(frame + 6, std::uint16_t(argEcx));
    guest.setDword(frame, guest.byte(kWindowType));
    return fe::direct<0x50b0b0, moveShape>(app, ts, frame, x, y, u32(0x5676b4));
}

nfs::u32 frontend::watbDrawFilledRect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_4fd540", 4, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::drawFilledRect(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4));
    });
}

/* sub_503bc0, readpixel: the pixel at eax, edx of the current window; 0 outside it. */
nfs::u32 frontend::watb::readPixel(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    const u32 x = argEax, y = argEdx;
    if (s32(x) < s32(guest.dword(kClipLeft)) || s32(x) >= s32(guest.dword(kClipRight))
        || s32(y) < s32(guest.dword(kClipTop)) || s32(y) >= s32(guest.dword(kClipBottom)))
        return 0;
    const u32 row = guest.dword(guest.dword(kRowOffsets) + y * 4);
    const u32 column = guest.dword(guest.dword(kColumnOffsets) + x * 4);
    const u32 pixel = guest.dword(kWindowPixels) + row + column;
    const std::uint8_t depth = guest.byte(kWindowDepth);
    switch (depth)
    {
    case 0xf:
    case 0x10:
        return guest.word(pixel);
    case 0x18:
        return guest.dword(pixel - 1) >> 8;
    case 0x20:
        return guest.dword(pixel);
    case 8:
        return guest.byte(pixel);
    case 4:
        break;
    default:
        reportError(guest, 0x54ebb8, 0x54ebc8, 0x89, args({}, { 0x54ebd4, guest.byte(kWindowDepth) }));
        break;
    }
    return (x & 1) != 0 ? (u32(guest.byte(pixel)) >> 4) & 0xf : u32(guest.byte(pixel)) & 0xf;
}

nfs::u32 frontend::watbReadPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_503bc0", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::readPixel(app, ts, args.eax, args.edx); });
}

/* sub_508b80, initvideowindow: a window's record of 0x38 bytes and its two row tables. */
nfs::u32 frontend::watb::initVideoWindow(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx,
                                           u32 argEcx, u32 depthWord, u32 pixelsWord, u32 tagWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x14);
    const u32 frame = guest.frame();
    const u32 window = argEax, width = argEdx, height = argEbx, stride = argEcx;
    u32 depth = depthWord;
    if (depth == 0)
        depth = guest.dword(0x56438c);
    if (depth == 0)
        depth = 8;
    const u32 bytesStep = roundedDepth(depth);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), window, 0x38);
    guest.setDword(window, 0x57494e44);   // 'WIND'
    guest.setByte(window + 0x1e, 0xff);
    guest.setDword(window + 4, width);
    guest.setDword(window + 8, height);
    guest.setDword(window + 0x14, width);
    guest.setDword(window + 0x18, height);
    guest.setDword(window + 0x28, stride);
    guest.setByte(window + 0x1c, std::uint8_t(depth));
    guest.setByte(window + 0x1d, std::uint8_t(guest::direct<0x5098d0, shapeTypeForDepth>(app, ts, depth)));
    guest.setDword(window + 0x20, pixelsWord);
    guest.setDword(window + 0x34, tagWord);
    const u32 rows = guest::direct<0x509a30, makeRow>(app, ts, u32(0), stride, u32(0), height);
    guest.setDword(window + 0x2c, rows);
    const u32 columns = guest::direct<0x509a30, makeRow>(app, ts, u32(0), bytesStep, u32(3), width);
    guest.setDword(window + 0x30, columns);
    if (rows != 0 && columns != 0)
        return columns;
    reportError(guest, 0x54f53c, 0x54f54c, 0x29, args({}, { 0x54f55c, width, height }));
    return 0;
}

nfs::u32 frontend::watbInitVideoWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_508b80", 12, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::initVideoWindow(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4),
                                     guest::at(app, ts.sp + 8), guest::at(app, ts.sp + 12));
    });
}

// ---------------------------------------------------------------------------
// The row tables: a block of 0x568380 cut into runs -- a count, the uses, the offsets
// ---------------------------------------------------------------------------

namespace
{
constexpr u32 kRowSpace = 0x568380, kRowSpaceWords = 0x56837c, kRowSpaceEnd = 0xa0a810;
}

/* sub_5098d0: the shape type of a window of this many bits a pixel, or 0. */
nfs::u32 frontend::watb::shapeTypeForDepth(App* app, ThreadState& ts, u32 argEax)
{
    switch (argEax)
    {
    case 1: return 0x79;
    case 4: return 0x7a;
    case 8: return 0x7b;
    case 0xf: return 0x7e;
    case 0x10: return 0x78;
    case 0x18: return 0x7f;
    case 0x20: return 0x7d;
    default: return 0;
    }
}

nfs::u32 frontend::watbShapeTypeForDepth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5098d0", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::shapeTypeForDepth(app, ts, args.eax); });
}

/* sub_509940, initrow: the space made if it is not there, one free run in it. */
nfs::u32 frontend::watb::initRowTable(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    const u32 words = guest.dword(kRowSpaceWords);
    if (s32(words) < 0xcc)
        reportError(guest, 0x54f718, 0x54f724, 0x14, args({}, { 0x54f72c, words }));
    if (guest.dword(kRowSpace) == 0)
    {
        guest.setDword(kErrorFile, 0x54f718);
        guest.setDword(kErrorFunction, 0x54f724);
        guest.setDword(kErrorLine, 0x15);
        guest.setDword(kRowSpace, *fe::direct<0x4e1620, ea::reserved>(app, ts, u32(0x54f770), guest.dword(kRowSpaceWords) << 2,
                                                                     guest.dword(kMemoryPool)));
    }
    const u32 space = guest.dword(kRowSpace);
    const u32 size = guest.dword(kRowSpaceWords);
    guest.setDword(space + 4, 0);
    guest.setDword(kRowSpaceEnd, space + (size << 2));
    guest.setDword(space, size - 2);
    return space;
}

nfs::u32 frontend::watbInitRowTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509940", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::initRowTable(app, ts); });
}

/* sub_509a00: the space given back.  No result: what purgememadr leaves, or with no space eax as
 * it came, which its thunk does. */
nfs::u32 frontend::watb::freeRowTable(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    const u32 space = guest.dword(kRowSpace);
    u32 left = 0;
    if (space != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, space);
    guest.setDword(kRowSpace, 0);
    return left;
}

nfs::u32 frontend::watbFreeRowTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509a00", 0, 0);
    const u32 previous = args.eax;
    const bool none = guest::at(app, kRowSpace) == 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 left = watb::freeRowTable(app, ts);
        return none ? previous : left;
    });
}

/* sub_509a30: a table of ecx offsets from eax, edx apart, each shifted right by ebx: an
 * equal one shared if there is one, else a free run taken; 0 if none is long enough. */
nfs::u32 frontend::watb::makeRow(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx)
{
    Guest guest(app, ts);
    u32 offset = argEax;
    const u32 step = argEdx;
    u32 count = argEcx;
    const unsigned shift = unsigned(argEbx & 0xff);
    u32 run = guest.dword(kRowSpace);
    const u32 end = guest.dword(kRowSpaceEnd);
    u32 freeRun = 0;
    if (run < end)
    {
        const u32 second = sar(step, shift) + offset;
        do
        {
            const u32 size = guest.dword(run);
            if (s32(size) >= s32(count))
            {
                if (guest.dword(run + 4) == 0)
                {
                    if (freeRun == 0)
                        freeRun = run;
                }
                else if (offset == guest.dword(run + 8) && second == guest.dword(run + 0xc))
                {
                    const u32 uses = guest.dword(run + 4) + 1;
                    guest.setDword(run + 4, uses);
                    return run + 8;
                }
            }
            run += (size << 2) + 8;
        } while (run < guest.dword(kRowSpaceEnd));
    }
    if (freeRun == 0)
        return 0;
    const u32 size = guest.dword(freeRun);
    if (s32(size) >= s32(count + 8))
    {
        guest.setDword(freeRun + count * 4 + 0xc, 0);
        guest.setDword(freeRun + count * 4 + 8, size - count - 2);
    }
    else
        count = size;
    guest.setDword(freeRun, count);
    guest.setDword(freeRun + 4, 1);
    const u32 table = freeRun + 8;
    if (positive(count))
    {
        for (u32 i = 0; i != count; ++i)
        {
            guest.setDword(table + i * 4, sar(offset, shift));
            offset += step;
        }
    }
    return table;
}

nfs::u32 frontend::watbMakeRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509a30", 0, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::makeRow(app, ts, args.eax, args.edx, args.ebx, args.ecx); });
}

/* sub_509b10, removerow: a use of a table dropped; the last, its run joined to free
 * neighbours. */
nfs::u32 frontend::watb::removeRow(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20);
    const u32 entry = argEax;
    const u32 run = entry - 8;
    const u32 uses = guest.dword(run + 4) - 1;
    guest.setDword(run + 4, uses);
    if (uses != 0)
        return entry;
    u32 at = guest.dword(kRowSpace);
    u32 previous = 0;
    while (at < run)
    {
        previous = at;
        at += (guest.dword(at) + 2) << 2;
    }
    if (at != run)
        reportError(guest, 0x54f718, 0x54f778, 0x71, args({}, { 0x54f784 }));
    u32 left = at + ((guest.dword(at) + 2) << 2);
    if (left < guest.dword(kRowSpaceEnd) && guest.dword(left + 4) == 0)
    {
        left = guest.dword(left) + 2;
        guest.setDword(at, guest.dword(at) + left);
    }
    if (previous != 0 && guest.dword(previous + 4) == 0)
    {
        left = guest.dword(at) + 2;
        guest.setDword(previous, guest.dword(previous) + left);
    }
    return left;
}

nfs::u32 frontend::watbRemoveRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509b10", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::removeRow(app, ts, args.eax); });
}

/* sub_509c00: the memory manager's block that eax lies in, by its 16 heaps: the block's
 * pixels, or 0. */
nfs::u32 frontend::watb::findBlockHeader(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    const u32 address = argEax;
    u32 data = argEdx;
    for (u32 heap = 0; s32(heap) <= 0xf; ++heap)
    {
        u32 block = guest.dword((heap & 0xf) * 4 + 0x9ef5c0);
        if (block == 0)
            continue;
        block = guest.dword(block + 8);
        for (;;)
        {
            block = guest.dword(block + 8);
            if (block == 0)
                break;
            data = block + 0x10;
            if (address >= data && address < guest.dword(block + 4) + data)
                break;
        }
        if ((guest.byte(block + 3) & 0xc0) == 0)
            return data;
    }
    return 0;
}

nfs::u32 frontend::watbFindBlockHeader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509c00", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::findBlockHeader(app, ts, args.eax, args.edx); });
}

// ---------------------------------------------------------------------------
// A span of pixels from one format to another: eax from, edx to, ebx how many.  These
// are what the tables at 0x568394 and 0x568494 hold, by the two types' indices.
// ---------------------------------------------------------------------------

/* sub_509c50, sub_509c60, sub_509eb0, sub_50a510: the same format: a copy, its length
 * in bytes from pixels of 4, 8, 16 and 32 bits.  The game's code jumps to memmove, which returns
 * for them: here memmove is called, and their thunks pop the return address.  What memmove leaves
 * in eax is no result -- these are void, and their callers through the tables read none -- so they
 * hand back 0.  No throw: the game stopped in it leaves ts.terminate set. */
nfs::u32 frontend::watb::copyNibbles(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    guest::direct<kMemmove, ea::moveMemory>(app, ts, to, from, sar(count + 1, 1));
    return 0;
}

nfs::u32 frontend::watbCopyNibbles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::copyNibbles(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::copyBytes(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    guest::direct<kMemmove, ea::moveMemory>(app, ts, to, from, count);
    return 0;
}

nfs::u32 frontend::watbCopyBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::copyBytes(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::copyWords(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    guest::direct<kMemmove, ea::moveMemory>(app, ts, to, from, count + count);
    return 0;
}

nfs::u32 frontend::watbCopyWords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::copyWords(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::copyDwords(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    guest::direct<kMemmove, ea::moveMemory>(app, ts, to, from, count << 2);
    return 0;
}

nfs::u32 frontend::watbCopyDwords(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::copyDwords(app, ts, args.eax, args.edx, args.ebx);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_50a1c0: 24 bits, memmove called; no result, as the four above. */
nfs::u32 frontend::watb::copyTriples(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    fe::direct<kMemmove, ea::moveMemory>(app, ts, argEdx, argEax, argEbx * 3);
    return 0;
}

nfs::u32 frontend::watbCopyTriples(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a1c0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::copyTriples(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509c70: bytes into nibbles, the even pixel in the low one. */
nfs::u32 frontend::watb::bytesToNibbles(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    if (!positive(count))
        return 0;
    u32 last = 0;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 at = to + sar(i, 1);
        const u32 value = guest.byte(from + i);
        last = (i & 1) != 0 ? (guest.byte(at) & 0xf) | (value << 4) : (guest.byte(at) & 0xf0) | (value & 0xf);
        last &= 0xff;
        guest.setByte(at, std::uint8_t(last));
    }
    return count;
}

nfs::u32 frontend::watbBytesToNibbles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509c70", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::bytesToNibbles(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509cd0: nibbles into bytes. */
nfs::u32 frontend::watb::nibblesToBytes(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    if (!positive(count))
        return 0;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 packed = guest.byte(from + sar(i, 1));
        guest.setByte(to + i, std::uint8_t((i & 1) != 0 ? packed >> 4 : packed & 0xf));
    }
    return count;
}

nfs::u32 frontend::watbNibblesToBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509cd0", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::nibblesToBytes(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509d20: 565 into palette indices: by the inverse table where there is one, else
 * the nearest colour one at a time. */
nfs::u32 frontend::watb::convert565ToIndex(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x20);
    u32 from = argEax;
    const u32 to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax;
    const u32 table = guest.dword(kInverseTable);
    if (!positive(count))
        return result;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        const u32 red = (pixel >> 11) & 0x1f, green = (pixel >> 5) & 0x3f, blue = pixel & 0x1f;
        from += 2;
        if (table == 0)
        {
            result = fe::direct<0x4ef760, frontend::eab::colourPack>(app, ts, 0xff000000 | (red << 19) | (green << 10) | (blue << 3));
            guest.setByte(to + i, std::uint8_t(result));
        }
        else
        {
            const u32 index = (red << 10) | ((green >> 1) << 5) | blue;
            const std::uint8_t found = guest.byte(guest.dword(kInverseTable) + index);
            result = (index & 0xffffff00) | found;
            guest.setByte(to + i, found);
        }
    }
    return result;
}

nfs::u32 frontend::watb565ToIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509d20", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert565ToIndex(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509df0: 8888 into 1555, nothing under a quarter of alpha, the top bit set on the rest. */
nfs::u32 frontend::watb::convert8888To1555Keyed(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax, leftEbx = count, leftEdx = argEdx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.dword(from);
        leftEbx = pixel;
        result = 0;
        if (pixel >= 0x40000000)
        {
            leftEdx = (pixel & 0xf800) >> 6;
            leftEbx = ((pixel & 0xf80000) >> 9) + ((pixel >> 3) & 0x1f) + leftEdx + 0x8000;
            result = leftEbx;
        }
        guest.setWord(to, std::uint16_t(result));
        to += 2;
        from += 4;
    }
    return result;
}

nfs::u32 frontend::watb8888To1555Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509df0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::convert8888To1555Keyed(app, ts, args.eax, args.edx, args.ebx);
    });
}

/* sub_509e50: the same with every pixel but 0 opaque. */
nfs::u32 frontend::watb::convert8888To1555(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax, leftEbx = count, leftEdx = argEdx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.dword(from);
        leftEbx = pixel;
        if (pixel != 0)
        {
            result = (pixel >> 3) & 0x1f;
            leftEdx = (pixel & 0xf800) >> 6;
            leftEbx = ((pixel & 0xf80000) >> 9) + result + leftEdx + 0x8000;
        }
        guest.setWord(to, std::uint16_t(leftEbx));
        to += 2;
        from += 4;
    }
    return result;
}

nfs::u32 frontend::watb8888To1555(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509e50", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8888To1555(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509ef0: 8888 into 565, nothing under a quarter of alpha, and what would come out 0 as 1. */
nfs::u32 frontend::watb::convert8888To565Keyed(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax, leftEbx = count, leftEdx = argEdx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.dword(from);
        leftEbx = pixel;
        result = 0;
        if (pixel >= 0x40000000)
        {
            leftEdx = (pixel & 0xfc00) >> 5;
            leftEbx = ((pixel & 0xf80000) >> 8) + ((pixel >> 3) & 0x1f) + leftEdx;
            result = std::uint16_t(leftEbx) != 0 ? leftEbx : 1;
        }
        guest.setWord(to, std::uint16_t(result));
        to += 2;
        from += 4;
    }
    return result;
}

nfs::u32 frontend::watb8888To565Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509ef0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::convert8888To565Keyed(app, ts, args.eax, args.edx, args.ebx);
    });
}

/* sub_509f50: 8888 into 565. */
nfs::u32 frontend::watb::convert8888To565(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax, leftEbx = count, leftEdx = argEdx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.dword(from);
        result = (pixel >> 3) & 0x1f;
        leftEdx = (pixel & 0xfc00) >> 5;
        leftEbx = ((pixel & 0xf80000) >> 8) + result + leftEdx;
        guest.setWord(to, std::uint16_t(leftEbx));
        to += 2;
        from += 4;
    }
    return result;
}

nfs::u32 frontend::watb8888To565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509f50", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8888To565(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_509fb0: 8888 into 4444. */
nfs::u32 frontend::watb::convert8888To4444(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 leftEbx = count;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 alpha = u32(guest.byte(from + 3)) >> 4, red = u32(guest.byte(from + 2)) >> 4;
        const u32 green = u32(guest.byte(from + 1)) >> 4, blue = u32(guest.byte(from)) >> 4;
        leftEbx = (red << 8) | (alpha << 12) | (green << 4);
        guest.setWord(to, std::uint16_t(blue | leftEbx));
        from += 4;
        to += 2;
    }
    return from;
}

nfs::u32 frontend::watb8888To4444(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509fb0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8888To4444(app, ts, args.eax, args.edx, args.ebx); });
}

namespace
{
constexpr u32 kKeyColour = 0x568388;        // the 16-bit colour that is no pixel
}

/* sub_50a020: 1555 into 4444, opaque, the key colour nothing. */
nfs::u32 frontend::watb::convert1555To4444(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax;
    if (!positive(count))
        return result;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        if (pixel == guest.dword(kKeyColour))
        {
            result = pixel;
            guest.setWord(to, 0);
        }
        else
        {
            const u32 blue = (pixel >> 1) & 0xf;
            result = (((pixel >> 12) & 0xf) << 8) | 0xf000 | (((pixel >> 7) & 0xf) << 4);
            guest.setWord(to, std::uint16_t(blue | result));
        }
        from += 2;
        to += 2;
    }
    return result;
}

nfs::u32 frontend::watb1555To4444(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a020", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert1555To4444(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a0a0: 8888's colour into 565 in the low half of a dword, the alpha kept. */
nfs::u32 frontend::watb::convert8888To8565(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax, leftEbx = count, leftEdx = argEdx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.dword(from);
        from += 4;
        result = (pixel >> 3) & 0x1f;
        leftEdx = (pixel & 0xfc00) >> 5;
        leftEbx = ((pixel & 0xf80000) >> 8) + result + leftEdx;
        guest.setDword(to, (pixel & 0xff000000) | leftEbx);
        to += 4;
    }
    return result;
}

nfs::u32 frontend::watb8888To8565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a0a0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8888To8565(app, ts, args.eax, args.edx, args.ebx); });
}

namespace
{
constexpr u32 kCurrentClut = 0x568390;
}

/* sub_50a100: palette indices into 888, by the palette at 0x568390. */
nfs::u32 frontend::watb::indexTo888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    const u32 clut = guest.dword(kCurrentClut);
    u32 result = argEax;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 scaled = u32(guest.byte(from)) << 2;
        const u32 colour = scaled + clut;
        const std::uint8_t two = guest.byte(colour + 2);
        guest.setByte(to + 2, two);
        const std::uint8_t one = guest.byte(colour + 1);
        ++from;
        guest.setByte(to + 1, one);
        const std::uint8_t zero = guest.byte(colour);
        guest.setByte(to, zero);
        to += 3;
        result = (colour & 0xffffff00) | zero;
    }
    return result;
}

nfs::u32 frontend::watbIndexTo888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a100", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::indexTo888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a150: 565 into 888. */
nfs::u32 frontend::watb::convert565To888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax;
    if (!positive(count))
        return result;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        const u32 red = ((pixel >> 11) & 0x1f) << 3;
        guest.setByte(to, std::uint8_t((pixel & 0x1f) << 3));
        guest.setByte(to + 1, std::uint8_t(((pixel >> 5) & 0x3f) << 2));
        guest.setByte(to + 2, std::uint8_t(red));
        result = red;
        from += 2;
        to += 3;
    }
    return result;
}

nfs::u32 frontend::watb565To888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a150", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert565To888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a1e0: 8888 into 888. */
nfs::u32 frontend::watb::convert8888To888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const std::uint8_t two = guest.byte(from + 2), one = guest.byte(from + 1), zero = guest.byte(from);
        guest.setByte(to, zero);
        guest.setByte(to + 1, one);
        guest.setByte(to + 2, two);
        from += 4;
        to += 3;
    }
    return from;
}

nfs::u32 frontend::watb8888To888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a1e0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8888To888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a240: 1555 into 8888, opaque, the key colour nothing. */
nfs::u32 frontend::watb::convert1555To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax;
    if (!positive(count))
        return result;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        if (pixel == guest.dword(kKeyColour))
        {
            result = pixel;
            guest.setDword(to, 0);
        }
        else
        {
            result = (pixel & 0x1f) << 3;
            guest.setDword(to, (((pixel >> 5) & 0x1f) << 11) | (((pixel >> 10) & 0x1f) << 19) | 0xff000000 | result);
        }
        from += 2;
        to += 4;
    }
    return result;
}

nfs::u32 frontend::watb1555To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a240", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert1555To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a2b0: 565 into 8888, opaque, the key colour nothing. */
nfs::u32 frontend::watb::convert565To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 result = argEax;
    if (!positive(count))
        return result;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        if (pixel == guest.dword(kKeyColour))
        {
            result = pixel;
            guest.setDword(to, 0);
        }
        else
        {
            result = (((pixel >> 11) & 0x1f) << 19) | 0xff000000 | (((pixel >> 5) & 0x3f) << 10);
            guest.setDword(to, ((pixel & 0x1f) << 3) | result);
        }
        from += 2;
        to += 4;
    }
    return result;
}

nfs::u32 frontend::watb565To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a2b0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert565To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a330: 4444 into 8888, each nibble doubled. */
nfs::u32 frontend::watb::convert4444To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 count = argEbx;
    u32 from = argEax, to = argEdx;
    if (!positive(count))
        return argEax;
    u32 red = 0;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 pixel = guest.word(from);
        const u32 a = (pixel >> 12) & 0xf, r = (pixel >> 8) & 0xf, g = (pixel >> 4) & 0xf, b = pixel & 0xf;
        red = (r | (r << 4)) << 16;
        guest.setDword(to, ((g | (g << 4)) << 8) | ((a | (a << 4)) << 24) | red | b | (b << 4));
        from += 2;
        to += 4;
    }
    return count;
}

nfs::u32 frontend::watb4444To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a330", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert4444To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a3c0: a dword with a 1555 colour low and the alpha on top, into 8888. */
nfs::u32 frontend::watb::convert8555To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 leftEdx = count;
    if (!positive(count))
    {
        return from;
    }
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 alpha = guest.byte(from + 3), pixel = guest.word(from);
        leftEdx = (((pixel >> 5) & 0x1f) << 11) | (((pixel >> 10) & 0x1f) << 19) | (alpha << 24) | ((pixel & 0x1f) * 8);
        guest.setDword(to, leftEdx);
        from += 4;
        to += 4;
    }
    return from;
}

nfs::u32 frontend::watb8555To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a3c0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8555To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a440: a dword with a 565 colour low and the alpha on top, into 8888. */
nfs::u32 frontend::watb::convert8565To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 leftEdx = count;
    if (!positive(count))
    {
        return from;
    }
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        const u32 alpha = guest.byte(from + 3), pixel = guest.word(from);
        leftEdx = (((pixel >> 11) & 0x1f) << 19) | (alpha << 24) | (((pixel >> 5) & 0x3f) << 10) | ((pixel & 0x1f) * 8);
        guest.setDword(to, leftEdx);
        from += 4;
        to += 4;
    }
    return from;
}

nfs::u32 frontend::watb8565To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a440", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert8565To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50a4c0: 888 into 8888, opaque. */
nfs::u32 frontend::watb::convert888To8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    const u32 count = argEbx;
    u32 leftEbx = count;
    for (u32 i = 0; s32(i) < s32(count); ++i)
    {
        leftEbx = (u32(guest.byte(from + 1)) << 8) | (u32(guest.byte(from + 2)) << 16) | 0xff000000
            | guest.byte(from);
        from += 3;
        guest.setDword(to, leftEbx);
        to += 4;
    }
    return from;
}

nfs::u32 frontend::watb888To8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a4c0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::convert888To8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50afc0: palette indices into 16 bits by the table at [0x9f09b0], index 0xff no
 * pixel; a dword at a time on a dword's boundary. */
nfs::u32 frontend::watb::spanTo16Keyed(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    s32 left = s32(argEbx);
    u32 last = argEbx;
    const u32 table = guest.dword(0x9f09b0);
    const auto colourOf = [&](std::uint8_t index) { return guest.dword(table + u32(index) * 4); };
    // A misaligned start goes a pixel at a time, as far as the first 0xff.
    if ((to & 3) != 0)
    {
        --left;
        while (left >= 0)
        {
            const std::uint8_t index = guest.byte(from);
            if (index == 0xff)
                break;
            last = colourOf(index);
            guest.setWord(to, std::uint16_t(last));
            ++from;
            to += 2;
            --left;
        }
        ++left;
    }
    left -= 2;
    while (left >= 0)
    {
        const std::uint8_t first = guest.byte(from), second = guest.byte(from + 1);
        if (first == 0xff)
        {
            if (second != 0xff)
            {
                last = colourOf(second);
                guest.setWord(to + 2, std::uint16_t(last));
            }
        }
        else
        {
            last = colourOf(first);
            if (second == 0xff)
                guest.setWord(to, std::uint16_t(last));
            else
            {
                last = (last & 0xffff) | (colourOf(second) << 16);
                guest.setDword(to, last);
            }
        }
        from += 2;
        to += 4;
        left -= 2;
    }
    left += 2;
    --left;
    while (left >= 0)
    {
        const std::uint8_t index = guest.byte(from);
        if (index == 0xff)
            break;
        last = colourOf(index);
        guest.setWord(to, std::uint16_t(last));
        ++from;
        to += 2;
        --left;
    }
    return from;
}

nfs::u32 frontend::watbSpanTo16Keyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50afc0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::spanTo16Keyed(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50b080: a span through two converters, by a buffer in the frame: to the middle
 * format by [0xa0b018], then to the window's by [0xa0b014]. */
nfs::u32 frontend::watb::spanTwoSteps(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1908);
    const u32 frame = guest.frame();
    const u32 to = argEdx, count = argEbx;
    fe::pointerCall(guest, guest.dword(0xa0b018), args({ argEax, frame, count }));
    return fe::pointerCall(guest, guest.dword(0xa0b014), args({ frame, to, count }));
}

nfs::u32 frontend::watbSpanTwoSteps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b080", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::spanTwoSteps(app, ts, args.eax, args.edx, args.ebx); });
}

// ---------------------------------------------------------------------------
// Palettes and conversion: win\convshp.c
// ---------------------------------------------------------------------------

namespace
{
constexpr u32 kSpanTablesKeyed = 0x568394, kSpanTablesPlain = 0x568494;  // by from * 8 + to
constexpr u32 kBetweenTypes = 0x568594;     // bytes: the type to go through
constexpr u32 kNeutralClut = 0xa0a814;      // the library's 256 colours
constexpr u32 kClutBuffer = 0xa0ac14;       // a 16-bit palette made here
constexpr u32 kExpand1555 = 0x50a240, kExpand565 = 0x50a2b0, kExpand8565 = 0x50a440, kExpand8555 = 0x50a3c0;
constexpr u32 kExpand8888To8565 = 0x50a0a0;
constexpr u32 kExpandTable24 = 0x51df60, kExpandTable18 = 0x51de60;
constexpr u32 k8888To4444 = 0x509fb0, kTo1555Keyed = 0x509df0, kTo1555 = 0x509e50, kTo565Keyed = 0x509ef0, kTo565 = 0x509f50;

/* A palette's colours through the converter the code picks among these of this file, as C++. */
void convertColours(App* app, ThreadState& ts, u32 converter, u32 from, u32 to, u32 count)
{
    switch (converter)
    {
    case kExpand1555: fe::direct<kExpand1555, frontend::watb::convert1555To8888>(app, ts, from, to, count); break;
    case kExpand565: fe::direct<kExpand565, frontend::watb::convert565To8888>(app, ts, from, to, count); break;
    case kExpand8565: fe::direct<kExpand8565, frontend::watb::convert8565To8888>(app, ts, from, to, count); break;
    case kExpand8555: fe::direct<kExpand8555, frontend::watb::convert8555To8888>(app, ts, from, to, count); break;
    case kExpand8888To8565:
        fe::direct<kExpand8888To8565, frontend::watb::convert8888To8565>(app, ts, from, to, count);
        break;
    case k8888To4444: fe::direct<k8888To4444, frontend::watb::convert8888To4444>(app, ts, from, to, count); break;
    case kTo1555Keyed: fe::direct<kTo1555Keyed, frontend::watb::convert8888To1555Keyed>(app, ts, from, to, count); break;
    case kTo1555: fe::direct<kTo1555, frontend::watb::convert8888To1555>(app, ts, from, to, count); break;
    case kTo565Keyed: fe::direct<kTo565Keyed, frontend::watb::convert8888To565Keyed>(app, ts, from, to, count); break;
    case kTo565: fe::direct<kTo565, frontend::watb::convert8888To565>(app, ts, from, to, count); break;
    default: break;
    }
}
}

/* The answers of sub_50a570's three ends, which it jumps to. */
nfs::u32 frontend::watb::depthIndex16(App*, ThreadState&)
{
    return 3;
}

nfs::u32 frontend::watbDepthIndex16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::depthIndex16(app, ts);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::depthIndex32(App*, ThreadState&)
{
    return 5;
}

nfs::u32 frontend::watbDepthIndex32(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::depthIndex32(app, ts);
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::depthIndex15(App*, ThreadState&)
{
    return 2;
}

nfs::u32 frontend::watbDepthIndex15(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::depthIndex15(app, ts);
    ts.sp += 4;
    return args.eax;
}

/* sub_50a570: a pixel format's index in the converters' tables, by its shape type; -1 for none. */
nfs::u32 frontend::watb::depthIndex(App* app, ThreadState& ts, u32 argEax)
{
    switch (argEax)
    {
    case 0x40: case 0x7a: return 0;
    case 0x41: case 0x7b: return 1;
    case 0x7e: return 2;
    case 0x78: return 3;
    case 0x43: case 0x7f: return 4;
    case 0x7d: return 5;
    case 0x6d: return 6;
    case 0x42: return 7;
    default: return 0xffffffff;
    }
}

nfs::u32 frontend::watbDepthIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a570", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::depthIndex(app, ts, args.eax); });
}

/* sub_50a5d0, getclutptr: a shape's palette as 8888, made at edx from whatever its
 * record holds; the neutral palette for a shape with none. */
nfs::u32 frontend::watb::clutPointer(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x210);
    const u32 frame = guest.frame();
    const u32 out = argEdx;
    const u32 record = guest::direct<0x4fd1c0, shapeClut>(app, ts, argEax);
    if (record == 0)
        return kNeutralClut;
    const u32 data = record + kShapePixels;
    if ((guest.dword(record) & 0xff) == 0x2a)
        return data;
    const u32 count = sar(guest.dword(record + kShapeWidth), 16);
    guest.setDword(out + 0x3fc, 0);
    const u32 type = guest.dword(record) & 0xff;
    switch (type)
    {
    case 0x24:
        fe::direct<kExpandTable24, frontend::menz::paletteFrom8Bits>(app, ts, count, data, out);
        break;
    case 0x29:
        convertColours(app, ts, kExpand565, data, out, count);
        break;
    case 0x2d:
        convertColours(app, ts, kExpand1555, data, out, count);
        break;
    case 0x2c:
        convertColours(app, ts, kExpand8565, data, out, count);
        break;
    case 0x2e:
        convertColours(app, ts, kExpand8555, data, out, count);
        break;
    case 0x22:
        fe::direct<kExpandTable18, frontend::menz::paletteFrom6Bits>(app, ts, count, data, out);
        break;
    case 0x23:
        fe::direct<0x5042c4, frontend::rendf::rowSwapRedBlue>(app, ts, data, frame, u32(0x100));
        convertColours(app, ts, kExpand1555, frame, out, u32(0x100));
        break;
    default:
        reportError(guest, 0x54f7a8, 0x54f7b8, 0x313, args({}, { 0x54f7c4, type }));
        break;
    }
    return out;
}

nfs::u32 frontend::watbClutPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a5d0", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::clutPointer(app, ts, args.eax, args.edx); });
}

/* sub_50a830, getclut16aptr: the same with the colours in 565, the alpha kept. */
nfs::u32 frontend::watb::clut16aPointer(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x410);
    const u32 frame = guest.frame();
    const u32 out = argEdx;
    const u32 record = guest::direct<0x4fd1c0, shapeClut>(app, ts, argEax);
    if (record == 0)
    {
        convertColours(app, ts, kExpand8888To8565, kNeutralClut, out, u32(0x100));
        return out;
    }
    const u32 data = record + kShapePixels;
    if ((guest.dword(record) & 0xff) == 0x2c)
        return data;
    const u32 count = sar(guest.dword(record + kShapeWidth), 16);
    guest.setDword(out + 0x3fc, 0);
    const u32 type = guest.dword(record) & 0xff;
    switch (type)
    {
    case 0x24:
        fe::direct<kExpandTable24, frontend::menz::paletteFrom8Bits>(app, ts, count, data, frame);
        break;
    case 0x29:
        convertColours(app, ts, kExpand565, data, frame, count);
        break;
    case 0x2d:
        convertColours(app, ts, kExpand1555, data, frame, count);
        break;
    case 0x2e:
        convertColours(app, ts, kExpand8555, data, frame, count);
        break;
    case 0x2a:
        convertColours(app, ts, kExpand8888To8565, data, out, count);
        return out;
    case 0x22:
        fe::direct<kExpandTable18, frontend::menz::paletteFrom6Bits>(app, ts, count, data, frame);
        break;
    case 0x23:
        fe::direct<0x5042c4, frontend::rendf::rowSwapRedBlue>(app, ts, data, out, count);
        convertColours(app, ts, kExpand565, out, frame, count);
        break;
    default:
        reportError(guest, 0x54f7a8, 0x54f82c, 0x397, args({}, { 0x54f83c, type }));
        return out;
    }
    convertColours(app, ts, kExpand8888To8565, frame, out, count);
    return out;
}

nfs::u32 frontend::watbClut16aPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a830", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::clut16aPointer(app, ts, args.eax, args.edx); });
}

/* sub_50a960: the converter from a shape's type straight to edx, or 0 where there is
 * none; for a shape with a palette going to more than 8 bits, the palette readied too,
 * in 0xa0ac14 at the target's depth. */
nfs::u32 frontend::watb::converterFor(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    const u32 shape = argEax, target = argEdx, keyed = argEbx;
    const u32 type = guest.dword(shape) & 0xff;
    const u32 fromIndex = guest::direct<0x50a570, depthIndex>(app, ts, type);
    const u32 slot = fromIndex * 8 + guest::direct<0x50a570, depthIndex>(app, ts, target);
    const u32 between = guest.byte(kBetweenTypes + slot);
    if (target != between)
        return 0;
    const u32 from = guest::direct<0x50a570, depthIndex>(app, ts, type) * 8;
    const u32 where = from + guest::direct<0x50a570, depthIndex>(app, ts, target);
    const u32 converter = guest.dword(where * 4 + (keyed != 0 ? kSpanTablesKeyed : kSpanTablesPlain));
    const u32 fromDepth = shapeDepth(guest, shape);
    const u32 toDepth = guest.byte(kDepthOfType + (between & 0x7f));
    if (s32(fromDepth) > 8 || s32(toDepth) <= 8)
        return converter;
    const u32 clut = fe::direct<0x50a5d0, clutPointer>(app, ts, shape, kClutBuffer);
    guest.setDword(kCurrentClut, clut);
    const u32 colours = u32(1) << (fromDepth & 31);
    u32 convert = 0;
    if (target == 0x6d)
        convert = k8888To4444;
    else if (toDepth == 0xf)
        convert = keyed != 0 ? kTo1555Keyed : kTo1555;
    else if (toDepth == 0x10)
        convert = keyed != 0 ? kTo565Keyed : kTo565;
    if (convert != 0)
        convertColours(app, ts, convert, clut, kClutBuffer, colours);
    return converter;
}

nfs::u32 frontend::watbConverterFor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50a960", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::converterFor(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50aad0, convertshapetype: a new shape of the type edx, from the shape at eax,
 * through as many types between as it takes (at most 10), each step in memory of its
 * own, freed once the next is made.  ebx asks for keyed colours, ecx for the keyed
 * table of converters.  What is at the end of each is checked: 'PHSC'. */
nfs::u32 frontend::watb::convertShape(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x64);
    constexpr u32 kSentinel = 0x50485343;
    const u32 frame = guest.frame();
    const u32 kBetweenSlot = frame + 0x20;      // read by the second report before it is set
    const u32 target = argEdx, mode = argEcx;
    u32 keyed = argEbx;
    u32 shape = argEax;
    const u32 firstClut = guest::direct<0x4fd1c0, shapeClut>(app, ts, shape);
    u32 previous = 0;
    for (u32 tries = 10;;)
    {
        const u32 fromIndex = guest::direct<0x50a570, depthIndex>(app, ts, guest.dword(shape) & 0xff);
        const u32 toIndex = guest::direct<0x50a570, depthIndex>(app, ts, target);
        if (s32(fromIndex) < 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x3fb, args({}, { 0x54f87c, guest.dword(shape) & 0xff }));
        if (s32(toIndex) < 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x3fd, args({}, { 0x54f8b4, guest.dword(kBetweenSlot) }));
        const u32 tableFrom = guest::direct<0x50a570, depthIndex>(app, ts, guest.dword(shape) & 0xff) * 8;
        const u32 tableWhere = tableFrom + guest::direct<0x50a570, depthIndex>(app, ts, target);
        const u32 convert = guest.dword(tableWhere * 4 + (mode != 0 ? kSpanTablesKeyed : kSpanTablesPlain));
        const u32 betweenFrom = guest::direct<0x50a570, depthIndex>(app, ts, guest.dword(shape) & 0xff) * 8;
        const u32 betweenWhere = betweenFrom + guest::direct<0x50a570, depthIndex>(app, ts, target);
        const u32 between = guest.byte(kBetweenTypes + betweenWhere);
        guest.setDword(kBetweenSlot, between);
        if (between == 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x409, args({}, { 0x54f8fc, guest.dword(shape) & 0xff, target }));

        const u32 fromDepth = shapeDepth(guest, shape);
        const u32 toDepth = guest.byte(kDepthOfType + (between & 0x7f));
        const u32 width = sar(guest.dword(shape + kShapeWidth), 16);
        const u32 height = sar(guest.dword(shape + kShapeHeight), 16);
        const u32 stride = fe::direct<0x5236c0, frontend::watc::shapeRowBytes>(app, ts, shape);
        const u32 rowBytes = sar((width * roundedDepth(toDepth) + 7) & ~u32(7), 3);

        // A palette's shape going to more than 8 bits: its colours at the new depth.
        if (s32(fromDepth) <= 8 && s32(toDepth) > 8)
        {
            const u32 colours = u32(1) << (fromDepth & 31);
            if (keyed != 0)
            {
                for (u32 i = 0; s32(i) < s32(colours); ++i)
                    guest.setDword(kClutBuffer + i * 4, guest.dword(u32(guest.byte(0x56ae64 + i)) * 4 + kNeutralClut));
                guest.setDword(kCurrentClut, kClutBuffer);
            }
            else
                guest.setDword(kCurrentClut, guest::direct<0x50a5d0, clutPointer>(app, ts, firstClut, kClutBuffer));
            u32 step = 0;
            if (target == 0x6d)
                step = k8888To4444;
            else if (toDepth == 0xf)
                step = mode != 0 ? kTo1555Keyed : kTo1555;
            else if (toDepth == 0x10)
                step = mode != 0 ? kTo565Keyed : kTo565;
            if (step != 0)
                convertColours(app, ts, step, guest.dword(kCurrentClut), kClutBuffer, colours);
            keyed = 0;
        }

        fe::direct<0x4df690, watcom::clib::sprintf>(guest.app(), guest.ts(), frame, 0x54f970, guest::Words{fromDepth, toDepth});
        const u32 pixelBytes = height * rowBytes;
        guest.setDword(kErrorFile, 0x54f7a8);
        guest.setDword(kErrorFunction, 0x54f868);
        guest.setDword(kErrorLine, 0x43e);
        const u32 made = *fe::direct<0x4e1620, ea::reserved>(app, ts, frame, pixelBytes + 0x14, guest.dword(kMemoryPool));
        guest.setDword(made + pixelBytes + 0x10, kSentinel);

        guest.setDword(made, between & 0xff);
        guest.setWord(made + 4, std::uint16_t(width));
        guest.setWord(made + 6, std::uint16_t(height));
        guest.setWord(made + 8, guest.word(shape + 8));
        guest.setWord(made + 0xa, guest.word(shape + 0xa));
        // Each read before its write: the memory made may be where the shape was.
        const u32 flagsLow = guest.dword(shape + 0xc) & 0xfff;
        guest.setWord(made + 0xc, guest.word(made + 0xc) & 0xf000);
        guest.setDword(made + 0xc, guest.dword(made + 0xc) | flagsLow);
        const u32 flagsHigh = guest.dword(shape + 0xc) & 0xfff0000;
        guest.setWord(made + 0xe, guest.word(made + 0xe) & 0xf000);
        const u32 header = guest.dword(made + 0xc) | flagsHigh;
        guest.setDword(made + 0xc, header);

        u32 from = shape + kShapePixels, to = made + kShapePixels;
        for (u32 row = 0; s32(row) < s32(height); ++row)
        {
            fe::pointerCall(guest, convert, args({ from, to, width }));
            to += rowBytes;
            from += stride;
        }
        if (guest.dword(made + pixelBytes + 0x10) != kSentinel)
        {
            reportError(guest, 0x54f7a8, 0x54f868, 0x461,
                        args({}, { 0x54f978, width, height, fromDepth, width, height, toDepth }));
        }
        if (previous != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, previous);
        shape = made;
        previous = made;
        if ((guest.dword(made) & 0xff) == target || --tries == 0)
            return made;
    }
}

nfs::u32 frontend::watbConvertShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50aad0", 0, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::convertShape(app, ts, args.eax, args.edx, args.ebx, args.ecx);
    });
}

// ---------------------------------------------------------------------------
// movdfl: win\movdfl.c
// ---------------------------------------------------------------------------

/* sub_50b0b0, movdfl: a shape drawn at edx, ebx through the blitters at ecx -- by depth
 * at +0 to +0x14, or one for all at +0x18, a type at +0x20, and at +0x1c how keyed it
 * is -- clipped to the window, converted as it goes where there is a converter and first
 * where there is not. */
nfs::u32 frontend::watb::moveShape(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x4c);
    constexpr u32 kSpanTwoSteps = 0x50b080;
    constexpr u32 kFirstStep = 0xa0b018, kSecondStep = 0xa0b014;
    const u32 frame = guest.frame();
    const u32 kBlitterSlot = frame + 0x38;      // kept where it is: a depth not in the table leaves what was there
    u32 shape = argEax, x = argEdx, y = argEbx;
    const u32 blitters = argEcx;
    if (shapeDepth(guest, shape) == 0)
        reportError(guest, 0x54f9ac, 0x54f9bc, 0x26, args({}, { 0x54f9c4, guest.dword(shape) & 0xff }));
    u32 fixed = guest.dword(blitters + 0x18);
    u32 wanted = guest.byte(kWindowType);
    guest.setDword(kFirstStep, 0);
    if (guest.dword(blitters + 0x20) != 0)
        wanted = guest.dword(blitters + 0x20);
    if (wanted == 0)
        reportError(guest, 0x54f9ac, 0x54f9bc, 0x2f, args({}, { 0x54f9ec, wanted }));
    u32 converted = 0;
    if (fixed == 0 && (guest.dword(shape) & 0xff) != wanted)
    {
        const u32 keyedConverter = guest.dword(blitters + 0x1c) == 2 ? 0 : 1;
        fixed = fe::direct<0x50a960, converterFor>(app, ts, shape, wanted, keyedConverter);
        if (fixed == 0)
        {
            const u32 mode = guest.dword(blitters + 0x1c) == 2 ? 0 : 1;
            const u32 keyed = guest.dword(blitters + 0x1c) == 1 ? 1 : 0;
            converted = fe::direct<0x50aad0, convertShape>(app, ts, shape, wanted, keyed, mode);
            shape = converted;
        }
        else if (guest.dword(blitters + 0x1c) != 2)
        {
            guest.setDword(kFirstStep, fixed);
            fixed = 0;
        }
    }

    u32 from = shape + kShapePixels;
    u32 width = sar(guest.dword(shape + kShapeWidth), 16);
    const u32 heightWord = guest.dword(shape + kShapeHeight);
    const u32 depth = roundedDepth(shapeDepth(guest, shape));
    const u32 rowBytes = sar(width * depth, 3);
    u32 fromSkip = fe::direct<0x5236c0, frontend::watc::shapeRowBytes>(app, ts, shape) - rowBytes;
    u32 height = sar(heightWord, 16);

    u32 result = guest.dword(kClipTop) - y;
    if (positive(result))
    {
        from += rowBytes * result;
        y += result;
        height -= result;
    }
    result = y + height - guest.dword(kClipBottom);
    if (positive(result))
        height -= result;
    result = guest.dword(kClipLeft) - x;
    if (positive(result))
    {
        const u32 skipped = sar(result * depth, 3);
        from += skipped;
        width -= result;
        x += result;
        fromSkip += skipped;
    }
    result = x + width - guest.dword(kClipRight);
    if (positive(result))
    {
        width -= result;
        fromSkip += sar(result * depth, 3);
    }

    if (positive(width) && positive(height))
    {
        const u32 pixels = guest.dword(kWindowPixels);
        const u32 fromBytes = sar(depth * width, 3);
        const u32 toBytes = sar(roundedDepth(guest.byte(kWindowDepth)) * width, 3);
        u32 to = guest.dword(guest.dword(kRowOffsets) + (y << 2)) + guest.dword(guest.dword(kColumnOffsets) + (x << 2));
        const u32 toSkip = guest.dword(kWindowStride) - toBytes;

        if (fixed != 0)
            guest.setDword(kBlitterSlot, fixed);
        else
        {
            switch (guest.byte(kWindowDepth))
            {
            case 4: guest.setDword(kBlitterSlot, guest.dword(blitters)); break;
            case 8: guest.setDword(kBlitterSlot, guest.dword(blitters + 4)); break;
            case 0xf: guest.setDword(kBlitterSlot, guest.dword(blitters + 8)); break;
            case 0x10: guest.setDword(kBlitterSlot, guest.dword(blitters + 0xc)); break;
            case 0x18: guest.setDword(kBlitterSlot, guest.dword(blitters + 0x10)); break;
            case 0x20: guest.setDword(kBlitterSlot, guest.dword(blitters + 0x14)); break;
            default: break;
            }
            if (guest.dword(kFirstStep) != 0)
            {
                guest.setDword(kSecondStep, guest.dword(kBlitterSlot));
                guest.setDword(kBlitterSlot, kSpanTwoSteps);
            }
        }
        const u32 blitter = guest.dword(kBlitterSlot);
        const u32 toStep = toBytes + toSkip, fromStep = fromBytes + fromSkip;

        if (guest.dword(kWindowPolled) != 0)
        {
            // The window's owner is let at the messages every so many pixels.
            u32 sinceLast = 0;
            for (u32 rows = height; rows != 0; --rows)
            {
                fe::pointerCall(guest, blitter, args({ from, pixels + to, width }));
                result = fromStep;
                to += toStep;
                from += fromStep;
                sinceLast += width;
                const u32 every = guest.dword(0x5643ec);
                if (s32(sinceLast) > s32(every))
                {
                    sinceLast = 0;
                    result = fe::direct<0x4f90e0, frontend::wata::windowReshow>(app, ts);
                    if (result == 0)
                        return 0;
                }
            }
        }
        else if (toSkip == 0 && fromSkip == 0 && blitter != kSpanTwoSteps)
        {
            // Without gaps: one long span.
            fe::pointerCall(guest, blitter, args({ from, pixels + to, width * height }));
            result = toBytes * height;
            from += height * fromBytes;
        }
        else
        {
            for (u32 rows = height; rows != 0; --rows)
            {
                fe::pointerCall(guest, blitter, args({ from, pixels + to, width }));
                result = toStep;
                to += toStep;
                from += fromStep;
            }
        }
    }

    if (converted != 0)
        result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, converted);
    return result;
}

nfs::u32 frontend::watbMoveShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b0b0", 0, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::moveShape(app, ts, args.eax, args.edx, args.ebx, args.ecx); });
}

// ---------------------------------------------------------------------------
// Text: font\textpc.c
// ---------------------------------------------------------------------------

/* sub_50b540: a string's next byte: eax the place of the pointer. */
nfs::u32 frontend::watb::nextByte(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const u32 place = argEax;
    const u32 at = guest.dword(place) + 1;
    const u32 value = guest.byte(at - 1);
    guest.setDword(place, at);
    return value;
}

nfs::u32 frontend::watbNextByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b540", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::nextByte(app, ts, args.eax); });
}

/* sub_50b560: a character's glyph number by the font's table, for ' ' to 0x7f. */
nfs::u32 frontend::watb::charWidthEntry(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const u32 character = argEax;
    if (s32(character) < 0x20 || s32(character) > 0x7f)
        return character;
    return guest.word(character * 2 + 0x568594);
}

nfs::u32 frontend::watbCharWidthEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b560", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::charWidthEntry(app, ts, args.eax); });
}

/* sub_50b580: a string's next character: two bytes with the top bit of the first set,
 * else one, through sub_50b560. */
nfs::u32 frontend::watb::nextChar(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    const u32 place = argEax;
    u32 at = guest.dword(place);
    const u32 first = guest.byte(at);
    ++at;
    u32 result;
    if ((first & 0x80) != 0)
    {
        result = ((first << 8) | guest.byte(at)) & 0xffff;
        ++at;
    }
    else
        result = guest::direct<0x50b560, charWidthEntry>(app, ts, first) & 0xffff;
    guest.setDword(place, at);
    return result;
}

nfs::u32 frontend::watbNextChar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b580", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::nextChar(app, ts, args.eax); });
}

namespace
{
constexpr u32 kFontPalette = 0x567280;      // the 16 ARGBs of the font's nibbles, then its blend table
constexpr u32 kFontColours = 0x567228;      // the 16 colours plotted
constexpr u32 kFontColour = 0x567220;
}

/* sub_50b5c0: a line of a 4-bit font in alpha: a fast loop for 15 and 16 bits fully
 * inside the window, sub_5239ec for 8 bits, and else plotted pixel by pixel.  Stack: the
 * first row, the width and height in pixels, the bytes a row. */
nfs::u32 frontend::watb::textAlpha(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                     u32 rowWord, u32 widthWord, u32 heightWord, u32 strideWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x40);
    const u32 frame = guest.frame();
    const u32 x = argEax, y = argEdx, xOffset = argEcx;
    const u32 stride = strideWord;
    const u32 width = widthWord, height = heightWord;
    u32 font = argEbx + kShapePixels + rowWord * stride + sar(xOffset - sar(xOffset, 31), 1);
    const u32 rowBytes = sar(width + 1, 1);
    const u32 palette = guest.dword(kFontPalette);
    const auto colourOf = [&](u32 nibble) { return guest.dword(palette + nibble * 4); };

    // What the tests leave in eax for a slow loop that has no rows to draw.
    u32 leftEax = rowBytes;
    const auto inside = [&] {
        const std::uint8_t locked = guest.byte(kWindowLocked);
        leftEax = (leftEax & 0xffff00ff) | (u32(locked) << 8);
        if (locked != 0)
            return false;
        leftEax = x;
        if (s32(x) < s32(guest.dword(kClipLeft)) || s32(x) >= s32(guest.dword(kClipRight)))
            return false;
        leftEax = y;
        if (s32(y) < s32(guest.dword(kClipTop)) || s32(y) >= s32(guest.dword(kClipBottom)))
            return false;
        leftEax = x + width;
        if (s32(leftEax) < s32(guest.dword(kClipLeft)) || s32(leftEax) >= s32(guest.dword(kClipRight)))
            return false;
        leftEax = y + height;
        if (s32(leftEax) < s32(guest.dword(kClipTop)) || s32(leftEax) >= s32(guest.dword(kClipBottom)))
            return false;
        return true;
    };

    if (inside())
    {
        const u32 skip = stride - rowBytes;
        const std::uint8_t depth = guest.byte(kWindowDepth);
        leftEax = skip;
        if (depth == 0x10 || depth == 0xf)
        {
            const bool is565 = depth == 0x10;
            u32 row = y, rows = height;
            Scratch left{ 0, 0 };
            u32 lastEbx = 0;
            const auto blendNibble = [&](u32 to, u32 nibble) {
                u32 argb = colourOf(nibble);
                left.eax = nibble;
                lastEbx = argb;
                if (argb < kVisibleFrom)
                    return;
                if (argb < kOpaqueFrom)
                    argb = is565 ? blendOver565(guest.word(to), argb) : blendOver1555(guest.word(to), argb);
                lastEbx = is565 ? packTo565(argb, left) : packTo1555(argb, left);
                guest.setWord(to, std::uint16_t(lastEbx));
            };
            do
            {
                u32 to = guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(guest.dword(kRowOffsets) + row * 4)
                    + guest.dword(kWindowPixels);
                u32 bytes = rowBytes;
                do
                {
                    blendNibble(to, u32(guest.byte(font)) >> 4);
                    to += 2;
                    blendNibble(to, guest.byte(font) & 0xf);
                    to += 2;
                    ++font;
                } while (--bytes != 0);
                ++row;
                font += skip;
            } while (--rows != 0);
            if (is565)
            {
                return left.eax;
            }
            return row;
        }
        if (depth == 8)
        {
            const u32 toSkip = guest.dword(kWindowStride) - rowBytes * 2;
            const u32 rowAt = guest.dword(guest.dword(kRowOffsets) + (y << 2));
            const u32 to = guest.dword(kWindowPixels) + rowAt + x;
            // sub_5239ec reads its seven words above its return address, where the code pushes them.
            const fe::Frame pushed(guest, ts, 7 * 4);
            const u32 words = guest.frame();
            const u32 values[] = { palette + 0x40, font, to, rowBytes, height, skip, toSkip };
            for (u32 i = 0; i != 7; ++i)
                guest.setDword(words + 4 * i, values[i]);
            // eax `to`, as the code's call has it: blend4To8 hands it back when it writes no pixels.
            return fe::direct<0x5239ec, frontend::watc::blend4To8>(app, ts, words, to);
        }
    }

    if (!positive(height))
    {
        return leftEax;
    }
    const u32 skip = stride - rowBytes, lastRow = y + height;
    u32 column = 0;
    for (u32 row = y; s32(row) < s32(lastRow); ++row)
    {
        column = 0;
        if (positive(width))
        {
            do
            {
                const u32 packed = guest.byte(font);
                ++font;
                const u32 high = sar(packed, 4), low = packed & 0xf;
                if (high != 0)
                    fe::direct<0x523760, frontend::watc::putPixel>(app, ts, x + column, row, colourOf(high));
                if (low != 0)
                    fe::direct<0x523760, frontend::watc::putPixel>(app, ts, x + 1 + column, row, colourOf(low));
                column += 2;
            } while (s32(column) < s32(width));
        }
        font += skip;
    }
    return positive(width) ? width : lastRow;
}

nfs::u32 frontend::watbTextAlpha(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50b5c0", 16, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::textAlpha(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4),
                               guest::at(app, ts.sp + 8), guest::at(app, ts.sp + 12), guest::at(app,
                               ts.sp + 16));
    });
}

/* sub_50ba90: the same plotted by sub_523b00 from the font's own colours. */
nfs::u32 frontend::watb::textPlotted(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                       u32 rowWord, u32 widthWord, u32 heightWord, u32 strideWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x2c);
    const u32 frame = guest.frame();
    const u32 x = argEax, y = argEdx, xOffset = argEcx;
    const u32 stride = strideWord;
    const u32 width = widthWord, height = heightWord;
    if (!positive(height))
        return x;
    const u32 half = sar(xOffset - sar(xOffset, 31), 1);
    u32 line = argEbx + kShapePixels + stride * rowWord;
    const u32 lastRow = y + height;
    u32 row = y;
    do
    {
        u32 font = line + half;
        u32 column = 0;
        if (positive(width))
        {
            do
            {
                const u32 packed = guest.byte(font);
                ++font;
                const u32 high = sar(packed, 4), low = packed & 0xf;
                if (high != 0)
                    fe::direct<0x523b00, frontend::watc::writePixel>(app, ts, x + column, row,
                                                                     guest.dword(high * 4 + kFontColours));
                if (low != 0)
                    fe::direct<0x523b00, frontend::watc::writePixel>(app, ts, x + 1 + column, row,
                                                                     guest.dword(low * 4 + kFontColours));
                column += 2;
            } while (s32(column) < s32(width));
        }
        line += stride;
        ++row;
    } while (s32(row) < s32(lastRow));
    return stride;
}

nfs::u32 frontend::watbTextPlotted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50ba90", 16, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::textPlotted(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4),
                                 guest::at(app, ts.sp + 8), guest::at(app, ts.sp + 12), guest::at(app,
                                 ts.sp + 16));
    });
}

/* sub_50bb80: a line of a 1-bit font: a byte of its mask at a time through what plots
 * eight pixels of this depth.  Stack: the first row, the width and height in bits, the
 * bytes a row. */
nfs::u32 frontend::watb::textMono(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                    u32 rowWord, u32 widthWord, u32 heightWord, u32 strideWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x38);
    const u32 frame = guest.frame();
    u32 x = argEax;
    const u32 y = argEdx, font = argEbx, bitOffset = argEcx;
    u32 width = widthWord;
    u32 height = heightWord;
    // What plots eight pixels at the window's depth (sub_50c220, sub_50c270, sub_50c2c0, sub_50c350):
    // the next eight's place.
    std::uint8_t plotDepth = 0;
    switch (guest.byte(kWindowDepth))
    {
    case 8: plotDepth = 8; break;
    case 0xf:
    case 0x10: plotDepth = 16; break;
    case 0x18: plotDepth = 24; break;
    case 0x20: plotDepth = 32; break;
    default:
        reportError(guest, 0x54fa18, 0x54fa28, 0xf1, args({}, { 0x54fa34, guest.byte(kWindowDepth) }));
        return 0;
    }
    const auto plot = [&](u32 bits, u32 colour, u32 to) -> u32 {
        switch (plotDepth)
        {
        case 8: return fe::direct<0x50c220, plotMask8>(app, ts, bits, colour, to);
        case 16: return fe::direct<0x50c270, plotMask16>(app, ts, bits, colour, to);
        case 24: return fe::direct<0x50c2c0, plotMask24>(app, ts, bits, colour, to);
        default: return fe::direct<0x50c350, plotMask32>(app, ts, bits, colour, to);
        }
    };
    const u32 clipLeft = guest.dword(kClipLeft);
    const u32 leftClip = s32(x) < s32(clipLeft) ? clipLeft - x : 0;
    u32 row = s32(y) < s32(guest.dword(kClipTop)) ? guest.dword(kClipTop) - y : 0;
    if (s32(x + width) > s32(guest.dword(kClipRight)))
        width = guest.dword(kClipRight) - x;
    if (s32(height + y) > s32(guest.dword(kClipBottom)))
        height = guest.dword(kClipBottom) - y;
    if (s32(width) < 0 || s32(height) < 0 || s32(leftClip) >= s32(width))
        return leftClip;
    if (s32(row) >= s32(height))
        return leftClip;
    const u32 stride = strideWord;
    const u32 aligned = leftClip & ~u32(7);
    u32 line = font + kShapePixels + (rowWord + row) * stride + sar(bitOffset + aligned, 3);
    const u32 rowAt = guest.dword(guest.dword(kRowOffsets) + (y + row) * 4);
    x += aligned;
    u32 to = guest.dword(kWindowPixels) + rowAt + guest.dword(guest.dword(kColumnOffsets) + 4) * x;
    width -= aligned;
    const u32 wholeBytes = sar(width, 3);
    const u32 restBits = width - (wholeBytes << 3);
    const std::uint8_t firstMask = std::uint8_t(sar(0xff, unsigned(leftClip & 7)));
    const std::uint8_t lastMask = std::uint8_t(0xff << (std::uint8_t(8 - std::uint8_t(restBits)) & 31));
    for (;;)
    {
        const u32 thisRow = row;
        ++row;
        if (s32(thisRow) >= s32(height))
        {
            return thisRow;
        }
        std::uint8_t mask = firstMask;
        u32 source = line;
        u32 at = to;
        if (wholeBytes != 0)
        {
            at = plot(u32(guest.byte(source) & firstMask), guest.dword(kFontColour), at);
            mask = 0xff;
            ++source;
            for (u32 bytes = wholeBytes - 1; bytes != 0; --bytes)
            {
                const u32 bits = guest.byte(source);
                ++source;
                at = plot(bits, guest.dword(kFontColour), at);
            }
        }
        if (restBits != 0)
            plot(u32(guest.byte(source) & mask & lastMask), guest.dword(kFontColour), at);
        line += stride;
        to += guest.dword(kWindowStride);
    }
}

nfs::u32 frontend::watbTextMono(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50bb80", 16, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::textMono(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4),
                              guest::at(app, ts.sp + 8), guest::at(app, ts.sp + 12), guest::at(app,
                              ts.sp + 16));
    });
}

/* sub_50bdf0: for each palette index and each of 16 ARGBs, the index nearest the ARGB
 * over that colour: the table the 8-bit blitter works from. */
nfs::u32 frontend::watb::blendTable(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x344);
    const u32 colours = argEax, table = argEdx;
    std::uint8_t red[256], green[256], blue[256];
    for (u32 i = 0; i != 256; ++i)
    {
        const u32 colour = guest.dword(kNeutralClut + i * 4);
        red[i] = std::uint8_t(colour >> 16);
        green[i] = std::uint8_t(colour >> 8);
        blue[i] = std::uint8_t(colour);
    }
    const auto over = [](std::int32_t under, std::int32_t show, std::int32_t top) {
        const std::int32_t value = under * show / 0xff + top;
        return value > 0xff ? 0xff : value;
    };
    for (u32 index = 0; index != 256; ++index)
    {
        for (u32 k = 0; k != 16; ++k)
        {
            const u32 colour = colours + k * 4;
            const std::int32_t alpha = guest.byte(colour + 3);
            u32 nearest = index;
            if (alpha != 0)
            {
                const std::int32_t show = 0xff - alpha;
                const std::int32_t r = over(red[index], show, guest.byte(colour + 2));
                const std::int32_t g = over(green[index], show, guest.byte(colour + 1));
                const std::int32_t b = over(blue[index], show, guest.byte(colour));
                nearest = fe::direct<0x4ef760, frontend::eab::colourPack>(app, ts, (u32(alpha) << 24) | (u32(r) << 16) | (u32(g) << 8) | u32(b));
            }
            guest.setByte(table + ((k << 8) | index), std::uint8_t(nearest));
        }
    }
    return 3 * 256;
}

nfs::u32 frontend::watbBlendTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50bdf0", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendTable(app, ts, args.eax, args.edx); });
}

/* sub_50bfd0: the font's 16 ARGBs, a colour faded in over alpha; the blend table for 8 bits. */
nfs::u32 frontend::watb::fadeTable(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x30);
    const u32 table = guest.dword(kFontPalette);
    const u32 argb = guest::direct<0x50c3a0, colourToArgb>(app, ts, argEax);
    const std::int32_t blue = argb & 0xff, green = (argb >> 8) & 0xff, red = (argb >> 16) & 0xff;
    for (std::int32_t i = 0; i != 16; ++i)
    {
        const u32 entry = table + u32(i) * 4;
        guest.setByte(entry + 3, std::uint8_t(i * 0xff / 0xf));
        guest.setByte(entry + 2, std::uint8_t(i * red / 0xf));
        guest.setByte(entry + 1, std::uint8_t(i * green / 0xf));
        guest.setByte(entry, std::uint8_t(i * blue / 0xf));
    }
    if (guest.byte(kWindowDepth) > 8)
        return table + 0x40;
    return guest::direct<0x50bdf0, blendTable>(app, ts, table, guest.dword(kFontPalette) + 0x40);
}

nfs::u32 frontend::watbFadeTable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50bfd0", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::fadeTable(app, ts, args.eax); });
}

namespace
{
/* Whether sub_50c180 calls nothing and leaves eax as it came: neither colours given nor a fade. */
bool textPaletteLeavesEax(App* app)
{
    return guest::at(app, 0x5672a4) == 0 && !(guest::at(app, 0x56720c) != 0 && guest::at(app, 0x567208) == 4);
}
}

/* sub_50c0d0, inittextdraw: the font's tables made and filled, its blitter chosen.  No result:
 * what sub_50c180 leaves, which where it leaves eax as it came is the block just made, or this
 * function's own eax as it came, which its thunk does. */
nfs::u32 frontend::watb::initTextDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    u32 made = 0;
    if (guest.dword(kFontPalette) == 0)
    {
        constexpr u32 kBytes = 0x1040;
        guest.setDword(0x567284, kBytes);
        guest.setDword(kErrorFile, 0x54fa18);
        guest.setDword(kErrorFunction, 0x54fa60);
        guest.setDword(kErrorLine, 0x150);
        made = *fe::direct<0x4e1620, ea::reserved>(app, ts, u32(0x54fa70), kBytes, guest.dword(kMemoryPool));
        guest.setDword(kFontPalette, made);
    }
    const bool passedOn = textPaletteLeavesEax(app);
    const u32 coloured = guest::direct<0x50c180, textPalette>(app, ts);
    const u32 result = passedOn ? made : coloured;
    u32 blitter = 0x50b5c0;
    if (guest.dword(0x567208) == 1)
        blitter = 0x50bb80;
    else if (guest.dword(0x56720c) == 0)
        blitter = 0x50ba90;
    guest.setDword(0x567288, blitter);
    guest.setDword(0x567290, 0);
    guest.setDword(0x56728c, 0);
    return result;
}

nfs::u32 frontend::watbInitTextDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c0d0", 0, 0);
    const u32 previous = args.eax;
    const bool passedOn = guest::at(app, kFontPalette) != 0 && textPaletteLeavesEax(app);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 result = watb::initTextDraw(app, ts);
        return passedOn ? previous : result;
    });
}

/* sub_50c180: the font's colours for its kind: copied where they are given, faded where not.  No
 * result: what memmove, sub_50bdf0 or sub_50bfd0 leaves -- memmove's is none, 0 here -- or eax as
 * it came where it calls nothing, which its thunk does. */
nfs::u32 frontend::watb::textPalette(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    if (guest.dword(0x5672a4) != 0)
    {
        fe::direct<kMemmove, ea::moveMemory>(app, ts, guest.dword(kFontPalette), kFontColours, u32(0x40));
        if (guest.byte(kWindowDepth) > 8)
            return 0;
        const u32 table = guest.dword(kFontPalette);
        return guest::direct<0x50bdf0, blendTable>(app, ts, table, table + 0x40);
    }
    if (guest.dword(0x56720c) != 0 && guest.dword(0x567208) == 4)
        return guest::direct<0x50bfd0, fadeTable>(app, ts, guest.dword(kFontColour));
    return 0;
}

nfs::u32 frontend::watbTextPalette(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c180", 0, 0);
    const u32 previous = args.eax;
    const bool passedOn = textPaletteLeavesEax(app);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 result = watb::textPalette(app, ts);
        return passedOn ? previous : result;
    });
}

/* sub_50c1e0: the font's tables given back.  No result: purgememadr's, or with no tables eax as
 * it came, which its thunk does. */
nfs::u32 frontend::watb::freeTextDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 8);
    const u32 table = guest.dword(kFontPalette);
    u32 left = 0;
    if (table != 0)
    {
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, table);
        guest.setDword(kFontPalette, 0);
        guest.setDword(0x567284, 0);
    }
    guest.setDword(0x567288, 0);
    return left;
}

nfs::u32 frontend::watbFreeTextDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c1e0", 0, 0);
    const u32 previous = args.eax;
    const bool none = guest::at(app, kFontPalette) == 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 left = watb::freeTextDraw(app, ts);
        return none ? previous : left;
    });
}

/* sub_50c220, sub_50c270, sub_50c2c0, sub_50c350: the set bits of al's eight pixels at
 * ebx in the colour edx, at 8, 16, 24 and 32 bits; the next eight's place. */
nfs::u32 frontend::watb::plotMask8(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 to = argEbx;
    for (u32 pixel = 0; pixel != 8; ++pixel)
        if ((argEax & (0x80 >> pixel)) != 0)
            guest.setByte(to + pixel, std::uint8_t(argEdx));
    return to + 8;
}

nfs::u32 frontend::watbPlotMask8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c220", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::plotMask8(app, ts, args.eax, args.edx, args.ebx); });
}

nfs::u32 frontend::watb::plotMask16(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 to = argEbx;
    for (u32 pixel = 0; pixel != 8; ++pixel)
        if ((argEax & (0x80 >> pixel)) != 0)
            guest.setWord(to + pixel * 2, std::uint16_t(argEdx));
    return to + 0x10;
}

nfs::u32 frontend::watbPlotMask16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c270", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::plotMask16(app, ts, args.eax, args.edx, args.ebx); });
}

nfs::u32 frontend::watb::plotMask24(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 to = argEbx, colour = argEdx;
    for (u32 pixel = 0; pixel != 8; ++pixel)
        if ((argEax & (0x80 >> pixel)) != 0)
        {
            guest.setByte(to + pixel * 3 + 1, std::uint8_t(colour >> 8));
            guest.setByte(to + pixel * 3 + 2, std::uint8_t(colour));
            guest.setByte(to + pixel * 3, std::uint8_t(colour >> 16));
        }
    return to + 0x18;
}

nfs::u32 frontend::watbPlotMask24(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c2c0", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::plotMask24(app, ts, args.eax, args.edx, args.ebx); });
}

nfs::u32 frontend::watb::plotMask32(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const u32 to = argEbx;
    for (u32 pixel = 0; pixel != 8; ++pixel)
        if ((argEax & (0x80 >> pixel)) != 0)
            guest.setDword(to + pixel * 4, argEdx);
    return to + 0x20;
}

nfs::u32 frontend::watbPlotMask32(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c350", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::plotMask32(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50c3a0, getargb: a colour of the current window's depth as an ARGB. */
nfs::u32 frontend::watb::colourToArgb(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    const u32 colour = argEax;
    const std::uint8_t depth = guest.byte(kWindowDepth);
    if (depth == 0xf)
    {
        if ((colour & 0xffff0000) != 0)
            reportError(guest, 0x54fa7c, 0x54fa8c, 0x90, args({}, { 0x54fa94, depth, colour, colour }));
        const u32 alpha = (colour & 0x8000) != 0 ? 0xff : 0;
        return (alpha << 24) | ((sar(colour, 10) & 0x1f) << 19) | ((sar(colour, 5) & 0x1f) << 11) | ((colour & 0x1f) << 3);
    }
    if (depth == 0x10)
    {
        if ((colour & 0xffff0000) != 0)
            reportError(guest, 0x54fa7c, 0x54fa8c, 0xa4, args({}, { 0x54faf4, depth, colour, colour }));
        const u32 alpha = colour != 0 ? 0xff : 0;
        return (alpha << 24) | ((sar(colour, 11) & 0x1f) << 19) | ((sar(colour, 5) & 0x3f) << 10) | ((colour & 0x1f) << 3);
    }
    if (depth == 0x18 || depth == 0x20)
        return colour;
    if (s32(colour) < 0 || s32(colour) >= 0x100)
        reportError(guest, 0x54fa7c, 0x54fa8c, 0xb8, args({}, { 0x54fb54, guest.byte(kWindowDepth), colour, colour }));
    return guest.dword(colour * 4 + kNeutralClut);
}

nfs::u32 frontend::watbColourToArgb(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c3a0", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::colourToArgb(app, ts, args.eax); });
}

/* sub_50c584: a key's handler called, at most one at a time, by the code in al or, with
 * al 0, the scan code in ah; the tables hold the handler's number, 0 for none. */
nfs::u32 frontend::watb::dispatchKey(App* app, ThreadState& ts, u32 argEax)
{
    constexpr u32 kNotInHandler = 0x56879a;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xc);
    const u32 key = argEax;
    const u32 free = guest.dword(kNotInHandler) - 1;
    guest.setDword(kNotInHandler, free);
    u32 number = 0;
    if (free == 0)
    {
        if (std::uint8_t(key) != 0)
            number = guest.byte((key & 0x7f) + 0x568694);
        else
        {
            u32 scan = (key >> 8) & 0xff;
            if (s32(scan) >= 0x84)
                scan = 0x84;
            number = guest.byte(scan + 0x568714);
        }
        if (number != 0)
        {
            fe::pointerCall(guest, guest.dword((number - 1) * 4 + 0x56879e), args({ key, number - 1 }));
            guest.setDword(kNotInHandler, guest.dword(kNotInHandler) + 1);
            return 0;
        }
    }
    guest.setDword(kNotInHandler, guest.dword(kNotInHandler) + 1);
    return key;
}

nfs::u32 frontend::watbDispatchKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c584", 0, 0, fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::dispatchKey(app, ts, args.eax); });
}

/* sub_50c5d4: a handler given a number, a free one or its own, and the key eax set to it. */
nfs::u32 frontend::watb::registerKey(App* app, ThreadState& ts, u32 argEax, u32 argEdx)
{
    Guest guest(app, ts);
    constexpr u32 kHandlers = 0x56879e;
    const u32 key = argEax, handler = argEdx;
    u32 number = 0;
    for (u32 i = 0; i != 0x40; ++i)
    {
        const u32 slot = kHandlers + i * 4;
        if (guest.dword(slot) == handler)
        {
            number = i + 1;
            break;
        }
        if (guest.word(slot) == 0)
        {
            guest.setDword(slot, handler);
            number = i + 1;
            break;
        }
    }
    if (number == 0)
    {
        return handler;
    }
    if (std::uint8_t(key) != 0)
    {
        if (s32(key) <= 0x7f)
            guest.setByte(key + 0x568694, std::uint8_t(number));
    }
    else
    {
        const u32 scan = (key >> 8) & 0xff;
        if (s32(scan) <= 0x84)
            guest.setByte(scan + 0x568714, std::uint8_t(number));
    }
    return number;
}

nfs::u32 frontend::watbRegisterKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50c5d4", 0, 0, kLeavesEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::registerKey(app, ts, args.eax, args.edx); });
}

// ---------------------------------------------------------------------------
// Spans in alpha, drawn by sub_50db30
// ---------------------------------------------------------------------------

/* sub_50d560: a memory copy, 32 bytes at a time through the x87 where it can.  fld and
 * fstp qword only move the bits, as the game's code's doubles do, in this order
 * (which shows where the two overlap). */
nfs::u32 frontend::watb::copyBlock(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 to = argEdx, from = argEax;
    u32 left = argEbx;
    const auto copy1 = [&] {
        guest.setByte(to, guest.byte(from));
        from += 1;
        to += 1;
        left -= 1;
    };
    const auto copy2 = [&] {
        guest.setWord(to, guest.word(from));
        from += 2;
        to += 2;
        left -= 2;
    };
    const auto copy4 = [&] {
        guest.setDword(to, guest.dword(from));
        from += 4;
        to += 4;
        left -= 4;
    };
    const auto load = [&](u32 offset) { return app->getMemory<double>(from + offset); };
    const auto store = [&](u32 offset, double value) { app->getMemory<double>(to + offset) = value; };
    if ((to & 7) != 0)
    {
        if ((to & 1) != 0 && s32(left) >= 1)
            copy1();
        if ((to & 2) != 0 && s32(left) >= 2)
            copy2();
        if ((to & 4) != 0 && s32(left) >= 4)
            copy4();
    }
    left -= 0x80;
    while (s32(left) >= 0)
    {
        const double a0 = load(0), a20 = load(0x20), a40 = load(0x40), a60 = load(0x60);
        const double a10 = load(0x10), a8 = load(8), a18 = load(0x18);
        store(0, a0);
        store(8, a8);
        store(0x10, a10);
        store(0x18, a18);
        const double a30 = load(0x30), a28 = load(0x28), a38 = load(0x38);
        store(0x20, a20);
        store(0x28, a28);
        store(0x30, a30);
        store(0x38, a38);
        const double a50 = load(0x50), a48 = load(0x48), a58 = load(0x58);
        store(0x40, a40);
        store(0x48, a48);
        store(0x50, a50);
        store(0x58, a58);
        const double a70 = load(0x70), a68 = load(0x68), a78 = load(0x78);
        store(0x60, a60);
        store(0x68, a68);
        store(0x70, a70);
        store(0x78, a78);
        from += 0x80;
        to += 0x80;
        left -= 0x80;
    }
    left += 0x60;
    while (s32(left) >= 0)
    {
        const double a0 = load(0), a8 = load(8);
        store(0, a0);
        store(8, a8);
        const double a10 = load(0x10), a18 = load(0x18);
        store(0x10, a10);
        store(0x18, a18);
        from += 0x20;
        to += 0x20;
        left -= 0x20;
    }
    left += 0x18;
    while (s32(left) >= 0)
    {
        store(0, load(0));
        from += 8;
        to += 8;
        left -= 8;
    }
    left += 8;
    if (left != 0)
    {
        if (left >= 4)
            copy4();
        if (left >= 2)
            copy2();
        if (left >= 1)
            copy1();
    }
    return to;
}

nfs::u32 frontend::watbCopyBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d560", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::copyBlock(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50d6e0: ARGBs over palette indices, each result the nearest index. */
nfs::u32 frontend::watb::blendSpanIndexed(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x24);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    u32 result = argEax, leftEbx = argEbx, leftEdx = argEdx;
    do
    {
        const std::uint8_t alpha = guest.byte(from + 3);
        if (alpha != 0)
        {
            const u32 under = guest::direct<0x50c3a0, colourToArgb>(app, ts, u32(guest.byte(to)));
            leftEdx = under;
            if ((under & 0xff000000) != 0)
            {
                const u32 show = std::uint8_t(0xff - alpha);
                const u32 red = std::uint8_t(guest.byte(from + 2) + sar(((under >> 16) & 0xff) * show, 8));
                const u32 green = std::uint8_t(guest.byte(from + 1) + sar(((under >> 8) & 0xff) * show, 8));
                const u32 blue = std::uint8_t(guest.byte(from) + sar((under & 0xff) * show, 8));
                leftEbx = red << 16;
                leftEdx = (green << 8) | leftEbx | blue;
            }
            result = fe::direct<0x4ef760, frontend::eab::colourPack>(app, ts, leftEdx);
            guest.setByte(to, std::uint8_t(result));
        }
        ++to;
        from += 4;
    } while (--count != 0);
    return result;
}

nfs::u32 frontend::watbBlendSpanIndexed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d6e0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendSpanIndexed(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50d7e0: ARGBs over 1555 pixels. */
nfs::u32 frontend::watb::blendSpan1555(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    Scratch left{ argEax, argEdx };
    u32 lastEbx = argEbx;
    do
    {
        u32 argb = guest.dword(from);
        lastEbx = argb;
        if (argb >= kVisibleFrom)
        {
            if (argb < kOpaqueFrom)
                argb = blendOver1555(guest.word(to), argb);
            lastEbx = packTo1555(argb, left);
            guest.setWord(to, std::uint16_t(lastEbx));
        }
        to += 2;
        from += 4;
    } while (--count != 0);
    return left.eax;
}

nfs::u32 frontend::watbBlendSpan1555(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d7e0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendSpan1555(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50d880: ARGBs over 565 pixels. */
nfs::u32 frontend::watb::blendSpan565(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    Scratch left{ argEax, argEdx };
    u32 lastEbx = argEbx;
    do
    {
        u32 argb = guest.dword(from);
        lastEbx = argb;
        if (argb >= kVisibleFrom)
        {
            if (argb < kOpaqueFrom)
                argb = blendOver565(guest.word(to), argb);
            lastEbx = packTo565(argb, left);
            guest.setWord(to, std::uint16_t(lastEbx));
        }
        to += 2;
        from += 4;
    } while (--count != 0);
    return left.eax;
}

nfs::u32 frontend::watbBlendSpan565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d880", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendSpan565(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50d910: ARGBs over 888 pixels. */
nfs::u32 frontend::watb::blendSpan888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    u32 lastEbx = argEbx;
    do
    {
        const std::uint8_t alpha = guest.byte(from + 3);
        if (alpha != 0)
        {
            const u32 show = std::uint8_t(0xff - alpha);
            const std::uint8_t two = std::uint8_t(guest.byte(from + 2) + sar(guest.byte(to + 2) * show, 8));
            const std::uint8_t one = std::uint8_t(guest.byte(from + 1) + sar(guest.byte(to + 1) * show, 8));
            const std::uint8_t zero = std::uint8_t(guest.byte(from) + sar(guest.byte(to) * show, 8));
            guest.setByte(to + 1, one);
            guest.setByte(to + 2, two);
            guest.setByte(to, zero);
            lastEbx = (lastEbx & 0xffff0000) | (u32(zero) << 8) | two;
        }
        to += 3;
        from += 4;
    } while (--count != 0);
    return from;
}

nfs::u32 frontend::watbBlendSpan888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d910", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendSpan888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50d9c0: ARGBs over ARGBs; over nothing, the ARGB as it is. */
nfs::u32 frontend::watb::blendSpan8888(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    u32 lastEbx = argEbx;
    do
    {
        const std::uint8_t alpha = guest.byte(from + 3);
        if (alpha != 0)
        {
            std::uint8_t two = guest.byte(from + 2), one = guest.byte(from + 1), zero = guest.byte(from);
            std::uint8_t outAlpha = alpha;
            const std::uint8_t underAlpha = guest.byte(to + 3);
            if (underAlpha != 0)
            {
                const u32 show = std::uint8_t(0xff - alpha);
                two = std::uint8_t(two + sar(guest.byte(to + 2) * show, 8));
                one = std::uint8_t(one + sar(guest.byte(to + 1) * show, 8));
                zero = std::uint8_t(zero + sar(guest.byte(to) * show, 8));
                outAlpha = std::uint8_t(0xff - sar(show * (0xff - u32(underAlpha)), 8));
            }
            guest.setByte(to + 3, outAlpha);
            guest.setByte(to + 2, two);
            guest.setByte(to + 1, one);
            guest.setByte(to, zero);
            lastEbx = (lastEbx & 0xffff0000) | (u32(zero) << 8) | zero;
        }
        to += 4;
        from += 4;
    } while (--count != 0);
    return from;
}

nfs::u32 frontend::watbBlendSpan8888(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50d9c0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::blendSpan8888(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50daa0: palette indices over 565 pixels, by a palette of the alpha over 565 at
 * [0xa0b020] (sub_50a830's). */
nfs::u32 frontend::watb::blendSpanPalette565(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    u32 from = argEax, to = argEdx;
    u32 count = argEbx;
    u32 result = argEax, lastEbx = argEbx, leftEdx = argEdx;
    do
    {
        const u32 scaled = u32(guest.byte(from)) * 4;
        result = scaled;
        const u32 argb = guest.dword(guest.dword(0xa0b020) + scaled);
        lastEbx = argb;
        ++from;
        if (argb >= kVisibleFrom)
        {
            if (argb < kOpaqueFrom)
            {
                const u32 pixel = guest.word(to);
                const u32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
                const std::uint64_t product = std::uint64_t(spread) * ((~argb) >> 24);
                const u32 low = u32(product), high = u32(product >> 32);
                leftEdx = (high << 8) & 0xf800;
                result = (low >> 16) & 0x7e0;
                lastEbx = argb + leftEdx + result + ((low >> 8) & 0x1f);
            }
            guest.setWord(to, std::uint16_t(lastEbx));
        }
        to += 2;
    } while (--count != 0);
    return result;
}

nfs::u32 frontend::watbBlendSpanPalette565(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50daa0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::blendSpanPalette565(app, ts, args.eax, args.edx, args.ebx);
    });
}

/* sub_50db30: a shape in alpha by movdfl with the table at 0x5688a8: an 8-bit one on a
 * 16-bit window through its palette and sub_50daa0. */
nfs::u32 frontend::watb::drawAlphaShape(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x410);
    constexpr u32 kBlitter16 = 0x5688c0, kType = 0x5688c8;
    const u32 frame = guest.frame();
    const u32 shape = argEax, x = argEdx, y = argEbx;
    guest.setDword(kBlitter16, 0);
    guest.setDword(kType, 0x7d);
    if (shapeDepth(guest, shape) == 8 && guest.byte(kWindowDepth) == 0x10)
    {
        guest.setDword(0xa0b020, guest::direct<0x50a830, clut16aPointer>(app, ts, shape, frame));
        guest.setDword(kBlitter16, 0x50daa0);
        guest.setDword(kType, 0x7b);
    }
    return fe::direct<0x50b0b0, moveShape>(app, ts, shape, x, y, u32(0x5688a8));
}

nfs::u32 frontend::watbDrawAlphaShape(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50db30", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::drawAlphaShape(app, ts, args.eax, args.edx, args.ebx); });
}

/* sub_50e5a0, qfetch: under the queue's lock, the first element edx's test says yes to,
 * taken off the queue. */
nfs::u32 frontend::watb::queueFetch(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    const u32 queue = argEax, test = argEdx, argument = argEbx;
    const u32 lock = guest.dword(queue + 0x18);
    fe::enterLock(guest, ts, lock);
    if (guest.dword(queue + 0x18) == 0)
    {
        reportError(guest, 0x54fbb8, 0x54fc94, 0x36a, args({}, { 0x54fbcc, 0x54fc94, queue }));
    }
    u32 element = guest.dword(queue + 8);
    while (element != 0 && test != 0)
    {
        // The element in ebx too, as the code has it there, for a test that would read it.
        if (fe::pointerCall(guest, test, args({ element, argument, element })) != 0)
            break;
        element = guest.dword(element);
    }
    if (element != 0)
    {
        if (fe::direct<0x50dc20, ea::queueUnlinked>(app, ts, queue, element) == 0)
        {
            reportError(guest, 0x54fbb8, 0x54fc94, 0x373, args({}, { 0x54fc9c }));
            element = 0;
        }
    }
    fe::leaveLock(guest, ts, guest.dword(queue + 0x18));
    return element;
}

nfs::u32 frontend::watbQueueFetch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_50e5a0", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::queueFetch(app, ts, args.eax, args.edx, args.ebx); });
}

// ---------------------------------------------------------------------------
// The runtime: the streams, the command line
// ---------------------------------------------------------------------------

namespace
{
constexpr u32 kIob = 0x566f58, kFileBytes = 0x1a;
constexpr u32 kOpenStreams = 0x9f38a0, kClosedStreams = 0x9f38a4;
constexpr u32 kNmalloc = 0x4f7900, kNfree = 0x4f79f0;
constexpr u32 kLinkBytes = 0x1d;
}

/* sub_509350: the closed streams' links freed.  No result: what _nfree leaves -- none, 0 here --
 * or with none to free eax as it came, which its thunk does. */
nfs::u32 frontend::watb::freeClosedStreams(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    while (guest.dword(kClosedStreams) != 0)
    {
        const u32 link = guest.dword(kClosedStreams);
        const u32 next = guest.dword(link);
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.ts(), link);
        guest.setDword(kClosedStreams, next);
        if (next == 0)
            break;
    }
    return 0;
}

nfs::u32 frontend::watbFreeClosedStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_509350", 0, 0);
    const u32 previous = args.eax;
    const bool none = guest::at(app, kClosedStreams) == 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 left = watb::freeClosedStreams(app, ts);
        return none ? previous : left;
    });
}

/* sub_515740, __InitFiles: a link for each FILE of __iob in use, stderr's flags first. */
nfs::u32 frontend::watb::initStreams(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kStderrFlags = 0x566f99;
    const std::uint8_t flags = guest.byte(kStderrFlags) & 0xf8;
    guest.setByte(kStderrFlags, flags | 4);
    u32 result = (argEax & 0xffff00ff) | (u32(flags) << 8);
    u32 file = kIob;
    u32 inUse = guest.dword(kIob + 0xc);
    while (inUse != 0)
    {
        u32 link = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), kLinkBytes);
        if (link == 0)
        {
            link = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), kLinkBytes);
            if (link == 0)
                fe::direct<0x511d0c, watcom::clib::fatalRuntimeError>(guest.app(), guest.ts(), 0x55057cu, 1u);
        }
        guest.setDword(link + 4, file);
        guest.setDword(link, guest.dword(kOpenStreams));
        guest.setDword(file + 8, link);
        guest.setDword(link + 8, 0);
        guest.setByte(guest.dword(file + 8) + 0x14, 0);
        result = guest.dword(file + 8);
        guest.setDword(result + 0xc, 0);
        guest.setDword(kOpenStreams, link);
        inUse = guest.dword(file + 0x26);
        file += kFileBytes;
    }
    guest.setDword(kClosedStreams, 0);
    return result;
}

nfs::u32 frontend::watbInitStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_515740", 0, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::initStreams(app, ts, args.eax); });
}

/* sub_5157e0, __full_io_exit: every stream closed, then the links freed by sub_509350, which the
 * code jumps to and which hands back what sub_5157f4 left in eax, the streams closed, when there
 * are none to free.  Here sub_509350 is called, and the thunk pops the return address. */
nfs::u32 frontend::watb::closeStreams(App* app, ThreadState& ts)
{
    try
    {
        const u32 closed = guest::direct<0x5157f4, closeStreamsFrom>(app, ts, 0);
        if (ts.terminate)
            return 0;
        const bool none = guest::at(app, kClosedStreams) == 0;
        const u32 freed = guest::direct<0x509350, freeClosedStreams>(app, ts);
        return none ? closed : freed;
    }
    catch (const fe::Terminated&)
    {
        return 0;
    }
}

nfs::u32 frontend::watbCloseStreams(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 result = watb::closeStreams(app, ts);
    if (ts.terminate)
        return args.eax;
    args.eax = result;
    ts.sp += 4;
    return args.eax;
}

/* sub_5157f4: the open streams from the eax'th of __iob on shut down; __iob's first
 * three only if they are not those. */
nfs::u32 frontend::watb::closeStreamsFrom(App* app, ThreadState& ts, u32 argEax)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x10);
    constexpr u32 kIobStandardEnd = 0x566fa6;
    const u32 first = kIob + argEax * kFileBytes;
    u32 closed = 0;
    u32 link = guest.dword(kOpenStreams);
    while (link != 0)
    {
        const u32 next = guest.dword(link);
        const u32 file = guest.dword(link + 4);
        const std::uint8_t flags = guest.byte(file + 0xd);
        u32 close = 1;
        bool skip = false;
        if ((flags & 0x40) == 0 && (flags & 8) == 0)
        {
            if (file < first)
                skip = true;
            else if (file < kIobStandardEnd)
                close = 0;
        }
        if (!skip)
        {
            fe::direct<0x4ee144, watcom::clib::streamShutdown>(guest.app(), guest.ts(), file, close);
            ++closed;
        }
        link = next;
    }
    return closed;
}

nfs::u32 frontend::watbCloseStreamsFrom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5157f4", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::closeStreamsFrom(app, ts, args.eax); });
}

/* sub_515900: the command line at [0x5677fc] split into argv: counted, copied, split again. */
nfs::u32 frontend::watb::buildArgv(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x24);
    constexpr u32 kCommandLine = 0x5677fc, kProgramName = 0x567800;
    constexpr u32 kArgv = 0x9f38c0, kArgc = 0x9f38bc;
    const u32 frame = guest.frame();
    const u32 words = guest::direct<0x5159e4, splitCommandLine>(app, ts, guest.dword(kCommandLine), u32(0), frame) + 1;
    const u32 length = guest.dword(frame) - guest.dword(kCommandLine);
    const u32 textBytes = (length + 4) & ~u32(3);
    const u32 argvBytes = words << 2;
    const u32 memory = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.ts(), (argvBytes + 4 + textBytes + 3) & ~u32(3));
    u32 count = 0;
    if (memory != 0)
    {
        guest.setDword(kArgv, memory + textBytes);
        copyRep(guest, memory, guest.dword(kCommandLine), length + 1);
        const u32 argv = guest.dword(kArgv);
        guest.setDword(argv, guest.dword(kProgramName));
        guest::direct<0x5159e4, splitCommandLine>(app, ts, memory, argv + 4, frame);
        guest.setDword(guest.dword(kArgv) + argvBytes, 0);
        count = words;
    }
    else
        guest.setDword(kArgv, 0);
    guest.setDword(kArgc, count);
    guest.setDword(0xa18210, guest.dword(kArgc));
    guest.setDword(0xa18214, guest.dword(kArgv));
    guest.setDword(0x9f38c4, guest.dword(0xa18210));
    const u32 last = guest.dword(0xa18214);
    guest.setDword(0x9f38c8, last);
    return last;
}

nfs::u32 frontend::watbBuildArgv(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_515900", 0, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::buildArgv(app, ts); });
}

/* sub_5159e4: the command line at eax split into words: their starts at edx if it is
 * not 0, each made a string in place; the end's place at [ebx]; how many.  A word is
 * quoted or not, with Microsoft's backslashes, or Watcom's where [0xa18218] says. */
nfs::u32 frontend::watb::splitCommandLine(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    Guest guest(app, ts);
    constexpr u32 kBackslashStyle = 0xa18218;
    u32 text = argEax;
    const u32 words = argEdx, endPlace = argEbx;
    const u32 style = guest.dword(kBackslashStyle);
    u32 count = 0;
    u32 out = argEbx;
    for (;;)
    {
        while (guest.byte(text) == ' ' || guest.byte(text) == '\t')
            ++text;
        if (guest.byte(text) == 0)
            break;
        std::uint8_t quote = 0;   // 1 opened by the first character, 2 by one later on
        if (guest.byte(text) == '"')
        {
            quote = 1;
            ++text;
        }
        const u32 start = text;
        out = text;
        for (;;)
        {
            if (guest.byte(text) == '"')
            {
                if (style == 0)
                {
                    ++text;
                    quote = quote == 0 ? 2 : 0;
                    continue;
                }
                if (quote == 1)
                    break;
            }
            const std::uint8_t c = guest.byte(text);
            if ((c == ' ' || c == '\t') && quote == 0)
                break;
            if (c == 0)
                break;
            if (c == '\\')
            {
                if (style == 0)
                {
                    if (guest.byte(text + 1) == '"')
                    {
                        const std::uint8_t before = guest.byte(text - 1);
                        ++text;
                        if (before == '\\')
                            continue;
                    }
                }
                else if (quote == 1)
                {
                    const std::uint8_t after = guest.byte(text + 1);
                    if (after == '"' || after == '\\')
                        ++text;
                }
                else if (guest.byte(text + 1) == '"')
                    ++text;
            }
            if (words != 0)
            {
                guest.setByte(out, guest.byte(text));
                ++out;
            }
            ++text;
        }
        const std::uint8_t ended = guest.byte(text);
        ++count;
        if (words != 0)
        {
            guest.setDword(words + (count - 1) * 4, start);
            guest.setByte(out, 0);
        }
        else
            out = (out & 0xffffff00) | ended;
        if (ended == 0)
            break;
        ++text;
    }
    guest.setDword(endPlace, text);
    guest.setDword(kBackslashStyle, style);
    return count;
}

nfs::u32 frontend::watbSplitCommandLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5159e4", 0, 0, kLeavesEbxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return watb::splitCommandLine(app, ts, args.eax, args.edx, args.ebx); });
}

// ---------------------------------------------------------------------------
// __LDcvt: a long double's digits, for printf's %e, %f and %g (0x5167b3).  Its parts
// lean on the frame ebp holds, and are entered at their own addresses too: what its code
// keeps in registers across them (ebp the frame, esi the CVT, ecx the decimal point, edi
// what is yet to be written as digits) goes from part to part as a LongDouble.
// ---------------------------------------------------------------------------

namespace
{

constexpr u32 kCvtDigits = 0, kCvtScale = 4, kCvtFlags = 8, kCvtExpWidth = 0x10, kCvtSign = 0x14;
constexpr u32 kCvtN1 = 0x18, kCvtNZ1 = 0x1c, kCvtN2 = 0x20, kCvtNZ2 = 0x24, kCvtExtra = 0x28;

// Below ebp.
constexpr u32 kFrameControl = 0x14, kFrameWanted = 0x18, kFrameBuffer = 0x20;
constexpr u32 kFrameCount = 0x24, kFrameWrite = 0x28, kFrameRoundTo = 0x10;
constexpr u32 kFrameValue = 0x34, kFrameTemp = 0x4c, kFrameQuotient = 0x58, kFrameDigits = 0x98;

u32 popDword(Guest& guest, nfs::ThreadState& ts)
{
    const u32 value = guest.dword(ts.sp);
    ts.sp += 4;
    return value;
}

nfs::Float load80(nfs::App* app, u32 address)
{
    return nfs::Float(app->getMemory<nfs::Float80>(address));
}

void store80(nfs::App* app, u32 address, nfs::Float value)
{
    app->getMemory<nfs::Float80>(address) = nfs::Float(value);
}

/* 1e8, as the long double the code builds at [address]. */
void hundredMillion(Guest& guest, u32 address)
{
    guest.setWord(address + 8, 0x4019);
    guest.setDword(address + 4, 0xbebc2000);
    guest.setDword(address, 0);
}

/* fistp with the rounding towards zero its code sets round it: the integer, or fistp's integer
 * indefinite for a NaN or a value past it, as nfs::FloatEnv::toInteger stores them. */
u32 truncatedToInteger(nfs::Float value)
{
    const nfs::Float rounded = std::trunc(value);
    const nfs::Float lowest = nfs::Float(std::numeric_limits<s32>::min());
    if (!(rounded >= lowest && rounded < -lowest))
    {
        ++nfs::FloatEnv::s_indefinite;
        return u32(std::numeric_limits<s32>::min());
    }
    return u32(s32(rounded));
}

/* sub_516ab4: the two decimal digits of `value`'s ax at `at`, which moves past them; eax as the
 * code leaves it. */
u32 writeTwoDigits(Guest& guest, u32 value, u32& at)
{
    const std::uint8_t low = std::uint8_t(value), high = std::uint8_t(value >> 8);
    std::uint8_t tens, ones;
    if (low < 10)
    {
        tens = high;
        ones = low;
    }
    else
    {
        const std::uint16_t whole = std::uint16_t(value);
        tens = std::uint8_t(whole / 10);
        ones = std::uint8_t(whole % 10);
    }
    tens = std::uint8_t(tens + '0');
    ones = std::uint8_t(ones + '0');
    guest.setByte(at, tens);
    guest.setByte(at + 1, ones);
    at += 2;
    return (value & 0xffff0000) | (u32(ones) << 8) | tens;
}

/* sub_516a9d: four. */
u32 writeFourDigits(Guest& guest, u32 value, u32& at)
{
    writeTwoDigits(guest, value / 100, at);
    return writeTwoDigits(guest, value % 100, at);
}

/* sub_516a87: eight. */
u32 writeEightDigits(Guest& guest, u32 value, u32& at)
{
    writeFourDigits(guest, value / 10000, at);
    return writeFourDigits(guest, value % 10000, at);
}

/* What __LDcvt's code keeps in registers across its parts: its frame (ebp), the CVT (esi), where
 * the decimal point goes (ecx), and the digits yet to be written eight at a time (edi). */
struct LongDouble
{
    u32 frame;
    u32 cvt;
    u32 point;
    u32 pending;
};

/* sub_516be2's eax: the control word the code saved in its frame. */
u32 savedControl(Guest& guest, const LongDouble& state)
{
    return guest.word(state.frame - kFrameControl);
}

/* sub_516ae2: the digits rounded to what is asked, sub_516bf5 or sub_516e13 to lay them out; the
 * end's eax. */
u32 roundAndLayOut(Guest& guest, nfs::App* app, nfs::ThreadState& ts, LongDouble& state)
{
    const u32 frame = state.frame;
    const u32 cvt = state.cvt;
    u32 available = guest.dword(frame - kFrameCount);
    u32 digits = frame - kFrameDigits + 1;
    u32 point = state.point + 7;
    while (guest.byte(digits) == '0')
    {
        --available;
        --point;
        ++digits;
    }
    const std::uint8_t mode = guest.byte(cvt + kCvtFlags);
    u32 keep = guest.dword(cvt + kCvtDigits);
    if ((mode & 2) != 0)
    {
        point += guest.dword(cvt + kCvtScale);
        keep += point + 1;
    }
    else if ((mode & 1) != 0)
    {
        const u32 scale = guest.dword(cvt + kCvtScale);
        if (positive(scale))
            ++keep;
        else
            keep += scale;
        ++point;
        point -= guest.dword(cvt + kCvtScale);
    }
    if (s32(keep) >= 0)
    {
        if (s32(keep) > s32(available))
            keep = available;
        u32 most = 0xf;
        if ((guest.byte(cvt + kCvtFlags) & 0x20) != 0)
            most = 0x14;
        if ((guest.byte(cvt + kCvtFlags) & 0x40) != 0)
            most += most;
        if (s32(keep) > s32(most))
            keep = most + 1;
        std::uint8_t dropTo = '0';
        guest.setByte(frame - kFrameRoundTo, dropTo);
        if (s32(available) > s32(keep) && guest.byte(digits + keep) >= '5')
        {
            dropTo = '9';
            guest.setByte(frame - kFrameRoundTo, dropTo);
        }
        u32 place = keep;
        u32 at = keep + digits;
        std::uint8_t value;
        for (;;)
        {
            --place;
            --at;
            value = guest.byte(at);
            if (value != dropTo)
                break;
            --keep;
        }
        if (dropTo == '9')
            guest.setByte(at, std::uint8_t(value + 1));
        if (s32(place) < 0)
        {
            --digits;
            ++keep;
            ++point;
        }
    }
    if (!positive(keep))
    {
        keep = 1;
        point = 0;
        guest.setByte(frame - kFrameDigits, '0');
        digits = frame - kFrameDigits;
        guest.setDword(cvt + kCvtSign, 0);
    }
    state.pending = keep;
    const std::uint8_t flags = guest.byte(cvt + kCvtFlags);
    bool plain = true;
    if ((flags & 2) == 0)
    {
        plain = false;
        if ((flags & 4) != 0)
        {
            if (s32(point) >= -4 && s32(point) < s32(guest.dword(cvt + kCvtDigits)))
                plain = true;
            else
                plain = (guest.byte(cvt + kCvtFlags) & 8) != 0;
        }
    }
    const u32 buffer = guest.dword(frame - kFrameBuffer);
    if (plain)
        fe::direct<0x516bf5, frontend::watb::formatF>(app, ts, cvt, digits, keep, point, buffer);
    else
        fe::direct<0x516e13, frontend::watb::formatE>(app, ts, cvt, digits, keep, point, buffer);
    return savedControl(guest, state);
}

/* sub_5169ef: eight digits at a time while more are wanted and any are left; then the rounding
 * and the layout, and the end's eax.  The arithmetic at the x87's 64 bits, which __LDcvt sets for
 * it: a double's own sums and products, as the x87 emulation gave them with that word. */
u32 writeDigits(Guest& guest, nfs::App* app, nfs::ThreadState& ts, LongDouble& state)
{
    const u32 frame = state.frame;
    for (;;)
    {
        const u32 wanted = guest.dword(frame - kFrameWanted);
        if (!positive(wanted))
            break;
        const u32 more = wanted - 8;
        guest.setDword(frame - kFrameWanted, more);
        if (state.pending == 0)
        {
            if ((guest.word(frame - kFrameValue + 8) & 0x7fff) == 0)
                break;
            const u32 value = frame - kFrameValue;
            state.pending = truncatedToInteger(load80(app, value));
            if (positive(more))
            {
                const u32 temp = frame - 0x40;
                store80(app, temp, nfs::Float(s32(state.pending)));
                store80(app, value, load80(app, value) - load80(app, temp));
                hundredMillion(guest, temp);
                store80(app, value, load80(app, value) * load80(app, temp));
            }
        }
        const u32 count = guest.dword(frame - kFrameCount);
        u32 write = guest.dword(frame - kFrameWrite);
        {
            // The code pushes ecx and the count round sub_516a87: their room kept, nothing written.
            const fe::Frame pushed(guest, ts, 8);
            fe::direct<0x516a87, frontend::watb::eightDigits>(app, ts, state.pending, write);
        }
        write += 8;  // where sub_516a87 leaves ebx: past the eight
        guest.setByte(write, 0);
        guest.setDword(frame - kFrameWrite, write);
        guest.setDword(frame - kFrameCount, count + 8);
        state.pending = 0;
    }
    return roundAndLayOut(guest, app, ts, state);
}

}

/* sub_5167b3, __LDcvt: the long double at `argEax` into the CVT at `argEdx`, its digits for the
 * layouts in the buffer at `argEbx`.  Its frame as the code's: ebp pushed (the frame from there),
 * then ecx, esi and edi -- their room kept, nothing written -- and its locals.  The control word it
 * saves there and hands back at its end is the port's x87's, 0x7f (nfs::FloatEnv::init); the 64 bits
 * it asks for meanwhile are a double's own arithmetic. */
nfs::u32 frontend::watb::longDoubleToDigits(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx)
{
    constexpr u32 kScale = 0x516759;
    constexpr u32 kControlWord = 0x7f;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4 + 3 * 4 + 0x8c);
    LongDouble state{ guest.frame() + 3 * 4 + 0x8c, argEdx, 0, 0 };
    const u32 frame = state.frame;
    const u32 from = argEax;
    const u32 cvt = state.cvt;
    guest.setDword(frame - kFrameBuffer, argEbx);
    guest.setDword(frame - kFrameControl, kControlWord);

    guest.setDword(cvt + kCvtSign, 0);
    const u32 value = frame - kFrameValue;
    guest.setWord(value + 8, guest.word(from + 8));
    guest.setDword(value + 4, guest.dword(from + 4));
    guest.setDword(value, guest.dword(from));
    if ((guest.byte(value + 9) & 0x80) != 0)
        guest.setDword(cvt + kCvtSign, 0xffffffff);
    guest.setByte(value + 9, guest.byte(value + 9) & 0x7f);
    guest.setDword(cvt + kCvtNZ1, 0);
    guest.setDword(cvt + kCvtN2, 0);
    guest.setDword(cvt + kCvtNZ2, 0);
    guest.setDword(cvt + kCvtExtra, 0);
    guest.setDword(cvt + kCvtN1, 0);

    const u32 kind = fe::direct<0x52448c, frontend::menz::longDoubleKind>(app, ts, value);
    const auto writeWord = [&](char a, char b, char c) {
        const u32 buffer = guest.dword(frame - kFrameBuffer);
        guest.setByte(buffer, std::uint8_t(a));
        guest.setByte(buffer + 1, std::uint8_t(b));
        guest.setByte(buffer + 2, std::uint8_t(c));
        guest.setByte(buffer + 3, 0);
        guest.setDword(cvt + kCvtNZ1, 3);
        return savedControl(guest, state);
    };
    switch (kind)
    {
    case 0:
    case 4:
        // Zero.
        guest.setDword(cvt + kCvtSign, 0);
        state.point = 0;
        break;
    case 2:
        return writeWord('n', 'a', 'n');
    case 3:
        return writeWord('i', 'n', 'f');
    case 1:
    {
        // Scaled by powers of ten to about eight digits before the point.
        const std::uint16_t exponent = guest.word(value + 8);
        const s32 guess = s32(u32(exponent) - 0x3ffe) * 0x7597 / 0x186a0;
        state.point = u32(guess - 4);
        if (state.point == 0)
            break;
        if (s32(state.point) < 0)
        {
            state.point = u32(0) - ((u32(0) - state.point + 3) & ~u32(3));
            fe::direct<kScale, watcom::clib::ldScale10x>(app, ts, value, u32(0) - state.point);
            break;
        }
        const u32 high = guest.dword(value + 4);
        if (exponent < 0x4019 || (exponent == 0x4019 && high < 0xbebc2000))
        {
            state.point = 0;
            break;
        }
        const bool below = exponent < 0x4034
            || (exponent == 0x4034
                && (high < 0x8e1bc9bf || (high == 0x8e1bc9bf && guest.dword(value) < 0x4000000)));
        if (below)
        {
            // Under 1e16: the first eight digits by a division, the rest by what is left.
            const u32 temp = frame - kFrameTemp, quotient = frame - kFrameQuotient;
            hundredMillion(guest, temp);
            store80(app, quotient, load80(app, value) / load80(app, temp));
            state.pending = truncatedToInteger(load80(app, quotient));
            store80(app, quotient, nfs::Float(s32(state.pending)));
            store80(app, temp, load80(app, quotient) * load80(app, temp));
            store80(app, value, load80(app, value) - load80(app, temp));
            state.point = 8;
            break;
        }
        state.point &= ~u32(3);
        fe::direct<kScale, watcom::clib::ldScale10x>(app, ts, value, u32(0) - state.point);
        break;
    }
    default:
        // No class sub_52448c gives: the code goes on with ecx as it came, 0 here.
        break;
    }

    u32 wanted;
    const std::uint8_t mode = guest.byte(cvt + kCvtFlags);
    if ((mode & 2) != 0)
    {
        wanted = guest.dword(cvt + kCvtDigits) + state.point + 0xa;
        const u32 scale = guest.dword(cvt + kCvtScale);
        if (positive(scale))
            wanted += scale;
    }
    else
        wanted = guest.dword(cvt + kCvtDigits) + 7;
    u32 most = (guest.byte(cvt + kCvtFlags) & 0x20) != 0 ? 0x14 : 0xf;
    if ((guest.byte(cvt + kCvtFlags) & 0x40) != 0)
        most += most;
    most += 4;
    if (s32(most) < s32(wanted))
        wanted = most;
    guest.setDword(frame - kFrameWanted, wanted);
    guest.setByte(frame - kFrameDigits, '0');
    guest.setByte(frame - kFrameDigits + 1, 0);
    guest.setDword(frame - kFrameWrite, frame - kFrameDigits + 1);
    guest.setDword(frame - kFrameCount, 0);
    return writeDigits(guest, app, ts, state);
}

/* The code keeps ecx, esi, edi and ebp (it pushes them) and the control word (its end loads the
 * saved one again). */
nfs::u32 frontend::watbLongDoubleToDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_5167b3", 0, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::longDoubleToDigits(app, ts, args.eax, args.edx, args.ebx);
    });
}

/* The parts of it: entered with its frame (ebp) and its registers, each runs on to the end
 * (sub_516be2, watbEndLongDouble). */
nfs::u32 frontend::watbDigitsLoop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Guest guest(app, ts);
    try
    {
        LongDouble state{ args.ebp, args.esi, args.ecx, args.edi };
        writeDigits(guest, app, ts, state);
        frontend::watbEndLongDouble(app, ts, args);
    }
    catch (const fe::Terminated&)
    {
    }
    return args.eax;
}

nfs::u32 frontend::watbAfterEightDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Guest guest(app, ts);
    try
    {
        const u32 count = popDword(guest, ts);
        args.ecx = popDword(guest, ts);
        guest.setByte(args.ebx, 0);
        guest.setDword(args.ebp - kFrameWrite, args.ebx);
        guest.setDword(args.ebp - kFrameCount, count + 8);
        LongDouble state{ args.ebp, args.esi, args.ecx, 0 };
        writeDigits(guest, app, ts, state);
        frontend::watbEndLongDouble(app, ts, args);
    }
    catch (const fe::Terminated&)
    {
    }
    return args.eax;
}

nfs::u32 frontend::watbRoundDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Guest guest(app, ts);
    try
    {
        LongDouble state{ args.ebp, args.esi, args.ecx, args.edi };
        roundAndLayOut(guest, app, ts, state);
        frontend::watbEndLongDouble(app, ts, args);
    }
    catch (const fe::Terminated&)
    {
    }
    return args.eax;
}

/* sub_516be2: the control word as it was, the registers popped, and out. */
nfs::u32 frontend::watbEndLongDouble(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Guest guest(app, ts);
    const u32 word = guest.word(args.ebp - kFrameControl);
    ts.fenv.setControl(nfs::u16(word));
    args.eax = word;
    ts.sp = args.ebp - 0xc;
    args.edi = popDword(guest, ts);
    args.esi = popDword(guest, ts);
    args.ecx = popDword(guest, ts);
    args.ebp = popDword(guest, ts);
    ts.sp += 4;
    return args.eax;
}

/* sub_516a87, sub_516a9d, sub_516ab4: `value`'s last eight, four and two decimal digits at `at`;
 * eax as the code leaves it.  Their thunks leave ebx past the digits, where the code does. */
nfs::u32 frontend::watb::eightDigits(App* app, ThreadState& ts, u32 value, u32 at)
{
    Guest guest(app, ts);
    return writeEightDigits(guest, value, at);
}

nfs::u32 frontend::watbEightDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::eightDigits(app, ts, args.eax, args.ebx);
    args.ebx += 8;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::fourDigits(App* app, ThreadState& ts, u32 value, u32 at)
{
    Guest guest(app, ts);
    return writeFourDigits(guest, value, at);
}

nfs::u32 frontend::watbFourDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::fourDigits(app, ts, args.eax, args.ebx);
    args.ebx += 4;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 frontend::watb::twoDigits(App* app, ThreadState& ts, u32 value, u32 at)
{
    Guest guest(app, ts);
    return writeTwoDigits(guest, value, at);
}

nfs::u32 frontend::watbTwoDigits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = watb::twoDigits(app, ts, args.eax, args.ebx);
    args.ebx += 2;
    ts.sp += 4;
    return args.eax;
}

/* sub_516bf5: %f's layout in the buffer on the stack: digits at edx, ebx of them, the
 * point after ecx + 1, what the CVT at eax asks; how many zeros go where, in the CVT. */
nfs::u32 frontend::watb::formatF(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                   u32 bufferWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x1c);
    const u32 cvt = argEax, digits = argEdx;
    u32 count = argEbx;
    u32 point = argEcx + 1;
    const u32 buffer = bufferWord;
    u32 precision = guest.dword(cvt + kCvtDigits);
    u32 out = 0;
    u32 leftEax = 0;
    const std::uint8_t flags = guest.byte(cvt + kCvtFlags);
    if ((flags & 4) != 0)
    {
        if (s32(count) < s32(precision) && (guest.byte(cvt + kCvtFlags) & 0x10) == 0)
            precision = count;
        precision -= point;
        if (s32(precision) < 0)
            precision = 0;
    }
    if (!positive(point))
    {
        if ((guest.byte(cvt + kCvtFlags) & 8) == 0)
        {
            guest.setByte(buffer + out, '0');
            ++out;
            if (positive(precision) || (guest.byte(cvt + kCvtFlags) & 0x10) != 0)
            {
                guest.setByte(buffer + out, '.');
                ++out;
            }
        }
        guest.setDword(cvt + kCvtNZ1, out);
        if (s32(u32(0) - point) > s32(precision))
            point = u32(0) - precision;
        guest.setDword(cvt + kCvtN1, point);
        guest.setDword(cvt + kCvtN2, u32(0) - point);
        precision += point;
        if (s32(count) > s32(precision))
            count = precision;
        copyRep(guest, buffer + out, digits, count);
        leftEax = count;
        guest.setDword(cvt + kCvtNZ2, count);
        out += count;
        guest.setDword(cvt + kCvtExtra, precision - count);
    }
    else if (s32(count) < s32(point))
    {
        copyRep(guest, buffer, digits, count);
        leftEax = count;
        guest.setDword(cvt + kCvtNZ1, count);
        out = count;
        guest.setDword(cvt + kCvtN2, point - count);
        const std::uint8_t now = guest.byte(cvt + kCvtFlags);
        guest.setDword(cvt + kCvtN1, point);
        if ((now & 8) == 0 && (positive(precision) || (now & 0x10) != 0))
        {
            guest.setByte(buffer + out, '.');
            ++out;
            guest.setDword(cvt + kCvtNZ2, 1);
        }
        guest.setDword(cvt + kCvtExtra, precision);
    }
    else
    {
        copyRep(guest, buffer, digits, point);
        const std::uint8_t now = guest.byte(cvt + kCvtFlags);
        out = point;
        count -= point;
        guest.setDword(cvt + kCvtN1, point);
        if ((now & 8) != 0)
        {
            if (guest.byte(buffer) == '0')
                guest.setDword(cvt + kCvtN1, 0);
        }
        else if (positive(precision) || (now & 0x10) != 0)
        {
            guest.setByte(buffer + out, '.');
            ++out;
        }
        if (s32(count) > s32(precision))
            count = precision;
        copyRep(guest, buffer + out, digits + point, count);
        leftEax = count;
        out += count;
        guest.setDword(cvt + kCvtNZ1, out);
        guest.setDword(cvt + kCvtN2, precision - count);
    }
    guest.setByte(buffer + out, 0);
    return leftEax;
}

nfs::u32 frontend::watbFormatF(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_516bf5", 4, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::formatF(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4));
    });
}

/* sub_516dfa: the end of sub_516bf5, which sub_516e13 jumps to. */
nfs::u32 frontend::watbEndFormat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    Guest guest(app, ts);
    ts.sp = args.ebp - 8;
    args.edi = popDword(guest, ts);
    args.esi = popDword(guest, ts);
    args.ebp = popDword(guest, ts);
    ts.sp += 4 + 4;
    return args.eax;
}

/* sub_516e13: %e's layout: digits, zeros, the point, more, the letter, the exponent's
 * sign and its digits, as few as the CVT's +0x10 asks and it needs. */
nfs::u32 frontend::watb::formatE(App* app, ThreadState& ts, u32 argEax, u32 argEdx, u32 argEbx, u32 argEcx,
                                   u32 bufferWord)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x28);
    constexpr u32 kMemset = 0x4e0640;
    const u32 cvt = argEax;
    u32 digits = argEdx, count = argEbx, exponent = argEcx;
    const u32 buffer = bufferWord;
    u32 precision = guest.dword(cvt + kCvtDigits);
    u32 out = 0;
    const u32 scale = guest.dword(cvt + kCvtScale);
    if (!positive(scale))
        precision += scale;
    else
    {
        precision = precision - scale + 1;
    }
    if ((guest.byte(cvt + kCvtFlags) & 4) != 0)
    {
        if (s32(count) < s32(precision))
            precision = count;
        --precision;
        if (s32(precision) < 0)
            precision = 0;
    }
    if (!positive(guest.dword(cvt + kCvtScale)))
    {
        guest.setByte(buffer, '0');
        out = 1;
    }
    else
    {
        const u32 first = guest.dword(cvt + kCvtScale);
        const u32 taken = s32(first) > s32(count) ? count : first;
        copyRep(guest, buffer, digits, taken);
        digits += taken;
        count -= taken;
        out = taken;
        const u32 scaleNow = guest.dword(cvt + kCvtScale);
        if (s32(taken) < s32(scaleNow))
        {
            const u32 zeros = scaleNow - taken;
            fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), buffer + out, '0', zeros);
            out += zeros;
        }
    }
    guest.setDword(cvt + kCvtN1, out);
    if ((guest.byte(cvt + kCvtFlags) & 8) == 0 && (positive(precision) || (guest.byte(cvt + kCvtFlags) & 0x10) != 0))
    {
        guest.setByte(buffer + out, '.');
        ++out;
    }
    const u32 scaleNow = guest.dword(cvt + kCvtScale);
    if (s32(scaleNow) < 0)
    {
        const u32 zeros = u32(0) - scaleNow;
        fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), buffer + out, '0', zeros);
        out += zeros;
    }
    if (positive(precision))
    {
        if (s32(precision) < s32(count))
            count = precision;
        if (count != 0)
        {
            copyRep(guest, buffer + out, digits, count);
            out += count;
        }
        guest.setDword(cvt + kCvtNZ1, out);
        guest.setDword(cvt + kCvtN2, precision - count);
    }
    if (guest.dword(cvt + 0xc) != 0)
    {
        guest.setByte(buffer + out, guest.byte(cvt + 0xc));
        ++out;
    }
    if (s32(exponent) >= 0)
        guest.setByte(buffer + out, '+');
    else
    {
        exponent = u32(0) - exponent;
        guest.setByte(buffer + out, '-');
    }
    ++out;

    u32 width = guest.dword(cvt + kCvtExpWidth);
    switch (width)
    {
    case 0:
        width = s32(exponent) >= 1000 ? 4 : 3;
        break;
    case 1:
    case 2:
    case 3:
        if (width == 1 && s32(exponent) >= 10)
            width = 2;
        if (width <= 2 && s32(exponent) >= 100)
            width = 3;
        if (s32(exponent) >= 1000)
            width = 4;
        break;
    default:
        break;
    }
    guest.setDword(cvt + kCvtExpWidth, width);
    const auto digit = [&](u32 unit) {
        u32 quotient = 0;
        if (s32(exponent) >= s32(unit))
        {
            quotient = u32(s32(exponent) / s32(unit));
            exponent -= quotient * unit;
        }
        guest.setByte(buffer + out, std::uint8_t(quotient + '0'));
        ++out;
    };
    if (s32(width) >= 4)
        digit(1000);
    if (s32(width) >= 3)
        digit(100);
    if (s32(width) >= 2)
    {
        digit(10);
    }
    guest.setByte(buffer + out, std::uint8_t(exponent + '0'));
    ++out;
    const u32 written = out - guest.dword(cvt + kCvtNZ1);
    guest.setDword(cvt + kCvtNZ2, written);
    guest.setByte(buffer + out, 0);
    return buffer + out;
}

nfs::u32 frontend::watbFormatE(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site = makeSite("sub_516e13", 4, 0, kLeavesEbxEcxEdx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return watb::formatE(app, ts, args.eax, args.edx, args.ebx, args.ecx, guest::at(app, ts.sp + 4));
    });
}

}
