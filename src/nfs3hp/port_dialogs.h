#ifndef NFS3HP_PORT_DIALOGS_H
#define NFS3HP_PORT_DIALOGS_H

#include "native_guest.h"
#include "port.h"

/* The dialogs over the screens, rewritten (port.h says how; port_dialogs.cpp):
 * the dialog up and its kind, what it draws -- its box, its lines, its buttons,
 * the field typed in, a list's entries, the help box over it -- the pointer
 * on its buttons and entries, each kind's keys and its pass (a question, a
 * message, a line to type in, a list to tick), a list opened and its entries
 * laid out, and the dialog's own keys and pass, which hand an answer to its
 * callback.
 *
 * The game's code calls them by name, the native
 * whole screens and the menu loop by address.  The thunk takes the
 * arguments from the registers and the stack and gives back the result in eax,
 * every other register as the caller had it.  What opens a question, a message
 * and a line to type in, and lays out their boxes, stays native whole
 * (native_frontend_dialogs.cpp, MENU_NATIVES). */
namespace nfs3hp
{
namespace port
{
namespace dialogs
{

/* The thunks, under the game's functions' names. */
x86::reg32 dialogShown(App* app, CPU& cpu, x86::Args& args);        // 0x4438b0
x86::reg32 dialogKind(App* app, CPU& cpu, x86::Args& args);         // 0x4438c0
x86::reg32 drawBox(App* app, CPU& cpu, x86::Args& args);            // 0x4438d0
x86::reg32 drawLines(App* app, CPU& cpu, x86::Args& args);          // 0x443f40
x86::reg32 drawButtons(App* app, CPU& cpu, x86::Args& args);        // 0x444000
x86::reg32 drawEditField(App* app, CPU& cpu, x86::Args& args);      // 0x444350
x86::reg32 buttonUnderMouse(App* app, CPU& cpu, x86::Args& args);   // 0x4445d0
x86::reg32 questionKey(App* app, CPU& cpu, x86::Args& args);        // 0x444910
x86::reg32 questionPass(App* app, CPU& cpu, x86::Args& args);       // 0x4449d0
x86::reg32 endMessage(App* app, CPU& cpu, x86::Args& args);         // 0x444a60
x86::reg32 messageKey(App* app, CPU& cpu, x86::Args& args);         // 0x444b20
x86::reg32 messagePass(App* app, CPU& cpu, x86::Args& args);        // 0x444b60
x86::reg32 editKey(App* app, CPU& cpu, x86::Args& args);            // 0x444f50
x86::reg32 editPass(App* app, CPU& cpu, x86::Args& args);           // 0x4450c0
x86::reg32 listFocus(App* app, CPU& cpu, x86::Args& args);          // 0x445150
x86::reg32 pointAtEntry(App* app, CPU& cpu, x86::Args& args);       // 0x445160
x86::reg32 entryUnderMouse(App* app, CPU& cpu, x86::Args& args);    // 0x4451d0
x86::reg32 placeEntries(App* app, CPU& cpu, x86::Args& args);       // 0x4452d0
x86::reg32 drawList(App* app, CPU& cpu, x86::Args& args);           // 0x445380
x86::reg32 listKey(App* app, CPU& cpu, x86::Args& args);            // 0x445520
x86::reg32 listPass(App* app, CPU& cpu, x86::Args& args);           // 0x445680
x86::reg32 openList(App* app, CPU& cpu, x86::Args& args);           // 0x445750
x86::reg32 drawDialogHelp(App* app, CPU& cpu, x86::Args& args);     // 0x445ac0
x86::reg32 loadDialogArt(App* app, CPU& cpu, x86::Args& args);      // 0x445de0
x86::reg32 dialogKey(App* app, CPU& cpu, x86::Args& args);          // 0x445e60
x86::reg32 closeDialog(App* app, CPU& cpu, x86::Args& args);        // 0x445f10
x86::reg32 dialogPass(App* app, CPU& cpu, x86::Args& args);         // 0x445f50

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `eax`, where it takes one, is what the game's code has in eax at the call (the function gives it back where
 * it leaves eax alone).  A core's `esp` is the frame base its thunk hands it: the game's esp on entry, at the
 * return address.  A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call
 * pushes: the esp the thunk hands it, so the core's frame lies where the game's does; by address the call
 * makes it. */

// The game's 0x4438b0: whether a dialog is up (the function's al; the rest of eax as it came).
uint8_t dialogShownOf(App* app);
// The same as the whole eax: al the answer, the rest of `eax` as it came.
reg32 dialogShownWith(App* app, reg32 eax);
// The game's 0x444a60: the message to go on the next pass.
Result endMessageOf(App* app, CPU& cpu, reg32 eax);
// The game's 0x445150: the list entry highlighted (-1 for none).
reg32 listFocusOf(App* app);
// The game's 0x445750: a list opened -- its lines, entries, ticks, the entries' help, its buttons, their help, the
// focus, one above the other with `column`, the callback; esp the frame base.
Result openListOf(App* app, CPU& cpu, reg32 esp, reg32 lineCount, reg32 lines, int32_t entryCount, reg32 names,
                  reg32 ticks, reg32 entryHelp, int32_t buttonCount, reg32 buttons, reg32 buttonHelp, int32_t focus,
                  reg32 column, reg32 callback);
// The game's 0x445e60: key `key` for the dialog up; esp the frame base.
Result dialogKeyOf(App* app, CPU& cpu, reg32 esp, reg32 key);
// The game's 0x445f10: the dialog closed (eax's ah cleared).
Result closeDialogOf(App* app, CPU& cpu, reg32 eax);
// The game's 0x445f50: a pass of the dialog up; esp the frame base.
Result dialogPassOf(App* app, CPU& cpu, reg32 esp, reg32 eax);

}  // namespace dialogs
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
