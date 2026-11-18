#include "native_frontend_rendf.h"
#include "native_ea_memory.h"
#include "native_ea_files.h"
#include "native_frontend.h"
#include <nfs3hp.h>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
x86::reg32 rendPixels565To1555(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixels1555To565(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixelsPal8To32(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixelsPal8To16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixelsRgb24ToArgb(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixelsSwapRedBlue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendPixelsPal4To16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendSetDisplayMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFontAlloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFontFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 rendFontSetOrigin(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}
}

/* What the voodoo2a path reaches of the code above 0x504000, decompiled:
 *
 *   the 2D pixels   win/convshp.c's assembler loops, a row of a shape from one
 *                   format to another, out of the conversion table at 0x568394
 *                   (sub_50a960, sub_50aad0), and the palette's bytes at driver
 *                   start (sub_503e50);
 *   the display     sub_5089d0, the mode request: on this driver a call through
 *                   the screen functions' pointer, [0x567dc8];
 *   the font        the two allocator entries of the font table at 0x569038 that
 *                   the voodoo2a path calls, and two words of a font's record.
 *
 * All of the game's state stays in guest memory.  Each loop leaves in eax, ebx and
 * edx what its assembler does (the pointers past the row, the count run below zero,
 * what it last read ahead): it saves only the registers it keeps, and Watcom's
 * callers may go on with the rest.
 *
 * Plan step 5: each function a typed core in frontend::rendf (native_frontend_rendf.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the
 * game's name for the calls by address.  The typed cores of other files are called as C++
 * (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::reg32;
using x86::sreg32;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline reg32 callAt(Guest& guest, reg32 function, const fe::Arguments& arguments = fe::Arguments(), reg32 esi = 0,
                    double* floatResult = nullptr)
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<reg32> result = guest::pointerCall(guest.app(), cpu, function, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

constexpr std::uint8_t kKeepsEcxEsi = fe::kRestoreEcx | fe::kRestoreEsi;
constexpr std::uint8_t kKeepsEcxEsiEdi = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;

/* The palette of a shape going to 16 bits: 16-bit entries, a pair of pixels read
 * as a dword two bytes below the higher pixel's, its lower half then overwritten
 * with the lower pixel's. */
constexpr reg32 kPalette16 = 0xa0ac14;
constexpr reg32 kPalette32Pointer = 0x568390;  // the palette of a shape going to 32 bits

/* One pixel, or two side by side in a dword: `upper` the bits above blue. */
reg32 pixels565To1555(reg32 pixels, reg32 upper, reg32 blue)
{
    return ((pixels & upper) >> 1) | (pixels & blue);
}

reg32 pixels1555To565(reg32 pixels, reg32 upper, reg32 blue)
{
    return ((pixels & upper) << 1) | (pixels & blue);
}

reg32 pixelsSwapRedBlue(reg32 pixels)
{
    return ((pixels & 0x7c007c00) >> 10) | (pixels & 0x83e083e0) | ((pixels & 0x001f001f) << 10);
}

/* A count gone below zero, as `js` after its sub reads it. */
bool below(reg32 count)
{
    return sreg32(count) < 0;
}

/* A row of 16-bit pixels through `convert`, sub_5040b8 and sub_504133: one pixel
 * first to bring the destination to four bytes, then two at a time, then what is
 * left.  It leaves eax past the source, edx past the destination, ebx below zero. */
reg32 row16(Guest& guest, reg32 source, reg32 destination, reg32 count, reg32 upper, reg32 blue,
            reg32 (*convert)(reg32, reg32, reg32))
{
    if ((destination & 2) != 0 && sreg32(count) >= 1)
    {
        guest.setWord(destination, std::uint16_t(convert(guest.word(source), upper, blue)));
        source += 2;
        destination += 2;
        count -= 1;
    }
    count -= 2;
    while (!below(count))
    {
        guest.setDword(destination, convert(guest.dword(source), upper | (upper << 16), blue | (blue << 16)));
        source += 4;
        destination += 4;
        count -= 2;
    }
    count += 1;
    while (!below(count))
    {
        guest.setWord(destination, std::uint16_t(convert(guest.word(source), upper, blue)));
        source += 2;
        destination += 2;
        count -= 1;
    }
    return source;
}

}  // namespace

/* sub_5040b8: a row of 16-bit pixels from 5:6:5 to 1:5:5:5. */
x86::reg32 frontend::rendf::row565To1555(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    return row16(guest, source, destination, count, 0xffc0, 0x1f, pixels565To1555);
}

/* sub_504133: a row of 16-bit pixels from 1:5:5:5 to 5:6:5. */
x86::reg32 frontend::rendf::row1555To565(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    return row16(guest, source, destination, count, 0x7fe0, 0x1f, pixels1555To565);
}

/* sub_5042c4: a row of 16-bit pixels with red and blue changed over, two at a time
 * and then the one left. */
x86::reg32 frontend::rendf::rowSwapRedBlue(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    count -= 2;
    while (!below(count))
    {
        guest.setDword(destination, pixelsSwapRedBlue(guest.dword(source)));
        count -= 2;
        source += 4;
        destination += 4;
    }
    count += 1;
    while (!below(count))
    {
        guest.setWord(destination, std::uint16_t(pixelsSwapRedBlue(guest.word(source))));
        count -= 1;
        source += 2;
        destination += 2;
    }
    return source;
}

/* sub_5041ae: a row of 8-bit pixels to 32 bits through the palette at [0x568390]:
 * one pixel at least, however small the count. */
x86::reg32 frontend::rendf::rowPal8To32(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    const reg32 palette = guest.dword(kPalette32Pointer);
    bool more;
    do
    {
        const reg32 index = guest.byte(source);
        more = sreg32(count) > 1;
        count -= 1;
        source += 1;
        destination += 4;
        guest.setDword(destination - 4, guest.dword(palette + index * 4));
    } while (more);
    return source;
}

namespace
{

/* The palette's entries for two pixels: `high`'s the upper half of a dword, `low`'s
 * the lower. */
reg32 palettePair(const Guest& guest, reg32 high, reg32 low)
{
    return (guest.dword(high * 2 + kPalette16 - 2) & 0xffff0000) | guest.word(low * 2 + kPalette16);
}

}  // namespace

/* sub_5041d0: a row of 8-bit pixels to 16 bits: four at a time when the destination
 * is on four bytes, then one at a time.  It reads the next four bytes ahead, the
 * two lower ones before the two pixels are written and the two upper after, and
 * what it last read is what eax, ebx and edx are left with. */
x86::reg32 frontend::rendf::rowPal8To16(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    reg32 first = 0, second = 0, third = 0, fourth = 0;
    reg32 eax = 0;
    count -= 4;
    if ((destination & 2) == 0 && !below(count))
    {
        second = guest.byte(source + 1);
        first = guest.byte(source);
        fourth = guest.byte(source + 3);
        third = guest.byte(source + 2);
        do
        {
            const reg32 lowPair = palettePair(guest, second, first);
            const reg32 highPair = palettePair(guest, fourth, third);
            source += 4;
            destination += 8;
            first = guest.byte(source);
            third = guest.byte(source + 2);
            guest.setDword(destination - 8, lowPair);
            guest.setDword(destination - 4, highPair);
            second = guest.byte(source + 1);
            count -= 4;
            fourth = guest.byte(source + 3);
        } while (!below(count));
        eax = second;
    }
    count += 3;
    while (!below(count))
    {
        third = guest.byte(source);
        source += 1;
        eax = guest.word(third * 2 + kPalette16);
        guest.setWord(destination, std::uint16_t(eax));
        destination += 2;
        count -= 1;
    }
    return eax;
}

/* sub_504335: a row of 4-bit pixels to 16 bits, each byte's high nibble first: two
 * bytes at a time when the destination is on four bytes and four pixels are left,
 * then a byte at a time, then a last nibble.  Only the two-bytes loop reads its
 * bytes in first: not entered, the source is not looked at and pixels of 0 go out,
 * as the original's do.  edx keeps its upper half until a pair is made: it came in
 * as the destination. */
x86::reg32 frontend::rendf::rowPal4To16(App* app, CPU& cpu, reg32 source, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    reg32 current = 0, next = 0, pair = destination;
    count -= 4;
    if ((destination & 2) == 0 && !below(count))
    {
        current = guest.byte(source);
        next = guest.byte(source + 1);
        do
        {
            pair = palettePair(guest, current >> 4, current & 0xf);
            const reg32 nextPair = palettePair(guest, next >> 4, next & 0xf);
            source += 2;
            destination += 8;
            current = guest.byte(source);
            next = guest.byte(source + 1);
            guest.setDword(destination - 8, pair);
            guest.setDword(destination - 4, nextPair);
            count -= 4;
        } while (!below(count));
    }
    count += 2;
    while (!below(count))
    {
        pair = palettePair(guest, current >> 4, current & 0xf);
        source += 1;
        destination += 4;
        current = guest.byte(source);
        guest.setDword(destination - 4, pair);
        count -= 2;
    }
    count += 1;
    if (!below(count))
    {
        current &= 0xf;
        pair = (pair & 0xffff0000) | guest.word(current * 2 + kPalette16);
        guest.setWord(destination, std::uint16_t(pair));
    }
    return current;
}

namespace
{

/* A palette entry: the three bytes after `at` turned round, alpha full whatever the
 * byte at `at` was. */
reg32 argbAfter(const Guest& guest, reg32 at)
{
    const reg32 v = guest.dword(at);
    return (v << 24) | ((v & 0xff00) << 8) | ((v >> 8) & 0xff00) | (v >> 24) | 0xff000000;
}

}  // namespace

/* sub_50425d: `count` palette entries of three bytes each to ARGB words: four at a
 * time, each two of them read before they are written, then one at a time.  eax is
 * left as it came, or the last entry the loops held in it. */
x86::reg32 frontend::rendf::rowRgb24ToArgb(App* app, CPU& cpu, reg32 entries, reg32 destination, reg32 count)
{
    Guest guest(app, cpu);
    reg32 eax = entries;
    reg32 at = eax - 1;
    count -= 4;
    while (!below(count))
    {
        const reg32 entry0 = argbAfter(guest, at);
        const reg32 entry1 = argbAfter(guest, at + 3);
        guest.setDword(destination, entry0);
        guest.setDword(destination + 4, entry1);
        eax = argbAfter(guest, at + 6);
        const reg32 entry3 = argbAfter(guest, at + 9);
        guest.setDword(destination + 8, eax);
        guest.setDword(destination + 12, entry3);
        count -= 4;
        at += 12;
        destination += 16;
    }
    count += 3;
    while (!below(count))
    {
        eax = argbAfter(guest, at);
        guest.setDword(destination, eax);
        count -= 1;
        at += 3;
        destination += 4;
    }
    return eax;
}

namespace
{

/* ---- The display ---- */

constexpr reg32 kSetModeFunction = 0x567dc8;  // the screen's function for a mode; sub_4f8830 sets it
constexpr reg32 kDriverRunning = 0x564364;
constexpr reg32 kScreenFlags = 0x564390;      // byte
constexpr reg32 kDirectDrawScreen = 0x9f38d0; // DirectDraw's record of the screen
constexpr reg32 kRequestedWidth = 0x9f38e4, kRequestedHeight = 0x9f38e8, kRequestedDepth = 0x9f38ec;
constexpr reg32 kRequestedDepthByte = 0x9f3d2c;
constexpr reg32 kInitChannel = 0x564ed4;      // the "[INIT]" log's channel
constexpr reg32 kLogLevel = 0xf;
constexpr reg32 kLog = 0x501050;
constexpr reg32 kPrepareDirectDraw = 0x508170, kOpenDirectDraw = 0x508390;
constexpr reg32 kSelectMode = 0x507f30, kApplyMode = 0x5082f0;
constexpr reg32 kLogRequested = 0x54f4c8, kLogSelecting = 0x54f4ec, kLogSelected = 0x54f518, kLogNoMode = 0x54f3c0;

}  // namespace

/* sub_5089d0: the display mode eax x edx x ebx.  With the screen's functions in place,
 * as they are from the driver's start on (sub_4f8830), the screen's function does it
 * all and nothing else here runs.  The rest is DirectDraw's (win\initgrf.c), which
 * the port's driver never gets to: 0 or -1 for a default of each number, the mode
 * looked for and, the first time none is found, looked for again with bit 0x80 of
 * the flags on.  The first register of each call is what the original has there. */
x86::reg32 frontend::rendf::setDisplayMode(App* app, CPU& cpu, reg32 width, reg32 height, reg32 depth)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 setMode = guest.dword(kSetModeFunction);
    if (setMode != 0)
    {
        return callAt(guest, setMode, args({ width, height, depth, width }));
    }

    // What eax holds as the mode's functions are called: the width it came with, then the log's answer.
    reg32 carried = width;
    if (width == 0)
        width = 640;
    else if (width == reg32(-1))
        width = 1024;
    if (height == 0)
        height = 480;
    else if (height == reg32(-1))
        height = 768;
    if (depth == 0 || depth == reg32(-1) || depth == 15)
        depth = 16;

    for (;;)
    {
        const reg32 prepared = callAt(guest, kPrepareDirectDraw, args({ carried }));
        guest.setDword(kRequestedWidth, width);
        guest.setDword(kRequestedHeight, height);
        guest.setDword(kRequestedDepth, depth);
        guest.setByte(kRequestedDepthByte, std::uint8_t(depth));
        fe::direct<0x501050, ea::reportedAt>(app, cpu, kInitChannel, kLogLevel, kLogRequested, guest::Words({ width, height, depth }));
        const reg32 opened = callAt(guest, kOpenDirectDraw, args({ prepared }));
        if (opened == 0)
        {
            guest.setDword(kDriverRunning, 0);
            return 0;
        }

        const reg32 requestedDepth = guest.dword(kRequestedDepth);
        const reg32 requestedHeight = guest.dword(kRequestedHeight);
        const reg32 requestedWidth = guest.dword(kRequestedWidth);
        fe::direct<0x501050, ea::reportedAt>(app, cpu, kInitChannel, kLogLevel, kLogSelecting,
                                             guest::Words({ requestedWidth, requestedHeight, requestedDepth }));
        const reg32 selected = callAt(guest, kSelectMode, args({ kDirectDrawScreen }));
        fe::direct<0x501050, ea::reportedAt>(app, cpu, kInitChannel, kLogLevel, kLogSelected, guest::Words({ selected }));
        if (selected != 0)
            return callAt(guest, kApplyMode);

        const reg32 logged = *fe::direct<0x501050, ea::reportedAt>(app, cpu, kInitChannel, kLogLevel, kLogNoMode, guest::Words({}));
        const std::uint8_t flags = guest.byte(kScreenFlags);
        const reg32 eax = (logged & 0xffff00ff) | (reg32(flags) << 8);
        if ((flags & 0x80) != 0)
        {
            guest.setDword(kDriverRunning, 0);
            return eax;
        }
        carried = eax;
        const std::uint8_t marked = std::uint8_t(flags | 0x80);
        guest.setByte(kScreenFlags, marked);
    }
}

namespace
{

/* ---- The font's allocator ---- */

constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kFontFile = 0x550780;         // "font\fontpc.c"
constexpr reg32 kFontFunction = 0x550790;     // "FONT_softfalloc"
constexpr reg32 kFontMemoryName = 0x5507a0;   // "THRASHFONT"
constexpr reg32 kFontAllocLine = 0x127;
constexpr reg32 kFontHeap = 0x5643f4;

}  // namespace

/* sub_518f80, the font table's +0x14: `bytes` of the font's heap, with the place the
 * game's messages name set first. */
x86::reg32 frontend::rendf::fontAlloc(App* app, CPU& cpu, reg32 bytes)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    guest.setDword(kErrorFile, kFontFile);
    guest.setDword(kErrorLine, kFontAllocLine);
    const reg32 heap = guest.dword(kFontHeap);
    guest.setDword(kErrorFunction, kFontFunction);
    // The game's code hands it the function's name in ecx too, as it stored it just before, which
    // reservememadr does not read.
    return *fe::direct<0x4e1620, ea::reserved>(app, cpu, kFontMemoryName, bytes, heap);
}

/* sub_518fc0, the font table's +0x18: free, a jmp to it, so what it leaves is left.
 * It runs a return slot lower on the stack than from the jmp, which it does not see. */
x86::reg32 frontend::rendf::fontFree(App* app, CPU& cpu, reg32 block)
{
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
}

/* sub_525b80: two words of a font's record, 0 and 0 as sub_519f10 makes it. */
x86::reg32 frontend::rendf::fontSetOrigin(App* app, CPU& cpu, reg32 font, reg32 second, reg32 first)
{
    Guest guest(app, cpu);
    guest.setDword(font + 0x50, first);
    guest.setDword(font + 0x4c, second);
    return font;
}

x86::reg32 frontend::rendPixels565To1555(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_5040b8", 0, 0, nullptr, 0, kKeepsEcxEsi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::row565To1555(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendPixels1555To565(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_504133", 0, 0, nullptr, 0, kKeepsEcxEsi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::row1555To565(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendPixelsPal8To32(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_5041ae", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::rowPal8To32(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendPixelsPal8To16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_5041d0", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::rowPal8To16(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendPixelsRgb24ToArgb(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_50425d", 0, 0, nullptr, 0, kKeepsEcxEsi };
    const reg32 entries = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::rowRgb24ToArgb(app, cpu, entries, destination, count); });
}

x86::reg32 frontend::rendPixelsSwapRedBlue(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_5042c4", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::rowSwapRedBlue(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendPixelsPal4To16(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_504335", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 source = args.eax, destination = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::rowPal4To16(app, cpu, source, destination, count); });
}

x86::reg32 frontend::rendSetDisplayMode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_5089d0", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 width = args.eax, height = args.edx, depth = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::setDisplayMode(app, cpu, width, height, depth); });
}

x86::reg32 frontend::rendFontAlloc(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_518f80", 0, 0, nullptr, 0, kKeepsEbxEcxEdx };
    const reg32 bytes = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::fontAlloc(app, cpu, bytes); });
}

x86::reg32 frontend::rendFontFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_518fc0", 0, 0, nullptr, 0, 0 };
    const reg32 block = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::fontFree(app, cpu, block); });
}

x86::reg32 frontend::rendFontSetOrigin(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_525b80", 0, 0, nullptr, 0, 0 };
    const reg32 font = args.eax, second = args.edx, first = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return rendf::fontSetOrigin(app, cpu, font, second, first); });
}

}
