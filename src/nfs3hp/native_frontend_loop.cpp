#include "native_frontend.h"
#include <iterator>

/* The menu loop and what it moves through: the screen shown, its items drawn
 * layer by layer, the item selected with the keys or found under the mouse,
 * a key handed to the item that has it, and the screens gone to and come back
 * from.
 *
 * The screen shown is a pointer into the table of screens (0x559230); the
 * screens left for it are a stack of 20 (0x662800), the one Back returns to
 * first.  A screen's selected item is an index from its first item, 100 for
 * none.  An item has handlers of its own, set by its widget: as the screen
 * opens (+0x24) and closes (+0x28), to draw it (+0x2c, told whether it is the
 * one selected), for a key (+0x30), once it is up (+0x34) and on every pass
 * (+0x38).  An item can be greyed (its state's bit 0), hidden (kHidden), or,
 * in a network race's lobby, closed to all but the "racenow" button.
 *
 * The same screens serve a race as its pause screens (0x559234 set, their
 * own loop at sub_44bac0): no fades then, no art of their own, and only the
 * items marked for it. */
namespace nfs3hp
{

namespace
{

using fe::Guest;

// The screens and their items.
constexpr x86::reg32 kMenus = 0x6040d0;            // the first is the one the loop goes back to
constexpr x86::reg32 kMenuCount = 0x660bb0;
constexpr x86::reg32 kPrevious = 0x1c;             // int16, the item selected before
constexpr std::int16_t kNoItem = 100;              // selected: none
// The widget registry: type, name, the item a new one starts as (12 bytes).
constexpr x86::reg32 kWidgets = 0x557a84;
constexpr x86::reg32 kWidgetSize = 12;
constexpr std::uint32_t kButton = 5;
// An item's fields beyond the .MNU's (see fe::item).
constexpr x86::reg32 kStateWord = 0x04;            // the state and flags bytes as one word
constexpr x86::reg32 kPriority = 0x0a;             // int16, its layer: drawn from the highest down
constexpr x86::reg32 kPostRaceOnly = 0x14;         // int16, "postonly"
constexpr x86::reg32 kMainOnly = 0x16;             // int16, "mainonly": not after a race
constexpr x86::reg32 kTop = 0x1a;                  // int16 each: the box the mouse finds it in
constexpr x86::reg32 kLeft = 0x1c;
constexpr x86::reg32 kWidth = 0x1e;
constexpr x86::reg32 kHeight = 0x20;
constexpr x86::reg32 kOnShow = 0x24;               // its handlers
constexpr x86::reg32 kOnHide = 0x28;
constexpr x86::reg32 kOnDraw = 0x2c;
constexpr x86::reg32 kOnKey = 0x30;
constexpr x86::reg32 kOnShown = 0x34;
constexpr x86::reg32 kOnPass = 0x38;
// The state word's bits.
constexpr std::uint16_t kUnusable = 0x0001;        // greyed out
constexpr std::uint16_t kHasKeys = 0x0400;         // the item has the keys to itself: an open list
constexpr std::uint16_t kInRaceToo = 0x0800;       // shown in the pause screens too
constexpr std::uint16_t kGreyAndHide = 0x1001;
constexpr std::uint16_t kNotSelectable = 0x1301;
constexpr std::uint16_t kNotUnderMouse = 0x1101;

// The loop's state.
constexpr x86::reg32 kCurrentMenu = 0x559230;
constexpr x86::reg32 kHistory = 0x662800;          // screens, the one Back goes to first
constexpr std::uint32_t kHistorySize = 20;
constexpr x86::reg32 kInRace = 0x559234;           // the menus are a race's pause screens
constexpr x86::reg32 kAfterRace = 0x662968;        // the menus opened on "postgame"
constexpr x86::reg32 kNetworkGame = 0x7a22dc;        // int16: set up to race others over a network
constexpr x86::reg32 kNetworkConnected = 0x7a22e0;
constexpr x86::reg32 kLobbyLocked = 0x6fd508;      // a network race's lobby: only "racenow"
constexpr x86::reg32 kMouseX = 0x55e5e4;
constexpr x86::reg32 kMouseY = 0x55e5e8;
constexpr x86::reg32 kMouseBlocked = 0x5556d0;     // the network screens have the mouse
constexpr x86::reg32 kMouseHeld = 0x559250;        // byte: the item under the mouse stays the selected one
// The depth range 2D is drawn in: one for the five lowest layers, one above.
constexpr x86::reg32 kDepthNear = 0x563aa8;
constexpr x86::reg32 kDepthFar = 0x563aac;
constexpr std::uint32_t kFloatAlmostOne = 0x3f7fff00;
constexpr std::uint32_t kFloatTiny = 0x37800080;

// The help box (sub_449aa0).
constexpr x86::reg32 kHelpMenu = 0x559248;         // the screen it was for
constexpr x86::reg32 kHelpNeedsMove = 0x559238;    // not before the pointer moves on a new screen
constexpr x86::reg32 kHelpLastX = 0x55923c;        // where the pointer was
constexpr x86::reg32 kHelpLastY = 0x559240;
constexpr x86::reg32 kHelpDue = 0x662950;          // the tick it shows at
constexpr x86::reg32 kHelpItem = 0x559244;         // the item it is shown for, -1 for none
constexpr x86::reg32 kHelpText = 0x55924c;
constexpr x86::reg32 kHelpFade = 0x662964;         // 0 or 255
constexpr x86::reg32 kHelpX = 0x662958;
constexpr x86::reg32 kHelpY = 0x66295c;
constexpr x86::reg32 kHelpWidth = 0x662960;
constexpr x86::reg32 kHelpHeight = 0x662954;
constexpr x86::reg32 kHelpAlpha = 0x559450;        // float, 0.9 with translucency
constexpr std::uint32_t kHelpFont = 0x12;
constexpr std::uint32_t kHelpWrap = 380;
constexpr std::uint32_t kHelpBoxColour = 0xff4b6cad;
constexpr std::uint32_t kHelpTextColour = 0x488cfd;
constexpr std::uint32_t kFloatOne = 0x3f800000;
constexpr std::uint32_t kFloatPoint9 = 0x3f666666;
constexpr x86::reg32 kTicks = 0x79c1a4;            // the game's clock
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kNoTranslucency = 0x02;     // no fades, an opaque help box
constexpr x86::reg32 kDrawColour = 0x563aa4;       // what 2D is drawn with, alpha on top

// The race settings sub_44be60 keeps consistent (see there), and what it
// last saw of them.
constexpr x86::reg32 kRaceType = 0x6fd3b8;
constexpr std::uint32_t kTournament = 1;
constexpr std::uint32_t kKnockout = 2;
constexpr std::uint32_t kHotPursuit = 3;
constexpr x86::reg32 kSplitScreen = 0x6fd3b0;      // players on this machine, 2 over a network
constexpr x86::reg32 kSetting4d0 = 0x6fd4d0;       // 0 to 2, goes with kSetting4d4
constexpr x86::reg32 kSetting4d4 = 0x6fd4d4;       // 0 to 4
constexpr x86::reg32 kSeen4d0 = 0x559258;          // -1 before the first pass
constexpr x86::reg32 kSeen4d4 = 0x55925c;
constexpr x86::reg32 kSeenRaceType = 0x559260;
constexpr x86::reg32 kSettingBbe4 = 0x6fbbe4;
constexpr x86::reg32 kSettingBbd8 = 0x6fbbd8;      // set when kSettingBbe4 is not
constexpr x86::reg32 kSettingBc00 = 0x6fbc00;      // 0 or 1, kSetting292c the other way
constexpr x86::reg32 kSetting292c = 0x55292c;
constexpr x86::reg32 kSetting504 = 0x6fd504;       // 0 to 3; 3 not in a two-player series
constexpr x86::reg32 kSetting639c = 0x55639c;      // word
constexpr x86::reg32 kSetting4e8 = 0x6fd4e8;
constexpr x86::reg32 kSetting4ac = 0x6fd4ac;
constexpr x86::reg32 kValue5d004 = 0x55d004;
constexpr x86::reg32 kSetting4b0 = 0x6fd4b0;
constexpr x86::reg32 kSetting4b4 = 0x6fd4b4;
constexpr x86::reg32 kSetting2ac = 0x6fd2ac;
constexpr x86::reg32 kNetworkSetting2276 = 0x7a2276;  // int16
constexpr x86::reg32 kNetworkSettingE5d0 = 0x55e5d0;
constexpr x86::reg32 kPlayer1Car = 0x6fd2bc;       // the players' cars, as sub_4396b0 reads them
constexpr x86::reg32 kPlayer2Car = 0x6fd328;

// The loop's (sub_44b230).
constexpr x86::reg32 kLastAction = 0x66296c;       // what left the last screen
constexpr x86::reg32 kScreenUp = 0x8b7630;         // a screen is being shown
constexpr x86::reg32 kFlagEb10 = 0x55eb10;
constexpr x86::reg32 kFrontendArt = 0x559228;      // mmnu.qfs (or .fsh), loaded once
constexpr x86::reg32 kScreenShapes = 0x55922c;     // a screen's own, let go as it closes
constexpr x86::reg32 kAfterMovieScreen = 0x559224; // the screen a movie goes on to, by name
constexpr x86::reg32 kNoHelpFlags = 0x55eb2e;      // byte: 0x10 or 0x20, no help box
constexpr x86::reg32 kQuitting = 0x559251;         // byte: Yes was the answer
constexpr x86::reg32 kNoQuitQuestion = 0x55e894;
constexpr x86::reg32 kDisplayChoice = 0x6fbb4c;
constexpr x86::reg32 kMusicChoice = 0x6fbbe8;
constexpr x86::reg32 kSuspended = 0x559444;        // the game's window gone to the background
constexpr x86::reg32 kResumed = 0x55fec0;
constexpr x86::reg32 kFade = 0x559c60;             // a screen's fade out, to 500
constexpr x86::reg32 kFadeIn = 0x559c68;
// The dialogs the loop opens, their lines and buttons.
constexpr x86::reg32 kDialogLines = 0x662970;
constexpr x86::reg32 kDialogButtons = 0x66297c;
constexpr std::uint32_t kQuitQuestion = 0xf4;
constexpr std::uint32_t kQuitYes = 0xc4;
constexpr std::uint32_t kQuitNo = 0xc5;
constexpr x86::reg32 kPlayer1Name = 0x6fd2fc;
constexpr x86::reg32 kPlayer2Name = 0x6fd368;
constexpr std::uint32_t kPlayer1NameTitle = 0xc9;
constexpr std::uint32_t kPlayer2NameTitle = 0xca;
constexpr std::uint32_t kPlayer1Default = 0xfa;    // "Player 1"
constexpr std::uint32_t kPlayer2Default = 0xfb;
constexpr std::uint32_t kNameDialogLine2 = 0xf8;
constexpr std::uint32_t kNameDialogOk = 0xf9;
constexpr std::uint32_t kNameDialogCancel = 0xc6;
constexpr std::uint32_t kPlayerNameLength = 10;
// What leaves a screen (its handlers' results, the loop's actions).
constexpr x86::reg32 kGoOn = 5;                    // to the selected button's nextmenu
constexpr x86::reg32 kRace = 6;                    // the menus left for the race
constexpr x86::reg32 kRace7 = 7;
constexpr x86::reg32 kQuit = 10;
constexpr x86::reg32 kOut = 11;                    // Back from the first screen

// Keys, as sub_451960 gives them: ASCII, or a scan code in the high byte.
constexpr std::uint16_t kKeyEnter = 0x0d;
constexpr std::uint16_t kKeyEscape = 0x1b;
constexpr std::uint16_t kKeyUp = 0x4800;
constexpr std::uint16_t kKeyLeft = 0x4b00;
constexpr std::uint16_t kKeyRight = 0x4d00;
constexpr std::uint16_t kKeyDown = 0x5000;

// What a key handler or a screen's handler returns.
constexpr x86::reg32 kHandled = 2;                 // taken, and nothing more: 0 to the loop
constexpr x86::reg32 kKeyEscapeButton = 0x0d;      // what the Escape button is handed

// Sounds, sub_4181d0(sound, volume).
constexpr x86::reg32 kPlaySound = 0x4181d0;
constexpr std::uint32_t kSoundMove = 0;
constexpr std::uint32_t kSoundBack = 3;
constexpr std::uint32_t kSoundEnter = 4;
constexpr std::uint32_t kSoundLeave = 5;
constexpr std::uint32_t kSoundStart = 7;
constexpr std::uint32_t kVolumeFull = 0x7f;
constexpr std::uint32_t kVolumeTick = 0x2d;

// The game's.
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;
constexpr x86::reg32 kTextBox = 0x451ff0;          // a text's size, wrapped: (text, font, wrap, &w, &h, 0)
constexpr x86::reg32 kDrawTextWrapped = 0x451fc0;
constexpr x86::reg32 kDrawTextInRace = 0x452190;
constexpr x86::reg32 kFillBox = 0x4d8550;
constexpr x86::reg32 kAllowOption = 0x43a6e0;      // (option bits, allowed)
constexpr x86::reg32 kCarFlag10 = 0x4396b0;        // a car's flag 0x10
constexpr x86::reg32 kMap4cf160 = 0x4cf160;        // 2 -> 0, 4 -> 1, else 2
constexpr x86::reg32 kMap4cf190 = 0x4cf190;        // 0 -> 2, 1 -> 4, else 8
constexpr x86::reg32 kMarkOption8 = 0x442b60;
constexpr x86::reg32 kLoadMenus = 0x449510;
constexpr x86::reg32 kCheckMenuFiles = 0x449960;
constexpr x86::reg32 kSprintf = 0x4df690;
constexpr x86::reg32 kFree = 0x4e1890;
constexpr x86::reg32 kSetDisplay = 0x4d1750;
constexpr x86::reg32 kResetInput = 0x451b20;
constexpr x86::reg32 kStartMusic = 0x4104f0;       // (track, music choice, after a race)
constexpr x86::reg32 kStopMusic = 0x4109d0;
constexpr x86::reg32 kPollEvents = 0x4c9860;
constexpr x86::reg32 kReadKey = 0x451960;          // the next key, 0 for none
constexpr x86::reg32 kBeginFrame = 0x4d9120;
constexpr x86::reg32 kPresentFrame = 0x4dab90;
constexpr x86::reg32 kEndFrame = 0x4d9140;         // and waits for the next one
constexpr x86::reg32 kItemChanged = 0x460760;
constexpr x86::reg32 kScreenLeft = 0x4534d0;       // (its name)
constexpr x86::reg32 kMenusEnd = 0x452110;
constexpr x86::reg32 kNetworkLeft = 0x440d20;
constexpr x86::reg32 kDialogPass = 0x445f50;       // a dialog's pass, its result the loop's
constexpr x86::reg32 kDialogKey = 0x445e60;
constexpr x86::reg32 kCloseDialog = 0x445f10;
constexpr x86::reg32 kNameValid = 0x4721f0;
constexpr x86::reg32 kResetDisplay = 0x4daf80;
constexpr x86::reg32 kLoadPointer = 0x49c140;
constexpr x86::reg32 kLoadShapes = 0x4956f0;       // a .qfs or .fsh file, 0 when it is not there
constexpr x86::reg32 kLoadDialogArt = 0x445de0;
constexpr x86::reg32 kClearBackground = 0x465d60;
constexpr x86::reg32 kSetBackground = 0x465d80;
constexpr x86::reg32 kLoadScreenArt = 0x4a4ba0;
constexpr x86::reg32 kFrameTime = 0x4f0650;
constexpr x86::reg32 kSleep = 0x4f0710;
constexpr x86::reg32 kRestoreDisplay = 0x9ef918;   // an import's slot
constexpr x86::reg32 kResumeSound = 0x4bef50;
constexpr x86::reg32 kArtFolder = 0x7a2db4;
constexpr x86::reg32 kBoxBorder = 0x4d8930;
constexpr x86::reg32 kStrcmp = 0x4ee310;
constexpr x86::reg32 kSetMousePosition = 0x49bfc0;
constexpr x86::reg32 kNetworkFrame = 0x441040;     // the network screens' pass; nonzero leaves
constexpr x86::reg32 kNetworkBusy = 0x441680;
constexpr x86::reg32 kNetworkKey = 0x441700;
constexpr x86::reg32 kNetworkShown = 0x440970;
constexpr x86::reg32 kHotKey = 0x4524e0;           // the keys that work on any screen
constexpr x86::reg32 kSetCursor = 0x49c0e0;

// This file's own, called where they are so each runs as itself.
constexpr x86::reg32 kItemUsable = 0x4499b0;
constexpr x86::reg32 kKeyAllowed = 0x4499f0;
constexpr x86::reg32 kSelectable = 0x449e50;
constexpr x86::reg32 kPointAtSelected = 0x449ea0;
constexpr x86::reg32 kSelectPrevious = 0x449f30;
constexpr x86::reg32 kSelectNext = 0x44a0c0;
constexpr x86::reg32 kEscapeButton = 0x44a240;
constexpr x86::reg32 kSelectedIndex = 0x44a440;
constexpr x86::reg32 kReleaseMouse = 0x44a690;
constexpr x86::reg32 kMenuByName = 0x44ae60;
constexpr x86::reg32 kPopMenu = 0x44aec0;
constexpr x86::reg32 kDialogShown = 0x4438b0;
constexpr x86::reg32 kCurrentMenuAddress = 0x44aeb0;
constexpr x86::reg32 kCurrentMenuName = 0x44ae00;
constexpr x86::reg32 kDrawItems = 0x44a4f0;
constexpr x86::reg32 kItemUnderMouse = 0x44a6b0;
constexpr x86::reg32 kItemsShown = 0x44a860;
constexpr x86::reg32 kPassItems = 0x44a8b0;
constexpr x86::reg32 kShowItems = 0x44a960;
constexpr x86::reg32 kHideItems = 0x44a9e0;
constexpr x86::reg32 kLeaveScreen = 0x44aa30;
constexpr x86::reg32 kResetItems = 0x44aad0;
constexpr x86::reg32 kEnterScreen = 0x44ab40;
constexpr x86::reg32 kPushMenu = 0x44af10;
constexpr x86::reg32 kSetHistory = 0x44af50;
constexpr x86::reg32 kHandleKey = 0x44a2d0;
constexpr x86::reg32 kWaitWhileSuspended = 0x44b1a0;
constexpr x86::reg32 kFitRaceSettings = 0x44be60;
constexpr x86::reg32 kDrawHelp = 0x449aa0;
constexpr x86::reg32 kQuitAnswer = 0x44b0f0;
constexpr x86::reg32 kPlayer1NameAnswer = 0x44af90;
constexpr x86::reg32 kPlayer2NameAnswer = 0x44b040;
constexpr x86::reg32 kKeysTaken = 0x449df0;

const fe::GuestString kRaceNow{ 0x538fb4, "racenow" };
const fe::GuestString kEscape{ 0x538fbc, "escape" };
const fe::GuestString kIpx{ 0x538fc4, "ipx" };
const fe::GuestString kPostGame{ 0x539048, "postgame" };
const fe::GuestString kMmnu{ 0x538fc8, "mmnu" };
const fe::GuestString kQfsFormat{ 0x538fd0, "%s%s.qfs" };
const fe::GuestString kFshFormat{ 0x538fdc, "%s%s.fsh" };
const fe::GuestString kOpponent{ 0x538fe8, "opponent" };
const fe::GuestString kCar1{ 0x538ff4, "car1" };
const fe::GuestString kCar2{ 0x538ffc, "car2" };
const fe::GuestString kShowcaseScreen{ 0x539084, "showcase" };
const fe::GuestString kInteriorScreen{ 0x539090, "interior" };
const fe::GuestString kHistoryScreen{ 0x53909c, "history" };
const fe::GuestString kCompareScreen{ 0x5390a4, "compare" };

x86::reg32 menuAt(std::uint32_t index)
{
    return kMenus + index * fe::menu::kSize;
}

x86::reg32 itemAt(std::int32_t index)
{
    return fe::item::kTable + x86::reg32(index) * fe::item::kSize;
}

std::int16_t signedWord(const Guest& guest, x86::reg32 address)
{
    return std::int16_t(guest.word(address));
}

std::int32_t firstItem(const Guest& guest, x86::reg32 menu)
{
    return signedWord(guest, menu + fe::menu::kFirstItem);
}

std::int16_t selected(const Guest& guest, x86::reg32 menu)
{
    return signedWord(guest, menu + fe::menu::kSelected);
}

void setSelected(Guest& guest, x86::reg32 menu, std::int32_t index)
{
    guest.setWord(menu + fe::menu::kSelected, std::uint16_t(index));
}

// The item `offset` from the screen's first.
x86::reg32 menuItem(const Guest& guest, x86::reg32 menu, std::int32_t offset)
{
    return itemAt(firstItem(guest, menu) + offset);
}

bool isEnd(const Guest& guest, x86::reg32 item)
{
    return guest.dword(item + fe::item::kType) == 0;
}

void playSound(Guest& guest, std::uint32_t sound, std::uint32_t volume)
{
    guest.call(kPlaySound, fe::args({ sound, volume }));
}

/* An item's state word as selection reads it: greyed out too when it is not
 * usable now (sub_4499b0). */
std::uint16_t selectionState(Guest& guest, x86::reg32 item)
{
    std::uint16_t state = guest.word(item + kStateWord);
    if (guest.call(kItemUsable, fe::args({ item })) == 0)
        state |= kUnusable;
    return state;
}

/* sub_4499b0: whether an item can be used: in a network race's locked lobby,
 * only the one whose codelink names "racenow". */
x86::reg32 itemUsable(Guest& guest, x86::reg32 item)
{
    if (guest.dword(kNetworkConnected) == 0 || guest.dword(kLobbyLocked) == 0 || guest.dword(kInRace) != 0)
        return 1;
    return guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), kRaceNow.address })) != 0
               ? 1
               : 0;
}

/* sub_4499f0: whether a key may act on a screen: always, but in a locked
 * lobby while the network is idle only on a usable item, and never Escape. */
x86::reg32 keyAllowed(Guest& guest, x86::reg32 menu, std::uint16_t key)
{
    if (guest.dword(kNetworkConnected) == 0 || guest.dword(kLobbyLocked) == 0 || guest.dword(kInRace) != 0)
        return 1;
    const x86::reg32 busy = guest.call(kNetworkBusy);
    if (busy != 0)
        return 1;
    if (selected(guest, menu) == kNoItem || key == kKeyEscape)
        return busy;
    return guest.call(kItemUsable, fe::args({ menuItem(guest, menu, selected(guest, menu)) })) != 0 ? 1 : 0;
}

/* sub_449e50: whether the item `offset` of a screen can be selected. */
x86::reg32 selectable(Guest& guest, std::int32_t offset, x86::reg32 menu)
{
    if (offset < 0 || offset == kNoItem)
        return 0;
    return (selectionState(guest, menuItem(guest, menu, offset)) & kNotSelectable) == 0 ? 1 : 0;
}

/* sub_449ea0: the mouse pointer moved onto the selected item, three quarters
 * along it and halfway down. */
x86::reg32 pointAtSelected(Guest& guest, x86::reg32 menu)
{
    if (selected(guest, menu) == kNoItem)
        return guest.eax();
    const x86::reg32 item = menuItem(guest, menu, selected(guest, menu));
    const std::int32_t x = signedWord(guest, item + kLeft) + 3 * signedWord(guest, item + kWidth) / 4;
    guest.setDword(kMouseX, x86::reg32(x));
    const std::int32_t y = signedWord(guest, item + kTop) + signedWord(guest, item + kHeight) / 2;
    guest.setDword(kMouseY, x86::reg32(y));
    return guest.call(kSetMousePosition, fe::args({ x86::reg32(x), x86::reg32(y) }));
}

/* What selecting another item ends with: the one left kept as the previous
 * when it could be selected, a tick when the selection moved -- in that order
 * going forward, the other way round going back -- and the pointer on the
 * new one. */
x86::reg32 afterSelecting(Guest& guest, x86::reg32 menu, std::int16_t was, bool forward)
{
    auto keepPrevious = [&]() {
        if (guest.call(kSelectable, fe::args({ x86::reg32(std::int32_t(was)), menu })) != 0)
            guest.setWord(menu + kPrevious, std::uint16_t(was));
    };
    if (forward)
        keepPrevious();
    if (selected(guest, menu) != was)
        playSound(guest, kSoundMove, kVolumeFull);
    if (!forward)
        keepPrevious();
    return guest.call(kPointAtSelected, fe::args({ menu }));
}

/* The previous selection back when the one selected cannot be: whether it was. */
bool backToPrevious(Guest& guest, x86::reg32 menu, std::int16_t was)
{
    if (was != kNoItem && guest.call(kSelectable, fe::args({ x86::reg32(std::int32_t(was)), menu })) != 0)
        return false;
    const std::int16_t previous = signedWord(guest, menu + kPrevious);
    if (guest.call(kSelectable, fe::args({ x86::reg32(std::int32_t(previous)), menu })) == 0)
        return false;
    guest.setWord(menu + fe::menu::kSelected, guest.word(menu + kPrevious));
    return true;
}

/* sub_449f30: the item before the selected one that can be selected, round
 * from the first to the last; with none selected, the last of them.  Not
 * while the selected item has the keys. */
x86::reg32 selectPrevious(Guest& guest, x86::reg32 menu)
{
    const std::int16_t was = selected(guest, menu);
    if (was != kNoItem && (guest.word(menuItem(guest, menu, was) + kStateWord) & kHasKeys) != 0)
        return guest.eax();
    if (!backToPrevious(guest, menu, was))
    {
        if (selected(guest, menu) == kNoItem)
        {
            for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
            {
                if ((selectionState(guest, itemAt(index)) & kNotSelectable) == 0)
                    setSelected(guest, menu, index - firstItem(guest, menu));
            }
        }
        else
        {
            do
            {
                if (selected(guest, menu) != 0)
                    setSelected(guest, menu, selected(guest, menu) - 1);
                else
                    while (!isEnd(guest, menuItem(guest, menu, selected(guest, menu) + 1)))
                        setSelected(guest, menu, selected(guest, menu) + 1);
            } while ((selectionState(guest, menuItem(guest, menu, selected(guest, menu))) & kNotSelectable) != 0);
        }
    }
    return afterSelecting(guest, menu, was, false);
}

/* sub_44a0c0: the item after the selected one that can be selected, round
 * from the last to the first; with none selected, the first of them. */
x86::reg32 selectNext(Guest& guest, x86::reg32 menu)
{
    const std::int16_t was = selected(guest, menu);
    if (was != kNoItem && (guest.word(menuItem(guest, menu, was) + kStateWord) & kHasKeys) != 0)
        return guest.eax();
    if (!backToPrevious(guest, menu, was))
    {
        if (selected(guest, menu) == kNoItem)
        {
            for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
            {
                if ((selectionState(guest, itemAt(index)) & kNotSelectable) == 0)
                {
                    setSelected(guest, menu, index - firstItem(guest, menu));
                    break;
                }
            }
        }
        else
        {
            do
            {
                setSelected(guest, menu, selected(guest, menu) + 1);
                if (isEnd(guest, menuItem(guest, menu, selected(guest, menu))))
                    setSelected(guest, menu, 0);
            } while ((selectionState(guest, menuItem(guest, menu, selected(guest, menu))) & kNotSelectable) != 0);
        }
    }
    return afterSelecting(guest, menu, was, true);
}


/* sub_44a240: a screen's Escape button: the button whose codelink names
 * "escape", the first of them, if it is not greyed out; 0 for none.  Its
 * index from the first item in *offset (0 for none). */
x86::reg32 escapeButton(Guest& guest, x86::reg32 menu, x86::reg32 offset)
{
    x86::reg32 found = 0;
    guest.setDword(offset, 0);
    if (menu == 0)
        return 0;
    for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        const x86::reg32 codelink = guest.dword(item + fe::item::kCodelink);
        if (guest.dword(item + fe::item::kType) == kButton && codelink != 0 && found == 0
            && guest.call(kCodelinkMatches, fe::args({ codelink, kEscape.address })) != 0
            && (guest.byte(item + fe::item::kState) & fe::item::kDisabled) == 0)
        {
            found = item;
            guest.setDword(offset, x86::reg32(index - firstItem(guest, menu)));
        }
    }
    return found;
}

/* sub_44a440: the selected item of the screen shown, by its index in the
 * item table; -1 for none. */
x86::reg32 selectedIndex(Guest& guest)
{
    const x86::reg32 menu = guest.dword(kCurrentMenu);
    if (selected(guest, menu) == kNoItem)
        return x86::reg32(-1);
    return x86::reg32(firstItem(guest, menu) + selected(guest, menu));
}

/* sub_44a470: `item` selected on the screen shown, if it is one of its
 * items: whether it is.  The selection left is kept as the previous when it
 * could be selected. */
x86::reg32 selectItem(Guest& guest, x86::reg32 item)
{
    const x86::reg32 menu = guest.dword(kCurrentMenu);
    for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
    {
        if (itemAt(index) != item)
            continue;
        const x86::reg32 current = guest.dword(kCurrentMenu);
        if (guest.call(kSelectable, fe::args({ x86::reg32(std::int32_t(selected(guest, current))), current })) != 0)
            guest.setWord(current + kPrevious, guest.word(current + fe::menu::kSelected));
        const x86::reg32 shown = guest.dword(kCurrentMenu);
        setSelected(guest, shown, index - firstItem(guest, shown));
        return 1;
    }
    return 0;
}

/* sub_44a660: whether an item is shown: in the pause screens, only one marked
 * for them. */
x86::reg32 shownNow(Guest& guest, x86::reg32 item)
{
    if (guest.dword(kInRace) != 0 && (guest.word(item + kStateWord) & kInRaceToo) == 0)
        return 0;
    return 1;
}

/* sub_44a690: the mouse let go of the item it held.  The flag is cleared
 * through ah, so eax's second byte goes with it. */
x86::reg32 releaseMouse(Guest& guest)
{
    guest.setByte(kMouseHeld, 0);
    return guest.eax() & ~x86::reg32(0xff00);
}

/* sub_44a6b0: the item under the mouse, by its index from the screen's first,
 * -1 for none: the lowest layer first, the first item of it the pointer is
 * inside of.  While a dialog is up, the mouse holds an item, or the selected
 * item has the keys, the selected one; none while the network screens have
 * the mouse.  Moving onto another item ticks. */
x86::reg32 itemUnderMouse(Guest& guest, x86::reg32 menu)
{
    constexpr std::int32_t kLastLayer = 9999;
    if (guest.dword(kMouseBlocked) != 0)
        return x86::reg32(-1);
    if ((guest.call(kDialogShown) & 0xff) != 0 || guest.byte(kMouseHeld) != 0
        || (selected(guest, menu) != kNoItem
            && (guest.word(menuItem(guest, menu, selected(guest, menu)) + kStateWord) & kHasKeys) != 0))
        return x86::reg32(std::int32_t(selected(guest, menu)));
    for (std::int32_t layer = 0; layer < kLastLayer;)
    {
        std::int32_t next = kLastLayer;
        for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
        {
            const x86::reg32 item = itemAt(index);
            const std::uint16_t state = selectionState(guest, item);
            if ((state & kNotUnderMouse) != 0)
                continue;
            const std::int32_t priority = signedWord(guest, item + kPriority);
            if (priority != layer)
            {
                if (priority < next && priority > layer)
                    next = priority;
                continue;
            }
            const std::int32_t x = std::int32_t(guest.dword(kMouseX)), y = std::int32_t(guest.dword(kMouseY));
            const std::int32_t top = signedWord(guest, item + kTop), left = signedWord(guest, item + kLeft);
            if (y <= top || y >= top + signedWord(guest, item + kHeight) || x <= left
                || left + signedWord(guest, item + kWidth) <= x)
                continue;
            const std::int32_t offset = index - firstItem(guest, menu);
            if (selected(guest, menu) != offset && (state & kGreyAndHide) == 0)
                playSound(guest, kSoundMove, kVolumeTick);
            return x86::reg32(offset);
        }
        layer = next;
    }
    return x86::reg32(-1);
}

/* sub_44a860: every item's kOnShown handler, the screen being up; then the
 * mouse lets go. */
x86::reg32 itemsShown(Guest& guest, x86::reg32 menu)
{
    for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + kOnShown) != 0)
            guest.call(guest.dword(item + kOnShown), fe::args({ item, item }));
    }
    return guest.call(kReleaseMouse);
}

/* sub_44a8b0: a pass of the items: each one's kOnPass handler -- 3 or 4
 * from one ends it -- then the selected one's again; the result the pointer's
 * shape (sub_49c0e0), 12 when a key would not be taken now. */
x86::reg32 passItems(Guest& guest, x86::reg32 menu)
{
    x86::reg32 result = 0;
    for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + kOnPass) == 0)
            continue;
        result = guest.call(guest.dword(item + kOnPass), fe::args({ item, x86::reg32(index) }));
        if (result == fe::kMenuBackToFirst || result == 3)
            return guest.call(kSetCursor, fe::args({ result }));
        result = 0;
    }
    if (selected(guest, menu) != kNoItem)
    {
        const x86::reg32 item = menuItem(guest, menu, selected(guest, menu));
        if (guest.dword(item + kOnPass) != 0)
            result = guest.call(guest.dword(item + kOnPass), fe::args({ item, item }));
    }
    if (guest.call(kKeyAllowed, fe::args({ menu, 0 })) == 0)
        result = 12;
    return guest.call(kSetCursor, fe::args({ result }));
}

/* sub_44a960: every item's kOnShow handler, the screen opening; an item for
 * after a race only, or for before one only, greyed out and hidden when it
 * is not the time for it.  The network screens get theirs. */
x86::reg32 showItems(Guest& guest, x86::reg32 menu)
{
    for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + kOnShow) == 0)
            continue;
        guest.call(guest.dword(item + kOnShow), fe::args({ item, item }));
        if (guest.word(item + kPostRaceOnly) != 0 && guest.dword(kAfterRace) == 0)
            guest.setWord(item + kStateWord, guest.word(item + kStateWord) | kGreyAndHide);
        if (guest.word(item + kMainOnly) != 0 && guest.dword(kAfterRace) != 0)
            guest.setWord(item + kStateWord, guest.word(item + kStateWord) | kGreyAndHide);
    }
    if (guest.dword(kNetworkConnected) != 0)
        guest.call(kNetworkShown);
    return guest.eax();
}

/* sub_44a9e0: every item's kOnHide handler, the screen closing. */
x86::reg32 hideItems(Guest& guest, x86::reg32 menu)
{
    for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        if (guest.dword(item + kOnHide) != 0)
            guest.call(guest.dword(item + kOnHide), fe::args({ item, item }));
    }
    return guest.eax();
}

/* sub_44aad0: every item's state and flags back to its widget's template's,
 * the screen opening. */
x86::reg32 resetItems(Guest& guest, x86::reg32 menu)
{
    for (std::int32_t index = firstItem(guest, menu); !isEnd(guest, itemAt(index)); ++index)
    {
        const x86::reg32 item = itemAt(index);
        for (x86::reg32 widget = kWidgets; guest.dword(widget) != 0; widget += kWidgetSize)
        {
            if (guest.dword(widget) != guest.dword(item + fe::item::kType))
                continue;
            const x86::reg32 from = guest.dword(widget + 8);
            if (from != 0)
                guest.setWord(item + kStateWord, guest.word(from + kStateWord));
            break;
        }
    }
    return guest.eax();
}

/* sub_44ae00: the name of the screen shown, 0 for none. */
x86::reg32 currentMenuName(Guest& guest)
{
    const x86::reg32 menu = guest.dword(kCurrentMenu);
    return menu != 0 ? guest.dword(menu + fe::menu::kName) : 0;
}

/* sub_44aeb0: the screen shown. */
x86::reg32 currentMenu(Guest& guest)
{
    return guest.dword(kCurrentMenu);
}

/* sub_44ae60: a screen by its name, 0 for none loaded. */
x86::reg32 menuByName(Guest& guest, x86::reg32 name)
{
    for (std::uint32_t index = 0; index < guest.dword(kMenuCount); ++index)
    {
        if (guest.call(kStrcmp, fe::args({ name, guest.dword(menuAt(index) + fe::menu::kName) })) == 0)
            return menuAt(index);
    }
    return 0;
}

/* sub_44aec0: back to the screen gone from last: off the history (its last
 * place kept as it was), the first screen when there is none. */
x86::reg32 popMenu(Guest& guest)
{
    const x86::reg32 back = guest.dword(kHistory);
    for (std::uint32_t i = 0; i + 1 < kHistorySize; ++i)
        guest.setDword(kHistory + 4 * i, guest.dword(kHistory + 4 * (i + 1)));
    guest.setDword(kCurrentMenu, back != 0 ? back : kMenus);
    return kHistorySize - 1;
}

/* sub_44af10: the screen shown onto the history, before going to another. */
x86::reg32 pushMenu(Guest& guest)
{
    for (std::uint32_t i = kHistorySize - 1; i > 0; --i)
        guest.setDword(kHistory + 4 * i, guest.dword(kHistory + 4 * (i - 1)));
    const x86::reg32 menu = guest.dword(kCurrentMenu);
    guest.setDword(kHistory, menu);
    return menu;
}

/* sub_44af50: the history set from a list of screen names, ended by a null
 * (the screens to come back through after a race), then the first of them
 * shown. */
x86::reg32 setHistory(Guest& guest, x86::reg32 names)
{
    for (std::uint32_t i = 0; guest.dword(names + 4 * i) != 0; ++i)
    {
        const x86::reg32 menu = guest.call(kMenuByName, fe::args({ guest.dword(names + 4 * i) }));
        if (menu != 0)
            guest.setDword(kHistory + 4 * i, menu);
    }
    return guest.call(kPopMenu);
}

/* sub_449df0: whether the keys go to something other than the screen: an
 * open list (an item of the list widgets that has the keys) or a dialog. */
x86::reg32 keysTaken(Guest& guest)
{
    const x86::reg32 index = guest.call(kSelectedIndex);
    if (index != x86::reg32(-1))
    {
        const x86::reg32 item = itemAt(std::int32_t(index));
        if ((guest.word(item + kStateWord) & kHasKeys) != 0)
        {
            const std::uint32_t type = guest.dword(item + fe::item::kType);
            if (type == 0x11 || type == 0x1d || type == 0x12 || type == 0x13)
                return 1;
        }
    }
    return (guest.call(kDialogShown) & 0xff) != 0 ? 1 : 0;
}


/* sub_44a2d0: a key on a screen.  In a network race the network screens see
 * it first; then the keys that work anywhere; then the selected item's key
 * handler.  Whatever none of them took: Escape presses the Escape button
 * (1, the screen left, when there is none), the arrows move the selection.
 * The result is the loop's (kHandled reads as 0). */
x86::reg32 handleKey(Guest& guest, x86::reg32 menu, x86::reg32 key)
{
    const std::uint16_t code = std::uint16_t(key);
    const x86::reg32 signedCode = x86::reg32(std::int32_t(std::int16_t(code)));
    x86::reg32 result = 0;
    if (guest.dword(kNetworkConnected) != 0)
    {
        result = guest.call(kNetworkKey, fe::args({ signedCode, signedCode }));
        if (result != 0 || guest.call(kKeyAllowed, fe::args({ menu })) == 0)
            return result == kHandled ? 0 : result;
    }
    if (guest.dword(kAfterRace) == 0)
        guest.call(kHotKey, fe::args({ signedCode }));
    if (selected(guest, menu) == kNoItem)
    {
        if (code == kKeyEnter)
            return 0;
    }
    else
    {
        const x86::reg32 item = menuItem(guest, menu, selected(guest, menu));
        if (guest.dword(item + kOnKey) != 0)
            result = guest.call(guest.dword(item + kOnKey), fe::args({ item, menu, signedCode }));
    }
    if (result == 0)
    {
        switch (code)
        {
        case kKeyEscape:
        {
            playSound(guest, kSoundBack, kVolumeFull);
            const x86::reg32 offset = guest.frame();  // [ebp - 4]
            guest.setDword(offset, result);
            const x86::reg32 button = guest.call(kEscapeButton, fe::args({ menu, offset }));
            if (button != 0)
            {
                guest.setWord(menu + fe::menu::kSelected, std::uint16_t(guest.dword(offset)));
                guest.setWord(menu + kPrevious, std::uint16_t(guest.dword(offset)));
                result = guest.call(guest.dword(button + kOnKey), fe::args({ button, menu, kKeyEscapeButton }));
            }
            else
                result = 1;
            break;
        }
        case kKeyUp:
        case kKeyLeft:
            playSound(guest, kSoundMove, kVolumeTick);
            guest.call(kSelectPrevious, fe::args({ menu }));
            break;
        case kKeyRight:
        case kKeyDown:
            playSound(guest, kSoundMove, kVolumeTick);
            guest.call(kSelectNext, fe::args({ menu }));
            break;
        default:
            break;
        }
    }
    return result == kHandled ? 0 : result;
}

/* sub_44a4f0: the screen's items drawn, layer by layer from the highest
 * priority down to 0, each told whether it is the selected one; hidden ones
 * not.  Layers 0 to 4 go in front.  Over a network race, the network screens
 * get each layer's pass (the lobby only its eighth); what they or an item's
 * drawing return, the last nonzero, is the result. */
x86::reg32 drawItems(Guest& guest, x86::reg32 menu)
{
    std::int32_t layer = 0;
    for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
    {
        const std::int32_t priority = signedWord(guest, itemAt(index) + kPriority);
        if (priority > layer)
            layer = priority;
    }
    x86::reg32 result = 0;
    for (; layer >= 0; --layer)
    {
        if (layer <= 4)
        {
            guest.setDword(kDepthFar, kFloatAlmostOne);
            guest.setDword(kDepthNear, 0);
        }
        else
        {
            guest.setDword(kDepthFar, kFloatTiny);
            guest.setDword(kDepthNear, kFloatAlmostOne);
        }
        for (std::int16_t index = std::int16_t(firstItem(guest, menu)); !isEnd(guest, itemAt(index)); ++index)
        {
            const x86::reg32 item = itemAt(index);
            if (signedWord(guest, item + kPriority) != layer)
                continue;
            const x86::reg32 isSelected = index == firstItem(guest, menu) + selected(guest, menu) ? 1 : 0;
            if (guest.dword(item + kOnDraw) != 0 && (guest.byte(item + fe::item::kFlags) & fe::item::kHidden) == 0)
            {
                const x86::reg32 drawn = guest.call(guest.dword(item + kOnDraw), fe::args({ item, isSelected }));
                if (drawn != 0)
                    result = drawn;
            }
        }
        if (guest.dword(kInRace) != 0)
        {
            if (guest.dword(kNetworkConnected) != 0)
            {
                const x86::reg32 network = guest.call(kNetworkFrame);
                if (network != 0)
                    result = network;
            }
        }
        else if (layer == 8 && guest.dword(kNetworkConnected) != 0
                 && guest.call(kStrcmp, fe::args({ guest.dword(menu + fe::menu::kName), kIpx.address })) != 0)
        {
            const x86::reg32 network = guest.call(kNetworkFrame);
            if (network != 0)
                result = network;
        }
    }
    guest.setDword(kDepthFar, kFloatTiny);
    guest.setDword(kDepthNear, kFloatAlmostOne);
    return result;
}


/* sub_44aa30: a screen left: its onExit handler (told how, `action`), then,
 * unless a movie played (9) or the pause screens are up, the leaving sound
 * and the items faded out -- drawn again as the fade (0x559c60) runs to 500,
 * twice as fast without fades -- then its items' kOnHide. */
x86::reg32 leaveScreen(Guest& guest, x86::reg32 menu, x86::reg32 action)
{
    const x86::reg32 step = (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? 50 : 25;
    if (guest.dword(menu + fe::menu::kOnExit) != 0)
        guest.call(guest.dword(menu + fe::menu::kOnExit), fe::args({ menu, action, step, menu }));
    if (guest.dword(kInRace) == 0)
    {
        if (action != fe::kMenuMoviePlayed)
        {
            playSound(guest, kSoundLeave, kVolumeFull);
            if ((guest.byte(kDisplayFlags) & kNoTranslucency) == 0)
            {
                for (guest.setDword(kFade, step); std::int32_t(guest.dword(kFade)) < 500;
                     guest.setDword(kFade, guest.dword(kFade) + step))
                {
                    guest.call(kBeginFrame);
                    guest.call(kDrawItems, fe::args({ menu }));
                    guest.call(kEndFrame);
                }
            }
            guest.setDword(kFade, 0);
        }
        guest.setDword(kFadeIn, 0);
    }
    return guest.call(kHideItems, fe::args({ menu }));
}

/* sub_44ab40: a screen opened.  Outside a race: the pointer reset, the
 * frontend's art loaded once (mmnu.qfs, else mmnu.fsh, from the art folder),
 * the car screens' backgrounds.  Then no item selected, the items' states
 * from their widgets', the onEnter handler -- its result is this one's --
 * and the items' kOnShow and kOnShown. */
x86::reg32 enterScreen(Guest& guest, x86::reg32 menu)
{
    if (guest.dword(kInRace) == 0)
    {
        guest.call(kSetCursor, fe::args({ 0 }));
        // eax still holds the art's name here, and edx 0, as the original has them.
        guest.call(kResetDisplay, fe::args({ kMmnu.address }));
        guest.call(kLoadPointer);
        if (guest.dword(kFrontendArt) == 0)
        {
            const x86::reg32 path = guest.frame();  // char[0x100]
            guest.call(kSprintf, fe::args({}, { path, kQfsFormat.address, kArtFolder, kMmnu.address }));
            guest.setDword(kFrontendArt, guest.call(kLoadShapes, fe::args({ path, 0 })));
            if (guest.dword(kFrontendArt) == 0)
            {
                guest.call(kSprintf, fe::args({}, { path, kFshFormat.address, kArtFolder, kMmnu.address }));
                guest.setDword(kFrontendArt, guest.call(kLoadShapes, fe::args({ path, 0 })));
            }
        }
        guest.call(kLoadDialogArt);
        guest.call(kClearBackground);
        if (guest.call(kStrcmp, fe::args({ guest.dword(menu + fe::menu::kName), kOpponent.address })) == 0
            || guest.call(kStrcmp, fe::args({ guest.dword(menu + fe::menu::kName), kCar1.address })) == 0
            || guest.call(kStrcmp, fe::args({ guest.dword(menu + fe::menu::kName), kCar2.address })) == 0)
            guest.call(kSetBackground, fe::args({ guest.dword(kFrontendArt) }));
        guest.call(kLoadScreenArt);
    }
    guest.setWord(menu + fe::menu::kSelected, std::uint16_t(kNoItem));
    guest.setWord(menu + kPrevious, guest.word(menu + fe::menu::kSelected));
    guest.setDword(kScreenShapes, 0);
    guest.call(kResetItems, fe::args({ menu }));
    x86::reg32 result = 0;
    if (guest.dword(menu + fe::menu::kOnEnter) != 0)
        result = guest.call(guest.dword(menu + fe::menu::kOnEnter), fe::args({ menu, 0 }));
    guest.call(kShowItems, fe::args({ menu }));
    guest.call(kItemsShown, fe::args({ menu }));
    guest.call(kFrameTime);
    return result;
}

/* sub_44b1a0: the game suspended (0x559444, its window gone to the
 * background): waited out, then the screen closed as on leaving it, *action
 * 13 (back to the first screen).  Whether it was. */
x86::reg32 waitWhileSuspended(Guest& guest, x86::reg32 menu, x86::reg32 action)
{
    if (guest.dword(kSuspended) == 0)
        return 0;
    guest.setDword(action, 13);
    do
    {
        guest.call(kSleep, fe::args({ 0 }));
        guest.call(kPollEvents);
    } while (guest.dword(kSuspended) != 0);
    guest.call(guest.dword(kRestoreDisplay));
    guest.setDword(kResumed, 0);
    guest.call(kResumeSound, fe::args({ 1 }));
    if (guest.dword(menu + fe::menu::kOnExit) != 0)
        guest.call(guest.dword(menu + fe::menu::kOnExit), fe::args({ menu, guest.edx(), menu }));
    guest.call(kHideItems, fe::args({ menu }));
    return 1;
}

/* sub_44af90, sub_44b040: the answers of the name dialogs the loop opens
 * when a race is started with no name for player 1 (or, split screen, player
 * 2): Enter (or OK) keeps what was typed -- "Player 1"/"Player 2" if nothing
 * -- and starts the race (5); Cancel or Escape clears it (2). */
x86::reg32 nameAnswer(Guest& guest, x86::reg32 answer, x86::reg32 close, x86::reg32 name, std::uint32_t defaultText)
{
    guest.setByte(close, 0);
    x86::reg32 result = 0;
    if (answer == fe::kDialogEnter)
    {
        const x86::reg32 focus = fe::dialogFocus(guest);
        if (focus == 0 || guest.dword(fe::kInputFromMouse) == 0)
        {
            x86::reg32 typed = fe::dialogEditText(guest);
            if (guest.call(kNameValid) == 0)
                typed = fe::text(guest, defaultText);
            guest.copyString(name, typed);
            result = kGoOn;
            guest.setByte(close, 1);
        }
        else if (focus == 1)
        {
            guest.setByte(name, 0);
            result = kHandled;
            guest.setByte(close, 1);
        }
    }
    else if (answer == fe::kDialogEscape)
    {
        guest.setByte(name, 0);
        result = kHandled;
        guest.setByte(close, 1);
    }
    return result;
}

/* sub_44b0f0: the answer to "Quit the game?": Yes ends the menus (10). */
x86::reg32 quitAnswer(Guest& guest, x86::reg32 answer, x86::reg32 close)
{
    if (answer < fe::kDialogEnter)
        return 0;
    if (answer == fe::kDialogEnter)
    {
        const x86::reg32 focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            if (close != 0)
                guest.setByte(close, 1);
            guest.setByte(kQuitting, 1);
            return 10;
        }
        if (focus == 1)
        {
            if (close != 0)
                guest.setByte(close, 1);
            guest.setByte(kQuitting, 0);
            return kHandled;
        }
        if (close != 0)
            guest.setByte(close, 0);
        return kHandled;
    }
    if (answer == fe::kDialogEscape)
    {
        if (close != 0)
            guest.setByte(close, 1);
        return kHandled;
    }
    return 0;
}


/* sub_449aa0: the help box: an item's help text, shown once the pointer has
 * rested on the selected item for 100 ticks, beside the pointer (kept on the
 * 640x480 screen), wrapped at 380 pixels, and gone when the selection moves,
 * the screen changes or a dialog comes up.  Not while the keys are taken. */
x86::reg32 drawHelp(Guest& guest)
{
    guest.setDword(kHelpAlpha, (guest.byte(kDisplayFlags) & kNoTranslucency) != 0 ? kFloatOne : kFloatPoint9);
    if (guest.call(kCurrentMenuAddress) != guest.dword(kHelpMenu) || (guest.call(kDialogShown) & 0xff) != 0)
    {
        guest.setDword(kHelpNeedsMove, 1);
        guest.setDword(kHelpMenu, guest.call(kCurrentMenuAddress));
        guest.setDword(kHelpText, 0);
        guest.setDword(kHelpLastX, guest.dword(kMouseX));
        guest.setDword(kHelpItem, x86::reg32(-1));
        guest.setDword(kHelpLastY, guest.dword(kMouseY));
    }
    if (guest.call(kKeysTaken) != 0)
        return guest.eax();

    const x86::reg32 index = guest.call(kSelectedIndex);
    bool timing = true;
    if ((guest.call(kDialogShown) & 0xff) != 0 || index == x86::reg32(-1) || index != guest.dword(kHelpItem))
        guest.setDword(kHelpItem, x86::reg32(-1));
    else if (guest.dword(kHelpItem) != x86::reg32(-1))
        timing = false;
    if (timing)
    {
        if (guest.dword(kHelpLastX) != guest.dword(kMouseX) || guest.dword(kHelpLastY) != guest.dword(kMouseY))
        {
            guest.setDword(kHelpLastX, guest.dword(kMouseX));
            guest.setDword(kHelpNeedsMove, 0);
            guest.setDword(kHelpLastY, guest.dword(kMouseY));
            guest.setDword(kHelpDue, guest.dword(kTicks) + 100);
        }
        else if (std::int32_t(guest.dword(kTicks)) > std::int32_t(guest.dword(kHelpDue)) && index != x86::reg32(-1)
                 && guest.dword(kHelpNeedsMove) == 0)
        {
            // The box: the text's size, wrapped, and a margin.
            guest.setDword(kHelpItem, index);
            guest.setDword(kHelpFade, 0);
            guest.setDword(kHelpText, guest.dword(itemAt(std::int32_t(index)) + fe::item::kHelp));
            const x86::reg32 width = guest.frame(), height = guest.frame() + 4;  // [ebp - 8], [ebp - 4]
            guest.call(kTextBox, fe::args({ guest.dword(kHelpText), kHelpFont, kHelpWrap, width }, { height, 0 }));
            const std::int32_t w = std::int32_t(guest.dword(width)) + 7, h = std::int32_t(guest.dword(height)) + 3;
            guest.setDword(kHelpX, guest.dword(kMouseX));
            guest.setDword(kHelpWidth, x86::reg32(w));
            guest.setDword(kHelpHeight, x86::reg32(h));
            if (std::int32_t(guest.dword(kMouseX)) + w > 630)
                guest.setDword(kHelpX, x86::reg32(630 - w));
            guest.setDword(kHelpY, guest.dword(kMouseY) + 20);
            if (std::int32_t(guest.dword(kMouseY)) + 20 + h > 460)
                guest.setDword(kHelpY, x86::reg32(std::int32_t(guest.dword(kMouseY)) - h - 8));
        }
    }

    const x86::reg32 text = guest.dword(kHelpText);
    if (text != 0 && guest.byte(text) == 0)
        guest.setDword(kHelpText, 0);
    if (guest.dword(kHelpText) != 0)
    {
        if (guest.dword(kHelpItem) == x86::reg32(-1))
        {
            if (std::int32_t(guest.dword(kHelpFade)) > 0)
                guest.setDword(kHelpFade, 0);
            guest.setDword(kHelpText, 0);
        }
        else if (guest.dword(kHelpFade) != 255)
            guest.setDword(kHelpFade, 255);
    }
    if (guest.dword(kHelpText) == 0)
    {
        guest.setDword(kHelpAlpha, kFloatOne);
        return guest.eax();
    }

    const x86::reg32 x = guest.dword(kHelpX), y = guest.dword(kHelpY);
    const x86::reg32 w = guest.dword(kHelpWidth), h = guest.dword(kHelpHeight);
    const x86::reg32 alpha = guest.dword(kHelpFade) << 24;
    guest.setDword(kDrawColour, alpha);
    guest.call(kFillBox, fe::args({ x, y, x + w, y + h }, { x86::reg32(-100), kHelpBoxColour }));
    guest.call(kBoxBorder, fe::args({ x, y, w, h }, { alpha }));
    if (guest.dword(kInRace) != 0)
        guest.call(kDrawTextInRace, fe::args({ guest.dword(kHelpText), x, y, kHelpFont }, { 0, kHelpTextColour }));
    else
        guest.call(kDrawTextWrapped, fe::args({ guest.dword(kHelpText), kHelpFont, 8, x + 7 }, { y, kHelpWrap, 0, 0, 0, 0 }));
    guest.setDword(kDrawColour, 0xff000000);
    guest.setDword(kHelpAlpha, kFloatOne);
    return guest.eax();
}


/* sub_44be60: the race settings kept consistent on every pass, and the options
 * the screens offer fitted to them (sub_43a6e0: an option allowed, by its
 * bits).  Which setting each of these is is not pinned down yet; they are
 * named by their place in the settings (0x6fd2a0 on).  Two of them follow
 * each other -- 0 goes with 0, 1 with 4, 2 with 1 to 3 -- whichever changed
 * since the last pass setting the other.  Nothing after a race. */
x86::reg32 fitRaceSettings(Guest& guest)
{
    if (guest.dword(kAfterRace) != 0)
        return guest.eax();
    if (guest.dword(kSeen4d0) == x86::reg32(-1))
    {
        guest.setDword(kSeen4d0, guest.dword(kSetting4d0));
        guest.setDword(kSeen4d4, guest.dword(kSetting4d4));
    }
    if (guest.dword(kSeenRaceType) == x86::reg32(-1))
        guest.setDword(kSeenRaceType, guest.dword(kRaceType));
    guest.setDword(kSettingBbd8, guest.dword(kSettingBbe4) == 0 ? 1 : 0);
    if (guest.dword(kSettingBc00) == 1)
        guest.setDword(kSetting292c, 0);
    else if (guest.dword(kSettingBc00) == 0)
        guest.setDword(kSetting292c, 1);
    if (guest.dword(kRaceType) == kHotPursuit)
        guest.setDword(kSetting4d0, 2);

    const std::uint32_t first = guest.dword(kSetting4d0);
    if (first != guest.dword(kSeen4d0))
    {
        guest.setDword(kSeen4d0, first);
        auto setSecond = [&](std::uint32_t value) {
            guest.setDword(kSeen4d4, value);
            guest.setDword(kSetting4d4, value);
        };
        if (first == 0 && guest.dword(kSetting4d4) != 0)
            setSecond(0);
        else if (first == 1 && guest.dword(kSetting4d4) == 0)
            setSecond(4);
        else if (first == 2 && guest.dword(kSetting4d4) == 0)
            setSecond(1);
    }
    else if (guest.dword(kSetting4d4) != guest.dword(kSeen4d4))
    {
        const std::uint32_t second = guest.dword(kSetting4d4);
        guest.setDword(kSeen4d4, second);
        auto setFirst = [&](std::uint32_t value) {
            guest.setDword(kSeen4d0, value);
            guest.setDword(kSetting4d0, value);
        };
        if (second == 0)
            setFirst(0);
        else if (second <= 3)
        {
            if (guest.dword(kSetting4d0) == 0)
                setFirst(2);
        }
        else if (guest.dword(kSetting4d0) == 0)
            setFirst(1);
    }
    if (guest.dword(kRaceType) != guest.dword(kSeenRaceType))
        guest.setDword(kSeenRaceType, guest.dword(kRaceType));

    auto allow = [&](std::uint32_t option, std::uint32_t on) { guest.call(kAllowOption, fe::args({ option, on })); };
    allow(0xd, 0);
    allow(0x4004, 1);
    if (std::int32_t(guest.dword(kSplitScreen)) >= 2)
    {
        if (guest.dword(kRaceType) == kTournament || guest.dword(kRaceType) == kKnockout)
        {
            if (guest.dword(kSetting504) == 3)
                guest.setDword(kSetting504, 0);
            guest.setWord(kSetting639c, 0);
        }
        else
            guest.setWord(kSetting639c, 1);
        allow(0x40004, 1);
        switch (guest.dword(kSetting504))
        {
        case 0:
            allow(0x66c, 1);
            break;
        case 1:
            allow(0x55c, 1);
            break;
        case 2:
            allow(0x33c, 1);
            break;
        default:
            break;
        }
    }
    if (guest.dword(kRaceType) == kTournament || guest.dword(kRaceType) == kKnockout)
    {
        guest.setDword(kSetting4e8, 1);
        guest.setDword(kSetting4d0, 2);
        allow(0x884, 1);
        switch (guest.dword(kSetting4d4))
        {
        case 1:
            allow(0x68, 1);
            break;
        case 2:
            allow(0x58, 1);
            break;
        case 3:
            allow(0x38, 1);
            break;
        default:
            break;
        }
    }
    if (guest.dword(kRaceType) != kHotPursuit)
        allow(0x4008, 1);
    else
    {
        allow(0x2008, 1);
        const std::int32_t players = std::int32_t(guest.dword(kSplitScreen));
        bool cops;
        if (players >= 2)
            cops = signedWord(guest, kNetworkSetting2276) == std::int32_t(guest.dword(kNetworkSettingE5d0));
        else if (players == 1)
            cops = guest.call(kCarFlag10, fe::args({ guest.dword(kPlayer1Car) })) != 0
                   && guest.call(kCarFlag10, fe::args({ guest.dword(kPlayer2Car) })) != 0;
        else
            cops = true;
        allow(cops ? 0x2084 : 0xf04, 1);
    }

    // The car screens: the name of the one shown, against each.
    auto shown = [&](const fe::GuestString& name) {
        const x86::reg32 current = guest.call(kCurrentMenuName, fe::args({ guest.eax(), name.address }));
        return guest.call(kStrcmp, fe::args({ current, name.address })) == 0;
    };
    if (shown(kShowcaseScreen) || shown(kInteriorScreen))
    {
        allow(9, 1);
        allow(0x80008, 1);
    }
    if (shown(kHistoryScreen))
    {
        allow(9, 1);
        allow(0x280008, 1);
    }
    if (shown(kCompareScreen))
        allow(9, 1);

    auto widen = [](x86::reg32 value) { return x86::reg32(std::int32_t(std::int16_t(value))); };
    if (guest.dword(kRaceType) == kTournament)
    {
        const x86::reg32 kind = std::int32_t(guest.dword(kSplitScreen)) > 1 ? 2 : 4;
        guest.setDword(kSetting4b0, widen(guest.call(kMap4cf160, fe::args({ kind }))));
        guest.setDword(kSetting4ac, guest.dword(kValue5d004));
    }
    else if (guest.dword(kRaceType) == kKnockout)
    {
        guest.setDword(kSetting4b0, widen(guest.call(kMap4cf160, fe::args({ 2 }))));
        guest.setDword(kSetting4ac, 0);
    }
    guest.setDword(kSetting4b4,
                   widen(guest.call(kMap4cf190, fe::args({ x86::reg32(std::int32_t(signedWord(guest, kSetting4b0))) }))));
    guest.setDword(kSeen4d0, guest.dword(kSetting4d0));
    guest.setDword(kSeen4d4, guest.dword(kSetting4d4));
    if (guest.dword(kSetting2ac) != 0)
        guest.call(kMarkOption8, fe::args({ 8 }));
    return guest.eax();
}

/* A dialog over the screen asking for a player's name, when a race is
 * started without one: the default name ("Player 1") already in. */
void askName(Guest& guest, x86::reg32 name, std::uint32_t title, std::uint32_t defaultText, x86::reg32 answer)
{
    guest.setDword(kDialogLines, fe::text(guest, title));
    guest.setDword(kDialogLines + 4, fe::text(guest, kNameDialogLine2));
    guest.setDword(kDialogButtons, fe::text(guest, kNameDialogOk));
    guest.setDword(kDialogButtons + 4, fe::text(guest, kNameDialogCancel));
    guest.copyString(name, fe::text(guest, defaultText));
    fe::Dialog dialog;
    dialog.lineCount = 2;
    dialog.lines = kDialogLines;
    dialog.buttonCount = 2;
    dialog.buttons = kDialogButtons;
    dialog.focus = 0;
    dialog.edit = name;
    dialog.editLength = kPlayerNameLength;
    dialog.editMode = 1;
    dialog.callback = answer;
    fe::openDialog(guest, dialog);
}

/* sub_44b230: the menus, from the screen `name` (its .MNU and every one it
 * leads to loaded first) until the player leaves them: the action that
 * ended them, 10 to quit or 11 out of the first screen.  `history`, when
 * not null, names the screens to come back through (a list ending in a
 * null), the first of them shown first.
 *
 * Each screen: opened (its onEnter, which may already leave it), then a pass
 * after pass until something leaves it -- its onFrame handler, the items
 * drawn and passed (or a dialog's), the help box, the pointer's item
 * selected, a key handed on.  What left it says where to go:
 *   1, 3   back: to the screen gone from last (out of the menus from the first)
 *   4      back to the first screen
 *   5      on: to the selected button's nextmenu (asking the players' names
 *          first, when they have none)
 *   6, 7   into the race
 *   9      a movie played: to the screen it named, if any
 *   10     quit (asked first), 11 out */
x86::reg32 menuLoop(Guest& guest, x86::reg32 name, x86::reg32 history)
{
    const x86::reg32 action = guest.frame();       // [ebp - 0x10]: sub_44b1a0 sets it
    const x86::reg32 offset = guest.frame() + 4;   // [ebp - 0xc]
    bool running = true, firstScreen = true;
    guest.setDword(action, 0);
    guest.setDword(offset, 0);

    guest.setDword(kAfterRace, guest.call(kStrcmp, fe::args({ name, kPostGame.address })) == 0 ? 1 : 0);
    guest.setDword(kInRace, 0);
    guest.setDword(kFlagEb10, 0);
    guest.setDword(kFrontendArt, 0);
    guest.call(kSetDisplay, fe::args({ guest.dword(kDisplayChoice) }));
    guest.call(kResetInput);
    const x86::reg32 afterRace = guest.dword(kAfterRace);
    guest.call(kStartMusic, fe::args({ afterRace != 0 ? guest.dword(kSetting4ac) : x86::reg32(0x16),
                                       guest.dword(kMusicChoice), afterRace }));
    guest.call(kLoadMenus, fe::args({ name, 1 }));
    guest.setDword(kLastAction, 5);
    for (std::uint32_t i = 0; i < kHistorySize; ++i)
        guest.setDword(kHistory + 4 * i, 0);
    if (history != 0)
        guest.call(kSetHistory, fe::args({ history }));
    else
        guest.setDword(kCurrentMenu, kMenus);

    while (running)
    {
        guest.setDword(action, guest.call(kEnterScreen, fe::args({ guest.dword(kCurrentMenu) })));
        if (firstScreen)
        {
            guest.call(kSelectNext, fe::args({ guest.dword(kCurrentMenu) }));
            firstScreen = false;
        }
        if (guest.dword(action) != 0)
            guest.setDword(action, 3);
        guest.setDword(kScreenUp, 1);
        playSound(guest, kSoundEnter, kVolumeFull);

        while (guest.dword(action) == 0)
        {
            guest.call(kPollEvents);
            const x86::reg32 menu = guest.dword(kCurrentMenu);
            if (guest.dword(menu + fe::menu::kOnFrame) != 0)
                guest.setDword(action, guest.call(guest.dword(menu + fe::menu::kOnFrame), fe::args({ menu, menu })));
            guest.call(kFitRaceSettings);
            if (guest.dword(action) == 0)
            {
                guest.call(kBeginFrame);
                guest.setDword(action, guest.call(kDrawItems, fe::args({ guest.dword(kCurrentMenu) })));
                if ((guest.call(kDialogShown) & 0xff) != 0)
                    guest.setDword(action, guest.call(kDialogPass));
                guest.call(kPassItems, fe::args({ guest.dword(kCurrentMenu) }));
                guest.call(kPresentFrame);
                const std::uint8_t noHelp = guest.byte(kNoHelpFlags);
                if ((noHelp & 0x10) == 0 && (noHelp & 0x20) == 0)
                    guest.call(kDrawHelp);
                guest.call(kEndFrame);
            }
            if (guest.dword(action) == 0)
            {
                // The network screens hold the mouse only while connected with no network race set up.
                if (guest.word(kNetworkGame) != 0 || guest.dword(kNetworkConnected) == 0)
                    guest.setDword(kMouseBlocked, 0);
                const x86::reg32 underMouse = guest.call(kItemUnderMouse, fe::args({ guest.dword(kCurrentMenu) }));
                const x86::reg32 current = guest.dword(kCurrentMenu);
                const std::int16_t was = selected(guest, current);
                if (x86::reg32(std::int32_t(was)) != underMouse
                    && guest.call(kSelectable, fe::args({ x86::reg32(std::int32_t(was)), current })) != 0)
                {
                    guest.call(kItemChanged);
                    const x86::reg32 shown = guest.dword(kCurrentMenu);
                    guest.setWord(shown + kPrevious, guest.word(shown + fe::menu::kSelected));
                }
                setSelected(guest, guest.dword(kCurrentMenu),
                            underMouse == x86::reg32(-1) ? kNoItem : std::int32_t(std::int16_t(underMouse)));
                const std::uint16_t key = std::uint16_t(guest.call(kReadKey, fe::args({ 1 })));
                if (key != 0)
                {
                    const x86::reg32 code = x86::reg32(std::int32_t(std::int16_t(key)));
                    if ((guest.call(kDialogShown) & 0xff) != 0)
                        guest.setDword(action, guest.call(kDialogKey, fe::args({ code })));
                    else
                        guest.setDword(action, guest.call(kHandleKey, fe::args({ guest.dword(kCurrentMenu), code })));
                    if (guest.call(kWaitWhileSuspended, fe::args({ guest.dword(kCurrentMenu), action })) != 0)
                        continue;
                }
            }

            if (guest.dword(action) == kQuit && guest.byte(kQuitting) == 0 && guest.dword(kNoQuitQuestion) == 0)
            {
                guest.setDword(kDialogLines, fe::text(guest, kQuitQuestion));
                guest.setDword(kDialogButtons, fe::text(guest, kQuitYes));
                guest.setDword(kDialogButtons + 4, fe::text(guest, kQuitNo));
                fe::Question question;
                question.lineCount = 1;
                question.lines = kDialogLines;
                question.buttonCount = 2;
                question.buttons = kDialogButtons;
                question.buttonHelp = 0;
                question.focus = 0;
                question.column = 0;
                question.callback = kQuitAnswer;
                fe::openQuestion(guest, question);
                guest.setDword(action, 0);
                continue;
            }
            if (guest.dword(action) == kGoOn)
            {
                if (guest.byte(kPlayer1Name) == 0)
                {
                    askName(guest, kPlayer1Name, kPlayer1NameTitle, kPlayer1Default, kPlayer1NameAnswer);
                    guest.setDword(action, 0);
                }
                else if (guest.dword(kSplitScreen) == 1 && guest.byte(kPlayer2Name) == 0)
                {
                    askName(guest, kPlayer2Name, kPlayer2NameTitle, kPlayer2Default, kPlayer2NameAnswer);
                    guest.setDword(action, 0);
                }
            }
            if (guest.dword(action) == fe::kMenuBackToFirst && guest.dword(kCurrentMenu) == kMenus)
                guest.setDword(action, 0);
        }

        // The screen left.
        if (guest.dword(kScreenShapes) != 0)
        {
            guest.call(kFree, fe::args({ guest.dword(kScreenShapes) }));
            guest.setDword(kScreenShapes, 0);
        }
        guest.setDword(kScreenUp, 0);
        if (guest.dword(kNetworkConnected) != 0)
            guest.call(kNetworkLeft);
        guest.setDword(kLastAction, guest.dword(action));
        guest.call(kScreenLeft, fe::args({ guest.dword(guest.dword(kCurrentMenu) + fe::menu::kName) }));

        bool back = false;
        switch (guest.dword(action))
        {
        case fe::kMenuBackToFirst:
            guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
            while (guest.dword(kCurrentMenu) != kMenus)
                guest.call(kPopMenu);
            break;
        case kGoOn:
        {
            const x86::reg32 menu = guest.dword(kCurrentMenu);
            const x86::reg32 item = menuItem(guest, menu, selected(guest, menu));
            if (guest.call(kEscapeButton, fe::args({ menu, offset, item, 0 })) == item)
            {
                guest.setDword(action, 3);
                back = true;
                break;
            }
            const x86::reg32 next = guest.dword(item + fe::item::kType) == kButton ? guest.dword(item + fe::item::kNextMenu) : 0;
            if (next != 0)
            {
                for (guest.setDword(offset, 0); std::int32_t(guest.dword(offset)) < std::int32_t(guest.dword(kMenuCount));
                     guest.setDword(offset, guest.dword(offset) + 1))
                {
                    if (guest.call(kStrcmp, fe::args({ next, guest.dword(menuAt(guest.dword(offset)) + fe::menu::kName) })) == 0)
                        break;
                }
                if (std::int32_t(guest.dword(offset)) < std::int32_t(guest.dword(kMenuCount)))
                {
                    guest.call(kPushMenu);
                    guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
                    guest.setDword(kCurrentMenu, menuAt(guest.dword(offset)));
                }
                else
                    guest.setDword(action, 3);
            }
            else
                guest.setDword(action, 3);
            running = true;
            break;
        }
        case kRace:
        case kRace7:
            playSound(guest, kSoundStart, kVolumeFull);
            guest.call(kBeginFrame);
            guest.call(kDrawItems, fe::args({ guest.dword(kCurrentMenu) }));
            running = false;
            guest.setDword(kScreenUp, 1);
            guest.call(kSetCursor, fe::args({ 3 }));
            guest.call(kPresentFrame);
            guest.call(kEndFrame);
            guest.setDword(kScreenUp, 0);
            guest.setDword(kFlagEb10, 0);
            guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
            break;
        case fe::kMenuMoviePlayed:
            guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
            if (guest.dword(kAfterMovieScreen) != 0
                && guest.call(kMenuByName, fe::args({ guest.dword(kAfterMovieScreen) })) != 0)
            {
                guest.setDword(kCurrentMenu, guest.call(kMenuByName, fe::args({ guest.dword(kAfterMovieScreen) })));
                guest.setDword(kAfterMovieScreen, 0);
            }
            running = true;
            break;
        case kQuit:
            guest.setDword(kFlagEb10, 0);
            guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
            running = false;
            break;
        case kOut:
            guest.setDword(kFlagEb10, 0);
            running = false;
            guest.call(kLeaveScreen, fe::args({ guest.dword(kCurrentMenu), guest.dword(action) }));
            break;
        default:
            break;
        }
        if (back || guest.dword(action) == 3 || guest.dword(action) == 1)
        {
            const x86::reg32 menu = guest.dword(kCurrentMenu);
            if (menu == kMenus)
            {
                guest.setDword(action, kOut);
                running = false;
            }
            else
            {
                guest.call(kLeaveScreen, fe::args({ menu, guest.dword(action) }));
                guest.call(kPopMenu);
            }
        }
    }

    guest.call(kCheckMenuFiles);
    if (guest.dword(kFrontendArt) != 0)
    {
        guest.call(kFree, fe::args({ guest.dword(kFrontendArt) }));
        guest.setDword(kFrontendArt, 0);
    }
    guest.call(kMenusEnd);
    guest.call(kCloseDialog);
    guest.call(kStopMusic);
    return guest.dword(action);
}


const fe::GuestString kUsableStrings[] = { kRaceNow };
const fe::GuestString kEscapeStrings[] = { kEscape };
const fe::GuestString kDrawStrings[] = { kIpx };
const fe::GuestString kEnterStrings[] = { kMmnu, kQfsFormat, kFshFormat, kOpponent, kCar1, kCar2 };
const fe::GuestString kSettingsStrings[] = { kShowcaseScreen, kInteriorScreen, kHistoryScreen, kCompareScreen };
const fe::GuestString kLoopStrings[] = { kPostGame };

}

/* Each stand-in: the generated function, what its ret pops, its frame (the
 * registers it pushes, its locals), the strings it hands to the game, and the
 * registers it restores. */
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::kRestoreEdi;
constexpr std::uint8_t kRestoreNone = 0;

bool menuItemUsable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4499b0", 0, 8, kUsableStrings, std::size(kUsableStrings), kRestoreEdx };
    const x86::reg32 item = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return itemUsable(guest, item); });
}

bool menuKeyAllowed(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4499f0", 0, 8, nullptr, 0, kRestoreEcx };
    const x86::reg32 menu = cpu.eax;
    const std::uint16_t key = std::uint16_t(cpu.edx);
    return fe::run(app, cpu, site, [&](Guest& guest) { return keyAllowed(guest, menu, key); });
}

bool menuDrawHelp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449aa0", 0, 24 + 8, nullptr, 0 };
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawHelp(guest); });
}

bool menuKeysTaken(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449df0", 0, 8, nullptr, 0, kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return keysTaken(guest); });
}

bool menuSelectable(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449e50", 0, 4, nullptr, 0, kRestoreNone };
    const std::int32_t offset = std::int32_t(cpu.eax);
    const x86::reg32 menu = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return selectable(guest, offset, menu); });
}

bool menuPointAtSelected(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449ea0", 0, 12, nullptr, 0, kRestoreEcx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return pointAtSelected(guest, menu); });
}

bool menuSelectPrevious(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_449f30", 0, 20 + 4, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return selectPrevious(guest, menu); });
}

bool menuSelectNext(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a0c0", 0, 20 + 4, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return selectNext(guest, menu); });
}

bool menuEscapeButton(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a240", 0, 20 + 4, kEscapeStrings, std::size(kEscapeStrings),
                          kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 menu = cpu.eax, offset = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return escapeButton(guest, menu, offset); });
}

bool menuHandleKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a2d0", 0, 20 + 4, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 menu = cpu.eax, key = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return handleKey(guest, menu, key); });
}

bool menuSelectedIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a440", 0, 8, nullptr, 0, kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return selectedIndex(guest); });
}

bool menuSelectItem(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a470", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 item = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return selectItem(guest, item); });
}

bool menuDrawItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a4f0", 0, 24 + 0xc, kDrawStrings, std::size(kDrawStrings) };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return drawItems(guest, menu); });
}

bool menuShownNow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a660", 0, 4, nullptr, 0, kRestoreNone };
    const x86::reg32 item = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return shownNow(guest, item); });
}

bool menuReleaseMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a690", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return releaseMouse(guest); });
}

bool menuItemUnderMouse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a6b0", 0, 24 + 0x10, nullptr, 0 };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return itemUnderMouse(guest, menu); });
}

bool menuItemsShown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a860", 0, 12, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return itemsShown(guest, menu); });
}

bool menuPassItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a8b0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return passItems(guest, menu); });
}

bool menuShowItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a960", 0, 12, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return showItems(guest, menu); });
}

bool menuHideItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44a9e0", 0, 12, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return hideItems(guest, menu); });
}

bool menuLeaveScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44aa30", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi };
    const x86::reg32 menu = cpu.eax, action = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return leaveScreen(guest, menu, action); });
}

bool menuResetItems(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44aad0", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return resetItems(guest, menu); });
}

bool menuEnterScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ab40", 0, 20 + 0x100, kEnterStrings, std::size(kEnterStrings),
                          kRestoreEcx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 menu = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return enterScreen(guest, menu); });
}

bool menuCurrentName(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ae00", 0, 8, nullptr, 0, kRestoreEdx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return currentMenuName(guest); });
}

bool menuByNameStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44ae60", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 name = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return menuByName(guest, name); });
}

bool menuCurrent(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44aeb0", 0, 4, nullptr, 0, kRestoreNone };
    return fe::run(app, cpu, site, [&](Guest& guest) { return currentMenu(guest); });
}

bool menuPop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44aec0", 0, 12, nullptr, 0, kRestoreEbx | kRestoreEcx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return popMenu(guest); });
}

bool menuPush(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44af10", 0, 8, nullptr, 0, kRestoreEcx };
    return fe::run(app, cpu, site, [&](Guest& guest) { return pushMenu(guest); });
}

bool menuSetHistory(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44af50", 0, 20, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const x86::reg32 names = cpu.eax;
    return fe::run(app, cpu, site, [&](Guest& guest) { return setHistory(guest, names); });
}

bool menuPlayer1NameAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44af90", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 answer = cpu.eax, close = cpu.edx;
    return fe::run(app, cpu, site,
                   [&](Guest& guest) { return nameAnswer(guest, answer, close, kPlayer1Name, kPlayer1Default); });
}

bool menuPlayer2NameAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44b040", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 answer = cpu.eax, close = cpu.edx;
    return fe::run(app, cpu, site,
                   [&](Guest& guest) { return nameAnswer(guest, answer, close, kPlayer2Name, kPlayer2Default); });
}

bool menuQuitAnswer(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44b0f0", 0, 8, nullptr, 0, kRestoreEcx };
    const x86::reg32 answer = cpu.eax, close = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return quitAnswer(guest, answer, close); });
}

bool menuWaitWhileSuspended(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44b1a0", 0, 16, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi };
    const x86::reg32 menu = cpu.eax, action = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return waitWhileSuspended(guest, menu, action); });
}

bool menuLoopStandIn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44b230", 0, 20 + 0x10, kLoopStrings, std::size(kLoopStrings),
                          kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const x86::reg32 name = cpu.eax, history = cpu.edx;
    return fe::run(app, cpu, site, [&](Guest& guest) { return menuLoop(guest, name, history); });
}

bool menuFitRaceSettings(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_44be60", 0, 20, kSettingsStrings, std::size(kSettingsStrings),
                          kRestoreEbx | kRestoreEdx | kRestoreEsi | kRestoreEdi };
    return fe::run(app, cpu, site, [&](Guest& guest) { return fitRaceSettings(guest); });
}

}
