#ifndef NFS3HP_NATIVE_FRONTEND_TAIL_H
#define NFS3HP_NATIVE_FRONTEND_TAIL_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_tail.cpp's typed functions (a tick of the race and its task lists, the controls'
 * queues emptied, the track's named places and lanes, an angle's sine and cosine, the weather),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its result is
 * what the game's function leaves in eax.  Some leave edx (or esi) to their callers as the game's
 * do, through the adapter; a C++ caller gets every register but eax back as it had them. */
namespace nfs3hp
{
namespace frontend
{
namespace tail
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4be240: `block` given back to its memory, if there is one.
reg32 blockFree(App* app, CPU& cpu, reg32 block);
// The game's 0x4be3a0: each task of `list` that has a function called, with its argument.
reg32 runTaskList(App* app, CPU& cpu, reg32 list);
// The game's 0x4c48d0: a tick of the race: the countdown, the task lists, the standings.
reg32 raceFrame(App* app, CPU& cpu);
// The game's 0x4c7b50: the controls' queues zeroed under their lock.
reg32 queuesClear(App* app, CPU& cpu);
// The game's 0x4c8470: the named place nearest `node`, which is in none, to `out`: 1, else 0.
reg32 placeNearNode(App* app, CPU& cpu, reg32 node, reg32 out);
// The game's 0x4c8550: the named place `node` is in, to `out`: 0, else -1.
reg32 placeAtNode(App* app, CPU& cpu, reg32 node, reg32 out);
// The game's 0x4c9a80: how far from the road's middle, 16.16, `lane` is at `node`; edx left the near limit.
reg32 laneOffset(App* app, CPU& cpu, reg32 node, reg32 lane);
// The game's 0x4dcdf0: the weather set up as a race loads.
reg32 weatherStart(App* app, CPU& cpu);
// The game's 0x4dd230: the camera of `view` changed: its player's drops put around it again.
reg32 weatherScatter(App* app, CPU& cpu, reg32 view);
// The game's 0x4dd330: `amount` of weather by the time of the race.
reg32 weatherFadeInTime(App* app, CPU& cpu, reg32 amount);
// The game's 0x4dd3d0: how many drops fall at `node`.
reg32 weatherFallAtNode(App* app, CPU& cpu, reg32 node);
// The game's 0x4dd4c0: of the drops, the share `seen` of them is for `player`, per cent; edx left.
reg32 weatherShare(App* app, CPU& cpu, reg32 seen, reg32 player);
// The game's 0x4dd510: whether `player`'s view is covered, its drops' places forgotten when it changes.
reg32 weatherViewChange(App* app, CPU& cpu, reg32 player);
// The game's 0x4dd5e0: the drops of `view`'s player moved, `wanted` of them put back round it; edx left.
reg32 weatherDropsStep(App* app, CPU& cpu, reg32 view, reg32 wanted);
// The game's 0x4dece0: the weather's pass over `view`, the list for the caller to `out` (esi); esi left.
reg32 weatherOnView(App* app, CPU& cpu, reg32 view, reg32 out);
// The game's 0x4eac60: a 10-bit angle's sine to `sineOut` and cosine to `cosineOut`; edx left.
reg32 angleParts(App* app, CPU& cpu, reg32 angle, reg32 sineOut, reg32 cosineOut);

}  // namespace tail
}  // namespace frontend

namespace guest
{

// The block in eax.
template <>
struct ByAddress<frontend::tail::blockFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ block }));
    }
};

// The list in eax.
template <>
struct ByAddress<frontend::tail::runTaskList>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list }));
    }
};

template <>
struct ByAddress<frontend::tail::raceFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::tail::queuesClear>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The node in eax, where it goes in edx.
template <>
struct ByAddress<frontend::tail::placeNearNode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ node, out }));
    }
};

// The node in eax, where it goes in edx.
template <>
struct ByAddress<frontend::tail::placeAtNode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ node, out }));
    }
};

// The node in eax, the lane in edx.
template <>
struct ByAddress<frontend::tail::laneOffset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 lane)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ node, lane }));
    }
};

template <>
struct ByAddress<frontend::tail::weatherStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The view in eax.
template <>
struct ByAddress<frontend::tail::weatherScatter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// The amount in eax.
template <>
struct ByAddress<frontend::tail::weatherFadeInTime>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 amount)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ amount }));
    }
};

// The node in eax.
template <>
struct ByAddress<frontend::tail::weatherFallAtNode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ node }));
    }
};

// How many were seen in eax, the player in edx.
template <>
struct ByAddress<frontend::tail::weatherShare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 seen, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ seen, player }));
    }
};

// The player in eax.
template <>
struct ByAddress<frontend::tail::weatherViewChange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// The view in eax, how many in edx.
template <>
struct ByAddress<frontend::tail::weatherDropsStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, wanted }));
    }
};

// The view in eax, where the list goes in esi.
template <>
struct ByAddress<frontend::tail::weatherOnView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// The angle in eax, where the sine goes in edx, the cosine in ebx.
template <>
struct ByAddress<frontend::tail::angleParts>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 angle, x86::reg32 sineOut,
                           x86::reg32 cosineOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ angle, sineOut, cosineOut }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
