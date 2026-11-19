#ifndef NFS3HP_NATIVE_FRONTEND_SIMB_H
#define NFS3HP_NATIVE_FRONTEND_SIMB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simb.cpp's typed functions (a car on the track: put on the road, its probes and
 * the road ahead, what it goes into, its downforce and drag, its numbers on the road, a turn by
 * three angles), which other files call as C++ (guest::direct), and how the game's code calls each
 * by address.  Each calls the game's other functions by
 * address on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax, and what it leaves on the x87's stack stays
 * there. */
namespace nfs3hp
{
namespace frontend
{
namespace simb
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4a0aa0: the car put on node `node`, `offset` (three 16.16 words) from it, facing back or not.
u32 placeOnRoad(App* app, ThreadState& ts, u32 car, u32 node, u32 offset, u32 facesBack);
// The game's 0x4a1210: eight points round the car's box against the track's walls; 1 when one hit.
u32 probeWalls(App* app, ThreadState& ts, u32 car);
// The game's 0x4a1610: the road a step ahead and what it does to the car, its up brought to the
// three floats the game's code pushes -- here at `wanted`, which it writes over as the game writes
// over its stack arguments.
u32 predictContact(App* app, ThreadState& ts, u32 car, u32 wanted);
// The game's 0x4a1ce0: what going into the surface of normal `normal` does to the car.
u32 reactToSurface(App* app, ThreadState& ts, u32 car, u32 normal);
// The game's 0x4a21a0: a car in a tunnel that has reached its node's roof.
u32 hitTunnelRoof(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2260: the downforce and the drag.
u32 downforceAndDrag(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2690: an active car's numbers as it stands on the road.
u32 settle(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2bc0: how far across the road the car is at its node, which it leaves on the x87.
double offsetAcrossValue(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2cc0: a turn by three angles (floats' bits) as one matrix at `out`, their
// magnitudes added to the float at `total`; 1 when one of them is not nothing.
u32 turnMatrix(App* app, ThreadState& ts, u32 out, u32 angleX, u32 angleY, u32 angleZ, u32 total);
// The game's 0x4a2f10: a thirty-second of the car's spin put on its frame.
u32 spinThirtySecond(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2f30: a sixty-fourth of it.
u32 spinSixtyFourth(App* app, ThreadState& ts, u32 car);

}  // namespace simb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
