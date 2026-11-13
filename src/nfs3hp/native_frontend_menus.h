#ifndef NFS3HP_NATIVE_FRONTEND_MENUS_H
#define NFS3HP_NATIVE_FRONTEND_MENUS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menus.cpp's typed function (the item lookup), which other files call as C++
 * (guest::direct), and how the game's code calls it by address (ByAddress, for the checks'
 * stand-ins).  It calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function
 * leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menus
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x442a40: a screen's item by its codelink (name), 0 when it has none.
reg32 findItem(App* app, CPU& cpu, reg32 menu, reg32 name);

}  // namespace menus
}  // namespace frontend

namespace guest
{

// The screen in eax, the codelink in edx.
template <>
struct ByAddress<frontend::menus::findItem>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu, name }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
