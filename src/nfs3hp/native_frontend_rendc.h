#ifndef NFS3HP_NATIVE_FRONTEND_RENDC_H
#define NFS3HP_NATIVE_FRONTEND_RENDC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendc.cpp's typed functions (what the race draws besides the track and the cars'
 * bodies: the cones, the lightning, a model's parts transformed, the driver's object lists, the
 * track lights' blinking, the sky's sprites and a few small ones), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax.  Several also leave registers they do not save as their code does (coneSideOffset's and
 * skyRandomBetween's edx, modelTransformPart's ecx and edx): a caller that reads such a register
 * calls by address.  modelTransformAll and modelTransformPart give back the depth their adapters leave
 * on the x87 besides their eax (Nearest). */
namespace nfs3hp
{
namespace frontend
{
namespace rendc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// What modelTransformAll and modelTransformPart give back: what the game's function leaves in eax,
// and the nearest depth it leaves on the x87's stack (0 for none).
struct Nearest
{
    reg32 eax;
    double depth;
};

// The game's 0x491120: the colour of track light `light`, 0 while it blinks off.
reg32 blinkColour(App* app, CPU& cpu, reg32 light);
// The game's 0x493380: how far across slice `slice` the kind of road `kind` puts a cone, 16.16.
reg32 coneSideOffset(App* app, CPU& cpu, reg32 slice, reg32 kind);
// The game's 0x4934c0: where a cone of `side` stands on slice `slice`, three floats to `out` (esi).
reg32 coneEdgePoint(App* app, CPU& cpu, reg32 slice, reg32 side, reg32 out);
// The game's 0x493640: both rows of cones set out for a race.
reg32 conesPlace(App* app, CPU& cpu);
// The game's 0x493a30: the car's pass of the cones counted, good or bad.
reg32 coneCountPass(App* app, CPU& cpu, reg32 car);
// The game's 0x493ad0: the cones near the car tested against its box, those inside knocked.
reg32 conesHit(App* app, CPU& cpu, reg32 car);
// The game's 0x493e50: every cone back up at its home.
reg32 conesReset(App* app, CPU& cpu);
// The game's 0x493ef0: the cones' rows and model, as the race loads.
reg32 conesLoad(App* app, CPU& cpu);
// The game's 0x493fb0: the cones' rows and model given back.
reg32 conesFree(App* app, CPU& cpu);
// The game's 0x494010: every cone stepped up to the race's tick.
reg32 conesStep(App* app, CPU& cpu);
// The game's 0x494170: the cones 20 slices either way of slice `centre` queued for view `view`.
reg32 conesDraw(App* app, CPU& cpu, reg32 view, reg32 centre);
// The game's 0x494360: the lightning's table cleared.
reg32 boltsClear(App* app, CPU& cpu);
// The game's 0x494390: the bolt alive's point to `out` and its brightness to `brightness`: 1, or 0 without one.
reg32 boltGlow(App* app, CPU& cpu, reg32 out, reg32 brightness);
// The game's 0x4943e0: a free node of the lightning's table, 0 for none.
reg32 boltFreeNode(App* app, CPU& cpu);
// The game's 0x494420: a node one segment on from `parent` into `slot`, and the bolt on from it for `depth` more, a
// fork at `forkAt`, bent by up to the float `bend`: 1 if it made one.
reg32 boltBranch(App* app, CPU& cpu, reg32 parent, reg32 slot, reg32 depth, reg32 forkAt, reg32 bend);
// The game's 0x4945a0: a bolt struck at the floats x, y, z, turned by `angle`: what its tree made.
reg32 boltStrike(App* app, CPU& cpu, reg32 x, reg32 y, reg32 z, reg32 angle);
// The game's 0x494630: every live node of the lightning stepped up to the race's tick.
reg32 boltsStep(App* app, CPU& cpu);
// The game's 0x495600: the picture shown on the way back to the menus.
reg32 backImage(App* app, CPU& cpu);
// The game's 0x4964d0: the float at `value` cut to its fraction.
reg32 wrapFraction(App* app, CPU& cpu, reg32 value);
// The game's 0x496550: a rough distance between the 16.16 points at `a` and `b`.
reg32 approximateDistance(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x496bf0: the floats x and y turned by the fraction of a turn `turn`, to the floats at `outX` and
// `outY`.
reg32 rotatePlane(App* app, CPU& cpu, reg32 x, reg32 y, reg32 turn, reg32 outX, reg32 outY);
// The game's 0x496f20: 1 when the points at `a` and `b` are nearer on the ground than the float `limit`.
reg32 withinFlatDistance(App* app, CPU& cpu, reg32 a, reg32 b, reg32 limit);
// The game's 0x49c410: the driver's objects started afresh.
reg32 objectsBegin(App* app, CPU& cpu);
// The game's 0x49c420: the objects at `pair` drawn for view `view` at depth `depth`, the new pair back into `pair`.
reg32 objectsDraw(App* app, CPU& cpu, reg32 view, reg32 pair, reg32 depth);
// The game's 0x49c470: the driver's objects' two ends to `out` (esi).
reg32 objectsEnd(App* app, CPU& cpu, reg32 out);
// The game's 0x49c4a0: the driver's chain of objects copied, the copy's two ends to `out` (esi).
reg32 objectsCopy(App* app, CPU& cpu, reg32 out);
// The game's 0x49d5a0: a model's data and the model given back.
reg32 modelFree(App* app, CPU& cpu, reg32 model);
// The game's 0x49d5c0: all of a model's parts at their places: the nearest depth among them.
Nearest modelTransformAll(App* app, CPU& cpu, reg32 model, reg32 matrix1, reg32 place1, reg32 matrix2, reg32 place2,
                          reg32 environment);
// The game's 0x49d720: the alphas a model is drawn with, `high` and `low`.
reg32 modelSetAlphas(App* app, CPU& cpu, reg32 high, reg32 low);
// The game's 0x49d750: whether a model's vertices are lit by their normals.
reg32 modelSetShading(App* app, CPU& cpu, reg32 shading);
// The game's 0x49d760: the direction a model is lit from, three floats at `light`.
reg32 modelSetLight(App* app, CPU& cpu, reg32 light);
// The game's 0x49d780: the matrix an environment map turns by, from the direction at `direction`.
reg32 modelSetEnvironment(App* app, CPU& cpu, reg32 direction);
// The game's 0x49d8f0: the colour a model is lit from.
reg32 modelSetColour(App* app, CPU& cpu, reg32 colour);
// The game's 0x49de20: part `part` of a model transformed by up to three steps and projected, its polygons culled:
// the nearest depth of its vertices.
Nearest modelTransformPart(App* app, CPU& cpu, reg32 model, reg32 part, reg32 matrix0, reg32 place0, reg32 matrix1,
                           reg32 place1, reg32 matrix2, reg32 place2, reg32 environment);
/* The game's 0x49d5c0 and 0x49de20 as the game's code calls them, for register code (native_render.cpp):
 * their arguments in eax, edx, ebx, ecx and on the stack above the return address's slot at esp, every
 * register and esp left as their adapters leave them (the stack's arguments taken off); the depth the
 * adapters leave on the x87, nullopt when the game stopped in them. */
std::optional<double> modelTransformAllCalled(App* app, CPU& cpu);
std::optional<double> modelTransformPartCalled(App* app, CPU& cpu);
// The game's 0x4a4060: a whole number from `a` to `b`, either way round, from the game's table of random bytes.
reg32 skyRandomBetween(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4a40c0: sky sprite `sprite` put back in at an edge of the screen.
reg32 skySpritePlace(App* app, CPU& cpu, reg32 sprite);

}  // namespace rendc
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::rendc::blinkColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 light)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ light }));
    }
};

template <>
struct ByAddress<frontend::rendc::coneSideOffset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice, x86::reg32 kind)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ slice, kind }));
    }
};

// The slice in eax, the side in edx, out in esi.
template <>
struct ByAddress<frontend::rendc::coneEdgePoint>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice, x86::reg32 side, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({ slice, side }));
    }
};

template <>
struct ByAddress<frontend::rendc::conesPlace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::coneCountPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::rendc::conesHit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::rendc::conesReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::conesLoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::conesFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::conesStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::conesDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 centre)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, centre }));
    }
};

template <>
struct ByAddress<frontend::rendc::boltsClear>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::boltGlow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out, x86::reg32 brightness)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ out, brightness }));
    }
};

template <>
struct ByAddress<frontend::rendc::boltFreeNode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The parent, the slot, the depth and the fork in eax, edx, ebx and ecx; the bend's bits on the stack (ret 4).
template <>
struct ByAddress<frontend::rendc::boltBranch>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 parent, x86::reg32 slot,
                           x86::reg32 depth, x86::reg32 forkAt, x86::reg32 bend)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ parent, slot, depth, forkAt }, { bend }));
    }
};

// All four on the stack (ret 0x10).
template <>
struct ByAddress<frontend::rendc::boltStrike>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y, x86::reg32 z,
                           x86::reg32 angle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { x, y, z, angle }));
    }
};

template <>
struct ByAddress<frontend::rendc::boltsStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::backImage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::wrapFraction>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ value }));
    }
};

template <>
struct ByAddress<frontend::rendc::approximateDistance>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// All five on the stack (ret 0x14).
template <>
struct ByAddress<frontend::rendc::rotatePlane>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y, x86::reg32 turn,
                           x86::reg32 outX, x86::reg32 outY)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { x, y, turn, outX, outY }));
    }
};

// The points in eax and edx, the limit's bits on the stack (ret 4).
template <>
struct ByAddress<frontend::rendc::withinFlatDistance>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 limit)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }, { limit }));
    }
};

template <>
struct ByAddress<frontend::rendc::objectsBegin>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The view in eax, the pair in edx, the depth on the stack (ret 4).
template <>
struct ByAddress<frontend::rendc::objectsDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 pair,
                           x86::reg32 depth)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, pair }, { depth }));
    }
};

// Out in esi.
template <>
struct ByAddress<frontend::rendc::objectsEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// Out in esi.
template <>
struct ByAddress<frontend::rendc::objectsCopy>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::rendc::modelFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 model)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ model }));
    }
};

// Four in eax, edx, ebx and ecx, two on the stack (ret 8); the depth taken off the x87's stack.
template <>
struct ByAddress<frontend::rendc::modelTransformAll>
{
    template <x86::reg32 Address>
    static frontend::rendc::Nearest call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 model,
                                         x86::reg32 matrix1, x86::reg32 place1, x86::reg32 matrix2, x86::reg32 place2,
                                         x86::reg32 environment)
    {
        const x86::reg32 eax = fe::callByAddress(app, cpu, Address, fe::args({ model, matrix1, place1, matrix2 },
                                                 { place2, environment }));
        return { eax, double(cpu.fpu.take()) };
    }
};

template <>
struct ByAddress<frontend::rendc::modelSetAlphas>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 high, x86::reg32 low)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ high, low }));
    }
};

template <>
struct ByAddress<frontend::rendc::modelSetShading>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shading)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shading }));
    }
};

template <>
struct ByAddress<frontend::rendc::modelSetLight>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 light)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ light }));
    }
};

template <>
struct ByAddress<frontend::rendc::modelSetEnvironment>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 direction)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ direction }));
    }
};

template <>
struct ByAddress<frontend::rendc::modelSetColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 colour)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ colour }));
    }
};

// Four in eax, edx, ebx and ecx, five on the stack (ret 0x14); the depth taken off the x87's stack.
template <>
struct ByAddress<frontend::rendc::modelTransformPart>
{
    template <x86::reg32 Address>
    static frontend::rendc::Nearest call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 model,
                                         x86::reg32 part, x86::reg32 matrix0, x86::reg32 place0, x86::reg32 matrix1,
                                         x86::reg32 place1, x86::reg32 matrix2, x86::reg32 place2,
                                         x86::reg32 environment)
    {
        const x86::reg32 eax = fe::callByAddress(app, cpu, Address, fe::args({ model, part, matrix0, place0 },
                                                 { matrix1, place1, matrix2, place2, environment }));
        return { eax, double(cpu.fpu.take()) };
    }
};

template <>
struct ByAddress<frontend::rendc::skyRandomBetween>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

template <>
struct ByAddress<frontend::rendc::skySpritePlace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 sprite)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ sprite }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
