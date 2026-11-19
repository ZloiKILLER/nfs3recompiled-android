#ifndef NFS3HP_NATIVE_FRONTEND_SIMC_H
#define NFS3HP_NATIVE_FRONTEND_SIMC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simc.cpp's typed functions (quaternions and matrices, a view's pass, the replay's
 * panel, the television director, small helpers, the view widget), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax, and what it leaves on the x87's stack and in the registers it does not save stays there.
 * A stack argument the game's code hands by its address (`...At`) is taken as that address. */
namespace nfs3hp
{
namespace frontend
{
namespace simc
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4b70f0: a pose mixed between two by the float `t`.
u32 poseBlend(App* app, ThreadState& ts, u32 quatA, u32 quatB, u32 placeA, u32 placeB, u32 t, u32 quatOut, u32 placeOut);
// The game's 0x4b71e0: quaternion slerp by the float `t`.
u32 quatSlerp(App* app, ThreadState& ts, u32 from, u32 to, u32 t, u32 out);
// The game's 0x4b7300: the quaternion at `b` negated if its negative is nearer `a`.
u32 quatNearer(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4b73c0: the 3x3 matrix made a quaternion.
u32 quatFromMatrix(App* app, ThreadState& ts, u32 matrix, u32 out);
// The game's 0x4b7590: the quaternion made a 3x3 matrix.
u32 matrixFromQuat(App* app, ThreadState& ts, u32 quat, u32 out);
// The game's 0x4b7680: two 3x3 matrices slerped through quaternions.
u32 matrixSlerp(App* app, ThreadState& ts, u32 from, u32 to, u32 t, u32 out);
// The game's 0x4b7720: the 3x3 matrix `out` is `a` minus `b`.
u32 matrixSubtract(App* app, ThreadState& ts, u32 a, u32 b, u32 out);
// The game's 0x4b7780: the 3x3 matrix `out` is `a` plus `b`.
u32 matrixAdd(App* app, ThreadState& ts, u32 a, u32 b, u32 out);
// The game's 0x4b77e0: the 3x3 matrix times the float `factor`.
u32 matrixScale(App* app, ThreadState& ts, u32 matrix, u32 factor, u32 out);
// The game's 0x4b7840: the 3x3 matrix made orthonormal again.
u32 matrixOrthonormalise(App* app, ThreadState& ts, u32 matrix, u32 terms);
// The game's 0x4b79c0: the 3x3 matrix turned a quarter.
u32 matrixQuarterTurn(App* app, ThreadState& ts, u32 matrix);
// The game's 0x4ea680: the 3x3 matrix turned about its first row by the angle at `angleAt`.
u32 turnAboutFirstRow(App* app, ThreadState& ts, u32 matrix, u32 angleAt, u32 out);
// The game's 0x4ea820: the 3x3 matrix turned about its last row by the angle at `angleAt`.
u32 turnAboutLastRow(App* app, ThreadState& ts, u32 matrix, u32 angleAt, u32 out);
// The game's 0x4ea920: the square root of the float `value`, which it leaves on the x87.
double squareRootOf(App* app, u32 value);
// The game's 0x4bc470: a view's pass: the view's record filled, the frame drawn through it.
u32 viewPass(App* app, ThreadState& ts, u32 pass, u32 record4, u32 record68, u32 record6c, u32 record70);
// The game's 0x4dbde0: the projection of the view.
u32 setView(App* app, ThreadState& ts, u32 view);
// The game's 0x4d5720: the race frame's end (0x434a90).
u32 raceFrameEnd(App* app, ThreadState& ts);
// The game's 0x4bd4d0: the pointer on the replay's panel: the buttons' word of a press, else 0.
u32 panelPointer(App* app, ThreadState& ts);
// The game's 0x4bd640: the replay's panel each frame.
u32 panelKeys(App* app, ThreadState& ts);
// The game's 0x4bdb60: player's camera made `camera`, a new shot begun.
u32 tvSetCamera(App* app, ThreadState& ts, u32 player, u32 camera);
// The game's 0x4bdbd0: player's next shot.
u32 tvNextCamera(App* app, ThreadState& ts, u32 player);
// The game's 0x4bdc70: 1 when player's shot has run its length.
u32 tvShotOver(App* app, ThreadState& ts, u32 player);
// The game's 0x4bdcc0: player's shot begins now, as long as camera's are.
u32 tvStartShot(App* app, ThreadState& ts, u32 player, u32 camera);
// The game's 0x4bdd10: the largest of `count` rows of a column of the replay's table; edx and ebx left where they stop.
u32 tvBusiest(App* app, ThreadState& ts, u32 column, u32 row, u32 count);
// The game's 0x4bdd70: the director's choice for player each frame.
u32 tvDirect(App* app, ThreadState& ts, u32 player);
// The game's 0x4be000: al 1 for a digit or a sign, 0 for anything else.
u32 isNumberCharacter(App* app, ThreadState& ts, u32 character);
// The game's 0x4be1a0: an off beat of the clock: 0 or 1.
u32 offBeat(App* app, ThreadState& ts, u32 rate, u32 offset);
// The game's 0x4c9dd0: siren `index` on at a node.
u32 sirenOn(App* app, ThreadState& ts, u32 index, u32 node, u32 side, u32 clock);
// The game's 0x4c9f50: siren `index` off.
u32 sirenOff(App* app, ThreadState& ts, u32 index);
// The game's 0x4ea570: the angle of (x, y) in 16-bit fixed point.
u32 angle(App* app, ThreadState& ts, u32 y, u32 x);
// The game's 0x4ea654: how many entries the pack has.
u32 packCount(App* app, ThreadState& ts, u32 pack);
// The game's 0x4ea658: entry `entry` of the pack, or 0 past its end.
u32 packEntry(App* app, ThreadState& ts, u32 pack, u32 entry);
// The game's 0x43bcc0: where on the car's interior picture its map starts.
u32 interiorMapX(App* app, ThreadState& ts, u32 car);
// The game's 0x465320: the view widget's item placed.
u32 viewLaidOut(App* app, ThreadState& ts, u32 item);
// The game's 0x465360: the view widget's empty entry: eax as it came.
u32 viewNoEntry(App* app, ThreadState& ts, u32 eax);
// The game's 0x465370: the showcase's car's interior picture loaded and the view set back.
u32 viewLoadCar(App* app, ThreadState& ts);

}  // namespace simc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
