#ifndef NFS3HP_PORT_LOOP_H
#define NFS3HP_PORT_LOOP_H

#include "native_guest.h"
#include "port.h"

/* The menu loop and what it moves through, rewritten (port.h says how;
 * port_loop.cpp): the screen shown and the screens gone through, its items
 * shown, drawn and passed, the selection moved by the keys and the mouse, a
 * key handed to the item that has it, the race settings kept consistent, the
 * help box, the loop itself, and the answers of the dialogs it opens.
 *
 * Each is a thunk under the game's name, which the native
 * whole screens (native_frontend_*.cpp) and the widgets call by address.  The
 * thunk takes the arguments from the registers and the stack and gives back
 * the result in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
namespace loop
{

/* The thunks, under the game's functions' names. */
nfs::u32 itemUsable(App* app, ThreadState& ts, nfs::Args& args);          // 0x4499b0
nfs::u32 keyAllowed(App* app, ThreadState& ts, nfs::Args& args);          // 0x4499f0
nfs::u32 drawHelp(App* app, ThreadState& ts, nfs::Args& args);            // 0x449aa0
nfs::u32 keysTaken(App* app, ThreadState& ts, nfs::Args& args);           // 0x449df0
nfs::u32 selectable(App* app, ThreadState& ts, nfs::Args& args);          // 0x449e50
nfs::u32 pointAtSelected(App* app, ThreadState& ts, nfs::Args& args);     // 0x449ea0
nfs::u32 selectPrevious(App* app, ThreadState& ts, nfs::Args& args);      // 0x449f30
nfs::u32 selectNext(App* app, ThreadState& ts, nfs::Args& args);          // 0x44a0c0
nfs::u32 escapeButton(App* app, ThreadState& ts, nfs::Args& args);        // 0x44a240
nfs::u32 handleKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x44a2d0
nfs::u32 selectedIndex(App* app, ThreadState& ts, nfs::Args& args);       // 0x44a440
nfs::u32 selectItem(App* app, ThreadState& ts, nfs::Args& args);          // 0x44a470
nfs::u32 drawItems(App* app, ThreadState& ts, nfs::Args& args);           // 0x44a4f0
nfs::u32 releaseMouse(App* app, ThreadState& ts, nfs::Args& args);        // 0x44a690
nfs::u32 itemUnderMouse(App* app, ThreadState& ts, nfs::Args& args);      // 0x44a6b0
nfs::u32 itemsShown(App* app, ThreadState& ts, nfs::Args& args);          // 0x44a860
nfs::u32 passItems(App* app, ThreadState& ts, nfs::Args& args);           // 0x44a8b0
nfs::u32 hideItems(App* app, ThreadState& ts, nfs::Args& args);           // 0x44a9e0
nfs::u32 leaveScreen(App* app, ThreadState& ts, nfs::Args& args);         // 0x44aa30
nfs::u32 resetItems(App* app, ThreadState& ts, nfs::Args& args);          // 0x44aad0
nfs::u32 enterScreen(App* app, ThreadState& ts, nfs::Args& args);         // 0x44ab40
nfs::u32 menuByName(App* app, ThreadState& ts, nfs::Args& args);          // 0x44ae60
nfs::u32 currentMenu(App* app, ThreadState& ts, nfs::Args& args);         // 0x44aeb0
nfs::u32 popMenu(App* app, ThreadState& ts, nfs::Args& args);             // 0x44aec0
nfs::u32 pushMenu(App* app, ThreadState& ts, nfs::Args& args);            // 0x44af10
nfs::u32 setHistory(App* app, ThreadState& ts, nfs::Args& args);          // 0x44af50
nfs::u32 player1NameAnswer(App* app, ThreadState& ts, nfs::Args& args);   // 0x44af90
nfs::u32 player2NameAnswer(App* app, ThreadState& ts, nfs::Args& args);   // 0x44b040
nfs::u32 quitAnswer(App* app, ThreadState& ts, nfs::Args& args);          // 0x44b0f0
nfs::u32 waitWhileSuspended(App* app, ThreadState& ts, nfs::Args& args);  // 0x44b1a0
nfs::u32 menuLoop(App* app, ThreadState& ts, nfs::Args& args);            // 0x44b230
nfs::u32 fitRaceSettings(App* app, ThreadState& ts, nfs::Args& args);     // 0x44be60

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `eax`, where it takes one, is what the game's code has in eax at the call (the function gives it back where
 * it leaves eax alone).  A core's `esp` is the frame base its thunk hands it: the game's esp on entry, at the
 * return address.  A caller through guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call
 * pushes: the esp the thunk hands it, so the core's frame lies where the game's does; by address the call
 * makes it. */

// The game's 0x449df0: whether the keys go to something other than the screen; esp the frame base.
Result keysTakenOf(App* app, ThreadState& ts, u32 esp, u32 eax);
// The game's 0x449e50: whether item `offset` of the screen at `menu` can be selected; esp the frame base.
Result selectableOf(App* app, ThreadState& ts, u32 esp, int32_t offset, u32 menu);
// The game's 0x449ea0: the pointer moved onto the selected item of the screen at `menu`; esp the frame base.
Result pointAtSelectedOf(App* app, ThreadState& ts, u32 esp, u32 menu);
// The game's 0x44a240: the Escape button of the screen at `menu`, its index to `offset`; esp the frame base.
Result escapeButtonOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 offset);
// The game's 0x44a2d0: key `key` on the screen at `menu`; esp the frame base.
Result handleKeyOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 key);
// The game's 0x44a4f0: the items of the screen at `menu` drawn; esp the frame base.
Result drawItemsOf(App* app, ThreadState& ts, u32 esp, u32 menu);
// The game's 0x44a6b0: the item of the screen at `menu` under the mouse; esp the frame base.
Result itemUnderMouseOf(App* app, ThreadState& ts, u32 esp, u32 menu);
// The game's 0x44a8b0: a pass of the items of the screen at `menu`; esp the frame base.
Result passItemsOf(App* app, ThreadState& ts, u32 esp, u32 menu);
// The game's 0x44aa30: the screen at `menu` left, how by `action`; esp the frame base.
Result leaveScreenOf(App* app, ThreadState& ts, u32 esp, u32 menu, u32 action);
// The game's 0x44ab40: the screen at `menu` opened; esp the frame base.
Result enterScreenOf(App* app, ThreadState& ts, u32 esp, u32 menu);
// The game's 0x44aeb0: the screen shown.
u32 currentMenuOf(App* app);
// The game's 0x44aec0: back to the screen gone from last.
Result popMenuOf(App* app, ThreadState& ts, u32 eax);
// The game's 0x44af10: the screen shown onto the history.
Result pushMenuOf(App* app, ThreadState& ts, u32 eax);
// The game's 0x44b230: the menus from the screen named at `name`, `history` the history; esp the frame base.
Result menuLoopOf(App* app, ThreadState& ts, u32 esp, u32 name, u32 history);

}  // namespace loop
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
