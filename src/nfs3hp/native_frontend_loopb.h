#ifndef NFS3HP_NATIVE_FRONTEND_LOOPB_H
#define NFS3HP_NATIVE_FRONTEND_LOOPB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loopb.cpp's typed functions (the race's loop around the simulation: laps and
 * standings, the race's end, the replay's record, the views, the pause's frame), which other files
 * call as C++ (guest::direct), and how the game's code calls each by address (ByAddress, for the
 * checks' stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loopb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4bc100: the view a player's race HUD is drawn in, by the form.
reg32 viewForHud(App* app, CPU& cpu, reg32 form);
// The game's 0x4bc130: the view a player's scene is drawn in, by the form.
reg32 viewForScene(App* app, CPU& cpu, reg32 form);
// The game's 0x4bc560: the pointer shown while the mouse moves, hidden 640 ticks after.
reg32 pointerIdle(App* app, CPU& cpu);
// The game's 0x4bc620: the frame ended, told whether it is the first under a pause.
reg32 endFrame(App* app, CPU& cpu, reg32 paused);
// The game's 0x4bc650: the race's text and its count drawn.
reg32 textAndCount(App* app, CPU& cpu);
// The game's 0x4bc680: the frame of a paused race, the menu over it if `menu`.
reg32 pauseDraw(App* app, CPU& cpu, reg32 menu);
// The game's 0x4bc820: nothing.
reg32 nothing(App* app, CPU& cpu);
// The game's 0x4bc980: the race's drawing started, its arena made.
reg32 drawStart(App* app, CPU& cpu);
// The game's 0x4bc990: the arena let go, the drawing ended.
reg32 drawEnd(App* app, CPU& cpu);
// The game's 0x4bc9a0: the replay's mode back to 0.
reg32 replayModeOff(App* app, CPU& cpu);
// The game's 0x4bc9b0: 32 bytes packed, the length to *length; the packed row's place.
reg32 packRow(App* app, CPU& cpu, reg32 row, reg32 length);
// The game's 0x4bca80: and back, the bytes read to *length; the row's place.
reg32 unpackRow(App* app, CPU& cpu, reg32 packed, reg32 length);
// The game's 0x4bcb90: the race's start: the record cleared, or played from the top.
reg32 replayStart(App* app, CPU& cpu);
// The game's 0x4bcd60: the race's end: the replay saved while it was recording.
reg32 replayEnd(App* app, CPU& cpu);
// The game's 0x4bcda0: a block's four rows (128 bytes at `block`, handed by value) packed onto the record.
reg32 storeBlock(App* app, CPU& cpu, reg32 block);
// The game's 0x4bcee0: a block's four rows taken off the record into `block` (esi), esi left past the frame's.
reg32 takeBlock(App* app, CPU& cpu, reg32 block);
// The game's 0x4bd000: this tick's controls of a player onto his block, 32 of them packed away.
reg32 recordControls(App* app, CPU& cpu, reg32 player);
// The game's 0x4bd090: a player's controls for this tick off the record, the television told.
reg32 playControls(App* app, CPU& cpu, reg32 player);
// The game's 0x4bd380: a car's task each tick: the ghost of the player's own, the record or its playback.
reg32 replayCarTask(App* app, CPU& cpu, reg32 car);
// The game's 0x4bd3f0: the pointer in the middle of a choice of the strip of four.
reg32 pointerAtChoice(App* app, CPU& cpu, reg32 choice);
// The game's 0x4bd9d0: the worth an event gives a car, the most kept; edx left the place.
reg32 noteEvent(App* app, CPU& cpu, reg32 worth, reg32 index);
// The game's 0x4bda00: each car's worth into this moment's row, the most kept.
reg32 recordEvents(App* app, CPU& cpu);
// The game's 0x4be020: the next number of a text at *cursor, the cursor moved on.
reg32 readNumber(App* app, CPU& cpu, reg32 cursor);
// The game's 0x4be5e0: all the race's views from the mode's size and the form's mirror.
reg32 viewsLayout(App* app, CPU& cpu, reg32 form);
// The game's 0x4bed20: a view's projection (the angle, the last argument) and clip.
reg32 useWithProjection(App* app, CPU& cpu, reg32 view, reg32 angle, reg32 last);
// The game's 0x4bed50: only a view's projection.
reg32 projectionOnly(App* app, CPU& cpu, reg32 view, reg32 angle, reg32 last);
// The game's 0x4bed70: only a view's clip.
reg32 useClip(App* app, CPU& cpu, reg32 view);
// The game's 0x4c4820: the config saved.
reg32 saveConfig(App* app, CPU& cpu);
// The game's 0x4c4830: everything the race holds, let go.
reg32 teardown(App* app, CPU& cpu);
// The game's 0x4c4d80: the race told to stop.
reg32 stop(App* app, CPU& cpu);
// The game's 0x4c4de0: whether a player's asked and the race may stop now; it is stopped then.
reg32 asked(App* app, CPU& cpu, reg32 key);
// The game's 0x4c9d90: the records at their table cleared.
reg32 clearRecords(App* app, CPU& cpu);
// The game's 0x4cdd00: the standings emptied, then made.
reg32 standingsStart(App* app, CPU& cpu);
// The game's 0x4cdd30: the qsort's comparison of the cars at *a and *b; edx left the first.
reg32 compareCars(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4cde00: the places: those still in it sorted, 0 for the others.
reg32 standingsUpdate(App* app, CPU& cpu);
// The game's 0x4cded0: a car's place.
reg32 placeOf(App* app, CPU& cpu, reg32 car);
// The game's 0x4cdee0: how many a place is among.
reg32 racerCount(App* app, CPU& cpu);
// The game's 0x4cdef0: the car a place ahead of `place` from 2 on.
reg32 carInPlace(App* app, CPU& cpu, reg32 place);
// The game's 0x4cdf10: the performance summary's times of a car.
reg32 performanceTimes(App* app, CPU& cpu, reg32 car);
// The game's 0x4ce280: a car's task each tick: its lap's top speed, the lap's end and time, how far on.
reg32 carTask(App* app, CPU& cpu, reg32 car);
// The game's 0x4ce5a0: a car the race ended before it was home: its order and made-up times.
reg32 estimateRest(App* app, CPU& cpu, reg32 car);
// The game's 0x4ce750: the race over: the states set, those still racing given their order.
reg32 closeRace(App* app, CPU& cpu, reg32 state);
// The game's 0x4ce800: a list's cars, but those home or out, to a state; edx left the last.
reg32 setStates(App* app, CPU& cpu, reg32 list, reg32 count, reg32 state);
// The game's 0x4ce840: whether everyone not out is home long enough.
reg32 allFinishedKnockout(App* app, CPU& cpu);
// The game's 0x4ce8e0: whether everyone is home long enough; the short times cleared then.
reg32 allFinishedTimed(App* app, CPU& cpu);
// The game's 0x4ce960: whether knockout's sides are done.
reg32 allFinishedSides(App* app, CPU& cpu);
// The game's 0x4ceae0: each tick: whether the race has ended, then it is closed.
reg32 checkRaceEnd(App* app, CPU& cpu);
// The game's 0x4ceb50: times too short to be true, gone.
reg32 clearShortTimes(App* app, CPU& cpu);
// The game's 0x4d5d10: the drawn flag cleared.
reg32 clearDrawnFlag(App* app, CPU& cpu);
// The game's 0x4dbce0: the projection a view asks for: its place and size, the angle and the last.
reg32 setProjection(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 angle, reg32 last);

}  // namespace loopb
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::loopb::viewForHud>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 form)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ form }));
    }
};

template <>
struct ByAddress<frontend::loopb::viewForScene>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 form)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ form }));
    }
};

template <>
struct ByAddress<frontend::loopb::pointerIdle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::endFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 paused)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ paused }));
    }
};

template <>
struct ByAddress<frontend::loopb::textAndCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::pauseDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::loopb::nothing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::drawStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::drawEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::replayModeOff>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::packRow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 row, x86::reg32 length)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ row, length }));
    }
};

template <>
struct ByAddress<frontend::loopb::unpackRow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 packed, x86::reg32 length)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ packed, length }));
    }
};

template <>
struct ByAddress<frontend::loopb::replayStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::replayEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The block's 128 bytes copied onto the stack, as the game's code hands them by value.
template <>
struct ByAddress<frontend::loopb::storeBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        const x86::reg32 esp = cpu.esp;
        cpu.esp -= 128;
        for (x86::reg32 i = 0; i < 128; i += 4)
            put(app, cpu.esp + i, at(app, block + i));
        const x86::reg32 result = fe::callByAddress(app, cpu, Address, fe::args({}));
        cpu.esp = esp;
        return result;
    }
};

// The block's place in esi.
template <>
struct ByAddress<frontend::loopb::takeBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        cpu.esi = block;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::recordControls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

template <>
struct ByAddress<frontend::loopb::playControls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

template <>
struct ByAddress<frontend::loopb::replayCarTask>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loopb::pointerAtChoice>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 choice)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ choice }));
    }
};

template <>
struct ByAddress<frontend::loopb::noteEvent>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 worth, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ worth, index }));
    }
};

template <>
struct ByAddress<frontend::loopb::recordEvents>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::readNumber>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cursor)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cursor }));
    }
};

template <>
struct ByAddress<frontend::loopb::viewsLayout>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 form)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ form }));
    }
};

template <>
struct ByAddress<frontend::loopb::useWithProjection>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 angle,
                           x86::reg32 last)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, angle, last }));
    }
};

template <>
struct ByAddress<frontend::loopb::projectionOnly>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 angle,
                           x86::reg32 last)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, angle, last }));
    }
};

template <>
struct ByAddress<frontend::loopb::useClip>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::loopb::saveConfig>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::teardown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::stop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::asked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key }));
    }
};

template <>
struct ByAddress<frontend::loopb::clearRecords>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::standingsStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::compareCars>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

template <>
struct ByAddress<frontend::loopb::standingsUpdate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::placeOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loopb::racerCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::carInPlace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ place }));
    }
};

template <>
struct ByAddress<frontend::loopb::performanceTimes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loopb::carTask>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loopb::estimateRest>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::loopb::closeRace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ state }));
    }
};

template <>
struct ByAddress<frontend::loopb::setStates>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 count,
                           x86::reg32 state)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list, count, state }));
    }
};

template <>
struct ByAddress<frontend::loopb::allFinishedKnockout>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::allFinishedTimed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::allFinishedSides>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::checkRaceEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::clearShortTimes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::clearDrawnFlag>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::loopb::setProjection>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y, x86::reg32 width,
                           x86::reg32 height, x86::reg32 angle, x86::reg32 last)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y, width, height }, { angle, last }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
