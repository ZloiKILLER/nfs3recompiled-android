#ifndef NFS3HP_NATIVE_HANDLING_H
#define NFS3HP_NATIVE_HANDLING_H

#include "native_guest.h"

/* native_handling.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and says when the game stopped in one. */
namespace nfs3hp
{
namespace handling
{

// How an axle's tyres ended: the game stopped, gripping, or sliding.
enum class AxleEnd { Stopped, Gripped, Slid };

// The game's 0x4aca4f: the forces of car's axle at `axle` (its record).
AxleEnd axleTyres(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 axle);
// axleTyres for a caller that asks only whether the game went on.
inline bool axleForcesSet(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 axle)
{
    return axleTyres(app, cpu, car, axle) != AxleEnd::Stopped;
}
// The game's 0x4aa8e7: car settled when it is all but stopped and upright; false when the game stopped.
bool settleCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4a78d6: car's handling made for the race from its player's setup; false when the game stopped.
bool tuneHandling(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4a8bb9: the automatic gearbox's choice for car (0 for no change, else the gear).
x86::reg32 gearChoice(win32::WinApplication* app, x86::reg32 car);
// The game's 0x4aa4f4: car's speeds along its axes, its clutch, throttle and slip cleared; false when the game stopped.
bool matchToSpeed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4aa79f: car's drive put back (gear, forces, speeds along the axes cleared).
void resetDrive(win32::WinApplication* app, x86::reg32 car);

}  // namespace handling

namespace guest
{

// The car in eax, the axle's record in edx.
template <>
struct ByAddress<handling::axleForcesSet>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 axle)
    {
        return watcom(app, cpu, Address, {car, axle});
    }
};

// The car in eax.
template <>
struct ByAddress<handling::settleCar>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The car in eax.
template <>
struct ByAddress<handling::tuneHandling>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The car in eax; the choice in eax.
template <>
struct ByAddress<handling::gearChoice>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax;
    }
};

// The car in eax.
template <>
struct ByAddress<handling::matchToSpeed>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The car in eax.
template <>
struct ByAddress<handling::resetDrive>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
