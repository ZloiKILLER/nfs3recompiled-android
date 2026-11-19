#ifndef NFS3HP_NATIVE_FRONTEND_RENDE_H
#define NFS3HP_NATIVE_FRONTEND_RENDE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rende.cpp's typed functions (the race's drawing around the port's vertex and polygon
 * code: a quad on the card, the skid marks, the depth buckets, the blinking quads, the particles, the
 * smoke, the effects' textures), which other files call as C++ (guest::direct), and how the game's code
 * calls each by address.  Each calls the game's other functions by
 * address on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result
 * is what the game's function leaves in eax.  A float on the stack comes as its bits. */
namespace nfs3hp
{
namespace frontend
{
namespace rende
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4c1900: the quad a b c d drawn, clipped by its corners' codes.
u32 drawQuadClipped(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d);
// The game's 0x4c19e0: the same as the triangles a b c and a c d.
u32 drawQuadSplit(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d);
// The game's 0x4c6cb0: a skid segment on the strip being laid, or a new one.
u32 skidAddSegment(App* app, ThreadState& ts, u32 from, u32 to, u32 alphaFrom, u32 alphaTo, u32 mode);
// The game's 0x4c6f00: the skid strips near the view drawn as one list.
u32 skidMarksDraw(App* app, ThreadState& ts, u32 view);
// The game's 0x4c7180: each polygon's depth; the least to *least, the scale to *scale.
u32 polygonDepths(App* app, ThreadState& ts, u32 list, u32 least, u32 scale);
// The game's 0x4c7260: each polygon into its bucket by depth.
u32 polygonsIntoBuckets(App* app, ThreadState& ts, u32 list, u32 least, u32 scale);
// The game's 0x4c7380: a list sorted by depth, to *result.
u32 polygonsSorted(App* app, ThreadState& ts, u32 result, u32 list, u32 last);
// The game's 0x4c73e0: the depth buckets cleared, when the polygons are sorted here.
u32 clearDepthBuckets(App* app, ThreadState& ts);
// The game's 0x4c7400: a list onto the bucket of a depth.
u32 depthBucketAppend(App* app, ThreadState& ts, u32 view, u32 first, u32 last, u32 depth);
// The game's 0x4c7480: each polygon of a list onto its own bucket.
u32 depthBucketsAdd(App* app, ThreadState& ts, u32 list, u32 base);
// The game's 0x4c7580: the depth buckets, deepest first, as one list to *result.
u32 depthBucketsGather(App* app, ThreadState& ts, u32 result);
// The game's 0x4c75f0: the far buckets cleared, on the card.
u32 clearFarBuckets(App* app, ThreadState& ts);
// The game's 0x4c76e0: the far buckets as one list to *result.
u32 farBucketsGather(App* app, ThreadState& ts, u32 result);
// The game's 0x4c7750: on the card, the near buckets cleared, and the depth ones when asked.
u32 clearBuckets(App* app, ThreadState& ts, u32 depthToo);
// The game's 0x4c7940: a list for the frame: pending on the card, else by depth.
u32 drawListAdd(App* app, ThreadState& ts, u32 first, u32 last, u32 base);
// The game's 0x4c9f70: the eight blinking lights on the track, those lit and in view.
u32 blinkingQuads(App* app, ThreadState& ts, u32 view);
// The game's 0x4cb220: a view's fading particles a step on.
u32 fadeParticles(App* app, ThreadState& ts, u32 view);
// The game's 0x4cb590: each kind of particle's textures out of sfx.fsh.
u32 loadParticleTextures(App* app, ThreadState& ts, u32 file);
// The game's 0x4cb6f0: the particles' states and fades cleared.
u32 clearParticles(App* app, ThreadState& ts);
// The game's 0x4cb730: the particles' states cleared.
u32 clearParticleStates(App* app, ThreadState& ts);
// The game's 0x4cc540: a view's camera changed: its particles' state bytes cleared.
u32 particlesCameraChanged(App* app, ThreadState& ts, u32 view);
// The game's 0x4cc5c0: the race frame's pass of the particles.
u32 particlesPass(App* app, ThreadState& ts, u32 view);
// The game's 0x4cc600: the frame's pass of the drops, the list to *result.
u32 dropsPass(App* app, ThreadState& ts, u32 view, u32 result);
// The game's 0x4d18b0: how many bits a texture's format has.
u32 formatBits(App* app, ThreadState& ts, u32 format);
// The game's 0x4d3380: an 8-bit shape widened to 16 bits by its palette.
u32 expandPalettedShape(App* app, ThreadState& ts, u32 shape, u32 to, u32 palette);
// The game's 0x4d4080: a shape made the texture `record`.
u32 loadShapeTexture(App* app, ThreadState& ts, u32 file, u32 named, u32 format, u32 record, u32 second);
// The game's 0x4d4120: the greatest power of two up to 256 no more than the lesser.
u32 fitPowerOfTwo(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4d4440: a bar of the cabin's strip into the buffer.
u32 copyCabinBar(App* app, ThreadState& ts, u32 bar, u32 shape, u32 buffer);
// The game's 0x4d5370: sfx.fsh's textures, each to its record.
u32 loadEffectTextures(App* app, ThreadState& ts);
// The game's 0x4d56b0: an empty function; eax as it came.
u32 nothing(App* app, ThreadState& ts);
// The game's 0x4d56f0: sfx.fsh loaded, the round robin from its start.
u32 loadEffects(App* app, ThreadState& ts);
// The game's 0x4db750: a piece of a skid mark, halved until it is short.
u32 skidSubdivide(App* app, ThreadState& ts, u32 depth, u32 from, u32 to, u32 alphaFrom, u32 alphaTo,
                    u32 mode);
// The game's 0x4db8b0: the two sides of a mark at `centre` to `out`.
u32 skidStrip(App* app, ThreadState& ts, u32 centre, u32 from, u32 to, u32 axle, u32 out);
// The game's 0x4db940: a car's wheel's mark begun, laid on or ended.
u32 skidWheel(App* app, ThreadState& ts, u32 car, u32 wheel, u32 position, u32 axle, u32 released,
                u32 strength);
// The game's 0x4dbcc0: the camera's horizontal angle by View Angle.
u32 defaultFieldOfView(App* app, ThreadState& ts);
// The game's 0x4dd930: a puff of smoke at the point, seen from the view.
u32 smokeEmit(App* app, ThreadState& ts, u32 view, u32 pointX, u32 pointY, u32 pointZ);
// The game's 0x4dedf0: the frame's pass of the sparks, the list to *result.
u32 sparksPass(App* app, ThreadState& ts, u32 view, u32 result);

}  // namespace rende
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
