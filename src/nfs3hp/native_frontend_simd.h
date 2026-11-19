#ifndef NFS3HP_NATIVE_FRONTEND_SIMD_H
#define NFS3HP_NATIVE_FRONTEND_SIMD_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simd.cpp's typed functions (the race cameras' steps, the shared and the 16.16
 * maths, the race's list of tasks, small things), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address.  Each calls
 * the game's other functions by address on `ts` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax, and what it leaves
 * on the x87's stack and in the registers it does not save stays there.  Stack arguments the
 * function writes over are taken by their address (`words`), as the game's code pushes them. */
namespace nfs3hp
{
namespace frontend
{
namespace simd
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4243a0: camera mode: behind the car.
u32 cameraChase(App* app, ThreadState& ts, u32 camera);
// The game's 0x424690: camera mode: round the car on the race's clock.
u32 cameraOrbit(App* app, ThreadState& ts, u32 camera);
// The game's 0x424740: camera mode: behind the car, low.
u32 cameraChaseLow(App* app, ThreadState& ts, u32 camera);
// The game's 0x424b50: camera mode: circling the car.
u32 cameraCircleCar(App* app, ThreadState& ts, u32 camera);
// The game's 0x424ce0: camera mode: looking round the car.
u32 cameraLookAround(App* app, ThreadState& ts, u32 camera);
// The game's 0x424e30: camera mode: at the side of the car.
u32 cameraSideOfCar(App* app, ThreadState& ts, u32 camera);
// The game's 0x424f30: a camera's turn eased.
u32 cameraEaseTurn(App* app, ThreadState& ts, u32 camera);
// The game's 0x424fd0: the camera to the spot, the zoom from the distance over the float `near`.
u32 cameraMoveTowards(App* app, ThreadState& ts, u32 camera, u32 spot, u32 near);
// The game's 0x425990: camera mode: held to the car at its offset.
u32 cameraBesideCar(App* app, ThreadState& ts, u32 camera);
// The game's 0x425ab0: camera mode: held on the car.
u32 cameraOnCar(App* app, ThreadState& ts, u32 camera);
// The game's 0x425b40: the camera to the spot, the zoom the floats `base` and `rate` at that distance.
u32 cameraGlideTowards(App* app, ThreadState& ts, u32 camera, u32 spot, u32 base, u32 rate);
// The game's 0x426370: a mode's camera of a view begun.
u32 cameraPrepare(App* app, ThreadState& ts, u32 camera, u32 view, u32 mode);
// The game's 0x4e0090: the length of `to` less `from`, in `length`.
u32 vectorDistance(App* app, ThreadState& ts, u32 from, u32 to, double& length);
// The game's 0x4e0110: a matrix turned over, `from` to `to`.
u32 transpose(App* app, ThreadState& ts, u32 from, u32 to);
// The game's 0x4e0160: the cross product a by b to `out`.
u32 cross(App* app, ThreadState& ts, u32 a, u32 b, u32 out);
// The game's 0x4e01a0: the cross product made one long, to `out`.
u32 unitCross(App* app, ThreadState& ts, u32 a, u32 b, u32 out);
// The game's 0x4ea930 and 0x4ea950: the sine and the cosine of the float `angle` in the game's units, which they leave on the x87.
double sineValue(App* app, u32 angle);
double cosineValue(App* app, u32 angle);
// The game's 0x4ea970: the sine and the cosine of the float at `angleAt`.
u32 sineCosine(App* app, ThreadState& ts, u32 angleAt, u32 sineAt, u32 cosineAt);
// The game's 0x4ea9c0: the angle of two floats in the game's units, which it leaves on the x87 (the runtime's atan2 it calls still runs on the x87's stack).
double angleOfFloatsValue(App* app, ThreadState& ts, u32 first, u32 second);
// The game's 0x4ea9e0: the matrix turned by the float `angle` about its first row, to `out`.
u32 rotateX(App* app, ThreadState& ts, u32 matrix, u32 angle, u32 out);
// The game's 0x4eaab0: about its second row.
u32 rotateY(App* app, ThreadState& ts, u32 matrix, u32 angle, u32 out);
// The game's 0x4eab80: about its third row.
u32 rotateZ(App* app, ThreadState& ts, u32 matrix, u32 angle, u32 out);
// The game's 0x496a50: a frame from a triangle's three points.
u32 frameOfTriangle(App* app, ThreadState& ts, u32 points, u32 out);
// The game's 0x4970c0: a 16.16 vector made one long.
u32 fixedNormalise(App* app, ThreadState& ts, u32 vector);
// The game's 0x4f44e0: nine 16.16 ints turned over.
u32 fixedTranspose(App* app, ThreadState& ts, u32 from, u32 to);
// The game's 0x4f4544: three 16.16 ints by nine, to `out`; edx the last sum's high word.
u32 fixedMatrixTimes(App* app, ThreadState& ts, u32 a, u32 m, u32 out);
// The game's 0x4969c0: the height of a point dropped onto a plane: the origin and the point, six words at `words`, written over.
u32 trackPointOnPlane(App* app, ThreadState& ts, u32 matrix, u32 words);
// The game's 0x4c9bc0: the place `lane` lanes across slice `index`, in floats to `out`.
u32 placeInLane(App* app, ThreadState& ts, u32 index, u32 lane, u32 away, u32 out);
// The game's 0x4be1f0: a list of tasks for `most`.
u32 taskListNew(App* app, ThreadState& ts, u32 most);
// The game's 0x4be250: a task put on a list.
u32 taskListAdd(App* app, ThreadState& ts, u32 list, u32 function, u32 argument, u32 priority);
// The game's 0x4bc910: the cameras' step and the HUD's on the race's second list.
u32 raceAddTasks(App* app, ThreadState& ts);
// The game's 0x4bc8e0: out of the race's frame.
u32 raceLeaveTasks(App* app, ThreadState& ts);
// The game's 0x4c45f0: the race opened.
u32 raceOpen(App* app, ThreadState& ts);
// The game's 0x4c4740: the race started again.
u32 raceRestart(App* app, ThreadState& ts);
// The game's 0x4c7bd0: the network's frame.
u32 netSendPlace(App* app, ThreadState& ts, u32 eax);
// The game's 0x485880: the HUD's panes' defaults.
u32 hudDefaults(App* app, ThreadState& ts);
// The game's 0x439cc0: whether a car is a locked bonus car.
u32 bonusCarLocked(App* app, ThreadState& ts, u32 car);
// The game's 0x49bfb0: `bits` added to the flags at 0x55e5ec.
u32 orFlags(App* app, ThreadState& ts, u32 bits);
// The game's 0x4a4020: as the game ends, the web page asked for.
u32 openWebPage(App* app, ThreadState& ts);
// The game's 0x4bebb0: the clip rectangle kept and set.
u32 setClipRect(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom);
// The game's 0x4c7b10: the depth's offsets set (0x4c7a60), every register handed on.
u32 depthOffsets(App* app, ThreadState& ts);
// The game's 0x4c9d70: the race's block cleared.
u32 raceClearA(App* app, ThreadState& ts);
// The game's 0x4c9db0: the race's block cleared, under another name.
u32 raceClearB(App* app, ThreadState& ts);
// The game's 0x4dbda0: the projection of a view.
u32 viewProjection(App* app, ThreadState& ts, u32 camera);
// The game's 0x4dcd10: the weather's step (0x4dcdf0), every register handed on.
u32 weatherStep(App* app, ThreadState& ts);
// The game's 0x4dcdd0: how hard it rains.
u32 rainAmount(App* app, ThreadState& ts);

}  // namespace simd
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
