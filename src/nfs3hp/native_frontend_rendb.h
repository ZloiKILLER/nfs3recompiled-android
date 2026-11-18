#ifndef NFS3HP_NATIVE_FRONTEND_RENDB_H
#define NFS3HP_NATIVE_FRONTEND_RENDB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendb.cpp's typed functions (the race's HUD under what port_hud.cpp draws: its
 * vertices and pictures, the quads of a group, the frame of element 9, the standings' rows, the
 * cop's markers and mark, a box, the road's turn), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace rendb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x47bc20: 1 when an element's kind is drawn as text, 0 for the rest and past the 23.
reg32 kindIsText(App* app, CPU& cpu, reg32 kind);
// The game's 0x47bc40: the HUD's vertices as a race starts, their batches taken from the queues.
reg32 setUpHudVertices(App* app, CPU& cpu);
// The game's 0x47d1d0: a box of four corners in screen numbers, filled or as four lines, in a colour.
reg32 drawBox(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 x3, reg32 y3,
              reg32 filled, reg32 colour);
// The game's 0x47d480: the cop's markers of the view for its car, set or taken out of the queue.
reg32 updateCopMarkers(App* app, CPU& cpu, reg32 view, reg32 car);
// The game's 0x47d580: the mark around the car of the view's row in pursuit, for the screen's width.
reg32 drawPursuitMark(App* app, CPU& cpu, reg32 view, reg32 row, reg32 screenWidth);
// The game's 0x47ee80: the standings' rows, the gaps from the car `from`; edx left as the code leaves it.
reg32 prepareStandings(App* app, CPU& cpu, reg32 from);
// The game's 0x47f090: the look for the HUD's mode, and the road's turn at the car in its matrix.
reg32 turnMatrixToRoad(App* app, CPU& cpu, reg32 car);
// The game's 0x47f2e0: a rectangle of 32-bit pixels from one picture into another, a row at a time.
reg32 copyRectangle(App* app, CPU& cpu, reg32 from, reg32 to, reg32 fromX, reg32 fromY, reg32 toX, reg32 toY,
                    reg32 fromPitch, reg32 toPitch, reg32 width, reg32 height);
// The game's 0x47f340: each view's car's speedometer scale laid into the picture "4444" of `file`.
reg32 layScalesIntoHud(App* app, CPU& cpu, reg32 file);
// The game's 0x47f590: hud98.fsh's pictures made textures, the scales laid in, the cabin's loaded.
reg32 loadHudPictures(App* app, CPU& cpu);
// The game's 0x47fbe0: a group's eight quads, numbered on from `first`, queued on the dials' queue.
reg32 queueEightQuads(App* app, CPU& cpu, reg32 first);
// The game's 0x47fcb0: the same eight quads taken out of it.
reg32 unqueueEightQuads(App* app, CPU& cpu, reg32 first);
// The game's 0x47fd80: the frame of element 9 set, queued, and fitted to the screen.
reg32 fitFrameToScreen(App* app, CPU& cpu);

}  // namespace rendb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
