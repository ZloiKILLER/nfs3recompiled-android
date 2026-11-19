#ifndef NFS3HP_NATIVE_FRONTEND_RENDB_H
#define NFS3HP_NATIVE_FRONTEND_RENDB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendb.cpp's typed functions (the race's HUD under what port_hud.cpp draws: its
 * vertices and pictures, the quads of a group, the frame of element 9, the standings' rows, the
 * cop's markers and mark, a box, the road's turn), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace rendb
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x47bc20: 1 when an element's kind is drawn as text, 0 for the rest and past the 23.
u32 kindIsText(App* app, ThreadState& ts, u32 kind);
// The game's 0x47bc40: the HUD's vertices as a race starts, their batches taken from the queues.
u32 setUpHudVertices(App* app, ThreadState& ts);
// The game's 0x47d1d0: a box of four corners in screen numbers, filled or as four lines, in a colour.
u32 drawBox(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3,
              u32 filled, u32 colour);
// The game's 0x47d480: the cop's markers of the view for its car, set or taken out of the queue.
u32 updateCopMarkers(App* app, ThreadState& ts, u32 view, u32 car);
// The game's 0x47d580: the mark around the car of the view's row in pursuit, for the screen's width.
u32 drawPursuitMark(App* app, ThreadState& ts, u32 view, u32 row, u32 screenWidth);
// The game's 0x47ee80: the standings' rows, the gaps from the car `from`; edx left as the code leaves it.
u32 prepareStandings(App* app, ThreadState& ts, u32 from);
// The game's 0x47f090: the look for the HUD's mode, and the road's turn at the car in its matrix.
u32 turnMatrixToRoad(App* app, ThreadState& ts, u32 car);
// The game's 0x47f2e0: a rectangle of 32-bit pixels from one picture into another, a row at a time.
u32 copyRectangle(App* app, ThreadState& ts, u32 from, u32 to, u32 fromX, u32 fromY, u32 toX, u32 toY,
                    u32 fromPitch, u32 toPitch, u32 width, u32 height);
// The game's 0x47f340: each view's car's speedometer scale laid into the picture "4444" of `file`.
u32 layScalesIntoHud(App* app, ThreadState& ts, u32 file);
// The game's 0x47f590: hud98.fsh's pictures made textures, the scales laid in, the cabin's loaded.
u32 loadHudPictures(App* app, ThreadState& ts);
// The game's 0x47fbe0: a group's eight quads, numbered on from `first`, queued on the dials' queue.
u32 queueEightQuads(App* app, ThreadState& ts, u32 first);
// The game's 0x47fcb0: the same eight quads taken out of it.
u32 unqueueEightQuads(App* app, ThreadState& ts, u32 first);
// The game's 0x47fd80: the frame of element 9 set, queued, and fitted to the screen.
u32 fitFrameToScreen(App* app, ThreadState& ts);

}  // namespace rendb
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
