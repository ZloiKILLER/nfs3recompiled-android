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
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_DIALOGS_NATIVES): the generated code calls them by name, the native
 * whole screens and the menu loop by address.  The adapter takes the
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

/* The adapters, under the game's functions' names. */
void dialogShown(App* app, CPU& cpu);        // 0x4438b0
void dialogKind(App* app, CPU& cpu);         // 0x4438c0
void drawBox(App* app, CPU& cpu);            // 0x4438d0
void drawLines(App* app, CPU& cpu);          // 0x443f40
void drawButtons(App* app, CPU& cpu);        // 0x444000
void drawEditField(App* app, CPU& cpu);      // 0x444350
void buttonUnderMouse(App* app, CPU& cpu);   // 0x4445d0
void questionKey(App* app, CPU& cpu);        // 0x444910
void questionPass(App* app, CPU& cpu);       // 0x4449d0
void endMessage(App* app, CPU& cpu);         // 0x444a60
void messageKey(App* app, CPU& cpu);         // 0x444b20
void messagePass(App* app, CPU& cpu);        // 0x444b60
void editKey(App* app, CPU& cpu);            // 0x444f50
void editPass(App* app, CPU& cpu);           // 0x4450c0
void listFocus(App* app, CPU& cpu);          // 0x445150
void pointAtEntry(App* app, CPU& cpu);       // 0x445160
void entryUnderMouse(App* app, CPU& cpu);    // 0x4451d0
void placeEntries(App* app, CPU& cpu);       // 0x4452d0
void drawList(App* app, CPU& cpu);           // 0x445380
void listKey(App* app, CPU& cpu);            // 0x445520
void listPass(App* app, CPU& cpu);           // 0x445680
void openList(App* app, CPU& cpu);           // 0x445750
void drawDialogHelp(App* app, CPU& cpu);     // 0x445ac0
void loadDialogArt(App* app, CPU& cpu);      // 0x445de0
void dialogKey(App* app, CPU& cpu);          // 0x445e60
void closeDialog(App* app, CPU& cpu);        // 0x445f10
void dialogPass(App* app, CPU& cpu);         // 0x445f50

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `eax`, where it takes one, is what the game's code has in eax at the call (the function gives it back where
 * it leaves eax alone).  A core's `esp` is the frame base its adapter hands it: the game's esp on entry, at the
 * return address.  A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call
 * pushes: the esp the adapter hands it, so the core's frame lies where the game's does; by address the call
 * makes it. */

// The game's 0x4438b0: whether a dialog is up (the function's al; the rest of eax as it came).
uint8_t dialogShownOf(App* app);
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

// Nothing handed over; al.
template <>
struct ByAddress<port::dialogs::dialogShownOf>
{
    template <x86::reg32 Address>
    static std::uint8_t call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return std::uint8_t(cpu.eax);
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::dialogs::endMessageOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// Nothing handed over; the entry in eax.
template <>
struct ByAddress<port::dialogs::listFocusOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// Stdcall: the line count in eax, the lines in edx, the entry count in ebx, the names in ecx; the ticks, the
// entries' help, the button count, the buttons, their help, the focus, the column and the callback pushed
// (ret 0x20); eax back.
template <>
struct ByAddress<port::dialogs::openListOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 lineCount,
                             x86::reg32 lines, std::int32_t entryCount, x86::reg32 names, x86::reg32 ticks,
                             x86::reg32 entryHelp, std::int32_t buttonCount, x86::reg32 buttons,
                             x86::reg32 buttonHelp, std::int32_t focus, x86::reg32 column, x86::reg32 callback)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = lineCount;
        cpu.edx = lines;
        cpu.ebx = x86::reg32(entryCount);
        cpu.ecx = names;
        if (!guest::call(app, cpu, Address,
                         {ticks, entryHelp, x86::reg32(buttonCount), buttons, buttonHelp, x86::reg32(focus), column,
                          callback}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// The key in eax; eax back.
template <>
struct ByAddress<port::dialogs::dialogKeyOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 key)
    {
        return watcomEax(app, cpu, Address, {key});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::dialogs::closeDialogOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::dialogs::dialogPassOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
