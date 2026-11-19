#ifndef NFS3HP_NATIVE_FRONTEND_MOVA_H
#define NFS3HP_NATIVE_FRONTEND_MOVA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_mova.cpp's typed functions (the movie's display, the integer projection), which
 * other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace mova
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4eba80: the screen's device, as DirectDraw's library keeps it.
u32 screenDevice(App* app, ThreadState& ts);
// The game's 0x4eba90: DirectDraw's object.
u32 directDraw(App* app, ThreadState& ts);
// The game's 0x4df340: the way a movie's frame reaches the screen.
u32 displayMode(App* app, ThreadState& ts);
// The game's 0x4df310: the movie's overlay surface let go, when the screen's device is the one it was made for.
u32 displayClose(App* app, ThreadState& ts);
// The game's 0x4df1f0: a movie's display opened at x, y for a frame of width by height, its quality, software.
u32 displayOpen(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 quality, u32 software);
// The game's 0x4fd280: the integer projection's centre at x, y, its far edges from its half sizes.
u32 projectionCentre(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x4fd2b0: the projection's view: its half angles, the view's width and height.
u32 projectionView(App* app, ThreadState& ts, u32 angleX, u32 angleY, u32 width, u32 height);
// The game's 0x4fd3f0: the projection's view with its half angles in degrees, the width and height handed on.
u32 projectionViewDegrees(App* app, ThreadState& ts, u32 degreesX, u32 degreesY, u32 width, u32 height);

}  // namespace mova
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
