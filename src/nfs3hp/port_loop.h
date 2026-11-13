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
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_LOOP_NATIVES): the generated code calls them by name, and the native
 * whole screens (native_frontend_*.cpp) and the widgets by address.  The
 * adapter takes the arguments from the registers and the stack and gives back
 * the result in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
namespace loop
{

/* The adapters, under the game's functions' names. */
void itemUsable(App* app, CPU& cpu);          // 0x4499b0
void keyAllowed(App* app, CPU& cpu);          // 0x4499f0
void drawHelp(App* app, CPU& cpu);            // 0x449aa0
void keysTaken(App* app, CPU& cpu);           // 0x449df0
void selectable(App* app, CPU& cpu);          // 0x449e50
void pointAtSelected(App* app, CPU& cpu);     // 0x449ea0
void selectPrevious(App* app, CPU& cpu);      // 0x449f30
void selectNext(App* app, CPU& cpu);          // 0x44a0c0
void escapeButton(App* app, CPU& cpu);        // 0x44a240
void handleKey(App* app, CPU& cpu);           // 0x44a2d0
void selectedIndex(App* app, CPU& cpu);       // 0x44a440
void selectItem(App* app, CPU& cpu);          // 0x44a470
void drawItems(App* app, CPU& cpu);           // 0x44a4f0
void releaseMouse(App* app, CPU& cpu);        // 0x44a690
void itemUnderMouse(App* app, CPU& cpu);      // 0x44a6b0
void itemsShown(App* app, CPU& cpu);          // 0x44a860
void passItems(App* app, CPU& cpu);           // 0x44a8b0
void hideItems(App* app, CPU& cpu);           // 0x44a9e0
void leaveScreen(App* app, CPU& cpu);         // 0x44aa30
void resetItems(App* app, CPU& cpu);          // 0x44aad0
void enterScreen(App* app, CPU& cpu);         // 0x44ab40
void menuByName(App* app, CPU& cpu);          // 0x44ae60
void currentMenu(App* app, CPU& cpu);         // 0x44aeb0
void popMenu(App* app, CPU& cpu);             // 0x44aec0
void pushMenu(App* app, CPU& cpu);            // 0x44af10
void setHistory(App* app, CPU& cpu);          // 0x44af50
void player1NameAnswer(App* app, CPU& cpu);   // 0x44af90
void player2NameAnswer(App* app, CPU& cpu);   // 0x44b040
void quitAnswer(App* app, CPU& cpu);          // 0x44b0f0
void waitWhileSuspended(App* app, CPU& cpu);  // 0x44b1a0
void menuLoop(App* app, CPU& cpu);            // 0x44b230
void fitRaceSettings(App* app, CPU& cpu);     // 0x44be60

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `eax`, where it takes one, is what the game's code has in eax at the call (the function gives it back where
 * it leaves eax alone).  A core's `esp` is the frame base its adapter hands it: the game's esp on entry, at the
 * return address.  A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call
 * pushes: the esp the adapter hands it, so the core's frame lies where the game's does; by address the call
 * makes it. */

// The game's 0x449df0: whether the keys go to something other than the screen; esp the frame base.
Result keysTakenOf(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x449e50: whether item `offset` of the screen at `menu` can be selected; esp the frame base.
Result selectableOf(App* app, CPU& cpu, reg32 esp, int32_t offset, reg32 menu);
// The game's 0x449ea0: the pointer moved onto the selected item of the screen at `menu`; esp the frame base.
Result pointAtSelectedOf(App* app, CPU& cpu, reg32 esp, reg32 menu);
// The game's 0x44a240: the Escape button of the screen at `menu`, its index to `offset`; esp the frame base.
Result escapeButtonOf(App* app, CPU& cpu, reg32 esp, reg32 menu, reg32 offset);
// The game's 0x44a2d0: key `key` on the screen at `menu`; esp the frame base.
Result handleKeyOf(App* app, CPU& cpu, reg32 esp, reg32 menu, reg32 key);
// The game's 0x44a4f0: the items of the screen at `menu` drawn; esp the frame base.
Result drawItemsOf(App* app, CPU& cpu, reg32 esp, reg32 menu);
// The game's 0x44a6b0: the item of the screen at `menu` under the mouse; esp the frame base.
Result itemUnderMouseOf(App* app, CPU& cpu, reg32 esp, reg32 menu);
// The game's 0x44a8b0: a pass of the items of the screen at `menu`; esp the frame base.
Result passItemsOf(App* app, CPU& cpu, reg32 esp, reg32 menu);
// The game's 0x44aa30: the screen at `menu` left, how by `action`; esp the frame base.
Result leaveScreenOf(App* app, CPU& cpu, reg32 esp, reg32 menu, reg32 action);
// The game's 0x44ab40: the screen at `menu` opened; esp the frame base.
Result enterScreenOf(App* app, CPU& cpu, reg32 esp, reg32 menu);
// The game's 0x44aeb0: the screen shown.
reg32 currentMenuOf(App* app);
// The game's 0x44aec0: back to the screen gone from last.
Result popMenuOf(App* app, CPU& cpu, reg32 eax);
// The game's 0x44af10: the screen shown onto the history.
Result pushMenuOf(App* app, CPU& cpu, reg32 eax);
// The game's 0x44b230: the menus from the screen named at `name`, `history` the history; esp the frame base.
Result menuLoopOf(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 history);

}  // namespace loop
}  // namespace port

namespace guest
{

// `eax` in eax; eax back.
template <>
struct ByAddress<port::loop::keysTakenOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// The item in eax, the screen in edx; eax back.
template <>
struct ByAddress<port::loop::selectableOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t offset,
                             x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(offset), menu});
    }
};

// The screen in eax; eax back.
template <>
struct ByAddress<port::loop::pointAtSelectedOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {menu});
    }
};

// The screen in eax, where the index goes in edx; eax back.
template <>
struct ByAddress<port::loop::escapeButtonOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu,
                             x86::reg32 offset)
    {
        return watcomEax(app, cpu, Address, {menu, offset});
    }
};

// The screen in eax, the key in edx; eax back.
template <>
struct ByAddress<port::loop::handleKeyOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu, x86::reg32 key)
    {
        return watcomEax(app, cpu, Address, {menu, key});
    }
};

// The screen in eax; eax back.
template <>
struct ByAddress<port::loop::drawItemsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {menu});
    }
};

// The screen in eax; eax back.
template <>
struct ByAddress<port::loop::itemUnderMouseOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {menu});
    }
};

// The screen in eax; eax back.
template <>
struct ByAddress<port::loop::passItemsOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {menu});
    }
};

// The screen in eax, how in edx; eax back.
template <>
struct ByAddress<port::loop::leaveScreenOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu,
                             x86::reg32 action)
    {
        return watcomEax(app, cpu, Address, {menu, action});
    }
};

// The screen in eax; eax back.
template <>
struct ByAddress<port::loop::enterScreenOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 menu)
    {
        return watcomEax(app, cpu, Address, {menu});
    }
};

// Nothing handed over; the screen in eax.
template <>
struct ByAddress<port::loop::currentMenuOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::loop::popMenuOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::loop::pushMenuOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// The name in eax, the history in edx; eax back.
template <>
struct ByAddress<port::loop::menuLoopOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 name,
                             x86::reg32 history)
    {
        return watcomEax(app, cpu, Address, {name, history});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
