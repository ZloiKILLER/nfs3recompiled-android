#include "native_frontend_menus.h"
#include "native_frontend.h"

/* The item lookup the screens use: a screen's item by its codelink.
 *
 * The .MNU files that make the screens and their items, the codelink match
 * the lookup is made with, and a screen's handlers by its name are
 * rewritten whole in src/nfs3hp/port_menus.cpp (port::menus), entered by
 * their addresses; this calls the match where it is, so that it runs as
 * itself.
 *
 * Plan step 5: the lookup a typed core in frontend::menus (native_frontend_menus.h), in a frame of
 * its own (fe::Frame: what it calls runs at the stack addresses it did), and an adapter under the
 * game's name for the calls by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;

// The match of a codelink with a name: port::menus::codelinkMatches.
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;

x86::reg32 itemAt(std::uint32_t index)
{
    return fe::item::kTable + index * fe::item::kSize;
}

}

/* sub_442a40: a screen's item by its codelink, 0 when it has none: its items
 * from its first to the end of the screen, an item of type 0. */
x86::reg32 frontend::menus::findItem(App* app, CPU& cpu, reg32 menu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    const std::int16_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    for (x86::reg32 index = x86::reg32(std::int32_t(first));; ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + fe::item::kType) == 0)
            return 0;
        const x86::reg32 codelink = guest.dword(item + fe::item::kCodelink);
        // By address: edx goes back to the caller as the match leaves it.
        if (codelink != 0 && guest.call(kCodelinkMatches, fe::args({ codelink, name })) != 0)
            return item;
    }
}

using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEsi;
using fe::kRestoreEdi;

/* Native whole, no generated code behind it (disassemble_nfs3hp.py's
 * MENU_NATIVES): the main menu looks its items up with it. */
namespace frontend
{

void menuFindItem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442a40", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 menu = cpu.eax, name = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return menus::findItem(app, cpu, menu, name); });
}

}  // namespace frontend

}
