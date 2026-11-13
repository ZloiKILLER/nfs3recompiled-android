#ifndef NFS3HP_NATIVE_GRAPHICS_H
#define NFS3HP_NATIVE_GRAPHICS_H

#include "native_guest.h"

/* native_graphics.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * eax as the code leaves it where its comment says so. */
namespace nfs3hp
{
namespace graphics
{

// The game's 0x431900: the render state `state` set to `value`; eax its answer.
bool sendState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value);
// The game's 0x432390: the software blending's rows for the colour 0xAARRGGBB chosen and sent.
bool softwareBlendColour(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 colour);
// The game's 0x433d40: a quad of a polygon list with colours of its own drawn.
bool colouredQuadRecord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x4347d0: a line of a polygon list drawn.
bool lineRecord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x482940: player's message box in the HUD, `text` in it.
bool messageBox(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 text);
// The game's 0x495230: the loading bar at the per cent at `perCentAt` (the address of the word the
// code pushes; 101 written back there past 100).
bool loadingProgress(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 perCentAt);
// The game's 0x49c600: a texture of `width` by `height` allocated; eax its handle, as the driver
// leaves it.
bool textureAllocated(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height);
// textureAllocated's handle.
inline std::optional<x86::reg32> allocatedTexture(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width,
                                                  x86::reg32 height)
{
    if (!textureAllocated(app, cpu, width, height))
        return std::nullopt;
    return cpu.eax;
}
// The game's 0x49c620: `texture` given the `width` by `height` 1555 pixels at `pixels`.
bool textureUpdated(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 texture, x86::reg32 pixels,
                    x86::reg32 width, x86::reg32 height);
// The game's 0x4bee80: the screen mode the Screen Size list would have for `width`, 1 for none.
std::optional<x86::reg32> screenModeFor(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width);
// The game's 0x4eb3c0: the table of reciprocals made, once.
bool reciprocalsMade(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4f8830: the screen's functions kept for the driver.
bool screenFunctionsSet(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace graphics

namespace guest
{

// The state in eax, the value in edx.
template <>
struct ByAddress<graphics::sendState>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value)
    {
        return watcom(app, cpu, Address, {state, value});
    }
};

// Those of one argument in eax.
struct GraphicsOneArgument
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 argument)
    {
        return watcom(app, cpu, Address, {argument});
    }
};

template <>
struct ByAddress<graphics::softwareBlendColour> : GraphicsOneArgument
{
};

template <>
struct ByAddress<graphics::colouredQuadRecord> : GraphicsOneArgument
{
};

template <>
struct ByAddress<graphics::lineRecord> : GraphicsOneArgument
{
};

// The player in eax, the text in edx.
template <>
struct ByAddress<graphics::messageBox>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 text)
    {
        return watcom(app, cpu, Address, {player, text});
    }
};

// The per cent's word on the stack (ret 4), and what the code writes there back to `perCentAt`.
template <>
struct ByAddress<graphics::loadingProgress>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 perCentAt)
    {
        const KeptRegisters kept(cpu);
        const x86::reg32 pushed = cpu.esp - 4;
        if (!guest::call(app, cpu, Address, {at(app, perCentAt)}))
            return false;
        put(app, perCentAt, at(app, pushed));
        return true;
    }
};

// The width in eax, the height in edx; eax the handle.
template <>
struct ByAddress<graphics::allocatedTexture>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width,
                                          x86::reg32 height)
    {
        return watcomEax(app, cpu, Address, {width, height});
    }
};

// The texture in eax, the pixels in edx, the width in ebx, the height in ecx.
template <>
struct ByAddress<graphics::textureUpdated>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 texture, x86::reg32 pixels,
                     x86::reg32 width, x86::reg32 height)
    {
        return watcom(app, cpu, Address, {texture, pixels, width, height});
    }
};

// The width in eax; eax the mode.
template <>
struct ByAddress<graphics::screenModeFor>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width)
    {
        return watcomEax(app, cpu, Address, {width});
    }
};

// Those of no argument.
struct GraphicsNoArgument
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<graphics::reciprocalsMade> : GraphicsNoArgument
{
};

template <>
struct ByAddress<graphics::screenFunctionsSet> : GraphicsNoArgument
{
};

}  // namespace guest
}  // namespace nfs3hp

#endif
