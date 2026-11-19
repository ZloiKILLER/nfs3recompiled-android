#ifndef NFS3HP_NATIVE_FRONTEND_VIEW_H
#define NFS3HP_NATIVE_FRONTEND_VIEW_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_view.cpp's typed functions (the pan-and-zoom widget's tiles, the interior on
 * them, its state), which other files call as C++ (guest::direct), and how the game's code calls
 * each by address.  Each calls the game's other functions by
 * address on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its
 * result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace view
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x464700: the tiles' records made from folder + fileName, the file freed.
u32 makeTiles(App* app, ThreadState& ts, u32 fileName, u32 folder);
// The game's 0x4647a0: the same records' textures filled from folder + fileName, the file freed.
u32 fillTiles(App* app, ThreadState& ts, u32 fileName, u32 folder);
// The game's 0x464eb0: the car's interior on the tiles, when the car is not the one they hold.
u32 loadInterior(App* app, ThreadState& ts, u32 eax);
// The game's 0x4652f0: the widget's state, its table's +0x14.
u32 widgetState(App* app, ThreadState& ts);

}  // namespace view
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
