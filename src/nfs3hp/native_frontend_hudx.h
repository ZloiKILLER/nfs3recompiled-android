#ifndef NFS3HP_NATIVE_FRONTEND_HUDX_H
#define NFS3HP_NATIVE_FRONTEND_HUDX_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudx.cpp's typed functions (the 3D letters' looks, the HUD's records, panes,
 * surface and text), which other files call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudx
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x474d50: the selected look's lights off.
reg32 clearLights(App* app, CPU& cpu);
// The game's 0x477070: the selected look flat, in three colours.
reg32 setFlat(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third);
// The game's 0x477170: the selected look lit from one direction.
reg32 setDirectional(App* app, CPU& cpu, reg32 colour, reg32 direction);
// The game's 0x4771c0: a light added to the selected look: its number, or -2 or -1.
reg32 addLight(App* app, CPU& cpu, reg32 first, reg32 position, reg32 third, reg32 second);
// The game's 0x477470: the selected look's texture.
reg32 setTexture(App* app, CPU& cpu, reg32 texture);
// The game's 0x484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down; eax left as it came.
void clearRebuildFlag(App* app, CPU& cpu);
// The game's 0x484260: a view's HUD on or off, its pane set afresh.
reg32 toggleShown(App* app, CPU& cpu, reg32 view);
// The game's 0x484450: the rear-view mirror on or off, in a race of one view (eax as it came otherwise).
reg32 toggleRearView(App* app, CPU& cpu);
// The game's 0x485640: a view's pane, its elements and its band made afresh.
reg32 rebuildView(App* app, CPU& cpu, reg32 view);
// The game's 0x487340: a group of the HUD's rectangles empty.
reg32 groupClearCount(App* app, CPU& cpu, reg32 group);
// The game's 0x4873c0: a group's last rectangle gone, if it has one.
reg32 groupDropLast(App* app, CPU& cpu, reg32 group);
// The game's 0x48ccf0: where a view's map pane is now, and its box's colour.
reg32 refreshPane(App* app, CPU& cpu, reg32 view);
// The game's 0x48eb60: the zoom stepped in the views.
reg32 stepZoom(App* app, CPU& cpu, reg32 step);
// The game's 0x497590 (stdcall): the length of the vector x, y (floats' bits), which its code leaves on the x87's
// stack; eax as it came.
double vectorLength(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x49bef0: a surface empty, its indices in order.
reg32 resetSurface(App* app, CPU& cpu, reg32 surface);
// The game's 0x49bf40: `pairs` pairs of vertices taken from a surface, the first one's address put at `first`.
reg32 takeVertices(App* app, CPU& cpu, reg32 surface, reg32 pairs, reg32 first);
// The game's 0x4d10e0: a text ending at x `right`, at `y`.
reg32 drawTextToRightEdge(App* app, CPU& cpu, reg32 text, reg32 right, reg32 y);
// The game's 0x4d1310: the two fonts' surfaces begun.
reg32 beginFontSurfaces(App* app, CPU& cpu);
// The game's 0x4d14f0: how far a character (its low byte) of the font drops, at the character size.
reg32 characterDrop(App* app, CPU& cpu, reg32 character);

}  // namespace hudx
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::hudx::clearLights>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The colours in eax, edx and ebx.
template <>
struct ByAddress<frontend::hudx::setFlat>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 second,
                           x86::reg32 third)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ first, second, third }));
    }
};

// The colour in eax, the direction in edx.
template <>
struct ByAddress<frontend::hudx::setDirectional>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 colour, x86::reg32 direction)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ colour, direction }));
    }
};

// In eax, edx, ebx and ecx.
template <>
struct ByAddress<frontend::hudx::addLight>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 position,
                           x86::reg32 third, x86::reg32 second)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ first, position, third, second }));
    }
};

// The texture in eax.
template <>
struct ByAddress<frontend::hudx::setTexture>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 texture)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ texture }));
    }
};

template <>
struct ByAddress<frontend::hudx::clearRebuildFlag>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The view in eax, for each of those of one.
template <>
struct ByAddress<frontend::hudx::toggleShown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudx::toggleRearView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::hudx::rebuildView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudx::groupClearCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 group)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ group }));
    }
};

template <>
struct ByAddress<frontend::hudx::groupDropLast>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 group)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ group }));
    }
};

template <>
struct ByAddress<frontend::hudx::refreshPane>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudx::stepZoom>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 step)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ step }));
    }
};

// x and y on the stack, popped; the length off the x87's stack.
template <>
struct ByAddress<frontend::hudx::vectorLength>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}, { x, y }));
        return double(cpu.fpu.take());
    }
};

// The surface in eax.
template <>
struct ByAddress<frontend::hudx::resetSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface }));
    }
};

// The surface in eax, the pairs in edx, where the first goes in ebx.
template <>
struct ByAddress<frontend::hudx::takeVertices>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface, x86::reg32 pairs,
                           x86::reg32 first)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface, pairs, first }));
    }
};

// The text in eax, x in edx, y in ebx.
template <>
struct ByAddress<frontend::hudx::drawTextToRightEdge>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 right,
                           x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, right, y }));
    }
};

template <>
struct ByAddress<frontend::hudx::beginFontSurfaces>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The character in eax.
template <>
struct ByAddress<frontend::hudx::characterDrop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
