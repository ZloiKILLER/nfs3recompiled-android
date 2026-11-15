#ifndef NFS3HP_NATIVE_FRONTEND_GLUE_H
#define NFS3HP_NATIVE_FRONTEND_GLUE_H

#include "native_guest.h"

/* native_frontend_glue.cpp's typed functions (what the main menu calls of the game's own: the
 * controls' digital flags, the sliders' entries, the attract mode's movies, the network's locks,
 * the mouse and the keyboard as keys), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the checks' stand-ins).
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in, runs on `cpu` as the code does, and returns what
 * it leaves in eax -- or nothing where that is no result (eax then as the code leaves it), a
 * slider's value left on the x87's stack.  When the game stopped in a call they return at once
 * with cpu.terminate set, as the code does -- they do not throw -- and so do their calls by
 * address here. */
namespace nfs3hp
{
namespace frontend
{
namespace glue
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x43c000: whether `binding` is digital: 1 unless a controller's that is not a game pad.
reg32 bindingIsDigital(App* app, CPU& cpu, reg32 binding);
// The game's 0x43c040: the game mode's control set looked at, whose steering, throttle and brake are digital.
void noteDigitalControls(App* app, CPU& cpu);
// The game's 0x46c9b0: the slider's value (a range's end `end`), as a float, which its code leaves on the x87's
// stack.
double sliderValue(App* app, CPU& cpu, reg32 slider, reg32 end);
// The game's 0x46cac0: the function the slider's moves go to.
void sliderSetOnMove(App* app, CPU& cpu, reg32 slider, reg32 function);
// The game's 0x46cca0: the slider's bar's colours, `first` (bl, +0x3e) and `second` (dl, +0x3f).
void sliderSetColour(App* app, CPU& cpu, reg32 slider, reg32 second, reg32 first);
// The game's 0x4960e0: the movie named played from the movies' folder, when movies are on; 1.
reg32 playMovie(App* app, CPU& cpu, reg32 movie);
// The game's 0x4961e0: the attract mode's wait started again, from the clock now.
void resetAttractTimer(App* app, CPU& cpu);
// The game's 0x496200: the main menu's attract mode, every pass; 1 when a movie played.
reg32 attractMode(App* app, CPU& cpu);
// The game's 0x49a4d0: the network's three locks freed and forgotten, its record cleared.
void freeNetworkLocks(App* app, CPU& cpu);
// The game's 0x49ab60: 1 when in a network race every other machine's player is ready.
reg32 allPlayersReady(App* app, CPU& cpu);
// The game's 0x49c080: the mouse read into the menus' as a key: Enter, Escape or 0.
reg32 mouseKey(App* app, CPU& cpu);
// The game's 0x49c1f0: whether the menus' pointer is inside `box` (y, x, width, height, words).
reg32 pointerInBox(App* app, CPU& cpu, reg32 box);
// The game's 0x4cc970: the next key off the keyboard's ring: its scan code high, its character low.
reg32 nextKey(App* app, CPU& cpu);

}  // namespace glue
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax, edx and ebx, every other
 * register as the caller has it.  With cpu.terminate set when the game stopped in it. */
template <>
struct ByAddress<frontend::glue::bindingIsDigital>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 binding)
    {
        cpu.eax = binding;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::noteDigitalControls>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::glue::sliderValue>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider, x86::reg32 end)
    {
        cpu.eax = slider;
        cpu.edx = end;
        if (!guest::call(app, cpu, Address))
            return 0.0;
        return double(cpu.fpu.take());
    }
};

template <>
struct ByAddress<frontend::glue::sliderSetOnMove>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider, x86::reg32 function)
    {
        cpu.eax = slider;
        cpu.edx = function;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::glue::sliderSetColour>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider, x86::reg32 second,
                     x86::reg32 first)
    {
        cpu.eax = slider;
        cpu.edx = second;
        cpu.ebx = first;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::glue::playMovie>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 movie)
    {
        cpu.eax = movie;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::resetAttractTimer>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::glue::attractMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::freeNetworkLocks>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::glue::allPlayersReady>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::mouseKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::pointerInBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 box)
    {
        cpu.eax = box;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

template <>
struct ByAddress<frontend::glue::nextKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
