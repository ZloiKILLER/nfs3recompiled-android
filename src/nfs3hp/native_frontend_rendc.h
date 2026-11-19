#ifndef NFS3HP_NATIVE_FRONTEND_RENDC_H
#define NFS3HP_NATIVE_FRONTEND_RENDC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendc.cpp's typed functions (what the race draws besides the track and the cars'
 * bodies: the cones, the lightning, a model's parts transformed, the driver's object lists, the
 * track lights' blinking, the sky's sprites and a few small ones), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
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

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// What modelTransformAll and modelTransformPart give back: what the game's function leaves in eax,
// and the nearest depth it leaves on the x87's stack (0 for none).
struct Nearest
{
    u32 eax;
    double depth;
};

// The game's 0x491120: the colour of track light `light`, 0 while it blinks off.
u32 blinkColour(App* app, ThreadState& ts, u32 light);
// The game's 0x493380: how far across slice `slice` the kind of road `kind` puts a cone, 16.16.
u32 coneSideOffset(App* app, ThreadState& ts, u32 slice, u32 kind);
// The game's 0x4934c0: where a cone of `side` stands on slice `slice`, three floats to `out` (esi).
u32 coneEdgePoint(App* app, ThreadState& ts, u32 slice, u32 side, u32 out);
// The game's 0x493640: both rows of cones set out for a race.
u32 conesPlace(App* app, ThreadState& ts);
// The game's 0x493a30: the car's pass of the cones counted, good or bad.
u32 coneCountPass(App* app, ThreadState& ts, u32 car);
// The game's 0x493ad0: the cones near the car tested against its box, those inside knocked.
u32 conesHit(App* app, ThreadState& ts, u32 car);
// The game's 0x493e50: every cone back up at its home.
u32 conesReset(App* app, ThreadState& ts);
u32 conesResetKeeping(App* app, ThreadState& ts, u32 eax);  // eax: what the game's function was entered with
// The game's 0x493ef0: the cones' rows and model, as the race loads.
u32 conesLoad(App* app, ThreadState& ts);
u32 conesLoadKeeping(App* app, ThreadState& ts, u32 eax);
// The game's 0x493fb0: the cones' rows and model given back.
u32 conesFree(App* app, ThreadState& ts);
u32 conesFreeKeeping(App* app, ThreadState& ts, u32 eax);
// The game's 0x494010: every cone stepped up to the race's tick.
u32 conesStep(App* app, ThreadState& ts);
// The game's 0x494170: the cones 20 slices either way of slice `centre` queued for view `view`.
u32 conesDraw(App* app, ThreadState& ts, u32 view, u32 centre);
// The game's 0x494360: the lightning's table cleared.
u32 boltsClear(App* app, ThreadState& ts);
// The game's 0x494390: the bolt alive's point to `out` and its brightness to `brightness`: 1, or 0 without one.
u32 boltGlow(App* app, ThreadState& ts, u32 out, u32 brightness);
// The game's 0x4943e0: a free node of the lightning's table, 0 for none.
u32 boltFreeNode(App* app, ThreadState& ts);
// The game's 0x494420: a node one segment on from `parent` into `slot`, and the bolt on from it for `depth` more, a
// fork at `forkAt`, bent by up to the float `bend`: 1 if it made one.
u32 boltBranch(App* app, ThreadState& ts, u32 parent, u32 slot, u32 depth, u32 forkAt, u32 bend);
// The game's 0x4945a0: a bolt struck at the floats x, y, z, turned by `angle`: what its tree made.
u32 boltStrike(App* app, ThreadState& ts, u32 x, u32 y, u32 z, u32 angle);
// The game's 0x494630: every live node of the lightning stepped up to the race's tick.
u32 boltsStep(App* app, ThreadState& ts);
// The game's 0x495600: the picture shown on the way back to the menus.
u32 backImage(App* app, ThreadState& ts, u32 eax, u32 edx, u32 ebx, u32 ecx);
// The game's 0x4964d0: the float at `value` cut to its fraction.
u32 wrapFraction(App* app, ThreadState& ts, u32 value);
// The game's 0x496550: a rough distance between the 16.16 points at `a` and `b`.
u32 approximateDistance(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x496bf0: the floats x and y turned by the fraction of a turn `turn`, to the floats at `outX` and
// `outY`.
u32 rotatePlane(App* app, ThreadState& ts, u32 x, u32 y, u32 turn, u32 outX, u32 outY);
// The game's 0x496f20: 1 when the points at `a` and `b` are nearer on the ground than the float `limit`.
u32 withinFlatDistance(App* app, ThreadState& ts, u32 a, u32 b, u32 limit);
// The game's 0x49c410: the driver's objects started afresh.
u32 objectsBegin(App* app, ThreadState& ts);
// The game's 0x49c420: the objects at `pair` drawn for view `view` at depth `depth`, the new pair back into `pair`.
u32 objectsDraw(App* app, ThreadState& ts, u32 view, u32 pair, u32 depth);
// The game's 0x49c470: the driver's objects' two ends to `out` (esi).
u32 objectsEnd(App* app, ThreadState& ts, u32 out);
// The game's 0x49c4a0: the driver's chain of objects copied, the copy's two ends to `out` (esi).
u32 objectsCopy(App* app, ThreadState& ts, u32 out);
// The game's 0x49d5a0: a model's data and the model given back.
u32 modelFree(App* app, ThreadState& ts, u32 model);
// The game's 0x49d5c0: all of a model's parts at their places: the nearest depth among them.
Nearest modelTransformAll(App* app, ThreadState& ts, u32 model, u32 matrix1, u32 place1, u32 matrix2, u32 place2,
                          u32 environment);
// The game's 0x49d720: the alphas a model is drawn with, `high` and `low`.
u32 modelSetAlphas(App* app, ThreadState& ts, u32 high, u32 low);
// The game's 0x49d750: whether a model's vertices are lit by their normals.
u32 modelSetShading(App* app, ThreadState& ts, u32 shading);
// The game's 0x49d760: the direction a model is lit from, three floats at `light`.
u32 modelSetLight(App* app, ThreadState& ts, u32 light);
// The game's 0x49d780: the matrix an environment map turns by, from the direction at `direction`.
u32 modelSetEnvironment(App* app, ThreadState& ts, u32 direction);
// The game's 0x49d8f0: the colour a model is lit from.
u32 modelSetColour(App* app, ThreadState& ts, u32 colour);
// The game's 0x49de20: part `part` of a model transformed by up to three steps and projected, its polygons culled:
// the nearest depth of its vertices.
Nearest modelTransformPart(App* app, ThreadState& ts, u32 model, u32 part, u32 matrix0, u32 place0, u32 matrix1,
                           u32 place1, u32 matrix2, u32 place2, u32 environment);
/* The game's 0x49d5c0 and 0x49de20 as the game's code calls them, for register code (native_render.cpp):
 * their register arguments in `args` and the stack ones above the return address's slot at esp, `args` left as
 * their thunks leave it (eax the result) and esp moved on (the stack's arguments taken off); the depth the
 * thunks leave as the float result, nullopt when the game stopped in them. */
std::optional<double> modelTransformAllCalled(App* app, ThreadState& ts, nfs::Args& args);
std::optional<double> modelTransformPartCalled(App* app, ThreadState& ts, nfs::Args& args);
// The game's 0x4a4060: a whole number from `a` to `b`, either way round, from the game's table of random bytes.
u32 skyRandomBetween(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4a40c0: sky sprite `sprite` put back in at an edge of the screen.
u32 skySpritePlace(App* app, ThreadState& ts, u32 sprite);

}  // namespace rendc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
