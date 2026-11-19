#include "native_frontend_connect.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_frontend.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_glue.h"
#include "native_frontend_loop.h"
#include "native_frontend_neta.h"
#include "native_frontend_netb.h"
#include "native_raceload.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_draw2d.h"
#include "port_loop.h"
#include "port_menus.h"
#include "port_snd_stream_calls.h"
#include "port_sound_music.h"
#include "native_graphics.h"
#include <nfs3hp.h>
#include <lib/app.h>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined (the header's declarations are the thunks'). */
namespace nfs3hp
{
namespace frontend {
nfs::u32 connectApplyPlayerVector(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectListingRedraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxRefreshGames(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectOpenProtocolList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectOpenAddressDialog(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectModemLabel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectComPortLabel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxGameNameAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectAddressAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectTcpAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectProtocolListAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSerialAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectPhoneAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectModemAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectRaceNetMissingAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectRaceNetAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectRaceNetClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectTcpClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSerialClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectModemClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxCreateGameClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxRefreshClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ipxOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectFlushKeys(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSetEditField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSetPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectConnectionAvailable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectListSessions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectCreateSession(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectJoinSession(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectUseTcp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectFindAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSetUpEndpoint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectEnumerateServices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectFindConnections(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectServiceFound(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSessionFound(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectQueuePeerByName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectQueuePeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectClearPeerSlots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectRememberPeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectClearQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectEnqueuePeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectStartConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectListModems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectSetUpLink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 connectShakeHands(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax: the first `Taken` of `words` its arguments (the
 * rest what the game's code has in the other registers, which the function does not read); its
 * result. */
template <nfs::u32 Address, auto Core, std::size_t... I>
auto directOf(nfs::App* app, nfs::ThreadState& ts, const nfs::u32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, ts, words[I]...);
}

template <nfs::u32 Address, auto Core, std::size_t Taken, typename... Words>
nfs::u32 directKept(nfs::App* app, nfs::ThreadState& ts, Words... words)
{
    const nfs::u32 all[] = { nfs::u32(words)..., 0 };
    const auto result = directOf<Address, Core>(app, ts, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, nfs::u32>)
        return result;
    else
        return *result;
}

}  // namespace
}  // namespace nfs3hp


/* The network's menus, decompiled:
 *
 *   connect    MENUS/CONNECT.MNU: how the game is connected -- IPX, TCP/IP, modem,
 *              serial or RaceNet -- with the questions, lists and lines to type
 *              in each of them puts up, and their answers;
 *   ipx        IPX.MNU: the games found on the network, a game's name, creating
 *              one, joining one, the list looked for again every so often.
 *
 * The port hides RaceNet, the modem and the serial cable (the port's hidden connections change):
 * connect gives those three items no handler, so their questions below are not
 * reached any more; they are kept as the game has them.
 *
 * Plan step 5: each function a typed core in frontend::connect (native_frontend_connect.h), in a
 * frame of its own where the game's function has one (fe::Frame), and a thunk under the game's
 * name for the calls by address.  A core of this file calls another as C++ (guest::direct); the
 * rest of the game still by address. */
namespace nfs3hp
{

/* The TCP/IP port a race is hosted and joined on, the launcher's (nfs3hp_main.cpp,
 * the port's network port change). */
nfs::u32 networkPort();

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;

/* ---------------------------------------------------------------------------
 * What these screens share
 * ------------------------------------------------------------------------- */

// The items' codelinks.
constexpr GuestString kRaceNet{ 0x5378f8, "racenet" };
constexpr GuestString kIpx{ 0x537900, "ipx" };
constexpr GuestString kTcp{ 0x537904, "tcp" };
constexpr GuestString kModem{ 0x537908, "modem" };
constexpr GuestString kSerial{ 0x537910, "serial" };
constexpr GuestString kJoinGame{ 0x537918, "joingame" };
constexpr GuestString kCreateGame{ 0x537924, "creategame" };
constexpr GuestString kRefreshList{ 0x537930, "refreshlist" };
constexpr GuestString kGameName{ 0x537840, "gamename" };   // an edit box's, and the name IPX's own session goes by
constexpr GuestString kEscape{ 0x5378e8, "escape" };
constexpr GuestString kAllocName{ 0x537830, "" };          // the memory's name: none
constexpr GuestString kColonSpace{ 0x537834, ": " };
constexpr GuestString kComPortFormat{ 0x537838, "%s: %d" };

// The game's state.
constexpr nfs::u32 kConnectWord = 0x5556cc;       // int16, cleared as connect opens
constexpr nfs::u32 kRaceNetUnwanted = 0x5556ce;   // int16: RaceNet greyed out and hidden
constexpr nfs::u32 kTicks = 0x79c1a4;
constexpr nfs::u32 kGameList = 0x5f3ff0;          // int16 count, and +4 the games, 64 bytes each
constexpr nfs::u32 kNextRefresh = 0x5f3fec;
constexpr std::uint32_t kRefreshEvery = 0x500;
constexpr nfs::u32 kSelectList = 0x5556c8;        // the games' list on the screen
constexpr nfs::u32 kModemCount = 0x5f3df0;
constexpr nfs::u32 kModemNames = 0x5f3df4;        // 30 bytes each
constexpr std::int32_t kComPorts = 4;
constexpr nfs::u32 kNetworkKind = 0x7a22e4;
constexpr nfs::u32 kNetworkHost = 0x7a22da;       // int16
constexpr nfs::u32 kDevFlags = 0x55eb2e;          // byte
constexpr std::uint8_t kSkipNetwork = 0x10;         // no network at all: the fourth kind of race
constexpr nfs::u32 kRaceNetDial = 0x55e894;
constexpr nfs::u32 kHeldKey = 0x664648;

// A list on an item: its rows, 8 bytes each (the text, and a byte 0 after it), and their count.
constexpr nfs::u32 kListCount = 0x48;
constexpr nfs::u32 kListRows = 0x70;
constexpr nfs::u32 kListRowSize = 8;
constexpr nfs::u32 kGameSize = 0x40;
constexpr nfs::u32 kGamesField = 4;
// An edit box's: the text it edits, how many characters, the third of the arguments it was set up with.
constexpr nfs::u32 kEditText = 0x44;
constexpr nfs::u32 kEditLength = 0x4c;
constexpr nfs::u32 kEditThird = 0x50;
// The item's state and flags together, as one word: greyed out and hidden.
constexpr std::uint16_t kDisabledAndHidden = 0x1001;

// The C library's.
constexpr nfs::u32 kMemset = 0x4e0640;
constexpr nfs::u32 kStrncat = 0x4eedf0;
constexpr nfs::u32 kSprintf = 0x4df690;
// The menu engine's.
constexpr nfs::u32 kDrawPointer = 0x4dab90;
constexpr nfs::u32 kSelectListUp = 0x4633d0;
// The handlers these screens hand the menu engine.
constexpr nfs::u32 kListingRedraw = 0x43f1f0;
constexpr nfs::u32 kGameNameAnswer = 0x43f640;
constexpr nfs::u32 kAddressAnswer = 0x43f680;
constexpr nfs::u32 kTcpAnswer = 0x43f760;
constexpr nfs::u32 kProtocolListAnswer = 0x43f7e0;
constexpr nfs::u32 kSerialAnswer = 0x43f960;
constexpr nfs::u32 kPhoneAnswer = 0x43fa00;
constexpr nfs::u32 kModemAnswer = 0x43faf0;
constexpr nfs::u32 kRaceNetMissingAnswer = 0x43fc20;
constexpr nfs::u32 kRaceNetAnswer = 0x43fc50;
constexpr nfs::u32 kTcpClicked = 0x43fe50;
constexpr nfs::u32 kCreateGameClicked = 0x440040;
constexpr nfs::u32 kRefreshClicked = 0x4400b0;
constexpr nfs::u32 kJoinListedGame = 0x43fdd0;    // port::buttons::joinListedGame
constexpr nfs::u32 kBackClicked = 0x4400d0;       // frontend::multiMenuOnBack

// What a screen's handler hands the menu loop: on to the selected button's nextmenu (port_loop.cpp).
constexpr nfs::u32 kMenuGoOn = 5;
constexpr nfs::u32 kMenuRaceNetDial = 0xa;
constexpr int kMessageRedraws = 15;

// A line of the language's text put in one of a dialog's arrays.
void putText(Guest& guest, nfs::u32 at, std::uint32_t id)
{
    guest.setDword(at, fe::text(guest, id));
}

/* sub_44b170: the screen drawn again and shown.  It reads no register (the ones its code hands on
 * to what it calls are the caller's leftovers, which none of those reads). */
void redrawMenu(Guest& guest, ThreadState& ts)
{
    fe::direct<0x44b170, port::buttons::redrawn>(guest.app(), ts, ts.sp - 4, port::Args{});
}

/* sub_444a60: the message to go on the next pass.  It reads no register. */
void endMessage(Guest& guest, ThreadState& ts)
{
    fe::direct<0x444a60, port::dialogs::endMessageOf>(guest.app(), ts, 0);
}

void flush(Guest& guest, ThreadState& ts)
{
    guest::direct<0x451940, frontend::connect::flushKeys>(guest.app(), ts, 0);
}

/* The "join" button's look: grey with no games listed. */
void showJoinable(Guest& guest, Item join)
{
    const nfs::u32 games = guest.dword(kGameList);
    join.setDisabled(games == 0 || guest.word(games) == 0);
}

void greyIfThere(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    if (Item item = fe::findItem(guest, menu, codelink))
        item.setDisabled(true);
}

}  // namespace

/* ---------------------------------------------------------------------------
 * Looking for games
 * ------------------------------------------------------------------------- */

/* sub_43f220: the games on the network looked for -- the pointer busy meanwhile,
 * the screen redrawn by sub_43f1f0 -- and shown as the rows of the list `list`:
 * one a game, or one line saying there are none. */
nfs::u32 frontend::connect::refreshGames(App* app, ThreadState& ts, u32 list)
{
    constexpr nfs::u32 kBusyPointer = 3, kNormalPointer = 0;
    constexpr nfs::u32 kLookFor = 0x180;
    constexpr std::uint32_t kNoGames = 0xbd;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    guest::direct<0x49c0e0, setPointer>(app, ts, kBusyPointer);
    guest.setDword(kGameList, guest::direct<0x49e3d0, listSessions>(app, ts, kLookFor, SETTINGS_ADDRESS(players[0].name),
                                                                    1, kListingRedraw));
    guest::direct<0x49c0e0, setPointer>(app, ts, kNormalPointer);
    guest.setDword(kNextRefresh, guest.dword(kTicks) + kRefreshEvery);

    if (const nfs::u32 old = guest.dword(list + kListRows))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, old);
        guest.setDword(list + kListRows, 0);
    }
    const nfs::u32 games = guest.dword(kGameList);
    const std::int32_t count = std::int16_t(guest.word(games));
    if (count != 0)
    {
        guest.setDword(list + kListCount, nfs::u32(count));
        guest.setDword(list + kListRows, *fe::direct<0x4e1620, ea::reserved>(app, ts, kAllocName.address,
                                                                             nfs::u32(count + 1) * kListRowSize, 0));
        const nfs::u32 rows = guest.dword(list + kListRows);
        for (std::int32_t i = 0; i < count; ++i)
        {
            const nfs::u32 row = rows + nfs::u32(i) * kListRowSize;
            guest.setDword(row, guest.dword(games + kGamesField) + nfs::u32(i) * kGameSize);
            guest.setByte(row + 4, 0);
        }
    }
    else
    {
        guest.setDword(list + kListCount, 1);
        guest.setDword(list + kListRows, *fe::direct<0x4e1620, ea::reserved>(app, ts, kAllocName.address,
                                                                             2 * kListRowSize, 0));
        const nfs::u32 line = fe::text(guest, kNoGames);
        guest.setDword(guest.dword(list + kListRows), line);
        guest.setByte(guest.dword(list + kListRows) + 4, 0);
    }
    // The eax it leaves is the item it was handed.
    fe::direct<kSelectListUp, port::buttons::selectLaidOut>(guest.app(), guest.ts(), list);
    return list;
}

/* ---------------------------------------------------------------------------
 * connect: IPX and TCP/IP's list, a TCP address
 * ------------------------------------------------------------------------- */

namespace
{

constexpr nfs::u32 kListTitle = 0x55571c;       // a line
constexpr nfs::u32 kListEntries = 0x5556f4;     // the entries' names
constexpr nfs::u32 kListEntryHelp = 0x555708;   // a line of help for each
constexpr nfs::u32 kListTicks = 0x5556e0;
constexpr nfs::u32 kListButtons = 0x555724;
constexpr nfs::u32 kListMode = 0x5f3fe8;        // what the list was opened for: 0 IPX, 1 TCP/IP, 2 a modem
constexpr nfs::u32 kKindTick = 0x5f3fe4;
constexpr nfs::u32 kOwnOnlyTick = 0x5f3fe0;

}  // namespace

/* sub_43f320: the list of how the game goes on the network, ticked as it is set. */
nfs::u32 frontend::connect::openProtocolList(App* app, ThreadState& ts, u32 mode)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kListEntries, 0, 0x14);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kListEntryHelp, 0, 0x14);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kListTitle, 0, 8);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kListButtons, 0, 0xc);
    putText(guest, kListTitle, 0x15);
    putText(guest, kListEntries, 0x1b);
    putText(guest, kListEntries + 4, 0x1c);
    guest.setDword(kListEntries + 8, 0);
    putText(guest, kListEntries + 12, 0x1d);
    putText(guest, kListEntries + 16, 0x1e);
    putText(guest, kListEntryHelp, 0x1f);
    putText(guest, kListEntryHelp + 4, 0x20);
    guest.setDword(kListEntryHelp + 8, 0);
    putText(guest, kListEntryHelp + 12, 0x21);
    putText(guest, kListEntryHelp + 16, 0x22);
    putText(guest, kListButtons, 0xf9);
    putText(guest, kListButtons + 4, 0xc6);

    guest.setDword(kListMode, mode);
    if (mode <= 2)
    {
        gameSettings(guest).networkGame = 0;
        guest.setDword(kNetworkKind, mode == 2 ? 0 : 1);
    }
    guest.setDword(kKindTick, guest.dword(kNetworkKind) == 0);
    guest.setDword(kOwnOnlyTick, gameSettings(guest).networkGame == 0);
    return nfs::u32(*fe::direct<0x445750, port::dialogs::openListOf>(app, ts, ts.sp - 4 - 4 * 8, 1, kListTitle, 2,
                                                                       kListEntries, kListTicks, kListEntryHelp, 2,
                                                                       kListButtons, 0, 0, 0, kProtocolListAnswer));
}

/* sub_43f4e0: a line to type the game's TCP address in. */
nfs::u32 frontend::connect::openAddressDialog(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLines = 0x5f4008, kButtons = 0x5f4010;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    putText(guest, kLines, 0x17);
    putText(guest, kLines + 4, 0xa);
    putText(guest, kButtons, 0xf9);
    putText(guest, kButtons + 4, 0xc6);
    return fe::direct<0x444c20, frontend::dialogs::openEdit>(app, ts, 2, kLines, 2, kButtons, 0,
                                                            SETTINGS_ADDRESS(tcpAddress), 0x1d, 1, kAddressAnswer);
}

/* sub_43f640: the player's name typed for a game to create: the list next. */
nfs::u32 frontend::connect::gameNameAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
            guest::direct<0x43f320, openProtocolList>(app, ts, 0);
        else if (focus == 1)
            guest.setByte(closes, 1);
    }
    else if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
    }
    return 0;
}

/* sub_43f680: a TCP address typed: kept, looked up, and the game there joined,
 * with a message up meanwhile. */
nfs::u32 frontend::connect::addressAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    constexpr std::uint32_t kConnecting = 0xc;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x18);
    const nfs::u32 typed = fe::dialogEditText(guest);
    guest.setByte(closes, 0);
    const std::uint32_t focus = fe::dialogFocus(guest);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    if (focus == 1)
    {
        guest.setByte(closes, 1);
    }
    else if (focus == 0)
    {
        const nfs::u32 endpoint = guest.frame();   // 0x14 bytes
        guest.copyString(SETTINGS_ADDRESS(tcpAddress), typed);
        if (guest::direct<0x49ea70, findAddress>(app, ts, typed, endpoint) != 0)
        {
            guest.setByte(closes, 1);
            fe::messageBox(guest, kConnecting);
            for (int i = 0; i < kMessageRedraws; ++i)
                redrawMenu(guest, ts);
            if (guest::direct<0x49e800, joinSession>(app, ts, endpoint) != 0)
            {
                endMessage(guest, ts);
                redrawMenu(guest, ts);
                flush(guest, ts);
                return kMenuGoOn;
            }
        }
    }
    flush(guest, ts);
    return 0;
}

/* sub_43f760: TCP/IP's question answered: create (the list), join (an address). */
nfs::u32 frontend::connect::tcpAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest.setByte(closes, 0);
    if (guest::direct<0x49e9d0, useTcp>(app, ts, SETTINGS_ADDRESS(players[0].name)) == 0)
    {
        guest.setByte(closes, 1);
        return 0;
    }
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
            guest::direct<0x43f320, openProtocolList>(app, ts, 1);
        else if (focus == 1)
            guest::direct<0x43f4e0, openAddressDialog>(app, ts);
        else if (focus == 2)
            guest.setByte(closes, 1);
    }
    else if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
    }
    return 0;
}

/* sub_43f7e0: the list's answer: its first button makes what the list was opened
 * for, its second closes it, and an entry answered clears what it stands for. */
nfs::u32 frontend::connect::protocolListAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    const std::uint32_t focus = fe::dialogFocus(guest);
    if (focus == 0)
    {
        switch (guest.dword(kListMode))
        {
        case 0:
            if (guest.byte(kDevFlags) & kSkipNetwork)
            {
                guest.setByte(closes, 1);
                gameSettings(guest).raceType = 3;
                return kMenuGoOn;
            }
            if (guest::direct<0x49e5f0, createSession>(app, ts, fe::dialogEditText(guest), 1) != 0)
            {
                guest.setByte(closes, 1);
                return kMenuGoOn;
            }
            break;
        case 1:
            if (guest::direct<0x49e5f0, createSession>(app, ts, kGameName.address, 2) != 0)
            {
                guest.setByte(closes, 1);
                return kMenuGoOn;
            }
            break;
        case 2:
            gameSettings(guest).playMode = 2;
            guest.setWord(kNetworkHost, 1);
            if (guest::direct<0x4c3f90, startConnection>(app, ts, gameSettings(guest).chosenModem) != 0)
            {
                endMessage(guest, ts);
                redrawMenu(guest, ts);
                guest.setByte(closes, 1);
                return kMenuGoOn;
            }
            gameSettings(guest).playMode = 0;
            break;
        default:
            break;
        }
        flush(guest, ts);
        return 0;
    }
    if (focus == 1)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    switch (fe::direct<0x445150, port::dialogs::listFocusOf>(app, ts))
    {
    case 0:
        guest.setDword(kNetworkKind, 0);
        break;
    case 1:
        guest.setDword(kKindTick, 0);
        break;
    case 3:
        guest.setDword(kOwnOnlyTick, 0);
        break;
    case 4:
        gameSettings(guest).networkGame = 0;
        break;
    default:
        break;
    }
    guest.setByte(closes, 0);
    flush(guest, ts);
    return 0;
}

/* ---------------------------------------------------------------------------
 * connect: the serial cable and the modem
 * ------------------------------------------------------------------------- */

/* sub_43f550: "Modem: <its name>", in a buffer of its own, within 30 characters. */
nfs::u32 frontend::connect::modemLabel(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLabel = 0x5f401c;
    constexpr std::uint32_t kLabelSize = 0x1e, kModemWord = 0x13;
    constexpr nfs::u32 kModemNameSize = 30;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    guest.copyString(kLabel, fe::text(guest, kModemWord));
    guest.copyString(kLabel + guest.length(kLabel), kColonSpace.address);
    const nfs::u32 name = kModemNames + gameSettings(guest).chosenModem * kModemNameSize;
    if (name != 0)
        fe::direct<kStrncat, watcom::clib::strncat>(guest.app(), guest.ts(), kLabel, name, kLabelSize - guest.length(kLabel) - 1);
    guest.setByte(kLabel + kLabelSize - 1, 0);
    return kLabel;
}

/* sub_43f610: "COM port: <n>", in a buffer of its own. */
nfs::u32 frontend::connect::comPortLabel(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLabel = 0x5f403a;
    constexpr std::uint32_t kPortWord = 0x14;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const nfs::u32 port = gameSettings(guest).comPort + 1;
    const nfs::u32 word = fe::text(guest, kPortWord);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kLabel, kComPortFormat.address, guest::Words{word, port});
    return kLabel;
}

/* sub_43f960: the serial question's answer: connect, or the next COM port. */
nfs::u32 frontend::connect::serialAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    nfs::u32 result = 0;
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    const std::uint32_t focus = fe::dialogFocus(guest);
    if (focus == 0)
    {
        gameSettings(guest).playMode = 3;
        if (guest::direct<0x4c3f90, startConnection>(app, ts, 0) != 0)
        {
            endMessage(guest, ts);
            redrawMenu(guest, ts);
            result = kMenuGoOn;
            guest.setByte(closes, 1);
        }
    }
    else if (focus == 1)
    {
        const std::int32_t next = std::int32_t(gameSettings(guest).comPort + 1) % kComPorts;
        gameSettings(guest).comPort = nfs::u32(next);
        guest::direct<0x43f610, comPortLabel>(app, ts);
    }
    else if (focus == 2)
    {
        guest.setByte(closes, 1);
    }
    flush(guest, ts);
    return result;
}

/* sub_43fa00: the phone number typed: dialled. */
nfs::u32 frontend::connect::phoneAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    const std::uint32_t focus = fe::dialogFocus(guest);
    nfs::u32 result = 0;
    if (focus == 0)
    {
        guest.copyString(SETTINGS_ADDRESS(phoneNumber), fe::dialogEditText(guest));
        if (guest.length(SETTINGS_ADDRESS(phoneNumber)) == 0)
        {
            guest.setByte(closes, 0);
            return 0;
        }
        guest.setByte(closes, 1);
        guest.setWord(kNetworkHost, 0);
        gameSettings(guest).playMode = 2;
        if (guest::direct<0x4c3f90, startConnection>(app, ts, gameSettings(guest).chosenModem) != 0)
        {
            endMessage(guest, ts);
            redrawMenu(guest, ts);
            result = kMenuGoOn;
        }
    }
    else if (focus == 1)
    {
        guest.setByte(closes, 1);
    }
    flush(guest, ts);
    return result;
}

/* sub_43faf0: the modem question's answer: dial (a line to type the number in),
 * wait for a call (the list), or with more than one modem the next of them. */
nfs::u32 frontend::connect::modemAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    constexpr nfs::u32 kModemButton = 0x5f3de0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        flush(guest, ts);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    std::uint32_t focus = fe::dialogFocus(guest);
    const std::int32_t modems = std::int32_t(guest.dword(kModemCount));
    if (modems > 1)
    {
        // The first button is the modem's name: the next modem.
        if (focus == 0)
        {
            guest.setByte(closes, 0);
            const std::int32_t next = std::int32_t(gameSettings(guest).chosenModem + 1) % modems;
            gameSettings(guest).chosenModem = nfs::u32(next);
            guest.setDword(kModemButton, guest::direct<0x43f550, modemLabel>(app, ts));
            return 0;
        }
        --focus;
    }
    if (focus == 0)
    {
        constexpr nfs::u32 kLines = 0x5f4050, kButtons = 0x5f4058;
        guest.setByte(closes, 0);
        putText(guest, kLines, 0x12);
        putText(guest, kLines + 4, 0xf);
        putText(guest, kButtons, 0xf9);
        putText(guest, kButtons + 4, 0xc6);
        fe::openDialog(guest, { 2, kLines, 2, kButtons, 0, SETTINGS_ADDRESS(phoneNumber), 0x1d, 2, kPhoneAnswer });
    }
    else if (focus == 1)
    {
        guest::direct<0x43f320, openProtocolList>(app, ts, 2);
        guest.setByte(closes, 0);
    }
    else if (focus == 2)
    {
        guest.setByte(closes, 1);
    }
    return 0;
}

/* sub_43ff50: the modem button: the question, the modem's name its first button
 * when there is more than one. */
nfs::u32 frontend::connect::modemClicked(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLine = 0x5f4094, kButtons = 0x5f3de0;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (std::int32_t(gameSettings(guest).chosenModem) >= std::int32_t(guest.dword(kModemCount)))
        gameSettings(guest).chosenModem = 0;
    putText(guest, kLine, 0x12);
    nfs::u32 at = kButtons;
    if (std::int32_t(guest.dword(kModemCount)) > 1)
    {
        guest.setDword(at, guest::direct<0x43f550, modemLabel>(app, ts));
        at += 4;
    }
    putText(guest, at, 0x19);
    putText(guest, at + 4, 0x1a);
    putText(guest, at + 8, 0xc6);
    fe::openQuestion(guest, { 1, kLine, (at - kButtons) / 4 + 3, kButtons, 0, 0, 1, kModemAnswer });
    return 0;
}

/* sub_43fee0: the serial button: its question. */
nfs::u32 frontend::connect::serialClicked(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLine = 0x5f4084, kButtons = 0x5f4088;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    putText(guest, kLine, 0x11);
    putText(guest, kButtons, 0x18);
    guest.setDword(kButtons + 4, guest::direct<0x43f610, comPortLabel>(app, ts));
    putText(guest, kButtons + 8, 0xc6);
    fe::openQuestion(guest, { 1, kLine, 3, kButtons, 0, 0, 1, kSerialAnswer });
    return 0;
}

/* ---------------------------------------------------------------------------
 * connect: RaceNet and TCP/IP's buttons
 * ------------------------------------------------------------------------- */

/* sub_43fc20: the message that dial-up networking is missing, answered. */
nfs::u32 frontend::connect::raceNetMissingAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer != fe::kDialogEnter && answer != fe::kDialogEscape)
        return 0;
    fe::dialogFocus(guest);
    if (closes != 0)
        guest.setByte(closes, 1);
    return fe::kDialogHandled;
}

/* sub_43fc50: RaceNet's question answered: dial, or not. */
nfs::u32 frontend::connect::raceNetAnswer(App* app, ThreadState& ts, u32 answer, u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            guest.setDword(kRaceNetDial, 3);
            return kMenuRaceNetDial;
        }
        if (closes != 0)
            guest.setByte(closes, focus == 1 ? 1 : 0);
        return fe::kDialogHandled;
    }
    if (answer == fe::kDialogEscape)
    {
        if (closes != 0)
            guest.setByte(closes, 1);
        return fe::kDialogHandled;
    }
    return 0;
}

/* sub_43fd10: RaceNet's button: a question, or a message that dial-up networking
 * is missing. */
nfs::u32 frontend::connect::raceNetClicked(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLines = 0x5f4064, kButtons = 0x5f406c;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (fe::direct<0x49f790, frontend::neta::rasAvailable>(app, ts) != 0 || fe::direct<0x49f880, frontend::neta::rasPhoneBook>(app, ts, 0) != 0)
    {
        putText(guest, kLines, 0x1f3);
        putText(guest, kLines + 4, 0x1f4);
        putText(guest, kButtons, 0xc4);
        putText(guest, kButtons + 4, 0xc5);
        fe::openQuestion(guest, { 2, kLines, 2, kButtons, 0, 0, 0, kRaceNetAnswer });
    }
    else
    {
        putText(guest, kLines, 0x1c2);
        putText(guest, kButtons, 0xf9);
        fe::openQuestion(guest, { 1, kLines, 1, kButtons, 0, 0, 0, kRaceNetMissingAnswer });
    }
    return 0;
}

/* sub_43fe50: TCP/IP's button: joining only, by an address at once; else a question. */
nfs::u32 frontend::connect::tcpClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (gameSettings(guest).installIsRemote != 0)
    {
        if (guest::direct<0x49e9d0, useTcp>(app, ts, SETTINGS_ADDRESS(players[0].name)) != 0)
            guest::direct<0x43f4e0, openAddressDialog>(app, ts);
        return 0;
    }
    constexpr nfs::u32 kLine = 0x5f4074, kButtons = 0x5f4078;
    putText(guest, kLine, 0x10);
    putText(guest, kButtons, 0x15);
    putText(guest, kButtons + 4, 0x17);
    putText(guest, kButtons + 8, 0xc6);
    fe::openQuestion(guest, { 1, kLine, 3, kButtons, 0, 0, 1, kTcpAnswer });
    return 0;
}

/* ---------------------------------------------------------------------------
 * connect itself
 * ------------------------------------------------------------------------- */

/* sub_4405c0: connect as it opens: what this machine cannot do greyed out, the
 * buttons' handlers.  The port's own: RaceNet hidden with IPX and TCP/IP moved up
 * into its place, the modem and serial hidden (the port's hidden connections change),
 * none of the three with a handler -- the game gives them sub_43fd10, sub_43ff50
 * and sub_43fee0. */
nfs::u32 frontend::connect::connectEnter(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kConnectionRaceNet = 3, kConnectionIpx = 1, kConnectionTcp = 2;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setWord(kConnectWord, 0);
    gameSettings(guest).networkLobby = 0;
    gameSettings(guest).raceType = 0;

    Item raceNet = fe::findItem(guest, menu, kRaceNet);
    if (guest::direct<0x49e360, connectionAvailable>(app, ts, kConnectionRaceNet) == 0 && raceNet)
        raceNet.setDisabled(true);
    if (guest.word(kRaceNetUnwanted) != 0 && raceNet)
    {
        const nfs::u32 state = raceNet.address() + fe::item::kState;
        guest.setWord(state, guest.word(state) | kDisabledAndHidden);
    }
    if (guest::direct<0x49e360, connectionAvailable>(app, ts, kConnectionIpx) == 0)
        greyIfThere(guest, menu, kIpx);
    if (guest::direct<0x49e360, connectionAvailable>(app, ts, kConnectionTcp) == 0)
        greyIfThere(guest, menu, kTcp);
    guest::direct<0x4c41d0, listModems>(app, ts, kModemCount);
    if (guest.dword(kModemCount) == 0)
        greyIfThere(guest, menu, kModem);
    if (gameSettings(guest).installIsRemote != 0)
    {
        greyIfThere(guest, menu, kRaceNet);
        greyIfThere(guest, menu, kModem);
        greyIfThere(guest, menu, kSerial);
    }

    if (Item hidden = fe::findItem(guest, menu, kRaceNet))
    {
        hidden.setHidden(true);
        const nfs::u32 ipx = fe::findItem(guest, menu, kIpx).address();
        const nfs::u32 tcp = fe::findItem(guest, menu, kTcp).address();
        const nfs::u32 raceNetRow = hidden.address() + fe::item::kY;
        if (ipx != 0 && tcp != 0 && guest.word(ipx + fe::item::kY) != guest.word(raceNetRow))
        {
            guest.setWord(tcp + fe::item::kY, guest.word(ipx + fe::item::kY));
            guest.setWord(ipx + fe::item::kY, guest.word(raceNetRow));
        }
    }
    if (Item tcp = fe::findItem(guest, menu, kTcp))
        tcp.setOnClick(kTcpClicked);
    if (Item modem = fe::findItem(guest, menu, kModem))
        modem.setHidden(true);
    if (Item serial = fe::findItem(guest, menu, kSerial))
        serial.setHidden(true);
    return 0;
}

/* sub_440730: connect as it closes: the network's mode from the button chosen. */
nfs::u32 frontend::connect::connectExit(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kButton = 5;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    const std::int32_t index = std::int32_t(std::int16_t(guest.word(menu + fe::menu::kFirstItem)))
        + std::int16_t(guest.word(menu + fe::menu::kSelected));
    const nfs::u32 chosen = fe::item::kTable + nfs::u32(index) * fe::item::kSize;
    gameSettings(guest).playMode = 0;
    const nfs::u32 codelink = guest.dword(chosen + fe::item::kCodelink);
    if (guest.dword(chosen + fe::item::kType) != kButton || codelink == 0)
        return 0;
    const auto is = [&](const GuestString& name) {
        // ebx, which the code hands on too, it does not read.
        return *fe::direct<0x4429c0, port::menus::codelinkMatched>(app, ts, ts.sp - 4, codelink, name.address,
                                                                  0) != 0;
    };
    if (is(kModem))
        gameSettings(guest).playMode = 2;
    if (is(kSerial))
        gameSettings(guest).playMode = 3;
    if (is(kIpx))
        gameSettings(guest).playMode = 4;
    if (is(kTcp))
        gameSettings(guest).playMode = 4;
    if (is(kRaceNet))
        gameSettings(guest).playMode = 4;
    return 0;
}

/* ---------------------------------------------------------------------------
 * ipx
 * ------------------------------------------------------------------------- */

/* sub_440040: "create game": a line to type the player's name in. */
nfs::u32 frontend::connect::createGameClicked(App* app, ThreadState& ts)
{
    constexpr nfs::u32 kLine = 0x5f409c, kButtons = 0x5f40a8;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    putText(guest, kLine, 0xe);
    putText(guest, kButtons, 0xf9);
    putText(guest, kButtons + 4, 0xc6);
    fe::openDialog(guest, { 1, kLine, 2, kButtons, 0, SETTINGS_ADDRESS(players[0].name), 0xa, 1, kGameNameAnswer });
    return 0;
}

/* sub_440810: ipx on every pass: the games looked for again when it is time and
 * no dialog is up. */
nfs::u32 frontend::connect::ipxFrame(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const nfs::u32 now = guest.dword(kTicks);
    if (std::int32_t(now) > std::int32_t(guest.dword(kNextRefresh))
        && fe::direct<0x4438b0, port::dialogs::dialogShownOf>(app, ts) == 0)
    {
        guest.setDword(kNextRefresh, now + kRefreshEvery);
        if (const nfs::u32 list = guest.dword(kSelectList))
            guest::direct<0x43f220, refreshGames>(app, ts, list);
    }
    if (Item join = fe::findItem(guest, menu, kJoinGame))
        showJoinable(guest, join);
    return 0;
}

/* sub_440890: ipx as it opens: the name's edit box, the buttons' handlers, and
 * the refresh due at once. */
nfs::u32 frontend::connect::ipxEnter(App* app, ThreadState& ts, u32 menu)
{
    constexpr nfs::u32 kNameLength = 0xb;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (const nfs::u32 name = fe::findItem(guest, menu, kGameName).address())
        guest::direct<0x458450, setEditField>(app, ts, name, SETTINGS_ADDRESS(ipxGameName), 0, kNameLength);
    if (Item create = fe::findItem(guest, menu, kCreateGame))
        create.setOnClick(kCreateGameClicked);
    if (Item join = fe::findItem(guest, menu, kJoinGame))
    {
        join.setOnClick(kJoinListedGame);
        showJoinable(guest, join);
    }
    if (Item refresh = fe::findItem(guest, menu, kRefreshList))
    {
        refresh.setOnClick(kRefreshClicked);
        guest.setDword(kNextRefresh, 0);
    }
    if (Item back = fe::findItem(guest, menu, kEscape))
        back.setOnClick(kBackClicked);
    if (gameSettings(guest).installIsRemote != 0)
        greyIfThere(guest, menu, kCreateGame);
    return 0;
}

/* ---------------------------------------------------------------------------
 * What the menus call of the network's session: the connections there are, the
 * games on them, a game created or joined, a modem or a cable started
 * ------------------------------------------------------------------------- */

namespace
{

// The kinds of connection the machine has: at most 7 of these.
constexpr nfs::u32 kConnections = 0x55e724;
constexpr nfs::u32 kConnectionSize = 44;
constexpr nfs::u32 kConnectionNameEnd = 0xe;      // byte: 14 characters at most
constexpr nfs::u32 kConnectionKind = 0x10;        // 1 IPX, 2 TCP/IP, 3 RaceNet
constexpr nfs::u32 kConnectionListed = 0x14;      // nonzero: the comm library told of it (sub_49f200)
constexpr nfs::u32 kConnectionGuid = 0x18;        // 16 bytes
constexpr nfs::u32 kConnectionCount = 0x55e718;
constexpr std::int32_t kMostConnections = 7;
constexpr nfs::u32 kCurrentConnection = 0x55e71c; // the kind open, or 0
constexpr nfs::u32 kGatherFound = 0x55e720;
constexpr nfs::u32 kConnectionsLock = 0x55e864;
constexpr nfs::u32 kGatherGuid = 0x55e878;
constexpr nfs::u32 kComm = 0x7a0390;              // the comm library's object
constexpr nfs::u32 kProtocolGuid = 0x55e868;
constexpr std::uint32_t kKindIpx = 1, kKindTcp = 2;
constexpr std::uint32_t kGameBaud = 0x2580, kGameBlocks = 0x64, kGameSlots = 8;
constexpr GuestString kTcpName{ 0x53c6d0, "TCP" };
constexpr GuestString kIpxName{ 0x53c6d4, "IPX" };
constexpr GuestString kGatherName{ 0x53c6d8, "TCP gather" };
constexpr GuestString kDirectPlay{ 0x53c6e4, "DirectPlay" };
constexpr GuestString kDirectPlayIpx{ 0x53c6f0, "DirectPlay IPX" };
constexpr GuestString kDirectPlayTcp{ 0x53c700, "DirectPlay TCP" };
constexpr GuestString kPeerNameFormat{ 0x53c710, "P %d" };

// The games found: a count and the list, 64 bytes each -- the name, four words
// from a byte each of the session's, the address.
constexpr nfs::u32 kSessionCount = 0x55e858;      // int16; kGameList points here
constexpr nfs::u32 kSessionList = 0x55e85c;
constexpr nfs::u32 kSessionStore = 0x79fce0;
constexpr std::int16_t kMostSessions = 0x14;
constexpr nfs::u32 kSessionWords = 0x1c;
constexpr nfs::u32 kSessionAddress = 0x2c;
constexpr nfs::u32 kPeerAddressSize = 0x14;

// A new game's state.
constexpr nfs::u32 kConnectionState = 0x7a22e0;   // 1 a modem, 2 a cable, 3 a network game under way
constexpr nfs::u32 kSessionSetUp = 0x7a2214;      // byte
constexpr nfs::u32 kHostName = 0x7a2215;
constexpr nfs::u32 kSessionPlayers = 0x7a2276;    // int16
constexpr nfs::u32 kMachineNumber = 0x7a2278;     // int16
constexpr nfs::u32 kMachineSlots = 0x7a227a;      // int16, 8 of them
constexpr nfs::u32 kRosterHere = 0x7a2296;        // int16: the host has told us who plays
constexpr nfs::u32 kHandshake = 0x7a22a2;         // int16: the other end's hello came
constexpr nfs::u32 kConnectionLost = 0x7a22dc;    // int16
constexpr nfs::u32 kConnectionLostToo = 0x7a22de; // int16
constexpr nfs::u32 kHostsKind = 0x7a228c;         // what the host has for kNetworkKind
constexpr nfs::u32 kCommName = 0x7a043c;
constexpr nfs::u32 kCommBlocks = 0x7a046c;
constexpr nfs::u32 kCommJoined = 0x7a0434;
constexpr nfs::u32 kEndpoint = 0x7a03ac;
constexpr nfs::u32 kConnectionRecords = 0x7a22e8; // 7, 108 bytes each
constexpr nfs::u32 kNetworkOpen = 0x55e860;
constexpr nfs::u32 kCommWaitBlocks = 0x5643f4;
constexpr nfs::u32 kQuit = 0x564819;              // byte: the game is asked to stop

// Peers waiting to be let in.
constexpr nfs::u32 kAdmissionsClosed = 0x55e710;
constexpr nfs::u32 kPeersKnown = 0x7a01e0;
constexpr nfs::u32 kQueueHead = 0x7a01e4;
constexpr nfs::u32 kQueueTail = 0x7a01e8;
constexpr nfs::u32 kQueueLength = 0x7a01ec;
constexpr nfs::u32 kQueue = 0x7a01f0;             // 6, 31 bytes each: a name of 11, an address
constexpr nfs::u32 kQueuedSize = 31, kQueuedAddress = 0xb;
constexpr std::int32_t kMostQueued = 6, kMostPeers = 7;
constexpr nfs::u32 kPeerSlots = 0x7a02e8;         // 7 pointers
constexpr nfs::u32 kPeerStore = 0x7a0304;         // 7 addresses

// The game's and the comm library's.
constexpr nfs::u32 kSessionIndex = 0x49f1c0;
constexpr nfs::u32 kCommSamePeer = 0x4f5040;
constexpr nfs::u32 kCommRestore = 0x4f5280;
constexpr nfs::u32 kStrcmp = 0x4ee310;
constexpr nfs::u32 kStrstr = 0x4e0820;
constexpr nfs::u32 kStrncpy = 0x4e0e30;
constexpr nfs::u32 kFillZero = 0x4e070c;
constexpr nfs::u32 kResetNetwork = 0x4a6390;
constexpr nfs::u32 kEndNetworkGame = 0x49eed0;
constexpr nfs::u32 kCloseNetwork = 0x4a65e0;
// This group's, handed to the comm library to call back.
constexpr nfs::u32 kServiceFound = 0x49f200;
constexpr nfs::u32 kSessionFound = 0x49f380;
constexpr nfs::u32 kQueuePeerByName = 0x49f470;
constexpr nfs::u32 kQueuePeer = 0x49f4b0;
// The network's lock.
constexpr nfs::u32 kNetworkLock = 0x55ecb4;

constexpr std::uint32_t kNoConnection = 0xc2;       // the message: it could not connect
constexpr nfs::u32 kTimerRate = 0x5643d8;

/* `count` bytes, as `rep movsd` and `rep movsb` copy them. */
void copyBytes(Guest& guest, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    nfs::u32 i = 0;
    for (; i + 4 <= count; i += 4)
        guest.setDword(to + i, guest.dword(from + i));
    for (; i < count; ++i)
        guest.setByte(to + i, guest.byte(from + i));
}

nfs::u32 connectionAt(nfs::u32 index)
{
    return kConnections + index * kConnectionSize;
}

}  // namespace

/* sub_49e360: whether the machine has a connection of this kind, the connections
 * looked for if it does not seem to. */
nfs::u32 frontend::connect::connectionAvailable(App* app, ThreadState& ts, u32 kind)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const auto listed = [&] {
        for (std::int32_t i = 0; i < std::int32_t(guest.dword(kConnectionCount)); ++i)
            if (guest.dword(connectionAt(nfs::u32(i)) + kConnectionKind) == kind)
                return true;
        return false;
    };
    if (listed())
        return 1;
    guest::direct<0x49f150, findConnections>(app, ts, 0);
    return listed() ? 1 : 0;
}

/* sub_49f150: the connections looked for, once: Winsock's IPX, its TCP/IP, and
 * what the comm library has. */
nfs::u32 frontend::connect::findConnections(App* app, ThreadState& ts, u32 handed)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    nfs::u32 eax = handed;   // as it came, then as the lock's making leaves it
    if (guest.dword(kConnectionsLock) == 0)
    {
        eax = *fe::direct<0x4eb2e0, ea::newLock>(app, ts);
        guest.setDword(kConnectionsLock, eax);
    }
    if (guest.dword(kConnectionCount) == 0)
    {
        guest.setDword(kNetworkKind, 1);
        fe::direct<0x4f63d0, frontend::netb::ipxOpenWinsock>(app, ts, kIpxName.address);
        if (fe::direct<0x4f7830, frontend::netb::tcpOpenWinsock>(app, ts, kGatherName.address) != 0)
            guest.setDword(kGatherFound, 1);
        return guest::direct<0x49f030, enumerateServices>(app, ts, kGameBaud, kGameBlocks, kGameSlots);
    }
    return eax;
}

/* sub_49f030: the list of connections made anew, TCP/IP's gathering among them if
 * Winsock has it; none without dial-up networking's say-so. */
nfs::u32 frontend::connect::enumerateServices(App* app, ThreadState& ts, u32 baud, u32 blocks, u32 slots)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
    guest.setDword(kConnectionCount, 0);
    const nfs::u32 found = fe::direct<0x4f4c70, frontend::neta::getServices>(app, ts, kServiceFound, baud, blocks, slots);
    if (found == 0)
        return found;
    if (guest.dword(kGatherFound) != 0)
    {
        const nfs::u32 n = guest.dword(kConnectionCount);
        const nfs::u32 connection = connectionAt(n);
        guest.setDword(connection + kConnectionKind, kKindTcp);
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), connection, kTcpName.address, kConnectionNameEnd + 1);
        guest.setByte(connection + kConnectionNameEnd, 0);
        copyBytes(guest, connection + kConnectionGuid, kGatherGuid, 16);
        guest.setDword(connection + kConnectionListed, 0);
        guest.setDword(kConnectionCount, n + 1);
    }
    const nfs::u32 dialUp = fe::direct<0x49f790, frontend::neta::rasAvailable>(app, ts);
    if (dialUp == 0)
        guest.setDword(kConnectionCount, 0);
    return dialUp;
}

/* sub_49f200: one the comm library has, told by name: DirectPlay's IPX and TCP/IP
 * named as such, anything else an IPX of its name. */
nfs::u32 frontend::connect::serviceFound(App* app, ThreadState& ts, u32 guid, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
    if (std::int32_t(guest.dword(kConnectionCount)) >= kMostConnections)
        return 0;
    const nfs::u32 n = guest.dword(kConnectionCount);
    const nfs::u32 connection = connectionAt(n);
    const nfs::u32 nameSize = kConnectionNameEnd + 1;
    if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kDirectPlay.address) != 0)
    {
        if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kIpxName.address) != 0)
            fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), connection, kDirectPlayIpx.address, nameSize);
        else if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kTcpName.address) != 0)
            fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), connection, kDirectPlayTcp.address, nameSize);
    }
    else
    {
        guest.setDword(connection + kConnectionKind, kKindIpx);
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), connection, name, nameSize);
    }
    guest.setByte(connection + kConnectionNameEnd, 0);
    copyBytes(guest, connection + kConnectionGuid, guid, 16);
    guest.setDword(connection + kConnectionListed, 1);
    guest.setDword(kConnectionCount, n + 1);
    return 1;
}

/* sub_49e3d0: the games on the connection of this kind, sorted by name; the
 * connection opened first if another is open, `redraw` run as a system task
 * meanwhile. */
nfs::u32 frontend::connect::listSessions(App* app, ThreadState& ts, u32 lookFor, u32 player, u32 kind, u32 redraw)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x74);
    const nfs::u32 swap = guest.frame();          // 64 bytes
    const nfs::u32 least = guest.frame() + 0x40;  // the least name yet
    const nfs::u32 blank = guest.frame() + 0x4c;  // 10 bytes
    const nfs::u32 none = guest.frame() + 0x70;   // a byte
    guest.setByte(none, guest.byte(0x49e06a));
    guest.setWord(kSessionCount, 0);
    copyBytes(guest, blank, 0x49e060, 10);
    const nfs::u32 index = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), kind);
    const nfs::u32 current = guest.dword(kCurrentConnection);
    if (current != kind)
    {
        if (current != 0)
            fe::direct<0x4f4e60, frontend::neta::killService>(app, ts, kComm);
        guest.setDword(kCurrentConnection, 0);
        if (fe::direct<0x4f4d10, frontend::neta::initService>(app, ts, kComm, 0, kProtocolGuid, connectionAt(index) + kConnectionGuid,
                                           0, blank, player) == 0)
            return kSessionCount;
        guest.setDword(kCurrentConnection, kind);
    }
    guest.setDword(kSessionList, kSessionStore);
    if (redraw != 0)
        fe::direct<0x4e7520, port::stream::scheduleTask>(app, ts, ts.sp - 4, redraw, 4, 0);
    // The service's own lister reads the callback, what to look for and the byte, which
    // NET_getgames hands on.
    if (fe::direct<0x4f4eb0, frontend::neta::getGames>(app, ts, kComm, kSessionFound, lookFor, none) == 0)
    {
        if (redraw != 0)
            fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, ts, redraw);
        return kSessionCount;
    }
    if (redraw != 0)
        fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, ts, redraw);

    const std::int32_t count = std::int16_t(guest.word(kSessionCount));
    if (count <= 1)
        return kSessionCount;
    const auto game = [&](std::int32_t i) { return guest.dword(kSessionList) + nfs::u32(i) * kGameSize; };
    for (std::int32_t i = 0; i < count; ++i)
    {
        std::int32_t chosen = i;
        guest.copyString(least, game(i));
        for (std::int32_t j = i; j < count; ++j)
        {
            if (std::int32_t(fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), least, game(j))) > 0)
            {
                guest.copyString(least, game(j));
                chosen = j;
            }
        }
        if (chosen != i)
        {
            copyBytes(guest, swap, game(i), kGameSize);
            copyBytes(guest, game(i), game(chosen), kGameSize);
            copyBytes(guest, game(chosen), swap, kGameSize);
        }
    }
    return kSessionCount;
}

/* sub_49f380: a game found, told by the comm library: put on the list, unless the
 * list is full or has it. */
nfs::u32 frontend::connect::sessionFound(App* app, ThreadState& ts, u32 comm, u32 session, u32 address)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 8);
    if (std::int16_t(guest.word(kSessionCount)) >= kMostSessions)
        return 0;
    const std::int32_t count = std::int16_t(guest.word(kSessionCount));
    const nfs::u32 list = guest.dword(kSessionList);
    for (std::int32_t i = 0; i < count; ++i)
    {
        const nfs::u32 there = list + nfs::u32(i) * kGameSize + kSessionAddress;
        if (*fe::direct<0x4f5040, ea::netPlayerChecked>(app, ts, comm, address, there) != 0)
        {
            return 1;
        }
    }
    const nfs::u32 game = list + nfs::u32(count) * kGameSize;
    guest.copyString(game, session);
    for (nfs::u32 i = 0; i < 4; ++i)
        guest.setDword(game + kSessionWords + 4 * i, guest.byte(session + i + 0x20));
    copyBytes(guest, game + kSessionAddress, address, kPeerAddressSize);
    const std::uint16_t was = guest.word(kSessionCount);
    guest.setWord(kSessionCount, std::uint16_t(was + 1));
    return 1;
}

/* sub_49e5f0: a game created on the connection of this kind, under `name`, and the
 * network's state made anew for it; on a connection of Winsock's own, on the port
 * the launcher says (the port's network port change). */
nfs::u32 frontend::connect::createSession(App* app, ThreadState& ts, u32 name, u32 kind)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x14);
    const nfs::u32 local = guest.frame();          // 0x14 bytes, handed over as the stack has them
    const nfs::u32 none = guest.frame() + 0x10;    // a byte
    guest.setByte(none, guest.byte(0x49e06b));
    const nfs::u32 index = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), kind);
    guest::direct<0x49f520, clearPeerSlots>(app, ts);
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 1);
    guest.setDword(kConnectionState, 3);
    guest.setWord(kSessionPlayers, 1);
    guest.setByte(kSessionSetUp, 1);
    guest.setWord(kMachineNumber, 0);
    guest.setWord(kMachineSlots, 0);
    guest.copyString(kHostName, name);
    for (nfs::u32 i = 1; i < 8; ++i)
        guest.setWord(kMachineSlots + 2 * i, 0xffff);
    guest::direct<0x49f650, clearQueue>(app, ts, 0);
    guest.copyString(kCommName, name);
    guest.setDword(kCommBlocks, guest.dword(kNetworkKind) == 0 ? 2 : 8);
    if (guest.dword(kCurrentConnection) != 0)
        fe::direct<0x4f4e60, frontend::neta::killService>(app, ts, kComm);
    guest.setDword(kCurrentConnection, 0);
    const nfs::u32 connection = connectionAt(index);
    const bool listed = guest.dword(connection + kConnectionListed) != 0;
    const nfs::u32 port = listed ? 0 : networkPort();
    const nfs::u32 onPeer = listed ? 0 : kQueuePeer;
    if (fe::direct<0x4f4d10, frontend::neta::initService>(app, ts, kComm, onPeer, kProtocolGuid, connection + kConnectionGuid,
                                       port, local, name) == 0)
    {
        fe::messageBox(guest, kNoConnection);
        directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 0);
        return 0;
    }
    guest.setDword(kCurrentConnection, kind);
    fe::direct<0x4a6500, frontend::neta::sessionReset>(app, ts);
    fe::direct<0x42d790, raceload::messagesOpened>(app, ts);
    if (guest.dword(connection + kConnectionListed) != 0)
    {
        guest.setDword(kCommWaitBlocks, 0x10);
        // The service's own maker reads the name, the byte and the handler, which NET_newgame hands
        // on.
        const nfs::u32 created =
            fe::direct<0x4f4f10, frontend::neta::newGame>(app, ts, kComm, kCommName, none, kQueuePeerByName);
        guest.setDword(kCommWaitBlocks, 0);
        if (created == 0)
        {
            fe::messageBox(guest, kNoConnection);
            directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
            fe::direct<0x42d7f0, raceload::closeAllMessages>(app, ts);
            fe::direct<0x49a4d0, frontend::glue::freeNetworkLocks>(app, ts);
            directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 0);
            return 0;
        }
    }
    guest.setDword(kNetworkOpen, 1);
    return 1;
}

/* sub_49e800: the game at `endpoint` joined: the comm library told, and the wait
 * until the host has said who plays and that the versions agree. */
nfs::u32 frontend::connect::joinSession(App* app, ThreadState& ts, u32 endpoint)
{
    constexpr std::uint16_t kNotYetANumber = 0xfffe;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 4);
    const nfs::u32 none = guest.frame();    // a byte
    guest.setByte(none, guest.byte(0x49e06c));
    const nfs::u32 index = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), guest.dword(kCurrentConnection));
    directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 1);
    guest.setByte(kSessionSetUp, 1);
    guest.setWord(kMachineNumber, kNotYetANumber);
    guest.setWord(kNetworkHost, 0);
    guest.setDword(kConnectionState, 3);

    const auto lost = [&] {
        fe::messageBox(guest, kNoConnection);
        guest.setWord(kConnectionLost, 1);
        directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
        guest.setWord(kConnectionLost, 0);
        directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.ts());
        return nfs::u32(0);
    };

    if (guest.dword(connectionAt(index) + kConnectionListed) != 0)
    {
        // The service's own joiner reads the endpoint and the byte, which NET_setgame hands on.
        if (std::int32_t(fe::direct<0x4f4ed0, frontend::neta::setGame>(app, ts, kComm, endpoint, none, index)) <= 0)
        {
            fe::messageBox(guest, kNoConnection);
            directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 0);
            return 0;
        }
        while (guest.dword(kCommJoined) == 0)
        {
            if (fe::direct<0x4a65b0, frontend::neta::hostGone>(app, ts) != 0)
            {
                fe::direct<0x49eed0, ea::networkGameEnded>(app, ts, 0);
                directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 0);
                return 0;
            }
        }
    }
    fe::direct<0x4a6500, frontend::neta::sessionReset>(app, ts);
    const nfs::u32 record = guest::direct<0x49efb0, setUpEndpoint>(app, ts, kEndpoint, 0);
    if (fe::direct<0x4f5850, frontend::neta::sendQueued>(app, ts, record, 0xd2, 7, kComm) == 0)
    {
        fe::messageBox(guest, kNoConnection);
        guest.setWord(kConnectionLost, 1);
        fe::direct<0x49eed0, ea::networkGameEnded>(app, ts, 0);
        directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
        guest.setWord(kConnectionLost, 0);
        fe::direct<0x49a4d0, frontend::glue::freeNetworkLocks>(app, ts);
        directKept<0x4a6390, ea::networkReset, 1>(guest.app(), guest.ts(), 0);
        return 0;
    }
    while (guest.word(kRosterHere) == 0)
    {
        if (fe::direct<0x4a65b0, frontend::neta::hostGone>(app, ts) != 0 || guest.word(kConnectionLost) != 0)
            return lost();
    }
    guest.setDword(kNetworkKind, guest.dword(kHostsKind));
    fe::direct<0x42d790, raceload::messagesOpened>(app, ts);
    if (fe::direct<0x4a6ce0, frontend::neta::compareVersions>(app, ts, 0) == 0)
        return lost();
    return 1;
}

/* sub_49e9d0: the TCP/IP connection opened on the launcher's port, if it is not
 * open; a message if it cannot be. */
nfs::u32 frontend::connect::useTcp(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xc);
    const nfs::u32 blank = guest.frame();    // 10 bytes
    copyBytes(guest, blank, 0x49e06d, 10);
    const nfs::u32 current = guest.dword(kCurrentConnection);
    if (current == kKindTcp)
        return 1;
    if (current != 0)
        fe::direct<0x4f4e60, frontend::neta::killService>(app, ts, kComm);
    guest.setDword(kCurrentConnection, 0);
    const nfs::u32 index = directKept<0x49f1c0, ea::sessionIndexOf, 1>(guest.app(), guest.ts(), kKindTcp);
    if (fe::direct<0x4f4d10, frontend::neta::initService>(app, ts, kComm, 0, kProtocolGuid, connectionAt(index) + kConnectionGuid,
                                       networkPort(), blank, player) == 0)
    {
        fe::messageBox(guest, kNoConnection);
        return 0;
    }
    guest.setDword(kCurrentConnection, kKindTcp);
    return 1;
}

/* sub_49ea70: a host's name made an address on the launcher's port, with TCP/IP
 * open; a message if it is no host. */
nfs::u32 frontend::connect::findAddress(App* app, ThreadState& ts, u32 host, u32 address)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (guest.dword(kCurrentConnection) != kKindTcp)
        return 0;
    if (host == 0)
        return 0;
    // The service's own lookup reads the host, the port and where the address goes, which its call
    // through the table hands on.
    if (fe::direct<0x4f5050, frontend::neta::servicePollCall>(app, ts, kComm, host, networkPort(), kEndpoint) != 0)
    {
        copyBytes(guest, address, kEndpoint, kPeerAddressSize);
        return 1;
    }
    fe::messageBox(guest, kNoConnection);
    return 0;
}

/* sub_49efb0: a connection's record: its three timeouts, a quarter of the timer's
 * rate, and the address it goes to. */
nfs::u32 frontend::connect::setUpEndpoint(App* app, ThreadState& ts, u32 address, u32 connection)
{
    Guest guest(app, ts);
    const nfs::u32 record = kConnectionRecords + nfs::u32(std::int32_t(std::int16_t(connection))) * 108;
    const std::int32_t rate = std::int32_t(guest.dword(kTimerRate));
    const std::uint16_t timeout = std::uint16_t(rate / 4);
    guest.setWord(record + 0x2a, timeout);
    guest.setWord(record + 0x2c, timeout);
    guest.setWord(record + 0x2e, timeout);
    copyBytes(guest, record + 0x34, address, kPeerAddressSize);
    if (guest.dword(kNetworkKind) == 2)
        guest.setDword(record + 0x24, 1);
    return record;
}

/* sub_49f470: a peer with a name of its own, in the queue to be let in. */
nfs::u32 frontend::connect::queuePeerByName(App* app, ThreadState& ts, u32 peer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (guest.dword(kAdmissionsClosed) != 0)
        return 0;
    const std::int32_t known = std::int32_t(guest.dword(kPeersKnown));
    if (known >= kMostPeers)
        return 0;
    guest.setDword(kPeersKnown, nfs::u32(known) + 1);
    guest::direct<0x49f6a0, enqueuePeer>(app, ts, peer + 0x48, peer);
    return 1;
}

/* sub_49f4b0: a peer at an address, in the queue under a name made up, "P 2",
 * unless it is known already. */
nfs::u32 frontend::connect::queuePeer(App* app, ThreadState& ts, u32 address)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0xc);
    if (guest.dword(kAdmissionsClosed) != 0 || std::int32_t(guest.dword(kPeersKnown)) >= kMostPeers)
        return 0;
    if (guest::direct<0x49f5c0, rememberPeer>(app, ts, address) == 0)
        return 1;
    const nfs::u32 name = guest.frame();    // 12 bytes
    const nfs::u32 known = guest.dword(kPeersKnown) + 1;
    guest.setDword(kPeersKnown, known);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kPeerNameFormat.address, guest::Words{known});
    guest::direct<0x49f6a0, enqueuePeer>(app, ts, name, address);
    return 1;
}

/* sub_49f5c0: a peer's address kept, in the first of seven free; 0 if it is there
 * or none is free. */
nfs::u32 frontend::connect::rememberPeer(App* app, ThreadState& ts, u32 address)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    for (nfs::u32 i = 0; i < nfs::u32(kMostPeers); ++i)
    {
        const nfs::u32 there = guest.dword(kPeerSlots + 4 * i);
        if (there != 0 && *fe::direct<0x4f5040, ea::netPlayerChecked>(app, ts, kComm, address, there) != 0)
            return 0;
    }
    for (nfs::u32 i = 0; i < nfs::u32(kMostPeers); ++i)
    {
        if (guest.dword(kPeerSlots + 4 * i) == 0)
        {
            const nfs::u32 slot = kPeerStore + i * kPeerAddressSize;
            guest.setDword(kPeerSlots + 4 * i, slot);
            copyBytes(guest, slot, address, kPeerAddressSize);
            return 1;
        }
    }
    return 0;
}

/* sub_49f6a0: a peer's name and address, at the queue's tail; what the division
 * leaves in eax and edx goes on to the caller. */
nfs::u32 frontend::connect::enqueuePeer(App* app, ThreadState& ts, u32 name, u32 address)
{
    Guest guest(app, ts);
    const nfs::u32 tail = guest.dword(kQueueTail);
    const nfs::u32 place = kQueue + tail * kQueuedSize;
    guest.copyString(place, name);
    copyBytes(guest, place + kQueuedAddress, address, kPeerAddressSize);
    const std::int32_t next = std::int32_t(tail + 1);
    guest.setDword(kQueueLength, guest.dword(kQueueLength) + 1);
    guest.setDword(kQueueTail, nfs::u32(next % kMostQueued));
    return nfs::u32(next / kMostQueued);
}

/* sub_4c41d0: the modems' names, at most 15, in `list`: a count and 30 bytes each. */
nfs::u32 frontend::connect::listModems(App* app, ThreadState& ts, u32 list)
{
    constexpr std::int32_t kMostModems = 15;
    constexpr nfs::u32 kNameSize = 30;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (list == 0)
        return 0;
    nfs::u32 last = fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), list, 0x1c8u);
    for (;;)
    {
        const std::int32_t n = std::int32_t(guest.dword(list));
        if (n >= kMostModems)
            return last;
        const nfs::u32 name = fe::direct<0x4fa5f0, frontend::netb::modemName>(app, ts, nfs::u32(n));
        if (name == 0)
            return 0;
        guest.copyString(list + 4 + nfs::u32(n) * kNameSize, name);
        guest.setDword(list, guest.dword(list) + 1);
        // eax as the copy leaves it: the offset, its low byte the terminator.
        last = (nfs::u32(n) * kNameSize) & 0xffffff00;
    }
}

/* sub_4c4230: a modem's or a cable's link set up: its port, the phone number if it
 * dials, its timeouts from the timer's rate. */
nfs::u32 frontend::connect::setUpLink(App* app, ThreadState& ts, u32 service)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    fe::direct<0x4a6500, frontend::neta::sessionReset>(app, ts);
    fe::direct<0x42d790, raceload::messagesOpened>(app, ts);
    guest.setWord(kSessionPlayers, 2);
    if (guest.dword(kConnectionState) == 1)
    {
        guest.setDword(0x7a234c, 0);
        guest.setDword(0x7a2350, 0);
        guest.setDword(0x7a2324, service);
        if (guest.word(kNetworkHost) == 0)
            guest.setDword(0x7a2350, SETTINGS_ADDRESS(phoneNumber));
    }
    else
    {
        guest.setDword(0x7a2320, gameSettings(guest).comPort);
    }
    const std::int32_t rate = std::int32_t(guest.dword(kTimerRate));
    guest.setWord(0x7a2312, std::uint16_t(rate / 16));
    const std::int32_t divided = rate / (guest.dword(kConnectionState) == 1 ? 5 : 10);
    guest.setWord(0x7a2314, std::uint16_t(divided));
    guest.setWord(0x7a2316, std::uint16_t(divided));
    guest.setDword(0x7a232c, 0);
    guest.setDword(0x7a2330, 0);
    guest.setDword(0x7a2334, 3);
    guest.setDword(0x7a2328, 9);
    return 3;
}

/* sub_4c4310: hello said to the other end, its hello awaited; on a cable the later
 * of the two, or the greater name, hosts.  0 if the two are one and the same. */
nfs::u32 frontend::connect::shakeHands(App* app, ThreadState& ts)
{
    constexpr std::uint8_t kHello = 7;
    constexpr nfs::u32 kTheirTicks = 0x7d3654, kTheirName = 0x7d3658;
    constexpr nfs::u32 kOtherEndsName = 0x7a25dc;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    const nfs::u32 packet = guest.frame();  // the kind, the ticks at +4, the name at +8
    const nfs::u32 myName = packet + 8;
    guest.setByte(packet, kHello);
    guest.setDword(packet + 4, guest.dword(kTicks));
    guest.copyString(myName, SETTINGS_ADDRESS(players[0].name));
    fe::direct<0x4a60e0, ea::packetSent>(app, ts, 0, packet, 0x14);
    while (guest.word(kHandshake) == 0)
    {
        if (fe::direct<0x4a65b0, frontend::neta::hostGone>(app, ts) != 0)
            return 0;
    }
    guest.setWord(kHandshake, 0);
    if (guest.dword(kConnectionState) == 2)
    {
        const auto weHost = [&] {
            gameSettings(guest).localPlayer = 0;
            guest.setWord(kMachineNumber, 0);
            guest.setWord(kNetworkHost, 1);
        };
        const auto theyHost = [&] {
            guest.setWord(kNetworkHost, 0);
            gameSettings(guest).localPlayer = 1;
            guest.setWord(kMachineNumber, 1);
        };
        const std::int32_t mine = std::int32_t(guest.dword(packet + 4));
        const std::int32_t theirs = std::int32_t(guest.dword(kTheirTicks));
        if (mine > theirs)
        {
            weHost();
        }
        else if (mine < theirs)
        {
            theyHost();
        }
        else
        {
            const std::int32_t order = std::int32_t(fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), myName, kTheirName));
            if (order == 0)
                return 0;
            if (order < 0)
                theyHost();
            else
                weHost();
        }
    }
    if (guest.word(kNetworkHost) != 0)
    {
        guest.copyString(kOtherEndsName, kTheirName);
        guest.copyString(kHostName, SETTINGS_ADDRESS(players[0].name));
    }
    return 1;
}

/* sub_4c3f90: a modem or the cable started, under a message: the link, the two ends
 * shaking hands, and the host saying who plays; a message if any of it fails. */
nfs::u32 frontend::connect::startConnection(App* app, ThreadState& ts, u32 service)
{
    constexpr nfs::u32 kLines = 0x7d3664;
    constexpr nfs::u32 kMessageTime = 0x14d;
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const auto enterLock = [&] {
        fe::enterLock(guest, ts, guest.dword(kNetworkLock));
    };
    const auto leaveLock = [&] {
        fe::leaveLock(guest, ts, guest.dword(kNetworkLock));
    };
    const nfs::u32 mode = gameSettings(guest).playMode;
    if (mode == 2)
    {
        guest.setDword(kConnectionState, 1);
    }
    else if (mode == 3)
    {
        guest.setDword(kNetworkKind, 0);
        gameSettings(guest).networkGame = 0;
        guest.setDword(kConnectionState, 2);
    }
    putText(guest, kLines, 0xc);
    putText(guest, kLines + 4, 0xb);
    fe::direct<0x444a80, frontend::dialogs::openMessage>(app, ts, 2, kLines, kMessageTime, 1, 0);
    for (int i = 0; i < kMessageRedraws; ++i)
        redrawMenu(guest, ts);
    guest::direct<0x4c4230, setUpLink>(app, ts, service);

    nfs::u32 started = 0;
    const nfs::u32 state = guest.dword(kConnectionState);
    if (state == 1)
    {
        if (guest.word(kNetworkHost) == 0)
            fe::direct<0x4109d0, port::musicStopped>(app, ts, ts.sp - 4, 0);
        started = fe::direct<0x4fa550, frontend::netb::modemOpenNetwork>(app, ts, kConnectionRecords, 0xd2, 7);
        if (guest.word(kNetworkHost) == 0)
            fe::direct<0x4104f0, port::musicStarted>(app, ts, ts.sp - 4, 0x16,
                                                     std::int32_t(gameSettings(guest).menuMusicVolume), 0);
    }
    else if (state == 2)
    {
        started = fe::direct<0x4fa780, frontend::netb::serialOpenNetwork>(app, ts, kConnectionRecords, 0xd2, 7);
    }
    if (started != 0)
    {
        enterLock();
        if (guest::direct<0x4c4310, shakeHands>(app, ts) == 0)
        {
            guest.setWord(kConnectionLost, 1);
            guest.setWord(kConnectionLostToo, 1);
            directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
            leaveLock();
            started = 0;
        }
        else if (guest.word(kNetworkHost) != 0)
        {
            leaveLock();
            fe::direct<0x4a66c0, frontend::neta::sendSetup>(app, ts);
        }
        else
        {
            while (guest.word(kRosterHere) == 0)
            {
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
                if (fe::direct<0x4a65b0, frontend::neta::hostGone>(app, ts) != 0)
                {
                    started = 0;
                    guest.setWord(kConnectionLost, 1);
                    guest.setWord(kConnectionLostToo, 1);
                    directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
                    break;
                }
                if (guest.word(kConnectionLost) != 0)
                {
                    directKept<0x4f5280, ea::connectionsRestored, 1>(guest.app(), guest.ts(), 0);
                    started = 0;
                    break;
                }
            }
            guest.setDword(kNetworkKind, guest.dword(kHostsKind));
            leaveLock();
        }
    }
    if (started == 0)
    {
        while (guest.byte(kQuit) != 0)
            fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        fe::messageBox(guest, kNoConnection);
        directKept<0x4a65e0, ea::networkClosed, 0>(guest.app(), guest.ts());
    }
    return started;
}

/* sub_43dab0: a player's three words as a packet tells them, into its record of
 * 108 bytes. */
nfs::u32 frontend::connect::applyPlayerVector(App* app, ThreadState& ts, u32 packet)
{
    constexpr nfs::u32 kRecordSize = 108;
    Guest guest(app, ts);
    const nfs::u32 record = kRecordSize * guest.byte(packet + 1);
    gameSettings(guest).entryAt(record).digital[0] = guest.dword(packet + 4);
    gameSettings(guest).entryAt(record).digital[1] = guest.dword(packet + 8);
    const nfs::u32 last = guest.dword(packet + 0xc);
    gameSettings(guest).entryAt(record).digital[2] = last;
    return last;
}

/* sub_43f1f0: the screen drawn once, while the games are looked for. */
nfs::u32 frontend::connect::listingRedraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, ts);
    fe::direct<0x44a4f0, port::loop::drawItemsOf>(app, ts, ts.sp - 4,
                                                  fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts));
    fe::direct<0x49c080, frontend::glue::mouseKey>(app, ts);
    fe::direct<kDrawPointer, port::draw2d::pointerDrawn>(app, ts, ts.sp - 4);
    fe::direct<0x4d9140, graphics::flipAndWait>(app, ts);
    return 0;
}

/* sub_4400b0: "refresh": the games looked for again. */
nfs::u32 frontend::connect::refreshClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (const nfs::u32 list = guest.dword(kSelectList))
        guest::direct<0x43f220, refreshGames>(app, ts, list);
    return 0;
}

/* sub_451940: the keys waiting thrown away; `held` is the key now counted as held. */
nfs::u32 frontend::connect::flushKeys(App* app, ThreadState& ts, u32 held)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest.setDword(kHeldKey, held);
    while (fe::direct<0x4cc970, frontend::glue::nextKey>(app, ts) != 0)
    {
    }
    return 0;
}

/* sub_458450: an edit box's text, its length and its third argument, if the box
 * is on the screen now. */
nfs::u32 frontend::connect::setEditField(App* app, ThreadState& ts, u32 field, u32 text, u32 third, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (fe::direct<0x44a660, frontend::loop::shownNow>(app, ts, field) == 0)
        return 0;
    guest.setDword(field + kEditThird, third);
    guest.setWord(field + kEditLength, std::uint16_t(length));
    guest.setDword(field + kEditText, text);
    return 1;
}

/* sub_49c0e0: the mouse's pointer: -1 back to the one before, -3 as it is, and any
 * other number one of 13 shapes, -2 none; its result what the division leaves. */
nfs::u32 frontend::connect::setPointer(App* app, ThreadState& ts, u32 asked)
{
    constexpr nfs::u32 kPointer = 0x563d3c, kPointerBefore = 0x563d40;
    constexpr std::int32_t kShapes = 13, kBack = -1, kKeep = -3;
    Guest guest(app, ts);
    const std::int32_t shape = std::int32_t(asked);
    if (shape == kKeep)
        return asked;
    if (shape == kBack)
    {
        guest.setDword(kPointer, guest.dword(kPointerBefore));
        return asked;
    }
    guest.setDword(kPointerBefore, guest.dword(kPointer));
    guest.setDword(kPointer, nfs::u32(shape % kShapes));
    return nfs::u32(shape / kShapes);
}

/* sub_49f520: the seven kept addresses forgotten. */
nfs::u32 frontend::connect::clearPeerSlots(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    for (nfs::u32 i = 0; i < nfs::u32(kMostPeers); ++i)
        guest.setDword(kPeerSlots + 4 * i, 0);
    return nfs::u32(kMostPeers);
}

/* sub_49f650: the queue of peers emptied. */
nfs::u32 frontend::connect::clearQueue(App* app, ThreadState& ts, u32 handed)
{
    Guest guest(app, ts);
    guest.setDword(kQueueHead, 0);
    guest.setDword(kQueueTail, 0);
    guest.setDword(kQueueLength, 0);
    guest.setDword(kPeersKnown, 0);
    return handed;
}

namespace
{

const GuestString kConnectEnterStrings[] = { kRaceNet, kIpx, kTcp, kModem, kSerial };
const GuestString kIpxFrameStrings[] = { kJoinGame };
const GuestString kIpxEnterStrings[] = { kGameName, kCreateGame, kJoinGame, kRefreshList, kEscape };
const GuestString kRefreshStrings[] = { kAllocName };
const GuestString kFindConnectionsStrings[] = { kIpxName, kGatherName };
const GuestString kEnumerateStrings[] = { kTcpName };
const GuestString kServiceStrings[] = { kDirectPlay, kIpxName, kTcpName, kDirectPlayIpx, kDirectPlayTcp };
const GuestString kQueueStrings[] = { kPeerNameFormat };
const GuestString kModemLabelStrings[] = { kColonSpace };
const GuestString kComPortStrings[] = { kComPortFormat };
const GuestString kProtocolStrings[] = { kGameName };

constexpr std::uint8_t kBCD = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kBCDS = kBCD | fe::kRestoreEsi;
constexpr std::uint8_t kBCSD = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

}

/* Each stand-in: the game's function, what its ret pops, the strings it hands
 * to the game and the registers it pushes; its frame is its core's. */

nfs::u32 frontend::connectApplyPlayerVector(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43dab0", 0, 0, nullptr, 0, 0 };
    const nfs::u32 packet = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::applyPlayerVector(app, ts, packet); });
}

nfs::u32 frontend::connectListingRedraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f1f0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::listingRedraw(app, ts); });
}

nfs::u32 frontend::ipxRefreshGames(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f220", 0, 0, kRefreshStrings, std::size(kRefreshStrings), kBCDS };
    const nfs::u32 list = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::refreshGames(app, ts, list); });
}

nfs::u32 frontend::connectOpenProtocolList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f320", 0, 0, nullptr, 0 };
    const nfs::u32 mode = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::openProtocolList(app, ts, mode); });
}

nfs::u32 frontend::connectOpenAddressDialog(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f4e0", 0, 0, nullptr, 0, kBCD };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::openAddressDialog(app, ts); });
}

nfs::u32 frontend::connectModemLabel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f550", 0, 0, kModemLabelStrings, std::size(kModemLabelStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::modemLabel(app, ts); });
}

nfs::u32 frontend::connectComPortLabel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f610", 0, 0, kComPortStrings, std::size(kComPortStrings), 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::comPortLabel(app, ts); });
}

nfs::u32 frontend::ipxGameNameAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f640", 0, 0, nullptr, 0, 0 };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::gameNameAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectAddressAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f680", 0, 0, nullptr, 0, kBCSD };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::addressAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectTcpAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f760", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::tcpAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectProtocolListAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f7e0", 0, 0, kProtocolStrings, std::size(kProtocolStrings), kBCSD };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::protocolListAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectSerialAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43f960", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::serialAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectPhoneAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fa00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::phoneAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectModemAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43faf0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::modemAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectRaceNetMissingAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fc20", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::raceNetMissingAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectRaceNetAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fc50", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::raceNetAnswer(app, ts, answer, closes); });
}

nfs::u32 frontend::connectRaceNetClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fd10", 0, 0, nullptr, 0, kBCDS };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::raceNetClicked(app, ts); });
}

nfs::u32 frontend::connectTcpClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fe50", 0, 0, nullptr, 0, kBCDS };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::tcpClicked(app, ts); });
}

nfs::u32 frontend::connectSerialClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43fee0", 0, 0, nullptr, 0, kBCD };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::serialClicked(app, ts); });
}

nfs::u32 frontend::connectModemClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43ff50", 0, 0, nullptr, 0, kBCDS };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::modemClicked(app, ts); });
}

nfs::u32 frontend::ipxCreateGameClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440040", 0, 0, nullptr, 0, kBCD };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::createGameClicked(app, ts); });
}

nfs::u32 frontend::ipxRefreshClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4400b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::refreshClicked(app, ts); });
}

nfs::u32 frontend::connectOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4405c0", 0, 0, kConnectEnterStrings, std::size(kConnectEnterStrings), kBCD };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::connectEnter(app, ts, menu); });
}

nfs::u32 frontend::connectOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440730", 0, 0, kConnectEnterStrings, std::size(kConnectEnterStrings), kBCDS };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::connectExit(app, ts, menu); });
}

nfs::u32 frontend::ipxOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440810", 0, 0, kIpxFrameStrings, std::size(kIpxFrameStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::ipxFrame(app, ts, menu); });
}

nfs::u32 frontend::ipxOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440890", 0, 0, kIpxEnterStrings, std::size(kIpxEnterStrings), kBCDS };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::ipxEnter(app, ts, menu); });
}

/* sub_440960: ipx as it closes: nothing to undo. */
nfs::u32 frontend::ipxOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_440960", 0, 4, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::connectFlushKeys(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_451940", 0, 0, nullptr, 0, 0 };
    const nfs::u32 held = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::flushKeys(app, ts, held); });
}

nfs::u32 frontend::connectSetEditField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_458450", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 field = args.eax, text = args.edx, third = args.ebx, length = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::setEditField(app, ts, field, text, third, length); });
}

nfs::u32 frontend::connectSetPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49c0e0", 0, 0, nullptr, 0, kBCD };
    const nfs::u32 asked = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::setPointer(app, ts, asked); });
}

nfs::u32 frontend::connectConnectionAvailable(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e360", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const nfs::u32 kind = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::connectionAvailable(app, ts, kind); });
}

nfs::u32 frontend::connectListSessions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e3d0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 lookFor = args.eax, player = args.edx, kind = args.ebx, redraw = args.ecx;
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return connect::listSessions(app, ts, lookFor, player, kind, redraw); });
    return args.eax;
}

nfs::u32 frontend::connectCreateSession(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e5f0", 0, 0, nullptr, 0, kBCSD };
    const nfs::u32 name = args.eax, kind = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::createSession(app, ts, name, kind); });
}

nfs::u32 frontend::connectJoinSession(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e800", 0, 0, nullptr, 0 };
    const nfs::u32 endpoint = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::joinSession(app, ts, endpoint); });
}

nfs::u32 frontend::connectUseTcp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49e9d0", 0, 0, nullptr, 0 };
    const nfs::u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::useTcp(app, ts, player); });
}

nfs::u32 frontend::connectFindAddress(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49ea70", 0, 0, nullptr, 0, kBCSD };
    const nfs::u32 host = args.eax, address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::findAddress(app, ts, host, address); });
}

nfs::u32 frontend::connectSetUpEndpoint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49efb0", 0, 0, nullptr, 0, kBCSD };
    const nfs::u32 address = args.eax, connection = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::setUpEndpoint(app, ts, address, connection); });
}

nfs::u32 frontend::connectEnumerateServices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f030", 0, 0, kEnumerateStrings, std::size(kEnumerateStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 baud = args.eax, blocks = args.edx, slots = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::enumerateServices(app, ts, baud, blocks, slots); });
}

nfs::u32 frontend::connectFindConnections(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f150", 0, 0, kFindConnectionsStrings, std::size(kFindConnectionsStrings), kBCDS };
    const nfs::u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::findConnections(app, ts, handed); });
}

nfs::u32 frontend::connectServiceFound(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f200", 0, 0, kServiceStrings, std::size(kServiceStrings),
                          fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 guid = args.eax, name = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::serviceFound(app, ts, guid, name); });
}

nfs::u32 frontend::connectSessionFound(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f380", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 comm = args.eax, session = args.edx, address = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::sessionFound(app, ts, comm, session, address); });
}

nfs::u32 frontend::connectQueuePeerByName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f470", 0, 0, nullptr, 0, fe::kRestoreEsi };
    const nfs::u32 peer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::queuePeerByName(app, ts, peer); });
}

nfs::u32 frontend::connectQueuePeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f4b0", 0, 0, kQueueStrings, std::size(kQueueStrings), fe::kRestoreEsi };
    const nfs::u32 address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::queuePeer(app, ts, address); });
}

nfs::u32 frontend::connectClearPeerSlots(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f520", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::clearPeerSlots(app, ts); });
}

nfs::u32 frontend::connectRememberPeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f5c0", 0, 0, nullptr, 0 };
    const nfs::u32 address = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::rememberPeer(app, ts, address); });
}

nfs::u32 frontend::connectClearQueue(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f650", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const nfs::u32 handed = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::clearQueue(app, ts, handed); });
}

nfs::u32 frontend::connectEnqueuePeer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49f6a0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 name = args.eax, address = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::enqueuePeer(app, ts, name, address); });
}

nfs::u32 frontend::connectStartConnection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c3f90", 0, 0, nullptr, 0 };
    const nfs::u32 service = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::startConnection(app, ts, service); });
}

nfs::u32 frontend::connectListModems(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c41d0", 0, 0, nullptr, 0,
                          fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const nfs::u32 list = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::listModems(app, ts, list); });
}

nfs::u32 frontend::connectSetUpLink(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4230", 0, 0, nullptr, 0, kBCDS };
    const nfs::u32 service = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::setUpLink(app, ts, service); });
}

nfs::u32 frontend::connectShakeHands(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4310", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return connect::shakeHands(app, ts); });
}

}
