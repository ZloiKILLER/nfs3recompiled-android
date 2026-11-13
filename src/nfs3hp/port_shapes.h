#ifndef NFS3HP_PORT_SHAPES_H
#define NFS3HP_PORT_SHAPES_H

#include "native_guest.h"
#include "port.h"

/* The game's shapes, rewritten (port.h says how): an FSH shape turned into a
 * texture's pixels -- its alpha scanned, a format the driver takes chosen,
 * its pixels converted and resized into it -- a shape made a texture of its
 * own, the menus' art cut into tiles of 32 pixels and each made a texture,
 * found by its name and drawn a tile at a time; the textures let go of.
 *
 * Each function is here twice: as C++ with arguments and a result, which the
 * others call, and as the game's entry by the name the game's code knows
 * (disassemble_nfs3hp.py's PORT_SHAPES_NATIVES binds its address to it), an
 * adapter around the first.  What they call that is not theirs -- the
 * driver's functions, the game's allocator, its render states, its trace --
 * they call by address, from where the game's stack had it. */
namespace nfs3hp
{
namespace port
{
namespace shapes
{

/* Shape `shape` turned into pixels at `target` + 0x10, the size kept at
 * `target` + 4 and + 6: at most `limit` a side (0 for its own size), in the
 * format at `format` -- kept if the driver takes it and it holds the
 * shape's alpha, else chosen and written there.  `flag` where the
 * converters say whether the pixels have alpha.  `esp` the game's at the
 * call (its return address).  The height. */
Result shapeToPixelsOf(App* app, CPU& cpu, reg32 esp, reg32 target, reg32 shape, reg32 flag, int32_t limit,
                       reg32 format);

/* The menus' shape named at `name` (4 characters): its record, found or made
 * from the menus' art; 0 for none. */
Result shapeNamed(App* app, CPU& cpu, reg32 esp, reg32 name);

/* Art record `record` drawn at `x`, `y` in `colour`, a tile at a time. */
Result drawArt(App* app, CPU& cpu, reg32 esp, int32_t record, int32_t x, int32_t y, reg32 colour);

/* The adapters the 2D drawing calls as the game's code does (port_draw2d.cpp). */
void resetTextures(App* app, CPU& cpu);  // 0x4d5c40
void shapeTexture(App* app, CPU& cpu);   // 0x4d6070

/* The typed cores the menus call as C++ (guest::direct) -- shapeNamed (0x4d5db0) and drawArt (0x4d7160) above,
 * and these -- and how the game's code calls each by address (ByAddress below, for the checks' stand-ins).  A
 * Result is nullopt when the game stopped in a call.  A core's `esp` is the frame base its adapter hands it:
 * the game's esp on entry, at the return address.  A caller through guest::direct passes cpu.esp - 4 - 4 * N, N
 * the stack words the game's call pushes: the esp the adapter hands it, so the core's frame lies where the
 * game's does; by address the call makes it. */

// The game's 0x4d5c40: the textures let go of; esp the frame base.
Result texturesLetGo(App* app, CPU& cpu, reg32 esp);
// The game's 0x4d6070: shape `shape` made a texture into the record at `out`, at most `size` a side, in the format
// at `format`; esp the frame base.
Result textureOf(App* app, CPU& cpu, reg32 esp, reg32 out, reg32 shape, int32_t size, reg32 format);
// The game's 0x4d6390: the art shape `shape` made textures, `again` the record to fill again (0 a new one), `ecx`
// handed on; the record; esp the frame base.
Result artTexturesOf(App* app, CPU& cpu, reg32 esp, reg32 shape, int32_t again, reg32 ecx);
// The game's 0x4d7990: art record `record` drawn at x, y, opaque; esp the frame base.
Result drawShapeOf(App* app, CPU& cpu, reg32 esp, int32_t record, int32_t x, int32_t y);

}  // namespace shapes
}  // namespace port

namespace guest
{

// Nothing handed over; eax back.
template <>
struct ByAddress<port::shapes::texturesLetGo>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The name in eax; the record in eax.
template <>
struct ByAddress<port::shapes::shapeNamed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 name)
    {
        return watcomEax(app, cpu, Address, {name});
    }
};

// The record in eax, the shape in edx, the size in ebx, the format in ecx; eax back.
template <>
struct ByAddress<port::shapes::textureOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 out, x86::reg32 shape,
                             std::int32_t size, x86::reg32 format)
    {
        return watcomEax(app, cpu, Address, {out, shape, x86::reg32(size), format});
    }
};

// The shape in eax, the record in edx, `ecx` in ecx (ebx left as it is); the record in eax.
template <>
struct ByAddress<port::shapes::artTexturesOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 shape,
                             std::int32_t again, x86::reg32 ecx)
    {
        return watcomEax(app, cpu, Address, {shape, x86::reg32(again), cpu.ebx, ecx});
    }
};

// The record in eax, x in edx, y in ebx, the colour in ecx; eax back.
template <>
struct ByAddress<port::shapes::drawArt>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t record,
                             std::int32_t x, std::int32_t y, x86::reg32 colour)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(record), x86::reg32(x), x86::reg32(y), colour});
    }
};

// The record in eax, x in edx, y in ebx; eax back.
template <>
struct ByAddress<port::shapes::drawShapeOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t record,
                             std::int32_t x, std::int32_t y)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(record), x86::reg32(x), x86::reg32(y)});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
