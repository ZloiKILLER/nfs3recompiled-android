#ifndef NFS3HP_NATIVE_FRONTEND_DIALOGS_H
#define NFS3HP_NATIVE_FRONTEND_DIALOGS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_dialogs.cpp's typed functions (a question, a message and a line to type in
 * opened, their layout, the pointer on a button), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address (ByAddress, for the checks' stand-ins).  Each calls
 * the game's other functions by address on `cpu` and throws fe::Terminated when the game stopped in
 * one, as Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace dialogs
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4446c0: the button highlighted, 0 for the first.
reg32 focusedButton(App* app, CPU& cpu);
// The game's 0x443a90: the box round count lines, in floats at rect; the dialog starts to fade in.
reg32 linesBox(App* app, CPU& cpu, reg32 count, reg32 lines, reg32 rowBelow, reg32 fieldBelow, reg32 rect);
// The game's 0x443c00: the box round a row of buttons, in floats at rect; the dialog starts to fade in.
reg32 buttonRowBox(App* app, CPU& cpu, reg32 count, reg32 buttons, reg32 rect);
// The game's 0x443d20: the buttons' boxes in the dialog's box at rect, in a column or a row.
reg32 placeButtons(App* app, CPU& cpu, reg32 count, reg32 buttons, reg32 column, reg32 rect);
// The game's 0x444570: the pointer moved onto a button's box.
reg32 pointAtButton(App* app, CPU& cpu, reg32 index);
// The game's 0x4446d0: a question opened: its lines over its buttons.
reg32 openQuestion(App* app, CPU& cpu, reg32 lineCount, reg32 lines, reg32 buttonCount, reg32 buttons, reg32 buttonHelp,
                   reg32 focus, reg32 column, reg32 callback);
// The game's 0x444a80: a message opened: lines and no buttons, up for time ticks.
reg32 openMessage(App* app, CPU& cpu, reg32 lineCount, reg32 lines, reg32 time, reg32 keys, reg32 callback);
// The game's 0x444be0: a message of one line of the language's text.
reg32 messageById(App* app, CPU& cpu, reg32 id);
// The game's 0x444c10: the dialog's own buffer, what was typed.
reg32 editText(App* app, CPU& cpu);
// The game's 0x444c20: a dialog opened to type a line in: its lines over the field.
reg32 openEdit(App* app, CPU& cpu, reg32 lineCount, reg32 lines, reg32 buttonCount, reg32 buttons, reg32 focus,
               reg32 source, reg32 maxLength, reg32 mode, reg32 callback);
// The game's 0x445f00: the dialog shown at once, faded in.
reg32 fadedIn(App* app, CPU& cpu);

}  // namespace dialogs
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::dialogs::focusedButton>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The count in eax, the lines in edx, a row of buttons below in ebx, a field in ecx; the rect on the stack.
template <>
struct ByAddress<frontend::dialogs::linesBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 lines,
                           x86::reg32 rowBelow, x86::reg32 fieldBelow, x86::reg32 rect)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, lines, rowBelow, fieldBelow }, { rect }));
    }
};

// The count in eax, the buttons in edx, the rect in ebx.
template <>
struct ByAddress<frontend::dialogs::buttonRowBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 buttons,
                           x86::reg32 rect)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, buttons, rect }));
    }
};

// The count in eax, the buttons in edx, a column in ebx, the rect in ecx.
template <>
struct ByAddress<frontend::dialogs::placeButtons>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 buttons,
                           x86::reg32 column, x86::reg32 rect)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, buttons, column, rect }));
    }
};

// The button in eax.
template <>
struct ByAddress<frontend::dialogs::pointAtButton>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// The lines' count and lines in eax and edx, the buttons' in ebx and ecx; the rest on the stack.
template <>
struct ByAddress<frontend::dialogs::openQuestion>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lineCount, x86::reg32 lines,
                           x86::reg32 buttonCount, x86::reg32 buttons, x86::reg32 buttonHelp, x86::reg32 focus,
                           x86::reg32 column, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ lineCount, lines, buttonCount, buttons },
                                          { buttonHelp, focus, column, callback }));
    }
};

// The count, the lines, the time, the keys (cl) in eax, edx, ebx, ecx; the callback on the stack.
template <>
struct ByAddress<frontend::dialogs::openMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lineCount, x86::reg32 lines,
                           x86::reg32 time, x86::reg32 keys, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ lineCount, lines, time, keys }, { callback }));
    }
};

// The text's number in eax.
template <>
struct ByAddress<frontend::dialogs::messageById>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

template <>
struct ByAddress<frontend::dialogs::editText>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The lines' count and lines in eax and edx, the buttons' in ebx and ecx; the rest on the stack.
template <>
struct ByAddress<frontend::dialogs::openEdit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lineCount, x86::reg32 lines,
                           x86::reg32 buttonCount, x86::reg32 buttons, x86::reg32 focus, x86::reg32 source,
                           x86::reg32 maxLength, x86::reg32 mode, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ lineCount, lines, buttonCount, buttons },
                                          { focus, source, maxLength, mode, callback }));
    }
};

template <>
struct ByAddress<frontend::dialogs::fadedIn>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
