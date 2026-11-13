#ifndef NFS3HP_NATIVE_FRONTEND_MENZ_H
#define NFS3HP_NATIVE_FRONTEND_MENZ_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menz.cpp's typed functions (EA's libraries' small change and the C runtime's
 * arithmetic: DirectDraw, start-up, the file device, video windows, fonts, palettes, the debug
 * keys, Watcom's doubles), which other files call as C++ (guest::direct), and how the game's code
 * calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other
 * functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menz
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4ed330: the display's surface given up and locked again.
reg32 ddrawRelockSurface(App* app, CPU& cpu, reg32 display);
// The game's 0x4ed480: makesurface: a DirectDraw surface, locked once to see that it can be.
reg32 ddrawMakeSurface(App* app, CPU& cpu, reg32 display, reg32 width, reg32 height, reg32 pages, reg32 backBuffers,
                       reg32 formatWanted, reg32 capsAsked, reg32 palette);
// The game's 0x4ed730: a surface released, unlocked first if the display holds it locked.
reg32 ddrawFreeSurface(App* app, CPU& cpu, reg32 display, reg32 surface);
// The game's 0x4ed770: the surface attached to one as its back buffer.
reg32 ddrawBackSurface(App* app, CPU& cpu, reg32 surface);
// The game's 0x4ed820: a surface's size, depth, pitch and memory, each where its pointer says.
reg32 ddrawSurfaceSize(App* app, CPU& cpu, reg32 surface, reg32 widthOut, reg32 heightOut, reg32 bitsOut,
                       reg32 pitchOut, reg32 memoryOut);
// The game's 0x4edbc0: EnumDisplayModes' callback: the mode noted and reported.
reg32 ddrawEnumModeCallback(App* app, CPU& cpu, reg32 desc, reg32 list);
// The game's 0x4f9060: one more change to what the display shows, counted.
reg32 ddrawNoteChange(App* app, CPU& cpu);
// The game's 0x4f91d0: the mutex by the window class's name; the other instance brought up.
reg32 singleInstanceCheck(App* app, CPU& cpu, reg32 name);
// The game's 0x4f9260: the RAM class's purge callback: nothing can be purged.
reg32 memClassNoPurge(App* app, CPU& cpu);
// The game's 0x4f9270: the memory class RAM.
reg32 memClassCreateRam(App* app, CPU& cpu, reg32 base, reg32 size);
// The game's 0x517b30: the text library's state cleared, once, and its end to run at exit.
reg32 eaTextStart(App* app, CPU& cpu);
// The game's 0x517b70: the text library's end.
reg32 eaTextEnd(App* app, CPU& cpu);
// The game's 0x517ba0: 1 and the NEC PC-98's scan codes when the keyboard is one.
reg32 keyboardIsPc98(App* app, CPU& cpu);
// The game's 0x518040: the load's hook set to sub_517f10.
reg32 eaSetLoadHook(App* app, CPU& cpu);
// The game's 0x518fd0: the video buffers' table, once, and the list of their numbers.
reg32 videoBuffersStart(App* app, CPU& cpu, reg32 count);
// The game's 0x517d70: key 0x17: the display's flag flipped and the mode set again.
reg32 debugKeyDisplayFlag(App* app, CPU& cpu);
// The game's 0x517da0: key 0x16: the screen saved as snap.tga, and a banner that says so.
reg32 debugKeyScreenshot(App* app, CPU& cpu);
// The game's 0x517de0: the debug keys bound, Ctrl+Escape set to leave.
reg32 debugKeysBind(App* app, CPU& cpu);
// The game's 0x524930: what is on the screen copied into the bitmap.
reg32 debugBoxCopyScreen(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y);
// The game's 0x524950: the bitmap back on the screen.
reg32 debugBoxPutBack(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y);
// The game's 0x524970: a box of text, what it covers kept and put back; the answer.
reg32 debugBoxAsk(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 width, reg32 height, reg32 mask);
// The game's 0x524c30: a line of text along the foot of the screen for a time.
reg32 debugBanner(App* app, CPU& cpu, reg32 text, reg32 time);
// The game's 0x524d90: key 0x10: pause, the box in the middle of the screen until a key.
reg32 debugKeyPause(App* app, CPU& cpu);
// The game's 0x524dd0: key 4: the memory viewer's first way in.
reg32 debugKeyViewerFirst(App* app, CPU& cpu);
// The game's 0x524de0: key 5: the memory viewer's second way in.
reg32 debugKeyViewerSecond(App* app, CPU& cpu);
// The game's 0x52e440: text with a filled box behind it.
reg32 textBackgroundBox(App* app, CPU& cpu, reg32 text);
// The game's 0x4fb860: FILE_startdevice: the device's thread, if it has none.
reg32 fileDeviceStart(App* app, CPU& cpu, reg32 index);
// The game's 0x4fbd10: an operation by its id on a device: under way, queued or finished.
reg32 fileOpFind(App* app, CPU& cpu, reg32 device, reg32 id, reg32 where);
// The game's 0x4fc340: an operation's state.
reg32 fileOpStatus(App* app, CPU& cpu, reg32 id);
// The game's 0x4fc600: FILE_completeop: a finished operation taken off its device's list.
reg32 fileOpComplete(App* app, CPU& cpu, reg32 id);
// The game's 0x4fc780: waits until an operation has finished; its state.
reg32 fileOpWait(App* app, CPU& cpu, reg32 id);
// The game's 0x50ea20: the critical section of a lock entered; 0, the token.
reg32 fileLockEnter(App* app, CPU& cpu, reg32 lock);
// The game's 0x50ea40: and left (the token is not read).
reg32 fileLockLeave(App* app, CPU& cpu, reg32 lock, reg32 token);
// The game's 0x508c70: makevideowindow: a window of a size, in a fixed record or its own.
reg32 videoWindowMake(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 passedOn, reg32 where,
                      reg32 passedOn2, reg32 passedOn3);
// The game's 0x508da0: updatevideowindow: a new row table when the row size changed.
reg32 videoWindowUpdate(App* app, CPU& cpu, reg32 window, reg32 rowBytes, reg32 bits);
// The game's 0x508e70: a window's row tables freed, and its record unless it is a fixed one.
reg32 videoWindowFree(App* app, CPU& cpu, reg32 window);
// The game's 0x5199b0: the glyph of a character found by halves in a sorted table.
reg32 fontFindGlyph(App* app, CPU& cpu, reg32 character, reg32 table, reg32 count, reg32 size);
// The game's 0x519a50: FONT_getcharacter: the glyph of a character in a font.
reg32 fontGlyph(App* app, CPU& cpu, reg32 font, reg32 character);
// The game's 0x519ea0: how many bits a pixel a shape's type gives a font's glyph.
reg32 fontBitsPerPixel(App* app, CPU& cpu, reg32 shape);
// The game's 0x519f10: FONT_create: a font's record for a .FNT in memory.
reg32 fontCreate(App* app, CPU& cpu, reg32 data);
// The game's 0x5257c0: a font's property set.
reg32 fontSetProperty(App* app, CPU& cpu, reg32 font, reg32 property, reg32 value);
// The game's 0x5259d0: a font's property read back; ecx for one it has not.
reg32 fontGetProperty(App* app, CPU& cpu, reg32 font, reg32 property);
// The game's 0x51de60: colours of 3 bytes of 6 bits to 4 bytes of 8.
reg32 paletteFrom6Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to);
// The game's 0x51df60: colours of 3 bytes to 4.
reg32 paletteFrom8Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to);
// The game's 0x52448c: what a long double is: zero, a number, NaN, infinity or denormal.
reg32 longDoubleKind(App* app, CPU& cpu, reg32 value);
// The game's 0x52643c: Watcom's __FDD: edx:eax divided by ecx:ebx.
reg32 doubleDivide(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x526642: Watcom's __FDS: edx:eax less ecx:ebx.
reg32 doubleSubtract(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x5267ff: Watcom's __FDM: edx:eax times ecx:ebx.
reg32 doubleMultiply(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x526674: the x87's way's end for sub_5267ff: st(0) the answer.
reg32 doubleProductFromX87(App* app, CPU& cpu);
// The game's 0x52699c: edx:eax against ecx:ebx: -1, 0 or 1.
reg32 doubleCompare(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh);
// The game's 0x51f2a0: the runtime's start-up test of the Pentium's FDIV bug.
reg32 fdivBugCheck(App* app, CPU& cpu);
// The game's 0x52700c: st(0) divided by a double on the stack, safe from the bug.
reg32 safeDivide(App* app, CPU& cpu, reg32 divisorLow, reg32 divisorHigh);
// The game's 0x526f87: st(1) and st(0) put in memory for the careful division, its answer back.
reg32 safeDivideCall(App* app, CPU& cpu);

}  // namespace menz
}  // namespace frontend

namespace guest
{

// The display in eax.
template <>
struct ByAddress<frontend::menz::ddrawRelockSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display }));
    }
};

// The display, the size, the pages in eax, edx, ebx, ecx; the rest on the stack.
template <>
struct ByAddress<frontend::menz::ddrawMakeSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display, x86::reg32 width,
                           x86::reg32 height, x86::reg32 pages, x86::reg32 backBuffers, x86::reg32 formatWanted,
                           x86::reg32 capsAsked, x86::reg32 palette)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ display, width, height, pages },
                                          { backBuffers, formatWanted, capsAsked, palette }));
    }
};

// The display (0 the main one) in eax, the surface in edx.
template <>
struct ByAddress<frontend::menz::ddrawFreeSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 display, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ display, surface }));
    }
};

// The surface in eax.
template <>
struct ByAddress<frontend::menz::ddrawBackSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface }));
    }
};

// The surface and three pointers in eax, edx, ebx, ecx; two more on the stack.
template <>
struct ByAddress<frontend::menz::ddrawSurfaceSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface, x86::reg32 widthOut,
                           x86::reg32 heightOut, x86::reg32 bitsOut, x86::reg32 pitchOut, x86::reg32 memoryOut)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ surface, widthOut, heightOut, bitsOut }, { pitchOut, memoryOut }));
    }
};

// The mode's description and the list on the stack.
template <>
struct ByAddress<frontend::menz::ddrawEnumModeCallback>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 desc, x86::reg32 list)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { desc, list }));
    }
};

template <>
struct ByAddress<frontend::menz::ddrawNoteChange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The name in eax.
template <>
struct ByAddress<frontend::menz::singleInstanceCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

template <>
struct ByAddress<frontend::menz::memClassNoPurge>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The base in eax, the size in edx.
template <>
struct ByAddress<frontend::menz::memClassCreateRam>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 base, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ base, size }));
    }
};

template <>
struct ByAddress<frontend::menz::eaTextStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::eaTextEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::keyboardIsPc98>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::eaSetLoadHook>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The count in eax.
template <>
struct ByAddress<frontend::menz::videoBuffersStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count }));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeyDisplayFlag>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeyScreenshot>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeysBind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The bitmap in eax, where in edx and ebx.
template <>
struct ByAddress<frontend::menz::debugBoxCopyScreen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bitmap, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ bitmap, x, y }));
    }
};

// The bitmap in eax, where in edx and ebx.
template <>
struct ByAddress<frontend::menz::debugBoxPutBack>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bitmap, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ bitmap, x, y }));
    }
};

// The text, where and how wide in eax, edx, ebx, ecx; its height and the buttons' mask on the stack.
template <>
struct ByAddress<frontend::menz::debugBoxAsk>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 x, x86::reg32 y,
                           x86::reg32 width, x86::reg32 height, x86::reg32 mask)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, x, y, width }, { height, mask }));
    }
};

// The text in eax, the time in edx.
template <>
struct ByAddress<frontend::menz::debugBanner>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 time)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, time }));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeyPause>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeyViewerFirst>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menz::debugKeyViewerSecond>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The text in eax.
template <>
struct ByAddress<frontend::menz::textBackgroundBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text }));
    }
};

// The device in eax.
template <>
struct ByAddress<frontend::menz::fileDeviceStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// The device in eax, the id in edx, where to say which in ebx.
template <>
struct ByAddress<frontend::menz::fileOpFind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 device, x86::reg32 id,
                           x86::reg32 where)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ device, id, where }));
    }
};

// The id in eax.
template <>
struct ByAddress<frontend::menz::fileOpStatus>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

// The id in eax.
template <>
struct ByAddress<frontend::menz::fileOpComplete>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

// The id in eax.
template <>
struct ByAddress<frontend::menz::fileOpWait>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

// The lock in eax.
template <>
struct ByAddress<frontend::menz::fileLockEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lock)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ lock }));
    }
};

// The lock in eax, the token in edx.
template <>
struct ByAddress<frontend::menz::fileLockLeave>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lock, x86::reg32 token)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ lock, token }));
    }
};

// The size and bits in eax, edx, ebx, ecx passed on; where and two more on the stack.
template <>
struct ByAddress<frontend::menz::videoWindowMake>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height,
                           x86::reg32 bits, x86::reg32 passedOn, x86::reg32 where, x86::reg32 passedOn2,
                           x86::reg32 passedOn3)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ width, height, bits, passedOn }, { where, passedOn2, passedOn3 }));
    }
};

// The window (0 the screen's) in eax, the row size in edx, the bits in ebx.
template <>
struct ByAddress<frontend::menz::videoWindowUpdate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 rowBytes,
                           x86::reg32 bits)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window, rowBytes, bits }));
    }
};

// The window in eax.
template <>
struct ByAddress<frontend::menz::videoWindowFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window }));
    }
};

// The character, the table, its count, a record's size.
template <>
struct ByAddress<frontend::menz::fontFindGlyph>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character, x86::reg32 table,
                           x86::reg32 count, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character, table, count, size }));
    }
};

// The font in eax, the character in edx.
template <>
struct ByAddress<frontend::menz::fontGlyph>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 font, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ font, character }));
    }
};

// The shape in eax.
template <>
struct ByAddress<frontend::menz::fontBitsPerPixel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape }));
    }
};

// The .FNT in eax.
template <>
struct ByAddress<frontend::menz::fontCreate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 data)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ data }));
    }
};

// The font in eax, the property in edx, the value in ebx.
template <>
struct ByAddress<frontend::menz::fontSetProperty>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 font, x86::reg32 property,
                           x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ font, property, value }));
    }
};

// The font in eax, the property in edx.
template <>
struct ByAddress<frontend::menz::fontGetProperty>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 font, x86::reg32 property)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ font, property }));
    }
};

// The count in eax, from in edx, to in ebx.
template <>
struct ByAddress<frontend::menz::paletteFrom6Bits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 from, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, from, to }));
    }
};

// The count in eax, from in edx, to in ebx.
template <>
struct ByAddress<frontend::menz::paletteFrom8Bits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 from, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, from, to }));
    }
};

// The long double's address in eax.
template <>
struct ByAddress<frontend::menz::longDoubleKind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ value }));
    }
};

// edx:eax and ecx:ebx: the first double's low and high words, then the second's.
template <>
struct ByAddress<frontend::menz::doubleDivide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 low, x86::reg32 high,
                           x86::reg32 otherLow, x86::reg32 otherHigh)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ low, high, otherLow, otherHigh }));
    }
};

// edx:eax and ecx:ebx: the first double's low and high words, then the second's.
template <>
struct ByAddress<frontend::menz::doubleSubtract>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 low, x86::reg32 high,
                           x86::reg32 otherLow, x86::reg32 otherHigh)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ low, high, otherLow, otherHigh }));
    }
};

// edx:eax and ecx:ebx: the first double's low and high words, then the second's.
template <>
struct ByAddress<frontend::menz::doubleMultiply>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 low, x86::reg32 high,
                           x86::reg32 otherLow, x86::reg32 otherHigh)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ low, high, otherLow, otherHigh }));
    }
};

template <>
struct ByAddress<frontend::menz::doubleProductFromX87>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// edx:eax and ecx:ebx: the first double's low and high words, then the second's.
template <>
struct ByAddress<frontend::menz::doubleCompare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 low, x86::reg32 high,
                           x86::reg32 otherLow, x86::reg32 otherHigh)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ low, high, otherLow, otherHigh }));
    }
};

template <>
struct ByAddress<frontend::menz::fdivBugCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The divisor's two words on the stack.
template <>
struct ByAddress<frontend::menz::safeDivide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 divisorLow, x86::reg32 divisorHigh)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { divisorLow, divisorHigh }));
    }
};

template <>
struct ByAddress<frontend::menz::safeDivideCall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
