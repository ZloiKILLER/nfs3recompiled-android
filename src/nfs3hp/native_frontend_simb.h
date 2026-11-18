#ifndef NFS3HP_NATIVE_FRONTEND_SIMB_H
#define NFS3HP_NATIVE_FRONTEND_SIMB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simb.cpp's typed functions (a car on the track: put on the road, its probes and
 * the road ahead, what it goes into, its downforce and drag, its numbers on the road, a turn by
 * three angles), which other files call as C++ (guest::direct), and how the game's code calls each
 * by address.  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax, and what it leaves on the x87's stack stays
 * there. */
namespace nfs3hp
{
namespace frontend
{
namespace simb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4a0aa0: the car put on node `node`, `offset` (three 16.16 words) from it, facing back or not.
reg32 placeOnRoad(App* app, CPU& cpu, reg32 car, reg32 node, reg32 offset, reg32 facesBack);
// The game's 0x4a1210: eight points round the car's box against the track's walls; 1 when one hit.
reg32 probeWalls(App* app, CPU& cpu, reg32 car);
// The game's 0x4a1610: the road a step ahead and what it does to the car, its up brought to the
// three floats the game's code pushes -- here at `wanted`, which it writes over as the game writes
// over its stack arguments.
reg32 predictContact(App* app, CPU& cpu, reg32 car, reg32 wanted);
// The game's 0x4a1ce0: what going into the surface of normal `normal` does to the car.
reg32 reactToSurface(App* app, CPU& cpu, reg32 car, reg32 normal);
// The game's 0x4a21a0: a car in a tunnel that has reached its node's roof.
reg32 hitTunnelRoof(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2260: the downforce and the drag.
reg32 downforceAndDrag(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2690: an active car's numbers as it stands on the road.
reg32 settle(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2bc0: how far across the road the car is at its node, which it leaves on the x87.
double offsetAcrossValue(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2cc0: a turn by three angles (floats' bits) as one matrix at `out`, their
// magnitudes added to the float at `total`; 1 when one of them is not nothing.
reg32 turnMatrix(App* app, CPU& cpu, reg32 out, reg32 angleX, reg32 angleY, reg32 angleZ, reg32 total);
// The game's 0x4a2f10: a thirty-second of the car's spin put on its frame.
reg32 spinThirtySecond(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2f30: a sixty-fourth of it.
reg32 spinSixtyFourth(App* app, CPU& cpu, reg32 car);

}  // namespace simb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
