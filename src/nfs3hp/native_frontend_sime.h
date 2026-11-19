#ifndef NFS3HP_NATIVE_FRONTEND_SIME_H
#define NFS3HP_NATIVE_FRONTEND_SIME_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_sime.cpp's typed functions (the cars' numbered folders, the fog at a camera, the
 * root table and a quick normal, the race's effects: skid strips, wheel marks, particles, sparks,
 * puffs, drops; the views resized, the track's pieces loaded), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax, and what it leaves in the registers it does not save stays there.  A function handed a
 * record on the stack takes the address of the words the game's code pushes (`on`, `parent`): a
 * C++ caller puts them where those pushes put them. */
namespace nfs3hp
{
namespace frontend
{
namespace sime
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x43bcf0: the cars' folders named by a number removed.
u32 removeNumberedFolders(App* app, ThreadState& ts);
// The game's 0x478e10: the fog's colour and distance at a view's camera.
u32 fogAtCamera(App* app, ThreadState& ts, u32 view, u32 colourOut, u32 distanceOut);
// The game's 0x496310: the table of reciprocal roots made.
u32 makeRootTable(App* app, ThreadState& ts);
// The game's 0x497500: the vector made about one long without a root, to `out`; eax the vector.
u32 quickNormal(App* app, ThreadState& ts, u32 vector, u32 out);
// The game's 0x4bc8b0: the race's effects reset.
u32 effectsReset(App* app, ThreadState& ts);
// The game's 0x4bc950: the race's effects freed.
u32 effectsFree(App* app, ThreadState& ts);
// The game's 0x4be970: the views made larger or smaller.
u32 viewsResize(App* app, ThreadState& ts, u32 direction);
// The game's 0x4bea40: a view made larger or smaller by `change`.
u32 viewResize(App* app, ThreadState& ts, u32 view, u32 change);
// The game's 0x4c6c70: a skid strip's segment put.
u32 skidSegmentPut(App* app, ThreadState& ts, u32 corners, u32 alphas, u32 from, u32 to, u32 alphaFrom, u32 alphaTo);
// The game's 0x4c70f0: the skid strips' memory taken.
u32 skidStripsTake(App* app, ThreadState& ts);
// The game's 0x4c7130: the skid strips reset.
u32 skidStripsReset(App* app, ThreadState& ts);
// The game's 0x4c7150: the skid strips' memory freed.
u32 skidStripsFree(App* app, ThreadState& ts);
// The game's 0x4c7b20: a race's end: nothing, eax as it came.
u32 effectsEndEmpty(App* app, ThreadState& ts, u32 eax);
// The game's 0x4ca160: the strip under a car's wheels.
u32 stripUnderWheels(App* app, ThreadState& ts, u32 car, u32 out);
// The game's 0x4ca4b0: a slot for a new particle.
u32 particleSlot(App* app, ThreadState& ts);
// The game's 0x4ca500: the rain's drops on a player's view.
u32 dropsOnView(App* app, ThreadState& ts, u32 player, u32 on);
// The game's 0x4ca850: a particle of a kind where the words at `on` say; 0 or the particle.
u32 spawnParticle(App* app, ThreadState& ts, u32 kind, u32 on);
// The game's 0x4cab60: a dying particle's children: the particle a copy at `parent`.
u32 particleChildren(App* app, ThreadState& ts, u32 parent);
// The game's 0x4cac20: sparks off a car where the words at `on` say.
u32 sparks(App* app, ThreadState& ts, u32 car, u32 on);
// The game's 0x4cae00: a puff of a kind where the words at `on` say.
u32 crashPuff(App* app, ThreadState& ts, u32 kind, u32 on);
// The game's 0x4caea0: the sparkle round a car as it is put back.
u32 respawnSparkle(App* app, ThreadState& ts, u32 car, u32 on);
// The game's 0x4cb010: a wheel's dust of a kind off a car.
u32 wheelParticles(App* app, ThreadState& ts, u32 car, u32 wheel, u32 on);
// The game's 0x4d5730: a race's end (0x4d56b0), every register handed on.
u32 effectsEndNothing(App* app, ThreadState& ts);
// The game's 0x4db860: a wheel's mark cut.
u32 wheelMarkCut(App* app, ThreadState& ts, u32 car, u32 wheel);
// The game's 0x4dbba0: the wheel marks reset as the race loads.
u32 wheelMarksResetAtLoad(App* app, ThreadState& ts);
// The game's 0x4dbbc0: the wheel marks reset.
u32 wheelMarksReset(App* app, ThreadState& ts);
// The game's 0x4dbbe0: a race's end for the marks: nothing, eax as it came.
u32 wheelMarksEndEmpty(App* app, ThreadState& ts, u32 eax);
// The game's 0x4dbbf0: the track's pieces loaded.
u32 loadTrackPieces(App* app, ThreadState& ts, u32 track);

}  // namespace sime
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
