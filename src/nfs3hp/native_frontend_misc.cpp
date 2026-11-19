#include "native_frontend_misc.h"
#include "native_load_saves.h"
#include "native_carstate.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"

#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_menub.h"
#include "native_frontend_neta.h"
#include "native_frontend_pause.h"
#include "native_load_cardb.h"
#include "port_buttons.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_menus.h"
#include "port_shapes.h"
#include "port_sound_speech.h"
#include "port_text.h"
#include <nfs3hp.h>
#include <array>
#include <iterator>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

/* The screens that are neither the race's pause, nor the cars', nor the network's,
 * decompiled:
 *
 *   racetype   the kind of race, of six;
 *   level      LEVEL.MNU: the cops' assists, a duel's own items;
 *   camera     CAMERA.MNU: the views the camera cycles through;
 *   setup      SETUP.MNU: the way in, one player or two, the web site's dialogs;
 *   loadsave   LOADSAVE.MNU: the list of kept replays, tournaments, knockouts
 *              and ghosts, with a description of the one selected;
 *   records    RECORDS.MNU: a track's records, and deleting them;
 *   postgame   POSTGAME.MNU: the sheets after a race, what can be saved of it,
 *              the awards it won, and the way back;
 *   credits    the credits' pages, sliding across; credits1 the showcase's.
 *
 * With them the race's results, which only these screens sort and look up: a
 * table of 16 participants for each of two groups (group 0 the race, group 1
 * the fastest laps) and the order they are sorted in at 0x5595d8.
 *
 * Checking a kept file's cars and loading a ghost's stay with loading (sub_478090,
 * sub_478100, sub_477b50), the network's way out with the network's code
 * (sub_4a6a10), and the dial-up check with Windows' libraries (sub_49f790,
 * sub_49f880): called by address.
 *
 * Plan step 5: each function a typed core in frontend::misc (native_frontend_misc.h), in a frame
 * of its own where the game's function has one and calls anything (fe::Frame: what it calls runs
 * at the stack addresses it did), and a thunk under the game's name for the calls by address.
 * A core of this file calls another as C++ (guest::direct), and so the typed cores of the other
 * menu files, the port's and the libraries' (fe::direct); the rest of the game by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;

/* The question as fe::openQuestion opens it (sub_4446d0), with what it answers. */
nfs::u32 openQuestionAnswer(Guest& guest, const fe::Question& question)
{
    return fe::direct<0x4446d0, frontend::dialogs::openQuestion>(
        guest.app(), guest.ts(), question.lineCount, question.lines, question.buttonCount, question.buttons,
        question.buttonHelp, question.focus, question.column, question.callback);
}
using fe::GuestString;
using fe::Item;

constexpr nfs::u32 kInRace = 0x559234;
constexpr nfs::u32 kNetworkConnected = 0x7a22e0;
constexpr nfs::u32 kNetworkHost = 0x7a22da;      // int16
constexpr nfs::u32 kNetworkLeaving = 0x7a22dc;   // int16: the game is over for this machine
constexpr nfs::u32 kNetworkLost = 0x7a22de;      // int16: to be told as the next screen opens
constexpr nfs::u32 kDeveloperFlags = 0x55eb2e;   // byte
constexpr std::uint8_t kPracticeOnly = 0x20;
constexpr std::uint8_t kNoFinishers = 0x40;

constexpr nfs::u32 kOnDraw = 0x2c;               // an item's drawing, set by code
constexpr nfs::u32 kOnDrawBox = 0x54;            // a box's drawing
constexpr nfs::u32 kListOnMove = 0x74;           // a list's: its selection moved
constexpr nfs::u32 kListCount = 0x48;
constexpr nfs::u32 kListSelected = 0x4c;
constexpr nfs::u32 kListEntries = 0x70;          // char*, 8 bytes each

// The game's.
constexpr nfs::u32 kSprintf = 0x4df690;
constexpr nfs::u32 kCopy = 0x4ea4f0;             // eax to edx, ebx bytes
constexpr nfs::u32 kFillZero = 0x4e070c;         // eax, edx bytes
constexpr nfs::u32 kMemset = 0x4e0640;           // eax, the byte edx, ebx bytes
constexpr nfs::u32 kCodelinkMatches = 0x4429c0;
constexpr nfs::u32 kCloseNetwork = 0x4a65e0;
constexpr nfs::u32 kStrcmp = 0x4ee310;
constexpr nfs::u32 kStricmp = 0x4e9ec0;
constexpr nfs::u32 kFopen = 0x4edfe8;
constexpr nfs::u32 kFclose = 0x4ee100;

constexpr std::uint32_t kShortMessage = 0x200;     // a message's time, in ticks
constexpr std::uint32_t kLongMessage = 0x400;

/* A message of `lineCount` lines up for `time` ticks: the dialogs' (keys 1, no callback). */
nfs::u32 openMessage(nfs::App* app, nfs::ThreadState& ts, nfs::u32 lineCount, nfs::u32 lines,
                       nfs::u32 time)
{
    return fe::direct<0x444a80, frontend::dialogs::openMessage>(app, ts, lineCount, lines, time, 1, 0);
}

/* Greyed out and hidden, the state and flags bytes as one word. */
void greyAndHide(Guest& guest, nfs::u32 item)
{
    guest.setWord(item + fe::item::kState, std::uint16_t(guest.word(item + fe::item::kState) | 0x1001));
}

void showAgain(Guest& guest, nfs::u32 item)
{
    guest.setWord(item + fe::item::kState, std::uint16_t(guest.word(item + fe::item::kState) & 0xeffe));
}

std::int32_t signedWord(const Guest& guest, nfs::u32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t signedDword(const Guest& guest, nfs::u32 address)
{
    return std::int32_t(guest.dword(address));
}

/* ---------------------------------------------------------------------------
 * level
 * ------------------------------------------------------------------------- */

constexpr GuestString kCopAssists[] = { { 0x53a75c, "copassist1" }, { 0x53a768, "copassist2" } };
constexpr GuestString kDuelDisabled[] = {
    { 0x53a774, "disable1" }, { 0x53a780, "disable2" }, { 0x53a78c, "disable3" },  { 0x53a798, "disable4" },
    { 0x53a7a4, "disable5" }, { 0x53a7b0, "disable6" }, { 0x53a7bc, "disable7" },  { 0x53a7c8, "disable8" },
    { 0x53a7d4, "disable9" }, { 0x53a7e0, "disable10" },
};

}  // namespace

/* sub_45d630, level as it opens: in a duel of one player the cops' assists are
 * open and the ten others closed; else the assists are closed. */
nfs::u32 frontend::misc::levelOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const bool soloDuel = gameSettings(guest).raceType == 3 && gameSettings(guest).playMode == 0;
    for (const GuestString& codelink : kCopAssists)
        if (Item item = fe::findItem(guest, menu, codelink))
            item.setDisabled(!soloDuel);
    if (soloDuel)
        for (const GuestString& codelink : kDuelDisabled)
            if (Item item = fe::findItem(guest, menu, codelink))
                item.setDisabled(true);
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * camera
 * ------------------------------------------------------------------------- */

// The views the cycle holds, on or off, and the copy Cancel puts back.
constexpr nfs::u32 kCameraViewsKept[] = { 0x6788fc, 0x6788f4, 0x6788f8, 0x6788f0 };
constexpr nfs::u32 kCameraMode = 0x5e1054;
constexpr nfs::u32 kDimBackground = 0x451530;    // a float on the stack: a race's pause screen behind
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
constexpr nfs::u32 kCameraEscape = 0x45d840;
constexpr nfs::u32 kBuildCameraCycle = 0x423a60;
constexpr nfs::u32 kRememberMode = 0x423ae0;     // eax the player, edx the mode
constexpr nfs::u32 kSetCameraMode = 0x4238b0;    // likewise
constexpr GuestString kCameraEscapeItem{ 0x53a7ec, "escape" };
constexpr GuestString kCameraAssists{ 0x53a7f4, "assists" };
constexpr GuestString kCameraCancel{ 0x53a7fc, "CANCEL_CAMERA_OPTIONS" };

/* sub_45d920, camera as it opens: the views kept for Cancel. */
nfs::u32 cameraOnEnterBody(Guest& guest, nfs::u32 menu)
{
    // The shade where the original pushes it, just under the frame: dimmed rewrites it there.  eax as
    // the caller has it, handed on.
    const nfs::u32 shade = guest.frame() - 4;
    guest.setDword(shade, kFloat0_6);
    fe::direct<kDimBackground, port::text::dimmed>(guest.app(), guest.ts(), shade, 0);  // eax: its result is not read
    for (nfs::u32 i = 0; i < std::size(kCameraViewsKept); ++i)
        guest.setDword(kCameraViewsKept[i], gameSettings(guest).cameraViewsChosen[i]);
    if (Item escape = fe::findItem(guest, menu, kCameraEscapeItem))
        escape.setOnClick(kCameraEscape);
    if (Item assists = fe::findItem(guest, menu, kCameraAssists))
        assists.setText(fe::text(guest, 0x196));
    return 0;
}

/* sub_45d9e0, camera as it closes: by its Cancel the views as they were, else
 * the new ones taken up, by the race too if the menus are its pause screens. */
nfs::u32 cameraOnExitBody(Guest& guest, nfs::u32 menu)
{
    const std::int32_t first = signedWord(guest, menu + fe::menu::kFirstItem);
    const std::int32_t selected = signedWord(guest, menu + fe::menu::kSelected);
    const nfs::u32 item = fe::item::kTable + fe::item::kSize * nfs::u32(first + selected);
    bool cancel = false;
    if (guest.dword(item) == 5)
    {
        cancel = *fe::direct<kCodelinkMatches, port::menus::codelinkMatched>(guest.app(), guest.ts(),
                                                                              guest.ts().sp - 4,
                                                                              guest.dword(item + fe::item::kCodelink),
                                                                              kCameraCancel.address, 0 /* ebx: not read by the core */) != 0;
    }
    if (cancel)
    {
        for (nfs::u32 i = 0; i < std::size(kCameraViewsKept); ++i)
            gameSettings(guest).cameraViewsChosen[i] = guest.dword(kCameraViewsKept[i]);
        return 0;
    }
    fe::direct<0x472300, load::cameraViewsSet>(guest.app(), guest.ts());
    fe::direct<kBuildCameraCycle, carstate::cycleBuilt>(guest.app(), guest.ts());
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.ts(), 0, guest.dword(kCameraMode));
    fe::direct<kRememberMode, carstate::modeRemembered>(guest.app(), guest.ts(), 1, guest.dword(kCameraMode));
    if (guest.dword(kInRace) != 0)
    {
        fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), 0, guest.dword(kCameraMode));
        if (gameSettings(guest).playMode == 1)
            fe::direct<kSetCameraMode, carstate::cameraModePut>(guest.app(), guest.ts(), 1, guest.dword(kCameraMode));
    }
    return 0;
}

/* sub_45d9a0, camera on every pass: one view at least. */
nfs::u32 cameraOnFrameBody(Guest& guest)
{
    bool any = false;
    for (nfs::u32 i = 0; i < 4 && !any; ++i)
        any = gameSettings(guest).cameraViewsChosen[i] != 0;
    if (!any)
        gameSettings(guest).cameraViewsChosen[0] = 1;
    return 0;
}

/* ---------------------------------------------------------------------------
 * setup
 * ------------------------------------------------------------------------- */

constexpr GuestString kSetupOnePlayer{ 0x539614, "SETUP_ONEPLAYERBUTTON" };
constexpr GuestString kSetupConnect{ 0x53962c, "SETUP_CONNECTPLAYERS" };
constexpr GuestString kSetupTwoPlayers{ 0x539644, "SETUP_TWOPLAYERBUTTON" };
constexpr GuestString kSetupWebSite{ 0x53965c, "SETUP_NfsWebSite" };
constexpr GuestString kSetupLoad{ 0x539670, "SETUP_LOADBUTTON" };
constexpr GuestString kSetupName{ 0x53960c, "Setup" };
constexpr nfs::u32 kOnePlayerClicked = 0x452f20;
constexpr nfs::u32 kTwoPlayersClicked = 0x452d40;
constexpr nfs::u32 kWebSiteClicked = 0x452e60;
constexpr nfs::u32 kConnectionLost = 0x664674;    // char*: its line
constexpr nfs::u32 kWebSiteAnswer = 0x55e894;
constexpr nfs::u32 kRaceDialogCanCancel = 0x559540;

}  // namespace

/* sub_452f80, setup as it opens: the network shut if the race left one; a
 * developer's start, and the buttons' clicks. */
nfs::u32 frontend::misc::setupOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    gameSettings(guest).playMode = 0;
    if (guest.dword(kNetworkConnected) != 0)
                fe::direct<kCloseNetwork, ea::networkClosed>(app, ts);
    if (guest.word(kNetworkLost) != 0)
    {
        guest.setDword(kConnectionLost, fe::text(guest, 0x114));
        openMessage(app, ts, 1, kConnectionLost, kShortMessage);
        guest.setWord(kNetworkLost, 0);
    }
    if (Item onePlayer = fe::findItem(guest, menu, kSetupOnePlayer))
    {
        const std::uint8_t flags = guest.byte(kDeveloperFlags);
        if (flags & kPracticeOnly)
        {
            gameSettings(guest).raceType = 3;
        }
        else if (flags & 0x10)
        {
            gameSettings(guest).raceType = 3;
            if (guest.byte(kDeveloperFlags + 1) & 4)
                if (Item connect = fe::findItem(guest, menu, kSetupConnect))
                    connect.setDisabled(true);
        }
        else
        {
            onePlayer.setOnClick(kOnePlayerClicked);
        }
    }
    if (Item twoPlayers = fe::findItem(guest, menu, kSetupTwoPlayers))
        twoPlayers.setOnClick(kTwoPlayersClicked);
    if (Item webSite = fe::findItem(guest, menu, kSetupWebSite))
        webSite.setOnClick(kWebSiteClicked);
    if (gameSettings(guest).installIsRemote != 0)
    {
        for (const GuestString& codelink : { kSetupOnePlayer, kSetupTwoPlayers, kSetupLoad })
            if (Item item = fe::findItem(guest, menu, codelink))
                item.setDisabled(true);
    }
    fe::direct<0x4961e0, frontend::glue::resetAttractTimer>(app, ts);
    return 0;
}

/* sub_452f60, setup on every pass: the attract-mode movie, when it is time. */
nfs::u32 frontend::misc::setupOnFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const nfs::u32 played = fe::direct<0x496200, frontend::glue::attractMode>(app, ts);
    return played != 0 ? fe::kMenuMoviePlayed : played;
}

/* sub_452ac0: the race dialog answered.  The kinds of race its buttons stand
 * for lie one lower in a network game, which has no duel; returns the menu
 * loop's answer, 5 on to the next screen. */
nfs::u32 frontend::misc::setupRaceAnswered(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kOnward = 5, kStay = 2;
    nfs::u32 next = 0;
    bool close = false;
    if (answer == fe::kDialogEnter)
    {
        const bool network = gameSettings(guest).networkGame != 0;
        close = true;
        next = kOnward;
        switch (fe::dialogFocus(guest))
        {
        case 0:
            gameSettings(guest).raceType = 0;
            break;
        case 1:
            gameSettings(guest).raceType = network ? 2 : 3;
            break;
        case 2:
            gameSettings(guest).raceType = network ? 1 : 2;
            break;
        case 3:
            if (network)
                next = kStay;
            else
                gameSettings(guest).raceType = 1;
            break;
        case 4:
            next = kStay;
            break;
        default:
            close = false;
            next = 0;
            break;
        }
    }
    else if (answer == fe::kDialogEscape && guest.dword(kRaceDialogCanCancel) != 0)
    {
        close = true;
        next = kStay;
    }
    if (closes != 0)
        guest.setByte(closes, close ? 1 : 0);
    if (std::int32_t(gameSettings(guest).playMode) > 1)
        next = kStay;
    return next;
}

/* sub_452bd0: the race dialog: its buttons one fewer on the network, each with
 * its line of help; the last, Cancel, only if `canCancel`. */
nfs::u32 frontend::misc::setupAskRace(App* app, ThreadState& ts, nfs::u32 canCancel)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    constexpr nfs::u32 kLine = 0x559544, kButtons = 0x559548, kHelp = 0x55955c;
    const nfs::u32 network = gameSettings(guest).networkGame != 0 ? 1 : 0;
    guest.setDword(kLine, 0);
    guest.setDword(kRaceDialogCanCancel, canCancel);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kButtons, 0, 0x14);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kHelp, 0, 0x14);
    guest.setDword(kLine, fe::text(guest, 0x10f));
    guest.setDword(kButtons, fe::text(guest, 0xd5));
    guest.setDword(kButtons + 4, fe::text(guest, 0xdc));
    const nfs::u32 third = 2 - network, fourth = 3 - network, fifth = 4 - network;
    guest.setDword(kButtons + 4 * third, fe::text(guest, 0xd7));
    guest.setDword(kButtons + 4 * fourth, fe::text(guest, 0xd6));
    guest.setDword(kButtons + 4 * fifth, fe::text(guest, 0xc6));
    guest.setDword(kHelp, fe::text(guest, 0x181));
    guest.setDword(kHelp + 4, fe::text(guest, 0x184));
    guest.setDword(kHelp + 4 * third, fe::text(guest, 0x183));
    guest.setDword(kHelp + 4 * fourth, fe::text(guest, 0x182));
    guest.setDword(kHelp + 4 * fifth, 0);
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = (canCancel != 0 ? 1 : 0) + 4 - network;
    question.buttons = kButtons;
    question.buttonHelp = kHelp;
    question.focus = canCancel != 0 ? gameSettings(guest).raceType : 0;
    question.column = 1;
    question.callback = 0x452ac0;
    return openQuestionAnswer(guest, question);
}

namespace
{

/* sub_452d40 and sub_452f20: the buttons, two players side by side or one, each
 * off the network, on to the race dialog. */
nfs::u32 setupPlayersClicked(nfs::App* app, nfs::ThreadState& ts, Guest& guest, nfs::u32 players)
{
    gameSettings(guest).playMode = players;
    gameSettings(guest).networkGame = 0;
    guest.setDword(kNetworkConnected, 0);
    guest.setWord(kNetworkHost, 1);
    guest::direct<0x452bd0, frontend::misc::setupAskRace>(app, ts, 1);
    return 2;
}

}  // namespace

/* sub_452d40: two players side by side. */
nfs::u32 frontend::misc::setupTwoPlayersClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return setupPlayersClicked(app, ts, guest, 1);
}

/* sub_452f20: one player. */
nfs::u32 frontend::misc::setupOnePlayerClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return setupPlayersClicked(app, ts, guest, 0);
}

/* sub_452d80: the web site's "not possible" answered: the dialog closes. */
nfs::u32 frontend::misc::setupWebSiteRefused(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer != fe::kDialogEnter && answer != fe::kDialogEscape)
        return 0;
    fe::dialogFocus(guest);
    if (closes != 0)
        guest.setByte(closes, 1);
    return 2;
}

/* sub_452db0: whether the screen up is setup itself. */
nfs::u32 frontend::misc::setupIsCurrent(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const nfs::u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
    return fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), guest.dword(menu + fe::menu::kName), kSetupName.address) == 0 ? 1 : 0;
}

/* sub_452de0: the web site's question answered: Yes leaves the menus for it,
 * coming back by which screen it was asked on. */
nfs::u32 frontend::misc::setupWebSiteAnswered(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer == fe::kDialogEnter)
    {
        const nfs::u32 focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            guest.setDword(kWebSiteAnswer, guest::direct<0x452db0, setupIsCurrent>(app, ts) != 0 ? 1 : 2);
            return 0xa;
        }
        if (closes != 0)
            guest.setByte(closes, focus == 1 ? 1 : 0);
        return 2;
    }
    if (answer == fe::kDialogEscape)
    {
        if (closes != 0)
            guest.setByte(closes, 1);
        return 2;
    }
    return 0;
}

/* sub_452e60: the web site's button: asked first, or refused if the machine
 * has no way to dial out. */
nfs::u32 frontend::misc::setupWebSiteClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLines = 0x664660, kButtons = 0x66466c;
    fe::Question question{};
    question.lines = kLines;
    question.buttons = kButtons;
    // sub_49f880 handed what the first gave back in eax: 0, no names wanted.
    const nfs::u32 running = fe::direct<0x49f790, neta::rasAvailable>(app, ts);
    if (running != 0 || fe::direct<0x49f880, neta::rasPhoneBook>(app, ts, running) != 0)
    {
        guest.setDword(kLines, fe::text(guest, 0x1c0));
        guest.setDword(kLines + 4, fe::text(guest, 0x1c1));
        guest.setDword(kButtons, fe::text(guest, 0xc4));
        guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
        question.lineCount = 2;
        question.buttonCount = 2;
        question.callback = 0x452de0;
    }
    else
    {
        guest.setDword(kLines, fe::text(guest, 0x1c2));
        guest.setDword(kButtons, fe::text(guest, 0xf9));
        question.lineCount = 1;
        question.buttonCount = 1;
        question.callback = 0x452d80;
    }
    fe::openQuestion(guest, question);
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The race's results
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kParticipants = 0x6647b0;
constexpr nfs::u32 kParticipantSize = 0x9ac;
constexpr nfs::u32 kGroupSize = 0x9ac0;           // 16 of them
constexpr nfs::u32 kGroupCounts = 0x677d30;
constexpr nfs::u32 kPartId = 0x1f0;
constexpr nfs::u32 kPartFlags = 0x200;            // byte
constexpr nfs::u32 kPartCar = 0x220;              // its car's record; +0x10 the car, +0x34 its name
constexpr nfs::u32 kPartRank = 0x238;
constexpr nfs::u32 kPartLapTime = 0x2b8;          // float
constexpr nfs::u32 kPartHuman = 0x2bc;
constexpr nfs::u32 kPartState = 0x2c4;
constexpr nfs::u32 kPartPoints = 0x2c8;
constexpr nfs::u32 kPartLaps = 0x2cc;
constexpr nfs::u32 kPartTime = 0x2d4;
constexpr nfs::u32 kSortOrders = 0x5595d8;        // int16[16] for each group: the participants in order, or -1
constexpr nfs::u32 kSortKey = 0x559618;
constexpr nfs::u32 kSortDescending = 0x55961c;

constexpr nfs::u32 kCompareParticipants = 0x455840;
constexpr nfs::u32 kQsort = 0x4ee4f8;             // eax the array, edx its count, ebx 4, ecx how to compare

nfs::u32 participant(nfs::u32 group, std::int32_t index)
{
    return kParticipants + group * kGroupSize + nfs::u32(index) * kParticipantSize;
}

}  // namespace

/* sub_4554e0: a group's participant by its id, or 0.  Edx left as the search
 * leaves it, which no caller reads. */
nfs::u32 frontend::misc::resultsParticipantById(App* app, ThreadState& ts, nfs::u32 group, nfs::u32 id)
{
    Guest guest(app, ts);
    nfs::u32 left = id;
    for (std::int32_t i = 0; i < signedDword(guest, kGroupCounts + 4 * group); ++i)
    {
        left = nfs::u32(i) * kParticipantSize;
        const nfs::u32 found = participant(group, i);
        if (guest.dword(found + kPartId) == id)
        {
            return found;
        }
    }
    return 0;
}

/* sub_455530: its car's two ranks, into *a and *b, if there is one. */
nfs::u32 frontend::misc::resultsParticipantRanks(App* app, ThreadState& ts, nfs::u32 found, nfs::u32 a, nfs::u32 b)
{
    Guest guest(app, ts);
    if (found == 0)
        return 0;
    guest.setDword(a, nfs::u32(std::int32_t(gameSettings(guest).tournament.raceScores[guest.dword(guest.dword(found + kPartCar) + 0x10)])));
    const nfs::u32 rank = nfs::u32(std::int32_t(gameSettings(guest).tournament.totals[guest.dword(guest.dword(found + kPartCar) + 0x10)]));
    guest.setDword(b, rank);
    return rank;
}

/* sub_4557b0: whether it is what the mask asks: 4 and 8 flags it has, 1 and 2
 * flags it has not, 0x10 and 0x20 its state is or is not `state`. */
nfs::u32 frontend::misc::resultsParticipantMatches(App* app, ThreadState& ts, nfs::u32 found, nfs::u32 mask,
                                                     nfs::u32 state)
{
    Guest guest(app, ts);
    const std::uint8_t flags = guest.byte(found + kPartFlags);
    if ((mask & 4) && !(flags & 2))
        return 0;
    if ((mask & 8) && !(flags & 4))
        return 0;
    if ((mask & 1) && (flags & 2))
        return 0;
    if ((mask & 2) && (flags & 4))
        return 0;
    if ((mask & 0x10) && state != guest.dword(found + kPartState))
        return 0;
    if ((mask & 0x20) && state == guest.dword(found + kPartState))
        return 0;
    return 1;
}

namespace
{

/* Bigger first: -1 when b's field is the bigger. */
std::int32_t biggerFirst(const Guest& guest, nfs::u32 a, nfs::u32 b, nfs::u32 field)
{
    const std::int32_t va = signedDword(guest, a + field), vb = signedDword(guest, b + field);
    return vb > va ? -1 : vb < va ? 1 : 0;
}

/* By a car's rank, and the same rank by who has flag 4 last. */
std::int32_t byCarRank(const Guest& guest, nfs::u32 a, nfs::u32 b, const std::int16_t* ranks)
{
    const std::int32_t ra = ranks[guest.dword(guest.dword(a + kPartCar) + 0x10)];
    const std::int32_t rb = ranks[guest.dword(guest.dword(b + kPartCar) + 0x10)];
    if (ra != rb)
        return ra < rb ? -1 : 1;
    const bool fa = (guest.byte(a + kPartFlags) & 4) != 0, fb = (guest.byte(b + kPartFlags) & 4) != 0;
    return fa == fb ? 0 : fa ? 1 : -1;
}

}  // namespace

/* sub_455840, for qsort: two participants in the order the sort asked for by
 * kSortKey (0 none, 1 rank, 2 lap time, 3 and 4 the car's ranks, 5 laps, 6
 * points, 7 time, 8 the car's name), ties by rank. */
nfs::u32 frontend::misc::resultsCompare(App* app, ThreadState& ts, nfs::u32 aAt, nfs::u32 bAt)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    const nfs::u32 a = guest.dword(aAt), b = guest.dword(bAt);
    std::int32_t order = 0;
    switch (guest.dword(kSortKey))
    {
    case 1:
        order = biggerFirst(guest, a, b, kPartRank);
        break;
    case 2:
    {
        // Three comparisons as the game makes them: NaN against anything is 0.
        FloatMath x87(guest);
        const nfs::Float fa = x87.load(a + kPartLapTime), fb = x87.load(b + kPartLapTime);
        if (x87.below(fa, fb))
            order = -1;
        if (x87.above(fa, fb))
            order = 1;
        if (!(fa < fb) && !(fa > fb))  // equal, or unordered: fcom's c3
            order = 0;
        break;
    }
    case 3:
        order = byCarRank(guest, a, b, gameSettings(guest).tournament.raceScores);
        break;
    case 4:
        order = byCarRank(guest, a, b, gameSettings(guest).tournament.totals);
        break;
    case 5:
        order = biggerFirst(guest, a, b, kPartLaps);
        break;
    case 6:
        order = biggerFirst(guest, a, b, kPartPoints);
        break;
    case 7:
    {
        // Smaller first, those without a time last, then more laps first.
        const std::int32_t ta = signedDword(guest, a + kPartTime), tb = signedDword(guest, b + kPartTime);
        if (ta != 0 && tb != 0)
            order = ta < tb ? -1 : tb < ta ? 1 : 0;
        else if (ta != 0)
            order = -1;
        else if (tb != 0)
            order = 1;
        else
            order = -biggerFirst(guest, a, b, kPartLaps);
        break;
    }
    case 8:
        order = std::int32_t(fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), guest.dword(a + kPartCar) + 0x34, guest.dword(b + kPartCar) + 0x34));
        break;
    default:
        break;
    }
    if (guest.dword(kSortDescending) == 1)
        order = -order;
    if (order == 0)
        order = biggerFirst(guest, a, b, kPartRank);
    return nfs::u32(order);
}

/* sub_455c10: a group sorted, by a key and a way: the order's table filled,
 * -1 past its participants.  (What the loops leave in eax, ebx and edx goes
 * on, read by no caller.) */
nfs::u32 frontend::misc::resultsSort(App* app, ThreadState& ts, nfs::u32 group, nfs::u32 key, nfs::u32 descending)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x40);
    const nfs::u32 sorted = guest.frame();    // nfs::u32[16]
    const nfs::u32 order = kSortOrders + (group << 5);
    for (nfs::u32 i = 0; i < 16; ++i)
    {
        guest.setWord(order + 2 * i, 0xffff);
        guest.setDword(sorted + 4 * i, participant(group, std::int32_t(i)));
    }
    guest.setDword(kSortKey, key);
    guest.setDword(kSortDescending, descending);
    const nfs::u32 count = guest.dword(kGroupCounts + 4 * group);
    fe::direct<kQsort, watcom::clib::qsort>(guest.app(), guest.ts(), sorted, count, 4, kCompareParticipants);
    const std::int32_t n = signedDword(guest, kGroupCounts + 4 * group);
    if (n <= 0)
    {
        return group;   // eax as it came: the group, the first argument (no caller reads it)
    }
    for (std::int32_t i = 0; i < n; ++i)
        for (std::int32_t j = 0; j < n; ++j)
            if (participant(group, j) == guest.dword(sorted + 4 * nfs::u32(i)))
                guest.setWord(order + 2 * nfs::u32(i), std::uint16_t(j));
    return nfs::u32(n);
}

/* sub_455cd0: the index-th in the order that matches a mask (0: any), or 0.
 * Past the participants, or past the 16 slots, none.  (Ebx, ecx and edx left
 * as its search leaves them, read by no caller.) */
nfs::u32 frontend::misc::resultsNth(App* app, ThreadState& ts, nfs::u32 group, nfs::u32 index, nfs::u32 mask,
                                      nfs::u32 state)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x14);
    if (std::int32_t(index) > 0x10)
        return 0;
    const nfs::u32 order = kSortOrders + (group << 5);
    const std::int32_t count = signedDword(guest, kGroupCounts + 4 * group);
    if (mask == 0)
    {
        const std::int32_t at = signedWord(guest, order + 2 * index);
        if (at < 0)
            return 0;
        if (at > count)
            return 0;
        return participant(group, at);
    }
    nfs::u32 leftEbx = 0, leftEdx = index;
    nfs::u32 found = 0;
    for (nfs::u32 slot = 0; slot < 16; ++slot)
    {
        leftEdx = (group << 5) + 2 * slot;
        const std::uint16_t word = guest.word(order + 2 * slot);
        leftEbx = (leftEbx & 0xffff0000) | word;
        const std::int32_t at = std::int16_t(word);
        if (at < 0 || at > count)
            continue;
        const nfs::u32 candidate = participant(group, at);
        const nfs::u32 matches = guest::direct<0x4557b0, resultsParticipantMatches>(app, ts, candidate, mask, state);
        leftEdx = mask;
        leftEbx = state;
        if (matches == 0)
            continue;
        if (found == index)
        {
            return candidate;
        }
        ++found;
        leftEbx = found;
    }
    return 0;
}

/* sub_455dd0: the same for an order made for the occasion, the group's kept
 * as it was. */
nfs::u32 frontend::misc::resultsNthSorted(App* app, ThreadState& ts, nfs::u32 group, nfs::u32 index, nfs::u32 key,
                                            nfs::u32 descending, nfs::u32 mask, nfs::u32 state)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x28);
    const nfs::u32 order = kSortOrders + (group << 5);
    const nfs::u32 kept = guest.frame();      // std::int16_t[16]
    fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kept, order, 0x20);
    guest::direct<0x455c10, resultsSort>(app, ts, group, key, descending);
    const nfs::u32 found = guest::direct<0x455cd0, resultsNth>(app, ts, group, index, mask, state);
        fe::direct<kCopy, ea::moveMemory>(app, ts, order, kept, 0x20);
    return found;
}

namespace
{

/* ---------------------------------------------------------------------------
 * records
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kRecordsFile = 0x559408;       // the records, loaded
constexpr nfs::u32 kRecordTimes = 0x55940c;       // the track's chosen, in it
constexpr nfs::u32 kRecordHolder = 0x559410;
constexpr nfs::u32 kRecordsDelete = 0x559424;     // the item
constexpr nfs::u32 kRecordsSelect = 0x44f670;
constexpr GuestString kRecordsDeleteItem{ 0x5395c0, "RECORD_DELETE" };
constexpr GuestString kRecordsSheet{ 0x5395d0, "RECORDS_SHEET" };

}  // namespace

/* sub_44f670: a track's and a direction's records chosen.  Edx left at the
 * holder's, read by no caller. */
nfs::u32 frontend::misc::recordsSelect(App* app, ThreadState& ts, nfs::u32 track, nfs::u32 kind)
{
    Guest guest(app, ts);
    const nfs::u32 file = guest.dword(kRecordsFile);
    const nfs::u32 times = nfs::u32(std::int32_t(kind) % 3) * 0x15634 + file + 0x16e0 + track * 0x1f1c;
    const nfs::u32 offset = track * 0x214;
    const nfs::u32 holder = file + 4 + offset;
    guest.setDword(kRecordTimes, times);
    guest.setDword(kRecordHolder, holder);
    return offset;
}

/* sub_44fa90: the holder of the chosen record: whether there is one, and its
 * names, into three buffers. */
nfs::u32 frontend::misc::recordsRead(App* app, ThreadState& ts, nfs::u32 first, nfs::u32 second, nfs::u32 third)
{
    Guest guest(app, ts);
    if (guest.dword(guest.dword(kRecordHolder)) != 0)
    {
        guest.copyString(first, guest.dword(kRecordHolder) + 8);
        guest.copyString(second, guest.dword(kRecordHolder) + 0x13);
        guest.copyString(third, guest.dword(kRecordHolder) + 0x113);
    }
    return guest.dword(guest.dword(kRecordHolder));
}

/* sub_44fbc0: the chosen records wiped. */
nfs::u32 frontend::misc::recordsClear(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(guest.dword(kRecordTimes), 0);
    const nfs::u32 holder = guest.dword(kRecordHolder);
    guest.setDword(holder, 0);
    return holder;
}

/* sub_450f70, records as it opens. */
nfs::u32 frontend::misc::recordsOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    Item erase = fe::findItem(guest, menu, kRecordsDeleteItem);
    guest.setDword(kRecordsDelete, erase.address());
    if (erase)
        erase.setOnClick(0x450eb0);
    const nfs::u32 language = gameSettings(guest).language;
    if (language == 1 || language == 5)
    {
        if (Item sheet = fe::findItem(guest, menu, kRecordsSheet))
        {
            guest.setWord(sheet.address() + 0x44, 0x59);
            guest.setWord(sheet.address() + 0x46, 0x2a);
            guest.setWord(sheet.address() + 0x42, 0x48);
        }
    }
    return 0;
}

/* sub_450f20, records on every pass: delete is open while there is something
 * to delete. */
nfs::u32 frontend::misc::recordsOnFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    constexpr nfs::u32 kChosenTrack = 0x559418, kChosenKind = 0x55941c;
    if (guest.dword(kRecordsDelete) == 0)
        return 0;
    bool any = false;
    if (guest::direct<0x44f5d0, recordsLoaded>(app, ts) != 0)
        any = fe::direct<0x44f8d0, frontend::menub::recordsCount>(app, ts, guest.dword(kChosenTrack),
                                                                 guest.dword(kChosenKind)) != 0;
    Item(guest, guest.dword(kRecordsDelete)).setDisabled(!any);
    return 0;
}

/* sub_450eb0: delete's click: asked first. */
nfs::u32 frontend::misc::recordsDeleteClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kButtons = 0x6645e4, kLine = 0x6645ec;
    guest.setDword(kButtons, fe::text(guest, 0xc4));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
    guest.setDword(kLine, fe::text(guest, 0x145));
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x450e50;
    fe::openQuestion(guest, question);
    return 2;
}

/* sub_450e50: the answer: Yes wipes them.  Either closes the dialog. */
nfs::u32 frontend::misc::recordsDeleteAnswered(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (answer == fe::kDialogEnter || answer == fe::kDialogEscape)
    {
        if (answer == fe::kDialogEnter && fe::dialogFocus(guest) == 0)
            guest::direct<0x44fbc0, recordsClear>(app, ts);
        if (closes != 0)
            guest.setByte(closes, 1);
        return 2;
    }
    if (closes != 0)
        guest.setByte(closes, 0);
    return 0;
}

/* sub_4cef00: whether a track has a record's holder, the records' file loaded
 * for the moment if it is not. */
nfs::u32 frontend::misc::recordsHolder(App* app, ThreadState& ts, nfs::u32 track)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x100);
    if (guest.byte(kDeveloperFlags) & kPracticeOnly)
        return 1;
    const nfs::u32 names = guest.frame();     // char[0x100]
    const nfs::u32 trackIndex = nfs::u32(std::int32_t(std::int16_t(track)));
    const bool loaded = guest::direct<0x44f5d0, recordsLoaded>(app, ts) != 0;
    if (!loaded)
        fe::direct<0x44f510, load::recordsTableLoaded>(app, ts);
    // The code hands ebx too, the buffer, which the core does not take.
    guest::direct<kRecordsSelect, recordsSelect>(app, ts, trackIndex, 0);
    const nfs::u32 holder = guest::direct<0x44fa90, recordsRead>(app, ts, names, names, names);
    if (!loaded)
        fe::direct<0x44f5f0, load::recordsTableSaved>(app, ts);
    return holder;
}

namespace
{

/* ---------------------------------------------------------------------------
 * postgame
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kSheetSummary = 0x5593f0;
constexpr nfs::u32 kSheetToggle = 0x5593f4;
constexpr nfs::u32 kSheetRecords = 0x5593f8;
constexpr nfs::u32 kSheetTournament = 0x5593fc;
constexpr nfs::u32 kSheetCop = 0x559400;
constexpr nfs::u32 kSheetPerformance = 0x559404;
constexpr nfs::u32 kSheetShown = 0x55936c;
constexpr nfs::u32 kSheetCycle = 0x559370;        // int16[], which sheets are toggled through, ending 0
// Each sheet's state word, and its fade's colour next to it: 3 hidden, 2 hiding,
// 1 shown.
constexpr nfs::u32 kSummaryState = 0x88;
constexpr nfs::u32 kRecordsState = 0xb8;
constexpr nfs::u32 kTableState = 0x7e;
constexpr nfs::u32 kPerformanceState = 0x6c;
constexpr std::uint16_t kSheetHidden = 3, kSheetHiding = 2, kSheetUp = 1;
constexpr std::uint16_t kSheetFadeColour = 0x2a8;
constexpr nfs::u32 kCyclePlain = 0x5593b0;
constexpr nfs::u32 kCycleTournament = 0x559390;
constexpr nfs::u32 kCycleDuel = 0x5593d0;

constexpr nfs::u32 kTrackNameIds = 0x44e152;      // int16[10]: a track's name's line, in the game's code segment
constexpr nfs::u32 kTrackLine = 0x6644c4;
constexpr nfs::u32 kTrackUnlocked = 0x556382;     // int16 for each track
constexpr nfs::u32 kTrackNames = 0x55636e;

constexpr GuestString kPgRaceNow{ 0x539494, "racenow" };
constexpr GuestString kPgSpeech{ 0x53949c, "PG_SPEECH" };
constexpr GuestString kPgTrackAndLaps{ 0x5394a8, "TRACK_AND_LAPS" };
constexpr GuestString kPgTrackFormat{ 0x5394b8, "%d %s %s" };
constexpr GuestString kPgRecords{ 0x5394c4, "RECORDS_SHEET" };
constexpr GuestString kPgSummary{ 0x5394d4, "RACESUMMARY_SHEET" };
constexpr GuestString kPgTournament{ 0x5394e8, "TOURNAMENT_SHEET" };
constexpr GuestString kPgCop{ 0x5394fc, "COP_SHEET" };
constexpr GuestString kPgToggle{ 0x539508, "TOGGLE_SHEETS" };
constexpr GuestString kPgPerformance{ 0x539518, "PERFORMANCE_SHEET" };
constexpr GuestString kPgViewReplay{ 0x53952c, "viewreplay" };
constexpr GuestString kPgSaveReplay{ 0x539538, "savereplay" };
constexpr GuestString kPgSaveGhost{ 0x539544, "Save_Ghost" };
constexpr GuestString kPgSaveTournament{ 0x539550, "Save_Tourny" };
constexpr GuestString kPgSaveKnockout{ 0x53955c, "Save_KO" };
constexpr GuestString kPgMainMenu{ 0x539564, "Main_Menu" };
constexpr GuestString kPgQuit{ 0x539570, "Quit" };

}  // namespace

/* sub_442b60: a bonus track open from now on. */
nfs::u32 frontend::misc::postgameUnlockTrack(App* app, ThreadState& ts, nfs::u32 track)
{
    Guest guest(app, ts);
    guest.setWord(2 * track + kTrackUnlocked, 1);
    guest.setWord(2 * track + kTrackNames, std::uint16_t(track + 0x66));
    return track + 0x66;
}

/* sub_44e170: a line of an award. */
nfs::u32 frontend::misc::postgameAwardMessage(App* app, ThreadState& ts, nfs::u32 id)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLine = 0x6644a8;
    guest.setDword(kLine, fe::text(guest, id));
    return openMessage(app, ts, 1, kLine, kLongMessage);
}

/* sub_44e1a0: what the race just won opens: a bonus track or car, and the line
 * that says so, the last one won.  (What a pass leaves in eax goes on as the
 * result, read by no caller.) */
nfs::u32 frontend::misc::postgameCheckAwards(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x14);
    constexpr nfs::u32 kFirstTournamentTrack = 0x55d020;
    constexpr std::int32_t kLastTrack = 8;
    const std::int32_t lastPlayer = gameSettings(guest).playMode == 1 ? 1 : 0;
    guest::direct<0x455c10, resultsSort>(app, ts, 0, 0, 0);
    nfs::u32 message = 0;
    nfs::u32 left = 0;
    const auto win = [&](std::uint8_t bit) {
        gameSettings(guest).unlockedCarsByte = std::uint8_t(std::uint8_t(gameSettings(guest).unlockedCars) | bit);
    };
    const nfs::u32 ours = guest.frame(), theirs = guest.frame() + 4, second = guest.frame() + 8;
    for (std::int32_t player = 0; player <= lastPlayer; ++player)
    {
        const nfs::u32 id = gameSettings(guest).localPlayer + nfs::u32(player);
        const nfs::u32 type = gameSettings(guest).raceType;
        left = type;
        if (type == 2)
        {
            const nfs::u32 found = guest::direct<0x4554e0, resultsParticipantById>(app, ts, 0, id);
            left = found;
            if (found == 0)
                continue;
            const nfs::u32 track = gameSettings(guest).track;
            if (track != nfs::u32(kLastTrack) || guest.dword(found + kPartState) == 2)
                continue;
            guest::direct<0x442b60, postgameUnlockTrack>(app, ts, track);
            if (gameSettings(guest).unlockedAll == 0)
                message = 0x1b7;
            gameSettings(guest).unlockedAll = 1;
            left = gameSettings(guest).seriesVariant;
            if (left != 1)
                continue;
            if (!(std::uint8_t(gameSettings(guest).unlockedCars) & 1))
                message = message == 0x1b7 ? 0x1b5 : 0x1b6;
            win(1);
        }
        else if (type == 1)
        {
            const nfs::u32 found = guest::direct<0x4554e0, resultsParticipantById>(app, ts, 0, id);
            left = found;
            if (found == 0 || guest.dword(kFirstTournamentTrack) != gameSettings(guest).track)
                continue;
            const nfs::u32 best = guest::direct<0x455dd0, resultsNthSorted>(app, ts, 0, 0, 4, 1, 0, 0);
            guest.setDword(ours, 0);
            guest.setDword(theirs, 0);
            guest.setDword(second, 0);
            guest::direct<0x455530, resultsParticipantRanks>(app, ts, best, theirs, ours);
            guest::direct<0x455530, resultsParticipantRanks>(app, ts, found, theirs, second);
            left = guest.dword(second);
            if (std::int32_t(left) < signedDword(guest, ours))
                continue;
            if (!(std::uint8_t(gameSettings(guest).unlockedCars) & 4))
            {
                message = 0x1b4;
                win(4);
            }
            if (gameSettings(guest).seriesVariant != 1 || (std::uint8_t(gameSettings(guest).unlockedCars) & 2))
                continue;
            message = message == 0x1b4 ? 0x1b2 : 0x1b3;
            win(2);
        }
        else if (type == 3)
        {
            const std::int32_t track = std::int32_t(gameSettings(guest).track);
            const std::uint32_t trackBit = 1u << (std::uint32_t(track) & 31);
            const nfs::u32 fastest = guest::direct<0x4554e0, resultsParticipantById>(app, ts, 1, id);
            if (fastest != 0 && (guest.byte(fastest + kPartFlags) & 0x20)
                && std::int32_t(gameSettings(guest).pursuitWinCount) <= signedDword(guest, fastest + kPartLaps))
            {
                if (track <= kLastTrack)
                    gameSettings(guest).duelsWon = std::uint16_t(std::uint16_t(gameSettings(guest).duelsWon) | trackBit);
                if (std::uint16_t(gameSettings(guest).duelsWon) == 0x1ff)
                {
                    if (!(std::uint8_t(gameSettings(guest).unlockedCars) & 8))
                        message = 0x1b8;
                    win(8);
                }
            }
            const nfs::u32 found = guest::direct<0x4554e0, resultsParticipantById>(app, ts, 0, id);
            left = found;
            if (found == 0 || (guest.byte(found + kPartFlags) & 0x20) || guest.dword(found + kPartHuman) != 1
                || guest.dword(found + kPartState) == 4)
                continue;
            if (track <= kLastTrack)
            {
                left = trackBit;
                gameSettings(guest).duelsLost = std::uint16_t(std::uint16_t(gameSettings(guest).duelsLost) | trackBit);
            }
            if (std::uint16_t(gameSettings(guest).duelsLost) == 0x1ff)
            {
                if (!(std::uint8_t(gameSettings(guest).unlockedCars) & 0x10))
                    message = 0x1b9;
                win(0x10);
            }
        }
    }
    if (message != 0)
        return guest::direct<0x44e170, postgameAwardMessage>(app, ts, message);
    return left;
}

/* sub_44e4c0: a network race left, by whichever way this machine has. */
nfs::u32 frontend::misc::postgameLeaveNetwork(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (std::int32_t(gameSettings(guest).playMode) > 1 && guest.dword(kNetworkConnected) != 0 && guest.word(kNetworkLeaving) == 0)
    {
        if (guest.word(kNetworkHost) != 0)
            return fe::direct<0x4a6a10, neta::barrier>(app, ts, 0x28, 0x21);
                return *fe::direct<kCloseNetwork, ea::networkClosed>(app, ts);
    }
    return 0;   // eax is whatever the caller had (none reads it)
}

/* sub_44e510: "leave?" answered. */
nfs::u32 frontend::misc::postgameLeaveAnswered(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
        return 0;
    }
    if (answer != fe::kDialogEnter)
        return 0;
    if (fe::dialogFocus(guest) == 1)
    {
        guest.setByte(closes, 1);
        return 0;
    }
    guest.setByte(closes, 1);
    guest::direct<0x44e4c0, postgameLeaveNetwork>(app, ts);
    return 0xb;
}

/* sub_44e550: "leave?" with Yes and No. */
nfs::u32 frontend::misc::postgameAskLeave(App* app, ThreadState& ts, nfs::u32 lineId)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLine = 0x6644b4, kButtons = 0x6644ac;
    guest.setDword(kLine, fe::text(guest, lineId));
    guest.setDword(kButtons, fe::text(guest, 0xf9));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc6));
    fe::Question question{};
    question.lineCount = 1;
    question.lines = kLine;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x44e510;
    return openQuestionAnswer(guest, question);
}

/* sub_44e5b0: Main Menu's click: asked first if a series has more to race. */
nfs::u32 frontend::misc::postgameMainMenuClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (guest::direct<0x4cf090, postgameRaceAgain>(app, ts) != 0)
    {
        const nfs::u32 type = gameSettings(guest).raceType;
        if (type == 1 || type == 2)
        {
            guest::direct<0x44e550, postgameAskLeave>(app, ts, type == 1 ? 0xfeu : 0xffu);
            return 2;
        }
    }
    return 0xb;
}

/* sub_44e600: the same in a network race. */
nfs::u32 frontend::misc::postgameNetworkMainMenuClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (guest.word(kNetworkHost) == 0)
    {
        if (guest.dword(kNetworkConnected) != 0 && guest.word(kNetworkLeaving) == 0)
        {
            guest::direct<0x44e550, postgameAskLeave>(app, ts, 0x101);
            return 2;
        }
        return 0xb;
    }
    if (guest::direct<0x4cf090, postgameRaceAgain>(app, ts) == 0)
    {
        guest::direct<0x44e4c0, postgameLeaveNetwork>(app, ts);
        return 0xb;
    }
    const nfs::u32 type = gameSettings(guest).raceType;
    guest::direct<0x44e550, postgameAskLeave>(app, ts, type == 1 ? 0xfeu : type == 2 ? 0xffu : 0xfdu);
    return 2;
}

/* sub_44e6c0: why a replay of this race cannot be kept. */
nfs::u32 frontend::misc::postgameCannotSaveReplay(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLines = 0x6644b8;
    guest.setDword(kLines, fe::text(guest, 0x1cd));
    guest.setDword(kLines + 4, fe::text(guest, 0x1ce));
    openMessage(app, ts, 2, kLines, kLongMessage);
    return 2;
}

/* sub_44e710: View Replay's click: the kept replay, the one just raced, the
 * two tables it loads over kept as they were. */
nfs::u32 frontend::misc::postgameViewReplayClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0xb8);
    constexpr nfs::u32 kCarsBytes = 0x50;
    constexpr nfs::u32 kOpponentsBytes = 0x62;
    std::array<std::uint8_t, kCarsBytes> cars;
    std::array<std::uint8_t, kOpponentsBytes> opponents;
    for (nfs::u32 i = 0; i < kCarsBytes; ++i)
        cars[i] = guest.byte(SETTINGS_ADDRESS(knockout) + i);
    for (nfs::u32 i = 0; i < kOpponentsBytes; ++i)
        opponents[i] = guest.byte(SETTINGS_ADDRESS(tournament) + i);
    const nfs::u32 name = guest.frame() + 0xb4;
    guest.setByte(name, guest.byte(0x44e150));      // the empty string
    nfs::u32 result = 2;
    if (*fe::direct<0x446990, load::keptReplayLoaded>(app, ts, name) != 0)
        result = 7;
    else
        fe::direct<0x444be0, frontend::dialogs::messageById>(app, ts, 0x41);
    for (nfs::u32 i = 0; i < kCarsBytes; ++i)
        guest.setByte(SETTINGS_ADDRESS(knockout) + i, cars[i]);
    for (nfs::u32 i = 0; i < kOpponentsBytes; ++i)
        guest.setByte(SETTINGS_ADDRESS(tournament) + i, opponents[i]);
    return result;
}

/* sub_44e7a0: the toggle's click: the next sheet in the race's cycle. */
nfs::u32 frontend::misc::postgameToggleSheets(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const nfs::u32 shown = guest.dword(kSheetShown);
    guest.setDword(kSheetShown, guest.word(2 * shown + kSheetCycle + 2) != 0 ? shown + 1 : 0);
    return 2;
}

/* sub_44efd0, postgame as it opens. */
nfs::u32 frontend::misc::postgameOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x18);
    if (guest::direct<0x4cef80, postgameFinishers>(app, ts) == 0)
        guest::direct<0x44e1a0, postgameCheckAwards>(app, ts);
    if (Item speech = fe::findItem(guest, menu, kPgSpeech))
        guest.setDword(speech.address() + kOnDraw, 0x44ed30);
    if (Item trackAndLaps = fe::findItem(guest, menu, kPgTrackAndLaps))
    {
        const nfs::u32 trackName = fe::text(guest, nfs::u32(signedWord(guest, kTrackNameIds + 2 * gameSettings(guest).track)));
        const nfs::u32 lapsWord = fe::text(guest, 5);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kTrackLine, kPgTrackFormat.address, guest::Words{gameSettings(guest).laps, lapsWord, trackName});
        trackAndLaps.setText(kTrackLine);
    }
    const nfs::u32 records = fe::findItem(guest, menu, kPgRecords).address();
    guest.setDword(kSheetRecords, records);
    const nfs::u32 summary = fe::findItem(guest, menu, kPgSummary).address();
    guest.setDword(kSheetSummary, summary);
    const nfs::u32 tournament = fe::findItem(guest, menu, kPgTournament).address();
    guest.setDword(kSheetTournament, tournament);
    const nfs::u32 cop = fe::findItem(guest, menu, kPgCop).address();
    guest.setDword(kSheetCop, cop);
    const nfs::u32 toggle = fe::findItem(guest, menu, kPgToggle).address();
    guest.setDword(kSheetToggle, toggle);
    const nfs::u32 performance = fe::findItem(guest, menu, kPgPerformance).address();
    guest.setDword(kSheetPerformance, performance);
    if (summary == 0 || toggle == 0 || records == 0 || tournament == 0 || cop == 0 || performance == 0)
        return 0;

    guest.setDword(toggle + fe::item::kOnClick, 0x44e7a0);
    const auto hideSheet = [&](nfs::u32 sheet, nfs::u32 state) {
        guest.setWord(sheet + state, kSheetHidden);
        guest.setWord(sheet + state + 2, kSheetFadeColour);
    };
    hideSheet(performance, kPerformanceState);
    hideSheet(records, kRecordsState);
    hideSheet(summary, kSummaryState);
    hideSheet(tournament, kTableState);
    hideSheet(cop, kTableState);

    const nfs::u32 type = gameSettings(guest).raceType;
    if (type == 1)
    {
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kSheetCycle, kCycleTournament, 0x20);
        guest.setDword(kSheetShown, 3);
    }
    else if (type == 3 && guest::direct<0x455cd0, resultsNth>(app, ts, 1, 0, 1, 0) != 0)
    {
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kSheetCycle, kCycleDuel, 0x20);
        const nfs::u32 local = gameSettings(guest).localPlayer;
        guest.setDword(kSheetShown, guest::direct<0x4554e0, resultsParticipantById>(app, ts, 1, local) != 0 ? 3 : 2);
    }
    else
    {
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, kSheetCycle, kCyclePlain, 0x20);
        guest.setDword(kSheetShown, 2);
    }

    const std::int32_t players = std::int32_t(gameSettings(guest).playMode);
    if (Item viewReplay = fe::findItem(guest, menu, kPgViewReplay))
    {
        if (players > 1 || gameSettings(guest).lastTickRaced == 0)
            viewReplay.setDisabled(true);
        else
            viewReplay.setOnClick(0x44e710);
    }
    if (Item saveReplay = fe::findItem(guest, menu, kPgSaveReplay))
    {
        if (gameSettings(guest).lastTickRaced == 0)
            saveReplay.setDisabled(true);
        else if (guest::direct<0x4cf040, postgameReplayExcluded>(app, ts) != 0)
            saveReplay.setOnClick(0x44e6c0);
        else if (players > 1 && gameSettings(guest).networkGame != 0)
            saveReplay.setDisabled(true);
        else
            saveReplay.setOnClick(0x44e6a0);
    }
    if (Item raceNow = fe::findItem(guest, menu, kPgRaceNow))
    {
        if (guest::direct<0x4cf090, postgameRaceAgain>(app, ts) == 0)
            raceNow.setDisabled(true);
        raceNow.setText(fe::text(guest, 0xf2));
        if (gameSettings(guest).raceType == 2 || gameSettings(guest).raceType == 1)
            raceNow.setText(fe::text(guest, 0xf1));
    }
    if (Item saveGhost = fe::findItem(guest, menu, kPgSaveGhost))
    {
        const nfs::u32 fastest =
            guest::direct<0x4554e0, resultsParticipantById>(app, ts, 1, gameSettings(guest).localPlayer);
        const nfs::u32 raceType = gameSettings(guest).raceType;
        if (gameSettings(guest).lastTickRaced != 0 && gameSettings(guest).playMode == 0 && raceType != 1 && raceType != 2 && fastest == 0)
        {
            saveGhost.setOnClick(0x44e4a0);
            saveGhost.setDisabled(false);
        }
        else
        {
            saveGhost.setDisabled(true);
        }
    }
    if (Item saveTournament = fe::findItem(guest, menu, kPgSaveTournament))
    {
        if (gameSettings(guest).raceType == 1 && players <= 1)
        {
            saveTournament.setOnClick(0x44e480);
            showAgain(guest, saveTournament.address());
            if (guest::direct<0x4cf090, postgameRaceAgain>(app, ts) == 0)
                saveTournament.setDisabled(true);
        }
        else
        {
            greyAndHide(guest, saveTournament.address());
        }
    }
    if (Item saveKnockout = fe::findItem(guest, menu, kPgSaveKnockout))
    {
        if (gameSettings(guest).raceType == 2 && players <= 1)
        {
            saveKnockout.setOnClick(0x44e460);
            showAgain(guest, saveKnockout.address());
            if (guest::direct<0x4cf090, postgameRaceAgain>(app, ts) == 0)
                saveKnockout.setDisabled(true);
        }
        else
        {
            greyAndHide(guest, saveKnockout.address());
        }
    }
    if (Item mainMenu = fe::findItem(guest, menu, kPgMainMenu))
    {
        mainMenu.setOnClick(players > 1 ? 0x44e600 : 0x44e5b0);
        mainMenu.setText(fe::text(guest, players > 1 && guest.word(kNetworkHost) == 0 ? 0x1a9 : 0x105));
    }
    if (Item quit = fe::findItem(guest, menu, kPgQuit))
        quit.setOnClick(0x44e690);
    const nfs::u32 current = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
    if (Item item = fe::findItem(guest, current, kPgSummary))
        showAgain(guest, item.address());
    const nfs::u32 language = gameSettings(guest).language;
    if (language == 1 || language == 5)
    {
        const nfs::u32 recordsSheet = guest.dword(kSheetRecords);
        guest.setWord(recordsSheet + 0x42, 0x48);
        guest.setWord(recordsSheet + 0x44, 0x59);
        guest.setWord(recordsSheet + 0x46, 0x2a);
        const nfs::u32 copSheet = guest.dword(kSheetCop);
        guest.setWord(copSheet + 0x3c, 0xa0);
        guest.setWord(copSheet + 0x3e, 0x8c);
    }
    return 0;
}

/* sub_44eda0, postgame on every pass: the network's end told; the sheet shown
 * coming up, the others hiding. */
nfs::u32 frontend::misc::postgameOnFrame(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr nfs::u32 kLine = 0x6644c0;
    fe::direct<0x4c9860, port::descriptionServed>(app, ts, ts.sp - 4);
    if (std::int32_t(gameSettings(guest).playMode) > 1 && guest.word(kNetworkLeaving) != 0 && guest.dword(kNetworkConnected) != 0)
    {
        guest.setDword(kLine, fe::text(guest, 0x114));
        openMessage(app, ts, 1, kLine, kShortMessage);
                guest.setWord(kNetworkLost, 0);
        fe::direct<kCloseNetwork, ea::networkClosed>(app, ts);
    }
    if (std::int32_t(gameSettings(guest).playMode) > 1 && guest.dword(kNetworkConnected) == 0)
        if (Item raceNow = fe::findItem(guest, menu, kPgRaceNow))
            raceNow.setDisabled(true);
    for (const nfs::u32 slot : { kSheetSummary, kSheetToggle, kSheetRecords, kSheetTournament, kSheetCop, kSheetPerformance })
        if (guest.dword(slot) == 0)
            return 0;
    const auto hiding = [&](nfs::u32 slot, nfs::u32 state) {
        const nfs::u32 sheet = guest.dword(slot);
        if (guest.word(sheet + state) != kSheetHidden)
            guest.setWord(sheet + state, kSheetHiding);
    };
    hiding(kSheetPerformance, kPerformanceState);
    hiding(kSheetRecords, kRecordsState);
    hiding(kSheetSummary, kSummaryState);
    hiding(kSheetTournament, kTableState);
    hiding(kSheetCop, kTableState);
    for (const nfs::u32 slot : { kSheetRecords, kSheetPerformance, kSheetSummary, kSheetTournament, kSheetCop })
        Item(guest, guest.dword(slot)).setDisabled(true);
    const auto bringUp = [&](nfs::u32 slot, nfs::u32 state) {
        const nfs::u32 sheet = guest.dword(slot);
        guest.setWord(sheet + state, kSheetUp);
        Item(guest, sheet).setDisabled(false);
    };
    switch (guest.dword(kSheetShown))
    {
    case 0:
        bringUp(kSheetPerformance, kPerformanceState);
        break;
    case 1:
        bringUp(kSheetRecords, kRecordsState);
        break;
    case 2:
        bringUp(kSheetSummary, kSummaryState);
        break;
    case 3:
        if (gameSettings(guest).raceType == 1)
            bringUp(kSheetTournament, kTableState);
        else if (gameSettings(guest).raceType == 3)
            bringUp(kSheetCop, kTableState);
        break;
    default:
        break;
    }
    return 0;
}

/* sub_4cef80: whether anybody finished the race, as the developer's flag lets it. */
nfs::u32 frontend::misc::postgameFinishers(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const nfs::u32 finisher = guest::direct<0x455dd0, resultsNthSorted>(app, ts, 0, 0, 0, 0, 0x10, 1);
    if (guest.byte(kDeveloperFlags) & kNoFinishers)
        return 0;
    return finisher != 0 ? 1 : 0;
}

/* sub_4cf000: how many are still in. */
nfs::u32 frontend::misc::postgameFinishersCount(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    std::int32_t i = 0;
    while (i < signedDword(guest, kGroupCounts)
           && guest::direct<0x455dd0, resultsNthSorted>(app, ts, 0, nfs::u32(i), 0, 0, 0x28, 2) != 0)
        ++i;
    return nfs::u32(i);
}

/* sub_4cf040: whether a machine in the network race cannot keep a replay. */
nfs::u32 frontend::misc::postgameReplayExcluded(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    if (std::int32_t(gameSettings(guest).playMode) > 1)
        for (std::int32_t i = 0; i < std::int32_t(gameSettings(guest).entryCount); ++i)
            if (gameSettings(guest).entryAt(nfs::u32(i) * 0x6c).flag68 != 0)
                return 1;
    return 0;
}

/* sub_4cf090: whether a series goes on with another race. */
nfs::u32 frontend::misc::postgameRaceAgain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLastTournamentTrack = 0x55d020;
    bool again = true;
    const nfs::u32 type = gameSettings(guest).raceType;
    if (type == 1 || type == 2)
    {
        const bool lastTrack = type == 2 ? std::int32_t(gameSettings(guest).knockout.tracks[6]) == std::int32_t(gameSettings(guest).track)
                                         : guest.dword(kLastTournamentTrack) == gameSettings(guest).track;
        if (lastTrack)
            again = false;
        if (guest::direct<0x4cf000, postgameFinishersCount>(app, ts) == 0
            || guest::direct<0x4cef80, postgameFinishers>(app, ts) != 0)
            again = false;
    }
    if (std::int32_t(gameSettings(guest).playMode) > 1 && (guest.dword(kNetworkConnected) == 0 || guest.word(kNetworkLeaving) != 0))
        again = false;
    return again ? 1 : 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * loadsave
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kReplayList = 0x5591c0;        // the list's item
constexpr nfs::u32 kDeleteButton = 0x5591c4;
constexpr nfs::u32 kLoadButton = 0x5591c8;
constexpr nfs::u32 kShownKind = 0x5591bc;         // 0 replays, 1 ghosts, 2 tournaments, 3 knockouts
constexpr nfs::u32 kDescription = 0x558d0c;       // ten lines of 0x78
constexpr nfs::u32 kDescriptionBytes = 0x4b0;
constexpr nfs::u32 kDescriptionLine2 = 0x558d84;
constexpr nfs::u32 kDescriptionLine3 = 0x558dfc;
constexpr nfs::u32 kDescriptionLine4 = 0x558e74;
constexpr nfs::u32 kKeptFolder = 0x7a2f34;
constexpr nfs::u32 kDeleteName = 0x6610a0;
constexpr nfs::u32 kListReplays = 0x4475a0;       // eax the list, edx what kind of file
constexpr nfs::u32 kSelectionMoved = 0x4471a0;
constexpr nfs::u32 kUnlink = 0x4e28a0;

constexpr GuestString kExtReplay{ 0x538c14, "rp3" };
constexpr GuestString kExtTournament{ 0x538c10, "trn" };
constexpr GuestString kExtKnockout{ 0x538c18, "kot" };
constexpr GuestString kExtGhost{ 0x538bec, "gst" };
constexpr GuestString kKeptNameFormat{ 0x538c1c, "%s%s." };
constexpr GuestString kBoxTitleFormat{ 0x538c24, "%s..." };
constexpr GuestString kPlain{ 0x538bc0, "%s" };
constexpr GuestString kLapsFormat{ 0x538bc4, "%d %s" };
constexpr GuestString kTwoFormat{ 0x538bcc, "%s, %s" };
constexpr GuestString kRaceFormat{ 0x538bd4, "%s %d" };
constexpr GuestString kLsBox{ 0x538c2c, "DESCRIPTION_BOX" };
constexpr GuestString kLsReplays{ 0x538c3c, "replays" };
constexpr GuestString kLsDelete{ 0x538c44, "DELETE_FILE" };
constexpr GuestString kLsLoadNow{ 0x538c50, "loadnow" };
constexpr GuestString kLsShowTournaments{ 0x538c58, "SHOW_TOURNAMENTS" };
constexpr GuestString kLsShowKnockouts{ 0x538c6c, "SHOW_KNOCKOUTS" };
constexpr GuestString kLsShowGhosts{ 0x538c7c, "SHOW_GHOSTS" };
constexpr GuestString kLsShowReplays{ 0x538c88, "SHOW_REPLAYS" };

/* The name the list has selected. */
nfs::u32 selectedName(const Guest& guest, nfs::u32 list)
{
    return guest.dword(guest.dword(list + kListEntries) + 8 * guest.dword(list + kListSelected));
}

/* A file's extension after its path. */
void appendExtension(Guest& guest, nfs::u32 path, const GuestString& extension)
{
    guest.copyString(path + guest.length(path), extension.address);
}

/* The screen's kinds of file. */
const GuestString& extensionOf(nfs::u32 kind)
{
    static constexpr GuestString kExtensions[] = { kExtReplay, kExtGhost, kExtTournament, kExtKnockout };
    return kExtensions[kind];
}

/* A kept file's player's car, among its cars. */
nfs::u32 playerCarOf(const Guest& guest, nfs::u32 file)
{
    return guest.dword(file + guest.dword(file + 0x19b4) * 0x6c + 0x1a00);
}

}  // namespace

/* sub_4461d0: a kept knockout's description. */
nfs::u32 frontend::misc::loadSaveKnockoutInfo(App* app, ThreadState& ts, nfs::u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, path))
        return 0;
    const nfs::u32 file = *fe::direct<0x4e0fe0, ea::loadQuietly>(app, ts, path, 0x10);
    if (file == 0)
        return 0;
    const std::int32_t round = signedWord(guest, file + 0x187c);
    const std::int32_t track = signedWord(guest, file + 2 * nfs::u32(round) + 0x18be);
    if (track <= 8 && track >= 0 && fe::direct<0x478090, loada::savedResolvePlayers>(app, ts, file) != 0
        && fe::direct<0x478100, loada::savedResolveEntries>(app, ts, file) != 0)
    {
        const nfs::u32 level = guest.dword(file + 0x1930) == 0 ? 0x190 : 0x191;
        const nfs::u32 car = playerCarOf(guest, file);
        const nfs::u32 kind = fe::text(guest, 0xd7);
        const nfs::u32 levelName = fe::text(guest, level);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescription, kTwoFormat.address, guest::Words{levelName, kind});
        const nfs::u32 trackName = fe::text(guest, nfs::u32(track + 0x66));
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine2, kPlain.address, guest::Words{trackName});
        const nfs::u32 carName = *fe::direct<0x439250, load::nameOf>(app, ts, car);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine3, kPlain.address, guest::Words{carName});
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

/* sub_4462e0: a kept tournament's description. */
nfs::u32 frontend::misc::loadSaveTournamentInfo(App* app, ThreadState& ts, nfs::u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    constexpr nfs::u32 kTournamentTracks = 0x55d004;
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, path))
        return 0;
    const nfs::u32 file = *fe::direct<0x4e0fe0, ea::loadQuietly>(app, ts, path, 0x10);
    if (file == 0)
        return 0;
    const std::int32_t round = signedWord(guest, file + 0x18cc);
    const std::int32_t track = signedDword(guest, 4 * nfs::u32(round) + kTournamentTracks);
    if (track < 8 && track >= 0 && fe::direct<0x478090, loada::savedResolvePlayers>(app, ts, file) != 0
        && fe::direct<0x478100, loada::savedResolveEntries>(app, ts, file) != 0)
    {
        const nfs::u32 level = guest.dword(file + 0x1930) == 0 ? 0x190 : 0x191;
        const nfs::u32 car = playerCarOf(guest, file);
        const nfs::u32 kind = fe::text(guest, 0xd6);
        const nfs::u32 levelName = fe::text(guest, level);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescription, kTwoFormat.address, guest::Words{levelName, kind});
        const nfs::u32 trackName = fe::text(guest, nfs::u32(track + 0x66));
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine2, kPlain.address, guest::Words{trackName});
        const nfs::u32 carName = *fe::direct<0x439250, load::nameOf>(app, ts, car);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine3, kPlain.address, guest::Words{carName});
        const nfs::u32 points = nfs::u32(signedWord(guest, file + 0x190e));
        const nfs::u32 pointsWord = fe::text(guest, 0xf0);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine4, kRaceFormat.address, guest::Words{pointsWord, points});
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

/* sub_446420: a kept ghost's description. */
nfs::u32 frontend::misc::loadSaveGhostInfo(App* app, ThreadState& ts, nfs::u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, path))
        return 0;
    const nfs::u32 file = *fe::direct<0x4e0fe0, ea::loadQuietly>(app, ts, path, 0x10);
    if (file == 0)
        return 0;
    const std::int32_t track = signedDword(guest, file + 0x196c);
    if (track <= 8 && track >= 0 && fe::direct<0x478090, loada::savedResolvePlayers>(app, ts, file) != 0
        && fe::direct<0x478100, loada::savedResolveEntries>(app, ts, file) != 0)
    {
        const nfs::u32 car = playerCarOf(guest, file);
        const nfs::u32 trackName = fe::text(guest, nfs::u32(track + 0x66));
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescription, kPlain.address, guest::Words{trackName});
        const nfs::u32 lapsWord = fe::text(guest, 5);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine2, kLapsFormat.address, guest::Words{guest.dword(file + 0x1974), lapsWord});
        const nfs::u32 carName = *fe::direct<0x439250, load::nameOf>(app, ts, car);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDescriptionLine3, kPlain.address, guest::Words{carName});
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
}

namespace
{

/* What a file loaded over the race's setup must not touch: the player's
 * preferences and the two players' controller ids. */
struct KeptSettings
{
    std::array<nfs::u32, 0x5db> preferences;
    std::array<nfs::u32, 3> first, second;

    void keep(const Guest& guest)
    {
        for (nfs::u32 i = 0; i < preferences.size(); ++i)
            preferences[i] = guest.dword(SETTINGS_ADDRESS(language) + 4 * i);
        for (nfs::u32 i = 0; i < 3; ++i)
        {
            first[i] = gameSettings(guest).players[0].digital[i];
            second[i] = gameSettings(guest).players[1].digital[i];
        }
    }

    void putBack(Guest& guest) const
    {
        for (nfs::u32 i = 0; i < preferences.size(); ++i)
            guest.setDword(SETTINGS_ADDRESS(language) + 4 * i, preferences[i]);
        for (nfs::u32 i = 0; i < 3; ++i)
        {
            gameSettings(guest).players[0].digital[i] = first[i];
            gameSettings(guest).players[1].digital[i] = second[i];
        }
    }
};

/* sub_4465e0 and sub_446800, the same code: a kept tournament or knockout
 * loaded: the whole race's setup taken from it. */
nfs::u32 loadKeptSeriesBody(nfs::App* app, nfs::ThreadState& ts, Guest& guest, nfs::u32 path)
{
    constexpr nfs::u32 kRaceSetupBytes = 0x20f0;
    KeptSettings kept;
    kept.keep(guest);
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, path))
        return 0;
    const nfs::u32 file = *fe::direct<0x4e0fe0, ea::loadQuietly>(app, ts, path, 0x10);
    if (file == 0)
        return 0;
    nfs::u32 loaded = 0;
    if (fe::direct<0x478090, frontend::loada::savedResolvePlayers>(app, ts, file) != 0)
    {
        fe::direct<0x4ea4f0, ea::moveMemory>(app, ts, SETTINGS_ADDRESS(fileSize), file, kRaceSetupBytes);
        kept.putBack(guest);
        loaded = 1;
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, file);
    return loaded;
}

}  // namespace

/* sub_4465e0: a kept knockout loaded. */
nfs::u32 frontend::misc::loadSaveLoadKnockout(App* app, ThreadState& ts, u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x178c);
    return loadKeptSeriesBody(app, ts, guest, path);
}

/* sub_446800: a kept tournament loaded. */
nfs::u32 frontend::misc::loadSaveLoadTournament(App* app, ThreadState& ts, u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x178c);
    return loadKeptSeriesBody(app, ts, guest, path);
}

/* sub_4466a0: a kept ghost loaded, and the track's record's holder looked up. */
nfs::u32 frontend::misc::loadSaveLoadGhost(App* app, ThreadState& ts, nfs::u32 path)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x1784);
    KeptSettings kept;
    kept.keep(guest);
    fe::direct<0x472820, load::configSaved>(app, ts);
    const nfs::u32 loaded = fe::direct<0x477b50, loopa::loadGhost>(app, ts, path);
    if (loaded == 0)
        return loaded;
    kept.putBack(guest);
    const nfs::u32 holder = guest::direct<0x4cef00, recordsHolder>(
        app, ts, nfs::u32(std::int32_t(std::int16_t(gameSettings(guest).track))));
    const nfs::u32 widened = nfs::u32(std::int32_t(std::int16_t(holder)));
    gameSettings(guest).recordsHolder = widened;
    gameSettings(guest).recordsHolderKept = widened;
    return 1;
}

namespace
{

/* sub_446ad0, sub_446b10, sub_446b60 and sub_446ba0: a Show button: the list
 * made of that kind of file. */
nfs::u32 showKind(nfs::App* app, nfs::ThreadState& ts, Guest& guest, const GuestString& extension,
                    nfs::u32 kind)
{
    const nfs::u32 list = guest.dword(kReplayList);
    if (list == 0)
        return 0;
    fe::direct<0x4475a0, load::replaysListed>(app, ts, list, extension.address);
    const nfs::u32 again = guest.dword(kReplayList);
    guest.setDword(kShownKind, kind);
    guest::direct<0x4471a0, frontend::misc::loadSaveSelectionMoved>(app, ts, again,
                                                                     guest.dword(again + kListSelected));
    return 2;
}

}  // namespace

/* sub_446ad0: the kept tournaments shown. */
nfs::u32 frontend::misc::loadSaveShowTournaments(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return showKind(app, ts, guest, kExtTournament, 2);
}

/* sub_446b10: the kept knockouts shown. */
nfs::u32 frontend::misc::loadSaveShowKnockouts(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return showKind(app, ts, guest, kExtKnockout, 3);
}

/* sub_446b60: the kept replays shown. */
nfs::u32 frontend::misc::loadSaveShowReplays(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return showKind(app, ts, guest, kExtReplay, 0);
}

/* sub_446ba0: the kept ghosts shown. */
nfs::u32 frontend::misc::loadSaveShowGhosts(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return showKind(app, ts, guest, kExtGhost, 1);
}

/* sub_446bf0: the file cannot be loaded. */
nfs::u32 frontend::misc::loadSaveLoadFailed(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    constexpr nfs::u32 kLines = 0x6611a0;
    guest.setDword(kLines, fe::text(guest, 0x41));
    guest.setDword(kLines + 4, fe::text(guest, 0x1c7));
    return openMessage(app, ts, 2, kLines, kLongMessage);
}

/* sub_446c50: Load Now's click: what the list has selected, loaded.  The race
 * just over (a line 0x143 in the list) has no file: its path empty. */
nfs::u32 frontend::misc::loadSaveLoadNow(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0xfc);
    const nfs::u32 list = guest.dword(kReplayList);
    if (list == 0)
        return 0;
    const std::int32_t selected = signedDword(guest, list + kListSelected);
    if (selected >= signedDword(guest, list + kListCount))
        return 2;
    if (selected < 0)
        return 0;
    const nfs::u32 path = guest.frame();      // char[0xfc]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kKeptNameFormat.address, guest::Words{kKeptFolder, selectedName(guest, list)});
    const nfs::u32 kind = guest.dword(kShownKind);
    if (kind > 3)
        return 2;
    nfs::u32 success = 6;
    if (kind == 0)
    {
        const nfs::u32 keptName = fe::text(guest, 0x143);
        if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), selectedName(guest, guest.dword(kReplayList)), keptName) == 0)
            guest.setByte(path, 0);
        else if (guest.byte(path) != 0)
            appendExtension(guest, path, kExtReplay);
        success = 7;
    }
    else
    {
        if (guest.byte(path) != 0)
            appendExtension(guest, path, extensionOf(kind));
    }
    // The kind's loader: the kept replay's the game's, the others this file's.
    nfs::u32 loaded;
    if (kind == 0)
        loaded = *fe::direct<0x446990, load::keptReplayLoaded>(app, ts, path);
    else if (kind == 2)
        loaded = guest::direct<0x446800, loadSaveLoadTournament>(app, ts, path);
    else if (kind == 3)
        loaded = guest::direct<0x4465e0, loadSaveLoadKnockout>(app, ts, path);
    else
        loaded = guest::direct<0x4466a0, loadSaveLoadGhost>(app, ts, path);
    if (loaded != 0)
        return success;
    guest::direct<0x446bf0, loadSaveLoadFailed>(app, ts);
    return 2;
}

/* sub_446ea0: the file selected deleted, the list made anew. */
nfs::u32 frontend::misc::loadSaveDeleteSelected(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const nfs::u32 list = guest.dword(kReplayList);
    if (signedDword(guest, list + kListSelected) >= signedDword(guest, list + kListCount))
    {
        guest.setByte(kDeleteName, 0);
        return list & 0xffff00ff;
    }
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kDeleteName, kKeptNameFormat.address, guest::Words{kKeptFolder, selectedName(guest, list)});
    const nfs::u32 kind = guest.dword(kShownKind);
    if (kind <= 3)
    {
        const GuestString& extension = extensionOf(kind);
        appendExtension(guest, kDeleteName, extension);
        callAt(guest, kUnlink, fe::args({ kDeleteName, extension.address }));
                // Its kind of file stays in edx from the call above.
        fe::direct<kListReplays, load::replaysListed>(app, ts, guest.dword(kReplayList), extension.address);
    }
    const nfs::u32 again = guest.dword(kReplayList);
    return guest::direct<0x4471a0, loadSaveSelectionMoved>(app, ts, again, guest.dword(again + kListSelected));
}

/* sub_446ff0: "delete?" answered. */
nfs::u32 frontend::misc::loadSaveDeleteAnswered(App* app, ThreadState& ts, nfs::u32 answer, nfs::u32 closes)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const nfs::u32 focus = fe::dialogFocus(guest);
    guest.setByte(closes, 0);
    if (answer == fe::kDialogEscape)
    {
        guest.setByte(closes, 1);
    }
    else if (answer == fe::kDialogEnter && focus <= 1)
    {
        if (focus == 0)
            guest::direct<0x446ea0, loadSaveDeleteSelected>(app, ts);
        guest.setByte(closes, 1);
    }
    return 0;
}

/* sub_447030: Delete's click: asked first, naming the file. */
nfs::u32 frontend::misc::loadSaveDeleteClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    constexpr nfs::u32 kLines = 0x6611a8, kButtons = 0x6611b0;
    const nfs::u32 list = guest.dword(kReplayList);
    if (signedDword(guest, list + kListSelected) >= signedDword(guest, list + kListCount))
    {
        guest.setByte(kDeleteName, 0);
        return 2;
    }
    guest.copyString(kDeleteName, selectedName(guest, list));
    guest.setDword(kLines, fe::text(guest, 0x144));
    guest.setDword(kLines + 4, kDeleteName);
    guest.setDword(kButtons, fe::text(guest, 0xc4));
    guest.setDword(kButtons + 4, fe::text(guest, 0xc5));
    fe::Question question{};
    question.lineCount = 2;
    question.lines = kLines;
    question.buttonCount = 2;
    question.buttons = kButtons;
    question.focus = 1;
    question.callback = 0x446ff0;
    fe::openQuestion(guest, question);
    return 2;
}

/* sub_4470e0: the description's box drawn: its title and its lines, those it
 * has.  (Eax left by the last line, read by no caller.) */
nfs::u32 frontend::misc::loadSaveDrawDescription(App* app, ThreadState& ts, nfs::u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x104);
    const std::int32_t itemX = signedWord(guest, item + fe::item::kX);
    const std::int32_t itemY = signedWord(guest, item + fe::item::kY);
    const nfs::u32 title = guest.frame();     // char[0x100]
    const nfs::u32 heading = fe::text(guest, 0x1e0);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), title, kBoxTitleFormat.address, guest::Words{heading});
    // A line of the menus' text, port_text.cpp's (stdcall, the alignment and the colour pushed): its eax.
    const auto draw = [&](nfs::u32 text, std::int32_t x, std::int32_t y, nfs::u32 colour) {
        return nfs::u32(
            *fe::direct<0x452190, port::text::menuTextDrawn>(app, ts, ts.sp - 4 - 8, text, x, y, 0x12, 3, colour));
    };
    draw(title, itemX + 10, itemY, 0xff5abeff);
    nfs::u32 left = 0;
    for (nfs::u32 line = 0; line < 10; ++line)
    {
        const nfs::u32 offset = line * 0x78;
        left = offset;
        if (guest.byte(kDescription + offset) != 0)
            left = draw(kDescription + offset, itemX + 20, itemY + 0x19 + 0x14 * std::int32_t(line), 0xff649dfd);
    }
    return left;
}

/* sub_4471a0, the list's selection moved: the description of the one selected,
 * a kept file's own read from it.  (Edx left as the list's selection, read by
 * no caller.) */
nfs::u32 frontend::misc::loadSaveSelectionMoved(App* app, ThreadState& ts, u32 /*list*/, u32 /*selected*/)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0xfc);
        const nfs::u32 zeroed = fe::direct<kFillZero, ea::clearBytes>(app, ts, kDescription, kDescriptionBytes);
    const nfs::u32 list = guest.dword(kReplayList);
    if (list == 0)
        return zeroed;
    const std::int32_t selected = signedDword(guest, list + kListSelected);
    if (selected >= signedDword(guest, list + kListCount) || selected < 0)
        return list;
    const nfs::u32 path = guest.frame();      // char[0xfc]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), path, kKeptNameFormat.address, guest::Words{kKeptFolder, selectedName(guest, list)});
    const nfs::u32 kind = guest.dword(kShownKind);
    if (kind > 3)
        return kind;
    if (kind == 0)
    {
        const nfs::u32 keptName = fe::text(guest, 0x143);
        if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), selectedName(guest, guest.dword(kReplayList)), keptName) == 0)
            guest.setByte(path, 0);
        else if (guest.byte(path) != 0)
            appendExtension(guest, path, kExtReplay);
        return *fe::direct<0x446060, load::replayInfoShown>(app, ts, path);
    }
    if (guest.byte(path) != 0)
        appendExtension(guest, path, extensionOf(kind));
    if (kind == 2)
        return guest::direct<0x4462e0, loadSaveTournamentInfo>(app, ts, path);
    if (kind == 3)
        return guest::direct<0x4461d0, loadSaveKnockoutInfo>(app, ts, path);
    return guest::direct<0x446420, loadSaveGhostInfo>(app, ts, path);
}

/* sub_447470, loadsave as it opens: the replays listed first. */
nfs::u32 frontend::misc::loadSaveOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setDword(kReplayList, 0);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kDescription, kDescriptionBytes);
    if (Item box = fe::findItem(guest, menu, kLsBox))
        guest.setDword(box.address() + kOnDrawBox, 0x4470e0);
    if (Item list = fe::findItem(guest, menu, kLsReplays))
    {
        guest.setDword(kReplayList, list.address());
        guest.setDword(list.address() + kListOnMove, kSelectionMoved);
    }
    Item erase = fe::findItem(guest, menu, kLsDelete);
    guest.setDword(kDeleteButton, erase.address());
    if (erase)
        erase.setOnClick(0x447030);
    Item load = fe::findItem(guest, menu, kLsLoadNow);
    guest.setDword(kLoadButton, load.address());
    if (load)
        load.setOnClick(0x446c50);
    if (Item show = fe::findItem(guest, menu, kLsShowTournaments))
        show.setOnClick(0x446ad0);
    if (Item show = fe::findItem(guest, menu, kLsShowKnockouts))
        show.setOnClick(0x446b10);
    if (Item show = fe::findItem(guest, menu, kLsShowGhosts))
        show.setOnClick(0x446ba0);
    if (Item show = fe::findItem(guest, menu, kLsShowReplays))
        show.setOnClick(0x446b60);
    if (const nfs::u32 list = guest.dword(kReplayList))
        fe::direct<0x4475a0, load::replaysListed>(app, ts, list, kExtReplay.address);
    const nfs::u32 list = guest.dword(kReplayList);
    guest.setDword(kShownKind, 0);
    guest::direct<0x4471a0, loadSaveSelectionMoved>(app, ts, list, guest.dword(list + kListSelected));
    return 0;
}

/* sub_447370, loadsave on every pass: the box and the buttons as the list has
 * it, and the list's title. */
nfs::u32 frontend::misc::loadSaveOnFrame(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (Item box = fe::findItem(guest, menu, kLsBox))
        box.setHidden(guest.byte(kDescription) == 0);
    const auto nothingSelected = [&](nfs::u32 list) {
        const std::int32_t count = signedDword(guest, list + kListCount);
        return count == 0 || count <= signedDword(guest, list + kListSelected);
    };
    if (const nfs::u32 erase = guest.dword(kDeleteButton))
    {
        Item(guest, erase).setDisabled(false);
        bool close = nothingSelected(guest.dword(kReplayList));
        if (!close && guest.dword(kShownKind) == 0)
        {
            // The race just over has a line in the list, and no file.
            const nfs::u32 keptName = fe::text(guest, 0x143);
            const nfs::u32 name = selectedName(guest, guest.dword(kReplayList));
            close = fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), name, keptName) == 0;
        }
        if (close)
            Item(guest, guest.dword(kDeleteButton)).setDisabled(true);
    }
    if (const nfs::u32 load = guest.dword(kLoadButton))
        Item(guest, load).setDisabled(nothingSelected(guest.dword(kReplayList)));
    static constexpr std::uint32_t kTitles[] = { 0x13f, 0x140, 0x141, 0x142 };
    const nfs::u32 kind = guest.dword(kShownKind);
    if (kind <= 3)
    {
        const nfs::u32 title = fe::text(guest, kTitles[kind]);
        guest.setDword(guest.dword(kReplayList) + fe::item::kText, title);
    }
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * credits
 * ------------------------------------------------------------------------- */

// The pages: 16 bytes each, a kind (0 and 1 a person's page with the picture
// and its signature, 2 and 3 lines of text, 4 a picture), its first line,
// its last line and the picture's name.
constexpr nfs::u32 kPages = 0x555758;
constexpr nfs::u32 kPageFirstLine = 0x55575c;
constexpr nfs::u32 kPageLastLine = 0x555760;
constexpr nfs::u32 kPagePicture = 0x555764;
constexpr nfs::u32 kCreditsArt = 0x555754;        // the pictures' file
constexpr nfs::u32 kCreditsPage = 0x555c28;       // the page asked for
constexpr nfs::u32 kCreditsX = 0x555c2c;
constexpr nfs::u32 kCreditsSpeed = 0x555c30;
constexpr nfs::u32 kCreditsShown = 0x555c34;      // the page drawn
constexpr nfs::u32 kCreditsCanvas = 0x555c38;
constexpr nfs::u32 kCreditsFont = 0x555c3c;
constexpr nfs::u32 kCreditsItalics = 0x555c40;
constexpr nfs::u32 kCreditsHeld = 0x555c44;       // passes the button has been down
constexpr nfs::u32 kCreditsState = 0x5f40c0;
// The states: sliding in, from the right or the left; sliding out, to the left
// or the right; still.
constexpr std::uint32_t kInFromRight = 0, kInFromLeft = 1, kOutToLeft = 2, kOutToRight = 3, kStill = 4;
constexpr std::int32_t kSlideStart = 0x280;
constexpr std::int32_t kSlideFar = 0x3c0;
constexpr std::int32_t kSpeedStart = 0x3f;
constexpr std::int32_t kSpeedMost = 0x3c;
constexpr nfs::u32 kBonusCarLine = 0x46a;         // a page only for those with the bonus car

constexpr nfs::u32 kNextPage = 0x442370;
constexpr nfs::u32 kPreviousPage = 0x4423e0;
// The drawing library (native_frontend_eaa/eab's cores): a canvas opened, what is drawn goes to it.
constexpr nfs::u32 kLocateShape = 0x4ef270;
constexpr std::int32_t kCanvasWidth = 0x260, kCanvasHeight = 0x140;
constexpr nfs::u32 kColourWhite = 0xffffff, kColourHeading = 0xffe440, kColourName = 0xffdf40;
constexpr nfs::u32 kDisplayDepth = 0x565010;      // byte

constexpr GuestString kCrSttl{ 0x537b90, "sttl" };
constexpr GuestString kCrVanc{ 0x537b98, "vanc" };
constexpr GuestString kCrDummy{ 0x537ba0, "dumm" };
constexpr GuestString kCrSignature{ 0x537ba8, "sig%d" };
constexpr GuestString kCrArtFormat{ 0x537bb0, "%scred.qfs" };
constexpr GuestString kCrNext{ 0x537bbc, "next" };
constexpr GuestString kCrPrevious{ 0x537bc4, "previous" };
constexpr GuestString kCrMore{ 0x537bd0, "More" };
constexpr GuestString kCrMoreFormat{ 0x537bd8, "%sIN_%d.fsh" };
constexpr GuestString kCrRb{ 0x537be4, "rb" };
constexpr GuestString kCrFont{ 0x537be8, "%scred.ffn" };
constexpr GuestString kCrItalics{ 0x537bf4, "%scredi.ffn" };
constexpr GuestString kCrItalics10{ 0x537c00, "%scred10i.ffn" };

}  // namespace

/* sub_439ce0: whether the bonus car is installed. */
nfs::u32 frontend::misc::creditsHaveBonusCar(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8 + 0x10);
    constexpr nfs::u32 kBonusCar = 0x25;
    const nfs::u32 id = guest.frame();
    fe::direct<0x438ad0, load::makeId>(app, ts, id, kBonusCar);
    return fe::direct<0x439990, load::placeById>(app, ts, id) == nfs::u32(-1) ? 0 : 1;
}

namespace
{

void setColours(Guest& guest, nfs::ThreadState& ts, nfs::u32 background, nfs::u32 colour)
{
    const nfs::u32 packedBackground = fe::direct<0x4ef760, frontend::eab::colourPack>(guest.app(), ts, background);
    const nfs::u32 packed = fe::direct<0x4ef760, frontend::eab::colourPack>(guest.app(), ts, colour);
    fe::direct<0x4ef830, frontend::eab::fontSetColours>(guest.app(), ts, packed, packedBackground);
}

void drawText(Guest& guest, nfs::ThreadState& ts, nfs::u32 text, std::int32_t x, std::int32_t y)
{
    fe::direct<0x4efe30, frontend::eab::textDrawAt>(guest.app(), ts, text, nfs::u32(x), nfs::u32(y));
}

/* A line with its shadow, a pixel off. */
void drawShadowed(Guest& guest, nfs::ThreadState& ts, nfs::u32 text, std::int32_t x, std::int32_t y, nfs::u32 colour)
{
    setColours(guest, ts, 0, 0);
    drawText(guest, ts, text, x + 1, y + 1);
    setColours(guest, ts, 0, colour);
    drawText(guest, ts, text, x, y);
}

/* A shape in the canvas's middle. */
void drawCentred(Guest& guest, nfs::ThreadState& ts, nfs::u32 shape)
{
    const std::int32_t y = (kCanvasHeight - signedWord(guest, shape + 6)) / 2;
    const std::int32_t x = (kCanvasWidth - signedWord(guest, shape + 4)) / 2;
    fe::direct<0x4ef3f0, frontend::eab::shapeDrawKeyed>(guest.app(), ts, shape, nfs::u32(x), nfs::u32(y));
}

}  // namespace

/* sub_441cb0: the page asked for, drawn into a canvas made into a texture,
 * `previous`'s reused: the texture.  A person's page, by the language: the
 * five titles down the side, and beside them the person's name and lines. */
nfs::u32 frontend::misc::creditsRenderPage(App* app, ThreadState& ts, nfs::u32 previous)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x40);
    constexpr std::uint32_t kPersonTitles = 0x1c8;
    const nfs::u32 canvas = fe::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, ts, kCanvasWidth, kCanvasHeight, 0x10, 0);
    fe::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, canvas);   // the canvas as it came back in eax
    fe::direct<0x4ef1b0, eab::drawClearClip>(app, ts, guest.byte(kDisplayDepth) == 8 ? 0xffu : 0u);
    const nfs::u32 page = guest.dword(kCreditsPage) << 4;
    const nfs::u32 kind = guest.dword(kPages + page);
    if (kind == 0 || kind == 1)
    {
        if (const nfs::u32 art = guest.dword(kCreditsArt))
        {
            drawCentred(guest, ts, fe::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, art, kind == 1 ? kCrSttl.address : kCrVanc.address));
            nfs::u32 picture = callAt(guest, kLocateShape, fe::args({ guest.dword(kCreditsArt), guest.dword(page + kPagePicture) }));
            if (picture == 0)
                picture = callAt(guest, kLocateShape, fe::args({ guest.dword(kCreditsArt), kCrDummy.address }));
            fe::direct<0x4ef3f0, eab::shapeDrawKeyed>(app, ts, picture, 0x19c, 0x4c);
            const nfs::u32 name = guest.frame() + 0x14;   // char[]
            const nfs::u32 number = nfs::u32(signedDword(guest, kCreditsPage) % 5 + 1);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kCrSignature.address, guest::Words{number});
            const nfs::u32 signature = callAt(guest, kLocateShape, fe::args({ guest.dword(kCreditsArt), name }));
            if (signature != 0)
                fe::direct<0x4ef3f0, eab::shapeDrawKeyed>(app, ts, signature, 0x19c, 0xfa);
        }
        const nfs::u32 first = guest.dword(page + kPageFirstLine);
        if (gameSettings(guest).language != 0)
        {
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsItalics));
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const nfs::u32 title = fe::text(guest, kPersonTitles + i);
                drawShadowed(guest, ts, title, 0x32, i * 30 + 0x7d, kColourHeading);
            }
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsFont));
            setColours(guest, ts, 0, 0);
            drawText(guest, ts, fe::text(guest, first), 0x33, 0x65);
            setColours(guest, ts, 0, kColourName);
            drawText(guest, ts, fe::text(guest, first), 0x32, 0x64);
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const nfs::u32 line = first + 1 + nfs::u32(i);
                setColours(guest, ts, 0, 0);
                drawText(guest, ts, fe::text(guest, line), 0x51, i * 30 + 0x8b);
                setColours(guest, ts, 0, kColourWhite);
                drawText(guest, ts, fe::text(guest, line), 0x50, i * 30 + 0x8a);
            }
        }
        else
        {
            std::array<nfs::u32, 5> widths;
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsItalics));
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const nfs::u32 title = fe::text(guest, kPersonTitles + i);
                drawShadowed(guest, ts, title, 0x32, i * 25 + 0x8c, kColourWhite);
                widths[i] = fe::direct<0x4efef0, eab::textMeasureAll>(app, ts, title);
            }
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsFont));
            setColours(guest, ts, 0, 0);
            drawText(guest, ts, fe::text(guest, first), 0x33, 0x74);
            setColours(guest, ts, 0, kColourName);
            drawText(guest, ts, fe::text(guest, first), 0x32, 0x73);
            for (std::int32_t i = 0; i < 5; ++i)
            {
                const std::int32_t x = std::int32_t(widths[i]) + 0x3c;
                const nfs::u32 line = first + 1 + nfs::u32(i);
                setColours(guest, ts, 0, 0);
                drawText(guest, ts, fe::text(guest, line), x + 1, i * 25 + 0x8d);
                setColours(guest, ts, 0, kColourWhite);
                drawText(guest, ts, fe::text(guest, line), x, i * 25 + 0x8c);
            }
        }
    }
    else if (kind == 2)
    {
        // Lines down a column, marked in their first characters: '$' starts the
        // second column, '#' and '@' a heading, '#' with a line left after it.
        std::int32_t x = 0, y = 0x14;
        for (std::int32_t line = signedDword(guest, page + kPageFirstLine);
             line <= signedDword(guest, (guest.dword(kCreditsPage) << 4) + kPageLastLine); ++line)
        {
            nfs::u32 text = fe::text(guest, nfs::u32(line));
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsFont));
            nfs::u32 colour = kColourWhite;
            std::int32_t indent = 0x1e;
            bool blankAfter = false;
            for (;; ++text)
            {
                const std::uint8_t mark = guest.byte(text);
                if (mark == '$')
                {
                    x = 0x130;
                    y = 0x14;
                }
                else if (mark == '#')
                {
                    blankAfter = true;
                    colour = kColourHeading;
                    indent = 0;
                }
                else if (mark == '@')
                {
                    colour = kColourHeading;
                    indent = 0;
                }
                else
                {
                    break;
                }
            }
            const std::int32_t at = x + indent;
            setColours(guest, ts, 0, 0);
            drawText(guest, ts, text, at, y);
            setColours(guest, ts, 0, colour);
            drawText(guest, ts, text, at + 1, y + 1);
            y += blankAfter ? 0x28 : 0x14;
        }
    }
    else if (kind == 3)
    {
        // Lines in the middle.
        std::int32_t y = 0x14;
        for (std::int32_t line = signedDword(guest, page + kPageFirstLine);
             line <= signedDword(guest, (guest.dword(kCreditsPage) << 4) + kPageLastLine); ++line)
        {
            const nfs::u32 text = fe::text(guest, nfs::u32(line));
            fe::direct<0x4ef520, eab::fontUse>(app, ts, guest.dword(kCreditsFont));
            const std::int32_t x = 0x130 - std::int32_t(fe::direct<0x4efef0, eab::textMeasureAll>(app, ts, text)) / 2;
            setColours(guest, ts, 0, 0);
            drawText(guest, ts, text, x, y);
            setColours(guest, ts, 0, kColourWhite);
            drawText(guest, ts, text, x + 1, y + 1);
            y += 0x14;
        }
    }
    else if (kind == 4)
    {
        if (const nfs::u32 art = guest.dword(kCreditsArt))
        {
            drawCentred(guest, ts, fe::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, art, kCrSttl.address));
            const nfs::u32 picture = guest.dword((guest.dword(kCreditsPage) << 4) + kPagePicture);
            drawCentred(guest, ts, fe::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, guest.dword(kCreditsArt), picture));
        }
    }
    fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    const nfs::u32 texture = nfs::u32(*fe::direct<0x4d6390, port::shapes::artTexturesOf>(
        app, ts, ts.sp - 4, guest.dword(canvas + 0x20), std::int32_t(previous), 0));
    fe::direct<0x4eff00, eab::windowBitmapFree>(app, ts, canvas);
    return texture;
}

/* sub_442370, next's click: on to the next page, once it is still; round to
 * the first after the last; past the bonus car's without the car. */
nfs::u32 frontend::misc::creditsNextPage(App* app, ThreadState& ts, u32 /*item*/)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (guest.dword(kCreditsState) != kStill)
        return 0;
    const nfs::u32 page = guest.dword(kCreditsPage) + 1;
    const nfs::u32 line = guest.dword((page << 4) + kPageFirstLine);
    guest.setDword(kCreditsPage, page);
    if (line == 0)
        guest.setDword(kCreditsPage, 0);
    if (guest::direct<0x439ce0, creditsHaveBonusCar>(app, ts) == 0
        && guest.dword((guest.dword(kCreditsPage) << 4) + kPageFirstLine) == kBonusCarLine)
        guest.setDword(kCreditsPage, guest.dword(kCreditsPage) + 1);
    guest.setDword(kCreditsState, kOutToLeft);
    return 0;
}

/* sub_4423e0, previous's click: back a page, round to the last from the first. */
nfs::u32 frontend::misc::creditsPreviousPage(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    nfs::u32 page = guest.dword(kCreditsPage);
    if (guest.dword(kCreditsState) == kStill)
    {
        if (page != 0)
        {
            --page;
            guest.setDword(kCreditsPage, page);
            const nfs::u32 have = guest::direct<0x439ce0, creditsHaveBonusCar>(app, ts);
            page = guest.dword(kCreditsPage);
            if (have == 0 && guest.dword((page << 4) + kPageFirstLine) == kBonusCarLine)
                --page;
        }
        else
        {
            while (guest.dword((page << 4) + kPageFirstLine) != 0)
                ++page;
            --page;
        }
        guest.setDword(kCreditsState, kOutToRight);
    }
    guest.setDword(kCreditsPage, page);
    return 0;
}

/* sub_442850, credits as it opens: the slide's start, the fonts. */
nfs::u32 frontend::misc::creditsOnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x100);
    constexpr nfs::u32 kMoviesFolder = 0x7a3234, kFontsFolder = 0x7a2e34;
    guest.setDword(kCreditsShown, 0);
    guest.setDword(kCreditsPage, 0);
    guest.setDword(kCreditsState, kInFromRight);
    guest.setDword(kCreditsSpeed, kSpeedStart);
    guest.setDword(kCreditsArt, 0);
    guest.setDword(kCreditsX, kSlideStart);
    const nfs::u32 name = guest.frame();      // char[0x100]
    if (Item more = fe::findItem(guest, menu, kCrMore))
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kCrMoreFormat.address, guest::Words{kMoviesFolder, 0});
        const nfs::u32 file = fe::direct<kFopen, watcom::clib::fopen>(guest.app(), guest.ts(), name, kCrRb.address);
        if (file != 0)
            fe::direct<kFclose, watcom::clib::fclose>(guest.app(), guest.ts(), file);
        else
            more.setDisabled(true);
    }
    if (Item next = fe::findItem(guest, menu, kCrNext))
        next.setOnClick(kNextPage);
    if (Item previous = fe::findItem(guest, menu, kCrPrevious))
        previous.setOnClick(kPreviousPage);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kCrFont.address, guest::Words{kFontsFolder});
    guest.setDword(kCreditsFont, *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, 0x10));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, gameSettings(guest).language == 0 ? kCrItalics.address : kCrItalics10.address, guest::Words{kFontsFolder});
    guest.setDword(kCreditsItalics, *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, 0x10));
    return 0;
}

/* sub_442980, credits as it closes. */
nfs::u32 frontend::misc::creditsOnExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (const nfs::u32 art = guest.dword(kCreditsArt))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, art);
        guest.setDword(kCreditsArt, 0);
    }
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(kCreditsItalics));
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(kCreditsFont));
    return 0;
}

/* sub_442580, credits on every pass: the page's slide.  A button held down turns
 * pages by itself and doubles the slide. */
nfs::u32 frontend::misc::creditsOnFrame(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame stack(guest, ts, 24 + 0x114);  // its address: `frame`, below
    constexpr nfs::u32 kArtFolder = 0x7a2db4;
    constexpr std::int32_t kAccelerate = 3;
    const nfs::u32 frame = guest.frame();
    const nfs::u32 name = frame;              // char[0x100]
    const nfs::u32 mouseDown = frame + 0x100, mouseX = frame + 0x104, mouseY = frame + 0x108;
    // Whether the item's codelink names `link`: port::menus's (ebx handed on as the code has it).
    const auto names = [&](nfs::u32 at, const GuestString& link) {
        return *fe::direct<0x4429c0, port::menus::codelinkMatched>(app, ts, ts.sp - 4,
                                                                    guest.dword(at + fe::item::kCodelink), link.address,
                                                                    0) != 0;
    };
    if (guest.dword(kCreditsArt) == 0)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, kCrArtFormat.address, guest::Words{kArtFolder});
        // The art's shapes loaded, port_buttons.cpp's (ebx handed on as the code has it).
        const port::Result art =
            fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, name, 0);
        guest.setDword(kCreditsArt, nfs::u32(*art));
        guest.setDword(kCreditsCanvas, guest::direct<0x441cb0, creditsRenderPage>(app, ts, 0));
    }
    fe::direct<0x435310, controls::mouseRead>(app, ts, mouseDown, mouseX, mouseY, 0);
    bool held = false;
    nfs::u32 item = 0;
    if (guest.word(menu + fe::menu::kSelected) != 0x64)
    {
        const std::int32_t selected = signedWord(guest, menu + fe::menu::kSelected);
        const std::int32_t first = signedWord(guest, menu + fe::menu::kFirstItem);
        item = fe::item::kTable + fe::item::kSize * nfs::u32(selected + first);
        if (guest.dword(item) == 5 && (names(item, kCrNext) || names(item, kCrPrevious)) && guest.dword(mouseDown) != 0)
            held = true;
    }
    if (held)
    {
        const nfs::u32 passes = guest.dword(kCreditsHeld) + 1;
        guest.setDword(kCreditsHeld, passes);
        if (std::int32_t(passes) > 10)
        {
            if (names(item, kCrNext))
                guest::direct<0x442370, creditsNextPage>(app, ts, 0);
            else
                guest::direct<0x4423e0, creditsPreviousPage>(app, ts);
        }
    }
    else
    {
        guest.setDword(kCreditsHeld, 0);
    }
    std::int32_t x = signedDword(guest, kCreditsX);
    std::int32_t speed = signedDword(guest, kCreditsSpeed);
    const auto turnPage = [&] {
        const nfs::u32 page = guest.dword(kCreditsPage);
        if (page != guest.dword(kCreditsShown))
        {
            const nfs::u32 canvas = guest.dword(kCreditsCanvas);
            guest.setDword(kCreditsShown, page);
            guest.setDword(kCreditsCanvas, guest::direct<0x441cb0, creditsRenderPage>(app, ts, canvas));
        }
    };
    const auto doubled = [&] { return guest.dword(kCreditsHeld) != 0; };
    switch (guest.dword(kCreditsState))
    {
    case kOutToLeft:
        if (x <= -kSlideFar)
        {
            turnPage();
            x = kSlideStart;
            speed = kSpeedStart;
            guest.setDword(kCreditsState, kInFromRight);
        }
        else
        {
            if (speed == 0 && signedDword(guest, kCreditsHeld) > 10)
                speed = 10 + kAccelerate;
            else if (speed < kSpeedMost)
                speed += kAccelerate;
            x -= doubled() ? 2 * speed : speed;
        }
        break;
    case kOutToRight:
        if (x >= kSlideFar)
        {
            turnPage();
            x = -kSlideStart;
            speed = kSpeedStart;
            guest.setDword(kCreditsState, kInFromLeft);
        }
        else
        {
            if (speed == 0 && guest.dword(kCreditsHeld) != 0)
                speed = 10 + kAccelerate;
            else if (speed < kSpeedMost)
                speed += kAccelerate;
            x += doubled() ? 2 * speed : speed;
        }
        break;
    case kInFromRight:
        if (speed > kAccelerate)
            speed -= kAccelerate;
        x -= doubled() ? 2 * speed : speed;
        if (x <= 0)
        {
            guest.setDword(kCreditsState, kStill);
            x = 0;
            speed = 0;
        }
        break;
    case kInFromLeft:
        if (speed > kAccelerate)
            speed -= kAccelerate;
        x += doubled() ? 2 * speed : speed;
        if (x >= 0)
        {
            guest.setDword(kCreditsState, kStill);
            x = 0;
            speed = 0;
        }
        break;
    default:
        break;
    }
    guest.setDword(kCreditsX, nfs::u32(x));
    guest.setDword(kCreditsSpeed, nfs::u32(speed));
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * credits1, the showcase's
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kGlamPage = 0x664794;
constexpr nfs::u32 kGlamPageShown = 0x664798;
constexpr nfs::u32 kGlamPageKept = 0x664780;
constexpr nfs::u32 kGlamTexts = 0x5563dc;         // int16: a page's title's line
constexpr nfs::u32 kGlamTitle = 0x664700;
constexpr nfs::u32 kGlamNextTitle = 0x664680;
constexpr GuestString kGlamSlider{ 0x5398d4, "ZoomSlider" };
constexpr GuestString kGlamNext{ 0x5398f4, "GlamNext" };
constexpr GuestString kGlamSub{ 0x5398e0, "GlamSub" };
constexpr GuestString kGlamPlain{ 0x5398e8, "%s" };
constexpr GuestString kGlamNextFormat{ 0x5398ec, "%s..." };

std::uint32_t glamText(const Guest& guest, nfs::u32 page)
{
    return nfs::u32(signedWord(guest, kGlamTexts + 2 * page));
}

}  // namespace

/* sub_454d60: the page's title, and on the button the next's, with "..." but in
 * the second language.  (Eax left as each way leaves it, read by no caller.) */
nfs::u32 frontend::misc::credits1ShowPage(App* app, ThreadState& ts, nfs::u32 nextButton)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    Item sub = fe::findItem(guest, fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts), kGlamSub);
    const nfs::u32 title = fe::text(guest, glamText(guest, guest.dword(kGlamPage)));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kGlamTitle, kGlamPlain.address, guest::Words{title});
    sub.setText(kGlamTitle);
    const nfs::u32 language = gameSettings(guest).language;
    const nfs::u32 next = nfs::u32((signedDword(guest, kGlamPage) + 1) % 4);
    const nfs::u32 nextTitle = fe::text(guest, glamText(guest, next));
    nfs::u32 left;
    if (language == 1)
    {
        guest.copyString(kGlamNextTitle, nextTitle);
        left = nextTitle & 0xffffff00;
    }
    else
    {
        left = fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, kGlamNextTitle, kGlamNextFormat.address,
                                                           guest::Words{nextTitle});
    }
    guest.setDword(nextButton + fe::item::kText, kGlamNextTitle);
    return left;
}

/* sub_454e40: next's click. */
nfs::u32 frontend::misc::credits1NextClicked(App* app, ThreadState& ts, nfs::u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    guest.setDword(kGlamPage, nfs::u32((signedDword(guest, kGlamPage) + 1) % 4));
    guest::direct<0x454d60, credits1ShowPage>(app, ts, item);
    return 0;
}

/* sub_454ea0, credits1 as it opens: from the first page, the one it was on kept. */
nfs::u32 frontend::misc::credits1OnEnter(App* app, ThreadState& ts, nfs::u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    guest.setDword(kGlamPageKept, guest.dword(kGlamPage));
    guest.setDword(kGlamPage, 0);
    guest.setDword(kGlamPageShown, 0);
    const nfs::u32 slider = fe::findItem(guest, menu, kGlamSlider).address();
    fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(app, ts, slider, 0x454cf0);
    fe::direct<0x46c950, port::fields::sliderSetValueOf>(app, ts, ts.sp - 4 - 8, slider, 0, 0);
    const nfs::u32 next = fe::findItem(guest, menu, kGlamNext).address();
    guest.setDword(next + fe::item::kOnClick, 0x454e40);
    guest::direct<0x454d60, credits1ShowPage>(app, ts, next);
    return 0;
}

/* sub_454e80, credits1 as it closes: the page it was on. */
nfs::u32 frontend::misc::credits1OnExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const nfs::u32 kept = guest.dword(kGlamPageKept);
    guest.setDword(kGlamPage, kept);
    guest.setDword(kGlamPageShown, kept);
    return 0;
}

namespace
{

/* sub_44e460, sub_44e480, sub_44e4a0 and sub_44e6a0: a Save button's click: the
 * dialog that asks for the name. */
nfs::u32 saveClicked(nfs::App* app, nfs::ThreadState& ts, nfs::u32 what)
{
    fe::direct<0x447a30, frontend::pause::askSaveName>(app, ts, what);
    return 2;
}

const GuestString kLevelStrings[] = {
    kCopAssists[0], kCopAssists[1], kDuelDisabled[0], kDuelDisabled[1], kDuelDisabled[2], kDuelDisabled[3],
    kDuelDisabled[4], kDuelDisabled[5], kDuelDisabled[6], kDuelDisabled[7], kDuelDisabled[8], kDuelDisabled[9],
};
const GuestString kCameraEnterStrings[] = { kCameraEscapeItem, kCameraAssists };
const GuestString kCameraExitStrings[] = { kCameraCancel };
const GuestString kSetupStrings[] = { kSetupOnePlayer, kSetupConnect, kSetupTwoPlayers, kSetupWebSite, kSetupLoad };
const GuestString kSetupNameStrings[] = { kSetupName };
const GuestString kRecordsStrings[] = { kRecordsDeleteItem, kRecordsSheet };
const GuestString kPostgameStrings[] = {
    kPgRaceNow, kPgSpeech, kPgTrackAndLaps, kPgTrackFormat, kPgRecords, kPgSummary, kPgTournament, kPgCop, kPgToggle,
    kPgPerformance, kPgViewReplay, kPgSaveReplay, kPgSaveGhost, kPgSaveTournament, kPgSaveKnockout, kPgMainMenu, kPgQuit,
};
const GuestString kPostgameFrameStrings[] = { kPgRaceNow };
const GuestString kKeptNameStrings[] = { kKeptNameFormat, kExtReplay, kExtTournament, kExtKnockout, kExtGhost };
const GuestString kInfoStrings[] = { kPlain, kLapsFormat, kTwoFormat, kRaceFormat };
const GuestString kLoadSaveStrings[] = {
    kLsBox, kLsReplays, kLsDelete, kLsLoadNow, kLsShowTournaments, kLsShowKnockouts, kLsShowGhosts, kLsShowReplays, kExtReplay,
};
const GuestString kLoadSaveFrameStrings[] = { kLsBox };
const GuestString kDrawDescriptionStrings[] = { kBoxTitleFormat };
const GuestString kRenderPageStrings[] = { kCrSttl, kCrVanc, kCrDummy, kCrSignature };
const GuestString kCreditsEnterStrings[] = { kCrNext, kCrPrevious, kCrMore, kCrMoreFormat, kCrRb, kCrFont, kCrItalics, kCrItalics10 };
const GuestString kCreditsFrameStrings[] = { kCrArtFormat, kCrNext, kCrPrevious };
const GuestString kGlamShowStrings[] = { kGlamSub, kGlamPlain, kGlamNextFormat };
const GuestString kGlamEnterStrings[] = { kGlamSlider, kGlamNext };

constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEdx = fe::kRestoreEbx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kEbxEcxEdxEsi = kEbxEcxEdx | fe::kRestoreEsi;
constexpr std::uint8_t kEbxEcxEsiEdi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;

}

/* sub_45dab0, racetype as it closes: the item selected is the race's type, of six. */
nfs::u32 frontend::misc::raceTypeOnExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    if (std::int16_t(guest.word(menu + fe::menu::kSelected)) < 6)
        gameSettings(guest).raceType = nfs::u32(std::int32_t(guest.dword(menu + 0x18)) >> 16);
    return nfs::u32(0);
}

/* sub_44f5d0: whether the records' file is in memory. */
nfs::u32 frontend::misc::recordsLoaded(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return nfs::u32(guest.dword(kRecordsFile) != 0 ? 1 : 0);
}

/* sub_44e460: Save's click, the dialog that asks for a knockout's name. */
nfs::u32 frontend::misc::postgameSaveKnockoutClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return saveClicked(app, ts, 0);
}

/* sub_44e480: Save's click, the dialog that asks for a tournament's name. */
nfs::u32 frontend::misc::postgameSaveTournamentClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return saveClicked(app, ts, 1);
}

/* sub_44e4a0: Save's click, the dialog that asks for a ghost's name. */
nfs::u32 frontend::misc::postgameSaveGhostClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return saveClicked(app, ts, 3);
}

/* sub_44e6a0: Save's click, the dialog that asks for the replay's name. */
nfs::u32 frontend::misc::postgameSaveReplayClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return saveClicked(app, ts, 2);
}

/* sub_44e690: the menu loop told to quit. */
nfs::u32 frontend::misc::postgameQuitClicked(App*, ThreadState&)
{
    return nfs::u32(0xa);
}

/* sub_44f500, postgame as it closes: the words spoken over the sheets stopped. */
nfs::u32 frontend::misc::postgameOnExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    fe::direct<0x4c99d0, port::spokenStopOf>(app, ts, ts.sp - 4);
    return nfs::u32(0);
}

/* The screens' tables and the items' handlers enter these as the game's code
 * does: a menu in eax, an answer in eax with the byte that closes the dialog in
 * edx, an item in eax. */

nfs::u32 frontend::raceTypeOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45dab0", 0, 0, nullptr, 0, 0 };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::raceTypeOnExit(app, ts, menu); });
}

nfs::u32 frontend::levelOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_45d630", 0, 0, kLevelStrings, std::size(kLevelStrings), kEcxEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::levelOnEnter(app, ts, menu); });
}

nfs::u32 frontend::setupOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452f80", 0, 0, kSetupStrings, std::size(kSetupStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupOnEnter(app, ts, menu); });
}

nfs::u32 frontend::setupOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452f60", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupOnFrame(app, ts); });
}

nfs::u32 frontend::setupRaceAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452ac0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupRaceAnswered(app, ts, answer, closes); });
}

nfs::u32 frontend::setupAskRace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452bd0", 0, 0, nullptr, 0 };
    const nfs::u32 canCancel = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupAskRace(app, ts, canCancel); });
}

nfs::u32 frontend::setupTwoPlayersClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452d40", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupTwoPlayersClicked(app, ts); });
}

nfs::u32 frontend::setupOnePlayerClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452f20", 0, 0, nullptr, 0, kEbxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupOnePlayerClicked(app, ts); });
}

nfs::u32 frontend::setupWebSiteRefused(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452d80", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupWebSiteRefused(app, ts, answer, closes); });
}

nfs::u32 frontend::setupIsCurrent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452db0", 0, 0, kSetupNameStrings, std::size(kSetupNameStrings), fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupIsCurrent(app, ts); });
}

nfs::u32 frontend::setupWebSiteAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452de0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupWebSiteAnswered(app, ts, answer, closes); });
}

nfs::u32 frontend::setupWebSiteClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_452e60", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::setupWebSiteClicked(app, ts); });
}

nfs::u32 frontend::resultsParticipantById(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4554e0", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const nfs::u32 group = args.eax, id = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsParticipantById(app, ts, group, id); });
}

nfs::u32 frontend::resultsParticipantRanks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455530", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 found = args.eax, a = args.edx, b = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsParticipantRanks(app, ts, found, a, b); });
}

nfs::u32 frontend::resultsParticipantMatches(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4557b0", 0, 0, nullptr, 0, 0 };
    const nfs::u32 found = args.eax, mask = args.edx, state = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsParticipantMatches(app, ts, found, mask, state); });
}

nfs::u32 frontend::resultsCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455840", 0, 0, nullptr, 0, kEbxEcxEsiEdi };
    const nfs::u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsCompare(app, ts, a, b); });
}

nfs::u32 frontend::resultsSort(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455c10", 0, 0, nullptr, 0, fe::kRestoreEcx | kEsiEdi };
    const nfs::u32 group = args.eax, key = args.edx, descending = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsSort(app, ts, group, key, descending); });
}

nfs::u32 frontend::resultsNth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455cd0", 0, 0, nullptr, 0, kEsiEdi };
    const nfs::u32 group = args.eax, index = args.edx, mask = args.ebx, state = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::resultsNth(app, ts, group, index, mask, state); });
}

nfs::u32 frontend::resultsNthSorted(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_455dd0", 8, 0, nullptr, 0, kEsiEdi };
    const nfs::u32 group = args.eax, index = args.edx, key = args.ebx, descending = args.ecx;
    const nfs::u32 mask = app->getMemory<nfs::u32>(ts.sp + 4), state = app->getMemory<nfs::u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return misc::resultsNthSorted(app, ts, group, index, key, descending, mask, state);
    });
}

nfs::u32 frontend::recordsLoaded(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f5d0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsLoaded(app, ts); });
}

nfs::u32 frontend::recordsSelect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f670", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const nfs::u32 track = args.eax, kind = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsSelect(app, ts, track, kind); });
}

nfs::u32 frontend::recordsRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fa90", 0, 0, nullptr, 0, kEsiEdi };
    const nfs::u32 first = args.eax, second = args.edx, third = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsRead(app, ts, first, second, third); });
}

nfs::u32 frontend::recordsClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44fbc0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsClear(app, ts); });
}

nfs::u32 frontend::recordsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450f70", 0, 0, kRecordsStrings, std::size(kRecordsStrings), kEcxEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsOnEnter(app, ts, menu); });
}

nfs::u32 frontend::recordsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_450fd0: nothing to undo.
    static fe::Site site{ "sub_450fd0", 0, 4, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::recordsOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450f20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsOnFrame(app, ts); });
}

nfs::u32 frontend::recordsDeleteClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450eb0", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsDeleteClicked(app, ts); });
}

nfs::u32 frontend::recordsDeleteAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_450e50", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsDeleteAnswered(app, ts, answer, closes); });
}

nfs::u32 frontend::recordsHolder(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cef00", 0, 0, nullptr, 0, kEbxEdx };
    const nfs::u32 track = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::recordsHolder(app, ts, track); });
}

nfs::u32 frontend::postgameUnlockTrack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442b60", 0, 0, nullptr, 0, kEbxEdx };
    const nfs::u32 track = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameUnlockTrack(app, ts, track); });
}

nfs::u32 frontend::postgameAwardMessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e170", 0, 0, nullptr, 0, kEbxEcxEdx };
    const nfs::u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameAwardMessage(app, ts, id); });
}

nfs::u32 frontend::postgameCheckAwards(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e1a0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameCheckAwards(app, ts); });
}

nfs::u32 frontend::postgameSaveKnockoutClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e460", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameSaveKnockoutClicked(app, ts); });
}

nfs::u32 frontend::postgameSaveTournamentClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e480", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameSaveTournamentClicked(app, ts); });
}

nfs::u32 frontend::postgameSaveGhostClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e4a0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameSaveGhostClicked(app, ts); });
}

nfs::u32 frontend::postgameSaveReplayClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e6a0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameSaveReplayClicked(app, ts); });
}

nfs::u32 frontend::postgameLeaveNetwork(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e4c0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameLeaveNetwork(app, ts); });
}

nfs::u32 frontend::postgameLeaveAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e510", 0, 0, nullptr, 0, 0 };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameLeaveAnswered(app, ts, answer, closes); });
}

nfs::u32 frontend::postgameAskLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e550", 0, 0, nullptr, 0, kEbxEcxEdx };
    const nfs::u32 lineId = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameAskLeave(app, ts, lineId); });
}

nfs::u32 frontend::postgameMainMenuClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e5b0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameMainMenuClicked(app, ts); });
}

nfs::u32 frontend::postgameNetworkMainMenuClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e600", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameNetworkMainMenuClicked(app, ts); });
}

nfs::u32 frontend::postgameQuitClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e690", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameQuitClicked(app, ts); });
}

nfs::u32 frontend::postgameCannotSaveReplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e6c0", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameCannotSaveReplay(app, ts); });
}

nfs::u32 frontend::postgameViewReplayClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e710", 0, 0, nullptr, 0, kEcxEdx | kEsiEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameViewReplayClicked(app, ts); });
}

nfs::u32 frontend::postgameToggleSheets(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44e7a0", 0, 0, nullptr, 0, kEbxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameToggleSheets(app, ts); });
}

nfs::u32 frontend::postgameOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44efd0", 0, 0, kPostgameStrings, std::size(kPostgameStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameOnEnter(app, ts, menu); });
}

nfs::u32 frontend::postgameOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44f500", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameOnExit(app, ts); });
}

nfs::u32 frontend::postgameOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_44eda0", 0, 0, kPostgameFrameStrings, std::size(kPostgameFrameStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameOnFrame(app, ts, menu); });
}

nfs::u32 frontend::postgameFinishers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cef80", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameFinishers(app, ts); });
}

nfs::u32 frontend::postgameFinishersCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf000", 0, 0, nullptr, 0, kEbxEcxEdxEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameFinishersCount(app, ts); });
}

nfs::u32 frontend::postgameReplayExcluded(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf040", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameReplayExcluded(app, ts); });
}

nfs::u32 frontend::postgameRaceAgain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf090", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::postgameRaceAgain(app, ts); });
}

nfs::u32 frontend::loadSaveKnockoutInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4461d0", 0, 0, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveKnockoutInfo(app, ts, path); });
}

nfs::u32 frontend::loadSaveTournamentInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4462e0", 0, 0, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveTournamentInfo(app, ts, path); });
}

nfs::u32 frontend::loadSaveGhostInfo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446420", 0, 0, kInfoStrings, std::size(kInfoStrings), kEbxEcxEdxEsi };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveGhostInfo(app, ts, path); });
}

nfs::u32 frontend::loadSaveLoadKnockout(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4465e0", 0, 0, nullptr, 0 };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveLoadKnockout(app, ts, path); });
}

nfs::u32 frontend::loadSaveLoadTournament(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446800", 0, 0, nullptr, 0 };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveLoadTournament(app, ts, path); });
}

nfs::u32 frontend::loadSaveLoadGhost(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4466a0", 0, 0, nullptr, 0, kEcxEdx | kEsiEdi };
    const nfs::u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveLoadGhost(app, ts, path); });
}

nfs::u32 frontend::loadSaveShowTournaments(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446ad0", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveShowTournaments(app, ts); });
}

nfs::u32 frontend::loadSaveShowKnockouts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446b10", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveShowKnockouts(app, ts); });
}

nfs::u32 frontend::loadSaveShowReplays(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446b60", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveShowReplays(app, ts); });
}

nfs::u32 frontend::loadSaveShowGhosts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446ba0", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveShowGhosts(app, ts); });
}

nfs::u32 frontend::loadSaveLoadFailed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446bf0", 0, 0, nullptr, 0, kEbxEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveLoadFailed(app, ts); });
}

nfs::u32 frontend::loadSaveLoadNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446c50", 0, 0, kKeptNameStrings, std::size(kKeptNameStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveLoadNow(app, ts); });
}

nfs::u32 frontend::loadSaveDeleteSelected(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446ea0", 0, 0, kKeptNameStrings, std::size(kKeptNameStrings) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveDeleteSelected(app, ts); });
}

nfs::u32 frontend::loadSaveDeleteAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_446ff0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const nfs::u32 answer = args.eax, closes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveDeleteAnswered(app, ts, answer, closes); });
}

nfs::u32 frontend::loadSaveDeleteClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447030", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveDeleteClicked(app, ts); });
}

nfs::u32 frontend::loadSaveDrawDescription(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4470e0", 0, 0, kDrawDescriptionStrings, std::size(kDrawDescriptionStrings) };
    const nfs::u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveDrawDescription(app, ts, item); });
}

nfs::u32 frontend::loadSaveSelectionMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4471a0", 0, 0, kKeptNameStrings, std::size(kKeptNameStrings), kEbxEcxEsiEdi };
    const nfs::u32 list = args.eax, selected = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveSelectionMoved(app, ts, list, selected); });
}

nfs::u32 frontend::loadSaveOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447470", 0, 0, kLoadSaveStrings, std::size(kLoadSaveStrings), kEbxEcxEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveOnEnter(app, ts, menu); });
}

nfs::u32 frontend::loadSaveOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_447590: nothing to undo.
    static fe::Site site{ "sub_447590", 0, 4, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::loadSaveOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447370", 0, 0, kLoadSaveFrameStrings, std::size(kLoadSaveFrameStrings),
                          kEbxEcxEdx | fe::kRestoreEdi };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::loadSaveOnFrame(app, ts, menu); });
}

nfs::u32 frontend::creditsHaveBonusCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_439ce0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsHaveBonusCar(app, ts); });
}

nfs::u32 frontend::creditsRenderPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_441cb0", 0, 0, kRenderPageStrings, std::size(kRenderPageStrings) };
    const nfs::u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsRenderPage(app, ts, previous); });
}

nfs::u32 frontend::creditsNextPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442370", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const nfs::u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsNextPage(app, ts, item); });
}

nfs::u32 frontend::creditsPreviousPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4423e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsPreviousPage(app, ts); });
}

nfs::u32 frontend::creditsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442850", 0, 0, kCreditsEnterStrings, std::size(kCreditsEnterStrings),
                          kEbxEcxEdx | fe::kRestoreEdi };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsOnEnter(app, ts, menu); });
}

nfs::u32 frontend::creditsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442980", 0, 0, nullptr, 0, kEcxEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsOnExit(app, ts); });
}

nfs::u32 frontend::creditsOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442580", 0, 0, kCreditsFrameStrings, std::size(kCreditsFrameStrings) };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::creditsOnFrame(app, ts, menu); });
}

nfs::u32 frontend::credits1ShowPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454d60", 0, 0, kGlamShowStrings, std::size(kGlamShowStrings) };
    const nfs::u32 nextButton = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::credits1ShowPage(app, ts, nextButton); });
}

nfs::u32 frontend::credits1NextClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454e40", 0, 0, nullptr, 0, kEbxEcxEdxEsi };
    const nfs::u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::credits1NextClicked(app, ts, item); });
}

nfs::u32 frontend::credits1OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454ea0", 0, 0, kGlamEnterStrings, std::size(kGlamEnterStrings), kEbxEcxEdx };
    const nfs::u32 menu = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::credits1OnEnter(app, ts, menu); });
}

nfs::u32 frontend::credits1OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_454e80", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return misc::credits1OnExit(app, ts); });
}

}
