#ifndef NFS3HP_NATIVE_FRONTEND_HUDR_H
#define NFS3HP_NATIVE_FRONTEND_HUDR_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudr.cpp's typed functions (the race HUD's elements drawn and made again, the
 * map's pane, the HUD's text and lines), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax.
 *
 * A text's place, as sub_4897f0 takes it on the stack and the functions over it pass on: its scale,
 * colour, left, right, top and how it is aligned, six words. */
namespace nfs3hp
{
namespace frontend
{
namespace hudr
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45ab30: an element of the HUD drawn for its view, `item` its item on the editor's screen.
reg32 drawElement(App* app, CPU& cpu, reg32 item);
// The game's 0x4853c0: one element of a view's HUD made again for the rectangle it now has.
reg32 rebuildElement(App* app, CPU& cpu, reg32 view, reg32 element);
// The game's 0x48e920: the views' map panes stepped on until a split screen's two agree.
reg32 stepMapPanes(App* app, CPU& cpu);
// The game's 0x4896f0: the letters after a place: "st", "nd", "rd" or "th".
reg32 ordinalLetters(App* app, CPU& cpu, reg32 place);
// The game's 0x489750: a time in 64ths of a second written into `buffer`; edx left 0.
reg32 formatTime(App* app, CPU& cpu, reg32 buffer, reg32 time);
// The game's 0x4898b0: a time at its place.
reg32 drawTime(App* app, CPU& cpu, reg32 time, reg32 scale, reg32 colour, reg32 left, reg32 right, reg32 top,
               reg32 how);
// The game's 0x489900: a percentage and, below it, a count, at their place.
reg32 drawPercentAndCount(App* app, CPU& cpu, reg32 percent, reg32 count, reg32 scale, reg32 colour, reg32 left,
                          reg32 right, reg32 top, reg32 how);
// The game's 0x4899d0: "Lap" with a count of a total, at its place.
reg32 drawLaps(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right,
               reg32 top, reg32 how);
// The game's 0x489a50: a line of the language's with a count of a total, at its place.
reg32 drawRatio(App* app, CPU& cpu, reg32 done, reg32 total, reg32 scale, reg32 colour, reg32 left, reg32 right,
                reg32 top, reg32 how);
// The game's 0x48a320: a place among so many, at its place.
reg32 drawPlaceAmong(App* app, CPU& cpu, reg32 place, reg32 among, reg32 scale, reg32 colour, reg32 left,
                     reg32 right, reg32 top, reg32 how);
// The game's 0x48b2a0: a triangle in a colour, filled or as three lines.
reg32 drawTriangle(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 filled,
                   reg32 colour);
// The game's 0x48b4a0: a line in a colour.
reg32 drawLine(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 colour);
// The game's 0x48e770: the HUD's line mesh flushed.
reg32 flushLines(App* app, CPU& cpu);
// The game's 0x48e780: the HUD's line mesh made ready.
reg32 prepareLines(App* app, CPU& cpu);

}  // namespace hudr
}  // namespace frontend

namespace guest
{

// The item in eax.
template <>
struct ByAddress<frontend::hudr::drawElement>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The view in eax, the element in edx.
template <>
struct ByAddress<frontend::hudr::rebuildElement>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 element)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, element }));
    }
};

template <>
struct ByAddress<frontend::hudr::stepMapPanes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The place in eax.
template <>
struct ByAddress<frontend::hudr::ordinalLetters>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ place }));
    }
};

// The buffer in eax, the time in edx.
template <>
struct ByAddress<frontend::hudr::formatTime>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 time)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ buffer, time }));
    }
};

// The time in eax, the place's six words on the stack.
template <>
struct ByAddress<frontend::hudr::drawTime>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 time, x86::reg32 scale,
                           x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top, x86::reg32 how)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ time }, { scale, colour, left, right, top, how }));
    }
};

// Two numbers in eax and edx, the place's six words on the stack: these four.
template <>
struct ByAddress<frontend::hudr::drawPercentAndCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 percent, x86::reg32 count,
                           x86::reg32 scale, x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top,
                           x86::reg32 how)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ percent, count }, { scale, colour, left, right, top, how }));
    }
};

template <>
struct ByAddress<frontend::hudr::drawLaps>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 done, x86::reg32 total,
                           x86::reg32 scale, x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top,
                           x86::reg32 how)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ done, total }, { scale, colour, left, right, top, how }));
    }
};

template <>
struct ByAddress<frontend::hudr::drawRatio>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 done, x86::reg32 total,
                           x86::reg32 scale, x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top,
                           x86::reg32 how)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ done, total }, { scale, colour, left, right, top, how }));
    }
};

template <>
struct ByAddress<frontend::hudr::drawPlaceAmong>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 among,
                           x86::reg32 scale, x86::reg32 colour, x86::reg32 left, x86::reg32 right, x86::reg32 top,
                           x86::reg32 how)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ place, among }, { scale, colour, left, right, top, how }));
    }
};

// Eight words on the stack.
template <>
struct ByAddress<frontend::hudr::drawTriangle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x0, x86::reg32 y0, x86::reg32 x1,
                           x86::reg32 y1, x86::reg32 x2, x86::reg32 y2, x86::reg32 filled, x86::reg32 colour)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { x0, y0, x1, y1, x2, y2, filled, colour }));
    }
};

// Five words on the stack.
template <>
struct ByAddress<frontend::hudr::drawLine>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x0, x86::reg32 y0, x86::reg32 x1,
                           x86::reg32 y1, x86::reg32 colour)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { x0, y0, x1, y1, colour }));
    }
};

template <>
struct ByAddress<frontend::hudr::flushLines>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::hudr::prepareLines>
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
