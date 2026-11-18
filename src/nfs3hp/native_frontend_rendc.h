#ifndef NFS3HP_NATIVE_FRONTEND_RENDC_H
#define NFS3HP_NATIVE_FRONTEND_RENDC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendc.cpp's typed functions (what the race draws besides the track and the cars'
 * bodies: the cones, the lightning, a model's parts transformed, the driver's object lists, the
 * track lights' blinking, the sky's sprites and a few small ones), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax.  Several also leave registers they do not save as their code does (coneSideOffset's and
 * skyRandomBetween's edx, modelTransformPart's ecx and edx): a caller that reads such a register
 * calls by address.  modelTransformAll and modelTransformPart give back the depth their thunks leave
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
reg32 conesResetKeeping(App* app, CPU& cpu, reg32 eax);  // eax: what the game's function was entered with
// The game's 0x493ef0: the cones' rows and model, as the race loads.
reg32 conesLoad(App* app, CPU& cpu);
reg32 conesLoadKeeping(App* app, CPU& cpu, reg32 eax);
// The game's 0x493fb0: the cones' rows and model given back.
reg32 conesFree(App* app, CPU& cpu);
reg32 conesFreeKeeping(App* app, CPU& cpu, reg32 eax);
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
reg32 backImage(App* app, CPU& cpu, reg32 eax, reg32 edx, reg32 ebx, reg32 ecx);
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
 * their register arguments in `args` and the stack ones above the return address's slot at esp, `args` left as
 * their thunks leave it (eax the result) and esp moved on (the stack's arguments taken off); the depth the
 * thunks leave as the float result, nullopt when the game stopped in them. */
std::optional<double> modelTransformAllCalled(App* app, CPU& cpu, x86::Args& args);
std::optional<double> modelTransformPartCalled(App* app, CPU& cpu, x86::Args& args);
// The game's 0x4a4060: a whole number from `a` to `b`, either way round, from the game's table of random bytes.
reg32 skyRandomBetween(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4a40c0: sky sprite `sprite` put back in at an edge of the screen.
reg32 skySpritePlace(App* app, CPU& cpu, reg32 sprite);

}  // namespace rendc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
