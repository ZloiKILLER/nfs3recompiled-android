#include "native_frontend_loop.h"
#include "native_frontend_screen.h"
#include "native_frontend.h"
#include <iterator>

/* What the menu loop (port::loop, src/nfs3hp/port_loop.cpp) leaves to the
 * main menu's decompiled code: native whole, with no other code behind
 * them, a screen's items shown, whether
 * an item is, and the name of the screen shown.
 *
 * The screen shown is a pointer into the table of screens (0x559230).  An item
 * has handlers of its own, set by its widget: as the screen opens (+0x24).
 * An item can be greyed (its state's bit 0) or hidden (kHidden).
 *
 * The same screens serve a race as its pause screens (0x559234 set, their
 * own loop at sub_44bac0): no fades then, no art of their own, and only the
 * items marked for it.
 *
 * Plan step 5: each function a typed core in frontend::loop (native_frontend_loop.h), in a frame
 * of its own where it calls anything (fe::Frame: what it calls runs at the stack addresses it
 * did), and a thunk under the game's name for the calls by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;

// The screens and their items.
constexpr nfs::u32 kPostRaceOnly = 0x14;         // int16, "postonly"
constexpr nfs::u32 kMainOnly = 0x16;             // int16, "mainonly": not after a race
constexpr nfs::u32 kOnShow = 0x24;               // its handler as the screen opens
constexpr nfs::u32 kStateWord = 0x04;            // the state and flags bytes as one word
// The state word's bits.
constexpr std::uint16_t kInRaceToo = 0x0800;       // shown in the pause screens too
constexpr std::uint16_t kGreyAndHide = 0x1001;

// The loop's state.
constexpr nfs::u32 kCurrentMenu = 0x559230;
constexpr nfs::u32 kInRace = 0x559234;           // the menus are a race's pause screens
constexpr nfs::u32 kAfterRace = 0x662968;        // the menus opened on "postgame"
constexpr nfs::u32 kNetworkConnected = 0x7a22e0;
constexpr nfs::u32 kNetworkShown = 0x440970;

nfs::u32 itemAt(std::int32_t index)
{
    return fe::item::kTable + nfs::u32(index) * fe::item::kSize;
}

std::int16_t signedWord(const Guest& guest, nfs::u32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t firstItem(const Guest& guest, nfs::u32 menu)
{
    return signedWord(guest, menu + fe::menu::kFirstItem);
}

bool isEnd(const Guest& guest, nfs::u32 item)
{
    return guest.dword(item + fe::item::kType) == 0;
}

}

/* sub_44a660: whether an item is shown: in the pause screens, only one marked
 * for them. */
nfs::u32 frontend::loop::shownNow(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    if (guest.dword(kInRace) != 0 && (guest.word(item + kStateWord) & kInRaceToo) == 0)
        return 0;
    return 1;
}

/* sub_44a960: every item's kOnShow handler, the screen opening; an item for
 * after a race only, or for before one only, greyed out and hidden when it
 * is not the time for it.  The network screens get theirs. */
nfs::u32 frontend::loop::showItems(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    std::int16_t index = std::int16_t(firstItem(guest, menu));
    for (; !isEnd(guest, itemAt(index)); ++index)
    {
        const nfs::u32 item = itemAt(index);
        if (guest.dword(item + kOnShow) == 0)
            continue;
        guest.call(guest.dword(item + kOnShow), fe::args({ item, item }));
        if (guest.word(item + kPostRaceOnly) != 0 && guest.dword(kAfterRace) == 0)
            guest.setWord(item + kStateWord, guest.word(item + kStateWord) | kGreyAndHide);
        if (guest.word(item + kMainOnly) != 0 && guest.dword(kAfterRace) != 0)
            guest.setWord(item + kStateWord, guest.word(item + kStateWord) | kGreyAndHide);
    }
    // eax as the game's function leaves it: the end's offset in the
    // table, or what the network screens' art leaves.
    if (guest.dword(kNetworkConnected) != 0)
        return fe::direct<kNetworkShown, frontend::screen::loadScreenArt>(guest.app(), guest.ts());
    return nfs::u32(std::int32_t(index)) * fe::item::kSize;
}

/* sub_44ae00: the name of the screen shown, 0 for none. */
nfs::u32 frontend::loop::currentMenuName(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const nfs::u32 menu = guest.dword(kCurrentMenu);
    return menu != 0 ? guest.dword(menu + fe::menu::kName) : 0;
}

using fe::kRestoreEbx;
using fe::kRestoreEdx;
constexpr std::uint8_t kRestoreNone = 0;

/* Native whole, no other code behind them: a screen's items shown, whether an item is, the shown
 * screen's name. */
namespace frontend
{

nfs::u32 menuShowItems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44a960", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loop::showItems(app, ts, menu); });
}

nfs::u32 menuShownNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44a660", 0, 0, nullptr, 0, kRestoreNone };
    const nfs::u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loop::shownNow(app, ts, item); });
}

nfs::u32 menuCurrentName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44ae00", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loop::currentMenuName(app, ts); });
}

}  // namespace frontend

}
