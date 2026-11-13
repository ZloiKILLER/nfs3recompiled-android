#ifndef NFS3HP_NATIVE_FRONTEND_HUDZ_H
#define NFS3HP_NATIVE_FRONTEND_HUDZ_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudz.cpp's typed functions (the race HUD's layout: the editor's test and drag,
 * the alignments, the vertices over the boxes, the grid's highlight, the replay bar, the queues),
 * which other files call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its result is
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

// The rectangle in eax, the view in edx, the element in ebx, the mode in ecx, where the item in the way goes on
// the stack.
template <>
struct ByAddress<frontend::hudz::elementFits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 tried, x86::reg32 view,
                           x86::reg32 moved, x86::reg32 mode, x86::reg32 blockerOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ tried, view, moved, mode }, { blockerOut }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::hudz::elementMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The view in eax, for each of those of one.
template <>
struct ByAddress<frontend::hudz::unqueueViewQuads>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudz::setAlignments>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// The seat in eax, the form in edx, the corners in ebx.
template <>
struct ByAddress<frontend::hudz::placeDialArt>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 seat, x86::reg32 form,
                           x86::reg32 corners)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ seat, form, corners }));
    }
};

// The view in eax, its width in edx.
template <>
struct ByAddress<frontend::hudz::layMarkersFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 width)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, width }));
    }
};

template <>
struct ByAddress<frontend::hudz::layPanel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudz::layNothing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudz::layReplayBar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudz::layGrid>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::hudz::mapPaneIsSmall>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// The part in eax, the view in edx, the colour in ebx.
template <>
struct ByAddress<frontend::hudz::setGridHighlight>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 part, x86::reg32 view,
                           x86::reg32 colour)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ part, view, colour }));
    }
};

template <>
struct ByAddress<frontend::hudz::drawGridHighlight>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// What a camera past the bar's hands on in eax, a speed's in ebx; edx as the caller has it.
template <>
struct ByAddress<frontend::hudz::drawReplayBar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cameraElse, x86::reg32 speedElse)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cameraElse, cpu.edx, speedElse }));
    }
};

template <>
struct ByAddress<frontend::hudz::flushFrameQueue>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The queue in eax, the texture in edx.
template <>
struct ByAddress<frontend::hudz::queueStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue, x86::reg32 texture)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ queue, texture }));
    }
};

// The queue in eax, the count in edx, where the corners and their number go in ebx and ecx.
template <>
struct ByAddress<frontend::hudz::queueTakeCorners>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue, x86::reg32 count,
                           x86::reg32 cornersOut, x86::reg32 numberOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ queue, count, cornersOut, numberOut }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
