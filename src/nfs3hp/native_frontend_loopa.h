#ifndef NFS3HP_NATIVE_FRONTEND_LOOPA_H
#define NFS3HP_NATIVE_FRONTEND_LOOPA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loopa.cpp's typed functions (the game's start and end, a race's session and its
 * loading, the ghost and its poses, the panel behind the menus' items), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loopa
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x43bcb0: always 1, what the loading screen asks.
reg32 loadingScreenFlag(App* app, CPU& cpu);
// The game's 0x459410: the frame of a menu's item, in the colour `colour`.
reg32 itemFrame(App* app, CPU& cpu, reg32 item, reg32 colour);
// The game's 0x475b10: the 3D font let go.
reg32 freeLoadedBlock(App* app, CPU& cpu);
// The game's 0x4777d0: the ghost's recording written to the file `name`, by one player alone.
reg32 saveGhost(App* app, CPU& cpu, reg32 name);
// The game's 0x477830: a pose's rotation from a matrix.
reg32 poseSetRotation(App* app, CPU& cpu, reg32 pose, reg32 matrix);
// The game's 0x477850: a pose's position into `out` (esi), esi left past the frame's copy.
reg32 posePosition(App* app, CPU& cpu, reg32 pose, reg32 out);
// The game's 0x4778c0: a position (x, y, z floats' bits, the slice's and the car's words) stored in a pose.
reg32 poseSetPosition(App* app, CPU& cpu, reg32 pose, reg32 x, reg32 y, reg32 z, reg32 slice, reg32 car);
// The game's 0x477990: the position and matrix `fraction` of the way between two poses.
reg32 poseBetween(App* app, CPU& cpu, reg32 from, reg32 to, reg32 fraction, reg32 positionOut, reg32 matrixOut);
// The game's 0x4779e0: the recording's pose of an index, or 0 past its end.
reg32 recordedAt(App* app, CPU& cpu, reg32 index);
// The game's 0x477a10: the loaded ghost's pose of an index, past its end the last but one.
reg32 loadedAt(App* app, CPU& cpu, reg32 index);
// The game's 0x477a50: a recording begun with the race's settings, unless this game records none.
reg32 recordingStart(App* app, CPU& cpu);
// The game's 0x477b50: a ghost's file read; 1, or 0 when it is not there, too long or not valid.
reg32 loadGhost(App* app, CPU& cpu, reg32 name);
// The game's 0x477c20: a car's pose recorded at this tick.
reg32 recordCar(App* app, CPU& cpu, reg32 car);
// The game's 0x477d90: the loaded ghost's pose at this tick; 1, or 0 with none loaded.
reg32 playbackPose(App* app, CPU& cpu, reg32 positionOut, reg32 matrixOut);
// The game's 0x495190: the loading bar's four corners scaled by the view's size.
reg32 barScale(App* app, CPU& cpu, reg32 corner0, reg32 corner1, reg32 corner2, reg32 corner3);
// The game's 0x497430: the table of reciprocal roots, a byte each.
reg32 fillRootBytes(App* app, CPU& cpu);
// The game's 0x4a3390: the game's start.
reg32 gameStart(App* app, CPU& cpu);
// The game's 0x4a33a0: the game's end.
reg32 gameEnd(App* app, CPU& cpu);
// The game's 0x4a33b0: what a race's loop does each time it goes round.
reg32 raceRestart(App* app, CPU& cpu);
// The game's 0x4a3400: a race set up and loaded; 1 when it was stopped, else 0.
reg32 raceLoad(App* app, CPU& cpu);
// The game's 0x4a3690: a whole race (`mode` 1 starts it finished, `replay` reads the config again); the menus' answer.
reg32 raceSession(App* app, CPU& cpu, reg32 mode, reg32 replay);
// The game's 0x4a6f70: nothing, called as the game ends.
reg32 nothingToEnd(App* app, CPU& cpu);
// The game's 0x4a7080: a flat quad of one colour, its corners (x, y) and the colour.
reg32 panelQuad(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 x3, reg32 y3,
                reg32 colour);
// The game's 0x4a7310: a quad of a texture, its corners, their places on the texture (floats' bits), the texture
// and the colour.
reg32 panelTexturedQuad(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 x3,
                        reg32 y3, reg32 u0, reg32 v0, reg32 u1, reg32 v1, reg32 u2, reg32 v2, reg32 u3, reg32 v3,
                        reg32 texture, reg32 colour);
// The game's 0x4a75e0: the panel behind the menus' items, sized by the view.
reg32 panelDraw(App* app, CPU& cpu);
// The game's 0x4b6020: the game started: screen, driver, heap, timer, keys, cars, config, sound, network, tables.
reg32 gameInit(App* app, CPU& cpu);
// The game's 0x4b62a0: whether the install's kind is "remote".
reg32 installIsRemote(App* app, CPU& cpu);
// The game's 0x4b6a30: a rotation and a position `fraction` of the way between two (16.16), registers left.
reg32 blend(App* app, CPU& cpu, reg32 rotationFrom, reg32 rotationTo, reg32 positionFrom, reg32 positionTo,
            reg32 fraction, reg32 rotationOut, reg32 positionOut);
// The game's 0x4b6ca0: the matrix of a rotation of four words.
reg32 matrixOfRotation(App* app, CPU& cpu, reg32 rotation, reg32 out);
// The game's 0x4b6d90: the second rotation negated when that brings it nearer the first.
reg32 rotationNearest(App* app, CPU& cpu, reg32 rotation, reg32 other);
// The game's 0x4b6e80: the rotation of a matrix, four words.
reg32 rotationOfMatrix(App* app, CPU& cpu, reg32 matrix, reg32 out);

}  // namespace loopa
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
