#include "native_frontend_watb.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_rendf.h"
#include "native_frontend_watc.h"
#include "native_windows.h"
#include "host_threads.h"
#include <nfs3hp.h>
#include <winapi/kernel32.h>

#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax (as
 * tools/frontend_checks/specs_from_exe.py reads it): the registers left as the call by address
 * leaves them -- `words` in eax, edx, ebx and ecx as that call loads them, the first `Taken` of
 * them the core's arguments -- and its result in eax, which the call by address leaves there. */
template <x86::reg32 Address, auto Core, std::size_t... I>
auto directOf(win32::WinApplication* app, x86::CPU& cpu, const x86::reg32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, cpu, words[I]...);
}

template <x86::reg32 Address, auto Core, std::size_t Taken, typename... Words>
x86::reg32 directKept(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    const x86::reg32 all[] = { x86::reg32(words)..., 0 };
    x86::reg32* const into[] = { &cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 0; i < sizeof...(Words) && i < 4; ++i)
        *into[i] = all[i];
    const auto result = directOf<Address, Core>(app, cpu, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, x86::reg32>)
        cpu.eax = result;
    else
        cpu.eax = *result;
    return cpu.eax;
}

}  // namespace
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
 * The state stays in guest memory.  These run under other code that may go on with what
 * a register holds, so what each leaves in the registers it does not save is left as
 * its code leaves it: that is why the converters' last values appear at their ends.
 *
 * Plan step 5: each function a typed core in frontend::watb (native_frontend_watb.h) that takes
 * its arguments into the registers its code reads them from, in a frame of its own (fe::Frame:
 * what it calls runs at the stack addresses it did), and an adapter under the game's name for the
 * calls by address.  A core of this file calls another as C++ (guest::direct) where nothing reads
 * a register the callee leaves, and the typed cores of other files too (fe::direct); the rest of
 * the game still by address, as are the jumps to other functions (memmove, the handles', the
 * streams') and the entries into the middle of __LDcvt and %f's layout (sub_5169ef, sub_516acc,
 * sub_516ae2, sub_516be2, sub_516dfa), which work on their function's frame and registers and have
 * no core. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::reg32;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

constexpr std::uint8_t kLeavesEbx = fe::kRestoreAll & ~fe::kRestoreEbx;
constexpr std::uint8_t kLeavesEdx = fe::kRestoreAll & ~fe::kRestoreEdx;
constexpr std::uint8_t kLeavesEbxEdx = fe::kRestoreAll & ~(fe::kRestoreEbx | fe::kRestoreEdx);
constexpr std::uint8_t kLeavesEbxEcxEdx = fe::kRestoreAll & ~(fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx);

fe::Site makeSite(const char* name, reg32 stackBytes, reg32 frameBytes, std::uint8_t restores = fe::kRestoreAll)
{
    fe::Site site{};
    site.name = name;
    site.stackBytes = stackBytes;
    site.frameBytes = frameBytes;
    site.restores = restores;
    return site;
}

reg32 sar(reg32 value, unsigned bits)
{
    return reg32(sreg32(value) >> (bits & 31));
}

bool positive(reg32 value)
{
    return sreg32(value) > 0;
}

// ---------------------------------------------------------------------------
// The library's data
// ---------------------------------------------------------------------------

// The library's error report: its printf to the log, and where it is told it was called from.
constexpr reg32 kLog = 0x401010;
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;

constexpr reg32 kMemmove = 0x4ea4f0;
constexpr reg32 kMemoryPool = 0x5643f4;
constexpr reg32 kFillZero = 0x4e070c;

// The current window.
constexpr reg32 kClipLeft = 0x565000, kClipTop = 0x565004, kClipRight = 0x565008, kClipBottom = 0x56500c;
constexpr reg32 kWindowDepth = 0x565010;      // byte, bits a pixel
constexpr reg32 kWindowType = 0x565011;       // byte, a shape type
constexpr reg32 kWindowLocked = 0x565012;     // byte
constexpr reg32 kWindowPixels = 0x565014;
constexpr reg32 kWindowStride = 0x56501c;
constexpr reg32 kRowOffsets = 0x565020, kColumnOffsets = 0x565024;
constexpr reg32 kWindowPolled = 0x565028;

constexpr reg32 kDepthOfType = 0x566d54;      // bytes, by a type's low 7 bits
constexpr reg32 kInverseTable = 0x5650c0;     // a 15-bit colour's nearest index, or 0

// A shape: its first dword the type in the low byte and the distance to the next record
// above it, then words: width at +4, height at +6, and its pixels from +0x10.
constexpr reg32 kShapePixels = 0x10;
constexpr reg32 kShapeWidth = 2, kShapeHeight = 4;   // dwords whose upper half is it

reg32 shapeDepth(Guest& guest, reg32 shape)
{
    return guest.byte(kDepthOfType + (guest.dword(shape) & 0x7f));
}

reg32 roundedDepth(reg32 depth)
{
    return (depth + 1) & ~reg32(1);
}

void reportError(Guest& guest, reg32 file, reg32 function, reg32 line, const fe::Arguments& arguments)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    // Error as C++: the registers as the code loads them for it, which it keeps; its format and words
    // pushed.
    x86::CPU& cpu = guest.cpu();
    x86::reg32* const into[] = { &cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    guest::Words words({});
    for (std::size_t i = 1; i < arguments.stackCount; ++i)
        words.word[words.count++] = arguments.stack[i];
    fe::direct<0x401010, ea::error>(guest.app(), cpu, arguments.stack[0], words);
}

/* The same by address where what Error leaves in eax is handed back, which ea::error does not give. */
void reportErrorLeft(Guest& guest, reg32 file, reg32 function, reg32 line, const fe::Arguments& arguments)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    guest.call(kLog, arguments);
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

/* rep movsd and rep movsb, as Watcom inlines memcpy. */
void copyRep(Guest& guest, reg32 to, reg32 from, reg32 bytes)
{
    reg32 dwords = bytes >> 2;
    while (dwords--)
    {
        guest.setDword(to, guest.dword(from));
        to += 4;
        from += 4;
    }
    reg32 rest = bytes & 3;
    while (rest--)
        guest.setByte(to++, guest.byte(from++));
}

// ---------------------------------------------------------------------------
// The blends of an ARGB over a pixel, which two blitters and the spans share
// ---------------------------------------------------------------------------

/* What a blend leaves in the registers its code uses: the callers go on with them. */
struct Scratch
{
    reg32 eax, edx;
};

/* A 1555 pixel under an ARGB of less than full alpha: the ARGB with what shows through
 * added. */
reg32 blendOver1555(reg32 pixel, reg32 argb)
{
    const reg32 spread = ((pixel & 0x7c00) << 17) + ((pixel & 0x3e0) << 9) + (pixel & 0x1f);
    const reg32 inverse = (argb >> 24) ^ 0xff;
    const std::uint64_t product = std::uint64_t(spread) * inverse;
    const reg32 low = reg32(product), high = reg32(product >> 32);
    return argb + (high << 16) + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
}

/* The same over a 565 pixel. */
reg32 blendOver565(reg32 pixel, reg32 argb)
{
    const reg32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
    const reg32 inverse = (~argb) >> 24;
    const std::uint64_t product = std::uint64_t(spread) * inverse;
    const reg32 low = reg32(product), high = reg32(product >> 32);
    return argb + (high << 16) + ((low >> 5) & 0xff) + ((low >> 11) & 0xff00);
}

reg32 packTo1555(reg32 argb, Scratch& left)
{
    left.eax = (argb >> 3) & 0x1f;
    left.edx = (argb & 0xf800) >> 6;
    return ((argb & 0xf80000) >> 9) + left.eax + left.edx;
}

reg32 packTo565(reg32 argb, Scratch& left)
{
    left.eax = (argb >> 3) & 0x1f;
    left.edx = (argb & 0xfc00) >> 5;
    return ((argb & 0xf80000) >> 8) + left.eax + left.edx;
}

constexpr reg32 kOpaqueFrom = 0xfc000000;     // this much alpha and more replaces the pixel
constexpr reg32 kVisibleFrom = 0x10000000;    // less than this much leaves it

}

// ---------------------------------------------------------------------------
// Small callbacks
// ---------------------------------------------------------------------------

/* sub_4fb820: qfetch's test: whether the element's key at +4 is edx. */
x86::reg32 frontend::watb::keyMatches(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    return cpu.edx == guest.dword(cpu.eax + 4) ? 1 : 0;
}

void frontend::watbKeyMatches(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4fb820", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::keyMatches(app, cpu, cpu.eax, cpu.edx); });
}

/* sub_4fb82b: the end of sub_4fb820, which sub_4fb850 jumps to: no match. */
x86::reg32 frontend::watb::noMatch(App* app, CPU& cpu)
{
    return 0;
}

void frontend::watbNoMatch(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4fb82b", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::noMatch(app, cpu); });
}

/* sub_4fb830: an element's sort key: its byte at +0x11 over the upper bits of its dword at +4. */
x86::reg32 frontend::watb::sortKey(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const reg32 record = cpu.eax;
    return (reg32(guest.byte(record + 0x11)) << 24) | (sar(guest.dword(record + 4), 5) & 0xffffff);
}

void frontend::watbSortKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4fb830", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::sortKey(app, cpu, cpu.eax); });
}

/* sub_4feffc: a hook of the library's that answers 0. */
x86::reg32 frontend::watb::returnZero(App* app, CPU& cpu)
{
    return 0;
}

void frontend::watbReturnZero(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4feffc", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::returnZero(app, cpu); });
}

/* sub_4ff620, sub_50d6d0, sub_50dbf0: hooks that do nothing. */
void frontend::watb::nothing4ff620(App*, CPU&)
{
}

void frontend::watbNothing4ff620(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::nothing4ff620(app, cpu);
    cpu.esp += 4;
}

void frontend::watb::nothing50d6d0(App*, CPU&)
{
}

void frontend::watbNothing50d6d0(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::nothing50d6d0(app, cpu);
    cpu.esp += 4;
}

void frontend::watb::nothing50dbf0(App*, CPU&)
{
}

void frontend::watbNothing50dbf0(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::nothing50dbf0(app, cpu);
    cpu.esp += 4;
}

/* sub_4ff0f8, sub_4ff100: __allocPOSIXHandle's and __freePOSIXHandle's jumps, which
 * 0x567770 and 0x567774 point at. */
/* Entered as the code enters it, the return address's slot under esp: the function it jumps to returns
 * for it. */
x86::reg32 frontend::watb::allocHandleJump(App* app, CPU& cpu)
{
    app->dynamic_call(0x51e304, cpu);
    return cpu.eax;
}

void frontend::watbAllocHandleJump(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::allocHandleJump(app, cpu);
}

/* Entered as the code enters it, the return address's slot under esp: the function it jumps to returns
 * for it. */
x86::reg32 frontend::watb::freeHandleJump(App* app, CPU& cpu)
{
    app->dynamic_call(0x51e45c, cpu);
    return cpu.eax;
}

void frontend::watbFreeHandleJump(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::freeHandleJump(app, cpu);
}

// ---------------------------------------------------------------------------
// The runtime's start-up hooks
// ---------------------------------------------------------------------------

/* sub_4ff570: printf's and scanf's floats, [__EFG_printf] and [__EFG_scanf]. */
x86::reg32 frontend::watb::initFloatFormatters(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x56ac64, 0x51f140);
    guest.setDword(0x56ac68, 0x51f254);
    return guest.eax();
}

void frontend::watbInitFloatFormatters(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4ff570", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::initFloatFormatters(app, cpu); });
}

namespace
{
constexpr reg32 k8087 = 0x5644c0, kReal87 = 0x5644c1, kNo87Found = 0x567834;
}

/* sub_4ff59c: where there is a real x87, its state's save and restore for the signal
 * handlers.  Its call of sub_51f272, the test for the Pentium's FDIV bug, is gone in the
 * port (disassemble_nfs3hp.py), which leaves the word it was handed in eax. */
x86::reg32 frontend::watb::initChipBugHooks(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    if (guest.byte(kReal87) != 0)
    {
        guest.setDword(0x56acf0, 0x4ff590);
        guest.setDword(0x56acf4, 0x4ff598);
    }
    return guest.word(0x56acf8);
}

void frontend::watbInitChipBugHooks(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4ff59c", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::initChipBugHooks(app, cpu); });
}

/* sub_4ff5dc: an x87 looked for, by what fninit leaves in the control word. */
x86::reg32 frontend::watb::init8087(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const std::uint8_t found = guest.byte(k8087);
    if (found != 0)
        return (cpu.eax & 0xffff00ff) | (reg32(found) << 8);
    guest.setByte(kReal87, 0);
    guest.fpu().init();
    const reg32 word = guest.fpu().control.word;
    const std::uint8_t high = std::uint8_t(word >> 8);
    reg32 result = (word & 0xffffff00) | high;
    std::uint8_t present = 0;
    if (high == 3)
    {
        result = guest::direct<0x4ff59c, initChipBugHooks>(app, cpu);
        present = std::uint8_t(result);
    }
    if (guest.byte(kNo87Found) == 0)
    {
        guest.setByte(k8087, present);
        guest.setByte(kReal87, present);
    }
    return result;
}

void frontend::watbInit8087(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4ff5dc", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::init8087(app, cpu, cpu.eax); });
}

// ---------------------------------------------------------------------------
// A shape's palette and a pixel of the window
// ---------------------------------------------------------------------------

/* sub_4fd1c0, getshapeclut: the palette among the records that follow a shape, or 0. */
x86::reg32 frontend::watb::shapeClut(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    reg32 record = cpu.eax;
    if (record == 0)
        return 0;
    if (shapeDepth(guest, record) == 0)
        reportError(guest, 0x54e04c, 0x54e05c, 0x46, args({}, { 0x54e06c, guest.dword(record) & 0xff }));
    while (record != 0)
    {
        const reg32 type = guest.dword(record) & 0xff;
        if (type == 0x22 || type == 0x24 || type == 0x23 || type == 0x29 || type == 0x2a || type == 0x2c
            || type == 0x2d || type == 0x2e)
            return record;
        const reg32 next = sar(guest.dword(record), 8);
        record = next != 0 ? record + next : 0;
    }
    return 0;
}

void frontend::watbShapeClut(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4fd1c0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::shapeClut(app, cpu, cpu.eax); });
}

/* sub_4fd540: a filled rectangle, drawn by movdfl as a shape of the window's type with
 * no pixels: the colour goes in 0x9f4fac, repeated to fill a dword, for the fillers in
 * the table at 0x5676b4.  The shape's header is built in the frame, and what movdfl
 * takes for its pixels is what lies above it there: the saved registers. */
x86::reg32 frontend::watb::drawFilledRect(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                          reg32 colourWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    const reg32 entryEbp = cpu.ebp, entryEdi = cpu.edi, entryEsi = cpu.esi;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    const reg32 frame = guest.frame();
    guest.setDword(frame + 0x14, entryEbp);
    guest.setDword(frame + 0x18, entryEdi);
    guest.setDword(frame + 0x1c, entryEsi);
    const reg32 x = cpu.eax, y = cpu.edx;
    reg32 colour = colourWord;
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
    guest.setWord(frame + 4, std::uint16_t(cpu.ebx));
    guest.setWord(frame + 6, std::uint16_t(cpu.ecx));
    guest.setDword(frame, guest.byte(kWindowType));
    return guest.call(0x50b0b0, args({ frame, x, y, 0x5676b4 }));
}

void frontend::watbDrawFilledRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_4fd540", 4, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::drawFilledRect(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4));
    });
}

/* sub_503bc0, readpixel: the pixel at eax, edx of the current window; 0 outside it. */
x86::reg32 frontend::watb::readPixel(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 x = cpu.eax, y = cpu.edx;
    if (sreg32(x) < sreg32(guest.dword(kClipLeft)) || sreg32(x) >= sreg32(guest.dword(kClipRight))
        || sreg32(y) < sreg32(guest.dword(kClipTop)) || sreg32(y) >= sreg32(guest.dword(kClipBottom)))
        return 0;
    const reg32 row = guest.dword(guest.dword(kRowOffsets) + y * 4);
    const reg32 column = guest.dword(guest.dword(kColumnOffsets) + x * 4);
    const reg32 pixel = guest.dword(kWindowPixels) + row + column;
    guest.leaveEdx(pixel);
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
    return (x & 1) != 0 ? (reg32(guest.byte(pixel)) >> 4) & 0xf : reg32(guest.byte(pixel)) & 0xf;
}

void frontend::watbReadPixel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_503bc0", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::readPixel(app, cpu, cpu.eax, cpu.edx); });
}

/* sub_508b80, initvideowindow: a window's record of 0x38 bytes and its two row tables. */
x86::reg32 frontend::watb::initVideoWindow(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx,
                                           reg32 argEcx, reg32 depthWord, reg32 pixelsWord, reg32 tagWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x14);
    const reg32 frame = guest.frame();
    const reg32 window = cpu.eax, width = cpu.edx, height = cpu.ebx, stride = cpu.ecx;
    reg32 depth = depthWord;
    if (depth == 0)
        depth = guest.dword(0x56438c);
    if (depth == 0)
        depth = 8;
    const reg32 bytesStep = roundedDepth(depth);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), window, 0x38);
    guest.setDword(window, 0x57494e44);   // 'WIND'
    guest.setByte(window + 0x1e, 0xff);
    guest.setDword(window + 4, width);
    guest.setDword(window + 8, height);
    guest.setDword(window + 0x14, width);
    guest.setDword(window + 0x18, height);
    guest.setDword(window + 0x28, stride);
    guest.setByte(window + 0x1c, std::uint8_t(depth));
    guest.setByte(window + 0x1d, std::uint8_t(guest::direct<0x5098d0, shapeTypeForDepth>(app, cpu, depth)));
    guest.setDword(window + 0x20, pixelsWord);
    guest.setDword(window + 0x34, tagWord);
    const reg32 rows = guest::direct<0x509a30, makeRow>(app, cpu, reg32(0), stride, reg32(0), height);
    guest.setDword(window + 0x2c, rows);
    const reg32 columns = guest.call(0x509a30, args({ 0, bytesStep, 3, width }));
    guest.setDword(window + 0x30, columns);
    guest.leaveEdx(rows);
    if (rows != 0 && columns != 0)
        return columns;
    guest.leaveEbx(0x54f53c);
    reportErrorLeft(guest, 0x54f53c, 0x54f54c, 0x29, args({}, { 0x54f55c, width, height }));
    return guest.eax();
}

void frontend::watbInitVideoWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_508b80", 12, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::initVideoWindow(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4),
                                     guest::at(app, cpu.esp + 8), guest::at(app, cpu.esp + 12));
    });
}

// ---------------------------------------------------------------------------
// The row tables: a block of 0x568380 cut into runs -- a count, the uses, the offsets
// ---------------------------------------------------------------------------

namespace
{
constexpr reg32 kRowSpace = 0x568380, kRowSpaceWords = 0x56837c, kRowSpaceEnd = 0xa0a810;
}

/* sub_5098d0: the shape type of a window of this many bits a pixel, or 0. */
x86::reg32 frontend::watb::shapeTypeForDepth(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    switch (cpu.eax)
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

void frontend::watbShapeTypeForDepth(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5098d0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::shapeTypeForDepth(app, cpu, cpu.eax); });
}

/* sub_509940, initrow: the space made if it is not there, one free run in it. */
x86::reg32 frontend::watb::initRowTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 words = guest.dword(kRowSpaceWords);
    if (sreg32(words) < 0xcc)
        reportError(guest, 0x54f718, 0x54f724, 0x14, args({}, { 0x54f72c, words }));
    if (guest.dword(kRowSpace) == 0)
    {
        guest.setDword(kErrorFile, 0x54f718);
        guest.setDword(kErrorFunction, 0x54f724);
        guest.setDword(kErrorLine, 0x15);
        guest.setDword(kRowSpace, *fe::direct<0x4e1620, ea::reserved>(app, cpu, reg32(0x54f770), guest.dword(kRowSpaceWords) << 2,
                                                                     guest.dword(kMemoryPool)));
    }
    const reg32 space = guest.dword(kRowSpace);
    const reg32 size = guest.dword(kRowSpaceWords);
    guest.setDword(space + 4, 0);
    guest.setDword(kRowSpaceEnd, space + (size << 2));
    guest.setDword(space, size - 2);
    return space;
}

void frontend::watbInitRowTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509940", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::initRowTable(app, cpu); });
}

/* sub_509a00: the space given back. */
x86::reg32 frontend::watb::freeRowTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    const reg32 space = guest.dword(kRowSpace);
    reg32 left = guest.eax();
    if (space != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, space);
    guest.setDword(kRowSpace, 0);
    return left;
}

void frontend::watbFreeRowTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509a00", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::freeRowTable(app, cpu); });
}

/* sub_509a30: a table of ecx offsets from eax, edx apart, each shifted right by ebx: an
 * equal one shared if there is one, else a free run taken; 0 if none is long enough. */
x86::reg32 frontend::watb::makeRow(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    reg32 offset = cpu.eax;
    const reg32 step = cpu.edx;
    reg32 count = cpu.ecx;
    const unsigned shift = unsigned(cpu.ebx & 0xff);
    reg32 run = guest.dword(kRowSpace);
    const reg32 end = guest.dword(kRowSpaceEnd);
    reg32 freeRun = 0;
    reg32 leftEcx = count, leftEdx = end;
    if (run < end)
    {
        const reg32 second = sar(step, shift) + offset;
        leftEdx = second;
        do
        {
            const reg32 size = guest.dword(run);
            if (sreg32(size) >= sreg32(count))
            {
                if (guest.dword(run + 4) == 0)
                {
                    if (freeRun == 0)
                        freeRun = run;
                }
                else if (offset == guest.dword(run + 8) && second == guest.dword(run + 0xc))
                {
                    const reg32 uses = guest.dword(run + 4) + 1;
                    guest.setDword(run + 4, uses);
                    guest.leaveEbx(freeRun);
                    guest.leaveEcx(size);
                    guest.leaveEdx(uses);
                    return run + 8;
                }
            }
            leftEcx = (size << 2) + 8;
            run += leftEcx;
        } while (run < guest.dword(kRowSpaceEnd));
    }
    if (freeRun == 0)
    {
        guest.leaveEbx(0);
        guest.leaveEcx(leftEcx);
        guest.leaveEdx(leftEdx);
        return 0;
    }
    leftEdx = count + 8;
    const reg32 size = guest.dword(freeRun);
    if (sreg32(size) >= sreg32(leftEdx))
    {
        leftEdx = size - count - 2;
        guest.setDword(freeRun + count * 4 + 0xc, 0);
        guest.setDword(freeRun + count * 4 + 8, leftEdx);
    }
    else
        count = size;
    guest.setDword(freeRun, count);
    guest.setDword(freeRun + 4, 1);
    const reg32 table = freeRun + 8;
    if (positive(count))
    {
        for (reg32 i = 0; i != count; ++i)
        {
            guest.setDword(table + i * 4, sar(offset, shift));
            offset += step;
        }
        leftEdx = table + count * 4;
        leftEcx = step;
    }
    guest.leaveEbx(table);
    guest.leaveEcx(leftEcx);
    guest.leaveEdx(leftEdx);
    return table;
}

void frontend::watbMakeRow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509a30", 0, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::makeRow(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx); });
}

/* sub_509b10, removerow: a use of a table dropped; the last, its run joined to free
 * neighbours. */
x86::reg32 frontend::watb::removeRow(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20);
    const reg32 entry = cpu.eax;
    const reg32 run = entry - 8;
    const reg32 uses = guest.dword(run + 4) - 1;
    guest.setDword(run + 4, uses);
    if (uses != 0)
        return entry;
    reg32 at = guest.dword(kRowSpace);
    reg32 previous = 0;
    while (at < run)
    {
        previous = at;
        at += (guest.dword(at) + 2) << 2;
    }
    if (at != run)
        reportError(guest, 0x54f718, 0x54f778, 0x71, args({}, { 0x54f784 }));
    reg32 left = at + ((guest.dword(at) + 2) << 2);
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

void frontend::watbRemoveRow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509b10", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::removeRow(app, cpu, cpu.eax); });
}

/* sub_509c00: the memory manager's block that eax lies in, by its 16 heaps: the block's
 * pixels, or 0. */
x86::reg32 frontend::watb::findBlockHeader(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    const reg32 address = cpu.eax;
    reg32 data = cpu.edx;
    for (reg32 heap = 0; sreg32(heap) <= 0xf; ++heap)
    {
        reg32 block = guest.dword((heap & 0xf) * 4 + 0x9ef5c0);
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

void frontend::watbFindBlockHeader(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509c00", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::findBlockHeader(app, cpu, cpu.eax, cpu.edx); });
}

// ---------------------------------------------------------------------------
// A span of pixels from one format to another: eax from, edx to, ebx how many.  These
// are what the tables at 0x568394 and 0x568494 hold, by the two types' indices.
// ---------------------------------------------------------------------------

/* sub_509c50, sub_509c60, sub_509eb0, sub_50a510: the same format: a copy, its length
 * in bytes from pixels of 4, 8, 16 and 32 bits.  memmove jumped to, which returns for them:
 * entered as the code enters them, the return address's slot under esp. */
x86::reg32 frontend::watb::copyNibbles(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = count;
    cpu.ebx = sar(cpu.ebx + 1, 1);
    app->dynamic_call(kMemmove, cpu);
    return cpu.eax;
}

void frontend::watbCopyNibbles(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::copyNibbles(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
}

x86::reg32 frontend::watb::copyBytes(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = count;
    app->dynamic_call(kMemmove, cpu);
    return cpu.eax;
}

void frontend::watbCopyBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::copyBytes(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
}

x86::reg32 frontend::watb::copyWords(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = count;
    cpu.ebx += cpu.ebx;
    app->dynamic_call(kMemmove, cpu);
    return cpu.eax;
}

void frontend::watbCopyWords(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::copyWords(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
}

x86::reg32 frontend::watb::copyDwords(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count)
{
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = count;
    cpu.ebx <<= 2;
    app->dynamic_call(kMemmove, cpu);
    return cpu.eax;
}

void frontend::watbCopyDwords(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::copyDwords(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
}

/* sub_50a1c0: 24 bits. */
x86::reg32 frontend::watb::copyTriples(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return guest.call(kMemmove, args({ cpu.eax, cpu.edx, cpu.ebx * 3 }));
}

void frontend::watbCopyTriples(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a1c0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::copyTriples(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509c70: bytes into nibbles, the even pixel in the low one. */
x86::reg32 frontend::watb::bytesToNibbles(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    if (!positive(count))
        return 0;
    reg32 last = 0;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 at = to + sar(i, 1);
        const reg32 value = guest.byte(from + i);
        last = (i & 1) != 0 ? (guest.byte(at) & 0xf) | (value << 4) : (guest.byte(at) & 0xf0) | (value & 0xf);
        last &= 0xff;
        guest.setByte(at, std::uint8_t(last));
    }
    guest.leaveEbx(last);
    guest.leaveEdx(from + count);
    return count;
}

void frontend::watbBytesToNibbles(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509c70", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::bytesToNibbles(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509cd0: nibbles into bytes. */
x86::reg32 frontend::watb::nibblesToBytes(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    if (!positive(count))
        return 0;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 packed = guest.byte(from + sar(i, 1));
        guest.setByte(to + i, std::uint8_t((i & 1) != 0 ? packed >> 4 : packed & 0xf));
    }
    guest.leaveEdx(to + count);
    return count;
}

void frontend::watbNibblesToBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509cd0", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::nibblesToBytes(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509d20: 565 into palette indices: by the inverse table where there is one, else
 * the nearest colour one at a time. */
x86::reg32 frontend::watb::convert565ToIndex(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x20);
    reg32 from = cpu.eax;
    const reg32 to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax;
    const reg32 table = guest.dword(kInverseTable);
    guest.leaveEbx(0);
    if (!positive(count))
        return result;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
        const reg32 red = (pixel >> 11) & 0x1f, green = (pixel >> 5) & 0x3f, blue = pixel & 0x1f;
        from += 2;
        if (table == 0)
        {
            result = fe::direct<0x4ef760, frontend::eab::colourPack>(app, cpu, 0xff000000 | (red << 19) | (green << 10) | (blue << 3));
            guest.setByte(to + i, std::uint8_t(result));
        }
        else
        {
            const reg32 index = (red << 10) | ((green >> 1) << 5) | blue;
            const std::uint8_t found = guest.byte(guest.dword(kInverseTable) + index);
            result = (index & 0xffffff00) | found;
            guest.setByte(to + i, found);
        }
    }
    guest.leaveEbx(count);
    guest.leaveEdx(to + count);
    return result;
}

void frontend::watb565ToIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509d20", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert565ToIndex(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509df0: 8888 into 1555, nothing under a quarter of alpha, the top bit set on the rest. */
x86::reg32 frontend::watb::convert8888To1555Keyed(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = count, leftEdx = cpu.edx;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.dword(from);
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
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watb8888To1555Keyed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509df0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::convert8888To1555Keyed(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    });
}

/* sub_509e50: the same with every pixel but 0 opaque. */
x86::reg32 frontend::watb::convert8888To1555(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = count, leftEdx = cpu.edx;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.dword(from);
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
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watb8888To1555(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509e50", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8888To1555(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509ef0: 8888 into 565, nothing under a quarter of alpha, and what would come out 0 as 1. */
x86::reg32 frontend::watb::convert8888To565Keyed(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = count, leftEdx = cpu.edx;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.dword(from);
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
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watb8888To565Keyed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509ef0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::convert8888To565Keyed(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    });
}

/* sub_509f50: 8888 into 565. */
x86::reg32 frontend::watb::convert8888To565(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = count, leftEdx = cpu.edx;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.dword(from);
        result = (pixel >> 3) & 0x1f;
        leftEdx = (pixel & 0xfc00) >> 5;
        leftEbx = ((pixel & 0xf80000) >> 8) + result + leftEdx;
        guest.setWord(to, std::uint16_t(leftEbx));
        to += 2;
        from += 4;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watb8888To565(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509f50", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8888To565(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_509fb0: 8888 into 4444. */
x86::reg32 frontend::watb::convert8888To4444(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 leftEbx = count;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 alpha = reg32(guest.byte(from + 3)) >> 4, red = reg32(guest.byte(from + 2)) >> 4;
        const reg32 green = reg32(guest.byte(from + 1)) >> 4, blue = reg32(guest.byte(from)) >> 4;
        leftEbx = (red << 8) | (alpha << 12) | (green << 4);
        guest.setWord(to, std::uint16_t(blue | leftEbx));
        from += 4;
        to += 2;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(to);
    return from;
}

void frontend::watb8888To4444(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509fb0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8888To4444(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

namespace
{
constexpr reg32 kKeyColour = 0x568388;        // the 16-bit colour that is no pixel
}

/* sub_50a020: 1555 into 4444, opaque, the key colour nothing. */
x86::reg32 frontend::watb::convert1555To4444(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax;
    guest.leaveEbx(0);
    if (!positive(count))
        return result;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
        if (pixel == guest.dword(kKeyColour))
        {
            result = pixel;
            guest.setWord(to, 0);
        }
        else
        {
            const reg32 blue = (pixel >> 1) & 0xf;
            result = (((pixel >> 12) & 0xf) << 8) | 0xf000 | (((pixel >> 7) & 0xf) << 4);
            guest.setWord(to, std::uint16_t(blue | result));
        }
        from += 2;
        to += 2;
    }
    guest.leaveEbx(count);
    guest.leaveEdx(to);
    return result;
}

void frontend::watb1555To4444(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a020", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert1555To4444(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a0a0: 8888's colour into 565 in the low half of a dword, the alpha kept. */
x86::reg32 frontend::watb::convert8888To8565(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = count, leftEdx = cpu.edx;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.dword(from);
        from += 4;
        result = (pixel >> 3) & 0x1f;
        leftEdx = (pixel & 0xfc00) >> 5;
        leftEbx = ((pixel & 0xf80000) >> 8) + result + leftEdx;
        guest.setDword(to, (pixel & 0xff000000) | leftEbx);
        to += 4;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watb8888To8565(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a0a0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8888To8565(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

namespace
{
constexpr reg32 kCurrentClut = 0x568390;
}

/* sub_50a100: palette indices into 888, by the palette at 0x568390. */
x86::reg32 frontend::watb::indexTo888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    const reg32 clut = guest.dword(kCurrentClut);
    reg32 result = cpu.eax, leftEbx = count;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 scaled = reg32(guest.byte(from)) << 2;
        const reg32 colour = scaled + clut;
        const std::uint8_t two = guest.byte(colour + 2);
        guest.setByte(to + 2, two);
        const std::uint8_t one = guest.byte(colour + 1);
        ++from;
        guest.setByte(to + 1, one);
        const std::uint8_t zero = guest.byte(colour);
        guest.setByte(to, zero);
        to += 3;
        leftEbx = (scaled & 0xffffff00) | one;
        result = (colour & 0xffffff00) | zero;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(to);
    return result;
}

void frontend::watbIndexTo888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a100", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::indexTo888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a150: 565 into 888. */
x86::reg32 frontend::watb::convert565To888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax;
    guest.leaveEbx(0);
    if (!positive(count))
        return result;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
        const reg32 red = ((pixel >> 11) & 0x1f) << 3;
        guest.setByte(to, std::uint8_t((pixel & 0x1f) << 3));
        guest.setByte(to + 1, std::uint8_t(((pixel >> 5) & 0x3f) << 2));
        guest.setByte(to + 2, std::uint8_t(red));
        result = red;
        from += 2;
        to += 3;
    }
    guest.leaveEbx(count);
    guest.leaveEdx(to);
    return result;
}

void frontend::watb565To888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a150", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert565To888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a1e0: 8888 into 888. */
x86::reg32 frontend::watb::convert8888To888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 leftEbx = count;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const std::uint8_t two = guest.byte(from + 2), one = guest.byte(from + 1), zero = guest.byte(from);
        guest.setByte(to, zero);
        guest.setByte(to + 1, one);
        guest.setByte(to + 2, two);
        leftEbx = two;
        from += 4;
        to += 3;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(to);
    return from;
}

void frontend::watb8888To888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a1e0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8888To888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a240: 1555 into 8888, opaque, the key colour nothing. */
x86::reg32 frontend::watb::convert1555To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax;
    guest.leaveEbx(0);
    if (!positive(count))
        return result;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
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
    guest.leaveEbx(count);
    guest.leaveEdx(to);
    return result;
}

void frontend::watb1555To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a240", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert1555To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a2b0: 565 into 8888, opaque, the key colour nothing. */
x86::reg32 frontend::watb::convert565To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 result = cpu.eax;
    guest.leaveEbx(0);
    if (!positive(count))
        return result;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
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
    guest.leaveEbx(count);
    guest.leaveEdx(to);
    return result;
}

void frontend::watb565To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a2b0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert565To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a330: 4444 into 8888, each nibble doubled. */
x86::reg32 frontend::watb::convert4444To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 count = cpu.ebx;
    reg32 from = cpu.eax, to = cpu.edx;
    guest.leaveEdx(0);
    if (!positive(count))
        return cpu.eax;
    reg32 red = 0;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 pixel = guest.word(from);
        const reg32 a = (pixel >> 12) & 0xf, r = (pixel >> 8) & 0xf, g = (pixel >> 4) & 0xf, b = pixel & 0xf;
        red = (r | (r << 4)) << 16;
        guest.setDword(to, ((g | (g << 4)) << 8) | ((a | (a << 4)) << 24) | red | b | (b << 4));
        from += 2;
        to += 4;
    }
    guest.leaveEbx(red);
    guest.leaveEdx(count);
    return count;
}

void frontend::watb4444To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a330", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert4444To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a3c0: a dword with a 1555 colour low and the alpha on top, into 8888. */
x86::reg32 frontend::watb::convert8555To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 leftEdx = count;
    guest.leaveEbx(0);
    if (!positive(count))
    {
        guest.leaveEdx(leftEdx);
        return from;
    }
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 alpha = guest.byte(from + 3), pixel = guest.word(from);
        leftEdx = (((pixel >> 5) & 0x1f) << 11) | (((pixel >> 10) & 0x1f) << 19) | (alpha << 24) | ((pixel & 0x1f) * 8);
        guest.setDword(to, leftEdx);
        from += 4;
        to += 4;
    }
    guest.leaveEbx(count);
    guest.leaveEdx(leftEdx);
    return from;
}

void frontend::watb8555To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a3c0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8555To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a440: a dword with a 565 colour low and the alpha on top, into 8888. */
x86::reg32 frontend::watb::convert8565To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 leftEdx = count;
    guest.leaveEbx(0);
    if (!positive(count))
    {
        guest.leaveEdx(leftEdx);
        return from;
    }
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        const reg32 alpha = guest.byte(from + 3), pixel = guest.word(from);
        leftEdx = (((pixel >> 11) & 0x1f) << 19) | (alpha << 24) | (((pixel >> 5) & 0x3f) << 10) | ((pixel & 0x1f) * 8);
        guest.setDword(to, leftEdx);
        from += 4;
        to += 4;
    }
    guest.leaveEbx(count);
    guest.leaveEdx(leftEdx);
    return from;
}

void frontend::watb8565To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a440", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert8565To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50a4c0: 888 into 8888, opaque. */
x86::reg32 frontend::watb::convert888To8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    const reg32 count = cpu.ebx;
    reg32 leftEbx = count;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        leftEbx = (reg32(guest.byte(from + 1)) << 8) | (reg32(guest.byte(from + 2)) << 16) | 0xff000000
            | guest.byte(from);
        from += 3;
        guest.setDword(to, leftEbx);
        to += 4;
    }
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(to);
    return from;
}

void frontend::watb888To8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a4c0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::convert888To8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50afc0: palette indices into 16 bits by the table at [0x9f09b0], index 0xff no
 * pixel; a dword at a time on a dword's boundary. */
x86::reg32 frontend::watb::spanTo16Keyed(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    sreg32 left = sreg32(cpu.ebx);
    reg32 last = cpu.ebx;
    const reg32 table = guest.dword(0x9f09b0);
    const auto colourOf = [&](std::uint8_t index) { return guest.dword(table + reg32(index) * 4); };
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
    guest.leaveEbx(last);
    guest.leaveEdx(to);
    return from;
}

void frontend::watbSpanTo16Keyed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50afc0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::spanTo16Keyed(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50b080: a span through two converters, by a buffer in the frame: to the middle
 * format by [0xa0b018], then to the window's by [0xa0b014]. */
x86::reg32 frontend::watb::spanTwoSteps(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1908);
    const reg32 frame = guest.frame();
    const reg32 to = cpu.edx, count = cpu.ebx;
    guest.call(guest.dword(0xa0b018), args({ cpu.eax, frame, count }));
    return guest.call(guest.dword(0xa0b014), args({ frame, to, count }));
}

void frontend::watbSpanTwoSteps(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b080", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::spanTwoSteps(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

// ---------------------------------------------------------------------------
// Palettes and conversion: win\convshp.c
// ---------------------------------------------------------------------------

namespace
{
constexpr reg32 kSpanTablesKeyed = 0x568394, kSpanTablesPlain = 0x568494;  // by from * 8 + to
constexpr reg32 kBetweenTypes = 0x568594;     // bytes: the type to go through
constexpr reg32 kNeutralClut = 0xa0a814;      // the library's 256 colours
constexpr reg32 kClutBuffer = 0xa0ac14;       // a 16-bit palette made here
constexpr reg32 kExpand1555 = 0x50a240, kExpand565 = 0x50a2b0, kExpand8565 = 0x50a440, kExpand8555 = 0x50a3c0;
constexpr reg32 kExpand8888To8565 = 0x50a0a0;
constexpr reg32 kExpandTable24 = 0x51df60, kExpandTable18 = 0x51de60;
constexpr reg32 k8888To4444 = 0x509fb0, kTo1555Keyed = 0x509df0, kTo1555 = 0x509e50, kTo565Keyed = 0x509ef0, kTo565 = 0x509f50;
}

/* The answers of sub_50a570's three ends, which it jumps to. */
x86::reg32 frontend::watb::depthIndex16(App*, CPU& cpu)
{
    cpu.eax = 3;
    return cpu.eax;
}

void frontend::watbDepthIndex16(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::depthIndex16(app, cpu);
    cpu.esp += 4;
}

x86::reg32 frontend::watb::depthIndex32(App*, CPU& cpu)
{
    cpu.eax = 5;
    return cpu.eax;
}

void frontend::watbDepthIndex32(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::depthIndex32(app, cpu);
    cpu.esp += 4;
}

x86::reg32 frontend::watb::depthIndex15(App*, CPU& cpu)
{
    cpu.eax = 2;
    return cpu.eax;
}

void frontend::watbDepthIndex15(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::depthIndex15(app, cpu);
    cpu.esp += 4;
}

/* sub_50a570: a pixel format's index in the converters' tables, by its shape type; -1 for none. */
x86::reg32 frontend::watb::depthIndex(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    switch (cpu.eax)
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

void frontend::watbDepthIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a570", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::depthIndex(app, cpu, cpu.eax); });
}

/* sub_50a5d0, getclutptr: a shape's palette as 8888, made at edx from whatever its
 * record holds; the neutral palette for a shape with none. */
x86::reg32 frontend::watb::clutPointer(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x210);
    const reg32 frame = guest.frame();
    const reg32 out = cpu.edx;
    const reg32 record = guest::direct<0x4fd1c0, shapeClut>(app, cpu, cpu.eax);
    guest.leaveEdx(record);
    if (record == 0)
        return kNeutralClut;
    const reg32 data = record + kShapePixels;
    if ((guest.dword(record) & 0xff) == 0x2a)
        return data;
    const reg32 count = sar(guest.dword(record + kShapeWidth), 16);
    guest.setDword(out + 0x3fc, 0);
    const reg32 type = guest.dword(record) & 0xff;
    switch (type)
    {
    case 0x24:
        guest.call(kExpandTable24, args({ count, data, out }));
        break;
    case 0x29:
        guest.call(kExpand565, args({ data, out, count }));
        break;
    case 0x2d:
        guest.call(kExpand1555, args({ data, out, count }));
        break;
    case 0x2c:
        guest.call(kExpand8565, args({ data, out, count }));
        break;
    case 0x2e:
        guest.call(kExpand8555, args({ data, out, count }));
        break;
    case 0x22:
        guest.call(kExpandTable18, args({ count, data, out }));
        break;
    case 0x23:
        fe::direct<0x5042c4, frontend::rendf::rowSwapRedBlue>(app, cpu, data, frame, reg32(0x100));
        guest.call(kExpand1555, args({ frame, out, 0x100 }));
        break;
    default:
        guest.leaveEdx(type);
        reportError(guest, 0x54f7a8, 0x54f7b8, 0x313, args({}, { 0x54f7c4, type }));
        break;
    }
    return out;
}

void frontend::watbClutPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a5d0", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::clutPointer(app, cpu, cpu.eax, cpu.edx); });
}

/* sub_50a830, getclut16aptr: the same with the colours in 565, the alpha kept. */
x86::reg32 frontend::watb::clut16aPointer(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x410);
    const reg32 frame = guest.frame();
    const reg32 out = cpu.edx;
    const reg32 record = guest::direct<0x4fd1c0, shapeClut>(app, cpu, cpu.eax);
    guest.leaveEdx(record);
    if (record == 0)
    {
        guest.call(kExpand8888To8565, args({ kNeutralClut, out, 0x100 }));
        return out;
    }
    const reg32 data = record + kShapePixels;
    if ((guest.dword(record) & 0xff) == 0x2c)
        return data;
    const reg32 count = sar(guest.dword(record + kShapeWidth), 16);
    guest.setDword(out + 0x3fc, 0);
    const reg32 type = guest.dword(record) & 0xff;
    switch (type)
    {
    case 0x24:
        guest.call(kExpandTable24, args({ count, data, frame }));
        break;
    case 0x29:
        guest.call(kExpand565, args({ data, frame, count }));
        break;
    case 0x2d:
        guest.call(kExpand1555, args({ data, frame, count }));
        break;
    case 0x2e:
        guest.call(kExpand8555, args({ data, frame, count }));
        break;
    case 0x2a:
        guest.call(kExpand8888To8565, args({ data, out, count }));
        return out;
    case 0x22:
        guest.call(kExpandTable18, args({ count, data, frame }));
        break;
    case 0x23:
        fe::direct<0x5042c4, frontend::rendf::rowSwapRedBlue>(app, cpu, data, out, count);
        guest.call(kExpand565, args({ out, frame, count }));
        break;
    default:
        guest.leaveEdx(type);
        reportError(guest, 0x54f7a8, 0x54f82c, 0x397, args({}, { 0x54f83c, type }));
        return out;
    }
    guest.call(kExpand8888To8565, args({ frame, out, count }));
    return out;
}

void frontend::watbClut16aPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a830", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::clut16aPointer(app, cpu, cpu.eax, cpu.edx); });
}

/* sub_50a960: the converter from a shape's type straight to edx, or 0 where there is
 * none; for a shape with a palette going to more than 8 bits, the palette readied too,
 * in 0xa0ac14 at the target's depth. */
x86::reg32 frontend::watb::converterFor(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    const reg32 shape = cpu.eax, target = cpu.edx, keyed = cpu.ebx;
    const reg32 type = guest.dword(shape) & 0xff;
    guest.leaveEbx(type);
    const reg32 fromIndex = guest::direct<0x50a570, depthIndex>(app, cpu, type);
    const reg32 slot = fromIndex * 8 + guest::direct<0x50a570, depthIndex>(app, cpu, target);
    const reg32 between = guest.byte(kBetweenTypes + slot);
    guest.leaveEdx(0);
    if (target != between)
        return 0;
    const reg32 from = guest::direct<0x50a570, depthIndex>(app, cpu, type) * 8;
    const reg32 where = from + guest::direct<0x50a570, depthIndex>(app, cpu, target);
    const reg32 converter = guest.dword(where * 4 + (keyed != 0 ? kSpanTablesKeyed : kSpanTablesPlain));
    const reg32 fromDepth = shapeDepth(guest, shape);
    const reg32 toDepth = guest.byte(kDepthOfType + (between & 0x7f));
    if (sreg32(fromDepth) > 8 || sreg32(toDepth) <= 8)
        return converter;
    const reg32 clut = guest.call(0x50a5d0, args({ shape, kClutBuffer }));
    guest.setDword(kCurrentClut, clut);
    const reg32 colours = reg32(1) << (fromDepth & 31);
    guest.leaveEbx(colours);
    reg32 convert = 0;
    if (target == 0x6d)
        convert = k8888To4444;
    else if (toDepth == 0xf)
        convert = keyed != 0 ? kTo1555Keyed : kTo1555;
    else if (toDepth == 0x10)
        convert = keyed != 0 ? kTo565Keyed : kTo565;
    if (convert != 0)
        guest.call(convert, args({ clut, kClutBuffer, colours }));
    return converter;
}

void frontend::watbConverterFor(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50a960", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::converterFor(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50aad0, convertshapetype: a new shape of the type edx, from the shape at eax,
 * through as many types between as it takes (at most 10), each step in memory of its
 * own, freed once the next is made.  ebx asks for keyed colours, ecx for the keyed
 * table of converters.  What is at the end of each is checked: 'PHSC'. */
x86::reg32 frontend::watb::convertShape(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x64);
    constexpr reg32 kSentinel = 0x50485343;
    const reg32 frame = guest.frame();
    const reg32 kBetweenSlot = frame + 0x20;      // read by the second report before it is set
    const reg32 target = cpu.edx, mode = cpu.ecx;
    reg32 keyed = cpu.ebx;
    reg32 shape = cpu.eax;
    const reg32 firstClut = guest::direct<0x4fd1c0, shapeClut>(app, cpu, shape);
    reg32 previous = 0;
    for (reg32 tries = 10;;)
    {
        const reg32 fromIndex = guest::direct<0x50a570, depthIndex>(app, cpu, guest.dword(shape) & 0xff);
        const reg32 toIndex = guest::direct<0x50a570, depthIndex>(app, cpu, target);
        if (sreg32(fromIndex) < 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x3fb, args({}, { 0x54f87c, guest.dword(shape) & 0xff }));
        if (sreg32(toIndex) < 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x3fd, args({}, { 0x54f8b4, guest.dword(kBetweenSlot) }));
        const reg32 tableFrom = guest::direct<0x50a570, depthIndex>(app, cpu, guest.dword(shape) & 0xff) * 8;
        const reg32 tableWhere = tableFrom + guest::direct<0x50a570, depthIndex>(app, cpu, target);
        const reg32 convert = guest.dword(tableWhere * 4 + (mode != 0 ? kSpanTablesKeyed : kSpanTablesPlain));
        const reg32 betweenFrom = guest::direct<0x50a570, depthIndex>(app, cpu, guest.dword(shape) & 0xff) * 8;
        const reg32 betweenWhere = betweenFrom + guest::direct<0x50a570, depthIndex>(app, cpu, target);
        const reg32 between = guest.byte(kBetweenTypes + betweenWhere);
        guest.setDword(kBetweenSlot, between);
        if (between == 0)
            reportError(guest, 0x54f7a8, 0x54f868, 0x409, args({}, { 0x54f8fc, guest.dword(shape) & 0xff, target }));

        const reg32 fromDepth = shapeDepth(guest, shape);
        const reg32 toDepth = guest.byte(kDepthOfType + (between & 0x7f));
        const reg32 width = sar(guest.dword(shape + kShapeWidth), 16);
        const reg32 height = sar(guest.dword(shape + kShapeHeight), 16);
        const reg32 stride = fe::direct<0x5236c0, frontend::watc::shapeRowBytes>(app, cpu, shape);
        const reg32 rowBytes = sar((width * roundedDepth(toDepth) + 7) & ~reg32(7), 3);

        // A palette's shape going to more than 8 bits: its colours at the new depth.
        if (sreg32(fromDepth) <= 8 && sreg32(toDepth) > 8)
        {
            const reg32 colours = reg32(1) << (fromDepth & 31);
            if (keyed != 0)
            {
                for (reg32 i = 0; sreg32(i) < sreg32(colours); ++i)
                    guest.setDword(kClutBuffer + i * 4, guest.dword(reg32(guest.byte(0x56ae64 + i)) * 4 + kNeutralClut));
                guest.setDword(kCurrentClut, kClutBuffer);
            }
            else
                guest.setDword(kCurrentClut, guest::direct<0x50a5d0, clutPointer>(app, cpu, firstClut, kClutBuffer));
            reg32 step = 0;
            if (target == 0x6d)
                step = k8888To4444;
            else if (toDepth == 0xf)
                step = mode != 0 ? kTo1555Keyed : kTo1555;
            else if (toDepth == 0x10)
                step = mode != 0 ? kTo565Keyed : kTo565;
            if (step != 0)
                guest.call(step, args({ guest.dword(kCurrentClut), kClutBuffer, colours }));
            keyed = 0;
        }

        fe::direct<0x4df690, watcom::clib::sprintf>(guest.app(), guest.cpu(), frame, 0x54f970, guest::Words{fromDepth, toDepth});
        const reg32 pixelBytes = height * rowBytes;
        guest.setDword(kErrorFile, 0x54f7a8);
        guest.setDword(kErrorFunction, 0x54f868);
        guest.setDword(kErrorLine, 0x43e);
        const reg32 made = *fe::direct<0x4e1620, ea::reserved>(app, cpu, frame, pixelBytes + 0x14, guest.dword(kMemoryPool));
        guest.setDword(made + pixelBytes + 0x10, kSentinel);

        guest.setDword(made, between & 0xff);
        guest.setWord(made + 4, std::uint16_t(width));
        guest.setWord(made + 6, std::uint16_t(height));
        guest.setWord(made + 8, guest.word(shape + 8));
        guest.setWord(made + 0xa, guest.word(shape + 0xa));
        // Each read before its write: the memory made may be where the shape was.
        const reg32 flagsLow = guest.dword(shape + 0xc) & 0xfff;
        guest.setWord(made + 0xc, guest.word(made + 0xc) & 0xf000);
        guest.setDword(made + 0xc, guest.dword(made + 0xc) | flagsLow);
        const reg32 flagsHigh = guest.dword(shape + 0xc) & 0xfff0000;
        guest.setWord(made + 0xe, guest.word(made + 0xe) & 0xf000);
        const reg32 header = guest.dword(made + 0xc) | flagsHigh;
        guest.setDword(made + 0xc, header);

        reg32 from = shape + kShapePixels, to = made + kShapePixels;
        guest.leaveEbx(height);
        guest.leaveEdx(header);
        for (reg32 row = 0; sreg32(row) < sreg32(height); ++row)
        {
            guest.call(convert, args({ from, to, width }));
            guest.leaveEdx(height);
            to += rowBytes;
            from += stride;
        }
        guest.leaveEcx(0);
        if (guest.dword(made + pixelBytes + 0x10) != kSentinel)
        {
            guest.leaveEbx(0x54f7a8);
            guest.leaveEcx(fromDepth);
            guest.leaveEdx(width);
            reportError(guest, 0x54f7a8, 0x54f868, 0x461,
                        args({}, { 0x54f978, width, height, fromDepth, width, height, toDepth }));
        }
        if (previous != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, previous);
        shape = made;
        previous = made;
        if ((guest.dword(made) & 0xff) == target || --tries == 0)
            return made;
    }
}

void frontend::watbConvertShape(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50aad0", 0, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::convertShape(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
    });
}

// ---------------------------------------------------------------------------
// movdfl: win\movdfl.c
// ---------------------------------------------------------------------------

/* sub_50b0b0, movdfl: a shape drawn at edx, ebx through the blitters at ecx -- by depth
 * at +0 to +0x14, or one for all at +0x18, a type at +0x20, and at +0x1c how keyed it
 * is -- clipped to the window, converted as it goes where there is a converter and first
 * where there is not. */
x86::reg32 frontend::watb::moveShape(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x4c);
    constexpr reg32 kSpanTwoSteps = 0x50b080;
    constexpr reg32 kFirstStep = 0xa0b018, kSecondStep = 0xa0b014;
    const reg32 frame = guest.frame();
    const reg32 kBlitterSlot = frame + 0x38;      // kept where it is: a depth not in the table leaves what was there
    reg32 shape = cpu.eax, x = cpu.edx, y = cpu.ebx;
    const reg32 blitters = cpu.ecx;
    if (shapeDepth(guest, shape) == 0)
        reportError(guest, 0x54f9ac, 0x54f9bc, 0x26, args({}, { 0x54f9c4, guest.dword(shape) & 0xff }));
    reg32 fixed = guest.dword(blitters + 0x18);
    reg32 wanted = guest.byte(kWindowType);
    guest.setDword(kFirstStep, 0);
    if (guest.dword(blitters + 0x20) != 0)
        wanted = guest.dword(blitters + 0x20);
    if (wanted == 0)
        reportError(guest, 0x54f9ac, 0x54f9bc, 0x2f, args({}, { 0x54f9ec, wanted }));
    reg32 converted = 0;
    if (fixed == 0 && (guest.dword(shape) & 0xff) != wanted)
    {
        const reg32 keyedConverter = guest.dword(blitters + 0x1c) == 2 ? 0 : 1;
        fixed = guest.call(0x50a960, args({ shape, wanted, keyedConverter }));
        if (fixed == 0)
        {
            const reg32 mode = guest.dword(blitters + 0x1c) == 2 ? 0 : 1;
            const reg32 keyed = guest.dword(blitters + 0x1c) == 1 ? 1 : 0;
            converted = guest.call(0x50aad0, args({ shape, wanted, keyed, mode }));
            shape = converted;
        }
        else if (guest.dword(blitters + 0x1c) != 2)
        {
            guest.setDword(kFirstStep, fixed);
            fixed = 0;
        }
    }

    reg32 from = shape + kShapePixels;
    reg32 width = sar(guest.dword(shape + kShapeWidth), 16);
    const reg32 heightWord = guest.dword(shape + kShapeHeight);
    const reg32 depth = roundedDepth(shapeDepth(guest, shape));
    const reg32 rowBytes = sar(width * depth, 3);
    reg32 fromSkip = fe::direct<0x5236c0, frontend::watc::shapeRowBytes>(app, cpu, shape) - rowBytes;
    reg32 height = sar(heightWord, 16);

    reg32 result = guest.dword(kClipTop) - y;
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
        const reg32 skipped = sar(result * depth, 3);
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
        const reg32 pixels = guest.dword(kWindowPixels);
        const reg32 fromBytes = sar(depth * width, 3);
        const reg32 toBytes = sar(roundedDepth(guest.byte(kWindowDepth)) * width, 3);
        reg32 to = guest.dword(guest.dword(kRowOffsets) + (y << 2)) + guest.dword(guest.dword(kColumnOffsets) + (x << 2));
        const reg32 toSkip = guest.dword(kWindowStride) - toBytes;

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
        const reg32 blitter = guest.dword(kBlitterSlot);
        const reg32 toStep = toBytes + toSkip, fromStep = fromBytes + fromSkip;

        if (guest.dword(kWindowPolled) != 0)
        {
            // The window's owner is let at the messages every so many pixels.
            reg32 sinceLast = 0;
            for (reg32 rows = height; rows != 0; --rows)
            {
                guest.call(blitter, args({ from, pixels + to, width }));
                result = fromStep;
                to += toStep;
                from += fromStep;
                sinceLast += width;
                const reg32 every = guest.dword(0x5643ec);
                guest.leaveEbx(every);
                if (sreg32(sinceLast) > sreg32(every))
                {
                    guest.leaveEcx(from);
                    guest.leaveEdx(sinceLast);
                    cpu.eax = 0;
                    sinceLast = 0;
                    result = guest.call(0x4f90e0);
                    if (result == 0)
                        return 0;
                }
            }
        }
        else if (toSkip == 0 && fromSkip == 0 && blitter != kSpanTwoSteps)
        {
            // Without gaps: one long span.
            guest.call(blitter, args({ from, pixels + to, width * height }));
            result = toBytes * height;
            from += height * fromBytes;
        }
        else
        {
            for (reg32 rows = height; rows != 0; --rows)
            {
                guest.call(blitter, args({ from, pixels + to, width }));
                result = toStep;
                to += toStep;
                from += fromStep;
            }
        }
    }
    else
        guest.leaveEbx(depth);

    guest.leaveEcx(from);
    guest.leaveEdx(converted);
    if (converted != 0)
        result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, converted);
    return result;
}

void frontend::watbMoveShape(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b0b0", 0, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::moveShape(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx); });
}

// ---------------------------------------------------------------------------
// Text: font\textpc.c
// ---------------------------------------------------------------------------

/* sub_50b540: a string's next byte: eax the place of the pointer. */
x86::reg32 frontend::watb::nextByte(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const reg32 place = cpu.eax;
    const reg32 at = guest.dword(place) + 1;
    const reg32 value = guest.byte(at - 1);
    guest.setDword(place, at);
    return value;
}

void frontend::watbNextByte(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b540", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::nextByte(app, cpu, cpu.eax); });
}

/* sub_50b560: a character's glyph number by the font's table, for ' ' to 0x7f. */
x86::reg32 frontend::watb::charWidthEntry(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const reg32 character = cpu.eax;
    if (sreg32(character) < 0x20 || sreg32(character) > 0x7f)
        return character;
    return guest.word(character * 2 + 0x568594);
}

void frontend::watbCharWidthEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b560", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::charWidthEntry(app, cpu, cpu.eax); });
}

/* sub_50b580: a string's next character: two bytes with the top bit of the first set,
 * else one, through sub_50b560. */
x86::reg32 frontend::watb::nextChar(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    const reg32 place = cpu.eax;
    reg32 at = guest.dword(place);
    const reg32 first = guest.byte(at);
    ++at;
    reg32 result;
    if ((first & 0x80) != 0)
    {
        result = ((first << 8) | guest.byte(at)) & 0xffff;
        ++at;
    }
    else
        result = guest::direct<0x50b560, charWidthEntry>(app, cpu, first) & 0xffff;
    guest.setDword(place, at);
    return result;
}

void frontend::watbNextChar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b580", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::nextChar(app, cpu, cpu.eax); });
}

namespace
{
constexpr reg32 kFontPalette = 0x567280;      // the 16 ARGBs of the font's nibbles, then its blend table
constexpr reg32 kFontColours = 0x567228;      // the 16 colours plotted
constexpr reg32 kFontColour = 0x567220;
}

/* sub_50b5c0: a line of a 4-bit font in alpha: a fast loop for 15 and 16 bits fully
 * inside the window, sub_5239ec for 8 bits, and else plotted pixel by pixel.  Stack: the
 * first row, the width and height in pixels, the bytes a row. */
x86::reg32 frontend::watb::textAlpha(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                     reg32 rowWord, reg32 widthWord, reg32 heightWord, reg32 strideWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x40);
    const reg32 frame = guest.frame();
    const reg32 x = cpu.eax, y = cpu.edx, xOffset = cpu.ecx;
    const reg32 stride = strideWord;
    const reg32 width = widthWord, height = heightWord;
    reg32 font = cpu.ebx + kShapePixels + rowWord * stride + sar(xOffset - sar(xOffset, 31), 1);
    const reg32 rowBytes = sar(width + 1, 1);
    const reg32 palette = guest.dword(kFontPalette);
    const auto colourOf = [&](reg32 nibble) { return guest.dword(palette + nibble * 4); };

    // What the tests leave for a slow loop that has no rows to draw.
    reg32 leftEax = rowBytes, leftEcx = xOffset, leftEdx = sar(xOffset, 31);
    const auto inside = [&] {
        const std::uint8_t locked = guest.byte(kWindowLocked);
        leftEax = (leftEax & 0xffff00ff) | (reg32(locked) << 8);
        if (locked != 0)
            return false;
        leftEax = x;
        if (sreg32(x) < sreg32(guest.dword(kClipLeft)) || sreg32(x) >= sreg32(guest.dword(kClipRight)))
            return false;
        leftEax = y;
        if (sreg32(y) < sreg32(guest.dword(kClipTop)) || sreg32(y) >= sreg32(guest.dword(kClipBottom)))
            return false;
        leftEax = x + width;
        leftEcx = width;
        if (sreg32(leftEax) < sreg32(guest.dword(kClipLeft)) || sreg32(leftEax) >= sreg32(guest.dword(kClipRight)))
            return false;
        leftEax = y + height;
        leftEcx = height;
        if (sreg32(leftEax) < sreg32(guest.dword(kClipTop)) || sreg32(leftEax) >= sreg32(guest.dword(kClipBottom)))
            return false;
        return true;
    };

    if (inside())
    {
        const reg32 skip = stride - rowBytes;
        const std::uint8_t depth = guest.byte(kWindowDepth);
        leftEax = skip;
        leftEcx = rowBytes;
        leftEdx = (leftEdx & 0xffffff00) | depth;
        if (depth == 0x10 || depth == 0xf)
        {
            const bool is565 = depth == 0x10;
            reg32 row = y, rows = height;
            Scratch left{ 0, 0 };
            reg32 lastEbx = 0;
            const auto blendNibble = [&](reg32 to, reg32 nibble) {
                reg32 argb = colourOf(nibble);
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
                reg32 to = guest.dword(guest.dword(kColumnOffsets) + x * 4) + guest.dword(guest.dword(kRowOffsets) + row * 4)
                    + guest.dword(kWindowPixels);
                reg32 bytes = rowBytes;
                do
                {
                    blendNibble(to, reg32(guest.byte(font)) >> 4);
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
                guest.leaveEbx(0);
                guest.leaveEcx(row);
                guest.leaveEdx(skip);
                return left.eax;
            }
            guest.leaveEbx(lastEbx);
            guest.leaveEcx(0);
            guest.leaveEdx(0);
            return row;
        }
        if (depth == 8)
        {
            const reg32 toSkip = guest.dword(kWindowStride) - rowBytes * 2;
            const reg32 rowAt = guest.dword(guest.dword(kRowOffsets) + (y << 2));
            const reg32 to = guest.dword(kWindowPixels) + rowAt + x;
            guest.leaveEbx(toSkip);
            guest.leaveEcx(x);
            guest.leaveEdx(rowAt);
            return guest.call(0x5239ec, args({ to }, { palette + 0x40, font, to, rowBytes, height, skip, toSkip }));
        }
    }

    if (!positive(height))
    {
        guest.leaveEbx(height);
        guest.leaveEcx(leftEcx);
        guest.leaveEdx(leftEdx);
        return leftEax;
    }
    const reg32 skip = stride - rowBytes, lastRow = y + height;
    reg32 column = 0;
    for (reg32 row = y; sreg32(row) < sreg32(lastRow); ++row)
    {
        column = 0;
        if (positive(width))
        {
            do
            {
                const reg32 packed = guest.byte(font);
                ++font;
                const reg32 high = sar(packed, 4), low = packed & 0xf;
                if (high != 0)
                    fe::direct<0x523760, frontend::watc::putPixel>(app, cpu, x + column, row, colourOf(high));
                if (low != 0)
                    fe::direct<0x523760, frontend::watc::putPixel>(app, cpu, x + 1 + column, row, colourOf(low));
                column += 2;
            } while (sreg32(column) < sreg32(width));
        }
        font += skip;
    }
    guest.leaveEbx(skip);
    guest.leaveEcx(column);
    guest.leaveEdx(lastRow);
    return positive(width) ? width : lastRow;
}

void frontend::watbTextAlpha(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50b5c0", 16, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::textAlpha(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4),
                               guest::at(app, cpu.esp + 8), guest::at(app, cpu.esp + 12), guest::at(app,
                               cpu.esp + 16));
    });
}

/* sub_50ba90: the same plotted by sub_523b00 from the font's own colours. */
x86::reg32 frontend::watb::textPlotted(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                       reg32 rowWord, reg32 widthWord, reg32 heightWord, reg32 strideWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x2c);
    const reg32 frame = guest.frame();
    const reg32 x = cpu.eax, y = cpu.edx, xOffset = cpu.ecx;
    const reg32 stride = strideWord;
    const reg32 width = widthWord, height = heightWord;
    if (!positive(height))
        return x;
    const reg32 half = sar(xOffset - sar(xOffset, 31), 1);
    reg32 line = cpu.ebx + kShapePixels + stride * rowWord;
    const reg32 lastRow = y + height;
    reg32 row = y;
    do
    {
        reg32 font = line + half;
        reg32 column = 0;
        if (positive(width))
        {
            do
            {
                const reg32 packed = guest.byte(font);
                ++font;
                const reg32 high = sar(packed, 4), low = packed & 0xf;
                if (high != 0)
                    fe::direct<0x523b00, frontend::watc::writePixel>(app, cpu, x + column, row,
                                                                     guest.dword(high * 4 + kFontColours));
                if (low != 0)
                    fe::direct<0x523b00, frontend::watc::writePixel>(app, cpu, x + 1 + column, row,
                                                                     guest.dword(low * 4 + kFontColours));
                column += 2;
            } while (sreg32(column) < sreg32(width));
        }
        line += stride;
        ++row;
    } while (sreg32(row) < sreg32(lastRow));
    guest.leaveEbx(lastRow);
    guest.leaveEcx(row);
    guest.leaveEdx(line);
    return stride;
}

void frontend::watbTextPlotted(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50ba90", 16, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::textPlotted(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4),
                                 guest::at(app, cpu.esp + 8), guest::at(app, cpu.esp + 12), guest::at(app,
                                 cpu.esp + 16));
    });
}

/* sub_50bb80: a line of a 1-bit font: a byte of its mask at a time through what plots
 * eight pixels of this depth.  Stack: the first row, the width and height in bits, the
 * bytes a row. */
x86::reg32 frontend::watb::textMono(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                    reg32 rowWord, reg32 widthWord, reg32 heightWord, reg32 strideWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x38);
    const reg32 frame = guest.frame();
    reg32 x = cpu.eax;
    const reg32 y = cpu.edx, font = cpu.ebx, bitOffset = cpu.ecx;
    reg32 width = widthWord;
    reg32 height = heightWord;
    reg32 plot = 0;
    switch (guest.byte(kWindowDepth))
    {
    case 8: plot = 0x50c220; break;
    case 0xf:
    case 0x10: plot = 0x50c270; break;
    case 0x18: plot = 0x50c2c0; break;
    case 0x20: plot = 0x50c350; break;
    default:
        guest.leaveEbx(0x54fa18);
        reportErrorLeft(guest, 0x54fa18, 0x54fa28, 0xf1, args({}, { 0x54fa34, guest.byte(kWindowDepth) }));
        return guest.eax();
    }
    const reg32 clipLeft = guest.dword(kClipLeft);
    const reg32 leftClip = sreg32(x) < sreg32(clipLeft) ? clipLeft - x : 0;
    reg32 row = sreg32(y) < sreg32(guest.dword(kClipTop)) ? guest.dword(kClipTop) - y : 0;
    if (sreg32(x + width) > sreg32(guest.dword(kClipRight)))
        width = guest.dword(kClipRight) - x;
    reg32 leftEcx = height + y;
    if (sreg32(leftEcx) > sreg32(guest.dword(kClipBottom)))
    {
        leftEcx = guest.dword(kClipBottom) - y;
        height = leftEcx;
    }
    const auto leave = [&](reg32 ebx, reg32 ecx, reg32 edx) {
        guest.leaveEbx(ebx);
        guest.leaveEcx(ecx);
        guest.leaveEdx(edx);
    };
    if (sreg32(width) < 0 || sreg32(height) < 0 || sreg32(leftClip) >= sreg32(width))
    {
        leave(width, leftEcx, y);
        return leftClip;
    }
    if (sreg32(row) >= sreg32(height))
    {
        leave(width, row, y);
        return leftClip;
    }
    const reg32 stride = strideWord;
    const reg32 aligned = leftClip & ~reg32(7);
    reg32 line = font + kShapePixels + (rowWord + row) * stride + sar(bitOffset + aligned, 3);
    const reg32 rowAt = guest.dword(guest.dword(kRowOffsets) + (y + row) * 4);
    x += aligned;
    reg32 to = guest.dword(kWindowPixels) + rowAt + guest.dword(guest.dword(kColumnOffsets) + 4) * x;
    width -= aligned;
    const reg32 wholeBytes = sar(width, 3);
    const reg32 restBits = width - (wholeBytes << 3);
    const std::uint8_t firstMask = std::uint8_t(sar(0xff, unsigned(leftClip & 7)));
    const std::uint8_t lastMask = std::uint8_t(0xff << (std::uint8_t(8 - std::uint8_t(restBits)) & 31));
    for (;;)
    {
        const reg32 thisRow = row;
        ++row;
        if (sreg32(thisRow) >= sreg32(height))
        {
            guest.leaveEcx(height);
            guest.leaveEdx(row);
            return thisRow;
        }
        std::uint8_t mask = firstMask;
        reg32 source = line;
        reg32 at = to;
        if (wholeBytes != 0)
        {
            at = guest.call(plot, args({ reg32(guest.byte(source) & firstMask), guest.dword(kFontColour), at }));
            mask = 0xff;
            ++source;
            for (reg32 bytes = wholeBytes - 1; bytes != 0; --bytes)
            {
                const reg32 bits = guest.byte(source);
                ++source;
                at = guest.call(plot, args({ bits, guest.dword(kFontColour), at }));
            }
        }
        if (restBits != 0)
            guest.call(plot, args({ reg32(guest.byte(source) & mask & lastMask), guest.dword(kFontColour), at }));
        line += stride;
        guest.leaveEbx(guest.dword(kWindowStride));
        to += guest.dword(kWindowStride);
    }
}

void frontend::watbTextMono(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50bb80", 16, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::textMono(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4),
                              guest::at(app, cpu.esp + 8), guest::at(app, cpu.esp + 12), guest::at(app,
                              cpu.esp + 16));
    });
}

/* sub_50bdf0: for each palette index and each of 16 ARGBs, the index nearest the ARGB
 * over that colour: the table the 8-bit blitter works from. */
x86::reg32 frontend::watb::blendTable(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x344);
    const reg32 colours = cpu.eax, table = cpu.edx;
    std::uint8_t red[256], green[256], blue[256];
    for (reg32 i = 0; i != 256; ++i)
    {
        const reg32 colour = guest.dword(kNeutralClut + i * 4);
        red[i] = std::uint8_t(colour >> 16);
        green[i] = std::uint8_t(colour >> 8);
        blue[i] = std::uint8_t(colour);
    }
    const auto over = [](std::int32_t under, std::int32_t show, std::int32_t top) {
        const std::int32_t value = under * show / 0xff + top;
        return value > 0xff ? 0xff : value;
    };
    for (reg32 index = 0; index != 256; ++index)
    {
        for (reg32 k = 0; k != 16; ++k)
        {
            const reg32 colour = colours + k * 4;
            const std::int32_t alpha = guest.byte(colour + 3);
            reg32 nearest = index;
            if (alpha != 0)
            {
                const std::int32_t show = 0xff - alpha;
                const std::int32_t r = over(red[index], show, guest.byte(colour + 2));
                const std::int32_t g = over(green[index], show, guest.byte(colour + 1));
                const std::int32_t b = over(blue[index], show, guest.byte(colour));
                nearest = fe::direct<0x4ef760, frontend::eab::colourPack>(app, cpu, (reg32(alpha) << 24) | (reg32(r) << 16) | (reg32(g) << 8) | reg32(b));
            }
            guest.leaveEdx(((k << 8) | index) + table);
            guest.setByte(table + ((k << 8) | index), std::uint8_t(nearest));
        }
    }
    return 3 * 256;
}

void frontend::watbBlendTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50bdf0", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendTable(app, cpu, cpu.eax, cpu.edx); });
}

/* sub_50bfd0: the font's 16 ARGBs, a colour faded in over alpha; the blend table for 8 bits. */
x86::reg32 frontend::watb::fadeTable(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x30);
    const reg32 table = guest.dword(kFontPalette);
    const reg32 argb = guest::direct<0x50c3a0, colourToArgb>(app, cpu, cpu.eax);
    const std::int32_t blue = argb & 0xff, green = (argb >> 8) & 0xff, red = (argb >> 16) & 0xff;
    for (std::int32_t i = 0; i != 16; ++i)
    {
        const reg32 entry = table + reg32(i) * 4;
        guest.setByte(entry + 3, std::uint8_t(i * 0xff / 0xf));
        guest.setByte(entry + 2, std::uint8_t(i * red / 0xf));
        guest.setByte(entry + 1, std::uint8_t(i * green / 0xf));
        guest.setByte(entry, std::uint8_t(i * blue / 0xf));
    }
    if (guest.byte(kWindowDepth) > 8)
        return table + 0x40;
    return guest::direct<0x50bdf0, blendTable>(app, cpu, table, guest.dword(kFontPalette) + 0x40);
}

void frontend::watbFadeTable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50bfd0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::fadeTable(app, cpu, cpu.eax); });
}

/* sub_50c0d0, inittextdraw: the font's tables made and filled, its blitter chosen. */
x86::reg32 frontend::watb::initTextDraw(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    if (guest.dword(kFontPalette) == 0)
    {
        constexpr reg32 kBytes = 0x1040;
        guest.setDword(0x567284, kBytes);
        guest.setDword(kErrorFile, 0x54fa18);
        guest.setDword(kErrorFunction, 0x54fa60);
        guest.setDword(kErrorLine, 0x150);
        const reg32 block = *fe::direct<0x4e1620, ea::reserved>(app, cpu, reg32(0x54fa70), kBytes, guest.dword(kMemoryPool));
        guest.setDword(kFontPalette, block);
        cpu.eax = block;  // what sub_50c180 hands back as it came, on one of its ways
    }
    const reg32 result = guest::direct<0x50c180, textPalette>(app, cpu);
    reg32 blitter = 0x50b5c0;
    if (guest.dword(0x567208) == 1)
        blitter = 0x50bb80;
    else if (guest.dword(0x56720c) == 0)
        blitter = 0x50ba90;
    guest.setDword(0x567288, blitter);
    guest.setDword(0x567290, 0);
    guest.setDword(0x56728c, 0);
    return result;
}

void frontend::watbInitTextDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c0d0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::initTextDraw(app, cpu); });
}

/* sub_50c180: the font's colours for its kind: copied where they are given, faded where not. */
x86::reg32 frontend::watb::textPalette(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    if (guest.dword(0x5672a4) != 0)
    {
        guest.call(kMemmove, args({ kFontColours, guest.dword(kFontPalette), 0x40 }));
        if (guest.byte(kWindowDepth) > 8)
            return guest.eax();
        const reg32 table = guest.dword(kFontPalette);
        return guest::direct<0x50bdf0, blendTable>(app, cpu, table, table + 0x40);
    }
    if (guest.dword(0x56720c) != 0 && guest.dword(0x567208) == 4)
        return guest::direct<0x50bfd0, fadeTable>(app, cpu, guest.dword(kFontColour));
    return guest.eax();
}

void frontend::watbTextPalette(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c180", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::textPalette(app, cpu); });
}

/* sub_50c1e0: the font's tables given back. */
x86::reg32 frontend::watb::freeTextDraw(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    const reg32 table = guest.dword(kFontPalette);
    reg32 left = guest.eax();
    if (table != 0)
    {
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, table);
        guest.setDword(kFontPalette, 0);
        guest.setDword(0x567284, 0);
    }
    guest.setDword(0x567288, 0);
    return left;
}

void frontend::watbFreeTextDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c1e0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::freeTextDraw(app, cpu); });
}

/* sub_50c220, sub_50c270, sub_50c2c0, sub_50c350: the set bits of al's eight pixels at
 * ebx in the colour edx, at 8, 16, 24 and 32 bits; the next eight's place. */
x86::reg32 frontend::watb::plotMask8(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 to = cpu.ebx;
    for (reg32 pixel = 0; pixel != 8; ++pixel)
        if ((cpu.eax & (0x80 >> pixel)) != 0)
            guest.setByte(to + pixel, std::uint8_t(cpu.edx));
    return to + 8;
}

void frontend::watbPlotMask8(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c220", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::plotMask8(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

x86::reg32 frontend::watb::plotMask16(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 to = cpu.ebx;
    for (reg32 pixel = 0; pixel != 8; ++pixel)
        if ((cpu.eax & (0x80 >> pixel)) != 0)
            guest.setWord(to + pixel * 2, std::uint16_t(cpu.edx));
    return to + 0x10;
}

void frontend::watbPlotMask16(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c270", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::plotMask16(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

x86::reg32 frontend::watb::plotMask24(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 to = cpu.ebx, colour = cpu.edx;
    for (reg32 pixel = 0; pixel != 8; ++pixel)
        if ((cpu.eax & (0x80 >> pixel)) != 0)
        {
            guest.setByte(to + pixel * 3 + 1, std::uint8_t(colour >> 8));
            guest.setByte(to + pixel * 3 + 2, std::uint8_t(colour));
            guest.setByte(to + pixel * 3, std::uint8_t(colour >> 16));
        }
    guest.leaveEdx((sar(colour, 16) & 0xffff00ff) | ((colour & 0xff) << 8));
    return to + 0x18;
}

void frontend::watbPlotMask24(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c2c0", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::plotMask24(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

x86::reg32 frontend::watb::plotMask32(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const reg32 to = cpu.ebx;
    for (reg32 pixel = 0; pixel != 8; ++pixel)
        if ((cpu.eax & (0x80 >> pixel)) != 0)
            guest.setDword(to + pixel * 4, cpu.edx);
    return to + 0x20;
}

void frontend::watbPlotMask32(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c350", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::plotMask32(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50c3a0, getargb: a colour of the current window's depth as an ARGB. */
x86::reg32 frontend::watb::colourToArgb(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24);
    const reg32 colour = cpu.eax;
    const std::uint8_t depth = guest.byte(kWindowDepth);
    if (depth == 0xf)
    {
        if ((colour & 0xffff0000) != 0)
            reportError(guest, 0x54fa7c, 0x54fa8c, 0x90, args({}, { 0x54fa94, depth, colour, colour }));
        const reg32 alpha = (colour & 0x8000) != 0 ? 0xff : 0;
        return (alpha << 24) | ((sar(colour, 10) & 0x1f) << 19) | ((sar(colour, 5) & 0x1f) << 11) | ((colour & 0x1f) << 3);
    }
    if (depth == 0x10)
    {
        if ((colour & 0xffff0000) != 0)
            reportError(guest, 0x54fa7c, 0x54fa8c, 0xa4, args({}, { 0x54faf4, depth, colour, colour }));
        const reg32 alpha = colour != 0 ? 0xff : 0;
        return (alpha << 24) | ((sar(colour, 11) & 0x1f) << 19) | ((sar(colour, 5) & 0x3f) << 10) | ((colour & 0x1f) << 3);
    }
    if (depth == 0x18 || depth == 0x20)
        return colour;
    if (sreg32(colour) < 0 || sreg32(colour) >= 0x100)
        reportError(guest, 0x54fa7c, 0x54fa8c, 0xb8, args({}, { 0x54fb54, guest.byte(kWindowDepth), colour, colour }));
    return guest.dword(colour * 4 + kNeutralClut);
}

void frontend::watbColourToArgb(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c3a0", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::colourToArgb(app, cpu, cpu.eax); });
}

/* sub_50c584: a key's handler called, at most one at a time, by the code in al or, with
 * al 0, the scan code in ah; the tables hold the handler's number, 0 for none. */
x86::reg32 frontend::watb::dispatchKey(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    constexpr reg32 kNotInHandler = 0x56879a;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xc);
    const reg32 key = cpu.eax;
    const reg32 free = guest.dword(kNotInHandler) - 1;
    guest.setDword(kNotInHandler, free);
    reg32 number = 0;
    if (free == 0)
    {
        if (std::uint8_t(key) != 0)
            number = guest.byte((key & 0x7f) + 0x568694);
        else
        {
            reg32 scan = (key >> 8) & 0xff;
            if (sreg32(scan) >= 0x84)
                scan = 0x84;
            number = guest.byte(scan + 0x568714);
        }
        if (number != 0)
        {
            guest.call(guest.dword((number - 1) * 4 + 0x56879e), args({ key, number - 1 }));
            guest.setDword(kNotInHandler, guest.dword(kNotInHandler) + 1);
            return 0;
        }
    }
    guest.setDword(kNotInHandler, guest.dword(kNotInHandler) + 1);
    return key;
}

void frontend::watbDispatchKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c584", 0, 0, fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::dispatchKey(app, cpu, cpu.eax); });
}

/* sub_50c5d4: a handler given a number, a free one or its own, and the key eax set to it. */
x86::reg32 frontend::watb::registerKey(App* app, CPU& cpu, reg32 argEax, reg32 argEdx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    Guest guest(app, cpu);
    constexpr reg32 kHandlers = 0x56879e;
    const reg32 key = cpu.eax, handler = cpu.edx;
    reg32 number = 0;
    for (reg32 i = 0; i != 0x40; ++i)
    {
        const reg32 slot = kHandlers + i * 4;
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
        guest.leaveEdx(kHandlers + 0x100);
        return handler;
    }
    if (std::uint8_t(key) != 0)
    {
        guest.leaveEdx(key);
        if (sreg32(key) <= 0x7f)
            guest.setByte(key + 0x568694, std::uint8_t(number));
    }
    else
    {
        const reg32 scan = (key >> 8) & 0xff;
        guest.leaveEdx(scan);
        if (sreg32(scan) <= 0x84)
            guest.setByte(scan + 0x568714, std::uint8_t(number));
    }
    return number;
}

void frontend::watbRegisterKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50c5d4", 0, 0, kLeavesEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::registerKey(app, cpu, cpu.eax, cpu.edx); });
}

// ---------------------------------------------------------------------------
// Spans in alpha, drawn by sub_50db30
// ---------------------------------------------------------------------------

/* sub_50d560: a memory copy, 32 bytes at a time through the x87 where it can.  fld and
 * fstp qword only move the bits, as the generated code's doubles do, in this order
 * (which shows where the two overlap). */
x86::reg32 frontend::watb::copyBlock(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 to = cpu.edx, from = cpu.eax;
    reg32 left = cpu.ebx;
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
    const auto load = [&](reg32 offset) { return app->getMemory<double>(from + offset); };
    const auto store = [&](reg32 offset, double value) { app->getMemory<double>(to + offset) = value; };
    if ((to & 7) != 0)
    {
        if ((to & 1) != 0 && sreg32(left) >= 1)
            copy1();
        if ((to & 2) != 0 && sreg32(left) >= 2)
            copy2();
        if ((to & 4) != 0 && sreg32(left) >= 4)
            copy4();
    }
    left -= 0x80;
    while (sreg32(left) >= 0)
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
    while (sreg32(left) >= 0)
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
    while (sreg32(left) >= 0)
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
    guest.leaveEbx(left);
    guest.leaveEdx(from);
    return to;
}

void frontend::watbCopyBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d560", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::copyBlock(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50d6e0: ARGBs over palette indices, each result the nearest index. */
x86::reg32 frontend::watb::blendSpanIndexed(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x24);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    reg32 result = cpu.eax, leftEbx = cpu.ebx, leftEdx = cpu.edx;
    do
    {
        const std::uint8_t alpha = guest.byte(from + 3);
        if (alpha != 0)
        {
            const reg32 under = guest::direct<0x50c3a0, colourToArgb>(app, cpu, reg32(guest.byte(to)));
            leftEdx = under;
            if ((under & 0xff000000) != 0)
            {
                const reg32 show = std::uint8_t(0xff - alpha);
                const reg32 red = std::uint8_t(guest.byte(from + 2) + sar(((under >> 16) & 0xff) * show, 8));
                const reg32 green = std::uint8_t(guest.byte(from + 1) + sar(((under >> 8) & 0xff) * show, 8));
                const reg32 blue = std::uint8_t(guest.byte(from) + sar((under & 0xff) * show, 8));
                leftEbx = red << 16;
                leftEdx = (green << 8) | leftEbx | blue;
            }
            result = fe::direct<0x4ef760, frontend::eab::colourPack>(app, cpu, leftEdx);
            guest.setByte(to, std::uint8_t(result));
        }
        ++to;
        from += 4;
    } while (--count != 0);
    guest.leaveEbx(leftEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watbBlendSpanIndexed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d6e0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendSpanIndexed(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50d7e0: ARGBs over 1555 pixels. */
x86::reg32 frontend::watb::blendSpan1555(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    Scratch left{ cpu.eax, cpu.edx };
    reg32 lastEbx = cpu.ebx;
    do
    {
        reg32 argb = guest.dword(from);
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
    guest.leaveEbx(lastEbx);
    guest.leaveEdx(left.edx);
    return left.eax;
}

void frontend::watbBlendSpan1555(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d7e0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendSpan1555(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50d880: ARGBs over 565 pixels. */
x86::reg32 frontend::watb::blendSpan565(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    Scratch left{ cpu.eax, cpu.edx };
    reg32 lastEbx = cpu.ebx;
    do
    {
        reg32 argb = guest.dword(from);
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
    guest.leaveEbx(lastEbx);
    guest.leaveEdx(left.edx);
    return left.eax;
}

void frontend::watbBlendSpan565(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d880", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendSpan565(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50d910: ARGBs over 888 pixels. */
x86::reg32 frontend::watb::blendSpan888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    reg32 lastEbx = cpu.ebx;
    do
    {
        const std::uint8_t alpha = guest.byte(from + 3);
        if (alpha != 0)
        {
            const reg32 show = std::uint8_t(0xff - alpha);
            const std::uint8_t two = std::uint8_t(guest.byte(from + 2) + sar(guest.byte(to + 2) * show, 8));
            const std::uint8_t one = std::uint8_t(guest.byte(from + 1) + sar(guest.byte(to + 1) * show, 8));
            const std::uint8_t zero = std::uint8_t(guest.byte(from) + sar(guest.byte(to) * show, 8));
            guest.setByte(to + 1, one);
            guest.setByte(to + 2, two);
            guest.setByte(to, zero);
            lastEbx = (lastEbx & 0xffff0000) | (reg32(zero) << 8) | two;
        }
        to += 3;
        from += 4;
    } while (--count != 0);
    guest.leaveEbx(lastEbx);
    guest.leaveEdx(to);
    return from;
}

void frontend::watbBlendSpan888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d910", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendSpan888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50d9c0: ARGBs over ARGBs; over nothing, the ARGB as it is. */
x86::reg32 frontend::watb::blendSpan8888(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    reg32 lastEbx = cpu.ebx;
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
                const reg32 show = std::uint8_t(0xff - alpha);
                two = std::uint8_t(two + sar(guest.byte(to + 2) * show, 8));
                one = std::uint8_t(one + sar(guest.byte(to + 1) * show, 8));
                zero = std::uint8_t(zero + sar(guest.byte(to) * show, 8));
                outAlpha = std::uint8_t(0xff - sar(show * (0xff - reg32(underAlpha)), 8));
            }
            guest.setByte(to + 3, outAlpha);
            guest.setByte(to + 2, two);
            guest.setByte(to + 1, one);
            guest.setByte(to, zero);
            lastEbx = (lastEbx & 0xffff0000) | (reg32(zero) << 8) | zero;
        }
        to += 4;
        from += 4;
    } while (--count != 0);
    guest.leaveEbx(lastEbx);
    guest.leaveEdx(to);
    return from;
}

void frontend::watbBlendSpan8888(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50d9c0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::blendSpan8888(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50daa0: palette indices over 565 pixels, by a palette of the alpha over 565 at
 * [0xa0b020] (sub_50a830's). */
x86::reg32 frontend::watb::blendSpanPalette565(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    reg32 from = cpu.eax, to = cpu.edx;
    reg32 count = cpu.ebx;
    reg32 result = cpu.eax, lastEbx = cpu.ebx, leftEdx = cpu.edx;
    do
    {
        const reg32 scaled = reg32(guest.byte(from)) * 4;
        result = scaled;
        const reg32 argb = guest.dword(guest.dword(0xa0b020) + scaled);
        lastEbx = argb;
        ++from;
        if (argb >= kVisibleFrom)
        {
            if (argb < kOpaqueFrom)
            {
                const reg32 pixel = guest.word(to);
                const reg32 spread = ((pixel << 16) & 0xf8000000) + (pixel & 0x1f) + ((pixel & 0x7e0) << 8);
                const std::uint64_t product = std::uint64_t(spread) * ((~argb) >> 24);
                const reg32 low = reg32(product), high = reg32(product >> 32);
                leftEdx = (high << 8) & 0xf800;
                result = (low >> 16) & 0x7e0;
                lastEbx = argb + leftEdx + result + ((low >> 8) & 0x1f);
            }
            guest.setWord(to, std::uint16_t(lastEbx));
        }
        to += 2;
    } while (--count != 0);
    guest.leaveEbx(lastEbx);
    guest.leaveEdx(leftEdx);
    return result;
}

void frontend::watbBlendSpanPalette565(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50daa0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::blendSpanPalette565(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    });
}

/* sub_50db30: a shape in alpha by movdfl with the table at 0x5688a8: an 8-bit one on a
 * 16-bit window through its palette and sub_50daa0. */
x86::reg32 frontend::watb::drawAlphaShape(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x410);
    constexpr reg32 kBlitter16 = 0x5688c0, kType = 0x5688c8;
    const reg32 frame = guest.frame();
    const reg32 shape = cpu.eax, x = cpu.edx, y = cpu.ebx;
    guest.setDword(kBlitter16, 0);
    guest.setDword(kType, 0x7d);
    if (shapeDepth(guest, shape) == 8 && guest.byte(kWindowDepth) == 0x10)
    {
        guest.setDword(0xa0b020, guest::direct<0x50a830, clut16aPointer>(app, cpu, shape, frame));
        guest.setDword(kBlitter16, 0x50daa0);
        guest.setDword(kType, 0x7b);
    }
    return guest.call(0x50b0b0, args({ shape, x, y, 0x5688a8 }));
}

void frontend::watbDrawAlphaShape(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50db30", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::drawAlphaShape(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

/* sub_50e5a0, qfetch: under the queue's lock, the first element edx's test says yes to,
 * taken off the queue. */
x86::reg32 frontend::watb::queueFetch(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588;
    const reg32 queue = cpu.eax, test = cpu.edx, argument = cpu.ebx;
    const reg32 lock = guest.dword(queue + 0x18);
    guest.leaveEdx(lock);
    callImport<kEnterLock, &host::enterLock>(guest, cpu, lock);
    if (guest.dword(queue + 0x18) == 0)
    {
        guest.leaveEdx(0x36a);
        reportError(guest, 0x54fbb8, 0x54fc94, 0x36a, args({}, { 0x54fbcc, 0x54fc94, queue }));
    }
    reg32 element = guest.dword(queue + 8);
    while (element != 0 && test != 0)
    {
        guest.leaveEbx(element);
        if (guest.call(test, args({ element, argument })) != 0)
            break;
        element = guest.dword(element);
    }
    if (element != 0)
    {
        if (directKept<0x50dc20, ea::queueUnlinked, 2>(guest.app(), guest.cpu(), queue, element) == 0)
        {
            reportError(guest, 0x54fbb8, 0x54fc94, 0x373, args({}, { 0x54fc9c }));
            element = 0;
        }
    }
    guest.leaveEbx(element);
    callImport<kLeaveLock, &host::leaveLock>(guest, cpu, guest.dword(queue + 0x18));
    return element;
}

void frontend::watbQueueFetch(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_50e5a0", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::queueFetch(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

// ---------------------------------------------------------------------------
// The runtime: the streams, the command line
// ---------------------------------------------------------------------------

namespace
{
constexpr reg32 kIob = 0x566f58, kFileBytes = 0x1a;
constexpr reg32 kOpenStreams = 0x9f38a0, kClosedStreams = 0x9f38a4;
constexpr reg32 kNmalloc = 0x4f7900, kNfree = 0x4f79f0;
constexpr reg32 kLinkBytes = 0x1d;
}

/* sub_509350: the closed streams' links freed. */
x86::reg32 frontend::watb::freeClosedStreams(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    while (guest.dword(kClosedStreams) != 0)
    {
        const reg32 link = guest.dword(kClosedStreams);
        const reg32 next = guest.dword(link);
        fe::direct<kNfree, watcom::clib::free>(guest.app(), guest.cpu(), link);
        guest.setDword(kClosedStreams, next);
        if (next == 0)
            break;
    }
    return guest.eax();
}

void frontend::watbFreeClosedStreams(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_509350", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::freeClosedStreams(app, cpu); });
}

/* sub_515740, __InitFiles: a link for each FILE of __iob in use, stderr's flags first. */
x86::reg32 frontend::watb::initStreams(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kStderrFlags = 0x566f99;
    const std::uint8_t flags = guest.byte(kStderrFlags) & 0xf8;
    guest.setByte(kStderrFlags, flags | 4);
    reg32 result = (cpu.eax & 0xffff00ff) | (reg32(flags) << 8);
    reg32 file = kIob;
    reg32 inUse = guest.dword(kIob + 0xc);
    while (inUse != 0)
    {
        reg32 link = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), kLinkBytes);
        if (link == 0)
        {
            link = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), kLinkBytes);
            if (link == 0)
                guest.call(0x511d0c, args({ 0x55057c, 1 }));
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

void frontend::watbInitStreams(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_515740", 0, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::initStreams(app, cpu, cpu.eax); });
}

/* sub_5157e0, __full_io_exit: every stream closed, the links freed. */
x86::reg32 frontend::watb::closeStreams(App* app, CPU& cpu)
{
    try
    {
        // eax its result: sub_509350 hands it back as it came when no stream is closed.
        cpu.eax = guest::direct<0x5157f4, closeStreamsFrom>(app, cpu, 0);
    }
    catch (const fe::Terminated&)
    {
        return cpu.eax;
    }
    // Jumped to, by address: it returns for this one.
    app->dynamic_call(0x509350, cpu);
    return cpu.eax;
}

void frontend::watbCloseStreams(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::closeStreams(app, cpu);
}

/* sub_5157f4: the open streams from the eax'th of __iob on shut down; __iob's first
 * three only if they are not those. */
x86::reg32 frontend::watb::closeStreamsFrom(App* app, CPU& cpu, reg32 argEax)
{
    cpu.eax = argEax;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x10);
    constexpr reg32 kIobStandardEnd = 0x566fa6;
    const reg32 first = kIob + cpu.eax * kFileBytes;
    reg32 closed = 0;
    reg32 link = guest.dword(kOpenStreams);
    while (link != 0)
    {
        const reg32 next = guest.dword(link);
        const reg32 file = guest.dword(link + 4);
        const std::uint8_t flags = guest.byte(file + 0xd);
        reg32 close = 1;
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
            guest.call(0x4ee144, args({ file, close }));
            ++closed;
        }
        link = next;
    }
    return closed;
}

void frontend::watbCloseStreamsFrom(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5157f4", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::closeStreamsFrom(app, cpu, cpu.eax); });
}

/* sub_515900: the command line at [0x5677fc] split into argv: counted, copied, split again. */
x86::reg32 frontend::watb::buildArgv(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x24);
    constexpr reg32 kCommandLine = 0x5677fc, kProgramName = 0x567800;
    constexpr reg32 kArgv = 0x9f38c0, kArgc = 0x9f38bc;
    const reg32 frame = guest.frame();
    const reg32 words = guest::direct<0x5159e4, splitCommandLine>(app, cpu, guest.dword(kCommandLine), reg32(0), frame) + 1;
    const reg32 length = guest.dword(frame) - guest.dword(kCommandLine);
    const reg32 textBytes = (length + 4) & ~reg32(3);
    const reg32 argvBytes = words << 2;
    const reg32 memory = fe::direct<kNmalloc, watcom::clib::malloc>(guest.app(), guest.cpu(), (argvBytes + 4 + textBytes + 3) & ~reg32(3));
    reg32 count = 0;
    if (memory != 0)
    {
        guest.setDword(kArgv, memory + textBytes);
        copyRep(guest, memory, guest.dword(kCommandLine), length + 1);
        const reg32 argv = guest.dword(kArgv);
        guest.setDword(argv, guest.dword(kProgramName));
        guest::direct<0x5159e4, splitCommandLine>(app, cpu, memory, argv + 4, frame);
        guest.setDword(guest.dword(kArgv) + argvBytes, 0);
        count = words;
    }
    else
        guest.setDword(kArgv, 0);
    guest.setDword(kArgc, count);
    guest.setDword(0xa18210, guest.dword(kArgc));
    guest.setDword(0xa18214, guest.dword(kArgv));
    guest.setDword(0x9f38c4, guest.dword(0xa18210));
    const reg32 last = guest.dword(0xa18214);
    guest.setDword(0x9f38c8, last);
    return last;
}

void frontend::watbBuildArgv(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_515900", 0, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::buildArgv(app, cpu); });
}

/* sub_5159e4: the command line at eax split into words: their starts at edx if it is
 * not 0, each made a string in place; the end's place at [ebx]; how many.  A word is
 * quoted or not, with Microsoft's backslashes, or Watcom's where [0xa18218] says. */
x86::reg32 frontend::watb::splitCommandLine(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    Guest guest(app, cpu);
    constexpr reg32 kBackslashStyle = 0xa18218;
    reg32 text = cpu.eax;
    const reg32 words = cpu.edx, endPlace = cpu.ebx;
    const reg32 style = guest.dword(kBackslashStyle);
    reg32 count = 0;
    reg32 out = cpu.ebx;
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
        const reg32 start = text;
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
    guest.leaveEbx(out);
    guest.leaveEdx(endPlace);
    return count;
}

void frontend::watbSplitCommandLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_5159e4", 0, 0, kLeavesEbxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) { return watb::splitCommandLine(app, cpu, cpu.eax, cpu.edx, cpu.ebx); });
}

// ---------------------------------------------------------------------------
// __LDcvt: a long double's digits, for printf's %e, %f and %g (0x5167b3).  Its parts
// lean on the frame ebp holds, and are entered at their own addresses too, so it keeps
// its registers and its frame as they are: esi the CVT, ecx the decimal point, edi what
// is yet to be written as digits.
// ---------------------------------------------------------------------------

namespace
{

constexpr reg32 kCvtDigits = 0, kCvtScale = 4, kCvtFlags = 8, kCvtExpWidth = 0x10, kCvtSign = 0x14;
constexpr reg32 kCvtN1 = 0x18, kCvtNZ1 = 0x1c, kCvtN2 = 0x20, kCvtNZ2 = 0x24, kCvtExtra = 0x28;

// Below ebp.
constexpr reg32 kFrameControl = 0x14, kFrameWanted = 0x18, kFrameBuffer = 0x20;
constexpr reg32 kFrameCount = 0x24, kFrameWrite = 0x28, kFrameRoundTo = 0x10;
constexpr reg32 kFrameValue = 0x34, kFrameTemp = 0x4c, kFrameQuotient = 0x58, kFrameDigits = 0x98;

void pushDword(Guest& guest, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    guest.setDword(cpu.esp, value);
}

reg32 popDword(Guest& guest, x86::CPU& cpu)
{
    const reg32 value = guest.dword(cpu.esp);
    cpu.esp += 4;
    return value;
}

x86::Float load80(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<x86::IEEEf80>(address));
}

void store80(win32::WinApplication* app, reg32 address, x86::Float value)
{
    app->getMemory<x86::IEEEf80>(address) = x86::Float(value);
}

/* 1e8, as the long double the code builds at [address]. */
void hundredMillion(Guest& guest, reg32 address)
{
    guest.setWord(address + 8, 0x4019);
    guest.setDword(address + 4, 0xbebc2000);
    guest.setDword(address, 0);
}

/* fistp with the rounding towards zero its code sets round it. */
reg32 truncatedToInteger(x86::CPU& cpu, x86::Float value)
{
    const x86::reg16 word = cpu.fpu.control.word;
    cpu.fpu.setControl(x86::reg16(word | 0xc00));
    const reg32 integer = reg32(cpu.fpu.toInteger<sreg32>(value));
    cpu.fpu.setControl(word);
    return integer;
}

/* sub_516ab4: two digits of ax at ebx. */
void writeTwoDigits(Guest& guest, x86::CPU& cpu)
{
    cpu.ecx = (cpu.ecx & 0xffffff00) | 10;
    const std::uint8_t low = std::uint8_t(cpu.eax), high = std::uint8_t(cpu.eax >> 8);
    std::uint8_t tens, ones;
    if (low < 10)
    {
        tens = high;
        ones = low;
    }
    else
    {
        const std::uint16_t value = std::uint16_t(cpu.eax);
        tens = std::uint8_t(value / 10);
        ones = std::uint8_t(value % 10);
    }
    tens = std::uint8_t(tens + '0');
    ones = std::uint8_t(ones + '0');
    cpu.eax = (cpu.eax & 0xffff0000) | (reg32(ones) << 8) | tens;
    guest.setByte(cpu.ebx, tens);
    guest.setByte(cpu.ebx + 1, ones);
    cpu.ebx += 2;
}

/* sub_516a9d: four. */
void writeFourDigits(Guest& guest, x86::CPU& cpu)
{
    cpu.ecx = 100;
    const reg32 value = cpu.eax;
    cpu.eax = value / 100;
    cpu.edx = value % 100;
    const reg32 rest = cpu.edx;
    writeTwoDigits(guest, cpu);
    cpu.eax = rest;
    writeTwoDigits(guest, cpu);
}

/* sub_516a87: eight. */
void writeEightDigits(Guest& guest, x86::CPU& cpu)
{
    const reg32 value = cpu.eax;
    cpu.eax = value / 10000;
    writeFourDigits(guest, cpu);
    cpu.eax = value % 10000;
    writeFourDigits(guest, cpu);
}

/* sub_516be2: the control word as it was, and out. */
void endLongDouble(Guest& guest, x86::CPU& cpu)
{
    const reg32 word = guest.word(cpu.ebp - kFrameControl);
    cpu.fpu.setControl(x86::reg16(word));
    cpu.eax = word;
    cpu.esp = cpu.ebp - 0xc;
    cpu.edi = popDword(guest, cpu);
    cpu.esi = popDword(guest, cpu);
    cpu.ecx = popDword(guest, cpu);
    cpu.ebp = popDword(guest, cpu);
    cpu.esp += 4;
}

/* sub_516ae2: the digits rounded to what is asked, sub_516bf5 or sub_516e13 to lay them out. */
void roundAndLayOut(Guest& guest, x86::CPU& cpu)
{
    constexpr reg32 kFormatF = 0x516bf5, kFormatE = 0x516e13;
    const reg32 frame = cpu.ebp;
    const reg32 cvt = cpu.esi;
    reg32 available = guest.dword(frame - kFrameCount);
    reg32 digits = frame - kFrameDigits + 1;
    reg32 point = cpu.ecx + 7;
    while (guest.byte(digits) == '0')
    {
        --available;
        --point;
        ++digits;
    }
    const std::uint8_t mode = guest.byte(cvt + kCvtFlags);
    reg32 keep = guest.dword(cvt + kCvtDigits);
    if ((mode & 2) != 0)
    {
        point += guest.dword(cvt + kCvtScale);
        keep += point + 1;
    }
    else if ((mode & 1) != 0)
    {
        const reg32 scale = guest.dword(cvt + kCvtScale);
        if (positive(scale))
            ++keep;
        else
            keep += scale;
        ++point;
        point -= guest.dword(cvt + kCvtScale);
    }
    if (sreg32(keep) >= 0)
    {
        if (sreg32(keep) > sreg32(available))
            keep = available;
        reg32 most = 0xf;
        if ((guest.byte(cvt + kCvtFlags) & 0x20) != 0)
            most = 0x14;
        if ((guest.byte(cvt + kCvtFlags) & 0x40) != 0)
            most += most;
        if (sreg32(keep) > sreg32(most))
            keep = most + 1;
        std::uint8_t dropTo = '0';
        guest.setByte(frame - kFrameRoundTo, dropTo);
        if (sreg32(available) > sreg32(keep) && guest.byte(digits + keep) >= '5')
        {
            dropTo = '9';
            guest.setByte(frame - kFrameRoundTo, dropTo);
        }
        reg32 place = keep;
        reg32 at = keep + digits;
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
        if (sreg32(place) < 0)
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
    cpu.edi = keep;
    const std::uint8_t flags = guest.byte(cvt + kCvtFlags);
    bool plain = true;
    if ((flags & 2) == 0)
    {
        plain = false;
        if ((flags & 4) != 0)
        {
            if (sreg32(point) >= -4 && sreg32(point) < sreg32(guest.dword(cvt + kCvtDigits)))
                plain = true;
            else
                plain = (guest.byte(cvt + kCvtFlags) & 8) != 0;
        }
    }
    const reg32 buffer = guest.dword(frame - kFrameBuffer);
    guest.call(plain ? kFormatF : kFormatE, args({ cvt, digits, keep, point }, { buffer }));
    endLongDouble(guest, cpu);
}

/* sub_5169ef: eight digits at a time while more are wanted and any are left. */
void writeDigits(Guest& guest, win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kEightDigits = 0x516a87;
    const reg32 frame = cpu.ebp;
    for (;;)
    {
        const reg32 wanted = guest.dword(frame - kFrameWanted);
        if (!positive(wanted))
            break;
        const reg32 more = wanted - 8;
        guest.setDword(frame - kFrameWanted, more);
        if (cpu.edi == 0)
        {
            if ((guest.word(frame - kFrameValue + 8) & 0x7fff) == 0)
                break;
            const reg32 value = frame - kFrameValue;
            cpu.edi = truncatedToInteger(cpu, load80(app, value));
            if (positive(more))
            {
                const reg32 temp = frame - 0x40;
                store80(app, temp, x86::Float(sreg32(cpu.edi)));
                store80(app, value, cpu.fpu.sub(load80(app, value), load80(app, temp)));
                hundredMillion(guest, temp);
                store80(app, value, cpu.fpu.mul(load80(app, value), load80(app, temp)));
            }
        }
        const reg32 count = guest.dword(frame - kFrameCount);
        cpu.ebx = guest.dword(frame - kFrameWrite);
        pushDword(guest, cpu, cpu.ecx);
        pushDword(guest, cpu, count);
        guest.call(kEightDigits, args({ cpu.edi }));
        const reg32 kept = popDword(guest, cpu);
        cpu.ecx = popDword(guest, cpu);
        guest.setByte(cpu.ebx, 0);
        guest.setDword(frame - kFrameWrite, cpu.ebx);
        guest.setDword(frame - kFrameCount, kept + 8);
        cpu.edi = 0;
    }
    roundAndLayOut(guest, cpu);
}

}

/* sub_5167b3, __LDcvt: the long double at eax into the CVT at edx, its digits for the
 * layouts in the buffer at ebx. */
x86::reg32 frontend::watb::longDoubleToDigits(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    constexpr reg32 kScale = 0x516759;
    Guest guest(app, cpu);
    try
    {
        pushDword(guest, cpu, cpu.ebp);
        cpu.ebp = cpu.esp;
        pushDword(guest, cpu, cpu.ecx);
        pushDword(guest, cpu, cpu.esi);
        pushDword(guest, cpu, cpu.edi);
        cpu.esp -= 0x8c;
        const reg32 frame = cpu.ebp;
        const reg32 from = cpu.eax;
        cpu.esi = cpu.edx;
        const reg32 cvt = cpu.esi;
        guest.setDword(frame - kFrameBuffer, cpu.ebx);
        const reg32 word = cpu.fpu.control.word;
        guest.setDword(frame - kFrameControl, word);
        cpu.fpu.setControl(x86::reg16(word | 0x300));

        guest.setDword(cvt + kCvtSign, 0);
        const reg32 value = frame - kFrameValue;
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
        cpu.edi = 0;

        const reg32 kind = fe::direct<0x52448c, frontend::menz::longDoubleKind>(app, cpu, value);
        const auto writeWord = [&](char a, char b, char c) {
            const reg32 buffer = guest.dword(frame - kFrameBuffer);
            guest.setByte(buffer, std::uint8_t(a));
            guest.setByte(buffer + 1, std::uint8_t(b));
            guest.setByte(buffer + 2, std::uint8_t(c));
            guest.setByte(buffer + 3, 0);
            guest.setDword(cvt + kCvtNZ1, 3);
            endLongDouble(guest, cpu);
        };
        switch (kind)
        {
        case 0:
        case 4:
            // Zero.
            guest.setDword(cvt + kCvtSign, 0);
            cpu.ecx = 0;
            break;
        case 2:
            writeWord('n', 'a', 'n');
            return cpu.eax;
        case 3:
            writeWord('i', 'n', 'f');
            return cpu.eax;
        case 1:
        {
            // Scaled by powers of ten to about eight digits before the point.
            const std::uint16_t exponent = guest.word(value + 8);
            const sreg32 guess = sreg32(reg32(exponent) - 0x3ffe) * 0x7597 / 0x186a0;
            cpu.ecx = reg32(guess - 4);
            if (cpu.ecx == 0)
                break;
            if (sreg32(cpu.ecx) < 0)
            {
                cpu.ecx = reg32(0) - ((reg32(0) - cpu.ecx + 3) & ~reg32(3));
                guest.call(kScale, args({ value, reg32(0) - cpu.ecx }));
                break;
            }
            const reg32 high = guest.dword(value + 4);
            if (exponent < 0x4019 || (exponent == 0x4019 && high < 0xbebc2000))
            {
                cpu.ecx = 0;
                break;
            }
            const bool below = exponent < 0x4034
                || (exponent == 0x4034
                    && (high < 0x8e1bc9bf || (high == 0x8e1bc9bf && guest.dword(value) < 0x4000000)));
            if (below)
            {
                // Under 1e16: the first eight digits by a division, the rest by what is left.
                const reg32 temp = frame - kFrameTemp, quotient = frame - kFrameQuotient;
                hundredMillion(guest, temp);
                store80(app, quotient, cpu.fpu.div(load80(app, value), load80(app, temp)));
                cpu.edi = truncatedToInteger(cpu, load80(app, quotient));
                store80(app, quotient, x86::Float(sreg32(cpu.edi)));
                store80(app, temp, cpu.fpu.mul(load80(app, quotient), load80(app, temp)));
                store80(app, value, cpu.fpu.sub(load80(app, value), load80(app, temp)));
                cpu.ecx = 8;
                break;
            }
            cpu.ecx &= ~reg32(3);
            guest.call(kScale, args({ value, reg32(0) - cpu.ecx }));
            break;
        }
        default:
            break;
        }

        reg32 wanted;
        const std::uint8_t mode = guest.byte(cvt + kCvtFlags);
        if ((mode & 2) != 0)
        {
            wanted = guest.dword(cvt + kCvtDigits) + cpu.ecx + 0xa;
            const reg32 scale = guest.dword(cvt + kCvtScale);
            if (positive(scale))
                wanted += scale;
        }
        else
            wanted = guest.dword(cvt + kCvtDigits) + 7;
        reg32 most = (guest.byte(cvt + kCvtFlags) & 0x20) != 0 ? 0x14 : 0xf;
        if ((guest.byte(cvt + kCvtFlags) & 0x40) != 0)
            most += most;
        most += 4;
        if (sreg32(most) < sreg32(wanted))
            wanted = most;
        guest.setDword(frame - kFrameWanted, wanted);
        guest.setByte(frame - kFrameDigits, '0');
        guest.setByte(frame - kFrameDigits + 1, 0);
        guest.setDword(frame - kFrameWrite, frame - kFrameDigits + 1);
        guest.setDword(frame - kFrameCount, 0);
        writeDigits(guest, app, cpu);
    }
    catch (const fe::Terminated&)
    {
    }
    return cpu.eax;
}

void frontend::watbLongDoubleToDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::longDoubleToDigits(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
}

/* The parts of it: entered with its frame. */
void frontend::watbDigitsLoop(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    try
    {
        writeDigits(guest, app, cpu);
    }
    catch (const fe::Terminated&)
    {
    }
}

void frontend::watbAfterEightDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    try
    {
        const reg32 count = popDword(guest, cpu);
        cpu.ecx = popDword(guest, cpu);
        guest.setByte(cpu.ebx, 0);
        guest.setDword(cpu.ebp - kFrameWrite, cpu.ebx);
        guest.setDword(cpu.ebp - kFrameCount, count + 8);
        cpu.edi = 0;
        writeDigits(guest, app, cpu);
    }
    catch (const fe::Terminated&)
    {
    }
}

void frontend::watbRoundDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    try
    {
        roundAndLayOut(guest, cpu);
    }
    catch (const fe::Terminated&)
    {
    }
}

void frontend::watbEndLongDouble(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    endLongDouble(guest, cpu);
}

/* sub_516a87, sub_516a9d, sub_516ab4: eax's last eight, four and two decimal digits at
 * ebx, which they move on. */
x86::reg32 frontend::watb::eightDigits(App* app, CPU& cpu, reg32 value, reg32 at)
{
    cpu.eax = value;
    cpu.ebx = at;
    Guest guest(app, cpu);
    writeEightDigits(guest, cpu);
    return cpu.eax;
}

void frontend::watbEightDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::eightDigits(app, cpu, cpu.eax, cpu.ebx);
    cpu.esp += 4;
}

x86::reg32 frontend::watb::fourDigits(App* app, CPU& cpu, reg32 value, reg32 at)
{
    cpu.eax = value;
    cpu.ebx = at;
    Guest guest(app, cpu);
    writeFourDigits(guest, cpu);
    return cpu.eax;
}

void frontend::watbFourDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::fourDigits(app, cpu, cpu.eax, cpu.ebx);
    cpu.esp += 4;
}

x86::reg32 frontend::watb::twoDigits(App* app, CPU& cpu, reg32 value, reg32 at)
{
    cpu.eax = value;
    cpu.ebx = at;
    Guest guest(app, cpu);
    writeTwoDigits(guest, cpu);
    return cpu.eax;
}

void frontend::watbTwoDigits(win32::WinApplication* app, x86::CPU& cpu)
{
    watb::twoDigits(app, cpu, cpu.eax, cpu.ebx);
    cpu.esp += 4;
}

/* sub_516bf5: %f's layout in the buffer on the stack: digits at edx, ebx of them, the
 * point after ecx + 1, what the CVT at eax asks; how many zeros go where, in the CVT. */
x86::reg32 frontend::watb::formatF(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                   reg32 bufferWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x1c);
    const reg32 cvt = cpu.eax, digits = cpu.edx;
    reg32 count = cpu.ebx;
    reg32 point = cpu.ecx + 1;
    const reg32 buffer = bufferWord;
    reg32 precision = guest.dword(cvt + kCvtDigits);
    reg32 out = 0;
    reg32 leftEbx = cvt, leftEcx = 0, leftEax = 0;
    const std::uint8_t flags = guest.byte(cvt + kCvtFlags);
    if ((flags & 4) != 0)
    {
        if (sreg32(count) < sreg32(precision) && (guest.byte(cvt + kCvtFlags) & 0x10) == 0)
            precision = count;
        precision -= point;
        if (sreg32(precision) < 0)
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
        if (sreg32(reg32(0) - point) > sreg32(precision))
            point = reg32(0) - precision;
        guest.setDword(cvt + kCvtN1, point);
        guest.setDword(cvt + kCvtN2, reg32(0) - point);
        precision += point;
        if (sreg32(count) > sreg32(precision))
            count = precision;
        copyRep(guest, buffer + out, digits, count);
        leftEax = count;
        guest.setDword(cvt + kCvtNZ2, count);
        out += count;
        guest.setDword(cvt + kCvtExtra, precision - count);
    }
    else if (sreg32(count) < sreg32(point))
    {
        copyRep(guest, buffer, digits, count);
        leftEax = count;
        guest.setDword(cvt + kCvtNZ1, count);
        out = count;
        guest.setDword(cvt + kCvtN2, point - count);
        leftEcx = guest.byte(cvt + kCvtFlags);
        guest.setDword(cvt + kCvtN1, point);
        if ((leftEcx & 8) == 0 && (positive(precision) || (leftEcx & 0x10) != 0))
        {
            leftEcx = buffer;
            guest.setByte(buffer + out, '.');
            ++out;
            guest.setDword(cvt + kCvtNZ2, 1);
        }
        guest.setDword(cvt + kCvtExtra, precision);
        leftEbx = precision;
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
        if (sreg32(count) > sreg32(precision))
            count = precision;
        copyRep(guest, buffer + out, digits + point, count);
        leftEax = count;
        out += count;
        guest.setDword(cvt + kCvtNZ1, out);
        guest.setDword(cvt + kCvtN2, precision - count);
    }
    guest.setByte(buffer + out, 0);
    guest.leaveEbx(leftEbx);
    guest.leaveEcx(leftEcx);
    guest.leaveEdx(buffer + out);
    return leftEax;
}

void frontend::watbFormatF(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_516bf5", 4, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::formatF(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4));
    });
}

/* sub_516dfa: the end of sub_516bf5, which sub_516e13 jumps to. */
void frontend::watbEndFormat(win32::WinApplication* app, x86::CPU& cpu)
{
    Guest guest(app, cpu);
    cpu.esp = cpu.ebp - 8;
    cpu.edi = popDword(guest, cpu);
    cpu.esi = popDword(guest, cpu);
    cpu.ebp = popDword(guest, cpu);
    cpu.esp += 4 + 4;
}

/* sub_516e13: %e's layout: digits, zeros, the point, more, the letter, the exponent's
 * sign and its digits, as few as the CVT's +0x10 asks and it needs. */
x86::reg32 frontend::watb::formatE(App* app, CPU& cpu, reg32 argEax, reg32 argEdx, reg32 argEbx, reg32 argEcx,
                                   reg32 bufferWord)
{
    cpu.eax = argEax;
    cpu.edx = argEdx;
    cpu.ebx = argEbx;
    cpu.ecx = argEcx;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x28);
    constexpr reg32 kMemset = 0x4e0640;
    const reg32 cvt = cpu.eax;
    reg32 digits = cpu.edx, count = cpu.ebx, exponent = cpu.ecx;
    const reg32 buffer = bufferWord;
    reg32 precision = guest.dword(cvt + kCvtDigits);
    reg32 out = 0;
    reg32 leftEcx = cpu.ecx;
    const reg32 scale = guest.dword(cvt + kCvtScale);
    if (!positive(scale))
        precision += scale;
    else
    {
        precision = precision - scale + 1;
        leftEcx = precision;
    }
    if ((guest.byte(cvt + kCvtFlags) & 4) != 0)
    {
        if (sreg32(count) < sreg32(precision))
            precision = count;
        --precision;
        if (sreg32(precision) < 0)
            precision = 0;
    }
    if (!positive(guest.dword(cvt + kCvtScale)))
    {
        guest.setByte(buffer, '0');
        out = 1;
    }
    else
    {
        const reg32 first = guest.dword(cvt + kCvtScale);
        const reg32 taken = sreg32(first) > sreg32(count) ? count : first;
        copyRep(guest, buffer, digits, taken);
        digits += taken;
        count -= taken;
        out = taken;
        leftEcx = taken;
        const reg32 scaleNow = guest.dword(cvt + kCvtScale);
        if (sreg32(taken) < sreg32(scaleNow))
        {
            const reg32 zeros = scaleNow - taken;
            fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), buffer + out, '0', zeros);
            out += zeros;
        }
    }
    guest.setDword(cvt + kCvtN1, out);
    if ((guest.byte(cvt + kCvtFlags) & 8) == 0 && (positive(precision) || (guest.byte(cvt + kCvtFlags) & 0x10) != 0))
    {
        guest.setByte(buffer + out, '.');
        ++out;
    }
    const reg32 scaleNow = guest.dword(cvt + kCvtScale);
    if (sreg32(scaleNow) < 0)
    {
        const reg32 zeros = reg32(0) - scaleNow;
        leftEcx = out;
        fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.cpu(), buffer + out, '0', zeros);
        out += zeros;
    }
    if (positive(precision))
    {
        if (sreg32(precision) < sreg32(count))
            count = precision;
        if (count != 0)
        {
            copyRep(guest, buffer + out, digits, count);
            leftEcx = 0;
            out += count;
        }
        guest.setDword(cvt + kCvtNZ1, out);
        guest.setDword(cvt + kCvtN2, precision - count);
    }
    if (guest.dword(cvt + 0xc) != 0)
    {
        leftEcx = buffer;
        guest.setByte(buffer + out, guest.byte(cvt + 0xc));
        ++out;
    }
    if (sreg32(exponent) >= 0)
        guest.setByte(buffer + out, '+');
    else
    {
        exponent = reg32(0) - exponent;
        guest.setByte(buffer + out, '-');
    }
    ++out;

    reg32 width = guest.dword(cvt + kCvtExpWidth);
    switch (width)
    {
    case 0:
        width = sreg32(exponent) >= 1000 ? 4 : 3;
        break;
    case 1:
    case 2:
    case 3:
        if (width == 1 && sreg32(exponent) >= 10)
            width = 2;
        if (width <= 2 && sreg32(exponent) >= 100)
            width = 3;
        if (sreg32(exponent) >= 1000)
            width = 4;
        break;
    default:
        break;
    }
    guest.setDword(cvt + kCvtExpWidth, width);
    reg32 leftEbx = width;
    const auto digit = [&](reg32 unit) {
        reg32 quotient = 0;
        if (sreg32(exponent) >= sreg32(unit))
        {
            quotient = reg32(sreg32(exponent) / sreg32(unit));
            exponent -= quotient * unit;
        }
        guest.setByte(buffer + out, std::uint8_t(quotient + '0'));
        ++out;
        leftEcx = out;
    };
    if (sreg32(width) >= 4)
        digit(1000);
    if (sreg32(width) >= 3)
        digit(100);
    if (sreg32(width) >= 2)
    {
        digit(10);
        leftEbx = out;
        leftEcx = buffer;
    }
    guest.setByte(buffer + out, std::uint8_t(exponent + '0'));
    ++out;
    const reg32 written = out - guest.dword(cvt + kCvtNZ1);
    guest.setDword(cvt + kCvtNZ2, written);
    guest.setByte(buffer + out, 0);
    guest.leaveEbx(leftEbx);
    guest.leaveEcx(leftEcx);
    guest.leaveEdx(written);
    return buffer + out;
}

void frontend::watbFormatE(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site = makeSite("sub_516e13", 4, 0, kLeavesEbxEcxEdx);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return watb::formatE(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, guest::at(app, cpu.esp + 4));
    });
}

}
