#ifndef NFS3HP_NATIVE_FRONTEND_MOVA_H
#define NFS3HP_NATIVE_FRONTEND_MOVA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_mova.cpp's typed functions (the movie's display, the integer projection), which
 * other files call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace mova
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4eba80: the screen's device, as DirectDraw's library keeps it.
reg32 screenDevice(App* app, CPU& cpu);
// The game's 0x4eba90: DirectDraw's object.
reg32 directDraw(App* app, CPU& cpu);
// The game's 0x4df340: the way a movie's frame reaches the screen.
reg32 displayMode(App* app, CPU& cpu);
// The game's 0x4df310: the movie's overlay surface let go, when the screen's device is the one it was made for.
reg32 displayClose(App* app, CPU& cpu);
// The game's 0x4df1f0: a movie's display opened at x, y for a frame of width by height, its quality, software.
reg32 displayOpen(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 quality, reg32 software);
// The game's 0x4fd280: the integer projection's centre at x, y, its far edges from its half sizes.
reg32 projectionCentre(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x4fd2b0: the projection's view: its half angles, the view's width and height.
reg32 projectionView(App* app, CPU& cpu, reg32 angleX, reg32 angleY, reg32 width, reg32 height);
// The game's 0x4fd3f0: the projection's view with its half angles in degrees, the width and height handed on.
reg32 projectionViewDegrees(App* app, CPU& cpu, reg32 degreesX, reg32 degreesY, reg32 width, reg32 height);

}  // namespace mova
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::mova::screenDevice>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::mova::directDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::mova::displayMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::mova::displayClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// x in eax, y in edx, the width in ebx, the height in ecx, the quality and software on the stack.
template <>
struct ByAddress<frontend::mova::displayOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y, x86::reg32 width,
                           x86::reg32 height, x86::reg32 quality, x86::reg32 software)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y, width, height }, { quality, software }));
    }
};

// x in eax, y in edx.
template <>
struct ByAddress<frontend::mova::projectionCentre>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y }));
    }
};

// The angles in eax and edx, the width in ebx, the height in ecx.
template <>
struct ByAddress<frontend::mova::projectionView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 angleX, x86::reg32 angleY,
                           x86::reg32 width, x86::reg32 height)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ angleX, angleY, width, height }));
    }
};

// The degrees in eax and edx, the width in ebx, the height in ecx.
template <>
struct ByAddress<frontend::mova::projectionViewDegrees>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 degreesX, x86::reg32 degreesY,
                           x86::reg32 width, x86::reg32 height)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ degreesX, degreesY, width, height }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
