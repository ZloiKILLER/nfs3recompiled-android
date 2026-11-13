#ifndef NFS3HP_NATIVE_CARSTATE_H
#define NFS3HP_NATIVE_CARSTATE_H

#include "native_guest.h"

#include <cstring>

/* native_carstate.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  None of these calls
 * the game's other functions. */
namespace nfs3hp
{
namespace carstate
{

// The game's 0x4217e0: the track's node player's camera keeps to.
x86::reg32 nodeOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x4227e0: for a view of mode 0 its player's camera's zoom's bits, where that is not 0.
std::optional<x86::reg32> ownZoom(win32::WinApplication* app, x86::reg32 view);
// ownZoom as the game's code hands it on: the zoom, or 1.0.
inline float zoomOfView(win32::WinApplication* app, x86::reg32 view)
{
    const std::optional<x86::reg32> own = ownZoom(app, view);
    if (!own)
        return 1.0f;
    float zoom;
    std::memcpy(&zoom, &*own, sizeof zoom);
    return zoom;
}
// The game's 0x422aa0: whether player sees from inside its car.
bool insideView(win32::WinApplication* app, x86::reg32 player);
// The game's 0x422ba0: the car player's camera rides with.
x86::reg32 carOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x422bd0: the car player's camera looks at.
x86::reg32 targetOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x423b00: the mode of player's camera.
x86::reg32 modeOfCamera(win32::WinApplication* app, x86::reg32 player);

}  // namespace carstate

namespace guest
{

// The player in eax; the node in eax.
template <>
struct ByAddress<carstate::nodeOfCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        watcom(app, cpu, Address, {player});
        return cpu.eax;
    }
};

// The view in eax; the zoom left on the x87's stack.
template <>
struct ByAddress<carstate::zoomOfView>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        const std::optional<x86::Float> zoom = watcomFloat(app, cpu, Address, {view});
        return zoom ? float(*zoom) : 1.0f;
    }
};

// The player in eax; 1 or 0 in eax.
template <>
struct ByAddress<carstate::insideView>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        watcom(app, cpu, Address, {player});
        return cpu.eax != 0;
    }
};

// The player in eax; the car in eax.
template <>
struct ByAddress<carstate::carOfCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        watcom(app, cpu, Address, {player});
        return cpu.eax;
    }
};

// The player in eax; the car in eax.
template <>
struct ByAddress<carstate::targetOfCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        watcom(app, cpu, Address, {player});
        return cpu.eax;
    }
};

// The player in eax; the mode in eax.
template <>
struct ByAddress<carstate::modeOfCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        watcom(app, cpu, Address, {player});
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
