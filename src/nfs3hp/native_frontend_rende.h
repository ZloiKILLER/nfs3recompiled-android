#ifndef NFS3HP_NATIVE_FRONTEND_RENDE_H
#define NFS3HP_NATIVE_FRONTEND_RENDE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rende.cpp's typed functions (the race's drawing around the port's vertex and polygon
 * code: a quad on the card, the skid marks, the depth buckets, the blinking quads, the particles, the
 * smoke, the effects' textures), which other files call as C++ (guest::direct), and how the game's code
 * calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its result
 * is what the game's function leaves in eax.  A float on the stack comes as its bits. */
namespace nfs3hp
{
namespace frontend
{
namespace rende
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4c1900: the quad a b c d drawn, clipped by its corners' codes.
reg32 drawQuadClipped(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x4c19e0: the same as the triangles a b c and a c d.
reg32 drawQuadSplit(App* app, CPU& cpu, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x4c6cb0: a skid segment on the strip being laid, or a new one.
reg32 skidAddSegment(App* app, CPU& cpu, reg32 from, reg32 to, reg32 alphaFrom, reg32 alphaTo, reg32 mode);
// The game's 0x4c6f00: the skid strips near the view drawn as one list.
reg32 skidMarksDraw(App* app, CPU& cpu, reg32 view);
// The game's 0x4c7180: each polygon's depth; the least to *least, the scale to *scale.
reg32 polygonDepths(App* app, CPU& cpu, reg32 list, reg32 least, reg32 scale);
// The game's 0x4c7260: each polygon into its bucket by depth.
reg32 polygonsIntoBuckets(App* app, CPU& cpu, reg32 list, reg32 least, reg32 scale);
// The game's 0x4c7380: a list sorted by depth, to *result.
reg32 polygonsSorted(App* app, CPU& cpu, reg32 result, reg32 list, reg32 last);
// The game's 0x4c73e0: the depth buckets cleared, when the polygons are sorted here.
reg32 clearDepthBuckets(App* app, CPU& cpu);
// The game's 0x4c7400: a list onto the bucket of a depth.
reg32 depthBucketAppend(App* app, CPU& cpu, reg32 view, reg32 first, reg32 last, reg32 depth);
// The game's 0x4c7480: each polygon of a list onto its own bucket.
reg32 depthBucketsAdd(App* app, CPU& cpu, reg32 list, reg32 base);
// The game's 0x4c7580: the depth buckets, deepest first, as one list to *result.
reg32 depthBucketsGather(App* app, CPU& cpu, reg32 result);
// The game's 0x4c75f0: the far buckets cleared, on the card.
reg32 clearFarBuckets(App* app, CPU& cpu);
// The game's 0x4c76e0: the far buckets as one list to *result.
reg32 farBucketsGather(App* app, CPU& cpu, reg32 result);
// The game's 0x4c7750: on the card, the near buckets cleared, and the depth ones when asked.
reg32 clearBuckets(App* app, CPU& cpu, reg32 depthToo);
// The game's 0x4c7940: a list for the frame: pending on the card, else by depth.
reg32 drawListAdd(App* app, CPU& cpu, reg32 first, reg32 last, reg32 base);
// The game's 0x4c9f70: the eight blinking lights on the track, those lit and in view.
reg32 blinkingQuads(App* app, CPU& cpu, reg32 view);
// The game's 0x4cb220: a view's fading particles a step on.
reg32 fadeParticles(App* app, CPU& cpu, reg32 view);
// The game's 0x4cb590: each kind of particle's textures out of sfx.fsh.
reg32 loadParticleTextures(App* app, CPU& cpu, reg32 file);
// The game's 0x4cb6f0: the particles' states and fades cleared.
reg32 clearParticles(App* app, CPU& cpu);
// The game's 0x4cb730: the particles' states cleared.
reg32 clearParticleStates(App* app, CPU& cpu);
// The game's 0x4cc540: a view's camera changed: its particles' state bytes cleared.
reg32 particlesCameraChanged(App* app, CPU& cpu, reg32 view);
// The game's 0x4cc5c0: the race frame's pass of the particles.
reg32 particlesPass(App* app, CPU& cpu, reg32 view);
// The game's 0x4cc600: the frame's pass of the drops, the list to *result.
reg32 dropsPass(App* app, CPU& cpu, reg32 view, reg32 result);
// The game's 0x4d18b0: how many bits a texture's format has.
reg32 formatBits(App* app, CPU& cpu, reg32 format);
// The game's 0x4d3380: an 8-bit shape widened to 16 bits by its palette.
reg32 expandPalettedShape(App* app, CPU& cpu, reg32 shape, reg32 to, reg32 palette);
// The game's 0x4d4080: a shape made the texture `record`.
reg32 loadShapeTexture(App* app, CPU& cpu, reg32 file, reg32 named, reg32 format, reg32 record, reg32 second);
// The game's 0x4d4120: the greatest power of two up to 256 no more than the lesser.
reg32 fitPowerOfTwo(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4d4440: a bar of the cabin's strip into the buffer.
reg32 copyCabinBar(App* app, CPU& cpu, reg32 bar, reg32 shape, reg32 buffer);
// The game's 0x4d5370: sfx.fsh's textures, each to its record.
reg32 loadEffectTextures(App* app, CPU& cpu);
// The game's 0x4d56b0: an empty function; eax as it came.
reg32 nothing(App* app, CPU& cpu);
// The game's 0x4d56f0: sfx.fsh loaded, the round robin from its start.
reg32 loadEffects(App* app, CPU& cpu);
// The game's 0x4db750: a piece of a skid mark, halved until it is short.
reg32 skidSubdivide(App* app, CPU& cpu, reg32 depth, reg32 from, reg32 to, reg32 alphaFrom, reg32 alphaTo,
                    reg32 mode);
// The game's 0x4db8b0: the two sides of a mark at `centre` to `out`.
reg32 skidStrip(App* app, CPU& cpu, reg32 centre, reg32 from, reg32 to, reg32 axle, reg32 out);
// The game's 0x4db940: a car's wheel's mark begun, laid on or ended.
reg32 skidWheel(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 position, reg32 axle, reg32 released,
                reg32 strength);
// The game's 0x4dbcc0: the camera's horizontal angle by View Angle.
reg32 defaultFieldOfView(App* app, CPU& cpu);
// The game's 0x4dd930: a puff of smoke at the point, seen from the view.
reg32 smokeEmit(App* app, CPU& cpu, reg32 view, reg32 pointX, reg32 pointY, reg32 pointZ);
// The game's 0x4dedf0: the frame's pass of the sparks, the list to *result.
reg32 sparksPass(App* app, CPU& cpu, reg32 view, reg32 result);

}  // namespace rende
}  // namespace frontend

namespace guest
{

// a in eax, b in edx, c in ebx, d in ecx.
template <>
struct ByAddress<frontend::rende::drawQuadClipped>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c,
                           x86::reg32 d)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b, c, d }));
    }
};

// a in eax, b in edx, c in ebx, d in ecx.
template <>
struct ByAddress<frontend::rende::drawQuadSplit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c,
                           x86::reg32 d)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b, c, d }));
    }
};

// from in eax, to in edx, alphaFrom in ebx, alphaTo in ecx, mode on the stack.
template <>
struct ByAddress<frontend::rende::skidAddSegment>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to,
                           x86::reg32 alphaFrom, x86::reg32 alphaTo, x86::reg32 mode)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to, alphaFrom, alphaTo }, { mode }));
    }
};

// view in eax.
template <>
struct ByAddress<frontend::rende::skidMarksDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// list in eax, least in edx, scale in ebx.
template <>
struct ByAddress<frontend::rende::polygonDepths>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 least,
                           x86::reg32 scale)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list, least, scale }));
    }
};

// list in eax, least and scale on the stack.
template <>
struct ByAddress<frontend::rende::polygonsIntoBuckets>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 least,
                           x86::reg32 scale)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list }, { least, scale }));
    }
};

// result in esi, list and last on the stack.
template <>
struct ByAddress<frontend::rende::polygonsSorted>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 result, x86::reg32 list,
                           x86::reg32 last)
    {
        cpu.esi = result;
        return fe::callByAddress(app, cpu, Address, fe::args({}, { list, last }));
    }
};

template <>
struct ByAddress<frontend::rende::clearDepthBuckets>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// view in eax, first, last and depth on the stack.
template <>
struct ByAddress<frontend::rende::depthBucketAppend>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 first,
                           x86::reg32 last, x86::reg32 depth)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }, { first, last, depth }));
    }
};

// list in eax, base on the stack.
template <>
struct ByAddress<frontend::rende::depthBucketsAdd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 base)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ list }, { base }));
    }
};

// result in esi.
template <>
struct ByAddress<frontend::rende::depthBucketsGather>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 result)
    {
        cpu.esi = result;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rende::clearFarBuckets>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// result in esi.
template <>
struct ByAddress<frontend::rende::farBucketsGather>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 result)
    {
        cpu.esi = result;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// depthToo in eax.
template <>
struct ByAddress<frontend::rende::clearBuckets>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 depthToo)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ depthToo }));
    }
};

// first, last and base on the stack.
template <>
struct ByAddress<frontend::rende::drawListAdd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 last,
                           x86::reg32 base)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { first, last, base }));
    }
};

// view in eax.
template <>
struct ByAddress<frontend::rende::blinkingQuads>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// view in eax.
template <>
struct ByAddress<frontend::rende::fadeParticles>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// file in eax.
template <>
struct ByAddress<frontend::rende::loadParticleTextures>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 file)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ file }));
    }
};

template <>
struct ByAddress<frontend::rende::clearParticles>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rende::clearParticleStates>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// view in eax.
template <>
struct ByAddress<frontend::rende::particlesCameraChanged>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// view in eax.
template <>
struct ByAddress<frontend::rende::particlesPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// view in eax, result in esi.
template <>
struct ByAddress<frontend::rende::dropsPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 result)
    {
        cpu.esi = result;
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// format in eax.
template <>
struct ByAddress<frontend::rende::formatBits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ format }));
    }
};

// shape in eax, to in edx, palette in ebx.
template <>
struct ByAddress<frontend::rende::expandPalettedShape>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape, x86::reg32 to,
                           x86::reg32 palette)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape, to, palette }));
    }
};

// file in eax, named in edx, format in ebx, record in ecx, second on the stack.
template <>
struct ByAddress<frontend::rende::loadShapeTexture>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 file, x86::reg32 named,
                           x86::reg32 format, x86::reg32 record, x86::reg32 second)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ file, named, format, record }, { second }));
    }
};

// a in eax, b in edx.
template <>
struct ByAddress<frontend::rende::fitPowerOfTwo>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// bar in eax, shape in edx, buffer in ebx.
template <>
struct ByAddress<frontend::rende::copyCabinBar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 bar, x86::reg32 shape,
                           x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ bar, shape, buffer }));
    }
};

template <>
struct ByAddress<frontend::rende::loadEffectTextures>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rende::nothing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rende::loadEffects>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// depth in eax, from in edx, to in ebx, alphaFrom in ecx, alphaTo and mode on the stack.
template <>
struct ByAddress<frontend::rende::skidSubdivide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 depth, x86::reg32 from,
                           x86::reg32 to, x86::reg32 alphaFrom, x86::reg32 alphaTo, x86::reg32 mode)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ depth, from, to, alphaFrom }, { alphaTo, mode }));
    }
};

// centre in eax, from in edx, to in ebx, axle in ecx, out on the stack.
template <>
struct ByAddress<frontend::rende::skidStrip>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 centre, x86::reg32 from,
                           x86::reg32 to, x86::reg32 axle, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ centre, from, to, axle }, { out }));
    }
};

// car in eax, wheel in edx, position in ebx, axle in ecx, released and strength on the stack.
template <>
struct ByAddress<frontend::rende::skidWheel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 wheel,
                           x86::reg32 position, x86::reg32 axle, x86::reg32 released, x86::reg32 strength)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, wheel, position, axle }, { released, strength }));
    }
};

template <>
struct ByAddress<frontend::rende::defaultFieldOfView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// view in eax, pointX, pointY and pointZ on the stack.
template <>
struct ByAddress<frontend::rende::smokeEmit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 pointX,
                           x86::reg32 pointY, x86::reg32 pointZ)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }, { pointX, pointY, pointZ }));
    }
};

// view in eax, result in esi.
template <>
struct ByAddress<frontend::rende::sparksPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 result)
    {
        cpu.esi = result;
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
