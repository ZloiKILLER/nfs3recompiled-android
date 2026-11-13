#ifndef NFS3HP_NATIVE_RACESTART_H
#define NFS3HP_NATIVE_RACESTART_H

#include "native_guest.h"

/* native_racestart.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * one without `cpu` calls nothing, and through ByAddress leaves cpu.terminate to say it. */
namespace nfs3hp
{
namespace racestart
{

/* What a function's code leaves in eax and edx, which it does not keep: the check compares them,
 * and a caller of this file's passes them on where its code does. */
struct Left
{
    x86::reg32 eax, edx;
};

// The game's 0x429100: the car put back on the road when the player asks; eax as the code leaves it.
std::optional<x86::reg32> resetCarOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x429b40: what wheel `wheel` (0 to 3) of the car is on.
std::optional<x86::reg32> wheelSurfaceOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                         x86::reg32 wheel);
// The game's 0x42b900: the car's frame of race state between the physics'; eax as the code leaves it.
std::optional<x86::reg32> updateCarOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x42bb80: the start's grid, from the track's start.dat or worked out; eax as the code leaves it.
std::optional<x86::reg32> loadStartGridOf(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x42bc70: grid place `place`, its slice to [slice] and its place to `where`.
Left gridPlaceOf(win32::WinApplication* app, x86::reg32 place, x86::reg32 slice, x86::reg32 where);
// The game's 0x42bcb0: the car put on its grid place (`number` its handling's), its race state the start's.
bool placeOnGridOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 number);

}  // namespace racestart

namespace guest
{

// The car in eax.
template <>
struct ByAddress<racestart::resetCarOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

// The car in eax, the wheel in edx; the surface left in eax (and edx).
template <>
struct ByAddress<racestart::wheelSurfaceOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 wheel)
    {
        return watcomEax(app, cpu, Address, {car, wheel});
    }
};

// The car in eax.
template <>
struct ByAddress<racestart::updateCarOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<racestart::loadStartGridOf>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The place in eax, the slice's and the place's addresses in edx and ebx; eax and edx as it leaves them.
template <>
struct ByAddress<racestart::gridPlaceOf>
{
    template <x86::reg32 Address>
    static racestart::Left call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place, x86::reg32 slice,
                                x86::reg32 where)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = place;
        cpu.edx = slice;
        cpu.ebx = where;
        guest::call(app, cpu, Address);
        return {cpu.eax, cpu.edx};
    }
};

// The car in eax, its number in edx.
template <>
struct ByAddress<racestart::placeOnGridOf>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 number)
    {
        return watcom(app, cpu, Address, {car, number});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
