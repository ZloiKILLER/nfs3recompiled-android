#ifndef NFS3HP_NATIVE_FRONTEND_SCREEN_H
#define NFS3HP_NATIVE_FRONTEND_SCREEN_H

#include "native_guest.h"

/* native_frontend_screen.cpp's typed functions (the pointer put where the keys moved the
 * selection, the screens' art loaded), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in and runs on `ts` as the code does, eax and the
 * registers it does not restore left as the code leaves them.  When the game stopped in a call
 * they return at once with ts.terminate set, as the code does -- they do not throw -- and so do
 * their calls by address here. */
namespace nfs3hp
{
namespace frontend
{
namespace screen
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x435230: the pointer put at x, y, inside its bounds, what the mouse has buffered taken first.
u32 placePointer(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x49bfc0: the menus' pointer to x, y on a 640 by 480 screen, and the mouse's to there.
u32 setPointer(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x440970: the screens' art for the one shown, the items' buffers made anew.
u32 loadScreenArt(App* app, ThreadState& ts);

}  // namespace screen
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
