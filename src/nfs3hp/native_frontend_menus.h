#ifndef NFS3HP_NATIVE_FRONTEND_MENUS_H
#define NFS3HP_NATIVE_FRONTEND_MENUS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menus.cpp's typed function (the item lookup), which other files call as C++
 * (guest::direct), and how the game's code calls it by address.  It calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function
 * leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menus
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x442a40: a screen's item by its codelink (name), 0 when it has none.
u32 findItem(App* app, ThreadState& ts, u32 menu, u32 name);

}  // namespace menus
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
