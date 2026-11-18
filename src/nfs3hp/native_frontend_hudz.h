#ifndef NFS3HP_NATIVE_FRONTEND_HUDZ_H
#define NFS3HP_NATIVE_FRONTEND_HUDZ_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudz.cpp's typed functions (the race HUD's layout: the editor's test and drag,
 * the alignments, the vertices over the boxes, the grid's highlight, the replay bar, the queues),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudz
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x459670: whether the HUD editor's element may stand at the rectangle `tried`: 1 or 0, the item in its way
// at `blockerOut`; ebx, ecx and edx left as they fall.
reg32 elementFits(App* app, CPU& cpu, reg32 tried, reg32 view, reg32 moved, reg32 mode, reg32 blockerOut);
// The game's 0x459db0: the item dragged put into the layout, its element made again and tested: 1 when it fits.
reg32 elementMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x4808b0: the quads a view has queued taken back out of the dials' queue.
reg32 unqueueViewQuads(App* app, CPU& cpu, reg32 view);
// The game's 0x480a00: the alignments of a view's texts.
reg32 setAlignments(App* app, CPU& cpu, reg32 view);
// The game's 0x481170: the art of a round dial on its four corners, by the seat's form; edx left the dial's form.
reg32 placeDialArt(App* app, CPU& cpu, reg32 seat, reg32 form, reg32 corners);
// The game's 0x481290: the frame's eight quads round a view's markers (the width the game's code hands it unread).
reg32 layMarkersFrame(App* app, CPU& cpu, reg32 view, reg32 width);
// The game's 0x481510: a view's panel quad over its box.
reg32 layPanel(App* app, CPU& cpu, reg32 view);
// The game's 0x481570: a form of the HUD's laying out with nothing to lay; the view left in eax.
reg32 layNothing(App* app, CPU& cpu, reg32 view);
// The game's 0x4815d0: the replay bar's quads over its box, the first view's only.
reg32 layReplayBar(App* app, CPU& cpu, reg32 view);
// The game's 0x4818b0: the grid's quads over a view's box.
reg32 layGrid(App* app, CPU& cpu, reg32 view);
// The game's 0x481f90: whether a view's map is the small form.
reg32 mapPaneIsSmall(App* app, CPU& cpu, reg32 view);
// The game's 0x482030: the grid's part highlighted in a view, in a colour (0 the part's own).
reg32 setGridHighlight(App* app, CPU& cpu, reg32 part, reg32 view, reg32 colour);
// The game's 0x482190: a view's grid and its highlighted part to the clipper.
reg32 drawGridHighlight(App* app, CPU& cpu, reg32 view);
// The game's 0x482880: the replay bar's lit quads; a camera or a speed past the bar's hands on eax or ebx.
reg32 drawReplayBar(App* app, CPU& cpu, reg32 cameraElse, reg32 speedElse);
// The game's 0x484030: the frames' queue flushed.
reg32 flushFrameQueue(App* app, CPU& cpu);
// The game's 0x49bc80: a queue made ready with its texture; edx left the count of its vertices.
reg32 queueStart(App* app, CPU& cpu, reg32 queue, reg32 texture);
// The game's 0x49bd10: `count` corners taken from a queue, their address at `cornersOut`, their number at `numberOut`.
reg32 queueTakeCorners(App* app, CPU& cpu, reg32 queue, reg32 count, reg32 cornersOut, reg32 numberOut);

}  // namespace hudz
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
