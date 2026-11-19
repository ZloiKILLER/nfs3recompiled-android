#ifndef NFS3HP_NATIVE_FRONTEND_LOOPA_H
#define NFS3HP_NATIVE_FRONTEND_LOOPA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loopa.cpp's typed functions (the game's start and end, a race's session and its
 * loading, the ghost and its poses, the panel behind the menus' items), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loopa
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x43bcb0: always 1, what the loading screen asks.
u32 loadingScreenFlag(App* app, ThreadState& ts);
// The game's 0x459410: the frame of a menu's item, in the colour `colour`.
u32 itemFrame(App* app, ThreadState& ts, u32 item, u32 colour);
// The game's 0x475b10: the 3D font let go.
u32 freeLoadedBlock(App* app, ThreadState& ts);
// The game's 0x4777d0: the ghost's recording written to the file `name`, by one player alone.
u32 saveGhost(App* app, ThreadState& ts, u32 name);
// The game's 0x477830: a pose's rotation from a matrix.
u32 poseSetRotation(App* app, ThreadState& ts, u32 pose, u32 matrix);
// The game's 0x477850: a pose's position into `out` (esi), esi left past the frame's copy.
u32 posePosition(App* app, ThreadState& ts, u32 pose, u32 out);
// The game's 0x4778c0: a position (x, y, z floats' bits, the slice's and the car's words) stored in a pose.
u32 poseSetPosition(App* app, ThreadState& ts, u32 pose, u32 x, u32 y, u32 z, u32 slice, u32 car);
// The game's 0x477990: the position and matrix `fraction` of the way between two poses.
u32 poseBetween(App* app, ThreadState& ts, u32 from, u32 to, u32 fraction, u32 positionOut, u32 matrixOut);
// The game's 0x4779e0: the recording's pose of an index, or 0 past its end.
u32 recordedAt(App* app, ThreadState& ts, u32 index);
// The game's 0x477a10: the loaded ghost's pose of an index, past its end the last but one.
u32 loadedAt(App* app, ThreadState& ts, u32 index);
// The game's 0x477a50: a recording begun with the race's settings, unless this game records none.
u32 recordingStart(App* app, ThreadState& ts);
// The game's 0x477b50: a ghost's file read; 1, or 0 when it is not there, too long or not valid.
u32 loadGhost(App* app, ThreadState& ts, u32 name);
// The game's 0x477c20: a car's pose recorded at this tick.
u32 recordCar(App* app, ThreadState& ts, u32 car);
// The game's 0x477d90: the loaded ghost's pose at this tick; 1, or 0 with none loaded.
u32 playbackPose(App* app, ThreadState& ts, u32 positionOut, u32 matrixOut);
// The game's 0x495190: the loading bar's four corners scaled by the view's size.
u32 barScale(App* app, ThreadState& ts, u32 corner0, u32 corner1, u32 corner2, u32 corner3);
// The game's 0x4a3390: the game's start.
u32 gameStart(App* app, ThreadState& ts);
// The game's 0x4a33a0: the game's end.
u32 gameEnd(App* app, ThreadState& ts);
// The game's 0x4a33b0: what a race's loop does each time it goes round.
u32 raceRestart(App* app, ThreadState& ts);
// The game's 0x4a3400: a race set up and loaded; 1 when it was stopped, else 0.
u32 raceLoad(App* app, ThreadState& ts);
// The game's 0x4a3690: a whole race (`mode` 1 starts it finished, `replay` reads the config again); the menus' answer.
u32 raceSession(App* app, ThreadState& ts, u32 mode, u32 replay);
// The game's 0x4a6f70: nothing, called as the game ends.
u32 nothingToEnd(App* app, ThreadState& ts);
// The game's 0x4a7080: a flat quad of one colour, its corners (x, y) and the colour.
u32 panelQuad(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3,
                u32 colour);
// The game's 0x4a7310: a quad of a texture, its corners, their places on the texture (floats' bits), the texture
// and the colour.
u32 panelTexturedQuad(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 x3,
                        u32 y3, u32 u0, u32 v0, u32 u1, u32 v1, u32 u2, u32 v2, u32 u3, u32 v3,
                        u32 texture, u32 colour);
// The game's 0x4a75e0: the panel behind the menus' items, sized by the view.
u32 panelDraw(App* app, ThreadState& ts);
// The game's 0x4b6020: the game started: screen, driver, heap, timer, keys, cars, config, sound, network, tables.
u32 gameInit(App* app, ThreadState& ts);
// The game's 0x4b62a0: whether the install's kind is "remote".
u32 installIsRemote(App* app, ThreadState& ts);
// The game's 0x4b6a30: a rotation and a position `fraction` of the way between two (16.16), registers left.
u32 blend(App* app, ThreadState& ts, u32 rotationFrom, u32 rotationTo, u32 positionFrom, u32 positionTo,
            u32 fraction, u32 rotationOut, u32 positionOut);
// The game's 0x4b6ca0: the matrix of a rotation of four words.
u32 matrixOfRotation(App* app, ThreadState& ts, u32 rotation, u32 out);
// The game's 0x4b6d90: the second rotation negated when that brings it nearer the first.
u32 rotationNearest(App* app, ThreadState& ts, u32 rotation, u32 other);
// The game's 0x4b6e80: the rotation of a matrix, four words.
u32 rotationOfMatrix(App* app, ThreadState& ts, u32 matrix, u32 out);

}  // namespace loopa
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
