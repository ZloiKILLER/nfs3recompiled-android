#ifndef NFS3HP_NATIVE_FRONTEND_LOOP_H
#define NFS3HP_NATIVE_FRONTEND_LOOP_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_loop.cpp's typed functions (what the menu loop leaves to the main menu's code: a
 * screen's items shown, whether an item is, the shown screen's name), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace loop
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x44a960: every item's handler as the screen (menu) opens, the items not for now greyed out and hidden.
u32 showItems(App* app, ThreadState& ts, u32 menu);
// The game's 0x44a660: whether an item is shown: in the pause screens only one marked for them.
u32 shownNow(App* app, ThreadState& ts, u32 item);
// The game's 0x44ae00: the name of the screen shown, 0 for none.
u32 currentMenuName(App* app, ThreadState& ts);

}  // namespace loop
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
