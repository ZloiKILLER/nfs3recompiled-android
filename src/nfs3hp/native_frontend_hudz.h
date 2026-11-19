#ifndef NFS3HP_NATIVE_FRONTEND_HUDZ_H
#define NFS3HP_NATIVE_FRONTEND_HUDZ_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudz.cpp's typed functions (the race HUD's layout: the editor's test and drag,
 * the alignments, the vertices over the boxes, the grid's highlight, the replay bar, the queues),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudz
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x459670: whether the HUD editor's element may stand at the rectangle `tried`: 1 or 0, the item in its way
// at `blockerOut`; ebx, ecx and edx left as they fall.
u32 elementFits(App* app, ThreadState& ts, u32 tried, u32 view, u32 moved, u32 mode, u32 blockerOut);
// The game's 0x459db0: the item dragged put into the layout, its element made again and tested: 1 when it fits.
u32 elementMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4808b0: the quads a view has queued taken back out of the dials' queue.
u32 unqueueViewQuads(App* app, ThreadState& ts, u32 view);
// The game's 0x480a00: the alignments of a view's texts.
u32 setAlignments(App* app, ThreadState& ts, u32 view);
// The game's 0x481170: the art of a round dial on its four corners, by the seat's form; edx left the dial's form.
u32 placeDialArt(App* app, ThreadState& ts, u32 seat, u32 form, u32 corners);
// The game's 0x481290: the frame's eight quads round a view's markers (the width the game's code hands it unread).
u32 layMarkersFrame(App* app, ThreadState& ts, u32 view, u32 width);
// The game's 0x481510: a view's panel quad over its box.
u32 layPanel(App* app, ThreadState& ts, u32 view);
// The game's 0x481570: a form of the HUD's laying out with nothing to lay; the view left in eax.
u32 layNothing(App* app, ThreadState& ts, u32 view);
// The game's 0x4815d0: the replay bar's quads over its box, the first view's only.
u32 layReplayBar(App* app, ThreadState& ts, u32 view);
// The game's 0x4818b0: the grid's quads over a view's box.
u32 layGrid(App* app, ThreadState& ts, u32 view);
// The game's 0x481f90: whether a view's map is the small form.
u32 mapPaneIsSmall(App* app, ThreadState& ts, u32 view);
// The game's 0x482030: the grid's part highlighted in a view, in a colour (0 the part's own).
u32 setGridHighlight(App* app, ThreadState& ts, u32 part, u32 view, u32 colour);
// The game's 0x482190: a view's grid and its highlighted part to the clipper.
u32 drawGridHighlight(App* app, ThreadState& ts, u32 view);
// The game's 0x482880: the replay bar's lit quads; a camera or a speed past the bar's hands on eax or ebx.
u32 drawReplayBar(App* app, ThreadState& ts, u32 cameraElse, u32 speedElse);
// The game's 0x484030: the frames' queue flushed.
u32 flushFrameQueue(App* app, ThreadState& ts);
// The game's 0x49bc80: a queue made ready with its texture; edx left the count of its vertices.
u32 queueStart(App* app, ThreadState& ts, u32 queue, u32 texture);
// The game's 0x49bd10: `count` corners taken from a queue, their address at `cornersOut`, their number at `numberOut`.
u32 queueTakeCorners(App* app, ThreadState& ts, u32 queue, u32 count, u32 cornersOut, u32 numberOut);

}  // namespace hudz
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
