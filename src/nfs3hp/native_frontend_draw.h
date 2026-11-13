#ifndef NFS3HP_NATIVE_FRONTEND_DRAW_H
#define NFS3HP_NATIVE_FRONTEND_DRAW_H

#include "native_guest.h"

/* native_frontend_draw.cpp's typed functions (a slider's value and a two-axis slider's onto the
 * x87's stack, a .qfs loaded by its name), which other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the checks' stand-ins).
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in, runs on `cpu` as the code does, and returns what
 * it leaves in eax -- or nothing, its value left on the x87's stack.  When the game stopped in a
 * call they return at once with cpu.terminate set, as the code does -- they do not throw -- and
 * so do their calls by address here. */
namespace nfs3hp
{
namespace frontend
{
namespace draw
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x469520: the value of the slider at `slider`, onto the x87's stack.
void sliderStoredValue(App* app, CPU& cpu, reg32 slider);
// The game's 0x46afd0: value `end` of the two-axis slider at `slider`, onto the x87's stack.
void twoAxisValue(App* app, CPU& cpu, reg32 slider, reg32 end);
// The game's 0x4956f0: the .qfs file named at `fileName` loaded, the menus' extension put after a name without one.
reg32 loadQfs(App* app, CPU& cpu, reg32 fileName);

}  // namespace draw
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax and edx, every other register
 * as the caller has it.  With cpu.terminate set when the game stopped in it. */
template <>
struct ByAddress<frontend::draw::sliderStoredValue>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider)
    {
        cpu.eax = slider;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::draw::twoAxisValue>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider, x86::reg32 end)
    {
        cpu.eax = slider;
        cpu.edx = end;
        guest::call(app, cpu, Address);
    }
};

template <>
struct ByAddress<frontend::draw::loadQfs>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fileName)
    {
        cpu.eax = fileName;
        guest::call(app, cpu, Address);
        return cpu.eax;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
