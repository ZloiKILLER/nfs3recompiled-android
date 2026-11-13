#ifndef NFS3HP_NATIVE_FRONTEND_SCREEN_H
#define NFS3HP_NATIVE_FRONTEND_SCREEN_H

#include "native_guest.h"

/* native_frontend_screen.cpp's typed functions (the pointer put where the keys moved the
 * selection, the screens' art loaded), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the checks' stand-ins).
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in and runs on `cpu` as the code does, eax and the
 * registers it does not restore left as the code leaves them.  When the game stopped in a call
 * they return at once with cpu.terminate set, as the code does -- they do not throw -- and so do
 * their calls by address here. */
namespace nfs3hp
{
namespace frontend
{
namespace screen
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x435230: the pointer put at x, y, inside its bounds, what the mouse has buffered taken first.
void placePointer(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x49bfc0: the menus' pointer to x, y on a 640 by 480 screen, and the mouse's to there.
void setPointer(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x440970: the screens' art for the one shown, the items' buffers made anew.
void loadScreenArt(App* app, CPU& cpu);

}  // namespace screen
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax and edx, every other register
 * as the caller has it.  With cpu.terminate set when the game stopped in it. */
template <>
struct ByAddress<frontend::screen::placePointer>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        cpu.eax = x;
        cpu.edx = y;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::screen::setPointer>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        cpu.eax = x;
        cpu.edx = y;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::screen::loadScreenArt>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        guest::call(app, cpu, Address);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
