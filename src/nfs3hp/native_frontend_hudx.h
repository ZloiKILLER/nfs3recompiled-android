#ifndef NFS3HP_NATIVE_FRONTEND_HUDX_H
#define NFS3HP_NATIVE_FRONTEND_HUDX_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_hudx.cpp's typed functions (the 3D letters' looks, the HUD's records, panes,
 * surface and text), which other files call as C++ (guest::direct), and how the game's code calls
 * each by address.  Each calls the game's other functions by
 * address on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace hudx
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x474d50: the selected look's lights off.
u32 clearLights(App* app, ThreadState& ts);
// The game's 0x477070: the selected look flat, in three colours.
u32 setFlat(App* app, ThreadState& ts, u32 first, u32 second, u32 third);
// The game's 0x477170: the selected look lit from one direction.
u32 setDirectional(App* app, ThreadState& ts, u32 colour, u32 direction);
// The game's 0x4771c0: a light added to the selected look: its number, or -2 or -1.
u32 addLight(App* app, ThreadState& ts, u32 first, u32 position, u32 third, u32 second);
// The game's 0x477470: the selected look's texture.
u32 setTexture(App* app, ThreadState& ts, u32 texture);
// The game's 0x484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down; eax left as it came.
void clearRebuildFlag(App* app, ThreadState& ts);
// The game's 0x484260: a view's HUD on or off, its pane set afresh.
u32 toggleShown(App* app, ThreadState& ts, u32 view);
// The game's 0x484450: the rear-view mirror on or off, in a race of one view (eax as it came otherwise).
u32 toggleRearView(App* app, ThreadState& ts);
// The same with the eax the game's function was entered with, which it leaves where it does nothing.
u32 toggleRearViewKeeping(App* app, ThreadState& ts, u32 eax);
// The game's 0x485640: a view's pane, its elements and its band made afresh.
u32 rebuildView(App* app, ThreadState& ts, u32 view);
// The game's 0x487340: a group of the HUD's rectangles empty.
u32 groupClearCount(App* app, ThreadState& ts, u32 group);
// The game's 0x4873c0: a group's last rectangle gone, if it has one.
u32 groupDropLast(App* app, ThreadState& ts, u32 group);
// The game's 0x48ccf0: where a view's map pane is now, and its box's colour.
u32 refreshPane(App* app, ThreadState& ts, u32 view);
// The game's 0x48eb60: the zoom stepped in the views.
u32 stepZoom(App* app, ThreadState& ts, u32 step);
// The game's 0x497590 (stdcall): the length of the vector x, y (floats' bits), which its code leaves on the x87's
// stack; eax as it came.
double vectorLength(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x49bef0: a surface empty, its indices in order.
u32 resetSurface(App* app, ThreadState& ts, u32 surface);
// The game's 0x49bf40: `pairs` pairs of vertices taken from a surface, the first one's address put at `first`.
u32 takeVertices(App* app, ThreadState& ts, u32 surface, u32 pairs, u32 first);
// The game's 0x4d10e0: a text ending at x `right`, at `y`.
u32 drawTextToRightEdge(App* app, ThreadState& ts, u32 text, u32 right, u32 y);
// The game's 0x4d1310: the two fonts' surfaces begun.
u32 beginFontSurfaces(App* app, ThreadState& ts);
// The game's 0x4d14f0: how far a character (its low byte) of the font drops, at the character size.
u32 characterDrop(App* app, ThreadState& ts, u32 character);

}  // namespace hudx
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
