#ifndef NFS3HP_NATIVE_FRONTEND_VIEW_H
#define NFS3HP_NATIVE_FRONTEND_VIEW_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_view.cpp's typed functions (the pan-and-zoom widget's tiles, the interior on
 * them, its state), which other files call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace view
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x464700: the tiles' records made from folder + fileName, the file freed.
reg32 makeTiles(App* app, CPU& cpu, reg32 fileName, reg32 folder);
// The game's 0x4647a0: the same records' textures filled from folder + fileName, the file freed.
reg32 fillTiles(App* app, CPU& cpu, reg32 fileName, reg32 folder);
// The game's 0x464eb0: the car's interior on the tiles, when the car is not the one they hold.
reg32 loadInterior(App* app, CPU& cpu);
// The game's 0x4652f0: the widget's state, its table's +0x14.
reg32 widgetState(App* app, CPU& cpu);

}  // namespace view
}  // namespace frontend

namespace guest
{

// The file's name in eax, its folder in edx.
template <>
struct ByAddress<frontend::view::makeTiles>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fileName, x86::reg32 folder)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ fileName, folder }));
    }
};

// The file's name in eax, its folder in edx.
template <>
struct ByAddress<frontend::view::fillTiles>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fileName, x86::reg32 folder)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ fileName, folder }));
    }
};

template <>
struct ByAddress<frontend::view::loadInterior>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::view::widgetState>
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
