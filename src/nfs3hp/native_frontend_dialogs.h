#ifndef NFS3HP_NATIVE_FRONTEND_DIALOGS_H
#define NFS3HP_NATIVE_FRONTEND_DIALOGS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_dialogs.cpp's typed functions (a question, a message and a line to type in
 * opened, their layout, the pointer on a button), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address.  Each calls
 * the game's other functions by address on `ts` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace dialogs
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4446c0: the button highlighted, 0 for the first.
u32 focusedButton(App* app, ThreadState& ts);
// The game's 0x443a90: the box round count lines, in floats at rect; the dialog starts to fade in.
u32 linesBox(App* app, ThreadState& ts, u32 count, u32 lines, u32 rowBelow, u32 fieldBelow, u32 rect);
// The game's 0x443c00: the box round a row of buttons, in floats at rect; the dialog starts to fade in.
u32 buttonRowBox(App* app, ThreadState& ts, u32 count, u32 buttons, u32 rect);
// The game's 0x443d20: the buttons' boxes in the dialog's box at rect, in a column or a row.
u32 placeButtons(App* app, ThreadState& ts, u32 count, u32 buttons, u32 column, u32 rect);
// The game's 0x444570: the pointer moved onto a button's box.
u32 pointAtButton(App* app, ThreadState& ts, u32 index);
// The game's 0x4446d0: a question opened: its lines over its buttons.
u32 openQuestion(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 buttonCount, u32 buttons, u32 buttonHelp,
                   u32 focus, u32 column, u32 callback);
// The game's 0x444a80: a message opened: lines and no buttons, up for time ticks.
u32 openMessage(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 time, u32 keys, u32 callback);
// The game's 0x444be0: a message of one line of the language's text.
u32 messageById(App* app, ThreadState& ts, u32 id);
// The game's 0x444c10: the dialog's own buffer, what was typed.
u32 editText(App* app, ThreadState& ts);
// The game's 0x444c20: a dialog opened to type a line in: its lines over the field.
u32 openEdit(App* app, ThreadState& ts, u32 lineCount, u32 lines, u32 buttonCount, u32 buttons, u32 focus,
               u32 source, u32 maxLength, u32 mode, u32 callback);
// The game's 0x445f00: the dialog shown at once, faded in.
u32 fadedIn(App* app, ThreadState& ts);

}  // namespace dialogs
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
