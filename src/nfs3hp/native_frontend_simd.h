#ifndef NFS3HP_NATIVE_FRONTEND_SIMD_H
#define NFS3HP_NATIVE_FRONTEND_SIMD_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simd.cpp's typed functions (the race cameras' steps, the shared and the 16.16
 * maths, the race's list of tasks, small things), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address.  Each calls
 * the game's other functions by address on `cpu` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax, and what it leaves
 * on the x87's stack and in the registers it does not save stays there.  Stack arguments the
 * function writes over are taken by their address (`words`), as the game's code pushes them. */
namespace nfs3hp
{
namespace frontend
{
namespace simd
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4243a0: camera mode: behind the car.
reg32 cameraChase(App* app, CPU& cpu, reg32 camera);
// The game's 0x424690: camera mode: round the car on the race's clock.
reg32 cameraOrbit(App* app, CPU& cpu, reg32 camera);
// The game's 0x424740: camera mode: behind the car, low.
reg32 cameraChaseLow(App* app, CPU& cpu, reg32 camera);
// The game's 0x424b50: camera mode: circling the car.
reg32 cameraCircleCar(App* app, CPU& cpu, reg32 camera);
// The game's 0x424ce0: camera mode: looking round the car.
reg32 cameraLookAround(App* app, CPU& cpu, reg32 camera);
// The game's 0x424e30: camera mode: at the side of the car.
reg32 cameraSideOfCar(App* app, CPU& cpu, reg32 camera);
// The game's 0x424f30: a camera's turn eased.
reg32 cameraEaseTurn(App* app, CPU& cpu, reg32 camera);
// The game's 0x424fd0: the camera to the spot, the zoom from the distance over the float `near`.
reg32 cameraMoveTowards(App* app, CPU& cpu, reg32 camera, reg32 spot, reg32 near);
// The game's 0x425990: camera mode: held to the car at its offset.
reg32 cameraBesideCar(App* app, CPU& cpu, reg32 camera);
// The game's 0x425ab0: camera mode: held on the car.
reg32 cameraOnCar(App* app, CPU& cpu, reg32 camera);
// The game's 0x425b40: the camera to the spot, the zoom the floats `base` and `rate` at that distance.
reg32 cameraGlideTowards(App* app, CPU& cpu, reg32 camera, reg32 spot, reg32 base, reg32 rate);
// The game's 0x426370: a mode's camera of a view begun.
reg32 cameraPrepare(App* app, CPU& cpu, reg32 camera, reg32 view, reg32 mode);
// The game's 0x4e0090: the length of `to` less `from`, in `length`.
reg32 vectorDistance(App* app, CPU& cpu, reg32 from, reg32 to, double& length);
// The game's 0x4e0110: a matrix turned over, `from` to `to`.
reg32 transpose(App* app, CPU& cpu, reg32 from, reg32 to);
// The game's 0x4e0160: the cross product a by b to `out`.
reg32 cross(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out);
// The game's 0x4e01a0: the cross product made one long, to `out`.
reg32 unitCross(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out);
// The game's 0x4ea930 and 0x4ea950: the sine and the cosine of the float `angle` in the game's units, which they leave on the x87.
double sineValue(App* app, reg32 angle);
double cosineValue(App* app, reg32 angle);
// The game's 0x4ea970: the sine and the cosine of the float at `angleAt`.
reg32 sineCosine(App* app, CPU& cpu, reg32 angleAt, reg32 sineAt, reg32 cosineAt);
// The game's 0x4ea9c0: the angle of two floats in the game's units, which it leaves on the x87 (the runtime's atan2 it calls still runs on the x87's stack).
double angleOfFloatsValue(App* app, CPU& cpu, reg32 first, reg32 second);
// The game's 0x4ea9e0: the matrix turned by the float `angle` about its first row, to `out`.
reg32 rotateX(App* app, CPU& cpu, reg32 matrix, reg32 angle, reg32 out);
// The game's 0x4eaab0: about its second row.
reg32 rotateY(App* app, CPU& cpu, reg32 matrix, reg32 angle, reg32 out);
// The game's 0x4eab80: about its third row.
reg32 rotateZ(App* app, CPU& cpu, reg32 matrix, reg32 angle, reg32 out);
// The game's 0x496a50: a frame from a triangle's three points.
reg32 frameOfTriangle(App* app, CPU& cpu, reg32 points, reg32 out);
// The game's 0x4970c0: a 16.16 vector made one long.
reg32 fixedNormalise(App* app, CPU& cpu, reg32 vector);
// The game's 0x4f44e0: nine 16.16 ints turned over.
reg32 fixedTranspose(App* app, CPU& cpu, reg32 from, reg32 to);
// The game's 0x4f4544: three 16.16 ints by nine, to `out`; edx the last sum's high word.
reg32 fixedMatrixTimes(App* app, CPU& cpu, reg32 a, reg32 m, reg32 out);
// The game's 0x4969c0: the height of a point dropped onto a plane: the origin and the point, six words at `words`, written over.
reg32 trackPointOnPlane(App* app, CPU& cpu, reg32 matrix, reg32 words);
// The game's 0x4c9bc0: the place `lane` lanes across slice `index`, in floats to `out`.
reg32 placeInLane(App* app, CPU& cpu, reg32 index, reg32 lane, reg32 away, reg32 out);
// The game's 0x4be1f0: a list of tasks for `most`.
reg32 taskListNew(App* app, CPU& cpu, reg32 most);
// The game's 0x4be250: a task put on a list.
reg32 taskListAdd(App* app, CPU& cpu, reg32 list, reg32 function, reg32 argument, reg32 priority);
// The game's 0x4bc910: the cameras' step and the HUD's on the race's second list.
reg32 raceAddTasks(App* app, CPU& cpu);
// The game's 0x4bc8e0: out of the race's frame.
reg32 raceLeaveTasks(App* app, CPU& cpu);
// The game's 0x4c45f0: the race opened.
reg32 raceOpen(App* app, CPU& cpu);
// The game's 0x4c4740: the race started again.
reg32 raceRestart(App* app, CPU& cpu);
// The game's 0x4c7bd0: the network's frame.
reg32 netSendPlace(App* app, CPU& cpu, reg32 eax);
// The game's 0x485880: the HUD's panes' defaults.
reg32 hudDefaults(App* app, CPU& cpu);
// The game's 0x439cc0: whether a car is a locked bonus car.
reg32 bonusCarLocked(App* app, CPU& cpu, reg32 car);
// The game's 0x49bfb0: `bits` added to the flags at 0x55e5ec.
reg32 orFlags(App* app, CPU& cpu, reg32 bits);
// The game's 0x4a4020: as the game ends, the web page asked for.
reg32 openWebPage(App* app, CPU& cpu);
// The game's 0x4bebb0: the clip rectangle kept and set.
reg32 setClipRect(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom);
// The game's 0x4c7b10: the depth's offsets set (0x4c7a60), every register handed on.
reg32 depthOffsets(App* app, CPU& cpu);
// The game's 0x4c9d70: the race's block cleared.
reg32 raceClearA(App* app, CPU& cpu);
// The game's 0x4c9db0: the race's block cleared, under another name.
reg32 raceClearB(App* app, CPU& cpu);
// The game's 0x4dbda0: the projection of a view.
reg32 viewProjection(App* app, CPU& cpu, reg32 camera);
// The game's 0x4dcd10: the weather's step (0x4dcdf0), every register handed on.
reg32 weatherStep(App* app, CPU& cpu);
// The game's 0x4dcdd0: how hard it rains.
reg32 rainAmount(App* app, CPU& cpu);

}  // namespace simd
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
