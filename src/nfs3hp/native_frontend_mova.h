#ifndef NFS3HP_NATIVE_FRONTEND_MOVA_H
#define NFS3HP_NATIVE_FRONTEND_MOVA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_mova.cpp's typed functions (the movie's display, the integer projection), which
 * other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace mova
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4eba80: the screen's device, as DirectDraw's library keeps it.
reg32 screenDevice(App* app, CPU& cpu);
// The game's 0x4eba90: DirectDraw's object.
reg32 directDraw(App* app, CPU& cpu);
// The game's 0x4df340: the way a movie's frame reaches the screen.
reg32 displayMode(App* app, CPU& cpu);
// The game's 0x4df310: the movie's overlay surface let go, when the screen's device is the one it was made for.
reg32 displayClose(App* app, CPU& cpu);
// The game's 0x4df1f0: a movie's display opened at x, y for a frame of width by height, its quality, software.
reg32 displayOpen(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 quality, reg32 software);
// The game's 0x4fd280: the integer projection's centre at x, y, its far edges from its half sizes.
reg32 projectionCentre(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x4fd2b0: the projection's view: its half angles, the view's width and height.
reg32 projectionView(App* app, CPU& cpu, reg32 angleX, reg32 angleY, reg32 width, reg32 height);
// The game's 0x4fd3f0: the projection's view with its half angles in degrees, the width and height handed on.
reg32 projectionViewDegrees(App* app, CPU& cpu, reg32 degreesX, reg32 degreesY, reg32 width, reg32 height);

}  // namespace mova
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
