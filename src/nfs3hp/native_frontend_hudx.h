#ifndef NFS3HP_NATIVE_FRONTEND_HUDX_H
#define NFS3HP_NATIVE_FRONTEND_HUDX_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudx.cpp's typed functions (the 3D letters' looks, the HUD's records, panes,
 * surface and text), which other files call as C++ (guest::direct), and how the game's code calls
 * each by address.  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudx
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x474d50: the selected look's lights off.
reg32 clearLights(App* app, CPU& cpu);
// The game's 0x477070: the selected look flat, in three colours.
reg32 setFlat(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third);
// The game's 0x477170: the selected look lit from one direction.
reg32 setDirectional(App* app, CPU& cpu, reg32 colour, reg32 direction);
// The game's 0x4771c0: a light added to the selected look: its number, or -2 or -1.
reg32 addLight(App* app, CPU& cpu, reg32 first, reg32 position, reg32 third, reg32 second);
// The game's 0x477470: the selected look's texture.
reg32 setTexture(App* app, CPU& cpu, reg32 texture);
// The game's 0x484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down; eax left as it came.
void clearRebuildFlag(App* app, CPU& cpu);
// The game's 0x484260: a view's HUD on or off, its pane set afresh.
reg32 toggleShown(App* app, CPU& cpu, reg32 view);
// The game's 0x484450: the rear-view mirror on or off, in a race of one view (eax as it came otherwise).
reg32 toggleRearView(App* app, CPU& cpu);
// The same with the eax the game's function was entered with, which it leaves where it does nothing.
reg32 toggleRearViewKeeping(App* app, CPU& cpu, reg32 eax);
// The game's 0x485640: a view's pane, its elements and its band made afresh.
reg32 rebuildView(App* app, CPU& cpu, reg32 view);
// The game's 0x487340: a group of the HUD's rectangles empty.
reg32 groupClearCount(App* app, CPU& cpu, reg32 group);
// The game's 0x4873c0: a group's last rectangle gone, if it has one.
reg32 groupDropLast(App* app, CPU& cpu, reg32 group);
// The game's 0x48ccf0: where a view's map pane is now, and its box's colour.
reg32 refreshPane(App* app, CPU& cpu, reg32 view);
// The game's 0x48eb60: the zoom stepped in the views.
reg32 stepZoom(App* app, CPU& cpu, reg32 step);
// The game's 0x497590 (stdcall): the length of the vector x, y (floats' bits), which its code leaves on the x87's
// stack; eax as it came.
double vectorLength(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x49bef0: a surface empty, its indices in order.
reg32 resetSurface(App* app, CPU& cpu, reg32 surface);
// The game's 0x49bf40: `pairs` pairs of vertices taken from a surface, the first one's address put at `first`.
reg32 takeVertices(App* app, CPU& cpu, reg32 surface, reg32 pairs, reg32 first);
// The game's 0x4d10e0: a text ending at x `right`, at `y`.
reg32 drawTextToRightEdge(App* app, CPU& cpu, reg32 text, reg32 right, reg32 y);
// The game's 0x4d1310: the two fonts' surfaces begun.
reg32 beginFontSurfaces(App* app, CPU& cpu);
// The game's 0x4d14f0: how far a character (its low byte) of the font drops, at the character size.
reg32 characterDrop(App* app, CPU& cpu, reg32 character);

}  // namespace hudx
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
